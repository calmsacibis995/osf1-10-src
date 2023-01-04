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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/fullview.c,v 1.1.2.3 92/12/11 08:34:55 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module FULLVIEW "V1-000"
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
**   Jonathan Joseph, March 1989
**
**  ABSTRACT:
**
**   FULLVIEW contains routines that are called to operate the full view
**   mechanism.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**--
**/           
#include "paintrefs.h"
#include "icons.h"

#define K_FV_SUCCESS 0
#define K_NO_MEM_SERVER 1
#define K_NO_MEM_CLIENT 2

#define MIN_POSW_IM_WD 14
#define MIN_POSW_IM_HT 14
#define MIN_POSW_WIN_WD 8
#define MIN_POSW_WIN_HT 8

#define POS_W_GRID_X_INC 8
#define POS_W_GRID_Y_INC 8

extern Force_Return_On_Failure();

/* static Widget select_portion_dialog = 0; */
static int sp_cropped;
static int pre_fv_picture_wd, pre_fv_picture_ht;
static int pre_fv_pic_xorigin, pre_fv_pic_yorigin;
static int portion_window_ht, portion_window_wd;
static int portion_x_left, portion_x_right, portion_y_top, portion_y_bottom;
static XPoint portion_pts[5];
static float visual_scale;

static Pixmap pos_window_pix;
static int no_position = FALSE;
static int update_pos_window_image_frame = FALSE;
static int pos_window_wd = 0, pos_window_ht = 0;

static int pos_w_im_x;
static int pos_w_im_y;
static int pos_w_im_wd, pos_w_im_ht;

static int pos_w_win_x = -1;
static int pos_w_win_y = -1;
static int pos_w_win_wd, pos_w_win_ht;
static float pos_w_win_x_inc, pos_w_win_y_inc;

static int pos_w_grid_x_left, pos_w_grid_x_right;
static int pos_w_grid_y_top, pos_w_grid_y_bottom;

static int pos_window_frame_grabbed = FALSE;


/*
 * MAJOR HACK TIME - I DON'T KNOW WHAT'S GOING ON, BUT THIS PREVENTS SOME
 * MAJOR SLOWNESS IN XGETIMAGE CALLS LATER ON
 *
 * What it does.  Finds the pixel in the middle of the picture pixmap, finds
 * its color, set's the foreground of the PD_SOLID GC to that color, then 
 * redraws that point.
 */
void hack_1 ()
{
    XImage *ximage;
    GC gc;
    int value;

    ximage = XGetImage (disp, picture, picture_wd / 2, picture_ht / 2, 
			1, 1, bit_plane, img_format);
    value = XGetPixel (ximage, 0, 0);
    XDestroyImage (ximage);
    gc = Get_GC (GC_PD_SOLID);
    XSetForeground (disp, gc, (value == picture_fg) ? picture_bg : picture_fg);
    XSetBackground (disp, gc, (value == picture_fg) ? picture_fg : picture_bg);
    XDrawPoint (disp, picture, gc, picture_wd / 2, picture_ht / 2);
    XSetForeground (disp, gc, value);
    XSetBackground (disp, gc, (value == picture_bg) ? picture_fg : picture_bg);
    XDrawPoint (disp, picture, gc, picture_wd / 2, picture_ht / 2);
}


void Redraw_Pos_Window (x, y, wd, ht)
    int x, y, wd, ht;
{
    if ((!widget_ids[POSITION_WINDOW]) || no_position)
	return;

    XCopyPlane (disp, pos_window_pix, XtWindow (widget_ids[POSITION_WINDOW]),
		Get_GC(GC_MD_COPY), x, y, wd, ht, x, y, 1);
}

