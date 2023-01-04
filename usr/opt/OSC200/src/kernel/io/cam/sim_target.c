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
static char *rcsid = "@(#)$RCSfile: sim_target.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/12/09 20:57:29 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_target.c		Version 1.01			Nov 13, 1991

	The SIM Target source file contains functions needed by
	the SIM to enable and operate in CAM processor mode.

Modification History:

	1.01	11/13/91	janet
	Removed all SC_ADD_FUNC().  Added error logging.

	1.00	06/12/91	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define CAMERRLOG
#define TARG_CMD_TO	(4 * hz)

/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_processor.h>
#include <io/cam/cam_logger.h>
#include <io/cam/cam_debug.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
#include <io/cam/cam_errlog.h>

/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern void	ss_finish();
extern U32	ss_sched();
extern void	sim_logger();
extern SIM_SOFTC *softc_directory[];
extern int	hz;

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
void		sm_targ_bus_free();
void		sm_targ_arbitration();
void		sm_targ_selection();
void		sm_targ_reselection();
void		sm_targ_datain();
void		sm_targ_dataout();
void		sm_targ_status();
void		sm_targ_msgin();
void		sm_targ_msgout();
void		sm_targ_unexpected();
void		targ_sm();
void		targ_release_default();
void 		sm_copy_targws();
U32		targ_setup_default();
U32		targ_find_sws();
SIM_WS *	sm_targ_command();
void		targ_cmd_timeout();
static U32	targ_setup_default_inquiry();
static U32	targ_setup_default_reqsns();
static U32	targ_setup_default_tur();
static void	sm_targ_msg_rejected();
static void	targ_default_comp();
static void	targ_release_default_ccb();
U32		targ_find_avail_sws();
U32		targ_validate_cdb();

static void (*local_errlog)() = sim_logger; 

/* ---------------------------------------------------------------------- */

/*
 * Routine Name :  sm_targ_bus_free
 *
 * Functional Description :
 *      This function is called from sim94_sm() when a disconnect interrupt
 *      is received while running in target mode.
 *	If this is not the temp working set, sm_targ_bus_free() calls 
 *	sx_command_complete() to do the Pdrv callback. If the lun is being 
 *	disabled then a wakeup is issued to indicate that the ccb is now
 *	available to be disabled. The ss_sched() routine is always called to 
 *	start another I/O on the nexus if one exists.
 *
 * Call Syntax :
 *	sm_targ_bus_free(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_bus_free(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    int s;
    SIM_MODULE(sm_targ_bus_free);
    
    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    SIM_SOFTC_LOCK(s, sc);

    /*
     * If this is the temp working set, don't do anything since there 
     * is no-one to callback.
     */
    if (sws != &sc->tmp_ws) {
	untimeout(targ_cmd_timeout, sws);
	(void) DME_END(sc, &sws->data_xfer);
	
	/*
	 * Set status of this I/O to completed.
	 */
	sws->cam_status = CAM_REQ_CMP;

	/*
	 * If there is a disable of this lun pending, then wakeup the thread
	 * waiting for any active target I/O on this nexus to complete. The
	 * waiter will then clear this I/O from the nexus and thereby 
	 * disable this command on this lun.
	 */
	if (sws->nexus->flags & SZ_TARG_DISABLE_LUN) {
	    sws->ccb->cam_ch.cam_flags |= CAM_TGT_CCB_AVAIL;
	    wakeup(sws->ccb);
	} else {
	    /*
	     * Do callback for this CCB
	     */
	    sx_command_complete(sws);
	}
    }

    /*
     * Clear active I/O since this I/O has now completed.
     */
    sc->active_io = NULL;

    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * If we were busy with a target operation when an I/O was queued to this
     * bus, then we must schedule any of these pending I/O's to run.
     */
    ss_sched(sc); 

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("end\n"));
}	 

