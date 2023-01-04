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
**  Facility:
**
**	LinkWorks Manager
**
**  Version:  V1.0
**
**  Abstract:
**
**	Customize support routines
**
**  Keywords:
**	Customize
**
**  Author:
**      Andre Pavanello, MEMEX Project
**
**  Creation Date:  22-Mar-90
**
**  Modification History:
**--
*/


/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_object.h"
#include "hs_decwindows.h"

/*
**  Type Definitions
*/

typedef struct __CustEnvPrivate {

	_Boolean    settings_changed;
	Widget	    box_widget;
	Widget	    highl_tgle;
	Widget	    highl_src_tgle;
	Widget	    highl_trg_tgle;
	Widget	    highl_orph_tgle;
	Widget	    highl_cur_src_tgle;
	Widget	    highl_cur_trg_tgle;
	Widget	    highl_cur_dest_tgle;
	Widget	    relation_cstxt;
	Widget	    ret_src_tgle;
	Widget	    select_dst_tgle;
	Widget	    oper_option;
	Widget	    oper_def_but;
	Widget	    oper_but1;
	Widget	    oper_but2;
	Widget	    oper_but3;
	int	    op_index;
	Widget      ok_button;
	
    } _CustEnvPrivateInstance, *_CustEnvPrivate;

typedef struct __CustHypPrivate {

	Widget	    box_widget;
	Widget	    window_tgle;
	Widget	    icon_tgle;
	Widget	    ok_button;
	
    } _CustHypPrivateInstance, *_CustHypPrivate;

/*
**  Table of Contents
*/

_DeclareFunction(_Void EnvCustEnvInitialize, ());
_DeclareFunction(static _Void CreateEnvBox,
    (_WindowPrivate private));

_DeclareFunction(static _Void ResetEnvActionButtonsState,
    (_CustEnvPrivate env));
_DeclareFunction(static _Void SetEnvironmentBox,
    (_WindowPrivate private));
_DeclareFunction(static _Void LoadHighlightingPart,
    (_EnvContext env_context,_CustEnvPrivate env));
_DeclareFunction(static _Void LoadConnectPart,
    (_EnvContext env_context,_CustEnvPrivate env));
_DeclareFunction(static _Void LoadFollowPart,
    (_EnvContext env_context,_CustEnvPrivate env));
_DeclareFunction(static _Void SetHighlightingPart,
    (_EnvContext env_context,_CustEnvPrivate env));
_DeclareFunction(static _Void SetConnectPart,
    (_EnvContext env_context,_CustEnvPrivate env));
_DeclareFunction(static _Void SetFollowPart,
    (_EnvContext env_context,_CustEnvPrivate env));

_DeclareFunction(static _Void UilActivateEnvOk,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateEnvCancel,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvHighlight,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvHighSources,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvHighTargets,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvHighOrphans,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvCurSource,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvCurTarget,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvCurDest,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvRelation,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvRetSource,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvOperOption,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvOptDefButton,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvOptButton1,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvOptButton2,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvOptButton3,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvSelectDst,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateEnvOk,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilValueChangedEnvFields,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void CreateHypBox, (_WindowPrivate private));
_DeclareFunction(static _Void ResetHypActionButtonsState,
    (_CustHypPrivate hyp));      
_DeclareFunction(static _Void SetLinkWorksMgrBox, (_WindowPrivate private));
_DeclareFunction(static _Void UilActivateHypOk,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateHypCancel,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateHypWindowState,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateHypIconState,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateHypOk,
    (Widget w, _WindowPrivate private, _Reason reason));

/*
**  Macro Definitions
*/

