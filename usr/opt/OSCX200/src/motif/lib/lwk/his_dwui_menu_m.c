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
**	LWK DXm Interface for the link menu.
**
**  Keywords:
**	LWK, UI
**
**  Environment:
**	User mode, executable image
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
#include "his_registry.h"

/*
**  Macro Definitions
*/

#define _SelectedSurrLinked(Ui, Closure) \
    LwkSelectedSurrLinked((Ui), (Closure))

_DeclareFunction(_Boolean LwkSelectedSurrLinked,
    (_Ui ui, _Closure closure));

_DeclareFunction(void LwkDXmHelpSystemError,
    (_DXmUiPrivate private, int status));

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _DXmUiPrivate InitializePrivate,
    (_DXmUi dwui));
_DeclareFunction(static void EnableAnnotate,
    (_DXmUiPrivate private));
_DeclareFunction(static void FreePixmaps,
    (_DXmUiPrivate private));
_DeclareFunction(static void MapMenu,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateGoTo,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateVisit,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateShowHistory,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateStep,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateUndo,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateSetSource,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateAnnotate,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateCompleteLink,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateCompleteLinkDialog,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateShowLinks,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateHighlight,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void CreateHighlightToggle,
    (Widget w, _MenuWidgets menu, _Reason reason));
_DeclareFunction(static void ActivateGoTo,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateVisit,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateShowHistory,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateStep,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateUndo,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateSetSource,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateAnnotate,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCompleteLink,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateCompleteLinkDialog,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateLinkDialog,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateShowLinks,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ActivateHighlight,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void ToggleHighlight,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static void CreatePulldownMenu,
    (_MenuWidgets menu));
_DeclareFunction(void LwkDXmHelpLinkMenuCascade,
    (Widget w, _DXmUiPrivate private, _Reason reason));

/*
**  Static Data Definitions
*/
                             
/*
**  MRM Names to register
*/

static MrmRegisterArg _Constant Register[] = {
	{"LwkContextSensitiveHelp", (caddr_t) LwkDXmHelpContextSensitiveHelp}
    };

static MrmCount _Constant RegisterSize = XtNumber(Register);

static MrmRegisterArg _Constant RegisterMenu[] = {
	{"LwkMapMenu", (caddr_t) MapMenu},
	{"LwkCreateGoTo", (caddr_t) CreateGoTo},
	{"LwkCreateVisit", (caddr_t) CreateVisit},
	{"LwkCreateShowHistory", (caddr_t) CreateShowHistory},
	{"LwkCreateStepForward", (caddr_t) CreateStep},
	{"LwkCreateUndo", (caddr_t) CreateUndo},
	{"LwkCreateStartLink", (caddr_t) CreateSetSource},
	{"LwkCreateAnnotate", (caddr_t) CreateAnnotate},
	{"LwkCreateCompleteLink", (caddr_t) CreateCompleteLink},
	{"LwkCreateCompleteLinkDialog", (caddr_t) CreateCompleteLinkDialog},
	{"LwkCreateShowLinks", (caddr_t) CreateShowLinks},
	{"LwkCreateHighlight", (caddr_t) CreateHighlight},
	{"LwkCreateHighlightToggle", (caddr_t) CreateHighlightToggle},
	{"LwkActivateGoTo", (caddr_t) ActivateGoTo},
	{"LwkActivateVisit", (caddr_t) ActivateVisit},
	{"LwkActivateShowHistory", (caddr_t) ActivateShowHistory},
	{"LwkActivateShowLinks", (caddr_t) ActivateShowLinks},
	{"LwkActivateStepForward", (caddr_t) ActivateStep},
	{"LwkActivateUndo", (caddr_t) ActivateUndo},
	{"LwkActivateStartLink", (caddr_t) ActivateSetSource},
	{"LwkActivateAnnotate", (caddr_t) ActivateAnnotate},
	{"LwkActivateCompleteLink", (caddr_t) ActivateCompleteLink},
	{"LwkActivateCompleteLinkDialog", (caddr_t) ActivateCompleteLinkDialog},
	{"LwkActivateHighlight", (caddr_t) ActivateHighlight},
	{"LwkToggleHighlight", (caddr_t) ToggleHighlight}
    };

static MrmCount _Constant RegisterMenuSize = XtNumber(RegisterMenu);

