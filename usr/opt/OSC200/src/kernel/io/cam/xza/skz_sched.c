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
static char *rcsid = "@(#)$RCSfile: skz_sched.c,v $ $Revision: 1.1.2.9 $ (DEC) $Date: 1992/12/14 16:00:27 $";
#endif

/* ---------------------------------------------------------------------- */
/* Local defines.
 */
#define SS_INC_ID		0x0001L	/* Increment the targ/lun pair	*/
#define SS_DEC_ID		0x0002L	/* Decrement the targ/lun pair	*/
#define SS_TIMEOUT_PERIOD	(2*hz)	/* Once every other second */
#define CAMERRLOG

/* ---------------------------------------------------------------------- */
/* Include files.
 */
#include <io/common/iotypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <kern/queue.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/domain.h>
#include <io/cam/dec_cam.h>
#include <mach/vm_param.h>
#include <io/cam/cam.h>
#include <io/cam/scsi_phases.h>
#include <io/cam/scsi_status.h>
#include <io/cam/scsi_all.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <io/cam/cam_debug.h>
#include <io/cam/cam_errlog.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/dme.h>
#include <io/cam/sim.h>
#include <io/cam/sim_common.h>
/* ---------------------------------------------------------------------- */
/* External declarations.
 */
extern void	sc_setup_ws();
extern void	sim_logger();
extern U32	sx_get_tag();
extern U32	sx_clear_itl_nexus();
extern U32	sx_command_complete();
extern U32	sx_reset_detected();
extern SIM_SOFTC *softc_directory[];
extern int	hz; /* number of times a second that the clock interrupts */
extern I32	sim_default_timeout;
extern void	ss_resched_request();

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
U32		skz_ss_start();
U32		skz_ss_device_reset();
U32		skz_ss_abort();
U32		skz_ss_terminate_io();
static SIM_WS*  ss_find_flagged_request();

static void (*local_errlog)() = sim_logger;

/* ---------------------------------------------------------------------- */

/*
 * Routine Name : skz_ss_start
 *
 * Functional Description :
 *	Ss_start() is responsible for starting off a SCSI command
 *	sequence.  Ss_start() should only be called for a CAM "SCSI I/O
 *	request CCB."  Separate entries are provided for bus device
 *	reset, abort, and terminate I/O.  This function is called
 *	directly by ss_sched().
 *
 * Call Syntax :
 *	ss_start(sws)
 *
 * Arguments:
 *	SIM_WS *sws -	Request which is to be started.
 *
 * Return Value:  CAM_REQ_CMP, CAM_BUSY, CAM_REQ_INVALID.
 */
U32
skz_ss_start(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    U32 status;
    int s;
    SIM_MODULE(skz_ss_start);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin - sws=0x%x, sc=0x%x\n", sws, sc));

    SC_ADD_FUNC(sws, module);

    /*
     * Error checking.  Make sure the CDB is greater than 0
     * bytes.
     */
    if (sws->ccb->cam_cdb_len == 0) {
	CAM_ERROR(module,
		  "CDB length is zero",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));

	return(CAM_REQ_INVALID);
    }

    /*
     * Set-up the necessary DME resources by calling DME_SETUP().
     * Check to be sure that a DME setup hasn't already been
     * performed.  It's possible that this request was rescheduled.
     */

