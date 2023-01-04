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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/lines.c,v 1.1.2.3 92/12/11 08:35:13 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port 
  #ifndef ULTRIX
  #module LINES "V1-000"
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
**   LINES creates the the line width menu.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**
**--
**/           

#include "paintrefs.h"

static int line_row;
static int line_pixmap;
/*
 *
 * ROUTINE:  Highlight_Line
 *
 * ABSTRACT: 
 *
 *  Highlights the given line
 *
 */
void Highlight_Line( row )
int row;
{
int x, y;

/* draw rectangle into pixmap and refresh window */
	x = 0;
	y = LINE_CELL_HT * row;

	XDrawRectangle (disp, line_pixmap, Get_GC(GC_D1_INVERT), x, y,
			LINE_CELL_WD - 1, LINE_CELL_HT - 1);
	XCopyPlane (disp, line_pixmap,
		    XtWindow(widget_ids[LINE_DIALOG_WINDOW]),
		    Get_GC(GC_MD_COPY), 
		    x, y, LINE_CELL_WD, LINE_CELL_HT, x, y, 1);
}


/*
 *
 * ROUTINE:  Clicked_On_Line
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button down events
 *
 */
void Clicked_On_Line( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;
{
int index;

    if (Finished_Action()) {
/* determine which line was clicked on */
/* unhighlight old and highlight new */
	Highlight_Line( line_row );
	line_row = event->y/LINE_CELL_HT;
	Highlight_Line( line_row );
	
	cur_line_wd = line_width[line_row];
	gc_values.line_width = cur_line_wd;
	XChangeGC( disp, Get_GC(GC_PD_OUTLINE), GCLineWidth, &gc_values );
	XChangeGC( disp, Get_GC(GC_PD_STROKE), GCLineWidth, &gc_values );
	XChangeGC( disp, Get_GC(GC_SD_OUTLINE), GCLineWidth, &gc_values );
	XChangeGC( disp, Get_GC(GC_SD_STROKE), GCLineWidth, &gc_values );
    }
}


void Refresh_Line_Window( w, event, params, num_params )
	Widget  w;
	XExposeEvent *event;
	char **params;
	int	num_params;
{    
	XCopyPlane( disp, line_pixmap,
		    XtWindow(widget_ids[LINE_DIALOG_WINDOW]),
		    Get_GC(GC_MD_COPY), event->x, event->y, event->width,
		    event->height, event->x, event->y, 1 );
}


void Dismiss_Line_Dialog( w, tag, r )
    Widget w;					/* pulldown menu */
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    if (Finished_Action())
	XtUnmanageChild(line_dialog);
}



void Create_Line_Dialog()
{
static int num_actions = 2;
static XtActionsRec action_table [] =
{
	{"Clicked_On_Line", (XtActionProc)Clicked_On_Line},  /* jj-port */
	{"Refresh_Line_Window", (XtActionProc)Refresh_Line_Window}  /* jj-port */
};

    int i;

    Set_Cursor_Watch (pwindow);
    if( !line_dialog ){
/* Create the pixmap for lines widths */
        line_pixmap = Create_Bitmap( 0, LINE_CELL_WD, 
				     NUM_LINE_SIZES * LINE_CELL_HT);
	if (line_pixmap) {
/* add actions to the action table, fetch the widget and manage it */
	    XtAddActions ( action_table, num_actions );
	    if (Fetch_Widget ("line_dialog_box", main_widget,
			      &line_dialog) != MrmSUCCESS)
		DRM_Error ("can't fetch line dialog");

/* center widgets in adb */
	    Set_Attribute (widget_ids[LINE_DIALOG_DISMISS_BUTTON],
			  XmNleftOffset,
		    - (XtWidth (widget_ids[LINE_DIALOG_DISMISS_BUTTON]) / 2));

/* Manage dialog box before creating the backup pixmap dl-10/6/88 */
	    XtManageChild( line_dialog );

	    XSetForeground( disp, Get_GC(GC_D1_COPY), 1 );
	    for( i = 0; i < NUM_LINE_SIZES; ++i ){
		gc_values.line_width = line_width[i]; 
		XChangeGC( disp, Get_GC(GC_D1_COPY), GCLineWidth, &gc_values );
		XDrawLine (disp, line_pixmap, Get_GC(GC_D1_COPY), 3,
			   (LINE_CELL_HT / 2) + (i * LINE_CELL_HT),
			   LINE_CELL_WD - 3,
			   (LINE_CELL_HT / 2) + (i * LINE_CELL_HT) );
		}
	    gc_values.line_width = 0;
	    XChangeGC( disp, Get_GC(GC_D1_COPY), GCLineWidth, &gc_values );

/* highlight current line width */
	    line_row = 1;
	    Highlight_Line( line_row );
	}
	else {
	    Display_Message ("T_NO_MEM_FOR_LINES");
	}
    }			
    if (line_dialog) {
        XtManageChild(line_dialog);
	Pop_Widget (line_dialog);
    }
    Set_Cursor (pwindow, current_action);
}		