/*
**  List header for DwUiPrivate structures
*/

static _DXmUiPrivate DwUiPrivateList = (_DXmUiPrivate) 0;


void  LwkOpDWInitialize()
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
    **  Initialize DRM
    */

    MrmInitialize();

    /*
    **  Initialize DRM for Digital widgets
    */

    DXmInitialize();


    return;
    }


_DXmUiPrivate  LwkDXmCreate(dwui, appl_name, create_menu, default_accelerators, main_window, menu_entry)
_DXmUi dwui;
 _CString appl_name;

    _Boolean create_menu;
 _Boolean default_accelerators;
 Widget main_window;

    Widget menu_entry;

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
    int status;
    int ac;                                  
    Arg arglist[10];
    _DXmUiPrivate volatile private;
    XtCallbackList callbacklist;
    XtCallbackRec help_cb[2];
    XtCallbackRec cascading_cb[2];
                            
    /*
    ** Check parameters.
    */

    if (main_window == (Widget) 0)
    	_Raise(inv_widget_id);

    if (create_menu && menu_entry == (Widget) 0)
    	_Raise(inv_widget_id);

    /*
    ** Initialize private data.
    */

    private = InitializePrivate(dwui);

    _StartExceptionBlock

    private->main_widget = main_window;
    private->appl_name = (XmString) _CopyCString(appl_name);
    private->pulldown->menu_entry = menu_entry;
    
    /*
    ** Register all DRM names (except for the Menu names)
    */

    MrmRegisterNames(Register, RegisterSize);

    if (create_menu) {

        private->pulldown->default_accelerators = default_accelerators;

	/*
        ** Create the menu widget and attach it to the appl's pdme
	*/

	CreatePulldownMenu(private->pulldown);

	/*        
        ** If the application hasn't already setup a help callback for
	** it's pulldown menu entry, provide one.
	*/

        ac = 0;
	XtSetArg(arglist[ac], XmNhelpCallback, &callbacklist); ac++;
	XtGetValues(menu_entry, arglist, ac);

	if (callbacklist[0].callback == (XtCallbackProc) 0) {
	    ac = 0;

	    help_cb[0].callback = (XtCallbackProc) LwkDXmHelpLinkMenuCascade;
	    help_cb[0].closure = (XtPointer) private;
	    help_cb[1].callback = (XtCallbackProc) 0;
	    help_cb[1].closure = (XtPointer) 0;

	    XtSetArg(arglist[ac], XmNhelpCallback, help_cb); ac++;

	    XtSetValues(menu_entry, arglist, ac);
	}

    
    }

    _BeginCriticalSection

    /*
    **	Link the new DwUiPrivate structure into the list of DwUiPrivate
    **	structures created by this application.
    */

    private->next = DwUiPrivateList;
    DwUiPrivateList = private;

    _EndCriticalSection

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers
	    _FreeMem(private);
	    _Reraise;
    _EndExceptionBlock

    return private;
    }


void  LwkDXmDelete(private)
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
    if (private == (_DXmUiPrivate) 0)
	return;

    /*
    **  Free all the Pixmaps
    */

    FreePixmaps(private);

    /*
    **  Free text literals
    */

    _DeleteCString(&private->appl_name);

    if (private->pulldown->menu != (Widget) 0) {
	_DeleteCString(&private->pulldown->highlight_on_literal);
	_DeleteCString(&private->pulldown->highlight_off_literal);
    }

    if (private->history->dialog != (Widget) 0) {
	_DeleteCString(&private->history->goto_literal);
	_DeleteCString(&private->history->visit_literal);
	_DeleteCString(&private->history->implicit_goto_literal);
    }

    /*
    ** Destroy all realized dialog boxes.
    */

    if (private->history->dialog != (Widget) 0)
	XtDestroyWidget(private->history->dialog);

    if (private->complete_link->dialog != (Widget) 0)
	XtDestroyWidget(private->complete_link->dialog);

    if (private->show_link->dialog != (Widget) 0)
	XtDestroyWidget(private->show_link->dialog);

    if (private->links->dialog != (Widget) 0)
	XtDestroyWidget(private->links->dialog);

    if (private->highlight->dialog != (Widget) 0)
	XtDestroyWidget(private->highlight->dialog);

    if (private->message_box != (Widget) 0)
	XtDestroyWidget(private->message_box);

    if (private->help_box != (Widget) 0)
	XtDestroyWidget(private->help_box);

