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
static char *BuildSystemHeader= "$Id: select.c,v 1.1.2.3 92/12/11 08:36:01 devrcs Exp $";
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
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This module handles selection and manipulating selected areas in
**   the picture
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**  11/1/88 dl
**  Added CUT/PASTE case in DeSelect
**
**  11/1/88 jj
**  Added condition in CLEAR case in DeSelect
**
**--
*/           
#include "paintrefs.h"

extern Force_Return_On_Failure();

static XtIntervalId timer_id;

/*
 *
 * ROUTINE:  Within_Select_Area
 *
 * ABSTRACT: 
 *
 * Returns a value true or false if the given point lies within the
 * current select area
 *
 */
int Within_Select_Area( w, x, y)
Widget w;
int x,y;
{
int x1, y1;

/*
	if( XtWindow(w) != select_window )
		return( FALSE );
*/

	if( w == zoom_widget ){
/* find current position in unzoomed coordinates */
		x1 = ZX_TO_IX( x );
		y1 = ZY_TO_IY( y );
		}
	else{
		x1 = PWX_TO_IX( x );
		y1 = PWY_TO_IY( y );
		}
	return( XPointInRegion( select_region, x1, y1 ) );
}

void Draw_Selection()
{
    XPoint  *pts;
    int	    i;

    XDrawLines (disp, select_window, gc_highlight, hi_points, num_hipts,
		CoordModeOrigin);
    pts = (XPoint *) XtMalloc( sizeof(XPoint)*num_hipts );
    for (i = 0; i < num_hipts; i++) {
	pts[i].x = PWX_TO_IX (hi_points[i].x);
	pts[i].y = PWY_TO_IY (hi_points[i].y);
    }
    XDrawLines (disp, picture, Get_GC(GC_PD_INVERT), pts, num_hipts, 
		CoordModeOrigin);
    if (zoomed) {
	Refresh_Zoom_View(select_x, select_y, select_width+1, select_height+1);
	Refresh_Magnifier(select_x, select_y, select_width+1, select_height+1);
    }
    XtFree ((char *)pts);
}

/*
 *                   
 * ROUTINE:  Highlight_Select
 *
 * ABSTRACT: 
 *
 * Highlight the selected area
 *
 */
void Highlight_Select()
{

    if( !highlight_on ){
	Draw_Selection ();
	highlight_on = TRUE;
    }
    else	
	highlight_on = FALSE;
}

/*
 *
 * ROUTINE:  UnHighlight_Select
 *
 * ABSTRACT: 
 *
 * UnHighlight the selected area
 *
 */
void UnHighlight_Select()  
{
	if( highlight_on )
	    Draw_Selection ();
	highlight_on = FALSE;
}

/*
 *
 * ROUTINE:  Start_Highlight_Blink
 *
 * ABSTRACT: 
 *
 * Adds a timer so that an event can be received to blink
 *
 */
void Start_Highlight_Blink()
{
	Highlight_Select();
/*
	timer_id = XtAddTimeOut( 500, Timer_Alarm(), NULL );
*/
}

/*
 *
 * ROUTINE:  Stop_Highlight_Blink
 *
 * ABSTRACT: 
 *
 * Adds a timer so that an event can be received to blink
 *
 */
void Stop_Highlight_Blink()
{
/*
	XtRemoveTimeOut( timer_id );
*/
	UnHighlight_Select();
}

/*
 *
 * ROUTINE:  Blink_Highlight
 *
 * ABSTRACT: 
 *
 * Blink the highlight on/off when called
 *
 */
void Blink_Highlight()
{
	if( highlight_on )
		UnHighlight_Select();
	else
		Highlight_Select();
/*
	timer_id = XtAddTimeOut( 100, Timer_Alarm(), NULL );
*/
}

/*
 *
 * ROUTINE:  Copy_Select_To_Screen
 *
 * ABSTRACT: 
 *                                
 *   Creates the pixmap of the select area copied to the picture at
 *   the given point.  x,y are pixmap coordinates.
 *
 */
