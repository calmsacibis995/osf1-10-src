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
** COPYRIGHT (c) 1989, 1990, 1991, 1992 BY
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
**	LinkWorks Manager User Interface
**
**  Version: V1.0
**
**  Abstract:
**	Linkbase Window DECwindows User Interface routines.
**
**  Keywords:
**	{keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**	Andre Pavanello
**
**  Creation Date: 15-Nov-89
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
#include <X11/Shell.h>
#include <Xm/Protocols.h>            /* for XmAddWmProtocolCallback */

/*
** Macro Definitions
*/           
         
#define _LbDefaultWidthValue	"LbWindowDefaultWidth"
#define _LbDefaultHeightValue	"LbWindowDefaultHeight"
#define _SaveLinkbaseChanges	"MsgSaveLinkbaseChanges"

/*
**  Static Data Definitions
*/
static int	LbWindowIconSize = (int) 0;
static Pixmap	LbWindowIconPx = (Pixmap) 0;
static Pixmap	LbWindowIconifyPx = (Pixmap) 0;

/*
**  External Routine Declarations
*/

_DeclareFunction(_Void EnvShowPropInitialize, ());


/*
**  Forward Routine Declarations	
*/

_DeclareFunction(static _Void LbWindowSetIconsOnShell,
    (Widget shell, caddr_t user_data, XEvent *event,
	Boolean *continue_to_dispatch));
_DeclareFunction(static _Void LbWindowSetIcons,
    (Widget shell));
_DeclareFunction(static _WindowPrivate LbWindowCreatePrivateData,
    (_LbWindow lbwindow, _Widget parent));
_DeclareFunction(static _Void LbWindowDeletePrivateData,
    (_WindowPrivate windowprivate));

_DeclareFunction(static _Void UilCreateSvn,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateLinkMenu,
    (Widget w, _WindowPrivate private));
_DeclareFunction(static _Void UilMapFileMenu,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateSave,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateOpen,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateNew,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateSaveOnOpen_Yes,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateSaveOnOpen_No,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(_Void UilActivateSaveLbOnOpen,
    (_WindowPrivate private, _Boolean save_flag));
_DeclareFunction(static _Void UilActivateSave,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void WMActivateClose,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateClose,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void CloseLinkbaseWindow,
    (_WindowPrivate private, _Boolean recycle_flag));

_DeclareFunction(static _Void UilMapEditMenu,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateCut,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateCut,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateCopy,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateCopy,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreatePaste,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivatePaste,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateDelete,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateDelete,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateSelect,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void LbWindowInsertEntry,
    (_WindowPrivate private, lwk_object object,	
    _Boolean selection));

_DeclareFunction(static _Void UilMapViewMenu,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateExpand,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateExpand,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void SetShowPropButtonLabel,
    (_WindowPrivate private, _SelectData select_data));
_DeclareFunction(static _Void UilCreateShowProp,
        (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateShowProp,
        (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void LinkbaseSave,
    (_WindowPrivate windowprivate));
                                         
_DeclareFunction(static _Void UilActivateLbCloseYes,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateLbCloseNo,
    (Widget w, _WindowPrivate private, _Reason reason));
	
_DeclareFunction(static _Void UilActivateCreateCnet,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateCreateNet,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateCreateCpath,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateCreatePath,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void CreatePersistentObject,
    (_WindowPrivate private, lwk_domain domain));

_DeclareFunction(_Void CreateNewObject,
    (lwk_domain domain, _HsObject *hs_object));
_DeclareFunction(static _Void CutOrCopyToClipboard,
    (_WindowPrivate private, _Reason reason,
    _Boolean cut));

_DeclareFunction(static _Void UilCreateMenuBar,
    (Widget w, _WindowPrivate private, _Reason reason));
                                 
_DeclareFunction(static _Void EnvLbWMSaveYourself,
    (Widget w, _WindowPrivate private, _Reason reason));

/*
**  Static Data Definitions
*/

/*
**  MRM Names to register
*/

static MrmRegisterArg _Constant Register[] = {
    {"env_create_svn",		    (caddr_t) UilCreateSvn},
    {"env_svn_entry_transfer",	    (caddr_t) EnvSvnLbWindowEntryTransfer},
    {"env_svn_selections_dragged",  (caddr_t) EnvSvnLbWindowSelectionsDrag},
    
    {"env_create_link_menu",	    (caddr_t) UilCreateLinkMenu},
    {"env_context_sensitive_help",  (caddr_t) EnvHelpPopupHelp},
    {"env_help_on_context_tracking",(caddr_t) EnvHelpOnContextTracking},
    {"env_svn_help",		    (caddr_t) EnvHelpSvnHelp},
    
    {"lb_create_menubar",	    (caddr_t) UilCreateMenuBar},
    
    {"lb_map_file_menu",	    (caddr_t) UilMapFileMenu},
    {"lb_create_save",		    (caddr_t) UilCreateSave},
    {"lb_activate_open",	    (caddr_t) UilActivateOpen},
    {"lb_activate_new",		    (caddr_t) UilActivateNew},
    {"lb_save_linkbase",	    (caddr_t) UilActivateSave},
    {"lb_activate_close",	    (caddr_t) UilActivateClose},

    {"lb_map_edit_menu",	    (caddr_t) UilMapEditMenu},
    {"lb_create_cut",		    (caddr_t) UilCreateCut},
    {"lb_activate_cut",		    (caddr_t) UilActivateCut},
    {"lb_create_copy",		    (caddr_t) UilCreateCopy},
    {"lb_activate_copy",	    (caddr_t) UilActivateCopy},
    {"lb_create_paste",		    (caddr_t) UilCreatePaste},
    {"lb_activate_paste",	    (caddr_t) UilActivatePaste},
    {"lb_create_delete",	    (caddr_t) UilCreateDelete},
    {"lb_activate_delete",	    (caddr_t) UilActivateDelete},
    {"lb_activate_select",	    (caddr_t) UilActivateSelect},
    
    {"lb_map_view_menu",	    (caddr_t) UilMapViewMenu},
    {"lb_create_expand",	    (caddr_t) UilCreateExpand},
    {"lb_activate_expand",	    (caddr_t) UilActivateExpand},
    {"lb_create_properties",	    (caddr_t) UilCreateShowProp},
    {"lb_activate_properties",	    (caddr_t) UilActivateShowProp},
    
    {"lb_activate_create_net_list",    (caddr_t) UilActivateCreateCnet},
    {"lb_activate_create_linknet",     (caddr_t) UilActivateCreateNet},
    {"lb_activate_create_path_list",   (caddr_t) UilActivateCreateCpath},
    {"lb_activate_create_path",	       (caddr_t) UilActivateCreatePath},

    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


_WindowPrivate  EnvOpDWLbWindowCreate(lbwindow, parent)
_LbWindow lbwindow;
 _Widget parent;

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
    _WindowPrivate volatile private;
    int			    status;
    int			    count = 0;
    _CString		    title;
    _CString		    icon_name;
    char		    *filename;
    MrmType		    *class;
    MrmHierarchy	    hierarchy;
    MrmRegisterArg	    drm_register[1];
    Arg		  	    arglist[10];
    _Integer		    lb_width;
    _Integer		    lb_height;
    Atom		    delete_window_atom;        
    Atom		    save_yourself_atom;        
    
    filename = _LbWindowUidFileName;

    _StartExceptionBlock

    /*
    ** Check parent.
    */

    if (parent == (_Widget) 0)
	_Raise(inv_widget_id);
	
    /*
    ** Initialize private data.
    */

    private = LbWindowCreatePrivateData(lbwindow, parent);

    /*
    ** Register all MRM names.
    */

    MrmRegisterNames(Register, RegisterSize);
    EnvShowPropInitialize();

    /*
    **  Open the MRM hierarchy
    */

    hierarchy = EnvDWOpenDRMHierarchy(filename);

    /*
    ** Register private data.
    */

    EnvDWRegisterDRMNames(private);

    /*									    
    ** Create the shell for the linkbase window.
    */

    EnvDWGetIntegerValue(_LbDefaultWidthValue, &lb_width);
    EnvDWGetIntegerValue(_LbDefaultHeightValue, &lb_height);

    count = 0;    
    XtSetArg(arglist[count], XmNwidth, (Dimension) lb_width); count++;
    XtSetArg(arglist[count], XmNheight, (Dimension) lb_height); count++;
    
    private->shell = XtAppCreateShell("LinkWorks Manager Linkbase",
	"LinkWorks Manager", topLevelShellWidgetClass, XtDisplay(parent),
	(ArgList) arglist, (Cardinal) count);
	
    XtSetMappedWhenManaged(private->shell, FALSE);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(private->shell, StructureNotifyMask, False,
		      LbWindowSetIconsOnShell, None);
		      
    XtRealizeWidget(private->shell);

    /*
    ** Fetch the linkbase window.
    */

    status = MrmFetchWidget(hierarchy, _DrmLinkbaseWindow, private->shell,
	&private->main_widget, (MrmType *) &class);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    **  Set the window's title 
    */

    EnvDWGetCStringLiteral(_DwtLbWindowTitleBase, &title);
    EnvDWSetTitle(private->shell, title);
    _DeleteCString(&title);

    /*
    **  Fetch the SVN fonts
    */
                
    EnvSvnWindowLoadFont(private, hierarchy);

   /*
    **  Set the window icon title
    */

    EnvDWGetCStringLiteral(_DwtLbWindowIconName, &icon_name);
    EnvDWSetIconName(private->shell, icon_name);
    _DeleteCString(&icon_name);
	

    /*
    ** Close the MRM hierarchy.
    */

    EnvDWCloseDRMHierarchy(hierarchy);

    XtRealizeWidget(private->main_widget);

    /*
    ** No more UI object for the linkbase window
    **           EnvHisCreateDwUi(private, "LinkWorks Manager");
    */
    
    private->his_dwui = (lwk_dxm_ui) 0;

    /*
    **  Install the accelerators on the widgets which accept input focus
    */

    XtInstallAllAccelerators(private->main_widget, private->menubar);

    /*
    ** Position the linkbase window 
    */

    _PositionWindow(private->window);
    
    /*
    ** Add the quick copy event handler and new actions.
    */

    EnvSvnLbWindowAddActions(private);

    XtManageChild(private->main_widget);
    XtPopup(private->shell, XtGrabNone);

    /* 
    ** Register for protocols we're willing to participate in.
    **
    **	WM_DELETE_WINDOW - clients are notified when the MWM f.kill function
    **	    is invoked by the user.  MWM does not terminate the client or
    **	    destroy the window when a WM_DELETE_WINDOW notification is done.
    **
    **	WM_SAVE_YOURSELF - clients with this atom will be notified when a
    **	    session manager or a window manager wishes the window's state
    **	    to be changed.  The typical change is when the windmow is about
    **	    to be deleted or the session terminated.
    **	    
    */

    delete_window_atom = XmInternAtom(XtDisplay(private->shell),
	    "WM_DELETE_WINDOW", False);

    save_yourself_atom = XmInternAtom(XtDisplay(private->shell),
	    "WM_SAVE_YOURSELF", False);

    XmAddWMProtocolCallback(private->shell, delete_window_atom,
	(XtCallbackProc) WMActivateClose, (caddr_t) private);    

    XmAddWMProtocolCallback(private->shell, save_yourself_atom,
	(XtCallbackProc) EnvLbWMSaveYourself, (caddr_t) private);
	
    XmActivateWMProtocol(private->shell, delete_window_atom);

    XmActivateWMProtocol(private->shell, save_yourself_atom);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    if (private != (_WindowPrivate) _NullObject)
		_FreeMem(private);
            _Reraise;
	
    _EndExceptionBlock

    return private;
    }


_Void  EnvOpDWLbWindowOpen(private, width, height, x, y)
_WindowPrivate private;
 _Cardinal width;

    _Cardinal height;
 _Position x;
 _Position y;

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
    **  Just manage the window so that it appears on the display where the
    **	user left it
    */

    XtPopup(private->shell, XtGrabNone);  

    return;
    }


_Void  EnvOpDWLbWindowDisplay(private, linkbase)
_WindowPrivate private;
 _HsObject linkbase;

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
    _Integer 	position;
    _CString	lb_title;	
    _CString	linkbase_name;
    _CString	title;

    DXmSvnDisableDisplay(private->svn);
                                 
    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    /*
    ** Get the linkbase name.
    */

    _GetCSProperty(linkbase, lwk_c_p_name, &linkbase_name);
	
    /*
    ** Set the window title.
    */

    EnvDWGetCStringLiteral(_DwtLbWindowTitle, &lb_title);

    title = EnvDWConcatCString(lb_title, linkbase_name);
    
    EnvDWSetTitle(private->shell, title);
    
    /*
    ** Clean up.
    */
    
    _DeleteCString(&title);
    _DeleteString(&lb_title);

    /*
    ** Load in memory the directory listing for the linkbase.
    */

    EnvSvnLbWindowLoadLinkbase(private, &(private->svn_entries),
	linkbase);
	
    /*
    ** Tell SVN about the new linkbase.
    */

    EnvSvnLbWindowDisplayLb(private);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers
	
	    DXmSvnEnableDisplay(private->svn);
	    _SetCursor(private->window, _DefaultCursor);
	    
	    _Reraise;
		
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


_Void  EnvDWLbWindowOpenLinkbase(private, object)
_WindowPrivate private;
 _HsObject object;

/*
**++
**  Functional Description:
**	gets the linkbase id containing the given object,
**	opens the his linkbase and checks if it isn't already displayed before
**	opening it in a new window.
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
    _CString	cs_lb;
    _String	linkbase_name;
    _LbWindow	window;
    _Status	stat[2];
    lwk_status	status;
    lwk_object	linkbase;

    /*
    ** Set the new flag.
    */

    private->new_window = _True;

    /*
    ** Get the linkbase name.
    */

    _GetLinkbase(object, &cs_lb, &linkbase_name);

    /*
    ** Open the linkbase.
    */

    status = lwk_open(linkbase_name, lwk_c_false, &linkbase);

    if (status == lwk_s_success) {

	/*
	** Check if this linkbase isn't already displayed and raise the window.
	*/
	
	if (EnvOpLbWindowLbInWindow(&window, linkbase)) {
	
	    _RaiseWindow((_Window) window);
                                  
	    stat[0] = _StatusCode(lb_already_open);
	    stat[1] = _StatusCode(new_wnd_not_open);
	
	    _DisplayMessage((_Window) window, stat, 2);
	}
	else
	
	    /*
	    ** Open the linkbase window.
	    */
	
	    EnvDWLbWindowOpenLbInWindow(private, linkbase);
    }
    else {

	_SaveHisStatus(status);
	_Raise(open_lb_err); /* Open linkbase failed */
    }

    return;
    }
    

_Void  EnvDWLbWindowOpenLbInWindow(private, linkbase)
_WindowPrivate private;

    lwk_linkbase linkbase;

/*
**++
**  Functional Description:
**	Opens a linkbase in an existing window or a new one.
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
**--
*/
    {
    _LbWindow	    lb_window;
    _HsObject	    hs_linkbase;
    lwk_status	    status;
    lwk_linkbase  prev_linkbase;

    _StartExceptionBlock

    /*
    ** If new window has been asked, create it.
    */

    if (private->new_window) {

	/*
	** Create a linkbase window.
	*/
	
	lb_window = (_LbWindow) _Open(_TypeLbWindow, _DefaultWidth,
	    _DefaultHeight, _DefaultX, _DefaultY,  private->parent);

	/*
	** Create an HsObject for the linkbase.
	*/

	hs_linkbase = (_HsObject) _CreateHsObject(_TypeHsObject,
	    lwk_c_domain_linkbase, linkbase);
    }
    else {

	lb_window = (_LbWindow) private->window;

	/*
	** It's time to close the linkbase previously displayed in the
	** linkbase window
	*/

	_GetValue(lb_window, _P_Linkbase, hs_c_domain_hsobject,
	    &hs_linkbase);

	_GetValue(hs_linkbase, _P_HisObject, hs_c_domain_lwk_object,
	    &prev_linkbase);

	status = lwk_close(prev_linkbase);
	if (status != lwk_s_success)
	    _Raise(close_lb_err);

	status = lwk_delete(&prev_linkbase);

	/*
	** Clear the current linkbase window
	*/
	
	_Clear(lb_window);
    }

    _SetValue(hs_linkbase, _P_HisObject, hs_c_domain_lwk_object,
	&linkbase, hs_c_set_property);

    _SetValue(lb_window, _P_Linkbase, hs_c_domain_hsobject, &hs_linkbase,
	hs_c_set_property);

    /*
    ** Display the linkbase.
    */

    _Display(lb_window, hs_linkbase);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status stat[2];

	    if (hs_linkbase != (_HsObject) _NullObject)
		if (private->new_window)
		    _Delete(hs_linkbase);
		
	    stat[0] = _StatusCode(wnd_open_error);
	    stat[1] = _Others;
	
	    _DisplayMessage(private->window, stat, 2);
	
    _EndExceptionBlock

    return;
    }


_Void  EnvOpDWLbWindowClose(private)
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

    /*
    **  Unmanage the window
    */

    XtPopdown(private->shell);

    /*
    ** Dismiss all mapped Prop boxes.
    */
    
    EnvDWShowPropDismiss(private);

    /*
    **  Text widgets which need to be reset should be done here
    */

    return;
    }


_Void  EnvOpDWLbWindowDelete(private)
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

    if (private != (_WindowPrivate) _NullObject) {

	/*
	** Free the private data structure.
	*/

	LbWindowDeletePrivateData(private);
    }

    return;
    }


