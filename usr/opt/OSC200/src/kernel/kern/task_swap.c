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
static char *rcsid = "@(#)$RCSfile: task_swap.c,v $ $Revision: 1.1.22.5 $ (DEC) $Date: 1993/09/22 18:28:35 $";
#endif
#ifndef lint
static char     *sccsid = "@(#)task_swap.c    3.2     (ULTRIX/OSF)   11/20/91";
#endif
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.1
 */
/*
 *
 *      File:   kern/task_swap.c
 *      Author: Larry Woodman
 *
 *      Copyright (C) 1987, Avadis Tevanian, Jr. and Richard F. Rashid
 *
 *      Mach task swapper:
 */

#include <kern/thread.h>
#include <kern/lock.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/vm_pagelru.h>
#include <mach/vm_param.h>
#include <kern/sched_prim.h>
#include <kern/processor.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/thread_swap.h>
#include <kern/task_swap.h>
#include <sys/proc.h>
#include <vm/vm_perf.h>
#include <vm/vm_tune.h>
#include <machine/machparam.h>          /* for splsched */
#include <vm/pmap.h>
#include <sys/sched_mon.h>
#include <sys/kernel.h>

#define	default_swapper_wakeup hz*5		/* default at 15 seconds */
#define inswap_request_swapper_wakeup hz*1	/* if anyone wants in 1 second */ 

int		task_swap_work;
time_t		task_swap_time;

queue_head_t	task_inswapped_queue;
queue_head_t	task_outswapped_queue;
queue_head_t	task_inswap_work_queue;
queue_head_t	task_outswap_work_queue;

decl_simple_lock_data(, task_inswapped_queue_lock)
decl_simple_lock_data(, task_outswapped_queue_lock)
decl_simple_lock_data(, task_inswap_work_queue_lock)
decl_simple_lock_data(, task_outswap_work_queue_lock)
decl_simple_lock_data(, task_swap_scan_lock)

int		task_inswapped_queue_count;
int		task_outswapped_queue_count;
int 		task_inswap_work_queue_count;
int 		task_outswap_work_queue_count;
int		inswap=0;
int		outswap=0;
boolean_t	task_swap_scan = FALSE;

int		 minimum_task_inswapped_ticks;

/*
 *      task_swapper_init: [exported]
 *
 *      Initialize the swapper module.
 */
void task_swapper_init()
{
		/* initialize the four task swap queues */
        queue_init(&task_inswapped_queue);
	queue_init(&task_outswapped_queue);
	queue_init(&task_inswap_work_queue);
	queue_init(&task_outswap_work_queue);

		/* initialize the four task queue locks and the swap mutex lock */
        simple_lock_init(&task_inswapped_queue_lock);
	simple_lock_init(&task_outswapped_queue_lock);
	simple_lock_init(&task_inswap_work_queue_lock);
	simple_lock_init(&task_outswap_work_queue_lock);
	simple_lock_init(&task_swap_scan_lock);

		/* indicate that the four task queues are empty */
        task_inswapped_queue_count=0;
        task_outswapped_queue_count=0;
        task_inswap_work_queue_count=0;
        task_outswap_work_queue_count=0;

		/* remember the last time that a task swap event occurred */
	task_swap_time = sched_tick;

		/* get the tunable number of ticks a task must be inswapped before it can go out */
	minimum_task_inswapped_ticks = vm_tune_value(inswappedmin);

		/* init the last zone garbage collect time as now
	vm_zone_gc_time = time.tv_sec;

		/* set up a task timer event */
	(void)task_swapper_timeout(TRUE);	

}

boolean_t
task_swapout_condition()
{
	extern int vmlfree_ave, vm_page_free_optimal, vm_page_free_target, 
		vm_pageout_burst_max, vm_page_free_min;

		/* if long-term free avg is less below free_optimal we are paging heavily, do some swapping */
	if ((MAX(vmlfree_ave,vpf_cload(freepages)) < vm_page_free_optimal) &&
		(((vpf_cload(pgiowrites) +  vpf_cload(pgioreads) +
			vpf_cload(ubcpushes)) >= vm_pageout_burst_max) ||
		(vpf_cload(freepages) < vm_page_free_min))) return TRUE;
	else return FALSE;
}


