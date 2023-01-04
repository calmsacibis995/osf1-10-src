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
static char *rcsid = "@(#)$RCSfile: cma_condition.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:43:47 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Operations on condition variable objects
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
 *	001	Dave Butenhof	18 September 1989
 *		Implement condition variables on top of dispatcher's
 *		semaphores.
 *	002	Dave Butenhof	4 October 1989
 *		Implement internal interfaces to wait, signal, broadcast.
 *	003	Dave Butenhof	9 October 1989
 *		Use cma__error to raise exceptions where necessary.
 *	004	Dave Butenhof	11 October 1989
 *		Convert to use internal mutex operations.
 *	005	Dave Butenhof	18 October 1989
 *		cma__queue_insert is now a macro, which expands to a block;
 *		this module includes a call which is the sole statement on
 *		the "then" of an if statement, and the trailing ";" (after a
 *		"}") breaks the "else" clause.  Fix the reference in such a
 *		way that some future conversion back to a routine call won't
 *		break it again.
 *	006	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *	007	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	008	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *      009     Webb Scales     19 October 1989
 *              Add type-casts where MIPS pcc requires them.
 *	010	Dave Butenhof	24 October 1989
 *		Enhance free & destroy to handle attr. obj. delete-pending
 *		and refcnt.
 *	011	Dave Butenhof	24 October 1989
 *		Implement the timer-oriented services (cma_get_expiration,
 *		cma_expired, and cma_delay).
 *	012	Dave Butenhof	27 October 1989
 *		Hook up waits to handle alerts...
 *	013	Dave Butenhof	All Saints Day 1989
 *		Include stack header file to get cma__get_self_tcb.
 *	014	Dave Butenhof	1 November 1989
 *		Put all conditions on a queue.
 *	015	Dave Butenhof	16 November 1989
 *		- Change time parameter types from "cma__t_ticks" (obsolete) to
 *		  cma_t_date_time.
 *		- Finish timed service implementation.
 *	016	Dave Butenhof	22 November 1989
 *		decrement_semaphore now requires caller to have kernel
 *		locked, and to pass TCB.  This allows moving the
 *		condition-specific checks for alert-pending from semaphore to
 *		here, where they belong!
 *	017	Dave Butenhof	30 November 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	018	Dave Butenhof	5 December 1989
 *		Reverse arguments to cma_time_get_expiration to match name's
 *		claim of "time" as primary "object".
 *	019	Dave Butenhof	11 December 1989
 *		The "disable" bit should be ignored for wait, since it
 *		controls *asynchronous* alerts only!
 *	020	Dave Butenhof & Webb Scales	14 December 1989
 *		cma_cond_signal should increment if semaphore is <= 0 to
 *		generate a "pending spurious wakeup" to catch late waiters.
 *	021	Dave Butenhof	12 February 1990
 *		Simplify cma_delay somewhat by using a local static mutex
 *		and CV, instead of dynamically creating and then deleting a
 *		scratch mutex and condition.
 *	022	Dave Butenhof	9 April 1990
 *		Use new "known_object" structure for known condition queue
 *		(includes mutex).
 *	023	Dave Butenhof	10 April 1990
 *		Catch memory errors over object allocation, and set names in
 *		internal objects.
 *	024	Dave Butenhof	23 May 1990
 *		Make delay mutex & cv extern for pthread delay
 *	025	Webb Scales	30 May 1990
 *		Put FINALLY clause in for cma_delay.
 *	026	Dave Butenhof	5 June 1990
 *		Support cache high water mark.
 *	027	Dave Butenhof	7 June 1990
 *		Erase destroyed objects if not NDEBUG, to catch references to
 *		dead objects (can be used in conjunction with cma_maxcond=0,
 *		which disables caching).
 *	028	Dave Butenhof	18 June 1990
 *		Use macros to clear object name (only defined for debug
 *		build).
 *	029	Dave Butenhof	03 August 1990
 *		Use new semaphore primitives (simplified and inlined).
 *	030	Dave Butenhof	08 August 1990
 *		Post pending wake on semaphore if no waiters, with single
 *		interlocked instruction.
 *	031	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	032	Webb Scales	30 October 1990
 *		Removed uses of external handle fields.
 *	033	Webb Scales	30 October 1990
 *		Added casts to change #32.
 *	034	Dave Butenhof	26 November 1990
 *		Fix a bug in signal; it set cv->event even though a signal
 *		may leave other waiters. Signal must declare that there may
 *		still be waiters.
 *	035	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	036	Dave Butenhof	1 February 1991
 *		Improve inlining of internal cv operations; and change args
 *		in int_wait to conform to external interface for consistency.
 *	037	Dave Butenhof	7 February 1991
 *		Changes to alert control bit names.
 *	038	Dave Butenhof	02 April 1991
 *		Remove pending wakeup when there are definitely no waiters.
 *	039	Dave Butenhof	09 April 1991
 *		Use new constants to init atomic bits
 *	040	Paul Curtin	10 May 1991
 *		Put a number of new macros to use.
 *	041	Paul Curtin	24 May 1991
 *		Added a cma__reinit_cv routine.
 *	042	Paul Curtin	31 May 1991
 *		Added code to clear cv waiter queues in reinit routine.
 *	043	Dave Butenhof	10 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	044	Dave Butenhof	05 June 1992
 *		Fix delete_pending attr handling in free code -- it needs to
 *		free the attr. obj. instead of destroying it (which also
 *		fixes a race in attr. handling).
 *	045	Dave Butenhof	26 August 1992
 *		Remove semaphores from code, and generally streamline and
 *		clean things up. Remove global defer queue -- instead, use
 *		per-CV atomic flags.
 *	046	Webb Scales	 3 September 1992
 *		Add cond-signal-preempt-int routines.
 *	047	Dave Butenhof & Webb Scales	09 September 1992
 *		Remove references to delay_mutex.
 *	048	Dave Butenhof	14 September 1992
 *		Add debug fields for deferred operations and pending wakes.
 *	049	Dave Butenhof	17 September 1992
 *		Turn cma___cv_wakeup into cma__cv_wakeup so alert can use it.
 *	050	Dave Butenhof	21 September 1992
 *		Move check for non-empty waiter queue out of cma__int_wait
 *		and into cma__cvunlock() macro so all unlocks will benefit.
 *	051	Dave Butenhof	 5 October 1992
 *		Use ifevent macros to ensure safe kernel use (use kernel_set
 *		on systems where atomic test-and-set locks kernel).
 *	052	Dave Butenhof	 7 October 1992
 *		Fix a kernel deadlock in wait -- it cvlocked condition before
 *		unlocking the associated mutex (which mulocked the mutex). I
 *		don't really see any need to unlock the mutex after the
 *		cvlock, since it doesn't gain any real atomicity.
 *	053	Dave Butenhof	 8 October 1992
 *		??? Somehow, my 052 changes didn't get in. ???
 *	054	Dave Butenhof	16 October 1992
 *		Fix the blocking code for VPs (sense of finish_block arg was
 *		wrong).
 *	055	Dave Butenhof	30 October 1992
 *		Don't set global CV defer bit for a normal cond_wake that's
 *		locked out -- that'll cause exit/enter kernel to search all
 *		known CVs for something to defer. We don't need that, since
 *		if the CV is currently locked, someone will soon UNLOCK it,
 *		and process the defer bits. The only time we need the global
 *		defer bit is for signal_int, which doesn't know whether the
 *		CV is locked (or ever will be).
 *	056	Dave Butenhof	30 October 1992
 *		Remove 'pend' arg from int_cond_wake: ALWAYS post a pending
 *		wake if the 'event' is set (solves possible wakeup waiting
 *		race with prioritized threads).
 *	057	Dave Butenhof	 5 November 1992
 *		055 is great for kernel threads, where I used CV spinlock;
 *		unfortunately, on user thread versions, ALL cv access is
 *		through kernel lock, so there ISN'T any guarantee that some
 *		thread will look at the CV soon. In particular, signalling
 *		the "master thread" on OpenVMS is rather sensitive to this.
 *		So restore setting of global defer bit if !_CMA_THREAD_IS_VP_
 *	058	Dave Butenhof	Friday the 13th, November 1992
 *		Reset all the defer counters and flags in reinit_cv clear, so
 *		that we can do this before releasing kernel lock in child
 *		fork wrapper, and not have to worry about manager thread
 *		getting run before we're finished with cleanup.
 *	059	Dave Butenhof	23 November 1992
 *		Modify condition wakeup function to allow condition or
 *		unconditional lock -- on kernel threads we usually want an
 *		unconditional lock to avoid races.
 *	060	Dave Butenhof	24 November 1992
 *		Fix bug in 059
 *	061	Brian Keane	21 December 1992
 *		Initialized spindle field in cv structure.
 *	062	Webb Scales	 8 February 1993
 *		Added a call in CV-undefer to clear NT pending wakeup.
 *	063	Brian Keane 3 March 1993
 *		Remove unnecessary mutex unlock in the cma__get_cv error path
 *	064	Dave Butenhof	29 March 1993
 *		Integrate March 10 review comments.
 *	065	Dave Butenhof	31 March 1993
 *		Use better name strings for user objects.
 *	066	Dave Butenhof	12 April 1993
 *		Step one in campaign to cut down cma__get_self_tcb() calls:
 *		add it as argument to cma__int[_timed]_wait (several callers
 *		also use it themselves).
 *	067	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	068	Dave Butenhof	21 May 1993
 *		cma__cv_wakeup() has been redesigned to release the CV lock
 *		on DEC OSF/1 before actually doing wakes. On other
 *		platforms, the lock can't be released until after, so there's
 *		a new cma__cvunlockwake() macro that's null on DEC OSF/1.
 *	069	Brian Keane	10 June 1993
 *		Touch up conditional code in cma_cond_signal_preempt_int - 
 *		cma__int_signal_preempt_int is a macro when _CMA_THREAD_IS_VP_
 *		is defined, and some compilers don't like #ifs in macro expansions.
 *	070	Brian Keane	14 June 1993
 *		Clear cv event bit in timeout path of timed wait - this was
 *		causing unintended pending wakeups to be posted.
 *	071	Dave Butenhof	21 June 1993
 *		Delay check for pending wake in condition wait until just
 *		before dispatch (in OSF/1, *after* the CV lock has been
 *		released; this solves race where cond_signal_int could find
 *		CV locked and set pending wake after waiter has tested it).
 *	072	Dave Butenhof	29 July 1993
 *		Fix bug in timed condition wait, caused by 071 -- when the
 *		timer is pre-expired, the thread isn't put on the CV queue;
 *		but it's still removed. Ooops.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_defer.h>
