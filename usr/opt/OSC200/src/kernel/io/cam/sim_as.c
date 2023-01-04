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
static char *rcsid = "@(#)$RCSfile: sim_as.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/27 17:55:20 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_as.c		Version 1.07			Nov. 13, 1991 

	The source file sim_as.c contains functions which
	are specific to performing auto-sense by the SIM layer.

Modification History:

	1.07	11/13/91	janet
	Removed functional queues.  Added error logging.

	1.06	10/22/91	janet
	o Added SIM_MODULE() to every function.
	o Replaced all PRINTD's with SIM_PRINTD's.

	1.05	07/23/91	jag
	Added the support to Autosense resid. in as_finish().

	1.04	07/08/91	janet
	In as_start() if the NEXUS is frozen and this is the CCB which froze
	it, unfreeze it so the autosense may proceed.

	1.03	05/29/91	janet
	as_start() now uses the "it_nexus" field of the SIM_WS.

	1.02	03/31/91	jag
	Changed unix_bp to req_map, Rev 2.3 cam.h

	1.01	03/26/91	janet
	Updated after code review.

	1.00	01/19/91	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define SZ_REQUEST_SENSE 0x03
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
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_status.h>
#include <io/cam/cam_logger.h>
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
extern void		sim_logger();
extern I32		xpt_ccb_free();
extern I32		xpt_action();
extern CCB_HEADER	*xpt_ccb_alloc();

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
void		as_start();
void		as_finish();
void		as_adjust();
static void (*local_errlog)() = sim_logger; 

/*

Description of AutoSense:

	Autosense means that on a SCSI check condition (or command
	terminated) status a SCSI request sense command will
	be created and sent to the target/lun.  This process is done
	invisibly to the peripheral driver.

	Autosense is initiated at the time a check condition status
	has been received.  The SIM_WS's "cam_status" is set to
	CAM_REQ_CMP_ERR by the SCSI status portion of the state
	machine.  The function as_start() is then called with the
	SIM_WS pointer.

	If autosense has been disabled (by setting CAM_DIS_AUTOSENSE
	in the cam_flags) the NEXUS queue will be frozen and no request
	sense CCB will be generated.

	If autosense is performed a request sense CCB will be generated
	and placed at the head of the NEXUS queue.  The CAM_SIM_QFREEZE
	bit will be set so the NEXUS queue will be left frozen
	after the request sense CCB has been started.

	If the request sense CCB is successful the CAM_AUTOSNS_VALID
	bit will be set in the "cam_status" of the CCB which generated
	the check condition.  The autosense residual field in the CCB
	will also be updated.  If for any reason the request sense
	fails the CCB's "cam_status" will be set to CAM_AUTOSENSE_FAIL.

	After the request sense CCB completes a call-back will
	be made to the peripheral driver (if call-backs were enabled).

	If the target which generated the check condition was running
	synchronously, a synchronous renegotiation will be scheduled.
*/

/*
 * Routine Name :  as_start
 *
 * Functional Description :
 *	As_start() will determine if auto-sense should be performed.
 *	Autosense will not be performed if CAM_DIS_AUTOSENSE is set
 *	in the cam_flags.  If autosense is not needed, this routine
 *	will do nothing.  If auto-sense should be performed
 *	this routine will create a SCSI request sense CCB and put
 *	it at the head of the NEXUS queue.
 *
 *	The request sense CCB will have the CAM_SIM_QFREEZE bit set,
 *	therefore the NEXUS queue will be left frozen after the
 *	autosense procedure has been completed.
 *
 *	If for any reason auto-sense cannot be performed, the
 *	SIM_WS's "cam_status" field will be set to CAM_AUTOSENSE_FAIL.
 *
 * Call Syntax :
 *	as_start(sws)
 *
 * Arguments:
 *	SIM_WS *sws 	SIM_WS pointer of request which generated the
 *			check condition or command terminated.
 *
 * Return Value :  None.
 */
