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
 *	@(#)$RCSfile: cma_handle.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/08/06 17:37:10 $
 */

/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	Header file for handles
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	20 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	25 August 1989
 *		Modify cma__validate_handle to return internal object
 *		pointer; check object type.
 *	002	Dave Butenhof	15 September 1989
 *		Define symbol for number of object types.
 *	003	Dave Butenhof	19 October 1989
 *		Add macros to encapsulate validation calls, and cast the
 *		result pointer to the appropriate type.
 *	004	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	005	Bob Conti	14 September 1990
 *		Move pointer to be first in the handle so users can
 *		more easily find it to pass it to debugger commands.
 *	006	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	007	Dave Butenhof	10 February 1992
 *		Add new macros for status-returning validation functions (so
 *		that pthread code doesn't have to use TRY).
 *	008	Dave Butenhof	19 February 1992
 *		Fix type cast errors in 007
 */

#ifndef CMA_HANDLE
#define CMA_HANDLE

/*
 *  INCLUDE FILES
 */

#include <cma_defs.h>
#include <cma_attr.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma__validate_attr(handle) \
    ((cma__t_int_attr *)cma__validate_handle ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_attr))

#define cma__validate_cv(handle) \
    ((cma__t_int_cv *)cma__validate_handle ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_cv))

#define cma__validate_mutex(handle) \
    ((cma__t_int_mutex *)cma__validate_handle ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_mutex))

#define cma__validate_tcb(handle) \
    ((cma__t_int_tcb *)cma__validate_handle ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_tcb))

#define cma__validate_stack(handle) \
    ((cma__t_int_stack *)cma__validate_handle ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_stack))

#define cma__validate_null_attr(handle) \
    ((cma__t_int_attr *)cma__validate_handle_null ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_attr))

#define cma__validate_null_cv(handle) \
    ((cma__t_int_cv *)cma__validate_handle_null ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_cv))

#define cma__validate_null_mutex(handle) \
    ((cma__t_int_mutex *)cma__validate_handle_null ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_mutex))

#define cma__validate_null_tcb(handle) \
    ((cma__t_int_tcb *)cma__validate_handle_null ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_tcb))

#define cma__validate_null_stack(handle) \
    ((cma__t_int_stack *)cma__validate_handle_null ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_stack))

#define cma__val_attr_stat(handle,obj) \
    cma__val_hand_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_attr, \
	    (cma__t_object **)obj)

#define cma__val_cv_stat(handle,obj) \
    cma__val_hand_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_cv, \
	    (cma__t_object **)obj)

#define cma__val_mutex_stat(handle,obj) \
    cma__val_hand_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_mutex, \
	    (cma__t_object **)obj)

#define cma__val_tcb_stat(handle) \
    cma__val_hand_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_tcb, \
	    (cma__t_object **)obj)

#define cma__val_stack_stat(handle,obj) \
    cma__val_hand_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_stack, \
	    (cma__t_object **)obj)

#define cma__val_nullattr_stat(handle,obj) \
    cma__val_handnull_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_attr, \
	    (cma__t_object **)obj)

#define cma__val_nullcv_stat(handle,obj) \
    cma__val_handnull_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_cv, \
	    (cma__t_object **)obj)

#define cma__val_nullmutex_stat(handle,obj) \
    cma__val_handnull_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_mutex, \
	    (cma__t_object **)obj)

#define cma__val_nulltcb_stat(handle,obj) \
    cma__val_handnull_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_tcb, \
	    (cma__t_object **)obj)

#define cma__val_nullstack_stat(handle) \
    cma__val_handnull_stat ( \
	    (cma_t_handle *)(handle), \
	    cma__c_obj_stack, \
	    (cma__t_object **)obj)

/*
 * TYPEDEFS
 */

/*
 * Internal format of a handle (to the outside world it's an array of two
 * addresses, but we know better).
 */
typedef struct CMA__T_INT_HANDLE {
    cma__t_object	*pointer;	/* Address of internal structure */
    cma__t_short	sequence;	/* Sequence number of object */
    cma__t_short	type;		/* Type code of object */
    } cma__t_int_handle;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

extern void
cma__clear_handle _CMA_PROTOTYPE_ ((		/* Clear a handle */
	cma_t_handle	*handle));	/*  Handle to clear */

extern void
cma__object_to_handle _CMA_PROTOTYPE_ ((	/* Create handle for an object */
	cma__t_object	*addr,		/*  Address of object struct */
	cma_t_handle	*handle));	/*  Address of handle */

extern cma__t_int_attr *
cma__validate_default_attr _CMA_PROTOTYPE_ ((	/* Validate attributes obj handle */
	cma_t_handle	*handle));	/*  Handle to validate */

extern cma_t_status
cma__val_defattr_stat _CMA_PROTOTYPE_ ((
	cma_t_handle	*handle,	/*  Handle to validate */
	cma__t_int_attr	**attr));

extern cma__t_object *
cma__validate_handle _CMA_PROTOTYPE_ ((	/* Validate a handle */
	cma_t_handle	*handle,	/*  Handle to validate */
	cma_t_natural	type));		/*  Expected type of object */

extern cma_t_status
cma__val_hand_stat _CMA_PROTOTYPE_ ((	/* Validate a handle */
	cma_t_handle	*handle,	/*  Handle to validate */
	cma_t_natural	type,		/*  Expected type of object */
	cma__t_object	**obj));

extern 	cma__t_object	*
cma__validate_handle_null _CMA_PROTOTYPE_ ((	/* Validate a handle (allow null) */
	cma_t_handle	*handle,	/*  Handle to validate */
	cma_t_natural	type));		/*  Expected type of object */

extern cma_t_status
cma__val_handnull_stat _CMA_PROTOTYPE_ ((	/* Validate a handle (allow null) */
	cma_t_handle	*handle,	/*  Handle to validate */
	cma_t_natural	type,		/*  Expected type of object */
	cma__t_object	**obj));

#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_HANDLE.H */
/*  *7    19-FEB-1992 07:16:14 BUTENHOF "Fix type cast errors" */
/*  *6    18-FEB-1992 15:29:03 BUTENHOF "Make status-returning variants of validate functions" */
/*  *5    10-JUN-1991 19:53:22 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:20:53 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:21:58 SCALES "Add sccs headers for Ultrix" */
/*  *2    14-DEC-1990 00:55:39 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:45:59 BUTENHOF "General handle support" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_HANDLE.H */
