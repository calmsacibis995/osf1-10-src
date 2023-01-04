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
** COPYRIGHT (c) 1988,1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  Subsystem:
**	HyperInformation Services
**
**  Version: X0.1
**
**  Abstract:
**	HyperInformation Services public interface definitions
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner, MEMEX Project
**	W. Ward Clark, MEMEX Project
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**  X0.6-1  WWC  20-Oct-89  lwk_object_desc --> lwk_object_descriptor
**			    lwk_get_object_desc --> lwk_get_object_descriptor
**  X0.16   WWC  26-Feb-91  move DXm routines to LWK_DXM_API.C
**--
*/


/*
**  Include Files
*/

#ifdef MSDOS
#include "include.h"
#include "abstobjs.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_abstract_objects.h"
#endif

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


lwk_status lwk_entry  lwk_add_element(object, domain, element)
lwk_object object;
 lwk_domain domain;

    lwk_any_pointer element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_any_pointer) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _AddElement((_Object) object, (_Domain) domain, (_AnyValuePtr) element,
	_True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_boolean(object, element)
lwk_object object;
 lwk_boolean element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _AddElement((_Object) object, lwk_c_domain_boolean, &element, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_ddif_string(object, element)
lwk_object object;

    lwk_ddif_string element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _AddElement((_Object) object, lwk_c_domain_ddif_string, &element, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_date(object, element)
lwk_object object;
 lwk_date element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _AddElement((_Object) object, lwk_c_domain_date, &element, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_integer(object, element)
lwk_object object;
 lwk_integer element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _AddElement((_Object) object, lwk_c_domain_integer, &element, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_object(object, element)
lwk_object object;
 lwk_object element;

/*
++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Domain domain;

    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    domain = _TypeToDomain(_Type_of((_Object) object));

    _AddElement((_Object) object, domain, &element, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_float(object, element)
lwk_object object;
 lwk_float element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    lwk_float float_pt;

    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    /*
    ** To avoid bug in Ultrix/RISC CC compiler V3.1 -- copy the argument to
    ** a local variable.
    */

    float_pt = element;

    _AddElement((_Object) object, lwk_c_domain_float, &float_pt, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_add_string(object, element)
lwk_object object;
 lwk_string element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _AddElement((_Object) object, lwk_c_domain_string, &element, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


#ifndef MSDOS
lwk_status lwk_entry  lwk_apply(ui, operation, surrogate)
lwk_ui ui;
 lwk_string operation;

    lwk_surrogate surrogate;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(ui))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _Apply((_Ui) ui, (_String) operation, (_Surrogate) surrogate);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_confirm_apply(ui, surrogate)
lwk_ui ui;
 lwk_surrogate surrogate;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(ui))
	_Raise(inv_object);

    if (!_IsValidObject(surrogate))
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _ConfirmApply((_Ui) ui, (_Surrogate) surrogate);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }
#endif /* !MSDOS */


lwk_status lwk_entry  lwk_create(domain, object)
lwk_domain domain;
 lwk_object_ptr object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Type type;

    _StartExceptionBlock

    /*
    **	Find the Type object for the given domain.  If there is no
    **	corresponding Type, return an error.
    */

    type = _DomainToType((_Domain) domain);

    if (type == (_Type) _NullObject)
	_Raise(inv_domain);

    /*
    **  Check the validity of the arguments.
    */

    if (object == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectPtr) object) = (_Object) _Create(type);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_create_list(domain, estimated_count, list)
lwk_domain domain;

    lwk_integer estimated_count;
 lwk_list_ptr list;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (list == (lwk_list_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ListPtr) list) = (_List) _CreateList(_TypeList, (_Domain) domain,
	(_Integer) estimated_count);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_create_set(domain, estimated_count, set)
lwk_domain domain;

    lwk_integer estimated_count;
 lwk_set_ptr set;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (set == (lwk_set_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_SetPtr) set) = (_Set) _CreateSet(_TypeSet, (_Domain) domain,
	(_Integer) estimated_count);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_close(object)
lwk_object object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _Close((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_copy(object, copy)
lwk_object object;
 lwk_object_ptr copy;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (copy == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectPtr) copy) = (_Object) _Copy((_Object) object, _False);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_copy_aggregate(object, copy)
lwk_object object;
 lwk_object_ptr copy;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (copy == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectPtr) copy) = (_Object) _Copy((_Object) object, _True);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_date_to_time(date, time)
lwk_date date;
 lwk_any_pointer time;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidDate(date))
	_Raise(inv_date);

    if (time == (lwk_any_pointer) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation
    */

    *((time_t _PtrTo) time) = _DateToTime(date);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_delete(object)
lwk_object_ptr object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (object == (lwk_object_ptr) _NullObject)
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _Delete((_ObjectPtr) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_delete_date(date)
lwk_date_ptr date;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (date == (lwk_date_ptr) _NullObject)
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _DeleteDate((_DatePtr) date);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_delete_encoding(encoding)
lwk_encoding_ptr encoding;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (encoding == (lwk_encoding_ptr) _NullObject)
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _DeleteEncoding((_VaryingStringPtr) encoding);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_delete_string(string)
lwk_string_ptr string;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (string == (lwk_string_ptr) _NullObject)
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _DeleteString((_StringPtr) string);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_delete_ddif_string(ddifstring)
lwk_ddif_string_ptr ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (ddifstring == (lwk_ddif_string_ptr) _NullObject)
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _DeleteDDIFString((_DDIFStringPtr) ddifstring);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_drop(object)
lwk_object object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _Drop((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_enumerate_properties(object, set)
lwk_object object;

    lwk_set_ptr set;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (set == (lwk_set_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_SetPtr) set) = (_Set) _EnumerateProperties((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_export(object, encoding, size)
lwk_object object;
 lwk_encoding_ptr encoding;

    lwk_integer_ptr size;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _VaryingString vstring;

    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (encoding == (lwk_encoding_ptr) _NullObject)
	_Raise(inv_argument);

    if (size == (lwk_integer_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *size = (lwk_integer) _Export((_Object) object, _True, &vstring);

    /*
    **	Cut the encoding allocation back to the minimum.
    **
    **	Note: this is due to a deficiency in the SQL implementation which
    **	requires that encodings be over-allocated (the maximum segment length)
    **	-- this can be removed when SQL is no longer supported, or when the
    **	deficiency is fixed.
    */

    _ReallocateVaryingString(vstring, _VLength_of(vstring));

    *encoding = (lwk_encoding) vstring;

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_get_domain(object, domain)
lwk_object object;
 lwk_domain_ptr domain;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (domain == (lwk_domain_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_DomainPtr) domain) = (_Domain) _GetDomain((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


#ifndef MSDOS
lwk_status lwk_entry  lwk_get_object_descriptor(object, odesc)
lwk_object object;

    lwk_object_descriptor_ptr odesc;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (odesc == (lwk_object_descriptor_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectDescPtr) odesc) =
	(_ObjectDesc) _GetObjectDesc((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }
#endif /* !MSDOS */


lwk_status lwk_entry  lwk_get_object_id(object, oid)
lwk_object object;

    lwk_object_id_ptr oid;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (oid == (lwk_object_id_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectIdPtr) oid) = (_ObjectId) _GetObjectId((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_get_value(object, property, domain, value)
lwk_object object;
 lwk_string property;

    lwk_domain domain;
 lwk_any_pointer value;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (value == (lwk_any_pointer) _NullObject)
	_Raise(inv_argument);

    if (property == (lwk_string) _NullObject)
	_Raise(no_such_property);

    /*
    **  Invoke the appropriate operation on the object
    */

    _GetValue((_Object) object, (_String) property, (_Domain) domain,
	(_AnyValuePtr) value);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_get_value_domain(object, property, domain)
lwk_object object;

    lwk_string property;
 lwk_domain_ptr domain;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (domain == (lwk_domain_ptr) _NullObject)
	_Raise(inv_argument);

    if (property == (lwk_string) _NullObject)
	_Raise(no_such_property);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_DomainPtr) domain) = (_Domain) _GetValueDomain((_Object) object,
	(_String) property);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_get_value_list(object, property, value_list)
lwk_object object;
 lwk_string property;

    lwk_list_ptr value_list;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (property == (lwk_string) _NullObject)
	_Raise(no_such_property);

    if (value_list == (lwk_list_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ListPtr) value_list) = (_List) _GetValueList((_Object) object,
	(_String) property);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_import(domain, encoding, object)
lwk_domain domain;
 lwk_encoding encoding;

    lwk_object_ptr object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Type type;

    _StartExceptionBlock

    /*
    **	Find the Type object for the given domain.  If there is no
    **	corresponding Type, return an error.
    */

    type = _DomainToType((_Domain) domain);

    if (type == (_Type) _NullObject)
	_Raise(inv_domain);

    /*
    **  Check the validity of the arguments.
    */

    if (object == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    if (encoding == (lwk_encoding ) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectPtr) object) =
	(_Object) _Import(type, (_VaryingString) encoding);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_initialize(hyperinvoked, operation, surrogate)
lwk_boolean_ptr hyperinvoked;

    lwk_string_ptr operation;
 lwk_surrogate_ptr surrogate;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (hyperinvoked == (lwk_boolean_ptr) _NullObject)
	_Raise(inv_argument);

    if (operation == (lwk_string_ptr) _NullObject)
	_Raise(inv_argument);

    if (surrogate == (lwk_surrogate_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the Initialization routine
    */

    LwkOpInitialize((_BooleanPtr) hyperinvoked, (_StringPtr) operation,
	(_SurrogatePtr) surrogate);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_is_multi_valued(object, property, answer)
lwk_object object;
 lwk_string property;

    lwk_boolean_ptr answer;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (property == (lwk_string) _NullObject)
	_Raise(no_such_property);

    if (answer == (lwk_boolean_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_BooleanPtr) answer) = _IsMultiValued((_Object) object,
	(_String) property);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_iterate(object, domain, closure, callback, termination)
lwk_object object;
 lwk_domain domain;

    lwk_closure closure;
 lwk_callback callback;

    lwk_termination_ptr termination;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (termination == (lwk_termination_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_TerminationPtr) termination) = _Iterate((_Object) object,
	(_Domain) domain, (_Closure) closure, (_Callback) callback);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_memory_statistics()
/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Invoke the appropriate operation on the object
    */

    LwkMemoryStatistics();

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_open(name, create, object)
lwk_string name;
 lwk_boolean create;

    lwk_object_ptr object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (object == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectPtr) object) =
	(_Object) _Open(_TypeLinkbase, (_String) name, (_Boolean) create);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_query(object, domain, expression, closure, callback, termination)
lwk_object object;
 lwk_domain domain;

    lwk_query_expression expression;
 lwk_closure closure;
 lwk_callback callback;

    lwk_termination_ptr termination;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (expression == (lwk_query_expression) _NullObject)
	_Raise(inv_argument);

    if (termination == (lwk_termination_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_TerminationPtr) termination) = _Query((_Object) object,
	(_Domain) domain, (_QueryExpression) expression, (_Closure) closure,
	(_Callback) callback);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_element(object, domain, element)
lwk_object object;
 lwk_domain domain;

    lwk_any_pointer element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_any_pointer) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, (_Domain) domain, (_AnyValuePtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_boolean(object, element)
lwk_object object;

    lwk_boolean_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_boolean_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_boolean,
	(_BooleanPtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_ddif_string(object, element)
lwk_object object;

    lwk_ddif_string_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_ddif_string_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_ddif_string,
	(_DDIFStringPtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_date(object, element)
lwk_object object;
 lwk_date_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_date_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_date, (_DatePtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_integer(object, element)
lwk_object object;

    lwk_integer_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_integer_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_integer,
	(_IntegerPtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_object(object, element)
lwk_object object;

    lwk_object_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_object,
	(_ObjectPtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_float(object, element)
lwk_object object;
 lwk_float_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_float_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_float,
	(_FloatPtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_remove_string(object, element)
lwk_object object;

    lwk_string_ptr element;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (element == (lwk_string_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _RemoveElement((_Object) object, lwk_c_domain_string,
	(_StringPtr) element);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_retrieve(object, retrieved_object)
lwk_object object;

    lwk_object_ptr retrieved_object;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (retrieved_object == (lwk_object_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *((_ObjectPtr) retrieved_object) =
	(_Object) _Retrieve((_Object) object);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_set_value(object, property, domain, value, flag)
lwk_object object;
 lwk_string property;

    lwk_domain domain;
 lwk_any_pointer value;
 lwk_set_operation flag;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (property == (lwk_string) _NullObject)
	_Raise(no_such_property);

    /*
    **  Invoke the appropriate operation on the object
    */

    _SetValue((_Object) object, (_String) property, (_Domain) domain,
	(_AnyValuePtr) value, (_SetFlag) flag);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_set_value_list(object, property, value_list, flag)
lwk_object object;
 lwk_string property;

    lwk_list value_list;
 lwk_set_operation flag;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (property == (lwk_string) _NullObject)
	_Raise(no_such_property);

    /*
    **  Invoke the appropriate operation on the object
    */

    _SetValueList((_Object) object, (_String) property, (_List) value_list,
	(_SetFlag) flag);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_status_to_ddif_string(status, ddifstring)
lwk_status status;

    lwk_ddif_string_ptr ddifstring;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (ddifstring == (lwk_ddif_string_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *ddifstring = _StatusToDDIFString((_Status) status);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }

lwk_status lwk_entry  lwk_status_to_string(status, string)
lwk_status status;

    lwk_string_ptr string;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (string == (lwk_string_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *string = _CopyString(_StatusToString((_Status) status));

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }

lwk_status lwk_entry  lwk_store(object, repository)
lwk_object object;
 lwk_linkbase repository;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (!_IsValidObject(repository))
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    _Store((_Object) object, (_Linkbase) repository);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


#ifndef MSDOS
lwk_status lwk_entry  lwk_surrogate_is_highlighted(object, surrogate, answer)
lwk_object object;

    lwk_surrogate surrogate;
 lwk_boolean_ptr answer;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    if (!_IsValidObject(surrogate))
	_Raise(inv_argument);

    if (answer == (lwk_boolean_ptr) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation on the object
    */

    *answer = (lwk_boolean) LwkOpUiNavSurrIsHighlighted((_Ui) object,
	(_Surrogate) surrogate);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }
#endif /* !MSDOS */


lwk_status lwk_entry  lwk_time_to_date(time, date)
lwk_any_pointer time;
 lwk_date_ptr date;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (date == (lwk_date_ptr) _NullObject)
	_Raise(inv_argument);

    if (time == (lwk_any_pointer) _NullObject)
	_Raise(inv_argument);

    /*
    **  Invoke the appropriate operation
    */

    *date = _TimeToDate(*((time_t _PtrTo) time));

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  lwk_transact(object, transaction)
lwk_object object;

    lwk_transaction transaction;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    _Transact((_Object) object, (_Transaction) transaction);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a success status code
    */

    return _StatusCode(success);
    }


lwk_status lwk_entry  his_verify(object, flags, file)
lwk_object object;
 lwk_integer flags;

    _OSFile file;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Boolean ok;

    _StartExceptionBlock

    /*
    **  Check the validity of the arguments.
    */

    if (!_IsValidObject(object))
	_Raise(inv_object);

    /*
    **  Invoke the appropriate operation on the object
    */

    ok = (_Boolean) _Verify((_Object) object, (_Integer) flags, file);

    /*
    **	If any exceptions are raised, return the associated status code.
    */

    _Exceptions
	_WhenOthers
	    return _Others;
    _EndExceptionBlock

    /*
    **  Return a status code
    */

    if (ok)
	return _StatusCode(success);
    else
	return _StatusCode(failure);
    }
