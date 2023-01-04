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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_LIBRARY_CREATE.C*/
/* *15   17-NOV-1992 22:51:24 BALLENGER "Special handling for Space and Return."*/
/* *14   30-OCT-1992 18:49:13 BALLENGER "Cleanup resource handling for window position and sizes."*/
/* *13   19-JUN-1992 20:19:38 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *12    8-MAY-1992 20:21:02 BALLENGER "UCX$CONVERT"*/
/* *11    8-MAY-1992 20:19:20 BALLENGER "Fix QAR 74, MWM close on library window causes crash."*/
/* *10   16-APR-1992 09:50:17 FITZELL "on alloc of widget array add 1 as the array is one based not zero based"*/
/* *9    14-MAR-1992 14:14:31 BALLENGER "Fix problems with window and icon titles..."*/
/* *8     5-MAR-1992 14:25:25 PARMENTER "adding simple search"*/
/* *7     3-MAR-1992 17:00:32 KARDON "UCXed"*/
/* *6    12-FEB-1992 12:04:35 PARMENTER "i18n support for titles and icons"*/
/* *5     7-JAN-1992 16:49:35 PARMENTER "adding CBR/Search"*/
/* *4    12-DEC-1991 14:48:24 BALLENGER "Fix LinkWorks coldstart timing  problems."*/
/* *3     1-NOV-1991 13:05:23 BALLENGER "reintegrate  memex support"*/
/* *2    17-SEP-1991 20:48:44 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:49 PARMENTER "Library window Creation"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_LIBRARY_CREATE.C*/
#ifndef VMS
 /*
#else
#module BKR_LIBRARY_CREATE "V03-0003"
#endif
#ifndef VMS
  */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Creation and initialization routines for the Library window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     3-Jan-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0003	DLB0003		David L Ballenger	30-Apr-1991
**           	Fix problems with handling of surrogate objects in the
**           	library window.
**
**  V03-0002	DLB0001		David L Ballenger	06-Feb-1991
**		Fix include of <DXm/DXmSvn.h> and spelling/capitalization
**              of <Xm/Protcols.h>.
**
**              Pass correct number of arguments to bkr_error_modal().
**
**  V03-0001	JAF0001	James A. Ferguson   	22-Jan-1990
**  	    	Fix QAR 684 - default icon used instead of Bookreaders. If the 
**  	    	icons can't be fetched when a window is created, an event 
**  	    	handler is setup to receive ReparentNotify events.  When the 
**  	    	reparent event is received the icons are set.  This is done 
**  	    	because the Bookreader might start before the window manager 
**  	    	does.  This will work for Reparenting window managers, but 
**  	    	non-reparenting will have problems but "that's life."
**
**--
**/


/*
 * INCLUDE FILES
 */

#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_library_create.h" /* function prototypes for .c module */
#include "bkr_copyright.h"   /* function prototypes for .c module */
#include "bkr_cursor.h"      /* function prototypes for .c module */
#include "bkr_error.h"       /* function prototypes for .c module */
#include "bkr_fetch.h"       /* function prototypes for .c module */
#include "bkr_library.h"     /* function prototypes for .c module */
#include "bkr_menu_create.h" /* function prototypes for .c module */
#include "bkr_resource.h"    /* function prototypes for .c module */
#include "bkr_window.h"      /* function prototypes for .c module */
#ifdef MEMEX
#include "bmi_library.h"
#endif
#include  <Xm/Protocols.h>


/*
 * FORWARD ROUTINES
 */


static Boolean	    create_library_menus();
static void  	    create_library_popups();


/*
 * FORWARD DEFINITIONS 
 */

static Widget	    	    open_library_file_selection = NULL;
static MrmRegisterArg	    tag_reglist[] = { { "tag", (caddr_t) 0 } };


static XtActionsRec svn_action_routines[] =
{
    {"LibraryKeyboardActions",	(XtActionProc)bkr_library_keyboard_actions}
};

static String svn_translations_table = 
    "<Key>osfActivate:	LibraryKeyboardActions(CollapseExpand)   \n\
     <Key>osfSelect:	LibraryKeyboardActions(OpenClose) \n\
     <Key>Return:	LibraryKeyboardActions(CollapseExpand)   \n\
     <Key>space:	LibraryKeyboardActions(OpenClose)";

