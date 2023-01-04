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
static char *rcsid = "@(#)$RCSfile: sim_kzq.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 17:51:27 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_kzq.c                                    Dec. 31, 1991 

        The SIM_KZQ source file (sim_kzq.c) contains functions which
        are specific to the KZQ SCSI host adapter.

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
#include <io/dec/uba/ubavar.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim_kzq.h>
#include <io/cam/cam_errlog.h>

#include "kzq.h"


/* ---------------------------------------------------------------------- */
/* Function prototypes.
 */

U32 simkzq_init();		/* Called during config to init the HBA      */
U32 simkzq_go();		/* Start an I/O in the SIM		     */
U32 simkzq_sm();		/* Called bi sim_sm state machine for intrpt */
U32 simkzq_bus_reset();		/* HBA specific routine to reset the SCSI bus*/
U32 simkzq_sel();		/* HBA specific routine to select a target   */
U32 simkzq_xfer_info();		/* HBA specific routine to xfer bytes        */
U32 simkzq_req_msgout();	/* HBA specific routine to assert ATN	     */
U32 simkzq_clear_msgout();	/* HBA specific routine to deassert ATN      */
U32 simkzq_send_msg();		/* HBA specific routine to send one msg byte */
U32 simkzq_msg_accept();	/* HBA specific routine to assert ACK for msg*/
U32 simkzq_setup_sync();	/* HBA specific routine to setup synch in HBA*/
U32 simkzq_discard_data();	/* HSR to accept and discard incoming bytes  */
U32 simkzq_send_cmd();		/* HSR to transfer command bytes             */
U32 simkzq_reselect_interrupt();/* Process reselection interrupts        */
U32 simkzq_clear_parity_error();/* Clear the KZQ's parity error condition*/
U32 kzq_simx_attach();		/* Config's SIM attachment to setup vectors  */
U32 simkzq_attach();
static SIM_WS *simkzq_get_ws(); /* Function which converts, bus/id/lun to WS */
void sim_kzq_chip_reset(); 
void simkzq_sel_timeout();	/* Routine called to handle selection TMO    */
void kzq_intr();		/* Interrupt Service Routine for the KZQ HBA */
void simkzq_logger();		/* HSR to log HBA unique information         */
void simkzq_clear_selection();  /* Clear the KZQ's selection state	     */
void simkzq_add_sm_queue();     /* Add an interrupt context softc queue  */
void simkzq_error_recov();	/* Error recovery function		     */



/* ---------------------------------------------------------------------- */
/* 
 * Local SIM_KZQ defines.
 */

char *simkzqidstring = "scsiid?";  /* string to search for                  */
#define CNTLR_INDEX     6       /* loc of controller char in the string     */
#define ASCII_0         0x30    /* add to binary # to get ACKZQ equivilent  */
#define PHASE_DISPATCH SC_SM	/* Create KZQ version of common function    */ 
#define KZQ_SEL_TMO	hz	/* Timeout period for selection once/second */
#define MAX_BYTE_SPIN 25000	/* Max time to spin transfering a byte	    */
#define ILLEGAL_PHASE 0x5	/* MSG and I/O, C/D Clear = Illegal SCSI phs*/
#define WAIT_REQ  MAX_BYTE_SPIN	/* Max time to wait for REQ to assert       */
#define FULLDEBUG 1
#define CAMERRLOG		/* SIM Error logging enabled		    */
#define SELECT_TIME 10*25000 	/* Time in ~us to wait for selection        */
#define ARB_WIN_TIME 500	/* Max time to spin to insure ARB started   */
#define MAX_KZQS 3
/*#define trace_ints		/* Print out trace of interrupt calls	    */

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */




/* ---------------------------------------------------------------------- */
/* External Routines
 */

extern void scsiisr();
extern void timeout();
extern I32 cam_at_boottime();    /* During boot not everything is running */
extern void untimeout();
extern void sc_setup_ws();
extern void sc_lost_arb();
extern void sc_sel_timeout();
extern void sim_err_sm();
extern SIM_WS *sc_find_ws();
extern I32 ccfg_slave();



/* ---------------------------------------------------------------------- */
/* External structures/variables
 */

extern int  scsiisr_thread_init;
extern U32 camdbg_flag;		/* PRINTD control flags 	      	*/
extern int hz; 	    		/* Number clock interrupts  per second	*/
extern U32  sm_queue_sz;	/* Global queue of interrupts		*/
extern SIM_SM sim_sm;
extern I32 scsi_bus_reset_at_boot; /* Whether to reset the bus at boot	*/
extern I32 nCAMBUS;
extern struct controller controller_list[]; 


/* ---------------------------------------------------------------------- */
/* Global storage for use during debug.
 */
SIMKZQ_REG *gl_kzqreg;
KZQ_INTR *gl_kzqintr;
SIMKZQ_SOFTC *gl_kzqhba_sc;
    
/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern SIM_SOFTC *softc_directory[];
extern void (*scsi_sm[SCSI_NSTATES][SCSI_NSTATES])();
extern CAM_SIM_ENTRY dec_sim_entry;
static void (*local_errlog)() = simkzq_logger;

   
/* ---------------------------------------------------------------------- */
/* KZQ specific defines and macros
 */

#define CLEAR_COMM(reg) (reg)->comm = 0

#define SIM_LOG_KZQ_ALL \
   (SIM_LOG_SIM_SOFTC | SIM_LOG_NEXUS | SIM_LOG_SIM_WS | SIM_LOG_DME_STRUCT)

#define CAM_DELAY(usec,expression,retval) \
    { U32 N = (usec); \
    N = N/10; \
    (retval) = CAM_REQ_CMP; \
    while ((--N > 0)&& !(expression)){ \
	DELAY(10); \
    } \
    if (N <= 0){ \
	(retval) = CAM_REQ_CMP_ERR; \
    } \
};


#define KZQ_DISMISS_INTERRUPT(cstat,dstat,kzqreg,sim_softc,s) \
    /*\
     * Clear any interrupts that were received and reenable KZQ \
     * interrupts.\
     */\
    kzqreg->dstat = dstat;\
    kzqreg->cstat = cstat;\
    kzqreg->csr |= KZQ_CSR_IE;\
    WBFLUSH();\
    SIM_SOFTC_UNLOCK(s, (SIM_SOFTC *)sim_softc);\
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
/* DALLAS */
int 
sim_getkzq_scsiid( cntlr )
    int cntlr;			/* controller/bus number on the system */
{
    char *env;			/* ptr for the NVR string */
    int nvr_id;			/* converted ID # from NVR */

    SIM_MODULE(sim_get_sckzqd);

    /* 
     * Build an id string from our controller #, i.e., scsiid0, 1, etc.  
     * The ID string can be reused. 
     */

    simkzqidstring[ CNTLR_INDEX ] = (char)((cntlr & 0xff) + ASCII_0);

    env = (char *)prom_getenv( simkzqidstring );
    if (env != NULL) {
	nvr_id = xtob(env);		/* convert ACKZQ hex to binary */

	/* Is the ID a valid #, ID's on the SCSI bus can only be [0-7]. */
	if ((nvr_id >= 0) && (nvr_id <= 7)) {
	    return( nvr_id );
	}
    }
    
    /* The SCSI bus ID conversion failed, return the default value to be used
    for this controller. */


    return( 7 );		/* return the default */
}


