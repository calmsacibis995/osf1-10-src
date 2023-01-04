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
static char	*sccsid = "@(#)$RCSfile: lock.c,v $ $Revision: 4.3.25.4 $ (DEC) $Date: 1993/11/16 23:35:02 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	File:	kern/lock.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Locking primitives implementation
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 3-June-91	Ron Widyono
 *	Fix assert_wait() race condition.  Remove lock counting code in
 *	wait locks.
 *
 * 6-April-91	Ron Widyono
 *	Lock counting code for preemption_points in wait locks.  Preemption
 *	point code do_preemption().  Preemption counters debugging.
 *	Conditionalized under RT_PREEMPT.
 *
 */

#include <cpus.h>
#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#include <sys/types.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>

#if	RT_PREEMPT
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <kern/ast.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/processor.h>
#include <machine/cpu.h>
#endif	/* RT_PREEMPT */

#if	MACH_LDEBUG
#include <kern/assert.h>

#define	LDEBUG(clause)		clause

int	check_locks = 0;	/* don't check until current_thread works */
int	check_lock_counts = 0;	
struct slock_debug slck_dbg[NCPUS];

#else	/* MACH_LDEBUG */

#define	LDEBUG(clause)

#if	RT_PREEMPT
/*
 *	Flag to indicate that current_thread() is valid and that lock counting
 *	can commence; per CPU simple lock counter.
 */
int     rt_preempt_enabled = 0; /* enable lock counting, etc. for preemption */
int     slock_count[NCPUS];     /* count spinlocks per cpu */
int	rt_preempt_fixups = 0;  /* number of fixups required */
int 	rt_slock_trace_count = 0;
extern 	int slocktrace;
extern	rt_preempt_enforce;	/* enable panic on bad slock_count */
extern	struct rt_lock_debug rt_slock_trace[];
extern	unsigned long get_caller_ra();
extern	char* panicstr;

#define RT_SLOCK_LOCKED		(1)	/* Low bit in the PC field! */
#define RT_SLOCK_UNLOCKED 	(0)

#define	slock_update(WHICH)						\
	if (!panicstr) {						\
		register int s = splhigh();				\
		register int count = rt_slock_trace_count;		\
		rt_slock_trace[count].rlb_pc =				\
			(unsigned long) get_caller_ra() + WHICH;	\
		rt_slock_trace[count].rlb_count =			\
			(unsigned long) slock_count[cpu_number()];	\
		if (++rt_slock_trace_count >= slocktrace)		\
			rt_slock_trace_count = 0;			\
		(void) splx(s);						\
	}
	
rt_inc_slock()
{
	slock_count[cpu_number()]++;
	slock_update(RT_SLOCK_LOCKED);
	return 1;
}

rt_dec_slock()
{
	slock_update(RT_SLOCK_UNLOCKED); 
#if	!RT_PREEMPT_DEBUG
	DEC_SLOCK_COUNT_PREEMPT(NULL);
#else
	DEC_SLOCK_COUNT_PREEMPT(&rt_preempt_simple);
#endif
	return 1;
}

#if	RT_PREEMPT_DEBUG
int	rt_preempt_count = 0;	/* # of preemptions */
int	rt_preempt_simple = 0;	/* # of preemptions due to simple unlock */
int	rt_preempt_sleep = 0;	/* # of preemptions due to sleep lock unlock */
int	rt_preempt_funnel = 0;	/* # of preemptions due to unfunneling */
int	rt_preempt_splx = 0;	/* # of preemptions due to lowering IPL */
int	rt_preempt_spl0 = 0;	/* # of preemptions due to lowering IPL */
int	rt_preempt_async = 0;	/* # of preemptions due to return from ISR */
int	rt_preempt_syscall = 0;	/* # of preemptions at end of syscall() */
int	rt_preempt_user = 0;	/* # of preemptions due to return to user mode */
int	rt_preempt_pp = 0;	/* # of preemption points */
int	rt_preempt_kpp = 0;	/* # of kernel ast preemption points */
int	rt_preempt_saveast = 0;
int	rt_preempt_restoreast = 0;
int	rt_preempt_skipped = 0;
int	rt_preempt_ast_set = 0;
#define	RT_PREEMPT_NCOUNTS 13
#endif	/* RT_PREEMPT_DEBUG */
#endif	/* RT_PREEMPT */

#endif	/* MACH_LDEBUG */

