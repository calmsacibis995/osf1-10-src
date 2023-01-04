/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: siop_kn430.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/10 17:07:16 $";
#endif

/***************************************************************************
 *
 * Machine depend part of NCR53C710 driver for KN430 (Cobra) CPU.
 * This code knows about the particulars of the Cobra platform and the
 * Cobra SCRIPTS.
 ***************************************************************************/
#include <sys/types.h>
#include <hal/kn430.h>
#include <io/dec/mbox/mbox.h>
#include <io/common/devdriver.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/lock.h>
#include <dec/binlog/errlog.h>  /* UERF errlog defines */
#include <arch/alpha/machparam.h>

#define CAMERRLOG

/* cam include files */
#include <io/common/iotypes.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <kern/parallel.h>
#include <kern/sched_prim.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/cam_logger.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/cam_errlog.h>


#include <io/cam/siop/scripthost.h>
#include <io/cam/siop/siopdefs.h>
#include <io/cam/siop/kn430/scripts/scriptram_kn430.h>


/* include the SCRIPTS compiled by the compiler */
#include <io/cam/siop/kn430/script_kn430.h>

#define KN430_SIOP_PER_LBUS	5

/* Forward references */
U32 siop_kn430_script_cntl();
int siopcointr();
void siop_kn430_csr_write();
U32 siop_kn430_csr_read();

/* #define SIOP_VERBOSE */
/* #define SIOP_PTR_DEBUG */

/* The following macro overrides SIM_PRINTD since compiling with CAMDEBUG
 * defined produces a huge kernel.  If you want diagnostic printouts just
 * un-comment out the following macro.
#define SIM_PRINTD(B,T,L,F,X) \
        if(F & (CAMD_ERRORS|CAMD_CONFIG)) \
        { \
            printf("SIOP Bus %d, target %d, lun %d: ",B,T,L); \
            printf X ; \
        }
 */


/* SCRIPTS BASE ADDRESSES */
U32 Siop_scriptbase[] = { 0, 0x6600, 0xcc00, 0x13200, 0x19800 };

/* Keep this around for now (with it's magic number) until the machine
 * independent module is implemented.
 */
SCRIPT_HOST *Siop_sh[KN430_SIOP_PER_LBUS];

/* Use the logging code in the main SIOP driver */
extern void siop_logger();
static void (*local_errlog)() = siop_logger;

/* The following table is used to set the SCSI bus speed based on the
 * console bus speed setting.  The nearest possible chip setting
 * less than or equel to the requested speed is used.  Requested speeds
 * are rounded down to the nearest speed the chip can support given
 * then chip's clock rate.
 */

static struct bus_speed {
	U8	period;		/* minimum transfer period allowed */
	U8	fast;		/* run the chip in "fast" mode */
	U16	speed;		/* Speed in Mbytes/sec */
} siop_bus_speed[] = {
	{ 20,	1,	12500  },
	{ 25,	1,	10000  },
	{ 30,	1,	8334   },
	{ 35,	1,	7143   },
	{ 40,	1,	6125   },
	{ 45,	1,	5557   },
	{ 50,	0,	5000   },
	{ 55,	1,	4545   },
	{ 60,	0,	4167   },
	{ 70,	0,	3571   },
	{ 80,	0,	3125   },
	{ 90,	0,	2777   },
	{ 100,	0,	2500   },
	{ 110,	0,	2273   },
	{ 0,	0,	0      },
};


/**************************************************************************
 *              DRIVER INITIALIZATION ROUTINES                            *
 **************************************************************************/

/* attach routine:       siop_kn430_attach()
 *
 *      arguments:      scp		SIM_SOFTC pointer for controller
 *
 *      returns:        0       if controller's not present
 *                      1       if controllers present.
 *
 *	This is the machine dependent part of the probe/attach sequence for the
 *	KN430 (Cobra) CPU.  The kernel configuration routines call
 *	here if a SCSI controller is found on the LBUS.
 *
 *      Ping each SIOP by reading their ISTAT register.  If the
 *      mailbox command fails then they're not really there.
 *      Also ping the SCRIPTS RAM for the controller.
 *	If all goes well set up the machine dependent function pointers
 *	and fetch the SCSI configuration info from the hardware RPB.
 *	finally, call the machine independent attach function.
 *
 *      The driver used the following members of the controller structure:
 *              slot - SIOP number (number of chip on each bus)
 *              ctlr_num - absolute SIOP number
 *              addr - value passed as first argument to probe routine.
 *                      On Cobra this is a pointer the rpb_device
 *                      structure passed in by the console.
 *              addr2 - the base SCRIPTS address is SCRIPTS RAM
 *              physaddr - the SIOP's base CSR address as seen from the host.
 *                      On Cobra this is the address used by the Mailbox.
 *              physaddr2 - the base address of teh memory allocaed for the
 *                      SCRIPT_HOST structure.
 */

int
siop_kn430_attach(scp)
register SIM_SOFTC *scp;
{
    register SCRIPT_HOST *shp;
    register struct controller *ctlr;
    register U64 l;
    extern I32 scsi_bus_reset_at_boot;
    extern char cam_ctlr_string[];
    SIM_MODULE(siop_kn430_attach);

    shp = scp->hba_sc;
    ctlr = shp->sh_ctlr;

    /* set the SIOP's base CSR and base SCRIPTS RAM address as seen through
     * the mailbox interface.
     */
    ctlr->physaddr = (caddr_t)((U32)ctlr->slot<<6);

    SIM_PRINTD(ctlr->slot,NULL,NULL,(CAMD_INOUT|CAMD_CONFIG),
		("siop_kn430_probe\n"));

    /* make sure that the SIOP and associated SCRIPTS RAM is responding */

    l = BADADDR(ctlr->physaddr, sizeof(U64), ctlr);
    if(l)
        return 0;
    l = BADADDR(Siop_scriptbase[ctlr->slot]|(1L<<63), sizeof(U64), ctlr);
    if(l)
        return 0;

    ctlr->addr2 = (caddr_t)Siop_scriptbase[ctlr->slot];
    Siop_sh[ctlr->slot] = shp;

    /* The maximum permissable bus transfer rate is set in the hwrpb.
     * Speeds above 13 Mbytes/sec are bogus and are silently set to
     * the chips maximum.  Speeds below 2.77 Mbytes/sec are too low
     * are will cause async operation on the bus.  The fields in
     * the SCRIPTHOST structure are set so that a target requesting
     * syncronous transfer will get the correct response from the
     * driver (with an async response if necessary).
     */
    for(l = 0; siop_bus_speed[l].speed; l++)
	if((U16)((struct rpb_device *)(ctlr->addr))->dev.scsi.speed >=
	     siop_bus_speed[l].speed)
	     break;

    if(siop_bus_speed[l].fast)
	shp->sh_flags |= SIOP_SH_FAST;

    /* set the minimum permissable period.  If this is 0 (async) also
     * make sure that the offset is 0.
     */
    if(!(shp->sh_min_period = siop_bus_speed[l].period))
	shp->sh_max_offset = 0;
    else
	shp->sh_max_offset = SIOP_MAX_OFFSET;

    printf("siop_kn430_attach: SCSI bus %d maximum transfer rate set to %d Kbytes/sec\n",
	ctlr->slot,siop_bus_speed[l].speed);

    /* The console code will decide is a bus is fast or not
    if(((struct rpb_device *)(ctlr->addr))->dev.scsi.fast)
        shp->sh_flags |= SIOP_SH_FAST;
    */

    /* before calling any machine independent routine, set up the
     * machine dependant callbacks in the SCRIPTHOST structure.
     */
    shp->sh_machdep.md_script_cntl = siop_kn430_script_cntl;
    shp->sh_machdep.md_intr = siopcointr;
    shp->sh_machdep.md_csr_write = siop_kn430_csr_write;
    shp->sh_machdep.md_csr_read = siop_kn430_csr_read;

    /* The SIOP SCSI ID is passed in the HW RPB on Cobra.  This allows the
     * user to set the ID from the console and for the console to pass
     * the ID to the OS so that they will use consistent ID's.
     */
    scp->scsiid = ((struct rpb_device *)(ctlr->addr))->dev.scsi.bus_id;

    /* call the hardware independent driver to set up all the stuff
     * needed by CAM.  If this fails then free up the SCRIPT_HOST
     * structure.
     */
    if(!siop_attach(scp))
        return(0);

    return CAM_REQ_CMP;
}

