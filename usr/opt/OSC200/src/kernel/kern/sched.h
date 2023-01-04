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
 *	@(#)$RCSfile: sched.h,v $ $Revision: 4.2.11.2 $ (DEC) $Date: 1993/09/21 22:14:24 $
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
 *	File:	sched.h
 *	Author:	Avadis Tevanian, Jr.
 *	Copyright (C) 1985, Avadis Tevanian, Jr.
 *
 *	Header file for scheduler.
 *
 *	Revision History:
 *
 * 02-May-91	Peter H. Smith
 *	Move the user-exportable portion back to usr/include/sched.h -- the
 *	mess of having two files with the same name is less than the mess of
 *	having mixed functionality in the one file.
 *
 * 9-Apr-91	Peter .h Smith
 *	Extend to 64 run queues.  Introduce NRQS_MAX, which is the number of
 *	run queues in the structure, so that NRQS can be smaller.  This allows
 *	the data structures to stay the same whether 32 or 64 queues are being
 *	used.
 *	Add run_queue_bitmask; this is only used if the run queue extensions
 *	(RT_SCHED_RQ) are being used.
 *	Define macros for manipulating and reading bits in run_queue_bitmask.
 *	Move the lock to the end of the run queue structure, so that access to
 *	the structure from locore.s is easier. (This can go away as soon as
 *	the performance bottleneck in sched_prim.c is relieved).
 *	Conditionalize on ASSEMBLER, so that NRQS_MAX can be picked up from
 *	locore.s. (This can go away as soon as the performance bottleneck in
 *	sched_prim.c goes away).
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_SCHED_H_
#define _KERN_SCHED_H_

#if !ASSEMBLER
#include <cpus.h>
#include <simple_clock.h>
#include <stat_time.h>
#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_SCHED_H_PREEMPT_
#endif /* _SKIP_PREEMPT_H_ */
#endif /* RT_PREEMPT */

#include <kern/queue.h>
#include <kern/lock.h>
#include <kern/macro_help.h>
#endif /* !ASSEMBLER */

#include <rt_sched_rq.h>
#if RT
#include <sys/limits.h>
#endif /* RT */

#if !ASSEMBLER
#if	STAT_TIME

/*
 *	Statistical timing uses microseconds as timer units.  18 bit shift
 *	yields priorities.  PRI_SHIFT_2 isn't needed.
 */
#define PRI_SHIFT	18

#else	/* STAT_TIME */

/*
 *	Otherwise machine provides shift(s) based on time units it uses.
 */
#include <machine/sched_param.h>

#endif	/* STAT_TIME */
#endif /* !ASSEMBLER */

#if RT
/* 
 * Use NRQS_MAX to specify the number of queue headers in the runq structure.
 * Use NRQS to specify the number of queue headers being used.  This will allow
 * compilation of data structures which accomodate 64 run queues, while
 * permitting the scheduler code to use only the first 32 queue headers.
 */
#define NRQS_MAX 64
#else /* RT */
#define NRQS_MAX 32
#endif /* RT */

#if RT_SCHED_RQ
/*
 * If the run queue extensions are enabled, use all NRQS_MAX queues.
 * Otherwise, only use NRQS queues.  The data structures always leave
 * space for NRQS_MAX queues, but the priority calculations and user
 * parameter checks use NRQS.  This allows us to have compatible
 * data structures while still being able to configure the number of
 * run queues we use.
 *
 * Probably this should be simplified once we decide how many queues we
 * want to have.
 */
#define NRQS	NRQS_MAX
#else /* RT_SCHED_RQ */
#define NRQS	32			/* 32 run queues per cpu */
#endif /* RT_SCHED_RQ */

/*
 * Provide an adjustment value, so that interfaces which need to be consistent
 * based on NRQS_MAX, regardless of the value of NRQS, can do so.  This is
 * used in thread_info (src/kernel/kern/thread.c) to return priority values.
 */
#define NRQS_ADJUST (NRQS_MAX - NRQS)

#if !ASSEMBLER
#if RT
/*
 * The run_queue_bitmask structure is a mask where each bit represents the
 * state of one run queue header.  If the run queue is empty, the bit is 0,
 * if the run queue is nonempty, the bit is 1.  This is made a struct so that
 * there is some level of data opaqueness -- a machine with 64-bit integers
 * could change the definition of bits within the structure.
 */
struct run_queue_bitmask {
  unsigned long		bits[(NRQS_MAX + (LONG_BIT -1)) / LONG_BIT];
};
#endif /* RT */

struct run_queue {
#if RT
	/*
	 * The RT_SCHED_OPT flag turns on some machine code which depends on
	 * the order of the fields in the runq structure. (This will all go
	 * away if the sched_prim.c performance bottleneck is solved by the
	 * new compiler).
	 */
	queue_head_t		runq[NRQS_MAX];	/* one for each priority */
#else /* RT */
	queue_head_t		runq[NRQS];	/* one for each priority */
	decl_simple_lock_data(,	lock)		/* one lock for all queues */
#endif /* RT */
	int			low;		/* low queue value */
	int			count;		/* count of threads runable */
#if RT
	struct run_queue_bitmask mask;		/* for fast lookup */
	decl_simple_lock_data(,	lock)		/* one lock for all queues */
#endif /* RT */
};

typedef struct run_queue	*run_queue_t;
#define RUN_QUEUE_NULL	((run_queue_t) 0)

#define csw_needed(thread, processor) ((thread)->state & TH_SUSP ||	\
		((processor)->runq.count > 0) ||			\
		((processor)->first_quantum == FALSE &&			\
		 ((processor)->processor_set->runq.count > 0 &&		\
		  (processor)->processor_set->runq.low <=		\
			((thread)->sched_pri))))


