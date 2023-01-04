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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SCROLL.C*/
/* *6     5-AUG-1992 21:53:24 BALLENGER "add bkr_scroll_max_y routine"*/
/* *5    20-JUL-1992 13:49:31 BALLENGER "Character cell support"*/
/* *4     9-JUN-1992 09:58:12 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *3     3-MAR-1992 17:03:09 KARDON "UCXed"*/
/* *2    18-SEP-1991 14:15:57 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:15 PARMENTER "Scroll bars"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SCROLL.C*/
#ifndef VMS
 /*
#else
#module BKR_SCROLL "V03-0001"
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
**	Scroll bar callbacks and Preview window routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     2-Feb-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0002    DLD0001     David L Ballenger       6-Feb-1991
**                  Fix "statement not reached" compilation warning.
**
**	V03-0001    JAF0001	James A. Ferguson   	2-Feb-1990
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
#include "bkr_scroll.h"      /* function prototypes for .c module */
#include "bkr_topic_init.h"  /* Topic window initialization */



/*
 * FORWARD DEFINITIONS
 */
static char *   	    show_callback_reason(); /* debugging */
static Boolean	    	    ignore_scrollbar_callback = FALSE;
static Boolean 	    	    debug_callbacks = FALSE;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_scroll_adjust_display 
**
** 	Moves the contents of a window either up or down,
**  	or left or right.
**
**  FORMAL PARAMETERS:
**
**  	window	- pointer to the the window whose contents is to be moved.
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
bkr_scroll_adjust_display PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    Arg	    	    	    arglist[20];
    int	    	    	    argcnt;
    Dimension	    	    window_width;
    Dimension	    	    window_height;
    int	    	    	    *x;
    int	    	    	    *y;
    int	    	    	    *internal_x;
    int	    	    	    *internal_y;
    Boolean 	    	    *grop_pending;

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    x = (int *) &window->u.topic.x;
    	    y = (int *) &window->u.topic.y;
    	    internal_x = (int *) &window->u.topic.internal_x;
    	    internal_y = (int *) &window->u.topic.internal_y;
    	    grop_pending = (Boolean *) &window->u.topic.grop_pending;
    	    break;
    }

    argcnt = 0;
    SET_ARG( XmNwidth, &window_width );
    SET_ARG( XmNheight, &window_height );
    XtGetValues( window->widgets[W_WINDOW], arglist, argcnt );

    /* Move the contents */

    XCopyArea(
        bkr_display,
        XtWindow( window->widgets[W_WINDOW] ), 	/* source */
        XtWindow( window->widgets[W_WINDOW] ), 	/* destintation */
        bkr_text_gc,
    	*x - *internal_x,	    	    	/* src x  */
        *y - *internal_y,	    	    	/* src y  */
        window_width,	    	    	    	/* width  */
        window_height,	    	    	    	/* height */
        0, 0 );	    	    	    	    	/* dest. x, y */

    *x = *internal_x;
    *y = *internal_y;

    /*  Generate a NoExpose event to prevent overrun  */

    XCopyArea(
        bkr_display,
        XtWindow( window->widgets[W_WINDOW] ), 	 /* source */
        XtWindow( window->widgets[W_WINDOW] ), 	 /* destintation */
        bkr_text_gc,
        0, 0, 0, 0, 0, 0 );

    *grop_pending = TRUE;

};  /* end of bkr_scroll_adjust_display */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_scroll_adjust_scrollbars
**
** 	Adjusts the size of the scroll bar slider, this includes the values
**  	for shown, value, maxValue, increment and PageIncrement.
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
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_scroll_adjust_scrollbars PARAM_NAMES((window,new_x,new_y,new_max_x,new_max_y))
    BKR_WINDOW	    *window PARAM_SEP
    int	    	    new_x PARAM_SEP
    int	    	    new_y PARAM_SEP
    int	    	    new_max_x PARAM_SEP
    int	    	    new_max_y PARAM_END

