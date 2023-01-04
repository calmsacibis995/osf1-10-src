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
**	LinkWorks Services User Interface.
**
**  Version: X0.1
**
**  Abstract:
**	DECwindows LWK User Interface Step Information dialog box.
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
**  Forward Routine Declarations
*/

_DeclareFunction(static void CreateOriginDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOriginType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateStepDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateStepTypeGoTo,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateStepTypeVisit,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOperationMenu,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateDestType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateDestDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOk,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateApply,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateReset,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeOriginDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeStepDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeStepType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void SelectOperation,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeDestDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateOk,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateApply,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateReset,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void SetOperations,
    (_DXmUiPrivate private, _Step step));
_DeclareFunction(static void PushBtnSensitivity,
    (_StepWidgets widgets, _Boolean sensitive));
_DeclareFunction(static void CancelBtnDefault,
    (_StepWidgets widgets));

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkCreateStepOriginDesc", (caddr_t) CreateOriginDesc},
	{"LwkCreateStepOriginType", (caddr_t) CreateOriginType},
	{"LwkCreateStepDesc", (caddr_t) CreateStepDesc},
	{"LwkCreateStepGoTo", (caddr_t) CreateStepTypeGoTo},
	{"LwkCreateStepVisit", (caddr_t) CreateStepTypeVisit},
	{"LwkCreateStepOperationMenu", (caddr_t) CreateOperationMenu},
	{"LwkCreateStepDestType", (caddr_t) CreateDestType},
	{"LwkCreateStepDestDesc", (caddr_t) CreateDestDesc},
	{"LwkCreateStepOk", (caddr_t) CreateOk},
	{"LwkCreateStepApply", (caddr_t) CreateApply},
	{"LwkCreateStepCancel", (caddr_t) CreateCancel},
	{"LwkCreateStepReset", (caddr_t) CreateReset},
	{"LwkChangeStepOriginDesc", (caddr_t) ChangeOriginDesc},
	{"LwkChangeStepDesc", (caddr_t) ChangeStepDesc},
	{"LwkChangeStepFollowType", (caddr_t) ChangeStepType},
	{"LwkChangeStepDestDesc", (caddr_t) ChangeDestDesc},
	{"LwkSelectStepOperation", (caddr_t) SelectOperation},
	{"LwkActivateStepOk", (caddr_t) ActivateOk},
	{"LwkActivateStepApply", (caddr_t) ActivateApply},
	{"LwkActivateStepReset", (caddr_t) ActivateReset},
	{"LwkActivateStepCancel", (caddr_t) ActivateCancel}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