#define  _CustomizeEnvironmentBox "customize_environment_box"
#define  _CustomizeEnvManagerBox  "customize_env_manager_box"

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant drm_registrations[] = {

    {"cust_env_activate_ok",	    (caddr_t) UilActivateEnvOk},
    {"cust_env_activate_cancel",    (caddr_t) UilActivateEnvCancel},

    {"cust_env_create_highl",	    (caddr_t) UilCreateEnvHighlight},
    {"cust_env_create_highl_sources",(caddr_t) UilCreateEnvHighSources},
    {"cust_env_create_highl_targets",(caddr_t) UilCreateEnvHighTargets},
    {"cust_env_create_highl_orphans",(caddr_t) UilCreateEnvHighOrphans},
    {"cust_env_create_pen_src",	    (caddr_t) UilCreateEnvCurSource},
    {"cust_env_create_pen_target",  (caddr_t) UilCreateEnvCurTarget},
    {"cust_env_create_nav_dest",    (caddr_t) UilCreateEnvCurDest},
    {"cust_env_create_relation",    (caddr_t) UilCreateEnvRelation},
    {"cust_env_create_ret_source",  (caddr_t) UilCreateEnvRetSource},
    {"cust_env_create_oper_option", (caddr_t) UilCreateEnvOperOption},
    {"cust_env_create_opt_def_button",(caddr_t) UilCreateEnvOptDefButton},
    {"cust_env_create_opt_1_button",(caddr_t) UilCreateEnvOptButton1},
    {"cust_env_create_opt_2_button",(caddr_t) UilCreateEnvOptButton2},
    {"cust_env_create_opt_3_button",(caddr_t) UilCreateEnvOptButton3},
    {"cust_env_create_selec_dst",   (caddr_t) UilCreateEnvSelectDst},
    {"cust_env_create_ok",	    (caddr_t) UilCreateEnvOk},
    {"cust_env_value_changed_fields",(caddr_t) UilValueChangedEnvFields},

    {"cust_mgr_activate_ok",	    (caddr_t) UilActivateHypOk},
    {"cust_mgr_activate_cancel",    (caddr_t) UilActivateHypCancel},
    {"cust_mgr_create_window_state",(caddr_t) UilCreateHypWindowState},
    {"cust_mgr_create_icon_state",  (caddr_t) UilCreateHypIconState},
    {"cust_mgr_create_ok",	    (caddr_t) UilCreateHypOk},
};

static MrmCount _Constant drm_registrations_size = XtNumber(drm_registrations);

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  EnvCustInitialize()
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
    **	Register the Customize callback routines so that the
    **	resource manager can resolve them at widget creation time
    */

    MrmRegisterNames(drm_registrations, drm_registrations_size);

    return;
    }


_Void  EnvCustEnvDisplayEnvironment(private)
_WindowPrivate private;

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
    _CustEnvPrivate	env;
    _EnvWindowPrivate	env_private;
    Widget		shell;
    _Boolean		recycled = _False;

    _StartExceptionBlock

    /*
    **  Has the box already been created?
    */

    env_private = (_EnvWindowPrivate) private->specific;

    if (env_private->environment == (_AnyPtr) 0) {
	CreateEnvBox(private);
	env = (_CustEnvPrivate) (env_private->environment);
	XtRealizeWidget(env->box_widget);
	EnvDWFitFormDialogOnScreen(env->box_widget);
    }
    else
	recycled = _True;
	
    /*
    **  Load the environment box with the attributes
    */

    SetEnvironmentBox(private);

    /*
    ** Reset the value changed flag which detect any changes in text widget
    ** We have to do this here because the value changed callback gets called
    ** when we load the text widgets - but nothing has really changed.
    */

    env = (_CustEnvPrivate) (env_private->environment);
    env->settings_changed = _False;
    
    /*
    **  If the box is already mapped just raise it, otherwise map it
    */

    if (XtIsManaged(env->box_widget)) {

	shell = XtParent(env->box_widget);
	XRaiseWindow(XtDisplay(shell), XtWindow(shell));
    }
    else
	XtManageChild(env->box_widget);

    if (recycled)
        /*
	** Reset the default button
	*/
	ResetEnvActionButtonsState(env);

    _Exceptions
	_WhenOthers

	    if (env_private->environment != (_AnyPtr) 0) {
		_FreeMem(env_private->environment);
		env_private->environment = (_AnyPtr) 0;
	    }
	    _Reraise;

    _EndExceptionBlock

    return;
    }


