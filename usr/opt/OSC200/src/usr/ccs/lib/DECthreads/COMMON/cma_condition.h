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
 * @(#)$RCSfile: cma_condition.h,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:44:04 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Header file for condition variable operations
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	6 September 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	14 September 1989
 *		Add semaphore to structure for low-level waiting.
 *	002	Dave Butenhof	04 October 1989
 *		Implement internal interfaces to wait, signal, broadcast.
 *	003	Dave Butenhof	1 November 1989
 *		Put all conditions on a queue.
 *	004	Dave Butenhof	16 November 1989
 *		Change time parameter types from "cma__t_ticks" (obsolete) to
 *		cma_t_date_time.
 *	005	Dave Butenhof	1 December 1989
 *		Abbreviate cma_t_condition to cma_t_cond for POSIX
 *		compatibility.
 *	006	Dave Butenhof & Webb Scales	4 December 1989
 *		Include cma_semaphore_defs.h instead of cma_semaphore.h
 *	007	Dave Butenhof	9 April 1990
 *		Remove definition of known condition queue header; it's now
 *		in cma_debugger.
 *	008	Dave Butenhof	23 May 1990
 *		Make delay mutex & cv extern for pthread delay
 *	009	Dave Butenhof	08 August 1990
 *		Post pending wake on semaphore if no waiters, with single
 *		interlocked instruction.
 *	010	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	011	Dave Butenhof	26 November 1990
 *		Fix a bug in signal; it set cv->event even though a signal
 *		may leave other waiters. Signal must declare that there may
 *		still be waiters.
 *	012	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	013	Dave Butenhof	4 February 1991
 *		Change prototypes for cma__int_wait and cma__int_timed_wait.
 *	014	Dave Butenhof	02 April 1991
 *		Remove pending wakeup when there are definitely no waiters.
 *	015	Dave Butenhof	09 April 1991
 *		Use new type for "atomic bit" operation target
 *	016	Paul Curtin	10 May 1991
 *		Added a number of new macros
 *	017	Dave Butenhof	17 May 1991
 *		Fix a bug isolated and diagnosed by Harold Seigel (thanks,
 *		Harold!)  T'was exposed by the removal of pending wake on
 *		condition variables... if a low priority thread waits on a
 *		condition while a high priority thread is waiting on the
 *		associated mutex, releasing the mutex will start the high
 *		priority thread.  If it then signals, there's no trace that
 *		the low priority thread may be waiting, since it hadn't
 *		established its intent to wait, and the signal is ignored.
 *		Now it will clear "event" before releasing the mutex; the
 *		signal will now issue a wake on the semaphore... since the
 *		semaphore queue is empty, cma__sem_wake_one will record a
 *		pending wakeup and the waiter's sem_wait operation will
 *		return immediately.
 *	018	Paul Curtin	24 May 1991
 *		Added a prototype for cma__reinit_cv.
 *	019	Dave Butenhof	22 November 1991
 *		Remove typedef into a separate cma_cond_defs.h file, to
 *		break a circularity from cma_defer.h.
 *	020	Dave Butenhof	10 February 1992
 *		Remove int_cond_create macro; function now differs between
 *		interfaces.
 *	021	Dave Butenhof	18 August 1992
 *		Remove redundant attempt_delivery call on waits.
 *	022	Dave Butenhof	26 August 1992
 *		Remove semaphores from code, and generally streamline and
 *		clean things up. Remove global defer queue -- instead, use
 *		per-CV atomic flags.
 *	023	Dave Butenhof	03 September 1992
 *		Restore the $WAKE in signal_int that got lost during 044.
 *	024	Webb Scales	 4 September 1992
 *		Add cond-signal-preempt-int.
 *	025	Dave Butenhof	11 September 1992
 *		Remove externs for obsolete delay_mutex & delay_cv globals.
 *	026	Dave Butenhof	15 September 1992
 *		Fix int_signal_preempt_int prototype
 *	027	Dave Butenhof	17 September 1992
 *		Make low-level cv wakeup accessible to other modules.
 *	028	Dave Butenhof	21 September 1992
 *		Set 'event' bit on any cvunlock operation.
 *	029	Dave Butenhof	30 September 1992
 *		Fix typo in k-thread macros.
 *	030	Dave Butenhof	16 October 1992
 *		As used in code, cma__mutrylock() returns previous state of
 *		bit; but cma__tryspinlock() returns "true" if lock is
 *		acquired. So reverse the sense of VP macro.
 *	031	Dave Butenhof	30 October 1992
 *		Enhance performance (at the possible expense of occasional
 *		latency) by changing cvlock/cvunlock to use set/unset kernel
 *		rather than enter/exit, saving cost of checking for global
 *		undeferral.
 *	032	Dave Butenhof	30 October 1992
 *		Remove 'pend' arg from int_cond_wake: ALWAYS post a pending
 *		wake if the 'event' is set (solves possible wakeup waiting
 *		race with prioritized threads).
 *	033	Dave Butenhof	23 November 1992
 *		Close race in CV wait/signal -- don't unlock CV around
 *		cma__ready() on signal.
 *	034	Dave Butenhof	 3 December 1992
 *		Fix typo reference to cma__set (which doesn't exist)
 *	035	Webb Scales	 8 February 1993
 *		Add a call to signal_int which prevents a wake-up waiting
 *		race on U*ix systems.
 *	036	Dave Butenhof	29 March 1993
 *		Integrate March 10 review comments.
 *	037	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	038	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	039	Dave Butenhof	21 May 1993
 *		cma__cv_wakeup() has been redesigned to release the CV lock
 *		on DEC OSF/1 before actually doing wakes. On other
 *		platforms, the lock can't be released until after, so there's
 *		a new cma__cvunlockwake() macro that's null on DEC OSF/1.
 *	040	Dave Butenhof	28 May 1993
 *		Fix bug in 039 -- I missed pending wake case!
 *	041	Dave Butenhof	15 June 1993
 *		Kernel thread signal_int should post pending wake, just to be
 *		safe.
 *	042	Dave Butenhof	21 June 1993
 *		cvwakeup needs to init tcb->header.queue in awakened
 * 		threads,to solve OSF/1 race between signal_int and wait.
 */


#ifndef CMA_CONDITION
#define CMA_CONDITION

/*
 *  INCLUDE FILES
 */

#include <cma_defer.h>
#include <cma_cond_defs.h>
#include <cma_attr.h>
#include <cma_timer.h>
#include <cma_mutex.h>
#include <cma_dispatch.h>
#include <cma_kernel.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * Macros to lock and unlock a condition variable. On kernel-thread versions
 * where the DECthreads scheduler is "mostly invisible", use a spinlock in
 * the CV to reduce contention. On user-mode scheduler versions, lock the
 * DECthreads kernel since we need to access scheduling data anyway.
 */

#if _CMA_THREAD_IS_VP_
# define cma__cvlock(cv) cma__spinlock (&(cv)->spindle)
# define cma__cvtrylock(cv) (!cma__tryspinlock (&(cv)->spindle))
# define cma__cvunlock(cv) {if (!cma__queue_empty (&(cv)->queue)) \
    cma__unset (&(cv)->event); cma__spinunlock (&(cv)->spindle);}
# define cma__cv_kpost_pend(cv) cma__unset (&(cv)->nopend)
# define cma__cv_ifpend(cv) (!cma__test_and_set (&(cv)->nopend))
# define cma__cv_post_event(cv) cma__unset (&(cv)->event)
# define cma__cv_ifkevent(cv) (!cma__test_and_set (&(cv)->event))
#else
# define cma__cvlock(cv) cma__set_kernel ()
# define cma__cvtrylock(cv) cma__tryenter_kernel ()
# define cma__cvunlock(cv) {if (!cma__queue_empty (&(cv)->queue)) \
    cma__unset (&(cv)->event); cma__unset_kernel ();}
