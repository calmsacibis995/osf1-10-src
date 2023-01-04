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
static char *rcsid = "@(#)$RCSfile: sim_94.c,v $ $Revision: 1.1.12.10 $ (DEC) $Date: 1993/12/09 15:06:38 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_94.c		Version 1.25			Jan. 23, 1992 */

/*
	The SIM_94 source file (sim_94.c) contains functions which
	are specific to the NCR53C94 SCSI host adapter.

Modification History:

	1.25	01/23/92	jag
	Following a DISCONNCET interrupt in ascintr() the FIFO is flushed
	only if there was a selection timeout to remove the unwanted 
	FIFO bytes.  The FIFO is nolonger cleared in sim94_get_ws().

	1.24	01/10/92	janet
	Moved the sim94_get_sim_ws() call from ascintr() to sim94_sm().

	1.23	11/20/91	janet
	Clear header size in sim94_logger().

	1.22	11/20/91	janet
	Changed a bunch of variables to be registers.

	1.21	11/15/91	janet
	Added "scsi_bus_reset_at_boot" hook.

	1.20	11/13/91	janet
	Added sim error logging.

	1.19	10/29/91	jag
	Added the controller argument to the dme.interrupt() handler call.

	1.18	10/28/91	janet
	Use the macro SIM94_PERIOD_CONVERT() when loading the 94's
	period register.

	1.17	10/24/91	janet
	Flush the FIFO before a selection and not on a disconnect.

	1.16	10/22/91	janet
	o Added the following target mode functions: sim94_targ_cmd_cmplt(),
	  sim94_targ_recv_cmd(), sim94_targ_send_msg(),
	  sim94_targ_disconnect(), sim94_targ_recv_msg().
	o Added SIM_MODULE() to every function.
	o Replaced all PRINTD's with SIM_PRINTD's.
	o Replaced all KM_ALLOC's with sc_alloc()'s.
	o Changed all registers to be volatile.
	o In sim94_sm() determine if acting as a target and call
	  sim94_targ_sm().
	o Removed call to sim_enable_interrupts().

	1.15	09/12/91	janet
	Flush the FIFO on bus free.

	1.14	09/09/91	rps
	Remove hard-wiring of chip reset.

	1.13    07/31/91        rps
	Changed to enable interrupts during probe routine

	1.12	07/26/91	janet
	Fixed sim94_function_complete to handle a non-command phase after
	a sel_atn3.

	1.11	07/20/91	rps
	Added spurious interrupt handling call-out to sim_config module.
	Added [b/t/l] to PRINTD's.

	1.10	07/08/91	janet
	ss_resched_request() is called instead of sc_lost_arb().
	In sim94_function_complete() in one case the FIFO needed to be flushed.

        1.09    06/28/91        rps
	Added dynamic config code.  Added check for dma engine interrupt in ISR


	1.08	06/20/91	janet
	Modified camintr() to check the sim_softc pointer instead of "reg."
	Moved check for disconnect from sim94_get_ws() to sim94_sm().  Check
	for and handle selection/reselection race condition where the
	SIM94_INTR_ILLCMD bit is set in	camintr().

	1.07	06/18/91	janet
	Modified to use the "global" state machine queues and macros

	1.06	06/04/91	janet
	Check return value from DME_ATTACH().
	Check return value from DME_PAUSE().

	1.05	05/29/91	janet
	Use sync info stored in IT_NEXUS.  Now including smp_lock.h

	1.04	05/07/91	dallas
	Change to reflect true state of dme engine.

	1.03	04/07/91	jag
	Following a selection time out the FIFO is flushed.

	1.02	03/27/91	jag
	Added the sim94_probe() code.

	1.01	03/26/91	janet
	Updated after code review.

	1.00	11/29/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define CAMERRLOG
#define SIM94_SELECT_TO  (4*hz)	/* Used for the case where the SCSI cable may */
				/* have been pulled from the system box, the */
				/* bus is untermianted so a selection T.O */
				/* interrupt will never occur. */

/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <net/netisr.h>
#include <sys/buf.h>
#include <io/dec/tc/tc.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <dec/binlog/errlog.h>	/* UERF errlog defines */
#include <io/cam/cam_logger.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <io/common/devdriver.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/sim_94.h>
#include <io/cam/cam_errlog.h>

/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern void	bzero();
extern void	sc_setup_ws();
extern void	ss_resched_request();
extern void	sc_sel_timeout();
extern void	scsiisr();
extern U32	dme_attach();
extern void	sim_err_sm();
extern SIM_WS	*sc_find_ws();
extern SIM_SOFTC *softc_directory[];
extern void (*scsi_sm[SCSI_NSTATES][SCSI_NSTATES])();
extern CAM_SIM_ENTRY dec_sim_entry;
extern int 	shutting_down;
extern int 	hz;

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
U32		sim94_probe();
U32		sim94_init();
U32		sim94_go();
U32		sim94_sm();
U32		sim94_bus_reset();
static U32	sim94_sel();
static U32	sim94_sel_atn3();
U32		sim94_sel_stop();
U32		sim94_xfer_info();
U32		sim94_req_msgout();
U32		sim94_clr_atn();
U32		sim94_send_msg();
U32		sim94_msg_accept();
U32		sim94_setup_sync();
U32		sim94_discard_data();
U32		sim94_nop();
void		sim94_load_fifo();
void		ascintr();
static void	sim94_function_complete();
static SIM_WS	*sim94_get_ws();
static void	sim94_add_sm_queue();
static void	sim94_error_recov();
void		sim94_logger();
U32		sim94_targ_cmd_cmplt();
U32		sim94_targ_recv_cmd();
U32		sim94_targ_send_msg();
U32		sim94_targ_disconnect();
U32		sim94_targ_recv_msg();
void 		sim94_shutdown();
static void	sim94_sel_timeout();
static void (*local_errlog)() = sim94_logger; 

int 		sim_poll_mode;		/* Operate in poll mode */

/* ---------------------------------------------------------------------- */
/* Functions and pointers reserved for debugging
 */
SIM94_INTR	*gl_sim94_intr;
SIM94_SOFTC	*gl_sim94_softc;
SIM94_REG	*gl_sim94_reg;

/*
 * Routine Name : sim94_probe
 *
 * Functional Description :
 *	Sim94_probe() will be called at boot time, once for each SCSI
 *	bus based on the configuration informaition. This function is
 *	responsible for allocating the SOFTC structure and placing it
 *      in the softc_dir[].  The passed arguments are stored in the softc
 *	for the other pass of the initialization call via (*sim_init)().
 *
 * Call Syntax:
 *	sim94_probe( csr, um)
 *
 * Arguments:
 *	caddr_t csr;
 *	struct uba_ctlr *um;		BSD
 *	or
 *	struct controller *prb;		OSF
 *
 * Return Value :  CAM_TRUE if the controller exist, CAM_FALSE otherwise
 */
U32
sim94_probe( csr, prb )
caddr_t csr;
struct controller *prb;
{
    extern CAM_SIM_ENTRY dec_sim_entry;
    extern SIM_SOFTC *softc_directory[];
    extern I32 scsi_bus_reset_at_boot;
    extern char cam_ctlr_string[];
    SIM_SOFTC *sc;
    SIM_MODULE(sim94_probe);

    SIM_PRINTD(prb->ctlr_num,NOBTL,NOBTL,(CAMD_INOUT | CAMD_CONFIG),
	   ("Entry - csr=0x%x, um=0x%x\n", csr, prb));

    /* Validate that this controller has not been previously probed.
       If the CSR's match, just continue with the rest of the
       initialization, there may be some PDrv configuration needed.  If
       the CSR's do not match return failure to the system configuation
       code. */

    if( softc_directory[ prb->ctlr_num ] != NULL )
    {
	CAM_ERROR(module,
		  "cntlr already probed",
		  SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE,
		  softc_directory[ prb->ctlr_num ], NULL, NULL);
	if( csr != (softc_directory[ prb->ctlr_num ])->reg )
	{
	    CAM_ERROR(module,
		      "cntlr already probed diff CSR",
		      SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE,
		      softc_directory[ prb->ctlr_num ], NULL, NULL);
	    return( 0 );		/* Signal that it is not there */
	}
    }
    else
    {
	softc_directory[ prb->ctlr_num ] =
	    (SIM_SOFTC *) sc_alloc(sizeof(SIM_SOFTC));
	if ( !softc_directory[ prb->ctlr_num ] )
	{
	    CAM_ERROR(module,
		      "SIM_SOFTC alloc failed",
		      SIM_LOG_PRISEVERE,
		      NULL, NULL, NULL);
	}
    }
    sc = softc_directory[ prb->ctlr_num ];

    SIM_PRINTD(prb->ctlr_num,NOBTL,NOBTL,(CAMD_INOUT | CAMD_CONFIG),
	       ("sim_softc allocated at 0x%x\n", sc));

    sc->hba_sc = (void *) sc_alloc(sizeof(SIM94_SOFTC));

    if ( !sc->hba_sc )
    {
	CAM_ERROR(module,
		  "SIM94_SOFTC alloc failed",
		  SIM_LOG_PRISEVERE,
		  NULL, NULL, NULL);
    }

    /* Update the probe storage fields in the softc structure. */

    SIM_PRINTD(prb->ctlr_num,NOBTL,NOBTL,(CAMD_INOUT | CAMD_CONFIG),
	       ("hba_sim_softc allocated at 0x%x\n", sc->hba_sc));

    /* used to be you could get the base address for the slot out of
     * the controller structure, since the chip registers started here
     * too.  The dual-scsi card breaks this assumption though so we have
     * to root around in the slot address array to get it
     */
    sc->csr_probe = (void *)CAM_PHYS_TO_KVA(tc_slotaddr[(int)prb->tcindx]);

    sc->um_probe = (void *)prb;

    /* Using the uba_ctlr structure and csr update the "normal" fields in the
       softc.  These are "expected" in the rest of the initialization path.  */
    
    sc->reg = (void *)csr;	/* base addr */
    sc->cntlr = prb->ctlr_num;	/* path id */

    /*
     * Register the shutdown routine for the 94 which will disable selections.
     */
    drvr_register_shutdown(sim94_shutdown, (caddr_t)sc, DRVR_REGISTER);

    /*
     * Now prepare the H/W itself to do real stuff
     * Reset the chip to clear any old interrupts
     */
    (SIM94_GET_CSR(sc))->sim94_cmd = SIM94_CMD_RSTCHIP;
    WBFLUSH();

    (SIM94_GET_CSR(sc))->sim94_cmd = SIM94_CMD_NOP;
    WBFLUSH();			/* The C94 spec says we must do a NOP */
    DELAY(25);			/* after a reset */

    /*
     * If "scsi_bus_reset_at_boot" is set to CAM_TRUE, perform a
     * SCSI bus reset.  scsi_bus_reset_at_boot is defined in
     * sim_data.c.
     */

    if (scsi_bus_reset_at_boot == CAM_TRUE) {
	sim94_bus_reset(sc);
    }

    /* Call the CDrv to complete the init path.  If the attachment failed
       signal back to the system that the controller is not ready. */

    if(ccfg_simattach(&dec_sim_entry, prb->ctlr_num) == CAM_FAILURE)
    {
	/*
	 * Turn off poll mode.
	 */
        sim_poll_mode = 0;
	return(0);
    }
    /*
     * Point to the CAM string in private area of controller structure so
     * that it can be identified as a CAM/SCSI controller later by peripheral
     * drivers.
     */
    prb->private[0] = (caddr_t) cam_ctlr_string;

    /*
     * Turn off poll mode.
     */
    sim_poll_mode = 0;
    /*
     * Indicate the 94 can support target mode operations.
     */
    sc->flags |= SZ_TARG_CAPABLE;

    SIM_PRINTD(prb->ctlr_num,NOBTL,NOBTL,(CAMD_INOUT|CAMD_CONFIG),
	       ("Exit\n"));

    return(1);
}

U32
sim94_attach(sc)
SIM_SOFTC *sc;
    {
    extern U32 sim94_init(), sim94_go(), sim94_sm(), sim94_bus_reset(),
                  sim94_send_msg(), sim94_xfer_info(), sim94_sel_stop(),
                  sim94_req_msgout(), sim94_msg_accept(), sim94_setup_sync(),
                  sim94_discard_data(), sim94_nop();
    SIM_MODULE(sim94_attach);

    sc->hba_init = sim94_init;
    sc->hba_go = sim94_go;
    sc->hba_sm = sim94_sm;
    sc->hba_bus_reset = sim94_bus_reset;
    sc->hba_send_msg = sim94_send_msg;
    sc->hba_xfer_info = sim94_xfer_info;
    sc->hba_sel_msgout = sim94_sel_stop;
    sc->hba_msgout_pend = sim94_req_msgout;
    sc->hba_msgout_clear = sim94_clr_atn;
    sc->hba_msg_accept = sim94_msg_accept;
    sc->hba_setup_sync = sim94_setup_sync;
    sc->hba_discard_data = sim94_discard_data;

    sc->hba_targ_cmd_cmplt = sim94_targ_cmd_cmplt;
    sc->hba_targ_recv_cmd = sim94_targ_recv_cmd;
    sc->hba_targ_send_msg = sim94_targ_send_msg;
    sc->hba_targ_disconnect = sim94_targ_disconnect;
    sc->hba_targ_recv_msg = sim94_targ_recv_msg;

    return(CAM_REQ_CMP);
}
 