static _Void  CreateEnvBox(private)
_WindowPrivate private;

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
    _CustEnvPrivate	env;
    _EnvWindowPrivate	envprivate;
    _Integer		status;
    MrmHierarchy	hierarchy_id;
    MrmType		*class_return;

    envprivate = (_EnvWindowPrivate) private->specific;

    /*
    **	Allocate memory for private structure
    */

    env = (_CustEnvPrivate) _AllocateMem(sizeof(_CustEnvPrivateInstance));

    envprivate->environment = (_AnyPtr) env;

    /*
    **	Initialize the box
    */

    _ClearMem(env, sizeof(_CustEnvPrivateInstance));

    /*
    **	Register private data
    */
    
    EnvDWRegisterDRMNames(private);

    /*
    **	Create the environment box
    */

    hierarchy_id = EnvDwGetMainHierarchy();

    status = MrmFetchWidget(hierarchy_id, _CustomizeEnvironmentBox,
	private->main_widget, &env->box_widget, (MrmType *) &class_return);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    return;
    }


static _Void  ResetEnvActionButtonsState(env)
_CustEnvPrivate env;

/*
**++
**  Functional Description:
**
**	Reset the state of the action buttons in the customize environment box.
**	The Ok button is the default one.
**
**  Keywords:
**      
**
**--
*/
{
    if (env->ok_button != NULL) 
	XmProcessTraversal(env->ok_button, XmTRAVERSE_CURRENT);

    return;
}


static _Void  SetEnvironmentBox(private)
_WindowPrivate private;

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
    _EnvContext		env_context;
    _EnvWindowPrivate	env_private;
    _CustEnvPrivate	env;

    env_private = (_EnvWindowPrivate) private->specific;
    env = (_CustEnvPrivate) env_private->environment;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    LoadHighlightingPart(env_context, env);

    LoadConnectPart(env_context, env);

    LoadFollowPart(env_context, env);

    return;
    }


static _Void  LoadHighlightingPart(env_context, env)
_EnvContext env_context;

    _CustEnvPrivate env;

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
    _Integer	    highlighting;

    _GetAttribute(env_context, _HighlightingAttr, &highlighting);

    /*
    **  Highlight toggle
    */

    if (highlighting & lwk_c_hl_on)
	XmToggleButtonGadgetSetState (env->highl_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_tgle, _False, _False);


    /*
    **  Sources toggle
    */
    	
    if (highlighting & lwk_c_hl_sources)
	XmToggleButtonGadgetSetState (env->highl_src_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_src_tgle, _False, _False);

    /*
    **  Targets toggle
    */
    	
    if (highlighting & lwk_c_hl_targets)
	XmToggleButtonGadgetSetState (env->highl_trg_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_trg_tgle, _False, _False);

    /*
    **  Orphans toggle
    */
	
    if (highlighting & lwk_c_hl_orphans)
	XmToggleButtonGadgetSetState (env->highl_orph_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_orph_tgle, _False, _False);


    /*
    **  Current source toggle
    */
	
    if (highlighting & lwk_c_hl_pending_source)
	XmToggleButtonGadgetSetState (env->highl_cur_src_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_cur_src_tgle, _False, _False);

	
    /*
    **  Current Target toggle
    */

    if (highlighting & lwk_c_hl_pending_target)
	XmToggleButtonGadgetSetState (env->highl_cur_trg_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_cur_trg_tgle, _False, _False);

    /*
    **  Current Destination toggle
    */

    if (highlighting & lwk_c_hl_destination_of_follow)
	XmToggleButtonGadgetSetState (env->highl_cur_dest_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->highl_cur_dest_tgle, _False, _False);
	
    return;
    }


static _Void  LoadConnectPart(env_context, env)
_EnvContext env_context;
 _CustEnvPrivate env;

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
    _Boolean		    retain_attr;
    lwk_ddif_string	    relation;
    long		    len;
    long		    status;
    XmString		    xm_str;

    /*
    **  Relationship attribute
    */

    _GetAttribute(env_context, _ConnectionTypeAttr, &relation);
    
    if (relation == _NullObject) {
    
	xm_str = _StringToXmString((char *)"");
	DXmCSTextSetString (env->relation_cstxt, xm_str);
    }
    else {
    
	xm_str = (XmString) DXmCvtDDIFtoCS((Opaque) relation, &len, &status);
	DXmCSTextSetString(env->relation_cstxt, xm_str);
	lwk_delete_ddif_string(&relation);
    }

    XmStringFree(xm_str);
	
    /*
    **  Retain source toggle
    */

    _GetAttribute(env_context, _RetainSourceAttr, &retain_attr);
    XmToggleButtonGadgetSetState (env->ret_src_tgle, retain_attr, _False);

    return;
    }