static _Void  LbWindowSetIconsOnShell(shell, user_data, event, continue_to_dispatch)
Widget shell;
 caddr_t user_data;

    XEvent *event;
 Boolean *continue_to_dispatch;

/*
**++
**  Functional Description:
**  
**      Callback routine which sets the icon pixmaps for Reparenting window
**	managers.
**
**  Keywords:
**      XUI, WindowManager
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
{
    Display 	    *dpy = XtDisplay(shell);
    Window  	    root_window = XDefaultRootWindow(dpy);
    XReparentEvent  *reparent = (XReparentEvent *) &event->xreparent;

    if (event->type != ReparentNotify)
    	return;

    /*
    ** Ignore reparents back to the root window.
    */
    if (reparent->parent == root_window)
    	return;

    /*
    ** Set the icons for this shell.
    */
    LbWindowSetIcons(shell);

    return;
}


static _Void  LbWindowSetIcons(shell)
Widget shell;

/*
**++
**  Functional Description:
**  
**      Sets the icon and iconify pixmaps for the given shell widget.
**
**  Keywords:
**      XUI, WindowManager
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
{
    _Integer		status;
    Dimension		width;
    Dimension		height;
    Display 	    	*dpy = XtDisplay(shell);
    Screen  	    	*scr = XtScreen(shell);
    unsigned int    	icon_size;
    char	    	*icon_name;
    static char     	*shell_icon_sizes[] = { "75", "50", "32", "17" };
    static int	    	num_sizes = XtNumber(shell_icon_sizes);

    /*
    ** Determine the icon pixmap name and size to fetch.
    */
    icon_name = EnvDWGetIconIndexName(dpy, _DwtLbWindowIcon, &icon_size, 
    	    	    	    	      shell_icon_sizes, num_sizes);
    if (icon_name != NULL) {

        /*
	** If the icon sizes are different we need to free the current ones, and
	** re-fetch new icons.  We assume that re-fetching new icons is an
	** infrequent operation, so we don't cache the old icons.
	*/

    	if ((LbWindowIconSize != 0)	    	    	/* Icon exists.     */
    	     && (LbWindowIconSize != icon_size))	/* New icon needed. */
    	{
    	    if (LbWindowIconPx)
    	    	XFreePixmap(dpy, LbWindowIconPx);
    	    if ((LbWindowIconifyPx)
		&& (LbWindowIconifyPx != LbWindowIconPx))
    	    	XFreePixmap(dpy, LbWindowIconifyPx);
    	    LbWindowIconPx = (Pixmap) 0;
    	    LbWindowIconifyPx = (Pixmap) 0;
    	    LbWindowIconSize = (int) 0;
    	}
	
    	if (LbWindowIconSize == 0)
    	{
    	    LbWindowIconSize = icon_size;
	    EnvDWFetchBitmap(shell, icon_name, &LbWindowIconPx);
    	}
	
    	XtFree(icon_name);
	
    	icon_name = NULL;
    }
    else    /* Can't get icon sizes for some reason */
    	return;

    /*
    ** Fetch the iconify pixmap for compatibility with the XUI window manager.
    */
    if (EnvDWIsXUIWMRunning(shell))
    {
    	if (icon_size == 17)  	    /* Don't fetch icon twice */
    	    LbWindowIconifyPx = LbWindowIconPx;
    	else if (icon_size > 17) {
	    icon_size = 17;
	    icon_name = (char *) XtMalloc(strlen(_DwtLbWindowIcon)  +
    	    	    	    	    	  sizeof("_")		    +
    	    	    	    	          sizeof("17")		    +
    	    	    	    	    	  1 );    /* for \0 char */
	    strcpy(icon_name, _DwtLbWindowIcon);
	    strcat(icon_name, "_");
	    strcat(icon_name, "17");

	    EnvDWFetchBitmap(shell, icon_name, &LbWindowIconifyPx);

	    XtFree(icon_name);
	    }
    }

    /* Set the icon pixmap on the shell.
     */
    if (LbWindowIconPx)
    {
    	if (XtWindow(shell) != 0)
    	{
    	    /* HACK: Under Motif 1.1 changing iconPixmap will cause the window 
    	    *  	 to go to its initial state.  This appears to be a side-effect 
    	    *  	 of ICCCM-compliant behavior, and doing XtSetValues in the
    	    *  	 X toolkit, so we need to call Xlib directly instead of 
    	    *  	 setting XtNiconPixmap. 
    	    */
    	    XWMHints    *wmhints = NULL;

    	    wmhints = XGetWMHints(dpy, XtWindow(shell));
    	    if (wmhints != NULL)
    	    {
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = LbWindowIconPx;
    	    	XSetWMHints(dpy, XtWindow(shell), wmhints);
    	    	XFree((_AnyPtr) wmhints);
    	    } 
    	    else
    	    {
	    	wmhints = (XWMHints *) XtCalloc(1, sizeof(XWMHints));
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = LbWindowIconPx;
    	    	XSetWMHints(dpy, XtWindow(shell), wmhints);
	    	XtFree((_AnyPtr) wmhints);
    	    }
    	}
    	else
    	{
    	    Arg	arglist[1];
    	    XtSetArg(arglist[0], XmNiconPixmap, LbWindowIconPx);
    	    XtSetValues(shell, arglist, 1);
    	}
    }

    /*
    ** Set the iconify pixmap for the XUI window manager
    */
    if (LbWindowIconifyPx)
    	EnvDWWmSetIconifyPixmap(shell, LbWindowIconifyPx);

    return;
}


