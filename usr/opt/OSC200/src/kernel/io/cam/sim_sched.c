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
static char *rcsid = "@(#)$RCSfile: sim_sched.c,v $ $Revision: 1.1.12.7 $ (DEC) $Date: 1994/01/11 22:10:17 $";
#endif

/* ---------------------------------------------------------------------- */

/* sim_sched.c		Version 1.22			Jan. 10, 1992

	This file contains the functions which compose the CAM
	SIM Scheduler (SIM SCHED) layer.  The SIM SCHED layer performs
	the tasks needed to start, finish, and reschedule CAM requests.

Modification History

	1.22	01/10/92	janet
	ss_resched_request() now correctly reschedules bus device resets,
	aborts, and termio's.  In ss_perform_timeout() set the
	SZ_ERRLOG_CMDTMO flag in the SIM_WS to prevent errors being
	logged.

	1.21	12/10/91	janet
	Modified ss_sched() to try to setup the DME resources after
	starting an I/O.

	1.20	11/20/91	janet
	Added sim_disable_timeout.

	1.19	11/20/91	janet
	Removed some error logging.  Changed a lot of things to be
	registers.

	1.18	11/13/91	janet
	Removed all SC_ADD_FUNC() calls.  Added error logging.

	1.17	10/28/91	janet
	Clear up needed flags in ss_device_reset_done().

	1.16	10/24/91	janet
	Fixed bugs in ss_abort_done(), ss_process_timeouts(), and
	ss_perform_timeout().

	1.15	10/22/91	janet
	o Added SIM_MODULE() to every function.
	o Replaced all PRINTD's with SIM_PRINTD's.

	1.14	09/10/91	janet
	Check for the SZ_ABORT_TAG_INPROG bit in ss_finish().  Clear the
	SZ_UNTAGGED flag in ss_finish().  When starting a request fails,
	update the message out queue.  Added the function ss_abort_done().
	Reorganized the file history.

	1.13	08/13/91	maria
	Clear error_recovery field before calling sx_async().

	1.12	08/12/91	jag
	Corrected some PRINTDs that were printing out invalid [b/t/l] #s.

	1.11	07/31/91	rps
	Changed a PRINTD which could panic.

	1.10	07/20/91	rps
	Added [b/t/l]'s to PRINTD's.

	1.09	07/08/91	janet
	A NEXUS waiting list is now kept and used to keep track and
	schedule I/O's.  Added async callback for BDR.

        1.08    06/28/91        rps
	Changed a PRINTD which could panic.

        1.07	06/24/91	janet
	Modified ss_resched_request() to be called at high IPL and to clear
	any working counts.

	1.06	06/18/91	janet
	Use new state machine structure.

	1.05	06/04/91	janet
	Ignore return value from DME_END()

	1.04	05/29/91	janet
	Sync info is now being kept in the IT_NEXUS structure.

	1.03	03/26/91	janet
	Updated after second code review.

	1.02	02/12/91	janet
	Updated after debugging.

	1.01	10/02/90        janet
	Updated after first review.

	1.00	09/26/90        janet
	Created this document.

*/

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
extern void	scsiisr();
extern void	sc_gen_msg();
extern void	sc_setup_ws();
extern void	sim_logger();
extern U32	sx_get_tag();
extern U32	sx_clear_itl_nexus();
extern U32	sx_command_complete();
extern U32	sx_reset_detected();
extern void	sx_done_device_reset();
extern SIM_SOFTC *softc_directory[];
extern int	hz; /* number of times a second that the clock interrupts */
extern I32	sim_default_timeout;
extern int 	shutting_down;

/* ---------------------------------------------------------------------- */
/* Local declarations.
 */
U32		ss_init();
U32		ss_go();
U32		ss_sched();
U32		ss_start_sm();
U32		ss_start();
void		ss_finish();
void		ss_resched_request();
void		ss_reset_detected();
U32		ss_device_reset();
U32		ss_perform_device_reset();
void		ss_device_reset_done();
U32		ss_abort();
U32 		ss_perform_abort();
U32		ss_terminate_io();
U32		ss_perform_termio();
void		ss_perform_timeout();
void		ss_process_timeouts();
static SIM_WS	*ss_find_request();
static SIM_WS	*ss_find_flagged_request();
static SIM_WS	*ss_find_ccb();
static void	ss_nexus_insert();
void		ss_nexus_delete();
static void (*local_errlog)() = sim_logger; 

/*
 * Sim_disable_timeout is provided as a back door to users who want
 * to disable all command timeouts.  It can only be modified on a
 * running system with a debugger.
 */
int sim_disable_timeout = 0;

/* ---------------------------------------------------------------------- */
/* Initialized and uninitialized data.
 */

/*

File contents:

The file sim_sched.c contains functions which are related to the starting,
completing, and rescheduling of CCB requests.  

Two different "threads" will use these functions.  The "User thread" is
the code flow initiated by the user via the Perf driver.  The
"State machine thread" is initiated by a software interrupt (softnet).

"User thread" mainline functions:

	ss_init()
	ss_go()
	ss_device_reset()
	ss_abort()
	ss_terminate_io()

"State machine thread" mainline functions:

	ss_sched()
	ss_start()
	ss_finish()
	ss_resched_request()
	ss_reset_detected()
	ss_perform_device_reset()
	ss_device_reset_done()
	ss_perform_abort()
	ss_perform_termio()

Support functions for the above mainline functions:

	ss_find_request()
	ss_find_flagged_request()
	ss_find_ccb()
	ss_mod_current()

*/

/*
 * Routine Name : ss_init
 *
 * Functional Description :
 *	Ss_init() will initialize the SIM SCHED related fields
 *	of the SIM_SOFTC.  Called by the SIM XPT layer.
 *
 * Call Syntax:
 *	ss_init(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *
 * Return Value :
 *	CAM status of CAM_REQ_CMP
 */
U32
ss_init(sc)
SIM_SOFTC *sc;
{
    extern U32 sm_queue_sz;
    extern SIM_SM sim_sm;
    extern SIM_SM_DATA sm_data[];
    static U32 sim_sm_init = 0;
    SIM_MODULE(ss_init);

    SIM_PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc ));

    /*
     * Have we already done this?
     */
    if (sc->sims_init != CAM_TRUE) {

	sc->sims_init = CAM_TRUE;

	/*
	 * Initialize the NEXUS waiting list.
	 */
	sc->nexus_list.nexus_flink = &sc->nexus_list;
	sc->nexus_list.nexus_blink = &sc->nexus_list;
	sc->nexus_list.flink = (SIM_WS *)&sc->nexus_list;
	sc->nexus_list.blink = (SIM_WS *)&sc->nexus_list;
	sc->next_nexus = &sc->nexus_list;
	sc->nexus_cnt = 1;

#ifdef SIM_FUNCQ_ON
	CIRQ_INIT_Q(sc->funcq);
	CIRQ_SET_DATA_SZ(sc->funcq, SIM_MAX_FUNCQ);
#endif /* SIM_FUNCQ_ON */

	/*
	 * Schedule the command timeout function.
	 */
	timeout(ss_process_timeouts, (caddr_t)sc, (int)SS_TIMEOUT_PERIOD);
    }

    /*
     * Setup the default DEC SIM scheduler functions.  These can
     * be overwritten by seting them up in the HBA init function.
     */
    sc->sched_start = ss_start;
    sc->sched_run_sm = ss_start_sm;
    sc->sched_abort = ss_perform_abort;
    sc->sched_termio = ss_perform_termio;
    sc->sched_bdr = ss_perform_device_reset;

    /*
     * Initialize the state machine queue.
     */
    if (!sim_sm_init) {
	sim_sm_init = 1;
	sim_sm.sm_active = CAM_FALSE;
	CIRQ_INIT_Q(sim_sm.sm_queue);
	CIRQ_SET_DATA_SZ(sim_sm.sm_queue, sm_queue_sz);
	CIRQ_USE_SEQ(sim_sm.sm_queue);
	sim_sm.sm_data = sm_data;
    }

    return(CAM_REQ_CMP);
}

