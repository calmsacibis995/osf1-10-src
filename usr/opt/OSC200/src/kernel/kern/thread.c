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
static char	*sccsid = "@(#)$RCSfile: thread.c,v $ $Revision: 4.4.22.8 $ (DEC) $Date: 1993/12/17 01:14:54 $";
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
 *	        File:	kern/thread.c
 *	Author:	        Avadis Tevanian, Jr., Michael Wayne Young, David Golub
 *      
 *	        Copyright (C) 1986, Avadis Tevanian, Jr., Michael Wayne Young
 *		        			David Golub
 *      
 *	Thread management primitives implementation.
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 27-Sep-1991	Peter H. Smith
 *	Update the processor quantum if a thread changes its own policy.
 *
 * 13-June-91	Brian Stevens
 *	Include the header file sys/security.h if SEC_BASE is defined.
 *
 * 10-june-1991 Ron Widyono
 *              Remove initialization os save_need_ast and lock_count
 *
 * 6-June-91	Brian Stevens
 *	Added maxthreads to enforce a limit on the number of threads
 *	per task.
 *
 * 13-May-91	Peter H. Smith
 *	Change thread_info so that it always returns priority information based
 *	on the range 0 (highest) to (NRQS_MAX-1) (lowest).  This will make it
 *	easier for programs like ps to interpret the priority information.
 *
 * 5-May-91	Ron Widyono
 *	Initialize save_need_ast field in thread structure.
 *
 * 02-May-91	Peter H. Smith
 *	Clean up based on code review comments.
 *
 * 9-Apr-91	Peter H. Smith
 *	Modify thread_policy to set sched_data and rmng_quantum for fixed
 *	priority threads.  Also, return the thread quantum in thread_info.
 *
 * 6-Apr-91	Ron Widyono
 *	Initialize per-thread wait lock counter.  Conditionalized (RT_PREEMPT).
 *
 */

#include <cpus.h>
#include <hw_footprint.h>
#include <mach_km.h>
#include <mach_host.h>
#include <simple_clock.h>
#include <rt_preempt.h>
#include <rt_sched.h>

#include <sys/types.h>
#include <kern/ast.h>
#include <kern/mach_param.h>
#include <mach/policy.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <kern/thread.h>
#include <mach/thread_status.h>
#include <mach/thread_info.h>
#include <mach/thread_special_ports.h>
#include <mach/time_value.h>
#include <kern/zalloc.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <kern/parallel.h>
#include <kern/ipc_tt.h>
#include <kern/sched_prim.h>
#include <kern/thread_swap.h>
#include <machine/machparam.h>		/* for splsched */
#include <machine/fpu.h>
#include <sys/user.h>			/* XXX Unix compatibility */
#include <sys/ucred.h>			/* XXX Unix compatibility */
#include <sys/syslog.h>                 /* XXX Unix syslog interface */

#if SEC_BASE
#include <sys/security.h>
#endif

#if	MACH_KM
#include <kern/kern_mon.h>
#endif	MACH_KM

#include <sys/sched_mon.h>
#include <sys/proc.h>
#include <kern/kd_thread_support.h>

/*
 * Local routine declarations
 */
void kd_thread_start();
void kd_thread_init();
int kd_thread_register();

/*
 * External routine declarations
 */
extern queue_entry_t dequeue_head();
extern void enqueue_tail();

thread_t active_threads[NCPUS];
struct zone *thread_zone, *u_zone;

queue_head_t		stack_queue;
int			stack_free_count = 0;
int			stack_free_target = 5;
decl_simple_lock_data(,	stack_queue_lock)

queue_head_t		reaper_queue;
decl_simple_lock_data(,	reaper_lock)

boolean_t		need_stack_wakeup = FALSE;

extern int		tick;

extern int		maxthreads;

/*
 * The kd_thread_list is a global variable which points to a linked
 * kd_thread_elt structure.  Each kd_thread_elt structure contains
 * callback routine for each registered device driver.  Kernel routines
 * and or Device driver  start up the kernel threads when the callback
 * routine is called.
 */
KD_THREAD_LIST kd_thread_list;

/*
 * kd_flags tell the state of the kd_thread_list structure
 * see kd_thread_support.h for flag definitions
 */
u_int kd_flags = 0;

/*
 * The lock structure for the list.
 */
KD_LOCK_THREAD kd_lock_thread;
                                         
vm_offset_t stack_alloc()
{
	register vm_offset_t	stack;
	register kern_return_t	result = THREAD_AWAKENED;

	do {
	    simple_lock(&stack_queue_lock);
	    if (stack_free_count != 0) {
		stack = (vm_offset_t) dequeue_head(&stack_queue);
		stack_free_count--;
	    } else {
		stack = (vm_offset_t)0;
	    }
	    simple_unlock(&stack_queue_lock);

	    /*
	     *	If no stacks on queue, kmem_alloc one.  If that fails,
	     *	pause and wait for a stack to be freed.
	     */
	    if (stack == (vm_offset_t)0)
		stack = kmem_alloc(kernel_map, KERNEL_STACK_SIZE);

	    if (stack == (vm_offset_t)0) {
		if (!need_stack_wakeup) {
	          log(LOG_WARNING, "stack_alloc: waiting for kernel stack\n");
		}
		else if (result != THREAD_AWAKENED) {
		    /*
		     *	Somebody wants us; return a bogus stack.
		     */
		    return((vm_offset_t)0);
		}

		/*
		 *	Now wait for stack, but first make sure one
		 *	hasn't appeared in the interim.
		 */
		simple_lock(&stack_queue_lock);
		if(stack_free_count != 0) {
		    simple_unlock(&stack_queue_lock);
		    result = THREAD_AWAKENED;
		    continue;
		}
		assert_wait((vm_offset_t)&stack_queue, FALSE);
		need_stack_wakeup = TRUE;
		simple_unlock(&stack_queue_lock);
		thread_block();
		result = current_thread()->wait_result;
	    }
	} while (stack == (vm_offset_t)0);

	return(stack);
}

void stack_free(stack)
	vm_offset_t	stack;
{
	simple_lock(&stack_queue_lock);
	if (stack_free_count < stack_free_target) {
		stack_free_count++;
		enqueue_head(&stack_queue, (queue_entry_t) stack);
		stack = 0;
	}
	/*
	 * If anyone is waiting for a stack, wake them up.
	 */
	if (need_stack_wakeup) {
		need_stack_wakeup = FALSE;
		thread_wakeup((vm_offset_t)&stack_queue);
	}
	simple_unlock(&stack_queue_lock);

	if (stack != 0)
		kmem_free(kernel_map, stack, KERNEL_STACK_SIZE);
}

/* private */
struct thread	thread_template;
extern int	threadmax;

void thread_init()
{
	thread_zone = zinit(
			sizeof(struct thread),
			threadmax * sizeof(struct thread),
			THREAD_CHUNK * sizeof(struct thread),
			"threads");
	/* Set thread zone to exhaustible and collectable */
	zchange(thread_zone, FALSE, FALSE, TRUE, TRUE);

	/*
	 *	Fill in a template thread for fast initialization.
	 *	[Fields that must be (or are typically) reset at
	 *	time of creation are so noted.]
	 */

	/* thread_template.links (none) */
	thread_template.runq = RUN_QUEUE_NULL;

	/* thread_template.task (later) */
	/* thread_template.thread_list (later) */
	/* thread_template.pset_threads (later) */

	/* thread_template.lock (later) */
	thread_template.ref_count = 1;

	thread_template.pcb = (struct pcb *) 0;		/* (reset) */
	thread_template.kernel_stack = (vm_offset_t) 0;	/* (reset) */

	thread_template.wait_event = 0;
        thread_template.wait_mesg = 0;
	/* thread_template.suspend_count (later) */
	thread_template.interruptible = TRUE;
	thread_template.wait_result = KERN_SUCCESS;
	thread_template.timer_set = FALSE;
	thread_template.wake_active = FALSE;
	thread_template.swap_state = TH_SW_IN;
	thread_template.state = TH_SUSP;

	thread_template.priority = BASEPRI_USER;
	thread_template.max_priority = BASEPRI_USER;
	thread_template.sched_pri = BASEPRI_USER;
	thread_template.sched_data = 0;
	thread_template.policy = POLICY_TIMESHARE;
	thread_template.depress_priority = -1;
	thread_template.cpu_usage = 0;
	thread_template.sched_usage = 0;
	/* thread_template.sched_stamp (later) */

	thread_template.recover = (vm_offset_t) 0;
	thread_template.vm_privilege = FALSE;
	thread_template.tmp_address = (vm_offset_t) 0;
	thread_template.tmp_object = VM_OBJECT_NULL;

	/* thread_template.u_address (later) */
	thread_template.unix_lock = -1;		/* XXX for Unix */

	thread_template.user_stop_count = 1;

	/* thread_template.<IPC structures> (later) */

	timer_init(&(thread_template.user_timer));
	timer_init(&(thread_template.system_timer));
	thread_template.user_timer_save.low = 0;
	thread_template.user_timer_save.high = 0;
	thread_template.system_timer_save.low = 0;
	thread_template.system_timer_save.high = 0;
	thread_template.cpu_delta = 0;
	thread_template.sched_delta = 0;

	thread_template.exception_port = PORT_NULL;
	thread_template.exception_clear_port = PORT_NULL;

	thread_template.active = FALSE; /* reset */
	thread_template.halted = FALSE;
	thread_template.ast = AST_ZILCH;

	/* thread_template.processor_set (later) */
	thread_template.bound_processor = PROCESSOR_NULL;
#if	MACH_HOST
	thread_template.may_assign = TRUE;
	thread_template.assign_active = FALSE;
#endif	MACH_HOST

#if	HW_FOOTPRINT
	/* thread_template.last_processor  (later) */
#endif	HW_FOOTPRINT

#if	MACH_KM
	thread_template.monitor_obj = MONITOR_NULL;
	thread_template.monitor_id = MONITOR_NO_ID;
	/* thread_template.monitored_threads (much later) */
#endif	MACH_KM

	/*
	 *	Initialize other data structures used in
	 *	this module.
	 */
	
	queue_init(&stack_queue);
	simple_lock_init(&stack_queue_lock);

	queue_init(&reaper_queue);
	simple_lock_init(&reaper_lock);
}

