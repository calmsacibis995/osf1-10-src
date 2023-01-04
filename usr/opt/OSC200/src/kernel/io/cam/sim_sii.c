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
static char *rcsid = "@(#)$RCSfile: sim_sii.c,v $ $Revision: 1.1.3.7 $ (DEC) $Date: 1992/09/29 14:31:42 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_sii.c                                    Dec. 31, 1991 

        The SIM_SII source file (sim_sii.c) contains functions which
        are specific to the SII SCSI host adapter.

Modification History:

	Date		Who	Reason
	01/10/91	rln	Get sim_ws for each interrupt in the HBA SM not
				the ISR
	
	12/31/91	rln	Comment and routine header cleanup.

	12/12/91	rln	Merge and Pathological Testing:

	1) Increase the MAX_BYTE_SPIN timeout and cleanup some bus_free 
	   handling in handle_interrupt.
	2) Deassert ATN only after REQ has been been asserted for multi byte 
	   message sequences.
	3) Initiate controller level error recovery if we have a NULL SIM_WS 
	   in simsii_sm and the sim_softc->error_recovery flag indicates an
	   error.

	12/04/91	rln	Fine tune some of the error logging calls to 
				reduce the size of the error log file.

	12/03/91	rln	Don't deassert ATN during XFER_INFO in. Solve
				the odd last byte of data phase problem.

	12/03/91	janet	Modified to use SIMSII_REG_VERS define.

        12/02/91        rln     Turn on logging of sim_softc.

        11/19/91        rln     First pass error logging. 

        11/12/91        rln     After a selection timeout, ignore the bus free 
				interrupt which follows the SII DISCON command.
	
	11/09/91	rln 	If clear_parity_error() fails then RESET the
	                        SCSI bus to force error recovery on the entire
                                bus. If we can't clear the IPE bit in the DSTAT
				it is dangerous to continue this I/O.

	11/07/91	rln 	In addition to reseting the SII chip in the 
				routine siireset(), reset the SCSI bus as well
				to clear the bus so dumps may be written.
	10/30/91	rln 	Assert the ATN signal on the bus after a 
	                        parity error and while flush bytes off of the
				the SCSI bus. Also remove cprintf during probe.
	10/22/91	rln 	PreEFT changes: 1) New ISR name (sii_intr) 
				2) simsii_sel, return CAM_BUSY rather than 
				reschedule reselected requests..
	10/10/91	rln 	First baselevel to be released to the group.
	7/20/91		rps	Moved spurious interrupt code to sim_config.c.
	7/1/91		rln	Removed references to camintr.
	3/6/91		rln	Created this file (Based on Janet's sim_94.c)
*/


/* ---------------------------------------------------------------------- */
/* Local SIM_SII defines.
 */

char *simidstring = "scsiid?";  /* string to search for                     */
#define CNTLR_INDEX     6       /* loc of controller char in the string     */
#define ASCII_0         0x30    /* add to binary # to get ACSII equivilent  */
#define PHASE_DISPATCH SC_SM	/* Create SII version of common function    */ 
#define SII_SEL_TMO	hz	/* Timeout period for selection once/second */
#define MAX_BYTE_SPIN 25000	/* Max time to spin transfering a byte	    */
#define ILLEGAL_PHASE 0x5	/* MSG and I/O, C/D Clear = Illegal SCSI phs*/
#define WAIT_REQ  MAX_BYTE_SPIN	/* Max time to wait for REQ to assert       */
#define FULLDEBUG 1
#define CAMERRLOG		/* SIM Error logging enabled		    */
#define SELECT_TIME 10*25000 	/* Time in ~us to wait for selection        */
#define ARB_WIN_TIME 500	/* Max time to spin to insure ARB started   */
/*#define trace_ints		/* Print out trace of interrupt calls	    */

/* ---------------------------------------------------------------------- */
/* Include files.
 */

#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <net/netisr.h>
#include <sys/buf.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/scsi_status.h>
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_logger.h>
#include <io/cam/scsi_all.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <machine/cpu.h>
#include <io/common/devdriver.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim_sii.h>
#include <io/cam/cam_errlog.h>

#include "sii.h"

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */




/* ---------------------------------------------------------------------- */
/* Function prototypes.
 */

U32 simsii_init();		/* Called during config to init the HBA      */
U32 simsii_go();		/* Start an I/O in the SIM		     */
U32 simsii_sm();		/* Called bi sim_sm state machine for intrpt */
U32 simsii_bus_reset();	/* HBA specific routine to reset the SCSI bus*/
U32 simsii_sel();		/* HBA specific routine to select a target   */
U32 simsii_xfer_info();	/* HBA specific routine to xfer bytes        */
U32 simsii_req_msgout();	/* HBA specific routine to assert ATN	     */
U32 simsii_clear_msgout();	/* HBA specific routine to deassert ATN      */
U32 simsii_send_msg();	/* HBA specific routine to send one msg byte */
U32 simsii_msg_accept();	/* HBA specific routine to assert ACK for msg*/
U32 simsii_setup_sync();	/* HBA specific routine to setup synch in HBA*/
U32 simsii_discard_data();	/* HSR to accept and discard incoming bytes  */
U32 simsii_send_cmd();	/* HSR to transfer command bytes             */
void simsii_sel_timeout();	/* Routine called to handle selection TMO    */
void sii_intr();		/* Interrupt Service Routine for the SII HBA */
void ini_brk();			/* Debugable breakpoint routine              */
void simsii_logger();		/* HSR to log HBA unique information         */
U32 simsii_reselect_interrupt(); /* Process reselection interrupts        */
void simsii_clear_selection();  /* Clear the SII's selection state	     */
U32 simsii_clear_parity_error(); /* Clear the SII's parity error condition*/
void simsii_add_sm_queue();	    /* Add an interrupt context softc queue  */
void simsii_error_recov();	/* Error recovery function		     */
U32 sii_simx_attach();	/* Config's SIM attachment to setup vectors  */
static SIM_WS *simsii_get_ws(); /* Function which converts, bus/id/lun to WS */


/* ---------------------------------------------------------------------- */
/* External Routines
 */

extern void scsiisr();
extern void timeout();
extern I32 cam_at_boottime();    /* During boot not everything is running */
extern void sim_kn01_chip_reset(); 
extern void untimeout();
extern void sc_setup_ws();
extern void sc_lost_arb();
extern void sc_sel_timeout();
extern void sim_err_sm();
extern SIM_WS *sc_find_ws();



/* ---------------------------------------------------------------------- */
/* External structures/variables
 */

extern int  scsiisr_thread_init;
extern U32 camdbg_flag;	/* PRINTD control flags 	      	*/
extern int hz; 	    		/* Number clock interrupts  per second	*/
extern U32  sm_queue_sz;	/* Global queue of interrupts		*/
extern SIM_SM sim_sm;
extern I32 scsi_bus_reset_at_boot; /* Whether to reset the bus at boot	*/
extern int shutting_down;


/* ---------------------------------------------------------------------- */
/* Global storage for use during debug.
 */
SIMSII_REG *gl_siireg;
SII_INTR *gl_siiintr;
SIMSII_SOFTC *gl_siihba_sc;
    
/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern SIM_SOFTC *softc_directory[];
extern void (*scsi_sm[SCSI_NSTATES][SCSI_NSTATES])();
extern CAM_SIM_ENTRY dec_sim_entry;
static void (*local_errlog)() = simsii_logger;

   
/* ---------------------------------------------------------------------- */
/* SII specific defines and macros
 */

#define CLEAR_COMM(reg) (reg)->comm = 0
#define SIM_LOG_SII_ALL \
   (SIM_LOG_SIM_SOFTC | SIM_LOG_NEXUS | SIM_LOG_SIM_WS | SIM_LOG_DME_STRUCT)

/*
 * Note this code is cloned from the DELAY macro in PARAMS.H.
 * It's used to delay a specific number of microseconds, however
 * since reads to I/O space are much (??) slower than memory reads,
 * all that is guaranteed, is that the delay will not be LESS THAN
 * the requested time.
 *
 * This macro must only be used for events which normally occur
 * quickly (order microseconds)!
 */

    
#define CAM_DELAY(usec,expression,retval) { U32 N = 3*(usec); \
(retval) = CAM_REQ_CMP; while ((--N > 0)&& !(expression)); if (N <= 0)\
 (retval) = CAM_REQ_CMP_ERR;}

/*    register int N = 3*(usec); \
	(retval) = CAM_REQ_CMP;\
	    while ((--N > 0)&& !(expression));\
		if (N <= 0) \
		    (retval) = CAM_REQ_CMP_ERR
	*/				       

#define SII_DISMISS_INTERRUPT(cstat,dstat,siireg,sim_softc,s) \
    /*\
     * Clear any interrupts that were received and reenable SII \
     * interrupts.\
     */\
    siireg->dstat = dstat;\
    siireg->cstat = cstat;\
    siireg->csr |= SII_CSR_IE;\
    WBFLUSH();\
    SC_UNLOCK(s, (SIM_SOFTC *)sim_softc);\
    /*\
     * We will need to count these later.. Perhaps these are stray.\
     */\


/*
 * If we are building a debuggable kernel, then don't use registers.
 */
#ifndef CAMDEBUG 
#define REGISTER  register 
#else
#define REGISTER 
#endif


/**
 * sim_get_scsid -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	Read the SCSI bus ID of this HBA from NV-RAM.
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * int cntlr	- SCSI Bus ID 
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	SCSI Bus ID of HBA
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/					   
int 
sim_get_scsiid( cntlr )
    int cntlr;			/* controller/bus number on the system */
{
    char *env;			/* ptr for the NVR string */
    int nvr_id;			/* converted ID # from NVR */

    SIM_MODULE(sim_get_scsiid);

    /* Build an id string from our controller #, i.e., scsiid0, 1, etc.  The 
    ID string can be reused. */

    simidstring[ CNTLR_INDEX ] = (char)((cntlr & 0xff) + ASCII_0);

    env = (char *)prom_getenv( simidstring );
    if (env != NULL) {
	nvr_id = xtob(env);		/* convert ACSII hex to binary */

	/* Is the ID a valid #, ID's on the SCSI bus can only be [0-7]. */
	if ((nvr_id >= 0) && (nvr_id <= 7)) {
	    return( nvr_id );
	}
    }
    
    /* The SCSI bus ID conversion failed, return the default value to be used
    for this controller. */


    return( 6 );		/* return the default */
}

/*
 * We need to figure out the best way to get this... This symbol should only
 * be defined in kn01.c in machine/mips...
 */
#define KN01SII_ADDR  PHYS_TO_K1(0x1a000000)/* phys addr of sii registers */

