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
**  Version: X0.1
**
**  Abstract:
**	LWK DXm User Interface Show History dialog box.
**
**  Keywords:
**	LWK, UI
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**
**  Creation Date:
**	16-Mar-89
**
**  Modification History:
**--
*/


/*         
**  Include Files
*/

#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_dwui_decwindows_m.h"
#include "his_registry.h"

/*
**  Type definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void DisplayHistory,
    (_DXmUiPrivate private, _HistoryInfo history_info, _Integer count));
_DeclareFunction(static void CreateSteps,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateShow,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateVisit,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateButtonsForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateListForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateVisitForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateShowForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOperationMenu,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateShow,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateVisit,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void SelectOperation,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void SelectStep,
    (Widget w, _DXmUiPrivate private, DXmSvnCallbackStruct *svn));
_DeclareFunction(static void ConfirmStep,
    (Widget w, _DXmUiPrivate private, DXmSvnCallbackStruct *svn));
_DeclareFunction(static void SelectDoneHistory,
    (Widget w, _DXmUiPrivate private, DXmSvnCallbackStruct *svn));
_DeclareFunction(static void SetOperations, (_DXmUiPrivate private, int entry));
_DeclareFunction(static void ShowMore, (_DXmUiPrivate private));
_DeclareFunction(static void Visit, (_DXmUiPrivate private));
_DeclareFunction(static void SetDefaultButton, (_DXmUiPrivate private));
_DeclareFunction(static void ChangeDefaultButton,
    (_DXmUiPrivate private, Widget button));
_DeclareFunction(static void SetButtonSensitivity,
    (_DXmUiPrivate private, Boolean state));
_DeclareFunction(static void FreeHistoryInfo,
    (_HistoryInfo history_info, int size));
_DeclareFunction(static Pixmap GetSurrogatePixmap,
    (_DXmUiPrivate private, _Surrogate surrogate));
_DeclareFunction(void LwkDXmHelpSvnHelpRequested,
    (Widget w, _CString topic, DXmSvnCallbackStruct *cb));

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkCreateHistorySteps", (caddr_t) CreateSteps},
	{"LwkCreateHistoryShow", (caddr_t) CreateShow},
	{"LwkCreateHistoryVisit", (caddr_t) CreateVisit},
	{"LwkCreateHistoryCancel", (caddr_t) CreateCancel},
	{"LwkCreateHistoryButtonsForm", (caddr_t) CreateButtonsForm},
	{"LwkCreateHistoryListForm", (caddr_t) CreateListForm},
	{"LwkCreateHistoryShowForm", (caddr_t) CreateShowForm},
	{"LwkCreateHistoryVisitForm", (caddr_t) CreateVisitForm},
	{"LwkCreateHistoryOperationMenu", (caddr_t) CreateOperationMenu},
	{"LwkActivateHistoryShow", (caddr_t) ActivateShow},
	{"LwkActivateHistoryVisit", (caddr_t) ActivateVisit},
	{"LwkActivateHistoryCancel", (caddr_t) ActivateCancel},
	{"LwkSelectHistoryStep", (caddr_t) SelectStep},
	{"LwkConfirmHistoryStep", (caddr_t) ConfirmStep},
	{"LwkSelectHistoryOperation", (caddr_t) SelectOperation},
	{"LwkSelectDoneHistory", (caddr_t) SelectDoneHistory},
	{"LwkSvnHelpRequested", (caddr_t) LwkDXmHelpSvnHelpRequested}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


void  LwkDXmShowHistoryCreate(private)
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
    Arg arglist[2];
    Screen *screen;
    Display *display;
    Pixel foreground;
    Pixel background;
    MrmType *dummy_class;
    XmString xstring;
    MrmHierarchy hierarchy;
    Widget shell;
    
    /*
    ** Register all DRM function names.
    */

    MrmRegisterNames(Register, RegisterSize);

    /*
    **  Open the DRM hierarchy
    */

    hierarchy = LwkDXmOpenDRMHierarchy(private);

    /*
    ** Fetch the Show History dialog box.
    */

    shell = XtAppCreateShell("HistoryBox", "HistoryBox",
	topLevelShellWidgetClass, XtDisplay(private->main_widget),
	(Arg *) 0, (int) 0);

    /*
    ** Fetch the box
    */
    if (MrmFetchWidget(hierarchy, "HistoryBox", shell,
	    &private->history->dialog, (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);
		
    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(XtParent(private->history->dialog), StructureNotifyMask,
		      False, LwkDXmSetIconsOnShell, private);

    /*
    **  Fetch Literal Text values
    */

    status = MrmFetchLiteral(hierarchy, "GoToLiteral",
	XtDisplay(private->main_widget), (caddr_t *) &xstring, &type);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    private->history->goto_literal =
	(XmString) _CopyCString((_CString) xstring);

    status = MrmFetchLiteral(hierarchy, "VisitLiteral",
	XtDisplay(private->main_widget), (caddr_t *) &xstring, &type);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    private->history->visit_literal =
	(XmString) _CopyCString((_CString) xstring);

    status = MrmFetchLiteral(hierarchy, "ImplicitGoToLiteral",
	XtDisplay(private->main_widget), (caddr_t *) &xstring, &type);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    private->history->implicit_goto_literal =
	(XmString) _CopyCString((_CString) xstring);

    /*
    **  Create the assorted Pixmaps using the Dialog Box's fore/background
    **	colors.
    */

    screen = XtScreen(private->history->dialog);
    display = XtDisplay(private->history->dialog);

    ac = 0;

    XtSetArg(arglist[ac], XmNforeground, &foreground);
    ac++;
    XtSetArg(arglist[ac], XmNbackground, &background);
    ac++;

    XtGetValues(private->history->dialog, arglist, ac);

    status = MrmFetchIconLiteral(hierarchy, "StepIcon", screen, display,
	foreground, background, &private->step_icon);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    status = MrmFetchIconLiteral(hierarchy, "OriginIcon", screen, display,
	foreground, background, &private->origin_icon);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    status = MrmFetchIconLiteral(hierarchy, "DestinationIcon", screen, display,
	foreground, background, &private->destination_icon);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    status = MrmFetchIconLiteral(hierarchy, "OriginAndDestinationIcon", screen,
	display, foreground, background, &private->origin_and_destination_icon);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }



void  LwkDXmFreeHistoryInfo(private)
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
    /*
    **  Free any History Info we have saved away
    */

    if (private->history_info != (_HistoryInfo) 0) {
	FreeHistoryInfo(private->history_info, private->history_info_size);

	private->history_info = (_HistoryInfo) 0;
    }

    return;
    }


