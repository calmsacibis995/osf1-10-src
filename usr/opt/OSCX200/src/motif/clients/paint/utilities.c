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
static char *BuildSystemHeader= "$Id: utilities.c,v 1.1.2.3 92/12/11 08:36:23 devrcs Exp $";
#endif		/* BuildSystemHeader */
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
**   DECpaint - DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   UTILITIES contains miscellaneous routines that are used by the other
**   modules.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      10/7/88
**      Always refresh zoom magnifier, even when rubberbanding
**
**      jj      10/12/88
**      Move_Picture (change zoom coordinates before grid refresh).
**
**      dl      10/19/88
**      Use proper select cursor, routine to set default type
**
**	td	09/28/93
**    	Changed data type of wd & ht from 'unsigned int' to 'int' in 
**	MY_XPutImage to allow for proper < 0 checking.
**
**--
*/           
#include <string.h>
#include "paintrefs.h"
#ifndef VMS
#include <X11/decwcursor.h>
#else
#include <decw$cursor.h>
#endif
#include <X11/cursorfont.h>

extern Force_Return_On_Failure();

static int losing_data = FALSE;
static int new_grid_size;
static int expand_image_use_fg_bg = FALSE;

#include "cursors.h"

/*                 
 *
 * ROUTINE:  sign
 *
 * ABSTRACT: 
 *
 *  This function returns the sign of the given argument.
 *
 */
int sign(j)
int j;
{
    if (j > 0) return (1);
    else if (j < 0) return (-1);
    else return (0);
}

/*
 *
 * ROUTINE:  Invert_Bitmap
 *
 * ABSTRACT: 
 *
 *  Invert the given depth 1 pixmap.  The bitmap is modified.
 *
 */
void Invert_Bitmap( bitmap, wd, ht )
Pixmap bitmap;
int wd, ht;
{
	XFillRectangle( disp, bitmap, Get_GC(GC_D1_INVERT), 0, 0, wd, ht);
}

/*
 *
 * ROUTINE:  Set_Cursor_Watch
 *
 * ABSTRACT: 
 *
 *  Set the cursor in the given window depending on the action.
 *
 */
void Set_Cursor_Watch( w )
Window w;
{
    static Cursor watch;
    Font cfont;
    int wait_c;
    static int first_time = TRUE;
    static XColor wait_fcolor; /*  = { 0, 65535, 0, 0 }; */
    static XColor wait_bcolor; /* = { 0, 65535, 65535, 65535 }; */
    XColor dummy;


    if( first_time )
    {
	XLookupColor (disp, default_colormap, "black", &dummy, &wait_fcolor);
	XLookupColor (disp, default_colormap, "white", &dummy, &wait_bcolor);

	cfont = XLoadFont(disp, "decw$cursor");
	if (!cfont) return;
	wait_c = decw$c_wait_cursor;
	watch = XCreateGlyphCursor( disp,
	cfont, cfont, wait_c, wait_c + 1, &wait_fcolor, &wait_bcolor);
	XUnloadFont(disp, cfont);
	first_time = FALSE;
    }

    XDefineCursor( disp, w, watch );

    if( zoomed && (w == pwindow) )
	XDefineCursor( disp, XtWindow(zoom_widget), watch );

    if (w == pwindow)
	XDefineCursor (disp, XtWindow (toplevel), watch);

    XFlush (disp);
}

/*
 *
 * ROUTINE:  Set_Cursor
 *
 * ABSTRACT: 
 *
 *  Set the cursor in the given window depending on the action.
 *  Different devices treat fg/bg differently:
 *  VSII - disregards foreground/background, so it doesn't really
 *         matter.
 *  VSII/GPX and VS2000 - works ok
 *
 */
