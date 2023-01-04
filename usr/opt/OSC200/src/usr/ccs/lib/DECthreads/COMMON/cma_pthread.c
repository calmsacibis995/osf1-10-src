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
static char *rcsid = "@(#)$RCSfile: cma_pthread.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/11/19 21:15:48 $";
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
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	19 February 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	5 March 1990
 *		Resolve use of ambiguous symbol cma_c_sched_default (change
 *		to cma_c_sched_use_default where appropriate).
 *	002	Dave Butenhof	23 March 1990
 *		Add one-time init code so caller doesn't have to call
 *		cma_init to use pthreads interface.
 *	003	Dave Butenhof	18 April 1990
 *		Remove default attribute handles to cma_client
 *	004	Dave Butenhof	2 May 1990
 *		Implement cancel.
 *	005	Webb Scales	4 May 1990
 *		Fixed typos in pthread_cond_timedwait (you can't catch 
 *		statuses with a CATCH clause), and added include for stack.h
 *		to pick up the proto for get self.
 *	006	Dave Butenhof	18 May 1990
 *		Change pthread_cond_timedwait to use struct timespec.
 *	007	Dave Butenhof	31 May 1990
 *		Check exception handling.
 *	008	Dave Butenhof	8 June 1990
 *		Reraise an alert in pthread_join, so that cancel isn't
 *		swallowed up.
 *	009	Dave Butenhof	22 June 1990
 *		Update interface to 1003.4a/D3 (May 11, 1990).  This includes
 *		changing synchronization variable parameters from "value" to
 *		"reference", but does not include degenerating attributes
 *		objects to public structures. It also does not include the
 *		new event operations.
 *	010	Dave Butenhof	29 June 1990
 *		Change interface to pthread_get_expiration_np (an in and an
 *		out instead of just one modify).
 *	011	Dave Butenhof	2 July 1990
 *		Implement pthread_mutexattr_setkind_np and
 *		pthread_mutexattr_getkind_np functions.
 *	012	Webb Scales	17 July 1990
 *		Moved return statements out from inside TRY blocks.
 *		Added compatibility for more scheduling policies.
 *	013	Webb Scales	16 August 1990
 *		Added accomodation for Sun platform
 *	014	Dave Butenhof	27 August 1990
 *		Change CMA interfaces to pass handles & structures by reference.
 *	015	Paul Curtin	13 September 1990
 *		Change ftime to cma__ftime.
 *	016	Dave Butenhof	8 October 1990
 *		Fix call to cma_once to include new argument. Also add back
 *		calls to cma_init() on the get/set current thread scheduling
 *		info functions, which might reasonably be called before
 *		creating any objects.
 *	017	Paul Curtin	21 November 1990
 *		Rearranged/removed return status on a number of routines
 *		for consistency and D4 compatibility.
 *	018	Paul Curtin	21 November 1990
 *		Switched the order of the arguments for pthread_keycreate
 *	019	Dave Butenhof	22 January 1991
 *		Fix exception names
 *	020	Dave Butenhof	5 February 1991
 *		Fix internal condition wait call arguments.
 *	021	Dave Butenhof	7 February 1991
 *		Adapt to new alert bit names.
 *	022	Dave Butenhof	20 March 1991
 *		Fix some return statuses to conform to P1003.4a/D4.
 *	023	Dave Butenhof	26 March 1991
 *		Add pthread_signal_to_cancel_np() for OSF DCE
 *	024	Dave Butenhof	03 April 1991
 *		Fix a bug in pthread___cancel_thread().
 *	025	Dave Butenhof	12 April 1991
 *		Use new cma__set_errno rather than direct assignment to
 *		"errno".
 *	026	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	027	Paul Curtin	13 May 1991
 *		Replaced a number of external interface calls
 *		with internale calls and macros. (Optimizations.)
 *	028	Dave Butenhof	29 May 1991
 *		Return ENOSYS for attempts to fiddle with scheduling
 *		parameters under Mach kernel thread implementation (since
 *		Mach threads don't have P1003.4a scheduling yet).
 *	029	Dave Butenhof and Webb Scales	05 June 1991
 *		Include cma_px.h to get struct timespec definition.
 *	030	Dave Butenhof	27 August 1991
 *		Fix bug in join: it should accept NULL status argument.
 *	031	Dave Butenhof	07 October 1991
 *		Add pthread_attr_setguardsize_np &
 *		pthread_attr_getguardsize_np functions.
 *	032	Paul Curtin	20 November 1991
 *		Alpha port: added a number of platform specific changes.
 *	033	Dave Butenhof	05 December 1991
 *		Fix pthread_attr_setsched to support background policy!
 *	034	Dave Butenhof	17 December 1991
 *		Fix bugs in 032: 1) initialize the local that's supposed to
 *		contain the lib$cvt_to_internal_time operation code. 2) Don't
 *		raise an exception on error, since this is the status
 *		returning interface.
 *	035	Dave Butenhof	10 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	036	Dave Butenhof	09 March 1992
 *		The mutex block/unblock functions have just been changed to
 *		return status values. Change pthread mutex lock/unlock to
 *		call this directly instead of using cma__int_[un]lock and
 *		convert status, rather than using expensive TRY/CATCH.
 *	037	Dave Butenhof	10 March 1992
 *		Change 'timeb' to 'timeval' so we can use gettimeofday rather
 *		than the BSD-only ftime function.
 *	038	Dave Butenhof	12 March 1992
 *		Clean up 036 a little.
 *	039	Dave Butenhof	12 March 1992
 *		More integration of direct code: pthread_self,
 *		pthread_getspecific, pthread_setspecific.
 *	040	Dave Butenhof	16 March 1992
 *		Fix typo in 039 for NDEBUG code.
 *	041	Webb Scales	19 May 1992
 *		Reworked signal-to-cancel to avoid holding the global lock 
 *		during thread creation.  Ended up implementing a new, modular
 *		version of it, and making the "pthread_" routines just call
 *		through to the "ptdexc_" routines.
 *	042	Brian Keane, Paul Curtin, Webb Scales	05 August 1992
 *		Re-worked pthread_yield.  It now calls cma__vp_yield directly.
 *	043	Dave Butenhof	24 August 1992
 *		Improve cond performance by calling things directly.
 *	044	Webb Scales	 3 September 1992
 *		Add cond-signal-preempt-int
 *	045	Dave Butenhof	04 September 1992
 *		Fix prototype for cond_sig_preempt_int_np
 *	046	Dave Butenhof	 9 September 1992
 *		Fix pthread_delay_np to use per-thread CV & mutex.
 *	047	Dave Butenhof	10 September 1992
 *		Generalize the check for unsupported POSIX scheduling
 *		functions, using _CMA_POSIX_SCHED_ rather than assuming Mach
 *		thread systems don't support it.
 *	048	Dave Butenhof	10 September 1992
 *		Remove accidentally pasted #if line.
 *	049	Dave Butenhof	15 September 1992
 *		For _CMA_RT4_KTHREAD_, all non-realtime scheduling policies
 *		are mapped into SCHED_OTHER; avoid duplicate cases in switch
 *		statements.
 *	050	Dave Butenhof	16 September 1992
 *		Add pthread_equal function (even though we define it as a
 *		macro in pthread.h -- this is only to make DEC OSF/1 happy).
 *	051	Dave Butenhof	23 September 1992
 *		Fix pthread_cond_timedwait to return -1 & errno = EAGAIN on
 *		timeout, rather than returning EAGAIN & errno = 0.
 *	052	Webb Scales	23 September 1992
 *		Add the "ada-rtb" scheduling policy.
 *	053	Dave Butenhof	30 October 1992
 *		Use int_signal and int_broadcast macros instead of directly
 *		accessing cma__int_cond_wake function, to isolate the code
 *		from interface changes.
 *	054	Dave Butenhof	25 November 1992
 *		Remove special cases for RT4_KTHREAD policies; all are now
 *		defined uniquely.
 *	055	Dave Butenhof	 2 December 1992
 *		pthread_create() -- return EPERM if no priv to set realtime
 *		scheduling.
 *	056	Dave Butenhof	29 January 1993
 *		pthread_setscheduler() should return the previous priority,
 *		not the previous scheduling policy.
 *	057	Dave Butenhof	 2 February 1993
 *		Auto-init on global lock.
 *	058	Dave Butenhof	12 February 1993
 *		(thought we already did this?) Validate timer values, so that
 *		select() can't complain later.
 *	059	Dave Butenhof	 2 March 1993
 *		I eliminated TCB locking in alert state modifications for the
 *		CMA interface long ago, but neglected to do so in the 1003.4a
 *		interfaces. So do it!
 *	060	Webb Scales	24 March 1993
 *		Handle carries properly when converting timespec's to VMS
 *		time format for timed condition waits.
 *	061	Brian Keane	30 March 1993
 *		Make internal return value of pthread_attr_getstacksize a
 *		long, to handle 64 bit values.
 *	062	Brian Keane	30 March 1993
 *		Add self-deadlock detection to pthread_join.
 *	063	Dave Butenhof	31 March 1993
 *		Use better names for user objects.
 *	064	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	065	Dave Butenhof	 3 May 1993
 *		Inline pthread_detach()
 *	066	Dave Butenhof	11 May 1993
 *		Make pthread_detach() more complicated again: it needs to
 *		move zombie thread to front of known thread queue "dead zone"
 *		so the zombie hunter can find it.
 *	067	Dave Butenhot	13 May 1993
 *		Brian discovered pthread_detach() bug -- it doesn't return 0
 *		on success. Repair.
 *	068	Brian Keane	10 June 1993
 *		Touch up conditional code in pthread_cond_sig_preempt_int_np  - 
 *		cma__int_signal_preempt_int is a macro when _CMA_THREAD_IS_VP_
 *		is defined, and some compilers don't like #ifs in macro 
 *		expansions.
 *	069	Dave Butenhof	26 July 1993
 *		pthread_create() dequeues known TCB on failure, without
 *		locking kernel. Tsk tsk.
 *	070	Brian Keane	20 September 1993
 *		pthread_setprio should set errno to ENOSYS if the cma_e_unimp
 *		exception is raised.
 *	071	Brian Keane	20 September 1993
 *		In pthread_create(), detect exc_s_exquota status from 
 *		cma__int_make_thread() 	and set errno to EAGAIN.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_px.h>			/* Get struct timespec */
