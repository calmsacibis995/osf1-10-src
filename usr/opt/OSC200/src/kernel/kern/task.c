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
static char	*sccsid = "@(#)$RCSfile: task.c,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1993/09/22 18:28:24 $";
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
 *	File:	kern/task.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young, David Golub,
 *		David Black
 *
 *	Task management primitives implementation.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <mach_host.h>
#include <mach_emulation.h>

#include <sys/types.h>
#include <kern/mach_param.h>
#include <kern/task.h>
#include <kern/task_swap.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>		/* for kernel_map */
#include <kern/ipc_tt.h>
#include <kern/ipc_globals.h>	/* for ipc_kernel_map */
#include <mach/task_info.h>
#include <mach/task_special_ports.h>
#include <kern/processor.h>
#include <kern/sched_prim.h>	/* for thread_wakeup */

#include <machine/machparam.h>	/* for splsched */

task_t	kernel_task;
zone_t	task_zone;

extern zone_t	u_zone;		/* UNIX */
extern int	taskmax;

int	tasks_created=0;
int	tasks_dealloced=0;
void task_init()
{
	task_zone = zinit(
			sizeof(struct task),
			taskmax * sizeof(struct task),
			TASK_CHUNK * sizeof(struct task),
			"tasks");

	zchange(task_zone, FALSE, FALSE, TRUE, TRUE);

/*	(void) task_create(TASK_NULL, FALSE, &kernel_task);*/
	kernel_task = (task_t) zalloc(task_zone);
	if (kernel_task == TASK_NULL)
		panic("task_init: could not create kernel task");

	kernel_task->ref_count = 1;
	kernel_task->map = kernel_map;
	simple_lock_init(&kernel_task->lock);
	ipc_task_init(kernel_task, TASK_NULL);
	ipc_task_enable(kernel_task);
}

kern_return_t task_create(parent_task, inherit_memory, child_task)
	task_t		parent_task;
	boolean_t	inherit_memory;
	task_t		*child_task;		/* OUT */
{
	register task_t	new_task;
	register processor_set_t	pset;

	new_task = (task_t) zalloc(task_zone);
	if (new_task == TASK_NULL) {
		printf("task zalloc failed");
		return(KERN_RESOURCE_SHORTAGE);
	}

	new_task->u_address = (struct utask *) zalloc(u_zone);	/* UNIX */
	new_task->ref_count = 1;

	if (inherit_memory)
		new_task->map = vm_map_fork(parent_task->map);
	else
		new_task->map = vm_map_create(pmap_create(0),
					round_page(VM_MIN_ADDRESS),
					trunc_page(VM_MAX_ADDRESS), TRUE);

        if (new_task->map == VM_MAP_NULL) {
                zfree(u_zone, new_task->u_address);
                zfree(task_zone, new_task);
                return KERN_RESOURCE_SHORTAGE;
        }

	simple_lock_init(&new_task->lock);
	queue_init(&new_task->thread_list);
	simple_lock_init(&new_task->thread_list_lock);
	new_task->suspend_count = 0;
	new_task->active = TRUE;
	new_task->user_stop_count = 0;
	new_task->thread_count = 0;
#if	MACH_EMULATION	
	new_task->eml_dispatch = EML_DISPATCH_NULL;
#endif	
	*child_task = new_task;

	new_task->kernel_ipc_space = FALSE;
	new_task->kernel_vm_space = FALSE;
	ipc_task_init(new_task, parent_task);

	new_task->total_user_time.seconds = 0;
	new_task->total_user_time.microseconds = 0;
	new_task->total_system_time.seconds = 0;
	new_task->total_system_time.microseconds = 0;

	if (parent_task != TASK_NULL) {
		task_lock(parent_task);
		pset = parent_task->processor_set;
		if (!pset->active)
			pset = &default_pset;
		new_task->priority = parent_task->priority;
	}
	else {
		pset = &default_pset;
		new_task->priority = BASEPRI_USER;
	}
	pset_lock(pset);
	pset_add_task(pset, new_task);
	pset_unlock(pset);
	if (parent_task != TASK_NULL)
		task_unlock(parent_task);

	new_task->may_assign = TRUE;
	new_task->assign_active = FALSE;

	ipc_task_enable(new_task);

	simple_lock(&task_inswapped_queue_lock);
	queue_enter(&task_inswapped_queue, new_task, task_t, task_link);
	simple_unlock(&task_inswapped_queue_lock);
	task_inswapped_queue_count++;
	new_task->swap_state = TASK_INSWAPPED;
	new_task->swap_request = TASK_ALL_SET;
	new_task->inswap_stamp = sched_tick;
	new_task->swap_nswap = 0;
	new_task->outswap_stamp = 0;
	new_task->working_set = 0;
	return(KERN_SUCCESS);
}

