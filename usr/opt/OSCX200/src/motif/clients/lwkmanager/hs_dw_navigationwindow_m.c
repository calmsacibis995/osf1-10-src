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
**	Environment Manager Window DECwindows User Interface routines.
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
**	Pascale Dardailler
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

#define _SaveCustomChangesMessage "MsgSaveCustomChanges"

/*
**  Type Definitions
*/
typedef struct __IndexPath {
    _SvnData		entry;
    struct __IndexPath	*next;
    } _IndexPathInstance, *_IndexPath;

typedef struct __LinknetList {
    lwk_linknet		    linknet;
    struct __LinknetList    *next;
    } _LinknetListInstance, *_LinknetList;

/*
**  Static Data Definitions
*/
static int	EnvWindowIconSize = (int) 0;
static Pixmap	EnvWindowIconPx = (Pixmap) 0;
static Pixmap	EnvWindowIconifyPx = (Pixmap) 0;
static XtAppContext AppContext = (XtAppContext) 0;

/*
**  External routine declarations
*/

_DeclareFunction(_Void EnvShowPropInitialize, ());

/*
**  Forward Routine Declarations	
*/

_DeclareFunction(static _Void EnvWindowSetIconsOnShell,
    (Widget shell, caddr_t user_data, XEvent *event,
	Boolean *continue_to_dispatch));
_DeclareFunction(static _Void EnvWindowSetIcons,
    (Widget shell));
_DeclareFunction(static _WindowPrivate EnvWindowCreatePrivateData,
    (_EnvWindow envwindow, _Widget parent));
_DeclareFunction(static _Void EnvWindowDeletePrivateData,
    (_WindowPrivate windowprivate));

_DeclareFunction(static _Void UilCreateSvn,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateLinkMenu,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateOpen,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateExit,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilMapControlMenu,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Integer CheckRecordMenuState,
    (_EnvWindowPrivate env_private, _SelectData select_data));
_DeclareFunction(static _Integer CheckActivateMenuState,
    (_EnvWindowPrivate env_private, _SelectData select_data));

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
_DeclareFunction(static _Void EnvWindowInsertEntry,
    (_WindowPrivate private, lwk_object object));
_DeclareFunction(static _Void CutOrCopyToClipboard,
    (_WindowPrivate private, _Reason reason,
    _Boolean cut));

_DeclareFunction(static _Void UilActivateActivate,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateRecord,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateRecordBtn,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateActivateBtn,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void GetActivePaths,
    (_SvnData entry, lwk_list *list));
_DeclareFunction(static _Void CollectActivePathEntry,
    (_IndexPath *index_path, _SvnData entry, lwk_list list));
_DeclareFunction(static _Void InsertHierarchyObjDesc,
    (_IndexPath index_path, _SvnData entry, lwk_list list));
_DeclareFunction(static _Void AddEntryInIndexPath,
    (_IndexPath *index_path, _SvnData entry));
_DeclareFunction(static _Void RemoveEntryFromIndexPath,
    (_IndexPath *index_path));
_DeclareFunction(static _Void GetActiveNets,
    (_SvnData entry, lwk_set *set));
_DeclareFunction(static _Void CollectActiveEntry,
    (_SvnData entry, _LinknetList *linknet_list, lwk_list list));
_DeclareFunction(static _Void FreeLinknetList,
    (_LinknetList linknet_list));

_DeclareFunction(static _Void UilMapViewMenu,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void SetShowPropButtonLabel,
    (_WindowPrivate private, _SelectData select_data));
_DeclareFunction(static _Void SetUpdateButton,
    (_WindowPrivate private, _SelectData select_data));

