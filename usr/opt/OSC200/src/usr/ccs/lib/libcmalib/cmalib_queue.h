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
/*
 * @(#)$RCSfile: cmalib_queue.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:59:18 $
 */
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
 *	Header file for generic queuing functions operating on circular
 *	double-linked lists.  (Copied directly from CMA_QUEUE.H)
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	8 August 1990
 *
 *  MODIFICATION HISTORY:
 * 
 *	001	Paul Curtin	20 February 1991
 *		Removed cma_lib__queue_dequeue macro.
 *	002	Paul Curtin	06 March 1991
 *		Added proto for queue_dequeue.
 *	003	Paul Curtin	07 March 1991
 *		Fixed proto for queue_dequeue, sigh.
 */


#ifndef CMALIB_QUEUE
#define CMALIB_QUEUE

/*
 *  INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

/*
 * Test whether a queue is empty.  Return cma_c_true if so, else
 * cma_c_false.
 */
#define cma_lib__queue_empty(head) (					\
    cma_lib__assert_fail (						\
	    (head) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_empty: null queue head"),			\
    cma_lib__assert_fail (						\
	    (((head)->flink != (cma_lib__t_queue *)cma_c_null_ptr)	\
	    && ((head)->blink != (cma_lib__t_queue *)cma_c_null_ptr)),	\
	    "cma_lib__queue_empty: uninitialized queue head"),		\
    cma_lib__assert_fail (						\
	    (head)->blink->flink == (head),				\
	    "cma_lib__queue_empty: queue corruption"),			\
    cma_lib__assert_fail (						\
	    (head)->flink->blink == (head),				\
	    "cma_lib__queue_empty: queue corruption"),			\
    (head)->flink == (head))

/*
 * Initialize a queue header to empty.  (Note that the comma operator is used
 * merely to avoid the necessity for a block, not because a return value is
 * actually useful).
 */
#define cma_lib__queue_init(head)  \
    ((head)->flink = (head), (head)->blink = (head))

/*
 * Insert an element in a queue preceding the specified item (or at end of
 * queue if "queue" is the queue head).
 */
#define cma_lib__queue_insert(element,queue) (				\
    cma_lib__assert_fail (						\
	    (queue) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_insert: null queue head"),			\
    cma_lib__assert_fail (						\
	    (element) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_insert: null queue element"),		\
    cma_lib__assert_fail (						\
	    (((queue)->flink != (cma_lib__t_queue *)cma_c_null_ptr)	\
	    && ((queue)->blink != (cma_lib__t_queue *)cma_c_null_ptr)),	\
	    "cma_lib__queue_insert: uninitialized queue head"),		\
    cma_lib__assert_fail (						\
	    (queue)->blink->flink == (queue),				\
	    "cma_lib__queue_insert: queue corruption"),			\
    cma_lib__assert_fail (						\
	    (queue)->flink->blink == (queue),				\
	    "cma_lib__queue_insert: queue corruption"),			\
    (element)->blink		= (queue)->blink,			\
    (element)->flink		= (queue),				\
    (queue)->blink->flink	= (element),				\
    (queue)->blink		= (element))

/*
 * Insert an element in a queue following the specified item (or at head of
 * queue if "queue" is the queue head).
 */
#define cma_lib__queue_insert_after(element,queue) (			\
    cma_lib__assert_fail (						\
	    (queue) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_insert_after: null queue head"),		\
    cma_lib__assert_fail (						\
	    (element) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_insert_after: null queue element"),		\
    cma_lib__assert_fail (						\
	    (((queue)->flink != (cma_lib__t_queue *)cma_c_null_ptr)	\
	    && ((queue)->blink != (cma_lib__t_queue *)cma_c_null_ptr)),	\
	    "cma_lib__queue_insert_after: uninitialized queue head"),	\
    cma_lib__assert_fail (						\
	    (queue)->blink->flink == (queue),				\
	    "cma_lib__queue_insert_after: queue corruption"),		\
    cma_lib__assert_fail (						\
	    (queue)->flink->blink == (queue),				\
	    "cma_lib__queue_insert_after: queue corruption"),		\
    (element)->flink		= (queue)->flink,			\
    (element)->blink		= (queue),				\
    (queue)->flink->blink	= (element),				\
    (queue)->flink		= (element))

