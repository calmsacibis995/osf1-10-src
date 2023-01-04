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
static char *rcsid = "@(#)$RCSfile: sim_sm.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/12/09 15:06:56 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_sm.c		Version 1.24			Jan. 10, 1992 

	This file contains the SCSI state machine array.

Modification History:

	1.24	01/10/92	janet
	The SM_DATA struct has been modified to contain a pointer 
	to the SIM_SOFTC instead of to the SIM_WS.

	1.23	12/23/91	janet
   	Made selection to status a valid transition.

	1.22	12/16/91	janet
	In sm_status() added the use a phase mask to mask out the
	reserved bits of the status.  In sm_unexpected() don't
	log the "Unexpected bus free" error at boot time.  This
	is being done to get around an RRD40 problem.  In sm_msgin()
	reject the modify data pointers message.

	1.21	12/10/91	janet
	Modified sm_bus_free() to start the next request before
	releasing the DME resources from the completing I/O.

	1.20	11/20/91	janet
	Changed a bunch of variables to be registers.

	1.19	11/13/91	janet
	Removed all SC_ADD_FUNC() calls.  Added sim error logging.

	1.18	10/22/91	janet
	o Added SIM_MODULE() to every function.
	o Replaced all PRINTD's with SIM_PRINTD's.

	1.17	09/13/91	janet
	When processing a command complete message, make sure that
	a status phase has occured.

	1.16	09/12/91	janet
	In sm_bus_free() clear the SZ_BUS_FREE_EXP flag.

	1.15	09/10/91	janet
	In sm_bus_free() check for SZ_ABORT. Moved clearing of SZ_UNTAGGED
	from sm_bus_free() to ss_finish().  In sm_unexpected() clear up any
	inconsistant state.  When a bus device reset message is sent, set
	SZ_DEVRS_INPROG.  When an abort message	is sent, set SZ_ABORT_INPROG.
	When an abort tag message is sent, set SZ_ABORT_TAG_INPROG.
	Reorganized the file history.

	1.14	08/08/91	maria
	Fixed check in scsiisr to start	off state machine.

	1.13	07/26/91	janet
	Made sel->msgin a valid phase change.

	1.12	07/24/91	janet
	In sm_msgin, and sync message, if period and offset are both zero, then
	we are running async.

	1.11	07/08/91	janet
	In sm_bus_free() if a bus device reset was performed, try to start
	another	request.  In sm_unexpected() if bus free, always call
	sm_bus_free().

        1.10    06/28/91        rps
	Changed for dynamic configuration.

	1.09	06/20/91	janet
	Fixed sm_bus_free() to try to start a new request anytime the bus is
	free.

	1.08	06/18/91	janet
	Re-wrote scsiisr to use one queue for all controllers.

	1.07	06/07/91	janet
	Fixed computation of "bytes_left" during message reject.

	1.06	06/05/91	janet
	sm_msg_rejected() will process the identify message before all others

	1.05	06/04/91	janet
	Ignore value returned by DME_END(). Check value returned by
	DME_RESUME(), DME_SAVE(), DME_RESTORE(), and DME_MODIFY().

	1.04	05/29/91	janet
	Sync info is kept per target, not per lun.  Use the IT_NEXUS struct.

	1.03	04/09/91	jag
	In SDP/MDP if dme is not setup these messages are treated like no-ops.

	1.02	04/02/91	jag
	In scsiisr() the loop on controllers checks for empty slots in
	softc_dir[].

	1.01	03/26/91	janet
	Updated after code review.

	1.00	11/13/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define SM_ACTIVE 0x8000
#define CAMERRLOG

/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <kern/parallel.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/cam_errlog.h>

/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern void	bzero();
extern void	ss_reset_detected();
extern U32	ss_sched();
extern void	ss_device_reset_done();
extern void	ss_finish();
extern void	as_start();
extern void	sc_gen_msg();
extern void	sim_err_sm();
extern void	sim_logger();
extern U32	sx_command_complete();
extern SIM_WS	*sc_find_ws();
extern SIM_SOFTC *softc_directory[];
extern int	shutting_down;
extern struct timeval time;

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
void		scsiisr();
void		sm_bus_free();
void		sm_arbitration();
void		sm_selection();
void		sm_reselection();
void		sm_command();
void		sm_datain();
void		sm_dataout();
void		sm_status();
void		sm_msgin();
void		sm_msgout();
void		sm_unexpected();
static void	sm_msg_rejected();
static void	sm_copy_ws();
static u_short	sm_last_msgout();
void (*scsi_sm[SCSI_NSTATES][SCSI_NSTATES])();
static void (*local_errlog)() = sim_logger; 


/* ---------------------------------------------------------------------- */

/*
 * Routine Name :  scsiisr
 *
 * Functional Description :
 *	The generic SCSI state machine.  This function will be scheduled
 *	by the HBA specific interrupt handler and by the SIM scheduler's
 *	start routine.  This function will be called via a software interrupt
 *	and performs the following:
 *
 * 1.  Check for bus reset.  If set, clear out all interrupts for
 *     that controller and call ss_reset_detected().  Continue.
 * 2.  Check for a new request.  If so, check active_io for that
 *     controller.  If NULL, call ss_sched().  Continue.
 * 3.  Get a SIM_SM struct off of the queue.  If NULL, then
 *     this entry was removed after a bus reset, continue.  If
 *     -1, then there are no entries, we are done.  Otherwise
 *     call SC_HBA_SM().
 *
 * Call Syntax :
 *	scsiisr()
 *
 * Arguments :  None
 *
 * Return Value :  None
 */
void
scsiisr()
{
    register SIM_SM_DATA *ssm;
    register int busy = CAM_TRUE;
    register int s;
    extern SIM_SM sim_sm;
    extern SIM_SOFTC sim_softc[];
    SIM_MODULE(scsiisr);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    SIM_SM_LOCK(s, &sim_sm);
    sim_sm.sm_active = CAM_TRUE;
    SIM_SM_UNLOCK(s, &sim_sm);

    while (busy) {


	/*
	 * Lock on the State Machine.
	 */
	SIM_SM_LOCK(s, &sim_sm);

	/*
	 * Determine if a SCSI bus reset has occurred.
	 */
	if (sim_sm.bus_reset) {

	    int i;
	    int controller = ffs(sim_sm.bus_reset) - 1;
	    SIM_SOFTC *sc = softc_directory[controller];
	    u_short index = CIRQ_CURR(sim_sm.sm_queue);

	    /*
	     * Clear the bus_reset bit.
	     */
	    sim_sm.bus_reset &= ~(1 << controller);

	    /*
	     * If so, go through the state machine's queue and
	     * clear out all interrupts for this controller.
	     */
	    for (i=0; i < CIRQ_CURR_SZ(sim_sm.sm_queue); i++) {

		if (sim_sm.sm_data[index].sim_sc != (SIM_SOFTC *)NULL)
		    if (sim_sm.sm_data[index].sim_sc->cntlr == controller)
			sim_sm.sm_data[index].sim_sc = (SIM_SOFTC *)NULL;

		index = CIRQ_ADJUST_INDEX(sim_sm.sm_queue, index, 1);
	    }

	    /*
	     * Unlock on the State Machine.
	     */
	    SIM_SM_UNLOCK(s, &sim_sm);

	    /*
	     * Call ss_reset_detected().
	     */
	    ss_reset_detected(sc);

	    continue;
	}

	/*
	 * Determine if a new request is waiting.
	 */
	if (sim_sm.waiting_io) {

	    int controller = ffs(sim_sm.waiting_io) - 1;
	    SIM_SOFTC *sc = softc_directory[controller];

	    /*
	     * Unlock on the State Machine.
	     */
	    SIM_SM_UNLOCK(s, &sim_sm);

	    /*
	     * Start the IO.
	     */
	    ss_sched(sc);

	    continue;
	}
	
	/*
	 * Get a SIM_SM pointer from the state machine's
	 * waiting queue.
	 */
	SC_GET_SM(&sim_sm, ssm);

	/*
	 * If the state machine's queue is empty, then we are done.
	 */
	if (ssm == (SIM_SM_DATA *) -1) {

	    sim_sm.sm_active = CAM_FALSE;
	    busy = CAM_FALSE;


	    /*
	     * Unlock on the State Machine.
	     */
	    SIM_SM_UNLOCK(s, &sim_sm);

	    continue;
	}

	/*
	 * Unlock on the State Machine.
	 */
	SIM_SM_UNLOCK(s, &sim_sm);

	/*
	 * Start the HBA specific state machine.
	 */
	if (ssm->sim_sc != (SIM_SOFTC *)NULL)
	    SC_HBA_SM(ssm->sim_sc, ssm);
    }

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}