/* General SCRIPTS control function	siop_kn430_script_cntl()
 *
 *	args:	shp		pointer to controllers SCRIPTHOST structure
 *		sjp		pointer to a SIOPJOB if needed
 *		cmd		function to perform
 *		value		value to write if needed
 *
 *	Returns 0 for success, 1 for failure unless the command is for
 *	an operation which returns data.  All SCRIPTS data is 32 bits
 *	since the SIOP is a 32 bit device.
 *
 *	This is a general SCRIPTS manipulation function.  Since the SCRIPTS
 *	layout will be different on each platform, the manipulation of
 *	the SCRIPTS and the SCRIPTS RAM must be done in the machine
 *	dependent code.  The machine independed code can have no knowledge
 *	of the SCRIPTS or their layout.  The machine independent code
 *	will just request that certain operations on SCRIPTS be performed
 *	and leave it to this routine to deal with the particulars.
 */

U32
siop_kn430_script_cntl(shp,sjp,cmd,value)
register SCRIPT_HOST *shp;
register SIOP_JOB *sjp;
U32 cmd, value;
{
    register struct controller *ctlr = shp->sh_ctlr;
    register U32 base;
    register U64 oldpri;
    U64 i;
    SIM_MODULE(siop_kn430_script_cntl);

    switch(cmd)
    {
	case SIOP_SCRIPTS_LOAD:		/* load and start the SCRIPTS */
	    /* zero out all SCRIPTS fields in the SCRIPT_HOST */
	    shp->sh_intstatus = 0;
	    shp->sh_rpcreq = 0;
	    shp->sh_rpcdata = 0;
	    shp->sh_rpcreply = 0;
	    shp->sh_currentjob = 0;
	    shp->sh_jobrequest = 0;
	    shp->sh_jobdone = 0;
	    shp->sh_version = SIOP_SH_VERSION;

	    base = (U32)ctlr->addr2;
	    SIM_PRINTD(ctlr->slot,NULL,NULL,
		       (CAMD_INOUT|CAMD_FLOW|CAMD_CONFIG),
		       ("siop_kn430_script_cntl: script addr = %x\n",base));

	    /* Adjust all relocatable SCRIPTS references by adding the base load
	     * address to the SCRIPTS instruction to be relocated.  Each value
	     * of the LABELPATCHES array is the offset into the SCRIPTS array of
	     * the intruction to be relocated.  These are setup by the SCRIPTS
	     * compiler.  PATCHES is the size of the LABELPATCHES array.
	     */
	    for(i = 0; i < PATCHES; i++)
		SCRIPT[LABELPATCHES[i]] += base;

	    /* Now download the BOOT SCRIPT.  I assume the SCRIPTS array is in
	     * contiguous physical memory since it was loaded as part of kernel
	     * dspace.
	     *
	     * The boot SCRIPT is hand build in SCRIPTS RAM and then executed.
	     * It will then copy in the current contents of the SCRIPT array
	     * starting at the SIOP's SCRIPTS base address (in base).  Once
	     * the SCRIPTS have been copied, it then jumps to the SCRIPTS
	     * entry point (the base address).
	     *
	     * The SCRIPTS boot code created below would look something like:
	     *
	     *  move memory SCRIPT_SIZE, &SCRIPT, base
	     *  jump    base
	     */
	    WRTCSR(LONG0,ctlr,BOOTBASE|(1L<<63),0xc0000000|(SCRIPT_SIZE*4));

	    if(pmap_svatophys(SCRIPT, &i) == KERN_INVALID_ADDRESS)
		panic("siop_startscript: can't map SCRIPT");
	    WRTCSR(LONG0,ctlr,BOOTBASE+4|(1L<<63),i);

	    i = base|0x80000000;                /* destination address */
	    WRTCSR(LONG0,ctlr,BOOTBASE+8|(1L<<63),i);

	    /* JUMP instruction */
	    WRTCSR(LONG0,ctlr,BOOTBASE+12|(1L<<63),0x80080000);

	    /* JUMP destination */
	    WRTCSR(LONG0,ctlr,BOOTBASE+16|(1L<<63),i);

	    /* now poke the address of the scripthost struct so it is present
	     * when the SCRIPTS start.
	     */
	    if(pmap_svatophys(shp, &i) == KERN_INVALID_ADDRESS)
		panic("siop_startscript: can't map scripthost");

	    SIM_PRINTD(ctlr->slot,NULL,NULL,
		       (CAMD_INOUT|CAMD_FLOW|CAMD_CONFIG),
		       ("scripthost at %x\n",i));

	    WRTCSR(LONG0,ctlr,(U64)base+SIOP_SR_SCRIPTHOST|(1L<<63),i);

	    /* Now kick off the boot SCRIPT. */
	    i = BOOTBASE|0x80000000;
	    siop_kn430_csr_write(shp,CSR_LONG,SIOP_REG_DSP0,i);

	    SIM_PRINTD(ctlr->slot,NULL,NULL,(CAMD_INOUT|CAMD_FLOW|CAMD_CONFIG),
	    	       ("siop_start_script: starting boot script... "));

	    /* now wait for the SCRIPTS to load and start.  This could take
	     * a while since the SCRIPTS must get downloaded first. This could
	     * a very long time, but we must wait here because we have to know
	     * that the SIOP has finished with the SCRIPTS array before
	     * moving on. Give the SCRIPTS w while to download and start.
	     * If it's not done by then, assume it's broke.
	     */
	    /* spin for some arbitrarily long period of time */
	    for(i = 0; i < 10000; i++)
	    {
		if(shp->sh_intstatus)
		    break;
		DELAY(100);
	    }

	    /* If the SCRIPTS start then the first things they will do after
	     * clearing
	     * local memory is a GOING_IDLE interrupt.  This will result in the
	     * setting of sh_intstatus.  This is how the host can confirm that
	     * the SCRIPTS are running.
	     */
	    if(!shp->sh_intstatus)
	    {
		CAM_ERROR(module,"SCRIPTS did not start", 
			  SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
			  shp->sh_softc,NULL,NULL);
		shp->sh_flags &= ~SIOP_SH_ALIVE;
		siop_dumpregs(shp);
		(void)siop_hardintr(shp);
	    }
	    else
	    {
		SIM_PRINTD(ctlr->slot,NULL,NULL,
			   (CAMD_INOUT|CAMD_FLOW|CAMD_CONFIG),
		    	   ("SCRIPTS started\n"));
		shp->sh_flags |= SIOP_SH_ALIVE;

		/* read the DSTAT to clear the interrupt condition */
		i = siop_kn430_csr_read(shp,CSR_LONG,SIOP_REG_DSTAT);

		/* enable SCRIPTS interrupts  and continue SCRIPTS */
		i = DCNTL_STD|DCNTL_EA|DCNTL_COM;
		i <<= 8;
		i |= SIOP_WATCHDOG;
		i <<= 8;
		i |= DIEN_BF|DIEN_ABRT|DIEN_SIR|DIEN_WTD|DIEN_IID|DIEN_SSI;
		i <<= 8;
		siop_kn430_csr_write(shp,CSR_LONG,SIOP_REG_DMODE,i);
	    }

	    /* unadjust all relacatable SCRIPTS values */
	    for(i = 0; i < PATCHES; i++)
		SCRIPT[LABELPATCHES[i]] -= base;

	    i = !shp->sh_intstatus;
	    return i;
	    break;

		/* clear SCRIPTS synchronous parameter table entry for a
		 * target ID.  The target is passed in the value argument.
		 * This code must know about the layout of the SCRIPTS
		 * synchronous transfer parameters table.
		 */
	case SIOP_SCRIPTS_CLEAR_SYNC:
		i = (U64)ctlr->addr2 + SIOP_SR_TDTABLE;
		i += value * 4;
		i |= 1L<<63;
		WRTCSR(LONG0,ctlr,i,0);
		break;

		/* Set a request cache (tagged or untagged) line entry to
		 * the supplied value.  This code must know about the
		 * layout of the caches on SCRIPTS RAM.
		 */
	case SIOP_SCRIPTS_SET_CACHE:
	    if(!sjp)
		return 1;
	    if(!sjp->sj_tagged)
	    {
		i = (U64)ctlr->addr2 + SIOP_SR_JOBTABLE;
		i += sjp->sj_lun * 4;
		i |= 1L<<63;
		oldpri = splsiop();
		WRTCSR(LONG0,ctlr,i,(U64)value);
		splx(oldpri);
	    }
	    else
	    {
		/* zero out the tag cache line entry */
		register U32 cache_entry;

		i = sjp->sj_tagged;
		i &= ~SCRIPTRAMBASE;
		i &= SCRIPT_RAM_MASK;
		i |= 1L<<63;
		oldpri = splsiop();
		cache_entry = (U32)RDCSR(LONG0,ctlr,i);
		/* only clear the cache line entry if the entry tag matches
		 * this job.
		 */
		if((cache_entry & 7) == (sjp->sj_lun & 7))
		    WRTCSR(LONG0,ctlr,i,(U64)value);
		splx(oldpri);
	    }
	    break;

	    /* Read a longword from SCRIPT RAM.  Value is an offset into
	     * SCRIPTS RAM.  Take care of any addressing peculiarities
	     * for the hardware platform.
	     */
	case SIOP_SCRIPTS_READ_RAM:
	    return RDCSR(LONG0,ctlr,(U64)value|(1L<<63));
	    break;

	    /* Set the DSP to the value specified.  This value is really
	     * an offset from the controller's base SCRIPTS address.
	     * Take care of the machine dependent address calculations.
	     */
	case SIOP_SCRIPTS_SET_DSP:
	    i = (U64)value + (U64)ctlr->addr2;
	    i += SCRIPTRAMBASE;
	    siop_kn430_csr_write(shp,CSR_LONG,SIOP_REG_DSP0,i);
	    break;

	    /* Get a DSP and convert it to a SCRIPT RAM address as
	     * seen in the host address space.
	     */
	case SIOP_SCRIPTS_GET_DSP:
	    i = (U64)siop_kn430_csr_read(shp,CSR_LONG,SIOP_REG_DSP0);
	    i &= SCRIPT_RAM_MASK;
	    return (U32)i;

	    /* Calculate the address of a tag cache entry for the request's
	     * target/lun/tag.  Take care of hardware specific addressing.
	     */
	case SIOP_SCRIPTS_CACHE:
	    i = SIOP_TAG_CACHE_LINE(sjp->sj_ws->targid,sjp->sj_ws->tag);
	    i += (U32)shp->sh_ctlr->addr2;
	    i |= SCRIPTRAMBASE;
	    return (U32)i;

	default:
	    SIM_PRINTD(ctlr->slot,NULL,NULL,(CAMD_ERRORS),
		("siop_kn430_script_cntl: bad cmd = %x, value = %x\n",
		 cmd,value));
	    CAM_ERROR(module,"bad cmd", 
		  SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		  shp->sh_softc,NULL,NULL);
	    break;
    }
    return 0;
}


