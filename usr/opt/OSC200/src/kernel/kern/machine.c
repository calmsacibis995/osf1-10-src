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
static char	*sccsid = "@(#)$RCSfile: machine.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1993/01/12 16:23:06 $";
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
 *	File:	kern/machine.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1987, Avadis Tevanian, Jr.
 *
 *	Support for machine independent machine abstraction.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <cpus.h>
#include <mach_host.h>
#include <mach_km.h>

#include <sys/types.h>
#include <mach/boolean.h>
#include <kern/ipc_host.h>
#include <mach/kern_return.h>
#include <kern/lock.h>
#include <mach/machine.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <kern/task.h>
#include <kern/thread.h>

#include <machine/machparam.h>	/* for splsched */

#if	MACH_KM
#include <kern/kern_mon.h>
#endif	MACH_KM

/*
 *	Exported variables:
 */

struct machine_info	machine_info;
struct machine_slot	machine_slot[NCPUS];

vm_offset_t		interrupt_stack[NCPUS];

queue_head_t	action_queue;	/* assign/shutdown queue */
decl_simple_lock_data(,action_lock);


/*
 *	xxx_host_info:
 *
 *	Return the host_info structure.  This routine is exported to the
 *	user level.
 */
kern_return_t xxx_host_info(task, info)
	task_t		task;
	machine_info_t	info;
{
#ifdef	lint
	task++;
#endif	lint
	*info = machine_info;
	return(KERN_SUCCESS);
}

/*
 *	xxx_slot_info:
 *
 *	Return the slot_info structure for the specified slot.  This routine
 *	is exported to the user level.
 */
kern_return_t xxx_slot_info(task, slot, info)
	task_t		task;
	int		slot;
	machine_slot_t	info;
{
#ifdef	lint
	task++;
#endif	lint
	if ((slot < 0) || (slot >= NCPUS))
		return(KERN_INVALID_ARGUMENT);
	*info = machine_slot[slot];
	return(KERN_SUCCESS);
}

/*
 *	xxx_cpu_control:
 *
 *	Support for user control of cpus.  The user indicates which cpu
 *	he is interested in, and whether or not that cpu should be running.
 *
 *	NOT IMPLEMENTED PENDING A MECHANISM TO GIVE ACCESS TO ONLY PRIVILEDGED
 *	TASKS.
 */
kern_return_t xxx_cpu_control(task, cpu, runnable)
	task_t		task;
	int		cpu;
	boolean_t	runnable;
{
#if	NCPUS > 1 && defined(vax)
	int		ret;
#endif	NCPUS > 1 && defined(vax)
	
#ifdef	lint
	task++; cpu++; runnable++;
#endif	lint

#if	NCPUS > 1 && defined(vax)
/*	if (!suser())
		return(KERN_FAILURE);*/

	if (cpu == -1) {
		start_processors(0);
		return KERN_SUCCESS;
	}

	switch (runnable) {
	extern int freeze_control;
	case 6:
		move_console(cpu);
		ret = 1;
		break;
	case 5:
		ret = freeze_processor(cpu);
		break;
	case 4:
		freeze_control &= ~(1<<cpu);
		ret = 1;
		break;
	case 3:
		ret = halt_processor(cpu);
		break;
	case 2:
		ret = continue_processor(cpu);
		break;
	case 1:
		ret = start_processor(cpu);
		break;
	case 0:
		ret = exit_processor(cpu);
		break;
	}
	if (ret)
		return KERN_SUCCESS;
	else
		return KERN_FAILURE;

#else	NCPUS > 1 && defined(vax)
	return(KERN_FAILURE);	/* for now */
#endif	NCPUS > 1 && defined(vax)
}

/*
 *	cpu_up:
 *
 *	Flag specified cpu as up and running.  Called when a processor comes
 *	online.
 */
cpu_up(cpu)
	int	cpu;
{
	register struct machine_slot	*ms;
	register processor_t	processor;
	register int s;

	processor = cpu_to_processor(cpu);
	if (processor != master_processor)
		ipc_processor_init(processor);
	pset_lock(&default_pset);
	s = splsched();
	processor_lock(processor);
#if	MACH_KM
	monitor_cpu_in(cpu);
#endif	MACH_KM
#if	NCPUS > 1
	init_ast_check(processor);
#endif	NCPUS > 1
	ms = &machine_slot[cpu];
	ms->running = TRUE;
	machine_info.avail_cpus++;
	pset_add_processor(&default_pset, processor);
	processor->state = PROCESSOR_RUNNING;
	processor_unlock(processor);
	splx(s);
	pset_unlock(&default_pset);
	if (processor != master_processor)
		ipc_processor_enable(processor);
}