void  LwkDXmStepInfoCreate(private)
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
    Arg arglist[2];
    MrmType *dummy_class;
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
    ** Fetch the Step Info dialog box.
    */

    shell = XtAppCreateShell("StepBox", "StepBox",
	topLevelShellWidgetClass, XtDisplay(private->main_widget),
	(Arg *) 0, (int) 0);

    /*
    ** Fetch the box
    */
    if (MrmFetchWidget(hierarchy, "StepBox", shell,
	    &private->step->dialog, (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(XtParent(private->step->dialog), StructureNotifyMask,
		      False, LwkDXmSetIconsOnShell, private);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }


void  LwkDXmFreeStepInfo(private)
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
    int i;

    /*
    **  Free any Step Info we have saved away
    */

    if (private->step_info != (_StepInfo) 0) {
	if (private->step_info->op_cnt > 0) {
	    for (i = 0; i < private->step_info->op_cnt; i++) {
		_DeleteString(&private->step_info->opr_ids[i]);
		_DeleteDDIFString(&private->step_info->opr_names[i]);
	    }

	    _FreeMem(private->step_info->opr_ids);
	    _FreeMem(private->step_info->opr_names);
	}

	_FreeMem(private->step_info);

	private->step_info = (_StepInfo) 0;
    }

    return;
    }


void  LwkDXmStepDisplay(step, private)
_Step step;
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
    _Integer integer;
    _Surrogate origin;
    _Surrogate destination;
    _String volatile string;
    _CString volatile cstring;
    _DDIFString volatile ddifstring;
    XmString volatile xstring;
    	
    /*
    ** Initialization.
    */

    xstring = (XmString) 0;
    string = (_String) _NullObject;
    cstring = (_CString) _NullObject;
    ddifstring = (_DDIFString) _NullObject;

    _StartExceptionBlock

    /*
    ** Get the Step's Origin.
    */

    _GetValue(step, _P_Origin, lwk_c_domain_surrogate, &origin);

    if (origin != (_Surrogate) _NullObject) {
	/*
	** Get the Origin description.
	*/

	_GetCSValue(origin, _P_Description, &cstring);

	if (cstring != (_CString) _NullObject) {
	    DXmCSTextSetString(private->step->origin_desc, (XmString) cstring);
	    _DeleteCString(&cstring);
	}
	else {
	    xstring = _StringToXmString((char *) "");
	    DXmCSTextSetString(private->step->origin_desc, (XmString) xstring);
	    XmStringFree(xstring);
	}
    }
    else {
	xstring = _StringToXmString((char *) "");
	DXmCSTextSetString(private->step->origin_desc, (XmString) xstring);
	XmStringFree(xstring);
    }

    DXmCSTextSetInsertionPosition(private->step->origin_desc,
	(XmTextPosition) 0);
    
	
    private->step->origin_desc_changed = _False;

    if (origin != (_Surrogate) _NullObject) {
	/*
	** Get the Origin type.
	*/

	_GetValue(origin, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &string);

	if (string != (_String) _NullObject) {
	    ddifstring = LwkRegTypeName(string);
	    _DeleteString(&string);
	}
	else
	    ddifstring = _StringToDDIFString("");
    }
    else
	ddifstring = _StringToDDIFString("");

    cstring = _DDIFStringToCString(ddifstring);

    XtSetArg(arglist[0], XmNlabelString, (XmString) cstring);

    XtSetValues(private->step->origin_type, arglist, 1);

    _DeleteCString(&cstring);
    _DeleteDDIFString(&ddifstring);

    /*
    ** Get the Step description.
    */

    _GetCSValue(step, _P_Description, &cstring);

    if (cstring != (_CString) _NullObject) {
	DXmCSTextSetString(private->step->step_desc, (XmString) cstring);
	_DeleteCString(&cstring);
    }
    else {
	xstring = _StringToXmString((char *) "");
	DXmCSTextSetString(private->step->step_desc, (XmString) xstring);
	XmStringFree(xstring);
    }
	
    DXmCSTextSetInsertionPosition(private->step->step_desc,
	(XmTextPosition) 0);

    private->step->step_desc_changed = _False;

    /*
    **  Get the Step Follow Type
    */

    _GetValue(step, _P_FollowType, lwk_c_domain_integer, &integer);

    if (integer == (_Integer) lwk_c_follow_visit) {
	XmToggleButtonGadgetSetState(private->step->follow_go_to, FALSE, FALSE);
	XmToggleButtonGadgetSetState(private->step->follow_visit, TRUE, FALSE);
    }
    else {
	XmToggleButtonGadgetSetState(private->step->follow_go_to, TRUE, FALSE);
	XmToggleButtonGadgetSetState(private->step->follow_visit, FALSE, FALSE);
    }

    private->step->follow_type_changed = _False;

    /*
    ** Set the Step operation.
    */

    SetOperations(private, step);

    private->step->operation_changed = _False;

    /*
    ** Get the Step's Destination.
    */

    _GetValue(step, _P_Destination, lwk_c_domain_surrogate, &destination);

    if (destination != (_Surrogate) _NullObject) {
	/*
	** Get the Destination description.
	*/

	_GetCSValue(destination, _P_Description, &cstring);

	if (cstring != (_CString) _NullObject) {
	    DXmCSTextSetString(private->step->dest_desc, (XmString) cstring);
	    _DeleteCString(&cstring);
	}
	else {
	    xstring = _StringToXmString((char *) "");
	    DXmCSTextSetString(private->step->dest_desc, (XmString) xstring);
	    XmStringFree(xstring);
	}
    }
    else {
	xstring = _StringToXmString((char *) "");
	DXmCSTextSetString(private->step->dest_desc, (XmString) xstring);
	XmStringFree(xstring);
    }

    DXmCSTextSetInsertionPosition(private->step->dest_desc,
	(XmTextPosition) 0);

    private->step->dest_desc_changed = _False;

    if (destination != (_Surrogate) _NullObject) {
	/*
	** Get the Destination type.
	*/

	_GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &string);

	if (string != (_String) _NullObject) {
	    ddifstring = LwkRegTypeName(string);
	    _DeleteString(&string);
	}
	else
	    ddifstring = _StringToDDIFString("");
    }
    else
	ddifstring = _StringToDDIFString("");

    cstring = _DDIFStringToCString(ddifstring);

    XtSetArg(arglist[0], XmNlabelString, (XmString) cstring);
    XtSetValues(private->step->dest_type, arglist, 1);

    _DeleteCString(&cstring);
    _DeleteDDIFString(&ddifstring);

    /*
    **  Make sure the dialog box is visible
    */

