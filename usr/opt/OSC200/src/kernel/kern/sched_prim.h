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
 *	@(#)$RCSfile: sched_prim.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/04/14 16:32:05 $
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
 *	File:	sched_prim.h
 *	Author:	David Golub
 *
 *	Scheduling primitive definitions file
 *
 */

#ifndef	_KERN_SCHED_PRIM_H_
#define _KERN_SCHED_PRIM_H_

/*
 *	Possible results of assert_wait - returned in
 *	current_thread()->wait_result.
 */
#define THREAD_AWAKENED		0		/* normal wakeup */
#define THREAD_TIMED_OUT	1		/* timeout expired */
#define THREAD_INTERRUPTED	2		/* interrupted by clear_wait */
#define THREAD_SHOULD_TERMINATE	3		/* thread should terminate */
#define THREAD_RESTART		4		/* restart operation entirely */

/*
 *	Exported interface to sched_prim.c 
 */

extern void	sched_init();
extern void	assert_wait();
extern void	clear_wait();
extern void	thread_sleep();
extern void	thread_wakeup();		/* for function pointers */
extern void	thread_wakeup_prim();
extern void	thread_block();
extern void	thread_run();
extern void	thread_set_timeout();
extern void	thread_setrun();

/*
 *	Routines defined as macros
 */

#define thread_wakeup(x)						\
		thread_wakeup_prim((x), FALSE, THREAD_AWAKENED)
#define thread_wakeup_with_result(x, z)					\
		thread_wakeup_prim((x), FALSE, (z))
#define thread_wakeup_one(x)						\
		thread_wakeup_prim((x), TRUE, THREAD_AWAKENED)


#define thread_wakeup_high(x)                                          \
                thread_wakeup_hi((x), THREAD_AWAKENED)

#endif	/* _KERN_SCHED_PRIM_H_ */