/*
 * Return the next item in a queue (or the first, if the address is of the
 * queue header)
 */
#define cma_lib__queue_next(element) (					\
    cma_lib__assert_fail (						\
	    (element) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_next: null queue element"),			\
    cma_lib__assert_fail (						\
	    (((element)->flink != (cma_lib__t_queue *)cma_c_null_ptr)	\
	    && ((element)->blink != (cma_lib__t_queue *)cma_c_null_ptr)), \
	    "cma_lib__queue_next: uninitialized queue pointers"),	\
    cma_lib__assert_fail (						\
	    (element)->blink->flink == (element),			\
	    "cma_lib__queue_next: queue corruption"),			\
    cma_lib__assert_fail (						\
	    (element)->flink->blink == (element),			\
	    "cma_lib__queue_next: queue corruption"),			\
    (element)->flink)

/*
 * Return the previous item in a queue (or the last, if the address is of the
 * queue header)
 */
#define cma_lib__queue_previous(element) (				\
    cma_lib__assert_fail (						\
	    (element) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_previous: null queue element"),		\
    cma_lib__assert_fail (						\
	    (((element)->flink != (cma_lib__t_queue *)cma_c_null_ptr)	\
	    && ((element)->blink != (cma_lib__t_queue *)cma_c_null_ptr)), \
	    "cma_lib__queue_previous: uninitialized queue pointers"),	\
    cma_lib__assert_fail (						\
	    (element)->blink->flink == (element),			\
	    "cma_lib__queue_previous: queue corruption"),		\
    cma_lib__assert_fail (						\
	    (element)->flink->blink == (element),			\
	    "cma_lib__queue_previous: queue corruption"),		\
    (element)->blink)

/*
 * Remove the specified item from a queue.  Return a pointer to the element.
 */
#define cma_lib__queue_remove(element) (				\
    cma_lib__assert_fail (						\
	    (element) != (cma_lib__t_queue *)cma_c_null_ptr,		\
	    "cma_lib__queue_remove: null queue element"),		\
    cma_lib__assert_fail (						\
	    (((element)->flink != (cma_lib__t_queue *)cma_c_null_ptr)	\
	    && ((element)->blink != (cma_lib__t_queue *)cma_c_null_ptr)), \
	    "cma_lib__queue_remove: uninitialized queue"),		\
    cma_lib__assert_fail (						\
	    (element)->blink->flink == (element),			\
	    "cma_lib__queue_remove: queue corruption"),			\
    cma_lib__assert_fail (						\
	    (element)->flink->blink == (element),			\
	    "cma_lib__queue_remove: queue corruption"),			\
    (element)->flink->blink = (element)->blink,				\
    (element)->blink->flink = (element)->flink,				\
    (element))

/*
 * TYPEDEFS
 */

typedef struct cma_lib__T_QUEUE {
    struct cma_lib__T_QUEUE	*flink;		/* Forward link */
    struct cma_lib__T_QUEUE	*blink;		/* Backward link */
    } cma_lib__t_queue;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */
extern cma_lib__t_queue *
cma_lib__queue_dequeue _CMA_PROTOTYPE_ ((
	cma_lib__t_queue	    *head
	));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_QUEUE.H */
/*  *6     1-JUN-1992 14:40:07 BUTENHOF "Modify for new build environment" */
/*  *5    15-MAR-1991 12:30:42 CURTIN "fixed the queue_dequeue proto " */
/*  *4     7-MAR-1991 10:16:14 CURTIN "fixed queue_dequeue proto" */
/*  *3     6-MAR-1991 12:22:45 CURTIN "added proto for cma_lib__queue_dequeue" */
/*  *2    20-FEB-1991 15:19:05 CURTIN "removed a queue_dequeue macro" */
/*  *1    27-AUG-1990 02:15:47 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_QUEUE.H */
