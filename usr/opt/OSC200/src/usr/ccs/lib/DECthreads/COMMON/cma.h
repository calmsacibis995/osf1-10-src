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
 *	@(#)$RCSfile: cma.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/11/23 23:42:40 $
 */
/*
 *  FACILITY:
 *
 *	Digital's Proprietary Interface to DECthreads (CMA)
 *
 *  ABSTRACT:
 *
 *	External definitions for CMA services
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 * 
 *	2 November 1988
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Bob Conti
 *	Paul Curtin
 *	Hans Oser
 *	Webb Scales
 */

#ifndef CMA_INCLUDE
#define CMA_INCLUDE

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * Define a symbol which client code can test to determine whether the 
 * underlying threads package is DECthreads or not.  This is especially
 * important to clients of the Pthreads interface who may want to use
 * certain DECthreads extensions, such as the global lock and non-real-time
 * scheduling policies, on the DECthreads platform while maintaining 
 * portability to a "vanilla" pthreads implementation.
 */
#define _DECTHREADS_	1

/*
 * The implementation makes these basic decisions
 */

#if defined(vms) || defined(__vms) || defined (VMS) || defined(__VMS) || defined(__vms__)
# include <cma_config.h>		/* configuration symbols */
# include <exc_handling.h>
#else
# include <dce/cma_config.h>		/* configuration symbols */
# include <dce/exc_handling.h>
#endif

#if _CMA_OSIMPL_ == _CMA__OS_OSF
/*
 * The OSF/1 unistd.h defines POSIX and OSF programming environment symbols
 * if none of the recognized symbols are already defined, but defines no more
 * if any are already defined. To retain as much as possible of the
 * application's desired environment, we'll include unistd.h first to get
 * whatever it thinks we need -- and then define what we really do need if it
 * didn't.
 *
 * We need _POSIX_4SOURCE to get the POSIX scheduling constants, and we need
 * _OSF_SOURCE and RT_SCHED as well -- define them next if they aren't
 * already there. Then include sched.h to get the scheduling constants.
 */
# include <unistd.h>
# if _CMA_RT4_KTHREAD_
#  ifndef _POSIX_4SOURCE
#   define _POSIX_4SOURCE 1
#  endif
#  ifndef _OSF_SOURCE
#   define _OSF_SOURCE    1
#  endif
#  ifndef RT_SCHED
#   define RT_SCHED	 1
#  endif
#  include <sched.h>
# endif
#endif

#if _CMA_OS_ != _CMA__VMS
# include <sys/types.h>
# include <sys/time.h>
#endif

#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
# define _REENTRANT	1		/* Enable reentrant errno.h */
#endif

/*
 * Sample decisions for the environment types
 */


typedef long			cma_t_integer;

typedef unsigned long		cma_t_natural;

typedef cma_t_natural		cma_t_boolean;

typedef float			cma_t_interval;

typedef	cma_t_integer		cma_t_key;

typedef cma_t_integer		cma_t_status;

typedef	cma_t_integer		cma_t_priority;

typedef	cma_t_integer		cma_t_sched_policy;

typedef int			cma_t_errno;

#if _CMA_VOID_
typedef void			*cma_t_address;
#else
typedef char			*cma_t_address;
#endif

#define cma_c_false	(cma_t_boolean)0
#define cma_c_true	(cma_t_boolean)1
#define cma_c_null_ptr	(cma_t_address)0

/*
 * If we're on a system with POSIX 1003.4 support, use the system symbols for
 * scheduling policy.
 */
#if _CMA_RT4_KTHREAD_
# define cma_c_sched_fifo	SCHED_FIFO
# define cma_c_sched_rr		SCHED_RR
# define cma_c_sched_throughput	SCHED_OTHER
# define cma_c_sched_background	4
# define cma_c_sched_ada_low	5
# define cma_c_sched_ada_rtb	6

/*
 * NOTE:
 *
 * On DEC OSF/1, each priority has one distinct scheduling queue, independent
 * of the POSIX scheduling policy. This means that FIFO and RR threads can be
 * blocked behind OTHER threads until a preemption occurs. This doesn't
 * conform to the semantics specified by 1003.4a (or implemented in user mode
 * DECthreads versions).
 *
 * Note that the DEC OSF/1 sched.h advises use of priorities 0 to 19 for
 * "user" threads, 20 to 31 for "system" threads, and 32 to 63 for "realtime"
 * threads. However, certain kernel "server" threads associated with
 * processes (which handle signals and timer events) run within the system
 * priority range -- in particular, the signal server runs at priority 21, so
 * any thread above priority 21 may be immune to SIGINT or SIGQUIT, which
 * makes debugging dangerous. Also, root privilege is required to set a
 * TIMESHARE or RR policy thread above priority 19 (or a FIFO thread above
 * 18).
 *
 * On the other hand, general timeshared user threads associated with
 * non-threaded processes typically run at priority 19, so threads running at
 * lower priorities are generally uncompetitive. These values are defines so
 * that the default scheduling (throughput/mid) will be priority 19 to be
 * consistent with non-threaded processes, and so that values at the "mid" or
 * lower priorities in all policies can be set without privilege. Note that
 * DECthreads will allow any priority up to 63 for FIFO and RR threads,
 * despite the "max" values here.
 */
# define cma_c_prio_fifo_max	22
# define cma_c_prio_fifo_mid	18
# define cma_c_prio_fifo_min	14
# define cma_c_prio_rr_max	22
# define cma_c_prio_rr_mid	19
# define cma_c_prio_rr_min	16
# define cma_c_prio_through_max	22
# define cma_c_prio_through_mid	19
# define cma_c_prio_through_min	16
# define cma_c_prio_back_max	15
# define cma_c_prio_back_mid	8
# define cma_c_prio_back_min	0
# define cma_c_prio_ada_low_max	15
# define cma_c_prio_ada_low_mid	8
# define cma_c_prio_ada_low_min	0
# define cma_c_prio_ada_rtb_max	15
# define cma_c_prio_ada_rtb_mid	8
# define cma_c_prio_ada_rtb_min	0
#else
# define cma_c_sched_fifo	0
# define cma_c_sched_rr		1
# define cma_c_sched_throughput	2
# define cma_c_sched_background	3
# define cma_c_sched_ada_low	4
# define cma_c_sched_ada_rtb	5

# define cma_c_prio_fifo_max	31
# define cma_c_prio_fifo_mid	((31+16)/2)
# define cma_c_prio_fifo_min	16
# define cma_c_prio_rr_max	cma_c_prio_fifo_max
# define cma_c_prio_rr_mid	cma_c_prio_fifo_mid
# define cma_c_prio_rr_min	cma_c_prio_fifo_min
# define cma_c_prio_through_max	15
# define cma_c_prio_through_mid	((15+8)/2)
# define cma_c_prio_through_min	8
# define cma_c_prio_back_max	7
# define cma_c_prio_back_mid	((7+0)/2)
# define cma_c_prio_back_min	0
# define cma_c_prio_ada_low_max	cma_c_prio_back_max
# define cma_c_prio_ada_low_mid	cma_c_prio_back_mid
# define cma_c_prio_ada_low_min	cma_c_prio_back_min
# define cma_c_prio_ada_rtb_max	cma_c_prio_ada_low_max
# define cma_c_prio_ada_rtb_mid	cma_c_prio_ada_low_mid
# define cma_c_prio_ada_rtb_min	cma_c_prio_ada_low_min
#endif

# define cma_c_sched_default	cma_c_sched_throughput
# define cma_c_sched_other	cma_c_sched_default

/*
 * The implementation of the cma_t_date_time type should match the "native
 * time" of the platform: that allows clients to use the full set of platform
 * time operations, rather than just "cma_get_expiration", to compute and
 * test timed waits.
 *
 * This section assumes the platform is either "VMS" or "UNIX-ish".  Others
 * will require changes.
 */
#if _CMA_OS_ == _CMA__VMS
typedef struct CMA_T_DATE_TIME {
    unsigned long int	low;
    unsigned long int	high;
    }				cma_t_date_time;
#else
typedef struct timeval		cma_t_date_time;
#endif

/* 
 * Sample decisions for what handles shall be 
 */

typedef struct CMA_T_HANDLE {
    cma_t_address	field1;
    short int		field2;
    short int		field3;
    } cma_t_handle;

#define cma_thread_get_unique(handle) \
    ((cma_t_natural)((cma_t_thread *)handle)->field2)

#if (_CMA_COMPILER_ == _CMA__CFRONT) && (_CMA_OS_ == _CMA__VMS)
/*
 * The following declaration is a hack to support CFRONT (C++ preprocessor);
 * without an actual instance, the struct definition isn't written out.
 * Creates problems for VAX C when it compiles CFRONT output.
 */
static struct CMA_T_HANDLE	cxxl_unused_handle;
#endif

#define cma_c_handle_size sizeof(cma_t_handle)

typedef	cma_t_handle	cma_t_mutex;	/* Needed for CMA_ONCE_BLOCK */

/*
 * Predefined null handle
 */
#ifndef _CMA_SUPPRESS_EXTERNALS_
_CMA_IMPORT_ cma_t_handle	cma_c_null;
#endif

/*
 * Sample decision for a one-time initialization control block and its
 * initialization macro.
 *
 * Declare a one time initialization control block as:
 *
 *	static cma_t_once	block = cma_once_init;
 */
typedef struct CMA_T_ONCE {
    cma_t_integer	field1;
    cma_t_integer	field2;
    cma_t_integer	field3;
    }				cma_t_once;

#define cma_once_init	{0, 0, 0}

/* 
 * Sample decision for a variable to save the current alert state.
 */
typedef struct CMA_T_ALERT_STATE {
    cma_t_integer	state1;
    cma_t_integer	state2;
    }				cma_t_alert_state;

/*
 * The following are the portable CMA definitions
 */

/*
 * Operations on Handles
 */

/*
 * The value of a handle can be assigned the value of another handle.	
 */

extern void
cma_handle_assign _CMA_PROTOTYPE_ ((
	cma_t_handle	*handle1,	/* Input handle */
	cma_t_handle	*handle2));	/* Output handle */

/*
 * The value of a handle can be compared to the value of another handle.
 */

extern cma_t_boolean
cma_handle_equal _CMA_PROTOTYPE_ ((
	cma_t_handle	*handle1,
	cma_t_handle	*handle2));

/*
 * Operations on attributes objects
 */

typedef cma_t_handle	cma_t_attr;

/*
 * An attributes object is created to specify the attributes of other CMA
 * objects that will be created.
 */

extern void
cma_attr_create _CMA_PROTOTYPE_ ((
	cma_t_attr	*new_attr,
	cma_t_attr	*attr));

/*
 * An attributes object can be deleted when it is no longer needed.
 */

extern void
cma_attr_delete _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr));

