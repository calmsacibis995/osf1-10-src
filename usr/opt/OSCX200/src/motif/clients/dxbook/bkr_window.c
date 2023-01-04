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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_WINDOW.C*/
/* *27   26-MAR-1993 15:42:51 BALLENGER "Fix compilation problems for VAX ULTRIX."*/
/* *26   22-MAR-1993 14:38:00 BALLENGER "Fix problems with polygon hotspots."*/
/* *25   24-FEB-1993 17:47:59 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *24   26-JAN-1993 16:54:01 RAMSHAW "QAR #41 - Fix non-scrolling of pop-up topics"*/
/* *23    9-DEC-1992 21:06:11 BALLENGER "Fix problem displaying long topics."*/
/* *22   17-NOV-1992 22:48:19 BALLENGER "Distinguish active/inactive windows."*/
/* *21   21-SEP-1992 22:12:21 BALLENGER "    (1)   BALLENGER  4       6-SEP-1992 16:46:05 ""Fix hotspot selection for character*/
/*cell."""*/
/* *20    5-AUG-1992 22:15:44 BALLENGER "Account for WM borders in sizing and placing windows."*/
/* *19   30-JUL-1992 03:35:24 BALLENGER "Fix hotspot highlighting."*/
/* *18   20-JUL-1992 13:33:47 BALLENGER "Character cell support."*/
/* *17   23-JUN-1992 18:55:13 BALLENGER "Fix QAR215, don't toggle wait cursor during expose when search is active."*/
/* *16   19-JUN-1992 20:20:09 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *15    9-JUN-1992 11:56:35 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *14    4-JUN-1992 10:06:23 FITZELL "ifdef copy for basic bookreader"*/
/* *13   20-MAY-1992 17:11:03 FITZELL "an comma mysteriously disappeared ??"*/
/* *12   20-MAY-1992 10:26:44 FITZELL "fat fingered last replace"*/
/* *11   19-MAY-1992 14:06:23 FITZELL "search highlight and exposure events with highlighted text"*/
/* *10   15-APR-1992 19:35:14 BALLENGER "Make sure window is active during expose."*/
/* *9    27-MAR-1992 16:54:25 ROSE "Checks added, XClearArea should be changed to XFillRectangle, much faster"*/
/* *8    27-MAR-1992 15:27:09 FITZELL "ucxed again"*/
/* *7    27-MAR-1992 13:21:09 BALLENGER "Test for char cell support on."*/
/* *6    14-MAR-1992 14:14:38 BALLENGER "Fix problems with window and icon titles..."*/
/* *5     8-MAR-1992 19:16:26 BALLENGER "  Add topic data and text line support"*/
/* *4     3-MAR-1992 17:06:33 KARDON "UCXed"*/
/* *3     1-NOV-1991 12:59:01 BALLENGER "Reintegrate  memex support"*/
/* *2    18-SEP-1991 18:49:59 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:41:03 PARMENTER "Expose, GraphicsExpose and NoExpose events in Topic Window"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_WINDOW.C*/
#ifndef VMS
 /*
#else
#module BKR_WINDOW  "V03-0002"
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
**	Expose, GraphicsExpose and NoExpose event handling callbacks for the 
**  	drawing window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     26-Jan-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0002    DLB0001	David L Ballenger	19-Feb-1991
**                  Add support for BMD_CHUNK_RAGS_NO_FILL for PIC 
**                  graphics support.
**
**	V03-0001    JAF0001	James A. Ferguson   	1-Feb-1991
**  	    	    Fix code which performs a union of the expose events
**  	    	    in bkr_window_expose (QARs 730 & 859).
**
**--
**/


/*
 * INCLUDE FILES
 */
#include <stdio.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_window.h"      /* function prototypes for .c module */
#include "bkr_button.h"      /* Button manipulation */
#include "bkr_close.h"       /* Close routines */
#include "bkr_cursor.h"      /* Cursor manipulation routines */
#include "bkr_display.h"     /* Display routines */
#include "bkr_icon.h"        /* Icon routines */
#include "bkr_library_create.h" /* Library window creation routines */
#include "bkr_pointer.h"     /* Pointer manipulation routines */
#include "bkr_scroll.h"      /* Scrollbar manipulation routines */
#include "bkr_selection_create.h" /* Selection window creation routines */
#include "bkr_topic_create.h" /* Topic window creation routines */

#include <X11/DECWmHints.h>
#include "dxisxuiwmrunning.h"

#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>


/*
 * LOCAL ROUTINES
 */
static Dimension     
resize_button_box PROTOTYPE((Widget  	    button_box,
			     Dimension	    width,
			     Dimension	    height));

#ifdef DEBUG
void	    	    	    print_size_hints();
void	    	    	    print_window_attr();
#endif /* DEBUG */

static int
expose_topic PROTOTYPE((BKR_WINDOW   *window,
			XExposeEvent *expose_event));

static BMD_CHUNK *
hotspot_in_event PROTOTYPE((BKR_WINDOW    *window,
                            BMD_RECTANGLE *event_rect,
                            BMD_CHUNK	  *hotspot));

static Boolean	    
fixup_highlight PROTOTYPE((BKR_WINDOW    *window,
                           BKR_TEXT_LINE *line,
                           BMD_RECTANGLE *event_rect,
			   int		 x,
			   int		 y,
			   int		 width,
			   int		 height)); 

static void  	    	    
setup_drawing_gc PROTOTYPE((Widget  drawing_window_widget));

#ifdef MEMEX
static void  	    
highlight_chunk PROTOTYPE((BKR_WINDOW *window,
			   BMD_CHUNK  *chunk));
#endif

static void
take_focus_callback PROTOTYPE((Widget widget,
                               XtPointer client_data,
                               XmAnyCallbackStruct *cbs));

static Boolean
line_in_event PROTOTYPE((BKR_TEXT_LINE *line,
                         BMD_RECTANGLE *event_rect));

static void
merge_rectangles PROTOTYPE((BMD_RECTANGLE *src, BMD_RECTANGLE *dst ));

/*
 * DEFINES 
 */

/* Macro to see if a chunk is in the event rectangle
 */
#define POINT_IN_RECT(x,y,rect) \
        (((x) >= ((BMD_RECTANGLE *)rect)->left) && ((x) <= ((BMD_RECTANGLE *)rect)->right) \
         && ((y) >= ((BMD_RECTANGLE *)rect)->top) && ((y) <= ((BMD_RECTANGLE *)rect)->bottom) \
         )

#define HLINE_IN_RECT(x1,x2,y,rect) \
        ((((y) >= ((BMD_RECTANGLE *)rect)->top) \
          && ((y) <= ((BMD_RECTANGLE *)rect)->bottom) \
          ) \
         && ((((x1) >= ((BMD_RECTANGLE *)rect)->left) \
              && ((x1) <= ((BMD_RECTANGLE *)rect)->right) \
              ) \
             || (((x2) >= ((BMD_RECTANGLE *)rect)->left) \
                 && ((x2) <= ((BMD_RECTANGLE *)rect)->right) \
                 ) \
             || (((x1) < ((BMD_RECTANGLE *)rect)->left) \
                 && ((x2) > ((BMD_RECTANGLE *)rect)->right) \
                 ) \
             ) \
         )

#define VLINE_IN_RECT(x,y1,y2,rect) \
        ((((x) >= ((BMD_RECTANGLE *)rect)->left) \
          && ((x) <= ((BMD_RECTANGLE *)rect)->right) \
          ) \
         && ((((y1) >= ((BMD_RECTANGLE *)rect)->top) \
              && ((y1) <= ((BMD_RECTANGLE *)rect)->bottom) \
              ) \
             || (((y2) >= ((BMD_RECTANGLE *)rect)->top) \
                 && ((y2) <= ((BMD_RECTANGLE *)rect)->bottom) \
                 ) \
             || (((y1) < ((BMD_RECTANGLE *)rect)->top) \
                 && ((y2) > ((BMD_RECTANGLE *)rect)->bottom) \
                 ) \
             ) \
         )

#define OLD_RECT_IN_RECT(rect_a,rect_b) \
        (POINT_IN_RECT(((BMD_RECTANGLE *)rect_a)->left,((BMD_RECTANGLE *)rect_a)->top,rect_b) \
         || POINT_IN_RECT(((BMD_RECTANGLE *)rect_a)->right,((BMD_RECTANGLE *)rect_a)->top,rect_b) \
         || POINT_IN_RECT(((BMD_RECTANGLE *)rect_a)->left,((BMD_RECTANGLE *)rect_a)->bottom,rect_b) \
         || POINT_IN_RECT(((BMD_RECTANGLE *)rect_a)->right,((BMD_RECTANGLE *)rect_a)->bottom,rect_b) \
         )

