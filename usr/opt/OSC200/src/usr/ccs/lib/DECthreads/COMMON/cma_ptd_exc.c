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
static char *rcsid = "@(#)$RCSfile: cma_ptd_exc.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:49:22 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads Core Services
 *
 *  ABSTRACT:
 *
 *	Implement POSIX 1003.4 draft thread operations on top of CMA (from
 *	9 January 1990 draft).
 *
 *	ptdexc_*  routines (Exception generating).
 *
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 *
 *	28 August 1990
 *
 *  MODIFICATION HISTORY:
 * 
 *	001	Paul Curtin	13 September 1990
 *		Changing ftime to cma__ftime.
 *	002	Dave Butenhof	8 October 1990
 *		Change cma_once call to include new argument. Also clean up
 *		code some and remove unnecessary calls to cma_init().
 *	003	Paul Curtin	21 November 1990
 *		Switched the order of the arguments to pthread_keycreate.
 *	004	Dave Butenhof	4 February 1991
 *		Change names from "pthread_<foo>_e" to "ptdexc_<foo>" to
 *		avoid conflicts with new pthread exception aliases.
 *	005	Dave Butenhof	7 February 1991
 *		Adapt to new alert bit names.
 *	006	Dave Butenhof	21 March 1991
 *		Implement ptdexc_cond_signal_int_np().
 *	007	Dave Butenhof	21 March 1991
 *		Raise pthread_badparam_e on bad cancel state input
 *	008	Dave Butenhof	26 March 1991
 *		Add pthread_signal_to_cancel_np() for OSF DCE
 *	009	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	010	Paul Curtin	04 May 1991
 *		Replace cma_init() with cma__int_init()
 *	011	Paul Curtin	13 May 1991
 *		Replaced a number of external interface calls
 *		with internal calls and macros. (Optimizations.)
 *	012	Dave Butenhof and Webb Scales	05 June 1991
 *		Include cma_px.h to get struct timespec definition.
 *	013	Paul Curtin	2 Aug 1991
 *		Added init check to ptdexc_lock_global_np
 *	014	Dave Butenhof	27 August 1991
 *		Fix bug in join: it should accept NULL status argument.
 *	015	Dave Butenhof	07 October 1991
 *		Add pthread_attr_setguardsize_np &
 *		pthread_attr_getguardsize_np functions.
 *	016	Paul Curtin	20 November 1991
 *		Alpha port: a number of platform specific changes.
 *	017	Dave Butenhof	05 December 1991
 *		Fix pthread_attr_setsched to support background policy!
 *	018	Dave Butenhof	10 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	019	Dave Butenhof	09 March 1992
 *		The mutex block/unblock functions have just been changed to
 *		return status values. Change pthread mutex lock/unlock to
 *		call this directly instead of using cma__int_[un]lock and
 *		convert status, rather than using expensive TRY/CATCH.
 *	020	Dave Butenhof	10 March 1992
 *		Change 'timeb' to 'timeval' so we can use gettimeofday rather
 *		than the BSD-only ftime function.
 *	021	Dave Butenhof	12 March 1992
 *		Clean up 019 a little.
 *	022	Dave Butenhof	12 March 1992
 *		More integration of direct code: pthread_self,
 *		pthread_getspecific, pthread_setspecific.
 *	023	Webb Scales	19 May 1992
 *		Reworked signal-to-cancel to avoid holding the global lock 
 *		during thread creation.  Ended up implmenting a new, modular
 *		version of it, which the old version is layered on top of.
 *	024	Brian Keane, Paul Curtin, Webb Scales	05 August 1992
 *		Re-worked pthread_yield.  It now calls cma__vp_yield directly.
 *	025	Dave Butenhof	24 August 1992
 *		Improve cond performance by calling things directly.
 *	026	Webb Scales	27 August 1992
 *		Add cond-signal-preempt-int
 *	027	Dave Butenhof	01 September 1992
 *		CV & mutex functions have changed some.
 *	028	Webb Scales	 3 September 1992
 *		Fix conditional proto in cond-signal-preempt-int
 *	029	Dave Butenhof	 9 September 1992
 *		Fix ptdexc_delay_np to use per-thread CV & mutex.
 *	030	Dave Butenhof	15 September 1992
 *		For _CMA_RT4_KTHREAD_, all non-realtime scheduling policies
 *		are mapped into SCHED_OTHER; avoid duplicate cases in switch
 *		statements.
 *	031	Webb Scales	23 September 1992
 *		Add the "ada-rtb" scheduling policy.
 *	032	Dave Butenhof	25 November 1992
 *		Remove special cases for RT4_KTHREAD policies; all are now
 *		defined uniquely.
 *	033	Dave Butenhof	12 February 1993
 *		(thought we already did this?) Validate timer values, so that
 *		select() can't complain later.
 *	034	Dave Butenhof	 2 March 1993
 *		I eliminated TCB locking in alert state modifications for the
 *		CMA interface long ago, but neglected to do so in the 1003.4a
 *		interfaces. So do it!
 *	035	Webb Scales	24 March 1993
 *		Handle carries properly when converting timespec's to VMS
 *		time format for timed condition waits.
 *	036	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	037	Dave Butenhof	 3 May 1993
 *		Inline ptdexc_detach()
 *	038	Dave Butenhof	15 June 1993
 *		Fix up obj_name macro on signal-to-cancel mutex.
 *	039	Brian Keane	1 July 1993
 *		Minor touch-ups to eliminate warnings with DEC C on OpenVMS AXP.
 *	040	Dave Butenhof	26 July 1993
 *		ptdexc_create() dequeues known TCB on failure, without
 *		locking kernel. Tsk tsk.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_px.h>			/* Get struct timespec */
#include <cma_util.h>
#include <cma_init.h>
#include <pthread_exc.h>
#include <cma_deb_core.h>
#if _CMA_OS_ == _CMA__VMS
# include <time.h>
#else
# include <sys/time.h>
#endif
#include <cma_tcb.h>
#include <cma_attr.h>
#include <cma_alert.h>
#include <cma_mutex.h>
#include <cma_stack.h>
#include <cma_condition.h>
#include <cma_int_errno.h>
#include <cma_context.h>
#include <cma_vp.h>

#if _CMA_OS_ == _CMA__VMS
#include <lib$routines.h>
#include <libdtdef.h>
#endif

#if _CMA_OS_ == _CMA__UNIX
# include <signal.h>
#endif

/*
 * GLOBAL DATA
 */

/*
 * LOCAL MACROS
 */

#if _CMA_OS_ == _CMA__UNIX
typedef struct PTDEXC___T_STC_BLK {
    sigset_t	    sigset;
    cma_t_thread    target;
    } ptdexc___t_stc_blk;
#endif

/*
 * LOCAL DATA
 */