kern_return_t thread_create(parent_task, child_thread)
	register task_t	parent_task;
	thread_t	*child_thread;		/* OUT */
{
	register thread_t	new_thread, prime_thread;
	register int		s;
	register processor_set_t	pset;
	struct uthread *cur_uthread, *new_uthread;

	if (parent_task == TASK_NULL)
		return(KERN_INVALID_ARGUMENT);

	/*
	 * Don't allow more than the maximum number of threads per task to
	 * be created - except for root.
	 */

#if     SEC_BASE
	if (parent_task->thread_count >= maxthreads &&
	    !privileged(SEC_LIMIT, 0))
#else
	if (parent_task->thread_count >= maxthreads &&
	    suser(u.u_cred, &u.u_acflag))
#endif
		return(KERN_FAILURE);

	/*
	 *	Allocate a thread and initialize static fields
	 */

	new_thread = (thread_t) zalloc(thread_zone);

	if (new_thread == THREAD_NULL)
		return(KERN_RESOURCE_SHORTAGE);

	*new_thread = thread_template;

	/*
	 *	Initialize runtime-dependent fields
	 */

	new_thread->task = parent_task;
	simple_lock_init(&new_thread->lock);
	new_thread->sched_stamp = sched_tick;
	/* New for /proc */
	if(!queue_empty(&parent_task->thread_list))
		prime_thread = (thread_t)queue_first(&parent_task->thread_list);
	else
		prime_thread = THREAD_NULL;
	if (prime_thread && prime_thread->t_procfs.pr_flags & PR_FORK)
	    new_thread->t_procfs = prime_thread->t_procfs;
	else {
	    bzero(&new_thread->t_procfs, sizeof(struct t_procfs) );
	}
#ifdef PROCFS_U
	if(parent_task->procfs.pr_qflags & PRFS_STOPTCR) {
		parent_task->procfs.pr_flags |=
			(PR_STOPPED | PR_ISTOP | PR_PCINVAL);
		parent_task->procfs.pr_why = PRFS_STOPTCR;
		parent_task->procfs.pr_what = (long )new_thread;
		wakeup(&parent_task->procfs);
		thread_suspend(current_thread());
		thread_block(current_thread());
	}
#endif
	/* End of /proc code */
#if	(MACH_LDEBUG)
	new_thread->lock_count = 0;
	bzero(&new_thread->lock_addr[0], sizeof(int)*MAX_LOCK);
#endif
	/*
	 *	Create a kernel stack, and put the PCB at the front.
	 */

	new_thread->kernel_stack = stack_alloc();

	/*
	 *	Only reason for stack_alloc failure is termination of
	 *	current thread.  Send back a return code anyway.
	 */
	if (new_thread->kernel_stack == 0) {
		zfree(thread_zone, (vm_offset_t) new_thread);
		return(KERN_RESOURCE_SHORTAGE);
	}

/*
 * Note: the next 2 ifdef mips are changes to where the uthread/pcb
 *       are allocated on the 2 pages of kernel stack space.
 *       These changes are related to the kernel stack changes
 *       to add guard pages, an expansion page, as well as for 
 *       hw indep. inclusions.
 */
#ifdef mips
	new_thread->pcb = (struct pcb *) (new_thread->kernel_stack
					+  PCB_START_OFFSET);
#else
	new_thread->pcb = (struct pcb *) new_thread->kernel_stack;
#endif /* mips */

	/*
	 *	Set up the u-address pointers.
	 */
#ifdef mips
	new_thread->u_address.uthread = (struct uthread *)
					(new_thread->kernel_stack
					+ UTHREAD_START_OFFSET);
#else
	new_thread->u_address.uthread = (struct uthread *)
		(new_thread->kernel_stack + sizeof(struct pcb));
#endif /* mips */
	uarea_zero(new_thread);
	new_thread->u_address.utask = parent_task->u_address;
	uarea_init(new_thread);
#ifdef __alpha
	/*
	 * if the inherit bit is set in the ieee_fp_control register,
	 * the ieee info is set are per specification.
	 */
	if (current_thread())
		cur_uthread = current_thread()->u_address.uthread;
	else
		cur_uthread = 0;

	if (cur_uthread && cur_uthread->uu_ieee_fp_control & IEEE_INHERIT){
		new_uthread = new_thread->u_address.uthread;

		new_uthread->uu_ieee_fp_control = 
		      cur_uthread->uu_ieee_fp_control;
		new_uthread->uu_ieee_set_state_at_signal =
		      cur_uthread->uu_ieee_set_state_at_signal;
		new_uthread->uu_ieee_fp_control_at_signal =
		      cur_uthread->uu_ieee_fp_control_at_signal;
		new_uthread->uu_ieee_fpcr_at_signal =
		      cur_uthread->uu_ieee_fpcr_at_signal;
	}
#endif /* __alpha */

	pcb_init(new_thread, new_thread->kernel_stack);

	*child_thread = new_thread;

	ipc_thread_init(new_thread);
	new_thread->ipc_kernel = FALSE;

	task_lock(parent_task);
	pset = parent_task->processor_set;
	if (!pset->active) {
		pset = &default_pset;
	}

	s = splsched();
	simple_lock(&parent_task->thread_list_lock);
	parent_task->thread_count++;
	queue_enter(&parent_task->thread_list, new_thread, thread_t,
					thread_list);
	simple_unlock(&parent_task->thread_list_lock);
	(void) splx(s);

	new_thread->suspend_count = parent_task->suspend_count + 1;
					/* account for start state */
	parent_task->ref_count++;

	new_thread->active = TRUE;

	pset_lock(pset);
	pset_add_thread(pset, new_thread);
	if (pset->empty)
		new_thread->suspend_count++;

	new_thread->max_priority = pset->max_priority;
	if (new_thread->max_priority > new_thread->priority)
		new_thread->priority = new_thread->max_priority;

#if	HW_FOOTPRINT
	/*
	 *	Need to set last_processor, idle processor would be best, but
	 *	that requires extra locking nonsense.  Go for tail of
	 *	processors queue to avoid master.
	 */
	if (!(pset->empty)) {
		thread->last_processor = 
			(processor_t)queue_first(&pset->processors);
	}
	else {
		/*
		 *	Thread created in empty processor set.  Pick
		 *	master processor as an acceptable legal value.
		 */
		thread->last_processor = master_processor;
	}
#endif	HW_FOOTPRINT
	
	pset_unlock(pset);

	if (!parent_task->active) {
		task_unlock(parent_task);
		(void) thread_terminate(new_thread);
		return(KERN_FAILURE);
	}
	task_unlock(parent_task);

	ipc_thread_enable(new_thread);

	return(KERN_SUCCESS);
}

