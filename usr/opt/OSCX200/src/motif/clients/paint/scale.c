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
static char *BuildSystemHeader= "$Id: scale.c,v 1.1.2.3 92/12/11 08:35:59 devrcs Exp $";
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
**   This module handles scaling a selected area
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

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <DXm/DXmCSText.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

extern Force_Return_On_Failure();

/* static Widget scale_dialog; */
static int scale_id;
static int suicidal = FALSE;

#define SCALE_SUCCESS 0
#define SCALE_TOO_SMALL 1
#define SCALE_TOO_BIG 2
#define NO_MEM_FOR_SCALE_SERVER 3
#define NO_MEM_FOR_SCALE_CLIENT 4

/*
 *
 * ROUTINE:  Reset_Select
 *
 * ABSTRACT: 
 *                                
 *   Setup the select area after a scale 
 *
 */
int Reset_Select (ximage)
XImage *ximage;
{
int wd, ht;
GC gc_copy;
Pixmap tmpmap;

    Stop_Highlight_Blink();

/* update the image from the undo_move_map */
    Restore_Image (undo_move_map, PMX_TO_IX (select_x), PMY_TO_IY (select_y),
                   0, 0, select_width, select_height, Get_GC(GC_PD_COPY),
                   FALSE);
    XFreePixmap (disp, copymap);
    copymap = 0;
    XFreePixmap (disp, undo_move_map);
    undo_move_map = 0;
    if (clip_mask != 0) {
        XFreePixmap (disp, clip_mask);
        clip_mask = 0;
        XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
    }

/* attempt to allocate all pixmaps first */
/* Create new copymap */
    copymap = Create_Pdepth_Pixmap (picture_bg, pixels_per_scanline,
				    scanline_count);
    if (copymap) {
        undo_move_map = Create_Pdepth_Pixmap (picture_bg, pixels_per_scanline,
                                              scanline_count);
	undo_move_wd = pixels_per_scanline;
	undo_move_ht = scanline_count;
        if ((undo_move_map) && (!opaque_fill)) {
            clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
                                       pixels_per_scanline, scanline_count, 1);
        }
    }

    if ((copymap == 0) || (undo_move_map == 0) ||
        ((!opaque_fill) && (clip_mask == 0))) {
/* if undo_move_map is 0 then selmap was succesfully allocated */
        if (copymap) {
            XFreePixmap (disp, copymap);
            copymap = 0;
        }
        if (undo_move_map) {
            XFreePixmap (disp, undo_move_map);
            undo_move_map = 0;
        }
	Set_Undo_Button (NO_ACTION);
	Start_Highlight_Blink();
	return (NO_MEM_FOR_SCALE_SERVER);
    }

    select_rectangle = TRUE;

/* Set up new select region */
/*
    prv_select_x = PMX_TO_IX (select_x);
    prv_select_y = PMY_TO_IY (select_y);
*/
    moved_xdist = 0;
    moved_ydist = 0;
		 
    wd = MAX (select_width, pixels_per_scanline);
    ht = MAX (select_height, scanline_count);
    select_width = pixels_per_scanline;
    select_height = scanline_count;
    
    MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), ximage, 0, 0, 0, 0,
                  select_width, select_height);

    if (!opaque_fill)
	Create_Clip_Mask ();
    else
	XSetClipMask (disp, Get_GC (GC_PD_MASK), None);

/* Create new undo_move_map */
    Restore_From_Image (undo_move_map, PMX_TO_IX (select_x),
                        PMY_TO_IY (select_y), 0, 0, select_width,
                        select_height, Get_GC(GC_PD_COPY));

/* Copy copymap into picture */
    Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), FALSE);


    undo_action = SCALE; /* in case user undo's the paste */
    numpts = 5;		
    points[0].x = select_x;		
    points[0].y = select_y;		
    points[1].x = select_x + select_width;		
    points[1].y = select_y;		
    points[2].x = points[1].x;		
    points[2].y = select_y + select_height;		
    points[3].x = select_x;		
    points[3].y = points[2].y;		
    points[4].x = points[0].x;		
    points[4].y = points[0].y;
    Find_Highlight();		
    select_region = XPolygonRegion( points, numpts, EvenOddRule );
    numpts = 0;
    first_move = FALSE;
    select_on = TRUE;
    Set_Undo_Button(SCALE); /* set undo select */
    Set_Edit_Buttons( SENSITIVE );
    Refresh_Picture( select_x, select_y, wd, ht );
    snafu = TRUE;

    Start_Highlight_Blink();
    return (SCALE_SUCCESS);
}

