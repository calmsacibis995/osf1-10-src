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
 * @(#)$RCSfile: cmalib_defs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:53 $
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
 *	Concert Multithread Architecture (CMA) Library services
 *
 *  ABSTRACT:
 *
 *	General CMA Library definitions
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
 *	001	Webb Scales	29 August 1990
 *		Change calls to CMA to pass structures by reference.
 *	002	Dave Butenhof	12 October 1990
 *		Change definition of cma_lib__init for new cma_once.
 *	003	Webb Scales	30 October 1990
 *		Make object name field unconditional.
 *	004	Paul Curtin	1 April 1990
 *		Quadword aligned queue allocations.
 *	005	Paul Curtin & Webb Scales   20 April 1992
 *		Rework cma_lib__free_object to better handle 
 *		attributes objects with existing references, etc.
 *	006	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 *	007	Paul Curtin	13 July 1992
 *		Fixed cma__lib_free_object macro if/else nesting.
 *	008	Paul Curtin	27 August 1992
 *		Created variant cma_lib__alloc_object macro for
 *		Alpha/OSF, it octaword aligns.
 */


#ifndef CMALIB_DEFS
#define CMALIB_DEFS

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_init.h>
#include <cmalib_queue.h>


/*
 * CONSTANTS AND MACROS
 */

/*
 * Enumerated type for objects (only, defined as constants since ENUMS can't
 * be stored in a word; starting after ten to leave room for Core objects).
 */
#define cma_lib__c_obj_attr 	1
#define cma_lib__c_obj_queue	2
/*
 * Define the number of object classes.  Note that it's actually one HIGHER
 * than the maximum object number... this allows an array to be indexed by
 * the actual type constants (which start at 1).  Too bad C doesn't allow
 * non-0-based array indices.
 */
#define cma_lib__c_obj_num	3


/*
 * Macro to allocate an object (and to hide the memory allocator)
 *
 * Note: this rounds up to quadword alignment, and insures that there
 *	 are four bytes behind the quadword aligned address to store
 *	 the memory segments real address (that returned by malloc).
 */


#if _CMA_PLATFORM_ == _CMA__ALPHA_UNIX
# define cma_lib__alloc_object(otype,ptr) \
    ptr = (otype *)(malloc((sizeof(otype))+31)); \
    *((cma_t_address *)(((((((cma_t_natural)ptr)+16)+15)/16)*16)-8)) = ((cma_t_address)ptr); \
    ptr = (otype *)((((((cma_t_natural)ptr)+16)+15)/16)*16)
#else
# define cma_lib__alloc_object(otype,ptr) \
    ptr = (otype *)(malloc((sizeof(otype))+15)); \
    *((cma_t_address *)(((((((cma_t_natural)ptr)+8)+7)/8)*8)-4)) = ((cma_t_address)ptr); \
    ptr = (otype *)((((((cma_t_natural)ptr)+8)+7)/8)*8)
#endif

/*
 * Macro to clear an object's storage when it is cached
 */
#ifdef NDEBUG
#define cma_lib__clear_object(object)
#else
#define cma_lib__clear_object(object) \
    memset ((cma_t_address)object, 0, sizeof *object)
#endif

/*
 * Macro to deallocate storage (and to hide the memory allocator)
 *
 * Return the real address of the memory segment which was stored
 * four bytes back of the quadword aligned address by 
 * cma_lib__alloc_object.
 */
#define cma_lib__dealloc_object(optr) \
    free(*((cma_t_address *)(((cma_t_natural)optr)-4)))


/*
 * Macro to destroy an object (ie, free to memory pool)
 */
#define cma_lib__destroy_object(obj_ptr)    {	\
    cma_lib__clear_object (obj_ptr);		\
    cma_lib__dealloc_object (obj_ptr);	\
    }

/*
 * Macro to free an object to cache (or, if the cache overflows, to memory pool)
 *
 * obj_type : typedef	 - object type to get (for allocation, type cast, etc.)
 * obj_num  : constant	 - type number, used in object header and cache index
 * obj_ptr  : variable	 - assigned address of new object
 * int_att  : expression - evaluated for address of attributes object
 * destroy  : routine	 - called to destroy object (instead of caching it)
 */
