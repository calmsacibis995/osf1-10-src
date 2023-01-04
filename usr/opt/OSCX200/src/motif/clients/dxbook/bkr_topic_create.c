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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CREATE.C*/
/* *23   17-NOV-1992 22:52:49 BALLENGER "Create CC-specific help menus."*/
/* *22    8-NOV-1992 19:15:48 BALLENGER "Fix handling of Next/Prev Screen keys."*/
/* *21   30-OCT-1992 18:49:17 BALLENGER "Cleanup resource handling for window position and sizes."*/
/* *20   12-OCT-1992 17:39:23 BALLENGER "Fix problems with formal topics in CC mode and clean up window creation."*/
/* *19   21-SEP-1992 22:13:46 BALLENGER "Focus highlight in topic window."*/
/* *18    5-AUG-1992 21:47:52 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *17   22-JUN-1992 19:23:21 BALLENGER "Make sure show exstensions is set correctly at window creation."*/
/* *16   18-JUN-1992 16:01:27 BALLENGER "Don't call create_topic_menus as a workproc."*/
/* *15    9-JUN-1992 09:59:19 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *14    3-APR-1992 17:19:27 FITZELL "decworld hooks"*/
/* *13   19-MAR-1992 11:59:28 FITZELL "put in edit button/pulldown for clipboard"*/
/* *12   14-MAR-1992 14:15:40 BALLENGER "Fix problems with window and icon titles..."*/
/* *11    3-MAR-1992 17:05:13 KARDON "UCXed"*/
/* *10   12-FEB-1992 12:14:27 PARMENTER "i18n support for titles and icons"*/
/* *9    11-FEB-1992 16:27:17 KLUM "removed bkr_copy_clipboard.h"*/
/* *8    11-FEB-1992 16:24:45 KLUM "removed bkr_sprite_position"*/
/* *7     6-FEB-1992 10:12:49 KLUM "to add cut-to-buffer"*/
/* *6     7-JAN-1992 16:51:46 PARMENTER "adding CBR/Search"*/
/* *5    16-DEC-1991 19:08:52 BALLENGER "Offset topic window from library window if no navigation window"*/
/* *4    16-DEC-1991 14:48:53 BALLENGER "Offset topic window from library window if no navigation window"*/
/* *3     1-NOV-1991 12:58:48 BALLENGER "Reintegrate  memex support"*/
/* *2    18-SEP-1991 18:32:55 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:51 PARMENTER "Creation routines for STANDARD and FORMAL Topic windows"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CREATE.C*/
#ifndef VMS
 /*
#else
#module BKR_TOPIC_CREATE "V03-0001"
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
**	Creation and initialization routines for the both STANDARD and
**  	FORMAL Topic windows.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     6-Nov-1989
**
**  MODIFICATION HISTORY:
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
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_topic_create.h" /* function prototypes for .c module */
#include "bkr_error.h"       /* Error reporting */
#include "bkr_fetch.h"       /* Fetch resource literals */
#include "bkr_menu_create.h" /* Menu creation */
#include "bkr_resource.h"    /* Resource routines */
#include "bkr_window.h"      /* function prototypes for .c module */
#ifdef MEMEX
#include "bmi_topic.h"
#endif
#include  <X11/StringDefs.h>
#include  <Xm/Protocols.h>
#include  <DXm/DECspecific.h>
#ifdef DECWORLD
#include "bkr_decworld.h"
#endif
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

static Boolean	    
create_topic_menus PROTOTYPE((BKR_WINDOW *window));

static void  	    
set_formal_topic_size PROTOTYPE((BKR_WINDOW *window));


/*
 * FORWARD DEFINITIONS 
 */

static MrmRegisterArg	    tag_reglist[] = { { "tag", (caddr_t) 0 } };
static WidgetList   	    topic_popup_widgets = NULL;

static String next_prev_screen_translations_table = 
    "<Key>osfPageUp:				TopicKeyboard(PrevScreen)   \n\
     <Key>osfPageDown:				TopicKeyboard(NextScreen)";