void Copy_Select_To_Screen (x, y)
int x, y;
{
GC gc_mask;

/* Copy the selected area to the new position */
    if (select_rectangle && opaque_fill) {
	Copy_Bitmap (copymap, pwindow, 0, 0, select_width, select_height,
		     IX_TO_PWX(x), IY_TO_PWY(y));
    }
    else {
	gc_mask = Get_GC (GC_PD_MASK);
	XSetClipOrigin (disp, gc_mask, 0, 0);
	XCopyArea (disp, picture, undo_move_map, Get_GC (GC_PD_COPY), x, y,
		   select_width, select_height, 0, 0);
/*
	Restore_From_Image (undo_move_map, PMX_TO_IX (x), PMY_TO_IY (y), 0, 0,
			    select_width, select_height, Get_GC(GC_PD_COPY));
*/
	XCopyArea (disp, copymap, undo_move_map,  Get_GC (GC_PD_MASK), 0, 0,
		   select_width, select_height, 0, 0);
	Copy_Bitmap (undo_move_map, pwindow, 0, 0, select_width,
		     select_height, IX_TO_PWX(x), IY_TO_PWY(y));
    }
    if (grid_on)
        Display_Grid (x, y, select_width, select_height);

}

/*
 *
 * ROUTINE:  Copy_Select_To_Image
 *
 * ABSTRACT: 
 *                                
 *   Takes care of all 4 cases for copying select into the image.
 *   x and y are in image coordinates.
 *
 */
void Copy_Select_To_Image (x, y, refresh)
int x, y;
{
    int x1, y1, wd1, ht1;
    XImage *ximage;

    GC gc_mask;


/* clip select to image */
    x1 = MAX (x, 0);
    y1 = MAX (y, 0);
    wd1 = MIN (select_width - (x1 - x), pimage_wd - x1);	
    ht1 = MIN (select_height - (y1 - y), pimage_ht - y1);


    gc_mask = Get_GC(GC_PD_MASK);

    if ((select_rectangle && opaque_fill) ||
	(Inside_Rectangle (x1, y1, wd1, ht1, picture_x, picture_y, picture_wd,
			   picture_ht))) {
	if (!opaque_fill) {
	    Restore_Image (undo_move_map, x, y, 0, 0, select_width,
			   select_height, Get_GC(GC_PD_COPY), FALSE);
	}
	XSetClipOrigin (disp, gc_mask, IX_TO_PMX(x), IY_TO_PMY(y));
	Restore_Image (copymap, x, y, 0, 0, select_width, select_height, 
		       gc_mask, refresh);
    }

    else {
/* else use undo_move_map */
	ximage = XGetImage (disp, undo_move_map, 0, 0, select_width,
			    select_height, bit_plane, img_format);
	XSetClipOrigin (disp, gc_mask, 0, 0);
	XCopyArea (disp, copymap, undo_move_map, gc_mask, 0, 0, 
		   select_width, select_height, 0, 0);
	Restore_Image (undo_move_map, x, y, 0, 0, select_width, 
		       select_height, Get_GC (GC_PD_COPY), refresh);
	MY_XPutImage (disp, undo_move_map, Get_GC(GC_PD_COPY), ximage,
		      0, 0, 0, 0, select_width, select_height);
	if (ximage != NULL)
	{
#if 0
	    XtFree ((char *)ximage->data);
#endif
	    XDestroyImage (ximage);
	    ximage = NULL;
	}
    }
}
;
/*
 *
 * ROUTINE:  Create_Clip_Mask
 *
 * ABSTRACT: Fill in the clip mask properly - the clip_mask pixmap must
 *	     already be allocated
 * 
 *
 */