#if	NCPUS > 1

/*
 *	Module:		lock
 *	Function:
 *		Provide reader/writer sychronization.
 *	Implementation:
 *		Simple interlock on a bit.  Readers first interlock
 *		increment the reader count, then let go.  Writers hold
 *		the interlock (thus preventing further readers), and
 *		wait for already-accepted readers to go away.
 */

/*
 *	The simple-lock routines are the primitives out of which
 *	the lock package is built.  The implementation is left
 *	to the machine-dependent code.
 */

#ifdef	notdef
/*
 *	A sample implementation of simple locks.
 *	assumes:
 *		boolean_t test_and_set(boolean_t *)
 *			indivisibly sets the boolean to TRUE
 *			and returns its old value
 *		and that setting a boolean to FALSE is indivisible.
 */
/*
 *	simple_lock_init initializes a simple lock.  A simple lock
 *	may only be used for exclusive locks.
 */

void simple_lock_init(l)
	simple_lock_t	l;
{
	*(boolean_t *)l = FALSE;
}

void simple_lock(l)
	simple_lock_t	l;
{
	while (test_and_set((boolean_t *)l))
		continue;
}

void simple_unlock(l)
	simple_lock_t	l;
{
	*(boolean_t *)l = FALSE;
}

boolean_t simple_lock_try(l)
	simple_lock_t	l;
{
    	return (!test_and_set((boolean_t *)l));
}
#endif	/* notdef */
#endif	/* NCPUS > 1 */

#if	NCPUS > 1
int lock_wait_time = 100;
#else

	/*
	 * 	It is silly to spin on a uni-processor as if we
	 *	thought something magical would happen to the
	 *	want_write bit while we are executing.
	 */
int lock_wait_time = 0;
#endif

#if	MACH_SLOCKS && (defined(ibmrt) || (NCPUS == 1))
/* Need simple lock sanity checking code if simple locks are being
   compiled in, and either we are on RT (which doesn't have any special
   locking code of its own) or we are compiling for a uniprocessor. */

void simple_lock_init(l)
	simple_lock_t l;
{
	l->lock_data = 0;
#if	MACH_LDEBUG
	l->slthread = (char *) -1;
	l->slck_addr = (int) -1;
	l->sunlck_addr = (int) -1;
#endif
}

void simple_lock(l)
	simple_lock_t l;
{
	assert(l->lock_data == 0);

	l->lock_data = 1;
#if	MACH_LDEBUG
	l->slthread = (char *) current_thread();
	inc_slock(l);
#endif
}

void simple_unlock(l)
	simple_lock_t l;
{
	assert(l->lock_data != 0);

	l->lock_data = 0;
#if	MACH_LDEBUG
	l->slthread = (char *) -1;
	dec_slock(l);
#endif
}

boolean_t simple_lock_try(l)
	simple_lock_t l;
{
	assert(l->lock_data == 0);

	l->lock_data = 1;
#if	MACH_LDEBUG
	l->slthread = (char *) current_thread();
	inc_slock(l);
#endif

	return TRUE;
}
#endif	/* MACH_SLOCKS && (defined(ibmrt) || (NCPUS == 1)) */

/*
 *	Routine:	lock_init
 *	Function:
 *		Initialize a lock; required before use.
 *		Note that clients declare the "struct lock"
 *		variables and then initialize them, rather
 *		than getting a new one from this module.
 */
void lock_init(l, can_sleep)
	lock_t		l;
	boolean_t	can_sleep;
{
	bzero(l, sizeof(lock_data_t));
	simple_lock_init(&l->interlock);
	l->want_write = FALSE;
	l->want_upgrade = FALSE;
	l->read_count = 0;
	l->can_sleep = can_sleep;
	l->thread = (char *)-1;		/* XXX */
	l->recursion_depth = 0;
#ifdef	ns32000
	l->lock_type = 0;
#endif
	LDEBUG(l->lck_addr = 0xffffffff);	/* Lock is not yet held */
#if	LOCK_STATS
	l->lock_tries = l->lock_fails = l->lock_sleeps = 0;
	l->lock_wait_min = -1;
	l->lock_wait_max = l->lock_wait_sum = 0;
	l->lock_max_read = 0;
	l->lock_nreads = 0;
#endif
}