void Redraw_Pos_Window_Pix ()
{
    int x, y;
    GC gc;
    int i, j;
    static int first_time = TRUE;
    static int tmp_x, tmp_y, tmp_wd, tmp_ht;
    int erase_old = TRUE;

    if ((!widget_ids[POSITION_WINDOW]) || no_position)
	return;

    if (first_time) {
	tmp_x = pos_w_win_x;
	tmp_y = pos_w_win_y;
	tmp_wd = pos_w_win_wd;
	tmp_ht = pos_w_win_ht;
	first_time = FALSE;
    }

    gc = Get_GC (GC_D1_SOLID);

/* If we need to, draw the image frame */
    if (update_pos_window_image_frame) {
	XSetForeground (disp, gc, 0);
	XFillRectangle (disp, pos_window_pix, gc, 0, 0, pos_window_wd,
			pos_window_ht);
	XFillRectangle (disp, pos_window_pix, Get_GC (GC_D1_GRAY), pos_w_im_x,
			pos_w_im_y, pos_w_im_wd, pos_w_im_ht);
	XFillRectangle (disp, pos_window_pix, gc, pos_w_im_x + 3,
			pos_w_im_y + 3, pos_w_im_wd - 6,
			pos_w_im_ht - 6);
	update_pos_window_image_frame = FALSE;
/* already erased the old window frame */
	erase_old = FALSE;
    }

/* Erase old window frame */
    if (erase_old) {
	XSetForeground (disp, gc, 0);
	XFillRectangle (disp, pos_window_pix, gc, tmp_x, tmp_y, tmp_wd, tmp_ht);
    }

/* Draw the new window frame */
    tmp_x = pos_w_win_x;
    tmp_y = pos_w_win_y;
    tmp_wd = pos_w_win_wd;
    tmp_ht = pos_w_win_ht;

    XSetForeground (disp, gc, 1);
    XFillRectangle (disp, pos_window_pix, gc, tmp_x, tmp_y, tmp_wd, tmp_ht);
    XSetForeground (disp, gc, 0);
    XFillRectangle (disp, pos_window_pix, gc, tmp_x + 3, tmp_y + 3,
		    tmp_wd - 6, tmp_ht - 6);

/* Draw the grid points */
    XSetForeground (disp, gc, 1);
    for (x = pos_w_grid_x_left; x <= pos_w_grid_x_right;
	 x += POS_W_GRID_X_INC) {
	for (y = pos_w_grid_y_top; y <= pos_w_grid_y_bottom; 
	     y += POS_W_GRID_Y_INC) {
	    XDrawPoint (disp, pos_window_pix, gc, x, y);
	}
    }

    Redraw_Pos_Window (0, 0, pos_window_wd, pos_window_ht);
}


void Update_Pos_Window_Window_Pos (update)
    int update;
{
    int x, y;
    int change = FALSE;

    if ((!widget_ids[POSITION_WINDOW]) || no_position)
	return;

    if (paint_view == FULL_VIEW) {
	0;
    }
    else {
	x = (((float) PMX_TO_IX (pic_xorigin)) / pos_w_win_x_inc) + 0.5;
	x = MIN (x, pos_w_im_wd - pos_w_win_wd - 6);
    }
    x += pos_w_im_x + 3;

    if ((x != pos_w_win_x) || update) {
	pos_w_win_x = x;
	change = TRUE;
    }
        
    if (paint_view == FULL_VIEW) {
	y = 0;
    }
    else {
	y = (((float) PMY_TO_IY (pic_yorigin)) / pos_w_win_y_inc) + 0.5;
	y = MIN (y, pos_w_im_ht - pos_w_win_ht - 6);
    }
    y += pos_w_im_y + 3;

    if ((y != pos_w_win_y) || update) {
	pos_w_win_y = y;
	change = TRUE;
    }
    if (change)
	Redraw_Pos_Window_Pix ();	
}


void Update_Pos_Window_Window_Size ()
{
    int wd, ht;
    float pix_wd, pix_ht;

    if ((!widget_ids[POSITION_WINDOW]) || no_position)
	return;

    if ((pwindow_wd == pimage_wd) || (paint_view == FULL_VIEW)) {
	pos_w_win_wd = pos_w_im_wd - 6;
	pos_w_win_x_inc = 1;
    }
    else {
	pix_wd = ((float)pimage_wd) / ((float)pos_w_im_wd);
	pos_w_win_wd = (((float)pwindow_wd) / pix_wd) + 0.5;
	pos_w_win_wd = MIN (pos_w_win_wd, pos_w_im_wd - 7);
	pos_w_win_wd = MAX (pos_w_win_wd, MIN_POSW_WIN_WD);

	pos_w_win_x_inc = ((float) (pimage_wd - pwindow_wd)) / 
			  ((float)(pos_w_im_wd - pos_w_win_wd - 6));
    }

    if ((pwindow_ht == pimage_ht) || (paint_view == FULL_VIEW)) {
	pos_w_win_ht = pos_w_im_ht - 6;
	pos_w_win_y_inc = 1;
    }
    else {
	pix_ht = ((float)pimage_ht) / ((float)(pos_w_im_ht - 3));
	pos_w_win_ht = (((float)pwindow_ht) / pix_ht) + 0.5;
	pos_w_win_ht = MIN (pos_w_win_ht, pos_w_im_ht - 7);
	pos_w_win_ht = MAX (pos_w_win_ht, MIN_POSW_WIN_HT);

	pos_w_win_y_inc = ((float) (pimage_ht - pwindow_ht)) / 
			  ((float)(pos_w_im_ht - pos_w_win_ht - 6));
    }

    pos_w_win_x = -1;
    pos_w_win_y = -1;
    update_pos_window_image_frame = TRUE;
    Update_Pos_Window_Window_Pos (TRUE);
}


