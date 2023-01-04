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
 * @(#)$RCSfile: cmalib_handle.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/16 21:58:58 $
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
 *	Header file for handles (basically a direct copy of CMA_HANDLE.H)
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
 *	001	Dave Butenhof	01 June 1992
 *		Modify for new build environment
 */


#ifndef CMALIB_HANDLE
#define CMALIB_HANDLE

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cmalib_defs.h>
#include <cmalib_attr.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma_lib__clear_handle(handle)				\
	((cma_lib__t_int_handle *)(handle))->sequence = 0;	\
	((cma_lib__t_int_handle *)(handle))->type     = 0;	\
	((cma_lib__t_int_handle *)(handle))->pointer  =		\
		(cma_lib__t_object *)cma_c_null_ptr;

#define cma_lib__object_to_handle(addr, handle)				  \
	((cma_lib__t_int_handle *)(handle))->sequence = (addr)->sequence; \
	((cma_lib__t_int_handle *)(handle))->type     = (addr)->type;	  \
	((cma_lib__t_int_handle *)(handle))->pointer  = (addr);

#define cma_lib__validate_attr(handle)			\
    ((cma_lib__t_int_attr *)cma_lib__validate_handle (	\
	    (cma_t_handle *)(handle),			\
	    cma_lib__c_obj_attr))

#define cma_lib__validate_default_attr(handle)				\
    ((handle) == (cma_t_handle *)cma_c_null_ptr				\
	? (RAISE (cma_e_existence),					\
		(cma_lib__t_int_attr *)cma_c_null_ptr)			\
	: (((cma_lib__t_int_handle *)(handle))->pointer ==		\
		(cma_lib__t_object *)cma_c_null_ptr			\
	    ? (((cma_lib__t_int_handle *)(handle))->sequence == 0	\
		    && ((cma_lib__t_int_handle *)(handle))->type == 0	\
		? &cma_lib__g_def_attr					\
		: (RAISE (cma_e_existence),				\
			(cma_lib__t_int_attr *)cma_c_null_ptr))		\
	    : (((cma_lib__t_int_handle *)(handle))->sequence !=		\
		    ((cma_lib__t_int_handle *)(handle))->pointer->sequence \
		? (RAISE (cma_e_existence),				\
			(cma_lib__t_int_attr *)cma_c_null_ptr)		\
		: (((cma_lib__t_int_handle *)(handle))->type !=		\
			((cma_lib__t_int_handle *)(handle))->pointer->type \
		    ? (RAISE (cma_e_existence),				\
			    (cma_lib__t_int_attr *)cma_c_null_ptr)	\
		    : (((cma_lib__t_int_handle *)(handle))->type !=	\
			    cma_lib__c_obj_attr				\
			? (RAISE (cma_e_use_error),			\
				(cma_lib__t_int_attr *)cma_c_null_ptr)	\
			: (cma_lib__t_int_attr *)			\
				((cma_lib__t_int_handle *)(handle))->pointer \
			)))))

#define cma_lib__validate_handle(handle, otype)				\
    ((handle) == (cma_t_handle *)cma_c_null_ptr				\
	? (RAISE (cma_e_existence),					\
		(cma_lib__t_object *)cma_c_null_ptr)			\
	: (((cma_lib__t_int_handle *)(handle))->pointer ==		\
		(cma_lib__t_object *)cma_c_null_ptr			\
	    ? (RAISE (cma_e_existence),					\
		    (cma_lib__t_object *)cma_c_null_ptr)		\
	    : (((cma_lib__t_int_handle *)(handle))->sequence !=		\
		    ((cma_lib__t_int_handle *)(handle))->pointer->sequence \
		? (RAISE (cma_e_existence),				\
			(cma_lib__t_object *)cma_c_null_ptr)		\
		: (((cma_lib__t_int_handle *)(handle))->type !=		\
			((cma_lib__t_int_handle *)(handle))->pointer->type \
		    ? (RAISE (cma_e_existence),				\
			    (cma_lib__t_object *)cma_c_null_ptr)	\
		    : (((cma_lib__t_int_handle *)(handle))->type != (otype) \
			? (RAISE (cma_e_use_error),			\
				(cma_lib__t_object *)cma_c_null_ptr)	\
			: ((cma_lib__t_int_handle *)(handle))->pointer	\
			)))))

#define cma_lib__validate_handle_null(handle, otype)			\
    ((handle) == (cma_t_handle *)cma_c_null_ptr				\
	? (RAISE (cma_e_existence),					\
		(cma_lib__t_object *)cma_c_null_ptr)			\
	: (((cma_lib__t_int_handle *)(handle))->pointer ==		\
		(cma_lib__t_object *)cma_c_null_ptr			\
	    ? (((cma_lib__t_int_handle *)(handle))->sequence == 0	\
		    && ((cma_lib__t_int_handle *)(handle))->type == 0	\
		? (cma_lib__t_object *)cma_c_null_ptr			\
		: (RAISE (cma_e_existence),				\
			(cma_lib__t_object *)cma_c_null_ptr))		\
	    : (((cma_lib__t_int_handle *)(handle))->sequence !=		\
		    ((cma_lib__t_int_handle *)(handle))->pointer->sequence \
		? (RAISE (cma_e_existence),				\
			(cma_lib__t_object *)cma_c_null_ptr)		\
		: (((cma_lib__t_int_handle *)(handle))->type !=		\
			((cma_lib__t_int_handle *)(handle))->pointer->type \
		    ? (RAISE (cma_e_existence),				\
			    (cma_lib__t_object *)cma_c_null_ptr)	\
		    : (((cma_lib__t_int_handle *)(handle))->type != (otype) \
			? (RAISE (cma_e_use_error),			\
				(cma_lib__t_object *)cma_c_null_ptr)	\
			: ((cma_lib__t_int_handle *)(handle))->pointer	\
			)))))

#define cma_lib__validate_queue_obj(handle)			\
    ((cma_lib__t_queue_obj *)cma_lib__validate_handle (		\
	    (cma_t_handle *)(handle),				\
	    cma_lib__c_obj_queue))

#define cma_lib__validate_null_attr(handle)			\
    ((cma_lib__t_int_attr *)cma_lib__validate_handle_null (	\
	    (cma_t_handle *)(handle),				\
	    cma_lib__c_obj_attr))

#define cma_lib__validate_null_queue_obj(handle)		\
    ((cma_lib__t_queue_obj *)cma_lib__validate_handle_null (	\
	    (cma_t_handle *)(handle),				\
	    cma_lib__c_obj_queue))

/*
 * TYPEDEFS
 */

/*
 * Internal format of a handle (to the outside world it's an array of two
 * addresses, but we know better).
 */
typedef struct CMA_LIB__T_INT_HANDLE {
    cma_lib__t_short	sequence;	/* Sequence number of object */
    cma_lib__t_short	type;		/* Type code of object */
    cma_lib__t_object	*pointer;	/* Address of internal structure */
    } cma_lib__t_int_handle;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#ifdef the_macro_didnt_work
extern cma_lib__t_int_attr *
cma_lib__validate_default_attr _CMA_PROTOTYPE_ ((   /* Validate attributes obj handle */
	cma_t_handle	*handle));	/*  Handle to validate */

extern cma_lib__t_object *
cma_lib__validate_handle _CMA_PROTOTYPE_ ((	/* Validate a handle */
	cma_t_handle	*handle,	/*  Handle to validate */
	cma_t_natural	type));		/*  Expected type of object */

extern cma_lib__t_object *
cma_lib__validate_handle_null _CMA_PROTOTYPE_ (( /* Validate a handle (allow null) */
	cma_t_handle	*handle,	/*  Handle to validate */
	cma_t_natural	type));		/*  Expected type of object */
#endif

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_HANDLE.H */
/*  *2     1-JUN-1992 14:39:53 BUTENHOF "Modify for new build environment" */
/*  *1    27-AUG-1990 02:15:39 SCALES "" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMALIB_HANDLE.H */
