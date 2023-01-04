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
 *	@(#)$RCSfile: lock.h,v $ $Revision: 4.2.7.6 $ (DEC) $Date: 1992/12/09 09:06:53 $
 */ 
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
 *	File:	kern/lock.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Locking primitives definitions
 *
 *	Revision History:
 *
 * 14-May-91	Ron Widyono
 *	Remove sleep lock restriction on preemption.
 *
 * 5-May-91	Ron Widyono
 *	Preemption point optimizations.  Incorporate run-time option for 
 *	preemption.
 *
 * 21-Apr-91	Ron Widyono
 *	Change simple_preemption, wait_preemption, pp_count to
 *	rt_preempt_simple, rt_preempt_sleep, rt_preempt_pp.
 *
 * 6-April-91	Ron Widyono
 *	Lock counting code for preemption points.  Preemption points in
 *	lock release functions (DEC_SLOCK_COUNT and DEC_LOCK_COUNT).
 *	Conditionalized under RT_PREEMPT.
 *
 */

#ifndef	_KERN_LOCK_H_
#define _KERN_LOCK_H_

#include <cpus.h>
#include <mach_ldebug.h>
#include <mach_ltracks.h>
#include <lock_stats.h>
#include <slock_stats.h>
#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#include <mach/boolean.h>
#include <machine/cpu.h>
#if	MACH_LDEBUG
#include <kern/assert.h>
#endif

#include <machine/inline_lock.h>

#define MACH_SLOCKS	((NCPUS > 1) || MACH_LDEBUG)

/*
 *	A simple spin lock.
 */

struct slock {
	int		lock_data;	/* in general 1 bit is sufficient */
#if	MACH_LDEBUG
	char		*slthread;	/* thread of locker */
	int		slck_addr;	/* pc where locked */
	int		sunlck_addr;	/* pc of last unlocker */
#endif
#if	SLOCK_STATS
	unsigned long	slock_tries;	/* attempts at taking lock */
	unsigned long	slock_fails;	/* misses on lock */
	unsigned long	slock_min_time;	/* minimum time lock held */
	unsigned long	slock_max_time;	/* longest time lock held */
	unsigned long	slock_avg_time;	/* average time lock held */
#endif
};

typedef struct slock	simple_lock_data_t;
typedef struct slock	*simple_lock_t;

#if	MACH_SLOCKS
extern void		simple_lock_init();
extern void		simple_lock();
extern void		simple_unlock();
extern boolean_t	simple_lock_try();
extern int		check_locks;	/* don't check until current_thread works */

#define decl_simple_lock_data(class,name)	class simple_lock_data_t name;
#define simple_lock_addr(lock)		(&(lock))
#else	/* MACH_SLOCKS */

#if	!RT_PREEMPT
/*
 *	No multiprocessor locking is necessary.
 */
#define simple_lock_init(l)
#define simple_lock(l)
#define simple_unlock(l)
#define simple_lock_try(l)	(1)	/* always succeeds */

#define decl_simple_lock_data(class,name)
#define simple_lock_addr(lock)		((simple_lock_t)0)

#else	/* !RT_PREEMPT */

/*
 *	For preemption points, lock counting code needed to make it possible
 *	to determine if a thread is executing with one or more simple or
 *	wait locks taken at any point in time.  There is a global simple lock
 *	counter slock_count[] (per CPU) and a per-thread wait lock counter
 *	thread->lock_count.  Simple locks are not actually declared.  The
 *	simple_lock_addr macro must return 1 (non-zero) because there is code
 *	that would skip an unlock otherwise.
 */
extern int		rt_preempt_enabled;
extern int		rt_preempt_enforce;
extern int		rt_preempt_fixups;
extern int		rt_check_locks;
extern int		slock_count[];	/* count spinlocks per cpu */
extern boolean_t	iplis0();
extern void		do_preemption();
extern int		rt_inc_slock(), rt_dec_slock();

struct rt_lock_debug {
	unsigned long rlb_count;
	unsigned long rlb_pc;
};

#if	RT_PREEMPT_DEBUG
extern int	rt_preempt_simple; /* # of preemptions due to simple unlock */
extern int	rt_preempt_sleep;  /* # of preemptions due to sleep lock unlock */
extern int	rt_preempt_pp;	   /* # of preemption points */
#endif

#define decl_simple_lock_data(class,name)
#define simple_lock_addr(lock)		((simple_lock_t)1)

#define	INC_SLOCK_COUNT							\
	(rt_preempt_enabled ? (rt_check_locks ? rt_inc_slock() :	\
		slock_count[cpu_number()]++) : 0)

#define	DEC_SLOCK_COUNT_PREEMPT(P)					\
	(((--slock_count[cpu_number()] == 0) && 			\
	ast_needed(cpu_number()) &&					\
	iplis0()) ? (do_preemption(P),1) : 0)

#if	!RT_PREEMPT_DEBUG
#define	DEC_SLOCK_COUNT							\
	(rt_preempt_enabled ? (rt_check_locks ? rt_dec_slock() :	\
		DEC_SLOCK_COUNT_PREEMPT(NULL)) : 0)
#else
#define	DEC_SLOCK_COUNT							\
	(rt_preempt_enabled ? (rt_check_locks ? rt_dec_slock() :	\
		(rt_preempt_pp++,					\
		DEC_SLOCK_COUNT_PREEMPT(&rt_preempt_simple))) : 0)
#endif

#define INC_LOCK_COUNT
#define DEC_LOCK_COUNT

#define simple_lock_init(l)
#define simple_lock(l)		INC_SLOCK_COUNT
#define simple_unlock(l)	DEC_SLOCK_COUNT
#define simple_lock_try(l)	(INC_SLOCK_COUNT, 1)

/*
 * PREEMPTION_FIXUP is the code for the preemption_fixup() routine.
 * syscall() and thread_block() use this macro. exception_exit() calls
 * the function in  lock.c
 */
#define PREEMPTION_FIXUP()						  \
{									  \
	if (rt_preempt_enabled) {					  \
		if (rt_preempt_enforce) {				  \
			/*						  \
			 * Need to panic here. Turn off preemption at	  \
			 * this point to prevent trashing the lock state. \
			 */						  \
			rt_preempt_enabled = 0;				  \
			panic("realtime lock count");			  \
		} else {						  \
			slock_count[0] = 0;				  \
			rt_preempt_fixups++;				  \
		}							  \
	}								  \
}

#endif	/* !RT_PREEMPT */

#endif	/* MACH_SLOCKS */

/*
 *	The general lock structure.  Provides for multiple readers,
 *	upgrading from read to write, and sleeping until the lock
 *	can be gained.
 *
 *	On some (many) architectures, assembly language code in the inline
 *	program fiddles the lock structures.  It must be changed in concert
 *	with the structure layout.
 */

struct lock {
#ifdef	vax
	/*
	 *	Efficient VAX implementation -- see field description below.
	 */
	unsigned int	read_count:16,
			want_upgrade:1,
			want_write:1,
			waiting:1,
			can_sleep:1,
			:0;
#else	/* vax */
#ifdef	ns32000
	/*
	 *	Efficient ns32000 implementation --
	 *	see field description below.
	 */
	union {
		struct {
			char	l_s_byte;	/* lock byte */
			char	l_s_unused1;
			char	l_s_unused2;
			char	l_s_type;	/* identity */
		} l_s;
		decl_simple_lock_data(,Interlock)
	} l_un;
	unsigned int	read_count:16,
			want_upgrade:1,
			want_write:1,
			waiting:1,
			can_sleep:1,
			:0;
#else	/* ns32000 */
	/*	Only the "interlock" field is used for hardware exclusion;
	 *	other fields are modified with normal instructions after
	 *	acquiring the interlock bit.
	 */
	boolean_t	want_write;	/* Writer is waiting, or locked for write */
	boolean_t	want_upgrade;	/* Read-to-write upgrade waiting */
	boolean_t	waiting;	/* Someone is sleeping on lock */
	boolean_t	can_sleep;	/* Can attempts to lock go to sleep */
	int		read_count;	/* Number of accepted readers */
#endif	/* ns32000 */
#endif	/* vax */
	char		*thread;
		/* Thread that has lock, if recursive locking allowed */
		/* (Not thread_t to avoid recursive definitions.) */

	int		recursion_depth;/* Depth of recursion */
#if	!defined(ns32000)
	/*	Put this field last in the structures, so that field
	 *	offsets are constant regardless of whether this is present.
	 *	This makes any assembly language code simpler.
	 */
	decl_simple_lock_data(,interlock)
#endif
#if	MACH_LDEBUG || MACH_LTRACKS
	char		*lthread;	/* Thread of locker */
	int		lck_addr;	/* pc where locked */
	int		unlck_addr;	/* pc of last unlocker */
#endif
#if	LOCK_STATS
	unsigned long	lock_tries;	/* number of attempts at taking lock */
	unsigned long	lock_fails;	/* misses on lock */
	unsigned long	lock_sleeps;	/* actual thread blocks on lock */
	unsigned long	lock_wait_min;	/* shortest time blocked on lock */
	unsigned long	lock_wait_max;	/* longest time blocked on lock */
	unsigned long	lock_wait_sum;	/* total time blocked on lock */
	unsigned long	lock_max_read;	/* maximum active readers */
	unsigned long	lock_nreads;	/* Number of read requests */
#endif
};

#ifdef	ns32000
#define	interlock	l_un.Interlock
#define	lock_type	l_un.l_s.l_s_type
#endif

typedef struct lock	lock_data_t;
typedef struct lock	*lock_t;


#if	MACH_LDEBUG
#define	MAX_LOCK	10

#define	LOCK_READERS(l)		((l)->read_count)
#define	LOCK_LOCKED(l)		((l)->want_write == TRUE)
#define	LOCK_THREAD(l)		((thread_t) (l)->lthread)
#define	LOCK_OWNER(l)		(LOCK_THREAD(l) == current_thread())
#define	LOCK_HOLDER(l)		(!LOCK_READERS(l) && LOCK_LOCKED(l) && \
				 LOCK_OWNER(l))