static _Void  LbWindowInsertEntry(private, object, selection)
_WindowPrivate private;
 lwk_object object;

    _Boolean selection;

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
    lwk_status	status;
    _Domain	domain;
    _Integer    tmp_domain;
    lwk_object  persistent_object;

    /*
    ** Get the object domain.
    */

    status = lwk_get_domain(object, &domain);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_domain_failed); /*get domain failed */
    }

    /*
    ** If it's a persistent object.
    */

    if (domain != lwk_c_domain_object_desc)
	persistent_object = object;
	
    else {

	/*
	** Else retrieve the persistent object and its domain.
	*/

        status = lwk_retrieve(object, &persistent_object);
        if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(object_retrieve_failed);/* object retrieve failed */
	}
		
	/*
	** Get the domain of the object to insert.
	*/
	
	status = lwk_get_value(object, lwk_c_p_object_domain,
	    lwk_c_domain_integer, &tmp_domain);
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_domain_failed); /* get object domain failed */
	}
        domain = (_Domain) tmp_domain;
    }

    /*
    ** Insert the entry and its children into the display list.
    */

    EnvSvnLbWindowInsertEntry(private, persistent_object, domain, selection);

    return;
    }


static _WindowPrivate  LbWindowCreatePrivateData(lbwindow, parent)
_LbWindow lbwindow;

    _Widget parent;

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
    _WindowPrivate	windowprivate;
    _LbWindowPrivate	lbwindowprivate;

    _StartExceptionBlock

    /*
    ** Initialize.
    */

    windowprivate = (_WindowPrivate) 0;
    lbwindowprivate = (_LbWindowPrivate) 0;

    /*
    ** Create window object private data.
    */

    windowprivate =
	(_WindowPrivate) EnvDWWindowCreatePrivateData((_Window) lbwindow,
	parent);

    /*
    ** Allocate memory for private data.
    */

    lbwindowprivate = (_LbWindowPrivate)
		_AllocateMem(sizeof(_LbWindowPrivateInstance));
    _ClearMem(lbwindowprivate, sizeof(_LbWindowPrivateInstance));

    /*
    ** Initialize private data.
    */

    windowprivate->specific = (_AnyPtr) lbwindowprivate;

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    if (lbwindowprivate != (_LbWindowPrivate) _NullObject)
		_FreeMem(lbwindowprivate);
            _Reraise;
	
    _EndExceptionBlock

    return windowprivate;
    }