/**
 * simkzq_logger -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function logs errors specific to the KZQ SIM modules.
 *
 * FORMAL PARAMETERS:  		
 *	u_char *func		calling module name, may be NULL
 *	u_char *msg		error description, may be NULL
 *	SIM_SOFTC *sc		SIM_SOFTC pointer, may be NULL
 *	SIM_WS *sws		SIM_WS pointer, may be NULL
 *	SIMKZQ_INTR *intr	SIMKZQ_INTR pointer, may be NULL
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
simkzq_logger(func, msg, flags, sc, sws, intr)
    u_char *func;
    u_char *msg;
    U32 flags;
    SIM_SOFTC *sc;
    SIM_WS *sws;
    KZQ_INTR *intr;

{
    SIM_MODULE(simkzq_logger);
    CAM_ERR_HDR hdr;
    static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];
    CAM_ERR_ENTRY *entry;
    KZQ_INTR *tintr;
    SIMKZQ_SOFTC *tssc;
    SIMKZQ_REG *tkzqreg;
    int i;

    bzero( &hdr, sizeof( CAM_ERR_HDR ));

    hdr.hdr_type = CAM_ERR_PKT;
    hdr.hdr_class = CLASS_KZQ;
    hdr.hdr_subsystem = SUBSYS_KZQ;
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
     * Log the active KZQ_INTR structure.
     */
    if (flags & SIM_LOG_HBA_INTR) {
	tintr = (KZQ_INTR*)NULL;
	if (intr != (KZQ_INTR*)NULL)
	    tintr = intr;
	else if (sc != (SIM_SOFTC *)NULL)
	    if (sc->hba_sc != (void *)NULL)
		tintr = ((SIMKZQ_SOFTC *)sc->hba_sc)->active_intr;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		if (sws->sim_sc->hba_sc != (void *)NULL)
		    intr =
			((SIMKZQ_SOFTC *)sws->sim_sc->hba_sc)->active_intr;
	if (tintr != (KZQ_INTR*)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_KZQ_INTR;
	    entry->ent_size = sizeof(KZQ_INTR);
	    entry->ent_vers = KZQ_INTR_VERS;
	    entry->ent_data = (u_char *)tintr;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the SIMKZQ_SOFTC
     */
    if (flags & SIM_LOG_HBA_SOFTC) {
	tssc = (SIMKZQ_SOFTC *)NULL;
	if (sc != (SIM_SOFTC *)NULL)
	    tssc = (SIMKZQ_SOFTC *)sc->hba_sc;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		tssc = sws->sim_sc->hba_sc;
	if (tssc != (SIMKZQ_SOFTC *)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIMKZQ_SOFTC;
	    entry->ent_size = sizeof(SIMKZQ_SOFTC);
	    entry->ent_vers = SIMKZQ_SOFTC_VERS;
	    entry->ent_data = (u_char *)tssc;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the SIM KZQ device CSR's, if the flags say to do so and the
     * softc is not NULL.
     */
    if ( (flags & SIM_LOG_HBA_CSR) &&  (sc != (SIM_SOFTC *)NULL))
    {
	tkzqreg = (SIMKZQ_REG *)sc->csr_probe;	/* Get addr of device CSR's */
	if (tkzqreg != (SIMKZQ_REG *)NULL)
	{
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIMKZQ_REG;
	    entry->ent_size = sizeof(SIMKZQ_REG);
	    entry->ent_vers = SIMKZQ_REG_VERS;
	    entry->ent_data = (u_char *)tkzqreg;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Call sc_logger to log the common structures.
     */
    sc_logger(&hdr, SIM_LOG_SIZE, sc, sws, flags);
}


/**
 * sim_init_kzq -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Called to initialize the KZQ chip. This routine must be called 
 * after a bus reset or power on. After this routine completes the KZQ will
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
sim_init_kzq(sim_softc)
    
    SIM_SOFTC* sim_softc;
    
{
    REGISTER SIMKZQ_SOFTC *hba_softc;
    U32 retval;			/* Return value    */
    SIMKZQ_REG *kzqreg;			/* KZQ CSR address */
    int s;

    SIM_MODULE(sim_init_kzq);

    /*
     * Local initialization
     */
    hba_softc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;
    retval = CAM_REQ_CMP;			/* Return value        */
    kzqreg = sim_softc->reg;			/* KZQ CSR address     */

    /*
     * Reset chip first Don't lock since the chip reset functon
     * does it.
     */
    (hba_softc->chip_reset)(sim_softc);

    /*
     * SMP lock on this controller.
     */
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * Setup the KZQ Diagnostic Control Register to allow the KZQ to drive
     * the SCSI bus.
     */
    kzqreg->dictrl |= KZQ_DICTRL_PRE;
    
    /*
     * Setup the KZQ CSR to enable interrupts, reselection
     * events and enable parity checking.
     */
    kzqreg->csr = (KZQ_CSR_RSE|KZQ_CSR_PCE|KZQ_CSR_IE);

    WBFLUSH();

    /*
     * Release SMP lock on this controller.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);


    PRINTD(NOBTL,NOBTL,NOBTL,CAMD_DMA_FLOW,
	   ("(sim_init_kzq) Completed KZQ initialization\n"));

    return(retval);
};			/* sim_init_kzq */


/**
 * simkzq_probe -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simkzq_probe() is called at boot time, once for each SCSI
 * bus based on the configuration information. This function is
 * responsible for allocating the SOFTC structure and placing it
 * in the softc_dir[].  The passed arguments are stored in the softc
 * for the next pass of the initialization call via (*sim_init)().
 *
 * FORMAL PARAMETERS:  		
 *
 *	caddr_t csr;
 *	BOP_PROBE_STRUCT *prb
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
simkzq_probe( csr, cntlr )

    caddr_t csr;
    int cntlr;
{
    SIM_SOFTC *sim_softc;
    struct controller *um;
    SIMKZQ_REG *kzqreg = (SIMKZQ_REG * )csr;
    I32 cnt;
    I32 i;
    U32 found;

    SIM_MODULE(simkzq_probe);

    PRINTD(um->um_ctlr, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	("(simkzq_probe) begin\n"));

    /*
     * Get our contoller struct
     */

    /* The following indexes through the controller_list array of
     * structs until it reaches the kzq controller or until it
     * runs out of valid struct addresses		
     * Must do it this way because at this point in time we don't
     * have enought information to get to our bus number.
     */
    found=0;
    um = controller_list;
    while((found == 0) && ((um++)->ctlr_name)) {
        if ((strcmp(um->ctlr_name,"kzq")==0) && (um->ctlr_num == cntlr)) {
	    found=1;

	}
    }
    if(found == NULL){
	return( NULL );
    }

    if( cntlr >= nCAMBUS){
	CAM_ERROR(module,
		   "simkzq_probe: Controller number excedes suppoort number",
		   SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE, NULL,
		   NULL,NULL);
    	return(NULL);
    }
	

    /* 
     * See if this has been probed before.
     */
    if( softc_directory[ cntlr ] != NULL){
	/* 
	 * See if we have already probed this one....
	 */
	if( (softc_directory[ cntlr ])->um_probe == um ){
	    CAM_ERROR(module,
		   "simkzq_probe: controller already probed",
		   (SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
		      softc_directory[ cntlr ],NULL,NULL);
    	    return(CAM_REQ_CMP);
	}
	else {
	    CAM_ERROR(module,
		   "simkzq_probe: Another type controller configured here",
		   (SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
		      softc_directory[ cntlr ],NULL,NULL);
    	    return(NULL);
	}
	    
    }

    /* 
     * The following code finds the first vacant vector and  
     * loads it with the kzqsa interupt address
     */
    cnt = (uba_hd[numuba].uh_lastiv -= 4);


    cvec = kzqreg->vector = cnt;
    WBFLUSH();

    softc_directory[ cntlr ] = (SIM_SOFTC *)sc_alloc(sizeof(SIM_SOFTC));

    if ( !softc_directory[cntlr] ){
	CAM_ERROR(module,
		 "simkzq_probe: memory allocation of softc failed",
		(SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),softc_directory[ cntlr ],
		NULL,NULL);
	return( NULL );
    }

    sim_softc = softc_directory[ cntlr ];


    sim_softc->hba_sc = (void *)sc_alloc(sizeof(SIMKZQ_SOFTC)); 

    if ( !sim_softc->hba_sc )
      {
	CAM_ERROR(module,
		  "simkzq_probe: memory allocation of hba_softc failed.",
		  (SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE),
		  softc_directory[ cntlr ],NULL,NULL);
	return( NULL );

      }

    /* 
     * Update the probe storage fields in the softc structure. 
     */

    sim_softc->csr_probe = csr;
    sim_softc->um_probe = (void *)um;

    /*
     * Using the controller structure and csr update the "normal" fields in the
     * softc.  These are "expected" in the rest of the initialization path.  
     */
    sim_softc->reg = csr;		/* base addr */
    sim_softc->cntlr = cntlr;	/* path id */

    /*
     * scsi_bus_reset_at_boot is a flag that is used to reset the SCSI bus at
     * boot. If the flag is set the bus is reset during probe.
     *
     * A better solution to this would be to allow for turning off synchronous 
     * negotiation on a target by target basis.
     */
    if (scsi_bus_reset_at_boot != 0)
    {
	simkzq_bus_reset(sim_softc);
    }
                                  
    /* 
     * DO NOT Call any other attachment since nothing is filled
     * in until the return from the probe call in unifind. 
     */

    PRINTD(um->um_ctlr, NOBTL, NOBTL, (CAMD_INOUT | CAMD_CONFIG),
	   ("(simkzq_probe) end\n"));
    
    return(CAM_REQ_CMP);
}

/**
 * simkzq_slave -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simkzq_slave() is called at boot time, atfer probe has been called
 * once for each SCSI device bus based on the configuration information. 
 * This function is responsible for determining to see if we have been
 * init'ed and not to call the attach code again.
 *
 * FORMAL PARAMETERS:  		
 *
 *	struct uba_device *ui;
 *	 caddr_t csr;
 *
 * IMPLICIT INPUTS:     	NONE
 *
 * IMPLICIT OUTPUTS:		NONE
 *
 * RETURN VALUE:		CAM_TRUE if device exists, CAM_FALSE otherwise
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE 
 *
 **/
int
simkzq_slave( dev_ctlr, reg )

    struct device *dev_ctlr;
    caddr_t reg;

{

    SIM_SOFTC *sim_softc;
    struct controller *um;
    int cntlr;

    /*
     * Get our controller structure pointer, since the um is not
     * filled in until after attach we get the cntr number and match
     * that way.
     */
    cntlr = dev_ctlr->ctlr_num;

    if(( sim_softc = softc_directory[ cntlr ] ) ==  (SIM_SOFTC *)NULL){
	/* 
	 * This has never been probed and we should not be here...
	 * We could panic but lets return failure
	 */
	return( NULL );
    }

    /* 
     * Now get the ctlr struct
     */
    if(( um = sim_softc->um_probe ) == ( struct controller *)NULL){
    
	/*
	 * Not valid return failure 
	 */
	return( NULL );
    }

    /*
     * Lets see if we are one and the same... Could probably do away 
     * we this check.
     */
    if ((strcmp(um->ctlr_name,"kzq")!= 0) && (um->ctlr_num != cntlr)) {
	/*
	 * Does not match fail it
	 */
	return( NULL );
    }

    /*
     * Lets see if we have been attached
     */
    if( sim_softc->simh_init != NULL){
	/* Already attached */
	return( ccfg_slave(dev_ctlr, reg));
    }
    else {
	/*
	 * Should only go thru once per controller
	 */
	if( ccfg_simattach( &dec_sim_entry, dev_ctlr->ctlr_num ) == 
				CAM_FAILURE ) {
	    return(0);
        }
    	return( ccfg_slave(dev_ctlr, reg));
    }
}

/**
 * simkzq_start_cntrlerr_recovery -
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
    simkzq_start_cntrlerr_recovery(sim_softc)
SIM_SOFTC *sim_softc;
{
    SIM_MODULE(simkzq_start_cntrlerr_recovery);
    CAM_ERROR(module,"Initiate controller recovery",
	      SIM_LOG_PRISEVERE,sim_softc,NULL,NULL);

    /*
     * Since at this point controller level recovery doesn't exist in the SIM,
     * simply assert bus reset if we run into "can't" happen conditions
     *
     * Controller level recovery is needed when the SCSI bus or SIM is in an
     * error state other than a state where there is a particular I/O which is
     * active. The error recovery in the SIM needs to be extended to allow
     * error recovery to begin WITHOUT requiring an active and uptodate SIM_WS
     */
    (void) simkzq_bus_reset(sim_softc);
    
    sim_softc->error_recovery &= ~ERR_UNKNOWN;

    return(CAM_REQ_CMP);
};

/**
 * simkzq_attach -
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
simkzq_attach( SIM_SOFTC *sim_softc )
    {
	SIM_MODULE(simkzq_attach);
	sim_softc->hba_init = simkzq_init;
	sim_softc->hba_go = simkzq_go;
	sim_softc->hba_sm = simkzq_sm;
	sim_softc->hba_bus_reset = simkzq_bus_reset;
	sim_softc->hba_send_msg = simkzq_send_msg;
	sim_softc->hba_xfer_info = simkzq_xfer_info;
	sim_softc->hba_sel_msgout = simkzq_sel;
	sim_softc->hba_msgout_pend = simkzq_req_msgout;
	sim_softc->hba_msgout_clear = simkzq_clear_msgout;
	sim_softc->hba_msg_accept = simkzq_msg_accept;
	sim_softc->hba_setup_sync = simkzq_setup_sync;
	sim_softc->hba_discard_data = simkzq_discard_data;

    return(CAM_REQ_CMP);
    }

/**
 * simkzq_init -
 *
 * FUNCTIONAL DESCRIPTION:
 *	simkzq_init() is called during  boot time, once for each SCSI
 * bus based on the DEC KZQ.  This function is responsible for
 * initializing the DEC KZQ SIM and associated DME (data mover engine).
 *
 * FORMAL PARAMETERS:  		
 *	caddr_t csr		DEC KZQ CSR address
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
simkzq_init(sim_softc, csr)
REGISTER SIM_SOFTC *sim_softc;
void *csr;

{

    SIMKZQ_SOFTC *hba_sc;

    SIM_MODULE(simkzq_init);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_init) begin\n"));

    /*
     * Determine the SCSI ID of this HBA on the SCSI bus.
     */
    sim_softc->scsiid = sim_getkzq_scsiid(sim_softc->cntlr);

    /*
     * Set "hba_sc" pointer in SIM_SOFTC to the appropriate SIMKZQ_SOFTC.
     * Make sure that the SIMKZQ_SOFTC assignment is done only
     * once per controller.
     */
    if (!sim_softc->simh_init) {
	/*
	 * Indicate to the balance of the SIM that this HBA has been
	 * initialized.
	 */
	sim_softc->simh_init = CAM_TRUE;
    }

    /*
     * Set the DEC KZQ register pointer in the SIM_SOFTC;
     */
    sim_softc->reg = (void *)csr;


    /*
     * Set-up the machine specific functions.  This will be done via
     * an attach routine, for now (for ISV) only 3MAX is supported.
     */
    hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;


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
    hba_sc->last_dismissed_kzq_intr =  &hba_sc->intrq_buf[SM_QUEUE_SZ];


    /*
     * Setup the KZQ chip specific reset entry point.
     */
    hba_sc->chip_reset = sim_kzq_chip_reset;


    /*
     * Intialize the temporary working set which is used for interrupts
     * that occur when it's not clear which if any I/O the interrupt is for.
     */
    sc_setup_ws(sim_softc, &sim_softc->tmp_ws, 0, 0);

    /*
     * Reset the KZQ chip using an KZQ specific command.
     */
    (hba_sc->chip_reset)(sim_softc);

    /*
     * Perform KZQ specific initialization.
     */
    sim_init_kzq(sim_softc);

    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_init) end\n"));

    return(CAM_TRUE);
}

/**
 * simkzq_go -
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
 * RETURN VALUE:        	Returned CAM status from simkzq_sel()
 *
 * SIDE EFFECTS:        	NONE
 *
 * ADDITIONAL INFORMATION:      NONE
 *
 **/
U32
simkzq_go(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    U32 status;
    int s;

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_go) begin\n"));


    /*
     * Start the selection from a target off. If the target selects we will
     * get a TBE or IBF indicating that the target is REQuesting a byte.
     *
     * NOTE: Tagged commands are not currently supported.
     */
    status = simkzq_sel(sim_ws);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_go) end, status is %x\n", status));

    /*
     * Return with the status from the selection command.
     */
    return(status);
}




/**
 * simkzq_print_interrupts -
 *
 * FUNCTIONAL DESCRIPTION:
 *     This function is used during debug to parse csr's from an interrupt
 * and print out the interrupt context in textual form to the console.
 *
 * FORMAL PARAMETERS:  		
 *	KZQ_INTR *kzqintr	Interrupt context
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
simkzq_print_interrupt(kzqintr)

REGISTER KZQ_INTR *kzqintr;

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
	    kzqintr->csr, kzqintr->comm, kzqintr->cstat, kzqintr->dstat));


    if (kzqintr->cstat & KZQ_CSTAT_CI)
    {			/* CI Interrupt */

	if (kzqintr->cstat & KZQ_CSTAT_SCH)
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,("SCH with "));
	    
	    /* Now check for what type of sch interrupt */
	    if (kzqintr->cstat & KZQ_CSTAT_DST)
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
	else if (kzqintr->cstat & ((KZQ_CSTAT_RST | KZQ_CSTAT_OBC | 
				     KZQ_CSTAT_BUF | KZQ_CSTAT_LDN)))
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("RST,OBC,BUF,LDN Interrupt, cstat %x\n",
		    kzqintr->cstat));
	}

    }			/* CI Interrupt */

    /*
     * Now check for DSTAT related interrupts.
     */
    if (kzqintr->cstat & ~KZQ_CSTAT_DI)
	
    {			/* DI Interrupt */
	
	if (kzqintr->dstat & KZQ_DSTAT_MIS)
	{
	PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	       ("MIS (PHASE MIS), "));
	
	/*
	 * The phase in the COMM register doesn't match the current bus phase.
	 * We need to transition to a new bus phase.
	 */
	}

	if (kzqintr->dstat & KZQ_DSTAT_DNE)

	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,("DNE, "));
	    
	}


	if (kzqintr->dstat & KZQ_DSTAT_TBE)

	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("TBE (TRANS BUF EMPTY), "));
	
	}

	if (kzqintr->dstat & KZQ_DSTAT_IBF)

	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("IBF (IN BUF FULL), "));
	
	}
	
	if (kzqintr->dstat & KZQ_DSTAT_TCZ)
	    
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("TCZ (Trans Count 0), "));
	}

	if (kzqintr->dstat & KZQ_DSTAT_OBB)
	    
	{
	    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	   ("(print_interrupt OBB) csr 0x%x,comm 0x%x,cstat 0x%x,dstat 0x%x\n",
	    kzqintr->csr, kzqintr->comm, kzqintr->cstat, kzqintr->dstat));

	/*
	 * Figure out what to do here, perhaps the DME needs to be paused and
	 * the count adjusted for "lost" bytes.
	 */
	}



	if (kzqintr->dstat & KZQ_DSTAT_IPE)
	    
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
 * simkzq_handle_interrupt-
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * This routine is called to process each interrupt serviced by the HBA state
 * machine. This routine's primary purpose is to determine which interrupts
 * represent a state change on the bus and which do not. Those interrupts 
 * what represent a state change will return with kzqintr->new_state= 1 those 
 * interrupts which don't represent a state change will return new_state =0.
 * 
 * If a state change has occured, it is expected that the caller of this 
 * routine will dispatch on this state change to a phase specific routine.
 *
 * FORMAL PARAMETERS:  		
 *	KZQ_INTR *kzqintr 	- Current interrupt context.
 *	SIM_SOFTC *sim_softc	- SIM software control structure
 *	SIMKZQ_SOFTC *hba_sc	- HBA specific software control structure
 *	SIMKZQ_REG *kzqreg	- KZQ CSR's
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
simkzq_handle_interrupt(kzqintr,sim_softc,hba_sc,kzqreg,sim_ws)

