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
static char *rcsid = "@(#)$RCSfile: siop.c,v $ $Revision: 1.1.20.2 $ (DEC) $Date: 1993/10/07 20:16:34 $";
#endif
/* 		COBRA NCR53C710 SCSI DEVICE DRIVER		*/

#include <sys/types.h>
#include <io/common/devdriver.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <kern/lock.h>
#include <sys/lock.h>
#include <dec/binlog/errlog.h>  /* UERF errlog defines */
#include <arch/alpha/machparam.h>
#include <vm/vm_kern.h>

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

/* #define SIOP_VERBOSE */
/* #define SIOP_PTR_DEBUG */

/* The following macro overrides SIM_PRINTD since compiling with CAMDEBUG
 * defined produces a huge kernel.  If you want diagnostic printouts just
 * un-comment out the following macro.
#define SIM_PRINTD(B,T,L,F,X) \
	if(F & (CAMD_ERRORS|CAMD_CONFIG)) \
	{ \
	    printf("%s: Bus %d, target %d, lun %d: ",module,B,T,L); \
	    printf X ; \
	}
 */

/*
 * Time definitions used during polled mode (boot/shutdown).
 */
#define SIOP_RESPONSE_TIME	2000	/* During boot, in 0.001 secs */
#define SIOP_ONE_SECOND_DELAY	1000000	/* in clock ticks */

/*
 * SMP Lock Macros:
 */
#define SIOP_LOCK_INIT(lock) simple_lock_init ( &lock )

#define SIOP_LOCK(lock, s) {			 			\
	s = splsiop();							\
	simple_lock ( &lock );						\
}

#define SIOP_UNLOCK(lock, s) { 						\
	simple_unlock ( &lock );					\
	splx(s);							\
}

/* Macro to conditionally set the error field in the siop_job structure */
#define SIOP_SET_SJ_ERR(S,E)	if(!(S)->sj_err) (S)->sj_err = E

#define SIOP_JOB_ALLOC	32	/* allocate 32 jobs when freelist is empty */
#define SIOP_BUF_SIZE	16	/* buffer size if 16 bytes */

/* Number of buffers to allocate when the free list is empty.  Allocate two
 * per job (each job will use two buffers).
 */
#define SIOP_BUF_ALLOC	(SIOP_JOB_ALLOC*2)

#define NORMAL		1
#define ABNORMAL	0

/* external declarations */
extern struct io_csr *Io_regs;
extern void     sc_setup_ws();
extern void     sc_sel_timeout();
extern void     scsiisr();
extern U32      dme_attach();
extern void     sim_err_sm();
extern SIM_WS   *sc_find_ws();
extern SIM_SOFTC *softc_directory[];
extern CAM_SIM_ENTRY dec_sim_entry;

extern int shutting_down;		/* non-zero when system is halting */


/* forward references */
int siop_probe();
int siop_attach();
void siop_init();
void siop_reset_scsi();
SIOP_JOB *siop_alloc_job();
void siop_free_job();
U8 *siop_alloc_buf();
void siop_free_buf();
void siop_qdel();
SIOP_JOB *siop_qfirst();
void siop_dma_window();
void siop_printop();
void siop_start_job();
void siop_wait_reset();
void siop_flush();
void siop_jobdone();
void siop_jobcomplete();
void siop_rpc();
void siop_bdr();
void siop_dumpregs();
U32 siop_cam_init();
U32 siop_go();
U32 siop_sm();
U32 siop_bus_reset();
U32 siop_send_msg();
U32 siop_xfer_info();
U32 siop_sel_stop();
U32 siop_req_msgout();
U32 siop_clr_atn();
U32 siop_msg_accept();
U32 siop_setup_sync();
U32 siop_discard_data();
U32 siop_targ_cmd_cmplt();
U32 siop_targ_recv_cmd();
U32 siop_targ_send_msg();
U32 siop_targ_disconnect();
U32 siop_targ_recv_msg();
U32 siop_chip_reset();
U32 siop_unload();
void siop_logger();

static void (*local_errlog)() = siop_logger;

/* macros for access of HBA specific fields of the SIM_WS */
#define SWS_NORMJOB(sws)	(sws->hba_data[0])
#define SWS_PRIJOB(sws)		(sws->hba_data[1])

SIOP_Q Siop_doneq;		/* queue for all completed requests */

/* flag set to 1 when the SIOP thread is running */
int siop_thread_started = 0;

/* put global locking structures here since they will be referenced in the
 * probe routine.
 */
static simple_lock_data_t   free_job_lock;
static simple_lock_data_t   free_buf_lock;
static lock_data_t	    siop_jalloc_lock;
static lock_data_t	    siop_balloc_lock;

/**************************************************************************
 *		DRIVER INITIALIZATION ROUTINES				  *
 **************************************************************************/

/* Probe routine:	siop_probe()
 *
 *	arguments:	shp	pointer to controller SCRIPTHOST structure
 *
 *	returns:	0	if controller's not present
 *			1	if controllers present.
 *
 *	Machine independent part of the probe sequence.  Called by the
 *	machine autoconfig routines at boot time.
 *	Allocate the SCRIPTHOST structure and do the CAM calls to initialize
 *	the controller.  CAM will call the machine dependent portion through
 *	the CPU type switch in sim_config.c (which is where the references
 *	to the machine specific attach routine should be placed).
 *
 *	To add a machine specific module you need to write a new module
 *	(in it's own subdirectory) with it's own SCRIPTS.  Then several
 *	CAM files and the configuration files need to be changed.  The
 *	name of the machine dependent driver should reflect the type of
 *	the machine (siopco is used for Cobra).  Make entries in cam_config.c
 *	and sim_config.c for the new machine dependent module.  No new
 *	device structure has to be made since the siop device structure
 *	which references the machine indepent module is used for all
 *	variants of the SIOP.  The main machine dependent entry point
 *	is the attach routine called through the CPU switch in sim_config.c.
 *	This is where the decision as to which attach routine gets
 *	called is made.  Next, new entries in the configuration files
 *	need to be made.  The "siop" entries need to be made to reference
 *	the machine dependent interrupt routine and any machine dependent
 *	vector/slot information.  Finally, an entry in conf/alpha/files
 *	must be made for the new .c file (it must be dependent on siop
 *	and the CPU type).  Of course, the makefiles in the CAM directory
 *	tree must be modified accordingly.
 *
 *	The driver used the following members of the controller structure:
 *		slot - SIOP number (number of chip on each bus)
 *		ctlr_num - absolute SIOP number
 *		addr - value passed as first argument to probe routine.
 *			On Cobra this is a pointer the rpb_device
 *			structure passed in by the console.
 *		addr2 - the base SCRIPTS address is SCRIPTS RAM
 *		physaddr - the SIOP's base CSR address as seen from the host.
 *			On Cobra this is the address used by the Mailbox.
 *		physaddr2 - the base address of teh memory allocaed for the
 *			SCRIPT_HOST structure.
 */

int
siop_probe(vbaddr,ctlr)
caddr_t		vbaddr;
struct controller *ctlr;
{
    register SCRIPT_HOST *shp;
    register SIM_SOFTC *scp;
    register U64 addr;
    register int target, lun;
    extern SIM_SOFTC sim_softc[], *softc_directory[];
    extern I32 scsi_bus_reset_at_boot;
    SIM_MODULE(siop_probe);
    extern char cam_ctlr_string[];
    static int been_called;

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_CONFIG),
	    ("siop_probe slot %d, pathid = %d\n",ctlr->slot,ctlr->ctlr_num));
    /* initialize global locks on first call */
    if(!been_called)
    {
	been_called++;
	usimple_lock_init(&free_job_lock);
	usimple_lock_init(&free_buf_lock);
	usimple_lock_init(&doneq);
	lock_init(&siop_jalloc_lock, 1);
	lock_init(&siop_balloc_lock, 1);
	SIOP_LOCK_INIT(Siop_doneq.siop_lock);
    }

    /* The SCRIPTS accessable portions of the scripthost structure
     * MUST be aligned on a 256 byte boundry
     * for the SCRIPTS to be able to read them.
     * Enough extra memory is allocated to permit rounding the address
     * up to the next 256 byte boundry.
     */
    if((addr = 
	(U64)kmem_alloc(kernel_map,sizeof(SCRIPT_HOST)+SIOP_SH_ALIGN)) == NULL)
        return 0;

    ctlr->physaddr2 = (caddr_t)addr;

    ctlr->addr = vbaddr;

    /* round the SCRIPT_HOST to a 256 byte boundry */
    addr = (addr + (SIOP_SH_ALIGN-1)) & ~(SIOP_SH_ALIGN-1);
    shp = (SCRIPT_HOST *)addr;
    bzero(shp,sizeof(SCRIPT_HOST));

    /* initialize the queue locks */
    for(target = 0; target < NDPS; target++)
	for(lun = 0; lun < NLPT; lun++)
	    usimple_lock_init(&shp->sh_activeq[target][lun].siopq_lock);
    usimple_lock_init(&shp->sh_lock);
    shp->sh_version = SIOP_SH_VERSION;

    shp->sh_ctlr = ctlr;

    /* First check to make sure this controller hasn't already been
     * attached.  If it has, return.  If another has been attached
     * in this softc slot, return 0.
     */
    if(softc_directory[ctlr->ctlr_num] != NULL)
    {
	CAM_ERROR(module, "cntlr already probed", SIM_LOG_SIM_SOFTC,
	      softc_directory[ctlr->ctlr_num], NULL, NULL);
	  if(ctlr->physaddr != softc_directory[ctlr->ctlr_num]->csr_probe)
	  {
	       CAM_ERROR(module, "cntlr already probed diff CSR",
		    SIM_LOG_SIM_SOFTC, softc_directory[ctlr->ctlr_num],
		     NULL, NULL);
		return 0;
	  }
	  return 1;
    }

    /* setup the SIM_SOFTC structure */
    scp = (SIM_SOFTC *)sc_alloc(sizeof(SIM_SOFTC));
    softc_directory[ctlr->ctlr_num] = scp;
    shp->sh_softc = (void *)scp;
    scp->hba_sc = (void *)shp;
    scp->csr_probe = vbaddr;
    scp->um_probe = (void *)ctlr;
    scp->reg = (void *)shp;
    scp->cntlr = ctlr->ctlr_num;
    scp->dme = 0;		/* no DME for this driver */

    if(ccfg_simattach(&dec_sim_entry, ctlr->ctlr_num) == CAM_FAILURE)
    {
        kmem_free((kernel_map,ctlr->physaddr2,
		  sizeof(SCRIPT_HOST)+SIOP_SH_ALIGN));
	sc_free(scp,sizeof(SIM_SOFTC));
	return 0;
    }

    ctlr->private[0] = cam_ctlr_string;

    return 1;
}

/* attach routine:	siopattach()
 *
 *	arguments:	ctlr		controller structure
 *
 *
 *	Set up the HBA functions in the SIM_SOFTC structure and perform
 *	any other machine independent initialization.
 */

int
siop_attach(scp)
register SIM_SOFTC *scp;
{
    SIM_MODULE(siop_attach);

    /* now set up all the stuff to hook the driver into CAM */

    scp->hba_init = siop_cam_init;
    scp->hba_go = siop_go;
    scp->hba_sm = siop_sm;
    scp->hba_bus_reset = siop_bus_reset;
    scp->hba_send_msg = siop_send_msg;
    scp->hba_xfer_info = siop_xfer_info;
    scp->hba_sel_msgout = siop_sel_stop;
    scp->hba_msgout_pend = siop_req_msgout;
    scp->hba_msgout_clear = siop_clr_atn;
    scp->hba_msg_accept = siop_msg_accept;
    scp->hba_setup_sync = siop_setup_sync;
    scp->hba_discard_data = siop_discard_data;

    scp->hba_targ_cmd_cmplt = siop_targ_cmd_cmplt;
    scp->hba_targ_recv_cmd = siop_targ_recv_cmd;
    scp->hba_targ_send_msg = siop_targ_send_msg;
    scp->hba_targ_disconnect = siop_targ_disconnect;
    scp->hba_targ_recv_msg = siop_targ_recv_msg;

    return 1;
}

/* 
 *	siop_init()
 *
 *	arguments:	shp		pointer to script host struct
 *			resetflag	reset bus if set
 *
 *	Reset the SIOP, reset the bus, and initialize the SIOP.
 *	Called when the SIOP must be stopped and reloaded.
 *	The SIOP registers are set up for SCRIPTS execution.
 *	Not all interrupts are enabled here since they can't be
 *	handled until initilization is completed.
 *
 *	This is considered machine independent since the CSR
 *	bit setting should be fairly standard, but the way in
 *	which the CSR's are written may vary.  However, watch out
 *	for machine specific constants for things like clock rates
 *	(which really shouldn't vary due to SCSI BUS timing requirements).
 *
 */
void
siop_init(shp,resetflag)
register SCRIPT_HOST *shp;
U32 resetflag;
{
    register struct controller *ctlr = shp->sh_ctlr;
    register U32 i;
    SIM_MODULE(siop_init);

    /* reset the controller.  Spin for a while while it resets */
    i = ISTAT_RST;
    i <<= 8;
    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_ISTAT,i);

    /* even though the mailbox stuff should provide enough delay for
     * the chip to reset, wait about 10 usec before clearing the reset
     * bit.
     */
    DELAY(10);

    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_ISTAT,0);

    /* check the chip revision number.  Rev 0 chips are bogus and will
     * probably hang. Also, rev 0 chips pass through bad parity bits
     * causing a localbus parity error and a fatal machine check.
     */
    i = SIOP_MD_CSR_READ(shp,CSR_BYTE2,SIOP_REG_CTEST8);
    /* mask off CTEST8 which is read in bits 16-23 */
    i >>= 20;
    i &= 0xf;
    if(i == 0)			/* REV 0 -- BAD!!! */
    {
	printf("\nsiop_init:\t**** NCR53C710 REV 0 NOT SUPPORTED ****\n");
	CAM_ERROR(module,"siop_hardintr: rev 0 chip not supported",
	    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
    }

    i = DCNTL_EA|DCNTL_COM;
    i <<= 8;
    i |= SIOP_WATCHDOG;
    i <<= 8;
    i |= DIEN_BF|DIEN_ABRT|DIEN_WTD|DIEN_IID|DIEN_SSI;
    i <<= 8;
    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_DMODE,i);

    i = SXFER_DHP;
    i <<= 8;
    i |= 1 << shp->sh_softc->scsiid;
    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SCID,i);


    /* reset the SCSI bus before interrupts are enabled */
    if(resetflag)
	siop_reset_scsi(shp);

    /* clear out the reset interrupt bit before enabling them */
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_DSTAT);

    /* configure the SIOP */
    i = SIEN_STO|SIEN_SGE|SIEN_UDC|SIEN_RST|SIEN_MA|SIEN_PAR;
    i <<= 16;
    i |= SCNTL1_ESR;
    i <<= 8;
    i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

    /* now set up the synchronous transfer clock frequency */
    if(shp->sh_flags & SIOP_SH_FAST)
	i = SBCL_SSCF0;
    else
	i = SBCL_SSCF1|SBCL_SSCF0;
    i <<= 24;
    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SFBR,i);
}


