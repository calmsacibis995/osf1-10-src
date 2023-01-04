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
 *	@(#)$RCSfile: kern_set.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:35 $
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
 * Kernel internal structure associated with a port set.
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_KERN_SET_H_
#define _KERN_KERN_SET_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_KERN_SET_H_PREEMPT_
#endif
#endif

#include <mach/port.h>
#include <mach/kern_return.h>
#include <kern/kern_obj.h>
#include <kern/msg_queue.h>
#include <kern/queue.h>

typedef struct kern_set {
	struct kern_obj set_obj;

	struct task *set_owner;	/* not task_t, to avoid recursion */
	port_name_t set_local_name;

	msg_queue_t set_messages;
	queue_head_t set_members;
	struct kern_port *set_traversal; /* don't ask */
} *kern_set_t;

#define set_data_lock		set_obj.obj_data_lock
#define set_in_use		set_obj.obj_in_use
#define set_references		set_obj.obj_references
#define set_home_zone		set_obj.obj_home_zone
#define set_translations	set_obj.obj_translations

#define KERN_SET_NULL	((kern_set_t) 0)

#define set_lock(set)		obj_lock(&(set)->set_obj)
#define set_lock_try(set)	obj_lock_try(&(set)->set_obj)
#define set_unlock(set)		obj_unlock(&(set)->set_obj)
#define set_check_unlock(set)	obj_check_unlock(&(set)->set_obj)
#define set_free(set)		obj_free(&(set)->set_obj)

#define set_reference_macro(set)	obj_reference(&(set)->set_obj)
#define set_release_macro(set)		obj_release(&(set)->set_obj)

extern void set_reference();
extern void set_release();
extern kern_return_t set_alloc();
extern void set_destroy();
extern void set_add_member();
extern void set_remove_member();

#if	RT_PREEMPT
#ifdef	_KERN_KERN_SET_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_KERN_SET_H_ */