static XtTranslations svn_translations = NULL;

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_popup_menu
**
** 	Callback routine which handles displaying the appropriate
**  	popup menu.
**
**  FORMAL PARAMETERS:
**
**  	svn_widget  - id of the widget that caused the callback.
**  	unused_tag  - user data.
**  	data	    - SVN callback data.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_library_popup_menu PARAM_NAMES((svn_widget,unused_tag,data))
    Widget  	    	    svn_widget PARAM_SEP
    int	    	    	    unused_tag PARAM_SEP
    DXmSvnCallbackStruct    *data PARAM_END

{
    XButtonPressedEvent	*xbutton;
    XButtonPressedEvent	dummy_xbutton;
    BKR_NODE	    	*node;
    Widget  	    	popup_menu = NULL;
    Widget  	    	*widget_array = &bkr_library->widgets[0];

    node = (BKR_NODE *) data->entry_tag;
    if ( node == NULL )
    {
    	node = (BKR_NODE *) DXmSvnGetEntryTag( svn_widget, data->entry_number );
    	if ( node == NULL )
    	    return;
    }

    /* Decide which popup menu to map */

    switch ( node->entry_type )
    {
    	case BKR_BOOK_FILE :
    	    popup_menu = (Widget) widget_array[W_BOOK_POPUP];
    	    break;
    	case BKR_SHELF_FILE :
    	    popup_menu = (Widget) widget_array[W_SHELF_POPUP];
    	    break;
    }
    if ( popup_menu == NULL )
    	return;
    
    /* Update the NODE pointer for this mapping and highlight the entry node */

    bkr_library->u.library.btn3down_entry_node = node;
    bkr_library_highlight_entry( svn_widget, NULL, data->entry_number, ON );

    /* Set the sensitivity of the push button entries 
     */
    switch ( node->entry_type )
    {
    	case BKR_BOOK_FILE :
    	    XtSetSensitive( widget_array[W_OPEN_IN_DEFAULT_POPUP_ENTRY], TRUE );
    	    XtSetSensitive( widget_array[W_OPEN_IN_NEW_POPUP_ENTRY], TRUE );
    	    break;
    	case BKR_SHELF_FILE :
    	    if ( node->u.shelf.opened )
    	    {
    	    	if ( ! XtIsManaged( widget_array[W_CLOSE_SHELF_POPUP_ENTRY] ) )
    	    	    XtManageChild( widget_array[W_CLOSE_SHELF_POPUP_ENTRY] );
    	    	if ( XtIsManaged( widget_array[W_OPEN_SHELF_POPUP_ENTRY] ) )
    	    	    XtUnmanageChild( widget_array[W_OPEN_SHELF_POPUP_ENTRY] );
    	    }
    	    else    /* node is closed */
    	    {
    	    	if ( ! XtIsManaged( widget_array[W_OPEN_SHELF_POPUP_ENTRY] ) )
    	    	    XtManageChild( widget_array[W_OPEN_SHELF_POPUP_ENTRY] );
    	    	if ( XtIsManaged( widget_array[W_CLOSE_SHELF_POPUP_ENTRY] ) )
    	    	    XtUnmanageChild( widget_array[W_CLOSE_SHELF_POPUP_ENTRY] );
    	    }
    	    break;
    }

    /*  If its a ButtonPress then use the event to position the popup
     *  menu, otherwise if its a KeyPress build a dummy XButton event.
     */
    if ( data->event->type == ButtonPress )
    	xbutton = (XButtonPressedEvent *) &data->event->xbutton;
    else if ( data->event->type == KeyPress )
    {
    	Window	junk_window;

    	xbutton = (XButtonPressedEvent *) &dummy_xbutton;
	xbutton->type	     = ButtonPress;
	xbutton->serial	     = 0;
	xbutton->send_event  = False;
	xbutton->display     = bkr_display;
	xbutton->window	     = XtWindow( svn_widget );
	xbutton->root	     = XDefaultRootWindow( bkr_display );
	xbutton->subwindow   = XtWindow( svn_widget );
	xbutton->time	     = data->event->xkey.time;
	xbutton->x   	     = 0;
    	xbutton->y   	     = 0;
	xbutton->state	     = 0;
	xbutton->button	     = Button3;
	xbutton->same_screen = True;
    	XTranslateCoordinates( bkr_display, xbutton->window, 
    	    	xbutton->root, xbutton->x, xbutton->y,
    	    	&xbutton->x_root, &xbutton->y_root, 
    	    	&junk_window );
    }

    /* Position the popup menu then manage it. */

    if ( ! XtIsRealized( popup_menu ) )
    	XtRealizeWidget( popup_menu );
    XmMenuPosition( popup_menu, xbutton );
    XtManageChild( popup_menu );

};  /* end of bkr_library_popup_menu */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_window_create
**
** 	Creates the Library window and does the needed initialization.
**
**  FORMAL PARAMETERS:
**
**  	start_as_icon - Boolean: whether to create window iconified.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	virtual memory is allocated.
**
**--
**/
void
bkr_library_window_create PARAM_NAMES((start_as_icon))
    Boolean 	start_as_icon PARAM_END
{
    Arg	    	arglist[20];
    int     	argcnt;
    unsigned	status;
    Widget  	new_main_window = NULL;    	    
    Widget      svn_primary_window;
    MrmType 	dummy_class;

    if ( bkr_library != NULL )
    	return;

    /* Create a Library window
     */
    bkr_library = (BKR_WINDOW *) BKR_MALLOC( sizeof(BKR_WINDOW) );
    memset( bkr_library, 0, sizeof(BKR_WINDOW) );
    bkr_library->type = BKR_LIBRARY;

    /* Get icon name and titlebar names for the library window
     * from the UID file 
     */
    bkr_library->icon_name = (char *) bkr_fetch_literal("s_library_window_icon_name",
                                                        MrmRtypeChar8 );
    bkr_library->title_bar_name = (char *) bkr_fetch_literal("s_library_window_title",
                                                             MrmRtypeChar8 );
    /* Create the application shell */

    argcnt = 0;

    if (bkrplus_g_charcell_display) 
    {
        /* Height and width are specified by the cheight and
         * cwidth resources in the app-defaults file, so
         * we don't explicitly set the height or the width,
         * we just prevent them from being set to small or
         * to large in the resource file.
         */
        SET_ARG( XmNminWidth, bkr_default_space_width * 40 );
        SET_ARG( XmNminHeight, bkr_default_line_height * 10 );
        SET_ARG( XmNmaxWidth, bkr_display_width );
        SET_ARG( XmNmaxHeight, bkr_display_height );
    }
    else 
    {
        /* Use the values converted from millimeters as specified by
         * the mm_{width,height} resources in the apps-default file.
         */
        SET_ARG( XmNwidth,bkr_library_resources.width );
        SET_ARG( XmNheight,bkr_library_resources.height );
        SET_ARG( XmNminWidth, 300 );
        SET_ARG( XmNminHeight,300 );
        SET_ARG( XmNmaxWidth, bkr_display_width );
        SET_ARG( XmNmaxHeight, bkr_display_height );
    }

    SET_ARG( XmNallowShellResize, TRUE );
    SET_ARG( XmNdeleteResponse,	  XmDO_NOTHING );

    if ( start_as_icon || (bkr_library_resources.initial_state == IconicState))
    	SET_ARG( XmNinitialState, IconicState );

    bkr_library->appl_shell_id 
    = XtAppCreateShell(BKR_LIBRARY_WINDOW_NAME,         /* application name  */
                       BKR_APPLICATION_CLASS, 		/* application class */
                       applicationShellWidgetClass,    /* class */
                       bkr_display,
                       arglist, argcnt ); 
    if ( bkr_library->appl_shell_id == 0 )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "CREATE_APPL_SHELL_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return;
    }

    /* Set the title bar and icon names
     */
    bkr_window_set_names(bkr_library);

    /* Setup the event handlers on the shell and the protocols we
     */
    bkr_window_setup_wm_protocols(bkr_library);