/*
 * Routine Name : sim94_init
 *
 * Functional Description :
 *	Sim94_init() should be call at boot time, once for each SCSI
 *	bus based on the NCR53C94.  This function is responsible for
 *	initializing the NCR53C94 SIM and associated DME (data mover
 *	engine).
 *
 * Call Syntax:
 *	sim94_init(sc, csr)
 *
 * Arguments:
 *	SIM_SOFTC *sc;
 *	caddr_t csr;
 *
 * Return Value :  CAM_TRUE if the controller exist, CAM_FALSE otherwise
 */
U32
sim94_init( sc )
register SIM_SOFTC *sc;
{
    extern int nNHBA94;
    SIM94_SOFTC *ssc;
    void *csr;
    SIM_MODULE(sim94_init);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("Entry - sc=0x%x\n", sc));

    /*
     * Determine the SCSI id of this HBA.
     */
#ifndef __alpha
    sc->scsiid = get_scsiid(sc->cntlr);
#else
    sc->scsiid = (U16)sim_kn15_get_scsiid(sc);
#endif

    /*
     * Set the path inquiry flags to the peripheral driver can find
     * out our capabilities.
     */
    sc->path_inq_flags = PI_TAG_ABLE; /* we know how to do tags */

    /*
     * Set "hba_sc" pointer in SIM_SOFTC to the appropriate SIM94_SOFTC.
     * Make sure that the SIM94_SOFTC assignment is done only
     * once per controller.
     */
    if (!sc->simh_init) {
	if (sc->cntlr >= nNHBA94) {
	    CAM_ERROR(module,
		      "INVALID controller",
		      SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE,
		      sc, NULL, NULL);
	    return(CAM_FALSE);
	}
	sc->simh_init = CAM_TRUE;
    }

    /*
     * Set-up the machine specific functions.  This will be done via
     * an attach routine.
     */
    ssc = (SIM94_SOFTC *)sc->hba_sc;

    CIRQ_INIT_Q(ssc->intrq);
    CIRQ_SET_DATA_SZ(ssc->intrq, 10);

    /*
     * Perform a chip reset.  This will include setting registers such
     * as configuration 1, 2, and 3; and select/reselect.
     */
    (sc->hba_chip_reset)(sc);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("Exit\n"));

    return(CAM_TRUE);
}

/*
 * Routine Name : sim94_go
 *
 * Functional Description :
 *	Sim94_go() is responsible for setting up the necessary FIFO
 *	information and starting the chip off.  Sim94_go() is only
 *	called when a full SCSI command sequence is needed.  Separate
 * 	entries are provided in the sim_common.c file for bus device
 *	reset, abort, and terminate I/O. If any of the following
 *	actions should fail, this function will return with a CAM
 *	FAILED status.
 *
 * Call Syntax :
 *	sim94_go(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value : CAM status
 */
U32
sim94_go(sws)
register SIM_WS *sws;
{
    U32 status;
    int s;
    SIM_MODULE(sim94_go);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("Entry - sws=0x%x\n", sws));

    SC_ADD_FUNC(sws, module);

    /*
     * SMP lock on SOFTC
     */
    SIM_SOFTC_LOCK(s, (SIM_SOFTC *)sws->sim_sc);

    /*
     * Check the "flags" field of the IT_NEXUS for SZ_SYNC_NEEDED or
     * SZ_SYNC_CLEAR.  If either of these bits are set, a synchronous
     * negotiation is needed.  Start the chip by calling sim94_sel_stop().
     */
    if (sws->it_nexus->flags & (SZ_SYNC_NEEDED | SZ_SYNC_CLEAR)) {
	status = sim94_sel_stop(sws);
    }

    /*
     * Otherwise, if the command is tagged, the SZ_TAGGED flag will be
     * set in the SIM_WS "flags" field.  Start the chip by calling
     * sim94_sel_atn3().
     */
    else if (sws->flags & SZ_TAGGED) {
	status = sim94_sel_atn3(sws);
    }

    /*
     * Otherwise, start the chip by calling sim94_sel().
     */
    else {

	/*
	 * JANET -- check the length of the CDB.  If too big for
	 * the FIFO, perform a sim94_sel_stop().  The plan is then
	 * once the target goes to command phase to DMA the command.
	 * A NOP message will be sent to get the device out of message
	 * out phase.
	 */

	status = sim94_sel(sws);
    }

    /*
     * SMP unlock.
     */
    SIM_SOFTC_UNLOCK(s, (SIM_SOFTC *)sws->sim_sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, status is 0x%x\n", status));

    /*
     * Return with the status from the selection command.
     */
    return(status);
}

/*
 * Routine Name : sim94_sm
 *
 * Functional Description :
 *	Sim94_sm() will first perform any NCR53C94
 *	specific actions necessary to handle the current phase.  It
 *	will then call SC_SM() (state machine) which will perform
 *	phase checking and continuation of the transaction.
 *	Determine the current phase by "anding" the status
 *	register with SIM94_PHASE_MASK.  Use the resulting phase in
 *	a switch.
 *
 * Call Syntax :
 *	sim94_sm(ssm)
 *
 * Arguments :
 *	SIM_SM *ssm	- State machine structure pointer, contains
 *			  a SIM_WS pointer and a pointer to HBA
 *			  specific interrupt information.
 *
 * Return Value :  None
 */