void  LwkDXmShowHistory(private, steps, origin_opr_ids, origin_opr_names, dest_opr_ids, dest_opr_names, iff_visible, closure)
_DXmUiPrivate private;
 _List steps;
 _List origin_opr_ids;

    _List origin_opr_names;
 _List dest_opr_ids;
 _List dest_opr_names;

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
    int i, j;
    _Integer op_cnt;
    _Integer volatile count;
    _HistoryInfo volatile history_info;
    _List_of(_String) volatile opr_ids_list;
    _List_of(_String) volatile opr_names_list;
    _Boolean initial_display = _False;
    Widget shell;
        						
    /*
    **  If we were requested to update the Dialog Box if and only if it is
    **	visible (i.e., a request to update contents), make sure it is visible.
    */

    if (iff_visible) {
	if (private->history->dialog == (Widget) 0)
	    return;

	if (!XtIsManaged(private->history->dialog))
	    return;
    }

    /*
    **  If the Dialog Box isn't created, do it.
    */

    if (private->history->dialog == (Widget) 0) {
	LwkDXmShowHistoryCreate(private);
	initial_display = _True;
    }
    
    /*
    ** If the History dialog box isn't realized, do it now (some of the SVN
    ** manipulations require this).
    */

    if (!XtIsRealized(private->history->dialog)) 
	XtRealizeWidget(private->history->dialog);

    /*
    **  Initialization
    */

    history_info = (_HistoryInfo) 0;
    opr_ids_list = (_List) _NullObject;
    opr_names_list = (_List) _NullObject;

    _StartExceptionBlock

    /*
    ** Get the number of steps in the list.
    */

    if (steps == (_List) _NullObject)
	count = 0;
    else
	_GetValue(steps, _P_ElementCount, lwk_c_domain_integer, (_AnyPtr) &count);

    /*
    ** Allocate the HistoryInfo structure.
    */

    if (count > 0) {
	history_info =
	    (_HistoryInfo) _AllocateMem(sizeof(_HistoryInfoInstance) * count);

	history_info->origin_op_cnt = 0;
	history_info->dest_op_cnt = 0;
    }

    /*
    **  Remove the Steps from the Step List
    */

    for (i = 0; i < count; i++) {
	_RemoveElement(steps, lwk_c_domain_step, &history_info[i].step);

	/*
	** Get the number of operations associated with the Origin and
	** build the operation name array.
	*/

	_RemoveElement(origin_opr_ids, lwk_c_domain_list, (_AnyPtr) &opr_ids_list);
	_RemoveElement(origin_opr_names, lwk_c_domain_list, (_AnyPtr) &opr_names_list);

	_GetValue(opr_ids_list, _P_ElementCount, lwk_c_domain_integer,
	    &op_cnt);

	if (op_cnt > 0) {
      	    /*
	    ** Allocate and fill in the operation name String array.
	    */

	    history_info[i].origin_opr_ids =
		(_String *) _AllocateMem(sizeof(_String) * op_cnt);

	    history_info[i].origin_opr_names =
		(_String *) _AllocateMem(sizeof(_String) * op_cnt);

	    for (j = 0; j < op_cnt; j++) {
		_RemoveElement(opr_ids_list, lwk_c_domain_string,
		    &history_info[i].origin_opr_ids[j]);
		_RemoveElement(opr_names_list, lwk_c_domain_ddif_string,
		    &history_info[i].origin_opr_names[j]);
	    }

	    history_info[i].origin_op_cnt = op_cnt;
	}

	/*
	** Delete the operation lists.
	*/

	_Delete(&opr_ids_list);
	_Delete(&opr_names_list);

	/*
	** Get the number of operations associated with the Destination and
	** build the operation name array.
	*/

	_RemoveElement(dest_opr_ids, lwk_c_domain_list, (_AnyPtr) &opr_ids_list);
	_RemoveElement(dest_opr_names, lwk_c_domain_list, (_AnyPtr) &opr_names_list);

	_GetValue(opr_ids_list, _P_ElementCount, lwk_c_domain_integer,
	    &op_cnt);

	if (op_cnt > 0) {
	    /*
	    ** Allocate and fill in the operation name String array.
	    */

	    history_info[i].dest_opr_ids =
		(_String *) _AllocateMem(sizeof(_String) * op_cnt);
	    history_info[i].dest_opr_names =
		(_String *) _AllocateMem(sizeof(_String) * op_cnt);

	    for (j = 0; j < op_cnt; j++) {
		_RemoveElement(opr_ids_list, lwk_c_domain_string,
		    &history_info[i].dest_opr_ids[j]);
		_RemoveElement(opr_names_list, lwk_c_domain_ddif_string,
		    &history_info[i].dest_opr_names[j]);
	    }

	    history_info[i].dest_op_cnt = op_cnt;
	}

	/*
	** Delete the operation lists.
	*/

	_Delete(&opr_ids_list);
	_Delete(&opr_names_list);
    }

    /*
    **  Fill in the SVN Widget with Step information
    */

    DisplayHistory(private, history_info, count);

    history_info = (_HistoryInfo) 0;

    /*
    ** Popup the dialog box and position it.
    */

    if (XtIsManaged(private->history->dialog)) 
	XRaiseWindow(XtDisplay(private->history->dialog),
	    XtWindow(XtParent(private->history->dialog)));
    else {

	if (initial_display) {
	
	    if (XtParent(private->main_widget) != 0)
		shell = XtParent(private->main_widget); /* get the shell */
	    else
		shell = private->main_widget;		/* it is a shell */
	
	    LwkDXmPositionWidget(private->history->dialog, shell);
	}

	XtManageChild(private->history->dialog);
    }

    /*
    ** If there are any Steps and the Step dialog box is visible, update it.
    */

    if (count > 0 && private->step->dialog != (Widget) 0) {
	if (XtIsManaged(private->step->dialog)) {
	    int index;

	    index = private->selected_history_entry / 3;

	    LwkDXmStepDisplay(private->history_info[index].step, private);
	}
    }

    /*
    ** De-iconize the box if necessary
    */

    LwkDXmSetShellIconState(XtParent(private->history->dialog), NormalState);

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&opr_ids_list);
	    _Delete(&opr_names_list);

	    if (history_info != (_HistoryInfo) 0)
		FreeHistoryInfo(history_info, count);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  DisplayHistory(private, history_info, count)
