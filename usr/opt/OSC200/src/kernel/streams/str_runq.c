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
static char *rcsid = "@(#)$RCSfile: str_runq.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/03 19:40:37 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1989-1991  Mentat Inc.  **/

#include <sys/param.h>
#include <sys/stat.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>

#include <net/netisr.h>

/*
 * Streams Run Queue Management
 */

SQH	streams_runq;

void
runq_init()
{
	int		n;

	ENTER_FUNC(runq_init, 0, 0, 0, 0);

	sqh_init(&streams_runq);
	streams_runq.sqh_parent = &streams_runq;
	if (netisr_add(NETISR_STREAMS, runq_run,
			(struct ifqueue *)0, (struct domain *)0))
		panic("runq_init");

	LEAVE_FUNC(runq_init, 0);
}

/*
 * runq_run - STREAMS thread "interrupt" routine.
 */
void
runq_run ()
{
reg	SQP	sq;
reg	queue_t	*q;
reg	SQHP	sqh = &streams_runq;
	SIMPLE_LOCK_DECL

	ENTER_FUNC(runq_run, 0, 0, 0, 0);

	LOCK_QUEUE(sqh);
	for ( ;; ) {
		sq = sqh->sqh_next;
		if ( sq == (SQP)sqh )
			break;
		remque(sq);
		UNLOCK_QUEUE(sqh);
		if (q = sq->sq_queue) {
#if	MACH_ASSERT
			SIMPLE_LOCK(&q->q_qlock);
			q->q_runq_sq->sq_flags &= ~SQ_QUEUED;
			SIMPLE_UNLOCK(&q->q_qlock);
#endif
			csq_lateral(&q->q_sqh, sq);
		} else {
			void *arg1 = sq->sq_arg1;
			sq->sq_arg1 = 0;
			(*sq->sq_entry)(sq->sq_arg0, arg1);
		}
		LOCK_QUEUE(sqh);
	}
	UNLOCK_QUEUE(sqh);
}

/*
 * sq_wrapper - wrapper function for the q's service procedure.
 */

staticf void
sq_wrapper (q)
	queue_t	* q;
{
	SIMPLE_LOCK_DECL

	SIMPLE_LOCK(&q->q_qlock);
	q->q_flag &= ~QWANTR;
#if	MACH_ASSERT
	if ((q->q_runq_sq->sq_flags & SQ_INUSE) == 0 || !q->q_qinfo->qi_srvp)
		panic("sq_wrapper");
#endif
	q->q_runq_sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
	SIMPLE_UNLOCK(&q->q_qlock);
	(void) (*q->q_qinfo->qi_srvp)(q);
}

/*
 * qenable - Enable a queue
 */
void
qenable (q)
reg	queue_t	* q;
{

	ENTER_FUNC(qenable, q, 0, 0, 0);
	if (q->q_qinfo->qi_srvp) {
		SQP	sq = q->q_runq_sq;
		int	gotit;
		SIMPLE_LOCK_DECL

		SIMPLE_LOCK(&q->q_qlock);
		if (gotit = !(sq->sq_flags & SQ_INUSE)) {
#if	MACH_ASSERT
			if (sq->sq_flags & SQ_QUEUED)
				panic("qenable");
#endif
			sq->sq_flags |= (SQ_INUSE|SQ_QUEUED);
			simple_lock(&streams_runq.sqh_lock);
			insque(sq, streams_runq.sqh_prev);
			simple_unlock(&streams_runq.sqh_lock);
		}
		SIMPLE_UNLOCK(&q->q_qlock);
		if (gotit)	/* Wake up a STREAMS thread */
			schednetisr(NETISR_STREAMS);
	}
	LEAVE_FUNC(qenable, 0);
}

/*
 * runq_sq_init - initialize a run queue element
 *
 * Called by q_alloc to initialize the run queue element which is
 * contained in every queue.
 */

void
runq_sq_init (q)
	queue_t	* q;
{
	ENTER_FUNC(runq_sq_init, q, 0, 0, 0);

	sq_init(q->q_runq_sq);
	q->q_runq_sq->sq_entry = (sq_entry_t)sq_wrapper;
	q->q_runq_sq->sq_queue = q;
	q->q_runq_sq->sq_arg0  = q;
	q->q_runq_sq->sq_arg1  = 0;

	LEAVE_FUNC(runq_sq_init, 0);
}

/*
 * runq_remove - remove a streams queue from the run queue.
 *
 * The specified queue is about to be deallocated.  If, somehow, it has
 * recently gotten itself scheduled, we need to get it off the run queue. 
 * This routine only needs to take action in a multiprocessing or preemptive
 * environment.
 */

void
runq_remove (q)
	queue_t	* q;
{
	SQP	sq = q->q_runq_sq;
	SIMPLE_LOCK_DECL

	ENTER_FUNC(runq_remove, q, 0, 0, 0);

	SIMPLE_LOCK(&q->q_qlock);
	if (sq->sq_flags & SQ_INUSE) {
		simple_lock(&streams_runq.sqh_lock);
		remque(sq);
		simple_unlock(&streams_runq.sqh_lock);
		sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
	}
	SIMPLE_UNLOCK(&q->q_qlock);

	LEAVE_FUNC(runq_remove, 0);
}