static String drawing_area_translations_table = 
    "<Key>osfPageUp:				TopicKeyboard(PrevScreen)   \n\
     <Key>osfPageDown:				TopicKeyboard(NextScreen)   \n\
     <Key>osfUp:				TopicKeyboard(PrevHotSpot)  \n\
     <Key>osfDown:				TopicKeyboard(NextHotSpot)  \n\
     ~Shift <Key>Execute:			TopicKeyboard(ViewHotSpot)  \n\
     ~Shift <Key>Select:			TopicKeyboard(ViewHotSpot)  \n\
     ~Shift <Key>space:				TopicKeyboard(ViewHotSpot)  \n\
     Shift <Key>Execute:			TopicKeyboard(ViewHotSpotInNew)  \n\
     Shift <Key>Select:				TopicKeyboard(ViewHotSpotInNew)  \n\
     Shift <Key>space:				TopicKeyboard(ViewHotSpotInNew)  \n\
     <FocusIn>:					TopicFocus(In) \n\
     <FocusOut>:				TopicFocus(Out) \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn1Motion>:	MB1_MOTION()  \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn1Down>:	MB1_ACTION(1) TopicFocus(Grab) \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn1Up>:		MB1_ACTION(2)  \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn2Motion>:	MB2_MOTION()  \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn2Down>:	MB2_ACTION(1)  \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn2Up>:		MB2_ACTION(2)  \n\
     ~Shift ~Ctrl ~Meta ~Help <Btn3Down>:	MB3_PRESSED()  \n\
     <Leave>:					LEAVE_WINDOW()  \n\
     <Motion>:					MOTION()  \n\
     <GrExp>:					GRAPHICS_EXPOSE()  \n\
     <NoExp>:					GRAPHICS_EXPOSE()";

static XtTranslations next_prev_screen_translations = NULL;
static XtTranslations drawing_area_translations = NULL;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_create_window
**
** 	Creates a the window to display a topic in.
**
**  FORMAL PARAMETERS:
**
**      window - pointer to the BKR_WINDOW structure that describes
**               the topic window to create.
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
**	Returns:    TRUE if the window is created, FALSE if not.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
Boolean
bkr_topic_create_window PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END