/*    LwkDXmSetShellIconState(XtParent(private->step->dialog),		    */
/*    NormalState); */

/*    XRaiseWindow(XtDisplay(private->step->dialog),
	XtWindow(XtParent(private->step->dialog)));
*/
    /*
    ** Because we have saved these changes already, make the
    ** Ok, Reset, and Apply buttons insensitve until the
    ** user makes a change.  Make Cancel the default button.
    */

    PushBtnSensitivity(private->step, _False);

    CancelBtnDefault(private->step);
    
    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
    	_WhenOthers
	    if (xstring != (XmString) 0)
		XmStringFree(xstring);

	    _DeleteString(&string);
	    _DeleteCString(&cstring);
	    _DeleteDDIFString(&ddifstring);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


void  LwkDXmStepUpdate(step, private)
_Step step;
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
    _String string;
    _Boolean changed;
    _Surrogate origin;
    _Surrogate dest;
    _DDIFString ddifstring;
    XmString volatile xstring;
    long byte_count;
    long status;

    /*
    **  Initialization
    */

    xstring = (XmString) 0;
    changed = _False;

    _StartExceptionBlock

    /*
    ** Get the Step's Origin.
    */

    _GetValue(step, _P_Origin, lwk_c_domain_surrogate, &origin);

    /*
    ** Set the Origin description if it was changed.
    */

    if (private->step->origin_desc_changed
	    && origin != (_Surrogate) _NullObject) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString(private->step->origin_desc);
	ddifstring = _CStringToDDIFString(xstring);

	_SetValue(origin, _P_Description, lwk_c_domain_ddif_string, &ddifstring,
	    lwk_c_set_property);

	XmStringFree(xstring);
	xstring = (XmString) 0;
	_DeleteDDIFString(&ddifstring);

	private->step->origin_desc_changed = _False;
    }

    /*
    ** Set the Step description if it was changed.
    */

    if (private->step->step_desc_changed) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString(private->step->step_desc);
	ddifstring = _CStringToDDIFString(xstring);

	_SetValue(step, _P_Description, lwk_c_domain_ddif_string, &ddifstring,
	    lwk_c_set_property);

	XmStringFree(xstring);
	xstring = (XmString) 0;
	_DeleteDDIFString(&ddifstring);

	private->step->step_desc_changed = _False;
    }

    /*
    ** Set the Step Operation if it was changed.
    */

    if (private->step->operation_changed) {
	changed = _True;

	string = private->step_info->opr_ids[private->selected_step_op_entry];

	_SetValue(step, _P_DestinationOperation, lwk_c_domain_string, &string,
	    lwk_c_set_property);

	private->step->operation_changed = _False;
    }

    /*
    ** Set the Step Follow Type if it was changed.
    */

    if (private->step->follow_type_changed) {
	Boolean state;
	_Integer integer;

	changed = _True;

	state = XmToggleButtonGadgetGetState(private->step->follow_go_to);

	if (state)
	    integer = (_Integer) lwk_c_follow_go_to;
	else
	    integer = (_Integer) lwk_c_follow_visit;

	_SetValue(step, _P_FollowType, lwk_c_domain_integer, &integer,
	    lwk_c_set_property);

	private->step->follow_type_changed = _False;
    }

    /*
    ** Get the Step's Destination.
    */

    _GetValue(step, _P_Destination, lwk_c_domain_surrogate, &dest);

    /*
    ** Set the Destination description if it was changed.
    */

    if (private->step->dest_desc_changed && dest != (_Surrogate) _NullObject) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString(private->step->dest_desc);
	ddifstring = _CStringToDDIFString(xstring);

	_SetValue(dest, _P_Description, lwk_c_domain_ddif_string, &ddifstring,
	    lwk_c_set_property);

	XmStringFree(xstring);
	xstring = (XmString) 0;
	_DeleteDDIFString(&ddifstring);

	private->step->dest_desc_changed = _False;
    }

    /*
    **	If any properties changed, Store the Path containing this Step so that
    **	the changes will get written to the Repository.
    */

    if (changed) {
	_Path path;
	_Linkbase repository;

	_GetValue(step, _P_Path, lwk_c_domain_path, &path);

	if (path != (_Path) _NullObject) {
	    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &repository);

	    if (repository != (_Linkbase) _NullObject)
		_Store(path, repository);
	}
    }

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (xstring != (XmString) 0)
		XtFree((_Void *) xstring);
	    _DeleteDDIFString(&ddifstring);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


