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
 *	@(#)$RCSfile: kern_obj.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:29 $
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
 * Common fields for dynamically managed kernel objects
 * for which tasks have capabilities.
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_KERN_OBJ_H_
#define _KERN_KERN_OBJ_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_KERN_OBJ_H_PREEMPT_
#endif
#endif

#include <mach/boolean.h>
#include <kern/lock.h>
#include <kern/zalloc.h>
#include <kern/queue.h>
#include <kern/assert.h>
#include <kern/macro_help.h>

typedef struct kern_obj {
	decl_simple_lock_data(,obj_data_lock)
	boolean_t obj_in_use;
	int obj_references;
	zone_t obj_home_zone;
	queue_head_t obj_translations;
} *kern_obj_t;

#define KERN_OBJ_NULL		((kern_obj_t) 0)
#define KERN_OBJ_INVALID	((kern_obj_t) -1)

#define obj_lock(obj) 						\
MACRO_BEGIN							\
	simple_lock(&(obj)->obj_data_lock);			\
	assert((obj)->obj_references > 0);			\
MACRO_END

#define obj_lock_try(obj)	simple_lock_try(&(obj)->obj_data_lock)

#define obj_unlock(obj) 					\
MACRO_BEGIN							\
	assert((obj)->obj_references > 0);			\
	simple_unlock(&(obj)->obj_data_lock);			\
MACRO_END

#define obj_check_unlock(obj) 					\
MACRO_BEGIN							\
	if ((obj)->obj_references <= 0)				\
		obj_free(obj);					\
	else							\
		simple_unlock(&(obj)->obj_data_lock);		\
MACRO_END

#define obj_free(obj) 	 					\
MACRO_BEGIN							\
	assert(!(obj)->obj_in_use);				\
	assert((obj)->obj_references == 0);			\
	assert(queue_empty(&(obj)->obj_translations));		\
	simple_unlock(&(obj)->obj_data_lock);			\
	ZFREE((obj)->obj_home_zone, (vm_offset_t) (obj)); 	\
MACRO_END

#define obj_reference(obj) 					\
MACRO_BEGIN							\
	obj_lock(obj);						\
	(obj)->obj_references++; 				\
	obj_unlock(obj);					\
MACRO_END

#define obj_release(obj) 					\
MACRO_BEGIN							\
	obj_lock(obj); 						\
	(obj)->obj_references--;				\
	obj_check_unlock(obj);					\
MACRO_END

#if	RT_PREEMPT
#ifdef	_KERN_KERN_OBJ_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_KERN_OBJ_H_ */
