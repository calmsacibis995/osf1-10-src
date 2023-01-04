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
 *	@(#)$RCSfile: pthread_exc.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:36:32 $
 */
/*
 *  FACILITY:
 *
 *	Common Multithread Architecture (CMA) services; POSIX 1003.4 interface
 *	
 *	Exception Generating Pthreads Interface (ptdexc)
 *
 *  ABSTRACT:
 *
 *	External definitions for CMA's pthreads exception (ptdexc) services
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 * 
 *	27 August 1990
 *
 *  MODIFIED BY:
 *
 *	Dave Butenhof
 *	Paul Curtin
 *	Webb Scales
 */

#ifndef PTHREAD_EXC
#define PTHREAD_EXC

#ifdef __cplusplus
    extern "C" {
#endif

#if defined(vms) || defined(__vms) || defined(VMS) || defined(__VMS)
# include <cma.h>
#else
# include <dce/cma.h>
# include <signal.h>
#endif

/*
 * The implementation makes these basic decisions
 */

#ifndef _POSIX_THREADS
# define _POSIX_THREADS				1
#endif
#ifndef _POSIX_THREAD_ATTR_STACKSIZE
# define _POSIX_THREAD_ATTR_STACKSIZE		1
#endif
#if _CMA_POSIX_SCHED_
# define _POSIX_THREADS_REALTIME_SCHEDULING	1
#elif defined(_POSIX_THREADS_REALTIME_SCHEDULING)
# undef _POSIX_THREADS_REALTIME_SCHEDULING
#endif
#ifndef _POSIX_THREADS_PER_PROCESS_SIGNALS_1
# define _POSIX_THREADS_PER_PROCESS_SIGNALS_1	1
#endif

/*
 * Implement push and pop for cancellation handlers, using TRY and ENDTRY
 */

#define pthread_cleanup_push(routine,arg)	\
    { \
    pthread_cleanup_t _XXX_proc = (pthread_cleanup_t)(routine); \
    pthread_addr_t _XXX_arg = (pthread_addr_t)(arg); \
    int _XXX_completed = 0; \
    TRY {

#define pthread_cleanup_pop(execute)	\
    _XXX_completed = 1;} \
    FINALLY { \
	if ((! _XXX_completed) || (execute)) _XXX_proc (_XXX_arg);} \
    ENDTRY}


/*
 *  Macros used to convert normal pthread routine calls to exception 
 *  returning routines.  This is done by including this file, pthread_exc.h,
 *  in the place of pthread.h .
 */


#define pthread_equal_np(thread1,thread2) \
    (((thread1).field1 == (thread2).field1) \
    && ((thread1).field2 == (thread2).field2) \
    && ((thread1).field3 == (thread2).field3))

#define pthread_equal(thread1,thread2) \
    (((thread1).field1 == (thread2).field1) \
    && ((thread1).field2 == (thread2).field2) \
    && ((thread1).field3 == (thread2).field3))

#define pthread_getunique_np(handle) \
    ((unsigned int)((pthread_t *)handle)->field2)

#define pthread_attr_create(attr) \
    ptdexc_attr_create(attr)

#define pthread_attr_delete(attr) \
    ptdexc_attr_delete(attr)

#define pthread_attr_setprio(attr,priority) \
    ptdexc_attr_setprio(attr,priority)

#define pthread_attr_getprio(attr) \
    ptdexc_attr_getprio(attr)

#define pthread_attr_setsched(attr,scheduler) \
    ptdexc_attr_setsched(attr,scheduler)

#define pthread_attr_getsched(attr) \
    ptdexc_attr_getsched(attr)

#define pthread_attr_setinheritsched(attr,inherit) \
    ptdexc_attr_setinheritsched(attr,inherit)

#define pthread_attr_getinheritsched(attr) \
    ptdexc_attr_getinheritsched(attr)

#define pthread_attr_setstacksize(attr,stacksize) \
    ptdexc_attr_setstacksize(attr,stacksize)

#define pthread_attr_getstacksize(attr) \
    ptdexc_attr_getstacksize(attr)

#define pthread_attr_setguardsize_np(attr,guardsize) \
    ptdexc_attr_setguardsize_np(attr,guardsize)

#define pthread_attr_getguardsize_np(attr) \
    ptdexc_attr_getguardsize_np(attr)

#define pthread_create(thread,attr,start_routine,arg) \
    ptdexc_create(thread,attr,start_routine,arg)

#define pthread_detach(thread) \
    ptdexc_detach(thread)

#define pthread_exit(status) \
    ptdexc_exit(status)

#define pthread_get_expiration_np(delta,abstime) \
    ptdexc_get_expiration_np(delta,abstime)

#define pthread_join(thread,status) \
    ptdexc_join(thread,status)

#define pthread_setprio(thread,priority) \
    ptdexc_setprio(thread,priority)

#define pthread_setscheduler(thread,scheduler,priority) \
    ptdexc_setscheduler(thread,scheduler,priority)

#define pthread_yield() \
    ptdexc_yield()

#define pthread_self() \
    ptdexc_self()

#define pthread_getprio(thread) \
    ptdexc_getprio(thread)

#define pthread_getscheduler(thread) \
    ptdexc_getscheduler(thread)

#define pthread_mutexattr_create(attr) \
    ptdexc_mutexattr_create(attr)

#define pthread_mutexattr_delete(attr) \
    ptdexc_mutexattr_delete(attr)

#define pthread_mutexattr_setkind_np(attr,kind) \
    ptdexc_mutexattr_setkind_np(attr,kind)

#define pthread_mutexattr_getkind_np(attr) \
    ptdexc_mutexattr_getkind_np(attr)

#define pthread_mutex_init(mutex,attr) \
    ptdexc_mutex_init(mutex,attr)

#define pthread_mutex_destroy(mutex) \
    ptdexc_mutex_destroy(mutex)

#define pthread_mutex_lock(mutex) \
    ptdexc_mutex_lock(mutex)

#define pthread_mutex_trylock(mutex) \
    ptdexc_mutex_trylock(mutex)

#define pthread_mutex_unlock(mutex) \
    ptdexc_mutex_unlock(mutex)

#define pthread_condattr_create(attr) \
    ptdexc_condattr_create(attr)

#define pthread_condattr_delete(attr) \
    ptdexc_condattr_delete(attr)

#define pthread_cond_init(cond,attr) \
    ptdexc_cond_init(cond,attr)

#define pthread_cond_destroy(cond) \
    ptdexc_cond_destroy(cond)

#define pthread_cond_broadcast(cond) \
    ptdexc_cond_broadcast(cond)

#define pthread_cond_signal(cond) \
    ptdexc_cond_signal(cond)

#define pthread_cond_signal_int_np(cond) \
    ptdexc_cond_signal_int_np(cond)

#if _CMA_OS_ == _CMA__UNIX
# define pthread_cond_sig_preempt_int_np(cond,arg) \
    ptdexc_cond_sigprmpt_int_np(cond,arg)
#else
# define pthread_cond_sig_preempt_int_np(cond) \
    ptdexc_cond_sigprmpt_int_np(cond)
#endif

#define pthread_cond_wait(cond,mutex) \
    ptdexc_cond_wait(cond,mutex)

#define pthread_cond_timedwait(cond,mutex,abstime) \
    ptdexc_cond_timedwait(cond,mutex,abstime)

#define pthread_once(once_block,init_routine) \
    ptdexc_once(once_block,init_routine)

#define pthread_keycreate(key,destructor) \
    ptdexc_keycreate(key,destructor)

#define pthread_setspecific(key,value) \
    ptdexc_setspecific(key,value)

#define pthread_getspecific(key,value) \
    ptdexc_getspecific(key,value)

#define pthread_mutexattr_setkind(attr,kind) \
    ptdexc_mutexattr_setkind_np(attr,kind)

#define pthread_mutexattr_getkind(attr) \
    ptdexc_mutexattr_getkind_np(attr)

#define pthread_cancel(thread) \
    ptdexc_cancel(thread)

#define pthread_testcancel() \
    ptdexc_testcancel()

#define pthread_setasynccancel(state) \
    ptdexc_setasynccancel(state)

#define pthread_setcancel(state) \
    ptdexc_setcancel(state)

#define pthread_delay_np(interval) \
    ptdexc_delay_np(interval)

#define pthread_lock_global_np() \
    ptdexc_lock_global_np()

#define pthread_unlock_global_np() \
    ptdexc_unlock_global_np()

#if _CMA_OS_ != _CMA__VMS
# define pthread_sig_to_can_thread_np(sigset,target,thread) \
    ptdexc_sig_to_can_thread_np(sigset,target,thread)

# define pthread_signal_to_cancel_np(sigset,target) \
    ptdexc_signal_to_cancel_np(sigset,target)
#endif

/*
 * Sample decisions for the environment types
 */

typedef	cma_t_key		pthread_key_t;

typedef cma_t_address		pthread_addr_t;

/*
 * For compatibility with OSF/1 pthreads
 */
typedef pthread_addr_t		any_t;

typedef void (*pthread_cleanup_t) _CMA_PROTOTYPE_ ((pthread_addr_t arg));

/*
 * Sample decision for a one-time initialization control block and its
 * initialization macro.
 *
 * Declare a one time initialization control block as:
 *
 *	static pthread_once_t	block = pthread_once_init;
 */
typedef cma_t_once	pthread_once_t;

#define pthread_once_init	cma_once_init

#define CANCEL_ON	1
#define CANCEL_OFF	0

/*
 * The following are the portable pthread definitions
 */

/*
 * Operations on Handles
 */

/*
 * Operations on attributes objects
 */

typedef cma_t_attr	pthread_attr_t;

/*
 * An attributes object is created to specify the attributes of other CMA
 * objects that will be created.
 */

void
ptdexc_attr_create _CMA_PROTOTYPE_ ((
	pthread_attr_t		*attr));

/*
 * An attributes object can be deleted when it is no longer needed.
 */

void
ptdexc_attr_delete _CMA_PROTOTYPE_ ((
	pthread_attr_t		*attr));

/*
 * Operations on threads
 */

typedef cma_t_thread		pthread_t;
typedef cma_t_start_routine	pthread_startroutine_t;
/*
 * For compatibility with OSF/1 pthreads
 */
typedef pthread_startroutine_t	pthread_func_t;

#define PTHREAD_INHERIT_SCHED	cma_c_sched_inherit
#define PTHREAD_DEFAULT_SCHED	cma_c_sched_use_default

#if !_CMA_RT4_KTHREAD_
# define SCHED_FIFO		cma_c_sched_fifo
# define SCHED_RR		cma_c_sched_rr
# define SCHED_OTHER		cma_c_sched_throughput
#endif
#define SCHED_FG_NP		cma_c_sched_throughput
#define SCHED_BG_NP		cma_c_sched_background

#define PRI_FIFO_MIN		cma_c_prio_fifo_min
#define PRI_FIFO_MAX		cma_c_prio_fifo_max
#define PRI_RR_MIN		cma_c_prio_rr_min
#define PRI_RR_MAX		cma_c_prio_rr_max
#define PRI_FG_MIN_NP		cma_c_prio_through_min
#define PRI_FG_MAX_NP		cma_c_prio_through_max
#define PRI_BG_MIN_NP		cma_c_prio_back_min
#define PRI_BG_MAX_NP		cma_c_prio_back_max
#define PRI_OTHER_MIN		cma_c_prio_through_min
#define PRI_OTHER_MAX		cma_c_prio_through_max


/*
 * Operations to define thread creation attributes
 */

/*
 * Set or obtain the default thread priority.
 */
void
ptdexc_attr_setprio _CMA_PROTOTYPE_ ((
	pthread_attr_t	*attr,
	int		priority));

int
ptdexc_attr_getprio _CMA_PROTOTYPE_ ((
	pthread_attr_t	attr));

/*
 * Set or obtain the default scheduling algorithm
 */
void
ptdexc_attr_setsched _CMA_PROTOTYPE_ ((
	pthread_attr_t	*attr,
	int		scheduler));

int
ptdexc_attr_getsched _CMA_PROTOTYPE_ ((
	pthread_attr_t	attr));

/*
 * Set or obtain whether a thread will use the default scheduling attributes,
 * or inherit them from the creating thread.
 */
void
ptdexc_attr_setinheritsched _CMA_PROTOTYPE_ ((
	pthread_attr_t	*attr,
	int		inherit));

int
ptdexc_attr_getinheritsched _CMA_PROTOTYPE_ ((
	pthread_attr_t		attr));

/*
 * Set or obtain the default stack size
 */
void
ptdexc_attr_setstacksize _CMA_PROTOTYPE_ ((
	pthread_attr_t	*attr,
	long		stacksize));

long
ptdexc_attr_getstacksize _CMA_PROTOTYPE_ ((
	pthread_attr_t	attr));

/*
 * Set or obtain the default guard size
 */
void
ptdexc_attr_setguardsize_np _CMA_PROTOTYPE_ ((
	pthread_attr_t	*attr,
	long		guardsize));

long
ptdexc_attr_getguardsize_np _CMA_PROTOTYPE_ ((
	pthread_attr_t	attr));

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
void
ptdexc_create _CMA_PROTOTYPE_ ((
	pthread_t		*thread,
	pthread_attr_t		attr,
	pthread_startroutine_t	start_routine,
	pthread_addr_t		arg));

/*
 * A thread object may be "detached" to specify that the return value and
 * completion status will not be requested.
 */
void
ptdexc_detach _CMA_PROTOTYPE_ ((
	pthread_t		*thread));

/* 
 * A thread may terminate it's own execution.
 */
void
ptdexc_exit _CMA_PROTOTYPE_ ((
	pthread_addr_t 		status));

/* 
 * A thread can await termination of another thread and retrieve the return
 * value of the thread.
 */
void
ptdexc_join _CMA_PROTOTYPE_ ((
	pthread_t		thread,
	pthread_addr_t		*status));

/*
 * Thread Scheduling Operations
 */

/*
 * The current user_assigned priority of a thread can be changed.
 */
int
ptdexc_setprio _CMA_PROTOTYPE_ ((
	pthread_t	thread,
	int		priority));

/*
 * The current user_assigned scheduler algorithm of a thread can be changed.
 */
int
ptdexc_setscheduler _CMA_PROTOTYPE_ ((
	pthread_t	thread,
	int		scheduler,
	int		priority));

/*
 * A thread may tell the scheduler that its processor can be made available.
 */
void
ptdexc_yield _CMA_PROTOTYPE_ ((void));

/*
 * Thread Information Operations
 */

/*
 * A thread may obtain a copy of its own thread handle.
 */
pthread_t
ptdexc_self _CMA_PROTOTYPE_ ((void));

/*
 * The current user_assigned priority of a thread can be read.
 */
int
ptdexc_getprio _CMA_PROTOTYPE_ ((
	pthread_t	thread));

/*
 * The current user_assigned scheduler algorithm of a thread can be read.
 */
int
ptdexc_getscheduler _CMA_PROTOTYPE_ ((
	pthread_t	thread));

/*
 * Operations on Mutexes
 */

#define MUTEX_FAST_NP		0
#define MUTEX_RECURSIVE_NP	1
#define MUTEX_NONRECURSIVE_NP	2

typedef cma_t_attr	pthread_mutexattr_t;
typedef	cma_t_mutex	pthread_mutex_t;

void
ptdexc_mutexattr_create _CMA_PROTOTYPE_ ((
	pthread_mutexattr_t	*attr));

void
ptdexc_mutexattr_delete _CMA_PROTOTYPE_ ((
	pthread_mutexattr_t	*attr));

void
ptdexc_mutexattr_setkind_np _CMA_PROTOTYPE_ ((
	pthread_mutexattr_t	*attr,
	int			kind));

int
ptdexc_mutexattr_getkind_np _CMA_PROTOTYPE_ ((
	pthread_mutexattr_t	attr));

/* 
 * The following routines create, delete, lock and unlock mutexes.
 */
void
ptdexc_mutex_init _CMA_PROTOTYPE_ ((
	pthread_mutex_t		*mutex,
	pthread_mutexattr_t	attr));

void
ptdexc_mutex_destroy _CMA_PROTOTYPE_ ((
	pthread_mutex_t		*mutex));

void
ptdexc_mutex_lock _CMA_PROTOTYPE_ ((
	pthread_mutex_t		*mutex));

int
ptdexc_mutex_trylock _CMA_PROTOTYPE_ ((
	pthread_mutex_t		*mutex));

void
ptdexc_mutex_unlock _CMA_PROTOTYPE_ ((
	pthread_mutex_t		*mutex));

/*
 * Operations on condition variables
 */

typedef cma_t_attr	pthread_condattr_t;
typedef cma_t_cond	pthread_cond_t;

void
ptdexc_condattr_create _CMA_PROTOTYPE_ ((
	pthread_condattr_t	*attr));

void
ptdexc_condattr_delete _CMA_PROTOTYPE_ ((
	pthread_condattr_t	*attr));

/*
 * A thread can create and delete condition variables.
 */
void
ptdexc_cond_init _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond,
	pthread_condattr_t	attr));