/*
 *	cpu_down:
 *
 *	Flag specified cpu as down.  Called when a processor is about to
 *	go offline.
 */
cpu_down(cpu)
	int	cpu;
{
	register struct machine_slot	*ms;
	register processor_t	processor;
	register int	s;

	s = splsched();
	processor = cpu_to_processor(cpu);
	ipc_processor_terminate(processor);
	processor_lock(processor);
#if	MACH_KM
	monitor_cpu_out(cpu);
#endif	MACH_KM		
	ms = &machine_slot[cpu];
	ms->running = FALSE;
	machine_info.avail_cpus--;
	/*
	 *	processor has already been removed from pset.
	 */
	processor->processor_set_next = PROCESSOR_SET_NULL;
	processor->state = PROCESSOR_OFF_LINE;
	processor_unlock(processor);
	splx(s);
}

#if	NCPUS > 1
/*
 *	processor_request_action - common internals of processor_assign
 *		and processor_shutdown.  If new_pset is null, this is
 *		a shutdown, else it's an assign.
 */
void
processor_request_action(processor, new_pset)
processor_t	processor;
processor_set_t	new_pset;
{
    register processor_set_t pset;

    /*
     *	Processor must be in a processor set.  Must lock its idle lock to
     *	get at processor state.
     */
    pset = processor->processor_set;
    simple_lock(&pset->idle_lock);

    /*
     *	If the processor is dispatching, let it finish - it will set its
     *	state to running very soon.
     */
    while (processor->state == PROCESSOR_DISPATCHING)
    	continue;

    /*
     *	Now lock the action queue and do the dirty work.
     */
    simple_lock(&action_lock);

    switch (processor->state) {
	case PROCESSOR_IDLE:
	    /*
	     *	Remove from idle queue.
	     */
	    queue_remove(&pset->idle_queue, processor, 	processor_t,
		processor_queue);
	    pset->idle_count--;

	    /* fall through ... */
	case PROCESSOR_RUNNING:
	    /*
	     *	Put it on the action queue.
	     */
	    queue_enter(&action_queue, processor, processor_t,
		processor_queue);

	    /* fall through ... */
	case PROCESSOR_ASSIGN:
	    /*
	     * And ask the action_thread to do the work.
	     */
	    if (new_pset == PROCESSOR_SET_NULL) {
		processor->state = PROCESSOR_SHUTDOWN;
	    }
	    else {
		processor->state = PROCESSOR_ASSIGN;
		processor->processor_set_next = new_pset;
	    }
	    break;

	default:
	    printf("state: %d\n", processor->state);
	    panic("processor_request_action: bad state");
    }
    simple_unlock(&action_lock);
    simple_unlock(&pset->idle_lock);

    thread_wakeup((vm_offset_t)&action_queue);

    return;
}

#if	MACH_HOST
/*
 *	processor_assign() changes the processor set that a processor is
 *	assigned to.  Any previous assignment in progress is overriden.
 *	Synchronizes with assignment completion if wait is TRUE.
 */
kern_return_t
processor_assign(processor, new_pset, wait)
processor_t	processor;
processor_set_t	new_pset;
boolean_t	wait;
{
    int		s;

    /*
     *	Check for null arguments.
     *  XXX Can't assign master processor.
     */
    if (processor == PROCESSOR_NULL || new_pset == PROCESSOR_SET_NULL ||
	processor == master_processor) {
	    return(KERN_FAILURE);
    }

    s = splsched();
    processor_lock(processor);
    if(processor->state == PROCESSOR_OFF_LINE ||
	processor->state == PROCESSOR_SHUTDOWN) {
	    /*
	     *	Already shutdown or being shutdown -- Can't reassign.
	     */
	    processor_unlock(processor);
	    (void) splx(s);
	    return(KERN_FAILURE);
    }

    processor_request_action(processor, new_pset);

    processor_unlock(processor);
    splx(s);

    /*
     *	Synchronization with completion.
     */
    if (wait) {
	assert_wait((vm_offset_t)processor, TRUE);
	while (processor->state == PROCESSOR_ASSIGN ||
	    processor->state == PROCESSOR_SHUTDOWN) {
		thread_block();
	    	assert_wait((vm_offset_t)processor, TRUE);
	}
	clear_wait(current_thread(), THREAD_AWAKENED, TRUE);
    }
    
    return(KERN_SUCCESS);
}