/*
    if (sws->data_xfer.segment == (SEGMENT_ELEMENT *)NULL) {

	if (DME_SETUP(sc, sws, (U32) sws->ccb->cam_dxfer_len,
		      (u_char *) sws->ccb->cam_data_ptr,
		      sws->cam_flags, &sws->data_xfer) != CAM_REQ_CMP) {

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end\n"));
	    return(CAM_PROVIDE_FAIL);
	}
    }
*/
    
    /*
     * SMP lock while changes flags in the NEXUS.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Check the "CAM flags" field for "disable disconnect".
     */
    if (sws->cam_flags & CAM_DIS_DISCONNECT) {
	/*
	 * If set, set the NEXUS "flags" field bit SZ_NO_DISCON.
	 */
	sws->nexus->flags |= SZ_NO_DISCON;
    }
    else {
	/*
	 * If not set, clear the NEXUS "flags" field bit SZ_NO_DISCON.
	 */
	sws->nexus->flags &= ~SZ_NO_DISCON;
    }

    /*
     * Check the "CAM flags" field for "initiate synchronous transfers".
     */
    if (sws->cam_flags & CAM_INITIATE_SYNC) {
	/*
	 * Synchronous information is kept per target, not per LUN,
	 * so use the IT_NEXUS structure.
	 * Version 1.04
	 */

	/*
	 * If "initiate synchronous transfers" is set, set the
	 * SZ_SYNC_NEEDED flag.  Make sure that SZ_SYNC_CLEAR and
	 * SZ_SYNC_NEG aren't set.  If SZ_SYNC_NEG is set then we
	 * are already trying to setup for sync. 
	 */
	if (!(sws->it_nexus->flags & SZ_SYNC_NEG)) {
	    sws->it_nexus->flags |= SZ_SYNC_NEEDED;
	    sws->it_nexus->flags &= ~SZ_SYNC_CLEAR;
	}
    }

    /*
     * Check the CCB's "CAM flags" field for "disable synchronous transfers".
     */
    if (sws->cam_flags & CAM_DIS_SYNC) {
	/*
	 * Synchronous information is kept per target, not per LUN,
	 * so use the IT_NEXUS structure.
	 * Version 1.04
	 */

	/*
	 * Is SZ_SYNC_NEEDED set?  If so, clear it.
	 */
	sws->it_nexus->flags &= ~SZ_SYNC_NEEDED;

	/*
	 * Is SZ_SYNC set?  If so, set the SZ_SYNC_CLEAR bit.  This
	 * will cause a renegotiation using zero offset and period.
	 */
	if (sws->it_nexus->flags & SZ_SYNC) {
	    sws->it_nexus->flags &= ~SZ_SYNC;
	    sws->it_nexus->flags |= SZ_SYNC_CLEAR | SZ_SYNC_NEEDED;
	}
    }

    /*
     * SMP unlock.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Call sc_gen_msg() with an action of SC_MSG_START.  Sc_gen_msg()
     * will use the setting above to determine it the device may
     * disconnect or is running tagged.
     */
/*
    sc_gen_msg(sws, SC_MSG_START);
*/

    /*
     * Call the HBA specific go function, hba_go(), to start the
     * chip off.  Return whatever hba_go() returns.
     */
    status = SC_HBA_GO(sc, sws);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, status is 0x%x\n", status));

    return(status);
}


/*
 * Routine Name : skz_ss_device_reset
 *
 * Functional Description :
 *	Ss_perform_device_reset() is called by ss_sched() at high IPL
 *	and SMP locked to perform a SCSI bus device reset.
 *
 * Call Syntax:
 *	status = ss_perform_device_reset(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *
 * Return Value:  CAM status of CAM_REQ_CMP, CAM_PROVIDE_FAIL, or
 *		  CAM_SCSI_BUSY.
 */
U32
skz_ss_device_reset(sc)
register SIM_SOFTC *sc;
{
    U32 targid, t_flags, status;
    register SIM_WS *sws;
    SIM_MODULE(ss_perform_device_reset);


    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc));

    /*
     * Make sure that the bdr_sws is available.
     */
    if(sc->bdr_sws.flags & SZ_DEVRS_INPROG)
	return(CAM_SCSI_BUSY);

    /*
     * Determine the target which is to be reset.
     */
    for(targid=0; targid<NDPS; targid++)
	if(sc->device_reset_needed & (1<<targid))
	    break;

    /*
     * Did we find a valid device to reset?
     */
    if (targid >= NDPS) {
	CAM_ERROR(module,
		  "reset_needed field corrupted",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	/*
	 * The "device_reset_needed" flag got corrupted somehow.  Clear it
	 * and return.
	 */
	sc->device_reset_needed = 0;
	SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		   ("end\n"));
	return(CAM_PROVIDE_FAIL);
    }

    SIM_PRINTD(sc->cntlr, targid, NOBTL, CAMD_FLOW,
	       ("target to be reset: %d\n", targid));

    /*
     * The SIM_SOFTC's bdr_ws working set will be used since none is 
     * provided for us.
     */
    sws = &(sc->bdr_sws);

    /*
     * Set-up the "tmp_ws."
     */
    sc_setup_ws(sc, sws, targid, 0L);
    SC_ADD_FUNC(sws, module);
    sws->flags = SZ_DEVRS_INPROG;
    sws->ccb = (CCB_SCSIIO *)NULL;

    /*
     * Don't allow a command phase to occure.  This is done by setting our
     * state so that it will appear that a commnd phase has already been
     * performed.
     */
/*
    SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);
*/

    /*
     * Don't allow disconnects.
     */
    t_flags = sws->nexus->flags;
    sws->nexus->flags |= SZ_NO_DISCON;

    /*
     * Call sc_gen_msg() with an action of SC_MSG_IDENTIFY.
     */
/*
    sc_gen_msg(sws, SC_MSG_IDENTIFY);
*/
    sws->nexus->flags = t_flags;

    /*
     * Put SCSI_BUS_DEVICE_RESET on the message out queue.
     */