#ifdef SEARCH
    bkr_initialize_search_context(bkr_library);
#endif 

    /* Update the MRM tag before fetching the widgets.
     */
    bkr_library->widgets = (Widget *)BKR_CALLOC( (K_MAX_LIBRARY_WIDGETS + 1), 
						 sizeof(Widget));

    BKR_UPDATE_TAG( bkr_library );    

    status = MrmFetchWidget(
    	    	    bkr_hierarchy_id,
    	    	    "libraryWindow",   	    	/* index into UIL   	*/
    	    	    bkr_library->appl_shell_id,	/* parent widget    	*/
    	    	    &new_main_window,   	/* widget being fetched	*/
    	    	    &dummy_class);	    	/* unused class     	*/
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, "libraryWindow" );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return;
    }
    bkr_library->widgets[W_MAIN_WINDOW] = new_main_window;

    XtManageChild( new_main_window );

    if (svn_translations == NULL) 
    {
        /* Parse the translation table.
         */
        svn_translations = XtParseTranslationTable(svn_translations_table);

        /* Add the actions for our translation tables 
         */
        XtAppAddActions( bkr_app_context, svn_action_routines, XtNumber(svn_action_routines));
    }

    /* Override the translations in the SVN primary window widget.
     */
    XtVaGetValues(bkr_library->widgets[W_SVN],
                  DXmSvnNprimaryWindowWidget,
                  &svn_primary_window,
                  NULL);
    XtOverrideTranslations(svn_primary_window,svn_translations);

    /* Set the window as the user data for the SVN widget, so that we can access
     * the window from the action routines.
     */
    XtVaSetValues(svn_primary_window,XmNuserData,bkr_library,NULL);

    /* Add event handlers to Library Window for Application Copyright
     * notice and display the copyright in the title bar.
     */
    bkr_copyright_display(bkr_library);

    XtRealizeWidget( bkr_library->appl_shell_id );
    bkr_cursor_display_wait( ON );

    /* Set the icons only after we have realized the shell
     */
    bkr_window_set_icons(bkr_library);

