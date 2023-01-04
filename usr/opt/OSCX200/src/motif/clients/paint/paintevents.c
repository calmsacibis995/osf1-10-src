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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/paintevents.c,v 1.1.2.3 92/12/11 08:35:23 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This module contains rountines that handle events.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      10/5/88
**      changed the way Polygons are terminated.
**
**      dl      10/6/88
**      display error message if nothing is drawn due to null fill
**
**      jj      11/22/88
**      paste bug fix
**
**      dl      11/18/88
**      check for outline pattern when using brush
**
**--
**/           
#include "paintrefs.h"
#include <X11/keysym.h>

static int xpt1, ypt1;
static int polygon_done;         
static int save_brush, save_brush_index;
static int move_cursor = FALSE;
static int skip_zoom_refresh = FALSE;
static int prv_x = -1, prv_y = -1;

/*
 *
 * ROUTINE:  Timer_Alarm
 *
 * ABSTRACT: 
 *
 *  Handles a timer event 
 *
 */
XtTimerCallbackProc Timer_Alarm( closure, id )
Opaque closure;
XtIntervalId *id;
{
	if( (current_action == SPRAYCAN) && rbanding ){
		Rband_Spray();
		XtAddTimeOut( 250, (XtTimerCallbackProc)Timer_Alarm, NULL );
		}
/*
		XtRemoveTimeOut( spray_timer );
*/

	if( select_on )
		Blink_Highlight();
}



/* motion events are always on if zoom window is up */
static int motion_events_on = FALSE;
/*
 *
 * ROUTINE:  Start_Motion_Events
 *
 * ABSTRACT: 
 *
 *  Get all mouse movements
 *
 */
void Start_Motion_Events(w)
Widget w;
{
    if ((w == picture_widget) && !motion_events_on) {
	Fetch_Set_Attribute (w, XmNtranslations, "motion_translation_table");
	XtInstallAllAccelerators (picture_widget, main_widget);
	motion_events_on = TRUE;
    }
}


/*
 *
 * ROUTINE:  Stop_Motion_Events
 *
 * ABSTRACT: 
 *
 *  Stop getting all mouse movements
 *
 */
void Stop_Motion_Events(w)
Widget w;
{
    if (!zoomed && !select_on) {
	if ((w == picture_widget) && motion_events_on) {
	    Fetch_Set_Attribute( w, XmNtranslations, "main_translation_table");
	    XtInstallAllAccelerators (picture_widget, main_widget);
	    motion_events_on = FALSE;
	}
    }
}


/*
 *
 * ROUTINE:  Begin_Action
 *
 * ABSTRACT: 
 *
 *  Call the appropriate routine to begin the current action
 *
 */
void Begin_Action()
{        
XPoint pts[2];
int tmp_x, tmp_y;


	Init_Rband();		/* initialize the rubber banding variables */

/* If the user has clicked to begin rubber banding and has the shift key 
	depressed, give the user the constrained action(if one exists),
	if the ctrl key is pressed, use the current brush to draw the shapes
	outline
 */
	alt_shape = FALSE;
	if( shift_key_down || (current_action == SQUARE) || 
	   (current_action == CIRCLE) )
		alt_shape = TRUE;
	else
		alt_shape = FALSE;
         
	switch ( current_action ) {

		case POLYGON : {
			Save_Picture_State ();
			Start_Motion_Events (ifocus);  /* jj-1/2089 */
			polygon_done = FALSE;
			break;
			}             
      
		case ERASE : {
			Save_Picture_State();
			save_brush = cur_brush;
			save_brush_index = cur_brush_index;
			cur_brush = SQUARE_BRUSH;
			cur_brush_index = 2;
			Rband_Brush();
			break;                   
			}

		case RECTANGLE :
		case SQUARE : {
			Rband_Rectangle();
			break;
			}

		case ELLIPSE :
		case CIRCLE : {	
			Rband_Ellipse();
			break;
			}

		case STROKE :{
			Save_Picture_State();
		 	Rband_Stroke();
			break;                   
			}

		case BRUSH : {
			Save_Picture_State();
			Rband_Brush();
			break;                   
			}

		case SPRAYCAN : {
			Save_Picture_State();
			Rband_Spray();
			break;
			}

		case ZOOM_MOVE : {
			UnHighlight_Zoom();
			Rband_Zoom_Move();
			break;
		}

		case SELECT_AREA : {
			xpt1 = cur_x;
			ypt1 = cur_y;
			break;
			}

		case PENCIL : {
			Save_Picture_State();
			Rband_Pencil();
		 	break;                   
			}
      
		case TEXT : {
			Begin_Text();
		 	break;
			}

		case ZOOM : {
			Begin_Zoom( points[0].x, points[0].y );
			break;
			}

		case FLOOD : {
			Flood_Fill( points[0].x, points[0].y );
			break;                                 
			}

		case DROPPER : {
			Pickup_Color (points[0].x, points[0].y);
			break;
			}

	}          
}

