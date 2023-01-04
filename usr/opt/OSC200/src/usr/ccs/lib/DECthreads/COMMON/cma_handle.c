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
 * @(#)$RCSfile: cma_handle.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/08/06 17:37:03 $
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Perform functions on handles
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	17 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	31 August 1989
 *		Fix style, add variants of validate handle.
 *	002	Dave Butenhof	12 September 1989
 *		Remove cma_c_null.
 *	003	Dave Butenhof	09 October 1989
 *		Use cma__error to raise exceptions on errors.
 *	004	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	005	Webb Scales	19 October 1989
 *		Added typecasts as required by MIPS/Ultrix.
 *	006	Dave Butenhof	20 October 1989
 *		On non-VMS platform, declare and initialize "cma_c_null"
 *		extern for client reference.
 *      007	Webb Scales	20 October 1989
 *		Remove explicit "extern" from "cma_c_null" declaration to allow
 *		it to be initialized.
 *	008	Dave Butenhof	30 November 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	009	Dave Butenhof	18 April 1990
 *		Remove cma_c_null
 *	010	Dave Butenhof	28 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	011	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	012	Dave Butenhof	10 February 1992
 *		Add status-returning variant of validate functions.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_handle.h>
#include <cma_attr.h>
#include <cma_errors.h>

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Copy one handle to another.
 *
 *  FORMAL PARAMETERS:
 *
 *	handle1		source handle
 *
 *	handle2		destination handle
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
cma_handle_assign
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle1,	/* source handle */
	cma_t_handle	*handle2)	/* destination handle */
#else	/* no prototypes */
	(handle1, handle2)
	cma_t_handle	*handle1;	/* source handle */
	cma_t_handle	*handle2;	/* destination handle */
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle1;	/* Cast to internal form for copy */
    cma__t_int_handle	*ihandle2;


    /*
     * Note that the validity of the handles is not checked: it's perfectly
     * reasonable to assign unreasonable values.
     */
    ihandle1 = (cma__t_int_handle *)handle1;
    ihandle2 = (cma__t_int_handle *)handle2;
    ihandle2->sequence	= ihandle1->sequence;
    ihandle2->type	= ihandle1->type;
    ihandle2->pointer	= ihandle1->pointer;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Compare two handles
 *
 *  FORMAL PARAMETERS:
 *
 *	handle1		first handle
 *
 *	handle2		second handle
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
 *	Boolean value of equality (TRUE if equal, FALSE if not)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_boolean
cma_handle_equal
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle1,	/* fiDrst handle */
	cma_t_handle	*handle2)	/* second handle */
#else	/* no prototypes */
	(handle1, handle2)
	cma_t_handle	*handle1;	/* first handle */
	cma_t_handle	*handle2;	/* second handle */
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle1;	/* Cast to internal form for copy */
    cma__t_int_handle	*ihandle2;


    /*
     * Note that the validity of the handles is not checked: it's perfectly
     * reasonable to compare unreasonable values.
     */
    ihandle1 = (cma__t_int_handle *)handle1;
    ihandle2 = (cma__t_int_handle *)handle2;

    if ((ihandle2->sequence == ihandle1->sequence)
	    && (ihandle2->type == ihandle1->type)
	    && (ihandle2->pointer == ihandle1->pointer))
	return cma_c_true;
    else
	return cma_c_false;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Clear a handle.
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
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
cma__clear_handle
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle)
#else	/* no prototypes */
	(handle)
	cma_t_handle	*handle;
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    ihandle->sequence	= 0;
    ihandle->type	= 0;
    ihandle->pointer	= (cma__t_object *)cma_c_null_ptr;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize a handle for an object
 *
 *  FORMAL PARAMETERS:
 *
 *	addr		address of object
 *	
 *	handle		address of handle
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
cma__object_to_handle
#ifdef _CMA_PROTO_
	(
	cma__t_object	*addr,
	cma_t_handle	*handle)
#else	/* no prototypes */
	(addr, handle)
	cma__t_object	*addr;
	cma_t_handle	*handle;
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    ihandle->sequence	= addr->sequence;
    ihandle->type	= addr->type;
    ihandle->pointer	= addr;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Validate a handle to be sure the format is OK, that it points to a
 *	valid object, and that the type is correct.  Report errors as
 *	appropriate.  This version will default a null handle to the default
 *	attributes object.
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
 *
 *	type		required type of object.
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
 *	Pointer to the internal object structure.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_attr *
