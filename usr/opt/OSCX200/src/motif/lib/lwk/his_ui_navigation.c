/*
** COPYRIGHT (c) 1988, 1989, 1990, 1991 BY
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
**	LinkWorks Services
**
**  Version: V1.0
**
**  Abstract:
**	Navigation Support for UI object methods
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Pat Avigdor
**
**  Creation Date: 13-Oct-88
**
**  Modification History:
**	BL4  dpr  24-Jan-89 -- some serious clean up after BL4 code review
**--
*/


/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_ui.h"
#include "his_ui_navigation.h"
#include "his_apply.h"
#include "his_registry.h"

/*
** Macro Definitions
*/

/*
** Type Definitions
*/

typedef struct __NextPathContext {
	    _Integer	count;
	    _Integer	index;
	    _Path	path;
	} _NextPathContext;

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void SetApplyProperties,
    (_Ui ui, _Integer value, _SetFlag flag));
_DeclareFunction(static void CurrencyChangeNotification,
    (_Ui ui, _DXmEnvState currency, _String property));
_DeclareFunction(static void CheckCompleteLinkCallback,
    (_Ui ui));
_DeclareFunction(static void UpdatePendingLink,
    (_Ui ui, lwk_environment_change code));
_DeclareFunction(static void UpdateShowLinks,
    (_Ui ui));
_DeclareFunction(static void MessageNotification,
    (_Ui ui, _DXmEnvState currency, _MessageHeader message));
_DeclareFunction(static void ReceiveApplyMessage,
    (_Ui ui, _ApplyMessage message));
_DeclareFunction(static void ReceiveApplyConfirmMessage,
    (_Ui ui, _ApplyConfirmMessage message));
_DeclareFunction(static void ReceiveCloseViewMessage,
    (_Ui ui, _CloseViewMessage message));
_DeclareFunction(static void ReceiveUpdateHistoryMessage,
    (_Ui ui, _UpdateHistoryMessage message));
_DeclareFunction(static void ReceiveShowHistoryMessage,
    (_Ui ui, _ShowHistoryMessage message));
_DeclareFunction(static void ReceiveCompleteLinkMessage,
    (_Ui ui, _CompleteLinkMessage message));
_DeclareFunction(static void ReceiveGoBackMessage,
    (_Ui ui, _GoBackMessage message));
_DeclareFunction(static void ReceiveStepForwardMessage,
    (_Ui ui, _StepForwardMessage message));
_DeclareFunction(static void ReceiveClientExitMessage,
    (_Ui ui, _ClientExitMessage message));
_DeclareFunction(static _Boolean SendApplyMessage,
    (_Ui ui, _Surrogate destination, _String operation,
	_FollowType follow_type, _Integer client_address));
_DeclareFunction(static _Boolean SendApplyConfirmMessage,
    (_Ui ui, _Surrogate destination));
_DeclareFunction(static _Boolean SendCloseViewMessage,
    (_Ui ui, _Integer client_address));
_DeclareFunction(static _Boolean SendShowHistoryMessage,
    (_Ui ui));
_DeclareFunction(static _Boolean SendUpdateHistoryMessage,
    (_Ui ui, _Surrogate origin, _Surrogate destination, _String operation,
	_FollowType follow_type));
_DeclareFunction(static _Boolean SendCompleteLinkMessage,
    (_Ui ui, _Boolean dialog));
_DeclareFunction(static _Boolean SendGoBackMessage,
    (_Ui ui));
_DeclareFunction(static _Boolean SendStepForwardMessage,
    (_Ui ui));
_DeclareFunction(static _Boolean SendClientExitMessage,
    (_Ui ui));
_DeclareFunction(static _Boolean Navigate,
    (_Ui ui, _Link link, _Surrogate origin, _Surrogate destination,
	_String operation, _FollowType follow_type, _Boolean path_navigation));
_DeclareFunction(static _Boolean SendApply,
    (_Ui ui, _Surrogate destination, _String operation,
	_FollowType follow_type));
_DeclareFunction(static void ConfirmApply,
    (_Ui ui, _Surrogate destination, _Integer client_address));
_DeclareFunction(static void UpdateHistory,
    (_Ui ui, _Surrogate origin, _Surrogate destination, _String operation,
	_FollowType follow_type, _Integer origin_address,
	_Boolean path_navigation));
_DeclareFunction(static void UpdatePath,
    (_Ui ui, _Surrogate origin, _Surrogate destination, _String operation,
	_FollowType follow_type, _Surrogate *previous_origin,
	_Surrogate *previous_destination));
_DeclareFunction(static void Highlight,
    (_Ui ui, _Closure closure));
_DeclareFunction(static void ToggleHighlight,
    (_Ui ui, _Closure closure));
_DeclareFunction(static void FollowSelected,
    (_Ui ui, _FollowType follow_type, lwk_reason reason, _Closure closure));
_DeclareFunction(static void Follow,
    (_Ui ui, _Object origins, _FollowType follow_type));
_DeclareFunction(static void ShowHistory,
    (_Ui ui, _Boolean iff_visible, _Closure closure));
_DeclareFunction(static void GoBack,
    (_Ui ui, _Closure closure));
_DeclareFunction(static void StepForward,
    (_Ui ui, _Closure closure));
_DeclareFunction(static void SetSurrogate,
    (_Ui ui, _Integer menu_action, lwk_reason reason, _Closure closure));
_DeclareFunction(static _Surrogate SelectSurrogate,
    (_Ui ui, _List surrogates));
_DeclareFunction(static void CompleteLink,
    (_Ui ui, lwk_reason reason, _Closure closure, _Boolean dialog));
_DeclareFunction(static void Annotate,
    (_Ui ui, lwk_reason reason, _Closure closure));
_DeclareFunction(static void ShowLinks,
    (_Ui ui, lwk_reason reason, _Closure closure, _Boolean update));
_DeclareFunction(static _Boolean CallbackApply,
    (_Ui ui, _Surrogate destination, _String operation,
	_FollowType follow_type));
_DeclareFunction(static void CallbackCloseView,
    (_Ui ui, lwk_reason reason, _Closure closure));
_DeclareFunction(static int CreateLinkLists,
    (_Object origins, _List *links, _List *directions,
	_List *opr_ids, _List *opr_names));
_DeclareFunction(static int FillInLinkLists,
    (_Surrogate surrogate, _List links, _List directions,
	_List opr_ids, _List opr_names));
_DeclareFunction(static void CreateStepLists,
    (_List steps, _List *origin_opr_ids, _List *origin_opr_names,
	_List *dest_opr_ids, _List *dest_opr_names));
_DeclareFunction(static _Step CreateStep, (_Surrogate origin,
    _Surrogate destination, _FollowType follow_type, _String operation));
_DeclareFunction(static void FindNextStep,
    (_Ui ui, _Surrogate *origin, _Surrogate *destination, _String *operation,
	_FollowType *follow_type));
_DeclareFunction(static void GetClientOriginAndDestination,
    (_Ui ui, _Integer client_address, _Surrogate *origin,
	_Surrogate *path, _Surrogate *destination));
_DeclareFunction(static void SaveClientOriginAndDestination,
    (_Ui ui, _Integer client_address, _Surrogate origin,
	_Surrogate path, _Surrogate destination));
_DeclareFunction(static _ClientEntry FindClientByOrigin,
    (_Ui ui, _Surrogate origin));
_DeclareFunction(static void DeleteClientEntry,
    (_Ui ui, _ClientEntry client));
_DeclareFunction(static void DeleteClientByAddress,
    (_Ui ui, _Integer client_address));
_DeclareFunction(static void FreeClientList,
    (_Ui ui));
_DeclareFunction(static _Termination FindNextPath,
    (_NextPathContext *context, _CompPath cpath, _Domain domain,
     _Path *path));
_DeclareFunction(static _Termination AnotherString,
    (_String *array, _Set set, _Domain domain, _String *string));
_DeclareFunction(static _Termination FindSurrogateSubType,
    (_String sought, _Set set, _Domain domain, _String *type));
_DeclareFunction(static _Termination FindOperation,
    (_String sought, _Set set, _Domain domain, _String *operation));
_DeclareFunction(static _String GetDefaultOperation,
    (_Surrogate surrogate));
_DeclareFunction(static _Integer GetClientAddress,
    (_Ui ui));
_DeclareFunction(static _Integer GetApplyAddress,
    (_Ui ui, _String property));
_DeclareFunction(static _Integer GetSurrogateClientAddress,
    (_Ui ui, _Surrogate surrogate));
_DeclareFunction(static _Integer CheckSumSurrogate,
    (_Surrogate surrogate));
_DeclareFunction(static _Integer CheckSumString,
    (_String string));
_DeclareFunction(static void SavePendingLink,
    (_Ui ui));
_DeclareFunction(static void DisplayWIP,
    (_Ui ui, _Surrogate surrogate));
_DeclareFunction(static void TriggerRemoveWIP,
    (_Ui ui, _Surrogate surrogate));
_DeclareFunction(static void RemoveWIP,
    (_Ui ui, _String string));

_DeclareFunction(void LwkOpDXmUiDisplayWIP,
    (_Ui ui, _String string));
_DeclareFunction(void LwkOpDXmUiRemoveWIP,
    (_Ui ui, _String string));

/*
**  Static Data Definitions
*/
    /*
    **  Table to map Currency indices into Currency Object Property Names
    */           

static _PropertyIndicesToNames;



void  LwkUiNavEstablishCurrency(ui)
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
    int code;
    _Callback routine;
    _Set_of(_String) notifications;
    _String string;
    
    /*
    **  Set the _UserData property of the Currency.
    */

    _SetValue(_EnvironmentState_of(ui), _P_UserData, lwk_c_domain_closure, &ui,
	lwk_c_set_property);

    /*
    **  Set the Notification Callback property of the Currency.
    */

    routine = (_Callback) CurrencyChangeNotification;

    _SetValue(_EnvironmentState_of(ui), _P_NotificationCb, lwk_c_domain_routine,
	&routine, lwk_c_set_property);

    /*
    **  Set the Message Callback property of the Currency.
    */

    routine = (_Callback) MessageNotification;

    _SetValue(_EnvironmentState_of(ui), _P_MessageCb, lwk_c_domain_routine,
	&routine, lwk_c_set_property);

    /*
    **	Create a Set with the names of all the properties for which currency is
    **	maintained.  We will get a notification if the value of any of
    **	these properties changes.
    */

    notifications = (_Set) _NullObject;

    _StartExceptionBlock

    notifications = (_Set) _CreateSet(_TypeSet, lwk_c_domain_string,
	lwk_c_env_change_max);

    for (code = (int) lwk_c_env_change_min;
	    code < (int) lwk_c_env_change_max; code++)
	_AddElement(notifications, lwk_c_domain_string, &PropertyNames[code],
	    _True);

    string = _P_WIPProperty;
    
    _AddElement(notifications, lwk_c_domain_string, &string, _True);

    _SetValue(_EnvironmentState_of(ui), _P_Notifications, lwk_c_domain_set,
	&notifications, lwk_c_set_property);


    /*
    **  Clean up
    */

    _Delete(&notifications);

    /*
    **  If there is an exception, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&notifications);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


void  LwkUiNavEstablishEnvManager(ui)
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
    **	We need a Currency object
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return;

    /*
    **	Set the EnvironmentManagerAddress property in the Currency if this is
    **	the Environment Manager.
    */

    if (_EnvironmentManager_of(ui)) {
	_Integer address;

	address = GetClientAddress(ui);

	_SetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	    lwk_c_domain_integer, &address, lwk_c_set_property);
    }

    return;
    }


void  LwkUiNavRegisterApply(ui)
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
    **  Make sure that there is a Currency object registered.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return;

    /*
    **	Set the Apply properties to the Client Address of this Ui.
    */

    SetApplyProperties(ui, GetClientAddress(ui), lwk_c_set_property);

    return;
    }


void  LwkUiNavUnregisterUi(ui)
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
    **  Make sure that there is a Currency object registered.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return;

    /*
    **  Delete the Apply properties from the Currency
    */

    SetApplyProperties(ui, _PendingClientAddress, lwk_c_delete_property);

    /*  
    **  If this Ui was the EnvironmentManager, delete the
    **	EnvironmentManagerAddress property from the Currency.  Otherwise, send
    **	the EnvironmentManager a message saying we are going away.
    */

    if (_EnvironmentManager_of(ui))
	_SetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	    lwk_c_domain_integer, (_Integer *) 0, lwk_c_delete_property);
    else
	SendClientExitMessage(ui);

    /*
    ** Free the Client List
    */

    FreeClientList(ui);

    return;
    }


void  LwkOpUiNavSelectMenu(ui, menu, closure)
_Ui ui;
 _Menu menu;
 _Closure closure;

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
    **  Dispatch based on which menu entry was selected
    */

    switch(menu){
	case lwk_c_dxm_menu_go_to :
	    FollowSelected(ui, lwk_c_follow_go_to, lwk_c_reason_goto, closure);
	    break;	

	case lwk_c_dxm_menu_visit :
	    FollowSelected(ui, lwk_c_follow_visit, lwk_c_reason_visit, closure);
	    break;	

	case lwk_c_dxm_menu_go_back :
	    GoBack(ui, closure);
	    break;

	case lwk_c_dxm_menu_step_forward :
	    StepForward(ui, closure);
	    break;

	case lwk_c_dxm_menu_show_history :
	    ShowHistory(ui, _False, closure);
	    break;

	case lwk_c_dxm_menu_start_link :
	    SetSurrogate(ui, lwk_c_dxm_menu_start_link,
		lwk_c_reason_start_link, closure);
	    break;

	case lwk_c_dxm_menu_comp_link :
	    SetSurrogate(ui, lwk_c_dxm_menu_comp_link,
		lwk_c_reason_complete_link, closure);
	    CompleteLink(ui, lwk_c_reason_complete_link, closure, _False);
	    break;

	case lwk_c_dxm_menu_comp_link_dialog :
	    SetSurrogate(ui, lwk_c_dxm_menu_comp_link,
		lwk_c_reason_complete_link, closure);
	    CompleteLink(ui, lwk_c_reason_complete_link, closure, _True);
	    break;

	case lwk_c_dxm_menu_annotate :
	    Annotate(ui, lwk_c_reason_annotate, closure);
	    break;

	case lwk_c_dxm_menu_show_links :
	    ShowLinks(ui, lwk_c_reason_show_links, closure, _False);
    	    break;

	case lwk_c_dxm_menu_highlight_dialog :
	    Highlight(ui, closure);
	    break;	

	case lwk_c_dxm_menu_highlight_on_off :
	    ToggleHighlight(ui, closure);
	    break;	

	default:
	    _Raise(inv_argument);
    }

    return;
    }


void  LwkOpUiNavApply(ui, operation, surrogate)
_Ui ui;
 _String operation;
 _Surrogate surrogate;

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
    _Boolean delete;

    /*
    ** If the operation is null, get the default operation for the Surrogate
    */

    if (operation != (_String) _NullObject)
	delete = _False;
    else {
	delete = _True;
	operation = GetDefaultOperation(surrogate);
    }

    /*
    **	Invoke the Apply callback if the proper conditions hold.  Otherwise,
    **	try sending an Apply Message to an existing application.  If all else
    **	fails, Ask the Apply module to do the grungy work.
    */

    if (!CallbackApply(ui, surrogate, operation, lwk_c_follow_visit))
	if (!SendApply(ui, surrogate, operation, lwk_c_follow_visit)) {
	    DisplayWIP(ui, surrogate);
	    LwkApply(surrogate, operation);
    }

    if (delete)
	_DeleteString(&operation);

    return;
    }


void  LwkOpUiNavConfirmApply(ui, destination)
_Ui ui;
 _Surrogate destination;

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
**  	{@identifier-list-or-none@}
**--
*/
    {
    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return;

    /*
    ** Confirm an Apply
    */

    ConfirmApply(ui, destination, GetClientAddress(ui));

    return;
    }


void  LwkOpUiNavConfirmLink(ui, confirmed, retain_source, retain_target)
_Ui ui;
 _Boolean confirmed;

    _Boolean retain_source;
 _Boolean retain_target;

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
    _Linknet linknet;
    _Boolean retained;
    _Surrogate surrogate;
    _Linkbase linkbase;

    if (!confirmed) {
	/*
	**  If not confirmed, delete the pending Link
	*/

	_Delete(&_PendingLink_of(ui));
    }                  
    else {
	/*
	**  Update the Recording Linknet in its Linkbase so that the new
	**  Link (and possible its Source and/or Target) will be stored.
	*/

	SavePendingLink(ui);
	
	_GetValue(_PendingLink_of(ui), _P_Linknet, lwk_c_domain_linknet,
	    &linknet);

	if (linknet != (_Linknet) _NullObject) {
	    _GetValue(linknet, _P_Linkbase, lwk_c_domain_linkbase,
		&linkbase);

	    if (linkbase != (_Linkbase) _NullObject)
		_Store(linknet, linkbase);
	}

	/*
	**  Make this Link the Pending Link (when we receive the
	**  Currency Changed notification for Pending Link, we will
	**  invoke the application Link callback).
	**
	**  Note: There are side effects caused by _SetValue -- that is why we
	**  use it here.
	*/
	
	_SetValue(ui, _P_NewLink, lwk_c_domain_link,
	    &_PendingLink_of(ui), lwk_c_set_property);

	/*
	**  Also, reset the Pending Source/Target.  The user may have requested
	**  that they not be retained, or we might have had to copy them to get
	**  them into the Recording Linknet.
	*/

	if (retain_source) {
	    retained = _True;

	    _GetValue(_PendingLink_of(ui), _P_Source,
		lwk_c_domain_surrogate, &surrogate);
	}
	else {
	    retained = _False;
	    surrogate = (_Surrogate) _NullObject;
	}

	_SetValue(ui, _P_RetainSource, lwk_c_domain_boolean, &retained,
	    lwk_c_set_property);

	_SetValue(ui, _P_PendingSource, lwk_c_domain_surrogate, &surrogate,
	    lwk_c_set_property);

	if (retain_target) {
	    retained = _True;

	    _GetValue(_PendingLink_of(ui), _P_Target,
		lwk_c_domain_surrogate, &surrogate);
	}
	else {
	    retained = _False;
	    surrogate = (_Surrogate) _NullObject;
	}

	_SetValue(ui, _P_RetainTarget, lwk_c_domain_boolean, &retained,
	    lwk_c_set_property);

	_SetValue(ui, _P_PendingTarget, lwk_c_domain_surrogate, &surrogate,
	    lwk_c_set_property);

	/*
	**  Clear the pending Link
	*/

	_PendingLink_of(ui) = (_Link) _NullObject;
    }	

    return;
    }