{
    BKR_SHELL           *shell = window->shell;
    Arg     	    	arglist[20];
    int     	    	argcnt;
    unsigned	    	status;
    Widget  	    	new_appl_shell = NULL;
    MrmType	   	dummy_class;
    char    	    	*index_name;
    Pixel               highlightColor;
    Pixel               backgroundColor;

    /* Fetch the shell resources
     */
    bkr_resource_fetch_topic();

    if (drawing_area_translations == NULL) {
        drawing_area_translations = XtParseTranslationTable(drawing_area_translations_table);
    }
    if (next_prev_screen_translations == NULL) {
        next_prev_screen_translations = XtParseTranslationTable(next_prev_screen_translations_table);
    }

    /* Set initial state for extesnions.
     */
    window->u.topic.show_extensions  = bkr_topic_resources.show_extensions;

    /* 
     *  Create the application shell.  For FORMAL topic shells set the width 
     *  and height to 0 because the shell size will be calculated later on.
     */
    argcnt = 0;
    
    if ((bkr_topic_resources.x_offset) || (bkr_topic_resources.y_offset)) 
    {
        /* If the x_offset or y_offset is non-zero, then we will override
         * position of the window and place it relative to the window that
         * it was opened from.
         */
        Position   x_offset     = bkr_topic_resources.x_offset;
        Position   y_offset     = bkr_topic_resources.y_offset;
        BKR_WINDOW *last_window = shell->other_topics;
        String     x_resource;
        String     y_resource;
        String     width_resource;

        /* Determine whether to use the character cell or bitmap versions of
         * the resources.
         */
        if (bkrplus_g_charcell_display) 
        {
            x_resource = XctNcx;
            y_resource = XctNcy;
            width_resource = XctNcwidth;
        }
        else 
        {
            x_resource = XtNx;
            y_resource = XtNy;
            width_resource = XtNwidth;
        }
        while (last_window 
               && ((last_window == window) || (last_window->appl_shell_id == NULL))
               ) {
            last_window = last_window->u.topic.sibling;
        }
        if ((last_window == NULL) || (last_window->appl_shell_id == NULL)) {
            last_window = shell->default_topic;
            if ((last_window == NULL) || (last_window->appl_shell_id == NULL)) {
                last_window = shell->selection;
                if ((last_window == NULL) || (last_window->appl_shell_id == NULL)) {
                    last_window = bkr_library;
                }
                if  (last_window && last_window->appl_shell_id) {
                    Position  width;
                    
                    XtVaGetValues(last_window->appl_shell_id,
                                  width_resource,&width,
                                  NULL);
                    x_offset += width;
                    y_offset = 0;
                }
            }
        }
        
        if  (last_window && last_window->appl_shell_id) {
            Arg       arg[2];
            Position  new_x;
            Position  new_y;
            
            XtVaGetValues(last_window->appl_shell_id,
                          x_resource,&new_x,
                          y_resource,&new_y,
                          NULL);
            SET_ARG( x_resource, new_x + x_offset );
            SET_ARG( y_resource, new_y + y_offset );
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
        SET_ARG( XmNallowShellResize, TRUE );
        SET_ARG( XmNminWidth, bkr_default_space_width * 40 );
        SET_ARG( XmNminHeight,bkr_default_line_height * 10 );
        SET_ARG( XmNmaxWidth, bkr_display_width );
        SET_ARG( XmNmaxHeight, bkr_display_height );
    }
    else 
    {
        if ( window->type == BKR_FORMAL_TOPIC )
        {
            /* Height and width are specified by the topic itself. Note that
             * we override thsi for character cell.
             */
            SET_ARG( XmNwidth, 0 );
            SET_ARG( XmNheight, 0 );
            SET_ARG( XmNallowShellResize, FALSE );
        }
        else if ( window->type == BKR_STANDARD_TOPIC )
        {
            /* Use the values converted from millimeters as specified by
             * the mm_{width,height} resources in the apps-default file.
             */
            SET_ARG( XmNwidth,  bkr_topic_resources.width );
            SET_ARG( XmNheight, bkr_topic_resources.height );
            SET_ARG( XmNallowShellResize, TRUE );
        }
        SET_ARG( XmNminWidth, 300 );
        SET_ARG( XmNminHeight,300 );
        SET_ARG( XmNmaxWidth, bkr_display_width );
        SET_ARG( XmNmaxHeight, bkr_display_height );
    }
    SET_ARG( XmNdeleteResponse, XmDO_NOTHING );
    
    
    new_appl_shell = XtAppCreateShell(BKR_TOPIC_WINDOW_NAME,	    	/* application name  */
                                      BKR_APPLICATION_CLASS, 		/* application class */
                                      topLevelShellWidgetClass,     	/* class	     */ 
                                      bkr_display,
                                      arglist, argcnt);	    	/* command line args */
    if ( new_appl_shell == NULL )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "CREATE_APPL_SHELL_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    } else {
        window->appl_shell_id = new_appl_shell;
    }
    
    /* Setup the event handlers on the shell and the protocols we
     * support
     */
    bkr_window_setup_wm_protocols(window);
    
    /*
     *  Decide which type of window to fetch, determine the size 
     *  of the widgetlist array and then allocate some memory for the TOPIC
     *  window data structure.
     */
    switch ( window->type )
    {
    	case BKR_STANDARD_TOPIC : {
            index_name = "standardTopicMainWindow";
            break;
        }
    	case BKR_FORMAL_TOPIC : {
    	    index_name = "formalTopicMainWindow";
    	    break;
        }
    }
    
    /* Update the Mrm tag and fetch the main window
     */
    BKR_UPDATE_TAG( window );
    
    status = MrmFetchWidget(bkr_hierarchy_id,
                            index_name,	    	    	/* index into UIL   	*/
                            new_appl_shell,	    	/* parent widget    	*/
                            &window->widgets[W_MAIN_WINDOW],   	/* widget being fetched	*/
                            &dummy_class);	    	/* unused class     	*/
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, index_name );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	return FALSE;
    }

    /* Add translations to handle Next Screen and Previous Screen
     * buttons in the button box and horizontal scroll bar. The vertical
     * scroll bar already handles it.
     */
    XtOverrideTranslations(window->widgets[W_BUTTON_BOX],next_prev_screen_translations);
    XtOverrideTranslations(window->widgets[W_HSCROLL_BAR],next_prev_screen_translations);
    XtOverrideTranslations(window->widgets[W_VSCROLL_BAR],next_prev_screen_translations);
    XtOverrideTranslations(window->widgets[W_MENU_BAR],next_prev_screen_translations);

    /* Override the translations for the drawing area
     */
    XtOverrideTranslations(window->widgets[W_WINDOW],drawing_area_translations);

    if ( ! bkrplus_g_charcell_display) 
    {
        /* In character cell mode, this is all handled in the  
         * bkr_topic_focus_highlight() routine by toggling the scroll window
         * border colors.
         * 
         * We use the drawing window border as the focus highlight rectangle,
         * so setup Pixmaps to use for toggling highlighting in the drawing 
         * window.
         */
        Screen *screen = XtScreen(window->widgets[W_WINDOW]);

        argcnt = 0;
        SET_ARG(XmNhighlightColor,&highlightColor);
        SET_ARG(XmNhighlightPixmap,&window->u.topic.drawing_win_highlight_on);
        SET_ARG(XmNbackground,&backgroundColor);
        SET_ARG(XmNbackgroundPixmap,&window->u.topic.drawing_win_highlight_off);
        XtGetValues(window->widgets[W_WINDOW],arglist,argcnt);
        
        /* If there is no highlight Pixmap create one that is solid highlight 
         * color.
         */
        if (window->u.topic.drawing_win_highlight_on == XmUNSPECIFIED_PIXMAP)
        {
            window->u.topic.drawing_win_highlight_on = XmGetPixmap(screen,
                                                                   "background",
                                                                   highlightColor,
                                                                   highlightColor);
        }
    
        /* If there is no background Pixmap create one that is solid color
         * color and use that for the highlight off pixmap.
         */
        if (window->u.topic.drawing_win_highlight_off == XmUNSPECIFIED_PIXMAP)
        {
            window->u.topic.drawing_win_highlight_off = XmGetPixmap(screen,
                                                                    "background",
                                                                    backgroundColor,
                                                                    backgroundColor);
        }

        argcnt = 0;
        SET_ARG(XmNborderWidth,2);
        XtSetValues(window->widgets[W_WINDOW],arglist,argcnt);
    }    

    /* Resize the Formal topic to its appropriate size */
    
    if ( ! bkrplus_g_charcell_display
        && ( window->type == BKR_FORMAL_TOPIC )
        ){
    	set_formal_topic_size( window );
    }
    
    /* Create the GC's */
    
    bkr_window_initialize_gc( window );
    
    /* Create the pulldown and popup menus */
    
    create_topic_menus( window );

