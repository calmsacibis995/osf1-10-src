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
 *	@(#)$RCSfile: event.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:23:19 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	File:	sys/event.h
 *
 *	Event structure and definitions.
 *
 *	Events with "memory".
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_EVENT_H_
#define	_KERN_EVENT_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_EVENT_H_PREEMPT_
#endif
#endif

#include <mach/boolean.h>
#include <kern/lock.h>

struct event {
	boolean_t		ev_event;
#if	defined(_KERNEL) && defined(MACH_LDEBUG)
	char		*ev_thread;	/* thread posted/cleared event */
	int		ev_paddr;	/* pc of last event_post */
	int		ev_caddr;	/* pc of last event_clear */
#endif
	decl_simple_lock_data(,ev_slock)
};

typedef struct event		event_t;

void	event_clear();			/* event hasn't happened yet */
void	event_init();			/* init lock and event */
void	event_post();			/* tell world event happened */
int	event_posted();			/* has event happened yet? */
int	event_wait();			/* wait for momentous event */

#if	RT_PREEMPT
#ifdef	_KERN_EVENT_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_EVENT_H_ */
