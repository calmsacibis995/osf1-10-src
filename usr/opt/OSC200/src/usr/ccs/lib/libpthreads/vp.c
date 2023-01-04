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
static char	*sccsid = "@(#)$RCSfile: vp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:13:12 $";
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
 * File: vp.c
 *
 * This file contains all the functions that manipulate virtual processors or
 * vps. This include creation, deletion, caching, cancellation and fork.
 *
 * pthreads map 1:1 with vps during their execution but a vp may execute
 * several pthreads during the course of a processes life time. Free vps
 * are hashed onto VP_HASH_MAX hash queues based on stack size.
 */

#include <pthread.h>
#include <errno.h>
#include "internal.h"

/*
 * Local externals
 */
extern void
swtch_pri C_PROTOTYPE((int));

/*
 * Local Definitions
 */
#define	VP_HASH_MAX	16
#define	VP_HASH_RANGE	(10 * vm_page_size)

/*
 * Local Macros
 */
#define	VP_HASH_NEXT(val)	(val == VP_HASH_MAX-1 ? 0 : val + 1)
#define	VP_HASH_PREV(val)	(val == 0 ? VP_HASH_MAX - 1 : val - 1)
#define	VP_HASH_INDEX(key)	(min((key / VP_HASH_RANGE), (VP_HASH_MAX - 1)))

/*
 * Local Variables
 */
private	volatile int	vp_lock;
private	unsigned	n_freevps;
private	pthread_queue	active_vps;
private	pthread_queue	free_vps[VP_HASH_MAX];

/*
 * Function:
 *	vp_alloc
 *
 * Return value:
 *	NO_VP	if a new vp could not be created for any reason (errno is set)
 *	pointer to a vp structure otherwise
 *
 * Description:
 *	Allocate a new vp data structure and event port. The kernel thread
 *	is created elsewhere.
 */
private vp_t
vp_alloc()
{
	vp_t	vp;

	/*
	 * Get the memory for the structure
	 */
	vp = (vp_t)malloc(sizeof(struct vp));
	if (vp == NO_VP) {
		set_errno(ENOMEM);
		return(NO_VP);
	}

	/*
	 * Allocate the mach ipc port for the event port
	 */
	if (!allocate_event_port(&vp->event_port)) {
		free(vp);
		return(NO_VP);
	}

	/*
	 * all done. Note that this is an ordinary vp and not attached to 
	 * a pthread.
	 */
	vp->flags = 0;
	vp->pthread = NO_PTHREAD;

	return(vp);
}

/*
 * Function:
 *	vp_free
 *
 * Parameters:
 *	vp	- The vp to deallocate
 *
 * Description:
 *	Free the vps event port and the memory associated with the vp
 *	structure
 */
private void
vp_free(vp_t vp)
{
	/*
	 * free the event port
	 */
	deallocate_event_port(vp->event_port);
	/*
	 * ditch the memory for the structure
	 */
	free(vp);
}

/*
 * Function:
 *	vp_startup
 *
 * Return value:
 *	A pointer to a vp structure for the initial vp
 *
 * Description:
 *	This function is called by pthread_init to initialize the vp global
 *	data and to create the vp for the initial thread.
 */
vp_t
vp_startup()
{
	vp_t	vp;
	int	i;

	/*
	 * Initialize the active vp queue and the free queues to be empty
	 * Start off with no free vps and initialize the lock which protects
	 * all this data.
	 */
	pthread_queue_init(&active_vps);
	for (i = 0; i < VP_HASH_MAX; i++)
		pthread_queue_init(&free_vps[i]);
	n_freevps = 0;
	vp_lock = SPIN_LOCK_UNLOCKED;

	/*
	 * allocate a vp structure for the initial vp and chain it onto the
	 * active list
	 */
	vp = vp_alloc();
	pthread_queue_enq(&active_vps, &vp->link);

	/*
	 * mark the vp as active and get its kernel thread id
	 */
	vp->flags |= (VP_INITIAL_STACK | VP_STARTED);
	vp->id = thread_self();

	return(vp);
}

/*
 * Function:
 *	vp_dealloc
 *
 * Parameters:
 *	vp	- the vp to be deallocated
 *
 * Description:
 *	Destroy a vp. This function frees all things associated with the vp.
 */
void
vp_dealloc(vp_t vp)
{
	/*
	 * remove the vp from any queue it may be on, get rid of the stack
	 * it is using and then free up the vp structure itself.
	 */
	pthread_queue_deq(&vp->link);
	dealloc_stack(vp);
	vp_free(vp);
}