void lock_init2(l, can_sleep, ltype)
	lock_t		l;
	boolean_t	can_sleep;
	int		ltype;
{
	lock_init(l, can_sleep);
#ifdef	ns32000
	l->lock_type = ltype;
#endif
}

void lock_sleepable(l, can_sleep)
	lock_t		l;
	boolean_t	can_sleep;
{
	simple_lock(&l->interlock);
	l->can_sleep = can_sleep;
	simple_unlock(&l->interlock);
}


/*
 *	Sleep locks.  These use the same data structure and algorithm
 *	as the spin locks, but the process sleeps while it is waiting
 *	for the lock.  These work on uniprocessor systems.
 */

#if	MACH_LDEBUG || MACH_LTRACKS
#define	LTRACK(clause)	clause

#if	defined(multimax)
#define	LTRACK_DONE(l)	((l)->lthread=(char*)((int)((l)->lthread)|0x80000000))
#else
#define	LTRACK_DONE(l)	((l)->lthread=(char*)-1)
#endif
#else	/* MACH_LDEBUG || MACH_LTRACKS */
#define	LTRACK(clause)
#define	LTRACK_DONE(l)
#endif

#if	(MACH_LDEBUG || MACH_LTRACKS) && defined(multimax)
#define	MMAX_LTRACK(clause)	clause
#else
#define	MMAX_LTRACK(clause)
#endif

#if	LOCK_STATS && defined(multimax)
#define	LOCK_STATS_ACTIONS	1
#else
#define	LOCK_STATS_ACTIONS	0
#endif


#if	LOCK_STATS_ACTIONS

#define	LSTATS(clause)		clause
#define	LSTATS_TIME(t)		((t) = FRcounter)

#define	LSTATS_TIME_SUM(t,s0,s1)					\
MACRO_BEGIN								\
	unsigned int	delta;						\
	if (((int)(delta = (s1) - (s0))) < 0)				\
		delta = 0 - delta;					\
	(t) += delta;							\
MACRO_END

#define	LSTATS_ACCUMULATE(l,total_time)					\
MACRO_BEGIN								\
	(l)->lock_wait_sum += (total_time);				\
	if ((total_time) > 0 && (total_time) < (l)->lock_wait_min)	\
		(l)->lock_wait_min = (int) (total_time);		\
	else if ((total_time) > (l)->lock_wait_max)			\
		(l)->lock_wait_max = (int) (total_time);		\
MACRO_END

#else	/* LOCK_STATS_ACTIONS */

#define	LSTATS(clause)
#define	LSTATS_TIME(t)
#define	LSTATS_TIME_SUM(t,s0,s1)
#define	LSTATS_ACCUMULATE(l,total_time)

#endif	/* LOCK_STATS_ACTIONS */

void lock_write(l)
	register lock_t	l;
{
	register int	i;
#if	LOCK_STATS_ACTIONS
	register unsigned int	start_time, stop_time, total_time;
#endif

	/*LASSERT(l>=kernel_map->vm_map_min&&l<=kernel_map->vm_map_max);*/
	simple_lock(&l->interlock);
	LSTATS(l->lock_tries++);

	if (((thread_t)l->thread) == current_thread()) {
		/*
		 *	Recursive lock.
		 */
		l->recursion_depth++;
		simple_unlock(&l->interlock);
		LDEBUG(inc_lock(l, current_thread()));
		return;
	}

	/*
	 * if read_count is non-zero, lthread is not reset in lock_done.
	 */
	LASSERT(l->read_count != 0 || !LOCK_OWNER(l));
	LSTATS(total_time = 0);

	/*
	 *	Try to acquire the want_write bit.
	 */
	while (l->want_write) {
		LSTATS(l->lock_fails++);
		if ((i = lock_wait_time) > 0) {
			simple_unlock(&l->interlock);
			while (--i > 0 && l->want_write)
				continue;
			simple_lock(&l->interlock);
		}

		if (l->can_sleep && l->want_write) {
			l->waiting = TRUE;
			LSTATS(l->lock_sleeps++);
			LSTATS_TIME(start_time);
			thread_sleep((vm_offset_t)l,
				simple_lock_addr(l->interlock), FALSE);
			LSTATS_TIME(stop_time);
			LSTATS_TIME_SUM(total_time, start_time, stop_time);
			simple_lock(&l->interlock);
		}
	}
	l->want_write = TRUE;

	/* Wait for readers (and upgrades) to finish */

	while ((l->read_count != 0) || l->want_upgrade) {
		LSTATS(l->lock_fails++);
		if ((i = lock_wait_time) > 0) {
			simple_unlock(&l->interlock);
			while (--i > 0 && (l->read_count != 0 ||
					l->want_upgrade))
				continue;
			simple_lock(&l->interlock);
		}

		if (l->can_sleep && (l->read_count != 0 || l->want_upgrade)) {
			l->waiting = TRUE;
			LSTATS(l->lock_sleeps++);
			LSTATS_TIME(start_time);
			thread_sleep((vm_offset_t)l,
				simple_lock_addr(l->interlock), FALSE);
			LSTATS_TIME(stop_time);
			LSTATS_TIME_SUM(total_time, start_time, stop_time);
			simple_lock(&l->interlock);
		}
	}

	LSTATS_ACCUMULATE(l,total_time);
	LASSERT(l->lck_addr & 0x80000000);
	LTRACK(l->lthread = (char *) current_thread());
	LTRACK(l->lck_addr = 1);	/* XXX */
	MMAX_LTRACK(l->lck_addr = getfrompc());

	simple_unlock(&l->interlock);
	LDEBUG(inc_lock(l, current_thread()));
}


