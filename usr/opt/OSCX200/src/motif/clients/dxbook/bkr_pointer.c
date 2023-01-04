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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_POINTER.C*/
/* *9    22-MAR-1993 14:37:51 BALLENGER "Fix problems with polygon hotspots."*/
/* *8    24-FEB-1993 17:47:24 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *7     5-AUG-1992 21:41:35 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *6    20-JUL-1992 13:49:16 BALLENGER "Character cell support"*/
/* *5     9-JUN-1992 09:57:50 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 17:01:35 KARDON "UCXed"*/
/* *3     2-JAN-1992 09:56:25 FITZELL "Xbook Reference foe OAF level B"*/
/* *2    17-SEP-1991 21:27:34 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:01 PARMENTER "Pointer motion for the drawing window"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_POINTER.C*/
#ifndef VMS
 /*
#else
#module BKR_POINTER "V03-0000"
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
**	Pointer motion and leave window event handling callbacks for the drawing window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     13-Apr-1990
**
**  MODIFICATION HISTORY:
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
#include "bkr_pointer.h"     /* function prototypes for .c module */
#include "bkr_window.h"      /* Window manipulation routines */


/*
 * FORWARD DEFINITIONS 
 */

static int  	    	    text_font_ascent;
static int  	    	    text_font_descent;
static int  	    	    text_line_spacing;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_pointer_leave_window
**
** 	LeaveWindow action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xevent	    	- X event associated with LeaveWindow event 
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
bkr_pointer_leave_window PARAM_NAMES((window_widget,xevent,params,num_params))
    Widget		    window_widget PARAM_SEP
    XEvent  	    	    *xevent PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_SHELL	    *shell;
    BKR_WINDOW	    *window;

    window =  bkr_window_find(window_widget);
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
    	    if ( window->u.topic.outlined_chunk != NULL )
	    	bkr_pointer_outline_hot_spot( window, window->u.topic.outlined_chunk, OFF );
    	    break;
    }

};  /* end of bkr_pointer_leave_window */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_pointer_motion
**
** 	Motion action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	event	    	- X event associated with the Motion event
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
bkr_pointer_motion PARAM_NAMES((window_widget,event,params,num_params))
    Widget		    window_widget PARAM_SEP
    XMotionEvent  	    *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW	    *window;
    BMD_CHUNK	    *chunk;
    Arg     	    arglist[5];
    int	    	    argcnt;

    window = bkr_window_find( window_widget );
    if (window
        && ((window->type == BKR_STANDARD_TOPIC)
            || (window->type == BKR_FORMAL_TOPIC)
            )
        && ( ! window->u.topic.show_hot_spots )
        )
    {
        /* Calculate the x and y for the point relative
         * to what part of the topic is being displayed.
         */
        int x = event->x - window->u.topic.x;
        int y = event->y - window->u.topic.y;

        chunk = window->u.topic.data->hot_spots;
        while (chunk) 
        {
            /* See if the point falls within the chunk.
             */
            if ((x >= chunk->rect.left) && (x <= chunk->rect.right)
                && (y >= chunk->rect.top) && (y <= chunk->rect.bottom)
                ) 
            {
                if (chunk->region) 
                {
                    if (XPointInRegion(chunk->region,x-chunk->rect.left,y-chunk->rect.top))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            chunk = chunk->next;
        }
        /* Already outlined ?
         */
        if ( window->u.topic.outlined_chunk != chunk )
        {  
            if (window->u.topic.outlined_chunk) 
            {   
                /* Remove current hot spot 
                 */
                bkr_pointer_outline_hot_spot(window,window->u.topic.outlined_chunk,OFF);
            }
            
            /* Outline hot spot
             */
            if (chunk) {
                bkr_pointer_outline_hot_spot( window, chunk, ON );
            }
        }
    }
};  /* end of bkr_pointer_motion */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_pointer_outline_hot_spot
**
** 	Draws or removes the outline around a hot spot in the Topic window.
**
**  FORMAL PARAMETERS:
**
**  	topic	    - pointer to the Topic window.
**  	chunk	    - pointer to the chunk which contains the hot spot.
**  	outline	    - Boolean: whether to outline or remove the outline from
**  	    	    	the hot spot.
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
bkr_pointer_outline_hot_spot PARAM_NAMES((window,chunk,outline))
    BKR_WINDOW	*window PARAM_SEP
    BMD_CHUNK   *chunk PARAM_SEP
    Boolean 	outline PARAM_END
{
    Widget  drawing_window = window->widgets[W_WINDOW];
    GC	    outline_gc;
    int     x_offset = window->u.topic.x + chunk->rect.left;
    int     y_offset = window->u.topic.y + chunk->rect.top;

    if ( outline )
    {
    	if ( window->u.topic.show_hot_spots )
    	    outline_gc = bkr_text_gc;
    	else
    	{
    	    outline_gc = bkr_xor_gc;
	    window->u.topic.outlined_chunk = chunk;
    	}
    }
    else
    {
    	outline_gc = bkr_xor_gc;
    	window->u.topic.outlined_chunk = NULL;
    }

    if (chunk->segments) 
    {
        int i;
        for (i = 0; i < chunk->n_segments ; i++) {
            chunk->segments[i].x1 += x_offset;
            chunk->segments[i].x2 += x_offset;
            chunk->segments[i].y1 += y_offset;
            chunk->segments[i].y2 += y_offset;
        }

        XDrawSegments(bkr_display,
                      XtWindow(drawing_window),
                      outline_gc,
                      chunk->segments,
                      chunk->n_segments);

        for (i = 0; i < chunk->n_segments ; i++) {
            chunk->segments[i].x1 -= x_offset;
            chunk->segments[i].x2 -= x_offset;
            chunk->segments[i].y1 -= y_offset;
            chunk->segments[i].y2 -= y_offset;
        }
    }
    else if ( chunk->chunk_type == BMD_REFERENCE_POLY )
    {
	int	    point;
        
        for ( point = 0; point < chunk->num_points; point++ )
        {
            chunk->xpoint_vec[point].x =  chunk->point_vec[point].x + window->u.topic.x;
            chunk->xpoint_vec[point].y =  chunk->point_vec[point].y + window->u.topic.y;
        }
        chunk->xpoint_vec[chunk->num_points].x = chunk->xpoint_vec[0].x;
        chunk->xpoint_vec[chunk->num_points].y = chunk->xpoint_vec[0].y;

	XDrawLines(
	    bkr_display, 
	    XtWindow( drawing_window ),	/*  window  	*/
	    outline_gc,	    	    	/*  GC      	*/
	    chunk->xpoint_vec,      	/*  points  	*/
	    chunk->num_points+1,      	/*  num_points  */
	    CoordModeOrigin );      	/*  mode    	*/
    }
    else
	XDrawRectangle(
	    bkr_display,
	    XtWindow( drawing_window ),	/*  window  */
            outline_gc,	    	    	/*  GC	    */
	    x_offset,
    	    y_offset,
	    chunk->rect.width,  	/*  width   */
    	    chunk->rect.height );    	/*  height  */

};  /* end of bkr_pointer_outline_hot_spot */