_DXmUiPrivate private;
 _HistoryInfo history_info;

    _Integer count;

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
    Pixmap pixmap;
    _Surrogate origin;
    _Surrogate destination;
    _Integer follow_type;
    _CString follow_type_text;
    _CString volatile origin_desc;
    _CString volatile dest_desc;
						
    /*
    **  Initialization
    */
                   
    origin_desc = (_CString) _NullObject;
    dest_desc = (_CString) _NullObject;

    _StartExceptionBlock

    DXmSvnDisableDisplay(private->history->steps);

    /*
    **  Empty the SVN widget of any old contents (one triplet for each Step).
    */

    if (private->history_info_size > 0) {
	DXmSvnDeleteEntries(private->history->steps, 0,
	    private->history_info_size * 3);

	FreeHistoryInfo(private->history_info, private->history_info_size);
    }

    /*
    **  Build a description of each Step
    */

    for (i = 0; i < count; i++) {
	/*
	** Get the Description of the Origin (else a blank).
	*/

	_GetValue(history_info[i].step, _P_Origin, lwk_c_domain_surrogate,
	    &origin);

	if (origin == (_Surrogate) _NullObject)
	    origin_desc = (_CString) _NullObject;
	else
	    _GetCSValue(origin, _P_Description, &origin_desc);

	if (origin_desc == (_CString) _NullObject)
	    origin_desc = _StringToCString((_String) "");

	/*
	** Get the Description of the Destination (else a blank).
	*/

	_GetValue(history_info[i].step, _P_Destination, lwk_c_domain_surrogate,
	    &destination);

	if (destination == (_Surrogate) _NullObject)
	    dest_desc = (_CString) _NullObject;
	else
	    _GetCSValue(destination, _P_Description, &dest_desc);

	if (dest_desc == (_CString) _NullObject)
	    dest_desc = _StringToCString((_String) "");

	/*
	**  Get the FollowType of the Step
	*/

	_GetValue(history_info[i].step, _P_FollowType, lwk_c_domain_integer,
	    &follow_type);

	if (follow_type == lwk_c_follow_go_to)
	    follow_type_text = (_CString) private->history->goto_literal;
	else if (follow_type == lwk_c_follow_implicit_go_to)
	    follow_type_text = (_CString) private->history->implicit_goto_literal;
	else
	    follow_type_text = (_CString) private->history->visit_literal;

	/*
	**  Set the first entry of this triplet to be the Origin
	*/

	DXmSvnAddEntries(private->history->steps, i * 3, 1, 0,
	    (XtPointer *) 0, TRUE);

	DXmSvnSetEntry(private->history->steps, i * 3 + 1, 0, 0, 2, TRUE, 0,
	    TRUE);

	pixmap = GetSurrogatePixmap(private, origin);

	DXmSvnSetComponentPixmap(private->history->steps, i * 3 + 1, 1, 0, 0,
	    pixmap, _IconWidth, _IconHeight);



	DXmSvnSetComponentText( private->history->steps,/* widget   */
			    i * 3 + 1,			/* entry    */
			    2,				/* comp	    */
			    (_IconWidth * 3) / 2,	/* x	    */
			    0,				/* y	    */
			    (XmString) origin_desc,	/* text	    */
			    (_AnyPtr) 0); 		/* font	    */
	
	/*
	**  Set the next entry of this triplet to be the FollowType
	*/

	DXmSvnAddEntries(private->history->steps, i * 3 + 1, 1, 1,
	    (XtPointer *) 0, TRUE);

	DXmSvnSetEntry(private->history->steps, i * 3 + 2, 0, 0, 2, TRUE, 0,
	    FALSE);

	pixmap = private->step_icon;

	DXmSvnSetComponentPixmap(private->history->steps, i * 3 + 2, 1, 0, 0,
	    pixmap, _IconWidth, _IconHeight);

	
	DXmSvnSetComponentText( private->history->steps,/* widget   */
			    i * 3 + 2,			/* entry    */
			    2,				/* comp	    */
			    (_IconWidth * 3) / 2,	/* x	    */
			    0,				/* y	    */
			    (XmString) follow_type_text,/* text	    */
			    (_AnyPtr) 0);		/* font	    */

	/*
	**  Set the last entry of this triplet to be the Destination
	*/

	DXmSvnAddEntries(private->history->steps, i * 3 + 2, 1, 2,
	    (XtPointer *) 0, TRUE);

	DXmSvnSetEntry(private->history->steps, i * 3 + 3, 0, 0, 2, TRUE, 0,
	    TRUE);

	pixmap = GetSurrogatePixmap(private, destination);

	DXmSvnSetComponentPixmap(private->history->steps, i * 3 + 3, 1, 0, 0,
	    pixmap, _IconWidth, _IconHeight);

	
	DXmSvnSetComponentText( private->history->steps,/* widget   */
			    i * 3 + 3,			/* entry    */
			    2,				/* comp	    */
			    (_IconWidth * 3) / 2,	/* x	    */
			    0,				/* y	    */
			    (XmString) dest_desc,	/* text	    */
			    (_AnyPtr) 0);		/* font	    */

	/*
	**  Free the Origin and Destination Descriptions
	*/

	_DeleteCString(&origin_desc);
	_DeleteCString(&dest_desc);
    }

    /*
    **  Save the new HistoryInfo
    */

    private->history_info_size = count;

    private->history_info = history_info;

    /*
    **	Select the Origin of the last Step (ready for a sort of Undo) and make
    **	sure that it is visible.
    */

    if (count > 0) {
	i = count * 3 - 2;

	private->selected_history_entry = i - 1;

	DXmSvnPositionDisplay(private->history->steps, i, DXmSVN_POSITION_TOP);

	DXmSvnSelectEntry(private->history->steps, i);

	SetButtonSensitivity(private, _True);
    }
    else {
	private->selected_history_entry = -1;

	SetButtonSensitivity(private, _False);
    }

    /*
    **  Repaint the SVN displays
    */

    DXmSvnEnableDisplay(private->history->steps);

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteCString(&origin_desc);
	    _DeleteCString(&dest_desc);
	    _Reraise;
    _EndExceptionBlock

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
**	w: widget.
**	private: dwui private data.
**	reason: reason code.
**
**  Result:
**	None
**
**  Exceptions:
**	DRMHierarchyFetchFailed
**--
*/
static void  CreateSteps(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->steps = w;

    return;
    }