/*
 * Routine Name : ss_go
 *
 * Functional Description :
 *	Ss_go() will queue a SIM_WS in the NEXUS queue if one is provided.
 *	It will then check to see if the "State machine thread" is running.
 *	If not it will schedule it.
 *
 * Call Syntax:
 *	status = ss_go(sc, sws)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *	SIM_WS *sws	-- SIM working set which should be queued.
 *			   Set to NULL if only starting up the "State
 *			   machine thread."
 *
 * Return Value: CAM status of CAM_REQ_CMP.
 */
U32
ss_go(sc, sws)
register SIM_SOFTC *sc;
register SIM_WS *sws;
{
    int s, s1;
    U32 status = CAM_REQ_INPROG;
    extern SIM_SM sim_sm;
    SIM_MODULE(ss_go);

    SIM_PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sws=0x%x, sc=0x%x\n", sws, sc ));

    /*
     * Raise IPL and SMP lock on SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * If sws is not NULL, add it to the NEXUS queue.  Check its
     * cam_flags for CAM_SIM_QHEAD to determine queue placement.
     */
    if (sws != (SIM_WS *)NULL) {

	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("begin - sws=0x%x, sc=0x%x\n", sws, sc ));

	if (sws->cam_flags & CAM_SIM_QHEAD) {
	    /*
	     * Insert at head of queue.
	     */
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		       ("insert SIM_WS at head of queue, seq_num 0x%x\n",
			sws->seq_num));
	    SC_WS_INSERT((void *)sws, sws->nexus);
	}
	else {

	    /*
	     * If not head of queue, call sc_sws_sort() to insert
	     * the SIM_WS in the NEXUS list.
             */
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
	               ("insert SIM_WS in NEXUS list, seq_num 0x%x\n",
	                sws->seq_num));
	    sc_sws_sort(sws);
	}

	/*
	 * Update the NEXUS waiting list.  If the NEXUS wasn't already
	 * on the waiting list, put it at the end.
	 */
	if (sws->nexus->sws_count++ == 0) {
	    ss_nexus_insert(sc, sws->nexus, sc->next_nexus->nexus_blink);
	}
    }

    /*
     * Start off the state machine.
     */
    SC_SCHED_RUN_SM(sc);

    SIM_SOFTC_UNLOCK(s, sc); 

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));

    return(status);
}

/*
 * Routine Name : ss_sched
 *
 * Functional Description :
 *	The function ss_sched() is responsible for starting a new
 *	request.  The waiting NEXUS queue will be searched for a 
 *	startable request.  This function will be called via the
 *	"State machine thread" (scsiintr).
 *
 * Call Syntax:
 *	status = ss_sched(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *
 * Return Value: None
 */
U32
ss_sched(sc)
register SIM_SOFTC *sc;
{
    int s, s1;
    U32 status;
    register SIM_WS *sws;
    SIM_MODULE(ss_sched);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc ));

    /*
     * Raise IPL and SMP lock on SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Clear the "waiting_io" field of the SIM_SM struct.
     */
    SIM_SM_LOCK(s1, &sim_sm);
    sim_sm.waiting_io &= ~(1 << sc->cntlr);
    SIM_SM_UNLOCK(s1, &sim_sm);

    /*
     * Check the device_reset flag in SIM_SOFTC.  If greater than
     * zero, a SCSI bus device reset needs to be performed.
     */
    if (sc->device_reset_needed) {
	status = SC_SCHED_BDR(sc);
	/*
	 * If the device reset is valid, return.
	 */
	if (status != CAM_PROVIDE_FAIL) {
	    SIM_SOFTC_UNLOCK(s, sc);
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		       ("end, device reset status 0x%x\n", status));
	    return(CAM_REQ_CMP);
	}
    }

    /*
     * Check the abort_pend_cnt in the SIM_SOFTC.  If greater
     * than zero, attempt to abort the request.
     */
    if (sc->abort_pend_cnt) {
	status = SC_SCHED_ABORT(sc);
	/*
	 * If the abort request is valid, return.
	 */
	if (status != CAM_PROVIDE_FAIL) {
	    SIM_SOFTC_UNLOCK(s, sc);
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		       ("end, abort status 0x%x\n", status));
	    return(CAM_REQ_CMP);
	}
    }

    /*
     * Check the termio_pend_cnt in the SIM_SOFTC.  If greater
     * than zero, attempt to terminate the request.
     */
    if (sc->termio_pend_cnt) {
	status = SC_SCHED_TERMIO(sc);
	/*
	 * If the termio request is valid, return.
	 */
	if (status != CAM_PROVIDE_FAIL) {
	    SIM_SOFTC_UNLOCK(s, sc);
	    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		       ("end, termio status 0x%x\n", status));
	    return(CAM_REQ_CMP);
	}
    }

    /*
     * If the bus is already busy, return now.
     */
    if ((sc->flags & SZ_TRYING_SELECT) ||
	(sc->active_io != (SIM_WS *)NULL)) {
	SIM_SOFTC_UNLOCK(s, sc);
	SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		   ("end, bus is already busy\n"));
	return(CAM_SCSI_BUSY);
    }

    /*
     * Call ss_find_request() to get the SIM_WS.
     */
    sws = (SIM_WS *)ss_find_request(sc);

    /*
     * Was an inactive SIM_WS found?
     */
    if (sws != (SIM_WS *)NULL) {

	/*
	 * For sorted SIM_WS's, if this SIM_WS is the head of the
	 * "next_list" update the current and next sort information.
	 */
	if (sws == sws->nexus->next_list) {

	    /*
	     * Move the next_list to the curr_list.
	     */
	    sws->nexus->curr_list = sws->nexus->next_list;
	    sws->nexus->curr_cnt = sws->nexus->next_cnt;
	    sws->nexus->curr_time = sws->nexus->next_time;
	    sws->nexus->next_list = (SIM_WS *)NULL;
	    sws->nexus->next_cnt = 0;
	    sws->nexus->next_time = 0;
	}

	/*
	 * If the request found wasn't tagged, set the SZ_UNTAGGED bit
	 * in the NEXUS "flags" field.
	 */
	if (!(sws->flags & SZ_TAGGED))
	    sws->nexus->flags |= SZ_UNTAGGED;

	/*
	 * Change the status of the SIM_WS to CAM_REQ_INPROG.
	 * The cam_status of the CCB doesn't change at this time.
	 */
	sws->cam_status = CAM_REQ_INPROG;
    }
    else {
	/*
	 * If no request was found, return now.
	 */
	SIM_SOFTC_UNLOCK(s, sc);
	SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
		   ("end, no request found\n"));
	return(CAM_REQ_CMP);
    }

    /*
     * Setup a timeout on this request.
     */
    SC_ENABLE_TMO(sws, ss_perform_timeout, sws, sws->ccb->cam_timeout);

    /*
     * SMP unlock SIM_SOFTC and lower IPL.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Once a request has been isolated, ss_sched() will start it
     * rolling by calling ss_start().
     */
    status = SC_SCHED_START(sc, sws);

    /*
     * If the request couldn't be started, reschedule it.
     */
    if (status != (U32) CAM_REQ_CMP) {
	SIM_SOFTC_LOCK(s, sc);
	ss_resched_request(sws);
	SIM_SOFTC_UNLOCK(s, sc);
    }
   
    SIM_PRINTD(NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("end, status 0x%x\n", status));

    /*
     * Return the status to the caller.
     */
    return(status);
}