U32
sim94_sm(ssm)
register SIM_SM_DATA *ssm;
{
    U32 sr = SIM94_GET_SR((SIM94_INTR *)ssm->hba_intr);
    U32 ir = SIM94_GET_IR((SIM94_INTR *)ssm->hba_intr);
    U32 ssr = SIM94_GET_SSR((SIM94_INTR *)ssm->hba_intr);
    register SIM_SOFTC *sc = ssm->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    register SIM_WS *sws;
    int s_softc;
    int s_reg;
    SIM_MODULE(sim94_sm);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_SM,
	       ("Entry - ssm=0x%x, ir=0x%x, sr=0x%x\n",
		ssm, ir, sr));
    
    /*
     * Determine the associated SIM_WS.  Locks on the SIM_SOFTC and 94 chip
     * are required for sim94_get_ws() call.
     */
    SIM_SOFTC_LOCK(s_softc, sc);
    SIM_REG_LOCK(s_reg, sc);
    sws = sim94_get_ws(sc, ir, sr, ssr);
    SIM_REG_UNLOCK(s_reg, sc);
    SIM_SOFTC_UNLOCK(s_softc, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Assign the active interrupt pointer in the HBA
     * specific SOFTC structure.
     */
    ssc->active_intr = (SIM94_INTR *)ssm->hba_intr;

    /*
     * Handle the "function complete" condition.
     */
    if (ir & SIM94_INTR_FC)
	sim94_function_complete(sws, ir, sr, ssr);

    /*
     * Was the data mover active?
     */
    if (sws->flags & SZ_DME_ACTIVE) {
	/*
	 * Check the return value from DME_PAUSE().  If not
	 * CAM_REQ_CMP, then an unknown error has occured.
	 */
	if (DME_PAUSE(sc, &sws->data_xfer) != CAM_REQ_CMP) {
	    CAM_ERROR(module,
		      "DME_PAUSE failed",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    sws->error_recovery |= ERR_UNKNOWN;
	}
    }

    /*
     * Did the target disconnect from the bus?
     */
    if (ir & SIM94_INTR_DIS) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_BUS_FREE\n"));

	SC_NEW_PHASE(sws, SCSI_BUS_FREE);
        /*
         * If we are running in target mode, call sm_targ_bus_free(sws) to
	 * indicate we have completed the transfer and gone to bus free.
	 */
        if(sc->flags & SZ_TARGET_MODE) {
                sm_targ_bus_free(sws);
                SIM_SOFTC_LOCK(s_softc, sc);
                sc->flags &= ~SZ_TARGET_MODE;
                SIM_SOFTC_UNLOCK(s_softc, sc);
        } else {
                SC_SM(sws);
	}
 
	return;
    }

    /*
     * Are we acting as a target?  If so, call sim94_targ_sm().
     */
    if (sc->flags & SZ_TARGET_MODE) {
        SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_SM,
                   ("Target mode\n"));
        sim94_targ_sm(ssm, sws);
        return;
    }

    /*
     * Did a reselection occurr?
     */
    if (ir & SIM94_INTR_RESEL) {
	SC_NEW_PHASE(sws, SCSI_RESELECTION);
	SC_SM(sws);
    }


    /*
     * If the last phase was SCSI_STATUS and the last
     * command was "command complete" then read the
     * status byte from the FIFO and handle it.
     */
    if (SC_GET_CURR_PHASE(sws) == SCSI_STATUS) {
	if (SIM94_LAST_CMD(sws) == SIM94_CMD_CMDCMPLT) {
	    if (!( sws->error_recovery & ERR_STATUS_PE )) {
		/* We can only read the status if there isn't an error */
		SIM_REG_LOCK(s_reg, sc);
		SIM94_READ_STATUS(sws, reg);
		SIM_REG_UNLOCK(s_reg, sc);
		SC_SM(sws);
	    }
	}
    }

    /*
     * NCR53C94 anomaly description:  The Transfer Information command is
     * used to transfer message bytes to the target and can be used to
     * send command bytes to the target when using the Select w/ATN and
     * stop sequence command.  The transfer counter must be set for the
     * desired count, if a Transfer Information command is issued with
     * DMA enabled.  Upon receipt of the Transfer Information command, all
     * message or command bytes will be transferred except the last byte.
     * An interrupt will be generated after all but the last byte have
     * been transferred.  If the phase remains the same, then another
     * transfer information command without DMA enabled needs to be
     * issued to send out the last byte.  Once this byte is sent, an
     * interrupt will be generated indicating the completion of the transfer.
     *
     * Note however, this problem was fixed in newer versions of the NCR
     * chips.  So, the code for this is no longer needed, and other
     * error cases can now be handled - see below for more.
     */

    /*
     * Handle the current bus phase.
     */
    switch (sr & SIM94_PHASE_MASK) {

    case SC_PHASE_DATAOUT:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_DATAOUT\n"));

	/*
	 * If data out phase, call SC_NEW_PHASE() with a phase
	 * of SCSI_DATAOUT and then call SC_SM().
	 */
	SC_NEW_PHASE(sws, SCSI_DATAOUT);

	SC_SM(sws);
	break;

    case SC_PHASE_DATAIN:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_DATAIN\n"));
	/*
	 * If data in phase, call SC_NEW_PHASE() with a phase of
	 * SCSI_DATAIN and then call SC_SM().
	 */
	SC_NEW_PHASE(sws, SCSI_DATAIN);

	SC_SM(sws);
	break;

    case SC_PHASE_COMMAND:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_COMMAND\n"));

	/*
	 * Check for anomaly described above.  If the previous
	 * phase was SCSI_COMMAND, issue a SIM94_CMD_XFERINFO
	 * command (non-dma).  The previous phase will still be
	 * stored as the current phase.
	 */
	if (SC_GET_CURR_PHASE(sws) == SCSI_COMMAND) {
	    CAM_ERROR(module,
		      "Chip anomaly, command phase",
		      SIM_LOG_SIM_WS|SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    SIM_REG_LOCK(s_reg, sc);
	    SIM94_SEND_CMD(sws, reg, SIM94_CMD_XFERINFO, sim_poll_mode);
	    WBFLUSH();
	    SIM_REG_UNLOCK(s_reg, sc);
	}

	/*
	 * Normal situation.  Call SC_NEW_PHASE() with a phase of
	 * SCSI_COMMAND and call SC_SM().
	 */
	else {
	    /*
	     * We just entered command phase - the FIFO better be empty.
	     * It might not be empty if we were just in message out and
	     * the target switched to command phase to early.  This
	     * might cause us to send a bad CDB (QAR 11918).
	     */
	    if (( SIM94_GET_FIFO_LN(reg) > 0 ) &&
		    (ssc->phase == SC_PHASE_MSGOUT)) {
		SIM94_FLUSH_FIFO(reg);
	    }
	    SC_NEW_PHASE(sws, SCSI_COMMAND);
	    SC_SM(sws);
	}
	break;

    case SC_PHASE_STATUS:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_STATUS\n"));

	if ( sws->error_recovery & ERR_STATUS_PE ) {
	    /*
	     * Any PE in the chip will propagate into our memory, and
	     * cause a hardware error machine check, so we can't
	     * read any of the bad data
	     */
	    SIM94_FLUSH_FIFO(reg);
	} else {

	    /*
	     * If status phase, start the command SIM94_CMD_CMDCMPLT.
	     * This will handle the status phase and move on to the
	     * following message in phase.
	     */
	    SIM_REG_LOCK(s_reg, sc);
	    SIM94_SEND_CMD(sws, reg, SIM94_CMD_CMDCMPLT, sim_poll_mode);
	    WBFLUSH();
	    SIM_REG_UNLOCK(s_reg, sc);
	    SC_NEW_PHASE(sws, SCSI_STATUS);
	}
	break;

    case SC_PHASE_MSGOUT:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_MSGOUT\n"));

	/*
	 * Check for anomaly described above.  If the previous
	 * phase was SCSI_MDGOUT, issue a SIM94_CMD_XFERINFO
	 * command (non-dma).  The previous phase is still stored
	 * as the current phase.
	 *
	 * For newer chips, the only reason we'd be back here (back
	 * in MSGOUT phase) is because the first try didn't get through
	 * and the target is requesting us to resend the whole thing.
	 */
	if (SC_GET_CURR_PHASE(sws) == SCSI_MSGOUT) {

	    /*
	     * If ATN is still set, then the anomaly occured.  If it
	     * is not, then we transfered the whole thing, but the
	     * target would like us to  transfer it all again.  So,
	     * request a retry of the previous message out stuff.
	     */
#ifdef NCR609-3400469_chip
	    /* However, there is no way to tell these two events apart. */
	    if ( atn_is_set ) {
		CAM_ERROR(module,
		      "Chip anomaly, message out phase",
		      SIM_LOG_SIM_WS|SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
		SIM_REG_LOCK(s_reg, sc);
		SIM94_SEND_CMD(sws, reg, SIM94_CMD_XFERINFO, sim_poll_mode);
		WBFLUSH();
		SIM_REG_UNLOCK(s_reg, sc);
		break;
	    } else
#else
	    /* all chips 609-3400486 and newer/higher have been fixed */
	    {
		/*
		 * Increment the parity error count in the NEXUS. Since
		 * the only real way this can happen is if the target
		 * got a parity error on the message it received.
		 */
		sws->nexus->parity_cnt++;

		/* next we'll resend the message (from sim_sm.c
		 * sm_msgout() - via SC_SM(sws) below).
		 */

	    }
#endif
	}

	/*
	 * Normal situation.  Call SC_NEW_PHASE() with a phase of
	 * SCSI_MSGOUT and call SC_SM().
	 */
	SC_NEW_PHASE(sws, SCSI_MSGOUT);
	SC_SM(sws);
	break;

    case SC_PHASE_MSGIN:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_PHASE,
		   ("phase is SCSI_MSGIN\n"));

        /*
	 * The message byte(s) will be in the FIFO.  This is true
	 * if the last command was a transfer information or
	 * a comand complete.  In the comand complete case, the status
	 * byte will already have been read out of the FIFO and
	 * handled above.  Check the last chip phase.  If SC_PHASE_MSGIN
	 * and the last command was a SIM94_CMD_CMDCMPLT then we have
	 * already transferred the byte.
	 */
	if (((ssc->phase == SC_PHASE_MSGIN) && 
	    (SIM94_LAST_CMD(sws) == SIM94_CMD_XFERINFO)) ||
	    ((ssc->phase == SC_PHASE_STATUS) &&
	     (SIM94_LAST_CMD(sws) == SIM94_CMD_CMDCMPLT))) {
	    if ( sws->error_recovery & ERR_MSGIN_PE ) {

		/* The bytes we have received so far are now invalid.  The 
		 * target will send all new information after we tell it
		 * there was a "Message Parity Error", so we must get rid
		 * of whats there (rev 1.1.8.3).
		 */
		SIM94_FLUSH_FIFO(reg);

	    } else {
		SIM_REG_LOCK(s_reg, sc);
		SIM94_READ_MESSAGE(sws, reg);
		SIM_REG_UNLOCK(s_reg, sc);
	    }
	}

	/*
	 * If we were just reselected, the message has already
	 * been placed in the message in queue.  Otherwise there are
	 * no message bytes in the FIFO.  This will be the case in
	 * all other situations besides reselections.  If the previous
	 * phase was SCSI_DATAOUT or SCSI_DATAIN call the DME_PAUSE()
	 * function.  Execute the command "transfer information"
	 * to allow the NCR53C94 to read the message from the bus.
	 */
	else if (!(ir & SIM94_INTR_RESEL)) {

	    /*
	     * At this point the FIFO should be empty.  If its
	     * not, check to see if the previous phase was
	     * MSGOUT.  If this is the case, the target
	     * is probably rejecting a message byte of a
	     * multi-byte sequence.  
	     */
	    if ((SIM94_GET_FIFO_LN(reg) > 0) && 
		(ssc->phase == SC_PHASE_MSGOUT)) {
		SC_RETRY_MSGOUT(sws);
		sws->msgout_sent = SC_GET_PREV_MSGOUT_LEN(sws) -
		    SIM94_GET_FIFO_LN(reg);
		SIM94_FLUSH_FIFO(reg);
	    }

	    SIM_REG_LOCK(s_reg, sc);
	    SIM94_SEND_CMD(sws, reg, SIM94_CMD_XFERINFO, sim_poll_mode);
	    WBFLUSH();
	    SIM_REG_UNLOCK(s_reg, sc);
	    break;
	}
	   
	/*
	 * There should now be message byte(s) in the message in
	 * queue.  Call SC_NEW_PHASE with a phase of SCSI_MSGIN,
	 * then call SC_SM().
	 */
	SC_NEW_PHASE(sws, SCSI_MSGIN);

	SC_SM(sws);
	break;

    default:
	CAM_ERROR(module,
		  "Invalid phase",
		  SIM_LOG_SIM_WS | SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
    }

    /*
     * Store the last "chip phase."
     */
    ssc->phase = sr & SIM94_PHASE_MASK;

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sim94_shutdown
 *
 * Functional Description :
 *	This function is called prior to halting the system to allow this
 * driver to set the underlying H/W into a benigh state.
 *
 * Call Syntax :
 *	sim94_shutdown(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc;
 *
 * Return Value : None
 */
void
sim94_shutdown(sc)
register SIM_SOFTC *sc;
{
    volatile SIM94_REG *reg;
    int s;

    SIM_MODULE(sim94_shutdown);

    /*
     * Raise IPL and SMP lock.
     */
    SIM_REG_LOCK(s, sc);

    /*
     * Disable selections and reselections (the easy way).
     * We can't use just DISSEL in case a sel/resel has already started.
     */
    (sc->hba_chip_reset)(sc);

    /*
     * Unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);

}	/* sim94_shutdown */

/*
 * Routine Name : sim94_bus_reset
 *
 * Functional Description :
 *	Perform a SCSI bus reset.
 *
 * Call Syntax :
 *	sim94_bus_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc;
 *
 * Return Value : CAM_REQ_CMP
 */
U32
sim94_bus_reset(sc)
register SIM_SOFTC *sc;
{
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    int s;
    SIM_MODULE(sim94_bus_reset);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Raise IPL and SMP lock.
     */
    SIM_REG_LOCK(s, sc);

    /*
     * Assert the SCSI bus reset.
     */
    reg->sim94_cmd = SIM94_CMD_RSTBUS;
    WBFLUSH();

    /*
     * Unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name :  sim94_sel
 *
 * Functional Description :
 *	Sim94_sel() will perform the NCR53C94 Select with ATN
 *	sequence.  One message byte will be sent.  This message byte
 *	must already be in the SIM_WS message out queue.  Following
 *	this message the CDB will be sent.  All data will be sent
 *	via the 94's FIFO.  This function should be called SMP locked
 *	on SOFTC.
 *
 * Call Syntax :
 *	sim94_sel(sws)
 *
 * Arguments :
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, success or fail.
 */
static U32
sim94_sel(sws)
register SIM_WS *sws;
{
    register u_char *cdb;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    int s;
    SIM_MODULE(sim94_sel);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Is SZ_TRYING_SELECT already set?
     */
    if (sc->flags & (SZ_TRYING_SELECT | SZ_RESELECTED)) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, sc->flags is 0x%x\n", sc->flags));
	return(CAM_BUSY);
    }

    /*
     * Did another target already get on the bus?
     */
    if (sc->active_io != (SIM_WS *)NULL) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, active_io not NULL\n"));
	return(CAM_BUSY);
    }

    /*
     * Were there bytes left over in the FIFO?  If so, flush them.
     */
    if (SIM94_GET_FIFO_LN(reg) > 0) {
	SIM94_FLUSH_FIFO(reg);

	/*
	 * If there are still bytes in the FIFO, then we are being
	 * reselected.  Return now.
	 */
	if (SIM94_GET_FIFO_LN(reg) > 0) {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end, reselection is in progress\n"));
	    return(CAM_BUSY);
	}

	/*
	 * If not a reselection log an error.
	 */
	CAM_ERROR(module,
		  "Flushing residual bytes from FIFO",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
    }

    /*
     * Set-up the synchronous offset and period.
     * this calls either sim94_setup_sync or simfast_setup_sync 
     */
    (*sc->hba_setup_sync)(sws);

    /*
     * Set the SIM94_SOFTC "sws" pointer to SIM_WS.
     */
    ssc->sws = sws;

    /*
     * Set the SIM_SOFTC "flags" field with SZ_TRYING_SELECT.
     * Reading and writing a field in SIM_SOFTC.  Must SMP
     * lock on SOFTC.
     */
    sc->flags |= SZ_TRYING_SELECT;
    /*
     * Setup a select/arbitration timeout for the 94.
     * This is needed for ASE in the case that the Y cable has been pulled 
     * from the PMAZC (leaving the bus unterminated) which will not generate 
     * a selection T.O. interrupt.
     */
    timeout(sim94_sel_timeout, sc, SIM94_SELECT_TO);

    /*
     * SMP lock on REG.
     */
    SIM_REG_LOCK(s, sc);

    /*
     * Set the destination target id.
     */
    reg->sim94_dbid = sws->targid;

    /*
     * Call SIM94_LOAD_FIFO_BYTE() once for the message byte.
     */
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
	       ("message out: 0x%x\n", SC_GET_MSGOUT(sws, 0)));
    SIM94_LOAD_FIFO_BYTE(reg, SC_GET_MSGOUT(sws, 0));
    WBFLUSH();
    SC_UPDATE_MSGOUT(sws, 1);

    /*
     * Determine if the CDB is a pointer.
     */
    if (sws->cam_flags & CAM_CDB_POINTER) 
	cdb = (u_char *)sws->ccb->cam_cdb_io.cam_cdb_ptr;
    else
	cdb = (u_char *)sws->ccb->cam_cdb_io.cam_cdb_bytes;

    /*
     * Call sim94_load_fifo() once for the CDB.
     */
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_CMD_EXP,
	       ("cdb_len is %d\n", sws->ccb->cam_cdb_len));
    sim94_load_fifo(reg, (U32)sws->ccb->cam_cdb_len, cdb);

    /*
     * Re-enable selections and reselections.
     */
    reg->sim94_cmd = SIM94_CMD_ENSEL;
    WBFLUSH();

    /*
     * Call SIM94_SEND_CMD() with a command of SIM94_CMD_SELECT.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_SELECT, sim_poll_mode);
    WBFLUSH();

    /*
     * SMP unlock on REG.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name :  sim94_sel_atn3
 *
 * Functional Description :
 *	Sim94_sel_atn3() will perform the NCR53C94 select with ATN3
 *	sequence.  Three message bytes will be sent (identify, tag
 *	opcode message, and tag).  These message bytes must already
 *	be in the SIM_WS message queue.  Next the CDB will be
 *	transferred.  All transfers will be performed via the FIFO.
 *	This function should be called SMP locked on SOFTC.
 *
 * Call Syntax :
 *	sim94_sel_atn3(sws)
 *
 * Arguments :
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, success or fail.
 */
static U32
sim94_sel_atn3(sws)
register SIM_WS *sws;
{
    register int i, s;
    register u_char *cdb;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    SIM_MODULE(sim94_sel_atn3);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Is SZ_TRYING_SELECT already set?
     */
    if (sc->flags & (SZ_TRYING_SELECT | SZ_RESELECTED)) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, sc->flags is 0x%x\n", sc->flags));
	return(CAM_BUSY);
    }

    /*
     * Did another target already get on the bus?
     */
    if (sc->active_io != (SIM_WS *)NULL) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, active_io not NULL\n"));
	return(CAM_BUSY);
    }

    /*
     * Were there bytes left over in the FIFO?  If so, flush them.
     */
    if (SIM94_GET_FIFO_LN(reg) > 0) {
	SIM94_FLUSH_FIFO(reg);

	/*
	 * If there are still bytes in the FIFO, then we are being
	 * reselected.  Return now.
	 */
	if (SIM94_GET_FIFO_LN(reg) > 0) {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end, reselection is in progress\n"));
	    return(CAM_BUSY);
	}

	/*
	 * If not a reselection log an error.
	 */
	CAM_ERROR(module,
		  "Flushing residual bytes from FIFO",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
    }

    /*
     * Set-up the synchronous offset and period.
     * this calls either sim94_setup_sync or simfast_setup_sync 
     */
    (*sc->hba_setup_sync)(sws);

    /*
     * Set the SIM94_SOFTC "sws" pointer to SIM_WS.
     */
    ssc->sws = sws;

    /*
     * Set the SIM_SOFTC "flags" field with SZ_TRYING_SELECT.
     * Reading and writing the SIM_SOFTC flags field, must
     * SMP lock on SOFTC.
     */
    sc->flags |= SZ_TRYING_SELECT;
    /*
     * Setup a select/arbitration timeout for the 94.
     * This is needed for ASE in the case that the Y cable has been pulled 
     * from the PMAZC (leaving the bus unterminated) which will not generate 
     * a selection T.O. interrupt.
     */
    timeout(sim94_sel_timeout, sc, SIM94_SELECT_TO);

    /*
     * SMP lock on REG.
     */
    SIM_REG_LOCK(s, sc);

    /*
     * Set the destination target id.
     */
    reg->sim94_dbid = sws->targid;

    /*
     * Call SIM94_LOAD_FIFO_BYTE() once for each of the three message
     * bytes.  The message bytes will be removed from the NEXUS's
     * message out queue.
     */
    for (i=0; i < 3; i++) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
		   ("message out: 0x%x\n", SC_GET_MSGOUT(sws,i)));
	SIM94_LOAD_FIFO_BYTE(reg, SC_GET_MSGOUT(sws, i));
	WBFLUSH();
    }
    SC_UPDATE_MSGOUT(sws, i);

    /*
     * Determine if the CDB is a pointer.
     */
    if (sws->cam_flags & CAM_CDB_POINTER) 
	cdb = (u_char *)sws->ccb->cam_cdb_io.cam_cdb_ptr;
    else
	cdb = (u_char *)sws->ccb->cam_cdb_io.cam_cdb_bytes;

    /*
     * Call sim94_load_fifo() once for the CDB.
     */
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_CMD_EXP,
	       ("cdb_len is %d\n", sws->ccb->cam_cdb_len));
    sim94_load_fifo(reg, (U32)sws->ccb->cam_cdb_len, cdb);

    /*
     * Re-enable selections and reselections.
     */
    reg->sim94_cmd = SIM94_CMD_ENSEL;
    WBFLUSH();

    /*
     * Call SIM94_SEND_CMD() with a command of SIM94_CMD_SELATN3.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_SELATN3, sim_poll_mode);
    WBFLUSH();

    /*
     * SMP unlock on REG.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name :  sim94_sel_stop
 *
 * Functional Description :
 *	Sim94_sel_stop() will perform the NCR53C94 "Select with ATN
 *	and stop sequence."  One message bye will be sent.  This
 *	message byte must already be on the message out queue.  This
 *	byte will be transferred via the FIFO.  This function should
 *	be called SMP locked on SOFTC.
 *
 * Call Syntax :
 *	sim94_sel_stop(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, CAM_REQ_CMP if no error.
 */
