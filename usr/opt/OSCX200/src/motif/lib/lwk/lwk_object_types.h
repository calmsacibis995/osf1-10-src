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
** Generated by ODL Version X0.1 on Tue Jul 14 18:32:32 1992
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
** Object-independent Type Definitions
*/


/*
** Useful Symbol Definitions
*/

#define _NUM_TYPES 18
#define _NUM_OPERATIONS 53

/*
** Type Instance Structure Definition
*/

typedef unsigned long int (_PtrTo _Method)();

typedef struct __Type {
    struct __Type _PtrTo supertype;
    int           instance_size;
    _Method       methods[_NUM_OPERATIONS];
} _TypeInstance, _PtrTo _Type, _PtrTo _PtrTo _TypePtr;

/*
** Type Property Accessor Definitions
*/

#define _Supertype_of(Type) (Type)->supertype
#define _Size_of(Type) (Type)->instance_size
#define _Method_of(Type, Opr) (Type)->methods[Opr]

/*
** Object Instance Structure Definition
*/

typedef struct __Object {
        _Type type;
    } _ObjectInstance, _PtrTo _Object, _PtrTo _PtrTo _ObjectPtr;

/*
** Object Property Accessor Definitions
*/

#define _Type_of(Obj) ((_Object) _Reference(Obj))->type

/*
** Property Table Entry Type Definition
*/

typedef struct __PropertyNameTableEntry {
            char *name;
            int index;
            int public;
        } _PropertyNameTableEntry, _PtrTo _PropertyNameTable, _PtrTo _PtrTo _PropertyNameTablePtr;

/*
** Type Instance Declarations
*/

#define _TypeObjectInstance LWK__ObjectTypeInstance
#define _TypeObject LWK__ObjectType
#define _TypeListInstance LWK__ListTypeInstance
#define _TypeList LWK__ListType
#define _TypeSetInstance LWK__SetTypeInstance
#define _TypeSet LWK__SetType
#define _TypePropertyInstance LWK__PropertyTypeInstance
#define _TypeProperty LWK__PropertyType
#define _TypeObjectIdInstance LWK__ObjectIdTypeInstance
#define _TypeObjectId LWK__ObjectIdType
#define _TypeObjectDescInstance LWK__ObjectDescTypeInstance
#define _TypeObjectDesc LWK__ObjectDescType
#define _TypeUiInstance LWK__UiTypeInstance
#define _TypeUi LWK__UiType
#define _TypeDXmUiInstance LWK__DXmUiTypeInstance
#define _TypeDXmUi LWK__DXmUiType
#define _TypeDXmEnvStateInstance LWK__DXmEnvStateTypeInstance
#define _TypeDXmEnvState LWK__DXmEnvStateType
#define _TypePersistentInstance LWK__PersistentTypeInstance
#define _TypePersistent LWK__PersistentType
#define _TypeSurrogateInstance LWK__SurrogateTypeInstance
#define _TypeSurrogate LWK__SurrogateType
#define _TypeLinkInstance LWK__LinkTypeInstance
#define _TypeLink LWK__LinkType
#define _TypeLinknetInstance LWK__LinknetTypeInstance
#define _TypeLinknet LWK__LinknetType
#define _TypeCompLinknetInstance LWK__CompLinknetTypeInstance
#define _TypeCompLinknet LWK__CompLinknetType
#define _TypeStepInstance LWK__StepTypeInstance
#define _TypeStep LWK__StepType
#define _TypePathInstance LWK__PathTypeInstance
#define _TypePath LWK__PathType
#define _TypeCompPathInstance LWK__CompPathTypeInstance
#define _TypeCompPath LWK__CompPathType
#define _TypeLinkbaseInstance LWK__LinkbaseTypeInstance
#define _TypeLinkbase LWK__LinkbaseType
_External _TypeInstance LWK__ObjectTypeInstance;
_External _Type LWK__ObjectType;
_External _TypeInstance LWK__ListTypeInstance;
_External _Type LWK__ListType;
_External _TypeInstance LWK__SetTypeInstance;
_External _Type LWK__SetType;
_External _TypeInstance LWK__PropertyTypeInstance;
_External _Type LWK__PropertyType;
_External _TypeInstance LWK__ObjectIdTypeInstance;
_External _Type LWK__ObjectIdType;
_External _TypeInstance LWK__ObjectDescTypeInstance;
_External _Type LWK__ObjectDescType;
_External _TypeInstance LWK__UiTypeInstance;
_External _Type LWK__UiType;
_External _TypeInstance LWK__DXmUiTypeInstance;
_External _Type LWK__DXmUiType;
_External _TypeInstance LWK__DXmEnvStateTypeInstance;
_External _Type LWK__DXmEnvStateType;
_External _TypeInstance LWK__PersistentTypeInstance;
_External _Type LWK__PersistentType;
_External _TypeInstance LWK__SurrogateTypeInstance;
_External _Type LWK__SurrogateType;
_External _TypeInstance LWK__LinkTypeInstance;
_External _Type LWK__LinkType;
_External _TypeInstance LWK__LinknetTypeInstance;
_External _Type LWK__LinknetType;
_External _TypeInstance LWK__CompLinknetTypeInstance;
_External _Type LWK__CompLinknetType;
_External _TypeInstance LWK__StepTypeInstance;
_External _Type LWK__StepType;
_External _TypeInstance LWK__PathTypeInstance;
_External _Type LWK__PathType;
_External _TypeInstance LWK__CompPathTypeInstance;
_External _Type LWK__CompPathType;
_External _TypeInstance LWK__LinkbaseTypeInstance;
_External _Type LWK__LinkbaseType;

