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
 * @(#)$RCSfile: thread.h,v $ $Revision: 4.3.13.2 $ (DEC) $Date: 1993/09/22 18:28:44 $
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
 *	File:	thread.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for threads.
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 3-June-91	Ron Widyono
 *	Remove save_need_ast and lock_count fields for RT.
 *
 * 11-Apr-91     Paula Long
 *      Added P1003.4 required extensions for Timers to the
 *      thread structure.
 *
 * 9-Apr-91	Peter H. Smith
 *	Add rmng_quantum field to hold quantum of POLICY_RR thread which has
 *	been preempted to the head of the run queue.
 * 6-Apr-91	Ron Widyono
 *	Save_ast_needed and lock_count fields in struct thread for
 *	preemption points (RT_PREEMPT).
 *
 */

#ifndef	_KERN_THREAD_H_
#define _KERN_THREAD_H_

#include <hw_footprint.h>
#include <mach_host.h>
#include <mach_km.h>
#include <mach_ldebug.h>
#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_THREAD_H_PREEMPT_
#endif
#endif

#include <mach/policy.h>
#include <mach/port.h>
#include <mach/message.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <kern/ast.h>
#include <kern/queue.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/timer.h>
#include <kern/lock.h>
#include <kern/sched.h>
#include <kern/thread_modes.h>
#include <kern/kern_msg.h>
#include <kern/processor.h>
#include <kern/event.h>			/* unix compatibility -- select */
#include <machine/cpu.h>
#include <machine/thread.h>
#include <machine/pcb.h>
#include <vm/vm_object.h>
#include <procfs/procfs.h>
#include <mach/events_info.h>


#if	MACH_KM
#include <kern/kern_mon.h>
#endif

typedef port_t			thread_port_t;

struct thread {
	/* Run queues */
	queue_chain_t	links;		/* current run queue links */
	run_queue_t	runq;		/* run queue p is on SEE BELOW */
/*
 *	NOTE:	The runq field in the thread structure has an unusual
 *	locking protocol.  If its value is RUN_QUEUE_NULL, then it is
 *	locked by the thread_lock, but if its value is something else
 *	(i.e. a run_queue) then it is locked by that run_queue's lock.
 */

	/* Task information */
	task_t		task;		/* Task to which I belong */
	queue_chain_t	thread_list;	/* list of threads in task */

	/* Thread bookkeeping */
	queue_chain_t	pset_threads;	/* list of all threads in proc set*/


	/* Self-preservation */
	decl_simple_lock_data(,lock)
	int		ref_count;	/* number of references to me */

	/* Hardware state */
	struct pcb	*pcb;		/* hardware pcb & machine state */
	vm_offset_t	kernel_stack;	/* Where kernel stack was allocated */

	/* Blocking information */
	vm_offset_t	wait_event;	/* event we are waiting on */
	int		suspend_count;	/* internal use only */
	boolean_t	interruptible;	/* interruptible wait? */
	kern_return_t	wait_result;	/* outcome of wait */
	boolean_t	timer_set;	/* timeout set on wait */
	boolean_t	wake_active;
	int		swap_state;	/* swap state (or unswappable flag) */
	int		state;		/* Thread state: */
        char            *wait_mesg;     /* wait mesg
/*
 *	Thread states [bits or'ed]
 */
#define TH_WAIT			0x01	/* thread is queued for waiting */
#define TH_SUSP			0x02	/* thread has been asked to stop */
#define TH_RUN			0x04	/* thread is running or on runq */
#define TH_SWAPPED		0x08	/* thread is swapped out */
#define TH_IDLE			0x10	/* thread is an idle thread */

	/* Scheduling information */
	int		priority;	/* thread's priority */
	int		max_priority;	/* maximum priority */
	int		sched_pri;	/* scheduled (computed) priority */
	int		sched_data;	/* for use by policy */
	int		policy;		/* scheduling policy */
#if RT
	/* Added for RT_SCHED support.  Holds the remaining quantum for a
         * RR thread which was preempted.  Used to restore the remaining
	 * quantum when the thread is restored. */
	int		rmng_quantum;	/* For RR rescheduling */
#endif /* RT */
	int		depress_priority; /* depressed from this priority */
	unsigned int	cpu_usage;	/* exp. decaying cpu usage [%cpu] */
	unsigned int	sched_usage;	/* load-weighted cpu usage [sched] */
	unsigned int	sched_stamp;	/* last time priority was updated */
	unsigned int	sleep_stamp;	/* last time in TH_WAIT state */

	/* VM global variables */

	vm_offset_t	recover;	/* page fault recovery (copyin/out) */
	boolean_t	vm_privilege;	/* Can use reserved memory? */
	vm_offset_t	tmp_address;	/* Kernel virtual address for
					 * temporary mapping */
	vm_object_t	tmp_object;	/* Temporary vm_object for use in
					 * memory_object_data_provided */
	

	/* Compatibility garbage */
	struct u_address {
		struct uthread	*uthread;
		struct utask	*utask;
	} u_address;
	struct event	select_event;	/* select waits on this */
	int		unix_lock;	/* bind to unix_master */

	/* User-visible scheduling state */
	int		user_stop_count;	/* outstanding stops */

	/* IPC data structures */
	decl_simple_lock_data(,ipc_state_lock) /* Locks most IPC fields */
	thread_port_t	thread_self;	/* Port representing this thread */
	thread_port_t	thread_tself;	/* What task thinks is thread_self */
	thread_port_t	thread_reply;	/* Initial reply port for RPCs */
	queue_chain_t	ipc_wait_queue;	/* Chain for IPC sleep queues */
	msg_return_t	ipc_state;	/* Operation state after awakening */
	union {
	msg_size_t	msize;		/* Maximum size of msg to receive */
	kern_msg_t	kmsg;		/* Message that was received */
	} ipc_data;
	boolean_t	ipc_kernel;	/* ipc buffer in kernel space */
	port_name_t	reply_port;	/* See kern/mig_support.c. */

	/* Timing data structures */
	timer_data_t	user_timer;	/* user mode timer */
	timer_data_t	system_timer;	/* system mode timer */
	timer_save_data_t user_timer_save;  /* saved user timer value */
	timer_save_data_t system_timer_save;  /* saved sys timer val. */
	unsigned int	cpu_delta;	/* cpu usage since last update */
	unsigned int	sched_delta;	/* weighted cpu usage since update */

	/* Exception data structures */
	thread_port_t	exception_port;
	thread_port_t	exception_clear_port;

	/* Ast/Halt data structures */
	boolean_t	active;		/* how alive is the thread */
	boolean_t	halted;		/* halted at clean point ? */
	int		ast;    	/* ast's needed.  See ast.h */

	/* Processor data structures */
	processor_set_t	processor_set;	/* assigned processor set */
	processor_t	bound_processor;	/* bound to processor ?*/
#if	MACH_HOST
	boolean_t	may_assign;	/* may assignment change? */
	boolean_t	assign_active;	/* someone waiting for may_assign */
#endif

#if	HW_FOOTPRINT
	processor_t	last_processor; /* processor this last ran on */
#endif

#if	MACH_KM	
	/* Kernel Monitor data structures */
	monitor_t	monitor_obj;	/* Monitor looking at thread */
	int		monitor_id;	/* Thread's identity for monitor */
	queue_chain_t	monitored_threads; /* queue link for monitor */
#endif

#if	MACH_LDEBUG
	unsigned int	lock_count;	/* blocking locks held by thread */
	int	lock_addr[MAX_LOCK];
#endif
#if RT
	int	psx4_sleep;		/* psx 4 clocks and timers */
        int     time_remaining;  	/* psx 4 clocks and timers */
#endif /* RT */

        /* per-thread event statistics */
        events_info_data_t      thread_events;

	struct t_procfs t_procfs;	/* /proc thread-specific data */
};


typedef struct thread *thread_t;

#define THREAD_NULL	((thread_t) 0)

typedef	port_t	*thread_array_t;	/* XXX compensate for MiG */


extern thread_t active_threads[];	/* active threads */

/*
 *	User routines
 */

extern kern_return_t	thread_create();
extern kern_return_t	thread_terminate();
extern kern_return_t	thread_suspend();
extern kern_return_t	thread_resume();
extern kern_return_t	thread_abort();
extern kern_return_t	thread_get_state();
extern kern_return_t	thread_set_state();
extern kern_return_t	thread_get_special_port();
extern kern_return_t	thread_set_special_port();
extern kern_return_t	thread_info();
extern kern_return_t	thread_assign();
extern kern_return_t	thread_assign_default();

/*
 *	Kernel-only routines
 */

extern void		thread_init();
extern void		thread_reference();
extern void		thread_deallocate();
extern void		thread_hold();
extern kern_return_t	thread_dowait();
extern void		thread_release();
extern void		thread_swappable();
extern void		thread_force_terminate();
extern kern_return_t	thread_halt();
extern void		thread_halt_self();
extern thread_t		kernel_thread();
extern thread_t		kernel_thread_w_arg();

extern void		reaper_thread();

#if	MACH_HOST
extern void		thread_freeze();
extern void		thread_doassign();
extern void		thread_unfreeze();
#endif

/*
 *	Macro-defined routines
 */

#define thread_pcb(th)		((th)->pcb)

#define thread_lock(th)		simple_lock(&(th)->lock)
#define thread_unlock(th)	simple_unlock(&(th)->lock)

#define thread_should_halt(thread)	\
		((thread)->ast & (AST_HALT|AST_TERMINATE))

#define ipc_thread_lock(th)	simple_lock(&(th)->ipc_state_lock)
#define ipc_thread_unlock(th)	simple_unlock(&(th)->ipc_state_lock)

/*
 *	Machine specific implementations of the current thread macro
 *	designate this by defining CURRENT_THREAD.
 */
#ifndef	CURRENT_THREAD
#define current_thread()	(active_threads[cpu_number()])
#endif

#define current_task()		((current_thread())->task)

#if	RT_PREEMPT
#ifdef	_THREAD_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_THREAD_H_ */