REGISTER KZQ_INTR *kzqintr;
REGISTER SIM_SOFTC *sim_softc;
REGISTER SIMKZQ_SOFTC *hba_sc;
REGISTER SIMKZQ_REG *kzqreg;
REGISTER SIM_WS *sim_ws;

{			/* handle_interrupt */
    int s;
    u_short kzq_cmd;	/* The KZQ command word        */
    u_short sc1;	/* KZQ SCSI bus state register */
    U32 retval;	/* Status of bit spins         */
    u_short dstat_phase = (kzqintr->dstat & KZQ_DSTAT_PHASE);


    SIM_MODULE(kzq_handle_interrupt);

    PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
	   ("(handle_interrupt) csr 0x%x, comm 0x%x, cstat 0x%x, dstat 0x%x\n",
	    kzqintr->csr, kzqintr->comm, kzqintr->cstat, kzqintr->dstat));


  /*
   * Has this sim_ws already completed, if so simply return, since there
   * are no other phases to process. We either got a MIS or a SCH interrupt
   * from the KZQ after the bus went free.
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
	    kzqintr->new_state= 0;

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
	    SIM_SOFTC_LOCK(s,sim_softc);
	    sim_softc->active_io = NULL;
	    SIM_SOFTC_UNLOCK(s,sim_softc);
	   
	    SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
    	    PHASE_DISPATCH(sim_ws);
	    kzqintr->new_state= 0;

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

    if (kzqintr->cstat & KZQ_CSTAT_CI)
    {			/* CI Interrupt */

	if (kzqintr->cstat & KZQ_CSTAT_SCH)
	{		/* SCH Interrupt checks */

	    /* Now check for what type of sch interrupt */
	    if (kzqintr->cstat & KZQ_CSTAT_DST)
	    {
		/*
		 * DST means that we were the DeSTination of a selection.
		 */
		(void) simkzq_reselect_interrupt(kzqintr, 
					       sim_ws, kzqreg, sim_softc);

		/*
		 * Wait for IBF to transition to next phase, since the target
		 * may not have already gone to message phase.
		 */
		kzqintr->new_state= 0;
	    }

	    else if ( !(kzqintr->cstat & KZQ_CSTAT_CON))
		
	    {		/* Not CONNECTED (DST and CON clear), BUS FREE */
		
		PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, 
		       CAMD_PHASE,("(simkzq_sm) phase is scsi_bus_free\n"));

		SIM_SOFTC_LOCK(s,sim_softc);
		sim_softc->active_io = NULL;		
		SIM_SOFTC_UNLOCK(s,sim_softc);

		/*
		 * Set the current bus phase to an illegal phase, so that any
		 * phase transition will be processed.
		 */
		hba_sc->last_int_phase = ILLEGAL_PHASE;

		/*
		 * After a failed selection attempt, the KZQ will interrupt 
		 * with a BUS FREE interrupt after the DISCON command has been
		 * issued to the KZQ. This interrupt should be ignored and not
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
		kzqintr->new_state= 0;
		return;
	    }			/* Not CONNECTED */
	    
	    
	    /*
	     * If we are connected to a target and we are not the destination
	     * (DST not set) then it means that our selection completed, 
	     * assuming that we are trying to select..
	     */
	    else if ( (kzqintr->cstat & KZQ_CSTAT_CON) && 
		     (sim_softc->flags & SZ_TRYING_SELECT))

	    {		/* Selection completed */
		
		
		sim_ws = hba_sc->sws;/* Make this I/O ACTIVE */

		PRINTD(sim_ws->cntlr,sim_ws->targid,sim_ws->lun,
		       CAMD_INOUT,
		       ("(simkzq_sm) Selection completed, stop selection\n"));

		
		
		SIM_SOFTC_LOCK(s,sim_softc);

		/*
		 * Clear the state such that selection has completed and 
		 * reselections might continue. Note that this routine clear
		 * ATN, but since the KZQ has a bug where it drops ATN if 
		 * selection completes, this is benign.
		 */
		simkzq_clear_selection(sim_softc,hba_sc,kzqreg);

		sim_softc->active_io = sim_ws;     /* Make this I/O ACTIVE   */
		SIM_SOFTC_UNLOCK(s,sim_softc);

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
		    simkzq_setup_sync(sim_ws,sim_softc,kzqreg);		

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
			      sim_softc,sim_ws,kzqintr);

		    sim_softc->error_recovery |= ERR_UNKNOWN;
		    /* Perhaps we should count these */
		}

		/*
		 * Wait for TBE or IBF to start next phase transition.
		 * We must wait for IBF and TBE to insure that the phase
		 * has stabilized in the KZQ. If we don't wait we could
		 * dispatch on the wrong phase since the KZQ is sometimes
		 * slow in doing this.
		 */
		kzqintr->new_state= 0;
		return; 
	    };		/* Selection completed */
	    
	}		/* SCH Interrupt checks */


	/*
	 * Receipt of any of these bits should be treated as an error, logged
	 * and recovered from etc...
	 */
        if (kzqintr->cstat & ((KZQ_CSTAT_RST | KZQ_CSTAT_OBC | KZQ_CSTAT_BUF |
				KZQ_CSTAT_LDN)))
	{

	    CAM_ERROR(module,
		       "RST,OBC,BUF,LDN Interrupt",
		      SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE,
		      sim_softc,sim_ws,kzqintr);

	    
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
	     * thread and reenable KZQ interrupts. 
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
	    SIM_SOFTC_LOCK(s,sim_softc);
	    sim_softc->active_io = NULL;
	    SIM_SOFTC_UNLOCK(s,sim_softc);

	    /*
	     * Set the current bus phase to an illegal phase, so that any
	     * phase transition will be processed.
	     */
	    hba_sc->last_int_phase = ILLEGAL_PHASE;
	    
	    SC_NEW_PHASE(sim_ws, SCSI_BUS_FREE);
	    PHASE_DISPATCH(sim_ws);
	    kzqintr->new_state= 0;
	    return;

	}		/* If error interrupt */

    }			/* CI Interrupt */

    /*
     * Check for parity errors before looking at other DSTAT conditions
     * If we are getting data from the target, then start error processing.
     */
	if (kzqintr->dstat & KZQ_DSTAT_IPE)

	{

	    CAM_ERROR(module,
		       "KZQ Parity Error",
		       (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		       sim_softc,sim_ws,kzqintr);

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
			  SIM_LOG_PRISEVERE,sim_softc,sim_ws,kzqintr);
		
		sim_ws->error_recovery |= ERR_PARITY;
	    }


	    /*
	     * Force the KZQ to clear the Parity Error condition
	     */
	    if (CAM_REQ_CMP != simkzq_clear_parity_error(kzqreg, 
							 sim_softc,sim_ws))
	    {

		CAM_ERROR(module,
			   "Parity error condition failed to clear",
			  SIM_LOG_PRISEVERE,sim_softc,sim_ws,kzqintr);

		/*
		 * Since we have been unable to clear the IPE, we have no
		 * choice but to reset the SCSI bus. This will force recovery
		 * in the SIM and prevent any possible data lose on the target.
		 */
		simkzq_start_cntrlerr_recovery(sim_softc);

		/*
		 * The SCSI bus reset, will cause a state transition, thus
		 * ignore this interrupt and go on.
		 */
		kzqintr->new_state = 0;	    
		return;
	    }

	    /*
	     * Since the IPE has been cleared in the KZQ. 
	     * Treat this as a new state, by setting new_state = 1.
	     */
	    kzqintr->new_state = 1;	    
	    return;

	}		/* If Parity Error */

    /*
     * Now check for DSTAT related interrupts. The DSTAT register is the KZQ's
     * DATA TRANSFER STATUS REGISTER. It contains the phase of the SCSI bus as
     * well as the KZQ's state relative to the bus phase and data interactions
     * with the target device. 
     */
    if (kzqintr->dstat & KZQ_CSTAT_DI)
	
    {			/* DI Interrupt */
	
	/*
	 * NOTE:
	 * Some device will select soo fast, that the KZQ might never
	 * see the selection complete, rather the first interrupt to 
	 * be seen will be a TBE interrupt for a message out byte.
	 * Thus, in those case we must clear any pending selection state
	 * that may still be in progress.
	 */
	if (sim_softc->flags & SZ_TRYING_SELECT)
	{		/* SZ_TRYING_SELECT */
	    
	    
	    SIM_SOFTC_LOCK(s,sim_softc);

	    
	    /*
	     * Clear the state such that selection has completed and 
	     * reselections might continue. Note that this routine clear
	     * ATN, but since the KZQ has a bug where it drops ATN if 
	     * selection completes, this is benign.
	     */
	    simkzq_clear_selection(sim_softc,hba_sc,kzqreg);
	    
	    sim_softc->active_io = sim_ws;     /* Make this I/O ACTIVE   */

	    SIM_SOFTC_UNLOCK(s,sim_softc);
	    
	    /*
	     * Now that selection has completed,
	     * Set-up the synchronous offset and period.
	     */
	    simkzq_setup_sync(sim_ws,sim_softc,kzqreg);		
	    
	    /*
	     * Indicated that we have completed arbitration and
	     * selection and continue with next phase
	     */
	    SC_NEW_PHASE(sim_ws, SCSI_ARBITRATION);
	    SC_NEW_PHASE(sim_ws, SCSI_SELECTION);

	    /*
	     * Once all this has been done, then goo off and look
	     * the actual interrupt the KZQ is seeing now.
	     */

	}		/* If SZ_TRYING_SELECT */
	
	/*
	 * Since A DI interrupt, check for TBE and IBF interrupts 
	 * and then for MIS. Thus if we have an IBF/TBE and a MIS 
	 * interrupt, the IBF/TBE interrupt will be serviced first.
	 *
	 * WARNING:
	 * However, there is a problem with the KZQ where if MIS is set
	 * with IBF or TBE the phase in the dstat MAYBE wrong! 
	 */
	if (kzqintr->dstat & KZQ_DSTAT_TBE) 
	    
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
		kzqintr->new_state = 1;	    
		return;
	    }
	    
	    /*
	     * REQ must be asserted, now since we had TBE set, if it
	     * isn't then the phase must have changed. If the phase 
	     * hasn't changed and the phase from the interrupt and
	     * the current dstat are not the same then this interrupt
	     * should not be used to transition the state.
	     */ 
	    else if ((kzqreg->sc1 & KZQ_SC1_REQ) && 
		     ((kzqreg->dstat & KZQ_DSTAT_PHASE) == dstat_phase))
	    {
		kzqintr->new_state = 1;	    
		return;
	    }
	    else	/* The phase must have changed */
	    {
		CAM_ERROR(module,
			  "KZQ TBE interrupt but phase changed",
			  (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			  sim_softc,sim_ws,kzqintr);
		
		kzqintr->new_state = 0;
		return;
	    };
	    
	}			/* TBE interrupt */	
	
	
	/* 
	 * Now check IBF for a byte coming in.
	 * There is a problem with the KZQ where if MIS is set
	 * with IBF or TBE the phase in the dstat is wrong. This
	 * should no longer be occuring since the ISR converts MIS
	 * interrupts to either IBF or TBE interrupts.
	 */
	else if (kzqintr->dstat & KZQ_DSTAT_IBF) 
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
		 * fast reselection, where the KZQ never interrupts us 
		 * when it reselected. Now treat this as a "normal reselection"
		 */
		(void) simkzq_reselect_interrupt(kzqintr, 
						 sim_ws, kzqreg, sim_softc);
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
		kzqintr->new_state = 1;	    
		return;
	    }
	    
	    /*
	     * REQ must be asserted, now since we had IBF set, if it
	     * isn't then the phase must have changed. If the phase 
	     * hasn't changed and the phase from the interrupt and
	     * the current dstat are not the same then this interrupt
	     * should not be used to transition the state.
	     */
	    else if ((kzqreg->sc1 & KZQ_SC1_REQ) && 
		     ((kzqreg->dstat & KZQ_DSTAT_PHASE) == dstat_phase))
	    {
		kzqintr->new_state = 1;	    
		return;
	    }
	    else	/* The phase must have changed */
	    {
		CAM_ERROR(module,
			  "KZQ IBF interrupt but phase changed",
			  (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			  sim_softc,sim_ws,kzqintr);
		
		kzqintr->new_state = 0;
		return;
	    };
	    
	}	/* IBF interrupt */
	else if (kzqintr->dstat & KZQ_DSTAT_MIS)
	    
	    /*
	     * Although the KZQ specification indicates that MIS interrupts are
	     * synchronous with the assertion of REQ on the SCSI, this is NOT
	     * the case. Thus, ISR ignores MIS interrupts and waits for IBF or
	     * TBE interrupts before it dispatches an interrupt to the state
	     * machine. Therefore, we should never see an MIS interrupt in this 
	     * routine. We will simply log and ignore the MIS interrupts here.
	     */
	    
	{		/* Check for MIS */
	    
	    CAM_ERROR(module,
		      "KZQ MIS interrupt",
		      (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		      sim_softc,sim_ws,kzqintr);
	    
	    kzqintr->new_state = 0; /* Ignore this interrupt */
	    return;
	    
	}	/* MIS interrupt */
	
	/*
	 * Check for DSTAT done interrupts
	 */
	
	if (kzqintr->dstat & KZQ_DSTAT_DNE)
	    
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
    
};			/* kzq_handle_interrupt */

