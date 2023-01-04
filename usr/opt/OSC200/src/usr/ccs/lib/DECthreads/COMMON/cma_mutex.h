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
 * @(#)$RCSfile: cma_mutex.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:48:25 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Header file for mutex operations
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	3 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	14 September 1989
 *		Add semaphore to mutex.
 *	002	Dave Butenhof	04 October 1989
 *		Implement internal mutex lock/unlock using object pointers
 *		instead of handles for convenient internal use.
 *	003	Dave Butenhof	1 November 1989
 *		Define queue of active mutexes.
 *	004	Dave Butenhof	22 November 1989
 *		Add "owner" field so debug code can report current owner of
 *		mutex.
 *	005	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	006	Dave Butenhof	9 April 1990
 *		Remove definition of known mutex queue header; it's now in
 *		cma_debugger.
 *	007	Dave Butenhof	15 June 1990
 *		Streamline mutex performance by using interlocked bit;
 *		semaphores will be used only to block the thread if the mutex
 *		is already locked.
 *	008	Dave Butenhof	26 June 1990
 *		Implement "friendly" mutexes which support nested locking by
 *		the same thread.  Use these to implement a new "global lock"
 *		primitive for use in synchronizing non-reentrant libraries.
 *	009	Dave Butenhof	31 July 1990
 *		Move internal mutex operations to .h so they can be inlined
 *		everywhere within CMA.
 *	010	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	011	Dave Butenhof	12 December 1991
 *		Change comments to reflect change of "friendly" mutex to
 *		"recursive".
 *	012	Dave Butenhof	09 April 1991
 *		Use new type for "atomic bit" operation target
 *	013	Paul Curtin	10 May 1991
 *		Added a couple of new macros.
 *	014	Paul Curtin	30 May 1991
 *		Added a proto for cma__reinit_mutex
 *	015	Dave Butenhof	09 March 1992
 *		Modify cma__int_[un]lock to handle return status from
 *		cma__int_mutex_[un]block(), raising exception on failure.
 *	016	Dave Butenhof	26 August 1992
 *		Get rid of semaphores!
 *	017	Dave Butenhof	15 September 1992
 *		Get rid of "sequence object" (it's in known object
 *		structure).
 *	018	Dave Butenhof	 5 October 1992
 *		Add macros for waiter bit access.
 *	019	Dave Butenhof	16 October 1992
 *		As used in code, cma__mutrylock() returns previous state of
 *		bit; but cma__tryspinlock() returns "true" if lock is
 *		acquired. So reverse the sense of VP macro.
 *	020	Dave Butenhof	30 October 1992
 *		Enhance performance (at the possible expense of occasional
 *		latency) by changing mulock/muunlock to use set/unset kernel
 *		rather than enter/exit, saving cost of checking for global
 *		undeferral.
 *	056	Dave Butenhof	 3 December 1992
 *		Fix typo reference to cma__set (which doesn't exist)
 */

#ifndef CMA_MUTEX
#define CMA_MUTEX

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_attr.h>
#include <cma_defs.h>
#include <cma_tcb_defs.h>
#include <cma_stack.h>

/*
 * CONSTANTS AND MACROS
 */

#if _CMA_THREAD_IS_VP_
# define cma__mulock(mutex) cma__spinlock (&(mutex)->spindle)
# define cma__mutrylock(mutex) (!cma__tryspinlock (&(mutex)->spindle))
# define cma__muunlock(mutex) cma__spinunlock (&(mutex)->spindle)
# define cma__mu_post_event(mutex) cma__unset (&(mutex)->event)
# define cma__mu_ifevent(mutex) (!cma__test_and_set (&(mutex)->event))
# define cma__mu_set_waiter(mutex) cma__unset (&(mutex)->waiters)
# define cma__mu_ifwaiter(mutex) (!cma__test_and_set (&(mutex)->waiters))
#else
# define cma__mulock(mutex) cma__set_kernel ()
# define cma__mutrylock(mutex) cma__tryenter_kernel ()
# define cma__muunlock(mutex) cma__unset_kernel ()
# define cma__mu_post_event(mutex) cma__kernel_unset (&(mutex)->event)
# define cma__mu_ifevent(mutex) (!cma__kernel_set (&(mutex)->event))
# define cma__mu_set_waiter(mutex) cma__kernel_unset (&(mutex)->waiters)
# define cma__mu_ifwaiter(mutex) (!cma__kernel_set (&(mutex)->waiters))
#endif

/*
 * TYPEDEFS
 */

typedef struct CMA__T_INT_MUTEX {
    cma__t_object	header;		/* Common header (sequence, type) */
    cma__t_int_attr	*attributes;	/* Back link */
    cma__t_int_tcb	*owner;		/* Current owner (if any) */
    cma_t_integer	nest_count;	/* Nesting level for recursive mutex */
    cma_t_mutex_kind	mutex_kind;	/* Kind of mutex */
    cma__t_queue	queue;		/* Queue of waiters */
    cma__t_atomic_bit	*unlock;	/* Pointer used for unlock operation */
    cma__t_atomic_bit	lock;		/* Set if currently locked */
    cma__t_atomic_bit	event;		/* Clear when unlock requires action */
    cma__t_atomic_bit	bitbucket;	/* Fake bit to disable unlock */
    cma__t_atomic_bit	waiters;	/* Clear when threads are waiting */
    cma__t_atomic_bit	spindle;	/* Control internal access */
    } cma__t_int_mutex;


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Lock a mutex (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Pointer to mutex object to lock
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
# define cma__int_lock(mutex) { \
    if (cma__test_and_set (&((cma__t_int_mutex *)mutex)->lock)) { \
	cma_t_status	res;\
	res = cma__int_mutex_block ((cma__t_int_mutex *)mutex); \
	if (res != cma_s_normal) cma__error (res); \
	} \
    }
#else
# define cma__int_lock(mutex) { \
    cma__t_int_tcb *__ltcb__; \
    __ltcb__ = cma__get_self_tcb (); \
    if (cma__test_and_set (&((cma__t_int_mutex *)mutex)->lock)) { \
	cma_t_status	res;\
	res = cma__int_mutex_block ((cma__t_int_mutex *)mutex); \
	if (res != cma_s_normal) cma__error (res); \
	} \
    ((cma__t_int_mutex *)mutex)->owner = __ltcb__; \
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unlock a mutex (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Pointer to mutex object to unlock
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
# define cma__int_unlock(mutex) { \
    cma__unset (((cma__t_int_mutex *)mutex)->unlock); \
    if (!cma__test_and_set (&((cma__t_int_mutex *)mutex)->event)) { \
	cma_t_status	res;\
	res = cma__int_mutex_unblock ((cma__t_int_mutex *)mutex); \
	if (res != cma_s_normal) cma__error (res);}}
#else
# define cma__int_unlock(mutex) { \
    cma__t_int_tcb	*__utcb__; \
    __utcb__ = cma__get_self_tcb (); \
    if (((cma__t_int_mutex *)mutex)->mutex_kind == cma_c_mutex_fast) { \
	cma__assert_warn ( \
		(__utcb__ == ((cma__t_int_mutex *)mutex)->owner), \
		"attempt to release mutex owned by another thread"); \
	((cma__t_int_mutex *)mutex)->owner = (cma__t_int_tcb *)cma_c_null_ptr; \
	} \
    cma__unset (((cma__t_int_mutex *)mutex)->unlock); \
    if (!cma__test_and_set (&((cma__t_int_mutex *)mutex)->event)) { \
	cma_t_status	res;\
	res = cma__int_mutex_unblock ((cma__t_int_mutex *)mutex); \
	if (res != cma_s_normal) cma__error (res);}}
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_mutex_delete - Performs work for cma_mutex_delete
 *
 *  FORMAL PARAMETERS:
 *
 *	cma__t_mutex	    _mutex_	- Mutex to be deleted
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
#define cma__int_mutex_delete(_mutex_) { \
    cma__t_int_mutex    *_int_mutex_; \
    _int_mutex_ = cma__validate_null_mutex (_mutex_); \
    if (_int_mutex_ == (cma__t_int_mutex *)cma_c_null_ptr) \
        return; \
    if (cma__int_mutex_locked (_int_mutex_)) \
        cma__error (cma_s_in_use); \
    cma__free_mutex (_int_mutex_); \
    cma__clear_handle (_mutex_); \
    }