void  LwkOpUiNavNavigate(ui, link, origin, destination, operation, follow_type)
_Ui ui;
 _Link link;
 _Surrogate origin;

    _Surrogate destination;
 _String operation;
 _FollowType follow_type;

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
    _Boolean applied;

    /*
    ** Call the common routine
    */

    applied = Navigate(ui, link, origin, destination, operation,
	follow_type, _False);

    /*
    ** If we didn't invoke the Apply callback and the Follow Type is not Visit,
    ** try to invoke the CloseView callback.
    */

    if (!applied && follow_type != lwk_c_follow_visit)
	CallbackCloseView(ui, lwk_c_reason_goto, (_Closure) 0);

    return;
    }


_Boolean  LwkOpUiNavSurrIsHighlighted(ui, surrogate)
_Ui ui;
 _Surrogate surrogate;

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
    _Surrogate pending;
    _Boolean has_incomming;
    _Boolean has_outgoing;

    /*
    **  If Highlighting is off, return False
    */

    if ((_ApplHighlight_of(ui) & lwk_c_hl_on) == 0)
	return _False;

    /*
    **  Get the InComming and OutGoing state for the Surrogate
    */

    has_incomming = (_Boolean) _SetState(surrogate, _StateHasInComming,
	_StateGet);

    has_outgoing = (_Boolean) _SetState(surrogate, _StateHasOutGoing,
	_StateGet);

    /*
    **  Check to see if it should be highlighted based on InterLinks.
    */

    if (has_incomming && has_outgoing) {
	if ((_ApplHighlight_of(ui) &
		(lwk_c_hl_sources | lwk_c_hl_targets)) != 0)
	    return _True;
    }
    else if (has_outgoing) {
	if ((_ApplHighlight_of(ui) & lwk_c_hl_sources) != 0)
	    return _True;
    }
    else if (has_incomming) {
	if ((_ApplHighlight_of(ui) & lwk_c_hl_targets) != 0)
	    return _True;
    }
    else {
	if ((_ApplHighlight_of(ui) & lwk_c_hl_orphans) != 0)
	    return _True;
    }

    /*
    **  Check to see if it should be highlighted based on Pending
    **	Source/Target/Destination.
    */

    /*
    **  Check the Pending Destination.
    **
    **	Note: There are side effects caused by _GetValue -- that is why we use
    **	it here.
    */

    if ((_ApplHighlight_of(ui) & lwk_c_hl_destination_of_follow) != 0) {
	_GetValue(ui, _P_FollowDestination, lwk_c_domain_surrogate, &pending);

	if (surrogate == pending)
	    return _True;
    }

    /*
    **  Check the Pending Source.
    **
    **	Note: There are side effects caused by _GetValue -- that is why we use
    **	it here.
    */

    if ((_ApplHighlight_of(ui) & lwk_c_hl_pending_source) != 0) {
	_GetValue(ui, _P_PendingSource, lwk_c_domain_surrogate, &pending);

	if (surrogate == pending)
	    return _True;
    }

    /*
    **  Check the Pending Target.
    **
    **	Note: There are side effects caused by _GetValue -- that is why we use
    **	it here.
    */

    if ((_ApplHighlight_of(ui) & lwk_c_hl_pending_target) != 0) {
	_GetValue(ui, _P_PendingTarget, lwk_c_domain_surrogate, &pending);

	if (surrogate == pending)
	    return _True;
    }

    return _False;
    }


static void  SetApplyProperties(ui, value, flag)
_Ui ui;
 _Integer value;
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
    int op;
    int type;
    _Integer op_count;
    _Integer type_count;
    _String volatile *ops;
    _String volatile *types;
    _String volatile property;

    /*
    **  Make sure that there some Surrogate Type(s) have been registered.
    */

    if (_SupportedSurrogates_of(ui) == (_Set) _NullObject)
	return;

    _GetValue(_SupportedSurrogates_of(ui), _P_ElementCount,
	lwk_c_domain_integer, (_AnyPtr) &type_count);

    if (type_count <= 0)
	return;

    /*
    **  Make sure that some Supported Operation(s) have been registered.
    */

    if (_SupportedOperations_of(ui) == (_Set) _NullObject)
	return;

    _GetValue(_SupportedOperations_of(ui), _P_ElementCount,
	lwk_c_domain_integer, &op_count);

    if (op_count <= 0)
	return;                                      

    /*
    **  Make sure that an Apply callback has been registered.
    */

    if (_ApplyCb_of(ui) == (_Callback) _NullObject)
	return;

    /*
    **  Okay, now generate the appropriate properties and store them in the
    **	Currency so that other applications can communicate with us when
    **	necessary.
    */

    ops = (_String *) _NullObject;
    types = (_String *) _NullObject;
    property = (_String) _NullObject;

    _StartExceptionBlock

    /*
    **  Build an array of type names.
    */

    types = (_String *) _AllocateMem(type_count * sizeof(_String));

    for (type = 0; type < type_count; type++)
	types[type] = (_String) _NullObject;

    _Iterate(_SupportedSurrogates_of(ui), lwk_c_domain_string, (_Closure) types,
	AnotherString);

    /*
    **  Build an array of operation names.
    */

    ops = (_String *) _AllocateMem(op_count * sizeof(_String));

    for (op = 0; op < op_count; op++)
	ops[op] = (_String) _NullObject;

    _Iterate(_SupportedOperations_of(ui), lwk_c_domain_string, (_Closure) ops,
	AnotherString);

    /*
    **	Create/Delete a property for each type/operation with the given value.
    */

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_read_write);

    for (type = 0; type < type_count; type++) {
	for (op = 0; op < op_count; op++) {
	    property = _CopyString(_CurrencyPropertyPrefix);
	    property = _ConcatString(property, types[type]);
	    property = _ConcatString(property, _CurrencyPropertyDelimiter);
	    property = _ConcatString(property, ops[op]);

	    _SetValue(_EnvironmentState_of(ui), property,
		lwk_c_domain_integer, &value, flag);

	    _DeleteString(&property);
	}
    }

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_commit);

    /*
    **	If we get an exception clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Transact(_EnvironmentState_of(ui), lwk_c_transact_rollback);

	    if (types != (_String *) _NullObject)
		_FreeMem(ops);

	    if (ops != (_String *) _NullObject)
		_FreeMem(types);

	    _DeleteString(&property);

	    _Reraise;
    _EndExceptionBlock

    /*
    **  Free the String arrays
    */

    _FreeMem(ops);
    _FreeMem(types);

    return;
    }
            

static void  CurrencyChangeNotification(ui, currency, property)
_Ui ui;
 _DXmEnvState currency;

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
    int code;
    _ObjectId oid;
    _Boolean found;
    _String string;
    _Integer integer;
    _Boolean boolean;
    _DDIFString ddifstring;
    _ObjectId old_oid;
    _String old_string;
    _DDIFString old_ddifstring;

    /*
    **  Find the currency index for the property.
    */

    found = _False;

    for (code = (int) lwk_c_env_change_min;
	    code < (int) lwk_c_env_change_max; code++)
	if (strcmp(property, PropertyNames[code]) == 0) {
	    found = _True;
	    break;
	}
    
    /*
    **  If it is an unknown property, return now.
    */

    if (!found) {
	if (strcmp(property, _P_WIPProperty) == 0) {
	    _GetValue(currency, property, lwk_c_domain_string, &string);
	    RemoveWIP(ui, string);
	    _DeleteString(&string);
	}
	return;
    }

    /*
    **	Get the value of the property from the Currency manager
    */

    _StartExceptionBlock

    switch ((lwk_environment_change) code) {
	case lwk_c_env_active_comp_linknet :
	case lwk_c_env_recording_linknet :
	case lwk_c_env_active_path :
	case lwk_c_env_active_comp_path :
	case lwk_c_env_recording_path :
	case lwk_c_env_followed_step :
	case lwk_c_env_new_link :
	case lwk_c_env_pending_source :
	case lwk_c_env_pending_target :
	case lwk_c_env_followed_link :
	case lwk_c_env_follow_destination :
	    _GetValue(currency, property, lwk_c_domain_object_id, &oid);
	    break;

	case lwk_c_env_default_operation :
	    _GetValue(currency, property, lwk_c_domain_string, &string);
	    break;

	case lwk_c_env_default_relationship :
	    _GetValue(currency, property, lwk_c_domain_ddif_string,
		&ddifstring);
	    break;

	case lwk_c_env_active_path_index :
	    _GetValue(currency, property, lwk_c_domain_integer, &integer);
	    break;

	case lwk_c_env_default_highlight :
	    _GetValue(currency, property, lwk_c_domain_integer, &integer);
	    break;

	case lwk_c_env_retain_source :
	case lwk_c_env_retain_target :
	case lwk_c_env_default_retain_source :
	case lwk_c_env_default_retain_target :
	    _GetValue(currency, property, lwk_c_domain_boolean, &boolean);
	    break;
	    	    
	default :
	    break;
    }

    /*
    **	It is possible we will get a NoSuchProperty exception -- that can
    **	happen if the property was deleted.  In this case, clear the currency
    **	object.
    */

    _Exceptions
	_When(no_such_property)
	    oid = (_ObjectId) _NullObject;
	    integer = 0;
	    boolean = _False;
	    string = (_String) _NullObject;
	    ddifstring = (_DDIFString) _NullObject;
	_WhenOthers
	    _Reraise;
    _EndExceptionBlock

    /*
    **	Reset the appropriate Active Object in the Ui.
    */

    old_oid = (_ObjectId) _NullObject;
    old_string = (_String) _NullObject;
    old_ddifstring = (_DDIFString) _NullObject;

    switch ((lwk_environment_change) code) {
	case lwk_c_env_active_comp_linknet :
	    old_oid = _ActiveCompLinknet_of(ui);
	    _ActiveCompLinknet_of(ui) = oid;
	    break;

	case lwk_c_env_recording_linknet :
	    old_oid = _RecordingLinknet_of(ui);
	    _RecordingLinknet_of(ui) = oid;
	    break;

	case lwk_c_env_active_path :
	    old_oid = _ActivePath_of(ui);
	    _ActivePath_of(ui) = oid;
	    break;

	case lwk_c_env_active_path_index :
	    _ActivePathIndex_of(ui) = integer;
	    break;

	case lwk_c_env_active_comp_path :
	    old_oid = _ActiveCompPath_of(ui);
	    _ActiveCompPath_of(ui) = oid;
	    break;

	case lwk_c_env_recording_path :
	    old_oid = _RecordingPath_of(ui);
	    _RecordingPath_of(ui) = oid;
	    break;

	case lwk_c_env_followed_step :
	    old_oid = _FollowedStep_of(ui);
	    _FollowedStep_of(ui) = oid;
	    break;

	case lwk_c_env_new_link :
	    old_oid = _NewLink_of(ui);
	    _NewLink_of(ui) = oid;
	    break;

	case lwk_c_env_pending_source :
	    old_oid = _PendingSource_of(ui);
	    _PendingSource_of(ui) = oid;
	    break;

	case lwk_c_env_pending_target :
	    old_oid = _PendingTarget_of(ui);
	    _PendingTarget_of(ui) = oid;
	    break;

	case lwk_c_env_followed_link :
	    old_oid = _FollowedLink_of(ui);
	    _FollowedLink_of(ui) = oid;
	    break;

	case lwk_c_env_follow_destination :
	    old_oid = _FollowDestination_of(ui);
	    _FollowDestination_of(ui) = oid;
	    break;

	case lwk_c_env_default_operation :
	    old_string = _DefaultOperation_of(ui);
	    _DefaultOperation_of(ui) = string;
	    break;

	case lwk_c_env_default_relationship :
	    old_ddifstring = _DefaultRelationship_of(ui);
	    _DefaultRelationship_of(ui) = ddifstring;
	    break;

	case lwk_c_env_default_highlight :
	    _DefaultHighlight_of(ui) = integer;
	    break;

	case lwk_c_env_default_retain_source :
	    _DefaultRetainSource_of(ui) = boolean;
	    break;

	case lwk_c_env_default_retain_target :
	    _DefaultRetainTarget_of(ui) = boolean;
	    break;

	case lwk_c_env_retain_source :
	    _RetainSource_of(ui) = boolean;
	    break;

	case lwk_c_env_retain_target :
	    _RetainTarget_of(ui) = boolean;
	    break;

	default :
	    break;
    }

    /*
    **  Delete any old Objects.
    */

    _Delete(&old_oid);
    _DeleteString(&old_string);
    _DeleteDDIFString(&old_ddifstring);

    /*
    ** Perform any pre-CurrencyChange callback processing
    */

    switch ((lwk_environment_change) code) {
	case lwk_c_env_new_link :
            /*
            ** If the Pending Link changes, we may need to invoke the
	    ** Link callback.
            */
	    
	    CheckCompleteLinkCallback(ui);

	    break;

	case lwk_c_env_pending_source :
	    /*
	    **	If the Pending Source changes, we need to decrement the Pending
	    **	Source Owner indicator (when it reaches zero, we are no longer
	    **	the owner!).  We may also need to update any pending Link.
	    */

	    if (_PendingSourceOwner_of(ui) > 0)
		_PendingSourceOwner_of(ui)--;

	    UpdatePendingLink(ui, lwk_c_env_pending_source);

	    break;

	case lwk_c_env_pending_target :
	    /*
	    **	If the Pending Target changes, we need to decrement the Pending
	    **	Target Owner indicator (when it reaches zero, we are no longer
	    **	the owner!).  We may also need to update any pending Link.
	    */

	    if (_PendingTargetOwner_of(ui) > 0)
		_PendingTargetOwner_of(ui)--;

	    UpdatePendingLink(ui, lwk_c_env_pending_target);

	    break;

	case lwk_c_env_retain_source :
	    /*
	    **	If the Pending Source is being retained, we need to increment
	    **	the Pending Source indicator.
	    */

	    if (_RetainSource_of(ui) && _PendingSourceOwner_of(ui) > 0)
		_PendingSourceOwner_of(ui)++;

	    break;

	case lwk_c_env_retain_target :
	    /*
	    **	If the Pending Target is being retained, we need to increment
	    **	the Pending Target Owner indicator.
	    */

	    if (_RetainTarget_of(ui) && _PendingTargetOwner_of(ui) > 0)
		_PendingTargetOwner_of(ui)++;

	    break;

	default :
	    break;
    }

    /*
    **  Notify the application via the EnvironmentChange callback, if provided.
    */

    if (_EnvironmentChangeCb_of(ui) != (_Callback) _NullObject)
	(_EnvironmentChangeCb_of(ui))(ui, lwk_c_reason_env_change,
	    (_Closure) 0, _UserData_of(ui), (_Integer) code);

    /*
    ** Perform any post-CurrencyChange callback processing
    */

    switch ((lwk_environment_change) code) {
	case lwk_c_env_active_comp_linknet :
            /*
            ** If the Active Linknet changes, we may need to update
	    ** any visible Show Links Dialog Box.
            */
	    
	    UpdateShowLinks(ui);

	    break;

	default :
	    break;
    }

    return;
    }


static void  CheckCompleteLinkCallback(ui)
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

    if (_NewLink_of(ui) == _NullObject)
	return;

    /*
    **	If the Pending Link changed, check to see if this application
    **	owned either the Pending Source, or the Pending Target, and if so, call
    **	any Complete Link Callback routine.
    */

    if (_PendingSourceOwner_of(ui) > 0 || _PendingTargetOwner_of(ui) > 0) {
	lwk_status status;
	_Surrogate surrogate;
	_Link link;

	/*
	**  Retrieve the Link and make sure it gets "hooked up" to the
	**  Pending Source and/or Pending Target in this application.
	*/

	link = (_Link) _Retrieve(_NewLink_of(ui));

	if (link != (_Link) _NullObject) {
	    if (_PendingSourceOwner_of(ui) > 0)
		_GetValue(link, _P_Source, lwk_c_domain_surrogate,
		    &surrogate);

	    if (_PendingTargetOwner_of(ui) > 0)
		_GetValue(link, _P_Target, lwk_c_domain_surrogate,
		    &surrogate);
	}

	/*
	**  Callback the application if it asked us to do so.
	*/

	if (_CompleteLinkCb_of(ui) != (_Callback) _NullObject) {
	    status = (lwk_status) (*_CompleteLinkCb_of(ui))(ui,
		lwk_c_reason_complete_link, (_Closure) 0, _UserData_of(ui),
		link);

	    if (status != _StatusCode(success))
		_Raise(complete_link_cb_error);
	}
    }

    return;
    }