/*
 *	Scheduler routines.
 */

extern struct run_queue	*rem_runq();
extern struct thread	*choose_thread();
extern queue_head_t	action_queue;	/* assign/shutdown queue */
decl_simple_lock_data(extern,action_lock)

extern int		min_quantum;	/* defines max context switch rate */

/*
 *	Default base priorities for threads.
 *
 *  RT_SCHED:	Always define these, and always use the constants in the
 *		routines which previously had hardcoded priority values.
 *		Eventually, we may want to make some of these values
 *		configurable.
 */
/*
 * Default base priorities for system and user threads.
 */
#define BASEPRI_SYSTEM (NRQS - 26)
#define BASEPRI_USER (NRQS - 20)

/*
 * Base priorities for lowest priority thread and highest priority non-realtime
 * (system) thread.
 */
#define BASEPRI_LOWEST (NRQS - 1)
#define BASEPRI_HIGHEST (NRQS - 32)

/*
 * Base priorities for specific kernel threads.
 */
#define BASEPRI_ACCTWATCH (NRQS - 22)
#define BASEPRI_SOFTCLOCK (NRQS - 32)
#define BASEPRI_PSIGNAL (NRQS - 22)

/*
 *	Macro to check for invalid priorities.
 */

#define invalid_pri(pri) (((pri) < 0) || ((pri) >= NRQS))

#if RT
/*
 * Macro to check for invalid POSIX priorities.  This will stay simple as long
 * as the priority ranges for all the policies are the same, and the POSIX
 * range has the same boundaries as the internal range.
 */
#define rt_sched_invalid_posix_pri(policy,pri) invalid_pri(pri)
#endif /* RT */

/*
 *	Shift structures for holding update shifts.  Actual computation
 *	is  usage = (usage >> shift1) +/- (usage >> abs(shift2))  where the
 *	+/- is determined by the sign of shift 2.
 */
struct shift {
	int	shift1;
	int	shift2;
};

typedef	struct shift	*shift_t, shift_data_t;

/*
 *	sched_tick increments once a second.  Used to age priorities.
 */

extern unsigned	sched_tick;

#define SCHED_SCALE	128
#define SCHED_SHIFT	7

/*
 *	thread_timer_delta macro takes care of both thread timers.
 */

#define thread_timer_delta(thread)  				\
MACRO_BEGIN							\
	register unsigned	delta;				\
								\
	delta = 0;						\
	TIMER_DELTA((thread)->system_timer,			\
		(thread)->system_timer_save, delta);		\
	TIMER_DELTA((thread)->user_timer,			\
		(thread)->user_timer_save, delta);		\
	(thread)->cpu_delta += delta;				\
	(thread)->sched_delta += delta * 			\
			(thread)->processor_set->sched_load;	\
MACRO_END

#if	SIMPLE_CLOCK
/*
 *	sched_usec is an exponential average of number of microseconds
 *	in a second for clock drift compensation.
 */

extern int	sched_usec;

#endif /* SIMPLE_CLOCK */

#if	RT_PREEMPT
#ifdef	_KERN_SCHED_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#if RT_SCHED_RQ
/*
 * Run Queue Bitmask Operations.
 *
 * These are defined as macros so that future 64-bit machines can use simpler
 * mechanisms.  Note that this code and the data structures are not optimized
 * for 64-bit machines, but the code ought to compile and run.
 */

#define RT_SCHED_RUNQ_BIT_VALUE(mask,pri) \
  (((struct run_queue_bitmask *)(mask))->bits[(pri)/LONG_BIT] \
   >> ((pri)%LONG_BIT) & 1)

#define RT_SCHED_CLEAR_RUNQ_BIT(mask,pri)				\
	MACRO_BEGIN							\
	((struct run_queue_bitmask *)(mask))->bits[(pri)/LONG_BIT]	\
  			&= ~(1L << ((pri)%LONG_BIT));			\
	MACRO_END

#define RT_SCHED_SET_RUNQ_BIT(mask,pri)					\
	MACRO_BEGIN							\
	((struct run_queue_bitmask *)(mask))->bits[(pri)/LONG_BIT]	\
			|= 1L << ((pri)%LONG_BIT);			\
	MACRO_END
        
extern int find_first_runq_bit_set();

#else /* RT_SCHED_RQ */
#define RT_SCHED_RUNQ_BIT_VALUE(mask,pri) 0
#define RT_SCHED_CLEAR_RUNQ_BIT(mask,pri)
#define RT_SCHED_SET_RUNQ_BIT(mask,pri)
#endif /* RT_SCHED_RQ */

#endif /* !ASSEMBLER */
#endif /* _KERN_SCHED_H_ */
