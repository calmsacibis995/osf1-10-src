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
 *	@(#)$RCSfile: queue.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 17:54:54 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	File:	queue.h
 *
 *	Type definitions for generic queues.
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_QUEUE_H_
#define _KERN_QUEUE_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_QUEUE_H_PREEMPT_
#endif
#endif

#include <mach/machine/vm_types.h>
#include <kern/lock.h>
#include <kern/macro_help.h>

/*
 *	Queue of abstract objects.  Queue is maintained
 *	within that object.
 *
 *	Supports fast removal from within the queue.
 *
 *	How to declare a queue of elements of type "foo_t":
 *		In the "*foo_t" type, you must have a field of
 *		type "queue_chain_t" to hold together this queue.
 *		There may be more than one chain through a
 *		"foo_t", for use by different queues.
 *
 *		Declare the queue as a "queue_t" type.
 *
 *		Elements of the queue (of type "foo_t", that is)
 *		are referred to by reference, and cast to type
 *		"queue_entry_t" within this module.
 */

/*
 *	A generic doubly-linked list (queue).
 */

struct queue_entry {
	struct queue_entry	*next;		/* next element */
	struct queue_entry	*prev;		/* previous element */
};

typedef struct queue_entry	*queue_t;
typedef	struct queue_entry	queue_head_t;
typedef	struct queue_entry	queue_chain_t;
typedef	struct queue_entry	*queue_entry_t;

/*
 *	enqueue puts "elt" on the "queue".
 *	dequeue returns the first element in the "queue".
 *	remqueue removes the specified "elt" from the specified "queue".
 */

#define enqueue(queue,elt)	enqueue_tail(queue, elt)
#define dequeue(queue)		dequeue_head(queue)

extern void		enqueue_head();
extern void		enqueue_tail();
extern queue_entry_t	dequeue_head();
extern queue_entry_t	dequeue_tail();
extern void		remqueue();

/*
 *	Macro:		queue_init
 *	Function:
 *		Initialize the given queue.
 *	Header:
 *		void queue_init(q)
 *			queue_t		q;	/* MODIFIED *\
 */
#define queue_init(q)	((q)->next = (q)->prev = q)

/*
 *	Macro:		queue_first
 *	Function:
 *		Returns the first entry in the queue,
 *	Header:
 *		queue_entry_t queue_first(q)
 *			queue_t	q;		/* IN *\
 */
#define queue_first(q)	((q)->next)

/*
 *	Macro:		queue_next
 *	Header:
 *		queue_entry_t queue_next(qc)
 *			queue_t qc;
 */
#define queue_next(qc)	((qc)->next)

/*
 *	Macro:		queue_end
 *	Header:
 *		boolean_t queue_end(q, qe)
 *			queue_t q;
 *			queue_entry_t qe;
 */
#define queue_end(q, qe)	((q) == (qe))

#define queue_empty(q)		queue_end((q), queue_first(q))

/*
 *	Macro:		queue_enter
 *	Header:
 *		void queue_enter(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_enter(head, elt, type, field)			\
MACRO_BEGIN							\
	if (queue_empty((head))) {				\
		(head)->next = (queue_entry_t) elt;		\
		(head)->prev = (queue_entry_t) elt;		\
		(elt)->field.next = head;			\
		(elt)->field.prev = head;			\
	}							\
	else {							\
		register queue_entry_t prev;			\
								\
		prev = (head)->prev;				\
		(elt)->field.prev = prev;			\
		(elt)->field.next = head;			\
		(head)->prev = (queue_entry_t)(elt);		\
		((type)prev)->field.next = (queue_entry_t)(elt);\
	}							\
MACRO_END

/*
 *	Macro:		queue_field [internal use only]
 *	Function:
 *		Find the queue_chain_t (or queue_t) for the
 *		given element (thing) in the given queue (head)
 */
#define queue_field(head, thing, type, field)			\
		(((head) == (thing)) ? (head) : &((type)(thing))->field)

/*
 *	Macro:		queue_remove
 *	Header:
 *		void queue_remove(q, qe, type, field)
 *			arguments as in queue_enter
 */
#define queue_remove(head, elt, type, field)			\
MACRO_BEGIN							\
	register queue_entry_t	next, prev;			\
								\
	next = (elt)->field.next;				\
	prev = (elt)->field.prev;				\
								\
	queue_field((head), next, type, field)->prev = prev;	\
	queue_field((head), prev, type, field)->next = next;	\
MACRO_END

/*
 *	Macro:		queue_assign
 */
#define queue_assign(to, from, type, field)			\
MACRO_BEGIN							\
	((type)((from)->prev))->field.next = (to);		\
	((type)((from)->next))->field.prev = (to);		\
	*to = *from;						\
MACRO_END

#define queue_remove_first(h, e, t, f)				\
MACRO_BEGIN							\
	e = (t) queue_first((h));				\
	queue_remove((h), (e), t, f);				\
MACRO_END

#define queue_remove_last(h, e, t, f)				\
MACRO_BEGIN							\
	e = (t) queue_last((h));				\
	queue_remove((h), (e), t, f);				\
MACRO_END

/*
 *	Macro:		queue_enter_first
 *	Header:
 *		void queue_enter_first(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_enter_first(head, elt, type, field)		\
MACRO_BEGIN							\
	if (queue_empty((head))) {				\
		(head)->next = (queue_entry_t) elt;		\
		(head)->prev = (queue_entry_t) elt;		\
		(elt)->field.next = head;			\
		(elt)->field.prev = head;			\
	}							\
	else {							\
		register queue_entry_t next;			\
								\
		next = (head)->next;				\
		(elt)->field.prev = head;			\
		(elt)->field.next = next;			\
		(head)->next = (queue_entry_t)(elt);		\
		((type)next)->field.prev = (queue_entry_t)(elt);\
	}							\
MACRO_END

/*
 *	Macro:		queue_last
 *	Function:
 *		Returns the last entry in the queue,
 *	Header:
 *		queue_entry_t queue_last(q)
 *			queue_t	q;		/* IN *\
 */
#define queue_last(q)	((q)->prev)

/*
 *	Macro:		queue_prev
 *	Header:
 *		queue_entry_t queue_prev(qc)
 *			queue_t qc;
 */
#define queue_prev(qc)	((qc)->prev)

/*
 *	Define macros for queues with locks.
 */
struct mpqueue_head {
	struct queue_entry	head;		/* header for queue */
	decl_simple_lock_data(,lock)		/* lock for queue */
};

typedef struct mpqueue_head	mpqueue_head_t;

#define round_mpq(size)		(size)

#define mpqueue_init(q)						\
MACRO_BEGIN							\
	queue_init(&(q)->head);					\
	simple_lock_init(&(q)->lock);				\
MACRO_END

#define mpenqueue_tail(q, elt)					\
MACRO_BEGIN							\
	simple_lock(&(q)->lock);				\
	enqueue_tail(&(q)->head, elt);				\
	simple_unlock(&(q)->lock);				\
MACRO_END

#define mpdequeue_head(q, elt)					\
MACRO_BEGIN							\
	simple_lock(&(q)->lock);				\
	if (queue_empty(&(q)->head))				\
		*(elt) = 0;					\
	else							\
		*(elt) = dequeue_head(&(q)->head);		\
	simple_unlock(&(q)->lock);				\
MACRO_END

#define QWAIT	1
#define QNOWAIT	0
#define mpdequeue1(q, elt, wait)				\
MACRO_BEGIN							\
	for (;;) {						\
		register int s = splhigh();			\
		simple_lock(&(q)->lock);			\
		if (queue_empty(&(q)->head))			\
			if (wait) {				\
				assert_wait((vm_offset_t) (q), FALSE);	\
				simple_unlock(&(q)->lock);	\
				splx(s);			\
				thread_block();			\
				continue;			\
			} else					\
				*(elt) = 0;			\
		else						\
			*((caddr_t *)elt) = (caddr_t)dequeue_head(&(q)->head); \
		simple_unlock(&(q)->lock);			\
		splx(s);					\
		break;						\
	}							\
MACRO_END

#define mpenqueue1(q, elt)					\
MACRO_BEGIN							\
	register int empty = 0;					\
	register int s = splhigh();				\
	simple_lock (&(q)->lock);				\
	if (queue_empty(&(q)->head)) ++empty;			\
	enqueue_tail(&(q)->head, (elt));			\
	simple_unlock (&(q)->lock);				\
	splx(s);						\
	if (empty) thread_wakeup((vm_offset_t)(q));		\
MACRO_END

/*
 *	Old queue stuff, will go away soon.
 */

/*
 * General purpose structure to define circular queues.
 *  Both the queue header and the queue elements have this
 *  structure.
 */

struct Queue
{
    struct Queue * F;
    struct Queue * B;
};

#define initQueue(q)	(queue_init((queue_t)(q)))
#define enQueue(q,elt)	(enqueue_tail((queue_t)(q),(queue_entry_t)(elt)))
#define deQueue(q)	((struct Queue *)dequeue_head((queue_t)(q)))
#define remQueue(q,elt)	(remqueue((queue_t)(q),(queue_entry_t)(elt)))
#define Queueempty(q)	(queue_empty((queue_t)(q)))


#if	__GNUC__ && !_NO_INLINE_QUEUE
/* Define fast inline functions for queues */
/*
 *	Insert element at head of queue.
 */
extern void __inline__ enqueue_head(que, elt)
	register queue_t	que;
	register queue_entry_t	elt;
{
	elt->next = que->next;
	elt->prev = que;
	elt->next->prev = elt;
	que->next = elt;
}

/*
 *	Insert element at tail of queue.
 */
extern void __inline__ enqueue_tail(que,elt)
	register queue_t	que;
	register queue_entry_t	elt;
{
	elt->next = que;
	elt->prev = que->prev;
	elt->prev->next = elt;
	que->prev = elt;
}

/*
 *	Remove and return element at head of queue.
 */
extern queue_entry_t __inline__ dequeue_head(que)
	register queue_t	que;
{
	register queue_entry_t	elt;

	if (que->next == que)
		return((queue_entry_t)0);

	elt = que->next;
	elt->next->prev = que;
	que->next = elt->next;
	return(elt);
}

/*
 *	Remove and return element at tail of queue.
 */
extern queue_entry_t __inline__ dequeue_tail(que)
	register queue_t	que;
{
	register queue_entry_t	elt;

	if (que->prev == que)
		return((queue_entry_t)0);

	elt = que->prev;
	elt->prev->next = que;
	que->prev = elt->prev;
	return(elt);
}

/*
 *	Remove arbitrary element from queue.
 *	Does not check whether element is on queue - the world
 *	will go haywire if it isn't.
 */

/*ARGSUSED*/
extern void __inline__ remqueue(que, elt)
	queue_t			que;
	register queue_entry_t	elt;
{
	elt->next->prev = elt->prev;
	elt->prev->next = elt->next;
}

#if	!defined(vax)
extern __inline__ insque(entry, pred)
	register struct queue_entry *entry, *pred;
{
	entry->next = pred->next;
	entry->prev = pred;
	(pred->next)->prev = entry;
	pred->next = entry;
}

extern __inline__ remque(elt)
	register struct queue_entry *elt;
{
	(elt->next)->prev = elt->prev;
	(elt->prev)->next = elt->next;
	return((int)elt);
}
#endif	/* vax */

#endif	/* __GNUC__ && !_NO_INLINE_QUEUE */

#if	RT_PREEMPT
#ifdef	_KERN_QUEUE_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_QUEUE_H_ */
