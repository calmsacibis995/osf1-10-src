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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CALLBACKS.C*/
/* *9    24-FEB-1993 17:47:33 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *8     8-NOV-1992 19:15:43 BALLENGER "Fix handling of Next/Prev Screen keys."*/
/* *7    22-SEP-1992 15:54:14 BALLENGER "Remove check for hotpsots in bkr_topic_focus()."*/
/* *6    21-SEP-1992 22:13:38 BALLENGER "Focus highlight in topic window."*/
/* *5     5-AUG-1992 21:46:35 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *4     9-JUN-1992 09:59:05 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *3     3-MAR-1992 17:05:01 KARDON "UCXed"*/
/* *2    18-SEP-1991 18:18:11 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:46 PARMENTER "Topic Window Callbacks"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CALLBACKS.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CALLBACKS.C*/
/* *8     8-NOV-1992 19:15:43 BALLENGER "Fix handling of Next/Prev Screen keys."*/
/* *7    22-SEP-1992 15:54:14 BALLENGER "Remove check for hotpsots in bkr_topic_focus()."*/
/* *6    21-SEP-1992 22:13:38 BALLENGER "Focus highlight in topic window."*/
/* *5     5-AUG-1992 21:46:35 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *4     9-JUN-1992 09:59:05 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *3     3-MAR-1992 17:05:01 KARDON "UCXed"*/
/* *2    18-SEP-1991 18:18:11 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:46 PARMENTER "Topic Window Callbacks"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CALLBACKS.C*/
#ifndef VMS
 /*
#else
#module BKR_TOPIC_CALLBACKS "V03-0002"
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
**	Callback routines for functions in the Topic window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     13-Mar-1990
**
**  MODIFICATION HISTORY:
**
**	V03-0002    JAF0002	James A. Ferguson   	1-Feb-1991
**  	    	    Fix code which performs a union of the expose events
**  	    	    in bkr_topic_goback.
**
**	V03-0001    JAF0001	James A. Ferguson   	13-Mar-1990
**  	    	    Extracted V2 routines and created new module.
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
#include "bkr_topic_callbacks.h" /* function prototypes for .c module */
#include "bkr_topic_data.h" /* function prototypes for .c module */
#include "bkr_button.h"      /* Button manipulation routines */
#include "bkr_close.h"       /* Close routines */
#include "bkr_scroll.h"      /* Scrollbar routines */
#include "bkr_topic_init.h"  /* Topic window initialization routines */l
#include "bkr_topic_open.h"  /* Topic window open routines */
#include "bkr_window.h"      /* Window manipulation routines */
#include <DXm/DECspecific.h>
#ifdef __osf__
#include "XcmStrings.h"
#else
#include <Xcm/XcmStrings.h>
#endif

/*
 * FORWARD ROUTINES
 */
static void  	    
remove_history_list_entry
    PROTOTYPE(( BKR_WINDOW *window ));


/*
 * FORWARD DEFINITIONS 
 */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_bottom
**
** 	Callback routine which positions the contents of the Topic window 
**  	at the end of the current topic.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	topic_shell 	- pointer to a Topic shell.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_bottom PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    int	    	    	last_screen_top;

    /* Topic already at top of last screen? */

    if ( bkr_find_last_screen_top( window, &last_screen_top ) )
    	return;

    /* Position the contents at (x=whatever, y=last_screen_top) */

    window->u.topic.internal_y = last_screen_top;
    bkr_scroll_adjust_display( window );
    bkr_scroll_adjust_scrollbars( window, window->u.topic.internal_x,
    	window->u.topic.internal_y, window->u.topic.width, window->u.topic.height);

};  /* end of bkr_topic_bottom */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_extensions_onoff
**
** 	Callback routine to toggle "show extensions"
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event.
**	shell	    	- pointer to the shell data structure.
**	callback_data	- pointer to callback data (not used).
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
bkr_topic_extensions_onoff PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XmToggleButtonCallbackStruct     *callback_data PARAM_END

