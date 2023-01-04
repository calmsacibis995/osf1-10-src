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
**	LWK DXm User Interface Show Links dialog box.
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
**	BL6  ap	  8-Nov-89  -- change the name of the show links dialog
**	                       box, probably a bug in DRM-UIL
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
                                    
_DeclareFunction(static void DisplayLinks,
    (_DXmUiPrivate private, _LinkInfo link_info, int count));
_DeclareFunction(static void CreateLinks,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateOperationMenu,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateShow,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateDelete,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateGoTo,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateVisit,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateListForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateButtonsForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateNavigateForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreateEditForm,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateUpdate,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateShow,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateDelete,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ConfirmDelete,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateGoTo,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateVisit,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCancel,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void SelectOperation,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void SelectLink,
    (Widget w, _DXmUiPrivate private, DXmSvnCallbackStruct *svn));
_DeclareFunction(static void ConfirmLink,
    (Widget w, _DXmUiPrivate private, DXmSvnCallbackStruct *svn));
_DeclareFunction(static void SelectDoneLinks,
    (Widget w, _DXmUiPrivate private, DXmSvnCallbackStruct *svn));
_DeclareFunction(static void SetOperations, (_DXmUiPrivate private, int index));
_DeclareFunction(static void Follow,
    (_DXmUiPrivate private, _FollowType volatile follow_type));