void lock_done(l)
	register lock_t	l;
{

	simple_lock(&l->interlock);

	if (l->read_count != 0) {
#if	LOCK_STATS_ACTIONS
		if (l->read_count > l->lock_max_read)
			l->lock_max_read = l->read_count;
#endif
		l->read_count--;
		LTRACK((l->read_count == 0 && l->thread == (char *)-1)
			? LTRACK_DONE(l) : 0);
	} else if (l->recursion_depth != 0)
		l->recursion_depth--;
	else if (l->want_upgrade) {
	 	l->want_upgrade = FALSE;
		LTRACK_DONE(l);
	} else {
		LASSERT(l->want_write == TRUE);
		LASSERT(LOCK_OWNER(l));
		LTRACK_DONE(l);
	 	l->want_write = FALSE;
	}

	if (l->waiting) {
		l->waiting = FALSE;
		thread_wakeup((vm_offset_t)l);
	}
	LTRACK(l->lck_addr |= 0x80000000);
	LTRACK(l->unlck_addr = 1);	/* XXX */
#if	0
	/*
	 * Gcc puts an enter [] for all functions, so lock_done
	 * can now use the fp to determine where it came from.
	 * (In any case, the stack has changed and getpc_fromld is
	 * incorrect.)
	 */
	MMAX_LTRACK(l->unlck_addr = getpc_fromld());
#else
	MMAX_LTRACK(l->unlck_addr = getfrompc());
#endif
	simple_unlock(&l->interlock);
	LDEBUG(dec_lock(l, current_thread()));
}

void lock_read(l)
	register lock_t	l;
{
	register int	i;
#if	LOCK_STATS_ACTIONS
	register unsigned int	start_time, stop_time, total_time;
#endif

	simple_lock(&l->interlock);
	LSTATS(l->lock_nreads++);

	if (((thread_t)l->thread) == current_thread()) {
		/*
		 *	Recursive lock.
		 */
		l->read_count++;
		simple_unlock(&l->interlock);
		LDEBUG(inc_lock(l, current_thread()));
		return;
	}

	LSTATS(total_time = 0);
	while (l->want_write || l->want_upgrade) {
		LSTATS(l->lock_fails++);
		if ((i = lock_wait_time) > 0) {
			simple_unlock(&l->interlock);
			while (--i > 0 && (l->want_write || l->want_upgrade))
				continue;
			simple_lock(&l->interlock);
		}

		if (l->can_sleep && (l->want_write || l->want_upgrade)) {
			l->waiting = TRUE;
			LSTATS(l->lock_sleeps++);
			LSTATS_TIME(start_time);
			thread_sleep((vm_offset_t)l,
				simple_lock_addr(l->interlock), FALSE);
			LSTATS_TIME(stop_time);
			LSTATS_TIME_SUM(total_time, start_time, stop_time);
			simple_lock(&l->interlock);
		}
	}

	LSTATS_ACCUMULATE(l,total_time);
	LTRACK(l->lthread = (char *) current_thread());
	LTRACK(l->lck_addr = 1);	/* XXX */
	MMAX_LTRACK(l->lck_addr = getfrompc());

	l->read_count++;
	simple_unlock(&l->interlock);
	LDEBUG(inc_lock(l, current_thread()));
}

