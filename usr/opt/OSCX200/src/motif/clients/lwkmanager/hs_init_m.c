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
** COPYRIGHT (c) 1990, 1991 BY
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
**	LinkWorks Manager Initialization
**
**  Version: V1.0
**
**  Abstract:
**	LinkWorks Manager Initialization routine 
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Andre Pavanello
**
**  Creation Date: 11-Dec-90
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

/*
**  Macro Definitions
*/

#define _DisplayRootDialog(yes_cb, no_cb, identifier) \
    EnvDWRootDialogBox((yes_cb), (no_cb), (identifier))
_DeclareFunction(_Void EnvDWRootDialogBox,
    (_Callback yes_callback, _Callback no_callback, _String identifier));

/*
**  Type Definitions
*/

typedef struct __InitData {
	Widget		    toplevel;
	_Boolean	    clear_currency;
	_EnvContext	    env_context;
	lwk_linkbase	    linkbase;
	lwk_dxm_ui	    dwui;
	lwk_composite_linknet   cnet;
	lwk_linknet	    network;
	lwk_composite_path  cpath;
	lwk_ddif_string	    relation;
	lwk_integer	    highlight;
	lwk_string	    operation;
	lwk_boolean	    retain_src;
	
	} _InitDataInstance, *_InitData;

/*
**  Static Data Definitions
*/

static _Boolean TriggerUi = _True;

/*
**  Global Data Definitions
*/

/*
**  Table of Contents
*/

_DeclareFunction(static _Void EnvInitEnvironment, (_InitData init_data));
_DeclareFunction(static _Void SetCurrency,
    (_InitData init_data, _CurrencyFlag currency, _AnyPtr value));
_DeclareFunction(static _Void CreateDwUiObject, (_InitData init_data));
_DeclareFunction(static _Void LoadNetworkData, (_InitData init_data));
_DeclareFunction(static lwk_termination GetPersistent,
    (lwk_persistent *persistent, lwk_object cnet, lwk_domain domain,
    lwk_object_descriptor *object_desc));
_DeclareFunction(static _Void LoadPathData, (_InitData init_data));
_DeclareFunction(static _Void LoadAttributesData, (_InitData init_data));
_DeclareFunction(static _Void EnvInitLBVersionYes,
    (Widget w, _WindowPrivate null_tag, _Reason reason));

/*
** External Functions Declaration
*/

_DeclareFunction(_Void EnvDwMessageDismissRootDb,());


_Void  EnvInit()
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
    

_Void  EnvInitRetrieveEnv(create_lb)
_Boolean create_lb;

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
    _String	    value;
    _Boolean	    new_lb;
    _InitData	    init_data;
    _Status	    status[2];
    lwk_linkbase    linkbase;
    XtAppContext    app_context;
    Widget	    toplevel;

    new_lb = _False;
    
    /*
    ** We need to keep the toplevel here because an exception can be raised in
    ** the allocate routine
    */

    toplevel = EnvDwMessageGetToplevel();

    _StartExceptionBlock

    init_data = (_InitData) _AllocateMem(sizeof(_InitDataInstance));
    _ClearMem(init_data, sizeof(_InitDataInstance));

    init_data->toplevel = (Widget) toplevel;

    /*
    ** If this symbols is set, the initialization application will clear all the
    ** HIS properties on the root window. Very useful for debug!
    ** 
    */

    value = (char *) getenv("HS_CLEAR_CURRENCY");

    if (value != NULL)
	init_data->clear_currency = _True;

    if (!init_data->clear_currency)
	init_data->env_context = (_EnvContext) _Retrieve(_TypeEnvContext,
	    &new_lb, create_lb);

    if ((init_data->env_context != (_EnvContext) _NullObject) ||
	(init_data->clear_currency)) {
	
	/*
	** Initialize the environment
	*/

	EnvInitEnvironment(init_data);

	/*
	** If a new linkbase was created, announce it
	*/

	if ((new_lb) && (!create_lb)) {

	    /*
	    ** Load the message block
	    */

	    status[0] = hs_s_lb_not_exist;
	    status[1] = hs_s_new_lb;

	    /*
	    ** Note: we need to unrealize the toplevel - realized when the the DWUI
	    ** object was created - otherwise the message box won't appear
	    */
	    
	    XtUnrealizeWidget(init_data->toplevel);
	    _DisplayRootMessage(status, 2, hs_c_fatal_message);
	}
	else
	    TriggerUi = _False;
    }

    /*
    **  If any exceptions, load the message block
    */

    _Exceptions
	_When(version_error)
	    if (init_data != (_InitData) 0)
		_FreeMem(init_data);
		
	    XtUnrealizeWidget(toplevel);
	    _DisplayRootDialog(EnvInitLBVersionYes, (_Callback) 0,
		_LbNewVersionMsg);

        _WhenOthers
	    status[0] = _StatusCode(retrieve_error);
	    status[1] = _Others;

	    XtUnrealizeWidget(toplevel);
	    _DisplayRootMessage(status, 2, hs_c_fatal_message);

    _EndExceptionBlock

    if (TriggerUi) {
    
	app_context = XtWidgetToApplicationContext(toplevel);
	XtAppMainLoop(app_context);
    }
    else
	exit(TerminationSuccess);

    return;
    }
    