#include <cma_kernel.h>
#include <cma_util.h>
#include <pthread.h>
#include <cma_deb_core.h>
#include <cma_int_errno.h>
#if _CMA_OS_ == _CMA__VMS
# include <time.h>
#else
# include <sys/time.h>
#endif
#include <cma_tcb_defs.h>
#include <cma_attr.h>
#include <cma_alert.h>
#include <cma_mutex.h>
#include <cma_stack.h>
#include <cma_condition.h>
#include <cma_init.h> 
#include <cma_context.h>
#include <cma_tcb.h>
#include <cma_vp.h>

#if _CMA_OS_ == _CMA__VMS
# include <lib$routines.h>
# include <libdtdef.h>
#endif

#if _CMA_OS_ == _CMA__UNIX
# include <signal.h>
#endif

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

#if _CMA_OS_ == _CMA__VMS
/*
 * Conversion: VMS absolute time of UNIX "epoch", 00:00:00 Jan 1, 1970
 */
 static cma_t_date_time pthread___g_epoch = {0x4BEB4000, 0x007C9567};
#endif

static char	*cma___g_user_obj = "<pthread user@0x%lx>";

/*
 * LOCAL MACROS
 */

#ifndef ENOSYS
# define ENOSYS	EINVAL
#endif

/*
 * LOCAL FUNCTIONS
 */