_DeclareFunction(static void Update, (_DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ShowMore, (_DXmUiPrivate private));
_DeclareFunction(static void DeleteLink, (_DXmUiPrivate private));
_DeclareFunction(static void SetDefaultButton, (_DXmUiPrivate private));
_DeclareFunction(static void ChangeDefaultButton,
    (_DXmUiPrivate private, Widget button));
_DeclareFunction(static void SetProcessTraversal, (_DXmUiPrivate private));
_DeclareFunction(static void SetButtonSensitivity,
    (_DXmUiPrivate private, Boolean state));
_DeclareFunction(static void FreeLinkInfo,
    (_LinkInfo link_info, int size));
_DeclareFunction(static Pixmap GetSurrogatePixmap,
    (_DXmUiPrivate private, _Surrogate surrogate,
	_Boolean out_going_link));
_DeclareFunction(static _Termination FindLinknet,
    (_Linknet linknet1, _CompLinknet cnet, _Domain domain,
	_Linknet *linknet2));
_DeclareFunction(void LwkDXmHelpSvnHelpRequested,
    (Widget w, _CString topic, DXmSvnCallbackStruct *cb));

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkCreateShowLinksLinks", (caddr_t) CreateLinks},
	{"LwkCreateShowLinksOperationMenu", (caddr_t) CreateOperationMenu},
	{"LwkCreateShowLinksShow", (caddr_t) CreateShow},
	{"LwkCreateShowLinksDelete", (caddr_t) CreateDelete},
	{"LwkCreateShowLinksCancel", (caddr_t) CreateCancel},
	{"LwkCreateShowLinksGoTo", (caddr_t) CreateGoTo},
	{"LwkCreateShowLinksVisit", (caddr_t) CreateVisit},
	{"LwkCreateShowLinksListForm", (caddr_t) CreateListForm},
	{"LwkCreateShowLinksButtonsForm", (caddr_t) CreateButtonsForm},
	{"LwkCreateShowLinksNavigateForm", (caddr_t) CreateNavigateForm},
	{"LwkCreateShowLinksEditForm", (caddr_t) CreateEditForm},
	{"LwkActivateShowLinksUpdate", (caddr_t) ActivateUpdate},
	{"LwkActivateShowLinksShow", (caddr_t) ActivateShow},
	{"LwkActivateShowLinksDelete", (caddr_t) ActivateDelete},
	{"LwkActivateShowLinksGoTo", (caddr_t) ActivateGoTo},
	{"LwkActivateShowLinksVisit", (caddr_t) ActivateVisit},
	{"LwkActivateShowLinksCancel", (caddr_t) ActivateCancel},
	{"LwkSelectShowLinksLink", (caddr_t) SelectLink},
	{"LwkConfirmShowLinksDelete", (caddr_t) ConfirmDelete},
	{"LwkConfirmShowLinksLink", (caddr_t) ConfirmLink},
	{"LwkSelectShowLinksOperation", (caddr_t) SelectOperation},
	{"LwkSelectShowLinksDone", (caddr_t) SelectDoneLinks},
	{"LwkSvnHelpRequested", (caddr_t) LwkDXmHelpSvnHelpRequested}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


void  LwkDXmShowLinksCreate(private)
_DXmUiPrivate private;

/*
**++
**  Functional Description:
**	Create the Show Links dialog box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**
**  Result:
**	None
**
**  Exceptions:
**	DRMHierarchyFetchFailed
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

    status = MrmFetchLiteral(hierarchy, "ShowLinksBoxTitle",
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
    ** Fetch the Show Links Dialog Box, setting its title
    */

    status = MrmFetchWidgetOverride(hierarchy, "ShowLinksBox",
	private->main_widget, "ShowLinksBox", arglist, ac,
	&private->links->dialog, (MrmType *) &dummy_class);
		
    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(XtParent(private->links->dialog), StructureNotifyMask,
		      False, LwkDXmSetIconsOnShell, private);
    
    /*
    ** Fetch the Delete Link Caution Box
    */

    status = MrmFetchWidgetOverride(hierarchy, "DeleteLinkCautionBox", 
	private->links->dialog, "DeleteLinkCautionBox", arglist, ac,
	&private->links->caution, (MrmType *) &dummy_class);
		
    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    **	Special case setting of multi segment compound string title
    **	for XUI Window Manager.
    */

    if (LwkDXmIsXUIWMRunning(private->links->dialog)) { 
	LwkDXmSetXUIDialogTitle(private->links->dialog,
	    private->appl_name, cs_template);
	LwkDXmSetXUIDialogTitle(private->links->caution,
	    private->appl_name, cs_template);
    };

    /*
    **  Clean up
    */

    XmStringFree(cs_title);
    XmStringFree(cs_template);

    /*
    **  Create the assorted Pixmaps if necessary
    */

    if (private->link_in_icon == (Pixmap) 0) {
	screen = XtScreen(private->links->dialog);
	display = XtDisplay(private->links->dialog);

	ac = 0;

	XtSetArg(arglist[ac], XmNforeground, &foreground);
	ac++;
	XtSetArg(arglist[ac], XmNbackground, &background);
	ac++;

	XtGetValues(private->links->dialog, arglist, ac);

	status = MrmFetchIconLiteral(hierarchy, "LinkInIcon", screen,
	    display, foreground, background, &private->link_in_icon);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	status = MrmFetchIconLiteral(hierarchy, "LinkOutIcon", screen,
	    display, foreground, background, &private->link_out_icon);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	status = MrmFetchIconLiteral(hierarchy, "SourceIcon", screen, display,
	    foreground, background, &private->source_icon);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	status = MrmFetchIconLiteral(hierarchy, "TargetIcon", screen, display,
	    foreground, background, &private->target_icon);
                                                
	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	status = MrmFetchIconLiteral(hierarchy, "SourceAndTargetIcon", screen,
	    display, foreground, background, &private->source_and_target_icon);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	status = MrmFetchIconLiteral(hierarchy, "TargetAndSourceIcon", screen,
	    display, foreground, background, &private->target_and_source_icon);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);
    }

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }


void  LwkDXmFreeLinkInfo(private)
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
    **  Free any Link Info we have saved away
    */

    if (private->link_info != (_LinkInfo) 0) {
	FreeLinkInfo(private->link_info, private->link_info_size);

	private->link_info = (_LinkInfo) 0;
    }

    return;
    }
        
                                                 
