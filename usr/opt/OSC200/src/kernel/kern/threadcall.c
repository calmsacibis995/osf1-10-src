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
static char	*sccsid = "@(#)$RCSfile: threadcall.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 18:00:12 $";
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
 * threadcall.c: Routines to manage callout threads.
 *
 * The callout threads are used to perform operations that need
 * to be initiated in response to I/O completions, but cannot be
 * performed directly from interrupt context. Also used to limit
 * stack growth caused by explicit or implicit recursion.
 *
 *	Revision History:
 *
 * 03-May-91	Peter H. Smith
 *	Replace a hardcoded priority with a constant.
 */
#include <sys/types.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>

#include <kern/threadcall.h>
#include <kern/sched.h>

/*
 * Function:
 * boolean_t thread_call(thread_callq_t *, void (*)(void *), void *)
 *
 * Arguments:
 *	tcq	pointer to thread_callq structure [initialized by
 *		thread_call_create()], indicating the class of thread
 *		this request should be routed to.
 *	func	pointer to function to be invoked.
 *	arg	argument to called function.
 *
 * Invoke a function in thread context.
 *
 * Return value:
 *	TRUE	request successfully queued.
 *	FALSE	insufficient memory to queue request.
 */
boolean_t
thread_call(tcq, func, arg)
register thread_callq_t *tcq;
register void (*func)();
register void *arg;
{
	register struct thread_call *tc;
	register int s = splhigh();
	/*
	 * Since the whole purpose is to make interrupt things
	 * turn into thread things, we must splhigh() around
	 * the zget/zfree calls.
	 */

	if ((tc = (thread_call_t *)zget(tcq->tcq_zone)) != NULL) {
		tc->tc_next = NULL;
		tc->tc_func = func;
		tc->tc_arg = arg;
		
		simple_lock(&tcq->tcq_lock);

		if (tcq->tcq_head == NULL)  {
			tcq->tcq_head = tc;
			/* thread_wakeup_one is not sufficient here */
			thread_wakeup((vm_offset_t)tcq);
		} else {
			tcq->tcq_tail->tc_next = tc;
		}
		tcq->tcq_tail = tc;

		simple_unlock(&tcq->tcq_lock);
	}
	splx(s);
	return((tc!=NULL)?TRUE:FALSE);
}

/*
 * Function:
 * boolean_t thread_call_one(thread_callq_t *, void (*)(void *), void *)
 *
 * Ensures that at least one occurrence of the specified func/arg pair
 * appears on the given thread call queue. If the specified entry already
 * exists, the function simply returns success (TRUE). If the specified
 * entry does not exist, a new call is allocated and added to the list.
 */
boolean_t
thread_call_one(tcq, func, arg)
register thread_callq_t *tcq;
register void (*func)();
register void *arg;
{
	register struct thread_call *tc;
	register int s = splhigh();

	simple_lock(&tcq->tcq_lock);
	/*
	 * search for matching request on the existing thread_call list
	 */
	for (tc = tcq->tcq_head; tc; tc = tc->tc_next) {
		if ((tc->tc_func == func) && (tc->tc_arg == arg)) {
			simple_unlock(&tcq->tcq_lock);
			splx(s);
			return(TRUE);
		}
	}
	/*
	 * No such request found, allocate and add to the list.
	 */
	if ((tc = (struct thread_call *)zget(tcq->tcq_zone)) != NULL) {
		tc->tc_next = NULL;
		tc->tc_func = func;
		tc->tc_arg = arg;
		
		if (tcq->tcq_head == NULL)  {
			tcq->tcq_head = tc;
			/* thread_wakeup_one is not sufficient here */
			thread_wakeup((vm_offset_t)tcq);
		} else {
			tcq->tcq_tail->tc_next = tc;
		}
		tcq->tcq_tail = tc;
	}
	simple_unlock(&tcq->tcq_lock);
	splx(s);
	return((tc!=NULL)?TRUE:FALSE);
}

/*
 * Function:
 * void thread_call_create(thread_callq_t *, int)
 *
 * Create a new class of callout threads.
 * It is the caller's responsibility to populate the thread_callq zone
 * with sufficient thread_call structures by calling thread_call_alloc().
 */