/*
 *
 * ROUTINE:  Display_Frame
 *
 * ABSTRACT: 
 *                                
 *   Display the ISL frame
 *
 */
int Display_Frame (image_id)
unsigned long image_id;
{
int get_index;
struct GET_ITMLST get_attributes[6];
char *image_data;
XImage *ximage;
unsigned long display_frame_id;
int off_modulo;
int image_bytes;
int set_index;
struct PUT_ITMLST set_attributes[7];
int st;

    start_get_itemlist(get_attributes, get_index);
    put_get_item(get_attributes, get_index, Img_BitsPerPixel, bits_per_pixel);
    put_get_item(get_attributes, get_index, Img_PixelsPerLine, pixels_per_scanline);
    put_get_item(get_attributes, get_index, Img_NumberOfLines, scanline_count);
    put_get_item(get_attributes, get_index, Img_ScanlineStride, scanline_stride);
    end_get_itemlist(get_attributes, get_index);
	
    ImgGetFrameAttributes(image_id, get_attributes); /* jj-port */

    off_modulo = scanline_stride % 32;
    if (off_modulo == 0)
    	display_frame_id = image_id;
    else{
	start_set_itemlist(set_attributes, set_index);
	scanline_stride += 32 - off_modulo;
	put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	end_set_itemlist(set_attributes, set_index);
 
	ISL_ERROR_HANDLER_SETUP
 	display_frame_id = ImgCopy(image_id, 0, set_attributes); /* jj-port */
        if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
            return (NO_MEM_FOR_SCALE_CLIENT);
        }
    }

    image_bytes = scanline_stride * scanline_count / 8;
    image_data = (char *) XtCalloc(image_bytes, sizeof(char)); /* jj-port */
    if (image_data == 0) {
	if (display_frame_id != image_id)
	    ImgDeleteFrame (display_frame_id);
	return (NO_MEM_FOR_SCALE_CLIENT);
    }

    ImgExportBitmap(display_frame_id, 0, image_data, image_bytes, 0, 0, 0, 0);
    if (display_frame_id != image_id)
	ImgDeleteFrame (display_frame_id);
	
    ximage = XCreateImage (disp, visual_info->visual,
			   (bits_per_pixel == 1) ? 1 : pdepth,
			   img_format, 0, image_data,
			   pixels_per_scanline, scanline_count, 32, 0);
    if (ximage == NULL) return (0);

    ximage->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
    ximage->byte_order = NATIVE_BYTE_ORDER;         /*ram*/


/* Set up the select area from the paste */
    st = Reset_Select (ximage);

#if 0
    XtFree (ximage->data);
#endif
    XDestroyImage (ximage);

    return (st);
}


/*
 *
 * ROUTINE:  Scale
 *
 * ABSTRACT: 
 *                                
 *   Scale the selected area
 *
 */