#include <cma_alert.h>
#include <cma_attr.h>
#include <cma_mutex.h>
#include <cma_condition.h>
#include <cma_handle.h>
#include <cma_vm.h>
#include <cma_stack.h>
#include <cma_errors.h>
#include <cma_timer.h>
#include <cma_dispatch.h>
#include <cma_deb_core.h>
#include <cma_init.h>
#include <cma_vp.h>
#if _CMA_OS_ == _CMA__UNIX
# include <signal.h>
#endif

/*
 * GLOBAL DATA
 */

cma__t_atomic_bit	cma__g_cv_defer;

/*
 * LOCAL MACROS
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Broadcast a condition variable
 *
 *  FORMAL PARAMETERS:
 *
 *	condition		Condition variable object to broadcast
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
cma_cond_broadcast
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition)	/* Condition to broadcast */
#else	/* no prototypes */
	(condition)
	cma_t_cond	*condition;	/* Condition to broadcast */
#endif	/* prototype */
    {
    cma__t_int_cv	*int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)condition)->pointer;
#else
    int_cv = cma__validate_cv (condition);
#endif

    /*
     * Unblock all waiters
     */
    cma__int_broadcast (int_cv);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a new condition variable object.
 *
 *  FORMAL PARAMETERS:
 *
 *	new_condition	Output handle
 *
 *	att		Attributes object to use in creation
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
cma_cond_create
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*new_condition,	/* New handle to fill in */
	cma_t_attr	*att)		/* Old attr obj to use */