/**
 * simkzq_phase_dispatch -
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * 	This function is called by the simkzq_sm "state machine". This function
 * will dispatch on the current bus phase to a phase specific routine.
 *
 * FORMAL PARAMETERS:  		
 *    	KZQ_INTR *kzqintr	- Current interrupt context
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
simkzq_phase_dispatch(kzqintr, sim_ws)
    REGISTER KZQ_INTR *kzqintr;
    REGISTER SIM_WS  *sim_ws;
{
    /*
     * Local storage and initialization
     */
    SIM_SOFTC  *sim_softc	= sim_ws->sim_sc;
    SIMKZQ_REG *kzqreg 		= sim_softc->reg;
    u_short sc1;				/* Snapshot of SC1 register */
    int s;
    u_char achar;
    U32 retval;


    SIM_MODULE(simkzq_phase_dispatch);
    
    /*
     * Although this check should not be necessary, it has proved to be soo
     * useful during debug that it is best to keep it in the driver. Soo many
     * of possible failure modes result in this condition being meet that even
     * in production level code this check will be a some benefit for not 
     * much cost.
     */
    if ((kzqreg->dstat & KZQ_DSTAT_PHASE) != 
	(kzqintr->dstat & KZQ_DSTAT_PHASE))
    {
	CAM_ERROR(module,
		  "Current SCSI bus phase doesn't match interrupt",
		  (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		  sim_softc,sim_ws,kzqintr);
	
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
    switch (kzqintr->dstat & KZQ_DSTAT_PHASE)

    {

    case SC_PHASE_DATAOUT:

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simkzq_phase_dispatch) phase is scsi_dataout\n"));

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
	       ("(simkzq_phase_dispatch) phase is scsi_datain\n"));
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
	       ("(simkzq_phase_dispatch) phase is scsi_command\n"));
	
	/*
	 * Record then phase, then call sm_command to transfer command 
	 * bytes.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_COMMAND);
	PHASE_DISPATCH(sim_ws);
	
	break;
	
    case SC_PHASE_STATUS:
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simkzq_phase_dispatch) phase is scsi_status\n"));

	/*
	 * Block interrupts until status phase complete 
	 */
	SIM_SOFTC_LOCK(s,sim_softc);

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
	
	retval = simkzq_xfer_info(sim_ws,&(sim_ws->scsi_status),1,CAM_DIR_IN);
	
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
			       (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			       sim_softc,sim_ws,kzqintr);
		    
		    sim_ws->error_recovery |= ERR_UNKNOWN;
		}

	};		/* Xfer_info failed */

	/*
	 * Execute common STATUS phase and error processing.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_STATUS);
	PHASE_DISPATCH(sim_ws);
	
	SIM_SOFTC_UNLOCK(s,sim_softc);
	
	break;				/* SC_PHASE_MSGIN */

    case SC_PHASE_MSGOUT:

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simkzq_phase_dispatch) phase is scsi_msgout\n"));

	/*
	 * Call the state machine to actual transfer the message bytes.
	 */
	SC_NEW_PHASE(sim_ws, SCSI_MSGOUT);
	PHASE_DISPATCH(sim_ws);
	break;

    case SC_PHASE_MSGIN:

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	       ("(simkzq_phase_dispatch) phase is scsi_msgin\n"));

	/*
	 * Before reading this new message insure that the parity is valid,
	 * if not start error recovery.
	 */
	if (!(kzqreg->dstat & KZQ_DSTAT_IPE))
	{

	    /*
	     * Since we are in message in, assume that there is a valid message
	     * byte pending on the bus. Read this byte into the SIM_WS message
	     * queue, without asserting ACK. 
	     */
	    SIMKZQ_GET_BYTE(achar,kzqreg);	/* Read message byte off bus */
	    SC_ADD_MSGIN(sim_ws,achar);		/* Copy the msg to SIM_WS    */

	}
	else
	{
	    CAM_ERROR(module,
		       "Parity error on message IN byte",
			   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,kzqintr);
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
		   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,sim_ws,kzqintr);
    }

    return(CAM_REQ_CMP);
};			/* simkzq_phase_dispatch */



/**
 * simkzq_sm -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called by sckzqsr() (State machine)  to process an 
 * interrupt from the HBA. In general this function is called at low IPL, 
 * however this is not a requirement. This function will perform the HBA 
 * processing of an interrupt. Simkzq_sm() first calls Sii_handle_interrupt()
 * to determine whether or not this interrupt represents a state change. If a
 * state change is detected then simkzq_phase_dispatch() is called to handle
 * the new bus phase. This function is entered once for each interrupt 
 * generated by the KZQ.
 *
 * FORMAL PARAMETERS:  		
 *	SIM_SM_DATA *simkzq_sm	- SIM State Machine interrupt context.
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
simkzq_sm(simkzq_sm)
    REGISTER SIM_SM_DATA *simkzq_sm;

{
  
  REGISTER   KZQ_INTR *kzqintr;
  REGISTER   SIM_WS *sim_ws;
  REGISTER   SIMKZQ_REG *kzqreg;
  REGISTER   SIMKZQ_SOFTC *hba_sc;
  REGISTER   SIM_SOFTC *sim_softc;
  U32 controller = NOBTL;
  int s;
  U32 retval;

  SIM_MODULE(simkzq_sm);

  /*
   * Local initailization
   */
  kzqintr = (KZQ_INTR*) simkzq_sm->hba_intr;
  sim_softc = simkzq_sm->sim_sc;
  kzqreg = SIMKZQ_GET_CSR(sim_softc);
  hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;
  

  PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_SM,
	 ("(SIMKZQ_SM) TARGID %D, LUN %D, SEQ_NUM 0X%X,\n",
	  sim_ws->targid, sim_ws->lun, sim_ws->seq_num));
  
  PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_SM,
	 (" CSR 0X%X, CSTAT 0X%X, DSTAT 0X%X\n",kzqintr->csr,
	  kzqintr->cstat, kzqintr->dstat));
  
  /*
   * When we enter the state machine, save away the current state
   * machine context such that other parts of the SIM (i.e. DME) can
   * use this context (interrupt) information.
   *
   * Access to this field need not be locked since it is implicitly
   * synchronized by using the softnet interrupt.
   */
  
  sim_softc->active_interrupt_context = simkzq_sm;
  
  
  /*
   * Get the SIM_WS associated with this interrupt. If there is an
   * active_io use that sim_ws, otherwise return the tmp_ws.
   */
  sim_ws = simkzq_get_ws(sim_softc, hba_sc, kzqreg, kzqintr->cstat, 
			 kzqintr->dstat, kzqintr->comm);


  /*
   * Print which interrupt has been received. Tracing the interrupts here 
   * rather than in the ISR has less and impact on performance since this
   * routine is executed at low IPL.
   */
#ifdef trace_ints
  simkzq_print_interrupt(kzqintr);
#endif 
    
  /*
   * Parse the interrupt's CSRs to determine what action to take in responce to
   * this interrupt. Handle_interrupt decodes the kzqintr structure to
   * what the interrupting condition was and how to proceed from the interrupt.
   */ 
   (void) simkzq_handle_interrupt(kzqintr,sim_softc,hba_sc,kzqreg,sim_ws);
   
 
   /*
    * Reselection may change the active io therefore reassign sim_ws.
    */
    sim_ws = sim_softc->active_io;
			      

  /*
   * Handle interrupt decided when an interrupt was redundant or when it 
   * represents a real state change. 
   * One of the major difficultly with the KZQ is that it generates several
   * interrupts for each state transition on the bus. Since during some of
   * these interrupts the CSR's are not valid, what is attempted here is
   * to decide which interrupts represent a state change and have valid CSR's.
   *
   * In order for a phase change to be valid there must be an active_io, 
   * otherwise don't treat this interrupt as a state change.
   */
  if ((kzqintr->new_state) && (sim_softc->active_io != NULL))
  {			/* New Phase */

      PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INTERRUPT,
		   ("*** State change handle phase %x, %x pending intr.\n",
		    (kzqintr->dstat & KZQ_DSTAT_PHASE),
		    sim_sm.sm_queue.curr_cnt));

      /*
       * On the odd case where the target has changed phase during a data
       * transfer, pause the DME to save it's state. During a typical double
       * buffered DMA, DME_PAUSE would have already been called in response
       * to the DNE interrupt, however in the case of phase changes the DNE
       * maynot be received depending on the next state the KZQ enters and the
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
      hba_sc->last_int_phase = kzqintr->dstat & KZQ_DSTAT_PHASE;


      /*
       * Now that we have handled the original interrupting condition, 
       * dispatch on the current bus phase to a phase specific routine. 
       * There is one routine for each of the SCSI bus phases.
       */
      if ( 
	  simkzq_phase_dispatch(kzqintr, sim_ws) != CAM_REQ_CMP
	  )
      {
	  CAM_ERROR(module,
		     "Error processing bus phase",
		     (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		     sim_softc,sim_ws,kzqintr);
	  /*
	   * An error has occured, that should never happen.
	   * Do the best we can to recover from this event.
	   */
	  simkzq_start_cntrlerr_recovery(sim_softc);
      
      };
      
      /*
       * For debug keep a ptr to interrupt context of last interrupt.
       */
      hba_sc->last_kzq_intr = kzqintr;

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
      if ( (sim_softc->error_recovery & ERR_UNKNOWN) || kzqintr->new_state)
      {  
	  simkzq_start_cntrlerr_recovery(sim_softc);
      }
   
      hba_sc->last_dismissed_kzq_intr = kzqintr; /* Switch over last one */
  };


  /*
   * Reenable interrupts
   */
  SIM_SOFTC_LOCK(s,sim_softc);

  kzqreg->csr |= KZQ_CSR_IE;
  WBFLUSH();
  SIM_SOFTC_UNLOCK(s,sim_softc);


  PRINTD(NOBTL, NOBTL, NOBTL, CAMD_SM,("(simkzq_sm) end\n"));

};



/**
 * simkzq_reselect_interrupt -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is entered when an interrupt which represents a 
 * reselection is recieved. Due to the design of the KZQ there are actually
 * two ways the KZQ indicates a reselection:
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
 *	KZQ_INTR *kzqintr	- Current interrupt context.
 *	SIM_WS *sim_ws		- Current I/O's state.
 * 	SIMKZQ_REG *kzqreg	- KZQ CSR's.
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
simkzq_reselect_interrupt(kzqintr, sim_ws, kzqreg, sim_softc)

  REGISTER   KZQ_INTR *kzqintr;
  REGISTER   SIM_WS *sim_ws;
  REGISTER   SIMKZQ_REG *kzqreg;
  REGISTER  SIM_SOFTC *sim_softc;
{
	      
  U32 controller = NOBTL;
  int s;
  U32 retval,msg_cnt;
  SIMKZQ_SOFTC *hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;


  SIM_MODULE(simkzq_reselection_interrupt);

  PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_PHASE,
	 ("(simkzq_reselect_interrupt) phase is scsi_reselection\n"));
  
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
	untimeout(simkzq_sel_timeout,tmp_ptr);

	/*
	 * Clear these fields before calling reschedule.
	 */
	SIM_SOFTC_LOCK(s,sim_softc);
	sim_softc->flags &= ~SZ_TRYING_SELECT;
	hba_sc->sws = NULL;		/* For good measure clear this */
	SIM_SOFTC_UNLOCK(s,sim_softc);

    
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
   * reselection interrupt (Isn't the KZQ great!).
   *
   * The reason that this check is needed is that sometimes, multiple 
   * interrupting conditions will be delivered at the same time by the
   * KZQ. If a reselection is real fast, DNE and RESELECT interrupts
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
      SIM_SOFTC_LOCK(s,sim_softc);
      sim_softc->active_io = NULL;
      SIM_SOFTC_UNLOCK(s,sim_softc);

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
  sc_setup_ws(sim_softc, sim_ws,(U32) kzqreg->destat, 0L);

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

};		/* simkzq_reselect_interrupt */