# define cma__cv_post_pend(cv) cma__unset (&(cv)->nopend)
# define cma__cv_kpost_pend(cv) cma__kernel_unset (&(cv)->nopend)
# define cma__cv_ifpend(cv) (!cma__kernel_set (&(cv)->nopend))
# define cma__cv_post_event(cv) cma__kernel_unset (&(cv)->event)
# define cma__cv_ifevent(cv) (!cma__test_and_set (&(cv)->event))
# define cma__cv_ifkevent(cv) (!cma__kernel_set (&(cv)->event))
#endif

#define cma__cv_ifevent(cv) (!cma__test_and_set (&(cv)->event))
#define cma__cv_post_pend(cv) cma__unset (&(cv)->nopend)
#define cma__int_cv_waiting(cv) (!cma__queue_empty (&(cv)->queue))

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Process any deferred wakeups on a condition variable. Should be
 *	called immediately following cma__cvunlock().
 *
 *  FORMAL PARAMETERS:
 *
 *	_cv_	Pointer to condition variable
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
#ifdef NDEBUG
# define cma__cvundefer(_cv_) \
    while (!cma__test_and_set (&(_cv_)->defer_sig)) { \
	cma_t_boolean _flag_ = (!cma__test_and_set (&(_cv_)->defer_bro)); \
	cma__cvlock (_cv_); cma__cv_wakeup (_cv_,_flag_); cma__cvunlockwake (_cv_);}