/**
 * simsii_logger -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function logs errors specific to the SII SIM modules.
 *
 * FORMAL PARAMETERS:  		
 *	u_char *func		calling module name, may be NULL
 *	u_char *msg		error description, may be NULL
 *	SIM_SOFTC *sc		SIM_SOFTC pointer, may be NULL
 *	SIM_WS *sws		SIM_WS pointer, may be NULL
 *	SIMSII_INTR *intr	SIMSII_INTR pointer, may be NULL
 *	U32 flags		flags described in sim.h
 *
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	NONE
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
void
simsii_logger(func, msg, flags, sc, sws, intr)
    u_char *func;
    u_char *msg;
    SIM_SOFTC *sc;
    SIM_WS *sws;
    SII_INTR *intr;

U32 flags;
{
    SIM_MODULE(simsii_logger);
    CAM_ERR_HDR hdr;
    static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];
    CAM_ERR_ENTRY *entry;
    SII_INTR *tintr;
    SIMSII_SOFTC *tssc;
    SIMSII_REG *tsiireg;
    int i;

    SC_ADD_FUNC(sws, module);

    bzero( &hdr, sizeof( CAM_ERR_HDR ));

    hdr.hdr_type = CAM_ERR_PKT;
    hdr.hdr_class = CLASS_SII;
    hdr.hdr_subsystem = SUBSYS_SII;
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
    if (func != (u_char *)NULL) {
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
    if (msg != (u_char *)NULL) {
	entry = &hdr.hdr_list[hdr.hdr_entries++];
	entry->ent_type = ENT_STRING;
	entry->ent_size = strlen(msg) + 1;
	entry->ent_vers = 0;
	entry->ent_data = msg;
	entry->ent_pri = PRI_BRIEF_REPORT;
    }
      
    /*
     * Log the active SII_INTR structure.
     */
    if (flags & SIM_LOG_HBA_INTR) {
	tintr = (SII_INTR*)NULL;
	if (intr != (SII_INTR*)NULL)
	    tintr = intr;
	else if (sc != (SIM_SOFTC *)NULL)
	    if (sc->hba_sc != (void *)NULL)
		tintr = ((SIMSII_SOFTC *)sc->hba_sc)->active_intr;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		if (sws->sim_sc->hba_sc != (void *)NULL)
		    intr =
			((SIMSII_SOFTC *)sws->sim_sc->hba_sc)->active_intr;
	if (tintr != (SII_INTR*)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SII_INTR;
	    entry->ent_size = sizeof(SII_INTR);
	    entry->ent_vers = SII_INTR_VERS;
	    entry->ent_data = (u_char *)tintr;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the SIMSII_SOFTC
     */
    if (flags & SIM_LOG_HBA_SOFTC) {
	tssc = (SIMSII_SOFTC *)NULL;
	if (sc != (SIM_SOFTC *)NULL)
	    tssc = (SIMSII_SOFTC *)sc->hba_sc;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		tssc = sws->sim_sc->hba_sc;
	if (tssc != (SIMSII_SOFTC *)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIMSII_SOFTC;
	    entry->ent_size = sizeof(SIMSII_SOFTC);
	    entry->ent_vers = SIMSII_SOFTC_VERS;
	    entry->ent_data = (u_char *)tssc;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the SIM SII device CSR's, if the flags say to do so and the
     * softc is not NULL.
     */
    if ( (flags & SIM_LOG_HBA_CSR) &&  (sc != (SIM_SOFTC *)NULL))
    {
	tsiireg = (SIMSII_REG *)sc->csr_probe;	/* Get addr of device CSR's */
	if (tsiireg != (SIMSII_REG *)NULL)
	{
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIMSII_REG;
	    entry->ent_size = sizeof(SIMSII_REG);
	    entry->ent_vers = SIMSII_REG_VERS;
	    entry->ent_data = (u_char *)tsiireg;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Call sc_logger to log the common structures.
     */
    sc_logger(&hdr, SIM_LOG_SIZE, sc, sws, flags);
}

/**
 * cam_sii_spurious_interrupt_before_initialization -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	In the event of an interrupt from the SII chip before the CAM subsystem
 * has been initialized, reset the SII chip.
 *
 * FORMAL PARAMETERS:  		controller -  SII controller number
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
cam_sii_spurious_interrupt_before_initialization(controller)
U32 controller;
{
    SIMSII_REG *siireg;
    
    SIM_MODULE(cam_sii_spurious_interrupt_before_initialization);

    if (controller == 0)
    {
	siireg = (SIMSII_REG *) KN01SII_ADDR; /* Get address of SII CSR */

	/*
	 * Clear interrupt enable on SII to allow system to boot, by disabling
	 * further SII interrupts.
	 */
	siireg->csr = 0;
	
	CAM_ERROR(module,
		   "Interrupt, CAM not initialized, interrupts now disabled.",
		   (SIM_LOG_PRISEVERE),NULL,NULL,NULL);
		   
    }
    else
    {
	
	CAM_ERROR(module,
		   "Interrupt, from unknown controller, interrupts now disabled.",
		   (SIM_LOG_PRISEVERE),NULL,NULL,NULL);
	panic("CAM: Spurious interrupt for unknown controller\n");
	
    };
    return(1);

};    /* cam_sii_spurious_interrupt_before_initialization  */


simsii_enable_interrupts(controller)
U32 controller;
{
    SIMSII_REG *siireg;
    
    SIM_MODULE(simsii_enable_interrupts);

    if (controller == 0)
    {
	siireg = (SIMSII_REG *) KN01SII_ADDR; /* Get address of SII CSR */

        /*
	 * Turn on interrupts.
	 */
	siireg->csr |= SII_CSR_IE;
	WBFLUSH();
    }
}


/**
 * sim_init_sii -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Called to initialize the SII chip. This routine must be called 
 * after a bus reset or power on. After this routine completes the SII will
 * post interrupts and accept commands from the balance of the HBA.
 *
 * FORMAL PARAMETERS:  		sim_softc - The softc for this hba.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
sim_init_sii(sim_softc)
    
    SIM_SOFTC* sim_softc;
    
{
    REGISTER SIMSII_SOFTC *hba_softc;
    U32 retval;			/* Return value    */
    SIMSII_REG *siireg;			/* SII CSR address */
    int s;

    SIM_MODULE(sim_init_sii);

    /*
     * Local initialization
     */
    hba_softc = (SIMSII_SOFTC *)sim_softc->hba_sc;
    retval = CAM_REQ_CMP;			/* Return value        */
    siireg = sim_softc->reg;			/* SII CSR address     */

    /* 
     * Reset chip first Don't lock since the chip reset functon
     * does it.
     */
    (hba_softc->chip_reset)(sim_softc);

    /*
     * SMP lock on this controller.
     */
    SC_LOCK(s, sim_softc);

    /*
     * Setup the SII Diagnostic Control Register to allow the SII to drive
     * the SCSI bus.
     */
    siireg->dictrl |= SII_DICTRL_PRE;
    
    /*
     * Setup the SII CSR to enable interrupts, reselection
     * events and enable parity checking.
     */
    siireg->csr    |= (SII_CSR_HPM|SII_CSR_RSE|SII_CSR_PCE|SII_CSR_IE);

    WBFLUSH();

    /*
     * Release SMP lock on this controller.
     */
    SC_UNLOCK(s, sim_softc);


    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("(sim_init_sii) Completed SII initialization\n"));

    return(retval);
};			/* sim_init_sii */


/**
 * simsii_probe -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simsii_probe() is called at boot time, once for each SCSI
 * bus based on the configuration information. This function is
 * responsible for allocating the SOFTC structure and placing it
 * in the softc_dir[].  The passed arguments are stored in the softc
 * for the next pass of the initialization call via (*sim_init)().
 *
 * FORMAL PARAMETERS:  		
 *
 *	caddr_t csr;
 *	BOP_PROBE_STRUCT *prb;
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:		CAM_TRUE if cntrl exist, CAM_FALSE otherwise
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE 
 *
 **/
U32
simsii_probe( csr, prb )
    caddr_t csr;
    struct controller *prb;
{
    SIM_SOFTC *sim_softc;
    extern char cam_ctlr_string[];

    SIM_MODULE(simsii_probe);
    PRINTD(prb->ctlr_num, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("(simsii_probe) begin\n"));

    /* 
     * Validate that this controller has not been previously probed.
     * If the CSR's match, just continue with the rest of the
     * initialization, there may be some PDrv configuration needed.  If
     * the CSR's do not match return failure to the system configuration
     * code.
     */

    if( softc_directory[prb->ctlr_num] != NULL )
    {
	CAM_ERROR(module,
		   "simsii_probe: controller already probed",
		   (SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
		   softc_directory[ prb->ctlr_num ],
		   NULL,NULL);

	if( csr != (softc_directory[prb->ctlr_num])->csr_probe )
	{
	    printf("simsii_probe: ERR ctlr %d already probed diff CSR 0x%x.\n",
		   prb->ctlr_num, csr );
	    return( 0 );		/* Signal that it is not there */
	}
    }
    else
    {
        softc_directory[prb->ctlr_num] =
            	(SIM_SOFTC *)sc_alloc(sizeof(SIM_SOFTC));

	if ( !softc_directory[prb->ctlr_num] )
	{
	      CAM_ERROR(module,
			"simsii_probe: memory allocation of softc failed",
			(SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
			softc_directory[ prb->ctlr_num ],
			NULL,NULL);
	}
	
    }

    sim_softc = softc_directory[prb->ctlr_num];

    sim_softc->hba_sc = (void *)sc_alloc(sizeof(SIMSII_SOFTC));

    if ( !sim_softc->hba_sc )
      {
	CAM_ERROR(module,
		  "simsii_probe: memory allocation of hba_softc failed.",
		  (SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
		  softc_directory[prb->ctlr_num],
		  NULL,NULL);

      }

    /* Update the probe storage fields in the softc structure. */

    sim_softc->csr_probe = csr;
    sim_softc->um_probe = (void *)prb;

    /*
     * Using the uba_ctlr structure and csr update the "normal" fields in the
     * softc.  These are "expected" in the rest of the initialization path.  
     */
    sim_softc->reg = csr;		/* base addr */
    sim_softc->cntlr = prb->ctlr_num;	/* path id */
    /*
     * Point to the CAM string in private area of controller structure so
     * that it can be identified as a CAM/SCSI controller later by peripheral
     * drivers.
     */
    prb->private[0] = (caddr_t) cam_ctlr_string;

    /*
     * scsi_bus_reset_at_boot is a flag that is used to reset the SCSI bus at
     * boot. If the flag is set the bus is reset during probe.
     *
     * A better solution to this would be to allow for turning off synchronous 
     * negotiation on a target by target basis.
     */
    if (scsi_bus_reset_at_boot != 0)
    {
	simsii_bus_reset(sim_softc);
    }
                                  
    /* 
     * Call the CDrv to complete the init path.  If the attachment failed
     * signal back to the system that the controller is not ready.  
     * The attachment process, allows a SIM to be included in the XPT 
     * dispatch table such that, peripheral driver may issue xpt calls to
     * to this SIM.
     */
    if(ccfg_simattach(&dec_sim_entry, prb->ctlr_num) == CAM_FAILURE)
    {
	return(0);
    }
    
    PRINTD(prb->ctlr_num, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	   ("(simsii_probe) end\n"));
    
    return(CAM_REQ_CMP);
}



/**
 * simsii_start_cntrlerr_recovery -
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 * 	This function currently logs an error and then resets the SCSI bus.
 * It should be called only in those cases of the most severe error conditions.
 * The purpose of this function is to start error recovery when the is no 
 * currently active I/O in the SIM.
 *
 * FORMAL PARAMETERS:  		
 *	SIM_SOFTC *sim_softc
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:              	CAM_REQ_CMP
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
    simsii_start_cntrlerr_recovery(sim_softc)
SIM_SOFTC *sim_softc;
{
    SIM_MODULE(simsii_start_cntrlerr_recovery);
    CAM_ERROR(module,"Initiate controller recovery",SIM_LOG_PRISEVERE,
	      sim_softc,NULL,NULL);

    /*
     * Since at this point controller level recovery doesn't exist in the SIM,
     * simply assert bus reset if we run into "can't" happen conditions
     *
     * Controller level recovery is needed when the SCSI bus or SIM is in an
     * error state other than a state where there is a particular I/O which is
     * active. The error recovery in the SIM needs to be extended to allow
     * error recovery to begin WITHOUT requiring an active and uptodate SIM_WS
     */
    (void) simsii_bus_reset(sim_softc);
    
    sim_softc->error_recovery &= ~ERR_UNKNOWN;

    return(CAM_REQ_CMP);
};

/**
 * simsii_attach -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called during configuration of the CAM subsystem. The
 * purpose of this function is to setup the HBA specific entry points for a SIM
 *
 * FORMAL PARAMETERS:  		
 * 	SIM_SOFTC *sim_softc 
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_attach( SIM_SOFTC *sim_softc )
    {
	SIM_MODULE(simsii_attach);
	sim_softc->hba_init = simsii_init;
	sim_softc->hba_go = simsii_go;
	sim_softc->hba_sm = simsii_sm;
	sim_softc->hba_bus_reset = simsii_bus_reset;
	sim_softc->hba_send_msg = simsii_send_msg;
	sim_softc->hba_xfer_info = simsii_xfer_info;
	sim_softc->hba_sel_msgout = simsii_sel;
	sim_softc->hba_msgout_pend = simsii_req_msgout;
	sim_softc->hba_msgout_clear = simsii_clear_msgout;
	sim_softc->hba_msg_accept = simsii_msg_accept;
	sim_softc->hba_setup_sync = simsii_setup_sync;
	sim_softc->hba_discard_data = simsii_discard_data;

    return(CAM_REQ_CMP);
    }

/**
 * simsii_init -
 *
 * FUNCTIONAL DESCRIPTION:
 *	simsii_init() is called during  boot time, once for each SCSI
 * bus based on the DEC SII.  This function is responsible for
 * initializing the DEC SII SIM and associated DME (data mover engine).
 *
 * FORMAL PARAMETERS:  		
 *	caddr_t csr		DEC SII CSR address
 *	SIM_SOFTC *sim_softc	SIM Software Control Structure
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_TRUE if the cntrl exists, 
 *				CAM_FALSE otherwise
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_init(sim_softc, csr)
REGISTER SIM_SOFTC *sim_softc;
void *csr;

{

    static int cntlr_cnt = 0;
    SIMSII_SOFTC *hba_sc;

    SIM_MODULE(simsii_init);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_init) begin\n"));

    /*
     * Determine the SCSI ID of this HBA on the SCSI bus.
     */
    sim_softc->scsiid = sim_get_scsiid(sim_softc->cntlr);

    /*
     * Set "hba_sc" pointer in SIM_SOFTC to the appropriate SIMSII_SOFTC.
     * Make sure that the SIMSII_SOFTC assignment is done only
     * once per controller.
     */
    if (!sim_softc->simh_init) {
	if (cntlr_cnt >= NSII)
	{

	    CAM_ERROR(module,
		       "(simsii_init) INVALID controller.",
		       (SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
		      sim_softc,NULL,NULL);

	    return(CAM_FALSE);
	}
	/*
	 * Indicate to the balance of the SIM that this HBA has been
	 * initialized.
	 */
	sim_softc->simh_init = CAM_TRUE;
    }

    /*
     * Set the DEC SII register pointer in the SIM_SOFTC;
     */
    sim_softc->reg = (void *)csr;


    /*
     * Set-up the machine specific functions.  This will be done via
     * an attach routine, for now (for ISV) only 3MAX is supported.
     */
    hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;


    /*
     * Now allocate and initialize the interrupt queue. This queue is HBA
     * specific and contains structures used to maintain the interrupt 
     * context. Each entry contains selected set of saved CSR from and 
     * this structure is normally associated with state machine queue entry.
     */
    CIRQ_INIT_Q(hba_sc->intrq);
    CIRQ_SET_DATA_SZ(hba_sc->intrq, SM_QUEUE_SZ);
    
    /*
     * Setup initial pointer to last entry in the interrupt array.
     */
    hba_sc->last_dismissed_sii_intr =  &hba_sc->intrq_buf[SM_QUEUE_SZ];


    /*
     * Setup the SII chip specific reset entry point.
     */
    hba_sc->chip_reset = sim_kn01_chip_reset;


    /*
     * Intialize the temporary working set which is used for interrupts
     * that occur when it's not clear which if any I/O the interrupt is for.
     */
    sc_setup_ws(sim_softc, &sim_softc->tmp_ws, 0, 0);

    /*
     * Reset the SII chip using an SII specific command.
     */
    (hba_sc->chip_reset)(sim_softc);

    /*
     * Perform SII specific initialization.
     */
    sim_init_sii(sim_softc);

    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_init) end\n"));

    return(CAM_TRUE);
}

/**
 * simsii_go -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called by ss_start() to initiate an I/O 
 * on this HBA. When this function returns the selection of a target 
 * will have begun.
 *
 * FORMAL PARAMETERS:  	       
 *	SIM_WS *sim_ws		CAM I/O control structure
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	Returned CAM status from simsii_sel()
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_go(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    U32 status;
    int s;

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_go) begin\n"));


    /*
     * Start the selection from a target off. If the target selects we will
     * get a TBE or IBF indicating that the target is REQuesting a byte.
     *
     * NOTE: Tagged commands are not currently supported.
     */
    status = simsii_sel(sim_ws);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_go) end, status is %x\n", status));

    /*
     * Return with the status from the selection command.
     */
    return(status);
}




/**
 * simsii_print_interrupts -
 *
 * FUNCTIONAL DESCRIPTION:
 *     This function is used during debug to parse csr's from an interrupt
 * and print out the interrupt context in textual form to the console.
 *
 * FORMAL PARAMETERS:  		
 *	SII_INTR *siiintr	Interrupt context
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	NONE
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
void
simsii_print_interrupt(siiintr)

REGISTER SII_INTR *siiintr;

{			/* Print_interrupt */

    int s;

/*
 * Determine first the general class of the interrupt and then handle
 * the details of that particular interrupt.
 * 
 * First determine if the I/O finished, then determine whether we have a
 * cstat or dstat interrupt, then handle either the cstat or dstat interrupts.
 */

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	   ("(print_interrupt) csr 0x%x, comm 0x%x, cstat 0x%x, dstat 0x%x\n",
	    siiintr->csr, siiintr->comm, siiintr->cstat, siiintr->dstat));


    if (siiintr->cstat & SII_CSTAT_CI)
    {			/* CI Interrupt */

	if (siiintr->cstat & SII_CSTAT_SCH)
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,("SCH with "));
	    
	    /* Now check for what type of sch interrupt */
	    if (siiintr->cstat & SII_CSTAT_DST)
	    {
		PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		       ("DST set, reselection, "));
	    }
	    else 
	    {
		PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,("DST clear, "));
	    }
	}
	
	/*
	 * Receipt of any of these bits should be treated as an error, logged
	 * and recovered from etc...
	 */
	else if (siiintr->cstat & ((SII_CSTAT_RST | SII_CSTAT_OBC | 
				     SII_CSTAT_BUF | SII_CSTAT_LDN)))
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("RST,OBC,BUF,LDN Interrupt, cstat %x\n",
		    siiintr->cstat));
	}

    }			/* CI Interrupt */

    /*
     * Now check for DSTAT related interrupts.
     */
    if (siiintr->cstat & ~SII_CSTAT_DI)
	
    {			/* DI Interrupt */
	
	if (siiintr->dstat & SII_DSTAT_MIS)
	{
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	       ("MIS (PHASE MIS), "));
	
	/*
	 * The phase in the COMM register doesn't match the current bus phase.
	 * We need to transition to a new bus phase.
	 */
	}

	if (siiintr->dstat & SII_DSTAT_DNE)

	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,("DNE, "));
	    
	}


	if (siiintr->dstat & SII_DSTAT_TBE)

	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("TBE (TRANS BUF EMPTY), "));
	
	}

	if (siiintr->dstat & SII_DSTAT_IBF)

	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("IBF (IN BUF FULL), "));
	
	}
	
	if (siiintr->dstat & SII_DSTAT_TCZ)
	    
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("TCZ (Trans Count 0), "));
	}

	if (siiintr->dstat & SII_DSTAT_OBB)
	    
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	   ("(print_interrupt OBB) csr 0x%x,comm 0x%x,cstat 0x%x,dstat 0x%x\n",
	    siiintr->csr, siiintr->comm, siiintr->cstat, siiintr->dstat));

	/*
	 * Figure out what to do here, perhaps the DME needs to be paused and
	 * the count adjusted for "lost" bytes.
	 */
	}



	if (siiintr->dstat & SII_DSTAT_IPE)
	    
	{
	    printf("IPE (Parity Error) Interrupt\n");
	    
	}

    }			/* DI Interrupt */

    else

    {			/* NEITHER CI OR DI ASSERTED */
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	       ("Neither a CI or DI interrupt, "));
    }; 			/* NEITHER CI OR DI ASSERTED */

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	   ("interrupt(s).\n"));

/*    camdbg_flag = 0x0;   */
};			/* Print_interrupt */



