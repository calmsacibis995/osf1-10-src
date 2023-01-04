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
static char *rcsid = "@(#)$RCSfile: str_weld.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/23 16:05:42 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1988-1991  Mentat Inc.
 ** mi.c 1.15, last change 5/15/90
 **/

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>

#include <net/netisr.h>

/*
 * The weld story.
 *
 * It is not specified in the STREAMS interface whether and how a module
 * or a driver should be allowed to modify the q_next pointers. However,
 * it is an established practice to do this, e.g. for the following purposes:
 *
 *	(TODO: fill in legal motivations here)
 *
 * Of course, not every connection is "legal" - the STREAMS framework must
 * rely on certain "conventions" about q_next connections. Therefor, the
 * following restrictions exist:
 *
 *	(TODO: fill in exact specification what is "legal",
 *	       and identify which conditions get actually checked.)
 *
 * In OSF STREAMS, there are additional pointers between queues
 * which are used for efficient and safe operations between queues with
 * service procedures (keywords: flow control, protection domains). We
 * must therefor insist on doing these "semi-legal" operations only through
 * this interface. We have designed it in a way which should be flexible
 * enough to allow all reasonable applications.
 *
 * Since welding queues together requires us to acquire a number of queues,
 * and since such operations are restricted to synchronous activities (that
 * is, the stream head, open and close routines, but not put and service
 * procedures), we provide an asynchronous interface for the use from all
 * places. An optional callback routine can be used by anyone who needs
 * notification about the completion of the activity. It should be noted
 * that this function is called without any protection, and that it therefore
 * needs to either be a "safe" STREAMS utility (putq, qenable, wakeup), or
 * provide its own protection. (This is the reason why we allow two parameters
 * instead of the "generic" approach with just one parameter).
 */

struct weld_s {
	queue_t *	weld_q1;
	queue_t *	weld_q2;
	queue_t *	weld_q3;
	queue_t *	weld_q4;
	weld_fcn_t	weld_func;
	weld_arg_t	weld_arg;
	queue_t *	weld_queue;
	int		weld_op;
};

typedef	struct weld_s	WELD;
typedef	struct weld_s *	WELDP;

decl_simple_lock_data(static,weldq_lock)
struct { MBLKP next, prev; }	weldq_runq;
static	SQ			weldq_sq;

staticf	int		weldq_comm(queue_t *, queue_t *, queue_t *, queue_t *,
				weld_fcn_t, weld_arg_t, queue_t *, int);
staticf	void		weldq_main(void);

/*
 * weldq - establish a uni-directional q_next pointer
 *	   between one or two pairs of queues.
 *
 * Parameters:
 *	q1, q2, q3, q4 - queue parameters for welding (see functionality
 *	func, arg - function and argument for callback (optional)
 *	protect_q - queue under which func should be executed
 *
 * Functionality:
 *	if ( q1 )
 *		q1->q_next = q2;
 *	if ( q3 )
 *		q3->q_next = q4;
 *	if ( func )
 *		(*func)(arg); (protected by protect_q)
 *
 * Return Value: error status (from registration, not from execution)
 *	0	- no error
 *	EINVAL	- invalid parameters
 *	EAGAIN	- could not allocate weld record
 */

int
weldq (q1, q2, q3, q4, func, arg, protect_q)
	queue_t *	q1;
	queue_t *	q2;
	queue_t *	q3;
	queue_t *	q4;
	weld_fcn_t	func;
	weld_arg_t	arg;
	queue_t *	protect_q;
{
	if ( !weldq_runq.next )
		return ENXIO;
	/*
	 * consistency check of parameters
	 */
	if ( q1 && (!q2 || q1->q_next) )
		return EINVAL;
	if ( q3 && (!q4 || q3->q_next) )
		return EINVAL;
	return weldq_comm(q1, q2, q3, q4, func, arg, protect_q, 1);
}

/*
 * unweldq - remove a previously established weld connection.
 *
 * Parameters:
 *	q1, q2, q3, q4 - queue parameters for un-welding
 *	func, arg - function and argument for callback (optional)
 *	protect_q - queue under which func should be executed
 *
 * Functionality:
 *	q1->q_next = nil;
 *	q2->q_next = nil;
 *	if ( func )
 *		(*func)(arg);
 *
 * Return Value: error status (from registration, not from execution)
 *	0	- no error
 *	EINVAL	- invalid parameters
 *	EAGAIN	- could not allocate weld record
 *
 * Issues:
 *	- we could eliminate the need for parameters q2 and q4,
 *	  and figure them out ourselves.
 */
 