/*
 * Routine Name : ss_start
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
ss_start(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    U32 status;
    int s;
    SIM_MODULE(ss_start);

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
    if (sws->data_xfer.segment == (SEGMENT_ELEMENT *)NULL) {

	if (DME_SETUP(sc, sws, (U32) sws->ccb->cam_dxfer_len,
		      (u_char *) sws->ccb->cam_data_ptr,
		      sws->cam_flags, &sws->data_xfer) != CAM_REQ_CMP) {

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end\n"));
	    return(CAM_PROVIDE_FAIL);
	}
    }
    
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
    sc_gen_msg(sws, SC_MSG_START);

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
 * Routine Name : ss_finish
 *
 * Functional Description :
 *	Ss_finish() clears NEXUS and SIM_SOFTC flags associated
 *	with the completing SIM_WS.  It will also remove the SIM_WS
 *	from the NEXUS queue.  This function will be called when a
 *	request completes via the "State machine thread."
 *
 * Call Syntax:
 *	ss_finish(sws)
 *
 * Arguments:
 *	SIM_WS *sws	-- SIM working set which is completing
 *			   its operation.
 *
 * Return Value: None
 */
void
ss_finish(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    int s;
    SIM_MODULE(ss_finish);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin - sws=0x%x, sc=0x%x\n", sws, sc ));

    SC_ADD_FUNC(sws, module);

    /*
     * Raise IPL and SMP lock.
     */
    SIM_SOFTC_LOCK(s, (SIM_SOFTC *)sws->sim_sc);

    /*
     * Check for special cases.  Do one check up front to
     * reduce the normal code flow overhead.
     */
    if (sws->flags & 
	(SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG | SZ_ABORT_NEEDED |
	 SZ_TERMIO_INPROG | SZ_TERMIO_NEEDED | SZ_CMD_TIMEOUT)) {

	/*
	 * Did an abort get scheduled, but not started?
	 * If so, decrement the abort_pend_cnt in the NEXUS
	 * and SIM_SOFTC.
	 */
	if (sws->flags & SZ_ABORT_NEEDED) {
	    sc->abort_pend_cnt--;
	    sws->nexus->abort_pend_cnt--;
	}

	/*
	 * Did a termio get scheduled, but not started?
	 * If so, decrement the termio_pend_cnt in the NEXUS
	 * and SIM_SOFTC.
	 */
	if (sws->flags & SZ_TERMIO_NEEDED) {
	    sc->termio_pend_cnt--;
	    sws->nexus->termio_pend_cnt--;
	}

	/*
	 * Determine if SIM_WS performed a terminate I/O operation.
	 * If so, set the cam_status field to CAM_REQ_TERMIO.
	 */
	if (sws->flags & SZ_TERMIO_INPROG) {
	    sws->cam_status = CAM_REQ_TERMIO;
	}

	/*
	 * Determine if SIM_WS performed an abort operation.  If so,
	 * set the SIM_WS status to CAM_REQ_ABORTED.
	 */
	if (sws->flags & (SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG)) {
	    sws->cam_status = CAM_REQ_ABORTED;
	}
    }

    /*
     * Is this SIM_WS the head of the "curr_list" for sorting?
     */
    if (sws->nexus->curr_list == sws) {
	NEXUS *nexus = sws->nexus;

	/*
	 * If there are still SIM_WS's in the current list
	 * update the "curr_list" pointer.
	 */
	if (--nexus->curr_cnt) {
	    nexus->curr_list = sws->flink;

            /*
             * If this was the oldest SIM_WS in the current list
	     * call sc_set_curr_age_time() to setup the new time.
             */
            if (nexus->curr_time == sws->time_stamp)
                sc_set_curr_age_time(nexus);
	}
    }
     
    /*
     * Is this SIM_WS the head of the "next_list" for sorting?
     */
    else if (sws->nexus->next_list == sws) {
	NEXUS *nexus = sws->nexus;

	/*
	 * If there are still SIM_WS's in the next list
	 * update the "next_list" pointer.
	 */
	if (--nexus->next_cnt) {
	    nexus->next_list = sws->flink;

            /*
             * If this was the oldest SIM_WS in the next list
	     * call sc_set_curr_age_time() to setup the new time.
             */
            if (nexus->next_time == sws->time_stamp)
                sc_set_next_age_time(nexus);
	}
    }
     
    /*
     * Remove the SIM_WS from the NEXUS queue.
     */
    SC_WS_REMOVE(sws);

    /*
     * Clear the SZ_UNTAGGED bit in the NEXUS flags field.  If the
     * request was tagged this bit won't be set, but in the normal case
     * it will be.  Just clear it.
     */
    sws->nexus->flags &= ~SZ_UNTAGGED;

    /*
     * Update the NEXUS waiting list.
     */
    if (--sws->nexus->sws_count == 0) {
	ss_nexus_delete(sc, sws->nexus);
    }

    /*
     * If not returning a cam_status of CAM_REQ_CMP then freeze
     * the NEXUS queue.
     */
    if (sws->cam_status != CAM_REQ_CMP) {
	
	/*
	 * If autosense is in progress, then don't freeze the queue.
	 * That will be taken care of when autosense completes.
	 *
	 * If CAM_SIM_QFRZDIS is set in the CAM flags, don't freeze
	 * the queue.
	 */
	if (!(sws->flags & SZ_AS_INPROG) &&
	    !(sws->cam_flags & CAM_SIM_QFRZDIS)) {

	    /*
	     * Freeze the NEXUS queue.
	     */
	    SC_FREEZE_QUEUE(sws);

	    /*
	     * Set the CAM_SIM_QFRZN status bit in the SIM_WS
	     * cam_status field.
	     */
	    sws->cam_status |= CAM_SIM_QFRZN;
	}
    }

    /*
     * If the cam_flags of the request specified CAM_SIM_QFREEZE and
     * the NEXUS queue is frozen, set the CAM_SIM_QFRZN bit in the
     * cam_status of the SIM_WS.
     */
    if (sws->cam_flags & CAM_SIM_QFREEZE) {
	if (sws->nexus->freeze_count) {
	    sws->cam_status |= CAM_SIM_QFRZN;
	}
    }
    /*
     * SMP unlock and lower IPL.
     */
    SIM_SOFTC_UNLOCK(s, (SIM_SOFTC *)sws->sim_sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, SIM_WS seq_num 0x%x\n", sws->seq_num));
}

/*
 * Routine Name : ss_resched_request
 *
 * Functional Description :
 *	Ss_resched_request() will mark the specified SIM_WS as
 *	inactive.  The SIM scheduler's pointers will be adjusted
 *	so that this request will be the next scheduled.  This
 *	function should be called at high IPL.
 *
 * Call Syntax:
 *	ss_resched_request(sws)
 *
 * Arguments:
 *	SIM_WS *sws	-- SIM working set which needs to be rescheduled.
 *
 * Return Value: None
 */
void
ss_resched_request(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    SIM_MODULE(ss_resched_request);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin - sws=0x%x, sc=0x%x\n", sws, sc ));

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
	       ("adjusting to targid %d, lun %d\n", sws->targid, sws->lun ));

    SC_ADD_FUNC(sws, module);

    /*
     * Increment SIM_WS field "lostarb." 
     */
    ++sws->lostarb;

    /*
     * Clear all working counters and the message queue.
     */
    sws->msgout_sent = 0;
    SC_UPDATE_MSGOUT(sws, SC_GET_MSGOUT_LEN(sws));

    /*
     * Did the request have the queue freeze bit set?  If so,
     * unfreeze the queue so that the request can be restarted
     * at a later time.
     */
    if (sws->cam_flags & CAM_SIM_QFREEZE) {
	SC_UNFREEZE_QUEUE(sws);
    }

    /*
     * Was this an autosense request?
     */
    if (sws->flags & SZ_AS_INPROG) {

	/* Make sure we requeue the request sense command so we get
	 * back to it first when we can.
	 */
	sws->nexus->as_simws_ptr = sws;
    }

    /*
     * Was a Bus Device Reset to be performed?
     */
    else if (sws->flags & SZ_DEVRS_INPROG) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("request was a device reset\n"));
	sc->device_reset_inprog &= ~(1<<sws->targid);
	sc->device_reset_needed |= 1<<sws->targid;

	/*
	 * Clear the SZ_DEVRS_INPROG bit.
	 */
	sws->flags &= ~SZ_DEVRS_INPROG;
    }

    /*
     * Was the request to be aborted?
     */
    else if (sws->flags & (SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG)) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("request was to be aborted\n"));

	/*
	 * Increment the abort counts in the SIM_SOFTC and NEXUS.
	 */
	((SIM_SOFTC *)sws->sim_sc)->abort_pend_cnt++;
	sws->nexus->abort_pend_cnt++;

	/*
	 * Update the SIM_WS flags to indicate that the abort hasn't
	 * been begun but still needs to be performed.
	 */
	sws->flags |= SZ_ABORT_NEEDED;
	sws->flags &= 
	    ~(SZ_ABORT_INPROG | SZ_ABORT_TAG_INPROG | SZ_EXP_BUS_FREE);
    }

    /*
     * Was a Terminate I/O to be performed?
     */
    else if (sws->flags & SZ_TERMIO_INPROG) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("request was to be terminated\n"));
	/*
	 * Increment the SIM_SOFTC and NEXUS counts.
	 */
	((SIM_SOFTC *)sws->sim_sc)->termio_pend_cnt++;
	sws->nexus->termio_pend_cnt++;
	/*
	 * Update the SIM_WS's flags.
	 */
	sws->flags |= SZ_TERMIO_NEEDED;
	sws->flags &= ~SZ_TERMIO_INPROG;
    }

    /*
     * If not an abort or terminate I/O request, change the
     * cam_status of the request to CAM_CDB_RECVD.
     */
    else {

	/*
	 * Adjust the SIM_SCHED next nexus pointer so this request will be
	 * the nexted scheduled.
	 */
	sc->next_nexus = sws->nexus;

	/*
	 * Clear the SZ_UNTAGGED bit in the NEXUS "flags" field.
	 */
	sws->nexus->flags &= ~SZ_UNTAGGED;

	/*
	 * Disable the timeout on this request.
	 */
	SC_DISABLE_TMO(sws);

	sws->cam_status = CAM_CDB_RECVD;
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : ss_reset_detected
 *
 * Functional Description :
 *	Ss_reset_detected() is called directly from the "State machine
 *	thread" when a SCSI bus reset is detected.  It is responsible
 *	for cleaning up all SIM_SOFTC and NEXUS flags, counts, and
 *	pointers.  It will then call sx_reset_detected() which will
 *	clear out all requests on the NEXUS queues.
 *
 * Call Syntax:
 *	ss_reset_detected(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc -	SIM_SOFTC for controller which the reset
 *			was detected on.
 *
 * Return Value: None
 */
void
ss_reset_detected(sc)
register SIM_SOFTC *sc;
{
    u_short targid, lun;
    int s;
    SIM_MODULE(ss_reset_detected);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc ));

    /*
     * SMP lock.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Clear all NEXUS flags.
     * Clear all IT_NEXUS flags, sync offset, sync period.
     * Version 1.04
     */
    for (targid = 0; targid < NDPS; targid++) {
	for (lun = 0; lun < NLPT; lun++) {
	    /*
	     * Don't clear any target mode bits after receiving a reset.
	     */
	    sc->nexus[targid][lun].flags &= (SZ_PROCESSOR | SZ_TARG_DEF);
	    sc->nexus[targid][lun].termio_pend_cnt = 0;
	    sc->nexus[targid][lun].abort_pend_cnt = 0;
	    sc->nexus[targid][lun].curr_list = (SIM_WS *)NULL;
	    sc->nexus[targid][lun].next_list = (SIM_WS *)NULL;
	    sc->nexus[targid][lun].curr_cnt = 0;
	    sc->nexus[targid][lun].next_cnt = 0;
	}

	sc->it_nexus[targid].flags = 0;
	sc->it_nexus[targid].sync_offset = 0;
	sc->it_nexus[targid].sync_period = 0;
    }

    /*
     * Clear and reset all SIM Scheduler and SIM HBA related
     * counts and flags.
     */
    sc->device_reset_needed = 0L;
    sc->device_reset_inprog = 0L;
    sc->abort_pend_cnt = 0;
    sc->termio_pend_cnt = 0;

    /*
     * Clear SIM_SOFTC related flags (except target mode capable) and pointers.
     * Note: this could be moved to the capabilities flags field in the future.
     */
    sc->flags &= SZ_TARG_CAPABLE;
    sc->active_io = (SIM_WS *)NULL;

    /*
     * Clear the flags in the bdr_sws.
     */
    sc->bdr_sws.flags = 0;

    /*
     * SMP unlock.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Call sx_reset_detected() to clear out all requests on the 
     * NEXUS queues.
     */
    (void)sx_reset_detected(sc->cntlr);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : ss_device_reset
 *
 * Function Description :
 *	Ss_device_reset() will request that a SCSI bus device reset be
 *	performed on the specified target.  This function  will be called
 *	by the "User thread" via the SIM XPT layer.
 *
 * Call Syntax:
 *	status = ss_device_reset(sc, targid)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *	U32 targid	-- Target to perform the reset on.
 *
 * Return Value: CAM status, returned from ss_go().
 */
U32
ss_device_reset(sc, targid)
register SIM_SOFTC *sc;
U32 targid;
{
    U32 status;
    int s;
    SIM_MODULE(ss_device_reset);

    SIM_PRINTD(sc->cntlr, targid, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc));

    /*
     * Set the target's bit in the SIM_SOFTC's "device_reset_needed" field.
     */
    SIM_SOFTC_LOCK(s, sc);
    sc->device_reset_needed |= (1 << targid);
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Start the device reset by calling ss_go().
     */
    status = ss_go(sc, (SIM_WS *)NULL);

    SIM_PRINTD(sc->cntlr, targid, NOBTL, CAMD_INOUT,
	       ("end, return status 0x%x\n", status));

    return(status);
}

