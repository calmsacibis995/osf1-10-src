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
static char *rcsid = "@(#)$RCSfile: cmalib_aq_op.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:12 $";
#endif
/*
 *  Copyright (c) 1990, 1992 by
 *  Digital Equipment Corporation, Maynard Massachusetts.
 *  All rights reserved.
 *
 *  This software is furnished under a license and may be used and  copied
 *  only  in  accordance  with  the  terms  of  such  license and with the
 *  inclusion of the above copyright notice.  This software or  any  other
 *  copies  thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of  the  software  is  hereby
 *  transferred.
 *
 *  The information in this software is subject to change  without  notice
 *  and  should  not  be  construed  as  a commitment by DIGITAL Equipment
 *  Corporation.
 *
 *  DIGITAL assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by DIGITAL.
 */

/*
 *  FACILITY:
 *
 *	CMA Library services
 *
 *  ABSTRACT:
 *
 *	Generalized (Atomic) Queuing Services
 *	Hardware-Independent (Uniprocessor) Unix Implementation
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	26 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	29 August 1990
 *		Change calls to CMA to pass structures by reference.
 *	002	Paul Curtin	20 March 1991
 *		Added casting on object and queue types for MIPS cc.
 *	003	Paul Curtin	25 March 1991
 *		Adjust condition variable use in enqueue/dequeue.
 *	004	Paul Curtin	26 March 1991
 *		Now use interrupt level interrupt enable/disable
 *		macros (..._int).
 *	005	Paul Curtin	27 March 1991
 *		Added a fix to _requeue
 *		Added some signals in places.
 *	006	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_library.h>
#include <cmalib_defs.h>
#include <cmalib_queue.h>
#include <cmalib_aq_cre.h>
#include <cmalib_aq_op.h>
#include <cmalib_handle.h>
#include <cmalib_init.h>
#include <signal.h>


/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */
extern void
cma_lib__aq_augment _CMA_PROTOTYPE_ ((
	cma_lib__t_aq_header	*queue_head,
	cma_t_integer		node_cnt));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_dequeue:  Remove an item from the front of a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of a pointer (or a variable) to receive the 
 *			address of the item (or the value) which was removed 
 *			from the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	The calling thread will block, if the queue is empty, until an item is
 *	inserted.
 */
extern void
cma_lib_queue_dequeue
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   *element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   *element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    sigset_t			prior;		/* For disabling interrupts */


    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);

    /*
     * Protect against concurrent access to the queue:
     *	  - block interrupts (signals) since signal access is assumed to be
     *		exclusive and therefore doesn't synchronize
     *	  - lock the queue object mutex to block out other threads.
     */
    cma_lib__interrupt_disable (prior);
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Wait at a condition variable as long as the queue is empty.
     */
    while (cma_lib__queue_empty (&(queue_obj->queue_head.items))) {
	/*
	 * During the wait, return access to the queue:
	 *    - reenable interrupts, in case it is an interrupt which will
	 *	    insert the item we're looking for.
	 *    - unlock the mutex (done automatically by the condition wait).
	 * After the wait completes, revoke it again.
	 */
	cma_lib__interrupt_enable (prior);
	cma_cond_wait (&(queue_obj->insert), &(queue_obj->mutex));
	cma_lib__interrupt_disable (prior);
	}

    /*
     * The queue now has something in it.  (There is no race here, because we 
     * have exclusive access to the queue object.)
     */
    queue_item = (cma_lib__t_queue_item *)
		    cma_lib__queue_dequeue (&(queue_obj->queue_head.items));


    /*
     * Retrieve user's value from our queue node
     */
    *element = queue_item->item;

    /*
     * "Free" the node
     */
    cma_lib__queue_insert_after	((cma_lib__t_queue *)queue_item, 
				    &(queue_obj->queue_head.free));


    /*
     * We are finished, return access to the queue:
     *    - unlock the mutex 
     *    - reenable interrupts
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_lib__interrupt_enable (prior);

    /*
     * Pass the news: there may be more than one item
     */
    cma_cond_signal (&(queue_obj->insert));

    /*
     * Indicate that there is now a free queue node (this will wake a thread
     * which is blocked on an insert).
     */
    cma_cond_signal (&(queue_obj->remove));
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_enqueue:  Insert an item at the tail of a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of an item, or an integer value, to be inserted
 *			into the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Calling thread will block, if the queue is full, until an item is 
 *	removed.
 */