/*
 * Routine Name :  sm_bus_free
 *
 * Functional Description :
 *	Sm_bus_free() will perform all needed "clean-up" after a bus
 *	free has been detected.  An attempt will be made to start a
 *	new request.  Also, if the last request has completed, its
 *	DME resources will be released and ss_finish() will be 
 *	called.
 *
 * Call Syntax :
 *	sm_bus_free(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_bus_free(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    U32 status;
    SIM_MODULE(sm_bus_free);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Are we expecting a bus free phase?  If not, call sm_unexpected().
     */
    if (!(sws->flags & SZ_EXP_BUS_FREE)) {
	sm_unexpected(sws);
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, bus free was not expected\n"));
	return;
    }

    /*
     * Clear the expected bus free bit.
     */
    sws->flags &= ~SZ_EXP_BUS_FREE;

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_BUS_FREE.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_BUS_FREE);

    /*
     * Make sure that this isn't the "tmp_ws."
     */
    if (sws == &((SIM_SOFTC *)sws->sim_sc)->tmp_ws) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
	       ("using temp working set\n"));

	/*
	 * Start a new request.
	 */
	ss_sched(sc);
    }

    /*
     * Was this request aborted or did a BDR get sent?
     */
    else if (sws->flags & (SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG |
							SZ_DEVRS_INPROG)) {

	if (sws->flags & SZ_DEVRS_INPROG) {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("a device reset completed\n"));
	    ss_device_reset_done(sc, sws);
	}

	else {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("an abort completed\n"));
	    ss_abort_done(sc, sws);
	}

	/*
	 * Start a new request.
	 */
	ss_sched(sc);
    }

    /*
     * If the SZ_CMD_CMPLT bit is set in the SIM_WS "flags" field
     * call DME_END() to release the DME resources.
     * Also call ss_finish() to clear NEXUS and SIM_SOFTC
     * related flags.  Call ss_sched() and finally call
     * sx_command_complete() which will perform all final
     * completion steps for the request.
     */
    else if (sws->flags & SZ_CMD_CMPLT) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("a command has completed\n"));

	/*
	 * Update the flags for this SIM_WS.
	 */
	ss_finish(sws);

	/*
	 * Start a new request.
	 */
	status = ss_sched(sc);

	/*
	 * Release the DME resources.
	 */
	(void) DME_END(sc, &sws->data_xfer);

	/*
	 * If ss_sched() failed because it couldn't get a DME
	 * resource, call it again now that we freed the
	 * completing I/O's resource.
	 */
	if (status == CAM_PROVIDE_FAIL)
	    ss_sched(sc);
	
	/*
	 * Perform a call back on the completing request.
	 */
	(void) sx_command_complete(sws);
    }
	 
    /*
     * If the device just disconnected, try to start a new request.
     */
    else {
	ss_sched(sc);
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}	 