/*
 * Function:
 *	vp_detach
 *
 * Parameters:
 *	The vp to be detached
 *
 * Description:
 *	This function is similar to pthread_detach in that it marks the
 *	resources of a vp free to be reused when necessary. It does this
 *	by putting it on the free queue.
 */
private void
vp_detach(vp_t vp)
{
	unsigned int	free_queue;

	/*
	 * Find out which free queue this vp should be put on
	 */
	free_queue = VP_HASH_INDEX(vp->stacksize);

	/*
	 * Lock the vp data and add the vp to the appropriate free vp
	 * list. update the free vp count
	 */
	spin_lock(&vp_lock);
	pthread_queue_remove(&active_vps, &vp->link);
	pthread_queue_enq(&free_vps[free_queue], &vp->link);
	n_freevps++;
	spin_unlock(&vp_lock);
}

/*
 * Function:
 *	vp_suspend
 *
 * Parameters:
 *	vp	- the vp to suspend. This must be the callers vp.
 *
 * Description:
 *	The vp is detached so that is can be cached and reused and then
 *	suspends execution by waiting for an event to arrive on its port.
 */
void
vp_suspend(vp_t vp)
{
	int	event;

	/*
	 * cache the vp and then suspend execution
	 */
	vp_detach(vp);
	vp_event_wait(vp, &event, NO_TIMEOUT);

	/*
	 * We have been woken up. Check that this is the correct event type
	 * We do not expect anything else but a resume
	 */
	if (event != EVT_RESUME)
		pthread_internal_error("vp_suspend");
}

/*
 * Function:
 *	vp_resume
 *
 * Parameters:
 *	vp	- the vp to be started up
 *
 * Description:
 *	start a vp executing. The vp may be in one of two states. either
 *	it is a new kernel thread in which case it needs to be resumed by
 *	the kernel or it is a cached vp and is woken by sending it an event.
 */
void
vp_resume(vp)
vp_t	vp;
{
	if (vp->flags & VP_STARTED) {
		/*
		 * This is a cached vp, send the resume event
		 */
		vp_event_notify(vp, EVT_RESUME);
	} else {
		/*
		 * this is a new kernel thread. Mark is as started and
		 * get the kernel to resume it
		 */
		vp->flags |= VP_STARTED;
		if (thread_resume(vp->id) != KERN_SUCCESS)
			pthread_internal_error("vp_resume");
	}
}

/*
 * Function:
 *	vp_restart
 *
 * Parameters:
 *	vp	- the vp to be restarted
 *
 * Description:
 *	A vp needs to be restarted when it has changed user stacks. This
 *	is done when there is no free vp with a large enough stack for a 
 *	new pthread. The stack is realloc'd and the vp restarted.
 */
void
vp_restart(vp_t vp)
{
	/*
	 * The vp will need a resume not an event to get going again
	 */
	vp->flags &= ~VP_STARTED;

	/*
	 * It is currently in a msg_receive so we abort that for the
	 * resume to work
	 */
	if (thread_suspend(vp->id) != KERN_SUCCESS)
		pthread_internal_error("thread_suspend");
	if (thread_abort(vp->id) != KERN_SUCCESS)
		pthread_internal_error("thread_abort");
}

/*
 * Function:
 *	vp_create
 *
 * Parameters:
 *	attr	- the attributes to create the vp with
 *
 * Return value:
 *	NO_VP	if the vp creation failed for any reason (errno is set)
 *	a pointer to the new vp otherwise
 *
 * Description:
 *	Create a vp ready to be resumed. If there are free vps (cached) then
 *	try and find one with at least the stack size specified in the
 *	attributes passed. If the are vps cached but the stacks are too small
 *	then realloc the stack. This should be quicker that creating a new one
 *	from scratch. If there are no vps cached then create a new vp
 *	completely.
 */