_DeclareFunction(static _Void UilCreateExpandBtn,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateShowPropBtn,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateUpdateBtn,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateShowProp,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateExpand,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateUpdate,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateMenuBar,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void UilActivateSessionAttr,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateEnvAttr,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateSaveAttr,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void SaveAttributes, (_WindowPrivate private));
_DeclareFunction(static _Void UilActivateRestoreAttr,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateRestoreSysAttr,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(_Void EnvCustEnvDisplayEnvironment,
    (_WindowPrivate private));
_DeclareFunction(_Void EnvCustHypDisplayLinkWorksMgr,
    (_WindowPrivate private));
_DeclareFunction(_Void EnvCustInitialize, ());

_DeclareFunction(static _Void SetWindowGeometryAttr,
    (_WindowPrivate private));

_DeclareFunction(static _Void ResetWindowGeometryAttr,
    (_WindowPrivate private));

_DeclareFunction(static _Void FinalCleanup,
    (_WindowPrivate private));
_DeclareFunction(static _Void UilActivateSaveCustom_Yes,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateSaveCustom_No,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void EnvEnvWMDeleteWindow,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void EnvEnvWMSaveYourself,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static void EnvEnvWarningHandler,
    (String message));

_DeclareFunction(_Void EnvCustSetCursor,
    (_WindowPrivate private, _CursorType cursor_type));
_DeclareFunction(_Void EnvDwLbBoxSetCursor,
    (_WindowPrivate private, _CursorType cursor_type));

/*
**  Static Data Definitions
*/

/*
**  MRM Names to register
*/

static MrmRegisterArg _Constant Register[] = {

    {"env_create_svn",		    (caddr_t) UilCreateSvn},
    {"env_svn_entry_transfer",	    (caddr_t) EnvSvnEnvWindowEntryTransfer},
    {"env_svn_selections_dragged",  (caddr_t) EnvSvnEnvWindowSelectionsDrag},
    {"env_create_link_menu",	    (caddr_t) UilCreateLinkMenu},
    {"env_context_sensitive_help",  (caddr_t) EnvHelpPopupHelp},
    {"env_help_on_context_tracking",(caddr_t) EnvHelpOnContextTracking},
    {"env_svn_help",                (caddr_t) EnvHelpSvnHelp},
    
    {"env_create_menubar",	    (caddr_t) UilCreateMenuBar},

    {"env_activate_open",	    (caddr_t) UilActivateOpen},
    {"env_activate_exit",	    (caddr_t) UilActivateExit},

    {"env_map_control_menu",	    (caddr_t) UilMapControlMenu},
    {"env_create_activate_btn",	    (caddr_t) UilCreateActivateBtn},
    {"env_create_record_btn",	    (caddr_t) UilCreateRecordBtn},
    {"env_activate_activate",	    (caddr_t) UilActivateActivate},
    {"env_activate_record",	    (caddr_t) UilActivateRecord},

    {"env_map_edit_menu",	    (caddr_t) UilMapEditMenu},
    {"env_create_cut",		    (caddr_t) UilCreateCut},
    {"env_activate_cut",	    (caddr_t) UilActivateCut},
    {"env_create_copy",		    (caddr_t) UilCreateCopy},
    {"env_activate_copy",	    (caddr_t) UilActivateCopy},
    {"env_create_paste",	    (caddr_t) UilCreatePaste},
    {"env_activate_paste",	    (caddr_t) UilActivatePaste},
    {"env_create_delete",	    (caddr_t) UilCreateDelete},
    {"env_activate_delete",	    (caddr_t) UilActivateDelete},

    {"env_map_view_menu",	    (caddr_t) UilMapViewMenu},
    {"env_create_properties_btn",	    (caddr_t) UilCreateShowPropBtn},
    {"env_activate_properties",	    (caddr_t) UilActivateShowProp},
    {"env_create_expand_btn",	    (caddr_t) UilCreateExpandBtn},
    {"env_activate_expand",	    (caddr_t) UilActivateExpand},
    {"env_create_update_btn",	    (caddr_t) UilCreateUpdateBtn},
    {"env_activate_update",	    (caddr_t) UilActivateUpdate},

    {"env_activate_manager_attr", (caddr_t) UilActivateSessionAttr},
    {"env_activate_environment_attr", (caddr_t) UilActivateEnvAttr},
    {"env_activate_save_attr",	    (caddr_t) UilActivateSaveAttr},
    {"env_activate_restore_attr",   (caddr_t) UilActivateRestoreAttr},
    {"env_activate_restore_sys_attr", (caddr_t) UilActivateRestoreSysAttr},

    };

static MrmCount _Constant RegisterSize = XtNumber(Register);


_WindowPrivate  EnvOpDWEnvWindowCreate(envwindow, env_context, parent)
_EnvWindow envwindow;

    _EnvContext env_context;
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
    _Integer	    status;
    MrmType	    *class;
    MrmHierarchy    hierarchy;
    MrmRegisterArg  drm_register[1];
    XWMHints	    hints;
    Arg		    arglist[15];
    _CString	    title;
    _CString	    icon_name;
    int		    count = 0;
    lwk_integer	    width;
    lwk_integer	    height;
    lwk_integer	    x_pos;
    lwk_integer	    y_pos;
    lwk_integer	    split;
    lwk_boolean	    iconized;
    Atom	    delete_window_atom;        
    Atom	    save_yourself_atom;        

    _StartExceptionBlock

    /*
    ** Check parent
    */

    if (parent == (_Widget) 0)
	_Raise(inv_widget_id);
	
    /*
    ** Initialize private data.
    */

    private = EnvWindowCreatePrivateData(envwindow, parent);

    /*
    ** Register all MRM names.
    */

    MrmRegisterNames(Register, RegisterSize);
    EnvShowPropInitialize();
    EnvCustInitialize();

    /*
    **  Open the MRM hierarchy
    */

    hierarchy = EnvDWOpenDRMHierarchy(_EnvWindowUidFileName);

    /*
    ** Register private data.
    */                              

    EnvDWRegisterDRMNames(private);

    /*
    **  Get the geometry attributes
    */

    _GetAttribute(env_context, _EnvXPositionAttr, &x_pos);
    _GetAttribute(env_context, _EnvYPositionAttr, &y_pos);
    _GetAttribute(env_context, _EnvWidthAttr, &width);
    _GetAttribute(env_context, _EnvHeightAttr, &height);

    XtSetArg(arglist[count], XmNx, (Position) x_pos); count++;
    XtSetArg(arglist[count], XmNy, (Position) y_pos); count++;
    XtSetArg(arglist[count], XmNwidth, (Dimension) width); count++;
    XtSetArg(arglist[count], XmNheight, (Dimension) height); count++;

    private->shell = XtAppCreateShell("LinkWorks Manager Environment",
	"LinkWorks Manager",
	topLevelShellWidgetClass, XtDisplay(parent), (ArgList) arglist,
	(Cardinal) count);

    XtSetMappedWhenManaged(private->shell, FALSE);

    /*
    ** Add an event handler to track Reparent notify events
    */
    XtAddEventHandler(private->shell, StructureNotifyMask, False,
		      EnvWindowSetIconsOnShell, None);

    XtRealizeWidget(private->shell);
    
    /*
    ** Fetch the environment window.
    */

    status = MrmFetchWidget(hierarchy, _DrmEnvironmentWindow, private->shell,
	&private->main_widget, (MrmType *) &class);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    **  Set the window's title with copyright
    */

    EnvDWGetCStringLiteral(_DwtEnvWinCopyrightTitle, &title);
    EnvDWSetTitle(private->shell, title);
    _DeleteCString(&title);
    
    /*
    **  Set the window icon title
    */

    EnvDWGetCStringLiteral(_DwtEnvWindowIconName, &icon_name);
    EnvDWSetIconName(private->shell, icon_name);
    _DeleteCString(&icon_name);


    /*
    **  Startup iconized?
    */

    _GetAttribute(env_context, _EnvIconizedAttr, &iconized);

    if (iconized) {
	count = 0;
	XtSetArg(arglist[count], XmNiconic, True); count++;
	XtSetValues(private->shell, arglist, count);

	if (EnvDWIsXUIWMRunning(private->shell)) {
	    hints.flags = StateHint;
	    hints.initial_state = IconicState;
	    XSetWMHints(XtDisplay(private->shell), XtWindow(private->shell),
		&hints);
	}
    }

    /* 
    ** Register for protocols we're willing to participate in.
    **
    **	WM_DELETE_WINDOW - clients are notified when the MWM f.kill function
    **	    is invoked by the user.  MWM does not terminate the client or
    **	    destroy the window when a WM_DELETE_WINDOW notification is done.
    **
    **	WM_SAVE_YOURSELF - clients with this atom will be notified when a
    **	    session manager or a window manager wishes the window's state
    **	    to be changed.  The typical change is when the window is about
    **	    to be deleted or the session terminated.
    **	    
    */

    delete_window_atom = XmInternAtom (XtDisplay(private->shell),
	    "WM_DELETE_WINDOW", False);

    save_yourself_atom = XmInternAtom (XtDisplay(private->shell),
	    "WM_SAVE_YOURSELF", False);

    XmAddWMProtocolCallback (private->shell, delete_window_atom,
	(XtCallbackProc)EnvEnvWMDeleteWindow, (caddr_t)private);    

    XmAddWMProtocolCallback (private->shell, save_yourself_atom,
	(XtCallbackProc)EnvEnvWMSaveYourself, (caddr_t)private);
	
    XmActivateWMProtocol (private->shell, delete_window_atom);

    XmActivateWMProtocol (private->shell, save_yourself_atom);
    

    /*
    **  Set the Svn LHS/RHS percentage
    */

    _GetAttribute(env_context, _EnvWindowSplit, &split);

    XtSetArg(arglist[0], DXmSvnNprimaryPercentage, (int) split);
    XtSetValues(private->svn, arglist, 1);

    /*
    **  Fetch the SVN fonts
    */
    
    EnvSvnWindowLoadFont(private, hierarchy);

    /*
    ** Close the MRM hierarchy.
    */

    if (MrmCloseHierarchy(hierarchy) != MrmSUCCESS)
	_Raise(drm_close_error);

    XtRealizeWidget(private->main_widget);

    /*
    ** Create the connection menu. 
    */

    private->his_dwui = (lwk_dxm_ui) EnvHisCreateDwUi(private,
	"LinkWorks Manager");

    /*
    **  Install the accelerators on the widgets which accept input focus
    */

    XtInstallAllAccelerators(private->main_widget, private->menubar);

    /*
    ** Initialize the quick copy atoms.
    */

    EnvDWQuickCopyInitialize(private);

    /*
    ** Add the quick copy event handler and new actions.
    */

    EnvSvnEnvWindowAddActions(private);


    XtManageChild(private->main_widget);
    XtPopup(private->shell, XtGrabNone);
    

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



_Void  EnvDWEnvWindowRemoveCopyright(private, title_id)
_WindowPrivate private;
 _String title_id;

/*
**++
**  Functional Description:
**	Title bar is resumed to its real name and event handler is removed.
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
    _CString	title;
    
    /*
    ** Set the window's title
    */

    if (title_id == (_String) 0) 
	EnvDWGetCStringLiteral(_DwtEnvWindowTitle, (_String *) &title);
    else
	EnvDWGetCStringLiteral(title_id, (_String *) &title);
		
    EnvDWSetTitle(private->shell, title);
    _DeleteCString(&title);
    
    return;
    }


static _Void  EnvWindowSetIconsOnShell(shell, user_data, event, continue_to_dispatch)
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
    **  Set the icons for this shell.
    */
    EnvWindowSetIcons(shell);

    return;
}