/*
 * siop_reset_scsi()
 *
 *	arguments:	shp	pointer to controller's scripthost struct 
 *
 *
 *	Performs a SCSI bus reset.  Since the SIOP requires the host
 *	to make sure that the reset is asserted for 25 usec, this
 *	routine will loop the requesit number of times. This will be
 *	done with interrupts masked to se don't take the RESET 
 *	interrupt until we're ready.
 *
 *	This routine MUST be called with SCRIPTS execution suspended.
 */

void
siop_reset_scsi(shp)
register SCRIPT_HOST *shp;
{
    register U64 i;
    register struct controller *ctlr = shp->sh_ctlr;
    register U64 oldpri;
    SIM_MODULE(siop_reset_scsi);

    SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_INOUT|CAMD_FLOW|CAMD_ERRORS),
    	("siop_reset_scsi: resetting SCSI bus\n"));

    /* disable reset interrupts */
    i = SIEN_STO|SIEN_SGE|SIEN_UDC|SIEN_MA|SIEN_PAR;
    i <<= 16;
    i |=  SCNTL1_ESR;
    i <<= 8;
    i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

    oldpri = splsiop();

    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

    /* hit the SCSI RESET bit */
    i = SIEN_STO|SIEN_SGE|SIEN_UDC|SIEN_MA|SIEN_PAR;
    i <<= 16;
    i |=  SCNTL1_ESR|SCNTL1_RST;
    i <<= 8;
    i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

    /* synchronize the mailboxes so the reset delay timing starts after
     * the CSR has really been written.
     */
    SIOP_MD_CSR_READ(shp,CSR_BYTE1,SIOP_REG_ISTAT);

    /* wait for at least 25 usec */
    DELAY(30);

    /* turn off reset */
    i = SIEN_STO|SIEN_SGE|SIEN_UDC|SIEN_MA|SIEN_PAR;
    i <<= 16;
    i |= SCNTL1_ESR;
    i <<= 8;
    i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

    /* flush the FIFO's */
    SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,CTEST8_CLF<<16);

    i = SIEN_STO|SIEN_SGE|SIEN_UDC|SIEN_RST|SIEN_MA|SIEN_PAR;
    i <<= 16;
    i |= SCNTL1_ESR;
    i <<= 8;
    i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

    SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

    splx(oldpri);
}

/***************************************************************************
 *		MEMORY ALLOCATION ROUTINES				   *
 ***************************************************************************/

/* All memory used by the driver and the SCRIPTS are allocated through
 * these routines so that physical continuity is assured.  Every time
 * the free list is exhausted more memory is allocated.  No memory is
 * ever freed.  The assumtion here is that if we reach some peak
 * usage once, we're likely to again.
 */

static SIOP_JOB *free_job = NULL;	/* LIFO Free list head */

/* Each buffer is really just 16 bytes for data (commands, messages).
 * However, when on the free list, there has to be a pointer.
 */
union scsi_buf {
	union scsi_buf *next;
	U8	data[SIOP_BUF_SIZE];
};

static union scsi_buf *free_buf = NULL;	/* LIFO free list head */

/*
 * siop_alloc_job()
 *
 *	return:		allocated siopjob struct or 0 if alloc fails
 *
 * Allocate an siopjob struct from the free list.  If the list is empty
 * then get some more.
 *
 * WARNING: these structures must be allocated on a quadword boundry, the
 * SCRIPTS depend on this.
 */

SIOP_JOB *
siop_alloc_job()
{
    register SIOP_JOB *jp, *jph, *jpt;
    register U32 i;
    register U64 oldpri;
    SIM_MODULE(siop_alloc_job);

again:
    oldpri = splsiop();

    usimple_lock(&free_job_lock);

    jp = free_job;
    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("free_job = 0x%lx\n", jp ) );
    if(jp == NULL)
    {
	simple_unlock(&free_job_lock);
	splx(oldpri);
	/* Prevent multiple threads from trying to alloc more memory when they
	 * both detect that the free list is empty.  First, try to get the
	 * lock.  If this is not successusfull, then another thread is 
	 * currently doing allocation.  In this case, just wait until the
	 * lock is released and then try again to get a buffer.  If the
	 * lock was not previously held then go ahead with the allocation.
	 *
	 * The point of doing things this way rather than using the lock
	 * routines to lock the whole allocation procedure is that for
	 * the majority of cases all that will have to be done for alloc/free
	 * is the manipulation of the free list head pointer (just a few
	 * instructions) which can be more effeciently protected with spin
	 * locks.  Only the long memory allocation need be protected
	 * by full read/write locks (which will also allow other
	 * threads to free jobs since the spin lock is not held during
	 * allocation).
	 */
	if(lock_try_write(&siop_jalloc_lock) == FALSE)
	{
	    lock_write(&siop_jalloc_lock);
	    if(free_job)
	    {
		lock_done(&siop_jalloc_lock);
		goto again;
	    }
	}

    	jp = (SIOP_JOB *)kmem_alloc(kernel_map,
		sizeof(SIOP_JOB)*SIOP_JOB_ALLOC+8);

	if(jp == NULL)
	{
	    lock_done(&siop_jalloc_lock);
	    return NULL;
	}


	/* make sure address is at a quad word boundry */
	jp = (SIOP_JOB *)(((U64)jp + 7) & ~7);
	jph = jpt = NULL;

	for(i = 0; i < SIOP_JOB_ALLOC; i++)
	{
	    /* use only if doesn't cross a page boundry */
	    if(((U64)jp & ~PGOFSET) == 
		(((U64)jp + sizeof(SIOP_JOB)) & ~PGOFSET))
	    {
		if(jpt == NULL)
		    jpt = jp;
		jp->sj_next = jph;
		jph = jp;

	    }
	    else 
	    {
		/* if it spans a page, try to use as much of it as
		 * possible as data buffers.
		 */
		register union scsi_buf *bp;
		
		for(bp = (union scsi_buf *)jp; (U8 *)(bp+1) < (U8 *)(jp+1); bp++)
		    if(((U64)bp & PGOFSET) < 
			    (NBPG - sizeof(union scsi_buf)))
			siop_free_buf(bp);

	    }
	    jp++;
	}
	oldpri = splsiop();
	usimple_lock(&free_job_lock);
	jpt->sj_next = free_job;
	free_job = jph;
	jp = jph;
	/* don't unlock until the free list is no longer empty */
	lock_done(&siop_jalloc_lock);
    }
    free_job = jp->sj_next;

    simple_unlock(&free_job_lock);
    splx(oldpri);

    bzero(jp,sizeof(SIOP_JOB));
    jp->sj_next = jp->sj_prev = NULL;
    jp->sj_myq = NULL;
    jp->sj_shp = NULL;
    jp->sj_me = jp;
    jp->sj_msgobuf = jp->sj_cmdbuf = NULL;
    jp->sj_ws = NULL;
    jp->sj_version = SIOP_SJ_VERSION;

    return jp;
}

/*
 * siop_free_job()
 *
 *	argument:	jp		pointer to job to free
 *
 *	Place a finished job structure back on the free list.
 *	Free any data buffers still associated with the job.
 */

void
siop_free_job(sjp)
register SIOP_JOB *sjp;
{
    register U64 oldpri;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_free_job: sjp = 0x%lx\n", sjp ) );
    /* free any data buffers associated with the job */
    if(sjp->sj_msgobuf)
	siop_free_buf(sjp->sj_msgobuf);
    if(sjp->sj_cmdbuf)
	siop_free_buf(sjp->sj_cmdbuf);

    oldpri = splsiop();
    usimple_lock(&free_job_lock);

    sjp->sj_next = free_job;
    free_job = sjp;

    simple_unlock(&free_job_lock);
    splx(oldpri);
}



/* siop_buf_alloc()
 *
 *	returns:	pointer to a buffer for use by the SCRIPTS
 *
 *	Allocate a character buffer for commands, messages, etc.
 *	This each is the largest size needed, rounded to a nice power
 *	of two.  The buffers must be correctly aligned and must not
 *	span a physical page.
 */

U8 *
siop_alloc_buf()
{
    register union scsi_buf *sbp;
    register U64 oldpri;
    register int i;
    SIM_MODULE(siop_alloc_buf);

again:
    oldpri = splsiop();
    usimple_lock(&free_buf_lock);

    sbp = free_buf;
    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("free_buf = 0x%lx\n", sbp ) );
    if(sbp == NULL)
    {
	/* unlock while allocating more memory in case this takes a while */
	simple_unlock(&free_buf_lock);
	splx(oldpri);

	if(lock_try_write(&siop_balloc_lock) == FALSE)
	{
	    lock_write(&siop_balloc_lock);
	    if(free_buf)
	    {
		lock_done(&siop_balloc_lock);
		goto again;
	    }
	}

	sbp = (union scsi_buf *)kmem_alloc(kernel_map,
		sizeof(union scsi_buf) * SIOP_BUF_ALLOC+8);
	if(sbp == NULL)
	{
	    lock_done(&siop_balloc_lock);
	    return NULL;
	}

	oldpri = splsiop();
	usimple_lock(&free_buf_lock);

	/* make sure address is at a quad word boundry */
	sbp = (union scsi_buf *)(((U64)sbp + 7) & ~7);

	for(i = 0; i < SIOP_BUF_ALLOC; i++)
	{
	    if(((U64)sbp & PGOFSET) < 
		(NBPG - sizeof(union scsi_buf)))
	    {
		sbp->next = free_buf;
		free_buf = sbp;
	    }
	    sbp++;
	}
	sbp = free_buf;
	lock_done(&siop_balloc_lock);
    }
    free_buf = sbp->next;
    simple_unlock(&free_buf_lock);
    splx(oldpri);
    bzero(sbp,sizeof(union scsi_buf));
    return (U8 *)sbp->data;
}

/* 
 * siop_free_buf()
 *
 *	args:	sbp		pointer to buffer to free
 *
 *	Put the buffer on the free list.
 */

void
siop_free_buf(sbp)
register union scsi_buf *sbp;
{
    register U64 oldpri;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_free_buf: sbp = 0x%lx\n", sbp ) );
    oldpri = splsiop();
    usimple_lock(&free_buf_lock);

    sbp->next = free_buf;
    free_buf = sbp;

    simple_unlock(&free_buf_lock);
    splx(oldpri);
}


/*************************************************************************
 *			QUEUE MANAGEMENT ROUTINES			 *
 *************************************************************************/

/* siop_qadd()
 *
 *	args:	sjp		pointer to siopjob struct to add
 *		qp		pointer to queue on which to add
 *
 *	returns:	non-zero if queue was previously non-empty
 *
 *	link the given job onto the end of the given queue.
 *	Return the previous tail pointer which will be 0 if the queue
 * 	was empty.  This allows callers to determine if previous
 *	state of the queue. Note that the return value should only
 *	be treated as a hint by the caller since many race conditions
 *	exist, especially on MP systems.
 *
 */

long
siop_qadd(sjp,qp)
register SIOP_JOB *sjp;
register SIOP_Q *qp;
{
    register U64 oldpri;
    register long retval;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_qadd: sjp = 0x%lx, qp = 0x%lx\n", sjp, qp ) );
    oldpri = splsiop();
    usimple_lock(qp);

    sjp->sj_next = NULL;
    sjp->sj_prev = NULL;
    sjp->sj_myq = qp;
    if((retval = (long)qp->siopq_tail) != NULL)
    {
	qp->siopq_tail->sj_next = sjp;
	sjp->sj_prev = qp->siopq_tail;
    }
    else
	qp->siopq_head = sjp;
    qp->siopq_tail = sjp;
    simple_unlock(qp);
    splx(oldpri);
    return retval;
}

/* siop_qhead()
 *
 *	args:	sjp		pointer to siopjob to insert
 *		qp		queue on which to insert it.
 *
 *	Place the given job at the head of a queue.
 *	Return the previous tail pointer which will be 0 if the queue
 * 	was empty.  This allows callers to determine if previous
 *	state of the queue. Note that the return value should only
 *	be treated as a hint by the caller since many race conditions
 *	exist, especially on MP systems.
 */

long
siop_qhead(sjp,qp)
register SIOP_JOB *sjp;
register SIOP_Q *qp;
{
    register U64 oldpri;
    register long retval;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_qhead: sjp = 0x%lx, qp = 0x%lx\n", sjp, qp ) );
    oldpri = splsiop();
    usimple_lock(qp);

    sjp->sj_next = sjp->sj_prev = NULL;
    sjp->sj_myq = qp;
    if((retval = (long)qp->siopq_head) != NULL)
    {
	qp->siopq_head->sj_prev = sjp;
	sjp->sj_next = qp->siopq_head;
	qp->siopq_head = sjp;
    }
    else
	qp->siopq_head = qp->siopq_tail = sjp;
    usimple_unlock(qp);
    splx(oldpri);
    return retval;
}

/* siop_qins()
 *
 *	args:	sjp	pointer to SIOP_JOB to insert
 *		qp	queue on which to insert it.
 *
 *	Insert the given job onto the given queue in order of the tag
 *	field.  This inserts jobs onto queues in increasing order of
 *	tag.  This permits quicker tag lookup.
 */

int
siop_qins(sjp,qp)
register SIOP_JOB *sjp;
register SIOP_Q *qp;
{
    register U64 oldpri;
    register U64 retval;
    register SIOP_JOB *sjp1;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_qins: sjp = 0x%lx, qp = 0x%lx\n", sjp, qp ) );
    oldpri = splsiop();
    usimple_lock(qp);

    sjp->sj_next = sjp->sj_prev = NULL;
    sjp->sj_myq = qp;
    if((retval = (U64)qp->siopq_head) != NULL)
    {
	for(sjp1 = qp->siopq_head; sjp1; sjp1 = sjp1->sj_next)
	    if(sjp->sj_tag < sjp1->sj_tag)
		break;
	if(sjp1 != NULL)
	{
	    if(sjp1->sj_prev != NULL)
	    {
		sjp1->sj_prev->sj_next = sjp;
		sjp->sj_prev = sjp1->sj_prev;
		sjp1->sj_prev = sjp;
		sjp->sj_next = sjp1;
	    }
	    else
	    {
		sjp1->sj_prev = sjp;
		sjp->sj_next = qp->siopq_head;
		qp->siopq_head = sjp;
	    }
	}
	else
	{
	    qp->siopq_tail->sj_next = sjp;
	    sjp->sj_prev = qp->siopq_tail;
	    qp->siopq_tail = sjp;
	}
    }
    else
	qp->siopq_head = qp->siopq_tail = sjp;
    usimple_unlock(qp);
    splx(oldpri);
    return retval;
}

/* siop_qdel()
 *
 *	args:	sjp		pointer to siopjob to remove from the queue.
 *
 *	Remove the given job from the queue.
 */

void
siop_qdel(sjp)
register SIOP_JOB *sjp;
{
    register SIOP_Q *qp;
    register U64 oldpri;

    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_qadd: sjp = 0x%lx\n", sjp ) );
    qp = sjp->sj_myq;

    if(qp != NULL)
    {
	oldpri = splsiop();
	usimple_lock(qp);

	if(sjp->sj_prev != NULL)
	    sjp->sj_prev->sj_next = sjp->sj_next;
	else
	    qp->siopq_head = sjp->sj_next;
	if(sjp->sj_next != NULL)
	    sjp->sj_next->sj_prev = sjp->sj_prev;
	else
	    qp->siopq_tail = sjp->sj_prev;

	usimple_unlock(qp);
	splx(oldpri);
    }

    sjp->sj_next = sjp->sj_prev = NULL;
    sjp->sj_myq = NULL;
}

