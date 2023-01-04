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
static char	*sccsid = "@(#)$RCSfile: thread_swap.c,v $ $Revision: 4.3.5.7 $ (DEC) $Date: 1993/01/08 17:59:47 $";
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
 *
 *	File:	kern/thread_swap.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1987, Avadis Tevanian, Jr. and Richard F. Rashid
 *
 *	Mach thread swapper:
 *		Find idle threads to swap, freeing up kernel stack resources
 *		at the expense of allowing them to execute.
 *
 *		Swap in threads that need to be run.  This is done here
 *		by the swapper thread since it cannot be done (in general)
 *		when the kernel tries to place a thread on a run queue.
 *
 *	Note: The act of swapping a thread in Mach does not mean that
 *	its memory gets forcibly swapped to secondary storage.  The memory
 *	for the task corresponding to a swapped thread is paged out
 *	through the normal paging mechanism.
 *
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 03-May-91	Peter H. Smith
 *	Add monitoring of swap events.
 */

#include <sys/types.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/lock.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <kern/sched_prim.h>
#include <kern/processor.h>
#include <kern/thread_swap.h>
#include <kern/task_swap.h>
#include <machine/machparam.h>		/* for splsched */
#include <sys/sched_mon.h>

queue_head_t		swapin_queue;
decl_simple_lock_data(,	swapper_lock_data)

#define swapper_lock()		simple_lock(&swapper_lock_data)
#define swapper_unlock()	simple_unlock(&swapper_lock_data)

/*
 *	thread_swapper_init: [exported]
 *
 *	Initialize the swapper module.
 */
void thread_swapper_init()
{
	queue_init(&swapin_queue);
	simple_lock_init(&swapper_lock_data);
}

/*
 *	thread_swapin: [exported]
 *
 *	Place the specified thread in the list of threads to swapin.  It
 *	is assumed that the thread is locked, therefore we are at splsched.
 */

void thread_swapin(thread)
	thread_t	thread;
{
	switch (thread->swap_state) {
	    case TH_SW_OUT:
		if (thread->task->swap_state == TASK_OUTSWAPPED) 
			task_swapin_request(thread->task, thread->priority);
		else {
			/*
		 	*	Swapped out - queue for swapin thread
		 	*/
			thread->swap_state = TH_SW_COMING_IN;
			swapper_lock();
			enqueue_tail(&swapin_queue, (queue_entry_t) thread);
			swapper_unlock();
			thread_wakeup((vm_offset_t)&swapin_queue);
		}
		break;

	    case TH_SW_GOING_OUT:
		/*
		 *	Being swapped out - wait until swapped out,
		 *	then queue for swapin thread (in thread_swapout).
		 */
		thread->swap_state = TH_SW_WANT_IN;
		break;

	    case TH_SW_WANT_IN:
	    case TH_SW_COMING_IN:
		/*
		 *	Already queued for swapin thread, or being
		 *	swapped in
		 */
		break;

	    default:
		/*
		 *	Swapped in or unswappable
		 */
		panic("thread_swapin");
	}
}

/*
 *	thread_doswapin:
 *
 *	Swapin the specified thread, if it should be runnable, then put
 *	it on a run queue.  No locks should be held on entry, as it is
 *	likely that this routine will sleep (waiting for page faults).
 */
void thread_doswapin(thread)
	thread_t	thread;
{
	register vm_offset_t	addr;
	register int		s;

	/*
	 *	If the that is outswapped, swap in in.
	 */

	if (thread->task->swap_state == TASK_OUTSWAPPED) {
		task_t	task = thread->task;

		simple_lock(&task_outswapped_queue);
		queue_remove(&task_outswapped_queue, task, task_t, task_link);
        	task_outswapped_queue_count--;
        	simple_unlock(&task_outswapped_queue);
		task->swap_state = TASK_COMMING_IN;
		task_swapin(task);
	}

	/*
	 *	Wire down the kernel stack.
	 */

	addr = thread->kernel_stack;
	(void) vm_map_pageable(kernel_map, trunc_page(addr),
			round_page(addr + KERNEL_STACK_SIZE),
			VM_PROT_READ|VM_PROT_WRITE);
#ifdef __alpha
	pcb_fixup(thread);	/* recompute paddr if needed */
#endif

	/*
	 *	Place on run queue if appropriate.
	 */

	s = splsched();
	RT_SCHED_HIST(RTS_swapin, thread, thread->policy,
		      thread->state, thread->swap_state, 0);
	thread_lock(thread);
	thread->swap_state = TH_SW_IN;
	thread->state &= ~TH_SWAPPED;
	if (thread->state & TH_RUN)
		thread_setrun(thread, TRUE);
	thread_unlock(thread);
	(void) splx(s);
}

/*
 *	thread_swapout:
 *
 *	Swap out the specified thread (unwire its kernel stack).
 *	The thread must already be marked as 'swapping out'.
 */
