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
static char	*sccsid = "@(#)$RCSfile: squeue.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/07 16:21:19 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* squeue.c
 * General-purpose queue manipulation routines.
 * All other general-purpose queue manipulation routines are macros
 * defined in "squeue.h".
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <loader.h>

#include "ldr_types.h"
#include "squeue.h"

void
sq1_ins_tail(struct squeue1 *q, struct sq_elem *elt)

/* Add an element to the tail of an sq1.  This requires searching
 * the queue to find the current tail, then adding the element
 * after it.
 * Note that for this queue type this is an O(n) operation.
 */
{
	struct	sq_elem	*prev;

	for (prev = (struct sq_elem *)(q); prev->sq_forw != NULL;
	     prev = prev->sq_forw)
		;			/* null loop body */

	sq1_ins_after(elt, prev);
}


struct sq_elem *
sq1_rem_head(struct squeue1 *q)

/* Dequeue and return the first element of the specified sq1.  Returns
 * a pointer to the first element if any, or NULL if the queue is empty.
 */
{
	register struct sq_elem *temp;	/* temp for result */
	
	if ((temp = q->sq_head) == NULL) /* queue empty? */
		return (NULL);		/* yes, show none */
	sq1_rem_after(temp, (struct sq_elem *)q);
	return (temp);
}


struct sq_elem *
sq2_rem_head(struct squeue2 *q)

/* Dequeue and return the first element of the specified sq2.  Returns
 * a pointer to the first element if any, or NULL if the queue is empty.
 */
{
	register struct sq_elem *temp;	/* temp for result */
	
	if ((temp = q->sq_head) == NULL) /* queue empty? */
		return (NULL);		/* yes, show none */
	sq2_rem_after(q, temp, (struct sq_elem *)q);
	return (temp);
}


struct sq_elem *
sq1_rem_tail(struct squeue1 *q)

/* Dequeue and return the last element of the specified queue.  Returns
 * a pointer to the last element if any, or NULL if the queue is empty.
 * Note that for this queue type this is an O(n) operation.
 * Variants are for both queue header types.
 */
{
	struct	sq_elem	*prev, *elt;

	if (sq1_empty(q))
		return(NULL);

	for (prev = (struct sq_elem *)(q); prev->sq_forw->sq_forw != NULL;
	     prev = prev->sq_forw)
		;			/* null loop body */

	elt = prev->sq_forw;
	sq1_rem_after(elt, prev);
	return(elt);
}


struct sq_elem *
sq2_rem_tail(struct squeue2 *q)

/* Dequeue and return the last element of the specified queue.  Returns
 * a pointer to the last element if any, or NULL if the queue is empty.
 * Variants are for both queue header types.
 */
{
	struct	sq_elem	*prev, *elt;

	if (sq2_empty(q))
		return(NULL);

	for (prev = (struct sq_elem *)(q); prev->sq_forw->sq_forw != NULL;
	     prev = prev->sq_forw)
		;			/* null loop body */

	elt = prev->sq_forw;
	sq2_rem_after(q, elt, prev);
	return(elt);
}


int
sq1_rem_elem(struct squeue1 *q, struct sq_elem *elt)

/* Delete the specified element from the sq1.  This requires scanning
 * the queue from the top to find and remove the element, so it takes
 * O(queue length) time to execute.  Note that this routine must not
 * run at interrupt level.
 * Returns nonzero if the element is successfully deleted, or 0 if
 * the element is not found in the queue.
 */
{
	register struct sq_elem	*prev;	/* temp for chaining */
	
	for (prev = (struct sq_elem *)(q); prev != NULL; prev = prev->sq_forw) {

		if (prev->sq_forw == elt) {
			sq1_rem_after(elt, prev);
			return (1);
		}
	}

	return(0);
}


int
sq2_rem_elem(struct squeue2 *q, struct sq_elem *elt)

/* Delete the specified element from the sq2.  This requires scanning
 * the queue from the top to find and remove the element, so it takes
 * O(queue length) time to execute.  Note that this routine must not
 * run at interrupt level.
 * Returns nonzero if the element is successfully deleted, or 0 if
 * the element is not found in the queue.
 */
{
	register struct sq_elem	*prev;	/* temp for chaining */
	
	for (prev = (struct sq_elem *)(q); prev != NULL; prev = prev->sq_forw) {

		if (prev->sq_forw == elt) {
			sq2_rem_after(q, elt, prev);
			return (1);
		}
	}

	return(0);
}