/*
 * siop_qfirst()
 *
 *	args:	qp		pointer to queue
 *
 *
 *	returns:	pointer to siopjob removed or 0
 */

SIOP_JOB *
siop_qfirst(qp)
register SIOP_Q *qp;
{
    register SIOP_JOB *sjp;
    register U64 oldpri;
    
    if(qp == NULL)
	return NULL;
    oldpri = splsiop();
    usimple_lock(qp);

    if(sjp = qp->siopq_head)
    {
	if((qp->siopq_head = sjp->sj_next) == NULL)
	    qp->siopq_tail = NULL;
	else
	    qp->siopq_head->sj_prev = NULL;
	sjp->sj_next = sjp->sj_prev = NULL;
	sjp->sj_myq = NULL;
    }

    usimple_unlock(qp);
    splx(oldpri);

    return sjp;
}

/***************************************************************************
 *		MEMORY MAPPING ROUTINES (virtual->physical)		   *
 ***************************************************************************/

/*
 * siop_map_data()
 *
 *	args:	addr		address of first byte to map
 *		length		maximum number of bytes to map
 *		dataip		pointer to scatter/gather entry to use
 *
 *	returns:	total number of bytes mapped
 *
 *	Map data to SIOP scatter/gather list.  Each call maps as
 *	much physically contiguous memory as possible.  Each
 *	call maps only ONE map entry.  Mapping is done one page at
 *	a time.
 *
 *	If an address can't be mapped then return 0 bytes mapped.  This
 *	will leave the scatter/gather entry null which will cause the
 *	SCRIPTS to fail the request with a data overrun (probably not
 *	the best solution, but since the entire address range isn't
 *	checked before the request starts it's possible that a bad
 *	address could be reached at any time).
 */

int
siop_map_data(addr,length,dataip,bp)
U8 *addr;
register struct  datai *dataip;
struct buf *bp;
{
    register U64 size;
    U64 lastaddr, newaddr;
    U64 i;
    SIM_MODULE(siop_map_data);

    dataip->di_count = 0;
    dataip->di_datap = 0;
    dataip->di_mbz = 0;

    if(CAM_IS_KUSEG(addr))
	lastaddr = pmap_extract(bp->b_proc->task->map->vm_pmap, addr);
    else if(pmap_svatophys(addr, &lastaddr) == KERN_INVALID_ADDRESS)
	return 0;

    dataip->di_datap = (U32)lastaddr;

    /* make sure that no more than the maximum number of bytes per 
     * entry allowed by the SIOP is being requested (24 bits of byte
     * count).
     */
    if(length >= SIOP_DMA_MAX_COUNT)
	length = SIOP_DMA_MAX_COUNT;
    while(length)
    {
	/* look at only one page at a time through the loop */
	size = (NBPG)-(lastaddr & 0x1fff);
	if(size > length)
	    size = length;

	dataip->di_count += size;
	addr += size;
	length -= size;

	/* if all bytes have been processed */
	if(!length)
	    break;

	if(CAM_IS_KUSEG(addr))
	    newaddr = pmap_extract(bp->b_proc->task->map->vm_pmap, addr);
	else if(pmap_svatophys(addr, &newaddr) == KERN_INVALID_ADDRESS)
	    break;

	/* If the next pageis not physically contiguous with the last
	 * page then stop here.
	 */
	if((newaddr - lastaddr) > NBPG)
	    break;
	lastaddr = newaddr;
    }
    return dataip->di_count;
}

/* siop_dma_window()
 *
 *	args:	sjp		pointer to siopjob struct for the request
 *		offset		offset at which to start mapping
 *
 *
 *	Map a DMA window starting at offset.  As many bytes as possible
 *	are mapped.  Since the argument is an byte offset the caller
 *	does not need to know anything about the data buffer's mapping.
 *	This routine and siop_map_data() take care of all the addressing
 *	details.  This is also easier when relating SCSI transfer byte counts
 *	to some address in the data buffer.
 */

void
siop_dma_window(sjp,offset)
register SIOP_JOB *sjp;
U64 offset;
{
    register struct datai *dip;
    register U64 i, count, dips, length;
    register U8 *cp;
    register SG_ELEM *sgp;
    register CCB_SCSIIO *ccbp = sjp->sj_ws->ccb;
    SIM_MODULE(siop_dma_window);

    sjp->sj_offset = offset;
    sjp->sj_count = 0;

    /* check for end of mapping, and map 0.
     * This is not an error since the SCRIPTS could ask for more data even
     * it there is no more (it deosn't know).  The fact that there is
     * no more data available is communicated back by setting up a 0
     * mapping.  The SCRIPTS will then handle the overflow condition.
     */
    if(offset >= ccbp->cam_dxfer_len)
    {
	sjp->sj_datai[0].di_count = 0;
	sjp->sj_datai[0].di_datap = 0;
	sjp->sj_doffset[0] = offset;
    }
    else
    {
	/* remap as much as possible */
	dips = 0;
	dip = sjp->sj_datai;

	/* this has to be done differently for scatter/gather lists */
	if(ccbp->cam_ch.cam_flags & CAM_SCATTER_VALID)
	{
	    count = offset;
	    sgp = (SG_ELEM *)ccbp->cam_data_ptr;
	    length = ccbp->cam_sglist_cnt;
	    while(count && length)
	    {
		if(sgp->cam_sg_count > count)
			break;
		count -= sgp->cam_sg_count;
		length--;
		sgp++;
	    }
	    cp = sgp->cam_sg_address;
	    cp += count;
	    count = sgp->cam_sg_count - count;
	    while(dips < 32)
	    {
#ifdef SIOP_PTR_DEBUG
		i = siop_map_data(cp,count<512?count:512,dip,ccbp->cam_req_map);
#else
		i = siop_map_data(cp,count,dip,ccbp->cam_req_map);
#endif
		if(!i)
		    sjp->sj_err = CAM_REQ_INVALID;
		sjp->sj_doffset[dips] = offset;
		offset += i;
		count -= i;
		cp += i;
		sjp->sj_count += i;
		dips++;
		dip++;
		if(count == 0)
		{
		    sgp++;
		    length--;
		    if(length)
		    {
			count = sgp->cam_sg_count;
			cp = sgp->cam_sg_address;
		    }
		    else
			break;
		}
	    }
	}
	else
	{
	    cp = ccbp->cam_data_ptr + offset;
	    count = ccbp->cam_dxfer_len - offset;
	    while(count && dips < 32)
	    {
#ifdef SIOP_PTR_DEBUG
		i = siop_map_data(cp,count<512?count:512,dip,ccbp->cam_req_map);
#else
		i = siop_map_data(cp,count,dip,ccbp->cam_req_map);
#endif
		if(!i)
		    sjp->sj_err = CAM_REQ_INVALID;
		sjp->sj_doffset[dips] = offset;
		offset += i;
		count -= i;
		cp += i;
		sjp->sj_count += i;
		dips++;
		dip++;
	    }
	}
    }
    if(dips < SIOP_DMA_WINDOW_SIZE)
	sjp->sj_doffset[dips] = offset;
}


/****************************************************************************
 *			INTERRUPT TIME ROUTINES				    *
 ****************************************************************************/

/* interrupts types that are expected */
#define SIOP_DSTAT_INTS	\
	(DSTAT_BF|DSTAT_ABRT|DSTAT_SSI|DSTAT_SIR|DSTAT_WTD|DSTAT_IID)
#define SIOP_SSTAT_INTS \
	(SSTAT0_MA|SSTAT0_STO|SSTAT0_SGE|SSTAT0_UDC|SSTAT0_RST|SSTAT0_PAR)

/* siop_hardintr()
 *
 *	args:	shp		pointer to script host of interrupting SIOP
 *
 *	returns: 		1 if valid interrupt, 0 if not.
 *
 *	Figure out what sort of hardware interrupt occurred and handle
 *	it. Everything must be done here since some interrupts are related.
 */

int
siop_hardintr(shp)
register SCRIPT_HOST *shp;
{
    register struct controller *ctlr = shp->sh_ctlr;
    register SIOP_JOB *sjp;
    register U32 pc;
    register int i;
    register U8 csr;
    register int restart = 0;
    register U32 r = 0;
    U32 new_dsp;
    SIM_MODULE(siop_hardintr);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("shp = 0x%lx\n", shp ) );

    /* Eat all pending interrupts.  Since the SIOP can stack the interrupts,
     * read out all interrupt bits until none are left.  This way they
     * are handled in correct order here.
     * The ISTAT register is read in bits 8-15 and must be shifted right
     * to be masked.
     */
    while((csr = 
	   (U8)(SIOP_MD_CSR_READ(shp,CSR_BYTE1,SIOP_REG_ISTAT)>>8))
	& (ISTAT_SIP|ISTAT_DIP))
    {
	/* if the ABRT bit is set then it must be reset before reading
	 * the DSTAT register.
	 */
	if((csr & (ISTAT_ABRT|ISTAT_DIP)) == (ISTAT_ABRT|ISTAT_DIP))
	    SIOP_MD_CSR_WRITE(shp,CSR_BYTE1,SIOP_REG_ISTAT,0);
	r |= SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_DSTAT);
    }

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("SIOP_REG_DSTAT = 0x%x\n", r ) );

    /* When the status registers are read they are returned in a longword.
     * The DSTAT register bits are in bits 0-7
     * The SSTAT0 register bits are in bits 8-15
     * The SSTAT1 register bits are in bits 16-23
     * The SSTAT2 register bits are in bits 24-31
     */
    if(!(r & (SIOP_DSTAT_INTS|(SIOP_SSTAT_INTS<<8))))
    {
	return NULL;
    }

    /* FIRST DO DMA INTERRUPTS */
    csr = (U8)(r & SIOP_DSTAT_INTS);

    /* Process software interrupts.  We want to restart the SCRIPTS
     * if there are no other hard interrupt bits set.
     */
    if(csr & DSTAT_SIR)
    {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
		("sh_intstatus = 0x%x\n", (shp->sh_intstatus & 0xff) ) );

	switch(shp->sh_intstatus & 0xff)
	{
	    case GOING_IDLE:               /* about to wait for reselect */
		shp->sh_currentjob = NULL;
		siop_start_job(shp);
		restart = DCNTL_STD;
		break;

	    case JOB_DONE:               	/* a job has finished */
		siop_jobdone(shp->sh_jobdone,NORMAL);
		shp->sh_jobdone = NULL;
		shp->sh_currentjob = NULL;
		siop_start_job(shp);
		restart = DCNTL_STD;
		break;

	    case RPC_REQUEST:               /* a request for help */
		shp->sh_rpcreply = 0;
		siop_rpc(shp);
		restart = DCNTL_STD;
		break;

	    default:
		CAM_ERROR(module,"siop_hardintr: unknown intr status",
		    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
		break;
	}
    }

    /* Panic on bus faults since this indicates a serious problem with the
     * LBUS.
     */
    if(csr & DSTAT_BF)
    {
	printf("SIOP %d:",shp->sh_ctlr->slot);
	panic("LOCAL BUS FAULT\n");
    }

    /* single step interrupts are used only to debug the SCRIPTS */
    if(csr & DSTAT_SSI)
    {
	restart = DCNTL_SSM|DCNTL_STD;
#ifdef SIOP_VERBOSE
	printf("SIOP SINGLE STEP:");
#endif
	pc = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_GET_DSP,0);
	siop_printop(shp,pc);
	restart = DCNTL_STD|DCNTL_SSM;
    }

    /* If the watchdog goes off this indicates some serious problems
     * with the LBUS, so panic.
     */
    if(csr & DSTAT_WTD)
    {
	printf("SIOP %d ",shp->sh_ctlr->slot);
	panic("LOCAL BUS TIMEOUT\n");
    }

    /* An abort happens when the host is attempting to reset the SCSI
     * bus.  The SCRIPTS must be stopped (via the ABORT interrupt),
     * the bus must be reset, and the SCRIPTS must be restarted from 
     * the beginning.
     */
    if(csr & DSTAT_ABRT)
    {
reset_bus:
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("DSTAT_ABRT detected\n") );

	shp->sh_flags &= ~SIOP_SH_ABORTED;

	/*
	 * Don't abort job here, let reset indication do it below via the
	 * siop_flush() function to avoid CCB completion race conditions.
	 * Also, since SCRIPTS has been stopped by the ABORT, the fields
	 * it initializes may not be complete (i.e., shp->sh_currentjob).
	 * (I've seen a partial current job pointer which caused a panic).
	 */
#ifdef notdef
	/* First, finish up any job currently active on the bus */
	if(sjp = shp->sh_currentjob)
	{
	    register SIM_WS *sws;
	    SIOP_SET_SJ_ERR(sjp,CAM_SCSI_BUS_RESET);
	    siop_jobdone(sjp,ABNORMAL);
	    if (sws = sjp->sj_ws)
	    {
		SWS_NORMJOB(sws) = NULL;
		SWS_PRIJOB(sws) = NULL;
		sjp->sj_ws = NULL;
	    }
	}
	shp->sh_currentjob = NULL;
	shp->sh_jobdone = NULL;
#endif
	new_dsp = 0;
	restart = 0;

	siop_reset_scsi(shp);
    }

    /* Handle illegal SCRIPTS instructions.  These should never happen.
     * If one of these occurrs, it means something VERY VERY BAD has
     * happened on the 710.  Either the 710 has wondered off into invalid
     * SCRIPTS RAM (or main RAM), or a data movement instruction has 
     * gotten bogus operands (probably due to something going wrong
     * in the driver).  In the first case all that can be done is to
     * restart the SCRIPTS.  In the second case, something could be done
     * but it might me more trouble than it's worth.  So...
     *
     * Reset the controller, reload the SCRIPTS, and then reset the bus.
     * This is necessary since the bus could have been doing something
     * when the illegal instruction was hit.
     */
    if(csr & DSTAT_IID)
    {
	register U32 op;
	register int i;
	register SIOP_Q *qp;

	shp->sh_flags &= ~SIOP_SH_ALIVE;
	printf("SIOP %d ILLEGAL INSTRUCTION:",shp->sh_ctlr->slot);
	restart = 0;
	pc = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_GET_DSP,0);
	pc -= 12;

	siop_printop(shp,pc);

	CAM_ERROR(module,"siop_hardintr: illegal instruction",
	    SIM_LOG_HBA_CSR|SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE|SIM_LOG_HBA_DME,
	    shp->sh_softc,shp->sh_currentjob,pc);