#if _CMA_OS_ == _CMA__VMS
/*
 * Conversion: VMS absolute time of UNIX "epoch", 00:00:00 Jan 1, 1970
 */
static cma_t_date_time ptdexc___g_epoch = {0x4BEB4000, 0x007C9567};
#endif

static char	*cma___g_user_obj = "<pthread_exc user@0x%lx>";

/*
 * LOCAL FUNCTIONS
 */

#if _CMA_OS_ == _CMA__UNIX
static cma_t_address
ptdexc___cancel_thread _CMA_PROTOTYPE_ ((cma_t_address ptr));
#endif

static void
ptdexc___create_int_mutex _CMA_PROTOTYPE_ ((cma_t_address arg));

static void
ptdexc___unlock_int_mutex _CMA_PROTOTYPE_ ((pthread_addr_t arg));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a thread attribute object
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Return attribute object handle
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
ptdexc_attr_create
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    cma__int_init ();
    int_att = cma__get_attributes (&cma__g_def_attr);

    if ((cma_t_address)int_att == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)int_att, (cma_t_attr *)attr);
	cma__obj_set_owner (int_att, (cma_t_integer)attr);
	cma__obj_set_name (int_att, "<pthread_exc user(thread)@0x%lx>");
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete a thread attribute object
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object handle
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
ptdexc_attr_delete
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    int_att = cma__validate_null_attr ((cma_t_attr *)attr);

    if (int_att == (cma__t_int_attr *)cma_c_null_ptr)
	return;

    cma__free_attributes (int_att);
    cma__clear_handle (attr);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set priority attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
 *	priority	new priority
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
ptdexc_attr_setprio
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr,
	int		priority)
#else	/* no prototypes */
	(attr, priority)
	pthread_attr_t	*attr;
	int		priority;
#endif	/* prototype */
    {
    cma_attr_set_priority (attr, (cma_t_priority)priority);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get priority attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
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
 *	priority attribute
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_attr_getprio
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int                 ret_val = 0;
    cma_t_priority	priority;


    cma__int_attr_get_priority (&attr, &priority);
    ret_val = (int)priority;

    return ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set scheduler attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
 *	scheduler	new scheduler
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
ptdexc_attr_setsched
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr,
	int		scheduler)
#else	/* no prototypes */
	(attr, scheduler)
	pthread_attr_t	*attr;
	int		scheduler;
#endif	/* prototype */
    {
    cma_t_sched_policy	policy;
    cma_t_priority	priority;


    /*
     * Hopefully, POSIX 1003.4 pthread_attr_setsched will be redefined to
     * accept both policy and priority.  Since CMA has already addressed this
     * problem, this emulation needs to come up with a valid priority in the
     * new policy: for lack of any better ideas, simply use the midrange.
     */
    switch (scheduler) {
	case SCHED_FIFO : {
	    policy = cma_c_sched_fifo;
	    priority = cma_c_prio_fifo_mid;
	    break;
	    }
	case SCHED_RR : {
	    policy = cma_c_sched_rr;
	    priority = cma_c_prio_rr_mid;
	    break;
	    }
	case SCHED_FG_NP : {
	    policy = cma_c_sched_throughput;
	    priority = cma_c_prio_through_mid;
	    break;
	    }
	case SCHED_BG_NP : {
	    policy = cma_c_sched_background;
	    priority = cma_c_prio_back_mid;
	    break;
	    }
	case (int)cma_c_sched_ada_low : {
	    policy = cma_c_sched_ada_low;
	    priority = cma_c_prio_ada_low_mid;
	    break;
	    }
	case (int)cma_c_sched_ada_rtb : {
	    policy = cma_c_sched_ada_rtb;
	    priority = cma_c_prio_ada_rtb_mid;
	    break;
	    }
	default : {
	    break;
	    }

	}

    cma_attr_set_sched (attr, policy, priority);

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get scheduler attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
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
 *	scheduling policy attribute
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_attr_getsched
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_sched_policy	scheduler;


    cma__int_attr_get_sched (&attr, &scheduler);

    switch (scheduler) {
	case cma_c_sched_fifo : {
	    ret_val = SCHED_FIFO;
	    break;
	    }
	case cma_c_sched_rr : {
	    ret_val = SCHED_RR;
	    break;
	    }
	case cma_c_sched_throughput : {
	    ret_val = SCHED_FG_NP;
	    break;
	    }
	case cma_c_sched_background : {
	    ret_val = SCHED_BG_NP;
	    break;
	    }
	case cma_c_sched_ada_low : {
	    ret_val = (int)cma_c_sched_ada_low;
	    break;
	    }
	case cma_c_sched_ada_rtb : {
	    ret_val = (int)cma_c_sched_ada_rtb;
	    break;
	    }
	case cma_c_sched_idle : {
	    ret_val = (int)cma_c_sched_idle;
	    break;
	    }
	default : {
	    cma__bugcheck ("attr_getsched:1");
	    break;
	    }
	}

    return ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set inherit scheduling attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
 *	inherit		new inherit scheduling attribute
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
ptdexc_attr_setinheritsched
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr,
	int		inherit)
#else	/* no prototypes */
	(attr, inherit)
	pthread_attr_t	*attr;
	int		inherit;
#endif	/* prototype */
    {
    cma_t_sched_inherit	flag;


    switch (inherit) {
	case PTHREAD_INHERIT_SCHED : {
	    flag = cma_c_sched_inherit;
	    break;
	    }
	case PTHREAD_DEFAULT_SCHED : {
	    flag = cma_c_sched_use_default;
	    break;
	    }
	default : {
	    break;
	    }
	}

    cma_attr_set_inherit_sched (attr, flag);

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get inherit scheduling attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
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
 *	inherit scheduling info attribute
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_attr_getinheritsched
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_sched_inherit	inherit;


    cma__int_attr_get_inherit_sched (&attr, &inherit);

    switch ((int)inherit) {
	case (int)cma_c_sched_inherit : {
	    ret_val = PTHREAD_INHERIT_SCHED;
	    break;
	    }
	case (int)cma_c_sched_use_default : {
	    ret_val = PTHREAD_DEFAULT_SCHED;
	    break;
	    }
	default : {
	    break;
	    }
	}

    return (int)ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set stack size attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
 *	stacksize	new stack size
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
ptdexc_attr_setstacksize
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr,
	long		stacksize)
#else	/* no prototypes */
	(attr, stacksize)
	pthread_attr_t	*attr;
	long		stacksize;
#endif	/* prototype */
    {
    cma__int_attr_set_stacksize (attr, (cma_t_natural)stacksize);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get stack size attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
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
 *	stack size attribute
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern long
ptdexc_attr_getstacksize
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_natural	stacksize;


    cma__int_attr_get_stacksize (&attr, &stacksize);
    ret_val = (long)stacksize;

    return (long)ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set guard size attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
 *	guardsize	new guard size
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
ptdexc_attr_setguardsize_np
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr,
	long		guardsize)
