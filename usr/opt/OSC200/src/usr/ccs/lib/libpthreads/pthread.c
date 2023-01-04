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
static char	*sccsid = "@(#)$RCSfile: pthread.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/08/11 14:24:55 $";
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

#ifndef	lint
#endif	not lint

/*
 * File: pthread.c
 *
 * This file contains all the functions to manipulate the pthreads themselves.
 * This include creation, deletion, caching, cancellation and fork.
 *
 * Pthreads map one to one with the underlying kernel threads (vp's). The
 * pthreads and the vp are bound for the life of the pthread (until the
 * thread returns or calls pthread_exit.
 */

#include <pthread.h>
#include <errno.h>
#include "internal.h"
#include <sched.h>

/*
 * Local Variables
 */
private	volatile int	pthread_lock = SPIN_LOCK_UNLOCKED;
private	unsigned	n_freepthreads;
private	pthread_queue	free_pthreads;
private	unsigned	n_activepthreads;
private	pthread_queue	active_pthreads;
private	unsigned	n_runningpthreads;
private	pthread_t	all_pthreads;

/*
 * Global Variables
 */
pthread_key_t	_pthread_cleanup_handlerqueue;
#ifdef DEBUG
#ifdef TRACE
int	pthread_trace = TRUE;
#else
int	pthread_trace = FALSE;
#endif
#endif

/*
 * Function:
 *	pthread_alloc
 *
 * Return value:
 *	NO_PTHREAD	if a pthread structure can't be allocated (errno is set)
 *	pointer to a pthread structure otherwise
 *
 * Description:
 *	Try to allocate a pthread structure. First look to see if a cached
 *	structure is available, if it is, reinitialize it and return. If not
 *	The allocate a new structure, with its mutex and condition variables,
 *	and then initialise that.
 *
 */
private pthread_t
pthread_alloc()
{
	static struct pthread	null_thread = { 0 };
	pthread_t		thread;

	thread = NO_PTHREAD;

	/*
	 * Only look for a cached thread is there is a chance that there might
	 * be one. This is a minor optimization for the case that there are
	 * no cached structures.
	 */
	if (n_freepthreads != 0) {
		/*
		 * n_freepthreads was non zero at one point but because we
		 * didn't have the lock we now have to check under the lock.
		 */
		spin_lock(&pthread_lock);

		if (n_freepthreads != 0) {
			/*
			 * there is still at least one cached thread structure
			 * remove it from the free queue and drop the lock
			 * for the next allocation.
			 */
			thread = (pthread_t)pthread_queue_head(&free_pthreads);
			pthread_queue_remove(&free_pthreads, &thread->link);
			n_freepthreads--;
			spin_unlock(&pthread_lock);

			/*
			 * we now have our used structure with its mutex and
			 * condition variable. Re-initialize everything to look
			 * like a new structure
			 */
			initialize_mutex(&thread->lock, pthread_mutexattr_default);
			initialize_condition(&thread->done, pthread_condattr_default);

			thread->state = 0;
			thread->flags = 0;
			thread->join_count = 0;
		} else {
			/*
			 * There are no free threads when we looked a second
			 * time so we free the lock and carry on to allocate
			 * a structure from scratch.
			 */
			spin_unlock(&pthread_lock);
		}
	}

	/*
	 * check we haven't got a thread structure yet. If not we have to
	 * make one from scratch
	 */
	if (thread == NO_PTHREAD) {
		/*
		 * Allocate and initialise the structure itself. If this
		 * fails than the caller is informed.
		 */
		thread = (pthread_t)malloc(sizeof(struct pthread));
		*thread = null_thread;
		if (thread == NO_PTHREAD) {
			set_errno(ENOMEM);
			return(NO_PTHREAD);
		}

		/*
		 * Now allocate the mutex to protect updates to the
		 * thread structure. If this fails, the thread structure
		 * is freed and the caller is informed.
		 */
		if (pthread_mutex_init(&thread->lock, pthread_mutexattr_default) != 0) {
			free(thread);
			return(NO_PTHREAD);
		}

		/*
		 * Now allocate the condition variable used for join.
		 * If this fails, the thread structure is freed, the 
		 * mutex deleted and the caller is informed.
		 */
		if (pthread_cond_init(&thread->done, pthread_condattr_default) != 0) {
			pthread_mutex_destroy(&thread->lock);
			free(thread);
			return(NO_PTHREAD);
		}
	}

	/*
	 * Success, the allocated structure is returned. The structure
	 * is not linked onto any queue at this point.
	 */
	return(thread);
}

/*
 * Function:
 *	pthread_dealloc
 *
 * Description:
 *	Performs the inverse function to pthread_alloc. The structure is not
 *	cached and the mutex and condition variable are deleted. This function
 *	assumes that the thread in not on any queue.
 *
 */
