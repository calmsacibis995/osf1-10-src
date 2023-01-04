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
 *	@(#)$RCSfile: squeue.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:33 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* squeue.h
 * Data structure declarations and macro definitions for the single-ended
 * queue manipulation functions.  The queues are maintained in this format:
 * every queueable object has as its first longwords a forward pointer.
 * When the object is not in a queue the pointer is NULL.
 * Two formats of queue headers are supported: queue headers containing
 * just a head pointer, and queue headers containing both head and
 * tail pointers.  Different routines are provided for each.
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_SQUEUE
#define	_H_SQUEUE

#include "ldr_macro_help.h"


struct	sq_elem	{			/* queue element: cast to right type */
	struct	sq_elem	*sq_forw;	/* it's just a pointer to next elt */
};

struct	squeue1	{			/* queue header with just head ptr */
	struct sq_elem	*sq_head;	/* first element in queue */
};

struct	squeue2	{			/* queue header with head and tail ptrs */
	struct sq_elem	*sq_head;	/* first element in queue */
	struct sq_elem	*sq_tail;	/* last element in queue */
};

/* The following macros implement most of the common queue operations */

/* Initialize a queue header */

#define	sq1_init(q)	((struct squeue1 *)(q))->sq_head = NULL

#define	sq2_init(q) \
	((struct squeue2 *)(q))->sq_head = ((struct squeue2 *)(q))->sq_tail = NULL

/* Add an element to the head of the queue */

#define	sq1_ins_head(q, elt) 	MACRO_BEGIN \
	((struct sq_elem *)(elt))->sq_forw = ((struct squeue1 *)(q))->sq_head; \
	((struct squeue1 *)(q))->sq_head = ((struct sq_elem *)(elt)); \
MACRO_END

#define	sq2_ins_head(q, elt) 	MACRO_BEGIN \
	if (((struct squeue2 *)(q))->sq_head == NULL) \
		((struct squeue2 *)(q))->sq_tail = ((struct sq_elem *)(elt)); \
	((struct sq_elem *)(elt))->sq_forw = ((struct squeue2 *)(q))->sq_head; \
	(q)->sq_head = ((struct sq_elem *)(elt)); \
MACRO_END

/* Add an element to the tail of a queue */

#define	sq2_ins_tail(q, elt)	MACRO_BEGIN \
	((struct sq_elem *)(elt))->sq_forw = NULL; \
	if (((struct squeue2 *)(q))->sq_head == NULL) { \
		((struct squeue2 *)(q))->sq_head = ((struct sq_elem *)(elt)); \
	} else { \
		((struct squeue2 *)(q))->sq_tail->sq_forw = ((struct sq_elem *)(elt)); \
	} \
	((struct squeue2 *)(q))->sq_tail = ((struct sq_elem *)(elt)); \
MACRO_END

/* Add an element after a specified element in the queue.  If prev == */
/* &q->sq_head, can be used to add an element to the head of the queue */

#define	sq1_ins_after(new, prev)	MACRO_BEGIN \
	((struct sq_elem *)(new))->sq_forw = ((struct sq_elem *)(prev))->sq_forw; \
	((struct sq_elem *)(prev))->sq_forw = ((struct sq_elem *)(new)); \
MACRO_END

#define	sq2_ins_after(q, new, prev)	MACRO_BEGIN \
	if (((struct squeue2 *)(q))->sq_tail == ((struct sq_elem *)(prev)) || \
	    ((struct squeue2 *)(q))->sq_tail == NULL) { \
		((struct squeue2 *)(q))->sq_tail = ((struct sq_elem *)(new)); \
	} \
	((struct sq_elem *)(new))->sq_forw = ((struct sq_elem *)(prev))->sq_forw; \
	((struct sq_elem *)(prev))->sq_forw = ((struct sq_elem *)(new)); \
MACRO_END

/* Delete an element from a queue, given a pointer to the preceeding element */
/* Will delete the first element if prev == &q->sq_head */

#define	sq1_rem_after(elt, prev)	MACRO_BEGIN \
	((struct sq_elem *)(prev))->sq_forw = ((struct sq_elem *)(elt))->sq_forw; \
	((struct sq_elem *)(elt))->sq_forw = NULL; \
MACRO_END

#define	sq2_rem_after(q, elt, prev)	MACRO_BEGIN \
	if (((struct squeue2 *)(q))->sq_tail == ((struct sq_elem *)(elt))) { \
		if (((struct squeue2 *)(q))->sq_head == ((struct sq_elem *)(elt))) \
			((struct squeue2 *)(q))->sq_tail = NULL; \
		else \
			((struct squeue2 *)(q))->sq_tail = ((struct sq_elem *)(prev)); \
	} \
	((struct sq_elem *)(prev))->sq_forw = ((struct sq_elem *)(elt))->sq_forw; \
	((struct sq_elem *)(elt))->sq_forw = NULL; \
MACRO_END

/* Return nonzero if the specified queue is empty, 0 otherwise. */

#define	sq1_empty(q)	(((struct squeue1 *)(q))->sq_head == NULL)

#define	sq2_empty(q)	(((struct squeue2 *)(q))->sq_head == NULL)

/* Return a pointer to the element at the head of the specified queue, cast
 * to a specified type.  First argument is a pointer to the queue header,
 * second is the type of element to be returned.  Returns NULL if the queue
 * is empty.
 */

#define	sq1_head(q, t)		(sq1_empty(q) ? NULL : \
(t)((struct squeue1 *)(q))->sq_head)

#define	sq2_head(q, t)		(sq2_empty(q) ? NULL : \
(t)((struct squeue2 *)(q))->sq_head)


/* Return a pointer to the element at the tail of the specified queue, cast
 * to a specified type.  First argument is a pointer to the queue header,
 * second is the type of element to be returned.  Returns NULL if the queue
 * is empty.
 */

#define	sq2_tail(q, t)		(sq2_empty(q) ? NULL : \
(t)((struct squeue2 *)(q))->sq_tail)


/* Add an element to the tail of an sq1 */

extern void sq1_ins_tail __((struct squeue1 *q, struct sq_elem *elt));

/* Dequeue and return the first element of the specified queue.  Returns
 * a pointer to the first element if any, or NULL if the queue is empty.
 * Variants are for both queue header types.
 */

extern struct sq_elem *sq1_rem_head __((struct squeue1 *q));

extern struct sq_elem *sq2_rem_head __((struct squeue2 *q));

/* Dequeue and return the last element of the specified queue.  Returns
 * a pointer to the last element if any, or NULL if the queue is empty.
 * Note that for this queue type this is an O(n) operation.
 * Variants are for both queue header types.
 */

extern struct sq_elem *sq1_rem_tail __((struct squeue1 *q));

extern struct sq_elem *sq2_rem_tail __((struct squeue2 *q));

/* Delete the specified element from the queue.  This requires scanning
 * the queue from the top to find and remove the element, so it takes
 * O(queue length) time to execute.  Note that this routine must not
 * run at interrupt level.
 * Returns nonzero if the element is successfully deleted, or 0 if
 * the element is not found in the queue.
 * Variants are for both queue header types.
 */

extern int sq1_rem_elem __((struct squeue1 *q, struct sq_elem *elt));

extern int sq2_rem_elem __((struct squeue2 *q, struct sq_elem *elt));

#endif	/* _H_SQUEUE */
