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
static char *rcsid = "@(#)$RCSfile: cma_vp_hdwr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/13 21:37:01 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Implement hardware-specific "virtual processor" functions to help out
 *	VP layer. For Alpha architecture.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	28 July 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	30 October 1992
 *		Modify cma__vp_interrupt to do a direct call if the target is
 *		the current VP (since we might have a little trouble
 *		suspending the current VP and then changing its state :-) ).
 *	002	Dave Butenhof	 3 November 1992
 *		Improve bugcheck reports.
 *	003	Dave Butenhof	 4 November 1992
 *		Simplify vp_interrupt -- don't make any attempt to let the
 *		target thread return from do_interrupt, since it really can't
 *		anyway. Also, provide some checks to ensure that the correct
 *		thread is interrupted.
 *	004	Dave Butenhof	23 November 1992
 *		Clear R26 (ra) for thread's base frame.
 *	005	Dave Butenhof	25 November 1992
 *		Set thread's initial policy/priority for RT4_KTHREAD
 *	006	Dave Butenhof	 3 December 1992
 *		In set_start, if the VP's current policy/priority isn't what
 *		the state array asks for, set what's in the state array, NOT
 *		what's in the VP struct! (oops :-) ).
 *	007	Dave Butenhof	 5 January 1993
 *		Return cma__c_vp_err_badstate for vp_interrupt when target
 *		state/identity has changed.
 *	008	Dave Butenhof	 8 March 1993
 *		thread_abort() the thread_suspend() when setting thread
 *		state.
 *	009	Dave Butenhof	 9 March 1993
 *		Don't thread_abort() a fresh thread.
 *	010	Dave Butenhof	10 March 1993
 *		Try some experiments with thread_abort().
 *	011	Brian Keane	30 March 1993
 *		Remove duplicate thread_abort().  Remove old initialization
 *		of the first few words of the stack, since we don't use
 *		cma__execute_thread anymore.
 *	012	Dave Butenhof	 1 April 1993
 *		Change %08x to 0x%lx.
 *	013	Dave Butenhof	 5 April 1993
 *		Remove 'fresh' state bit, zombie queue, reset pointer, and
 *		thread_abort.
 */

/*
 *  INCLUDE FILES
 */

#if _CMA_KTHREADS_ == _CMA__MACH	/* Module is NULL if not Mach */
#include <cma.h>
#include <cma_defs.h>
#include <cma_thread.h>
#include <cma_vp.h>
#include <cma_vm.h>
#include <cma_assem.h>
#include <mach.h>
#include <mach_error.h>
#include <mach/machine/syscall_sw.h>

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	interrupt a VP
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to interrupt
 *	handler		Address of routine to call in target VP
 *	arg		Argument to pass to interrupt handler
 *	vpseq		Sequence of VP caller wants to interrupt (atomicity)
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
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 *
 */
cma__t_vp_status
cma__vp_interrupt
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid,
	cma__t_vp_handler	handler,
	cma_t_address		arg,
	cma_t_integer		vpseq)
#else	/* no prototypes */
	(vpid, handler, arg, vpseq)
	cma__t_vpid		vpid;
	cma__t_vp_handler	handler;
	cma_t_address		arg;
	cma_t_integer		vpseq;