cma__validate_default_attr
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle)	/* Handle to validate */
#else	/* no prototypes */
	(handle)
	cma_t_handle	*handle;	/* Handle to validate */
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    if (ihandle == (cma__t_int_handle *)cma_c_null_ptr)
	cma__error (cma_s_existence);
    else if (ihandle->pointer == (cma__t_object *)cma_c_null_ptr)
	{

	/*
	 * All object creation routines allow passing cma_c_null for the
	 * attributes object, which defaults to a special attributes object
	 * known (internally) as "cma__g_def_attr".  So to simplify
	 * attributes handle processing, cma__validate_default_attr will
	 * provide that defaulting automatically.
	 */
	if ((ihandle->sequence == 0) && (ihandle->type == 0))
	    return &cma__g_def_attr;
	else
	    cma__error (cma_s_existence);

	}
    else if ((ihandle->sequence != ihandle->pointer->sequence)
	    || (ihandle->type != ihandle->pointer->type))
	cma__error (cma_s_existence);
    else if (ihandle->type != cma__c_obj_attr)
	cma__error (cma_s_use_error);

    return (cma__t_int_attr *)ihandle->pointer;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Validate a handle to be sure the format is OK, that it points to a
 *	valid object, and that the type is correct.  Report errors as
 *	appropriate.  This version will default a null handle to the default
 *	attributes object.
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
 *
 *	type		required type of object.
 *
 *	attr		return address of attributes object
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
 *	Status code
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_status
cma__val_defattr_stat
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle,	/* Handle to validate */
	cma__t_int_attr	**attr)
#else	/* no prototypes */
	(handle, attr)
	cma_t_handle	*handle;	/* Handle to validate */
	cma__t_int_attr	**attr;
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    if (ihandle == (cma__t_int_handle *)cma_c_null_ptr)
	return cma_s_existence;
    else if (ihandle->pointer == (cma__t_object *)cma_c_null_ptr)
	{

	/*
	 * All object creation routines allow passing cma_c_null for the
	 * attributes object, which defaults to a special attributes object
	 * known (internally) as "cma__g_def_attr".  So to simplify
	 * attributes handle processing, cma__validate_default_attr will
	 * provide that defaulting automatically.
	 */
	if ((ihandle->sequence == 0) && (ihandle->type == 0))
	    return (*attr = &cma__g_def_attr, cma_s_normal);
	else
	    return cma_s_existence;

	}
    else if ((ihandle->sequence != ihandle->pointer->sequence)
	    || (ihandle->type != ihandle->pointer->type))
	return cma_s_existence;
    else if (ihandle->type != cma__c_obj_attr)
	return cma_s_use_error;

    *attr = (cma__t_int_attr *)ihandle->pointer;
    return cma_s_normal;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Validate a handle to be sure the format is OK, that it points to a
 *	valid object, and that the type is correct.  Report errors as
 *	appropriate.
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
 *
 *	type		required type of object.
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
 *	Pointer to the internal object structure.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_object *
cma__validate_handle
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle,	/* Handle to validate */
	cma_t_natural	type)		/* Expected type of object */
#else	/* no prototypes */
	(handle, type)
	cma_t_handle	*handle;	/* Handle to validate */
	cma_t_natural	type;		/* Expected type of object */
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    if (ihandle == (cma__t_int_handle *)cma_c_null_ptr)
	cma__error (cma_s_existence);
    else if (ihandle->pointer == (cma__t_object *)cma_c_null_ptr)
	cma__error (cma_s_existence);
    else if ((ihandle->sequence != ihandle->pointer->sequence)
	    || (ihandle->type != ihandle->pointer->type))
	cma__error (cma_s_existence);
    else if (ihandle->type != type)
	cma__error (cma_s_use_error);

    return ihandle->pointer;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Validate a handle to be sure the format is OK, that it points to a
 *	valid object, and that the type is correct.  Report errors as
 *	appropriate.
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
 *
 *	type		required type of object.
 *
 *	obj		return address of object
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
 *	status
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_status
cma__val_hand_stat
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle,	/* Handle to validate */
	cma_t_natural	type,		/* Expected type of object */
	cma__t_object	**obj)