/*
 * Routine Name : ss_perform_device_reset
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
ss_perform_device_reset(sc)
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
    SC_ADD_PHASE_BIT(sws, SCSI_COMMAND);

    /*
     * Don't allow disconnects.
     */
    t_flags = sws->nexus->flags;
    sws->nexus->flags |= SZ_NO_DISCON;

    /*
     * Call sc_gen_msg() with an action of SC_MSG_IDENTIFY.
     */
    sc_gen_msg(sws, SC_MSG_IDENTIFY);
    sws->nexus->flags = t_flags;

    /*
     * Put SCSI_BUS_DEVICE_RESET on the message out queue.
     */
    SC_ADD_MSGOUT(sws, SCSI_BUS_DEVICE_RESET);

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
    status = SC_HBA_SEL_MSGOUT(sc, sws);

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
 * Routine Name : ss_device_reset_done
 *
 * Functional Description :
 *	Ss_device_reset_done() will notify the SIM XPT layer that the
 *	requested SCSI bus device reset has been completed.  This
 *	function will be called from the "State machine thread."
 *
 * Call Syntax:
 *	ss_device_reset_done(sc, sws)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *      SIM_WS *sws     -- SIM_WS structure for this device.
 *
 * Return Value:  None
 */
void
ss_device_reset_done(sc, sws)
register SIM_SOFTC *sc;
SIM_WS *sws;
{
    U32 targid = sws->targid;
    u_short lun;
    int s;
    SIM_MODULE(ss_device_reset_done);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sc->cntlr, targid, NOBTL, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Did a BDR really get sent?
     *
     * Check for a status phase.
     */
    if (sws->phase_sum & SCSI_PHASEBIT(SCSI_STATUS)) {

	/*
	 * If the scsi status was busy, then the BDR didn't get
	 * performed.
	 */
	if((sws->scsi_status & ~SCSI_STAT_RESERVED) == SCSI_STAT_BUSY) {

	    /*
	     * Clear the status phase from the phase summary.
	     */
	    sws->phase_sum &= ~(SCSI_PHASEBIT(SCSI_STATUS));

	    /*
	     * Reschedule the BDR.
	     */
	    SIM_SOFTC_LOCK(s, sc);
	    ss_resched_request(sws);
	    SIM_SOFTC_UNLOCK(s, sc);

	    return;
	}
    }


    /*
     * Don't log a message if at boot time.
     */
    if (!cam_at_boottime()) {
	CAM_ERROR(module,
		  "Bus device reset has been performed",
		  SIM_LOG_PRISEVERE,
		  sc, sws, NULL);
    }

    /*
     * Clear the SZ_DEVRS_INPROG flag.
     */
    sws->flags &= ~SZ_DEVRS_INPROG;

    SIM_SOFTC_LOCK(s, sc);

    /*
     * Clear out the nexus flags for this target.
     */
    for (lun = 0; lun < NLPT; lun++) {
	sc->nexus[targid][lun].flags = 0;
	sc->abort_pend_cnt -= sc->nexus[targid][lun].abort_pend_cnt;
	sc->termio_pend_cnt -= sc->nexus[targid][lun].termio_pend_cnt;
	sc->nexus[targid][lun].abort_pend_cnt = 0;
	sc->nexus[targid][lun].termio_pend_cnt = 0;
	sc->nexus[targid][lun].curr_list = (SIM_WS *)NULL;
	sc->nexus[targid][lun].next_list = (SIM_WS *)NULL;
	sc->nexus[targid][lun].curr_cnt = 0;
	sc->nexus[targid][lun].next_cnt = 0;
    }
    sc->it_nexus[targid].flags = 0;
    sc->it_nexus[targid].sync_offset = 0;
    sc->it_nexus[targid].sync_period = 0;

    /*
     * Clear the target's bit in the device_reset_needed field
     * of the SIM_SOFTC.
     */
    sc->device_reset_needed &= ~(1 << targid);
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Call sx_clear_itl_nexus with the target identifier and a -1
     * as the LUN.  Sx_clear_itl_nexus will be responsible for
     * clearing all queues associated with the target and LUN's.
     */
    (void)sx_clear_itl_nexus(sc, (I32)targid, -1L,
			     (U32)CAM_BDR_SENT);


    /*
     * Instruct the SIM_XPT layer to perform an async-callback.
     */
    sx_done_device_reset((U32)AC_SENT_BDR, sc->cntlr, targid, -1L); 

    SIM_PRINTD(sc->cntlr, targid, NOBTL, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : ss_abort
 *
 * Functional Description :
 *	Ss_abort() will request that the specified request is aborted
 *	by sending a SCSI abort message sequence to the device.  The
 *	abort request will be queued.  If the request which is being
 *	aborted is tagged, an abort tag message sequence will be used.
 *	This function will be called by the "User thread" via the
 *	SIM XPT layer.
 *
 * Call Syntax:
 *	status = ss_abort(sc, targid, lun, ccb)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *	U32 targid	-- Target identifier which CCB is related with.
 *	U32 lun	-- LUN which CCB is related with.
 *	CCB_SCSIIO *ccb	-- CCB to abort.
 *
 * Return Value : CAM status, CAM_REQ_CMP or returned from ss_go().
 */
U32
ss_abort(sc, targid, lun, ccb)
register SIM_SOFTC *sc;
U32 targid;
U32 lun;
register CCB_SCSIIO *ccb;
{
    int s;
    U32 status;
    register SIM_WS *sws;
    SIM_MODULE(ss_abort);

    SIM_PRINTD(sc->cntlr, targid, lun, CAMD_INOUT,
	       ("begin - sc=0x%x, targid=%d, lun=%d, ccb=0x%x\n", 
		sc, targid, lun, ccb));

    /*
     * Raise IPL and SMP lock on SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Try to find the specified CCB on the NEXUS queue.
     * If not present, return with a status of CAM_UA_ABORT.
     */
    if ((sws = ss_find_ccb(&sc->nexus[targid][lun], ccb)) == (SIM_WS *) NULL) {
	/*
	 * Unlock and lower IPL.
	 */
	SIM_SOFTC_UNLOCK(s, sc);
	SIM_PRINTD(sc->cntlr, targid, lun, CAMD_INOUT,
		   ("end, CCB not in NEXUS queue.\n"));
	return(CAM_UA_ABORT);
    }

    /*
     * Determine if the specified CCB is active.  If inactive then the
     * request hasn't made it out to the device yet.  Set the SZ_ABORT_INPROG
     * bit in the SIM_WS's flag field to inform ss_finish() that this request
     * was aborted.  Set the CAM_REQ_INPROG bit in the SIM_WS to prevent this
     * request from being scheduled after IPL is lowered.
     * Lower IPL and call ss_finish() and sx_command_complete().
     */
    if(sws->cam_status == CAM_CDB_RECVD) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("CCB is inactive\n"));
	sws->cam_status = CAM_REQ_INPROG;
	sws->flags |= SZ_ABORT_INPROG;

	/*
	 * If any DME resources have been allocated, free them.
	 * Resources may have been allocated if this request
	 * had been rescheduled.
	 */
	if (sws->data_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	    (void)DME_END(sc, &sws->data_xfer);

	ss_finish(sws);
	(void)sx_command_complete(sws);
	SIM_SOFTC_UNLOCK(s, sc);
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));
	return(CAM_REQ_CMP);
    }

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
	       ("CCB is active\n"));

    /*
     * Check to see if an abort request has already been made.
     */
    if (!(sws->flags & SZ_ABORT_NEEDED)) {
	/*
	 * Increment the abort_pend_cnt in the SIM_SOFTC and
	 * NEXUS.  Set the SZ_ABORT_NEEDED bit the SIM_WS "flags" field.
	 */
	sc->abort_pend_cnt++;
	sws->nexus->abort_pend_cnt++;
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("abort_pend_cnt is now: %d\n",
		    sws->nexus->abort_pend_cnt));
	sws->flags |= SZ_ABORT_NEEDED;
    }

    /*
     * SMP unlock and lower IPL.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Start the abort request.
     */
    status = ss_go(sc, (SIM_WS *)NULL);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, return status 0x%x\n", status));

    return(status);
}