/*
 * Routine Name : sm_arbitration
 *
 * Functional Description :
 *	Sm_arbitration() will handle the SCSI arbitration phase.
 *
 * Call Syntax :
 *	sm_arbitration(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_arbitration(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_arbitration);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_ARBITRATION.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_ARBITRATION);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_selection
 *
 * Functional Description :
 *	Handle the SCSI selection phase.
 *
 * Call Syntax :
 *	sm_selection(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_selection(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_selection);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_SELECTION.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_SELECTION);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_reselection
 *
 * Functional Description :
 *	Sm_reselection() will handle the SCSI reselection
 *	phase.  The SIM_WS pointer might point to the temporary
 *	SIM_WS, from the SIM_SOFTC.  It is expected that the
 *	correct "targid" and "lun" field of the SIM_WS will
 *	have already been filled in.
 *
 * Call Syntax :
 *	sm_reselection(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_reselection(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    int s;
    SIM_MODULE(sm_reselection);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * A reselection implies "restore pointers."  Clear the
     * command and status phase summary bits.  This will restore our
     * command and status pointers to the beginning of the command.
     * This will allow the target to perform these phases, again.
     */
    sws->phase_sum &=
	~(SCSI_PHASEBIT(SCSI_STATUS) | SCSI_PHASEBIT(SCSI_COMMAND));

    /*
     * Set the SZ_RDP_NEEDED bit in the SIM_WS's flags field.
     * Before any other data is transferred, a DME_RESTORE()
     * will be performed.
     */
    sws->flags |= SZ_RDP_NEEDED;

    /*
     * Set the "active_io" pointer in the SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);
    sc->active_io = sws;
    SIM_SOFTC_UNLOCK(s, sc);
    
    /*
     * Setup the offset and period of the reselecting target.
     */
    SC_HBA_SETUP_SYNC(sc, sws);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_RESELECTION.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_RESELECTION);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_command
 *
 * Functional Description :
 *	Sm_command() will handle the SCSI command phase.
 *
 * Call Syntax :
 *	sm_command(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_command(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register u_char *cdb;
    SIM_MODULE(sm_command);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Make sure that a command phase hasn't already been completed.
     * If one has, an invalid phase sequence has occurred.  Invoke
     * the error handling state machine.
     */
    if (sws->phase_sum & SCSI_PHASEBIT(SCSI_COMMAND)) {

	CAM_ERROR(module, "Invalid command phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL,
		  sws, NULL);
	sws->error_recovery |= ERR_PHASE;
	SC_SM(sws);
	return;
    }

    /*
     * Check for the SZ_CONT_LINK bit set in the NEXUS "flags"
     * field.  If set, the last command was part of a linked
     * command.  Clear this bit.  Set the "active_io" pointer in
     * the SIM_SOFTC to the pointer field "linked_ws" in the
     * SIM_WS.  Call ss_finish() for the previous "partial"
     * command.  It will be up to the SIM XPT to determine if the
     * peripheral driver is notified via async callback.
     */
    if (sws->nexus->flags & SZ_CONT_LINK) {
	sws->nexus->flags &= ~SZ_CONT_LINK;
	if ((sc->active_io = sws->linked_ws) == (SIM_WS *)NULL) {
	    CAM_ERROR(module, "Linked command error, NULL command",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	}
	/*
	 * Release the DME resources for the previous request
	 * and call ss_finish().
	 */
	(void) DME_END(sc, &sws->data_xfer);
	ss_finish(sws);
    }

    /*
     * Determine if the CDB is a pointer.
     */
    if (sws->cam_flags & CAM_CDB_POINTER) 
	cdb = (u_char *)sws->ccb->cam_cdb_io.cam_cdb_ptr;
    else
	cdb = (u_char *)sws->ccb->cam_cdb_io.cam_cdb_bytes;

    /*
     * Call hba_xfer_info() with a pointer to the CDB.
     */
    SC_HBA_XFER_INFO(sc, sws, cdb, (U32) sws->ccb->cam_cdb_len,
		     (U32) CAM_DIR_OUT); 
       
    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_COMMAND.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_datain
 *
 * Functional Description :
 *	Sm_datain() will handle the SCSI data in phase.  The bus
 *	should be in data in phase with REQ asserted.
 *
 * Call Syntax :
 *	sm_datain(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_datain(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *) sws->sim_sc;
    SIM_MODULE(sm_datain);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Check for the SZ_NO_DME bit in the SIM_WS "flags" field.
     * If not set, the DME machine will be used. 
     *
     * The current implementation will only use DME.
     *
     * This is a place holder for future implementations.
     */

    /*
     * Do we need to restore the data pointer?  If so, call
     * DME_RESTORE().
     */
    if (sws->flags & SZ_RDP_NEEDED) {
	sws->flags &= ~SZ_RDP_NEEDED;
	if (DME_RESTORE(sc, &sws->data_xfer) != CAM_REQ_CMP) {
	    CAM_ERROR(module, "DME_RESTORE failed",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	    SC_HBA_MSGOUT_PEND(sc, sws);
	    sws->error_recovery |= ERR_UNKNOWN;
	    SC_SM(sws);
	    return;
	}
    }

    /*
     * Start the data transfer.
     */
    switch (DME_RESUME(sc, &sws->data_xfer)) {

    case CAM_REQ_CMP:
	break;

    case CAM_DATA_RUN_ERR:
	/*
	 * The target is transferring too much data.
	 */
	SC_HBA_MSGOUT_PEND(sc, sws);
	sws->error_recovery |= ERR_DATA_RUN;
	SC_SM(sws);
	return;

    case CAM_REQ_CMP_ERR:
    default:
	/*
	 * An error has occured.  Probably not setup for
	 * this data phase.
	 */
	SC_HBA_MSGOUT_PEND(sc, sws);
	sws->error_recovery |= ERR_PHASE;
	SC_SM(sws);
	return;
    }

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_DATAIN.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_DATAIN);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_dataout
 * 
 * Functional Description :
 *	Sm_dataout() will handle the SCSI data out phase.  The bus
 *	should be in data out phase with REQ asserted.
 *
 * Call Syntax :
 *	sm_dataout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_dataout(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sm_dataout);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Check for the SZ_NO_DME bit in the SIM_WS "flags" field.
     * If not set, the DME machine will be used.
     *
     * The current implementation will only use DME.
     *
     * This is a place holder for future implementations.
     */

    /*
     * Do we need to restore the data pointer?  If so, call
     * DME_RESTORE().
     */
    if (sws->flags & SZ_RDP_NEEDED) {
	sws->flags &= ~SZ_RDP_NEEDED;
	if (DME_RESTORE(sc, &sws->data_xfer) != CAM_REQ_CMP) {
	    CAM_ERROR(module, "DME_RESTORE failed",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	    SC_HBA_MSGOUT_PEND(sc, sws);
	    sws->error_recovery |= ERR_UNKNOWN;
	    SC_SM(sws);
	    return;
	}
    }

    /*
     * Start the data transfer.
     */
    switch (DME_RESUME(sc, &sws->data_xfer)) {

    case CAM_REQ_CMP:
	break;

    case CAM_DATA_RUN_ERR:
	/*
	 * The target is transferring too much data.
	 */
	SC_HBA_MSGOUT_PEND(sc, sws);
	sws->error_recovery |= ERR_DATA_RUN;
	SC_SM(sws);
	return;

    case CAM_REQ_CMP_ERR:
    default:
	/*
	 * An error has occured.  Probably not setup for
	 * this data phase.
	 */
	SC_HBA_MSGOUT_PEND(sc, sws);
	sws->error_recovery |= ERR_PHASE;
	SC_SM(sws);
	return;
    }

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_DATAOUT.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_DATAOUT);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_status
 *
 * Functional Description :
 *	Sm_status() expects to find a status byte in the "scsi_status"
 * 	field of the SIM_WS.
 *
 * Call Syntax :
 *	sm_status(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_status(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_status);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin, status of 0x%x\n", sws->scsi_status));

    SC_ADD_FUNC(sws, module);

    /*
     * Make sure that a status phase hasn't already been completed.
     * If one has, an invalid phase sequence has occurred.  Invoke
     * the error handling state machine.
     */
    if (sws->phase_sum & SCSI_PHASEBIT(SCSI_STATUS)) {

	CAM_ERROR(module, "Invalid status phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	sws->error_recovery |= ERR_PHASE;
	SC_SM(sws);
	return;
    }

    /*
     * Mask out the reserved status bits.
     */
    switch(sws->scsi_status & ~SCSI_STAT_RESERVED) {

    case SCSI_STAT_INTERMEDIATE:
    case SCSI_STAT_INTER_COND_MET:
	/* 
	 * For the status codes of SCSI_STAT_INTERMEDIATE and
	 * SCSI_STAT_INTER_COND_MET set the SZ_CONT_LINK bit
	 * in the NEXUS "flags" field.
	 */
	sws->nexus->flags |= SZ_CONT_LINK;
	break;

    case SCSI_STAT_CHECK_CONDITION:
    case SCSI_STAT_COMMAND_TERMINATED:
	/*
	 * If a check condition or command terminated, call as_start()
	 * to schedule a request sense request (if auto sense is enabled).
	 *
	 * Set the cam_status of the request to CAM_REQ_CMP_ERR.
	 */
	sws->cam_status = CAM_REQ_CMP_ERR;
	as_start(sws);
	break;

    case SCSI_STAT_GOOD:
	sws->cam_status = CAM_REQ_CMP;
	break;

    case SCSI_STAT_BUSY:
	/*
	 * In the case of SCSI_STAT_BUSY, set the "cam_status" of
	 * the SIM_WS to CAM_REQ_CMP_ERR.
	 */
	sws->cam_status = CAM_REQ_CMP_ERR;
	break;

    case SCSI_STAT_QUEUE_FULL:
    case SCSI_STAT_CONDITION_MET:
    case SCSI_STAT_RESERVATION_CONFLICT:
    default:
	sws->cam_status = CAM_REQ_CMP_ERR;
	break;
    }

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_STATUS.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_STATUS);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_msgin
 *
 * Functional Description :
 *	This routine is called via the SCSI state machine when the
 *	target has entered the message in phase. The SIM HBA will
 *	not have asserted ACK for any message bytes, but has moved
 *	one byte into the message in queue of the SIM_WS.  This
 *	routine will process the byte at the head of the message
 *	queue, execute message specific code and then call
 *	hba_msg_accept().  If there are more message bytes to be
 *	received, this routine will be entered for each message
 *	byte. For multibyte messages, the state of the message
 *	sequence will be maintained in the message in queue
 *	structure. 
 *
 * Call Syntax :
 *	sm_msgin(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_msgin(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register u_char msg;
    SIM_MODULE(sm_msgin);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Check the "needed" count for the queue.  If greater than
     * zero, accept the message without any processing.
     */
    if (CIRQ_GET_NEEDED(sws->msginq) > 0) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("need %d more message bytes before processing\n",
		    CIRQ_GET_NEEDED(sws->msginq)));
	SC_HBA_MSG_ACCEPT(sc, sws);
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));
	return;
    }

    /*
     * Determine the beginning message byte.
     */
    msg = SC_GET_MSGIN(sws, 0);

    /*
     * Reject the reserved message.
     * Message code 12h - 1Fh, 24h - 2Fh, 30h - 7fh are reserved.
     */
    if (SCSI_IS_MSG_RESERVED(msg)) {

        CAM_ERROR(module, "Reject reserved message",
                  SIM_LOG_SIM_SOFTC, NULL, sws, NULL);
        SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
        SC_HBA_MSGOUT_PEND(sc, sws);
        SC_HBA_MSG_ACCEPT(sc, sws);
        SC_ADD_PHASE_BIT(sws, SCSI_MSGIN);
        SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
               ("end\n"));
        return;
    }

    /*
     * Determine if the message is a two byte type.  If it is,
     * make sure that both bytes have been received before
     * proceeding.  It is possible that the message will be
     * rejected when it is processed, but we still need to
     * read in both bytes.
     */
    if (SCSI_IS_MSG_TWO_BYTE(msg) && (SC_GET_MSGIN_LEN(sws) == 1)) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("two byte message, still need second byte\n"));
	SC_SET_MSGIN_LEN(sws, 1);
	SC_HBA_MSG_ACCEPT(sc, sws);
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));
	return;
    }

    /*
     * Handle the identify message as a special case.
     */
    if ((msg & SCSI_IDENTIFY) == SCSI_IDENTIFY) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_IDENTIFY: 0x%x\n", msg));

	if (msg & SCSI_ID_RES_BITS) {
                CAM_ERROR(module, "Reserved ID message",
                          SIM_LOG_SIM_SOFTC, NULL, sws, NULL);
                SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
                SC_HBA_MSGOUT_PEND(sc, sws);
        	SC_HBA_MSG_ACCEPT(sc, sws);
        	SC_ADD_PHASE_BIT(sws, SCSI_MSGIN);
        	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
               		("end\n"));
		return;
        }

	/*
	 * Identify.  This is used during the reselect sequence to
	 * determine the LUN of the target that is reselecting the
	 * initiator.  Determine if this message is needed to complete
	 * the T/L NEXUS.  This is the case if we are currently running
	 * with the temporary working set.
	 */
	if (sws == &sc->tmp_ws) {
	    SIM_WS *tsws;
	    int s;

	    /*
	     * SMP lock on SOFTC.
	     */
	    SIM_SOFTC_LOCK(s, sc);
	    sws->lun = SC_DECODE_IDENTIFY(msg);
	    tsws = sc_find_ws(sc, sws->targid, sws->lun, -1L);

	    /*
	     * Are we still using the temp_ws?
	     */
	    if (tsws != sws) {
		sm_copy_ws(sws, tsws);
		sws = tsws;
		sc->active_io = sws;
	    }
	    SIM_SOFTC_UNLOCK(s, sc);
	}

	/*
	 * Accept the message.
	 */
	SC_HBA_MSG_ACCEPT(sc, sws);

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));

	return;
    }

    /*
     * Process the message.
     */
    switch(msg) {

    case SCSI_COMMAND_COMPLETE:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_COMMAND_COMPLETE: 0x%x\n", msg));

	/*
	 * The command complete, linked command complete, and linked
	 * command complete with flag messages are sent from a target
	 * to an initiator to indicate that execution of a command
	 * has completed and that status has been sent.
	 *
	 * Make sure that a status phase has occured.
	 */
	if (!(sws->phase_sum & SCSI_PHASEBIT(SCSI_STATUS))) {
	    sws->error_recovery |= ERR_PHASE;
	    SC_SM(sws);
	    return;
	}

	/*
	 * Command Complete.  Set the SZ_CMD_CMPLT flag bit in
	 * the SIM_WS "flags" field.
	 */
	sws->flags |= SZ_CMD_CMPLT;

	/*
	 * A bus free phase is now expected.
	 */
	sws->flags |= SZ_EXP_BUS_FREE;

	/*
	 * If the cam_status of the request is CAM_REQ_INPROG, set
	 * the status to CAM_REQ_CMP.
	 */
	if (sws->cam_status == CAM_REQ_INPROG)
	    sws->cam_status = CAM_REQ_CMP;

	break;

    case SCSI_DISCONNECT:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_DISCONNECT: 0x%x\n", msg));

	/*
	 * We now expect a bus free phase to occure.
	 */
	sws->flags |= SZ_EXP_BUS_FREE;
	break;

    case SCSI_EXTENDED_MESSAGE:
	/*
	 * An extended message is a multi-byte message
	 * sequence.  The format is "extended message code", count,
	 * followed by "count" message bytes.  Check the "message in"
	 * queue count.
	 */
	if (SC_GET_MSGIN_LEN(sws) == 1) {

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_EXTENDED_MESSAGE\n"));

	    /*
	     * If the count is 1 the extended count still needs to be read
	     * in.  Set the message in queue's "needed" value to one
	     * by calling SC_SET_MSGLEN().
	     */
	    SC_SET_MSGIN_LEN(sws, 1);
	    break;  /* Break out of SCSI_EXTENDED_MESSAGE. */
	}

	/*
	 * If the count is 2 the extended count was just read in.  Set
	 * the needed value to the extended count by calling
	 * SC_SET_MSGIN_LEN().
	 */
	if (SC_GET_MSGIN_LEN(sws) == 2) {

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_EXTENDED_MESSAGE: length is 0x%x\n",
			SC_GET_MSGIN(sws, 1)));

	    SC_SET_MSGIN_LEN(sws, SC_GET_MSGIN(sws, 1));

	    break;  /* Break out of SCSI_EXTENDED_MESSAGE. */
	}

	/*
	 * If count is greater than 2 then entire extended message
	 * has been read in.  Process this message.
	 */
	switch(SC_GET_MSGIN(sws, 2)) {
		
	case SCSI_SYNCHRONOUS_XFER: {

	    u_char period = SC_GET_MSGIN(sws, 3);
	    u_char offset = SC_GET_MSGIN(sws, 4);
	    int s;

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_EXTENDED_MESSAGE: SYNCH_XFER\n"));
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_SYNCHRONOUS_XFER: period 0x%x\n",
			period));
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_SYNCHRONOUS_XFER: offset 0x%x\n",
			offset));

	    /*
	     * Did we get a valid sync negotiation message?  If not,
	     * then reject it, and try again later.
	     */
	     if ( SC_GET_MSGIN_LEN(sws) != 5 ) {
		CAM_ERROR(module, "Illegal SDT message length",
			  SIM_LOG_SIM_WS|SIM_LOG_SIM_SOFTC, NULL, sws, NULL);
		SC_ADD_MSGOUT(sws, SCSI_ABORT);
		SC_HBA_MSGOUT_PEND(sc, sws);
		break;
            }

	    /*
	     * SMP lock on SOFTC.
	     */
	    SIM_SOFTC_LOCK(s, sc);

	    /*
	     * If synchronous data transfer request, check the
	     * current synchronous request state.  This could be
	     * the initial request or the final in the negotiation.
	     */

	    /*
	     * Check for the SZ_SYNC_NEG bit in the IT_NEXUS "flags"
	     * field.  If set then this is the target responding
	     * to our sync request.
	     */
	    if (sws->it_nexus->flags & SZ_SYNC_NEG) {

		/*
		 * Clear SZ_SYNC_NEG.
		 */
		sws->it_nexus->flags &= ~SZ_SYNC_NEG;

		/*
		 * Check the transfer period.  It should
		 * be equal or greater to "sync_period" in SIM_SOFTC.
		 *
		 * Check the offset.  It should be equal or less to
		 * "sync_offset" in SIM_SOFTC.
		 */
                if (((period >= sc->sync_period)&&(offset <= sc->sync_offset))
		    || (offset == 0)) {

		    /*
		     * If the offset is zero, then we are running async.  
		     * Set the period to zero, if offset is zero.
		     */
		    if (offset == 0) {
			period = 0;
		    }

		    /*
		     * If both are good, store them in the IT_NEXUS
		     * "sync_offset" and "sync_period" and set the
		     * SZ_SYNC bit in the "flags" field on the IT_NEXUS.
		     */
		    sws->it_nexus->sync_period = period;
		    sws->it_nexus->sync_offset = offset;
	
                    /*
		     * The SCSI spec. said REQ/ACK offset equal to zero
 		     * implied asynchrous transfer.  Since we set the
		     * period equal to zero if offset is zero in the previous
		     * statement, we can say both have to be zero.
                     */
                    if (period || offset)
                        sws->it_nexus->flags |= SZ_SYNC;

		    SIM_SOFTC_UNLOCK(s, sc);

		    /*
		     * Setup the HBA for the new period and offset.
		     */
		    SC_HBA_SETUP_SYNC(sc, sws);

		    /*
		     * Break out of SCSI_SYNCHRONOUS_XFER case.
		     */
		    break;
		}

		SIM_SOFTC_UNLOCK(s, sc);

		/*
		 * If either are bad put a SZ_MESSAGE_REJECT on the message
		 * out queue and request a message out phase.  The target
		 * is having problems dealing with our required period
		 * and offset.  We will have to run async.
		 */
		CAM_ERROR(module, "Invalid period/offset in SCSI_SYNC message",
			  SIM_LOG_SIM_WS|SIM_LOG_SIM_SOFTC, NULL, sws, NULL);
		SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
		SC_HBA_MSGOUT_PEND(sc, sws);

		/*
		 * Break out of  SCSI_SYNCHRONOUS_XFER case.
		 */
		break;
	    }

	    /*
	     * If SZ_SYNC_NEG was not set, then this is the target
	     * initiating the request.  Make sure that SZ_SYNC_NEEDED.
	     * isn't set.
	     */
	    sws->it_nexus->flags &= ~SZ_SYNC_NEEDED;

	    /*
	     * Make sure that we are allowed to do synchronous.  If not,
	     * reject the message.
	     */
	    if (sws->cam_flags & CAM_DIS_SYNC) {
		/*
		 * If synchronous is not allowed, reject the
		 * message sequence.
		 */
		CAM_ERROR(module, "Sync not allowed, rejecting sync message",
			  SIM_LOG_SIM_WS|SIM_LOG_SIM_SOFTC, NULL, sws, NULL);
		SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
		SC_HBA_MSGOUT_PEND(sc, sws);

		/*
		 * Break out of  SCSI_SYNCHRONOUS_XFER case.
		 */
		SIM_SOFTC_UNLOCK(s, sc);
		break;
	    }
		
	    /*
	     * Store the requested period and offset in the IT_NEXUS and
	     * set the SZ_SYNC_NEG flag.  We must determine an
	     * appropiate period and offset based on the capabilities
	     * of the target and on the capabilities of the HBA.
	     */
	    sws->it_nexus->sync_period = SC_GET_MAX(sc->sync_period, period);
	    sws->it_nexus->sync_offset = SC_GET_MIN(sc->sync_offset, offset);
	    sws->it_nexus->flags |= SZ_SYNC_NEG;
	    SIM_SOFTC_UNLOCK(s, sc);

	    /*
	     * Request message out with a call to
	     * hba_msgout_pend().
	     */
	    SC_HBA_MSGOUT_PEND(sc, sws);

	    break;  /* Break out of SCSI_SYNCHRONOUS_XFER case. */
	}

	case SCSI_MODIFY_DATA_POINTER:

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_EXTENDED_MESSAGE: MODIFY_DATA_POINTER\n"));

	    /*
	     * Currently the modify data pointer message is not implemented.
	     */
	    SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
	    SC_HBA_MSGOUT_PEND(sc, sws);