/*
 *  GLOBAL DATA
 */

extern cma__t_int_mutex	*cma__g_global_lock;

/*
 * INTERNAL INTERFACES
 */

extern void
cma__destroy_mutex _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*old_mutex)); /* Deallocate (don't cache) */

extern void
cma__free_mutex _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*old_mutex)); /* Mutex object to free */

extern void
cma__free_mutex_nolock _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*old_mutex)); /* Mutex object to free */

extern cma__t_int_mutex *
cma__get_first_mutex _CMA_PROTOTYPE_ ((
	cma__t_int_attr	*attr));	/* Attributes object to use */

extern cma__t_int_mutex *
cma__get_mutex _CMA_PROTOTYPE_ ((
	cma__t_int_attr	*attr));	/* Attributes object to use */

extern void
cma__init_mutex _CMA_PROTOTYPE_ ((void));

extern cma_t_status
cma__int_mutex_block _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*mutex));

extern cma_t_boolean
cma__int_mutex_locked _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*mutex));

extern cma_t_boolean
cma__int_try_lock _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*mutex));

extern cma_t_status
cma__int_mutex_unblock _CMA_PROTOTYPE_ ((
	cma__t_int_mutex	*mutex));

extern cma_t_boolean
cma__mutex_locked _CMA_PROTOTYPE_ ((
	cma_t_mutex	*mutex));