{
    Widget    	    	    hscroll;
    Widget    	    	    vscroll;
    Arg     	    	    arglist[20];
    int	    	    	    argcnt;
    int	    	    	    horizontal_increment;
    int	    	    	    vertical_increment;
    int	    	    	    horizontal_page_increment;
    int	    	    	    vertical_page_increment;
    int	    	    	    horizontal_max_value;
    int	    	    	    vertical_max_value;
    int	    	    	    new_x_value;
    int	    	    	    new_y_value;
    int	    	    	    max_y;
    Dimension	    	    window_width;
    Dimension	    	    window_height;

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;
    	    break;

    	case BKR_STANDARD_TOPIC  :
    	case BKR_FORMAL_TOPIC :
    	    hscroll = (Widget) window->widgets[W_HSCROLL_BAR];
    	    vscroll = (Widget) window->widgets[W_VSCROLL_BAR];
    	    argcnt = 0;
    	    SET_ARG( XmNwidth, &window_width );
    	    SET_ARG( XmNheight, &window_height );
    	    XtGetValues( window->widgets[W_WINDOW], arglist, argcnt );

            if (bkrplus_g_charcell_display) 
            {
                horizontal_increment = bkr_default_space_width;
                vertical_increment = bkr_default_line_height;
            }
            else 
            {
                horizontal_increment = TOPIC_UNIT_INCREMENT;
                vertical_increment = TOPIC_UNIT_INCREMENT;
            }
    	    horizontal_page_increment = (int) window_width - horizontal_increment;
    	    vertical_page_increment = (int) window_height - vertical_increment;
    	    horizontal_max_value = MAX( new_max_x, (int) window_width );
	    if ( window->type == BKR_FORMAL_TOPIC )
	      vertical_max_value = MAX( new_max_y, (int) window_height );
            else
	    {
    	      /* Round to the next full screen - only for STANDARD TOPICS */
    	      max_y = new_max_y;
	      if ( new_max_y > (int) window_height )
    	      {
                  max_y = bkr_scroll_max_y( window->u.topic.height, window_height );
    	      }
	      vertical_max_value = MAX( max_y, (int) window_height );
	    }	
    	    break;
    }

    /* 
     *  KLUDGE: 17-Jul-1989 	James A. Ferguson
     *  This is a major kludge to avoid a "value changed callback" when we
     *  do an XtSetValues to modify the resource XmNmaximum of each scroll bar.
     *  We can't use XmScrollBarSetValues because it won't let us set the
     *  resource XmNmaximum.  
     */

    if ( hscroll != NULL )
    {
    	/*  The value of XmNvalue resource must be less than or equal to 
    	 *  (XmNmaximum - XmNsliderSize) and greater than or equal to XmNminimum,
    	 *  otherwise we get toolkit warnings!  We negate "new_x" because its
    	 *  value could be negative.
    	 */
    	new_x_value = MIN( - new_x, horizontal_max_value - (int) window_width );
    	new_x_value = MAX( 0, new_x_value );

    	ignore_scrollbar_callback = TRUE;
	argcnt = 0;
	SET_ARG( XmNvalue,  	    new_x_value ); 
    	SET_ARG( XmNminimum, 	    0 );
	SET_ARG( XmNmaximum, 	    horizontal_max_value ); 
	SET_ARG( XmNsliderSize,     window_width ); 
    	SET_ARG( XmNincrement, 	    horizontal_increment );
	SET_ARG( XmNpageIncrement,  horizontal_page_increment );
	XtSetValues( hscroll, arglist, argcnt );
    }
    
    if ( vscroll != NULL )
    {
    	/*  The value of XmNvalue resource must be less than or equal to 
    	 *  (XmNmaximum - XmNsliderSize) and greater than or equal to XmNminimum,
    	 *  otherwise we get toolkit warnings!  We negate "new_y" because its
    	 *  value could be negative.
    	 */
    	new_y_value = MIN( - new_y, vertical_max_value - (int) window_height );
    	new_y_value = MAX( 0, new_y_value );

    	ignore_scrollbar_callback = TRUE;
	argcnt = 0;
	SET_ARG( XmNvalue,  	    new_y_value );
    	SET_ARG( XmNminimum, 	    0 );
	SET_ARG( XmNmaximum, 	    vertical_max_value );
	SET_ARG( XmNsliderSize,     (int) window_height ); 
    	SET_ARG( XmNincrement, 	    vertical_increment );
	SET_ARG( XmNpageIncrement,  vertical_page_increment );
	XtSetValues( vscroll, arglist, argcnt );
    }

    /* Re-enable scrollbar callbacks */

    ignore_scrollbar_callback = FALSE;

};  /* end of bkr_scroll_adjust_scrollbars */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_scroll_horizontal_callback
**
** 	Horizontal scroll bar callback routine.
**
**  FORMAL PARAMETERS:
**
**	hscrollbar   	- id of the scroll bar widget that caused the event
**	shell	    	- pointer to the shell data structure
**	callback_data	- pointer to callback data
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
bkr_scroll_horizontal_callback PARAM_NAMES((hscrollbar,window,callback_data))
    Widget    	    	    	hscrollbar PARAM_SEP
    BKR_WINDOW	    	    	*window PARAM_SEP
    XmScrollBarCallbackStruct	*callback_data PARAM_END