static _Void  EnvWindowSetIcons(shell)
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
    icon_name = EnvDWGetIconIndexName(dpy, _DwtEnvWindowIcon, &icon_size, 
    	    	    	    	      shell_icon_sizes, num_sizes);
    if (icon_name != NULL) {

        /*
	** If the icon sizes are different we need to free the current ones, and
	** re-fetch new icons.  We assume that re-fetching new icons is an
	** infrequent operation, so we don't cache the old icons.
	*/
    	if ((EnvWindowIconSize != 0)	    	    	/* Icon exists.     */
    	     && (EnvWindowIconSize != icon_size))	/* New icon needed. */
    	{
    	    if (EnvWindowIconPx)
    	    	XFreePixmap(dpy, EnvWindowIconPx);
    	    if ((EnvWindowIconifyPx)
		&& (EnvWindowIconifyPx != EnvWindowIconPx))
    	    	XFreePixmap(dpy, EnvWindowIconifyPx);
    	    EnvWindowIconPx = (Pixmap) 0;
    	    EnvWindowIconifyPx = (Pixmap) 0;
    	    EnvWindowIconSize = (int) 0;
    	}
	
    	if (EnvWindowIconSize == 0)
    	{
    	    EnvWindowIconSize = icon_size;

	    EnvDWFetchBitmap(shell, icon_name, &EnvWindowIconPx);
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
    	    EnvWindowIconifyPx = EnvWindowIconPx;
    	else if (icon_size > 17) {
	    icon_size = 17;
	    icon_name = (char *) XtMalloc(strlen(_DwtEnvWindowIcon) +
    	    	    	    	    	  sizeof("_")		    +
    	    	    	    	          sizeof("17")		    +
    	    	    	    	    	  1 );    /* for \0 char */
	    strcpy(icon_name, _DwtEnvWindowIcon);
	    strcat(icon_name, "_");
	    strcat(icon_name, "17");

	    EnvDWFetchBitmap(shell, icon_name, &EnvWindowIconifyPx);

	    XtFree(icon_name);
	    }
    }

    /*
    ** Set the icon pixmap on the shell.
    */
    if (EnvWindowIconPx)
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
    	    	wmhints->icon_pixmap = EnvWindowIconPx;
    	    	XSetWMHints(dpy, XtWindow(shell), wmhints);
    	    	XFree((_AnyPtr) wmhints);
    	    } 
    	    else
    	    {
	    	wmhints = (XWMHints *)XtCalloc(1, sizeof(XWMHints));
	    	wmhints->flags &= ~StateHint;
    	    	wmhints->flags |= IconPixmapHint;
    	    	wmhints->icon_pixmap = EnvWindowIconPx;
    	    	XSetWMHints(dpy, XtWindow(shell), wmhints);
	    	XtFree((_AnyPtr) wmhints);
    	    }
    	}
    	else
    	{
    	    Arg	arglist[1];
    	    XtSetArg(arglist[0], XmNiconPixmap, EnvWindowIconPx);
    	    XtSetValues(shell, arglist, 1);
    	}
    }

    /*
    ** Set the iconify pixmap for the XUI window manager
    */
    if (EnvWindowIconifyPx)
    	EnvDWWmSetIconifyPixmap(shell, EnvWindowIconifyPx);

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


_Void  EnvOpDWEnvWindowDelete(private)
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

    if (private == (_WindowPrivate) _NullObject)
	return;

    /*
    ** Free the private data structure.
    */

    EnvWindowDeletePrivateData(private);

    return;
    }


_Void  EnvOpDWEnvWindowDisplay(private, envcontext)
_WindowPrivate private;
 _EnvContext envcontext;

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
    _Status	status[2];
    _Boolean	message;

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock
     
    _SetCursor(private->window, _WaitCursor);

    /*
    ** Load in memory the svn display list for the environment context.
    */

    EnvSvnEnvWindowLoad(private, &(private->svn_entries), envcontext);

    /*
    ** Tell SVN about the environment context.
    */

    EnvSvnEnvWindowDisplayEnv(private);

    /*
    **  Display a message if a new default linkbase has been created
    */

    message = _SetWindowState(private->window, _StateMessageSet, _StateGet);

    if (message) {

	status[0] = hs_s_lb_not_exist;
	status[1] = hs_s_new_lb;
	_DisplayMessage(private->window, status, 2);

	_SetWindowState(private->window, _StateMessageSet, _StateClear);
    }

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


_Void  EnvOpDWEnvWindowClose(private)
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
    
    return;
    }

          
_Void  EnvDWEnvWinUpdateCurrencyCNet(private)
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
    _EnvContext		env_ctxt;
    _EnvWindowPrivate	envprivate;
    lwk_set		set;
    lwk_status		status;

    envprivate = (_EnvWindowPrivate) private->specific;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_ctxt);
	
    /*
    **  Collect the network list if not empty
    */

    if (envprivate->networks_head != (_SvnData) 0) 
	GetActiveNets(envprivate->networks_head, &set);
    else
	set = lwk_c_null_object;

    /*
    **  Set the currecy
    */
	    
    _SetContextCurrency(env_ctxt, lwk_c_env_active_comp_linknet, set,
	(_HsObject) _NullObject);
    
    if (set != lwk_c_null_object)
	status = lwk_delete(&set);

    return;
    }

          
_Void  EnvDWEnvWinUpdateCurrencyCPath(private)
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
    _EnvContext		env_ctxt;
    _EnvWindowPrivate	envprivate;
    lwk_list		list;
    lwk_status		status;

    envprivate = (_EnvWindowPrivate) private->specific;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_ctxt);
	
    /*
    **  Collect the path list if not empty
    */

    if (envprivate->paths_head != (_SvnData) 0) 
	GetActivePaths(envprivate->paths_head, &list);
    else
	list = lwk_c_null_object;
	
    /*
    **  Set the currecy
    */
	    
    _SetContextCurrency(env_ctxt, lwk_c_env_active_comp_path, list,
	(_HsObject) _NullObject);
    
    if (list != lwk_c_null_object)
	status = lwk_delete(&list);

    return;
    }


static _WindowPrivate  EnvWindowCreatePrivateData(envwindow, parent)
_EnvWindow envwindow;

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
    _EnvWindowPrivate	envwindowprivate;

    _StartExceptionBlock

    /*
    ** Initialize.
    */

    windowprivate = (_WindowPrivate) 0;
    envwindowprivate = (_EnvWindowPrivate) 0;

    /*
    ** Create window object private data.
    */

    windowprivate = (_WindowPrivate)
	EnvDWWindowCreatePrivateData((_Window) envwindow, parent);

    /*
    ** Allocate memory for private data.
    */

    envwindowprivate = (_EnvWindowPrivate)
	_AllocateMem(sizeof(_EnvWindowPrivateInstance));

    /*
    **	Initialize private data
    */

    _ClearMem(envwindowprivate, sizeof(_EnvWindowPrivateInstance));

    windowprivate->specific = (_AnyPtr) envwindowprivate;

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers
	
	    if (envwindowprivate != _NullObject)
		_FreeMem(envwindowprivate);
	    _Reraise;
	
    _EndExceptionBlock

    return windowprivate;
    }


static _Void  EnvWindowDeletePrivateData(windowprivate)
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
    if (windowprivate == (_WindowPrivate) _NullObject)
	return;

    /*
    ** Free the envwindow private structure.
    */

    _FreeMem(windowprivate->specific);

    /*
    ** Free the window private structure.
    */

    EnvDWWindowDeletePrivateData(windowprivate);

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
    **  Set Svn attributes for the environment window
    */

    EnvSvnEnvWindowInitialize(private);

    return;
    }


static _Void  UilCreateLinkMenu(w, private, reason)
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
    _Boolean new = _True;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    /*
    **	Popup the linkbase selection box
    */

    EnvDWLbBox(private, new);

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


static _Void  UilActivateExit(w, private, reason)
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
    _Boolean	    changes_not_saved;

    _StartExceptionBlock

    /*
    ** Have customization changes been made and saved?
    */

    changes_not_saved = _SetWindowState(private->window, _StateCustomNotSaved,
	_StateGet);

    if (changes_not_saved)
	EnvDWQuestion(private, UilActivateSaveCustom_Yes,
	    UilActivateSaveCustom_No, _SaveCustomChangesMessage);
    else
    
	/*
	** Delete whatever is left and exit
	*/
	
	FinalCleanup(private);

    /*
    ** If an exception is generated, the message handler will let the user quit
    ** the application
    */
    

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(quit_failed);
	    status[1] = _Others;

	    _DisplayRootMessage(status, 2, hs_c_fatal_message);
	
    _EndExceptionBlock
    }