/*      task_swapout:
 *
 *      Swap out this task.  This routine halts all the threads in the task,
 *      by calling task_hold(), removes the task from the task_inswapped_queue,
 *      loops through all threads in the task calling thread_swapout(),
 *      calls vm_map_swapout() which frees up the physical memory which is
 *      allocated to the address space and finally puts the task on the
 *      task_outswapped_queue.  Thread kernel stack unwiring
 *      and pmap_collect()ing is done by thread_swapout.  Task_swapout() only
 *      frees physical memory that has been allocated to a task's address space.
 *      This does not imply reclaiming task, thread or vm data structures allocated
 *      to the task.
 */

pid_t exclude_pids[] = {0, 1, 2, 3};
char swap_kill_message[] = 
	"process (pid = %d) killed because of no swap space\n";

kern_return_t task_swapout(task)
        register task_t task;
{
        register        thread_t        thread, last_thread;
        register        kern_return_t   ret = KERN_SUCCESS;
        register        queue_head_t    *list;
        int     	first_pass = 0, working_set;
	boolean_t	task_kill = FALSE;
	boolean_t	swapit = TRUE;
	extern vm_size_t vm_swap_space;

		/* stop the task */
	task_hold(task);
	task_dowait(task,TRUE);
        task_lock(task);
		/* if the task is going away, put it back in the inswapped queue and let it go */
	if ((task->thread_count == 0) || (!task->active)) {
		simple_lock(&task_inswapped_queue_lock);
		queue_enter(&task_inswapped_queue, task, task_t, task_link);
		task_inswapped_queue_count++;
		task->swap_state = TASK_INSWAPPED;
		simple_unlock(&task_inswapped_queue_lock);
		task_unlock(task);
		task_deallocate(task);
		task_release(task);
		return ret;
	}
	simple_lock(&task->thread_list_lock);
        list = &task->thread_list;
        thread = (thread_t) queue_first(list);
	last_thread = THREAD_NULL;
        while (!queue_end(list, (queue_entry_t) thread)){
                thread_lock(thread);
			/* swap the threads that are currently in and stopped */
                if(!(thread->state & TH_SWAPPED) &&
		   !(thread->state & TH_RUN) &&
                   (thread->swap_state == TH_SW_IN)){
                        thread->state |= TH_SWAPPED;
                        thread->swap_state = TH_SW_GOING_OUT;
			thread_reference(thread);
			thread_unlock(thread);
			simple_unlock(&task->thread_list_lock);
			task_unlock(task);
			if (last_thread != THREAD_NULL)
                                thread_deallocate(last_thread);
			last_thread = thread;
                        thread_swapout(thread, (++first_pass == 1));
			task_lock(task);
                        simple_lock(&task->thread_list_lock);
                } else {
				/* dont deal with tasks which have threads that are still out */
                	thread_unlock(thread);
			swapit = FALSE;
		}	
                thread = (thread_t) queue_next(&thread->thread_list);
        }
	if (last_thread != THREAD_NULL) thread_deallocate(last_thread);
	simple_unlock(&task->thread_list_lock);
		/* if all the threads got swapped out and we can quickly grab the map lock, swap the task */
	if(swapit && (lock_try_write(&task->map->vm_lock)))  {
		working_set = 0;
		task_unlock(task);
			/* swap out the address space and record the number of pages pushed */
        	ret = vm_map_swapout(task->map, &working_set);
		task_lock(task);
		task->working_set = working_set;
		vm_map_unlock(task->map);
		if (ret != KERN_SUCCESS) {
				/* if it failed because swap space was exhausted, kill this task */
			task_kill = TRUE;
		}
		else {
				/* if it succeeded, put the task on the outswapped queue and release it */
			vpf_add(taskswapouts,1);
	        	simple_lock(&task_outswapped_queue_lock);
       		 	queue_enter(&task_outswapped_queue, task, task_t, 
				task_link);
			task_outswapped_queue_count++;
       		 	simple_unlock(&task_outswapped_queue_lock);
			task->swap_state = TASK_OUTSWAPPED;
			task->outswap_stamp = sched_tick;
			task->swap_nswap++;
			task_unlock(task);
			task_release(task);
			return ret;
		}
	}
		/* if we decided not to swap the task body, put it bace on the inswapped queue */
	simple_lock(&task_inswapped_queue_lock);
	queue_enter(&task_inswapped_queue, task, task_t, task_link);
	task_inswapped_queue_count++;
	simple_unlock(&task_inswapped_queue_lock);
	task->swap_state = TASK_INSWAPPED;
	task_unlock(task);
	task_release(task);

		/* if we ran out of swap space while pushing out the address space SIGKILL the process */
	if (task_kill == TRUE && vm_swap_space == 0) {
		register struct proc *p;
		register int i;
		register pid_t *pid;

			/* if this task cant be killed, just bail out without attempting */
		p = &proc[task->proc_index];
		for (i = (sizeof exclude_pids) / (sizeof exclude_pids[0]), 
			pid = exclude_pids; i; i--, pid++)
			if (*pid == p->p_pid) {
				task_deallocate(task);
				return ret;
			}

			/* tell the world why the process is dieing and kill it */
		aprintf(swap_kill_message, p->p_pid);
		p->p_flag |= SSWPKIL;
		psignal(p, SIGKILL);
		task_deallocate(task);
		thread_set_timeout(hz);
		thread_block();
	} else task_deallocate(task);
        return(ret);
}