/*
 *	task_deallocate:
 *
 *	Give up a reference to the specified task and destroy it if there
 *	are no other references left.  It is assumed that the current thread
 *	is never in this task.
 */
void task_deallocate(task)
	register task_t	task;
{
	register int c;

	if (task == TASK_NULL)
		return;

	task_lock(task);
	c = --(task->ref_count);
	task_unlock(task);
	if (c != 0) {
		return;
	}
	else {
		register processor_set_t pset;

		pset = task->processor_set;
		pset_lock(pset);
		pset_remove_task(pset,task);
		pset_unlock(pset);
		pset_deallocate(pset);
		vm_map_deallocate(task->map);
		if (task->swap_state == TASK_INSWAPPED) {
			simple_lock(&task_inswapped_queue_lock);
			queue_remove(&task_inswapped_queue, task, task_t, task_link);
			task_inswapped_queue_count--;
			simple_unlock(&task_inswapped_queue_lock);
		}else if (task->swap_state == TASK_OUTSWAPPED) {
			simple_lock(&task_outswapped_queue_lock);
			queue_remove(&task_outswapped_queue, task, task_t, task_link);
			task_outswapped_queue_count--;
			simple_unlock(&task_outswapped_queue_lock);
		}else if (task->swap_state == TASK_COMMING_IN) {
			simple_lock(&task_inswap_work_queue_lock);
			queue_remove(&task_inswap_work_queue, task, task_t, task_link);
			task_inswap_work_queue_count--;
			simple_unlock(&task_inswap_work_queue_lock);
		}else if (task->swap_state == TASK_GOING_OUT) {
			simple_lock(&task_outswap_work_queue_lock);
			queue_remove(&task_outswap_work_queue, task, task_t, task_link);
			task_outswap_work_queue_count--;
			simple_unlock(&task_outswap_work_queue_lock);
		}else panic("task_deallocate: task not any swap queue");
		zfree(u_zone, (vm_offset_t) task->u_address);
		zfree(task_zone, (vm_offset_t) task);
	}
}

void task_reference(task)
	register task_t	task;
{
	if (task == TASK_NULL)
		return;

	task_lock(task);
	task->ref_count++;
	task_unlock(task);
}

/*
 *	task_terminate:
 *
 *	Terminate the specified task.  See comments on thread_terminate
 *	(kern/thread.c) about problems with terminating the "current task."
 */
