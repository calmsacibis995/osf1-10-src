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
 *	@(#)$RCSfile: select.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/19 20:42:50 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1988 Encore Computer Corporation
 * All rights reserved.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_SYS_SELECT_H_
#define	_SYS_SELECT_H_

#ifdef	_KERNEL
#include <kern/event.h>
#include <kern/queue.h>

/*
 * Problems with select in a parallelized Unix environment.
 *
 * The existing BSD select mechanism, a set of variable contained in
 * each selectable object and a set of global variables to handle
 * "select collisions" does not work well in a parallelized Unix kernel.
 * First, there is the obvious race between one thread attempting to
 * set itself up to be notified of a change in the state of a selectable
 * object and another thread attempting to signal that a selectable
 * object's state has just changed.
 *
 * Second, and more insidious, is the problem of the task's file descriptor
 * array changing during the select due to the action of another thread
 * within the task.  For example, at the time thread A initiates a select()
 * operation, file descriptor X could be open and specified as part of one
 * of the select masks.  Thread A might then sleep, waiting for one of the
 * select'ed objects to change state.  In the meantime, thread B might close
 * file descriptor X.  According to the original code, thread A would not be
 * awakened as the result of thread B's close operation.  Possibly thread A
 * might never awaken, if it was only waiting on the object closed out by B.
 * Should thread A finally awoken, thread A will return with a select error
 * because it now has an invalid file descriptor in one of its select masks.
 *
 * The solution Encore developed for select uses a queue of select events in
 * each selectable object and a select event in the uthread structure.
 * Threads add themselves to a selectable object's select queue and then sleep
 * on the uthread's select event.  When a state change takes place, any
 * threads linked on the object's select queue are awoken.  At the end of a
 * select operation, unselect routines are invoked for each object to clean
 * up any remaining entries on the various select queues.
 *
 * Each object close routine changes to wake up any threads queued on the
 * object's select queue.  These threads will then re-poll all of the
 * objects that interest them; possibly some will then report invalid
 * file descriptors as the result of the original close.
 *
 * Changes:
 *	fo_select	now takes an additional "scanning" argument
 *	xxx_select	now takes an additional "scanning" argument
 *			and calls select_{enqueue,dequeue} as appropriate
 *	xxx_close	now must do a "select_wakeup"
 *	xxx_whatever	changes to do a "select_wakeup"
 * When "scanning" is non-zero, select() is performing the normal select
 * operation and the lower-level routine must return non-zero if the
 * object is selectable.  When "scanning" is zero, select() is cleaning
 * out old select queue entries and the lower-level routine must return zero.
 */

struct sel_queue {
	struct queue_entry	links;
	struct event		*event;
};

typedef struct sel_queue	sel_queue_t;

void	select_init();		/* Initialize select subsystem */
void	select_enqueue();	/* Put current thread on a select queue */
void	select_dequeue();	/* Remove current thread from a sel queue */
void	select_dequeue_all();	/* Remove all threads from a sel queue */
void	select_wakeup();	/* Notify all threads parked on a sel queue */
void	select_cleanup();	/* Purge accumulated sel events */

#endif  /* _KERNEL */
#endif	/* _SYS_SELECT_H_ */