U32
sim94_sel_stop(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    int s;
    SIM_MODULE(sim94_sel_stop);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Is SZ_TRYING_SELECT already set?
     */
    if (sc->flags & (SZ_TRYING_SELECT | SZ_RESELECTED)) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, sc->flags is 0x%x\n", sc->flags));
        SC_ADD_FUNC(sws, "sim94_sel_stop: SZ_TRYING_SELECT or SZ_RESELECTED");
	return(CAM_BUSY);
    }

    /*
     * Did another target already get on the bus?
     */
    if (sc->active_io != (SIM_WS *)NULL) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, active_io not NULL\n"));
        SC_ADD_FUNC(sws, "sim94_sel_stop: active_io not NULL");
	return(CAM_BUSY);
    }

    /*
     * Were there bytes left over in the FIFO?  If so, flush them.
     */
    if (SIM94_GET_FIFO_LN(reg) > 0) {
	SIM94_FLUSH_FIFO(reg);

	/*
	 * If there are still bytes in the FIFO, then we are being
	 * reselected.  Return now.
	 */
	if (SIM94_GET_FIFO_LN(reg) > 0) {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end, reselection is in progress\n"));
	    return(CAM_BUSY);
	}

	/*
	 * If not a reselection log an error.
	 */
	CAM_ERROR(module,
		  "Flushing residual bytes from FIFO",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
    }

    /*
     * Set-up the synchronous offset and period.
     * this calls either sim94_setup_sync or simfast_setup_sync 
     */
    (*sc->hba_setup_sync)(sws);

    /*
     * Set the SIM94_SOFTC "sws" pointer to SIM_WS.
     */
    ssc->sws = sws;

    /*
     * Set the SIM_SOFTC "flags" field with SZ_TRYING_SELECT.
     * Reading and writing SIM_SOFTC flags field, must SMP
     * lock on SOFTC.
     */
    sc->flags |= SZ_TRYING_SELECT;
    /*
     * Setup a select/arbitration timeout for the 94.
     * This is needed for ASE in the case that the Y cable has been pulled 
     * from the PMAZC (leaving the bus unterminated) which will not generate 
     * a selection T.O. interrupt.
     */
    timeout(sim94_sel_timeout, sc, SIM94_SELECT_TO);

    /*
     * SMP lock on REG.
     */
    SIM_REG_LOCK(s, sc);

    /*
     * Set the destination target id.
     */
    reg->sim94_dbid = sws->targid;
    WBFLUSH();

    /*
     * Call SIM94_LOAD_FIFO_BYTE once to load the first message byte
     * from the message out queue.  Update the message out queue.
     */
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
	       ("message out: 0x%x\n", SC_GET_MSGOUT(sws, 0)));
    SIM94_LOAD_FIFO_BYTE(reg, SC_GET_MSGOUT(sws, 0));
    WBFLUSH();

    /*
     * We have more message bytes to send, but they might not be on
     * the message out queue yet.  Increment the message out count
     * by hand.
     */
    sws->msgout_sent++;

    /*
     * Re-enable selections and reselections.
     */
    reg->sim94_cmd = SIM94_CMD_ENSEL;
    WBFLUSH();

    /*
     * Call SIM94_SEND_CMD with the command SIM94_CMD_SELSTOP.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_SELSTOP, sim_poll_mode);
    WBFLUSH();

    /*
     * SMP unlock on REG.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : sim94_xfer_info
 *
 * Functional Description :
 *	Sim94_xfer_info() will perform a SCSI transfer (could be any
 *	phase) bypassing the DME interface.  The data located at
 *	"buf" will be loaded into the FIFO.  The transfer info command
 *	will then be issued.
 *
 *	If the direction of the transfer is "in" no bytes will be
 *	loaded into the FIFO.  This will only be needed after
 *	(and if) the no-DME data transfer method is implemented.
 *
 * Call Syntax :
 *	sim94_xfer_info(sws, buf, cnt, dir)
 *
 * Arguments:
 *	SIM_WS *sws;
 *	u_char *buf;
 *	U32 cnt;
 *	U32 dir;
 *
 * Return Value :  CAM status, CAM_REQ_CMP if no error.
 */