void Create_Clip_Mask ()
{
    int i,j;
    GC gc_solid, gc_copy;


/* fill it with zeros */
    gc_solid = Get_GC (GC_D1_SOLID);
    XSetForeground (disp, gc_solid, 0);
    XFillRectangle (disp, clip_mask, gc_solid, 0, 0, select_width,
		    select_height);

/* if transparent, then determine clip mask */
    if (!opaque_fill) {
	gc_copy = Get_GC (GC_PD_FUNCTION);
	XSetFunction (disp, gc_copy, GXxor);
	XSetForeground (disp, gc_copy, picture_bg);
/* turn all background pixels of copymap into zeros */
	if (select_rectangle) {
	    XFillRectangle (disp, copymap, gc_copy, 0, 0, select_width,
			    select_height);
	}
	else {
	    XFillPolygon (disp, copymap, gc_copy, orig_hi_points,
			  orig_num_hipts, Complex, CoordModeOrigin);
	}

/* Set pixels to 1 in clip_mask if pixel in copymap is not now 0 */
	gc_copy = Get_GC (GC_D1_FUNCTION);
	XSetFunction (disp, gc_copy, GXor);
	for (i = 0, j = 1; i < pdepth; i++, j <<= 1) {
	    XCopyPlane (disp, copymap, clip_mask, gc_copy, 0, 0,
			select_width, select_height, 0, 0, j);
	}

	gc_copy = Get_GC (GC_PD_FUNCTION);
/* foreground already set to picture_bg, function already set to GXxor */
/* return copymap to its original state */
	if (select_rectangle) {
	    XFillRectangle (disp, copymap, gc_copy, 0, 0, select_width,
			    select_height);
	}
	else {
	    XFillPolygon (disp, copymap, gc_copy, orig_hi_points, 
			  orig_num_hipts, Complex, CoordModeOrigin);
	}
/* reset foreground to picture_fg */
	XSetForeground (disp, gc_copy, picture_fg);
    }

    else {	
/* Otherwise just set clip mask for select area */
	XSetForeground (disp, gc_solid, 1);
	XFillPolygon (disp, clip_mask, gc_solid, orig_hi_points,
		      orig_num_hipts, Complex, CoordModeOrigin); 
    }     
/* hack so it will always pick up the new clip_mask */
    XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
    XSetClipMask (disp, Get_GC (GC_PD_MASK), clip_mask);
}

/*
 *
 * ROUTINE:  Invert_Copymap
 *
 * ABSTRACT: 
 *
 * Inverts the copymap
 *
 */
void Invert_Copymap ( )
{
    GC gc_copy, gc_mask;
    int image_size;
    unsigned long image_id, new_id;
    Pixmap tmp_mask;
    int set_index;
    struct PUT_ITMLST set_attributes[7];
    XImage *ximage;
    buf_descriptor_type array_descriptor;
    int i;
    unsigned char *image_data;

/* Create new copymap*/
    if (pdepth == 1) {
	gc_copy = Get_GC (GC_PD_INVERT);
	if (select_rectangle) {
	    XFillRectangle (disp, copymap, gc_copy, 0, 0, select_width, 
			    select_height);
	}
	else {		
	    XFillPolygon (disp, copymap, gc_copy, orig_hi_points,
			  orig_num_hipts, Complex, CoordModeOrigin); 
	}
    }
    else {
	ximage = XGetImage (disp, copymap, 0, 0, select_width,
			    select_height, bit_plane, img_format);

	bits_per_pixel = 8;
        pixel_order = ImgK_StandardPixelOrder;

        bytes = ximage->bytes_per_line;
        pixels_per_scanline = ximage-> width;
        scanline_count = ximage -> height;
        scanline_stride = 8 * bytes;    /* 8 bits per byte */
	image_data = (unsigned char *)ximage->data;
        image_size = bytes * scanline_count;

        start_set_itemlist(set_attributes, set_index);
        put_set_item(set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item(set_attributes, set_index, Img_BitsPerPixel, bits_per_pixel);
	put_set_item(set_attributes, set_index, Img_PixelsPerLine, pixels_per_scanline);
        put_set_item(set_attributes, set_index, Img_NumberOfLines, scanline_count);
        put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
        end_set_itemlist(set_attributes, set_index);

	image_id = ImgCreateFrame(set_attributes, image_stype);
	ImgImportBitmap (image_id, (char *)image_data, image_size, 0, 0, 0, 0);
    
	for (i = 0; i < MAX_COLORS; i++) {
	    pixel_remap[i] = i;
	}
	for (i = 1; i < num_colors; i += 2) {
	    pixel_remap[colormap[i - 1].pixel] = colormap[i].pixel;
	    pixel_remap[colormap[i].pixel] = colormap[i - 1].pixel;
	}

	array_descriptor.dsc$w_length = sizeof(unsigned char);
        array_descriptor.dsc$b_dtype = DSC_K_DTYPE_BU;
        array_descriptor.dsc$b_class = DSC_K_CLASS_A;
        array_descriptor.dsc$a_pointer = (char *)pixel_remap;

        array_descriptor.dsc$b_scale = 0;
        array_descriptor.dsc$b_digits = 0;
        array_descriptor.dsc$b_aflags = 0;

        array_descriptor.dsc$b_dimct = 1;
        array_descriptor.dsc$l_arsize = sizeof(pixel_remap);

	ISL_ERROR_HANDLER_SETUP
        new_id = ImgPixelRemap (image_id, (unsigned long)&array_descriptor, 0, 0, 0, 0, 0);
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
        }
	else {
	    if (image_id != new_id)
		ImgDeleteFrame (image_id);
	    image_id = new_id;
	}

	ImgExportBitmap(image_id, 0, (char *)image_data, image_size, 0, 0, 0, 0);
	MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), ximage, 0, 0, 0, 0,
		      select_width, select_height);

	if (!select_rectangle) {
	    gc_mask = Get_GC (GC_PD_MASK);
	    gc_copy = Get_GC (GC_D1_FUNCTION);

	    tmp_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
				      select_width, select_height, 1);

	    XSetFunction (disp, gc_copy, GXset);
	    XFillRectangle (disp, tmp_mask, gc_copy, 0, 0, select_width,
			    select_height);
	    XSetFunction (disp, gc_copy, GXclear);
	    XFillPolygon (disp, tmp_mask, gc_copy, orig_hi_points,
			  orig_num_hipts, Complex, CoordModeOrigin);

	    XSetClipMask (disp, gc_mask, tmp_mask);
	    XSetClipOrigin (disp, gc_mask, 0, 0);
	    XSetForeground (disp, gc_mask, 0);

	    XFillRectangle (disp, copymap, gc_mask, 0, 0,
			    select_width, select_height);

	    XSetForeground (disp, gc_mask, picture_fg);
	    XSetClipMask (disp, gc_mask, clip_mask);

	    XFreePixmap (disp, tmp_mask);
	}
    }
    if (!opaque_fill) {
	Create_Clip_Mask ();
    }
}