static _Void  FinalCleanup(private)
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
    lwk_status	    status;
    
    /*
    **	Global cleanup should be done here but isn't really needed since the
    **  application is running down. However, we must delete the main HIS DW UI
    **	object because it is declared as the environment manager and also
    **	because we have all LinkWorks Manager's surrogates types posted on the root
    **	window which should be deleted by the Services.
    */

    status = lwk_delete(&private->his_dwui);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(envmgr_deletion_failed);
    }

    exit(TerminationSuccess);
    }


static _Void  UilActivateSaveCustom_Yes(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    _Boolean	exc_raised;

    XtUnmanageChild(private->question_box);

    /*
    **  Restore the default cursor 
    */
    _SetCursor(private->window, _DefaultCursor);

    exc_raised = _False;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    SaveAttributes(private);

    _Exceptions
	_WhenOthers
	    _Status status[2];

	    exc_raised = _True;
	
	    status[0] = _StatusCode(quit_failed);
	    status[1] = _Others;

	    _DisplayRootMessage(status, 2, hs_c_fatal_message);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    if (!exc_raised)
	FinalCleanup(private);

    return;
    }

static _Void  UilActivateSaveCustom_No(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {

    XtUnmanageChild(private->question_box);

    /*
    ** Restore the default cursor
    */
    _SetCursor(private->window, _DefaultCursor);

    FinalCleanup(private);

    return;
    }


_Void  EnvOpDWEnvWindowSetCurrency(private, currency, value)
_WindowPrivate private;
 _CurrencyFlag currency;

    _AnyPtr value;

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
    lwk_status	    status;
    lwk_object	    null_object;

    /*
    **  Set the value to be the current of that type
    */

    switch (currency) {

	case lwk_c_env_appl_highlight :
	
	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_appl_highlight, lwk_c_domain_integer, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_active_comp_linknet :
	
	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_active_comp_linknet, 	lwk_c_domain_comp_linknet,
		value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_recording_linknet :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_recording_linknet,
		lwk_c_domain_linknet, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_active_comp_path :
	
	    status = lwk_set_value(private->his_dwui,
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

	    _SetCurrency(private->window, lwk_c_env_active_path, (_AnyPtr) &null_object);

	    _SetCurrency(private->window, lwk_c_env_active_path_index,
		(_AnyPtr) &null_object);

	    _SetCurrency(private->window, lwk_c_env_followed_step, (_AnyPtr) &null_object);
	    
	    break;

	case lwk_c_env_active_path :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_active_path,
		lwk_c_domain_path, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_active_path_index :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_active_path_index,
		lwk_c_domain_integer, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_recording_path :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_recording_path,
		lwk_c_domain_path, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_followed_step :

	    status = lwk_set_value(private->his_dwui, lwk_c_p_followed_step,
		lwk_c_domain_step, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_new_link :
	
	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_new_link, lwk_c_domain_link, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_pending_source :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_pending_source,
		lwk_c_domain_surrogate, value, 	lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_pending_target :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_pending_target,
		lwk_c_domain_surrogate, value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_followed_link :

	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_followed_link, lwk_c_domain_link, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_follow_destination :
	
	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_follow_destination, lwk_c_domain_surrogate, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_default_operation :
	
	    status = lwk_set_value(private->his_dwui, lwk_c_p_default_operation,
		lwk_c_domain_string, value, lwk_c_set_property );
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_default_highlight :
	
	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_default_highlight, lwk_c_domain_integer, value,
		lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }
	    break;

	case lwk_c_env_default_relationship :
	
	    status = lwk_set_value(private->his_dwui,
		lwk_c_p_default_relationship, lwk_c_domain_ddif_string,
		value, lwk_c_set_property);
		
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(cur_set_failure);
	    }

	    break;

	case lwk_c_env_default_retain_source :
	
	    status = lwk_set_value(private->his_dwui,
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


static _Void  UilMapControlMenu(w, private, reason)
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
    Arg		arglist[5];
    _Integer	count = 0;
    _Integer	record_state;
    _Integer	activate_state;
    _EnvWindowPrivate	env_private;
    _SelectData	select_data;

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    env_private = (_EnvWindowPrivate) private->specific;

    /*
    ** For checking the sensitivity of the record button the user must have
    ** selected only one svn entry.
    */
    
    _GetSelection(private->window, _SingleSelection, &select_data);

    /*
    **  See what the state of the record button is for the given selection
    */

    record_state = CheckRecordMenuState(env_private, select_data);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    switch (record_state) {

	case _MenuOn :
	    if (env_private->record_on == (_CString) 0)
		EnvDWGetCStringLiteral(_EnvRecordOnMenuLabel,
		    &(env_private->record_on));
	    if (env_private->record_on_keysym == (KeySym) 0)
		EnvDWGetKeySym(_EnvRecordOnKeySym,
		    &(env_private->record_on_keysym));
	    XtSetArg(arglist[count], XmNmnemonic, env_private->record_on_keysym);
	    count++;
	    XtSetArg(arglist[count], XmNlabelString, env_private->record_on);
	    count++;
	    XtSetArg(arglist[count], XmNsensitive, _True); count++;
	    break;
	
	case _MenuOff :
	    if (env_private->record_off == (_CString) 0)
		EnvDWGetCStringLiteral(_EnvRecordOffMenuLabel,
		    &(env_private->record_off));
	    if (env_private->record_off_keysym == (KeySym) 0)
		EnvDWGetKeySym(_EnvRecordOffKeySym,
		    &(env_private->record_off_keysym));
	    XtSetArg(arglist[count], XmNmnemonic, env_private->record_off_keysym);
	    count++;
	    XtSetArg(arglist[count], XmNlabelString, env_private->record_off);
	    count++;
	    XtSetArg(arglist[count], XmNsensitive, _True); count++;
	    break;
	
	case _MenuDim :
	    if (env_private->record_on == (_CString) 0)
		EnvDWGetCStringLiteral(_EnvRecordOnMenuLabel,
		    &(env_private->record_on));
	    if (env_private->record_on_keysym == (KeySym) 0)
		EnvDWGetKeySym(_EnvRecordOnKeySym,
		    &(env_private->record_on_keysym));
	    XtSetArg(arglist[count], XmNmnemonic, env_private->record_on_keysym);
	    count++;
	    XtSetArg(arglist[count], XmNlabelString, env_private->record_on);
	    count++;
	    XtSetArg(arglist[count], XmNsensitive, _False); count++;
	    break;
    }

    XtSetValues(env_private->record_button, arglist, count);

    /*
    **  See what the state of the activate button should be according to the
    **	selection. If the user has selected more than one Svn entry, the first
    **	one determines the label of the activate button.
    */
    
    _GetSelection(private->window, _FirstSelection, &select_data);

    activate_state = CheckActivateMenuState(private->specific, select_data);
    count = 0;

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    switch (activate_state) {

	case _MenuOn :
	    if (env_private->activate == (_CString) 0)
		EnvDWGetCStringLiteral(_EnvActivateMenuLabel,
		    &(env_private->activate));
	    if (env_private->activate_keysym == (KeySym) 0)
		EnvDWGetKeySym(_EnvActivateKeySym,
		    &(env_private->activate_keysym));
	    XtSetArg(arglist[count], XmNmnemonic, env_private->activate_keysym);
	    count++;
	    XtSetArg(arglist[count], XmNlabelString, env_private->activate);
	    count++;
	    XtSetArg(arglist[count], XmNsensitive, _True); count++;
	    break;

	case _MenuOff :
	    if (env_private->deactivate == (_CString) 0)
		EnvDWGetCStringLiteral(_EnvDeactivateMenuLabel,
		    &(env_private->deactivate));
	    if (env_private->deactivate_keysym == (KeySym) 0)
		EnvDWGetKeySym(_EnvDeactivateKeySym,
		    &(env_private->deactivate_keysym));
	    XtSetArg(arglist[count], XmNmnemonic, env_private->deactivate_keysym);
	    count++;
	    XtSetArg(arglist[count], XmNlabelString, env_private->deactivate);
	    count++;
	    XtSetArg(arglist[count], XmNsensitive, _True); count++;
	    break;

	case _MenuDim :
	    if (env_private->activate == (_CString) 0)
		EnvDWGetCStringLiteral(_EnvActivateMenuLabel,
		    &(env_private->activate));
	    if (env_private->activate_keysym == (KeySym) 0)
		EnvDWGetKeySym(_EnvActivateKeySym,
		    &(env_private->activate_keysym));
	    XtSetArg(arglist[count], XmNmnemonic, env_private->activate_keysym);
	    count++;
	    XtSetArg(arglist[count], XmNlabelString, env_private->activate);
	    count++;
	    XtSetArg(arglist[count], XmNsensitive, _False); count++;
	    break;
    }

    XtSetValues(env_private->activate_button, arglist, count);

    /*
    ** Set the state for the update button. Can be set on a multiple selection.
    */
    if (_GetSelection(private->window, _MultipleSelection, &select_data)) 
	SetUpdateButton(private, select_data);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(map_menu_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Integer  CheckRecordMenuState(env_private, select_data)
_EnvWindowPrivate env_private;

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
    _SvnData	    tag;
    _Integer        type;
    lwk_persistent  his_obj;
    lwk_domain      domain;

    /*
    **  Nothing is selected
    */

    if (select_data == (_SelectData) 0)
	 return(_MenuDim);
	
    /*
    **  Check if the right component is selected
    */

    if ((select_data->component[0] != _EnvSvnIconComponent) &&
	(select_data->component[0] != _EnvSvnNameComponent) &&
	(select_data->component[0] != _EnvSvnRecordComponent))
	return(_MenuDim);

    /*
    ** Check if the object selected is retrievable.
    */
    tag = select_data->svn_data[0];

    if (tag->retrievable != _Retrievable) 
	return(_MenuDim);
	
    /*
    **  Check if the object selected is legal
    */


    if (tag->object == ((_HsObject) _NullObject))
	return(_MenuDim);
	
    _GetValue(tag->object, _P_HisType, hs_c_domain_integer, &type);

    domain = (lwk_domain) type;

    if ((domain == lwk_c_domain_comp_linknet) ||
	(domain == lwk_c_domain_comp_path))
	return(_MenuDim);

    /*
    **  Check if it is the current network or path
    */

    _GetValue(tag->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    if ((his_obj == env_private->current_network) ||
	(his_obj == env_private->trail))
	return(_MenuOff);

    /*
    **  A path cannot be set current if already active
    */

    if ((domain == lwk_c_domain_path) && (tag->active == _Active))
	return(_MenuDim);
	
    return(_MenuOn);
    }


static _Integer  CheckActivateMenuState(private, select_data)
_EnvWindowPrivate private;

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
    _Integer    type;
    _Integer	active_state;
    _SvnData	tag;

    /*
    **  Nothing is selected
    */

    if (select_data == (_SelectData) 0)
	 return(_MenuDim);

    /*
    ** Check if the object is retrievable.
    */
    tag = select_data->svn_data[0];

    if (tag->retrievable != _Retrievable)
	return(_MenuDim);
	
    /*
    **  Check if there is an object (we don't want titles or segment headers)
    */


    if (tag->object == ((_HsObject) _NullObject))
	return(_MenuDim);
	
    /*
    **  Check if the right component is selected
    */

    if ((select_data->component[0] != _EnvSvnIconComponent) &&
	(select_data->component[0] != _EnvSvnNameComponent) &&
	(select_data->component[0] != _EnvSvnActivateComponent))
	return(_MenuDim);
	
    /*
    **  Check the state of the object selected
    */

    if ((tag->active == _Active) || (tag->active == _PartActive))
	return(_MenuOff);

    else {

	/*
	**  A path cannot be set active if it is current
	*/

	_GetValue(tag->object, _P_HisType, hs_c_domain_integer, &type);

	if ((lwk_domain) type == lwk_c_domain_path) {

	    tag->active = _Active;
	
	    if (!EnvDWEnvWindowCheckPathState(private, tag)) {
		tag->active = _NotActive;
		return(_MenuDim);
	    }
	
	    tag->active = _NotActive;
	}
    }

    return(_MenuOn);
    }


_Boolean  EnvDWEnvWindowCheckPathState(private, entry)
_EnvWindowPrivate private;

    _SvnData entry;

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
    lwk_object	    active_obj;
    lwk_object	    current_obj;

    /*
    **  If the entry is not active, no problem
    */

    if (entry->active == _NotActive)
	return(_True);

    /*
    **  Get the his object for the active object
    */

    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &active_obj);

    /*
    **  Get the his object for the current object
    */

    current_obj = private->trail;

    /*
    **  Check if different
    */

    if (current_obj != active_obj)
	return(_True);
	
    /*
    **  Same object, therefore cannot be active and record at the same time
    */

    return _False;
    }


static _Void  UilActivateActivate(w, private, reason)
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
    _EnvContext	    env_ctxt;
    _SelectData     select_data;
    _Integer	    valid_selection;
    lwk_set	    set;
    lwk_list	    list;
    Time	    timestamp;

    DXmSvnDisableDisplay(private->svn);

    /*
    ** Wait cursor on
    */

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    /*
    **  Get the selected items in Svn
    */

    _GetSelection(private->window, _MultipleSelection, &select_data);

    /*
    **  Update the components in the Svn display
    */

    EnvSvnEnvWindowToggleActivate(private, select_data);

    /*
    ** Relinquish selection ownership.
    */
    EnvDWGetTime(reason, &timestamp);
    EnvDWQuickCopyDisown(private, timestamp);

    /*
    **  Build a list of active objects and update currency depending on what has
    **	been activated.
    */

    if (_SetWindowState(private->window, _StateActiveNetworks, _StateGet)) {
	EnvDWEnvWinUpdateCurrencyCNet(private);
	_SetWindowState(private->window, _StateActiveNetworks, _StateClear);
    }

    if (_SetWindowState(private->window, _StateActivePaths, _StateGet)) {
	EnvDWEnvWinUpdateCurrencyCPath(private);
	_SetWindowState(private->window, _StateActivePaths, _StateClear);
    }

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(activate_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateRecord(w, private, reason)
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
    Time	    timestamp;

    DXmSvnDisableDisplay(private->svn);

    /*
    ** Wait cursor on
    */

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    /*
    **  Get the selected items in Svn
    */

    _GetSelection(private->window, _SingleSelection, &select_data);

    /*
    **  Update the Svn display and the currency
    */

    EnvSvnEnvWindowToggleRecording(private, select_data);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(record_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    _SetCursor(private->window, _DefaultCursor);

    DXmSvnEnableDisplay(private->svn);

    return;
    }


static _Void  UilCreateActivateBtn(w, private, reason)
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

    ((_EnvWindowPrivate) (private->specific))->activate_button = w;

    return;
    }


static _Void  UilCreateRecordBtn(w, private, reason)
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

    ((_EnvWindowPrivate) (private->specific))->record_button = w;

    return;
    }