/*
**++
**  Functional Description:
**	Create widget callbacks grouped here
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
static void  CreateOriginDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->origin_desc = w;
    return;
    }

static void  CreateOriginType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->origin_type = w;
    return;
    }

static void  CreateStepDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->step_desc = w;
    return;
    }

static void  CreateOperationMenu(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->operation = w;
    return;
    }

static void  CreateStepTypeGoTo(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->follow_go_to = w;
    return;
    }

static void  CreateStepTypeVisit(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->follow_visit = w;
    return;
    }

static void  CreateDestType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->dest_type = w;
    return;
    }

static void  CreateDestDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->dest_desc = w;
    return;
    }


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
static void  CreateOk(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->ok_button = w;
    return;
    }

static void  CreateApply(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->apply_button = w;
    return;
    }

static void  CreateCancel(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->cancel_button = w;
    return;
    }

static void  CreateReset(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->reset_button = w;
    return;
    }


/*
**++
**  Functional Description:
**	Value change callback routines grouped here.
**
**--
*/

static void  ChangeOriginDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    if (private->step->origin_desc != 0) {
	private->step->origin_desc_changed = _True;
	PushBtnSensitivity(private->step, _True);
    }
    return;
    }

static void  ChangeStepDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    if (private->step->step_desc != 0) {
	private->step->step_desc_changed = _True;
	PushBtnSensitivity(private->step, _True);
    }
    return;
    }

static void  ChangeStepType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->step->follow_type_changed = _True;
    PushBtnSensitivity(private->step, _True);
    return;
    }

static void  ChangeDestDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    if (private->step->dest_desc != 0) {
	private->step->dest_desc_changed = _True;
	PushBtnSensitivity(private->step, _True);
    }
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
	private->selected_step_op_entry = i;
    else
	private->selected_step_op_entry = 0;

    private->step->operation_changed = _True;

    PushBtnSensitivity(private->step, _True);

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
    int index;

    _StartExceptionBlock

    /*
    ** Unmanage the dialog box.
    */

    XtUnmanageChild(private->step->dialog);

    /*
    **  Apply any changes to Step properties
    */

    index = private->selected_history_entry / 3;

    LwkDXmStepUpdate(private->history_info[index].step, private);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(step_update_error);
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
    int index;

    _StartExceptionBlock

    /*
    **  Apply any changes to Step properties
    */

    index = private->selected_history_entry / 3;

    LwkDXmStepUpdate(private->history_info[index].step, private);

    /*
    ** Because we have saved these changes already, make the
    ** Ok, Reset, and Apply buttons insensitve until the
    ** user makes a change.  Make Cancel the default button.
    */
    PushBtnSensitivity(private->step, _False);

    CancelBtnDefault(private->step);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(step_update_error);
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
    int index;

    _StartExceptionBlock

    /*
    ** Set the step info box properties to their initial state.
    */

    index = private->selected_history_entry / 3;

    LwkDXmStepDisplay(private->history_info[index].step, private);

    /*
    ** Because we have saved these changes already, make the
    ** Ok, Reset, and Apply buttons insensitve until the
    ** user makes a change.  Make Cancel the default button.
    */
    PushBtnSensitivity(private->step, _False);

    CancelBtnDefault(private->step);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(step_update_error);
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
    ** Unmanage the Step Info dialog box.
    */

    XtUnmanageChild(private->step->dialog);

    return;
}


