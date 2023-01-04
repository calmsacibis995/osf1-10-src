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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/patterns.c,v 1.1.2.3 92/12/11 08:35:39 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module PATTERNS "V1-000"
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
**   Creates pattern dialog box and handles changes
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**              dl      10/6/88
**              spraycan, brush now use the fill pattern instead of the outline
**              pattern.
**
**--
**/           
#include "paintrefs.h"
#include "patterns.h"


Pixmap fill_sample, outline_sample;
Pixmap fg_pattern, bg_pattern;
static int set_outline;

/* These variables are to keep track of the current pattern indices
that are being used for the fill and outline stipple */
static int fill_index = -1;
static int outline_index = -1;

void Refresh_Pattern_Window (w, event, params, num_params)
	Widget  w;
	XExposeEvent *event;
	char **params;
	int	num_params;
{    
	XCopyPlane (disp, pattern_pixmap,
		    XtWindow(widget_ids[PATTERN_SAMPLE_WINDOW]),
		    Get_GC(GC_MD_COPY), event->x, event->y, event->width,
		    event->height, event->x, event->y, 1 );
}


void Redraw_Small_Pattern_Window (w, pix, x, y, wd, ht)
    Window w;
    Pixmap pix;
    int x, y, wd, ht;
{
    XCopyArea (disp, pix, w, Get_GC(GC_MD_COPY), x, y, wd, ht, x, y);
}

void Refresh_Outline_Pattern_Window (w, event, params, num_params)
    Widget  w;
    XExposeEvent *event;
    char **params;
    int	num_params;
{    
    Redraw_Small_Pattern_Window (XtWindow(widget_ids[OUTLINE_PATTERN_WINDOW]),
				 outline_sample, event->x, event->y,
				 event->width, event->height);
}

void Refresh_Fill_Pattern_Window (w, event, params, num_params)
    Widget  w;
    XExposeEvent *event;
    char **params;
    int	num_params;
{    
    Redraw_Small_Pattern_Window (XtWindow(widget_ids[FILL_PATTERN_WINDOW]),
				 fill_sample, event->x, event->y,
				 event->width, event->height);
}


void Refresh_Pattern_Windows ()
{
    if (set_outline) {
	Redraw_Small_Pattern_Window (
	    XtWindow (widget_ids[OUTLINE_PATTERN_WINDOW]),
	    outline_sample, 0, 0, 32, 32);
    }
    else {
	Redraw_Small_Pattern_Window (
	    XtWindow (widget_ids[FILL_PATTERN_WINDOW]),
	    fill_sample, 0, 0, 32, 32);
    }
}


/*
 *
 * ROUTINE:  Set_Edit_Pattern_Button
 *
 * ABSTRACT: 
 *
 * Change the sensitivity of the Edit pattern button.  It should only
 * be sensitive when the current pattern can be edited.
 *                    
 */
void Set_Edit_Pattern_Button (state)
    int state;
{
	XtSetSensitive (widget_ids[OPTIONS_EDIT_PATTERN_BUTTON], state);
}


static void More_Patterns( w, tag, r )
    Widget w;					/* pulldown menu */
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
printf( "more\n" );
}


void Dismiss_Pattern_Dialog( w, tag, r )
    Widget w;					/* pulldown menu */
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    if (Finished_Action())
	XtUnmanageChild( pattern_dialog );
}


/*
 *
 * ROUTINE:  Set_No_Stipple
 *
 * ABSTRACT: 
 *
 *  Sets No Stipple
 *
 */
void Set_No_Stipple( p )
Pixmap p;
{
    XDrawLine (disp, p, Get_GC (GC_MD_COPY), 0, 0, 32, 32);
    XDrawLine (disp, p, Get_GC (GC_MD_COPY), 32, 0, 0, 32);
}



/*
 *
 * ROUTINE:  Set_Outline_Sample
 *
 * ABSTRACT: 
 *
 *  Sets outline sample
 *
 */
void Set_Outline_Sample()
{

/* clear pixmap- it may have contained the no stipple symbol */
    XSetForeground (disp, Get_GC(GC_MD_SOLID), window_bg);
    XFillRectangle (disp, outline_sample, Get_GC(GC_MD_SOLID), 0, 0, 32, 32);

    if (outline_stipple) {
	XFillRectangle (disp, outline_sample, Get_GC(GC_MD_OUTLINE),
			2, 2, 28, 28);
    }
    else
	Set_No_Stipple (outline_sample);
		
}



