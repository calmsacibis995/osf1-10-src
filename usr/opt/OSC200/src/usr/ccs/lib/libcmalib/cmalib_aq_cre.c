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
static char *rcsid = "@(#)$RCSfile: cmalib_aq_cre.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:01 $";
#endif
/*
 *  Copyright (c) 1990, 1993 by
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
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	10 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	29 August 1990
 *		Change calls to CMA to pass structures by reference.
 *	002	Dave Butenhof	12 October 1990
 *		Fix breakage due to CMA perturbations; cma_once needs an
 *		extra argument.
 *	003	Paul Curtin	19 February 1991
 *		Fix call to *__clear_handle, pass handle instead of obj.
 *	004	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_library.h>
#include <cmalib_defs.h>
#include <cmalib_aq_cre.h>
#include <cmalib_attr.h>
#include <cmalib_seq.h>
#include <cmalib_handle.h>


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


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a queue object
 *
 *  FORMAL PARAMETERS:
 *
 *	new_queue   - Address of queue object handle
 *	att	    - Address of attributes object
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	Queue nodes are allocated and linked together.
 */
extern void
cma_lib_queue_create
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*new_queue,
	cma_lib_t_attr  *att)
#else	/* no prototypes */
	(new_queue, att)
	cma_lib_t_queue	*new_queue;
	cma_lib_t_attr  *att;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_int_attr 	*int_att;	/* Ptr to internal attr obj */
    cma_t_boolean		new;		/* Indicates non-cached obj */
    cma_t_natural		queue_size;	/* Number of queue nodes */


    /*
     * The CMA Library is self initializing.
     *
     * All "create" routines and any other routines which have no dependence
     * on prior CMA Library routine calls must initialize the Library.
     */
    cma_once (&cma_lib__g_init_block, cma_lib__init, cma_c_null_ptr);

    /*
     * Ensure that the specified attributes object handle is valid, and extract
     * the address of the attributes object from it.
     */
    int_att = cma_lib__validate_default_attr (att);

    /*
     * Lock the attributes object before checking the cache and accessing the
     * object attributes
     */
    cma_mutex_lock (&(int_att->mutex));

    /*
     * Get a queue object, either from the cache in the attributes object or
     * from the memory manager.  Do object-generic initialization.
     */
    cma_lib__get_object(
	    cma_lib__t_queue_obj,
	    cma_lib__c_obj_queue,
	    queue_obj, 
	    int_att, 
	    new);

    /* 
     * Access object-specific attributes in attributes object.
     */

    /*
     * Fetch queue size.
     */
    queue_size = int_att->queue_size;

    /*
     * We are now finished with the attributes object:  release its mutex.
     */
    cma_mutex_unlock (&(int_att->mutex));

    /*
     * Initialize the queue object
     *
     * If this is a brand-new object, then create the subordinate objects.
     * Otherwise, only initialize the queue head (just to be sure).
     */
    if (new) {
	cma_lib__aq_init (&(queue_obj->queue_head));
	cma_mutex_create (&(queue_obj->mutex),  &(cma_c_null));
	cma_cond_create  (&(queue_obj->insert), &(cma_c_null));
	cma_cond_create  (&(queue_obj->remove), &(cma_c_null));
	}
    else {
	cma_lib__aq_init (&(queue_obj->queue_head));
	}

    /*
     * Preallocate the queue nodes.
     */
    cma_lib__aq_augment (&(queue_obj->queue_head), queue_size);

    /*
     * Finally, initialize the user's handle.
     */
    cma_lib__object_to_handle ((cma_lib__t_object *)queue_obj, new_queue);
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete a queue object
 *
 *  FORMAL PARAMETERS:
 *
 *	queue_obj   - Address of queue object handle
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
 *	Object may be placed in a cache or freed to the memory pool.
 *	May cause the cache to be flushed.
 */
extern void
cma_lib_queue_delete
#ifdef _CMA_PROTO_
	(
	cma_lib_t_queue	*queue)
#else	/* no prototypes */
	(queue)
	cma_lib_t_queue	*queue;
#endif	/* prototype */
    {
    cma_lib__t_queue_obj	*queue_obj;	/* Ptr to internal Q object */
    cma_lib__t_int_attr 	*int_att;	/* Ptr to internal attr obj */


    /*
     * Ensure that the specified queue object handle is valid, and extract
     * the address of the queue object from it.  If the address is null, 
     * simply return (idem potent operation).
     */
    queue_obj = cma_lib__validate_null_queue_obj (queue);
    if (queue_obj == (cma_lib__t_queue_obj *)cma_c_null_ptr)	return;

/*
 * FIX-ME: test to see if the queue is "in use" before deleting it.
 *
	RAISE (cma_e_in_use);
 */

    /*
     * Free the queue object, either to the cache in the attributes object or
     * to the memory manager.  Flush the cache if we reach the high-water mark.
     */
    cma_lib__free_object (
	    cma_lib__t_queue_obj,
	    cma_lib__c_obj_queue,
	    queue_obj,
	    int_att,
	    cma_lib__destroy_queue_obj);

    /*
     * Finally, zero the handle to reduce danger of a "dangling" handle error.
     */
    cma_lib__clear_handle (queue);
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Deallocate a queue object -- don't try to cache it (this is
 *	used to remove objects from a cache list!)
 *
 *  FORMAL PARAMETERS:
 *
 *	old_queue 	Address of the object
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_lib__destroy_queue_obj
#ifdef _CMA_PROTO_
	(
	cma_lib__t_queue_obj	*queue_obj)	/* The attr obj to delete */
#else	/* no prototypes */
	(queue_obj)
	cma_lib__t_queue_obj	*queue_obj;	/* The attr obj to delete */
#endif	/* prototype */
    {
    /*
     * Lock out other threads during the deletion.
     */
    cma_mutex_lock (&(queue_obj->mutex));

    /*
     * Destroy the queue nodes in the free list.  (If there are any nodes in
     * the item list, this routine will raise an exception; let it fly past.)
     */
    cma_lib__aq_disperse (&(queue_obj->queue_head));

    /*
     * If other threads are waiting on the condition variables, we'll get an
     * exception when we try to delete them.  Let it fly to the user.
     */
    cma_cond_delete (&(queue_obj->insert));
    cma_cond_delete (&(queue_obj->remove));

    /*
     * OK, the queue is pretty much gone, now.  Unlock and delete the mutex.
     */
    cma_mutex_unlock (&(queue_obj->mutex));
    cma_mutex_delete (&(queue_obj->mutex));

    /*
     * Nothing left but a hulk.  Destroy it.
     */
    cma_lib__destroy_object (queue_obj);
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_CREDEL.C */
/*  *6     1-JUN-1992 14:39:24 BUTENHOF "Modify for new build environment" */
/*  *5    20-FEB-1991 15:18:37 CURTIN "made a fix; pass handle instead of obj" */
/*  *4    12-OCT-1990 07:11:01 BUTENHOF "Fix for changes in CMA (cma_once, cma$client)" */
/*  *3    29-AUG-1990 17:09:17 SCALES "Convert to stream" */
/*  *2    29-AUG-1990 17:02:55 SCALES "Convert to stream format" */
/*  *1    27-AUG-1990 02:14:55 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_AQ_CREDEL.C */