#ifdef	ns32000
#define	SLOCK_LOCKED(l)		(((l)->lock_data & 0xff) != 0)
#else
#define	SLOCK_LOCKED(l)		((l)->lock_data != 0)
#endif
#define	SLOCK_THREAD(l)		((thread_t) (l)->slthread)
#define	SLOCK_OWNER(l)		(SLOCK_THREAD(l) == current_thread())
#define	SLOCK_HOLDER(l)		(SLOCK_LOCKED(l) && SLOCK_OWNER(l))
#define	LASSERT(clause)		if (check_locks) ASSERT(clause)

struct slock_debug {
	int count;
	int addr[MAX_LOCK];
};
extern	int	check_locks;
extern	int	check_lock_counts;
extern	struct slock_debug slck_dbg[];

#else	/* MACH_LDEBUG */

#define	LOCK_READERS(l)
#define	LOCK_LOCKED(l)
#define	LOCK_THREAD(l)
#define	LOCK_OWNER(l)
#define	LOCK_HOLDER(l)
#define	SLOCK_LOCKED(l)
#define	SLOCK_THREAD(l)
#define	SLOCK_OWNER(l)
#define	SLOCK_HOLDER(l)
#define	LASSERT(clause)

#endif	/* MACH_LDEBUG */

/* Sleep locks must work even if no multiprocessing */

