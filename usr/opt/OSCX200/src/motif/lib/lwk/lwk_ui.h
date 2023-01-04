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
** Private Type Definitions for the Ui Object
*/

/*
** Abstract Type Definitions for subtypes of Object
*/

typedef _Object _List, _PtrTo _ListPtr;
typedef _Object _Set, _PtrTo _SetPtr;
typedef _Object _Property, _PtrTo _PropertyPtr;
typedef _Object _ObjectId, _PtrTo _ObjectIdPtr;
typedef _Object _ObjectDesc, _PtrTo _ObjectDescPtr;
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
** Ui Instance Structure Definition
*/

typedef struct __Ui {
        _Type type;
        _Set_of(_String) surrogate_types;
        _Set_of(_String) operations;
        _String default_operation;
        _HighlightFlags default_highlight;
        _DDIFString default_relationship;
        _Boolean default_retain_source;
        _Boolean default_retain_target;
        _ObjectId active_clinknet;
        _ObjectId active_cpath;
        _ObjectId recording_linknet;
        _ObjectId active_path;
        _Integer active_path_index;
        _ObjectId recording_path;
        _ObjectId new_link;
        _ObjectId followed_link;
        _ObjectId followed_step;
        _ObjectId pending_source;
        _ObjectId pending_target;
        _ObjectId follow_destination;
        _HighlightFlags appl_highlight;
        _Boolean retain_source;
        _Boolean retain_target;
        _Closure user_data;
        _Callback get_surrogate_cb;
        _Callback create_surrogate_cb;
        _Callback close_view_cb;
        _Callback apply_cb;
        _Callback complete_link_cb;
        _Callback environment_change_cb;
        _DXmEnvState environment_state;
        _Boolean environment_manager;
        _AnyPtr client_list;
        _Path history;
        _Integer step_destination;
        _Link pending_link;
        _Integer pending_source_owner;
        _Integer pending_target_owner;
    } _UiInstance, _PtrTo _Ui, _PtrTo _PtrTo _UiPtr;

/*
** Ui Property Name Table Index Definitions
*/

#define _SupportedSurrogatesIndex 0
#define _SupportedOperationsIndex 1
#define _DefaultOperationIndex 2
#define _DefaultHighlightIndex 3
#define _DefaultRelationshipIndex 4
#define _DefaultRetainSourceIndex 5
#define _DefaultRetainTargetIndex 6
#define _ActiveCompLinknetIndex 7
#define _ActiveCompPathIndex 8
#define _RecordingLinknetIndex 9
#define _ActivePathIndex 10
#define _ActivePathIndexIndex 11
#define _RecordingPathIndex 12
#define _NewLinkIndex 13
#define _FollowedLinkIndex 14
#define _FollowedStepIndex 15
#define _PendingSourceIndex 16
#define _PendingTargetIndex 17
#define _FollowDestinationIndex 18
#define _ApplHighlightIndex 19
#define _RetainSourceIndex 20
#define _RetainTargetIndex 21
#define _UserDataIndex 22
#define _GetSurrogateCbIndex 23
#define _CreateSurrogateCbIndex 24
#define _CloseViewCbIndex 25
#define _ApplyCbIndex 26
#define _CompleteLinkCbIndex 27
#define _EnvironmentChangeCbIndex 28
#define _EnvironmentStateIndex 29
#define _EnvironmentManagerIndex 30
#define _P_ClientList "$ClientList"
#define _ClientListIndex 31
#define _P_History "$History"
#define _HistoryIndex 32
#define _P_StepDestination "$StepDestination"
#define _StepDestinationIndex 33
#define _P_PendingLink "$PendingLink"
#define _PendingLinkIndex 34
#define _P_PendingSourceOwner "$PendingSourceOwner"
#define _PendingSourceOwnerIndex 35
#define _P_PendingTargetOwner "$PendingTargetOwner"
#define _PendingTargetOwnerIndex 36

/*
** Ui Property Name Table Definition
*/