#ifdef DECWORLD
    if(bkr_decworld_start_timer){
        bkr_decworld_reset( window->widgets[W_MAIN_WINDOW], NULL );
        bkr_decworld_start_timer = FALSE;
        }
#endif
    return TRUE;

};  /* end of bkr_topic_create_window */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_create_popups
**
** 	Fetches the Topic window popup menu.
**
**  FORMAL PARAMETERS:
**
**	popup_menu_rtn - address to return the created popup widget.
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
bkr_topic_create_popups PARAM_NAMES((popup_menu_rtn))
    WidgetList	*popup_menu_rtn PARAM_END
{
    MrmType 	dummy_class;
    unsigned	status;
    char    	*topic_popup_menu_index = "topicPopupMenu";

    *popup_menu_rtn = NULL; 	/* initial value */

    topic_popup_widgets = (Widget *) BKR_CALLOC( K_MAX_TOPIC_POPUP_WIDGETS,
    	    	    	    	    	sizeof( Widget ) );
    BKR_UPDATE_TAG( topic_popup_widgets );
    status = MrmFetchWidget(
    	    	    bkr_hierarchy_id,
    	    	    topic_popup_menu_index, 	    	 /* index	   */
    	    	    bkr_toplevel_widget,    	    	 /* parent widget  */
    	    	    &topic_popup_widgets[W_TOPIC_POPUP], /* widget fetched */
    	    	    &dummy_class );	    	    	 /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, topic_popup_menu_index );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    	BKR_CFREE( topic_popup_widgets );
    }

    /* Return the widget id */

    *popup_menu_rtn = topic_popup_widgets;

};  /* end of bkr_topic_create_popups */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_create_save_ids
**
** 	Callback routine to store the widget ids for the Topic window
**  	popup menu.
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
bkr_topic_create_save_ids PARAM_NAMES((widget,tag,reason))
    Widget  	    	    widget PARAM_SEP
    int 	    	    *tag PARAM_SEP
    XmAnyCallbackStruct	    *reason PARAM_END