extern void
cma_lib_queue_enqueue
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    sigset_t			prior;		/* For disabling interrupts */


    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);

    /*
     * Protect against concurrent access to the queue:
     *	  - block interrupts (signals) since signal access is assumed to be
     *		exclusive and therefore doesn't synchronize
     *	  - lock the queue object mutex to block out other threads.
     */
    cma_lib__interrupt_disable (prior);
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Wait at a condition variable as long as the queue is full (ie, as long
     * and the free list is empty).
     */
    while (cma_lib__queue_empty (&(queue_obj->queue_head.free))) {
	/*
	 * During the wait, return access to the queue:
	 *    - reenable interrupts, in case it is an interrupt which will
	 *	    insert the item we're looking for.
	 *    - unlock the mutex (done automatically by the condition wait).
	 * After the wait completes, revoke it again.
	 */
	cma_lib__interrupt_enable (prior);
	cma_cond_wait (&(queue_obj->remove), &(queue_obj->mutex));
	cma_lib__interrupt_disable (prior);
	}

    /*
     * The free list now has something in it.  (There is no race here, because 
     * we have exclusive access to the queue object.)
     */
    queue_item = (cma_lib__t_queue_item *)
		    cma_lib__queue_dequeue (&(queue_obj->queue_head.free));

    /*
     * Store user's value in our queue node
     */
    queue_item->item = element;

    /*
     * Insert the node at the tail of the queue
     */
    cma_lib__queue_insert ((cma_lib__t_queue *)queue_item, 
			    &(queue_obj->queue_head.items));

    /*
     * We are finished, return access to the queue:
     *    - unlock the mutex 
     *    - reenable interrupts
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_lib__interrupt_enable (prior);

    /*
     * Pass the news: there may be more than one item
     */
    cma_cond_signal (&(queue_obj->remove));

    /*
     * Indicate that there is now an item in the queue (this will wake a thread
     * which is blocked on a remove).
     */
    cma_cond_signal (&(queue_obj->insert));
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_requeue:  Insert an item at the head of a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of an item, or an integer value, to be inserted
 *			into the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Calling thread will block, if the queue is full, until an item is 
 *	removed.
 */
extern void
cma_lib_queue_requeue
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    sigset_t			prior;		/* For disabling interrupts */


    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);

    /*
     * Protect against concurrent access to the queue:
     *	  - block interrupts (signals) since signal access is assumed to be
     *		exclusive and therefore doesn't synchronize
     *	  - lock the queue object mutex to block out other threads.
     */
    cma_lib__interrupt_disable (prior);
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Wait at a condition variable as long as the queue is full (ie, as long
     * and the free list is empty).
     */
    while (cma_lib__queue_empty (&(queue_obj->queue_head.free))) {
	/*
	 * During the wait, return access to the queue:
	 *    - reenable interrupts, in case it is an interrupt which will
	 *	    insert the item we're looking for.
	 *    - unlock the mutex (done automatically by the condition wait).
	 * After the wait completes, revoke it again.
	 */
	cma_lib__interrupt_enable (prior);
	cma_cond_wait (&(queue_obj->remove), &(queue_obj->mutex));
	cma_lib__interrupt_disable (prior);
	}

    /*
     * The free list now has something in it.  (There is no race here, because 
     * we have exclusive access to the queue object.)
     */
    queue_item = (cma_lib__t_queue_item *)
		    cma_lib__queue_dequeue (&(queue_obj->queue_head.free));

    /*
     * Store user's value in our queue node
     */
    queue_item->item = element;

    /*
     * Insert the node at the head of the queue
     */
    cma_lib__queue_insert_after ((cma_lib__t_queue *)queue_item, 
				 &(queue_obj->queue_head.items));

    /*
     * We are finished, return access to the queue:
     *    - unlock the mutex 
     *    - reenable interrupts
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_lib__interrupt_enable (prior);

    /*
     * Pass the news: there may be more than one item
     */
    cma_cond_signal (&(queue_obj->remove));

    /*
     * Indicate that there is now an item in the queue (this will wake a thread
     * which is blocked on a remove).
     */
    cma_cond_signal (&(queue_obj->insert));
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_try_dequeue:  Remove an item from the front of a queue.
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of a pointer (or a variable) to receive the 
 *			address of the item (or the value) which was removed 
 *			from the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Boolean indicating successful dequeue.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern cma_t_boolean