/**
 * simsii_handle_interrupt-
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is called to process each interrupt serviced by the HBA state
 * machine. This routine's primary purpose is to determine which interrupts
 * represent a state change on the bus and which do not. Those interrupts 
 * what represent a state change will return with siiintr->new_state= 1 those 
 * interrupts which don't represent a state change will return new_state =0.
 * 
 * If a state change has occured, it is expected that the caller of this 
 * routine will dispatch on this state change to a phase specific routine.
 *
 * FORMAL PARAMETERS:  		
 *	SII_INTR *siiintr 	- Current interrupt context.
 *	SIM_SOFTC *sim_softc	- SIM software control structure
 *	SIMSII_SOFTC *hba_sc	- HBA specific software control structure
 *	SIMSII_REG *siireg	- SII CSR's
 *	SIM_WS *sim_ws		- Current I/O's working set.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_handle_interrupt(siiintr,sim_softc,hba_sc,siireg,sim_ws)

REGISTER SII_INTR *siiintr;
REGISTER SIM_SOFTC *sim_softc;
REGISTER SIMSII_SOFTC *hba_sc;
REGISTER SIMSII_REG *siireg;
REGISTER SIM_WS *sim_ws;

{			/* handle_interrupt */
    int s;
    u_short sii_cmd;	/* The SII command word        */
    u_short sc1;	/* SII SCSI bus state register */
    U32 retval;	/* Status of bit spins         */
    u_short dstat_phase = (siiintr->dstat & SII_DSTAT_PHASE);


    SIM_MODULE(sii_handle_interrupt);
    SC_ADD_FUNC(sim_ws, module);

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	   ("(handle_interrupt) csr 0x%x, comm 0x%x, cstat 0x%x, dstat 0x%x\n",
	    siiintr->csr, siiintr->comm, siiintr->cstat, siiintr->dstat));


  /*
   * Has this sim_ws already completed, if so simply return, since there
   * are no other phases to process. We either got a MIS or a SCH interrupt
   * from the SII after the bus went free.
   * 
   * When the state machine accepts the command complete message, it 
   * set the SZ_CMD_CMPLT and SZ_EXP_BUS_FREE bits in the sim_ws flags 
   * field. If the we receive an interrupt and SZ_CMD_CMPLT is set and 
   * SZ_EXP_BUS_FREE is clear then this interrupt has occured after the I/O
   * completed. Otherwise, process it as any other interrupt.
   */
    if ((sim_ws->flags & SZ_CMD_CMPLT))
    {
	
	if  (!(sim_ws->flags & SZ_EXP_BUS_FREE))

	{			/* Command Complete */
	    /*
	     * This I/O must have already gone to bus free so, this interrupt
	     * is meaningless. Simply return and dismiss this interrupt.
	     */
	    siiintr->new_state= 0;

	    return;
	    
	}
	else
	    /*
	     * The I/O didn't complete, rather the bus just went free.
	     */

	{
	    
	    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, 
		   CAMD_PHASE,("(handle_interrupt) phase is scsi_bus_free\n"));

	    /*
	     * Set the current bus phase to an illegal phase, so that any
	     * phase transition will be processed.
	     */
	    hba_sc->last_int_phase = ILLEGAL_PHASE;

	    /*
	     * Since the bus has indeed gone free simply set the state of the
	     * SIM such that there is no active I/O. An clear the error 
	     * recovery state to prevent any other 
	     */
	    SC_LOCK(s,sim_softc);
	    sim_softc->active_io = NULL;
	    SC_UNLOCK(s,sim_softc);
	   
	    SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
    	    PHASE_DISPATCH(sim_ws);
	    siiintr->new_state= 0;

	    /*
	     * If the bus has gone free then return to caller.
	     */
	    return;
	    
	};
	
    };			/* Command Complete */


    /*
     * Now look at each of the possible interrupting conditions and decide
     * what should be done for each of them.
     *
     * First look for all the possible CI (connection) interrupts, then
     * look for any DI (Data) interrupts.
     */

    if (siiintr->cstat & SII_CSTAT_CI)
    {			/* CI Interrupt */

	if (siiintr->cstat & SII_CSTAT_SCH)
	{		/* SCH Interrupt checks */

	    /* Now check for what type of sch interrupt */
	    if (siiintr->cstat & SII_CSTAT_DST)
	    {
		/*
		 * DST means that we were the DeSTination of a selection.
		 */
		(void) simsii_reselect_interrupt(siiintr, 
					       sim_ws, siireg, sim_softc);

		/*
		 * Wait for IBF to transition to next phase, since the target
		 * may not have already gone to message phase.
		 */
		siiintr->new_state= 0;
	    }

	    else if ( !(siiintr->cstat & SII_CSTAT_CON))
		
	    {		/* Not CONNECTED (DST and CON clear), BUS FREE */
		
		PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, 
		       CAMD_PHASE,("(simsii_sm) phase is scsi_bus_free\n"));

		SC_LOCK(s,sim_softc);
		sim_softc->active_io = NULL;		
		SC_UNLOCK(s,sim_softc);

		/*
		 * Set the current bus phase to an illegal phase, so that any
		 * phase transition will be processed.
		 */
		hba_sc->last_int_phase = ILLEGAL_PHASE;

		/*
		 * After a failed selection attempt, the SII will interrupt 
		 * with a BUS FREE interrupt after the DISCON command has been
		 * issued to the SII. This interrupt should be ignored and not
 		 * passed through the state machine. If it were to be passed 
		 * through the state machine, then an errorneous unexpected 
		 * bus free would be logged.
		 */
		if (sim_ws != &sim_softc->tmp_ws)
		    
		{
		    SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
		    PHASE_DISPATCH(sim_ws);
		};

		/*
		 * If the bus has gone, free then return to caller.
		 */
		siiintr->new_state= 0;
		return;
	    }			/* Not CONNECTED */
	    
	    
	    /*
	     * If we are connected to a target and we are not the destination
	     * (DST not set) then it means that our selection completed, 
	     * assuming that we are trying to select..
	     */
	    else if ( (siiintr->cstat & SII_CSTAT_CON) && 
		     (sim_softc->flags & SZ_TRYING_SELECT))

	    {		/* Selection completed */
		
		
		sim_ws = hba_sc->sws;/* Make this I/O ACTIVE */

		PRINTD(sim_ws->cntlr,sim_ws->targid,sim_ws->lun,
		       CAMD_INOUT,
		       ("(simsii_sm) Selection completed, stop selection\n"));

		SC_LOCK(s,sim_softc);

		/*
		 * Clear the state such that selection has completed and 
		 * reselections might continue. Note that this routine clear
		 * ATN, but since the SII has a bug where it drops ATN if 
		 * selection completes, this is benign.
		 */
		simsii_clear_selection(sim_softc,hba_sc,siireg);

		sim_softc->active_io = sim_ws;     /* Make this I/O ACTIVE   */
		SC_UNLOCK(s,sim_softc);

		/*
		 * If sim_ws = NULL then this selection complete occured
		 * when no selection was in progress, start  error processing.
		 */
		if (sim_ws != NULL)
		{
		    /*
		     * Now that selection has completed,
		     * Set-up the synchronous offset and period.
		     */
		    simsii_setup_sync(sim_ws,sim_softc,siireg);		

		    /*
		     * Indicated that we have completed arbitration and
		     * selection and continue with next phase
		     */
		    SC_NEW_PHASE(sim_ws, SCSI_ARBITRATION);
		    SC_NEW_PHASE(sim_ws, SCSI_SELECTION);

		}
		else 	/* NULL sim_ws */
		{
		    /* Start error recovery */

		    CAM_ERROR(module,
			       "(handle_interrupt) NULL SIM_WS",
			       (SIM_LOG_SIM_SOFTC|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			      sim_softc,sim_ws,siiintr);

		    sim_softc->error_recovery |= ERR_UNKNOWN;
		    /* Perhaps we should count these */
		}

		/*
		 * Wait for TBE or IBF to start next phase transition.
		 * We must wait for IBF and TBE to insure that the phase
		 * has stabilized in the SII. If we don't wait we could
		 * dispatch on the wrong phase since the SII is sometimes
		 * slow in doing this.
		 */
		siiintr->new_state= 0;
		return; 
	    };		/* Selection completed */
	    
	}		/* SCH Interrupt checks */


	/*
	 * Receipt of any of these bits should be treated as an error, logged
	 * and recovered from etc...
	 */
        if (siiintr->cstat & ((SII_CSTAT_RST | SII_CSTAT_OBC | SII_CSTAT_BUF |
				SII_CSTAT_LDN)))
	{

	    CAM_ERROR(module,
		      "RST,OBC,BUF,LDN Interrupt",
		      (SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		      sim_softc,sim_ws,siiintr);

	    
	    /*
	     * Did the target disconnect from the bus? If so then don't 
	     * dispatch to a phase specific routine, simply return to the 
	     * state machine and set the active_io to be NULL.
	     *
	     * If a target drops off the bus, we should get a BER. If the 
	     * target completes an I/O request and returns a command complete
	     * message, the state machine (sim_sm) will set the SZ_CMD_CMPLT
	     * flag in this I/O's working set. In the case of BER  we simply 
	     * complete the I/O, dismiss the interrupt that started this 
	     * thread and reenable SII interrupts. 
	     *
	     * NOTE: BER is to be ignored as per recommendation of the chip
	     * designer. It appears, the BER is often set during normal 
	     * reselections sequences.
	     *
	     */
	    
	    PRINTD(sim_ws->cntlr,sim_ws->targid,sim_ws->lun, CAMD_PHASE,
		   ("(handle_interrupt) phase is scsi_bus_free\n"));
	    
	    /*
	     * No currently active I/O request.
	     */
	    SC_LOCK(s,sim_softc);
	    sim_softc->active_io = NULL;
	    SC_UNLOCK(s,sim_softc);

	    /*
	     * Set the current bus phase to an illegal phase, so that any
	     * phase transition will be processed.
	     */
	    hba_sc->last_int_phase = ILLEGAL_PHASE;
	    
	    SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
	    PHASE_DISPATCH(sim_ws);
	    siiintr->new_state= 0;
	    return;

	}		/* If error interrupt */

    }			/* CI Interrupt */

    /*
     * Check for parity errors before looking at other DSTAT conditions
     * If we are getting data from the target, then start error processing.
     */
	if (siiintr->dstat & SII_DSTAT_IPE)

	{

	    CAM_ERROR(module,
		       "SII Parity Error",
		       (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		       sim_softc,sim_ws,siiintr);

	    /*
	     * Increment the parity error count in the NEXUS.
	     */
	    sim_ws->nexus->parity_cnt++;

	    /*
	     * What phase is this parity error on?
	     */
	    if ( (dstat_phase) == SC_PHASE_MSGIN)
	    {
		sim_ws->error_recovery |= ERR_MSGIN_PE;
	    }
	    else if ( (dstat_phase) == SC_PHASE_DATAIN)
	    {
		sim_ws->error_recovery |= ERR_DATAIN_PE;
	    }
	    else if ( (dstat_phase) == SC_PHASE_STATUS) 
	    {
		sim_ws->error_recovery |= ERR_STATUS_PE;
	    }
	    else 
	    {
	  
		CAM_ERROR(module,
			   "Parity error when not in information IN phase",
			  SIM_LOG_PRISEVERE,sim_softc,sim_ws,siiintr);
		
		sim_ws->error_recovery |= ERR_PARITY;
	    }


	    /*
	     * Force the SII to clear the Parity Error condition
	     */
	    if (CAM_REQ_CMP != simsii_clear_parity_error(siireg, 
							 sim_softc,sim_ws))
	    {

		CAM_ERROR(module,
			  "Parity error condition failed to clear",
			  SIM_LOG_PRISEVERE,sim_softc,sim_ws,siiintr);

		/*
		 * Since we have been unable to clear the IPE, we have no
		 * choice but to reset the SCSI bus. This will force recovery
		 * in the SIM and prevent any possible data lose on the target.
		 */
		simsii_start_cntrlerr_recovery(sim_softc);

		/*
		 * The SCSI bus reset, will cause a state transition, thus
		 * ignore this interrupt and go on.
		 */
		siiintr->new_state = 0;	    
		return;
	    }

	    /*
	     * Since the IPE has been cleared in the SII. 
	     * Treat this as a new state, by setting new_state = 1.
	     */
	    siiintr->new_state = 1;	    
	    return;

	}		/* If Parity Error */

    /*
     * Now check for DSTAT related interrupts. The DSTAT register is the SII's
     * DATA TRANSFER STATUS REGISTER. It contains the phase of the SCSI bus as
     * well as the SII's state relative to the bus phase and data interactions
     * with the target device. 
     */
    if (siiintr->dstat & SII_CSTAT_DI)
	
    {			/* DI Interrupt */
	
	/*
	 * NOTE:
	 * Some device will select soo fast, that the SII might never
	 * see the selection complete, rather the first interrupt to 
	 * be seen will be a TBE interrupt for a message out byte.
	 * Thus, in those case we must clear any pending selection state
	 * that may still be in progress.
	 */
	if (sim_softc->flags & SZ_TRYING_SELECT)
	{		/* SZ_TRYING_SELECT */
	    
	    SC_LOCK(s,sim_softc);

	    
	    /*
	     * Clear the state such that selection has completed and 
	     * reselections might continue. Note that this routine clear
	     * ATN, but since the SII has a bug where it drops ATN if 
	     * selection completes, this is benign.
	     */
	    simsii_clear_selection(sim_softc,hba_sc,siireg);
	    
	    sim_softc->active_io = sim_ws;     /* Make this I/O ACTIVE   */

	    SC_UNLOCK(s,sim_softc);
	    
	    /*
	     * Now that selection has completed,
	     * Set-up the synchronous offset and period.
	     */
	    simsii_setup_sync(sim_ws,sim_softc,siireg);		
	    
	    /*
	     * Indicated that we have completed arbitration and
	     * selection and continue with next phase
	     */
	    SC_NEW_PHASE(sim_ws, SCSI_ARBITRATION);
	    SC_NEW_PHASE(sim_ws, SCSI_SELECTION);

	    /*
	     * Once all this has been done, then goo off and look
	     * the actual interrupt the SII is seeing now.
	     */

	}		/* If SZ_TRYING_SELECT */
	
	/*
	 * Since A DI interrupt, check for TBE and IBF interrupts 
	 * and then for MIS. Thus if we have an IBF/TBE and a MIS 
	 * interrupt, the IBF/TBE interrupt will be serviced first.
	 *
	 * WARNING:
	 * However, there is a problem with the SII where if MIS is set
	 * with IBF or TBE the phase in the dstat MAYBE wrong! 
	 */
	if (siiintr->dstat & SII_DSTAT_TBE) 
	    
	{		/* TBE interrupt */
	    /*
	     * TBE interrupts indicated that the target requested a byte
	     * and we didn't have one ready. TBE interrupts are useful for
	     * sending multiple byte message sequences or when targets makes
	     * rapid phase transitions, such that the MIS interrupt is often
	     * not received or received in such away the it must be ignored.
	     * As described above, there are cases where the phase information
	     * in the DSTAT register is wrong if both MIS and TBE are set.
	     * If we did ignore the "bad" MIS interrupt, then the 
	     * next interrupt would be a TBE, this TBE interrupt would contain
	     * valid phase information and should be used to move us to the
	     * next state.
	     *
	     * If this interrupt represents a new phase or a repeat of a msgout
	     * or dataout phase, then treat it as a new state otherwise, 
	     * dismiss it as a redundant interrupt.
	     *
	     */
	    
	    /*
	     * If we get TBE and we are in DATAOUT phase then, always treat 
	     * this as a new state. Since it's normal to get a repeated data
	     * phase without a MIS in between.
	     */
	    if (SC_PHASE_DATAOUT == dstat_phase)
	    {
		/*
		 * If we get TBE and we are in a data phase, then treat
		 * this as a new state.
		 */
		siiintr->new_state = 1;	    
		return;
	    }
	    
	    /*
	     * REQ must be asserted, now since we had TBE set, if it
	     * isn't then the phase must have changed. If the phase 
	     * hasn't changed and the phase from the interrupt and
	     * the current dstat are not the same then this interrupt
	     * should not be used to transition the state.
	     */ 
	    else if ((siireg->sc1 & SII_SC1_REQ) && 
		     ((siireg->dstat & SII_DSTAT_PHASE) == dstat_phase))
	    {
		siiintr->new_state = 1;	    
		return;
	    }
	    else	/* The phase must have changed */
	    {
		CAM_ERROR(module,
			  "SII TBE interrupt but phase changed",
			  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			  sim_softc,sim_ws,siiintr);
		
		siiintr->new_state = 0;
		return;
	    };
	    
	}			/* TBE interrupt */	
	
	
	/* 
	 * Now check IBF for a byte coming in.
	 * There is a problem with the SII where if MIS is set
	 * with IBF or TBE the phase in the dstat is wrong. This
	 * should no longer be occuring since the ISR converts MIS
	 * interrupts to either IBF or TBE interrupts.
	 */
	else if (siiintr->dstat & SII_DSTAT_IBF) 
	{
	    
	    /*
	     * IBF Interrupt -
	     *
	     * "Fast Reselection Workaround"
	     *
	     * If the input buffer is full and the active IO is NULL then this 
	     * is a fast reselection. If IBF and there is an active I/O simply
	     * dispatch on the current phase. This can happen in the case of
	     * a "fast" reselection. A fast reselection occurs when the Bus
	     * Free interrupt is not seen before the target reselects.
	     */
	    
	    if (sim_softc->active_io == NULL)
	    {
		
		/*
		 * If there is no active IO now, then this must have been a 
		 * fast reselection, where the SII never interrupts us 
		 * when it reselected. Now treat this as a "normal reselection"
		 */
		(void) simsii_reselect_interrupt(siiintr, 
						 sim_ws, siireg, sim_softc);
	    {
		/*
		 * Set the current bus phase to an illegal phase, so that
		 * any phase transition will be processed.
		 */
		hba_sc->last_int_phase = ILLEGAL_PHASE;
	    }
		
	    }		/* Active_io */
	    
	    /*
	     * If we get a IBF in data phase, then always treat this 
	     * as a new state, since it's "normal" to get multiple IBF's
	     * in data phase without a phase change (MIS).
	     */
	    if (SC_PHASE_DATAIN == dstat_phase) 
	    {
		/*
		 * If we get IBF and we are in a data phase, then treat
		 * this as a new state.
		 */
		siiintr->new_state = 1;	    
		return;
	    }
	    
	    /*
	     * REQ must be asserted, now since we had IBF set, if it
	     * isn't then the phase must have changed. If the phase 
	     * hasn't changed and the phase from the interrupt and
	     * the current dstat are not the same then this interrupt
	     * should not be used to transition the state.
	     */
	    else if ((siireg->sc1 & SII_SC1_REQ) && 
		     ((siireg->dstat & SII_DSTAT_PHASE) == dstat_phase))
	    {
		siiintr->new_state = 1;	    
		return;
	    }
	    else	/* The phase must have changed */
	    {
		CAM_ERROR(module,
			  "SII IBF interrupt but phase changed",
			  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			  sim_softc,sim_ws,siiintr);
		
		siiintr->new_state = 0;
		return;
	    };
	    
	}	/* IBF interrupt */
	else if (siiintr->dstat & SII_DSTAT_MIS)
	    
	    /*
	     * Although the SII specification indicates that MIS interrupts are
	     * synchronous with the assertion of REQ on the SCSI, this is NOT
	     * the case. Thus, ISR ignores MIS interrupts and waits for IBF or
	     * TBE interrupts before it dispatches an interrupt to the state
	     * machine. Therefore, we should never see an MIS interrupt in this 
	     * routine. We will simply log and ignore the MIS interrupts here.
	     */
	    
	{		/* Check for MIS */
	    
	    CAM_ERROR(module,
		      "SII MIS interrupt",
		      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		      sim_softc,sim_ws,siiintr);
	    
	    siiintr->new_state = 0; /* Ignore this interrupt */
	    return;
	    
	}	/* MIS interrupt */
	
	/*
	 * Check for DSTAT done interrupts
	 */
	
	if (siiintr->dstat & SII_DSTAT_DNE)
	    
	{
	    /*
	     * Was the data mover active? If it was, then we need to pause it.
	     * A DNE interrupt with the DME active means that the DME must have
	     * completed at least one part of a transfer. The assumption here
	     * is that when DNE is asserted, that the DMLOTC counter will be 
	     * stable (this assumption appears to be valid, rln.).
	     * Simply pause the DME now, wait for IBF or TBE to resume the 
	     * transfer.
	     */
	    if (sim_ws->flags & SZ_DME_ACTIVE)
	    {
		DME_PAUSE(sim_softc, &sim_ws->data_xfer);
	    };
	    
	    
	};
	
    }			/* DI Interrupt */
    
};			/* sii_handle_interrupt */

/**
 * simsii_phase_dispatch -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * 	This function is called by the simsii_sm "state machine". This function
 * will dispatch on the current bus phase to a phase specific routine.
 *
 * FORMAL PARAMETERS:  		
 *    	SII_INTR *siiintr	- Current interrupt context
 *	SIM_WS  *sim_ws		- Current I/O's state
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_phase_dispatch(siiintr, sim_ws)
    REGISTER SII_INTR *siiintr;
    REGISTER SIM_WS  *sim_ws;
{
    /*
     * Local storage and initialization
     */
    SIM_SOFTC  *sim_softc	= sim_ws->sim_sc;
    SIMSII_REG *siireg 		= sim_softc->reg;
    u_short sc1;				/* Snapshot of SC1 register */
    int s;
    u_char achar;
    U32 retval;


    SIM_MODULE(simsii_phase_dispatch);
    SC_ADD_FUNC(sim_ws, module);
    
    /*
     * Although this check should not be necessary, it has proved to be soo
     * useful during debug that it is best to keep it in the driver. Soo many
     * of possible failure modes result in this condition being meet that even
     * in production level code this check will be a some benefit for not 
     * much cost.
     */
    if ((siireg->dstat & SII_DSTAT_PHASE) != 
	(siiintr->dstat & SII_DSTAT_PHASE))
    {
	CAM_ERROR(module,
		  "Current SCSI bus phase doesn't match interrupt",
		  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		  sim_softc,sim_ws,siiintr);
	
	return(CAM_REQ_CMP);
    }


    /*
     * Dispatch on the saved SCSI bus phase. SWITCH on the three bus phase
     * lines saved in the DSTAT register.
     *
     * Each of the phase specific paths will record the new phase information
     * in the SIM_WS and then call the phase specific action routine, which
     * inturn invokes the HBA specific function.
     */
    switch (siiintr->dstat & SII_DSTAT_PHASE)

    {

    case SC_PHASE_DATAOUT:

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simsii_phase_dispatch) phase is scsi_dataout\n"));

	/*
	 * If data out phase, call sc_new_state() with a phase
	 * of scsi_dataout and then call sc_sm(). This will cause
	 * the DME to be resumed via sm_dataout.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_DATAOUT);
	PHASE_DISPATCH(sim_ws);
	break;
	
    case SC_PHASE_DATAIN:
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simsii_phase_dispatch) phase is scsi_datain\n"));
	/*
	 * if data in phase, call sc_new_state() with a phase of
	 * scsi_datain and then call sc_sm(). This will cause
	 * the DME to be resumed via sm_datain.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_DATAIN);
	
	PHASE_DISPATCH(sim_ws);
	break;
	
    case SC_PHASE_COMMAND:
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simsii_phase_dispatch) phase is scsi_command\n"));
	
	/*
	 * Record then phase, then call sm_command to transfer command 
	 * bytes.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_COMMAND);
	PHASE_DISPATCH(sim_ws);
	
	break;
	
    case SC_PHASE_STATUS:
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simsii_phase_dispatch) phase is scsi_status\n"));

	/*
	 * Block interrupts until status phase complete 
	 */
	SC_LOCK(s,sim_softc);

	/*
	 * Here we need to get the status and then return to the state 
	 * machine and get the command complete message.
	 */
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("does the state machine handle status phase correctly\n"));

	/*
	 * Here we will need to copy the SCSI status from the bus into the 
	 * appropriate cell. Assume that the xfer_info sets error recovery
	 * bits for parity errors.
	 */
	
	retval = simsii_xfer_info(sim_ws,&(sim_ws->scsi_status),1,CAM_DIR_IN);
	
	/*
	 * Determine if the xfer_info worked, if not start error recovery
	 */
	if (retval != CAM_REQ_CMP)
	{
	    /*
	     * Depending upon which error condition exists set, error_recovery.
	     * By setting error recovery the PHASE_DISPATCH will being error
	     * recovery.
	     */
	    if ( retval == CAM_UNCOR_PARITY)
	    {
		/*
		 * Xfer_info is likely to have set ERR_STATUS_PE already, but 
		 * let's be extra safe here.
		 */
		sim_ws->error_recovery |= ERR_STATUS_PE;
	    }
	    else
		{
		    /*
		     * Start general purpose error recovery.
		     */
		    CAM_ERROR(module,
			      "Error accepting SCSI status byte",
			      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			      sim_softc,sim_ws,siiintr);
		    
		    sim_ws->error_recovery |= ERR_UNKNOWN;
		}

	};		/* Xfer_info failed */

	/*
	 * Execute common STATUS phase and error processing.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_STATUS);
	PHASE_DISPATCH(sim_ws);
	
	SC_UNLOCK(s,sim_softc);
	
	break;				/* SC_PHASE_MSGIN */

    case SC_PHASE_MSGOUT:

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simsii_phase_dispatch) phase is scsi_msgout\n"));

	/*
	 * Call the state machine to actual transfer the message bytes.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_MSGOUT);
	PHASE_DISPATCH(sim_ws);
	break;

    case SC_PHASE_MSGIN:

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simsii_phase_dispatch) phase is scsi_msgin\n"));

	/*
	 * Before reading this new message insure that the parity is valid,
	 * if not start error recovery.
	 */
	if (!(siireg->dstat & SII_DSTAT_IPE))
	{

	    /*
	     * Since we are in message in, assume that there is a valid message
	     * byte pending on the bus. Read this byte into the SIM_WS message
	     * queue, without asserting ACK. 
	     */
	    SIMSII_GET_BYTE(achar,siireg);	/* Read message byte off bus */
	    SC_ADD_MSGIN(sim_ws,achar);		/* Copy the msg to SIM_WS    */

	}
	else
	{
	    CAM_ERROR(module,
		      "Parity error on message IN byte",
		      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		      sim_softc,sim_ws,siiintr);
	    sim_ws->error_recovery |= ERR_MSGIN_PE;
	}


        /*
	 * There should now be message byte(s) in the message in
	 * queue.  Call sc_new_state with a phase of scsi_msgin,
	 * then call sc_sm().
	 */
	SC_NEW_PHASE(sim_ws, SCSI_MSGIN);
	PHASE_DISPATCH(sim_ws);
	break;

    default:

	CAM_ERROR(module,
		  "Bad SCSI bus phase detected during phase dispatch",
		  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		  sim_softc,sim_ws,siiintr);
    }

    return(CAM_REQ_CMP);
};			/* simsii_phase_dispatch */