kern_return_t task_terminate(task)
	register task_t	task;
{
	register thread_t	thread, cur_thread;
	register queue_head_t	*list;
	register task_t		cur_task;
	int			s;

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	list = &task->thread_list;
	cur_task = current_task();
	cur_thread = current_thread();

	/*
	 *	Deactivate task so that it can't be terminated again,
	 *	and so lengthy operations in progress will abort.
	 *
	 *	If the current thread is in this task, remove it from
	 *	the task's thread list to keep the thread-termination
	 *	loop simple.
	 */
	if (task == cur_task) {
		task_lock(task);
		if (!task->active) {
			/*
			 *	Task is already being terminated.
			 */
			task_unlock(task);
			return(KERN_FAILURE);
		}
		/*
		 *	Make sure current thread is not being terminated.
		 */
		s = splsched();
		simple_lock(&task->thread_list_lock);
		thread_lock(cur_thread);
		if (!cur_thread->active) {
			thread_unlock(cur_thread);
			simple_unlock(&task->thread_list_lock);
			(void) splx(s);
			task_unlock(task);
			thread_terminate(cur_thread);
			return(KERN_FAILURE);
		}
		task->active = FALSE;
		queue_remove(list, cur_thread, thread_t, thread_list);
		thread_unlock(cur_thread);
		simple_unlock(&task->thread_list_lock);
		(void) splx(s);
		task_unlock(task);

		/*
		 *	Shut down this thread's ipc now because it must
		 *	be left alone to terminate the task.
		 */
		ipc_thread_disable(cur_thread);
		ipc_thread_terminate(cur_thread);
	}
	else {
		/*
		 *	Lock both current and victim task to check for
		 *	potential deadlock.
		 */
		if ((int)task < (int)cur_task) {
			task_lock(task);
			task_lock(cur_task);
		}
		else {
			task_lock(cur_task);
			task_lock(task);
		}
		/*
		 *	Check if current thread or task is being terminated.
		 */
		s = splsched();
		thread_lock(cur_thread);
		if ((!cur_task->active) ||(!cur_thread->active)) {
			/*
			 * Current task or thread is being terminated.
			 */
			thread_unlock(cur_thread);
			(void) splx(s);
			task_unlock(task);
			task_unlock(cur_task);
			thread_terminate(cur_thread);
			return(KERN_FAILURE);
		}
		thread_unlock(cur_thread);
		(void) splx(s);
		task_unlock(cur_task);

		if (!task->active) {
			/*
			 *	Task is already being terminated.
			 */
			task_unlock(task);
			return(KERN_FAILURE);
		}
		task->active = FALSE;
		task_unlock(task);
	}

	/*
	 *	Prevent further execution of the task.  ipc_task_disable
	 *	prevents further task operations via the task port.
	 *	If this is the current task, the current thread will
	 *	be left running.
	 */
	ipc_task_disable(task);
	(void) task_hold(task);
	(void) task_dowait(task,TRUE);			/* may block */

	/*
	 *	Terminate each thread in the task.
	 *
	 *	The task_port is closed down, so no more thread_create
	 *	operations can be done.  Thread_force_terminate closes the
	 *	thread port for each thread; when that is done, the
	 *	thread will eventually disappear.  Thus the loop will
	 *	terminate.  Call thread_force_terminate instead of
	 *	thread_terminate to avoid deadlock checks.  Need
	 *	to call thread_block() inside loop because some other
	 *	thread (e.g., the reaper) may have to run to get rid
	 *	of all references to the thread; it won't vanish from
	 *	the task's thread list until the last one is gone.
	 */
	task_lock(task);
	while (!queue_empty(list)) {
		thread = (thread_t) queue_first(list);
		thread_reference(thread);
		task_unlock(task);
		thread_force_terminate(thread);
		thread_deallocate(thread);
		thread_block();
		task_lock(task);
	}
	task_unlock(task);

	/*
	 *	Shut down IPC.
	 */
	ipc_task_terminate(task);

#if	MACH_EMULATION
	eml_task_exit(task);
#endif

	/*
	 *	Deallocate the task's reference to itself.
	 */
	task_deallocate(task);

	/*
	 *	If the current thread is in this task, it has not yet
	 *	been terminated (since it was removed from the task's
	 *	thread-list).  Put it back in the thread list (for
	 *	completeness), and terminate it.  Since it holds the
	 *	last reference to the task, terminating it will deallocate
	 *	the task.
	 */
	if (cur_thread->task == task) {
		task_lock(task);
		s = splsched();
		simple_lock(&task->thread_list_lock);
		queue_enter(list, cur_thread, thread_t, thread_list);
		simple_unlock(&task->thread_list_lock);
		(void) splx(s);
		task_unlock(task);
		(void) thread_terminate(cur_thread);
	}

	return(KERN_SUCCESS);
}

/*
 *	task_hold:
 *
 *	Suspend execution of the specified task.
 *	This is a recursive-style suspension of the task, a count of
 *	suspends is maintained.
 */