U32
sim94_xfer_info(sws, buf, cnt, dir)
register SIM_WS *sws;
register u_char *buf; /* pointer to local transfer point */
register U32 cnt;  /* number of bytes to transfer */
U32 dir;	  /* direction of the transfer, CAM_DIR_IN or CAM_DIR_OUT */
{
    int s;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM_MODULE(sim94_xfer_info);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * The FIFO's size is limited.  A check will be done on "cnt."
     * If larger than the FIFO, only the maximum number of bytes
     * that the FIFO can hold will be transferred.
     */

    /*
     * Raise IPL and SMP lock.
     */
    SIM_REG_LOCK(s, sc);

    /*
     * If the direction is CAM_DIR_OUT, load the FIFO.
     */
    if (dir == CAM_DIR_OUT)
	sim94_load_fifo(reg, cnt, buf);

    /*
     * Instruct the NCR53C94 to transfer the data.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_XFERINFO, sim_poll_mode);
    WBFLUSH();

    /*
     * Unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : sim94_req_msgout
 *
 * Functional Description :
 *	Sim94_req_msgout() will perform a NCR53C94
 *	"SIM94_CMD_SETATN" command by calling SIM94_SEND_CMD() with
 *	a command of SIM94_CMD_SETATN.
 *
 * Call Syntax :
 *	sim94_req_msgout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, CAM_REQ_CMP if no error.
 */
U32
sim94_req_msgout(sws)
register SIM_WS *sws;
{
    int s;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM_MODULE(sim94_req_msgout);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    /*
     * If the fifo contains anything don't send a SIM94_CMD_SETATN.
     * The 94 will generate an illegal command interrupt.
     */
    if (SIM94_GET_FIFO_LN(reg) > 0)
	return(CAM_REQ_CMP);

    /*
     * Raise IPL and SMP lock.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Pull ATN.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_SETATN, 0);
    WBFLUSH();

    /*
     * SMP unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : sim94_clr_atn
 *
 * Functional Description :
 *	Sim94_clr_atn() will perform a NCR53C94
 *	"SIM94_CMD_CLRATN" command by calling SIM94_SEND_CMD() with
 *	a command of SIM94_CMD_CLRATN.
 *
 * Call Syntax :
 *	sim94_req_msgout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, CAM_REQ_CMP if no error.
 */
U32
sim94_clr_atn(sws)
register SIM_WS *sws;
{
    int s;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM_MODULE(sim94_req_msgout);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Raise IPL and SMP lock.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Clear ATN.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_CLRATN, 0);
    WBFLUSH();

    /*
     * SMP unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : sim94_nop
 *
 * Functional Description :
 *	This function is called when ATN has been asserted and
 *	now, for some reason, we want to release it.  The 94
 *	hasn't provided for this, so this function is called
 *	which does nothing.  Message out phase will be entered
 *	and a NOP message will be generated and set out.  The
 *	94 will then decide to release ATN on its own.
 *
 * Call Syntax :
 *	sim94_nop(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, CAM_REQ_CMP if no error.
 */
U32
sim94_nop(sws)
SIM_WS *sws;
{
    SIM_MODULE(sim94_nop);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  sim94_send_msg
 *
 * Functional Description :
 *	Sim94_send_msg() assumes that the current bus phase is
 *	message out.  All bytes in the message out queue will be
 *	loaded into the FIFO by repeatedly calling SIM94_LOAD_FIFO_BYTE().
 *	A XFER_INFO command will then be issued by calling
 *	SIM94_SEND_CMD() with a command of SIM94_CMD_XFERINFO.
 *
 * Call Syntax :
 *	sim94_send_msg(sws)
 * 
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :   CAM SUCCESS or FAIL (if no message bytes to
 */
U32
sim94_send_msg(sws)
register SIM_WS *sws;
{
    int s;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register U32 i, count = SC_GET_MSGOUT_LEN(sws);
    SIM_MODULE(sim94_send_msg);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin, sending %d bytes\n", count));

    /*
     * Raise IPL and SMP Lock.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Make sure that all the bytes will fit in the FIFO.  If not, fail.
     * It is assumed that all the message bytes should be sent in the
     * current message out phase.  
     */
    if (count > (SIM94_FIFO_SIZE - SIM94_GET_FIFO_LN(reg))) {
	CAM_ERROR(module,
		  "FIFO wont hold all message bytes",
		  SIM_LOG_SIM_WS | SIM_LOG_HBA_INTR | SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
	SIM_REG_UNLOCK(s, sc);
	return(CAM_FAILURE);
    }

    /*
     * Load the message bytes in the FIFO, one at a time.
     */
    for (i=0; i < count; i++) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
	       ("message out: 0x%x\n", SC_GET_MSGOUT(sws, i)));
	SIM94_LOAD_FIFO_BYTE(reg, SC_GET_MSGOUT(sws, i));
	WBFLUSH();
    }
    SC_UPDATE_MSGOUT(sws, count);

    SIM94_SEND_CMD(sws, reg, SIM94_CMD_XFERINFO, sim_poll_mode);
    WBFLUSH();

    /*
     * Unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);


    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : sim94_msg_accept
 *
 * Functional Description :
 *	Sim94_msg_accept() will issue a NCR53C94 message accepted
 *	command by calling SIM94_SEND_CMD() with a command of
 *	SIM94_CMD_MSGACPT.
 *
 * Call Syntax :
 *	sim94_msg_accept(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM status, CAM_REQ_CMP if no error.
 */
U32
sim94_msg_accept(sws)
register SIM_WS *sws;
{
    int s;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM_MODULE(sim94_msg_accept);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("(sim94_msg_accept) begin\n"));

    /*
     * Raise IPL and SMP lock.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Accept the message.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_MSGACPT, sim_poll_mode);
    WBFLUSH();

    /*
     * Unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : sim94_setup_sync
 *
 * Functional Description :
 *	Sim94_setup_sync will set the sync offset and period
 *	of the NCR53C94 to the values stored in the IT_NEXUS
 *	"sync_period" and "sync_offset" fields.
 *
 * Call Syntax :
 *	sim94_setup_sync(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM_TRUE
 */
U32
sim94_setup_sync(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    int s;
    SIM_MODULE(sim94_setup_sync);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin, period 0x%x, offset 0x%x\n",
		sws->it_nexus->sync_period, sws->it_nexus->sync_offset ));
    
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Set the NCR53C94's synchronous offset and period.
     * Use "sync_offset" and "sync_period" from the IT_NEXUS.
     *
     * The period in the IT_NEXUS must be multiplied by four
     * to determine the value in nanoseconds.  Convert this value
     * to clocks per byte by dividing by the clock speed of the chip.
     * Convert the clock speed from MHz to nanoseconds by dividing
     * 10000 by the clock speed (the clock speed was stored as
     * MHz * 10).  Finally call SIM94_PERIOD_CONVERT() to come up with
     * the correct value to load into the chip.
     */
    reg->sim94_sp = SIM94_PERIOD_CONVERT(
	((U32)sws->it_nexus->sync_period * 4L) / (10000L / (U32)ssc->clock),
        ((U32)sws->it_nexus->sync_period * 4L));

    WBFLUSH();
    reg->sim94_so = sws->it_nexus->sync_offset;
    WBFLUSH();

    SIM_REG_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_TRUE);
}

/*
 * Routine Name : sim94_discard_data
 *
 * Functional Description :
 *	Sim94_discard_data is used during error recovery to read
 *	data from the SCSI bus and throw it away, one byte at a
 *	time.  This function should be called with IPL high and
 *	SIM_SOFTC SMP locked.
 *
 * Call Syntax :
 *	sim94_discard_data(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  CAM_TRUE
 */
U32
sim94_discard_data(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    int s;
    SIM_MODULE(sim94_discard_data);

    /*
     * Transfer one byte.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    reg->sim94_tcmsb = 0;
    WBFLUSH();
    reg->sim94_tclsb = 1;
    WBFLUSH();

    SIM94_SEND_CMD(sws, reg, SIM94_CMD_DMA | SIM94_CMD_XFERPAD, sim_poll_mode);
    WBFLUSH();
    SIM_REG_UNLOCK(s, sc);

    return(CAM_TRUE);
}

/*
 * Routine Name : sim94_load_fifo
 *
 * Functional Description :
 *	Sim94_load_fifo will load the given data bytes onto the
 *	FIFO.  All bytes should fit in the FIFO, otherwise this
 *	function shouldn't be called.  This function should be
 *	called REG SMP locked.
 *
 * Call Syntax :
 *	sim94_load_fifo(reg, cnt, buf)
 *
 * Arguments:
 *	SIM94_REG *reg		-- Pointer to chip's registers
 *	U32 cnt		-- Number of bytes to load
 *	u_char *buf		-- Pointer to bytes to load
 *
 * Return Value :  None
 */
void
sim94_load_fifo(reg, cnt, buf)
volatile SIM94_REG *reg;
register U32 cnt;
register u_char *buf;
{
    register U32 i;
    SIM_MODULE(sim94_load_fifo);

    for (i=0; i < cnt; i++) {
    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	   ("load byte 0x%x\n", buf[i]));
    SIM94_LOAD_FIFO_BYTE(reg, buf[i]);
    WBFLUSH();
    }
}

/*
 * Routine Name : sim94_flush_fifo
 *
 * Functional Description :
 *	Sim94_flush_fifo will empty the '94 fifo iff the fifo data count
 *      is non-zero.
 *
 * Call Syntax :
 *	sim94_flush_fifo( reg )
 *
 * Arguments:
 *	SIM94_REG *reg		-- Pointer to chip's registers
 *
 * Return Value :  None
 */
static void
sim94_flush_fifo(sc)
SIM_SOFTC *sc;
{
    volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM_MODULE(sim94_flush_fifo);

    /*
     * Check the count in the FIFO.  If greater than zero,
     * flush the FIFO.
     */
    if ( SIM94_GET_FIFO_LN( reg )) {
	SIM94_FLUSH_FIFO(reg);
    }
}
 
/*
 * Routine Name :  sim94_intr
 * 		:  ascintr
 *
 * Functional Description :
 *	Sim94_intr will handle all interrupts generated by the
 *	NCR53C94.
 *
 * Call Syntax :
 *	ascint(controller)
 *
 * Arguments:
 *	int controller;
 *
 * Return Value :  None
 */
void
ascintr(controller)
int controller;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)SC_GET_SOFTC(controller);
    register volatile SIM94_REG *reg;
    register U32 ssr, sr, ir;
    extern SIM_SM sim_sm;
    extern int netisr;
    int s1, s2, s3;
    SIM_MODULE(ascintr);

    SIM_PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * If "sc" is NULL then this is a spurious interrupt at boot time.
     * Spurious int's shouldn't be able to happen anymore (fk 3/93).
     */
    if (sc == (SIM_SOFTC *)NULL )
      {
	CAM_ERROR(module,
		  "Spurious interrupt - no sc",
		  SIM_LOG_PRISEVERE,
		  NULL, NULL, NULL);
        return;
      }

    /*
     * The softc is valid, get the register pointer from it.
     */
    reg = SIM94_GET_CSR(sc);

    /*
     * SMP lock on this controller for both the SOFTC and REG.
     */
    SIM_SOFTC_LOCK(s1, sc);
    SIM_REG_LOCK(s2, sc);

    /*
     * Read and save the following 94 registers:  sequence step
     * register, status register, interrupt status.  Read the
     * registers in this order.  The final read on the interrupt
     * status will clear all three registers.
     */
    ssr = reg->sim94_ss;
    sr = reg->sim94_stat;
    ir = reg->sim94_ir;

    SIM_PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
	       ("ssr 0x%x, sr 0x%x, ir 0x%x\n", ssr, sr, ir));

    /*
     * Make sure this is a valid interrupt.  The "interrupt"
     * bit of the status register should always be set.
     */
    if (!(sr & SIM94_STAT_INT)) {
	CAM_ERROR(module,
		  "Spurious interrupt - no INT",
		  SIM_LOG_PRISEVERE,
		  NULL, NULL, NULL);
	SIM_REG_UNLOCK(s2, sc);
	SIM_SOFTC_UNLOCK(s1, sc);
	return;
    }

    /*
     * Check for and handle the following errors (the status
     * register has corresponding bits):  GE (gross error).
     * If this happens, something very wrong has occurred.
     * Print an error message, clear the FIFO, perform a chip
     * reset, and finally perform a SCSI bus reset if either of
     * these bits are set.  
     */
    if (sr & SIM94_STAT_GE) {
	sim94_add_sm_queue(sc, ir, sr, ssr);
	CAM_ERROR(module, "Gross error", SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		  sc, NULL, NULL);
	SIM94_FLUSH_FIFO(reg);
	SIM_REG_UNLOCK(s2, sc);
	SIM_SOFTC_UNLOCK(s1, sc);
	(sc->hba_chip_reset)(sc);
	SC_HBA_BUS_RESET(sc);
	return;
    }

    /*
     * Check to determine if a SCSI bus reset was detected.
     */
    if (ir & SIM94_INTR_RST) {

	/*
	 * Check the count in the FIFO.  If greater than zero,
	 * flush the FIFO.
	 */
	if (SIM94_GET_FIFO_LN(reg)) {
	    SIM94_FLUSH_FIFO(reg);
	}

	/*
	 * Are we recovering from a previous bus reset?  If so,
	 * do nothing more for this one.
	 */
	if (!(sc->error_recovery & ERR_BUS_RESET)) {

	    /*
	     * Note that the interrupt line first asserts itself 320nsec
	     * after the reset starts.  The interrupt line then stays
	     * asserted for the full 25usec that the reset is on the
	     * bus.  Therefore, we may get multiple interrupts from
	     * the same reset (because we can service it so quickly,
	     * we can get interrupted again before the reset completes).
	     * This was noticed in QAR 05286.
	     */
	    CAM_ERROR(module,
		  "Bus reset detected",
		  SIM_LOG_PRISEVERE,
		  sc, NULL, NULL);
	    /*
	     * Increment the bus reset count in the SIM_SOFTC.
	     */
	    sc->bus_reset_count++;

	    /*
	     * Set the SIM_SOFTC ERR_BUS_RESET bit in the SIM_SOFTC
	     * "error_recovery" field.  Make sure that the state machine
	     * scheduler is running.
	     */
	    sc->error_recovery |= ERR_BUS_RESET;

	    /*
	     * Lock on the State Machine.
	     */
	    SIM_SM_LOCK(s3, &sim_sm);

	    /*
	     * Set the bus_reset bit in the state machine's struct.
	     */
	    sim_sm.bus_reset |= (1 << controller);
	    if (!sim_sm.sm_active || shutting_down ) {

                /*
                 * After a SCSI bus reset is detected we must Re-enable
                 * selections/reselections.
                 */
                reg->sim94_cmd = SIM94_CMD_ENSEL;
                WBFLUSH();

		/*
		 * Call the state machine to handle this interrupt.
		 */
		SIM_SCHED_ISR();
	    }

	    /*
	     * Unlock the State Machine.
	     */
	    SIM_SM_UNLOCK(s3, &sim_sm);
	}
        /*
         * After a SCSI bus reset is detected we must Re-enable selections
         * /reselections. Sometimes we will do this twice, but that's ok.
         */
        reg->sim94_cmd = SIM94_CMD_ENSEL;
        WBFLUSH();
	
	SIM_REG_UNLOCK(s2, sc);
	SIM_SOFTC_UNLOCK(s1, sc);

	/*
	 * Return now.
	 */
	SIM_PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
	       ("end, bus reset detected\n"));
	return;
    }

    /*
     * Check for and handle an illegal command.
     */
    if (ir & SIM94_INTR_ILLCMD) {
	SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
	U32 cmd = reg->sim94_cmd;

	/*
	 * Flush the FIFO.
	 */
	SIM94_FLUSH_FIFO(reg);

	/*
	 * Were we trying to select?  If so, we may have hit
	 * a race condition where we were reselected just before
	 * we issued the select command.  If this was the case,
	 * reschedule the select.
	 */
	if (sc->flags & SZ_TRYING_SELECT) {

	    /*
	     * Was the illegal command a select command?
	     */
	    if ((cmd == SIM94_CMD_SELECT) || (cmd == SIM94_CMD_SELSTOP) ||
		(cmd == SIM94_CMD_SELATN3)) {

		sc->flags &= ~SZ_TRYING_SELECT;
		/*
		 * Remove the selection/arbitration timeout.
		 */
		untimeout(sim94_sel_timeout, sc);

		/*
		 * Call ss_resched_request() to take care of the
		 * losing request.
		 */
		ss_resched_request(ssc->sws);

		/*
		 * Make sure that the state machine will retry the
		 * request.
		 */
		SIM_SM_LOCK(s3, &sim_sm);
		sim_sm.waiting_io |= (1 << sc->cntlr);
		if (!sim_sm.sm_active || shutting_down ) {

		    SIM_SCHED_ISR();
		}
		SIM_SM_UNLOCK(s3, &sim_sm);

	    }
	}
	else {
	    CAM_ERROR(module,
		      "Illegal command",
		      SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		      sc, NULL, NULL);
	}

    }

    /*
     * If the target disconnects, re-enable selections and reselections.
     */
    if (ir & SIM94_INTR_DIS) {

	/*
	 * Rev. 1.25
	 * Check for a selection timeout.
	 * JAG: Is this checking for too little a window ?
	 */
	if ((sc->flags & SZ_TRYING_SELECT) &&
	    ((ssr & SIM94_SSTEP_MASK) == SIM94_SSTEP_TO)) {

	    /*
	     * Clear out of the FIFO the bytes loaded in for the
	     * selection.  They are nolonger needed and the '94
	     * still thinks that they are "valid".  The SIM94_SEND_CMD
	     * macro is not used, there is no need to store the
	     * flush command.
	     */
	    SIM94_FLUSH_FIFO(reg);
	}

	/*
	 * Reenable selections and reselections to allow another target to
	 * get to the system.
	 */
	reg->sim94_cmd = SIM94_CMD_ENSEL;
	WBFLUSH();
    }

    /*
     * If the interrupt is due to a phase change (this includes
     * select and reselect) or a command complete, determine the
     * associated working set and schedule the state machine to
     * handle the current phase.
     */
    if (ir &
	(SIM94_INTR_DIS | SIM94_INTR_BS | SIM94_INTR_FC |
	 SIM94_INTR_RESEL | SIM94_INTR_SELATN | SIM94_INTR_SEL)) {

	/*
	 * If reselected, or in the SZ_RESELECTED flag.
	 */
	if (ir & SIM94_INTR_RESEL)
	    sc->flags |= SZ_RESELECTED;

	/*
	 * Are we recovering from an error condition, such as a bus
	 * reset?
	 */
	if (sc->error_recovery) {

	    SIM_REG_UNLOCK(s2, sc);
	    SIM_SOFTC_UNLOCK(s1, sc);
	    sim94_error_recov(sc, ir, sr, ssr);
	    CAM_ERROR(module,
		      "Error recovery in progress",
		      SIM_LOG_PRISEVERE,
		      NULL, NULL, NULL);
	    SIM_PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
		       ("end, error_recovery\n"));
	    return;
	}

	/*
	 * Put the interrupt on the state machine's processing queue.
	 * If the state machine isn't already running, schedule it.
	 */
	SIM_SM_LOCK(s3, &sim_sm);
	sim94_add_sm_queue(sc, ir, sr, ssr);
	if (!sim_sm.sm_active || shutting_down ) {

	    /*
	     * Schedule the CAM state machine via the softnet
	     * interrupt.
	     */
	    SIM_SCHED_ISR();

	}
	SIM_SM_UNLOCK(s3, &sim_sm);
    }

    SIM_REG_UNLOCK(s2, sc);
    SIM_SOFTC_UNLOCK(s1, sc);

    SIM_PRINTD(controller, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  sim94_function_complete
 *
 * Functional Description :
 *	Sim94_function_complete is called by the interrupt handler
 *	at high IPL to handle chip specific operations which are
 *	needed the the NCR53C94 completes a function.  Called with
 *	both SOFTC and REG SMP locked.
 *
 * Call Syntax :
 *	sim94_function_complete(sws, sr, ssr)
 *
 * Arguments:
 *	SIM_WS *sws		-- SIM_WS pointer
 *	U32 sr			-- status register
 *	U32 ssr			-- sequence step register
 *
 * Return Value :  None
 */
static void
sim94_function_complete(sws, ir, sr, ssr)
register SIM_WS *sws;
register U32 ir, sr, ssr;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register U_WORD last_cmd = SIM94_LAST_CMD(sws);
    SIM_MODULE(sim94_function_complete);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	   ("begin, command was 0x%x\n", last_cmd));

    SC_ADD_FUNC(sws, module);

    /*
     * The last command could have been a DMA or non-DMA command.
     * It will be handled the same in both cases.  Strip the DMA
     * bit from the command to simplify the following checks.
     */
    last_cmd &= ~SIM94_CMD_DMA;

    /*
     * When issuing any of the Select commands, the command descriptor block
     * is loaded into the FIFO.  When the internal state machine checks
     * the phase before sending out the last command byte at the bottom
     * of the FIFO, it is masked out and the FIFO appears empty.  An extra
     * pad byte is automatically loaded into the FIFO by the 53C94.  The
     * last command byte is then sent to the target and the extra pad
     * byte is not sent to the target.  Once an interrupt is received
     * (Function Complete and Bus Service bits set), this extra pad byte
     * must be removed.  This can be accomplished by either reading the
     * byte out of the FIFO or by issuing a Flush FIFO command.  The
     * following commands require the removal of a pad byte once
     * completed:  Select w/ATN, Select w/o ATN, and Select w/ATN3.
     */
    if ((last_cmd == SIM94_CMD_SELECT) ||
	(last_cmd == SIM94_CMD_SELWOATN) ||
	(last_cmd == SIM94_CMD_SELATN3)) {
	/*
	 * Flush the FIFO if not synchronous data in.
	 */
	if (!(((sr & SIM94_PHASE_MASK) == SC_PHASE_DATAIN) &&
	      (sws->it_nexus->sync_offset > 0))) {
	    SIM94_FLUSH_FIFO(reg);
	}
    }

    /*
     * Handle function complete on the following commands:
     * initiator select with ATN, initiator select with ATN3,
     * and initiator select with ATN and stop.
     */
    if ((last_cmd == SIM94_CMD_SELECT) ||
	(last_cmd == SIM94_CMD_SELATN3) ||
	(last_cmd == SIM94_CMD_SELSTOP)) {

	switch (ssr & SIM94_SSTEP_MASK) {

	case SIM94_SSTEP_TO:
	    /*
	     * Arbitration and selection complete.  Stopped because
	     * target did not assert message out phase.  ATN still driven
	     * by NCR53C94.
	     */
	    /*
	     * Completed arbitration and selection.
	     */
	    SC_NEW_PHASE(sws, SCSI_ARBITRATION);
	    SC_NEW_PHASE(sws, SCSI_SELECTION);

	    /*
	     * We had put one message out byte in the FIFO.
	     * This byte must now be flushed.
	     * 1.10
	     */
	    SIM94_FLUSH_FIFO(reg);
	    SC_RETRY_MSGOUT(sws);

	    break;

	case SIM94_SSTEP_NOCMD:
	case SIM94_SSTEP_MSGO:
	    /*
	     * Arbitration and selection complete.  Stopped because
	     * target did not assert command phase.
	     *
	     * or
	     *
	     * Message out complete.  Sent one message byte; ATN on.
	     */
	    /*
	     * Completed arbitration and selection.
	     */
	    SC_NEW_PHASE(sws, SCSI_ARBITRATION);
	    SC_NEW_PHASE(sws, SCSI_SELECTION);

	    /*
	     * Determine if a message out phase has occured.
	     */
	    if ((last_cmd == SIM94_CMD_SELATN3) &&
		(SIM94_GET_FIFO_LN(reg) != 3)) {
		SC_NEW_PHASE(sws, SCSI_MSGOUT);
	    }
	    else if ((last_cmd == SIM94_CMD_SELECT) &&
		     (SIM94_GET_FIFO_LN(reg) != 1)) {
		SC_NEW_PHASE(sws, SCSI_MSGOUT);
	    }

	    /*
	     * Are there left over bytes in the FIFO that need to
	     * be flushed?
	     */
	    if (SIM94_GET_FIFO_LN(reg)) {
		SIM94_FLUSH_FIFO(reg);
		SC_RETRY_MSGOUT(sws);
	    }

	    break;

	case SIM94_SSTEP_CMDFAIL:
	    /*
	     * Stopped during command transfer due to premature phase change.
	     * Some CDB bytes may not have been sent; check FIFO flags.
	     */
	    /*
	     * Completed arbitration, selection, and message out.
	     */
	    SC_NEW_PHASE(sws, SCSI_ARBITRATION);
	    SC_NEW_PHASE(sws, SCSI_SELECTION);
	    SC_NEW_PHASE(sws, SCSI_MSGOUT);
	    /*
	     * Need to perform a recovery on the partial command
	     * which may have been sent.  JANET
	     */
	    break;

	case SIM94_SSTEP_CMPLT:
	    /*
	     * Sequence complete.
	     */
	    /*
	     * Completed arbitration, selection, message out,
	     * and command phase.
	     */
	    SC_NEW_PHASE(sws, SCSI_ARBITRATION);
	    SC_NEW_PHASE(sws, SCSI_SELECTION);
	    SC_NEW_PHASE(sws, SCSI_MSGOUT);
	    SC_NEW_PHASE(sws, SCSI_COMMAND);
	    break;

	default:
	    sim94_add_sm_queue(sc, ir, sr, ssr);
	    CAM_ERROR(module,
		      "Invalid sequence step register",
		      SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		      sc, sws, NULL);
	}
    }

    /*
     * Do any other commands need to be handled?  -- JANET
     */

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  sim94_get_ws
 *
 * Functional Description :
 *	Sim94_get_ws will determine the SIM_WS which is related
 *	to the current interrupt.  If it is not possible to make
 *	this determination, the SIM_SOFTC's "tmp_ws" will be used.
 *	This function is called SMP locked on both SOFTC and REG.
 *
 * Call Syntax :
 *	sim94_get_ws(sc, ir, sr, ssr)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *	U32 ir			-- interrupt register
 *	U32 sr			-- status register
 *	U32 ssr			-- sequence step register
 *
 * Return Value :  Pointer the the SIM_WS
 */