/*
 *
 * ROUTINE:  Free_Select_Pixmaps
 *
 * ABSTRACT: 
 *
 *	If can't allocate space for all pixmaps involved with select, deallocate
 *	space which may have been used.  Set undo button to no action.
 */
Free_Select_Pixmaps ()
{
    if (copymap) {
	XFreePixmap (disp, copymap);
	copymap = 0;
    }
    if (undo_move_map) {
	XFreePixmap (disp, undo_move_map);
	undo_move_map = 0;
    }
    if (clip_mask) {
	XFreePixmap (disp, clip_mask);
	clip_mask = 0;
	XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
    }
    Set_Undo_Button (NO_ACTION);
    Set_Cursor (pwindow, current_action);
    Display_Message ("T_NO_SELECT");
}

void Create_Undo_Move_Map ()
{
    int i;
    GC gc_solid;

    undo_move_x = PMX_TO_IX (select_x);
    undo_move_y = PMY_TO_IY (select_y);
    if (undo_move_map) {
	if ((undo_move_wd != select_width) || (undo_move_ht != select_height)) {
	    XFreePixmap( disp, undo_move_map);
	    undo_move_map = 0;
	}    
    }
    if (!undo_move_map) {
	undo_move_map = Create_Pdepth_Pixmap (picture_bg, select_width,
					      select_height);
/* if couldn't create undo_move_map - bail out */
	if (undo_move_map == 0)
	    return;
    }

    if (!select_rectangle) {
	Restore_From_Image (undo_move_map, PMX_TO_IX (select_x), 
			    PMY_TO_IY (select_y), 0, 0, select_width, 
			    select_height, Get_GC (GC_PD_COPY));
	gc_solid = Get_GC (GC_PD_SOLID);    
	XSetForeground (disp, gc_solid, picture_bg);
	XFillPolygon (disp, undo_move_map, gc_solid, orig_hi_points,
		      orig_num_hipts, Complex, CoordModeOrigin); 
    }
    undo_move_wd = select_width;
    undo_move_ht = select_height;			
}

/*
 *
 * ROUTINE:  Update_Undomap
 *
 * ABSTRACT: 
 *
 *	Updates the undomap so it contains the rectangle x_in, y_in, wd_in, 
 *	ht_in.
 *
 */
void Update_Undomap (x_in, y_in, wd_in, ht_in)
    int x_in, y_in, wd_in, ht_in;
{
    int x, y, wd, ht;

  if (undo_available) {

    x = MAX (0, x_in);
    y = MAX (0, y_in);
    wd = MIN (pimage_wd - x, wd_in - (x - x_in));
    ht = MIN (pimage_ht - y, ht_in - (y - y_in));


    if (Inside_Rectangle (x, y, wd, ht, picture_x, picture_y, (int)picture_wd, 
			  (int)picture_ht)) {
	undo_picture_x = picture_x;
	undo_picture_y = picture_y;
    }
    else {
	if (x + (int) picture_wd > pimage_wd)
	    undo_picture_x = pimage_wd - picture_wd;
	else
	    undo_picture_x = x;

	if (y + (int) picture_ht > pimage_ht)
	    undo_picture_y = pimage_ht - picture_ht;
	else 
	    undo_picture_y = y;
    }

    Restore_From_Image (undomap, undo_picture_x, undo_picture_y, 0, 0,
			picture_wd, picture_ht, Get_GC(GC_PD_COPY));

  }
}

