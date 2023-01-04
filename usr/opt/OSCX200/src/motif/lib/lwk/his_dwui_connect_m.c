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
**	LinkWorks Services User Interface
**
**  Version: V1.0
**
**  Abstract:
**	LWK DXm User Interface Complete Link dialog box.
**
**  Keywords:
**	LWK, UI
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 7-Oct-88
**
**  Modification History:
**	BL4  dpr  24-Jan-89 -- some serious clean up after BL4 code review
**--
*/


/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_dwui_decwindows_m.h"

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void CreateSourceDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateSourceType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCompleteLinkType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCompleteLinkDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateTargetType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateTargetDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateRetainSource,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOkButton,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateResetButton,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeSourceDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeCompleteLinkType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeCompleteLinkDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeTargetDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateOk,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateApply,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateReset,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void LinkResetBtnSensitivity,
    (_LinkWidgets widgets, _Boolean sensitive));
_DeclareFunction(static void LinkOkBtnDefault,
    (_LinkWidgets widgets));

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkCreateLinkSourceDesc", (caddr_t) CreateSourceDesc},
	{"LwkCreateLinkSourceType", (caddr_t) CreateSourceType},
	{"LwkCreateLinkType", (caddr_t) CreateCompleteLinkType},
	{"LwkCreateLinkDesc", (caddr_t) CreateCompleteLinkDesc},
	{"LwkCreateLinkTargetType", (caddr_t) CreateTargetType},
	{"LwkCreateLinkTargetDesc", (caddr_t) CreateTargetDesc},
	{"LwkCreateLinkRetainSource", (caddr_t) CreateRetainSource},
	{"LwkCreateLinkOkButton", (caddr_t) CreateOkButton},
	{"LwkCreateLinkResetButton", (caddr_t) CreateResetButton},
	{"LwkChangeLinkSourceDesc", (caddr_t) ChangeSourceDesc},
	{"LwkChangeLinkType", (caddr_t) ChangeCompleteLinkType},
	{"LwkChangeLinkDesc", (caddr_t) ChangeCompleteLinkDesc},
	{"LwkChangeLinkTargetDesc", (caddr_t) ChangeTargetDesc},
	{"LwkActivateLinkOk", (caddr_t) ActivateOk},
	{"LwkActivateLinkApply", (caddr_t) ActivateApply},
	{"LwkActivateLinkReset", (caddr_t) ActivateReset},
	{"LwkActivateLinkCancel", (caddr_t) ActivateCancel}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);

          
void  LwkDXmCompleteLinkCreate(private)
_DXmUiPrivate private;

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
    int ac;
    int status;
    Arg arglist[2];
    char *filename;
    MrmType *dummy_class ;
    MrmHierarchy hierarchy;
    MrmRegisterArg drm_register[1];
    Widget shell;
    
    /*
    ** Register DRM names.
    */

    MrmRegisterNames(Register, RegisterSize);

    drm_register[0].name = _DrmPrivateIdentifier;
    drm_register[0].value = (caddr_t) private;

    MrmRegisterNames(drm_register, (MrmCount) 1);

    /*
    ** Open the DRM hierarchy.
    */
    
    filename = _UidFileName;
           
    status = MrmOpenHierarchy((MrmCount) 1, &filename, (MrmOsOpenParamPtr *) 0,
	&hierarchy);

    if (status != MrmSUCCESS)
	_Raise(drm_open_error);

    /*
    ** Fetch the Complete Link dialog box.
    */

    shell = XtAppCreateShell("CompleteLinkBox", "CompleteLinkBox",
	topLevelShellWidgetClass, XtDisplay(private->main_widget),
	(Arg *) 0, (int) 0);

    /*
    ** Fetch the box
    */
    if (MrmFetchWidget(hierarchy, "CompleteLinkBox", shell,
	    &private->complete_link->dialog, (MrmType *) &dummy_class)
	    != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(XtParent(private->complete_link->dialog),
		      StructureNotifyMask, False,
		      LwkDXmSetIconsOnShell, private);
    
    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }



void  LwkDXmCompleteLink(private, link, iff_visible, closure)
_DXmUiPrivate private;
 _Link link;

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

    Widget shell;
    
    /*
    **  If we were requested to update the Dialog Box if and only if it is
    **	visible (i.e., a request to update contents), make sure it is visible.
    */

    if (iff_visible) {
	if (private->complete_link->dialog == (Widget) 0)
	    return;

	if (!private->complete_link->dialog_on_screen)
	    return;
    }

    /*
    ** Set Link to be the given one.
    */
             
    private->link_to_link = link;

    /*
    **  If the Dialog Box isn't created, do it.
    */

    if (private->complete_link->dialog == (Widget) 0)
	LwkDXmCompleteLinkCreate(private);
    
    /*
    ** Set the properties in the Link dialog box.  If we are just updating a
    ** visible Dialog Box, save any changes the user made first.
    */

    if (iff_visible)
	LwkDXmLinkUpdate(link, private->complete_link);

    LwkDXmLinkDisplay(private, link, private->complete_link);

    /*
    **  If the Dialog Box is not already visible
    */

    if (!XtIsManaged(private->complete_link->dialog)) {
	_Boolean retain_source;

	/*
	**  Get the Retain Source Toggle Button
	*/

	_GetValue(private->dwui, _P_RetainSource, lwk_c_domain_boolean,
	    &retain_source);

	XmToggleButtonGadgetSetState(private->complete_link->retain_source,
	    (Boolean) retain_source, FALSE);

	/*
	** Popup the dialog box.  Position the window with
	** reference to the main application window.
	*/

	if (XtParent(private->main_widget) != 0)
	    shell = XtParent(private->main_widget);	/* get the shell */
	else
	    shell = private->main_widget;		 /* it is a shell */

	LwkDXmLinkPopup(private->complete_link, shell, _True,
	    (_Reason) closure);
    }

    /*
    **  Set the Reset button insensitive until
    **  the user makes a change.
    */

    LinkResetBtnSensitivity(private->complete_link, _False);

    /*
    ** Ok button gets keyboard events
    */
					   
    LinkOkBtnDefault (private->complete_link);

    LwkDXmSetShellIconState(XtParent(private->complete_link->dialog), NormalState);

    return;
    }

          
void  LwkDXmLinkPopup(widgets, shell, pending_delete, reason)
_LinkWidgets widgets;
 Widget shell;

    _Boolean pending_delete;
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
    Time time;
    XmString xstring;
    _Boolean initial_display = _False;

    /*
    **  Realize the Dialog Box, if necessary
    */

    if (!XtIsRealized(widgets->dialog)) {

	XtRealizeWidget(widgets->dialog);

	initial_display = _True;  /* we'll assume it's the first time */
    }
    
    /*
    **  If the Dialog box is not already up, position it and then pop it up
    */

    if (XtIsManaged(widgets->dialog)) 
	XRaiseWindow(XtDisplay(widgets->dialog),
	    XtWindow(XtParent(widgets->dialog)));
    else {

	/* Position the widget if it is the first time we're displaying it  */
	
        if (initial_display) 
	    LwkDXmPositionWidget(widgets->dialog, shell);

	XtManageChild(widgets->dialog);

        /*
	** Keep track of the fact that the dialog box has		   
	** been displayed.  We need to remember this because the user	    
	** may iconize the box, in which case we can't use XtIsManaged	    
	** to tell us if the box is dispalyed.  XtIsManaged returns	   
	** false if the widget is iconzied.
	*/
	widgets->dialog_on_screen = _True;
    }

    /*
    ** De-iconize the box if necessary
    */

    LwkDXmSetShellIconState(XtParent(widgets->dialog), NormalState);
    
    /*
    **  If requested, preselect for pending delete the Link Relationship
    */

    if (pending_delete) {
	int length;

	if (reason == (_Reason) 0)
	    time = CurrentTime;
	else
	    time = ((XButtonEvent *) reason->event)->time;
                                                         
	xstring = (XmString) DXmCSTextGetString(widgets->link_type);

	length = XmStringLength((XmString) xstring);

	DXmCSTextSetSelection (widgets->link_type, 0, length, time);	

	XtCallAcceptFocus(widgets->link_type, &time);

	XmStringFree(xstring);
    }

    return;
    }