static void  CreateShow(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->show = w;

    return;
    }

static void  CreateVisit(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->visit = w;

    return;
    }

static void  CreateCancel(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->cancel = w;

    return;
    }

static void  CreateButtonsForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->buttonsform = w;

    return;
    }

static void  CreateListForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->listform = w;

    return;
    }

static void  CreateShowForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->showform = w;

    return;
    }

static void  CreateVisitForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->visitform = w;

    return;
    }

static void  CreateOperationMenu(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->history->operations = w;

    return;
    }


static void  ActivateShow(w, private, reason)
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
    ShowMore(private);

    return;
    }


static void  ActivateVisit(w, private, reason)
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
    Visit(private);

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
    ** Unmanage the Show History box.
    */

    XtUnmanageChild(private->history->dialog);

    return;
    }


static void  SelectOperation(menu, private, reason)
Widget menu;
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
    int i;
    int cnt;
    _Boolean found;
    Widget operation;
    Widget *operations;

    operation = ((XmRowColumnCallbackStruct *) reason)->widget;

    cnt = DXmNumChildren((CompositeWidget) menu);
    operations = (Widget *) DXmChildren((CompositeWidget) menu);

    found = _False;

    for (i = 0; i < cnt; i++) {
	if (operation == operations[i]) {
	    found = _True;
	    break;
	}
    }

    if (found)
	private->selected_history_op_entry = i;
    else
	private->selected_history_op_entry = 0;

    return;
    }