#define RECT_IN_RECT(rect_a,rect_b) \
        (HLINE_IN_RECT(((BMD_RECTANGLE *)rect_a)->left,((BMD_RECTANGLE *)rect_a)->right,\
                       ((BMD_RECTANGLE *)rect_a)->top,\
                       rect_b) \
         || HLINE_IN_RECT(((BMD_RECTANGLE *)rect_a)->left,((BMD_RECTANGLE *)rect_a)->right,\
                          ((BMD_RECTANGLE *)rect_a)->bottom,\
                          rect_b) \
         || VLINE_IN_RECT(((BMD_RECTANGLE *)rect_a)->left,\
                          ((BMD_RECTANGLE *)rect_a)->top,((BMD_RECTANGLE *)rect_a)->bottom,\
                          rect_b) \
         || VLINE_IN_RECT(((BMD_RECTANGLE *)rect_a)->right,\
                          ((BMD_RECTANGLE *)rect_a)->top,((BMD_RECTANGLE *)rect_a)->bottom,\
                          rect_b) \
         )

#define RECTS_OVERLAP(rect_1,rect_2) (RECT_IN_RECT((rect_1),(rect_2)) \
                                      || RECT_IN_RECT((rect_2),(rect_1)))


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_set_names
**
** 	Set the names used in the title bar and for the icon names
**
**  FORMAL PARAMETERS:
**
**	window - pointer to the BKR_WINDOW structure
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
BKR_WINDOW *
bkr_window_set_names PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    unsigned	    	    status;
    XmString	 	    cs_name;
    long         	    cs_length;
    long         	    cs_status;

    
    if (bkrplus_g_charcell_display) {
        take_focus_callback(NULL,(XtPointer)window,NULL);
    }
    else 
    {
        cs_name = DXmCvtFCtoCS( window->title_bar_name, &cs_length, &cs_status);
        DWI18n_SetTitle( window->appl_shell_id, cs_name );
        COMPOUND_STRING_FREE( cs_name );
    }
    
    cs_name = DXmCvtFCtoCS( window->icon_name, &cs_length, &cs_status);
    DWI18n_SetIconName( window->appl_shell_id, cs_name );
    COMPOUND_STRING_FREE( cs_name );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_free_pixmaps
**
** 	Routine to free the pixmps associated with a window.
**
**  FORMAL PARAMETERS:
**
**  	window     - pointer to the BKR_WINDOW strucutre
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
bkr_window_free_pixmaps PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    if ( window->icon_pixmap ) {
        XFreePixmap( bkr_display, window->icon_pixmap );
    }
    if (window->iconify_pixmap 
        && (window->iconify_pixmap != window->icon_pixmap) 
        )
    {
        XFreePixmap( bkr_display, window->iconify_pixmap );
    }
    window->icon_pixmap = NULL;
    window->iconify_pixmap = NULL;
    window->icon_size = 0;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_set_icons