/*
** Property Name Definitions
*/

#define _P_ActiveCompLinknet "$ActiveCompLinknet"
#define _P_ActiveCompPath "$ActiveCompPath"
#define _P_ActivePath "$ActivePath"
#define _P_ActivePathIndex "$ActivePathIndex"
#define _P_Address "$Address"
#define _P_ApplHighlight "$ApplHighlight"
#define _P_ApplyCb "$ApplyCb"
#define _P_Author "$Author"
#define _P_CloseViewCb "$CloseViewCb"
#define _P_CompleteLinkCb "$CompleteLinkCb"
#define _P_Container "$Container"
#define _P_ContainerIdentifier "$ContainerIdentifier"
#define _P_CreateSurrogateCb "$CreateSurrogateCb"
#define _P_CreationDate "$CreationDate"
#define _P_DefaultHighlight "$DefaultHighlight"
#define _P_DefaultOperation "$DefaultOperation"
#define _P_DefaultRelationship "$DefaultRelationship"
#define _P_DefaultRetainSource "$DefaultRetainSource"
#define _P_DefaultRetainTarget "$DefaultRetainTarget"
#define _P_Description "$Description"
#define _P_Destination "$Destination"
#define _P_DestinationOperation "$DestinationOperation"
#define _P_Domain "$Domain"
#define _P_ElementCount "$ElementCount"
#define _P_EnvironmentChangeCb "$EnvironmentChangeCb"
#define _P_EnvironmentManager "$EnvironmentManager"
#define _P_EnvironmentState "$EnvironmentState"
#define _P_FirstStep "$FirstStep"
#define _P_FollowDestination "$FollowDestination"
#define _P_FollowType "$FollowType"
#define _P_FollowedLink "$FollowedLink"
#define _P_FollowedStep "$FollowedStep"
#define _P_GetSurrogateCb "$GetSurrogateCb"
#define _P_Identifier "$Identifier"
#define _P_InterLinks "$InterLinks"
#define _P_Interval "$Interval"
#define _P_Keywords "$Keywords"
#define _P_LastStep "$LastStep"
#define _P_Linkbase "$Linkbase"
#define _P_LinkbaseIdentifier "$LinkbaseIdentifier"
#define _P_LinkbaseName "$LinkbaseName"
#define _P_Linknet "$Linknet"
#define _P_Linknets "$Linknets"
#define _P_Links "$Links"
#define _P_MainWidget "$MainWidget"
#define _P_MessageCb "$MessageCb"
#define _P_Name "$Name"
#define _P_NewLink "$NewLink"
#define _P_NextStep "$NextStep"
#define _P_NotificationCb "$NotificationCb"
#define _P_Notifications "$Notifications"
#define _P_ObjectDomain "$ObjectDomain"
#define _P_ObjectIdentifier "$ObjectIdentifier"
#define _P_ObjectName "$ObjectName"
#define _P_Open "$Open"
#define _P_Origin "$Origin"
#define _P_OriginOperation "$OriginOperation"
#define _P_Path "$Path"
#define _P_Paths "$Paths"
#define _P_PendingSource "$PendingSource"
#define _P_PendingTarget "$PendingTarget"
#define _P_PreviousStep "$PreviousStep"
#define _P_PropertyName "$PropertyName"
#define _P_RecordingLinknet "$RecordingLinknet"
#define _P_RecordingPath "$RecordingPath"
#define _P_RelationshipType "$RelationshipType"
#define _P_RetainSource "$RetainSource"
#define _P_RetainTarget "$RetainTarget"
#define _P_Source "$Source"
#define _P_SourceDescription "$SourceDescription"
#define _P_SourceKeywords "$SourceKeywords"
#define _P_Steps "$Steps"
#define _P_SupportedOperations "$SupportedOperations"
#define _P_SupportedSurrogates "$SupportedSurrogates"
#define _P_SurrogateSubType "$SurrogateSubType"
#define _P_Surrogates "$Surrogates"
#define _P_Target "$Target"
#define _P_TargetDescription "$TargetDescription"
#define _P_TargetKeywords "$TargetKeywords"
#define _P_UserData "$UserData"
#define _P_Value "$Value"

