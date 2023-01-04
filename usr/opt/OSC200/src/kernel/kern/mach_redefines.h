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
 *	@(#)$RCSfile: mach_redefines.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:54 $
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

#ifndef	_KERN_MACH_REDEFINES_H_
#define _KERN_MACH_REDEFINES_H_

#define task_create	task_create_not_implemented
/*ARGSUSED*/
kern_return_t task_create(parent_task, inherit_memory, child_task)
	task_t		parent_task;
	boolean_t	inherit_memory;
	task_t		*child_task;
{
	uprintf("task_create is not implemented yet\n");
	return(KERN_FAILURE);
}

#define task_terminate	task_terminate_not_implemented
/*ARGSUSED*/
kern_return_t task_terminate(task)
	task_t		task;
{
	uprintf("task_terminate is not implemented yet\n");
	return(KERN_FAILURE);
}

#include <mach_xp.h>

#if	!MACH_XP
#include <mach/memory_object.h>

#define vm_allocate_with_pager	vm_allocate_with_pager_not_implemented
/*ARGSUSED*/
kern_return_t vm_allocate_with_pager(map, addr, size, find_space, pager,
		pager_offset)
	vm_map_t		map;
	vm_offset_t		*addr;
	vm_size_t		size;
	boolean_t		find_space;
	memory_object_t		pager;
	vm_offset_t		pager_offset;
{
	uprintf("vm_allocate_with_pager is not implemented in this kernel\n");
	return(KERN_FAILURE);
}
#endif	/* !MACH_XP */

#if	MACH_XP
#include <vm/vm_object.h>
#include <mach/memory_object.h>
#include <mach/boolean.h>

#define pager_cache	xxx_pager_cache
kern_return_t pager_cache(object, should_cache)
	vm_object_t	object;
	boolean_t	should_cache;
{
	if (object == VM_OBJECT_NULL)
		return(KERN_INVALID_ARGUMENT);

	return(memory_object_set_attributes(object, TRUE, should_cache, MEMORY_OBJECT_COPY_NONE));
}
#endif	/* MACH_XP */

#endif	/* _KERN_MACH_REDEFINES_H_ */