void Update_Pos_Window_Image_Size ()
{
    float wd_scale, ht_scale;
    int wd, ht;
    int i;

    if ((!widget_ids[POSITION_WINDOW]) || no_position)
	return;

    if (!pos_window_wd) {
	pos_window_wd = XtWidth (widget_ids[POSITION_WINDOW]);
	pos_window_ht = XtHeight (widget_ids[POSITION_WINDOW]);
	pos_window_pix = Create_Bitmap (0, pos_window_wd, pos_window_ht);
	if (!pos_window_pix) {
	    no_position = TRUE;
	    XtUnmanageChild (widget_ids[POSITION_WINDOW]);
	    Display_Message ("T_NO_MEM_FOR_POSITION");
	    return;
	}
    }

    wd_scale = ((float)pimage_wd) / ((float)pos_window_wd);
    ht_scale = ((float)pimage_ht) / ((float)pos_window_ht);

    if (wd_scale > ht_scale) {
	pos_w_im_x = 0;
	pos_w_im_wd = pos_window_wd;
	pos_w_im_ht = (((float)pimage_ht) / wd_scale) + 0.5;
	pos_w_im_ht = MIN (pos_w_im_ht, pos_window_ht);
	pos_w_im_ht = MAX (pos_w_im_ht, MIN_POSW_IM_HT);
	pos_w_im_y = (pos_window_ht - pos_w_im_ht) / 2;
    }
    else {
	pos_w_im_y = 0;
	pos_w_im_ht = pos_window_ht;
	pos_w_im_wd = (((float)pimage_wd) / ht_scale) + 0.5;
	pos_w_im_wd = MIN (pos_w_im_wd, pos_window_wd);
	pos_w_im_wd = MAX (pos_w_im_wd, MIN_POSW_IM_WD);
	pos_w_im_x = (pos_window_wd - pos_w_im_wd) / 2;
    }

/* find grid starting and ending points */
    wd = pos_w_im_wd - 6;
    if (wd <= POS_W_GRID_X_INC)
	i = (wd + 1) / 2;
    else {
	for (i = POS_W_GRID_X_INC / 2; i < POS_W_GRID_X_INC; i++) {
	    if ((i == ((wd - i - 1) % POS_W_GRID_X_INC)) ||
		(i == (((wd - i - 1) % POS_W_GRID_X_INC) + 1))) {
		break;
	    }
	}
    }
    pos_w_grid_x_left = pos_w_im_x + 3 + i;
    pos_w_grid_x_right = pos_w_im_x + pos_w_im_wd - 4;

    ht = pos_w_im_ht - 6;
    if (ht <= POS_W_GRID_Y_INC)
	i = (ht + 1) / 2;
    else {
	for (i = POS_W_GRID_Y_INC / 2; i < POS_W_GRID_Y_INC; i++) {
	    if ((i == ((ht - i - 1) % POS_W_GRID_Y_INC)) ||
		(i == (((ht - i - 1) % POS_W_GRID_Y_INC) + 1))) {
		break;
	    }
	}
    }

    pos_w_grid_y_top = pos_w_im_y + 3 + i;
    pos_w_grid_y_bottom = pos_w_im_y + pos_w_im_ht - 4;

    Update_Pos_Window_Window_Size ();
}


void Refresh_Pos_Window (w, event, params, num_params)
    Widget w;
    XExposeEvent *event;
    char **params;
    int num_params;
{
    static int first_time = TRUE;

    if (first_time) {
	Update_Pos_Window_Image_Size ();
	first_time = FALSE;
    }
    else {
	Redraw_Pos_Window (event->x, event->y, event->width, event->height);
    }
}


void Find_Pos_Window_Window_Coords (x_in, y_in)
    int x_in, y_in;
{
    int x, y;

    x = x_in - (pos_w_win_wd / 2);
    x = MAX (x, pos_w_im_x + 3);
    x = MIN (x, pos_w_im_x + pos_w_im_wd - pos_w_win_wd - 3);

    y = y_in - (pos_w_win_ht / 2);
    y = MAX (y, pos_w_im_y + 3);
    y = MIN (y, pos_w_im_y + pos_w_im_ht - pos_w_win_ht - 3);

    if ((x != pos_w_win_x) || (y != pos_w_win_y)) {
	pos_w_win_x = x;
	pos_w_win_y = y;
	Redraw_Pos_Window_Pix (0, 0, pos_window_wd, pos_window_ht);
    }
}


void Clicked_In_Pos_Window (w, event, params, num_params)
    Widget w;
    XButtonPressedEvent *event;
    char **params;
    int num_params;
{
/* If it's not possible to move it anywhere, don't even try */
    if ((paint_view == FULL_VIEW) ||
	((pimage_wd == pwindow_wd) && (pimage_ht == pwindow_ht)))
	return;

    if ((event->x >= (pos_w_im_x + 3)) &&
	(event->x <= (pos_w_im_x + pos_w_im_wd - 4)) &&
	(event->y >= (pos_w_im_y + 3)) &&
	(event->y <= (pos_w_im_y + pos_w_im_ht - 4))) {
	pos_window_frame_grabbed = TRUE;
	Find_Pos_Window_Window_Coords (event->x, event->y);
    }
}