#ifdef MEMEX
    /* Create the Memex UI for the Library window.
     */
    bmi_create_library_ui( bkr_library );

    /* Create the pulldown and popup menus 
     */
    create_library_menus();
#else
    /* Create the pulldown and popup menus using a workproc
     */
    (void) XtAppAddWorkProc( XtWidgetToApplicationContext( bkr_library->appl_shell_id ),
    	    	    	     create_library_menus, NULL );
#endif 

    bkr_library->active = TRUE;

    bkr_cursor_display_wait( OFF );

};  /* end bkr_library_window_create */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_library_save_widgets
**
** 	Save the individual widget ids during widget creation from a fetch
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget to be saved
**	tag 	    	- pointer to the tag passed from the UIL file
**	reason  	- pointer callback data (not used)
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_library_save_widgets PARAM_NAMES((widget,tag,reason))
    Widget	    	widget PARAM_SEP
    int 	    	*tag PARAM_SEP
    XmAnyCallbackStruct *reason PARAM_END

{
    int     index = (int) *tag;

    if ( ( index < 1 ) || ( index > K_MAX_LIBRARY_WIDGETS ) )
    	return;

#ifdef DEBUG
    printf( "saved %31s = %2d\n", XtName( widget ), index );
#endif

    bkr_library->widgets[index] = widget;    	/* save it! */

};  /* end bkr_library_save_widgets */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_library_menus 
**
** 	Work procedure to create the pulldown and popup menus, and 
**  	to install the accelerators for the Library window.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    TRUE if the work procedure completed.
**  	    	    FALSE if the work procedure needs to be called again.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static Boolean
create_library_menus(VOID_PARAM)
{
    Widget  	*widget_array = &bkr_library->widgets[0];

    /* Execute this procedure again later on */

    if ( widget_array[W_SVN] == NULL )
    	return ( FALSE );

    (void)bkr_menu_create_file(bkr_library);
    (void)bkr_menu_create_view(bkr_library);
#ifdef SEARCH
    (void)bkr_menu_create_search(bkr_library);
#endif
    bkr_menu_create_help(bkr_library);
    create_library_popups( widget_array[W_SVN] );

    XtInstallAllAccelerators( widget_array[W_MAIN_WINDOW], widget_array[W_MENU_BAR] );

    /* Only call this procedure once */

    return ( TRUE );

};  /* end of create_library_menus */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_library_popups
**
** 	Fetches the popup menus for a given shell.
**
**  FORMAL PARAMETERS:
**
**	parent_widget - id of parent widget for popup menus.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static void
create_library_popups( parent_widget )
    Widget  parent_widget;
{
    Widget  	*book_popup  = (Widget *) &bkr_library->widgets[W_BOOK_POPUP];
    Widget  	*shelf_popup = (Widget *) &bkr_library->widgets[W_SHELF_POPUP];
    char    	*book_index  = "libraryWindowBookEntryPopup";
    char    	*shelf_index = "libraryWindowShelfEntryPopup";
    unsigned	status;
    MrmType 	dummy_class;

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	book_index, 	    	/* index into UIL */
    	    	parent_widget, 	    	/* parent widget  */
    	    	book_popup, 	    	/* widget fetched */
    	    	&dummy_class );	    	/* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, book_index );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    }

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	shelf_index,	    	/* index into UIL */
    	    	parent_widget, 	    	/* parent widget  */
    	    	shelf_popup, 	    	/* widget fetched */
    	    	&dummy_class );	    	/* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, shelf_index );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    }

};  /* end of create_library_popups */


