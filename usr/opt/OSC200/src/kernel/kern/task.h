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
 *	@(#)$RCSfile: task.h,v $ $Revision: 4.3.11.2 $ (DEC) $Date: 1993/09/22 18:28:31 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	task.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for tasks.
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_TASK_H_
#define _KERN_TASK_H_

#include <mach_emulation.h>
#include <mach_ipc_xxxhack.h>
#include <mach_ipc_tcache.h>
#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_TASK_H_PREEMPT_
#endif
#endif

#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/time_value.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/mach_param.h>
#include <kern/kern_obj.h>
#include <kern/kern_set.h>
#include <kern/processor.h>
#include <kern/syscall_emulation.h>
#include <kern/processor.h>
#include <vm/vm_map.h>
#include <procfs/procfs.h>
#include <mach/task_info.h>

typedef port_t			task_port_t;

struct task {
	/* Synchronization/destruction information */
	decl_simple_lock_data(,lock)	/* Task's lock */
	int		ref_count;	/* Number of references to me */
	boolean_t	active;		/* Task has not been terminated */

	/* Miscellaneous */
	vm_map_t	map;		/* Address space description */
	queue_chain_t	pset_tasks;	/* list of tasks assigned to pset */
	int		suspend_count;	/* Internal scheduling only */

	/* Thread information */
	queue_head_t	thread_list;	/* list of threads */
	int		thread_count;	/* number of threads */
	decl_simple_lock_data(,thread_list_lock) /* XXX thread_list lock */
	processor_set_t	processor_set;	/* processor set for new threads */
	boolean_t	may_assign;	/* can assigned pset be changed? */
	boolean_t	assign_active;	/* waiting for may_assign */

	/* Garbage */
	struct utask	*u_address;
	int		proc_index;	/* corresponding process, by index */

	/* User-visible scheduling information */
	int		user_stop_count;	/* outstanding stops */
	int		priority;		/* for new threads */

	/* Information for kernel-internal tasks */
	boolean_t	kernel_ipc_space; /* Uses kernel's port names? */
	boolean_t	kernel_vm_space; /* Uses kernel's pmap? */

	/* Statistics */
	time_value_t	total_user_time;
				/* total user time for dead threads */
	time_value_t	total_system_time;
				/* total system time for dead threads */

	/* Special ports */
	task_port_t	task_self;	/* Port representing the task */
	task_port_t	task_tself;	/* What the task thinks is task_self */
	task_port_t	task_notify;	/* Where notifications get sent */
	task_port_t	exception_port;	/* Where exceptions are sent */
	task_port_t	bootstrap_port;	/* Port passed on for task startup */

	/* IPC structures */
	boolean_t	ipc_privilege;	/* Can use kernel resource pools? */
	decl_simple_lock_data(,ipc_translation_lock)
	queue_head_t	ipc_translations; /* Per-task port naming */
	boolean_t	ipc_active;	/* Can IPC rights be added? */
	port_name_t	ipc_next_name;	/* Next local name to use */
#if	MACH_IPC_XXXHACK
	kern_set_t	ipc_enabled;	/* Port set for PORT_ENABLED */
#endif	/* MACH_IPC_XXXHACK */

#if	MACH_IPC_TCACHE
#define OBJ_CACHE_MAX		010	/* Number of cache lines */
#define OBJ_CACHE_MASK		007	/* Mask for name->line */

	struct {
		port_name_t	name;
		kern_obj_t	object;
	}		obj_cache[OBJ_CACHE_MAX];
					/* Fast object translation cache */
#endif	/* MACH_IPC_TCACHE */

	/* IPC compatibility garbage */
	boolean_t	ipc_intr_msg;	/* Send signal upon message arrival? */
	task_port_t		ipc_ports_registered[TASK_PORT_REGISTER_MAX];

#if	MACH_EMULATION
	/* User space system call emulation support */
	struct 	eml_dispatch	*eml_dispatch;
#endif	/* MACH_EMULATION */
	queue_chain_t	task_link;	/* Pointer to next task on swap queue */
	int		swap_state;	/* current swap state of task */
	int		swap_request;	/* current swap request of task */
	unsigned	inswap_stamp;	/* time of last inswap */
	unsigned	outswap_stamp;	/* time of last outswap */
	int		working_set;	/* number of pages reclaimed by swapper */
	int 		swap_nswap;	/* number of times this task swapped */
	/* task event statistics (including VM faults) */
	events_info_data_t task_events; /* statistics for dead threads */

	struct procfs procfs;		/* /proc task-specific data */
};
/* task swap_state values */
#define TASK_INSWAPPED		0x01
#define TASK_OUTSWAPPED		0x02
#define	TASK_COMMING_IN		0x03
#define TASK_GOING_OUT		0x04
/* task swap_request values */ 
#define TASK_ALL_SET		0x01
#define TASK_WANTS_IN		0x02

typedef struct task *task_t;

#define TASK_NULL	((task_t) 0)

typedef	port_t	*task_array_t;

#define task_lock(task)		simple_lock(&(task)->lock)
#define task_unlock(task)	simple_unlock(&(task)->lock)

#define ipc_task_lock(t)	simple_lock(&(t)->ipc_translation_lock)
#define ipc_task_unlock(t)	simple_unlock(&(t)->ipc_translation_lock)

/*
 *	Exported routines/macros
 */

extern kern_return_t	task_create();
extern kern_return_t	task_terminate();
extern kern_return_t	task_suspend();
extern kern_return_t	task_resume();
extern kern_return_t	task_threads();
extern kern_return_t	task_ports();
extern kern_return_t	task_info();
extern kern_return_t	task_get_special_port();
extern kern_return_t	task_set_special_port();
extern kern_return_t	task_assign();
extern kern_return_t	task_assign_default();

/*
 *	Internal only routines
 */

extern void		task_init();
extern void		task_reference();
extern void		task_deallocate();
extern kern_return_t	task_hold();
extern kern_return_t	task_dowait();
extern kern_return_t	task_release();
extern kern_return_t	task_halt();
extern void             events_info_add();
extern kern_return_t	task_suspend_nowait();

extern task_t	kernel_task;

#if	RT_PREEMPT
#ifdef	_TASK_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_TASK_H_ */