static _Void  LoadFollowPart(env_context, env)
_EnvContext env_context;
 _CustEnvPrivate env;

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
    _Integer	select_dest;
    lwk_string	operation;
    Arg		arglist[1];

    _GetAttribute(env_context, _HighlightingAttr, &select_dest);

    /*
    **  Select Destination toggle
    */

    if (select_dest & lwk_c_hl_select_destination)
	XmToggleButtonGadgetSetState (env->select_dst_tgle, _True, _False);
    else
	XmToggleButtonGadgetSetState (env->select_dst_tgle, _False, _False);

    /*
    **  Default Operation setting
    */

    _GetAttribute(env_context, _OperationAttr, &operation);

    XtSetArg(arglist[0], XmNmenuHistory, env->oper_def_but);

    if (operation != lwk_c_null_object) {

	if (_CompareString(lwk_c_view_op_id, operation) == 0)
	    XtSetArg(arglist[0], XmNmenuHistory, env->oper_but1);
	    
	if (_CompareString(lwk_c_edit_op_id, operation) == 0)
	    XtSetArg(arglist[0], XmNmenuHistory, env->oper_but2);

	if (_CompareString(lwk_c_activate_op_id, operation) == 0)
	    XtSetArg(arglist[0], XmNmenuHistory, env->oper_but3);
    }

    XtSetValues(env->oper_option, arglist, 1);

    return;
    }