/**
 * simsii_sm -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called by scsiisr() (State machine)  to process an 
 * interrupt from the HBA. In general this function is called at low IPL, 
 * however this is not a requirement. This function will perform the HBA 
 * processing of an interrupt. Simsii_sm() first calls Sii_handle_interrupt()
 * to determine whether or not this interrupt represents a state change. If a
 * state change is detected then simsii_phase_dispatch() is called to handle
 * the new bus phase. This function is entered once for each interrupt 
 * generated by the SII.
 *
 * FORMAL PARAMETERS:  		
 *	SIM_SM_DATA *simsii_sm	- SIM State Machine interrupt context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_sm(simsii_sm)
    REGISTER SIM_SM_DATA *simsii_sm;

{
  
  REGISTER   SII_INTR *siiintr;
  REGISTER   SIM_WS *sim_ws;
  REGISTER   SIMSII_REG *siireg;
  REGISTER   SIMSII_SOFTC *hba_sc;
  REGISTER   SIM_SOFTC *sim_softc;
  U32 controller = NOBTL;
  int s;
  U32 retval;

  SIM_MODULE(simsii_sm);

  /*
   * Local initailization
   */
  siiintr = (SII_INTR*) simsii_sm->hba_intr;
  sim_softc = simsii_sm->sim_sc;
  siireg = SIMSII_GET_CSR(sim_softc);
  hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;
  

  PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_SM,
	 ("(SIMSII_SM) TARGID %D, LUN %D, SEQ_NUM 0X%X,\n",
	  sim_ws->targid, sim_ws->lun, sim_ws->seq_num));
  
  PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_SM,
	 (" CSR 0X%X, CSTAT 0X%X, DSTAT 0X%X\n",siiintr->csr,
	  siiintr->cstat, siiintr->dstat));
  
  /*
   * When we enter the state machine, save away the current state
   * machine context such that other parts of the SIM (i.e. DME) can
   * use this context (interrupt) information.
   *
   * Access to this field need not be locked since it is implicitly
   * synchronized by using the softnet interrupt.
   */
  
  sim_softc->active_interrupt_context = simsii_sm;
  
  
  /*
   * Get the SIM_WS associated with this interrupt. If there is an
   * active_io use that sim_ws, otherwise return the tmp_ws.
   */
  sim_ws = simsii_get_ws(sim_softc, hba_sc, siireg, siiintr->cstat, 
			 siiintr->dstat, siiintr->comm);

  SC_ADD_FUNC(sim_ws, module);

  /*
   * Print which interrupt has been received. Tracing the interrupts here 
   * rather than in the ISR has less and impact on performance since this
   * routine is executed at low IPL.
   */
#ifdef trace_ints
  simsii_print_interrupt(siiintr);
#endif 
    
  /*
   * Parse the interrupt's CSRs to determine what action to take in responce to
   * this interrupt. Handle_interrupt decodes the siiintr structure to
   * what the interrupting condition was and how to proceed from the interrupt.
   */ 
   (void) simsii_handle_interrupt(siiintr,sim_softc,hba_sc,siireg,sim_ws);
   
 
   /*
    * Reselection may change the active io therefore reassign sim_ws.
    */
    sim_ws = sim_softc->active_io;
			      

  /*
   * Handle interrupt decided when an interrupt was redundant or when it 
   * represents a real state change. 
   * One of the major difficultly with the SII is that it generates several
   * interrupts for each state transition on the bus. Since during some of
   * these interrupts the CSR's are not valid, what is attempted here is
   * to decide which interrupts represent a state change and have valid CSR's.
   *
   * In order for a phase change to be valid there must be an active_io, 
   * otherwise don't treat this interrupt as a state change.
   */
  if ((siiintr->new_state) && (sim_softc->active_io != NULL))
  {			/* New Phase */

      PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("*** State change handle phase %x, %x pending intr.\n",
		    (siiintr->dstat & SII_DSTAT_PHASE),
		    sim_sm.sm_queue.curr_cnt));

      /*
       * On the odd case where the target has changed phase during a data
       * transfer, pause the DME to save it's state. During a typical double
       * buffered DMA, DME_PAUSE would have already been called in response
       * to the DNE interrupt, however in the case of phase changes the DNE
       * maynot be received depending on the next state the SII enters and the
       * speed of the target device.
       */
      if (sim_ws->flags & SZ_DME_ACTIVE)
      {
	  DME_PAUSE(sim_softc, &sim_ws->data_xfer);
      };

      /*
       * This variable is used to record the bus phase of the last interrupt
       * in the format of the DSTAT register. Access to this cell is 
       * synchronized using the softclock thread as a synchronizer. 
       *
       * The variable is used to determine whether or not an interrupt really
       * represents a phase(state) change. If the phase of the current 
       * interrupt is the same as the last interrupt, then no phase change is
       * needed.
       */
      hba_sc->last_int_phase = siiintr->dstat & SII_DSTAT_PHASE;


      /*
       * Now that we have handled the original interrupting condition, 
       * dispatch on the current bus phase to a phase specific routine. 
       * There is one routine for each of the SCSI bus phases.
       */
      if ( 
	  simsii_phase_dispatch(siiintr, sim_ws) != CAM_REQ_CMP
	  )
      {
	  CAM_ERROR(module,
		    "Error processing bus phase",
		    (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		    sim_softc,sim_ws,siiintr);
	  /*
	   * An error has occured, that should never happen.
	   * Do the best we can to recover from this event.
	   */
	  simsii_start_cntrlerr_recovery(sim_softc);
      
      };
      
      /*
       * For debug keep a ptr to interrupt context of last interrupt.
       */
      hba_sc->last_sii_intr = siiintr;

  }			/* New Phase */
  else			     
  {			/* NO phase/state change */
      /*
       * Since this interrupt is not being treated as one that requires
       * a SCSI bus phase transition, ignore this interrupt. However, if this
       * interrupt has resulted in an error recovery being initiated, then
       * start error recovery now, also if this interrupt should have been
       * treated as a new_state (new_state = 1), start error recovery.
       */
      if ( (sim_softc->error_recovery & ERR_UNKNOWN) || siiintr->new_state)
      {  
	  simsii_start_cntrlerr_recovery(sim_softc);
      }
   
      hba_sc->last_dismissed_sii_intr = siiintr; /* Switch over last one */
  };


  /*
   * Reenable interrupts
   */
  SC_LOCK(s,sim_softc);

  siireg->csr |= SII_CSR_IE;
  WBFLUSH();
  SC_UNLOCK(s,sim_softc);


  PRINTD(NOBTL, NOBTL, NOBTL, CAMD_SM,("(simsii_sm) end\n"));

};



/**
 * simsii_reselect_interrupt -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is entered when an interrupt which represents a 
 * reselection is recieved. Due to the design of the SII there are actually
 * two ways the SII indicates a reselection:
 * 		1) CON & DST	- Normal Reselection
 * 		2) CON & IBF	- Fast Reselection
 *
 * This routine allows the reselecting devices to be made the ACTIVE I/O in
 * the SIM as well as causing any currently selection (out going) I/O's to be
 * rescheduled.
 * 
 *
 * FORMAL PARAMETERS:  		
 *
 *	SII_INTR *siiintr	- Current interrupt context.
 *	SIM_WS *sim_ws		- Current I/O's state.
 * 	SIMSII_REG *siireg	- SII CSR's.
 * 	SIM_SOFTC *sim_softc	- SIM software control structure.
 *
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
    simsii_reselect_interrupt(siiintr, sim_ws, siireg, sim_softc)

  REGISTER   SII_INTR *siiintr;
  REGISTER   SIM_WS *sim_ws;
  REGISTER   SIMSII_REG *siireg;
  REGISTER  SIM_SOFTC *sim_softc;
{
	      
  U32 controller = NOBTL;
  int s;
  U32 retval,msg_cnt;
  SIMSII_SOFTC *hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;


  SIM_MODULE(simsii_reselection_interrupt);
  SC_ADD_FUNC(sim_ws, module);

  PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	 ("(simsii_reselect_interrupt) phase is scsi_reselection\n"));
  
  /*
   * Handle the case where we were reselected while attempting a selection.
   * The selecting request (hba_sc->sws) must simply be rescheduled.
   * Remember to clear the trying selection flag.
   */
  if (sim_softc->flags & SZ_TRYING_SELECT)
    {
	SIM_WS *tmp_ptr;
	
	/*
	 * Get copy of the pointer to the "selecting" I/O.
	 */
	tmp_ptr = hba_sc->sws;

	/*
	 * Since we have been reselected, dismiss the timer and reshedule the
	 * request.
	 */
	untimeout(simsii_sel_timeout,tmp_ptr);

	/*
	 * Clear these fields before calling reschedule.
	 */
	SC_LOCK(s,sim_softc);
	sim_softc->flags &= ~SZ_TRYING_SELECT;
	hba_sc->sws = NULL;		/* For good measure clear this */
	SC_UNLOCK(s,sim_softc);

    
	/*
	 * Call ss_resched_request() to reschedule this request until the 
	 * bus goes free again.
	 */
	ss_resched_request(tmp_ptr);

    }

  /*
   * Since it's possible that we will receive the RESELECTION (DST)
   * interrupt before we receive the BUS FREE (CON false), it's possible
   * that we will be entering this routine with an active I/O in the 
   * SIM (sim_softc->active_io != 0). Thus we must fake this sim_ws into
   * thinking that we really did see a bus free interrupt before the 
   * reselection interrupt (Isn't the SII great!).
   *
   * The reason that this check is needed is that sometimes, multiple 
   * interrupting conditions will be delivered at the same time by the
   * SII. If a reselection is real fast, DNE and RESELECT interrupts
   * might be compressed into one interrupt. Thus here we must
   * handle that case and force the active_io to bus free now.
   */
  if (sim_softc->active_io != NULL)

  {		/* If not NULL active_io */
      
      /*
       * Set the current bus phase to an illegal phase, so that any
       * phase transition will be processed.
       */
      hba_sc->last_int_phase = ILLEGAL_PHASE;
      
      SC_NEW_PHASE(sim_softc->active_io, SCSI_BUS_FREE);
      PHASE_DISPATCH(sim_softc->active_io);

/*
  ***************************** 
  The phase dispatch may be replaced with a clear of the bus free 
  expected bit in the sim_ws. Removing this might increase performance
  *****************************
*/
      
      /*
       * Since the bus has indeed gone free simple set the state of the
       * SIM such that there is no active I/O.
       */
      SC_LOCK(s,sim_softc);
      sim_softc->active_io = NULL;
      SC_UNLOCK(s,sim_softc);

  }		/* If not NULL active_io */
  
    
  /*
   * Until we get the identify message we don't know which
   * working set to use. Not until we get the IDENTIFY message from
   * the target and extract the LUN can we know which SIM_WS this 
   * reselection is for. Therefore we will use the temporary 
   * working set in the sim_softc. The message handling code
   * for the identify message will flip to the correct sim_ws
   * when the message has been accepted from the target.
   */
  sim_ws = &sim_softc->tmp_ws;
  

  /*
   * Setup the temporary working set for use during this reselection.
   */
  sc_setup_ws(sim_softc, sim_ws,(U32) siireg->destat, 0L);

  /*
   * Now, dispatch on the fact that we have been reselected.
   */
  SC_NEW_PHASE(sim_ws, SCSI_RESELECTION);
  PHASE_DISPATCH(sim_ws);

  /*
   * The target went to message in with an identify message pending.
   * The following phase dispatch will receive the pending message
   * in byte.
   */
  return(CAM_REQ_CMP);

};		/* simsii_reselect_interrupt */



