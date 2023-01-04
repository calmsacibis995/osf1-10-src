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
/*
 * @(#)$RCSfile: cma_alert.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:43:04 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Implement CMA alert services.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	14 September 1989
 *
 *  MODIFICATION HISTORY:
 * 	001	Bob Conti	6 October 1989
 *	    	Add in the code to raise cma_s_alerted. 	
 *	002	Dave Butenhof	12 October 1989
 *		Use internal mutex operations on tcb->mutex.
 *	003	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *	004	Webb Scales	19 October 1989
 *		Include cma_stack.h for cma__get_self_tcb() and type-cast
 *		cma_c_null_ptr
 *	005	Dave Butenhof	27 October 1989
 *		Make cma___attempt_delivery externally visible.
 *	006	Dave Butenhof	All Saints Day 1989
 *		Make use of cma__enter_kernel instead of manually whiling on
 *		test-and-set.
 *	007	Bob Conti	4 November 1989
 *		Remove superfluous include of cma_host
 *	008	Dave Butenhof	17 November 1989
 *		Include cma_condition.h explicitly, since cma_tcb.h no longer
 *		does it.
 *	009	Dave Butenhof	30 November 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	010	Dave Butenhof	5 December 1989
 *		Tweak some names.
 *	011	Dave Butenhof	11 December 1989
 *		Ignore "disable" bit for attempt_delivery: disable really
 *		controls asynchronous alerts, and we're dealing with the
 *		synchronous variety!
 *	012	Dave Butenhof	12 December 1989
 *		Don't enter kernel for cma_thread_alert.  It increments a
 *		semaphore... which will go into kernel mode.  The fact that
 *		we are already in kernel means the increment is deferred,
 *		which is unnecessary.
 *	013	Dave Butenhof	14 December 1989
 *		Include cma_semaphore.h (nobody noticed the omission before
 *		since it only called a function: now unblock_all_waiting has
 *		been made a macro).
 *	014	Dave Butenhof	01 March 1990
 *		Early versions of the CMA architecture specified that
 *		cma_thread_alert should ignore a null handle.  This is wrong;
 *		the only operations which should do that are those which
 *		clear the handle (to make them idempotent).  The new version
 *		of the spec corrects this problem; so I'm fixing the code,
 *		too.
 *	015	Dave Butenhof	01 May 1990
 *		Fix error in implementation of the architected behavior of
 *		"cma___attempt_delivery".  It should unconditionally
 *		*disable* (asynch) delivery before raising the alert
 *		exception, to restore default operation. Instead, it *defers*
 *		(all) delivery.
 *	016	Dave Butenhof and Webb Scales	03 August 1990
 *		Change semaphore usage.
 *	017	Dave Butenhof	15 August 1990
 *		Add new function cma__asynch_delivery, used by assembler code
 *		to call cma__attempt_delivery with appropriate TCB (since
 *		the assembler code doesn't have access to the macros
 *		specifying how to get the current TCB).
 *	018	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	019	Dave Butenhof	7 February 1991
 *		Implement new rationalized alert control primitives, allowing
 *		scoped "general disable," "general enable," and "asynchronous
 *		disable."  Asynchronous enable must still be done inside an
 *		asynchronous disable scope, since it cannot reliably return a
 *		previous state.
 *	020	Dave Butenhof	20 February 1991
 *		Add explicit casts to pointer assignments.
 *	021	Dave Butenhof	11 April 1991
 *		For non-uniprocessor implementations, use VP interrupt to
 *		deliver async alert.
 *	022	Paul Curtin	 8 May 1991
 *		cma_alert_test now uses the cma__int_alert_test macro.
 *	023	Dave Butenhof	04 October 1991
 *		Correct decision on whether to use cma__vp_interrupt(); it's
 *		not just "!_CMA_UNIPROCESSOR_".
 *	024	Dave Butenhof	01 September 1992
 *		Remove semaphore references. Also, improve efficiency by
 *		avoiding necessity of locking thread's mutex to manipulate
 *		alert state. "alert_pending" is now an atomic variable, and
 *		sending an alert relies on the alerted thread to test its
 *		alert state via attempt_delivery.
 *	025	Dave Butenhof	17 September 1992
 *		When I changed alert_pending to be an atomic, I neglected to
 *		declare an alert by CLEARING the bit. Sigh.
 *	026	Dave Butenhof	21 September 1992
 *		Fix bug in cma__attempt_delivery. If it finds a pending
 *		alert, but can't deliver due to g_enable being off, "re-pend"
 *		the alert so it can be detected later.
 *	027	Dave Butenhof	30 October 1992
 *		Create a cma___attempt_async function, which is a version of
 *		cma__attempt_delivery that expects to be run via
 *		vp_interrupt, asynchronously within the target thread. It
 *		checks for asynch enable, and whether the thread is waiting
 *		on a condition variable (in the latter case it'll just return
 *		and let CV wait exit code handle it).
 *	028	Dave Butenhof	 4 November 1992
 *		Go back to testing target thread's alert state before
 *		interrupting it: cma__do_interrupt can't be completely
 *		reliable on RISC systems (MIPS or AXP) without an absurd
 *		amount of complication. Instead, just "cheat" and add an
 *		interface to machine-specific memory barrier control for MP
 *		systems (currently not needed anyway), so one thread can
 *		reliably READ another thread's state.
 *	029	Dave Butenhof	 5 January 1993
 *		vp_interrupt returns status, not boolean.
 *	030	Dave Butenhof	29 March 1993
 *		As part of integrating March 10 review of cma_condition.c, I
 *		added cma__cvundefer() macro to replace most instances of the
 *		undefer loop. Use the new macro here.
 *	031	Dave Butenhof	12 April 1993
 *		Remove duplicate CMS history from module.
 *	032	Dave Butenhof	21 May 1993
 *		cma__cv_wakeup() has been redesigned to release the CV lock
 *		on DEC OSF/1 before actually doing wakes. On other
 *		platforms, the lock can't be released until after, so there's
 *		a new cma__cvunlockwake() macro that's null on DEC OSF/1.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_alert.h>
#include <cma_condition.h>
#include <cma_dispatch.h>
#include <cma_errors.h>
#include <cma_tcb.h>
#include <cma_handle.h>
#include <cma_mutex.h>
#include <cma_stack.h>
#include <cma_vp.h>

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

static void
cma___attempt_async _CMA_PROTOTYPE_ ((cma__t_int_tcb *tcb));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Disable asynchronous alert delivery.
 *
 *  FORMAL PARAMETERS:
 *
 *	prior		Return previous state of deferral
 *
 *  IMPLICIT INPUTS:
 *
 *	TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Thread's alert state
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_alert_disable_asynch
#ifdef _CMA_PROTO_
	(
	cma_t_alert_state	*ep)	/* Previous state */