void Position_Moved_Mouse (w, event, params, num_params)
    Widget  w;
    XMotionEvent *event;
    char **params;
    int num_params;
{
    XEvent nextevent;
    int more_motion;
    XMotionEvent *cur_event;

    if (pos_window_frame_grabbed) {
/* Compress the motion events */
	cur_event = event;
	more_motion = TRUE;
	while (more_motion) {
	    if (XtPending ()) {
		XtPeekEvent (&nextevent);
		if (nextevent.type == MotionNotify) {
		    XtNextEvent (&nextevent);
		    cur_event = (XMotionEvent *) &nextevent;
		}
		else
		    more_motion = FALSE;
	    }
	    else
		more_motion = FALSE;
	}

/* jj - 3/30/89  If event came from a different screen, ignore it. */
	if (cur_event->same_screen) {
	    Find_Pos_Window_Window_Coords (cur_event->x, cur_event->y);
	}
    }
}


void Released_In_Pos_Window (w, event, params, num_params)
    Widget w;
    XButtonReleasedEvent *event;
    char **params;
    int num_params;
{
    int x, y;
    int new_x, new_y;

    if (pos_window_frame_grabbed) {
	x = pos_w_win_x - pos_w_im_x - 3;
	y = pos_w_win_y - pos_w_im_y - 3;

	if (x == (pos_w_im_x + pos_w_im_wd - pos_w_win_wd - 3))
	    new_x = pimage_wd - pwindow_wd;
	else {
	    new_x = (((float)x) * pos_w_win_x_inc) + 0.5;
	    new_x = MIN (new_x, pimage_wd - pwindow_wd);
	}

	if (y == (pos_w_im_y + pos_w_im_ht - pos_w_win_ht - 3))
	    new_y = pimage_ht - pwindow_ht;
	else {
	    new_y = (((float)y) * pos_w_win_y_inc) + 0.5;
	    new_y = MIN (new_y, pimage_ht - pwindow_ht);
	}
	Move_Picture (new_x, new_y);
	pos_window_frame_grabbed = FALSE;
/* reset the scroll bars */
	Set_Attribute (widget_ids[PAINT_H_SCROLL_BAR], XmNvalue, new_x);
	Set_Attribute (widget_ids[PAINT_V_SCROLL_BAR], XmNvalue, new_y);
    }
}


/*
 *
 * ROUTINE:  Scale_Image
 *
 * ABSTRACT: 
 *                                
 *   Scale the selected area and display it in the Select Portion window
 *
 */
int Scale_Image (scale_value)
    float scale_value;
{
    unsigned long image_id, display_frame_id, scale_frame_id;
    char *image_data;
    int image_size;
    XImage *ximage;
    int set_index, get_index;
    struct PUT_ITMLST set_attributes[7];
    struct GET_ITMLST get_attributes[6];
    int off_modulo;
    int bytes;
    long image_bytes, pixel_order;
    int pixels_per_scanline, scanline_count;
    long scanline_stride, bits_per_pixel;
    GC gc_solid;
    float scale_xy;

    if (scale_value != 1.0) {

	if (picture_image->bitmap_bit_order != NATIVE_BIT_ORDER) /*ram*/
	    ConvertImageNative (picture_image);                  /*ram*/

	bits_per_pixel = (pdepth == 1) ? 1 : 8;
	pixel_order = ImgK_StandardPixelOrder; 

	bytes = picture_image->bytes_per_line;
	scanline_stride = 8 * bytes;	/* 8 bits per byte */
	image_data = picture_image -> data;
	image_size = bytes * pimage_ht;

	start_set_itemlist(set_attributes, set_index);
	put_set_item (set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item (set_attributes, set_index, Img_BitsPerPixel,
		      bits_per_pixel);
	put_set_item (set_attributes, set_index, Img_PixelsPerLine, 
		      pimage_wd);
	put_set_item (set_attributes, set_index, Img_NumberOfLines, 
		      pimage_ht);
	put_set_item (set_attributes, set_index, Img_ScanlineStride,
		      scanline_stride);
	end_set_itemlist (set_attributes, set_index);

	image_id = ImgCreateFrame(set_attributes, image_stype);

	ISL_ERROR_HANDLER_SETUP
	image_id = ImgImportBitmap(image_id, image_data, image_size, 0,0,0,0);
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
            return (K_NO_MEM_CLIENT);
	}

	scale_xy = scale_value; /* necessary to work on PMAX */
	
	ISL_ERROR_HANDLER_SETUP
	scale_frame_id = ImgScale( image_id, &scale_xy, 0, 0,
				    ImgM_NearestNeighbor, 0);
	ImgDeleteFrame (image_id);
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
            return (K_NO_MEM_CLIENT);
	}

/*  Display scale_frame_id  */

	start_get_itemlist(get_attributes, get_index);
	put_get_item(get_attributes, get_index, Img_BitsPerPixel, bits_per_pixel);
	put_get_item(get_attributes, get_index, Img_PixelsPerLine, pixels_per_scanline);
	put_get_item(get_attributes, get_index, Img_NumberOfLines, scanline_count);
	put_get_item(get_attributes, get_index, Img_ScanlineStride, scanline_stride);
	end_get_itemlist(get_attributes, get_index);
	
	ImgGetFrameAttributes(scale_frame_id, get_attributes); /* jj-port */
	
	off_modulo = scanline_stride % 32;
	if (off_modulo == 0)
	    display_frame_id = scale_frame_id;
	else {
	    start_set_itemlist(set_attributes, set_index);
	    scanline_stride += 32 - off_modulo;
	    put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	    end_set_itemlist(set_attributes, set_index);

	    ISL_ERROR_HANDLER_SETUP
	    display_frame_id = ImgCopy(scale_frame_id, 0, set_attributes); /* jj-port */
	    ImgDeleteFrame (scale_frame_id);
	    if (failure_occurred) {
		ISL_RECOVER_FROM_ERROR
		return (K_NO_MEM_CLIENT);
	    }
	}

	image_bytes = scanline_stride * scanline_count / 8;

	image_data = (char *) XtCalloc(image_bytes, sizeof(char)); /* jj-port */
	if (image_data == 0) 
	    return (K_NO_MEM_CLIENT);

	ImgExportBitmap(display_frame_id, 0, image_data,
			  image_bytes, 0, 0, 0, 0);
	
	ximage = XCreateImage(disp, visual_info->visual,
			      (bits_per_pixel == 1) ? 1 : pdepth, 
			      img_format, 0, image_data,
			      pixels_per_scanline, scanline_count, 32, 0 );

	ximage->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
        ximage->byte_order = NATIVE_BYTE_ORDER;         /*ram*/
    }
    else
	ximage = picture_image;