static SIM_WS *
sim94_get_ws(sc, ir, sr, ssr)
register SIM_SOFTC *sc;
register U32 ir, sr, ssr;
{
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    register SIM_WS *sws;
    register u_char msg, fifo_ln = SIM94_GET_FIFO_LN(reg);
    register U32 targid, lun;
    int s;
    SIM_MODULE(sim94_get_ws);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("Entry - sc=0x%x, ir=0x%x, sr=0x%x, ssr=0x%x\n",
		sc, ir, sr, ssr));

    /*
     * Did we perform (or attempt to perform) a selection?
     */
    if (sc->flags & SZ_TRYING_SELECT) {

	/*
	 * Clear the SZ_TRYING_SELECT flag.  The SIM_WS pointer has
	 * been stored in the SIM94_SOFTC structure.
	 */
	sc->flags &= ~SZ_TRYING_SELECT;
	/*
	 * Remove the 94 selection/arbitration timeout.
	 */
	untimeout(sim94_sel_timeout, sc);

	sws = ssc->sws;
	SC_ADD_FUNC(sws, "sim94_get_ws: SZ_TRYING_SELECT");

	/*
	 * Were we reselected during a selection attempt or did a
	 * reselection occurr before our selection attempt completed?
	 */
	if ((ir & SIM94_INTR_RESEL) || (sc->active_io != (SIM_WS *)NULL)) {

	    SIM_PRINTD( sc->cntlr, NOBTL, NOBTL, CAMD_FLOW,
		       ("lost selection due to reselection\n"));
	    /*
	     * Call ss_resched_request() to take care of the losing request.
	     */
	    ss_resched_request(sws);

	    /*
	     * Determine the SIM_WS of the reselecting device.
	     */
	    sws = sim94_get_ws(sc, ir, sr, ssr);
            SC_ADD_FUNC(sws, "sim94_get_ws: lost select due to resl");

	    /*
	     * All done.  Return now.
	     */
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end\n"));

	    return(sws);
	}

        /*
         * Check whether we have been selected as a target while trying to
         * select a target.
         */
        if ((ir & SIM94_INTR_SELATN) || (ir & SIM94_INTR_SEL)) {
            /*
             * Call ss_resched_request() to take care of the losing request.
             */
            ss_resched_request(sws);

            /*             
	     * Determine the selecting target ID.  The first byte
             * of the FIFO will contain the target ID along with the
             * host ID.  Mask out the ID and convert the bit-wise
             * coded target ID.
             */
            targid = SIM94_READ_FIFO(reg) & 0xff;
            targid &= ~(1 << sc->scsiid);
            targid = ffs((int)targid) - 1;
                                                                                            sws = &sc->tmp_ws;
            sc_setup_ws(sc, sws, targid, 0L);
            sws->flags |= SZ_TARGET_MODE;

            SIM_SOFTC_LOCK(s, sc);
            sc->flags |= SZ_TARGET_MODE;
            sc->active_io = sws;
            SIM_SOFTC_UNLOCK(s, sc);

            /*
             * Return the temporary SIM_WS.
             */
            return(sws);
	}
	/*
	 * Check for function complete.
	 */
	if (!(ir & SIM94_INTR_FC)) {

	    /*
	     * Check for a selection timeout.
	     */
	    if ((ir & SIM94_INTR_DIS) &&
		((ssr & SIM94_SSTEP_MASK) == SIM94_SSTEP_TO)) {
		
		SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_FLOW,
			   ("selection timeout\n"));

		sc_sel_timeout(sws);
	    }
	    else {
		/*
		 * One way this can happen is if the target gets a parity
		 * error trying to receive the command packet and it
		 * disconnects from the bus - then causing an unexpected
		 * free condition.  In general, anytime a selection
		 * sequence we've started doesn't get all the way through
		 * (and out of) command phase we'll get here.  This is
		 * because all the message/command processing is done by
		 * the chip itself (without any intervention from us).
		 */
		CAM_ERROR(module,
			  "SZ_TRYING_SELECT set and no FC.",
			  SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
			  sc, sws, NULL);
	    }
	}
    }

    /*
     * Was there a reselection?  If so, attempt to determine the
     * working set using the target and lun.  If this is not
     * enough information, use the SIM_SOFTC temp working set.
     */
    else if (ir & SIM94_INTR_RESEL) {

	/*
	 * Clear the SZ_RESELECTED flag.
	 */
	sc->flags &= ~SZ_RESELECTED;

	/*
	 * There should be two bytes in the FIFO.  If only one,
	 * the device may have reselected without an identify
	 * message.  This case will be handled (JANET -- add code).
	 */
	if (fifo_ln < 2) {
	    CAM_ERROR(module,
		      "Reselected, but FIFO contained less than two bytes",
		      SIM_LOG_HBA_SOFTC | SIM_LOG_PRISEVERE,
		      sc, NULL, NULL);
	}

	/*
	 * If there are more than two bytes in the FIFO, there may
	 * have been an error condition which left the residual.
	 */
	if (fifo_ln > 2) {
	    CAM_ERROR(module,
		      "Reselected, flushing residual bytes from FIFO",
		      SIM_LOG_PRISEVERE,
		      NULL, NULL, NULL);
	    while((fifo_ln = SIM94_GET_FIFO_LN(reg)) > 2)
		msg = SIM94_READ_FIFO(reg);
	}

	/*
	 * Determine the reselecting target ID.  The first byte
	 * of the FIFO will contain the target ID along with the
	 * host ID.  Mask out the host ID and convert the bit-wise
	 * coded target ID.
	 * JANET -- Need to look into parity error on reselection.
	 * 	They must be handled BEFORE we read the fifo!
	 */
	targid = SIM94_READ_FIFO(reg) & 0xff;
	targid &= ~(1 << sc->scsiid);
	targid = ffs((int)targid) - 1;

	msg = SIM94_READ_FIFO(reg);
	lun = SC_DECODE_IDENTIFY(msg);

	/*
	 * Make sure that the targid and lun are within
	 * the legal limits.
	 */
	if ((targid >= NDPS) || (lun >= NLPT)) {
	    CAM_ERROR(module,
		      "Invalid target/LUN",
		      SIM_LOG_HBA_SOFTC | SIM_LOG_PRISEVERE,
		      sc, NULL, NULL);
	    panic("(sim94_get_ws) invalid T/L!");
	}

	/*
	 * Did a parity error occurr on the identify message?  If
	 * so, we can't use the "lun" to determine the SIM_WS.  In
	 * this case use the temp working set.
	 */
	if (sr & SIM94_STAT_PE) {
	    CAM_ERROR(module,
		      "Parity error on reselection",
		      SIM_LOG_PRISEVERE,
		      NULL, NULL, NULL);
	    sws = &sc->tmp_ws;
            SC_ADD_FUNC(sws, "sim94_get_ws: parity_error_on_resel");
	}
	else {
	    /*
	     * Call sc_find_ws() to determine our working set.
	     */
	    sws = sc_find_ws(sc, targid, lun, -1L);
            SC_ADD_FUNC(sws, "sim94_get_ws: sc_find_ws()");
	}

	/*
	 * If we have to use the temporary working set, initialize it.
	 */
	if (sws == &sc->tmp_ws) {
	    sc_setup_ws(sc, sws, targid, lun);
	}

	/*
	 * Add the identify message to the message in queue.
	 */
	SC_ADD_MSGIN(sws, msg);
    }

    /*
     * Were we selected?
     */
    else if ((ir & SIM94_INTR_SELATN) || (ir & SIM94_INTR_SEL)) {

	/*
	 * Determine the selecting target ID.  The first byte
	 * of the FIFO will contain the target ID along with the
	 * host ID.  Mask our the ID and convert the bit-wise
	 * coded target ID.
	 */
	targid = SIM94_READ_FIFO(reg) & 0xff;
	targid &= ~(1 << sc->scsiid);
	targid = ffs((int)targid) - 1;

	sws = &sc->tmp_ws;
	sc_setup_ws(sc, sws, targid, 0L);
	sws->flags |= SZ_TARGET_MODE;

        SIM_SOFTC_LOCK(s, sc);
        sc->flags |= SZ_TARGET_MODE;
        sc->active_io = sws;
        SIM_SOFTC_UNLOCK(s, sc);
    }

    /*
     * No selection or reselection has occurred.  Use the SIM_WS pointed
     * to by the SIM_SOFTC "active_io."
     */
    else {
	if (sc->active_io == (SIM_WS *)NULL) {
	    sws = &sc->tmp_ws;
	    sc_setup_ws(sc, sws, targid, 0l);
            SC_ADD_FUNC(sws, "sim94_get_ws: active io is NULL");
	}
	else {
	    sws = sc->active_io;
            SC_ADD_FUNC(sws, "sim94_get_ws: using active io");
        }
    }

    /*
     * If the DIS bit is set in the interrupt register, then there
     * is no active I/O on the bus.  Otherwise, set the active_io to
     * the SIM_WS which was found.
     */
    if (ir & SIM94_INTR_DIS)
	sc->active_io = (SIM_WS *)NULL;
    else
	sc->active_io = sws;

    /*
     * Determine if a parity error has occurred.
     */
    if (sr & SIM94_STAT_PE) {

	/*
	 * Increment the parity error count in the NEXUS.
	 */
	sws->nexus->parity_cnt++;

	/*
	 * What phase is this parity error on?
	 */
	switch(ssc->phase) {

	case SC_PHASE_MSGIN:
	    CAM_ERROR(module,
		      "Parity error on message in byte",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);

	    sws->error_recovery |= ERR_MSGIN_PE;
	    break;

	case SC_PHASE_DATAIN:
	    CAM_ERROR(module,
		      "Parity error on data in byte",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    sws->error_recovery |= ERR_DATAIN_PE;
	    break;

	case SC_PHASE_STATUS:
	    CAM_ERROR(module,
		      "Parity error on status",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    sws->error_recovery |= ERR_STATUS_PE;
	    break;

	default:
	    CAM_ERROR(module,
		      "Parity error",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    sws->error_recovery |= ERR_PARITY;
	    break;
	}
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, seq_num 0x%x\n", sws->seq_num ));

    return(sws);
}
/*
 * Routine Name :  sim94_sel_timeout
 *
 * Functional Description :
 *	Sim94_sel_timeout will handle the case that no selection timeout 
 *	interrupt was generated probably due to the fact that the SCSI 
 *	cable had been pulled from the system and the bus was left 
 *	unterminated (Needed for ASE). First we check whether 
 *	SZ_TRYING_SELECT is set which should be the case since this 
 *	timeout is cancelled when a selection succeeds or on the receipt 
 *	of a selection T.O. interrupt. If SZ_TRYING_SELECT is set, we 
 *	perform the normal I/O completion operations and return
 *	the I/O with a CAM status of CAM_SEL_TIMEOUT.
 *
 *	NOTE: 	This is really an Arbitration Timeout since the arbitration 
 *		could not complete due to the cable being removed from 
 *		the system. However, we still inform the PDRV that it is 
 *		a selection T.O. to keep consistent.
 *
 * Call Syntax :
 *	sim94_sel_timeout(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *
 * Return Value :  None
 */
static void
sim94_sel_timeout(sc)
SIM_SOFTC *sc;
{
    SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    int s;

    SIM_SOFTC_LOCK(s, sc);

    if ( !(sc->flags & SZ_TRYING_SELECT)) {
    	SIM_SOFTC_UNLOCK(s, sc);
	return;
    }

    sc_sel_timeout(ssc->sws);
    (void) DME_END(sc, &ssc->sws->data_xfer);
    ss_finish(ssc->sws);
    sc->flags &= ~SZ_TRYING_SELECT;
    sc->hba_chip_reset(sc); 
    (void) sx_command_complete(ssc->sws);
    SIM_SOFTC_UNLOCK(s, sc);
    ss_sched(sc);
}

/*
 * Routine Name :  sim94_add_sm_queue
 *
 * Functional Description :
 *	Sim94_add_sm_queue will store the interrupt register,
 *	status register, and sequence step register in the state
 *	machine's queue.  Called with SOFTC SMP locked.
 *
 * Call Syntax :
 *	sim94_add_sm_queue(sc, sws, ir, sr, ssr)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *	U32 ir			-- interrupt register
 *	U32 sr			-- status register
 *	U32 ssr			-- sequence step register
 *
 * Return Value :  None
 */
static void
sim94_add_sm_queue(sc, ir, sr, ssr)
register SIM_SOFTC *sc;
register U32 ir, sr, ssr;
{
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    register SIM_SM_DATA *ssm;
    register SIM94_INTR *sintr;
    extern SIM_SM sim_sm;
    SIM_MODULE(sim94_add_sm_queue);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_SM,
	       ("ir 0x%x, sr 0x%x, ssr 0x%x\n", ir, sr, ssr));

    /*
     * Get the needed space to store the interrupt data.
     */
    SIM94_GET_INTR(ssc, sintr);

    /*
     * Save all interrupt information.
     */
    sintr->cntl = reg->sim94_tclsb;
    sintr->cnth = reg->sim94_tcmsb;
    sintr->cmd = reg->sim94_cmd;
    sintr->sr = sr;
    sintr->ir = ir;
    sintr->ssr = ssr;
    sintr->fflag = reg->sim94_ffss;
    sintr->cnf1 = reg->sim94_cnf1;
    sintr->cnf2 = reg->sim94_cnf2;
    sintr->cnf3 = reg->sim94_cnf3;

    /*
     * Get space on the SIM State Machine's queue.
     */
    SC_GET_SM_BUF(&sim_sm, ssm);

    /*
     * Store the Interrupt data and the SIM_WS in this structure which
     * is already on the State Machine's queue.
     */
    ssm->hba_intr = (u_char *)sintr;
    ssm->sim_sc = sc;
}

/*
 * Routine Name : sim94_targ_sm
 *
 * Functional Description :
 *	Sim94_targ_sm() is called by sim94_sm when the NCR53C94
 *	is operating as a target. It will perform all
 *	specific actions necessary to handle the current phase.  It
 *	will then pass control to the generic target mode functions.
 *
 * Call Syntax :
 *	sim94_targ_sm(ssm)
 *
 * Arguments :
 *	SIM_SM *ssm	- State machine structure pointer, contains
 *			  a SIM_WS pointer and a pointer to HBA
 *			  specific interrupt information.
 *
 * Return Value :  None
 */
sim94_targ_sm(ssm, sws)
register SIM_SM_DATA *ssm;
register SIM_WS *sws;
{
    U32 sr = SIM94_GET_SR((SIM94_INTR *)ssm->hba_intr);
    U32 ir = SIM94_GET_IR((SIM94_INTR *)ssm->hba_intr);
    U32 ssr = SIM94_GET_SSR((SIM94_INTR *)ssm->hba_intr);
    register SIM_TARG_WS *tws = &sws->targ_ws;
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    register U_WORD last_cmd;
    U32 fifo_ln;
    int s, i;
    SIM_MODULE(sim94_targ_sm);

    /*
     * If this is a bus free interrupt, return.
     */
    if (ir & SIM94_INTR_DIS)
	return;

    /*
     * Determine if ATN is set.  Set TARG_ATNSET.
     */
    if (ir & SIM94_INTR_BS)
	tws->flags |= TARG_ATNSET;

    /*
     * If "function complete" is not set, check to see
     * if selected with ATN or selected.
     */
    if (!(ir & SIM94_INTR_FC)) {

	if (ir & SIM94_INTR_SELATN) {

	    switch(ssr & SIM94_SSTEP_MASK) {

	    case SIM94_SSTEP_NOCMD:
		/*
		 * Selection Complete.  Received one message byte and the
		 * entire command descriptor block.
		 */

		/*
		 * Make sure that "bus service" isn't set in the IR.
		 * If so, initiator asserted ATN during command phase.
		 */
		if (ir & SIM94_INTR_BS) {
		    CAM_ERROR(module,
			      "Select with ATN and BS",
			      SIM_LOG_HBA_INTR|SIM_LOG_PRISEVERE,
			      NULL, sws, NULL);
		}

		SC_NEW_PHASE(sws, SCSI_ARBITRATION);
		SC_NEW_PHASE(sws, SCSI_SELECTION);
		SC_NEW_PHASE(sws, SCSI_MSGOUT);
		SC_NEW_PHASE(sws, SCSI_COMMAND);

		/*
		 * Read the message from the FIFO.  This should
		 * be the identify message.
		 */
		SIM94_READ_MESSAGE_OUT(sws, reg);

		/*
		 * The remaining bytes in the FIFO is the command.
		 * Determine how many bytes this is.
		 */
		fifo_ln = SIM94_GET_FIFO_LN(reg);

		/*
		 * Make sure that we have the space for this command.
		 * If not, then we don't support the command.
		 */
		if ((fifo_ln + sws->targ_ws.command_len) >
                     SIM_TARG_MODE_CMD_LN) {
                    sws->targ_ws.command_len = 0;
		    SIM94_FLUSH_FIFO(reg);
                }

		/*
		 * Read the command from the FIFO.
		 */
		for (i=0; i < fifo_ln; i++)
		    sws->targ_ws.command[i + sws->targ_ws.command_len] =
	                SIM94_READ_FIFO(reg);
		sws->targ_ws.flags |= TARG_GOT_CMD;

		/*
		 * Handle the message byte.
		 */
		sm_targ_msgout(sws);

		/*
		 * Handle the command.
		 */
                sws->targ_ws.command_len += fifo_ln;
                sws = (SIM_WS *) sm_targ_command(sws);/* Return with real WS */
		
		break;

	    case SIM94_SSTEP_TARG_MSGO:

		if (!(ir & SIM94_INTR_BS)) {
		    CAM_ERROR(module,
			      "Select with ATN but no BS",
			      SIM_LOG_HBA_INTR|SIM_LOG_PRISEVERE,
			      NULL, sws, NULL);
		    return;
		}

		/*
		 * Selected with ATN, stored Bus ID and one Message byte.
		 * Stopped because ATN remained true after 1st message byte.
		 */

		SC_NEW_PHASE(sws, SCSI_ARBITRATION);
		SC_NEW_PHASE(sws, SCSI_SELECTION);
		SC_NEW_PHASE(sws, SCSI_MSGOUT);

		/*
		 * Read the message from the FIFO.  This should
		 * be the identify message.
		 */
		SIM94_READ_MESSAGE_OUT(sws, reg);

		/*
		 * Handle the message byte.
		 */
		sm_targ_msgout(sws);

		break;


	    default:
		CAM_ERROR(module,
			  "Bad sequence step register",
			  SIM_LOG_HBA_INTR|SIM_LOG_PRISEVERE,
			  NULL, sws, NULL);
	    }
	}

	if (sr & SIM94_INTR_SEL) {
	    CAM_ERROR(module,
		      "Selected without ATN",
		      SIM_LOG_HBA_INTR|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	}
    }

    /*
     * Function Complete was set.  
     */
    else {

	last_cmd = SIM94_LAST_CMD(sws) & ~SIM94_CMD_DMA;

	switch (last_cmd) {

	case SIM94_CMD_RECMSGSEQ:
	    /*
	     * The last command was a receive message command.
	     * There should be one message byte in the fifo.
	     * Read it from the FIFO and call sm_targ_msgout
	     * to handle it.
	     */

	    /*
	     * There should be one byte in the FIFO.
	     */
	    if ((fifo_ln = SIM94_GET_FIFO_LN(reg)) != 1)
		CAM_ERROR(module,
			  "Fifo length is NOT one byte",
			  SIM_LOG_HBA_INTR|SIM_LOG_PRISEVERE,
			  NULL, sws, NULL);

	    /*
	     * Read the message from the FIFO.  This should
	     * be the identify message.
	     */
	    SIM94_READ_MESSAGE_OUT(sws, reg);
	    SC_NEW_PHASE(sws, SCSI_MSGOUT);

	    sm_targ_msgout(sws);
	    break;

        case SIM94_CMD_RECCMD:
	case SIM94_CMD_RECCMDSEQ:
	    /*
	     * The last command was a receive command command.
	     * The entire command should be in the fifo.
	     * Read it from the FIFO and call sm_targ_command
	     * to handle it.
	     */

	    /*
	     * Determine how many bytes are in the FIFO.
	     */
	    fifo_ln = SIM94_GET_FIFO_LN(reg);

            /*
             * If the command wont fit in our buffer, flush the FIFO and 
	     * allow sm_targ_command() to reject the command.
             */
            if ((sws->targ_ws.command_len + fifo_ln) >= SIM_TARG_MODE_CMD_LN) {
                fifo_ln = 0;
		SIM94_FLUSH_FIFO(reg);
            }

	    for (i=0; i < fifo_ln; i++)
		sws->targ_ws.command[i + sws->targ_ws.command_len] =
	            SIM94_READ_FIFO(reg);

            sws->targ_ws.command_len += fifo_ln;
            sws->targ_ws.flags |= TARG_GOT_CMD;
	    sws = (SIM_WS *) sm_targ_command(sws);/* Return with real WS */
	    break;

	case SIM94_CMD_TERMSEQ:
	    /*
	     * The Command has completed.  Simply return.
	     */
	    return;

	default:

	    /*
	     * If the command has completed, then we are done.
	     */
	    if (tws->flags & TARG_CMDCPLT)
		break;

	    /*
	     * Has all data been transferred?  If so, complete the
	     * command.
	     */
	    if ((sws->data_xfer.flags & DME_DONE) &&
		!(tws->flags & TARG_CMDCPLT)) {
		    tws->flags |= TARG_CMDCPLT;
		    DME_END(sc, &sws->data_xfer);
	    }

           break;
	}
    }

    /*
     * Move onto the next step.
     */
    targ_sm(sws);

}

/*
 * Routine Name : sim94_targ_cmd_cmplt
 *
 * Functional Description :
 *	Sim94_targ_cmd_cmplt() is called to perform the
 *	"terminate sequence" when operating in target mode.
 *	The 94 will send the status byte and a message (message in) byte.
 *
 * Call Syntax :
 *	sim94_targ_cmd_cmplt(sws)
 *
 * Arguments :
 *	SIM_WS *sws	- SIM_WS pointer 
 *
 * Return Value :  None
 */
U32
sim94_targ_cmd_cmplt(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = sws->sim_sc;
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    int s;
    SIM_MODULE(sim94_targ_cmd_cmplt);

    SC_ADD_FUNC(sws, module);

    /*
     * Use the "terminate sequence" command to cause the ASC to first
     * assert status phase, send one byte; then assert message in phase
     * and send one more byte.   If ATN is asserted by the initiator,
     * the bus service and function complete bits will be set; an
     * interrupt will be generated; but the ASC will not disconnect.
     */

    SIM_REG_LOCK(s, sc);

    /*
     * The status should be in "scsi_status" of the SIM_WS.
     */
    SIM94_LOAD_FIFO_BYTE(reg, sws->scsi_status);
    WBFLUSH();

    /*
     * The message byte will be command complete.  This message should
     * not be on the message in queue.
     */
    SIM94_LOAD_FIFO_BYTE(reg, SCSI_COMMAND_COMPLETE);
    WBFLUSH();

    /*
     * Start the terminate sequence command.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_TERMSEQ, sim_poll_mode);
    WBFLUSH();
    /*
     * Since we have done the command complete sequence, clear this
     * flag.
     */
     sws->targ_ws.flags &= ~TARG_CMDCPLT;

    SIM_REG_UNLOCK(s, sc);

}

/*
 * Routine Name : sim94_targ_recv_cmd
 *
 * Functional Description :
 *	Sim94_targ_recv_cmd() is called to instruct the 94
 *	to read the SCSI command from the bus.
 *
 * Call Syntax :
 *	sim94_targ_recv_cmd(sws)
 *
 * Arguments :
 *	SIM_WS *sws	- SIM_WS pointer 
 *
 * Return Value :  None
 */
U32
sim94_targ_recv_cmd(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = sws->sim_sc;
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    int s;
    SIM_MODULE(sim94_targ_recv_cmd);

    /*
     * Raise IPL and SMP Lock.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Start the receive command.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_RECCMDSEQ, sim_poll_mode);
    WBFLUSH();

    /*
     * Unlock and lower IPL.
     */
    SIM_REG_UNLOCK(s, sc);
}

/*
 * Routine Name : sim94_targ_send_msg
 *
 * Functional Description :
 *	Sim94_targ_recv_cmd() is called to instruct the 94
 *	to send a message (message in phase).
 *
 * Call Syntax :
 *	sim94_targ_send_msg(sws)
 *
 * Arguments :
 *	SIM_WS *sws	- SIM_WS pointer 
 *
 * Return Value :  None
 */
U32
sim94_targ_send_msg(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = sws->sim_sc;
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    register int count = SC_GET_MSGIN_LEN(sws);
    register int s, i;
    SIM_MODULE(sim94_targ_send_msg);

    /*
     * Raise IPL and SMP Lock.
     */
    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Load the message bytes in the FIFO, one at a time.
     */
    for (i=0; i < count; i++) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
		   ("message in: 0x%x\n", SC_GET_MSGIN(sws, i)));
	SIM94_LOAD_FIFO_BYTE(reg, SC_GET_MSGIN(sws, i));
	WBFLUSH();
    }

    /*
     * Update the message in queue.
     */
    SC_UPDATE_MSGIN(sws, count);

    /*
     * Start the send message command.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_SNDMSG, sim_poll_mode);
    WBFLUSH();

    SIM_REG_UNLOCK(s, sc);
}

/*
 * Routine Name : sim94_targ_disconnect
 *
 * Functional Description :
 *	Sim94_targ_disconnect() is called to instruct the 94
 *	to disconnect from the bus.  Not implemented.
 *
 * Call Syntax :
 *	sim94_targ_disconnect(sws)
 *
 * Arguments :
 *	SIM_WS *sws	- SIM_WS pointer 
 *
 * Return Value :  None
 */
U32
sim94_targ_disconnect(sws)
SIM_WS *sws;
{
    SIM_MODULE(sim94_targ_disconnect);
}

/*
 * Routine Name : sim94_targ_recv_msg
 *
 * Functional Description :
 *	Sim94_targ_recv_msg() is called to instruct the 94
 *	to read a message from the bus (message in phase).
 *
 * Call Syntax :
 *	sim94_targ_recv_msg(sws)
 *
 * Arguments :
 *	SIM_WS *sws	- SIM_WS pointer 
 *
 * Return Value :  None
 */
U32
sim94_targ_recv_msg(sws)
register SIM_WS *sws;
{
    SIM_SOFTC *sc = sws->sim_sc;
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM_TARG_WS *tws = &sws->targ_ws;
    int s;
    SIM_MODULE(sim94_targ_recv_msg);

    tws->flags &= ~TARG_ATNSET;

    SIM_REG_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Make sure that attention is asserted.  Lock on REG.
     */
    SIM94_SEND_CMD(sws, reg, SIM94_CMD_RECMSGSEQ, sim_poll_mode);
    WBFLUSH();

    SIM_REG_UNLOCK(s, sc);
}

/*
 * Routine Name : sim94_error_recov
 *
 * Functional Description :
 *	Sim94_error_recov is used during controller error recovery
 *	procedures (such as race conditions around a bus reset).
 *	This function is called by the interrupt handler at high IPL.
 *
 * Call Syntax :
 *	sim94_error_recov(sc, ir, sr, ssr)
 *
 * Arguments:
 *	SIM_SOFTC *sc		-- SIM_SOFTC pointer
 *	U32 ir			-- interrupt register
 *	U32 sr			-- status register
 *	U32 ssr			-- sequence step register
 *
 * Return Value :  None
 */
static void
sim94_error_recov(sc, ir, sr, ssr)
register SIM_SOFTC *sc;
register U32 ir, sr, ssr;
{
    register volatile SIM94_REG *reg = SIM94_GET_CSR(sc);
    SIM94_SOFTC *ssc = (SIM94_SOFTC *)sc->hba_sc;
    SIM_SM_DATA ssm;
    register SIM_WS *sws;
    register SIM94_INTR *sintr;
    int s1, s2;
    SIM_MODULE(sim94_error_recov);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Lock on the SOFTC and REG.
     */
    SIM_SOFTC_LOCK(s1, sc);
    SIM_REG_LOCK(s2, sc);

    /*
     * Use the temp. SIM working set.
     */
    sws = &sc->tmp_ws;

    /*
     * Is this the first time that this routine has been called
     * since bus free?  If so, perform some setup.
     */
    if (!sc->err_recov_cnt++) {
	sws->sim_sc = (SIM_SOFTC *)sc;
	sws->nexus = &sc->nexus[0][0];

	/*
	 * Make sure that attention is asserted.  Lock on REG.
	 */
	SIM94_SEND_CMD(sws, reg, SIM94_CMD_SETATN, 0);
	WBFLUSH();

	/*
	 * We don't know what the last command was, but there is
	 * a good chance that it was a select command if the
	 * bus was free and function complete is set.
	 */
	SIM94_STORE_CMD(sws, SIM94_CMD_SELECT);

	/*
	 * Set the bus phase to bus free.
	 */
	SC_NEW_PHASE(sws, SCSI_BUS_FREE);
    }

    /*
     * Set the error recovery bits.
     */
    sws->error_recovery = sc->error_recovery;

    /*
     * Get the needed space to store the interrupt data.
     */
    SIM94_GET_INTR(ssc, sintr);

    /*
     * Save all interrupt information.
     */
    sintr->cntl = reg->sim94_tclsb;
    sintr->cnth = reg->sim94_tcmsb;
    sintr->cmd = reg->sim94_cmd;
    sintr->sr = sr;
    sintr->ir = ir;
    sintr->ssr = ssr;
    sintr->fflag = reg->sim94_ffss;
    sintr->cnf1 = reg->sim94_cnf1;
    sintr->cnf2 = reg->sim94_cnf2;
    sintr->cnf3 = reg->sim94_cnf3;

    /*
     * Handle the "function complete" condition.
     */
    if (ir & SIM94_INTR_FC)
	sim94_function_complete(sws, ir, sr, ssr);

    /*
     * Setup the SIM State Machine structure.
     */
    ssm.sim_sc = sc;
    ssm.hba_intr = (u_char *)sintr;

    /*
     * Call the NCR53C94 specific state machine to handle the current
     * phase.  Unlock the REG and SOFTC before calling.
     */
    SIM_REG_UNLOCK(s2, sc);
    SIM_SOFTC_UNLOCK(s1, sc);
    sim94_sm(&ssm);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  sim94_logger
 *
 * Functional Description :
 *	This function logs errors specific to the 94 SIM modules.
 * 
 * Call Syntax:
 *	sim94_logger(func, msg, sc, sws, intr, flags)
 *
 * Arguments:
 *	u_char *func		calling module name, may be NULL
 *	u_char *msg		error description, may be NULL
 *	SIM_SOFTC *sc		SIM_SOFTC pointer, may be NULL
 *	SIM_WS *sws		SIM_WS pointer, may be NULL
 *	SIM94_INTR *intr	SIM94_INTR pointer, may be NULL
 *	U32 flags		flags described in sim.h
 *
 * Return Value : None
 */
void
sim94_logger(func, msg, flags, sc, sws, intr)
u_char *func;
u_char *msg;
U32 flags;
SIM_SOFTC *sc;
SIM_WS *sws;
SIM94_INTR *intr;
{
    SIM_MODULE(sim94_logger);
    register CAM_ERR_HDR hdr;
    static CAM_ERR_ENTRY entrys[SIM_LOG_SIZE];
    register CAM_ERR_ENTRY *entry;
    SIM94_INTR *tintr;
    SIM94_SOFTC *tssc;

    hdr.hdr_type = CAM_ERR_PKT;
    hdr.hdr_class = CLASS_SIM94;
    hdr.hdr_subsystem = SUBSYS_SIM94;
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
     * Log the active SIM94_INTR structure.
     */
    if (flags & SIM_LOG_HBA_INTR) {
	tintr = (SIM94_INTR *)NULL;
	if (intr != (SIM94_INTR *)NULL)
	    tintr = intr;
	else if (sc != (SIM_SOFTC *)NULL)
	    if (sc->hba_sc != (void *)NULL)
		tintr = ((SIM94_SOFTC *)sc->hba_sc)->active_intr;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		if (sws->sim_sc->hba_sc != (void *)NULL)
		    tintr =
			((SIM94_SOFTC *)sws->sim_sc->hba_sc)->active_intr;
	if (tintr != (SIM94_INTR *)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIM94_INTR;
	    entry->ent_size = sizeof(SIM94_INTR);
	    entry->ent_vers = SIM94_INTR_VERS;
	    entry->ent_data = (u_char *)tintr;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Log the SIM94_SOFTC
     */
    if (flags & SIM_LOG_HBA_SOFTC) {
	tssc = (SIM94_SOFTC *)NULL;
	if (sc != (SIM_SOFTC *)NULL)
	    tssc = (SIM94_SOFTC *)sc->hba_sc;
	else if (sws != (SIM_WS *)NULL)
	    if (sws->sim_sc != (SIM_SOFTC *)NULL)
		tssc = sws->sim_sc->hba_sc;
	if (tssc != (SIM94_SOFTC *)NULL) {
	    entry = &hdr.hdr_list[hdr.hdr_entries++];
	    entry->ent_type = ENT_SIM94_SOFTC;
	    entry->ent_size = sizeof(SIM94_SOFTC);
	    entry->ent_vers = SIM94_SOFTC_VERS;
	    entry->ent_data = (u_char *)tssc;
	    entry->ent_pri = PRI_FULL_REPORT;
	}
    }

    /*
     * Call sc_logger to log the common structures.
     */
    sc_logger(&hdr, SIM_LOG_SIZE, sc, sws, flags);
}

int
sim94_unload( int controller, int misc )
{
    SIM_MODULE(sim94_unload);
    return 1;
}

