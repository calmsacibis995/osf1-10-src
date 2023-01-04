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
static char	*sccsid = "@(#)$RCSfile: MP_queue.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:21 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright 1990 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 *	General purpose multiprocessor queuing routines, includes support
 *		for free queue starvation recovery (done in thread).
 */
/*
 * OSF/1 Release 1.0
 */

#include "sys/types.h"
#include "sys/param.h"
#include "kern/lock.h"
#include "kern/queue.h"
#include "kern/sched_prim.h"
#include "mmax/MP_queue.h"
#include "mmax/isr_env.h"
#include "kern/assert.h"

MP_enqueue(q, elm)
register struct MP_q_hd	*q;
struct isr_que	*elm;
{
	register s;

	s = splhigh();
	MPQ_SLOCK(&q->lock);
	enqueue_tail(&q->head, elm);
	q->cnt++;
	MPQ_SULOCK(&q->lock);
	splx(s);
	thread_wakeup_one((int)q);
	return;
}

MP_dequeue(q, elmp, wait)
register struct MP_q_hd	*q;
register struct isr_que	**elmp;
register long			wait;
{
	register s;

dq_retry:
	s = splhigh();
	MPQ_SLOCK(&q->lock);
	ASSERT(!(queue_empty(&q->head)) || q->cnt == 0);
	ASSERT(q->cnt != 0 || queue_empty(&q->head));
	if(queue_empty(&q->head)) {
		if(wait == MP_QWAIT) {
			assert_wait((int)q, FALSE);
			MPQ_SULOCK(&q->lock);
			splx(s);
			thread_block();
			goto dq_retry;
		}
		*elmp = (struct isr_que *)NULL;
	} else {
		*elmp = (struct isr_que *)dequeue_head(&q->head);
		/*
		 * NOTE: The ISR threads are responsible
		 *	for allocating more queue memory
		 *	as needed.
		*/
		q->cnt--;
	}
	MPQ_SULOCK(&q->lock);
	splx(s);
	return;
}