bad_instruction:
	/* complete the pending job */
	if(sjp = shp->sh_currentjob)
	{
	    SIOP_SET_SJ_ERR(sjp,CAM_REQ_INVALID);
	    siop_jobdone(sjp,ABNORMAL);
	}
	shp->sh_currentjob = NULL;
	shp->sh_jobrequest = NULL;
	shp->sh_jobdone = NULL;

	/* Reset the SCSI bus before restarting the SIOP.  This will
	 * eliminate any current request so it doesn't bother the
	 * SIOP when it's restarted.  Reset the SCSI here so that
	 * an interrupt is not generated
	 */
	i = 0;
	i |=  SCNTL1_ESR;
	i <<= 8;
	i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

	SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

	/* hit the SCSI RESET bit */
	i |=  SCNTL1_ESR|SCNTL1_RST;
	i <<= 8;
	i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

	SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);


	/* wait for at least 25 usec */
	DELAY(30);

	/* turn off reset */
	i |= SCNTL1_ESR;
	i <<= 8;
	i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;

	SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

	/* flush the FIFO's */
	SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,CTEST8_CLF<<16);

	/* complete all active jobs */
	siop_flush(shp,CAM_SCSI_BUS_RESET);

	/* Now, reset and restart the SIOP */
	siop_chip_reset(shp->sh_softc);
	return 1;
    }


    /* NOW DO SCSI INTERRUPTS */
    /* SSTAT0 is in bits 8-15 of r */
    csr = (U8)((r >> 8)&0xff);

    /* STO interrupts should only happen on selection timeouts.  The
     * SCRIPTS disables these interrupts at all other times.  Other
     * timeouts (bus timeout and disconnect timeout) are detected
     * by the SIM scheduler timeout function.
     *
     * Just return the job to the scheduler and restart the SCRIPTS
     * from the main entry point.
     */
    if(csr & SSTAT0_STO)
    {
	SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),("STO\n"));
	if(sjp = shp->sh_currentjob)
	{
	    if(sjp->sj_ws)
		sjp->sj_ws->it_nexus->flags &= ~SZ_SYNC_NEG;
	    SIOP_SET_SJ_ERR(sjp,CAM_SEL_TIMEOUT);
	    siop_jobdone(sjp,ABNORMAL);
	}
	shp->sh_jobdone = NULL;
	shp->sh_jobrequest = NULL;
	shp->sh_currentjob = NULL;
	restart = 0;
	new_dsp = SCRIPT_ENTRY_WARM;
	siop_start_job(shp);
    }

    /* Phase Mismatches will occur whenever the target changes the phase
     * unexpectedly.  This happens during a block move when the target
     * changes phases before all bytes have been transferred.  This can
     * happen when tansferring a message, a command, or data.  The trick
     * is knowing the phase in which the data was being transferred.
     * Once the transfer phase is known, the appropriate actions can
     * be taken.  Unfortunately, the only way to determine this is to
     * read the instruction that was executing at the time of the interrupt.
     * This, combined with all the other processing done for the phase
     * mismatch make this a VERY expensive interrupt to take.
     */
    if((csr & (SSTAT0_MA|SSTAT0_PAR)) == SSTAT0_MA)
    {
	register U32 dbc, dnad, dfifo;

	/* Read the DSP register to get the current SIOP "pc".  Subtract
	 * 8 from that to get the SCRIPTS RAM address of the instruction
	 * causing the M/A.  It had better be a block move instruction.
	 */
	pc = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_GET_DSP,0);
	pc -= 8;	/* the DSP will be at the next instruction */
	dbc = SIOP_MD_SCRIPT_CNTL(shp,0,
		SIOP_SCRIPTS_READ_RAM,pc);
	/* now figure out the last phase from the last instruction */
	if((dbc & 0xC0000000))
	{
	    /* if the current instruction is not a block move instruction
	     * then something is very wrong.  This is because a phase
	     * mismatch interrupt can only occur on a block move instruction.
	     * In this case, just do the illegal instruction processing
	     * to try and recover.
	     * Also log the event.
	     */
	    CAM_ERROR(module,"siop_hardintr: bad phase mismatch",
		SIM_LOG_HBA_CSR|SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		shp->sh_softc,shp->sh_currentjob,pc);
	    goto bad_instruction;
	}

	sjp = shp->sh_currentjob;
	if(sjp == NULL)
	{
	    CAM_ERROR(module,"SIOP MA with no current request",
		SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
	}
	else switch(dbc & 0x07000000)
	{
	    /* for data transfer phases figure out the last byte
	     * sent on the bus and update the current data pointer
	     * to reflect the actual number of bytes sent/received.
	     */
	    case 0x01000000:	/* DATA IN */
		/* for incoming data, make sure the SIOP's FIFO's are
		 * flushed before reading any transfer count values.
		 */
		SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,CTEST8_FLF<<16);

		/* wait a while since there is no indication of how long
		 * a flush takes.
		 */
		DELAY(20);	/* delay 20 usec */

		/* the flush bit must be turned off manually */
		SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,0);

		/* fall through */
	    case 0:		/* DATA OUT */
		/* try to figure out how many bytes were transferred and
		 * how many were left.  The NCR53C710 spec describes this
		 * algorithm.
		 */
		dbc = SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_DBC0);
		dbc &= 0xffffff;
		dnad = SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_DNAD0);
		dfifo = SIOP_MD_CSR_READ(shp,CSR_BYTE0,SIOP_REG_DFIFO);
		SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,CTEST8_CLF<<16);

		/* set up the new ldatai structure */
		/* The following algorithm is defined by the NCR 53C710
		 * spec.  The SSTAT1 bits are in bits 16-23 for the
		 * word read from the 710.
		 */
#define SIOP_DEAD_XFER(F,R,C,A)  { register int SdXi; \
		SdXi = (F & 0x7f) - (C & 0x7f); \
		SdXi & = 0x7f; \
		if(R & (SSTAT1_ORF<<16)) \
		    SdXi++; \
		if(R & (SSTAT1_OLF<<16)) \
		    SdXi++; \
		C += SdXi; \
		A -= SdXi; }

		SIOP_DEAD_XFER(dfifo,r,dbc,dnad);

		sjp->sj_ldatai.di_count = dbc;
		sjp->sj_ldatai.di_datap = dnad;
		sjp->sj_ldatai.di_mbz = 0;

		/* figure out which datai entry is currently being used.
		 * This must be done this way since the SIOP doesn't have
		 * the chance to flush the siopjob struct out before
		 * interrupting.
		 */
		for(i = 0; i < 32 && sjp->sj_datai[i].di_count;i++)
		    if(sjp->sj_datai[i].di_datap <= dnad &&
		       dnad < (sjp->sj_datai[i].di_datap + 
			sjp->sj_datai[i].di_count))
			break;
			
		/* set up the offset of the current start of the current
		 * data pointer.
		 */
		sjp->sj_loffset = sjp->sj_doffset[i] + 
			    (sjp->sj_datai[i].di_count - dbc);

		restart = 0;
		new_dsp = SCRIPT_ENTRY_DATA_MA;
		break;

	    /* A phase mismatach error on a COMMAND phase probably occurs
	     * because the target detected a parity error and will either
	     * retry or terminate the request.
	     * This can also occur on an underrun when the target is
	     * reading the command.
	     * Abort the request since we can't be sure that the target
	     * will read the entire command.
	     */
	    case 0x02000000:		/* COMMAND */
		CAM_ERROR(module,"Phase mismatch on command phase",
		    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
		restart = 0;
		new_dsp = SCRIPT_ENTRY_ABORT;
		break;

	    /* Phase mismatached on a status phase should never occur since
	     * this phase only transfers one byte and will not be entered
	     * until the target is ready to trasnfer that byte.
	     * If this every happens, just punt.
	     */
	    case 0x03000000:		/* STATUS */
		CAM_ERROR(module,"Phase mismatch on status phase",
		    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
		restart = 0;
		new_dsp = SCRIPT_ENTRY_CONT;
		break;

	    /* A phase change on a message in should only occur because
	     * the SIOP has raised attention on detecting a parity
	     * error.  In this case, the parity bit should also be
	     * set so this code should never be reached.
	     * Another possibility is that the target has not sent the
	     * proper number of bytes for message transfers performed
	     * entirely in SCRIPTS RAM.
	     */
	    case 0x07000000:		/* MSG IN */
		new_dsp = SCRIPT_ENTRY_CONT;
		restart = 0;
		CAM_ERROR(module,"Phase mismatch on msg_in phase",
		    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
		break;

	    /* A phase mismatch on a MSGOUT phase should only happen when
	     * the target is rejecting a message.  In this case, the SCRIPTS
	     * will do an RPC_MSG_REJECT RPC when the reject message is 
	     * is received.  Figure out the actual number of bytes 
	     * transferred and put this in the rpcdata field for the
	     * expected RPC call.
	     */
	    case 0x06000000:		/* MSG OUT */
		dbc = SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_DBC0);
		dnad = SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_DNAD0);
		dbc &= 0xffffff;
		dfifo = SIOP_MD_CSR_READ(shp,CSR_BYTE0,SIOP_REG_DFIFO);
		SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,CTEST8_CLF<<16);

		SIOP_DEAD_XFER(dfifo,r,dbc,dnad);

		SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,sjp->sj_ws->lun,
		    (CAMD_ERRORS),
		    ("Phase mismatch on msg_out dbc = %x, dnad = %x\n",dbc,
		    dnad));
		/* update current message pointer */
		sjp->sj_rejptr = dnad - sjp->sj_smsgoptr.di_datap;
		sjp->sj_msgoptr.di_count = dbc;
		sjp->sj_msgoptr.di_datap = dnad;

		restart = 0;
		new_dsp = SCRIPT_ENTRY_MSG_MA;
		break;
	}
    }

    /* When a parity interrupt occurrs figure out if anything useful
     * can be done.  If the SIOP was in the middle of a transfer
     * to the target then normal SCSI error processing can take 
     * place.  If parity error occurred at some random point
     * in SCRIPTS execution then it is probably due to a parity
     * error in reading from the Lbus.  In this case just panic.
     */
    if((csr & (SSTAT0_PAR|SSTAT0_RST)) == SSTAT0_PAR)
    {
	SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),("PARITY ERROR\n"));
	/* Get the SCRIPTS PC (dsp) from the SIOP.  Subtract 8 to find
	 * the address of the instruction executing at the time of the
	 * error.
	 */
	pc = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_GET_DSP,0);
	pc -= 8;	/* the DSP will be at the next instruction */
	pc = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_READ_RAM,pc);

	CAM_ERROR(module,"PARITY ERROR",
	    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE|SIM_LOG_HBA_DME|SIM_LOG_SIM_WS|SIM_LOG_HBA_CSR,
	    shp->sh_softc,shp->sh_currentjob,pc);

	/* If the last instruction was not a block move instruction then
	 * the error occurreed on the LBUS and all that can be done
	 * is panic.
	 * If the last instruction was a block move instruction, but
	 * the SIOP was sending data to the SCSI bus then the error
	 * was probably an LBUS parity error.
	 * If the data was coming in from the target then assume a
	 * SCSI parity error.
	 */
	if((pc & 0xC0000000))
	    panic("siop_hard_intr: LBUS parity error");

	switch(pc & 0x07000000)
	{
	    case 0:			/* DATA OUT */
	    case 0x06000000:		/* MSG OUT */
	    case 0x02000000:		/* COMMAND */
		panic("siop_hard_intr: (LBUS?) parity error on data out phase");
		break;

	    /* if the data was incoming, then jump to the parity error
	     * handler in the SCRIPTS.
	     */
	    default:
	    case 0x01000000:		/* DATA IN */
	    case 0x03000000:		/* STATUS */
	    case 0x07000000:		/* MSG IN */
		break;
	}

	restart = 0;
	new_dsp = SCRIPT_ENTRY_ERROR;

	/* no job completion is done here.  This is all handled by the
	 * SCRIPTS.  If the target retries then the SCRIPTS will handle
	 * the retry.  If the target disconnects then we get a UDC.
	 * If the target returns a CHECK CONDITION then the SCRIPTS
	 * handle it.  However, mark the request as getting a parity
	 * error in case the target just disconnects (to override
	 * the UDC error status).
	 */
	if(sjp = shp->sh_currentjob)
	    sjp->sj_err = CAM_UNCOR_PARITY;
    }

    /* If a gross error occurs it's usually because the SIOP's SCSI FIFO
     * has overflowed or some other non-recoverable error has occurred
     * during the data transfer.
     * Treat this as a fatal error and reset the SCSI bus (this will
     * clear all the FIFO's).
     */
    if(csr & SSTAT0_SGE)
    {
	sjp = shp->sh_currentjob;
	SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),("SIOP SGE\n"));
	CAM_ERROR(module,"SCSI GROSS ERROR",
	    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE|SIM_LOG_HBA_DME|SIM_LOG_SIM_WS,
	    shp->sh_softc,sjp,NULL);

	/* First, finish up any job currently active on the bus */
	if(sjp)
	{
	    /* set sj_err explicitly, even if other errors occurred */
	    sjp->sj_err = CAM_SEQUENCE_FAIL;
	    siop_jobdone(sjp,ABNORMAL);
	}

	siop_reset_scsi(shp);
	restart = 0;
	new_dsp = 0;		/* wait for the RESET interrupt */
	shp->sh_jobdone = NULL;
    }

    /* if an unexpected disconnect occurs it's probably the target's
     * way of telling us it has had a fatal error.  At any rate, it
     * shouldn't reconnect.  Finish off the request and restart the SCRIPTS.
     */
    if((csr & (SSTAT0_UDC|SSTAT0_RST|SSTAT0_STO)) == SSTAT0_UDC)
    {
#ifdef SIOP_VERBOSE
	register U32 op;

	printf("SIOP %d - UDC ",shp->sh_ctlr->slot);
	pc = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_GET_DSP,0);
	pc -= 12;
	siop_printop(shp,pc);
#endif
	if(sjp = shp->sh_currentjob)
	{
	    if(sjp->sj_ws)
		sjp->sj_ws->it_nexus->flags &= ~SZ_SYNC_NEG;
	    SIOP_SET_SJ_ERR(sjp,CAM_UNEXP_BUSFREE);
	    siop_jobdone(sjp,ABNORMAL);
	}
	shp->sh_jobdone = NULL;
	shp->sh_currentjob = NULL;
	shp->sh_jobrequest = NULL;
	restart = 0;
	new_dsp = SCRIPT_ENTRY_WARM;
    }
    /* for bus resets all queues for this bus must be flushed but the
     * requests must not be returned to CAM.  CAM will take care of
     * all pending requests when it received the RESET notification
     * (which it must receive to reset it's state and to notify
     * upper drivers).
     */
    if(csr & SSTAT0_RST)
    {
	extern SIM_SM sim_sm;

	SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),
	    ("SCSI RESET DETECTED\n"));
        CAM_ERROR(module, "Bus reset detected", 
	    SIM_LOG_PRISEVERE|SIM_LOG_HBA_SOFTC,shp->sh_softc, NULL, NULL);

	restart = 0;
	new_dsp = 0;

	/*
	 * Complete all done jobs first, so they don't get completed
	 * twice (SIM reset code) and cause XPT callback Q corruption.
	 */
	siop_jobcomplete();

	/* flush all currently active and waiting jobs */
	siop_flush(shp,CAM_SCSI_BUS_RESET);

	/* turn off reset before restarting SCRIPTS */
	i = SIEN_STO|SIEN_SGE|SIEN_UDC|SIEN_RST|SIEN_MA|SIEN_PAR;
	i <<= 16;
	i |= SCNTL1_ESR;
	i <<= 8;
	i |= SCNTL0_ARB1|SCNTL0_ARB0|SCNTL0_EPC|SCNTL0_AAP;
	SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_SIEN,i);

	/* If we're not already waiting to finish a reset, set the RESET
	 * flag and call siop_wait_reset() to restart the SCRIPTS if
	 * the bus reset is complete (the RESET line is de-asserted).
	 */
	if(!(shp->sh_flags & SIOP_SH_RESET))
	{
	    shp->sh_flags |= SIOP_SH_RESET;
	    siop_wait_reset(shp);

	    /* kick of the CAM ISR thread to handle CAM cleanup if not
	     * already doing it.
	     */
	    if (!(shp->sh_softc->error_recovery & ERR_BUS_RESET)) 
	    {
	       U64 s3;

	       shp->sh_softc->error_recovery |= ERR_BUS_RESET;

	       /*
		* Lock on the State Machine.
		*/
	       SIM_SM_LOCK(s3, &sim_sm);

		/*
		 * Set the bus_reset bit in the state machine's struct.
		 */
	       sim_sm.bus_reset |= (1 << shp->sh_ctlr->ctlr_num);
	       if(!sim_sm.sm_active || shutting_down) 
	       {
		   /*
		    * Call the state machine to handle this interrupt.
		    * (NOTE: The state machine is no longer a thread.)
		    */
		   SIM_SCHED_ISR();
		}
		SIM_SM_UNLOCK(s3, &sim_sm);
	    }
	}
    }

    if(shp->sh_flags & SIOP_SH_RESET)
	new_dsp = restart = 0;

    shp->sh_intstatus = 0;

    /* If the SCRIPTS must be restarted in a place other than at the 
     * point where they interrupted then a new DSP will have been
     * set.  Just poke the DSP register to kick it off.
     */
    if(!restart && new_dsp)
	SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_SET_DSP,new_dsp);
    else if(restart)
    {
	restart |= DCNTL_EA|DCNTL_COM|DCNTL_STD;
	restart <<= 8;
	restart |= SIOP_WATCHDOG;
	restart <<= 8;
	restart |= DIEN_BF|DIEN_ABRT|DIEN_SIR|DIEN_WTD|DIEN_IID|DIEN_SSI;
	restart <<= 8;
	SIOP_MD_CSR_WRITE(shp,CSR_LONG,SIOP_REG_DMODE,restart);
    }

    return 1;
}