**
** 	Routine to set the pixmps associated with a window.
**
**  FORMAL PARAMETERS:
**
**  	window     - pointer to the BKR_WINDOW strucutre
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
bkr_window_set_icons PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    unsigned int    icon_size;
    char	    	*window_icon_name;
    Arg	    	arglist[5];

    /* Fetch the icon pixmap 
     */
    window_icon_name = bkr_window_get_icon_index_name( "BOOKREADER_ICON",
                                                      &icon_size );
    if ( window_icon_name == NULL )
    {
        return;
    }

    /*  If the icon sizes are different we need to free the current
     *  ones, and re-fetch new icons.
     */
    if ( ( window->icon_size != 0 )	    	/* Some icon set.   */
        && ( window->icon_size != icon_size )   /* Size changed.    */
        )
    {
        bkr_window_free_pixmaps(window);
    }
    if ( window->icon_size == 0 )
    {
        window->icon_size = icon_size;
        window->icon_pixmap = bkr_fetch_window_icon( window_icon_name );
    }
    BKR_FREE( window_icon_name );
    
    /* Fetch the iconify pixmap 
     */
    if ( DXIsXUIWMRunning( window->appl_shell_id, TRUE ) )
    {
        if ( window->icon_size == 17 )  	    /* Don't fetch icon twice */
        {
            window->iconify_pixmap = window->icon_pixmap;
        }
        else if ( window->icon_size > 17 )
        {
            if ( window->iconify_pixmap == NULL)
            {
                window->iconify_pixmap = bkr_fetch_window_icon("BOOKREADER_ICON_17X17");
            }
        }
    }    
    if ( window->icon_pixmap )
    {
        XtSetArg( arglist[0], XmNiconPixmap, window->icon_pixmap );
        XtSetValues( window->appl_shell_id, arglist, 1 );
    }
    
    /* Set the iconify pixmap for the XUI window manager 
     */
    if ( window->iconify_pixmap ) 
    {
        bkr_window_set_iconify_pixmap(window->appl_shell_id, 
                                      window->iconify_pixmap 
                                      );
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_initialize_icons
**
** 	Callback routine for initializing the icons.
**
**  FORMAL PARAMETERS:
**
**  	appl_shell - id of the shell widget.
**	shell	   - pointer to data shell.
**  	event	   - pointer to the X Event.
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
bkr_window_initialize_icons PARAM_NAMES((appl_shell,user_data,event,continue_to_dispatch))
    Widget     appl_shell PARAM_SEP
    XtPointer  user_data PARAM_SEP
    XEvent     *event PARAM_SEP
    Boolean    *continue_to_dispatch PARAM_END

{
    XIconSize       *size_list;
    int	    	    num_sizes;
    Window  	    root_window;
    XReparentEvent  *reparent = (XReparentEvent *) &event->xreparent;
    BKR_WINDOW      *window = (BKR_WINDOW *)user_data;

    if ( event->type != ReparentNotify )
    	return;

    root_window = XDefaultRootWindow( bkr_display );

    /* Ignore reparents back to the root window.
     */
    if ( reparent->parent == root_window )
    	return;

    if ( ! XGetIconSizes( bkr_display, root_window, &size_list, &num_sizes ) )
        return;
    else
    {
    	XFree( size_list );
        bkr_window_set_icons(window);
    }
};  /* bkr_window_initialize_icons */
	    	    	    


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_client_message
**
** 	Callback routine which handles dispatching of X Client Messages.
**
**  FORMAL PARAMETERS:
**
**	appl_shell - id of the application shell widget which received the message.
**	window	   - pointer to bookreader's information about the window
**	event	   - pointer to the X event.
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
bkr_window_client_message PARAM_NAMES((appl_shell,user_data,event,continue_to_dispatch))
    Widget  	appl_shell PARAM_SEP
    XtPointer   user_data PARAM_SEP
    XEvent  	*event PARAM_SEP
    Boolean     *continue_to_dispatch PARAM_END
{
    XClientMessageEvent	    *cmevent = (XClientMessageEvent *)event;
    BKR_WINDOW	*window = (BKR_WINDOW *)user_data;

    if ( event->type != ClientMessage )
    	return;

    if ( cmevent->message_type == WM_PROTOCOLS_ATOM )
    {
    	if ( cmevent->data.l[0] == WM_DELETE_WINDOW_ATOM )
    	{
    	    if ( window == NULL )
    	    {
                /*  We pass NULL for the user data for the file selection dialog.
    	    	 *  Turn off the inactive cursor just in case the user closed 
    	     	 *  the file selection widget by using the window menu instead 
    	     	 *  of the "cancel" button.
    	     	 */
    	    	if ( bkr_library != NULL )
    	    	    bkr_cursor_display_inactive( bkr_library->appl_shell_id, OFF );
    	    }
    	    else if ( window->type == BKR_LIBRARY )
    	    {
    	    	bkr_close_quit_callback();
    	    }
    	    else if ( window->type == BKR_SELECTION )
    	    {
    	    	XmPushButtonCallbackStruct reason;

    	    	bkr_close_selection_window( window->widgets[W_CLOSE_BOOK_ENTRY], 
    	    	    	    	    	    window, &reason );
    	    }
    	    else if ( ( window->type == BKR_STANDARD_TOPIC )
    	    	      || ( window->type == BKR_FORMAL_TOPIC ) )
    	    {
    	    	XmPushButtonCallbackStruct reason;

    	    	bkr_close_topic_window( window->widgets[W_CLOSE_TOPIC_ENTRY], 
    	    	    	    	    	(caddr_t)window, &reason );
    	    }
    	}
    }

#ifdef DEBUG
    if ( cmevent->message_type == WM_PROTOCOLS_ATOM )
    {
    	printf( " --- Unrecognized WM_PROTOCOLS: %s \n",
    	    	XGetAtomName( bkr_display, cmevent->data.l[0] ) );
    }
    else
    {
    	printf( " --- Unrecognized ClientMessage: %s\n", 
	    	    XGetAtomName( bkr_display, cmevent->message_type ) );
    }
#endif /* DEBUG */

};  /* end of bkr_window_client_message */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_get_icon_index_name
**
** 	Determines the best size for the icon to be set used when setting 
**  	the XmNiconPixmap resource and builds the icon index name given
**  	the root part of the name.  The root part of the name is like
**  	"BOOKREADER_ICON" and this routine will append the string "wwXhh",
**  	where "ww" is the width of the icon and "hh" is the height.
**
**  FORMAL PARAMETERS:
**
**	root_index_name - pointer to the root icon index name
**  	icon_size_rtn   - address to return size of icon fetched.
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
**	Returns:    A pointer to a complete icon index name.
**  	    	    NULL if no index name could be created.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
char *
bkr_window_get_icon_index_name PARAM_NAMES((root_index_name,icon_size_rtn))
    char    	    *root_index_name PARAM_SEP
    unsigned int    *icon_size_rtn PARAM_END

{
    static char *shell_icon_sizes[] = { "75", "50", "32", "17" };
    XIconSize	*size_list;
    int	    	num_sizes;
    int	    	cursize;
    int	    	i;
    int	    	our_num_sizes;
    char    	*icon_index = NULL;
    int	    	icon_size;
    char    	*icon_size_ptr;
    Boolean 	found_icon_size = FALSE;

    *icon_size_rtn = 0;	    /* Initial value */

    if ( ! XGetIconSizes( bkr_display, 
    	    	    	  XDefaultRootWindow( bkr_display ),
    	    	    	  &size_list, 
    	    	    	  &num_sizes ) )
    	return ( NULL );

    /* Find the largest icon */

    cursize = 0;
    for ( i = 1; i < num_sizes; i++ )
    {
    	if ( ( size_list[i].max_width >= size_list[cursize].max_width )
    	      && ( size_list[i].max_height >= size_list[cursize].max_height ) )
    	    cursize = i;
    }
    if ( ( size_list[cursize].max_width <= 0 ) 
    	 || ( size_list[cursize].max_height <= 0 ) )
    {
    	XFree( size_list );
    	return ( NULL );
    }

    /* Find our largest icon */

    our_num_sizes = sizeof( shell_icon_sizes ) / sizeof( shell_icon_sizes[0] );
    for ( i = 0; i < our_num_sizes; i++ )
    {
    	icon_size = atoi( shell_icon_sizes[i] );
    	if ( ( icon_size <= size_list[cursize].max_width )
    	      && ( icon_size <= size_list[cursize].max_height ) )
    	{
    	    icon_size_ptr = shell_icon_sizes[i];
    	    found_icon_size = TRUE;
    	    break;
    	}
    }
    XFree( size_list );

    /*  Build the icon index name
     *  
     *      format: root_index_name + "_" + icon_size_ptr + "X" + icon_size_ptr
      */

    if ( found_icon_size )
    {
    	char	*strptr;

    	icon_index = (char *) BKR_MALLOC( strlen( root_index_name ) 	+
    	    	    	    	    	sizeof( "_" )	    	    	+
    	    	    	    	        ( 2 * sizeof( icon_size_ptr ) ) +
    	    	    	    	    	1 );    /* for \0 char */
    	strptr = icon_index;
    	strcpy( strptr, root_index_name );
    	strptr = strptr + strlen( root_index_name );
    	strcpy( strptr, "_" );
    	strptr++;
    	strcpy( strptr, icon_size_ptr );
    	strptr = strptr + strlen( icon_size_ptr );
    	strcpy( strptr, "X" );
    	strptr++;
    	strcpy( strptr, icon_size_ptr );

    	*icon_size_rtn = (unsigned int) icon_size;
    }

    return( icon_index );

};  /* end of bkr_window_get_icon_index_name */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_expose
**
** 	Expose callback routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	shell	    	- pointer to the shell data structure
**	callback_data	- pointer to callback data.  If NULL, a dummy event 
**  	    	    	    will be created using the width and height of the
**  	    	    	    window widget passed.
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
bkr_window_expose PARAM_NAMES((widget,window,callback_data))
    Widget		    	widget PARAM_SEP
    BKR_WINDOW                  *window PARAM_SEP
    XmDrawingAreaCallbackStruct *callback_data PARAM_END

{
    XExposeEvent    	    *event;
    XExposeEvent    	    dummy_event;
    XExposeEvent    	    merged_event;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;
    Dimension	    	    window_width;
    Dimension	    	    window_height;

    /* Make sure window is active
     */
    if ((window == NULL) 
        || (window->active == FALSE)
        || (window->shell == NULL)
        || (window->appl_shell_id == NULL)
        )
    {
        return;
    }

    /* Create the dummy event if we need to */

    if ( callback_data == NULL )
    {
    	argcnt = 0;
    	SET_ARG( XmNwidth, &window_width );
    	SET_ARG( XmNheight, &window_height );
    	XtGetValues( widget, arglist, argcnt );

    	event = &dummy_event;
    	event->x = 0;
    	event->y = 0;
    	event->width = (int) window_width;
    	event->height = (int) window_height;
#ifdef DEBUG_EXPOSURES
    	printf( "Dummy Expose created @ %4d,%4d  %4d X%4d win: %8x\n", 
    	    	event->x, event->y, event->width, event->height, XtWindow(widget) );
#endif
    }
    else
    {
    	XExposeEvent	*next_event;
    	XEvent	    	xevent;
    	int 	    	x, y;
    	
    	event = (XExposeEvent *) &callback_data->event->xexpose;
#ifdef DEBUG_EXPOSURES
    	printf( "Expose @ %4d,%4d  %4d X%4d win: %8x\n", 
    	    	event->x, event->y, event->width, event->height, XtWindow(widget) );
#endif
    	/*  Merge the exposures for the window into one event 
    	 *  by performing a union.
    	 */
    	if ( event->count > 0 )
    	{
    	    COPY_XEXPOSE_EVENT( merged_event, event );
    	    while ( XCheckWindowEvent( bkr_display, XtWindow( widget ), 
    	    	    	    	    	    ExposureMask, &xevent ) )
    	    {
    	    	next_event = (XExposeEvent *) &xevent.xexpose;
#ifdef DEBUG_EXPOSURES
    	    	printf( "  Next Expose @ %4d,%4d  %4d X%4d win: %8x\n", 
    	    	    next_event->x, next_event->y, next_event->width, 
    	    	    next_event->height, XtWindow(widget) );
#endif
    	    	x = MIN( merged_event.x, next_event->x );
    	    	y = MIN( merged_event.y, next_event->y );
    	    	merged_event.width = MAX( merged_event.x + merged_event.width,
    	    	    	    	    	  next_event->x + next_event->width ) - x;
    	    	merged_event.height = MAX( merged_event.y + merged_event.height,
    	    	    	    	    	   next_event->y + next_event->height ) - y;
    	    	merged_event.x = x;
    	    	merged_event.y = y;
    	    }
    	    event = &merged_event;
#ifdef DEBUG_EXPOSURES
    	    printf( "--Merged Expose @ %4d,%4d  %4d X%4d win: %8x\n", 
    	    	    event->x, event->y, event->width, event->height, XtWindow(widget) );
#endif
    	}   /* end if */
    } 	  /* end else */

    /*
     *  HACK ALERT:   Clear the entire event area first just in case 
     *                some entry or hot spot was reverse video.
     */

    XClearArea(
    	bkr_display,
    	XtWindow( widget ), 	    	/* window   	 */
    	event->x, event->y, 	    	/* x, y     	 */
    	event->width, event->height,	/* width, height */
    	FALSE );    	    	    	/* no exposures */

    if ( window->u.topic.chunk_list == NULL ||
	 window->shell == NULL ||
	 window->shell->book == NULL) {
        return;
    }
    /* Expose the Topic */
    
    expose_topic( window, event );

};  /* end of bkr_window_expose */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_graphics_expose
**
** 	GraphicExpose and NoExpose action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xevent	    	- X event associated with GraphicExpose/NoExpose event
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
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
bkr_window_graphics_expose PARAM_NAMES((widget,xevent,params,num_params))
    Widget		    widget PARAM_SEP
    XEvent  	    	    *xevent PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW	            *window;
    XGraphicsExposeEvent    *event = &xevent->xgraphicsexpose;
    Arg	    	    	    arglist[5];
    int	    	    	    argcnt;
    Dimension	    	    window_width;
    Dimension	    	    window_height;
    int			    rags;

    argcnt = 0;
    SET_ARG( XmNwidth, &window_width );
    SET_ARG( XmNheight, &window_height );
    XtGetValues( widget, arglist, argcnt );

    /* Only interested in GraphicsExpose and NoExpose events */

    if ( ( xevent->xany.type != GraphicsExpose ) 
    	 && ( xevent->xany.type != NoExpose ) )
    	return;

    window = bkr_window_find( widget );
    /* Make sure window is active
     */
    if ((window == NULL) 
        || (window->active == FALSE)
        || (window->shell == NULL)
        || (window->appl_shell_id == NULL)
        )
    {
        return;
    }

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;
    	    break;

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    if ( window->u.topic.chunk_list == NULL )
	    	return;

    	    /* NoExpose: Adjust the display only if it has not be moved already */

    	    if ( xevent->xany.type == NoExpose )
    	    {
	    	if ( ( window->u.topic.x == window->u.topic.internal_x ) 
    	    	     && ( window->u.topic.y == window->u.topic.internal_y ) )
	    	    window->u.topic.grop_pending = FALSE;
	    	else
	    	    bkr_scroll_adjust_display( window );
	    	return;
    	    }

#ifdef DEBUG_EXPOSURES
    	printf( "GrapicsExpose @ %4d,%4d  %4d X%4d win: %8x \n", 
    	    	event->x, event->y, event->width, event->height, XtWindow(widget) );
#endif

    	    /* Expose the Topic */

    	    rags = expose_topic( window, (XExposeEvent *)event );

	    /* Generate a NoExpose event when expose_topic displayed rags
	     * data. The Rags processing eats the NoExpose event previously
	     * queued and prevents the next callback.
	     * window->u.topic.grop_pending is never reset causing scroll
	     * to stop functioning.
	     */
	    if (rags)
		XCopyArea( bkr_display,
			   XtWindow( window->widgets[W_WINDOW] ), 	 /* source */
			   XtWindow( window->widgets[W_WINDOW] ), 	 /* destintation */
			   bkr_text_gc,
			   0, 0, 0, 0, 0, 0 );

    	    break;  /* end case BKR_STANDARD_TOPIC and BKR_FORMAL_TOPIC */

    }	/* end switch */

};  /* end of bkr_window_graphics_expose */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_initialize_gc
**
** 	Initialize the Graphic Contexts (GC's) for a particular Selection 
**  	window.  The GC's are shared, so all this means is to copy the local
**  	GC id into the fields within the Selection window data structure.
**
**  FORMAL PARAMETERS:
**
**  	shell	    - pointer to the Shell to be initialized.
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
bkr_window_initialize_gc PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{

    if ( bkr_text_gc != NULL )
    	return;

    switch ( window->type )
    {
    	case BKR_SELECTION : {
    	    return;
    	    break;
        }
    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC : {
            setup_drawing_gc( window->widgets[W_WINDOW] );
    	    break;
        }
    }


};  /* end of bkr_window_initialize_gc */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_modify_attributes
**
** 	Modifies the do_not_propagate field of a window so that all events
**  	are propagated to its parent window.  
**
**  	NOTE:  This was not need for the XUI toolkit kit because the DwtWindow
**  	       widget propagated all events to its parent unlike Motif's
**  	       XmDrawingArea which does not propagate:
**
**  	    	    KeyPressMask, KeyReleaseMask, ButtonPressMask
**  	    	    ButtonReleaseMask, PointerMotionMask
**
**  	    The change is needed so that the DrawingArea's translation table
**  	    works correctly, especially for Leave window and mouse button 3 events.
**
**  FORMAL PARAMETERS:
**
**	window - id of the window to be modified.
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
bkr_window_modify_attributes( window )
    Window  	window;
{
    unsigned long   	 attr_mask;
    XSetWindowAttributes attributes;

    attr_mask = CWDontPropagate;
    attributes.do_not_propagate_mask = 0;

    XChangeWindowAttributes( bkr_display, window, attr_mask, &attributes );

};  /* end of bkr_window_modify_attributes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_resize_work_area
**
** 	Resize action routine called when the work area widget
**  	needs to be reconfigured.
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**  	event	    	- X event associated with the Configure event
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
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
bkr_window_resize_work_area PARAM_NAMES((widget,event,params,num_params))
    Widget	    widget PARAM_SEP
    XConfigureEvent *event PARAM_SEP
    String  	    *params PARAM_SEP	    /* not used */
    Cardinal	    *num_params PARAM_END   /* not used */

{
    Arg		    	arglist[20];
    int 	    	argcnt = 0;
    Dimension	    	window_width;
    Dimension	    	window_height;
    BKR_WINDOW	        *window;
    int	    	    	*x_ptr;
    int	    	    	*y_ptr;
    int	    	    	*internal_x_ptr;
    int	    	    	*internal_y_ptr;
    int	    	    	data_width;
    int	    	    	data_height;
    Boolean 	    	check_data_position = FALSE;
    Boolean 	    	adjust_scrollbars = FALSE;
    Dimension		hscroll_height;
    Dimension		vscroll_width;

    if ( event->type != ConfigureNotify )
        return;

    window = bkr_window_find( widget);
    if (window == NULL) {
        return;
    }
    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;
    	    break;

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :

    	    /* Resize the button box */

    	    if ( window->type == BKR_STANDARD_TOPIC )
    	    	resize_button_box( window->widgets[W_BUTTON_BOX],
    	    	    event->width, event->height );

    	    x_ptr = (int *) &window->u.topic.x;
    	    y_ptr = (int *) &window->u.topic.y;
    	    internal_x_ptr = (int *) &window->u.topic.internal_x;
    	    internal_y_ptr = (int *) &window->u.topic.internal_y;
    	    data_width = window->u.topic.width;
    	    data_height = window->u.topic.height;
    	    adjust_scrollbars = TRUE;
    	    check_data_position = TRUE;
    	    break;
    }

    /* Get the dimensions of the scrolled window and subtract off 
     * the horiz and vert scroll dimensions if the scroll bars are managed.
     */

    argcnt = 0;
    SET_ARG( XmNwidth,  &window_width );
    SET_ARG( XmNheight, &window_height );
    XtGetValues( window->widgets[W_SCROLLED_WINDOW], arglist, argcnt);

    if ( XtIsManaged( window->widgets[W_HSCROLL_BAR] ) )
    {
        argcnt = 0;
        SET_ARG( XmNheight, &hscroll_height );
        XtGetValues( window->widgets[W_HSCROLL_BAR], arglist, argcnt );
        window_height -= hscroll_height;
    }
	
    if ( XtIsManaged( window->widgets[W_VSCROLL_BAR] ) )
    {
        argcnt = 0;
        SET_ARG( XmNwidth, &vscroll_width );
        XtGetValues( window->widgets[W_VSCROLL_BAR], arglist, argcnt );
        window_width -= vscroll_width;
    }

    if (bkrplus_g_charcell_display)
    {
        int modulous;

        argcnt = 0;

        modulous = window_height % bkr_default_line_height;
        if (modulous) {
            window_height -= modulous;
            SET_ARG( XmNheight, &window_height );
        }
        modulous = window_width % bkr_default_space_width;
        if (modulous) {
            window_width -= modulous;
            SET_ARG( XmNwidth, &window_width );
        }
        if (argcnt) {
            XtSetValues( window->widgets[W_SCROLLED_WINDOW], arglist, argcnt);
        }
    }

    /* Manage/Unmanage the scroll bars if the FORMAL topic is fully visible */

    if ( window->type == BKR_FORMAL_TOPIC )
    {
    	if ( window_width >= (Dimension) data_width )
    	{
    	    if ( XtIsManaged( window->widgets[W_HSCROLL_BAR] ) )
    	    	XtUnmanageChild( window->widgets[W_HSCROLL_BAR] );
    	}
    	else
    	{
    	    if ( ! XtIsManaged( window->widgets[W_HSCROLL_BAR] ) )
    	    	XtManageChild( window->widgets[W_HSCROLL_BAR] );
    	}

    	if ( window_height >= (Dimension) data_height )
    	{
    	    if ( XtIsManaged( window->widgets[W_VSCROLL_BAR] ) )
    	    	XtUnmanageChild( window->widgets[W_VSCROLL_BAR] );
    	}
    	else
    	{
    	    if ( ! XtIsManaged( window->widgets[W_VSCROLL_BAR] ) )
    	    	XtManageChild( window->widgets[W_VSCROLL_BAR] );
    	}
    }

    /*
     *  Reposition the contents of the window if it was shifted down or
     *  or shift to the right and the window is now large enough to 
     *  display more data.
     */
    if ( check_data_position )
    {
	if  ( ( *x_ptr + data_width ) < (int) window_width )
	{
            int new_offset;

            new_offset = *x_ptr + data_width - (int) window_width;
	    *x_ptr = MIN( 0, ( *x_ptr - new_offset ) );
	    *internal_x_ptr = *x_ptr; 
	    adjust_scrollbars = TRUE;
	}

	if ( ( *y_ptr + data_height ) < (int) window_height )
	{
            int new_offset;

	    new_offset = *y_ptr + data_height - (int) window_height;
	    *y_ptr = MIN( 0, ( *y_ptr - new_offset ) );
	    *internal_y_ptr = *y_ptr;
	    adjust_scrollbars = TRUE;
	}

        /* Clear the area, and let it generate an expose event.
         */
        XClearArea(bkr_display,
                   XtWindow(window->widgets[W_WINDOW]),
                   0,0,0,0,
                   TRUE);

    }	    /* end if check_data_position */

    /* Adjust the scroll bars */

    if ( adjust_scrollbars )
    	bkr_scroll_adjust_scrollbars( window, (int) *x_ptr, 
    	    (int) *y_ptr, data_width, data_height );

};  /* end of bkr_window_resize_work_area */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_set_iconify_pixmap
**
** 	Sets the iconify pixmap when the XUI window manager is running.
**  	This routine exists only for compatibility.
**
**  FORMAL PARAMETERS:
**
**	widget	- id of the shell widget to set the icon pixmap.
**  	pixmap  - id of pixmap to set on the shell.
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
**	This routine creates and updates the Atom "DEC_WM_HINTS" on the
**  	window of the shell widget passed in.
**
**--
**/
void
bkr_window_set_iconify_pixmap PARAM_NAMES((widget,pixmap))
    Widget  widget PARAM_SEP
    Pixmap  pixmap PARAM_END

