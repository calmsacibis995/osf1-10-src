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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: subr_select.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 23:49:48 $";
#endif 
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
 * Mach Operating System
 * Copyright (c) 1988 Encore Computer Corporation
 * All rights reserved.
 */

/*
 * subr_select.c:  a multi-threaded object select implementation.
 * Each object (e.g., socket or tty) includes a select queue, upon
 * which a thread may insert an entry pointing back to itself.  When
 * the object becomes active, its queue is traversed and all interested
 * threads are notified by posting the thread's select_event.
 *
 * It is not possible to use the existing thread wait event mechanism
 * as the select event must be cleared before any individual select
 * subroutine is invoked.  As an object-specific select subroutine may
 * need to take locks and therefore may have to wait on those locks,
 * the existing thread wait event mechanism would become overloaded.
 * Therefore a separate select event is required within the thread.
 *
 * Important Note:  the select queues themselves provide no mutual
 * exclusion.  The caller of these subroutines must use a lock to
 * guarantee mutual exclusion to the queue being manipulated.
 */

#include <mach_debug.h>		/* XXX */

#include <sys/unix_defs.h>
#include <sys/proc.h>
#include <sys/select.h>
#include <sys/user.h>
#include <kern/event.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <mach/vm_param.h>

zone_t			select_zone;
extern int		select_max_elements;
extern int		select_chunk_elements;

#define	SELECT_DEBUG	MACH_ASSERT
#if	SELECT_DEBUG
simple_lock_data_t	select_element_count_lock;
int			select_element_count;
static void		increment_select_element_count();
static void		decrement_select_element_count();
#define	SELDEBUG(s)	s
#else
#define	SELDEBUG(s)
#endif


void
select_init()
{
	if (select_max_elements < 1024)
		panic("select_init:  select_max_elements %d\n",
		      select_max_elements);
	if (select_chunk_elements < 128)
		panic("select_init:  select_chunk_elements %d\n",
		      select_chunk_elements);
	select_zone = zinit(sizeof(struct sel_queue),
		round_page(select_max_elements * sizeof(struct sel_queue)),
		round_page(select_chunk_elements * sizeof(struct sel_queue)),
		"Select queues");
#if	SELECT_DEBUG
	simple_lock_init(&select_element_count_lock);
	select_element_count = 0;
#endif
}


/*
 * select_enqueue:  add the current thread to the list
 * of threads waiting for something to happen.
 *
 * N.B.:  The lock protecting this queue must be held
 * while calling this routine.
 */
void
select_enqueue(selq)
sel_queue_t	*selq;
{
	sel_queue_t	*qp;
	struct thread	*th;

	th = current_thread();
	qp = (sel_queue_t *) zalloc(select_zone);
	qp->event = &th->select_event;
	enqueue_tail(&selq->links, qp);
	SELDEBUG(increment_select_element_count());
}


/*
 * select_dequeue:  remove the current thread from
 * the list of threads waiting for something to happen.
 *
 * N.B.:  The lock protecting this queue must be held
 * while calling this routine.
 * N.B.:  We can't assume that the element won't be re-
 * allocated as soon as it is zfree'd.
 */
void
select_dequeue(selq)
sel_queue_t	*selq;
{
	register event_t	*target;
	register sel_queue_t	*qp;
	register sel_queue_t	*next;
	struct thread		*th;

	th = current_thread();
	target = &th->select_event;
	qp = (sel_queue_t *) queue_first(&selq->links);
	while (!queue_end(qp, (sel_queue_t *) &selq->links)) {
		if (qp->event == target) {
			remqueue(&selq->links, &qp->links);
			next = (sel_queue_t *) queue_next(&qp->links);
			zfree(select_zone, (vm_offset_t) qp);
			SELDEBUG(decrement_select_element_count());
			qp = next;
		}
		else
			qp = (sel_queue_t *) queue_next(&qp->links);
	}
}


void
select_dequeue_all(selq)
sel_queue_t	*selq;
{
	register sel_queue_t	*qp;
	register sel_queue_t	*next;

	while (!queue_empty(&selq->links))
	{
		qp = (sel_queue_t *) dequeue_head(&selq->links);
		zfree(select_zone, (vm_offset_t) qp);
		SELDEBUG(decrement_select_element_count());
	}
}


/*
 * select_wakeup:  Wakeup any threads who've selected
 * this event.
 *
 * N.B.:  The lock protecting this queue must be held
 * while calling this routine.
 */
void
select_wakeup(selq)
sel_queue_t	*selq;
{
	sel_queue_t	*qp;
	struct queue_entry *first;

	first = selq->links.prev->next;
	while(selq->links.next != first) {
		qp = (sel_queue_t *)selq->links.next;
		event_post(qp->event);
		selq = (sel_queue_t *)selq->links.next;
	}
}


#if	SELECT_DEBUG
static void
increment_select_element_count()
{
	simple_lock(&select_element_count_lock);
	++select_element_count;
	simple_unlock(&select_element_count_lock);
}

static void
decrement_select_element_count()
{
	simple_lock(&select_element_count_lock);
	--select_element_count;
	simple_unlock(&select_element_count_lock);
}
#endif