/*
 * Operations on threads
 */

typedef cma_t_handle	cma_t_thread;
typedef cma_t_address	(*cma_t_start_routine) _CMA_PROTOTYPE_ ((
	cma_t_address	arg));

typedef cma_t_integer	cma_t_exit_status;

#define cma_c_term_error	0
#define cma_c_term_normal	1
#define cma_c_term_alert	2

typedef cma_t_integer		cma_t_sched_inherit;

#define cma_c_sched_inherit	0
#define cma_c_sched_use_default	1

/*
 * The following procedures can be used to control thread creation,
 * termination and deletion.
 */

/*
 * To create a thread object and runnable thread, a routine must be specified
 * as the new thread's start routine.  An argument may be passed to this
 * routine, as an untyped address; an untyped address may also be returned as
 * the routine's value.  An attributes object may be used to specify details
 * about the kind of thread being created.
 */
extern void
cma_thread_create _CMA_PROTOTYPE_ ((
	cma_t_thread		*new_thread,
	cma_t_attr		*attr,
	cma_t_start_routine	start_routine,
	cma_t_address		arg));

/*
 * A thread object may be "detached" to specify that the return value and
 * completion status will not be requested.
 */
extern void
cma_thread_detach _CMA_PROTOTYPE_ ((
	cma_t_thread	*thread));