/*
 * siopcointr()
 *
 *	This is the machine dependent part of the interrupt routine.
 *	This routine needs to know how to figure out which SIOP is
 *	interrupting and to call the machine independent interrupt
 *	routine (siop_hard()) with the controller's SCRIPTHOST structure.
 *
 *      Main SIOP interrupt entry point.  Look at the lint to figure out
 *      which SIOP's are interrupting and process each one in order.
 *      Continue doing this until there are no more interrupts to process.
 */

int
siopcointr()
{
    register SCRIPT_HOST *shp;
    register U64 io_lint;
    register U64 vecmask;
    register int slot;
    register U64 oldpri;
    extern struct io_csr *Io_regs;
    SIM_MODULE(siopintr);

    WBFLUSH();                          /* mb before reading the lint */
    while(io_lint = (Io_regs->io_lint & 0x1f))
    {
        Io_regs->io_lint = io_lint;
        WBFLUSH();                      /* mb after writing the lint */

        for(slot = 0; slot < KN430_SIOP_PER_LBUS; slot++)
        {
            vecmask = 1<<slot;

            /* skip if this controller not interrupting */
            if(!(io_lint & vecmask))
                continue;

            /* Make sure this controller is initialized */
            shp = Siop_sh[slot];
            if(!shp || !(shp->sh_flags & SIOP_SH_ALIVE))
            {
                CAM_ERROR(module,
                    "siopintr: interrupt for non-initialized controller",
                    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,NULL,NULL,NULL)
;
                continue;
            }

            /* The real work is done in this subroutine which looks as the
             * SIOP registers and determines what to do.
             */
            (void)siop_hardintr(shp);

        }
        WBFLUSH();              /* mb before reading the lint */
    }

    return NULL;
}
/**************************************************************************
 *                      MAILBOX INTERFACE ROUTINES                        *
 **************************************************************************/

