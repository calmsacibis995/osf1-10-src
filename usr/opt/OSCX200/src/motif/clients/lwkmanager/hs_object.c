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
** COPYRIGHT (c) 1989, 1991 BY
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
**	HyperSession
**
**  Version: V1.0
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
**	André Pavanello
**
**  Creation Date: 9-Nov-89
**
**  Modification History:
**  X0.7-1  Pat	17-Nov-89   fix _Delete parameter
**--
*/

/*
**  Include Files
*/
#include "hs_include.h"
#include "hs_object.h"

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Global Data Definitions
*/

_Global _ObjectTypeInstance;      /* Instance record for Type */
_Global _ObjectType;              /* Type */

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeObjectInstance;
static _ObjectPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    ObjectPropertyNameTable;


_Void  EnvOpObj(object)
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


_Void  EnvOpObjIllOp(object)
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


_Object  EnvOpObjCreate(type)
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
    **  Allocate an instance.  The size is stored in the Type
    */

    new_object = (_Object) _AllocateMem(_Size_of(type));
    _ClearMem(new_object, _Size_of(type));
    
    /*
    **  Set type property of instance
    */

    _Type_of(new_object) = type;

    /*
    **  Invoke the Initialize operation on the instance
    */

    _Initialize(new_object, _NullObject);

    /*
    **  If any exceptions are raised, delete the new object then reraise the
    **  exception
    */

    _Exceptions
        _WhenOthers
            _Delete(new_object);
            _Reraise;
    _EndExceptionBlock

    /*
    **  Return the object to the caller
    */
    
    return new_object;
    }


_Boolean  EnvOpObjIsType(object, type)
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


_Object  EnvOpObjCopy(object)
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
    _Object volatile new_object;

    new_object = (_Object) _NullObject;

    _StartExceptionBlock

    /*
    **  Allocate an instance.  The size is stored in the Type.
    */

    new_object = (_Object) _AllocateMem(_Size_of(_Type_of(object)));
    _ClearMem(new_object, _Size_of(_Type_of(object)));
    
    /*
    **  Set type property of instance
    */

    _Type_of(new_object) = _Type_of(object);

    /*
    **  Invoke the Initialize operation on the instance
    */

    _Initialize(new_object, object);

    /*
    **  If any exceptions are raised, delete the new object then reraise the
    **  exception.
    */

    _Exceptions
        _WhenOthers
            _Delete(new_object);
            _Reraise;
    _EndExceptionBlock

    /*
    **  Return the object to the caller
    */

    return new_object;
    }


_Void  EnvOpObjDelete(object)
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


_Void  EnvOpObjInitialize(object, prototype)
_Object object;
 _Object prototype;

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


_Void  EnvOpObjFree(object)
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


_Void  EnvOpObjGetValue(object, property, domain, value)
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
    **  See if the property is defined by our type.  If not, raise the
    **  appropriate exception.
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


_Void  EnvOpObjSetValue(object, property, domain, value, flag)
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
    **  See if the property is defined by our type.  If not, raise the
    **  appropriate exception.
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