/*
** Methods Dispatch Table Symbolic Definitions
*/

#define _AddElement_ 0
#define _AppendElements_ 1
#define _Apply_ 2
#define _Close_ 3
#define _CompleteLink_ 4
#define _ConfirmApply_ 5
#define _ConfirmLink_ 6
#define _Copy_ 7
#define _Create_ 8
#define _CreateDXmUi_ 9
#define _CreateList_ 10
#define _CreateSet_ 11
#define _Decode_ 12
#define _DeleteElement_ 13
#define _DisplayMessage_ 14
#define _Drop_ 15
#define _Encode_ 16
#define _EnumerateProperties_ 17
#define _Export_ 18
#define _Expunge_ 19
#define _Free_ 20
#define _GetDomain_ 21
#define _GetObjectDesc_ 22
#define _GetObjectId_ 23
#define _GetValue_ 24
#define _GetValueDomain_ 25
#define _GetValueList_ 26
#define _Highlight_ 27
#define _Import_ 28
#define _ImportKeyProperties_ 29
#define _Initialize_ 30
#define _IsMultiValued_ 31
#define _IsNamed_ 32
#define _IsType_ 33
#define _Iterate_ 34
#define _Navigate_ 35
#define _Open_ 36
#define _Query_ 37
#define _RemoveElement_ 38
#define _Retrieve_ 39
#define _SelectElement_ 40
#define _SelectMenu_ 41
#define _Selected_ 42
#define _SendMessage_ 43
#define _SetState_ 44
#define _SetValue_ 45
#define _SetValueList_ 46
#define _ShowHistory_ 47
#define _ShowLinks_ 48
#define _Store_ 49
#define _SurrogateIsHighlighted_ 50
#define _Transact_ 51
#define _Verify_ 52

/*
** Operation Invocation Macro Definitions
*/

#define _AddElement(Obj, A1, A2, A3) \
    (*((_AddElement_P) ((_Object) _Reference(Obj))->type->methods[_AddElement_]))((_Object) _Reference(Obj), (A1), (A2), (A3))
