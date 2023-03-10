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
** Generated by ODL Version X0.1 on Tue Jul 14 18:32:26 1992
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
** Private Type Definitions for the Link Object
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
typedef _Object _Linknet, _PtrTo _LinknetPtr;
typedef _Object _CompLinknet, _PtrTo _CompLinknetPtr;
typedef _Object _Step, _PtrTo _StepPtr;
typedef _Object _Path, _PtrTo _PathPtr;
typedef _Object _CompPath, _PtrTo _CompPathPtr;
typedef _Object _Linkbase, _PtrTo _LinkbasePtr;

/*
** Link Instance Structure Definition
*/

typedef struct __Link {
        _Type type;
        _Linkbase linkbase;
        _Integer identifier;
        _DDIFString description;
        _DDIFString author;
        _Date creation_date;
        _Set_of(_DDIFString) keywords;
        _Set_of(_Property) properties;
        _StateFlags flags;
        _Linknet linknet;
        union {_Surrogate surrogate; _Integer id;} source;
        union {_Surrogate surrogate; _Integer id;} target;
        _DDIFString relationship_type;
        _DDIFString source_desc;
        _Set_of(_DDIFString) source_keywords;
        _DDIFString target_desc;
        _Set_of(_DDIFString) target_keywords;
    } _LinkInstance, _PtrTo _Link, _PtrTo _PtrTo _LinkPtr;

/*
** Link Property Name Table Index Definitions
*/

#define _LinknetIndex 0
#define _SourceIndex 1
#define _TargetIndex 2
#define _RelationshipTypeIndex 3
#define _SourceDescriptionIndex 4
#define _SourceKeywordsIndex 5
#define _TargetDescriptionIndex 6
#define _TargetKeywordsIndex 7

/*
** Link Property Name Table Definition
*/

#define _LinkPropertyNameTable \
    _PropertyNameTableEntry _Constant LinkPropertyNameTable[] = { \
        {_P_Linknet, _LinknetIndex, _True}, \
        {_P_Source, _SourceIndex, _True}, \
        {_P_Target, _TargetIndex, _True}, \
        {_P_RelationshipType, _RelationshipTypeIndex, _True}, \
        {_P_SourceDescription, _SourceDescriptionIndex, _True}, \
        {_P_SourceKeywords, _SourceKeywordsIndex, _True}, \
        {_P_TargetDescription, _TargetDescriptionIndex, _True}, \
        {_P_TargetKeywords, _TargetKeywordsIndex, _True}, \
        {(_String) 0, 0, _False} \
    }

/*
** Link Property Accessor Definitions
*/

#define _Linknet_of(Obj) ((_Link) _Reference(Obj))->linknet
#define _Source_of(Obj) ((_Link) _Reference(Obj))->source
#define _Target_of(Obj) ((_Link) _Reference(Obj))->target
#define _RelationshipType_of(Obj) ((_Link) _Reference(Obj))->relationship_type
#define _SourceDescription_of(Obj) ((_Link) _Reference(Obj))->source_desc
#define _SourceKeywords_of(Obj) ((_Link) _Reference(Obj))->source_keywords
#define _TargetDescription_of(Obj) ((_Link) _Reference(Obj))->target_desc
#define _TargetKeywords_of(Obj) ((_Link) _Reference(Obj))->target_keywords

/*
** Include Generic Operation Support
*/

#include "lwk_operation_prototypes.h"

/*
** Link Method Declarations
*/

_DeclareFunction(void LwkOpObjIllOp, (_in _Object object));
_DeclareFunction(_Link LwkOpObjCopy, (_in _Link object, _in _Boolean aggreate));
_DeclareFunction(_Link LwkOpObjCreate, (_in _Type type));
_DeclareFunction(void LwkOpLinkDecode, (_inout _Link persistent,
    _in _DDIShandle handle, _in _Boolean keys_only,
    _inout _SetPtr properties));
_DeclareFunction(void LwkOpLinkEncode, (_in _Link persistent,
    _in _Boolean aggregate, _in _DDIShandle handle));
_DeclareFunction(_Set LwkOpLinkEnumProps, (_in _Link object));
_DeclareFunction(_Integer LwkOpPersistExport, (_in _Link persistent,
    _in _Boolean aggregate, _out _VaryingStringPtr encoding));
_DeclareFunction(void LwkOpLinkExpunge, (_inout _Link object));
_DeclareFunction(void LwkOpLinkFree, (_inout _Link object));
_DeclareFunction(_Domain LwkOpObjGetDomain, (_in _Link object));
_DeclareFunction(_ObjectDesc LwkOpPersistGetObjectDesc, (
    _in _Link persistent));
_DeclareFunction(_ObjectId LwkOpLinkGetObjectId, (
    _in _Link persistent));
_DeclareFunction(void LwkOpLinkGetValue, (_in _Link object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value));
_DeclareFunction(_Domain LwkOpLinkGetValueDomain, (_in _Link object,
    _in _String property_name));
_DeclareFunction(_List LwkOpLinkGetValueList, (_in _Link object,
    _in _String property_name));
_DeclareFunction(_Link LwkOpPersistImport, (_in _Type type,
    _in _VaryingString encoding));
_DeclareFunction(void LwkOpPersistImportKeyProperties, (
    _in _Type type, _in _VaryingString encoding,
    _inout _SetPtr properties));
_DeclareFunction(void LwkOpLinkInitialize, (_inout _Link object,
    _in _Link proto_object, _Boolean aggregate));
_DeclareFunction(_Boolean LwkOpLinkIsMultiValued, (_in _Link object,
    _in _String property_name));
_DeclareFunction(_Boolean LwkOpObjIsType, (_in _Link object,
    _in _Type type));
_DeclareFunction(_Boolean LwkOpLinkSelected, (_in _Link link,
    _in _QueryExpression expression));
_DeclareFunction(_Boolean LwkOpPersistSetState, (_inout _Link persistent,
    _in _PersistentState state, _in _StateOperation operation));
_DeclareFunction(void LwkOpLinkSetValue, (_inout _Link object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value, _in _SetFlag flag));
_DeclareFunction(void LwkOpLinkSetValueList, (_inout _Link object,
    _in _String property_name, _in _List_of(_Domain) value_list,
    _in _SetFlag flag));

/*
** Link Type Instance Definition
*/

#define _LinkTypeInstance \
    _TypeInstance _Constant LWK__LinkTypeInstance = { \
        &LWK__PersistentTypeInstance, \
        sizeof(_LinkInstance), \
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
        (_Method) LwkOpLinkDecode, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpLinkEncode, \
        (_Method) LwkOpLinkEnumProps, \
        (_Method) LwkOpPersistExport, \
        (_Method) LwkOpLinkExpunge, \
        (_Method) LwkOpLinkFree, \
        (_Method) LwkOpObjGetDomain, \
        (_Method) LwkOpPersistGetObjectDesc, \
        (_Method) LwkOpLinkGetObjectId, \
        (_Method) LwkOpLinkGetValue, \
        (_Method) LwkOpLinkGetValueDomain, \
        (_Method) LwkOpLinkGetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpPersistImport, \
        (_Method) LwkOpPersistImportKeyProperties, \
        (_Method) LwkOpLinkInitialize, \
        (_Method) LwkOpLinkIsMultiValued, \
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
        (_Method) LwkOpLinkSelected, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpPersistSetState, \
        (_Method) LwkOpLinkSetValue, \
        (_Method) LwkOpLinkSetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp \
        } \
    }

#define _LinkType \
    _Type _Constant LWK__LinkType = &LWK__LinkTypeInstance