static _Void  GetActivePaths(entry, list)
_SvnData entry;
 lwk_list *list;

/*
**++
**  Functional Description:
**	Run down the path list Svn entries and collect the active ones.
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
    lwk_status		    status;
    _IndexPath		    index_path;

    /*
    **  Create a list
    */

    status = lwk_create_list(lwk_c_domain_object_desc, 15, list);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(create_list_failed); /* his create list failed */
    }

    index_path = (_IndexPath) 0;

    /*
    **  run down the list of entries
    */
    CollectActivePathEntry(&index_path, entry, *list);

    return;
    }


static _Void  CollectActivePathEntry(index_path, entry, list)
_IndexPath *index_path;
 _SvnData entry;

    lwk_list list;

/*
**++
**  Functional Description:
**	Treat an entry and all its siblings
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

    if ((entry->active == _Active) || (entry->active == _PartActive))

        /*
	** Insert this object with its hierarchy in the active index list
	*/
	InsertHierarchyObjDesc(*index_path, entry, list);

    else
        /*
	** If it is a non active composite object we have to check all the
	** children to collect the active ones
	*/
	if (entry->children != (_SvnData) 0) {
	    AddEntryInIndexPath(index_path, entry);
	    CollectActivePathEntry(index_path, entry->children, list);
	    RemoveEntryFromIndexPath(index_path);
	}

    /*
    ** Process the siblings
    */
    if (entry->siblings != (_SvnData) 0)
	CollectActivePathEntry(index_path, entry->siblings, list);

    return;
    }


	