cma_lib_queue_try_dequeue 
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   *element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   *element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    cma_t_boolean		ret_val;
    sigset_t			prior;		/* For disabling interrupts */
    cma_t_integer		sgnl;

    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);
    sgnl = 0;
    /*
     * Protect against concurrent access to the queue:
     *	  - block interrupts (signals) since signal access is assumed to be
     *		exclusive and therefore doesn't synchronize
     *	  - lock the queue object mutex to block out other threads.
     */
    cma_lib__interrupt_disable (prior);
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Check to see if there is anything in the queue
     */
    if (!cma_lib__queue_empty (&(queue_obj->queue_head.items))) {
	/*
	 * The queue has something in it, remove the first item.  (There is no 
	 * race here, because we have exclusive access to the queue object.)
	 */
	queue_item = (cma_lib__t_queue_item *)
			cma_lib__queue_dequeue (&(queue_obj->queue_head.items));

	/*
	 * Retrieve user's value from our queue node
	 */
	*element = queue_item->item;

	/*
	 * "Free" the node
	 */
	cma_lib__queue_insert_after ((cma_lib__t_queue *)queue_item, 
				     &(queue_obj->queue_head.free));

	/*
	 * Indicate that there is now a free queue node (this will wake a 
	 * thread which is blocked on an insert).
	 */
	 sgnl = 1;

	/*
	 * Indicate a successful dequeue.
	 */
	ret_val = cma_c_true;
	}
    else {
	/*
	 * Be tidy.
	 */
	*element = cma_c_null_ptr;

	/*
	 * Indicate a failure (ie, queue empty).
	 */
	ret_val = cma_c_false;
	}

    /*
     * We are finished, return access to the queue:
     *    - unlock the mutex 
     *    - reenable interrupts
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_lib__interrupt_enable (prior);

    /*
     * Signal that there is a free queue node.
     */
    if (sgnl)
	cma_cond_signal (&(queue_obj->remove));

    return ret_val;
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_try_enqueue:  Insert an item at the tail of a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of an item, or an integer value, to be inserted
 *			into the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Boolean indicating successful enqueue.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern cma_t_boolean
cma_lib_queue_try_enqueue
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    cma_t_boolean		ret_val;
    sigset_t			prior;		/* For disabling interrupts */
    cma_t_integer		sgnl;

    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);
    sgnl = 0;
 
    /*
     * Protect against concurrent access to the queue:
     *	  - block interrupts (signals) since signal access is assumed to be
     *		exclusive and therefore doesn't synchronize
     *	  - lock the queue object mutex to block out other threads.
     */
    cma_lib__interrupt_disable (prior);
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Check to see if the queue is full (ie, check if the free list is empty)
     */
    if (!cma_lib__queue_empty (&(queue_obj->queue_head.free))) {
	/*
	 * The free list has something in it, remove the first item.  (There is 
	 * no race here, because we have exclusive access to the queue object.)
	 */
	queue_item = (cma_lib__t_queue_item *)
			cma_lib__queue_dequeue (&(queue_obj->queue_head.free));

	/*
	 * Store user's value in our queue node
	 */
	queue_item->item = element;

	/*
	 * Insert the node at the tail of the queue
	 */
	cma_lib__queue_insert ((cma_lib__t_queue *)queue_item, 
				&(queue_obj->queue_head.items));

	/*
	 * Indicate that there is now an item on the queue (this will wake a 
	 * thread which is blocked on an insert).
	 */
	sgnl = 1;

	/*
	 * Indicate a successful enqueue.
	 */
	ret_val = cma_c_true;
	}
    else {
	/*
	 * Indicate a failure (ie, queue full).
	 */
	ret_val = cma_c_false;
	}

    /*
     * We are finished, return access to the queue:
     *    - unlock the mutex 
     *    - reenable interrupts
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_lib__interrupt_enable (prior);

    /*
     * Signal there is an item on the queue.
     */
    if (sgnl) 
	cma_cond_signal (&(queue_obj->insert));


    return ret_val;
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_try_enqueue_int:  Insert an item on the tail of a queue.
 *		For use by interrupt level code.
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of an item, or an integer value, to be inserted
 *			into the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Boolean indicating successful enqueue.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern cma_t_boolean