#ifdef VMS  /* Use the DXmHelpSystem (hyperhelp) for VMS */
    if (private->help_context != (Opaque) 0)
	DXmHelpSystemClose(private->help_context, (void *) NULL, (Opaque) NULL);
#endif /* ifdef VMS (hyperhelp) */

    /*
    ** Deallocate the Link Info if there was any.
    */

    LwkDXmFreeLinkInfo(private);

    /*
    **  Deallocate the HistoryInfo if there was any.
    */

    LwkDXmFreeHistoryInfo(private);

    /*
    **  Deallocate the StepInfo if there was any.
    */

    LwkDXmFreeStepInfo(private);

    /*
    **  Deallocate the WIPBox Info if there was any.
    */

    LwkDXmFreeWIPBoxInfo(private);

    /*
    **	Unlink this DwUiPrivate from the list of DwUiPrivate structures
    **	created by this application.
    */

    _BeginCriticalSection

    if (DwUiPrivateList != (_DXmUiPrivate) 0) {
	_DXmUiPrivate previous;

	if (private == DwUiPrivateList)
	    DwUiPrivateList = private->next;
	else {
	    previous = DwUiPrivateList;

	    while (previous->next != (_DXmUiPrivate) 0) {
		if (previous->next == private) {
		    previous->next = private->next;
		    break;
		}

		previous = previous->next;
	    }
	}
    }

    _EndCriticalSection

    /*
    ** Free the dwui private structure.
    */

    _FreeMem(private->pulldown);
    _FreeMem(private->links);
    _FreeMem(private->complete_link);
    _FreeMem(private->show_link);
    _FreeMem(private->history);
    _FreeMem(private->step);
    _FreeMem(private->highlight);
    _FreeMem(private->wip);
    _FreeMem(private);

    return;
    }


MrmHierarchy  LwkDXmOpenDRMHierarchy(private)
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
    int status;
    char *filename;
    MrmHierarchy hierarchy;
    MrmRegisterArg drm_register[1];

    /*
    ** Register PrivateData name with DRM.
    */

    drm_register[0].name = _DrmPrivateIdentifier;
    drm_register[0].value = (caddr_t) private;

    MrmRegisterNames(drm_register, (MrmCount) 1);

    /*
    ** Open the DRM hierarchy.
    */

    filename = _UidFileName;

    status = MrmOpenHierarchy((MrmCount) 1, &filename,
	(MrmOsOpenParamPtr *) 0, &hierarchy);

    if (status != MrmSUCCESS)
	_Raise(drm_open_error);

    return hierarchy;
    }


void  LwkDXmCloseDRMHierarchy(hierarchy)
MrmHierarchy hierarchy;

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
    ** Close the DRM hierarchy.
    */

    if (MrmCloseHierarchy(hierarchy) != MrmSUCCESS)
	_Raise(drm_close_error);

    return;
    }


static _DXmUiPrivate  InitializePrivate(dwui)
_DXmUi dwui;

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
    _DXmUiPrivate private;

    /*
    ** Allocate memory for private data.
    */

    private = (_DXmUiPrivate) _AllocateMem(sizeof(_DXmUiPrivateInstance));

    /*
    ** Clear memory.
    */
    _ClearMem(private, sizeof(_DXmUiPrivateInstance));
    
    /*
    ** Initialize private data.
    */

    private->dwui = dwui;
    private->links_follow_type = lwk_c_follow_visit;
    private->selected_link_entry = -1;
    private->selected_link_op_entry = -1;
    private->selected_history_entry = -1;
    private->selected_history_op_entry = -1;
    private->selected_step_op_entry = -1;
    private->highlighting = lwk_c_hl_none;
    
    /*
    **  Allocate and initialize sub-structures
    */

    private->pulldown =
	(_MenuWidgets) _AllocateMem(sizeof(_MenuWidgetsInstance));

    _ClearMem(private->pulldown, sizeof(_MenuWidgetsInstance));

    private->pulldown->private = (_AnyPtr) private;
    private->pulldown->enabled_annotate = _False;
    

    private->links =
	(_LinksWidgets) _AllocateMem(sizeof(_LinksWidgetsInstance));

    _ClearMem(private->links, sizeof(_LinksWidgetsInstance));


    private->complete_link =
	(_LinkWidgets) _AllocateMem(sizeof(_LinkWidgetsInstance));

    _ClearMem(private->complete_link, sizeof(_LinkWidgetsInstance));


    private->show_link =
	(_LinkWidgets) _AllocateMem(sizeof(_LinkWidgetsInstance));

    _ClearMem(private->show_link, sizeof(_LinkWidgetsInstance));
             

    private->history =
	(_HistoryWidgets) _AllocateMem(sizeof(_HistoryWidgetsInstance));

    _ClearMem(private->history, sizeof(_HistoryWidgetsInstance));


    private->step =
	(_StepWidgets) _AllocateMem(sizeof(_StepWidgetsInstance));

    _ClearMem(private->step, sizeof(_StepWidgetsInstance));


    private->highlight =
	(_HighlightWidgets) _AllocateMem(sizeof(_HighlightWidgetsInstance));

    _ClearMem(private->highlight, sizeof(_HighlightWidgetsInstance));


    private->wip =
	(_WIPListStruct) _AllocateMem(sizeof(_WIPListStructInstance));

    _ClearMem(private->wip, sizeof(_WIPListStructInstance));
    
    return private;
    }


