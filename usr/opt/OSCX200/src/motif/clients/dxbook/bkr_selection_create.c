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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION_CREATE.C*/
/* *18   17-NOV-1992 22:51:37 BALLENGER "Special handling for Space and Return."*/
/* *17    8-NOV-1992 19:17:16 BALLENGER "Fix problem with updating view menu."*/
/* *16   30-OCT-1992 18:49:47 BALLENGER "Cleanup resource handling for window position and sizes."*/
/* *15   18-JUN-1992 15:27:52 BALLENGER "Don't call create_selection_menus as a workproc."*/
/* *14    9-JUN-1992 16:14:28 ROSE "Done"*/
/* *13    9-JUN-1992 10:40:50 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *12   15-MAY-1992 14:48:53 BALLENGER "Don't update view menu until after directory hierarchy created."*/
/* *11   14-MAR-1992 14:15:33 BALLENGER "Fix problems with window and icon titles..."*/
/* *10    5-MAR-1992 14:26:03 PARMENTER "adding simple search"*/
/* *9     3-MAR-1992 17:03:48 KARDON "UCXed"*/
/* *8    13-FEB-1992 18:23:31 BALLENGER "Conditionalize use of work procs."*/
/* *7    12-FEB-1992 12:14:23 PARMENTER "i18n support for titles and icons"*/
/* *6     5-FEB-1992 10:38:47 FITZELL "fixes accvio when no directoriea arre found"*/
/* *5     7-JAN-1992 16:51:10 PARMENTER "adding CBR/Search"*/
/* *4    16-DEC-1991 19:08:28 BALLENGER "Fix bug in getting x pos of previous window"*/
/* *3     1-NOV-1991 13:15:07 BALLENGER "Reintegrate memex support"*/
/* *2    18-SEP-1991 14:27:37 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:28 PARMENTER "Creates Selection window"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION_CREATE.C*/
#ifndef VMS
 /*
#else
#module BKR_SELECTION_CREATE "V03-0003"
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
**	Creation and initialization routines for the Selection window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     25-Oct-1989
**
**  MODIFICATION HISTORY:
**
**  V03-0004  DLB0003   David L Ballenger       30-Apr-1991
**            Make sure the selection window when resuing a selection shell
**            by calling RAISE_WINDOW at end of bkr_selection_create().
**
**  V03-0003  DLB0002	David L Ballenger	11-Feb-1991
**            Cache one dummy parent by leaving the book "open" when all
**            topic windows are closed, so that "reopening" the book
**            will be faster when coming in through HyperHelp.
**
**  V03-0002	DLB0001		David L Ballenger	06-Feb-1991
**		Fix spelling/capitalization of <Xm/Protcols.h>.
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
#include "br_resource.h"     /* typedefs and #defines for Xrm customizable
                                resources used in BR */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_selection.h"   /* function prototypes for .c module */
#include "bkr_selection_create.h"    /* function prototypes for .c module */
#include "bkr_copyright.h"   /* Copyright Display routines */
#include "bkr_error.h"       /* Error display routines */
#include "bkr_fetch.h"       /* Resource literal fetching */
#include "bkr_menu.h"        /* Menu handling routines */
#include "bkr_menu_create.h" /* Menu handling routines */
#include "bkr_resource.h"    /* Resource handling routines */
#ifdef SEARCH
#include "bkr_search.h"      /* function prototypes for .c module */
#endif /* SEARCH */
#include "bkr_window.h"      /* function prototypes for .c module */
#include <Xm/Protocols.h>
#include <X11/StringDefs.h>
#ifdef __osf__
#include "XctStrings.h"
#include "XcmStrings.h"
#else
#include <Xct/XctStrings.h>
#include <Xcm/XcmStrings.h>
#endif


/*
 * FORWARD ROUTINES
 */
static Boolean	    create_selection_menus
    PROTOTYPE((BKR_WINDOW	*window
               ));
static Boolean       create_selection_window
    PROTOTYPE((BKR_SHELL *shell
               ));



/*
 * FORWARD DEFINITIONS 
 */