/*
 *	Routine:	lock_read_to_write
 *	Function:
 *		Improves a read-only lock to one with
 *		write permission.  If another reader has
 *		already requested an upgrade to a write lock,
 *		no lock is held upon return.
 *
 *		Returns TRUE if the upgrade *failed*.
 */
boolean_t lock_read_to_write(l)
	register lock_t	l;
{
	register int	i;
#if	LOCK_STATS_ACTIONS
	register unsigned int	start_time, stop_time, total_time;
#endif

	simple_lock(&l->interlock);

	if (l->read_count == 0)	panic("lock upgrade w/o read lock");
	l->read_count--;
	LSTATS(l->lock_tries++);

	if (((thread_t)l->thread) == current_thread()) {
		/*
		 *	Recursive lock.
		 */
		l->recursion_depth++;
		simple_unlock(&l->interlock);
		return(FALSE);
	}

	LSTATS(total_time = 0);
	if (l->want_upgrade) {
		/*
		 *	Someone else has requested upgrade.
		 *	Since we've released a read lock, wake
		 *	him up.
		 */
		if (l->waiting) {
			l->waiting = FALSE;
			thread_wakeup((vm_offset_t)l);
		}
		LSTATS(l->lock_fails++);
		LTRACK(l->lthread = (char *) -1);
		simple_unlock(&l->interlock);
		LDEBUG(dec_lock(l, current_thread()));
		return (TRUE);
	}

	l->want_upgrade = TRUE;

	while (l->read_count != 0) {
		LSTATS(l->lock_fails++);
		if ((i = lock_wait_time) > 0) {
			simple_unlock(&l->interlock);
			while (--i > 0 && l->read_count != 0)
				continue;
			simple_lock(&l->interlock);
		}

		if (l->can_sleep && l->read_count != 0) {
			l->waiting = TRUE;
			LSTATS(l->lock_sleeps++);
			LSTATS_TIME(start_time);
			thread_sleep((vm_offset_t)l,
				simple_lock_addr(l->interlock), FALSE);
			LSTATS_TIME(stop_time);
			LSTATS_TIME_SUM(total_time, start_time, stop_time);
			simple_lock(&l->interlock);
		}
	}

	LSTATS_ACCUMULATE(l,total_time);
	LTRACK(l->lthread = (char *) current_thread());

	simple_unlock(&l->interlock);
	return (FALSE);
}

void lock_write_to_read(l)
	register lock_t	l;
{
	simple_lock(&l->interlock);

	l->read_count++;
	LSTATS(l->lock_nreads++);
	if (l->recursion_depth != 0)
		l->recursion_depth--;
	else
	if (l->want_upgrade)
		l->want_upgrade = FALSE;
	else
	 	l->want_write = FALSE;

	if (l->waiting) {
		l->waiting = FALSE;
		thread_wakeup((vm_offset_t)l);
	}

	simple_unlock(&l->interlock);
}


/*
 *	Routine:	lock_try_write
 *	Function:
 *		Tries to get a write lock.
 *
 *		Returns FALSE if the lock is not held on return.
 */

boolean_t lock_try_write(l)
	register lock_t	l;
{

	simple_lock(&l->interlock);

	LSTATS(l->lock_tries++);

	if (((thread_t)l->thread) == current_thread()) {
		/*
		 *	Recursive lock
		 */
		l->recursion_depth++;
		simple_unlock(&l->interlock);
		LDEBUG(inc_lock(l, current_thread()));
		return(TRUE);
	}

	if (l->want_write || l->want_upgrade || l->read_count) {
		/*
		 *	Can't get lock.
		 */
		LSTATS(l->lock_fails++);
		simple_unlock(&l->interlock);
		return(FALSE);
	}

	/*
	 *	Have lock.
	 */

	l->want_write = TRUE;
	LTRACK(l->lthread = (char *) current_thread());
	LTRACK(l->lck_addr = 1);	/* XXX */
	MMAX_LTRACK(l->lck_addr = getfrompc());
	simple_unlock(&l->interlock);
	LDEBUG(inc_lock(l, current_thread()));
	return(TRUE);
}

/*
 *	Routine:	lock_try_read
 *	Function:
 *		Tries to get a read lock.
 *
 *		Returns FALSE if the lock is not held on return.
 */

