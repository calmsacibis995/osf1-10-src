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
static char	*sccsid = "@(#)$RCSfile: kd_queue.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:25 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/* **********************************************************************
 File:         kd_queue.c
 Description:  Event queue code for keyboard/display (and mouse) driver.


 Copyright Ing. C. Olivetti & C. S.p.A. 1989.
 All rights reserved.
********************************************************************** */


#include <i386/AT386/kd_queue.h>

/*
 * Notice that when adding an entry to the queue, the caller provides 
 * its own storage, which is copied into the queue.  However, when 
 * removing an entry from the queue, the caller is given a pointer to a 
 * queue element.  This means that the caller must either process the 
 * element or copy it into its own storage before unlocking the queue.
 *
 * These routines should be called only at a protected SPL.
 */

#define q_next(index)	(((index)+1) % KDQSIZE)

boolean_t
kdq_empty(q)
	kd_event_queue *q;
{
	return(q->firstfree == q->firstout);
}

boolean_t
kdq_full(q)
	kd_event_queue *q;
{
	return(q_next(q->firstfree) == q->firstout);
}

void
kdq_put(q, ev)
	kd_event_queue *q;
	kd_event *ev;
{
	kd_event *qp = q->events + q->firstfree;

	qp->type = ev->type;
	qp->time = ev->time;
	qp->value = ev->value;
	q->firstfree = q_next(q->firstfree);
}

kd_event *
kdq_get(q)
	kd_event_queue *q;
{
	kd_event *result = q->events + q->firstout;

	q->firstout = q_next(q->firstout);
	return(result);
}

void
kdq_reset(q)
	kd_event_queue *q;
{
	q->firstout = q->firstfree = 0;
}