/*
 * Routine Name : ss_perform_abort
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
ss_perform_abort(sc)
register SIM_SOFTC *sc;
{
    U32 status, t_flags;
    register SIM_WS *sws = (SIM_WS *)NULL;
    SIM_MODULE(ss_perform_abort);

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
    sc_gen_msg(sws, SC_MSG_IDENTIFY);
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
	SC_ADD_MSGOUT(sws, SCSI_SIMPLE_QUEUE_TAG);
	SC_ADD_MSGOUT(sws, sws->tag);
	SC_ADD_MSGOUT(sws, SCSI_ABORT_TAG);
	sws->flags |= SZ_ABORT_TAG_INPROG;
    }
    else {
	/*
	 * If request is not tagged, perform the following:
	 *	Put SCSI_ABORT on the message out queue.
	 */
	SC_ADD_MSGOUT(sws, SCSI_ABORT);
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
    status = SC_HBA_SEL_MSGOUT(sc, sws);

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
 * Routine Name : ss_abort_done
 *
 * Functional Description :
 *	Ss_abort_done() will notify the SIM XPT layer that a
 *	SCSI abort message has been sent to the specified target/lun.
 *
 * Call Syntax:
 *	ss_abort_done(sws)
 *
 * Arguments:
 *	SIM_WS *sws	-- SIM_WS pointer which performed the abort
 *
 * Return Value:  None
 */