static _Void  EnvInitEnvironment(init_data)
_InitData init_data;

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
    lwk_object		null_object;
    lwk_status		status;

    null_object = lwk_c_null_object;

    /*
    ** We need an HIS DWUI object
    */

    CreateDwUiObject(init_data);

    /*
    ** Just clear all the properties on the root window if flag is set
    */

    if (init_data->clear_currency) {
    
	SetCurrency(init_data, lwk_c_env_active_comp_linknet, &null_object);

	SetCurrency(init_data, lwk_c_env_recording_linknet, &null_object);

	SetCurrency(init_data, lwk_c_env_active_comp_path, &null_object);

	SetCurrency(init_data, lwk_c_env_default_relationship, &null_object);
		
	SetCurrency(init_data, lwk_c_env_default_highlight, &null_object);
		
	SetCurrency(init_data, lwk_c_env_default_operation, &null_object);
		
	SetCurrency(init_data, lwk_c_env_default_retain_source, &null_object);
    }
    else {

	LoadNetworkData(init_data);
	LoadPathData(init_data);
	LoadAttributesData(init_data);

	/*
	** Set all the currencies needed for the environment
	*/

	SetCurrency(init_data, lwk_c_env_active_comp_linknet, &(init_data->cnet));

	SetCurrency(init_data, lwk_c_env_recording_linknet, (_AnyPtr)
	    &(init_data->network));

	/*
	**  Set the active path list as the current composite path
	*/

	SetCurrency(init_data, lwk_c_env_active_comp_path,
	    &(init_data->cpath));

	/*
	** Set all the atributes currencies
	*/
	
	SetCurrency(init_data, lwk_c_env_default_relationship,
	    &(init_data->relation));
		
	SetCurrency(init_data, lwk_c_env_default_highlight, &(init_data->highlight));
		
	SetCurrency(init_data, lwk_c_env_default_operation, &(init_data->operation));
		
	SetCurrency(init_data, lwk_c_env_default_retain_source,
	    &(init_data->retain_src));
    }

    /*
    ** Clear all the remaining currencies
    */
    
    SetCurrency(init_data, lwk_c_env_appl_highlight, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_recording_path, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_active_path, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_active_path_index, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_followed_step, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_new_link, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_pending_source, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_pending_target, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_followed_link, (_AnyPtr) &null_object);

    SetCurrency(init_data, lwk_c_env_follow_destination, (_AnyPtr) &null_object);

    /*
    ** Delete the HIS ui object
    */

    status = lwk_delete(&init_data->dwui);

    return;
    }
    

static _Void  SetCurrency(init_data, currency, value)
_InitData init_data;
 _CurrencyFlag currency;

    _AnyPtr value;

    {
    lwk_status	    status;
    lwk_object	    null_object;

    /*
    **  Set the value to be the current of that type
    */

    switch (currency) {

	case lwk_c_env_appl_highlight :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_appl_highlight, lwk_c_domain_integer, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_active_comp_linknet :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_active_comp_linknet, 	lwk_c_domain_comp_linknet,
		value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_recording_linknet :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_recording_linknet,
		lwk_c_domain_linknet, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_active_comp_path :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_active_comp_path, lwk_c_domain_comp_path,
		value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }

            /*
	    ** Clear the current path and the current step.
	    */
	    
	    null_object = lwk_c_null_object;

	    SetCurrency(init_data, lwk_c_env_active_path, (_AnyPtr) &null_object);

	    SetCurrency(init_data, lwk_c_env_active_path_index,
		(_AnyPtr) &null_object);

	    SetCurrency(init_data, lwk_c_env_followed_step, (_AnyPtr) &null_object);
	    
	    break;

	case lwk_c_env_active_path :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_active_path,
		lwk_c_domain_path, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_active_path_index :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_active_path_index,
		lwk_c_domain_integer, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_recording_path :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_recording_path,
		lwk_c_domain_path, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_followed_step :

	    status = lwk_set_value(init_data->dwui, lwk_c_p_followed_step,
		lwk_c_domain_step, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_new_link :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_new_link, lwk_c_domain_link, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_pending_source :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_pending_source,
		lwk_c_domain_surrogate, value, 	lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_pending_target :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_pending_target,
		lwk_c_domain_surrogate, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_followed_link :

	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_followed_link, lwk_c_domain_link, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_follow_destination :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_follow_destination, lwk_c_domain_surrogate, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_default_operation :
	
	    status = lwk_set_value(init_data->dwui, lwk_c_p_default_operation,
		lwk_c_domain_string, value, lwk_c_set_property );
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_default_highlight :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_default_highlight, lwk_c_domain_integer, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_default_relationship :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_default_relationship, lwk_c_domain_ddif_string,
		value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }

	    break;

	case lwk_c_env_default_retain_source :
	
	    status = lwk_set_value(init_data->dwui,
		lwk_c_p_default_retain_source, lwk_c_domain_boolean, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	default:
	    _Raise(inv_currency);
	    break;
    }

    return;
    }