static void  SetOperations(private, step)
_DXmUiPrivate private;
 _Step step;

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
    Widget menu;
    Widget entry;
    int entry_cnt;
    Arg arglist[2];
    _Boolean found;
    Widget *entries;
    _Surrogate destination;
    _String volatile subtype;
    _String volatile operation;
    _CString cstring;
    XmString xstring;

    /*
    **  Initialize
    */

    subtype = (_String) _NullObject;
    operation = (_String) _NullObject;

    _StartExceptionBlock

    /*
    ** Free any old Operations
    */

    LwkDXmFreeStepInfo(private);

    /*
    **  Get the Operations list for the Destination
    */

    _GetValue(step, _P_Destination, lwk_c_domain_surrogate, &destination);

    if (destination != (_Surrogate) _NullObject)
	_GetValue(destination, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &subtype);

    if (subtype != (_String) _NullObject) {
	private->step_info =
	    (_StepInfo) _AllocateMem(sizeof(_StepInfoInstance));

	private->step_info->op_cnt = LwkRegTypeOperations(subtype,
	    &private->step_info->opr_ids);

	private->step_info->opr_names = (_DDIFString *)
	    _AllocateMem(private->step_info->op_cnt * sizeof(_DDIFString));

	for (i = 0; i < private->step_info->op_cnt; i++)
	    private->step_info->opr_names[i] = LwkRegOperationName(subtype,
		private->step_info->opr_ids[i]);

	_DeleteString(&subtype);
    }

    /*
    **  Get the Option Menu
    */

    menu = (Widget) 0;

    XtSetArg(arglist[0], XmNsubMenuId, &menu);
    XtGetValues(private->step->operation, arglist, 1);

    /*
    **  Set the Operation names in the Operation Popup Menu
    */

    if (private->step_info->op_cnt > 0 && menu != (Widget) 0) {
	/*
	**  Get the count and list of Menu entries
	*/

	entry_cnt = DXmNumChildren((CompositeWidget) menu);
	entries = (Widget *) DXmChildren((CompositeWidget) menu);

	/*
	**  Set the Operation names in the entries, creating new entries if
	**  required
	*/

	cstring = 0;
	
	for (i = 0; i < private->step_info->op_cnt; i++) {
	    cstring = _DDIFStringToCString(private->step_info->opr_names[i]);
	    XtSetArg(arglist[0], XmNlabelString, cstring);

	    if (i < entry_cnt)
		XtSetValues(entries[i], arglist, 1);
	    else {
		entry = (Widget) XmCreatePushButtonGadget(menu, "", arglist, 1);
		XtManageChild(entry);
	    }

	    if(cstring != 0) _DeleteCString(&cstring);
	}

	/*
	**  Delete any unused entries
	*/

	for (i = private->step_info->op_cnt; i < entry_cnt; i++) {
	    XtUnmanageChild(entries[i]);
	    XtDestroyWidget(entries[i]);
	}

	/*
	**  Refetch the count and entries list
	*/

	entry_cnt = DXmNumChildren((CompositeWidget) menu);
	entries = (Widget *) DXmChildren((CompositeWidget) menu);

	/*
	**  Get the Destination Operation
	*/

	_GetValue(step, _P_DestinationOperation, lwk_c_domain_string,
	    (_AnyPtr) &operation);

	/*
	**  Set the Option Menu to the Destination Operation or to the first
	**  operation (if Destination Operation is not in the list).
	*/

	if (operation == (_String) _NullObject)
	    i = 0;
	else {
	    found = _False;

	    for (i = 0; i < private->step_info->op_cnt; i++)
		if (_CompareString(private->step_info->opr_ids[i], operation)
			== 0) {
		    found = _True;
		    break;
		}

	    if (!found)
		i = 0;

	    _DeleteString(&operation);
	}

	private->selected_step_op_entry = i;

	XtSetArg(arglist[0], XmNmenuHistory, entries[i]);

	XtSetValues(private->step->operation, arglist, 1);
    }

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&subtype);
	    _DeleteCString(&cstring);
	    _DeleteString(&operation);
	    _Reraise;
    _EndExceptionBlock

    return;
    }

static void  PushBtnSensitivity(widgets, sensitive)
_StepWidgets widgets;
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


static void  CancelBtnDefault(widgets)
_StepWidgets widgets;

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