#else	/* no prototypes */
	(ep)
	cma_t_alert_state	*ep;	/* Previous state */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread TCB */
    cma__t_int_alert	*prior = (cma__t_int_alert *)ep;


    tcb = cma__get_self_tcb ();
    *prior = tcb->alert;		/* Save current alert state */
    tcb->alert.count++;			/* Increment nesting count */
    tcb->alert.a_enable = cma_c_false;
    cma__mp_barrier ();
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Disable general alert processing until further notice (allow "atomic"
 * 	operations).
 *
 *  FORMAL PARAMETERS:
 *
 *	prior		Return previous state of general alertability
 *
 *  IMPLICIT INPUTS:
 *
 *	TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Thread's alert state
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_alert_disable_general
#ifdef _CMA_PROTO_
	(
	cma_t_alert_state	*ep)	/* Previous state */
#else	/* no prototypes */
	(ep)
	cma_t_alert_state	*ep;	/* Previous state */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread TCB */
    cma__t_int_alert	*prior = (cma__t_int_alert *)ep;


    tcb = cma__get_self_tcb ();
    *prior = tcb->alert;		/* Save current alert state */
    tcb->alert.count++;			/* Increment nesting count */
    tcb->alert.g_enable = cma_c_false;
    cma__mp_barrier ();
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Enable asynchronous alerts.  This should be done within a section of
 *	code bracketed by cma_alert_disable_asynch and cma_alert_restore calls,
 *	since enable cannot return a prior state reliably (it must
 *	enable asynchronous delivery before it returns, so return value
 *	linkages may not be completed).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Thread's alert state
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_alert_enable_asynch
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread TCB */


    tcb = cma__get_self_tcb ();
    tcb->alert.a_enable = cma_c_true;
    cma__mp_barrier ();
    cma__attempt_delivery (tcb);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Enable general alert processing until further notice.
 *
 *  FORMAL PARAMETERS:
 *
 *	prior		Return previous state of general alertability
 *
 *  IMPLICIT INPUTS:
 *
 *	TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Thread's alert state
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_alert_enable_general
#ifdef _CMA_PROTO_
	(
	cma_t_alert_state	*ep)	/* Previous state */