kern_return_t task_hold(task)
	register task_t	task;
{
	register queue_head_t	*list;
	register thread_t	thread, cur_thread;

	cur_thread = current_thread();

	task_lock(task);
	if (!task->active) {
		task_unlock(task);
		return(KERN_FAILURE);
	}

	task->suspend_count++;

	/*
	 *	Iterate through all the threads and hold them.
	 *	Do not hold the current thread if it is within the
	 *	task.
	 */
	list = &task->thread_list;
	thread = (thread_t) queue_first(list);
	while (!queue_end(list, (queue_entry_t) thread)) {
		if (thread != cur_thread)
			thread_hold(thread);
		thread = (thread_t) queue_next(&thread->thread_list);
	}
	task_unlock(task);
	return(KERN_SUCCESS);
}

/*
 *	task_dowait:
 *
 *	Wait until the task has really been suspended (all of the threads
 *	are stopped).  Skip the current thread if it is within the task.
 *
 *	If task is deactivated while waiting, return a failure code unless
 *	must_wait is true.
 */
kern_return_t task_dowait(task, must_wait)
	register task_t	task;
	boolean_t must_wait;
{
	register queue_head_t	*list;
	register thread_t	thread, cur_thread, prev_thread;
	register kern_return_t	ret = KERN_SUCCESS;

	/*
	 *	Iterate through all the threads.
	 *	While waiting for each thread, we gain a reference to it
	 *	to prevent it from going away on us.  This guarantees
	 *	that the "next" thread in the list will be a valid thread.
	 *
	 *	We depend on the fact that if threads are created while
	 *	we are looping through the threads, they will be held
	 *	automatically.  We don't care about threads that get
	 *	deallocated along the way (the reference prevents it
	 *	from happening to the thread we are working with).
	 *
	 *	If the current thread is in the affected task, it is skipped.
	 *
	 *	If the task is deactivated before we're done, and we don't
	 *	have to wait for it (must_wait is FALSE), just bail out.
	 */
	cur_thread = current_thread();

	list = &task->thread_list;
	prev_thread = THREAD_NULL;
	task_lock(task);
	thread = (thread_t) queue_first(list);
	while (!queue_end(list, (queue_entry_t) thread)) {
		if (!(task->active) && !(must_wait)) {
			ret = KERN_FAILURE;
			break;
		}
		if (thread != cur_thread) {
			thread_reference(thread);
			task_unlock(task);
			if (prev_thread != THREAD_NULL)
				thread_deallocate(prev_thread);
							/* may block */
			(void) thread_dowait(thread, TRUE);  /* may block */
			prev_thread = thread;
			task_lock(task);
		}
		thread = (thread_t) queue_next(&thread->thread_list);
	}
	task_unlock(task);
	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);		/* may block */
	return(ret);	
}

kern_return_t task_release(task)
	register task_t	task;
{
	register queue_head_t	*list;
	register thread_t	thread, next;

	task_lock(task);
	if (!task->active) {
		task_unlock(task);
		return(KERN_FAILURE);
	}

	task->suspend_count--;

	/*
	 *	Iterate through all the threads and release them
	 */
	list = &task->thread_list;
	thread = (thread_t) queue_first(list);
	while (!queue_end(list, (queue_entry_t) thread)) {
		next = (thread_t) queue_next(&thread->thread_list);
		thread_release(thread);
		thread = next;
	}
	task_unlock(task);
	return(KERN_SUCCESS);
}

/*
 *	task_halt:
 *
 *	Halt all threads in the task.  Do not halt the current thread if
 *	it is within the task.
 *
 *	Only called from exit().
 */
