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
static char *rcsid = "@(#)$RCSfile: sim_error.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/12/09 15:06:45 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_error.c		Version 1.09			Jan. 10, 1992 */

/*
	The SIM ERROR source file (sim_error.c) contains functions which
	are specific to the handling of SCSI errors.  It is expected
	that all functions contained in this file will be called with
	IPL high at interrupt time.

Modification History:

	1.09	01/10/92	janet
	Dont log errors if the SIM_WS is being timed out.

	1.08	12/16/91	janet
	Only log the datain and message in errors once per SIM_WS.

	1.07	11/13/91	janet
	Added error logging.

	1.06	10/25/91	janet
	Fixed bug when bus free and abort.

	1.05	10/24/91	janet
	Fixed argument to sim_err_status().  If a selection timeouts
	on an abort, reschedule it.
	
	1.04	10/22/91	janet
	o Added SIM_MODULE() to every function.
	o Replaced all PRINTD's with SIM_PRINTD's.
	o Modified sim_err_bus_free() to check for all types of parity errors.
	o Make sure ATN is asserted in sim_err_command(), sim_err_datain(),
	  sim_err_status(), and in sim_err_msgin().
	o In sim_err_msgin() don't examine the message byte if a parity
	  error occured on it.

	1.03	07/26/91	janet
	If a message reject is received on an abort tag message, an abort will
	be sent.

	1.02	06/04/91	janet
	Added handling of ERR_UNKNOWN and ERR_DATA_RUN.

	1.01	03/26/91	janet
	Updated after code review.

	1.00	01/20/91	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define SZ_INQUIRY 0x12
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
#include <io/cam/scsi_all.h>
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
extern void sm_bus_free();
extern void sim_logger();

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
void sim_err_sm();
void sim_err_bus_free();
void sim_err_arbitration();
void sim_err_selection();
void sim_err_reselection();
void sim_err_command();
void sim_err_datain();
void sim_err_dataout();
void sim_err_status();
void sim_err_msgin();
void sim_err_msgout();
static void (*local_errlog)() = sim_logger; 

/*
 * Routine Name : sim_err_sm
 *
 * Functional Description :
 *	Sim_err_sm() is called by SC_SM() when the target is in an error
 *	recovery state.  It will determine the current phase and call the
 *	appropriate error recover function to handle it.
 *
 * Call Syntax:
 *	sim_err_sm(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_sm(sws)
SIM_WS *sws;
{
    SIM_MODULE(sim_err_sm);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    switch(SC_GET_CURR_PHASE(sws)) {

    case SCSI_BUS_FREE:
	/*
	 * Don't log this error if timing out an I/O.
	 */
	if (!(sws->errors_logged & SZ_ERRLOG_CMDTMO)) {
	    CAM_ERROR(module,
		      "Target went to bus free phase",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	}
	sim_err_bus_free(sws);
	break;

    case SCSI_ARBITRATION:
	CAM_ERROR(module,
		  "Target went to arbitration phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	sim_err_arbitration(sws);
	break;

    case SCSI_SELECTION:
	/*
	 * Don't log this error if timing out an I/O.
	 */
	if (!(sws->errors_logged & SZ_ERRLOG_CMDTMO)) {
	    CAM_ERROR(module,
		      "Target went to selection phase",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	}
	sim_err_selection(sws);
	break;

    case SCSI_RESELECTION:
	CAM_ERROR(module,
		  "Target went to reselection phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	sim_err_reselection(sws);
	break;

    case SCSI_COMMAND:
	CAM_ERROR(module,
		  "Target went to command phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	sim_err_command(sws);
	break;

    case SCSI_DATAIN:
	/*
	 * Only log this error once per SIM_WS.
	 */
	if (!(sws->errors_logged & SZ_ERRLOG_DATAIN)) {
	    sws->errors_logged |= SZ_ERRLOG_DATAIN;
	    CAM_ERROR(module,
		      "Target went to data in phase",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	}
	sim_err_datain(sws);
	break;

    case SCSI_DATAOUT:
	CAM_ERROR(module,
		  "Target went to data out phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	sim_err_dataout(sws);
	break;

    case SCSI_STATUS:
	CAM_ERROR(module,
		  "Target went to status phase",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	sim_err_status(sws);
	break;

    case SCSI_MSGIN:
	/*
	 * Only log this error once per SIM_WS.
	 */
	if (!(sws->errors_logged & SZ_ERRLOG_MSGIN)) {
	    sws->errors_logged |= SZ_ERRLOG_MSGIN;
	    /*
	     * Don't log this if caused by a MSGIN parity error.
	     * In this case, it's expected.
	     */
	    if (!(sws->error_recovery & ERR_MSGIN_PE)) {
		CAM_ERROR(module,
		      "Target went to message in phase",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    }
	}
	sim_err_msgin(sws);
	break;

    case SCSI_MSGOUT:
	/*
	 * Don't log this error if timing out an I/O.
	 */
	if (!(sws->errors_logged & SZ_ERRLOG_CMDTMO)) {
	    /*
	     * Don't log this if caused by a MSGIN parity error.
	     * In this case, it's expected.
	     */
	    if (!(sws->error_recovery & ERR_MSGIN_PE)) {
		CAM_ERROR(module,
		      "Target went to message out phase",
		      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		      NULL, sws, NULL);
	    }
	}
	sim_err_msgout(sws);
	break;

    default:
	break;
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}

/*
 * Routine Name : sim_err_bus_free
 *
 * Functional Description :
 *	Sim_err_bus_free() will handle error recovery when the SCSI
 *	bus has gone free.
 *
 * Call Syntax:
 *	sim_err_bus_free(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_bus_free(sws)
SIM_WS *sws;
{
    int s;
    extern SIM_SM sim_sm;
    SIM_MODULE(sim_err_bus_free);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    ((SIM_SOFTC *)sws->sim_sc)->err_recov_cnt = 0;

    /*
     * Decode the "error_recovery" field and set the appropriate
     * "cam_status."
     */
    if (sws->error_recovery &
	(ERR_PARITY | ERR_MSGIN_PE | ERR_DATAIN_PE | ERR_STATUS_PE)){
	sws->cam_status = CAM_UNCOR_PARITY;
    }
    if (sws->error_recovery & ERR_PHASE) {
	sws->cam_status = CAM_SEQUENCE_FAIL;
    }
    if (sws->error_recovery & ERR_MSG_REJ) {
	sws->cam_status = CAM_MSG_REJECT_REC;
    }
    if (sws->error_recovery & ERR_TIMEOUT) {

	/*
	 * Were we trying to abort this request when the timeout occured?
	 */
	if (sws->flags & (SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG)) {

	    /*
	     * If the selection timed out, reschedule the abort.
	     */
	    if (sws->cam_status == CAM_SEL_TIMEOUT) {
		sws->cam_status = CAM_REQ_INPROG;
		SIM_SOFTC_LOCK(s, sws->sim_sc);
		((SIM_SOFTC *)sws->sim_sc)->abort_pend_cnt++;
		sws->nexus->abort_pend_cnt++;
		sws->flags |= SZ_ABORT_NEEDED;
		sws->flags &= ~SZ_ABORT_INPROG;
		SIM_SOFTC_UNLOCK(s, sws->sim_sc);
		SIM_SM_LOCK(s, &sim_sm);
		sim_sm.waiting_io |= (1 << sws->cntlr);
		SIM_SM_UNLOCK(s, &sim_sm);
		SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
			   ("Timeout on abort selection\n"));
		return;
	    }
	}
	else {
	    sws->cam_status = CAM_CMD_TIMEOUT;
	}
    }
    if (sws->error_recovery & ERR_DATA_RUN) {
	sws->cam_status = CAM_DATA_RUN_ERR;
    }
    if (sws->error_recovery & ERR_UNKNOWN) {
	sws->cam_status = CAM_REQ_CMP_ERR;
    }

    /*
     * Set the command complete flag in the SIM_WS so that
     * this command will complete.
     */
    sws->flags |= SZ_CMD_CMPLT;

    /*
     * Handle the bus free.  Do not call sm_bus_free() if using
     * the temporary working set.
     */
    if (sws != &((SIM_SOFTC *)sws->sim_sc)->tmp_ws) {
	sm_bus_free(sws);
    }
    /*
     * If not calling sm_bus_free() call ss_sched() to start the
     * next request.
     */
    else {
	ss_sched(sws->sim_sc);
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}

/*
 * Routine Name : sim_err_arbitration
 *
 * Functional Description :
 *	Sim_err_arbitration() will handle error recovery when the
 *	target has entered the arbitration phase.
 *
 * Call Syntax:
 *	sim_err_arbitration()
 *
 * Arguments :  None
 *
 * Return Value :  None
 */
void
sim_err_arbitration()
{
    SIM_MODULE(sim_err_arbitration);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("Does nothing\n"));
}

/*
 * Routine Name : sim_err_selection
 *
 * Functional Description :
 *	Sim_err_selection() will handle error recovery when the
 *	target has entered the selection phase.
 *
 * Call Syntax:
 *	sim_err_selection()
 *
 * Arguments :  None
 *
 * Return Value :  None
 */
void
sim_err_selection()
{
    SIM_MODULE(sim_err_selection);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("Does nothing\n"));
}

/*
 * Routine Name : sim_err_reselection
 *
 * Functional Description :
 *	Sim_err_reselection() will handle error recovery when the
 *	target reselects.
 *
 * Call Syntax:
 *	sim_err_reselection()
 *
 * Arguments :  None
 *
 * Return Value :  None
 */
void
sim_err_reselection()
{
    SIM_MODULE(sim_err_reselection);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("Does nothing\n"));
}

/*
 * Routine Name : sim_err_command
 *
 * Functional Description :
 *	Sim_err_command() will handle error recovery when the
 *	target enters the command phase.  The only command that is
 *	supported by all devices and least affects the sense data
 *	is the inquiry command.  Send this command specifying a length
 *	of 36 bytes.  If the target makes it to data in phase, the bytes
 *	will be read and thrown away.
 *
 * Call Syntax:
 *	sim_err_command(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_command(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    u_char cdb[6];
    SIM_MODULE(sim_err_command);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    /*
     * Make sure that ATN has been asserted.
     */
    SC_HBA_MSGOUT_PEND(sc, sws);

    /*
     * Create the Inquiry CDB.
     */
    cdb[0] = SZ_INQUIRY;
    cdb[1] = 0;
    cdb[2] = 0;
    cdb[3] = 0;
    cdb[4] = 0x24;
    cdb[5] = 0;

    /*
     * Transfer the CCB.
     */
    SC_HBA_XFER_INFO(sc, sws, cdb, 6L, (U32) CAM_DIR_OUT); 

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
} 

/*
 * Routine Name : sim_err_datain
 *
 * Functional Description :
 *	Sim_err_datain() will handle error recovery when the
 *	target enters the data in phase.  Data bytes will be
 *	read from the target and thrown away until the target
 *	changes phase.
 *
 * Call Syntax:
 *	sim_err_datain(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_datain(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sim_err_datain);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    /*
     * Make sure that ATN has been asserted.
     */
    SC_HBA_MSGOUT_PEND(sc, sws);

    /*
     * Read the data byte and throw it away.
     */
    SC_HBA_DISCARD_DATA(sc, sws);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_DATAIN.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_DATAIN);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}

/*
 * Routine Name : sim_err_dataout
 *
 * Functional Description :
 *	Sim_err_dataout() will handle error recovery when the
 *	target enters the data out phase.  There is no graceful
 *	error recovery at this point.  All that can be done is
 *	to perform a SCSI bus reset.
 *
 * Call Syntax:
 *	sim_err_dataout(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_dataout(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sim_err_dataout);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	   ("begin, resetting the bus!\n"));

    CAM_ERROR(module,
	      "Resetting the SCSI bus",
	      SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
	      NULL, sws, NULL);

    SC_HBA_BUS_RESET(sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}

/*
 * Routine Name : sim_err_status
 *
 * Functional Description :
 *	Sim_err_status() will handle error recovery when the
 *	target enters the status phase.
 *
 * Call Syntax:
 *	sim_err_status()
 *
 * Arguments :  None
 *
 * Return Value :  None
 */
void
sim_err_status(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sim_err_status);
    
    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("begin\n"));

    /*
     * Make sure that ATN has been asserted.
     */
    SC_HBA_MSGOUT_PEND(sc, sws);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT, ("end\n"));
}

