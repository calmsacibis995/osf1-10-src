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
static char *rcsid = "@(#)$RCSfile: cmalib_attr.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:43 $";
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
 *	Concert Multithread Architecture (CMA) Library services
 *
 *  ABSTRACT:
 *
 *	Manage library objects attributes objects
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	7 August 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	28 August 1990
 *		Fix parameter passing to pass structures by reference.
 *	002	Webb Scales	29 August 1990
 *		Change calls to CMA to pass structures by reference.
 *		Make get/set attributes initialize the library.
 *	003	Dave Butenhof	12 October 1990
 *		Fix breakage due to CMA perturbations; cma_once needs an
 *		extra argument.
 *	004	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_library.h>
#include <cmalib_defs.h>
#include <cmalib_attr.h>
#include <cmalib_handle.h>
#include <cmalib_seq.h>
#include <cmalib_aq_cre.h>
#include <cmalib_init.h>

/*
 *  GLOBAL DATA
 */

cma_lib__t_int_attr	cma_lib__g_def_attr;

/*
 *  LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

static void
cma_lib___free_cache _CMA_PROTOTYPE_ ((
        cma_lib__t_int_attr	*att,
        cma_t_natural		type));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a new public attributes structure.
 *
 *  FORMAL PARAMETERS:
 *
 *	new_att		Output handle
 *
 *	att		Attributes object to use in creation
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
cma_lib_attr_create
#ifdef _CMA_PROTO_
	(
	cma_lib_t_attr	*new_att,	/* New handle to fill in */
	cma_lib_t_attr	*att)		/* Old attr obj to use */
#else	/* no prototypes */
	(new_att, att)
	cma_lib_t_attr	*new_att;	/* New handle to fill in */
	cma_lib_t_attr	*att;		/* Old attr obj to use */
#endif	/* prototype */
    {
    cma_lib__t_int_attr	*att_obj;	/* Pointer to attributes object */
    cma_lib__t_int_attr	*int_att;
    cma_t_boolean	new;		/* Indicates non-cached obj */
    cma_t_integer	i;


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
     * Get an attributes object, either from the cache in the parent attributes 
     * object or from the memory manager.  Do object-generic initialization.
     */
    cma_lib__get_object(
	    cma_lib__t_int_attr,
	    cma_lib__c_obj_attr,
	    att_obj, 
	    int_att, 
	    new);

    /* 
     * Access attributes object-specific attributes in parent attributes object.
     * (Currently, there are none.)
     */

    /*
     * We are now finished with the parent attributes object: release its mutex.
     */
    cma_mutex_unlock (&(int_att->mutex));

    /*
     * Initialize the newly-gotten attributes object
     *
     */
    att_obj->refcnt = 0;
    att_obj->delete_pending = cma_c_false;

    if (new) {
	cma_t_integer	i;


	/*
	 * This is a brand-new attributes object:  create its mutex and 
	 * initialize its cache queues.
	 */
	cma_mutex_create (&(att_obj->mutex), &cma_c_null);

	for (i = 1; i < cma_lib__c_obj_num; i++)  {
	    att_obj->cache[i].revision = 0;
	    att_obj->cache[i].count = 0;
	    cma_lib__queue_init (&(att_obj->cache[i].queue));
	    }
	}
    else {
	/*
	 * This is a quality pre-owned attributes object, just clean it up.
	 *
	 * Note that we do this when we allocate it, not when we free it:
	 * that way free_attributes pays minimal overhead for caching.
	 *
	 * Primarily, this involves deallocating all objects on the cache
	 * list (since they presumably won't be valid in the attributes
	 * object's new incarnation).
	 */
	for (i = 1; i < cma_lib__c_obj_num; i++)  {
	    att_obj->cache[i].revision = 0;
	    att_obj->cache[i].count = 0;
	    cma_lib___free_cache (att_obj, i);
	    }
	}

    /*
     * Initialize default attributes values.
     */
    att_obj->queue_size	= cma_lib__c_default_queue;	/* Def. queue alloc */

    /*
     * Finally, initialize the user's handle.
     */
    cma_lib__object_to_handle ((cma_lib__t_object *)att_obj, new_att);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete (free) a public attributes structure.
 *
 *  FORMAL PARAMETERS:
 *
 *	att		Attributes object to free
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
cma_lib_attr_delete
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*attr)		/* Address of handle of obj to delete */
#else	/* no prototypes */
	(attr)
	cma_t_attr	*attr;		/* Address of handle of obj to delete */