static _Void  LbWindowDeletePrivateData(windowprivate)
_WindowPrivate windowprivate;

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
    if (windowprivate != (_WindowPrivate) _NullObject) {
	    
	/*
	** Free the lbwindow private structure.
	*/

	_FreeMem(windowprivate->specific);

	/*
	** Free the window private structure.
	*/

	EnvDWWindowDeletePrivateData(windowprivate);
    }

    return;
    }


static _Void  UilCreateSvn(w, private, reason)
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

    /*
    ** Store svn widget id.
    */

    private->svn = w;

    /*
    **  Set Svn attributes for the linkbase window
    */

    EnvSvnLbWindowInitialize(private);

    return;
    }


static _Void  UilCreateMenuBar(w, private, reason)
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

    private->menubar = w;

    return;
    }


static _Void  UilCreateLinkMenu(w, private)
Widget w;
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

    /*
    ** Store connection menu widget id.
    */

    private->connection_menu = w;

    return;
    }


static _Void  UilActivateOpen(w, private, reason)
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
    _Boolean	new = _False;
    _Boolean	modified;
    _HsObject	hs_linkbase;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    _GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	    &hs_linkbase);

    /*
    ** Check if the linkbase has been modified.
    */

    modified = _SetHsObjState(hs_linkbase, _StateModified, _StateGet);

    /*
    ** Ask the user if it should be saved.
    */

    if (modified)
	EnvDWQuestion(private, UilActivateSaveOnOpen_Yes,
	    UilActivateSaveOnOpen_No, _SaveLinkbaseChanges);

    else {
   
	/*
	** Popup the linkbase box.
	*/

	EnvDWLbBox(private, new);
    }

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(open_lb_err);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateNew(w, private, reason)
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
    _Boolean	new = _True;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    /*
    ** Popup the linkbase box.
    */

    EnvDWLbBox(private, new);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(wnd_open_error);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateSaveOnOpen_Yes(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {

    UilActivateSaveLbOnOpen(private, _True);

    return;
    }

static _Void  UilActivateSaveOnOpen_No(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {

    UilActivateSaveLbOnOpen(private, _False);

    return;
    }


_Void  UilActivateSaveLbOnOpen(private, save_flag)
_WindowPrivate private;
 _Boolean save_flag;

/*
**++
**  Functional Description:
**	Callback routine.
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
**--
*/
    {
    _Boolean    new = _False;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    XtUnmanageChild(private->question_box);

    /*
    ** Restore the default cursor
    */
    _SetCursor(private->window, _DefaultCursor);

    /*
    ** Save the current linkbase.
    */

    if (save_flag)
	LinkbaseSave(private);

    /*
    ** Now proceed with the Open request
    */

    EnvDWLbBox(private, new);
	
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(save_lb_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateSave(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

/*
**++
**  Functional Description:
**  
**	Clients receiving a WM_DELETE_WINDOW message should behave as if
**	the user selected "delete window" from a (hypothetical) menu.  They
**	should perform any confirmation dialog with the user.
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

    _SetCursor(private->window, _WaitCursor);

    /*
    ** Save the current linkbase if modified
    */

    LinkbaseSave(private);
	
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(save_lb_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  WMActivateClose(w, private, reason)
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

    _StartExceptionBlock

    CloseLinkbaseWindow(private, _False);
    
    /*
    ** Exceptions handling
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(close_lb_err);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }


static _Void  UilActivateClose(w, private, reason)
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

    _StartExceptionBlock

    CloseLinkbaseWindow(private, _True);
    
    /*
    ** Exceptions handling
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(close_lb_err);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }


static _Void  CloseLinkbaseWindow(private, recycle_flag)
_WindowPrivate private;

    _Boolean recycle_flag;

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
    _HsObject	hs_linkbase;
    _Boolean	modified;

    /*
    ** Get the linkbase object displayed in the window
    */

    _GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	    &hs_linkbase);

    /*
    ** Check if the linkbase has been modified.
    */

    modified = _SetHsObjState(hs_linkbase, _StateModified, _StateGet);

    /*
    ** Ask the user if it should be saved.
    */

    if (modified)

	EnvDWQuestion(private, UilActivateLbCloseYes, UilActivateLbCloseNo,
	    _SaveLinkbaseChanges);

    else {

	/*
	** Else close the linkbase window.
	*/

	if (private->fileselection != (Widget) 0)
	    XtUnmanageChild(private->fileselection);
	
	_Close((_LbWindow)(private->window), recycle_flag);
    }

    return;
    }


static _Void  UilMapEditMenu(w, private, reason)
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
    _SelectData select_data;
    Arg		arglist[2];
    Time	timestamp;

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    /*
    ** Set the buttons to insensitive.
    */

    EnvDWSetEditButtons(private, _False);

    /*
    ** If there is a single selection and the right components have been
    ** selected, set the cut, copy and delete buttons to sensitive.
    */

    if (_GetSelection(private->window, _SingleSelection, &select_data)) {

	if (select_data->svn_data[0]->object != _NullObject) {
    	
	    if (select_data->component[0] == _LbSvnIconComponent||
		select_data->component[0] == _LbSvnNameComponent) {
		if (select_data->svn_data[0]->retrievable == _Retrievable)
		    EnvDWSetEditButtons(private, _True);
	    }
	}
     }

    /*
    ** Check if there is something into the clipboard and if it has the right
    ** format to set the paste button.
    */

    EnvDWGetTime(reason, &timestamp);

    if (EnvDWClipboardInquireNextPaste(private, timestamp))
	XtSetArg(arglist[0], XmNsensitive, _True);
    else
	XtSetArg(arglist[0], XmNsensitive, _False);

    XtSetValues(private->paste_button, arglist, 1);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    /*
    ** Check if there is a multiple selection to set the delete button to
    ** sensitive.
    */
    
    if (_GetSelection(private->window, _MultipleSelection, &select_data)) {
	XtSetArg(arglist[0], XmNsensitive, _True);
	XtSetValues(private->delete_button, arglist, 1);
    }
    
    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions

	_When(clip_locked)
	    XtSetArg(arglist[0], XmNsensitive, _False);
	    XtSetValues(private->paste_button, arglist, 1);
	
        _WhenOthers
	    _Status status[2];
	
	    if (select_data != (_SelectData) 0)
		EnvSvnWindowFreeSelection(select_data);
	    	
	    status[0] = _StatusCode(map_menu_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Void  UilCreateCut(w, private, reason)
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

    /*
    ** Store cut button id.
    */

    private->cut_button = w;

    return;
    }


static _Void  UilActivateCut(w, private, reason)
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

    _SetCursor(private->window, _WaitCursor);

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    CutOrCopyToClipboard(private, reason, _True);

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(cut_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilCreateCopy(w, private, reason)
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

    /*
    ** Store copy button id.
    */

    private->copy_button = w;

    return;
    }


static _Void  UilActivateCopy(w, private, reason)
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

    _SetCursor(private->window, _WaitCursor);

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    CutOrCopyToClipboard(private, reason, _False);

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(copy_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilCreatePaste(w, private, reason)
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

    /*
    ** Store paste button id.
    */

    private->paste_button = w;

    return;
    }


static _Void  UilActivatePaste(w, private, reason)
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
    lwk_object	object;
    Time	timestamp;

    _SetCursor(private->window, _WaitCursor);

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    /*
    ** Get the data from the clipboard.
    */
    EnvDWGetTime(reason, &timestamp);

    EnvDWClipboardCopyFromClipboard(private, &object, timestamp);

    /*
    ** Insert it in the right place.
    */

    LbWindowInsertEntry(private, object, _True);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(paste_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilCreateDelete(w, private, reason)
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

    /*
    ** Store delete button id.
    */

    private->delete_button = w;

    return;
    }


static _Void  UilActivateDelete(w, private, reason)
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
    _SelectData	    select_data;
    _SvnData        last_l1_deleted;
    _SvnData        svn_data;
    _Integer	    count,
		    index;
    Time	    timestamp;

    _SetCursor(private->window, _WaitCursor);

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    /*
    ** Get the selected data.
    */

    _GetSelection(private->window, _MultipleSelection, &select_data);

    /*
    ** Remove the entries from the display.
    */

    EnvDWGetTime(reason, &timestamp);

    count = select_data->count;
    index = 0;

    /*
    ** Keep track of the last level 1 entry deleted. If the next selected entry
    ** has this level 1 entry as parent, you don't need to remove it, it has
    ** already been removed with its parent.
    */
    
    last_l1_deleted = (_SvnData) 0;
    
    while (count > 0) {
	count --;
	svn_data = (_SvnData) select_data->svn_data[index];
	
	if (svn_data->parent != last_l1_deleted) {
	
	    if (svn_data->child_level == 2)
		last_l1_deleted = svn_data;

	    EnvSvnLbWindowRemoveEntry(private, svn_data, timestamp);
	}
	
	index++;
    }
	
    /*
    ** Free the selected data structure.
    */

    EnvSvnWindowFreeSelection(select_data);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(delete_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateSelect(w, private, reason)
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

    _StartExceptionBlock

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(select_cb_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }


static _Void  UilMapFileMenu(w, private, reason)
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
    _HsObject	hs_linkbase;
    _Boolean	modified;
    Arg		arglist[1];

    _StartExceptionBlock

    /*
    ** Get the linkbase object displayed in the window
    */

    _GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	    &hs_linkbase);

    /*
    ** Check if the linkbase has been modified.
    */

    modified = _SetHsObjState(hs_linkbase, _StateModified, _StateGet);

    /*
    ** Ask the user if it should be saved.
    */

    if (modified)
	XtSetArg(arglist[0], XmNsensitive, _True);
    else
	XtSetArg(arglist[0], XmNsensitive, _False);

    XtSetValues(((_LbWindowPrivate)(private->specific))->save_button, arglist,
	1);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(map_menu_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }


static _Void  UilMapViewMenu(w, private, reason)
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
    _SelectData select_data;

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    /*
    ** Get the current selection.
    */

    _GetSelection(private->window, _SingleSelection, &select_data);

    /*
    **  Set the state for the Expand button
    */

    EnvDWSetExpandButtonLabel(private, select_data);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    _GetSelection(private->window, _MultipleSelection, &select_data);

    /*
    **  Set the state for the Show Properties button
    */

    SetShowPropButtonLabel(private, select_data);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    DXmSvnEnableDisplay(private->svn);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(map_menu_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }


static _Void  SetShowPropButtonLabel(private, select_data)
_WindowPrivate private;

    _SelectData select_data;

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
    Arg		arglist[1];
    _SvnData	tag;

    XtSetArg(arglist[0], XmNsensitive, TRUE);

    /*
    **  If nothing is selected leave it on for linkbase
    */

    if (select_data != (_SelectData) 0) {

	if (select_data->count > 1)

	    /*
	    **  If more than one selection, turn it off
	    */

	    XtSetArg(arglist[0], XmNsensitive, FALSE);

	else {
	
	    /*
	    **  Check if the object selected is legal
	    */
	
	    tag = select_data->svn_data[0];

	    if (tag->object == ((_HsObject) _NullObject) ||
		tag->retrievable != _Retrievable)
	
		XtSetArg(arglist[0], XmNsensitive, FALSE);
	}
    }

    XtSetValues(private->showprop_button, arglist, 1);

    return;
    }


static _Void  UilCreateShowProp(w, private, reason)
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

    /*
    ** Store show properties button id.
    */

    private->showprop_button = w;

    return;
    }


static _Void  UilActivateShowProp(w, private, reason)
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
    _SelectData	    select_data;
    _SvnData	    entry;
    _HsObject	    hs_object;

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    _GetSelection(private->window, _SingleSelection, &select_data);

    if (select_data == (_SelectData) 0)

	/*
	**  Request for the linkbase properties
	*/

	_GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	    &hs_object);
	
    else {
	entry = select_data->svn_data[0];
	hs_object = entry->object;
    }

    /*
    **  Call the show properties operation for the object
    */

    _DisplayProperties(hs_object, private, _False);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(show_prop_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilCreateExpand(w, private, reason)
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

    /*
    ** Store the expand button.
    */

    private->expand_button = w;

    return;
    }


static _Void  UilCreateSave(w, private, reason)
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

    /*
    ** Store the save button
    */

    ((_LbWindowPrivate) (private->specific))->save_button = w;

    return;
    }