/* siop_printop()
 *
 *	args:	shp		pointer to SCRIPT_HOST for SIOP
 *		pc		SCRIPTS RAM address if instruction
 *
 *	Common routine for printing a SCRIPTS instruction (in hex).
 *	Print three words to be sure all words of the instruction were
 * 	printed.  Also print the absolute and relative DSP value 
 *	(relative is offset into the SCRIPTS).
 */

void
siop_printop(shp,pc)
register SCRIPT_HOST *shp;
register U32 pc;
{
    register U64 op;

    /* the following line is slightly machine dependent, but it's not
     * worth moving to a separate module.
     */
    printf("%x(%x) - ",pc,pc - (U32)shp->sh_ctlr->addr2);
    op = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_READ_RAM,pc);
    printf("%x ",op);
    op = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_READ_RAM,pc+4);
    printf("%x ",op);
    op = SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_READ_RAM,pc+8);
    printf("%x\n",op);
}

/* siop_wait_reset()
 *
 *	args:	shp		SCHRIP_HOST pointer for controller resetting.
 *
 *	Because the BUS reset can be held for long periods of time and
 *	because we want to wait for RESET to be de-asserted before
 *	continuing, check the status of the reset signal every few
 *	msec.  When it is de-asserted complete reset processing.
 *	Until then, the bus is frozen and siop_go() should return
 *	a status of CAM_BUSY.
 */

void
siop_wait_reset(shp)
register SCRIPT_HOST *shp;
{
    register U32 r, i;
    register U64 oldpri;
    SIM_MODULE(siop_wait_reset);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("sjp = 0x%lx\n", shp ) );
    r = SIOP_MD_CSR_READ(shp,CSR_BYTE2,SIOP_REG_DSTAT);

    /* if RESET is still asserted, try again later */
    if(r & (SSTAT1_RST<<16))
    {
	SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),
	    ("reset still asserted\n"));
	timeout(siop_wait_reset,(caddr_t)shp,10);
	oldpri = splsiop();
	shp->sh_flags |= SIOP_SH_RESET;
	splx(oldpri);
	return;
    }

    oldpri = splsiop();

    shp->sh_flags &= ~SIOP_SH_RESET;

    /* Make sure FIFO's are cleared before restarting */
    SIOP_MD_CSR_WRITE(shp,CSR_BYTE2,SIOP_REG_CTEST8,CTEST8_CLF<<16);

    /* restart SCRIPTS at reset entry point */
    shp->sh_intstatus = 0;

    SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_SET_DSP,SCRIPT_ENTRY_COLD);

    splx(oldpri);
}

/* siop_flush()
 *
 *	args:	shp		SCRIPT_HOST pointer for SIOP to flush
 *		status		status to return to CAM
 *
 *	Remove all requests from all queues associated with the
 *	controller and return them to CAM with the indicated status.
 */

void
siop_flush(shp,status)
register SCRIPT_HOST *shp;
register int status;
{
    register SIOP_JOB *sjp;
    register U64 target, lun;
    register SIOP_Q *qp;
    register SIM_WS *sws;
    SIM_MODULE(siop_flush);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
		("shp = 0x%lx, status = 0x%x\n", shp, status ) );
    shp->sh_currentjob = NULL;

    /* complete all active jobs */
    for(target = 0; target < NDPS; target++)
    {
	for(lun = 0; lun < NLPT; lun++)
	{
	    qp = &shp->sh_activeq[target][lun];
	    while(sjp = siop_qfirst(qp))
	    {
		/* set the sj_err explicitly */
		sjp->sj_err = (U64)status;
		sws = sjp->sj_ws;
		if(sws)
		{
		    SWS_NORMJOB(sws) = NULL;
		    SWS_PRIJOB(sws) = NULL;
		    sjp->sj_ws = NULL;
		}
		SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),
			("flushed active job, sjp = 0x%lx, sws = 0x%lx\n", sjp, sws));
		siop_jobdone(sjp,ABNORMAL);
	    }
	}
    }

    /* finally, toss any jobs waiting to go (they probably would
     * get rejected by the device after the reset anyway).
     */
    qp = &shp->sh_outq;
    while(sjp = qp->siopq_head)
    {
	/* set sj_err explicitly */
	sjp->sj_err = (U64)status;
	sws = sjp->sj_ws;
	if(sws)
	{
	    SWS_NORMJOB(sws) = NULL;
	    SWS_PRIJOB(sws) = NULL;
	    sjp->sj_ws = NULL;
	}
	SIM_PRINTD(shp->sh_ctlr->slot,NOBTL,NOBTL,(CAMD_ERRORS),
	    ("flushed outgoing job, sjp = 0x%lx, sws = 0x%lx\n", sjp, sws));
	siop_jobdone(sjp,ABNORMAL);
    }

    shp->sh_jobdone = NULL;
    shp->sh_jobrequest = NULL;
}

/* siop_start_job()
 *
 *	args:	shp		pointer to scripthost for controller
 *
 *
 *	Remove the job at the head of the outgoing queue and send it to
 * 	the SIOP.  Also, place it on the TID/LUN's active queue.
 */

void
siop_start_job(shp)
register SCRIPT_HOST *shp;
{
    register struct controller *ctlr = shp->sh_ctlr;
    register SIOP_JOB *sjp;
    register U8 tid, lun;
    register SIOP_Q *qp;
    U64 i;
    register U64 oldpri;
    SIM_MODULE(siop_start_job);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("shp = 0x%lx\n", shp ) );

    /* If the queue is empty try to get CAM to give up more work */
    if(!shp->sh_outq.siopq_head)
    {
	if(!cam_at_boottime() && !shutting_down)
	{
	    sim_sm.waiting_io |= (1 << shp->sh_softc->cntlr);
	    SIM_SCHED_ISR();
	}
    }

    oldpri = splsiop();

    while(!shp->sh_jobrequest && ((sjp = siop_qfirst(&shp->sh_outq)) != NULL))
    {
	sjp->sj_next = sjp->sj_prev = NULL;
	tid = sjp->sj_ws->targid;
	lun = sjp->sj_ws->lun;
	qp = &shp->sh_activeq[tid][lun];
	if(sjp->sj_tagged)
	    (void)siop_qins(sjp,qp);
	else
	    (void)siop_qadd(sjp,qp);

	/* if the request was terminated or aborted then complete it  */
	if(sjp->sj_term || sjp->sj_abort)
	{
	    /* set sj_err explicitly */
	    sjp->sj_err = CAM_REQ_ABORTED;
	    siop_jobdone(sjp,NORMAL);
	}
	else
	{
	    if(pmap_svatophys(sjp, &i) == KERN_INVALID_ADDRESS)
		panic("siop_start_job: can't map request");
	    shp->sh_jobrequest = (U32)i;
	}
    }
    splx(oldpri);
}

/* siop_jobdone()
 *
 *	args:	sjp		pointer to siop job that has completed
 *		normal		type of completion.
 *
 *	Remove a finished job from it's queue and queue it on the
 *	global done queue to be picked up by the SIOP thread.  If
 *	a job has an ABORT/TERM request pending, make sure that the
 *	SWS is not passed back until all outstanding requests complete.
 *	The normal flag is set if the job completed as the result of
 *	a JOB_DONE interrupt (the SCRIPTS has cleared the target/lun
 *	table).  If the job was finished due to a hardware interrupt,
 *	the target/lun table entry must be cleared before the next
 *	job is started.
 */

void
siop_jobdone(sjp,normal)
register SIOP_JOB *sjp;
int normal;
{
    register SIM_WS *sws;
    SIM_MODULE(siop_jobdone);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("sjp = 0x%lx, normal = 0x%x\n", sjp, normal ) );

    if(sjp == NULL)
	return;

    usimple_lock(&sjp->sj_shp->sh_lock);

    siop_qdel(sjp);

    /* make absolutely sure the the SCRIPTS job table entry is zero.
     * This only has to be done when a hardware interrupt causes
     * a job's termination without the SCRIPTS knowledge.
     */
    if(!normal)
	SIOP_MD_SCRIPT_CNTL(sjp->sj_shp,sjp,SIOP_SCRIPTS_SET_CACHE,0);

    /* Jobs won't always have working sets since either the working set
     * can be disconnected from the job or a job without a working set
     * can be created (not currently done, but should be allowed).
     */
    sws = sjp->sj_ws;
    if(sws)
    {
	/* if the device was reset we must clear the synchronous transfer
	 * parameters before the next request is started.
	 */
	if(sws->flags & SZ_DEVRS_INPROG)
	    siop_bdr(sjp->sj_shp,sws->targid,ABNORMAL);

	/* If a priority (ABORT or TERMINATE) job completed, then make sure
	 * the associated normal job completes.  This must be done since
	 * the priority job completing will mean that the normal job will
	 * never complete.
	 *
	 * However, if the normal job completes first, it will be orphaned
	 * so that the SWS will not be finished until all jobs are completed.
	 * (The priority job can't be aborted due to race conditions between
	 * the host and the 710, but this shouldn't be a problem since the
	 * worse this will do is cause a TERMINATE/ABORT to be sent to
	 * a non existant I_T_x nexus.)
	 */
	if(sjp == (SIOP_JOB *)SWS_PRIJOB(sws))
	{
	    SWS_PRIJOB(sws) = NULL;
	    if(SWS_NORMJOB(sws))
	    {
		sjp->sj_ws = NULL;
		siop_jobdone(SWS_NORMJOB(sws),NORMAL);
	    }
	}
	else if(SWS_PRIJOB(sws))	/* normal job w/priority job */
	{
	    /*
	     * Complete this job after the priority job completes above,
	     * to avoid adding this job to Siop_doneq twice.  This latter
	     * action also caused the free_buf list to be corrupted, since
	     * the same buffers get released twice, which causes a panic.
	     */
	    return;
	}
    }

    siop_qadd(sjp,&Siop_doneq);
    usimple_unlock(&sjp->sj_shp->sh_lock);

    if(siop_thread_started && !shutting_down)
	thread_wakeup_one((vm_offset_t)siop_jobcomplete);
    else
	siop_jobcomplete();
}

/* siop_jobcomplete()
 *
 *	args:
 *
 *	Handle processing of all jobs on the done queue.  This is the
 *	routine that does all the work in the SIOP thread.  Just remove
 *	each job from the head of the queue, set the final transfer
 *	count and completion status, handle the Autosense call,
 *	free up all resources, and pass back the SIM WS to CAM.
 */

void
siop_jobcomplete()
{
    register U64 i = 0;
    register SIM_WS *sws;
    register SIOP_JOB *sjp;
    SIM_MODULE(siop_jobcomplete);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("entering %s...\n", module) );

    while((sjp = siop_qfirst(&Siop_doneq)) != NULL)
    {
	sws = sjp->sj_ws;
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
			("sjp = 0x%lx, sws = 0x%lx\n", sjp, sws ) );
	if(sws)
	{
	    if(sws->flags & SZ_DEVRS_INPROG)
	    {
		sws->it_nexus->flags &= ~(SZ_SYNC | SZ_SYNC_NEG);
	    }

	    /* set the actual number of bytes transferred */
	    sws->data_xfer.xfer_count = sjp->sj_loffset;

	    /* if the upper byte of the status word is non-zero then
	     * valid status info was received.
	     */
	    sws->flags |= SZ_CMD_CMPLT;
	    if(sjp->sj_status & 0xff000000)
	    {
		sws->scsi_status = (U8)sjp->sj_status;
	    }
	    else
	    {
		sws->error_recovery |= ERR_PHASE;
		SIOP_SET_SJ_ERR(sjp,CAM_SEQUENCE_FAIL);
	    }


	    if(sjp->sj_err)
		  sws->cam_status = sjp->sj_err;
	    else
		  sws->cam_status = CAM_REQ_CMP;

	    switch(sws->scsi_status & ~SCSI_STAT_RESERVED) 
	    {
		 case SCSI_STAT_INTERMEDIATE:
		 case SCSI_STAT_INTER_COND_MET:
		     sws->nexus->flags |= SZ_CONT_LINK;
		     break;

		 case SCSI_STAT_CHECK_CONDITION:
		 case SCSI_STAT_COMMAND_TERMINATED:
		      sws->cam_status = CAM_REQ_CMP_ERR;
		      as_start(sws);
		      break;

		  default:
		  case SCSI_STAT_GOOD:
		      if(sjp->sj_err)
			  sws->cam_status = sjp->sj_err;
		      else
			  sws->cam_status = CAM_REQ_CMP;
		      break;

		  case SCSI_STAT_BUSY:
		      sws->cam_status = CAM_REQ_CMP_ERR;
		      break;

		  case SCSI_STAT_QUEUE_FULL:
		  case SCSI_STAT_CONDITION_MET:
		  case SCSI_STAT_RESERVATION_CONFLICT:
			sws->cam_status = CAM_REQ_CMP_ERR;
			break;
	    }
	    if(sws->cam_status != CAM_UNEXP_BUSFREE)
		sws->flags |= SZ_EXP_BUS_FREE;
	}

	siop_free_job(sjp);

	if(sws)
	    sm_bus_free(sws);
    }
    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("returning from %s...\n", module) );
}

/* siop_rpc()
 *
 *	args:	shp		pointer to scripthost struct
 *
 *	Handle RPC requests fromt he SCRIPTS. The SCRIPTS RPC mechanism
 *	is described in detail in the design spec.
 */