cma_lib_queue_try_enqueue_int
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    cma_t_boolean		ret_val;
    sigset_t			prior;		/* For disabling interrupts */
    cma_t_integer		sgnl;

    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     *
     * FIX-ME:  this might result in an exception, which we are philosophically
     *		opposed to at interrupt level.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);
    sgnl = 0;

    /*
     * Protect against concurrent access to the queue.
     *
     * Note, it would not be necessary to block interrupts explicitly if we
     *	could assume that all signals are blocked for this signal handler.
     *
     * Note, do NOT lock a mutex here.  Assuming that we are excuting on a 
     *	uniprocessor, if there is a signal executing, then there cannot be
     *  a thread executing; therefore, locking a mutex is superfluous.  (Also,
     *  mutex services are not supported from interrupt level.)
     */
    cma_lib__interrupt_disable_int (prior);

    /*
     * Check to see if the queue is full (ie, check if the free list is empty)
     */
    if (!cma_lib__queue_empty (&(queue_obj->queue_head.free))) {
	/*
	 * The free list has something in it, remove the first item.  (There is 
	 * no race here, because we have exclusive access to the queue object.)
	 */
	queue_item = (cma_lib__t_queue_item *)
			cma_lib__queue_dequeue (&(queue_obj->queue_head.free));

	/*
	 * Store user's value in our queue node
	 */
	queue_item->item = element;

	/*
	 * Insert the node at the tail of the queue
	 */
	cma_lib__queue_insert ((cma_lib__t_queue *)queue_item, 
				&(queue_obj->queue_head.items));

	/*
	 * Indicate that there is now an item on the queue (this will wake a 
	 * thread which is blocked on an insert).
	 */
	sgnl = 1;

	/*
	 * Indicate a successful enqueue.
	 */
	ret_val = cma_c_true;
	}
    else {
	/*
	 * Indicate a failure (ie, queue full).
	 */
	ret_val = cma_c_false;
	}

    /*
     * We are finished, return access to the queue:
     *    - reenable interrupts
     */
    cma_lib__interrupt_enable_int (prior);

    /*
     * Signal that there is an item on the queue
     */
     if (sgnl)
	cma_cond_signal_int (&(queue_obj->insert));

    return ret_val;
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib_queue_try_requeue:  Insert an item at the head of a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue:	    The address of the handle for the queue object which 
 *			contains the queue
 *
 *	element:    The address of an item, or an integer value, to be inserted
 *			into the queue.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Boolean indicating successful requeue.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern cma_t_boolean
cma_lib_queue_try_requeue
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue,
	cma_t_address   element)
#else	/* no prototypes */
	(queue, element)
	cma_lib_t_queue	*queue;
	cma_t_address   element;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_queue_item	*queue_item;	/* Ptr to node removed from Q */
    cma_t_boolean		ret_val;
    sigset_t			prior;		/* For disabling interrupts */
    cma_t_integer		sgnl;


    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.
     */
    queue_obj = cma_lib__validate_queue_obj (queue);
    sgnl = 0;

    /*
     * Protect against concurrent access to the queue:
     *	  - block interrupts (signals) since signal access is assumed to be
     *		exclusive and therefore doesn't synchronize
     *	  - lock the queue object mutex to block out other threads.
     */
    cma_lib__interrupt_disable (prior);
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Check to see if the queue is full (ie, check if the free list is empty)
     */
    if (!cma_lib__queue_empty (&(queue_obj->queue_head.free))) {
	/*
	 * The free list has something in it, remove the first item.  (There is 
	 * no race here, because we have exclusive access to the queue object.)
	 */
	queue_item = (cma_lib__t_queue_item *)
			cma_lib__queue_dequeue (&(queue_obj->queue_head.free));

	/*
	 * Store user's value in our queue node
	 */
	queue_item->item = element;

	/*
	 * Insert the node at the tail of the queue
	 */
	cma_lib__queue_insert_after((cma_lib__t_queue *)queue_item, 
				    &(queue_obj->queue_head.items));

	/*
	 * Indicate that there is now an item on the queue (this will wake a 
	 * thread which is blocked on an insert).
	 */
	sgnl = 1;

	/*
	 * Indicate a successful enqueue.
	 */
	ret_val = cma_c_true;
	}
    else {
	/*
	 * Indicate a failure (ie, queue full).
	 */
	ret_val = cma_c_false;
	}

    /*
     * We are finished, return access to the queue:
     *    - unlock the mutex 
     *    - reenable interrupts
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_lib__interrupt_enable (prior);

    /*
     * Signal there is an item on the queue.
     */
    if (sgnl)
	cma_cond_signal (&(queue_obj->insert));


    return ret_val;
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib__aq_augment:  preallocate nodes for a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue_head:	The address of the actual queue header
 *
 *	node_cnt:	The number of queue nodes to allocate
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_lib__aq_augment 
#ifdef _CMA_PROTO_
	(
	cma_lib__t_aq_header	*queue_head,
	cma_t_integer		item_cnt)
#else	/* no prototypes */
	(queue_head, item_cnt)
	cma_lib__t_aq_header	*queue_head;
	cma_t_integer		item_cnt;