{


#define WmNumDECWmHintsElements (sizeof(DECWmHintsRec)/4)

    DECWmHintsRec prop;
    Atom    	  decwmhints;

    prop.value_mask 	    = DECWmIconifyPixmapMask;
    prop.icon_box_x 	    = -1;
    prop.icon_box_y 	    = -1;
    prop.tiled 	    	    = FALSE;
    prop.sticky     	    = FALSE;
    prop.no_iconify_button  = FALSE;
    prop.no_lower_button    = FALSE;
    prop.no_resize_button   = FALSE;
    prop.iconify_pixmap     = pixmap;

    decwmhints = XmInternAtom( bkr_display, "DEC_WM_HINTS", FALSE );
    XChangeProperty( bkr_display, XtWindow( widget ), decwmhints, 
    	    	     decwmhints, 32, PropModeReplace, 
    	    	     (unsigned char *) &prop, WmNumDECWmHintsElements );

};  /* end of bkr_window_set_iconify_pixmap */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_setup
**
** 	Sets up a new window so that its totally on the screen, iconifies
**  	with its parent and raises it to the top of the window stack.
**
**  FORMAL PARAMETERS:
**
**	window	     - pointer to the main window widget.
**  	parent	     - pointer to parent shell.
**  	raise_window - Boolean: whether to raise the window
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
bkr_window_setup PARAM_NAMES((widget,parent,raise_window))
    Widget	    widget PARAM_SEP
    Widget  	    parent PARAM_SEP
    Boolean 	    raise_window PARAM_END