kern_return_t task_halt(task)
	register task_t	task;
{
	register queue_head_t	*list;
	register thread_t	thread, cur_thread, prev_thread;
	register kern_return_t	ret = KERN_SUCCESS;

	/*
	 *	Iterate through all the threads.
	 *	While waiting for each thread, we gain a reference to it
	 *	to prevent it from going away on us.  This guarantees
	 *	that the "next" thread in the list will be a valid thread.
	 *
	 *	If the current thread is in the affected task, it is skipped.
	 */
	cur_thread = current_thread();

	list = &task->thread_list;
	prev_thread = THREAD_NULL;
	task_lock(task);
	thread = (thread_t) queue_first(list);
	while (!queue_end(list, (queue_entry_t) thread)) {
		if (thread != cur_thread) {
			thread_reference(thread);
			task_unlock(task);
			if (prev_thread != THREAD_NULL)
			    thread_deallocate(prev_thread); /* may block */
#if	MACH_HOST
			thread_freeze(thread);
			if (thread->processor_set != &default_pset)
			    thread_doassign(thread, &default_pset, FALSE);
#endif
			thread_halt(thread, TRUE); /* may block */
#if	MACH_HOST
			thread_unfreeze(thread);
#endif
			prev_thread = thread;
			task_lock(task);
		}
		thread = (thread_t) queue_next(&thread->thread_list);
	}
	task_unlock(task);
	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);		/* may block */
	return(ret);	
}

kern_return_t task_threads(task, thread_list, count)
	task_t		task;
	thread_array_t	*thread_list;
	unsigned int	*count;
{
	unsigned int actual;	/* this many threads */
	thread_t thread;
	port_t *threads;
	int i;

	vm_size_t size;
	vm_offset_t addr;

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	size = 0; addr = 0;

	for (;;) {
		vm_size_t size_needed;

		task_lock(task);
		if (!task->active) {
			task_unlock(task);
			return KERN_FAILURE;
		}

		actual = task->thread_count;

		/* do we have the memory we need? */

		size_needed = actual * sizeof(port_t);
		if (size_needed <= size)
			break;

		/* unlock the task and allocate more memory */
		task_unlock(task);

		if (size != 0)
			(void) kmem_free(ipc_kernel_map, addr, size);

		size = round_page(2 * size_needed);

		/* allocate memory non-pageable, so don't fault
		   while holding locks */

		if (vm_allocate(ipc_kernel_map, &addr, size, TRUE) != KERN_SUCCESS)
			return KERN_FAILURE;
		if (vm_map_pageable(ipc_kernel_map, addr, addr + size,
			VM_PROT_READ|VM_PROT_WRITE) != KERN_SUCCESS) {
			(void) kmem_free(ipc_kernel_map, addr, size);
			return KERN_FAILURE;
		}
	}

	/* OK, have memory and the task is locked & active */

	threads = (port_t *) addr;

	for (i = 0, thread = (thread_t) queue_first(&task->thread_list);
	     i < actual;
	     i++, thread = (thread_t) queue_next(&thread->thread_list))
		threads[i] = convert_thread_to_port(thread);
	assert(queue_end(&task->thread_list, (queue_entry_t) thread));

	/* can unlock task now that we've got the thread ports */
	task_unlock(task);

	if (actual == 0) {
		/* no threads, so return null pointer and deallocate memory */
		*thread_list = 0;
		*count = 0;

		if (size != 0)
			(void) kmem_free(ipc_kernel_map, addr, size);
	} else {
		vm_size_t size_used;

		*thread_list = threads;
		*count = actual;

		size_used = round_page(actual * sizeof(thread_t));

		/* finished touching it, so make the memory pageable */
		(void) vm_map_pageable(ipc_kernel_map,
				       addr, addr + size_used, VM_PROT_NONE);

		/* free any unused memory */
		if (size_used != size)
			(void) kmem_free(ipc_kernel_map,
					 addr + size_used, size - size_used);
	}

	return KERN_SUCCESS;
}