static _Void  UilActivateEnvOk(w, private, reason)
Widget w;
 _WindowPrivate private;
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
    _EnvWindowPrivate	env_private;
    _EnvContext		env_context;
    _CustEnvPrivate	env;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    env_private = (_EnvWindowPrivate) private->specific;
    env = (_CustEnvPrivate) env_private->environment;

    /*
    **  Get the environment context object
    */

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    /*
    **  Set the attributes that have been modified
    */

    SetHighlightingPart(env_context, env);
    SetConnectPart(env_context, env);
    SetFollowPart(env_context, env);

    if (env->settings_changed)
	_SetWindowState(private->window, _StateCustomNotSaved, _StateSet);

    /*
    **  Unmanage the box
    */

    XtUnmanageChild(((_CustEnvPrivate) (env_private->environment))->box_widget);

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(failure);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  SetHighlightingPart(env_context, env)
_EnvContext env_context;
 _CustEnvPrivate env;

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
    _Integer	    highlighting;
    _Integer	    prev_highlighting;
    Boolean	    value_on;

    highlighting = lwk_c_hl_none;

    /*
    **  Highlight toggle
    */

    value_on = XmToggleButtonGadgetGetState(env->highl_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_on;

    /*
    **  Sources toggle
    */
    	
    value_on = XmToggleButtonGadgetGetState(env->highl_src_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_sources;

    /*
    **  Targets toggle
    */

    value_on = XmToggleButtonGadgetGetState(env->highl_trg_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_targets;

    /*
    **  Orphans toggle
    */
	
    value_on = XmToggleButtonGadgetGetState(env->highl_orph_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_orphans;
	
    /*
    **  Current source toggle
    */
	
    value_on = XmToggleButtonGadgetGetState(env->highl_cur_src_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_pending_source;
	
    /*
    **  Current Target toggle
    */

    value_on = XmToggleButtonGadgetGetState(env->highl_cur_trg_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_pending_target;

    /*
    **  Current Destination toggle
    */

    value_on = XmToggleButtonGadgetGetState(env->highl_cur_dest_tgle);
	
    if (value_on)
	highlighting = highlighting | lwk_c_hl_destination_of_follow;

    /*
    **  Select Destination toggle
    */

    value_on = XmToggleButtonGadgetGetState(env->select_dst_tgle);

    if (value_on)
	highlighting = highlighting | lwk_c_hl_select_destination;

    /*
    ** Check if something has changed
    */
    
    _GetAttribute(env_context, _HighlightingAttr, &prev_highlighting);
	
    if (prev_highlighting != highlighting) {
    
	/*
	**  Set the attribute on the environment context
	*/
    
	_SetAttribute(env_context, _HighlightingAttr, &highlighting);
	env->settings_changed = _True;
    }

    return;
    }


static _Void  SetConnectPart(env_context, env)
_EnvContext env_context;
 _CustEnvPrivate env;

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
    lwk_boolean	    his_retain_attr;
    lwk_boolean	    prev_retain_attr;
    lwk_ddif_string ddif_string;
    Boolean	    retain_attr;
    XmString	    relation;

    /*
    **  Retain source toggle
    */
                                                  
    retain_attr = XmToggleButtonGadgetGetState(env->ret_src_tgle);

    his_retain_attr = retain_attr;

    _GetAttribute(env_context, _RetainSourceAttr, &prev_retain_attr);

    if (prev_retain_attr != his_retain_attr) {

	_SetAttribute(env_context, _RetainSourceAttr, &his_retain_attr);
	env->settings_changed = _True;
    }

    /*
    **  Relationship attribute
    */

    relation = (XmString) DXmCSTextGetString(env->relation_cstxt);

    if (_IsEmptyCString(relation)) 
	_SetAttribute(env_context, _ConnectionTypeAttr, (_AnyPtr) 0);
    else {
	ddif_string = _CStringToDDIFString((_CString) relation);	
	_SetAttribute(env_context, _ConnectionTypeAttr, &ddif_string);
	_DeleteDDIFString(&ddif_string);
    };
    
    XmStringFree(relation);

    return;
    }


static _Void  SetFollowPart(env_context, env)
_EnvContext env_context;
 _CustEnvPrivate env;

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
    lwk_string	lwk_operation;
    lwk_string	prev_operation;
    Widget	button;
    Arg		arglist[1];

    /*
    **  Select Destination toggle -- The value is collected in the highlight
    **	code
    */

    /*
    **  Default operation value
    */

    XtSetArg(arglist[0], XmNmenuHistory, &button);
    XtGetValues(env->oper_option, arglist, 1);

    _GetAttribute(env_context, _OperationAttr, &prev_operation);

    if (button == env->oper_def_but) {

	/*
	**  If it is the default operation don't set anything
	*/

	if (prev_operation != (lwk_string) 0) {
	    _SetAttribute(env_context, _OperationAttr, lwk_c_null_object);
	    env->settings_changed = _True;
	}
    }
    else {
    
	if (button == env->oper_but1)
	    lwk_operation = lwk_c_view_op_id;

	if (button == env->oper_but2)
	    lwk_operation = lwk_c_edit_op_id;

	if (button == env->oper_but3)
	    lwk_operation = lwk_c_activate_op_id;

	if (_CompareString(prev_operation, lwk_operation) != 0) {
	    _SetAttribute(env_context, _OperationAttr, &lwk_operation);
	    env->settings_changed = _True;
	}
    }

    return;
    }


static _Void  UilValueChangedEnvFields(w, private, reason)
Widget w;
 _WindowPrivate private;

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
    _EnvWindowPrivate   env_private;
    _CustEnvPrivate     env;

    env_private = (_EnvWindowPrivate) private->specific;
    env = (_CustEnvPrivate) env_private->environment;

    if (!env->settings_changed) {
	env->settings_changed = _True;
    }

    return;
    }
    

static _Void  UilActivateEnvCancel(w, private, reason)
Widget w;
 _WindowPrivate private;

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
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    XtUnmanageChild(((_CustEnvPrivate) (env_private->environment))->box_widget);

    return;
    }


static _Void  UilCreateEnvHighlight(w, private, reason)
Widget w;
 _WindowPrivate private;

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
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_tgle) = w;

    return;
    }

static _Void  UilCreateEnvHighSources(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_src_tgle) = w;

    return;
    }