void thread_swapout(thread,collect)
	thread_t	thread;
	boolean_t		collect;
{
	register vm_offset_t	addr;
	int			s;

	/*
	 *	Thread is marked as swapped before we swap it out; if
	 *	it is awakened while we are swapping it out, it will
	 *	be put on the swapin list.
	 */

	/*
	 * Notify the pcb module that it must update any
 	 * hardware state associated with this thread.
	 */
	pcb_synch(thread);

	/*
	 *	Unwire the kernel stack.
	 */

	addr = thread->kernel_stack;
	(void) vm_map_pageable(kernel_map, trunc_page(addr),
			round_page(addr + KERNEL_STACK_SIZE), VM_PROT_NONE);

	if (collect)
	pmap_collect(vm_map_pmap(thread->task->map));

	s = splsched();
	thread_lock(thread);
	RT_SCHED_HIST(RTS_swapout, thread, thread->policy,
		      thread->swap_state, thread->state, 0);
	switch (thread->swap_state) {
	    case TH_SW_GOING_OUT:
		thread->swap_state = TH_SW_OUT;
		break;

	    case TH_SW_WANT_IN:
		thread->swap_state = TH_SW_OUT;
		thread_swapin(thread);
		break;

	    default:
		panic("thread_swapout");
	}
	thread_unlock(thread);
	splx(s);
}

/*
 *	swapin_thread: [exported]
 *
 *	This procedure executes as a kernel thread.  Threads that need to
 *	be swapped in are swapped in by this thread.
 */
void swapin_thread()
{
	while (TRUE) {
		register thread_t	thread;
		register int		s;

		s = splsched();
		swapper_lock();

		while ((thread = (thread_t) dequeue_head(&swapin_queue))
							== THREAD_NULL) {
			assert_wait((vm_offset_t)&swapin_queue, FALSE);
			swapper_unlock();
			splx(s);
			thread_block();
			s = splsched();
			swapper_lock();
		}

		swapper_unlock();
		splx(s);

		thread_doswapin(thread);
	}
}

#ifndef jmfix
boolean_t	thread_swapout_allowed = FALSE;	/* until kernel stack changes */
#else
boolean_t	thread_swapout_allowed = TRUE;
#endif

int	thread_swap_tick = 0;
int	last_swap_tick = 0;

#define MAX_SWAP_RATE	60

/*
 *	swapout_threads: [exported]
 *
 *	This procedure is called periodically by the pageout daemon.  It
 *	determines if it should scan for threads to swap and starts that
 *	scan if appropriate.  (Algorithm is like that of old package)
 *	The pageout daemon sets the now flag if all the page queues are
 *	empty and it wants to start the swapper right away.
 */
void swapout_threads(now)
	boolean_t	now;
{
	if (thread_swapout_allowed &&
	    (now || (thread_swap_tick > (last_swap_tick + MAX_SWAP_RATE)))) {
		last_swap_tick = thread_swap_tick;
		thread_wakeup((vm_offset_t)&thread_swap_tick);/* poke swapper */
	}
}

/*
 *	swapout_scan:
 *
 *	Scan the list of all threads looking for threads to swap.
 */
void swapout_scan()
{
	register thread_t	thread, prev_thread;
	processor_set_t		pset, prev_pset;
	register int		s;

	zone_gc();

	prev_thread = THREAD_NULL;
	prev_pset = PROCESSOR_SET_NULL;
	simple_lock(&all_psets_lock);
	pset = (processor_set_t) queue_first(&all_psets);
	while (!queue_end(&all_psets, (queue_entry_t) pset)) {
		pset_lock(pset);
		thread = (thread_t) queue_first(&pset->threads);
		while (!queue_end(&pset->threads, (queue_entry_t) thread)) {
			s = splsched();
			thread_lock(thread);
			if ((thread->state & (TH_RUN | TH_SWAPPED)) == 0 &&
			    thread->swap_state == TH_SW_IN &&
			    thread->interruptible &&
			    (sched_tick - thread->sleep_stamp > 10)) {
				thread->state |= TH_SWAPPED;
				thread->swap_state = TH_SW_GOING_OUT;
				thread->ref_count++;
				thread_unlock(thread);
				(void) splx(s);
				pset->ref_count++;
				pset_unlock(pset);
				simple_unlock(&all_psets_lock);
				thread_swapout(thread,TRUE);		/* swap it */

				if (prev_thread != THREAD_NULL)
					thread_deallocate(prev_thread);
				if (prev_pset != PROCESSOR_SET_NULL)
					pset_deallocate(prev_pset);

				prev_thread = thread;
				prev_pset = pset;
				simple_lock(&all_psets_lock);
				pset_lock(pset);
				s = splsched();
				thread_lock(thread);
			}
			thread_unlock(thread);
			splx(s);
			thread = (thread_t) queue_next(&thread->pset_threads);
		}
		pset_unlock(pset);
		pset = (processor_set_t) queue_next(&pset->all_psets);
	}
	simple_unlock(&all_psets_lock);

	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);
	if (prev_pset != PROCESSOR_SET_NULL)
		pset_deallocate(prev_pset);
}

/*
 *	swapout_thread: [exported]
 *
 *	Executes as a separate kernel thread.  This thread is periodically
 *	woken up.  When this happens, it initiates the scan for threads
 *	to swap.
 */
void swapout_thread()
{
	(void) spl0();
	while (TRUE) {
		assert_wait((vm_offset_t)&thread_swap_tick, FALSE);
		thread_block();
		swapout_scan();
	}
}

/*
 *	Mark a thread as swappable or unswappable.  Must be called
 *	before the thread is first allowed to run.
 */
void thread_swappable(thread, is_swappable)
	thread_t	thread;
	boolean_t	is_swappable;
{
	int	s = splsched();
	thread_lock(thread);
	if (is_swappable)
	    thread->swap_state = TH_SW_IN;
	else
	    thread->swap_state = TH_SW_UNSWAPPABLE;
	thread_unlock(thread);
	(void) splx(s);
}