kern_return_t task_suspend(task)
	register task_t	task;
{
	register boolean_t	hold;

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	hold = FALSE;
	task_lock(task);
	if ((task->user_stop_count)++ == 0)
		hold = TRUE;
	task_unlock(task);

	/*
	 *	If the stop count was positive, the task is
	 *	already stopped and we can exit.
	 */
	if (!hold) {
		return (KERN_SUCCESS);
	}

	/*
	 *	Hold all of the threads in the task, and wait for
	 *	them to stop.  If the current thread is within
	 *	this task, hold it separately so that all of the
	 *	other threads can stop first.
	 */

	if (task_hold(task) != KERN_SUCCESS)
		return(KERN_FAILURE);

	if (task_dowait(task, FALSE) != KERN_SUCCESS)
		return(KERN_FAILURE);

	if (current_task() == task) {
		thread_hold(current_thread());
		(void) thread_dowait(current_thread(), TRUE);
	}

	return(KERN_SUCCESS);
}

kern_return_t task_resume(task)
	register task_t	task;
{
	register boolean_t	release;

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	release = FALSE;
	task_lock(task);
	if (task->user_stop_count > 0) {
		if (--(task->user_stop_count) == 0)
	    		release = TRUE;
	}
	else {
		task_unlock(task);
		return(KERN_FAILURE);
	}
	task_unlock(task);

	/*
	 *	Release the task if necessary.
	 */
	if (release)
		return(task_release(task));

	return(KERN_SUCCESS);
}

kern_return_t
task_info(
	task_t			task,
	int			flavor,
	task_info_t		task_info_out,	/* pointer to OUT array */
	unsigned int		*task_info_count )	/* IN/OUT */
{
	task_basic_info_t	basic_info;
	task_events_info_t	events_info;
	vm_map_t		map;

	extern task_t		first_task;	/* kernel task */

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	switch (flavor) {
	case TASK_BASIC_INFO:
		if (*task_info_count < TASK_BASIC_INFO_COUNT)
			return(KERN_INVALID_ARGUMENT);

		basic_info = (task_basic_info_t) task_info_out;

		map = (task == first_task) ? kernel_map : task->map;

	basic_info->virtual_size = vm_map_actual_size (map,
		(task == first_task) ? 1 : 0);
	basic_info->resident_size = pmap_resident_count(map->vm_pmap)
					   * PAGE_SIZE;
		basic_info->base_priority =
		(task == first_task) ? BASEPRI_SYSTEM : BASEPRI_USER;
				/* may change later XXX */

		task_lock(task);
		basic_info->suspend_count = task->user_stop_count;
		basic_info->user_time.seconds = task->total_user_time.seconds;
		basic_info->user_time.microseconds
				= task->total_user_time.microseconds;
		basic_info->system_time.seconds
				= task->total_system_time.seconds;
		basic_info->system_time.microseconds 
				= task->total_system_time.microseconds;
		task_unlock(task);

		*task_info_count = TASK_BASIC_INFO_COUNT;
		return(KERN_SUCCESS);
		break;

	case TASK_EVENTS_INFO:
	case TASK_ALL_EVENTS_INFO:
		if (*task_info_count < TASK_EVENTS_INFO_COUNT)
			return(KERN_INVALID_ARGUMENT);

		events_info = (task_events_info_t) task_info_out;

		/*
		 * lock against thread creation/termination.
		 */
		task_lock(task);

		*task_info_count = TASK_EVENTS_INFO_COUNT;
		*events_info = task->task_events;

		if (flavor == TASK_ALL_EVENTS_INFO) {
		    register queue_head_t	*list;
		    register thread_t	thread;

		    list = &task->thread_list;
		    thread = (thread_t) queue_first(list);
		    while (!queue_end(list, (queue_entry_t) thread)) {
			events_info_add(events_info, &thread->thread_events);
			thread = (thread_t) queue_next(&thread->thread_list);
		    }
		}
		task_unlock(task);

		return(KERN_SUCCESS);

	default:
		return(KERN_INVALID_ARGUMENT);
	}
}

/*
 * Below is a groty hack, but means that we don't have to keep changing
 * this code every time an item gets added to the events_info.  If for
 * some reason a structure containing a bunch of longs can't be treated
 * like an array of longs, this routine has to change.
 */
void
events_info_add( struct events_info *devents, struct events_info *sevents)
{
	register long *dp = (long *)devents;
	register long *sp = (long *)sevents;
	long *ep = dp + TASK_EVENTS_INFO_COUNT;

	while (dp < ep) *dp++ += *sp++;
}