/*
 * Routine Name : sim_err_msgin
 *
 * Functional Description :
 *	Sim_err_msgin() will handle error recovery when the
 *	target enters the message in phase.  It is assumed that
 *	if a parity error occurred on a message in byte that the
 *	ERR_MSGIN_PE bit will have already been set in the SIM_WS's
 *	"error_recovery" field.  This bit won't be used by
 *	sim_err_msgin() but will be used by sim_err_msgout().
 *
 * Call Syntax:
 *	sim_err_msgin(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_msgin(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sim_err_msgin);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    /*
     * Make sure that ATN has been asserted.
     */
    SC_HBA_MSGOUT_PEND(sc, sws);

    /*
     * If a parity error occured on the message byte, dont try
     * to determine what it was.
     */
    if (!(sws->error_recovery & (ERR_PARITY | ERR_MSGIN_PE))) {

	/*
	 * Check to see if the message received was a SCSI_MESSAGE_REJECT.
	 * If it was, check to see if the message rejected was a
	 * SCSI_ABORT or SCSI_BUS_DEVICE_RESET message.
	 */
	if (SC_GET_MSGIN(sws, 0) == SCSI_MESSAGE_REJECT) {

	    switch(SC_GET_PREV_MSGOUT(sws, 0)) {

	    case SCSI_ABORT_TAG:
		/*
		 * If a SCSI_ABORT_TAG message was rejected, set the
		 * ERR_ABORT_TAG_REJECTED bit in the "error_recovery" field
		 * and request a message out phase.  This will force the
		 * sending of a SCSI_ABORT message.
		 */
		sws->error_recovery |= ERR_ABORT_TAG_REJECTED;
		SC_HBA_MSGOUT_PEND(sc, sws);
		break;

	    case SCSI_ABORT:
		/*
		 * If a SCSI_ABORT message was rejected, set the
		 * ERR_ABORT_REJECTED bit in the "error_recovery" field
		 * and request a message out phase.  This will force the
		 * sending of a SCSI_BUS_DEVICE_RESET message.
		 */
		sws->error_recovery |= ERR_ABORT_REJECTED;
		SC_HBA_MSGOUT_PEND(sc, sws);
		break;

	    case SCSI_BUS_DEVICE_RESET:
		/*
		 * If a SCSI_BUS_DEVICE_RESET_MESSAGE is rejected, we will have
		 * to perform a SCSI BUS reset.
		 */
		SC_HBA_BUS_RESET(sc);
		break;

	    default:
		break;
	    }
	}
    }

    /*
     * Accept the message.
     */
    SC_HBA_MSG_ACCEPT(sc, sws);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGIN.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_MSGIN);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}