#else
# define cma__cvundefer(_cv_) \
    while (!cma__test_and_set (&(_cv_)->defer_sig)) {cma_t_boolean _flag_; \
	if (!cma__test_and_set (&(_cv_)->defer_bro)) { \
	    _flag_ = cma_c_true; (_cv_)->defbro_cnt++;} \
	else {_flag_ = cma_c_false; (_cv_)->defsig_cnt++;} \
	cma__cvlock (_cv_); cma__cv_wakeup (_cv_,_flag_); cma__cvunlockwake (_cv_);}
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wakeup waiters on a condition variable. This is NOT an internal
 *	replacement for cma_cond_wake() -- it's a primitive used by
 *	the internal function performing the wake, and it's "external" only
 *	because it's also needed by the alert operation.
 *
 *  FORMAL PARAMETERS:
 *
 *	_cv_	Pointer to condition variable
 *	_all_	Flag: 1 (cma_c_true) to wake all waiters, 0 to wake only one
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
 *	On kernel thread implementations, cma__cv_wakeup() unlocks the CV
 *	spinlock before actually readying threads, and cma__cvunlockwake()
 *	is null. On user mode context switch implementations,
 *	cma__cv_wakeup() can't unlock the CV -- so cma__cvunlockwake() does
 *	so.
 */

/*
 * NOTE: When a thread is removed from the queue, the TCB->header.queue
 * element is initialized as a *queue header*. This may seem a little silly,
 * but it allows a solution for a wait/signal_int race on OSF/1 that requires
 * testing for pending wake with the CV unlocked, which causes a great deal
 * of difficulty in determining whether the TCB must be dequeued from the CV,
 * unless we know it's *always* dequeued.
 *
 * For symmetry (for now), I'm doing the same everywhere.
 */
#if _CMA_THREAD_IS_VP_
# define cma__cv_wakeup(_cv_,_all_) { \
    cma__t_int_tcb	*__tcb__; \
    if (!cma__queue_empty (&(_cv_)->queue)) { \
	if (_all_) {cma__t_queue	_q_,*_tq_; \
	    cma__queue_zero (&_q_); \
	    cma__queue_insert (&_q_, &(_cv_)->queue); \
	    cma__queue_remove (&(_cv_)->queue, _tq_, cma__t_queue); \
	    cma__queue_init (&(_cv_)->queue); \
	    cma__cvunlock (_cv_); \
	    while (!cma__queue_empty (&_q_)) { \
		cma__queue_dequeue (&_q_, __tcb__, cma__t_int_tcb); \
		cma__queue_init (&__tcb__->header.queue); \
		cma__ready (__tcb__, cma_c_false); \
		cma__try_run (__tcb__); } } \
	else { \
	    cma__queue_dequeue (&(_cv_)->queue, __tcb__, cma__t_int_tcb); \
	    cma__queue_init (&__tcb__->header.queue); \
	    cma__cvunlock (_cv_); \
	    cma__ready (__tcb__, cma_c_false); \
	    cma__try_run (__tcb__); } } \
    else { cma__cv_kpost_pend (_cv_); cma__cvunlock (_cv_); } }