/*      task_swapin:
 *
 *      Swap in this task.  Inswapping a task consists of removing the task
 *      from the task_outswapped_queue, looping through the threads in the
 *      task calling thread_swapin() for each one, calling vm_map_swapin()
 *      putting the task on the task_inswapped_queue and finally calling
 *      deallocating the reference to the task.  The individual threads will
 *      fault the actual data back in when the task runs again.  
 */

kern_return_t task_swapin(task)
        register task_t task;
{
        register        thread_t        thread, last_thread;
        register        kern_return_t   ret = KERN_SUCCESS;
        register        queue_head_t    *list;

	vpf_add(taskswapins,1);
        task_lock(task);
	vm_map_lock(task->map);
		/* swap in the address space of the task */
        vm_map_swapin(task->map);
	vm_map_unlock(task->map);
        simple_lock(&task_inswapped_queue_lock);
		/* Put the task on the list of inswapped tasks */
        queue_enter(&task_inswapped_queue, task, task_t, task_link);
	task_inswapped_queue_count++;
        simple_unlock(&task_inswapped_queue_lock);
		/* tell the world that the task and its address space in in */
	task->swap_state = TASK_INSWAPPED;
		/* record when the task was last inswapped */
	task->inswap_stamp = sched_tick;
	simple_lock(&task->thread_list_lock);
	thread=(thread_t)queue_first(&task->thread_list);
	last_thread = THREAD_NULL;
		/* loop through all the threads of the task, inswapping the runnable ones */
	while (!queue_end((vm_offset_t)(&task->thread_list),(vm_offset_t)thread)) {
		if ((thread->state & TH_RUN) &&
		    (thread->swap_state == TH_SW_OUT)) {
			thread->swap_state = TH_SW_COMING_IN;
			thread_reference(thread);
			simple_unlock(&task->thread_list_lock);
			task_unlock(task);
			if (last_thread != THREAD_NULL)
				thread_deallocate(last_thread);
			last_thread = thread;
				/* Wire the kernel stack of the thread and make it runnable */
			thread_doswapin(thread);
			task_lock(task);
			simple_lock(&task->thread_list_lock);
		}
		thread=(thread_t)queue_next(&thread->thread_list);
	}
	if (last_thread != THREAD_NULL) thread_deallocate(last_thread);
	simple_unlock(&task->thread_list_lock);
	task_unlock(task);
	task_deallocate(task);
        return(ret);
}