/* 
 * A thread may terminate it's own execution.
 */
extern void
cma_thread_exit_error _CMA_PROTOTYPE_ ((void));

extern void
cma_thread_exit_normal _CMA_PROTOTYPE_ ((
	cma_t_address	result));

/* 
 * A thread can await termination of another thread and retrieve the return
 * value and completion status of the thread.
 */
extern void
cma_thread_join _CMA_PROTOTYPE_ ((
	cma_t_thread		*thread,
	cma_t_exit_status	*exit_status,
	cma_t_address		*result));

/*
 * Operations to define thread creation attributes
 */

/*
 * Set or obtain the default thread priority.
 */
extern void
cma_attr_set_priority _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr,
	cma_t_priority	pri));

extern void
cma_attr_get_priority _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr,
	cma_t_priority	*pri));

/*
 * Set or obtain the default scheduling algorithm
 */
extern void
cma_attr_set_sched _CMA_PROTOTYPE_ ((
	cma_t_attr		*attr,
	cma_t_sched_policy	policy,
	cma_t_priority		priority));

extern void
cma_attr_get_sched _CMA_PROTOTYPE_ ((
	cma_t_attr		*attr,
	cma_t_sched_policy	*policy));

/*
 * Set or obtain whether a thread will use the default scheduling attributes,
 * or inherit them from the creating thread.
 */