void
siop_rpc(shp)
register SCRIPT_HOST *shp;
{
    register SIOP_JOB *sjp;
    SIM_MODULE(siop_rpc);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("shp = 0x%lx\n", shp ) );

    /* a DEVICE RESET message can be sent even when there is no current job.
     * This could be caused by the SCRIPTS trying to reject an incoming
     * reconnect or trying to finish aborting a job.
     */
    if(shp->sh_rpcreq == RPC_DEVICE_RESET)
    {
	register int i, target;
	register SIOP_Q *qp;
	register int resetflagged = 0;

	/* the SCRIPTS need a device bus reset done.  To do this cleanly
	 * the SCRIPTS first tells us that it is about to do the reset.
	 * This allows us to clean out the active job queue.
	 * If the SCRIPTS are unable to perform the reset, they will
	 * do a hard reset on the entire bus.
	 */
	i = shp->sh_rpcdata >> 16;
	for(target = 0; target < NDPS; target++)
	    if(i & (1<<target))
		break;
	SIM_PRINTD(shp->sh_ctlr->slot,target,NOBTL,(CAMD_ERRORS),
	    ("GOT DEVICE RESET RPC\n"));

	/* If no target is specified this means that the SDID register
	 * was invalid.  Just ignore the request and let any
	 * active requests time out.
	 */
	if(target >= NDPS)
	{
	    CAM_ERROR(module,"Invalid TID on RPC_DEVICE_RESET",
		SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		shp->sh_softc,NULL,NULL);
	    return;
	}

	if(sjp = shp->sh_currentjob)
	{
	    sjp->sj_ws->flags |= SZ_DEVRS_INPROG;
	    siop_jobdone(sjp,ABNORMAL);
	}
	else
	    siop_bdr(shp,target,NORMAL);

	shp->sh_currentjob = NULL;
	shp->sh_jobdone = NULL;

	return;
    }

    /* A tag lookup does not have a current job, and shouldn't most of the
     * time.  Get the info passed back by the SCRIPTS and look for
     * the requested job in the active queue.  If it's not there then
     * the reconnect is bogus and the SCRIPTS will abort the reconnected
     * request.
     */
    if(shp->sh_rpcreq == RPC_LOOK_TAG)
    {
	register int target, lun, tag;
	register SIOP_Q *qp;
	register U64 oldpri;

looktag:
	/* a new job is being looked up so invalidate any current job */
	shp->sh_currentjob = NULL;

	/* The SCRIPTS passes the target ID in bits 0-7, the LUN in
	 * bits 8-15, and the tag in bits 16-23 of sh_rpcdata.
	 */
	target = shp->sh_rpcdata & 0x7;
	lun = (shp->sh_rpcdata>>8) & 0x7;
	tag = (shp->sh_rpcdata>>16) & 0xff;

	qp = &shp->sh_activeq[target][lun];

	SIM_PRINTD(shp->sh_ctlr->slot,target,lun,(CAMD_TAGS),
	    ("TAG LOOKUP: tag %d qp = %x\n",tag,qp));

	/* look the request up in the active queue for the target/lun.
	 * Lock the queue to keep it's pointers from changing during the
	 * search.
	 */
	oldpri = splsiop();
	usimple_lock(qp);
	sjp = qp->siopq_head;
	while(sjp)
	{
	    if(sjp->sj_tag >= tag)
		break;
	    sjp = sjp->sj_next;
	}
	usimple_unlock(qp);
	splx(oldpri);

	if(sjp && (sjp->sj_tag == tag))
	{
	    U64 addr;

	    if(pmap_svatophys(sjp, &addr) == KERN_INVALID_ADDRESS)
		panic("siop_rpc: can't map request");
	    shp->sh_rpcreply = (U32)addr;
	    shp->sh_currentjob = sjp;
	}
	else
	{
	    shp->sh_rpcreply = 0;
	    SIM_PRINTD(shp->sh_ctlr->slot,target,lun,(CAMD_TAGS),
	    	("can't find tag %d:\n",tag));
	}
    }

    /* the rest of the RPC's require a current job to be valid */
    if((sjp = shp->sh_currentjob) == NULL)
    {
	CAM_ERROR(module,"no job for RPC",
	    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,shp->sh_softc,NULL,NULL);
	return;
    }

    
    /* Move the DMA window up */
    if(shp->sh_rpcreq == RPC_MORE_DATA)
    {
	siop_dma_window(sjp,sjp->sj_offset+sjp->sj_count);

	/* no RPC return data */
	/* set up the current data pointers and mark the saved pointers as
	 * not being valid for the current mapping.
	 */
	sjp->sj_ldatai = sjp->sj_datai[0];
	sjp->sj_ldatap = 0;
	sjp->sj_sdatai.di_mbz = 0xff;
	sjp->sj_loffset = sjp->sj_doffset[0];
    }
    else if(shp->sh_rpcreq == RPC_MODIFY_PTR)
    {
	register int i, j;
	register U64 offset;

	SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,sjp->sj_ws->lun,
	    (CAMD_INOUT|CAMD_FLOW),("got RPC_MODIFY_PTR\n"));
	j = shp->sh_rpcdata;

	/* the offset provided by the target is in big endian format.
	 * convert it to little endian.
	 */
	i = (j & 0xff)<<24;
	i |= (j & 0xff00) << 8;
	i |= (j & 0xff0000) >> 8;
	i |= (j & 0xff000000) >> 24;
	offset = sjp->sj_loffset;

	SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,sjp->sj_ws->lun,
		(CAMD_FLOW),("current offset = %x, delta = %x\n",offset,i));

	siop_dma_window(sjp,offset+i);

	sjp->sj_ldatai = sjp->sj_datai[0];
	sjp->sj_ldatap = 0;
	sjp->sj_loffset = sjp->sj_doffset[0];
	sjp->sj_sdatai.di_mbz = 0xff;

	/* if unable to map, the map count will be 0 */
	if(sjp->sj_count)
	   shp->sh_rpcreply = 0;
   	else
	   shp->sh_rpcreply = 1;
    }
    else if(shp->sh_rpcreq == RPC_REST_PNTR)
    {
	SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,sjp->sj_ws->lun,
	    (CAMD_INOUT|CAMD_FLOW),("RPC RESTORE POINTER\n"));

	siop_dma_window(sjp,sjp->sj_soffset);
	sjp->sj_sdatai.di_mbz = 0;

	/* When the data pointers ware restored, the current DMA
	 * window will start AT the saved data pointer.  Therefor,
	 * update the save data pointer to the start of the scatter/gather
	 * list. The saved offset should be left alone since it should
	 * still be valid.
	 */
	sjp->sj_sdatap = 0;
	sjp->sj_sdatai = sjp->sj_datai[0];
    }
    else if(shp->sh_rpcreq == RPC_MSG_REJECT)
    {
	register U8 *c;
	register int count, i;
	register SIM_WS *sws;

	shp->sh_rpcreply = 0;
	/* figure out number of message bytes actually sent to the target.
	 * If a message was rejected in the middle of a message sequence
	 * then smsgoptr will be valid.  Otherwise, assume the reject
	 * occurred on the last byte of the message (i.e., no phase
	 * mismatch interrupt occurred.
	 */
	if(sjp->sj_rejptr)
		count = sjp->sj_rejptr;
	else
	    count = sjp->sj_smsgoptr.di_count;
	sjp->sj_rejptr = 0;
	SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,sjp->sj_ws->lun,
	   (CAMD_MSGIN|CAMD_ERRORS),
	   ("got RPC_MSG_REJECT at byte = %d\n",count-1));

	/* now go through the message list and find the rejected
	 * message. Take whatever action is appropriate for the
	 * rejected mesage.
	 *
	 * There are only a few messages which can get rejected since
	 * we don't use sim_sm.c to handle state changes.
	 * These are:
	 *	IDENTIFY
	 *	SYNC NEGOCIATION
	 *	TAG_XXX
	 *	ABORT
	 *	TERMINATE
	 *	DEVICE BUS RESET
	 *
	 * All other messages are automatically sent by the SCRIPTS and
	 * rejections are handled local to the SCRIPTS.
	 */
	c = (U8 *)sjp->sj_msgobuf;
	sws = sjp->sj_ws;
	i = 0;
	while(i < count)
	{
	    /* two byte messages */
	    if(SCSI_IS_MSG_TWO_BYTE(*c))
	    {
		if((count - i) <= 2)
		{
		    SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,
			sjp->sj_ws->lun,(CAMD_MSGIN|CAMD_ERRORS),
			("rejected %x %x\n",*c,*(c+1)));
		    switch(*c)
		    {
			/* if a tag message is rejected abort the request */
			case SCSI_HEAD_OF_QUEUE_TAG:
			case SCSI_ORDERED_QUEUE_TAG:
			case SCSI_SIMPLE_QUEUE_TAG:
			    shp->sh_rpcreply = 
				RPC_REPLY_ATN | RPC_REPLY_ERROR | SIOP_KILL_ABORT;
			    break;
		    }
		}
		c += 2;
		i += 2;
	    }
	    else if(*c == SCSI_EXTENDED_MESSAGE)	/* extended message */
	    {
		if((count >= (i+1)) && (count < (i + *(c+1) + 2)))
		{
		    SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,
			sjp->sj_ws->lun,(CAMD_MSGIN|CAMD_ERRORS),
		        ("rejected %x %x\n",*c,*(c+2)));
		    switch(*(c+2))
		    {
			case SCSI_SYNCHRONOUS_XFER:
			    /* if the reply to a target initiated negotiation
			     * was rejected then the sync table in SCRIPTS
			     * RAM should be reset.
			     */
			    if(!(sws->it_nexus->flags & SZ_SYNC_NEG))
			    {
				SIOP_MD_SCRIPT_CNTL(shp,sjp,
					SIOP_SCRIPTS_CLEAR_SYNC,sws->targid);
			    }

			    sws->it_nexus->flags &= ~(SZ_SYNC | SZ_SYNC_NEG);
			    sws->it_nexus->sync_offset = 0;
			    sws->it_nexus->sync_period = 0;
			    break;

			case SCSI_WIDE_XFER:
			    break;
		    }
		}
		i += *(c+1) + 2;
		c += *(c+1) + 2;
	    }
	    else
	    {
		if((i+1) != count)
		{
		    i++;
		    c++;
		    continue;
		}
		SIM_PRINTD(shp->sh_ctlr->slot,sjp->sj_ws->targid,
		    sjp->sj_ws->lun,(CAMD_MSGIN|CAMD_ERRORS),
		    ("rejected %x at %d\n",*c,i+1));

		if((*c & SCSI_IDENTIFY) == SCSI_IDENTIFY)
		{
		    sjp->sj_ws->error_recovery |= ERR_MSG_REJ;
		    i++;
		    c++;
		    break;
		}
		else switch(*c)
		{
		    case SCSI_INITIATOR_DETECTED_ERROR:
			if(sws->recovery_status & 
				(ERR_DATAIN_PE | ERR_STATUS_PE)) 
			    sws->error_recovery |= ERR_PARITY;
			/* fall through */
		    case SCSI_ABORT_TAG:
			shp->sh_rpcreply = 
			    RPC_REPLY_ATN | RPC_REPLY_ERROR | SIOP_KILL_ABORT;
			break;

		    case SCSI_MESSAGE_PARITY_ERROR:
			if(sws->recovery_status & ERR_MSGIN_PE) 
			    sws->error_recovery |= ERR_PARITY;
			shp->sh_rpcreply = 
			    RPC_REPLY_ATN | RPC_REPLY_ERROR | SIOP_KILL_ABORT;
			break;

		    case SCSI_BUS_DEVICE_RESET:
			/* just let the SCRIPTS reset the SCSI bus */
			shp->sh_rpcreply = 
			    RPC_REPLY_ATN | RPC_REPLY_ERROR | SIOP_KILL_RESET;
			break;

		    case SCSI_ABORT:
		    case SCSI_CLEAR_QUEUE:
			shp->sh_rpcreply = 
			    RPC_REPLY_ATN | RPC_REPLY_ERROR | SIOP_KILL_DRESET;
			break;

		    case SCSI_TERMINATE_IO_PROCESS:
			shp->sh_rpcreply = RPC_REPLY_ATN | RPC_REPLY_ERROR;
			if(sws->flags & SZ_TAGGED) 
			    shp->sh_rpcreply |= SIOP_KILL_AT;
			else 
			    shp->sh_rpcreply |= SIOP_KILL_ABORT;
			break;

		    case SCSI_RELEASE_RECOVERY:
		    case SCSI_MESSAGE_REJECT:
			sws->error_recovery |= ERR_MSG_REJ;
			shp->sh_rpcreply = 
			    RPC_REPLY_ATN | RPC_REPLY_ERROR | SIOP_KILL_ABORT;
			break;

			
		    /* rejection of the following messages will cause no 
		     * action to be taken.
		     */
		    case SCSI_DISCONNECT:
		    case SCSI_NO_OPERATION:
			break;

		    default:
			CAM_ERROR(module,"Can't handle reject message",
			    SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE|SIM_LOG_HBA_DME|SIM_LOG_SIM_WS,
			    shp->sh_softc,sjp,NULL);
			break;
		}
	    }
	}
	/* skip to the start of the next complete message */
	sjp->sj_msgoptr.di_datap = sjp->sj_smsgoptr.di_datap + i;
	sjp->sj_msgoptr.di_count = sjp->sj_smsgoptr.di_count - i;
	if(sjp->sj_msgoptr.di_count)
	    shp->sh_rpcreply |= RPC_REPLY_ATN;
	shp->sh_rpcreply |= RPC_REPLY_ACK;

	/* if the ERROR bit is set then the job is being aborted, so might
	 * as well get rid of it now.
	 */
	if(shp->sh_rpcreply & RPC_REPLY_ERROR)
	{
	    siop_jobdone(sjp,ABNORMAL);
	    shp->sh_currentjob = NULL;
	}
    } 
    else if(shp->sh_rpcreq == RPC_CAL_SYNC)
    {
	register SIM_WS *sws;
	register U8 period, offset, value;
	register SIM_SOFTC *scp;

	/* figure out if this is the first SDTR from the target or a
	 * response to ours. These RPC's are only expected when there
	 * is a current job since target connections are not currently
	 * allowed.
	 */
	sjp = shp->sh_currentjob;
	if(sjp == NULL)
	{
	    shp->sh_rpcreply |= RPC_REPLY_ACK|RPC_REPLY_ERROR|RPC_REPLY_ATN;
	    return;
	}
	sws = sjp->sj_ws;
	scp = shp->sh_softc;

	period = shp->sh_rpcdata & 0xff;
	offset = (shp->sh_rpcdata>>8) & 0xff;
	value = 0;
	shp->sh_rpcreply = 0;

	if(sws->it_nexus->flags & SZ_SYNC_NEG)
	{
	    /* this is a response to our SDTR */
	    sws->it_nexus->flags &= ~SZ_SYNC_NEG;
	    if((period && (period < shp->sh_min_period)) ||
	       (offset && (offset > shp->sh_max_offset)) ||
	       ((shp->sh_flags & SIOP_SH_FAST) && period > SIOP_FAST_MAX_PERIOD) ||
	       (!(shp->sh_flags & SIOP_SH_FAST) && period > SIOP_SLOW_MAX_PERIOD))
	    {
		/* Either the period or offset is bogus, so reject
		 * the message.  The reason rejection is permissable
		 * here (and not when the target initiates negociation)
		 * is that the target should NEVER return a lesser
		 * period or greater offset.  If it returns a greater
		 * offset than is supported by the chip, then the 
		 * negociation is rejected and async mode is used.
		 */
		shp->sh_rpcreply = RPC_REPLY_ACK|RPC_REPLY_ERROR|RPC_REPLY_ATN;
		period = offset = 0;
	    }
	    /* it's ok, set up the reply to the SCRIPTS.
	     * The following values are very dependant on the controller's
	     * clock rate.
	     * The algorithm for converting the SCSI transfer period to
	     * something the SIOP will understand is given in the chip
	     * data sheet.
	     */
	    if(period)
	    {
		if(shp->sh_flags & SIOP_SH_FAST)
		    SIOP_FAST_PERIOD(period,value)
		else
		    SIOP_SLOW_PERIOD(period,value)
	    }

	    /* if the period is too great then reject the SDTR */
	    value <<= 4;	/* put in bits 4-6 of register */
	    if(value > SXFER_TP)
	    {
		shp->sh_rpcreply |= RPC_REPLY_ACK|RPC_REPLY_ERROR|RPC_REPLY_ATN;
		value = 0;
		period = 0;
		offset = 0;
	    }
	    value += offset & SXFER_MO;

	    /* the RPC reply contains four pieces of info, one in each byte of
	     * the reply.
	     * Byte0 contains the offset into the tdtable for the sync
	     * parameters.
	     * Byte 1 contains the values to stuff into the SXFER regiser.
	     * Byte 2 contains the reply xfer period (if needed).
	     * Byte 3 contains bits telling the SCRIPTS how to reply to
	     * the SDTR message.
	     */
	    shp->sh_rpcreply |= sws->targid*4;
	    shp->sh_rpcreply |= value<<8;
	    shp->sh_rpcreply |= RPC_REPLY_ACK;
	    scp->sync_period = period;
	    scp->sync_offset = offset;
	    if (period || offset)
		sws->it_nexus->flags |= SZ_SYNC;
	}
	else
	{
	    register U8 *c;

	    /* the target is initiating negociation. */
	    sws->it_nexus->flags &= ~SZ_SYNC_NEEDED;

	    /* check if sync is allowed on this nexus.
	     * If sync is not allowed on this device then negociate async
	     * rather than rejecting the SDTR (the SCSI spec says that no
	     * device capable of performing SYNC transfer should reject
	     * an SDTR, and the initiator is capable).
	     */
	    if(sws->cam_flags & CAM_DIS_SYNC)
		period = offset = 0;
	    else if(!shp->sh_min_period || !shp->sh_max_offset)
		period = offset = 0;
	    else
	    {
		if(period && period < shp->sh_min_period)
		    period = shp->sh_min_period;
		if(offset && offset > shp->sh_max_offset)
		    offset = shp->sh_max_offset;
		if(shp->sh_flags & SIOP_SH_FAST)
		{
		    if(period > SIOP_FAST_MAX_PERIOD)
			period = offset = 0;
		}
		else if(period > SIOP_SLOW_MAX_PERIOD)
		    period = offset = 0;
	    }

	    if(!period || !offset)
		period = offset = 0;
	    else
	    {
		if(shp->sh_flags & SIOP_SH_FAST)
		    SIOP_FAST_PERIOD(period,value)
		else
		    SIOP_SLOW_PERIOD(period,value)
	    }

	    /* if the period is too great then negociate async */
	    value <<= 4;
	    if(value > SXFER_TP)
	    {
		value = 0;
		period = offset = 0;
	    }
	    value += offset & SXFER_MO;
	    shp->sh_rpcreply = sws->targid*4;
	    shp->sh_rpcreply |= value<<8;
	    shp->sh_rpcreply |= RPC_REPLY_ACK|RPC_REPLY_ATN;
	    shp->sh_rpcreply |= period << 16;
	    scp->sync_period = period;
	    scp->sync_offset = offset;
	    if (period || offset)
		sws->it_nexus->flags |= SZ_SYNC;
	}
    }
}