static void  UpdatePendingLink(ui, code)
_Ui ui;
 lwk_environment_change code;

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
    _Surrogate surrogate;
    _Persistent container;
    _Linknet recording_linknet;
    _DDIFString volatile ddifstring;
    _Set_of(_DDIFString) volatile keywords;

    /*
    **  If there is no Link pending, return now.
    */

    if (_PendingLink_of(ui) == (_Link) _NullObject)
	return;

    /*
    ** Initialization.
    */

    keywords = (_Set) _NullObject;
    ddifstring = (_DDIFString) _NullObject;

    _StartExceptionBlock

    /*
    **	If the Pending Source changed, we need to make some changes in the
    **	Source-related properties of the pending Link.
    **
    **	Note: There are side effects caused by _GetValue -- that is why we use
    **	it here.
    */

    if (code == lwk_c_env_pending_source) {
	_GetValue(ui, _P_PendingSource, lwk_c_domain_surrogate, &surrogate);

	/*
	**  If the Pending Source was cleared, leave the pending Link
	**  alone.
	*/

	if (surrogate != (_Surrogate) _NullObject) {

	    /*
	    **  If the Pending Source changed, reset the Source-related
	    **	properties of the pending Link (i.e., Source
	    **	Description/Keywords).
	    */

	    _GetValue(surrogate, _P_Description, lwk_c_domain_ddif_string,
		(_AnyPtr) &ddifstring);

	    _SetValue(_PendingLink_of(ui), _P_SourceDescription,
		lwk_c_domain_ddif_string, (_AnyPtr) &ddifstring, lwk_c_set_property);

	    _DeleteDDIFString(&ddifstring);

	    _GetValue(surrogate, _P_Keywords, lwk_c_domain_set,
		(_AnyPtr) &keywords);

	    _SetValue(_PendingLink_of(ui), _P_SourceKeywords,
		lwk_c_domain_set, (_AnyPtr) &keywords, lwk_c_set_property);

	    _Delete(&keywords);
	}
    }

    /*
    **	If the Pending Target changed, we need to make some changes in the
    **	Target-related properties of the pending Link.
    **
    */

    if (code == lwk_c_env_pending_target) {
	_GetValue(ui, _P_PendingTarget, lwk_c_domain_surrogate, &surrogate);

	/*
	**  If the Pending Target was cleared, leave the pending Link
	**  alone.
	*/

	if (surrogate != (_Surrogate) _NullObject) {

	    /*
	    **  If the Pending Target changed, reset the Target-related
	    **	properties of the pending Link (i.e., Target
	    **	Description/Keywords).
	    */

	    _GetValue(surrogate, _P_Description, lwk_c_domain_ddif_string,
		(_AnyPtr) &ddifstring);

	    _SetValue(_PendingLink_of(ui), _P_TargetDescription,
		lwk_c_domain_ddif_string, (_AnyPtr) &ddifstring, lwk_c_set_property);

	    _DeleteDDIFString(&ddifstring);

	    _GetValue(surrogate, _P_Keywords, lwk_c_domain_set,
		(_AnyPtr) &keywords);

	    _SetValue(_PendingLink_of(ui), _P_TargetKeywords,
		lwk_c_domain_set, (_AnyPtr) &keywords, lwk_c_set_property);

	    _Delete(&keywords);
	}
    }

    /*
    **  If the Recording Linknet changed, we need to move the pending Link
    **	to the new Linknet, and to make eventually some changes in the
    **	Source-related and Target-related properties of the pending Link.
    */

    if (code == lwk_c_env_recording_linknet) {

	/*
	**  Find the Recording Linknet -- if there isn't one, return now.
	**
	**	Note: There are side effects caused by _GetValue -- that is why
	**	we use it here.
	*/

	_GetValue(ui, _P_RecordingLinknet, lwk_c_domain_linknet,
	    &recording_linknet);

	if (recording_linknet == (_Linknet) _NullObject)
	    _Raise(normal_return);

	_SetValue(_PendingLink_of(ui), _P_Linknet, lwk_c_domain_linknet,
	    &recording_linknet, lwk_c_set_property);

        /*
	**  Get the pending source.
	*/
	
	_GetValue(ui, _P_PendingSource, lwk_c_domain_surrogate, &surrogate);

	/*
	**  If the Pending Source was cleared, leave the pending Link
	**  alone.
	*/

	if (surrogate != (_Surrogate) _NullObject) {
	    /*
	    **	If the Pending Source is no longer in the Recording Linknet,
	    **	make a copy of it (it will be put in the Recording Linknet
	    **	along with the pending Link).
	    */

	    _GetValue(surrogate, _P_Container, lwk_c_domain_persistent,
     		&container);

	    if (recording_linknet != container)
		surrogate = (_Surrogate) _Copy(surrogate, _False);

	    /*
	    **  Set the Source property of the pending Link.
	    */

	    _SetValue(_PendingLink_of(ui), _P_Source,
		lwk_c_domain_surrogate, &surrogate, lwk_c_set_property);

	}

        /*
	**  Get the Pending Target.
	*/
	
	_GetValue(ui, _P_PendingTarget, lwk_c_domain_surrogate, &surrogate);

	/*
	**  If the Pending Target was cleared, leave the pending Link
	**  alone.
	*/

	if (surrogate != (_Surrogate) _NullObject) {
	    /*
	    **	If the Pending Target is no longer in the Recording Linknet,
	    **	make a copy of it (it will be put in the Recording Linknet
	    **	along with the pending Link).
	    */

	    _GetValue(surrogate, _P_Container, lwk_c_domain_persistent,
		&container);

	    if (recording_linknet != container)
		surrogate = (_Surrogate) _Copy(surrogate, _False);

	    /*
	    ** Set the Target property of the pending Link.
	    */

	    _SetValue(_PendingLink_of(ui), _P_Target,
		lwk_c_domain_surrogate, &surrogate, lwk_c_set_property);

	}
    }
    
    /*
    ** Update the Complete Link Dialog Box if it is visible.
    */

    _CompleteLink(ui, _PendingLink_of(ui), _True, (_Closure) 0);

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_When(normal_return)
	_WhenOthers
	    _Delete(&keywords);
	    _DeleteDDIFString(&ddifstring);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  UpdateShowLinks(ui)
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
    _StartExceptionBlock

    /*
    ** Simply ShowLinks if there is a Show Links box displayed
    */

    ShowLinks(ui, lwk_c_reason_show_links, (_Closure) 0, _True);

    /*
    ** If any exceptions other than NotLinked are raised, display error message.
    ** We just ignore Not Linked.
    */

    _Exceptions
	_When(not_linked)

	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_links_error);
	    status[1] = _Others;

	    _DisplayMessage(ui, status, 2);
	    
    _EndExceptionBlock

    return;
    }


static void  MessageNotification(ui, currency, message)
_Ui ui;
 _DXmEnvState currency;

    _MessageHeader message;

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
    **  Dispatch based on the message type
    */

    switch (message->type) {
	case _MessageApply :
	    ReceiveApplyMessage(ui, (_ApplyMessage) message);
	    break;

	case _MessageApplyConfirmation :
	    ReceiveApplyConfirmMessage(ui, (_ApplyConfirmMessage) message);
	    break;

	case _MessageCloseView :
	    ReceiveCloseViewMessage(ui, (_CloseViewMessage) message);
	    break;

	case _MessageShowHistory :
	    ReceiveShowHistoryMessage(ui, (_ShowHistoryMessage) message);
	    break;

	case _MessageCompleteLink :
	    ReceiveCompleteLinkMessage(ui, (_CompleteLinkMessage) message);
	    break;

	case _MessageUpdateHistory :
	    ReceiveUpdateHistoryMessage(ui, (_UpdateHistoryMessage) message);
	    break;

	case _MessageGoBack :
	    ReceiveGoBackMessage(ui, (_GoBackMessage) message);
	    break;

	case _MessageStepForward :
	    ReceiveStepForwardMessage(ui, (_StepForwardMessage) message);
	    break;

	case _MessageClientExit :
	    ReceiveClientExitMessage(ui, (_ClientExitMessage) message);
	    break;
    }

    return;
    }         


static void  ReceiveApplyMessage(ui, message)
_Ui ui;
 _ApplyMessage message;

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
    _Boolean applied;
    _Integer checksum;
    _Surrogate volatile destination;
    _String volatile operation;
    _String volatile subtype;
    _Integer volatile wip_posted = 0;
    
    /*
    ** Initialize
    */

    operation = (_String) _NullObject;
    subtype = (_String) _NullObject;

    _StartExceptionBlock

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_read_write);

    /*
    **  Get and clear the Surrogate which is the Destination of the Apply.
    */

    _GetValue(_EnvironmentState_of(ui), _P_ApplyDestination,
	lwk_c_domain_surrogate, (_AnyPtr) &destination);

    _SetValue(_EnvironmentState_of(ui), _P_ApplyDestination,
	lwk_c_domain_surrogate, (_AnyPtr) &destination, lwk_c_delete_property);

    /*
    **  Get and clear the Operation name to Apply to the Surrogate.
    */

    _GetValue(_EnvironmentState_of(ui), _P_ApplyOperation, lwk_c_domain_string,
	(_AnyPtr) &operation);
            
    _SetValue(_EnvironmentState_of(ui), _P_ApplyOperation, lwk_c_domain_string,
	(_AnyPtr) &operation, lwk_c_delete_property);

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_commit);

    /*
    ** Verify the argument checksum
    */

    checksum = CheckSumSurrogate(destination);
    checksum ^= CheckSumString(operation);

    if (message->header.checksum != checksum)
	_Raise(unexpected_error);

    /*
    **  Invoke the Apply callback
    */

    applied = CallbackApply(ui, destination, operation, message->follow_type);

    /*
    **	If the Apply callback failed, and we are not the Environment Manager,
    **	notify user of the error.  If we are the Environment Manager, ask the
    **	Apply module to the grungy work.
    */

    if (!applied) {
	if (_EnvironmentManager_of(ui)) {
	    wip_posted = 1;
	    _GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
		(_AnyPtr) &subtype);
	    DisplayWIP(ui, destination);
	    LwkApply(destination, operation);
	    _DeleteString(&subtype);
	} else
	    _Raise(failure);
    }

    /*
    **  Delete the Strings
    */

    _DeleteString(&operation);
    
    /*
    **	If we get an exception clean up, display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    _Transact(_EnvironmentState_of(ui), lwk_c_transact_rollback);
	    if (operation != (_String) _NullObject)
		_DeleteString(&operation);

	    status[0] = _StatusCode(apply_error);
	    status[1] = _Others;

	    if (wip_posted) {
		RemoveWIP(ui, subtype);
		if (subtype != (_String) _NullObject)
		    _DeleteString(&subtype);
	    };

	    _DisplayMessage(ui, status, 2);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveApplyConfirmMessage(ui, message)
_Ui ui;
 _ApplyConfirmMessage message;

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
    _Integer checksum;                                            
    _Surrogate destination;

    /*
    ** Initialize
    */

    _StartExceptionBlock

    /*
    **  Get and clear the Surrogate which is being confirmed.
    */

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_read_write);

    _GetValue(_EnvironmentState_of(ui), _P_ApplyConfirmation,
	lwk_c_domain_surrogate, &destination);

    _SetValue(_EnvironmentState_of(ui), _P_ApplyConfirmation,
	lwk_c_domain_surrogate, &destination, lwk_c_delete_property);

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_commit);

    /*
    ** Verify the argument checksum
    */

    checksum = CheckSumSurrogate(destination);

    if (message->header.checksum != checksum)
	_Raise(unexpected_error);

    /*
    **  Confirm the Apply
    */

    ConfirmApply(ui, destination, message->client_address);

    /*
    **	If we get an exception clean up, display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    _Transact(_EnvironmentState_of(ui), lwk_c_transact_rollback);

	    status[0] = _StatusCode(apply_error);
	    status[1] = _Others;

	    _DisplayMessage(ui, status, 2);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveCloseViewMessage(ui, message)
_Ui ui;
 _CloseViewMessage message;

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
    **  Invoke the Close View callback
    */

    CallbackCloseView(ui, lwk_c_reason_goto, (_Closure) 0);

    /*
    **	If we get an exception display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status;

	    status = _Others;
	    _DisplayMessage(ui, &status, 1);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveUpdateHistoryMessage(ui, message)
_Ui ui;
 _UpdateHistoryMessage message;

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
    _Integer checksum;
    _Surrogate origin;
    _Surrogate destination;
    _String volatile operation;

    /*
    ** Initialize
    */

    operation = (_String) _NullObject;

    _StartExceptionBlock

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_read_write);

    /*
    **	Get and clear the Surrogates which were the Origin and Destination of
    **	the Follow and the Operation name.
    */

    _GetValue(_EnvironmentState_of(ui), _P_HistoryOrigin,
	lwk_c_domain_surrogate, &origin);

    _SetValue(_EnvironmentState_of(ui), _P_HistoryOrigin,
	lwk_c_domain_surrogate, &origin, lwk_c_delete_property);

    _GetValue(_EnvironmentState_of(ui), _P_HistoryDestination,
	lwk_c_domain_surrogate, &destination);

    _SetValue(_EnvironmentState_of(ui), _P_HistoryDestination,
	lwk_c_domain_surrogate, &destination, lwk_c_delete_property);

    _GetValue(_EnvironmentState_of(ui), _P_HistoryOperation,
	lwk_c_domain_string, (_AnyPtr) &operation);

    _SetValue(_EnvironmentState_of(ui), _P_HistoryOperation,
	lwk_c_domain_string, (_AnyPtr) &operation, lwk_c_delete_property);

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_commit);

    /*
    ** Verify the argument checksum
    */

    checksum = CheckSumSurrogate(origin);
    checksum ^= CheckSumSurrogate(destination);
    checksum ^= CheckSumString(operation);

    if (message->header.checksum != checksum)
	_Raise(unexpected_error);

    /*
    ** Update the History
    */

    UpdateHistory(ui, origin, destination, operation, message->follow_type,
	message->origin_address, _False);

    /*
    **  Delete the Operation String
    */

    _DeleteString(&operation);

    /*
    **	If we get an exception clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Transact(_EnvironmentState_of(ui), lwk_c_transact_rollback);
	    _DeleteString(&operation);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveShowHistoryMessage(ui, message)
_Ui ui;
 _ShowHistoryMessage message;

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
    ** Invoke the function
    */

    ShowHistory(ui, _False, (_Closure) 0);

    /*
    **	If we get an exception display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_history_error);
	    status[1] = _Others;

	    _DisplayMessage(ui, status, 2);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveCompleteLinkMessage(ui, message)
_Ui ui;
 _CompleteLinkMessage message;

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
    ** Invoke the function
    */

    CompleteLink(ui, lwk_c_reason_complete_link, (_Closure) 0, message->dialog);

    /*
    **	If we get an exception display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(complete_link_error);
	    status[1] = _Others;

	    _DisplayMessage(ui, status, 2);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveGoBackMessage(ui, message)
_Ui ui;
 _GoBackMessage message;

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
    ** Invoke the function
    */

    GoBack(ui, (_Closure) 0);

    /*
    **	If we get an exception display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(go_back_error);
	    status[1] = _Others;

	    _DisplayMessage(ui, status, 2);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveStepForwardMessage(ui, message)
_Ui ui;
 _StepForwardMessage message;

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
    ** Invoke the function
    */

    StepForward(ui, (_Closure) 0);

    /*
    **	If we get an exception display a message, then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(step_forward_error);
	    status[1] = _Others;

	    _DisplayMessage(ui, status, 2);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ReceiveClientExitMessage(ui, message)
_Ui ui;
 _ClientExitMessage message;

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
    ** Forget about this Client
    */

    DeleteClientByAddress(ui, message->client_address);

    return;
    }