void thread_deallocate(thread)
	register thread_t	thread;
{
	int		s;
	register task_t	task;
	register processor_set_t	pset;
	struct ucred *cr;

	time_value_t	user_time, system_time;

	extern void thread_depress_timeout();

	if (thread == THREAD_NULL)
		return;

	/*
	 *	First, check for new count > 0 (the common case).
	 *	Only the thread needs to be locked.
	 */
	s = splsched();
	thread_lock(thread);
	if (--thread->ref_count > 0) {
		thread_unlock(thread);
		(void) splx(s);
		return;
	}

	/*
	 *	Count is zero.  However, the task's and processor set's
	 *	thread lists have implicit references to
	 *	the thread, and may make new ones.  Their locks also
	 *	dominate the thread lock.  To check for this, we
	 *	temporarily restore the one thread reference, unlock
	 *	the thread, and then lock the other structures in
	 *	the proper order.
	 */
	thread->ref_count = 1;
	thread_unlock(thread);
	(void) splx(s);

#if	MACH_HOST
	/*
	 *	Freeze the thread so that its processor_set pointer is valid.
	 */
	thread_freeze(thread);
#endif	MACH_HOST

	task = thread->task;
	pset = thread->processor_set;

	task_lock(task);
	pset_lock(pset);

	s = splsched();
	simple_lock(&task->thread_list_lock);
	thread_lock(thread);
	
	if (--thread->ref_count > 0) {
		/*
		 *	Task or processor_set made extra reference.
		 */
		thread_unlock(thread);
		simple_unlock(&task->thread_list_lock);
		(void) splx(s);
		pset_unlock(pset);
		task_unlock(task);
#if	MACH_HOST
		thread_unfreeze(thread);
#endif	MACH_HOST
		return;
	}

	/*
	 *	Remember thread's credentials so they can be
	 *	deallocated later (Unix compatibility).		XXX
	 */
	cr = thread->u_address.uthread->uu_nd.ni_cred;

	/*
	 *	Thread has no references - we can remove it.
	 */

	/*
	 *	Remove pending depress timeout if there is one.
	 */
	if (thread->depress_priority >= 0) {
#if	NCPUS > 1
	    if (!untimeout_try(thread_depress_timeout, thread)) {
		/*
		 *	Missed it, wait for timeout to happen.
		 */
		while (thread->depress_priority >= 0)
		    continue;
	    }
#else	NCPUS > 1
	    untimeout(thread_depress_timeout, thread);
#endif	NCPUS > 1
	}
	/*
	 *	Accumulate times for dead threads in task.
	 */

	thread_read_times(thread, &user_time, &system_time);
	time_value_add(&task->total_user_time, &user_time);
	time_value_add(&task->total_system_time, &system_time);

	/*
	 *	Remove thread from task list and processor_set threads list.
	 */
	task->thread_count--;
	queue_remove(&task->thread_list, thread, thread_t, thread_list);

	pset_remove_thread(pset, thread);

	thread_unlock(thread);		/* no more references - safe */
	simple_unlock(&task->thread_list_lock);
	(void) splx(s);
	pset_unlock(pset);
	pset_deallocate(pset);
	task_unlock(task);

        /*
	 * XXX Unix compatibility
	 * It is guaranteed that if a thread has a valid cred, it has
	 * a reference to it.  So, unreference it before deallocating thread.
	 */
	if (cr != NULL && cr != NOCRED)
		crfree(cr);

        /*
	 * XXX SVR4 Unix compatibility
	 * Remove all sigqueue structures.
	 * There should probably be a uarea_free() routine
	 * to put this kind of stuff (the cred stuff and the
	 * sigqueue stuff) into  - a routine that is the 
	 * opposite of uarea_init().
	 */
	if (thread->u_address.utask->uu_procp) {
		sig_lock_simple(thread->u_address.utask->uu_procp);
		sigq_remove_all(&thread->u_address.uthread->uu_sigqueue, 
				ALL_SIGQ_SIGS);
		SIGQ_FREE(thread->u_address.uthread->uu_curinfo);
		sig_unlock(thread->u_address.utask->uu_procp);
	} else {
		sigq_remove_all(&thread->u_address.uthread->uu_sigqueue, 
				ALL_SIGQ_SIGS);
		SIGQ_FREE(thread->u_address.uthread->uu_curinfo);
	}

	/*
	 *	Clean up global variables
	 */

	if (thread->tmp_address != (vm_offset_t) 0)
		kmem_free(kernel_map, thread->tmp_address, PAGE_SIZE);
	if (thread->tmp_object != VM_OBJECT_NULL)
		vm_object_deallocate(thread->tmp_object);

	/*
	 *	A couple of quick sanity checks
	 */

	if (thread == current_thread()) {
	    panic("thread deallocating itself");
	}
	if ((thread->state & ~(TH_RUN | TH_SWAPPED)) != TH_SUSP)
		panic("unstopped thread destroyed!");

	/*
	 *	Deallocate the task reference, since we know the thread
	 *	is not running.
	 */
	task_deallocate(thread->task);			/* may block */

	/*
	 *	Since there are no references left, we need not worry about
	 *	locking the thread.
	 */
	if (thread->state & TH_SWAPPED)
		thread_doswapin(thread);

	/*
	 *	Clean up any machine-dependent resources.
	 */
	pcb_terminate(thread);

	stack_free(thread->kernel_stack);
	zfree(thread_zone, (vm_offset_t) thread);
}
	
/*
 *	thread_deallocate_interrupt:
 *
 *	XXX special version of thread_deallocate that can be called from
 *	XXX interrupt level to solve a nasty problem in psignal().
 */

void thread_deallocate_interrupt(thread)
	register thread_t	thread;
{
	int		s;

	if (thread == THREAD_NULL)
		return;

	/*
	 *	First, check for new count > 0 (the common case).
	 *	Only the thread needs to be locked.
	 */
	s = splsched();
	thread_lock(thread);
	if (--thread->ref_count > 0) {
		thread_unlock(thread);
		(void) splx(s);
		return;
	}

	/*
	 *	Count is zero, but we can't actually free the thread
	 *	because that requires a task and a pset lock that
	 *	can't be held at interrupt level.  Since this was called
	 *	from interrupt level, we know the thread's reference to
	 *	itself is gone,	so it can't be running.  Similarly we know
	 *	it's not on the reaper's queue (else it would have
	 *	an additional reference).  Hence we can just put it
	 *	on the reaper's queue so that the reaper will get rid of
	 *	our reference for us.  We have to put that reference
	 *	back (of course).  As long as the thread is on the
	 *	reaper's queue, it will have a reference and hence can't
	 *	be requeued.
	 */

	thread->ref_count = 1;

	simple_lock(&reaper_lock);
	enqueue_tail(&reaper_queue, (queue_entry_t) thread);
	simple_unlock(&reaper_lock);

	thread_unlock(thread);
	(void) splx(s);

	thread_wakeup((vm_offset_t)&reaper_queue);
}


void thread_reference(thread)
	register thread_t	thread;
{
	int		s;

	if (thread == THREAD_NULL)
		return;

	s = splsched();
	thread_lock(thread);
	thread->ref_count++;
	thread_unlock(thread);
	(void) splx(s);
}

/*
 *	thread_terminate:
 *
 *	Permanently stop execution of the specified thread.
 *
 *	A thread to be terminated must be allowed to clean up any state
 *	that it has before it exits.  The thread is broken out of any
 *	wait condition that it is in, and signalled to exit.  It then
 *	cleans up its state and calls thread_halt_self on its way out of
 *	the kernel.  The caller waits for the thread to halt, terminates
 *	its IPC state, and then deallocates it.
 *
 *	If the caller is the current thread, it must still exit the kernel
 *	to clean up any state (thread and port references, messages, etc).
 *	When it exits the kernel, it then terminates its IPC state and
 *	queues itself for the reaper thread, which will wait for the thread
 *	to stop and then deallocate it.  (A thread cannot deallocate itself,
 *	since it needs a kernel stack to execute.)
 */
kern_return_t thread_terminate(thread)
	register thread_t	thread;
{
	register thread_t	cur_thread = current_thread();
	register task_t		cur_task;
#if	MACH_KM
	register monitor_t      cur_monitor;
#endif	MACH_KM
	int	s;

	if (thread == THREAD_NULL)
		return(KERN_INVALID_ARGUMENT);

	/* New for /proc */
	/* If the stop-on-thread-terminate bit is set, stop the task */
	if(thread->task->procfs.pr_qflags & PRFS_STOPTTERM) {
		thread->task->procfs.pr_flags |=
			(PR_STOPPED | PR_ISTOP | PR_PCINVAL);
		thread->task->procfs.pr_why = PRFS_STOPTTERM;
		thread->task->procfs.pr_what = (long )thread;
		wakeup(&thread->task->procfs);
		thread_suspend(thread);
		thread_block(thread);
	}
	/* End /proc code */

	/*
	 *	Break IPC control over the thread.
	 */
	ipc_thread_disable(thread);

	if (thread == cur_thread) {

	    /*
	     *	Current thread will queue itself for reaper when
	     *	exiting kernel.
	     */
	    s = splsched();
	    thread_lock(thread);
	    if (thread->active) {
		    thread->active = FALSE;
		    thread_ast_set(thread, AST_TERMINATE);
	    }
#if	MACH_KM
	    /*
	     * Check to see if the thread being monitored.
	     * If so, disconnect the thread from the monitor.
	     */
	    
	    if (thread-> monitor_obj != MONITOR_NULL) {
		cur_monitor = thread-> monitor_obj;
		thread-> monitor_obj = MONITOR_NULL;
		thread-> monitor_id = MONITOR_NO_ID;
		thread_unlock(thread);
		thread_monitor_dequeue(thread, cur_monitor);
		splx(s);
		monitor_deallocate(cur_monitor);   
	    }
	    else {
		thread_unlock(thread);
		splx(s);
	    }
#else	MACH_KM
	    thread_unlock(thread);
	    splx(s);
#endif	MACH_KM
	    aston();
	    return(KERN_SUCCESS);
	}

	/*
	 *	Lock both threads and the current task
	 *	to check termination races and prevent deadlocks.
	 */
	cur_task = current_task();
	task_lock(cur_task);
	s = splsched();
	if ((int)thread < (int)cur_thread) {
		thread_lock(thread);
		thread_lock(cur_thread);
	}
	else {
		thread_lock(cur_thread);
		thread_lock(thread);
	}

	/*
	 *	If the current thread being terminated, help out.
	 */
	if ((!cur_task->active) || (!cur_thread->active)) {
		thread_unlock(cur_thread);
		thread_unlock(thread);
		(void) splx(s);
		task_unlock(cur_task);
		thread_terminate(cur_thread);
		return(KERN_FAILURE);
	}
    
	thread_unlock(cur_thread);
	task_unlock(cur_task);

	/*
	 *	Terminate victim thread.
	 */
	if (!thread->active) {
		/*
		 *	Someone else got there first.
		 */
		thread_unlock(thread);
		(void) splx(s);
		return(KERN_FAILURE);
	}

	thread->active = FALSE;

#if	MACH_KM
	/*
	 * Check to see if the thread being monitored.
	 * If so, disconnect the thread from the monitor.
	 */
	
	if (thread-> monitor_obj != MONITOR_NULL) {
	    cur_monitor = thread-> monitor_obj;
	    thread-> monitor_obj = MONITOR_NULL;
	    thread-> monitor_id = MONITOR_NO_ID;
	    thread_unlock(thread);
	    splx(s);
	    thread_monitor_dequeue(thread, cur_monitor);
	}
	else {
	    thread_unlock(thread);
	    (void) splx(s);
	}
#else	MACH_KM
	thread_unlock(thread);
	(void) splx(s);
#endif	MACH_KM

#if	MACH_HOST
	/*
	 *	Reassign thread to default pset if needed.
	 */
	thread_freeze(thread);
	if (thread->processor_set != &default_pset) {
		thread_doassign(thread, &default_pset, FALSE);
	}
#endif	MACH_HOST

	/*
	 *	Halt the victim at the clean point.
	 */
	(void) thread_halt(thread, TRUE);
#if	MACH_HOST
	thread_unfreeze(thread);
#endif	MACH_HOST
	/*
	 *	Shut down the victims IPC and deallocate its
	 *	reference to itself.
	 */
	ipc_thread_terminate(thread);
	thread_deallocate(thread);
	return(KERN_SUCCESS);
}