void
ptdexc_cond_destroy _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond));

/*
 * A thread can signal to and broadcast on a condition variable.
 */
void
ptdexc_cond_broadcast _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond));

void
ptdexc_cond_signal _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond));

void
ptdexc_cond_signal_int_np _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond));

void
ptdexc_cond_sigprmpt_int_np 
#if _CMA_OS_ == _CMA__UNIX
	_CMA_PROTOTYPE_ ((
		pthread_cond_t	*condition,
		pthread_addr_t	scp
		));
#else
	_CMA_PROTOTYPE_ ((
		pthread_cond_t	*condition
		));
#endif

/*
 * A thread can wait for a condition variable to be signalled or broadcast.
 */
void
ptdexc_cond_wait _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond,
	pthread_mutex_t		*mutex));

/*
 * Operations for timed waiting
 */

/*
 * A thread can perform a timed wait on a condition variable.
 */
int
ptdexc_cond_timedwait _CMA_PROTOTYPE_ ((
	pthread_cond_t		*cond,
	pthread_mutex_t		*mutex,
	struct timespec		*abstime));

/*
 * Operations for client initialization.
 */

typedef void (*pthread_initroutine_t) _CMA_PROTOTYPE_ ((void));

void
ptdexc_once _CMA_PROTOTYPE_ ((
	pthread_once_t		*once_block,
	pthread_initroutine_t	init_routine));