#ifdef notdef
	    /*
	     * If data segment is setup, then perform modify data pointer
	     * handeling and call DME_MODIFY().
	     */
	    if (sws->data_xfer.segment != (SEGMENT_ELEMENT *)NULL) {
		if (DME_MODIFY(sc, &sws->data_xfer) != CAM_REQ_CMP) {
		    CAM_ERROR(module, "DME_MODIFY failed",
			      SIM_LOG_SIM_WS|SIM_LOG_SIM_SOFTC, NULL, sws,
			      NULL);
		    SC_HBA_MSGOUT_PEND(sc, sws);
		    sws->error_recovery |= ERR_UNKNOWN;
		    SC_SM(sws);
		    return;
		}
	    }
#endif

	    break;  /* Break out of SCSI_MODIFY_DATA_POINTER */

	case SCSI_WIDE_XFER:

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("SCSI_EXTENDED_MESSAGE: SCSI_WIDE_XFER\n"));

	    /*
	     * Determine if the HBA is capable of wide transfers.
	     * If no, reject the message sequence.  The SZ_WIDE_XFER
	     * bit is only set at boot time, so no SMP lock is
	     * needed to check for it.
	     */
	    if (!(sc->flags & SZ_WIDE_XFER)) {

		SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
		       ("message out: 0x%x\n", SCSI_MESSAGE_REJECT));
		SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
		SC_HBA_MSGOUT_PEND(sc, sws);
	    }

	    break;  /* Break out of SCSI_WIDE_XFER */

	default:
	    CAM_ERROR(module, "Unknown extended message",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	    break;
	}

	break;  /* Break out of SCSI_EXTENDED_MESSAGE */

    case SCSI_INITIATE_RECOVERY:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_INITIATE_RECOVERY: 0x%x\n", msg));

	/*
	 * Initiate Recovery.  Set a flag in the SIM_WS.  It is up to
	 * the SIM XPT layer to freeze the NEXUS queue.  It is then
	 * up to the peripheral driver to perform a recovery.
	 * JANET -- Need to implement this!
	 */
	sws->flags |= SZ_INIT_RECOVERY;
	break;
	
    case SCSI_MESSAGE_REJECT:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_MESSAGE_REJECT: 0x%x\n", msg));

	/*
	 * Message Reject.  Receipt of this message means that the
	 * target rejected the last message byte in the msgout queue.
	 * In general, the initiator should never be sending message
	 * bytes that are unsupported by a target, therefore if a
	 * message byte is rejected the current I/O should be failed.
	 * There are several exceptions to this, for example synchronous
	 * data transfer.
	 *
	 * Call sm_msg_rejected().
	 */
	sm_msg_rejected(sws);
	break;

    case SCSI_SIMPLE_QUEUE_TAG:
    {
	SIM_WS *tsws;
	I32 tag = SC_GET_MSGIN(sws, 1);
	int s;

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_SIMPLE_QUEUE_TAG: tag is 0x%x\n", tag));

	/*
	 * Simple Queue Tag.  This sequence will be received during
	 * the reselection process.  A simple queue message consists
	 * of two message bytes, the simple queue message and the
	 * tag.  Both bytes should have already been read in.
	 * Process the tag message to determine the active I/O.
	 * Call sc_find_ws().  Copy the "msginq",
	 * "phaseq", and "phase_sum" from "tmp_ws" to the new SIM_WS
	 * (returned by sc_find_ws()).  Set the "active_io" field in
	 * the SIM_SOFTC to the new SIM_WS.
	 */
	SIM_SOFTC_LOCK(s, sc);
	tsws = sc_find_ws(sc, sws->targid, sws->lun, tag);

	/*
	 * If we are still working with the tmp_ws, then something
	 * went wrong.  Mark the tmp_ws as tagged, pull ATN, and
	 * set the error_recovery field to ERR_PHASE.
	 */
	if (tsws == &sc->tmp_ws) {

	    printf("(sm_msgin) Unable to find SIM_WS, T/L %d/%d, tag 0x%x\n",
		   tsws->targid, tsws->lun, tag);
	    tsws->flags |= SZ_TAGGED;
	    tsws->error_recovery |= ERR_PHASE;
	    SC_HBA_MSGOUT_PEND(sc, tsws);
	}

	/*
	 * If we found a matching SIM_WS, copy all stored information.
	 */
	else if (tsws != sws) {
	    sm_copy_ws(sws, tsws);
	    sws = tsws;
	    sc->active_io = sws;
	}
	SIM_SOFTC_UNLOCK(s, sc);
	break;
    }

    case SCSI_RESTORE_POINTERS:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_RESTORE_POINTERS: 0x%x\n", msg));

	/*
	 * Set the SZ_RDP_NEEDED bit in the SIM_WS's flags field.
	 * before any other data is transferred, a DME_RESTORE()
	 * will be performed.
	 */
	sws->flags |= SZ_RDP_NEEDED;

	/*
	 * Clear the command and status phase summary bits.  This will
	 * restore our command and status pointers to the beginning of
	 * the command.  This will allow the target to perform these
	 * phases, again.
	 */
	sws->phase_sum &=
	    ~(SCSI_PHASEBIT(SCSI_STATUS) | SCSI_PHASEBIT(SCSI_COMMAND));

	break;

    case SCSI_SAVE_DATA_POINTER:

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		   ("SCSI_SAVE_DATA_POINTER: 0x%x\n", msg));

	/*
	 * Save Data Pointer.  The message subsystem will call
	 * DME_SAVE() to instruct the DME to save the active data
	 * pointers.  If the data segment is setup then call 
	 * DME_SAVE().
	 */
	if (sws->data_xfer.segment != (SEGMENT_ELEMENT *)NULL) {
	    if (DME_SAVE(sc, &sws->data_xfer) != CAM_REQ_CMP) {
		/*
		 * An unknown error has occured.
		 */
		CAM_ERROR(module, "DME_SAVE failed",
			  SIM_LOG_SIM_WS|SIM_LOG_SIM_SOFTC|SIM_LOG_PRISEVERE,
			  NULL, sws, NULL);
		SC_HBA_MSGOUT_PEND(sc, sws);
		sws->error_recovery |= ERR_UNKNOWN;
		SC_SM(sws);
		return;
	    }
	}
	break;

    case SCSI_ABORT:
    case SCSI_ABORT_TAG:
    case SCSI_BUS_DEVICE_RESET:
    case SCSI_CLEAR_QUEUE:
    case SCSI_MESSAGE_PARITY_ERROR:
    case SCSI_INITIATOR_DETECTED_ERROR:
    case SCSI_NO_OPERATION:
    case SCSI_IGNORE_WIDE_RESIDUE:
    case SCSI_HEAD_OF_QUEUE_TAG:
    case SCSI_ORDERED_QUEUE_TAG:
    case SCSI_RELEASE_RECOVERY:
    case SCSI_TERMINATE_IO_PROCESS:
    case SCSI_LINKED_COMMAND_COMPLETE:
    case SCSI_LINKED_COMMAND_COMPLETE_WFLAG:
    default:

	CAM_ERROR(module, "Unsupported message in",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);

	/*
	 * Perform the reject message sequence for the following
	 * messages:  SCSI_ABORT, SCSI_ABORT_TAG, SCSI_BUS_DEVICE_RESET,
	 * SCSI_CLEAR_QUEUE, SCSI_MESSAGE_PARITY_ERROR,
	 * SCSI_INITIATOR_DETECTED_ERROR, SCSI_NO_OPERATION,
	 * SCSI_IGNORE_WIDE_RESIDUE, SCSI_HEAD_OF_QUEUE_TAG,
	 * SCSI_ORDERED_QUEUE_TAG, SCSI_RELEASE_RECOVERY,
	 * SCSI_TERMINATE_IO_PROCESS.
	 *
	 * All unknow messages will be rejected.
	 *
	 * The reject message sequence consists of putting the
	 * SCSI_MESSAGE_REJECT message on the message out queue,
	 * calling hba_msgout_pend().
	 */
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGOUT,
		   ("message out: 0x%x\n", SCSI_MESSAGE_REJECT));
	SC_ADD_MSGOUT(sws, SCSI_MESSAGE_REJECT);
	SC_HBA_MSGOUT_PEND(sc, sws);
	break;
    }

    /*
     * The message has now been processed.  Accept it.
     */
    SC_HBA_MSG_ACCEPT(sc, sws);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGIN.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_MSGIN);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_msgout
 *
 * Functional Description :
 *	Sm_msgout() is called via the SCSI state machine.  It is
 *	assumed that the current bus phase is message out.  The
 *	message out queue ("msgoutq") of the active SIM_WS must
 *	already contain the message bytes which need to be sent out.
 *
 * Call Syntax :
 *	sm_msgout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_msgout(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sm_msgout);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Do we need to send a synchronous setup message sequence?
     */
    if (sws->it_nexus->flags & (SZ_SYNC_NEEDED | SZ_SYNC_NEG)) {
	
	/*
	 * If SZ_SYNC_NEG is set, then we are about to send the
	 * final sync message.  Call hba_setup_sync() to set the
	 * HBA for the new period and offset.  We must setup for
	 * the new sync period and offset now since the target
	 * may go directly to data phase after we send this
	 * message.  If the target rejects this request the HBA
	 * will be set back to async.
	 */
	if (sws->it_nexus->flags & SZ_SYNC_NEG)
	    SC_HBA_SETUP_SYNC(sc, sws);

	/*
	 * Generate the sync message.
	 */
	sc_gen_msg(sws, SC_MSG_SYNC);
    }

    /*
     * Determine the number of message out bytes to send by
     * calling SC_GET_MSGOUT_LEN().  If the length is zero, check
     * the previous phase.  If not SCSI_MSGOUT, put SCSI_NO_OPERATION
     * on the message out queue.
     */
    if (SC_GET_MSGOUT_LEN(sws) == 0) {

	if (SC_GET_PREV_PHASE(sws) != SCSI_MSGOUT) {
	    SC_ADD_MSGOUT(sws, SCSI_NO_OPERATION);
	}

	/*
	 * If the target detects one or more parity error(s) on the message
	 * bytes(s) received, it may indicate its desire to retry the
	 * message(s) by asserting the REQ signal after detecting the ATN
	 * signal has gone false and prior to changing to any other phase.
	 * The initiator, upon detecting this condition, shall resend all of
	 * the previous message bytes(s) in the same order as previously sent
	 * during this phase.  When re-sending more than one message byte,
	 * the initiator shall assert the ATN signal at least two diskew
	 * delays prior to asserting the ACK signal on the first byte and
	 * shall maintain the ATN signal asserted until the last byte is sent.
	 */
	else {

	    CAM_ERROR(module, "Resending previous sequence",
		      SIM_LOG_SIM_WS, NULL, sws, NULL);

	    /*
	     * Restore the message out pointer to the previous sequence.
	     */
	    SC_RETRY_MSGOUT(sws);

	    /*
	     * If the number of message out bytes is greater than one,
	     * pull ATN.
	     */
	    if (SC_GET_MSGOUT_LEN(sws) > 1)
		SC_HBA_MSGOUT_PEND(sc, sws);
	}
    }

    /*
     * Call hba_send_msg() to send the message(s).
     */
    SC_HBA_SEND_MSG(sc, sws);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGOUT.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_MSGOUT);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_unexpected
 *
 * Functional Description :
 *	Will handle invalid phase transitions.  Some transitions
 *	which enter this code will not be invalid, but will be
 *	special in some other way.  The special cases will be handled
 *	individually.  All other cases will enter the error recovery
 *	state machine.
 *
 * Call Syntax :
 *	sm_unexpected(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_unexpected(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_unexpected);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Determine the current phase.
     */
    switch(SC_GET_CURR_PHASE(sws)) {

    case SCSI_BUS_FREE:

        if (sws == &((SIM_SOFTC *)sws->sim_sc)->tmp_ws) {
                CAM_ERROR(module, "Unexpected bus free", SIM_LOG_SIM_WS, NULL,
                          sws, NULL);
                break;
        }
	/*
	 * Check to see if a selection timed out.  If this is the
	 * case, the "cam_status" will have been set to CAM_SEL_TIMEOUT.
	 */
	if (sws->cam_status != CAM_SEL_TIMEOUT) {
	
	    /*
	     * Don't log an error if SZ_EXP_BUS_FREE is set.
	     * Don't log an error if SZ_DEVRS_INPROG is set.
	     */
	    if (!(sws->flags & (SZ_EXP_BUS_FREE | SZ_DEVRS_INPROG)))

		CAM_ERROR(module, "Unexpected bus free", SIM_LOG_SIM_WS, NULL,
			  sws, NULL);

	    /*
	     * Clear up any state which may be inconsistant due
	     * to the unexpected bus free.
	     */
	    sws->it_nexus->flags &= ~SZ_SYNC_NEG;

	    /*
	     * Not a selection timeout.  The target may go to bus free
	     * at anytime.  Set the cam_status to CAM_UNEXP_BUSFREE.
	     */
	    sws->cam_status = CAM_UNEXP_BUSFREE;
	}
	else {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		       ("selection timeout\n"));
	}

	/*
	 * Set the command complete flag in the SIM_WS so that
	 * this command will complete.  Also set the SZ_EXP_BUS_FREE bit.
	 */
	sws->flags |= SZ_CMD_CMPLT | SZ_EXP_BUS_FREE;

	/*
	 * Handle the bus free phase.
	 */
	sm_bus_free(sws);

	break;

    default:

	/*
	 * Enter the error recovery state machine.
	 */
	CAM_ERROR(module, "Unexpected phase change", SIM_LOG_SIM_WS, NULL,
		  sws, NULL);
	sws->error_recovery |= ERR_PHASE;
	SC_SM(sws);

	break;
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}                         