void  LwkDXmShowLinks(private, links, opr_ids, opr_names, directions, follow_type, iff_visible, update, closure)
_DXmUiPrivate private;
 _List links;

    _List opr_ids;
 _List opr_names;
 _List directions;
 _FollowType follow_type;

    _Boolean iff_visible;
 _Boolean update;
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
    _LinkInfo volatile link_info;
    _List_of(_String) volatile opr_ids_list;
    _List_of(_String) volatile opr_names_list;
    _Boolean initial_display = _False;
    Widget shell;
        						
    /*
    **  If we were requested to update the Dialog Box if and only if it is
    **	visible (i.e., a request to update contents), make sure it is visible.
    */

    if (iff_visible) {
	if (private->links->dialog == (Widget) 0)
	    return;
	if (!XtIsManaged(private->links->dialog))
	    return;
    }

    /*
    **  If the Dialog Box isn't created, do it.
    */

    if (private->links->dialog == (Widget) 0) {
	LwkDXmShowLinksCreate(private);
	initial_display = _True;
    }

    /*
    ** If the Dialog Box isn't realized, do it now (some of the SVN
    ** manipulations require this).
    */

    if (!XtIsRealized(private->links->dialog)) 
	XtRealizeWidget(private->links->dialog);

    /*
    **  Initialization
    */

    link_info = (_LinkInfo) 0;
    opr_ids_list = (_List) _NullObject;
    opr_names_list = (_List) _NullObject;

    _StartExceptionBlock

    /*
    ** Get the number of Links in the List.
    */

    if (links == (_List) _NullObject)
	count = 0;
    else
	_GetValue(links, _P_ElementCount, lwk_c_domain_integer, (_AnyPtr) &count);

    /*
    ** Allocate the LinkInfo structure.
    */

    if (count > 0) {
	link_info =
	    (_LinkInfo) _AllocateMem(sizeof(_LinkInfoInstance) * count);

	link_info->op_cnt = 0;
    }

    /*
    **  Remove the Links from the Link List.
    */

    for (i = 0; i < count; i++) {
	_RemoveElement(links, lwk_c_domain_link,
	    &link_info[i].link);

	_RemoveElement(directions, lwk_c_domain_boolean,
	    &link_info[i].out_going);

	/*
	** Get the number of operations associated with the Link and
	** build the operation name array.
	*/

	_RemoveElement(opr_ids, lwk_c_domain_list, (_AnyPtr) &opr_ids_list);
	_RemoveElement(opr_names, lwk_c_domain_list, (_AnyPtr) &opr_names_list);

	_GetValue(opr_ids_list, _P_ElementCount, lwk_c_domain_integer,
	    &op_cnt);

	if (op_cnt > 0) {
	    /*
	    ** Allocate and fill in the operation name String array.
	    */

	    link_info[i].opr_ids =
		(_String *) _AllocateMem(sizeof(_String) * op_cnt);

	    link_info[i].opr_names =
		(_String *) _AllocateMem(sizeof(_String) * op_cnt);

	    for (j = 0; j < op_cnt; j++) {
		_RemoveElement(opr_ids_list, lwk_c_domain_string,
		    &link_info[i].opr_ids[j]);
		_RemoveElement(opr_names_list, lwk_c_domain_ddif_string,
		    &link_info[i].opr_names[j]);
	    }

	    link_info[i].op_cnt = op_cnt;
	}

	/*
	** Delete the operation lists.
	*/

	_Delete(&opr_ids_list);
	_Delete(&opr_names_list);
    }

    /*
    **  Save the default FollowType
    */

    if (follow_type != lwk_c_follow_implicit_go_to)
	private->links_follow_type = follow_type;

    /*
    **  Fill in the SVN widgets with the Link information
    */

    DisplayLinks(private, link_info, count);

    link_info = (_LinkInfo) 0;

    /*
    ** Popup the dialog box if needed. We don't want to popup the dialog box
    ** when you are forcing an update of the box, as a result of a delete link
    ** in an other application.
    */

    if (!update) {
    
	if (XtIsManaged(private->links->dialog)) 
	    XRaiseWindow(XtDisplay(private->links->dialog),
		XtWindow(XtParent(private->links->dialog)));
	else {

	    /*
	    ** Position with respect to main window
	    */

	    if (initial_display)  {

		if (XtParent(private->main_widget) != 0)
		    shell = XtParent(private->main_widget); /* get the shell */
		else
		    shell = private->main_widget;	    /* it is a shell */
	    
		LwkDXmPositionWidget(private->links->dialog, shell);
	    }

	    XtManageChild(private->links->dialog);

	    if (initial_display) {

		/*
		** Added a workaround in uil to set the Visit button
		** to show as default so that the size of the dialog
		** box would allow room for the pushbutton.  Turn this
		** off after the box has been created.
		*/
		
		Arg arglist[1];

/*		XtSetArg(arglist[0], XmNshowAsDefault, 0);
		XtSetValues(private->links->visit, arglist, 1);
*/	    }

	    SetProcessTraversal(private);
	}
    }
    
    /*
    ** If the Link dialog box is visible or if you are forcing an update,
    ** update it.
    */

    if (count > 0 && private->show_link->dialog != (Widget) 0) {
	if (XtIsManaged(private->show_link->dialog) || update) {
	    int index;

	    index = private->selected_link_entry / 2;

	    LwkDXmLinkDisplay(private, private->link_info[index].link,
		private->show_link);
	}
    }

    /*
    **  Reset the default button 
    */

    if (!update) {
	SetDefaultButton(private);
    }

    /*
    ** De-iconized the dialog box if necessary but not when we are forced
    ** to updated it (side effect from another application deleting a link)
    */

    if (!update)
	LwkDXmSetShellIconState(XtParent(private->links->dialog), NormalState);
	
    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&opr_ids_list);
	    _Delete(&opr_names_list);

	    if (link_info != (_LinkInfo) 0)
		FreeLinkInfo(link_info, count);

	    _Reraise;
    _EndExceptionBlock
		
    return;
    }