boolean_t lock_try_read(l)
	register lock_t	l;
{
	simple_lock(&l->interlock);

	if (((thread_t)l->thread) == current_thread()) {
		/*
		 *	Recursive lock
		 */
		l->read_count++;
		simple_unlock(&l->interlock);
		LDEBUG(inc_lock(l, current_thread()));
		return(TRUE);
	}

	if (l->want_write || l->want_upgrade) {
		simple_unlock(&l->interlock);
		return(FALSE);
	}

	l->read_count++;
	simple_unlock(&l->interlock);
	LDEBUG(inc_lock(l, current_thread()));
	return(TRUE);
}

/*
 *	Routine:	lock_try_read_to_write
 *	Function:
 *		Improves a read-only lock to one with
 *		write permission.  If another reader has
 *		already requested an upgrade to a write lock,
 *		the read lock is still held upon return.
 *
 *		Returns FALSE if the upgrade *failed*.
 */
boolean_t lock_try_read_to_write(l)
	register lock_t	l;
{

	simple_lock(&l->interlock);

	if (((thread_t)l->thread) == current_thread()) {
		/*
		 *	Recursive lock
		 */
		if (l->read_count == 0)	panic("lock upgrade w/o read lock");
		l->read_count--;
		l->recursion_depth++;
		simple_unlock(&l->interlock);
		return(TRUE);
	}

	if (l->want_upgrade) {
		simple_unlock(&l->interlock);
		LDEBUG(dec_lock(l, current_thread()));
		return(FALSE);
	}
	l->want_upgrade = TRUE;
	if (l->read_count == 0)	panic("lock upgrade w/o read lock");
	l->read_count--;

	while (l->read_count != 0) {
		l->waiting = TRUE;
		thread_sleep((vm_offset_t)l,
				simple_lock_addr(l->interlock), FALSE);
		simple_lock(&l->interlock);
	}

	LTRACK(l->lthread = (char *) current_thread());
	LTRACK(l->lck_addr = 1);	/* XXX */
	MMAX_LTRACK(l->lck_addr = getfrompc());
	simple_unlock(&l->interlock);
	return(TRUE);
}

/*
 *	Allow a process that has a lock for write to acquire it
 *	recursively (for read, write, or update).
 */
void lock_set_recursive(l)
	lock_t		l;
{
	simple_lock(&l->interlock);
	if (!l->want_write) {
		panic("lock_set_recursive: don't have write lock");
	}
	l->thread = (char *) current_thread();
	simple_unlock(&l->interlock);
	LDEBUG(inc_lock(l, current_thread()));
}

/*
 *	Prevent a lock from being re-acquired.
 */
void lock_clear_recursive(l)
	lock_t		l;
{
	simple_lock(&l->interlock);
	if (((thread_t) l->thread) != current_thread()) {
		panic("lock_clear_recursive: wrong thread");
	}
	if (l->recursion_depth == 0)
		l->thread = (char *)-1;		/* XXX */
	simple_unlock(&l->interlock);
	LDEBUG(dec_lock(l, current_thread()));
}


/*
 * Test existence of the lock
 */

int
lock_islocked(l)
lock_t	l;
{
	int	ret;

	LASSERT(l != 0);

	if(simple_lock_try(&l->interlock)) {
		ret = l->want_write || l->read_count;
		simple_unlock(&l->interlock);
		return(ret);
	} else
		return(1);
}


#if	MACH_LDEBUG

/*
 * Probably this code should go away.  XXX
 */

decl_simple_lock_data(extern, printf_lock)

/*
 * Panic on unlocking an unlocked simple lock.  There's an exception
 * for the printf lock, which we force to be unlocked at panic time.
 */
slpanic(lp, pc)
struct	slock	*lp;
int	pc;
{
	printf("Unlocking unlocked simple lock @ 0x%x from 0x%x\n", lp, pc);
	printf("lckaddr = 0x%x, unlockaddr = 0x%x\n",
	       lp->slck_addr, lp->sunlck_addr);
	if (lp == simple_lock_addr(printf_lock))
		return;
	panic("simple unlock");
}

/*
 * Record current thread and lock addresses.
 */
slhack(lp, pc)
struct	slock	*lp;
int	pc;
{
	lp->slthread = (char *) current_thread();
	lp->slck_addr = pc;
	lp->sunlck_addr |= 0x80000000;
	inc_slock(lp);
}