#else	MACH_HOST

kern_return_t
processor_assign(processor, new_pset, wait)
processor_t	processor;
processor_set_t	new_pset;
boolean_t	wait;
{
#ifdef	lint
	processor++; new_pset++; wait++;
#endif	lint
	return(KERN_FAILURE);
}

#endif	MACH_HOST

/*
 *	processor_shutdown() queues a processor up for shutdown.
 *	Any assignment in progress is overriden.  It does not synchronize
 *	with the shutdown (can be called from interrupt level).
 */
kern_return_t
processor_shutdown(processor)
processor_t	processor;
{
    processor_set_t	pset;
    int		s;

    s = splsched();
    processor_lock(processor);
    if(processor->state == PROCESSOR_OFF_LINE ||
	processor->state == PROCESSOR_SHUTDOWN) {
	    /*
	     *	Already shutdown or being shutdown -- nothing to do.
	     */
	    processor_unlock(processor);
	    splx(s);
	    return(KERN_SUCCESS);
    }

    processor_request_action(processor, PROCESSOR_SET_NULL);
    processor_unlock(processor);
    splx(s);

    return(KERN_SUCCESS);
}

/*
 *	action_thread() shuts down processors or changes their assignment.
 */

void action_thread()
{
	register processor_t	processor;
	register int		s;

	s = splsched();		/* runs with interrupts locked out */
	simple_lock(&action_lock);
	while (TRUE) {
		thread_sleep((vm_offset_t)&action_queue, &action_lock, FALSE);
		simple_lock(&action_lock);
		while ( !queue_empty(&action_queue)) {
			processor = (processor_t) queue_first(&action_queue);
			queue_remove(&action_queue, processor, processor_t,
				processor_queue);
			simple_unlock(&action_lock);
			(void) splx(s);
			processor_doaction(processor);
			s = splsched();
			simple_lock(&action_lock);
		}
	}
}

/*
 *	processor_doaction actually does the shutdown.  The trick here
 *	is to schedule ourselves onto a cpu and then save our
 *	context back into the runqs before taking out the cpu.
 */

processor_doaction(processor)
register processor_t	processor;
{
	register thread_t		thread, prev_thread;
	thread_t			this_thread;
	int				s;
	register processor_set_t	pset, new_pset;
	boolean_t			have_pset_ref = FALSE;
	void				processor_doshutdown();

	/*
	 *	Get onto the processor to shutdown
	 */
	this_thread = current_thread();
	thread_bind(this_thread, processor);
	thread_block();

#if	MACH_HOST
	/*
	 *	If this is the last processor in the processor_set,
	 *	stop all the threads first.
	 */
	pset = processor->processor_set;
	pset_lock(pset);
	if (pset->processor_count == 1) {
		/*
		 *	First suspend all of them.
		 */
		thread = (thread_t) queue_first(&pset->threads);
		while (!queue_end(&pset->threads, (queue_entry_t) thread)) {
			thread_hold(thread);
			thread = (thread_t) queue_next(&thread->pset_threads);
		}
		pset->empty = TRUE;
		/*
		 *	Now actually stop them.  Need a pset reference.
		 */
		pset->ref_count++;
		have_pset_ref = TRUE;

Restart_thread:
		prev_thread = THREAD_NULL;
		thread = (thread_t) queue_first(&pset->threads);
		while (!queue_end(&pset->threads, (queue_entry_t) thread)) {
			thread_reference(thread);
			pset_unlock(pset);
			if (prev_thread != THREAD_NULL)
				thread_deallocate(prev_thread);

			/*
			 *	Only wait for threads still in the pset.
			 */
			thread_freeze(thread);
			if (thread->processor_set != pset) {
				/*
				 *	It got away - start over.
				 */
				thread_unfreeze(thread);
				thread_deallocate(thread);
				pset_lock(pset);
				goto Restart_thread;
			}

			(void) thread_dowait(thread, TRUE);
			prev_thread = thread;
			pset_lock(pset);
			thread = (thread_t) queue_next(&thread->pset_threads);
			thread_unfreeze(prev_thread);
		}
	}
#endif	MACH_HOST

	/*
	 *	At this point, it is ok to remove the processor from the pset.
	 */
	s = splsched();
	processor_lock(processor);
	pset_remove_processor(pset, processor);
	pset_unlock(pset);

#if	MACH_HOST
	if (processor->state == PROCESSOR_ASSIGN) {

Restart_pset:
	    /*
	     *	Nasty problem: we want to lock the target pset, but
	     *	we have to enable interrupts to do that which requires
	     *  dropping the processor lock.  While the processor
	     *  is unlocked, it could be reassigned or shutdown.
	     */
	    new_pset = processor->processor_set_next;
	    processor_unlock(processor);
	    splx(s);

	    /*
	     *  Lock target pset and handle remove last / assign first race.
	     *	Only happens if there is more than one action thread.
	     */
	    pset_lock(new_pset);
	    while (new_pset->empty && new_pset->processor_count > 0) {
		pset_unlock(new_pset);
	    	while (new_pset->empty && new_pset->processor_count > 0)
			/* spin */;
		pset_lock(new_pset);
	    }

	    /*
	     *	Finally relock the processor and see if something changed.
	     *	The only possibilities are assignment to a different pset
	     *	and shutdown.
	     */
	    s = splsched();
	    processor_lock(processor);

	    if (processor->state == PROCESSOR_SHUTDOWN) {
		pset_unlock(new_pset);
		goto shutdown;
	    }

	    if (new_pset != processor->processor_set_next) {
		pset_unlock(new_pset);
		goto Restart_pset;
	    }

	    /*
	     *	Do assignment, then wakeup anyone waiting for it.
	     *	Finally context switch to have it take effect.
	     */
	    pset_add_processor(new_pset, processor);
	    if (new_pset->empty) {
		/*
		 *	Set all the threads loose
		 */
		thread = (thread_t) queue_first(&new_pset->threads);
		while (!queue_end(&new_pset->threads,(queue_entry_t)thread)) {
		    thread_release(thread);
		    thread = (thread_t) queue_next(&thread->pset_threads);
		}
		new_pset->empty = FALSE;
	    }
	    processor->processor_set_next = PROCESSOR_SET_NULL;
	    processor->state = PROCESSOR_RUNNING;
	    thread_wakeup((vm_offset_t)processor);
	    processor_unlock(processor);
	    splx(s);
	    pset_unlock(new_pset);

	    /*
	     *	Clean up dangling references, and release our binding.
	     */
	    if (have_pset_ref)
		pset_deallocate(pset);
	    if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);
	    thread_bind(this_thread, PROCESSOR_NULL);

	    thread_block();
	    return;
	}