static _Boolean  SendApplyMessage(ui, destination, operation, follow_type, client_address)
_Ui ui;
 _Surrogate destination;

    _String operation;
 _FollowType follow_type;
 _Integer client_address;

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
    _Boolean result;
    _Integer checksum;
    _ApplyMessageInstance message;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **	Set the value of the ApplyDestination and ApplyOperation properties in
    **	the Currency.  These will be retrieved and cleared by the receiver of
    **	the Apply message.
    */

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_read_write);

    _SetValue(_EnvironmentState_of(ui), _P_ApplyDestination,
	lwk_c_domain_surrogate, &destination, lwk_c_set_property);

    _SetValue(_EnvironmentState_of(ui), _P_ApplyOperation, lwk_c_domain_string,
	&operation, lwk_c_set_property);

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_commit);

    /*
    ** Calculate a checksum for the arguments
    */

    checksum = CheckSumSurrogate(destination);
    checksum ^= CheckSumString(operation);

    /*
    **  Build the message and send it.
    */

    message.header.type = _MessageApply;
    message.header.checksum = checksum;
    message.follow_type = follow_type;

    _SendMessage(_EnvironmentState_of(ui), client_address,
	sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
	    _Transact(_EnvironmentState_of(ui), lwk_c_transact_rollback);
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendApplyConfirmMessage(ui, destination)
_Ui ui;
 _Surrogate destination;

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
    _Boolean result;
    _Integer address;
    _Integer checksum;
    _ApplyConfirmMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **	Set the value of the ApplyConfirmation property in the Currency.  This
    **	will be retrieved and cleared by the receiver of the Apply Confirmation
    **	message.
    */

    _SetValue(_EnvironmentState_of(ui), _P_ApplyConfirmation,
	lwk_c_domain_surrogate, &destination, lwk_c_set_property);

    /*
    ** Calculate a checksum for the arguments
    */

    checksum = CheckSumSurrogate(destination);

    /*
    **  Build the message and send it.
    */

    message.header.type = _MessageApplyConfirmation;
    message.header.checksum = checksum;
    message.client_address = GetClientAddress(ui);

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendCloseViewMessage(ui, client_address)
_Ui ui;
 _Integer client_address;

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
    _Boolean result;
    _CloseViewMessageInstance message;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  Build the message and send it.
    */

    message.header.type = _MessageCloseView;
    message.header.checksum = 0;

    _SendMessage(_EnvironmentState_of(ui), client_address, sizeof(message),
	&message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendShowHistoryMessage(ui)
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
    _Boolean result;
    _Integer address;
    _ShowHistoryMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **  Build the message and try to send it
    */

    message.header.type = _MessageShowHistory;
    message.header.checksum = 0;

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendUpdateHistoryMessage(ui, origin, destination, operation, follow_type)
_Ui ui;
 _Surrogate origin;

    _Surrogate destination;
 _String operation;
 _FollowType follow_type;

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
    _Boolean result;
    _Integer address;
    _Integer checksum;
    _UpdateHistoryMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **	Set the value of the HistoryOrigin, HistoryDestination and
    **	HistoryOperation properties in the Currency.  These will be retrieved
    **	and cleared by the Environment Manager.
    */

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_read_write);

    _SetValue(_EnvironmentState_of(ui), _P_HistoryOrigin,
	lwk_c_domain_surrogate, &origin, lwk_c_set_property);

    _SetValue(_EnvironmentState_of(ui), _P_HistoryDestination,
	lwk_c_domain_surrogate, &destination, lwk_c_set_property);

    _SetValue(_EnvironmentState_of(ui), _P_HistoryOperation,
	lwk_c_domain_string, &operation, lwk_c_set_property);

    _Transact(_EnvironmentState_of(ui), lwk_c_transact_commit);

    /*
    ** Calculate a checksum for the arguments
    */

    checksum = CheckSumSurrogate(origin);
    checksum ^= CheckSumSurrogate(destination);
    checksum ^= CheckSumString(operation);

    /*
    **  Build the message and try to send it
    */

    message.header.type = _MessageUpdateHistory;
    message.header.checksum = checksum;
    message.follow_type = follow_type;
    message.origin_address = GetClientAddress(ui);

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
	_Transact(_EnvironmentState_of(ui), lwk_c_transact_rollback);
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendCompleteLinkMessage(ui, dialog)
_Ui ui;
 _Boolean dialog;

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
    _Boolean result;
    _Integer address;
    _CompleteLinkMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **  Build the message and try to send it
    */

    message.header.type = _MessageCompleteLink;
    message.header.checksum = 0;
    message.dialog = dialog;

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendGoBackMessage(ui)
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
    _Boolean result;
    _Integer address;
    _GoBackMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **  Build the message and try to send it
    */

    message.header.type = _MessageGoBack;
    message.header.checksum = 0;

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendStepForwardMessage(ui)
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
    _Boolean result;
    _Integer address;
    _StepForwardMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **  Build the message and try to send it
    */

    message.header.type = _MessageStepForward;
    message.header.checksum = 0;

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  SendClientExitMessage(ui)
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
    _Boolean result;
    _Integer address;
    _ClientExitMessageInstance message;

    /*
    **  If we are the Environment Manager, then we better do it ourselves!
    */

    if (_EnvironmentManager_of(ui))
	return _False;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _False;

    /*
    **  Initialize
    */

    result = _False;

    _StartExceptionBlock

    /*
    **  See if there is a registered Environment Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
	lwk_c_domain_integer, &address);

    /*
    **  Build the message and try to send it
    */

    message.header.type = _MessageClientExit;
    message.header.checksum = 0;
    message.client_address = GetClientAddress(ui);

    _SendMessage(_EnvironmentState_of(ui), address, sizeof(message), &message);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return result;
    }


static _Boolean  Navigate(ui, link, origin, destination, operation, follow_type, path_navigation)
_Ui ui;
 _Link link;
 _Surrogate origin;

    _Surrogate destination;
 _String operation;
 _FollowType follow_type;

    _Boolean path_navigation;

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
    _Boolean delete;
    _Boolean applied;
    _Integer origin_address;

    /*
    ** If the operation is null, get the default operation for the destination
    ** Surrogate Subtype.
    */

    if (operation != (_String) _NullObject)
	delete = _False;
    else {
	delete = _True;
	operation = GetDefaultOperation(destination);
    }

    /*
    **	Update the History.  We do this earlier than we might (the apply
    **	callback or the HyperApplication hot/cold start may fail!) because we
    **	need the Client Entries created by UpdateHistory to be present when the
    **	ApplyConfirm happens.
    */

    if (path_navigation)
	origin_address = _StepDestination_of(ui);
    else
	origin_address = GetClientAddress(ui);

    UpdateHistory(ui, origin, destination, operation, follow_type,
	origin_address, path_navigation);

    /*
    ** If a Link was followed, set the Followed Link
    **
    **  Note: There are side effects caused by _SetValue -- that is why we
    **  use it here.
    */

    if (link != (_Link) _NullObject)
	_SetValue(ui, _P_FollowedLink, lwk_c_domain_link, &link,
	    lwk_c_set_property);

    /*
    **	Invoke the Apply callback if the proper conditions hold.  Otherwise,
    **	try sending an Apply Message to an existing application.  If all else
    **	fails, invoke the Apply operation on the Destination Surrogate.
    */

    applied = CallbackApply(ui, destination, operation, follow_type);

    if (!applied) {
	/*
	**  Try sending an Apply message to the appropriate, running
	**  application.  If there isn't one, ask the Apply module to do the
	**  grungy work.
	**
	**  Note: If we are sending a hot-start message to another application,
	**  we need to change the FollowType from GoTo into a Visit.
	*/

	if (!SendApply(ui, destination, operation, lwk_c_follow_visit)) {
	    DisplayWIP(ui, destination);
	    LwkApply(destination, operation);
	}
    }

    if (delete)
	_DeleteString(&operation);

    return applied;
    }


static _Boolean  SendApply(ui, destination, operation, follow_type)
_Ui ui;
 _Surrogate destination;

    _String operation;
 _FollowType follow_type;

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
    _Boolean result;
    _String volatile type;
    _String volatile subtype;
    _String volatile property;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
    	return _False;

    /*
    **  Initialize
    */

    result = _False;
    type = (_String) _NullObject;
    subtype = (_String) _NullObject;
    property = (_String) _NullObject;

    _StartExceptionBlock

    /*
    **  Generate the name of the property which should have been registered by
    **	an application which can perform the given operation on a Surrogate of
    **	the given type.
    */

    _GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
	(_AnyPtr) &subtype);

    type = LwkRegOperationType(subtype, operation);

    property = _CopyString(_CurrencyPropertyPrefix);
    property = _ConcatString(property, type);
    property = _ConcatString(property, _CurrencyPropertyDelimiter);
    property = _ConcatString(property, operation);

    /*
    **	Send the message to the appropriate Client or the EnvironmentManager.
    */

    SendApplyMessage(ui, destination, operation, follow_type,
	GetApplyAddress(ui, property));

    /*
    **  Delete the Strings which we generated
    */

    _DeleteString(&type);
    _DeleteString(&subtype);
    _DeleteString(&property);

    result = _True;

    /*
    **  If there is an exception, clean up, but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&type);
	    _DeleteString(&subtype);
	    _DeleteString(&property);
    _EndExceptionBlock

    return result;
    }


static void  ConfirmApply(ui, destination, client_address)
_Ui ui;
 _Surrogate destination;

    _Integer client_address;

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
    _ClientEntry client;
    _Persistent container;

    /*
    ** If the Surrogate has no Container, there is no point in sending the
    ** ApplyConfirm message to the Environment Manager since it won't be able
    ** to find this instance of the Surrogate its Client list!
    */

    _GetValue(destination, _P_Container, lwk_c_domain_persistent, &container);

    if (container == (_Persistent) _NullObject)
	return;

    /*
    ** Ask the Environment Manager to do it -- if that fails, do it
    ** ourselves.
    */

    if (SendApplyConfirmMessage(ui, destination))
	return;

    /*      
    ** Find the Client entry for that Destination and update the Client's
    ** address.  Also, update the StepDestination if we were waiting to get
    ** this Confirmation.
    */

    client = (_ClientEntry) _ClientList_of(ui);

    while (client != (_ClientEntry) 0) {
	if (client->destination == destination
		&& client->address == _PendingClientAddress) {
	    client->address = client_address;

	    if (_StepDestination_of(ui) == _PendingClientAddress)
		_StepDestination_of(ui) = client_address;

	    break;
	}

	client = client->next;
    }

    TriggerRemoveWIP(ui, destination);

    return;
    }


static void  UpdateHistory(ui, origin, destination, operation, follow_type, origin_address, path_navigation)
_Ui ui;
 _Surrogate origin;
 _Surrogate destination;

    _String operation;
 _FollowType follow_type;
 _Integer origin_address;

    _Boolean path_navigation;

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
    _Step volatile step;
    _ClientEntry client;
    _Linkbase linkbase;
    _Surrogate history_origin;
    _Surrogate step_origin;
    _Surrogate step_destination;
    _Surrogate path_origin;
    _Surrogate path_destination;
    _Surrogate previous_origin;
    _Surrogate previous_destination;

    /*
    **	Ask the Environment Manager to do it -- if that fails, do it ourselves.
    */

    if (SendUpdateHistoryMessage(ui, origin, destination, operation,
	    follow_type))
	return;

    /*
    **	Note: there may be no Origin given.  In that case, we shouldn't update
    **	the History.  This can happen when the navigation event is from the
    **	Show History Dialog Box.  However, it can also happen when we Step
    **	Forward into the Origin of a new Composite Path.  In this case,
    **	remember the Step Destination.
    */

    if (origin == (_Surrogate) _NullObject) {
	if (path_navigation) {
	    _StepDestination_of(ui) = _PendingClientAddress;

	    SaveClientOriginAndDestination(ui, _PendingClientAddress,
		(_Surrogate) _NullObject, (_Surrogate) _NullObject,
		destination);
	}

	return;
    }

    /*
    ** Find the Origin of the Current History.  If there is no Current History
    ** yet, create one.
    */

    if (_History_of(ui) != (_Path) _NullObject)
	_GetValue(_History_of(ui), _P_Origin, lwk_c_domain_surrogate,
	    &history_origin);
    else {
	_History_of(ui) = (_Path) _Create(_TypePath);
	history_origin = (_Surrogate) _NullObject;
    }

    /*
    **  Initialize
    */

    step = (_Step) _NullObject;

    _StartExceptionBlock

    /*
    ** Find the previous Origin and Destination for this Client
    */

    GetClientOriginAndDestination(ui, origin_address, &previous_origin,
	&path_origin, &previous_destination);

    path_destination = previous_destination;

    /*
    **	If the previous Destination is not equal to the Origin of the Follow,
    **	and there is some previous Origin, we need to create an inferred Step.
    */

    if (previous_destination != origin
	    && (previous_origin != (_Surrogate) _NullObject
		|| history_origin != (_Surrogate) _NullObject)) {
	/*
	**  The Origin of the Step is either the previous Origin or the Origin
	**  of the History.
	*/

	if (previous_origin != (_Surrogate) _NullObject)
	    step_origin = previous_origin;
	else
	    step_origin = history_origin;

	/*
	**  The Destination is a copy of the Origin of the Follow
	*/

	step_destination = (_Surrogate) _Copy(origin, _False);

	/*
	**  Create the inferred Step.
	*/

	step = CreateStep(step_origin, step_destination,
	    _InferredStepFollowType, _InferredStepOperation);

	/*
	**  Add the inferred Step to the History.
	*/

	_SetValue(step, _P_Path, lwk_c_domain_path, &_History_of(ui),
	    lwk_c_set_property);

	/*
	**  The Destination of the inferred Step is the Origin of the new Step
	*/

	step_origin = step_destination;
    }
    else {
	/*
	** If there was no need to create an inferred Step, the Origin of the
	** new Step is either the previous Origin, the Origin of the History,
	** or a copy of the Origin of the Follow operation.
	*/

	if (previous_origin != (_Surrogate) _NullObject)
	    step_origin = previous_origin;
	else if (history_origin != (_Surrogate) _NullObject)
	    step_origin = history_origin;
	else
	    step_origin = (_Surrogate) _Copy(origin, _False);
    }

    /*
    **	The Destination is a copy of the Destination of the Follow operation.
    */

    step_destination = (_Surrogate) _Copy(destination, _False);

    /*
    **  Create the new Step.
    */

    step = CreateStep(step_origin, step_destination, follow_type, operation);

    /*
    **  Add the new Step to the History.
    */

    _SetValue(step, _P_Path, lwk_c_domain_path, &_History_of(ui),
	lwk_c_set_property);

    /*
    ** Update the Path if necessary
    */

    UpdatePath(ui, origin, destination, operation, follow_type,
	&path_origin, &path_destination);

    /*
    ** Remember the Origin and Destination for these clients
    */

    SaveClientOriginAndDestination(ui, origin_address, step_origin,
	path_origin, origin);

    SaveClientOriginAndDestination(ui, _PendingClientAddress,
	step_destination, path_destination, destination);

    /*
    ** If this was a Path navigation (i.e., a Step Forward) remember the
    ** Destination Client Address -- we may need it to do a CloseView on the
    ** next Step Forward.
    */

    if (path_navigation)
    	_StepDestination_of(ui) = _PendingClientAddress;

    /*
    ** Set the PendingDestination.
    **
    **  Note: There are side effects caused by _SetValue -- that is why we
    **  use it here.
    */

    _SetValue(ui, _P_FollowDestination, lwk_c_domain_surrogate, &destination,
	lwk_c_set_property);

    /*
    ** Update the Show History Dialog Box (if and only if it is visible)
    */

    ShowHistory(ui, _True, (_Closure) 0);

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&step);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  UpdatePath(ui, origin, destination, operation, follow_type, previous_origin, previous_destination)
_Ui ui;
 _Surrogate origin;
 _Surrogate destination;

    _String operation;
 _FollowType follow_type;
 _Surrogate *previous_origin;

    _Surrogate *previous_destination;

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
    _Path path;
    _Step volatile step;
    _Persistent container;
    _Linkbase linkbase;
    _Surrogate path_origin;
    _Surrogate step_origin;
    _Surrogate step_destination;

    /*
    ** Get the Recording Path.  If there is none, return now.
    **
    **  Note: There are side effects caused by _GetValue -- that is why we
    **  use it here.
    */

    _GetValue(ui, _P_RecordingPath, lwk_c_domain_path, &path);

    if (path == (_Path) _NullObject) {
        /*
        ** Return null Origin and Destination Surrogates
        */

	*previous_origin = (_Surrogate) _NullObject;
	*previous_destination = (_Surrogate) _NullObject;

	return;
    }

    /*
    ** Find the Origin of the Path.
    */

    _GetValue(path, _P_Origin, lwk_c_domain_surrogate, &path_origin);

    /*
    ** Make sure that the previous Origin is a Surrogate in the Path.  If not,
    ** forget about the it.  This can happen if the Path was changed on us.
    */

    if (*previous_origin != (_Surrogate) _NullObject) {
	_GetValue(*previous_origin, _P_Container, lwk_c_domain_persistent,
	    &container);

	if (container != path)
	    *previous_origin = (_Surrogate) _NullObject;
    }

    /*
    **  Initialize
    */

    step = (_Step) _NullObject;

    _StartExceptionBlock

    /*
    **	If the previous Destination is not equal to the Origin of the Follow,
    **	and there is some previous Origin, we need to create an inferred Step.
    */

    if (*previous_destination != origin
	    && (*previous_origin != (_Surrogate) _NullObject
		|| path_origin != (_Surrogate) _NullObject)) {
	/*
	**  The Origin of the Step is either the previous Origin or the Origin
	**  of the Path.
	*/

	if (*previous_origin != (_Surrogate) _NullObject)
	    step_origin = *previous_origin;
	else
	    step_origin = path_origin;

	/*
	**  The Destination is a copy of the Origin of the Follow
	*/

	step_destination = (_Surrogate) _Copy(origin, _False);

	/*
	**  Create the inferred Step.
	*/

	step = CreateStep(step_origin, step_destination,
	    _InferredStepFollowType, _InferredStepOperation);

	/*
	**  Add the inferred Step to the Path.
	*/

	_SetValue(step, _P_Path, lwk_c_domain_path, &path, lwk_c_set_property);

	/*
	**  The Destination of the inferred Step is the Origin of the new Step
	*/

	step_origin = step_destination;
    }
    else {
	/*
	** If there was no need to create an inferred Step, the Origin of the
	** new Step is either the previous Origin, the Origin of the Path,
	** or a copy of the Origin of the Follow operation.
	*/

	if (*previous_origin != (_Surrogate) _NullObject)
	    step_origin = *previous_origin;
	else if (path_origin != (_Surrogate) _NullObject)
	    step_origin = path_origin;
	else
	    step_origin = (_Surrogate) _Copy(origin, _False);
    }

    /*
    **	The Destination is a copy of the Destination of the Follow operation.
    */

    step_destination = (_Surrogate) _Copy(destination, _False);

    /*
    **  Create the new Step.
    */

    step = CreateStep(step_origin, step_destination, follow_type, operation);

    /*
    **  Add the new Step to the Path.
    */

    _SetValue(step, _P_Path, lwk_c_domain_path, &path, lwk_c_set_property);

    /*
    **  Update the Path in its linkbase so that the new Step(s) will be
    **	Stored.
    */

    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &linkbase);

    if (linkbase != (_Linkbase) _NullObject)
	_Store(path, linkbase);

    /*
    ** Return the Origin and Destination of the new Path Step
    */

    *previous_origin = step_origin;
    *previous_destination = step_destination;

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&step);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  Highlight(ui, closure)
_Ui ui;
 _Closure closure;

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
    **  Invoke the Highlight operation on the UI Object
    */

    _Highlight(ui);

    return;
    }