private void
pthread_dealloc(pthread_t thread)
{
	pthread_mutex_destroy(&thread->lock);
	pthread_cond_destroy(&thread->done);
	free(thread);
}

/*
 * Function:
 *	pthread_free
 *
 * Parameters:
 *	thread	- the thread structure to free
 *
 * Description:
 *	Free a thread previously allocated using pthread_alloc. Unlike
 *	pthread_dealloc, the thread structure is cached and it is
 *	assumed that the structure is on the active list.
 *
 */
private void
pthread_free(pthread_t thread)
{
	/*
	 * Lock the active and free queues.
	 */
	spin_lock(&pthread_lock);
	/*
	 * Take off the active list
	 */
	n_activepthreads--;
	pthread_queue_remove(&active_pthreads, &thread->link);
	/*
	 * put on the free queue
	 */
	pthread_queue_enq(&free_pthreads, &thread->link);
	n_freepthreads++;
	/*
	 * Drop the queue lock
	 */
	spin_unlock(&pthread_lock);
}

/*
 * Function:
 *	pthread_init
 *
 * Description:
 *	Initialization function for the whole pthread package. Set up
 *	all the thread global data and locks and then call the initialization
 * 	functions for mutexes, condition variables, thread attributes, specific
 *	data, signal handling and vp and stack management.
 *
 *	Next the initial thread is created. This function is called from crt0
 *	so this flow of execution is the initial thread by the time main
 *	gets called. To create the initial thread, much of the code in thread
 *	create is duplicated.
 *
 */
void
pthread_init()
{
	static	volatile int	pthreads_started = FALSE;
	pthread_t	self;
	vp_t		vp;

	spin_lock(&pthread_lock);
	/*
	 * Ensure that this is only executed once regardless of how many times
	 * it is called.
	 */
	if (pthreads_started) {
		spin_unlock(&pthread_lock);
		return;
	}
	pthreads_started = TRUE;
	/*
	 * initialize the active and free queues and the queue of all threads
	 * in existence.
	 */
	n_freepthreads = 0;
	pthread_queue_init(&free_pthreads);
	n_activepthreads = 0;
	pthread_queue_init(&active_pthreads);
	n_runningpthreads = 0;
	all_pthreads = NULL;
	spin_unlock(&pthread_lock);

	/*
	 * Initialize all the other threads components
	 */
	pthread_mutex_startup();
	pthread_cond_startup();
	pthread_attr_startup();
	specific_data_startup();
	pthread_sigwait_startup();

	vp = vp_startup();
	stack_startup(vp);

	/*
	 * try to create the initial thread. Allocate the thread structure
	 * but do not create a vp as we are already executing in one,
	 * vp_init has returned a vp id to describe the initial vp instead.
	 */
	self = pthread_alloc();
	if (self == NO_PTHREAD)
		pthread_internal_error("pthread_init");

	/*
	 * note that we are the initial thread.
	 */
	self->flags |= PTHREAD_INITIAL_THREAD;
	self->func = NULL;
	self->arg = 0;
	pthread_setname_np(self, "main");

	/*
	 * this is stolen from pthread_create
	 */
	pthread_queue_enq(&active_pthreads, &(self)->link);
	n_activepthreads++;
	n_runningpthreads++;
	self->all_thread_link = NULL;
	all_pthreads = self;

	vp_bind(vp, self);
	specific_data_setup(self);
	pthread_keycreate(&_pthread_cleanup_handlerqueue, NULL);
	pthread_setspecific(_pthread_cleanup_handlerqueue, (void *)&self->cleanup_queue);

	pthread_libs_init();
}

/*
 * Create the function pointer to pthread_init so that crt0 can find us.
 */
pthread_func_t _pthread_init_routine = (pthread_func_t)pthread_init;

/*
 * Function:
 *	pthread_create
 *
 * Parameters:
 *	thread	- pointer to the place to store the new pthread id
 *	attr	- attributes of the newly created thread
 *	start_routine - Function that the new thread is to execute
 *	arg	- parameter to be passed to the start routine
 *
 * Return value:
 *	0	Success
 *	-1	if the thread is an invalid pointer (EINVAL)
 *		if the attribute pointer is invalid (EINVAL)
 *		If no thread structure could be allocated
 *		if no vp could be created for the thread to execute on
 *
 * Description:
 *	Create a new thread of execution. Create a thread structure and a
 *	vp and bind them together. The stack is associated with the vp rather
 *	than the pthread, which is probably a mistake, which is why the
 *	specific data must be set up here. The completely created pthread
 *	is then put on the active list before it is allowed to execute.
 *
 */
