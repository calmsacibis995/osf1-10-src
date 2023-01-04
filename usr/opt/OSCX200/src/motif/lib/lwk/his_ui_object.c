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
**	User Interface object methods 
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

#include "his_include.h"
#include "lwk_ui.h"
#include "his_ui_navigation.h"

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void SetHighlight, (_Ui ui, _HighlightFlags old));
_DeclareFunction(static void StoreCurrentObjectId,
    (_Ui ui, _Integer index, _Domain prop_domain, _ObjectId *property,
	_Domain val_domain, _AnyPtr value, _SetFlag flag,
	_Boolean value_is_list));
_DeclareFunction(static void RetrieveObject,
    (_Ui ui, _ObjectId *oid, _Integer property, _Domain prop_domain,
	_AnyPtr value, _Domain val_domain, _Boolean list));
_DeclareFunction(static void StoreValue,
    (_Ui ui, _Integer index, _Domain prop_domain, _AnyPtr property,
	_Domain val_domain, _AnyPtr value, _SetFlag flag,
	_Boolean value_is_list));
_DeclareFunction(static void RetrieveValue,
    (_Ui ui, _AnyPtr *property, _Integer index, _Domain prop_domain,
	_AnyPtr value, _Domain val_domain, _Boolean list));

/*
**  Global Data Definitions
*/

_Global _UiTypeInstance;      /* Instance record for Type */
_Global _UiType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeUiInstance;
static _UiPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties = UiPropertyNameTable;

/*
**  Table to map Currency indices into Currency Object Property Names
*/

static _PropertyIndicesToNames;


void  LwkOpUi(ui)
_Ui ui;

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


void  LwkOpUiInitialize(ui, proto, aggregate)
_Ui ui;
 _Ui proto;
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
    _Integer highlight;
    _Boolean retain_source;

    /*
    **  Initialize the properties defined by our type
    */

    highlight = lwk_c_hl_none;
    _CopyValue(&highlight, &_ApplHighlight_of(ui), lwk_c_domain_integer);
    _CopyValue(&highlight, &_DefaultHighlight_of(ui), lwk_c_domain_integer);

    _ClearValue(&_SupportedSurrogates_of(ui), lwk_c_domain_set);
    _ClearValue(&_SupportedOperations_of(ui), lwk_c_domain_set);
    _ClearValue(&_DefaultOperation_of(ui), lwk_c_domain_string);
    _ClearValue(&_DefaultRelationship_of(ui), lwk_c_domain_ddif_string);
    _ClearValue(&_DefaultRetainSource_of(ui), lwk_c_domain_boolean);
    _ClearValue(&_DefaultRetainTarget_of(ui), lwk_c_domain_boolean);
    _ClearValue(&_UserData_of(ui), lwk_c_domain_closure);
    _ClearValue(&_ActiveCompLinknet_of(ui), lwk_c_domain_comp_linknet);
    _ClearValue(&_RecordingLinknet_of(ui), lwk_c_domain_linknet);
    _ClearValue(&_ActivePath_of(ui), lwk_c_domain_path);
    _ClearValue(&_ActivePathIndex_of(ui), lwk_c_domain_integer);
    _ClearValue(&_ActiveCompPath_of(ui), lwk_c_domain_comp_path);
    _ClearValue(&_RecordingPath_of(ui), lwk_c_domain_path);
    _ClearValue(&_FollowedStep_of(ui), lwk_c_domain_step);
    _ClearValue(&_NewLink_of(ui), lwk_c_domain_link);
    _ClearValue(&_PendingSource_of(ui), lwk_c_domain_surrogate);
    _ClearValue(&_PendingTarget_of(ui), lwk_c_domain_surrogate);
    _ClearValue(&_FollowedLink_of(ui), lwk_c_domain_link);
    _ClearValue(&_FollowDestination_of(ui), lwk_c_domain_surrogate);
    _ClearValue(&_RetainSource_of(ui), lwk_c_domain_boolean);
    _ClearValue(&_RetainTarget_of(ui), lwk_c_domain_boolean);
    _ClearValue(&_GetSurrogateCb_of(ui), lwk_c_domain_routine);
    _ClearValue(&_CreateSurrogateCb_of(ui), lwk_c_domain_routine);
    _ClearValue(&_CloseViewCb_of(ui), lwk_c_domain_routine);
    _ClearValue(&_ApplyCb_of(ui), lwk_c_domain_routine);
    _ClearValue(&_CompleteLinkCb_of(ui), lwk_c_domain_routine);
    _ClearValue(&_EnvironmentChangeCb_of(ui), lwk_c_domain_routine);
    _ClearValue(&_EnvironmentState_of(ui), lwk_c_domain_environment_state);
    _ClearValue(&_EnvironmentManager_of(ui), lwk_c_domain_boolean);

    _ClientList_of(ui) = (_AnyPtr) 0;
    _ClearValue(&_History_of(ui), lwk_c_domain_path);
    _ClearValue(&_StepDestination_of(ui), lwk_c_domain_integer);
    _ClearValue(&_PendingLink_of(ui), lwk_c_domain_link);
    _ClearValue(&_PendingSourceOwner_of(ui), lwk_c_domain_integer);
    _ClearValue(&_PendingTargetOwner_of(ui), lwk_c_domain_integer);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(ui, proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Ui) _NullObject) {
	_CopyValue(&_SupportedSurrogates_of(proto),
	    &_SupportedSurrogates_of(ui), lwk_c_domain_set);
	_CopyValue(&_SupportedOperations_of(proto),
	    &_SupportedOperations_of(ui), lwk_c_domain_set);
	_CopyValue(&_DefaultOperation_of(proto),
	    &_DefaultOperation_of(ui), lwk_c_domain_string);
	_CopyValue(&_ApplHighlight_of(proto),
	    &_ApplHighlight_of(ui), lwk_c_domain_integer);
	_CopyValue(&_DefaultRelationship_of(proto),
	    &_DefaultRelationship_of(ui), lwk_c_domain_ddif_string);
	_CopyValue(&_DefaultRetainSource_of(proto),
	    &_DefaultRetainSource_of(ui), lwk_c_domain_boolean);
	_CopyValue(&_DefaultRetainTarget_of(proto),
	    &_DefaultRetainTarget_of(ui), lwk_c_domain_boolean);
	_CopyValue(&_DefaultHighlight_of(proto),
	    &_DefaultHighlight_of(ui), lwk_c_domain_integer);
	_CopyValue(&_ActiveCompLinknet_of(proto),
	    &_ActiveCompLinknet_of(ui), lwk_c_domain_comp_linknet);
	_CopyValue(&_RecordingLinknet_of(proto),
	    &_RecordingLinknet_of(ui), lwk_c_domain_linknet);
	_CopyValue(&_ActivePath_of(proto),
	    &_ActivePath_of(ui), lwk_c_domain_path);
	_CopyValue(&_ActivePathIndex_of(proto),
	    &_ActivePathIndex_of(ui), lwk_c_domain_integer);
	_CopyValue(&_ActiveCompPath_of(proto),
	    &_ActiveCompPath_of(ui), lwk_c_domain_comp_path);
	_CopyValue(&_RecordingPath_of(proto),
	    &_RecordingPath_of(ui), lwk_c_domain_path);
	_CopyValue(&_FollowedStep_of(proto),
	    &_FollowedStep_of(ui), lwk_c_domain_step);
	_CopyValue(&_NewLink_of(proto),
	    &_NewLink_of(ui), lwk_c_domain_link);
	_CopyValue(&_PendingSource_of(proto),
	    &_PendingSource_of(ui), lwk_c_domain_surrogate);
	_CopyValue(&_PendingTarget_of(proto),
	    &_PendingTarget_of(ui), lwk_c_domain_surrogate);
	_CopyValue(&_FollowedLink_of(proto),
	    &_FollowedLink_of(ui), lwk_c_domain_link);
	_CopyValue(&_FollowDestination_of(proto),
	    &_FollowDestination_of(ui), lwk_c_domain_surrogate);
	_CopyValue(&_RetainSource_of(proto),
	    &_RetainSource_of(ui), lwk_c_domain_boolean);
	_CopyValue(&_RetainTarget_of(proto),
	    &_RetainTarget_of(ui), lwk_c_domain_boolean);
	_CopyValue(&_UserData_of(proto),
	    &_UserData_of(ui), lwk_c_domain_closure);
	_CopyValue(&_GetSurrogateCb_of(proto),
	    &_GetSurrogateCb_of(ui), lwk_c_domain_routine);
	_CopyValue(&_CreateSurrogateCb_of(proto),
	    &_CreateSurrogateCb_of(ui), lwk_c_domain_routine);
	_CopyValue(&_CloseViewCb_of(proto),
	    &_CloseViewCb_of(ui), lwk_c_domain_routine);
	_CopyValue(&_ApplyCb_of(proto),
	    &_ApplyCb_of(ui), lwk_c_domain_routine);
	_CopyValue(&_CompleteLinkCb_of(proto),
	    &_CompleteLinkCb_of(ui), lwk_c_domain_routine);
	_CopyValue(&_EnvironmentChangeCb_of(proto),
	    &_EnvironmentChangeCb_of(ui), lwk_c_domain_routine);
	_CopyValue(&_EnvironmentState_of(proto), &_EnvironmentState_of(ui),
	    lwk_c_domain_environment_state);
    }

    /*
    ** Set retain source to be the default for that property.
    */

    _GetValue(ui, _P_DefaultRetainSource, lwk_c_domain_boolean,
	&retain_source);
    _SetValue(ui, _P_RetainSource, lwk_c_domain_boolean, &retain_source,
	lwk_c_set_property);
    
    return;
    }