void Set_Cursor( w, action )
Window w;
int action;
{
#define C_CROSS 0
#define C_PENCIL 1
#define C_FLOOD 2
#define C_SPRAYCAN 3
#define C_TEXT 4
#define C_ERASE 5
#define C_POINTER 6
#define C_BRUSH 7
#define C_MOVE 8
#define C_DROPPER 9
#define NUM_CURSORS 10
static int first_time = TRUE;
static Cursor cursors[NUM_CURSORS];
Cursor new_cursor;
int i;
Font cfont;
Pixmap pencil, pencil_mask;
Pixmap dropper, dropper_mask;
Pixmap spraycan, spraycan_mask;
Pixmap brush, brush_mask;
Pixmap cross, cross_mask;
Pixmap flood, flood_mask;
Pixmap erase, erase_mask;
Pixmap text, text_mask;
XColor dummy;

    if( first_time )
    {
	for( i = 0; i < NUM_CURSORS; ++i )
	    cursors[i] = 0;

	XQueryColor (disp, default_colormap, &cursor_fg);
	XQueryColor (disp, default_colormap, &cursor_bg);

/*
	XLookupColor (disp, default_colormap, "black", &dummy, &cursor_fg);
    	XLookupColor (disp, default_colormap, "white", &dummy, &cursor_bg);
*/
	first_time = FALSE;
    }

    switch ( action ) {
	case LINE: 
	case STROKE: 
	case RECTANGLE: 
	case SQUARE :
	case ARC :
	case POLYGON :
	case CIRCLE :
	case ELLIPSE :
	    if( cursors[C_CROSS] == (Cursor)0) {
		cross = XCreatePixmapFromBitmapData (disp, btmap, cross_bits,
			    cross_width, cross_height, 1, 0, 1);
		cross_mask = XCreatePixmapFromBitmapData (disp, btmap,
				 cross_mask_bits, cross_mask_width,
				 cross_mask_height, 1, 0, 1);
		if (cross && cross_mask) {
		    cursors[C_CROSS] = XCreatePixmapCursor (disp, cross,
					   cross_mask, &cursor_fg, &cursor_bg,
					   cross_xhot, cross_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (cross)
		{
		    XFreePixmap (disp, cross);
		    cross = 0;
		}
		if (cross_mask)
		{
		    XFreePixmap (disp, cross_mask);
		    cross_mask = 0;
		}
	    }
	    new_cursor = cursors[C_CROSS];
	    break;                   

	case PENCIL :
	    if( cursors[C_PENCIL] == (Cursor)0) {
		pencil = XCreatePixmapFromBitmapData (disp, btmap,
			     pencil_bits, pencil_width, pencil_height, 1, 0, 1);
		pencil_mask = XCreatePixmapFromBitmapData (disp, btmap,
				  pencil_mask_bits, pencil_mask_width,
				  pencil_mask_height, 1, 0, 1);
		if (pencil && pencil_mask) {
		    cursors[C_PENCIL] = XCreatePixmapCursor (disp, pencil, 
					    pencil_mask, &cursor_fg, &cursor_bg,
					    pencil_xhot, pencil_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (pencil)
		{
		    XFreePixmap (disp, pencil);
		    pencil = 0;
		}
		if (pencil_mask)
		{
		    XFreePixmap (disp, pencil_mask);
		    pencil_mask = 0;
		}
	    }
	    new_cursor = cursors[C_PENCIL];
	    break;                   

	case TEXT :
	    if( cursors[C_TEXT] == (Cursor)0) {
		text = XCreatePixmapFromBitmapData (disp, btmap,
			   text_bits, text_width, text_height, 1, 0, 1 );
		text_mask = XCreatePixmapFromBitmapData (disp, btmap,
				text_mask_bits, text_mask_width,
				text_mask_height, 1, 0, 1);
		if (text && text_mask) {
		    cursors[C_TEXT] = XCreatePixmapCursor (disp, text,
					  text_mask,&cursor_fg, &cursor_bg,
					  text_xhot, text_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (text)
		{
		    XFreePixmap (disp, text);
		    text = 0;
		}
		if (text_mask)
		{
		    XFreePixmap (disp, text_mask);
		    text_mask = 0;
		}
	    }
	    new_cursor = cursors[C_TEXT];
	    break;                   

	case ERASE :
	    if( cursors[C_ERASE] == (Cursor)0) {
		erase = XCreatePixmapFromBitmapData (disp, btmap,
			    erase_bits, erase_width, erase_height, 1, 0, 1);
		erase_mask = XCreatePixmapFromBitmapData (disp, btmap,
				 erase_mask_bits, erase_width, erase_height, 1,
				 0, 1);
		if (erase && erase_mask) {
		    cursors[C_ERASE] = XCreatePixmapCursor (disp, erase,
					   erase_mask, &cursor_fg, &cursor_bg,
					   erase_xhot, erase_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (erase)
		{
		    XFreePixmap (disp, erase);
		    erase = 0;
		}
		if (erase_mask)
		{
		    XFreePixmap (disp, erase_mask);
		    erase_mask = 0;
		}
	    }
	    new_cursor = cursors[C_ERASE];
	    break;                   

    	case FLOOD :
	    if( cursors[C_FLOOD] == (Cursor)0) {
		flood = XCreatePixmapFromBitmapData (disp, btmap,
			    flood_bits, flood_width, flood_height, 1, 0, 1);
		flood_mask = XCreatePixmapFromBitmapData (disp, btmap,
				 flood_mask_bits, flood_mask_width,
				 flood_mask_height, 1, 0, 1);
		if (flood && flood_mask) {
		    cursors[C_FLOOD] = XCreatePixmapCursor (disp, flood,
					   flood_mask, &cursor_fg, &cursor_bg,
					   flood_xhot, flood_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (flood)
		{
		    XFreePixmap (disp, flood);
		    flood = 0;
		}
		if (flood_mask)
		{
		    XFreePixmap (disp, flood_mask);
		    flood_mask = 0;
		}
	    }
	    new_cursor = cursors[C_FLOOD];
	    break;                   

	case SPRAYCAN :
	    if( cursors[C_SPRAYCAN] == (Cursor)0) {
		spraycan = XCreatePixmapFromBitmapData (disp, btmap,
			       spraycan_bits, spraycan_width, spraycan_height,
			       1, 0, 1);
		spraycan_mask = XCreatePixmapFromBitmapData (disp, btmap,
				    spraycan_mask_bits, spraycan_mask_width,
				    spraycan_mask_height, 1, 0, 1);
		if (spraycan && spraycan_mask) {
		    cursors[C_SPRAYCAN] = XCreatePixmapCursor (disp, spraycan,
					       spraycan_mask, &cursor_fg,
					       &cursor_bg, spraycan_xhot,
					       spraycan_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (spraycan)
		{
		    XFreePixmap (disp, spraycan);
		    spraycan = 0;
		}
		if (spraycan_mask)
		{
		    XFreePixmap (disp, spraycan_mask);
		    spraycan_mask = 0;
		}
	    }
	    new_cursor = cursors[C_SPRAYCAN];
	    break;                   

	case BRUSH :
	    if( cursors[C_BRUSH] == (Cursor)0) {
		brush = XCreatePixmapFromBitmapData (disp, btmap,
			    brush_bits, brush_width, brush_height, 1, 0, 1);
		brush_mask = XCreatePixmapFromBitmapData (disp, btmap,
				 brush_mask_bits, brush_mask_width, 
				 brush_mask_height, 1, 0, 1);
		if (brush && brush_mask) {
		    cursors[C_BRUSH] = XCreatePixmapCursor (disp, brush,
					   brush_mask, &cursor_fg, &cursor_bg,
					   brush_xhot, brush_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (brush)
		{
		    XFreePixmap (disp, brush);
		    brush = 0;
		}
		if (brush_mask)
		{
		    XFreePixmap (disp, brush_mask);
		    brush_mask = 0;
		}
	    }
	    new_cursor = cursors[C_BRUSH];
	    break;                   

	case CROP :
	case SELECT_AREA : 
	case SELECT_RECT :
	    if (cursors[C_POINTER] == (Cursor)0) {
		cfont = XLoadFont (disp, "decw$cursor");
		if (!cfont) return;
		cursors[C_POINTER] = XCreateGlyphCursor (disp, cfont,
					 cfont, decw$c_select_cursor,
					 decw$c_select_cursor + 1, &cursor_fg,
					 &cursor_bg);
		XUnloadFont (disp, cfont);
	    }
/*
	    cursors[C_POINTER] = XCreateFontCursor (disp, XC_left_ptr);
*/
	    new_cursor = cursors[C_POINTER];
	    break;                   

	case ZOOM_MOVE :
	case MOVE :
	    if (cursors[C_MOVE] == (Cursor)0) {
		cfont = XLoadFont (disp, "decw$cursor");
		if (!cfont) return;
		cursors[C_MOVE] = XCreateGlyphCursor (disp,
				      cfont, cfont, decw$c_grabhand_cursor,
				      decw$c_grabhand_cursor + 1, 
				      &cursor_fg,&cursor_bg);
		XUnloadFont (disp, cfont);
	    }
	    new_cursor = cursors[C_MOVE];
	    break;                   

	case DROPPER :
	    if( cursors[C_DROPPER] == (Cursor)0) {
		dropper = XCreatePixmapFromBitmapData (disp, btmap,
		    dropper_bits, dropper_width, dropper_height, 1, 0, 1);
		dropper_mask = XCreatePixmapFromBitmapData (disp, btmap,
		    dropper_mask_bits, dropper_mask_width,
		    dropper_mask_height, 1, 0, 1);
		if (dropper && dropper_mask) {
		    cursors[C_DROPPER] = XCreatePixmapCursor (disp, dropper,
			dropper_mask, &cursor_fg, &cursor_bg,
			dropper_xhot, dropper_yhot);
		}
		else {
		    Display_Message ("T_NO_MEM_FOR_CURSOR");
		}
		if (dropper)
		{
		    XFreePixmap (disp, dropper);
		    dropper = 0;
		}
		if (dropper_mask)
		{
		    XFreePixmap (disp, dropper_mask);
		    dropper_mask = 0;
		}
	    }
	    new_cursor = cursors[C_DROPPER];
	    break;                   

    }
    XUndefineCursor (disp, XtWindow (toplevel));
    if ((action == NO_ACTION) || (new_cursor == (Cursor)0))
	XUndefineCursor (disp, w);
    else {
	XDefineCursor (disp, w, new_cursor);
	if (zoomed && (w == pwindow))
	    XDefineCursor (disp, XtWindow(zoom_widget), new_cursor);
    }			
    XFlush (disp);
}

/*
 *
 * ROUTINE:  DRM_Error
 *
 * ABSTRACT: 
 *
 *  There was an Error with some uil/drm stuff.  Print error message,
 *
 */
void DRM_Error (problem_string)
    char *problem_string;
{
    printf ("%s\n", problem_string);
    exit(1);
}

/*
 *
 * ROUTINE:  Set_Attribute
 *
 * ABSTRACT: 
 *
 *  Alter a value in the widgets structure
 *
 */
void Set_Attribute (w, attr, value)			/* tweak a single value */
  Widget w;					/* in a widget */
  char *attr;
  caddr_t value;
{
    Arg al[2];

    XtSetArg (al[0], attr, value );
    XtSetValues (w, al, 1);
}

/*
 *
 * ROUTINE:  Fetch_Set_Attribute
 *
 * ABSTRACT: 
 *
 *  Alter a value in the widgets structure with resource fetched from UID file.
 *
 */
void Fetch_Set_Attribute (w, attr, value)	/* tweak a single value */
  Widget w;					/* in a widget */
  char *attr;
  char *value;
{
    int error;
    Arg al[2];

    XtSetArg (al[0], attr, value );
    error = MrmFetchSetValues (s_DRMHierarchy, w, al, 1);
/*    if (MrmFetchSetValues (s_DRMHierarchy, w, al, 1) != MrmSUCCESS) */
    if (error != MrmSUCCESS)
	DRM_Error ("couldn't Fetch/Set value");
    
}

/*
 *
 * ROUTINE:  Get_Attribute
 *
 * ABSTRACT: 
 *
 *  Fetch a value in the widgets structure
 *
 */
void Get_Attribute (w, attr, value)			
  Widget w;
  char *attr;
  caddr_t value;
{
    Arg al[2];

    XtSetArg (al[0], attr, value );
    XtGetValues (w, al, 1);
}

/*
 *
 * ROUTINE:  Snap_To_Grid
 *
 * ABSTRACT: 
 *
 *  If gravity is on, use the grid points as points for the shape
 *  x, y, ret_x, ret_y are in image coordinates, 
 */
void Snap_To_Grid( x, y, ret_x, ret_y )
int x,y;
int *ret_x, *ret_y;
{
	float bias;
	int grid_xoff = (grid_size - (picture_x % grid_size)) % grid_size;
	int grid_yoff = (grid_size - (picture_y % grid_size)) % grid_size;

	if( (current_action == LINE) ||
	  ( current_action == RECTANGLE) ||
	  ( current_action == SQUARE )||
		( current_action == ELLIPSE) ||
		( current_action == CIRCLE ) ||
		( current_action == ARC) ||
		( current_action == POLYGON) ||
		( current_action == SELECT_RECT) ){

/* find snap point */
			bias = grid_size/2;
			x += bias - grid_xoff;
			if (x < 0)
			    x -= grid_size;
			x = (x/grid_size) * grid_size + grid_xoff;

			y += bias  - grid_yoff;
			if (y < 0)
			    y -= grid_size;
			y = (y/grid_size) * grid_size + grid_yoff;
		}
		*ret_x = x;
		*ret_y = y;
}

/*
 *
 * ROUTINE:  Rectangle_Intersect
 *
 * ABSTRACT: 
 *
 *  Given x, y, wd, ht, of two rectangles which intersect, retrun the x, y, wd,
 *  ht, of the intersection.
 *
 */
Rectangle_Intersect (x1, y1, wd1, ht1, x2, y2, wd2, ht2, x3, y3, wd3, ht3)
    int x1, y1, wd1, ht1, x2, y2, wd2, ht2, *x3, *y3, *wd3, *ht3;
{
    *x3 = MAX (x1, x2);
    *y3 = MAX (y1, y2);
    *wd3 = MIN ((x1 + wd1), (x2 + wd2)) - *x3;
    *ht3 = MIN ((y1 + ht1), (y2 + ht2)) - *y3;
}

/*
 *
 * ROUTINE:  Rectangle_Not_Intersect
 *
 * ABSTRACT: 
 *
 *  Given x, y, wd, ht, of two rectangles, retrun the x, y, wd, ht, of the
 *  (up to 4) areas which rectangle 2 falls outside of rectangle 1.
 *
 */
Rectangle_Not_Intersect (x1, y1, wd1, ht1, x2, y2, wd2, ht2, out_rects)
    int x1, y1, wd1, ht1, x2, y2, wd2, ht2;
    MyRect *out_rects;
{
    int i;

    for (i = 0; i < 4; i++) {
	out_rects[i].wd = 0;
    }

    if (y2 < y1) {
	out_rects[0].x = x2;
	out_rects[0].y = y2;
	out_rects[0].wd = wd2;
	out_rects[0].ht = MIN (ht2, y1-y2);
    }
    if ((y2 + ht2) > (y1 + ht1)) {
	out_rects[1].x = x2;
	out_rects[1].y = MAX (y2, (y1 + ht1));
	out_rects[1].wd = wd2;
	out_rects[1].ht = y2 + ht2 - out_rects[1].y;
    }
    if (((y2 + ht2) > y1) && (y2 < (y1 + ht1))) {
	if (x2 < x1) {
	    out_rects[2].x = x2;
	    out_rects[2].y = MAX (y2, y1);
	    out_rects[2].wd = MIN (wd2, (x1 - x2));
	    out_rects[2].ht = MIN ((y2 + ht2), (y1 + ht1)) - out_rects[2].y;
	}
	if ((x2 + wd2) > (x1 + wd1)) {
	    out_rects[3].x = MAX (x2, x1 + wd1);
	    out_rects[3].y = MAX (y2, y1);
	    out_rects[3].wd = x2 + wd2 - out_rects[3].x;
	    out_rects[3].ht = MIN ((y2 + ht2), (y1 + ht1)) - out_rects[3].y;
	}
    }
}

/*
 *
 * ROUTINE:  Draw_Zoom_Grid
 *
 * ABSTRACT: 
 *
 *  Displays the grid as rectangles in the given color. 
 *  x,y are coordinates in the image.   This routine is used to either
 *  draw or erase the zoom grid depending on the given color.
 *  
 */
void Draw_Zoom_Grid (x, y, wd, ht)
int x, y, wd, ht;
{
int i, j;
int beg_x, beg_y;
int end_x, end_y;                          
int grid_xoff = (grid_size - (picture_x % grid_size)) % grid_size;
int grid_yoff = (grid_size - (picture_y % grid_size)) % grid_size;
GC gc_draw;
long *bp;

    gc_draw = Get_GC( GC_SD_SOLID );
    XSetForeground (disp, gc_draw, colormap[BLACK].pixel);

    beg_x = ceil((float)(x - grid_xoff)/grid_size) * grid_size + grid_xoff;
    beg_y = ceil((float)(y - grid_yoff)/grid_size) * grid_size + grid_yoff;
    end_x = x + wd;
    end_y = y + ht;


    for( i = beg_x; i < end_x; i+=grid_size )
	for( j = beg_y; j < end_y; j+=grid_size ) {
	    bp = bitvals + (j - zoom_yorigin) * zoom_width + (i - zoom_xorigin);
	    if (*bp != colormap[BLACK].pixel) {
		XFillRectangle (disp, zoom, gc_draw,
				IX_TO_ZX(i) + zoom_pixborder,
				IY_TO_ZY(j) + zoom_pixborder,
				zoom_pixsize - zoom_pixborder,
				zoom_pixsize - zoom_pixborder);
		*bp = colormap[BLACK].pixel;
	    }
	}
}            

/*
 *
 * ROUTINE:  Display_Grid
 *
 * ABSTRACT: 
 *
 *  Displays a the grid as single points, Draw only the grid points
 *  specified by the given rectangle.  x,y are coordinates in the image.
 *  
 */
void Display_Grid( x, y, wd, ht )
int x, y, wd, ht;
{
int i, j;
int beg_x, beg_y;
int end_x, end_y;
int grid_xoff = (grid_size - (picture_x % grid_size)) % grid_size;
int grid_yoff = (grid_size - (picture_y % grid_size)) % grid_size;
GC gc_grid;

	gc_grid = Get_GC (GC_SD_SOLID);
	XSetForeground (disp, gc_grid, colormap[BLACK].pixel);
	beg_x = ceil((float)(x - grid_xoff)/grid_size) * grid_size + grid_xoff;
	beg_y = ceil((float)(y - grid_yoff)/grid_size) * grid_size + grid_yoff;
	end_x = x + wd;
	end_y = y + ht;
	for( i = beg_x; i < end_x; i+=grid_size )
		for( j = beg_y; j < end_y; j+=grid_size )
			XDrawPoint( disp, pwindow, gc_grid, IX_TO_PWX(i), IY_TO_PWY(j) );

	if( zoomed ){
/*
		Refresh_Zoom_View( zoom_xorigin, zoom_yorigin, zoom_width, zoom_height);
*/
		Draw_Zoom_Grid( zoom_xorigin, zoom_yorigin, zoom_width, zoom_height);
		}
}            

/*
 *
 * ROUTINE:  Copy_Bitmap
 *
 * ABSTRACT: 
 *
 * Copies one bitmap to another bitmap
 *
 */
void Copy_Bitmap( src, dest, src_x, src_y, src_wd, src_ht, dest_x, dest_y)
    Drawable src, dest;
    int src_x, src_y, src_wd, src_ht;
    int dest_x, dest_y;
{
GC gc_copy;

    gc_copy = Get_GC( GC_SD_COPY );

    if (src_x < 0) {
	dest_x -= src_x;
	src_wd += src_x;
	src_x = 0;
    }
    if (src_y < 0) {
	dest_y -= src_y;
	src_ht += src_y;
	src_y = 0;
    }

    if ((src_wd <= 0) || (src_ht <= 0))
	return;

    if (pdepth == 1)
        XCopyPlane (disp, src, dest, gc_copy, src_x, src_y, src_wd, src_ht,
                    dest_x, dest_y, bit_plane);
    else
        XCopyArea (disp, src, dest, gc_copy, src_x, src_y, src_wd, src_ht,
                   dest_x, dest_y);
}

/*
 *
 * ROUTINE:  Refresh_Select_Highlight
 *
 * ABSTRACT: 
 *
 * Refresh the select highlight if it was obscured. The given x, y 
 * values are in image coordinates.
 *
 */
void Refresh_Select_Highlight( x, y, wd, ht )
int x, y, wd, ht;
{
/* If the given rectangle intersects with the selected area, stop highlighting
 * refresh the area, and start highlighting again.
 */
/*
	if( !rbanding ){
*/
		if( Intersecting_Rectangles( x, y, wd, ht, 
				select_x, select_y, select_width, select_height ) ){
/*		 	Stop_Highlight_Blink(); */
			Copy_Bitmap( picture, pwindow, select_x-1, select_y-1, 
				     select_width+2, select_height+2,
				     IX_TO_PWX(select_x-1), IY_TO_PWY(select_y-1) );
/*		 	Start_Highlight_Blink(); */
			}
/*
		}		
*/
}

/*
 *
 * ROUTINE:  Refresh_Picture
 *
 * ABSTRACT: 
 *
 * Refresh the picture window.  The given x, y values are in image 
 *  coordinates.
 *
 */
void Refresh_Picture (x, y, wd, ht)
    int x, y, wd, ht;
{
    if (!((wd == 0) && (ht == 0))) {
	if (picture)
	    Copy_Bitmap (picture, pwindow, x, y, wd, ht, IX_TO_PWX(x),
			 IY_TO_PWY(y));

	if (grid_on)
	    Display_Grid (x, y, wd, ht);

	if (zoomed && (current_action != ZOOM_MOVE)) {
	    if (current_action != MOVE)	
		Refresh_Magnifier (x, y, wd, ht);
	    if (!window_exposure) {
		if ((current_action != MOVE) || (ifocus == zoom_widget)) 
		    Refresh_Zoom_View (x, y, wd, ht);
	    }
	}
    }
    refresh_picture_pending = FALSE;
}

/*
 *
 * ROUTINE:  Delete_Grid
 *
 * ABSTRACT: 
 *
 *  Deletes the grid from the picture window and zoom window.
 *  
 */
void Delete_Grid()
{
	Refresh_Picture( pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht );
}

/*
 *                                      
 * ROUTINE:  Get_Zoomed_Position
 *
 * ABSTRACT: 
 *
 *  If input is being accepted from the zoom widget, calculate the 
 *  correct place to position the cursor on the fatpixel.
 *  true_x, and true_y are the current position of the pointer
 *  x and y are the values that are returned and should be used for rubberbanding
 *           
 */
void Get_Zoomed_Position( true_x, true_y, x, y )
int true_x, true_y;
int *x, *y;
{
int tmp_x, tmp_y;
int pic_x, pic_y;

	tmp_x = true_x/zoom_pixsize;
	tmp_y = true_y/zoom_pixsize;
    
/* If the grid is on, translate into picture coordinates, find the nearest 
 * grid point, then translate back into zoom coordinates.
 */
	if( grid_on ){
		Snap_To_Grid( ZX_TO_IX(true_x), ZY_TO_IY(true_y), &tmp_x, &tmp_y );
		tmp_x = tmp_x - zoom_xorigin;
		tmp_y = tmp_y - zoom_yorigin;
		}

/* When drawing line, center in fatbit, otherwise position the echo
 * in the upper left corner of the fatbit.  
 */
	if( (current_action == LINE) ){
		/* position in center of cell */
		*x = tmp_x*zoom_pixsize + zoom_pixsize/2;		
		*y = tmp_y*zoom_pixsize + zoom_pixsize/2;		
		}
	else{
		/* position in the corner of the cell */
		*x = tmp_x*zoom_pixsize;
 		*y = tmp_y*zoom_pixsize;
		}
}

/*
 *
 * ROUTINE:  Current_Position
 *  
 * ABSTRACT: 
 *
 * Returns the current position according to any drawing constraints
 *
 */
void Current_Position( x, y )
	int *x, *y;
{                
	int tmp_x, tmp_y;

	tmp_x = *x;
	tmp_y = *y;

	true_ptr_x = *x;
	true_ptr_y = *y;
/* if the user is in zoomed mode, calculate the current position by
 * determining which fatpixel the cursor is under.
 */
	if( ifocus == zoom_widget ){
		Get_Zoomed_Position( tmp_x, tmp_y, x, y );
		}		
 	else{
		if( grid_on ){
			Snap_To_Grid( PWX_TO_IX(tmp_x), PWY_TO_IY(tmp_y), &tmp_x, &tmp_y );
			tmp_x = IX_TO_PWX( tmp_x );
			tmp_y = IY_TO_PWY( tmp_y );
			}
		*x = tmp_x;
		*y = tmp_y;
		}
}

/*
 *
 * ROUTINE:  Distance
 *
 * ABSTRACT: 
 *
 *  This is based on the Moler-Morrison algorithm, as presented in
 *  Tom Duff's SIGGRAPH '84 Tutorial: "Numerical Methods in Computer
 *  Graphics".  It finds the distance between 2 points.
 *
 */     
double Distance ( x1, y1, x2, y2 )
int x1, y1, x2, y2;
{
int i;
double d;
double p, q, r, s;

	p = x2 - x1;
	q = y2 - y1;
	if( p < 0 )
		p = - p;
	if (q < 0)
		q = - q;
	
	i = MAX( q, p );
	q = MIN( q, p );
	p = i;
	if (p != 0.0 ){
		for( i = 1; i <=4; ++i ){
			r = q / p;
			r = r*r;
			s = r + 4;
			r = r / s;
			p = p + (2 * r * p);
			q = q * r;    
			}
		}
	return(p);
}

/*
 *
 * ROUTINE:  Get_UIL_Value
 *
 * ABSTRACT: 
 *
 *  Get the value in the UID file indexed by the index string and place it in
 *   the value character array. Return 
 *
 */
int Get_UIL_Value (buffer, index)
    char *buffer;
    char *index;
{
    int temp = SUCCESS;
    MrmCode data_type; /* JJ-MOTIF */
    caddr_t value;

/* JJ-MOTIF -> */
    if (MrmFetchLiteral (s_DRMHierarchy, index, XtDisplay (toplevel), &value,
                         &data_type) != MrmSUCCESS) {
	temp = FAILURE;
    }
    else
    {
	strcpy (buffer, value);
	XtFree ((char*)value);
	value = NULL;
    }
/*
    MRMResourceContextPtr DRMResource;
    MrmGetResourceContext (NULL, NULL, 0, &DRMResource);
    if (UrmHGetIndexedLiteral (s_DRMHierarchy, index, DRMResource) ==
	MrmSUCCESS) {
	switch (MrmRCType (DRMResource)) {
	    case RGMrTypeChar8 :
		strcpy (value, MrmRCBuffer (DRMResource));
		break;
	    case RGMrTypeCString :
		strcpy (value, Extract_String (MrmRCBuffer (DRMResource)));
		break;
	}
    }
    else
	temp = FAILURE;
    MrmFreeResourceContext (DRMResource);
*/
/* <- JJ-MOTIF */ 

    return (temp);
}

/*
 *
 * ROUTINE:  ERROR
 *
 * ABSTRACT: 
 *
 *  This routine handles display of error messages.
 *
 */     
void ERROR( fatal, errmsg )
int fatal;
char *errmsg;
{
	printf( "%s", errmsg );
	if( fatal )
		exit(1);
}

/*
 *
 * ROUTINE:  Error_Handler
 *
 * ABSTRACT: 
 *
 *  Handles errors from the X server
 *
 */     
Error_Handler( d, event )
Display *d;
XErrorEvent *event;
{
char msg[80], error_string[80];
int len;

#ifdef EPIC_CALLABLE
	if (session) {
	    if (AiXErrorHandler (&session, d, event) == AI_SUCCESS)
		return (1);
	}
#endif

	XGetErrorText( disp, event->error_code, msg, 80 );
/*
	if (Get_UIL_Value (error_string, "T_ERROR_STRING") != SUCCESS)
            DRM_Error ("Could not fetch value from UID file");
	printf( error_string, msg );
*/
	printf ("Paint Error: %s\n", msg);
}

/*
 *
 * ROUTINE:  Map_Zoom_Points
 *
 * ABSTRACT: 
 *              
 *
 * This routine takes the points input from the zoom window,
 * and translates them into the picture windows coordinates.
 *
 */
void Map_Zoom_Points()
{
	int i;

		for( i = 0; i < numpts; ++i ){
			points[i].x = ZX_TO_PWX(points[i].x);
			points[i].y = ZY_TO_PWY(points[i].y);
/*
printf( "points[%d].x= %d, points[%d].y=%d\n", i, points[i].x, i, points[i].y);
*/
			}
		shape_xmin = ZX_TO_PWX(shape_xmin);
		shape_ymin = ZY_TO_PWY(shape_ymin);
		shape_wd = shape_wd/zoom_pixsize; 
		shape_ht = shape_ht/zoom_pixsize;
}

/*
 *
 * ROUTINE:  Map_Picture_Points
 *
 * ABSTRACT: 
 *
 *
 * This routine takes the points input from the picture window,
 * and translates them into the picture's true "image" coordinates.
 *
 */
void Map_Picture_Points()
{
	int i;

	if( (pic_xorigin != 0) || (pic_yorigin != 0) ){
		for( i = 0; i < numpts; ++i ){
			points[i].x = PWX_TO_IX(points[i].x);
			points[i].y = PWY_TO_IY(points[i].y);
			}
		shape_xmin = PWX_TO_IX(shape_xmin);
		shape_ymin = PWY_TO_IY(shape_ymin);

		if( current_action == ARC ){
			arc_x = PWX_TO_IX(arc_x);
			arc_y = PWY_TO_IY(arc_y);
			}

		if( (current_action == RECTANGLE) || 
		    (current_action == ELLIPSE) ){
			rect_x = PWX_TO_IX( rect_x );
			rect_y = PWY_TO_IY( rect_y );
			}
		}
}

/*
 *
 * ROUTINE:  Angle
 *
 * ABSTRACT: 
 *
 * This returns the angle between two points
 *
 */
double Angle(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
	double theta;

	if (x1 == x2){
		if (y1 < y2)
	  	theta = 270.0;
	  else 
			theta = 90.0;
		}
	else
		{
		theta = atan ( (double)(y1 - y2) / (double)(x2 - x1) ) * DEGREES;
/* Arctan returns a value in the range of PI/2 to -PI/2
 *
 *
 *            |
 *        -p/2|pi/2
 *            |
 *      0     |       0
 * -------------------------
 *      0     |       0
 *            |
 *         p/2|-pi/2
 *            |                                   
 * So offset returned angle accordingly
 */
		if(x2 < x1) 
			theta += 180.0;
		else
		if((x2 > x1) && (y2 > y1))
			theta += 360.0;
		}
		return( theta );
}

/*
 *
 * ROUTINE:  Find_Rectangle
 *
 * ABSTRACT: 
 *
 * Finds the rectangle using x1,y1 and x2,y2
 *
 * OUTPUT: 
 *  rect_x, rect_y, rect_wd, rect_ht     
 */
void Find_Rectangle( x1, y1, x2, y2 )
int x1, y1, x2, y2;
{
int a, b, tmp_x, tmp_y, tmp_width, tmp_height;

		tmp_x = MIN( x1, x2 );
		tmp_y = MIN( y1, y2 );
		tmp_width = MAX( x1, x2 ) - tmp_x;
		tmp_height = MAX( y1, y2 ) - tmp_y;

/* Calculate a square */
		if( alt_shape ){                               
			a = abs( x1 - x2 );
			b = abs( y1 - y2 );
			tmp_width = MAX( a, b );
			tmp_height = tmp_width;
			if( tmp_x < x1 )
				tmp_x = x1 - tmp_width;
			if(tmp_y < y1 )
				tmp_y = y1 - tmp_height;
			}
		rect_x = tmp_x;
		rect_y = tmp_y;
		rect_wd = tmp_width;
		rect_ht = tmp_height;
}

/*
 *
 * ROUTINE:  Get_Rband_Pencil_GC
 *
 * ABSTRACT: 
 *
 * Return the gc used to echo the pencil in the window
 *
 */
GC Get_Rband_Pencil_GC()
{
int value;
XImage *image;
int format;
GC gc_return;

    gc_return = Get_GC( GC_SD_SOLID );

    image = XGetImage (disp, picture, points[0].x, points[0].y, 1, 1,
		       bit_plane, img_format);
    value = XGetPixel (image, 0, 0);
    XDestroyImage (image);

    if (pdepth == 1) {
	if (value == picture_fg)
	    pencil_value = colormap[paint_bg_color].pixel;
/*	    pencil_value = window_bg; */
	else
	    pencil_value = colormap[paint_color].pixel;
/*	    pencil_value = window_fg; */
    }
    else {
	if (value == colormap[paint_color].pixel)
	    pencil_value = colormap[paint_bg_color].pixel;
	else
	    pencil_value = colormap[paint_color].pixel;
    }

    gc_values.foreground = pencil_value;
    XChangeGC( disp, gc_return, GCForeground, &gc_values );
    return( gc_return );
}

/*
 *
 * ROUTINE:  Get_Pencil_GC
 *
 * ABSTRACT: 
 *
 * Return the gc used to draw the pencil in the picture
 *
 */
GC Get_Pencil_GC()
{
int value;
GC gc_return;


/* in case the picture is not the same depth as the window, use 1 and
 * zero to represent foreground and background.
 */
    if( pdepth == 1 ){
	if (pencil_value == colormap[paint_color].pixel)
/*	if (pencil_value == window_fg) */
	    value = 1;
	else
	    value = 0;
    }
    else
	value = pencil_value;
    gc_return = Get_GC( GC_PD_SOLID );
    XSetForeground( disp, gc_return, value );
    return( gc_return );
}	

/*
 *
 * ROUTINE:  Reset_PD_Clip_Rect
 *
 * ABSTRACT: 
 *
 *  Reset the stipple origins of the Picture Depth GC's
 *
 *
 */
void Reset_PD_Clip_Rect ()
{
    XRectangle clip_rect[1];

    clip_rect[0].x = pic_xorigin;
    clip_rect[0].y = pic_yorigin;
    clip_rect[0].width = pwindow_wd;
    clip_rect[0].height = pwindow_ht;

    XSetClipRectangles (disp, Get_GC(GC_PD_SPRAY), 0, 0, clip_rect, 1,
			Unsorted);
    XSetClipRectangles (disp, Get_GC(GC_PD_FILL), 0, 0, clip_rect, 1,
			Unsorted);
    XSetClipRectangles (disp, Get_GC(GC_PD_OUTLINE), 0, 0, clip_rect, 1,
			Unsorted);
    XSetClipRectangles (disp, Get_GC(GC_PD_STROKE), 0, 0, clip_rect, 1,
			Unsorted);
    XSetClipRectangles (disp, Get_GC(GC_PD_ROUND_BRUSH), 0, 0, clip_rect, 1,
			Unsorted);
    XSetClipRectangles (disp, Get_GC(GC_PD_SQUARE_BRUSH), 0, 0, clip_rect, 1,
			Unsorted);
}

/*
 *
 * ROUTINE:  Reset_PD_Stipple_Origin 
 *
 * ABSTRACT: 
 *
 *  Reset the stipple origins of the Picture Depth GC's
 *
 *
 */
void Reset_PD_Stipple_Origin (x, y)
    int x, y;
{
    XSetTSOrigin (disp, Get_GC(GC_PD_SPRAY), x, y);
    XSetTSOrigin (disp, Get_GC(GC_PD_FLOOD), x, y);
    XSetTSOrigin (disp, Get_GC(GC_PD_FILL), x, y);
    XSetTSOrigin (disp, Get_GC(GC_PD_OUTLINE), x, y);
    XSetTSOrigin (disp, Get_GC(GC_PD_STROKE), x, y);
    XSetTSOrigin (disp, Get_GC(GC_PD_ROUND_BRUSH), x, y);
    XSetTSOrigin (disp, Get_GC(GC_PD_SQUARE_BRUSH), x, y);
}

/*
 *
 * ROUTINE:  Reset_SD_Stipple_Origin 
 *
 * ABSTRACT: 
 *
 *  Reset the stipple origins of the Picture Depth GC's
 *
 *
 */
void Reset_SD_Stipple_Origin (x, y)
    int x, y;
{
    XSetTSOrigin (disp, Get_GC(GC_SD_OUTLINE),x, y);
    XSetTSOrigin (disp, Get_GC(GC_SD_STROKE), x, y);
    XSetTSOrigin (disp, Get_GC(GC_SD_ROUND_BRUSH), x, y);
    XSetTSOrigin (disp, Get_GC(GC_SD_SQUARE_BRUSH), x, y);
}

/*
 *
 * ROUTINE:  Offset_Select_Rband
 *
 * ABSTRACT: 
 *
 *  Update the highlight polygon by the delta_x, delta_y
 *  If the select hilight is in the zoom window, don't move it.
 *
 */
void Offset_Select_Rband( )
{
int i;

    if( select_window == pwindow ){
	if (num_hipts != 0){
	    for( i = 0; i < num_hipts; ++i ) {
		hi_points[i].x += xscroll;
		hi_points[i].y += yscroll;
	    }
	}
    }
}

/*
 *
 * ROUTINE:  Create_Bitmap
 *
 * ABSTRACT: 
 *
 * Create and initialize a pixmap with a depth of 1, pixval must
 * be 1 or 0.
 *
 */
Pixmap Create_Bitmap( pixval, width, height )
int pixval;
int width, height;
{
Pixmap tmpmap;
GC gc_bitmap;

    tmpmap = XCreatePixmap( disp, DefaultRootWindow(disp), width, height, 1);
    if (tmpmap) {
	gc_bitmap = Get_GC( GC_D1_COPY );
	XSetForeground( disp, gc_bitmap, pixval );
	XFillRectangle( disp, tmpmap, gc_bitmap, 0, 0, width, height );
    }
    return(tmpmap);
}

/*
 *
 * ROUTINE:  Create_Pdepth_Pixmap
 *
 * ABSTRACT: 
 *
 * Create and initialize a pixmap with a depth of pdepth
 *
 */
Pixmap Create_Pdepth_Pixmap( value, width, height )
long value;
int width, height;
{
Pixmap tmpmap;
GC gc_solid;

	if( pdepth == 1 ) {
		tmpmap = Create_Bitmap( value, width, height );
		}
	else {
		tmpmap = XCreatePixmap (disp, DefaultRootWindow(disp), width,
                                        height, pdepth);
		if (tmpmap) {
		    gc_solid = Get_GC( GC_PD_SOLID );
		    XSetForeground (disp, gc_solid, value );
		    XFillRectangle (disp, tmpmap, gc_solid, 0, 0, width,height);
		}
	}
	return(tmpmap);
}

void Increase_Change_Pix (x, y, wd, ht)
    int x, y, wd, ht;
{
    int old_max_x, old_max_y;

    if ((wd == 0) || (ht == 0))
	return;

    if (Intersecting_Rectangles (0, 0, picture_wd, picture_ht, x, y, wd, ht)) {
	Rectangle_Intersect (0, 0, picture_wd, picture_ht, x, y, wd, ht,
			     &x, &y, &wd, &ht);
    }
    else {
	return;
    }

    if ((wd == 0) || (ht == 0))
	return;

    if (change_pix_wd == 0) {
	change_pix_x = x;
	change_pix_y = y;
	change_pix_wd = wd;
	change_pix_ht = ht;
    }
    else {
	old_max_x = change_pix_x + change_pix_wd;
	old_max_y = change_pix_y + change_pix_ht;
	change_pix_x = MIN (x, change_pix_x);
	change_pix_y = MIN (y, change_pix_y);
	change_pix_wd = MAX (old_max_x, x + wd) - change_pix_x;
	change_pix_ht = MAX (old_max_y, y + ht) - change_pix_y;
    }
}

void Decrease_Change_Pix (x, y, wd, ht)
    int x, y, wd, ht;
{
    int mx, my, cpmx, cpmy;

    if ((wd == 0) || (ht == 0))
	return;

    if (Intersecting_Rectangles (0, 0, picture_wd, picture_ht, x, y, wd, ht)) {
	Rectangle_Intersect (0, 0, picture_wd, picture_ht, x, y, wd, ht,
			     &x, &y, &wd, &ht);
    }
    else {
	return;
    }

    if ((wd == 0) || (ht == 0))
	return;

    mx = x + wd;
    my = y + ht;
    cpmx = change_pix_x + change_pix_wd; 
    cpmy = change_pix_y + change_pix_ht;
    
    if ((change_pix_x >= x) && (change_pix_y >= y) && (mx >= cpmx) &&
	(my >= cpmy)) {
	change_pix_wd = 0;
	change_pix_ht = 0;
	return;
    }

    if ((change_pix_x >= x) && (cpmx <= mx) && (cpmy > y) && (cpmy <= my)) {
	change_pix_ht = y - change_pix_y;
	return;
    }

    if ((change_pix_x >= x) && (cpmx <= mx) && (change_pix_y >= y) && 
	(change_pix_y < my)) {
	change_pix_y = my;
	change_pix_ht = cpmy - my;
	return;
    }

    if ((change_pix_y >= y) && (cpmy <= my) && (cpmx > x) && (cpmx <= mx)) {
	change_pix_wd = x - change_pix_x;
	return;
    }

    if ((change_pix_y >= y) && (cpmy <= my) && (change_pix_x >= x) && 
	(change_pix_x < mx)) {
	change_pix_x = mx;
	change_pix_wd = cpmx - mx;
	return;
    }
}

/*
 *
 * ROUTINE:  Find_Pixmap_Position
 *
 * ABSTRACT: 
 *
 *   Return in x_out and y_out, the new picture_x and picture_y coordinates for
 *   the picture pixmap in the image if x, y are the image coordinates where 
 *   the scroll window should begin.
 *
 */              
void Find_Pixmap_Position (x, y, x_out, y_out)
    int x, y, *x_out, *y_out;
{

    *x_out = x - (int) ((picture_wd - pwindow_wd) / 2);
    *y_out = y - (int) ((picture_ht - pwindow_ht) / 2);
/* make sure pixmap doesn't extend past image boundary */
    if (*x_out < 0)
	*x_out = 0;
    else
	if ((*x_out + (int) picture_wd) > pimage_wd)
	    *x_out = pimage_wd - (int) picture_wd;
    if (*y_out < 0)
	*y_out = 0;
    else
	if ((*y_out + (int) picture_ht) > pimage_ht)
	    *y_out = pimage_ht - (int) picture_ht;

}

/*
 *
 * ROUTINE:  Position_Pixmap
 *
 * ABSTRACT: 
 *
 *   Reposition the Pixmap at the image coordinates x,y
 *
 */              
void Position_Pixmap (x, y)
    int x, y;
{
    int old_pic_x = picture_x;
    int old_pic_y = picture_y;
    MyRect rects[4];
    int i;
    int cp_x, cp_y, cp_wd, cp_ht;

    picture_x = x;
    picture_y = y;

/* If there is a selected piece  change pix should cover the area which
   drawn into the image */
    if (select_on) {
	Rectangle_Not_Intersect (0, 0, picture_wd, picture_ht, select_x,
				 select_y, select_width, select_height, rects);
    }
    if (num_hipts > 0) {
	select_x += (old_pic_x - picture_x);
	select_y += (old_pic_y - picture_y);
	XOffsetRegion (select_region, (old_pic_x - picture_x),
		       (old_pic_y - picture_y));
    }

    Reset_PD_Stipple_Origin (-picture_x, -picture_y);
    change_pix_x += (old_pic_x - picture_x);
    change_pix_y += (old_pic_y - picture_y);

/* If there is a selected piece  change pix should cover the area which got
   drawn into the image */
    if (select_on) {
	for (i = 0; i < 4; i++) {
	    if (rects[i].wd != 0) {
		rects[i].x += (old_pic_x - picture_x);
		rects[i].y += (old_pic_y - picture_y);
		if (Intersecting_Rectangles (picture_x, picture_y,
					     picture_wd, picture_ht,
					     rects[i].x, rects[i].y,
					     rects[i].wd, rects[i].ht)) {
		    Rectangle_Intersect (picture_x, picture_y,
					 picture_wd, picture_ht,
					 rects[i].x, rects[i].y,
					 rects[i].wd, rects[i].ht,
					 &cp_x, &cp_y, &cp_wd, &cp_ht);
		    Increase_Change_Pix (cp_x, cp_y, cp_wd, cp_ht);
		}
	    }
	}
    }

}

/*
 *
 * ROUTINE:  Merge_Image
 *
 * ABSTRACT: 
 *
 *   Merge the source image (sximage) into the destination image (dximage) given
 *   source x, y (sx,sy) destination x, y (dx, dy) and width, height of the
 *   portion to be merged.
 *
 */
void Merge_Image(sx, sy, width, height, dx, dy, sximage, dximage, combo_rule)
    int sx, sy, dx, dy, width, height;
    XImage *sximage, *dximage;
    int combo_rule;
{
    unsigned long image_id, merge_image_id;
    char *image_data, *merge_image_data;
    int image_size, image_size_2;
    int set_index;
    struct PUT_ITMLST set_attributes[7];
    int bytes;
    long scanline_stride, bits_per_pixel, pixel_order, lwidth, lheight;
    long dstroi, srcroi; 
    long rect_buf[4];

/* if region extends past boundary of image then clip it properly to image */
    if (sx < 0) {
	width += sx;
	dx -= sx;
	sx = 0;
    }
    if (sy < 0) {
	height += sy;
	dy -= sy;
	sy = 0;
    }
    if ((sx + width) > sximage->width)
	width = sximage->width - sx;
    if ((sy + height) > sximage->height)
	height = sximage->height - sy;
    if (dx < 0) {
	width += dx;
	sx -= dx;
	dx = 0;
    }
    if (dy < 0) {
	height += dy;
	sy -= dy;
	dy = 0;
    }
    if ((dx + width) > dximage->width)
	width = dximage->width - dx;
    if ((dy + height) > dximage->height)
	height = dximage->height - dy;
    if ((width > 0) && (height > 0)) {
	bits_per_pixel = (pdepth == 1) ? 1 : 8;
	pixel_order = ImgK_StandardPixelOrder; 

/* create image id for image to merge into */
	bytes = dximage->bytes_per_line;
	scanline_stride = 8 * bytes;	/* 8 bits per byte */
	image_data = dximage -> data;
	image_size = bytes * dximage->height;
	lwidth = dximage->width;
	lheight = dximage->height;

	start_set_itemlist(set_attributes, set_index);
	put_set_item (set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item (set_attributes, set_index, Img_BitsPerPixel,
		      bits_per_pixel);
	put_set_item (set_attributes, set_index, Img_PixelsPerLine,
		      lwidth);
	put_set_item (set_attributes, set_index, Img_NumberOfLines,
		      lheight);
	put_set_item (set_attributes, set_index, Img_ScanlineStride,
		      scanline_stride);
	end_set_itemlist (set_attributes, set_index);

	image_id = ImgCreateFrame(set_attributes, image_stype); 

	ISL_ERROR_HANDLER_SETUP
	image_id = ImgImportBitmap (image_id, image_data, image_size,
				      0, 0, 0, 0);
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
	    losing_data = TRUE;
	    Display_Message ("T_LOSING_DATA");
	    return;
        }


/* create image id for image to be merged */
	bytes = sximage->bytes_per_line;
	scanline_stride = 8 * bytes;	/* 8 bits per byte */
	merge_image_data = sximage -> data;
	image_size_2 = bytes * sximage->height;
	lwidth = sximage->width;
	lheight = sximage->height;

	start_set_itemlist(set_attributes, set_index);
	put_set_item (set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item (set_attributes, set_index, Img_BitsPerPixel,
		      bits_per_pixel);
	put_set_item (set_attributes, set_index, Img_PixelsPerLine,
		      lwidth);
	put_set_item (set_attributes, set_index, Img_NumberOfLines,
		      lheight);
	put_set_item (set_attributes, set_index, Img_ScanlineStride,
		      scanline_stride);
	end_set_itemlist (set_attributes, set_index);

	merge_image_id = ImgCreateFrame(set_attributes, image_stype);

	ISL_ERROR_HANDLER_SETUP
	merge_image_id = ImgImportBitmap(merge_image_id, merge_image_data,
					   image_size_2, 0, 0, 0, 0);
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
	    ImgDeleteFrame (merge_image_id);
	    losing_data = TRUE;
	    Display_Message ("T_LOSING_DATA");
	    return;
        }

	make_rect_buf (rect_buf, (long)sx, (long)sy, (long)width, (long)height);
	srcroi = ImgCreateRoi (ImgK_RoitypeRect, (char *)rect_buf,
			         RoiK_RectLength);
	make_rect_buf (rect_buf, (long)dx, (long)dy, (long)width, (long)height);
	dstroi = ImgCreateRoi (ImgK_RoitypeRect, (char *)rect_buf,
				 RoiK_RectLength);
	ISL_ERROR_HANDLER_SETUP
	image_id = ImgCombine (image_id, (struct ROI *)dstroi, merge_image_id, 
			       (struct ROI *)srcroi, 0, combo_rule);
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
	    ImgDeleteFrame (merge_image_id);
	    losing_data = TRUE;
	    Display_Message ("T_LOSING_DATA");
	    return;
        }

	ImgDeleteFrame (merge_image_id);
	ImgExportBitmap (image_id, 0, image_data, image_size, 0, 0, 0, 0);

	ImgDeleteFrame (image_id);
    }
    losing_data = FALSE;
}

/*
 * ROUTINE:  MY_XPutImage
 *
 * ABSTRACT : Workaround to GPX server bug which will not allow XPutImage with
 *	      width greater than 1024-32.  This routine will work recursively.
 */
void MY_XPutImage (disp, d, gc, image, src_x, src_y, dst_x, dst_y, wd, ht)
    Display *disp;
    Drawable d;
    GC gc;
    XImage *image;
    int src_x, src_y;
    int dst_x, dst_y;
    int wd, ht;
{
#define MAX_WD  1024-32
    int wd_1;

/* put in some checking in case get numbers which are out of range - the
   X function can't handle these */

    if (src_x < 0) {
	dst_x -= src_x;
	wd += src_x;
	src_x = 0;
    }
    if ((src_x + wd) > image->width) {
	wd = image->width - src_x;
    }
    if (src_y < 0) {
	dst_y -= src_y;
	ht += src_y;
	src_y = 0;
    }
    if ((src_y + ht) > image->height) {
	ht = image->height - src_y;
    }

    if ((wd <= 0) || (ht <= 0))
	return;

    if  (wd > MAX_WD) { 
	wd_1 = wd / 2;
	MY_XPutImage (disp, d, gc, image, src_x, src_y, dst_x, dst_y, wd_1, ht);
	MY_XPutImage (disp, d, gc, image, src_x + wd_1, src_y, dst_x + wd_1,
		      dst_y, wd - wd_1, ht);
    }
    else
        XPutImage (disp, d, gc, image, src_x, src_y, dst_x, dst_y, wd, ht);
}
			
/*
 * ROUTINE:  MY_XGetSubImage
 *
 * ABSTRACT : Workaround to slow XGetSubImage
 *
 */
void MY_XGetSubImage (disp, d, x, y, wd, ht, plane_mask, format, image,
		      dst_x, dst_y, combo_rule)
    Display *disp;
    Drawable d;
    int x, y;
    unsigned int wd, ht;
    unsigned long plane_mask;
    int format;
    XImage *image;
    int dst_x, dst_y;
    int combo_rule;
{
    XImage *ximage;

    ximage = XGetImage (disp, d, x, y, wd, ht, plane_mask, format );
    if (ximage != NULL)
    {
	if (ximage->bitmap_bit_order != NATIVE_BIT_ORDER)
	    ConvertImageNative (ximage);
	Merge_Image (0, 0, wd, ht, dst_x, dst_y, ximage, image, combo_rule);
#if 0
	XtFree (ximage->data);
#endif
	XDestroyImage (ximage);
    }

    if (losing_data) {
/* if region extends past boundary of image then clip it properly to image */
	if (dst_x < 0) {
	    wd += dst_x;
	    x -= dst_x;
	    dst_x = 0;
	}
	if (dst_y < 0) {
	    ht += dst_y;
	    y -= dst_y;
	    dst_y = 0;
	}
	if ((dst_x + wd) > image->width)
	    wd = image->width - dst_x;
	if ((dst_y + ht) > image->height)
	    ht = image->height - dst_y;
	if ((wd > 0) && (ht > 0)) {
	    XGetSubImage (disp, d, x, y, wd, ht, plane_mask, format, image,
			  dst_x, dst_y);
	}
	losing_data = FALSE;
    }
}

/*
 *
 * ROUTINE:  Update_Image
 *
 * ABSTRACT: 
 *
 *  If the pixmap has changed then update the the picture image by drawing the
 *  designated portion of the pixmap into it.  This should normally be done with
 *  XGetSubImage (but since this is slow.  An XGetImage is performed followed by
 *  Merge_Image).  If the picture_image is non_existant, then create it. 
 *
 *
 */

void Update_Image (x, y, wd, ht, px, py, pix, combo_rule)
    int x, y, wd, ht, px, py;
    Pixmap pix;
    int combo_rule;
{
    XImage *ximage;
    int hl = FALSE;

    if (pix == picture) {
	if (Intersecting_Rectangles (x, y, wd, ht, change_pix_x, change_pix_y,
				     change_pix_wd, change_pix_ht)) {
	    Rectangle_Intersect (x, y, wd, ht, change_pix_x, change_pix_y,
				 change_pix_wd, change_pix_ht, 
				 &x, &y, &wd, &ht);
	}
	else {
	    wd = ht = 0;
	}
    }

    if ((wd != 0) && (ht != 0)) {
        if (select_on && highlight_on) {
            UnHighlight_Select ();
            hl = TRUE;
        }
        MY_XGetSubImage (disp, pix, x, y, wd, ht, bit_plane, img_format,
                         picture_image, px + x, py + y, combo_rule);

        if (hl)
            Highlight_Select ();

        if (pix == picture)
	    Decrease_Change_Pix (x, y, wd, ht);
        else
	    Increase_Change_Pix (IX_TO_PMX (px + x), IY_TO_PMY (py + y),
				 wd, ht);
    }
}

/*
void Update_Image (x, y, wd, ht, px, py, pix, combo_rule)
    int x, y, wd, ht, px, py;
    Pixmap pix;
    int combo_rule;
{
    XImage *ximage;
    int hl = FALSE;

    if (pixmap_changed || (pix != picture)) {
	if (select_on && highlight_on) {
	    UnHighlight_Select ();
	    hl = TRUE;
	}

	MY_XGetSubImage (disp, pix, x, y, wd, ht, bit_plane, img_format,
			 picture_image, px + x, py + y, combo_rule);

	if (hl)
	    Highlight_Select ();
	if (pix == picture)
	    pixmap_changed = FALSE;
	else 
	    pixmap_changed = TRUE;
    }
}
*/

/*
 *
 * ROUTINE:  Restore_Image
 *
 * ABSTRACT:
 *
 *   Update picture_image and the picture pixmap properly, from the designated
 *   area of the given pixmap (pix).  If refresh flag is true then refresh the
 *   picture window from the picture pixmap.
 *
 *   This is accomplished in three steps:
 *     1: Update picture_image if any portion of pix falls to left or right of
 *	  the picture pixmap.
 *     2: Update picture_image if any portion of pix falls above or below the
 *	  picture pixmap.
 *     3: Update the picture pixmap if any portion of pix falls inside the
 *	  picture pixmap.
 */
 /* 
            0     picture_x          x
            |         |              |
           #012345678#90123456789012#34567890123456789#012345#678001234#
          #+-----------------------------------------------------------+
        0-0|                                                           |
          1|                                                           |
          2|   picture_image                                           |
          3|   -------------                                           |
          4|                                                           |
          #|                        +------------------------+         |
        y-5|                        |     pix      |         |         |
          6|                        |     ---    y_off       |         |
          7|                        |              |         |         |
          8|                        |--x_off--22222222 111111|----     |
          9|                        |         22222222 111111|   |     |
          0|                        |         22222222 111111|   |     |
          #|         +--------------+-----------------+111111| height  |
picture_y-1|         |              |         33333333|111111|   |     |
          2|         |              |         33333333|111111|   |     |
          3|         |              |         33333333|111111|----     |
          4|         |              |         |       |     ||         |
          #|         |              +---------|-------+-----|+         |
          5|         |                        |       |     |          |
          6|         |                        |----width----|          |
          7|         | picture pixmap                 |                |
          8|         | ------- ------                 |                |
          9|         |                                |                |
          #|         +--------------------------------+                |
          0|                                                           |
          1|                                                           |
          2|                                                           |
          #+-----------------------------------------------------------+

 */
Restore_Image (pix, x, y, x_off, y_off, width, height, gc, refresh)
    Pixmap pix;
    int x, y, x_off, y_off, width, height;
    int refresh;
    GC gc;
{
    Pixmap tmp;
    XImage ximage;
    XGCValues gcValues;
    int sx_1, sy_1, width_1 = 0, height_1 = 0;
    int sx_2, sy_2, width_2 = 0, height_2 = 0;
    int sx_3, sy_3, width_3 = 0, height_3 = 0;
    int sx_4, sy_4, width_4 = 0, height_4 = 0;
    int sx_5, sy_5, width_5, height_5, dx_5, dy_5;

    XGetGCValues(disp, gc, GCFunction, &gcValues);

    if (y + y_off < picture_y) {
	sx_1 = x_off;
	sy_1 = y_off;
	width_1 = width;
	height_1 = MIN (picture_y, y + y_off + height) - (y + y_off);
        Update_Image (sx_1, sy_1, width_1, height_1, x, y, pix,
                      COMBO_RULE (gcValues.function));
    }

    if ((y + y_off < picture_y + (int) picture_ht) &&
	(y + y_off + height > picture_y)) {

	if (x + x_off < picture_x) {
	    sx_2 = x_off;
	    sy_2 = MAX (y_off, picture_y - y);
	    width_2 = MIN (picture_x, x + x_off + width) - (x + x_off);
	    height_2 = MIN (y + y_off + height, picture_y + (int) picture_ht) - 
		       (y + sy_2);
	    Update_Image (sx_2, sy_2, width_2, height_2, x, y, pix,
			  COMBO_RULE (gcValues.function));
	}

	if (x + x_off + width > picture_x + (int) picture_wd) {
	    sx_3 = MAX (x_off, picture_x + (int) picture_wd - x);
	    sy_3 = MAX (y_off, picture_y - y);
	    width_3 = x_off + width - sx_3;
	    height_3 = MIN (y + y_off + height, picture_y + (int) picture_ht) - 
		       (y + sy_3);
	    Update_Image (sx_3, sy_3, width_3, height_3, x, y, pix,
			  COMBO_RULE (gcValues.function));
	}
    }

    if (y + y_off + height > picture_y + (int) picture_ht) {
	sx_4 = x_off;
	sy_4 = MAX (y_off, picture_y + (int) picture_ht - y);
	width_4 = width;
	height_4 = y_off + height - sy_4;
        Update_Image (sx_4, sy_4, width_4, height_4, x, y, pix,
                      COMBO_RULE (gcValues.function));
    }

    width_5 = width - width_2 - width_3;
    height_5 = height - height_1 - height_4;	
    if ((width_5 > 0) && (height_5 > 0)) {
/* pix did not fall entirely above or below the picture pixmap */
	sx_5 = x_off + width_2;
	sy_5 = y_off + height_1;

	dx_5 =  IX_TO_PMX (x) + sx_5;
	dy_5 =  IY_TO_PMY (y) + sy_5;
	XCopyArea (disp, pix, picture, gc, sx_5, sy_5,
		   width_5, height_5, dx_5, dy_5);
	if (refresh)
	    Refresh_Picture (dx_5, dy_5, width_5, height_5);
    }
}

/*
 *
 * ROUTINE:  Restore_From_Image
 *
 * ABSTRACT:
 *
 *   Update pix from picture_image and the picture pixmap properly, by drawing 
 *   into the designated area of the pix. 
 *
 *   This is accomplished in three steps:
 *     1: Update from picture_image if any portion of pix falls to left or right
 *	  of the picture pixmap.
 *     2: Update from picture_image if any portion of pix falls above or below
 *	  the picture pixmap.
 *     3: Update from the picture pixmap if any portion of pix falls inside the
 *	  picture pixmap.
 */
 /* 
            0     picture_x          x
            |         |              |
           #012345678#90123456789012#34567890123456789#012345#678001234#
          #+-----------------------------------------------------------+
        0-0|                                                           |
          1|                                                           |
          2|   picture_image                                           |
          3|   -------------                                           |
          4|                                                           |
          #|                        +------------------------+         |
        y-5|                        |     pix      |         |         |
          6|                        |     ---    y_off       |         |
          7|                        |              |         |         |
          8|                        |--x_off--22222222 111111|----     |
          9|                        |         22222222 111111|   |     |
          0|                        |         22222222 111111|   |     |
          #|         +--------------+-----------------+111111| height  |
picture_y-1|         |              |         33333333|111111|   |     |
          2|         |              |         33333333|111111|   |     |
          3|         |              |         33333333|111111|----     |
          4|         |              |         |       |     ||         |
          #|         |              +---------|-------+-----|+         |
          5|         |                        |       |     |          |
          6|         |                        |----width----|          |
          7|         | picture pixmap                 |                |
          8|         | ------- ------                 |                |
          9|         |                                |                |
          #|         +--------------------------------+                |
          0|                                                           |
          1|                                                           |
          2|                                                           |
          #+-----------------------------------------------------------+

 */
Restore_From_Image (pix, x, y, x_off, y_off, width, height, gc)
    Pixmap pix;
    int x, y, x_off, y_off, width, height;
    GC gc;
{
    XImage ximage;
    int sx_1, sy_1, width_1 = 0, height_1 = 0;
    int sx_2, sy_2, width_2 = 0, height_2 = 0;
    int sx_3, sy_3, width_3 = 0, height_3 = 0;
    int sx_4, sy_4, width_4 = 0, height_4 = 0;
    int sx_5, sy_5, width_5, height_5, dx_5, dy_5;

    if (y + y_off < picture_y) {
	sx_1 = x_off;
	sy_1 = y_off;
	width_1 = width;
	height_1 = MIN (picture_y, y + y_off + height) - (y + y_off);
	MY_XPutImage (disp, pix, gc, picture_image, x + sx_1, 
		      y + sy_1, sx_1, sy_1, width_1, height_1);
    }

    if ((y + y_off < picture_y + (int) picture_ht) &&
	(y + y_off + height > picture_y)) {

	if (x + x_off < picture_x) {
	    sx_2 = x_off;
	    sy_2 = MAX (y_off, picture_y - y);
	    width_2 = MIN (picture_x, x + x_off + width) - (x + x_off);
	    height_2 = MIN (y + y_off + height, picture_y + (int) picture_ht) - 
		       (y + sy_2);
	    MY_XPutImage (disp, pix, gc, picture_image, x + sx_2, 
			  y + sy_2, sx_2, sy_2, width_2, height_2);
	}

	if (x + x_off + width > picture_x + (int) picture_wd) {
	    sx_3 = MAX (x_off, picture_x + (int) picture_wd - x);
	    sy_3 = MAX (y_off, picture_y - y);
	    width_3 = x_off + width - sx_3;
	    height_3 = MIN (y + y_off + height, picture_y + (int) picture_ht) - 
		       (y + sy_3);
	    MY_XPutImage (disp, pix, gc, picture_image, x + sx_3, 
			  y + sy_3, sx_3, sy_3, width_3, height_3);
	}
    }

    if (y + y_off + height > picture_y + (int) picture_ht) {
	sx_4 = x_off;
	sy_4 = MAX (y_off, picture_y + (int) picture_ht - y);
	width_4 = width;
	height_4 = y_off + height - sy_4;
	MY_XPutImage (disp, pix, gc, picture_image, x + sx_4, 
		      y + sy_4, sx_4, sy_4, width_4, height_4);
    }

    width_5 = width - width_2 - width_3;
    height_5 = height - height_1 - height_4;	
    if ((width_5 > 0) && (height_5 > 0)) {
/* pix did not fall entirely above or below the picture pixmap */
	sx_5 = x_off + width_2;
	sy_5 = y_off + height_1;

	dx_5 =  IX_TO_PMX (x) + sx_5;
	dy_5 =  IY_TO_PMY (y) + sy_5;

/* pix did not fall entirely above or below the picture pixmap */
	XCopyArea (disp, picture, pix, gc, dx_5, dy_5,
		   width_5, height_5, sx_5, sy_5);
    }
}

/*
 * Deal with cases where pixmap must be moved around image in order to do
 * the scroll.
 *
 *
 */
void Boundary_Move_Picture (x, y)
{
    int int_x, int_y, int_wd, int_ht;
    int delta_x, delta_y;
    int intersect;
    int r1_x, r1_y, r1_wd = 0, r1_ht = 0;
    int r2_x, r2_y, r2_wd = 0, r2_ht = 0;

    delta_x = x - picture_x;
    delta_y = y - picture_y;
    intersect = ((abs (delta_x) < (int) picture_wd) &&
		 (abs (delta_y) < (int) picture_ht));
    if (intersect) {
	Rectangle_Intersect (0, 0, picture_wd, picture_ht, delta_x, delta_y,
			     picture_wd, picture_ht, &int_x, &int_y, &int_wd,
			     &int_ht);

	if (delta_y != 0) {
	    r1_x = 0;
	    if (delta_y > 0) {
		r1_y =  0;
		r2_y = delta_y;
	    }
	    else {
		r1_y = picture_ht + delta_y;
		r2_y = 0;
	    }
	    r1_wd = picture_wd;
	    r1_ht = abs(delta_y);

	    if (delta_x != 0) {
		if (delta_x > 0)
		    r2_x = 0;
		else 
		    r2_x = picture_wd + delta_x;
		r2_wd = abs (delta_x);
		r2_ht = picture_ht - abs (delta_y);
	    }
	}
	else {
	    r2_y = 0;
	    if (delta_x > 0)
		r2_x = 0;
	    else
		r2_x = picture_wd + delta_x;
	    r2_wd = abs (delta_x);
	    r2_ht = picture_ht;
	}
    }

    else {
	r1_x = 0;
	r1_y = 0;
	r1_wd = picture_wd;
	r1_ht = picture_ht;
    }

    if (r1_wd != 0) {
	Update_Image (r1_x, r1_y, r1_wd, r1_ht, picture_x, picture_y, picture,
		      ImgK_Src);
    }
    if (r2_wd != 0) {
	Update_Image (r2_x, r2_y, r2_wd, r2_ht, picture_x, picture_y, picture,
		      ImgK_Src);
    }
    XCopyArea (disp, picture, picture, Get_GC(GC_PD_COPY), int_x, int_y, int_wd,
	       int_ht, int_x - delta_x, int_y - delta_y);
    
    if (delta_y >= 0)
	r2_y = y;
    else 
	r2_y = picture_y;
    if (delta_x > 0)
	r2_x = x + picture_wd - delta_x;
    else
	r2_x = x;

    r1_x = x;
    if (delta_y > 0)
	r1_y = y + picture_ht - delta_y;
    else 
	r1_y = y;
    
    if (r1_wd)
	MY_XPutImage (disp, picture, Get_GC(GC_PD_COPY), picture_image,
                      r1_x, r1_y, r1_x - x, r1_y - y,
		      (unsigned int) r1_wd, (unsigned int) r1_ht);
    if (r2_wd)
	MY_XPutImage (disp, picture, Get_GC(GC_PD_COPY), picture_image,
                      r2_x, r2_y, r2_x - x, r2_y - y,
		      (unsigned int) r2_wd, (unsigned int) r2_ht);
}

/*
 *
 * ROUTINE:  Move_Picture
 *
 * ABSTRACT: 
 *
 *   Refresh picture after scrolling
 *
 */              
void Move_Picture(x,y)
    int x, y;
{                  
int window_xscroll, window_yscroll;
XImage *ximage;
int new_picture_x, new_picture_y;

/* Since the zoom area may have been scrolled out of the picture window,
 * recalculate the zoom rectangle.
 */
    xscroll = (picture_x + pic_xorigin) - x;
    yscroll = (picture_y + pic_yorigin) - y;
    if( (xscroll != 0) || (yscroll != 0) ){
	if( select_on )
	    Stop_Highlight_Blink();

/* see if new pixmap must be read from image */
	if ((x < picture_x) || (y < picture_y) || 
	    (x + (int) pwindow_wd > picture_x + (int) picture_wd) ||
	    (y + (int) pwindow_ht > picture_y + (int) picture_ht)) {
/* if pixmap has been changed, save it to the image */
/*
	    Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y,
			  picture, ImgK_Src);
	    Position_Pixmap (x, y);
	    MY_XPutImage( disp, picture, Get_GC(GC_PD_COPY), picture_image,
		       picture_x, picture_y, 0, 0, (unsigned int) picture_wd,
		       (unsigned int) picture_ht );
*/
	    Set_Cursor_Watch (pwindow);

	    Find_Pixmap_Position (x, y, &new_picture_x, &new_picture_y);
	    Boundary_Move_Picture (new_picture_x, new_picture_y);
	    Position_Pixmap (new_picture_x, new_picture_y);
	    window_xscroll =  pic_xorigin - (x - picture_x);
	    window_yscroll =  pic_yorigin - (y - picture_y);

	    Set_Cursor (pwindow, current_action);
	}
	else {
	    window_xscroll = xscroll;
	    window_yscroll = yscroll;
	}

	pic_xorigin -= window_xscroll;
	pic_yorigin -= window_yscroll;
	Copy_Bitmap( picture, pwindow, pic_xorigin, pic_yorigin, pwindow_wd, 
		     pwindow_ht, 0, 0 );

	if ((num_hipts > 0) || (prv_num_hipts > 0)) {
	    Offset_Select_Rband();     
	}

/* change zoom coordinates before grid refresh. jj-10/12/88 */
	if( zoomed ){
	    zoom_xorigin -= window_xscroll;
	    zoom_yorigin -= window_yscroll;
	}

	if( grid_on )
	    Display_Grid( pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht );
	if( select_on )
	    Start_Highlight_Blink ();
/*
	    Refresh_Select_Highlight( pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht );
*/
	if( zoomed ){
	    Initialize_Magnifier();
	    Refresh_Zoom_View( zoom_xorigin, zoom_yorigin, zoom_width, zoom_height);
	}

/* reset stipple origin */
	Reset_SD_Stipple_Origin (-x, -y);
/*	Reset_PD_Clip_Rect (); */

/* Reset scroll variables */
	xscroll = 0;
	yscroll = 0;  
    }

    Update_Pos_Window_Window_Pos (FALSE);
}	

/*
 *
 * ROUTINE:  Scroll_Picture
 *
 * ABSTRACT: 
 *
 * Move the picture window to the given x,y coordinate in the image
 *   
 */
void Scroll_Picture( x, y )
int x, y;          
{
int xoff, yoff;

	xoff = picture_x + pic_xorigin - x;
	yoff = picture_y + pic_yorigin - y;

	if( (xoff != 0) || (yoff != 0) ){
		pic_xorigin = x;
		pic_yorigin = y;

/*
		if( select_on ){
		 	Stop_Highlight_Blink(); 
			}

		if( entering_text ){
			Set_Attribute( text_widget, XmNx, (Position)(IX_TO_PWX(text_x)+ xoff) );
			Set_Attribute( text_widget, XmNy,
				       (Position)(IY_TO_PWY(text_y)+ yoff) );
		  }
*/

		xscroll += xoff;
		yscroll += yoff;
		}
}	

void Clip_To_Line (pts, numpts, dir, val)
    XPoint pts[];
    int *numpts, dir, val;
{
#define PIX_ROUNDOFF 0.5
#define INTERSECT_X(p1,p2,y0) (int)((float)((p1).x)-\
				    (float)(((p1).x-(p2).x)*((p1).y - (y0)))/\
				    (float)((p1).y-(p2).y)+PIX_ROUNDOFF)
#define INTERSECT_Y(p1,p2,x0) (int)((float)((p1).y)-\
				    (float)(((p1).y-(p2).y)*((p1).x - (x0)))/\
				    (float)((p1).x-(p2).x)+PIX_ROUNDOFF)

    int in = TRUE;
    int i, j = 1;

    for (i = 1; i < *numpts; ++i) {
	if (in) {
	    switch (dir) {
		case LEFT:
		    if (pts[i].x < val) {
			in = FALSE;
			pts [j].y = INTERSECT_Y (pts[i], pts[j-1], val);
			pts [j].x = val;
		    }
		    break;
		case RIGHT:
		    if (pts[i].x > val) {
			in = FALSE;
			pts [j].y = INTERSECT_Y (pts[i], pts[j-1], val);
			pts [j].x = val;
		    }
		    break;
		case TOP:
		    if (pts[i].y < val) {
			in = FALSE;
			pts [j].x = INTERSECT_X (pts[i], pts[j-1], val);
			pts [j].y = val;
		    }
		    break;
		case BOTTOM:
		    if (pts[i].y > val) {
			in = FALSE;
			pts [j].x = INTERSECT_X (pts[i], pts[j-1], val);
			pts [j].y = val;
		    }
		    break;
	    }
	    if (in)
		if ( i != j) {
		    pts[j].x = pts[i].x;
		    pts[j].y = pts[i].y;
		}
	    ++j;
	}
	else {
	    switch (dir) {
		case LEFT:
		    if (pts[i+1].x >= val) {
			in = TRUE;
			pts [j].y = INTERSECT_Y (pts[i+1], pts[i], val);
			pts [j].x = val;
		    }
		    break;
		case RIGHT:
		    if (pts[i+1].x <= val) {
			in = TRUE;
			pts [j].y = INTERSECT_Y (pts[i+1], pts[i], val);
			pts [j].x = val;
		    }
		    break;
		case TOP:
		    if (pts[i+1].y >= val) {
			in = TRUE;
			pts [j].x = INTERSECT_X (pts[i+1], pts[i], val);
			pts [j].y = val;
		    }
		    break;
		case BOTTOM:
		    if (pts[i+1].y <= val) {
			in = TRUE;
			pts [j].x = INTERSECT_X (pts[i+1], pts[i], val);
			pts [j].y = val;
		    }
		    break;
	    }
	    if (in)
		++j;
	}
    }
    *numpts = j;
}

void Clip_To_Rect (x, y, wd, ht, pts, numpts)
    XPoint pts[];
    int *numpts;
{
    Clip_To_Line (pts, numpts, TOP, y);
    Clip_To_Line (pts, numpts, RIGHT, x + wd);
    Clip_To_Line (pts, numpts, BOTTOM, y + ht);
    Clip_To_Line (pts, numpts, LEFT, x);
}    

/*
 *
 * ROUTINE:  Create_Pixmap
 *
 * ABSTRACT: 
 *
 * Create and initialize a pixmap with the screen_depth
 *
 */
Pixmap Create_Pixmap (value, width, height)
    long value;
    int width, height;
{
    Pixmap tmpmap;
    GC gc_solid;

    tmpmap = XCreatePixmap (disp, DefaultRootWindow(disp), width, height,
			    screen_depth);
    if (tmpmap) {
	gc_solid = Get_GC (GC_MD_SOLID);
	XSetForeground (disp, gc_solid, value);
	XFillRectangle (disp, tmpmap, gc_solid, 0, 0, width, height);
    }
    return(tmpmap);
}

/*
 *
 * ROUTINE:  Create_Bitmap_Menu
 *
 * ABSTRACT: 
 *
 * Creates a pixmap that contains a menu.  The menu is formed by
 * the input pixmaps, size and spacing information.  The resulting
 * pixmap is returned.
 *
 */
Pixmap Create_Bitmap_Menu( parray, rows, cols, sz, row_space, col_space )
Pixmap parray[];
int rows, cols;
int sz;
int row_space, col_space;
{
Pixmap menu;
int wd, ht;
int i, j;

	wd = cols*sz + (cols+1)*col_space;
	ht = rows*sz + (rows+1)*row_space;
	menu = Create_Bitmap( 0, wd, ht );
	if (menu) {
	    for( i = 0; i < cols; ++i )
		for( j = 0; j < rows; ++j )
		    XCopyArea (disp, parray[i*rows+j], menu, 
			       Get_GC(GC_D1_COPY), 0, 0, sz, sz, 
			       col_space+(i*(sz+col_space)), 
			       row_space+(j*(sz+row_space)));
	}
	return( menu );	
}

/*
 *
 * ROUTINE:  Change_Menu_Entry
 *
 * ABSTRACT: 
 *
 * Change an entry in the given pixmap menu.  
 *
 */
void Change_Menu_Entry( w, menu, entry, row, col, sz, col_space, row_space )
    Window w;
    Pixmap menu;
    Pixmap entry;
    int row, col;
    int sz;
    int col_space, row_space;
{
    int x, y;

    x = col_space + (col * (sz + col_space));
    y = row_space + (row * (sz + row_space));
    XCopyArea (disp, entry, menu, Get_GC(GC_D1_COPY), 0, 0, sz, sz, x, y);

    XCopyPlane (disp, menu, w, Get_GC(GC_MD_COPY), x, y, sz, sz, x, y, 1);
}

/*
 *
 * ROUTINE:  Change_Action
 *
 * ABSTRACT: 
 *
 *  Change the current action
 *
 */
void Change_Action( action )
int action;
{   
    if (entering_text) {
/*	Exit_Select_Portion (); */
	End_Text();
    }
    if (num_hipts != 0) /* is a piece selected? */
    	DeSelect(TRUE);
    if (sp_select_on)
	DeSelect_Portion_Rect ();
    prv_action = current_action;
    current_action = action;
}                       

/*
 *
 * ROUTINE:  Remove_Points
 *
 * ABSTRACT: 
 *                                                 
 * Remove adjacent identical points from a sequence of points
 * If length of sequence drops beolow 2 make it 2 (for purpose of drawing lines
 * sequence must have length >= 2)
 * 
 */
void Remove_Points (pts, num)
    XPoint  *pts;
    int	    *num;
{
    int i=1,j=1;

    if (*num > 2) {   /* if sequence has less than 3 points don't bother */
	while (j < *num) {
	    if ((pts[i-1].x != pts[j].x) || (pts[i-1].y != pts[j].y)) {
		if (i != j) {
		    pts[i].x = pts[j].x;
		    pts[i].y = pts[j].y;
		}
		i++;
	    }
	    j++;
	}
	*num = MAX (i, 2);
    }
}

/*
 *
 * ROUTINE:  Save_Point
 *
 * ABSTRACT: 
 *                                                 
 * Save a points to define the shape and keep track of the min and max
 *
 */
void Save_Point (x, y)
int x, y;
{

    if (numpts >= num_alloc) {
	points = (XPoint *) XtRealloc
	    ((char *)points, sizeof(XPoint)*(numpts+ALLOC_NUM));
	num_alloc = numpts + ALLOC_NUM;
    }

    if (ifocus == zoom_widget) {
	x = ZX_TO_IX (x);
	y = ZY_TO_IY (y);
    }

    if (ifocus == picture_widget) {
	x = PWX_TO_IX (x);
	y = PWY_TO_IY (y);
    }

    points[numpts].x = x;
    points[numpts].y = y;
    ++numpts;

/* Save the extents for later */
    if (x > shape_xmax) shape_xmax = x;
    if (x < shape_xmin) shape_xmin = x;
    if (y > shape_ymax) shape_ymax = y;
    if (y < shape_ymin) shape_ymin = y;
    shape_wd = shape_xmax - shape_xmin;
    shape_ht = shape_ymax - shape_ymin;
/*
    printf( "shape_xmax= %d, shape_xmin= %d\n", shape_xmax, shape_xmin );
    printf( "shape_ymax= %d, shape_ymin= %d\n", shape_ymax, shape_ymin );
*/
}

/*
 *
 * ROUTINE:  Extract_Filename
 *
 * ABSTRACT: 
 *
 *   Extract the filename from the current file spec
 */
void Extract_Filename()
{

#ifndef VMS
/* On an Ultrix system, the pathname will look like this:
 *
 *   directory_path/filename
 * parse the current filespec from the end until the beginning
 * of the string is found or a slash.
 */
    int len;
    int i;
    int found;

    i = strlen(cur_file);
    if (i == 0)
	return;
    found = FALSE;
    while( (i > 0) && !found )
    {
	if( cfile[i] == '/' )
	    found = TRUE;
	else
	    --i;
    }
    if (found) ++i;
    strcpy( &cname[0], &cfile[i] );

#else
/* On a VMS system, the filespec is more complicated, so let the
 * system do it via sys$filescan.
 */
#include <fscndef.h>

    vms_descriptor_type srcstr;
    item_desc item[3];
    int st;
    int flags;
    int i;

    srcstr.string_length = strlen(cur_file);
    if (srcstr.string_length == 0) return;

    srcstr.string_address = (unsigned char *) cur_file;
    srcstr.string_class = 1;  /*  DSC_K_CLASS_S  */
    srcstr.string_type = 14;  /*  DSC_K_TYPE_T   */

    item[0].len = 0;
    item[0].code = FSCN$_NAME;
    item[0].str = 0UL;

    item[1].len = 0;
    item[1].code = FSCN$_TYPE;
    item[1].str = 0UL;

    item[2].len = 0;
    item[2].code = 0;
    item[2].str = 0UL;

    st = sys$filescan (&srcstr, item, &flags);

    i = 0;
    if( item[0].len != 0 )
    {
	strncpy (&cname[i], (char *)item[0].str, item[0].len);
	i += item[0].len;
    }
    if( item[1].len != 0 )
    {
	strncpy (&cname[i], (char *)item[1].str, item[1].len );
	i += item[1].len;
    }
    cname[ i ] = '\0';

#endif
}

/*
 *
 * ROUTINE:  Set_Default_Filetype
 *
 * ABSTRACT:
 *
 *   Set the default filetype if one doesn't exist.
 *   The contents of cfile[] are modified if necessary.
 *   dl - 10/19/88
 */
void Set_Default_Filetype( defstr )
char *defstr;
{
#ifndef VMS
    char *period;
    if (defstr) {
/* search for the last '.' in cur_file  */
	period = strrchr (cur_file, (int)'.');
/* if there is one, then cur_file already has a file delimiter */
/* otherwise attatch the file delimiter in defstr) */
	if (period == NULL) {
	    strcat (cur_file, defstr);
	}
    }
#else
#include <rms.h>
    /* On a VMS system, the filespec is more complicated, so let the
     * system do it via sys$filescan.
     */
    struct NAM nam;
    struct FAB fab;
    unsigned long status;
    char name_buffer[NAM$C_MAXRSS+1];

    nam = cc$rms_nam;

    nam.nam$b_nop = NAM$M_SYNCHK;
    nam.nam$b_ess = NAM$C_MAXRSS;
    nam.nam$l_esa = name_buffer;

    fab = cc$rms_fab;

    fab.fab$l_fna = cur_file;
    fab.fab$b_fns = strlen(cur_file);
    if (defstr)
    {
        fab.fab$l_dna = defstr;
        fab.fab$b_dns = strlen(defstr);
    }

    fab.fab$l_nam = &nam;

    status = sys$parse(&fab);

    if (!(status & 1))
    {
	return;
    }
    else
    {
	strncpy (cfile, name_buffer, nam.nam$b_esl);
	cfile[nam.nam$b_esl] = 0;
    }
#endif
}

/*
 *
 * ROUTINE:  Bresenham_Line
 *
 * ABSTRACT: 
 *
 * Bresenhams Algorithm - taken from Procedural Elements for Computer Graphics
 * by David F. Rogers.
 *   
 */
void Bresenham_Line( x1, y1, x2, y2 )
int x1, y1, x2, y2;
{
	int i, a, b;
	int delta_x, delta_y;
	int error, temp, interchange;
	int s1, s2;
	int npts;
	XPoint *pts;                

		pts = (XPoint *) XtMalloc(1); /* jj-port */
		npts = 0;
		a = x1;
	 	b = y1;
		delta_x = abs( x2 - x1 );
		delta_y = abs( y2 - y1 );
		s1 = sign( x2 - x1 );
		s2 = sign( y2 - y1 );
		if( delta_y > delta_x ){
			temp = delta_x;
			delta_x = delta_y;
	 		delta_y = temp;
			interchange = TRUE;
		}
		else
			interchange = FALSE;
		error = 2*delta_y - delta_x;
		for( i = 1; i <= delta_x; ++i ){        
			pts = (XPoint *) realloc(pts, sizeof(XPoint)*(npts+1)); /* jj-port */
			pts[npts].x = a;
			pts[npts].y = b;
			++npts;
			while( error >= 0 ){
				if (interchange) 
					a = a + s1;
				else
					b = b + s2;                         
				error = error - 2*delta_x;
				}
			if (interchange)
				b = b + s2;
			else
				a = a + s1;
		 	error = error + 2*delta_y;
			}
		Draw_With_Brush( pts, npts );
		XtFree ((char *)pts);
}

/*
 * given integers x and n, return x raised to the poer of n.
 *
 */
int x_to_n (x, n)
    int x, n;
{
    int i, p;

    p = 1;
    for (i = 1; i <= n; ++i)
	p *= x;
    return (p);
}

void Invert_Image_Data (data, pixels, lines, bpl)
    unsigned char *data;
{
    int i, j, index;
    int full_bytes, extra_bits;
    unsigned char mask = 0;

    full_bytes = pixels / 8;
    extra_bits = pixels % 8;
    for (i = 0; i < extra_bits; i++) {
	mask <<= 1;
	mask += 1;
    }
    
    for (i = 0; i < lines; i++) {
	for (j = 0; j < full_bytes; j++) {
	    index = i * bpl + j;
	    data[index] = ~data[index];
	}
	if (extra_bits) {
	    index = i * bpl + full_bytes;
	    data[index] = data[index] ^ mask;
	}
    }
}

/* Expand image data from 1 bit per pixel to 8 bits per pixel */
/* If fg is true, a set bit signifies a pixel set to the foreground color */
/* Otherwise, a set bit signigies a pixel set to the background color */
unsigned char *Expand_Image_Data (image_data, wd, ht, bpl_in, bpl_out, fg)
    unsigned char *image_data;
    int wd, ht;
    int bpl_in, *bpl_out;
    int fg;
{
    unsigned char fg_byte, bg_byte;
    unsigned char bitmask;
    unsigned char *new_image_data;
    int i, j;

    *bpl_out = wd;
    while (((*bpl_out) % 4) != 0)
	(*bpl_out)++;
    new_image_data = (unsigned char *) XtMalloc(ht * (*bpl_out));

    if (new_image_data == 0)
        return (0);

    if (expand_image_use_fg_bg) {
/* use the paint foreground and background colors */
	fg_byte = colormap[paint_color].pixel;
	bg_byte = colormap[paint_bg_color].pixel;
    }
    else {
	fg_byte = colormap[BLACK].pixel;
	bg_byte = colormap[WHITE].pixel;
    }
    for (i = 0; i < ht; i++) {
	    bitmask = 0x01;
/*	    bitmask = 0x80; */
	for (j = 0; j < wd; j++) {
	    if (image_data[i * bpl_in + j / 8] & bitmask) {
		if (fg)
		    new_image_data[i * (*bpl_out) + j] = fg_byte;
		else 
		    new_image_data[i * (*bpl_out) + j] = bg_byte;
	    }
	    else {
		if (fg)
		    new_image_data[i * (*bpl_out) + j] = bg_byte;
		else 
		    new_image_data[i * (*bpl_out) + j] = fg_byte;
	    }
	    if ((bitmask <<= 1) == 0)
		bitmask = 0x01;
/*
	    if ((bitmask >>= 1) == 0)
		bitmask = 0x80;
*/
	}
    }

    return (new_image_data);
}

/* Fetch a widget and set its colormap */
/* if the server has multiple visuals, don't set the colormap on the widget */
int Fetch_Widget (name, parent, widget)
    char *name;
    Widget parent, *widget;
{
    int retval;

    if ((retval = MrmFetchWidget (s_DRMHierarchy, name, parent, widget,
				  &dummy_class)) == MrmSUCCESS) {
	if (visual_info->visual == XDefaultVisual (disp, screen)) {
	    XSetWindowColormap (disp, XtWindow (XtParent (*widget)),
				paint_colormap);	
	}
    }
    return (retval);
}

char *Expand_Image_Data_Cmap_To_RGB (image_data, wd, ht, bpl_in, bpl_out,
				     order, num_colors_used)
    char *image_data;
    int wd, ht;
    int bpl_in, *bpl_out;
    int *order;
    int *num_colors_used;
{
    int i, j, out_ind;
    unsigned char ind;
    char *new_image_data;
    unsigned char remap_r[MAX_COLORS], remap_g[MAX_COLORS]; 
    unsigned char remap_b[MAX_COLORS];
    int colors_used[MAX_COLORS], back_ptr[MAX_COLORS];
    int index, tmp;
    int done, black_used, white_used;

/* set up the remapping for colormap values */
    for (i = 0; i < num_colors; i++) {
	remap_r[colormap[i].pixel] = colormap[i].i_red >> 8;
	remap_g[colormap[i].pixel] = colormap[i].i_green >> 8;
	remap_b[colormap[i].pixel] = colormap[i].i_blue >> 8;
	back_ptr[colormap[i].pixel] = i;
	colors_used[i] = 0;
    }

    *bpl_out = wd * 3;
    while (((*bpl_out) % 4) != 0)
	(*bpl_out)++;
    new_image_data = (char *) XtMalloc(ht * (*bpl_out));

    if (new_image_data == 0) {
	return (0);
    }

    for (i = 0; i < ht; i++) {
	for (j = 0; j < wd; j++) {
	    ind = image_data[i * bpl_in + j];
	    out_ind = (i * (*bpl_out)) + (3 * j);
	    new_image_data[out_ind] = remap_r[ind];
	    new_image_data[out_ind + 1] = remap_g[ind];
	    new_image_data[out_ind + 2] = remap_b[ind];
	    (colors_used[back_ptr[ind]])++;
	}
    }
    
    black_used = white_used = FALSE;
    *num_colors_used = 0;
/* order the colors from most used to least used */
    done = FALSE;
    while (!done) {
	tmp = 0;
	for (i = 0; i < num_colors; i++) {
	    if (colors_used[i] > tmp) {
		tmp = colors_used[i];
		index = i;
	    }
	}
	if (tmp) {
	    order[*num_colors_used] = index;
	    colors_used[index] = 0;
	    (*num_colors_used)++;
	    if (index == BLACK) {
		black_used = TRUE;
	    }
	    if (index == WHITE) {
		white_used = TRUE;
	    }
	}
	else {
	    done = TRUE;
	}
    }

/* always add black and white */
    if (!black_used) {
	order[*num_colors_used] = BLACK;
	(*num_colors_used)++;
    }
    if (!white_used) {
	order[*num_colors_used] = WHITE;
	(*num_colors_used)++;
    }    

    return (new_image_data);
}

unsigned char *Convert_Image_Data_RGB_To_Cmap (image_data, wd, ht, bpl_in,
					       bpl_out, ddif_colors,
					       num_colors_used, remap, status)
    unsigned char *image_data;
    int wd, ht;
    int bpl_in, *bpl_out;
    DDIF_COLOR_TYPE *ddif_colors;
    int num_colors_used;
    int *remap;
    int *status;
{
    int i, j, k, ind;
    unsigned char *new_image_data;
    unsigned char remap_r[MAX_COLORS], remap_g[MAX_COLORS], remap_b[MAX_COLORS];

    *status = K_SUCCESS;
/*******************************************************************************
 * a very sleazy hack to get around what I think is an ISL bug in
 * Img_IMPORTBitmap - with 24 bit/pixel images, it is not padding to the
 * proper byte boundary
 *
 * Apparently, it has been fixed (10/8/90)
 *
 *   if ((wd % 4) != 3) {
 *	bpl_in = wd * 3;
 *   }
 *
*******************************************************************************/

    *bpl_out = wd;
    while (((*bpl_out) % 4) != 0)
	(*bpl_out)++;

    new_image_data = (unsigned char *) XtMalloc(ht * (*bpl_out));
    if (new_image_data == NULL)
    {
	*status = K_NO_CLIENT_MEMORY;
	return (0);
    }

    for (i = 0; i < num_colors_used; i++) {
	remap_r[i] = ddif_colors[i].red >> 8;
	remap_g[i] = ddif_colors[i].green >> 8;
	remap_b[i] = ddif_colors[i].blue >> 8;
    }

    for (i = 0; i < ht; i++) {
	for (j = 0; j < wd; j++) {
	    ind = i * bpl_in + (3 * j);
	    for (k = 0; (((image_data[ind] != remap_r[k]) ||
			  (image_data[ind + 1] != remap_g[k]) ||
			  (image_data[ind + 2] != remap_b[k])) &&
			 (k < num_colors_used)); k++);
	    if (k >= num_colors_used)
	    {
		XtFree ((char *)new_image_data);
		*status = K_NO_CONVERT_IMAGE_DATA;
		return (0);
	    }
	    new_image_data[i * (*bpl_out) + j] = colormap[remap[k]].pixel;
	}
    }
    return (new_image_data);
}

static unsigned short int ones[2] =
    {0, 255};
static unsigned short int twos[4] =
    {0, 85, 170, 255};
static unsigned short int threes[8] = 
    {0, 36, 73, 109, 146, 182, 219, 255};
static unsigned short int fours[16] =
    {0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};
static unsigned short int fives[32] =
    {0, 8, 16, 24, 33, 41, 49, 57, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140,
     148, 156, 165, 173, 181, 189, 198, 206, 214, 222, 231, 239, 247, 255};
static unsigned short int sixes[64] =
    {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 65, 69, 73,
     77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138,
     142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 195, 199,
     203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255};
static unsigned short int sevens[128] =
    {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
     40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76,
     78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
     112, 114, 116, 118, 120, 122, 124, 126, 129, 131, 133, 135, 137, 139, 141,
     143, 145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171,
     173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201,
     203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231,
     233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255};

/* turn a pixel value into an XColor (DDIF Color) value, based on the */
/* type of server */
/* 4 bit per pixel RGB = ____BGGR */
/* 8 bit per pixel RGB = BBGGGRRR */
void Convert_Pixel_To_DDIF_Color (pixel_in, ddif_color, sm, bpp, brt)
    int pixel_in;
    DDIF_COLOR_TYPE *ddif_color;
    int sm, bpp;  /* spectral mapping, bits per pixel */
    int brt; /* brightness polarity */
{
    unsigned char pixel;
    unsigned char mask = 0;
    int i;
    
    pixel = pixel_in;

/* if zero max intensity, flip the bits before creating the color */
    if (brt == ImgK_ZeroMaxIntensity) {
	for (i = 0; i < bpp; i++) {
	    mask <<= 1;
	    mask += 1;
	}
	pixel = pixel ^ mask;
    }

    if (sm == ImgK_MonochromeMap) {
	switch (bpp) {
	    case 2 :
		ddif_color->red = twos[pixel] << 8;
		break;
	    case 3 :
		ddif_color->red = threes[pixel] << 8;
		break;
	    case 4 :
		ddif_color->red = fours[pixel] << 8;
		break;
	    case 5 :
		ddif_color->red = fives[pixel] << 8;
		break;
	    case 6 :
		ddif_color->red = sixes[pixel] << 8;
		break;
	    case 7 :
		ddif_color->red = sevens[pixel] << 8;
		break;
	    case 8 :
		ddif_color->red = pixel << 8;
		break;
	}

	ddif_color->green = ddif_color->red;
	ddif_color->blue = ddif_color->red;
    }

    else { /* (sm == ImgK_RgbMap) */
	if (bpp== 4) {
	    ddif_color->red = ones[(pixel & 0x01)] << 8;
	    ddif_color->green = twos[(pixel & 0x06) >> 1] << 8;
	    ddif_color->blue = ones [(pixel & 0x08) >> 3] << 8;
	}
	else { /* (bpp == 8) */
	    ddif_color->red = threes[(pixel & 0x07)] << 8;
	    ddif_color->green = threes[(pixel & 0x38) >> 3] << 8;
	    ddif_color->blue = twos[(pixel & 0xc0) >> 6] << 8;
	}
    }
}

/* convert image data from Colormapped image to a black and white image */
unsigned char *Convert_Image_Data_Cmap_To_BW (image_data, wd, ht, bpl_in,
					      bpl_out, pad, status)
    unsigned char *image_data;
    int wd, ht;
    int bpl_in, *bpl_out;
    int pad;  /* pad each scanline to this many bytes in output */
    int *status;
{
    int i, j, ind_in, ind_out;
    unsigned char bit;
    unsigned char *new_image_data;

    *status = K_SUCCESS;

    *bpl_out = wd / 8 + ((wd % 8) ? 1 : 0);
    while (((*bpl_out) % pad) != 0)
	(*bpl_out)++;

    new_image_data = (unsigned char *) XtCalloc(ht * (*bpl_out), sizeof(unsigned char));

    if (new_image_data == NULL) {
	*status = K_FAILURE;
	return (0);
    }

    for (i = 0; i < ht; i++) {
	ind_out = i * (*bpl_out);
	bit = 1;
	for (j = 0; j < wd; j++) {
	    ind_in = i * bpl_in + j;
	    if (image_data[ind_in] != colormap[WHITE].pixel) {
		if (image_data[ind_in] == colormap[BLACK].pixel) {
		    new_image_data[ind_out] += bit;
		}
		else {
		    XtFree ((char *)new_image_data);
		    *status = K_PICTURE_NOT_BW;
		    return (0);
		}
	    }
	    if (bit == 0x80) {
		bit = 1;
		ind_out++;
	    }
	    else {
		bit <<= 1;
	    }
	}
    }
    return (new_image_data);
}

void Remap_4_To_8 (in_data, out_data, bpl_in, bpl_out, pixels, lines)
    unsigned char *in_data, *out_data;
    int bpl_in, bpl_out, pixels, lines;
{
    int i, j;
    unsigned char *out, *in;

    out = out_data;
    in = in_data;

    for (j = 0; j < lines; j++) {
	out = out_data + (j * bpl_out);
	in = in_data + (j * bpl_in);
	for (i = 0; i < pixels; i += 2) {
	    *out = (*in & 0x0f);
	    if (i + 1 < pixels) {
		out++;
		*out = (*in & 0xf0) >> 4;
	    }
	    in++;
	    out++;
	}
    }
}

void Remap_2_To_8 (in_data, out_data, bpl_in, bpl_out, pixels, lines)
    unsigned char *in_data, *out_data;
    int bpl_in, bpl_out, pixels, lines;
{
    int i, j;
    unsigned char *out, *in;

    out = out_data;
    in = in_data;

    for (j = 0; j < lines; j++) {
	out = out_data + (j * bpl_out);
	in = in_data + (j * bpl_in);
	for (i = 0; i < pixels; i += 4) {
	    *out = (*in & 0x03);
	    if (i + 1 < pixels) {
		out++;
		*out = (*in & 0x0c) >> 2;
	    }
	    if (i + 2 < pixels) {
		out++;
		*out = (*in & 0x30) >> 4;
	    }
	    if (i + 3 < pixels) {
		out++;
		*out = (*in & 0xc0) >> 6;
	    }
	    in++;
	    out++;
	}
    }
}

void Change_Grid_Size_Val (w, wclose, info)
    int w;
    int wclose;
    XmScaleCallbackStruct *info;
{
    new_grid_size = info->value;
}

void Grid_Size_Button (w, button_id, reason)
    Widget   w;
    int	     *button_id;
    int	     reason;
{
    int reset = FALSE;

    if (!Finished_Action())
	return;

    switch (*button_id) {
	case GSD_OK_ID :
	    XtUnmanageChild (grid_size_dialog);
	case GSD_APPLY_ID :
	    if (grid_on) {
		Set_Grid ();
		reset = TRUE;
	    }
	    grid_size = new_grid_size;
	    if (reset) {
		Set_Grid ();
	    }
	    break;
	case GSD_CANCEL_ID :
	    XtUnmanageChild (grid_size_dialog);
	    if (grid_size != new_grid_size)
		XmScaleSetValue (widget_ids[GSD_SCALE], grid_size);
	    break;
    }
}

void Create_Grid_Size_Dialog ()
{
    Set_Cursor_Watch (pwindow);
    if (!grid_size_dialog) {
	if (Fetch_Widget ("grid_size_dialog_box", main_widget,
			   &grid_size_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch grid size dialog");
    }
    if (grid_size_dialog) {
	if (!XtIsManaged (grid_size_dialog)) {
	    new_grid_size = grid_size;
	    XtManageChild (grid_size_dialog);
	}
	Pop_Widget (grid_size_dialog);
    }
    Set_Cursor (pwindow, current_action);
}

void Clear_Work_Window ()
{
    if (entering_text)
	End_Text ();
    if (select_on)
	DeSelect (TRUE);
    Set_Cursor_Watch (pwindow);
    if (undo_available) {
	Clear_UG ();
	undo_x = pic_xorigin;
	undo_y = pic_yorigin;
	undo_width = pwindow_wd;
	undo_height = pwindow_ht;
	undo_picture_x = picture_x;
	undo_picture_y = picture_y;
	UG_image[UG_last] = XGetImage (disp, picture, pic_xorigin, pic_yorigin,
				       pwindow_wd, pwindow_ht, bit_plane,
				       img_format);
	UG_used[0] = UG_last;
	UG_num_used = 1;
    }
    undo_action = CLEAR_WW;
    Set_Undo_Button (undo_action);
    XSetForeground (disp, Get_GC (GC_PD_SOLID), picture_bg);
    XFillRectangle (disp, picture, Get_GC (GC_PD_SOLID), pic_xorigin,
		    pic_yorigin, pwindow_wd, pwindow_ht);
    Increase_Change_Pix (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
    Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
    Set_Cursor (pwindow, current_action);
}