int
pthread_create(pthread_t *thread, pthread_attr_t attr,
		  pthread_func_t start_routine, any_t arg)
{
	register vp_t	vp;

	PTHREAD_LOG("pthread_create", NULL);

	if ((attr == NO_ATTRIBUTE) || !(attr->flags&ATTRIBUTE_VALID) ||
	    (thread == NULL)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * allocate a thread structure
	 */
	*thread = pthread_alloc(attr);
	if (*thread == NO_PTHREAD)
		return(-1);

	/*
	 * set up the defaults for cancellation
	 */
	(*thread)->attr = *attr;
	(*thread)->async_cancel = CANCEL_OFF;
	(*thread)->sync_cancel = CANCEL_ON;
	(*thread)->pending_cancel = FALSE;

	/*
	 * create a vp for the thread to execute on
	 */
	vp = vp_create(attr);
	if (vp == NO_VP) {
		pthread_dealloc(*thread);
		return(-1);
	}

	/*
	 * save the threads function information used by vp_resume
	 */
	(*thread)->func = start_routine;
	(*thread)->arg = arg;

	vp_bind(vp, *thread);

	specific_data_setup(*thread);

	PTHREAD_LOG("pthread_create: new thread = %x", *thread);

	spin_lock(&pthread_lock);
	/*
	 * put the new thread on the active queue
	 */
	pthread_queue_enq(&active_pthreads, &(*thread)->link);
	n_activepthreads++;
	n_runningpthreads++;
	/*
	 * and add it to the list of all threads in the process
	 */
	(*thread)->all_thread_link = all_pthreads;
	all_pthreads = *thread;
	spin_unlock(&pthread_lock);

	/*
	 * start the new thread executing
	 */
	vp_resume(vp);
	return(0);
}

/*
 * Function:
 *	pthread_self
 *
 * Return value:
 *	The thread id of the calling thread
 *
 * Description:
 *	The thread id is found by finding the vp id and then our id is held
 *	in the vp structure, put there by vp_bind.
 */
pthread_t
pthread_self()
{
	register vp_t	vp;

	vp = vp_self();
	return(vp->pthread);
}

/*
 * Function:
 *	pthread_yield
 *
 * Description:
 *	yield the cpu to another deserving thread. Since the kernel can 
 *      schedule threads use the standard scheduler.
 *
 *      Note: vp_yield is not used since it yields by artifically droping
 *      the priority of the thread to the lowest value, that mechanism can
 *      interfere with realtime threads an thus is no longer used.
 *
 */
void
pthread_yield()
{
	PTHREAD_LOG("pthread_yield", NULL);
	swtch();
}

/*
 * Function:
 *	pthread_cleanup_unwind
 *
 * Parameters:
 *	self	- The pthread id of the calling thread
 *
 * Description:
 *	All the cleanup handlers that this thread has pushed into the
 *	cleanup stack are popped off and executed.
 *
 */
private void
pthread_cleanup_unwind(pthread_t self)
{
	pthread_cleanup_handler_t	*handler;

	while ((handler = self->cleanup_queue) != NULL) {
		/*
		 * remove the handler from the stack...
		 */
		self->cleanup_queue = handler->next_handler;
		/*
		 * ...and call it
		 */
		(*handler->handler_function)(handler->handler_arg);
	}
}
	

/*
 * Function:
 *	pthread_cleanup
 *
 * Parameters:
 *	self	- The pthread id of the calling thread
 *
 * Description:
 *	Cleanup a thread after it has exited. If this is the only thread
 *	left then exit the process. If not and there are other threads
 *	joining with this one then wake them up. If the thread has been
 *	detached then all the resources can be freed.
 */
private void
pthread_cleanup(pthread_t thread)
{
	vp_t	vp;

	pthread_mutex_lock(&thread->lock);
	thread->sync_cancel = CANCEL_OFF;
	pthread_mutex_unlock(&thread->lock);

	/*
	 * call any cleanup handlers
	 */
	pthread_cleanup_unwind(thread);

	/*
	 * call any specific data destructors the thread data
	 */
	specific_data_cleanup(thread);

	/*
	 * exit if we are the last thread left
	 */
	spin_lock(&pthread_lock);
	if (--n_runningpthreads == 0) {
		if (thread->state&PTHREAD_DETACHED)
			exit(0);
		else
			exit(-1);
	}
	spin_unlock(&pthread_lock);

	vp = thread->vp;

	/*
	 * lock the thread structure
	 */
	pthread_mutex_lock(&thread->lock);

	if ((thread->state & PTHREAD_DETACHED) && (thread->join_count == 0)) {
		/*
		 * The thread is detached and there are no other threads
		 * waiting for this thread so we can simply free up our
		 * resources.
		 */
		pthread_mutex_unlock(&thread->lock);
		pthread_free(thread);
	} else {
		/*
		 * This thread is either not detached or is detached with
		 * other threads waiting. Note that the thread status is
		 * valid and then signal the waiting threads.
		 */
		thread->state |= PTHREAD_RETURNED;
		if (thread->join_count > 0)
			pthread_cond_broadcast(&thread->done);
		pthread_mutex_unlock(&thread->lock);
	}

	/*
	 * Show that this vp is no longer bound to a thread
	 */
	vp_bind(vp, NO_PTHREAD);
}

/*
 * Function:
 *	pthread_body
 *
 * Parameters:
 *	vp	- the vp id of the calling thread
 *
 * Description:
 *	This is the function that every thread is created executing. It
 *	calls the start_routine specified to pthread create. As the vp
 *	being used can be cached and reused then this loop may be
 *	executed for a number of different pthreads.
 */
void
pthread_body(vp_t vp)
{
	pthread_t	thread;

	for (;;) {
#ifdef DEBUG
		if (vp != vp_self())
			pthread_internal_error("pthread_body: vp_self");
#endif
		PTHREAD_LOG("pthread_body: vp = %x", vp);

		/*
		 * find out which thread we are this time round the loop
		 */
		thread = vp->pthread;

		/*
		 * set up the thread specific cleanup queue for cancellation
		 */
		pthread_setspecific(_pthread_cleanup_handlerqueue,
					(void *)&thread->cleanup_queue);
		thread->cleanup_queue = NULL;

		/*
		 * we have to be able to be to cope with both a pthread_exit
		 * or a return from this function. Pthread_exit will longjmp
		 * back here.
		 */
		if (_setjmp(thread->exit_jmp) == 0) {
			thread->returned = (*(thread->func))(thread->arg);
			pthread_cleanup(thread); 
		}

		PTHREAD_LOG("pthread_body: returned = %x", thread->returned);

		/*
		 * The thread function is complete, clean the thread up and
		 * then suspend the vp for reuse by another thread
		 */

		vp_suspend(vp);
		/*
		 * We are a new pthread at this point so we go round the loop
		 * again. Note that the vp and the stack are the same but
		 * the vp->pthread and therefore thread->func are different.
		 */
	}
}

/*
 * Function:
 *	pthread_exit
 *
 * Parameters:
 *	status - exit status of the thread
 *
 * Description:
 *	Save the exit status of the thread so that other threads joining
 *	with this thread can find it. If this is the initial thread (ie
 *	using the starting threads stack) then we can't longjmp as the
 *	thread was not called via pthread_body so we fix it and cleanup
 *	as if it was. Otherwise just jump back as if the thread function
 *	returned (see pthread_body).
 */
void
pthread_exit(any_t status)
{
	pthread_t	thread;
	vp_t		vp;

	PTHREAD_LOG("pthread_exit: status = %d", status);

	thread = pthread_self();

	thread->returned = status;

	pthread_cleanup(thread);

	if (thread->flags & PTHREAD_INITIAL_THREAD) {
		/*
		 * There is no jumpbuf set as this is the main() thread.
		 * cleanup and suspend ourselves. If we get resumed we
		 * call pthread_body and pretend that this is a normal
		 * thread create.
		 */
		vp = thread->vp;
		vp_suspend(vp);
		pthread_body(vp);
	} else
		_longjmp(thread->exit_jmp, 1);
}

/*
 * Function:
 *	pthread_detach
 *
 * Parameters:
 *	thread_ptr - pointer to the thread to be detached
 *
 * Return value:
 *	0	Success
 *	-1	if thread_ptr is an invalid pointer (EINVAL)
 *		if the thread id is invalid (EINVAL)
 *		if the thread is already detached (ESRCH)
 *
 * Description:
 *	Detaching a running thread simply consists of marking it as such.
 *	If the thread has returned then the resources are also freed.
 */
int
pthread_detach(pthread_t *thread_ptr)
{
	pthread_t	thread;

	if ((thread_ptr == NULL) || (*thread_ptr == NO_PTHREAD)) {
		set_errno(EINVAL);
		return(-1);
	}

	thread = *thread_ptr;

	PTHREAD_LOG("pthread_detach: thread = %x", thread);

	/*
	 * lock the thread we are detaching
	 */
	pthread_mutex_lock(&thread->lock);

	/*
	 * check we are not detaching a detached thread
	 */
	if (thread->state & PTHREAD_DETACHED) {
		pthread_mutex_unlock(&thread->lock);
		set_errno(ESRCH);
		return(-1);
	}

	/*
	 * invalidate the callers handle
	 */
	*thread_ptr = NO_PTHREAD;

	/*
	 * mark the thread detached
	 */
	thread->state |= PTHREAD_DETACHED;

	if (thread->state & PTHREAD_RETURNED) {
		/*
		 * The thread is no longer executing. There will be
		 * no-one joining with it so we can free the resources.
		 */
		pthread_mutex_unlock(&thread->lock);
		pthread_free(thread);
	} else
		pthread_mutex_unlock(&thread->lock);

	return(0);
}

/*
 * Function:
 *	pthread_unjoin
 *
 * Parameters:
 *	thread - the thread that has been joined
 *
 * Description:
 *	Negate the effect of a join. This is used when a thread is cancelled
 *	when joining another thread. The thread would have been locked for
 *	us again so the join count is decremented. If this thread was the
 *	only joiner and the thread has detached then it is freed.
 */
private void
pthread_unjoin(pthread_t thread)
{
	if ((--thread->join_count == 0) && (thread->state & PTHREAD_DETACHED)) {
		pthread_mutex_unlock(&thread->lock);
		pthread_free(thread);
	} else
		pthread_mutex_unlock(&thread->lock);
}

/*
 * Function:
 *	pthread_join
 *
 * Parameters:
 *	thread - The id of the thread to be waited for
 *	status - pointer to a place to store the target threads exit status
 *
 * Return value:
 *	0	Success
 *	-1	if thread is an invalid pointer (EINVAL)
 *		if thread is the calling thread (EDEADLK)
 *		if the target thread is detached (ESRCH)
 *
 * Description:
 *	Wait for a thread to exit. If the status parameter is non-NULL then
 *	that threads exit status is stored in it.
 */
int
pthread_join(pthread_t thread, any_t *status)
{
	PTHREAD_LOG("pthread_join: thread = %x", thread);

	/*
	 * check the target thread is specified.
	 */
	if (thread == NO_PTHREAD) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * We cannot wait for ourselves.
	 */
	if (thread == pthread_self()) {
		set_errno(EDEADLK);
		return(-1);
	}

	pthread_mutex_lock(&thread->lock);

	/*
	 * You cannot wait for a detached thread
	 */
	if (thread->state & PTHREAD_DETACHED) {
		pthread_mutex_unlock(&thread->lock);
		set_errno(ESRCH);
		return(-1);
	}

	/*
	 * Note that we are joining with this thread
	 */
	thread->join_count++;

	/*
	 * prepare for cancellation as pthread_cond_wait is a cancellation
	 * point.
	 */
	pthread_cleanup_push(pthread_unjoin, (void *)thread);

	/*
	 * wait for the thread to exit.
	 */
	while (!(thread->state & PTHREAD_RETURNED))
		pthread_cond_wait(&thread->done, &thread->lock);

	/*
	 * pop the cleanup handler.
	 */
	pthread_cleanup_pop(FALSE);

	/*
	 * save the exit status if it is wanted.
	 */
	if (status != NULL)
		*status = thread->returned;

	/*
	 * note that we have joined. If this means that no-one else is
	 * waiting to join and no-one else can join as the thread is
	 * detached then we can free the threads resources.
	 */
	if ((--thread->join_count == 0) && (thread->state & PTHREAD_DETACHED))
		pthread_free(thread);

	pthread_mutex_unlock(&thread->lock);
	return(0);
}

/*
 * Function:
 *	pthread_once
 *
 * Parameters:
 *	once_block - determines if whether the init_routine has been called
 *	init_routine - The initialization routine to be called
 *
 * Return value:
 *	0	Success
 *	-1	if once_block is an invalid pointer (EINVAL)
 *		if init_routine is an invalid pointer (EINVAL)
 *		if a mutex cannot be allocated for the once_block
 *		if a condition variable cannot be allocated for the once_block
 *
 * Description:
 *	This function ensures that the init_routine is called only once and
 *	that no thread returns from this function until the function has
 *	has returned.
 *
 *	The once block is a skeleton initially and the first caller fills
 *	the rest of the block in with the mutex and condition variable. this
 *	is done as mutexes and condition variables cannot be statically
 *	intialized.
 */
int
pthread_once(pthread_once_t *once_block, void (*init_routine)())
{
	PTHREAD_LOG("pthread_once: once_block = %x", once_block);

	/*
	 * check the pointers were have been given are OK
	 */
	if ((once_block == NULL) || (init_routine == NULL)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * lock the once block to check if it has been initialised.
	 */
	spin_lock(&once_block->lock);
	if (once_block->flag == ONCE_UNINITIALIZED) {
		/*
		 * This is the first time through. We need to create
		 * a mutex and a condition variable for the once block
		 */
		if (!(once_block->mutex.flags&MUTEX_VALID)) {
			if (pthread_mutex_init(&once_block->mutex,
					pthread_mutexattr_default) < 0) {
				spin_unlock(&once_block->lock);
				return(-1);
			}
		}
		/*
		 * got the mutex, now get the condition variable
		 */
		if (!(once_block->executed.flags&COND_VALID)) {
			if (pthread_cond_init(&once_block->executed,
					pthread_condattr_default) < 0) {
				spin_unlock(&once_block->lock);
				return(-1);
			}
		}
		/*
		 * The once block is now complete.
		 */
		once_block->flag = ONCE_INITIALIZED;
	}
	spin_unlock(&once_block->lock);

	pthread_mutex_lock(&once_block->mutex);
	/*
	 * check to see what state the initalization routine is in.
	 */
	switch (once_block->flag) {
	case	ONCE_EXECUTED:
		/*
		 * the initalization routine has returned, so can we
		 */
		pthread_mutex_unlock(&once_block->mutex);
		break;
	case	ONCE_EXECUTING:
		/*
		 * the initalization routine is executing so we wait on
		 * the condition variable until it is done
		 */
		do {
			pthread_cond_wait(&once_block->executed,
							&once_block->mutex);
		} while (once_block->flag == ONCE_EXECUTING);
		pthread_mutex_unlock(&once_block->mutex);
		break;
	case	ONCE_INITIALIZED:
		/*
		 * the initalization routine has not been called yet.
		 * mark the once block as executing and call the function
		 */
		once_block->flag = ONCE_EXECUTING;
		pthread_mutex_unlock(&once_block->mutex);
		(*init_routine)();
		/*
		 * We can now update the once block to show the initialization
		 * is complete and signal anyone who was waiting
		 */
		pthread_mutex_lock(&once_block->mutex);
		once_block->flag = ONCE_EXECUTED;
		pthread_mutex_unlock(&once_block->mutex);
		pthread_cond_broadcast(&once_block->executed);
		break;
	}
	return(0);
}

/*
 * Function:
 *	pthread_deactivate
 *
 * Parameters:
 *	thread - the thread to deactivate
 *
 * Description:
 *	The action of deactivation simply means removing the thread
 *	structure from the active queue. This function is used by the
 *	condition variable functions so they do not have to know the
 *	details of the thread queue organization.
 */
void
pthread_deactivate(pthread_t thread)
{
	spin_lock(&pthread_lock);
	n_activepthreads--;
	pthread_queue_remove(&active_pthreads, &thread->link);
	spin_unlock(&pthread_lock);
}

/*
 * Function:
 *	pthread_activate
 *
 * Parameters:
 *	thread - the thread to activate
 *
 * Description:
 *	The action of activation simply means adding the thread
 *	structure to the active queue. This function is used by the
 *	condition variable functions so they do not have to know the
 *	details of the thread queue organization.
 */
void
pthread_activate(pthread_t thread)
{
	spin_lock(&pthread_lock);
	n_activepthreads++;
	pthread_queue_enq(&active_pthreads, &thread->link);
	spin_unlock(&pthread_lock);
}

/*
 * Function:
 *	pthread_equal
 *
 * Parameters:
 *	t1 - one thread id
 *	t2 - the second id for comparison
 *
 * Return value:
 *	0 if the two thread ids do not refer to the same thread. Otherwise
 *	non-zero.
 *
 * Description:
 *	This function will normally be used as the macro in pthread.h
 */
#undef pthread_equal
int
pthread_equal(pthread_t t1, pthread_t t2)
{
	return(t1 == t2);
}

/*
 * Function:
 *	pthread_cancel_deliver
 *
 * Parameters:
 *	thread - the thread id of the calling thread
 *
 * Description:
 *	This function calls the cleanup handlers and then exits the thread.
 *	This implements the action of being cancelled.
 */
private void
pthread_cancel_deliver(pthread_t thread)
{
	pthread_cleanup_unwind(thread);
	pthread_exit((void *)-1);
}

/*
 * Function:
 *	pthread_testcancel
 *
 * Description:
 *	Open a cancellation point. The thread will be cancelled if general
 *	cancellability is on and a cancel is pending.
 */
void
pthread_testcancel()
{

	pthread_t self;

	self = pthread_self();

	pthread_mutex_lock(&self->lock);
	/*
	 * if general cancellability is on and a cancel is pending then
	 * cancel the thread.
	 */
	if ((self->pending_cancel == TRUE) &&
	    (self->sync_cancel == CANCEL_ON)) {
		/*
		 * turn off all cancellation during cleanup
		 */
		self->async_cancel = CANCEL_OFF;
		self->sync_cancel = CANCEL_OFF;
		pthread_mutex_unlock(&self->lock);
		/*
		 * clean up and exit
		 */
		pthread_cancel_deliver(self);
		/* NOTREACHED */
	}
	pthread_mutex_unlock(&self->lock);
}

/*
 * Function:
 *	pthread_setcancel
 *
 * Parameters:
 *	state - what state to set cancelability, ON or OFF.
 *
 * Return value:
 *	0	Success
 *	-1	if state is not either CANCEL_ON or CANCEL_OFF (EINVAL)
 *
 * Description:
 *	set the general cancelability of the calling thread. This is not
 *	a cancellation point.
 */
int
pthread_setcancel(int state)
{
	pthread_t	self;
	int		oldstate;

	/*
	 * check the new state is valid
	 */
	if ((state != CANCEL_ON) && (state != CANCEL_OFF)) {
		set_errno(EINVAL);
		return(-1);
	}

	self = pthread_self();

	pthread_mutex_lock(&self->lock);
	/*
	 * the old state is to be returned so save it before the new state
	 * is assigned
	 */
	oldstate = self->sync_cancel;
	self->sync_cancel = state;
	pthread_mutex_unlock(&self->lock);
	if (state == CANCEL_ON)
		pthread_testcancel();
	return(oldstate);
}

/*
 * Function:
 *	pthread_setasynccancel
 *
 * Parameters:
 *	state - what state to set cancelability, ON or OFF.
 *
 * Return value:
 *	0	Success
 *	-1	if state is not either CANCEL_ON or CANCEL_OFF (EINVAL)
 *
 * Description:
 *	set the general cancelability of the calling thread. This is
 *	opens a cancellation point in case there is a pending cancel.
 *	General cancelability must be on for async cancelability to
 *	mean anything.
 */
int
pthread_setasynccancel(int state)
{
	pthread_t	self;
	int		oldstate;

	/*
	 * check the new state is valid
	 */
	if ((state != CANCEL_ON) && (state != CANCEL_OFF)) {
		set_errno(EINVAL);
		return(-1);
	}

	self = pthread_self();

	pthread_mutex_lock(&self->lock);
	/*
	 * the old state is to be returned so save it before the new state
	 * is assigned
	 */
	oldstate = self->async_cancel;
	self->async_cancel = state;
	pthread_mutex_unlock(&self->lock);
	/*
	 * if the new state is to enable cancellation then check that one
	 * is not already pending from when is was blocked.
	 */
	if (state == CANCEL_ON)
		pthread_testcancel();
	return(oldstate);
}

/*
 * Function:
 *	pthread_cancel
 *
 * Parameters:
 *	thread - the id of the target thread to cancel
 *
 * Return value:
 *	0	Success
 *	-1	if thread is an invalid poiner (EINVAL)
 *
 * Description:
 *	If the target threads general cancelability is off the cancellation
 *	is marked pending. If the general cancelability is on then if the
 *	async cancelability is on, the thread is forced to cancel immediately.
 *	Otherwise the cancellation is marked pending and if the thread is at
 *	a cancellation point it is aborted.
 */
int
pthread_cancel(pthread_t thread)
{
	pthread_t	self;

	if (thread == NO_PTHREAD) {
		set_errno(EINVAL);
		return(-1);
	}

	pthread_mutex_lock(&thread->lock);
	/*
	 * if general cancelability is off then mark the cancel pending
	 * and return
	 */
	if (thread->sync_cancel == CANCEL_OFF) {
		thread->pending_cancel = TRUE;
		pthread_mutex_unlock(&thread->lock);
		return(0);
	}

	if (thread->async_cancel == CANCEL_ON) {
		/*
		 * both general and async cancelability are on. Turn both
		 * states off and then force the thread to cancel
		 */
		thread->async_cancel = CANCEL_OFF;
		thread->sync_cancel = CANCEL_OFF;
		pthread_mutex_unlock(&thread->lock);
		/*
		 * If we are canceling the calling thread then just call the
		 * the cancellation delivery function which never returns.
		 * If we are cancelling another thread then we have to muck
		 * with its PC to get it to call the cancellation delivery
		 * function itself.
		 */
		if (thread == pthread_self())
			pthread_cancel_deliver(thread);
		else
			vp_call(thread->vp, pthread_cancel_deliver, thread);
	} else {
		/*
		 * general cancelability is on but async cancelability is
		 * off. If the thread is at a cancellation point and this
		 * is the first cancel, then break it out of the wait.
		 */
		if ((thread->state&PTHREAD_CONDWAIT) &&
		   !(thread->pending_cancel))
			vp_event_notify(thread->vp, EVT_CANCEL);
		/*
		 * set the cancellation pending so we can see it
		 */
		thread->pending_cancel = TRUE;
	}
	pthread_mutex_unlock(&thread->lock);
	return(0);
}

/*
 * Function:
 *	pthread_internal_error
 *
 * Parameters:
 *	error - string to be printed describing the error
 *
 * Description:
 *	Fatal internal error. print a message and die.
 */
void
pthread_internal_error(char *error)
{
	printf("pthread internal error in %s\n", error);
#ifdef DEBUG
	pthread_dump();
	pthread_vp_dump();
#endif
	exit(1);
}

/*
 * Function:
 *	pthread_fork_prepare
 *
 * Description:
 *	Quiesce the threads package prior to a fork. This makes it easier
 *	to clean up in the child after the fork has completed. Each component
 *	quiesces itself.
 */
void
pthread_fork_prepare()
{
	pthread_t	self;

	self = pthread_self();
	pthread_mutex_lock(&self->lock);

	spin_lock(&pthread_lock);

	specific_fork_prepare();
	malloc_fork_prepare();
	stack_fork_prepare();
	vp_fork_prepare();
}

/*
 * Function:
 *	pthread_fork_parent
 *
 * Description:
 *	This is called in the  parent after a fork. All components are set
 *	free so the parent can continue to run as before. Components are
 *	released in the reverse order that they were frozen to avoid deadlock.
 */
void
pthread_fork_parent()
{
	pthread_t	self;

	vp_fork_parent();
	stack_fork_parent();
	malloc_fork_parent();
	specific_fork_parent();

	spin_unlock(&pthread_lock);

	self = pthread_self();
	pthread_mutex_unlock(&self->lock);
}

/*
 * Function:
 *	pthread_fork_child
 *
 * Description:
 *	This is called in the child process after a fork. Clean up all the
 *	global data in all the subsystems and make it look like the world
 *	just after pthread_init returns. There is only one thread (and one
 *	vp therefore) running.
 */
void
pthread_fork_child()
{
	pthread_t	self;
	pthread_t	thread;

	/*
	 * unlock all the other components
	 */
	vp_fork_child();
	stack_fork_child();
	malloc_fork_child();
	specific_fork_child();

	spin_unlock(&pthread_lock);

	/*
	 * self will still give the same as the parent as the stack is at the
	 * same address.
	 */
	self = pthread_self();
	pthread_mutex_unlock(&self->lock);

	/*
	 * go through the list of all threads and free up all the resources
	 * they are using.
	 */
	while (all_pthreads != NULL) {
		thread = all_pthreads;
		all_pthreads = thread->all_thread_link;
		/*
		 * dequeue the thread regardless of where is is linked to,
		 * the active queue, free queue or a condition waiters queue
		 * The important one is the condition queues. This is because
		 * we don't want non-existent pthread handles attached to a
		 * application created condition variable after the fork.
		 */
		pthread_queue_deq(&thread->link);
		if (thread != self) {
			vp_dealloc(thread->vp);
			pthread_dealloc(thread);
		}
	}
	self->all_thread_link = NULL;
	all_pthreads = self;

	/*
	 * Taken from pthread_init()
	 */
	n_freepthreads = 0;
	pthread_queue_init(&free_pthreads);
	n_activepthreads = 0;
	pthread_queue_init(&active_pthreads);
	n_runningpthreads = 0;

	pthread_queue_enq(&active_pthreads, &(self)->link);
	n_activepthreads++;
	n_runningpthreads++;
}

#ifdef DEBUG

int	log_lock = SPIN_LOCK_UNLOCKED;

/*
 * Function:
 *	pthread_log
 *
 * Parameters:
 *	string - a printf format string
 *	value - a single argument to be given to printf
 *
 * Description:
 *	print out interlocked debugging information.
 */
void
pthread_log(char *string, any_t value)
{
	pthread_t	self;

	self = pthread_self();

	spin_lock(&log_lock);
	/*
	 * print out the callers name, if there is one
	 */
	if (self->name == 0)
		printf("[%x] ", self);
	else
		printf("[%s] ", pthread_getname_np(self));
	/*
	 * now print the message passed
	 */
	printf(string, value);
	printf("\n");
	spin_unlock(&log_lock);
}

/*
 * Function:
 *	pthread_dump_queue
 *
 * Parameters:
 *	queue - a queue of pthread structures to print
 *
 * Description:
 *	Print out information about all the threads in a specified queue of
 *	threads.
 */
void
pthread_dump_queue(pthread_queue *queue)
{
	pthread_t	thread;

	printf("NAME     ADDRESS  FLAGS    STATE    VP\n");
	/*
	 * for every thread in the queue ...
	 */
	for (thread = (pthread_t)pthread_queue_head(queue);
	     thread != (pthread_t)pthread_queue_end(queue);
	     thread = (pthread_t)pthread_queue_next(&thread->link)) {
		/*
		 * first the thread information
		 */
		printf("%-8s %-8x %-8x %-8x %-8x ", pthread_getname_np(thread),
			thread, thread->flags, thread->state, thread->vp);
		/*
		 * Then information about the threads mutex
		 */
		pthread_mutex_dump(&thread->lock);
		/*
		 * Then information about the threads condition variable
		 */
		pthread_cond_dump(&thread->done);
		printf("\n");
	}
}

/*
 * Function:
 *	pthread_dump
 *
 * Description:
 *	Print all the information available about all the thread global
 *	structures. These are the thread lock, the active and the free
 *	queues.
 */
void
pthread_dump()
{
	int	must_unlock;

	if (!spin_trylock(&pthread_lock)) {
		printf("Thread lock already set\n");
		must_unlock = FALSE;
	} else
		must_unlock = TRUE;

	/*
	 * Even if we can't get the lock, we are in debug mode so we
	 * do our best
	 */
	printf("Active Queue: %d entries\n", n_activepthreads);
	if (n_activepthreads > 0)
		pthread_dump_queue(&active_pthreads);

	printf("Free Queue: %d entries\n", n_freepthreads);
	if (n_freepthreads > 0)
		pthread_dump_queue(&free_pthreads);

	if (must_unlock)
		spin_unlock(&pthread_lock);
}
#endif