static void  SelectStep(w, private, svn)
Widget w;
 _DXmUiPrivate private;
 DXmSvnCallbackStruct *svn;

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
    int old;
    int selected;

    _StartExceptionBlock

    /*
    ** Get the index of the selected Step.
    */

    old = private->selected_history_entry / 3;

    private->selected_history_entry = svn->entry_number - 1;

    selected = private->selected_history_entry / 3;

    /*
    **  Reset the Operations list
    */

    SetOperations(private, private->selected_history_entry);

    /*
    **  Reset the default button
    */

    SetDefaultButton(private);

    /*
    ** If the Step Information box is managed, and needs updating, do it.
    */

    if (private->step->dialog != (Widget) 0) {
    
	if (XtIsManaged(private->step->dialog) && old != selected)
	    LwkDXmStepDisplay(private->history_info[selected].step, private);
	    
	XRaiseWindow(XtDisplay(private->step->dialog),
	    XtWindow(XtParent(private->step->dialog)));
    }

    /*
    ** If any exceptions are raised, display error message
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(unexpected_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ConfirmStep(w, private, svn)
Widget w;
 _DXmUiPrivate private;

    DXmSvnCallbackStruct *svn;

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
    **	If the Origin or Destination is selected, a do a Visit.  If a Step
    **	itself is selected, do a Show More.
    */

    if ((private->selected_history_entry % 3) == 1)
	ShowMore(private);
    else
	Visit(private);

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(unexpected_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  SelectDoneHistory(w, private, svn)
Widget w;
 _DXmUiPrivate private;

    DXmSvnCallbackStruct *svn;

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
    **	If there is no Step selected, or no Operation selected, grey out all
    **	buttons except Cancel
    */

    if (DXmSvnGetNumSelections(private->history->steps) > 0)
	SetButtonSensitivity(private, TRUE);
    else {
	SetButtonSensitivity(private, FALSE);

        /*
	** Set the cancel button to be the default one.
	*/
	ChangeDefaultButton(private, private->history->cancel);
    }

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(unexpected_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  SetOperations(private, selected)
_DXmUiPrivate private;
 int selected;

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
    int index;
    int op_cnt;
    Widget menu;
    Widget entry;
    int entry_cnt;
    Arg arglist[2];
    _Boolean found;
    Widget *entries;
    _String *opr_ids;
    _DDIFString *opr_names;
    _String volatile operation;
    _CString cstring;

    /*
    **  If a Step is selected, no need to change Operations list
    */

    if ((selected % 3) == 1)
	return;

    /*
    **  Get the Operations list for the selected Origin or Destination
    */

    index = selected / 3;

    if ((selected % 3) == 0) {
	op_cnt = private->history_info[index].origin_op_cnt;
	opr_ids = private->history_info[index].origin_opr_ids;
	opr_names = private->history_info[index].origin_opr_names;
    }
    else {
	op_cnt = private->history_info[index].dest_op_cnt;
	opr_ids = private->history_info[index].dest_opr_ids;
	opr_names = private->history_info[index].dest_opr_names;
    }

    /*
    **  Initialize
    */

    operation = (_String) _NullObject;

    _StartExceptionBlock

    /*
    **  Get the Option Menu
    */

    menu = (Widget) 0;

    XtSetArg(arglist[0], XmNsubMenuId, &menu);
    XtGetValues(private->history->operations, arglist, 1);

    /*
    **  Set the Operation names in the Operation Popup Menu
    */

    if (op_cnt > 0 && menu != (Widget) 0) {
	/*
	**  Get the count and list of Menu entries
	*/

	entry_cnt = DXmNumChildren((CompositeWidget) menu);
	entries = (Widget *) DXmChildren((CompositeWidget) menu);

	/*
	**  Set the Operation names in the entries, creating new entries if
	**  required
	*/

	for (i = 0; i < op_cnt; i++) {
	    cstring = _DDIFStringToCString(opr_names[i]);
	
	    XtSetArg(arglist[0], XmNlabelString, cstring);

	    if (i < entry_cnt)
		XtSetValues(entries[i], arglist, 1);
	    else {
		entry = (Widget) XmCreatePushButtonGadget(menu, "", arglist, 1);
		XtManageChild(entry);
	    }
	
	    _DeleteCString(&cstring);
	}

	/*
	**  Delete any unused entries
	*/

	for (i = op_cnt; i < entry_cnt; i++) {
	    XtUnmanageChild(entries[i]);
	    XtDestroyWidget(entries[i]);
	}

	/*
	**  Refetch the count and entries list
	*/

	entry_cnt = DXmNumChildren((CompositeWidget) menu);
	entries = (Widget *) DXmChildren((CompositeWidget) menu);

	/*
	**  Get the Default Operation
	*/

	_GetValue(private->dwui, _P_DefaultOperation, lwk_c_domain_string,
	    (_AnyPtr) &operation);

	/*
	**  Set the Option Menu to the Default Operation or to the first
	**  operation (no default, or the default is not in the list).
	*/

	if (operation == (_String) _NullObject)
	    i = 0;
	else {
	    found = _False;

	    for (i = 0; i < op_cnt; i++)
		if (_CompareString(opr_ids[i], operation) == 0) {
		    found = _True;
		    break;
		}

	    if (!found)
		i = 0;

	    _DeleteString(&operation);
	}

	private->selected_history_op_entry = i;

	XtSetArg(arglist[0], XmNmenuHistory, entries[i]);

	XtSetValues(private->history->operations, arglist, 1);
    }

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&operation);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  Visit(private)
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
    _Step step;
    int op_index;
    int step_index;
    _String operation;
    _Surrogate origin;
    _Surrogate destination;

    _StartExceptionBlock

    /*
    **  Use the currently selected Step
    */

    step_index = private->selected_history_entry / 3;

    step = private->history_info[step_index].step;

    /*
    ** Get the Destination -- this is either the Origin of the Step (if it was
    ** selected), or the Destination of the Step (if it was selected, or if the
    ** Step itself was selected).
    **
    */

    if ((private->selected_history_entry % 3) == 0)
	_GetValue(step, _P_Origin, lwk_c_domain_surrogate, &destination);
    else
	_GetValue(step, _P_Destination, lwk_c_domain_surrogate, &destination);

    /*
    **  Get the selected operation.
    */

    op_index = private->selected_history_op_entry;

    if ((private->selected_history_entry % 3) == 0)
	operation = private->history_info[step_index].origin_opr_ids[op_index];
    else
	operation = private->history_info[step_index].dest_opr_ids[op_index];

    /*
    ** Navigate to the destination.  Because we do not provide an Origin, no
    ** new Step will be added to the History.
    */

    _Navigate(private->dwui, (_Link) _NullObject,
	(_Surrogate) _NullObject, destination, operation, lwk_c_follow_visit);

    /*
    ** If any exceptions are raised, display error message
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(visit_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ShowMore(private)
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
    int index;
    _Boolean initial_display = _False;
    
    _StartExceptionBlock

    /*
    **  Create the Step Dialog Box if necessary
    */

    if (private->step->dialog == (Widget) 0) {
	LwkDXmStepInfoCreate(private);
	initial_display = _True;
    }

    /*
    **  Realize the Dialog Box, if necessary
    */

    if (!XtIsRealized(private->step->dialog)) 
	XtRealizeWidget(private->step->dialog);
    
    /*
    ** Update origin properties in the Step Info dialog box.
    */

    index = private->selected_history_entry / 3;

    LwkDXmStepDisplay(private->history_info[index].step, private);

    /*
    ** Popup the dialog box if not already managed.  
    */

    if (XtIsManaged(private->step->dialog))
	XRaiseWindow(XtDisplay(private->step->dialog),
	    XtWindow(XtParent(private->step->dialog)));
    else {

	if (initial_display) {
	    /* 
	    ** Position the History dialog box with respect to
	    ** the main application window.
	    */
	    
	    Widget ref_widget;
	    
	    if (XtParent(private->main_widget) != 0)
		ref_widget = XtParent(private->main_widget); /* get the shell */
	    else
		ref_widget = private->main_widget;	     /* it is a shell */
	
	    LwkDXmPositionWidget(private->step->dialog, ref_widget);

	}
	
	XtManageChild(private->step->dialog);
    }

    /*
    ** De-iconize the dialog box if needed
    */

    LwkDXmSetShellIconState(XtParent(private->step->dialog), NormalState);

    /*
    ** If any exceptions are raised, display error message
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_step_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  SetDefaultButton(private)
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
    Arg arglist[1];
    Widget button;
    Widget default_button;

    /*
    **	If the Origin or the Destination is selected, make the Visit button the
    **	default.  If a Step itself is selected, make the Show More button the
    **	default.
    */

    if ((private->selected_history_entry % 3) == 1)
	button = private->history->show;	
    else
	button = private->history->visit;	

    /*
    **  Only do the SetValues/SetSensitive if it is not already correctly set.
    */

    XtSetArg(arglist[0], XmNdefaultButton, &default_button);

    XtGetValues(private->history->dialog, arglist, 1);

    if (default_button != button) 
	ChangeDefaultButton(private, button);

    return;
    }