{
    int	    index = (int) *tag;

    if ( topic_popup_widgets == NULL )
    	return;
    	
    topic_popup_widgets[index] = widget;   	/* save it! */

};  /* end of bkr_topic_create_save_ids */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	create_topic_menus
**
** 	Work procedure to create the pulldown and popup menus, and 
**  	to install the accelerators for a given Topic shell.
**
**  FORMAL PARAMETERS:
**
**	shell - pointer to the Topic shell.
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
create_topic_menus PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    unsigned	status1;
    unsigned	status2;
    unsigned	status3;
    unsigned	status4;

    /* Execute this procedure again later on */

    if ( window->widgets[W_WINDOW] == NULL )
    	return ( FALSE );

    status1 = bkr_menu_create_file( window );
#ifdef COPY
    status2 = bkr_menu_create_edit( window );
#endif
    status3 = bkr_menu_create_view( window );
#ifdef SEARCH
    status4 = bkr_menu_create_search( window );
#else
    status4 = TRUE;
#endif
    (void)bkr_menu_create_help( window );

    if ( status1 && status3 ) {
    	XtInstallAllAccelerators( window->widgets[W_MAIN_WINDOW], 
    	    	    	    	  window->widgets[W_MENU_BAR] );
    }

    /* Only call this procedure once */

    return ( TRUE );

};  /* end of create_topic_menus */
    
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	set_formal_topic_size
**
** 	Calculates the dimensions for a FORMAL topic shell.
**
**  FORMAL PARAMETERS:
**
**  	shell	    	    	- id of the shell widget.
**  	drawing_window_width	- width of the drawing window.
**  	drawing_window_height	- height of the drawing window.
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
set_formal_topic_size PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    Arg     	arglist[20];
    int	    	argcnt;
    int	    	shell_width;
    int	    	shell_height;
    Dimension	main_window_border;
    Dimension	work_area_border;
    Dimension	scrolled_window_border;
    Dimension	window_border;
    Dimension	hscroll_height;
    Dimension	hscroll_border;
    Dimension	vscroll_width;
    Dimension	vscroll_border;
    Dimension	menu_bar_border;
    Dimension	menu_bar_height;
    Dimension	label_height;
    Dimension	label_border;
    Dimension	new_window_width;
    Dimension	new_window_height;

    argcnt = 0;
    SET_ARG( XmNborderWidth, &main_window_border );
    XtGetValues( window->widgets[W_MAIN_WINDOW], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNborderWidth, &work_area_border );
    XtGetValues( window->widgets[W_WORK_AREA], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNborderWidth, &scrolled_window_border );
    XtGetValues( window->widgets[W_SCROLLED_WINDOW], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNborderWidth, &window_border );
    XtGetValues( window->widgets[W_WINDOW], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNheight,	    	&hscroll_height );
    SET_ARG( XmNborderWidth,	&hscroll_border );
    XtGetValues( window->widgets[W_HSCROLL_BAR], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNwidth, 	    	&vscroll_width );
    SET_ARG( XmNborderWidth,	&vscroll_border );
    XtGetValues( window->widgets[W_VSCROLL_BAR], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNheight,	    	&menu_bar_height );
    SET_ARG( XmNborderWidth,	&menu_bar_border );
    XtGetValues( window->widgets[W_MENU_BAR], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNheight,	     &label_height );
    SET_ARG( XmNborderWidth, &label_border );
    XtGetValues( window->widgets[W_LABEL], arglist, argcnt );

    shell_width = ( main_window_border * 2 ) 	    +
    	    	    ( work_area_border * 2 ) 	    +
    	    	    ( scrolled_window_border * 2 )  +
    	    	    window->u.topic.width           +
     	    	    ( window_border  * 2 ) 	    +
    	    	    vscroll_width   	     	    +
    	    	    ( vscroll_border * 2 );
