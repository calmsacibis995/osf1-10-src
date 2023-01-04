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
static char	*sccsid = "@(#)$RCSfile: clock_prim.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/04/27 20:01:01 $";
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
 *	File:	clock_prim.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr.
 *
 *	Clock primitives.
 *
 *	Revision History:
 *
 * 02-May-91	Peter H. Smith
 *	Modify quantum expiration code to support POSIX realtime scheduling
 *	policies.
 */

#include <cpus.h>
#include <rt_sched.h>

#include <machine/cpu.h>
#include <mach/machine.h>
#include <mach/policy.h>
#include <kern/processor.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <sys/sched_mon.h>

#include <machine/machparam.h>

/*
 *	USAGE_THRESHOLD is the amount by which usage must change to
 *	cause a priority shift that moves a thread between run queues.
 */

#ifdef	PRI_SHIFT_2
#if	PRI_SHIFT_2 > 0
#define USAGE_THRESHOLD (((1 << PRI_SHIFT) + (1 << PRI_SHIFT_2)) << (2 + SCHED_SHIFT))
#else	PRI_SHIFT_2 > 0
#define USAGE_THRESHOLD (((1 << PRI_SHIFT) - (1 << -(PRI_SHIFT_2))) << (2 + SCHED_SHIFT))
#endif	PRI_SHIFT_2 > 0
#else	PRI_SHIFT_2
#define USAGE_THRESHOLD	(1 << (PRI_SHIFT + SCHED_SHIFT))
#endif	PRI_SHIFT_2
int usage_value_is = USAGE_THRESHOLD;
/*
 *	clock_tick:
 *
 *	Handle hardware clock ticks.  The number of ticks that has elapsed
 *	since we were last called is passed as "nticks."  Note that this
 *	is called for each processor that is taking clock interrupts, and
 *	that some processors may be running at different clock rates.
 *	However, all of these rates must be some multiple of the basic clock
 *	tick.
 *
 *	The state the processor was executing in is passed as "state."
 */

clock_tick(nticks, state)
	int		nticks;
	register int	state;
{
	int				mycpu;
	register thread_t		thread;
	register int			quantum;
	register processor_t		myprocessor;
#if	NCPUS > 1
	register processor_set_t	pset;
#endif	NCPUS > 1
	int				s;

	mycpu = cpu_number();		/* who am i? */
	myprocessor = cpu_to_processor(mycpu);
#if	NCPUS > 1
	pset = myprocessor->processor_set;
#endif	NCPUS > 1

	/*
	 *	Update the cpu ticks for this processor. XXX
	 */

	machine_slot[mycpu].cpu_ticks[state] += nticks;

	/*
	 *	Account for thread's utilization of these ticks.
	 *	This assumes that there is *always* a current thread.
	 *	When the processor is idle, it should be the idle thread.
	 */

	thread = current_thread();

	/*
	 *	Update set_quantum and calculate the current quantum.
	 */
#if	NCPUS > 1
	pset->set_quantum = pset->machine_quantum[
		((pset->runq.count > pset->processor_count) ?
		  pset->processor_count : pset->runq.count)];

	if (myprocessor->runq.count != 0)
		quantum = min_quantum;
	else
		quantum = pset->set_quantum;
#else	NCPUS > 1
	quantum = min_quantum;
	default_pset.set_quantum = quantum;
#endif	NCPUS > 1
		
	/*
	 *	Now recompute the priority of the thread if appropriate.
	 */

	if (state != CPU_STATE_IDLE) {
		myprocessor->quantum -= nticks;
#if	NCPUS > 1
		/*
		 *	Runtime quantum adjustment.  Use quantum_adj_index
		 *	to avoid synchronizing quantum expirations.
		 */
		if ((quantum != myprocessor->last_quantum) &&
		    (pset->processor_count > 1)) {
			myprocessor->last_quantum = quantum;
			simple_lock(&pset->quantum_adj_lock);
			quantum = min_quantum + (pset->quantum_adj_index *
				(quantum - min_quantum)) / 
					(pset->processor_count - 1);
			if (++(pset->quantum_adj_index) >=
			    pset->processor_count)
				pset->quantum_adj_index = 0;
			simple_unlock(&pset->quantum_adj_lock);
		}
#endif	NCPUS > 1
		if (myprocessor->quantum <= 0) {
			s = splsched();
			thread_lock(thread);
			if (thread->sched_stamp != sched_tick) {
				update_priority(thread);
			}
			else {
			    if ((thread->policy == POLICY_TIMESHARE) &&
				(thread->depress_priority < 0)) {
				    thread_timer_delta(thread);
				    compute_my_priority(thread);
			    }
			}
			thread_unlock(thread);
			(void) splx(s);
			/*
			 *	This quantum is up, give this thread another.
			 */
			if (thread->policy == POLICY_TIMESHARE) {
				myprocessor->quantum += quantum;
				myprocessor->first_quantum = FALSE;
			}
			else {
				/*
				 *    Fixed priority has per-thread quantum.
				 */
				myprocessor->quantum += thread->sched_data;
#if RT_SCHED
				/*
				 * Clear remaining quantum.  If running RR,
				 * clear first_quantum to allow threads waiting
				 * at same priority to get in.
				 */
				RT_SCHED_HIST_SPL(RTS_rrqexp, thread,
						  thread->policy,
						  thread->rmng_quantum, 
						  myprocessor->quantum, 
						  myprocessor->first_quantum);
				thread->rmng_quantum = 0;
				if (thread->policy == POLICY_RR) {
				  myprocessor->first_quantum = FALSE;
				  myprocessor->was_first_quantum = FALSE;
				}
#endif /* RT_SCHED */
			}
		}
		/*
		 *	Recompute priority if appropriate.
		 */
		else {
		    s = splsched();
		    thread_lock(thread);
		    if (thread->sched_stamp != sched_tick) {
			update_priority(thread);
		    }
		    else {
			if ((thread->policy == POLICY_TIMESHARE) &&
			    (thread->depress_priority < 0)) {
				thread_timer_delta(thread);
				if (thread->sched_delta >= USAGE_THRESHOLD) {
				    compute_my_priority(thread);
				}
			}
		    }
		    thread_unlock(thread);
		    (void) splx(s);
		}
		/*
		 * Check for and schedule ast if needed.
		 */
		ast_check();
	}
}