vp_t
vp_create(pthread_attr_t attr)
{
	vp_t	vp;
	int	stack_size;
	int	start;
	int	end;

	stack_size = pthread_attr_getstacksize(attr);

	/*
	 * check to see if there are any used vps around for us to reuse.
	 */
	spin_lock(&vp_lock);
	if (n_freevps == 0) {
		/*
		 * There are no free vps so we have to create one for
		 * ourselves. This involves allocating a vp structure,
		 * a kernel thread, a stack and then initializing the
		 * lot. This is then put on the active queue and returned.
		 */
		spin_unlock(&vp_lock);
		vp = vp_alloc();
		if (vp == NO_VP)
			return(NO_VP);

		/*
		 * Now create the kernel thread
		 */
		if (thread_create(task_self(), &vp->id) != KERN_SUCCESS) {
			vp_free(vp);
			set_errno(EAGAIN);
			return(NO_VP);
		}

		/*
		 * Create a stack to use
		 */
		if (!alloc_stack(vp, stack_size)) {
			vp_free(vp);
			if (thread_terminate(vp->id) != KERN_SUCCESS)
				pthread_internal_error("thread_terminate");
			return(NO_VP);
		}

		/*
		 * set up the stack to execute pthread_body when it is resumed
		 */
		vp_setup(vp);

		/*
		 * Put the new thread on the active queue
		 */
		spin_lock(&vp_lock);
		pthread_queue_enq(&active_vps, &vp->link);
		spin_unlock(&vp_lock);
		return(vp);
	}


	/*
	 * There is a used vp for us somewhere. We make the assumption that
	 * it is cheaper to reuse a vp than to create a new one from scratch.
	 * The vp free queue is a hash queue based on stacksize. Search it
	 * to find one that has at least the stack size requested. If not take
	 * the first one that comes along.
	 */
	start = VP_HASH_INDEX(stack_size);
	end = start;
	vp = NO_VP;

	do {
		if (!pthread_queue_empty(&free_vps[start])) {
			/*
			 * We have found a vp to use. Remove it from the
			 * free list and break out of the search loop.
			 * We still have to check that its stack is big
			 * enough.
			 */
			vp = (vp_t)pthread_queue_head(&free_vps[start]);
			pthread_queue_remove(&free_vps[start], &vp->link);
			n_freevps--;
			spin_unlock(&vp_lock);
			break;
		}
		/*
		 * No cached vp found yet, look in the next hash bucket
		 */
		start = VP_HASH_NEXT(start);
	} while (start != end);

	if (vp == NO_VP)
		pthread_internal_error("free vp search");

	/*
	 * check to see that the stacksize of the vp we found was big
	 * enough. If not we have to replace the stack with a bigger
	 * one. This means we have to restart the vp.
	 */
	if (vp->stacksize < stack_size) {
		/*
		 * We need a new stack so all the user context on the
		 * last one will be lost. This means we have to restart
		 * the vp as if it was new.
		 */
		vp_restart(vp);
		spin_lock(&vp_lock);
		if (!realloc_stack(vp, stack_size)) {
			spin_unlock(&vp_lock);
			vp_free(vp);
			if (thread_terminate(vp->id) != KERN_SUCCESS)
				pthread_internal_error("thread_terminate");
			return(NO_VP);
		}
		spin_unlock(&vp_lock);
		/*
		 * set up the stack to execute pthread_body when it is resumed
		 */
		vp_setup(vp);
	}

	/*
	 * All done, put the vp on the active queue and return
	 */
	spin_lock(&vp_lock);
	pthread_queue_enq(&active_vps, &vp->link);
	spin_unlock(&vp_lock);
	return(vp);
}

/*
 * Function:
 *	vp_self
 *
 * Return value:
 *	a pointer to the vp structure that describes the calling vp
 *
 * Description:
 *	Search through all the vps in the active queue to find ours. If we
 *	are not there then search through the free vps. If not there we are
 *	in big trouble and we might as well not go on.
 */
vp_t
vp_self()
{
	register vp_t	vp;
	register int	hash_index;

	spin_lock(&vp_lock);

	/*
	 * Search the active list for the vp using our stack
	 */
	for (vp = (vp_t)pthread_queue_head(&active_vps);
	     vp != (vp_t)pthread_queue_end(&active_vps);
	     vp = (vp_t)pthread_queue_next(&vp->link)) {
		if (stack_self(vp)) {
			spin_unlock(&vp_lock);
			return(vp);
		}
	}

	/*
	 * We are being called by someone on the free queue.
	 * We are in the process of being suspended.
	 * Search the free list sequentially.
	 */
	for (hash_index = 0; hash_index < VP_HASH_MAX; hash_index++) {
		for (vp = (vp_t)pthread_queue_head(&free_vps[hash_index]);
		     vp != (vp_t)pthread_queue_end(&free_vps[hash_index]);
		     vp = (vp_t)pthread_queue_next(&vp->link)) {
			if (stack_self(vp)) {
				spin_unlock(&vp_lock);
				return(vp);
			}
		}
	}

	/*
	 * We are in trouble. Our vp is nowhere to be found.
	 */
	pthread_internal_error("vp_self");

	/* NOTREACHED */
}

/*
 * Function:
 *	vp_yield
 *
 * Description:
 *	allow another vp to run in preference by depressing our priority
 */