/*
 *
 * ROUTINE:  Invert_Piece
 *
 * ABSTRACT: 
 *
 * Inverts the selected area
 *
 */
void Invert_Piece()
{

	Stop_Highlight_Blink();

	Invert_Copymap ();
	Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), TRUE);
	Start_Highlight_Blink();
}

/*
 *
 * ROUTINE:  Invert
 *
 * ABSTRACT: 
 *
 * Inverts the selected area
 *
 */
void Invert()
{
	Invert_Piece();
	undo_action = INVERT;
	Set_Undo_Button( INVERT );
	moved_only = FALSE;
	Clear_UG ();
	deselected = FALSE;
}

/*           
 *
 * ROUTINE:  Find_Highlight
 *
 * ABSTRACT: 
 *
 * Create an array of points that is used to highlight an area.
 *
 */  
void Find_Highlight()
{
int i;

/*
 * "hi_points" is the array containing points used to highlight the selected 
 * area.
 */
    if (hi_points) {
	XtFree ((char *)hi_points);
	hi_points = 0;
    }
    hi_points = (XPoint *) XtMalloc( sizeof(XPoint)*numpts ); /* jj-port */
    num_hipts = numpts;
    for( i = 0; i < numpts; ++i ) {
	hi_points[i].x = IX_TO_PWX(points[i].x);
	hi_points[i].y = IY_TO_PWY(points[i].y);
    }	       
    if ((undo_action != PASTE)  && (undo_action != SCALE) &&
	(undo_action != INCLUDE)) {
	Clip_To_Rect (- picture_x - pic_xorigin, - picture_y - pic_yorigin,
		      pimage_wd, pimage_ht, hi_points, &num_hipts);
    }
}

Region Pimage_Region ()
{
    XPoint pts[5];
	
    pts[0].x = IX_TO_PMX (0);
    pts[0].y = IY_TO_PMY (0);
    pts[1].x = IX_TO_PMX (pimage_wd);
    pts[1].y = IY_TO_PMY (0);	
    pts[2].x = IX_TO_PMX (pimage_wd);
    pts[2].y = IY_TO_PMY (pimage_ht);
    pts[3].x = IX_TO_PMX (0);	
    pts[3].y = IY_TO_PMY (pimage_ht);
    pts[4].x = IX_TO_PMX (0);		
    pts[4].y = IY_TO_PMY (0);

    return (XPolygonRegion (pts, 5, EvenOddRule));
}

/*
 *
 * ROUTINE:  Select
 *
 * ABSTRACT: 
 *
 * Save an area after it has been selected
 *
 */