void
ss_abort_done(sc, sws)
register SIM_SOFTC *sc;
register SIM_WS *sws;
{
    int s;
    register NEXUS *nexus = sws->nexus;
    register SIM_WS *sws1;
    SIM_MODULE(ss_abort_done);

    SC_ADD_FUNC(sws, module);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));

    /*
     * Did the I/O really get aborted?
     *
     * Check for a status phase.
     */
    if (sws->phase_sum & SCSI_PHASEBIT(SCSI_STATUS)) {

	/*
	 * If the scsi status was busy, then the abort didn't get
	 * performed.
	 */
	if((sws->scsi_status & ~SCSI_STAT_RESERVED) == SCSI_STAT_BUSY) {

	    /*
	     * Clear the status phase from the phase summary.
	     */
	    sws->phase_sum &= ~(SCSI_PHASEBIT(SCSI_STATUS));

	    /*
	     * Reschedule the abort.
	     */
	    SIM_SOFTC_LOCK(s, sc);
	    ss_resched_request(sws);
	    SIM_SOFTC_UNLOCK(s, sc);

	    return;
	}
    }

    if (sws->flags & SZ_TAGGED) {
	CAM_ERROR(module,
	      "SCSI abort tag has been performed",
	      SIM_LOG_PRISEVERE,
	      NULL, sws, NULL);

	/*
	 * Clear this request from the NEXUS.
	 */

	/*
	 * Make sure that the SZ_ABORT_INPROG bit is set in the
	 * "flags" field.  
	 */
	sws->flags |= SZ_ABORT_TAG_INPROG;
	(void) DME_END(sc, &sws->data_xfer);
	ss_finish(sws);
	(void) sx_command_complete(sws);
    } else {
	CAM_ERROR(module,
	      "SCSI abort has been performed",
	      SIM_LOG_PRISEVERE,
	      NULL, sws, NULL);

	/*
	 * Clear all requests which were active on the NEXUS.
	 */
	SIM_SOFTC_LOCK(s, sc);
	sws1 = nexus->flink;
	while (sws1 != (SIM_WS *)nexus) {

	    /*
	     * Is this request in progess?
	     */
	    if (sws1->cam_status != CAM_CDB_RECVD) {

		/*
		 * Make sure that the SZ_ABORT_INPROG bit is set in the
		 * "flags" field.  
		 */
		sws1->flags |= SZ_ABORT_INPROG;
		(void) DME_END(sc, &sws1->data_xfer);
		ss_finish(sws1);
		(void) sx_command_complete(sws1);
		sws1 = nexus->flink;
		continue;
	    }
	    sws1 = sws1->flink;
	}
    }
    SIM_SOFTC_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end\n"));
}

/*
 * Routine Name : ss_terminate_io
 *
 * Functional Description :
 *	Ss_terminate_io() will request that the specified request
 *	be terminated.  If the request is not active, no SCSI bus
 *	activity will be needed to terminate it.  Otherwise, a SCSI
 *	terminate I/O message sequence will be used.  This function
 *	will be called by the "User thread" via the SIM XPT layer.
 *
 * Call Syntax:
 *	status = ss_terminate_io(sc, targid, lun, ccb)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller
 *	U32 targid	-- Target identifier which CCB is related with.
 *	U32 lun	-- LUN which CCB is related with.
 *	CCB_SCSIIO *ccb	-- CCB to terminate.
 *
 * Return Value: CAM status, CAM_REQ_CMP or returned from ss_go().
 */
U32
ss_terminate_io(sc, targid, lun, ccb)
register SIM_SOFTC *sc;
U32 targid;
U32 lun;
register CCB_SCSIIO *ccb;
{
    int s;
    U32 status;
    register SIM_WS *sws;
    SIM_MODULE(ss_terminate_io);

    SIM_PRINTD(sc->cntlr, targid, lun, CAMD_INOUT,
	       ("begin - sc=0x%x, targid=%d, lun=%d, ccb=0x%x\n",
		sc, targid, lun, ccb ));

    /*
     * Raise IPL and SMP lock.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * Try to find the specified CCB on the NEXUS queue.
     * If not present, return with a status of CAM_UA_TERMIO.
     */
    if ((sws = ss_find_ccb(&sc->nexus[targid][lun], ccb)) == (SIM_WS *) NULL) {
	/*
	 * Unlock and lower IPL.
	 */
	SIM_SOFTC_UNLOCK(s, sc);
	SIM_PRINTD(sc->cntlr, targid, lun, CAMD_INOUT,
		   ("end, CCB not in NEXUS queue.\n"));
	return(CAM_UA_TERMIO);
    }

    /*
     * Determine if the specified request is active.  If inactive then the
     * request hasn't made it out to the device yet.  Set the SZ_TERMIO_INPROG
     * bit in the SIM_WS's "flags" field to inform ss_finish() that this
     * request was terminated.  Set the CAM_REQ_INPROG bit in the SIM_WS to
     * prevent this request from being scheduled after IPL is lowered.  Lower
     * IPL and call ss_finish() and sx_command_complete().
     */
    if(sws->cam_status == CAM_CDB_RECVD) {
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("request is inactive\n"));
	sws->cam_status = CAM_REQ_INPROG;
	sws->flags |= SZ_TERMIO_INPROG;

	/*
	 * If any DME resources have been allocated, free them.
	 * Resources may have been allocated if this request
	 * had been rescheduled.
	 */
	if (sws->data_xfer.segment != (SEGMENT_ELEMENT *)NULL)
	    (void)DME_END(sc, &sws->data_xfer);

	ss_finish(sws);
	(void)sx_command_complete(sws);
	SIM_SOFTC_UNLOCK(s, sc);
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		   ("end\n"));
	return(CAM_REQ_CMP);
    }

    /*
     * If an abort is already in progress on this request, disregard
     * the terminate request.
     */
    if (!(sws->flags & (SZ_ABORT_NEEDED | SZ_ABORT_INPROG |
			SZ_ABORT_TAG_INPROG)) &&
	
	/*
	 * Check to see if a termio request has already been made.
	 */
	(!(sws->flags & (SZ_TERMIO_NEEDED | SZ_TERMIO_INPROG)))) {

	/*
	 * Increment the termio_pend_cnt in the SIM_SOFTC and NEXUS.  Set the
	 * SZ_TERMIO_NEEDED bit in the SIM_WS flags field.
	 */
	sc->termio_pend_cnt++;
	sws->nexus->termio_pend_cnt++;
	SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		   ("termio_pend_cnt is now: %d\n",
		    sws->nexus->termio_pend_cnt));
	sws->flags |= SZ_TERMIO_NEEDED;
    }
	
    /*
     * SMP unlock and lower IPL.
     */
    SIM_SOFTC_UNLOCK(s, sc);

    /*
     * Call ss_go().
     */
    status = ss_go(sc, (SIM_WS *)NULL);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("end, return status 0x%x\n", status));

    return(status);
}