struct slock *
sunhack(lp, pc)
struct	slock	*lp;
int	pc;
{
	if(lp->slck_addr & 0x80000000)
		slpanic(lp, pc);
	if((int)lp->slthread & 0x80000000)
		slpanic(lp, pc);

	dec_slock(lp);
	if(lp->slthread != (char *) current_thread()) {
		printf("sunhack: lp = 0x%x, thread = 0x%x\n",
			lp, lp->slthread);
		if (lp == simple_lock_addr(printf_lock))
			return;
		panic("I didn't lock this lock");
	}

	lp->sunlck_addr = pc;
	lp->slck_addr |= 0x80000000;
	lp->slthread = (char *)((int)lp->slthread | 0x80000000);
	/*
	 * We return LP here so the in-line assembly
	 *  version doesn't have to save and restore
	 *  the lock address.
	 */
	return(lp);
}

dec_slock(l)
struct slock *l;
{
	int i, j, found;
	int cpu = cpu_number();

	if (!check_locks || !check_lock_counts ||
	    l == simple_lock_addr(printf_lock))
		return;
	slck_dbg[cpu].count--;
	j = 0;
	found = MAX_LOCK;
	for (i = 0; i < MAX_LOCK; i++) {
		if (slck_dbg[cpu].addr[i] == (int)l) {
			slck_dbg[cpu].addr[i] = 0;
			found = i;
		}
		if (slck_dbg[cpu].addr[i] != 0)
			j++;
	}
	if (found == MAX_LOCK)
		panic("dec_slock");
	ASSERT(j == slck_dbg[cpu].count);

}

inc_slock(l)
struct slock *l;
{
	int cpu = cpu_number();
	int i, j, found;

	if (!check_locks  || !check_lock_counts ||
	    l == simple_lock_addr(printf_lock))
		return;
	if (slck_dbg[cpu].count++ >= MAX_LOCK)
		panic("slock debug");
	j = 0;
	found = 0;
	for (i = 0; i < MAX_LOCK; i++) {
		if (!found && slck_dbg[cpu].addr[i] == 0) {
			slck_dbg[cpu].addr[i] = (int)l;
			found = 1;
		} 
		if (slck_dbg[cpu].addr[i] != 0)
			j++;
	}
	ASSERT(j == slck_dbg[cpu].count);
}

inc_lock(l, th)
lock_t l;
thread_t th;
{
	int i, j, found;

	if (!check_locks || !check_lock_counts)
		return;
	if (th->lock_count++ >= MAX_LOCK)
		panic("lock debug");
	j = 0;
	found = 0;
	for (i = 0; i < MAX_LOCK; i++) {
		if (!found && th->lock_addr[i] == 0) {
			th->lock_addr[i] = (int)l;
			found = 1;
		} 
		if (th->lock_addr[i] != 0)
			j++;
	}
	ASSERT(j == th->lock_count);
}

dec_lock(l, th)
lock_t l;
thread_t th;
{
	int i, j, found;

	if (!check_locks || !check_lock_counts)
		return;
	th->lock_count--;
	j = 0;
	found = MAX_LOCK;
	for (i = 0; i < MAX_LOCK; i++) {
		if (found == MAX_LOCK && th->lock_addr[i] == (int)l) {
			th->lock_addr[i] = 0;
			found = i;
		}
		if (th->lock_addr[i] != 0)
			j++;
	}
	if (found == MAX_LOCK) {
		printf("Dec_lock: Looking for lock 0x%x\n",l);
		for (i = 0; i < MAX_LOCK; i++) 
			printf("th->lock_addr[%d] = 0x%x\n", i, th->lock_addr[i]);
		panic("dec_lock");
	}
	ASSERT(j == th->lock_count);

}
#endif	/* MACH_LDEBUG */

#if	RT_PREEMPT
/*
 *	Routine:	preemption_fixup
 *	Function:
 *		Correct the slock_count and optionally panic the system.
 *		For aid in debugging bad locks. Actual code in lock.h.
 */
void	preemption_fixup()
{
	PREEMPTION_FIXUP();
}

/*
 *	Routine:	do_preemption
 *	Function:
 *		Implements a preemption point after a possible preemption 
 *		request (AST) has been recognized.  It is assumed that
 *		preemption conditions have been met, either through explicit
 *		tests or because the calling code is implicitly "clean."
 *		
 */