static void  ChangeDefaultButton(private, button)
_DXmUiPrivate private;
 Widget button;

/*
**++
**  Functional Description:
**	Change the default button to button in all forms.
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

    XtSetArg(arglist[0], XmNdefaultButton, button);

    XtSetValues(private->history->dialog, arglist, 1);
    XtSetValues(private->history->listform, arglist, 1);
    XtSetValues(private->history->buttonsform, arglist, 1);
    XtSetValues(private->history->visitform, arglist, 1);
    XtSetValues(private->history->showform, arglist, 1);

    return;
    }


static void  SetButtonSensitivity(private, state)
_DXmUiPrivate private;
 Boolean state;

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
    Boolean old_state;

    /*
    **  If the Sensitivity of the various Buttons needs changing, do it.
    */

    old_state = XtIsSensitive(private->history->show);

    if (state != old_state)
	XtSetSensitive(private->history->show, state);

    /*
    **	Only set Operations Menu and Visit Button sensitive if an Origin or
    **	Destination entry is selected in the Steps list
    */

    old_state = XtIsSensitive(private->history->visit);

    if (!state || (state && ((private->selected_history_entry % 3) != 1))) {
	if (state != old_state) {
	    XtSetSensitive(private->history->visit, state);
	    XtSetSensitive(private->history->operations, state);
	}
    }
    else {
	if (old_state) {
	    XtSetSensitive(private->history->visit, FALSE);
	    XtSetSensitive(private->history->operations, FALSE);
	}
    }

    return;
    }