{
    Widget drawing_window = window->widgets[W_WINDOW];

    window->u.topic.show_extensions = ! window->u.topic.show_extensions;

    XClearWindow( bkr_display, XtWindow( drawing_window ) );

    /* callback data - NULL forces a dummy event to be created 
     */
    bkr_window_expose( drawing_window, window, NULL ); 

};  /* end of bkr_topic_extensions_onoff */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_goback
**
** 	Callback routine for performing a "Go Back" operation
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event.
**  	data	    	- generic user data.
**	callback_data	- pointer to callback data.
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
bkr_topic_goback PARAM_NAMES((widget,data,callback_data))
    Widget		    widget PARAM_SEP
    caddr_t	    	    data PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    BKR_WINDOW          *window;
    BKR_BACK_TOPIC  	*back_topic;
    Arg	    	    	arglist[5];
    int	    	    	argcnt;

    /* 
     *  Get the user data, if non-NULL then the callback is from a Topic popup 
     *  menu, otherwise its from a Topic shell pulldown or button box.
     */

    argcnt = 0;
    SET_ARG( XmNuserData, &window );
    XtGetValues( widget, arglist, argcnt );
    if ( window == NULL )
    	window = (BKR_WINDOW *) data;

    if ( window->type != BKR_STANDARD_TOPIC ) 
    	return;

    back_topic = window->u.topic.back_topic;

    if ( back_topic == NULL )
        return;

    /*  Fixes the case where a "Go Back" will cause an upscroll 
     *  within the same topic.  We must first gather all
     *  outstanding expose events for the window before scrolling.
     */

    if ( window->u.topic.page_id == back_topic->page_id )
    {
    	Window	    	event_window = XtWindow( window->widgets[W_WINDOW] );
        XExposeEvent	*xexpose;
        XExposeEvent    merged_event;
    	XEvent	    	xevent;
    	Boolean	    	expose_pending = FALSE;

    	/* Union all the XExpose events together.
    	 */
    	while ( XCheckWindowEvent( bkr_display, event_window,
    	    	    	    	   ExposureMask, &xevent ) )
    	{
    	    xexpose = (XExposeEvent *) &xevent.xexpose;
    	    if ( ! expose_pending )
    	    {
    	    	COPY_XEXPOSE_EVENT( merged_event, xexpose );
    	    	expose_pending = TRUE;
    	    }
    	    else
    	    {
    	    	int x, y;

    	    	x = MIN( merged_event.x, xexpose->x );
    	    	y = MIN( merged_event.y, xexpose->y );
    	    	merged_event.width = MAX( merged_event.x + merged_event.width,
    	    	    	    	    	  xexpose->x + xexpose->width ) - x;
    	    	merged_event.height = MAX( merged_event.y + merged_event.height,
    	    	    	    	    	   xexpose->y + xexpose->height ) - y;
    	    	merged_event.x = x;
    	    	merged_event.y = y;
    	    }
    	}
    	if ( expose_pending )
    	{
    	    XmDrawingAreaCallbackStruct window_callback;

    	    window_callback.reason = XmCR_EXPOSE;
    	    window_callback.event  = (XEvent *) &merged_event;
    	    window_callback.window = event_window;
	    bkr_window_expose( window->widgets[W_WINDOW], 
    	    	    	       window, &window_callback );
    	}
    }
    else 
    {	 /*  Unselect the hotspot before re-opening the topic; this 
     	  *  will update the sensitivity of the pulldown menu push buttons.
     	  */
    	if ( window->u.topic.selected_chunk != NULL )
    	    bkr_button_select_hot_spot( window, window->u.topic.selected_chunk, UNSELECT );
    }

    /* 
     *  Reopen the Topic 
     */

    bkr_topic_open_to_position(
    	window->shell,  /* Parent shell     	    	*/
    	window,	    	/* Display in this Topic shell  */
    	back_topic->page_id,	/* Topic id 	    	    	*/
    	back_topic->x,	    	/* X position   	    	*/
    	back_topic->y,	    	/* Y position   	    	*/
    	0, 	    	    	/* Chunk id 	    	    	*/
    	FALSE );    	    	/* Don't add to history	    	*/

    /* Remove this entry from the history list queue */

    remove_history_list_entry( window );

    BKR_FLUSH_EVENTS;

};  /* end of bkr_topic_goback */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_hot_spots_onoff
**
** 	Callback routine to toggle "show hot spots"
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event.
**	shell	    	- pointer to the shell data structure.
**	callback_data	- pointer to callback data (not used).
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
bkr_topic_hot_spots_onoff PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XmToggleButtonCallbackStruct     *callback_data PARAM_END

