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
static char	*sccsid = "@(#)$RCSfile: cond.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/06/16 16:15:43 $";
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

#ifndef	lint
#endif	not lint

/*
 * File: cond.c
 *
 * Functions supporting condition variables and their attributes. In fact
 * there are no attributes currently defined for condition variables.
 * Condition variables block the vp that the thread is running on by waiting
 * for an event.
 */

#include <pthread.h>
#include "internal.h"
#include <errno.h>

/*
 * Global Variables
 */
pthread_condattr_t	pthread_condattr_default;

/*
 * Function:
 *	pthread_condattr_startup
 *
 * Description:
 * 	Initialize condition attributes. This function creates the default
 *	attribute structure.
 */
private void
pthread_condattr_startup()
{
	pthread_condattr_create(&pthread_condattr_default);
	/* No attributes to initialize */
}

/*
 * Function:
 *	pthread_condattr_create
 *
 * Parameters:
 *	attr - pointer to the newly created attribute structure
 *
 * Return value:
 *	0	Success
 *	-1	if the pointer passed is and invalid pointer (EINVAL)
 *
 * Description:
 *	The pthread_condattr_t is the attribute structure and this function
 *	marks it as being initialized. There is no real initialization to be
 *	done as there are no attributes.
 */
int
pthread_condattr_create(pthread_condattr_t *attr)
{
	if (attr == NULL) {
		set_errno(EINVAL);
		return(-1);
	}
	*attr = (pthread_condattr_t)malloc(sizeof(struct pthread_condattr));
	if (*attr == NO_COND_ATTRIBUTE) {
		set_errno(ENOMEM);
		return(-1);
	}
	(*attr)->flags = CONDATTR_VALID;
	return(0);
}

/*
 * Function:
 *	pthread_condattr_delete
 *
 * Parameters:
 *	attr - pointer to the attribute structure to be deleted
 *
 * Return value:
 *	0	Success
 *	-1	if the pointer passed is and invalid pointer (EINVAL)
 *		if the structure had not be previously initialized (EINVAL)
 *
 * Description:
 *	The attribute structure is simply marked as being no longer
 *	valid after the appropriate checks have been made.
 */