/*
 * Routine Name : sm_msg_rejected
 *
 * Functional Description :
 *	This function will be called when a message which the
 *	initiator sent to the target is rejected by the target.
 *
 * Call Syntax :
 *	sm_msg_rejected(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
static void
sm_msg_rejected(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    register u_short start_index, end_index, msg_index, tmp_index;
    u_short bytes_left;
    register int i;
    SIM_MODULE(sm_msg_rejected);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SC_ADD_FUNC(sws, module);

    /*
     * Determine the starting index of the message sequence
     * and the ending index.  The ending index is the index
     * of the byte which was rejected.  This sequence could be
     * the previous sequence if all the bytes of the sequence
     * were sent to the target.  Otherwise, the sequence will
     * still be the current sequence.
     */
    if (sws->msgout_sent) {
	start_index = CIRQ_CURR(sws->msgoutq);
	i = sws->msgout_sent - 1;
    }
    else {
	start_index = CIRQ_PREV(sws->msgoutq);
	i = SC_GET_PREV_MSGOUT_LEN(sws) - 1;
    }
    end_index = start_index;
    for (; i > 0; i--, end_index = CIRQ_INC(sws->msgoutq, end_index));

    /*
     * Determine the index of the "beginning" of the message
     * which was rejected.
     */
    msg_index = sm_last_msgout(sws, start_index, end_index, &bytes_left);

    /*
     * If there are bytes left over in the current message out sequence,
     * remove them.
     */
    if (bytes_left && sws->msgout_sent) {

	SC_UPDATE_MSGOUT(sws, bytes_left);
	/*
	 * ATN will still be asserted at this point.  If there
	 * are no more message out bytes, call the HBA specific
	 * clear ATN function.
	 */
	if (sws->msgout_sent == 0)
	    SC_HBA_MSGOUT_CLEAR(sc, sws);
    }

    /*
     * Check for a message reject on the Identify message.
     */
    if ((sws->msgoutq_buf[msg_index] & SCSI_IDENTIFY) == SCSI_IDENTIFY) {
	/*
         * Identify.  This message is sent to establish the ITL
	 * nexus.  This message is sent anytime a target is selected
	 * with attention. A message reject on this message will
	 * result in an error being written to the error log and the
	 * current I/O will enter the error state machine.
	 */
	sws->error_recovery |= ERR_MSG_REJ;

	CAM_ERROR(module, "SCSI_IDENTIFY message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));
	return;
    }

    /*
     * Determine the message which was rejected.
     */
    switch(sws->msgoutq_buf[msg_index]) {

    case SCSI_ABORT:
	/*
         * Abort.  The message subsystem will send this message in response
	 * to an ABORT CCB and during error recovery procedures.
	 * If this message is rejected, a SCSI_BUS_DEVICE_RESET message
	 * will be sent.
	 */
	CAM_ERROR(module, "SCSI_ABORT message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	SC_ADD_MSGOUT(sws, SCSI_BUS_DEVICE_RESET);
	sws->flags |= SZ_DEVRS_INPROG;
	sws->flags &= ~SZ_ABORT_INPROG;
	SC_HBA_MSGOUT_PEND(sc, sws);
	break;

    case SCSI_ABORT_TAG:
	/*
         * Abort tag.  The message subsystem will send this message in
	 * response to an ABORT CCB or during error recovery procedures.
	 * If this message is rejected, a SCSI_ABORT message will be
	 * sent.
	 */
	CAM_ERROR(module, "SCSI_ABORT_TAG message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	SC_ADD_MSGOUT(sws, SCSI_ABORT);
	sws->flags |= SZ_ABORT_INPROG;
	sws->flags &= ~SZ_ABORT_TAG_INPROG;
	SC_HBA_MSGOUT_PEND(sc, sws);
	break;

    case SCSI_BUS_DEVICE_RESET:
	/*
         * Bus Device Reset.  The message subsystem will send this
	 * message in response to a RESET SCSI DEVICE CCB or during
	 * error recovery procedures.  A message reject on this message
	 * will result in an error being written to the error log and
	 * the SCSI bus being reset.
	 */
	CAM_ERROR(module, "SCSI_BUS_DEVICE_RESET message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	SC_HBA_BUS_RESET(sc);
	break;

    case SCSI_CLEAR_QUEUE:
	/*
         * Clear Queue.  This will not be implemented in the current
	 * CAM driver.  If a NEXUS is operating with tags, then
	 * during error sequences the SIM HBA may need to clear all
	 * pending requests active on a target. A message reject on
	 * this message will result in an error being written to the
	 * error log.  The message subsystem will attempt to send a
	 * bus device reset.
	 */
	CAM_ERROR(module, "SCSI_CLEAR_QUEUE message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	SC_ADD_MSGOUT(sws, SCSI_BUS_DEVICE_RESET);
	sws->flags |= SZ_DEVRS_INPROG;
	SC_HBA_MSGOUT_PEND(sc, sws);
	break;

    case SCSI_DISCONNECT:
	/*
         * Disconnect.  Some SIM's may want to break excessively
	 * large transfers into smaller pieces, use of this message
	 * allows initiators to suggest to a target to disconnect. A
	 * message reject on this message will result in an error
	 * being written to the error log and no other action will be
	 * taken by the SIM.
	 */
	CAM_ERROR(module, "SCSI_DISCONNECT message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	break;

    case SCSI_INITIATE_RECOVERY:
	/*
         * Initiate Recovery.  During asynchronous event notification,
	 * SIM's may issue this command to force a target to initiate
	 * a recovery sequence. A message reject on this message will
	 * result in an error being written to the error log and
	 * current the I/O will enter the error state machine.
	 */
	CAM_ERROR(module, "SCSI_INIT_RECOVERY message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	sws->error_recovery |= ERR_MSG_REJ;
	break;

    case SCSI_INITIATOR_DETECTED_ERROR:
	/*
         * Initiator Detected Error.  This message will be sent if
	 * a parity error is detected on a data in byte or on a 
	 * status byte.  If this message is rejected, then the parity
	 * error will be classified as un-recoverable.
	 */
	CAM_ERROR(module, "SCSI_INIT_DETECTED_ERROR message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	if (sws->recovery_status & (ERR_DATAIN_PE | ERR_STATUS_PE)) {
	    sws->error_recovery |= ERR_PARITY;
	}
	break;

    case SCSI_MESSAGE_PARITY_ERROR:
	/*
         * Message Parity Error.  This message will be sent during error
	 * recovery if a parity error is detected on a message in.  If
	 * this message is rejected, the parity error will be
	 * classified as un-recoverable.
	 */
	CAM_ERROR(module, "SCSI_MESSAGE_PARITY_ERROR message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	if (sws->recovery_status & ERR_MSGIN_PE) {
	    sws->error_recovery |= ERR_PARITY;
	}
	break;

    case SCSI_MESSAGE_REJECT:
	/*
         * Message Reject .  This message is sent by the SIM to
	 * notify a target that the last message received was not
	 * acceptable to the initiator. A message reject on this
	 * message will result in the current I/O being failed.
	 */
	CAM_ERROR(module, "SCSI_MESSAGE_REJECT message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	sws->error_recovery |= ERR_MSG_REJ;
	break;

    case SCSI_NO_OPERATION:
	/*
	 * NOP.  When the message subsystem has no other message to
	 * send, it will send a NOP.  No action will be taken if this
	 * message is rejected.  It served its purpose by getting us
	 * out of message out phase.
	 */
	CAM_ERROR(module, "SCSI_NO_OPERATION message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	break;

    case SCSI_HEAD_OF_QUEUE_TAG:
    case SCSI_ORDERED_QUEUE_TAG:
    case SCSI_SIMPLE_QUEUE_TAG:
	/*
	 * Depending upon the type of command queuing that is specified
	 * by the CCB, the SIM will either send a simple, head of queue,
	 * or ordered queue tag message.  A message reject on this
	 * message will result in the request being aborted.
	 */
	CAM_ERROR(module, "SCSI_QUEUE message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	SC_ADD_MSGOUT(sws, SCSI_ABORT);
	sws->flags |= SZ_ABORT_INPROG;
	sws->error_recovery |= ERR_MSG_REJ;
	SC_HBA_MSGOUT_PEND(sc, sws);
	break;

    case SCSI_RELEASE_RECOVERY:
	/*
         * Release Recovery.  SIM's send this message to targets to
	 * end an extended contingent allegiance. The identify
	 * message must proceed this message for it to take effect.
	 * A message reject on this message will result in an error
	 * being written to the error log and the error state machine
	 * will be entered.
	 */
	CAM_ERROR(module, "SCSI_RELEASE_RECOVERY message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);
	sws->error_recovery |= ERR_MSG_REJ;
	break;

    case SCSI_TERMINATE_IO_PROCESS:
	/*
         * Terminate I/O process.  The message subsystem will send
	 * this message in response to a Terminate I/O CCB.  A message
	 * reject on this message will result in an error being
	 * written to the error log and the SIM will attempt to send
	 * an abort message.
	 */
	CAM_ERROR(module, "SCSI_TERMINATE_IO message was rejected",
		  SIM_LOG_SIM_WS, NULL, sws, NULL);

	/*
	 * If the request was tagged, send a SCSI_ABORT_TAG message,
	 * otherwise, send a SCSI_ABORT message.
	 */
	if (sws->flags & SZ_TAGGED) {
	    SC_ADD_MSGOUT(sws, SCSI_SIMPLE_QUEUE_TAG);
	    SC_ADD_MSGOUT(sws, sws->tag);
	    SC_ADD_MSGOUT(sws, SCSI_ABORT_TAG);
	    sws->flags |= SZ_ABORT_TAG_INPROG;
	}
	else {
	    SC_ADD_MSGOUT(sws, SCSI_ABORT);
	    sws->flags |= SZ_ABORT_INPROG;
	}
	SC_HBA_MSGOUT_PEND(sc, sws);
	break;

    case SCSI_EXTENDED_MESSAGE:

	/*
	 * Determine the message.  Skip the message length.
	 */
	tmp_index = CIRQ_INC(sws->msgoutq, msg_index);
	tmp_index = CIRQ_INC(sws->msgoutq, tmp_index);

	switch(sws->msgoutq_buf[tmp_index]) {

	case SCSI_SYNCHRONOUS_XFER:

	    /*
	     * Synchronous Data Transfer.  SIM's will send this message
	     * to targets to negotiate synchronous data transfer
	     * parameters.  A message reject on this message will result
	     * in an asynchronous setup.  Clear the SZ_SYNC_NEG and
	     * SZ_SYNC flags in the SIM_WS's flags field.  Set the
	     * period and offset for the IT_NEXUS to zero.  Also, make
	     * sure that the HBA is setup with the zero values.
	     */
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
		       ("targ %d reject SCSI_SYNC message\n",
			sws->targid));
	    sws->it_nexus->flags &= ~(SZ_SYNC | SZ_SYNC_NEG);
	    sws->it_nexus->sync_offset = 0;
	    sws->it_nexus->sync_period = 0;
	    SC_HBA_SETUP_SYNC(sc, sws);
	    break;

	case SCSI_WIDE_XFER:
	    /*
	     * Wide Data Transfer Request.  Since wide SCSI is not
	     * supported by any of the existing or planned systems, this
	     * message will not be sent.
	     */
	    break;

	default:
	    CAM_ERROR(module, "Unknown extended message was rejected",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	    break;
	}
	break; /* SCSI_EXTENDED_MESSAGE */

    default:
	CAM_ERROR(module, "Unknown message was rejected",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE, NULL, sws, NULL);
	break;
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_copy_ws
 *
 * Functional Description :
 *	This function will copy the contents of the first SIM_WS
 *	into the second SIM_WS.
 *
 * Call Syntax :
 *	sm_copy_ws(sws1, sws2)
 *
 * Arguments:
 *	SIM_WS *sws1;
 *	SIM_WS *sws2;
 *
 * Return Value :  None
 */
static void
sm_copy_ws(sws1, sws2)
register SIM_WS *sws1, *sws2;
{
    register int i;
    SIM_MODULE(sm_copy_ws);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Copy the phase data...
     */
    SC_ADD_PHASE_BIT(sws2, sws1->phase_sum);
    for (i=1; i < sws1->phaseq.curr; i++)
	SC_NEW_PHASE(sws2, sws1->phaseq_buf[i]);

    /*
     * Copy the message in data...
     */
    for (i=0; i < sws1->msginq.curr; i++)
	SC_ADD_MSGIN(sws2, sws1->msginq_buf[i]);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_last_msgout
 *
 * Functional Description :
 *	This function will determine the index into the message
 *	out queue of the beginning of the last message sequence.
 *
 * Call Syntax :
 *	sm_last_msgout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *	u_short start_index	Index into the message out queue
 *				where the message sequence started.
 *	u_short end_index	Index into the message out queue
 *				where the message sequence ended.
 *	u_short bytes_left	Pointer which will be set to the
 *				number of bytes of the sequence
 *				which haven't been sent out.
 *
 * Return Value :  u_short -- message out index
 */
static u_short
sm_last_msgout(sws, start_index, end_index, bytes_left)
register SIM_WS *sws;
u_short start_index, end_index, *bytes_left;
{
    register u_short index1 = start_index, index2 = start_index;
    u_short tmp_index;
    u_char msg;
    SIM_MODULE(sm_last_msgout);

    *bytes_left = 0;

    /*
     * Stop when we hit the end_index and have computed the
     * length of the message.
     */
    while ((index2 != end_index) || (*bytes_left == 0)) {

	/*
	 * If currently in a message sequence, don't process
	 * the message.
	 */
	if (*bytes_left) {
	    index2 = CIRQ_INC(sws->msgoutq, index2);
	    (*bytes_left)--;
	    continue;
	}

	/*
	 * Determine the "message" which was last sent.  The message
	 * could be multi-byte.
	 */
	index1 = index2;
	msg = sws->msgoutq_buf[index2];
    
	/*
	 * Is this an extended message?
	 */
	if (msg == SCSI_EXTENDED_MESSAGE) {

	    /*
	     * Set tmp_index to the index which contains the
	     * size of the extended message.
	     */
	    tmp_index = CIRQ_INC(sws->msgoutq, index2);

	    /*
	     * Set the sequence size.  Add two for the space
	     * taken by the message size and by the extended
	     * message.
	     */
	    *bytes_left = sws->msgoutq_buf[tmp_index] + 2;
	}
	    
	/*
	 * Is this message a two-byte?
	 */
	else if (SCSI_IS_MSG_TWO_BYTE(msg)) {
	    *bytes_left = 2;
	}

	/*
	 * This was a one byte message.
	 */
	else {
	    *bytes_left = 1;
	}
    }

    /*
     * "bytes_left" will contain the number of bytes in
     * the message, including the rejected byte.  Subtract
     * one.  "bytes_left" will then indicate the number
     * bytes of this message which haven't been sent.
     */
    (*bytes_left)--;

    return(index1);
}

/*
 * The SCSI Initiator State Machine
 */
void
(*scsi_sm[SCSI_NSTATES][SCSI_NSTATES])() = {

    /* SCSI bus free, SCSI_BUS_FREE (0) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
	/* SCSI_ARBITRATION      (1) */   sm_arbitration,
	/* SCSI_SELECTION        (2) */   sm_unexpected,
	/* SCSI_RESELECTION      (3) */   sm_reselection,
	/* SCSI_COMMAND          (4) */   sm_unexpected,
	/* SCSI_DATAIN           (5) */   sm_unexpected,
	/* SCSI_DATAOUT          (6) */   sm_unexpected,
	/* SCSI_STATUS           (7) */   sm_unexpected,
	/* SCSI_MSGIN            (8) */   sm_unexpected,
	/* SCSI_MSGOUT           (9) */   sm_unexpected,
    },  /* end bus free */

    /* SCSI arbitration phase, SCSI_ARBITRATION (1) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
	/* SCSI_ARBITRATION      (1) */   sm_unexpected,
	/* SCSI_SELECTION        (2) */   sm_selection,
	/* SCSI_RESELECTION      (3) */   sm_reselection,
	/* SCSI_COMMAND          (4) */   sm_unexpected,
	/* SCSI_DATAIN           (5) */   sm_unexpected,
	/* SCSI_DATAOUT          (6) */   sm_unexpected,
	/* SCSI_STATUS           (7) */   sm_unexpected,
	/* SCSI_MSGIN            (8) */   sm_unexpected,
	/* SCSI_MSGOUT           (9) */   sm_unexpected,
    },  /* end arbitration phase */

    /* SCSI selection phase, SCSI_SELECTION (2) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
	/* SCSI_ARBITRATION      (1) */   sm_unexpected,
	/* SCSI_SELECTION        (2) */   sm_unexpected,
	/* SCSI_RESELECTION      (3) */   sm_unexpected,
	/* SCSI_COMMAND          (4) */   sm_command,
	/* SCSI_DATAIN           (5) */   sm_unexpected,
	/* SCSI_DATAOUT          (6) */   sm_unexpected,
	/* SCSI_STATUS           (7) */   sm_status,
	/* SCSI_MSGIN            (8) */   sm_msgin,
	/* SCSI_MSGOUT           (9) */   sm_msgout,
    },  /* end selection phase */

    /* SCSI reselection phase, SCSI_RESELECTION (3) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
	/* SCSI_ARBITRATION      (1) */   sm_unexpected,
	/* SCSI_SELECTION        (2) */   sm_unexpected,
	/* SCSI_RESELECTION      (3) */   sm_unexpected,
	/* SCSI_COMMAND          (4) */   sm_unexpected,
	/* SCSI_DATAIN           (5) */   sm_unexpected,
	/* SCSI_DATAOUT          (6) */   sm_unexpected,
	/* SCSI_STATUS           (7) */   sm_unexpected,
	/* SCSI_MSGIN            (8) */   sm_msgin,
	/* SCSI_MSGOUT           (9) */   sm_unexpected,
    },  /* end reselection phase */

    /* SCSI command phase, SCSI_COMMAND (4) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
	/* SCSI_ARBITRATION      (1) */   sm_unexpected,
	/* SCSI_SELECTION        (2) */   sm_unexpected,
	/* SCSI_RESELECTION      (3) */   sm_unexpected,
	/* SCSI_COMMAND          (4) */   sm_unexpected,
	/* SCSI_DATAIN           (5) */   sm_datain,
	/* SCSI_DATAOUT          (6) */   sm_dataout,
	/* SCSI_STATUS           (7) */   sm_status,
	/* SCSI_MSGIN            (8) */   sm_msgin,
	/* SCSI_MSGOUT           (9) */   sm_unexpected,
    },  /* end command phase */

    /* SCSI data in phase, SCSI_DATAIN (5) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
        /* SCSI_ARBITRATION      (1) */   sm_unexpected,
        /* SCSI_SELECTION        (2) */   sm_unexpected,
        /* SCSI_RESELECTION      (3) */   sm_unexpected,
        /* SCSI_COMMAND          (4) */   sm_unexpected,
        /* SCSI_DATAIN           (5) */   sm_datain,
        /* SCSI_DATAOUT          (6) */   sm_unexpected,
        /* SCSI_STATUS           (7) */   sm_status,
        /* SCSI_MSGIN            (8) */   sm_msgin,
        /* SCSI_MSGOUT           (9) */   sm_msgout
    },  /* end data in phase */

    /* SCSI data out phase, SCSI_DATAOUT (6) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
        /* SCSI_ARBITRATION      (1) */   sm_unexpected,
        /* SCSI_SELECTION        (2) */   sm_unexpected,
        /* SCSI_RESELECTION      (3) */   sm_unexpected,
        /* SCSI_COMMAND          (4) */   sm_unexpected,
        /* SCSI_DATAIN           (5) */   sm_unexpected,
        /* SCSI_DATAOUT          (6) */   sm_dataout,
        /* SCSI_STATUS           (7) */   sm_status,
        /* SCSI_MSGIN            (8) */   sm_msgin,
        /* SCSI_MSGOUT           (9) */   sm_msgout,
    },  /* end data out phase */

    /* SCSI status phase, SCSI_STATUS (7) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_unexpected,
        /* SCSI_ARBITRATION      (1) */   sm_unexpected,
        /* SCSI_SELECTION        (2) */   sm_unexpected,
        /* SCSI_RESELECTION      (3) */   sm_unexpected,
        /* SCSI_COMMAND          (4) */   sm_unexpected,
        /* SCSI_DATAIN           (5) */   sm_unexpected,
        /* SCSI_DATAOUT          (6) */   sm_unexpected,
        /* SCSI_STATUS           (7) */   sm_unexpected,
        /* SCSI_MSGIN            (8) */   sm_msgin,
        /* SCSI_MSGOUT           (9) */   sm_msgout
    },	/* end status phase */

    /* SCSI message in phase, SCSI_MSGIN (8) */
    {
	/* SCSI_BUS_FREE         (0) */   sm_bus_free,
        /* SCSI_ARBITRATION      (1) */   sm_unexpected,
        /* SCSI_SELECTION        (2) */   sm_unexpected,
        /* SCSI_RESELECTION      (3) */   sm_unexpected,
        /* SCSI_COMMAND          (4) */   sm_command,
        /* SCSI_DATAIN           (5) */   sm_datain,
        /* SCSI_DATAOUT          (6) */   sm_dataout,
        /* SCSI_STATUS           (7) */   sm_status,
        /* SCSI_MSGIN            (8) */   sm_msgin,
        /* SCSI_MSGOUT           (9) */   sm_msgout
    },	/* end message in phase */

   /* SCSI message out phase, SCSI_MSGOUT (9) */
   {
       /* SCSI_BUS_FREE         (0) */   sm_unexpected,
       /* SCSI_ARBITRATION      (1) */   sm_unexpected,
       /* SCSI_SELECTION        (2) */   sm_unexpected,
       /* SCSI_RESELECTION      (3) */   sm_unexpected,
       /* SCSI_COMMAND          (4) */   sm_command,
       /* SCSI_DATAIN           (5) */   sm_datain,
       /* SCSI_DATAOUT          (6) */   sm_dataout,
       /* SCSI_STATUS           (7) */   sm_status,
       /* SCSI_MSGIN            (8) */   sm_msgin,
       /* SCSI_MSGOUT           (9) */   sm_msgout,
    }	/* end message out phase */
};