static void  EnableAnnotate(private)
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
    _String method;

    _StartExceptionBlock

    /*
    ** See if there is a method for the Annotate Operation on Any Surrogate
    ** Subtype.  If so, enable the Annotate menu option.
    */

    method = LwkRegOperationMethod(_AnyTypeIdentifier, _AnnotateOperationId);

    if (method != (_String) _NullObject) {
	XtManageChild(private->pulldown->annotate);
	_DeleteString(&method);
    }

    /*
    ** Ignore exceptions (e.g., NotRegistered).
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return;
    }
                                                                 

static void  FreePixmaps(private)
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
    **  Free all the Pixmaps
    */

    if (private->menu_icon != (Pixmap) 0) {
	XFreePixmap(XtDisplay(private->pulldown->menu), private->menu_icon);

	XFreePixmap(XtDisplay(private->pulldown->menu),
	    private->menu_icon_hlight);

	XFreePixmap(XtDisplay(private->pulldown->menu), private->dbox_icon);
    }

    if (private->links->dialog != (Widget) 0) {
	XFreePixmap(XtDisplay(private->links->dialog),
	    private->link_in_icon);

	XFreePixmap(XtDisplay(private->links->dialog),
	    private->link_out_icon);

	XFreePixmap(XtDisplay(private->links->dialog),
	    private->source_icon);

	XFreePixmap(XtDisplay(private->links->dialog),
	    private->target_icon);

	XFreePixmap(XtDisplay(private->links->dialog),
	    private->source_and_target_icon);

	XFreePixmap(XtDisplay(private->links->dialog),
	    private->target_and_source_icon);
    }

    if (private->history->dialog != (Widget) 0) {
	XFreePixmap(XtDisplay(private->history->dialog), private->step_icon);
                          
	XFreePixmap(XtDisplay(private->history->dialog), private->origin_icon);

	XFreePixmap(XtDisplay(private->history->dialog),
	    private->destination_icon);

	XFreePixmap(XtDisplay(private->history->dialog),
	    private->origin_and_destination_icon);
    }

    return;               
    }

           
static void  CreatePulldownMenu(menu)
_MenuWidgets menu;