extern void
cma_attr_set_inherit_sched _CMA_PROTOTYPE_ ((
	cma_t_attr		*attr,
	cma_t_sched_inherit	setting));

extern void
cma_attr_get_inherit_sched _CMA_PROTOTYPE_ ((
	cma_t_attr		*attr,
	cma_t_sched_inherit	*setting));

/*
 * Set or obtain the default stack size
 */
extern void
cma_attr_set_stacksize _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr,
	cma_t_natural	stacksize));

extern void
cma_attr_get_stacksize _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr,
	cma_t_natural	*stacksize));

/*
 * Set or obtain the default guard size
 */
extern void
cma_attr_set_guardsize _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr,
	cma_t_natural	guardsize));

extern void
cma_attr_get_guardsize _CMA_PROTOTYPE_ ((
	cma_t_attr	*attr,
	cma_t_natural	*guardsize));

/*
 * Thread Scheduling Operations
 */

/*
 * The current user_assigned priority of a thread can be changed.
 */
extern void
cma_thread_set_priority _CMA_PROTOTYPE_ ((
	cma_t_thread	*thread,
	cma_t_priority	priority));

/*
 * The current user_assigned scheduler algorithm of a thread can be changed.
 */
extern void
cma_thread_set_sched _CMA_PROTOTYPE_ ((
	cma_t_thread		*thread,
	cma_t_sched_policy	policy,
	cma_t_priority		priority));

/*
 * A thread may tell the scheduler that its processor can be made available.
 */
extern void
cma_yield _CMA_PROTOTYPE_ ((void));

/*
 * A thread may enter a wait state for a speciifed period of time.
 */
extern void
cma_delay _CMA_PROTOTYPE_ ((
	cma_t_interval	interval));

/*
 * Thread Information Operations
 */

/*
 * A thread may obtain a copy of its own thread handle.
 */
extern void
cma_thread_get_self _CMA_PROTOTYPE_ ((
	cma_t_thread	*thread));

/*
 * The current user_assigned priority of a thread can be read.
 */
extern void
cma_thread_get_priority _CMA_PROTOTYPE_ ((
	cma_t_thread	*thread,
	cma_t_priority	*priority));

/*
 * The current user_assigned scheduler algorithm of a thread can be read.
 */
extern void
cma_thread_get_sched _CMA_PROTOTYPE_ ((
	cma_t_thread		*thread,
	cma_t_sched_policy	*policy));

/*
 * Operations on Mutexes
 */

typedef enum CMA_T_MUTEX_KIND {
    cma_c_mutex_fast = 0,
    cma_c_mutex_recursive = 1,
    cma_c_mutex_nonrecursive = 2
    }				cma_t_mutex_kind;

/*
 * Operations to define mutex creation attributes
 */

/*
 * Set or obtain whether mutex locks can nest.
 */
extern void
cma_attr_set_mutex_kind _CMA_PROTOTYPE_ ((
	cma_t_attr		*attr,
	cma_t_mutex_kind	nest));

extern void
cma_attr_get_mutex_kind _CMA_PROTOTYPE_ ((
	cma_t_attr		*attr,
	cma_t_mutex_kind	*nest));