{
    Widget drawing_window = window->widgets[W_WINDOW];

    window->u.topic.show_hot_spots = ! window->u.topic.show_hot_spots;
    window->u.topic.outlined_chunk = NULL;

    XClearWindow( bkr_display, XtWindow( drawing_window ) );

    /* callback data - NULL forces a dummy event to be created 
     */
    bkr_window_expose( drawing_window, window, NULL );

};  /* end of bkr_topic_hot_spots_onoff */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_next
**
** 	Callback routine for performing a "Next Topic" operation
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	data	    	- generic user data.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_next PARAM_NAMES((widget,data,callback_data))
    Widget		    widget PARAM_SEP
    caddr_t	    	    data PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    BKR_WINDOW	    	*window;
    BMD_OBJECT_ID	page_id;
    Arg	    	    	arglist[5];
    int	    	    	argcnt;

    if (widget == NULL) 
    {
        /* Called directly from next screen which was called with
         * a NULL widget;
         */
        window = (BKR_WINDOW *) data;
    }
    else 
    {
        /* 
         *  Get the user data, if non-NULL then the callback is from a Topic popup 
         *  menu, otherwise its from a Topic shell pulldown or button box.
         */
        argcnt = 0;
        SET_ARG( XmNuserData, &window );
        XtGetValues( widget, arglist, argcnt );
        if ( window == NULL )
        {
            window = (BKR_WINDOW *) data;
        }
    }

    if ( window->type != BKR_STANDARD_TOPIC ) 
    	return;

    page_id = bri_page_next( window->shell->book->book_id, window->u.topic.page_id );
    if ( page_id != 0 ) 
    {
    	bkr_topic_open_to_position(window->shell,  /* Parent shell 	    	    */
                                   window,    	    /* Display in this Topic shell  */
                                   page_id,	    /* Topic id     	    	    */
                                   0,0,            /* x,y                          */
                                   0, 	    	    /* Chunk id     	    	    */
                                   TRUE );	    /* Add to history list          */
        
        BKR_FLUSH_EVENTS;
    }
};  /* end of bkr_topic_next */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_next_screen
**
** 	Callback routine for scrolling the Topic window one screen forward.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	window 	- pointer to a Topic shell.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_next_screen PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    int	    	    	new_top_y;
    Arg	    	    	arglist[5];
    int	    	    	argcnt;
    int	    	    	increment;
    int	    	    	page_increment;
    int	    	    	last_screen_top;

    if ( ( window->type != BKR_STANDARD_TOPIC ) 
    	    && ( window->type != BKR_FORMAL_TOPIC ) )
    	return;

    /*  
     *  If we are NOT at the bottom of the window, calculate the new top "y" 
     *  value for the next screen and adjust the display otherwise just 
     *  open the "Next" topic.
     */

    if ( ! bkr_find_last_screen_top( window, &last_screen_top ) )
    {
        int screen_height;
        Dimension window_height;

        /* We need to get the window height, in case it's been resized.
         */
        XtVaGetValues(window->widgets[W_WINDOW],XmNheight,&window_height,NULL);

        if (bkrplus_g_charcell_display) 
        {
            screen_height = (int)window_height - bkr_default_line_height;
        }
        else 
        {
            screen_height = (int)window_height - TOPIC_UNIT_INCREMENT;
        }

    	if ( window->u.topic.y == 0 )
    	    new_top_y =  -screen_height;
    	else
    	    new_top_y = window->u.topic.y - screen_height;
    	new_top_y = MAX( new_top_y, last_screen_top );

    	window->u.topic.internal_y = new_top_y;
    	bkr_scroll_adjust_display( window );
        bkr_scroll_adjust_scrollbars( window, window->u.topic.internal_x, 
    	    window->u.topic.internal_y, window->u.topic.width, window->u.topic.height );
        if (window->type == BKR_STANDARD_TOPIC) {
            bkr_screen_init_sensitivity( window );
        }
    	/* Only need to flush events here because next_topic will flush.
    	 */
    	BKR_FLUSH_EVENTS;
    }
    else if (window->type == BKR_STANDARD_TOPIC) 
    {
        bkr_topic_next( NULL, (caddr_t)window, callback_data );
    }
};  /* end bkr_topic_next_screen */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_previous
**
** 	Callback routine for performing a "Previous Topic" operation
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	window 	- pointer to a Topic shell.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_previous PARAM_NAMES((widget,data,callback_data))
    Widget		    widget PARAM_SEP
    caddr_t	    	    data PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    BKR_WINDOW	    	*window;
    BMD_OBJECT_ID	page_id;
    Arg	    	    	arglist[5];
    int	    	    	argcnt;

    if (widget == NULL) 
    {
        /* Called directly from previous screen which was called with
         * a NULL widget;
         */
        window = (BKR_WINDOW *) data;
    }
    else 
    {
        /* 
         *  Get the user data, if non-NULL then the callback is from a Topic popup 
         *  menu, otherwise its from a Topic shell pulldown or button box.
         */
        argcnt = 0;
        SET_ARG( XmNuserData, &window );
        XtGetValues( widget, arglist, argcnt );
        if ( window == NULL )
        {
            window = (BKR_WINDOW *) data;
        }
    }

    if ( window->type != BKR_STANDARD_TOPIC ) 
    	return;

    page_id = bri_page_previous( window->shell->book->book_id, window->u.topic.page_id );
    if ( page_id != 0 ) {
    	bkr_topic_open_to_position(window->shell, /* Parent shell                */
                                   window,    	  /* Display in this Topic shell */
                                   page_id,	  /* Topic id                    */
                                   0,0,           /* x,y                         */
                                   0, 	    	  /* Chunk id     	    	 */
                                   TRUE );	  /* Add to history list         */

        BKR_FLUSH_EVENTS;
    }
};  /* end of bkr_topic_previous */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_previous_screen
**
** 	Callback routine for scrolling the Topic window one screen backwards.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	window 	- pointer to a Topic shell.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_previous_screen PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    Dimension	        window_height;

    if ( ( window->type != BKR_STANDARD_TOPIC )     
    	    && ( window->type != BKR_FORMAL_TOPIC ) )
    	return;

    /* We need to get the window height, in case it's been resized.
     */
    XtVaGetValues(window->widgets[W_WINDOW],XmNheight,&window_height,NULL);

    /*  
     *  If we are at the top of the window then open the "Previous" topic
     *  to the last screen otherwise calculate the new top "y" value for 
     *  the previous screen and adjust the display.
     */
    if ( window->u.topic.y == 0 )
    {
        BMD_OBJECT_ID	page_id;

    	page_id = bri_page_previous( window->shell->book->book_id, window->u.topic.page_id );
    	if ( page_id != 0 ) {

            BKR_TOPIC_DATA *topic;
            int last_screen_top;

            /* Get the topic data and calculate the last screen top
             * for that topic.  We can't just call bkr_find_last_screen_top()
             * because it uses the topic data that is already in the window.
             */
            topic = bkr_topic_data_get(window->shell->book, page_id);

            if (topic->height <= window_height) {
                last_screen_top = 0;
            }
            else 
            {
                int maximum_value = bkr_scroll_max_y( topic->height, window_height );
                last_screen_top =  - ( maximum_value - (int) window_height );
            }

            /* Mark the data as free.  So it won't be marked as opened twice when we call
             * bkr_topic_open_to_position().
             */
            bkr_topic_data_free(window->shell->book, page_id);
            
            /* Open the topic to the last screen of the previous topic. We're reusing this
             * window which will cause the topic data in it to be freed. But it will first
             * do a bkr_topic_data_get for the topic that we just freed, thus reusing it.
             */
            bkr_topic_open_to_position(window->shell, 
                                       window, 
                                       page_id, 
                                       0, last_screen_top, 
                                       0, 
                                       FALSE);
        }
    }
    else if (window->type == BKR_STANDARD_TOPIC) /* No previous topic for FORMAL */
    {
        int screen_height;

        if (bkrplus_g_charcell_display) 
        {
            screen_height = (int)window_height - bkr_default_line_height;
        }
        else 
        {
            screen_height = (int)window_height - TOPIC_UNIT_INCREMENT;
        }

    	window->u.topic.internal_y = MIN( 0, window->u.topic.y + screen_height );
    	bkr_scroll_adjust_display( window );
        bkr_scroll_adjust_scrollbars( window, window->u.topic.internal_x,
    	    window->u.topic.internal_y, window->u.topic.width, window->u.topic.height);
        if ( window->type == BKR_STANDARD_TOPIC )
        {	
            bkr_topic_init_sensitivity( window );
            bkr_screen_init_sensitivity( window );
        }
    }
    BKR_FLUSH_EVENTS;

};  /* end bkr_topic_previous_screen */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_top
**
** 	Callback routine which positions the contents of the Topic window 
**  	at the start of the current topic.
**
**  FORMAL PARAMETERS:
**
**  	widget	    	- widget id of the push button that caused the event.
**  	window 	- pointer to a Topic shell.
**	callback_data	- pointer to callback data (unused).
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
bkr_topic_top PARAM_NAMES((widget,window,callback_data))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XmAnyCallbackStruct     *callback_data PARAM_END