#endif	/* prototype */
    {
    struct alpha_thread_state	state_array;
    unsigned int		state_count = ALPHA_THREAD_STATE_COUNT;
    kern_return_t		status;


    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_interrupt) interrupting vp %ld to 0x%lx(0x%lx)",
	    vpid->vp,
	    handler,
	    arg));

    if (thread_self () == vpid->vp) {
	(handler) (arg);
	status = cma__c_vp_normal;
	}
    else {
	cma__spinlock (&cma__g_vp_lock);

	/*
	 * First, suspend the target thread
	 */
	if ((status = thread_suspend (vpid->vp)) != KERN_SUCCESS)
	    cma__bugcheck (
		    "vp_interrupt: %s (%ld) thread_suspend(%ld)",
		    mach_error_string (status),
		    status,
		    vpid->vp);

	cma__spinunlock (&cma__g_vp_lock);

	/*
	 * Now that it's suspended, check whether it's REALLY the one we want
	 * to play with.
	 */

	if (!(vpid->flags & cma__c_vp_running) || (vpid->seq != vpseq))
	    status = cma__c_vp_err_badstate;
	else {

	    /*
	     * Now, abort any kernel wait in the target so it won't be
	     * restarted when the thread wakes up.
	     */
	    if ((status = thread_abort (vpid->vp)) != KERN_SUCCESS)
		cma__bugcheck (
			"vp_interrupt: %s (%ld) thread_abort(%ld)",
			mach_error_string (status),
			status,
			vpid->vp);

	    /*
	     * Set up for the async call
	     */

	    if ((status = thread_get_state (
		    vpid->vp,
		    ALPHA_THREAD_STATE,
		    (thread_state_t)&state_array,
		    &state_count)) != KERN_SUCCESS)
		cma__bugcheck (
			"vp_interrupt: %s (%ld) thread_get_state(%ld)",
			mach_error_string (status),
			status,
			vpid->vp);

	    state_array.pc = (cma_t_integer)cma__do_interrupt;
	    state_array.r16 = (cma_t_integer)handler;
	    state_array.r17 = (cma_t_integer)arg;
	    state_array.r27 = (cma_t_integer)cma__do_interrupt;

	    if ((status = thread_set_state (
		    vpid->vp,
		    ALPHA_THREAD_STATE,
		    (thread_state_t)&state_array,
		    ALPHA_THREAD_STATE_COUNT)) != KERN_SUCCESS)
		cma__bugcheck (
			"vp_interrupt: %s (%ld) thread_set_state(%ld)",
			mach_error_string (status),
			status,
			vpid->vp);

	    }

	/*
	 * Now let it run the new code (or resume where it was)
	 */
	if ((status = thread_resume (vpid->vp)) != KERN_SUCCESS)
	    cma__bugcheck (
		    "vp_interrupt: %s (%ld) thread_resume(%ld)",
		    mach_error_string (status),
		    status,
		    vpid->vp);

	}

    return status;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set up initial VP stack and start function
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		ID of kernel thread
 *	state		state array
 *
 *  IMPLICIT INPUTS:
 *
 *	the current kernel thread
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Mach status
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_set_start
#ifdef _CMA_PROTO_
	(cma__t_vpid	vpid,
	cma__t_vp_state	*state)
#else	/* no prototypes */
	(vpid, state)
	cma__t_vpid	vpid;
	cma__t_vp_state	*state;
#endif	/* prototype */
    {
    kern_return_t		status;
    struct alpha_thread_state	thd_state;
    int				state_count = ALPHA_THREAD_STATE_COUNT;
    extern long 		_gp;	/* ld(1) defines this */


    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_set_start) setting vp %ld state to tcb 0x%lx",
	    vpid->vp,
	    state->tcb));

    /*
     * Set up Alpha call frame and registers.  First, zero the area.
     */
    bzero((char *)&thd_state, sizeof(struct alpha_thread_state));
    thd_state.r29 = (cma_t_integer)&_gp;
    thd_state.r30 = (cma_t_integer)state->stack;
    thd_state.r27 = (cma_t_integer)cma__thread_base;
    thd_state.r26 = 0;
    thd_state.pc  = (cma_t_integer)cma__thread_base;
    thd_state.r16 = state->tcb;		/* a0 */

    /*
     * Set the thread state so it can restart in proper place
     */
    if ((status = thread_set_state (
	    vpid->vp,
	    ALPHA_THREAD_STATE,
	    (thread_state_t)&thd_state,
	    ALPHA_THREAD_STATE_COUNT)) != KERN_SUCCESS)
	cma__bugcheck (
		"vp_set_start: %s (%ld) thread_set_state(%ld)",
		mach_error_string (status),
		status,
		vpid->vp);

    if (state->priority != vpid->priority || state->policy != vpid->policy)
	status = cma__vp_set_sched (vpid, state->policy, state->priority);

    return status;
    }
#endif