static void  ToggleHighlight(ui, closure)
_Ui ui;
 _Closure closure;

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
    _Integer highlighting;

    /*
    **  Toggle the Current Highlighting
    */

    highlighting = _ApplHighlight_of(ui) ^ lwk_c_hl_on;

    /*
    **  Note: we use SetValue here to take advantage of the important
    **	side-effect of issuing the Curreny Changed callback
    */

    _SetValue(ui, _P_ApplHighlight, lwk_c_domain_integer, &highlighting,
	lwk_c_set_property);

    return;
    }


static void  FollowSelected(ui, follow_type, reason, closure)
_Ui ui;
 _FollowType follow_type;
 lwk_reason reason;

    _Closure closure;

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
    _Object origins;

    /*
    ** Ask the application for the Surrogate(s) describing the currently
    ** selected object.
    */

    if (_GetSurrogateCb_of(ui) == (_Callback) _NullObject)
	_Raise(get_surrogate_cb_error);

    origins = (_Object) _NullObject;

    status = (lwk_status) (*_GetSurrogateCb_of(ui))(ui, reason, closure,
	_UserData_of(ui), &origins);

    if (status != _StatusCode(success))	
	_Raise(get_surrogate_cb_error);

    if (origins != (_Object) _NullObject && !_IsValidObject(origins))	
	_Raise(get_surrogate_cb_error);

    /*
    **  Follow one of the Links associated with the Surrogate(s) to
    **  its destination.
    */

    _StartExceptionBlock

    Follow(ui, origins, follow_type);

    /*
    **  If a list of Surrogates was provided by the application, delete the
    **	List.
    */

    if (origins != (_Object) _NullObject)
	if (_IsType(origins, _TypeList))
	    _Delete(&origins);

    /*
    **  If there is an exception, clean up any lists, then reraise it
    */

    _Exceptions
	_WhenOthers
	    if (origins != (_Object) _NullObject)
		if (_IsType(origins, _TypeList))
		    _Delete(&origins);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  Follow(ui, origins, follow_type)
_Ui ui;
 _Object origins;
 _FollowType follow_type;

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
    int cnt;
    _Boolean out_going;
    _Surrogate origin;
    _Surrogate destination;
    _Link link;
    _String volatile operation;
    _List_of(_String) volatile operations;
    _List_of(_Link) volatile link_list;
    _List_of(_Boolean) volatile direction_list;
    _List_of(_List_of(_String)) volatile opr_ids_list;
    _List_of(_List_of(_DDIFString)) volatile opr_names_list;

    /*
    ** Initialization.
    */

    link_list = (_List) _NullObject;
    direction_list = (_List) _NullObject;
    opr_ids_list = (_List) _NullObject;
    opr_names_list = (_List) _NullObject;
    operations = (_List) _NullObject;
    operation = (_String) _NullObject;

    _StartExceptionBlock

    /*
    ** Create a list of Links, a list of directions and a list of
    ** operation lists. For each surrogate fill in the lists.
    */

    cnt = CreateLinkLists(origins, &link_list, &direction_list,
	&opr_ids_list, &opr_names_list);

    /*
    ** Determine the number of links. If there is only one, follow it
    ** else popup the Show Links dialog box.
    */

    if (cnt <= 0) {
	/*
	** No Links -- if the Show Links dialog box is visible,
	** clear it.  Then raise an exception.
	*/

	_ShowLinks(ui, link_list, opr_ids_list, opr_names_list,
	    direction_list, follow_type, _True, _False, (_Closure) 0);

	_Raise(not_linked);
    }
    else if (cnt > 1) {
	/*
	** More than 1 Link -- popup the Show Links dialog box.
	*/

	_ShowLinks(ui, link_list, opr_ids_list, opr_names_list,
	    direction_list, follow_type, _False, _False, (_Closure) 0);
    }
    else {
	/*
	** Get the Link and its Origin and Destination Surrogates.
	*/

	_RemoveElement(link_list, lwk_c_domain_link, &link);
	_RemoveElement(direction_list, lwk_c_domain_boolean, &out_going);

	if (out_going) {
	    _GetValue(link, _P_Target, lwk_c_domain_surrogate,
		&destination);
	    _GetValue(link, _P_Source, lwk_c_domain_surrogate,
		&origin);
	}
	else {
	    _GetValue(link, _P_Source, lwk_c_domain_surrogate,
		&destination);
	    _GetValue(link, _P_Target, lwk_c_domain_surrogate,
		&origin);
	}

	/*
	**  If there is no Default Operation, use the first operation defined
	**  on the Destination.
	*/
	
	_GetValue(ui, _P_DefaultOperation, lwk_c_domain_string,
	    (_AnyPtr) &operation);

	if (operation == (_String) _NullObject) {
	    _RemoveElement(opr_ids_list, lwk_c_domain_list,
		(_AnyPtr) &operations);

	    _RemoveElement(operations, lwk_c_domain_string,
		(_AnyPtr) &operation);

	    _Delete(&operations);
	}

	_Navigate(ui, link, origin, destination, operation, follow_type);

	_DeleteString(&operation);
    }

    /*
    ** Delete the Link/direction/operations Lists.
    */

    _Delete(&link_list);
    _Delete(&direction_list);
    _Delete(&opr_ids_list);
    _Delete(&opr_names_list);

    /*
    ** If any exceptions are raised, delete the lists and reraise the exception
    */

    _Exceptions
	_WhenOthers
	    _Delete(&link_list);
	    _Delete(&direction_list);
	    _Delete(&opr_ids_list);
	    _Delete(&opr_names_list);
	    _Delete(&operations);
	    _DeleteString(&operation);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  ShowHistory(ui, iff_visible, closure)
_Ui ui;
 _Boolean iff_visible;
 _Closure closure;

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
    _Integer count;
    _List steps;
    _List origin_opr_ids;
    _List origin_opr_names;
    _List dest_opr_ids;
    _List dest_opr_names;

    /*
    **	Ask the Environment Manager to do it -- if that fails, do it ourselves.
    */

    if (SendShowHistoryMessage(ui))
	return;

    /*
    **  Initialize
    */

    steps = (_List) _NullObject;
    origin_opr_ids = (_List) _NullObject;
    origin_opr_names = (_List) _NullObject;
    dest_opr_ids = (_List) _NullObject;
    dest_opr_names = (_List) _NullObject;

    /*
    **	If there is no History, make sure that any visible Show History Dialog
    **	Box gets cleared, then either return quietly, or raise an exception.
    */

    if (_History_of(ui) == (_Path) _NullObject) {
	_ShowHistory(ui, steps, origin_opr_ids, origin_opr_names, dest_opr_ids,
	    dest_opr_names, _True, closure);

	if (iff_visible)
	    return;
	else
	    _Raise(no_history);
    }

    _StartExceptionBlock

    /*
    **  Get the List of Steps
    */

    _GetValue(_History_of(ui), _P_Steps, lwk_c_domain_list, &steps);

    /*
    **  If there aren't any, raise an exception.
    */

    if (steps == (_List) _NullObject)
	_Raise(history_is_empty);

    _GetValue(steps, _P_ElementCount, lwk_c_domain_integer, &count);

    if (count <= 0)
	_Raise(history_is_empty);

    /*
    **  Generate the Origin and Destination operations Lists
    */

    CreateStepLists(steps, &origin_opr_ids, &origin_opr_names, &dest_opr_ids,
	&dest_opr_names);

    /*
    **  Invoke the ShowHistory operation on the UI to display the History
    */

    _ShowHistory(ui, steps, origin_opr_ids, origin_opr_names, dest_opr_ids,
	dest_opr_names, iff_visible, closure);

    /*
    **  Delete the Lists we created.
    */

    _Delete(&steps);
    _Delete(&origin_opr_ids);
    _Delete(&origin_opr_names);
    _Delete(&dest_opr_ids);
    _Delete(&dest_opr_names);

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&steps);
	    _Delete(&origin_opr_ids);
	    _Delete(&origin_opr_names);
	    _Delete(&dest_opr_ids);
	    _Delete(&dest_opr_names);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  GoBack(ui, closure)
_Ui ui;
 _Closure closure;

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
    _Step step;
    _Set steps;
    _Integer count;
    _String operation;
    _Surrogate origin;
    _Step previous_step;
    _Surrogate step_origin;
    _Integer follow_type;
    _Integer origin_address;
    _ClientEntry origin_client;
    _Surrogate step_destination;
    _Integer destination_address;
    _ClientEntry destination_client;

    /*
    **	Ask the Environment Manager to do it -- if that fails, do it ourselves.
    */

    if (SendGoBackMessage(ui))
	return;

    /*
    **  If there is no History, raise an exception
    */

    if (_History_of(ui) == (_Path) _NullObject)
	_Raise(no_history);

    /*
    **  Initialize
    */

    steps = (_Set) _NullObject;
    operation = (_String) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the Last Step
    */

    _GetValue(_History_of(ui), _P_LastStep, lwk_c_domain_step, &step);

    /*
    **  If there isn't any, raise an exception.
    */

    if (step == (_Step) _NullObject)
	_Raise(history_is_empty);

    /*
    ** Get the Step's Origin and Destination
    */

    _GetValue(step, _P_Origin, lwk_c_domain_surrogate, &step_origin);
    _GetValue(step, _P_Destination, lwk_c_domain_surrogate, &step_destination);

    /*
    ** Undo the last navigation event.  First, figure out who the principals
    ** where.
    */

    origin_client = FindClientByOrigin(ui, step_origin);

    if (origin_client == (_ClientEntry) 0) {
	origin_address = _UnknownOriginClientAddress;
	origin = step_origin;
    }
    else {
	origin_address = origin_client->address;
	origin = origin_client->destination;
    }

    destination_client = FindClientByOrigin(ui, step_destination);

    if (destination_client == (_ClientEntry) 0)
	destination_address = _UnknownDestinationClientAddress;
    else
	destination_address = destination_client->address;

    /*
    ** Now, determine the Follow Type.  If the Step said the Follow Type was
    ** Visit but the Origin and Destination Clients are the same, the Client
    ** doesn't support Visit (there are some out there!!).
    */

    _GetValue(step, _P_FollowType, lwk_c_domain_integer, &follow_type);

    if ((_FollowType) follow_type == lwk_c_follow_visit
	    && origin_address != destination_address) {
	/*
	** The Follow Type really was Visit and we know the Destination Client.
	** Simply invoke the CloseView callback in the Destination Client.
	*/

	if (_ValidClientAddress(destination_address))
	    SendCloseViewMessage(ui, destination_address);
    }
    else {
	/*
	** The Follow Type was really GoTo.  We have to get back to the Origin
	** and possibly close the Destination.
	**
	** First we need to know what the operation -- it was: the Destination
	** Operation of the Previous Step (if there was one), else the Origin
	** Operation of the History, else the Default operation for the Ui,
	** else the Default Operation for the Surrogate type.
	*/

	_GetValue(step, _P_PreviousStep, lwk_c_domain_step,
	    &previous_step);

	if (previous_step != (_Step) _NullObject)
	    _GetValue(previous_step, _P_DestinationOperation,
		lwk_c_domain_string, &operation);
	else
	    _GetValue(_History_of(ui), _P_OriginOperation,
		lwk_c_domain_string, &operation);

	if (operation == (_String) _NullObject) {
	    _GetValue(ui, _P_DefaultOperation, lwk_c_domain_string, &operation);

	    if (operation == (_String) _NullObject)
		operation = GetDefaultOperation(origin);
	}

	if (origin_address == destination_address) {
	    /*
	    ** The Origin and Destination were the same Client: try to send a
	    ** Go To (i.e. Go Back) request to the Origin.  If that fails,
	    ** Visit the Origin.
	    */

	    if (!_ValidClientAddress(origin_address)
		    || !SendApplyMessage(ui, origin, operation,
			lwk_c_follow_go_to, origin_address))
		Navigate(ui, (_Link) _NullObject,
		    (_Surrogate) _NullObject, origin, operation,
		    lwk_c_follow_visit, _False);
	}
	else {
	    /*
	    ** The Origin and Destination were different Clients: re-Visit the
	    ** Origin, then invoke the CloseView callback in the Destination.
	    */

	    Navigate(ui, (_Link) _NullObject,
		(_Surrogate) _NullObject, origin, operation,
		lwk_c_follow_visit, _False);

	    if (_ValidClientAddress(destination_address))
		SendCloseViewMessage(ui, destination_address);
	}

	_DeleteString(&operation);
    }

    /*
    ** Forget about the Destination Client
    */
    
    if (destination_client != (_ClientEntry) 0)
	DeleteClientEntry(ui, destination_client);

    /*
    ** Delete the Step
    */

    _Delete(&step);

    /*
    ** If the Step Origin or Destination have become orphans, delete them.
    */

    _GetValue(step_origin, _P_InterLinks, lwk_c_domain_set, &steps);

    if (steps == (_Set) _NullObject)
	count = 0;
    else {
	_GetValue(steps, _P_ElementCount, lwk_c_domain_integer, &count);
	_Delete(&steps);
    }

    if (count <= 0)
	_Delete(&step_origin);

    _GetValue(step_destination, _P_InterLinks, lwk_c_domain_set, &steps);

    if (steps == (_Set) _NullObject)
	count = 0;
    else {
	_GetValue(steps, _P_ElementCount, lwk_c_domain_integer, &count);
	_Delete(&steps);
    }

    if (count <= 0)
	_Delete(&step_destination);

    /*
    ** If the History is now empty, clean up
    */

    _GetValue(_History_of(ui), _P_FirstStep, lwk_c_domain_step, &step);

    if (step == (_Step) _NullObject) {
	_Delete(&_History_of(ui));
	FreeClientList(ui);
    }

    /*
    ** Update the Show History Dialog Box (if and only if it is visible)
    */

    ShowHistory(ui, _True, (_Closure) 0);

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&steps);
	    _DeleteString(&operation);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  StepForward(ui, closure)