/*
    SC_ADD_MSGOUT(sws, SCSI_BUS_DEVICE_RESET);
*/

    /*
     * Set the device's bit in the device_reset_inprog field of the
     * SIM_SOFTC and clear it in the device_reset_needed field.
     */
    sc->device_reset_inprog |= 1<<targid;
    sc->device_reset_needed &= ~(1<<targid);

    /*
     * Call the HBA specific send message function,
     * hba_sel_msgout().
     */
/*
    status = SC_HBA_SEL_MSGOUT(sc, sws);
*/
    status = skz_reset_device ( sws );

    /*
     * If the bus device reset failed to start, reschedule it.
     */
    if (status != (U32)CAM_REQ_CMP) {
	SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		   ("end, unable to start\n"));
	CAM_ERROR(module,
		  "Bus device reset is being rescheduled",
		  SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
	ss_resched_request(sws);

	return(CAM_SCSI_BUSY);
    }

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}


/*
 * Routine Name : skz_ss_abort
 *
 * Functional Description :
 *	Ss_perform_abort() is called by ss_sched() at high IPL
 *	to perform a SCSI abort operation.  The waiting NEXUS
 *	queues will be searched for a SIM_WS to abort.
 *
 * Call Syntax:
 *	status = ss_perform_abort(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller.
 *
 * Return Value:  CAM status of CAM_REQ_CMP, CAM_PROVIDE_FAIL, or
 *                CAM_SCSI_BUSY.
 */
U32
skz_ss_abort(sc)
register SIM_SOFTC *sc;
{
    U32 status, t_flags;
    register SIM_WS *sws = (SIM_WS *)NULL;
    SIM_MODULE(skz_ss_abort);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_FLOW,
	       ("begin - sc=0x%x, abort_pend_cnt is: 0x%x\n",
		sc, sc->abort_pend_cnt));

    /*
     * Determine a request which needs to be aborted.
     */
    sws = (SIM_WS *)ss_find_flagged_request(sc, SZ_ABORT_NEEDED);

    /*
     * If we didn't find a request to abort, return now.
     */
    if (sws == (SIM_WS *)NULL) {
	SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		   ("end, failed, abort_pend_cnt %d\n", sc->abort_pend_cnt));
	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Don't allow disconnects.
     */
    t_flags = sws->nexus->flags;
    sws->nexus->flags |= SZ_NO_DISCON;

    /*
     * Call sc_gen_msg() specifying SC_MSG_IDENTIFY.
     */
/*
    sc_gen_msg(sws, SC_MSG_IDENTIFY);
*/
    sws->nexus->flags = t_flags;

    /*
     * Determine if the request was tagged.
     */
    if (sws->flags & SZ_TAGGED) {
	/*
	 * If tagged perform the following:
	 *	Put a SCSI_SIMPLE_QUEUE_TAG on the message out queue.
	 *	Put the tag on the message out queue.
	 *	Put SCSI_ABORT_TAG on the message out queue.
	 */
/*
	SC_ADD_MSGOUT(sws, SCSI_SIMPLE_QUEUE_TAG);
	SC_ADD_MSGOUT(sws, sws->tag);
	SC_ADD_MSGOUT(sws, SCSI_ABORT_TAG);
*/
	sws->flags |= SZ_ABORT_TAG_INPROG;
    }
    else {
	/*
	 * If request is not tagged, perform the following:
	 *	Put SCSI_ABORT on the message out queue.
	 */
/*
	SC_ADD_MSGOUT(sws, SCSI_ABORT);
*/
	sws->flags |= SZ_ABORT_INPROG;
    }

    /*
     * Adjust the abort related counts and flags in the SIM_SOFTC,
     * NEXUS, and SIM_WS.
     */
    sc->abort_pend_cnt--;
    sws->nexus->abort_pend_cnt--;

    /*
     * After the abort message is sent a bus free is expected.
     */
    sws->flags |= SZ_EXP_BUS_FREE;

    /*
     * Clear the SZ_ABORT_NEEDED flag.
     */
    sws->flags &= ~SZ_ABORT_NEEDED;

    /*
     * Call the HBA specific send message function, hba_sel_msgout().
     */
/*
    status = SC_HBA_SEL_MSGOUT(sc, sws);
*/
    status = skz_abort_command ( sc, sws, NULL );

    /*
     * If the abort failed to start, reschedule it.
     */
    if (status != CAM_REQ_CMP) {

	ss_resched_request(sws);

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end, failed, abort_pend_cnt %d\n",
		    sc->abort_pend_cnt));

	return(CAM_SCSI_BUSY);
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}



/*
 * Routine Name : skz_ss_termio
 *
 * Functional Description :
 *	Ss_perform_termio() is called by ss_sched() at high IPL
 *	to perform a SCSI terminate I/O operation.  Search through
 *	the waiting NEXUS queues for a SIM_WS which needs to be
 *	terminated.
 *
 * Call Syntax:
 *	status = ss_perform_termio(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller.
 *
 * Return Value:  CAM status of CAM_REQ_CMP, CAM_PROVIDE_FAIL, or
 *                CAM_SCSI_BUSY.
 */
