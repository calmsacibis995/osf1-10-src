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
** Generated by ODL Version X0.1 on Tue Jul 14 11:01:25 1992
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
** Private Type Definitions for the HsObject Object
*/

/*
** Abstract Type Definitions for subtypes of Object
*/

typedef _Object _List, _PtrTo _ListPtr;
typedef _Object _Window, _PtrTo _WindowPtr;
typedef _Object _EnvWindow, _PtrTo _EnvWindowPtr;
typedef _Object _LbWindow, _PtrTo _LbWindowPtr;
typedef _Object _EnvContext, _PtrTo _EnvContextPtr;

/*
** HsObject Instance Structure Definition
*/

typedef struct __HsObject {
        _Type type;
        lwk_object HISobject;
        _Integer his_type;
        _StateFlags flags;
    } _HsObjectInstance, _PtrTo _HsObject, _PtrTo _PtrTo _HsObjectPtr;

/*
** HsObject Property Name Table Index Definitions
*/

#define _HisObjectIndex 0
#define _HisTypeIndex 1
#define _P_StateFlags "$StateFlags"
#define _StateFlagsIndex 2

/*
** HsObject Property Name Table Definition
*/

#define _HsObjectPropertyNameTable \
    _PropertyNameTableEntry _Constant HsObjectPropertyNameTable[] = { \
        {_P_HisObject, _HisObjectIndex, _True}, \
        {_P_HisType, _HisTypeIndex, _True}, \
        {_P_StateFlags, _StateFlagsIndex, _False}, \
        {(_String) 0, 0, _False} \
    }

/*
** HsObject Property Accessor Definitions
*/

#define _HisObject_of(Obj) ((_HsObject) _Reference(Obj))->HISobject
#define _HisType_of(Obj) ((_HsObject) _Reference(Obj))->his_type
#define _StateFlags_of(Obj) ((_HsObject) _Reference(Obj))->flags

/*
** Include Generic Operation Support
*/

#include "hs_operation_prototypes.h"

/*
** HsObject Method Declarations
*/

_DeclareFunction(_Void EnvOpObjIllOp, (_in _Object object));
_DeclareFunction(_HsObject EnvOpObjCopy, (_in _HsObject object));
_DeclareFunction(_HsObject EnvOpObjCreate, (_in _Type type));
_DeclareFunction(_HsObject EnvOpHsObjCreate, (_in _Type type,
    _in _Integer his_type, _in lwk_object his_obj));
_DeclareFunction(_Void EnvOpObjDelete, (_inout _HsObject object));
_DeclareFunction(_Void EnvOpHsObjDisplayProperties, (
    _in _HsObject hs_object, _in _AnyPtr display_data,
    _in _Boolean new_object));
_DeclareFunction(_Void EnvOpHsObjFree, (_inout _HsObject object));
_DeclareFunction(_Void EnvOpHsObjGetCSProperty, (_in _HsObject hs_object,
    _in _String property_name, _out _CString *cs_string));
_DeclareFunction(_Boolean EnvOpHsObjGetLinkbase, (_in _HsObject hs_object,
    _inout _CString *linkbase_name,
    _inout _String *linkbase_id));
_DeclareFunction(_Void EnvOpHsObjGetProperty, (_in _HsObject hs_object,
    _in lwk_domain domain, _in _String property_name,
    _inout _AnyPtr *value));
_DeclareFunction(_Void EnvOpHsObjGetValue, (_in _HsObject object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value));
_DeclareFunction(_Void EnvOpHsObjInitialize, (_inout _HsObject object,
    _in _HsObject proto_object));
_DeclareFunction(_Boolean EnvOpObjIsType, (_in _HsObject object, _in _Type type));
_DeclareFunction(_Void EnvOpHsObjSave, (_inout _HsObject hs_object,
    _in _HsObject hs_linkbase, _in _SaveOperation operation));
_DeclareFunction(_Void EnvOpHsObjSaveComposite, (_inout _HsObject container,
    _in _HsObject hs_object, _in _SaveOperation operation));
_DeclareFunction(_Void EnvOpHsObjSetCSProperty, (_in _HsObject hs_object,
    _in _String property_name, _in _CString cs_string, 
    lwk_set_operation operation));
_DeclareFunction(_Boolean EnvOpHsObjSetHsObjState, (
    _inout _HsObject hs_object, _in _HsObjectState state,
    _in _StateOperation operation));
_DeclareFunction(_Void EnvOpHsObjSetProperty, (_in _HsObject hs_object,
    _in lwk_domain domain, _in _String property_name,
    _inout _AnyPtr *value, lwk_set_operation operation));
_DeclareFunction(_Void EnvOpHsObjSetValue, (_inout _HsObject object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value, _in _SetFlag flag));
_DeclareFunction(_Void EnvOpHsObjStore, (_in _HsObject hs_object));

/*
** HsObject Type Instance Definition
*/

#define _HsObjectTypeInstance \
    _TypeInstance _Constant HS__HsObjectTypeInstance = { \
        &HS__ObjectTypeInstance, \
        sizeof(_HsObjectInstance), \
        { \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjCopy, \
        (_Method) EnvOpObjCreate, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjCreate, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjDelete, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjDisplayProperties, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjFree, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjGetCSProperty, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjGetLinkbase, \
        (_Method) EnvOpHsObjGetProperty, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjGetValue, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjInitialize, \
        (_Method) EnvOpObjIsType, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjSave, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjSaveComposite, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjSetCSProperty, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjSetHsObjState, \
        (_Method) EnvOpHsObjSetProperty, \
        (_Method) EnvOpHsObjSetValue, \
        (_Method) EnvOpObjIllOp, \
        (_Method) EnvOpHsObjStore, \
        (_Method) EnvOpObjIllOp \
        } \
    }

#define _HsObjectType \
    _Type _Constant HS__HsObjectType = &HS__HsObjectTypeInstance