/*
 * Routine Name : sm_targ_arbitration
 *
 * Functional Description :
 *	Handle the SCSI arbitration phase - does nothing since currently 
 *	not called.
 *
 * Call Syntax :
 *	sm_targ_arbitration(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_arbitration(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_arbitration);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_ARBITRATION.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_ARBITRATION);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : sm_targ_selection
 *
 * Functional Description :
 *	Handle the SCSI selection phase - does nothing since currently not 
 *	called.
 *
 * Call Syntax :
 *	sm_targ_selection(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_selection(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_selection);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_SELECTION.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_SELECTION);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : sm_targ_reselection
 *
 * Functional Description :
 *	Handle a reselection - does nothing since currently not called.
 *
 * Call Syntax :
 *	sm_targ_reselection(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_reselection(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_reselection);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_RESELECTION.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_RESELECTION);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : targ_cmd_timeout
 *
 * Functional Description :
 *	Handle a target mode command timeout.
 *	Perform any necessary cleanup and re-enable the ccb for use.
 *
 * Call Syntax :
 *	sm_targ_reselection(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
targ_cmd_timeout(sws)
SIM_WS *sws;
{
	SIM_SOFTC *sc;
	int s;

	if( (sws->targ_ws.flags & TARG_GOT_CMD) &&
	     !(sws->targ_ws.flags & TARG_CMDCPLT) ) {
		sc = sws->sim_sc;
    		SIM_SOFTC_LOCK(s, sc);

		sws->targ_ws.flags &= ~TARG_GOT_CMD;

		(void) DME_END(sc, &sws->data_xfer);

	    	sws->ccb->cam_ch.cam_flags |= CAM_TGT_CCB_AVAIL;
		/*
		 * If there is a disable of this lun pending, then wakeup 
		 * the thread waiting for any active target I/O on this 
		 * nexus to complete. 
	 	 */
		if (sws->nexus->flags & SZ_TARG_DISABLE_LUN) {
	    		wakeup(sws->ccb);
		}
    		sc->active_io = NULL;
    		SIM_SOFTC_UNLOCK(s, sc);
		/* dgdfix -- STERLING BL8 build fix -- 
				to be checked by Maria 
		sim94_bus_reset(sc);
		*/
		SC_HBA_BUS_RESET(sc);
    		ss_sched(sc); 
	} 
}
/*
 * Routine Name : sm_targ_command
 *
 * Functional Description :
 *	sm_targ_command() will process the SCSI command based on
 *	the command byte in the CDB.
 *	This function is called from sim94_targ_sm() after a CDB has
 *	been received.
 *
 * Call Syntax :
 *	sm_targ_command(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
SIM_WS * 
sm_targ_command(sws)
SIM_WS * sws;
{
    SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    U32 nbytes;
    SIM_WS *tsws;
    U32 targid, targlun;
    int s;

    SIM_MODULE(sm_targ_command);
    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT, ("begin\n"));

    /*
     * Has a valid command been read in?  If not, the command length will 
     * be zero. Set the SCSI status to CHECK CONDITION and indicate the 
     * command is complete.
     */
    if (sws->targ_ws.command_len == 0) {
	sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
	sws->targ_ws.flags |= TARG_CMDCPLT;
        SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        return(sws);
    }

    /*
     * Are we using the temporary SIM_WS?  If so, find the true working set.
     */
    if (sws == &sc->tmp_ws) {
	/*
	 * Check the flags on the NEXUS. If neither SZ_TARG_DEF or
	 * SZ_PROCESSOR are set, call targ_setup_default() to setup a 
	 * set of default CCB's to allow target operations to be executed
	 * before a peripheral driver specifically enables a target lun.
	 */
	if( !(sc->nexus[sc->scsiid][sws->lun].flags 
	     & (SZ_TARG_DEF | SZ_PROCESSOR))) {

	    /*
	     * If unable to setup defaults, end this command with
	     * a status of check condition.
	     */
	    if (targ_setup_default(sws->cntlr, sc->scsiid, sws->lun) !=
		CAM_REQ_CMP) {
		sws->scsi_status = SCSI_STAT_BUSY;
		sws->targ_ws.flags |= TARG_CMDCPLT;
		SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
		return(sws);
	    }
	}

	/*
	 * Find the SIM_WS for the CDB.
	 */
	switch(targ_find_avail_sws(sc, &sc->nexus[sc->scsiid][sws->lun],
			    sws->targ_ws.command[0],
			    (U32)sws->targ_ws.command_len, &tsws)) {
	case CAM_REQ_CMP:
	    /* 
	     * Save the senders target id and LUN.
	     */
    	    targid = sws->targid;
    	    targlun = sws->lun;
	    sm_copy_targws(sws, tsws);
	    sws = tsws;

	    SIM_SOFTC_LOCK(s, sc);
	    sc->active_io = sws;
	    SIM_SOFTC_UNLOCK(s, sc);

            /*
             * Indicate who the request came from in the CCB if this lun was 
	     * enabled by a processor driver.
             */
            if (sws->nexus->flags & SZ_PROCESSOR) {
    	    	sws->ccb->cam_iorsvd0[INITIATOR_BUS] = sc->cntlr;
    	    	sws->ccb->cam_iorsvd0[INITIATOR_TARGET] = targid;
    	    	sws->ccb->cam_iorsvd0[INITIATOR_LUN] = targlun;
    	    }
	    /*
	     * Setup a timeout for this command.
	     */
	    timeout(targ_cmd_timeout, sws, TARG_CMD_TO);
	    break;

	case CAM_PROVIDE_FAIL:
	    /*
	     * Here if we could not find the command in our list of 
	     * enabled ccbs or default ccbs.
	     */
	    sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
	    sws->targ_ws.flags |= TARG_CMDCPLT;
	    SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
	    return(sws);

	case CAM_BUSY:
	    /*
	     * Here if the command is enabled but currently being processsed
	     * by the processor driver so indicate a BUSY SCSI status.
	     */
	    sws->scsi_status = SCSI_STAT_BUSY;
	    sws->targ_ws.flags |= TARG_CMDCPLT;
	    SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
	    return(sws);

	default:
	    /*
	     * This should never happen since there is no other return status
	     * from targ_find_avail_sws().
	     */
	    break;

	}  /* switch */

    } /* if (sws == &sc->tmp_ws) */

    /*
     * Retrieve the number of bytes to transfer for the associated data phase
     * from the CDB reecived. The actual number of bytes will be the minimum 
     * of the number specified in the CDB or the number specified in the CCB.
     */
    switch (sws->targ_ws.command[0]) 
    {
    case ALL_CHANGE_DEF10_OP: {

	ALL_CHANGE_DEF_CDB10 *change_def_cdb = (ALL_CHANGE_DEF_CDB10 *)
	    sws->targ_ws.command;
        /*
         * Has the entire change definition command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_CHANGE_DEF_CDB10)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}

	nbytes = change_def_cdb->param_ln;
	break;
    }

    case ALL_COMPARE10_OP: {
	ALL_COMPARE_CDB10 *compare_cdb = (ALL_COMPARE_CDB10 *)
	    sws->targ_ws.command;
        /*
         * Has the entire compare command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_COMPARE_CDB10)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (compare_cdb->param_len2 << 16) |
	         (compare_cdb->param_len1 << 8) |
	         (compare_cdb->param_len0);
	break;
    }

    case ALL_COPY_OP: {
	ALL_COPY_CDB *copy_cdb = (ALL_COPY_CDB *)sws->targ_ws.command;
        /*
         * Has the entire copy command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_COPY_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (copy_cdb->param_len2 << 16) |
	         (copy_cdb->param_len1 << 8) |
	         (copy_cdb->param_len0);
	break;
    }

    case ALL_COPY_AND_VERIFY10_OP: {
	ALL_COPY_AND_VERIFY_CDB10 *copy_and_verify = 
	    (ALL_COPY_AND_VERIFY_CDB10 *)sws->targ_ws.command;
        /*
         * Has the entire copy and verify command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_COPY_AND_VERIFY_CDB10)) {
            return;
        }
	nbytes = (copy_and_verify->param_len2 << 16) |
	         (copy_and_verify->param_len1 << 8) |
	         (copy_and_verify->param_len0);
	break;
    }

    case ALL_INQ_OP: {
	ALL_INQ_CDB *inquiry_cdb = (ALL_INQ_CDB *)sws->targ_ws.command;

        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_INQ_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = inquiry_cdb->alloc_len;
	break;
    }

    case ALL_LOG_SEL_OP: {
	ALL_LOG_SEL_CDB10 *log_select =
	    (ALL_LOG_SEL_CDB10 *)sws->targ_ws.command;

        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_LOG_SEL_CDB10)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (log_select->param_len1 << 8) | (log_select->param_len0);
	break;
    }

    case ALL_LOG_SNS_OP: {
	ALL_LOG_SNS_CDB10 *log_sense =
	    (ALL_LOG_SNS_CDB10 *)sws->targ_ws.command;

        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_LOG_SNS_CDB10)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (log_sense->alloc_len1 << 8) | (log_sense->alloc_len0);
	break;
    }

    case ALL_READ_BUFFER_OP: {
	ALL_READ_BUFFER_CDB *read_buffer =
	    (ALL_READ_BUFFER_CDB *)sws->targ_ws.command;

        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_READ_BUFFER_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (read_buffer->alloc_len2 << 16) |
	         (read_buffer->alloc_len1 << 8) |
	         (read_buffer->alloc_len0);
	break;
    }

    case PROC_RECEIVE_OP: {
	PROC_RECEIVE_CDB *receive =
	    (PROC_RECEIVE_CDB *)sws->targ_ws.command;
        /*
         * Has the entire receive command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(PROC_RECEIVE_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (receive->alloc_len2 << 16) |
	         (receive->alloc_len1 << 8) |
	         (receive->alloc_len0);
	break;
    }

    case ALL_RECEIVE_DIAGNOSTIC_OP: {
	ALL_RECEIVE_DIAGNOSTIC_CDB *recv_diag =
	    (ALL_RECEIVE_DIAGNOSTIC_CDB *)sws->targ_ws.command;

        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_RECEIVE_DIAGNOSTIC_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (recv_diag->alloc_len1 << 8) | (recv_diag->alloc_len0);
	break;
    }

    case ALL_REQ_SENSE6_OP: {
	ALL_REQ_SENSE_CDB6 *request_sense =
	    (ALL_REQ_SENSE_CDB6 *)sws->targ_ws.command;

        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_REQ_SENSE_CDB6)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = request_sense->alloc_len;
	break;
    }

    case PROC_SEND_OP: {
	PROC_SEND_CDB *send =
	    (PROC_SEND_CDB *)sws->targ_ws.command;
        /*
         * Has the entire send command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(PROC_SEND_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (send->alloc_len2 << 16) |
	         (send->alloc_len1 << 8) |
	         (send->alloc_len0);
	break;
    }

    case ALL_SEND_DIAGNOSTIC_OP: {
	ALL_SEND_DIAGNOSTIC_CDB *send_diag =
	    (ALL_SEND_DIAGNOSTIC_CDB *)sws->targ_ws.command;

        /*
         * Has the entire send diagnostic command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_SEND_DIAGNOSTIC_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (send_diag->param_len1 << 8) | (send_diag->param_len0);
	break;
    }

    case ALL_TUR_OP: {
        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_TUR_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}

	/*
	 * Since there is'nt any data transfered with a TUR, set the
	 * TARG_CMDCPLT bit.
	 */
	sws->targ_ws.flags |= TARG_CMDCPLT;
	nbytes = 0;
	break;
    }

    case ALL_WRITE_BUFFER_OP: {
	ALL_WRITE_BUFFER_CDB *write_buffer =
	    (ALL_WRITE_BUFFER_CDB *)sws->targ_ws.command;
        /*
         * Has the entire inquiry command been read in yet?
         */
        if (sws->targ_ws.command_len != sizeof(ALL_WRITE_BUFFER_CDB)) {
		sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
		sws->targ_ws.flags |= TARG_CMDCPLT;
        	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        	return(sws);
	}
	nbytes = (write_buffer->param_len2 << 16) |
	         (write_buffer->param_len1 << 8) |
	         (write_buffer->param_len0);
	break;
    }

    default:

	/*
	 * Unsupported CDB.  Log a message.
	 */
	nbytes = 0;
	CAM_ERROR(module,
		  "Unsupported CDB",
		  SIM_LOG_SIM_WS | SIM_LOG_HBA_SOFTC|SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
	/*
	 * This was an unsupported command so return a status of CHECK
	 * CONDITION.
	 */
	sws->scsi_status = SCSI_STAT_CHECK_CONDITION;
	sws->targ_ws.flags |= TARG_CMDCPLT;
        SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
        return(sws);
    }

    /*
     * If we have no bytes to transfer, then we are all done.  No point
     * trying to do a transfer if there aren't any bytes to transfer.
     */
    if(nbytes == 0) {
	sws->scsi_status = SCSI_STAT_GOOD;
	sws->targ_ws.flags |= TARG_CMDCPLT;
	SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
	return(sws);
    }

    /*
     * Set the HBA with the appropriate sync values.
     */
    SC_HBA_SETUP_SYNC(sc, sws);

    /*
     * Setup the data transfer.  Transfer the minimum of the number
     * of bytes requested or the number of bytes provided in the CCB.
     */
    if(sws->data_xfer.segment == NULL) {
        if (DME_SETUP(sc, sws,
	 	SC_GET_MIN(nbytes, sws->ccb->cam_dxfer_len),
		(u_char *) sws->ccb->cam_data_ptr,
		sws->cam_flags, &sws->data_xfer) != CAM_REQ_CMP) {
		    CAM_ERROR(module, "Unable to setup for data transfer",
		  	  SIM_LOG_SIM_WS, NULL, sws, NULL);
		    /*
		     * Lets return a status of busy.
		     */
	            sws->scsi_status = SCSI_STAT_BUSY;
	            sws->targ_ws.flags |= TARG_CMDCPLT;
	}
    }
	    
    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_COMMAND.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(sws);
}