#else	/* no prototypes */
	(attr, guardsize)
	pthread_attr_t	*attr;
	long		guardsize;
#endif	/* prototype */
    {
    cma__int_attr_set_guardsize (attr, (cma_t_natural)guardsize);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get guard size attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
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
 *	guard size attribute
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern long
ptdexc_attr_getguardsize_np
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_natural	guardsize;


    cma__int_attr_get_guardsize (&attr, &guardsize);
    ret_val = (long)guardsize;

    return (long)ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a thread object
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		Return new thread handle
 *	attr		Attribute object handle
 *	start_routine	Thread's start routine
 *	arg		Thread's argument
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
ptdexc_create
#ifdef _CMA_PROTO_
	(
	pthread_t		*thread,
	pthread_attr_t		attr,
	pthread_startroutine_t	start_routine,
	pthread_addr_t		arg)
#else	/* no prototypes */
	(thread, attr, start_routine, arg)
	pthread_t		*thread;
	pthread_attr_t		attr;
	pthread_startroutine_t	start_routine;
	pthread_addr_t		arg;
#endif	/* prototype */
    {
    cma__t_int_attr	*new_att;	/* Internal form of attrib. obj */
    cma__t_int_tcb	*new_tcb;	/* New thread's TCB */
    cma_t_status	status;
    cma__t_queue	*tq;


    cma__int_init ();

    /*    
     * Get internal pointer to att object (and validate it while we're at it)
     */
    new_att = cma__validate_default_attr (&attr);
		
    /* 
     * Get a TCB (and stack) for the new thread
     */
    cma__dispell_zombies ();		/* see if we can free a tcb/stack */
    new_tcb = cma__get_tcb (new_att);

    if ((cma_t_address)new_tcb == cma_c_null_ptr)
	cma__error (exc_s_insfmem);

    status = cma__int_make_thread (
	    new_tcb,
	    thread,
	    start_routine,
	    arg);

    if (status != cma_s_normal) {
	cma__enter_kernel ();
	cma__queue_remove (&new_tcb->threads, tq, cma__t_queue);
	cma__g_threadcnt--;
	cma__exit_kernel ();
	cma__free_tcb (new_tcb);
	cma__error (status);
	}

    /*
     * Point user's handle at the new thread object
     */
    cma__object_to_handle ((cma__t_object *)new_tcb, thread);
    cma__obj_set_owner (new_tcb, (cma_t_integer)thread);
    cma__obj_set_name (new_tcb, cma___g_user_obj);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Join with a thread (wait for it to terminate)
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		The thread to join with
 *	status		Return thread's status
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
ptdexc_join
#ifdef _CMA_PROTO_
	(
	pthread_t		thread,
	pthread_addr_t		*status)
#else	/* no prototypes */
	(thread, status)
	pthread_t		thread;
	pthread_addr_t		*status;
#endif	/* prototype */
    {
    cma_t_exit_status	thdstatus;
    cma_t_address	result;


    /*
     * CMA thread exit results in two separate values: a "result", and an
     * "exit status".  Pthread exit status combines these two into a single
     * value, with the value -1 corresponding to an error and other values
     * corresponding to a successful return.  Therefore, translate an error
     * status into a value of -1; otherwise return the thread's actual value.
     * The emulation of pthread_exit effectively reverses this mapping to
     * ensure that the correct value is returned.
     */
    cma_thread_join (&thread, &thdstatus, &result);

    if (status != (pthread_addr_t *)cma_c_null_ptr) {
	if (thdstatus == cma_c_term_normal)
	    *status = (pthread_addr_t)result;
	else
	    *status = (pthread_addr_t)-1;
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Detach a thread (declare that its storage may be reclaimed when it
 *	has terminated: it will not be joined with).
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		The thread to detach
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
ptdexc_detach
#ifdef _CMA_PROTO_
	(
	pthread_t		*thread)
#else	/* no prototypes */
	(thread)
	pthread_t		*thread;
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;


    /* 
     * Validate the handle, insure that it's a thread, and get the TCB address
     */
    tcb = cma__validate_null_tcb (thread);

    /*
     * If thread is a null handle, do nothing and return
     */
    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr) return;

    cma__int_detach (tcb);

    /*
     * This handle is no longer valid, clear it
     */
    cma__clear_handle (thread);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Terminate the thread, returning status to joiner(s).
 *
 *  FORMAL PARAMETERS:
 *
 *	status		The status/result to return.
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
 *	Terminates the thread: this operation does not return to the caller.
 */
extern void
ptdexc_exit
#ifdef _CMA_PROTO_
	(
	pthread_addr_t	status)
#else	/* no prototypes */
	(status)
	pthread_addr_t	status;
#endif	/* prototype */
    {

    /*
     * Since CMA specifies separate exit operations depending on whether the
     * thread completed successfully or not, we have to appropriately
     * interpret the intent of the pthread_exit caller.  Since the standard
     * convention is to return "-1" on failure, we will call
     * cma_thread_exit_error on a pthread status of -1; and otherwise pass
     * the pthread status to cma_thread_exit_normal.
     *
     * Also, notice that noeptions are caught: the CMA exit routines can
     * only raise the special internal "termination" exception, and we want
     * that to propagate to the thread's base frame so it can do its job.
     */
    if ((int)status == -1)
	cma_thread_exit_error ();
    else
	cma_thread_exit_normal ((cma_t_address)status);

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return the current thread's ID.
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
 *	Thread ID
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern pthread_t
ptdexc_self
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;
    cma_t_thread	thread;


    cma__int_init ();

    /*
     * Get a pointer to the current thread's TCB
     */
    tcb = cma__get_self_tcb ();

    /*
     * Point user's handle at the current thread object
     */
    cma__object_to_handle ((cma__t_object *)tcb, &thread);
    return thread;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set thread's priority
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		Thread handle
 *	priority	new priority
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
 *	Previous priority, or -1 if error.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_setprio
#ifdef _CMA_PROTO_
	(
	pthread_t	thread,
	int		priority)
#else	/* no prototypes */
	(thread, priority)
	pthread_t	thread;
	int		priority;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_priority	oldpri;


    cma__int_init ();
    cma_thread_get_priority (&thread, &oldpri);
    cma_thread_set_priority (&thread, (cma_t_priority)priority);
    ret_val = (int)oldpri;

    return ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get thread's priority
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		Thread ID
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
 *	Priority, or -1 if error.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_getprio
#ifdef _CMA_PROTO_
	(
	pthread_t	thread)
#else	/* no prototypes */
	(thread)
	pthread_t	thread;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_priority	priority;


    cma__int_init ();
    cma_thread_get_priority (&thread, &priority);
    ret_val = (int)priority;

    return ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set thread's scheduler
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		Thread ID
 *	scheduler	new scheduler
 *	priority	new priority
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
 *	Old scheduling policy, or -1 if error
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_setscheduler
#ifdef _CMA_PROTO_
	(
	pthread_t	thread,
	int		scheduler,
	int		priority)
#else	/* no prototypes */
	(thread, scheduler, priority)
	pthread_t	thread;
	int		scheduler;
	int		priority;
#endif	/* prototype */
    {
    cma_t_priority	oldpriority;


    cma__int_init ();
    cma_thread_get_priority (&thread, &oldpriority);
    cma_thread_set_sched (
	    &thread,
	    (cma_t_sched_policy)scheduler,
	    (cma_t_priority)priority);
    return (int)oldpriority;
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get thread's scheduler
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		Thread ID
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
 *	Scheduling policy, or -1 if error
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_getscheduler
#ifdef _CMA_PROTO_
	(
	pthread_t	thread)
#else	/* no prototypes */
	(thread)
	pthread_t	thread;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_sched_policy	scheduler;


    cma__int_init ();
    cma_thread_get_sched (&thread, &scheduler);

    switch (scheduler) {
	case cma_c_sched_fifo : {
	    ret_val = SCHED_FIFO;
	    break;
	    }
	case cma_c_sched_rr : {
	    ret_val = SCHED_RR;
	    break;
	    }
	case cma_c_sched_throughput : {
	    ret_val = SCHED_FG_NP;
	    break;
	    }
	case cma_c_sched_background : {
	    ret_val = SCHED_BG_NP;
	    break;
	    }
	case cma_c_sched_ada_low : {
	    ret_val = (int)cma_c_sched_ada_low;
	    break;
	    }
	case cma_c_sched_ada_rtb : {
	    ret_val = (int)cma_c_sched_ada_rtb;
	    break;
	    }
	case cma_c_sched_idle : {
	    ret_val = (int)cma_c_sched_idle;
	    break;
	    }
	default : {
	    break;
	    }

	}

    return ret_val;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Yield processor
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
ptdexc_yield
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_THREAD_IS_VP_
    cma__vp_yield ();
#else
    cma__enter_kernel ();
    cma__yield_processor ();
    cma__exit_kernel ();
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	One-time initialization
 *
 *  FORMAL PARAMETERS:
 *
 *	once_block	Control block (pthread_once_t)
 *	init_routine	Routine to call for initialization
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
ptdexc_once
#ifdef _CMA_PROTO_
	(
	pthread_once_t		*once_block,
	pthread_initroutine_t	init_routine)
#else	/* no prototypes */
	(once_block, init_routine)
	pthread_once_t		*once_block;
	pthread_initroutine_t	init_routine;
#endif	/* prototype */
    {
    cma__int_init ();
    cma_once (
	    (cma_t_once *)once_block,
	    (cma_t_init_routine)init_routine,
	    cma_c_null_ptr);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create thread-specific key value
 *
 *  FORMAL PARAMETERS:
 *
 *	destructor	Routine to call on thread termination
 *	key		Return value of key
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
ptdexc_keycreate
#ifdef _CMA_PROTO_
	(
	pthread_key_t		*key,
	pthread_destructor_t	destructor)
#else	/* no prototypes */
	(key,destructor)
	pthread_key_t		*key;
	pthread_destructor_t	destructor;
#endif	/* prototype */
    {
    cma__int_init ();
    cma_key_create (key, &cma_c_null, destructor);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set thread-specific key value
 *
 *  FORMAL PARAMETERS:
 *
 *	key		Key to set
 *	value		Value to set
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
ptdexc_setspecific
#ifdef _CMA_PROTO_
	(
	pthread_key_t	key,
	pthread_addr_t	value)
#else	/* no prototypes */
	(key, value)
	pthread_key_t	key;
	pthread_addr_t	value;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;		/* Pointer to TCB */
    cma_t_integer	next_context;	/* Snapshot of global key count */


    /*
     * Note that we don't lock any mutexes. This keeps the getcontext
     * function lean and mean. There are two important assumptions in this:
     *
     * 1) If any other thread is modifying the maximum context value, we may
     * get the wrong value, with a possible "spurious" failure. However, that
     * would mean someone is creating a context key in one thread and USING
     * it in another without synchronization. Tough cookies: you get what you
     * pay for.
     *
     * 2) We assume that no other thread will access or modify the context
     * data in this thread's TCB. Since we don't lock the TCB, such access
     * could result in erroneous results in one or the other. DECthreads
     * doesn't support any interfaces to touch another thread's context
     * values, so anyone doing so is just asking for trouble, and I don't
     * mind giving it to them to save a lock operation!
     */
    next_context = cma__g_context_next;

    if ((key <= 0) || (key >= next_context))
	cma__error (cma_s_badparam);

    self = cma__get_self_tcb ();	/* Get current thread's TCB */

    /*
     * If the key value is higher than the current size of the thread's
     * context array, then allocate a new array.  Make it large enough to
     * hold the highest defined key value.  Copy the current context array
     * (if any) into the new one, and free the old one.
     */
    if (self->context_count <= key) {
	cma__t_context_list	new_list;	/* Pointer to new list */
	cma_t_natural		new_size;	/* Size of new list */


	new_size = sizeof (cma_t_address) * next_context;
	new_list = (cma__t_context_list)cma__alloc_zero (new_size);

	if ((cma_t_address)new_list == cma_c_null_ptr) {
	    cma__error (exc_s_insfmem);
	    }

	if (self->contexts != (cma__t_context_list)cma_c_null_ptr) {
	    cma__memcpy (
		    (char *)new_list,
		    (char *)self->contexts,
		    (self->context_count) * sizeof (cma_t_address));
	    cma__free_mem ((cma_t_address)self->contexts);
	    }

	self->contexts = new_list;
	self->context_count = next_context;
	}

    self->contexts[key] = value;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get thread-specific key value
 *
 *  FORMAL PARAMETERS:
 *
 *	key		key to get
 *	value		Return value
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
ptdexc_getspecific
#ifdef _CMA_PROTO_
	(
	pthread_key_t	key,
	pthread_addr_t	*value)
#else	/* no prototypes */
	(key, value)
	pthread_key_t	key;
	pthread_addr_t	*value;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;		/* Pointer to TCB */
    cma_t_integer	next_context;	/* Local copy of next key value */


    /*
     * Note that we don't lock any mutexes. This keeps the getcontext
     * function lean and mean. There are two important assumptions in this:
     *
     * 1) If any other thread is modifying the maximum context value, we may
     * get the wrong value, with a possible "spurious" failure. However, that
     * would mean someone is creating a context key in one thread and USING
     * it in another without synchronization. Tough cookies: you get what you
     * pay for.
     *
     * 2) We assume that no other thread will access or modify the context
     * data in this thread's TCB. Since we don't lock the TCB, such access
     * could result in erroneous results in one or the other. DECthreads
     * doesn't support any interfaces to touch another thread's context
     * values, so anyone doing so is just asking for trouble, and I don't
     * mind giving it to them to save a lock operation!
     */
    next_context = cma__g_context_next;

    if ((key <= 0) || (key >= next_context))
	cma__error (cma_s_badparam);

    self = cma__get_self_tcb ();	/* Get current thread's TCB */

    /*
     * If the requested key is not within the allocated context array (or if
     * there is no context array), then return the value null ("no context");
     * otherwise return the current value of the context (which may also be
     * null).
     */
    if (self->context_count <= key)
	*value = cma_c_null_ptr;
    else
	*value = self->contexts[key];

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a mutex attribute object
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Return attribute object handle
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
ptdexc_mutexattr_create
#ifdef _CMA_PROTO_
	(
	pthread_mutexattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_mutexattr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    cma__int_init ();
    int_att = cma__get_attributes (&cma__g_def_attr);

    if ((cma_t_address)int_att == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)int_att, (cma_t_attr *)attr);
	cma__obj_set_owner (int_att, (cma_t_integer)attr);
	cma__obj_set_name (int_att, "<pthread_exc user(mutex)@0x%lx>");
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete a mutex attribute object
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object handle
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
ptdexc_mutexattr_delete
#ifdef _CMA_PROTO_
	(
	pthread_mutexattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_mutexattr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    int_att = cma__validate_null_attr ((cma_t_attr *)attr);

    if (int_att == (cma__t_int_attr *)cma_c_null_ptr)
	return;

    cma__free_attributes (int_att);
    cma__clear_handle (attr);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set mutex kind attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
 *	kind		mutex kind
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
ptdexc_mutexattr_setkind_np
#ifdef _CMA_PROTO_
	(
	pthread_mutexattr_t	*attr,
	int			kind)
#else	/* no prototypes */
	(attr, kind)
	pthread_mutexattr_t	*attr;
	int			kind;
#endif	/* prototype */
    {
    cma_attr_set_mutex_kind (attr, (cma_t_mutex_kind)kind);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get mutex kind attribute
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object
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
 *	mutex kind attribute
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_mutexattr_getkind_np
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    cma_t_mutex_kind	kind;


    cma_attr_get_mutex_kind (&attr, &kind);
    return (int)kind;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a mutex object
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Return mutex handle
 *	attr		Attribute object handle
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
ptdexc_mutex_init
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t		*mutex,
	pthread_mutexattr_t	attr)
#else	/* no prototypes */
	(mutex, attr)
	pthread_mutex_t		*mutex;
	pthread_mutexattr_t	attr;
#endif	/* prototype */
    {
    cma__t_int_mutex    *imutex;
    cma__t_int_attr     *int_att;


    cma__int_init ();
    int_att = cma__validate_default_attr (&attr);
    imutex = cma__get_mutex (int_att);

    if ((cma_t_address)imutex == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)imutex, mutex);
	cma__obj_set_owner (imutex, (cma_t_integer)mutex);
	cma__obj_set_name (imutex, cma___g_user_obj);
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete a mutex object
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object handle
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
ptdexc_mutex_destroy
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    cma_mutex_delete (mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Lock a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object handle
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
ptdexc_mutex_lock
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;
    cma__t_int_mutex    *int_mutex;

    /*
     * Get a pointer to the mutex structure; if this is a debugging CMA,
     * we'll validate the mutex handle to be sure it's valid.  For
     * performance, if it's an NDEBUG ("production") CMA, just fetch the
     * address of the object from the handle's pointer field.
     */
#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    int_mutex = cma__validate_mutex (mutex);
    tcb = cma__get_self_tcb ();
#endif
                          
    /*
     * First, try to acquire the lock; if we get it, then we're done
     */
    if (cma__test_and_set (&int_mutex->lock)) {
	cma_t_status	res;


	res = cma__int_mutex_block (int_mutex);

	if (res != cma_s_normal)
	    cma__error (res);

	}

#ifndef NDEBUG
    int_mutex->owner = tcb;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Attempt to lock a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object handle
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
 *	0 if already locked; 1 if successful
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_mutex_trylock
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    cma__t_int_mutex	*int_mutex;
    cma__t_int_tcb	*tcb;


#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    int_mutex = cma__validate_mutex (mutex);
    tcb = cma__get_self_tcb ();
#endif

    if (cma__test_and_set (&int_mutex->lock)) {

	switch (int_mutex->mutex_kind) {
	    case cma_c_mutex_nonrecursive : {
#ifdef NDEBUG
		tcb = cma__get_self_tcb ();
#endif
		cma__mulock (int_mutex);

		if (int_mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		    int_mutex->owner = tcb;
		    cma__muunlock (int_mutex);
		    return 1;
		    }
		else {
		    cma__muunlock (int_mutex);
		    return 0;
		    }

		break;
		}

	    case cma_c_mutex_recursive : {
#ifdef NDEBUG
		tcb = cma__get_self_tcb ();
#endif
		cma__mulock (int_mutex);

		if (tcb == int_mutex->owner
			|| int_mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		    int_mutex->nest_count++;
		    int_mutex->owner = tcb;
		    cma__muunlock (int_mutex);
		    return 1;
		    }
		else {
		    cma__muunlock (int_mutex);
		    return 0;
		    }

		break;
		}

	    case cma_c_mutex_fast : {
		return 0;
		break;
		}

	    }

	}

#ifndef NDEBUG
    int_mutex->owner = tcb;
#endif
    return 1;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unlock a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object handle
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
ptdexc_mutex_unlock
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    cma__t_int_mutex    *int_mutex;

#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    cma__t_int_tcb	*tcb;

    int_mutex = cma__validate_mutex (mutex);
    tcb = cma__get_self_tcb ();

    if (int_mutex->mutex_kind == cma_c_mutex_fast) {
	cma__assert_warn (
		(tcb == int_mutex->owner),
		"attempt to release mutex owned by another thread");
	int_mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;
	}

#endif
    cma__unset (int_mutex->unlock);

    /*
     * Check whether there might be waiters, and reset the bit (TRUE means
     * "no waiters").  If there might be waiters, increment the semaphore to
     * release one.
     */
    if (!cma__test_and_set (&int_mutex->event)) {
	cma_t_status	res;


	res = cma__int_mutex_unblock (int_mutex);

	if (res != cma_s_normal)
	    cma__error (res);

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a condition attribute object
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Return attribute object handle
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
ptdexc_condattr_create
#ifdef _CMA_PROTO_
	(
	pthread_condattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_condattr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    cma__int_init ();
    int_att = cma__get_attributes (&cma__g_def_attr);

    if ((cma_t_address)int_att == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)int_att, (cma_t_attr *)attr);
	cma__obj_set_owner (int_att, (cma_t_integer)attr);
	cma__obj_set_name (int_att, "<pthread_exc user(cond)@0x%lx>");
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete a condition attribute object
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attribute object handle
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
ptdexc_condattr_delete
#ifdef _CMA_PROTO_
	(
	pthread_condattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_condattr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    int_att = cma__validate_null_attr ((cma_t_attr *)attr);

    if (int_att == (cma__t_int_attr *)cma_c_null_ptr)
	return;

    cma__free_attributes (int_att);
    cma__clear_handle (attr);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a condition object
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Return condition handle
 *	attr		Attribute object handle
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
ptdexc_cond_init
#ifdef _CMA_PROTO_
	(
	pthread_cond_t		*cond,
	pthread_condattr_t	attr)
#else	/* no prototypes */
	(cond, attr)
	pthread_cond_t		*cond;
	pthread_condattr_t	attr;
#endif	/* prototype */
    {
    cma__t_int_cv               *cv; 
    cma__t_int_attr             *int_att; 


    cma__int_init ();
    int_att = cma__validate_default_attr (&attr);
    cv = cma__get_cv (int_att);

    if ((cma_t_address)cv == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)cv, cond);
	cma__obj_set_owner (cv, (cma_t_integer)cond);
	cma__obj_set_name (cv, cma___g_user_obj);
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete a condition object
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
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
ptdexc_cond_destroy
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond)
#else	/* no prototypes */
	(cond)
	pthread_cond_t	*cond;
#endif	/* prototype */
    {
    cma__int_cond_delete (cond);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
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
ptdexc_cond_signal
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond)
#else	/* no prototypes */
	(cond)
	pthread_cond_t	*cond;
#endif	/* prototype */
    {
    cma__t_int_cv       *int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)cond)->pointer;
#else
    int_cv = cma__validate_cv (cond);
#endif

    cma__int_signal (int_cv);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition from interrupt level
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
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
ptdexc_cond_signal_int_np
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond)
#else	/* no prototypes */
	(cond)
	pthread_cond_t	*cond;
#endif	/* prototype */
    {
    cma_cond_signal_int (cond);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition from interrupt level
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
#if _CMA_OS_ == _CMA__UNIX
 *	scp		Address of the Signal Context Structure
#endif
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
 *	When the call returns from this routine, the process may not be 
 *	"at interrupt level" anymore.  
 *
 *	Note, this routine may ONLY be called from interrupt level.
 */
extern void
ptdexc_cond_sigprmpt_int_np
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond
# if _CMA_OS_ == _CMA__UNIX
	,pthread_addr_t	scp
# endif
	)
#else	/* no prototypes */
# if _CMA_OS_ == _CMA__UNIX
	(cond, scp)
	pthread_cond_t	*cond;
	pthread_addr_t	scp;
# else
	(cond)
	pthread_cond_t	*cond;
# endif
#endif	/* prototype */
    {
    cma_cond_signal_preempt_int (
	    cond
#if _CMA_OS_ == _CMA__UNIX
	    ,
	    scp
#endif
	    );
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Broadcast a condition
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
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
ptdexc_cond_broadcast
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond)
#else	/* no prototypes */
	(cond)
	pthread_cond_t	*cond;
#endif	/* prototype */
    {
    cma__t_int_cv       *int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)cond)->pointer;
#else
    int_cv = cma__validate_cv (cond);
#endif

    cma__int_broadcast (int_cv);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait on a condition
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
 *	mutex		Mutex associated with condition
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
ptdexc_cond_wait
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond,
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(cond, mutex)
	pthread_cond_t	*cond;
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    cma__int_cond_wait (cond, mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait on a condition, with timeout
 *
 *  FORMAL PARAMETERS:
 *
 *	cond		Condition object handle
 *	mutex		Mutex associated with condition
 *	abstime		Time to wait
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
 *	status
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_cond_timedwait
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond,
	pthread_mutex_t	*mutex,
	struct timespec	*abstime)
#else	/* no prototypes */
	(cond, mutex, abstime)
	pthread_cond_t	*cond;
	pthread_mutex_t	*mutex;
	struct timespec	*abstime;
#endif	/* prototype */
    {
    cma__t_int_mutex	*int_mutex;
    cma__t_int_cv	*int_cv;
    cma__t_int_tcb	*self = cma__get_self_tcb();
    cma_t_status	status;
    cma_t_date_time	time;
#if _CMA_OS_ == _CMA__VMS
    cma_t_date_time	delta;
    int			vstat;
    int			delta_secs;
#endif


#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)cond)->pointer;
#else
    int_mutex = cma__validate_mutex (mutex);
    int_cv = cma__validate_cv (cond);
#endif

    if (abstime->tv_nsec >= (1000 * 1000000) || abstime->tv_nsec < 0)
	cma__error (cma_s_badparam);

#if _CMA_OS_ == _CMA__VMS
    delta_secs = LIB$K_DELTA_SECONDS;

    vstat = lib$cvt_to_internal_time (
	    &delta_secs,
	    &abstime->tv_sec,
	    &delta);

    if ((vstat & 7) != 1)
	cma__error (cma_s_badparam);

    /*
     * Adjust the resulting delta time by the number of nanoseconds in the
     * timespec.  Note that VMS delta time is negative, so we subtract. Be
     * careful to handle the carry properly.
     */
    if (delta.low < ((abstime->tv_nsec + 99) / 100))
	delta.high -= 1;
    delta.low -= ((abstime->tv_nsec + 99) / 100);
    lib$add_times (&ptdexc___g_epoch, &delta, &time);
#else
    time.tv_sec = abstime->tv_sec;
    time.tv_usec = (abstime->tv_nsec + 999) / 1000;
#endif

    status = cma__int_timed_wait (int_cv, int_mutex, &time, self);
    
    if (status == cma_s_timed_out)
	status = EAGAIN;
    else
	status = 0;

    return status;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Cancel a thread
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		ID of thread to cancel
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
ptdexc_cancel
#ifdef _CMA_PROTO_
	(
	pthread_t	thread)
#else	/* no prototypes */
	(thread)
	pthread_t	thread;
#endif	/* prototype */
    {
    cma_thread_alert (&thread);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Test for pending cancel
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
 *	cancel thread if cancel is pending.
 */
extern void
ptdexc_testcancel
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__int_init ();
    cma__int_alert_test ();
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set general cancelability of thread
 *
 *  FORMAL PARAMETERS:
 *
 *	state	New state of general cancelability (CANCEL_ON or CANCEL_OFF)
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
 *	CANCEL_ON if cancellation was enabled; CANCEL_OFF if it was disabled.
 *
 *  SIDE EFFECTS:
                       *
 *	none
 */
extern int
ptdexc_setcancel
#ifdef _CMA_PROTO_
	(int	state)
#else	/* no prototypes */
	(state)
	int	state;
#endif	/* prototype */
    {
    int			previous;
    cma__t_int_tcb	*tcb;


    cma__int_init ();

    if (state != CANCEL_ON && state != CANCEL_OFF)
	cma__error (cma_s_badparam);

    tcb = cma__get_self_tcb ();
    previous = (tcb->alert.g_enable ? CANCEL_ON : CANCEL_OFF);
    tcb->alert.g_enable = (state == CANCEL_ON);
    return previous;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set async cancelability of thread
 *
 *  FORMAL PARAMETERS:
 *
 *	state	New state of async cancelability (CANCEL_ON or CANCEL_OFF)
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
 *	CANCEL_ON if cancellation was enabled; CANCEL_OFF if it was disabled.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
ptdexc_setasynccancel
#ifdef _CMA_PROTO_
	(int	state)
#else	/* no prototypes */
	(state)
	int	state;
#endif	/* prototype */
    {
    int			previous;
    cma__t_int_tcb	*tcb;


    cma__int_init ();

    if (state != CANCEL_ON && state != CANCEL_OFF)
	cma__error (cma_s_badparam);

    tcb = cma__get_self_tcb ();
    previous = (tcb->alert.a_enable ? CANCEL_ON : CANCEL_OFF);
    tcb->alert.a_enable = (state == CANCEL_ON);
    return previous;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Convert a delta timespec to absolute (offset by current time)
 *
 *  FORMAL PARAMETERS:
 *
 *	delta	struct timespec; input delta time
 *
 *	abstime	struct timespec; output absolute time
 *
 *  IMPLICIT INPUTS:
 *
 *	current time
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
extern int
ptdexc_get_expiration_np
#ifdef _CMA_PROTO_
	(struct timespec	*delta,
	struct timespec		*abstime)
#else	/* no prototypes */
	(delta, abstime)
	struct timespec		*delta;
	struct timespec		*abstime;
#endif	/* prototype */
    {
    struct timespec	current_time;


    if (delta->tv_nsec >= (1000 * 1000000) || delta->tv_nsec < 0)
	cma__error (cma_s_badparam);

    cma__gettimespec (&current_time);
    abstime->tv_nsec	= delta->tv_nsec + (current_time.tv_nsec);
    abstime->tv_sec	= delta->tv_sec + current_time.tv_sec;

    if (abstime->tv_nsec >= (1000 * 1000000)) {	/* check for carry */
	abstime->tv_nsec -= (1000 * 1000000);
	abstime->tv_sec += 1;
	}

    return 0;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait for interval.
 *
 *  FORMAL PARAMETERS:
 *
 *	interval	struct timespec
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
ptdexc_delay_np
#ifdef _CMA_PROTO_
	(struct timespec	*interval)
#else	/* no prototypes */
	(interval)
	struct timespec		*interval;
#endif	/* prototype */
    {
    cma_t_date_time	time;
    cma__t_int_tcb	*self;


    cma__int_init ();

    if (interval->tv_nsec >= (1000 * 1000000) || interval->tv_nsec < 0)
	cma__error (cma_s_badparam);

    /*
     * The simplest way to get through to the underlying CMA timed wait
     * mechanism is to convert the pthread delta "timespec" into a CMA
     * "interval" (floating point seconds), and use cma_time_get_expiration
     * to convert that to an absolute cma_t_date_time.
     */
    cma_time_get_expiration (
	    &time,
	    (float)interval->tv_sec
		+ ((float)interval->tv_nsec / (1000000000.0)));
    self = cma__get_self_tcb ();
    cma__int_lock (self->tswait_mutex);	/* Lock the mutex */

    /*
     * Using the pthread cleanup handler will catch any CMA exceptions which
     * get raised by the wait (specifically, cma_e_alerted, which is pthread
     * cancel).
     */
    pthread_cleanup_push (ptdexc___unlock_int_mutex, self->tswait_mutex);

    while (
	cma__int_timed_wait (
		self->tswait_cv,
		self->tswait_mutex,
		&time,
		self) != cma_s_timed_out);

    /*
     * Pop the cleanup handler, and execute it (ensure that the delay mutex
     * is unlocked whether the timed wait was cancelled or not).
     */
    pthread_cleanup_pop (1);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Lock global lock (for nonreentrant libraries)
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
ptdexc_lock_global_np
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__int_init ();
    cma__int_lock (cma__g_global_lock);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unlock global lock (for nonreentrant libraries)
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
ptdexc_unlock_global_np
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__int_unlock (cma__g_global_lock);
    }

#if _CMA_OS_ == _CMA__UNIX
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a thread that will sigwait() on a set of signals; when any of
 *	them arrives, it will cancel a specified thread.  Returns thread 
 *	handle to allow caller to control the server thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	sigset	The set of signals to catch
 *	target	The ID of the thread to be canceled
 *	thread	The ID of the sigwaiting thread 
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
ptdexc_sig_to_can_thread_np
#ifdef _CMA_PROTO_
	(sigset_t	*sigset,
	pthread_t	*target,
	pthread_t	*thread)
#else
	(sigset, target, thread)
	sigset_t	*sigset;
	pthread_t	*target;
	pthread_t	*thread;
#endif
    {
    ptdexc___t_stc_blk	*ptr;
    cma_t_thread	th;


    (void) cma__validate_tcb (target);

    ptr = (ptdexc___t_stc_blk *)cma__alloc_mem (sizeof(ptdexc___t_stc_blk));
    if (!(ptr))  cma__error (exc_s_insfmem);

    ptr->target = *target;
    ptr->sigset = *sigset;

    cma_thread_create (
	    &th,
	    &cma_c_null,
	    (cma_t_start_routine)ptdexc___cancel_thread,
	    (cma_t_address)ptr);

    if (thread)  *thread = th;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a thread that will sigwait() on a set of signals; when any of
 *	them arrives, it will cancel a specified thread.  Any thread previously 
 *	created by this routine will be cancelled.
 *
 *  FORMAL PARAMETERS:
 *
 *	sigset	The set of signals to catch
 *	target	The ID of the thread to be canceled
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
ptdexc_signal_to_cancel_np
#ifdef _CMA_PROTO_
	(sigset_t	*sigset,
	pthread_t	*target)
#else
	(sigset, target)
	sigset_t	*sigset;
	pthread_t	*target;
#endif
    {
    static cma_t_once		once_blk = cma_once_init;
    static cma__t_int_mutex	*mutex;
    static cma__t_int_handle	thread;


    cma_once (&once_blk, ptdexc___create_int_mutex, (cma_t_address)&mutex);

    cma__int_lock (mutex);

    TRY {
	if (thread.sequence != 0) {
	    /*
	     * If there is already a server thread, cancel and reclaim it.
	     * It is not important to actually wait for it to terminate.
	     * Detach will clear the handle.  Create will over-write it.
	     */
	    cma_thread_alert ((cma_t_thread *)&thread);
	    cma_thread_detach ((cma_t_thread *)&thread);
	    }

	ptdexc_sig_to_can_thread_np (sigset, target, (pthread_t *)&thread);
	}
    FINALLY {
	cma__int_unlock (mutex);
	}
    ENDTRY;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	The start routine for a thread that simply sigwaits; if sigwait()
 *	returns successfully, then a target thread is cancelled; if it
 *	returns with an error, the thread exits.
 *
 *  FORMAL PARAMETERS:
 *
 *	The address of a signal-to-cancel data block
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
 *	Cancel target thread when signal occurs.
 */
static cma_t_address
ptdexc___cancel_thread
#ifdef _CMA_PROTO_
	(cma_t_address ptr)
#else
	(ptr)
	cma_t_address ptr;
#endif
    {
    sigset_t	    sigset;
    cma_t_thread    target;


    sigset = ((ptdexc___t_stc_blk *)ptr)->sigset;
    target = ((ptdexc___t_stc_blk *)ptr)->target;

    cma__free_mem(ptr);

    while (1) {

	/*
	 * sigwait() on the signal set until canceled, or until sigwait()
	 * complains (which ought to be on the first try, if at all). Then
	 * just let the thread run down.
	 */
	if (cma_sigwait (&sigset) == -1)
	    return cma_c_null_ptr;

	cma_thread_alert (&target);
	}
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a mutex (used in a once block)
 *
 *  FORMAL PARAMETERS:
 *
 *	arg		address of mutex handle
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
ptdexc___create_int_mutex
#ifdef _CMA_PROTO_
	(pthread_addr_t	arg)
#else	/* no prototypes */
	(arg)
	pthread_addr_t	arg;
#endif	/* prototype */
    {
    *((cma__t_int_mutex **)arg) = cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (
	    *(cma__t_int_mutex **)arg,
	    "signal-to-cancel mutex");
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unlock a mutex on cancel or exit (used as cleanup handler)
 *
 *  FORMAL PARAMETERS:
 *
 *	arg		address of mutex handle
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
ptdexc___unlock_int_mutex
#ifdef _CMA_PROTO_
	(pthread_addr_t	arg)
#else	/* no prototypes */
	(arg)
	pthread_addr_t	arg;
#endif	/* prototype */
    {
    cma__int_unlock ((cma__t_int_mutex *)arg);
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_PTD_EXC.C */
/*  *42   26-JUL-1993 13:35:00 BUTENHOF "Fix thread create error path" */
/*  *41    2-JUL-1993 09:43:40 KEANE "Fix DEC C warnings: add return statements to non-void routines" */
/*  *40   16-JUN-1993 10:48:17 BUTENHOF "Fix obj_name on can_sig mutex" */
/*  *39   14-MAY-1993 15:55:57 BUTENHOF "Detach needs to move terminated thread after g_last_thread" */
/*  *38    3-MAY-1993 13:44:36 BUTENHOF "Make pthread_detach local" */
/*  *37   16-APR-1993 13:04:24 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *36    1-APR-1993 14:33:01 BUTENHOF "Add names to user objects" */
/*  *35   24-MAR-1993 20:07:46 SCALES "Fix gotcha in timespec to VMS time conversion" */
/*  *34    2-MAR-1993 13:08:24 BUTENHOF "Some performance twiddling" */
/*  *33   12-FEB-1993 11:32:41 BUTENHOF "Validate wait times" */
/*  *32   29-JAN-1993 12:46:16 BUTENHOF "Fix setscheduler to return priority, not policy" */
/*  *31    1-DEC-1992 14:05:30 BUTENHOF "OSF/1 scheduling" */
/*  *30   24-SEP-1992 08:56:39 SCALES "Add ""ada-rtb"" scheduling policy" */
/*  *29   15-SEP-1992 13:49:59 BUTENHOF "Fix sched case stmnts" */
/*  *28   10-SEP-1992 09:46:39 BUTENHOF "Fix delay" */
/*  *27    4-SEP-1992 08:45:46 BUTENHOF "New CV & mutex interfaces" */
/*  *26    3-SEP-1992 15:57:17 SCALES """Fix conditional""" */
/*  *25    3-SEP-1992 14:28:44 SCALES "Add cond-signal-preempt-int" */
/*  *24   25-AUG-1992 11:48:34 BUTENHOF "Adjust Mach yield operations" */
/*  *23    5-AUG-1992 21:52:01 KEANE "Change yield to not enter kernel on OSF/1" */
/*  *22   22-MAY-1992 17:43:39 SCALES "Rework signal-to-cancel to remove use of global lock" */
/*  *21   13-MAR-1992 14:09:11 BUTENHOF "Fix mutex lock/unlock code" */
/*  *20   10-MAR-1992 16:26:30 BUTENHOF "Eliminate need for TRY/CATCH on pthread mutex lock" */
/*  *19   18-FEB-1992 15:29:47 BUTENHOF "Adapt to alloc_mem changes" */
/*  *18    6-DEC-1991 07:19:38 BUTENHOF "Add background policy to attr_setsched" */
/*  *17   20-NOV-1991 12:31:36 CURTIN "Alpha work" */
/*  *16   18-NOV-1991 11:22:22 BUTENHOF "Modify signal_to_cancel_np" */
/*  *15   14-OCT-1991 13:39:20 BUTENHOF "Add get/set guardsize functions" */
/*  *14   27-AUG-1991 17:48:01 BUTENHOF "Fix pthread_join to accept null status" */
/*  *13    2-AUG-1991 15:51:23 CURTIN "added an init check to ptdexc_lock_global_np" */
/*  *12   10-JUN-1991 18:22:42 SCALES "Add sccs headers for Ultrix" */
/*  *11    6-JUN-1991 11:18:38 BUTENHOF "Fix cma_once" */
/*  *10    5-JUN-1991 18:37:50 BUTENHOF "Include cma_px.h" */
/*  *9    14-MAY-1991 13:19:47 CURTIN "Replaced external calls with internal macros" */
/*  *8     2-MAY-1991 13:58:33 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *7    12-APR-1991 23:36:15 BUTENHOF "Change errno access for OSF/1" */
/*  *6     1-APR-1991 18:09:05 BUTENHOF "Add pthread_signal_to_cancel_np()" */
/*  *5    22-MAR-1991 13:42:50 BUTENHOF "Raise error on ptdexc_setcancel w/ bad value" */
/*  *4    21-MAR-1991 09:26:28 BUTENHOF "implement ptdexc_cond_signal_int()" */
/*  *3    12-FEB-1991 01:29:10 BUTENHOF "Redefine alert state" */
/*  *2     5-FEB-1991 00:59:43 BUTENHOF "Change pthread exc interface names" */
/*  *1    12-DEC-1990 21:48:38 BUTENHOF "P1003.4a support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_PTD_EXC.C */