#else	/* no prototypes */
	(new_condition, att)
	cma_t_cond	*new_condition;	/* New handle to fill in */
	cma_t_attr	*att;		/* Old attr obj to use */
#endif	/* prototype */
    {
    cma__t_int_cv               *cv; 
    cma__t_int_attr             *int_att; 


    int_att = cma__validate_default_attr (att); 
    cv = cma__get_cv (int_att); 

    if ((cma_t_address)cv == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)cv, new_condition); 
	cma__obj_set_owner (cv, (cma_t_integer)new_condition);
	cma__obj_set_name (cv, "<CMA user@0x%lx>");
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delay for some amount of time.
 *
 *  FORMAL PARAMETERS:
 *
 *	interval	length of time to wait
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
cma_delay
#ifdef _CMA_PROTO_
	(
	cma_t_interval	interval)
#else	/* no prototypes */
	(interval)
	cma_t_interval	interval;
#endif	/* prototype */
    {
    cma_t_date_time	expiration;
    cma__t_int_tcb	*self;


    self = cma__get_self_tcb ();
    cma_time_get_expiration (&expiration, interval);
    cma__int_lock (self->tswait_mutex);	/* Lock the mutex */

    /*
     * Wait until the specified time. Just ignore any signals on the CV,
     * since we only care about timeout. If an alert occurs, release the
     * TCB's mutex and resignal the alert for the client to handle.
     */
    TRY {
	while (cma__int_timed_wait (
		    self->tswait_cv,
		    self->tswait_mutex,
		    &expiration,
		    self) != cma_s_timed_out);
	}
    FINALLY {
	cma__int_unlock (self->tswait_mutex);
	}
    ENDTRY

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete (free) a condition variable object
 *
 *  FORMAL PARAMETERS:
 *
 *	condition	Condition variable object to free
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
cma_cond_delete
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition)	/* Condition variable to free */
#else	/* no prototypes */
	(condition)
	cma_t_cond	*condition;	/* Condition variable to free */
#endif	/* prototype */
    {
    cma__int_cond_delete (condition);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition variable
 *
 *  FORMAL PARAMETERS:
 *
 *	condition		Condition variable object to signal
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
cma_cond_signal
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition)	/* Condition to signal */
#else	/* no prototypes */
	(condition)
	cma_t_cond	*condition;	/* Condition to signal */
#endif	/* prototype */
    {
    cma__t_int_cv	*int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)condition)->pointer;
#else
    int_cv = cma__validate_cv (condition);
#endif

    /*
     * Unblock one waiter
     */
    cma__int_signal (int_cv);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Request a condition variable signal, from an interrupt routine
 *
 *  FORMAL PARAMETERS:
 *
 *	condition		Condition variable object to signal
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The "defer" bits in the condition variable and possibly the kernel
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	Note:  This routine does not actually signal the indicated condition 
 *		variable.  Instead it causes it to be signalled at the next 
 *		time a thread unlocks the condition variable
 */
extern void
cma_cond_signal_int
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition)	/* Condition to signal */
#else	/* no prototypes */
	(condition)
	cma_t_cond	*condition;	/* Condition to signal */
#endif	/* prototype */
    {
    cma__t_int_cv	*int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)condition)->pointer;
#else
    int_cv = cma__validate_cv (condition);
#endif

    cma__int_signal_int (int_cv);
    }	

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition variable from an interrupt routine, willing to
 *	be preempted (i.e., the signal may be immediate).
 *
 *	ASSUMPTIONS:
 *	    1) This routine is only called by an AST or Signal.
 *
 *  FORMAL PARAMETERS:
 *
 *	condition		Condition variable object to signal