/*
 *
 * ROUTINE:  Set_Fill_Sample
 *
 * ABSTRACT: 
 *
 *  Sets fill sample
 *
 */
void Set_Fill_Sample()
{

/* clear pixmap- it may have contained the no stipple symbol */
    XSetForeground (disp, Get_GC(GC_MD_SOLID), window_bg);
    XFillRectangle (disp, fill_sample, Get_GC(GC_MD_SOLID), 0, 0, 32, 32);

    if (fill_stipple) {
	XFillRectangle (disp, fill_sample, Get_GC(GC_MD_FILL),
			2, 2, 28, 28);
    }
    else{
    	Set_No_Stipple (fill_sample);
    }

}

/*
 *
 * ROUTINE:  Create_Samples
 *
 * ABSTRACT: 
 *
 *  Creates samples for outline/fill patterns
 *
 */
int Create_Samples()
{
	
/* Create outline and fill push buttons */
    outline_sample = Create_Pixmap (window_bg, 32, 32);
    fill_sample = Create_Pixmap (window_bg, 32, 32);
/*
    fg_pattern = Create_Pixmap (window_fg, 16, 16);
    bg_pattern = Create_Pixmap (window_bg, 16, 16);
*/
    if (outline_sample && fill_sample /* && fg_pattern && bg_pattern */	) {
	return (TRUE);
    }
    else {
	if (outline_sample) {
	    XFreePixmap (disp, outline_sample);
	    outline_sample = 0;
	}
	if (fill_sample) {
	    XFreePixmap (disp, fill_sample);
	    fill_sample = 0;
	}
/*
	if (fg_pattern) {
	    XFreePixmap (disp, fg_pattern);
	    fg_pattern = 0;
	}
	if (bg_pattern) {
	    XFreePixmap (disp, bg_pattern);
	    bg_pattern = 0;
	}
*/
	return (FALSE);
    }
}


void Set_Sample_Buttons ()
{
    Set_Outline_Sample();
    Set_Fill_Sample();

    set_outline = FALSE;
/*
    Set_Attribute (widget_ids[SOLID_FG_BUTTON], XmNlabelPixmap, fg_pattern);
    Set_Attribute (widget_ids[SOLID_BG_BUTTON], XmNlabelPixmap, bg_pattern);
*/
}


/*
 *
 * ROUTINE:  Set_Stipple
 *
 * ABSTRACT: 
 *
 *  Sets the all gc's that use the stipple pattern
 *
 */
void Set_Stipple()
{
    if (set_outline) {
	if (outline_stipple) {
	    XSetStipple (disp, Get_GC(GC_PD_OUTLINE), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_PD_STROKE), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_OUTLINE), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_STROKE), outline_stipple);
/* jj - 3/27/89 */ 
	    XSetStipple (disp, Get_GC(GC_PD_ROUND_BRUSH), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_PD_SQUARE_BRUSH), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_PD_SPRAY), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_ROUND_BRUSH), outline_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_SQUARE_BRUSH), outline_stipple);

	    XSetStipple (disp, Get_GC(GC_MD_OUTLINE), outline_stipple);
	}	    
	Set_Outline_Sample ();
    }
    else {
	if (fill_stipple) {
	    XSetStipple (disp, Get_GC(GC_PD_FLOOD), fill_stipple);
	    XSetStipple (disp, Get_GC(GC_PD_FILL), fill_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_FILL), fill_stipple);
/* jj - 3/27/89 
	    XSetStipple (disp, Get_GC(GC_PD_ROUND_BRUSH), fill_stipple);
	    XSetStipple (disp, Get_GC(GC_PD_SQUARE_BRUSH), fill_stipple);
	    XSetStipple (disp, Get_GC(GC_PD_SPRAY), fill_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_ROUND_BRUSH), fill_stipple);
	    XSetStipple (disp, Get_GC(GC_SD_SQUARE_BRUSH), fill_stipple);
*/

	    XSetStipple (disp, Get_GC(GC_MD_FILL), fill_stipple);
	}
	Set_Fill_Sample();
    }

/* dl - 10/19/88 set edit pattern button to the correct state */
    if( pindex == -1 )
	Set_Edit_Pattern_Button (INSENSITIVE);
    else
	Set_Edit_Pattern_Button (SENSITIVE);
}