/*
 *
 * ROUTINE:  End_Action
 *
 * ABSTRACT: 
 *
 *  Call the appropriate routine to end the current action
 *
 */      
void End_Action()
{                            
int x, y, width, height;
int i;
extern double Distance();
int tmp_x, tmp_y;
int close_polygon;
int side;
                                 
/*
 * No longer rubberbanding, this may be changed if a POLYGON is not completed.
*/

    rbanding = FALSE;

    switch (current_action) {
	case LINE :
	    Save_Point (rband_x, rband_y );
	    Erase_Rband ();
	    Draw_Line ();
	    break;
      
	case STROKE :
	    Draw_Stroke ();
	    break;

	case SQUARE :
	case RECTANGLE :
	    Save_Point (rband_x, rband_y);
	    Erase_Rband ();
	    Draw_Rectangle ();
	    break;

	case ARC :
	    Save_Point (rband_x, rband_y);
	    Erase_Rband ();
	    Draw_Arc ();
	    break;

	case SPRAYCAN :
/* display a message if no drawing took place dl - 10/6/88 */
	    if( outline_stipple == 0 )  /* jj - 3/27/89 */
		Display_Message ("T_NO_OUTLINE_PATTERN");
	    else 
		Draw_Spray ();
	    break;

	case FLOOD :
	    if (zoomed)
		Update_Zoom();
/* display a message if no drawing took place dl - 10/6/88 */
	    if( fill_stipple == 0 )
		Display_Message ("T_NO_FILL_PATTERN");
	    if (flood_same_pattern) {	
	    	Display_Message ("T_FLOOD_HAS_NO_EFFECT");
		flood_same_pattern = FALSE;
	    }    
	    break;                                 

	case CIRCLE :
	case ELLIPSE :
	    Save_Point (rband_x, rband_y);
	    Erase_Rband ();
	    Draw_Ellipse ();
	    break;
     
	case POLYGON :
/* Save all upclicks after the first one */
	    Save_Point( rband_x, rband_y );

/* At least 3 points are necessary to create a polygon */
/*	    if( numpts > 2 ){  */
/*      
 * Distance between last two points must be less than 5.0 to finish the
 * polygon OR
 * if last point is close to first point - dl 10/5/88
 */
	    Erase_Rband();
	    if( Distance (points[0].x, points[0].y,
			  points[numpts-1].x, points[numpts-1].y ) < 
		MAX (5, cur_line_wd) ) {
		numpts -= 1;
		if (numpts > 1)
		    close_polygon = TRUE;
		else {
		    rband_x = anchor_x;
		    rband_y = anchor_y;			
		}
	    }
	    else
		close_polygon = FALSE;

	    if (numpts > 1) {	
		if ((Distance (points[numpts-2].x, points[numpts-2].y, 
			       points[numpts-1].x, points[numpts-1].y) < 5.0) ||
		    close_polygon) {

		    if (close_polygon) {
			points[numpts].x = points[0].x;
			points[numpts].y = points[0].y;
			++numpts;
		    }
		    else 
			numpts -= 1;
		    Stop_Motion_Events (ifocus); /* jj-1/20/89 */
/*
		    Fetch_Set_Attribute (ifocus, XmNtranslations, "main_translation_table");
*/
		    Draw_Polygon();  
		    polygon_done = TRUE;
		}
		else
		    polygon_done = FALSE;
	    } 
/*	    } */ /* numpts > 2 */

	    if (!polygon_done){
		Draw_Polyline ();
		anchor_x = rband_x;
		anchor_y = rband_y;
		rbanding = TRUE;
	    }
	    break;

	case ERASE :
	    Draw_Erase ();
	    cur_brush = save_brush;
	    cur_brush_index = save_brush_index;
	    break;

	case BRUSH :
/* display a message if no drawing took place dl - 11/18/88 */
            if (outline_stipple == 0)  /* jj - 3/27/89 */
		Display_Message ("T_NO_OUTLINE_PATTERN");
	    else 
		Draw_Brush ();	
	    break;
                      
	case PENCIL :
	    Draw_Pencil();
	    if (grid_on)
		Display_Grid (undo_x, undo_y, undo_width, undo_height);
	    break;
                      
	case SELECT_RECT :
	    Erase_Rband ();
	    if (paint_view == FULL_VIEW) {
		Select_Portion_Rect ();
		break;
	    }
/* if shift key was held down - make it a square */
	    if (alt_shape) {
		side = MAX (abs (rband_x - anchor_x), abs (rband_y - anchor_y));
		if (rband_x < anchor_x)
		    rband_x = anchor_x - side;
		else
		    rband_x = anchor_x + side;

		if (rband_y < anchor_y)
		    rband_y = anchor_y - side;
		else
		    rband_y = anchor_y + side;
	    }
/* Create polygonal select region, (first point was already saved) */
	    Save_Point (rband_x, anchor_y);
	    Save_Point (rband_x, rband_y);
	    Save_Point (anchor_x, rband_y);
	    Save_Point (anchor_x, anchor_y);
                                         
	    first_move = TRUE;                                 
	    select_rectangle = TRUE;
/*
	    Find_Highlight ();
*/
	    Select_Piece ();
	    break;
      
	case SELECT_AREA :
/* close off polygon */
	    Save_Point (xpt1, ypt1); /* close the stroke */
/*
	    Find_Highlight ();
	    Erase_Rband ();
*/
	    first_move = TRUE;
	    select_rectangle = FALSE;
/* hack so und select will always work properly */
	    prv_num_hipts = 0;
	    Select_Piece ();
	    break;

/*
	case TEXT :
	    Begin_Text ();
	    break;
*/
        
	case ZOOM_MOVE:
	    Erase_Rband ();

/* Recalculate zoom_xorigin and zoom_yorigin */
	    tmp_x = zoom_xorigin + (cur_x - anchor_x) + (zoom_width / 2);
	    tmp_y = zoom_yorigin + (cur_y - anchor_y) + (zoom_height / 2);
	    Find_Zoom_Rectangle (tmp_x, tmp_y, zoom_width,zoom_height);
			   
/* Refresh zoom window */
	    Refresh_Zoom_View (zoom_xorigin, zoom_yorigin, zoom_width, zoom_height);
	    Initialize_Magnifier ();
	    break;

	case MOVE :
	    Move_Area ();
	    break;

    }
    if (!rbanding) {
	if (numpts >= ALLOC_NUM)
	{
	    if (points != NULL) XtFree ((char *)points);
	    points = (XPoint *) XtMalloc (sizeof(XPoint)*ALLOC_NUM);
	    num_alloc = ALLOC_NUM;
	}
	numpts = 0;
    }

    if ((current_action == ZOOM_MOVE) || (current_action == MOVE)) {
	if (current_action == MOVE)
	    Increase_Change_Pix (select_x, select_y, select_width,
				 select_height);
	    current_action = prv_action;
	}
    else
	Increase_Change_Pix (undo_x, undo_y, undo_width, undo_height);

}    