static _Void  InsertHierarchyObjDesc(index_path, entry, list)
_IndexPath index_path;
 _SvnData entry;

    lwk_list list;

/*
**++
**  Functional Description:
**	Insert the entry and its hierarchy of object descriptor in the index
**	path list.
**	Add the active paths index descriptor to the list to separate each
**	sequence.
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
    _WindowPrivate	    private;
    _EnvContext		    env_ctxt;
    _IndexPath		    tmp_index;
    _SvnData		    tmp_entry;
    _HsObject		    hs_obj;
    lwk_object		    his_obj;
    lwk_object_descriptor   obj_desc;
    lwk_status		    status;

    tmp_index = index_path;

    /*
    ** Run down the list of object descriptor hierarchy of this entry 
    */
    while (tmp_index != (_IndexPath) 0) {

	tmp_entry = tmp_index->entry;
	
	/*
	** Get the object descriptor and add it in the list
	*/
	_GetValue(tmp_entry->object, _P_HisObject, hs_c_domain_lwk_object,
	    &his_obj);

	status =  lwk_get_object_descriptor(his_obj, &obj_desc);

	if (status != lwk_s_success) {
	     _SaveHisStatus(status);
	      _Raise(get_objdsc_failed); /* get obj descriptor failed   */
	}
    
	status = lwk_add_element(list, lwk_c_domain_object_desc, &obj_desc);
		    
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(add_ele_failed); /* add element to list failed */
	}

	tmp_index = tmp_index->next;
    }

    /*
    ** Add the entry object descriptor to the list
    */
    _GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object, &his_obj);

    status =  lwk_get_object_descriptor(his_obj, &obj_desc);

    if (status != lwk_s_success) {
	 _SaveHisStatus(status);
	  _Raise(get_objdsc_failed); /* get obj descriptor failed   */
    }

    status = lwk_add_element(list, lwk_c_domain_object_desc, &obj_desc);
		
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(add_ele_failed); /* add element to list failed */
    }
    
    /*
    ** Separe each sequence of object descriptor by adding the active paths
    ** index object descriptor 
    */

    private = (_WindowPrivate) entry->private;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_ctxt);

    _GetValue(env_ctxt, _P_ActivePathIndex, hs_c_domain_hsobject, &hs_obj);

    _GetValue(hs_obj, _P_HisObject, hs_c_domain_lwk_object, &his_obj);
    
    status =  lwk_get_object_descriptor(his_obj, &obj_desc);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_objdsc_failed); /* get obj descriptor failed   */
    }
    
    status = lwk_add_element(list, lwk_c_domain_object_desc, &obj_desc);
		
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(add_ele_failed); /* add element to list failed */
    }

    return;
    }	


static _Void  AddEntryInIndexPath(index_path, entry)
_IndexPath *index_path;
 _SvnData entry;

/*
**++
**  Functional Description:
**	Add the entry in the index path
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
    _IndexPath	new_index_path,
		tmp_index;

    /*
    ** Allocate memory for index path structure to keep track of the hierarchy
    ** of a potential active object.
    */
    new_index_path = (_IndexPath) _AllocateMem(sizeof(_IndexPathInstance));

    new_index_path->entry = entry;
    new_index_path->next = (_IndexPath) 0;

    /*
    ** Add this index at the end of the index path
    */
    if (*index_path == (_IndexPath) 0)
	*index_path = new_index_path;

    else {

	tmp_index = *index_path;

	while (tmp_index->next != (_IndexPath) 0)
	    tmp_index = tmp_index->next;

	tmp_index->next = new_index_path;
    }

    return;
    }
    

static _Void  RemoveEntryFromIndexPath(index_path)
_IndexPath *index_path;

/*
**++
**  Functional Description:
**	Remove the last element of the list and free the allocated memory.
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
    _IndexPath	tmp_index;
    
    tmp_index = *index_path;

    if (tmp_index->next == (_IndexPath) 0) {
	_FreeMem(tmp_index);
	*index_path = (_IndexPath) 0;
    }

    else {
	while(((_IndexPath) (tmp_index->next))->next != (_IndexPath) 0)
	    tmp_index = tmp_index->next;

	_FreeMem(tmp_index->next);
	tmp_index->next = (_IndexPath) 0;
    }

    return;
    }


static _Void  GetActiveNets(entry, set)
_SvnData entry;
 lwk_set *set;

/*
**++
**  Functional Description:
**	Build a set containing the active linknets. Don't put composite linknet
**	in it.
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
    _LinknetList	    linknet_list;
    lwk_status		    status;

    /*
    **  Create a set
    */

    status = lwk_create_set(lwk_c_domain_object_desc, 15, set);

    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(create_list_failed); /* his create set failed */
    }

    /*
    ** We will keep track of the linknets treated so that we have only one
    ** occurance of each of them.
    */
    linknet_list = (_LinknetList) 0;

    /*
    **  run down the list of entries
    */

    if (entry != (_SvnData) 0) 
	CollectActiveEntry(entry, &linknet_list, *set);

    /*
    ** Free memory
    */
    FreeLinknetList(linknet_list);
        
    return;
    }	


static _Void  CollectActiveEntry(entry, linknet_list, set)
_SvnData entry;
 _LinknetList *linknet_list;

    lwk_set set;

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
    _LinknetList	    new_linknet_list,
			    tmp_linknet_list;
    lwk_integer		    type;
    lwk_object_descriptor   obj_dsc;
    lwk_status		    status;
    lwk_object		    his_obj;

    /*
    ** Only add active linknets in the set.
    */
    _GetValue(entry->object, _P_HisType, hs_c_domain_integer, &type);

    if ((entry->active == _Active) && ((lwk_domain) type == lwk_c_domain_linknet)) {

	_GetValue(entry->object, _P_HisObject, hs_c_domain_lwk_object,
	    &his_obj);

	/*
	** Check for redundant linknets
	*/
	tmp_linknet_list = *linknet_list;
	
	while (tmp_linknet_list != (_LinknetList) 0) {
	    if (tmp_linknet_list->linknet == his_obj)
		break;

	    tmp_linknet_list = tmp_linknet_list->next;
	}

	/*
	** If we haven't seen it yet, add it to the list and add its object
	** descriptor to the set
	*/
	if (tmp_linknet_list == (_LinknetList) 0) {

	    new_linknet_list = (_LinknetList)
		    _AllocateMem(sizeof(_LinknetListInstance));

	    new_linknet_list->linknet = his_obj;
	    new_linknet_list->next = *linknet_list;

	    *linknet_list = new_linknet_list;

	    status =  lwk_get_object_descriptor(his_obj, &obj_dsc);

	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(get_objdsc_failed); /* get obj descriptor failed */
	    }
		
	    status = lwk_add_element(set, lwk_c_domain_object_desc, &obj_dsc);
			
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(add_ele_failed); /* add element to set failed */
	    }
	}
    }
    else {

	/*
	**  If it is a composite object we have to check all the
	**	children to collect the active ones
	*/
    
	if (entry->children != (_SvnData) 0)
	    CollectActiveEntry(entry->children, linknet_list, set);
    }

    /*
    **  Process the siblings
    */

    if (entry->siblings != (_SvnData) 0)
	CollectActiveEntry(entry->siblings, linknet_list, set);
	
    return;
    }


