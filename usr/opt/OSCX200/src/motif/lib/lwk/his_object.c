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
** COPYRIGHT (c) 1988 BY
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
**	Object object methods 
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**--
*/


/*
**  Include Files
*/

#ifdef MSDOS
#include "include.h"
#include "object.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_object.h"
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
**  Global Data Definitions
*/

_Global _ObjectTypeInstance;      /* Instance record for Type */
_Global _ObjectType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeObjectInstance;
static _ObjectPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    ObjectPropertyNameTable;


void  LwkOpObj(object)
_Object object;

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
    return;
    }


void  LwkOpObjIllOp(object)
_Object object;

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
    _Raise(inv_operation);

    return;
    }


_Object  LwkOpObjCreate(type)
_Type type;

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
    _Object volatile new_object;

    /*
    **  Can't create an instance of and Abstract Object Type
    */

    if (_Size_of(type) <= 0)
	_Raise(abstract_object);

    /*
    **  Initialize
    */

    new_object = (_Object) _NullObject;

    _StartExceptionBlock

    /*
    **  Allocate an instance.  The size is stored in the Type.
    */

    new_object = (_Object) _AllocateMem(_Size_of(type));

    /*
    **  Set type property of instance
    */

    _Type_of(new_object) = type;

    /*
    **  Invoke the Initialize operation on the instance
    */

    _Initialize(new_object, _NullObject, _False);

    /*
    **	If any exceptions are raised, delete the new object then reraise the
    **	exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&new_object);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the object to the caller
    */

    return new_object;
    }


_Domain  LwkOpObjGetDomain(object)
_Object object;

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
    return _TypeToDomain(_Type_of(object));
    }


_Boolean  LwkOpObjIsType(object, type)
_Object object;
 _Type type;

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
    _Type object_type;

    object_type = _Type_of(object);

    if (object_type == type)
	return _True;

    while ((object_type = _Supertype_of(object_type)) != (_Type) _NullObject)
	if (object_type == type)
	    return _True;

    return _False;
    }


_Object  LwkOpObjCopy(object, aggregate)
_Object object;
 _Boolean aggregate;

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
    _Object volatile new_object;

    new_object = (_Object) _NullObject;

    _StartExceptionBlock

    /*
    **  Allocate an instance.  The size is stored in the Type.
    */

    new_object = (_Object) _AllocateMem(_Size_of(_Type_of(object)));

    /*
    **  Set type property of instance
    */

    _Type_of(new_object) = _Type_of(object);

    /*
    **  Invoke the Initialize operation on the instance
    */

    _Initialize(new_object, object, aggregate);

    /*
    **  If any exceptions are raised, delete the new object then reraise the
    **	exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&new_object);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the object to the caller
    */

    return new_object;
    }


void  LwkOpObjExpunge(object)
_Object object;

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
    /*
    **  Invoke the Free operation on the instance
    */

    _Free(object);

    /*
    **  Free the instance storage
    */

    _FreeMem(object);

    /*
    **  Set the Object to be the Null Object
    */

    return;
    }


void  LwkOpObjInitialize(object, prototype, aggregate)
_Object object;
 _Object prototype;
 _Boolean aggregate;

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
    /*
    **  No properties need initialization (type was initialize in Create).
    */

    return;
    }


void  LwkOpObjFree(object)
_Object object;

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
    /*
    **  No properties need freeing (type is stored in the instance record).
    */

    return;
    }


_Set  LwkOpObjEnumProps(object)
_Object object;

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
    _Set volatile set;

    set = (_Set) _NullObject;

    _StartExceptionBlock

    /*
    **  Create a Set.
    */

    set = (_Set) _CreateSet(_TypeSet, lwk_c_domain_string,
	(_Integer) _EnumListLength);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **	If any exceptions are raised, delete the property name set then reraise
    **	the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&set);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the Set of property names to the caller
    */

    return set;
    }


_Boolean  LwkOpObjIsMultiValued(object, property)
_Object object;
 _String property;

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
    int index;
    _Boolean answer;

    /*
    **	See if the property is defined by our type.  If not, raise the
    **	appropriate exception.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_Raise(no_such_property);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpObjGetValueDomain(object, property)
_Object object;
 _String property;

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
    int index;
    _Domain domain;

    /*
    **	See if the property is defined by our type.  If not, raise the
    **	appropriate exception.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_Raise(no_such_property);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpObjGetValue(object, property, domain, value)
_Object object;
 _String property;
 _Domain domain;

    _AnyPtr value;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, raise the
    **	appropriate exception.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_Raise(no_such_property);
    }
    else {
	/*
	**  Deal with each property individually
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_List  LwkOpObjGetValueList(object, property)
_Object object;
 _String property;

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
    int index;
    _List value_list;

    value_list = (_List) _NullObject;

    /*
    **	See if the property is defined by our type.  If not, raise the
    **	appropriate exception.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_Raise(no_such_property);
    }
    else {
	/*
	**  Deal with each property individually
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpObjSetValue(object, property, domain, value, flag)
_Object object;
 _String property;
 _Domain domain;

    _AnyPtr value;
 _SetFlag flag;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, raise the
    **	appropriate exception.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_Raise(no_such_property);
    }
    else {
	/*
	**  Deal with each property individually
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpObjSetValueList(object, property, values, flag)
_Object object;
 _String property;
 _List values;

    _SetFlag flag;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, raise the
    **	appropriate exception.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_Raise(no_such_property);
    }
    else {
	/*
	**  Deal with each property individually
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }
