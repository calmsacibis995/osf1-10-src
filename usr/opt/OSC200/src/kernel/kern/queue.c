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
static char	*sccsid = "@(#)$RCSfile: queue.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:26:45 $";
#endif 
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
 *	Routines to implement queue package.
 */

#include <kern/queue.h>

#if	defined(lint) || !defined(vax)

/*
 *	Insert element at head of queue.
 */
void enqueue_head(que, elt)
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
void enqueue_tail(que,elt)
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
queue_entry_t dequeue_head(que)
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
queue_entry_t dequeue_tail(que)
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
void remqueue(que, elt)
	queue_t			que;
	register queue_entry_t	elt;
{
	elt->next->prev = elt->prev;
	elt->prev->next = elt->next;
}

#endif	defined(lint) || !defined(vax)

/*
 *	Routines to directly imitate the VAX hardware queue
 *	package.
 */
#if	!(defined(vax) || defined(sun))
insque(entry, pred)
	register struct queue_entry *entry, *pred;
{
	entry->next = pred->next;
	entry->prev = pred;
	(pred->next)->prev = entry;
	pred->next = entry;
}

remque(elt)
	register struct queue_entry *elt;
{
	(elt->next)->prev = elt->prev;
	(elt->prev)->next = elt->next;
	return((int)elt);
}
#endif	!(defined(vax) || defined(sun))