/*
 *	Special version of task_suspend that doesn't wait.
 *	Called only from interrupt level (U*X psignal).
 *	Will go away when signal code becomes sane.
 */
kern_return_t task_suspend_nowait(task)
	register task_t	task;
{
	register boolean_t	hold;

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	hold = FALSE;
	task_lock(task);
	if ((task->user_stop_count)++ == 0)
		hold = TRUE;
	task_unlock(task);

	/*
	 *	If the stop count was positive, the task is
	 *	already stopped and we can exit.
	 */
	if (!hold) {
		return (KERN_SUCCESS);
	}

	/*
	 *	Hold all of the threads in the task.
	 *	If the current thread is within
	 *	this task, hold it separately so that all of the
	 *	other threads can stop first.
	 */

	if (task_hold(task) != KERN_SUCCESS)
		return(KERN_FAILURE);

	if (current_task() == task) {
		thread_hold(current_thread());
	}

	return(KERN_SUCCESS);
}

kern_return_t task_get_special_port(task, which_port, port)
	task_t		task;
	int		which_port;
	task_port_t	*port;
{
	task_port_t	*portp;

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	switch (which_port) {
	    case TASK_KERNEL_PORT:
		portp = &task->task_tself;
		break;
	    case TASK_NOTIFY_PORT:
		portp = &task->task_notify;
		break;
	    case TASK_EXCEPTION_PORT:
		portp = &task->exception_port;
		break;
	    case TASK_BOOTSTRAP_PORT:
		portp = &task->bootstrap_port;
		break;
	    default:
		return(KERN_INVALID_ARGUMENT);
	}

	ipc_task_lock(task);
	if (!task->ipc_active) {
		ipc_task_unlock(task);
		return(KERN_FAILURE);
	}
	
	if ((*port = *portp) != PORT_NULL) {
		port_reference(*portp);
	}
	ipc_task_unlock(task);

	return(KERN_SUCCESS);
}