/*
 * Routine Name : sim_err_msgout
 *
 * Functional Description :
 *	Sim_err_msgout() will handle error recovery when the
 *	target enters the message out phase.
 *
 * Call Syntax:
 *	sim_err_msgin(sws)
 *
 * Arguments:
 *	SIM_WS *sws	SIM_WS pointer for request which is in an error 
 *			state
 *
 * Return Value :  None
 */
void
sim_err_msgout(sws)
SIM_WS *sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(sim_err_msgout);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    /*
     * If the ERR_MSGIN_PE bit is set, send a SCSI_MESSAGE_PARITY_ERROR
     * message.
     */
    if (sws->error_recovery & ERR_MSGIN_PE) {

	SC_UPDATE_MSGIN(sws, 0);	/* we'll get them all again anyway */
	SC_ADD_MSGOUT(sws, SCSI_MESSAGE_PARITY_ERROR);

	/*
	 * This is a recoverable parity error.  Clear the ERR_MSGIN_PE
	 * bit in the "error_recovery" field.
	 */
	sws->error_recovery &= ~ERR_MSGIN_PE;
	sws->recovery_status = ERR_MSGIN_PE;
    }

    /*
     * If a parity error occurred on a status or data in byte, send
     * an initiator detected error message.
     */
    else if (sws->error_recovery & (ERR_DATAIN_PE | ERR_STATUS_PE)) {
	SC_ADD_MSGOUT(sws, SCSI_INITIATOR_DETECTED_ERROR);
	sws->recovery_status =
	    sws->error_recovery & (ERR_DATAIN_PE | ERR_STATUS_PE);
	sws->error_recovery &= ~sws->recovery_status;
    }

    /*
     * If any other error, send a SCSI_ABORT or SCSI_ABORT_TAG message.
     * If a SCSI_ABORT_TAG has already been sent and the device rejected
     * it, send a SCSI_ABORT message.
     * If a SCSI_ABORT has already been sent and the device rejected it,
     * send a SCSI_BUS_DEVICE_RESET message.
     *
     * Note that we don't add another abort to the queue if there is
     * already one there waiting to go out.
     */
    else {
	if (sws->error_recovery & ERR_ABORT_REJECTED) {
	    sws->error_recovery &= ~ERR_ABORT_REJECTED;
	    SC_ADD_MSGOUT(sws, SCSI_BUS_DEVICE_RESET);
	}
	else if (sws->error_recovery & ERR_ABORT_TAG_REJECTED) {
	    sws->error_recovery &= ~ERR_ABORT_TAG_REJECTED;
	    SC_ADD_MSGOUT(sws, SCSI_ABORT);
	}
	else if (!(sws->flags & (SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG))) {
	    if (sws->flags & SZ_TAGGED) {
		SC_ADD_MSGOUT(sws, SCSI_SIMPLE_QUEUE_TAG);
		SC_ADD_MSGOUT(sws, sws->tag);
		SC_ADD_MSGOUT(sws, SCSI_ABORT_TAG);
	    }
	    else {
		SC_ADD_MSGOUT(sws, SCSI_ABORT);
	    }
	}
    }

    /*
     * Call hba_send_msg() to send the message.
     */
    SC_HBA_SEND_MSG(sc, sws);

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGOUT.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_MSGOUT);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}