#define cma_lib__free_object(obj_type, obj_num, obj_ptr, int_att, destroy) \
    {  \
    cma_mutex_lock (&(cma_lib__g_known_obj[obj_num].mutex));  \
    cma_lib__queue_remove (&obj_ptr->header.queue);  \
    cma_mutex_unlock (&(cma_lib__g_known_obj[obj_num].mutex));  \
    int_att = obj_ptr->attributes;  \
    cma_mutex_lock (&(int_att->mutex));  \
    if (obj_num == cma_lib__c_obj_attr) cma_mutex_lock (&(obj_ptr->mutex)); \
    if ((obj_num == cma_lib__c_obj_attr) \
	    && (((cma_lib__t_int_attr *)obj_ptr)->refcnt > 0)) { \
	((cma_lib__t_int_attr *)obj_ptr)->delete_pending = cma_c_true; \
	cma_mutex_unlock (&(obj_ptr->mutex)); \
	cma_mutex_unlock (&(int_att->mutex));  \
	} \
    else { \
	obj_ptr->header.sequence = 0;  \
	if (obj_num == cma_lib__c_obj_attr)  \
	    cma_mutex_unlock (&(obj_ptr->mutex));  \
	int_att->refcnt--;  \
	if ((obj_ptr->header.revision == int_att->cache[obj_num].revision) \
		&& (! int_att->delete_pending)) {  \
	    if (int_att->cache[obj_num].count  \
		    < cma_lib__g_env[(obj_num-1)*2].value) {  \
		int_att->cache[obj_num].count += 1;  \
		cma_lib__queue_insert (  \
			&obj_ptr->header.queue,  \
			&int_att->cache[obj_num].queue);  \
		}  \
	    else {  \
		destroy (obj_ptr);  \
		while ((int_att->cache[obj_num].count  \
			> cma_lib__g_env[(obj_num-1)*2+1].value)  \
			&& (! cma_lib__queue_empty (  \
				&int_att->cache[obj_num].queue))) {\
		    obj_type *item;  \
		    item = (obj_type *)cma_lib__queue_dequeue (  \
			    &int_att->cache[obj_num].queue);  \
		    int_att->cache[obj_num].count -= 1;  \
		    destroy (item);  \
		    }  \
		}  \
	    }  \
	else  \
	    destroy (obj_ptr);  \
	if ((int_att->refcnt == 0) && (int_att->delete_pending)) {  \
	    cma_lib__t_int_attr	*par_att;  \
	    cma_mutex_unlock (&(int_att->mutex));  \
	    par_att = int_att->attributes;  \
	    cma_mutex_lock (&(par_att->mutex));  \
	    cma_lib__destroy_attributes (int_att);  \
	    cma_mutex_unlock (&(par_att->mutex));  \
	    }  \
	else  \
	    cma_mutex_unlock (&(int_att->mutex));  \
	} \
    }

/*
 * Macro to get a new object either from cache or from memory pool
 *
 * obj_type : typedef	 - object type to get (for allocation, type cast, etc.)
 * obj_num  : constant	 - type number, used in object header and cache index
 * obj_ptr  : variable	 - assigned address of new object
 * int_att  : expression - evaluated for address of attributes object
 * obj_new  : variable	 - assigned boolean indicating a new, non-cached object
 *
 * Assumes that caller has locked the attributes object mutex.
 */