/*
 * Routine Name : ss_perform_termio
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
ss_perform_termio(sc)
register SIM_SOFTC *sc;
{
    U32 status, t_flags;
    register SIM_WS *sws = (SIM_WS *)NULL;
    SIM_MODULE(ss_perform_termio);
    
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
    sc_gen_msg(sws, SC_MSG_IDENTIFY);
    sws->nexus->flags = t_flags;

    /*
     * Determine if a the request is tagged.
     */
    if (sws->flags & SZ_TAGGED) {
	/*
	 * If so, call sc_gen_msg() specifying SC_MSG_TAG.
	 */
	sc_gen_msg(sws, SC_MSG_TAG);
    }

    /*
     * Put the SCSI_TERMINATE_IO_PROCESS message on the message out
     * queue.
     */
    SC_ADD_MSGOUT(sws, SCSI_TERMINATE_IO_PROCESS);

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
    status = SC_HBA_SEL_MSGOUT(sc, sws);

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
 * Routine Name : ss_perform_timeout
 *
 * Functional Description :
 *	Ss_perform_timeout() is called by the "command timeout"
 *	thread when a SIM_WS has been isolated which needs to
 *	be timed out.  This function will initiate an abort
 *	on this CCB.  It will also schedule a backup timeout
 *	which will perform a SCSI bus reset if it expires and
 *	the request is on the bus.
 *
 * Call Syntax:
 *	ss_perform_timeout(sws)
 *
 * Arguments:
 *	SIM_WS *sws	-- SIM_WS which needs to be timed out
 *
 * Return Value:  None
 */
void
ss_perform_timeout(sws)
register SIM_WS *sws;
{
    register SIM_SOFTC *sc = (SIM_SOFTC *)sws->sim_sc;
    I32 next_timeout = SIM_DEFAULT_TIMEOUT;
    int s;
    SIM_MODULE(ss_perform_timeout);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin - sws=0x%x, sc=0x%x\n", sws, sc ));

    SIM_SOFTC_LOCK(s, sc);

    SC_ADD_FUNC(sws, module);

    /*
     * Increment the abort count associated with this I/O.
     */
    sws->abort_cnt += 1;

    /*
     * Set the ERR_TIMEOUT bit in the SIM_WS's error_recovery field.
     * This will force the State Machine to use the error recovery
     * functions when processing this SIM_WS.
     */
    sws->error_recovery |= ERR_TIMEOUT;

    /* Set the SZ_CMD_TIMEOUT in SIM_WS flag, so we know that timeout has occurred */
    sws->flags |= SZ_CMD_TIMEOUT;

    /* 
     * Determine if the request which has been timed-out is currently
     * on the bus.  If so, setup a secondary timeout which will perform
     * a SCSI bus reset.  If the request has disconnected, setup a
     * secondary timeout which will call this routine again.
     */
    if (sc->active_io == sws) {
	CAM_ERROR(module,
		  "timeout on request on the bus, scheduled bus reset",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	SC_ENABLE_TMO(sws, sc->hba_bus_reset, sc, next_timeout);
    }
    else if (sws->abort_cnt >= SIM_MAX_ABORT_CNT) {
	CAM_ERROR(module,
		  "Reached max abort count, scheduled bus reset",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	SC_ENABLE_TMO(sws, sc->hba_bus_reset, sc, next_timeout);
    }
    else {
	CAM_ERROR(module,
		  "timeout on disconnected request",
		  SIM_LOG_SIM_WS|SIM_LOG_PRISEVERE,
		  NULL, sws, NULL);
	sws->errors_logged |= SZ_ERRLOG_CMDTMO;
	SC_ENABLE_TMO(sws, ss_perform_timeout, sws, next_timeout);
    }

    /*
     * Schedule an abort on this request.
     */
    ss_abort(sc, sws->targid, sws->lun, sws->ccb);

    SIM_SOFTC_UNLOCK(s, sc);

    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
	       ("begin\n"));
}

/*
 * Routine Name : ss_process_timeouts
 *
 * Functional Description :
 *	Ss_process_timeouts() will traverse the NEXUS queues looking
 *	for any SIM_WS's which need to be timed out.  If a SIM_WS
 *	needs to be timed out, the timeout function specified in
 *	the SIM_WS will be called.
 *
 *	The last thing that this function will do is to reschedule
 *	its self via timeout().
 *
 * Call Syntax:
 *	ss_process_timeouts(sc)
 *
 * Arguments:
 *	SIM_SOFTC *sc
 *
 * Return Value:  None
 */
void
ss_process_timeouts(sc)
register SIM_SOFTC *sc;
{
    register I32 targid, lun;
    register NEXUS *nexus;
    register SIM_WS *sws;
    int s;
    SIM_MODULE(ss_process_timeouts);

    SIM_PRINTD( NOBTL, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc));
    /*
     * Lock on the SIM_SOFTC.
     */
    SIM_SOFTC_LOCK(s, sc);

    /*
     * If we are recovering from a bus reset, dont perform any timeouts.
     */
    if (sc->error_recovery & ERR_BUS_RESET) {

	/*
	 * Schedule another timeout.
	 */
	timeout(ss_process_timeouts, (caddr_t)sc, (int)SS_TIMEOUT_PERIOD);

	/*
	 * Unlock the SIM_SOFTC.
	 */
	SIM_SOFTC_UNLOCK(s, sc);

	return;
    }
    
    /*
     * Search through all the target and lun NEXUS queues.
     */
    for (targid=MAX_TARGETS; targid >= 0; --targid) {
	for (lun=MAX_LUNS; lun >= 0; --lun) {

	    /*
	     * Traverse the NEXUS queue and check the timeout time for all
	     * the pending requests.
	     */
	    nexus = SC_GET_NEXUS(sc, targid, lun);
	    sws = nexus->flink;
	    while (sws != (SIM_WS *)nexus) {

		/* 
		 * If the timeout time in the SIM_WS is not zero and the 
		 * current system time (time->tv_sec) is later, timeout
		 * this I/O by calling the timeout function in the SIM_WS.
		 */
		if (sws->time.tv_sec &&
		    (sws->time.tv_sec <= sc_get_time_sec())) {

		    /*
		     * Disable other timeouts.
		     */
		    SC_DISABLE_TMO(sws);

		    /*
		     * Call timeout routine.
		     */		     
		    (sws->tmo_fn)(sws->tmo_arg);
		}

		/*
		 * Move on to the next SIM_WS.
		 */
		sws = sws->flink;
	    }
	}
    }

    /*
     * Schedule another timeout.
     */
    timeout(ss_process_timeouts, (caddr_t)sc, (int)SS_TIMEOUT_PERIOD);

    /*
     * Unlock the SIM_SOFTC.
     */
    SIM_SOFTC_UNLOCK(s, sc);
}

U32
ss_start_sm(sc)
SIM_SOFTC *sc;
{
    extern SIM_SM sim_sm;
    int s;

    /*
     * If the SCSI bus is already busy don't bother scheduling the
     * request.  It will be started as soon as the bus goes free.
     */
    if (sc->active_io == (SIM_WS *)NULL) {

	SIM_SM_LOCK(s, &sim_sm);

	/*
	 * Schedule the starting of the request via the CAM soft interrupt.
	 * If the CAM soft interrupt handler is already running, don't try
	 * to start it.  Set the controller's bit in the state machine's
	 * "waiting_io" field.
	 */
	sim_sm.waiting_io |= (1 << sc->cntlr);
	if (!sim_sm.sm_active || shutting_down) {
	    
	    /*
	     * Schedule the CAM state machine via the softnet
	     * interrupt.
	     */
	    SIM_SCHED_ISR();
	}

	SIM_SM_UNLOCK(s, &sim_sm);
    }
}

