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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/editpat.c,v 1.1.2.3 92/12/11 08:34:43 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module EDITPAT "V1-000"
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
**   This module contains rountines that handle pattern editing
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

static long *bvals = 0;
static int prv_x, prv_y;
static int new_x, new_y;
static Pixmap stipmap;
static int drawing_line;
static int new_fg;


/*
 *
 * ROUTINE:  Update_Edit_Pattern_Dialog
 *
 * ABSTRACT:  Refreshed the windows in the pattern editing dialog box
 *
 */                                                       
static void Refresh_Fatbits()
{
   XImage *image;
   int i,j;
   long value;
   int xpt, ypt;
   int wd, ht;                                                        
   int xtmp, ytmp;
   int zwd, zht;
   int fg;
   GC gc_solid;
                   
/*******************************************************************
 * refresh the fatbits representation of the pattern         *
 ******************************************************************/
   zwd = PATTERN_WD;
   zht = PATTERN_HT;
   image = XGetImage (disp, stipmap, 0, 0, zwd, zht, 1, XYPixmap);
   wd = zoom_pixsize -zoom_pixborder /* *2 */;
   ht = wd;
   gc_solid = Get_GC (GC_MD_SOLID);

   for (i = 0; i < zht; ++i) {
	for (j = 0; j < zwd; ++j) {
	    value = XGetPixel (image, j, i);
	    if (value != bvals[i*zwd+j]) {
/* Set color */
		if (value == 1)
		    fg = window_fg;
		else
		    fg = window_bg;
		XSetForeground (disp, gc_solid, fg);

/* Draw pixel as a rectangle */
		xpt = j*zoom_pixsize + zoom_pixborder;
		ypt = i*zoom_pixsize + zoom_pixborder;
		XFillRectangle (disp, XtWindow(widget_ids[PBITS_WINDOW]), 
				gc_solid, xpt, ypt, wd, ht);
		bvals[i*zwd+j] = value;
	    }
	} /* for */
   }
   XDestroyImage (image);
}


/*
 *
 * ROUTINE:  Refresh_Pattern_Sample
 *
 * ABSTRACT:  Refreshed the window containing the pattern sample
 *
 */                                                       
static void Refresh_Pattern_Sample()
{
GC gc_fill;
    Pixmap tmpmap;

/* Refresh the pattern status window - obtain stipple by inverting pattern*/

/* Seems to be a bug in XSetStipple. If the same pixmap is used nothing
 * happens */
	tmpmap = XCreatePixmap (disp, DefaultRootWindow(disp), PATTERN_WD, 
				PATTERN_HT, 1);
	XCopyArea (disp, stipmap, tmpmap, Get_GC(GC_D1_COPY), 0, 0, PATTERN_WD,
                   PATTERN_HT, 0, 0);
	gc_fill = Get_GC (GC_MD_FILL);
	XSetStipple (disp, gc_fill, tmpmap);  
	XFillRectangle (disp, XtWindow(widget_ids[PSAMPLE_WINDOW]),
			gc_fill, 0, 0, PATTERN_WD*zoom_pixsize, 
			PATTERN_HT*zoom_pixsize);
	XFreePixmap (disp, tmpmap);
}				

/*
 *
 * ROUTINE:  Refresh_Edit_Pattern_Dialog
 *
 * ABSTRACT:  Refreshed the windows in the pattern editing dialog box
 *
 */                                                       
void Refresh_Edit_Pattern(w, stuff, reason)
    Widget   w;
    caddr_t  stuff;
    int      reason;
{
int i,j;

/* Re-initialize the array */
    for( i = 0; i < PATTERN_WD; ++i )
	for( j = 0; j < PATTERN_HT; ++j )
	    bvals[j*PATTERN_WD +i] = 0;
    Refresh_Fatbits();
    Refresh_Pattern_Sample();
}
                                      
/*
 *
 * ROUTINE:  Draw_Fatbit
 *
 * ABSTRACT: 
 *
 *  Draw the pixel from picture coordinates into the zoom view window
 *
 */
void Draw_Fatbit( x, y )
int x, y;   
{
int wd, ht;
int xpt, ypt;
int x1, y1;
int fore;
GC gc_solid;

    wd = zoom_pixsize -zoom_pixborder /* *2 */;
    ht = wd;
    /* jj-12/14/88 only process if point is inside window */
    if ((x >= 0) && (y >= 0) && (x < PATTERN_WD) && (y < PATTERN_HT)) {	
	if (new_fg != bvals[y*PATTERN_WD+x]) {
	    if (new_fg == 1)
		fore = window_fg;
	    else
		fore = window_bg;
	    gc_solid = Get_GC (GC_MD_SOLID);
	    XSetForeground (disp, gc_solid, fore);
	    xpt = x*zoom_pixsize + zoom_pixborder;
	    ypt = y*zoom_pixsize + zoom_pixborder;
	    XFillRectangle (disp, XtWindow(widget_ids[PBITS_WINDOW]),
			    gc_solid, xpt, ypt, wd, ht);
	    bvals[y*PATTERN_WD +x] = new_fg;

/* Refresh Pattern window */
	    Refresh_Pattern_Sample();
	}
    }
}	                                         


/*   
 *
 * ROUTINE:  Epat_Pressed_Button
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button down events
 *
 */