int Scale (x_scale_value, y_scale_value)
    float x_scale_value, y_scale_value;
{
    unsigned long image_id;
    char *image_data;
    int image_size;
    Pixmap tmpmap;
    int set_index;
    struct PUT_ITMLST set_attributes[7];
    unsigned long scale_frame_id;
    GC gc_copy;
    float scale_x, scale_y; /* jj-port */
    int st = SCALE_SUCCESS;
    XImage *ximage;

/* check to see if scaled up area will fit in the pixmap */
    if (((select_width * x_scale_value) > screen_wd) ||
	((select_height * y_scale_value) > screen_ht))
	return(SCALE_TOO_BIG);
    if (((select_width * x_scale_value) < 1) ||
	((select_height * y_scale_value) < 1))
	return (SCALE_TOO_SMALL);
    else {
	Set_Cursor_Watch (pwindow);
	if( !select_rectangle ) {
/* turn 0's around perimeter into background before saving to the image */
            gc_copy = Get_GC (GC_PD_FUNCTION);
            XSetFunction (disp, gc_copy, GXxor);
            XSetForeground (disp, gc_copy, picture_bg);
            XFillPolygon (disp, copymap, gc_copy, orig_hi_points,
			  orig_num_hipts, Complex, CoordModeOrigin);
            XFillRectangle (disp, copymap, gc_copy, 0, 0, select_width,
                            select_height);
            Set_Select_Icon (SELECT_RECT);  /* set the select rect icon */
            XSetForeground (disp, gc_copy, picture_fg);
        }

        ximage = XGetImage (disp, copymap, 0, 0, select_width,
                            select_height, bit_plane, img_format );
	if (ximage == NULL) return (0);

        if (ximage->bitmap_bit_order != NATIVE_BIT_ORDER) /*ram*/
            ConvertImageNative (ximage);                  /*ram*/

	bits_per_pixel = (pdepth == 1) ? 1 : 8;
	pixel_order = ImgK_StandardPixelOrder; 

        bytes = ximage->bytes_per_line;
        pixels_per_scanline = ximage-> width;
        scanline_count = ximage -> height;
        scanline_stride = 8 * bytes;    /* 8 bits per byte */
        image_data = ximage -> data;
        image_size = bytes * scanline_count;

	start_set_itemlist(set_attributes, set_index);
	put_set_item(set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item(set_attributes, set_index, Img_BitsPerPixel, bits_per_pixel);
	put_set_item(set_attributes, set_index, Img_PixelsPerLine, pixels_per_scanline);
	put_set_item(set_attributes, set_index, Img_NumberOfLines, scanline_count);
	put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	end_set_itemlist(set_attributes, set_index);

	image_id = ImgCreateFrame(set_attributes, image_stype);

	ISL_ERROR_HANDLER_SETUP
	image_id = ImgImportBitmap(image_id, image_data, image_size, 0, 0, 0,
				     0); /* jj-port */
#if 0
	XtFree (ximage->data);
#endif
	XDestroyImage (ximage);
	ximage = NULL;

	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
/* reset cursor before premature return */
            Set_Cursor (pwindow, current_action);
	    return (NO_MEM_FOR_SCALE_CLIENT);	    
	}
	
	scale_x = x_scale_value; /* jj-port */
	scale_y = y_scale_value; /* jj-port */

	ISL_ERROR_HANDLER_SETUP
	scale_frame_id = ImgScale (image_id, &scale_x, &scale_y, 0,
				    ImgM_NearestNeighbor, 0); /* jj-port */
	ImgDeleteFrame (image_id);
	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
/* reset cursor before premature return */
            Set_Cursor (pwindow, current_action);
	    return (NO_MEM_FOR_SCALE_CLIENT);	    
	}

	st = Display_Frame (scale_frame_id);
	ImgDeleteFrame (scale_frame_id);
	Set_Cursor (pwindow, current_action);
	return (st);
    }
}

/*
 *
 * ROUTINE:  Scale_Picture
 *
 * ABSTRACT: 
 *                                
 *   Scale the entire picture
 *
 */