# define cma__cvunlockwake(__cv_)
#else
# define cma__cv_wakeup(_cv_,_all_) { \
    cma__t_int_tcb	*__tcb__; \
    if (!cma__queue_empty (&(_cv_)->queue)) { \
	if (_all_) {cma__t_queue	_q_,*_tq_; \
	    cma__queue_zero (&_q_); \
	    cma__queue_insert (&_q_, &(_cv_)->queue); \
	    cma__queue_remove (&(_cv_)->queue, _tq_, cma__t_queue); \
	    cma__queue_init (&(_cv_)->queue); \
	    while (!cma__queue_empty (&_q_)) { \
		cma__queue_dequeue (&_q_, __tcb__, cma__t_int_tcb); \
		cma__queue_init (&__tcb__->header.queue); \
		cma__ready (__tcb__, cma_c_false); \
		cma__try_run (__tcb__); } } \
	else { \
	    cma__queue_dequeue (&(_cv_)->queue, __tcb__, cma__t_int_tcb); \
	    cma__queue_init (&__tcb__->header.queue); \
	    cma__ready (__tcb__, cma_c_false); \
	    cma__try_run (__tcb__); } } \
    else { cma__cv_kpost_pend (_cv_); } }
# define cma__cvunlockwake(__cv_) cma__cvunlock (__cv_)
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Broadcast a condition variable (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	_cv_		Pointer to condition variable
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
#define cma__int_broadcast(_cv_) \
    if (cma__cv_ifevent ((cma__t_int_cv *)_cv_)) \
	cma__int_cond_wake ((cma__t_int_cv *)_cv_, cma_c_true, cma_c_false);

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_cond_delete - performs work for cma_cond_delete
 *
 *  FORMAL PARAMETERS:
 *
 *	cma_t_cond	*_condition_	    - Condition variable to free
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	New CV
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#define cma__int_cond_delete(_condition_) { \
    cma__t_int_cv       *_int_cv_; \
    (_int_cv_) = cma__validate_null_cv (_condition_); \
    if ((_int_cv_) == (cma__t_int_cv *)cma_c_null_ptr) return; \
    if (cma__int_cv_waiting (_int_cv_)) cma__error (cma_s_in_use); \
    cma__free_cv (_int_cv_); \
    cma__clear_handle (_condition_); \
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_cond_wait - Performs work for cma_cond_wait
 *
 *  FORMAL PARAMETERS:
 *
 *	cma_t_cond	*_condition_	- Condition variable to wait on
 *	cma_t_mutex	*_mutex_	_ Mutex to unlock during wait
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	New CV
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#ifdef NDEBUG
# define cma__int_cond_wait(_cond_,_mu_) { \
    cma__t_int_mutex    *_int_mu_; \
    cma__t_int_cv       *_int_cv_; \
    (_int_mu_) = (cma__t_int_mutex *)((cma__t_int_handle *)(_mu_))->pointer; \
    (_int_cv_) = (cma__t_int_cv *)((cma__t_int_handle *)(_cond_))->pointer; \
    cma__int_wait ((_int_cv_), (_int_mu_), cma__get_self_tcb()); \
    }
#else
# define cma__int_cond_wait(_condition_,_mutex_) { \
    cma__t_int_mutex    *_int_mutex_; \
    cma__t_int_cv       *_int_cv_; \
    (_int_mutex_) = cma__validate_mutex (_mutex_); \
    (_int_cv_) = cma__validate_cv (_condition_); \
    cma__int_wait ((_int_cv_), (_int_mutex_), cma__get_self_tcb()); \
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition variable (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	_cv_		Pointer to condition variable
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
#define cma__int_signal(_cv_) \
    if (cma__cv_ifevent ((cma__t_int_cv *)_cv_)) \
	cma__int_cond_wake ((cma__t_int_cv *)_cv_, cma_c_false, cma_c_false);

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition variable from interrupt level.
 *
 *	On kernel thread versions, we don't maintain much of a scheduling
 *	database, so there's no need to defer the wakeup -- just do it (for
 *	example, on DEC OSF/1, the wakeup is really just sending a message to
 *	a VP's "synch" port).
 *
 *	On user-mode thread systems, the null thread blocks the process.  To
 *	ensure that the block ends in good time, post a wake-up for the null
 *	thread.  When it receives the wake-up, it will resume, undefer, and
 *	yield.
 *
 *  FORMAL PARAMETERS:
 *
 *	_cv_		Pointer to condition variable
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
#if _CMA_THREAD_IS_VP_
# define cma__int_signal_int(_cv_) { \
    cma__cv_kpost_pend (_cv_); \
    cma__int_cond_wake ((cma__t_int_cv *)(_cv_), cma_c_false, cma_c_true); }