#endif	/* prototype */
    {
    cma_lib__t_queue_item    *queue_item;
    cma_t_integer   i;


    for (i = 0; i < item_cnt; i++) {
	/*
	 * Allocate a new node
	 */
	cma_lib__alloc_object(cma_lib__t_queue_item, queue_item);

	/*
	 * Insert the new item on the free nodes list.
	 */
	cma_lib__queue_insert_after ((cma_lib__t_queue *)queue_item, 
				    &(queue_head->free));
	}
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_lib__aq_disperse:  deallocate nodes of a queue
 *
 *  FORMAL PARAMETERS:
 *
 *	queue_head:	The address of the actual queue header
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	Raise an exception if the item queue is not empty.
 *
 *	Note: this routine assumes that caller has locked the queue obj mutex.
 */
extern void
cma_lib__aq_disperse
#ifdef _CMA_PROTO_
	(
	cma_lib__t_aq_header	*queue_head)
#else	/* no prototypes */
	(queue_head)
	cma_lib__t_aq_header	*queue_head;
#endif	/* prototype */
    {
    cma_lib__t_queue_item    *queue_item;
    cma_t_integer  s;


/*
 * FIX-ME: atomically set a "delete-pending" flag which is read by the 
 *	    interrupt-level enqueue routine.  (Or maybe just block signals
 *	    instead...) This will prevent collisions
 *	    from interrupt level code (normal threads are held at bay by
 *	    the object's mutex).  If, after locking this bit, we find the
 *	    queue is busy/non-empty, clear the bit and raise the exception;
 *	    otherwise, leave the bit set until the object is reinitialized
 *	    or destroyed.
 */

    /*
     * If the forward link is non-zero, then there is a item on the queue, in 
     * which case, the queue is not empty and should not be deleted.
     */
    if (queue_head->items.flink != (cma_lib__t_queue *)cma_c_null_ptr) {
	RAISE (cma_e_in_use);
	}

    /*
     * As long as the free list is non-empty, pull off nodes and disperse them.
     */
    while (!cma_lib__queue_empty (&(queue_head->free))) {
	queue_item = (cma_lib__t_queue_item *)
			cma_lib__queue_dequeue (&(queue_head->free));

	/*
	 * Clear and deallocate the memory in the queue item.
	 */
	cma_lib__destroy_object ((cma_lib__t_queue *)queue_item);
	}

    cma_lib__assert_fail (
	    queue_head->items.flink == (cma_lib__t_queue *)cma_c_null_ptr,
	    "cma_lib__aq_disperse: item list not empty at finish");
    cma_lib__assert_fail (
	    queue_head->items.blink == (cma_lib__t_queue *)cma_c_null_ptr,
	    "cma_lib__aq_disperse: item list corrupt at finish");
    cma_lib__assert_fail (
	    queue_head->free.flink == (cma_lib__t_queue *)cma_c_null_ptr,
	    "cma_lib__aq_disperse: free list not empty at finish");
    cma_lib__assert_fail (
	    queue_head->free.blink == (cma_lib__t_queue *)cma_c_null_ptr,
	    "cma_lib__aq_disperse: free list corrupt at finish");
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_OP.C */
/*  *17    1-JUN-1992 14:41:22 BUTENHOF "Modify for new build environment" */
/*  *16    1-APR-1991 12:31:21 CURTIN "change : to ;" */
/*  *15   29-MAR-1991 16:50:34 CURTIN "" */
/*  *14   29-MAR-1991 09:43:11 CURTIN "" */
/*  *13   28-MAR-1991 17:19:36 CURTIN "fix" */
/*  *12   28-MAR-1991 17:16:16 CURTIN "fix" */
/*  *11   27-MAR-1991 16:54:38 CURTIN "Added some signals" */
/*  *10   27-MAR-1991 16:17:22 CURTIN "fix cv waits/signals" */
/*  *9    27-MAR-1991 13:16:19 CURTIN "fixed _requeue" */
/*  *8    26-MAR-1991 14:12:18 CURTIN "added _int interrupt macros" */
/*  *7    25-MAR-1991 15:49:02 CURTIN "Fixed c.v. usage in enqueue/dequeue" */
/*  *6    22-MAR-1991 13:27:49 CURTIN "name change" */
/*  *5    22-MAR-1991 12:22:29 CURTIN "making names consistent" */
/*  *4    22-MAR-1991 12:20:54 CURTIN "changed some names from _ to __" */
/*  *3    20-MAR-1991 16:48:06 CURTIN "added casting for object and queue types" */
/*  *2    29-AUG-1990 17:03:04 SCALES "Convert to stream format" */
/*  *1    27-AUG-1990 02:15:03 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_OP.C */