static void  DisplayLinks(private, link_info, count)
_DXmUiPrivate private;
 _LinkInfo link_info;

    int count;

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
    _Surrogate destination;
    _CString volatile description;
    _CString volatile relationship;

    /*
    **  Initialization
    */

    description = (_CString) _NullObject;
    relationship = (_CString) _NullObject;

    _StartExceptionBlock

    DXmSvnDisableDisplay(private->links->links);

    /*
    **  Empty the SVN widgets of any old contents.
    */

    if (private->link_info_size > 0) {
	DXmSvnDeleteEntries(private->links->links, 0,
	    private->link_info_size * 2);

	FreeLinkInfo(private->link_info, private->link_info_size);
    }

    /*
    **  Build a description for each Link
    */

    for (i = 0; i < count; i++) {
	/*
	**  Get the RelationshipType of the Link (else the Description,
	**  else a blank)
	*/

	_GetCSValue(link_info[i].link, _P_RelationshipType,
	    &relationship);
	      	
	if (relationship == (_CString) _NullObject) {
	    _GetCSValue(link_info[i].link, _P_Description,
		&relationship);

	    if (relationship == (_CString) _NullObject)
		relationship = _StringToCString((_String) "");
	}

	/*
	** Get the Description of the Destination (else a blank).
	*/

	if (link_info[i].out_going)
	    _GetCSValue(link_info[i].link, _P_TargetDescription,
		&description);
	else
	    _GetCSValue(link_info[i].link, _P_SourceDescription,
		&description);

	if (description == (_CString) _NullObject)
	    description = _StringToCString((_String) "");

	/*
	**  Get the Destination Surrogate
	*/

	if (link_info[i].out_going)
	    _GetValue(link_info[i].link, _P_Target,
		lwk_c_domain_surrogate, &destination);
	else
	    _GetValue(link_info[i].link, _P_Source,
		lwk_c_domain_surrogate, &destination);
	/*
	**  Set the first entry of this pair to be the Relationship
	*/

	DXmSvnAddEntries(private->links->links, i * 2, 1, 0,
	    (XtPointer *) 0, TRUE);

	DXmSvnSetEntry(private->links->links, i * 2 + 1, 0, 0,
	    2, TRUE, 0, TRUE);

	if (link_info[i].out_going)
	    pixmap = private->link_out_icon;
	else
	    pixmap = private->link_in_icon;

	DXmSvnSetComponentPixmap(private->links->links,
	    i * 2 + 1, 1, 0, 0, pixmap, _IconWidth, _IconHeight);

	
	DXmSvnSetComponentText( private->links->links, /* widget   */
			    i * 2 + 1,			/* entry    */
			    2,				/* comp	    */
			    (_IconWidth * 3) / 2,	/* x	    */
			    0,				/* y	    */
			    (XmString) relationship,	/* text	    */
			    (_AnyPtr) 0);		/* font	    */

	/*
	**  Set the second entry of this pair to be the Destination Description
	*/

	DXmSvnAddEntries(private->links->links, i * 2 + 1, 1, 1,
	    (XtPointer *) 0, TRUE);

	DXmSvnSetEntry(private->links->links, i * 2 + 2, 0, 0,
	    2, TRUE, 0, TRUE);

	pixmap = GetSurrogatePixmap(private, destination,
	    link_info[i].out_going);

	DXmSvnSetComponentPixmap(private->links->links,
	    i * 2 + 2, 1, 0, 0, pixmap, _IconWidth, _IconHeight);

	
	DXmSvnSetComponentText( private->links->links, /* widget   */
			    i * 2 + 2,			/* entry    */
			    2,				/* comp	    */
			    (_IconWidth * 3) / 2,	/* x	    */
			    0,				/* y	    */
			    (XmString) description,	/* text	    */
			    (_AnyPtr) 0);		/* font	    */

	/*
	**  Free the Relationship and Destination Descriptions
	*/

	_DeleteCString(&relationship);
	_DeleteCString(&description);
    }

    /*
    **  Save the new LinkInfo
    */

    private->link_info_size = count;

    private->link_info = link_info;

    /*
    **	Select the Destination of the first Link and make sure that it is
    **	visible.
    */

    if (count > 0) {
	private->selected_link_entry = 1;

	DXmSvnPositionDisplay(private->links->links, 1,
	    DXmSVN_POSITION_TOP);

	DXmSvnSelectEntry(private->links->links, 2);

	SetButtonSensitivity(private, _True);
    }
    else {
	private->selected_link_entry = -1;

	SetButtonSensitivity(private, _False);

	/*
	** If the Show Link box is managed, unmanages it.
	*/

	if (private->show_link->dialog != (Widget) 0)
	    if (XtIsManaged(private->show_link->dialog))
		XtUnmanageChild(private->show_link->dialog);
    }

    /*
    **  Repaint the SVN displays
    */

    DXmSvnEnableDisplay(private->links->links);

    /*
    **  If an exception is raised, clean up then reraise it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteCString(&relationship);
	    _DeleteCString(&description);
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
static void  CreateLinks(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->links = w;

    return;
    }

static void  CreateOperationMenu(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->operations = w;

    return;
    }

static void  CreateShow(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->show = w;

    return;
    }

static void  CreateDelete(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->delete = w;

    return;
    }

static void  CreateCancel(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->cancel = w;

    return;
    }

static void  CreateGoTo(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->go_to = w;

    return;
    }

static void  CreateVisit(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    
    private->links->visit = w;

    return;
    }

static void  CreateListForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->listform = w;

    return;
    }
static void  CreateButtonsForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->buttonsform = w;

    return;
    }
static void  CreateNavigateForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->navigateform = w;

    return;
    }
static void  CreateEditForm(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

    {
    private->links->editform = w;

    return;
    }


static void  ActivateUpdate(w, private, reason)
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
    Update(private, reason);

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


static void  ActivateDelete(w, private, reason)
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
    ** Pop up the Caution Box and wait for confirmation
    */

    XtManageChild(private->links->caution);

    return;
    }


