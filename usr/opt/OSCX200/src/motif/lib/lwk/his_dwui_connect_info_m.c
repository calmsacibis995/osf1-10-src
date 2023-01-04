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
**  Version: V1.0
**
**  Abstract:
**	DECwindows LinkWorks User Interface Complete Link... and
**	Show Link... dialog boxes.
**
**  Keywords:
**	HIS, UI
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
**	     LG	  15-Nov-90	Make push buttons insensitive when not useful
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

_DeclareFunction(static void CreateSourceDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateSourceType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateLinkType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateLinkDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateTargetType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateTargetDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOkButton,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateResetButton,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCancelButton,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateApplyButton,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeSourceDesc,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeLinkType,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ChangeLinkDesc,
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

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkCreateLinkSourceDesc", (caddr_t) CreateSourceDesc},
	{"LwkCreateLinkSourceType", (caddr_t) CreateSourceType},
	{"LwkCreateLinkType", (caddr_t) CreateLinkType},
	{"LwkCreateLinkDesc", (caddr_t) CreateLinkDesc},
	{"LwkCreateLinkTargetType", (caddr_t) CreateTargetType},
	{"LwkCreateLinkTargetDesc", (caddr_t) CreateTargetDesc},
	{"LwkCreateLinkOkButton", (caddr_t) CreateOkButton},
	{"LwkCreateLinkApplyButton", (caddr_t) CreateApplyButton},
	{"LwkCreateLinkResetButton", (caddr_t) CreateResetButton},
	{"LwkCreateLinkCancelButton", (caddr_t) CreateCancelButton},
	{"LwkChangeLinkSourceDesc", (caddr_t) ChangeSourceDesc},
	{"LwkChangeLinkType", (caddr_t) ChangeLinkType},
	{"LwkChangeLinkDesc", (caddr_t) ChangeLinkDesc},
	{"LwkChangeLinkTargetDesc", (caddr_t) ChangeTargetDesc},
	{"LwkActivateLinkOk", (caddr_t) ActivateOk},
	{"LwkActivateLinkApply", (caddr_t) ActivateApply},
	{"LwkActivateLinkReset", (caddr_t) ActivateReset},
	{"LwkActivateLinkCancel", (caddr_t) ActivateCancel}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


void  LwkDXmShowLinkCreate(private)
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


    status = MrmFetchLiteral(hierarchy, "ShowLinkBoxTitle",
	XtDisplay(private->main_widget), (caddr_t *) &cs_template, &type);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    **  Construct the Dialog Box Title from the template and the Application
    **	Name.
    */

    cs_title = XmStringConcat(private->appl_name, cs_template);

    ac = 0;

    XtSetArg(arglist[ac], XmNdialogTitle, cs_title);
    ac++;

    /*
    ** Fetch the Show Link Dialog Box, setting its title
    */

    shell = XtAppCreateShell("ShowLinkBox", "ShowLinkBox",
	topLevelShellWidgetClass, XtDisplay(private->main_widget),
	(Arg *) 0, (int) 0);

    /*
    ** Fetch the box
    */
    status = MrmFetchWidgetOverride (hierarchy, "ShowLinkBox", shell,
	    "ShowLinkBox", arglist, ac,
	    &private->show_link->dialog, (MrmType *) &dummy_class);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(XtParent(private->show_link->dialog),
		      StructureNotifyMask, False,
		      LwkDXmSetIconsOnShell, private);


    /*
    **	Special case setting of multi segment compound string title
    **	for XUI Window Manager for systems that support the XUI WM.
    */

    if (LwkDXmIsXUIWMRunning(private->show_link->dialog)) 
	LwkDXmSetXUIDialogTitle(private->show_link->dialog,
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


void  LwkDXmLinkDisplay(private, link, widgets)
_DXmUiPrivate private;
 _Link link;
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
    Arg arglist[1];
    _Surrogate source;
    _Surrogate target;
    _String volatile string;
    _DDIFString volatile ddifstring;
    _CString volatile cstring;
    XmString volatile xstring;
    _Surrogate psource;
    _Surrogate ptarget;

    /*
    ** Initialization.
    */

    xstring = (XmString) 0;
    cstring = (_CString) _NullObject;
    string = (_String) _NullObject;
    ddifstring = (_DDIFString) _NullObject;

    _StartExceptionBlock

    /*
    ** Get the Source description.
    */

    _GetCSValue(link, _P_SourceDescription, &cstring);

    if (cstring != (_CString) _NullObject) {
	DXmCSTextSetString(widgets->source_desc, (XmString) cstring);
     	_DeleteCString(&cstring);
    }
    else {
	DXmCSTextSetString(widgets->source_desc, cstring);	
	_DeleteCString(&cstring);
    }
	          
    DXmCSTextSetInsertionPosition(widgets->source_desc, (XmTextPosition) 0);

    widgets->source_desc_changed = _False;

    /*
    ** Get the Link's Source.
    */

    _GetValue(link, _P_Source, lwk_c_domain_surrogate, &source);

    if (source != (_Surrogate) _NullObject) {
	/*
	** Get the Source type.
	*/

	_GetValue(source, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &string);

	if (string != (_String) _NullObject) {
	    ddifstring = LwkRegTypeName(string);
	    _DeleteString(&string);
	}
	else
	    ddifstring = _StringToDDIFString("");
    }
    else {
	/* Use the pending source */

        _GetValue(private->dwui, _P_PendingSource, lwk_c_domain_surrogate,
	    	&psource);
		
	string = (_String) _NullObject;
	    
	if (psource != _NullObject) 
	    _GetValue(psource, _P_SurrogateSubType, lwk_c_domain_string,
		(_AnyPtr) &string);

	if (string != (_String) _NullObject) {
	    ddifstring = LwkRegTypeName(string);
	    _DeleteString(&string);
	}
	else
	    ddifstring = _StringToDDIFString("");
    }	
    
    cstring = _DDIFStringToCString(ddifstring);

    XtSetArg(arglist[0], XmNlabelString, (char *) cstring);

    XtSetValues(widgets->source_type, arglist, 1);

    _DeleteCString(&cstring);
    _DeleteDDIFString(&ddifstring);

    /*
    ** Get the Link relationship.
    */

    _GetCSValue(link, _P_RelationshipType, &cstring);

    if (cstring != (_CString) _NullObject) {
	DXmCSTextSetString(widgets->link_type, (XmString)cstring);
	_DeleteCString(&cstring);

	/*
	**  In case insert position was changed by pending delete at popup
    	**  time, reset it to the end of the string.
	*/
	
	DXmCSTextSetInsertionPosition(widgets->link_type,
	    _LengthCString(cstring));
    }
    else {
	xstring = _StringToXmString((char *) "");
	DXmCSTextSetString(widgets->link_type, (XmString) xstring);
	XmStringFree(xstring);

     	DXmCSTextSetInsertionPosition(widgets->link_type,
	    (XmTextPosition) 0);
    }

    widgets->link_type_changed = _False;

    /*
    ** Get the Links description.
    */

    _GetCSValue(link, _P_Description, &cstring);

    if (cstring != (_CString) _NullObject) {
	DXmCSTextSetString(widgets->link_desc, (XmString) cstring);
	_DeleteCString(&cstring);
    }
    else {
	xstring = _StringToXmString((char *) "");
	DXmCSTextSetString (widgets->link_desc, (XmString)xstring);
	XmStringFree(xstring);
    }
	
    DXmCSTextSetInsertionPosition(widgets->link_desc, (XmTextPosition) 0);

    widgets->link_desc_changed = _False;

    /*
    ** Get the Target description.
    */

    _GetCSValue(link, _P_TargetDescription, &cstring);

    if (cstring != (_CString) _NullObject) {
	DXmCSTextSetString(widgets->target_desc, (XmString) cstring);
	_DeleteCString(&cstring);
    }
    else {
	xstring = _StringToXmString((char *) "");
	DXmCSTextSetString (widgets->target_desc, (XmString) xstring);
	XmStringFree(xstring);
    }

    DXmCSTextSetInsertionPosition(widgets->target_desc, (XmTextPosition) 0);
	
    widgets->target_desc_changed = _False;

    /*
    ** Get the Links's Target.
    */

    _GetValue(link, _P_Target, lwk_c_domain_surrogate, &target);

    if (target != (_Surrogate) _NullObject) {
	/*
	** Get the Target type.
	*/

	_GetValue(target, _P_SurrogateSubType, lwk_c_domain_string,
	    (_AnyPtr) &string);

	if (string != (_String) _NullObject) {
	    ddifstring = LwkRegTypeName(string);
	    _DeleteString(&string);
	}
	else
	    ddifstring = _StringToDDIFString("");
    }
    else {
	/*
	** Use the Pending Target type.
	*/

	string = (_String) _NullObject;
	    
	_GetValue(private->dwui, _P_PendingTarget, lwk_c_domain_surrogate,
	    &ptarget);

	if (ptarget != _NullObject) 
	    _GetValue(ptarget, _P_SurrogateSubType, lwk_c_domain_string,
		(_AnyPtr) &string);

	if (string != (_String) _NullObject) {
	    ddifstring = LwkRegTypeName(string);
	    _DeleteString(&string);
	}
	else
	    ddifstring = _StringToDDIFString("");
    }
    
    cstring = _DDIFStringToCString(ddifstring);
    XtSetArg(arglist[0], XmNlabelString, (char *) cstring);

    XtSetValues(widgets->target_type, arglist, 1);

    _DeleteDDIFString(&ddifstring);
    _DeleteCString(&cstring);

    /*
    **  Make sure the dialog box is visible
    */
    
    XRaiseWindow(XtDisplay(widgets->dialog), 
	XtWindow(XtParent(widgets->dialog)));

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


void  LwkDXmLinkUpdate(link, widgets)
_Link link;
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
    _Boolean changed;
    _String volatile string;
    _DDIFString volatile ddifstring;
    XmString volatile xstring;
								
    /*
    **  Initialization
    */

    string = (_String) _NullObject;
    ddifstring = (Opaque) _NullObject;
    changed = _False;

    _StartExceptionBlock

    /*
    ** Set the Source description if it was changed.
    */

    if (widgets->source_desc_changed) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString(widgets->source_desc);
	ddifstring = _CStringToDDIFString(xstring);
	XmStringFree(xstring);

	_SetValue(link, _P_SourceDescription, lwk_c_domain_ddif_string,
	    (_AnyPtr) &ddifstring, lwk_c_set_property);

	_DeleteDDIFString(&ddifstring);

	widgets->source_desc_changed = _False;
    }

    /*
    ** Set the Link relationship type if it was changed.
    */
                 
    if (widgets->link_type_changed) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString (widgets->link_type);
	ddifstring = _CStringToDDIFString(xstring);
        XmStringFree(xstring);
	
	_SetValue(link, _P_RelationshipType, lwk_c_domain_ddif_string,
	    (_AnyPtr) &ddifstring, lwk_c_set_property);

	_DeleteDDIFString(&ddifstring);

	widgets->link_type_changed = _False;
    }

    /*
    ** Set the Link description if it was changed.
    */
                 
    if (widgets->link_desc_changed) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString(widgets->link_desc);
	ddifstring = _CStringToDDIFString(xstring);
        XmStringFree(xstring);

	_SetValue(link, _P_Description, lwk_c_domain_ddif_string,
	    (_AnyPtr) &ddifstring, lwk_c_set_property);

	_DeleteDDIFString(&ddifstring);

	widgets->link_desc_changed = _False;
    }

    /*
    ** Set the Target description if it was changed.
    */

    if (widgets->target_desc_changed) {
	changed = _True;

	xstring = (XmString) DXmCSTextGetString(widgets->target_desc);
	ddifstring = _CStringToDDIFString(xstring);
        XmStringFree(xstring);

	_SetValue(link, _P_TargetDescription, lwk_c_domain_ddif_string,
	    (_AnyPtr) &ddifstring, lwk_c_set_property);

	_DeleteDDIFString(&ddifstring);
                
	widgets->target_desc_changed = _False;
    }

    /*                                       
    **  If any properties changed, Store the Linknet containing this
    **  Link so that the changes will get written to the Linkbase.
    */

    if (changed) {
	_Linknet linknet;
	_Linkbase linkbase;

	_GetValue(link, _P_Linknet, lwk_c_domain_linknet, &linknet);

	if (linknet != (_Linknet) _NullObject) {
	    _GetValue(linknet, _P_Linkbase, lwk_c_domain_linkbase,
		&linkbase);

	    if (linkbase != (_Linkbase) _NullObject)
		_Store(linknet, linkbase);
	}
    }

    /*
    ** If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (xstring != (XmString) 0)
		XmStringFree(xstring);

	    _DeleteDDIFString(&ddifstring);
	    _DeleteString(&string);

	    _Reraise;
    _EndExceptionBlock

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
static void  CreateSourceDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->source_desc = w;

    return;
    }

static void  CreateSourceType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->source_type = w;

    return;
    }

static void  CreateLinkType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->link_type = w;

    return;
    }

static void  CreateLinkDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->link_desc = w;

    return;
    }

static void  CreateTargetType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->target_type = w;

    return;
    }

static void  CreateTargetDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->target_desc = w;

    return;
    }


static void  CreateOkButton(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->ok_button = w;

    return;
    }

static void  CreateApplyButton(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->apply_button = w;

    return;
    }

static void  CreateResetButton(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->reset_button = w;

    return;
    }

static void  CreateCancelButton(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->cancel_button = w;

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
static void  ChangeSourceDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->source_desc_changed = _True;
    if (private->show_link->source_desc != 0)
	LwkDXmShowLinkBtnSensitivity(private->show_link, _True);    

    return;
    }

static void  ChangeLinkType(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->link_type_changed = _True;

    if (private->show_link->link_type != 0)
	LwkDXmShowLinkBtnSensitivity(private->show_link, _True);    

    return;
    }

static void  ChangeLinkDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->link_desc_changed = _True;

    if (private->show_link->link_desc != 0)
	LwkDXmShowLinkBtnSensitivity(private->show_link, _True);    

    return;
    }

static void  ChangeTargetDesc(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->show_link->target_desc_changed = _True;

    if (private->show_link->target_desc != 0)
	LwkDXmShowLinkBtnSensitivity(private->show_link, _True);    

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

    XtUnmanageChild(private->show_link->dialog);

    /*
    **  Apply any changes to Link properties
    */
                              
    index = private->selected_link_entry / 2;
                              
    LwkDXmLinkUpdate(private->link_info[index].link,
	private->show_link);

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
    **  Apply any changes to Link properties
    */

    int index;

    index = private->selected_link_entry / 2;

    LwkDXmLinkUpdate(private->link_info[index].link,
	private->show_link);


    /*
    ** Because we have saved these changes already, make the
    ** Ok, Reset, and Apply buttons insensitve until the
    ** user makes a change.
    */

    LwkDXmShowLinkBtnSensitivity(private->show_link, _False);    

    LwkDXmShowLinkCancelBtnDef(private->show_link);


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
    ** Set the Link box properties to their initial state.
    */

    int index;

    index = private->selected_link_entry / 2;

    LwkDXmLinkDisplay(private, private->link_info[index].link,
	private->show_link);

    /*
    ** Because we have saved these changes already, make the
    ** Ok, Reset, and Apply buttons insensitive until the
    ** user makes a change.
    */

    LwkDXmShowLinkBtnSensitivity(private->show_link, _False);    

    LwkDXmShowLinkCancelBtnDef(private->show_link);


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
    /*
    ** Unmanage the Show Link dialog box.
    */

    XtUnmanageChild(private->show_link->dialog);

    return;
}


void  LwkDXmShowLinkBtnSensitivity(widgets, sensitive)
_LinkWidgets widgets;

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

void  LwkDXmShowLinkCancelBtnDef(widgets)
_LinkWidgets widgets;

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
