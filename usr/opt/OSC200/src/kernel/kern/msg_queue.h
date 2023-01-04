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
 *	@(#)$RCSfile: msg_queue.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:26:23 $
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
 *	kern/msg_queue.h
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_MSG_QUEUE_H_
#define _KERN_MSG_QUEUE_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_MSG_QUEUE_H_PREEMPT_
#endif
#endif

#include <kern/queue.h>
#include <kern/lock.h>
#include <kern/macro_help.h>

typedef struct {
	queue_head_t messages;
	decl_simple_lock_data(,lock)
	queue_head_t blocked_threads;
} msg_queue_t;

#define msg_queue_lock(mq)	simple_lock(&(mq)->lock)
#define msg_queue_unlock(mq)	simple_unlock(&(mq)->lock)

#define msg_queue_init(mq)			\
MACRO_BEGIN					\
	simple_lock_init(&(mq)->lock);		\
	queue_init(&(mq)->messages);		\
	queue_init(&(mq)->blocked_threads);	\
MACRO_END

#if	RT_PREEMPT
#ifdef	_KERN_MSG_QUEUE_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_MSG_QUEUE_H_ */