static _Void  UilActivateExpand(w, private, reason)
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
    _SelectData	    select_data;
    _IsExpandable   expand;
    _Integer	    num;

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    /*
    ** Get the current selection.
    */

    if (_GetSelection(private->window, _SingleSelection, &select_data)) {

	/*
	** Check if it's expandable or not.
	*/
	
	expand = EnvSvnWindowIsExpandable(private, select_data->svn_data[0],
	    select_data->entry[0], select_data->component[0]);
		
	switch (expand) {
	
	    case _ExpandEntry:
		_Expand(private->window, select_data->svn_data[0]);
		break;
		
	    case _CollapseEntry:
		_Collapse(private->window, select_data->svn_data[0]);
		break;
		
	    default:
		break;
	}
	
	if (select_data != (_SelectData) 0)
	    EnvSvnWindowFreeSelection(select_data);
    }

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    if (select_data != (_SelectData) 0)
		EnvSvnWindowFreeSelection(select_data);
		
	    status[0] = _StatusCode(expand_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }



static _Void  LinkbaseSave(private)
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
    _Boolean	    modified;
    _HsObject	    linkbase;
    _SvnData	    entry;
    _SvnData	    tmp;
    _EnvWindow	    env_window;
    _Boolean	    ignored;

    /*
    **	Get the linkbase object displayed in the window
    */

    _GetValue(private->window, _P_Linkbase, hs_c_domain_hsobject,
	&linkbase);

    modified = _SetHsObjState(linkbase, _StateModified, _StateGet);

    if (modified) {

	env_window = (_EnvWindow) _GetEnvWindow(private->window);

	/*
	**  Save the persistent object that have been modified
	*/

	entry = (_SvnData)(private->svn_entries)->children;
	
	while (entry != (_SvnData) 0) {

	    modified = _SetHsObjState(entry->object, _StateModified, _StateGet);
		
	    if (modified) {
	
		/*
		**  Update the navigation window as well
		*/

		_Update(env_window, entry->object, _ModifiedObject);

		_Save(entry->object, linkbase, _StoreObject);
	    }
		
	    entry = entry->siblings;

	}

	/*
	**  Delete the entries that have been deleted
	*/

	entry = private->deleted_entries;
	
	while (entry != (_SvnData) 0) {

	    _Update(private->window, entry->object, _DeletedObject);
	    
	    /*
	    **  Update the environment window as well
	    */

	    _Update(env_window, entry->object, _DeletedObject);

	    _Save(entry->object, linkbase, _DeleteObject);

	    tmp = entry;
	    entry = entry->siblings;
	    EnvSvnWindowFreeSvnData(tmp);
	}

	private->deleted_entries = (_SvnData) 0;
	
	_SetHsObjState(linkbase, _StateModified, _StateClear);
    }

    return;
    }