void Select_Piece()
{
int i;
static XPoint *pts;                
GC gc_copy;
          
    if( (shape_wd>0) && (shape_ht>0) ){
	Set_Cursor_Watch (pwindow);
	select_window = pwindow; /*XtWindow(ifocus);*/

	select_x = MAX (shape_xmin, IX_TO_PMX (0));
	select_y = MAX (shape_ymin, IY_TO_PMY (0));

	prv_select_x = orig_select_x = PMX_TO_IX (select_x);
	prv_select_y = orig_select_y = PMY_TO_IY (select_y);
	shape_wd -= select_x - shape_xmin;
	shape_ht -= select_y - shape_ymin;
	orig_select_wd = select_width = 
	    MIN (shape_wd, pimage_wd - PMX_TO_IX (select_x));
	orig_select_ht = select_height = 
	    MIN (shape_ht, pimage_ht - PMY_TO_IY (select_y)); 
	Find_Highlight();

	orig_num_hipts = num_hipts;
	if (orig_hi_points) {
	    XtFree ((char *)orig_hi_points);
	    orig_hi_points = 0;
	}
	orig_hi_points = (XPoint *) XtMalloc (sizeof (XPoint) * num_hipts); 
	for (i = 0; i < num_hipts; i++) {
	    orig_hi_points[i].x = hi_points[i].x - IX_TO_PWX(select_x);
	    orig_hi_points[i].y = hi_points[i].y - IY_TO_PWY(select_y);
	}

	select_region = XPolygonRegion (orig_hi_points, num_hipts, EvenOddRule);
	orig_select_region = XPolygonRegion (orig_hi_points, orig_num_hipts, 
					     EvenOddRule);
	XOffsetRegion (select_region, select_x, select_y);

	orig_select_rect = select_rectangle;
/* Erase_rband uses the results of find_highlight when selecting an area */
	if( !select_rectangle ) {
/* if prv_num_hipts > 0 then select_piece was called from undo and there is */
/* no rband to erase */
	    if (prv_num_hipts == 0) {
		XDrawLine (disp, XtWindow(ifocus), gc_rband, 
			   rband_x, rband_y, anchor_x, anchor_y );
		Erase_Rband();
	    }
	}
	prv_num_hipts = 0;

/* create pixmaps used in copying areas */
	copymap = Create_Pdepth_Pixmap (0, select_width, select_height);
/* if couldn't create copymap - bail out */
	if (copymap == 0) {
	    Free_Select_Pixmaps ();
	    return;
	}

	gc_copy = Get_GC( GC_PD_FUNCTION );

	if (!select_rectangle) {
/* erase internal area of select */
	    XSetFunction (disp, gc_copy, GXset);
	    XFillPolygon (disp, copymap, gc_copy, orig_hi_points,
			  orig_num_hipts, Complex, CoordModeOrigin);
	    XSetFunction (disp, gc_copy, GXand);
	}
	else {	
	    XSetFunction (disp, gc_copy, GXcopy);
	}

/* fill in the copymap */
	Restore_From_Image (copymap, PMX_TO_IX (select_x), PMY_TO_IY (select_y),
			    0, 0, select_width, select_height, gc_copy);

/* Cut hole in map - used for refresh after move */
	Create_Undo_Move_Map ();
/* if couldn't create undo_move_map - bail out */
	if (undo_move_map == 0) {
	    Free_Select_Pixmaps ();
	    return;
	}

/* if necessary, create the clip_mask */
	if ((!select_rectangle) || (!opaque_fill)) {
	    clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
				       select_width, select_height, 1);
	    if (clip_mask == 0) {
		Free_Select_Pixmaps ();
		return;
	    }


	    Create_Clip_Mask  ();
	}

	if (copymap_image != NULL)
	{
#if 0
	    XtFree ((char *)copymap_image->data);
#endif
	    XDestroyImage (copymap_image);
	    copymap_image = NULL;
	}

	copymap_image = XGetImage (disp, copymap, 0, 0, select_width,
				   select_height, bit_plane, img_format);

	if (copymap_image->bitmap_bit_order != NATIVE_BIT_ORDER) /*ram*/
	    ConvertImageNative (copymap_image);			 /*ram*/

	undo_move_x = PMX_TO_IX (select_x);
	undo_move_y = PMY_TO_IY (select_y);

	Save_Picture_State ();

	select_on = TRUE;
	Start_Highlight_Blink();
	Set_Undo_Button(current_action); /* set undo select */
	Set_Edit_Buttons( SENSITIVE );
	undo_action = current_action; /* in case user undo's the select */
/* For future undo use */
	moved_xdist = 0;
	moved_ydist = 0;
	moved_only = TRUE;
	no_distortion = TRUE;
	deselected = FALSE;
	Start_Motion_Events(ifocus);
	Set_Cursor (pwindow, current_action);
    }
}
             
/*           
 *
 * ROUTINE:  DeSelect
 *
 * ABSTRACT: 
 *
 * Free the the selected area
 *
 */  