/*
 *
 * ROUTINE:  Cancel_Action
 *
 * ABSTRACT: 
 *
 * In the middle of rubberbanding an action, the user may cancel
 *
 */
void Cancel_Action()
{
/*
	Erase_Rband();
	if( numpts > 0 )
	{
	    XtFree(points);
	    points = NULL;
	}
*/
}

/*
 *               
 * ROUTINE:  Move_Or_Reselect
 *
 * ABSTRACT: 
 *
 * Check to see if the current position is in the selected area.  If it
 * is initiate a move action.  If it is not, start a re-selection.
 *
 */
void Move_Or_ReSelect( w, x, y )
Widget w;
int *x, *y;
{
    if (! Within_Select_Area( w, *x, *y)){
	DeSelect(TRUE);
	Current_Position (x, y);
	if( select_rectangle )
	    current_action = SELECT_RECT;
	else
	    current_action = SELECT_AREA;
	Begin_Action();
    }
    else{
	rbanding = TRUE;
	prv_action = current_action;
	current_action = MOVE;
	rband_x = *x;
	rband_y = *y;

/* clear the undo grid */
	Clear_UG ();
	deselected = FALSE;		    

/* remove highlighting */
	Stop_Highlight_Blink();
/* Copy the area that was covered by the last move to the picture */
        Restore_Image (undo_move_map, PMX_TO_IX (select_x),
		       PMY_TO_IY (select_y), 0, 0, select_width,
                       select_height, Get_GC (GC_PD_COPY), FALSE);
    	undo_action = MOVE;
	if (zoomed)
	    if (ifocus != zoom_widget) 
		Refresh_Zoom_View (select_x, select_y, select_width,
				   select_height);
	Rband_Move();
    }        
}

     

/*
 *
 * ROUTINE:  Pressed_Button
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button down events
 *
 */