/*
 * Operations for per-thread context
 */

typedef cma_t_destructor	pthread_destructor_t;

/*
 * A unique per-thread context key can be obtained for the process
 */
void
ptdexc_keycreate _CMA_PROTOTYPE_ ((
	pthread_key_t		*key,
	pthread_destructor_t	destructor));

/*
 * A thread can set a per-thread context value identified by a key.
 */
void
ptdexc_setspecific _CMA_PROTOTYPE_ ((
	pthread_key_t	key,
	pthread_addr_t	value));

/*
 * A thread can retrieve a per-thread context value identified by a key.
 */
void
ptdexc_getspecific _CMA_PROTOTYPE_ ((
	pthread_key_t	key,
	pthread_addr_t	*value));

/*
 * Operations for alerts.
 */

/*
 * The current thread can request that a thread terminate it's execution.
 */

void
ptdexc_cancel _CMA_PROTOTYPE_ ((
	pthread_t	thread));

/*
 * The current thread can poll for alert delivery.
 */
void
ptdexc_testcancel _CMA_PROTOTYPE_ ((void));

/*
 * The current thread can enable or disable alert delivery (PTHREAD
 * "cancels"); it can control "general cancelability" (CMA "defer") or
 * just "asynchronous cancelability" (CMA "asynch disable").
 */