_Ui ui;
 _Closure closure;

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
    _Boolean applied;
    _Surrogate origin;
    _Surrogate destination;
    _FollowType follow_type;
    _Integer origin_address;
    _Integer destination_address;
    _String volatile operation;

    /*
    **	Ask the Environment Manager to do it -- if that fails, do it ourselves.
    */

    if (SendStepForwardMessage(ui))
	return;

    /*
    ** Initialize
    */

    operation = (_String) _NullObject;

    _StartExceptionBlock

    /*
    ** Find the next Step and get the parameters for the navigation event.
    */

    FindNextStep(ui, &origin, &destination, &operation, &follow_type);

    /*
    ** Keep the client origin address, we need it in case of a GoTo, and
    ** Navigate modify it. If we are the environment manager, the client origin
    ** address is the last destination, else we have to find it. 
    */

    if (follow_type == lwk_c_follow_go_to)
	if (_EnvironmentManager_of(ui))
	    origin_address = _StepDestination_of(ui);
	else 
	    origin_address = GetSurrogateClientAddress(ui, origin);
	
    Navigate(ui, (_Link) _NullObject, origin, destination, operation,
	     follow_type, _True);

    /*
    ** If the Follow Type is Go To, send a CloseView request to the Origin
    ** of the Step, only if this Step is between two different windows.
    */

    if (follow_type == lwk_c_follow_go_to) 

	if (_ValidClientAddress(origin_address)) {

	    /*
	    **	Find the appropriate Client.
	    */

	    destination_address = GetSurrogateClientAddress(ui, destination);

	    if (origin_address != destination_address)
		SendCloseViewMessage(ui, origin_address);
	}

    _DeleteString(&operation);

    /*
    **  If there is an exception, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&operation);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetSurrogate(ui, menu_action, reason, closure)
_Ui ui;
 _Integer menu_action;
 lwk_reason reason;

    _Closure closure;

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
    _Object surrogates;
    _Surrogate surrogate;
    _Persistent container;
    _Linknet recording_linknet;
    _Linkbase linkbase;
    _Surrogate current_surrogate;

    surrogate = (_Surrogate) _NullObject;

    /*
    ** Ask the application for the Surrogate(s) describing the currently
    ** selected object before asking it to create a new one.
    */

    if (_GetSurrogateCb_of(ui) == (_Callback) _NullObject)
	_Raise(get_surrogate_cb_error);

    surrogates = (_Object) _NullObject;

    status = (lwk_status) (*_GetSurrogateCb_of(ui))(ui,	reason, closure,
	_UserData_of(ui), &surrogates);

    if (status != _StatusCode(success))	
	_Raise(get_surrogate_cb_error);

    if (surrogates != (_Object) _NullObject && !_IsValidObject(surrogates))
	_Raise(get_surrogate_cb_error);

    if (surrogates != (_Object) _NullObject) {

	if (_IsType(surrogates, _TypeSurrogate)) {
	    surrogate = surrogates;
	}
	else if (_IsType(surrogates, _TypeList)) {
	    surrogate = SelectSurrogate(ui, surrogates);
	}
	else
	    _Raise(inv_object);
    }

    /*
    ** Ask the application to create a Surrogate if no surrogate was returned by
    ** the GetSurrogate callback.
    */

    if (surrogate == (_Surrogate) _NullObject) {

	if (_CreateSurrogateCb_of(ui) == (_Callback) _NullObject)
	    _Raise(create_surrogate_cb_error);

	status = (lwk_status) (*_CreateSurrogateCb_of(ui))(ui, reason,
	    closure, _UserData_of(ui), &surrogate);

	if (status != _StatusCode(success))
	    _Raise(create_surrogate_cb_error);

        /*
	** If the application returned an invalid object raise an exception.
	** However if the application did not return any surrogate, don't do
	** anything.
	*/

	if (surrogate == (_Surrogate) _NullObject)
	    return;
	else
	    if (!_IsValidObject(surrogate))
		_Raise(create_surrogate_cb_error);
    }

    /*
    **	Make sure the Surrogate is in the some stored Linknet -- if it is not,
    **	put the Surrogate or a copy of it into the Recording Linknet.
    */

    _GetValue(surrogate, _P_Linkbase, lwk_c_domain_linkbase, &linkbase);

    if (linkbase != (_Linkbase) _NullObject)
	current_surrogate = surrogate;
    else {
	/*
	**  Find the Recording Linknet -- if there is none, raise an exception.
	**
	**  Note: There are side effects caused by _GetValue -- that is why we
	**  use it here.
	*/

	_GetValue(ui, _P_RecordingLinknet, lwk_c_domain_linknet,
	    &recording_linknet);

	if (recording_linknet == (_Linknet) _NullObject)
	    _Raise(no_recording_linknet);

	/*
	**  If the Surrogate is not in any Linknet, put it into the Recording
	**  Linknet, else if it is not already in the Recording Linknet, put a
	**  copy of it into the Recording Linknet.
	*/

	_GetValue(surrogate, _P_Container, lwk_c_domain_persistent, &container);

	if (container == (_Persistent) _NullObject
		|| container == recording_linknet)
	    current_surrogate = surrogate;
	else
	    current_surrogate = (_Surrogate) _Copy(surrogate, _False);

	if (container != recording_linknet)
	    _SetValue(current_surrogate, _P_Container, lwk_c_domain_linknet,
		&recording_linknet, lwk_c_set_property);

	/*
	**  Make sure the new Surrogate gets stored in the Linkbase
	*/

	_GetValue(recording_linknet, _P_Linkbase, lwk_c_domain_linkbase,
	    &linkbase);

	if (linkbase != (_Linkbase) _NullObject)
	    _Store(recording_linknet, linkbase);
    }

    /*	
    ** Set the Surrogate to be either the Pending Source or the Pending Target.
    ** Also set an indicator that we are the owner of the Pending
    ** Source/Target.  This indicator is decremented as the Pending
    ** Source/Target change notifications come in.  If it is greater than zero,
    ** we own the Pending Source/Target.  We set it to 2 because we will get a
    ** Pending Source/Target changed message as a side-effect of setting the
    ** Pending Source/Target value!
    */

    switch(menu_action){
	case lwk_c_dxm_menu_start_link:
	    _SetValue(ui, _P_PendingSource, lwk_c_domain_surrogate,
		&current_surrogate, lwk_c_set_property);

	    _PendingSourceOwner_of(ui) = 2;

	    break;

	case lwk_c_dxm_menu_comp_link:
	    _SetValue(ui, _P_PendingTarget, lwk_c_domain_surrogate,
		&current_surrogate, lwk_c_set_property);

	    _PendingTargetOwner_of(ui) = 2;

	    break;
    }

    return;
    }


static _Surrogate  SelectSurrogate(ui, surrogates)
_Ui ui;
 _List surrogates;

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
    _Integer i;
    _Integer elements;
    _Surrogate surrogate;
    _Linknet recording_linknet;
    _Linknet linknet;

    surrogate = (_Surrogate) _NullObject;
    
    /*
    ** Get the number of elements in the List
    */

    _GetValue(surrogates, _P_ElementCount, lwk_c_domain_integer, &elements);

    /*
    ** Process only a non-empty list
    */

    if (elements > 0) {
    
	_GetValue(ui, _P_RecordingLinknet, lwk_c_domain_linknet,
	    &recording_linknet);
		    
	/*
	** Run down the list of surrogates to see if one is in the recording
	** linknet and return it if found. Otherwise just return the last
	** one in the list.
	*/

	for (i = 0; i < elements; i++) {

	    _RemoveElement(surrogates, lwk_c_domain_surrogate, &surrogate);

	    _GetValue(surrogate, lwk_c_p_container, lwk_c_domain_persistent,
		 &linknet);
		 
	    if (linknet == recording_linknet)
		break;
	}
	_Delete(&surrogates);
    }

    return surrogate;
    }
    

static void  CompleteLink(ui, reason, closure, dialog)
_Ui ui;
 lwk_reason reason;
 _Closure closure;

    _Boolean dialog;

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
    _Surrogate source;
    _Surrogate target;
    _Persistent container;
    _Linknet recording_linknet;
    _Date volatile date;
    _Link volatile link;
    _DDIFString volatile ddifstring;
    _Set_of(_DDIFString) volatile keywords;

    /*
    **  Find the Recording Linknet -- if there isn't one, raise an exception.
    **
    **	Note: There are side effects caused by _GetValue -- that is why we use
    **	it here.
    */

    _GetValue(ui, _P_RecordingLinknet, lwk_c_domain_linknet,
	&recording_linknet);

    if (recording_linknet == (_Linknet) _NullObject)
	_Raise(no_recording_linknet);

    /*
    **  Get the Pending Source and Pending Target.
    **
    **	Note: There are side effects caused by _GetValue -- that is why we use
    **	it here.
    */

    _GetValue(ui, _P_PendingSource, lwk_c_domain_surrogate, &source);

    if (source == (_Surrogate) _NullObject)
	_Raise(no_pending_source);

    _GetValue(ui, _P_PendingTarget, lwk_c_domain_surrogate, &target);

    if (target == (_Surrogate) _NullObject)
	_Raise(no_pending_target);

    /*
    **	Now, ask the Environment Manager to actually do the Link -- if that
    **	fails, we'll do it ourselves.
    */

    if (SendCompleteLinkMessage(ui, dialog))
	return;

    /*
    **	If the Pending Source or Target is not already in the Recording Linknet,
    **	make a copy of it.  It will be put it in the Recording Linknet along
    **	with the Link.
    */

    _GetValue(source, _P_Container, lwk_c_domain_persistent, &container);

    if (recording_linknet != container)
	source = (_Surrogate) _Copy(source, _False);

    _GetValue(target, _P_Container, lwk_c_domain_persistent, &container);

    if (recording_linknet != container)
	target = (_Surrogate) _Copy(target, _False);

    /*
    ** Initialization.
    */

    link = (_Link) _NullObject;
    keywords = (_Set) _NullObject;
    ddifstring = (_DDIFString) _NullObject;
    date = (_Date) _NullObject;

    _StartExceptionBlock

    /*
    ** Create a new Link (or reuse any left-over PendingLinks).
    */

    if (_PendingLink_of(ui) == (_Link) _NullObject)
	link = (_Link) _Create(_TypeLink);
    else
	link = _PendingLink_of(ui);

    /*
    **  Default the Source/Target Description/Keywords properties of the
    **	Link from the Surrogates.
    */

    _GetValue(source, _P_Description, lwk_c_domain_ddif_string,
	(_AnyPtr) &ddifstring);
    _SetValue(link, _P_SourceDescription, lwk_c_domain_ddif_string,
	(_AnyPtr) &ddifstring, lwk_c_set_property);

    _DeleteDDIFString(&ddifstring);

    _GetValue(target, _P_Description, lwk_c_domain_ddif_string,
	(_AnyPtr) &ddifstring);
    _SetValue(link, _P_TargetDescription, lwk_c_domain_ddif_string,
	(_AnyPtr) &ddifstring, lwk_c_set_property);

    _DeleteDDIFString(&ddifstring);

    _GetValue(source, _P_Keywords, lwk_c_domain_set, (_AnyPtr) &keywords);
    _SetValue(link, _P_SourceKeywords, lwk_c_domain_set, (_AnyPtr) &keywords,
	lwk_c_set_property);

    _Delete(&keywords);

    _GetValue(target, _P_Keywords, lwk_c_domain_set, (_AnyPtr) &keywords);
    _SetValue(link, _P_TargetKeywords, lwk_c_domain_set, (_AnyPtr) &keywords,
	lwk_c_set_property);

    _Delete(&keywords);

    /*
    **  Default the Relationship Type
    */

    _GetValue(ui, _P_DefaultRelationship, lwk_c_domain_ddif_string,
	(_AnyPtr) &ddifstring);

    if (ddifstring == (_DDIFString) _NullObject)
	ddifstring = _StringToDDIFString(_DefaultRelationshipType);

    _SetValue(link, _P_RelationshipType, lwk_c_domain_ddif_string,
	(_AnyPtr) &ddifstring, lwk_c_set_property);

    _DeleteDDIFString(&ddifstring);

    /*
    **  Default the Author to the current user
    */

    ddifstring = _UserNameToDDIFString;
    _SetValue(link, _P_Author, lwk_c_domain_ddif_string, (_AnyPtr) &ddifstring,
	lwk_c_set_property);

    _DeleteDDIFString(&ddifstring);

    /*
    **  Default the Creation Date to Now
    */

    date = _NowToDate;
    _SetValue(link, _P_CreationDate, lwk_c_domain_date, (_AnyPtr) &date,
	lwk_c_set_property);

    _DeleteDate(&date);

    /*
    ** Popup the Complete Link Dialog Box, or go ahead and confirm
    ** the Link
    */

    _PendingLink_of(ui) = link;

    if (dialog)
	_CompleteLink(ui, _PendingLink_of(ui), _False, closure);
    else {
	_Boolean retain_source;
	_Boolean retain_target;

	/*
	**  Note: There are side effects caused by _GetValue -- that is why we
	**  use it here.
	*/

	_GetValue(ui, _P_RetainSource, lwk_c_domain_boolean,
	    &retain_source);

	_GetValue(ui, _P_RetainTarget, lwk_c_domain_boolean,
	    &retain_target);

	_ConfirmLink(ui, _True, retain_source, retain_target);
    }

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&_PendingLink_of(ui));
	    _Delete(&keywords);
	    _DeleteDDIFString(&ddifstring);
	    _DeleteDate(&date);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  Annotate(ui, reason, closure)
_Ui ui;
 lwk_reason reason;
 _Closure closure;

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
    _Surrogate surrogate;
    _Persistent container;
    _Linknet recording_linknet;
    _Linkbase linkbase;

    /*
    ** Ask the application to create a Surrogate.
    */

    if (_CreateSurrogateCb_of(ui) == (_Callback) _NullObject)
	_Raise(create_surrogate_cb_error);

    surrogate = (_Surrogate) _NullObject;

    status = (lwk_status) (*_CreateSurrogateCb_of(ui))(ui, reason, closure,
	_UserData_of(ui), &surrogate);

    /*
    **	If the application returned a failure status or an invalid Surrogate,
    **	raise an exception.
    */

    if (status != _StatusCode(success) || !_IsValidObject(surrogate))
	_Raise(create_surrogate_cb_error);

    /*
    **	Make sure the Surrogate is in the some stored Linknet -- if it is not,
    **	put the Surrogate or a copy of it into the Recording Linknet.
    */

    _GetValue(surrogate, _P_Linkbase, lwk_c_domain_linkbase, &linkbase);

    if (linkbase == (_Linkbase) _NullObject) {
	/*
	**  Find the Recording Linknet -- if there is none, raise an exception.
	**
	**  Note: There are side effects caused by _GetValue -- that is why we
	**  use it here.
	*/

	_GetValue(ui, _P_RecordingLinknet, lwk_c_domain_linknet,
	    &recording_linknet);

	if (recording_linknet == (_Linknet) _NullObject)
	    _Raise(no_recording_linknet);

	/*
	**  If the Surrogate is not in any Linknet, put it into the Recording
	**  Linknet, else if it is not already in the Recording Linknet, put a
	**  copy of it into the Recording Linknet.
	*/

	_GetValue(surrogate, _P_Container, lwk_c_domain_persistent, &container);

	if (container != (_Persistent) _NullObject
		&& container != recording_linknet)
	    surrogate = (_Surrogate) _Copy(surrogate, _False);

	if (container != recording_linknet)
	    _SetValue(surrogate, _P_Container, lwk_c_domain_linknet,
		&recording_linknet, lwk_c_set_property);

	/*
	**  Make sure the new Surrogate gets stored in the Linkbase
	*/

	_GetValue(recording_linknet, _P_Linkbase, lwk_c_domain_linkbase,
	    &linkbase);

	if (linkbase != (_Linkbase) _NullObject)
	    _Store(recording_linknet, linkbase);
    }


    /*
    ** Apply the Annotate operation to the Surrogate
    */

    _Apply(ui, _AnnotateOperationId, surrogate);

    return;
    }


static void  ShowLinks(ui, reason, closure, update)
_Ui ui;
 lwk_reason reason;
 _Closure closure;

    _Boolean update;

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
    int cnt;
    lwk_status status;
    _Object origins;
    _List_of(_Link) links;
    _List_of(_Boolean) directions;
    _List_of(_List_of(_String)) opr_ids;
    _List_of(_List_of(_DDIFString)) opr_names;

    /*
    ** Ask the application for the Surrogate(s) describing the currently
    ** selected object.
    */

    if (update && (_GetSurrogateCb_of(ui) == (_Callback) _NullObject))
	return;

    if (_GetSurrogateCb_of(ui) == (_Callback) _NullObject)
	_Raise(get_surrogate_cb_error);

    origins = (_Object) _NullObject;

    status = (lwk_status) (*_GetSurrogateCb_of(ui))(ui, reason,	closure,
	_UserData_of(ui), &origins);

    if (status != _StatusCode(success))	
	_Raise(get_surrogate_cb_error);

    if (origins != (_Object) _NullObject && !_IsValidObject(origins))	
	_Raise(get_surrogate_cb_error);

    /*
    **  Initialization
    */

    links = (_List) _NullObject;
    directions = (_List) _NullObject;
    opr_ids = (_List) _NullObject;
    opr_names = (_List) _NullObject;

    _StartExceptionBlock

    /*	
    ** Create the Links Lists.
    */

    cnt = CreateLinkLists(origins, &links, &directions,
	&opr_ids, &opr_names);

    /*
    ** If there were some Links, popup the Show Links Dialog Box.
    ** If not, raise an exception after making sure that an already visible
    ** Show Links Dialog Box gets cleared.
    */

    if (cnt > 0 && !update)
	_ShowLinks(ui, links, opr_ids, opr_names, directions,
	    lwk_c_follow_implicit_go_to, _False, update, closure);
    else {
	_ShowLinks(ui, links, opr_ids, opr_names, directions,
	    lwk_c_follow_implicit_go_to, _True, update, closure);

	_Raise(not_linked);
    }

    /*
    ** Delete the Link/direction/operations Lists.
    */

    _Delete(&links);
    _Delete(&directions);
    _Delete(&opr_ids);
    _Delete(&opr_names);

    /*
    **  If a list of Surrogates was provided by the application, delete the
    **	List.
    */

    if (origins != (_Object) _NullObject)
	if (_IsType(origins, _TypeList))
	    _Delete(&origins);

    /*
    ** If any exceptions are raised, delete the Lists and reraise the exception
    */

    _Exceptions
	_WhenOthers
	    _Delete(&links);
	    _Delete(&directions);
	    _Delete(&opr_ids);
	    _Delete(&opr_names);

	    if (origins != (_Object) _NullObject)
		if (_IsType(origins, _TypeList))
		    _Delete(&origins);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


_Boolean  LwkSelectedSurrLinked(ui, closure)
_Ui ui;
 _Closure closure;

/*
**++
**  Functional Description:
**  
**	Boolean function which returns true if there are selected
**	linked surrogates associated with the given ui object.
**
**	Primarily used to determine whether or not GoTo, Visit, or
**	Show Links... should be available (or dimmed) from the LinkWorks
**	Link menu.
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
    _Integer cnt;
    _Integer count;
    int i;
    lwk_status status;
    _Integer elements;
    _Object origins;
    _List_of(_Link) link_list;
    _Surrogate surrogate;
    _List origin_list;

    origin_list = (_List) _NullObject;
    origins = (_Object) _NullObject;


    /* If there is no GetSurrogate callback return False */
	
    if (_GetSurrogateCb_of(ui) == (_Callback) _NullObject)
	return _False;

    /*
    ** Ask the application for the Surrogate(s) describing the currently
    ** selected object.
    */

    status = (lwk_status) (*_GetSurrogateCb_of(ui))(ui,
	lwk_c_reason_menu_pulldown, closure, _UserData_of(ui), &origins);

    if (status != _StatusCode(success))	
	return _False;

    if (origins != (_Object) _NullObject && !_IsValidObject(origins))	
	return _False;

    if (origins == (_Object) _NullObject)
	return _False;

    
    /*
    ** Determine if origins is of type Surrogate or of type List.
    */

    if (_IsType(origins, _TypeSurrogate)) {

	/*
	** Get the set of Links associated with the Surrogate.
	*/

	link_list = (_List) _NullObject;
	link_list = (_List) _GetValueList(origins, _P_InterLinks);

	/*
	** Get the number of Links in the set.
	*/

	if (link_list == (_List) _NullObject)
	    count = 0;
	else
	    _GetValue(link_list, _P_ElementCount,
		lwk_c_domain_integer, &count);

	_Delete(&link_list);

    }
    else if (_IsType(origins, _TypeList)) {

	/*
	** Determine the number of elements in the List.
	*/

	_GetValue(origins, _P_ElementCount, lwk_c_domain_integer, &elements);

	if (elements <= 0)
	    return _False;

	origin_list = (_List) _Copy(origins, _False);

	count = 0;

	for (i = 0; i < elements; i++) {

	    _RemoveElement(origin_list, lwk_c_domain_surrogate, &surrogate);

            cnt = 0;
	    link_list = (_List) _NullObject;
	    link_list = (_List) _GetValueList(surrogate,
		_P_InterLinks);

	    if (link_list != (_List) _NullObject)
		_GetValue(link_list, _P_ElementCount,
		    lwk_c_domain_integer, &cnt);

	    count += cnt;

	    _Delete(&link_list);
    
	}

	_Delete(&origin_list);

    }


    /*
    **  If a list of Surrogates was provided by the application, delete the
    **	List.
    */

    if (origins != (_Object) _NullObject)
	if (_IsType(origins, _TypeList))
	    _Delete(&origins);


    if (count > 0)
	return _True;
    else
	return _False;

    }