{                         
    MrmHierarchy hierarchy;
    MrmRegisterArg drm_register[2];
    MrmCode type;
    MrmType *class;
    int status;
    _DXmUiPrivate private;
    int ac;
    Arg arglist[10];
    XmString xstring;

    private = (_DXmUiPrivate) menu->private;

    if (menu->menu == 0){

	/*
	** Register all DRM names associated with the Menu
	*/

	MrmRegisterNames(RegisterMenu, RegisterMenuSize);
                                              
	drm_register[0].name = _DrmPrivateIdentifier;
	drm_register[0].value = (caddr_t) private;
	drm_register[1].name = _DrmMenuIdentifier;
	drm_register[1].value = (caddr_t) menu;
             
	MrmRegisterNames(drm_register, (MrmCount) 2);

	/*
	**  Open the DRM hierarchy
	*/

	hierarchy = LwkDXmOpenDRMHierarchy(private);

	/*
	** Fetch and Realize the Link Menu widget.
	*/

	if (menu->default_accelerators)
	    status = MrmFetchWidgetOverride(hierarchy, _MrmLinkMenuAccel,
		XtParent(menu->menu_entry), _MrmLinkMenu,
		(ArgList) 0, (int) 0, &menu->menu, (MrmType *) &class);
        else
	    status = MrmFetchWidgetOverride(hierarchy, _MrmLinkMenu,
		XtParent(menu->menu_entry), _MrmLinkMenu,
		(ArgList) 0, (int) 0, &menu->menu, (MrmType *) &class);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

    
        /*
        ** Realize the Link Menu
        */

	XtRealizeWidget(menu->menu);

	/*
	**  Fetch the alternate labels and mnemonics for the Highlight button
	*/

	status = MrmFetchLiteral(hierarchy, _MrmHighlightOnLiteral,
	    XtDisplay(private->main_widget), (caddr_t *) &xstring, &type);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);
	
	menu->highlight_on_literal =
	    (XmString) _CopyCString((_CString) xstring);


	status = MrmFetchLiteral(hierarchy, _MrmHighlightOnMnemonic,
	    XtDisplay(private->main_widget),
	    (caddr_t *) &(menu->highlight_on_keysym), &type);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	status = MrmFetchLiteral(hierarchy, _MrmHighlightOffLiteral,
	    XtDisplay(private->main_widget), (caddr_t *) &xstring, &type);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	menu->highlight_off_literal =
	    (XmString) _CopyCString((_CString) xstring);

	status = MrmFetchLiteral(hierarchy, _MrmHighlightOffMnemonic,
	    XtDisplay(private->main_widget),
	    (caddr_t *) &(menu->highlight_off_keysym), &type);

	if (status != MrmSUCCESS)
	    _Raise(drm_fetch_error);

	/*
	** Attach the Link menu to the pulldown entry.
	*/

	ac = 0;
	XtSetArg(arglist[ac], XmNsubMenuId, menu->menu); ac++;

	XtSetValues(menu->menu_entry, arglist, ac);

	/*
	** Close the DRM hierarchy.
	*/

	LwkDXmCloseDRMHierarchy(hierarchy);


    }    
}