/* Copy the image to the pixmap */
    if (!picture) {
	picture = Create_Pdepth_Pixmap (picture_bg, select_portion_wd,
					select_portion_ht);
    }
    else {
	if ((select_portion_wd != picture_wd) ||
	    (select_portion_ht != picture_ht)) {
	    XFreePixmap (disp, picture);
	    picture = Create_Pdepth_Pixmap (picture_bg, select_portion_wd, 
					    select_portion_ht);
	}
    }

    picture_wd = select_portion_wd;
    picture_ht = select_portion_ht;

    if (picture)
	MY_XPutImage (disp, picture, Get_GC(GC_PD_COPY), ximage, 0, 0, 0, 0,
		      picture_wd, picture_ht);

    if ((scale_value != 1.0) && (ximage != NULL))
    {
#if 0
	XtFree (ximage->data);
#endif
	XDestroyImage (ximage);
	ximage=NULL;
    }

    if (picture)
	return (K_FV_SUCCESS);
    else 
	return (K_NO_MEM_SERVER);
}



void Draw_Portion_Rect ()
{
/* draw the selection into the window in invert mode */
    XDrawLines (disp, pwindow, gc_highlight, portion_pts, 5, CoordModeOrigin);
/* draw the selection into the pixmap in invert mode */
    XDrawLines (disp, picture, Get_GC (GC_PD_INVERT), portion_pts, 5,
		CoordModeOrigin);

}


void Set_Portion_Rect_Points ()
{
    portion_pts[0].x = portion_x_left;
    portion_pts[0].y = portion_y_top;
    portion_pts[1].x = portion_x_right;
    portion_pts[1].y = portion_y_top;
    portion_pts[2].x = portion_x_right;
    portion_pts[2].y = portion_y_bottom;
    portion_pts[3].x = portion_x_left;
    portion_pts[3].y = portion_y_bottom;
    portion_pts[4].x = portion_x_left;
    portion_pts[4].y = portion_y_top;
}


void Select_Portion_Rect ()
{
/* contain the selection to the window */
    portion_x_left = MAX (0, rect_x);
    portion_x_right = MIN (pwindow_wd - 1, rect_x + rect_wd);
    portion_y_top = MAX (0, rect_y);
    portion_y_bottom = MIN (pwindow_ht - 1, rect_y + rect_ht);

    if ((portion_x_left == portion_x_right) ||
	(portion_y_top == portion_y_bottom)) {
	return;
    }

    XtSetSensitive (widget_ids[EDIT_CROP_BUTTON], SENSITIVE);

    Set_Portion_Rect_Points ();

/* draw the selection into the pixmap and window */
    Draw_Portion_Rect ();

    sp_select_on = TRUE;
}


