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
** Generated by ODL Version X0.1 on Tue Jul 14 18:32:29 1992
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
** Private Type Definitions for the Step Object
*/

/*
** Abstract Type Definitions for subtypes of Object
*/

typedef _Object _List, _PtrTo _ListPtr;
typedef _Object _Set, _PtrTo _SetPtr;
typedef _Object _Property, _PtrTo _PropertyPtr;
typedef _Object _ObjectId, _PtrTo _ObjectIdPtr;
typedef _Object _ObjectDesc, _PtrTo _ObjectDescPtr;
typedef _Object _Ui, _PtrTo _UiPtr;
typedef _Object _DXmUi, _PtrTo _DXmUiPtr;
typedef _Object _DXmEnvState, _PtrTo _DXmEnvStatePtr;
typedef _Object _Persistent, _PtrTo _PersistentPtr;
typedef _Object _Surrogate, _PtrTo _SurrogatePtr;
typedef _Object _Link, _PtrTo _LinkPtr;
typedef _Object _Linknet, _PtrTo _LinknetPtr;
typedef _Object _CompLinknet, _PtrTo _CompLinknetPtr;
typedef _Object _Path, _PtrTo _PathPtr;
typedef _Object _CompPath, _PtrTo _CompPathPtr;
typedef _Object _Linkbase, _PtrTo _LinkbasePtr;

/*
** Step Instance Structure Definition
*/

typedef struct __Step {
        _Type type;
        _Linkbase linkbase;
        _Integer identifier;
        _DDIFString description;
        _DDIFString author;
        _Date creation_date;
        _Set_of(_DDIFString) keywords;
        _Set_of(_Property) properties;
        _StateFlags flags;
        _Path path;
        union {_Surrogate surrogate; _Integer id;} origin;
        union {_Surrogate surrogate; _Integer id;} dest;
        union {struct __Step *step; _Integer id;} previous;
        union {struct __Step *step; _Integer id;} next;
        _FollowType follow_type;
        _String destination_operation;
        _Integer interval;
    } _StepInstance, _PtrTo _Step, _PtrTo _PtrTo _StepPtr;

/*
** Step Property Name Table Index Definitions
*/

#define _PathIndex 0
#define _OriginIndex 1
#define _DestinationIndex 2
#define _PreviousStepIndex 3
#define _NextStepIndex 4
#define _FollowTypeIndex 5
#define _DestinationOperationIndex 6
#define _IntervalIndex 7

/*
** Step Property Name Table Definition
*/

#define _StepPropertyNameTable \
    _PropertyNameTableEntry _Constant StepPropertyNameTable[] = { \
        {_P_Path, _PathIndex, _True}, \
        {_P_Origin, _OriginIndex, _True}, \
        {_P_Destination, _DestinationIndex, _True}, \
        {_P_PreviousStep, _PreviousStepIndex, _True}, \
        {_P_NextStep, _NextStepIndex, _True}, \
        {_P_FollowType, _FollowTypeIndex, _True}, \
        {_P_DestinationOperation, _DestinationOperationIndex, _True}, \
        {_P_Interval, _IntervalIndex, _True}, \
        {(_String) 0, 0, _False} \
    }

/*
** Step Property Accessor Definitions
*/

#define _Path_of(Obj) ((_Step) _Reference(Obj))->path
#define _Origin_of(Obj) ((_Step) _Reference(Obj))->origin
#define _Destination_of(Obj) ((_Step) _Reference(Obj))->dest
#define _PreviousStep_of(Obj) ((_Step) _Reference(Obj))->previous
#define _NextStep_of(Obj) ((_Step) _Reference(Obj))->next
#define _FollowType_of(Obj) ((_Step) _Reference(Obj))->follow_type
#define _DestinationOperation_of(Obj) ((_Step) _Reference(Obj))->destination_operation
#define _Interval_of(Obj) ((_Step) _Reference(Obj))->interval

/*
** Include Generic Operation Support
*/

#include "lwk_operation_prototypes.h"

/*
** Step Method Declarations
*/