int Scale_Picture (x_scale_value, y_scale_value)
    float x_scale_value, y_scale_value;
{
    unsigned long image_id, scale_image_id;
    char *image_data;
    int image_bytes;
    int image_size;
    int set_index, get_index;
    struct PUT_ITMLST set_attributes[7];
    struct GET_ITMLST get_attributes[6];
    unsigned long display_frame_id;
    int off_modulo;
    float scale_x, scale_y; /* jj-port */


    int new_wd = pimage_wd * x_scale_value;
    int new_ht = pimage_ht * y_scale_value;
    int big_dim = printer_resolution * MAX (printer_page_wd, printer_page_ht);
    int small_dim = printer_resolution * MIN (printer_page_wd, printer_page_ht);

/* if undo is not available and scaling up - low on memory don't try to scale
   up */
    if ((!undo_available) && ((x_scale_value * y_scale_value) > 1) && 
	(!suicidal))
	return (NO_MEM_FOR_SCALE_SERVER);
/* make sure scaled up/down area is not too large/small */
    if ((new_wd < MIN_PICTURE_WD) || (new_ht < MIN_PICTURE_HT))
	return (SCALE_TOO_SMALL);
    if ((new_wd > big_dim) || (new_ht > big_dim) ||
        ((new_wd > small_dim) && (new_ht > small_dim)))
	return (SCALE_TOO_BIG);
    else {
	Set_Cursor_Watch (pwindow);
	Set_Undo_Button (NO_ACTION);
	if (entering_text)
	    End_Text ();
	if (select_on) 
	    DeSelect (TRUE);
	Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y,
		      picture, ImgK_Src);
	if (picture_image->bitmap_bit_order != NATIVE_BIT_ORDER) /*ram*/
	    ConvertImageNative (picture_image);                  /*ram*/

	bits_per_pixel = (pdepth == 1) ? 1 : 8;
	pixel_order = ImgK_StandardPixelOrder; 

	bytes = picture_image->bytes_per_line;
	pixels_per_scanline = picture_image-> width;
	scanline_count = picture_image-> height;
	scanline_stride = 8 * bytes;	/* 8 bits per byte */
	image_data = picture_image-> data;
	image_size = bytes * scanline_count;

	start_set_itemlist(set_attributes, set_index);
	put_set_item(set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item(set_attributes, set_index, Img_BitsPerPixel, bits_per_pixel);
	put_set_item(set_attributes, set_index, Img_PixelsPerLine, pixels_per_scanline);
	put_set_item(set_attributes, set_index, Img_NumberOfLines, scanline_count);
	put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	end_set_itemlist(set_attributes, set_index);

	image_id = ImgCreateFrame(set_attributes, image_stype);

	ISL_ERROR_HANDLER_SETUP
	image_id = ImgImportBitmap(image_id, image_data, image_size, 0, 0, 0,
				     0); /* jj-port */
	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
	    return (NO_MEM_FOR_SCALE_CLIENT);	    
	}
	
	scale_x = x_scale_value; /* jj-port */
	scale_y = y_scale_value; /* jj-port */

	ISL_ERROR_HANDLER_SETUP
	scale_image_id = ImgScale (image_id, &scale_x, &scale_y, 0,
				    ImgM_NearestNeighbor, 0); /* jj-port */
	ImgDeleteFrame (image_id);
	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    return (NO_MEM_FOR_SCALE_CLIENT);	    
	}

	start_get_itemlist(get_attributes, get_index);
	put_get_item(get_attributes, get_index, Img_BitsPerPixel, bits_per_pixel);
	put_get_item(get_attributes, get_index, Img_PixelsPerLine, pixels_per_scanline);
	put_get_item(get_attributes, get_index, Img_NumberOfLines, scanline_count);
	put_get_item(get_attributes, get_index, Img_ScanlineStride, scanline_stride);
	end_get_itemlist(get_attributes, get_index);
	
	ImgGetFrameAttributes(scale_image_id, get_attributes); /* jj-port */
	
	off_modulo = scanline_stride % 32;
	if (off_modulo == 0)
	    display_frame_id = scale_image_id;
	else{
	    start_set_itemlist(set_attributes, set_index);
	    scanline_stride += 32 - off_modulo;
	    put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	    end_set_itemlist(set_attributes, set_index);

	    ISL_ERROR_HANDLER_SETUP
	    display_frame_id = ImgCopy(scale_image_id, 0, set_attributes); /* jj-port */
	    ImgDeleteFrame (scale_image_id);
	    if (failure_occurred) {
		ISL_RECOVER_FROM_ERROR
		return (NO_MEM_FOR_SCALE_CLIENT);
	    }
	}

	image_bytes = scanline_stride * scanline_count / 8;

	image_data = (char *) XtCalloc(image_bytes, sizeof(char)); /* jj-port */
	if (image_data == 0) {
	    ImgDeleteFrame (display_frame_id);
	    return (NO_MEM_FOR_SCALE_CLIENT);	    
	}

	ImgExportBitmap(display_frame_id, 0, image_data, image_bytes, 0, 0, 0,
			  0);

	ImgDeleteFrame (display_frame_id);

	if (undo_available) {
	    Clear_UG ();
	    undo_width = pimage_wd;
	    undo_height = pimage_ht;
	    UG_image[UG_last] = picture_image;
	    UG_used[0] = UG_last;
	    UG_num_used = 1;
	    undo_action = SCALE_PICTURE;
	    Set_Undo_Button (SCALE_PICTURE);
	}
	else
	{
#if 0
	    XtFree (picture_image->data);
#endif
	    XDestroyImage (picture_image);
	}
	
	picture_image = XCreateImage (disp, visual_info->visual,
				      (bits_per_pixel == 1) ? 1 : pdepth,
				      img_format, 0, image_data,
				      pixels_per_scanline, scanline_count,
				      32, 0 );
	if (picture_image == NULL) return (NO_MEM_FOR_SCALE_SERVER);

	picture_image->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
        picture_image->byte_order = NATIVE_BYTE_ORDER;         /*ram*/

	almost_new_file = TRUE;  /* it's as if we got a new file */
