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
 *	@(#)$RCSfile: processor.h,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/04/06 15:19:08 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	processor.h:	Processor and processor-set definitions.
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 9-Apr-91	Peter .H Smith
 *	Add was_first_quantum.  This field gets set if a scheduler preemption
 *	is requested while in a thread's first quantum (the requestor clears
 *	the first_quantum field, so we have to save the info somewhere).
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_PROCESSOR_H_
#define	_KERN_PROCESSOR_H_

/*
 *	Data structures for managing processors and sets of processors.
 */

#include <cpus.h>
#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_PROCESSOR_H_PREEMPT_
#endif
#endif

#include <mach/boolean.h>
#include <kern/lock.h>
#include <mach/policy.h>
#include <mach/port.h>
#include <kern/queue.h>
#include <kern/sched.h>

#if	NCPUS > 1
#include <machine/ast_types.h>
#endif	/* NCPUS > 1 */

typedef port_t			pset_port_t;

struct processor_set {
	struct	run_queue	runq;		/* runq for this set */
	queue_head_t		idle_queue;	/* idle processors */
	int			idle_count;	/* how many ? */
	simple_lock_data_t	idle_lock;	/* lock for above */
	queue_head_t		processors;	/* all processors here */
	int			processor_count;	/* how many ? */
	boolean_t		empty;		/* true if no processors */
	queue_head_t		tasks;		/* tasks assigned */
	int			task_count;	/* how many */
	queue_head_t		threads;	/* threads in this set */
	int			thread_count;	/* how many */
	int			ref_count;	/* structure ref count */
	queue_chain_t		all_psets;	/* link for all_psets */
	boolean_t		active;		/* is pset in use */
	simple_lock_data_t	lock;		/* lock for everything else */
	pset_port_t		pset_self;	/* port for operations */
	pset_port_t		pset_name_self;	/* port for information */
	int			max_priority;	/* maximum priority */
	int			policies;	/* bit vector for policies */
	int			set_quantum;	/* current default quantum */
#if	NCPUS > 1
	int			quantum_adj_index; /* runtime quantum adj. */
	simple_lock_data_t	quantum_adj_lock;  /* lock for above */
	int			machine_quantum[NCPUS+1]; /* ditto */
#endif	/* NCPUS > 1 */
	long			mach_factor;	/* mach_factor */
	long			load_average;	/* load_average */
	long			sched_load;	/* load avg for scheduler */
};

typedef	struct processor_set *processor_set_t;

#define PROCESSOR_SET_NULL	(processor_set_t)0

extern struct processor_set	default_pset;

struct processor {
	struct run_queue runq;		/* local runq for this processor */
		/* XXX want to do this round robin eventually */
	queue_chain_t	processor_queue; /* idle/assign/shutdown queue link */
	int		state;		/* See below */
	struct thread	*next_thread;	/* next thread to run if dispatched */
	struct thread	*idle_thread;	/* this processor's idle thread. */
	int		quantum;	/* quantum for current thread */
	boolean_t	first_quantum;	/* first quantum in succession */
#if RT
	/* Support for RR scheduling policy.  When a preemption is about to
	 * occur, was_first_quantum is set if first_quantum was true, and then
	 * first_quantum is cleared.  Later, thread_block() saves the thread's
	 * remaining quantum if was_first_quantum is true.
	 */
	boolean_t	was_first_quantum; /* RT_SCHED: FIFO/RR preemption */
#endif /* RT */
	int		last_quantum;	/* last quantum assigned */

	processor_set_t	processor_set;	/* processor set I belong to */
	processor_set_t processor_set_next;	/* set I will belong to */
	queue_chain_t	processors;	/* all processors in set */
	simple_lock_data_t	lock;
	pset_port_t		processor_self;	/* port for operations */
	int		slot_num;	/* machine-indep slot number */
#if	NCPUS > 1
	ast_check_t	ast_check_data;	/* for remote ast_check invocation */
#endif	/* NCPUS > 1 */
	/* punt id data temporarily */
};

typedef struct processor *processor_t;

#define PROCESSOR_NULL	(processor_t)0

extern struct processor	processor_array[NCPUS];

/*
 *	Chain of all processor sets.
 */
extern queue_head_t		all_psets;
extern int			all_psets_count;
decl_simple_lock_data(extern, all_psets_lock)

/*
 *	XXX need a pointer to the master processor structure
 */

extern processor_t	master_processor;

/*
 *	NOTE: The processor->processor_set link is needed in one of the
 *	scheduler's critical paths.  [Figure out where to look for another
 *	thread to run on this processor.]  It is accessed without locking.
 *	The following access protocol controls this field.
 *
 *	Read from own processor - just read.
 *	Read from another processor - lock processor structure during read.
 *	Write from own processor - lock processor structure during write.
 *	Write from another processor - NOT PERMITTED.
 *
 */

/*
 *	Processor state locking:
 *
 *	Values for the processor state are defined below.  If the processor
 *	is off-line or being shutdown, then it is only necessary to lock
 *	the processor to change its state.  Otherwise it is only necessary
 *	to lock its processor set's idle_lock.  Scheduler code will
 *	typically lock only the idle_lock, but processor manipulation code
 *	will often lock both.
 */

#define PROCESSOR_OFF_LINE	0	/* Not in system */
#define	PROCESSOR_RUNNING	1	/* Running normally */
#define	PROCESSOR_IDLE		2	/* idle */
#define PROCESSOR_DISPATCHING	3	/* dispatching (idle -> running) */
#define	PROCESSOR_ASSIGN	4	/* Assignment is changing */
#define PROCESSOR_SHUTDOWN	5	/* Being shutdown */

/*
 *	Use processor ptr array to find current processor's data structure.
 *	This replaces a multiplication (index into processor_array) with
 *	an array lookup and a memory reference.  It also allows us to save
 *	space if processor numbering gets too sparse.
 */

extern processor_t	processor_ptr[NCPUS];

#define cpu_to_processor(i)	(processor_ptr[i])

#define current_processor()	(processor_ptr[cpu_number()])
#define current_processor_set()	(current_processor()->processor_set)

/* Compatibility -- will go away */

#define cpu_state(slot_num)	(processor_ptr[slot_num]->state)
#define cpu_idle(slot_num)	(processor_state(slot_num) == PROCESSOR_IDLE)

/* Useful lock macros */

#define	pset_lock(pset)		simple_lock(&(pset)->lock)
#define pset_unlock(pset)	simple_unlock(&(pset)->lock)

#define processor_lock(pr)	simple_lock(&(pr)->lock)
#define processor_unlock(pr)	simple_unlock(&(pr)->lock)

typedef port_t	*processor_array_t;
typedef port_t	*processor_set_array_t;

#if	RT_PREEMPT
#ifdef	_KERN_PROCESSOR_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _PROCESSOR_H_ */