static _Void  UilCreateEnvHighTargets(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_trg_tgle) = w;

    return;
    }

static _Void  UilCreateEnvHighOrphans(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_orph_tgle) = w;

    return;
    }

static _Void  UilCreateEnvCurSource(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_cur_src_tgle) = w;

    return;
    }

static _Void  UilCreateEnvCurTarget(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_cur_trg_tgle) = w;

    return;
    }

static _Void  UilCreateEnvCurDest(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->highl_cur_dest_tgle) = w;

    return;
    }

static _Void  UilCreateEnvRelation(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->relation_cstxt) = w;

    return;
    }

static _Void  UilCreateEnvRetSource(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->ret_src_tgle) = w;

    return;
    }

static _Void  UilCreateEnvOperOption(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->oper_option) = w;

    return;
    }

static _Void  UilCreateEnvOptDefButton(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->oper_def_but) = w;

    return;
    }

static _Void  UilCreateEnvOptButton1(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->oper_but1) = w;

    return;
    }

static _Void  UilCreateEnvOptButton2(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->oper_but2) = w;

    return;
    }

static _Void  UilCreateEnvOptButton3(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->oper_but3) = w;

    return;
    }

static _Void  UilCreateEnvSelectDst(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->select_dst_tgle) = w;

    return;
    }

static _Void  UilCreateEnvOk(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustEnvPrivate) (env_private->environment))->ok_button) = w;

    return;
    }


_Void  EnvCustHypDisplayLinkWorksMgr(private)
_WindowPrivate private;

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
    _CustHypPrivate	hyp;
    _EnvWindowPrivate	env_private;
    Widget		shell;
    _Boolean		recycled = _False;

    _StartExceptionBlock

    /*
    **  Has the box already been created
    */

    env_private = (_EnvWindowPrivate) private->specific;

    if (env_private->linkworksmgr == (_AnyPtr) 0)
	CreateHypBox(private);

    else
	recycled = _True;

    /*
    **  Load the environment box with the attributes
    */

    SetLinkWorksMgrBox(private);

    hyp = (_CustHypPrivate) (env_private->linkworksmgr);

    /*
    **  If the box is already mapped just raise it, otherwise map it
    */

    if (XtIsManaged(hyp->box_widget)) {

	shell = XtParent(hyp->box_widget);
	XRaiseWindow(XtDisplay(shell), XtWindow(shell));
    }
    else
	XtManageChild(hyp->box_widget);

    if (recycled)
        /*
	** Reset the default button
	*/
	ResetHypActionButtonsState(hyp);

    _Exceptions
	_WhenOthers

	    if (env_private->linkworksmgr != (_AnyPtr) 0) {
		_FreeMem(env_private->linkworksmgr);
		env_private->linkworksmgr = (_AnyPtr) 0;
	    }
	    _Reraise;

    _EndExceptionBlock

    return;
    }


static _Void  CreateHypBox(private)
_WindowPrivate private;

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
    _CustHypPrivate	hyp;
    _EnvWindowPrivate	envprivate;
    _Integer		status;
    MrmHierarchy	hierarchy_id;
    MrmType		*class_return;

    envprivate = (_EnvWindowPrivate) private->specific;

    /*
    **	Allocate memory for private structure
    */

    hyp = (_CustHypPrivate) _AllocateMem(sizeof(_CustHypPrivateInstance));

    envprivate->linkworksmgr = (_AnyPtr) hyp;

    /*
    **	Initialize the box
    */

    _ClearMem(hyp, sizeof(_CustHypPrivateInstance));

    /*
    **	Register private data
    */

    EnvDWRegisterDRMNames(private);

    /*
    **	Create the environment box
    */

    hierarchy_id = EnvDwGetMainHierarchy();

    status = MrmFetchWidget(hierarchy_id, _CustomizeEnvManagerBox,
	private->main_widget, &hyp->box_widget, (MrmType *) &class_return);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    return;
    }


static _Void  ResetHypActionButtonsState(hyp)
_CustHypPrivate hyp;