void
thread_call_create(tcq, ncallthreads)
thread_callq_t *tcq;
int ncallthreads;
{
	zone_t zone;

	zone = zinit(sizeof(thread_call_t), 0, 0, "thread_call zone");
	zchange(zone, FALSE, FALSE, FALSE, FALSE);

	simple_lock_init(&tcq->tcq_lock);
	tcq->tcq_head = NULL;
	tcq->tcq_tail = NULL;
	tcq->tcq_zone = zone;
	tcq->tcq_zone_size = 0;
	tcq->tcq_threadcall_size = 0;
	
	thread_call_add(tcq, ncallthreads);
}

/*
 * Function:
 * void thread_call_add(thread_callq_t *, int)
 *
 * Add new threads to a thread_call class. This is useful on a multiprocessor
 * to allow multiple parallel execution of a class of thread_calls. Note
 * that this depends on the degree of lock contention (if any) for the
 * work that these threads will be carrying out.
 */
void
thread_call_add(tcq, ncallthreads)
thread_callq_t *tcq;
int ncallthreads;
{
	extern task_t first_task;
	thread_t thread;
	extern void call_thread();

	while (ncallthreads--) {
		thread = kernel_thread_w_arg(first_task, call_thread, tcq);
	}
}

/*
 * Function:
 * void thread_call_alloc(thread_callq_t *, int)
 *
 * Allocate space for a certain number of thread_calls. Users of
 * thread_calls are responsible for allocating an appropriate number
 * of thread_calls for the maximum number of thread_calls that will
 * be used simultaneously. It is an error to use thread_call without
 * allocating appropriate space, or being prepared for thread_call to
 * fail.
 *
 * nthreadcalls may be negative to deallocate space. It is an error
 * to deallocate more space than was allocated.
 */
void
thread_call_alloc(tcq, nthreadcalls)
thread_callq_t *tcq;
int nthreadcalls;
{
	zone_t zone;
	vm_offset_t addr;
	int size;
	int s;

	/*
	 * Compute the number of new 'thread_call' structs we actually
	 * have to allocate. Might be negative, if we already have more
	 * in the zone than we currently need.
	 */
	simple_lock(&tcq->tcq_lock);

	tcq->tcq_threadcall_size += nthreadcalls;

	ASSERT (tcq->tcq_threadcall_size >= 0);

	/* Compute new number of thread_call_t's to allocate */
	nthreadcalls = (tcq->tcq_threadcall_size - tcq->tcq_zone_size);

	if (nthreadcalls > 0) {
		size = round_page(sizeof(thread_call_t) * nthreadcalls);
		zone = tcq->tcq_zone;
		addr = kmem_alloc(kernel_map, size);
		tcq->tcq_zone_size += size/sizeof(thread_call_t);

		simple_unlock(&tcq->tcq_lock);

		s = splhigh();
		zcram(zone, addr, size);
		splx(s);
	} else {
		simple_unlock(&tcq->tcq_lock);
	}
}

/*
 * Callout thread: general-purpose thread to execute functions on
 * behalf of other code; for example, if an interrupt needs to perform
 * operations that can only be performed in thread context, it can
 * send a call to this (these) threads to perform the work.
 */
void
call_thread()
{
	register thread_t thread;
	register thread_callq_t *tcq;

	thread = current_thread();
/*
 * RT_SCHED:  Use a constant instead of a hardcoded priority.  The constant
 * is defined in kern/sched.h.  Note that it is alright to stomp on sched_pri
 * here, because the thread is the current thread and therefore not on a
 * run queue.  If it was on a run queue the run queue bitmask would be
 * currupted.
 */
	thread->priority = thread->sched_pri = BASEPRI_HIGHEST;
	thread_swappable(thread, FALSE);

	/* Collect argument left by kernel_thread_w_arg() */
	tcq = (thread_callq_t *)thread->reply_port;
	thread->reply_port = PORT_NULL;

	for(;;) {
		register struct thread_call *tc;
		void (*func)();
		void *arg;
		int s = splhigh();

		simple_lock(&tcq->tcq_lock);

		while ((tc = tcq->tcq_head) == NULL) {
			assert_wait((vm_offset_t)tcq, FALSE);
			simple_unlock(&tcq->tcq_lock);
			splx(s);
			thread_block();
			s = splhigh();
			simple_lock(&tcq->tcq_lock);
		}
		tcq->tcq_head = tc->tc_next; tc->tc_next = NULL;

		simple_unlock(&tcq->tcq_lock);

		func = tc->tc_func; tc->tc_func = NULL;
		arg = tc->tc_arg; tc->tc_arg = NULL;

		zfree(tcq->tcq_zone, tc);

		splx(s);

		(*func)(arg);
	}
}