static MrmRegisterArg	    tag_reglist[] = { { "tag", (caddr_t) 0 } };
static WidgetList   	    sel_popup_widgets = NULL;
static BKR_WINDOW  *default_selection_window;

static XtActionsRec svn_action_routines[] =
{
    {"SelectionKeyboardActions",	(XtActionProc)bkr_selection_keyboard_actions}
};

static String svn_translations_table = 
    "<Key>Return:	SelectionKeyboardActions(CollapseExpand)   \n\
     <Key>space:	SelectionKeyboardActions(OpenClose)";

static XtTranslations svn_translations = NULL;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_create
**
** 	Creates a new Selection window or re-uses a cached one to 
**  	save on the creation time.
**
**  FORMAL PARAMETERS:
**
**  	window_title	- string to set for the title bar title.
**  	icon_title  	- string to set for the icon title.
**	book_id		- id of the book.
**  	create_default	- Boolean: whether to create a "default" 
**  	    	    	    	Selection window.
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
Boolean
bkr_selection_create PARAM_NAMES((shell))
    BKR_SHELL *shell PARAM_END
{
    BKR_WINDOW	 *window = shell->selection;
    BKR_BOOK_CTX *book   = shell->book;
    Arg	    	 arglist[5];
    int	    	 argcnt;

    if ( window != NULL )
    {
        if (window->active || (window->type != BKR_SELECTION)) {
            return FALSE;
        }

    	window->title_bar_name = book->title;
    	window->icon_name = book->title;

        /* Set the title bar andicon names
         */
        bkr_window_set_names(window);

    } else {
    	/* Create a new Selection window.
	 */ 
        if (create_selection_window(shell)) {
            window = shell->selection;
        } else {
            return FALSE;
        }
    }

    /* Add the directory push buttons to the VIEW pulldown menu 
     */
    bkr_menu_update_view_pulldown( window );

   /* Display the Copyright message from the book if it wasn't already
    * displayed with a new window.
    */
    if (shell->n_active_windows == 0) {
        bkr_copyright_display(window);
    }

    bkr_window_setup( window->widgets[W_MAIN_WINDOW], NULL, FALSE );

    /* Note:  the call to XtPopup has been replaced with a manage call
     *	      due to the occurrance of Xtremovegrab errors.  We also
     *        need to do a RAISE_WINDOW to make sure it is mapped.
     */		
    window->active = TRUE;
    if ( ! XtIsManaged( window->widgets[W_MAIN_WINDOW] ) ) {
         XtManageChild( window->widgets[W_MAIN_WINDOW] );
         RAISE_WINDOW(window->appl_shell_id);
     }

    shell->n_active_windows++;

    return TRUE;

};  /* end of bkr_selection_create */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_create_popups
**
** 	Creates the Selection window popup menu.
**
**  FORMAL PARAMETERS:
**
**	popup_menu_rtn - address to return the popup menu created.
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
bkr_selection_create_popups PARAM_NAMES((popup_menu_rtn))
    WidgetList *popup_menu_rtn PARAM_END
{
    MrmType 	dummy_class;
    unsigned	status;
    char    	*sel_popup_menu_index = "selectionPopupMenu";

    *popup_menu_rtn = NULL; 	/* initial value */

    sel_popup_widgets = (WidgetList) BKR_CALLOC( K_MAX_SELECTION_POPUP_WIDGETS,
    	    	    	    	    	sizeof( Widget ) );
    BKR_UPDATE_TAG( sel_popup_widgets );
    status = MrmFetchWidget(
    	    	    bkr_hierarchy_id,
    	    	    sel_popup_menu_index,   	    	/* index	     */
    	    	    bkr_toplevel_widget,    	     	/* parent widget     */
    	    	    &sel_popup_widgets[W_SELECTION_POPUP], /* widget fetched */
    	    	    &dummy_class );	    	    	 /* unused class     */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, sel_popup_menu_index );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	BKR_CFREE( sel_popup_widgets );
    }

    /* Return the widget ids */

    *popup_menu_rtn = sel_popup_widgets;

};  /* end of bkr_selection_create_popups */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_selection_create_save_ids
**
** 	Callback routine to store the widget ids for the Selection 
**  	window popup menu.
**
**  FORMAL PARAMETERS:
**
**	widget	    - id of the widget to be saved
**	tag 	    - pointer to the tag passed from the UIL file
**	reason      - pointer callback data (not used)
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
bkr_selection_create_save_ids PARAM_NAMES((widget,tag,reason))
    Widget  	    	    widget PARAM_SEP
    int 	    	    *tag PARAM_SEP
    XmAnyCallbackStruct	    *reason PARAM_END