/* 
 * The following routines create, delete, lock and unlock mutexes.
 */
extern void
cma_mutex_create _CMA_PROTOTYPE_ ((
	cma_t_mutex	*new_mutex,
	cma_t_attr	*attr));

extern void
cma_mutex_delete _CMA_PROTOTYPE_ ((
	cma_t_mutex	*mutex));

extern void
cma_mutex_lock _CMA_PROTOTYPE_ ((
	cma_t_mutex	*mutex));

extern cma_t_boolean
cma_mutex_try_lock _CMA_PROTOTYPE_ ((
	cma_t_mutex	*mutex));

extern void
cma_mutex_unlock _CMA_PROTOTYPE_ ((
	cma_t_mutex	*mutex));

extern void
cma_lock_global _CMA_PROTOTYPE_ ((void));

extern void
cma_unlock_global _CMA_PROTOTYPE_ ((void));

/*
 * Operations on condition variables
 */

typedef cma_t_handle	cma_t_cond;

/*
 * A thread can create and delete condition variables.
 */
extern void
cma_cond_create _CMA_PROTOTYPE_ ((
	cma_t_cond	*new_condition,
	cma_t_attr	*attr));

extern void
cma_cond_delete _CMA_PROTOTYPE_ ((
	cma_t_cond	*condition));

/*
 * A thread can signal to and broadcast on a condition variable.
 */
extern void
cma_cond_broadcast _CMA_PROTOTYPE_ ((
	cma_t_cond	*condition));

extern void
cma_cond_signal _CMA_PROTOTYPE_ ((
	cma_t_cond	*condition));

extern void
cma_cond_signal_int _CMA_PROTOTYPE_ ((
	cma_t_cond	*condition));

extern void
cma_cond_signal_preempt_int 
#if _CMA_OS_ == _CMA__UNIX
	_CMA_PROTOTYPE_ ((
		cma_t_cond	*condition,
		cma_t_address	scp
		));
#else
	_CMA_PROTOTYPE_ ((
		cma_t_cond	*condition
		));
#endif

/*
 * A thread can wait for a condition variable to be signalled or broadcast.
 */
extern void
cma_cond_wait _CMA_PROTOTYPE_ ((
	cma_t_cond	*condition,
	cma_t_mutex	*mutex));

/*
 * Operations for timed waiting
 */

/*
 * A thread can perform a timed wait on a condition variable.
 */
extern cma_t_status
cma_cond_timed_wait _CMA_PROTOTYPE_ ((
	cma_t_cond	*condition,
	cma_t_mutex	*mutex,
	cma_t_date_time	*expiration));

/*
 * A thread may perform some operations on absolute date-time and intervals.
 */

extern void
cma_time_get_expiration _CMA_PROTOTYPE_ ((
	cma_t_date_time	*expiration,
	cma_t_interval	interval));

/*
 * Operations for CMA and client initialization.
 */

/*
 * Initialize the CMA facility.
 */
extern void
cma_init _CMA_PROTOTYPE_ ((void));

/*
 * A thread can declare a one-time initialization routine.  The address of
 * the init block and routine are passed as parameters.
 */

typedef void		(*cma_t_init_routine) _CMA_PROTOTYPE_ ((
	cma_t_address	arg));

extern void
cma_once _CMA_PROTOTYPE_ ((
	cma_t_once		*init_block,
	cma_t_init_routine	init_routine,
	cma_t_address		arg));

/*
 * Operations for per-thread context
 */

typedef void		(*cma_t_destructor) _CMA_PROTOTYPE_ ((
	cma_t_address	ctx_value));

/*
 * A unique per-thread context key can be obtained for the process
 */
extern void
cma_key_create _CMA_PROTOTYPE_ ((
	cma_t_key		*key,
	cma_t_attr		*attr,
	cma_t_destructor	destructor));

/*
 * A thread can set a per-thread context value identified by a key.
 */
extern void
cma_key_set_context _CMA_PROTOTYPE_ ((
	cma_t_key	key,
	cma_t_address	context_value));

/*
 * A thread can retrieve a per-thread context value identified by a key.
 */
extern void
cma_key_get_context _CMA_PROTOTYPE_ ((
	cma_t_key	key,
	cma_t_address	*context_value));