void
as_start(sws)
SIM_WS *sws;
{
    CCB_SCSIIO *rs_ccb;		/* CCB pointer for request sense */
    SIM_WS *rs_sws;		/* SIM_WS pointer for request sense */
    SIM_MODULE(as_start);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    /*
     * If we are currently running in synchronous mode a renegotiation
     * is needed.  If this is the case, set the SZ_SYNC_NEEDED bit
     * in the IT_NEXUS flags field.  Version 1.03
     */
    if (sws->it_nexus->flags & SZ_SYNC)
	sws->it_nexus->flags |= SZ_SYNC_NEEDED;

    /*
     * Determine if autosense should be performed.  It is not necessary
     * to freeze the NEXUS queue, that will be taken care of in
     * ss_finish().
     */
    if (sws->cam_flags & CAM_DIS_AUTOSENSE) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, autosense not enabled.\n"));
	return;
    }

    /*
     * If the NEXUS is frozen and this is the CCB which froze it,
     * unfreeze it so the autosense may proceed.
     * Version 1.04
     */
    if ((sws->nexus->freeze_count) && (sws->cam_flags & CAM_SIM_QFREEZE)) {
	SC_UNFREEZE_QUEUE(sws);
    }

    /*
     * Allocate a CCB from the XPT layer.  If unable to allocate,
     * return.  The CAM_AUTOSENSE_FAIL cam_status will be used to
     * indicate the inability to perform the request sense command.
     */
    if ((rs_ccb = (CCB_SCSIIO *)xpt_ccb_alloc()) == (CCB_SCSIIO *)NULL) {
	sws->cam_status = CAM_AUTOSENSE_FAIL;
	CAM_ERROR(module,
		  "Unable to allocate a CCB",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, unable to allocate a CCB.\n"));
	return;
    }

    /*
     * Set-up the new CCB.
     */
    rs_ccb->cam_ch.cam_ccb_len = sizeof(CCB_SCSIIO);
    rs_ccb->cam_ch.cam_func_code = XPT_SCSI_IO;
    rs_ccb->cam_ch.cam_path_id = sws->cntlr;
    rs_ccb->cam_ch.cam_target_id = sws->targid;
    rs_ccb->cam_ch.cam_target_lun = sws->lun;
    rs_ccb->cam_ch.cam_flags =
	CAM_DIR_IN | CAM_DIS_AUTOSENSE | CAM_SIM_QHEAD | CAM_SIM_QFREEZE;

    /*
     * Set the request sense callback function to as_finish().
     */
    rs_ccb->cam_cbfcnp = as_finish;

    /*
     * Get the data transfer length and data buffer from the autosense
     * fields of the original CCB.
     */
    rs_ccb->cam_data_ptr = sws->ccb->cam_sense_ptr;
    rs_ccb->cam_dxfer_len = sws->ccb->cam_sense_len;

    /*
     * Create the request sense CDB.
     */
    rs_ccb->cam_cdb_len = 6;
    rs_ccb->cam_cdb_io.cam_cdb_bytes[0] = SZ_REQUEST_SENSE;
    rs_ccb->cam_cdb_io.cam_cdb_bytes[1] = 0;
    rs_ccb->cam_cdb_io.cam_cdb_bytes[2] = 0;
    rs_ccb->cam_cdb_io.cam_cdb_bytes[3] = 0;
    rs_ccb->cam_cdb_io.cam_cdb_bytes[4] = sws->ccb->cam_sense_len;
    rs_ccb->cam_cdb_io.cam_cdb_bytes[5] = 0;

    /*
     * Set the buff struct pointer to the same as the
     * original request's.
     */
    rs_ccb->cam_req_map = sws->ccb->cam_req_map;

    /*
     * Modify the original CCB so that its "callback" routine will
     * call as_adjust().  Save the original "callback" function pointer
     * in the SIM_WS.
     */
    sws->as_callback = sws->ccb->cam_cbfcnp;
    sws->ccb->cam_cbfcnp = as_adjust;

    /*
     * Set the SZ_AS_INPROG bit in the SIM_WS.  This will let the
     * SIM_XPT layer know not to update the cam_status of this
     * CCB when it completes.
     */
    sws->flags |= SZ_AS_INPROG;

    /*
     * Make sure the scheduler knows which ccb is the autosense one (so
     * it always does that one first - before any other head of queue requests).
     */
    if (sws->nexus->as_simws_ptr) {
	CAM_ERROR(module,
		  "Second autosense not possible",
		  SIM_LOG_SIM_WS,
		  NULL, rs_sws, NULL);
    }
    sws->nexus->as_simws_ptr = SC_GET_WS(rs_ccb);

    /*
     * Start the new CCB by calling the XPT action function.
     */
    xpt_action((CCB_HEADER *)rs_ccb);

    /*
     * After calling xpt_action() store the original CCB in the
     * request sense SIM_WS.  This must be done after the xpt_action()
     * call since the SIM_WS will be zeroed out.
     */
    rs_sws = SC_GET_WS(rs_ccb);
    rs_sws->as_ccb = sws->ccb;

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  as_finish
 *
 * Functional Description :
 *	As_finish() will complete the autosense procedure.  As_finish()
 *	is the "call-back" function for the autosense CCB.  Therefore
 *	this function will be called by the SIM XPT layer when the
 *	request completes.  This function will check the CAM status
 *	of the request sense command.  If it is CAM_REQ_CMP the
 *	CAM_AUTOSNS_VALID bit will be set in the original CCB (that is
 *	the CCB which generated the check condition).  If for any reason
 *	the request sense command did not complete or completed
 *	with an error the "cam_status" of CAM_AUTOSENSE_FAIL will
 *	be set in the original CCB.
 *
 *	As_finish() is then responsible for freeing the request
 *	sense CCB and performing a "call-back" on the original CCB.
 *
 * Call Syntax :
 *	as_finish(rs_ccb)
 *
 * Arguments:
 *	CCB_SCSIIO *rs_ccb	- pointer to the request sense CCB.
 *
 * Return Value :  None.
 */
void
as_finish(rs_ccb)
CCB_SCSIIO *rs_ccb;
{
    SIM_WS *rs_sws = SC_GET_WS(rs_ccb);
    CCB_SCSIIO *orig_ccb = rs_sws->as_ccb;
    SIM_WS *orig_sws = SC_GET_WS(orig_ccb);
    U32 cam_status;
    SIM_MODULE(as_finish);

    SIM_PRINTD(rs_sws->cntlr, rs_sws->targid, rs_sws->lun, CAMD_INOUT,
	       ("begin\n"));

    SIM_PRINTD(rs_sws->cntlr, rs_sws->targid, rs_sws->lun, CAMD_FLOW,
	       ("orig seq_num 0x%x, request sense seq_num 0x%x\n",
		orig_sws->seq_num, rs_sws->seq_num));

    /*
     * Determine the cam_status for the original CCB.
     */
    cam_status = orig_sws->cam_status;

    /*
     * Check the cam status of the CCB.  If CAM_REQ_CMP, set the
     * CAM_AUTOSNS_VALID bit for the original CCB.  The CAM_AUTOSNS_VALID
     * bit will NOT be set if any other status is present and the CAM
     * status of the request will be set to CAM_AUTOSENSE_FAIL.
     */
    if (rs_sws->cam_status & CAM_REQ_CMP) {
	cam_status |= CAM_AUTOSNS_VALID;
    }
    else {
	CAM_ERROR(module,
		  "Autosense failed",
		  SIM_LOG_SIM_WS,
		  NULL, rs_sws, NULL);
	cam_status = CAM_AUTOSENSE_FAIL;
    }

    /*
     * "Or" in the CAM_SIM_QFRZN bit.  A check will be performed to
     * see if the queue has been frozen, but there is no case where
     * it hasn't been.
     */
    if (orig_sws->nexus->freeze_count)
	cam_status |= CAM_SIM_QFRZN;

    /*
     * Rev: 1.05
     * Update the Autosense residual field.  This will allow the PDrvs to
     * determine the number of valid sense data bytes.
     */
    orig_ccb->cam_sense_resid = (u_char)rs_ccb->cam_resid;

    /*
     * Free the request sense CCB.
     */
    xpt_ccb_free((CCB_HEADER *)rs_ccb);

    /*
     * Clear the SZ_AS_INPROG bit in the SIM_WS.
     */
    orig_sws->flags &= ~SZ_AS_INPROG;

    /*
     * Assign the cam_status to the original CCB.
     */
    orig_ccb->cam_ch.cam_status = cam_status;

    /*
     * Callback, if enabled.
     */
    if (!(orig_sws->cam_flags & CAM_DIS_CALLBACK))
	orig_ccb->cam_cbfcnp(orig_ccb);

    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name :  as_adjust
 *
 * Functional Description :
 *	As_adjust() will be called when the CCB which generated the
 *	check condition completes.  This is done to prevent the
 *	peripheral driver from being called until the autosense
 *	procedure (request sense CCB) completes.
 *
 * Call Syntax :
 *	as_adjust(ccb)
 *
 * Arguments:
 *	CCB_SCSIIO *ccb		- pointer to the CCB which generated
 *				  the check condition.
 *
 * Return Value :  None.
 */
void
as_adjust(ccb)
CCB_SCSIIO *ccb;
{
    SIM_WS *sws = SC_GET_WS(ccb);
    SIM_MODULE(as_adjust);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    ccb->cam_cbfcnp = sws->as_callback;

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, seq_num 0x%x\n", sws->seq_num));
}