/*
 * Routine Name : ss_find_request
 *
 * Functional Description :
 *	The function ss_find_request() is responsible for searching
 *	through the NEXUS active list looking for a request which
 *	is inactive.
 *
 *	A NEXUS list which contains waiting SIM_WS's will be used.
 *	If no SIM_WS can be started, a NULL SIM_WS pointer will be
 *	returned.
 *
 *	In the case of tagged requests, if an active tagged request is
 *	found on the NEXUS queue, the queue will be searched for an inactive
 *	tagged request.  The search of the queue will be terminated if an
 *	untagged request is encountered.
 *
 *	This function is called by ss_sched() with IPL raised and SIM_SOFTC
 *	SMP locked.
 *
 * Call Syntax :
 *	sws = ss_find_request(sc)
 *
 * Arguments :
 *	SIM_SOFTC *sc	-- SIM_SOFTC structure for this controller.
 *
 * Return Value :
 *	SIM_WS *, NULL if no inactive request is found.
 */
static SIM_WS *
ss_find_request(sc)
register SIM_SOFTC *sc;
{
    register NEXUS *nexus = sc->next_nexus;
    register SIM_WS *sws;
    register int i;
    SIM_MODULE(ss_find_request);

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("begin - sc=0x%x\n", sc ));

    for (i=0; i < sc->nexus_cnt; i++, nexus = nexus->nexus_flink) {

	/* Check if there is an autosense ccb waiting (always do that one
	 * first).  If so, clear its pointer, and return that sws.
	 */
	if (sws = nexus->as_simws_ptr) {
	    nexus->as_simws_ptr = 0;
	    return( sws );
	}

	/*
	 * If no requests are present in the current list,
	 * move on to the next list.
	 */
	if ((SIM_WS *)nexus == nexus->flink)
	    continue;

	/*
	 * If the list has been frozen or there is an active untagged 
	 * request or this is a processor nexus, move on to the next list.
	 */
	if ( (nexus->freeze_count) 
            || (nexus->flags & (SZ_UNTAGGED | SZ_PROCESSOR | SZ_TARG_DEF)) )
	    continue;

	sws = nexus->flink;

	/*
	 * Do a check up-front to see if this request is tagged.
	 * This will speed up the normal cases since tagged
	 * requests aren't all that common, at least for now.
	 */
	if (sws->flags & (SZ_TAGGED | SZ_TAG_PENDING)) {

	    /*
	     * Search through the NEXUS queue looking for an 
	     * appropriate tagged request.
	     */
	    do {
		/*
		 * If we find an untagged request, set "sws" to the
		 * head of the NEXUS queue to signify that no requests
		 * were found.  Don't continue the search on this NEXUS.
		 */
		if (!(sws->flags & (SZ_TAGGED | SZ_TAG_PENDING))) {
		    sws = (SIM_WS *)nexus;
		    break;
		}

		/*
		 * If we find an inactive request with "tag pending" set,
		 * try to assign a tag.
		 */
		if (sws->flags & SZ_TAG_PENDING)
		    sx_get_tag(sws);
		
		/*
		 * If the current request is tagged and inactive, break.
		 */
		if ((sws->flags & SZ_TAGGED) &&
		    (sws->cam_status == CAM_CDB_RECVD))
			break;

	    } while ((sws = sws->flink) != (SIM_WS *)nexus);

	    /*
	     * Do we have a valid SIM_WS?  If not, move onto the next NEXUS.
	     */
	    if (sws == (SIM_WS *)nexus)
		continue;
	}

	/*
	 * Have we found an inactive request?  If so, its "cam_status"
	 * will be CAM_CDB_RECVD.
	 */
	if (sws->cam_status == CAM_CDB_RECVD) {
	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_FLOW,
		       ("found: seq_num 0x%x\n", sws->seq_num));

	    SIM_PRINTD(sws->cntlr, sws->targid, sws->lun, CAMD_INOUT,
		       ("end\n"));

	    sc->next_nexus = nexus->nexus_flink;
	    return(sws);
	}
    }

    SIM_PRINTD(sc->cntlr, NOBTL, NOBTL, CAMD_INOUT,
	       ("no requests found\n"));

    return ((SIM_WS *)NULL);
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

/*
 * Routine Name : ss_find_ccb
 *
 * Functional Description :
 *	Ss_find_ccb() will search through the given NEXUS queue
 *	looking for a SIM_WS which contains a pointer to the 
 *	specified CCB.  If one is found, a pointer to it will be
 *	returned.  Otherwise, a NULL pointer will be returned.
 *	This function is called at high IPL and SMP locked by
 *	ss_abort() and ss_terminate_io().
 *
 * Call Syntax:
 *	ss_find_ccb(nexus, ccb)
 *
 * Arguments:
 *	NEXUS *nexus	-- NEXUS queue to search
 *	CCB_SCSIIO *ccb	-- CCB to search for
 *
 * Return Value :
 *	SIM_WS *, SIM_WS which was found or NULL if none found
 */
static SIM_WS *
ss_find_ccb(nexus, ccb)
register NEXUS *nexus;
register CCB_SCSIIO *ccb;
{
    register SIM_WS *sws;
    SIM_MODULE(ss_find_ccb);

    /*
     * Start the search at the head of the queue.
     */
    sws = (SIM_WS *)nexus;

    /*
     * Search through the NEXUS queue looking for the specified
     * ccb.
     */
    while ((sws = sws->flink) != (SIM_WS *)nexus)
	if (sws->ccb == ccb)
	    break;

    /*
     * Did we find a valid SIM_WS?  If not, return NULL.
     */
    if (sws == (SIM_WS *)nexus)
	return ((SIM_WS *) NULL);
    else {
	return (sws);
    }
}

/*
 * Routine Name : ss_nexus_insert
 *
 * Functional Description :
 *	Ss_nexus_insert() will insert the given NEXUS pointer
 *	after the given head pointer.  The SIM_SOFTC's nexus_cnt
 *	will be incremented.
 *
 * Call Syntax:
 *	ss_nexus_insert(sc, n, head)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- Associated SIM_SOFTC pointer
 *	NEXUS *nexus	-- NEXUS to be added
 *	NEXUS *head	-- Head of NEXUS linked list
 *
 * Return Value :  None
 */
static void
ss_nexus_insert(sc, n, head)
register SIM_SOFTC *sc;
register NEXUS *n, *head;
{
    SIM_MODULE(ss_nexus_insert);
    sc->nexus_cnt++;
    n->nexus_flink = head->nexus_flink;
    n->nexus_blink = head;
    head->nexus_flink->nexus_blink = n;
    head->nexus_flink = n;
}

/*
 * Routine Name : ss_nexus_delete
 *
 * Functional Description :
 *	Ss_nexus_delete() will delete the given NEXUS pointer
 *	from the NEXUS list.  The SIM_SOFTC's nexus_cnt
 *	will be decremented.
 *
 * Call Syntax:
 *	ss_nexus_insert(sc, n, head)
 *
 * Arguments:
 *	SIM_SOFTC *sc	-- Associated SIM_SOFTC pointer
 *	NEXUS *nexus	-- NEXUS to be deleted
 *
 * Return Value :  None
 */
void
ss_nexus_delete(sc, n)
register SIM_SOFTC *sc;
register NEXUS *n;
{
    SIM_MODULE(ss_nexus_delete);

    if (sc->next_nexus == n)
        sc->next_nexus = n->nexus_flink;
    sc->nexus_cnt--;
    n->nexus_flink->nexus_blink = n->nexus_blink;
    n->nexus_blink->nexus_flink = n->nexus_flink;
}