static void  ConfirmDelete(w, private, reason)
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
    ** Delete the Link 
    */

    DeleteLink(private);

    return;
    }


static void  ActivateGoTo(w, private, reason)
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
    Follow(private, lwk_c_follow_go_to);

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
    Follow(private, lwk_c_follow_visit);

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
    ** Unmanage the Show Links box.
    */

    XtUnmanageChild(private->links->dialog);

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
	private->selected_link_op_entry = i;
    else
	private->selected_link_op_entry = 0;

    return;
    }
    

static void  SelectLink(w, private, svn)
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
    ** Get the index of the selected Link.
    */

    old = private->selected_link_entry / 2;

    private->selected_link_entry = svn->entry_number - 1;

    selected = private->selected_link_entry / 2;

    /*
    **  Reset the Operations list
    */

    SetOperations(private, selected);
    
    /*
    **  Reset the default button
    */

    SetDefaultButton(private);

    /*
    ** If the Show Link box is managed, and needs updating, do it.
    */

    if (private->show_link->dialog != (Widget) 0)
	if (XtIsManaged(private->show_link->dialog) && old != selected)
		LwkDXmLinkDisplay(private, private->link_info[selected].link,
		    private->show_link);
	
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


static void  ConfirmLink(w, private, svn)
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
    **	If the Destination is selected, do a GoTo or a Visit.  If a Link
    **	itself is selected, do a Show More.
    */

    if ((private->selected_link_entry % 2) == 0)
	ShowMore(private);
    else
	Follow(private, private->links_follow_type);

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(unexpected_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  SelectDoneLinks(w, private, svn)
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
    int selected;

    _StartExceptionBlock

    /*
    **	If there is no Link selected, or no Operation selected, grey out
    **	all buttons except Cancel
    */

    if (DXmSvnGetNumSelections(private->links->links) > 0)
	SetButtonSensitivity(private, TRUE);
    else {
	SetButtonSensitivity(private, FALSE);

        /*
	** Set the cancel button to be the default one.
	*/
	ChangeDefaultButton(private, private->links->cancel);
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


static void  SetOperations(private, index)
_DXmUiPrivate private;
 int index;

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
    **  Get the Operations list for the selected Link
    */

    op_cnt = private->link_info[index].op_cnt;
    opr_ids = private->link_info[index].opr_ids;
    opr_names = private->link_info[index].opr_names;

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
    XtGetValues(private->links->operations, arglist, 1);

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
		entries[i] = (Widget) XmCreatePushButtonGadget(menu, "", arglist, 1);
		/* XtManageChild(entry);*/
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

	if (entry_cnt > 0)
	    XtManageChildren(entries, entry_cnt);


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

	private->selected_link_op_entry = i;

	/*
	** Now set the option menu history
	*/

	XtSetArg(arglist[0], XmNmenuHistory, entries[i]);
	XtSetValues(private->links->operations, arglist, 1);
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


static void  Follow(private, follow_type)
_DXmUiPrivate private;
 _FollowType volatile follow_type;

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
    _String operation;
    _Surrogate origin;
    _Surrogate destination;
    _LinkInfo link_info;

    _StartExceptionBlock

    /*
    **  Use the currently selected Link information
    */

    index = private->selected_link_entry / 2;

    link_info = &private->link_info[index];

    /*
    ** Get the Origin, Destination and selected Operation.
    */

    if (link_info->out_going) {
	_GetValue(link_info->link, _P_Source, lwk_c_domain_surrogate,
	    &origin);

	_GetValue(link_info->link, _P_Target, lwk_c_domain_surrogate,
	    &destination);
    }
    else {
	_GetValue(link_info->link, _P_Source, lwk_c_domain_surrogate,
	    &destination);

	_GetValue(link_info->link, _P_Target, lwk_c_domain_surrogate,
	    &origin);
    }

    operation = link_info->opr_ids[private->selected_link_op_entry];

    /*
    ** Navigate to the destination.
    */

    _Navigate(private->dwui, link_info->link, origin, destination,
	operation, follow_type);

    /*
    ** If any exceptions are raised, clean up, then display error message
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    if (follow_type == lwk_c_follow_go_to)
		status[0] = _StatusCode(go_to_error);
	    else
		status[0] = _StatusCode(visit_error);

	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  Update(private, reason)
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
    **  Simply re-request Show Links
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_show_links, (_Closure) reason);

    /*
    ** If any exceptions other than Not Linked are raised, display error
    ** message.  We just ignore Not Linked.
    */

    _Exceptions
	_When(not_linked)

	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_links_error);
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

    _StartExceptionBlock

    /*
    **  Create the Show Link Dialog Box if necessary
    */

    if (private->show_link->dialog == (Widget) 0)
	LwkDXmShowLinkCreate(private);

    /*
    ** Update source properties in the Show Link dialog box.
    */

    index = private->selected_link_entry / 2;

    LwkDXmLinkDisplay(private, private->link_info[index].link,
	private->show_link);

    /*
    ** Popup the dialog box if not already managed.
    */

    LwkDXmShowLinkBtnSensitivity(private->show_link, _False);
    
    LwkDXmLinkPopup(private->show_link, private->main_widget, _False,
	(_Reason) 0);

    LwkDXmShowLinkCancelBtnDef(private->show_link);

    /*
    ** If any exceptions are raised, display error message
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_link_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  DeleteLink(private)
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
    _Boolean found;
    _Linknet linknet;
    _Linkbase linkbase;
    _Link link;
    _Linknet current_linknet;
    _CompLinknet current_cnet;

    _StartExceptionBlock

    /*
    **  Get the selected Link
    */

    index = private->selected_link_entry / 2;

    link = private->link_info[index].link;

    if (link!= (_Link) _NullObject) {
        /*
        ** Get the Link's Linknet and the Linknet's Linkbase
        */

	_GetValue(link, _P_Linknet, lwk_c_domain_linknet, &linknet);

	if (linknet == (_Linknet) _NullObject)
	    linkbase = (_Linkbase) _NullObject;
	else
	    _GetValue(linknet, _P_Linkbase, lwk_c_domain_linkbase,
		&linkbase);

	/*
	** Delete the Link
	*/

	_Delete(&link);

        /*
	** If the Link was Stored in a Linkbase, let the world know
	** what we just did.
        */
	
	if (linkbase != (_Linkbase) _NullObject) {
	    /*
	    ** Store the Linknet so that the deletion takes effect
	    */

	    _Store(linknet, linkbase);

	    /*
	    ** If the Linknet is the Current Linknet, reset Current Linknet so
	    ** that others will notice the change.
	    */

	    _GetValue(private->dwui, _P_RecordingLinknet, lwk_c_domain_linknet,
		&current_linknet);

	    if (linknet == current_linknet)
		_SetValue(private->dwui, _P_RecordingLinknet, lwk_c_domain_linknet,
		    &current_linknet, lwk_c_set_property);

	    /*
	    ** If the Linknet is one of the Linknets in the Active
	    ** Linknet, reset Active Linknet so that others will
	    ** notice the change.
	    */

	    _GetValue(private->dwui, _P_ActiveCompLinknet,
		lwk_c_domain_comp_linknet, &current_cnet);

	    if (current_cnet != (_CompLinknet) _NullObject) {
		found = (_Boolean) _Iterate(current_cnet, lwk_c_domain_linknet,
		    (_Closure) linknet, FindLinknet);

		if (found)
		    _SetValue(private->dwui, _P_ActiveCompLinknet,
			lwk_c_domain_comp_linknet, &current_cnet,
			lwk_c_set_property);
	    }
	}
    }

    /*
    ** If any exceptions are raised, display error message
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(delete_link_error);
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
    **	If the Destination is selected, make the GoTo or Visit button the
    **	default.  If a Link itself is selected, make the Show More button
    **	the default.
    */

    if ((private->selected_link_entry % 2) == 0)
	button = private->links->show;	
    else {
	if (private->links_follow_type == lwk_c_follow_go_to)
	    button = private->links->go_to;
	else
	    button = private->links->visit;	
    }
    
    /*
    **  Only do the SetValues if it is not already correctly set.
    */

    XtSetArg(arglist[0], XmNdefaultButton, &default_button);

    XtGetValues(private->links->dialog, arglist, 1);

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

    XtSetValues(private->links->dialog, arglist, 1);
    XtSetValues(private->links->listform, arglist, 1);
    XtSetValues(private->links->buttonsform, arglist, 1);
    XtSetValues(private->links->editform, arglist, 1);
    XtSetValues(private->links->navigateform, arglist, 1);

    return;
    }