/* siop_bdr()
 *
 *  args:	shp		SCRIPT HOST pointer
 *		target		target id of device receiving reset.
 *		flag		NORMAL if caller wants normal completion
 *				of request
 *
 * returns:	void
 *
 * Clear out all pending requests for a device which has been sent
 * a BDR (by the host or by the SCRIPTS).
 */

void
siop_bdr(shp,target,flag)
register SCRIPT_HOST *shp;
int target;
int flag;
{
    register int i;
    register SIOP_Q *qp;
    register SIOP_JOB *sjp, *nsjp;
    register U64 oldpri;
    register SIM_WS *sws;
    SIM_MODULE(siop_bdr);

    SIM_PRINTD(shp->sh_ctlr->slot,target,NOBTL,(CAMD_ERRORS),
	("RECEIVED BDR\n"));

    SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_CLEAR_SYNC,target);

    /* First, clear out any jobs waiting to go on the output queue.
     * Do this now to try to prevent more requests from getting started.
     */
    qp = &shp->sh_outq;

    SIM_SOFTC_LOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);
    usimple_lock(&shp->sh_lock);
    sjp = siop_qfirst(qp);
    while(sjp)
    {
	nsjp = sjp->sj_next;
	if(sjp->sj_ws->targid == target)
	{
	    sws = sjp->sj_ws;
	    if(sws && flag != NORMAL)
	    {
		SWS_NORMJOB(sws) = NULL;
		SWS_PRIJOB(sws) = NULL;
		sjp->sj_ws = NULL;
	    }
	    sjp->sj_err = CAM_BDR_SENT;
	    siop_jobdone(sjp,ABNORMAL);
	}
	sjp = nsjp;
    }
    usimple_unlock(&shp->sh_lock);
    SIM_SOFTC_UNLOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);

    /* clear out all requests for every LUN on the reset target */
    for(i = 0; i < NLPT; i++)
    {
	qp = &shp->sh_activeq[target][i];
	while(sjp = siop_qfirst(qp))
	{
	    sws = sjp->sj_ws;
	    if(sws && flag != NORMAL)
	    {
		SWS_NORMJOB(sws) = NULL;
		SWS_PRIJOB(sws) = NULL;
		sjp->sj_ws = NULL;
	    }
	    /* set sj_err explicitly */
	    sjp->sj_err = CAM_BDR_SENT;
	    siop_jobdone(sjp,ABNORMAL);
	}
    }
}

void
siop_dumpregs(shp)
register SCRIPT_HOST *shp;
{
#ifdef SIOP_VERBOSE
    register struct controller *ctlr = shp->sh_ctlr;
    register U32 i;

    printf("siopregs = ");
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,0);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,4);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,8);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,12);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,16);
    printf("%x, ",i);
    printf("-0-, ");
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,24);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,28);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,32);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,36);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,40);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,44);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,48);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,52);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,56);
    printf("%x, ",i);
    i = SIOP_MD_CSR_READ(shp,CSR_LONG,60);
    printf("%x\n",i);
#endif
}

/***************************************************************************
 *			JOB CREATION ROUTINES				   *
 ***************************************************************************/

/* siop_msg_gen()
 *
 *	args:	sjp		pointer to siopjob struct to message
 *		sws		pointer to SIM_WS for request.
 *
 *	Copy an outgoing message from the SIM_WS msgqueue to the
 *	output buffer.
 */

int
siop_msg_gen(sjp,sws)
register SIOP_JOB *sjp;
register SIM_WS *sws;
{
    register U8 *c;
    U64 i;
    register U32 size = 0;
    SIM_MODULE(siop_msg_gen);

    if((c = sjp->sj_msgobuf) == NULL)
    {
	c = siop_alloc_buf();
	if(c == NULL)
	    return 0;
	sjp->sj_msgobuf = c;
	if(pmap_svatophys(c, &i) == KERN_INVALID_ADDRESS)
	    panic("siop_msg_gen: can't map request");
	sjp->sj_smsgoptr.di_datap = (U32)i;
	sjp->sj_smsgoptr.di_count = 0;
    }

    size = 0;
    while(size < SC_GET_MSGOUT_LEN(sws))
    {
	*c++ = SC_GET_MSGOUT(sws,size);
	size++;
    }
    SC_UPDATE_MSGOUT(sws,size);

    sjp->sj_smsgoptr.di_count = size;
    sjp->sj_msgoptr.di_count = size;
    sjp->sj_msgoptr.di_datap = sjp->sj_smsgoptr.di_datap;
    sjp->sj_rejptr = 0;
    return size;
}

/* siop_cmd_gen()
 *
 *	args:	sjp		pointer to siopjob struct
 *		cmdp		pointer to command bytes
 *		cmdsz		number of command bytes
 *
 *	Allocate a buffer and fill it with the command.  Set up the
 *	cmdptr member of the siopjob struct.
 */

int
siop_cmd_gen(sjp,cmdp,cmdsz)
register SIOP_JOB *sjp;
register U8 *cmdp;
int cmdsz;
{
    U64 i;
    register U8 *c;
    SIM_MODULE(siop_cmd_gen);

    if((c = sjp->sj_cmdbuf) == NULL)
    {
	c = siop_alloc_buf();
	if(c == NULL)
	    return 0;
	sjp->sj_cmdbuf = c;
	if(pmap_svatophys(c, &i) == KERN_INVALID_ADDRESS)
	    panic("siop_cmd_gen: can't map SCB");
	sjp->sj_cmdptr.di_datap = (U32)i;
    }
    i = cmdsz;
    while(i--)
	*c++ = *cmdp++;
    sjp->sj_cmdptr.di_count = cmdsz;
    return cmdsz;
}

/* CAM INTERFACE ROUTINES */

/* siop_go()
 *
 *	args:	scp		SIM_SOFTC pointer for controller
 *		sws		pointer to working set of request
 *
 *
 *	returns:	cam status of request
 *
 *	Kick off a new request.
 */
U32 
siop_go(sws) 
register SIM_WS *sws;
{ 
    register CCB_SCSIIO *ccbp;
    register SIOP_JOB *sjp;
    register SIM_SOFTC *scp = sws->sim_sc;
    register SCRIPT_HOST *shp = (struct scripthost *)scp->hba_sc;
    long i;
    SIM_MODULE(siop_go);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("sws = 0x%lx\n", sws ) );

    /* if the bus is not alive then there is no HBA here */
    if(!(shp->sh_flags & SIOP_SH_ALIVE))
    {
	sws->cam_status = CAM_NO_HBA;
	return CAM_NO_HBA;
    }

    /* if the bus is being reset, return BUSY */
    if(shp->sh_flags & SIOP_SH_RESET)
    {
	sws->cam_status = CAM_BUSY;
	return CAM_BUSY;
    }

    /* The driver does not support linked commands yet. */
    if(sws->cam_flags & CAM_CDB_LINKED)
    {
	sws->cam_status = CAM_PROVIDE_FAIL;
	return CAM_PROVIDE_FAIL;
    }

    ccbp = sws->ccb;

    SIM_PRINTD(shp->sh_ctlr->slot,sws->targid,sws->lun,(CAMD_CMD_EXP),
	("siop_go called\n",shp->sh_ctlr->slot,sws->targid,sws->lun));

    /* allocate and setup the siopjob structure for the request */
    sjp = siop_alloc_job();
    if(sjp == NULL)
    {
	SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),
	    ("siop_go: can't allocate siop job\n"));
	sws->cam_status = CAM_BUSY;
	return CAM_BUSY;
    }

    /* The target ID is really the bitmap of the target to select.
     * this is put in the correct position for the SCRIPTS to use
     * it's value in the table-indirect form of the SELECT instruction.
     */
    sjp->sj_tid = 1<<(sws->targid+16);

    /* The lun used by the SCRIPTS is really the target/lun in a format
     * in which the lun can be easily masked, and which can be easily
     * multiplied by 4 by the SCRIPTS to get an offset into the job table.
     */
    sjp->sj_lun = sws->lun | (sws->targid*NDPS);
    sjp->sj_ws = sws;
    sjp->sj_shp = shp;
    if(sws->flags & SZ_TAGGED)
    {
	/* make sj_tagged ad tag line address that can be used directly
	 * by the SCRIPTS to save SCRIPTS code and time.
	 */
	sjp->sj_tagged = SIOP_MD_SCRIPT_CNTL(shp,sjp,
		SIOP_SCRIPTS_CACHE,0);
	sjp->sj_tag = sws->tag;
    }

    /* figure out if we need to do any synchronous negociation.
     * If CAM is requesting this, it will have to be done by the driver
     * since we make no callbacks to the SIM state machine.
     */
    if(sws->it_nexus->flags & (SZ_SYNC_NEEDED|SZ_SYNC_CLEAR))
    {
	SC_ADD_MSGOUT(sws, SCSI_EXTENDED_MESSAGE);
	SC_ADD_MSGOUT(sws, 0x03);
	SC_ADD_MSGOUT(sws, SCSI_SYNCHRONOUS_XFER);
	if(sws->it_nexus->flags & SZ_SYNC_CLEAR)
	{
	    SC_ADD_MSGOUT(sws,0);
	    SC_ADD_MSGOUT(sws,0);
	    scp->sync_period = 0;
	    scp->sync_offset = 0;
	    sws->it_nexus->flags &= ~SZ_SYNC_CLEAR;
	}
	else
	{
	    SC_ADD_MSGOUT(sws,shp->sh_min_period);
	    SC_ADD_MSGOUT(sws,shp->sh_max_offset);
	    scp->sync_period = shp->sh_min_period;;
	    scp->sync_offset = shp->sh_max_offset;
	}
	sws->it_nexus->flags &= ~SZ_SYNC_NEEDED;
	sws->it_nexus->flags |= SZ_SYNC_NEG;
    }
	     
	
    /* set up the buffer for outgoing messages and load it with whatever
     * messages CAM has placed in it's buffer.
     */
    if(!siop_msg_gen(sjp,sws))
    {
	siop_free_job(sjp);
	sws->cam_status = CAM_BUSY;
	return CAM_BUSY;
    }

    /* If there is a command, set up the command buffer. */
    if(ccbp && ccbp->cam_cdb_len)
    {
	if(ccbp->cam_ch.cam_flags & CAM_CDB_POINTER)
	    i = siop_cmd_gen(sjp,ccbp->cam_cdb_io.cam_cdb_ptr, 
		ccbp->cam_cdb_len);
	else
	    i = siop_cmd_gen(sjp,ccbp->cam_cdb_io.cam_cdb_bytes,
		ccbp->cam_cdb_len);
	if(!i)
	{
	    siop_free_job(sjp);
	    sws->cam_status = CAM_BUSY;
	    return CAM_BUSY;
	}
    }

    /* set up the first dma window */
    if(ccbp)
    {
	sws->data_xfer.data_count = ccbp->cam_dxfer_len;
	siop_dma_window(sjp,0);
    }

    /* if sj_err is set here then one of the setup routines encountered
     * and error and the request should not be started.
     * Release all resources and pass the error back to CAM.
     */
    if(sjp->sj_err)
    {
        sjp->sj_ws->cam_status = sjp->sj_err;
	siop_free_job(sjp);
	return sjp->sj_ws->cam_status;
    }

    /* set up the current data pointers and mark the saved pointers as
     * being valid for the current mapping.
     */
    sjp->sj_ldatai = sjp->sj_datai[0];
    sjp->sj_ldatap = 0;
    sjp->sj_loffset = sjp->sj_doffset[0];

    sjp->sj_sdatai = sjp->sj_ldatai;
    sjp->sj_sdatap = sjp->sj_ldatap;
    sjp->sj_soffset = sjp->sj_loffset;

    /* if queue was previously empty then kick the 710 to get it's
     * attention.  Device reset jobs get put at the head of the queue.
     */
    if(sws->flags & 
	(SZ_DEVRS_INPROG|SZ_ABORT_TAG_INPROG|SZ_ABORT_INPROG|SZ_TERMIO_INPROG))
    {
	SWS_PRIJOB(sws) = (U64)sjp;
	i = siop_qhead(sjp,&shp->sh_outq);
    }
    else
    {
	SWS_NORMJOB(sws) = (U64)sjp;
	i = siop_qadd(sjp,&shp->sh_outq);
    }

    /* if i is 0 then the active queue was previously empty so the SIGP
     * bit in the ISTAT register should be hit since the SIOP may be idle.
     */
    if(!i)
    {
	i = ISTAT_SIGP;
	i <<= 8;
	SIOP_MD_CSR_WRITE(shp,CSR_BYTE1,SIOP_REG_ISTAT,i);
    }

    /* if at boot time then interrupts are disabled.  We'll just have
     * to spin until the SIOP interrupts.
     */
    if(cam_at_boottime() || shutting_down)
    {
	int delay = SIOP_RESPONSE_TIME;

	while ( delay-- )
	{
	    /* wait for request to complete. */
            DELAY ( SIOP_ONE_SECOND_DELAY / 1000 );

	    i = (long)SIOP_MD_CSR_READ(shp,CSR_LONG,SIOP_REG_ISTAT);
	    i >>= 8;
	    i &= 0xff;
	    if(i & (ISTAT_SIP|ISTAT_DIP))
	    {
		SIOP_MD_INTR(shp);
		if(sws->flags & SZ_CMD_CMPLT)
		    break;
	    }
	}
	/*
	 * If command did not complete, set failure status.
	 */
	if ( delay == 0 )
	    sws->cam_status = CAM_CMD_TIMEOUT;
    }
    else if(!siop_thread_started)
	siop_thread_init();

    return CAM_REQ_CMP;
}