void DeSelect(update_undo_toggle)
    int update_undo_toggle;
{
GC gc_copy, gc_mask, gc_solid;

    Set_Cursor_Watch (pwindow);
    Stop_Highlight_Blink();
    gc_copy = Get_GC( GC_PD_COPY );

    if( copymap != 0 )
	XFreePixmap( disp, copymap );
    copymap = 0;

    if (clip_mask != 0) {
	XFreePixmap (disp, clip_mask);
	XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
	clip_mask = 0;
    }

/* Unhighlight the selected area */
    Set_Edit_Buttons (INSENSITIVE);
    XDestroyRegion (select_region);

/* if undo action is select then save the points */
    if (prv_hi_points) {
	XtFree ((char *)prv_hi_points);
	prv_hi_points = 0;
    }
    prv_num_hipts = 0;

    if (update_undo_toggle) {
	if (undo_action == COPY) {
	    undo_action = prv_undo_action;
	    Restore_Toggle_State ();
	    if ((undo_action == SELECT_RECT) || (undo_action == SELECT_AREA))
		Set_Undo_Button (undo_action);
	}

	if ((Undo_Button_State ()) &&
	    (undo_action != SELECT_RECT) && (undo_action != SELECT_AREA)) {
	    if (orig_select_rect)
		undo_action = SELECT_RECT;
	    else
		undo_action = SELECT_AREA;
	    Set_Undo_Button (undo_action);
	}

	if ((undo_action == SELECT_RECT) || (undo_action == SELECT_AREA))
	    Set_Redo_Button ();
	else  {
	    Set_Undo_Button (undo_action);
	}
    }

    switch  (undo_action) {
	case CLEAR:
	case CUT:
	    if (undo_move_map != 0) {
		XFreePixmap( disp, undo_move_map);
		undo_move_map = 0;
	    }
	    undo_width = 0;
	    undo_height = 0;
	    break;
	default :
	    undo_picture_x = PMX_TO_IX (select_x);
	    undo_picture_y = PMY_TO_IY (select_y);
	    if (Intersecting_Rectangles (0, 0, pimage_wd, pimage_ht,
					 undo_picture_x, undo_picture_y,
					 select_width, select_height)) {
		Rectangle_Intersect (0, 0, pimage_wd, pimage_ht,
				     undo_picture_x, undo_picture_y, 
				     select_width, select_height,
				     &undo_x, &undo_y, &undo_width, 
				     &undo_height);
		undo_x -= undo_picture_x;
		undo_y -= undo_picture_y;
	    }
	    else {
		undo_width = undo_height = 0;
	    }

	    Increase_Change_Pix (undo_picture_x + undo_x - picture_x,
				 undo_picture_y + undo_y - picture_y,
				 undo_width, undo_height);
	    break;
    }

    if (hi_points) {
        XtFree ((char *)hi_points);
	hi_points = 0;
    }
    num_hipts = 0;

    select_on = FALSE;
    Stop_Motion_Events(ifocus);

/* Get rid off highlight in the zoom window if necessary */
/* add 1 to width and height to include borders of select - jj 10/7/88 */
    if( zoomed ) {
	Refresh_Magnifier (select_x, select_y, select_width + 1,
                           select_height + 1);
	Refresh_Zoom_View( select_x, select_y, select_width + 1,
			   select_height + 1);
    }
    if (grid_on)
	Display_Grid (select_x, select_y, select_width + 1, select_height + 1);
    deselected = TRUE;
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Clear_Piece
 *
 * ABSTRACT: 
 *
 * Clear the selected piece.  Save the original in case of an undo.
 *
 */
void Clear_Piece()
{
    Stop_Highlight_Blink();

    Restore_Image (undo_move_map, PMX_TO_IX(select_x), PMY_TO_IY(select_y), 
		   0, 0, select_width, select_height, Get_GC (GC_PD_COPY),
		   TRUE);

    undo_action = CLEAR;
    DeSelect (TRUE);
}

/*
 *
 * ROUTINE:  Select_All
 *
 * ABSTRACT: 
 *
 * Select the entire picture
 *
 */
void Select_All()
{
    if (entering_text)
	End_Text ();
    if (num_hipts != 0)
	DeSelect(TRUE);
    numpts = 5;		
    points[0].x = pic_xorigin;
    points[0].y = pic_yorigin;
    points[1].x = pic_xorigin + pwindow_wd-1;
    points[1].y = pic_yorigin;	
    points[2].x = pic_xorigin + pwindow_wd-1;
    points[2].y = pic_yorigin + pwindow_ht-1;
    points[3].x = pic_xorigin;	
    points[3].y = pic_yorigin + pwindow_ht-1;
    points[4].x = pic_xorigin;		
    points[4].y = pic_yorigin;

    shape_xmin = pic_xorigin;
    shape_ymin = pic_yorigin;
    shape_wd = pwindow_wd;
    shape_ht = pwindow_ht;

    Set_Select_Icon (SELECT_RECT);
    select_rectangle = TRUE;
    ifocus = picture_widget;
    Select_Piece();
    numpts = 0;

    Increase_Change_Pix (undo_x, undo_y, undo_width, undo_height);
}

/*           
 *
 * ROUTINE:  Move_Area
 *
 * ABSTRACT: 
 *
 * Move the selected area
 * 
 */
void Move_Area()
{
int i;
int change_x, change_y;
GC gc_copy, gc_mask;

/* save in case additional moves are made */
    gc_copy = Get_GC(GC_PD_COPY);

/* save from the image into the undo move map */
    Restore_From_Image (undo_move_map, PMX_TO_IX (select_x), 
			PMY_TO_IY (select_y), 0, 0, select_width, select_height,
			Get_GC(GC_PD_COPY));



/* draw the select into the image */
    Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), FALSE);