{
    Arg     	    	    arglist[20];
    int     	    	    argcnt = 0;
    int	    	    	    old_x;
    int     	    	    new_x;
    int	    	    	    *internal_x;
    Boolean 	    	    grop_pending;

    /* Modifying scrollbar values; ignore callback */

    if ( ignore_scrollbar_callback )
    	return;

    if ( debug_callbacks ) 
	printf( "hscrollbar callback %s %d.\n", 
	    show_callback_reason( callback_data->reason ),
	    callback_data->value );

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;
    	    break;

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    old_x = window->u.topic.x;
    	    internal_x = (int *) &window->u.topic.internal_x;
    	    grop_pending = window->u.topic.grop_pending;
    	    break;
    }	/* end switch */

    if ( ( callback_data->reason == XmCR_TO_TOP )
    	 || ( callback_data->reason == XmCR_TO_BOTTOM ) )
    {
    	new_x = - callback_data->value;

    	/* Set the slider because the callback doesn't */

    	ignore_scrollbar_callback = TRUE;
	argcnt = 0;
	SET_ARG( XmNvalue, callback_data->value );
	XtSetValues( hscrollbar, arglist, argcnt );
    	ignore_scrollbar_callback = FALSE;
    }
    else    /* All other callbacks are handled here */
	new_x = - callback_data->value;

    /* Update internal value with changes */

    *internal_x = new_x;

    /* Return if waiting for a NoExpose, otherwise adjust the display */

    if ( grop_pending )
	return;
    else if ( old_x != new_x )
	bkr_scroll_adjust_display( window );

};  /* end of bkr_scroll_horizontal_callback */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_scroll_vertical_callback
**
** 	Vertical scroll bar callback routine.
**
**  FORMAL PARAMETERS:
**
**	vscrollbar   	- id of the scroll bar widget that caused the event
**	shell	    	- pointer to the shell data structure
**	callback_data	- pointer to callback data
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
bkr_scroll_vertical_callback PARAM_NAMES((vscrollbar,window,callback_data))
    Widget    	    	    	vscrollbar PARAM_SEP
    BKR_WINDOW	    	    	*window PARAM_SEP
    XmScrollBarCallbackStruct	*callback_data PARAM_END


{
    Boolean 	    	    grop_pending;
    Arg     	    	    arglist[20];
    int     	    	    argcnt;
    int     	    	    top_id;
    int	    	    	    old_y;
    int     	    	    new_y;

    /* Modifying scrollbar values; ignore callback */

    if ( ignore_scrollbar_callback )
    	return;

    if ( debug_callbacks ) 
	printf( "vscroll callback %s %d. \n", 
	    show_callback_reason( callback_data->reason ),
	    callback_data->value );

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;
    	    break;

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    grop_pending = window->u.topic.grop_pending;
    	    old_y = window->u.topic.y;
	    if ( ( callback_data->reason == XmCR_TO_TOP )   ||
    	    	 ( callback_data->reason == XmCR_TO_BOTTOM ) )
    	    {
	    	new_y = - callback_data->value;

    	    	/* Set the slider because the callback doesn't */

    	    	ignore_scrollbar_callback = TRUE;
	    	argcnt = 0;
	    	SET_ARG( XmNvalue, callback_data->value );
	    	XtSetValues( vscrollbar, arglist, argcnt );
    	    	ignore_scrollbar_callback = FALSE;
    	    }
    	    else    /* All other callbacks are handled here */
	    	new_y = - callback_data->value;

    	    /* Update internal value with changes */

    	    window->u.topic.internal_y = new_y;

    	    break;  /* end case BKR_STANDARD_TOPIC and BKR_FORMAL_TOPIC */
    }	/* end switch */

    /* Return if waiting for a NoExpose, otherwise adjust the display */

    if ( grop_pending )
	return;
    else if ( old_y != new_y )
	bkr_scroll_adjust_display( window );

    bkr_screen_init_sensitivity( window );

};  /* end of bkr_scroll_vertical_callback */

int
bkr_scroll_max_y PARAM_NAMES((topic_height,window_height))
    int topic_height PARAM_SEP
    int window_height PARAM_END