static _Boolean  CallbackApply(ui, destination, operation, follow_type)
_Ui ui;
 _Surrogate destination;
 _String operation;

    _FollowType follow_type;

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
    lwk_reason reason;
    _String type;
    _Boolean found;
    _Boolean applied;

    /*
    **  Validate the arguments
    */

    if (destination == (_Surrogate) _NullObject
	    || operation == (_String) _NullObject)
	return _False;

    /*
    **  Initialization
    */

    type = (_String) _NullObject;

    applied = _False;

    _StartExceptionBlock

    /*
    **  See if there is an Apply callback provided
    */

    if (_ApplyCb_of(ui) != (_Callback) _NullObject) {
	/*
	**  See of the type of the Surrogate is in the List of Supported
	**  Surrogate Types
	*/

	if (_SupportedSurrogates_of(ui) != (_List) _NullObject) {
	    _GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
		(_AnyPtr) &type);

	    found = (_Boolean) _Iterate(_SupportedSurrogates_of(ui),
		lwk_c_domain_string, (_Closure) type, FindSurrogateSubType);

	    _DeleteString(&type);

	    if (found) {
		/*
		**  See if the operation is in the List of Supported Operations
		*/

		if (_SupportedOperations_of(ui) != (_List) _NullObject) {
		    found = (_Boolean) _Iterate(_SupportedOperations_of(ui),
			lwk_c_domain_string, (_Closure) operation,
			FindOperation);

		    if (found) {
			/*
			**  All criteria are satisfied, so invoke the callback
			*/

			applied = _True;

			if (follow_type == lwk_c_follow_go_to)
			    reason = lwk_c_reason_goto;
			else
			    reason = lwk_c_reason_visit;

			status = (lwk_status) (*_ApplyCb_of(ui))(ui, reason,
			    (_Closure) 0, _UserData_of(ui), destination,
			    operation, follow_type);

			if (status != _StatusCode(success))
			    _Raise(apply_cb_error);
		    }
	        }
	    }
	}
    }

    /*
    ** If any exceptions are raised, delete any Strings reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&type);
	    _Reraise;
    _EndExceptionBlock

    return applied;
    }


static void  CallbackCloseView(ui, reason, closure)
_Ui ui;
 lwk_reason reason;
 _Closure closure;

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
    _Status status;

    /*
    ** If the application provided a Close View callback, invoke it
    */

    if (_CloseViewCb_of(ui) != (_Callback) _NullObject) {
	status = (_Status) (*_CloseViewCb_of(ui))(ui, reason, closure, 
	    _UserData_of(ui));

	if (status != _StatusCode(success))
	    _Raise(close_view_cb_error);
    }

    return;
    }         


static int  CreateLinkLists(origins, links, directions, opr_ids, opr_names)
_Object origins;
 _List *links;

    _List *directions;
 _List *opr_ids;
 _List *opr_names;

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
    int i;
    int count;
    _Integer elements;
    _Surrogate surrogate;
    _List volatile origin_list;
    _List volatile link_list;
    _List volatile direction_list;
    _List volatile opr_ids_list;
    _List volatile opr_names_list;

    /*
    ** If there were no Origins, there are no Links
    */

    if (origins == (_Object) _NullObject)
	return 0;

    /*
    **  Initialization.
    */

    origin_list = (_List) _NullObject;
    link_list = (_List) _NullObject;
    direction_list = (_List) _NullObject;
    opr_ids_list = (_List) _NullObject;
    opr_names_list = (_List) _NullObject;

    _StartExceptionBlock

    /*
    ** Determine if origins is of type Surrogate or of type List.
    */

    if (_IsType(origins, _TypeSurrogate)) {
	/*
	** It's a single Surrogate, so fill in the list of Links,
	** directions and operations.
	*/

	link_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_link, 1);

	direction_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_boolean, 1);

	opr_ids_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_list, 1);

	opr_names_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_list, 1);

	count = FillInLinkLists(origins, link_list,
	    direction_list, opr_ids_list, opr_names_list);
    }
    else if (_IsType(origins, _TypeList)) {
	/*
	** Determine the number of elements in the List.
	*/

	_GetValue(origins, _P_ElementCount, lwk_c_domain_integer, &elements);

	/*
	** If the List is empty, raise an exception.
	*/

	if (elements <= 0)
	    _Raise(list_empty);

	/*
	** For each surrogate fill in the lists.
	*/

	link_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_link, elements);

	direction_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_boolean, elements);

	opr_ids_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_list, elements);

	opr_names_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_list, elements);

	count = 0;

	origin_list = (_List) _Copy(origins, _False);

	for (i = 0; i < elements; i++) {
	    _RemoveElement(origin_list, lwk_c_domain_surrogate, &surrogate);

	    count += FillInLinkLists(surrogate, link_list,
		direction_list, opr_ids_list, opr_names_list);
	}

	_Delete(&origin_list);
    }
    else
	_Raise(inv_object);
	
    /*
    ** Return the lists.
    */

    *links = link_list;
    *directions = direction_list;
    *opr_ids = opr_ids_list;
    *opr_names = opr_names_list;

    /*
    ** If any exceptions are raised, delete the Lists and reraise the exception
    */

    _Exceptions
	_WhenOthers
	    _Delete(&origin_list);
	    _Delete(&link_list);
	    _Delete(&direction_list);
	    _Delete(&opr_ids_list);
	    _Delete(&opr_names_list);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the number of Links in the Lists.
    */

    return count;
    }


static int  FillInLinkLists(surrogate, links, directions, opr_ids, opr_names)
_Surrogate surrogate;
 _List links;

    _List directions;
 _List opr_ids;
 _List opr_names;

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
    int i, j;
    _Integer cnt;
    _String string;
    _DDIFString opr_name;
    _Boolean out_going;
    _Surrogate destination;
    _Persistent link;
    int volatile op_cnt;
    _String volatile type;
    _String volatile *supported_operations;
    _List_of(_String) volatile opr_ids_list;
    _List_of(_DDIFString) volatile opr_names_list;
    _List_of(_Link) volatile link_list;

    /*
    ** Initialization.
    */

    op_cnt = 0;
    type = (_String) _NullObject;
    link_list = (_List) _NullObject;
    opr_ids_list = (_List) _NullObject;
    opr_names_list = (_List) _NullObject;

    _StartExceptionBlock

    /*
    ** Get the set of Links associated with the Surrogate.
    */

    link_list = (_List) _GetValueList(surrogate, _P_InterLinks);

    /*
    ** Get the number of Links in the set.
    */

    if (link_list == (_List) _NullObject)
	cnt = 0;
    else
	_GetValue(link_list, _P_ElementCount, lwk_c_domain_integer, &cnt);

    /*
    ** For each link, fill in lists.
    */

    for (i = 0; i < cnt; i++) {
	/*
	** Get the Link.
	*/

	_RemoveElement(link_list, lwk_c_domain_persistent, &link);

	/*
	** Add it to the Links List.
	*/

	_AddElement(links, lwk_c_domain_link, &link, _True);

	/*
	** Get the Target of the Link and compare it to the Surrogate. If
	** they are different, this is an out going link, else this is an
	** in comming link and the Destination Surrogate is the Source.
	*/

	_GetValue(link, _P_Target, lwk_c_domain_surrogate, &destination);

	if (surrogate == destination) {
	    out_going = _False;

	    _GetValue(link, _P_Source, lwk_c_domain_surrogate,
		&destination);

            /*
            ** Get the supported Operations for this Surrogate SubType
            */

	    _GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
		(_AnyPtr) &type);

	    op_cnt = LwkRegTypeOperations(type, &supported_operations);
                     
	    opr_ids_list =
		(_List) _CreateList(_TypeList, lwk_c_domain_string, op_cnt);

	    opr_names_list =
		(_List) _CreateList(_TypeList, lwk_c_domain_ddif_string,
		    op_cnt);

	    if (op_cnt > 0) {
		for (j = 0; j < op_cnt; j++) {
		    _AddElement(opr_ids_list, lwk_c_domain_string,
			(_AnyPtr) &supported_operations[j], _False);

		    opr_name = LwkRegOperationName(type,
			supported_operations[j]);

		    _AddElement(opr_names_list, lwk_c_domain_ddif_string,
			&opr_name, _False);
		}

		_FreeMem(supported_operations);
		op_cnt = 0;
	    }

	    _DeleteString(&type);
	}
	else {
	    out_going = _True;

            /*
            ** Get the supported Operations for this Surrogate SubType
            */

	    _GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
		(_AnyPtr) &type);

	    op_cnt = LwkRegTypeOperations(type, &supported_operations);

	    opr_ids_list =
		(_List) _CreateList(_TypeList, lwk_c_domain_string, op_cnt);

	    opr_names_list =
		(_List) _CreateList(_TypeList, lwk_c_domain_ddif_string,
		    op_cnt);

	    if (op_cnt > 0) {
		for (j = 0; j < op_cnt; j++) {
		    _AddElement(opr_ids_list, lwk_c_domain_string,
			(_AnyPtr) &supported_operations[j], _False);

		    opr_name = LwkRegOperationName(type,
			supported_operations[j]);

		    _AddElement(opr_names_list, lwk_c_domain_ddif_string,
			&opr_name, _False);
		}

		_FreeMem(supported_operations);
		op_cnt = 0;
	    }

	    _DeleteString(&type);
	}

	/*
	** Add the direction to the direction list.
	*/

	_AddElement(directions, lwk_c_domain_boolean, &out_going, _True);

	/*
	** Add the operation List to the operations List List.
	*/

	_AddElement(opr_ids, lwk_c_domain_list, (_AnyPtr) &opr_ids_list, _False);
	_AddElement(opr_names, lwk_c_domain_list,
	    (_AnyPtr) &opr_names_list, _False);

	opr_ids_list = (_List) _NullObject;
	opr_names_list = (_List) _NullObject;
    }

    /*
    ** Delete the Links List.
    */

    _Delete(&link_list);

    /*
    ** If any exceptions are raised, delete the lists and reraise the exception
    */

    _Exceptions
	_WhenOthers
	    if (op_cnt > 0)
		_FreeMem(supported_operations);

	    _DeleteString(&type);
	    _Delete(&link_list);
	    _Delete(&opr_ids_list);
	    _Delete(&opr_names_list);

	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the number of Links added to the Lists.
    */

    return cnt;
    }


static void  CreateStepLists(steps, origin_opr_ids, origin_opr_names, dest_opr_ids, dest_opr_names)
_List steps;
 _List *origin_opr_ids;

    _List *origin_opr_names;
 _List *dest_opr_ids;
 _List *dest_opr_names;

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
    int i, j;
    _Step step;
    _String string;
    _Integer elements;
    _Surrogate surrogate;
    int volatile op_cnt;
    _DDIFString opr_name;
    _String volatile type;
    _List volatile steps_list;
    _List volatile opr_ids_list;
    _List volatile opr_names_list;
    _List volatile origin_opr_ids_list;
    _List volatile origin_opr_names_list;
    _List volatile dest_opr_ids_list;
    _List volatile dest_opr_names_list;
    _String volatile *supported_operations;

    /*
    **  Initialization.
    */

    op_cnt = 0;
    steps_list = (_List) _NullObject;
    opr_ids_list = (_List) _NullObject;
    opr_names_list = (_List) _NullObject;
    origin_opr_ids_list = (_List) _NullObject;
    origin_opr_names_list = (_List) _NullObject;
    dest_opr_ids_list = (_List) _NullObject;
    dest_opr_names_list = (_List) _NullObject;

    _StartExceptionBlock

    /*
    ** Determine the number of elements in the Steps List.
    */

    _GetValue(steps, _P_ElementCount, lwk_c_domain_integer, &elements);

    /*
    ** If the List is empty, raise an exception.
    */

    if (elements <= 0)
	_Raise(list_empty);

    /*
    **  Create the Origin and Destination operations Lists
    */

    origin_opr_ids_list =
	(_List) _CreateList(_TypeList, lwk_c_domain_list, elements);

    origin_opr_names_list =
	(_List) _CreateList(_TypeList, lwk_c_domain_list, elements);

    dest_opr_ids_list =
	(_List) _CreateList(_TypeList, lwk_c_domain_list, elements);

    dest_opr_names_list =
	(_List) _CreateList(_TypeList, lwk_c_domain_list, elements);

    /*
    **	Fill in those Lists from the Origins and Destinations of the given
    **	Steps.
    */

    steps_list = (_List) _Copy(steps, _False);

    for (i = 0; i < elements; i++) {
	/*
	**  Get the next Step
	*/

	_RemoveElement(steps_list, lwk_c_domain_step, &step);

	/*
	**  Build the operations List for the Origin of the Step
	*/

	_GetValue(step, _P_Origin, lwk_c_domain_surrogate, &surrogate);

	_GetValue(surrogate, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &type);

	op_cnt = LwkRegTypeOperations(type, &supported_operations);

	opr_ids_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_string, op_cnt);

	opr_names_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_ddif_string, op_cnt);

	if (op_cnt > 0) {
	    for (j = 0; j < op_cnt; j++) {
		_AddElement(opr_ids_list, lwk_c_domain_string,
		    (_AnyPtr) &supported_operations[j], _False);

		opr_name = LwkRegOperationName(type, supported_operations[j]);

		_AddElement(opr_names_list, lwk_c_domain_ddif_string,
		    &opr_name, _False);
	    }

	    _FreeMem(supported_operations);
	    op_cnt = 0;
	}

	_DeleteString(&type);

	_AddElement(origin_opr_ids_list, lwk_c_domain_list,
	    (_AnyPtr) &opr_ids_list, _False);

	opr_ids_list = (_List) _NullObject;

	_AddElement(origin_opr_names_list, lwk_c_domain_list,
	    (_AnyPtr) &opr_names_list, _False);

	opr_names_list = (_List) _NullObject;

	/*
	**  Build the operations List for the Destination of the Step
	*/

	_GetValue(step, _P_Destination, lwk_c_domain_surrogate, &surrogate);

	_GetValue(surrogate, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &type);

	op_cnt = LwkRegTypeOperations(type, &supported_operations);

	opr_ids_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_string, op_cnt);

	opr_names_list =
	    (_List) _CreateList(_TypeList, lwk_c_domain_ddif_string, op_cnt);

	if (op_cnt > 0) {
	    for (j = 0; j < op_cnt; j++) {
		_AddElement(opr_ids_list, lwk_c_domain_string,
		    (_AnyPtr) &supported_operations[j], _False);

		opr_name = LwkRegOperationName(type, supported_operations[j]);

		_AddElement(opr_names_list, lwk_c_domain_ddif_string,
		    &opr_name, _False);
	    }

	    _FreeMem(supported_operations);
	    op_cnt = 0;
	}

	_DeleteString(&type);

	_AddElement(dest_opr_ids_list, lwk_c_domain_list,
	    (_AnyPtr) &opr_ids_list, _False);

	opr_ids_list = (_List) _NullObject;

	_AddElement(dest_opr_names_list, lwk_c_domain_list,
	    (_AnyPtr) &opr_names_list, _False);

	opr_names_list = (_List) _NullObject;
    }

    /*
    **  Delete the Steps List
    */

    _Delete(&steps_list);

    /*
    ** If any exceptions are raised, delete the Lists and reraise the exception
    */

    _Exceptions
	_WhenOthers
	    if (op_cnt > 0)
		_FreeMem(supported_operations);

	    _DeleteString(&type);
	    _Delete(&steps_list);
	    _Delete(&opr_ids_list);
	    _Delete(&opr_names_list);
	    _Delete(&origin_opr_ids_list);
	    _Delete(&origin_opr_names_list);
	    _Delete(&dest_opr_ids_list);
	    _Delete(&dest_opr_names_list);

	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the Lists we created
    */

    *origin_opr_ids = origin_opr_ids_list;
    *origin_opr_names = origin_opr_names_list;

    *dest_opr_ids = dest_opr_ids_list;
    *dest_opr_names = dest_opr_names_list;

    return;
    }