/**
 * simsii_bus_reset -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Perform a RESET of the SCSI bus, logs an error and resets the SII chip.
 *
 * FORMAL PARAMETERS:  		
 *	SIM_SOFTC *sim_softc	- SIM Software Control
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success
 *				CAM_REQ_CMP_ERR - Error
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32 
    simsii_bus_reset(sim_softc)
REGISTER SIM_SOFTC *sim_softc;
{
    REGISTER SIMSII_REG *siireg;
    REGISTER SIMSII_SOFTC *hba_softc;
    u_long retval;
    int s, s1;

    SIM_MODULE(simsii_bus_reset);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_bus_reset) begin\n"));

    siireg = SIMSII_GET_CSR(sim_softc);


    CAM_ERROR(module,
	      "HBA initiated SCSI bus reset",
	      SIM_LOG_PRISEVERE,sim_softc,sim_softc->active_io,NULL);

    /*
     * Raise ipl and smp lock.
     */
    SC_LOCK(s, sim_softc);
			      
    hba_softc = (SIMSII_SOFTC *)sim_softc->hba_sc;

    /*
     * If a selection is in progress clear it.....
     */
    simsii_clear_selection( sim_softc, hba_softc, siireg);

    /*
     * Reset the SII chip to a known state before we issue a bus reset.
     */
    sim_init_sii(sim_softc);

    /*
     * Assert reset on the scsi bus
     */
    siireg->comm |= SII_COMM_RST;
    WBFLUSH();

    /*
     * Since the reset line is not latched in the chip
     * We can be kept at high IPL for longer then the 25usec
     * the reset pulse is on the line..In other words we
     * can miss the reset interrupt.....
     * So just spin waiting for the it to be seen up to 25 usec's
     */
    CAM_DELAY( 25, (siireg->cstat & SII_CSTAT_RST), retval );

    /* 
     * If the event did not happen try it one more time
     */
    if (retval != CAM_REQ_CMP ){
	siireg->comm |= SII_COMM_RST;
	WBFLUSH();
	CAM_DELAY( 25, (siireg->cstat & SII_CSTAT_RST), retval );

	if( retval != CAM_REQ_CMP ){
	    CAM_ERROR(module, "HBA tried a SCSI bus reset, but couldn't",
		      SIM_LOG_PRISEVERE,sim_softc,sim_softc->active_io,NULL);

	    SC_UNLOCK(s, sim_softc);
	    return(retval);
	}
    }

    hba_softc->cnt_bus_resets++;

    /*
     * If we are recovering from a previous bus reset or performing a
     * dump, do nothing more for this one.
     */
    if (!(sim_softc->error_recovery & ERR_BUS_RESET) && !shutting_down)
    {	/* No ERROR recovery in progress */

	/*
	 * Set the SIM_SOFTC ERR_BUS_RESET bit in the SIM_SOFTC
	 * "error_recovery" field.  Make sure that the state machine
	 * scheduler is running.
	 */
	sim_softc->error_recovery |= ERR_BUS_RESET;

    	/*
    	 * Lock on the State Machine.
    	 */
    	SIM_SM_LOCK(s1, &sim_sm);

	/*
	 * Set the bus_reset bit in the state machine's struct.
	 */
	sim_sm.bus_reset |= (1 << sim_softc->cntlr);
	if (!sim_sm.sm_active) {
	    sim_sm.sm_active = CAM_TRUE;

	    /*
	     * Schedule the CAM state machine via the softnet
	     * interrupt.
	     */
	    SIM_SCHED_ISR();
	}

	/*
	 * Unlock the State Machine.
	 */
	SIM_SM_UNLOCK(s1, &sim_sm);
    }

    /*
     * Make sure reset is off the bus
     */
    CAM_DELAY( 25, !(siireg->cstat & SII_CSTAT_RST), retval );

    /*
     * Reset the SII chip to a known state after we issue a bus reset.
     */
    sim_init_sii(sim_softc);

    /*
     * Unlock and lower ipl.
     */
    SC_UNLOCK(s, sim_softc);
    
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_bus_reset) end\n"));

    return(CAM_REQ_CMP);
}


/**
 * simsii_clear_parity_error - 
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called to clear the parity error condition in SII.
 * NOTE: There is NO SAFE way to clear this condition in the SII. The
 * SII specification indicates that a transfer info command should be issued
 * to clear this condition. Issuing such a command runs the risk of corrupting
 * additional data or more often then not cause the current bus phase to 
 * change and cause the loss of message or status byte from the target.
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:		CAM_REQ_CMP_ERR	- IPE failed to clear.
 *				CAM_REQ_CMP)	- IPE cleared.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:	This function must be called at elavated IPL
 * 				with device locks held.
 *
 **/
U32 simsii_clear_parity_error(siireg,sim_softc,sim_ws)

REGISTER     SIMSII_REG *siireg;
REGISTER     SIM_SOFTC *sim_softc;
REGISTER     SIM_WS *sim_ws;
{
    u_short dmlotc;
    u_short sii_cmd;
    U32 retval;

    /*
     * First check to determine whether the parity error bit is
     * actualy still set, if not return success.
     */
    if (siireg->dstat & SII_DSTAT_IPE)
    {
	/* 
	 * Since the SII will only clear the Parity Error bit when a DMA
	 * command is issued to the chip, we will issue a DMA for ZERO bytes
	 * to the SII to clear the parity error bit in the DSTAT register.
	 */
	dmlotc = siireg->dmlotc;	/* Save original value */
	siireg->dmlotc = 0;
	WBFLUSH(); /* For write Buffer mergers */

	/*
	 * Issue a transfer info command with dma and the phase set to
	 * the current phase to start a DMA transfer. The SII requires that
	 * bits 0-2 of DSTAT and 4-6 of CSTAT be the same for the COMM reg
	 *
	 * During error recovery keep ATN asserted.
	 */
	sii_cmd = ((siireg->cstat &  SII_CSTAT_STATE) |
		   (siireg->dstat & SII_DSTAT_PHASE)|	
		   (SII_COMM_INFO_XFER | SII_COMM_ATN));

/* 
 * NOTE:
 * It is not acceptable to issue a transfer info command when 
 * recovering from a parity error. At this point we have decided
 * to simply reset the bus and go one. The SII specification indicates
 * that the way to clear the IPE bit is to issue some form of DMA operation.
 * There are several data integritty issues with this approach, thus we have
 * elected to simply leave this bit set and allow the caller to determine
 * how best to recover from this event.
 */
/*	CMD_PENDING(sim_softc->hba_sc, sii_cmd,siireg); */



	siireg->dmlotc = dmlotc;	/* Restore original value */

	siireg->comm |= SII_COMM_ATN;

	/*
	 * Before we lower IPL, clear any DNE interrupts that might be pending
	 * as the result of the DMA performed above. This is done by
	 * writing a 1 to the DNE bit in the DSTAT register.
	 */
	siireg->dstat =  SII_DSTAT_DNE;
	WBFLUSH();

	/*
	 * Did IPE clear, if not return error, otherwise return success.
	 */
	if (!(siireg->dstat & SII_DSTAT_IPE))
	{
	    return(CAM_REQ_CMP);
	}
	else
	{
	    return(CAM_REQ_CMP_ERR);
	}
    }		/* IPE Already Clear */
    else
    {
	return(CAM_REQ_CMP);
    }
}		/* simsii_clear_parity_error */


/**
 * simsii_clear_selection -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called to clear an active selection on the SII.
 * The HBA selection state is cleared, ATN is deassert and reselections 
 * are reenabled. 
 *
 * FORMAL PARAMETERS:  		
 * 	SIM_SOFTC *sim_softc	- SIM Software state.
 *	SIMSII_SOFTC *hba_sc	- HBA specific SIM state.
 *	SIMSII_REG *siireg	- SII CSR's.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      This function must be called at elevated IPL 
 *				with device locks held.
 *
 *
 **/
void simsii_clear_selection(sim_softc,hba_sc,siireg)
REGISTER     SIM_SOFTC *sim_softc;
REGISTER     SIMSII_SOFTC *hba_sc;
REGISTER     SIMSII_REG *siireg;
{


    SIM_MODULE(simsii_clear_selection);

    /* 
     * untimeout the selection thread if one is pending
     */
    if( hba_sc->sws != (SIM_WS *)NULL){
	untimeout( simsii_sel_timeout, hba_sc->sws);
    }

    /*
     * In one way or another we are done with the selection process.
     * Clear this flag to signal to the SIM that selection 
     * is complete.
     */			      
    sim_softc->flags &= ~SZ_TRYING_SELECT;
    sim_softc->active_io = NULL;   /* Insure that there is NO active IO */
    hba_sc->sws = NULL;		   /* Clear for good measure 		*/
 
    /*
     * Clear ATN before reenabling reselections, to prevent ATN from 
     * dangling during reselections.
     */
    siireg->comm &= (~SII_COMM_ATN);
    WBFLUSH();



    /*
     * It's safe now to enable reselections, since we know that we didn't lose
     * arbitration and that we weren't reselected. Thus there is no chance for
     * ATN to be dangling. If we reenable reselection prior to knowing this,
     * since there is a SELECT with ATN command in the COMM register, ATN might
     * be asserted by the SII when a target reselects us.
     *
     * NOTE: The SII won't assert ATN on the SCSI bus until the CON bit
     * in the cstat register is set and that clearing the RSE bit in CSR not
     * only disables the reselection interrupt, but also disables the SII from
     * responding to reselections.
     *
     * Reenable RESELECTION. 
     * This is a workaround for an SII problem which interacts with 
     * the RDAT to cause problems during reselection of the RDAT if
     * the SII was attempting to arbitrate and select a target while the
     * RDAT was reselecting. The "solution" was to always disable reselection
     * while the SII was selecting a target.
     */
    siireg->csr    |= (SII_CSR_RSE);
    WBFLUSH();
    
};

/**
 * simsii_sel -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called by ss_go() to start the selection of a target.
 * Selection is attempted only if the SIM is not currently selecting another
 * target and there is NO active I/O in the SIM.
 * This function during boot will wait for selection to complete before 
 * returning, however after boot a timeout is setup in case the selection 
 * fails.
 *
 * FORMAL PARAMETERS:  		NONE
 *	SIM_WS *sim_ws		- Currently active I/O's state.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	NONE
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
    simsii_sel(sim_ws)
REGISTER SIM_WS *sim_ws;

{			/* simsii_sel */
    REGISTER     u_char *cdb;
    REGISTER     SIM_SOFTC *sim_softc;
    REGISTER     SIMSII_REG *siireg;
    REGISTER     SIMSII_SOFTC *hba_sc;
    int s;
    U32 retval;		      
    u_short cstat,csr,tmp1_dstat, tmp1_cstat, tmp1_sc1, 
		tmp2_dstat, tmp2_cstat, tmp2_sc1;

    SIM_MODULE(simsii_sel);
    SC_ADD_FUNC(sim_ws, module);

    /*
     * Local setup of variables
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    siireg = SIMSII_GET_CSR(sim_softc);
    hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_sel) begin\n"));


    SC_LOCK(s,sim_softc);
    
	
    /*
     * Is sz_trying_select already set? If so rescehdule this request since
     * somehow we are already trying to select a device?
     */
    if (sim_softc->flags & SZ_TRYING_SELECT)
    {
	SC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    };

    /*
     * Did another target already get on the bus? If so, reschedule this 
     * request. The sim_softc->active_io is used by this SIM determine which
     * SIM_WS is active (in a cmd, data, message or status phase) in the SIM.
     */
    if (sim_softc->active_io != (SIM_WS *)NULL)
    {
	SC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    }


    /*
     * Setup ID of target to be selected on the SCSI bus.
     */
    siireg->slcsr = sim_ws->targid;

    /*
     * Save the address of the I/O (sim_ws) which is attempting to select a
     * target. During the selection the "active_io" is kept in the hba_sc.
     * This is done since there is NO guarantee that since we are about
     * select a target, that another target might have already reselected
     * or is about to reselect us.
     */
    hba_sc->sws = sim_ws;

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("Issue Select Command to SII.\n"));

    /*
     * NOTE 1: That we are always selecting with ATN.
     *
     * NOTE 2: There is a bug in the SII where it will deassert ATN
     * after selection completes, if we were selecting with ATN. The 
     * workaround is to always set ATN in the message processing code
     * where needed.
     *
     * Note 3: There is a bug in the SII where if were selecting with ATN
     * and we lose the selection, ATN will be left asserted. The workaround 
     * for this problem is to turn off reselection while selecting. This will
     * prevent the problem of us losing arbitration and a target reselecting
     * us and getting confused by the ATN signal beening asserted on the bus.
     * The TLZ04 exhibits an interesting behavior if this were to occur.
     */
    
    /*
     * Turn Off reselections before selecting. See Note 3 above.
     * Reselection will be reenabled when the selection completes or
     * it times out.
     */
    siireg->csr &= ~SII_CSR_RSE;
    WBFLUSH();

    /*
     * After we have disabled reselection, insure that we weren't reselected
     * just before we disabled it. If we were reselected, don't try to select.
     * If the bus has BSY or SEL is asserted, we must have already been 
     * reselected, thus don't try to select.
     */
    if (siireg->sc1 & (SII_SC1_SEL  | SII_SC1_BSY))
    {
	hba_sc->cnt_resched_sel++;
	simsii_clear_selection(sim_softc,hba_sc,siireg);
	SC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    };


    /*
     * Set the sim_softc "flags" field to the TRYING_SELECT state. This
     * prevents another selection from being started.
     */
    sim_softc->flags |= SZ_TRYING_SELECT;

    /*
     * We always SELECT with ATN. Now issue the SELECTION command to the SII.
     */
    CMD_PENDING(sim_softc->hba_sc,SII_COMM_SELECT_ATN,siireg);
    WBFLUSH(); 

    /*
     * NOTE:
     * Before we continue and wait for a timeout insure that the selection
     * actually started, wait around for the SII to at least start the command.
     * If the SII doesn't start the selection, then backoff and retry the 
     * selection since it's likely that we are being reselected during this
     * interval. If we don't wait for selection to start, it's possible that we
     * might either reenable reselections too soon or too late.
     *
     * This is necessary since the SII is slow in setting the LST (Lost arbit)
     * bit, during a selection attempt and we are reselected. Without this 
     * we might leave reselections disabled and cause a reselecting target to
     * timeout during it's reselections.
     */
    tmp1_cstat = siireg->cstat, tmp1_dstat = siireg->dstat, 
    tmp1_sc1 = siireg->sc1;
    CAM_DELAY(ARB_WIN_TIME,siireg->cstat & 
       (SII_CSTAT_SCH|SII_CSTAT_SIP|SII_CSTAT_LST|SII_CSTAT_DST|SII_CSTAT_CON),
	      retval);
    tmp2_cstat = siireg->cstat, tmp2_dstat = siireg->dstat, 
				tmp2_sc1 = siireg->sc1;

    /*
     * If arbitration/selection doesn't start then assume that we lost 
     * arbitration, clear the selection and retry later.
     */
    if (!(retval == CAM_REQ_CMP))
    { 
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("SIMSII_SEL: select didn't start, %x, %x\n",
		siireg->cstat,siireg->dstat));

	hba_sc->cnt_resched_sel++;
	simsii_clear_selection(sim_softc,hba_sc,siireg);
	SC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    }
    
    /*
     * Before going off and waiting for the selection to begin, first
     * determine that the selection has indeed started. 
     * If we have lost arbitration or we have been reselected, then retry
     * this selection later, since it never started.
     *
     * This code may not be needed any longer due to the CAM_DELAY above.
     */
    if ( (siireg->cstat & SII_CSTAT_LST) ||  (siireg->cstat & SII_CSTAT_DST))
    {
	hba_sc->cnt_resched_sel++;
	simsii_clear_selection(sim_softc,hba_sc,siireg);
	SC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);

    }	/* If we lost arbitration or were reselected */


    /*
     * Since timeouts are not possible during the boot sequence 
     * (SOFTCLOCK not running) we will need to spin waiting for selection
     * to complete. 
     * However, after boot we will use the timeout mechanism to timeout
     * selections that fail to complete.
     */
    if (cam_at_boottime())
    {

	/*
	 * Spin for SCH to be set, allow longer for selection than strickly
	 * required by the specificiation since some devices like RZ24 might
	 * be slow to respond.
	 */
	CAM_DELAY(SELECT_TIME,siireg->cstat & SII_CSTAT_SCH,retval);
	
	/*
	 * Now, check to determine whether a target selected. Here we spin 
	 * for the SCH interrupt bit indicating that a state change occured.
	 * If no state change occurrs, then treat this as a timed out 
	 * selection.
	 */
	if (retval !=CAM_REQ_CMP)
	{
	    /*
	     * Selection failed.
	     */
	    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
		   ("(simsii_sel) Selection completed, stop selection\n"));
	    
	    /*
	     * Simulate a timeout ocurring during the selection process.
	     */
	    simsii_sel_timeout(sim_ws);

	    SC_UNLOCK(s,sim_softc);
	    return(CAM_REQ_CMP);
	}
	
	/*
	 * Since interrupts are enabled, the selection completion notification
	 * will occur with a SCH interrupt from the SII.
	 */

    }
    else
    {			/* Not Boottime */
	/*
	 * Schedule a timeout to occur to catch the case where the target,
	 * doesn't select.
	 */
	(void) timeout(simsii_sel_timeout, sim_ws, SII_SEL_TMO);

    }			/* Not Boottime */


    SC_UNLOCK(s,sim_softc);
			    
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_sel) end\n"));


    return(CAM_REQ_CMP);
};   			/* simsii_sel */