static void
pthread___unlock_int_mutex _CMA_PROTOTYPE_ ((pthread_addr_t arg));


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
extern int
pthread_attr_create
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	*attr;
#endif	/* prototype */
    {
    int	status = 0;
    cma__t_int_attr     *att_obj;


    cma__int_init ();
    att_obj = cma__get_attributes (&cma__g_def_attr);

    if ((pthread_addr_t)att_obj == cma_c_null_ptr) {
	cma__set_errno (ENOMEM);
	status = -1;
	}
    else {
	cma__object_to_handle ((cma__t_object *)att_obj, (cma_t_attr *)attr);
	cma__obj_set_owner (att_obj, (cma_t_integer)attr);
	cma__obj_set_name (att_obj, "<pthread user(thread)@0x%lx>");
	}

    return status;
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
extern int
pthread_attr_delete
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    if (cma__val_nullattr_stat (attr, &int_att) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (int_att == (cma__t_int_attr *)cma_c_null_ptr)
	return 0;

    cma__free_attributes (int_att);
    cma__clear_handle (attr);
    return 0;
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
extern int
pthread_attr_setprio
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
    int	status = 0;


    TRY {
	cma_attr_set_priority (attr, (cma_t_priority)priority);
	}
    CATCH (exc_e_nopriv) {
	cma__set_errno (EPERM);
	status = -1;
	}
    CATCH (cma_e_badparam) {
	cma__set_errno (ERANGE);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_attr_getprio
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_priority	priority;


    TRY {
	cma__int_attr_get_priority (&attr, &priority);
	ret_val = (int)priority;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

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
extern int
pthread_attr_setsched
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
    int			status = 0;
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
	    cma__set_errno (EINVAL);
	    return -1;
	    break;
	    }

	}

    TRY {
	cma_attr_set_sched (attr, policy, priority);
	}
    CATCH (exc_e_nopriv) {
	cma__set_errno (EPERM);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_attr_getsched
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


    TRY {
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
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

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
extern int
pthread_attr_setinheritsched
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
    int			status = 0;
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
	    cma__set_errno (EINVAL);
	    return -1;
	    break;
	    }
	}

    TRY {
	cma_attr_set_inherit_sched (attr, flag);
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_attr_getinheritsched
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


    TRY {
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
		cma__bugcheck ("attr_getinheritsched:1");
		break;
		}
	    }

	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
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
extern int
pthread_attr_setstacksize
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
    int	status = 0;


    TRY {
	cma__int_attr_set_stacksize (attr, (cma_t_natural)stacksize);
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_attr_getstacksize
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    long		ret_val = 0;
    cma_t_natural	stacksize;


    TRY {
	cma__int_attr_get_stacksize (&attr, &stacksize);
	ret_val = (long)stacksize;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
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
extern int
pthread_attr_setguardsize_np
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
    int	status = 0;


    TRY {
	cma__int_attr_set_guardsize (attr, (cma_t_natural)guardsize);
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_attr_getguardsize_np
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


    TRY {
	cma__int_attr_get_guardsize (&attr, &guardsize);
	ret_val = (long)guardsize;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

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
 *	0 if success, -1 if error (error code in errno).
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_create
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
    cma__t_queue	*tq;
    cma_t_boolean	error;


    cma__int_init ();

    /*    
     * Get internal pointer to att object (and validate it while we're at it)
     */
    if (cma__val_defattr_stat (&attr, &new_att) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
		
    /* 
     * Get a TCB (and stack) for the new thread
     */
    cma__dispell_zombies ();		/* see if we can free a tcb/stack */
    new_tcb = cma__get_tcb (new_att);

    if ((cma_t_address)new_tcb == cma_c_null_ptr) {
	cma__set_errno (ENOMEM);
	return -1;
	}

    switch (cma__int_make_thread (new_tcb, thread, start_routine, arg)) {
	case cma_s_normal :
	    error = cma_c_false;
	    break;
	case exc_s_nopriv : 
	    error = cma_c_true;
	    cma__set_errno (EPERM);
	    break;
	case exc_s_insfmem : 
	case exc_s_exquota : 
	    error = cma_c_true;
	    cma__set_errno (EAGAIN);
	    break;
	default :
	    error = cma_c_true;
	    cma__set_errno (EINVAL);
	    break;
	}

    if (error) {
	cma__enter_kernel ();
	cma__queue_remove (&new_tcb->threads, tq, cma__t_queue);
	cma__g_threadcnt--;
	cma__exit_kernel ();
	cma__free_tcb (new_tcb);
	return -1;
	}

    /*
     * Point user's handle at the new thread object
     */
    cma__object_to_handle ((cma__t_object *)new_tcb, (cma_t_thread *)thread);
    cma__obj_set_owner (new_tcb, (cma_t_integer)thread);
    cma__obj_set_name (new_tcb, cma___g_user_obj);
    return 0;
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
 *	0 if success, -1 if error (error code in errno).
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_join
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
    int			retstatus = 0;
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
    TRY {
	cma_thread_join (&thread, &thdstatus, &result);

	if (status != (pthread_addr_t *)cma_c_null_ptr) {
	    if (thdstatus == cma_c_term_normal)
		*status = (pthread_addr_t)result;
	    else
		*status = (pthread_addr_t)-1;
	    }

	}
    CATCH (cma_e_alerted) {
	RERAISE;
	}
    CATCH (cma_e_use_error) {
	cma__set_errno (ESRCH);
	retstatus = -1;
	}
    CATCH (cma_e_in_use) {
	cma__set_errno (EDEADLK);
	retstatus = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	retstatus = -1;
	}
    ENDTRY

    return retstatus;
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
 *	0 if success, -1 if error (error code in errno).
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_detach
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
    if (cma__val_nulltcb_stat (thread, &tcb) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    /*
     * If thread is a null handle, do nothing and return
     */
    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr)
	return 0;

    cma__int_detach (tcb);

    /*
     * This handle is no longer valid, clear it
     */
    cma__clear_handle (thread);
    return 0;
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
pthread_exit
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
     * Also, notice that no exceptions are caught: the CMA exit routines can
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
pthread_self
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;
    cma_t_thread	thread;


    /*
     * NOTE: cma__get_self_tcb () (on a multiprocessor) may raise the
     * exception cma_e_notcmastack to indicate that it can't determine the
     * thread identity because the client code has switched to a stack not
     * controlled by DECthreads. If this happens, we really can't return a
     * meaningful answer, nor does pthread_self() provide for an error
     * return. Thus, we might as well just let the exception be raised for
     * now, so the thread will terminate in a controlled way.
     */
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
pthread_setprio
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
#if !_CMA_POSIX_SCHED_
    return (cma__set_errno (ENOSYS), -1);
#else
    int			ret_val = 0;
    cma_t_priority	oldpri;


    cma__int_init ();
    TRY {
	cma_thread_get_priority (&thread, &oldpri);
	cma_thread_set_priority (&thread, (cma_t_priority)priority);
	ret_val = (int)oldpri;
	}
    CATCH (exc_e_nopriv) {
	cma__set_errno (EPERM);
	ret_val = -1;
	}
    CATCH (cma_e_use_error) {
	cma__set_errno (ESRCH);
	ret_val = -1;
	}
    CATCH (cma_e_unimp) {
	cma__set_errno (ENOSYS);
	ret_val = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
#endif
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
pthread_getprio
#ifdef _CMA_PROTO_
	(
	pthread_t	thread)
#else	/* no prototypes */
	(thread)
	pthread_t	thread;
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    return (cma__set_errno (ENOSYS), -1);
#else
    int			ret_val = 0;
    cma_t_priority	priority;


    cma__int_init ();
    TRY {
	cma_thread_get_priority (&thread, &priority);
	ret_val = (int)priority;
	}
    CATCH (cma_e_use_error) {
	cma__set_errno (ESRCH);
	ret_val = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
#endif
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
pthread_setscheduler
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
#if !_CMA_POSIX_SCHED_
    return (cma__set_errno (ENOSYS), -1);
#else
    int			ret_val = 0;
    cma_t_priority	oldpriority;


    cma__int_init ();

    TRY {
	cma_thread_get_priority (&thread, &oldpriority);
	cma_thread_set_sched (
		&thread,
		(cma_t_sched_policy)scheduler,
		(cma_t_priority)priority);
	ret_val = (int)oldpriority;
	}
    CATCH (exc_e_nopriv) {
	cma__set_errno (EPERM);
	ret_val = -1;
	}
    CATCH (cma_e_unimp) {
	cma__set_errno (ENOSYS);
	ret_val = -1;
	}
    CATCH (cma_e_use_error) {
	cma__set_errno (ESRCH);
	ret_val = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
#endif
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
pthread_getscheduler
#ifdef _CMA_PROTO_
	(
	pthread_t	thread)
#else	/* no prototypes */
	(thread)
	pthread_t	thread;
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    return (cma__set_errno (ENOSYS), -1);
#else
    int			ret_val = 0;
    cma_t_sched_policy	scheduler;


    cma__int_init ();
    TRY {
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
		cma__bugcheck ("getscheduler:1");
		break;
		}
	    }

	}
    CATCH (cma_e_use_error) {
	cma__set_errno (ESRCH);
	ret_val = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
#endif
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
pthread_yield
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
 *	0 if successful, else -1 (with error in errno)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_once
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
    int	status = 0;


    cma__int_init ();

    TRY {
	cma_once (
		(cma_t_once *)once_block,
		(cma_t_init_routine)init_routine,
		cma_c_null_ptr);
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
 *	0 if successful, else -1 (with error in errno)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_keycreate
#ifdef _CMA_PROTO_
	(
	pthread_key_t		*key,
	pthread_destructor_t	destructor)
#else	/* no prototypes */
	(key, destructor)
	pthread_key_t		*key;
	pthread_destructor_t	destructor;
#endif	/* prototype */
    {
    int	status = 0;


    cma__int_init ();

    TRY {
	cma_key_create (key, &cma_c_null, destructor);
	}
    CATCH (exc_e_exquota) {
	cma__set_errno (ENOMEM);
	status = -1;
	}
    CATCH (exc_e_insfmem) {
	cma__set_errno (EAGAIN);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
 *	0 if successful, else -1 (with error in errno)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_setspecific
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

    if ((key <= 0) || (key >= next_context)) {
	cma__set_errno (EINVAL);
	return -1;
	}

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
	    cma__set_errno (ENOMEM);
	    return -1;
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
    return 0;
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
 *	0 if successful, else -1 (with error in errno)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_getspecific
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

    if ((key <= 0) || (key >= next_context)) {
	cma__set_errno (EINVAL);
	return -1;
	}

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

    return 0;
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
extern int
pthread_mutexattr_create
#ifdef _CMA_PROTO_
	(
	pthread_mutexattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_mutexattr_t	*attr;
#endif	/* prototype */
    {
    int	status = 0;
    cma__t_int_attr     *att_obj;


    cma__int_init ();
    att_obj = cma__get_attributes (&cma__g_def_attr);

    if ((pthread_addr_t)att_obj == cma_c_null_ptr) {
	cma__set_errno (ENOMEM);
	status = -1;
	}
    else {
	cma__object_to_handle ((cma__t_object *)att_obj, (cma_t_attr *)attr);
	cma__obj_set_owner (att_obj, (cma_t_integer)attr);
	cma__obj_set_name (att_obj, "<pthread user(mutex)@0x%lx>");
	}

    return status;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_mutexattr_delete
#ifdef _CMA_PROTO_
	(
	pthread_mutexattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_mutexattr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    if (cma__val_nullattr_stat (attr, &int_att) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (int_att == (cma__t_int_attr *)cma_c_null_ptr)
	return 0;

    cma__free_attributes (int_att);
    cma__clear_handle (attr);
    return 0;
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
extern int
pthread_mutexattr_setkind_np
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
    int	status = 0;


    TRY {
	cma_attr_set_mutex_kind (attr, (cma_t_mutex_kind)kind);
	}
    CATCH (exc_e_nopriv) {
	cma__set_errno (EPERM);
	status = -1;
	}
    CATCH (cma_e_badparam) {
	cma__set_errno (ERANGE);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_mutexattr_getkind_np
#ifdef _CMA_PROTO_
	(
	pthread_attr_t	attr)
#else	/* no prototypes */
	(attr)
	pthread_attr_t	attr;
#endif	/* prototype */
    {
    int			ret_val = 0;
    cma_t_mutex_kind	kind;


    TRY {
	cma_attr_get_mutex_kind (&attr, &kind);
	ret_val = (int)kind;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	ret_val = -1;
	}
    ENDTRY

    return ret_val;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_mutex_init
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

    if (cma__val_defattr_stat (&attr, &int_att) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    imutex = cma__get_mutex (int_att);

    if ((cma_t_address)imutex == cma_c_null_ptr) {
	cma__set_errno (ENOMEM);
	return -1;
	}
    else {
	cma__object_to_handle ((cma__t_object *)imutex, mutex);
	cma__obj_set_owner (imutex, (cma_t_integer)mutex);
	cma__obj_set_name (imutex, cma___g_user_obj);
	}

    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_mutex_destroy
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    int	status = 0;


    TRY {
	cma_mutex_delete (mutex);
	}
    CATCH (cma_e_in_use) {
	cma__set_errno (EBUSY);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_mutex_lock
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    int			status = 0;
    cma__t_int_mutex    *int_mutex;
    cma_t_status	res;
#ifndef NDEBUG
    cma__t_int_tcb	*ltcb;
#endif


    /*
     * Get a pointer to the mutex structure; if this is a debugging CMA,
     * we'll validate the mutex handle to be sure it's valid.  For
     * performance, if it's an NDEBUG ("production") CMA, just fetch the
     * address of the object from the handle's pointer field.
     */
#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;

    if (cma__test_and_set (&int_mutex->lock))
	res = cma__int_mutex_block (int_mutex);
    else
	res = cma_s_normal;

#else

    if (cma__val_mutex_stat (mutex, &int_mutex) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    ltcb = cma__get_self_tcb ();

    if (cma__test_and_set (&int_mutex->lock))
	res = cma__int_mutex_block (int_mutex);
    else
	res = cma_s_normal;

#endif

    switch (res) {
	case cma_s_normal : {
#ifndef NDEBUG
	    int_mutex->owner = ltcb;
#endif
	    return 0;
	    }
	case cma_s_in_use : {
	    cma__set_errno (EDEADLK);
	    return -1;
	    }
	default : {
	    cma__set_errno (EINVAL);
	    return -1;
	    }

	}

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
 *	0 if already locked; 1 if successful, -1 if error
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_mutex_trylock
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_mutex_unlock
#ifdef _CMA_PROTO_
	(
	pthread_mutex_t	*mutex)
#else	/* no prototypes */
	(mutex)
	pthread_mutex_t	*mutex;
#endif	/* prototype */
    {
    cma__t_int_mutex    *int_mutex;
    cma_t_status	res;


#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    cma__t_int_tcb	*utcb;


    if (cma__val_mutex_stat (mutex, &int_mutex) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    utcb = cma__get_self_tcb ();

    if (int_mutex->mutex_kind == cma_c_mutex_fast) {
	cma__assert_warn (
		(utcb == int_mutex->owner),
		"attempt to release mutex owned by another thread");
	int_mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;
	}

#endif
    cma__unset (int_mutex->unlock);

    if (!cma__test_and_set (&int_mutex->event)) {
	res = cma__int_mutex_unblock (int_mutex);

	if (res != cma_s_normal) {
	    cma__set_errno (EINVAL);
	    return -1;
	    }

	}

    return 0;
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
extern int
pthread_condattr_create
#ifdef _CMA_PROTO_
	(
	pthread_condattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_condattr_t	*attr;
#endif	/* prototype */
    {
    int	status = 0;
    cma__t_int_attr     *att_obj;


    cma__int_init ();
    att_obj = cma__get_attributes (&cma__g_def_attr);

    if ((pthread_addr_t)att_obj == cma_c_null_ptr) {
	cma__set_errno (ENOMEM);
	status = -1;
	}
    else {
	cma__object_to_handle ((cma__t_object *)att_obj, (cma_t_attr *)attr);
	cma__obj_set_owner (att_obj, (cma_t_integer)attr);
	cma__obj_set_name (att_obj, "<pthread user(cond)@0x%lx>");
	}

    return status;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_condattr_delete
#ifdef _CMA_PROTO_
	(
	pthread_condattr_t	*attr)
#else	/* no prototypes */
	(attr)
	pthread_condattr_t	*attr;
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    if (cma__val_nullattr_stat (attr, &int_att) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (int_att == (cma__t_int_attr *)cma_c_null_ptr)
	return 0;

    cma__free_attributes (int_att);
    cma__clear_handle (attr);
    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_init
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
    cma__t_int_cv	*cv; 
    cma__t_int_attr	*int_att; 


    cma__int_init ();

    if (cma__val_defattr_stat (&attr, &int_att) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    cv = cma__get_cv (int_att);

    if ((cma_t_address)cv == cma_c_null_ptr) {
	cma__set_errno (ENOMEM);
	return -1;
	}
    else {
	cma__object_to_handle ((cma__t_object *)cv, cond);
	cma__obj_set_owner (cv, (cma_t_integer)cond);
	cma__obj_set_name (cv, cma___g_user_obj);
	}

    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_destroy
#ifdef _CMA_PROTO_
	(
	pthread_cond_t	*cond)
#else	/* no prototypes */
	(cond)
	pthread_cond_t	*cond;
#endif	/* prototype */
    {
    int	status = 0;


    TRY {
	cma__int_cond_delete (cond);
	}
    CATCH (cma_e_in_use) {
	cma__set_errno (EBUSY);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_signal
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
    if (cma__val_cv_stat (cond, &int_cv) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
#endif

    cma__int_signal (int_cv);
    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_signal_int_np
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
    if (cma__val_cv_stat (cond, &int_cv) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
#endif

    cma__int_signal_int (int_cv);
    return 0;
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
extern int
pthread_cond_sig_preempt_int_np 
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
    cma__t_int_cv       *int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)cond)->pointer;
#else
    if (cma__val_cv_stat (cond, &int_cv) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
#endif

#if _CMA_OS_ == _CMA__UNIX
    cma__int_signal_preempt_int (int_cv, scp);
#else
    cma__int_signal_preempt_int (int_cv);
#endif

    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_broadcast
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
    if (cma__val_cv_stat (cond, &int_cv) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
#endif

    /*
     * Unblock all waiters
     */
    cma__int_broadcast (int_cv);
    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_wait
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
    cma__t_int_mutex    *int_mu;
    cma__t_int_cv       *int_cv;


#ifdef NDEBUG
    int_mu = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)cond)->pointer;
#else
    if (cma__val_mutex_stat (mutex, &int_mu) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (cma__val_cv_stat (cond, &int_cv) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
#endif

    cma__int_wait (int_cv, int_mu, cma__get_self_tcb());
    return 0;
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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cond_timedwait
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
    int			status = 0;
    cma_t_status	retval;
    cma_t_date_time	time;
#if _CMA_OS_ == _CMA__VMS
    cma_t_date_time	delta;
    int			vstat;
    int                 delta_secs;
#endif


#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)cond)->pointer;
#else
    if (cma__val_mutex_stat (mutex, &int_mutex) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}

    if (cma__val_cv_stat (cond, &int_cv) != cma_s_normal) {
	cma__set_errno (EINVAL);
	return -1;
	}
#endif

    if (abstime->tv_nsec >= (1000 * 1000000) || abstime->tv_nsec < 0) {
	cma__set_errno (EINVAL);
	return -1;
	}    

#if _CMA_OS_ == _CMA__VMS
    delta_secs = LIB$K_DELTA_SECONDS;
    vstat = lib$cvt_to_internal_time (
            &delta_secs,
	    &abstime->tv_sec,
	    &delta);

    if ((vstat & 7) != 1) {
	cma__set_errno (EINVAL);
	return -1;
	}

    /*
     * Adjust the resulting delta time by the number of nanoseconds in the
     * timespec.  Note that VMS delta time is negative, so we subtract. Be
     * careful to handle the carry properly.
     */
    if (delta.low < ((abstime->tv_nsec + 99) / 100))
	delta.high -= 1;
    delta.low -= ((abstime->tv_nsec + 99) / 100);
    lib$add_times (&pthread___g_epoch, &delta, &time);
#else
    time.tv_sec = abstime->tv_sec;
    time.tv_usec = (abstime->tv_nsec + 999) / 1000;
#endif

    if (cma__int_timed_wait (int_cv, int_mutex, &time, cma__get_self_tcb ())
	    == cma_s_timed_out) {
	cma__set_errno (EAGAIN);
	return -1;
	}

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
 *	0 if successful, else -1
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_cancel
#ifdef _CMA_PROTO_
	(
	pthread_t	thread)
#else	/* no prototypes */
	(thread)
	pthread_t	thread;
#endif	/* prototype */
    {
    int	status = 0;


    TRY {
	cma_thread_alert (&thread);
	}
    CATCH (cma_e_use_error) {
	cma__set_errno (ESRCH);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
pthread_testcancel
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
pthread_setcancel
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

    if (state != CANCEL_ON && state != CANCEL_OFF) {
	cma__set_errno (EINVAL);
	return -1;
	}

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
pthread_setasynccancel
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

    if (state != CANCEL_ON && state != CANCEL_OFF) {
	cma__set_errno (EINVAL);
	return -1;
	}

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
 *	0 if successful, else -1 and errno set to error code
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_get_expiration_np
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


    if (delta->tv_nsec >= (1000 * 1000000) || delta->tv_nsec < 0) {
	cma__set_errno (EINVAL);
	return -1;
	}

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
 *	0 if successful, else -1 and errno set to error code
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_delay_np
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

    if (interval->tv_nsec >= (1000 * 1000000) || interval->tv_nsec < 0) {
	cma__set_errno (EINVAL);
	return -1;
	}

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
    pthread_cleanup_push (pthread___unlock_int_mutex, self->tswait_mutex);

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
    return 0;
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
pthread_lock_global_np
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
pthread_unlock_global_np
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
 *	handle parameter to allow caller to control the server thread.
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
 *	On error, returns -1 after setting ERRNO; otherwise returns zero.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int 
pthread_sig_to_can_thread_np
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
    int status;


    TRY {
	ptdexc_sig_to_can_thread_np (sigset, target, thread);
	status = 0;
	}
    CATCH (exc_e_insfmem) {
	cma__set_errno (ENOMEM);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
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
 *	On error, returns -1 after setting ERRNO; otherwise returns zero.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern int
pthread_signal_to_cancel_np
#ifdef _CMA_PROTO_
	(sigset_t	*sigset,
	pthread_t	*target)
#else
	(sigset, target)
	sigset_t	*sigset;
	pthread_t	*target;
#endif
    {
    int status;


    TRY {
	ptdexc_signal_to_cancel_np (sigset, target);
	status = 0;
	}
    CATCH (exc_e_insfmem) {
	cma__set_errno (ENOMEM);
	status = -1;
	}
    CATCH_ALL {
	cma__set_errno (EINVAL);
	status = -1;
	}
    ENDTRY

    return status;
    }
#endif

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
pthread___unlock_int_mutex
#ifdef _CMA_PROTO_
	(pthread_addr_t	arg)
#else	/* no prototypes */
	(arg)
	pthread_addr_t	arg;
#endif	/* prototype */
    {
    cma__int_unlock ((cma__t_int_mutex *)arg);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Compare two thread handles. This doesn't go in any transfer vectors,
 *	and there isn't a pthread exception version. Our pthread.h header
 *	supplied a more efficient macro version. This routine can be called
 *	by non-C languages, but is primarily there to satisfy DEC OSF/1
 *	requirements for 'nm' compatibility with the OSF/1 libpthreads.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread1		address of thread handle
 *	thread2		address of thread handle
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
 *	TRUE (1) if handles are equal, else FALSE (0)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#undef pthread_equal			/* Remove the macro */
extern int
pthread_equal
#ifdef _CMA_PROTO_
	(pthread_t	thread1,
	pthread_t	thread2)
#else	/* no prototypes */
	(thread1, thread2)
	pthread_t	thread1;
	pthread_t	thread2;
#endif	/* prototype */
    {
    return ((thread1.field1 == thread2.field1)
	&& (thread1.field2 == thread2.field2)
	&& (thread1.field3 == thread2.field3));
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_PTHREAD.C */
/*  *56   20-SEP-1993 09:54:23 KEANE "Fix errno return in pthread_setprio" */
/*  *55   26-JUL-1993 13:35:09 BUTENHOF "Fix thread create error path" */
/*  *54   16-JUN-1993 10:48:23 BUTENHOF "Fix obj_name on can_sig mutex" */
/*  *53   15-JUN-1993 08:00:02 KEANE "Touch up cond_signal_preempt_int for DEC C on OSF/1 AXP" */
/*  *52   14-MAY-1993 15:56:09 BUTENHOF "Detach needs to move terminated thread after g_last_thread" */
/*  *51    3-MAY-1993 13:44:43 BUTENHOF "Make pthread_detach local" */
/*  *50   16-APR-1993 13:04:41 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *49    1-APR-1993 14:33:07 BUTENHOF "Add names to user objects" */
/*  *48   31-MAR-1993 15:16:32 KEANE "Add self-deadlock detection to join (back propagate from OSF/1)" */
/*  *47   30-MAR-1993 09:44:00 KEANE "Fix minor 64 bit problem in attr_get_stacksize" */
/*  *46   24-MAR-1993 20:07:57 SCALES "Fix gotcha in timespec to VMS time conversion" */
/*  *45    2-MAR-1993 13:08:29 BUTENHOF "Some performance twiddling" */
/*  *44   12-FEB-1993 11:32:47 BUTENHOF "Validate wait times" */
/*  *43    2-FEB-1993 07:06:37 BUTENHOF "auto-init on global lock" */
/*  *42   29-JAN-1993 12:46:21 BUTENHOF "Fix setscheduler to return priority, not policy" */
/*  *41    2-DEC-1992 13:29:00 BUTENHOF "Allow stack size of 0 (round up)" */
/*  *40    1-DEC-1992 14:05:37 BUTENHOF "OSF/1 scheduling" */
/*  *39    2-NOV-1992 13:25:25 BUTENHOF "Fix cond_wake calls" */
/*  *38   24-SEP-1992 08:56:47 SCALES "Add ""ada-rtb"" scheduling policy" */
/*  *37   23-SEP-1992 12:52:55 BUTENHOF "Fix status return" */
/*  *36   16-SEP-1992 14:02:42 BUTENHOF "Add pthread_equal routine" */
/*  *35   15-SEP-1992 13:50:07 BUTENHOF "Fix sched case stmnts" */
/*  *34   10-SEP-1992 12:23:34 BUTENHOF "Fix typo" */
/*  *33   10-SEP-1992 09:46:47 BUTENHOF "Fix delay" */
/*  *32    4-SEP-1992 14:42:22 BUTENHOF "Fix preempt function prototype" */
/*  *31    3-SEP-1992 15:57:24 SCALES "Add cond-signal-preempt-int" */
/*  *30    2-SEP-1992 16:25:42 BUTENHOF "Separate semaphores from kernel lock" */
/*  *29   25-AUG-1992 16:15:50 KEANE "Fix minor typo" */
/*  *28   25-AUG-1992 11:48:42 BUTENHOF "Adjust Mach yield operations" */
/*  *27    5-AUG-1992 21:52:07 KEANE "Make yield on mach platforms avoid entering kernel" */
/*  *26   22-MAY-1992 17:43:53 SCALES "Rework signal-to-cancel to remove use of global lock" */
/*  *25   16-MAR-1992 09:04:18 BUTENHOF "Fix typo in NDEBUG code" */
/*  *24   13-MAR-1992 14:09:27 BUTENHOF "Fix mutex lock/unlock code" */
/*  *23   10-MAR-1992 16:26:54 BUTENHOF "Eliminate need for TRY/CATCH on pthread mutex lock" */
/*  *22   19-FEB-1992 07:16:20 BUTENHOF "Fix type cast" */
/*  *21   18-FEB-1992 15:30:00 BUTENHOF "Adapt to alloc_mem changes" */
/*  *20   18-DEC-1991 06:45:25 BUTENHOF "Fix timedwait" */
/*  *19    6-DEC-1991 07:19:50 BUTENHOF "Add background policy to attr_setsched" */
/*  *18   20-NOV-1991 12:34:54 CURTIN "Alpha work" */
/*  *17   18-NOV-1991 11:22:36 BUTENHOF "Modify signal_to_cancel_np" */
/*  *16   14-OCT-1991 13:39:32 BUTENHOF "Add get/set guardsize functions" */
/*  *15   27-AUG-1991 17:48:12 BUTENHOF "Fix pthread_join to accept null status" */
/*  *14   10-JUN-1991 18:22:48 SCALES "Add sccs headers for Ultrix" */
/*  *13    5-JUN-1991 18:37:56 BUTENHOF "Include cma_px.h" */
/*  *12   29-MAY-1991 17:58:50 BUTENHOF "Return ENOSUP for sched ops on Mach threads" */
/*  *11   14-MAY-1991 12:44:29 CURTIN "Added some macros" */
/*  *10    2-MAY-1991 13:58:45 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *9    12-APR-1991 23:36:23 BUTENHOF "Change errno access for OSF/1" */
/*  *8     3-APR-1991 13:40:31 BUTENHOF "Fix signal_to_cancel_np()" */
/*  *7     1-APR-1991 18:09:16 BUTENHOF "Add pthread_signal_to_cancel_np()" */
/*  *6    22-MAR-1991 13:43:05 BUTENHOF "Check param value for setasynccancel, too" */
/*  *5    21-MAR-1991 09:26:39 BUTENHOF "Fix some return statuses" */
/*  *4    12-FEB-1991 01:29:19 BUTENHOF "Redefine alert state" */
/*  *3     6-FEB-1991 01:33:00 BUTENHOF "Fix internal timed_wait call" */
/*  *2    24-JAN-1991 00:35:03 BUTENHOF "Fix exception name references" */
/*  *1    12-DEC-1990 21:48:32 BUTENHOF "P1003.4a support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_PTHREAD.C */