/*
 *	thread_force_terminate:
 *
 *	Version of thread_terminate called by task_terminate.  thread is
 *	not the current thread.  task_terminate is the dominant operation,
 *	so we can force this thread to stop.
 */
void
thread_force_terminate(thread)
register thread_t	thread;
{
	boolean_t	deallocate_here = FALSE;
#if	MACH_KM
	register monitor_t      cur_monitor;
#endif	MACH_KM
	int s;

	/* New for /proc */
	/* If the stop-on-thread-terminate bit is set, stop the task */
	if(thread->task->procfs.pr_qflags & PRFS_STOPTTERM) {
		thread->task->procfs.pr_flags |=
			(PR_STOPPED | PR_ISTOP | PR_PCINVAL);
		thread->task->procfs.pr_why = PRFS_STOPTTERM;
		thread->task->procfs.pr_what = (long )thread;
		wakeup(&thread->task->procfs);
		thread_suspend(thread);
		thread_block(thread);
	}
	/* End /proc code */

	ipc_thread_disable(thread);

#if	MACH_HOST
	/*
	 *	Reassign thread to default pset if needed.
	 */
	thread_freeze(thread);
	if (thread->processor_set != &default_pset)
		thread_doassign(thread, &default_pset, FALSE);
#endif	MACH_HOST

	s = splsched();
	thread_lock(thread);
	deallocate_here = thread->active;
	thread->active = FALSE;
#if	MACH_KM
	/*
	 * Check to see if the thread being monitored.
	 * If so, disconnect the thread from the monitor.
	 */
	
	if (thread-> monitor_obj != MONITOR_NULL) {
	    cur_monitor = thread-> monitor_obj;
	    thread-> monitor_obj = MONITOR_NULL;
	    thread-> monitor_id = MONITOR_NO_ID;
	    thread_unlock(thread);
	    thread_monitor_dequeue(thread, cur_monitor);
	    splx(s);
	    monitor_deallocate(cur_monitor);    
	}
	else {
	    thread_unlock(thread);
	    splx(s);
	}
#else	MACH_KM
	thread_unlock(thread);
	(void) splx(s);
#endif	MACH_KM

	(void) thread_halt(thread, TRUE);
	ipc_thread_terminate(thread);

#if	MACH_HOST
	thread_unfreeze(thread);
#endif	MACH_HOST

	if (deallocate_here)
		thread_deallocate(thread);
}


/*
 *	Halt a thread at a clean point, leaving it suspended.
 *
 *	must_halt indicates whether thread must halt.
 *
 */
kern_return_t thread_halt(thread, must_halt)
	register thread_t	thread;
	boolean_t		must_halt;
{
	register thread_t	cur_thread = current_thread();
	register kern_return_t	ret;
	int	s;

	if (thread == cur_thread)
		panic("thread_halt: trying to halt current thread.");
	/*
	 *	If must_halt is FALSE, then a check must be made for
	 *	a cycle of halt operations.
	 */
	if (!must_halt) {
		/*
		 *	Grab both thread locks.
		 */
		s = splsched();
		if ((int)thread < (int)cur_thread) {
			thread_lock(thread);
			thread_lock(cur_thread);
		}
		else {
			thread_lock(cur_thread);
			thread_lock(thread);
		}

		/*
		 *	If target thread is already halted, grab a hold
		 *	on it and return.
		 */
		if (thread->halted) {
			thread->suspend_count++;
			thread_unlock(cur_thread);
			thread_unlock(thread);
			(void) splx(s);
			return(KERN_SUCCESS);
		}

		/*
		 *	If someone is trying to halt us, we have a potential
		 *	halt cycle.  Break the cycle by interrupting anyone
		 *	who is trying to halt us, and causing this operation
		 *	to fail; retry logic will only retry operations
		 *	that cannot deadlock.  (If must_halt is TRUE, this
		 *	operation can never cause a deadlock.)
		 */
		if (cur_thread->ast & AST_HALT) {
			thread_wakeup_with_result(
					(vm_offset_t)&cur_thread->wake_active,
					THREAD_INTERRUPTED);
			thread_unlock(thread);
			thread_unlock(cur_thread);
			(void) splx(s);
			return(KERN_FAILURE);
		}

		thread_unlock(cur_thread);
	
	}
	else {
		/*
		 *	Lock thread and check whether it is already halted.
		 */
		s = splsched();
		thread_lock(thread);
		if (thread->halted) {
			thread->suspend_count++;
			thread_unlock(thread);
			(void) splx(s);
			return(KERN_SUCCESS);
		}
	}

	/*
	 *	Suspend thread - inline version of thread_hold() because
	 *	thread is already locked.
	 */
	thread->suspend_count++;
	thread->state |= TH_SUSP;

	/*
	 *	If someone else is halting it, wait for that to complete.
	 *	Fail if wait interrupted and must_halt is false.
	 */
        while ((thread->ast & AST_HALT) && (!thread->halted)) {
                thread->wake_active = TRUE;
                thread_sleep((vm_offset_t) &thread->wake_active,
                        simple_lock_addr(thread->lock), TRUE);

                if ((current_thread()->wait_result != THREAD_AWAKENED)
                    && !(must_halt)) {
                        splx(s);
                        thread_release(thread);
                        return(KERN_FAILURE);
                }
                thread_lock(thread);
        }
        if (thread->halted) {
                /*
                 *      It's possible that the AST simply got cleared.
                 *      This time the thread really halted.
                 */
                thread_unlock(thread);
                splx(s);
                return(KERN_SUCCESS);
        }
	/*
	 *	Otherwise, have to do it ourselves.
	 */
		
	thread_ast_set(thread, AST_HALT);

	while (TRUE) {
	  	/*
		 *	Wait for thread to stop.
		 */
		thread_unlock(thread);
		(void) splx(s);

		ret = thread_dowait(thread, must_halt);

		/*
		 *	If the dowait failed, so do we.
		 */
		if (ret != KERN_SUCCESS) {
                        boolean_t need_wakeup;

                        /*
                         *      Clean up
                         */
                        thread_release(thread);
                        s = splsched();
                        thread_lock(thread);
                        thread_ast_clear(thread, AST_HALT);
                        /* The following is supposed to be an assignment */
                        if (need_wakeup = thread->wake_active)
                                thread->wake_active = FALSE;
                        thread_unlock(thread);
                        splx(s);
                        if (need_wakeup)
                                thread_wakeup((vm_offset_t) &thread->wake_active);
                        return(ret);
                }
		/*
		 *	Clear any interruptible wait.
		 */
		clear_wait(thread, THREAD_INTERRUPTED, TRUE);

		/*
		 *	If the thread's at a clean point, we're done.
		 *	Don't need a lock because it really is stopped.
		 */
		if (thread->halted) {
			return(KERN_SUCCESS);
		}

		/*
		 *	Force the thread to stop at a clean
		 *	point, and arrange to wait for it.
		 *
		 *	Set it running, so it can notice.  Override
		 *	the suspend count.  We know that the thread
		 *	is suspended and not waiting.
		 *
		 *	Since the thread may hit an interruptible wait
		 *	before it reaches a clean point, we must force it
		 *	to wake us up when it does so.  This involves some
		 *	trickery:
		 *	  We mark the thread SUSPENDED so that thread_block
		 *	will suspend it and wake us up.
		 *	  We mark the thread RUNNING so that it will run.
		 *	  We mark the thread UN-INTERRUPTIBLE (!) so that
		 *	some other thread trying to halt or suspend it won't
		 *	take it off the run queue before it runs.  Since
		 *	dispatching a thread (the tail of thread_block) marks
		 *	the thread interruptible, it will stop at the next
		 *	context switch or interruptible wait.
		 */

		s = splsched();
		thread_lock(thread);
		switch (thread->state) {

		    case TH_SUSP | TH_SWAPPED:
			thread->state = TH_SUSP | TH_SWAPPED | TH_RUN;
			thread->interruptible = FALSE;
			thread_swapin(thread);
			break;

		    case TH_SUSP:
			thread->state = TH_SUSP | TH_RUN;
			thread->interruptible = FALSE;
			thread_setrun(thread, FALSE);
			break;

		    default:
			panic("thread_halt");
		}

		/*
		 *	Continue loop and wait for thread to stop.
		 */
	}
}

