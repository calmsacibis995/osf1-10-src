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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/brush.c,v 1.1.2.3 92/12/11 08:33:43 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module BRUSH "V1-000"
  #endif
*/
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved
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
**   This module contains routines that create the brush shape dialog
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      10/6/88
**              make sure brush menu gets created regardless of current fill
**              pattern.
**
**--
**/           
#include "paintrefs.h" 

Pixmap brush_pixmap;

void Dismiss_Brush_Dialog( w, tag, r )
    Widget w;					/* pulldown menu */
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    if (Finished_Action())
	XtUnmanageChild( brush_dialog );
}

/*
 *
 * ROUTINE:  Initialize_Brushes
 *
 * ABSTRACT: 
 *
 *  Define the brush shapes
 *
 */
void Initialize_Brushes()
{        
int wd, ht;
int i;

/* Initialize square brush */
	wd = 6;
	ht = 6;
	for( i = 0; i < NUM_BRUSH_SIZES; ++i ){
		brushes[ SQUARE_BRUSH ][ i ].wd = wd;
/* not used 
		brushes[ SQUARE_BRUSH ][ i ].x1 = wd;
		brushes[ SQUARE_BRUSH ][ i ].y1 = ht;
		brushes[ SQUARE_BRUSH ][ i ].x2 = wd;
		brushes[ SQUARE_BRUSH ][ i ].y2 = ht;
*/
		wd += 2;
		ht += 2;
		}

	wd = 6;
	ht = 6;
	for( i = 0; i < NUM_BRUSH_SIZES; ++i ){
		brushes[ ROUND_BRUSH ][ i ].wd = wd;
/* not used
		brushes[ ROUND_BRUSH ][ i ].x1 = wd;
		brushes[ ROUND_BRUSH ][ i ].y1 = ht;
		brushes[ ROUND_BRUSH ][ i ].x2 = wd;
		brushes[ ROUND_BRUSH ][ i ].y2 = ht;
*/
		wd += 2;
		ht += 2;
		}

	wd = 2;
	ht = 2;
 	for( i = 0; i < NUM_BRUSH_SIZES; ++i ){         
		brushes[ R45_BRUSH ][ i ].wd = wd+ht+1;
		brushes[ R45_BRUSH ][ i ].x1 = -wd;
		brushes[ R45_BRUSH ][ i ].y1 = ht;
		brushes[ R45_BRUSH ][ i ].x2 = wd;
		brushes[ R45_BRUSH ][ i ].y2 = -ht;
		wd += 2;
		ht += 2;           
		}

	wd = 2;
	ht = 2;
	for( i = 0; i < NUM_BRUSH_SIZES; ++i ){
		brushes[ L45_BRUSH ][ i ].wd = wd+ht+1;
		brushes[ L45_BRUSH ][ i ].x1 = -wd;
		brushes[ L45_BRUSH ][ i ].y1 = -ht;
		brushes[ L45_BRUSH ][ i ].x2 = wd;
		brushes[ L45_BRUSH ][ i ].y2 = ht;
		wd += 2;
		ht += 2;
		}

	wd = 2;
	ht = 2;
	for( i = 0; i < NUM_BRUSH_SIZES; ++i ){
		brushes[ HORIZ_BRUSH ][ i ].wd = wd+ht+1;
		brushes[ HORIZ_BRUSH ][ i ].x1 = -wd;
		brushes[ HORIZ_BRUSH ][ i ].y1 = 0;
		brushes[ HORIZ_BRUSH ][ i ].x2 = wd;
		brushes[ HORIZ_BRUSH ][ i ].y2 = 0;
		wd += 2;
		ht += 2;
		}

	wd = 2;
	ht = 2;
	for( i = 0; i < NUM_BRUSH_SIZES; ++i ){
		brushes[ VERT_BRUSH ][ i ].wd = wd+ht+1;
		brushes[ VERT_BRUSH ][ i ].x1 = 0;
		brushes[ VERT_BRUSH ][ i ].y1 = -ht;
		brushes[ VERT_BRUSH ][ i ].x2 = 0;
		brushes[ VERT_BRUSH ][ i ].y2 = ht;
		wd += 2;
		ht += 2;
		}


}

/*
 *
 * ROUTINE:  Highlight_Bshape
 *
 * ABSTRACT: 
 *
 *  Highlights/Unhighlights the current brush shape
 *
 */
void Highlight_Bshape()
{
int x, y;

/* draw rectangle into pixmap and refresh window */
	x = cur_brush_index * BRUSH_CELL_SZ;
	y = cur_brush * BRUSH_CELL_SZ;

	XFillRectangle( disp, brush_pixmap, Get_GC(GC_D1_INVERT), x, y,
	  BRUSH_CELL_SZ, BRUSH_CELL_SZ);
	
	XCopyPlane( disp, brush_pixmap,
		    XtWindow(widget_ids[BRUSH_DIALOG_WINDOW]),
		    Get_GC(GC_MD_COPY), 
		    x, y, BRUSH_CELL_SZ, BRUSH_CELL_SZ, x, y, 1 );
}
/*
 *
 * ROUTINE:  Clicked_On_Bshape
 *
 * ABSTRACT: 
 *
 *  User clicked on brush shape
 *
 */