/* this will reset the cursor in the main window as well */
	Change_Picture_Size (picture_image->width, picture_image->height);
	if (exiting_paint)
            return (NO_MEM_FOR_SCALE_SERVER);

	return(SCALE_SUCCESS);
    }
}


void Scale_Size (w, button_id, reason)		/* user pushed the button */
    Widget   w;
    int	     *button_id;
    int	     reason;
{
    char *xstr, *ystr, scale_factor[4];
    XmString xm_scale_factor;
    float xvalue, yvalue;
    float xpercent, ypercent;
    int st;
    long bc, status;
    XmString xm_xstr,xm_ystr;
    

    switch (*button_id) {
	case SCALE_OK_ID :
	case SCALE_APPLY_ID :
	    Set_Cursor_Watch (XtWindow (scale_dialog));

	    xm_xstr = DXmCSTextGetString (widget_ids[SCALE_X_BY_TEXT]);
	    if (xm_xstr)
	    {
		xstr = (char *)DXmCvtCStoOS (xm_xstr, &bc, &status);
		XmStringFree (xm_xstr);
		xpercent = (float) atoi (xstr);
		XtFree (xstr);
	    }
	    else
	    {
		xpercent = 0;
	    }

	    xm_ystr = DXmCSTextGetString (widget_ids[SCALE_Y_BY_TEXT]);
	    if (xm_ystr)
	    {
		ystr = (char *)DXmCvtCStoOS (xm_ystr, &bc, &status);
		XmStringFree (xm_ystr);
		ypercent = (float) atoi (ystr);
		XtFree (ystr);
	    }
	    else
	    {
		ypercent = 0;
	    }

	    xvalue = xpercent / 100.0;
	    yvalue = ypercent / 100.0;

	    if ((xvalue == 1.0) && (yvalue == 1.0)) {
		XUndefineCursor (disp, XtWindow (scale_dialog));
		if (*button_id != SCALE_APPLY_ID)
		    XtUnmanageChild (scale_dialog);
		return;
	    }

	    switch (scale_id) {
		case EDIT_SCALE_ID :
		    st = Scale (xvalue, yvalue);
		    break;
		case EDIT_SCALE_PICTURE_ID :
		    st = Scale_Picture (xvalue, yvalue);
		    if (exiting_paint)
                        return;
		    break;
	    }

            XUndefineCursor (disp, XtWindow (scale_dialog));
	    switch (st) {
		case SCALE_SUCCESS :
		    if (scale_id == EDIT_SCALE_ID) {
			moved_only = FALSE;
			no_distortion = FALSE;
			deselected = FALSE;
			Clear_UG ();
		    }
		    if (*button_id != SCALE_APPLY_ID)
			XtUnmanageChild( scale_dialog );
		    break;
		case SCALE_TOO_SMALL :    
		    switch (scale_id) {
			case EDIT_SCALE_ID :
			    Display_Message( "T_SCALE_DOWN_ERROR" );
			    break;
			case EDIT_SCALE_PICTURE_ID :
			    Display_Message( "T_SCALE_DOWN_ERROR_P" );
			    break;
		    }
		    break;
		case SCALE_TOO_BIG :    
		    switch (scale_id) {
			case EDIT_SCALE_ID :
			    Display_Message( "T_SCALE_UP_ERROR" );
			    break;
			case EDIT_SCALE_PICTURE_ID :
			    Display_Message( "T_SCALE_UP_ERROR_P" );
			    break;
		    }
		    break;
		case NO_MEM_FOR_SCALE_SERVER :
		    if (*button_id != SCALE_APPLY_ID)
			XtUnmanageChild( scale_dialog );
		    Display_Message ("T_NO_SCALE");
		    break;
		case NO_MEM_FOR_SCALE_CLIENT :
		    if (*button_id != SCALE_APPLY_ID)
			XtUnmanageChild( scale_dialog );
		    Display_Message ("T_NO_MEM_FOR_SCALE");
		    break;
	    }
	    break;
	case SCALE_CANCEL_ID :
	    XtUnmanageChild (scale_dialog);
	    break;
	default :
	    switch (*button_id) {
		case SCALE_X_25_ID :
		case SCALE_Y_25_ID :
		    strcpy (scale_factor, "25");
		    break;
		case SCALE_X_50_ID :
		case SCALE_Y_50_ID :
		    strcpy (scale_factor, "50");
		    break;
		case SCALE_X_100_ID :
		case SCALE_Y_100_ID :
		    strcpy (scale_factor, "100");
		    break;
		case SCALE_X_200_ID :
		case SCALE_Y_200_ID :
		    strcpy (scale_factor, "200");
		    break;
		case SCALE_X_300_ID :
		case SCALE_Y_300_ID :
		    strcpy (scale_factor, "300");
		    break;
		case SCALE_X_400_ID :
		case SCALE_Y_400_ID :
		    strcpy (scale_factor, "400");
		    break;
	    }
	    xm_scale_factor = DXmCvtOStoCS (scale_factor, &bc, &status);
	    free (scale_factor);
	    if (*button_id > MAX_SCALE_X_ID)
		DXmCSTextSetString ((DXmCSTextWidget)widget_ids[SCALE_Y_BY_TEXT],
				    xm_scale_factor);
	    else
		DXmCSTextSetString ((DXmCSTextWidget)widget_ids[SCALE_X_BY_TEXT],
				    xm_scale_factor);
	    XmStringFree (xm_scale_factor);
	    break;
    }
}