static void Epat_Pressed_Button( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;
{
int value;
int fg;
GC gc_copy;

    prv_x = event->x/(zoom_pixsize);
    prv_y = event->y/(zoom_pixsize);
/* Set the correct foreground color */
    value = bvals[ prv_y*PATTERN_WD + prv_x ]; 
    if( value == 0 )
	new_fg = 1;
    else
	new_fg = 0;

/* Draw into bitmap */
    gc_copy = Get_GC( GC_D1_COPY );
    XSetForeground( disp, gc_copy, new_fg );
    XDrawPoint( disp, stipmap, gc_copy, prv_x, prv_y );
    Draw_Fatbit( prv_x, prv_y );
    drawing_line = TRUE;
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
static void Epat_Released_Button( w, event, params, num_params )
	Widget  w;
	XButtonReleasedEvent *event;
	char **params;
	int	num_params;
{

	drawing_line = FALSE;
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
static void Epat_Moved_Mouse( w, event, params, num_params )
	Widget  w;
	XMotionEvent *event;
	char **params;
	int	num_params;
{                                  

	if( drawing_line ){
		new_x = event->x/zoom_pixsize;
		new_y = event->y/zoom_pixsize;
		if( (prv_x != new_x) || (prv_y != new_y) ){
			XDrawPoint( disp, stipmap, Get_GC(GC_D1_COPY), new_x, new_y );
			prv_x = new_x;
			prv_y = new_y;
			Draw_Fatbit( new_x, new_y );		
			}
		}
		
}

#define NUM_ATABLE2_ENTRIES 4
static XtActionsRec action_table2[] =
{
	{"Epat_Pressed_Button",		(XtActionProc)Epat_Pressed_Button },
	{"Epat_Released_Button",	(XtActionProc)Epat_Released_Button},
	{"Epat_Moved_Mouse",		(XtActionProc)Epat_Moved_Mouse},
	{"Refresh_Edit_Pattern",	(XtActionProc)Refresh_Edit_Pattern}
};
/*
 *
 * ROUTINE:  Create_Edit_Pattern_Dialog
 *
 * ABSTRACT:  Initialize the Edit_Pattern_Dialog box
 *
 */                                                       
Create_Edit_Pattern_Dialog()
{
Arg args[10];
int argcnt;
int i, j;                                      

    Set_Cursor_Watch (pwindow);
    if( pindex >= 0 ){
	if( !edit_pat_dialog ){
/* Intialize pixmaps used to create new pattern */
	    stipmap = XCreatePixmap (disp, DefaultRootWindow(disp), PATTERN_WD, 
				     PATTERN_HT, 1);
	    if (stipmap)
	    {
		if (bvals != NULL)
		{
		    XtFree ((char *)bvals);
		    bvals = NULL;
		}
		bvals = (long *)XtMalloc (sizeof(long)*PATTERN_WD*PATTERN_HT);
		XtAddActions ( action_table2, NUM_ATABLE2_ENTRIES );
		if (Fetch_Widget ("edit_pattern_dialog_box", main_widget,
				  &edit_pat_dialog) != MrmSUCCESS)
		    DRM_Error ("can't fetch edit pattern dialog");

/* center widgets in adb */
	    Set_Attribute (widget_ids[EPAT_OK_BUTTON], XmNleftOffset,
			  - (XtWidth (widget_ids[EPAT_OK_BUTTON]) / 2));
	    Set_Attribute (widget_ids[EPAT_CANCEL_BUTTON], XmNleftOffset,
			  - (XtWidth (widget_ids[EPAT_CANCEL_BUTTON]) / 2));

/* define the cursor for the window */
		XtManageChild(edit_pat_dialog);  
		Set_Cursor( XtWindow(widget_ids[PBITS_WINDOW]), PENCIL );
	    }
	    else {
		Display_Message ("T_NO_MEM_FOR_EDIT_PAT");
	    }
	}
	if (edit_pat_dialog) {
/* make sure pattern sample updates properly */
	    if (stipmap != 0) {
		XFreePixmap (disp, stipmap);
		stipmap = 0;
	    }
	    stipmap = XCreatePixmap (disp, DefaultRootWindow(disp), PATTERN_WD, 
				     PATTERN_HT, 1);
/* Copy current pattern to scratch area */
	    XCopyArea (disp, patterns[pindex], stipmap, Get_GC(GC_D1_COPY), 0,
		       0, PATTERN_WD, PATTERN_HT, 0, 0); 
	
	    XtManageChild(edit_pat_dialog);
	}
    }
    else
	Display_Message ("T_NO_EDIT_PATTERN");
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Delete_Edit_Pattern_Dialog
 *
 * ABSTRACT:  Handles buttons and deletes dialog box if necessary
 *
 */                                                       
void Delete_Edit_Pattern_Dialog(w, button_id, reason)
    Widget   w;
    int	     *button_id;
    int	     reason;
{
    if (*button_id == EPAT_OK_ID) {
	XFreePixmap (disp, patterns[pindex]);
	patterns[pindex] = stipmap;
	stipmap = 0;
/* Update the pattern menu */
	Edited_Pattern();
    }
    else {
/* reset fill stipple GC */
	XSetStipple (disp, Get_GC (GC_MD_FILL), fill_stipple);
    }
}