/*
 * Routine Name : sm_targ_datain
 *
 * Functional Description :
 *	Handle target data in phase - does nothing since currently not called.
 *
 * Call Syntax :
 *	sm_targ_datain(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_datain(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_datain);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_DATAIN.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_DATAIN);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : sm_targ_dataout
 * 
 * Functional Description :
 *	Handle target data out phase - does nothing since currently not called.
 *
 * Call Syntax :
 *	sm_targ_dataout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_dataout(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_dataout);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_DATAOUT.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_DATAOUT);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : sm_targ_status
 *
 * Functional Description :
 *	Sm_targ_status() does nothing since currently not called.
 *
 * Call Syntax :
 *	sm_targ_status(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_status(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_status);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_STATUS.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_STATUS);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : sm_targ_msgin
 *
 * Functional Description :
 *	Handle MSGIN phase - does nothing since currently not called.
 *
 * Call Syntax :
 *	sm_targ_msgin(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_msgin(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_msgin);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGIN.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_MSGIN);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : sm_targ_msgout
 *
 * Functional Description :
 *	Sm_targ_msgout() is called via the HBA target state machine.
 *	It is assumed that the current bus phase is message out, that
 *	the message has been read from the SCSI bus, and that the
 *	message has been put in the message out queue.
 *
 *	Currently only the identify message is supported.  If any
 *	other messages are received they will be rejected.
 *
 * Call Syntax :
 *	sm_targ_msgout(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_msgout(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    u_char msg;
    SIM_MODULE(sm_targ_msgout);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Get the message from the message out queue and update
     * the queue.
     */
    msg = SC_GET_MSGOUT(sws, 0);
    SC_UPDATE_MSGOUT(sws, 1);

    /*
     * Process the identify message first.
     */
    if ((msg & SCSI_IDENTIFY) == SCSI_IDENTIFY) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_MSGIN,
	       ("SCSI_IDENTIFY: 0x%x\n", msg));

	/*
	 * Read the LUN from the identify message.
	 */
	sws->lun = SC_DECODE_IDENTIFY(msg);

	/*
	 * Update the NEXUS pointer for this SIM_WS.  Make
	 * sure that the SIM_WS is the temp working set first.
	 */
	if (sws == &sc->tmp_ws) {
	    sws->nexus = SC_GET_NEXUS(sc, sws->targid, sws->lun);
	}

	/*
	 * Are disconnects allowed?
	 */
	sws->targ_ws.disconnect = SC_GET_IDENTIFY_DISC(msg);

	/*
	 * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGOUT.
	 */
	SC_ADD_PHASE_BIT(sws, SCSI_MSGOUT);

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));

	return;
    }

    /*
     * Process the message.
     */
    switch(msg) {

    case SCSI_NO_OPERATION:			/* Mandatory */
        break;

    case SCSI_ABORT:				/* Mandatory */
    case SCSI_ABORT_TAG:
    case SCSI_BUS_DEVICE_RESET:			/* Mandatory */
    case SCSI_CLEAR_QUEUE:
    case SCSI_INITIATOR_DETECTED_ERROR:		/* Mandatory */
    case SCSI_MESSAGE_PARITY_ERROR:		/* Mandatory */
    case SCSI_MESSAGE_REJECT:			/* Mandatory */
    case SCSI_HEAD_OF_QUEUE_TAG:
    case SCSI_ORDERED_QUEUE_TAG:
    case SCSI_SIMPLE_QUEUE_TAG:
    case SCSI_EXTENDED_MESSAGE:
    case SCSI_TERMINATE_IO_PROCESS:
    default:
	SC_ADD_MSGIN(sws, SCSI_MESSAGE_REJECT);
	break;
    }

    /*
     * Call SC_ADD_PHASE_BIT() with a phase of SCSI_MSGOUT.
     */
    SC_ADD_PHASE_BIT(sws, SCSI_MSGOUT);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : sm_targ_unexpected
 *
 * Functional Description:
 * 	Does nothing since never called.
 *
 * Call Syntax :
 *	sm_targ_unexpected(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
sm_targ_unexpected(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_unexpected);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}                         