/**
 * simsii_sel_timeout -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called by the timeout() code when a timeout occurs 
 * while attempting to select a target. 
 * 
 * FORMAL PARAMETERS:  		
 *    SIM_WS *sim_ws		- Selection I/O's context
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
void
simsii_sel_timeout(sim_ws)
    SIM_WS *sim_ws;

{
    SIM_SOFTC *sim_softc;
    SIMSII_SOFTC *hba_sc;
    REGISTER     SIMSII_REG *siireg;
    int s, s1;

    SIM_MODULE(simsii_sel_timeout);
    SC_ADD_FUNC(sim_ws, module);

    /*
     * Local setup of variables
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;
    siireg = SIMSII_GET_CSR(sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	   ("(simsii_sel_timeout) Selection failed, stop selection\n"));

    /*
     * Since the selection timed out, clear the flag indicating that a 
     * selection is in progress.
     */			      
    SC_LOCK(s,sim_softc);

    /*
     * If selection is still in progress, then DISCON the SII from the
     * bus.
     */
    if (siireg->cstat & SII_CSTAT_SIP)
    {
	CMD_PENDING(sim_softc->hba_sc,SII_COMM_DISCON,siireg);
	WBFLUSH(); 

    };

    /*
     * Clear the selection state, ATN and reenable RESELECTIONS 
     */
    simsii_clear_selection(sim_softc,hba_sc,siireg);

    SC_UNLOCK(s,sim_softc);

    /*
     * Call back the peripheral driver and let him know that a 
     * selection timeout occured.
     */
    
    /*
     * There is really no need for this to be a routine, since it's just
     * copying the  status.
     */
    sc_sel_timeout(sim_ws);
    
    /*
     * Now cause the I/O to be completed.
     */
    SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
    PHASE_DISPATCH(sim_ws);
};			/* simsii_sel_timeout */


/**
 * simsii_send_cmd -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called to send SCSI CDB bytes to a target when the 
 * the current SCSI bus phase is the command phase. 
 *
 * FORMAL PARAMETERS:  		
 *	SIM_WS *sim_ws		- Currently active I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_send_cmd(sim_ws)
REGISTER   SIM_WS *sim_ws;

{
    U32   retval;
    REGISTER SIMSII_REG *siireg;	/* SII csr's			   */
    REGISTER SIM_SOFTC *sim_softc;
    REGISTER u_char *cdb;

    SIM_MODULE(simsii_send_cmd);
    SC_ADD_FUNC(sim_ws, module);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("simsii_send_cmd:)entered\n"));

    /*
     * Local setup of variables
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    siireg = SIMSII_GET_CSR(sim_softc);

    /*
     * Determine if the cdb is a pointer, so that we can pass the actual
     * CDB starting address to xfer_info.
     */
    if (sim_ws->cam_flags & CAM_CDB_POINTER) 
	cdb = (u_char *)sim_ws->ccb->cam_cdb_io.cam_cdb_ptr;
    else
	cdb = (u_char *)sim_ws->ccb->cam_cdb_io.cam_cdb_bytes;

    /*
     * Send the command bytes to the target
     */
    retval = simsii_xfer_info(sim_ws, cdb,(U32)sim_ws->ccb->cam_cdb_len,
			      CAM_DIR_OUT);
    if (retval != CAM_REQ_CMP)
    {
	CAM_ERROR(module,
		   "Error during transfer of SCSI command bytes",
		   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,sim_ws,NULL);
	
	/*
	 * Start Error processing in command phase at next transition.
	 */
	sim_ws->error_recovery |= ERR_UNKNOWN;
    }

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("simsii_send_cmd:)exited\n"));
    return(retval);
};			/* simsii_send_cmd */

/**
 * simsii_xfer_info -
 *
 * FUNCTIONA LDESCRIPTION:
 *	simsii_xfer_info() will perform a scsi transfer (could be any
 * phase) bypassing the dme interface.  The data located at "buf" will be 
 * transfered using programmed I/O over the bus. The data transfers here use
 * the SII in a very low level mode, each byte is handshaked using programed 
 * I/O.
 *
 * FORMAL PARAMETERS:  		
 *	sim_ws *sim_ws		- Active I/O's context
 *	u_char *buf		- Source or destination buffer
 *	U32 cnt		- Number of bytes to move
 *	u_char dir		- Direction of data transfer
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/

U32
simsii_xfer_info(sim_ws, buf, cnt, dir)
    REGISTER SIM_WS *sim_ws;
    REGISTER u_char *buf;		/* pointer to local transfer point */
    REGISTER U32 cnt; 		/* number of bytes to transfer     */
    U32 dir; 			/* direction cam_dir_out/in 	   */


{
    REGISTER SIMSII_SOFTC *hba_softc;
    REGISTER u_char achar;
    REGISTER SIM_SOFTC *sim_softc;
    REGISTER SIMSII_REG *siireg;	/* SII csr's			   */
    int s;
    U32 bcnt;			/* Loop byte count */
    U32 i,retval;
    u_short sii_command;	    	/* Command to be sent to the SII   */
    U32 controller = NOBTL;

    
    SIM_MODULE(simsii_xfer_info);
    SC_ADD_FUNC(sim_ws, module);

    /*
     * Local initailization
     */
    sim_softc = sim_ws->sim_sc;
    siireg = SIMSII_GET_CSR(sim_softc);
    hba_softc = (SIMSII_SOFTC *)sim_softc->hba_sc;


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_xfer_info) begin\n"));

    /*
     * Raise ipl and smp lock.
     */
    SC_LOCK(s, sim_softc);

    /*
     * Decide the direction of the transfer, IN our OUT.
     */
    if (dir & CAM_DIR_OUT)
    {			/* Handshake bytes out */
	

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_xfer_info) Data OUT handshaking.\n"));

	/*
	 * Send all the bytes requested.
	 */
	for (bcnt = cnt; bcnt > 0; --bcnt)
	{			/* For send all bytes */
	    /*
	     * Wait for REQ to assert, if it hasn't already.
	     *
	     */
	    if (!(siireg->sc1 & SII_SC1_REQ))
	    {		/* REQ not asserted */

		CAM_DELAY(MAX_BYTE_SPIN,siireg->sc1 & SII_SC1_REQ,retval);
		
		/*
		 * If we didn't get REQ, error..
		 */
		if ( retval != CAM_REQ_CMP)
		{
		    CAM_ERROR(module,
			       "REQ failed to assert in handshake OUT",
			       (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			       sim_softc,sim_ws,NULL);
		    SC_UNLOCK(s, sim_softc);
		    return(retval);
		}
	    };		/* REQ not asserted */


	    /*
	     * Before transfering a byte of data insure that the phase has
	     * not changed. If it has log an error and return to the caller.
	     */
	    if (siireg->dstat & SII_DSTAT_MIS) 
	    {

		/*
		 * It may be "normal" for this to happen, if a target
		 * for example rejects a message out sequence from us.
		 * Thus, if we are in a message sequence don't log this
		 * as an error. Simpy return this as an error the caller
		 * and let it decide what to do.
		 */
		if (!(siireg->dstat & SII_DSTAT_MSG))
		{
		    /*
		     * Phase changed before transfer completed.
		     */
		    CAM_ERROR(module,
			      "Unexpected bus phase change in handshake OUT",
			      (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			      sim_softc,sim_ws,NULL);
		};
		SC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);
	    }
	

	    /*
	     * Do we need to clear ATN?
	     */
	    if (hba_softc->flags & SIMSII_CLR_ATN) {
		hba_softc->flags &= ~SIMSII_CLR_ATN;
		siireg->comm &= ~SII_COMM_ATN;
		WBFLUSH();
	    }

	    /*
	     * Write byte to data bus.
	     */
	    achar = *buf;
	    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		       ("Byte out %x. phase = %x\n",
			achar,(siireg->dstat & 7)));
	    SIMSII_PUT_BYTE(achar,siireg);
	    buf++;
	    

	    /*
	     * In order to start a transfer on the SII parts of the CSTAT and 
	     * DSTAT register must match the COMM register. Also, by doing 
	     * this we clear the phase mismatch bit. 
	     * 
	     * NOTE: That we are maintaining the state of the ATN signal as
	     * it is on the bus at this time. The caller of this function
	     * will determine when the ATN signal should be set or cleared.
	     * This routine will simply maintain the state of this signal(ATN).
	     */
	    sii_command = ((siireg->dstat & (SII_DSTAT_PHASE|SII_DSTAT_ATN)) |
			   (siireg->cstat & SII_CSTAT_STATE));
	    
	    /*
	     * Issue transfer without DMA for one byte. Note the bus phase
	     * in the COMM register must match the DSTAT register.
	     */
	    sii_command |= SII_COMM_INFO_XFER;

	    /*
	     * Before starting a programmed I/O transfer insure that DNE
	     * is clear. If this is not cleared then slow devices might trick
	     * us into thinking that the following xfer_info command completed
	     * when it didn't.
	     */
	    siireg->dstat = SII_DSTAT_DNE;

	    /* Issue the SII command built up in sii_command to the SII chip */
	    CMD_PENDING(sim_softc->hba_sc,sii_command,siireg);
	    WBFLUSH();
	    
	    /*
	     * Insure that the transfer info command completes.
	     * Wait for DNE (Done) to assert saying the operation
	     * completed.
	     */
	    CAM_DELAY(MAX_BYTE_SPIN,siireg->dstat & 
		      (SII_DSTAT_DNE|SII_DSTAT_MIS),retval);    


	    /*
	     * If DNE or MIS are asserted the target took the byte.
	     */
	    if (retval != CAM_REQ_CMP)
	    {
		CAM_ERROR(module,
			   "DNE not asserted after handshake OUT",
			   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);

		/*
		 * Enable SII interrupts
		 */
		siireg->csr |= SII_CSR_IE;
		WBFLUSH();
		SC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);

	    };
	    
	    
	};			/* For send all bytes */
	
    }   		/* Handshake bytes out */
    else
    {			/* Handshake bytes in */
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_xfer_info) Data IN handshaking.\n"));

	/*
	 * Insure that ATN is deasserted since we are already info IN
	 * phase. ATN asserted during data in my confuse some targets i.e.
	 * RDAT.
	 */
/*	siireg->comm &= ~SII_COMM_ATN;
	WBFLUSH();
*/
	
	/*
	 * Loop through and receive all the bytes requested.
	 */
	for (bcnt = cnt; bcnt > 0; --bcnt)
	{			/* For receive all bytes */


	    /*
	     * Wait for REQ to assert, if it hasn't already.
	     *
	     */
	    if (!(siireg->sc1 & SII_SC1_REQ))
	    {		/* REQ not asserted */
		CAM_DELAY(MAX_BYTE_SPIN,siireg->sc1 & SII_SC1_REQ,retval);

		/*
		 * If we didn't get REQ, error..
		 */
		if ( retval != CAM_REQ_CMP)
		{
		    CAM_ERROR(module,
			       "REQ failed to assert in handshake IN",
			       (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			       sim_softc,sim_ws,NULL);
		    SC_UNLOCK(s, sim_softc);
		    return(retval);
		};
	    };		/* REQ not asserted */

	    /*
	     * If after the first byte the phase has not changed, then continue
	     * to transfer bytes. Since the MIS bit is not valid until the COMM
	     * register is written and writing the COMM register will cause a
	     * byte to be transfered, the check for MIS is only valid after 
	     * the first byte.
	     */
	    if (siireg->dstat & SII_DSTAT_MIS)
	    {
		
		/*
		 * Phase changed before transfer completed.
		 */
		CAM_ERROR(module,
			   "Unexpected bus phase change in handshake OUT",
			   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);
	    

		/*
		 * Enable SII interrupts and return.
		 */
		siireg->csr |= SII_CSR_IE;
		SC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);
	    }
	

	    /*
	     * Read byte from the SCSI data bus.
	     */
	    SIMSII_GET_BYTE(achar,siireg);
	    *buf = achar;
	    buf++;

	    /*
	     * Before accepting this byte of data, check it's parity.
	     * If the parity is bad start error recovery.
	     */
	    if (siireg->dstat & SII_DSTAT_IPE)
	    {
		/*
		 * Parity error detected on incoming byte.
		 */
		CAM_ERROR(module,
			   "SII Parity Error",
			   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);

		/*
		 * Increment the parity error count in the NEXUS.
		 */
		sim_ws->nexus->parity_cnt++;

		/*
		 * What phase is this parity error on?
		 */
		if ((siireg->dstat & SII_DSTAT_PHASE) == SC_PHASE_MSGIN)
		{
		    sim_ws->error_recovery |= ERR_MSGIN_PE;
		}
		else if ((siireg->dstat & SII_DSTAT_PHASE) == SC_PHASE_DATAIN) 
		{
		    sim_ws->error_recovery |= ERR_DATAIN_PE;
		}
		else if ((siireg->dstat & SII_DSTAT_PHASE) == SC_PHASE_STATUS) 
		{
		    sim_ws->error_recovery |= ERR_STATUS_PE;
		}
		else
		{

		    CAM_ERROR(module,
			      "Parity error when not in information IN phase",
			      SIM_LOG_PRISEVERE,sim_softc,sim_ws,NULL);

		    sim_ws->error_recovery |= ERR_PARITY;
		}

		/*
		 * Force the SII to clear the Parity Error condition
		 */
		if (CAM_REQ_CMP != simsii_clear_parity_error(siireg, 
							     sim_softc,sim_ws))
		{
		    CAM_ERROR(module,
			      "Parity error condition failed to clear",
			      SIM_LOG_PRISEVERE,
			      sim_softc,sim_ws,NULL);
		    
		    /*
		     * Since we have been unable to clear the IPE, we have no
		     * choice but to reset the SCSI bus.
		     */
		    simsii_start_cntrlerr_recovery(sim_softc);
		}

		/*
		 * Enable SII interrupts
		 */
		siireg->csr |= SII_CSR_IE;
		
		SC_UNLOCK(s, sim_softc);
		return(CAM_UNCOR_PARITY);
	    };

	    /*
	     * In order to start a transfer on the parts of the CSTAT and 
	     * DSTAT register must match the COMM register. Also, by doing 
	     * this we clear the phase mismatch bit.
	     */
	    sii_command = (siireg->dstat & SII_DSTAT_PHASE) |
		(siireg->cstat & SII_CSTAT_STATE);
	    
	    /*
	     * Issue transfer without DMA for one byte. Note the bus phase
	     * in the COMM register must match the DSTAT register.
	     */
	    sii_command |= SII_COMM_INFO_XFER;
	    
	    /*
	     * Before starting a programmed I/O transfer insure, that DNE
	     * is clear.
	     */
	    siireg->dstat = SII_DSTAT_DNE;

	    /*
	     * NOTE:
	     * Sometimes the SII get's stuck and won't execute the next 
	     * command unless the previous command is cleared from the COMM 
	     * register. What we do here is clear the COMM register to reset
	     * the chip to a state where it will accept the next command.
	     * Since interrupts are now disabled, the clear of the COMM 
	     * register will have no adverse effects on the SII.
	     * 
	     * An example of when the SII get's stuck is on a msgin
	     * phase (0x02 - Save Pointers) after a reselection followed by a
	     * short datain phase.
	     */
	    siireg->comm = 0;
	    WBFLUSH(); /* For write Buffer mergers */

	    /* Execute this SII function */
	    CMD_PENDING(sim_softc->hba_sc,sii_command,siireg);
	    WBFLUSH();
	  
	    /*
	     * Insure that the transfer info command completes.
	     */
	    CAM_DELAY(MAX_BYTE_SPIN,siireg->dstat & 
		      (SII_DSTAT_DNE|SII_DSTAT_MIS|SII_DSTAT_IBF),retval);
	
	    /*
	     * If DNE or MIS are asserted, then we took the byte
	     */
	    if (retval != CAM_REQ_CMP)
	    {
		CAM_ERROR(module,
			  "DNE not asserted after handshake OUT",
			  (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			  sim_softc,sim_ws,NULL);

		/*
		 * Enable SII interrupts
		 */
		siireg->csr |= SII_CSR_IE;
		WBFLUSH();
		SC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);
	    };
	    
	};			/* For receieve all bytes */
	
    };			/* Handshake bytes in */
    

    /*
     * Before we lower IPL, clear any DNE interrupts that might be pending
     * as the result of the programmed I/O performed above. This is done by
     * writing a 1 to the DNE bit in the DSTAT register.
     */
    siireg->dstat =  SII_DSTAT_DNE;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_xfer_info) end\n"));

    return(retval);
}