static _Void  UilActivateCreateCnet(w, private, reason)
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

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    CreatePersistentObject(private, lwk_c_domain_comp_linknet);


    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(persobj_creation_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Void  UilActivateCreateNet(w, private, reason)
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

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    CreatePersistentObject(private, lwk_c_domain_linknet);


    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(persobj_creation_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Void  UilActivateCreateCpath(w, private, reason)
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

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    CreatePersistentObject(private, lwk_c_domain_comp_path);


    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(persobj_creation_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Void  UilActivateCreatePath(w, private, reason)
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

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    CreatePersistentObject(private, lwk_c_domain_path);


    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(persobj_creation_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Void  CreatePersistentObject(private, domain)
_WindowPrivate private;
 lwk_domain domain;

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
    _HsObject	    hs_persistent;

    /*
    ** Create the new persistent object.
    */
    CreateNewObject(domain, &hs_persistent);
    
    /*
    **	Display the show properties box so the user can set the properties
    */

    _DisplayProperties(hs_persistent, private, _True);

    return;
    }


_Void  CreateNewObject(domain, hs_persistent)
lwk_domain domain;
 _HsObject *hs_persistent;

/*									    
**++								    
**  Functional Description:					    
**      Create a new persistent object and set the date and the author
**      properties, and create the corresponding HS object.    
**							    
**  Keywords:						    
**      {@keyword-list-or-none@}			    
**							    
**  Arguments:					    
**      {@identifier-list-or-none@}			    
**  [@non-local-references@]			    
**  [@pre-and-post-conditions@]			    
**						    
**  Result:					    
**      {@return-value-list-or-none@}		    
**						    
**  Exceptions:				    
**      {@identifier-list-or-none@}		    
**--
*/
    {
    _CString	    cstring;
    _DDIFString	    ddif_str;
    _Date	    date;
    lwk_persistent  persistent;
    lwk_ddif_string lwk_ddifstr;
    lwk_status	    status;

    /*
    **  Create the his persistent object
    */

    status = lwk_create(domain, &persistent);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(persobj_creation_failed);    /* object creation failed */
    }

    /*
    **  Set the creation date property
    */

    date = _NowToDate;
    status = lwk_set_value(persistent, lwk_c_p_creation_date, lwk_c_domain_date,
	&date, lwk_c_set_property);

    _DeleteDate(&date);

    /*
    **  Set the username property
    */

    cstring = _UserNameToCString;
    ddif_str = _CStringToDDIFString(cstring);
    lwk_ddifstr = (lwk_ddif_string) ddif_str;
    
    status = lwk_set_value(persistent, lwk_c_p_author,
	lwk_c_domain_ddif_string, &lwk_ddifstr, lwk_c_set_property);

    _DeleteCString(&cstring);
    _DeleteDDIFString(&ddif_str);

    /*
    **  Create the corresponding HS object
    */

    *hs_persistent = (_HsObject) _CreateHsObject(_TypeHsObject, domain,
	persistent);

    _SetHsObjState(*hs_persistent, _StateModified, _StateSet);

    return;
    }
    

static _Void  CutOrCopyToClipboard(private, reason, cut)
_WindowPrivate private;
 _Reason reason;

_Boolean cut;

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
    _SelectData	    select_data = (_SelectData) 0;
    Time	    timestamp;

    _StartExceptionBlock

    /*
    ** Get the selected data.
    */

    _GetSelection(private->window, _SingleSelection, &select_data);

    /*
    ** Get the timestamp.
    */

    EnvDWGetTime(reason, &timestamp);

    /*
    ** Copy the selected data to the clipboard.
    */

    EnvDWClipboardCopyToClipboard(private, select_data, timestamp);

    /*
    ** Remove the entry from the display if cut.
    */

    if (cut)
	EnvSvnLbWindowRemoveEntry(private, select_data->svn_data[0], timestamp);
	
    /*
    ** Free the selected data structure.
    */

    EnvSvnWindowFreeSelection(select_data);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	
	    if (select_data != (_SelectData) 0)
		EnvSvnWindowFreeSelection(select_data);
	    _Reraise;
	    	
    _EndExceptionBlock

    return;
    }


static _Void  UilActivateLbCloseYes(w, private, reason)
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
    _StartExceptionBlock
   
    XtUnmanageChild(private->question_box);

    /*
    ** Restore the default cursor
    */
    _SetCursor(private->window, _DefaultCursor);

    /*
    ** Save the current linkbase.
    */
    
    LinkbaseSave(private);

    /*
    **  Unmap the file selection if mapped
    */

    if (private->fileselection != (Widget) 0)
	XtUnmanageChild(private->fileselection);

    /*
    ** Close the linkbase window.
    */

    _Close((_LbWindow)(private->window), _True);

    /*
    ** If any exception is raised, display the error message.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(close_lb_err);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }
    
    
static _Void  UilActivateLbCloseNo(w, private, reason)
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
    _StartExceptionBlock
   
    XtUnmanageChild(private->question_box);

    /*
    ** Restore the default cursor
    */
    _SetCursor(private->window, _DefaultCursor);

    _Close((_LbWindow)(private->window), _True);

    /*
    ** If any exception is raised, display the error message.
    */

    _Exceptions
        _WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(close_lb_err);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    return;
    }