/*
 * Routine Name : sm_targ_msg_rejected
 *
 * Functional Description:
 *	Does nothing since never called.
 *
 * Call Syntax :
 *	sm_targ_msg_rejected(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
static void
sm_targ_msg_rejected(sws)
register SIM_WS *sws;
{
    SIM_MODULE(sm_targ_msg_rejected);

    SC_ADD_FUNC(sws, module);
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("begin\n"));
    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,("end\n"));
}

/*
 * Routine Name : targ_sm
 *
 * Functional Description :
 * 	Targ_sm() is called by the HBA specific target state machine.
 *	Targ_sm() is responsible for starting the next action based
 *	on what has occured so far in the connection.
 *
 * Call Syntax :
 *	targ_sm(sws)
 *
 * Arguments:
 *	SIM_WS *sws;
 *
 * Return Value :  None
 */
void
targ_sm(sws)
SIM_WS *sws;
{
    SIM_WS *new_sws;
    SIM_SOFTC *sc = sws->sim_sc;
    SIM_TARG_WS *tws = &sws->targ_ws;
    SIM_MODULE(targ_sm);

    SC_ADD_FUNC(sws, module);

    /*
     * Do we have messages to send? (message in phase)
     */
    if (SC_GET_MSGIN_LEN(sws)) {
	(sc->hba_targ_send_msg)(sws);
    }

    /*
     * Does the initiator want to send us messages? (message out phase)
     */
    else if (tws->flags & TARG_ATNSET) {
	(sc->hba_targ_recv_msg)(sws);
    }

    /*
     * If we haven't received the command yet, get it. (command phase)
     */
    else if (!(tws->flags & TARG_GOT_CMD)) {
	(sc->hba_targ_recv_cmd)(sws);
    }

    /*
     * If we have completed the command, finish up. (status phase and bus free)
     */
    else if (tws->flags & TARG_CMDCPLT) {
	(sc->hba_targ_cmd_cmplt)(sws);
    }

    /*
     * Continue a data transfer. (data phase)
     */
    else if (!(sws->data_xfer.flags & DME_DONE)) {

/* Todo, Must check status of RESUME !!! */
    	/*
     	 * Find the SIM_WS for the CDB.
     	 */
     	targ_find_sws(sc, &sc->nexus[sc->scsiid][sws->lun],
		     sws->targ_ws.command[0],
		     (U32)sws->targ_ws.command_len, &new_sws);
	
    	if(new_sws != (SIM_WS *)NULL) {
	   	sws = new_sws;
    	}

	DME_RESUME(sc, &sws->data_xfer);
    }
}

/*
 * Routine Name : targ_setup_default
 *
 * Functional Description :
 * 	Targ_setup_default() will setup the required default CCB's to be used 
 *	for CAM processor mode.  This function 	will only be called if the 
 *	CPU has been selected and an enable LUN CCB has not been recieved.  
 *	CCB's for inquiry, request sense, and test unit ready are created.
 *
 * Call Syntax :
 *	targ_setup_default(b, t, l)
 *
 * Arguments:
 *	U32 b	- Specifies the bus
 *	U32 t	- Specifies the target
 *	U32 l	- Specifies the lun
 *
 * Return Value :  
 *	CAM_REQ_CMP		The default CCB's were successfully setup.
 *	CAM_PROVIDE_FAIL	Unable to setup.  No CCB's will be setup.
 */
