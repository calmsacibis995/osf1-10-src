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
 *	@(#)$RCSfile: sched_prim_macros.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:27:08 $
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
 *	File:	sched_prim_macros.h
 *	Author:	David Golub
 *
 *	Scheduling primitive macros file
 *
 */

#ifndef	_KERN_SCHED_PRIM_MACROS_H_
#define _KERN_SCHED_PRIM_MACROS_H_

#include <sys/types.h>
#include <mach/boolean.h>
#include <kern/processor.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <kern/thread_swap.h>
#include <kern/macro_help.h>
#include <machine/machparam.h>	/* for splsched */

extern int	thread_timeout();

/*
 *	Make a thread wait interruptibly.
 */
#define thread_will_wait(thread) 			\
MACRO_BEGIN						\
	register int s;					\
	s = splsched();					\
	thread_lock(thread);				\
	(thread)->state |= TH_WAIT;			\
	(thread)->sleep_stamp = sched_tick;		\
	thread_unlock(thread);				\
	splx(s);					\
MACRO_END

/*
 *	Make a thread wait interruptibly, with a
 *	timeout.
 */
#define thread_will_wait_with_timeout(thread, t)		\
MACRO_BEGIN							\
	register int s;						\
	s = splsched();						\
	thread_lock(thread);					\
	(thread)->state |= TH_WAIT;				\
	(thread)->sleep_stamp = sched_tick;			\
	(thread)->timer_set = TRUE;				\
	timeout(thread_timeout, (caddr_t)(thread), (int)(t));	\
	thread_unlock(thread);					\
	splx(s);						\
MACRO_END

/*
 *	Wakeup a thread, and queue it to run normally.
 */
#define thread_go(thread)  					\
MACRO_BEGIN							\
	register int s, state; 					\
	s = splsched();						\
	thread_lock(thread); 					\
	if ((thread)->timer_set) {				\
		(thread)->timer_set = FALSE;			\
		untimeout(thread_timeout, (caddr_t) (thread));	\
	}							\
	state = (thread)->state;				\
	switch (state) {					\
	    case TH_WAIT | TH_SUSP:				\
		/*						\
		 *	Suspend thread if interruptible		\
		 */						\
		if ((thread)->interruptible) {			\
		    (thread)->state = TH_SUSP;			\
		    (thread)->wait_result = THREAD_AWAKENED;	\
		    break;					\
		}						\
		/* fall through */				\
	    case TH_WAIT:					\
		/*						\
		 *	Sleeping and not suspendable - put	\
		 *	on run queue.				\
		 */						\
		(thread)->state = (state & ~TH_WAIT) | TH_RUN;	\
		(thread)->wait_result = THREAD_AWAKENED;	\
		thread_setrun(thread, TRUE);				\
		break;						\
								\
	    case TH_WAIT | TH_SWAPPED:				\
		/*						\
		 *	Thread is swapped out, but runnable	\
		 */						\
		(thread)->state = TH_RUN | TH_SWAPPED;		\
		(thread)->wait_result = THREAD_AWAKENED;	\
		thread_swapin(thread);				\
		break;						\
								\
	    default:						\
		/*						\
		 *	Either already running, or suspended.	\
		 */						\
		if (state & TH_WAIT) {				\
		    (thread)->state = state & ~TH_WAIT;		\
		    (thread)->wait_result = THREAD_AWAKENED;	\
		    break;					\
		}						\
	}							\
	thread_unlock(thread); 					\
	splx(s); 						\
MACRO_END


/*
 *	Wakeup a thread.  Block, transferring control to
 *	it immediately, if it can run (and will run on the
 *	same CPU).
 */
#define thread_go_and_switch(thread) 				\
MACRO_BEGIN							\
	register int s, state; 					\
	s = splsched();						\
	thread_lock(thread); 					\
	if ((thread)->timer_set) {				\
		(thread)->timer_set = FALSE;			\
		untimeout(thread_timeout, (caddr_t) (thread));	\
	}							\
	state = (thread)->state;				\
	switch (state) {					\
	    case TH_WAIT | TH_SUSP:				\
		/*						\
		 *	Suspend thread if interruptible		\
		 */						\
		if ((thread)->interruptible) {			\
		    (thread)->state = TH_SUSP;			\
		    (thread)->wait_result = THREAD_AWAKENED;	\
		    break;					\
		}						\
		/* fall through */				\
	    case TH_WAIT:					\
		/*						\
		 *	Sleeping and not suspendable - put	\
		 *	on run queue.				\
		 */						\
		(thread)->state = (state & ~TH_WAIT) | TH_RUN;	\
		(thread)->wait_result = THREAD_AWAKENED;	\
		if (((thread)->processor_set->idle_count > 0) ||\
		    ((thread)->processor_set !=			\
		     current_thread()->processor_set)) {	\
			/*					\
			 *	Other cpus can/must run thread.	\
			 *	Put it on the run queues.	\
			 */					\
			thread_setrun(thread, TRUE);		\
			break;					\
		}						\
		else {						\
		    /*						\
		     *	Switch immediately to new thread.	\
		     */						\
		    thread_unlock(thread);			\
		    thread_run(thread);				\
		    goto done;					\
		}						\
								\
	    case TH_WAIT | TH_SWAPPED:				\
		/*						\
		 *	Thread is swapped out, but runnable	\
		 */						\
		(thread)->state = TH_RUN | TH_SWAPPED;		\
		(thread)->wait_result = THREAD_AWAKENED;	\
		thread_swapin(thread);				\
		break;						\
								\
	    default:						\
		/*						\
		 *	Either already running, or suspended.	\
		 */						\
		if (state & TH_WAIT) {				\
		    (thread)->state = state & ~TH_WAIT;		\
		    (thread)->wait_result = THREAD_AWAKENED;	\
		    break;					\
		}						\
	}							\
	thread_unlock(thread); 					\
    done:							\
	splx(s); 						\
MACRO_END

#endif	/* _KERN_SCHED_PRIM_MACROS_H_ */