/**
 * simkzq_bus_reset -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Perform a RESET of the SCSI bus, logs an error and resets the KZQ chip.
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
simkzq_bus_reset(sim_softc)
REGISTER SIM_SOFTC *sim_softc;
{
    REGISTER SIMKZQ_REG *kzqreg;
    REGISTER SIMKZQ_SOFTC *hba_softc;
    u_long retval;
    int s, s1;

    SIM_MODULE(simkzq_bus_reset);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_bus_reset) begin\n"));

    kzqreg = SIMKZQ_GET_CSR(sim_softc);


    CAM_ERROR(module,
	       "HBA initiated SCSI bus reset",
	      SIM_LOG_PRISEVERE,sim_softc,sim_softc->active_io,NULL);

    /*
     * Raise ipl and smp lock.
     */
    SIM_SOFTC_LOCK(s, sim_softc);
			      
    hba_softc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;

    /*
     * If a selection is in progress clear it.....
     */
    simkzq_clear_selection( sim_softc, hba_softc, kzqreg);


    /*
     * Reset the SII chip to a known state before we issue a bus reset.
     */
    sim_init_kzq(sim_softc);

    /*
     * Assert reset on the scsi bus
     */
    kzqreg->comm |= KZQ_COMM_RST;
    WBFLUSH();

    /*
     * Since the reset line is not latched in the chip
     * We can be keep at high IPL for longer then the 25usec
     * the reset pulse is on the line..In other words we
     * can miss the reset interrupt.....
     * So just spin waiting for the it to be seen up to 25 usec's
     */
    CAM_DELAY( 25, (kzqreg->cstat & KZQ_CSTAT_RST), retval );

    /* 
     * If the event did not happen try it one more time
     */
    if (retval != CAM_REQ_CMP ){
	kzqreg->comm |= KZQ_COMM_RST;
    WBFLUSH();
	CAM_DELAY( 25, (kzqreg->cstat & KZQ_CSTAT_RST), retval );

	if( retval != CAM_REQ_CMP ){
	    CAM_ERROR(module, "HBA tried a SCSI bus reset, but couldn't",
		    0,sim_softc,sim_softc->active_io,NULL);

	    SIM_SOFTC_UNLOCK(s, sim_softc);
	    return(retval);
	}
    }

    hba_softc->cnt_bus_resets++;
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
    CAM_DELAY( 25, !(kzqreg->cstat & KZQ_CSTAT_RST), retval );

    /*
     * Reset the SII chip to a known state after we issue a bus reset.
     */
    sim_init_kzq(sim_softc);

    /*
     * Unlock and lower ipl.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);
    

    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_bus_reset) end\n"));

    return(CAM_REQ_CMP);
}


/**
 * simkzq_clear_parity_error - 
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called to clear the parity error condition in KZQ.
 * NOTE: There is NO SAFE way to clear this condition in the KZQ. The
 * KZQ specification indicates that a transfer info command should be issued
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
U32 simkzq_clear_parity_error(kzqreg,sim_softc,sim_ws)

REGISTER     SIMKZQ_REG *kzqreg;
REGISTER     SIM_SOFTC *sim_softc;
REGISTER     SIM_WS *sim_ws;
{
    u_short dmlotc;
    u_short kzq_cmd;
    U32 retval;

    /*
     * First check to determine whether the parity error bit is
     * actualy still set, if not return success.
     */
    if (kzqreg->dstat & KZQ_DSTAT_IPE)
    {
	/* 
	 * Since the KZQ will only clear the Parity Error bit when a DMA
	 * command is issued to the chip, we will issue a DMA for ZERO bytes
	 * to the KZQ to clear the parity error bit in the DSTAT register.
	 */
	dmlotc = kzqreg->dmlotc;	/* Save original value */
	kzqreg->dmlotc = 0;

	/*
	 * Issue a transfer info command with dma and the phase set to
	 * the current phase to start a DMA transfer. The KZQ requires that
	 * bits 0-2 of DSTAT and 4-6 of CSTAT be the same for the COMM reg
	 *
	 * During error recovery keep ATN asserted.
	 */
	kzq_cmd = ((kzqreg->cstat &  KZQ_CSTAT_STATE) |
		   (kzqreg->dstat & KZQ_DSTAT_PHASE)|	
		   (KZQ_COMM_INFO_XFER | KZQ_COMM_ATN));

/* 
 * NOTE:
 * It is not acceptable to issue a transfer info command when 
 * recovering from a parity error. At this point we have decided
 * to simply reset the bus and go one. The KZQ specification indicates
 * that the way to clear the IPE bit is to issue some form of DMA operation.
 * There are several data integritty issues with this approach, thus we have
 * elected to simply leave this bit set and allow the caller to determine
 * how best to recover from this event.
 */
/*	CMD_PENDING(sim_softc->hba_sc, kzq_cmd,kzqreg); */



	kzqreg->dmlotc = dmlotc;	/* Restore original value */

	kzqreg->comm |= KZQ_COMM_ATN;

	/*
	 * Before we lower IPL, clear any DNE interrupts that might be pending
	 * as the result of the DMA performed above. This is done by
	 * writing a 1 to the DNE bit in the DSTAT register.
	 */
	kzqreg->dstat =  KZQ_DSTAT_DNE;
	WBFLUSH();

	/*
	 * Did IPE clear, if not return error, otherwise return success.
	 */
	if (!(kzqreg->dstat & KZQ_DSTAT_IPE))
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
}		/* simkzq_clear_parity_error */


/**
 * simkzq_clear_selection -
 *
 * FUNCTIONAL DESCRIPTION:
 * 	This function is called to clear an active selection on the KZQ.
 * The HBA selection state is cleared, ATN is deassert and reselections 
 * are reenabled. 
 *
 * FORMAL PARAMETERS:  		
 * 	SIM_SOFTC *sim_softc	- SIM Software state.
 *	SIMKZQ_SOFTC *hba_sc	- HBA specific SIM state.
 *	SIMKZQ_REG *kzqreg	- KZQ CSR's.
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
void 
simkzq_clear_selection(sim_softc,hba_sc,kzqreg)
REGISTER     SIM_SOFTC *sim_softc;
REGISTER     SIMKZQ_SOFTC *hba_sc;
REGISTER     SIMKZQ_REG *kzqreg;
{


    SIM_MODULE(simkzq_clear_selection);

    /*
     * untimeout the selection thread if one is pending
     */
    if( hba_sc->sws != (SIM_WS *)NULL){
	untimeout( simkzq_sel_timeout, hba_sc->sws);
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
    kzqreg->comm &= (~KZQ_COMM_ATN);
    WBFLUSH();



    /*
     * It's safe now to enable reselections, since we know that we didn't lose
     * arbitration and that we weren't reselected. Thus there is no chance for
     * ATN to be dangling. If we reenable reselection prior to knowing this,
     * since there is a SELECT with ATN command in the COMM register, ATN might
     * be asserted by the KZQ when a target reselects us.
     *
     * NOTE: The KZQ won't assert ATN on the SCSI bus until the CON bit
     * in the cstat register is set and that clearing the RSE bit in CSR not
     * only disables the reselection interrupt, but also disables the KZQ from
     * responding to reselections.
     *
     * Reenable RESELECTION. 
     * This is a workaround for an KZQ problem which interacts with 
     * the RDAT to cause problems during reselection of the RDAT if
     * the KZQ was attempting to arbitrate and select a target while the
     * RDAT was reselecting. The "solution" was to always disable reselection
     * while the KZQ was selecting a target.
     */
    kzqreg->csr    |= (KZQ_CSR_RSE);
    WBFLUSH();
    
};

/**
 * simkzq_sel -
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
simkzq_sel(sim_ws)
REGISTER SIM_WS *sim_ws;

{			/* simkzq_sel */
    REGISTER     u_char *cdb;
    REGISTER     SIM_SOFTC *sim_softc;
    REGISTER     SIMKZQ_REG *kzqreg;
    REGISTER     SIMKZQ_SOFTC *hba_sc;
    int s;
    U32 retval;		      
    u_short cstat,csr,tmp1_dstat, tmp1_cstat, tmp1_sc1, 
		tmp2_dstat, tmp2_cstat, tmp2_sc1;

    SIM_MODULE(simkzq_sel);

    /*
     * Local setup of variables
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    kzqreg = SIMKZQ_GET_CSR(sim_softc);
    hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_sel) begin\n"));


    SIM_SOFTC_LOCK(s,sim_softc);
    
	
    /*
     * Is sz_trying_select already set? If so rescehdule this request since
     * somehow we are already trying to select a device?
     */
    if (sim_softc->flags & SZ_TRYING_SELECT)
    {
	SIM_SOFTC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    };

    /*
     * Did another target already get on the bus? If so, reschedule this 
     * request. The sim_softc->active_io is used by this SIM determine which
     * SIM_WS is active (in a cmd, data, message or status phase) in the SIM.
     */
    if (sim_softc->active_io != (SIM_WS *)NULL)
    {
	SIM_SOFTC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    }


    /*
     * Setup ID of target to be selected on the SCSI bus.
     */
    kzqreg->slcsr = sim_ws->targid;

    /*
     * Save the address of the I/O (sim_ws) which is attempting to select a
     * target. During the selection the "active_io" is kept in the hba_sc.
     * This is done since there is NO guarantee that since we are about
     * select a target, that another target might have already reselected
     * or is about to reselect us.
     */
    hba_sc->sws = sim_ws;

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("Issue Select Command to KZQ.\n"));

    /*
     * NOTE 1: That we are always selecting with ATN.
     *
     * NOTE 2: There is a bug in the KZQ where it will deassert ATN
     * after selection completes, if we were selecting with ATN. The 
     * workaround is to always set ATN in the message processing code
     * where needed.
     *
     * Note 3: There is a bug in the KZQ where if were selecting with ATN
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
    kzqreg->csr &= ~KZQ_CSR_RSE;
    WBFLUSH();

    /*
     * After we have disabled reselection, insure that we weren't reselected
     * just before we disabled it. If we were reselected, don't try to select.
     * If the bus has BSY or SEL is asserted, we must have already been 
     * reselected, thus don't try to select.
     */
    if (kzqreg->sc1 & (KZQ_SC1_SEL  | KZQ_SC1_BSY))
    {
	hba_sc->cnt_resched_sel++;
	simkzq_clear_selection(sim_softc,hba_sc,kzqreg);
	SIM_SOFTC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);
    };


    /*
     * Set the sim_softc "flags" field to the TRYING_SELECT state. This
     * prevents another selection from being started.
     */
    sim_softc->flags |= SZ_TRYING_SELECT;

    /*
     * We always SELECT with ATN. Now issue the SELECTION command to the KZQ.
     */
    CMD_PENDING(sim_softc->hba_sc,KZQ_COMM_SELECT_ATN,kzqreg);
    WBFLUSH(); 

    /*
     * NOTE:
     * Before we continue and wait for a timeout insure that the selection
     * actually started, wait around for the KZQ to at least start the command.
     * If the KZQ doesn't start the selection, then backoff and retry the 
     * selection since it's likely that we are being reselected during this
     * interval. If we don't wait for selection to start, it's possible that we
     * might either reenable reselections too soon or too late.
     *
     * This is necessary since the KZQ is slow in setting the LST (Lost arbit)
     * bit, during a selection attempt and we are reselected. Without this 
     * we might leave reselections disabled and cause a reselecting target to
     * timeout during it's reselections.
     */
    tmp1_cstat = kzqreg->cstat, tmp1_dstat = kzqreg->dstat, 
    tmp1_sc1 = kzqreg->sc1;
    CAM_DELAY(ARB_WIN_TIME,kzqreg->cstat & 
       (KZQ_CSTAT_SCH|KZQ_CSTAT_SIP|KZQ_CSTAT_LST|KZQ_CSTAT_DST|KZQ_CSTAT_CON),
	      retval);
    tmp2_cstat = kzqreg->cstat, tmp2_dstat = kzqreg->dstat, 
				tmp2_sc1 = kzqreg->sc1;

    /*
     * If arbitration/selection doesn't start then assume that we lost 
     * arbitration, clear the selection and retry later.
     */
    if (!(retval == CAM_REQ_CMP))
    { 
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("SIMKZQ_SEL: select didn't start, %x, %x\n",
		kzqreg->cstat,kzqreg->dstat));

	hba_sc->cnt_resched_sel++;
	simkzq_clear_selection(sim_softc,hba_sc,kzqreg);
	SIM_SOFTC_UNLOCK(s,sim_softc);
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
    if ( (kzqreg->cstat & KZQ_CSTAT_LST) ||  (kzqreg->cstat & KZQ_CSTAT_DST))
    {
	hba_sc->cnt_resched_sel++;
	simkzq_clear_selection(sim_softc,hba_sc,kzqreg);
	SIM_SOFTC_UNLOCK(s,sim_softc);
	return(CAM_BUSY);

    }	/* If we lost arbitration or were reselected */


    /*
     * Since timeouts are not possible during the boot sequence 
     * (SOFTCLOCK not running) we will need to spin waiting for selection
     * to complete. 
     * However, after boot we will use the timeout mechanism to timeout
     * selections that fail to complete.
     */
    if( cam_at_boottime()) 
    {

	/*
	 * Spin for SCH to be set, allow longer for selection than strickly
	 * required by the specificiation since some devices like RZ24 might
	 * be slow to respond.
	 */
	CAM_DELAY(SELECT_TIME,kzqreg->cstat & KZQ_CSTAT_SCH ,retval);
	
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
		   ("(simkzq_sel) Selection completed, stop selection\n"));
	    /* 
	     * Unlock before we call simkzq_sel_timeout
	     * Since we a simulating a timeout.. the routine
	     * will relock.
	     */
	    SIM_SOFTC_UNLOCK(s,sim_softc);
	    
	    /*
	     * Simulate a timeout ocurring during the selection process.
	     */
	    simkzq_sel_timeout(sim_ws);

	    return(CAM_REQ_CMP);
	}
	
	/*
	 * Since interrupts are enabled, the selection completion notification
	 * will occur with a SCH interrupt from the KZQ.
	 */

    }
    else
    {			/* Not Boottime */
	/*
	 * Schedule a timeout to occur to catch the case where the target,
	 * doesn't select.
	 */
	(void) timeout(simkzq_sel_timeout, sim_ws, KZQ_SEL_TMO);

    }			/* Not Boottime */


    SIM_SOFTC_UNLOCK(s,sim_softc);
			    
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_sel) end\n"));


    return(CAM_REQ_CMP);
};   			/* simkzq_sel */


/**
 * simkzq_sel_timeout -
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
simkzq_sel_timeout(sim_ws)
    SIM_WS *sim_ws;

{
    SIM_SOFTC *sim_softc;
    SIMKZQ_SOFTC *hba_sc;
    REGISTER     SIMKZQ_REG *kzqreg;
    int s, s1;

    SIM_MODULE(simkzq_sel_timeout);

    /*
     * Local setup of variables
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;
    kzqreg = SIMKZQ_GET_CSR(sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	   ("(simkzq_sel_timeout) Selection failed, stop selection\n"));

    /*
     * Since the selection timed out, clear the flag indicating that a 
     * selection is in progress.
     */			      
    SIM_SOFTC_LOCK(s,sim_softc);

    /*
     * If selection is still in progress, then DISCON the KZQ from the
     * bus.
     */
    if (kzqreg->cstat & KZQ_CSTAT_SIP)
    {
	CMD_PENDING(sim_softc->hba_sc,KZQ_COMM_DISCON,kzqreg);
	WBFLUSH(); 

    };

    /*
     * Clear the selection state, ATN and reenable RESELECTIONS 
     */
    simkzq_clear_selection(sim_softc,hba_sc,kzqreg);

    SIM_SOFTC_UNLOCK(s,sim_softc);

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
};			/* simkzq_sel_timeout */