#define VM_ZONE_GC_DELTA        60              /* 60 seconds between garbage collections */

long vm_zone_gc = 0;		/* the number of times that zone garbage collection has occurred */
long vm_zone_gc_time = 0;	/* last time zone garbage collection occurred */
long vm_zone_gc_delta = VM_ZONE_GC_DELTA;	/* min time between garbage collections */

#define ZONE_GC() 								\
	if (((time.tv_sec - vm_zone_gc_time) > vm_zone_gc_delta)) {     	\
        	vm_zone_gc++;                                                   \
        	zone_gc();                                                      \
        	vm_zone_gc_time = time.tv_sec;                                  \
	}



/*
 *      task_swapper_thread: [exported]
 *
 *	task_swapper_thread() is the endless loop of the task swapper kernel thread.  This loop
 *	calls task_swapout_condition() to determine if swapping is necessary, calls 
 *	swapout_victim_tasks() to queue up all the swap victims on the task_outswap_work_queue
 *	and swaps out those victim tasks.  After it has swapped out all the swap victims, it 
 *	swaps in any tasks that are on the task_inswap_work_queue.  Finally it goes back to 
 *	sleep until another swap event is needed.
 *	
 */

void task_swapper_thread_loop()
{
	register swap_out;

	while(TRUE){

                register task_t	task, inswap_task;
                register int    s, available_pages;

		swap_out = TRUE;

		ZONE_GC();

		s = splsched();

                simple_unlock(&task_outswap_work_queue_lock);
                task_swap_scan = TRUE;
                simple_unlock(&task_swap_scan_lock);
   
	/* Swap out any tasks that have been slated by swapout_victim_tasks() */

		if (task_swapout_condition() && swapout_victim_tasks()) {

				/* if swapout_victim_tasks() found any victims, swap them out */

			simple_lock(&task_outswap_work_queue_lock);
			while (!queue_empty(&task_outswap_work_queue)) {
				task=(task_t)queue_first(&task_outswap_work_queue);
				queue_remove(&task_outswap_work_queue, task, task_t, task_link);
				task_outswap_work_queue_count--;
	 			simple_unlock(&task_outswap_work_queue_lock);
					/* if we still need to swap based on current memory status, do it */
				if (swap_out == TRUE) {
					task_swapout(task);
						/* reevaluate the current memory situation */
					swap_out = task_swapout_condition();
				}
					/* if previous swapping improved the low memory condition, back off */
				else {
					simple_lock(&task_inswapped_queue_lock);
						/* put the tasks back on inswapped queue and let them run */
					queue_enter(&task_inswapped_queue, task, 
						task_t, task_link);
					task_inswapped_queue_count++;
					task->swap_state = TASK_INSWAPPED;
					simple_unlock(&task_inswapped_queue_lock);
					task_deallocate(task);
				}
				simple_lock(&task_outswap_work_queue_lock);
			}
			simple_unlock(&task_outswap_work_queue_lock);
		}

	/* Bring in as many tasks as we can that have been put on the inswap work queue by task_swapin_request() */

                simple_lock(&task_inswap_work_queue_lock);
                while (!queue_empty(&task_inswap_work_queue) && (vm_page_free_count >= vm_page_free_min)) {
			task=(task_t)queue_first(&task_inswap_work_queue);
			queue_remove(&task_inswap_work_queue, task, task_t, task_link);
			task_inswap_work_queue_count--;
                        simple_unlock(&task_inswap_work_queue_lock);
                       	task_swapin(task);
                        simple_lock(&task_inswap_work_queue_lock);
                }
                simple_unlock(&task_inswap_work_queue_lock);

                simple_lock(&task_swap_scan_lock);
                task_swap_scan = FALSE;
                simple_unlock(&task_swap_scan_lock);

		assert_wait((vm_offset_t) &task_swap_work, FALSE);
		splx(s);
		thread_block();

	}
}