static _Void  EnvLbWMSaveYourself(w, private, reason)
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
    char    **argv;
    int     argc = 1;
    static char *argv_const[]={"lwkmanager","FOO"};
    
    LinkbaseSave(private);

    /*									    
    **	Let the Window Mgr or Session Mgr know that we're done
    */

    if(XGetCommand(XtDisplay(private->shell),
	XtWindow(private->shell), &argv, &argc)) {

	XSetCommand(XtDisplay(private->shell),
	    XtWindow(private->shell), argv, argc);

	XFlush(XtDisplay(private->shell));

    } else {

	XSetCommand(XtDisplay(private->shell),
	    XtWindow(private->shell), argv_const, 1);
	    
	XFlush(XtDisplay(private->shell));
    }

    return;
    }


_Void  EnvDwLbWindowSetCursor(private, cursor_type)
_WindowPrivate private;
 _CursorType cursor_type;

/*
**++
**  Functional Description:
**	Set the cursor to the specified cursor type on all windows associated
**	with the linkbase window.
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
    /* Set the cursor on the main window */
						 
    EnvDwSetCursor(private->main_widget, cursor_type);

						 
    /* Set the cursor on all subordinate windows */

    EnvDwLbBoxSetCursor(private, cursor_type);

    EnvShowPropSetCursor(private, cursor_type);

    return;
}