static void  MapMenu(w, menu, reason)
Widget w;
 _MenuWidgets menu;
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
    Arg arglist[5];
    _ObjectId oid;
    _Callback callback;
    _Boolean sensitivity;
    _DXmUiPrivate private;
    _Integer highlighting;
    _Boolean current_source;
    _Boolean current_linknet;
    _Integer ac = 0;
    
    private = (_DXmUiPrivate) menu->private;

    if (menu->enabled_annotate == _False) {
	/*
	** Include the Annotate entry only if the Annotate operation is
	** defined.
	*/
	
	EnableAnnotate(private);

	menu->enabled_annotate = _True; /* only do this once */
    }


    _StartExceptionBlock

    /*
    ** Grey out the GoTo, Visit and Show Links menu items if the
    ** currently selected surrogate isn't linked.
    */

    if (_SelectedSurrLinked(private->dwui, (_Closure) reason))
	sensitivity = _True;
    else
	sensitivity = _False;
	
    XtSetSensitive(menu->go_to, sensitivity);
    XtSetSensitive(menu->visit, sensitivity);
    XtSetSensitive(menu->show_links, sensitivity);

    /*
    ** If the CreateSurrogate callback is null, or there is no Current Linknet,
    ** grey out the Set Source menu item.
    */

    _GetValue(private->dwui, _P_CreateSurrogateCb, lwk_c_domain_routine,
	&callback);

    _GetValue(private->dwui, _P_RecordingLinknet, lwk_c_domain_object_id, &oid);

    if (oid == (_ObjectId) _NullObject)
	current_linknet = _False;
    else {
	current_linknet = _True;
	_Delete(&oid);
    }

    if (current_linknet && callback != (_Callback) _NullObject)
	sensitivity = _True;
    else
	sensitivity = _False;

    XtSetSensitive(menu->set_source, sensitivity);

    /*
    ** If there is no Current Composite Path, grey out the Step Forward menu
    ** item.
    */

    _GetValue(private->dwui, _P_ActiveCompPath, lwk_c_domain_object_id,
	&oid);

    if (oid == (_ObjectId) _NullObject)
	sensitivity = _False;
    else {
	sensitivity = _True;
	_Delete(&oid);
    }

    XtSetSensitive(menu->step, sensitivity);

    /*
    ** See if we have a current source (i.e. Begin Link was chosen)
    */

    _GetValue(private->dwui, _P_PendingSource, lwk_c_domain_object_id, &oid);

    if (oid == (_ObjectId) _NullObject)
	current_source = _False;
    else {
	current_source = _True;
	_Delete(&oid);
    }

    /*
    ** If there is no Current Linknet or if Start Link wasn't chosen 
    ** grey out the Complete Link and Complete Link... menu items.
    */	

    if (current_linknet && current_source)
	sensitivity = _True;
    else
	sensitivity = _False;

    XtSetSensitive(menu->complete_link, sensitivity);
    XtSetSensitive(menu->complete_link_dialog, sensitivity);

    if (private->complete_link->dialog_on_screen)
        XtSetSensitive(menu->complete_link, _False);

    /*
    ** If the CurrencyChange callback is null, grey out the Highlight menu
    ** items.
    */

    _GetValue(private->dwui, _P_EnvironmentChangeCb, lwk_c_domain_routine,
	&callback);

    if (callback == (_Callback) _NullObject)
	sensitivity = _False;
    else
	sensitivity = _True;

    XtSetSensitive(menu->highlight, sensitivity);
    XtSetSensitive(menu->highlight_toggle, sensitivity);

    /*
    **  Set the Highlight Toggle to reflect reality
    */

    _GetValue(private->dwui, _P_ApplHighlight, lwk_c_domain_integer,
	&highlighting);


    ac = 0;
    
    if ((highlighting & lwk_c_hl_on) == 0) {
	XtSetArg(arglist[ac], XmNlabelString, menu->highlight_on_literal);
	ac++;
	XtSetArg(arglist[ac], XmNmnemonic, menu->highlight_on_keysym);
	ac++;
    }
    else {
	XtSetArg(arglist[ac], XmNlabelString, menu->highlight_off_literal);
	ac++;
	XtSetArg(arglist[ac], XmNmnemonic, menu->highlight_off_keysym);
	ac++;
    }

    XtSetValues(menu->highlight_toggle, arglist, ac);

    /*
    ** If any exceptions are raised, popup the message box.
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
static void  CreateGoTo(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->go_to = w;

    return;
    }

static void  CreateVisit(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->visit = w;

    return;
    }

static void  CreateShowHistory(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->show_history = w;

    return;
    }

static void  CreateStep(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->step = w;

    return;
    }

static void  CreateUndo(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->undo = w;

    return;
    }

static void  CreateSetSource(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->set_source = w;

    return;
    }


static void  CreateAnnotate(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->annotate = w;

    return;
    }

static void  CreateCompleteLink(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->complete_link = w;

    return;
    }
static void  CreateCompleteLinkDialog(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->complete_link_dialog = w;

    return;
    }

static void  CreateShowLinks(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->show_links = w;

    return;
    }

static void  CreateHighlight(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->highlight = w;

    return;
    }

static void  CreateHighlightToggle(w, menu, reason)
Widget w;
 _MenuWidgets menu;
 _Reason reason;

    {
    menu->highlight_toggle = w;

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
    _StartExceptionBlock


    /*
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_go_to, (_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(go_to_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

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
    _StartExceptionBlock

    /*
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_visit, (_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
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


static void  ActivateShowHistory(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_show_history, (_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_history_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateStep(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_step_forward, (_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(step_forward_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateUndo(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_go_back, (_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(go_back_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateSetSource(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_start_link, (_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(start_link_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateAnnotate(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_annotate, (_Closure) reason);
    
    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(annotate_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    } 


static void  ActivateCompleteLink(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_comp_link, (_Closure) reason);

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

static void  ActivateCompleteLinkDialog(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_comp_link_dialog,
	(_Closure) reason);

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


static void  ActivateShowLinks(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_show_links,
	(_Closure) reason);

    /*
    ** If any exceptions are raised, popup the message box.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    status[0] = _StatusCode(show_links_error);
	    status[1] = _Others;

	    _DisplayMessage(private->dwui, status, 2);

    _EndExceptionBlock

    return;
    }


static void  ActivateHighlight(w, private, reason)
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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_highlight_dialog,
	(_Closure) reason);

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
    ** Invoke the Ui SelectMenu operation.
    */

    _SelectMenu(private->dwui, lwk_c_dxm_menu_highlight_on_off,
	(_Closure) reason);

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