void Create_Scale_Dialog (id)
    int id;
{
    int wd1, wd2, sp;
    int ht1, ht2, y = 0, yoff;

    Set_Cursor_Watch (pwindow);
    scale_id = id;
    if (!scale_dialog) {
        if (Fetch_Widget ("scale_dialog_box", main_widget,
		          &scale_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch scale dialog");

	XtManageChild (scale_dialog);

/* Center the labels with respect to the Text widgets */
	Get_Attribute (widget_ids[SCALE_FORM_1], XmNverticalSpacing, &y);
	ht1 = XtHeight (widget_ids[SCALE_X_BY_LABEL]);
	ht2 = XtHeight (widget_ids[SCALE_X_BY_TEXT]);
	yoff = y + ((ht2 - ht1) / 2);
	Set_Attribute (widget_ids[SCALE_X_BY_LABEL], XmNtopOffset, yoff);
	Set_Attribute (widget_ids[SCALE_Y_BY_LABEL], XmNtopOffset, yoff);

/* Center the push buttons */
	wd1 = XtWidth (widget_ids[SCALE_BUTTONS_ROW_COLUMN]);
	wd2 = XtWidth (widget_ids[SCALE_OK_BUTTON]);
	sp = (wd1 - (3 * wd2)) / 2;
	Set_Attribute (widget_ids[SCALE_BUTTONS_ROW_COLUMN], XmNspacing, sp);
    }
    else {
	XtManageChild (scale_dialog);
    }
    Set_Cursor (pwindow, current_action);
}