/*
 *
 * Function description:
 *
 *      Calculate the maximum y value for a vertical scroll bar in the topic
 *      window.
 *
 * Arguments:
 *
 *      topic_height  - height of the topic
 *      window_height - height of the window
 *
 * Return value:
 *
 *      The maximum y value for the scroll bar.
 *
 * Side effects:
 *
 *      None
 *
 */
{
    int screen_height;
    int num_screens;

    if (bkrplus_g_charcell_display) 
    {
        screen_height = window_height - bkr_default_line_height;
    }
    else 
    {
        screen_height = window_height - TOPIC_UNIT_INCREMENT;
    }

    num_screens = (topic_height+screen_height) / screen_height;
    return (num_screens * screen_height);
}
   	    	
/*
 *  debugging routines
 */

static void
print_callback_reason( reason )
    int reason;
{
    char    *string;
    string = show_callback_reason( reason );

    if ( ( string != NULL ) && ( string[0] != NULLCHAR ) )
    	printf( " callback reason = %s\n", string );
};

static char * 
show_callback_reason( reason )
    int reason;
{
    switch (reason)
    {
    	case XmCR_NONE :    	    	    return "XmCR_NONE";
    	case XmCR_HELP :    	    	    return "XmCR_HELP";
    	case XmCR_VALUE_CHANGED :   	    return "XmCR_VALUE_CHANGED";
    	case XmCR_INCREMENT :	    	    return "XmCR_INCREMENT";
    	case XmCR_DECREMENT :	    	    return "XmCR_DECREMENT";
    	case XmCR_PAGE_INCREMENT :  	    return "XmCR_PAGE_INCREMENT";
    	case XmCR_PAGE_DECREMENT :  	    return "XmCR_PAGE_DECREMENT";
    	case XmCR_TO_TOP :  	    	    return "XmCR_TO_TOP";
    	case XmCR_TO_BOTTOM :	    	    return "XmCR_TO_BOTTOM";
    	case XmCR_DRAG :    	    	    return "XmCR_DRAG";
    	case XmCR_ACTIVATE :	    	    return "XmCR_ACTIVATE";
    	case XmCR_ARM :	    	    	    return "XmCR_ARM";
    	case XmCR_DISARM :  	    	    return "XmCR_DISARM";
    	case XmCR_MAP :	    	    	    return "XmCR_MAP";
    	case XmCR_UNMAP :   	    	    return "XmCR_UNMAP";
    	case XmCR_FOCUS :   	    	    return "XmCR_FOCUS";
    	case XmCR_LOSING_FOCUS :    	    return "XmCR_LOSING_FOCUS";
    	case XmCR_MODIFYING_TEXT_VALUE :    return "XmCR_MODIFYING_TEXT_VALUE";
    	case XmCR_MOVING_INSERT_CURSOR :    return "XmCR_MOVING_INSERT_CURSOR";
    	case XmCR_EXECUTE : 	    	    return "XmCR_EXECUTE";
    	case XmCR_SINGLE_SELECT :   	    return "XmCR_SINGLE_SELECT";
    	case XmCR_MULTIPLE_SELECT : 	    return "XmCR_MULTIPLE_SELECT";
    	case XmCR_EXTENDED_SELECT : 	    return "XmCR_EXTENDED_SELECT";
    	case XmCR_BROWSE_SELECT :   	    return "XmCR_BROWSE_SELECT";
    	case XmCR_DEFAULT_ACTION :  	    return "XmCR_DEFAULT_ACTION";
    	case XmCR_CLIPBOARD_DATA_REQUEST :  return "XmCR_CLIPBOARD_DATA_REQUEST";
    	case XmCR_CLIPBOARD_DATA_DELETE :   return "XmCR_CLIPBOARD_DATA_DELETE";
    	case XmCR_CASCADING :	    	    return "XmCR_CASCADING";
    	case XmCR_OK :	    	    	    return "XmCR_OK";
    	case XmCR_CANCEL :  	    	    return "XmCR_CANCEL";
    	case XmCR_APPLY :   	    	    return "XmCR_APPLY";
    	case XmCR_NO_MATCH :	    	    return "XmCR_NO_MATCH";
    	case XmCR_COMMAND_ENTERED : 	    return "XmCR_COMMAND_ENTERED";
    	case XmCR_COMMAND_CHANGED : 	    return "XmCR_COMMAND_CHANGED";
    	case XmCR_EXPOSE :  	    	    return "XmCR_EXPOSE";
    	case XmCR_RESIZE :  	    	    return "XmCR_RESIZE";
    	case XmCR_INPUT :   	    	    return "XmCR_INPUT";
    	case XmCR_GAIN_PRIMARY :    	    return "XmCR_GAIN_PRIMARY";
    	case XmCR_LOSE_PRIMARY :    	    return "XmCR_LOSE_PRIMARY";
    }

    return "Unknown Xm callback reason";

};  /* end of show_callback_reason */