{
    Widget	    shell;
    Widget          wm_shell;
    Position	    x;
    Position	    y;
    Dimension	    width;
    Dimension	    height;
    int		    diff;
    Arg		    arglist[20];
    int		    argcnt;
    Dimension       wm_width = 0;
    Dimension       wm_height = 0;

    /* Get the widget id of the shell around the main window */

    shell = XtParent( widget );

    wm_shell = shell;
    while (wm_shell && !XtIsWMShell(wm_shell)) 
    {
        wm_shell = XtParent(wm_shell);
    }

    /* Make sure the window iconifies with its parent */

    if ( ( parent != NULL ) && XtIsRealized( shell ) )
    {
    	XSetTransientForHint( bkr_display, XtWindow( shell ), 
    	    	    	    	XtWindow( parent ) );
    	XtManageChild( widget );
    }

    /* Get the shell's position and dimensions */

    argcnt = 0;
    SET_ARG( XmNx,  	&x );
    SET_ARG( XmNy,  	&y );
    SET_ARG( XmNwidth,	&width );
    SET_ARG( XmNheight,	&height );
    XtGetValues( shell, arglist, argcnt );

    if (wm_shell) 
    {
        Widget wm_parent = XtParent(wm_shell);
        Widget wm_child = wm_shell;
        
        while (wm_parent && XtIsWMShell(wm_parent)) {
            wm_child = wm_shell;
            wm_shell = wm_parent;
            wm_parent = XtParent(wm_shell);
        }

        argcnt = 0;
        SET_ARG( XmNwidth, &wm_width );
        SET_ARG( XmNheight, &wm_height );
        XtGetValues( wm_child, arglist, argcnt );
    }
    else 
    {
        wm_width = width;
        wm_height = height;
    }

    /* Make sure the window fits vertically 
     */
    argcnt = 0;
    
    diff = (int) wm_height + (int) y - bkr_display_height; /* portion off bottom */
    if ( diff > 0 )
    {
        if ( diff <= (int) y )    /* Height ok, adjust Y position */
        {
            SET_ARG( XmNy, (Position)( (int) y - diff ) );	
        }
        else	    /* Window too tall, adjust both */
        {
            int	    height_adjust;
            
            height_adjust = diff - (int) y; 	/* portion too tall */
            SET_ARG( XmNy, 0 );
            SET_ARG( XmNheight, (Dimension)( (int) height - height_adjust ) );
        }
    }
        
    /* Make sure the window fits horizontally */
    
    diff = (int) wm_width + (int) x - bkr_display_width;   /* portion off side */
    if ( diff > 0 )
    {
        if ( diff <= (int) x )    /* Width ok, adjust X position */
        {
            SET_ARG( XmNx, (Position)( (int) x - diff ) );
        }
        else	    /* Window too wide, adjust both */
        {
            int	    width_adjust;
            
            width_adjust = diff - (int) x;  	/* portion too wide */
            SET_ARG( XmNx, 0 );
            SET_ARG( XmNwidth, (Dimension)( (int) width - width_adjust ) );
        }
    }

    /* Update the shell */

    if ( argcnt > 0 )
    	XtSetValues( shell, arglist, argcnt );

    /* Raise the window to the top if necessary */

    if ( ( XtWindow( shell ) != 0 ) && ( raise_window ) )
    {
    	RAISE_WINDOW( shell );
    }

};  /* end of bkr_window_setup */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_find
**
** 	Sets up a new window so that its totally on the screen, iconifies
**  	with its parent and raises it to the top of the window stack.
**
**  FORMAL PARAMETERS:
**
**	widget_id    - widget_id of the window we're looking for
**
**  IMPLICIT INPUTS:
**
**	bkr_all_shells - list of all shells containing windows
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns a pointer to the BKR_WINDOW structure for the window
**      or NULL if not found
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_WINDOW *
bkr_window_find PARAM_NAMES((widget_id))
    Widget  widget_id PARAM_END