/*
 *	Thread calls this routine on exit from the kernel when it
 *	notices a halt request.
 */
void	thread_halt_self()
{
	register thread_t	thread = current_thread();
	int	s;

	if (thread->ast & AST_TERMINATE) {
		/*
		 *	Thread is terminating itself.  Shut
		 *	down IPC, then queue it up for the
		 *	reaper thread.
		 */
		ipc_thread_terminate(thread);

		thread_hold(thread);

		s = splsched();
		simple_lock(&reaper_lock);
		enqueue_tail(&reaper_queue, (queue_entry_t) thread);
		simple_unlock(&reaper_lock);

		thread_lock(thread);
		thread->halted = TRUE;
		thread_unlock(thread);
		(void) splx(s);

		thread_wakeup((vm_offset_t)&reaper_queue);
		thread_block();
		panic("the zombie walks!");
		/*NOTREACHED*/
	}
	else {
		/*
		 *	Thread was asked to halt - show that it
		 *	has done so.
		 */
		s = splsched();
		thread_lock(thread);
		thread->halted = TRUE;
		thread_ast_clear(thread, AST_HALT);
		thread_unlock(thread);
		splx(s);
		thread_block();
		/*
		 *	thread_release resets thread->halted.
		 */
	}
}

/*
 *	thread_hold:
 *
 *	Suspend execution of the specified thread.
 *	This is a recursive-style suspension of the thread, a count of
 *	suspends is maintained.
 */
void thread_hold(thread)
	register thread_t	thread;
{
	int			s;

	s = splsched();
	thread_lock(thread);
	thread->suspend_count++;
	thread->state |= TH_SUSP;
	thread_unlock(thread);
	(void) splx(s);
}

/*
 *	thread_dowait:
 *
 *	Wait for a thread to actually enter stopped state.
 *
 *	must_halt argument indicates if this may fail on interruption.
 *	This is FALSE only if called from thread_abort via thread_halt.
 */
kern_return_t
thread_dowait(thread, must_halt)
	register thread_t	thread;
	boolean_t		must_halt;
{
	register boolean_t	need_wakeup;
	register kern_return_t	ret = KERN_SUCCESS;
	int			s;

	/*
	 *	If we are requested to wait for the thread to really
	 *	be stopped, and that thread is us, we need to context
	 *	switch (giving up execution, stopping ourselves).
	 */

	if (thread == current_thread()) {
		thread_block();
		return(KERN_SUCCESS);
	}

	/*
	 *	If a thread is not interruptible, it may not be suspended
	 *	until it becomes interruptible.  In this case, we wait for
	 *	the thread to stop itself, and indicate that we are waiting
	 *	for it to stop so that it can wake us up when it does stop.
	 *
	 *	If the thread is interruptible, we may be able to suspend
	 *	it immediately.  There are several cases:
	 *
	 *	1) The thread is already stopped (trivial)
	 *	2) The thread is runnable (marked RUN and on a run queue).
	 *	   We pull it off the run queue and mark it stopped.
	 *	3) The thread is running.  We wait for it to stop.
	 */

	need_wakeup = FALSE;
	s = splsched();
	thread_lock(thread);
	switch(thread->state) {
	    case TH_SUSP:
	    case TH_SUSP | TH_SWAPPED:
	    case TH_SUSP | TH_WAIT | TH_SWAPPED:
		/*
		 *	We win!  Since thread is suspended (without any
		 *	other states) or swapped out, it must be
		 *	interruptible.
		 */
		break;

	    case TH_RUN | TH_SUSP:
		/*
		 *	If the thread is interruptible, and we can pull
		 *	it off a runq, stop it here.
		 */
		if ((thread->interruptible) &&
		    (rem_runq(thread) != RUN_QUEUE_NULL)) {
			thread->state = TH_SUSP;
			need_wakeup = thread->wake_active;
			thread->wake_active = FALSE;
			break;
		}
		/*
		 *	Fall through to wait for thread to stop.
		 */

	    case TH_RUN | TH_SUSP | TH_SWAPPED:
	    case TH_RUN | TH_WAIT | TH_SUSP:
	    case TH_WAIT | TH_SUSP:
		/*
		 *	Wait for the thread to stop, or sleep interruptibly
		 *	(thread_block will stop it in the latter case).
		 *	Check for failure if interrupted.
		 */
		while ((thread->state & (TH_RUN | TH_SUSP)) != TH_SUSP ||
		    !thread->interruptible) {
			thread->wake_active = TRUE;
			thread_sleep((vm_offset_t)&thread->wake_active,
				simple_lock_addr(thread->lock), FALSE);
			thread_lock(thread);
			if ((current_thread()->wait_result !=
			    THREAD_AWAKENED) && !(must_halt)) {
				ret = KERN_FAILURE;
				break;
			}
		    }
		break;
	}
	thread_unlock(thread);
	(void) splx(s);

	if (need_wakeup)
	    thread_wakeup((vm_offset_t)&thread->wake_active);

	return(ret);
}
void thread_release(thread)
	register thread_t	thread;
{
	int			s;

	s = splsched();
	thread_lock(thread);
	if (--thread->suspend_count == 0) {

		thread->halted = FALSE;
		thread->state &= ~TH_SUSP;
		switch (thread->state & (TH_WAIT | TH_RUN | TH_SWAPPED)) {

		    case TH_SWAPPED:
			thread->state |= TH_RUN;
			thread_swapin(thread);
			break;

		    case 0:	/* was only suspended */
			thread->state |= TH_RUN;
			thread_setrun(thread, TRUE);
			break;

		    default:
			break;
		}
	}
	thread_unlock(thread);
	(void) splx(s);

}

kern_return_t thread_suspend(thread)
	register thread_t	thread;
{
	register boolean_t	hold;
	int			spl;

	if (thread == THREAD_NULL)
		return(KERN_INVALID_ARGUMENT);

	hold = FALSE;
	spl = splsched();
	thread_lock(thread);
	if (thread->user_stop_count++ == 0) {
		hold = TRUE;
		thread->suspend_count++;
		thread->state |= TH_SUSP;
	}
	thread_unlock(thread);
	(void) splx(spl);

	/*
	 *	Now wait for the thread if necessary.
	 */
	if (hold)
		(void) thread_dowait(thread, TRUE);

	return(KERN_SUCCESS);
}

kern_return_t thread_resume(thread)
	register thread_t	thread;
{
	register kern_return_t	ret;
	int			s;

	if (thread == THREAD_NULL)
		return(KERN_INVALID_ARGUMENT);

	ret = KERN_SUCCESS;

	s = splsched();
	thread_lock(thread);
	if (thread->user_stop_count > 0) {
	    if (--thread->user_stop_count == 0) {
		if (--thread->suspend_count == 0) {

		    thread->halted = FALSE;
		    thread->state &= ~TH_SUSP;
		    switch (thread->state & (TH_WAIT | TH_RUN | TH_SWAPPED)) {

		        case TH_SWAPPED:
			    thread->state |= TH_RUN;
			    thread_swapin(thread);
			    break;

		        case 0:	/* was only suspended */
			    thread->state |= TH_RUN;
			    thread_setrun(thread, TRUE);
			    break;

		        default:
			    break;
			}
		}

	    }
	}
	else {
		ret = KERN_FAILURE;
	}
	thread_unlock(thread);
	(void) splx(s);

	return(ret);
}

/*
 *	Return thread's machine-dependent state.
 */
kern_return_t thread_get_state(thread, flavor, old_state, old_state_count)
	register thread_t	thread;
	int			flavor;
	thread_state_t		old_state;	/* pointer to OUT array */
	unsigned int		*old_state_count;	/*IN/OUT*/
{
	kern_return_t		ret;

	if (thread == THREAD_NULL || thread == current_thread()) {
		return (KERN_INVALID_ARGUMENT);
	}

	thread_hold(thread);
	(void) thread_dowait(thread, TRUE);

	ret = thread_getstatus(thread, flavor, old_state, old_state_count);

	thread_release(thread);
	return(ret);
}

/*
 *	Change thread's machine-dependent state.
 */
kern_return_t thread_set_state(thread, flavor, new_state, new_state_count)
	register thread_t	thread;
	int			flavor;
	thread_state_t		new_state;
	unsigned int		new_state_count;
{
	kern_return_t		ret;

	if (thread == THREAD_NULL || thread == current_thread()) {
		return (KERN_INVALID_ARGUMENT);
	}

	thread_hold(thread);
	(void) thread_dowait(thread, TRUE);

	ret = thread_setstatus(thread, flavor, new_state, new_state_count);

	thread_release(thread);
	return(ret);
}

kern_return_t
thread_info(thread, flavor, thread_info_out,
	    thread_info_count)