static _Void  FreeLinknetList(linknet_list)
_LinknetList linknet_list;

/*
**++
**  Functional Description:
**	Free the LinknetList and all its successors.
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
    
    if (linknet_list != (_LinknetList) 0) {

        /*
	** clean all the successors first.
	*/
	if (linknet_list->next != (_LinknetList) 0)
	    FreeLinknetList(linknet_list->next);

	_FreeMem(linknet_list);
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
    _Integer	count,
		index;
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
    ** selected, set the cut and copy buttons to sensitive.
    */

    if (_GetSelection(private->window, _SingleSelection, &select_data)) {

	if (select_data->svn_data[0]->object != _NullObject) {
	
	    if (select_data->component[0] == _EnvSvnIconComponent ||
		select_data->component[0] == _EnvSvnNameComponent) {
		
		if (select_data->svn_data[0]->retrievable == _Retrievable) {
		
		    if (select_data->svn_data[0]->child_level == 3)
			EnvDWSetEditButtons(private, _True);
		    else {
		
			XtSetArg(arglist[0], XmNsensitive, _True);
			XtSetValues(private->copy_button, arglist, 1);
		    }
		}
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
    ** If there is a multiple selection, check if all the selected entries are
    ** at the first level to put the delete button to sensitive.
    */
    if (_GetSelection(private->window, _MultipleSelection, &select_data)) {

	count = select_data->count;
	index = 0;

	XtSetArg(arglist[0], XmNsensitive, _True);

	while ((count > 0) && (select_data->svn_data[index]->child_level == 3)){
	    count--;
	    index++;
	}

	if (count != 0)
	    XtSetArg(arglist[0], XmNsensitive, _False);
		
	XtSetValues(private->delete_button, arglist, 1);

	if (select_data != (_SelectData) 0)
	    EnvSvnWindowFreeSelection(select_data);
    }
    
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
	
	    status[0] = _StatusCode(copy_failed);
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

    EnvWindowInsertEntry(private, object);

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

    while (count > 0) {
	count --;
	EnvSvnEnvWindowRemoveEntry(private, select_data->svn_data[index],
	    timestamp);
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
    _SelectData		select_data;

    DXmSvnDisableDisplay(private->svn);

    _StartExceptionBlock

    _GetSelection(private->window, _SingleSelection, &select_data);

    /*
    **  Set the state for the Show Properties button
    */

    SetShowPropButtonLabel(private, select_data);

    /*
    **  Set the state for the Expand button
    */

    EnvDWSetExpandButtonLabel(private, select_data);

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(map_menu_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

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

    XtSetArg(arglist[0], XmNsensitive, FALSE);

    /*
    **  Nothing is selected
    */

    if (select_data != (_SelectData) 0) {

	/*
	**  Check if the right component is selected
	*/

	tag = select_data->svn_data[0];
		
	if ((select_data->component[0] == _EnvSvnIconComponent) ||
	    (select_data->component[0] == _EnvSvnNameComponent)) {
	
	    /*
	    **  Check if the object selected is legal
	    */
	
	    if (tag->object != ((_HsObject) _NullObject) &&
		(tag->retrievable == _Retrievable))
		XtSetArg(arglist[0], XmNsensitive, TRUE);
	}

	if ((select_data->component[0] == _EnvSvnLinkbaseComponent) &&
	    (tag->retrievable != _LbNotRetrievable) &&
	    (tag->object != (_HsObject) _NullObject))
	    XtSetArg(arglist[0], XmNsensitive, TRUE);
    }

    XtSetValues(private->showprop_button, arglist, 1);

    return;
    }


static _Void  SetUpdateButton(private, select_data)
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
    _EnvWindowPrivate	envprivate;
    _SvnData		tag;
    _Integer		count,
			index;
    _Boolean		sensitive;
    Arg			arglist[1];

    envprivate = (_EnvWindowPrivate) private->specific;

    count = select_data->count;
    index = 0;
    sensitive = _False;

    if (count >0)
	sensitive = _True;

    while ((count > 0) && (sensitive)) {

	count--;
	
	/*
	**  Check if the right component is selected
	*/

	if ((select_data->component[index] == _EnvSvnIconComponent) ||
	    (select_data->component[index] == _EnvSvnNameComponent)) {
	
	    if (select_data->svn_data[index]->object
		== ((_HsObject) _NullObject))
		sensitive = _False;
	}

	else
	    sensitive = _False;

	index++;
    }

    if (sensitive)
        XtSetArg(arglist[0], XmNsensitive, TRUE);
    else
        XtSetArg(arglist[0], XmNsensitive, FALSE);
	
    XtSetValues(envprivate->update_button, arglist, 1);

    return;
    }

static _Void  UilCreateShowPropBtn(w, private, reason)
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
    _HsObject	    hs_object;
    _CString	    cs_name;
    _String	    identifier;
    _SelectData	    select_data;
    _SvnData	    entry;
    lwk_status	    status;
    lwk_linkbase  linkbase;

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    _GetSelection(private->window, _SingleSelection, &select_data);

    entry = select_data->svn_data[0];

    if (select_data->component[0] == _EnvSvnLinkbaseComponent) {

	/*
	**  Get the linkbase
	*/

	_GetLinkbase(entry->object, &cs_name, &identifier);

	status = lwk_open(identifier, _False, &linkbase);

	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(open_lb_err);
	}

	hs_object = (_HsObject) _CreateHsObject(_TypeHsObject,
	    lwk_c_domain_linkbase, linkbase);
    }
    else
	hs_object = entry->object;

    _DisplayProperties(hs_object, private, _False);

    if (select_data->component[0] == _EnvSvnLinkbaseComponent) {

	/*
	**  Cleanup the linkbase stuff
	*/
	
	status = lwk_close(linkbase);
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(close_lb_err);
	}
    }

    /*
    **  Exception handling
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(show_prop_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    EnvSvnWindowFreeSelection(select_data);

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilCreateExpandBtn(w, private, reason)
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

    private->expand_button = w;

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
	
	expand = EnvSvnWindowIsExpandable(private,
		    select_data->svn_data[0], select_data->entry[0],
		    select_data->component[0]);
		
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


static _Void  UilCreateUpdateBtn(w, private, reason)
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
    _EnvWindowPrivate envprivate;

    envprivate = (_EnvWindowPrivate) private->specific;

    envprivate->update_button = w;

    return;
    }


static _Void  UilActivateUpdate(w, private, reason)
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
    _Integer	    count,
		    index;

    DXmSvnDisableDisplay(private->svn);

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    /*
    ** Get the current selection.
    */

    if (_GetSelection(private->window, _MultipleSelection, &select_data)) {

	count = select_data->count;
	index = 0;

	while (count > 0) {
	    count--;

	    /*
	    ** Update the entry if possible.
	    */
	    
	    EnvSvnEnvWindowUpdateEntry(private, select_data->svn_data[index]);

	    index++;
	}
    }

    if (select_data != (_SelectData) 0)
	EnvSvnWindowFreeSelection(select_data);
		
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    if (select_data != (_SelectData) 0)
		EnvSvnWindowFreeSelection(select_data);
		
	    status[0] = _StatusCode(update_err);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	
    _EndExceptionBlock

    DXmSvnEnableDisplay(private->svn);

    _SetCursor(private->window, _DefaultCursor);

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
    _SelectData	    select_data;
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
	EnvSvnEnvWindowRemoveEntry(private, select_data->svn_data[0],
	    timestamp);
	
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

static _Void  EnvWindowInsertEntry(private, object)
_WindowPrivate private;
 lwk_object object;

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
    lwk_domain  domain;
    lwk_object  persistent_object;
    lwk_integer tmp_int;

    /*
    ** Get the object domain.
    */
    status = lwk_get_domain(object, &domain);
    if (status != lwk_s_success) {
	_SaveHisStatus(status);
	_Raise(get_domain_failed); /*get domain failed */
    }

    /*
    ** It's a persistent object.
    */
    if (domain != lwk_c_domain_object_desc)
	persistent_object = object;	
    /*
    ** Else retrieve the persistent object and its domain.
    */
    else {
        status = lwk_retrieve(object, &persistent_object);
        if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(object_retrieve_failed); /* object retrieve failed */
	}
	/*
	** Get the domain of the object to insert.
	*/
	status = lwk_get_value(object, lwk_c_p_object_domain, lwk_c_domain_integer,
			     &tmp_int);
	if (status != lwk_s_success) {
	    _SaveHisStatus(status);
	    _Raise(get_domain_failed); /* get object domain failed */
	}
	domain = (lwk_domain) tmp_int;
    }

    /*
    ** Insert the entry and its children into the display list.
    */

    EnvSvnEnvWindowInsertEntry(private, persistent_object, domain);

    return;
    }