#if _CMA_OS_ == _CMA__UNIX
 *	scp			address of signal context structure
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
 *	Note:  this routine may be preempted by another thread, so the 
 *		guarantees of atomicity or non-interruptibility of software
 *		interrupts ends when this routine is called.  And, when this
 *		routine returns, the process may not be "at interrupt level"
 *		any more!
 */
extern void
cma_cond_signal_preempt_int
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition	/* Condition to signal */
# if _CMA_OS_ == _CMA__UNIX
	,cma_t_address	scp		/* Address of signal ctx structure */
# endif
	)
#else	/* no prototypes */
# if _CMA_OS_ == _CMA__UNIX
	(condition, scp)
	cma_t_cond	*condition;	/* Condition to signal */
	cma_t_address	scp;		/* Address of signal ctx structure */
# else
	(condition)
	cma_t_cond	*condition;	/* Condition to signal */
# endif
#endif	/* prototype */
    {
    cma__t_int_cv	*int_cv;


#ifdef NDEBUG
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)condition)->pointer;
#else
    int_cv = cma__validate_cv (condition);
#endif

#if _CMA_OS_ == _CMA__UNIX
    cma__int_signal_preempt_int (int_cv, scp);
#else
    cma__int_signal_preempt_int (int_cv);
#endif
    }	

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait on a condition variable, with a timeout.
 *
 *  FORMAL PARAMETERS:
 *
 *	condition	Condition variable object to wait on.
 *
 *	mutex		Mutex to release during wait.
 *
 *	expiration	Time at which wait should expire
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
extern cma_t_status
cma_cond_timed_wait
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition,	/* Condition variable to wait on */
	cma_t_mutex	*mutex,		/* Mutex to unlock during wait */
	cma_t_date_time	*expiration)	/* Timeout */
#else	/* no prototypes */
	(condition, mutex, expiration)
	cma_t_cond	*condition;	/* Condition variable to wait on */
	cma_t_mutex	*mutex;		/* Mutex to unlock during wait */
	cma_t_date_time	*expiration;	/* Timeout */