static _Step  CreateStep(origin, destination, follow_type, operation)
_Surrogate origin;
 _Surrogate destination;

    _FollowType follow_type;
 _String operation;

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
    _Step volatile step;
    _Date volatile date;
    _DDIFString volatile ddifstring;

    /*
    **  Initialize
    */

    step = (_Step) _NullObject;
    date = (_Date) _NullObject;
    ddifstring = (_DDIFString) _NullObject;

    _StartExceptionBlock

    /*
    **  Create the new Step.
    */

    step = (_Step) _Create(_TypeStep);

    /*
    **  Set its Origin, Destination, FollowType and Operation
    */

    _SetValue(step, _P_Origin, lwk_c_domain_surrogate, &origin,
	lwk_c_set_property);

    _SetValue(step, _P_Destination, lwk_c_domain_surrogate, &destination,
	lwk_c_set_property);

    _SetValue(step, _P_FollowType, lwk_c_domain_integer, &follow_type,
	lwk_c_set_property);

    _SetValue(step, _P_DestinationOperation, lwk_c_domain_string, &operation,
	lwk_c_set_property);

    /*
    **  Default the Author to the current user
    */

    ddifstring = _UserNameToDDIFString;
    _SetValue(step, _P_Author, lwk_c_domain_ddif_string, (_AnyPtr) &ddifstring,
	lwk_c_set_property);

    _DeleteDDIFString(&ddifstring);

    /*
    **  Default the Creation Date to Now
    */

    date = _NowToDate;
    _SetValue(step, _P_CreationDate, lwk_c_domain_date, (_AnyPtr) &date,
	lwk_c_set_property);

    _DeleteDate(&date);

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteDDIFString(&ddifstring);
	    _DeleteDate(&date);
	    _Delete(&step);
	    _Reraise;
    _EndExceptionBlock

    return step;
    }


static void  FindNextStep(ui, origin, destination, operation, follow_type)
_Ui ui;
 _Surrogate *origin;
 _Surrogate *destination;

    _String *operation;
 _FollowType *follow_type;

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
    _List list;
    _Step step;
    _Path path;
    _Object null_object = _NullObject;
    _Boolean new_path;
    _Step previous_step;
    _Path previous_path;
    _CompPath cpath;
    _Termination termination;
    _NextPathContext context;
    _Integer type;

    /*
    ** Try the Next Step of the Followed Step.  If none, try the First Step in
    ** the next Path in the Active Composite Path.
    **
    ** Note: There are side effects caused by _GetValue/_SetValue of Currency
    ** -- that is why we use them here.
    */

    new_path = _False;
    step = (_Step) _NullObject;
    path = (_Path) _NullObject;

    _GetValue(ui, _P_FollowedStep, lwk_c_domain_step, &previous_step);

    if (previous_step != (_Step) _NullObject)
	_GetValue(previous_step, _P_NextStep, lwk_c_domain_step, &step);

    if (step == (_Step) _NullObject) {
        /*
	** If we are past the followed step, we must first enter at the Origin
	** of the next Path, then Step Forward from there.  We need to
	** distinguish these two case.  So, when we enter the new Path, we
	** change the active path but leave the followed step alone.  The next
	** Step Forward steps to the Destination of the First Step on that
	** Path.
        */
	
	if (previous_step == (_Step) _NullObject)
	    previous_path = (_Path) _NullObject;
	else
	    _GetValue(previous_step, _P_Path, lwk_c_domain_path,
		&previous_path);

	_GetValue(ui, _P_ActivePath, lwk_c_domain_path, &path);

	if (path != previous_path) {
            /*
	    ** Previous Step Forward entered at the Origin of the active path,
	    ** so now we can Step to the Destination of the First Step.
            */
	
	    _GetValue(path, _P_FirstStep, lwk_c_domain_step, &step);
	}
	else {
	    /*
	    ** Get Active Composite Path
	    */
	
	    _GetValue(ui, _P_ActiveCompPath, lwk_c_domain_comp_path,
		&cpath);

	    if (cpath == (_CompPath) _NullObject)
		_Raise(no_active_cpath);                          

	    /*
	    ** Find the First Step in the next, non-empty Path of the Active
	    ** Composite Path
	    */
	
	    new_path = _True;

	    context.path = path;
	    context.count = 0;
	    _GetValue(ui, _P_ActivePathIndex, lwk_c_domain_integer,
		&context.index);

	    while (_True) {
		termination = (_Termination) _Iterate(cpath, lwk_c_domain_path,
		    (_Closure) &context, FindNextPath);

		/*
		** Did we reach the end of the path list in the composite?
		*/

		if (termination == (_Termination) 0)
		    _Raise(no_more_steps);

		_SetValue(ui, _P_ActivePathIndex, lwk_c_domain_integer,
		    &(context.index), lwk_c_set_property);

		path = context.path;
		_GetValue(path, _P_FirstStep, lwk_c_domain_step, &step);

		if (step != (_Step) _NullObject) 
		    break;
		else
		    context.count = 0;
	    }
	    
            /*
	    ** We have a new path and a new step, so clear the followed step
	    */
	    
	    _SetValue(ui, _P_FollowedStep, lwk_c_domain_step, &null_object,
		lwk_c_set_property);
	}
    }

    if (new_path) {
    
	/*
	** Make this Path the active path.
	*/

	_SetValue(ui, _P_ActivePath, lwk_c_domain_path, &path,
	    lwk_c_set_property);

        /*
	** When stepping into the Origin of a new Path, Visit the Origin of the
	** Path from the Destination of the Previous Step using the Origin
	** Operation of the Path (or the Default Operation if null).
        */
	
	if (previous_step == (_Step) _NullObject)
	    *origin = (_Surrogate) _NullObject;
	else
	    _GetValue(previous_step, _P_Destination, lwk_c_domain_surrogate,
		origin);

	_GetValue(step, _P_Origin, lwk_c_domain_surrogate, destination);

	_GetValue(path, _P_OriginOperation, lwk_c_domain_string, operation);

	if (*operation == (_String) _NullObject)
	    _GetValue(ui, _P_DefaultOperation, lwk_c_domain_string, operation);

	*follow_type = lwk_c_follow_visit;
    }
    else {

	/*
	** Make this Step the followed step. 
	*/

	_SetValue(ui, _P_FollowedStep, lwk_c_domain_step, &step,
	    lwk_c_set_property);
	    
        /*
	** When stepping along active path, get the parameters from the
	** followed step.
        */
	
	_GetValue(step, _P_Origin, lwk_c_domain_surrogate, origin);
	_GetValue(step, _P_Destination, lwk_c_domain_surrogate, destination);
	_GetValue(step, _P_DestinationOperation, lwk_c_domain_string, operation);
	_GetValue(step, _P_FollowType, lwk_c_domain_integer, &type);
	*follow_type = (_FollowType) type;
    }

    return;
    }


static void  GetClientOriginAndDestination(ui, client_address, origin, path, destination)
_Ui ui;
 _Integer client_address;

    _Surrogate *origin;
 _Surrogate *path;
 _Surrogate *destination;

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
    _ClientEntry client;

    /*
    ** Look for the entry for this Client
    */

    client = (_ClientEntry) _ClientList_of(ui);

    while (client != (_ClientEntry) 0) {
	if (client->address == client_address)
	    break;

	client = client->next;
    }

    /*
    ** Return the Origin and Destination -- null if we have no information
    ** about this Client
    */

    if (client == (_ClientEntry) 0) {
	*origin = (_Surrogate) _NullObject;
	*path = (_Surrogate) _NullObject;
	*destination = (_Surrogate) _NullObject;
    }
    else {
	*origin = client->origin;
	*path = client->recording_path;
	*destination = client->destination;
    }

    return;
    }


static void  SaveClientOriginAndDestination(ui, client_address, origin, path, destination)
_Ui ui;
 _Integer client_address;

    _Surrogate origin;
 _Surrogate path;
 _Surrogate destination;

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
    _ClientEntry client;

    /*
    ** Allocate a new Client Entry, fill in its fields, and add it to the
    ** beginning of the Client list.
    */

    client = (_ClientEntry) _AllocateMem(sizeof(_ClientInstance));

    client->address = client_address;
    client->origin = origin;
    client->recording_path = path;
    client->destination = destination;

    client->next = (_ClientEntry) _ClientList_of(ui);
    _ClientList_of(ui) = (_AnyPtr) client;

    return;
    }


static _ClientEntry  FindClientByOrigin(ui, origin)
_Ui ui;
 _Surrogate origin;

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
    _ClientEntry client;

    /*
    ** Look for the entry for this Client
    */

    client = (_ClientEntry) _ClientList_of(ui);

    while (client != (_ClientEntry) 0) {
	if (client->origin == origin)
	    break;

	client = client->next;
    }

    return client;
    }


static void  DeleteClientEntry(ui, client)
_Ui ui;
 _ClientEntry client;

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
    _ClientEntry entry;
    _ClientEntry previous;

    /*
    ** Remove the entry from the Client List
    */

    entry = (_ClientEntry) _ClientList_of(ui);
    previous = (_ClientEntry) 0;

    while (entry != (_ClientEntry) 0) {
	if (entry == client) {
	    if (previous == (_ClientEntry) 0)
		_ClientList_of(ui) = (_AnyPtr) entry->next;
	    else
		previous->next = entry->next;

	    _FreeMem(entry);

	    break;
	}

	previous = entry;
	entry = entry->next;
    }

    return;
    }


static void  DeleteClientByAddress(ui, client_address)
_Ui ui;
 _Integer client_address;

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
    _ClientEntry temp;
    _ClientEntry client;
    _ClientEntry previous;

    /*
    ** Remove all entries for this Client from the Client List
    */

    client = (_ClientEntry) _ClientList_of(ui);
    previous = (_ClientEntry) 0;

    while (client != (_ClientEntry) 0) {
	if (client->address == client_address) {
	    if (previous == (_ClientEntry) 0)
		_ClientList_of(ui) = (_AnyPtr) client->next;
	    else
		previous->next = client->next;

	    temp = client;
	    client = client->next;

	    _FreeMem(temp);
	}
	else {
	    previous = client;
	    client = client->next;
	}
    }

    /*
    ** If the Client was the Destination of the last Step Forward, clear the
    ** Step Destination.
    */

    if (client_address == _StepDestination_of(ui))
	_StepDestination_of(ui) = _UnknownDestinationClientAddress;

    return;
    }


static void  FreeClientList(ui)
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
    _ClientEntry next;
    _ClientEntry client;

    /*
    ** Remove all the Clients from the Client list
    */

    next = (_ClientEntry) _ClientList_of(ui);

    while (next != (_ClientEntry) 0) {
	client = next;
	next = client->next;

	_FreeMem(client);
    }

    _ClientList_of(ui) = (_AnyPtr) 0;

    return;
    }


static _Termination  FindNextPath(context, cpath, domain, path)
_NextPathContext *context;

    _CompPath cpath;
 _Domain domain;
 _Path *path;

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

    context->count++;
    
    /*
    **	When count is greater than lwk_c_env_active_path_index, we have the next
    **	path
    */

    if (context->count > context->index) {
	context->path = *path;
	context->index++;
	return (_Termination) 1;
    }

    return (_Termination) 0;
    }


static _Termination  AnotherString(array, set, domain, string)
_String *array;
 _Set set;
 _Domain domain;

    _String *string;

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
    int i;

    /*
    **  Find the next empty array slot
    */

    i = 0;

    while (array[i] != (_String) _NullObject)
	i++;

    /*
    **  Fill the array slot with this String
    */

    array[i] = *string;

    /*
    **  Continue iterating
    */

    return (_Termination) 0;
    }


static _Termination  FindSurrogateSubType(sought, list, domain, type)
_String sought;
 _List list;

    _Domain domain;
 _String *type;

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
    **	If the Surrogate subtype equals the sought after type, or one of its
    **	supertypes, return 1, else return 0.
    */

    if (LwkRegIsSuperType(sought, *type))
	return (_Termination) 1;
    else
	return (_Termination) 0;
    }


static _Termination  FindOperation(sought, list, domain, operation)
_String sought;
 _List list;
 _Domain domain;

    _String *operation;

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
    **	If the Operation equals the given sought after Operation, return 1,
    **	else return 0.
    */

    if (_CompareString(sought, *operation) == 0)
	return (_Termination) 1;
    else
	return (_Termination) 0;
    }


static _Integer  GetClientAddress(ui)
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
    _Integer address;

    /*
    **  We must have a Currency object registered to do this.
    */

    if (_EnvironmentState_of(ui) == (_DXmEnvState) _NullObject)
	return _ThisClientAddress;

    /*
    **  Get the address from the Currency Manager
    */

    _GetValue(_EnvironmentState_of(ui), _P_Address, lwk_c_domain_integer, &address);

    return address;
    }


static _Integer  GetApplyAddress(ui, property)
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
    _Integer address;

    /*
    ** Try to get the address associated with the Registered application.  If
    ** that fails, try to get the address of the Environment Manager.  We
    ** validate the whichever address before returning it.
    */

    _StartExceptionBlock

    _GetValue(_EnvironmentState_of(ui), property, lwk_c_domain_integer, &address);

    /*             
    ** Validate the address by sending a negative length message to it
    */

    _SendMessage(_EnvironmentState_of(ui), address, -1, (_AnyPtr) 0);

    _Exceptions
	_WhenOthers
	    if (_EnvironmentManager_of(ui))
		_Reraise;
	    else {
		_GetValue(_EnvironmentState_of(ui), _P_EnvironmentManagerAddress,
		    lwk_c_domain_integer, &address);

		_SendMessage(_EnvironmentState_of(ui), address, -1, (_AnyPtr) 0);
	    }
    _EndExceptionBlock

    return address;
    }


static _Integer  GetSurrogateClientAddress(ui, surrogate)
_Ui ui;
 _Surrogate surrogate;

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
    _Integer client_address;
    _String default_operation;
    _String type;
    _String subtype;
    _String property;

    /*
    **  Initialize
    */

    default_operation = GetDefaultOperation(surrogate); 
    type = (_String) _NullObject;
    subtype = (_String) _NullObject;
    property = (_String) _NullObject;

    /*
    **  Generate the name of the property which should have been
    **	registered by an application which can perform the default
    **	operation on a Surrogate of the given type.
    */

    _GetValue(surrogate, _P_SurrogateSubType, lwk_c_domain_string, &subtype);

    type = LwkRegOperationType(subtype, default_operation);

    property = _CopyString(_CurrencyPropertyPrefix);
    property = _ConcatString(property, type);
    property = _ConcatString(property, _CurrencyPropertyDelimiter);
    property = _ConcatString(property, default_operation);

    /*
    **	Find the appropriate Client.
    */

    client_address = GetApplyAddress(ui, property);

    /*
    **  Delete the Strings which were generated
    */

    _DeleteString(&default_operation);
    _DeleteString(&type);
    _DeleteString(&subtype);
    _DeleteString(&property);

    return(client_address);
    }


static _String  GetDefaultOperation(surrogate)
_Surrogate surrogate;

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
    _String type;
    _String operation;

    /*
    ** Ask the registry what the default operation is for this Surrogate type.
    */

    _GetValue(surrogate, _P_SurrogateSubType, lwk_c_domain_string, &type);

    operation = LwkRegTypeDefaultOperation(type);

    _DeleteString(&type);

    return operation;
    }


static _Integer  CheckSumSurrogate(surrogate)
_Surrogate surrogate;

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
    _String string;
    _Integer checksum;
    _Linkbase linkbase;


    if (surrogate == (_Surrogate) _NullObject)
	checksum = 0;
    else {
	/*
	** The checksum of a Surrogate is its Identifier XOR'd with the
	** checksum of its Linkbase's Identifier.
	*/

	_GetValue(surrogate, _P_Identifier, lwk_c_domain_integer, &checksum);

	_GetValue(surrogate, _P_Linkbase, lwk_c_domain_linkbase, &linkbase);

	if (linkbase != (_Linkbase) _NullObject) {
	    _GetValue(linkbase, _P_Identifier, lwk_c_domain_string, &string);

	    checksum ^= CheckSumString(string);

	    _DeleteString(&string);
	}
    }
    
    return checksum;
    }


static _Integer  CheckSumString(string)
_String string;

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
    int i;
    _Integer checksum;

    /*
    ** The checksum of a String is the sum of the values of each character in
    ** the string.
    */

    i = 0;
    checksum = 0;

    if (string != (_String) _NullObject)
	while (string[i] != _EndOfString) {
	    checksum += string[i];
	    i++;
	}

    return checksum;
    }



static void  SavePendingLink(ui)
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
    ** Update any pending Link with the Recording Linknet.
    */
    
    UpdatePendingLink(ui, lwk_c_env_recording_linknet);

    return;
    }


static void  DisplayWIP(ui, surrogate)
_Ui ui;
 _Surrogate surrogate;

{
    _String string;
    /*
    ** Display the WIP box and add it to the list of
    ** WIP structures.
    */

    _GetValue(surrogate, _P_SurrogateSubType, lwk_c_domain_string,
		&string);

    LwkOpDXmUiDisplayWIP(ui, string);
    
    _DeleteString(&string);
}


static void  TriggerRemoveWIP(ui, surrogate)
_Ui ui;
 _Surrogate surrogate;

{
    _String string;

    /*									  
    ** Trigger the event which will cause the right application to
    ** remove the WIP box.
    */

    _GetValue(surrogate, _P_SurrogateSubType, lwk_c_domain_string,
		&string);

    _SetValue(_EnvironmentState_of(ui), _P_WIPProperty,
	lwk_c_domain_string, &string, lwk_c_set_property);

    _DeleteString(&string);
    
}


static void  RemoveWIP(ui, string)
_Ui ui;
 _String string;

{
    /* 
    ** Look at the WIP structures for a match in property
    ** name.  If there is a match, unmanage the associated
    ** WIP box and remove the structure from the list.
    */

    LwkOpDXmUiRemoveWIP(ui, string);

}