/**
 * simkzq_send_cmd -
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
simkzq_send_cmd(sim_ws)
REGISTER   SIM_WS *sim_ws;

{
    U32   retval;
    REGISTER SIMKZQ_REG *kzqreg;	/* KZQ csr's			   */
    REGISTER SIM_SOFTC *sim_softc;
    REGISTER u_char *cdb;

    SIM_MODULE(simkzq_send_cmd);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("simkzq_send_cmd:)entered\n"));

    /*
     * Local setup of variables
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    kzqreg = SIMKZQ_GET_CSR(sim_softc);

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
    retval = simkzq_xfer_info(sim_ws, cdb,(U32)sim_ws->ccb->cam_cdb_len,
			      CAM_DIR_OUT);
    if (retval != CAM_REQ_CMP)
    {
	CAM_ERROR(module,
		   "Error during transfer of SCSI command bytes",
		   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,sim_ws,NULL);
	
	/*
	 * Start Error processing in command phase at next transition.
	 */
	sim_ws->error_recovery |= ERR_UNKNOWN;
    }

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("simkzq_send_cmd:)exited\n"));
    return(retval);
};			/* simkzq_send_cmd */

/**
 * simkzq_xfer_info -
 *
 * FUNCTIONA LDESCRIPTION:
 *	simkzq_xfer_info() will perform a scsi transfer (could be any
 * phase) bypassing the dme interface.  The data located at "buf" will be 
 * transfered using programmed I/O over the bus. The data transfers here use
 * the KZQ in a very low level mode, each byte is handshaked using programed 
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
simkzq_xfer_info(sim_ws, buf, cnt, dir)
    REGISTER SIM_WS *sim_ws;
    REGISTER u_char *buf;		/* pointer to local transfer point */
    REGISTER U32 cnt; 		/* number of bytes to transfer     */
    U32 dir; 			/* direction cam_dir_out/in 	   */


{
    REGISTER SIMKZQ_SOFTC *hba_softc;
    REGISTER u_char achar;
    REGISTER SIM_SOFTC *sim_softc;
    REGISTER SIMKZQ_REG *kzqreg;	/* KZQ csr's			   */
    int s;
    U32 bcnt;			/* Loop byte count */
    U32 i,retval;
    u_short kzq_command;	    	/* Command to be sent to the KZQ   */
    U32 controller = NOBTL;

    
    SIM_MODULE(simkzq_xfer_info);

    /*
     * Local initailization
     */
    sim_softc = sim_ws->sim_sc;
    kzqreg = SIMKZQ_GET_CSR(sim_softc);
    hba_softc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_xfer_info) begin\n"));

    /*
     * Raise ipl and smp lock.
     */
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * Decide the direction of the transfer, IN our OUT.
     */
    if (dir & CAM_DIR_OUT)
    {			/* Handshake bytes out */
	

	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_xfer_info) Data OUT handshaking.\n"));

	/*
	 * Send all the bytes requested.
	 */
	for (bcnt = cnt; bcnt > 0; --bcnt)
	{			/* For send all bytes */
	    /*
	     * Wait for REQ to assert, if it hasn't already.
	     *
	     */
	    if (!(kzqreg->sc1 & KZQ_SC1_REQ))
	    {		/* REQ not asserted */

		CAM_DELAY(MAX_BYTE_SPIN,kzqreg->sc1 & KZQ_SC1_REQ,retval);
		
		/*
		 * If we didn't get REQ, error..
		 */
		if ( retval != CAM_REQ_CMP)
		{
		    CAM_ERROR(module,
			       "REQ failed to assert in handshake OUT",
			       (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			       sim_softc,sim_ws,NULL);
		    SIM_SOFTC_UNLOCK(s, sim_softc);
		    return(retval);
		}
	    };		/* REQ not asserted */


	    /*
	     * Before transfering a byte of data insure that the phase has
	     * not changed. If it has log an error and return to the caller.
	     */
	    if (kzqreg->dstat & KZQ_DSTAT_MIS) 
	    {

		/*
		 * It may be "normal" for this to happen, if a target
		 * for example rejects a message out sequence from us.
		 * Thus, if we are in a message sequence don't log this
		 * as an error. Simpy return this as an error the caller
		 * and let it decide what to do.
		 */
		if (!(kzqreg->dstat & KZQ_DSTAT_MSG))
		{
		    /*
		     * Phase changed before transfer completed.
		     */
		    CAM_ERROR(module,
			      "Unexpected bus phase change in handshake OUT",
			      (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			      sim_softc,sim_ws,NULL);
		};
		SIM_SOFTC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);
	    }
	

	    /*
	     * Do we need to clear ATN?
	     */
	    if (hba_softc->flags & SIMKZQ_CLR_ATN) {
		hba_softc->flags &= ~SIMKZQ_CLR_ATN;
		kzqreg->comm &= ~KZQ_COMM_ATN;
		WBFLUSH();
	    }

	    /*
	     * Write byte to data bus.
	     */
	    achar = *buf;
	    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		       ("Byte out %x. phase = %x\n",
			achar,(kzqreg->dstat & 7)));
	    SIMKZQ_PUT_BYTE(achar,kzqreg);
	    buf++;
	    

	    /*
	     * In order to start a transfer on the KZQ parts of the CSTAT and 
	     * DSTAT register must match the COMM register. Also, by doing 
	     * this we clear the phase mismatch bit. 
	     * 
	     * NOTE: That we are maintaining the state of the ATN signal as
	     * it is on the bus at this time. The caller of this function
	     * will determine when the ATN signal should be set or cleared.
	     * This routine will simply maintain the state of this signal(ATN).
	     */
	    kzq_command = ((kzqreg->dstat & (KZQ_DSTAT_PHASE|KZQ_DSTAT_ATN)) |
			   (kzqreg->cstat & KZQ_CSTAT_STATE));
	    
	    /*
	     * Issue transfer without DMA for one byte. Note the bus phase
	     * in the COMM register must match the DSTAT register.
	     */
	    kzq_command |= KZQ_COMM_INFO_XFER;

	    /*
	     * Before starting a programmed I/O transfer insure that DNE
	     * is clear. If this is not cleared then slow devices might trick
	     * us into thinking that the following xfer_info command completed
	     * when it didn't.
	     */
	    kzqreg->dstat = KZQ_DSTAT_DNE;

	    /* Issue the KZQ command built up in kzq_command to the KZQ chip */
	    CMD_PENDING(sim_softc->hba_sc,kzq_command,kzqreg);
	    WBFLUSH();
	    
	    /*
	     * Insure that the transfer info command completes.
	     * Wait for DNE (Done) to assert saying the operation
	     * completed.
	     */
	    CAM_DELAY(MAX_BYTE_SPIN,kzqreg->dstat & 
		      (KZQ_DSTAT_DNE|KZQ_DSTAT_MIS),retval);    


	    /*
	     * If DNE or MIS are asserted the target took the byte.
	     */
	    if (retval != CAM_REQ_CMP)
	    {
		CAM_ERROR(module,
			   "DNE not asserted after handshake OUT",
			   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);

		/*
		 * Enable KZQ interrupts
		 */
		kzqreg->csr |= KZQ_CSR_IE;
		WBFLUSH();
		SIM_SOFTC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);

	    };
	    
	    
	};			/* For send all bytes */
	
    }   		/* Handshake bytes out */
    else
    {			/* Handshake bytes in */
	
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_xfer_info) Data IN handshaking.\n"));

	/*
	 * Insure that ATN is deasserted since we are already info IN
	 * phase. ATN asserted during data in my confuse some targets i.e.
	 * RDAT.
	 */
/*	kzqreg->comm &= ~KZQ_COMM_ATN;
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
	    if (!(kzqreg->sc1 & KZQ_SC1_REQ))
	    {		/* REQ not asserted */
		CAM_DELAY(MAX_BYTE_SPIN,kzqreg->sc1 & KZQ_SC1_REQ,retval);

		/*
		 * If we didn't get REQ, error..
		 */
		if ( retval != CAM_REQ_CMP)
		{
		    CAM_ERROR(module,
			       "REQ failed to assert in handshake IN",
			       (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			       sim_softc,sim_ws,NULL);
		    SIM_SOFTC_UNLOCK(s, sim_softc);
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
	    if (kzqreg->dstat & KZQ_DSTAT_MIS)
	    {
		
		/*
		 * Phase changed before transfer completed.
		 */
		CAM_ERROR(module,
			   "Unexpected bus phase change in handshake OUT",
			   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);
	    

		/*
		 * Enable KZQ interrupts and return.
		 */
		kzqreg->csr |= KZQ_CSR_IE;
		WBFLUSH();
		SIM_SOFTC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);
	    }
	

	    /*
	     * Read byte from the SCSI data bus.
	     */
	    SIMKZQ_GET_BYTE(achar,kzqreg);
	    *buf = achar;
	    buf++;

	    /*
	     * Before accepting this byte of data, check it's parity.
	     * If the parity is bad start error recovery.
	     */
	    if (kzqreg->dstat & KZQ_DSTAT_IPE)
	    {
		/*
		 * Parity error detected on incoming byte.
		 */
		CAM_ERROR(module,
			   "KZQ Parity Error",
			   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);

		/*
		 * Increment the parity error count in the NEXUS.
		 */
		sim_ws->nexus->parity_cnt++;

		/*
		 * What phase is this parity error on?
		 */
		if ((kzqreg->dstat & KZQ_DSTAT_PHASE) == SC_PHASE_MSGIN)
		{
		    sim_ws->error_recovery |= ERR_MSGIN_PE;
		}
		else if ((kzqreg->dstat & KZQ_DSTAT_PHASE) == SC_PHASE_DATAIN) 
		{
		    sim_ws->error_recovery |= ERR_DATAIN_PE;
		}
		else if ((kzqreg->dstat & KZQ_DSTAT_PHASE) == SC_PHASE_STATUS) 
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
		 * Force the KZQ to clear the Parity Error condition
		 */
		if (CAM_REQ_CMP != simkzq_clear_parity_error(kzqreg, 
							     sim_softc,sim_ws))
		{
		    CAM_ERROR(module,
			      "Parity error condition failed to clear",
			      SIM_LOG_PRISEVERE,sim_softc,sim_ws,NULL);
		    
		    /*
		     * Since we have been unable to clear the IPE, we have no
		     * choice but to reset the SCSI bus.
		     */
		    simkzq_start_cntrlerr_recovery(sim_softc);
		}

		/*
		 * Enable KZQ interrupts
		 */
		kzqreg->csr |= KZQ_CSR_IE;
		WBFLUSH();
		
		SIM_SOFTC_UNLOCK(s, sim_softc);
		return(CAM_UNCOR_PARITY);
	    };

	    /*
	     * In order to start a transfer on the parts of the CSTAT and 
	     * DSTAT register must match the COMM register. Also, by doing 
	     * this we clear the phase mismatch bit.
	     */
	    kzq_command = (kzqreg->dstat & KZQ_DSTAT_PHASE) |
		(kzqreg->cstat & KZQ_CSTAT_STATE);
	    
	    /*
	     * Issue transfer without DMA for one byte. Note the bus phase
	     * in the COMM register must match the DSTAT register.
	     */
	    kzq_command |= KZQ_COMM_INFO_XFER;
	    
	    /*
	     * Before starting a programmed I/O transfer insure, that DNE
	     * is clear.
	     */
	    kzqreg->dstat = KZQ_DSTAT_DNE;

	    /*
	     * NOTE:
	     * Sometimes the KZQ get's stuck and won't execute the next 
	     * command unless the previous command is cleared from the COMM 
	     * register. What we do here is clear the COMM register to reset
	     * the chip to a state where it will accept the next command.
	     * Since interrupts are now disabled, the clear of the COMM 
	     * register will have no adverse effects on the KZQ.
	     * 
	     * An example of when the KZQ get's stuck is on a msgin
	     * phase (0x02 - Save Pointers) after a reselection followed by a
	     * short datain phase.
	     */
	    kzqreg->comm = 0;
	    WBFLUSH();

	    /* Execute this KZQ function */
	    CMD_PENDING(sim_softc->hba_sc,kzq_command,kzqreg);
	    WBFLUSH();
	  
	    /*
	     * Insure that the transfer info command completes.
	     */
	    CAM_DELAY(MAX_BYTE_SPIN,kzqreg->dstat & 
		      (KZQ_DSTAT_DNE|KZQ_DSTAT_MIS|KZQ_DSTAT_IBF),retval);
	
	    /*
	     * If DNE or MIS are asserted, then we took the byte
	     */
	    if (retval != CAM_REQ_CMP)
	    {
		CAM_ERROR(module,
			   "DNE not asserted after handshake OUT",
			   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,sim_ws,NULL);

		/*
		 * Enable KZQ interrupts
		 */
		kzqreg->csr |= KZQ_CSR_IE;
		WBFLUSH();
		SIM_SOFTC_UNLOCK(s, sim_softc);
		return(CAM_DATA_RUN_ERR);
	    };
	    
	};			/* For receieve all bytes */
	
    };			/* Handshake bytes in */
    

    /*
     * Before we lower IPL, clear any DNE interrupts that might be pending
     * as the result of the programmed I/O performed above. This is done by
     * writing a 1 to the DNE bit in the DSTAT register.
     */
    kzqreg->dstat =  KZQ_DSTAT_DNE;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_xfer_info) end\n"));

    return(retval);
}

/**
 * simkzq_req_msgout -
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
simkzq_req_msgout(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    int s;
    REGISTER SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER SIMKZQ_REG *kzqreg = SIMKZQ_GET_CSR(sim_softc);

    SIM_MODULE(simkzq_req_msgout);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_req_msgout) begin\n"));

    /*
     * Raise ipl and smp lock.
     */
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * Assert the ATN signal on the SCSI bus to force the target into
     * error recovery.
     */
    kzqreg->comm |= KZQ_COMM_ATN;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_req_msgout) end\n"));

    return(CAM_REQ_CMP);
}

