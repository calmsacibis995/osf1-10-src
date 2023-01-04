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
 *	@(#)$RCSfile: threadcall.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:04 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Header file for general-purpose thread callout routines.
 * (see threadcall.c)
 */
#ifndef _KERN_THREADCALL_H_
#define _KERN_THREADCALL_H_

#include <kern/lock.h>
#include <kern/zalloc.h>

struct thread_call {
	struct thread_call *tc_next;
	void	(*tc_func)();
	void	*tc_arg;
};
typedef struct thread_call thread_call_t;

struct thread_callq {
	decl_simple_lock_data(,tcq_lock)
	thread_call_t *tcq_head;
	thread_call_t *tcq_tail;
	zone_t	tcq_zone;
	int	tcq_zone_size;
	int	tcq_threadcall_size;
};
typedef struct thread_callq thread_callq_t;

extern boolean_t thread_call();
extern boolean_t thread_call_one();
extern void thread_call_create();
extern void thread_call_add();
extern void thread_call_alloc();

#endif	/* _KERN_THREADCALL_H_ */
