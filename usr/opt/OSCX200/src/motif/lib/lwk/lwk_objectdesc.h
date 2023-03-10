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
** Generated by ODL Version X0.1 on Tue Jul 14 18:32:21 1992
*/

/*
** COPYRIGHT (c) 1989 BY
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
** Private Type Definitions for the ObjectDesc Object
*/

/*
** Abstract Type Definitions for subtypes of Object
*/

typedef _Object _List, _PtrTo _ListPtr;
typedef _Object _Set, _PtrTo _SetPtr;
typedef _Object _Property, _PtrTo _PropertyPtr;
typedef _Object _ObjectId, _PtrTo _ObjectIdPtr;
typedef _Object _Ui, _PtrTo _UiPtr;
typedef _Object _DXmUi, _PtrTo _DXmUiPtr;
typedef _Object _DXmEnvState, _PtrTo _DXmEnvStatePtr;
typedef _Object _Persistent, _PtrTo _PersistentPtr;
typedef _Object _Surrogate, _PtrTo _SurrogatePtr;
typedef _Object _Link, _PtrTo _LinkPtr;
typedef _Object _Linknet, _PtrTo _LinknetPtr;
typedef _Object _CompLinknet, _PtrTo _CompLinknetPtr;
typedef _Object _Step, _PtrTo _StepPtr;
typedef _Object _Path, _PtrTo _PathPtr;
typedef _Object _CompPath, _PtrTo _CompPathPtr;
typedef _Object _Linkbase, _PtrTo _LinkbasePtr;

/*
** ObjectDesc Instance Structure Definition
*/

typedef struct __ObjectDesc {
        _Type type;
        _Domain object_domain;
        _Integer object_id;
        _Integer container_id;
        _String linkbase_id;
        _DDIFString object_name;
        _DDIFString linkbase_name;
    } _ObjectDescInstance, _PtrTo _ObjectDesc, _PtrTo _PtrTo _ObjectDescPtr;

/*
** ObjectDesc Property Name Table Index Definitions
*/

#define _ObjectNameIndex 0
#define _LinkbaseNameIndex 1

/*
** ObjectDesc Property Name Table Definition
*/

#define _ObjectDescPropertyNameTable \
    _PropertyNameTableEntry _Constant ObjectDescPropertyNameTable[] = { \
        {_P_ObjectName, _ObjectNameIndex, _True}, \
        {_P_LinkbaseName, _LinkbaseNameIndex, _True}, \
        {(_String) 0, 0, _False} \
    }

/*
** ObjectDesc Property Accessor Definitions
*/

#define _ObjectName_of(Obj) ((_ObjectDesc) _Reference(Obj))->object_name
#define _LinkbaseName_of(Obj) ((_ObjectDesc) _Reference(Obj))->linkbase_name

/*
** Include Generic Operation Support
*/

#include "lwk_operation_prototypes.h"

/*
** ObjectDesc Method Declarations
*/

_DeclareFunction(void LwkOpObjIllOp, (_in _Object object));
_DeclareFunction(_ObjectDesc LwkOpObjCopy, (_in _ObjectDesc object, _in _Boolean aggreate));
_DeclareFunction(_ObjectDesc LwkOpObjCreate, (_in _Type type));
_DeclareFunction(void LwkOpODescDecode, (_inout _ObjectDesc odesc,
    _in _DDIShandle handle, _in _Boolean keys_only,
    _inout _SetPtr properties));
_DeclareFunction(void LwkOpODescEncode, (_in _ObjectDesc odesc,
    _in _Boolean aggregate, _in _DDIShandle handle));
_DeclareFunction(_Set LwkOpODescEnumProps, (_in _ObjectDesc object));
_DeclareFunction(_Integer LwkOpOidExport, (_in _ObjectDesc oid,
    _in _Boolean aggregate, _out _VaryingStringPtr encoding));
_DeclareFunction(void LwkOpObjExpunge, (_inout _ObjectDesc object));
_DeclareFunction(void LwkOpODescFree, (_inout _ObjectDesc object));
_DeclareFunction(_Domain LwkOpObjGetDomain, (_in _ObjectDesc object));
_DeclareFunction(void LwkOpODescGetValue, (_in _ObjectDesc object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value));
_DeclareFunction(_Domain LwkOpODescGetValueDomain, (_in _ObjectDesc object,
    _in _String property_name));
_DeclareFunction(_List LwkOpODescGetValueList, (_in _ObjectDesc object,
    _in _String property_name));
_DeclareFunction(_ObjectDesc LwkOpOidImport, (_in _Type type,
    _in _VaryingString encoding));
_DeclareFunction(void LwkOpODescInitialize, (_inout _ObjectDesc object,
    _in _ObjectDesc proto_object, _Boolean aggregate));
_DeclareFunction(_Boolean LwkOpODescIsMultiValued, (_in _ObjectDesc object,
    _in _String property_name));
_DeclareFunction(_Boolean LwkOpObjIsType, (_in _ObjectDesc object,
    _in _Type type));
_DeclareFunction(_Persistent LwkOpOidRetrieve, (_in _ObjectDesc id));
_DeclareFunction(void LwkOpODescSetValue, (_inout _ObjectDesc object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value, _in _SetFlag flag));
_DeclareFunction(void LwkOpODescSetValueList, (_inout _ObjectDesc object,
    _in _String property_name, _in _List_of(_Domain) value_list,
    _in _SetFlag flag));

/*
** ObjectDesc Type Instance Definition
*/

#define _ObjectDescTypeInstance \
    _TypeInstance _Constant LWK__ObjectDescTypeInstance = { \
        &LWK__ObjectIdTypeInstance, \
        sizeof(_ObjectDescInstance), \
        { \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjCopy, \
        (_Method) LwkOpObjCreate, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpODescDecode, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpODescEncode, \
        (_Method) LwkOpODescEnumProps, \
        (_Method) LwkOpOidExport, \
        (_Method) LwkOpObjExpunge, \
        (_Method) LwkOpODescFree, \
        (_Method) LwkOpObjGetDomain, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpODescGetValue, \
        (_Method) LwkOpODescGetValueDomain, \
        (_Method) LwkOpODescGetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpOidImport, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpODescInitialize, \
        (_Method) LwkOpODescIsMultiValued, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIsType, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpOidRetrieve, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpODescSetValue, \
        (_Method) LwkOpODescSetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp \
        } \
    }

#define _ObjectDescType \
    _Type _Constant LWK__ObjectDescType = &LWK__ObjectDescTypeInstance