#if	!RT_PREEMPT_DEBUG
void do_preemption()
#else
void do_preemption(preemption_counter)
int		*preemption_counter;
#endif
{
	    int	s;
	    boolean_t save_ast;
	    register thread_t	thread = current_thread();

	    /* do not preempt if unix_master, or open via /proc */
	    if (thread->unix_lock != -1 ||
	        thread->task->procfs.pr_qflags & PRFS_OPEN)
		return;

	    s = splsched();						
	    if (csw_needed(thread, current_processor())) {	
		/*						
		 * Check other reasons for AST
		 */						
		if (CHECK_SIGNALS(u.u_procp, thread, thread->u_address.uthread)) {
		    save_ast = TRUE;		
#if	RT_PREEMPT_DEBUG
		    rt_preempt_saveast++;				
#endif
		}
		else
		    save_ast = FALSE;

		if (!(thread->state & (TH_WAIT | TH_SUSP | TH_IDLE))) {
#if	RT_PREEMPT_DEBUG
		    rt_preempt_count++;		/* global preemption counter */
		    (*preemption_counter)++;	/* preemption type counter */
#endif
		    thread_block();
		}
#if	RT_PREEMPT_DEBUG
		else
		    rt_preempt_skipped++;
#endif
		if (save_ast) {			
		    aston();					
#if	RT_PREEMPT_DEBUG
		    rt_preempt_restoreast++;				
#endif
		}
	    }							
	    splx_nopreempt(s);						
}

#if	RT_PREEMPT_DEBUG
/*
 * getpreemptcounts()
 */

#include <kern/parallel.h>

getpreemptcounts(p, args, retval)
     struct proc *p;
     void *args;
     long *retval;
{
  register struct args {
	void	*buff;
  } *uap = (struct args *) args;

  int	counts[RT_PREEMPT_NCOUNTS];
  int	s = splsched();

/*
 *	Arguments to getpreemptcounts:
 *		SYS_getpreemptcounts: syscall index for indirect call
 *		buff:		pointer to buffer
 */
	printf("Total Preemptions = %d\n", rt_preempt_count);
	printf("Preemptions by type:\n");
	printf("  Simple Lock   = %d\n", rt_preempt_simple);
	printf("  Wait Lock     = %d\n", rt_preempt_sleep);
	printf("  Funneled Code = %d\n", rt_preempt_funnel);
	printf("  Raised SPL    = %d\n", rt_preempt_splx);
	printf("  IPL 0         = %d\n", rt_preempt_spl0);
	printf("  Asynchronous  = %d\n", rt_preempt_async);
	printf("  Syscall       = %d\n", rt_preempt_syscall);
	printf("  User mode     = %d\n", rt_preempt_user);
	printf("Total Preemption Points = %d\n", rt_preempt_pp);
	printf("kernel mode AST preemption points = %d\n",rt_preempt_kpp);
	printf("ast_needed Saved    = %d\n", rt_preempt_saveast);
	printf("ast_needed Restored = %d\n", rt_preempt_restoreast);
	printf("AST set on Syscall Preemption = %d\n", rt_preempt_ast_set);
	counts[0] = rt_preempt_count; 	/* # of preemptions */
	counts[1] = rt_preempt_simple; 	/* # of preemptions due to simple unlock */
	counts[2] = rt_preempt_sleep; 	/* # of preemptions due to sleep lock unlock */
	counts[3] = rt_preempt_funnel; 	/* # of preemptions due to unfunneling */
	counts[4] = rt_preempt_splx; 	/* # of preemptions due to lowering IPL */
	counts[5] = rt_preempt_async; 	/* # of preemptions due to return from ISR */
	counts[6] = rt_preempt_pp;	/* # of preemptions due to return from ISR */
	counts[7] = rt_preempt_syscall;	/* # of preemptions at end of syscall() */
	counts[8] = rt_preempt_user;	/* # of preemptions due to return to user mode */
	counts[9] = rt_preempt_saveast;
	counts[10] = rt_preempt_restoreast;
	counts[11] = rt_preempt_ast_set;
	counts[12] = rt_preempt_spl0;

	copyout((caddr_t)counts, (caddr_t)uap->buff, RT_PREEMPT_NCOUNTS*4);

	splx(s);
}
#else
/*
 * Stubs so that linker doesn't complain.
 */
void getpreemptcounts()
{
}
#endif
#endif	/* RT_PREEMPT */