static void  FreeHistoryInfo(history_info, size)
_HistoryInfo history_info;
 int size;

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

    /*
    **  Delete the operation name Strings
    */

    for (i = 0; i < size; i++) {
	if (history_info[i].origin_op_cnt > 0) {
	    for (j = 0; j < history_info[i].origin_op_cnt; j++) {
		_DeleteString(&history_info[i].origin_opr_ids[j]);
		_DeleteString(&history_info[i].origin_opr_names[j]);
	    }

	    _FreeMem(history_info[i].origin_opr_ids);
	    _FreeMem(history_info[i].origin_opr_names);
	}

	if (history_info[i].dest_op_cnt > 0) {
	    for (j = 0; j < history_info[i].dest_op_cnt; j++) {
		_DeleteString(&history_info[i].dest_opr_ids[j]);
		_DeleteString(&history_info[i].dest_opr_names[j]);
	    }

	    _FreeMem(history_info[i].dest_opr_ids);
	    _FreeMem(history_info[i].dest_opr_names);
	}
    }

    /*
    **  Free the HistoryInfo structure
    */

    _FreeMem(history_info);

    return;
    }


static Pixmap  GetSurrogatePixmap(private, surrogate)
_DXmUiPrivate private;
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
    Pixmap pixmap;
    _Boolean has_incomming;
    _Boolean has_outgoing;

    /*
    **  Find out if this Surrogate is an Origin, a Destination, or both, and
    **	return the appropriate Icon Pixmap.
    */

    has_incomming = (_Boolean) _SetState(surrogate, _StateHasInComming,
	_StateGet);

    has_outgoing = (_Boolean) _SetState(surrogate, _StateHasOutGoing,
	_StateGet);

    if (has_incomming && has_outgoing)
	pixmap = private->origin_and_destination_icon;
    else if (has_incomming)
	pixmap = private->destination_icon;
    else
	pixmap = private->origin_icon;

    return pixmap;
    }