#else
# define cma__int_signal_int(_cv_) { \
    cma__cv_kpost_pend (_cv_); \
    cma__unset (&((cma__t_int_cv *)_cv_)->defer_sig); \
    cma__unset (&cma__g_mgr_wake); \
    cma__unset (&cma__g_cv_defer); \
    cma__nt_post_wakeup ();}
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition variable from interrupt level, preempting the
 *	current thread immediately, if appropriate.
 *
 *	This operation must occur at the END of an interrupt service routine,
 *	as the interrupt state (and any atomicity guarantees that entails)
 *	may evaporate before this operation completes (dispatch to another
 *	thread entails clearing the interrupt state).
 *
 *	Finally, note that with kernel threads there's nothing special about
 *	this: DECthreads scheduling state is minimal enough that there's no
 *	need for funny stuff. On user-mode thread versions, cma_condition.h
 *	declares an extern prototype for this as a function (implemented in
 *	cma_condition.c).
 *
 *  FORMAL PARAMETERS:
 *
 *	_cv_		Pointer to condition variable
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
#if _CMA_THREAD_IS_VP_
# define cma__int_signal_preempt_int(cv, dummy) \
    cma__int_cond_wake ((cma__t_int_cv *)(cv), cma_c_false, cma_c_true);
#endif

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

extern cma__t_atomic_bit	cma__g_cv_defer;

/*
 * INTERNAL INTERFACES
 */

extern void
cma__free_cv _CMA_PROTOTYPE_ ((		/* Free a CV structure */
	cma__t_int_cv	*old_cv));

extern cma__t_int_cv *
cma__get_cv _CMA_PROTOTYPE_ ((		/* Allocate a new CV structure */
	cma__t_int_attr	*attr));

extern void
cma__init_cv _CMA_PROTOTYPE_ ((void));	/* Initialize CV module */

extern void
cma__int_cond_wake _CMA_PROTOTYPE_ ((	/* Wake up waiting threads */
	cma__t_int_cv		*cv,
	cma_t_boolean		all,
	cma_t_boolean		trylock));

#if !_CMA_THREAD_IS_VP_
# if _CMA_OS_ == _CMA__UNIX
extern void
cma__int_signal_preempt_int		/* Wake immediately, from interrupt */
    _CMA_PROTOTYPE_ ((
	cma__t_int_cv	*cond,
	cma_t_address	scp));
# else
extern void
cma__int_signal_preempt_int		/* Wake immediately, from interrupt */
    _CMA_PROTOTYPE_ ((
	cma__t_int_cv	*cond));
# endif
#endif

extern cma_t_status
cma__int_timed_wait _CMA_PROTOTYPE_ ((	/* Wait on CV, with timeout */
	cma__t_int_cv		*condition,
	cma__t_int_mutex	*mutex,
	cma_t_date_time		*timeout,
	cma__t_int_tcb		*self));

extern void
cma__int_wait _CMA_PROTOTYPE_ ((	/* Wait on CV */
	cma__t_int_cv		*condition,
	cma__t_int_mutex	*mutex,
	cma__t_int_tcb		*self));

extern void
cma__reinit_cv _CMA_PROTOTYPE_ ((	/* Reinit module after fork() */
	cma_t_integer		flag));