int
ptdexc_setasynccancel _CMA_PROTOTYPE_ ((
	int	state));

int
ptdexc_setcancel _CMA_PROTOTYPE_ ((
	int	state));

_CMA_IMPORT_ pthread_attr_t		pthread_attr_default;
_CMA_IMPORT_ pthread_mutexattr_t	pthread_mutexattr_default;
_CMA_IMPORT_ pthread_condattr_t		pthread_condattr_default;

/*
 * Define nonportable extensions
 */

extern int
ptdexc_get_expiration_np _CMA_PROTOTYPE_ ((
	struct timespec	*delta,
	struct timespec	*abstime));

extern void
ptdexc_delay_np _CMA_PROTOTYPE_ ((
	struct timespec	*interval));

extern void
ptdexc_lock_global_np _CMA_PROTOTYPE_ ((void));

extern void
ptdexc_unlock_global_np _CMA_PROTOTYPE_ ((void));

# if _CMA_OS_ != _CMA__VMS
extern void
ptdexc_sig_to_can_thread_np _CMA_PROTOTYPE_ ((
	sigset_t	*sigset,
	pthread_t	*target,
	pthread_t	*thread));

extern void
ptdexc_signal_to_cancel_np _CMA_PROTOTYPE_ ((
	sigset_t	*sigset,
	pthread_t	*target));
# endif

#ifdef __cplusplus
    }
#endif

#endif