void  LwkOpUiFree(ui)
_Ui ui;

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
    **	If there is a Currency object, unregister any properties then Delete
    **	it.
    */

    if (_EnvironmentState_of(ui) != (_DXmEnvState) _NullObject) {
	LwkUiNavUnregisterUi(ui);
	_Delete(&_EnvironmentState_of(ui));
    }

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_SupportedSurrogates_of(ui), lwk_c_domain_set);
    _DeleteValue(&_SupportedOperations_of(ui), lwk_c_domain_set);
    _DeleteValue(&_DefaultOperation_of(ui), lwk_c_domain_string);
    _DeleteValue(&_DefaultHighlight_of(ui), lwk_c_domain_integer);
    _DeleteValue(&_DefaultRelationship_of(ui), lwk_c_domain_ddif_string);
    _DeleteValue(&_DefaultRetainSource_of(ui), lwk_c_domain_boolean);
    _DeleteValue(&_DefaultRetainTarget_of(ui), lwk_c_domain_boolean);
    _DeleteValue(&_UserData_of(ui), lwk_c_domain_closure);
    _DeleteValue(&_ApplHighlight_of(ui), lwk_c_domain_integer);
    _DeleteValue(&_ActiveCompLinknet_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_RecordingLinknet_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_ActivePath_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_ActivePathIndex_of(ui), lwk_c_domain_integer);
    _DeleteValue(&_ActiveCompPath_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_RecordingPath_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_FollowedStep_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_NewLink_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_PendingSource_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_PendingTarget_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_FollowedLink_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_FollowDestination_of(ui), lwk_c_domain_object_id);
    _DeleteValue(&_RetainSource_of(ui), lwk_c_domain_boolean);
    _DeleteValue(&_RetainTarget_of(ui), lwk_c_domain_boolean);
    _DeleteValue(&_GetSurrogateCb_of(ui), lwk_c_domain_routine);
    _DeleteValue(&_CreateSurrogateCb_of(ui), lwk_c_domain_routine);
    _DeleteValue(&_CloseViewCb_of(ui), lwk_c_domain_routine);
    _DeleteValue(&_ApplyCb_of(ui), lwk_c_domain_routine);
    _DeleteValue(&_CompleteLinkCb_of(ui), lwk_c_domain_routine);
    _DeleteValue(&_EnvironmentChangeCb_of(ui), lwk_c_domain_routine);
    _DeleteValue(&_EnvironmentManager_of(ui), lwk_c_domain_boolean);

    _Delete(&_History_of(ui));

    _DeleteValue(&_StepDestination_of(ui), lwk_c_domain_integer);
    _DeleteValue(&_PendingLink_of(ui), lwk_c_domain_link);
    _DeleteValue(&_PendingSourceOwner_of(ui), lwk_c_domain_integer);
    _DeleteValue(&_PendingTargetOwner_of(ui), lwk_c_domain_integer);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(ui, MyType);

    return;
    }