#endif	/* prototype */
    {
    cma_lib__t_int_attr	*old_int_attr;	/* Address of object to delete */
    cma_lib__t_int_attr	*par_int_attr;	/* Address of att obj to cache obj on */


    /*
     * Ensure that the specified attributes object handle is valid, and extract
     * the address of the attributes object from it.  If the address is null, 
     * simply return (idem potent operation).
     */
    old_int_attr = cma_lib__validate_null_attr (attr);
    if (old_int_attr == (cma_lib__t_int_attr *)cma_c_null_ptr)	return;

    cma_lib__free_object (
	cma_lib__t_int_attr,
	cma_lib__c_obj_attr,
	old_int_attr, 
	par_int_attr, 
	cma_lib__destroy_attributes);

    /*
     * Finally, zero the handle to reduce danger of a "dangling" handle error.
     */
    cma_lib__clear_handle (attr);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get and Set attribute routines
 *
 *  FORMAL PARAMETERS:
 *
 *	att		Attributes object to modify (set) or read (get)
 *
 *	setting		New (set) or Current (get) attribute setting
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

/*
 * Return current setting of "queue size" attribute
 */
extern void
cma_lib_attr_get_queuesize
#ifdef _CMA_PROTO_
	(
	cma_lib_t_attr	*att,		/* Attr obj to use */
	cma_t_natural	*setting)	/* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_lib_t_attr	*att;		/* Attr obj to use */
	cma_t_natural	*setting;	/* Current setting */
#endif	/* prototype */
    {
    cma_lib__t_int_attr	*int_att;


    /*
     * The CMA Library is self initializing.
     *
     * All "create" routines and any other routines which have no dependence
     * on prior CMA Library routine calls must initialize the Library.
     */
    cma_once (&cma_lib__g_init_block, cma_lib__init, cma_c_null_ptr);

    int_att = cma_lib__validate_default_attr (att);
    cma_mutex_lock (&(int_att->mutex));
    *setting = int_att->queue_size;
    cma_mutex_unlock (&(int_att->mutex));
    }


/*
 * Modify current setting of "queue size" attribute
 */
extern void
cma_lib_attr_set_queuesize
#ifdef _CMA_PROTO_
	(
	cma_lib_t_attr	*att,		/* Attr obj to use */
	cma_t_natural	setting)	/* New setting */
#else	/* no prototypes */
	(att, setting)
	cma_lib_t_attr	*att;		/* Attr obj to use */
	cma_t_natural	setting;	/* New setting */
#endif	/* prototype */
    {
    cma_lib__t_int_attr	*int_att;


    /*
     * The CMA Library is self initializing.
     *
     * All "create" routines and any other routines which have no dependence
     * on prior CMA Library routine calls must initialize the Library.
     */
    cma_once (&cma_lib__g_init_block, cma_lib__init, cma_c_null_ptr);

    if (setting <= 0)			/* If non-positive queue size */
	RAISE (cma_e_badparam);		/* . raise error */

    int_att = cma_lib__validate_attr (att);
    cma_mutex_lock (&(int_att->mutex));
    int_att->queue_size = setting;
    cma_lib___free_cache (int_att, cma_lib__c_obj_queue);
    int_att->cache[cma_lib__c_obj_queue].revision++;
    cma_mutex_unlock (&(int_att->mutex));
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Deallocate an attributes structure---don't try to cache it (this is
 *	used to remove attributes from a cache list!)
 *
 *  FORMAL PARAMETERS:
 *
 *	old_attr	Address of the object
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
cma_lib__destroy_attributes
#ifdef _CMA_PROTO_
	(
	cma_lib__t_int_attr	*old_attr)	/* The attr obj to delete */
#else	/* no prototypes */
	(old_attr)
	cma_lib__t_int_attr	*old_attr;	/* The attr obj to delete */
#endif	/* prototype */
    {
    /*
     * Free all the objects which are contained in the attributes obj, and
     * then deallocate the object's memory.  THIS ROUTINE ASSUMES THAT THE
     * PARENT ATTRIBUTES OBJECT IS LOCKED, AND THAT THE CALLER MANAGES THE
     * CACHE LINKS.
     */
    cma_lib__assert_warn (
	    !cma_mutex_try_lock (&(old_attr->attributes->mutex)),
	    "cma_lib__destroy_attr called without attributes object locked.");

    cma_mutex_lock (&(old_attr->mutex));
    cma_lib___free_cache (old_attr, cma_lib__c_obj_attr);
    cma_lib___free_cache (old_attr, cma_lib__c_obj_queue);

    /*
     * If the attributes object has some objects still alive, we can't
     * arbitrarily free the memory (which could have dire consequences when
     * those objects later attempt to lock it and cache themselves).
     * Instead, just mark it as "delete pending", and wait for the objects to
     * go away (the last one will turn the light off).
     */
    if (old_attr->refcnt == 0) {
	cma_mutex_unlock (&(old_attr->mutex));
	cma_mutex_delete (&(old_attr->mutex));
	cma_lib__destroy_object (old_attr);
	}
    else {
	old_attr->delete_pending = cma_c_true;
	cma_mutex_unlock (&(old_attr->mutex));
	}
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize CMA_ATTRIBUTES.C local data
 *
 *  FORMAL PARAMETERS:
 *
 *	none
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
 *	initialize static data
 */
extern void
cma_lib__init_attr
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer   i;


    cma_lib__g_def_attr.header.type	= cma_lib__c_obj_attr;
    cma_lib__g_def_attr.header.sequence	= 1;
    cma_lib__obj_set_name (&cma_lib__g_def_attr, "lib default attr");
    cma_lib__queue_insert (
	    &cma_lib__g_def_attr.header.queue, 
	    &cma_lib__g_known_obj[cma_lib__c_obj_attr].queue);
    cma_mutex_create (&cma_lib__g_def_attr.mutex, &cma_c_null);

    cma_lib__g_def_attr.queue_size = cma_lib__c_default_queue;

    cma_lib__g_def_attr.refcnt = 0;
    cma_lib__g_def_attr.delete_pending = cma_c_false;

    for (i = 1; i < cma_lib__c_obj_num; i++)  {
	cma_lib__g_def_attr.cache[i].revision = 0;
	cma_lib__g_def_attr.cache[i].count = 0;
	cma_lib__queue_init (&cma_lib__g_def_attr.cache[i].queue);
	}

    cma_mutex_lock (&(cma_lib__g_sequence[cma_lib__c_obj_attr].mutex));
    cma_lib__g_sequence[cma_lib__c_obj_attr].seq = 2;
    cma_mutex_unlock (&(cma_lib__g_sequence[cma_lib__c_obj_attr].mutex));
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free all the objects which are hung off an attributes object's cache
 *	list.  This must be done when an attributes object structure is
 *	deleted or reused (since the cached items won't be valid).
 *
 *	THIS ROUTINE ASSUMES THAT THE ATTRIBUTES OBJECT MUTEX IS LOCKED!
 *
 *  FORMAL PARAMETERS:
 *
 *	head	Address of queue head for list
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
static void
cma_lib___free_cache
#ifdef _CMA_PROTO_
	(
	cma_lib__t_int_attr	*att,
	cma_t_natural	type)
#else	/* no prototypes */
	(att, type)
	cma_lib__t_int_attr	*att;
	cma_t_natural	type;
#endif	/* prototype */
    {
    cma_lib__assert_warn (
	    !cma_mutex_try_lock (&(att->mutex)),
	    "cma_lib___free_cache called without attributes object locked.");

    if (cma_lib__queue_empty (&att->cache[type].queue))
	return;				/* Just return if queue is empty */

    while (! cma_lib__queue_empty (&att->cache[type].queue))
	{
	cma_lib__t_object	*item = 
		(cma_lib__t_object *)cma_lib__queue_dequeue (
			&att->cache[type].queue);

	switch (item->type) {
	    case cma_lib__c_obj_attr : {
		cma_lib__destroy_attributes ((cma_lib__t_int_attr *)item);
		break;
		}
	    case cma_lib__c_obj_queue : {
		cma_lib__destroy_queue_obj ((cma_lib__t_queue_obj *)item);
		break;
		}
	    default :
		cma_lib__assert_fail (
			0,
			"Bad type code in object at cma_lib___free_cache.");
	    }

	}

    att->cache[type].count = 0;		/* No more cached items */
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ATTR.C */
/*  *5     1-JUN-1992 14:39:37 BUTENHOF "Modify for new build environment" */
/*  *4    12-OCT-1990 07:11:05 BUTENHOF "Fix for changes in CMA (cma_once, cma$client)" */
/*  *3    29-AUG-1990 17:03:40 SCALES "Change CMA services to by-reference" */
/*  *2    28-AUG-1990 18:34:50 SCALES "Fix parameter passing" */
/*  *1    27-AUG-1990 02:15:28 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_ATTR.C */