#else	/* no prototypes */
	(handle, type, obj)
	cma_t_handle	*handle;	/* Handle to validate */
	cma_t_natural	type;		/* Expected type of object */
	cma__t_object	**obj;
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    if (ihandle == (cma__t_int_handle *)cma_c_null_ptr)
	return cma_s_existence;
    else if (ihandle->pointer == (cma__t_object *)cma_c_null_ptr)
	return cma_s_existence;
    else if ((ihandle->sequence != ihandle->pointer->sequence)
	    || (ihandle->type != ihandle->pointer->type))
	return cma_s_existence;
    else if (ihandle->type != type)
	return cma_s_use_error;

    *obj = ihandle->pointer;
    return cma_s_normal;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Validate a handle to be sure the format is OK, that it points to a
 *	valid object, and that the type is correct.  Report errors as
 *	appropriate.  This version allows the handle to be null (e.g.,
 *	sequence, type, and pointer are all 0)... the returned pointer will
 *	be null. This should be used by functions which specifically allow
 *	the null handle (except for attributes object parameters which should
 *	default to the default attr. obj: these should be validated with
 *	cma__validate_attr_handle).
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
 *
 *	type		required type of object.
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
 *	Pointer to the internal object structure.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_object *
cma__validate_handle_null
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle,	/* Handle to validate */
	cma_t_natural	type)		/* Expected type of object */
#else	/* no prototypes */
	(handle, type)
	cma_t_handle	*handle;	/* Handle to validate */
	cma_t_natural	type;		/* Expected type of object */
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    if (ihandle == (cma__t_int_handle *)cma_c_null_ptr)
	cma__error (cma_s_existence);
    else if (ihandle->pointer == (cma__t_object *)cma_c_null_ptr)
	{

	/*
	 * Allow the handle to have all fields null.  If not, return the
	 * normal existance error.  If so, return the null pointer.  The
	 * caller must test the returned pointer, obviously.
	 */
	if ((ihandle->sequence == 0)
		&& (ihandle->type == 0))
	    return (cma__t_object *)cma_c_null_ptr;
	else
	    cma__error (cma_s_existence);

	}
    else if ((ihandle->sequence != ihandle->pointer->sequence)
	    || (ihandle->type != ihandle->pointer->type))
	cma__error (cma_s_existence);
    else if (ihandle->type != type)
	cma__error (cma_s_use_error);

    return ihandle->pointer;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Validate a handle to be sure the format is OK, that it points to a
 *	valid object, and that the type is correct.  Report errors as
 *	appropriate.  This version allows the handle to be null (e.g.,
 *	sequence, type, and pointer are all 0)... the returned pointer will
 *	be null. This should be used by functions which specifically allow
 *	the null handle (except for attributes object parameters which should
 *	default to the default attr. obj: these should be validated with
 *	cma__validate_attr_handle).
 *
 *  FORMAL PARAMETERS:
 *
 *	handle		address of handle
 *
 *	type		required type of object.
 *
 *	obj		return address of object
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
 *	status
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_status
cma__val_handnull_stat
#ifdef _CMA_PROTO_
	(
	cma_t_handle	*handle,	/* Handle to validate */
	cma_t_natural	type,		/* Expected type of object */
	cma__t_object	**obj)
#else	/* no prototypes */
	(handle, type, obj)
	cma_t_handle	*handle;	/* Handle to validate */
	cma_t_natural	type;		/* Expected type of object */
	cma__t_object	**obj;
#endif	/* prototype */
    {
    cma__t_int_handle	*ihandle = (cma__t_int_handle *)handle;


    if (ihandle == (cma__t_int_handle *)cma_c_null_ptr)
	return cma_s_existence;
    else if (ihandle->pointer == (cma__t_object *)cma_c_null_ptr)
	{

	/*
	 * Allow the handle to have all fields null.  If not, return the
	 * normal existance error.  If so, return the null pointer.  The
	 * caller must test the returned pointer, obviously.
	 */
	if ((ihandle->sequence == 0)
		&& (ihandle->type == 0))
	    return (*obj = (cma__t_object *)cma_c_null_ptr, cma_s_normal);
	else
	    return cma_s_existence;

	}
    else if ((ihandle->sequence != ihandle->pointer->sequence)
	    || (ihandle->type != ihandle->pointer->type))
	return cma_s_existence;
    else if (ihandle->type != type)
	return cma_s_use_error;

    *obj = ihandle->pointer;
    return cma_s_normal;
    }
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_HANDLE.C */
/*  *4    18-FEB-1992 15:28:55 BUTENHOF "Make status-returning variants of validate functions" */
/*  *3    10-JUN-1991 18:21:55 SCALES "Add sccs headers for Ultrix" */
/*  *2    14-DEC-1990 00:55:34 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:45:56 BUTENHOF "General handle support" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_HANDLE.C */