/*
**++
**  Functional Description:
**	Create callback routines for various widgets.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget id of the widget doing the callback.
**	private: dwui private data.
**	reason: reason for which this routine was invoked.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
static void  CreateSourceDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->source_desc = w;

    return;
    }

static void  CreateSourceType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->source_type = w;

    return; 
    }

static void  CreateCompleteLinkType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->link_type = w;

    return;
    }

static void  CreateCompleteLinkDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->link_desc = w;

    return;
    }

static void  CreateTargetType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->target_type = w;

    return;
    }

static void  CreateTargetDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->target_desc = w;

    return;
    }

static void  CreateRetainSource(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->retain_source = w;

    return;
    }


static void  CreateOkButton(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->ok_button = w;

    return;
    }

static void  CreateResetButton(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->reset_button = w;

    return;
    }


/*
**++
**  Functional Description:
**	Value Changed callback routines for various text widgets.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget id of the widget doing the callback.
**	private: dwui private data.
**	reason: reason for which this routine was invoked.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
static void  ChangeSourceDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->source_desc_changed = _True;

    if (private->complete_link->source_desc != 0)
	LinkResetBtnSensitivity(private->complete_link, _True);    

    return;
    }

static void  ChangeCompleteLinkType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->link_type_changed = _True;

    if (private->complete_link->link_type != 0)
	LinkResetBtnSensitivity(private->complete_link, _True);    

    return;
    }

static void  ChangeCompleteLinkDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->link_desc_changed = _True;

    if (private->complete_link->link_desc != 0)
	LinkResetBtnSensitivity(private->complete_link, _True);    

    return;
    }

static void  ChangeTargetDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->complete_link->target_desc_changed = _True;

    if (private->complete_link->target_desc != 0)
	LinkResetBtnSensitivity(private->complete_link, _True);    

    return;
    }


static void  ActivateOk(w, private, reason)
Widget w;
 _DXmUiPrivate private;
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
    _Boolean retain_source;

    _StartExceptionBlock

    /*
    ** Unmanage the dialog box.
    */

    XtUnmanageChild(private->complete_link->dialog);

    private->complete_link->dialog_on_screen = _False;
    
    /*
    **  Apply any changes to Link properties
    */

    LwkDXmLinkUpdate(private->link_to_link, private->complete_link);

    /*
    ** Confirm the Link.
    */

    retain_source = XmToggleButtonGadgetGetState(
	private->complete_link->retain_source);

    _ConfirmLink(private->dwui, _True, retain_source, _False);

    private->link_to_link = (_Link) _NullObject;

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(complete_link_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateApply(w, private, reason)
Widget w;
 _DXmUiPrivate private;
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
    _StartExceptionBlock

    /*
    **  Apply any changes to Link properties
    */

    LwkDXmLinkUpdate(private->link_to_link, private->complete_link);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(link_update_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateReset(w, private, reason)
Widget w;
 _DXmUiPrivate private;

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
**  Result:
**	{@return-value-list-or-none@}
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _StartExceptionBlock

    /*
    ** Set the Link properties to their initial state.
    */

    LwkDXmLinkDisplay(private, private->link_to_link, private->complete_link);

    /*
    ** Because the current state has been saved, make the
    ** Reset button insensitive.
    */

    LinkResetBtnSensitivity(private->complete_link, _False);    
    LinkOkBtnDefault(private->complete_link);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(link_update_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateCancel(w, private, reason)
Widget w;
 _DXmUiPrivate private;

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
    _Boolean retain_source;

    _StartExceptionBlock

    /*
    ** Unmanage the Link dialog box.
    */

    XtUnmanageChild(private->complete_link->dialog);

    private->complete_link->dialog_on_screen = _False;

    /*
    ** Cancel the Link.
    */

    retain_source = XmToggleButtonGadgetGetState(
	private->complete_link->retain_source);

    _ConfirmLink(private->dwui, _False, retain_source, _False);

    private->link_to_link = (_Link) _NullObject;

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(complete_link_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  LinkResetBtnSensitivity(widgets, sensitive)
_LinkWidgets widgets;

    _Boolean sensitive;

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
    Arg arglist[1];
    
    XtSetArg(arglist[0], XmNsensitive, sensitive);

    XtSetValues(widgets->reset_button, arglist, 1);
}


static void  LinkOkBtnDefault (widgets)
_LinkWidgets widgets;

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
    XmProcessTraversal(widgets->ok_button, XmTRAVERSE_CURRENT);
}