#define _UiPropertyNameTable \
    _PropertyNameTableEntry _Constant UiPropertyNameTable[] = { \
        {_P_SupportedSurrogates, _SupportedSurrogatesIndex, _True}, \
        {_P_SupportedOperations, _SupportedOperationsIndex, _True}, \
        {_P_DefaultOperation, _DefaultOperationIndex, _True}, \
        {_P_DefaultHighlight, _DefaultHighlightIndex, _True}, \
        {_P_DefaultRelationship, _DefaultRelationshipIndex, _True}, \
        {_P_DefaultRetainSource, _DefaultRetainSourceIndex, _True}, \
        {_P_DefaultRetainTarget, _DefaultRetainTargetIndex, _True}, \
        {_P_ActiveCompLinknet, _ActiveCompLinknetIndex, _True}, \
        {_P_ActiveCompPath, _ActiveCompPathIndex, _True}, \
        {_P_RecordingLinknet, _RecordingLinknetIndex, _True}, \
        {_P_ActivePath, _ActivePathIndex, _True}, \
        {_P_ActivePathIndex, _ActivePathIndexIndex, _True}, \
        {_P_RecordingPath, _RecordingPathIndex, _True}, \
        {_P_NewLink, _NewLinkIndex, _True}, \
        {_P_FollowedLink, _FollowedLinkIndex, _True}, \
        {_P_FollowedStep, _FollowedStepIndex, _True}, \
        {_P_PendingSource, _PendingSourceIndex, _True}, \
        {_P_PendingTarget, _PendingTargetIndex, _True}, \
        {_P_FollowDestination, _FollowDestinationIndex, _True}, \
        {_P_ApplHighlight, _ApplHighlightIndex, _True}, \
        {_P_RetainSource, _RetainSourceIndex, _True}, \
        {_P_RetainTarget, _RetainTargetIndex, _True}, \
        {_P_UserData, _UserDataIndex, _True}, \
        {_P_GetSurrogateCb, _GetSurrogateCbIndex, _True}, \
        {_P_CreateSurrogateCb, _CreateSurrogateCbIndex, _True}, \
        {_P_CloseViewCb, _CloseViewCbIndex, _True}, \
        {_P_ApplyCb, _ApplyCbIndex, _True}, \
        {_P_CompleteLinkCb, _CompleteLinkCbIndex, _True}, \
        {_P_EnvironmentChangeCb, _EnvironmentChangeCbIndex, _True}, \
        {_P_EnvironmentState, _EnvironmentStateIndex, _True}, \
        {_P_EnvironmentManager, _EnvironmentManagerIndex, _True}, \
        {_P_ClientList, _ClientListIndex, _False}, \
        {_P_History, _HistoryIndex, _False}, \
        {_P_StepDestination, _StepDestinationIndex, _False}, \
        {_P_PendingLink, _PendingLinkIndex, _False}, \
        {_P_PendingSourceOwner, _PendingSourceOwnerIndex, _False}, \
        {_P_PendingTargetOwner, _PendingTargetOwnerIndex, _False}, \
        {(_String) 0, 0, _False} \
    }

/*
** Ui Property Accessor Definitions
*/

#define _SupportedSurrogates_of(Obj) ((_Ui) _Reference(Obj))->surrogate_types
#define _SupportedOperations_of(Obj) ((_Ui) _Reference(Obj))->operations
#define _DefaultOperation_of(Obj) ((_Ui) _Reference(Obj))->default_operation
#define _DefaultHighlight_of(Obj) ((_Ui) _Reference(Obj))->default_highlight
#define _DefaultRelationship_of(Obj) ((_Ui) _Reference(Obj))->default_relationship
#define _DefaultRetainSource_of(Obj) ((_Ui) _Reference(Obj))->default_retain_source
#define _DefaultRetainTarget_of(Obj) ((_Ui) _Reference(Obj))->default_retain_target
#define _ActiveCompLinknet_of(Obj) ((_Ui) _Reference(Obj))->active_clinknet
#define _ActiveCompPath_of(Obj) ((_Ui) _Reference(Obj))->active_cpath
#define _RecordingLinknet_of(Obj) ((_Ui) _Reference(Obj))->recording_linknet
#define _ActivePath_of(Obj) ((_Ui) _Reference(Obj))->active_path
#define _ActivePathIndex_of(Obj) ((_Ui) _Reference(Obj))->active_path_index
#define _RecordingPath_of(Obj) ((_Ui) _Reference(Obj))->recording_path
#define _NewLink_of(Obj) ((_Ui) _Reference(Obj))->new_link
#define _FollowedLink_of(Obj) ((_Ui) _Reference(Obj))->followed_link
#define _FollowedStep_of(Obj) ((_Ui) _Reference(Obj))->followed_step
#define _PendingSource_of(Obj) ((_Ui) _Reference(Obj))->pending_source
#define _PendingTarget_of(Obj) ((_Ui) _Reference(Obj))->pending_target
#define _FollowDestination_of(Obj) ((_Ui) _Reference(Obj))->follow_destination
#define _ApplHighlight_of(Obj) ((_Ui) _Reference(Obj))->appl_highlight
#define _RetainSource_of(Obj) ((_Ui) _Reference(Obj))->retain_source
#define _RetainTarget_of(Obj) ((_Ui) _Reference(Obj))->retain_target
#define _UserData_of(Obj) ((_Ui) _Reference(Obj))->user_data
#define _GetSurrogateCb_of(Obj) ((_Ui) _Reference(Obj))->get_surrogate_cb
#define _CreateSurrogateCb_of(Obj) ((_Ui) _Reference(Obj))->create_surrogate_cb
#define _CloseViewCb_of(Obj) ((_Ui) _Reference(Obj))->close_view_cb
#define _ApplyCb_of(Obj) ((_Ui) _Reference(Obj))->apply_cb
#define _CompleteLinkCb_of(Obj) ((_Ui) _Reference(Obj))->complete_link_cb
#define _EnvironmentChangeCb_of(Obj) ((_Ui) _Reference(Obj))->environment_change_cb
#define _EnvironmentState_of(Obj) ((_Ui) _Reference(Obj))->environment_state
#define _EnvironmentManager_of(Obj) ((_Ui) _Reference(Obj))->environment_manager
#define _ClientList_of(Obj) ((_Ui) _Reference(Obj))->client_list
#define _History_of(Obj) ((_Ui) _Reference(Obj))->history
#define _StepDestination_of(Obj) ((_Ui) _Reference(Obj))->step_destination
#define _PendingLink_of(Obj) ((_Ui) _Reference(Obj))->pending_link
#define _PendingSourceOwner_of(Obj) ((_Ui) _Reference(Obj))->pending_source_owner
#define _PendingTargetOwner_of(Obj) ((_Ui) _Reference(Obj))->pending_target_owner