/* siop_kn430_csr_write()
 *
 *      args:   shp                     pointer to SCRIPT_HOST for SIOP
 *              mask			spcify which bytes to write
 *              addr                    destination address
 *              value                   value to write
 *
 *      Handle all mbox writes of CSR's through a common point to localize
 *      error handling code.  If a write fails then the controller
 *      should be taken off line.
 */

void
siop_kn430_csr_write(shp,mask,addr,value)
register SCRIPT_HOST *shp;
U64 addr, mask, value;
{
    register U64 oldpri;
    register U64 type;

    switch(mask)
    {
	case CSR_BYTE0:
	    type = BYTE0;
	    break;

	case CSR_BYTE1:
	    type = BYTE1;
	    break;

	case CSR_BYTE2:
	    type = BYTE2;
	    break;

	case CSR_BYTE3:
	    type = BYTE3;
	    break;

	case CSR_WORD0:
	    type = WORD0;
	    break;

	case CSR_WORD1:
	    type = WORD1;
	    break;

	default:
	case CSR_LONG:
	    type = LONG0;
	    break;
    }

    addr &= ~3L;        /* make sure address is longword aligned */
    addr += (U64)shp->sh_ctlr->physaddr;
    oldpri = splbio();
    WRTCSR(type,shp->sh_ctlr,addr,value);
    splx(oldpri);
}