#endif	/* prototype */
    {
    cma__t_int_mutex	*int_mutex;
    cma__t_int_cv	*int_cv;
    cma__t_int_tcb	*self = cma__get_self_tcb ();


#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
    int_cv = (cma__t_int_cv *)((cma__t_int_handle *)condition)->pointer;
#else
    int_mutex = cma__validate_mutex (mutex);
    int_cv = cma__validate_cv (condition);
#endif

#if _CMA_OS_ != _CMA__VMS
    if (expiration->tv_usec >= 1000000 || expiration->tv_usec < 0)
	cma__error (cma_s_badparam);
#endif

    return cma__int_timed_wait (int_cv, int_mutex, expiration, self);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait on a condition variable
 *
 *  FORMAL PARAMETERS:
 *
 *	condition	Condition variable object to wait on.
 *
 *	mutex		Mutex to release during wait.
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
cma_cond_wait
#ifdef _CMA_PROTO_
	(
	cma_t_cond	*condition,	/* Condition variable to wait on */
	cma_t_mutex	*mutex)		/* Mutex to unlock during wait */
#else	/* no prototypes */
	(condition, mutex)
	cma_t_cond	*condition;	/* Condition variable to wait on */
	cma_t_mutex	*mutex;		/* Mutex to unlock during wait */
#endif	/* prototype */
    {
    cma__int_cond_wait (condition, mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return a cma_t_date_time for some specified interval in the future (to
 *	be used as an expiration time).
 *
 *  FORMAL PARAMETERS:
 *
 *	interval	The number of seconds (float) in the future.
 *
 *	expiration	Address of cma_t_date_time structure for return.
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
cma_time_get_expiration
#ifdef _CMA_PROTO_
	(
	cma_t_date_time	*expiration,
	cma_t_interval	interval)
#else	/* no prototypes */
	(expiration, interval)
	cma_t_date_time	*expiration;
	cma_t_interval	interval;
#endif	/* prototype */
    {
    cma_t_date_time	delta, now;


    cma__get_time (&now);
    cma__interval_to_time (interval, &delta);
    cma__add_time (expiration, &now, &delta);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free a condition object.
 *
 *  FORMAL PARAMETERS:
 *
 *	old_cv	Condition object to free
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
cma__free_cv
#ifdef _CMA_PROTO_
	(
	cma__t_int_cv	*old_cv)	/* Cv object to free */
#else	/* no prototypes */
	(old_cv)
	cma__t_int_cv	*old_cv;	/* Cv object to free */
#endif	/* prototype */
    {
    /*
     * Assert that the condition being destroyed doesn't have waiters, just
     * to be safe...
     */
    cma__assert_warn (
	    !cma__int_cv_waiting (old_cv),
	    "cma__free_cv called for condition with waiters.");

    cma__trace ((
	    cma__c_trc_obj,
	    "(free_cv) freeing condition %d",
	    old_cv->header.sequence));

    old_cv->header.sequence = 0;
    cma__int_lock (cma__g_known_cvs.mutex);
    cma__queue_remove (&old_cv->header.queue, old_cv, cma__t_int_cv);
    cma__int_unlock (cma__g_known_cvs.mutex);
    cma__free_mem ((cma_t_address)old_cv);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Allocate a condition object.
 *
 *  FORMAL PARAMETERS:
 *
 *	attrib		Attributes object to use
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
 *	Address of new condition object
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_cv *
cma__get_cv
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*attrib)	/* Attributes object to use */
#else	/* no prototypes */
	(attrib)
	cma__t_int_attr	*attrib;	/* Attributes object to use */
#endif	/* prototype */
    {
    cma__t_int_cv	*new_cv;	/* Pointer to new cv */


    cma__trace ((
	    cma__c_trc_obj,
	    "(get_cv) creating condition, attr %d",
	    attrib->header.sequence));

    new_cv = cma__alloc_object (cma__t_int_cv);

    if ((cma_t_address)new_cv == cma_c_null_ptr) {
	return (cma__t_int_cv *)cma_c_null_ptr;
	}

    new_cv->header.type = cma__c_obj_cv;
    cma__queue_zero (&new_cv->header.queue);
    cma__obj_clear_name (new_cv);
    cma__queue_init (&new_cv->queue);
    cma__tac_set (&new_cv->event);	/* no waiters yet */
    cma__tac_set (&new_cv->nopend);	/* no pending wakes yet */
    cma__tac_set (&new_cv->defer_sig);	/* no deferred signals yet */
    cma__tac_set (&new_cv->defer_bro);	/* no deferred broadcasts yet */
    cma__tac_clear (&new_cv->spindle);  /* initially unlocked */
#ifndef NDEBUG
    new_cv->defsig_cnt = 0;		/* Clear debug counters */
    new_cv->defbro_cnt = 0;
    new_cv->pend_cnt = 0;
#endif
    cma__int_lock (cma__g_known_cvs.mutex);
    new_cv->header.sequence = cma__g_known_cvs.sequence++;
    cma__queue_insert (&new_cv->header.queue, &cma__g_known_cvs.queue);
    cma__int_unlock (cma__g_known_cvs.mutex);

    return new_cv;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize CMA_CONDITION.C local data
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
 *	initialize static data
 */
extern void
cma__init_cv
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__g_known_cvs.sequence	= 1;
    cma__g_known_cvs.mutex	= cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_known_cvs.mutex, "known cond list");
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wake one or more threads waiting on a condition variable
 *
 *  FORMAL PARAMETERS:
 *
 *	cv		Pointer to condition
 *
 *	all		flag: true for "wake all", false for "wake one"
 *
 *	trylock		flag: true to try to get CV lock, but continue if
 *			unable. false to wait for lock (ignored on
 *			uniprocessor -- always uses "true")
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
cma__int_cond_wake
#ifdef _CMA_PROTO_
	(
	cma__t_int_cv		*cv,
	cma_t_boolean		all,
	cma_t_boolean		trylock)
#else	/* no prototypes */
	(cv, all, trylock)
	cma__t_int_cv		*cv;
	cma_t_boolean		all;
	cma_t_boolean		trylock;
#endif	/* prototype */
    {
    cma_t_boolean	waslocked;


#if _CMA_THREAD_IS_VP_
    if (trylock)
	waslocked = cma__cvtrylock (cv);
    else {
	cma__cvlock (cv);
	waslocked = cma_c_false;	/* previous lock state is UNLOCKED */
	}
#else
    waslocked = cma__cvtrylock (cv);
#endif

    if (!waslocked) {
	cma__cv_wakeup (cv, all);
	cma__cvunlockwake (cv);
	cma__cvundefer (cv);
	}
    else {

	if (all)
	    cma__unset (&cv->defer_bro);

	cma__unset (&cv->defer_sig);
#if !_CMA_THREAD_IS_VP_
	cma__unset (&cma__g_mgr_wake);
	cma__unset (&cma__g_cv_defer);
#endif
	}

    }

#if !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Signal a condition variable from an interrupt routine, willing to
 *	be preempted (i.e., the signal may be immediate).  (Internal version)
 *
 *	ASSUMPTIONS:
 *	    1) This routine is only called by an AST or Signal.
 *
 *  FORMAL PARAMETERS:
 *
 *	condition		Condition variable object to signal
#if _CMA_OS_ == _CMA__UNIX
 *	scp			address of signal context structure
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
 *	Note:  this routine may be preempted by another thread, so the 
 *		guarantees of atomicity or non-interruptibility of software
 *		interrupts ends when this routine is called.  And, when this
 *		routine returns, the process may not be "at interrupt level"
 *		any more!
 */
extern void
cma__int_signal_preempt_int
#ifdef _CMA_PROTO_
	(
	cma__t_int_cv	*cond
# if _CMA_OS_ == _CMA__UNIX
	,cma_t_address	scp		/* Address of signal ctx structure */
# endif
	)
#else	/* no prototypes */
# if _CMA_OS_ == _CMA__UNIX
	(cond, scp)
	cma__t_int_cv	*cond;
	cma_t_address	scp;		/* Address of signal ctx structure */
# else
	(cond)
	cma__t_int_cv	*cond;
# endif
#endif	/* prototype */
    {
    cma__t_int_tcb *cur_tcb = cma__get_self_tcb();


# if _CMA_OS_ == _CMA__UNIX
    cur_tcb->async_ctx.interrupt_ctx =
	(cma_t_address)&((struct sigcontext *)scp)->sc_mask;
# endif
    cur_tcb->async_ctx.valid = cma_c_true;
    cma__int_cond_wake (cond, cma_c_false, cma_c_true);
    cur_tcb->async_ctx.valid = cma_c_false;
    }	
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait on a condition variable with timeout (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	cv		Pointer to condition variable on which to wait.
 *
 *	mutex		Pointer to mutex to release during wait.
 *
 *	timeout		Absolute time at which wait should give up.
 * 
 *	self		TCB pointer
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
extern cma_t_status
cma__int_timed_wait
#ifdef _CMA_PROTO_
	(
	cma__t_int_cv		*cv,
	cma__t_int_mutex	*mutex,
	cma_t_date_time		*timeout,
	cma__t_int_tcb		*self)
#else	/* no prototypes */
	(cv, mutex, timeout, self)
	cma__t_int_cv		*cv;
	cma__t_int_mutex	*mutex;
	cma_t_date_time		*timeout;
	cma__t_int_tcb		*self;
#endif	/* prototype */
    {
    cma_t_status	return_value = cma_s_normal;
    cma_t_boolean	back_out = cma_c_false;
    cma_t_boolean	value = cma_c_true;
    cma_t_address	*qtmp;
#if _CMA_THREAD_IS_VP_
    cma_t_date_time	delta, current;
    cma_t_integer	delta_ms;
#endif


    if (self->alert_pending || !self->alert.g_enable) {
	cma__cv_post_event (cv);	/* We're probably going to wait */
	cma__int_unlock (mutex);
	cma__cvlock (cv);
	self->wait_cv = cv;

	/*
	 * Now, we've got the CV locked, and we've declared that we're
	 * waiting. If someone wants to send us an alert, we are now atomic.
	 * So make a final check whether there's an alert pending. If so,
	 * forget about the wait. Reset the pending state, so that
	 * attempt_delivery will see it.
	 */
	if (!cma__kernel_set (&self->alert_pending)) {

	    if (self->alert.g_enable)
		back_out = cma_c_true;

	    cma__kernel_unset (&self->alert_pending);
	    }

	if (!back_out) {
#if _CMA_THREAD_IS_VP_
	    cma__get_time (&current);
	    cma__subtract_time (&delta, timeout, &current);
# if _CMA_KTHREADS_ == _CMA__MACH
	    delta_ms = ((delta.tv_sec * 1000) + ((delta.tv_usec + 999) / 1000));
# else
	    Someone added non-Mach kernel threads without coding this!
# endif

	    /*
	     * Wait for timeout or a wakeup, whichever comes first. Attempt
	     * to insert a timer entry.  If it succeeds (the specified time
	     * hasn't passed), then block the thread until either a wakeup
	     * occurs or the timer expires.
	     * 
	     * If the insertion fails (the timeout has already expired), mark
	     * the wait status as "timeout" for the common code below. Also,
	     * initialize the element's queue link as a header, so that the
	     * common code can arbitrarily remove it without worrying that it
	     * wasn't inserted.
	     */
	    if (delta_ms <= 0) {
		value = cma_c_false;	/* Instant timeout */
		cma__queue_init (&self->header.queue);	/* Prepare for remove */
		}
	    else {
		/*
		 * In the kernel thread version, release the condition lock
		 * before we block; it doesn't need to be locked, and it
		 * would prevent some other thread from accessing the
		 * condition to signal it. Note that the "window" here
		 * doesn't matter: we're already on the block queue, so the
		 * signaller will see us -- it ends up sending a message to
		 * the waiter's port, and that will stick around even if we
		 * haven't made it to the read yet.
		 */
		cma__prepare_block (self);	/* prepare to block */
		cma__insert_by_sched (self, &cv->queue);
		cma__cvunlock (cv);

		if (!cma__cv_ifpend (cv))
		    value = cma__dispatch (self, cma_c_true, delta_ms);
# ifndef NDEBUG
		else
		    cv->pend_cnt++;
# endif

		cma__cvlock (cv);
		}

	    return_value = (value ? cma_s_normal : cma_s_timed_out);

#else
	    if (cma__insert_timer (self, timeout)) {
		/*
		 * In user-mode thread version, we don't release the
		 * "condition lock" while we block, since we actually use the
		 * kernel lock; and we also need it to block.
		 */
		cma__prepare_block (self);	/* prepare to block */
		cma__insert_by_sched (self, &cv->queue);

		if (!cma__cv_ifpend (cv))
		    cma__dispatch (self, cma_c_true, 0);
# ifndef NDEBUG
		else
		    cv->pend_cnt++;
# endif

		}
	    else {
		cma__undefer ();
		cma__yield_processor ();
		self->timer.event_status = cma__c_timeout;
		}

	    /*
	     * If the timer expired, then return value cma_s_timed_out
	     * otherwise, leave the default value cma_s_normal, and remove
	     * the TCB from the timer queue.
	     */
	    if (self->timer.event_status == cma__c_timeout)
		return_value = cma_s_timed_out;
	    else
		cma__queue_remove (&self->timer.queue, qtmp, cma_t_address);

#endif

	    cma__queue_remove (&self->header.queue, qtmp, cma_t_address);
	    }

	/*
	 * At this point, we'll check the waiter queue.  If there are no
	 * other waiters, we'll set the event bit.  This reduces subsequent
	 * pending wakeups.
	 */
	if (cma__queue_empty (&(cv)->queue))
	    cma__tac_set(&(cv)->event);

	self->wait_cv = (cma__t_int_cv *)cma_c_null_ptr;
	cma__cvunlock (cv);
	cma__cvundefer (cv);
	cma__int_lock (mutex);
	}

    cma__attempt_delivery (self);
    return return_value;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Wait on a condition variable (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	cv		Pointer to condition variable object on which to wait.
 *
 *	mutex		Pointer to mutex to release during wait.
 *
 *	self		TCB pointer
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
cma__int_wait
#ifdef _CMA_PROTO_
	(
	cma__t_int_cv		*cv,
	cma__t_int_mutex	*mutex,
	cma__t_int_tcb		*self)
#else	/* no prototypes */
	(cv, mutex, self)
	cma__t_int_cv		*cv;
	cma__t_int_mutex	*mutex;
	cma__t_int_tcb		*self;
#endif	/* prototype */
    {
    cma_t_boolean	back_out = cma_c_false;
    cma_t_address	*tmp;


    if (self->alert_pending || !self->alert.g_enable) {
	cma__cv_post_event (cv);	/* We're probably going to wait */
	cma__int_unlock (mutex);
	cma__cvlock (cv);
	self->wait_cv = cv;

	/*
	 * Now, we've got the CV locked, and we've declared that we're
	 * waiting. If someone wants to send us an alert, we are now atomic.
	 * So make a final check whether there's an alert pending. If so,
	 * forget about the wait. Reset the pending state, so that
	 * attempt_delivery will see it.
	 */
	if (!cma__kernel_set (&self->alert_pending)) {

	    if (self->alert.g_enable)
		back_out = cma_c_true;

	    cma__kernel_unset (&self->alert_pending);
	    }

	if (!back_out) {
	    cma__prepare_block (self);	/* prepare to block */
	    cma__insert_by_sched (self, &cv->queue);
#if _CMA_THREAD_IS_VP_
	    /*
	     * In the kernel thread version, we need to release the CV lock
	     * before blocking; on uniprocessor versions, the "CV lock" is
	     * really just kernel_critical, which needs to be locked anyway
	     * on scheduling operations.
	     *
	     * We need to check for pending wakeup *after* the unlock, to
	     * avoid a race with cond_signal_int() where the latter could
	     * find the CV locked and post a pending wakeup after the waiter
	     * checked for pending wakes.
	     */
	    cma__cvunlock (cv);

	    if (!cma__cv_ifpend (cv))
		cma__dispatch (self, cma_c_true, 0);
# ifndef NDEBUG
	    else
		cv->pend_cnt++;
# endif

	    cma__cvlock (cv);
#else
	    if (!cma__cv_ifpend (cv))
		cma__dispatch (self, cma_c_true, 0);
# ifndef NDEBUG
	    else
		cv->pend_cnt++;
# endif
#endif
	    cma__queue_remove (&self->header.queue, tmp, cma_t_address);
	    }

	self->wait_cv = (cma__t_int_cv *)cma_c_null_ptr;
	cma__cvunlock (cv);
	cma__cvundefer (cv);
	cma__int_lock (mutex);
	}

    cma__attempt_delivery (self);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Performs pre- or post- `fork() reinitialization' work.
 *
 *  FORMAL PARAMETERS:
 *
 *	flag
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
 *	initialize static data
 */
extern void
cma__reinit_cv
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;	
#endif	/* prototype */
    {
    cma__t_queue        *ptr;
    cma__t_int_cv	*cv;

    if (flag == cma__c_reinit_prefork_lock) {
	cma__int_lock(cma__g_known_cvs.mutex);
	}
    else if (flag == cma__c_reinit_postfork_unlock) {
	cma__int_unlock(cma__g_known_cvs.mutex);
	}
    else if (flag == cma__c_reinit_postfork_clear) {
	    ptr = &cma__g_known_cvs.queue;

	    while ((ptr = cma__queue_next (ptr)) != &cma__g_known_cvs.queue) {
		cv = (cma__t_int_cv *)ptr;
		cma__queue_init (&cv->queue);
		cma__tac_set (&cv->event);
		cma__tac_set (&cv->nopend);
		cma__tac_set (&cv->defer_sig);
		cma__tac_set (&cv->defer_bro);
		cma__tac_clear (&cv->spindle);
#ifndef NDEBUG
		cv->defsig_cnt = 0;
		cv->defbro_cnt = 0;
		cv->pend_cnt = 0;
#endif
		}

	    }                                                     	

    }

#if !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Search the known CV queue for any operations that were deferred, and
 *	performs them.
 *
 *	THIS IS ONLY USED IN THE "UNIPROCESSOR" VERSION.
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
extern int
cma__undefer_cv
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_queue        *ptr;
    cma__t_int_cv	*cv;


    /*
     * We don't use cma__cvlock() and cma__cvunlock() here -- this ASSUMES
     * that these operations really only set and clear the global kernel
     * lock, which is already set by the manager thread when calling us.
     *
     * This is only true when _CMA_THREAD_IS_VP_ is false (this function is
     * not compiled or called otherwise).
     */

    cma__assert_fail(
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma__cv_undefer with kernel unlocked");

# if _CMA_OS_ == _CMA__UNIX
    /*
     * We are handling the defered CV signals; clear the null thread's pending
     * wake-up since it's no longer needed.
     */
    cma__nt_clear_wakeup ();
# endif

    ptr = &cma__g_known_cvs.queue;

    while ((ptr = cma__queue_next (ptr)) != &cma__g_known_cvs.queue) {
	cv = (cma__t_int_cv *)ptr;

	while (!cma__kernel_set (&cv->defer_sig)) {
	    cma_t_boolean	flag = cma_c_false;

	    if (!cma__kernel_set (&cv->defer_bro)) {
		flag = cma_c_true;
#ifndef NDEBUG
		cv->defbro_cnt++;
#endif
		}
#ifndef NDEBUG
	    else
		cv->defsig_cnt++;
#endif
	    cma__cv_wakeup (cv, flag);
	    }

	}

    return 1;
    }
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONDITION.C */
/*  *42   29-JUL-1993 11:18:57 BUTENHOF "Fix timed CV wait pre-timeout" */
/*  *41    2-JUL-1993 14:37:34 BUTENHOF "Fix wait/signal_int race for OSF/1" */
/*  *40   15-JUN-1993 09:10:05 KEANE "Fix compile probs w/DEC C & reduce pending wakes after timeouts" */
/*  *39   27-MAY-1993 14:31:53 BUTENHOF "Remove OSF/1 performance glitch" */
/*  *38   16-APR-1993 13:02:30 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *37    1-APR-1993 14:32:36 BUTENHOF "Add names to user objects" */
/*  *36   29-MAR-1993 13:55:58 BUTENHOF "Integrate March 10 review comments" */
/*  *35    3-MAR-1993 15:21:55 KEANE "remuve unneeded mutex unlock from get_cv error path" */
/*  *34   23-FEB-1993 16:26:47 SCALES "Fix two wake-up waiting races in the null thread" */
/*  *33   22-DEC-1992 17:22:38 KEANE "Initialize spindle field in cv structure" */
/*  *32   24-NOV-1992 04:55:56 BUTENHOF "Fix call" */
/*  *31   23-NOV-1992 14:00:08 BUTENHOF "Fix kthread CVs" */
/*  *30   13-NOV-1992 14:40:23 BUTENHOF "Reset pend/defer flags in reinit" */
/*  *29   12-NOV-1992 11:52:30 BUTENHOF "Fix alert hole" */
/*  *28    5-NOV-1992 14:24:03 BUTENHOF "Fix async alert" */
/*  *27    2-NOV-1992 13:25:01 BUTENHOF "Speedyize & fix race" */
/*  *26   16-OCT-1992 12:19:27 BUTENHOF "Fix VP block" */
/*  *25    8-OCT-1992 08:15:40 BUTENHOF "Re-insert last change" */
/*  *24    7-OCT-1992 12:46:52 BUTENHOF "Fix bug in wait" */
/*  *23    5-OCT-1992 15:50:37 BUTENHOF "Remove kernel-sets inside kernel" */
/*  *22    5-OCT-1992 15:07:31 BUTENHOF "Move defer bit init" */
/*  *21   21-SEP-1992 13:31:11 BUTENHOF "Reset 'event' on cvunlock" */
/*  *20   17-SEP-1992 14:35:23 BUTENHOF "Make cv_wakeup extern" */
/*  *19   15-SEP-1992 13:49:01 BUTENHOF "Fix infinite loop in undefer_cv!" */
/*  *18   10-SEP-1992 09:46:24 BUTENHOF "Fix delay" */
/*  *17    4-SEP-1992 15:52:44 SCALES "Add cond-signal-preempt-int" */
/*  *16    4-SEP-1992 10:03:22 BUTENHOF "Restore $wake on VMS" */
/*  *15    2-SEP-1992 16:23:45 BUTENHOF "Separate semaphores from kernel lock" */
/*  *14   25-AUG-1992 11:47:46 BUTENHOF "Cut down on event bit" */
/*  *13   21-AUG-1992 13:41:19 BUTENHOF "Use spinlocks on kernel thread semaphores instead of kernel_critical" */
/*  *12    5-JUN-1992 13:34:48 BUTENHOF "Fix delete_pending attr handling" */
/*  *11   18-FEB-1992 15:27:36 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *10   10-JUN-1991 18:17:32 SCALES "Add sccs headers for Ultrix" */
/*  *9     5-JUN-1991 16:11:36 CURTIN "fork work" */
/*  *8    24-MAY-1991 16:43:34 CURTIN "Added a new reinit routine" */
/*  *7    10-MAY-1991 11:56:09 CURTIN "Added a number of new macros" */
/*  *6    12-APR-1991 23:34:50 BUTENHOF "Init atomic bits using new constants" */
/*  *5     3-APR-1991 10:51:36 BUTENHOF "Remove pending wakeup" */
/*  *4    12-FEB-1991 01:28:39 BUTENHOF "Change to alert control bits" */
/*  *3     5-FEB-1991 00:59:27 BUTENHOF "Improve inlining of internal cv operations" */
/*  *2    14-DEC-1990 00:55:10 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:42:44 BUTENHOF "Condition variables" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONDITION.C */
