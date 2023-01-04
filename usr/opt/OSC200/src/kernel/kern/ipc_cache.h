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
 *	@(#)$RCSfile: ipc_cache.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:23:39 $
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

#ifndef	_KERN_IPC_CACHE_H_
#define _KERN_IPC_CACHE_H_

/*
 * The per-task Translation Cache
 *
 * We cache mappings between a task's local name for a port and the
 * internal pointer to that port.  Actually, we can cache mappings for
 * general objects, but currently the cache has two lines which
 * are used only for caching ports encounted in message headers.
 *
 * Each cache line has two fields, a local_name (port_name_t) and
 * a global name (kern_obj_t).  Values of PORT_NULL/KERN_OBJ_NULL
 * indicate an invalid cache line.  Both values in a valid cache
 * line are non-null.
 *
 * [There is no advantage to making the PORT_NULL/KERN_OBJ_NULL mapping
 * a valid cache line, because the fast path code has to check for NULL
 * as a special case anyway, to know whether the object should
 * gain/lose a reference.]
 *
 * The cache is part of the task structure, and is accessed under the
 * IPC task lock.  The cache does not hold any references for objects.
 * If there is a valid mapping in the cache, then the task must have
 * a translation record for the object, which holds a reference.
 *
 * Any changes to the task's object/local-name mappings must flush
 * old mappings from the cache.  This can happen when the task loses
 * rights to an object or the task's local name for the object changes.
 *
 * If a valid mapping is found in the cache, then one may assume that
 * the task is ipc_active.  (And so the mapping may be used without
 * checking ipc_active.)  This is because mappings are only entered
 * into the cache when the task is active, and when the task is terminated
 * all cached mappings are immediately flushed.
 *
 * If a valid mapping is found in the cache, then one may assume that
 * the task is not a kernel_ipc_space task.  (And so the mapping may be
 * used without checking kernel_ipc_space.)  This is because mappings
 * are never entered into the cache of kernel_ipc_space tasks.
 *
 * The obj_cache macros which manipulate the cache generally require their
 * caller to hold the task's IPC task lock and no other locks.
 */

#include <mach_ipc_tcache.h>

#if	MACH_IPC_TCACHE

#include <mach/boolean.h>
#include <kern/kern_obj.h>
#include <kern/task.h>
#include <mach_debug/ipc_statistics.h>
#include <kern/macro_help.h>
#include <kern/ipc_copyin.h>

/*
 * extern unsigned int
 * obj_cache_index(name)
 *	port_name_t name;
 *
 * Given a port name, return index of the cache line
 * that might cache a translation for that name.
 */

#define obj_cache_index(name)	((unsigned int)((name) & OBJ_CACHE_MASK))


/*
 * extern void
 * obj_cache_set(task, name, object)
 *	task_t task;
 *	port_name_t name;
 *	kern_obj_t object;
 *
 * Stores a mapping in the specified cache line.
 * The task must be locked.
 */

#define obj_cache_set(task, _name, _object)				\
MACRO_BEGIN								\
	unsigned int index = obj_cache_index(_name);			\
									\
	(task)->obj_cache[index].name = (_name);			\
	(task)->obj_cache[index].object = (_object);			\
MACRO_END


/*
 * extern void
 * obj_cache_clear(task, index)
 *	task_t task;
 *	int index;
 *
 * Resets the specified cache line.
 * The task must be locked.
 */

#define obj_cache_clear(task, index)					\
MACRO_BEGIN								\
	(task)->obj_cache[index].name = PORT_NULL;			\
	(task)->obj_cache[index].object = KERN_OBJ_NULL;		\
MACRO_END


/*
 * extern void
 * obj_cache_flush(task, name)
 *	task_t task;
 *	port_name_t name;
 *
 * Flushes all mappings for the name from the task's cache.
 * The task must be locked.
 */

#define obj_cache_flush(task, _name) 					\
MACRO_BEGIN								\
	unsigned int index = obj_cache_index(_name);			\
									\
	if ((task)->obj_cache[index].name == (_name))			\
		obj_cache_clear((task), index);				\
MACRO_END


/*
 * extern void
 * obj_cache_copyin(task, name, object, barf)
 *	task_t task;
 *	port_name_t name;
 *	kern_obj_t &object;
 *	code barf;
 *
 * Copies in the local name, returning an object.
 * The caller gains a reference to the object.
 * If the copyin fails, executes the "barf" code,
 * which syntactically must be a complete statement
 * and which should return/goto.  The task must be locked.
 */

#define obj_cache_copyin(task, _name, _object, barf)			\
MACRO_BEGIN								\
	unsigned int index = obj_cache_index(_name);			\
									\
	if ((task)->obj_cache[index].name == (_name)) {			\
		(_object) = (task)->obj_cache[index].object;		\
		obj_reference(_object);					\
		ipc_event(port_copyin_hits);				\
	} else {							\
		if (object_copyin_cache((task), (_name), &(_object)))	\
			ipc_event(port_copyin_miss);			\
		else							\
			barf						\
	} 								\
MACRO_END


/*
 * Initializes the task's cache.
 * The task doesn't have to be locked if nobody can get at it.
 */
extern void obj_cache_init(/* task_t task */);


/*
 * extern void
 * obj_cache_terminate(task)
 *	task_t task;
 *
 * Finalizes the task's cache.
 * The task must be locked.
 */

#define obj_cache_terminate(task)	obj_cache_init(task);

#else	/* MACH_IPC_TCACHE */

#include <kern/ipc_copyin.h>

#define obj_cache_set(task, name, object)
#define obj_cache_clear(task, index)
#define obj_cache_flush(task, name)
#define obj_cache_init(task)
#define obj_cache_terminate(task)

#define obj_cache_copyin(task, name, object, barf)			\
MACRO_BEGIN								\
	if (object_copyin_cache((task), (name), &(object)))		\
		ipc_event(port_copyin_miss);				\
	else								\
		barf							\
MACRO_END

#endif	/* MACH_IPC_TCACHE */

#endif	/* _KERN_IPC_CACHE_H_ */