/*
 *	swapout_victim_tasks:
 *
 */	
int	swapout_victim_tasks()
{
	register task_t task, swap_victim;
	register thread_t thread;
	int		got_one = 0;
	boolean_t	swap_it;
        time_t          delta_ticks;
	int		average_vm_fault_rate=0;

	/* First of all calculate the page fault rates for all inswapped tasks */

        simple_lock(&task_inswapped_queue_lock);
        task=(task_t)queue_first(&task_inswapped_queue);
        while (!queue_end((vm_offset_t)&task_inswapped_queue, (vm_offset_t)task)) {
                task_lock(task);
                delta_ticks = sched_tick-task->map->vm_faultrate_time;
                task->map->vm_fault_rate = ((delta_ticks) ?
                                (task->map->vm_pagefaults/delta_ticks) : 0);
                average_vm_fault_rate += task->map->vm_fault_rate;
                task->map->vm_pagefaults = 0;
                task->map->vm_faultrate_time = sched_tick;
                task_unlock(task);
                task = (task_t)queue_next(&task->task_link);
        }

	/* calculculate the system average pagefault rate */
        average_vm_fault_rate = ((task_inswapped_queue_count) ?
                                 (average_vm_fault_rate/task_inswapped_queue_count) : 0);

	/* Next, swap out all tasks that have been sitting idle for a long time */

        task=(task_t)queue_first(&task_inswapped_queue);
        while (!queue_end((vm_offset_t)&task_inswapped_queue, (vm_offset_t)task)) {
		task_lock(task);
		simple_lock(&task->thread_list_lock);
		if (task->thread_count >0) swap_it = TRUE;
		else swap_it = FALSE;
                thread=(thread_t)queue_first(&task->thread_list);
                while (!queue_end((vm_offset_t)(&task->thread_list),(vm_offset_t)thread)) {
			thread_lock(thread);
                        if ((thread->state & TH_RUN) ||
     			    (thread->swap_state != TH_SW_IN) ||
                            (!thread->interruptible) ||
			    (!thread->task->active) ||
                            (!THREAD_LONGTIME_SLEEPER(thread->sleep_stamp))) { 
				thread_unlock(thread);
				thread=(thread_t)queue_next(&thread->thread_list);
				swap_it = FALSE;
				break;
			}
			thread_unlock(thread);
			thread=(thread_t)queue_next(&thread->thread_list);
		}
		simple_unlock(&task->thread_list_lock);
		swap_victim = task;
		task = (task_t)queue_next(&task->task_link);
			/* if this task has been sitting idle, swap it out and get what we can */
		if (swap_it) {
			got_one ++;
			queue_remove(&task_inswapped_queue, swap_victim, task_t, task_link);
			task_inswapped_queue_count--;
			simple_lock(&task_outswap_work_queue_lock);
			queue_enter(&task_outswap_work_queue, swap_victim, task_t, task_link);
			task_outswap_work_queue_count++;
			simple_unlock(&task_outswap_work_queue_lock);
			swap_victim->swap_state = TASK_GOING_OUT;
			swap_victim->ref_count++;
		} 
		task_unlock(swap_victim);
	}

	/* If we didn't get and idle tasks, swap out the lowest priority runnable task */

	if (got_one == 0) {

		int swap_victim_pri = 0;

		
		swap_victim = TASK_NULL;
        	task=(task_t)queue_first(&task_inswapped_queue);

        	while (!queue_end((vm_offset_t)&task_inswapped_queue, (vm_offset_t)task)) {
			int	task_lowest_pri = NRQS_MAX;

                	task_lock(task);

			/* if this is a user task, not being destroyed and has been inswapped for some time, its swappable */

			if ((task->thread_count > 0) &&
			    (task->active) &&
			    (task->map->vm_umap) &&
			    ((sched_tick > (task->inswap_stamp + minimum_task_inswapped_ticks)) ||
			     ((sched_tick > (task->inswap_stamp + minimum_task_inswapped_ticks/2)) &&
			      (EXCESSIVE_FAULTRATE(task->map->vm_fault_rate, average_vm_fault_rate))))) {

				swap_it = TRUE;

                		simple_lock(&task->thread_list_lock);
                		thread=(thread_t)queue_first(&task->thread_list);
                		while (!queue_end((vm_offset_t)(&task->thread_list),(vm_offset_t)thread)) {
                        		thread_lock(thread);
						/* if the threads are swappable */
					if ((thread->swap_state == TH_SW_IN) &&
					    (thread->interruptible)) { 
							/* if this is the lowest priority thread yet */
						if (thread->sched_pri < task_lowest_pri) 
							task_lowest_pri = thread->sched_pri;
					} else {
							/* dont swap tasks if its thread(s) cant be swapped */
						swap_it = FALSE;
						thread_unlock(thread);
						break;
					}	
                        		thread_unlock(thread);
                        		thread=(thread_t)queue_next(&thread->thread_list);
                		}
				simple_unlock(&task->thread_list_lock);
					/* remember the lowest priority task with the largest resident set size */
				if ((swap_it) &&
				    (task_lowest_pri > swap_victim_pri) ||
				    ((swap_victim) &&
				     (task_lowest_pri == swap_victim_pri) &&
				     (pmap_resident_count(task->map->vm_pmap) > pmap_resident_count(swap_victim->map->vm_pmap)) &&
				     (pmap_resident_count(task->map->vm_pmap)*NBPG > (task->u_address->uu_rlimit[RLIMIT_RSS].rlim_cur)))) {
					swap_victim = task;
					swap_victim_pri = thread->sched_pri;
				}
			}
			task_unlock(task);
                	task = (task_t)queue_next(&task->task_link);
		}
			/* if we found a good outswap candidate, put it on the outswap work queue and let the caller swap it out */
	       	if (swap_victim) {
			task_lock(swap_victim);
                       	got_one++;
                       	queue_remove(&task_inswapped_queue, swap_victim, task_t, task_link);
                       	task_inswapped_queue_count--;
                       	simple_lock(&task_outswap_work_queue_lock);
                       	queue_enter(&task_outswap_work_queue, swap_victim, task_t, task_link);
                       	task_outswap_work_queue_count++;
                       	simple_unlock(&task_outswap_work_queue_lock);
                       	swap_victim->swap_state = TASK_GOING_OUT;
                       	swap_victim->ref_count++;
                	task_unlock(swap_victim);
        	}
	}

	simple_unlock(&task_inswapped_queue_lock);	
		/* remember the last time we swapped a task */
	task_swap_time = sched_tick;
		/* tell the caller how many swap victims we found */
	return got_one;
}