void
vp_yield()
{
	swtch_pri(0);
}

/*
 * Function:
 *	vp_bind
 *
 * Parameters:
 *	vp	- the vp to bind to the pthread
 *	thread	- the pthread to binf to the vp
 *
 * Description:
 *	Mark both the thread and vp that they belong to each other. The thread
 *	may be NO_PTHREAD which means that the vp is being unbound from the
 *	thread probably in order to be put on the free list
 */
void
vp_bind(vp_t vp, pthread_t thread)
{
	vp->pthread = thread;
	if (thread != NO_PTHREAD)
		thread->vp = vp;
}

/*
 * Function:
 *	vp_call
 *
 * Parameters:
 *	vp	- the vp to make the asynchronous function call.
 *	func	- the function to call
 *	arg	- the argument to pass to the function
 *
 * Description:
 *	This is used for asynchronous pthread cancellation. The vp is
 *	stopped in its tracks, a fake call frame is built and the vp resumed
 *	executing the trampoline code which calls the function.
 */
void
vp_call(vp, func, arg)
vp_t		vp;
pthread_func_t	func;
any_t		arg;
{
	/*
	 * Stop the vp, abort it in case it is waiting for an event (on a port)
	 * or executing a blocking system call.
	 */
	if (thread_suspend(vp->id) != KERN_SUCCESS)
		pthread_internal_error("thread_suspend");
	if (thread_abort(vp->id) != KERN_SUCCESS)
		pthread_internal_error("thread_abort");
	/*
	 * remember the function to call and its argument.
	 */
	vp->async_func = func;
	vp->async_arg = arg;
	/*
	 * build the fake stack frame and then resume the thread executing
	 * the delivery function
	 */
	vp_call_setup(vp);
	if (thread_resume(vp->id) != KERN_SUCCESS)
		pthread_internal_error("thread_resume");
}

/*
 * Function:
 *	vp_fork_prepare
 *
 * Description:
 *	quiesce the vp sub system prior to a fork. The lock protects all the
 *	global vp data likely to be changed.
 */
void
vp_fork_prepare()
{
	spin_lock(&vp_lock);
}

/*
 * Function:
 *	vp_fork_parent
 *
 * Description:
 *	Allow more vps to be created in the parent following a fork
 */
void
vp_fork_parent()
{
	spin_unlock(&vp_lock);
}

/*
 * Function:
 *	vp_fork_child
 *
 * Description:
 *	Free all the vps on the free queue in the child processes after
 *	a fork. Unlock the data structures so that future calls to vp_create
 *	will work.
 */
void
vp_fork_child()
{
	int	i;

	spin_unlock(&vp_lock);
	for (i = 0; i < VP_HASH_MAX; i++)
		pthread_queue_init(&free_vps[i]);
	n_freevps = 0;
}

#ifdef DEBUG

/*
 * Function:
 *	pthread_dump_vpqueue
 *
 * Parameters:
 *	queue	- the queue to print out
 *
 * Description:
 *	Debugging function to print all the vps on a given vp queue
 */
void
pthread_dump_vpqueue(pthread_queue *queue)
{
	vp_t	vp;

	if (pthread_queue_empty(queue)) {
		printf("QUEUE EMPTY\n");
		return;
	}

	printf("ADDRESS  FLAGS    ID       STACKBASE STACKSIZE EVT PORT PTHREAD\n");
	for (vp = (vp_t)pthread_queue_head(queue);
	     vp != (vp_t)pthread_queue_end(queue);
	     vp = (vp_t)pthread_queue_next(&vp->link)) {
		printf("%-8x %-8x %-8x %-8x  %-8x  %-8x %-8x\n", vp, vp->flags,
			vp->id, vp->stackbase, vp->stacksize, vp->event_port,
			vp->pthread);
	}
}

/*
 * Function:
 *	pthread_vp_dump
 *
 * Description:
 *	Debugging function to print out all the vo structures including all
 *	the free and active vp queues.
 */
void
pthread_vp_dump()
{
	int	i;
	int	must_unlock;

	if (!spin_trylock(&vp_lock)) {
		printf("VP lock already set\n");
		must_unlock = FALSE;
	} else
		must_unlock = TRUE;

	printf("Active VPs:\n");
	pthread_dump_vpqueue(&active_vps);
	printf("Free VPs: %d entries\n", n_freevps);
	for (i = 0; i < VP_HASH_MAX; i++) {
		printf("Free Queue %d\n", i);
		pthread_dump_vpqueue(&free_vps[i]);
	}
	if (must_unlock)
		spin_unlock(&vp_lock);
}
#endif