/**
 * simkzq_clear_msgout -
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
simkzq_clear_msgout(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    int s;
    REGISTER SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER SIMKZQ_REG *kzqreg = SIMKZQ_GET_CSR(sim_softc);

    SIM_MODULE(simkzq_clear_msgout);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_req_msgout) begin\n"));

    /*
     * Raise ipl and smp lock.
     */
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * Deassert the ATN signal on the SCSI bus.
     */
    kzqreg->comm &= ~KZQ_COMM_ATN;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_req_msgout) end\n"));

    return(CAM_REQ_CMP);
}		/* simkzq_clear_msgout */

/**
 * simkzq_send_msg- 
 *
 * FUNCTIONAL DESCRIPTION:
 *	simkzq_send_msg() assumes that the current bus phase is
 * message out.  All bytes in the message out queue will be sent to the 
 * target one by one calling simkzq_xfer_info.
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
simkzq_send_msg(sim_ws)
    REGISTER SIM_WS *sim_ws;
{
    int s;
    u_char achar;		      /* Hold message byte to send */
    REGISTER SIM_SOFTC *sim_softc;
    REGISTER SIMKZQ_REG *kzqreg; /* KZQ csr's */
    REGISTER SIMKZQ_SOFTC *hba_softc;
    REGISTER u_short i, count;
    
    SIM_MODULE(simkzq_send_msg);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_send_msg) begin, sending %d bytes\n", count));

    /*
     * Local intialization
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    kzqreg = SIMKZQ_GET_CSR(sim_softc);
    count = SC_GET_MSGOUT_LEN(sim_ws);
    hba_softc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;

    /*
     * Raise ipl and smp lock.
     */
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * Send the message bytes to the target one byte at a time.
     * This done in order to allow us to respond to message reject
     * or other phase changes.
     */
    for (i=0; i < count; i++) 
    {			/* For loop */
	PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_MSGOUT,
	       ("(simkzq_send_msg) message out: 0x%x\n",
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
	    hba_softc->flags |= SIMKZQ_CLR_ATN;
	}
	else
	{
	    /*
	     * Due to a problem in the KZQ, where during a selection with
	     * ATN the KZQ will clear ATN after the selection completed,
	     * insure that ATN is asserted when the first message byte is
	     * sent to the  target.
	     */
	    kzqreg->comm |= KZQ_COMM_ATN;
	    WBFLUSH();
	}
		
	/*
	 * Get the message byte about to be transfered.
	 */
	achar = (u_char) SC_GET_MSGOUT(sim_ws, i);

	/*
	 * Send this message byte and exit loop if an error occurs.
	 */
	if (simkzq_xfer_info(sim_ws, &achar, 1, CAM_DIR_OUT) != CAM_REQ_CMP)
	    break;
	
	
	/*
	 * Insure that the phase doesn't change while sending message bytes
	 * out. If the phase does change, then simply record how many message
	 * bytes were actually sent and await the MIS interrupt for the phase
	 * change.
	 */
	if (kzqreg->dstat & KZQ_DSTAT_MIS) break;

	    
    }		/* For loop */
    
    /*
     * Insure that ATN is deasserted since all the message bytes have
     * been sent.
     */
    kzqreg->comm &= ~KZQ_COMM_ATN;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);

    /*
     * The assumption here is that the caller will detect that not all the
     * message bytes have been sent and then recover. Recovery at this point is
     * will be difficult. The caller must clear this condition or it is
     * possible that we will continue to loop getting TBE interrupts!
     */
    SC_UPDATE_MSGOUT(sim_ws, (i == count) ? count : i+1 );


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_send_msg) end\n"));

    return(CAM_REQ_CMP);
}

/**
 * simkzq_msg_accept -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simkzq_msg_accept() cause the KZQ to issue an ACK for a pending message
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
simkzq_msg_accept(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    u_char achar;
    REGISTER SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER SIMKZQ_REG *kzqreg = SIMKZQ_GET_CSR(sim_softc);
    U32 retval = CAM_REQ_CMP;
    int s;

    SIM_MODULE(simkzq_msg_accept);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_msg_accept) begin\n"));

    /*
     * Local intialization
     */
    sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;

    retval = simkzq_xfer_info(sim_ws, &achar, 1, CAM_DIR_IN);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_msg_accept) end\n"));

    return(retval);
}


/**
 * simkzq_setup_sync -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This function is called to setup the REQ_ACK offset and synchronous
 * period for the KZQ. This information is retrieved from the it_nexus 
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
simkzq_setup_sync(sim_ws)

REGISTER SIM_WS *sim_ws;

{
    int s;
    REGISTER SIM_SOFTC *sim_softc = 
	(SIM_SOFTC *)sim_ws->sim_sc;    
    REGISTER  SIMKZQ_REG *kzqreg = SIMKZQ_GET_CSR(sim_softc);


    SIM_MODULE(simkzq_setup_sync);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	   ("[%d/%d/%d] (simkzq_setup_sync) begin, period 0x%x, offset 0x%x\n",
	    sim_ws->cntlr, sim_ws->targid, sim_ws->lun,
	    sim_ws->it_nexus->sync_period, sim_ws->it_nexus->sync_offset ));
    
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * Set the DEC's KZQ synchronous offset and period.
     * Use "sync_offset" and "sync_period" from the IT_NEXUS.
     */
    /* sim_ws->it_nexus->sync_period, cannot be changed for the kzq. */
    kzqreg->dmctrl = sim_ws->it_nexus->sync_offset;
    WBFLUSH();

    SIM_SOFTC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("[%d/%d/%d] (simkzq_setup_sync) end\n",
		sim_ws->cntlr, sim_ws->targid, sim_ws->lun ));

    return(CAM_REQ_CMP);
}

/**
 * simkzq_discard_data -
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
simkzq_discard_data(sim_ws)
REGISTER SIM_WS *sim_ws;
{
    REGISTER 	SIM_SOFTC *sim_softc = (SIM_SOFTC *)sim_ws->sim_sc;
    REGISTER	SIMKZQ_REG *kzqreg = SIMKZQ_GET_CSR(sim_softc);
    U32 retval = CAM_REQ_CMP;
    u_char achar;
    int s;
    u_short kzq_command;	    	/* Command to be sent to the KZQ   */

    SIM_MODULE(simkzq_discard_data);
    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_discard_data) begin\n"));


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	   ("(simkzq_discard_data) Assert and Clear ACK.\n"));

    /*
     * Raise ipl and smp lock.
     */
    SIM_SOFTC_LOCK(s, sim_softc);

    /*
     * If after the first byte the phase has not changed, then continue
     * to transfer bytes. Since the MIS bit is not valid until the COMM
     * register is written and writing the COMM register will cause a
     * byte to be transfered, the check for MIS is only valid after 
     * the first byte.
     */
    if (kzqreg->dstat & KZQ_DSTAT_MIS)
    {
	
	/*
	 * Phase changed before transfer completed.
	 */
	CAM_ERROR(module,
		  "Unexpected bus phase change discarding data",
		  SIM_LOG_PRISEVERE,
		  sim_softc,sim_ws,NULL);
	/*
	 * Enable KZQ interrupts
	 */
	kzqreg->csr |= KZQ_CSR_IE;
	WBFLUSH();
	SIM_SOFTC_UNLOCK(s, sim_softc);
	return(CAM_DATA_RUN_ERR);
    }
    
    /*
     * In order to start a transfer on the parts of the CSTAT and 
     * DSTAT register must match the COMM register. Also, by doing 
     * this we clear the phase mismatch bit.
     */
    kzq_command = (kzqreg->dstat & KZQ_DSTAT_PHASE) |
	(kzqreg->cstat & KZQ_CSTAT_STATE);
    
    /*
     * Issue transfer without DMA for one byte. Note the bus phase
     * in the COMM register must match the DSTAT register.
     */
    kzq_command |= KZQ_COMM_INFO_XFER | KZQ_COMM_ATN;
    
    
    /*
     * Before starting a programmed I/O transfer insure, that DNE
     * is clear.
     */
    kzqreg->dstat = KZQ_DSTAT_DNE;
    
    /*
     * NOTE:
     * Sometimes the KZQ get's stuck and won't execute the next 
     * command unless the previous command is cleared from the COMM 
     * register. What we do here is clear the COMM register to reset
     * the chip to a state where it will accept the next command.
     * Since interrupts are now disabled, the clear of the COMM 
     * register will have no adverse effects on the KZQ.
     * 
     * An example of when the KZQ get's stuck is on a msgin
     * phase (0x02 - Save Pointers) after a reselection followed by a
     * short datain phase.
     */
    kzqreg->comm = 0;
    WBFLUSH();
    
    /* Execute this KZQ function */
    CMD_PENDING(sim_softc->hba_sc,kzq_command,kzqreg);
    WBFLUSH();
    
    /*
     * Insure that the transfer info command completes.
     */
    CAM_DELAY(MAX_BYTE_SPIN,kzqreg->dstat & 
	      (KZQ_DSTAT_DNE|KZQ_DSTAT_MIS|KZQ_DSTAT_IBF),retval);
    
    /*
     * If DNE or MIS are asserted, then the we took the byte
     */
    if (retval != CAM_REQ_CMP)
    {
	CAM_ERROR(module,
		   "DNE not asserted during discard data",
		   (SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),sim_softc,sim_ws,NULL);

	/*
	 * Enable KZQ interrupts
	 */
	kzqreg->csr |= KZQ_CSR_IE;
	WBFLUSH();
	SIM_SOFTC_UNLOCK(s, sim_softc);
	return(CAM_DATA_RUN_ERR);
    };
    
  
    /*
     * Before we lower IPL, clear any DNE interrupts that might be pending
     * as the result of the programmed I/O performed above. This is done by
     * writing a 1 to the DNE bit in the DSTAT register.
     */
    kzqreg->dstat =  KZQ_DSTAT_DNE;
    WBFLUSH();

    /*
     * Unlock and lower ipl.
     */
    SIM_SOFTC_UNLOCK(s, sim_softc);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_INOUT,
	       ("(simkzq_discard_data) end\n"));

    return(retval);
    
};		/* simkzq_discard_data */

