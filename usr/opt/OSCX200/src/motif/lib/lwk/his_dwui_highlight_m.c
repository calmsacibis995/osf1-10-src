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
**	LinkWorks Services User Interface
**
**  Version: V1.0
**
**  Abstract:
**	LWK DXm User Interface Highlight dialog box.
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

_DeclareFunction(static void CreateHighlight,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateSources,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateTargets,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOrphans,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCurrentSource,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCurrentTarget,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateDestination,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateOk,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ToggleHighlight,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ToggleHighlightToggle,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateApply,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateReset,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static _HighlightFlags GetHighlightSettings,
    (_DXmUiPrivate private));
_DeclareFunction(static void SetHighlightBox,
    (_DXmUiPrivate private, _HighlightFlags flags));
_DeclareFunction(static void SetButtonSensitivity,
    (_DXmUiPrivate private));
_DeclareFunction(static void CreateOk,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateApply,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateReset,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void HighlightPushBtnSensitivity,
    (_HighlightWidgets widgets, _Boolean sensitive));
_DeclareFunction(static void HighlightCancelBtnDef,
    (_HighlightWidgets widgets));
    
/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkCreateHlightHighlight", (caddr_t) CreateHighlight},
	{"LwkCreateHlightSources", (caddr_t) CreateSources},
	{"LwkCreateHlightTargets", (caddr_t) CreateTargets},
	{"LwkCreateHlightOrphans", (caddr_t) CreateOrphans},
	{"LwkCreateHlightCurrentSource", (caddr_t) CreateCurrentSource},
	{"LwkCreateHlightCurrentTarget", (caddr_t) CreateCurrentTarget},
	{"LwkCreateHlightDestination", (caddr_t) CreateDestination},
	{"LwkToggleHlightHighlight", (caddr_t) ToggleHighlight},
	{"LwkToggleHlightToggle", (caddr_t) ToggleHighlightToggle},
	{"LwkActivateHlightOk", (caddr_t) ActivateOk},
	{"LwkActivateHlightApply", (caddr_t) ActivateApply},
	{"LwkActivateHlightReset", (caddr_t) ActivateReset},
	{"LwkActivateHlightCancel", (caddr_t) ActivateCancel},
	{"LwkCreateHlightOk", (caddr_t) CreateOk},
	{"LwkCreateHlightApply", (caddr_t) CreateApply},
	{"LwkCreateHlightReset", (caddr_t) CreateReset},
	{"LwkCreateHlightCancel", (caddr_t) CreateCancel}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


void  LwkDXmHighlightCreate(private)
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
    MrmCode type;
    Arg arglist[5];
    XmString cs_title;
    XmString cs_template;
    MrmType *dummy_class;
    MrmHierarchy hierarchy;
    Widget shell;

    /*
    ** Register all DRM function names and dwui object name.
    */

    MrmRegisterNames(Register, RegisterSize);

    /*
    **  Open the DRM hierarchy
    */

    hierarchy = LwkDXmOpenDRMHierarchy(private);

    /*
    **  Fetch the Dialog Box Title template
    */

    status = MrmFetchLiteral(hierarchy, "HlightBoxTitle",
	XtDisplay(private->main_widget), (caddr_t *) &cs_template, &type);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    **  Construct the Dialog Box Title from the template and the
    **  Application Name.
    */

    cs_title = XmStringConcat(private->appl_name, cs_template);

    ac = 0;
    XtSetArg(arglist[ac], XmNdialogTitle, cs_title);
    ac++;

    /*
    ** Fetch the Highlight Dialog Box, setting its title
    */

    shell = XtAppCreateShell("HighlightBox", "HighlightBox",
	topLevelShellWidgetClass, XtDisplay(private->main_widget),
	(Arg *) 0, (int) 0);

    /*
    ** Fetch the box
    */
    status = MrmFetchWidgetOverride(hierarchy, "HighlightBox",	shell,
	"HighlightBox", arglist, ac,
	&private->highlight->dialog, (MrmType *) &dummy_class);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(XtParent(private->highlight->dialog),
		      StructureNotifyMask, False,
		      LwkDXmSetIconsOnShell, private);

    /*
    **	Special case setting of multi segment compound string title
    **	for XUI Window Manager.
    */

    if (LwkDXmIsXUIWMRunning(private->highlight->dialog)) 
	LwkDXmSetXUIDialogTitle(private->highlight->dialog,
	    private->appl_name, cs_template);

    /*
    **  Clean up
    */

    XmStringFree(cs_title);
    XmStringFree(cs_template);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }


void  LwkDXmHighlight(private)
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

    _Boolean initial_display = _False;

    /*
    **  If the Dialog Box isn't created, do it.
    */

    if (private->highlight->dialog == (Widget) 0) {
	LwkDXmHighlightCreate(private);
	initial_display = _True;
    }

    /*
    ** If the Dialog Box isn't realized, do it.
    */

    if (!XtIsRealized(private->highlight->dialog)) 
	XtRealizeWidget(private->highlight->dialog);

    /*
    ** If the box isn't already managed then set it and pop it up.
    */

    if (!XtIsManaged(private->highlight->dialog)) {

    	/*
	** Get the current DWUI object settings.
	*/

	_GetValue(private->dwui, _P_ApplHighlight, lwk_c_domain_integer,
	    &private->highlighting);

	/*
	** Set the highlight box to use the current dwui settings.
	*/

	SetHighlightBox(private, private->highlighting);
	
	/*
	** Popup the dialog box.
	*/

	if (initial_display) {
	    /*								
	    ** Position with respect to the main applicaiton window.
	    */

	    Widget ref_widget;

	    if (XtParent(private->main_widget) != 0)
		ref_widget = XtParent(private->main_widget); /* get the shell */
	    else
		ref_widget = private->main_widget;	     /* it is a shell */
	
	    LwkDXmPositionWidget(private->highlight->dialog, ref_widget);
	}

	XtManageChild(private->highlight->dialog);
    }

    HighlightPushBtnSensitivity(private->highlight, _False);

    HighlightCancelBtnDef(private->highlight);

    /*
    ** De-iconize the dialog box if needed
    */

    LwkDXmSetShellIconState(XtParent(private->highlight->dialog), NormalState);

    return;
    }


/*
**++
**  Functional Description:
**	Create callback routines various widgets.
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
static void  CreateHighlight(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->highlight = w;

    return;
    }

static void  CreateSources(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->source = w;

    return;
    }

static void  CreateTargets(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->target = w;

    return;
    }

static void  CreateOrphans(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->orphan = w;

    return;
    }

static void  CreateCurrentSource(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->pending_source = w;

    return;
    }

static void  CreateCurrentTarget(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->pending_target = w;

    return;
    }

static void  CreateDestination(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->destination = w;

    return;
    }

static void  CreateOk(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->ok_button= w;

    return;
    }

static void  CreateApply(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->apply_button= w;

    return;
    }

static void  CreateReset(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->reset_button= w;

    return;
    }

static void  CreateCancel(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->highlight->cancel_button= w;

    return;
    }


static void  ToggleHighlight(w, private, reason)
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
    **  Make sure the Toggles have the proper sensitivity
    */

    SetButtonSensitivity(private);

    /*
    ** Value changed, so enable Reset, Ok, and Apply push buttons
    */
    
    HighlightPushBtnSensitivity(private->highlight, _True);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(highlight_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ToggleHighlightToggle(w, private, reason)
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
    ** Value changed, so enable Reset, Ok, and Apply push buttons
    */
    
    HighlightPushBtnSensitivity(private->highlight, _True);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(highlight_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

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
    _HighlightFlags flags;

    _StartExceptionBlock

    /*
    ** Unmanage the highlight box.
    */

    XtUnmanageChild(private->highlight->dialog);

    /*
    ** Get the highlight box settings.
    */

    flags = GetHighlightSettings(private);

    /*
    ** If these flags are different from the current settings, apply them.
    */

    if (flags != private->highlighting)
	_SetValue(private->dwui, _P_ApplHighlight, lwk_c_domain_integer,
	    &flags, lwk_c_set_property);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(highlight_error);
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
    _HighlightFlags flags;

    _StartExceptionBlock

    /*
    ** Get the highlight box settings.
    */

    flags = GetHighlightSettings(private);

    /*
    ** Apply these settings if they have changed
    */

    if (flags != private->highlighting) {
	private->highlighting = flags;

	_SetValue(private->dwui, _P_ApplHighlight, lwk_c_domain_integer,
	    &private->highlighting, lwk_c_set_property);
    }

    /*
    ** Because we have saved these changes already, make the
    ** Ok, Reset, and Apply buttons insensitve until the
    ** user makes a change.
    */

    HighlightPushBtnSensitivity(private->highlight, _False);    

    HighlightCancelBtnDef(private->highlight);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(highlight_error);
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
    ** Reset the dialog box.
    */

    _GetValue(private->dwui, _P_ApplHighlight, lwk_c_domain_integer,
	&private->highlighting);

    SetHighlightBox(private, private->highlighting);

    /*
    ** Because we reset these changes, make the
    ** Ok, Reset, and Apply buttons insensitve until the
    ** user makes a change.
    */

    HighlightPushBtnSensitivity(private->highlight, _False);    

    HighlightCancelBtnDef(private->highlight);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(highlight_error);
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
    /*
    ** Unmanage the Highlight box.
    */

    XtUnmanageChild(private->highlight->dialog);

    return;
    }


static _HighlightFlags  GetHighlightSettings(private)
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
    _HighlightFlags flags;

    /*
    ** Get the highlight box settings.
    */

    flags = lwk_c_hl_none;

    if (XmToggleButtonGadgetGetState(private->highlight->highlight))
        flags |= lwk_c_hl_on;

    if (XmToggleButtonGadgetGetState(private->highlight->source))
	flags |= lwk_c_hl_sources;

    if (XmToggleButtonGadgetGetState(private->highlight->target))
	flags |= lwk_c_hl_targets;

    if (XmToggleButtonGadgetGetState(private->highlight->orphan))
	flags |= lwk_c_hl_orphans;

    if (XmToggleButtonGadgetGetState(private->highlight->pending_source))
	flags |= lwk_c_hl_pending_source;

    if (XmToggleButtonGadgetGetState(private->highlight->pending_target))
	flags |= lwk_c_hl_pending_target;

    if (XmToggleButtonGadgetGetState(private->highlight->destination))
	flags |= lwk_c_hl_destination_of_follow;

    return flags;
    }