/**
 * simsii_req_msgout -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called to assert the ATN signal on the SCSI bus.
 * The common SIM error recovery will call this function at the beginning of
 * a SIM error recovery sequence.
 *
 *
 * FORMAL PARAMETERS:  		
 *	SIM_WS *sim_ws		- Current I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_req_msgout(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    int s;
    REGISTER SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER SIMSII_REG *siireg = SIMSII_GET_CSR(sim_softc);

    SIM_MODULE(simsii_req_msgout);
    SC_ADD_FUNC(sim_ws, module);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_req_msgout) begin\n"));

    /*
     * Raise ipl and smp lock.
     */
    SC_LOCK(s, sim_softc);

    /*
     * Assert the ATN signal on the SCSI bus to force the target into
     * error recovery.
     */
    siireg->comm |= SII_COMM_ATN;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_req_msgout) end\n"));

    return(CAM_REQ_CMP);
}

/**
 * simsii_clear_msgout -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called to deassert the ATN signal on the SCSI bus.
 * The common SIM error recovery will call this function at the end of a SIM
 * error recovery sequence.
 *
 *
 * FORMAL PARAMETERS:  		
 *	SIM_WS *sim_ws		- Current I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success.
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_clear_msgout(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    int s;
    REGISTER SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER SIMSII_REG *siireg = SIMSII_GET_CSR(sim_softc);

    SIM_MODULE(simsii_clear_msgout);
    SC_ADD_FUNC(sim_ws, module);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_req_msgout) begin\n"));

    /*
     * Raise ipl and smp lock.
     */
    SC_LOCK(s, sim_softc);

    /*
     * Deassert the ATN signal on the SCSI bus.
     */
    siireg->comm &= ~SII_COMM_ATN;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_req_msgout) end\n"));

    return(CAM_REQ_CMP);
}		/* simsii_clear_msgout */

/**
 * simsii_send_msg- 
 *
 * FUNCTIONAL DESCRIPTION:
 *	simsii_send_msg() assumes that the current bus phase is
 * message out.  All bytes in the message out queue will be sent to the 
 * target one by one calling simsii_xfer_info.
 *
 * FORMAL PARAMETERS:  		
 *	SIM_WS *sim_ws		- Current I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_send_msg(sim_ws)
    REGISTER SIM_WS *sim_ws;
{
    int s;
    u_char achar;		      /* Hold message byte to send */
    REGISTER SIM_SOFTC *sim_softc;
    REGISTER SIMSII_REG *siireg; /* SII csr's */
    REGISTER SIMSII_SOFTC *hba_softc;
    REGISTER u_short i, count;
    
    SIM_MODULE(simsii_send_msg);
    SC_ADD_FUNC(sim_ws, module);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_send_msg) begin, sending %d bytes\n", count));

    /*
     * Local intialization
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    siireg = SIMSII_GET_CSR(sim_softc);
    count = SC_GET_MSGOUT_LEN(sim_ws);
    hba_softc = (SIMSII_SOFTC *)sim_softc->hba_sc;

    /*
     * Raise ipl and smp lock.
     */
    SC_LOCK(s, sim_softc);

    /*
     * Send the message bytes to the target one byte at a time.
     * This done in order to allow us to respond to message reject
     * or other phase changes.
     */
    for (i=0; i < count; i++) 
    {			/* For loop */
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_MSGOUT,
	       ("(simsii_send_msg) message out: 0x%x\n",
		SC_GET_MSGOUT(sim_ws, i)));
	
	/*
	 * Since we are sending a message out, we must keep 
	 * ATN assert until the last REQ of the last byte of the
	 * message sequence. 
	 * Now, we will clear ATN if this is the last message byte or
	 * insure that it is set if, we are in the middle of multiple byte
	 * message sequence.
	 */
	if (i >= (count - 1))
	{
	    hba_softc->flags |= SIMSII_CLR_ATN;
	}
	else
	{
	    /*
	     * Due to a problem in the SII, where during a selection with
	     * ATN the SII will clear ATN after the selection completed,
	     * insure that ATN is asserted when the first message byte is
	     * sent to the  target.
	     */
	    siireg->comm |= SII_COMM_ATN;
	    WBFLUSH();
	}
		
	/*
	 * Get the message byte about to be transfered.
	 */
	achar = (u_char) SC_GET_MSGOUT(sim_ws, i);

	/*
	 * Send this message byte and exit loop if an error occurs.
	 */
	if (simsii_xfer_info(sim_ws, &achar, 1, CAM_DIR_OUT) != CAM_REQ_CMP)
	    break;
	
	
	/*
	 * Insure that the phase doesn't change while sending message bytes
	 * out. If the phase does change, then simply record how many message
	 * bytes were actually sent and await the MIS interrupt for the phase
	 * change.
	 */
	if (siireg->dstat & SII_DSTAT_MIS) break;

	    
    }		/* For loop */
    
    /*
     * Insure that ATN is deasserted since all the message bytes have
     * been sent.
     */
    siireg->comm &= ~SII_COMM_ATN;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SC_UNLOCK(s, sim_softc);

    /*
     * The assumption here is that the caller will detect that not all the
     * message bytes have been sent and then recover. Recovery at this point is
     * will be difficult. The caller must clear this condition or it is
     * possible that we will continue to loop getting TBE interrupts!
     */
    SC_UPDATE_MSGOUT(sim_ws, (i == count) ? count : i+1 );


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_send_msg) end\n"));

    return(CAM_REQ_CMP);
}

/**
 * simsii_msg_accept -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simsii_msg_accept() cause the SII to issue an ACK for a pending message
 * byte on the SCSI bus. The SIM delays asserting ACK until the message pending
 * on the SCSI has been parsed and interpreted by the common message system in 
 * the SIM
 *
 * FORMAL PARAMETERS:  		NONE
 *	SIM_WS *sim_ws		- Current I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
/*U32*/

U32
simsii_msg_accept(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    u_char achar;
    REGISTER SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER SIMSII_REG *siireg = SIMSII_GET_CSR(sim_softc);
    U32 retval = CAM_REQ_CMP;
    int s;

    SIM_MODULE(simsii_msg_accept);
    SC_ADD_FUNC(sim_ws, module);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_msg_accept) begin\n"));

    /*
     * Local intialization
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;

    retval = simsii_xfer_info(sim_ws, &achar, 1, CAM_DIR_IN);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_msg_accept) end\n"));

    return(retval);
}


/**
 * simsii_setup_sync -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called to setup the REQ_ACK offset and synchronous
 * period for the SII. This information is retrieved from the it_nexus 
 * structure and must be setup before a selection or after a reselection goes
 * to data phase.
 *
 * FORMAL PARAMETERS:  		NONE
 *	SIM_WS *sim_ws		- Current I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_setup_sync(sim_ws)

REGISTER SIM_WS *sim_ws;

{
    int s;
    REGISTER SIM_SOFTC *sim_softc = 
	(SIM_SOFTC *)sim_ws->sim_sc;    
    REGISTER  SIMSII_REG *siireg = SIMSII_GET_CSR(sim_softc);


    SIM_MODULE(simsii_setup_sync);
    SC_ADD_FUNC(sim_ws, module);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	   ("[%d/%d/%d] (simsii_setup_sync) begin, period 0x%x, offset 0x%x\n",
	    sim_ws->cntlr, sim_ws->targid, sim_ws->lun,
	    sim_ws->it_nexus->sync_period, sim_ws->it_nexus->sync_offset ));
    
    SC_LOCK(s, sim_softc);

    /*
     * Set the DEC's SII synchronous offset and period.
     * Use "sync_offset" and "sync_period" from the IT_NEXUS.
     */
    /* sim_ws->it_nexus->sync_period, cannot be changed for the sii. */
    siireg->dmctrl = sim_ws->it_nexus->sync_offset;
    WBFLUSH();

    SC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("[%d/%d/%d] (simsii_setup_sync) end\n",
		sim_ws->cntlr, sim_ws->targid, sim_ws->lun ));

    return(CAM_REQ_CMP);
}

/**
 * simsii_discard_data -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is used during error recovery to read data from the SCSI
 * bus one byte at a time and throw it away. This function should be called 
 * with IPL high and SIM_SOFTC SMP locked.
 *
 * FORMAL PARAMETERS:  		NONE
 *	SIM_WS *sim_ws		- Current I/O's context.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	CAM_REQ_CMP	- Success
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_discard_data(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    REGISTER 	SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER	SIMSII_REG *siireg = SIMSII_GET_CSR(sim_softc);
    U32 retval = CAM_REQ_CMP;
    u_char achar;
    int s;
    u_short sii_command;	    	/* Command to be sent to the SII   */

    SIM_MODULE(simsii_discard_data);
    SC_ADD_FUNC(sim_ws, module);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_discard_data) begin\n"));


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	   ("(simsii_discard_data) Assert and Clear ACK.\n"));

    /*
     * Raise ipl and smp lock.
     */
    SC_LOCK(s, sim_softc);

    /*
     * If after the first byte the phase has not changed, then continue
     * to transfer bytes. Since the MIS bit is not valid until the COMM
     * register is written and writing the COMM register will cause a
     * byte to be transfered, the check for MIS is only valid after 
     * the first byte.
     */
    if (siireg->dstat & SII_DSTAT_MIS)
    {
	
	/*
	 * Phase changed before transfer completed.
	 */
	CAM_ERROR(module,
		  "Unexpected bus phase change discarding data",
		  SIM_LOG_PRISEVERE,
		  sim_softc,sim_ws,NULL);
	/*
	 * Enable SII interrupts
	 */
	siireg->csr |= SII_CSR_IE;
	WBFLUSH();
	SC_UNLOCK(s, sim_softc);
	return(CAM_DATA_RUN_ERR);
    }
    
    /*
     * In order to start a transfer on the parts of the CSTAT and 
     * DSTAT register must match the COMM register. Also, by doing 
     * this we clear the phase mismatch bit.
     */
    sii_command = (siireg->dstat & SII_DSTAT_PHASE) |
	(siireg->cstat & SII_CSTAT_STATE);
    
    /*
     * Issue transfer without DMA for one byte. Note the bus phase
     * in the COMM register must match the DSTAT register.
     */
    sii_command |= SII_COMM_INFO_XFER | SII_COMM_ATN;
    
    
    /*
     * Before starting a programmed I/O transfer insure, that DNE
     * is clear.
     */
    siireg->dstat = SII_DSTAT_DNE;
    
    /*
     * NOTE:
     * Sometimes the SII get's stuck and won't execute the next 
     * command unless the previous command is cleared from the COMM 
     * register. What we do here is clear the COMM register to reset
     * the chip to a state where it will accept the next command.
     * Since interrupts are now disabled, the clear of the COMM 
     * register will have no adverse effects on the SII.
     * 
     * An example of when the SII get's stuck is on a msgin
     * phase (0x02 - Save Pointers) after a reselection followed by a
     * short datain phase.
     */
    siireg->comm = 0;
    WBFLUSH(); /* FOR write buffer mergers */
    
    /* Execute this SII function */
    CMD_PENDING(sim_softc->hba_sc,sii_command,siireg);
    WBFLUSH();
    
    /*
     * Insure that the transfer info command completes.
     */
    CAM_DELAY(MAX_BYTE_SPIN,siireg->dstat & 
	      (SII_DSTAT_DNE|SII_DSTAT_MIS|SII_DSTAT_IBF),retval);
    
    /*
     * If DNE or MIS are asserted, then the we took the byte
     */
    if (retval != CAM_REQ_CMP)
    {
	CAM_ERROR(module,
		  "DNE not asserted during discard data",
		  (SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),sim_softc,sim_ws,NULL);

	/*
	 * Enable SII interrupts
	 */
	siireg->csr |= SII_CSR_IE;
	WBFLUSH();
	SC_UNLOCK(s, sim_softc);
	return(CAM_DATA_RUN_ERR);
    };
    
  
    /*
     * Before we lower IPL, clear any DNE interrupts that might be pending
     * as the result of the programmed I/O performed above. This is done by
     * writing a 1 to the DNE bit in the DSTAT register.
     */
    siireg->dstat =  SII_DSTAT_DNE;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simsii_discard_data) end\n"));

    return(retval);
    
};		/* simsii_discard_data */