void Clicked_On_Bshape( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;
{

    if (Finished_Action()) {
/* determine which icon was clicked on */
/* unhighlight old and highlight new */
	Highlight_Bshape();
	cur_brush_index = event->x/BRUSH_CELL_SZ;
	cur_brush = event->y/BRUSH_CELL_SZ;
	Highlight_Bshape();	

	if(cur_brush == ROUND_BRUSH){
		gc_values.line_width = brushes[cur_brush ][ cur_brush_index ].wd;
		XChangeGC( disp, Get_GC(GC_SD_ROUND_BRUSH), GCLineWidth, &gc_values );
		XChangeGC( disp, Get_GC(GC_PD_ROUND_BRUSH), GCLineWidth, &gc_values );
		}
    }
}

/*
 *
 * ROUTINE:  Refresh_Brush_Window
 *
 * ABSTRACT: 
 *
 *  Refresh the brush window on expose events
 *
 */
void Refresh_Brush_Window( w, event, params, num_params )
	Widget  w;
	XExposeEvent *event;
	char **params;
	int	num_params;
{    
	XCopyPlane( disp, brush_pixmap,
		    XtWindow(widget_ids[BRUSH_DIALOG_WINDOW]),
		    Get_GC(GC_MD_COPY), event->x, event->y, event->width,
		    event->height, event->x, event->y, 1 );
}

void Create_Brush_Dialog()
{
	Arg args[6];
	int argcnt;
	int i, j;
	int x, y;
	int save_brush, save_index, save_action;

static int num_actions = 2;
static XtActionsRec action_table [] =
{
	{"Clicked_On_Bshape", (XtActionProc)Clicked_On_Bshape}, /* jj-port */
	{"Refresh_Brush_Window", (XtActionProc)Refresh_Brush_Window}  /* jj-port */
};

    Set_Cursor_Watch (pwindow);
    if( !brush_dialog ){
	brush_pixmap = Create_Bitmap( 0, BRUSH_CELL_SZ * NUM_BRUSH_SIZES,
				      BRUSH_CELL_SZ * NUM_BRUSH_SHAPES );

	if (brush_pixmap) {
	    Initialize_Brushes();
	    if (Fetch_Widget ("brush_dialog_box", main_widget,
			      &brush_dialog) != MrmSUCCESS)
		DRM_Error ("can't fetch brush dialog");

/* center widgets in adb */
	    Set_Attribute (widget_ids[BRUSH_DIALOG_DISMISS_BUTTON],
			  XmNleftOffset,
		    - (XtWidth (widget_ids[BRUSH_DIALOG_DISMISS_BUTTON]) / 2));
		
/* Create the pixmap containing the brush shapes */
	    save_brush = cur_brush;
	    save_index = cur_brush_index;
/* by setting the current action to ERASE, Draw_With_Brush will always
draw a brush; regardless of the current fill stipple.  This is kind of
sloppy, but it enables us to use 1 routine for drawing brushes dl-10/6/88 */
	    save_action = current_action;
	    current_action = ERASE;
	    cur_gc = Get_GC( GC_D1_COPY );
	    XSetForeground( disp, cur_gc, 1 );
	    for( i = 0; i < NUM_BRUSH_SIZES; ++i )
		for( j = 0; j < NUM_BRUSH_SHAPES; ++j ){
		    cur_brush_index = i;
		    cur_brush = j;
		    brush_x1 = brushes[cur_brush ][ cur_brush_index].x1;
		    brush_y1 = brushes[cur_brush ][ cur_brush_index].y1;
		    brush_x2 = brushes[cur_brush ][ cur_brush_index].x2;
		    brush_y2 = brushes[cur_brush ][ cur_brush_index].y2;
		    brush_wd = brushes[cur_brush ][ cur_brush_index].wd;
		    x = (BRUSH_CELL_SZ / 2) + (BRUSH_CELL_SZ * i);
		    y = (BRUSH_CELL_SZ / 2) + (BRUSH_CELL_SZ * j);
		    Draw_With_Brush( brush_pixmap, x, y );
		}
	    cur_brush = save_brush;
	    cur_brush_index = save_index;
	    current_action = save_action;

/* Add actions to action table */
	    XtAddActions ( action_table, num_actions );		

/* highlight current brush shape */
	    x = cur_brush_index * BRUSH_CELL_SZ;
	    y = cur_brush * BRUSH_CELL_SZ;
	    XFillRectangle( disp, brush_pixmap, Get_GC(GC_D1_INVERT), x, y,
			    BRUSH_CELL_SZ, BRUSH_CELL_SZ);

	}
	else {
	    Display_Message ("T_NO_MEM_FOR_BRUSH");
	}
    }			

    if (brush_dialog) {
	XtManageChild( brush_dialog );
	Pop_Widget (brush_dialog);
    }
    Set_Cursor (pwindow, current_action);
}		