static void  SetHighlightBox(private, flags)
_DXmUiPrivate private;
 _HighlightFlags flags;

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
    ** Update the highlight box.
    */

    if ((flags & lwk_c_hl_on) != 0)
	XmToggleButtonGadgetSetState(private->highlight->highlight, TRUE, TRUE);
    else
	XmToggleButtonGadgetSetState(private->highlight->highlight, FALSE, TRUE);

    if ((flags & lwk_c_hl_sources) != 0)
	XmToggleButtonGadgetSetState(private->highlight->source, TRUE, FALSE);
    else
	XmToggleButtonGadgetSetState(private->highlight->source, FALSE, FALSE);

    if ((flags & lwk_c_hl_targets) != 0)
	XmToggleButtonGadgetSetState(private->highlight->target, TRUE, FALSE);
    else
	XmToggleButtonGadgetSetState(private->highlight->target, FALSE, FALSE);

    if ((flags & lwk_c_hl_orphans) != 0)
	XmToggleButtonGadgetSetState(private->highlight->orphan, TRUE, FALSE);
    else
	XmToggleButtonGadgetSetState(private->highlight->orphan, FALSE, FALSE);

    if ((flags & lwk_c_hl_pending_source) != 0)
	XmToggleButtonGadgetSetState(private->highlight->pending_source, TRUE,
	    FALSE);
    else
	XmToggleButtonGadgetSetState(private->highlight->pending_source, FALSE,
	    FALSE);

    if ((flags & lwk_c_hl_pending_target) != 0)
	XmToggleButtonGadgetSetState(private->highlight->pending_target, TRUE,
	    FALSE);
    else
	XmToggleButtonGadgetSetState(private->highlight->pending_target, FALSE,
	    FALSE);

    if ((flags & lwk_c_hl_destination_of_follow) != 0)
	XmToggleButtonGadgetSetState(private->highlight->destination, TRUE, FALSE);
    else
	XmToggleButtonGadgetSetState(private->highlight->destination, FALSE, FALSE);

    /*
    **  Make sure the Toggles have the proper sensitivity
    */

    SetButtonSensitivity(private);

    return;
    }

static void  SetButtonSensitivity(private)
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
    Boolean sensitive;

    /*
    **	If the Highlight Toggle Button is on, set the other Toggle Buttons
    **	sensitive, otherwise set them insensitive.
    */

    sensitive = XmToggleButtonGadgetGetState(private->highlight->highlight);

    XtSetSensitive(private->highlight->source, sensitive);
    XtSetSensitive(private->highlight->target, sensitive);
    XtSetSensitive(private->highlight->orphan, sensitive);
    XtSetSensitive(private->highlight->pending_source, sensitive);
    XtSetSensitive(private->highlight->pending_target, sensitive);
    XtSetSensitive(private->highlight->destination, sensitive);

    return;
    }



static void  HighlightPushBtnSensitivity(widgets, sensitive)
_HighlightWidgets widgets;

    _Boolean sensitive;

/*
**++
**  Functional Description:
**  
**	Set the sensitivity of the pushbuttons to the given value.
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

    XtSetValues(widgets->ok_button, arglist, 1);
    XtSetValues(widgets->apply_button, arglist, 1);
    XtSetValues(widgets->reset_button, arglist, 1);     

    /*
    ** If the Ok button is set to sensitive, it becomes the default button.
    */
    if (sensitive) {
	XtSetArg(arglist[0], XmNdefaultButton, widgets->ok_button);
	XtSetValues(widgets->dialog, arglist, 1);
    }
}


static void  HighlightCancelBtnDef(widgets)
_HighlightWidgets widgets;

/*
**++
**  Functional Description:
**	Set the Cancel button to be the default one.
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

    /*
    ** Change the default push button
    */
    XtSetArg(arglist[0], XmNdefaultButton, widgets->cancel_button);
    XtSetValues(widgets->dialog, arglist, 1);
        
    /*
    ** The Cancel button will receive keyboard events.
    */
    
    XmProcessTraversal(widgets->cancel_button, XmTRAVERSE_CURRENT);
}