U32
targ_setup_default(b, t, l)
U32 b, t, l;
{
    SIM_SOFTC *sc = SC_GET_SOFTC(b);
    NEXUS *nexus = SC_GET_NEXUS(sc, t, l);
    int s;

    /*
     * Set the SZ_TARG_DEF bit in the NEXUS flag field to indicate we have
     * setup the default CCBs.
     */
    SIM_SOFTC_LOCK(s, sc);
    nexus->flags |= SZ_TARG_DEF;
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Add the default inquiry CCB.
     */
    if (targ_setup_default_inquiry(b, t, l) != CAM_REQ_CMP)
	return(CAM_PROVIDE_FAIL);

    /*
     * Add the default request sense CCB.
     */
    if (targ_setup_default_reqsns(b, t, l) != CAM_REQ_CMP) {

	/*
	 * Release the default inquiry CCB.
	 */
	targ_release_default_ccb(sc, nexus, (u_char)ALL_INQ_OP,
				 (U32)sizeof(ALL_INQ_CDB),
				 (U32)sizeof(ALL_INQ_DATA));

	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Add the test unit ready CCB.
     */
    if (targ_setup_default_tur(b, t, l) != CAM_REQ_CMP) {

	/*
	 * Release the default inquiry CCB.
	 */
	targ_release_default_ccb(sc, nexus, (u_char)ALL_INQ_OP,
				 (U32)sizeof(ALL_INQ_CDB),
				 (U32)sizeof(ALL_INQ_DATA));

	/*
	 * Release the default request sense CCB.
	 */
	targ_release_default_ccb(sc, nexus, (u_char)ALL_REQ_SENSE6_OP,
				 (U32)sizeof(ALL_REQ_SENSE_CDB6),
				 (U32)sizeof(ALL_REQ_SNS_DATA));

	return(CAM_PROVIDE_FAIL);
    }

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : targ_release_default
 *
 * Functional Description :
 *	Targ_release_default() will release the default CCB's which were
 *	setup with a call to targ_setup_default().
 *
 * Call Syntax :
 *	targ_release_default(sc, nexus)
 *
 * Arguments:
 *	sc	Pointer to associated SIM_SOFTC structure
 *	nexus	Pointer to associated NEXUS structure
 *
 * Return Value :  None
 */
void
targ_release_default(sc, nexus)
SIM_SOFTC *sc;
NEXUS *nexus;
{
    int s;

    /*
     * Clear the SZ_TARG_DEF bit in the NEXUS flag field.
     */
    SIM_SOFTC_LOCK(s, sc);
    nexus->flags &= ~SZ_TARG_DEF;
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Release the default inquiry CCB.
     */
    targ_release_default_ccb(sc, nexus, (u_char)ALL_INQ_OP,
			     (U32)sizeof(ALL_INQ_CDB),
			     (U32)sizeof(ALL_INQ_DATA));

    /*
     * Release the default request sense CCB.
     */
    targ_release_default_ccb(sc, nexus, (u_char)ALL_REQ_SENSE6_OP,
			     (U32)sizeof(ALL_REQ_SENSE_CDB6),
			     (U32)sizeof(ALL_REQ_SNS_DATA));

    /*
     * Release the default test unit ready CCB.
     */
    targ_release_default_ccb(sc, nexus, (u_char)ALL_TUR_OP,
			     (U32)sizeof(ALL_TUR_CDB), (U32)0);
}


/*
 * Routine Name : targ_setup_inquiry
 *
 * Functional Description :
 *	Setup the CCB, CDB, and inquiry data for the default Inquiry
 *	CCB.
 *
 * Call Syntax :
 *	targ_setup_inquiry(b, t, l)
 *
 * Arguments:
 *	b - associated bus
 *	t - associated target
 *	l - associated lun
 *
 * Return Value :
 *	CAM_REQ_CMP		- The setup completed
 *	CAM_PROVIDE_FAIL	- Unable to setup
 */
static U32
targ_setup_default_inquiry(b, t, l)
U32 b, t, l;
{
    CCB_SCSIIO *ccb;
    ALL_INQ_CDB *inq;
    ALL_INQ_DATA *inq_data;
    SIM_WS *sws;
    extern char *hostname;
    SIM_MODULE(targ_setup_inquiry);
    u_long status;

    CAM_ERROR(module, "Setting up default Inquiry CCB", 
	      SIM_LOG_PRILOW, NULL, NULL, NULL);

    if ((ccb = (CCB_SCSIIO *)xpt_ccb_alloc()) == (CCB_SCSIIO *)NULL) {
	CAM_ERROR(module, "Unable to allocate CCB.", 
		  SIM_LOG_PRIHIGH, NULL, NULL, NULL);
	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Setup the CCB.
     */
    ccb->cam_ch.cam_ccb_len = sizeof(CCB_SCSIIO);
    ccb->cam_ch.cam_func_code = XPT_SCSI_IO;
    ccb->cam_ch.cam_path_id = b;
    ccb->cam_ch.cam_target_id = t;
    ccb->cam_ch.cam_target_lun = l;
    ccb->cam_ch.cam_flags =
	CAM_DIR_OUT | CAM_DIS_AUTOSENSE | CAM_TGT_CCB_AVAIL;
    ccb->cam_cbfcnp = targ_default_comp;
    ccb->cam_timeout = CAM_TIME_INFINITY;	/* disable timers */
    ccb->cam_vu_flags = 0;			/* no VU stuff */
    ccb->cam_req_map = NULL;			/* no associated bp */
    ccb->cam_dxfer_len = sizeof(ALL_INQ_DATA);
    ccb->cam_cdb_len = sizeof(ALL_INQ_CDB);

    /*
     * Allocate data pointer for the default inquiry info.
     */
    if ((ccb->cam_data_ptr = (u_char *)sc_alloc(sizeof(ALL_INQ_DATA))) ==
	(u_char *)NULL) {

	CAM_ERROR(module, "Unable to allocate data buffer.", 
		  SIM_LOG_PRIHIGH, NULL, NULL, NULL);
	/*
	 * Free the allocated CCB.
	 */
	xpt_ccb_free(ccb);
	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Setup the Inquiry CDB.
     */
    inq = (ALL_INQ_CDB *)ccb->cam_cdb_io.cam_cdb_bytes;
    inq->opcode = ALL_INQ_OP;			/* inquiry command */
    inq->evpd = 0;				/* no product data */
    inq->lun = 0;				/* not used in SCSI-2 */
    inq->page = 0;				/* no product pages */
    inq->alloc_len = sizeof(ALL_INQ_DATA);	/* for the EDT array */
    inq->control = 0;				/* no control flags */

    /*
     * Setup the Inquiry data.
     */
    inq_data = (ALL_INQ_DATA *)ccb->cam_data_ptr;
    inq_data->pqual = ALL_PQUAL_NO_PHYS;
    inq_data->dtype = ALL_DTYPE_PROCESSOR;
    inq_data->rmb = !(ALL_RMB);
    inq_data->dmodify = 0;
    inq_data->iso = 0;
    inq_data->ecma = 0;
    inq_data->ansi = ALL_SCSI2;
    inq_data->aenc = !(ALL_ASYNC_NOTICE);
    inq_data->trmiop = !(ALL_TERMIOP);
    inq_data->rdf = ALL_RDF_SCSI2;
    inq_data->addlen = 0;
    inq_data->reladdr = !(ALL_RELATIVE_ADDR);
    inq_data->wbus32 = !(ALL_WBUS32);
    inq_data->wbus16 = !(ALL_WBUS16);
    inq_data->sync = !(ALL_SYNC);
    inq_data->linked = !(ALL_LINKED);
    inq_data->cmdque = !(ALL_CMD_QUE);
    inq_data->sftre = !(ALL_SOFTRESET);
    bcopy(TARG_VENDOR_ID, inq_data->vid, sizeof(inq_data->vid));
    bcopy(TARG_PRODUCT_ID, inq_data->pid, sizeof(inq_data->pid));
    bcopy(TARG_REVISION, inq_data->revlevel, sizeof(inq_data->revlevel));
    
    /*
     * Put this CCB into the SIM's queue.
     */
    if( (status = xpt_action((CCB_HEADER *)ccb)) != CAM_REQ_INPROG) {
	return(status);
    }

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : targ_setup_reqsns
 *
 * Functional Description :
 *	Setup the CCB, CDB, and sense data for the default request
 *	sense CCB.
 *
 * Call Syntax :
 *	targ_setup_reqsns(b, t, l)
 *
 * Arguments:
 *	b - associated bus
 *	t - associated target
 *	l - associated lun
 *
 * Return Value :
 *	CAM_REQ_CMP		- The setup completed
 *	CAM_PROVIDE_FAIL	- Unable to setup
 */
static U32
targ_setup_default_reqsns(b, t, l)
U32 b, t, l;
{
    CCB_SCSIIO *ccb;
    ALL_REQ_SENSE_CDB6 *reqsns;
    ALL_REQ_SNS_DATA *reqsns_data;
    SIM_WS *sws;
    U32 status;
    SIM_MODULE(targ_setup_reqsns);

    CAM_ERROR(module, "Setting up default Request Sense CCB", 
	      SIM_LOG_PRILOW, NULL, NULL, NULL);

    if ((ccb = (CCB_SCSIIO *)xpt_ccb_alloc()) == (CCB_SCSIIO *)NULL) {

	CAM_ERROR(module, "Unable to allocate CCB.", 
		  SIM_LOG_PRIHIGH, NULL, NULL, NULL);

	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Setup the CCB.
     */
    ccb->cam_ch.cam_ccb_len = sizeof(CCB_SCSIIO);
    ccb->cam_ch.cam_func_code = XPT_SCSI_IO;
    ccb->cam_ch.cam_path_id = b;
    ccb->cam_ch.cam_target_id = t;
    ccb->cam_ch.cam_target_lun = l;
    ccb->cam_ch.cam_flags =
	CAM_DIR_OUT | CAM_DIS_AUTOSENSE | CAM_TGT_CCB_AVAIL;
    ccb->cam_cbfcnp = targ_default_comp;
    ccb->cam_timeout = CAM_TIME_INFINITY;	/* disable timers */
    ccb->cam_vu_flags = 0;			/* no VU stuff */
    ccb->cam_req_map = NULL;			/* no associated bp */
    ccb->cam_dxfer_len = sizeof(ALL_REQ_SNS_DATA);
    ccb->cam_cdb_len = sizeof(ALL_REQ_SENSE_CDB6);

    /*
     * Allocate data pointer for the default request sense data.
     */
    if ((ccb->cam_data_ptr = (u_char *)sc_alloc(sizeof(ALL_REQ_SNS_DATA))) ==
	(u_char *)NULL) {

	CAM_ERROR(module, "Unable to allocate request sense buffer", 
		  SIM_LOG_PRIHIGH, NULL, NULL, NULL);

	/*
	 * Free the CCB
	 */
	xpt_ccb_free(ccb);
	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Setup the request sense CDB.
     */
    reqsns = (ALL_REQ_SENSE_CDB6 *)ccb->cam_cdb_io.cam_cdb_bytes;
    reqsns->opcode = ALL_REQ_SENSE6_OP;
    reqsns->lun = 0;
    reqsns->alloc_len = sizeof(ALL_REQ_SNS_DATA);
    reqsns->control = 0;

    /*
     * Setup the Request sense data.
     */
    reqsns_data = (ALL_REQ_SNS_DATA *)ccb->cam_data_ptr;
    reqsns_data->valid = 1;
    reqsns_data->error_code = ALL_IMED_ERR_CODE;
    reqsns_data->segment = 0;
    reqsns_data->filemark = 0;
    reqsns_data->eom = 0;
    reqsns_data->ili = 0;
    reqsns_data->sns_key = ALL_ILLEGAL_REQ;
    reqsns_data->info_byte3 = 0;
    reqsns_data->info_byte2 = 0;
    reqsns_data->info_byte1 = 0;
    reqsns_data->info_byte0 = 0;
    reqsns_data->addition_len = 0;
    reqsns_data->cmd_specific3 = 0;
    reqsns_data->cmd_specific2 = 0;
    reqsns_data->cmd_specific1 = 0;
    reqsns_data->cmd_specific0 = 0;
    reqsns_data->asc = ASCQ_LUN_NSUP;
    reqsns_data->asq = 0;
    reqsns_data->fru = 0;
    
    /*
     * Put this CCB into the SIM's queue.
     */
    if( (status = xpt_action((CCB_HEADER *)ccb)) != CAM_REQ_INPROG) {
	return(status);
    }
    return(CAM_REQ_CMP);
}

/*
 * Routine Name : targ_setup_tur
 *
 * Functional Description :
 *	Setup the CCB and CDB for the default test unit ready CCB.
 *
 * Call Syntax :
 *	targ_setup_tur(b, t, l)
 *
 * Arguments:
 *	b - associated bus
 *	t - associated target
 *	l - associated lun
 *
 * Return Value :
 *	CAM_REQ_CMP		- The setup completed
 *	CAM_PROVIDE_FAIL	- Unable to setup
 */
static U32
targ_setup_default_tur(b, t, l)
U32 b, t, l;
{
    CCB_SCSIIO *ccb;
    ALL_TUR_CDB *tur;
    SIM_WS *sws;
    U32 status;
    SIM_MODULE(targ_setup_tur);

    CAM_ERROR(module, "Setting up default test unit ready CCB", 
	      SIM_LOG_PRILOW, NULL, NULL, NULL);

    if ((ccb = (CCB_SCSIIO *)xpt_ccb_alloc()) == (CCB_SCSIIO *)NULL) {

	CAM_ERROR(module, "Unable to allocate CCB.", 
		  SIM_LOG_PRIHIGH, NULL, NULL, NULL);

	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Setup the CCB.
     */
    ccb->cam_ch.cam_ccb_len = sizeof(CCB_SCSIIO);
    ccb->cam_ch.cam_func_code = XPT_SCSI_IO;
    ccb->cam_ch.cam_path_id = b;
    ccb->cam_ch.cam_target_id = t;
    ccb->cam_ch.cam_target_lun = l;
    ccb->cam_ch.cam_flags =
	CAM_DIR_OUT | CAM_DIS_AUTOSENSE | CAM_TGT_CCB_AVAIL;
    ccb->cam_cbfcnp = NULL;
    ccb->cam_timeout = CAM_TIME_INFINITY;	/* disable timers */
    ccb->cam_vu_flags = 0;			/* no VU stuff */
    ccb->cam_req_map = NULL;			/* no associated bp */
    ccb->cam_cbfcnp = targ_default_comp;
    ccb->cam_dxfer_len = 0;
    ccb->cam_cdb_len = sizeof(ALL_TUR_CDB);

    /*
     * Setup the Inquiry CDB.
     */
    tur = (ALL_TUR_CDB *)ccb->cam_cdb_io.cam_cdb_bytes;
    tur->opcode = ALL_TUR_OP;		/* test unit ready command */
    tur->lun = 0;			/* not used in SCSI-2 */
    tur->control = 0;			/* no control flags */
    
    /*
     * Put this CCB into the SIM's queue.
     */
    if( (status = xpt_action((CCB_HEADER *)ccb)) != CAM_REQ_INPROG) {
	return(status);
    }

    return(CAM_REQ_CMP);
}

/**
 * targ_default_comp()
 *
 * FUNCTIONAL DESCRIPTION:
 *	Callback completion funtion for all the default target CCBs.
 *
 * FORMAL PARAMETERS:
 *	ccb - Pointer to completed default target ccb.
 *
 * IMPLICIT INPUTS:
 *       NONE
 *
 * IMPLICIT OUTPUTS:
 *       NONE
 *
 * RETURN VALUE:
 *       NONE
 *
 * SIDE EFFECTS:
 *       NONE
 *
 * ADDITIONAL INFORMATION:
 *       NONE
 *
 **/
void
targ_default_comp(ccb)
CCB_SCSIIO *ccb;
{
    U32 status;
    int s;
    SIM_SOFTC *sc = SC_GET_SOFTC(ccb->cam_ch.cam_path_id);

    /*
     * Just recycle CCB no matter what the status is - we don't need to
     * do anything when a default target ccb completes.
     */
    SIM_SOFTC_LOCK(s, sc); 
    ccb->cam_ch.cam_status = CAM_REQ_INPROG;
    ccb->cam_ch.cam_flags |= CAM_TGT_CCB_AVAIL;
    SIM_SOFTC_UNLOCK(s,sc);

}		/* targ_default_comp */


/*
 * Routine Name : targ_release_default_ccb
 *
 * Functional Description :
 *	Free the specified default CCB.
 *
 * Call Syntax :
 *	targ_release_default_ccb(sc, nexus, cmd, cmdln, dataln)
 *
 * Arguments:
 *	sc	Associated SIM_SOFTC structure pointer
 *	nexus	Associated NEXUS structure pointer
 *	cmd	SCSI opcode of command to match on.
 *	cmdln	Number of bytes in CDB.
 *	dataln	Number of bytes in associated data buffer.
 *
 * Return Value : None
 */
static void
targ_release_default_ccb(sc, nexus, cmd, cmdln, dataln)
SIM_SOFTC *sc;
NEXUS *nexus;
u_char cmd;
U32 cmdln;
U32 dataln;
{
    SIM_WS *sws;

    /*
     * Find the associated SIM_WS for the default command passed.
     */
    if (targ_find_sws(sc, nexus, cmd, cmdln, &sws) != CAM_REQ_CMP) {
	return;
    }

    /*
     * Call ss_finish() to remove the SIM_WS from the NEXUS queue.
     */
    ss_finish(sws);

    /*
     * Call DME_END to release any assigned DME resources.
     */
    DME_END(sc, &sws->data_xfer);

    /*
     * Release the data area.
     */
    if (dataln)
	sc_free(sws->ccb->cam_data_ptr, dataln);

    /*
     * Release the CCB.
     */
    xpt_ccb_free(sws->ccb);
}

/*
 * Routine Name : targ_find_sws
 *
 * Functional Description :
 *	Find a SIM_WS within the specified NEXUS list which contains
 *	the specified CDB.
 *
 * Call Syntax :
 *	targ_find_sws(nexus, cmd, cdb_len, return_sws)
 *
 * Arguments:
 *	sci		SIM Softc
 *	nexus		Associated NEXUS structure pointer
 *	cmd		SCSI opcode of command to match on.
 *	cdb_len		Number of bytes in CDB.
 *	return_sws	Once a match has been completed, the associated
 *			SIM_WS pointer will be returned to the calling
 *			function in the return_sws pointer.
 *
 * Return Value : 
 *	CAM_REQ_CMP		If a useable SIM_WS has been found.
 *	CAM_PROVIDE_FAIL	If unable to find a SIM_WS
 *	CAM_BUSY		If SIM_WS was found, but the CAM_TGT_CCB_AVAIL
 *				bit isn't set in its cam flags.
 */
U32
targ_find_sws(sc, nexus, cmd, cdb_len, return_sws)
SIM_SOFTC *sc;
NEXUS *nexus;
u_char cmd;
U32 cdb_len;
SIM_WS **return_sws;
{
    U32 retval = CAM_PROVIDE_FAIL;
    SIM_WS *sws;
    int s;

    /*
     * Lock on the SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Initialize the return SIM_WS to NULL.
     */
    *return_sws = (SIM_WS *)NULL;

    /*
     * Start the search at the head of the queue.
     */
    sws = (SIM_WS *)nexus;

    /*
     * Search through the NEXUS queue looking for the specified
     * ccb.
     */
    while ((sws = sws->flink) != (SIM_WS *)nexus) {

	/*
	 * Look for the specified SCSI command.
	 */
	if (sws->cam_flags & CAM_CDB_POINTER) {
	    if (sws->ccb->cam_cdb_io.cam_cdb_ptr[0] != cmd)
		continue;
	}
	else {
	    if (sws->ccb->cam_cdb_io.cam_cdb_bytes[0] != cmd)
		continue;
	}

	/*
	 * Check the length of the CDB.
	 */
	if (sws->ccb->cam_cdb_len != cdb_len)
	    continue;

	retval = CAM_REQ_CMP;
	*return_sws = sws;
	break;
    }

    /*
     * unlock on the SIM_SOFTC.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    return(retval);
}

U32
targ_find_avail_sws(sc, nexus, cmd, cdb_len, return_sws)
SIM_SOFTC *sc;
NEXUS *nexus;
u_char cmd;
U32 cdb_len;
SIM_WS **return_sws;
{
    U32 retval = CAM_PROVIDE_FAIL;
    SIM_WS *sws;
    int s;

    /*
     * Lock on the SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Initialize the return SIM_WS to NULL.
     */
    *return_sws = (SIM_WS *)NULL;

    /*
     * Start the search at the head of the queue.
     */
    sws = (SIM_WS *)nexus;

    /*
     * Search through the NEXUS queue looking for the specified
     * ccb.
     */
    while ((sws = sws->flink) != (SIM_WS *)nexus) {

	/*
	 * Look for the specified SCSI command.
	 */
	if (sws->cam_flags & CAM_CDB_POINTER) {
	    if (sws->ccb->cam_cdb_io.cam_cdb_ptr[0] != cmd)
		continue;
	} else {
	    if (sws->ccb->cam_cdb_io.cam_cdb_bytes[0] != cmd)
		continue;
	}

	/*
	 * Check the length of the CDB.
	 */
	if (sws->ccb->cam_cdb_len != cdb_len)
	    continue;

	/*
	 * Check whether the upper layer us trying to disable this lun.
	 */
   	if(sws->nexus->flags & SZ_TARG_DISABLE_LUN) {
		/*
		 * If the ccb is available then wake the process waiting in 
		 * in sx_enable_lun.
		 */
		if (sws->ccb->cam_ch.cam_flags & CAM_TGT_CCB_AVAIL)
			wakeup(sws->ccb);
	    	retval = CAM_PROVIDE_FAIL;
	    	continue;
	}

	/*
	 * Make sure that the CAM_TGT_CCB_AVAIL bit is set in
	 * the CAM flags.
	 *
	 * If it's not, set the return value to CAM_BUSY just
	 * in case this is the only match.
	 */
	if ((sws->ccb->cam_ch.cam_flags & CAM_TGT_CCB_AVAIL) == 0) {
	    retval = CAM_BUSY;
	    continue;
	}

	/*
	 * We've made a match.  Clear the CAM_TGT_CCB_AVAIL bit.
	 */
	sws->ccb->cam_ch.cam_flags &= ~CAM_TGT_CCB_AVAIL;
	retval = CAM_REQ_CMP;
	*return_sws = sws;
	break;
    }

    /*
     * UnLock on the SIM_SOFTC.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    return(retval);

}

/*
 * Routine Name : targ_validate_cdb
 *
/*
 * Routine Name : targ_validate_cdb
 *
 * Functional Description :
 * 	Validate CDB.  Make sure that the specified CDB is supported
 * 	for target mode.  Check the CDB length.
 *
 * Call Syntax :
 *	targ_validate_cdb(cmd, command_len)
 *
 * Arguments:
 *	cmd		SCSI opcode of command to match on.
 *	command_len	Number of bytes in CDB.
 *
 * Return Value : 
 *	CAM_REQ_CMP		If validation is successful
 *	CAM_REQ_CMP_ERR		If validation fails
 */
U32
targ_validate_cdb(cmd, command_len)
u_char cmd;
u_char command_len;
{
    U32 retval = CAM_REQ_CMP_ERR;

    switch(cmd) {

    case ALL_CHANGE_DEF10_OP:
	if (command_len == sizeof(ALL_CHANGE_DEF_CDB10))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_COMPARE10_OP:
	if (command_len == sizeof(ALL_COMPARE_CDB10))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_COPY_OP:
	if (command_len == sizeof(ALL_COPY_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_COPY_AND_VERIFY10_OP:
	if (command_len == sizeof(ALL_COPY_AND_VERIFY_CDB10))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_INQ_OP:
        if (command_len == sizeof(ALL_INQ_CDB)) 
	    retval = CAM_REQ_CMP;
	break;

    case ALL_LOG_SEL_OP:
        if (command_len == sizeof(ALL_LOG_SEL_CDB10))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_LOG_SNS_OP:
        if (command_len == sizeof(ALL_LOG_SNS_CDB10))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_READ_BUFFER_OP:
        if (command_len == sizeof(ALL_READ_BUFFER_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case PROC_RECEIVE_OP:
	if (command_len == sizeof(PROC_RECEIVE_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_RECEIVE_DIAGNOSTIC_OP:
        if (command_len == sizeof(ALL_RECEIVE_DIAGNOSTIC_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_REQ_SENSE6_OP:
        if (command_len == sizeof(ALL_REQ_SENSE_CDB6))
	    retval = CAM_REQ_CMP;
	break;

    case PROC_SEND_OP:
	if (command_len == sizeof(PROC_SEND_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_SEND_DIAGNOSTIC_OP:
        if (command_len == sizeof(ALL_SEND_DIAGNOSTIC_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_TUR_OP:
        if (command_len == sizeof(ALL_TUR_CDB))
	    retval = CAM_REQ_CMP;
	break;

    case ALL_WRITE_BUFFER_OP:
        if (command_len == sizeof(ALL_WRITE_BUFFER_CDB))
	    retval = CAM_REQ_CMP;
	break;

    default:
	break;
    }

    return(retval);
}
/*
 * Routine Name : sm_copy_targws
 *
 * Functional Description :
 *	This function will copy the contents of the first SIM_WS
 *	into the second SIM_WS.
 *
 * Call Syntax :
 *	sm_copy_targws(sws1, sws2)
 *
 * Arguments:
 *	SIM_WS *sws1;
 *	SIM_WS *sws2;
 *
 * Return Value :  None
 */
void
sm_copy_targws(sws1, sws2)
SIM_WS *sws1, *sws2;
{
    register int i;
    SIM_MODULE(sm_copy_targws);

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
    for (i=sws1->msginq.prev; i < sws1->msginq.prev_cnt; i++)
	SC_ADD_MSGIN(sws2, sws1->msginq_buf[i]);
    for (i=sws1->msginq.curr; i < sws1->msginq.curr_cnt; i++)
	SC_ADD_MSGIN(sws2, sws1->msginq_buf[i]);
    sws2->msginq.prev_cnt = sws1->msginq.prev_cnt;
    sws2->msginq.curr_cnt = sws1->msginq.curr_cnt;

    /*
     * Update the SIM_WS flags field.
     */
    sws2->flags |= sws1->flags;

    /*
     * Copy the SIM_TARG_WS
     */
    sws2->targ_ws.flags = sws1->targ_ws.flags;
    sws2->targ_ws.disconnect = sws1->targ_ws.disconnect;
    
    /*
     * To act as a target, copy the command recv and the dme_descriptor.
     */
    for (i=0; i < SIM_TARG_MODE_CMD_LN; i++)
	sws2->targ_ws.command[i] = sws1->targ_ws.command[i];

    /*
     * Copy the recieved CDB directly into the ccb si that it can be returned
     * to the PDRV for examination.
     */
    for (i=0; i < sws2->ccb->cam_cdb_len; i++)
	sws2->ccb->cam_cdb_io.cam_cdb_bytes[i] = sws1->targ_ws.command[i];

    sws2->targ_ws.command_len = sws1->targ_ws.command_len;
    sws2->data_xfer = sws1->data_xfer;

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