void Resize_Portion_Rect (scale)
    float scale;
{
    int right, bottom;

    portion_x_left = (portion_x_left * scale) + 0.5;
    right = (portion_x_right * scale) + 0.5;
    portion_x_right = MIN (pwindow_wd - 1, right);
    portion_y_top = (portion_y_top * scale) + 0.5;
    bottom = (portion_y_bottom * scale) + 0.5;
    portion_y_bottom = MIN (pwindow_ht - 1, bottom);

    Set_Portion_Rect_Points ();

    Draw_Portion_Rect ();
}


void DeSelect_Portion_Rect ()
{
    XtSetSensitive (widget_ids[EDIT_CROP_BUTTON], INSENSITIVE);

/* undraw the selection from the pixmap and window */
    Draw_Portion_Rect ();

    sp_select_on = FALSE;
}


void Exit_Select_Portion ()
{
    Widget wl[2];
    Arg args[5];
    int argcnt;
    int x_coord, y_coord;
    int i;

    Clear_No_Undo_Actions ();
    if (paint_view == FULL_VIEW) {
	Set_Cursor_Watch (pwindow);

	if (sp_select_on)
	    DeSelect_Portion_Rect ();

	paint_view = NORMAL_VIEW;

	wl[0] = widget_ids[PAINT_H_SCROLL_BAR];
	wl[1] = widget_ids[PAINT_V_SCROLL_BAR];
	XtManageChildren (wl, 2);
/*
	XtManageChild (widget_ids[PAINT_H_SCROLL_BAR]);
	XtManageChild (widget_ids[PAINT_V_SCROLL_BAR]);
*/
	Find_Pwindow_Size ();

	if (!sp_cropped) {
	    if (!picture) {
		picture = Create_Pdepth_Pixmap (picture_bg, pre_fv_picture_wd,
					        pre_fv_picture_ht);
	    }
	    else {
		if ((pre_fv_picture_wd != picture_wd) ||
		    (pre_fv_picture_ht != picture_ht)) {
		    XFreePixmap (disp, picture);
		    picture = Create_Pdepth_Pixmap (picture_bg,
						    pre_fv_picture_wd,
						    pre_fv_picture_ht);
		}
	    }
	    if (!picture) {
                exiting_paint = TRUE;
                Display_Message ("T_EXIT_NO_SERVER_MEMORY");
                return;
            }
	}

	picture_wd = pre_fv_picture_wd;
	picture_ht = pre_fv_picture_ht;

	if (pre_fv_pic_xorigin + pwindow_wd > pimage_wd)
	    pre_fv_pic_xorigin = pimage_wd - pwindow_wd;
	if (pre_fv_pic_yorigin + pwindow_ht > pimage_ht)
	    pre_fv_pic_yorigin = pimage_ht - pwindow_ht;

	Find_Pixmap_Position (pre_fv_pic_xorigin, pre_fv_pic_yorigin,
			      &picture_x, &picture_y);

	MY_XPutImage (disp, picture, Get_GC (GC_PD_COPY), picture_image,
		      picture_x, picture_y,  0, 0, picture_wd, picture_ht);

	Refresh_Picture (0, 0, pwindow_wd, pwindow_ht);

	pic_xorigin = IX_TO_PMX (pre_fv_pic_xorigin);
	pic_yorigin = IY_TO_PMY (pre_fv_pic_yorigin);

/* set these values simultaneously for each scroll bar */	
/* First the horizontal scroll bar */
	argcnt = 0;
/* if we cropped it reset slider max values */
	if (sp_cropped) {
	    XtSetArg (args[argcnt], XmNmaximum, pimage_wd);
	    argcnt++;
	}
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_wd);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_wd);
	argcnt++;
/* reset slider values dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNvalue, pre_fv_pic_xorigin);
	argcnt++;
	XtSetValues (widget_ids[PAINT_H_SCROLL_BAR], args, argcnt);

/* Now the vertical scroll bar */
	argcnt = 0;
/* if we cropped it reset slider max values */
	if (sp_cropped) {
	    XtSetArg (args[argcnt], XmNmaximum, pimage_ht);
	    argcnt++;
	}
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_ht);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_ht);
	argcnt++;