_DeclareFunction(void LwkOpObjIllOp, (_in _Object object));
_DeclareFunction(_Step LwkOpObjCopy, (_in _Step object, _in _Boolean aggreate));
_DeclareFunction(_Step LwkOpObjCreate, (_in _Type type));
_DeclareFunction(void LwkOpStepDecode, (_inout _Step persistent,
    _in _DDIShandle handle, _in _Boolean keys_only,
    _inout _SetPtr properties));
_DeclareFunction(void LwkOpStepEncode, (_in _Step persistent,
    _in _Boolean aggregate, _in _DDIShandle handle));
_DeclareFunction(_Set LwkOpStepEnumProps, (_in _Step object));
_DeclareFunction(_Integer LwkOpPersistExport, (_in _Step persistent,
    _in _Boolean aggregate, _out _VaryingStringPtr encoding));
_DeclareFunction(void LwkOpStepExpunge, (_inout _Step object));
_DeclareFunction(void LwkOpStepFree, (_inout _Step object));
_DeclareFunction(_Domain LwkOpObjGetDomain, (_in _Step object));
_DeclareFunction(_ObjectDesc LwkOpPersistGetObjectDesc, (
    _in _Step persistent));
_DeclareFunction(_ObjectId LwkOpStepGetObjectId, (
    _in _Step persistent));
_DeclareFunction(void LwkOpStepGetValue, (_in _Step object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value));
_DeclareFunction(_Domain LwkOpStepGetValueDomain, (_in _Step object,
    _in _String property_name));
_DeclareFunction(_List LwkOpStepGetValueList, (_in _Step object,
    _in _String property_name));
_DeclareFunction(_Step LwkOpPersistImport, (_in _Type type,
    _in _VaryingString encoding));
_DeclareFunction(void LwkOpPersistImportKeyProperties, (
    _in _Type type, _in _VaryingString encoding,
    _inout _SetPtr properties));
_DeclareFunction(void LwkOpStepInitialize, (_inout _Step object,
    _in _Step proto_object, _Boolean aggregate));
_DeclareFunction(_Boolean LwkOpStepIsMultiValued, (_in _Step object,
    _in _String property_name));
_DeclareFunction(_Boolean LwkOpObjIsType, (_in _Step object,
    _in _Type type));
_DeclareFunction(_Boolean LwkOpStepSelected, (_in _Step step,
    _in _QueryExpression expression));
_DeclareFunction(_Boolean LwkOpPersistSetState, (_inout _Step persistent,
    _in _PersistentState state, _in _StateOperation operation));
_DeclareFunction(void LwkOpStepSetValue, (_inout _Step object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value, _in _SetFlag flag));
_DeclareFunction(void LwkOpStepSetValueList, (_inout _Step object,
    _in _String property_name, _in _List_of(_Domain) value_list,
    _in _SetFlag flag));

/*
** Step Type Instance Definition
*/

#define _StepTypeInstance \
    _TypeInstance _Constant LWK__StepTypeInstance = { \
        &LWK__PersistentTypeInstance, \
        sizeof(_StepInstance), \
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
        (_Method) LwkOpStepDecode, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpStepEncode, \
        (_Method) LwkOpStepEnumProps, \
        (_Method) LwkOpPersistExport, \
        (_Method) LwkOpStepExpunge, \
        (_Method) LwkOpStepFree, \
        (_Method) LwkOpObjGetDomain, \
        (_Method) LwkOpPersistGetObjectDesc, \
        (_Method) LwkOpStepGetObjectId, \
        (_Method) LwkOpStepGetValue, \
        (_Method) LwkOpStepGetValueDomain, \
        (_Method) LwkOpStepGetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpPersistImport, \
        (_Method) LwkOpPersistImportKeyProperties, \
        (_Method) LwkOpStepInitialize, \
        (_Method) LwkOpStepIsMultiValued, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIsType, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpStepSelected, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpPersistSetState, \
        (_Method) LwkOpStepSetValue, \
        (_Method) LwkOpStepSetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp \
        } \
    }

#define _StepType \
    _Type _Constant LWK__StepType = &LWK__StepTypeInstance