extern void		lock_init();
extern void		lock_sleepable();
extern void		lock_write();
extern void		lock_read();
extern void		lock_done();
extern boolean_t	lock_read_to_write();
extern void		lock_write_to_read();
extern boolean_t	lock_try_write();
extern boolean_t	lock_try_read();
extern boolean_t	lock_try_read_to_write();

#define lock_read_done(l)	lock_done(l)
#define lock_write_done(l)	lock_done(l)

extern void		lock_set_recursive();
extern void		lock_clear_recursive();

#if	__GNUC__ && MACH_SLOCKS && !_NO_INLINE_LOCKS
/*
 * Define _simple_lock and _simple_lock_try in the machine
 * dependant inline_lock.h file if they need something more
 * complicated than the simple Uniprocessor locks below. If you
 * redefine these be sure to define __SLOCK so that the default
 * versions are not used.
 *
 * Also to support the debugging lock macros you should define a
 * function, current_pc, that returns the current location to be
 * used in recording the last locker.
 */

#if	!__SLOCKS && (NCPUS == 1)

extern __inline__ _simple_lock_init(l)
        int *l;
{
        *(l) = 0;
}

extern __inline__ _simple_lock(l)
        int *l;
{
        LASSERT(*(l) == 0);
        *(l) = 1;
}

extern __inline__ _simple_lock_try(l)
        int *l;
{
        LASSERT(*(l) == 0);
        *(l) = 1;
        return(1);
}

extern __inline__ _simple_unlock(l)
        int *l;
{
        LASSERT(*(l) != 0);
        *(l) = 0;
}

#define SIMPLE_LOCK_DEBUG(l) \
        l->slthread = (char *) current_thread(); \
        inc_slock(l);

#define SIMPLE_UNLOCK_DEBUG(l) \
        l->slthread = (char *) -1; \
        dec_slock(l);

#endif	/* !__SLOCKS && (NCPUS == 1) */

extern void __inline__ simple_lock_init(l)
	simple_lock_t	l;
{
#if	MACH_LDEBUG
	l->slthread = (char *) -1;
        l->slck_addr = (int) -1;
        l->sunlck_addr = (int) -1;
#endif
        _simple_lock_init(&l->lock_data);
}

extern void __inline__ simple_lock(l)
	simple_lock_t l;
{
        _simple_lock(&l->lock_data);
#if	MACH_LDEBUG
        SIMPLE_LOCK_DEBUG(l);
#endif
}

extern boolean_t __inline__ simple_lock_try(l)
	simple_lock_t l;
{
#if	MACH_LDEBUG
        register int state = _simple_lock_try(&l->lock_data);
        if (state)
                SIMPLE_LOCK_DEBUG(l);
        return(state);
#else
        return(_simple_lock_try(&l->lock_data));
#endif
}

extern void __inline__ simple_unlock(l)
	simple_lock_t l;
{
#if	MACH_LDEBUG
        SIMPLE_UNLOCK_DEBUG(l);
#endif
        _simple_unlock(&l->lock_data);
}

#endif  /* __GNUC__ && MACH_SLOCKS && !_NO_INLINE_LOCKS */

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_LOCK_H_ */