U32
skz_ss_termio(sc)
register SIM_SOFTC *sc;
{
    U32 status, t_flags;
    register SIM_WS *sws = (SIM_WS *)NULL;
    SIM_MODULE(skz_ss_termio);
    
    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x, termio_pend_cnt is 0x%x\n",
		sc, sc->termio_pend_cnt));

    /*
     * Determine a request which needs to be terminated.
     */
    sws = (SIM_WS *)ss_find_flagged_request(sc, SZ_TERMIO_NEEDED);

    /*
     * Did we find a request to terminate?  If not, return now.
     */
    if (sws == (SIM_WS *)NULL) {
	SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		   ("end, failed\n"));
	return(CAM_PROVIDE_FAIL);
    }

    /*
     * Don't allow disconnects.
     */
    t_flags = sws->nexus->flags;
    sws->nexus->flags |= SZ_NO_DISCON;

    /*
     * Call sc_gen_msg() specifying an action of SC_MSG_IDENTIFY.
     */
/*
    sc_gen_msg(sws, SC_MSG_IDENTIFY);
*/
    sws->nexus->flags = t_flags;

    /*
     * Determine if a the request is tagged.
     */
/*
    if (sws->flags & SZ_TAGGED) {
*/
	/*
	 * If so, call sc_gen_msg() specifying SC_MSG_TAG.
	 */
/*
	sc_gen_msg(sws, SC_MSG_TAG);
    }
*/

    /*
     * Put the SCSI_TERMINATE_IO_PROCESS message on the message out
     * queue.
     */
/*
    SC_ADD_MSGOUT(sws, SCSI_TERMINATE_IO_PROCESS);
*/

    /*
     * Adjust the termio related flags and counts in the SIM_SOFTC,
     * NEXUS, and SIM_WS.
     */
    sc->termio_pend_cnt--;
    sws->nexus->termio_pend_cnt--;
    sws->flags |= SZ_TERMIO_INPROG;
    sws->flags &= ~SZ_TERMIO_NEEDED;

    /*
     * Call the HBA specific send message function, hba_sel_msgout().
     */
/*
    status = SC_HBA_SEL_MSGOUT(sc, sws);
*/
    status = skz_abort_command ( sc, sws, 1 );

    /*
     * The terminate failed, reschedule it.
     */
    if (status != CAM_REQ_CMP) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("end, FAILED\n"));
	ss_resched_request(sws);
	return(CAM_SCSI_BUSY);
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : ss_find_flagged_request
 *
 * Functional Description :
 *	The function ss_find_flagged_request() is responsible for
 *	searching through the target/lun NEXUS queues looking for
 *	a request which has the specified "flags" bits set in the SIM_WS.
 *
 *	This function is called by ss_perform_abort() and by
 *	ss_perform_termio() with IPL raised and SIM_SOFTC SMP locked.
 *
 * Call Syntax :
 *	sws = ss_find_flagged_request(sc, flags)
 *
 * Arguments :
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller.
 *	U32 flags	-- SIM_WS "flags" bits which must be set to 
 *			   make a match.
 *
 * Return Value :
 *	SIM_WS *, NULL if no inactive request is found.
 */
static SIM_WS *
ss_find_flagged_request(sc, flags)
register SIM_SOFTC *sc;
U32 flags;
{
    register NEXUS *nexus = sc->next_nexus;
    register SIM_WS *sws;
    int i;
    SIM_MODULE(ss_find_flagged_request);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin sc=0x%x\n", sc));

    /*
     * The last thing that is done in the loop is to decrement the
     * target/lun pair.  This is done in the for loop control.
     */
    for (i=0; i < sc->nexus_cnt; i++, nexus = nexus->nexus_flink) {

	/*
	 * If no requests are present in the current list,
	 * move on to the next list.
	 */
	if (nexus->flink == (SIM_WS *)nexus)
	    continue;

	sws = nexus->flink;

	/*
	 * Search through the NEXUS queues looking for a request
	 * with the specified "flags" bit(s) set.
	 */
	do {
	    /*
	     * If we find a request with the specified "flags" bit(s)
	     * set, return.
	     */
	    if ((sws->flags & flags) == flags) {
		SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
			   ("end, found, flags is 0x%x:\n", sws->flags));
		return(sws);
	    }

	} while ((sws = sws->flink) != (SIM_WS *)nexus);
    }

    /*
     * An appropriate request couldn't be found.
     */
    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_FLOW,
	       ("end, no request found\n"));

    return ((SIM_WS *)NULL);
}