/**
 * kzq_intr -
 *
 * FUNCTIONAL DESCRIPTION:
 *	This is the Interrupt Service Routine (ISR) for the KZQ CAM HBA.
 * This code is entered once for each hardware interrupt generated by the 
 * the KZQ. Except for a few special cases and workarounds for KZQ problems
 * this code main purpose is to snapshot the KZQ registers and place them in
 * a structure which can be queued to the state machines interrupt queue.
 * Then a software interrupt will be scheduled to wakeup the state machine
 * (SCKZQSR) at low IPL to remove the interrupt context from this queue and
 * deliver this interrupt to the simkzq_sm().
 *
 * FORMAL PARAMETERS:  		
 *	u_short controller	- Controller number for the KZQ on this system
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
kzq_intr( controller ) /* Name changed for ISV release */
u_short controller;
{
    REGISTER SIM_SOFTC *sim_softc = SC_GET_SOFTC(controller);
    REGISTER SIMKZQ_REG *kzqreg; 
    REGISTER SIMKZQ_SOFTC *hba_softc;
    REGISTER u_short csr, cstat, dstat, comm, sc1, sdb, dmlotc;
    extern SIM_SM sim_sm;
    extern int netisr;
    int s1, s2, s3;
    U32 retval;
    u_short kzq_comm;		/* Build up KZQ command */
    u_short orig_dstat;
    SIM_WS *tsws;
    DME_DESCRIPTOR *tdme;

    SIM_MODULE(kzq_intr);
    PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
               ("(kzqintr) begin\n"));


    /*
     * If there are interrupts pending from the KZQ before the CAM subsystem
     * has been initialized, then we may be called at this entry point to
     * process an interrupt before the data structures exist to process
     * this interrupt. At this time the only feasiable solution is to
     * dismiss the interrupt and disable interrupts until the subsystem has
     * initialized. 
     */
     /*
    if (sim_softc == NULL) 
    {
	cam_kzq_spurious_interrupt_before_initialization(controller);
	return;
    }
    */

    /* 
     * Lock ISR to prevent other interrupts from occuring
     */
    SIM_SOFTC_LOCK(s1,sim_softc);

    /*
     * Setup local variables 
     */
    kzqreg = SIMKZQ_GET_CSR(sim_softc);
    
    /*
     * In the odd case that we disabled interrupts, but there was already one
     * latched, then simply return from the ISR.
     */
    if (!(kzqreg->csr & KZQ_CSR_IE))
    {
	CAM_ERROR(module,
		   "IE not enabled, but got interrupt",
		   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,NULL,NULL);
        SIM_SOFTC_UNLOCK(s1,sim_softc);
	return;
    }
    
    hba_softc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;

    /*
     * Disable other KZQ interrupts.
     */
    kzqreg->csr &= ~KZQ_CSR_IE;
    WBFLUSH();


    /*
     * Read (Snapshot) and save the following KZQ registers:  
     */
    sc1 = kzqreg->sc1;
    sdb = kzqreg->sdb;
    csr = kzqreg->csr;
    cstat = kzqreg->cstat;
    orig_dstat = dstat = kzqreg->dstat;
    dmlotc = kzqreg->dmlotc;


    /*
     * NOTE: To workaround a problem in the KZQ where the OBB is cleared
     * when a MIS interrupt is cleared, create a "soft" copy of the OBB
     * in the DME structure. The DME_PAUSE and DME_END code will check this
     * bit to determine whether the transfer ended on an odd byte boundry.
     *
     * Without this software OBB bit, odd bytes might be lost if the OBB bit
     * was set at the end of data phase and MIS was set.
     */
    if (dstat & KZQ_DSTAT_OBB)
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
               ("(kzqintr) csr 0x%x, cstat 0x%x, dstat 0x%x\n",
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
    if (cstat & KZQ_CSTAT_BER)
    {
	kzqreg->cstat = cstat;	/* Clear the bus error bit 	*/
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
    if (cstat & KZQ_CSTAT_RST)
    {


	/*
	 * If the was a selection inprogress before the reset
	 * came in make sure the timeout thread is cleared by
	 * calling simkzq_clear_selection. This will untimeout
	 * the thread if one is pending
	 */

	simkzq_clear_selection(sim_softc, hba_softc, kzqreg);

	CAM_ERROR(module,
		   "SCSI bus reset detected",
		   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		   sim_softc,NULL,NULL);

        /*
         * Increment the bus reset count in the SIM_SOFTC.
         */
        sim_softc->bus_reset_count++;

	/*
	 * After a SCSI bus reset is detected reinitialize the KZQ to a
	 * known state.
	 */
	sim_init_kzq(sim_softc);

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

    	SIM_SOFTC_UNLOCK(s1,sim_softc);
	return;

    };

    /*
     * KZQ NOTE:
     * The KZQ chip designer indicted that IBF/TBE interrupts will not be 
     * latched unless the phase in the COMM register matches the DSTAT register
     * In order to use IBF/TBE for the first byte of a new phase, MIS must be
     * clear. If MIS is set then, assign the COMM register the same phase as 
     * the DSTAT register (thus clearing MIS) and then wait for IBF or TBE to 
     * be set before handling this interrupt.
     */
    while (dstat & KZQ_DSTAT_MIS)
    { 		/* While MIS set */

        /*
	 * If DNE is asserted, then there is no need to take the following 
	 * steps since, DMLOTC will be zero and we just need to clear MIS.
	 */
         if (!(dstat & KZQ_DSTAT_DNE))
	 {
	     /*
	      * Before attempting to clear the MIS interrupt, we must clear the
	      * dmlotc register and the comm register to fakeout the KZQ into
	      * asserting DNE. If these register are not cleared before we 
	      * write the new phase into the comm register the KZQ will 
	      * errorneously attempt to continue the previous transfer.
	      */
	     kzqreg->dmlotc = 0;	/* Clear out the DMA count register */
	     WBFLUSH();

	     kzqreg->comm = 0;		/* Cause DNE to be asserted         */
	     WBFLUSH();

	     /*
	      * Set the phase, to dismiss the MIS interrupt and to allow the
	      * TBE/IBF interrupts to be delivered. 
	      */
	     kzq_comm = ((dstat & (KZQ_DSTAT_PHASE|KZQ_DSTAT_ATN)) | 
			 (cstat & KZQ_CSTAT_STATE));

	     kzqreg->comm = kzq_comm;	/* Clear the MIS interrupt          */
	     WBFLUSH();

	     /*
	      * Restore the transfer count so that other parts of the SIM can
	      * determine how many bytes were sent.
	      */
	     kzqreg->dmlotc = dmlotc;	/* Restore the DMA count register   */
	     WBFLUSH();

	 }		/* DNE set   */
         else	
	 {		/* DNE Clear */
	     /*
	      * Set the phase, to dismiss the MIS interrupt and to allow the
	      * TBE/IBF interrupts to be delivered. 
	      */
	     kzq_comm = ((dstat & (KZQ_DSTAT_PHASE|KZQ_DSTAT_ATN)) | 
			 (cstat & KZQ_CSTAT_STATE));

	     
	     kzqreg->comm = kzq_comm;	/* Clear the MIS interrupt          */
	     WBFLUSH();

	 };		/* DNE Clear */

	/*
	 * One might think by reading the KZQ specification that the following
	 * code would not be necessary, however due to some targets changing 
	 * phase after asserting REQ and some timing problems with the KZQ, 
	 * we must loop waiting for MIS to clear. In general, this while loop
	 * will not be executed, since MIS will already be set. By waiting for
	 * MIS to clear we guarantee that IBF/TBE interrupt will be delivered.
	 */
	while (kzqreg->dstat & KZQ_DSTAT_MIS)
	{	/* While MIS */

	    kzqreg->comm = kzq_comm;
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
	if (!(kzqreg->dstat & (KZQ_DSTAT_TBE|KZQ_DSTAT_IBF)))
	{

	    CAM_DELAY(MAX_BYTE_SPIN,kzqreg->dstat & 
		      (KZQ_DSTAT_IBF|KZQ_DSTAT_TBE|KZQ_DSTAT_MIS),retval);

	    if (retval != CAM_REQ_CMP)
	    {
		CAM_ERROR(module,
			   "IBF or TBE never set",
			   (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
			   sim_softc,NULL,NULL);
		
		/*
		 * An error has occured that should never happen.
		 * Do the best we can to recover.
		 */
		simkzq_start_cntrlerr_recovery(sim_softc);

	    }
	    
	}	/* If not IBF||TBE */
	 
	 /*
	  * Before processing this interrupt insure that the BER bit is not 
	  * set. If BER is set, dismiss this interrupt and reenable interrupts.
	  * This may occur with devices like RDAT.
	  */
	if (!(kzqreg->cstat & KZQ_CSTAT_BER))
	{
	
	    /*
	     * Reread the DSTAT register so that we process the current state
	     * of this register, in case it has changed.
	     */
	    dstat = kzqreg->dstat;
	}
	else
	{	/* BER set */

	    CAM_ERROR(module,
		       "Bus ERROR Interrupt after MIS interrupt",
		       (SIM_LOG_KZQ_ALL|SIM_LOG_HBA_CSR|SIM_LOG_PRISEVERE),
		       sim_softc,NULL,NULL);
	    
	    /*
	     * Clear any interrupts that were received and reenable KZQ 
	     * interrupts.
	     */
	    KZQ_DISMISS_INTERRUPT(cstat,dstat,kzqreg,sim_softc,s1);
	    return;
	}	/* BER set */


    }		/* While MIS set */

    /*
     * If bus reset recovery is not in progress then schedule a softnet 
     * interrupt, otherwise simply set some flags and reenable interrupts.
     */
    if (!(sim_softc->error_recovery & ERR_BUS_RESET))
    {
	comm = kzqreg->comm;		/* Read the comm register */

	/*
	 * Clear the command outstanding in the KZQ.
	 * This mask will clear all bit other than ATN which can 
	 * effect the state of the KZQ chip.
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

	simkzq_add_sm_queue(sim_softc,csr,cstat,dstat,comm,sc1,sdb,
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
     * Clear any interrupts that were received and reenable KZQ interrupts.
     */
    kzqreg->dstat = dstat;
    kzqreg->cstat = cstat;
    WBFLUSH();

    SIM_SOFTC_UNLOCK(s1,sim_softc);

    PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
	   ("(kzqintr) end\n"));
    return;
}



/**
 * simkzq_get_ws -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simkzq_get_ws will determine the SIM_WS which is related
 * to the current interrupt.  Using the passed interrupt state this
 * routine will determine which SIM_WS this interrupt is for. If it is not 
 * possible to make this determination, the SIM_SOFTC's "tmp_ws" will be used.
 *
 * FORMAL PARAMETERS:  		
 * 	SIM_SOFTC *sim_softc;	- SIM software control structure.
 *	SIMKZQ_SOFTC *hba_softc	- HBA specific control structure.
 *  	u_short csr, cstat, dstat, comm 	- KZQ CSR's
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
simkzq_get_ws(sim_softc, hba_softc, csr, cstat, dstat, comm)

REGISTER SIM_SOFTC *sim_softc;
REGISTER SIMKZQ_SOFTC *hba_softc;
REGISTER u_short csr, cstat, dstat, comm;


{
    SIM_WS *sim_ws;
    int s;

    SIM_MODULE(simkzq_get_ws);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_get_ws) begin\n"));
    
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
	     * from the KZQ during selection which might otherwise get the 
	     * wrong sim_ws assigned to it.
	     */
	    if ((sim_softc->flags & SZ_TRYING_SELECT) /* Selection ongoing */
		&&
		(cstat & KZQ_CSTAT_CON) 	      /* Connected target  */
		&&
		(!(cstat & KZQ_CSTAT_DST)))	      /* Not Reselected    */

	    {		/* During Selection */
		
		SIM_SOFTC_LOCK(s,sim_softc);
		
		/*
		 * Setup address of active IO from the address of the IO
		 * which was attempting selection.
		 */
		sim_softc->active_io = hba_softc->sws;
		
		SIM_SOFTC_UNLOCK(s,sim_softc);
		return(sim_softc->active_io);
	    }		/* During selection */
	    else
		
	    {		/* NOT during selection */
		/*
		 * Set the address of the active to be the temporary working
		 * set and initialize the sim_ws.
		 */
		SIM_SOFTC_LOCK(s,sim_softc);
/*		sim_softc->active_io = &sim_softc->tmp_ws;*/

		/*
		 * Setup the temporary working set for use during this
		 * interrupt.
		 */
		sc_setup_ws(sim_softc, &sim_softc->tmp_ws, 0L, 0L);

		SIM_SOFTC_UNLOCK(s,sim_softc);
		return(&sim_softc->tmp_ws);
	    }		/* NOT during selection */
	    
	} 		/* active_io != NULL */

}

/**
 * simkzq_add_sm_queue -
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simkzq_add_sm_queue will store the SIM_WS, and relevant KZQ interrupt
 * information in the state machine's queue. This structure will be used to 
 * pass the interrupt context from the KZQ ISR to SCKZQSR (state machine) and
 * finally the simkzq_sm() (HBA state machine).
 *
 * FORMAL PARAMETERS:  		
 *
 *	SIM_SOFTC *sim_softc	-- SIM_SOFTC pointer
 *	u_short csr, cstat, 	-- KZQ CSR's 
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
simkzq_add_sm_queue(sim_softc, csr, cstat, dstat, 
		    comm, sc1, sdb, orig_dstat)

REGISTER SIM_SOFTC *sim_softc;
REGISTER u_short 
    csr, cstat, dstat, comm, sc1, sdb, orig_dstat;

 {
    SIMKZQ_SOFTC *hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;
    extern SIM_SM sim_sm;
    SIM_SM_DATA *ssm;
    REGISTER KZQ_INTR *sintr;

    SIM_MODULE(simkzq_add_sm_queue);

    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_SM,
	   ("SIMKZQ_ADD_SM_QUE sim_ws 0x%x, csr 0x%x, cstat 0x%x, dstat 0x%x,",
	    "comm 0x%x\n", sim_ws, csr, cstat, dstat, comm));

    /*
     * Get the needed space to store the interrupt data.
     */
    SIMKZQ_GET_INTR(hba_sc, sintr);

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


    PRINTD(sim_ws->cntlr, sim_ws->targid, sim_ws->lun, CAMD_SM,
	   ("hba_sc = %x, kzqintr = %x, ssm = %x\n",hba_sc, sintr, ssm));

}

/**
 * simkzq_error_recov-
 *
 * FUNCTIONAL DESCRIPTION:
 *	Simkzq_error_recov is used during controller error recovery
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
simkzq_error_recov(sim_softc)
SIM_SOFTC *sim_softc;

{
    SIMKZQ_REG *reg = SIMKZQ_GET_CSR(sim_softc);
    SIMKZQ_SOFTC *hba_sc = (SIMKZQ_SOFTC *)sim_softc->hba_sc;
    SIM_SM_DATA ssm;
    SIM_WS *sim_ws;
    KZQ_INTR *sintr;
    
    SIM_MODULE(simkzq_error_recov);
    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_error_recov) begin\n"));

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
	 * Force the KZQ to assert attension on the bus
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
     * Call the DEC KZQ specific state machine to handle the current
     * phase.
     */
    simkzq_sm(&ssm);

    PRINTD(sim_softc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("(simkzq_error_recov) end\n"));
}


/*
 *
 * Return Value :  None
 */

/**
 * simkzq_unload-
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
simkzq_unload()
{
    SIM_MODULE(simkzq_unload);
    return(1);
}



/**
 * kzqreset -
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
void kzqreset()
{
    SIM_MODULE(kzqreset);
    int controller = 0;

    /*
     * Issue a SCSI bus reset before starting to write a crash dump.
     * WARNING: this only works for systems with a single
     * controller.
     * To make this work for multiple controllers, we will need
     * to pass the controller number from the caller down to this
     * routine.
     */
    (void) simkzq_bus_reset(softc_directory[controller]);

    /*
     * Issue a chip reset to the KZQ chip.
     *
     */
    sim_kzq_chip_reset(softc_directory[controller]);
};




/*
 * Routine Name : sim_kzq_chip_reset
 *
 * Functional Description :
 *	Sim_kzq_chip_reset will reset the DEC SII. 
 *
 * Call Syntax :
 *	sim_kzq_chip_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *
 * Return Value :  None
 */
void
sim_kzq_chip_reset(sc)
SIM_SOFTC *sc;
{
    int s;
    SIMKZQ_REG *kzqreg = (SIMKZQ_REG *)sc->reg;
    SIM_MODULE(sim_kzq_chip_reset);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * This function does NOT perform a SCSI bus reset.
     */

    /*
     * Reset the DEC KZQ SCSI chip.
     */
    SIM_SOFTC_LOCK(s, sc);
    CMD_PENDING(sc->hba_sc,KZQ_COMM_CHIP_RST,kzqreg);

    DELAY(25);
    /*
     * Set the host's SCSI ID.
     */
    kzqreg->id = KZQ_ID_IO | sc->scsiid;

    /*
     * Enable interrupts from the KZQ.
     */
    kzqreg->csr |= KZQ_CSR_IE;
    WBFLUSH();

    sc->sync_offset = SIM_KZQ_SYNC_OFFSET;
    sc->sync_period = SIM_KZQ_SYNC_PERIOD;
  
    SIM_SOFTC_UNLOCK(s, sc);
    DELAY(25);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}