/* siop_sel_stop()
 *
 *	args:	sws		pointer to SIM_WS for request to stop.
 *
 *
 *	This routine is called by the SIM SCHEDULER when it abort or
 *	terminates a request, or when it is sending a device reset
 *	message to a controller.
 *
 *	To handle this, a new job is created for the TERM/ABORT
 *	message, and the existing job is marked for termination.
 *	If the current job reselects and aborts before the new
 *	job starts, then the TERM/ABORT message is still sent
 *	but will have no effect since it will identify a non
 *	active I_T_x nexus.  If the ABORT/TERM finishes first then
 *	the original job will never complete.  All this is handled
 *	in the jobdone processing.
 *
 *	Note that the original job (or the new job) can not be 
 *	dequeued at any place other than in the jobdone processing
 *	to avoid race conditions with the SIOP.
 *
 *	For bus resets, there is no current request so just make a
 *	job to put on the queue.
 */
U32 
siop_sel_stop(sws)
register SIM_WS *sws;
{ 
    register SIOP_JOB *sjp;
    register SCRIPT_HOST *shp;
    register U64 oldpri;
    register retval;
    SIM_MODULE(siop_sel_stop);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("sws = 0x%lx\n", sws ) );

    shp = (struct scripthost *)sws->sim_sc->hba_sc;

    SIM_SOFTC_LOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);
    usimple_lock(&shp->sh_lock);
    sjp = (SIOP_JOB *)SWS_NORMJOB(sws);

    /* If another abort/reset is already in progress then ignore this one */
    if(SWS_PRIJOB(sws))
    {
	usimple_unlock(&shp->sh_lock);
	SIM_SOFTC_UNLOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);
	return CAM_REQ_CMP;
    }

    /* if there is a current job, just update it */
    if(sws->flags & (SZ_ABORT_TAG_INPROG|SZ_ABORT_INPROG|SZ_TERMIO_INPROG))
    {
	/* CAM must be aborting an existing request */
	if(sjp == NULL)
	{
	    usimple_unlock(&shp->sh_lock);
	    SIM_SOFTC_UNLOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);
	    return CAM_REQ_CMP;
	}
	if(sws->flags & (SZ_ABORT_TAG_INPROG|SZ_ABORT_INPROG))
	{
	    SIM_PRINTD(shp->sh_ctlr->slot,sws->targid,sws->lun,
		(CAMD_INOUT|CAMD_FLOW|CAMD_ERRORS),
	        ("siop_sel_stop: request aborted\n"));
	    sjp->sj_abort = 1;
	}
	if(sws->flags & SZ_TERMIO_INPROG)
	{
	    SIM_PRINTD(shp->sh_ctlr->slot,sws->targid,sws->lun,
		(CAMD_INOUT|CAMD_FLOW|CAMD_ERRORS),
	        ("siop_sel_stop: request terminated\n"));
	    sjp->sj_term = 1;
	}

	/* make sure job hasn't gone yet */
	if(sjp->sj_myq == &shp->sh_outq)
	{
	    int i;
	    usimple_unlock(&shp->sh_lock);
	    SIM_SOFTC_UNLOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);
	    splx(oldpri);
	    i = ISTAT_SIGP;
	    i <<= 8;
	    SIOP_MD_CSR_WRITE(shp,CSR_BYTE1,SIOP_REG_ISTAT,i);
	    return CAM_REQ_CMP;
	}
    }
    else	/* this is a device reset message */
    {
	SIM_PRINTD(shp->sh_ctlr->slot,sws->targid,sws->lun,
	    (CAMD_INOUT|CAMD_FLOW|CAMD_ERRORS),
	    ("siop_sel_stop: resetting device\n"));
	sws->flags |= SZ_EXP_BUS_FREE;
    }

    usimple_unlock(&shp->sh_lock);
    SIM_SOFTC_UNLOCK(oldpri, (SIM_SOFTC *)sws->sim_sc);

    retval = siop_go(sws);

    return retval;
}


/* siop_chip_reset()
 *
 * 	args:	scp		pointer to SIM_SOFTC for this controller.
 *
 *	Perform SIOP reset on behalf of CAM.  Just call the internal
 *	routines do do the hard work.
 */

U32 
siop_chip_reset(scp) 
register SIM_SOFTC *scp;
{ 
    register SCRIPT_HOST *shp;
    SIM_MODULE(siop_chip_reset);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("scp = 0x%lx\n", scp ) );
    shp = (SCRIPT_HOST *)scp->hba_sc;
    shp->sh_intstatus = 0;
    shp->sh_jobdone = NULL;
    shp->sh_currentjob = NULL;

    siop_init(shp,1);		/* initialize the controller */

    /* The SIOP should now be ready to run SCRIPTS.  Set up the SCRIPTS
     * and go.
     */
    if(SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_LOAD,0))
	return NULL;

    return CAM_REQ_CMP;
}

/* siop_bus_reset()
 *
 *	args:	scp		pointer to SIM_SOFTC for bus to reset.
 *
 *	Reset the bus by aborting the SIOP.  SInce this is the only
 *	reason that an ABORT interrupt will be generated, the interrupt
 *	routine will cause the bus reset when the abort interrupt comes
 *	in.  The interrupt routine will also handle the removal of
 *	all active jobs.
 */
U32 siop_bus_reset(scp) 
register SIM_SOFTC *scp;
{ 
    register SCRIPT_HOST *shp;
    register U64 oldpri;
    SIM_MODULE(siop_bus_reset);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT,
	("scheduling bus reset via aborting SCRIPTS, scp = 0x%lx\n", scp ) );

    shp = (SCRIPT_HOST *)scp->hba_sc;

    oldpri = splsiop();
    shp->sh_flags |= SIOP_SH_ABORTED;
    SIOP_MD_CSR_WRITE(shp,CSR_BYTE1,SIOP_REG_ISTAT,ISTAT_ABRT<<8);
    splx(oldpri);

    return CAM_REQ_CMP;
}


U32 
siop_cam_init(scp) 
register SIM_SOFTC *scp;
{ 
    register SCRIPT_HOST *shp;
    extern scsi_bus_reset_at_boot;

    /*
     * Set the path inquiry flags to the peripheral driver can find
     * out our capabilities.
     */
    scp->path_inq_flags = PI_TAG_ABLE; /* we know how to do tags */

    shp = (SCRIPT_HOST *)scp->hba_sc;

    /* scsi_bus_reset_at_boot is declared in cam_data.c */
    siop_init(shp,scsi_bus_reset_at_boot);

    /* The SIOP should now be ready to run SCRIPTS.  Set up the SCRIPTS
     * and go.
     */
    if(SIOP_MD_SCRIPT_CNTL(shp,0,SIOP_SCRIPTS_LOAD,0))
	return NULL;

    return CAM_REQ_CMP; 
}

/***************************************************************************
 *			STUBS FOR UNUSED HBA ROUTINES		  	   *
 ***************************************************************************/

char siop_unused[] = "unused HBA routine called\n";

U32 siop_sm()
{
    SIM_MODULE(siop_sm);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_send_msg()
{
    SIM_MODULE(siop_send_msg);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP; 
}

U32 siop_xfer_info()
{
    SIM_MODULE(siop_xfer_info);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_req_msgout()
{
    SIM_MODULE(siop_req_msgout);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_clr_atn()
{
    SIM_MODULE(siop_clr_atn);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_msg_accept()
{
    SIM_MODULE(siop_msg_accept);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_setup_sync()
{
    SIM_MODULE(siop_setup_sync);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_discard_data()
{
    SIM_MODULE(siop_discard_data);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_targ_cmd_cmplt()
{
    SIM_MODULE(siop_targ_cmd_cmplt);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_targ_recv_cmd()
{
    SIM_MODULE(siop_targ_recv_cmd);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_targ_send_msg()
{
    SIM_MODULE(siop_targ_send_msg);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_targ_disconnect()
{
    SIM_MODULE(siop_targ_disconnect);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_targ_recv_msg()
{
    SIM_MODULE(siop_targ_recv_msg);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

U32 siop_unload()
{
    SIM_MODULE(siop_unload);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_ERRORS),(siop_unused));
    return CAM_REQ_CMP;
}

/***************************************************************************
 *			SIOP CAM ERROR LOGGER ROUTINES			   *
 ***************************************************************************/

/* siop_logger()
 *
 * siop interface to the CAM error logger.
 */
void
siop_logger(func, msg, flags, sc, sjp, spc)
U8 *func;
U8 *msg;
U32 flags;
SIM_SOFTC *sc;
SIOP_JOB *sjp;
U32 spc;
{
    register CAM_ERR_HDR hdr;
    static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];
    register CAM_ERR_ENTRY *entry;
    SIM_SOFTC *tssc;
    SIM_MODULE(siop_logger);

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,CAMD_ERRORS,("siop_logger: '%s' '%s'\n",func,msg));

    hdr.hdr_type = CAM_ERR_PKT;
    hdr.hdr_class = CLASS_SIOP;
    hdr.hdr_subsystem = SUBSYS_SIOP;
    hdr.hdr_size = 0;
    hdr.hdr_entries = 0;
    hdr.hdr_list = entrys;
    if (flags & SIM_LOG_PRISEVERE)
	hdr.hdr_pri = EL_PRISEVERE;
    else if (flags & SIM_LOG_PRIHIGH)
	hdr.hdr_pri = EL_PRIHIGH;
    else
	hdr.hdr_pri = EL_PRILOW;

    /*
     * Log the module name.
     */
    if (func != (U8 *)NULL) 
    {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STR_MODULE;
	entry->ent_size = strlen(func) + 1;
	entry->ent_vers = 0;
	entry->ent_data = func;
	entry->ent_pri = PRI_BRIEF_REPORT;
    }

    /*
     * Log the message.
     */
    if (msg != (U8 *)NULL) 
    {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STRING;
	entry->ent_size = strlen(msg) + 1;
	entry->ent_vers = 0;
	entry->ent_data = msg;
	entry->ent_pri = PRI_BRIEF_REPORT;
    }

    /*
     * Log the active script host structure
     */
    if (flags & SIM_LOG_HBA_SOFTC) 
    {
	if (sc->hba_sc)
	{
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIOP_SCRIPTHOST;
	    /* don't log the entire SCRIPTHOST structure since most of
	     * it is the activeq array which is 4K on Alpha and not
	     * needed by the error logger.
	     */
	    entry->ent_size = sizeof(SCRIPT_HOST) - 
		sizeof(((SCRIPT_HOST *)sc->hba_sc)->sh_activeq);
	    entry->ent_vers = SIOP_SH_VERSION;
	    entry->ent_data = (U8 *)sc->hba_sc;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the scsijob struct
     */
    if (flags & SIM_LOG_HBA_DME) 
    {
	if(sjp)
	{
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIOP_SIOPJOB;
	    entry->ent_size = sizeof(SIOP_JOB);
	    entry->ent_vers = SIOP_SJ_VERSION;
	    entry->ent_data = (U8 *)sjp;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the SCRIPTS PC if there was one.
     */
    if (flags & SIM_LOG_HBA_CSR) 
    {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STRUCT_UNKNOWN;
	entry->ent_size = sizeof(U32);
	entry->ent_vers = 1;
	entry->ent_data = (U8 *)&spc;
	entry->ent_pri = PRI_FULL_REPORT;
    }
    /*
     * Call sc_logger to log the common structures.
     */
    sc_logger(&hdr, SIM_LOG_SIZE, sc, sjp?sjp->sj_ws:0, flags);
}

/* routines for handling driver kernel thread.  This thread is used to
 * handle job completions off interrupt level.  This is also necessary
 * because the CAM SM layer can't be used directly so the spl's in
 * the cam code may be too low for a hardware interrupt.
 *
 * All completed requests are put on the Siop_doneq queue and are
 * de-allocated when the kernel thread runs.
 */

siop_thread()
{
    int s;
    thread_t thread = current_thread();
    register SIOP_Q *qp = &Siop_doneq;
    SIM_MODULE(siop_thread);

    thread_swappable(thread,FALSE);

    thread->priority = thread->sched_pri = BASEPRI_HIGHEST;

    (void)spl0();

    SIM_PRINTD(NOBTL,NOBTL,NOBTL,(CAMD_INOUT|CAMD_FLOW),("SIOP THREAD STARTED\n"));

    /*
     * Loop, processing jobs placed on our done queue.
     */
    for (;;) 
    {
	/*
	 * We'll wait again after we've processed all of the
	 * jobs on the done queue.
	 */
	SIOP_LOCK ( qp->siopq_lock, s );
	if ( qp->siopq_head == NULL )
	    assert_wait ( (vm_offset_t)siop_jobcomplete, FALSE );

	SIOP_UNLOCK ( qp->siopq_lock, s );

	/*
	 * Wait for job requests to complete.
	 */
	thread_block();

	PRINTD(NOBTL,NOBTL,NOBTL,CAMD_INOUT, ("siop_thread running...\n" ) );

	/*
	 * Process any jobs that have arrived.
	 */
	siop_jobcomplete();
    }
    /*NOTREACHED*/
}

siop_thread_init()
{
    extern task_t first_task;

    if(siop_thread_started)
	return 1;
    siop_thread_started = 1;

    (void) kernel_thread(first_task, siop_thread);
}