/*
 *
 * ROUTINE:  Edited_Pattern
 *
 * ABSTRACT: 
 *
 *  The pattern has been changed, reset the pattern menu
 *
 */
void Edited_Pattern()
{
	Change_Menu_Entry( XtWindow(widget_ids[PATTERN_SAMPLE_WINDOW]),
			   pattern_pixmap, patterns[pindex], patrow, patcol,
			   PATTERN_SZ, PATTERN_SPACE, PATTERN_SPACE);

/* pick up the new pixmap */
	if (set_outline)
	    outline_stipple = patterns[pindex];
	else
	    fill_stipple = patterns[pindex];

	Set_Stipple();
	Refresh_Pattern_Windows ();
}

/*
 *
 * ROUTINE:  Clicked_On_Pattern
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button down events
 *
 */
void Clicked_On_Pattern( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;

{
int row, col;
int argcnt;
Arg args[3];

    if (Finished_Action()) {
/* determine which pattern was clicked on */
	row = event->y/(PATTERN_SZ + PATTERN_SPACE);
	col = event->x/(PATTERN_SZ + PATTERN_SPACE);

/* change current pattern */
	pindex = row + col*(PATTERN_ROWS);

	if( set_outline ){
		outline_stipple = patterns[pindex];
		outline_index = pindex;
		}
	else{
		fill_stipple = patterns[pindex];
		fill_index = pindex;
		}
	Set_Stipple();
	Refresh_Pattern_Windows ();
	patrow = row;
	patcol = col;
    }
}


/*
 *
 * ROUTINE:  Create_Patterns
 *
 * ABSTRACT: 
 *
 *  Creates a scrollable window to access all patterns
 *
 */
int Create_Patterns()
{
int i;

/* create pattern bitmaps */
	for( i = 0; i < NUM_PATTERNS; ++i )
		patterns[i] = XCreatePixmapFromBitmapData( disp, btmap,
	 	 pattern_bits[i], 16, 16, 1, 0, 1 );

	if (patterns[NUM_PATTERNS - 1]) {
	    pattern_pixmap = Create_Bitmap_Menu( patterns, PATTERN_ROWS,
		    PATTERN_COLS, PATTERN_SZ, PATTERN_SPACE, PATTERN_SPACE);
	}

	if (pattern_pixmap == 0) {
/* free up space */
	    for( i = 0; i < NUM_PATTERNS; ++i ) {
		if (patterns[i])
		    XFreePixmap (disp, patterns[i]);
		patterns[i] = 0;
	    }
	    return (FALSE);
	}
	else {
	    return (TRUE);
	}
}

/*
 *
 * ROUTINE:  Create_Pattern_Dialog
 *
 * ABSTRACT: 
 *
 *  Creates a pattern dialog box
 *
 */
Widget Create_Pattern_Dialog()
{
    static int num_actions = 4;
    static XtActionsRec action_table [] =
    {
	{"Clicked_On_Pattern", (XtActionProc)Clicked_On_Pattern},  /* jj-port */
	{"Refresh_Pattern_Window", (XtActionProc)Refresh_Pattern_Window},  /* jj-port */
	{"Refresh_Outline_Pattern_Window", (XtActionProc)Refresh_Outline_Pattern_Window},
	{"Refresh_Fill_Pattern_Window", (XtActionProc)Refresh_Fill_Pattern_Window}
    };

    int db_wd, frame_wd;
    int small_b_wd, large_b_wd;
    int small_b_ht, large_b_ht;
    int x, offset;
    
    Set_Cursor_Watch (pwindow);
    if( !pattern_dialog ){
	if (Create_Samples()) {      
	    if (Create_Patterns()) {
		XtAddActions ( action_table, num_actions );		
		if (Fetch_Widget ("pattern_dialog_box", main_widget,
				  &pattern_dialog) != MrmSUCCESS)
		    DRM_Error ("can't fetch pattern dialog");
/* Center widgets */
		Set_Attribute (widget_ids[PATTERN_DIALOG_DISMISS_BUTTON],
	XmNleftOffset, 
	- (XtWidth (widget_ids[PATTERN_DIALOG_DISMISS_BUTTON]) / 2));

		small_b_ht = XtHeight (widget_ids[SOLID_FG_BUTTON]);
		large_b_ht = XtHeight (widget_ids[PATTERN_NONE_BUTTON]);
		Set_Attribute (widget_ids[SOLID_FG_BUTTON], XmNtopOffset, 
			       (large_b_ht - small_b_ht) / 2);
		Set_Attribute (widget_ids[SOLID_BG_BUTTON], XmNtopOffset, 
			       (large_b_ht - small_b_ht) / 2);

/* add tab groups */
		XmAddTabGroup (widget_ids[PATTERN_FORM_1]);
		XmAddTabGroup (widget_ids[PATTERN_FORM_2]);
		XmAddTabGroup (widget_ids[PATTERN_DIALOG_DISMISS_BUTTON]);
		Set_Sample_Buttons ();

/* Manage the dialog box, so we can center the rest of the widgets */
		XtManageChild( pattern_dialog );
		db_wd = XtWidth (pattern_dialog);
		x = XtX (widget_ids[SOLID_FG_BUTTON]) +
		    XtX (widget_ids[PATTERN_FORM_2]);
		small_b_wd = XtWidth (widget_ids[SOLID_FG_BUTTON]);
		large_b_wd = XtWidth (widget_ids[PATTERN_NONE_BUTTON]);
		offset = (db_wd - ((2*x) + (2*small_b_wd) + large_b_wd)) / 2;
		Set_Attribute (widget_ids[SOLID_BG_BUTTON], XmNleftOffset,
			       offset);
		Set_Attribute (widget_ids[PATTERN_NONE_BUTTON], XmNleftOffset,
			       offset);

		frame_wd = XtWidth (widget_ids[PATTERN_SAMPLE_WINDOW_FRAME]);
		Set_Attribute (widget_ids[PATTERN_SAMPLE_WINDOW_FRAME],
			       XmNleftOffset, (db_wd - frame_wd) / 2);
	    }
	}
    }
    if (pattern_dialog) {
	if (!XtIsManaged (pattern_dialog))
	    XtManageChild( pattern_dialog );
	Pop_Widget (pattern_dialog);
    }
    else {
	Display_Message ("T_NO_MEM_FOR_PATTERNS");
    }
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Set_Sample_Pattern
 *
 * ABSTRACT: 
 *
 *  Sets the fill/outline sample
 *
 */
void Set_Sample_Pattern( w, pattern_id, r )
    Widget w;					/* pulldown menu */
    int    *pattern_id;
    XtCallbackList *r;		/* just pick up the boolean */
{
    if (Finished_Action()) {
	if( set_outline ){
	    outline_index = -1;
	    switch (*pattern_id) {
		case PATTERN_NONE_ID :
			outline_stipple = 0;
			break;
		case PATTERN_BG_ID :
			outline_stipple = solid_bg_stipple;
			break;
		case PATTERN_FG_ID :
			outline_stipple = solid_fg_stipple;
			break;
	    }
	}
	else{
	    fill_index = -1;
	    switch (*pattern_id) {
		case PATTERN_NONE_ID :
			fill_stipple = 0;
			break;
		case PATTERN_BG_ID :
			fill_stipple = solid_bg_stipple;
			break;
		case PATTERN_FG_ID :
			fill_stipple = solid_fg_stipple;
			break;
	    }
	}
	pindex = -1;
	Set_Stipple();
	Refresh_Pattern_Windows ();
    }
}

void Set_Sample( w, pattern_id, r )
    Widget w;					/* pulldown menu */
    int    *pattern_id;
    XtCallbackList *r;		/* just pick up the boolean */
{
    if (Finished_Action()) {
	switch (*pattern_id) {
	    case OUTLINE_PATTERN_ID :
		if (set_outline) {
		    Set_Attribute (w, XmNset, TRUE);
		}
		else {
		    Set_Attribute (widget_ids[FILL_PATTERN_TOGGLE], XmNset,
				   FALSE);
		    set_outline = TRUE;
		    pindex = outline_index;
		}
		break;
	    case FILL_PATTERN_ID :
		if (set_outline) {
		    Set_Attribute (widget_ids[OUTLINE_PATTERN_TOGGLE], XmNset,
				   FALSE);
		    set_outline = FALSE;
		    pindex = fill_index;
		}
		else {
		    Set_Attribute (w, XmNset, TRUE);
		}
		break;
	}

/* dl - 10/19/88 set edit pattern button to the correct state */
	if (pindex == -1)
		Set_Edit_Pattern_Button (INSENSITIVE);
	else
		Set_Edit_Pattern_Button (SENSITIVE);
    }
}