/* Re-set the point values */
    for( i=0; i < num_hipts; ++i ){
	hi_points[i].x += moved_xdist;
	hi_points[i].y += moved_ydist;
    }          
    
    prv_select_x = orig_select_x;
    prv_select_y = orig_select_y;

    shape_xmin += moved_xdist;
    shape_ymin += moved_ydist;
    XOffsetRegion( select_region, moved_xdist, moved_ydist );
    moved_xdist = 0;
    moved_ydist = 0;
             
    Start_Highlight_Blink();

/* Update the zoom window */
    if( zoomed ){
	Refresh_Zoom_View( zoom_xorigin, zoom_yorigin, zoom_width,zoom_height);
	Refresh_Magnifier( zoom_xorigin, zoom_yorigin, zoom_width,zoom_height);
    }
    Set_Undo_Button(current_action); /* set undo move */
}            

Quick_Copy () {
    int tmp_x, tmp_y;
    int i;

    Stop_Highlight_Blink();
    
    tmp_x = select_x;
    tmp_y = select_y;
    qc_x = PMX_TO_IX (select_x);
    qc_y = PMY_TO_IY (select_y);
    if (undo_available) {
	Clear_UG ();
	UG_image[UG_last] = XGetImage (disp, undo_move_map, 0, 0, select_width,
				       select_height, bit_plane, img_format);
	UG_used[0] = UG_last;
	UG_num_used = 1;
    }

    select_x = PWX_TO_IX(pwindow_wd/2 - select_width/2);
    select_y = PWY_TO_IY(pwindow_ht/2 - select_height/2);
    prv_select_x = orig_select_x = PMX_TO_IX (select_x);
    prv_select_y = orig_select_y = PMY_TO_IY (select_y);
    orig_select_wd = select_width;
    orig_select_ht = select_height;
    orig_select_rect = select_rectangle;
    if (orig_num_hipts != num_hipts) {
	if (orig_hi_points) {
	    XtFree ((char *)orig_hi_points);
	    orig_hi_points = 0;
	}
	orig_hi_points = (XPoint *) XtMalloc (sizeof(XPoint) * num_hipts);
	orig_num_hipts = num_hipts;
    }
    for (i = 0; i < num_hipts; ++i) {
	hi_points[i].x += select_x - tmp_x;
	hi_points[i].y += select_y - tmp_y;
	orig_hi_points[i].x = hi_points[i].x - IX_TO_PWX(select_x);
	orig_hi_points[i].y = hi_points[i].y - IY_TO_PWY(select_y);
    }
    XOffsetRegion (select_region, select_x - tmp_x, select_y - tmp_y);
    if (!moved_only) {
	if (copymap_image != NULL)
	{
#if 0
	    XtFree ((char *)copymap_image->data);
#endif
	    XDestroyImage (copymap_image);
	    copymap_image = NULL;
	}
	copymap_image = XGetImage (disp, copymap, 0, 0, select_width,
				   select_height, bit_plane, img_format);
	if (copymap_image->bitmap_bit_order != NATIVE_BIT_ORDER) /*ram*/
	    ConvertImageNative (copymap_image);
    }
    
    Restore_From_Image (undo_move_map, orig_select_x, orig_select_y, 0, 0, 
			select_width, select_height, Get_GC(GC_PD_COPY));

    Copy_Select_To_Image (orig_select_x, orig_select_y, TRUE);

    moved_xdist = 0;
    moved_ydist = 0;
    moved_only = TRUE;
    no_distortion = TRUE;
    deselected = FALSE;
    Start_Highlight_Blink();
    undo_action = QUICK_COPY;
    Set_Undo_Button (QUICK_COPY);
    qc_redo = FALSE;
}