/**
 * sii_intr -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This is the Interrupt Service Routine (ISR) for the SII CAM HBA.
 * This code is entered once for each hardware interrupt generated by the 
 * the SII. Except for a few special cases and workarounds for SII problems
 * this code main purpose is to snapshot the SII registers and place them in
 * a structure which can be queued to the state machines interrupt queue.
 * Then a software interrupt will be scheduled to wakeup the state machine
 * (SCSIISR) at low IPL to remove the interrupt context from this queue and
 * deliver this interrupt to the simsii_sm().
 *
 * FORMAL PARAMETERS:  		
 *	u_short controller	- Controller number for the SII on this system
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	NONE
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
void
sii_intr( controller ) /* Name changed for ISV release */
u_short controller;
{
    REGISTER SIM_SOFTC *sim_softc = SC_GET_SOFTC(controller);
    REGISTER SIMSII_REG *siireg; 
    REGISTER SIMSII_SOFTC *hba_softc;
    REGISTER u_short csr, cstat, dstat, comm, sc1, sdb, dmlotc;
    extern SIM_SM sim_sm;
    extern int netisr;
    int s1, s2, s3;
    U32 retval;
    u_short sii_comm;		/* Build up SII command */
    u_short orig_dstat;
    SIM_WS *tsws;
    DME_DESCRIPTOR *tdme;

    SIM_MODULE(sii_intr);
    PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
               ("(siiintr) begin\n"));


    /*
     * If there are interrupts pending from the SII before the CAM subsystem
     * has been initialized, then we may be called at this entry point to
     * process an interrupt before the data structures exist to process
     * this interrupt. At this time the only feasiable solution is to
     * dismiss the interrupt and disable interrupts until the subsystem has
     * initialized. 
     */
    if (sim_softc == NULL) 
    {
	cam_sii_spurious_interrupt_before_initialization(controller);
	return;
    }

    /* 
     * Lock ISR to prevent other interrupts from occuring
     */
    SC_LOCK(s1,sim_softc);

    /*
     * Setup local variables 
     */
    siireg = SIMSII_GET_CSR(sim_softc);
    
    /*
     * In the odd case that we disabled interrupts, but there was already one
     * latched, then simply return from the ISR.
     */
    if (!(siireg->csr & SII_CSR_IE))
    {
	CAM_ERROR(module,
		   "IE not enabled, but got interrupt",
		   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,NULL,NULL);
    	SC_UNLOCK(s1,sim_softc);
	return;
    }
    
    hba_softc = (SIMSII_SOFTC *)sim_softc->hba_sc;

    /*
     * Disable other SII interrupts.
     */
    siireg->csr &= ~SII_CSR_IE;
    WBFLUSH();


    /*
     * Read (Snapshot) and save the following SII registers:  
     */
    sc1 = siireg->sc1;
    sdb = siireg->sdb;
    csr = siireg->csr;
    cstat = siireg->cstat;
    orig_dstat = dstat = siireg->dstat;
    dmlotc = siireg->dmlotc;

    /*
     * NOTE: To workaround a problem in the SII where the OBB is cleared
     * when a MIS interrupt is cleared, create a "soft" copy of the OBB
     * in the DME structure. The DME_PAUSE and DME_END code will check this
     * bit to determine whether the transfer ended on an odd byte boundry.
     *
     * Without this software OBB bit, odd bytes might be lost if the OBB bit
     * was set at the end of data phase and MIS was set.
     */
    if (dstat & SII_DSTAT_OBB)
    {

	/* Get Active I/O */
	tsws = sim_softc->active_io;

	if (tsws != NULL) 
	{
	    if (tsws->flags & SZ_DME_ACTIVE)
	    {
		tdme = &tsws->data_xfer;

		/*
		 * Set the OBB bit for DME, this flag is used inlieu of the 
		 * the OBB bit in the DSTAT register.
		 */
		tdme->flags |= DME_OBB;
	    }
	}
    }


    PRINTD(controller, NOBTL, NOBTL, CAMD_INTERRUPT,
               ("(siiintr) csr 0x%x, cstat 0x%x, dstat 0x%x\n",
                csr, cstat, dstat));

    /*
     * NOTE: As per the recommendations of the chip designer. The BER interrupt
     * should be ignored, since during normal reselection sequences it is often
     * set.
     *
     * If we get BER alone, then clear the interrupt and proceed.
     *
     * By dismissing an interrupt at this point we never get to the
     * common sim state machine. Simply log this as an error interrupt and
     * proceed.
     *
     */
    if (cstat & SII_CSTAT_BER)
    {
	siireg->cstat = cstat;	/* Clear the bus error bit 	*/
	WBFLUSH();

	/*
	 * Someday we should count these...
	 */
    }
    
    
    /*
     * Determine whether this is a SCSI bus reset interrupt.
     * Just set the softc reset detected flag and schedule the
     * state machine.
     */
    if (cstat & SII_CSTAT_RST)
    {

	/*
	 * If the was a selection inprogress before the reset
	 * came in make sure the timeout thread is cleared by
	 * calling simsii_clear_selection. This will untimeout
	 * the thread if one is pending
	 */
	simsii_clear_selection(sim_softc, hba_softc, siireg);

	CAM_ERROR(module,
		   "SCSI bus reset detected",
		   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,NULL,NULL);

        /*
         * Increment the bus reset count in the SIM_SOFTC.
         */
        sim_softc->bus_reset_count++;

	/*
	 * After a SCSI bus reset is detected reinitialize the SII to a
	 * known state.
	 */
	sim_init_sii(sim_softc);

	/*
	 * Are we recovering from a previous bus reset?  If so,
	 * do nothing more for this one.
	 */
	if (!(sim_softc->error_recovery & ERR_BUS_RESET)) 

	{	/* No ERROR recovery in progress */

	    /*
	     * Set the SIM_SOFTC ERR_BUS_RESET bit in the SIM_SOFTC
	     * "error_recovery" field.  Make sure that the state machine
	     * scheduler is running.
	     */
	    sim_softc->error_recovery |= ERR_BUS_RESET;

	    /*
	     * Lock on the State Machine.
	     */
	    SIM_SM_LOCK(s3, &sim_sm);

	    /*
	     * Set the bus_reset bit in the state machine's struct.
	     */
	    sim_sm.bus_reset |= (1 << controller);
	    if (!sim_sm.sm_active) {
		sim_sm.sm_active = CAM_TRUE;

		/*
		 * Schedule the CAM state machine via the softnet
		 * interrupt.
		 */
		SIM_SCHED_ISR();
	    }

	    /*
	     * Unlock the State Machine.
	     */
	    SIM_SM_UNLOCK(s3, &sim_sm);
	}

    	SC_UNLOCK(s1,sim_softc);
	return;

    };

    /*
     * SII NOTE:
     * The SII chip designer indicted that IBF/TBE interrupts will not be 
     * latched unless the phase in the COMM register matches the DSTAT register
     * In order to use IBF/TBE for the first byte of a new phase, MIS must be
     * clear. If MIS is set then, assign the COMM register the same phase as 
     * the DSTAT register (thus clearing MIS) and then wait for IBF or TBE to 
     * be set before handling this interrupt.
     */
    while (dstat & SII_DSTAT_MIS)
    { 		/* While MIS set */

        /*
	 * If DNE is asserted, then there is no need to take the following 
	 * steps since, DMLOTC will be zero and we just need to clear MIS.
	 */
         if (!(dstat & SII_DSTAT_DNE))
	 {
	     /*
	      * Before attempting to clear the MIS interrupt, we must clear the
	      * dmlotc register and the comm register to fakeout the SII into
	      * asserting DNE. If these register are not cleared before we 
	      * write the new phase into the comm register the SII will 
	      * errorneously attempt to continue the previous transfer.
	      */
	     siireg->dmlotc = 0;	/* Clear out the DMA count register */
	     WBFLUSH();

	     siireg->comm = 0;		/* Cause DNE to be asserted         */
	     WBFLUSH();

	     /*
	      * Set the phase, to dismiss the MIS interrupt and to allow the
	      * TBE/IBF interrupts to be delivered. 
	      */
	     sii_comm = ((dstat & (SII_DSTAT_PHASE|SII_DSTAT_ATN)) | 
			 (cstat & SII_CSTAT_STATE));

	     siireg->comm = sii_comm;	/* Clear the MIS interrupt          */
	     WBFLUSH();

	     /*
	      * Restore the transfer count so that other parts of the SIM can
	      * determine how many bytes were sent.
	      */
	     siireg->dmlotc = dmlotc;	/* Restore the DMA count register   */
	     WBFLUSH();

	 }		/* DNE set   */
         else	
	 {		/* DNE Clear */
	     /*
	      * Set the phase, to dismiss the MIS interrupt and to allow the
	      * TBE/IBF interrupts to be delivered. 
	      */
	     sii_comm = ((dstat & (SII_DSTAT_PHASE|SII_DSTAT_ATN)) | 
			 (cstat & SII_CSTAT_STATE));

	     
	     siireg->comm = sii_comm;	/* Clear the MIS interrupt          */
	     WBFLUSH();

	 };		/* DNE Clear */

	/*
	 * One might think by reading the SII specification that the following
	 * code would not be necessary, however due to some targets changing 
	 * phase after asserting REQ and some timing problems with the SII, 
	 * we must loop waiting for MIS to clear. In general, this while loop
	 * will not be executed, since MIS will already be set. By waiting for
	 * MIS to clear we guarantee that IBF/TBE interrupt will be delivered.
	 */
	while (siireg->dstat & SII_DSTAT_MIS)
	{	/* While MIS */

	    siireg->comm = sii_comm;
	    WBFLUSH();

	    /*
	     * Someday count these...
	     */

	}	/* While MIS */

    
	/*
	 * After receiving a MIS interrupt and setting the bus phase in the 
	 * COMM register, either IBF or TBE must be set. Now given that MIS
	 * has been cleared, IBF or TBE should be set since it's likely that
	 * sometime through all of this REQ was asserted by the target.
	 * Normally, there is NO spin for IBF or TBE executed here just
	 * a one read of the CSR will occur.
	 */
	if (!(siireg->dstat & (SII_DSTAT_TBE|SII_DSTAT_IBF)))
	{

	    CAM_DELAY(MAX_BYTE_SPIN,siireg->dstat & 
		      (SII_DSTAT_IBF|SII_DSTAT_TBE|SII_DSTAT_MIS),retval);

	    if (retval != CAM_REQ_CMP)
	    {
		CAM_ERROR(module,
			   "IBF or TBE never set",
			   (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,NULL,NULL);
		
		/*
		 * An error has occured that should never happen.
		 * Do the best we can to recover.
		 */
		simsii_start_cntrlerr_recovery(sim_softc);

	    }
	    
	}	/* If not IBF||TBE */
	 
	 /*
	  * Before processing this interrupt insure that the BER bit is not 
	  * set. If BER is set, dismiss this interrupt and reenable interrupts.
	  * This may occur with devices like RDAT.
	  */
	if (!(siireg->cstat & SII_CSTAT_BER))
	{
	
	    /*
	     * Reread the DSTAT register so that we process the current state
	     * of this register, in case it has changed.
	     */
	    dstat = siireg->dstat;
	}
	else
	{	/* BER set */

	    CAM_ERROR(module,
		       "Bus ERROR Interrupt after MIS interrupt",
		       (SIM_LOG_SII_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		       sim_softc,NULL,NULL);
	    
	    /*
	     * Clear any interrupts that were received and reenable SII 
	     * interrupts.
	     */
	    SII_DISMISS_INTERRUPT(cstat,dstat,siireg,sim_softc,s1);
	    return;
	}	/* BER set */


    }		/* While MIS set */

    /*
     * If bus reset recovery is not in progress then schedule a softnet 
     * interrupt, otherwise simply set some flags and reenable interrupts.
     */
    if (!(sim_softc->error_recovery & ERR_BUS_RESET))
    {
	comm = siireg->comm;		/* Read the comm register */

	/*
	 * Clear the command outstanding in the SII.
	 * This mask will clear all bit other than ATN which can 
	 * effect the state of the SII chip.
	 */
	
	/*
	 * Lower IPL and get off the interrupt stack, queue the interrupt
	 * to the hba state machine. (sim_sm.c)
	 */
	
	/*
	 * Schedule the CAM state machine via the softnet interrupt. 
	 * Put the SIM_WS on the state machine's processing queue.
	 * If the state machine isn't already running, schedule it.
	 */
	SIM_SM_LOCK(s3, &sim_sm);

	simsii_add_sm_queue(sim_softc,csr,cstat,dstat,comm,sc1,sdb,
			    orig_dstat);

	if (!sim_sm.sm_active) 
	{
	    sim_sm.sm_active = CAM_TRUE;
	    
	    /*
	     * Schedule the CAM state machine via the softnet
	     * interrupt.
	     */
	    SIM_SCHED_ISR();
	}
	
	SIM_SM_UNLOCK(s3, &sim_sm);
    }

    /*
     * Clear any interrupts that were received and reenable SII interrupts.
     */
    siireg->dstat = dstat;
    siireg->cstat = cstat;
    WBFLUSH();

    SC_UNLOCK(s1,sim_softc);

    PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
	   ("(siiintr) end\n"));
    return;
}



/**
 * simsii_get_ws -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simsii_get_ws will determine the SIM_WS which is related
 * to the current interrupt.  Using the passed interrupt state this
 * routine will determine which SIM_WS this interrupt is for. If it is not 
 * possible to make this determination, the SIM_SOFTC's "tmp_ws" will be used.
 *
 * FORMAL PARAMETERS:  		
 * 	SIM_SOFTC *sim_softc;	- SIM software control structure.
 *	SIMSII_SOFTC *hba_softc	- HBA specific control structure.
 *  	u_short csr, cstat, dstat, comm 	- SII CSR's
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:    		SIM_WS	- Address of SIM_WS
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
static SIM_WS *
simsii_get_ws(sim_softc, hba_softc, csr, cstat, dstat, comm)

REGISTER SIM_SOFTC *sim_softc;
REGISTER SIMSII_SOFTC *hba_softc;
REGISTER u_short csr, cstat, dstat, comm;


{
    SIM_WS *sim_ws;
    int s;

    SIM_MODULE(simsii_get_ws);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_get_ws) begin\n"));
    
    /*
     * First check the sim_softc->active_io cell. If this is filled
     * then a request is current, simply return this sim_ws. If there
     * is no "active" request then return the temporary working set.
     */
    if (sim_softc->active_io != NULL) 

    { 		    	/* active_io == NULL */
	return(sim_softc->active_io);
    }
    else
	{
	    /*
	     * If we are attempting a selection and it completed successfully,
	     * then clear the selection and assign a new active IO. This must 
	     * be done in the ISR since there could be other interrupts coming
	     * from the SII during selection which might otherwise get the 
	     * wrong sim_ws assigned to it.
	     */
	    if ((sim_softc->flags & SZ_TRYING_SELECT) /* Selection ongoing */
		&&
		(cstat & SII_CSTAT_CON) 	      /* Connected target  */
		&&
		(!(cstat & SII_CSTAT_DST)))	      /* Not Reselected    */

	    {		/* During Selection */
		
		SC_LOCK(s,sim_softc);
		
		/*
		 * Setup address of active IO from the address of the IO
		 * which was attempting selection.
		 */
		sim_softc->active_io = hba_softc->sws;
		
		SC_UNLOCK(s,sim_softc);
		return(sim_softc->active_io);
	    }		/* During selection */
	    else
		
	    {		/* NOT during selection */
		/*
		 * Set the address of the active to be the temporary working
		 * set and initialize the sim_ws.
		 */
		SC_LOCK(s,sim_softc);
/*		sim_softc->active_io = &sim_softc->tmp_ws;*/

		/*
		 * Setup the temporary working set for use during this
		 * interrupt.
		 */
		sc_setup_ws(sim_softc, &sim_softc->tmp_ws, 0L, 0L);

		SC_UNLOCK(s,sim_softc);
		return(&sim_softc->tmp_ws);
	    }		/* NOT during selection */
	    
	} 		/* active_io != NULL */

}

/**
 * simsii_add_sm_queue -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simsii_add_sm_queue will store the SIM_WS, and relevant SII interrupt
 * information in the state machine's queue. This structure will be used to 
 * pass the interrupt context from the SII ISR to SCSIISR (state machine) and
 * finally the simsii_sm() (HBA state machine).
 *
 * FORMAL PARAMETERS:  		
 *
 *	SIM_SOFTC *sim_softc	-- SIM_SOFTC pointer
 *	u_short csr, cstat, 	-- SII CSR's 
 *	dstat, comm, sc1, sdb, 
 *	orig_dstat

 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
static void
simsii_add_sm_queue(sim_softc, csr, cstat, dstat, 
		    comm, sc1, sdb, orig_dstat)

REGISTER SIM_SOFTC *sim_softc;
REGISTER u_short 
    csr, cstat, dstat, comm, sc1, sdb, orig_dstat;

 {
    SIMSII_SOFTC *hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;
    extern SIM_SM sim_sm;
    SIM_SM_DATA *ssm;
    REGISTER SII_INTR *sintr;

    SIM_MODULE(simsii_add_sm_queue);

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_SM,
	   ("SIMSII_ADD_SM_QUE csr 0x%x, cstat 0x%x, dstat 0x%x,",
	    "comm 0x%x\n", csr, cstat, dstat, comm));

    /*
     * Get the needed space to store the interrupt data.
     */
    SIMSII_GET_INTR(hba_sc, sintr);

    /*
     * Save the csr, cstat, dstat, comm
     */
    sintr->csr 		= csr;
    sintr->cstat	= cstat;
    sintr->dstat	= dstat;
    sintr->comm 	= comm;
    sintr->sc1		= sc1;
    sintr->new_state	= 0;/* Assume no state change, until later */
    sintr->sdb		= sdb;
    sintr->orig_dstat	= orig_dstat;

    /*
     * Get space on the SIM State Machine's queue.
     */
    SC_GET_SM_BUF( &sim_sm, ssm );

    /*
     * Store the Interrupt data and the SIM_WS in this structure which
     * is already on the State Machine's queue.
     */
    ssm->hba_intr = (u_char *)sintr;
    ssm->sim_sc = sim_softc;


    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_SM,
	   ("hba_sc = %x, siiintr = %x, ssm = %x\n",hba_sc, sintr, ssm));

}

/**
 * simsii_error_recov-
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simsii_error_recov is used during controller error recovery
 * procedures (such as race conditions around a bus reset).
 *
 * FORMAL PARAMETERS:  	       
 * 	SIM_SOFTC *sim_soft	-- SIM software control structure
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
static void
simsii_error_recov(sim_softc)
SIM_SOFTC *sim_softc;

{
    SIMSII_REG *reg = SIMSII_GET_CSR(sim_softc);
    SIMSII_SOFTC *hba_sc = (SIMSII_SOFTC *)sim_softc->hba_sc;
    SIM_SM_DATA ssm;
    SIM_WS *sim_ws;
    SII_INTR *sintr;
    
    SIM_MODULE(simsii_error_recov);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_error_recov) begin\n"));

    panic("CAM: We should not get here yet\n");

    /*
     * Use the temp. SIM working set.
     */
    sim_ws = &sim_softc->tmp_ws;

    /*
     * Is this the first time that this routine has been called
     * since bus free?  If so, perform some setup.
     */
    if (!sim_softc->err_recov_cnt++) {
	sim_ws->sim_sc = (SIM_SOFTC *)sim_softc;
	sim_ws->nexus = &sim_softc->nexus[0][0];

	/*
	 * Make sure that attention is asserted.
	 * 
	 * Force the SII to assert attension on the bus
	 */


	/*
	 * Set the bus phase to bus free.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
    }

    /*
     * Set the error recovery bits.
     */
    sim_ws->error_recovery = sim_softc->error_recovery;


    /*
     * Setup th interrupt state as is done in ISR
     */

    /*
     * Setup the SIM State Machine structure.
     */
    ssm.hba_intr = (u_char *)sintr;

    /*
     * Call the DEC SII specific state machine to handle the current
     * phase.
     */
    simsii_sm(&ssm);

    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simsii_error_recov) end\n"));
}


/*
 *
 * Return Value :  None
 */

/**
 * simsii_unload-
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This routine is not used at this time. It's intention is to
 * be used when a driver is unloaded.
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	1
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simsii_unload()
{
    SIM_MODULE(simsii_unload);
    return(1);
}


/**
 * ini_brk-
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is used during debug to set breakpoints in the CAM
 * driver. To use this add a call to this function in the driver then boot
 * the kernel with DBX and set a break in ini$brk.
 *
 * FORMAL PARAMETERS:  		
 *	u_char * cptr		-- Message string to be printed when call.
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	NONE
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
/*U32*/
void ini_brk(cptr)
u_char * cptr;
{
     SIMSII_REG *siireg;

     siireg = (SIMSII_REG *) KN01SII_ADDR; /* Get address of SII CSR */

     printf("CAMBKPT: %s,cstat = %x, dstat = %x\n",
	     cptr,siireg->cstat,siireg->dstat);
};




/**
 * siireset -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function exists for compatibility reasons with /sys/crashdump.c.
 * This entry point is called to reset the SCSI bus in the event a hung bus
 * during a system crash.
 *
 * FORMAL PARAMETERS:  		NONE
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:        	TBD
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
void siireset()
{
    SIM_MODULE(siireset);
    int controller = 0;

    /*
     * Issue a SCSI bus reset before starting to write a crash dump.
     * WARNING: this only works for systems with a single
     * controller.
     * To make this work for multiple controllers, we will need
     * to pass the controller number from the caller down to this
     * routine.
     */
    (void) simsii_bus_reset(softc_directory[controller]);

    /*
     * Issue a chip reset to the SII chip.
     *
     */
    sim_kn01_chip_reset(softc_directory[controller]);
};