#else	/* no prototypes */
	(ep)
	cma_t_alert_state	*ep;	/* Previous state */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread TCB */
    cma__t_int_alert	*prior = (cma__t_int_alert *)ep;


    tcb = cma__get_self_tcb ();
    *prior = tcb->alert;		/* Save current alert state */
    tcb->alert.count++;			/* Increment nesting count */
    tcb->alert.g_enable = cma_c_true;
    cma__mp_barrier ();
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Restore previous state of alert delivery.
 *
 *  FORMAL PARAMETERS:
 *
 *	prior		Previous state of delivery
 *
 *  IMPLICIT INPUTS:
 *
 *	TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Thread's alert state
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_alert_restore
#ifdef _CMA_PROTO_
	(
	cma_t_alert_state	*ep)	/* Previous state */
#else	/* no prototypes */
	(ep)
	cma_t_alert_state	*ep;	/* Previous state */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread TCB */
    cma__t_int_alert	*prior = (cma__t_int_alert *)ep;


    tcb = cma__get_self_tcb ();

    /*
     * Compare the nesting counts in the prior state and the current TCB.
     * There are three separate cases:
     *
     * 1) If the counts are equal, then this scope has already been restored;
     *    the call is ignored (this supports idempotency).
     * 2) If the prior count is lower than the TCB's count, then this is an
     *    outer scope; restore it and set the nesting count to the prior
     * 	  value (note that this allows intermediate scopes to be skipped).
     * 3) If the prior count is higher than the TCB's count, this is a
     *    nesting violation; raise the appropriate exception.
     */
    if (tcb->alert.count != prior->count) {

	if (prior->count < tcb->alert.count) {
	    tcb->alert.count = prior->count;
	    tcb->alert.g_enable = prior->g_enable;
	    tcb->alert.a_enable = prior->a_enable;
	    cma__mp_barrier ();
	    }
	else {
	    cma__error (cma_s_alert_nesting);
	    }

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Test whether alert is pending, and raise cma_s_alerted if so.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_alert_test
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__int_alert_test ();
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Send an alert to another thread...
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		Thread handle for target thread
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Target thread's alert state
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	May cause the target thread to become runnable and preempt the
 *	current thread.
 */
extern void
cma_thread_alert
#ifdef _CMA_PROTO_
	(
	cma_t_thread	*thread)	/* Target thread handle */
#else	/* no prototypes */
	(thread)
	cma_t_thread	*thread;	/* Target thread handle */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;		/* Current thread TCB */
    cma__t_int_cv	*cv;
    cma_t_boolean	cv_waiter = cma_c_false;


    tcb = cma__validate_tcb (thread);
    cma__unset (&tcb->alert_pending);	/* Mark that alert is pending */

    /*
     * If the thread appears to be waiting on a condition variable, broadcast
     * it. Most of cma__int_cond_wake is duplicated here, so that we can lock
     * the CV first, recheck that the thread is still waiting, and then do
     * the actual work -- otherwise, we'd risk a spurious wakeup (which is
     * legal, but a trifle tacky).
     */
    cma__mp_barrier ();			/* Get good look at thread's state */

    if (tcb->alert.g_enable) {		/* If alerts are enabled */
	cv = tcb->wait_cv;

	if (cv != (cma__t_int_cv *)cma_c_null_ptr) {

	    if (!cma__cvtrylock (cv)) {
		cma__t_int_tcb	*wtcb;


		/*
		 * Now that we have the condition locked, see if the thread
		 * is still waiting -- just unlock if not.
		 */
		if (cv != tcb->wait_cv) {
		    cma__cvunlock (cv);
		    }
		else {
		    cv_waiter = cma_c_true;
		    cma__cv_wakeup (cv, cma_c_true);
		    cma__cvunlockwake (cv);
		    cma__cvundefer (cv);
		    }

		}
	    else {
		/*
		 * If the condition was locked, prevent "wakeup-waiting" race
		 * by deferring a broadcast.
		 */
		cma__unset (&cv->defer_bro);
		cma__unset (&cv->defer_sig);
		cma__unset (&cma__g_cv_defer);
		}

	    }

#if _CMA_THREAD_IS_VP_
	else {

	    /*
	     * If a kernel thread isn't currently waiting, and is allowing
	     * asynchronous cancellation, request a VP interrupt to deliver
	     * the alert.
	     */
	    if (tcb->alert.a_enable) {
		cma_t_integer	vpseq;


		/*
		 * Re-validate the TCB just to make sure it hasn't terminated
		 * on us (that would be inconvenient). Use the VP sequence
		 * number (saved BEFORE the revalidation) to ensure
		 * atomicity.
		 */
		vpseq = cma__vp_get_sequence (tcb->sched.processor->vp_id);
		tcb = cma__validate_tcb (thread);

		if (cma__vp_interrupt (
			tcb->sched.processor->vp_id,
			(cma__t_vp_handler)cma___attempt_async,
			(cma_t_address)tcb,
			vpseq) != cma__c_vp_normal)
		    cma__error (cma_s_existence);

		}

	    }
#endif
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Used by macro code to attempt delivery of an alert to the current
 *	thread (the assem module doesn't have access to cma__get_self_tcb,
 *	which may be a C macro in UNIPROCESSOR builds of DECthreads).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__async_delivery
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__attempt_delivery (cma__get_self_tcb ());
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Attempt to deliver any pending alerts for the current thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb	Address of the TCB object.
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__attempt_delivery
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*tcb)		/* TCB to check */
#else	/* no prototypes */
	(tcb)
	cma__t_int_tcb	*tcb;		/* TCB to check */
#endif	/* prototype */
    {

    /*
     * Test whether alert is pending and we're allowed to raise it.
     *
     * NOTE: asynchronous alert delivery is disabled during alert delivery
     */
    if (!cma__test_and_set (&tcb->alert_pending)) {

	if (tcb->alert.g_enable) {
	    tcb->alert.a_enable = cma_c_false;
	    cma__error (cma_s_alerted);
	    }
	else
	    cma__unset (&tcb->alert_pending);	/* Else re-pend for later */

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Attempt to deliver a pending alert for the current thread. This is
 *	called via cma__vp_interrupt ONLY on kernel thread versions, and only
 *	when async alerts are enabled. So it doesn't need to check the state.
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb	Address of the TCB object.
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
static void
cma___attempt_async
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*tcb)		/* TCB to check */
#else	/* no prototypes */
	(tcb)
	cma__t_int_tcb	*tcb;		/* TCB to check */
#endif	/* prototype */
    {
    tcb->alert.a_enable = cma_c_false;
    cma__error (cma_s_alerted);
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ALERT.C */
/*  *17   27-MAY-1993 14:31:49 BUTENHOF "Remove OSF/1 performance glitch" */
/*  *16   16-APR-1993 13:02:16 BUTENHOF "Remove duplicate CMS history" */
/*  *15   29-MAR-1993 13:55:51 BUTENHOF "Use new cma__cvundefer()" */
/*  *14    5-JAN-1993 13:00:08 BUTENHOF "vp_interrupt returns status, not boolean" */
/*  *13    5-NOV-1992 14:23:55 BUTENHOF "Fix async alert" */
/*  *12    2-NOV-1992 13:24:56 BUTENHOF "Handle kthread asynch alert better" */
/*  *11   21-SEP-1992 13:31:08 BUTENHOF "Reset pending if not enabled" */
/*  *10   17-SEP-1992 14:35:19 BUTENHOF "Uh -- we CLEAR alert_pending, not set it" */
/*  *9     2-SEP-1992 16:23:22 BUTENHOF "Remove semaphore references" */
/*  *8    14-OCT-1991 13:37:26 BUTENHOF "Refine/fix use of config symbols" */
/*  *7    10-JUN-1991 18:16:27 SCALES "Add sccs headers for Ultrix" */
/*  *6    29-MAY-1991 16:58:49 BUTENHOF "Fix call to cma__vp_interrupt()" */
/*  *5    10-MAY-1991 11:09:48 CURTIN "added a use of the cma__int_alert_test macro" */
/*  *4    12-APR-1991 23:34:42 BUTENHOF "Use VP interrupt for async alert if possible" */
/*  *3    20-FEB-1991 22:07:50 BUTENHOF "Add casts for pointer assignment" */
/*  *2    12-FEB-1991 01:28:28 BUTENHOF "New alert control primitives" */
/*  *1    12-DEC-1990 21:40:32 BUTENHOF "alerts" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ALERT.C */