/* reset slider values dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNvalue, pre_fv_pic_yorigin);
	argcnt++;
	XtSetValues (widget_ids[PAINT_V_SCROLL_BAR], args, argcnt);

	for (i = 0; i < NUM_ICONS; i++) {
	    if (i != DROPPER_ICON_ID)
		Set_Icon_Button_Sensitivity (i, SENSITIVE);
	}
	if (pdepth != 1) {
	    Set_Icon_Button_Sensitivity (DROPPER_ICON_ID, SENSITIVE);
	}
	Set_Attribute (widget_ids[OPTIONS_PAINT_VIEW_BUTTON], XmNset, TRUE);
	Set_Attribute (widget_ids[OPTIONS_FULL_VIEW_BUTTON], XmNset, FALSE);

	XtSetSensitive (widget_ids[OPTIONS_ZOOM_BUTTON], SENSITIVE);
	XtSetSensitive (widget_ids[OPTIONS_GRID_BUTTON], SENSITIVE);
	XtSetSensitive (widget_ids[EDIT_PASTE_BUTTON], SENSITIVE);
	XtSetSensitive (widget_ids[EDIT_CLEAR_WW_BUTTON], SENSITIVE);
	XtSetSensitive (widget_ids[EDIT_SELECT_ALL_BUTTON], SENSITIVE);
	XtSetSensitive (widget_ids[EDIT_SCALE_PIC_BUTTON], SENSITIVE);
	if (undo_action != NO_ACTION)
	    XtSetSensitive (widget_ids[EDIT_UNDO_BUTTON], SENSITIVE);

	if (sp_cropped)
	    Update_Pos_Window_Image_Size ();
	else
	    Update_Pos_Window_Window_Size ();

/*	hack_1 (); */

	Set_Cursor (pwindow, current_action);
    }
}



void Exit_Select_Portion_OK ()
{
    Exit_Select_Portion ();
}




void SP_Crop ()
{ 
    int x_left, y_top;
    int x_right, y_bottom;
    int new_wd, new_ht;
    XImage *ximage;

    Set_Cursor_Watch (pwindow);

    DeSelect_Portion_Rect ();
/* x_left,y_top = x and y coordinates of rectangle selected from image */
    if (portion_x_left == 0)
	x_left = 0;
    else {
	x_left = (int) ((float)portion_x_left * visual_scale);
	x_left = MAX (x_left, 0);
	x_left = MIN (x_left, pimage_wd - 1);
    }

    if (portion_y_top == 0)
	y_top = 0;
    else  {
	y_top = (int) ((float)portion_y_top * visual_scale);
	y_top = MAX (y_top, 0);
	y_top = MIN (y_top, pimage_ht - 1);
    }

    if (portion_x_right == (pwindow_wd - 1))
	x_right = pimage_wd - 1;
    else {
	x_right = (int) ((float)portion_x_right * visual_scale);
	x_right = MAX (x_right, 0);
	x_right = MIN (x_right, pimage_wd - 1);
    }

    if (portion_y_bottom == (pwindow_ht - 1))
	y_bottom = pimage_ht - 1;
    else {
	y_bottom = (int) ((float)portion_y_bottom * visual_scale);
	y_bottom = MAX (y_bottom, 0);
	y_bottom = MIN (y_bottom, pimage_ht - 1);
    }

    new_wd = x_right - x_left + 1;
    new_ht = y_bottom - y_top + 1;

    if ((new_wd < MIN_PICTURE_WD) || (new_ht < MIN_PICTURE_HT)) {
	Display_Message ("T_INVALID_SIZE");
	return;
    }

    ximage = XSubImage (picture_image, x_left, y_top, new_wd, new_ht);
    if (undo_available) {
	Clear_UG ();
	undo_width = pimage_wd;
	undo_height = pimage_ht;
	UG_image[UG_last] = picture_image;
	UG_used[0] = UG_last;
	UG_num_used = 1;
	undo_action = FULLVIEW_CROP;
	Set_Undo_Button (FULLVIEW_CROP);
    }
    else
    {
#if 0
	XtFree (picture_image->data);
#endif
	XDestroyImage (picture_image);
    }
    picture_image = ximage;

/*
    almost_new_file = TRUE;  * it's as if we got a new file *

    Change_Picture_Size (new_wd, new_ht);
*/

    pimage_wd = new_wd;
    pimage_ht = new_ht;
    pre_fv_pic_xorigin = 0;
    pre_fv_pic_yorigin = 0;
    pre_fv_picture_wd = MIN (pimage_wd, max_picture_wd);
    pre_fv_picture_ht = MIN (pimage_ht, max_picture_ht);

    XFreePixmap (disp, picture);
    picture = 0;
    Create_Pixmaps (picture_bg, pre_fv_picture_wd, pre_fv_picture_ht);
    if (exiting_paint)
        return;
    sp_cropped = TRUE;

    Exit_Select_Portion_OK ();
    if (exiting_paint)
        return;

    Update_Pic_Shape_Dialog_Fields (FALSE);
}