static void  SetProcessTraversal(private)
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
    **	If the Destination is selected, make the GoTo or Visit the
    **	traversal destination.  If a Link itself is selected, make
    **	the Show More button the default for traversal.
    */

    if ((private->selected_link_entry % 2) == 0)
	button = private->links->show;	
    else {
	if (private->links_follow_type == lwk_c_follow_go_to)
	    button = private->links->go_to;
	else
	    button = private->links->visit;	
    }

    XmProcessTraversal(button, XmTRAVERSE_CURRENT);

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

    old_state = XtIsSensitive(private->links->show);

    if (state != old_state) {
	XtSetSensitive(private->links->show, state);
	XtSetSensitive(private->links->delete, state);
	XtSetSensitive(private->links->go_to, state);
	XtSetSensitive(private->links->visit, state);
    }

    return;
    }


static void  FreeLinkInfo(link_info, size)
_LinkInfo link_info;
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
	if (link_info[i].op_cnt > 0) {
	    for (j = 0; j < link_info[i].op_cnt; j++) {
	    	_DeleteString(&link_info[i].opr_ids[j]);
		_DeleteDDIFString(&link_info[i].opr_names[j]);
	    }

	    _FreeMem(link_info[i].opr_ids);
	    _FreeMem(link_info[i].opr_names);
	}
    }

    /*
    **  Free the LinkInfo structure
    */

    _FreeMem(link_info);

    return;
    }


static Pixmap  GetSurrogatePixmap(private, surrogate, out_going_link)
_DXmUiPrivate private;
 _Surrogate surrogate;

    _Boolean out_going_link;

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
    **  Find out if this Surrogate is a Source, a Target, or both, and return
    **	the appropriate Icon Pixmap.
    */

    has_incomming = (_Boolean) _SetState(surrogate, _StateHasInComming,
	_StateGet);

    has_outgoing = (_Boolean) _SetState(surrogate, _StateHasOutGoing,
	_StateGet);

    if (has_incomming && has_outgoing) {
	if (out_going_link)
	    pixmap = private->target_and_source_icon;
	else
	    pixmap = private->source_and_target_icon;
    }
    else if (has_incomming)
	pixmap = private->target_icon;
    else
	pixmap = private->source_icon;

    return pixmap;
    }


static _Termination  FindLinknet(linknet1, cnet, domain, linknet2)
_Linknet linknet1;
 _CompLinknet cnet;

    _Domain domain;
 _Linknet *linknet2;

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
    **  If the two Linknets are the same, return 1, else return 0.
    */

    if (linknet1 == *linknet2)
	return (_Termination) 1;
    else
	return (_Termination) 0;
    }