/*
** Include Generic Operation Support
*/

#include "lwk_operation_prototypes.h"

/*
** Ui Method Declarations
*/

_DeclareFunction(void LwkOpObjIllOp, (_in _Object object));
_DeclareFunction(void LwkOpUiNavApply, (_in _Ui ui, _in _String operation,
    _in _Surrogate surrogate));
_DeclareFunction(void LwkOpUiNavConfirmApply, (
    _in _Ui ui, _in _Surrogate surrogate));
_DeclareFunction(void LwkOpUiNavConfirmLink, (
    _in _Ui ui, _in _Boolean confirmed,
    _in _Boolean retain_source, _in _Boolean retain_target));
_DeclareFunction(_Ui LwkOpObjCopy, (_in _Ui object, _in _Boolean aggreate));
_DeclareFunction(_Ui LwkOpObjCreate, (_in _Type type));
_DeclareFunction(_Set LwkOpUiEnumProps, (_in _Ui object));
_DeclareFunction(void LwkOpObjExpunge, (_inout _Ui object));
_DeclareFunction(void LwkOpUiFree, (_inout _Ui object));
_DeclareFunction(_Domain LwkOpObjGetDomain, (_in _Ui object));
_DeclareFunction(void LwkOpUiGetValue, (_in _Ui object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value));
_DeclareFunction(_Domain LwkOpUiGetValueDomain, (_in _Ui object,
    _in _String property_name));
_DeclareFunction(_List LwkOpUiGetValueList, (_in _Ui object,
    _in _String property_name));
_DeclareFunction(void LwkOpUiInitialize, (_inout _Ui object,
    _in _Ui proto_object, _Boolean aggregate));
_DeclareFunction(_Boolean LwkOpUiIsMultiValued, (_in _Ui object,
    _in _String property_name));
_DeclareFunction(_Boolean LwkOpObjIsType, (_in _Ui object,
    _in _Type type));
_DeclareFunction(void LwkOpUiNavNavigate, (_in _Ui ui,
    _in _Link link, _in _Surrogate origin,
    _in _Surrogate destination, _in _String Operation,
    _in _FollowType follow_type));
_DeclareFunction(void LwkOpUiNavSelectMenu, (_inout _Ui ui,
    _in _Menu menu, _in _Closure closure));
_DeclareFunction(void LwkOpUiSetValue, (_inout _Ui object,
    _in _String property_name, _in _Domain domain,
    _in _AnyPtr value, _in _SetFlag flag));
_DeclareFunction(void LwkOpUiSetValueList, (_inout _Ui object,
    _in _String property_name, _in _List_of(_Domain) value_list,
    _in _SetFlag flag));
_DeclareFunction(_Boolean LwkOpUiNavSurrIsHighlighted, (
    _in _Ui ui, _in _Surrogate surrogate));

/*
** Ui Type Instance Definition
*/

#define _UiTypeInstance \
    _TypeInstance _Constant LWK__UiTypeInstance = { \
        &LWK__ObjectTypeInstance, \
        0, \
        { \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiNavApply, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiNavConfirmApply, \
        (_Method) LwkOpUiNavConfirmLink, \
        (_Method) LwkOpObjCopy, \
        (_Method) LwkOpObjCreate, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiEnumProps, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjExpunge, \
        (_Method) LwkOpUiFree, \
        (_Method) LwkOpObjGetDomain, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiGetValue, \
        (_Method) LwkOpUiGetValueDomain, \
        (_Method) LwkOpUiGetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiInitialize, \
        (_Method) LwkOpUiIsMultiValued, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIsType, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiNavNavigate, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiNavSelectMenu, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiSetValue, \
        (_Method) LwkOpUiSetValueList, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpUiNavSurrIsHighlighted, \
        (_Method) LwkOpObjIllOp, \
        (_Method) LwkOpObjIllOp \
        } \
    }

#define _UiType \
    _Type _Constant LWK__UiType = &LWK__UiTypeInstance