{
    int	    index = (int) *tag;

    if ( sel_popup_widgets == NULL )
    	return;
    	
    sel_popup_widgets[index] = widget;   	/* save it! */

};  /* end of bkr_selection_create_save_ids */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_selection_menus
**
** 	Work procedure to create the pulldown and popup menus, and 
**  	to install the accelerators for a given Selection window.
**
**  FORMAL PARAMETERS:
**
**	window - pointer to the Selection window.
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
create_selection_menus PARAM_NAMES((window))
    BKR_WINDOW	*window PARAM_END
{
    unsigned	    	    status1;
    unsigned	    	    status2;
    unsigned	    	    status3;

    /* Execute this procedure again later on */

    if ( window->widgets[W_SVN] == NULL )
    	return ( FALSE );

    /* make sure the book got opened succesfully */
    if(window->shell->book == NULL)
	return ( FALSE );

    status1 = bkr_menu_create_file( window );
    status2 = bkr_menu_create_view( window );
#ifdef SEARCH
    status3 = bkr_menu_create_search( window );
#else
    status3 = TRUE;
#endif

    bkr_menu_create_help(window);

    if ( status1 && status2 )
    	XtInstallAllAccelerators( window->widgets[W_MAIN_WINDOW], 
    	    	    	    	  window->widgets[W_MENU_BAR] );

    /* Only call this procedure once */

    return ( TRUE );

};  /* end of create_selection_menus */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_window
**
** 	Creates the selection window window and all of its associated widgets.
**
**  FORMAL PARAMETERS:
**
**  	title_bar_name	- string to set for the title bar title.
**  	icon_name   	- string to set for the icon title.
**      book_id		- id of book used to get copyright info.
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
**	Returns:   TRUE if the new Selection window was created
**  	            or FALSE if an error occurred.
**
**  SIDE EFFECTS:
**
**	virtual memory is allocated.
**
**--
**/
static Boolean
create_selection_window PARAM_NAMES((shell))
    BKR_SHELL *shell PARAM_END
{
    Arg     	    	    arglist[20];
    int     	    	    argcnt;
    unsigned	    	    status;
    Widget  	    	    new_appl_shell = NULL;
    Widget  	    	    new_main_window = NULL;
    BKR_WINDOW	    	    *window = NULL;
    MrmType	   	    dummy_class;
    Widget                  svn_primary_window;

    /* Fetch the window resources
     */
    bkr_resource_fetch_selection();

    /* Create a new selection window for the shell
     */
    window = (BKR_WINDOW *) BKR_MALLOC( sizeof( BKR_WINDOW ) );
    if (window) {
        memset( window, 0, sizeof( BKR_WINDOW ) );
        window->widgets = (WidgetList) BKR_CALLOC(K_MAX_SELECTION_WIDGETS,sizeof(Widget));
        if (window->widgets == NULL) {
            BKR_FREE(window);
            return FALSE;
        }
        memset(window->widgets,0,(K_MAX_SELECTION_WIDGETS * sizeof(Widget)));
        shell->selection = window;
        window->shell = shell;
    } else {
        return FALSE;
    }

    /* Initialize fields 
     */
    window->title_bar_name = shell->book->title;
    window->icon_name = shell->book->title;
    window->type = BKR_SELECTION;
    window->active = FALSE;
#ifdef SEARCH
        bkr_initialize_search_context(window);
#endif 

    /* Create the application window
     */ 
    argcnt = 0;

    if ((bkr_resources.x_offset) || (bkr_resources.y_offset)) 
    {
        /* If the x_offset or y_offset is non-zero, then we will override
         * position of the window and place it relative to the window that
         * it was opened from.
         */
        BKR_SHELL  *last_found     = NULL;
        Widget     last_appl_shell = NULL;
        
        /* Offset the position of this window from the position of the
         * last created selection window or the library window if there
         * are no selection windows
         */
        last_found = bkr_all_shells;
        while (last_found && (last_appl_shell == NULL)) {
            if ((last_found != shell) && last_found->selection) {
                last_appl_shell = last_found->selection->appl_shell_id;
                break;
            }
            last_found = last_found->all_shells;
        }
        
        if (last_appl_shell == NULL) {
            last_appl_shell = bkr_library->appl_shell_id;
        }
        if (last_appl_shell) {
            Position  new_x;
            Position  new_y;
            String     x_resource;
            String     y_resource;
            
            /* Determine whether to use the character cell or bitmap versions of
             * the resources.
             */
            if (bkrplus_g_charcell_display) 
            {
                x_resource = XctNcx;
                y_resource = XctNcy;
            }
            else 
            {
                x_resource = XtNx;
                y_resource = XtNy;
            }
            
            XtVaGetValues(last_appl_shell,
                          x_resource, &new_x,
                          y_resource, &new_y,
                          NULL);
            SET_ARG( x_resource, new_x + bkr_resources.x_offset );
            SET_ARG( y_resource, new_y + bkr_resources.y_offset );
        }
    }

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
        SET_ARG( XmNwidth, bkr_resources.width );
        SET_ARG( XmNheight, bkr_resources.height );
        SET_ARG( XmNminWidth, 300 );
        SET_ARG( XmNminHeight,300 );
        SET_ARG( XmNmaxWidth, bkr_display_width );
        SET_ARG( XmNmaxHeight, bkr_display_height );
    }

    SET_ARG( XmNallowShellResize, TRUE );
    SET_ARG( XmNdeleteResponse,	  XmDO_NOTHING );

    new_appl_shell = XtAppCreateShell(
    	    	    	BKR_SELECTION_WINDOW_NAME,  	/* application name  */
    	    	    	BKR_APPLICATION_CLASS, 		/* application class */
      	    	    	topLevelShellWidgetClass,       /* class	     */
    	    	    	bkr_display,
    	    	    	arglist, argcnt );
    if ( new_appl_shell == 0 )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "CREATE_APPL_WINDOW_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }
    window->appl_shell_id = new_appl_shell;

    /* Set the title bar and icon names
     */
    bkr_window_set_names(window);

    /* Setup the event handlers on the window and the protocols we
     * support 
     */
    bkr_window_setup_wm_protocols(window);

    /* Update the DRM tag and fetch the main window */

    BKR_UPDATE_TAG( window );
    status = MrmFetchWidget(
    	    	    bkr_hierarchy_id,
    	    	    "selectionMainWindow",  	/* index into UIL   	*/
    	    	    new_appl_shell,	    	/* parent widget    	*/
    	    	    &new_main_window,   	/* widget being fetched	*/
    	    	    &dummy_class);	    	/* unused class     	*/
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, "selectionMainWindow" );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }

    window->widgets[W_MAIN_WINDOW] = new_main_window;

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
    XtVaGetValues(window->widgets[W_SVN],DXmSvnNprimaryWindowWidget,&svn_primary_window,NULL);
    XtOverrideTranslations(svn_primary_window,svn_translations);

    /* Set the window as the user data for the SVN widget, so that we can access
     * the window from the action routines.
     */
    XtVaSetValues(svn_primary_window,XmNuserData,window,NULL);

    XtManageChild( new_main_window );

    /* Add the tab groups for keyboard traversal */
/***    XmAddTabGroup( window->widgets[W_SVN] ); ***/

    XtRealizeWidget( new_appl_shell );

    /* Set the icons only after we have realized the shell
     */
    bkr_window_set_icons(window);

    /* Create the pulldown and popup menus */
    create_selection_menus(window);

    /* Return the newly created window */

    return TRUE;

};  /* end create_selection_window */