/* siop_kn430_csr_read()
 *
 *      args:   shp                     pointer to SCRIPT_HOST for SIOP
 *              cmd                     mbox commnad
 *              mask                    mbox data type
 *              addr                    destination address
 *
 *      Handle all mbox reads of CSR's through a common point to localize
 *      error handling code.  If a read fails then the controller
 *      should be taken off line.
 */

U32
siop_kn430_csr_read(shp,mask,addr)
register SCRIPT_HOST *shp;
register U64 addr, mask;
{
    register U32 retval;
    register U64 oldpri;
    register U64 type;

    switch(mask)
    {
	case CSR_BYTE0:
	    type = BYTE0;
	    break;

	case CSR_BYTE1:
	    type = BYTE1;
	    break;

	case CSR_BYTE2:
	    type = BYTE2;
	    break;

	case CSR_BYTE3:
	    type = BYTE3;
	    break;

	case CSR_WORD0:
	    type = WORD0;
	    break;

	case CSR_WORD1:
	    type = WORD1;
	    break;

	default:
	case CSR_LONG:
	    type = LONG0;
	    break;
    }

    addr &= ~3L;        /* make sure address is longword aligned */
    addr += (U64)shp->sh_ctlr->physaddr;
    oldpri = splbio();
    retval = (U32)RDCSR(type,shp->sh_ctlr,addr);
    splx(oldpri);
    return retval;
}