int
unweldq (q1, q2, q3, q4, func, arg, protect_q)
	queue_t *	q1;
	queue_t *	q2;
	queue_t *	q3;
	queue_t *	q4;
	weld_fcn_t	func;
	weld_arg_t	arg;
	queue_t *	protect_q;
{
	if ( !weldq_runq.next )
		return ENXIO;
	/*
	 * consistency check of parameters, as far as we can go here...
	 */
	if ( q1 && (!q2 || q1->q_next != q2) )
		return EINVAL;
	if ( q3 && (!q4 || q3->q_next != q4) )
		return EINVAL;

	return weldq_comm(q1, q2, q3, q4, func, arg, protect_q, 0);
}
 

/*
 *	weldq_comm - common subroutine for weldq and unweldq
 */
staticf int
weldq_comm (q1, q2, q3, q4, func, arg, protect_q, op)
	queue_t *	q1;
	queue_t *	q2;
	queue_t *	q3;
	queue_t *	q4;
	weld_fcn_t	func;
	weld_arg_t	arg;
	queue_t *	protect_q;
	int		op;
{
	MBLKP		weld_msg;
	WELDP		weld;

	/*
	 * Allocate a message to hold the weld job description.
	 * (We don't really need a message here, but a non-blocking
	 * allocator.)
	 */
	if ( (weld_msg = allocb(sizeof(WELD), BPRI_MED)) == nil(MBLKP) )
		return EAGAIN;

	weld = (WELDP)weld_msg->b_rptr;
	weld_msg->b_wptr += sizeof(WELD);
	weld->weld_q1 =		q1;
	weld->weld_q2 =		q2;
	weld->weld_q3 =		q3;
	weld->weld_q4 =		q4;
	weld->weld_func =	func;
	weld->weld_arg  =	arg;
	weld->weld_queue =	protect_q;
	weld->weld_op =		op;

	/*
	 * queue it into the str_weld run queue.
	 */
	simple_lock(&weldq_lock);
	weld_msg->b_cont = nil(MBLKP);
	insque(weld_msg, weldq_runq.prev);
	simple_unlock(&weldq_lock);
	schednetisr(NETISR_STRWELD);
	return 0;
}

int
weldq_init()
{

	simple_lock_init(&weldq_lock);
	sq_init(&weldq_sq);
	weldq_runq.next = weldq_runq.prev = (MBLKP)&weldq_runq;
#if	!NETISR_THREAD
	/* Cannot take blocking locks in softnets... */
	return ENXIO;
#else
	return netisr_add(NETISR_STRWELD, weldq_main,
			(struct ifqueue *)0, (struct domain *)0);
#endif
}

/*
 * weldq_main - the streams weld routine
 *
 * Started at system initialization time.
 * Waits for weld jobs to be entered into its run queue.
 * Is involved into running the jobs:
 *	- handles the mult_sqh synchronization.
 *	- disassembles the weld job
 *	- uses two subroutines to do the actual work
 *	- handles the callback function
 */

staticf void
weldq_main ()
{
	MBLKP	weld_msg;
	WELDP	weld;

	simple_lock(&weldq_lock);
	while ( (weld_msg = weldq_runq.next) != (MBLKP)&weldq_runq ) {
		remque(weld_msg);
		simple_unlock(&weldq_lock);
		/*
		 * execute the job
		 * under protection of the mult_sqh
		 */
		csq_acquire(&mult_sqh, &weldq_sq);

		weld = (WELDP)weld_msg->b_rptr;
		switch (weld->weld_op ) {
		case 0:
			unweldq_exec(weld->weld_q1, weld->weld_q2, &weldq_sq);
			unweldq_exec(weld->weld_q3, weld->weld_q4, &weldq_sq);
			break;
		case 1:
			weldq_exec(weld->weld_q1, weld->weld_q2, &weldq_sq);
			weldq_exec(weld->weld_q3, weld->weld_q4, &weldq_sq);
			break;
		default:
STR_DEBUG(printf("STREAMS: weldq_main: found illegal job on run queue.\n"));
			weld = 0;
			break;
		}

		/*
		 * We release the mult_sqh before calling the callback
		 * function. Whenever the callback function should need
		 * the protection of more queues (perhaps of all the
		 * queues which were involved in the welding), we
		 * have to change program structure here.
		 */
		csq_release(&mult_sqh);

		if ( weld && weld->weld_func ) {
			if ( weld->weld_queue )
				csq_acquire(
					&weld->weld_queue->q_sqh,
					&weldq_sq);
			(*weld->weld_func)( weld->weld_arg );
			if ( weld->weld_queue )
				csq_release(&weld->weld_queue->q_sqh);
		}

		/*
		 * release the job structure
		 */
		freemsg(weld_msg);
		simple_lock(&weldq_lock);
	}
	simple_unlock(&weldq_lock);
}