#define cma_lib__get_object(obj_type, obj_num, obj_ptr, int_att, obj_new) \
    { \
    if (obj_new = cma_lib__queue_empty (&((int_att)->cache[obj_num].queue))) { \
	TRY { \
	    cma_lib__alloc_object (obj_type, obj_ptr); \
	    obj_ptr->header.type = obj_num; \
	    obj_ptr->attributes = int_att; \
	    } \
	CATCH_ALL { \
	    cma_mutex_unlock (&((int_att)->mutex)); \
	    RERAISE; \
	    } \
	ENDTRY \
	} \
    else { \
	obj_ptr = (obj_type *)cma_lib__queue_dequeue ( \
		&((int_att)->cache[obj_num].queue)); \
	(int_att)->cache[obj_num].count--; \
	} \
    (int_att)->refcnt++; \
    obj_ptr->header.sequence = cma_lib__assign_sequence ( \
	    &cma_lib__g_sequence[obj_num]); \
    obj_ptr->header.revision = (int_att)->cache[obj_num].revision; \
    cma_lib__obj_clear_name (obj_ptr); \
    cma_mutex_lock (&(cma_lib__g_known_obj[obj_num].mutex)); \
    cma_lib__queue_insert ( \
	    &(obj_ptr->header.queue), \
	    &cma_lib__g_known_obj[obj_num].queue); \
    cma_mutex_unlock (&(cma_lib__g_known_obj[obj_num].mutex)); \
    }

/*
 * Set the name of an object
 */
#ifdef NDEBUG
# define cma_lib__obj_set_name(o,string)
#else
# define cma_lib__obj_set_name(o,string) \
	(((cma_lib__t_object *)(o))->name = (string))
#endif

/*
 * Clear the name of an object
 */
#ifdef NDEBUG
# define cma_lib__obj_clear_name(o)
#else
# define cma_lib__obj_clear_name(o) \
    (((cma_lib__t_object *)(o))->name = (cma_lib__t_string)cma_c_null_ptr)
#endif

/*
 * Test whether name is null
 */
#ifdef NDEBUG
# define cma_lib__obj_null_name(o) (cma_c_true)
#else
# define cma_lib__obj_null_name(o) \
    (((cma_lib__t_object *)(o))->name == (cma_lib__t_string)cma_c_null_ptr)
#endif


/*
 * TYPEDEFS
 */

typedef char *		cma_lib__t_string;
typedef short int	cma_lib__t_short;

typedef struct CMA_LIB__T_OBJECT {
    cma_lib__t_queue	queue;		/* Queue element MUST BE FIRST */
    cma_lib__t_short	sequence;	/* Sequence number */
    cma_lib__t_short	type;		/* Type of object */
    cma_t_natural	revision;	/* Revision count of attr. obj */
    cma_lib__t_string	name;		/* Name of object for debugging */
    } cma_lib__t_object;

typedef struct CMA_LIB__T_KNOWN_OBJECT {
    cma_lib__t_queue    queue;          /* Queue header for known objects */
    cma_t_mutex		mutex;		/* Mutex to control access to queue */
    } cma_lib__t_known_object;


/*
 * Initialization Definitions (referenced in almost every module)
 */
extern cma_t_once		cma_lib__g_init_block;
extern cma_lib__t_known_object	cma_lib__g_known_obj[cma_lib__c_obj_num];

extern void
cma_lib__init _CMA_PROTOTYPE_ ((
	cma_t_address	arg));

/*
 * IMPLICIT INCLUDE FILES FOR ALL MODULES
 */
#include <cmalib_assert.h>

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_DEFS.H */
/*  *10   27-AUG-1992 16:51:39 CURTIN "Added octaword alignment to cma_lib__alloc_object for alpha/osf" */
/*  *9    13-JUL-1992 14:10:00 CURTIN "Fixed cma__lib_free_object macro's if/else nesting" */
/*  *8     2-JUN-1992 07:10:36 BUTENHOF "Fix header file name" */
/*  *7     1-JUN-1992 14:39:50 BUTENHOF "Modify for new build environment" */
/*  *6    20-APR-1992 11:57:16 CURTIN "Rework cma_lib__free_object" */
/*  *5     1-APR-1991 11:49:10 CURTIN "Made queue obj allocations quadword aligned" */
/*  *4    30-OCT-1990 21:59:18 SCALES "Make name-string pointer unconditional" */
/*  *3    12-OCT-1990 07:11:11 BUTENHOF "Fix definition of cma_lib_init" */
/*  *2    29-AUG-1990 17:03:47 SCALES "Convert to stream format" */
/*  *1    27-AUG-1990 02:15:35 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_DEFS.H */