#endif	MACH_HOST
	
	/*
	 *	Do shutdown, make sure we live when processor dies.
	 */
shutdown:
	if (processor->state != PROCESSOR_SHUTDOWN) {
		printf("state: %d\n", processor->state);
	    	panic("action_thread -- bad processor state");
	}
	processor_unlock(processor);
	/*
	 *	Clean up dangling references, and release our binding.
	 */
	if (have_pset_ref)
		pset_deallocate(pset);
	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);

	ipc_processor_disable(processor);

	thread_bind(this_thread, PROCESSOR_NULL);
	processor_doshutdown(processor);
	splx(s);
}

/*
 *	Actually do the processor shutdown.  This is called at splsched.
 */

void processor_doshutdown(processor)
register processor_t	processor;
{
	register thread_t	th = current_thread();
	register int		cpu = processor->slot_num;


#if     MACH_KM		
	if (th->monitor_obj != MONITOR_NULL) 
	  monitor_old_thread(th, THREAD_NULL, cpu);	
#endif  MACH_KM							

	if (save_context()) {
		return;
	}

	timer_switch(&kernel_timer[cpu]);

	/*
	 *	Map paranoia.
	 */
	if (vm_map_pmap(th->task->map) != kernel_pmap)
		PMAP_DEACTIVATE(vm_map_pmap(th->task->map), th, cpu);
	
	/*
	 *	We must be in TH_RUN state here; put us back on a runq.
	 */
	thread_lock(th);
	thread_setrun(th, FALSE);
	thread_unlock(th);

	/*
	 *	Ok, now exit this cpu.
	 */
	PMAP_DEACTIVATE(kernel_pmap, th, cpu);
	active_threads[cpu] = THREAD_NULL;
	cpu_down(cpu);
	thread_wakeup((vm_offset_t)processor);
	halt_cpu();
	panic("zombie processor");

	/*
	 *	The action thread returns to life at save_context()
	 *	above on some other cpu.
	 */

	/*NOTREACHED*/
}
#else	NCPUS > 1

kern_return_t
processor_assign(processor, new_pset, wait)
processor_t	processor;
processor_set_t	new_pset;
boolean_t	wait;
{
#ifdef	lint
	processor++; new_pset++; wait++;
#endif	lint
	return(KERN_FAILURE);
}

#endif NCPUS > 1