void Pressed_Button( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;
{

    if (event->state & ShiftMask)
	shift_key_down = TRUE;
    if (((w == picture_widget) && (event->x >= 0) && (event->y >= 0)) || 
	 (w == zoom_widget)) {
	XSetInputFocus (disp, XtWindow(w), RevertToParent, event->time);
	cur_x = event->x;
	cur_y = event->y;

	if (paint_view == FULL_VIEW) {
	    ifocus = w;	/* input focus */
	    if (sp_select_on)
		DeSelect_Portion_Rect ();
	    Begin_Action ();
	}
	else {
	    pixmap_changed = TRUE;
	    picture_changed = TRUE;
/* The user depressed a mouse button while in the picture window */
	    if (rbanding) {
		Current_Position( &cur_x, &cur_y );
		if( current_action != POLYGON ) /* enter polygon point on upclick */
		    End_Action();
	    }
	    else { /* not rbanding */
		ifocus = w;	/* input focus */
		if( zoomed && Within_Zoom_Region( w, cur_x, cur_y ) ){
		    prv_action = current_action;
		    current_action = ZOOM_MOVE;
		    zoom_move_xdist = 0;
		    zoom_move_ydist = 0;
		    Begin_Action();			
		}
		else {
		    if( select_on ) 
			Move_Or_ReSelect(w, &cur_x, &cur_y);
		    else{
			Current_Position( &cur_x, &cur_y );
			prv_x = cur_x;
			prv_y = cur_y;
			Begin_Action();
		    }
		}
	    }
	}
    }
    shift_key_down = FALSE;
}

/*
 *
 * ROUTINE:  Pressed_Button2
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button down events
 *  For now, (for MOTIF) this will only have an effect when clicking inside a
 *  selected piece (or the zoom magnifier) - in which case it will be the same 
 *  as MB1.
 *
 */
void Pressed_Button2( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;
{
    int zz = FALSE;	/* within zoom region */
    int ss = FALSE;	/* within selected area */

    if (paint_view == FULL_VIEW)
	return;
    if (rbanding)
	return;

    if (!zoomed && !select_on)
	return;

    if (((w == picture_widget) && (event->x >= 0) && (event->y >= 0)) ||
	 (w == zoom_widget)) {

	if (zoomed && Within_Zoom_Region (w, event->x, event->y)) {
	    zz = TRUE;
	}
	if (select_on)
	    if (Within_Select_Area (w, event->x, event->y))
		ss = TRUE;
	if (!zz && !ss)
	    return;

	XSetInputFocus (disp, XtWindow(w), RevertToParent, event->time);
	cur_x = event->x;
	cur_y = event->y;

	pixmap_changed = TRUE;
	picture_changed = TRUE;

/* The user depressed a mouse button while in the picture window */

	ifocus = w;	/* input focus */
	if (zz) {
	    prv_action = current_action;
	    current_action = ZOOM_MOVE;
	    zoom_move_xdist = 0;
	    zoom_move_ydist = 0;
	    Begin_Action();			
	}
	else {
	    Move_Or_ReSelect(w, &cur_x, &cur_y);
	}
    }
}


/*
 *
 * ROUTINE:  Released_Button
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button up events
 *                            
 */
void Released_Button (w)
    Widget  w;
{
    void End_Action();

    if (rbanding && button_down_mode && (w == ifocus)) {
	End_Action();
    }
}


/*
 *
 * ROUTINE:  Released_Button_1
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button 1 up events
 *                            
 */
void Released_Button_1 (w, event, params, num_params)
    Widget  w;
    XButtonReleasedEvent *event;
    char **params;
    int	num_params;
{
    Released_Button (w);
}


/*
 *
 * ROUTINE:  Released_Button_2
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button 2 up events
 *                            
 */
void Released_Button_2 (w, event, params, num_params)
    Widget  w;
    XButtonReleasedEvent *event;
    char **params;
    int	num_params;
{
    Released_Button (w);
}


/*
 *
 * ROUTINE:  Released_Button_3
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button 3 up events
 *                            
 */
void Released_Button_3 (w, event, params, num_params)
    Widget  w;
    XButtonReleasedEvent *event;
    char **params;
    int	num_params;
{
    Released_Button (w);
}



/*
 *
 * ROUTINE:  Moved_Mouse
 *
 * ABSTRACT: 
 *
 *  Handle Mouse movement events
 *
 */
void Moved_Mouse( w, event, params, num_params )
	Widget  w;
	XMotionEvent *event;
	char **params;
	int	num_params;
{
    XEvent nextevent;
    int more_motion;
    int echo;
    XMotionEvent *cur_event;
    int zmove, smove;

/* Compress the motion events */
    cur_event = event;	
    more_motion = TRUE;
    while (more_motion) {            
	if (XtPending ()) {
	    XtPeekEvent (&nextevent);
	    if (nextevent.type == MotionNotify) {
  		XtNextEvent (&nextevent);
  		cur_event = (XMotionEvent *)&nextevent; /* jj-port */
	    }
	    else
		more_motion = FALSE;
	}
	else
	    more_motion = FALSE;
    }		
/* Handle motion events only if rubberbanding.  Only echo the action if
 * the position has changed.
 */
/*
    XtNextEvent (&nextevent);
    cur_event = &nextevent;
*/
/* jj - 3/30/89  If event came from a different screen, ignore it. */
    if (event->same_screen) {
	if (rbanding) {
	    if (w == ifocus) {
		cur_x = event->x;
		cur_y = event->y;
		Current_Position (&cur_x, &cur_y);
		echo = FALSE;
		
/* Convert zoom coordinates to picture coordinates and check for change*/
		if (ifocus == zoom_widget){
		    if ((ZX_TO_PWX(cur_x) != ZX_TO_PWX(prv_x)) || 
			(ZY_TO_PWY(cur_y) != ZY_TO_PWY(prv_y)))
			echo = TRUE;
		}
		else {
/* Check for change in picture window */
		    if (ifocus == picture_widget) {
			if ((cur_x != prv_x) || (cur_y != prv_y))
			    echo = TRUE;
		    }
		}
/* If position has changed, echo the action */
		if (echo) {
		    Echo_Action ();
		    prv_x = cur_x;
		    prv_y = cur_y;
	    	}
	    }
	}	    
/* if not rubberbanding, check to see if the cursor has entered a
 * "sensitive area"- that is one that can be moved.
 */
	else {
	    cur_x = event->x;
	    cur_y = event->y;

/* set cursor if in zoom region */
	    zmove = FALSE;
	    if (zoomed)
		if (Within_Zoom_Region( w, cur_x, cur_y ))
		    zmove = TRUE;

/* set cursor if in select region */
	    smove = FALSE;
	    if (select_on)
		if (Within_Select_Area(w, cur_x, cur_y))
		    smove = TRUE;

	    if (zmove || smove) {
		if(!move_cursor) {
		    Set_Cursor (XtWindow(w), MOVE);
		    move_cursor = TRUE;
		}
	    }
	    else {
		if (move_cursor) {
		    Set_Cursor (XtWindow(w),current_action);
		    move_cursor = FALSE;
		}
	    }
	}
    }
}

/*
 *
 * ROUTINE:  Pressed_Key
 *
 * ABSTRACT: 
 *
 *  Handle keyboard key down events
 *
 */
/* Keyboard events */
void Pressed_Key( w, event, params, num_params )
	Widget  w;
	XKeyEvent *event;
	char **params;
	int	num_params;
{

    int XLS_Return;			
    char XLS_Buffer[2]; 			
    int XLS_MaxBytes = 3;                  
    KeySym XLS_KeySym;			
    static XComposeStatus XLS_ComposeStatus;	
    unsigned long charcode;

    XLS_Return = XLookupString(event, XLS_Buffer, XLS_MaxBytes, &XLS_KeySym,
			       &XLS_ComposeStatus);

    charcode = XLS_KeySym ;
    if ((charcode == XK_Shift_L) || (charcode == XK_Shift_R))
	shift_key_down = TRUE;
    if ((charcode == XK_Alt_L) || (charcode == XK_Alt_L) )
	alt_key_down = TRUE;
    else{

/* Handle keyboard shortcuts */
	if( alt_key_down && Finished_Action() ){
	    switch (charcode) {
		case 'z' :
		case 'Z' :
		    if (undo_available) {
			Undo (event->time);
			if (exiting_paint)
			    return;
		    }
		    break;
		case 'q' :
		case 'Q' :
		    if (picture_changed)
			Create_Quit_Dialog ();
		    else
			Exit_Paint(0);
		    break;
		case 'x' :
		case 'X' :
		    if( select_on )
			Cut (event->time);
		    break;
		case 'c' :
		case 'C' :
		    if( select_on )
			Copy (event->time);
		    break;
		case 'v' :
		case 'V' :
		    Paste (event->time);
		    break;
		case 'p' :
		case 'P' :
		    if( !print_dialog )
			Create_Print_Dialog();
		    else
			Print_File();
		    break;
		case 's' :
		case 'S' :
		    if( strlen(cur_file) == 0 )
			Create_Write_Dialog();
		    else
			if (picture_changed)	
			    Write_File();
		    break;
		case 'w' :
		case 'W' :
		    Refresh_Picture( 0, 0, picture_wd, picture_ht );
		    break;
	    }
	}
    }
}

/*
 *
 * ROUTINE:  Released_Key
 *
 * ABSTRACT: 
 *
 *  Handle keyboard key up events
 *
 */
void Released_Key( w, event, params, num_params )
	Widget  w;
	XKeyEvent *event;
	char **params;
	int	num_params;
{
    int XLS_Return;			
    char XLS_Buffer[2]; 			
    int XLS_MaxBytes = 3;                  
    KeySym XLS_KeySym;			
    static XComposeStatus XLS_ComposeStatus;	
    unsigned long charcode;

    XLS_Return = XLookupString(event, XLS_Buffer, XLS_MaxBytes,
			       &XLS_KeySym, &XLS_ComposeStatus);

    charcode = XLS_KeySym ;
    if ((charcode == XK_Shift_L) || (charcode == XK_Shift_R))
	shift_key_down = FALSE;
    if ((charcode == XK_Alt_L) || (charcode == XK_Alt_L) )
		alt_key_down = FALSE;
}


/*
 *
 * ROUTINE:  Refresh_Key
 *
 * ABSTRACT: 
 *
 *  User hit the refresh key - Refresh the screen
 *
 */
/* Keyboard events */
void Refresh_Key( w, event, params, num_params )
	Widget  w;
	XEvent *event;
	char **params;
	int	num_params;
{
    Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
}

/*
 *
 * ROUTINE:  Refresh_Window
 *
 * ABSTRACT: 
 *
 *  Handle exposure events
 *
 */
void Refresh_Window( w, event, params, num_params )
    Widget  w;
    XExposeEvent *event;
    char **params;
    int	num_params;
{    
    XEvent nextevent;
    XExposeEvent *new_event;
    int more_expose;
    int x, y, wd, ht;
    int min_x, min_y, max_x, max_y;
    int zx, zy, zwd, zht;
    int i, j;
    int xtmp, ytmp;
    long *bp;
    int redraw_zoom_move = FALSE;
    int no_refresh = FALSE;
    static XRectangle clip_rect[9];
    int ncr;	/* number of clip rectangles */

    min_x = event->x;
    min_y = event->y;
    max_x = min_x + event->width;
    max_y = min_y + event->height;

/* look ahead for more expose events */
    more_expose = TRUE;
    while( more_expose ) {            
	if( XtPending() ){
	    XtPeekEvent( &nextevent );
	    if( (nextevent.type == Expose) ){
		new_event = (XExposeEvent *)&nextevent; /* jj-port */
		if( new_event->window == XtWindow(w)){
		    min_x = MIN( min_x, new_event->x );
		    min_y = MIN( min_y, new_event->y );
		    max_x = MAX( max_x, new_event->x + new_event->width );
		    max_y = MAX( max_y, new_event->y + new_event->height );
		    XtNextEvent( &nextevent );
		}
		else
		    more_expose = FALSE;
	    }
	    else {
		more_expose = FALSE;
/* if next event is a configure of the picture window, don't do the refresh */
/* this avoids a screen blink. */
/*
		if ((w == picture_widget) && 
		    (nextevent.type == ConfigureNotify)) {
		    if (nextevent.xconfigure.window == 
	    		XtWindow (widget_ids[PAINT_WINDOW])) {
			if (!((XtWidth (picture_widget) == pimage_wd) &&
			      (XtHeight (picture_widget) == pimage_ht) &&
			      (nextevent.xconfigure.width >= pwindow_wd +
			       Find_Vslider_Space () +
			       XtBorderWidth (widget_ids[PAINT_WINDOW])) &&
                              (nextevent.xconfigure.height >= pwindow_ht +
                               Find_Hslider_Space () +
                               XtBorderWidth (widget_ids[PAINT_WINDOW])))) {
			    no_refresh = TRUE;
			}
		    }
		}
*/
	    }
	}
	else
	    more_expose = FALSE;
    }		

  if (!no_refresh) {
/* Refresh the picture window or zoom window */
    wd = max_x - min_x;
    ht = max_y - min_y;
    if( w == picture_widget ){
/* hack so if expose because clicked on zoom and zoom move highlight becomes
   unobscured, it will be refreshed) */
	if ( current_action == ZOOM_MOVE ) {
	    if (Intersecting_Rectangles (min_x, min_y, wd, ht, 
					 IX_TO_PWX(zoom_xorigin) - 1,
					 IY_TO_PWY(zoom_yorigin) - 1,
					 zoom_width + 2, zoom_height + 2)) {
		XDrawRectangle (disp, pwindow, gc_highlight,
				IX_TO_PWX(zoom_xorigin) - 1, 
				IY_TO_PWY(zoom_yorigin) - 1,
				zoom_width+1, zoom_height+1);
		redraw_zoom_move = TRUE;
	    }
	}

	window_exposure = TRUE;
	Refresh_Picture (PWX_TO_IX(min_x), PWY_TO_IY(min_y), wd, ht);
	window_exposure = FALSE;

/* hack so if expose because clicked on select piece and part of it becomes
   unobscured, it will be refreshed from the proper place. */
	if (select_on && (current_action == MOVE)) {
	    if (Intersecting_Rectangles (min_x, min_y, wd, ht, 
					 IX_TO_PWX(select_x),
					 IY_TO_PWY(select_y),
					 select_width, select_height))
		Copy_Bitmap (copymap, pwindow, 0, 0, select_width,
			     select_height, IX_TO_PWX(select_x),
			     IY_TO_PWY(select_y));

	}
/* hack so if expose because clicked on select piece and zoom magnifier becomes
   unobscured, it will be refreshed) */
	if (zoomed && (current_action == MOVE))
	    Refresh_Magnifier (PWX_TO_IX(min_x), PWY_TO_IY(min_y), wd, ht);
/* hack so if expose because clicked on zoom and zoom move highlight becomes
   unobscured, it will be refreshed) */
	if (redraw_zoom_move)
		XDrawRectangle (disp, pwindow, gc_highlight,
				IX_TO_PWX(zoom_xorigin) - 1, 
				IY_TO_PWY(zoom_yorigin) - 1,
				zoom_width+1, zoom_height+1);
    }
    else {
	if (w == zoom_widget) {
	    if (!skip_zoom_refresh) {
/* make sure area of window is cleared */
		XSetForeground (disp, Get_GC (GC_SD_SOLID),
				colormap[paint_bg_color].pixel);
		XFillRectangle (disp, zoom, Get_GC (GC_SD_SOLID),
				min_x, min_y, wd, ht);

/* draw cross hairs */
		XSetForeground (disp, Get_GC (GC_SD_SOLID),
				colormap[WHITE].pixel);
		for (i = 0; i < zoom_width; i++) {
		    XDrawLine (disp, zoom, Get_GC (GC_SD_SOLID),
			       i * zoom_pixsize, 0,
			       i * zoom_pixsize, zoom_height * zoom_pixsize);
		}
		for (i = 0; i < zoom_height; i++) {
		    XDrawLine (disp, zoom, Get_GC (GC_SD_SOLID),
			       0, i * zoom_pixsize,
			       zoom_width * zoom_pixsize, i * zoom_pixsize);
		}
		

		zx = min_x/zoom_pixsize;
		zy = min_y/zoom_pixsize;

		zwd = max_x/zoom_pixsize - zx + 1;
		zht = max_y/zoom_pixsize - zy + 1;
/*
		zwd = (int)ceil((double)wd/(double)zoom_pixsize);
		zht = (int)ceil((double)ht/(double)zoom_pixsize);
*/
/* check bounds */
		if( zht > zoom_height) 
			zht = zoom_height;
		if( zwd > zoom_width )
			zwd = zoom_width;

/* set pixel values to picture background */
		for( i = 0; i < zht; ++i )
			for( j = 0; j < zwd; ++j ){
				xtmp = j + zx;
				ytmp = i + zy;
				bp = bitvals + (ytmp*zoom_width+xtmp);
				*bp = picture_bg;
				}
		x = zoom_xorigin + zx;
		y = zoom_yorigin + zy;
		Refresh_Zoom_View( x, y, zwd, zht );
/* hack so if expose because clicked on select piece and part of it becomes
   unobscured, it will be refreshed from the proper place. */
		if (select_on && (current_action == MOVE)) {
		    if (Intersecting_Rectangles (x, y, zwd, zht,
						 IX_TO_PWX(select_x),
						 IY_TO_PWY(select_y),
						 select_width, select_height)) {
			if (ifocus == zoom_widget);
			    refresh_zoom_from = FROM_COPYMAP;
			Refresh_Zoom_View (select_x, select_y, select_width,
					   select_height);
			refresh_zoom_from = FROM_PICTURE;
		    }
		}
	    }
	    else {
		skip_zoom_refresh = FALSE;
	    }
	}
    }
    if ((w == ifocus) && (current_action == POLYGON) && rbanding) {
	clip_rect[0].x = min_x;
        clip_rect[0].y = min_y;
        clip_rect[0].width = wd;
        clip_rect[0].height = ht;
	ncr = 1;
	if (zoomed && 
	    (Intersecting_Rectangles (min_x, min_y, wd, ht, 
				      IX_TO_PWX(zoom_xorigin) - 12,
				      IY_TO_PWY(zoom_yorigin) - 12,
				      zoom_width + 24, zoom_height + 24))) {

	    clip_rect[1].x = IX_TO_PWX(zoom_xorigin) - 12;
	    clip_rect[1].y = IY_TO_PWY(zoom_yorigin) - 12;
	    clip_rect[1].width = zoom_width + 24;
	    clip_rect[1].height = 12;

	    clip_rect[2].x = IX_TO_PWX(zoom_xorigin) - 12;
	    clip_rect[2].y = IY_TO_PWY(zoom_yorigin);
	    clip_rect[2].width = 12;
	    clip_rect[2].height = zoom_height;

	    clip_rect[3].x = IX_TO_PWX(zoom_xorigin) + zoom_width;
	    clip_rect[3].y = IY_TO_PWY(zoom_yorigin);
	    clip_rect[3].width = 12;
	    clip_rect[3].height = zoom_height;

	    clip_rect[4].x = IX_TO_PWX(zoom_xorigin) - 12;
	    clip_rect[4].y = IY_TO_PWY(zoom_yorigin) + zoom_height;
	    clip_rect[4].width = zoom_width + 24;
	    clip_rect[4].height = 12;

	    ncr += 4;
	    if (Intersecting_Rectangles (min_x, min_y, wd, ht, 
					 clip_rect[1].x, clip_rect[1].y,
					 clip_rect[1].width, 
					 clip_rect[1].height)) {
		Rectangle_Intersect (min_x, min_y, wd, ht,
				     clip_rect[1].x, clip_rect[1].y,
				     clip_rect[1].width, clip_rect[1].height,
				     &clip_rect[ncr].x, &clip_rect[ncr].y,
				     &clip_rect[ncr].width, 
				     &clip_rect[ncr].height);
		ncr +=1;
	    }

	    if (Intersecting_Rectangles (min_x, min_y, wd, ht, 
					 clip_rect[2].x, clip_rect[2].y,
					 clip_rect[2].width, 
					 clip_rect[2].height)) {
		Rectangle_Intersect (min_x, min_y, wd, ht,
				     clip_rect[2].x, clip_rect[2].y,
				     clip_rect[2].width, clip_rect[2].height,
				     &clip_rect[ncr].x, &clip_rect[ncr].y,
				     &clip_rect[ncr].width, 
				     &clip_rect[ncr].height);
		ncr +=1;
	    }

	    if (Intersecting_Rectangles (min_x, min_y, wd, ht, 
					 clip_rect[3].x, clip_rect[3].y,
					 clip_rect[3].width, 
					 clip_rect[3].height)) {
		Rectangle_Intersect (min_x, min_y, wd, ht,
				     clip_rect[3].x, clip_rect[3].y,
				     clip_rect[3].width, clip_rect[3].height,
				     &clip_rect[ncr].x, &clip_rect[ncr].y,
				     &clip_rect[ncr].width, 
				     &clip_rect[ncr].height);
		ncr +=1;
	    }

	    if (Intersecting_Rectangles (min_x, min_y, wd, ht, 
					 clip_rect[4].x, clip_rect[4].y,
					 clip_rect[4].width, 
					 clip_rect[4].height)) {
		Rectangle_Intersect (min_x, min_y, wd, ht,
				     clip_rect[4].x, clip_rect[4].y,
				     clip_rect[4].width, clip_rect[4].height,
				     &clip_rect[ncr].x, &clip_rect[ncr].y,
				     &clip_rect[ncr].width, 
				     &clip_rect[ncr].height);
		ncr +=1;
	    }

	}
	XSetClipRectangles (disp, gc_rband, 0, 0, clip_rect, ncr, Unsorted);
        Rband_Miter_Line ();
        XSetClipMask (disp, gc_rband, None);
    }
  }
}

void Resize_Widget (w, wd, ht)
    Widget w;
    Dimension wd, ht;
{
    Arg args[3];
    int argcnt = 0;

    XtSetArg (args[argcnt], XmNwidth, wd);
    ++argcnt;
    XtSetArg (args[argcnt], XmNheight, ht);
    ++argcnt;
    XtSetValues (w, args, argcnt);
}


/*
 *
 * ROUTINE:  Configure_Window
 *
 * ABSTRACT: 
 *
 *  Handle configure events
 *
 *  Paint Window must be confined to the size of the screen.
 *
 *  -------------------------------------------------------
 *  | Paint						  |
 *  -------------------------------------------------------
 *  | Main Menu Bar					  |
 *  -------------------------------------------------------
 *  |		   |	/\				|V|
 *  |		   |	|				| |
 *  |<- icon box ->|<- paint window (<= screen width) ->|s|
 *  |		   |	|				|l|
 *  |		   |	|				|i|
 *  |		   |	|				|d|
 *  |		   |	|				|e|
 *  |		   |	|				|r|
 *  |		   |  (<= screen height)		| |
 *  |		   |	|				| |
 *  |		   |	|				| |
 *  |		   |	|				| |
 *  |		   |	\/				| |
 *  |              |------------------------------------+-|
 *  |		   | H slider				| |
 *  -------------------------------------------------------
 *
 *
 */
void Configure_Window (w, event, params, num_params)
	Widget  w;
	XConfigureEvent *event;
	char **params;
	int	num_params;
{
    int new_wd, new_ht;
    int extra_wd, extra_ht;
    int vss, hss;

    skip_zoom_refresh = FALSE;
    new_wd = event->width;
    new_ht = event->height;
    if (w == widget_ids[PAINT_WINDOW]) {
	refresh_picture_pending = TRUE;
/* if confine paint_window to be <= sise of the screen */
/*
	vss = Find_Vslider_Space() + XtBorderWidth (widget_ids[PAINT_WINDOW]);
	hss = Find_Hslider_Space() + XtBorderWidth (widget_ids[PAINT_WINDOW]);
	if ((new_wd > (XDisplayWidth (disp, screen) + vss)) ||
	    (new_ht > (XDisplayHeight (disp, screen) + hss))) {
	    Resize_Widget (toplevel,
			   XtWidth (toplevel) - new_wd + 
			    MIN (new_wd, XDisplayWidth (disp, screen) + vss),
			   XtHeight (toplevel) - new_ht +
			    MIN (new_ht, XDisplayHeight (disp, screen) + hss));
	}
	else {
*/
	    Resize_Picture_Window (new_wd, new_ht);
	    if (exiting_paint)
		return;
/*
	}
*/
    }
    else {
	if ((w == zoom_widget) &&
	    ((new_wd != old_zoom_wd) || (new_ht != old_zoom_ht))) {

	    old_zoom_wd = new_wd;
	    old_zoom_ht = new_ht;

	    if (new_wd > (zoom_pixsize * pwindow_wd))
		new_wd = zoom_pixsize * pwindow_wd;

	    if (new_ht > (zoom_pixsize * pwindow_ht))
		new_ht = zoom_pixsize * pwindow_ht;
			 
	    if ((new_wd != old_zoom_wd) || (new_ht != old_zoom_ht)) {
		Resize_Widget (zoom_dialog, new_wd, new_ht);
		skip_zoom_refresh = TRUE;
	    }
	    else {
		Resize_Zoom_Window (new_wd, new_ht);
	    }
	}
	else {
	    if (w == zoom_dialog) {
		skip_zoom_refresh = TRUE;
	    }
	}
    }
}