extern void
cma__reinit_mutex _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_MUTEX.H */
/*  *21    2-JUL-1993 14:38:08 BUTENHOF "Fiddle w/ queue element on unblock" */
/*  *20    4-DEC-1992 12:42:01 BUTENHOF "Fix cma__set typos" */
/*  *19    2-NOV-1992 13:25:21 BUTENHOF "Speedyize & fix race" */
/*  *18   16-OCT-1992 08:17:21 BUTENHOF "Fix VP trylock" */
/*  *17    5-OCT-1992 15:50:53 BUTENHOF "Remove kernel-sets inside kernel" */
/*  *16   22-SEP-1992 12:39:28 BUTENHOF "Fix mutex locking" */
/*  *15   15-SEP-1992 13:49:56 BUTENHOF "Change sequencing" */
/*  *14    2-SEP-1992 16:25:36 BUTENHOF "Separate semaphores from kernel lock" */
/*  *13   21-AUG-1992 13:42:15 BUTENHOF "Use spinlocks on kernel thread semaphores instead of kernel_critical" */
/*  *12   10-MAR-1992 16:26:18 BUTENHOF "Eliminate need for TRY/CATCH on pthread mutex lock" */
/*  *11   18-FEB-1992 15:29:33 BUTENHOF "Remove macros for create object" */
/*  *10   10-JUN-1991 19:54:29 SCALES "Convert to stream format for ULTRIX build" */
/*  *9    10-JUN-1991 19:21:09 BUTENHOF "Fix the sccs headers" */
/*  *8    10-JUN-1991 18:22:30 SCALES "Add sccs headers for Ultrix" */
/*  *7     5-JUN-1991 16:13:23 CURTIN "fork work" */
/*  *6    10-MAY-1991 11:46:33 CURTIN "Added a couple of new macros" */
/*  *5    12-APR-1991 23:36:12 BUTENHOF "Change type of internal locks" */
/*  *4    13-FEB-1991 17:54:51 BUTENHOF "Change name of mutex attributes" */
/*  *3    12-FEB-1991 23:09:58 BUTENHOF "Recursive/nonrecursive mutexes" */
/*  *2    14-DEC-1990 00:55:43 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:47:32 BUTENHOF "Mutexes" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_MUTEX.H */