static _Void  UilActivateSessionAttr (w, private, reason)
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

    _StartExceptionBlock

    EnvCustHypDisplayLinkWorksMgr(private);

    /*
    **  Exception handling
    */

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


static _Void  UilActivateEnvAttr(w, private, reason)
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

    _StartExceptionBlock

    EnvCustEnvDisplayEnvironment(private);

    /*
    **  Exception handling
    */

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


static _Void  UilActivateSaveAttr (w, private, reason)
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
    _EnvContext	env_context;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    SaveAttributes(private);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(save_attr_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  SaveAttributes(private)
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
    _EnvContext	env_context;

    /*
    **  Save environment window geometry - The attributes will be saved in the
    **	in-memory copy of the Attribute Cnet object. The actual store will
    **	happen with the _SaveAttributes call.
    */

    SetWindowGeometryAttr(private);

    /*
    **  Save the environment attributes
    */

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    _SaveAttributes(env_context);

    _SetWindowState(private->window, _StateCustomNotSaved, _StateClear);

    return;
    }


static _Void  UilActivateRestoreAttr (w, private, reason)
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
    _EnvContext	env_context;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    _ResetAttributes(env_context);

    ResetWindowGeometryAttr(private);

    _SetWindowState(private->window, _StateCustomNotSaved, _StateClear);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(restore_attr_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateRestoreSysAttr (w, private, reason)
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
    _EnvContext	env_context;

    _StartExceptionBlock

    _SetCursor(private->window, _WaitCursor);

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    _ResetDefaultAttribute(env_context);

    ResetWindowGeometryAttr(private);

    _SetWindowState(private->window, _StateCustomNotSaved, _StateClear);

    _Exceptions
	_WhenOthers
	    _Status status[2];
	
	    status[0] = _StatusCode(restore_sys_attr_failed);
	    status[1] = _Others;

	    _DisplayMessage(private->window, status, 2);
	    	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  SetWindowGeometryAttr(private)
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
    Arg		    arglist[5];
    int		    count = 0;
    Position	    x_pos;
    Position	    y_pos;
    lwk_integer	    his_x_pos = (lwk_integer) 0;
    lwk_integer	    his_y_pos = (lwk_integer) 0;
    Dimension 	    width;
    Dimension	    height;
    lwk_integer	    his_width = (lwk_integer) 0;
    lwk_integer	    his_height = (lwk_integer) 0;
    lwk_integer	    split;
    _EnvContext	    env_context;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    /*
    **  Get the window geometry attributes
    */

    XtSetArg(arglist[count], XmNx, &x_pos); count++;
    XtSetArg(arglist[count], XmNy, &y_pos); count++;
    XtSetArg(arglist[count], XmNwidth, &width); count++;
    XtSetArg(arglist[count], XmNheight, &height); count++;

    XtGetValues(private->shell, arglist, count);

    /*
    **  Set the attribute on the in-memory copy of the Attribute object. The
    **  actual store will happen with _SaveAttributes.
    */

    his_x_pos = (lwk_integer) x_pos;
    his_y_pos = (lwk_integer) y_pos;
    his_width = (lwk_integer) width;
    his_height = (lwk_integer) height;

    _SetAttribute(env_context, _EnvXPositionAttr, &his_x_pos);
    _SetAttribute(env_context, _EnvYPositionAttr, &his_y_pos);
    _SetAttribute(env_context, _EnvWidthAttr, &his_width);
    _SetAttribute(env_context, _EnvHeightAttr, &his_height);

    /*
    **  Get the svn window split percentage
    */

    XtSetArg(arglist[0], DXmSvnNprimaryPercentage, &split);
    XtGetValues(private->svn, arglist, 1);

    _SetAttribute(env_context, _EnvWindowSplit, &split);

    return;
    }


static _Void  ResetWindowGeometryAttr(private)
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
    _EnvContext	    env_context;
    Arg		    arglist[5];
    int		    count = 0;
    lwk_integer	    x_pos;
    lwk_integer	    y_pos;
    lwk_integer	    width;
    lwk_integer	    height;
    lwk_integer	    split;

    _GetValue(private->window, _P_EnvironmentContext, hs_c_domain_environment_ctxt,
	&env_context);

    /*
    **  Get the geometry attributes
    */

    _GetAttribute(env_context, _EnvXPositionAttr, &x_pos);
    _GetAttribute(env_context, _EnvYPositionAttr, &y_pos);
    _GetAttribute(env_context, _EnvWidthAttr, &width);
    _GetAttribute(env_context, _EnvHeightAttr, &height);

    XtSetArg(arglist[count], XmNx, (Position) x_pos); count++;
    XtSetArg(arglist[count], XmNy, (Position) y_pos); count++;
    XtSetArg(arglist[count], XmNwidth, (Dimension) width); count++;
    XtSetArg(arglist[count], XmNheight, (Dimension) height); count++;

    XtSetValues(private->shell, arglist, count);

    /*
    **  Set the Svn LHS/RHS percentage
    */

    _GetAttribute(env_context, _EnvWindowSplit, &split);

    XtSetArg(arglist[0], DXmSvnNprimaryPercentage, (int) split);
    XtSetValues(private->svn, arglist, 1);

    return;
    }



static _Void  EnvEnvWMSaveYourself(w, private, reason)
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
    XtErrorHandler	orig_handler;

    SaveAttributes(private);

    /*
    ** Create an application context if we don't have one for
    ** this module.
    */

    if (AppContext == (XtAppContext) 0)
	AppContext = XtCreateApplicationContext();

    orig_handler = XtAppSetWarningHandler(AppContext,
	(XtErrorHandler) EnvEnvWarningHandler);

    /*									    
    **	Let the Window Mgr or Session Mgr know that we're done
    */

    if(XGetCommand(XtDisplay(private->shell),
	XtWindow(private->shell), &argv, &argc)){

	XSetCommand(XtDisplay(private->shell),
	    XtWindow(private->shell), argv, argc);

	XFlush(XtDisplay(private->shell));

    } else {

	XSetCommand(XtDisplay(private->shell),
	    XtWindow(private->shell), argv_const, 1);
	    
	XFlush(XtDisplay(private->shell));
    }
}

static void  EnvEnvWarningHandler(message)
String message;

{
    return;
}


static _Void  EnvEnvWMDeleteWindow(w, private, reason)
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
    UilActivateExit(w, private, reason);
}    


_Void  EnvDwEnvWindowSetCursor(private, cursor_type)
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
    /* Set the cursor on the main window */
						 
    EnvDwSetCursor(private->main_widget, cursor_type);

						 
    /* Set the cursor on all subordinate windows */

    EnvDwLbBoxSetCursor(private, cursor_type);

    EnvCustSetCursor(private, cursor_type);

    return;
}