static _Void  CreateDwUiObject(init_data)
_InitData init_data;

    {
    lwk_status	status;
    lwk_boolean	env_manager;
    Arg		arglist[2];

    XtSetArg(arglist[0], XmNwidth, (Dimension) 10);
    XtSetArg(arglist[1], XmNheight, (Dimension) 10);
    XtSetValues(init_data->toplevel, arglist, 2);
    
    XtSetMappedWhenManaged(init_data->toplevel, FALSE);

    XtRealizeWidget(init_data->toplevel);

    status = lwk_create_dxm_ui((lwk_any_pointer) 0, lwk_c_false,
	lwk_c_false, init_data->toplevel, (lwk_any_pointer) 0,
	&(init_data->dwui));

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(dwui_creation_failed);
    }

    return;
    }
    

static _Void  LoadNetworkData(init_data)
_InitData init_data;


    {
    _HsObject		hs_obj;
    lwk_composite_linknet	cnet;
    lwk_linknet		network;
    lwk_status		status;
    lwk_termination	termination;
    
    /*
    **  Get the active network list for the current composite network
    */

    _GetValue(init_data->env_context, _P_ActiveNetworks, hs_c_domain_hsobject,
	&hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &(init_data->cnet));

    /*
    **  Get the current network in the correponding cnet
    */

    _GetValue(init_data->env_context, _P_CurrentNetwork, hs_c_domain_hsobject,
	&hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &cnet);

    network = lwk_c_null_object;

    status = lwk_iterate(cnet, lwk_c_domain_object_desc,
	(lwk_closure) &network, (lwk_callback) GetPersistent, &termination);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(iterate_failed);   /* iterate on cnet failed */
    }

    init_data->network = network;

    return;
    }


static lwk_termination  GetPersistent(persistent, cnet, domain, object_desc)
lwk_persistent *persistent;

    lwk_object cnet;
 lwk_domain domain;
 lwk_object_descriptor *object_desc;


    {
    lwk_status	    status;

    status = lwk_retrieve(*object_desc, persistent);

    /*
    **  If the retrieve fails it means that the current network is not
    **	available and it won't be set current. No special action is needed
    */

    return (_True);
    }


static _Void  LoadPathData(init_data)
_InitData init_data;

    {
    _HsObject	hs_obj;
    lwk_status	status;
    lwk_list	list;
    lwk_integer count;

    _GetValue(init_data->env_context, _P_ActivePaths, hs_c_domain_hsobject,
	&hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &(init_data->cpath));

    /*
    **	If there are no elements in the composite path don't load one
    **	so the Step Forward menu in the connection menu is dimed
    */

    status = lwk_get_value(init_data->cpath, lwk_c_p_paths, lwk_c_domain_list,
	&list);

    if (list != lwk_c_null_object) {

	status = lwk_get_value(list, lwk_c_p_element_count,
	    lwk_c_domain_integer, &count);
	
	if (count == 0)
	    init_data->cpath = lwk_c_null_object;
    }
    else
	init_data->cpath = lwk_c_null_object;

    return;
    }
    

static _Void  LoadAttributesData(init_data)
_InitData init_data;

    {

    _GetAttribute(init_data->env_context, _HighlightingAttr,
	&(init_data->highlight));

    _GetAttribute(init_data->env_context, _ConnectionTypeAttr,
	&(init_data->relation));

    _GetAttribute(init_data->env_context, _RetainSourceAttr,
	&(init_data->retain_src));

    _GetAttribute(init_data->env_context, _OperationAttr,
	&(init_data->operation));

    return;
    }
    

static _Void  EnvInitLBVersionYes(w, null_tag, reason)
Widget w;
 _WindowPrivate null_tag;

    _Reason reason;

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

    EnvDwMessageDismissRootDb();

    /*
    ** The toplevel widget has been stored in the previous call to this routine
    */    

    EnvInitRetrieveEnv(_True);

    return;
    }
    