/*
 * Operations for alerts.
 */

/*
 * The current thread can request that a thread terminate it's execution.
 */

extern void
cma_thread_alert _CMA_PROTOTYPE_ ((
	cma_t_thread	*thread));

/*
 * The current thread can poll for alert delivery.
 */
extern void
cma_alert_test _CMA_PROTOTYPE_ ((void));

/*
 * The current thread can disable asynchronous alert delivery, restore the
 * previous state of asynchronous alert delivery, or enable asynchronous
 * alert delivery.
 */
extern void
cma_alert_disable_asynch _CMA_PROTOTYPE_ ((
	cma_t_alert_state	*prior));

extern void
cma_alert_disable_general _CMA_PROTOTYPE_ ((
	cma_t_alert_state	*prior));

extern void
cma_alert_enable_asynch _CMA_PROTOTYPE_ ((void));

extern void
cma_alert_enable_general _CMA_PROTOTYPE_ ((
	cma_t_alert_state	*prior));

extern void
cma_alert_restore _CMA_PROTOTYPE_ ((
	cma_t_alert_state	*prior));

/*
 * Operations on stacks
 */

typedef	cma_t_handle	cma_t_stack_np;
typedef void		(*cma_t_call_routine) ();

/*
 * Allocate stack space
 */
extern void
cma_stack_allocate_np _CMA_PROTOTYPE_ ((
	cma_t_integer	size,
	cma_t_address	*new_size));

/*
 * Assign a stack to a thread
 */
extern void
cma_stack_assign_np _CMA_PROTOTYPE_ ((
	cma_t_stack_np	*stack,
	cma_t_thread	*thread));

/*
 * Call a routine on a new stack
 */
extern void
cma_stack_call_routine_np _CMA_PROTOTYPE_ ((
	cma_t_stack_np		*stack,
	cma_t_call_routine	routine,
	cma_t_address		arg,
	cma_t_address		*result));

/*
 * Check stack limit
 */
extern cma_t_boolean
cma_stack_check_limit_np _CMA_PROTOTYPE_ ((
	cma_t_integer	size));

/*
 * Create a new stack
 */
extern void
cma_stack_create_np _CMA_PROTOTYPE_ ((
	cma_t_stack_np	*stack,
	cma_t_attr	*attr));

/*
 * Delete a stack
 */
extern void
cma_stack_delete_np _CMA_PROTOTYPE_ ((
	cma_t_stack_np	*stack));

/*
 * Debug threads
 */
extern void
cma_debug _CMA_PROTOTYPE_ ((void));

extern void
cma_debug_cmd _CMA_PROTOTYPE_ ((char *cmd));

#ifndef _CMA_SUPPRESS_EXTERNALS_
_CMA_IMPORT_ void (*cma_g_debug) _CMA_PROTOTYPE_ ((void));
#endif

# if _CMA_OS_ == _CMA__VMS
#  include <cma_px.h>
# else
#  include <dce/cma_px.h>
# endif

# ifndef _CMA_NOWRAPPERS_
#  if !_CMA_REENTRANT_CLIB_
#   if _CMA_OS_ == _CMA__VMS
#    include <cma_crtlx.h>
#   else
#    include <dce/cma_errno.h>
#    include <dce/cma_crtlx.h>
#    include <dce/cma_stdio.h>
#   endif
#  endif
#  if _CMA_OS_ == _CMA__UNIX
#   include <dce/cma_ux.h>
#  endif
#  if _CMA_OS_ == _CMA__VMS
#   include <cma_sigwait.h>
#  else
#   include <dce/cma_sigwait.h>
#  endif
# endif

# ifdef _HP_LIBC_R
#  include <cma_libc_calls.h>
# endif

#if ((_CMA_COMPILER_ == _CMA__DECC) || (_CMA_COMPILER_ == _CMA__DECCPLUS)) && _CMA_OS_ == _CMA__VMS
# pragma __extern_model __restore	/* saved in cma_config.h */
#elif _CMA_COMPILER_ == _CMA__VAXC
# pragma standard			/* set in cma_config.h */
#endif

#ifdef __cplusplus
    }
#endif

#endif