#define _AddElement_S(Obj, A1, A2, A3, MyType) \
    (*((_AddElement_P) (MyType)->supertype->methods[_AddElement_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _AppendElements(Obj, A1) \
    (*((_AppendElements_P) ((_Object) _Reference(Obj))->type->methods[_AppendElements_]))((_Object) _Reference(Obj), (A1))

#define _Apply(Obj, A1, A2) \
    (*((_Apply_P) ((_Object) _Reference(Obj))->type->methods[_Apply_]))((_Object) _Reference(Obj), (A1), (A2))

#define _Close(Obj) \
    (*((_Close_P) ((_Object) _Reference(Obj))->type->methods[_Close_]))((_Object) _Reference(Obj))

#define _CompleteLink(Obj, A1, A2, A3) \
    (*((_CompleteLink_P) ((_Object) _Reference(Obj))->type->methods[_CompleteLink_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _ConfirmApply(Obj, A1) \
    (*((_ConfirmApply_P) ((_Object) _Reference(Obj))->type->methods[_ConfirmApply_]))((_Object) _Reference(Obj), (A1))

#define _ConfirmLink(Obj, A1, A2, A3) \
    (*((_ConfirmLink_P) ((_Object) _Reference(Obj))->type->methods[_ConfirmLink_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _Copy(Obj, A1) \
    (*((_Copy_P) ((_Object) _Reference(Obj))->type->methods[_Copy_]))((_Object) _Reference(Obj), (A1))

#define _Create(Type) \
    (*((_Create_P) (Type)->methods[_Create_]))((Type))

#define _CreateDXmUi(Type, A1, A2, A3, A4, A5) \
    (*((_CreateDXmUi_P) (Type)->methods[_CreateDXmUi_]))((Type), (A1), (A2), (A3), (A4), (A5))

#define _CreateList(Type, A1, A2) \
    (*((_CreateList_P) (Type)->methods[_CreateList_]))((Type), (A1), (A2))

#define _CreateSet(Type, A1, A2) \
    (*((_CreateSet_P) (Type)->methods[_CreateSet_]))((Type), (A1), (A2))

#define _Decode(Obj, A1, A2, A3) \
    (*((_Decode_P) ((_Object) _Reference(Obj))->type->methods[_Decode_]))((_Object) _Reference(Obj), (A1), (A2), (A3))
#define _Decode_S(Obj, A1, A2, A3, MyType) \
    (*((_Decode_P) (MyType)->supertype->methods[_Decode_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _DeleteElement(Obj, A1, A2) \
    (*((_DeleteElement_P) ((_Object) _Reference(Obj))->type->methods[_DeleteElement_]))((_Object) _Reference(Obj), (A1), (A2))

#define _DisplayMessage(Obj, A1, A2) \
    (*((_DisplayMessage_P) ((_Object) _Reference(Obj))->type->methods[_DisplayMessage_]))((_Object) _Reference(Obj), (A1), (A2))

#define _Drop(Obj) \
    (*((_Drop_P) ((_Object) _Reference(Obj))->type->methods[_Drop_]))((_Object) _Reference(Obj))

#define _Encode(Obj, A1, A2) \
    (*((_Encode_P) ((_Object) _Reference(Obj))->type->methods[_Encode_]))((_Object) _Reference(Obj), (A1), (A2))
#define _Encode_S(Obj, A1, A2, MyType) \
    (*((_Encode_P) (MyType)->supertype->methods[_Encode_]))((_Object) _Reference(Obj), (A1), (A2))

#define _EnumerateProperties(Obj) \
    (*((_EnumerateProperties_P) ((_Object) _Reference(Obj))->type->methods[_EnumerateProperties_]))((_Object) _Reference(Obj))
#define _EnumerateProperties_S(Obj, MyType) \
    (*((_EnumerateProperties_P) (MyType)->supertype->methods[_EnumerateProperties_]))((_Object) _Reference(Obj))

#define _Export(Obj, A1, A2) \
    (*((_Export_P) ((_Object) _Reference(Obj))->type->methods[_Export_]))((_Object) _Reference(Obj), (A1), (A2))

#define _Expunge(Obj) \
    (*((_Expunge_P) ((_Object) _Reference(Obj))->type->methods[_Expunge_]))((_Object) _Reference(Obj))
#define _Expunge_S(Obj, MyType) \
    (*((_Expunge_P) (MyType)->supertype->methods[_Expunge_]))((_Object) _Reference(Obj))

#define _Free(Obj) \
    (*((_Free_P) ((_Object) _Reference(Obj))->type->methods[_Free_]))((_Object) _Reference(Obj))
#define _Free_S(Obj, MyType) \
    (*((_Free_P) (MyType)->supertype->methods[_Free_]))((_Object) _Reference(Obj))

#define _GetDomain(Obj) \
    (*((_GetDomain_P) ((_Object) _Reference(Obj))->type->methods[_GetDomain_]))((_Object) _Reference(Obj))

#define _GetObjectDesc(Obj) \
    (*((_GetObjectDesc_P) ((_Object) _Reference(Obj))->type->methods[_GetObjectDesc_]))((_Object) _Reference(Obj))

#define _GetObjectId(Obj) \
    (*((_GetObjectId_P) ((_Object) _Reference(Obj))->type->methods[_GetObjectId_]))((_Object) _Reference(Obj))
#define _GetObjectId_S(Obj, MyType) \
    (*((_GetObjectId_P) (MyType)->supertype->methods[_GetObjectId_]))((_Object) _Reference(Obj))

#define _GetValue(Obj, A1, A2, A3) \
    (*((_GetValue_P) ((_Object) _Reference(Obj))->type->methods[_GetValue_]))((_Object) _Reference(Obj), (A1), (A2), (A3))
#define _GetValue_S(Obj, A1, A2, A3, MyType) \
    (*((_GetValue_P) (MyType)->supertype->methods[_GetValue_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _GetValueDomain(Obj, A1) \
    (*((_GetValueDomain_P) ((_Object) _Reference(Obj))->type->methods[_GetValueDomain_]))((_Object) _Reference(Obj), (A1))
#define _GetValueDomain_S(Obj, A1, MyType) \
    (*((_GetValueDomain_P) (MyType)->supertype->methods[_GetValueDomain_]))((_Object) _Reference(Obj), (A1))

#define _GetValueList(Obj, A1) \
    (*((_GetValueList_P) ((_Object) _Reference(Obj))->type->methods[_GetValueList_]))((_Object) _Reference(Obj), (A1))
#define _GetValueList_S(Obj, A1, MyType) \
    (*((_GetValueList_P) (MyType)->supertype->methods[_GetValueList_]))((_Object) _Reference(Obj), (A1))

#define _Highlight(Obj) \
    (*((_Highlight_P) ((_Object) _Reference(Obj))->type->methods[_Highlight_]))((_Object) _Reference(Obj))

#define _Import(Type, A1) \
    (*((_Import_P) (Type)->methods[_Import_]))((Type), (A1))

#define _ImportKeyProperties(Type, A1, A2) \
    (*((_ImportKeyProperties_P) (Type)->methods[_ImportKeyProperties_]))((Type), (A1), (A2))

#define _Initialize(Obj, A1, A2) \
    (*((_Initialize_P) ((_Object) _Reference(Obj))->type->methods[_Initialize_]))((_Object) _Reference(Obj), ((_Object) (A1)), (A2))
#define _Initialize_S(Obj, A1, A2, MyType) \
    (*((_Initialize_P) (MyType)->supertype->methods[_Initialize_]))((_Object) _Reference(Obj), ((_Object) (A1)), (A2))

#define _IsMultiValued(Obj, A1) \
    (*((_IsMultiValued_P) ((_Object) _Reference(Obj))->type->methods[_IsMultiValued_]))((_Object) _Reference(Obj), (A1))
#define _IsMultiValued_S(Obj, A1, MyType) \
    (*((_IsMultiValued_P) (MyType)->supertype->methods[_IsMultiValued_]))((_Object) _Reference(Obj), (A1))

#define _IsNamed(Obj, A1) \
    (*((_IsNamed_P) ((_Object) _Reference(Obj))->type->methods[_IsNamed_]))((_Object) _Reference(Obj), (A1))

#define _IsType(Obj, A1) \
    (*((_IsType_P) ((_Object) _Reference(Obj))->type->methods[_IsType_]))((_Object) _Reference(Obj), (A1))

#define _Iterate(Obj, A1, A2, A3) \
    (*((_Iterate_P) ((_Object) _Reference(Obj))->type->methods[_Iterate_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _Navigate(Obj, A1, A2, A3, A4, A5) \
    (*((_Navigate_P) ((_Object) _Reference(Obj))->type->methods[_Navigate_]))((_Object) _Reference(Obj), (A1), (A2), (A3), (A4), (A5))

#define _Open(Type, A1, A2) \
    (*((_Open_P) (Type)->methods[_Open_]))((Type), (A1), (A2))

#define _Query(Obj, A1, A2, A3, A4) \
    (*((_Query_P) ((_Object) _Reference(Obj))->type->methods[_Query_]))((_Object) _Reference(Obj), (A1), (A2), (A3), (A4))

#define _RemoveElement(Obj, A1, A2) \
    (*((_RemoveElement_P) ((_Object) _Reference(Obj))->type->methods[_RemoveElement_]))((_Object) _Reference(Obj), (A1), (A2))

#define _Retrieve(Obj) \
    (*((_Retrieve_P) ((_Object) _Reference(Obj))->type->methods[_Retrieve_]))((_Object) _Reference(Obj))

#define _SelectElement(Obj, A1, A2, A3) \
    (*((_SelectElement_P) ((_Object) _Reference(Obj))->type->methods[_SelectElement_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _SelectMenu(Obj, A1, A2) \
    (*((_SelectMenu_P) ((_Object) _Reference(Obj))->type->methods[_SelectMenu_]))((_Object) _Reference(Obj), (A1), (A2))

#define _Selected(Obj, A1) \
    (*((_Selected_P) ((_Object) _Reference(Obj))->type->methods[_Selected_]))((_Object) _Reference(Obj), (A1))

#define _SendMessage(Obj, A1, A2, A3) \
    (*((_SendMessage_P) ((_Object) _Reference(Obj))->type->methods[_SendMessage_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _SetState(Obj, A1, A2) \
    (*((_SetState_P) ((_Object) _Reference(Obj))->type->methods[_SetState_]))((_Object) _Reference(Obj), (A1), (A2))

#define _SetValue(Obj, A1, A2, A3, A4) \
    (*((_SetValue_P) ((_Object) _Reference(Obj))->type->methods[_SetValue_]))((_Object) _Reference(Obj), (A1), (A2), (A3), (A4))
#define _SetValue_S(Obj, A1, A2, A3, A4, MyType) \
    (*((_SetValue_P) (MyType)->supertype->methods[_SetValue_]))((_Object) _Reference(Obj), (A1), (A2), (A3), (A4))

#define _SetValueList(Obj, A1, A2, A3) \
    (*((_SetValueList_P) ((_Object) _Reference(Obj))->type->methods[_SetValueList_]))((_Object) _Reference(Obj), (A1), (A2), (A3))
#define _SetValueList_S(Obj, A1, A2, A3, MyType) \
    (*((_SetValueList_P) (MyType)->supertype->methods[_SetValueList_]))((_Object) _Reference(Obj), (A1), (A2), (A3))

#define _ShowHistory(Obj, A1, A2, A3, A4, A5, A6, A7) \
    (*((_ShowHistory_P) ((_Object) _Reference(Obj))->type->methods[_ShowHistory_]))((_Object) _Reference(Obj), (A1), (A2), (A3), (A4), (A5), (A6), (A7))

#define _ShowLinks(Obj, A1, A2, A3, A4, A5, A6, A7, A8) \
    (*((_ShowLinks_P) ((_Object) _Reference(Obj))->type->methods[_ShowLinks_]))((_Object) _Reference(Obj), (A1), (A2), (A3), (A4), (A5), (A6), (A7), (A8))

#define _Store(Obj, A1) \
    (*((_Store_P) ((_Object) _Reference(Obj))->type->methods[_Store_]))((_Object) _Reference(Obj), (A1))

#define _SurrogateIsHighlighted(Obj, A1) \
    (*((_SurrogateIsHighlighted_P) ((_Object) _Reference(Obj))->type->methods[_SurrogateIsHighlighted_]))((_Object) _Reference(Obj), (A1))

#define _Transact(Obj, A1) \
    (*((_Transact_P) ((_Object) _Reference(Obj))->type->methods[_Transact_]))((_Object) _Reference(Obj), (A1))

#define _Verify(Obj, A1, A2) \
    (*((_Verify_P) ((_Object) _Reference(Obj))->type->methods[_Verify_]))((_Object) _Reference(Obj), (A1), (A2))