void SP_Resize ()
{
    Arg args[5];
    float vs1, vs2;
    int wd, ht;
    int st;
    float old_vs = visual_scale;

    Set_Cursor_Watch (pwindow);

    if (sp_select_on) {
        Draw_Portion_Rect ();
    }

    wd = XtWidth (widget_ids[PAINT_WINDOW]);
    ht = XtHeight (widget_ids[PAINT_WINDOW]);

    vs1 = (float) pimage_wd / (float) wd;
    vs2 = (float) pimage_ht / (float) ht;

    visual_scale = MAX (vs1, vs2);

    if (visual_scale == vs1) {
	select_portion_wd = wd;
	select_portion_ht = (float) pimage_ht / visual_scale;
    }
    else {
	select_portion_ht = ht;
	select_portion_wd = (float) pimage_wd / visual_scale;
    }

    Find_Pwindow_Size ();
    
    st = Scale_Image (1.0 / visual_scale);

/* If couldn't create pixmap then bail out */
    switch (st) {
	case K_FV_SUCCESS :
	    break;
	case K_NO_MEM_SERVER :
	    Display_Message ("T_NO_FULLVIEW");
	    break;
	case K_NO_MEM_CLIENT :
	    Display_Message ("T_NO_MEM_FOR_FULLVIEW");
	    break;
    }

    if (st != K_FV_SUCCESS) {
        Exit_Select_Portion ();
        return;
    }

    Refresh_Picture (0, 0, pwindow_wd, pwindow_ht);

    if (sp_select_on) {
        Resize_Portion_Rect (old_vs / visual_scale);
    }

    Set_Cursor (pwindow, current_action);
}




/*
 * ROUTINE:  Select_Portion
 *
 * ABSTRACT:
 *
 *   This routine is called if file user wishes to open is larger than the 
 *   screen.  Allows user to select portion of the picture for editing.
 *
 */
void Select_Portion ()
{
    Widget wl[2];
    Arg args[5];
    float vs1, vs2;
    int wd, ht;
    int st;
    int i;

    Set_Cursor_Watch (pwindow);

    if (entering_text)
	End_Text ();
    if (select_on)
	DeSelect (TRUE);
    if (zoomed)
    	Zoom_Off ();
    if (grid_on) 
	Set_Grid ();

/*
    if (undo_available) {
	Clear_UG ();
	Set_Undo_Button (NO_ACTION);
    }
*/

    sp_cropped = FALSE;
    sp_select_on = FALSE;

    pre_fv_picture_wd = picture_wd;
    pre_fv_picture_ht = picture_ht;

    pre_fv_pic_xorigin = PMX_TO_IX (pic_xorigin);
    pre_fv_pic_yorigin = PMY_TO_IY (pic_yorigin);

    picture_x = 0;
    picture_y = 0;
    pic_xorigin = 0;
    pic_yorigin = 0;

/* if pixmap has been changed, save it to the image */
    paint_view = FULL_VIEW;
    Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y, picture,
		  ImgK_Src);


    wl[0] = widget_ids[PAINT_H_SCROLL_BAR];
    wl[1] = widget_ids[PAINT_V_SCROLL_BAR];
    XtUnmanageChildren (wl, 2);
/*
    XtUnmanageChild (widget_ids[PAINT_H_SCROLL_BAR]);
    XtUnmanageChild (widget_ids[PAINT_V_SCROLL_BAR]);
*/

    wd = XtWidth (widget_ids[PAINT_WINDOW]);
    ht = XtHeight (widget_ids[PAINT_WINDOW]);

    vs1 = (float) pimage_wd / (float) wd;
    vs2 = (float) pimage_ht / (float) ht;

    visual_scale = MAX (vs1, vs2);

    if (visual_scale == vs1) {
	select_portion_wd = wd;
	select_portion_ht = (float) pimage_ht / visual_scale;
    }
    else {
	select_portion_ht = ht;
	select_portion_wd = (float) pimage_wd / visual_scale;
    }

    Find_Pwindow_Size ();

    st = Scale_Image (1.0 / visual_scale);

/* If couldn't create pixmap then bail out */
    switch (st) {
	case K_FV_SUCCESS :
	    XtSetSensitive (widget_ids[OPTIONS_ZOOM_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids[OPTIONS_GRID_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids[EDIT_PASTE_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids[EDIT_CLEAR_WW_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids[EDIT_SELECT_ALL_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids[EDIT_SCALE_PIC_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids[EDIT_UNDO_BUTTON], INSENSITIVE);
	    Set_Select_Icon (SELECT_RECT);
	    for (i = 0; i < NUM_ICONS; i++) {
		if (i != SELECT_ICON_ID)
		    Set_Icon_Button_Sensitivity (i, INSENSITIVE);
	    }
	    Set_Attribute (widget_ids[OPTIONS_PAINT_VIEW_BUTTON],
			   XmNset, FALSE);
	    break;
	case K_NO_MEM_SERVER :
	    Display_Message ("T_NO_FULLVIEW");
	    break;
	case K_NO_MEM_CLIENT :
	    Display_Message ("T_NO_MEM_FOR_FULLVIEW");
	    break;
    }

    if (st != K_FV_SUCCESS) {
        Exit_Select_Portion ();
        return;
    }

    Refresh_Picture (0, 0, pwindow_wd, pwindow_ht);
    Update_Pos_Window_Window_Size ();

    Set_Cursor (pwindow, current_action);
}