register thread_t	thread;
int			flavor;
thread_info_t		thread_info_out;     /* pointer to OUT array */
unsigned int		*thread_info_count;  /*IN/OUT*/
{
	int	s, state, flags;

	if (thread == THREAD_NULL)
		return(KERN_INVALID_ARGUMENT);

	if (flavor == THREAD_BASIC_INFO) {
	    register thread_basic_info_t	basic_info;

	    if (*thread_info_count < THREAD_BASIC_INFO_COUNT) {
		return(KERN_INVALID_ARGUMENT);
	    }

	    basic_info = (thread_basic_info_t) thread_info_out;

	    s = splsched();
	    thread_lock(thread);

	    /*
	     *	Update lazy-evaluated scheduler info because someone wants it.
	     */
	    if ((thread->state & TH_RUN) == 0 &&
		thread->sched_stamp != sched_tick)
		    update_usage(thread);

	    /* fill in info */
	    thread_read_times(thread,
			&basic_info->user_time,
			&basic_info->system_time);

/*
 * RT:  Always return priorities in the range 0..(NRQS_MAX-1), where 0 is
 * the highest priority.  If fewer than NRQS_MAX queues are being used, bias
 * the returned values.  This makes it easier for programs like ps to deal
 * with the priority information between RT and non-RT kernels.
 */
	    basic_info->base_priority = thread->priority + NRQS_ADJUST;
	    basic_info->cur_priority  = thread->sched_pri + NRQS_ADJUST;

	    /*
	     *	To calculate cpu_usage, first correct for timer rate,
	     *	then for 5/8 ageing.  The correction factor [3/5] is
	     *	(1/(5/8) - 1).
	     */
	    basic_info->cpu_usage = thread->cpu_usage /
					(TIMER_RATE/TH_USAGE_SCALE);
	    basic_info->cpu_usage = (basic_info->cpu_usage * 3) / 5;
#if	SIMPLE_CLOCK
	    /*
	     *	Clock drift compensation.
	     */
	    basic_info->cpu_usage =
		(basic_info->cpu_usage * 1000000)/sched_usec;
#endif	SIMPLE_CLOCK

	    if (thread->state & TH_SWAPPED)
		flags = TH_FLAGS_SWAPPED;
	    else if (thread->state & TH_IDLE)
		flags = TH_FLAGS_IDLE;
	    else
		flags = 0;

	    if (thread->halted)
		state = TH_STATE_HALTED;
	    else
	    if (thread->state & TH_RUN)
		state = TH_STATE_RUNNING;
	    else
	    if (!thread->interruptible)
		state = TH_STATE_UNINTERRUPTIBLE;
	    else
	    if (thread->state & TH_SUSP)
		state = TH_STATE_STOPPED;
	    else
	    if (thread->state & TH_WAIT)
		state = TH_STATE_WAITING;
	    else
		state = 0;		/* ? */

	    basic_info->run_state = state;
	    basic_info->flags = flags;
	    basic_info->suspend_count = thread->user_stop_count;
	    if (state == TH_STATE_RUNNING)
		basic_info->sleep_time = 0;
	    else
		basic_info->sleep_time = sched_tick - thread->sleep_stamp;

	    if (thread->state & TH_WAIT)
                basic_info->wait_event = thread->wait_event;
            else
                basic_info->wait_event = 0;
                    
            if (thread->wait_mesg)
                bcopy(thread->wait_mesg, basic_info->wait_mesg, WMESGLEN);
            else
                bzero(basic_info->wait_mesg, WMESGLEN);
            
	    thread_unlock(thread);
	    splx(s);

	    *thread_info_count = THREAD_BASIC_INFO_COUNT;
	    return(KERN_SUCCESS);
	}
	else if (flavor == THREAD_SCHED_INFO) {
	    register thread_sched_info_t	sched_info;

	    if (*thread_info_count < THREAD_SCHED_INFO_COUNT) {
		return(KERN_INVALID_ARGUMENT);
	    }

	    sched_info = (thread_sched_info_t) thread_info_out;

	    s = splsched();
	    thread_lock(thread);

	    sched_info->policy = thread->policy;
#if RT_SCHED
	    /*
	     * Report RR quantum as well as FIXEDPRI quantum.  Note that the
	     * sched_data field is set to MAX_INT as used as a fake FIFO
	     * quantum to simplify some code, but this should not be made
	     * visible to the user.
	     */
	    if (thread->policy & (POLICY_FIXEDPRI|POLICY_RR)) {
#else /* RT_SCHED */
	    if (thread->policy & POLICY_FIXEDPRI) {
#endif /* RT_SCHED */
		sched_info->data = (thread->sched_data * tick)/1000;
	    }
	    else {
		sched_info->data = 0;
	    }

/*
 * RT:  Always return priorities in the range 0..(NRQS_MAX-1), where 0 is
 * the highest priority.  If fewer than NRQS_MAX queues are being used, bias
 * the returned values.  This makes it easier for programs like ps to deal
 * with the priority information between RT and non-RT kernels.
 */
	    sched_info->base_priority = thread->priority + NRQS_ADJUST;
	    sched_info->max_priority  = thread->max_priority + NRQS_ADJUST;
	    sched_info->cur_priority  = thread->sched_pri + NRQS_ADJUST;
	    
	    sched_info->depressed = (thread->depress_priority >= 0);
	    sched_info->depress_priority = 
	      thread->depress_priority + NRQS_ADJUST;

	    thread_unlock(thread);
	    splx(s);

	    *thread_info_count = THREAD_BASIC_INFO_COUNT;
	    return(KERN_SUCCESS);
	}

	return(KERN_INVALID_ARGUMENT);
}

kern_return_t	thread_abort(thread)
	register thread_t	thread;
{
	if (thread == THREAD_NULL || thread == current_thread()) {
		return (KERN_INVALID_ARGUMENT);
	}

	/*
	 *	Try to force the thread to a clean point
	 *	If the halt operation fails return KERN_ABORTED.
	 *	ipc code will convert this to an ipc interrupted error code.
	 */
	if (thread_halt(thread, FALSE) != KERN_SUCCESS)
		return(KERN_ABORTED);

	/*
	 *	If the thread was in an exception, abort that too.
	 */
	thread_exception_abort(thread);

	/*
	 *	Then set it going again.
	 */
	thread_release(thread);

	return(KERN_SUCCESS);
}

kern_return_t thread_get_special_port(thread, which_port, port)
	register thread_t	thread;
	int		which_port;
	thread_port_t	*port;
{
	register thread_port_t	*portp;

	if (thread == THREAD_NULL)
		return(KERN_INVALID_ARGUMENT);

	switch (which_port) {
	    case THREAD_KERNEL_PORT:
		portp = &thread->thread_tself;
		break;
	    case THREAD_REPLY_PORT:
		portp = &thread->thread_reply;
		break;
	    case THREAD_EXCEPTION_PORT:
		portp = &thread->exception_port;
		break;
	    default:
		return(KERN_INVALID_ARGUMENT);
	}

	ipc_thread_lock(thread);
	if (thread->thread_self == PORT_NULL) {
		/* thread's IPC already inactive */
		ipc_thread_unlock(thread);
		return(KERN_FAILURE);
	}
	
	if ((*port = *portp) != PORT_NULL) {
		port_reference(*portp);
	}
	ipc_thread_unlock(thread);

	return(KERN_SUCCESS);
}

kern_return_t thread_set_special_port(thread, which_port, port)
	register thread_t	thread;
	int		which_port;
	thread_port_t	port;
{
	register thread_port_t	*portp;
	register thread_port_t	old_port;

	if (thread == THREAD_NULL)
		return(KERN_INVALID_ARGUMENT);

	switch (which_port) {
	    case THREAD_KERNEL_PORT:
		portp = &thread->thread_tself;
		break;
	    case THREAD_REPLY_PORT:
		portp = &thread->thread_reply;
		break;
	    case THREAD_EXCEPTION_PORT:
		portp = &thread->exception_port;
		break;
	    default:
		return(KERN_INVALID_ARGUMENT);
	}

	ipc_thread_lock(thread);
	if (thread->thread_self == PORT_NULL) {
		/* thread's IPC already inactive */
		ipc_thread_unlock(thread);
		return(KERN_FAILURE);
	}
	
	old_port = *portp;
	if ((*portp = port) != PORT_NULL)
		port_reference(port);

	ipc_thread_unlock(thread);

	if (old_port != PORT_NULL)
		port_release(old_port);

	return(KERN_SUCCESS);
}

/*
 *	kernel_thread:
 *
 *	Start up a kernel thread in the specified task.
 */

thread_t kernel_thread(task, start)
	task_t	task;
	void	(*start)();
{
	thread_t	thread;

	if (thread_create(task, &thread) != KERN_SUCCESS) {
		panic("kernel_thread: can't create a kernel thread");
	}
	thread_start(thread, start, THREAD_SYSTEMMODE);
#if RT_SCHED
	/*
	 * In case somebody is debugging security.
	 */
	thread->max_priority = BASEPRI_HIGHEST;
#endif /* RT_SCHED */
	thread->priority = BASEPRI_SYSTEM;
	thread->sched_pri = BASEPRI_SYSTEM;
	thread->ipc_kernel = TRUE;
	(void) thread_resume(thread);
	return(thread);
}

/*
 * Function:
 * thread_t kernel_thread_w_arg(task_t, void (*)(), void *)
 *
 * Begin a kernel thread at a given entry point with a specified argument.
 * Returns the thread_t for the created thread. The argument must be
 * collected from the 'thread->reply_port' field of the created thread
 * structure prior to performing any IPC things. This groty hack was 
 * copied from the vnode pager startup code.
 */
thread_t kernel_thread_w_arg(task, start, arg)
	task_t task;
	void (*start)();
	void *arg;
{
	thread_t thread;

        if (thread_create(task, &thread) != KERN_SUCCESS) {
                panic("kernel_thread_w_arg: can't create kernel thread");
	}

        thread_start(thread, start, THREAD_SYSTEMMODE);

        thread->priority = BASEPRI_SYSTEM;
        thread->sched_pri = BASEPRI_SYSTEM;
        thread->ipc_kernel = TRUE;

	/* Set argument for later collection by the started thread. 
	 * It must reset this to PORT_NULL after copying the value
	 * to a safe place. */
        thread->reply_port = (port_t) arg;

        if (thread_resume(thread) != KERN_SUCCESS) {
                panic("kernel_thread_w_arg: can't start thread");
	}
	return(thread);
}

/*
 *	reaper_thread:
 *
 *	This kernel thread runs forever looking for threads to destroy
 *	(when they request that they be destroyed, of course).
 */
void reaper_thread()
{
	spl0();

	for (;;) {
		register thread_t thread;
		register int s;

		s = splsched();
		simple_lock(&reaper_lock);

		while ((thread = (thread_t) dequeue_head(&reaper_queue))
					== THREAD_NULL) {
			assert_wait((vm_offset_t)&reaper_queue, FALSE);
			simple_unlock(&reaper_lock);
			thread_block();
			simple_lock(&reaper_lock);
		}

		simple_unlock(&reaper_lock);
		(void) splx(s);

		(void) thread_dowait(thread, TRUE);	/* may block */
		thread_deallocate(thread);	/* may block */
	}
}

#if	MACH_HOST
/*
 *	thread_assign:
 *
 *	Change processor set assignment.
 *	Caller must hold an extra reference to the thread (if this is
 *	called directly from the ipc interface, this is an operation
 *	in progress reference).  Caller must hold no locks -- this may block.
 */

kern_return_t
thread_assign(thread, new_pset)
thread_t	thread;
processor_set_t	new_pset;
{
	if (thread == THREAD_NULL || new_pset == PROCESSOR_SET_NULL) {
		return(KERN_INVALID_ARGUMENT);
	}

	thread_freeze(thread);
	thread_doassign(thread, new_pset, TRUE);
}

/*
 *	thread_freeze:
 *
 *	Freeze thread's assignment.  Prelude to assigning thread.
 *	Only one freeze may be held per thread.  
 */
void
thread_freeze(thread)
thread_t	thread;
{
	int s;
	/*
	 *	Freeze the assignment, deferring to a prior freeze.
	 */
	s = splsched();
	thread_lock(thread);
	while (thread->may_assign == FALSE) {
		thread->assign_active = TRUE;
		assert_wait((vm_offset_t)&thread->assign_active, TRUE);
		thread_unlock(thread);
		splx(s);
		thread_block();
		s = splsched();
		thread_lock(thread);
	}
	thread->may_assign = FALSE;
	thread_unlock(thread);
	(void) splx(s);

}

/*
 *	thread_unfreeze: release freeze on thread's assignment.
 */
void
thread_unfreeze(thread)
thread_t	thread;
{
	int 	s;

	s = splsched();
	thread_lock(thread);
	thread->may_assign = TRUE;
	if (thread->assign_active) {
		thread->assign_active = FALSE;
		thread_wakeup((vm_offset_t)&thread->assign_active);
	}
	thread_unlock(thread);
	splx(s);
}

/*
 *	thread_doassign:
 *
 *	Actually do thread assignment.  thread_will_assign must have been
 *	called on the thread.  release_freeze argument indicates whether
 *	to release freeze on thread.
 */

void
thread_doassign(thread, new_pset, release_freeze)
register thread_t		thread;
register processor_set_t	new_pset;
boolean_t			release_freeze;
{
	register processor_set_t	pset;
	register boolean_t		old_empty, new_empty;
	boolean_t	recompute_pri = FALSE;
	int	s;
	
	/*
	 *	Check for silly no-op.
	 */
	pset = thread->processor_set;
	if (pset == new_pset) {
		return;
	}
	/*
	 *	Suspend the thread and stop it if it's not the current thread.
	 */
	thread_hold(thread);
	if (thread != current_thread())
		(void) thread_dowait(thread, TRUE);

	/*
	 *	Lock both psets now, use ordering to avoid deadlocks.
	 */
	if ((int)pset < (int)new_pset) {
	    pset_lock(pset);
	    pset_lock(new_pset);
	}
	else {
	    pset_lock(new_pset);
	    pset_lock(pset);
	}

	/*
	 *	Grab the thread lock and move the thread.
	 *	Then drop the lock on the old pset and the thread's
	 *	reference to it.
	 */
	s = splsched();
	thread_lock(thread);

	pset_remove_thread(pset, thread);
	pset_add_thread(new_pset, thread);

	old_empty = pset->empty;
	new_empty = new_pset->empty;

	pset_unlock(pset);
	pset_deallocate(pset);

	/*
	 *	Reset policy and priorities if needed.
	 */
	if (thread->policy & new_pset->policies == 0) {
	    thread->policy = POLICY_TIMESHARE;
	    recompute_pri = TRUE;
	}

	if (thread->max_priority < new_pset->max_priority) {
	    thread->max_priority = new_pset->max_priority;
	    if (thread->priority < thread->max_priority) {
		thread->priority = thread->max_priority;
		recompute_pri = TRUE;
	    }
	    else {
		if ((thread->depress_priority >= 0) &&
		    (thread->depress_priority < thread->max_priority)) {
			thread->depress_priority = thread->max_priority;
		}
	    }
	}

	pset_unlock(new_pset);

	if (recompute_pri)
		compute_priority(thread, TRUE);

	if (release_freeze) {
		thread->may_assign = TRUE;
		if (thread->assign_active) {
			thread->assign_active = FALSE;
			thread_wakeup((vm_offset_t)&thread->assign_active);
		}
	}

	thread_unlock(thread);
	splx(s);

	/*
	 *	Figure out hold status of thread.  Threads assigned to empty
	 *	psets must be held.  Therefore:
	 *		If old pset was empty release its hold.
	 *		Release our hold from above unless new pset is empty.
	 */

	if (old_empty)
		thread_release(thread);
	if (!new_empty)
		thread_release(thread);

	/*
	 *	If current_thread is assigned, context switch to force
	 *	assignment to happen.  This also causes hold to take
	 *	effect if the new pset is empty.
	 */
	if (thread == current_thread())
		thread_block();
}
#else	MACH_HOST
kern_return_t
thread_assign(thread, new_pset)
thread_t	thread;
processor_set_t	new_pset;
{
#ifdef	lint
	thread++; new_pset++;
#endif	lint
	return(KERN_FAILURE);
}
#endif	MACH_HOST

/*
 *	thread_assign_default:
 *
 *	Special version of thread_assign for assigning threads to default
 *	processor set.
 */
kern_return_t
thread_assign_default(thread)
thread_t	thread;
{
	return (thread_assign(thread, &default_pset));
}

/*
 *	thread_get_assignment
 *
 *	Return current assignment for this thread.
 */	    
kern_return_t thread_get_assignment(thread, pset)
thread_t	thread;
processor_set_t	*pset;
{
	*pset = thread->processor_set;
	return(KERN_SUCCESS);
}

/*
 *	thread_priority:
 *
 *	Set priority (and possibly max priority) for thread.
 */
kern_return_t
thread_priority(thread, priority, set_max, may_reschedule)
thread_t	thread;
int		priority;
boolean_t	set_max;
boolean_t	may_reschedule;
{
    int			s;
    kern_return_t	ret = KERN_SUCCESS;

    if ((thread == THREAD_NULL) || invalid_pri(priority))
	return(KERN_INVALID_ARGUMENT);

    s = splsched();
    thread_lock(thread);

    /*
     *	Check for violation of max priority
     */
    if (priority < thread->max_priority) {
	ret = KERN_FAILURE;
    }
    else {
	/*
	 *	Set priorities.  If a depression is in progress,
	 *	change the priority to restore.
	 */
	if (thread->depress_priority >= 0) {
	    thread->depress_priority = priority;
	}
	else {
	    thread->priority = priority;
	    compute_priority(thread, may_reschedule);
	}

	if (set_max)
	    thread->max_priority = priority;
    }
    RT_SCHED_HIST(RTS_priority, thread, thread->policy,
		  thread->priority, 0, 0)
    thread_unlock(thread);
    (void) splx(s);

    return(ret);
}

/*
 *	thread_max_priority:
 *
 *	Reset the max priority for a thread.
 */
kern_return_t
thread_max_priority(thread, pset, max_priority, may_reschedule)
thread_t	thread;
processor_set_t	pset;
int		max_priority;
boolean_t	may_reschedule;
{
    int			s;
    kern_return_t	ret = KERN_SUCCESS;

    if ((thread == THREAD_NULL) || (pset == PROCESSOR_SET_NULL) ||
    	invalid_pri(max_priority))
	    return(KERN_INVALID_ARGUMENT);

    s = splsched();
    thread_lock(thread);

#if	MACH_HOST
    /*
     *	Check for wrong processor set, or thread in transit.
     */
    if (!(thread->may_assign) || pset != thread->processor_set) {
	ret = KERN_FAILURE;
    }
    else {
#endif	MACH_HOST
	thread->max_priority = max_priority;

	/*
	 *	Reset priority if it violates new max priority
	 */
	if (max_priority > thread->priority) {
	    thread->priority = max_priority;

	    compute_priority(thread, may_reschedule);
	}
	else {
	    if (thread->depress_priority >= 0 &&
		max_priority > thread->depress_priority)
		    thread->depress_priority = max_priority;
	    }
#if	MACH_HOST
    }
#endif	MACH_HOST

    RT_SCHED_HIST(RTS_max_priority, thread, thread->policy,
		  max_priority, 0, 0);
    thread_unlock(thread);
    (void) splx(s);

    return(ret);
}

/*
 * RT_SCHED_SYNCH_QUANTUM
 *
 * If a thread is on a processor when its quantum gets adjusted, the
 * processor quantum should be adjusted as well.  For instance, moving
 * from RR to FIFO, the quantum should increase.  Moving from FIFO to RR,
 * the quantum should decrease.  Otherwise, the quantum isn't updated with
 * the policy, but only gets updated after one yield.
 *
 * This macro synchronizes the processor quantum with the new quantum.
 *
 * It is easy to handle the case where a thread is setting its own policy or
 * quantum.  It is not as straightforward to handle the MP case where a thread
 * is setting the policy/quantum of another running thread.  This is another
 * case where an interprocessor interrupt might be handy.  At a minimum, a
 * special thread ast is needed, to be recognized by the hardclock routine on
 * the thread's processor, so that the quantum gets updated at the next clock
 * tick.  Note that this is _not_ done now, and needs to be added for Silver.
 */
#if RT_SCHED
#if NCPUS <= 1
#define RT_SCHED_SYNCH_QUANTUM(thread,new_quantum)			\
        MACRO_BEGIN							\
        if ((thread) == current_thread()) {				\
	  RT_SCHED_HIST(RTS_qsynch, (thread), (thread)->policy,		\
			current_processor()->quantum, 			\
			(new_quantum),					\
			FALSE);						\
	  current_processor()->quantum = (new_quantum);			\
	}								\
        MACRO_END
#else /* NCPUS > 1 */
#define RT_SCHED_SYNCH_QUANTUM(thread,new_quantum)			\
        MACRO_BEGIN							\
        if ((thread) == current_thread()) {				\
	  RT_SCHED_HIST(RTS_qsynch, (thread), (thread)->policy,		\
			current_processor()->quantum, 			\
			(new_quantum),					\
			FALSE);						\
	  current_processor()->quantum = (new_quantum);			\
	}								\
        else if (((thread)->runq == RUN_QUEUE_NULL) &&			\
		 ((thread)->state & TH_RUN)) {				\
	  RT_SCHED_HIST(RTS_qsynch, (thread), (thread)->policy,		\
			0 /* processor is unknown */, 			\
			(new_quantum),					\
			TRUE);						\
	  /* punt */							\
	}								\
        MACRO_END
#endif /* NCPUS > 1 */
#else /* RT_SCHED */
#define RT_SCHED_SYNCH_QUANTUM(thread,new_quantum)
#endif /* RT_SCHED */

/*
 *	thread_policy:
 *
 *	Set scheduling policy for thread.
 */
kern_return_t
thread_policy(thread, policy, data)
thread_t	thread;
int		policy;
int		data;
{
	register kern_return_t	ret = KERN_SUCCESS;
	register int s, temp;

	if ((thread == THREAD_NULL) || invalid_policy(policy))
		return(KERN_INVALID_ARGUMENT);

	s = splsched();
	thread_lock(thread);

	/*
	 *	Check if changing policy.
	 */
	if (policy == thread->policy) {
	    /*
	     *	Just changing data.  This is meaningless for
	     *	timeshareing, quantum for fixed priority (but
	     *	has no effect until current quantum runs out).
	     */
#if RT_SCHED
	    if (policy & (POLICY_FIXEDPRI|POLICY_RR)) {
#else /* RT_SCHED */
	    if (policy & POLICY_FIXEDPRI) {
#endif /* RT_SCHED */
                temp = data * 1000;
                if (temp % tick)
                        temp += tick;
                thread->sched_data = temp/tick;
		RT_SCHED_SYNCH_QUANTUM(thread, thread->sched_data);
	    }
	}
	else {
	    /*
	     *	Changing policy.  Check if new policy is allowed.
	     */
	    if ((thread->processor_set->policies & policy) == 0) {
		    ret = KERN_FAILURE;
	    }
	    else {
		/*
		 *	Changing policy.  Save data and calculate new
		 *	priority.
		 */
		thread->policy = policy;
#if RT_SCHED
		RT_SCHED_HIST(RTS_policy, thread, (policy|thread->policy),
			      policy, thread->priority, 0 );
		thread->rmng_quantum = 0;

		/*
		 * It conforms to SVR4 priocntl() behavior
		 * to set the quantum to 0 in the thread and in the
		 * processor the thread's running on when the thread's
		 * policy is changing to TIMESHARE from some other 
		 * policy.
		 */
		if (policy & POLICY_TIMESHARE) {
		    /*
		     * The quantum calculation in clock_tick() is complicated.
		     * Rather than duplicating it, just cause the quantum to
		     * expire so that the correct quantum gets calculated at
		     * the next clock tick.  This has the side effect of
		     * pushing the thread to the tail of its run queue, but
		     * that doesn't much matter when returning to timesharing
		     * from a real time policy.
		     */
		    RT_SCHED_SYNCH_QUANTUM(thread, 0);
		}
		else if (policy & POLICY_FIFO) {
		    thread->sched_data = INT_MAX;
		    RT_SCHED_SYNCH_QUANTUM(thread, INT_MAX);
		}
		else if (policy & (POLICY_FIXEDPRI|POLICY_RR)) {
#else /* RT_SCHED */
		if (policy & POLICY_FIXEDPRI) {
#endif /* RT_SCHED */
                        temp = data * 1000;
                        if (temp % tick)
                                temp += tick;
                        thread->sched_data = temp/tick;
			RT_SCHED_SYNCH_QUANTUM(thread, thread->sched_data);
		}
		compute_priority(thread, TRUE);
	    }
	}
	thread_unlock(thread);
	(void) splx(s);

	return(ret);
}

/*
 * Name: kd_thread_init
 *
 * Description:
 *      Initialized the kernel driver thread framework.
 */
void
kd_thread_init()
{


    KD_LOCK_THREAD_LIST_INIT();
    queue_init((queue_t)&kd_thread_list.kd_start_threads);

    /*
     * List state goes to initialized
     */
    KD_LOCK_THREAD_LIST();
    kd_flags |= KD_INITIALIZED;
    KD_UNLOCK_THREAD_LIST();


}

/*
 * Name: kd_thread_register
 *
 * Description:
 *      Register a routine to be called when the kernel thread environment
 *      is initizlized.
 */

int
kd_thread_register(int (*func)(), caddr_t arg)
{
    KD_THREAD_ELT* dt_elt;

    /*
     * Don't lock the list just incase the lock has not
     * been init'ed. Look at the initialized flag and if
     * not set the return a failure
     */
    if(( kd_flags & KD_INITIALIZED) == 0 ){
        /*
         * Too early return a failure
         */
        return(-1);
    }

    /*
     * Get memory for the thread list struct
     */
    if ((dt_elt = (KD_THREAD_ELT *) kalloc(sizeof (KD_THREAD_ELT))) == NULL)
    {
        panic("kd_thread_register: not enough memory");
    }

    dt_elt->dt_func = func;
    dt_elt->dt_arg = arg;
    /*
     * Lock the list before touching anything
     */
    KD_LOCK_THREAD_LIST();
    enqueue_tail(&kd_thread_list, (queue_entry_t)dt_elt);

    /*
     * If init_main as all ready has gone thru then call
     * directly to get this thread up and running
     */
    if((kd_flags & KD_STARTED) != 0){
        KD_UNLOCK_THREAD_LIST();
        kd_thread_start();
    }
    else {
        KD_UNLOCK_THREAD_LIST();
    }

    return(0);
}

/*
 * Name: kd_thread_start
 *
 * Description:
 *      This routine will call the registered driver routines.
 *      The entry will be destroyed after the driver routines called to
 *      free up the memory.
 */
void
kd_thread_start()
{
    KD_THREAD_ELT *dt_elt;

    while ( 1 ) {
        KD_LOCK_THREAD_LIST();
        dt_elt = (KD_THREAD_ELT *)dequeue_head(&kd_thread_list);
        KD_UNLOCK_THREAD_LIST();
        if(dt_elt == NULL){
                break;
        }
        (dt_elt->dt_func)(dt_elt->dt_arg);
        kfree(dt_elt, sizeof (KD_THREAD_ELT));
    }
    /*
     * Signify that init_main() call is done
     */
    KD_LOCK_THREAD_LIST();
    kd_flags |= KD_STARTED;
    KD_UNLOCK_THREAD_LIST();

}