/*
**++
**  Functional Description:
**
**	Reset the state of the action buttons in the customize hyperEnvironment
**	box.
**	The Ok button is the default one.
**
**  Keywords:
**      
**
**--
*/
{
    if (hyp->ok_button != NULL) 
	XmProcessTraversal(hyp->ok_button, XmTRAVERSE_CURRENT);

    return;
}


static _Void  SetLinkWorksMgrBox(private)
_WindowPrivate private;

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
    _EnvContext		env_context;
    _EnvWindowPrivate	env_private;
    _CustHypPrivate	hyp;
    lwk_boolean		icon_state;

    env_private = (_EnvWindowPrivate) private->specific;
    hyp = (_CustHypPrivate) env_private->linkworksmgr;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    /*
    **  Set the startup state for the radio buttons
    */

    _GetAttribute(env_context, _EnvIconizedAttr, &icon_state);

    if (icon_state) {
	XmToggleButtonGadgetSetState (hyp->icon_tgle, _True, _False);
	XmToggleButtonGadgetSetState (hyp->window_tgle, _False, _False);
    }
    else {
	XmToggleButtonGadgetSetState (hyp->icon_tgle, _False, _False);
	XmToggleButtonGadgetSetState (hyp->window_tgle, _True, _False);
    }

    return;
    }


static _Void  UilActivateHypOk(w, private, reason)
Widget w;
 _WindowPrivate private;
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
    _EnvWindowPrivate	env_private;
    _EnvContext		env_context;
    _CustHypPrivate	hyp;
    lwk_boolean		his_icon_state;
    lwk_boolean		prev_state;
    Boolean		icon_state;

    env_private = (_EnvWindowPrivate) private->specific;
    hyp = (_CustHypPrivate) env_private->linkworksmgr;

    /*
    **  Unmanage the box
    */

    XtUnmanageChild(hyp->box_widget);

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    /*
    **  Get the environment context object
    */

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    /*
    **  Set the attributes that have been modified
    */

    _GetAttribute(env_context, _EnvIconizedAttr, &prev_state);

    icon_state = XmToggleButtonGadgetGetState(hyp->icon_tgle);
    his_icon_state = (lwk_boolean) icon_state;

    if (his_icon_state != prev_state) {

	_SetAttribute(env_context, _EnvIconizedAttr, &his_icon_state);
	_SetWindowState(private->window, _StateCustomNotSaved, _StateSet);
    }

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(failure);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateHypCancel(w, private, reason)
Widget w;
 _WindowPrivate private;

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
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    XtUnmanageChild(((_CustHypPrivate) (env_private->linkworksmgr))->box_widget);

    return;
    }


static _Void  UilCreateHypWindowState(w, private, reason)
Widget w;
 _WindowPrivate private;

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
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustHypPrivate) (env_private->linkworksmgr))->window_tgle) = w;

    return;
    }

static _Void  UilCreateHypIconState(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustHypPrivate) (env_private->linkworksmgr))->icon_tgle) = w;

    return;
    }

static _Void  UilCreateHypOk(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _EnvWindowPrivate	env_private;

    env_private = (_EnvWindowPrivate) private->specific;

    (((_CustHypPrivate) (env_private->linkworksmgr))->ok_button) = w;

    return;
    }


_Void  EnvCustSetCursor(private, cursor_type)
_WindowPrivate private;
 _CursorType cursor_type;

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
    _EnvWindowPrivate	env_private;
    _CustEnvPrivate	env;
    _CustHypPrivate	hyp;

    env_private = (_EnvWindowPrivate) private->specific;

    env = (_CustEnvPrivate) env_private->environment;
    hyp = (_CustHypPrivate) env_private->linkworksmgr;

    if (env != (_CustEnvPrivate) 0)
	if (env->box_widget != (Widget) 0)
	    if (XtIsManaged(env->box_widget))
		EnvDwSetCursor(env->box_widget, cursor_type);


    hyp = (_CustHypPrivate) env_private->linkworksmgr;

    if (hyp != (_CustHypPrivate) 0)
	if (hyp->box_widget != (Widget) 0)
	    if (XtIsManaged(hyp->box_widget))
		EnvDwSetCursor(hyp->box_widget, cursor_type);

    return;
    }