/*
 * weldq_exec - subroutine to do the welding work
 *
 * Parameters:
 *	q1, q2 - queues which should get welded.
 *	sq - synch queue to use for acquires
 *
 * This routine might get called with zero queues (easier coding on the
 * caller's level). If, however, the first queue exists, the second queue
 * exists also (this was checked on the entry level to the module).
 * Furthermore, the first queue does not have a q_next entry yet.
 *
 * Functionality:
 *	q1->q_next = q2;
 *
 * Return value: none.
 */

void
weldq_exec (q1, q2, sq)
	queue_t *	q1;
	queue_t *	q2;
	SQP		sq;
{
	queue_t *	q;

#if	MACH_ASSERT
	if (mult_sqh.sqh_owner != sq)
		panic("weldq_exec");
#endif
	if ( !q1 )
		return;
	csq_acquire(&q1->q_sqh, sq);
	csq_acquire(&q2->q_sqh, sq);

	/*
	 * The big Weld!
	 */
	q1->q_next = q2;
	q1->q_flag |= QWELDED;

	/*
	 * Set the rear queue's forward flow control pointer and the
	 * front queue's backward flow control pointer.
	 */
	q1->q_ffcp = q2->q_qinfo->qi_srvp ? q2 : q2->q_ffcp ? q2->q_ffcp : q2;
	q2->q_bfcp = q1->q_qinfo->qi_srvp ? q1 : q1->q_bfcp;

	/*
	 * If the rear queue does not have a service routine,
	 * then walk forward from its backward flow control
	 * pointer resetting forward flow control pointers.
	 */
	if ( !q1->q_qinfo->qi_srvp ) {
		for (q = q1->q_bfcp; q && q != q1; q = q->q_next) {
			csq_acquire(&q->q_sqh, sq);
			q->q_ffcp = q1->q_ffcp;
			csq_release(&q->q_sqh);
		}
	}
	/*
	 * Now, if the front queue does not have a service routine,
	 * then walk forward to its forward flow control
	 * pointer resetting backward flow control pointers.
	 */
	if ( !q2->q_qinfo->qi_srvp ) {
		q = q2;
		while (q = q->q_next) {
			csq_acquire(&q->q_sqh, sq);
			q->q_bfcp = q2->q_bfcp;
			csq_release(&q->q_sqh);
			if (q == q2->q_ffcp)
			    break;
		}
	}
	csq_release(&q2->q_sqh);
	csq_release(&q1->q_sqh);
}
	
/*
 * unweldq_exec - do the unweld work
 *
 * Parameters:
 *	q1, q2 - queues which ARE welded, and should be unwelded.
 *	sq - synch queue to use for acquires
 *
 * The stream head close routine calls here, as well.
 *
 * Functionality:
 *	q1->q_next = nil;
 *
 * Return value: none.
 */

void
unweldq_exec (q1, q2, sq)
	queue_t *	q1;
	queue_t *	q2;
	SQP		sq;
{
	queue_t *	q;

#if	MACH_ASSERT
	if (mult_sqh.sqh_owner != sq)
		panic("unweldq_exec");
#endif
	/*
	 * Since we are holding the mult_sqh, it is OK to
	 * look at QWELDED - it will change only under
	 * mult_sqh control.
	 */
	if ( !q1 || !(q1->q_flag & QWELDED) )
		return;
	csq_acquire(&q1->q_sqh, sq);
	csq_acquire(&q2->q_sqh, sq);

	/* Unweld time has come... */
	q1->q_next = nil(queue_t *);
	q1->q_flag &= ~QWELDED;

	/*
	 * Reset the rear queue's forward flow control pointer and the
	 * front queue's backward flow control pointer.
	 */
	q1->q_ffcp = nilp(queue_t);
	q2->q_bfcp = nilp(queue_t);

	/*
	 * If the rear queue does not have a service routine, then
	 * walk forward from its backward flow control
	 * pointer resetting forward flow control pointers.
	 */
	if ( !q1->q_qinfo->qi_srvp ) {
		for ( q = q1->q_bfcp; q && q != q1; q = q->q_next ) {
			csq_acquire(&q->q_sqh, sq);
			q->q_ffcp = q1;
			csq_release(&q->q_sqh);
		};
	}
	/*
	 * Now, if the front queue does not have a service routine,
	 * then walk forward to its forward flow control
	 * pointer resetting backward flow control pointers.
	 */
	if ( !q2->q_qinfo->qi_srvp  &&  q2->q_next) {
		q = q2;
		while (q = q->q_next) {
			csq_acquire(&q->q_sqh, sq);
			q->q_bfcp = q2;
			csq_release(&q->q_sqh);
			if (q == q2->q_ffcp)
			    break;
		}
	}
	csq_release(&q2->q_sqh);
	csq_release(&q1->q_sqh);
}