int
pthread_condattr_delete(pthread_condattr_t *attr)
{
	if ((attr == NULL) || (*attr == NO_COND_ATTRIBUTE) ||
	    !((*attr)->flags&CONDATTR_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}
	(*attr)->flags &= ~CONDATTR_VALID;
	free(*attr);
	*attr = NO_COND_ATTRIBUTE;
	return(0);
}

/*
 * Function:
 *	pthread_cond_startup
 *
 * Description:
 *	Intialize all of the condition variable functions. In fact there
 *	is no initialization to do other than for condition attributes.
 */
void
pthread_cond_startup()
{
	pthread_condattr_startup();
}

/*
 * Function:
 *	initialize_condition
 *
 * Parameters:
 *	cond - the condition variable to be initialized
 *	attr - attributes to indicate how it should be initialized
 *
 * Description:
 *	Initialize a condition variable. Make the the queue of waiting
 *	pthreads empty, unlock the spin lock protecting the structure
 *	and make the condition valid.
 */
void
initialize_condition(pthread_cond_t *cond, pthread_condattr_t attr)
{
	pthread_queue_init(&cond->waiters);
	cond->lock = SPIN_LOCK_UNLOCKED;
	cond->name = NULL;
	cond->flags = COND_VALID;
}

/*
 * Function:
 *	pthread_cond_init
 *
 * Parameters:
 *	cond - the condition variable to be created
 *	attr - attributes to indicate how it should be created
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the condition variable was invalid (EINVAL)
 *		The condition attribute was invalid (EINVAL)
 *
 * Description:
 *	The initialize_condition function is used to do all the real
 *	work of creation once the parameters have been checked.
 */
int
pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t attr)
{
	PTHREAD_LOG("pthread_cond_create", NULL);

	if ((cond == NO_COND) || (attr == NO_COND_ATTRIBUTE) ||
	    !(attr->flags&CONDATTR_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	initialize_condition(cond, attr);

	return(0);
}

/*
 * Function:
 *	pthread_cond_destroy
 *
 * Parameters:
 *	cond - the condition variable to be deleted
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the condition variable was invalid (EINVAL)
 *		The condition variable was not valid (EINVAL)
 *		At least one thread is waiting on the condition variable (EBUSY)
 *
 * Description:
 *	Destroy a condition variable if there are no waiters. It is
 *	destroyed by marking it invalid.
 */
int
pthread_cond_destroy(pthread_cond_t *cond)
{
	PTHREAD_LOG("pthread_cond_delete: cond = %x", cond);

	if ((cond == NO_COND) || !(cond->flags&COND_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}
	/*
	 * lock the condition structure before we touch it
	 */
	spin_lock(&cond->lock);

	/*
	 * Check to see if anyone is one the queue waiting to be signalled
	 * If so we return an error.
	 */
	if (!pthread_queue_empty(&cond->waiters)) {
		spin_unlock(&cond->lock);
		set_errno(EBUSY);
		return(-1);
	}
	/*
	 * No one is waiting. Make the condition structure invalid
	 * so future calls with this handle fail and then unlock it.
	 */
	cond->flags &= ~COND_VALID;
	spin_unlock(&cond->lock);
	return(0);
}

/*
 * Function:
 *	pthread_cond_wait
 *
 * Parameters:
 *	cond - the condition variable being waited on
 *	mutex - the locked mutex associated with this condition.
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the condition variable is invalid (EINVAL)
 *		The condition variable is invalid (EINVAL)
 *		The pointer to the mutex is invalid (EINVAL)
 *		The mutex is invalid (EINVAL)
 *		The mutex is not locked by the caller (EDEADLK)
 *
 * Description:
 *	Once the checks have been completed, locks are taken on both
 *	the condition variable structure and the thread. The thread is
 *	taken off the thread queues (pthread_deactivate) and queued onto
 *	condition variable with the state changed to indicate the wait.
 *	The thread is then blocked waiting for an event. This will be either
 *	a signal event, meaning that the condition has either been signalled
 *	or broadcast, or a cancellation event. If it was signalled the thread
 *	has already been reactivated and so the mutex is reaquired and return.
 *	If the event was a cancel we must remove the thread from the waiting
 *	queue, put the thread back on the active thread queues
 *	(pthread_activate) and the cancellation point is created with
 *	pthread_testcancel().
 */
int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	pthread_t	thread;
	int		event;

	PTHREAD_LOG("pthread_cond_wait: cond = %x", cond);

	if ((cond == NO_COND) || !(cond->flags&COND_VALID) ||
	    (mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	thread = pthread_self();

#ifdef MUTEX_OWNER
	if (pthread_mutex_getowner_np(mutex) != thread) {
		set_errno(EDEADLK);
		return(-1);
	}
#endif

	/*
	 * Lock the condition variable and the thread.
	 * move the thread onto the condition waiters
	 * queue. We set the state to CONDWAIT so that
	 * a cancellation event is sent if the thread
	 * is cancelled.
	 */
	spin_lock(&cond->lock);
	pthread_mutex_lock(&thread->lock);

	pthread_deactivate(thread);
	pthread_queue_enq(&cond->waiters, &thread->link);
	thread->state |= PTHREAD_CONDWAIT;

	/*
	 * Unlock everything (in reverse order) before we
	 * wait for the event.
	 */
	pthread_mutex_unlock(&thread->lock);
	spin_unlock(&cond->lock);
	pthread_mutex_unlock(mutex);

	vp_event_wait(thread->vp, &event, NO_TIMEOUT);

	/*
	 * An event has arrived. Lock the condition and the thread
	 * before we look to see what it was. We also set the threads
	 * state back to be not waiting.
	 */
	spin_lock(&cond->lock);
	pthread_mutex_lock(&thread->lock);
	thread->state &= ~PTHREAD_CONDWAIT;
	if (event == EVT_CANCEL) {
		/*
		 * The thread was cancelled. Take it off the waiting queue
		 * and put it back on the thread queue. Everything is then
		 * unlocked and the cancellation point opened. We should
		 * not return from this.
		 */
		pthread_queue_remove(&cond->waiters, &thread->link);
		pthread_activate(thread);
		pthread_mutex_unlock(&thread->lock);
		spin_unlock(&cond->lock);
		/*
		 * try and lock the mutex for the cleanup handler. Ignore
		 * whether this worked as there is nothing we can do if
		 * it didn't.
		 */
		pthread_mutex_lock(mutex);
		pthread_testcancel();
		pthread_internal_error("condwait cancel error");
		/* NOTREACHED */
	}
#ifdef DEBUG
	else if (event != EVT_SIGNAL)
		pthread_internal_error("pthread_cond_wait");
#endif
	/*
	 * The thread was signalled. The thread is already back on
	 * the active list so the thread and condition are unlocked
	 * and the mutex is re locked for the caller. The only way this
	 * could fail is if the mutex had been deleted while the thread
	 * was waiting.
	 */
	pthread_mutex_unlock(&thread->lock);
	spin_unlock(&cond->lock);
	if (pthread_mutex_lock(mutex) == -1)
		return(-1);
	return(0);
}

/*
 * Function:
 *	pthread_cond_timedwait
 *
 * Parameters:
 *	cond - the condition variable being waited on
 *	mutex - the locked mutex associated with this condition.
 *	timeout - the maximum time to wait for the condition to be signalled
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the condition variable is invalid (EINVAL)
 *		The condition variable is invalid (EINVAL)
 *		The pointer to the mutex is invalid (EINVAL)
 *		The mutex is invalid (EINVAL)
 *		The mutex is not locked by the caller (EDEADLK)
 *		The timeout occurred (EAGAIN)
 *
 * Description:
 *	This function works in the same way as pthread_cond_wait with
 *	the exception of the added complexity of the timeout. The timeout
 *	is delivered as a timeout event. If the timeout happened then the
 *	thread is put back on the active queue (pthread_activate) and the
 *	error is returned (although a timeout is not really an error).
 */
int
pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
			struct timespec *timeout)
{
	pthread_t	thread;
	int		event;

	PTHREAD_LOG("pthread_cond_timedwait: cond = %x", cond);

	if ((cond == NO_COND) || !(cond->flags&COND_VALID) ||
	    (mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID) ||
	    (timeout == NO_TIMEOUT) || (timeout->tv_nsec < 0)) {
		set_errno(EINVAL);
		return(-1);
	}

	thread = pthread_self();

#ifdef MUTEX_OWNER
	if (pthread_mutex_getowner_np(mutex) != thread) {
		set_errno(EDEADLK);
		return(-1);
	}
#endif

	/*
	 * Lock the condition variable and the thread.
	 * move the thread onto the condition waiters
	 * queue. We set the state to CONDWAIT so that
	 * a cancellation event is sent if the thread
	 * is cancelled.
	 */
	spin_lock(&cond->lock);
	pthread_mutex_lock(&thread->lock);

	pthread_deactivate(thread);
	pthread_queue_enq(&cond->waiters, &thread->link);
	thread->state |= PTHREAD_CONDWAIT;

	/*
	 * Unlock everything (in reverse order) before we
	 * wait for the event.
	 */
	pthread_mutex_unlock(&thread->lock);
	spin_unlock(&cond->lock);
	pthread_mutex_unlock(mutex);

	vp_event_wait(thread->vp, &event, timeout);

	/*
	 * An event has arrived. Lock the condition and the thread
	 * before we look to see what it was. We also set the threads
	 * state back to be not waiting.
	 */
	spin_lock(&cond->lock);
	pthread_mutex_lock(&thread->lock);
	thread->state &= ~PTHREAD_CONDWAIT;

	if ((event == EVT_CANCEL) || (event == EVT_TIMEOUT)) {
		/*
		 * If it was a cancellation or a timeout, the thread
		 * is still on the waiters queue so it must be put
		 * back on the active queue
		 */
		pthread_queue_remove(&cond->waiters, &thread->link);
		pthread_activate(thread);
		pthread_mutex_unlock(&thread->lock);
		spin_unlock(&cond->lock);
		/*
		 * There is a race that a cancel may have been sent after
		 * the timeout but before we turned off the CONDWAIT state
		 * (which determines whether the cancel event will be sent
		 * or not. If the pending_cancel is set then we have to
		 * flush the extra event that is queued. Even if the timeout
		 * did occur this is now a cancel.
		 */
		if (thread->pending_cancel) {
			/*
			 * Throw away the cancel event
			 */
			if (!(event == EVT_CANCEL))
				vp_event_flush(thread->vp);
			/*
			 * try and lock the mutex for the cleanup handler.
			 * Ignore whether this worked as there is nothing
			 * we can do if it didn't.
			 */
			pthread_mutex_lock(mutex);
			pthread_testcancel();
			pthread_internal_error("condtimedwait cancel error");
		} else {
			/*
			 * return after a timeout. The mutex must be locked
			 * and we return EAGAIN with -1.
			 */
			pthread_mutex_lock(mutex);
			set_errno(EAGAIN);
			return(-1);
		}
	}
#ifdef DEBUG
	else if (event != EVT_SIGNAL)
		pthread_internal_error("pthread_cond_wait");
#endif
	/*
	 * The thread was signalled. The thread is already back on
	 * the active list so the thread and condition are unlocked
	 * and the mutex is re locked for the caller. The only way this
	 * could fail is if the mutex had been deleted while the thread
	 * was waiting.
	 */
	pthread_mutex_unlock(&thread->lock);
	spin_unlock(&cond->lock);
	if (pthread_mutex_lock(mutex) == -1)
		return(-1);
	return(0);
}

/*
 * Function:
 *	pthread_cond_signal
 *
 * Parameters:
 *	cond - the condition variable that is to be signalled.
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the condition variable is invalid (EINVAL)
 *		The condition variable is invalid (EINVAL)
 *
 * Description:
 *	Scan through the waiter list of threads looking for one that
 *	has not been cancelled. The first one found is removed from the
 *	list, put on the active thread list and sent the signal event.
 *
 */
int
pthread_cond_signal(pthread_cond_t *cond)
{
	pthread_t	thread;

	PTHREAD_LOG("pthread_cond_signal: cond = %x", cond);

	if ((cond == NO_COND) || !(cond->flags&COND_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * lock the condition as we are about to change the waiting queue
	 */
	spin_lock(&cond->lock);
	/*
	 * Step through every thread in the queue
	 */
	for (thread = (pthread_t)pthread_queue_head(&cond->waiters);
	     thread != (pthread_t)pthread_queue_end(&cond->waiters);
	     thread = (pthread_t)pthread_queue_next(&thread->link)) {
		/*
		 * lock the thread before we look at it. Check to see
		 * if there is a cancel pending. If not then we have a good
		 * thread. Put the thread back on the active list and send
		 * the signal event. The thread can then be unlocked and we
		 * can return.
		 */
		pthread_mutex_lock(&thread->lock);
		if (!thread->pending_cancel) {
			pthread_queue_remove(&cond->waiters, &thread->link);
#ifdef DEBUG
			if (thread->vp == NO_VP)
				pthread_internal_error("pthread_cond_signal");
#endif
			pthread_activate(thread);
			vp_event_notify(thread->vp, EVT_SIGNAL);
			pthread_mutex_unlock(&thread->lock);
			break;
		}
		pthread_mutex_unlock(&thread->lock);
	}
	spin_unlock(&cond->lock);
	return(0);
}

/*
 * Function:
 *	pthread_cond_broadcast
 *
 * Parameters:
 *	cond - the condition variable that is to be broadcast.
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the condition variable is invalid (EINVAL)
 *		The condition variable is invalid (EINVAL)
 *
 * Description:
 *	Step through the list of waiting threads and send a signal
 *	event to every thread that has not already been cancelled.
 */
int
pthread_cond_broadcast(pthread_cond_t *cond)
{
	pthread_t	thread;
	pthread_t	next_thread;
	pthread_queue	queue;

	PTHREAD_LOG("pthread_cond_broadcast: cond = %x", cond);

	if ((cond == NO_COND) || !(cond->flags&COND_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * lock the condition as we are about to change the waiting queue
	 */
	spin_lock(&cond->lock);
	/*
	 * Step through every thread in the queue
	 */
	for (thread = (pthread_t)pthread_queue_head(&cond->waiters);
	     thread != (pthread_t)pthread_queue_end(&cond->waiters);
	     thread = next_thread) {
		/*
		 * Lock the thread and take a note of its next pointer
		 * in case we remove it from the list and put it on the
		 * active list. If the thread has not been cancelled then
		 * we send it the signal event and put it on the active list
		 */
		pthread_mutex_lock(&thread->lock);
		next_thread = (pthread_t)pthread_queue_next(&thread->link);
		if (!thread->pending_cancel) {
			pthread_queue_remove(&cond->waiters, &thread->link);
#ifdef DEBUG
			if (thread->vp == NO_VP)
				pthread_internal_error("pthread_cond_signal");
#endif
			pthread_activate(thread);
			vp_event_notify(thread->vp, EVT_SIGNAL);
		}
		pthread_mutex_unlock(&thread->lock);
	}
	spin_unlock(&cond->lock);
	return(0);
}

#ifdef DEBUG
/*
 * Function:
 *	pthread_cond_dump
 *
 * Parameters:
 *	cond - thre condition variable to be dumped
 *
 * Description:
 *	This is a debugging function which prints out the state of a condition
 *	variable and the threads waiting on it.
 */
void
pthread_cond_dump(cond)
pthread_cond_t	*cond;
{
	pthread_queue	*q;

	if ((cond == NO_COND) || !(cond->flags&COND_VALID)) {
		printf("NO CONDITION\n");
		return;
	}
	printf("CND: %s %x %s Wait:", pthread_cond_getname_np(cond),
		cond, cond->lock == SPIN_LOCK_LOCKED ? "Lck" : "Unlck");

	for (q = pthread_queue_head(&cond->waiters);
	     q != pthread_queue_end(&cond->waiters);
	     q = pthread_queue_next(q))
		printf(" %x", q);
}
#endif