kern_return_t task_set_special_port(task, which_port, port)
	task_t		task;
	int		which_port;
	task_port_t	port;
{
	task_port_t		*portp;
	task_port_t		old_port;

	if (task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	switch (which_port) {
	    case TASK_KERNEL_PORT:
		portp = &task->task_tself;
		break;
	    case TASK_NOTIFY_PORT:
		portp = &task->task_notify;
		break;
	    case TASK_EXCEPTION_PORT:
		portp = &task->exception_port;
		break;
	    case TASK_BOOTSTRAP_PORT:
		portp = &task->bootstrap_port;
		break;
	    default:
		return(KERN_INVALID_ARGUMENT);
	}

	ipc_task_lock(task);
	if (!task->ipc_active) {
		ipc_task_unlock(task);
		return(KERN_FAILURE);
	}
	
	old_port = *portp;
	if ((*portp = port) != PORT_NULL)
		port_reference(port);

	ipc_task_unlock(task);

	if (old_port != PORT_NULL)
		port_release(old_port);

	return(KERN_SUCCESS);
}

#if	MACH_HOST
/*
 *	task_assign:
 *
 *	Change the assigned processor set for the task
 */
kern_return_t
task_assign(task, new_pset, assign_threads)
task_t	task;
processor_set_t	new_pset;
boolean_t	assign_threads;
{
	kern_return_t		ret = KERN_SUCCESS;
	register thread_t	thread, prev_thread;
	register queue_head_t	*list;
	register processor_set_t	pset;

	if (task == TASK_NULL || new_pset == PROCESSOR_SET_NULL) {
		return(KERN_INVALID_ARGUMENT);
	}

	task_lock(task);

	/*
	 *	If may_assign is false, task is already being assigned,
	 *	wait for that to finish.
	 */
	while (task->may_assign == FALSE) {
		task->assign_active = TRUE;
		assert_wait((vm_offset_t)&task->assign_active, TRUE);
		task_unlock(task);
		thread_block();
		task_lock(task);
	}

	/*
	 *	Do assignment.
	 */
	pset = task->processor_set;
	pset_lock(pset);
	pset_remove_task(pset,task);
	pset_unlock(pset);
	pset_deallocate(pset);
	pset_lock(new_pset);
	pset_add_task(new_pset,task);
	pset_unlock(new_pset);

	if (assign_threads == FALSE) {
		task_unlock(task);
		return(KERN_SUCCESS);
	}
	    
	/*
	 *	Now freeze this assignment while we get the threads
	 *	to catch up to it.
	 */
	task->may_assign = FALSE;

	/*
	 *	If current thread is in task, freeze its assignment.
	 */
	if (current_thread()->task == task) {
		task_unlock(task);
		thread_freeze(current_thread());
		task_lock(task);
	}

	/*
	 *	Iterate down the thread list reassigning all the threads.
	 *	New threads pick up task's new processor set automatically.
	 *	Do current thread last because new pset may be empty.
	 */
	list = &task->thread_list;
	prev_thread = THREAD_NULL;
	thread = (thread_t) queue_first(list);
	while (!queue_end(list, (queue_entry_t) thread)) {
		if (!(task->active)) {
			ret = KERN_FAILURE;
			break;
		}
		if (thread != current_thread()) {
			thread_reference(thread);
			task_unlock(task);
			if (prev_thread != THREAD_NULL)
			    thread_deallocate(prev_thread); /* may block */
			thread_assign(thread,new_pset);	    /* may block */
			prev_thread = thread;
			task_lock(task);
		}
		thread = (thread_t) queue_next(&thread->thread_list);
	}

	/*
	 *	Done, wakeup anyone waiting for us.
	 */
	task->may_assign = TRUE;
	if (task->assign_active) {
		task->assign_active = FALSE;
		thread_wakeup((vm_offset_t)&task->may_assign);
	}
	task_unlock(task);
	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);		/* may block */

	/*
	 *	Finish assignment of current thread.
	 */
	if (current_thread()->task == task)
		thread_doassign(current_thread(), new_pset, TRUE);

	return(ret);
}
#else	/* MACH_HOST */
/*
 *	task_assign:
 *
 *	Change the assigned processor set for the task
 */
kern_return_t
task_assign(task, new_pset, assign_threads)
task_t	task;
processor_set_t	new_pset;
boolean_t	assign_threads;
{
#ifdef	lint
	task++; new_pset++; assign_threads++;
#endif	
	return(KERN_FAILURE);
}
#endif	/* MACH_HOST */
	

/*
 *	task_assign_default:
 *
 *	Version of task_assign to assign to default processor set.
 */
kern_return_t
task_assign_default(task, assign_threads)
task_t		task;
boolean_t	assign_threads;
{
    return (task_assign(task, &default_pset, assign_threads));
}

/*
 *	task_get_assignment
 *
 *	Return name of processor set that task is assigned to.
 */
kern_return_t task_get_assignment(task, pset)
task_t	task;
processor_set_t	*pset;
{
	if (!task->active)
		return(KERN_FAILURE);

	*pset = task->processor_set;
	return(KERN_SUCCESS);
}

/*
 *	task_priority
 *
 *	Set priority of task; used only for newly created threads.
 *	Optionally change priorities of threads.
 */
kern_return_t
task_priority(task, priority, change_threads)
task_t		task;
int		priority;
boolean_t	change_threads;
{
	kern_return_t	ret = KERN_SUCCESS;

	if (task == TASK_NULL || invalid_pri(priority))
		return(KERN_INVALID_ARGUMENT);

	task_lock(task);
	task->priority = priority;

	if (change_threads) {
		register thread_t	thread;
		register queue_head_t	*list;

		list = &task->thread_list;
		thread = (thread_t) queue_first(list);
		while (!queue_end(list, (queue_entry_t) thread)) {

			if (thread_priority(thread, priority, FALSE, TRUE)
				!= KERN_SUCCESS)
					ret = KERN_FAILURE;
			thread = (thread_t) queue_next(&thread->thread_list);
		}
	}

	task_unlock(task);
	return(ret);
}