{
    /* Already at top of topic */

    if ( window->u.topic.y == 0 )
    	return;

    /* Position the contents at (x=whatever, y=0) */

    window->u.topic.internal_y = 0;
    bkr_scroll_adjust_display( window );
    bkr_scroll_adjust_scrollbars( window, window->u.topic.internal_x,
    	window->u.topic.internal_y, window->u.topic.width, window->u.topic.height);

};  /* end of bkr_topic_top */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_find_last_screen_top
**
** 	Determines the Y value for the last screen of data to be 
**  	displayed in the Topic window.
**
**  FORMAL PARAMETERS:
**
**  	topic	     	    - pointer to the currently displayed Topic.
**  	last_screen_top_rtn - address of an integer for returning the new Y position.
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
**	Returns:    TRUE - if the topic Y position is already on the last screen
**  	    	    FALSE - otherwise
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
unsigned
bkr_find_last_screen_top PARAM_NAMES((window,last_screen_top_rtn))
    BKR_WINDOW *window PARAM_SEP
    int *last_screen_top_rtn PARAM_END
{
    Arg	    	arglist[5];
    int	    	argcnt;
    Dimension	window_height;
    int	    	new_top_y;
    int	    	maximum_value;

    *last_screen_top_rtn = 0;	/* initial value */

    argcnt = 0;
    SET_ARG( XmNheight, &window_height );
    XtGetValues( window->widgets[W_WINDOW], arglist, argcnt );

    if (window->u.topic.height <= (int) window_height ) {
    	new_top_y = 0;
    } else {
    	maximum_value = bkr_scroll_max_y( window->u.topic.height, window_height );
        new_top_y =  - ( maximum_value - (int) window_height );
    }

    if ( new_top_y == window->u.topic.y )
    	return ( 1 );

    *last_screen_top_rtn = new_top_y;
    return ( 0);

};  /* end of bkr_find_last_screen_top */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_keyboard_actions
**
** 	Key input action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xbutton_event	- X event associated with the Btn1Up event
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
bkr_topic_keyboard_actions PARAM_NAMES((widget,event,params,num_params))
    Widget		    widget PARAM_SEP
    XKeyEvent  	            *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window = NULL;
    BMD_CHUNK               *chunk = NULL;
    char                    buffer[50];
    XComposeStatus          compose;
    KeySym                  keysym;

    window = bkr_window_find( widget );
    if ((window == NULL) 
        || ((window->type != BKR_STANDARD_TOPIC) && (window->type != BKR_FORMAL_TOPIC))
        ) {
    	return;
    }
    
    if (*num_params < 1) 
    {
        switch (event->type) 
        {
            case KeyPress: 
            {
                fprintf(stderr,"KeyPress:   ");
                break;
            }
            case KeyRelease: 
            {
                fprintf(stderr,"KeyRelease: ");
                break;
            }
            default:
            {
                return;
            }
        }
        (void)XLookupString(event,buffer,50,&keysym,&compose);
        fprintf(stderr,"%04x, %s\n",keysym,buffer);
    }
    else if (strcmp(params[0],"PrevScreen") == 0 )
    {
        bkr_topic_previous_screen(window->widgets[W_PREV_SCREEN_BUTTON],window,NULL);
    }
    else if (strcmp(params[0],"NextScreen") == 0 )
    {
        bkr_topic_next_screen(window->widgets[W_NEXT_SCREEN_BUTTON],window,NULL);
    }
    else if (window->u.topic.data->hot_spots)
    {
        if (strcmp(params[0],"ViewHotSpot") == 0 )
        {
            if (window->u.topic.selected_chunk) {
                bkr_object_id_dispatch(window->shell,
                                       window,
                                       window->u.topic.selected_chunk->target);
            }
        }
        else if (strcmp(params[0],"ViewHotSpotInNew") == 0 )
        {
            if (window->u.topic.selected_chunk) {
                bkr_object_id_dispatch(window->shell,
                                       NULL,
                                       window->u.topic.selected_chunk->target);
            }
        }
        else 
        {
            if ( strcmp(params[0],"PrevHotSpot") == 0)
            {
                BMD_CHUNK *last_chunk = window->u.topic.data->hot_spots;
                
                while (chunk == NULL)
                {
                    if ((last_chunk->next == NULL)
                        || (last_chunk->next == window->u.topic.selected_chunk)
                        )
                    {
                        chunk = last_chunk;
                    } 
                    else 
                    {
                        last_chunk = last_chunk->next;
                    }
                }
            }
            else if (strcmp(params[0],"NextHotSpot") == 0 )
            {
                if (window->u.topic.selected_chunk 
                    && (window->u.topic.selected_chunk->next)
                    ) 
                {
                    chunk = window->u.topic.selected_chunk->next;
                } 
                else 
                {
                    chunk =  window->u.topic.data->hot_spots;
                }
            }
            if (chunk) 
            {
                Arg	  arglist[20];
                int       argcnt;
                Dimension window_width;
                Dimension window_height;
                int       new_offset;
                
                argcnt = 0;
                SET_ARG( XmNwidth, &window_width );
                SET_ARG( XmNheight, &window_height );
                XtGetValues( window->widgets[W_WINDOW], arglist, argcnt );

                /* If the hotspot is not completely visible then try to center it
                 * in the window.
                 */
                if (((chunk->rect.left + window->u.topic.x) < 0) 
                    || ((chunk->rect.right + window->u.topic.x) > window_width)
                    || ((chunk->rect.top + window->u.topic.y) < 0)
                    || ((chunk->rect.bottom + window->u.topic.y) > window_height)
                    )
                {
                    /* Make sure no graphics operations are pending 
                     */
                    window->u.topic.grop_pending = FALSE;
                    
                    /* Calculate the new x offset for the window.
                     */
                    new_offset = chunk->rect.left - (window_width/2) - (chunk->rect.width/2);
                    if (new_offset > 0) {
                        new_offset -= new_offset % bkr_default_space_width;
                        window->u.topic.internal_x = -new_offset;
                    }
                    else 
                    {
                        window->u.topic.internal_x = 0;
                    }
                    
                    /* Calculate the new y offset for the window.
                     */
                    new_offset = chunk->rect.top - (window_height/2) - (chunk->rect.height/2);
                    if (new_offset > 0) {
                        new_offset -= new_offset % bkr_default_line_height;
                        window->u.topic.internal_y = -new_offset;
                    }
                    else 
                    {
                        window->u.topic.internal_y = 0;
                    }
                    
                    /* Update the display!
                     */
                    bkr_scroll_adjust_display( window );
                    bkr_scroll_adjust_scrollbars(window,
                                                 window->u.topic.internal_x, 
                                                 window->u.topic.internal_y, 
                                                 window->u.topic.width,
                                                 window->u.topic.height );
                }
                /* Select the hotspot
                 */
                bkr_button_select_hot_spot( window, chunk, SELECT );
            }
        }
    }
};  /* end of bkr_topic_keyboard_actions */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_focus_highlight
**
** 	Turns the highlighting of the draing window on and off.
**
**  FORMAL PARAMETERS:
**
**	window        - pointer to the topic window
**	highlight_on  - turn the highlight on or off
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
bkr_topic_focus_highlight PARAM_NAMES((window,highlight_on))
    BKR_WINDOW *window PARAM_SEP
    Boolean highlight_on PARAM_END
{
    if (bkrplus_g_charcell_display) 
    {
        static Boolean got_colors = FALSE;
        static Pixel on_color;
        static Pixel off_color;

        if (got_colors == FALSE) {
            off_color = WhitePixel(bkr_display,0);
            on_color = BlackPixel(bkr_display,0);
            got_colors = TRUE;
        }
        XtVaSetValues(window->widgets[W_SCROLLED_WINDOW],
                      XcmNborderForegroundColor,
                      (XtArgVal)((highlight_on) ? on_color : off_color),
                      XcmNborderBackgroundColor,
                      (XtArgVal)((highlight_on) ? off_color : on_color),
                      NULL);
    }
    else 
    {
        Pixmap border ;

        border = (highlight_on) ? window->u.topic.drawing_win_highlight_on
                                : window->u.topic.drawing_win_highlight_off;
       
        XtVaSetValues(window->widgets[W_WINDOW],
                    XmNborderPixmap,
                    (XtArgVal)border,
                    NULL);

    }
} /* end bkr_topic_focus_highlight */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_focus
**
** 	Focus action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xevent	        - X event associated with call
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
bkr_topic_focus PARAM_NAMES((widget,event,params,num_params))
    Widget		    widget PARAM_SEP
    XEvent  	            *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window = NULL;
    BMD_CHUNK               *chunk = NULL;

    window = bkr_window_find( widget );
    if ((window == NULL) 
        || (*num_params != 1)
        || (params[0] == NULL)
        || ((window->type != BKR_STANDARD_TOPIC) && (window->type != BKR_FORMAL_TOPIC))
        ) {
    	return;
    }

    if (strcmp(params[0],"In") == 0) 
    {
        /* Gaining focus in the drawing window so set the border color
         * to be the highlight color and/or pixmap.         */
        bkr_topic_focus_highlight(window,TRUE);
    }
    else if (strcmp(params[0],"Out") == 0) 
    {
        /* Losing focus in the drawing window, so change the border
         * back to the highlight off pixmap which should match the background.
         */
        bkr_topic_focus_highlight(window,FALSE);
    }
    else if (strcmp(params[0],"Grab") == 0) 
    {
        /* We grab or change focus to the drawing window, by
         * traversing to it.  We don't bother to do this if there
         * are no hotspots.
         */
        XmProcessTraversal(widget,XmTRAVERSE_CURRENT);
    }
};  /* end of bkr_topic_focus */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_btn_box_focus
**
** 	Focus action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xevent	        - X event associated with call
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
bkr_topic_btn_box_focus PARAM_NAMES((widget,event,params,num_params))
    Widget		    widget PARAM_SEP
    XEvent  	            *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window = NULL;
    BMD_CHUNK               *chunk = NULL;
    Arg     	    	    arg;

    window = bkr_window_find( widget );
    if ((window == NULL) 
        || (*num_params != 1)
        || ((window->type != BKR_STANDARD_TOPIC) && (window->type != BKR_FORMAL_TOPIC))
        || (window->u.topic.data->hot_spots == NULL) 
        ) {
    	return;
    }

    if (strcmp(params[0],"In") == 0) 
    {
        /* The button box just got focus, traverse to the Next Screen
         * button instead of the Close button.
         */
        XmProcessTraversal(window->widgets[ W_NEXT_SCREEN_BUTTON],XmTRAVERSE_CURRENT);
    }
};  /* end of bkr_topic_btn_box_focus */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	remove_history_list_entry
**
** 	Removes the first entry from the history list queue
**  	for a given Topic window.
**
**  FORMAL PARAMETERS:
**
**	topic - pointer to the Topic window which contains the history list.
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
**	Virtual memory is freed.
**
**--
**/
static void
remove_history_list_entry PARAM_NAMES((window))
    BKR_WINDOW	*window PARAM_END
{
    BKR_BACK_TOPIC  	*back_topic = window->u.topic.back_topic;

    if ( back_topic == NULL )
        return;

    window->u.topic.back_topic = back_topic->next;
    BKR_FREE( back_topic );

    if ( window->u.topic.back_topic == NULL )
    {
    	if ( window->widgets[W_VIEW_MENU] != NULL )
    	{
	    XtSetSensitive( window->widgets[W_GOBACK_ENTRY],  FALSE );
    	}
    	XtSetSensitive( window->widgets[W_GOBACK_BUTTON], FALSE );
    }

};  /* end of remove_history_list_entry */