{
    Widget  	    	   app_shell_id = (Widget) (widget_id);    	     
    BKR_SHELL       	   *shell;  	    	    	    	     	     

    while ( ! XtIsSubclass( app_shell_id, topLevelShellWidgetClass ) ) {
    	app_shell_id = (Widget) XtParent( app_shell_id );
    }

    shell = bkr_all_shells;
    while ( shell )  {

        
    	if ( shell->selection && ( shell->selection->appl_shell_id ==  app_shell_id ) ) {
    	    return shell->selection;
    	} else if (shell->default_topic && (shell->default_topic->appl_shell_id==app_shell_id)) {
            return shell->default_topic;
        } else {
            BKR_WINDOW *window = shell->other_topics;
            while (window) {
                if (window->appl_shell_id == app_shell_id) {
                    return window;
                }
                window = window->u.topic.sibling;
            }
        }
        shell = shell->all_shells;
    }
    return NULL;

};  /* end of bkr_window_find */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_draw_cc_text
**
** 	Draws the text in the topic window in character cell mode.
**
**  FORMAL PARAMETERS:
**
**	window - pointer to the BKR_WINDOW structure for the topic window
**      
**      gc     - the graphic context for drawing the text
**
**      region - clip region
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
**      or NULL if not found
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_window_draw_cc_text PARAM_NAMES((window,gc,event_rect))
    BKR_WINDOW    *window PARAM_SEP
    GC            gc PARAM_SEP
    BMD_RECTANGLE *event_rect PARAM_END
{
    BKR_TEXT_LINE *line = window->u.topic.data->text_lines;

    while (line)
    {
        if ((line->n_bytes > 1) && line_in_event(line,event_rect))
        {
            XDrawImageString(bkr_display,
                             XtWindow(window->widgets[W_WINDOW]),
                             gc,
                             line->x + window->u.topic.x,
                             line->baseline + window->u.topic.y,
                             line->chars,
                             (line->n_bytes - 1)
                             );
        }
        line = line->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	resize_button_box
**
** 	Resizes the button box which will force any push buttons to wrap.
**
**  FORMAL PARAMETERS:
**
**	button_box  	- id of the button box widget
**	width	    	- new width for the button box
**	height	    	- new height for the button box
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
static Dimension 
resize_button_box PARAM_NAMES((button_box,width,height))
    Widget  	    button_box PARAM_SEP
    Dimension	    width PARAM_SEP
    Dimension	    height PARAM_END

{
    Arg		    	arglist[5];
    int     	    	argcnt = 0;
    Dimension	    	bbox_height;
    XtWidgetGeometry	intended,		/* intended size structure */
			reply;			/* reply size structure	   */

    /*
     * Change the height of the button box iff a change in the width
     * affects the height; determined by calling XtQueryGeometry.
     */

    intended.request_mode = CWWidth;
    intended.width = width;
    switch ( XtQueryGeometry( button_box, &intended, &reply ) )
    {
	case XtGeometryAlmost:		/* he wants to compromise */
	    /*
	     * use the suggested height, if the reply isn't equal to the 
	     * current button_box height.
	     */

    	    argcnt = 0;
    	    SET_ARG( XmNheight, &bbox_height );
    	    XtGetValues( button_box, arglist, argcnt );

	    if ( ( reply.request_mode & CWHeight) &&
		( bbox_height != reply.height) )
	    {
		argcnt = 0;
		SET_ARG( XmNheight, reply.height );
		XtSetValues( button_box, arglist, argcnt );
	    }
	    break;

	case XtGeometryYes :		/* he agrees, no problem  */
	case XtGeometryNo :		/* current height	  */
	    break;
    }	/* end switch */

    return( (Dimension) reply.height );

};  /* end of resize_button_box */


#ifdef DEBUG
void
print_size_hints( hints )
    XSizeHints	*hints;
{
    printf( "       flags = %X \n", hints->flags );
    printf( "           x = %d,           y = %d \n", hints->x, hints-> y );
    printf( "       width = %d,      height = %d \n", hints->width, hints->height );
    printf( "   min_width = %d,  min_height = %d \n", hints->min_width, hints->min_height );
    printf( "   max_width = %d,  max_height = %d \n", hints->max_width, hints->max_height );
    printf( "   width_inc = %d,  height_inc = %d \n", hints->width, hints->height_inc );
    printf( "  base_width = %d, base_height = %d \n", hints->base_width, hints->base_height );
    printf( " win_gravity = %d \n", hints->win_gravity );
};
void
print_window_attr( window, title )
    Window  window;
    char    *title;
{
    XWindowAttributes attr;

    if ( ! XGetWindowAttributes( bkr_display, window, &attr ) )
    	return;

    printf( " ----  %s XWindowAttributes ---- \n", title );
    printf( "                x = %d \n", attr.x );
    printf( "                y = %d \n", attr.y );
    printf( "            width = %d \n", attr.width );
    printf( "           height = %d \n", attr.height );
    printf( "  all_event_masks = %X \n", attr.all_event_masks );
    printf( "  your_event_mask = %X \n", attr.your_event_mask );
};

static void
print_rect PARAM_NAMES((name,rect))
    char *name PARAM_SEP
    BMD_RECTANGLE *rect PARAM_END
{
    fprintf(stderr,
            "%s top = %3d, bottom = %3d, height = %3d, left = %3d, right = %3d, width = %3d\n",
            name,rect->top,rect->bottom,rect->height,rect->left,rect->right,rect->width);
}
#endif /* DEBUG */

static Boolean
line_in_event PARAM_NAMES((line,event_rect))
    BKR_TEXT_LINE *line PARAM_SEP
    BMD_RECTANGLE *event_rect PARAM_END
{
    BMD_RECTANGLE line_rect;
    
    line_rect.top = line->y;
    line_rect.bottom = line_rect.top + line->height;
    line_rect.height = line->height;
    line_rect.left = line->x;
    line_rect.right = line_rect.left + line->width;
    line_rect.width = line->width;

    return (RECTS_OVERLAP(&line_rect,event_rect));
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	expose_topic
**
** 	Determines the type of data to be displayed and then
**  	calls the appropriate topid display routine.
**
**  FORMAL PARAMETERS:
**
**	topic	     - pointer to the Topic for display.
**  	expose_event - the Expose event for the drawing window.
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
**	TRUE if rags data was displayed
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static int
expose_topic PARAM_NAMES((window,event))
    BKR_WINDOW	 *window PARAM_SEP
    XExposeEvent *event PARAM_END

{
    BMD_CHUNK 	    *chunk;
    Widget  	    drawing_window = window->widgets[W_WINDOW];
    BMD_CHUNK	    *selected_chunk;
    BMD_CHUNK	    *outlined_chunk;
#ifdef USE_TEXT_LINES
    BKR_TEXT_LINE      *line;
#endif 
    Boolean	    highlight_exposed = FALSE;
    int		    rags = FALSE;
    BMD_RECTANGLE   event_rect;

    /* This could take a while...
     */
#ifdef SEARCH
    /* If search is active, it has the wait cursor on.
     */
    if ( ! bkrplus_g_search_in_progress)
    {
        bkr_cursor_display_wait( ON );
    }
#else
    bkr_cursor_display_wait( ON );
#endif 

    /* Copy and adjust the event rectangle relative to the window origin.
     * This simplifies comparing the event rect/region to the topic 
     * elements, i.e. we don't have to continually adjust them realtive to
     * the window offsets.  Of course we occasionaly have to readjust
     * the offsets of the event region, but that happens less than checking
     * to see if the topic elements fall in the region.
     */
    event_rect.top    = event->y - window->u.topic.y;
    event_rect.bottom = event_rect.top + event->height;
    event_rect.height = event->height;
    event_rect.left   = event->x - window->u.topic.x;
    event_rect.right  = event_rect.left + event->width;
    event_rect.width  = event->width;

    /* 
     *  We must special case selected and outlined hot spots, otherwise if 
     *  we re-expose the text of a selected hot spot we will be drawing 
     *  foreground color on foreground color or only part of the outline box.
     */
    selected_chunk = hotspot_in_event(window,&event_rect,
                                      window->u.topic.selected_chunk);
    outlined_chunk = hotspot_in_event(window,&event_rect,
                                      window->u.topic.outlined_chunk);

#ifdef COPY
    /* have to check for highlighted areas now */
    if(bkr_copy_ctx.active && bkr_copy_ctx.enabled)
    {
	line = bkr_copy_ctx.startline;

	if( line == bkr_copy_ctx.endline) 
        {
             int x;

             x = (bkr_copy_ctx.x_end < bkr_copy_ctx.x_start) 
                 ? bkr_copy_ctx.x_end : bkr_copy_ctx.x_start;
	    
	    if (fixup_highlight(window, line, &event_rect,
                                x, line->y, bkr_copy_ctx.start_width, line->height)
                )
            {
		line->exposed = highlight_exposed = TRUE;
            }
        }
	else 
        {
            BKR_TEXT_LINE *templine;
            int	i, width;

	    if(line->y > bkr_copy_ctx.endline->y)
            {
                templine = bkr_copy_ctx.startline;
                bkr_copy_ctx.startline = bkr_copy_ctx.endline;
                bkr_copy_ctx.endline = templine;
                i = bkr_copy_ctx.x_start;
                bkr_copy_ctx.x_start = bkr_copy_ctx.x_end;
                bkr_copy_ctx.x_end = i;
                line = bkr_copy_ctx.startline;
            }

	    /* starting at x_pos clear to the end of line
             */
	    width = line->width - bkr_copy_ctx.x_start;
	    
            if (fixup_highlight(window, line, &event_rect,
                                bkr_copy_ctx.x_start,
                                line->y,
                                width,
                                line->height)
                )
            {
		line->exposed = highlight_exposed = TRUE;
            }

	    templine = line->next;

	    while ( templine != bkr_copy_ctx.endline )
            {
		if (fixup_highlight(window, templine, 
                                    &event_rect,
                                    templine->x,
                                    templine->y,
                                    templine->width,
                                    templine->height)
                    )
                {
                    templine->exposed = highlight_exposed = TRUE;
                }
				
		if(templine->next) 
                {
                    templine = templine->next;
                }
		else
		{
                    break;
                }
            } /* end while(templine != endline) */

	    /* last line now find out how far to the right to clear*/
	    width = bkr_copy_ctx.x_end;
	
	    if (fixup_highlight(window, templine,
                                &event_rect,
                                templine->x,
                                templine->y,
                                width,
                                templine->height)
                )
            {
		templine->exposed = highlight_exposed = TRUE;
            }
        }
    } /* end if(bkr_copy_ctx.active && */	
#endif

    /* Expose those graphics chunks which intersect the expose event
     */
    if ( ! bkrplus_g_charcell_display) 
    {
        
        chunk = window->u.topic.data->graphic_data;
        while (chunk)
        {
            if (RECTS_OVERLAP(&chunk->rect,&event_rect))
            {
                bkr_display_data(XtWindow( drawing_window ), /* Window */
                                 chunk,			     /* Chunk descriptor */
                                 window->u.topic.x,   	     /* Virtual topic origin */
                                 window->u.topic.y,	     /* Virtual topic origin */
                                 event,		    	     /* Expose event */
                                 window );
#ifdef MEMEX    	    
                if ( chunk->highlight )
                {
                    highlight_chunk( window, chunk );
                }
#endif
		if ( chunk->data_type == BMD_CHUNK_RAGS ||
		     chunk->data_type == BMD_CHUNK_RAGS_NO_FILL )
		    rags = TRUE;
            }
            chunk = chunk->next;
        }
    }

#ifdef USE_TEXT_LINES
    if (bkrplus_g_charcell_display) 
    {
        bkr_window_draw_cc_text(window,bkr_text_gc,&event_rect);
    } 
    else 
    {
        line = window->u.topic.data->text_lines;
        while (line)
        {
            if (line_in_event(line,&event_rect))
            {
                BKR_TEXT_ITEM_LIST *item_list = line->item_lists;

                while (item_list) {
                    if (item_list->is2byte) {
                        XDrawText16(bkr_display,
                                    XtWindow( drawing_window ),
                                    bkr_text_gc,
                                    item_list->x + window->u.topic.x,
                                    line->baseline + window->u.topic.y,
                                    item_list->u.item16s,
                                    item_list->n_items);
                    } else {
                        XDrawText(bkr_display,
                                  XtWindow( drawing_window ),
                                  bkr_text_gc,
                                  item_list->x + window->u.topic.x,
                                  line->baseline + window->u.topic.y,
                                  item_list->u.items,
                                  item_list->n_items);
                    }
                    item_list = item_list->next;
                }
            }
            line = line->next;
        }
    }

#endif 
    /* Outline any hotspots if they are turned on.
     */
    if ( window->u.topic.show_hot_spots )
    {
        chunk = window->u.topic.data->hot_spots;
        while (chunk) 
        {
            if (RECTS_OVERLAP(&chunk->rect,&event_rect))
            {
                bkr_pointer_outline_hot_spot( window, chunk, ON );
            }
            chunk = chunk->next;
        }
    }

    /* Display any extensions if they are turned on.
     */
    if ( window->u.topic.show_extensions )
    {
        chunk = window->u.topic.data->extensions;
        while (chunk) 
        {
            if (RECTS_OVERLAP(&chunk->rect,&event_rect))
            {

                XSetTSOrigin(bkr_display, bkr_extension_gc, 
                             window->u.topic.x & 15, window->u.topic.y & 15 );

                if (chunk->chunk_type == BMD_EXTENSION_RECT) 
                {
                    XFillRectangle(bkr_display, 
                                   XtWindow( drawing_window ),
                                   bkr_extension_gc,	    	/* GC */
                                   window->u.topic.x + chunk->rect.left,
                                   window->u.topic.y + chunk->rect.top,
                                   chunk->rect.width, 
                                   chunk->rect.height );
                }
                else 
                {
		    /*
                     *  The points are scaled and translated relative to their 
                     *  parent chunk, but need to be translated relative to the 
                     *  current topic x & y.
                     */
                    int	    point;
                    int     x_offset = window->u.topic.x + chunk->rect.left;
                    int     y_offset = window->u.topic.y + chunk->rect.top;
                    
                    for ( point = 0; point < chunk->num_points; point++ )
                    {
                        chunk->xpoint_vec[point].x =  chunk->point_vec[point].x + x_offset;
                        chunk->xpoint_vec[point].y =  chunk->point_vec[point].y + y_offset;
                    }
                    chunk->xpoint_vec[chunk->num_points].x = chunk->xpoint_vec[0].x;
                    chunk->xpoint_vec[chunk->num_points].y = chunk->xpoint_vec[0].y;

                    XFillPolygon(bkr_display, 
                                 XtWindow( drawing_window ),	/* Window */
                                 bkr_extension_gc,		/* GC */
                                 chunk->xpoint_vec,      	/* Points */
                                 chunk->num_points+1,      	/* Num_points */
                                 Complex,	    	    	/* Shape  */
                                 CoordModeOrigin );       	/* Mode  */
                }
            }
            chunk = chunk->next;
        }
    }


    /* Re select/outline hot spots if necessary 
     */
    if (selected_chunk)
    {
    	bkr_button_select_hot_spot( window, selected_chunk, SELECT );
    }
    if (outlined_chunk)
    {
    	bkr_pointer_outline_hot_spot( window, outlined_chunk, ON );
    }


#ifdef COPY
    /* turn on highlighting if needed */
    if(bkr_copy_ctx.active && (!bkr_copy_ctx.enabled)) {
	bkr_highlight_exposure(window);
	bkr_copy_ctx.enabled = TRUE;
	} 
     else if( bkr_copy_ctx.active && highlight_exposed ) {
	bkr_highlight_exposure(window);
	}
#endif

#ifdef SEARCH
    /* See if search already has the wait cursor on.
     */
    if ( ! bkrplus_g_search_in_progress) 
    {
        bkr_cursor_display_wait( OFF );
    }
#else
    bkr_cursor_display_wait( OFF );
#endif 

    return rags;
};  /* end of expose_topic */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	hotspot_in_event
**
** 	Unions the position and dimensions of the event passed with the 
**  	hot spot passed.
**
**  	Selected and outlined hot spots must be special cased.  If the text 
**  	of a selected or outlined hot spot is re-drawn, without first 
**  	clearing the area within the window, the text will be painted with 
**  	foreground color on foreground color, or part of the outlined box
**  	will be missing.
**  	
**
**  FORMAL PARAMETERS:
**
**	topic	     - pointer to the Topic for display.
**  	expose_rect  - the clip box of the Expose event for the drawing window.
**  	this_chunk   - id of the chunk to union.
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
**	Returns:    TRUE  - if the hot spot intersects the event,
**  	    	    FALSE - otherwise 
**
**  SIDE EFFECTS:
**
**	The event passed in might be modified during the union.
**
**--
**/
static BMD_CHUNK *
hotspot_in_event PARAM_NAMES((window,event_rect,hotspot))
    BKR_WINDOW    *window     PARAM_SEP
    BMD_RECTANGLE *event_rect PARAM_SEP
    BMD_CHUNK	  *hotspot    PARAM_END

{

    if (hotspot)
    {
        merge_rectangles(&hotspot->rect,event_rect);
        XClearArea(bkr_display,
                   XtWindow( window->widgets[W_WINDOW] ),
                   hotspot->rect.left + window->u.topic.x,
                   hotspot->rect.top + window->u.topic.y,
                   hotspot->rect.width, 
                   hotspot->rect.height, 
                   FALSE );
    }
    return hotspot;

};  /* end of hotspot_in_event */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	fixup_highlight
**
** 	Unions the position and dimensions of the event passed with the 
**  	highlighted area passed.
**
**  	Highlighted areas must be special cased.  If the text 
**  	of a Highlighted area is re-drawn, without first 
**  	clearing the area within the window, the text will be painted with 
**  	foreground color on foreground color, or part of the outlined box
**  	will be missing.
**  	
**
**  FORMAL PARAMETERS:
**
**	topic	     - pointer to the Topic for display.
**  	event_rect   - rectangle for expose event adjusted for topic offset.
**  	x ,y
**	width
**	height
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
**	Returns:    TRUE  - if the hot spot intersects the event,
**  	    	    FALSE - otherwise 
**
**  SIDE EFFECTS:
**
**	The event passed in might be modified during the union.
**
**--
**/
static Boolean
fixup_highlight PARAM_NAMES((window,line,event_rect,x,y,width,height))
    BKR_WINDOW    *window PARAM_SEP
    BKR_TEXT_LINE *line PARAM_SEP
    BMD_RECTANGLE *event_rect PARAM_SEP
    int		 x PARAM_SEP
    int		 y PARAM_SEP
    int		 width PARAM_SEP
    int		 height PARAM_END
{

    BMD_RECTANGLE highlight_rect;

    highlight_rect.top = y;
    highlight_rect.bottom = y + height;
    highlight_rect.height = height;
    highlight_rect.left = x;
    highlight_rect.right = x + width;
    highlight_rect.width = width;

    if (RECTS_OVERLAP(&highlight_rect,event_rect)
        || line_in_event(line,event_rect)
        )
    {
        merge_rectangles(&highlight_rect,event_rect);

        XClearArea(bkr_display,
                   XtWindow(window->widgets[W_WINDOW]),
                   window->u.topic.x + x,
                   window->u.topic.y + y,
                   width,
                   height,
                   FALSE );

        return (TRUE);
    }
    return (FALSE);

};  /* end of fixup_highlight */


#ifdef MEMEX
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	highlight_chunk
**
** 	Highlights a chunk in the Topic window.  Highlighting is used
**  	to show connections for MEMEX.
**
**  FORMAL PARAMETERS:
**
**	topic - pointer to the Topic window.
**  	chunk - pointer to the chunk to highlight.
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
highlight_chunk PARAM_NAMES((window,chunk))
    BKR_WINDOW *window PARAM_SEP
    BMD_CHUNK  *chunk PARAM_END

{
    Widget  	    	drawing_window = window->widgets[W_WINDOW];
    int     	    	bbox_x;
    int     	    	bbox_y;
    int     	    	bbox_width;
    int     	    	bbox_height;
    Arg	    	    	arglist[5];
    int	    	    	argcnt;
    Dimension	    	window_width;
    Dimension	    	window_height;

    argcnt = 0;
    SET_ARG( XmNwidth, &window_width );
    SET_ARG( XmNheight, &window_height );
    XtGetValues( drawing_window, arglist, argcnt );

    /* Maximize the rectangle to stay on the screen */

    bbox_x  	= window->u.topic.x + chunk->rect.left;
    bbox_y  	= window->u.topic.y + chunk->rect.top;
    bbox_width	= chunk->rect.width;
    bbox_height	= chunk->rect.height;

    /* Make sure the edges are visible */

    if ( bbox_x == 0 )
    {
    	bbox_x = 2;
    	bbox_width -= 4;
    }
    if ( bbox_y == 0 )
    	bbox_y = 3;
    else
    	bbox_y += 3;	    /* prevent overlap of highlight boxes */
    bbox_height -= 3;

    XDrawRectangle(
    	bkr_display,
    	XtWindow( drawing_window ),
    	bkr_text_gc,
    	bbox_x, bbox_y,
    	bbox_width, bbox_height );

};  /* end of highlight_chunk */
#endif /* MEMEX */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	setup_drawing_gc
**
** 	Creates the shared Graphic Contexts (GC's) used in the Bookreader.
**
**  FORMAL PARAMETERS:
**
**	drawing_window_widget  - widget id of the drawing area widget.
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
setup_drawing_gc PARAM_NAMES((drawing_window_widget))
    Widget  drawing_window_widget PARAM_END
{
    Arg	    	    arglist[5];
    int	    	    argcnt;
    XGCValues	    xgcvalues;
    unsigned long   value_mask;
    Pixmap  	    extension_stipple_pixmap;

    /* Get the foreground and background colors */

    argcnt = 0;
    SET_ARG( XmNforeground, &bkr_window_foreground );
    SET_ARG( XmNbackground, &bkr_window_background );
    XtGetValues( drawing_window_widget, arglist, argcnt );

    /* Create the text and reverse-video text GC's */

    xgcvalues.foreground = bkr_window_foreground;
    xgcvalues.background = bkr_window_background;
    xgcvalues.font = bkr_default_font->fid;
    value_mask = GCForeground | GCBackground | GCFont ;
    bkr_text_gc = (GC) XCreateGC( 
    	    	    	    bkr_display,
    	    	    	    XDefaultRootWindow( bkr_display ),
    	    	    	    value_mask, 
    	    	    	    &xgcvalues );


    xgcvalues.background = bkr_window_foreground;
    xgcvalues.foreground = bkr_window_background;
    xgcvalues.font = bkr_default_font->fid;
    value_mask = GCForeground | GCBackground | GCFont ;
    bkr_reverse_text_gc = (GC) XCreateGC( 
    	    	    	    bkr_display,
    	    	    	    XDefaultRootWindow( bkr_display ),
    	    	    	    value_mask, 
    	    	    	    &xgcvalues );

    xgcvalues.foreground = bkr_window_background;
    xgcvalues.background = bkr_window_foreground;
    value_mask = GCForeground | GCBackground;
    bkr_outline_off_gc = (GC) XCreateGC(
    	    	    	    	bkr_display,
    	    	    	    	XDefaultRootWindow( bkr_display ),
    	    	    	    	value_mask, 
    	    	    	    	&xgcvalues );

    xgcvalues.foreground = 0xFFFFFFFF;
    xgcvalues.background = 0;
    xgcvalues.function 	 = GXxor;
    xgcvalues.plane_mask = bkr_window_foreground ^ bkr_window_background;
    value_mask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
    bkr_xor_gc = (GC) XCreateGC(
    	    	    	bkr_display,
    	    	    	XDefaultRootWindow( bkr_display ),
    	    	    	value_mask, 
    	    	    	&xgcvalues );

    /* Create the extension stipple GC */

    extension_stipple_pixmap = bkr_icon_create_stipple_pixmap();

    xgcvalues.foreground = bkr_window_foreground;
    xgcvalues.background = bkr_window_background;
    xgcvalues.fill_style = FillStippled;
    xgcvalues.stipple 	 = extension_stipple_pixmap;
    value_mask = GCForeground | GCBackground | GCFillStyle | GCStipple;
    bkr_extension_gc = XCreateGC( 
    	    	    	    bkr_display,
    	    	    	    XDefaultRootWindow( bkr_display ),
    	    	    	    value_mask, 
    	    	    	    &xgcvalues );

};  /* end of setup_drawing_gc */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	take_focus_callback
**
** 	Handles the callback for WM_TAKE_FOCUS by pre- and
**	post-pending '*' to the title bar name for the window getting
**	focus.
**
**  FORMAL PARAMETERS:
**
**	widget    - the application shell id (NOT USED)
**
**	user_data - the BKR_WINDOW pointer for the window getting
**                  focus 
**
**      cbs       - callback data (NOT USED)
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
take_focus_callback PARAM_NAMES((widget,client_data,cbs))
    Widget widget PARAM_SEP
    XtPointer client_data PARAM_SEP
    XmAnyCallbackStruct *cbs PARAM_END
{
    BKR_WINDOW *focus_window = (BKR_WINDOW *)client_data;
    BKR_SHELL *shell         = bkr_all_shells;
    unsigned	    	    status;
    XmString	 	    cs_name;
    long         	    cs_length;
    long         	    cs_status;
    char                    buffer[1024];

    static BKR_WINDOW       *previous_focus_window = NULL;

    if ((previous_focus_window) 
        && (previous_focus_window != focus_window)
        && (previous_focus_window->active)
        )
    {
        cs_name = DXmCvtFCtoCS( previous_focus_window->title_bar_name, &cs_length, &cs_status);
        DWI18n_SetTitle( previous_focus_window->appl_shell_id, cs_name );
        COMPOUND_STRING_FREE( cs_name );
    }

    sprintf(buffer," *%s* ",focus_window->title_bar_name);
    cs_name = DXmCvtFCtoCS( buffer, &cs_length, &cs_status);
    DWI18n_SetTitle( focus_window->appl_shell_id, cs_name );
    COMPOUND_STRING_FREE( cs_name );

    previous_focus_window = focus_window;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_window_setup_wm_protocols
**
** 	Setup for window manager protocols and callbacks.
**
**  FORMAL PARAMETERS:
**
**	window - BKR_WINDOW pointer
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
bkr_window_setup_wm_protocols PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    static Atom protocols[2];
    static Boolean first_call = TRUE;
    static Atom WM_TAKE_FOCUS_ATOM;
    static n_protocols;

    /* The first time we're called we need to do some initialization.
     */
    if (first_call) {

        /* We always handle the WM_DELETE_WINDOW protocl
         */
        protocols[0] = WM_DELETE_WINDOW_ATOM;

        if (bkrplus_g_charcell_display) 
        {
            /* In character cell mode we want to know when the window
             * gets focus.
             */
            WM_TAKE_FOCUS_ATOM = XmInternAtom(bkr_display,"WM_TAKE_FOCUS",FALSE);
            protocols[1] = WM_TAKE_FOCUS_ATOM;
            n_protocols = 2;
        }
        else 
        {
            n_protocols = 1;
        }
        first_call = TRUE;
    }

    /* Add the protocols for the window.
     */
    XmAddWMProtocols(window->appl_shell_id, protocols, n_protocols );

    /* In character cell mode, setup a callback to WM_TAKE_FOCUS.
     */
    if (bkrplus_g_charcell_display) 
    {
        XmAddWMProtocolCallback(window->appl_shell_id,
                                WM_TAKE_FOCUS_ATOM,
                                take_focus_callback,
                                window);
    }
    /* Add event handlers
     */
    XtAddEventHandler(window->appl_shell_id, NoEventMask, TRUE, 
                      bkr_window_client_message, window );
    XtAddEventHandler(window->appl_shell_id, StructureNotifyMask, FALSE, 
                      bkr_window_initialize_icons, window );

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	merge_rectangles
**
** 	Merge the source BMD_RECTANGLE into the destination BMD_RECTANGLE such
**      that the destination is the smallest enclosing rectangle of the union
**      of the source and the destination.
**
**  FORMAL PARAMETERS:
**
**	src - pointer to the source BMD_RECTANGLE
**
**	dst - pointer to the desitnation BMD_RECTANGLE
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
merge_rectangles PARAM_NAMES((src, dst))
    register BMD_RECTANGLE *src PARAM_SEP
    register BMD_RECTANGLE *dst PARAM_END
{
    if (src->left < dst->left) {
        dst->left = src->left;
    }
    if (src->right > dst->right) {
        dst->right = src->right;
    }
    dst->width = dst->right - dst->left;

    if (src->top < dst->top) {
        dst->top = src->top;
    }
    if (src->bottom > dst->bottom) {
        dst->bottom = src->bottom;
    }
    dst->height = dst->bottom - dst->top;
}