#if !_CMA_THREAD_IS_VP_
extern int
cma__undefer_cv _CMA_PROTOTYPE_ ((void));	/* Undefer CV operations */
#endif

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONDITION.H */
/*  *39    2-JUL-1993 14:37:43 BUTENHOF "Fix wait/signal_int race for OSF/1" */
/*  *38   18-JUN-1993 06:44:27 BUTENHOF "check CMS problem..." */
/*  *36   28-MAY-1993 12:17:33 BUTENHOF "Fix OSF/1 pending wake" */
/*  *35   27-MAY-1993 14:31:58 BUTENHOF "Remove OSF/1 performance glitch" */
/*  *34   16-APR-1993 13:02:38 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *33   29-MAR-1993 13:56:12 BUTENHOF "Integrate March 10 review comments" */
/*  *32   23-FEB-1993 16:28:24 SCALES "Fix two wake-up waiting races in the null thread" */
/*  *31    4-DEC-1992 12:41:15 BUTENHOF "Fix cma__set typos" */
/*  *30   23-NOV-1992 14:00:19 BUTENHOF "Fix kthread CVs" */
/*  *29   12-NOV-1992 11:52:37 BUTENHOF "Fix alert hole" */
/*  *28    5-NOV-1992 14:24:40 BUTENHOF "Fix async alert" */
/*  *27    2-NOV-1992 13:25:08 BUTENHOF "Speedyize & fix race" */
/*  *26   16-OCT-1992 08:17:17 BUTENHOF "Fix VP trylock" */
/*  *25    5-OCT-1992 15:50:43 BUTENHOF "Remove kernel-sets inside kernel" */
/*  *24    5-OCT-1992 15:07:36 BUTENHOF "Move defer bit init" */
/*  *23   30-SEP-1992 15:20:07 BUTENHOF "Fix typo in OSF/1 code" */
/*  *22   21-SEP-1992 13:31:17 BUTENHOF "Reset 'event' on cvunlock" */
/*  *21   17-SEP-1992 14:35:28 BUTENHOF "Make cv_wakeup extern" */
/*  *20   15-SEP-1992 13:49:08 BUTENHOF "Fix prototype" */
/*  *19   11-SEP-1992 05:31:31 BUTENHOF "Remove delay_mutex/cv externs" */
/*  *18    4-SEP-1992 15:52:52 SCALES "Add cond-signal-preempt-int" */
/*  *17    4-SEP-1992 10:08:12 BUTENHOF "Restore $wake on VMS" */
/*  *16    2-SEP-1992 16:23:55 BUTENHOF "Separate semaphores from kernel lock" */
/*  *15   25-AUG-1992 11:47:50 BUTENHOF "Cut down on event bit" */
/*  *14   21-AUG-1992 13:41:25 BUTENHOF "Use spinlocks on kernel thread semaphores instead of kernel_critical" */
/*  *13   18-FEB-1992 15:27:44 BUTENHOF "Remove macros for create object" */
/*  *12   22-NOV-1991 11:55:02 BUTENHOF "Split into cma_condition.h and cma_cond_defs.h" */
/*  *11   10-JUN-1991 19:50:55 SCALES "Convert to stream format for ULTRIX build" */
/*  *10   10-JUN-1991 19:20:08 BUTENHOF "Fix the sccs headers" */
/*  *9    10-JUN-1991 18:17:46 SCALES "Add sccs headers for Ultrix" */
/*  *8    24-MAY-1991 16:43:50 CURTIN "Added a new reinit routine" */
/*  *7    17-MAY-1991 15:10:10 BUTENHOF "Unset ""event"" in cond wait before releasing mutex" */
/*  *6    10-MAY-1991 11:56:35 CURTIN "Put to use a number of new macros" */
/*  *5    12-APR-1991 23:34:56 BUTENHOF "Change type of internal locks" */
/*  *4     3-APR-1991 10:51:46 BUTENHOF "Remove pending wakeup" */
/*  *3     5-FEB-1991 00:59:33 BUTENHOF "Change prototypes for int CV ops" */
/*  *2    14-DEC-1990 00:55:15 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:42:47 BUTENHOF "Condition variables" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONDITION.H */