/*
 *	task_swapper_timeout();
 *
 *	This timeout routine runs every "default_swapper_wakeup" ticks unless a task wants in but cant
 *	due to low memory, then it runs every "inswap_request_swapper_wakeup" ticks.  This routine
 * 	calls task_swapout_condition() to determine if we need to swap and if no swap activity is in
 *	progress wakes up the task swapper.
 *
 */

task_swapper_timeout(register boolean_t scheduled)
{ 
	register int free, active, inactive;
	int tasks_swapped;
		/* if task swapping needs to occur */
	if (task_swapout_condition() == TRUE) {
		register int s;

		s = splsched();
		simple_lock(&task_swap_scan_lock);
			/* if no task swapping is corrently in progress */
		if (task_swap_scan == FALSE) {
			simple_lock(&task_outswap_work_queue_lock);
			if (queue_empty(&task_outswap_work_queue)) {
				if (scheduled == FALSE) {
					untimeout(task_swapper_timeout, 
						(caddr_t) TRUE);
					scheduled = TRUE;
				}
					/* let the task swapper run and deal with the low memory condition */
				thread_wakeup((vm_offset_t)&task_swap_work);
			}
			simple_unlock(&task_outswap_work_queue_lock);
		}
		simple_unlock(&task_swap_scan_lock);
		(void) splx(s);
	}
		/* if we dont need more memory but a task needs to be inswapped, wake the swapper to swap it in */
        else if (task_inswap_work_queue_count > 0)
        	thread_wakeup((vm_offset_t)&task_swap_work);

		/* if this was clock tick event, schedule the next one based on whether there are tasks that want in or not */
	if (scheduled == TRUE) 
		timeout(task_swapper_timeout, (caddr_t) TRUE, 
			((task_inswap_work_queue_count > 0) ? inswap_request_swapper_wakeup : default_swapper_wakeup));
}