#ifdef FOO
    if (bkrplus_g_charcell_display) {
        shell_width = MAX(shell_width,bkr_topic_resources.width);
    }
#endif 
    /* 
     *  Note:  We maximize the drawing window height to not be less 
     *  than 150 pixels otherwise the menu bar will wrap and screw up
     *  our height calculations once the topic shell is realized.
     */

    shell_height = ( main_window_border * 2 )	    	+
    	    	    menu_bar_height     	    	+
    	    	    ( menu_bar_border 	* 2 )	    	+
    	    	    ( work_area_border  * 2 )	    	+
    	    	    label_height    	    	    	+
    	    	    ( label_border * 2 )    	    	+
    	    	    ( scrolled_window_border * 2 )  	+
    	    	    MAX( window->u.topic.height, 150 )	+
     	    	    ( window_border * 2 ) 	     	+
    	    	    hscroll_height  	    	    	+
    	    	    ( hscroll_border * 2 )		+
		    ( menu_bar_height * 2 ); /*extra space/close button and separator*/	
#ifdef FOO
    if (bkrplus_g_charcell_display) {
        shell_height = MAX(shell_height,bkr_topic_resources.height);
    }
#endif 
    /* 
     *  Maximize the default size of the FORMAL shell's width and height 
     *  using the resource values and also determine if we need scroll bars.  
     *  The initial shell size should not exceed the resources values.
     */

    if ( window->type == BKR_FORMAL_TOPIC )
    {
    	shell_width = MIN( shell_width, bkr_topic_resources.max_default_topic_width );
    	shell_height = MIN( shell_height, bkr_topic_resources.max_default_topic_height );

    	new_window_width = shell_width - 
    	    	    ( main_window_border * 2 ) 	    -
    	    	    ( work_area_border * 2 ) 	    -
    	    	    ( scrolled_window_border * 2 )  -
     	    	    ( window_border  * 2 ) 	    -
    	    	    vscroll_width   	     	    -
    	    	    ( vscroll_border * 2 );
    	if ( window->u.topic.width <= new_window_width )
    	    XtUnmanageChild( window->widgets[W_HSCROLL_BAR] );

    	new_window_height = shell_height -
    	    	    ( main_window_border * 2 )	    	-
    	    	    menu_bar_height    	    		-
    	    	    ( menu_bar_border 	* 2 )	    	-
    	    	    ( work_area_border  * 2 )	    	-
    	    	    label_height    	    	    	-
    	    	    ( label_border * 2 )    	    	-
    	    	    ( scrolled_window_border * 2 )  	-
     	    	    ( window_border * 2 ) 	     	-
    	    	    hscroll_height  	    	    	-
    	    	    ( hscroll_border * 2 )		-
		    ( menu_bar_height * 2 );  /*extra space/close button and separator*/
    	if ( window->u.topic.height <= new_window_height )
    	    XtUnmanageChild( window->widgets[W_VSCROLL_BAR] );
    }

    argcnt = 0;
    SET_ARG( XmNwidth,	shell_width );
    SET_ARG( XmNheight, shell_height );
    XtSetValues( window->widgets[W_MAIN_WINDOW], arglist, argcnt );

};  /* end of set_formal_topic_size */