_Set  LwkOpUiEnumProps(ui)
_Ui ui;

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
    _Set set;

    /*
    **  Ask our supertype to enumerate its properties.
    */

    set = (_Set) _EnumerateProperties_S(ui, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpUiIsMultiValued(ui, property)
_Ui ui;
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
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	answer = _IsMultiValued_S(ui, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _DefaultOperationIndex :
	    case _DefaultHighlightIndex :
	    case _DefaultRelationshipIndex :
	    case _DefaultRetainSourceIndex :
	    case _DefaultRetainTargetIndex :
	    case _UserDataIndex :
	    case _ApplHighlightIndex :
	    case _ActiveCompLinknetIndex :
	    case _RecordingLinknetIndex :
	    case _ActivePathIndex :
	    case _ActiveCompPathIndex :
	    case _RecordingPathIndex :
	    case _FollowedStepIndex :
	    case _NewLinkIndex :
	    case _PendingSourceIndex :
	    case _PendingTargetIndex :
	    case _FollowedLinkIndex :
	    case _FollowDestinationIndex :
	    case _RetainSourceIndex :
	    case _RetainTargetIndex :
	    case _GetSurrogateCbIndex :
	    case _CreateSurrogateCbIndex :
	    case _CloseViewCbIndex :
	    case _ApplyCbIndex :
	    case _CompleteLinkCbIndex :
	    case _EnvironmentChangeCbIndex :
	    case _EnvironmentStateIndex :
	    case _EnvironmentManagerIndex :
	    case _ActivePathIndexIndex :
		answer = _False;
		break;

	    case _SupportedSurrogatesIndex :
	    case _SupportedOperationsIndex :
		answer = _True;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpUiGetValueDomain(ui, property)
_Ui ui;
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
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	domain = (_Domain) _GetValueDomain_S(ui, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _SupportedSurrogatesIndex :
	    case _SupportedOperationsIndex :
		domain = lwk_c_domain_set;
		break;

	    case _DefaultOperationIndex :
		domain = lwk_c_domain_string;
		break;

	    case _DefaultRelationshipIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _DefaultHighlightIndex :
	    case _ApplHighlightIndex :
	    case _ActivePathIndexIndex :
		domain = lwk_c_domain_integer;
		break;

	    case _UserDataIndex :
		domain = lwk_c_domain_closure;
		break;

	    case _ActiveCompLinknetIndex :
		domain = lwk_c_domain_comp_linknet;
		break;

	    case _RecordingLinknetIndex :
		domain = lwk_c_domain_linknet;
		break;

	    case _ActivePathIndex :
		domain = lwk_c_domain_path;
		break;

	    case _ActiveCompPathIndex :
		domain = lwk_c_domain_comp_path;
		break;

	    case _RecordingPathIndex :
		domain = lwk_c_domain_path;
		break;

	    case _FollowedStepIndex :
		domain = lwk_c_domain_step;
		break;

	    case _NewLinkIndex :
	    case _FollowedLinkIndex :
		domain = lwk_c_domain_link;
		break;

	    case _PendingSourceIndex :
	    case _PendingTargetIndex :
	    case _FollowDestinationIndex :
		domain = lwk_c_domain_surrogate;
		break;

	    case _GetSurrogateCbIndex :
	    case _CreateSurrogateCbIndex :
	    case _CloseViewCbIndex :
	    case _ApplyCbIndex :
	    case _CompleteLinkCbIndex :
	    case _EnvironmentChangeCbIndex :
		domain = lwk_c_domain_routine;
		break;

	    case _EnvironmentStateIndex :
		domain = lwk_c_domain_environment_state;
		break;

	    case _EnvironmentManagerIndex :
	    case _DefaultRetainSourceIndex :
	    case _DefaultRetainTargetIndex :
	    case _RetainSourceIndex :
	    case _RetainTargetIndex :
		domain = lwk_c_domain_boolean;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpUiGetValue(ui, property, domain, value)
_Ui ui;
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
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_GetValue_S(ui, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SupportedSurrogatesIndex :
		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_SupportedSurrogates_of(ui), value,
			lwk_c_domain_set);
		else
		    _Raise(inv_domain);

		break;

	    case _SupportedOperationsIndex :
		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_SupportedOperations_of(ui), value,
			lwk_c_domain_set);
		else
		    _Raise(inv_domain);

		break;

	    case _DefaultOperationIndex :
		RetrieveValue(ui, &_DefaultOperation_of(ui),
		    lwk_c_env_default_operation, lwk_c_domain_string, value,
		    domain, _False);

		break;

	    case _DefaultHighlightIndex :
		RetrieveValue(ui, &_DefaultHighlight_of(ui),
		    lwk_c_env_default_highlight, lwk_c_domain_integer, value,
		    domain, _False);

		break;

	    case _DefaultRelationshipIndex :
		RetrieveValue(ui, &_DefaultRelationship_of(ui),
		    lwk_c_env_default_relationship, lwk_c_domain_ddif_string,
		    value, domain, _False);

		break;

	    case _DefaultRetainSourceIndex :
		RetrieveValue(ui, &_DefaultRetainSource_of(ui),
		    lwk_c_env_default_retain_source, lwk_c_domain_boolean,
		    value, domain, _False);

		break;

	    case _DefaultRetainTargetIndex :
		RetrieveValue(ui, &_DefaultRetainTarget_of(ui),
		    lwk_c_env_default_retain_target, lwk_c_domain_boolean,
		    value, domain, _False);

		break;

	    case _ApplHighlightIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_ApplHighlight_of(ui), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _ActiveCompLinknetIndex :
		RetrieveObject(ui, &_ActiveCompLinknet_of(ui),
		    lwk_c_env_active_comp_linknet, lwk_c_domain_comp_linknet,
		    value, domain, _False);

		break;

	    case _RecordingLinknetIndex :
		RetrieveObject(ui, &_RecordingLinknet_of(ui),
		    lwk_c_env_recording_linknet, lwk_c_domain_linknet, value,
		    domain, _False);
		break;

	    case _ActivePathIndex :
		RetrieveObject(ui, &_ActivePath_of(ui), lwk_c_env_active_path,
		    lwk_c_domain_path, value, domain, _False);
		break;

	    case _ActivePathIndexIndex :
		RetrieveValue(ui, &_ActivePathIndex_of(ui),
		    lwk_c_env_active_path_index, lwk_c_domain_integer, value,
		    domain, _False);

		break;

	    case _ActiveCompPathIndex :
		RetrieveObject(ui, &_ActiveCompPath_of(ui),
		    lwk_c_env_active_comp_path, lwk_c_domain_comp_path,
		    value, domain, _False);
		break;

	    case _RecordingPathIndex :
		RetrieveObject(ui, &_RecordingPath_of(ui), lwk_c_env_recording_path,
		    lwk_c_domain_path, value, domain, _False);
		break;

	    case _FollowedStepIndex :
		RetrieveObject(ui, &_FollowedStep_of(ui), lwk_c_env_followed_step,
		    lwk_c_domain_step, value, domain, _False);
		break;

	    case _NewLinkIndex :
		RetrieveObject(ui, &_NewLink_of(ui),
		    lwk_c_env_new_link, lwk_c_domain_link, value,
		    domain, _False);
		break;

	    case _PendingSourceIndex :
		RetrieveObject(ui, &_PendingSource_of(ui), lwk_c_env_pending_source,
		    lwk_c_domain_surrogate, value, domain, _False);
		break;

	    case _PendingTargetIndex :
		RetrieveObject(ui, &_PendingTarget_of(ui), lwk_c_env_pending_target,
		    lwk_c_domain_surrogate, value, domain, _False);
		break;

	    case _FollowDestinationIndex :
		RetrieveObject(ui, &_FollowDestination_of(ui),
		    lwk_c_env_follow_destination, lwk_c_domain_surrogate, value,
		    domain, _False);
		break;

	    case _FollowedLinkIndex :
		RetrieveObject(ui, &_FollowedLink_of(ui),
		    lwk_c_env_followed_link, lwk_c_domain_link, value,
		    domain, _False);
		break;

	    case _RetainSourceIndex :
		RetrieveValue(ui, &_RetainSource_of(ui),
		    lwk_c_env_retain_source, lwk_c_domain_boolean, value,
		    domain, _False);

		break;

	    case _RetainTargetIndex :
		RetrieveValue(ui, &_RetainTarget_of(ui),
		    lwk_c_env_retain_target, lwk_c_domain_boolean, value,
		    domain, _False);

		break;

	    case _UserDataIndex :
		if (_IsDomain(domain, lwk_c_domain_closure))
		    _CopyValue(&_UserData_of(ui), value, lwk_c_domain_closure);
		else
		    _Raise(inv_domain);

		break;

	    case _GetSurrogateCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_GetSurrogateCb_of(ui), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _CreateSurrogateCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_CreateSurrogateCb_of(ui), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _CloseViewCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_CloseViewCb_of(ui), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _ApplyCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_ApplyCb_of(ui), value, lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _CompleteLinkCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_CompleteLinkCb_of(ui), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _EnvironmentChangeCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_EnvironmentChangeCb_of(ui), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _EnvironmentStateIndex :
		if (_IsDomain(domain, lwk_c_domain_environment_state))
		    _CopyValue(&_EnvironmentState_of(ui), value, lwk_c_domain_environment_state);
		else
		    _Raise(inv_domain);

		break;

	    case _EnvironmentManagerIndex :
		if (_IsDomain(domain, lwk_c_domain_boolean))
		    _CopyValue(&_EnvironmentManager_of(ui), value,
			lwk_c_domain_boolean);
		else
		    _Raise(inv_domain);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_List  LwkOpUiGetValueList(ui, property)
_Ui ui;
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

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	return (_List) _GetValueList_S(ui, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _SupportedSurrogatesIndex :
		_CopyValue(&_SupportedSurrogates_of(ui), &value_list,
			lwk_c_domain_set);
		break;

	    case _SupportedOperationsIndex :
		_CopyValue(&_SupportedOperations_of(ui), &value_list,
			lwk_c_domain_set);
		break;

	    case _DefaultOperationIndex :
		RetrieveValue(ui, &_DefaultOperation_of(ui),
		    lwk_c_env_default_operation, lwk_c_domain_string,
		    &value_list, lwk_c_domain_string, _True);

		break;

	    case _DefaultHighlightIndex :
		RetrieveValue(ui, &_DefaultHighlight_of(ui),
		    lwk_c_env_default_highlight, lwk_c_domain_integer,
		    &value_list, lwk_c_domain_string, _True);

		break;

	    case _DefaultRelationshipIndex :
		RetrieveValue(ui, &_DefaultRelationship_of(ui),
		    lwk_c_env_default_relationship, lwk_c_domain_ddif_string,
		    &value_list, lwk_c_domain_ddif_string, _True);

		break;

	    case _DefaultRetainSourceIndex :
		RetrieveValue(ui, &_DefaultRetainSource_of(ui),
		    lwk_c_env_default_retain_source, lwk_c_domain_boolean,
		    &value_list, lwk_c_domain_boolean, _True);

		break;

	    case _DefaultRetainTargetIndex :
		RetrieveValue(ui, &_DefaultRetainTarget_of(ui),
		    lwk_c_env_default_retain_target, lwk_c_domain_boolean,
		    &value_list, lwk_c_domain_boolean, _True);

		break;

	    case _ApplHighlightIndex :
		_ListValue(&_ApplHighlight_of(ui), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _ActiveCompLinknetIndex :
		RetrieveObject(ui, &_ActiveCompLinknet_of(ui),
		    lwk_c_env_active_comp_linknet, lwk_c_domain_comp_linknet,
		    &value_list, lwk_c_domain_comp_linknet, _True);
		break;

	    case _RecordingLinknetIndex :
		RetrieveObject(ui, &_RecordingLinknet_of(ui),
		    lwk_c_env_recording_linknet, lwk_c_domain_linknet, &value_list,
		    lwk_c_domain_linknet, _True);
		break;

	    case _ActivePathIndex :
		RetrieveObject(ui, &_ActivePath_of(ui),
		    lwk_c_env_active_path, lwk_c_domain_path, &value_list,
		    lwk_c_domain_path, _True);
		break;

	    case _ActivePathIndexIndex :
		RetrieveValue(ui, &_ActivePathIndex_of(ui),
		    lwk_c_env_active_path_index, lwk_c_domain_integer, value_list,
		    lwk_c_domain_integer, _False);

		break;

	    case _ActiveCompPathIndex :
		RetrieveObject(ui, &_ActiveCompPath_of(ui),
		    lwk_c_env_active_comp_path, lwk_c_domain_comp_path,
		    &value_list, lwk_c_domain_comp_path, _True);
		break;

	    case _RecordingPathIndex :
		RetrieveObject(ui, &_RecordingPath_of(ui), lwk_c_env_recording_path,
		    lwk_c_domain_path, &value_list, lwk_c_domain_path, _True);
		break;

	    case _FollowedStepIndex :
		RetrieveObject(ui, &_FollowedStep_of(ui), lwk_c_env_followed_step,
		    lwk_c_domain_step, &value_list, lwk_c_domain_step, _True);
		break;

	    case _NewLinkIndex :
		RetrieveObject(ui, &_NewLink_of(ui),
		    lwk_c_env_new_link, lwk_c_domain_link,
		    &value_list, lwk_c_domain_link, _True);
		break;

	    case _PendingSourceIndex :
		RetrieveObject(ui, &_PendingSource_of(ui),
		    lwk_c_env_pending_source, lwk_c_domain_surrogate, &value_list,
		    lwk_c_domain_surrogate, _True);
		break;

	    case _PendingTargetIndex :
		RetrieveObject(ui, &_PendingTarget_of(ui),
		    lwk_c_env_pending_target, lwk_c_domain_surrogate, &value_list,
		    lwk_c_domain_surrogate, _True);
		break;

	    case _FollowedLinkIndex :
		RetrieveObject(ui, &_FollowedLink_of(ui),
		    lwk_c_env_followed_link, lwk_c_domain_link, &value_list,
		    lwk_c_domain_link, _True);
		break;

	    case _FollowDestinationIndex :
		RetrieveObject(ui, &_FollowDestination_of(ui),
		    lwk_c_env_follow_destination, lwk_c_domain_surrogate,
		    &value_list, lwk_c_domain_surrogate, _True);
		break;

	    case _RetainSourceIndex :
		RetrieveValue(ui, &_RetainSource_of(ui),
		    lwk_c_env_retain_source, lwk_c_domain_boolean,
		    &value_list, lwk_c_domain_boolean, _True);

		break;

	    case _RetainTargetIndex :
		RetrieveValue(ui, &_RetainTarget_of(ui),
		    lwk_c_env_retain_target, lwk_c_domain_boolean,
		    &value_list, lwk_c_domain_boolean, _True);

		break;

	    case _UserDataIndex :
		_ListValue(&_UserData_of(ui), &value_list,
		    lwk_c_domain_closure);
		break;

	    case _GetSurrogateCbIndex :
		_ListValue(&_GetSurrogateCb_of(ui), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _CreateSurrogateCbIndex :
		_ListValue(&_CreateSurrogateCb_of(ui), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _CloseViewCbIndex :
		_ListValue(&_CloseViewCb_of(ui), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _ApplyCbIndex :
		_ListValue(&_ApplyCb_of(ui), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _CompleteLinkCbIndex :
		_ListValue(&_CompleteLinkCb_of(ui), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _EnvironmentChangeCbIndex :
		_ListValue(&_EnvironmentChangeCb_of(ui), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _EnvironmentStateIndex :
		_ListValue(&_EnvironmentState_of(ui), &value_list,
		    lwk_c_domain_environment_state);
		break;

	    case _EnvironmentManagerIndex :
		_ListValue(&_EnvironmentManager_of(ui), &value_list,
		    lwk_c_domain_boolean);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpUiSetValue(ui, property, domain, value, flag)
_Ui ui;
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
    _Persistent object;
    _HighlightFlags old;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(ui, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Set the requested property accordingly
	*/

	switch (index) {
	    case _SupportedSurrogatesIndex :
		_SetMultiValuedProperty(&_SupportedSurrogates_of(ui),
		    lwk_c_domain_string, domain, value, flag, _False, _True);

		LwkUiNavRegisterApply(ui);

		break;

	    case _SupportedOperationsIndex :
		_SetMultiValuedProperty(&_SupportedOperations_of(ui),
		    lwk_c_domain_string, domain, value, flag, _False, _True);

		LwkUiNavRegisterApply(ui);

		break;

	    case _DefaultOperationIndex :
		StoreValue(ui, lwk_c_env_default_operation, lwk_c_domain_string,
		    &_DefaultOperation_of(ui), domain, value, flag, _False);
		break;

	    case _DefaultHighlightIndex :
		StoreValue(ui, lwk_c_env_default_highlight, lwk_c_domain_integer,
		    &_DefaultHighlight_of(ui), domain, value, flag, _False);
		break;

	    case _DefaultRelationshipIndex :
		StoreValue(ui, lwk_c_env_default_relationship,
		    lwk_c_domain_ddif_string, &_DefaultRelationship_of(ui),
		    domain, value, flag, _False);
		break;

	    case _DefaultRetainSourceIndex :
		StoreValue(ui, lwk_c_env_default_retain_source,
		    lwk_c_domain_boolean, &_DefaultRetainSource_of(ui),
		    domain, value, flag, _False);
		break;

	    case _DefaultRetainTargetIndex :
		StoreValue(ui, lwk_c_env_default_retain_target,
		    lwk_c_domain_boolean, &_DefaultRetainTarget_of(ui),
		    domain, value, flag, _False);
		break;

	    case _ApplHighlightIndex :
		old = _ApplHighlight_of(ui);

		_SetSingleValuedProperty(&_ApplHighlight_of(ui),
		    lwk_c_domain_integer, domain, value, flag, _False);

		SetHighlight(ui, old);

		break;

	    case _ActiveCompLinknetIndex :
		StoreCurrentObjectId(ui, lwk_c_env_active_comp_linknet,
		    lwk_c_domain_comp_linknet, &_ActiveCompLinknet_of(ui),
		    domain, value, flag, _False);
		break;

	    case _RecordingLinknetIndex :
		StoreCurrentObjectId(ui, lwk_c_env_recording_linknet,
		    lwk_c_domain_linknet, &_RecordingLinknet_of(ui), domain,
		    value, flag, _False);
		break;

	    case _ActivePathIndex :
		StoreCurrentObjectId(ui, lwk_c_env_active_path, lwk_c_domain_path,
		    &_ActivePath_of(ui), domain, value, flag, _False);
		break;

	    case _ActivePathIndexIndex :
		StoreValue(ui, lwk_c_env_active_path_index, lwk_c_domain_integer,
		    &_ActivePathIndex_of(ui), domain, value, flag, _False);
		break;

	    case _ActiveCompPathIndex :
		StoreCurrentObjectId(ui, lwk_c_env_active_comp_path,
		    lwk_c_domain_comp_path, &_ActiveCompPath_of(ui),
		    domain, value, flag, _False);
		break;

	    case _RecordingPathIndex :
		StoreCurrentObjectId(ui, lwk_c_env_recording_path, lwk_c_domain_path,
		    &_RecordingPath_of(ui), domain, value, flag, _False);
		break;

	    case _FollowedStepIndex :
		StoreCurrentObjectId(ui, lwk_c_env_followed_step, lwk_c_domain_step,
		    &_FollowedStep_of(ui), domain, value, flag, _False);
		break;

	    case _NewLinkIndex :
		StoreCurrentObjectId(ui, lwk_c_env_new_link,
		    lwk_c_domain_link, &_NewLink_of(ui), domain,
		    value, flag, _False);
		break;

	    case _PendingSourceIndex :
		StoreCurrentObjectId(ui, lwk_c_env_pending_source,
		    lwk_c_domain_surrogate, &_PendingSource_of(ui), domain,
		    value, flag, _False);
		break;

	    case _PendingTargetIndex :
		StoreCurrentObjectId(ui, lwk_c_env_pending_target,
		    lwk_c_domain_surrogate, &_PendingTarget_of(ui), domain,
		    value, flag, _False);
		break;

	    case _FollowedLinkIndex :
		StoreCurrentObjectId(ui, lwk_c_env_followed_link,
		    lwk_c_domain_link, &_FollowedLink_of(ui), domain,
		    value, flag, _False);
		break;

	    case _FollowDestinationIndex :
		StoreCurrentObjectId(ui, lwk_c_env_follow_destination,
		    lwk_c_domain_surrogate, &_FollowDestination_of(ui), domain,
		    value, flag, _False);
		break;

	    case _RetainSourceIndex :
		StoreValue(ui, lwk_c_env_retain_source,
		    lwk_c_domain_boolean, &_RetainSource_of(ui),
		    domain, value, flag, _False);
		break;

	    case _RetainTargetIndex :
		StoreValue(ui, lwk_c_env_retain_target,
		    lwk_c_domain_boolean, &_RetainTarget_of(ui),
		    domain, value, flag, _False);
		break;

	    case _UserDataIndex :
		_SetSingleValuedProperty(&_UserData_of(ui),
		    lwk_c_domain_closure, domain, value, flag, _False);
		break;

	    case _GetSurrogateCbIndex :
		_SetSingleValuedProperty(&_GetSurrogateCb_of(ui),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _CreateSurrogateCbIndex :
		_SetSingleValuedProperty(&_CreateSurrogateCb_of(ui),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _CloseViewCbIndex :
		_SetSingleValuedProperty(&_CloseViewCb_of(ui),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _ApplyCbIndex :
		_SetSingleValuedProperty(&_ApplyCb_of(ui),
		    lwk_c_domain_routine, domain, value, flag, _False);

		LwkUiNavRegisterApply(ui);

		break;

	    case _CompleteLinkCbIndex :
		_SetSingleValuedProperty(&_CompleteLinkCb_of(ui),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _EnvironmentChangeCbIndex :
		_SetSingleValuedProperty(&_EnvironmentChangeCb_of(ui),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _EnvironmentStateIndex :
		_SetSingleValuedProperty(&_EnvironmentState_of(ui),
		    lwk_c_domain_environment_state, domain, value, flag, _False);

		LwkUiNavEstablishCurrency(ui);

		break;

	    case _EnvironmentManagerIndex :
		_SetSingleValuedProperty(&_EnvironmentManager_of(ui),
		    lwk_c_domain_boolean, domain, value, flag, _False);

		LwkUiNavEstablishEnvManager(ui);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpUiSetValueList(ui, property, values, flag)
_Ui ui;
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
    _Persistent object;
    _HighlightFlags old;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(ui, property, values, flag, MyType);
    }
    else {
	/*
	**  Set the requested property accordingly
	*/

	switch (index) {
	    case _SupportedSurrogatesIndex :
		_SetMultiValuedProperty(&_SupportedSurrogates_of(ui),
		    lwk_c_domain_string, lwk_c_domain_list, &values, flag,
		    _True, _True);

		LwkUiNavRegisterApply(ui);

		break;

	    case _SupportedOperationsIndex :
		_SetMultiValuedProperty(&_SupportedOperations_of(ui),
		    lwk_c_domain_string, lwk_c_domain_list, &values, flag,
		    _True, _True);

		LwkUiNavRegisterApply(ui);

		break;

	    case _DefaultOperationIndex :
		StoreValue(ui, lwk_c_env_default_operation, lwk_c_domain_string,
		    &_DefaultOperation_of(ui), lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _DefaultHighlightIndex :
		StoreValue(ui, lwk_c_env_default_highlight, lwk_c_domain_integer,
		    &_DefaultHighlight_of(ui), lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _DefaultRelationshipIndex :
		StoreValue(ui, lwk_c_env_default_relationship,
		    lwk_c_domain_ddif_string, &_DefaultRelationship_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _DefaultRetainSourceIndex :
		StoreValue(ui, lwk_c_env_default_retain_source,
		    lwk_c_domain_boolean, &_DefaultRetainSource_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _DefaultRetainTargetIndex :
		StoreValue(ui, lwk_c_env_default_retain_target,
		    lwk_c_domain_boolean, &_DefaultRetainTarget_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _ApplHighlightIndex :
		old = _ApplHighlight_of(ui);

		_SetSingleValuedProperty(&_ApplHighlight_of(ui),
		    lwk_c_domain_integer, lwk_c_domain_list, &values, flag,
		    _True);

		SetHighlight(ui, old);

		break;

	    case _ActiveCompLinknetIndex :
		StoreCurrentObjectId(ui, lwk_c_env_active_comp_linknet,
		    lwk_c_domain_comp_linknet, &_ActiveCompLinknet_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _RecordingLinknetIndex :
		StoreCurrentObjectId(ui, lwk_c_env_recording_linknet,
		    lwk_c_domain_linknet, &_RecordingLinknet_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _ActivePathIndex :
		StoreCurrentObjectId(ui, lwk_c_env_active_path, lwk_c_domain_path,
		    &_ActivePath_of(ui), lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _ActivePathIndexIndex :
		StoreValue(ui, lwk_c_env_active_path_index, lwk_c_domain_integer,
		    &_ActivePathIndex_of(ui), lwk_c_domain_integer, values,
		    flag, _False);
		break;

	    case _ActiveCompPathIndex :
		StoreCurrentObjectId(ui, lwk_c_env_active_comp_path,
		    lwk_c_domain_comp_path, &_ActiveCompPath_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _RecordingPathIndex :
		StoreCurrentObjectId(ui, lwk_c_env_recording_path, lwk_c_domain_path,
		    &_RecordingPath_of(ui), lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _FollowedStepIndex :
		StoreCurrentObjectId(ui, lwk_c_env_followed_step, lwk_c_domain_step,
		    &_FollowedStep_of(ui), lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _NewLinkIndex :
		StoreCurrentObjectId(ui, lwk_c_env_new_link,
		    lwk_c_domain_link, &_NewLink_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _PendingSourceIndex :
		StoreCurrentObjectId(ui, lwk_c_env_pending_source,
		    lwk_c_domain_surrogate, &_PendingSource_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _PendingTargetIndex :
		StoreCurrentObjectId(ui, lwk_c_env_pending_target,
		    lwk_c_domain_surrogate, &_PendingTarget_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _FollowedLinkIndex :
		StoreCurrentObjectId(ui, lwk_c_env_followed_link,
		    lwk_c_domain_link, &_FollowedLink_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _FollowDestinationIndex :
		StoreCurrentObjectId(ui, lwk_c_env_follow_destination,
		    lwk_c_domain_surrogate, &_FollowDestination_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _RetainSourceIndex :
		StoreValue(ui, lwk_c_env_retain_source,
		    lwk_c_domain_boolean, &_RetainSource_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _RetainTargetIndex :
		StoreValue(ui, lwk_c_env_retain_target,
		    lwk_c_domain_boolean, &_RetainTarget_of(ui),
		    lwk_c_domain_list, &values, flag, _True);
		break;

	    case _UserDataIndex :
		_SetSingleValuedProperty(&_UserData_of(ui),
		    lwk_c_domain_closure, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _GetSurrogateCbIndex :
		_SetSingleValuedProperty(&_GetSurrogateCb_of(ui),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _CreateSurrogateCbIndex :
		_SetSingleValuedProperty(&_CreateSurrogateCb_of(ui),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _CloseViewCbIndex :
		_SetSingleValuedProperty(&_CloseViewCb_of(ui),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _ApplyCbIndex :
		_SetSingleValuedProperty(&_ApplyCb_of(ui),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);

		LwkUiNavRegisterApply(ui);

		break;

	    case _CompleteLinkCbIndex :
		_SetSingleValuedProperty(&_CompleteLinkCb_of(ui),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _EnvironmentChangeCbIndex :
		_SetSingleValuedProperty(&_EnvironmentChangeCb_of(ui),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _EnvironmentStateIndex :
		_SetSingleValuedProperty(&_EnvironmentState_of(ui),
		    lwk_c_domain_environment_state, lwk_c_domain_list, &values, flag,
		    _True);

		LwkUiNavEstablishCurrency(ui);

		break;

	    case _EnvironmentManagerIndex :
		_SetSingleValuedProperty(&_EnvironmentManager_of(ui),
		    lwk_c_domain_boolean, lwk_c_domain_list, &values, flag,
		    _True);

		LwkUiNavEstablishEnvManager(ui);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


static void  SetHighlight(ui, old)
_Ui ui;
 _HighlightFlags old;

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
    lwk_status status;

    /*
    ** If the Current Highlighting has changed, activate the currency change
    ** callback if the application has provided one.
    */

    if (_ApplHighlight_of(ui) != old)
	if (_EnvironmentChangeCb_of(ui) != (_Callback) _NullObject) {
	    status = (lwk_status) (*_EnvironmentChangeCb_of(ui))(ui,
		lwk_c_reason_env_change, (_Closure) 0, _UserData_of(ui),
	        lwk_c_env_appl_highlight);

	    if (status != _StatusCode(success))
		_Raise(env_change_cb_error);
	}

    return;
    }


static void  StoreCurrentObjectId(ui, index, prop_domain, property, val_domain, value, flag, value_is_list)
_Ui ui;
 _Integer index;
 _Domain prop_domain;

    _ObjectId *property;
 _Domain val_domain;
 _AnyPtr value;
 _SetFlag flag;

    _Boolean value_is_list;

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
    _ObjectId oid;
    _Boolean volatile is_oid;

    is_oid = _True;
    oid = (_ObjectId) _NullObject;

    _StartExceptionBlock

    /*
    **  First try to set the property assuming it is an Object Identifier
    */

    _SetSingleValuedProperty(&oid, lwk_c_domain_object_id, val_domain, value,
	flag, value_is_list);

    /*
    **	If we get an exception other than InvalidDomain, reraise it.
    */

    _Exceptions
	_When(inv_domain)
            is_oid = _False;
	_WhenOthers
	    _Reraise;
    _EndExceptionBlock

    /*
    **	If the value was of domain Object Identifier, make sure the ObjectId is
    **	of the proper domain for the property,  If the value was not of domain
    **	Object Identifier, try again to set the property assuming it is an
    **	object of the appropriate domain for the property.  If this succeeds,
    **	get that object's Object Identifier.
    */

    if (is_oid) {
	if (oid != (_ObjectId) _NullObject) {
	    _Integer domain;

	    _GetValue(oid, _P_ObjectDomain, lwk_c_domain_integer, &domain);

	    if (!_IsDomain(prop_domain, domain))
		_Raise(inv_domain);
	}
    }
    else {
	_Persistent persistent;

	persistent = (_Persistent) _NullObject;

	_SetSingleValuedProperty(&persistent, prop_domain, val_domain,
	    value, flag, value_is_list);

	if (persistent == (_Persistent) _NullObject)
	    oid = (_ObjectId) _NullObject;
	else
	    oid = (_ObjectId) _GetObjectId(persistent);
    }

    _DeleteValue(property, lwk_c_domain_object_id);
    _MoveValue(&oid, property, lwk_c_domain_object_id);

    /*
    **  If there is no Currency in which to store the property, return now.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return;

    /*
    **	Set the value of the property in the Currency -- it will get stored
    **	away someplace.
    */
    
    _SetValue(_EnvironmentState_of(ui), PropertyNames[index], lwk_c_domain_object_id,
	property, lwk_c_set_property);

    return;
    }


static void  RetrieveObject(ui, property, index, prop_domain, value, val_domain, list)
_Ui ui;
 _ObjectId *property;
 _Integer index;

    _Domain prop_domain;
 _AnyPtr value;
 _Domain val_domain;
 _Boolean list;

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
    _Object object;

    /*
    **  Check that the value domain is either ObjectId or is the domain of
    **	the property.
    */

    if (val_domain != lwk_c_domain_object_id)
	if (!_IsDomain(val_domain, prop_domain))
	    _Raise(inv_domain);

    /*
    **	If there is no current value for the property, ask the Currency manager
    **	(if any) for the current value.
    */

    if (*property == (_ObjectId) _NullObject
	    && _EnvironmentState_of(ui) != (_DXmEnvState) _NullObject) {

	_StartExceptionBlock

	_GetValue(_EnvironmentState_of(ui), PropertyNames[index],
	    lwk_c_domain_object_id, property);

	/*
	**  It is possible we will get a NoSuchProperty exception -- just
	**  ignore it.
	*/

	_Exceptions
	    _When(no_such_property)
		*property = (_ObjectId) _NullObject;
	    _WhenOthers
		_Reraise;
	_EndExceptionBlock
    }

    /*
    **	Either return the ObjectId or retrieve the real object.
    */

    if (val_domain == lwk_c_domain_object_id)
	    object = (_Object) *property;
    else {
	if (*property == (_ObjectId) _NullObject)
	    object = (_Object) _NullObject;
	else
	    object = (_Object) _Retrieve(*property);
    }

    /*
    **  Return the object to the caller.
    */

    if (list)
	_ListValue(&object, value, val_domain);
    else
	_CopyValue(&object, value, val_domain);

    return;
    }


static void  StoreValue(ui, index, prop_domain, property, val_domain, value, flag, value_is_list)
_Ui ui;
 _Integer index;
 _Domain prop_domain;

    _AnyPtr property;
 _Domain val_domain;
 _AnyPtr value;
 _SetFlag flag;

    _Boolean value_is_list;

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
    _SetSingleValuedProperty(property, prop_domain, val_domain, value,
	flag, value_is_list);

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return;

    /*
    **	Set the value of the property in the Currency -- it will get stored
    **	away someplace.
    */
    
    _SetValue(_EnvironmentState_of(ui), PropertyNames[index], prop_domain,
	property, lwk_c_set_property);

    return;
    }


static void  RetrieveValue(ui, property, index, prop_domain, value, val_domain, list)
_Ui ui;
 _AnyPtr *property;
 _Integer index;

    _Domain prop_domain;
 _AnyPtr value;
 _Domain val_domain;
 _Boolean list;

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
    **	If there is no current value for the property, ask the Currency manager
    **	(if any) for the current value.
    */

    if (*property == (_AnyPtr) _NullObject
	    && _EnvironmentState_of(ui) != (_DXmEnvState) _NullObject) {

	_StartExceptionBlock

	_GetValue(_EnvironmentState_of(ui), PropertyNames[index], prop_domain,
	    property);

	/*
	**  It is possible we will get a NoSuchProperty exception -- just
	**  ignore it.
	*/

	_Exceptions
	    _When(no_such_property)
		*property = (_AnyPtr) _NullObject;
	    _WhenOthers
		_Reraise;
	_EndExceptionBlock
    }

    /*
    **  Return the object to the caller.
    */

    if (list)
	_ListValue(property, value, val_domain);
    else
	_CopyValue(property, value, val_domain);

    return;
    }
