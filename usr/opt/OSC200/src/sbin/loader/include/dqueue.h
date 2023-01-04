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
 *	@(#)$RCSfile: dqueue.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:36:41 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* dqueue.h
 * Data structure declarations and macro definitions for the double-ended
 * queue manipulation functions.  The queues are maintained in this format:
 * every queueable object has as its first two longwords
 * forward and backward pointers.  When the object is not in a queue both
 * pointers point to the object itself.
 * Queues are assumed to be headed by a queue header, which looks like a
 * queueable object (begins with forward and backward pointers).
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_DQUEUE
#define	_H_DQUEUE


struct	dqueue_elem {			/* a generic queue element */
	struct dqueue_elem *dq_forw;	/* forward link */
	struct dqueue_elem *dq_back;	/* backward link */
};


/* Insert an element at the head of a queue.  First argument is a pointer
 * to the element, second is a pointer to the queue header.
 */

#define	dq_ins_head(q, e)	insque((struct dqueue_elem *)(e), \
(struct dqueue_elem *) (q))


/* Insert an element at the tail of a queue.  First argument is a pointer to
 * the element, second is a pointer to the queue header.
 */

#define	dq_ins_tail(q, e)	insque((struct dqueue_elem *)(e), \
((struct dqueue_elem *)(q))->dq_back)


/* Remove the element at the head of a queue and return it cast to a specified
 * type.  First argument is a pointer to the queue header, second is the type
 * of element to be returned.  Returns NULL if the queue is empty.
 */

#define	dq_rem_head(q, t)	(dq_empty((struct dqueue_elem *)(q)) ? NULL : \
(t)remque(((struct dqueue_elem *)(q))->dq_forw))


/* Remove the element at the tail of a queue and return it cast to a specified
 * type.  First argument is a pointer to the queue header, second is the type
 * of element to be returned.  Returns NULL if the queue is empty.
 */

#define	dq_rem_tail(q, t)	(dq_empty((struct dqueue_elem *)(q)) ? NULL : \
(t)remque(((struct dqueue_elem *)(q))->dq_back))


/* Return nonzero if the specified queue is empty, 0 otherwise.  The argument
 * is a pointer to the queue header.
 */

#define	dq_empty(q)		(((struct dqueue_elem *)(q))->dq_forw == \
(struct dqueue_elem *)(q))


/* Return nonzero if the specified element is presently in a queue, 0
 * otherwise.  Argument is a pointer to the element.
 */

#define	dq_enqueued(e)		(((struct dqueue_elem *)(e))->dq_forw != \
(struct dqueue_elem *)(e))


/* Insert element e after element p in whatever queue p is presently
 * in.  This is just insque, with the appropriate casts done already.
 */

#define	dq_ins_after(e, p)	insque((struct dqueue_elem *)(e), \
(struct dqueue_elem *)(p))


/* Insert element e before element p in whatever queue p is presently
 * in.
 */

#define	dq_ins_before(e, p)	insque((struct dqueue_elem *)(e), \
((struct dqueue_elem *)(p))->dq_back)


/* Remove the specified element from whatever queue it's presently in.  This
 * is just remque with the appropriate casts already done.
 */

#define	dq_rem_elem(e)		remque((struct dqueue_elem *)(e))


/* Return a pointer to the element at the head of the specified queue, cast
 * to a specified type.  First argument is a pointer to the queue header,
 * second is the type of element to be returned.  Returns NULL if the queue
 * is empty.
 */

#define	dq_head(q, t)		(q_empty(q) ? NULL : \
(t)((struct dqueue_elem *)(q))->dq_forw)


/* Return a pointer to the element at the tail of the specified queue, cast
 * to a specified type.  First argument is a pointer to the queue header,
 * second is the type of element to be returned.  Returns NULL if the queue
 * is empty.
 */

#define	dq_tail(q, t)		(q_empty(q) ? NULL : \
(t)((struct dqueue_elem *)(q))->dq_back)


/* Initialize a queue header so its forward and backward links point to itself.
 */

#define	dq_init(q)		((struct dqueue_elem *)(q))->dq_forw = \
((struct dqueue_elem *)(q))->dq_back = (struct dqueue_elem *)(q)

/* insque and remque themselves */

extern void insque __((struct dqueue_elem *elem, struct dqueue_elem *pred));
extern struct dqueue_elem *remqueue __((struct dqueue_elem *elem));

#endif	/* _H_DQUEUE */