/*
 *      task_swapin_request():
 *
 *	task_swapin_request is called from thread_swapin if the whole task is outswapped.  The priority is also
 *	passed to that this routine can order the task_inswap_work_queue by schedualing priority, the highest
 *	priority task cones in first. 
 *
 */

void task_swapin_request(task, priority)
        register task_t task;
	register int priority;
{

	boolean_t	inserted = FALSE;
	task_t		next_task;
	thread_t	thread;
	int		next_task_pri;

        task_lock(task);
        simple_lock(&task_outswapped_queue_lock);
		/* remove the task from the outswapped queue */
        queue_remove(&task_outswapped_queue, task, task_t, task_link);
        task_outswapped_queue_count--;
        simple_unlock(&task_outswapped_queue_lock);
        simple_lock(&task_inswap_work_queue_lock);
       	next_task=(task_t)queue_first(&task_inswap_work_queue);
		/* find where in the task_inswapped_work_queue this task goes based on scheduling priority */
       	while (!queue_end((vm_offset_t)&task_inswap_work_queue, (vm_offset_t)next_task)) {
               	task_lock(next_task);
               	simple_lock(&next_task->thread_list_lock);
		next_task_pri = NRQS_MAX;
               	thread=(thread_t)queue_first(&task->thread_list);
			/* find the highest priority thread in the task */
               	while (!queue_end((vm_offset_t)(&task->thread_list),(vm_offset_t)thread)) {
			if ((thread->sched_pri < next_task_pri) &&
			    (thread->state & TH_RUN)) 
				next_task_pri = thread->sched_pri;
                	thread=(thread_t)queue_next(&thread->thread_list);
               	}
			/* if the next task has a lower priority, insert this task here */
		if (next_task_pri > priority) { 
			if (next_task == (task_t)queue_first(&task_inswap_work_queue))
				queue_enter_first(&task_inswap_work_queue, task, task_t, task_link);
			else {	
				task->task_link.prev = (queue_t)(next_task->task_link.prev);
				task->task_link.next = (queue_t)next_task;
				((task_t)(next_task->task_link.prev))->task_link.next = (queue_t)task;
				next_task->task_link.prev = (queue_t)task;
			}
			inserted = TRUE;
			break;
		}
               	simple_unlock(&next_task->thread_list_lock);
		task_unlock(next_task);
               	next_task = (task_t)queue_next(&next_task->task_link);
       	}
		/* if there were no other tasks on the queue, just insert this task */
	if (!inserted)
		queue_enter(&task_inswap_work_queue, task, task_t, task_link);

		/* adjust the queue count, mark the task comming in and wake up the swapper */
        task_inswap_work_queue_count++;
        simple_unlock(&task_inswap_work_queue_lock);
        task->swap_state = TASK_COMMING_IN;
        task_unlock(task);
        thread_wakeup((vm_offset_t)&task_swap_work); 
}
