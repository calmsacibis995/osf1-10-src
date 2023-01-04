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
static char *BuildSystemHeader= "$Id: resize.c,v 1.1.2.3 92/12/11 08:35:54 devrcs Exp $";
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module RESIZE "V1-000"
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
**   RESIZE contains routines that handle resizing windows
**   The picture window can be resized directly by the user using
**   the X resize mechanism or indirectly by changing the picture
**   size.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**
**
**	dl	9/27/88
**      Reset pwindow_wd and pwindow_ht after changing picture size
**
**	dl	9/30/88
**	Reset slider page increments
**	Reset slider max values
**
**	jj	10/11/88
**	Change determination of pwindow_wd and pwindow_ht
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
extern double atof();
/* static Widget pic_shape_dialog; */

#define PIXELS 0
#define INCHES 1
#define CENTIMETERS 2
#define CM_PER_INCH 2.54
#define PIX_ROUNDOFF 0.5
#define INCH_ROUNDOFF 0.005
#define CM_ROUNDOFF 0.005
static int units = PIXELS, cur_size, cur_res, non_standard_res;
static int resize_crop_or_scale, resize_crop_or_scale_new;

void Update_Pic_Shape_Dialog_Fields (); /* declare this procedure */

int Find_Vslider_Space ()
{
    return (XtWidth (widget_ids[PAINT_V_SCROLL_BAR]) +
	    2 * XtBorderWidth (widget_ids[PAINT_V_SCROLL_BAR]));
}

int Find_Hslider_Space ()
{
    return (XtHeight (widget_ids[PAINT_H_SCROLL_BAR]) +
	    2 * XtBorderWidth (widget_ids[PAINT_H_SCROLL_BAR]));
}

/*
 * ROUTINE:  Find_Pwindow_Size
 *
 * ABSTRACT: 
 *
 *  Sets the variables for the scrollable window size of the picture
 *
 */
void Find_Pwindow_Size()
{
    Arg args[3];
    int wd, ht;

/* changed determination of pwindow_wd and pwindow_ht. They now are the width 
   and height of the drawable area of the scroll_window - jj 10/11/88 
   again - 5/24/90
*/

    if (paint_view == FULL_VIEW) {
	pwindow_wd = 
	    MIN (XtWidth(widget_ids[SCROLLING_WINDOW]), select_portion_wd);
	pwindow_ht = 
	    MIN (XtHeight(widget_ids[SCROLLING_WINDOW]), select_portion_ht);
    }
    else {
	pwindow_wd = MIN (XtWidth(widget_ids[SCROLLING_WINDOW]), pimage_wd);
	pwindow_ht = MIN (XtHeight(widget_ids[SCROLLING_WINDOW]), pimage_ht);
    }

/* Reset picture window size if necessary */
/* only necessary if size of picture has changed */

    if ((XtWidth(picture_widget) != pwindow_wd) ||
	(XtHeight(picture_widget) != pwindow_ht))
    {
	XtSetArg (args[0], XmNwidth, pwindow_wd);
	XtSetArg (args[1], XmNheight, pwindow_ht);
	XtSetValues (picture_widget, args, 2);
    }
}

/*
 *
 * ROUTINE:  Resize_Picture_Window
 *
 * ABSTRACT: 
 *
 *   This routine is called when the user resizes the main window
 */
void Resize_Picture_Window (new_wd, new_ht)
    int new_wd, new_ht;
{
    int dummy;
    int xvalue, yvalue;
    int new_picture_x, new_picture_y;
    int orig_x, orig_y;
    Arg args[5];
    int argcnt;

    if (paint_view == FULL_VIEW) {
	SP_Resize ();
	return;
    }

/* Set new picture window width and height */
	orig_x = xvalue = picture_x + pic_xorigin;
	orig_y = yvalue = picture_y + pic_yorigin;

	Find_Pwindow_Size ();


/* Get the picture origin */
	if ((xvalue + (int)pwindow_wd) > pimage_wd)
	    xvalue = pimage_wd - (int)pwindow_wd;
	if ((yvalue + (int)pwindow_ht) > pimage_ht)
	    yvalue = pimage_ht - (int)pwindow_ht;

/* see if new pixmap must be read from image */
	if ((xvalue < picture_x) || (yvalue < picture_y) ||
	    ((xvalue + (int)pwindow_wd) > (picture_x + (int)picture_wd)) ||
	    ((yvalue + (int)pwindow_ht) > (picture_y + (int)picture_ht))) {

	    Find_Pixmap_Position (xvalue, yvalue,
				  &new_picture_x, &new_picture_y);
	    Boundary_Move_Picture (new_picture_x, new_picture_y);
	    Position_Pixmap (new_picture_x, new_picture_y);
	}
	pic_xorigin = xvalue - picture_x;
	pic_yorigin = yvalue - picture_y;

	if ((num_hipts > 0) || (prv_num_hipts > 0)) {
	    xscroll = orig_x - xvalue;
	    yscroll = orig_y - yvalue;
	    Offset_Select_Rband();
	}	

	if (zoomed) {
	    Move_Or_Resize_Zoom_Maybe ();
/*
	    Refresh_Zoom_View (zoom_xorigin, zoom_yorigin, zoom_width,
			       zoom_height);
*/
	}

/* set these values simultaneously for each scroll bar */	
/* First the horizontal scroll bar */
	argcnt = 0;
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_wd);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_wd);
	argcnt++;
/* reset slider values dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNvalue, picture_x + pic_xorigin);
	argcnt++;
	XtSetValues (widget_ids[PAINT_H_SCROLL_BAR], args, argcnt);

/* Now the vertical scroll bar */
	argcnt = 0;
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_ht);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_ht);
	argcnt++;
/* reset slider values dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNvalue, picture_y + pic_yorigin);
	argcnt++;
	XtSetValues (widget_ids[PAINT_V_SCROLL_BAR], args, argcnt);

/*	Reset_PD_Clip_Rect (); */

	Update_Pos_Window_Window_Size ();
}

/*
 *
 * ROUTINE:  Change_Picture_Size
 *
 * ABSTRACT: 
 *
 * This routine is called when the user changes the picture size through 
 * reading a file
 */
void Change_Picture_Size( wd, ht )
    Dimension wd, ht;
{
    int ret_wd, ret_ht;
    Arg args[5];
    int argcnt;
    Pixmap old_picture;
    XImage *old_image;
    char *image_data;
    int p_wd, p_ht;
    int new_wd, new_ht;
    int old_pxo = PMX_TO_IX (pic_xorigin), old_pyo = PMY_TO_IY (pic_yorigin);
    int bits_per_line, extra_bits, image_bytes, bytes_per_line;
    int bits_per_pixel;

    bits_per_pixel = (pdepth == 1) ? 1 : 8; 

    Set_Cursor_Watch (pwindow);
    if ((wd != pimage_wd) || (ht != pimage_ht) || new_file || almost_new_file) {
	if (select_on)
	    DeSelect(TRUE);
	if (entering_text)
	    End_Text ();
/* If a new image has not been read, copy the old image to the new image */
	if (!(new_file || almost_new_file)) {
	    Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y,
			  picture, ImgK_Src);
	    old_image = picture_image;
	    bits_per_line = wd * bits_per_pixel;
/* pad to multiple of 32 bits */
	    if (extra_bits = bits_per_line % 32)
		bits_per_line = bits_per_line + 32 - extra_bits;
	    bytes_per_line = bits_per_line / 8;
	    image_bytes = bytes_per_line  * ht;
	    image_data = (char *) XtCalloc (image_bytes, sizeof(char)); /* jj-port */
	    if (image_data == 0) {
                if ((wd > pimage_wd) || (ht > pimage_ht)) {
                    Display_Message ("T_NO_MEM_FOR_CPS");
/* reset cursor before premature return */
                    Set_Cursor (pwindow, current_action);
                    return;
                }
            }
	    else {
		if (picture_bg_byte != 0) {
                    memset (image_data, picture_bg_byte, image_bytes);
                }
		picture_image = 
		    XCreateImage (disp, 
				  visual_info->visual,
				  (bits_per_pixel == 1) ? 1 : pdepth,
				  img_format, 0, image_data, wd,
				  ht, 32, 0);
		picture_image->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
		picture_image->byte_order = NATIVE_BYTE_ORDER;         /*ram*/
		Merge_Image (0, 0, MIN(wd, pimage_wd), MIN(ht, pimage_ht), 0, 0,
			     old_image, picture_image, ImgK_Src);
		if (undo_available)
		{
		    Clear_UG ();
		    undo_width = pimage_wd;
		    undo_height = pimage_ht;
		    UG_image[UG_last] = old_image;
		    UG_used[0] = UG_last;
		    UG_num_used = 1;
		    undo_action = CHANGE_PICTURE_SIZE;
		    Set_Undo_Button (CHANGE_PICTURE_SIZE);
		}
		else
		{
#if 0
		    XtFree (old_image->data);
#endif
		    XDestroyImage (old_image);
		    old_image = NULL;
		}
		picture_changed = TRUE;
		pixmap_changed = FALSE;
	    }
	}
	else {
	    if (new_file) {
		if (undo_available)
		    Clear_UG ();
		Set_Undo_Button (NO_ACTION);
		new_file = FALSE;
	    }
	    else {
		almost_new_file = FALSE;
	    }
	}

	pimage_wd = wd;
	pimage_ht = ht;
	
/* Reset the picture origin and picture size */
        p_wd = MIN (pimage_wd, max_picture_wd);
        p_ht = MIN (pimage_ht, max_picture_ht);
/* get new scroll window width/height jj-10/11/88 */
	Find_Pwindow_Size ();

/* change sensitivity of option menu select portion button if necessary */
	if ((pimage_wd > (int) pwindow_wd) || (pimage_ht > (int) pwindow_ht)) {
/*
	    XtSetSensitive (widget_ids[OPTIONS_SELECT_PORTION_BUTTON],
			   SENSITIVE);
*/
	}
  	else {
/*
	    XtSetSensitive (widget_ids[OPTIONS_SELECT_PORTION_BUTTON],
			    INSENSITIVE);
*/
	}
	
/* if the position of the pixmap extends past the right edge of the new image
   then reposition it at the right edge of the new image.  If the scroll window
   view was OK, reposition it in the new pixmap so it will not have moved in the
   image, else reposition it at the right edge of the new pixmap. */
        if (picture_x + p_wd > pimage_wd) 
	    picture_x = pimage_wd - p_wd;	    
	if (old_pxo + (int) pwindow_wd > pimage_wd)
	    pic_xorigin = IX_TO_PMX (pimage_wd - (int)pwindow_wd);
	else
	    pic_xorigin = old_pxo - picture_x;

/* if the position of the pixmap extends past the bottom of the new image then
   reposition it at the bottom of the new image.  If the scroll window view
   was OK, reposition it in the new pixmap so it will not have moved in the
   image else reposition it at the bottom of the new pixmap. */
        if (picture_y + p_ht > pimage_ht) 
	    picture_y = pimage_ht - p_ht;
	if (old_pyo + (int) pwindow_ht > pimage_ht)
	    pic_yorigin = IY_TO_PMY (pimage_ht - (int)pwindow_ht);
	else
	    pic_yorigin = old_pyo - picture_y;

/* Delete the old pixmaps and create new ones */
/* dl - 10/11/88 test to see if pixmaps exist before freeing */
/*	Clear_UG (); */
	if (!picture) {
	    Create_Pixmaps (picture_bg, p_wd, p_ht);
	}
	else {
	    if ((p_wd != picture_wd) || (p_ht != picture_ht)) {
		XFreePixmap (disp, picture);
		Create_Pixmaps (picture_bg, p_wd, p_ht);
	    }    
	}

/* if could not create picture pixmap, exit paint. */ 
	if (exiting_paint)
            return;
/*
	if (!picture) {
	    printf ("Out of pixmap memory.  Exiting paint");
	    Exit_Paint (0);
	}
*/

	if (( p_wd != picture_wd ) || ( p_ht != picture_ht )) {    
	    picture_wd = p_wd;
	    picture_ht = p_ht;
	}

/*
	argcnt = 0;
	XtSetArg (args[argcnt], XmNwidth, picture_wd);
	argcnt++;
	XtSetArg (args[argcnt], XmNheight, picture_ht);
	argcnt++;
	XtSetValues (widget_ids[BORDER_WINDOW], args, argcnt);
*/

/* Copy the correct portion of the image to the new picture */
	MY_XPutImage (disp, picture, Get_GC(GC_PD_COPY), picture_image, picture_x,
		   picture_y, 0, 0, picture_wd, picture_ht);

/* set these values simultaneously for each scroll bar */	
/* First the horizontal scroll bar */
	argcnt = 0;
/* reset slider max values */
	XtSetArg (args[argcnt], XmNmaximum, pimage_wd);
	argcnt++;
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_wd);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_wd);
	argcnt++;
/* reset slider values dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNvalue, picture_x + pic_xorigin);
	argcnt++;
	XtSetValues (widget_ids[PAINT_H_SCROLL_BAR], args, argcnt);

/* Now the vertical scroll bar */
	argcnt = 0;
/* reset slider max values */
	XtSetArg (args[argcnt], XmNmaximum, pimage_ht);
	argcnt++;
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_ht);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_ht);
	argcnt++;
/* reset slider values dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNvalue, picture_y + pic_yorigin);
	argcnt++;
	XtSetValues (widget_ids[PAINT_V_SCROLL_BAR], args, argcnt);

/*	Reset_PD_Clip_Rect (); */
/* Re-initialize zoom window if necessary */
	if( zoomed ){
	    End_Zoom ();
	    Begin_Zoom( pic_xorigin + MIN ((int) pwindow_wd, pimage_wd) / 2,
		        pic_yorigin + MIN ((int) pwindow_ht, pimage_ht) / 2 ); 
	}

	Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);

	Update_Pos_Window_Image_Size ();
	Update_Pic_Shape_Dialog_Fields (FALSE);
    }
    Set_Cursor (pwindow, current_action);
}


/*
 * Convert the given width and height into pixel width/heigth.
 */
void Convert_To_Pixels( wd, ht, new_wd, new_ht )
float wd, ht;
int *new_wd, *new_ht;
{

/* translate wd & ht in prev units into pixels */
/* round off to nearest pixel */
/* maintain correct picture size -jj 10/14/88 */
    switch (units) {
	case PIXELS :
	    *new_wd = (int) (wd + PIX_ROUNDOFF);
	    *new_ht = (int) (ht + PIX_ROUNDOFF);
	    break;
	case INCHES :
	    *new_wd = (int) ((wd * (float) cur_res) + PIX_ROUNDOFF);
	    *new_ht = (int) ((ht * (float) cur_res) + PIX_ROUNDOFF);
	    break;
	case CENTIMETERS :
	    *new_wd = (int) ((wd * (float) cur_res)/CM_PER_INCH + PIX_ROUNDOFF);
	    *new_ht = (int) ((ht * (float) cur_res)/CM_PER_INCH + PIX_ROUNDOFF);
	    break;
    }
}


/*
 * Width and height are in PIXELS
 */
void Fill_In_Dimension( wd, ht )
    int wd, ht;
{
    char *ret_wd_str, *ret_ht_str;
    char wd_str[80], ht_str[80];
    char *wd_ptr, *ht_ptr;
    int decpt, sign;
    float new_wd, new_ht;
    int i,j;
    long bc, status;
    XmString wd_xmstr, ht_xmstr;
    int precision;
    float roundoff;

    switch (units) {
	case INCHES :
	    if (cur_res <= 100) {
		roundoff = 0.005;
		precision = 2;
	    }
	    else {
		if (cur_res <= 1000) {
		    roundoff = 0.0005;
		    precision = 3;
		}
		else {
		    roundoff = 0.00005;
		    precision = 4;
		}
	    }
	    new_wd = (float)wd/(float)cur_res + roundoff;
	    new_ht = (float)ht/(float)cur_res + roundoff;
	    break;
	case CENTIMETERS :
	    if (cur_res <= 254) {
		roundoff = 0.005;
		precision = 2;
	    }
	    else {
		if (cur_res <= 2540) {
		    roundoff = 0.0005;
		    precision = 3;
		}
		else {
		    roundoff = 0.00005;
		    precision = 4;
		}
	    }
	    new_wd = (float)wd/((float)cur_res/CM_PER_INCH) + roundoff;
	    new_ht = (float)ht/((float)cur_res/CM_PER_INCH) + roundoff;
	    break;
	case PIXELS :
	    new_wd = wd;
	    new_ht = ht;
	    break;
    }
/* Format the wd & ht in ascii */
    ret_wd_str = (char *) ecvt( (double)new_wd, 80, &decpt, &sign ); /* jj-port */
    wd_ptr = wd_str;
    for( i = 0; i <decpt; ++i )
	*wd_ptr++ = *ret_wd_str++;
    if( units != PIXELS ){
	*wd_ptr++ = '.';
/* add leading zeros after decimal point */
	for (i = 0, j = precision; i > decpt && j > 0; --i, --j)
	    *wd_ptr++ = '0';
	for( i = 0; i < j; ++i )
	    *wd_ptr++ = *ret_wd_str++;
    }
    *wd_ptr++ = '\0';

    ret_ht_str = (char *) ecvt( (double)new_ht, 10, &decpt, &sign ); /* jj-port */
    ht_ptr = ht_str;
    for( i = 0; i <decpt; ++i )
	*ht_ptr++ = *ret_ht_str++;
    if( units != PIXELS ){
	*ht_ptr++ = '.';
/* add leading zeros after decimal point */
	for (i = 0, j = precision; i > decpt && j > 0; --i, --j)
	    *ht_ptr++ = '0';
	for( i = 0; i < j; ++i )
	    *ht_ptr++ = *ret_ht_str++;
    }
    *ht_ptr++ = '\0';

    wd_xmstr = DXmCvtOStoCS (wd_str, &bc, &status);
    ht_xmstr = DXmCvtOStoCS (ht_str, &bc, &status);
    DXmCSTextSetString ((DXmCSTextWidget)widget_ids[PIC_SHAPE_WIDTH_TEXT], wd_xmstr );
    DXmCSTextSetString ((DXmCSTextWidget)widget_ids[PIC_SHAPE_HEIGHT_TEXT], ht_xmstr );
    XmStringFree (wd_xmstr);
    XmStringFree (ht_xmstr);
}


void Fill_In_Resolution (res)
    int res;
{
    char *ret_res_str;
    char res_str[10];
    char *res_ptr;
    int decpt, sign;
    int i;
    long bc, status;
    XmString res_xmstr;

    ret_res_str = (char *) ecvt( (double)res, 10, &decpt, &sign ); /* jj-port */
    res_ptr = res_str;
    for( i = 0; i <decpt; ++i )
	*res_ptr++ = *ret_res_str++;
    *res_ptr++ = '\0';

    res_xmstr = DXmCvtOStoCS (res_str, &bc, &status);
    DXmCSTextSetString ((DXmCSTextWidget)widget_ids[RESOLUTION_TEXT], res_xmstr);
    XmStringFree (res_xmstr);
}


int Update_Resolution ()
{
    int res;
    char *res_str;
    XmString res_xmstr;
    long bc, status;

    res_xmstr = DXmCSTextGetString (widget_ids[RESOLUTION_TEXT]);
    if (res_xmstr)
    {
	res_str = (char *)DXmCvtCStoOS (res_xmstr, &bc, &status);
	XmStringFree (res_xmstr);
	res = atof (res_str);
	XtFree (res_str);
    }
    else
    {
	res = 0;
    }

    if (res >= 1) {
	cur_res = res;
	return (K_SUCCESS);
    }
    else {
	cur_res = resolution;
	return (K_FAILURE);
    }
}


void Update_Pic_Shape_Dialog_Fields (update)
    int update;
{
    if (pic_shape_dialog == 0)
	return;
    if ((!XtIsManaged (pic_shape_dialog)) && (!update))
	return;
    cur_res = resolution;
    switch (cur_res) {
	case DPI75 :
	    Set_Attribute (widget_ids[RESOLUTION_OPTION_MENU],
			   XmNmenuHistory, widget_ids[RES_75_TOGGLE]);
	    XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
	    non_standard_res = FALSE;
	    break;
	case DPI100 :
	    Set_Attribute (widget_ids[RESOLUTION_OPTION_MENU],
			   XmNmenuHistory, widget_ids[RES_100_TOGGLE]);
	    XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
	    non_standard_res = FALSE;
	    break;
	case DPI300 :
	    Set_Attribute (widget_ids[RESOLUTION_OPTION_MENU],
			   XmNmenuHistory, widget_ids[RES_300_TOGGLE]);
	    XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
	    non_standard_res = FALSE;
	    break;
	default :
	    Set_Attribute (widget_ids[RESOLUTION_OPTION_MENU],
			   XmNmenuHistory,
			   widget_ids[RES_NON_STANDARD_TOGGLE]);
	    XtSetSensitive (widget_ids[RESOLUTION_TEXT], SENSITIVE);
	    non_standard_res = TRUE;
	    break;
    }
    Fill_In_Resolution (cur_res);
/* set current value of picture size option menu */
    if ((pimage_wd == (8 * cur_res)) && (pimage_ht == (10 * cur_res))) {
	Set_Attribute (widget_ids[PIC_SHAPE_OPTION_MENU], XmNmenuHistory,
		       widget_ids[PAGE_SIZE_TOGGLE]);
	XtSetSensitive (widget_ids[PIC_SHAPE_HEIGHT_TEXT], INSENSITIVE);
	XtSetSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT], INSENSITIVE);
	cur_size = PAGE_ID;
    }
    else {
	if ((pimage_wd == DisplayWidth (disp, screen)) && 
	    (pimage_ht == DisplayHeight (disp, screen))) {
	    Set_Attribute (widget_ids[PIC_SHAPE_OPTION_MENU],
			   XmNmenuHistory, widget_ids[SCREEN_SIZE_TOGGLE]);
	    XtSetSensitive (widget_ids[PIC_SHAPE_HEIGHT_TEXT], INSENSITIVE);
	    XtSetSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT], INSENSITIVE);
	    cur_size = SCREEN_ID;
	}
	else {
	    Set_Attribute (widget_ids[PIC_SHAPE_OPTION_MENU],
			   XmNmenuHistory,
			   widget_ids[NON_STANDARD_SIZE_TOGGLE]);
	    XtSetSensitive (widget_ids[PIC_SHAPE_HEIGHT_TEXT], SENSITIVE);
	    XtSetSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT], SENSITIVE);
	    cur_size = NON_STANDARD_ID;
	}
    }
    Fill_In_Dimension( pimage_wd, pimage_ht );
}


void Create_Picture_Shape_Dialog()
{
    Widget w;
    Widget ht_label, wd_label;
    Arg args[5];
    int argcnt;                                      
    Widget submenu;
    int ht1, ht2, y = 0, yoff;
    int wd1, wd2, sp;

    Set_Cursor_Watch (pwindow);
    if (!pic_shape_dialog) {
        if (Fetch_Widget ("pic_shape_dialog_box", main_widget,
		          &pic_shape_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch pic shape dialog");

/* add tab groups */
	XmAddTabGroup (widget_ids[PS_SVC_RADIO_BOX]);
	XmAddTabGroup (widget_ids[PIC_SHAPE_OPTION_MENU]);
	XmAddTabGroup (widget_ids[PIC_SHAPE_WIDTH_TEXT]);
	XmAddTabGroup (widget_ids[PIC_SHAPE_HEIGHT_TEXT]);
	XmAddTabGroup (widget_ids[PS_UNITS_RADIO_BOX]);
	XmAddTabGroup (widget_ids[RESOLUTION_OPTION_MENU]);
	XmAddTabGroup (widget_ids[RESOLUTION_TEXT]);
	XmAddTabGroup (widget_ids[PS_BUTTONS_ROW_COLUMN]);

	XtManageChild (pic_shape_dialog);

/* Center the labels with respect to the text widgets */
	Get_Attribute (widget_ids[PS_FORM_1], XmNverticalSpacing, &y);
	ht1 = XtHeight (widget_ids[PS_WIDTH_LABEL]);
	ht2 = XtHeight (widget_ids[PIC_SHAPE_WIDTH_TEXT]);
	yoff = y + ((ht2 - ht1) / 2);
	Set_Attribute (widget_ids[PS_WIDTH_LABEL], XmNtopOffset, yoff);
	Set_Attribute (widget_ids[PS_RESOLUTION_LABEL], XmNtopOffset, yoff);
	yoff = y + (ht2 - ht1);
	Set_Attribute (widget_ids[PS_HEIGHT_LABEL], XmNtopOffset, yoff);

/* center the push buttons */
        wd1 = XtWidth (widget_ids[PS_BUTTONS_ROW_COLUMN]);
        wd2 = XtWidth (widget_ids[PS_OK_BUTTON]);
        sp = (wd1 - (3 * wd2)) / 2;
        Set_Attribute (widget_ids[PS_BUTTONS_ROW_COLUMN], XmNspacing, sp);
    }

/* set current value of resolution option menu */
    Update_Pic_Shape_Dialog_Fields (TRUE);
    if (!XtIsManaged (pic_shape_dialog)) {
	resize_crop_or_scale_new = resize_crop_or_scale;
	XtManageChild( pic_shape_dialog );
    }
    Pop_Widget (pic_shape_dialog);
    Set_Cursor (pwindow, current_action);
}


void Set_Picture_Size(w, button_id, reason)	/* user pushed the button */
    Widget   w;
    int	     *button_id;
    int	     reason;
{
    float wd, ht;
    int new_wd, new_ht;
    char *wd_str, *ht_str, *res_str;
    XmString wd_xmstr, ht_xmstr, res_xmstr;
    long bc, stat;
    int big_dim = printer_resolution * MAX (printer_page_wd, printer_page_ht);
    int small_dim = printer_resolution * MIN (printer_page_wd, printer_page_ht);
    int status;

  if (Finished_Action()) {
    switch (*button_id) {    	    
	case PIC_SHAPE_OK_ID : 
	case PIC_SHAPE_APPLY_ID : 
	    status = Update_Resolution ();
	    if ((cur_res < MIN_RESOLUTION) || (status != K_SUCCESS))
		Display_Message ("T_INVALID_RESOLUTION");
	    else {
	        if (cur_size == NON_STANDARD_ID) {
		    wd_xmstr =
			DXmCSTextGetString (widget_ids[PIC_SHAPE_WIDTH_TEXT]);
		    if (wd_xmstr)
		    {
			wd_str = (char *)DXmCvtCStoOS (wd_xmstr, &bc, &stat);
			XmStringFree (wd_xmstr);
			wd = atof (wd_str);
			XtFree(wd_str);
		    }
		    else {
			wd = 0;
		    }
		    ht_xmstr =
			DXmCSTextGetString (widget_ids[PIC_SHAPE_HEIGHT_TEXT]);
		    if (ht_xmstr)
		    {
			ht_str = (char *)DXmCvtCStoOS (ht_xmstr, &bc, &stat);
			XmStringFree (ht_xmstr);
			ht = atof (ht_str);
			XtFree (ht_str);
		    }
		    else {
			ht = 0;
		    }
/* maintain correct picture size -jj 10/14/88 */
/* convert into pixels */
		    Convert_To_Pixels (wd, ht, &new_wd, &new_ht);
		}
		else
		    if (cur_size == PAGE_ID) {
			new_wd = 8 * cur_res;
			new_ht = 10 * cur_res;
		    }
		    else {			     /* cur_size = SCREEN_ID */
			new_wd = DisplayWidth(disp, screen);
			new_ht = DisplayHeight(disp, screen);
		    }


/* check the new size */
		if ((new_wd < MIN_PICTURE_WD) || (new_ht < MIN_PICTURE_HT)
		    || (new_wd > big_dim) || (new_ht > big_dim)
		    || ((new_wd > small_dim) && (new_ht > small_dim))) 
		    Display_Message( "T_INVALID_SIZE" );
		else{
		    Exit_Select_Portion ();
		    if (exiting_paint)
			return;
		    Set_Cursor_Watch (XtWindow (pic_shape_dialog));
		    if (resolution != cur_res) {
			picture_changed = TRUE;
			resolution = cur_res;
		    }
		    resize_crop_or_scale = resize_crop_or_scale_new;
		    if ((resize_crop_or_scale == RESIZE_CROP_ID) ||
			((new_wd == pimage_wd) && (new_ht == pimage_ht))) {
			Change_Picture_Size (new_wd, new_ht);
			if (exiting_paint)
			    return;
		    }
		    else {
			status = 
			    Scale_Picture ((float)new_wd / (float)pimage_wd,
				           (float)new_ht / (float)pimage_ht);
			if (exiting_paint)
			    return;
			if (status) {
			    Display_Message ("T_SCALE_FAILED");
			}
		    }
		    XUndefineCursor (disp, XtWindow (pic_shape_dialog));
		    if (*button_id != PIC_SHAPE_APPLY_ID)
			XtUnmanageChild (pic_shape_dialog);
		}
	    }
	    break;
	case PIC_SHAPE_CANCEL_ID :
	    if (resize_crop_or_scale_new != resize_crop_or_scale) {
		if (resize_crop_or_scale == RESIZE_CROP_ID) {
		    Set_Attribute (widget_ids[PS_RESIZE_CROP_TOGGLE], XmNset,
				   TRUE);
		    Set_Attribute (widget_ids[PS_RESIZE_SCALE_TOGGLE], XmNset,
				   FALSE);
		}
		else {
		    Set_Attribute (widget_ids[PS_RESIZE_CROP_TOGGLE], XmNset,
				   FALSE);
		    Set_Attribute (widget_ids[PS_RESIZE_SCALE_TOGGLE], XmNset,
				   TRUE);
		}
	    }
	    XtUnmanageChild( pic_shape_dialog );
	    break;
    }
  }
}

void Set_Standard_Size(w, toggle_id, r )
    Widget   w;
    int      *toggle_id;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
int wd, ht;
char *wd_str, *ht_str;

    if( r->set ){
	Update_Resolution ();
	switch (*toggle_id) {
	    case PAGE_ID :
		wd = 8*cur_res;
		ht = 10*cur_res;
/* set width and height field insensitive */
		XtSetSensitive (widget_ids[PIC_SHAPE_HEIGHT_TEXT], INSENSITIVE);
		XtSetSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT], INSENSITIVE);
		cur_size = PAGE_ID;
		break;
	    case SCREEN_ID :
		wd = DisplayWidth(disp, screen);
		ht = DisplayHeight(disp, screen);
/* set width and height field insensitive */
		XtSetSensitive (widget_ids[PIC_SHAPE_HEIGHT_TEXT], INSENSITIVE);
		XtSetSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT], INSENSITIVE);
		cur_size = SCREEN_ID;
		break;
	    case NON_STANDARD_ID :
/* set width and height field sensetive */
		XtSetSensitive (widget_ids[PIC_SHAPE_HEIGHT_TEXT], SENSITIVE);
		XtSetSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT], SENSITIVE);
		cur_size = NON_STANDARD_ID;
		break;
	}
	if (*toggle_id != NON_STANDARD_ID)
	    Fill_In_Dimension (wd, ht);
    }
}


void Change_Resolution (w, tag, r)
    Widget w;
    caddr_t tag;
    XmAnyCallbackStruct *r;
{
    int wd, ht, status;

    status = Update_Resolution ();
    if (status != K_SUCCESS)
	return;
    if ((cur_size == PAGE_ID) && (units == PIXELS)) {
	wd = 8.0 * cur_res;
	ht = 10.0 * cur_res;
	Fill_In_Dimension (wd, ht);
    }
    if ((cur_size == SCREEN_ID) && (units != PIXELS))
	Fill_In_Dimension (screen_wd, screen_ht);
}


void Set_Resolution (w, toggle_id, r )
    Widget   w;
    int      *toggle_id;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    int res;
    char *res_str;
    int wd, ht;

    if( r->set ){
	switch (*toggle_id) {
	    case RES_75_ID :
		res = DPI75;
		XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
		non_standard_res = FALSE;
		break;
	    case RES_100_ID :
		res = DPI100;
		XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
		non_standard_res = FALSE;
		break;
	    case RES_300_ID :
		res = DPI300;
		XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
		non_standard_res = FALSE;
		break;
	    case RES_NON_STANDARD_ID :
/*		res = 0; */
		XtSetSensitive (widget_ids[RESOLUTION_TEXT], SENSITIVE);
		non_standard_res = TRUE;
		break;
	}
	if (*toggle_id != RES_NON_STANDARD_ID) {
	    Fill_In_Resolution (res);
	    if (res != 0)
		cur_res = res;
	    else
		cur_res = resolution;
	    if ((cur_size == PAGE_ID) && (units == PIXELS)) {
		wd = 8.0 * cur_res;
		ht = 10.0 * cur_res;
		Fill_In_Dimension (wd, ht);
	    }
	    if ((cur_size == SCREEN_ID) && (units != PIXELS))
		Fill_In_Dimension (screen_wd, screen_ht);
	}
    }
}	
	


void Change_Units( w, toggle_id, r )
    Widget w;					/* pulldown menu */
    int    *toggle_id;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    float wd, ht;
    int new_wd, new_ht, new_units;
    char *wd_str, *ht_str;
    XmString wd_xmstr, ht_xmstr;
    long bc, status;

    if( r->set ){
	Update_Resolution ();
	switch (*toggle_id) {
	    case PIXEL_ID :
		new_units = PIXELS;
/*
		XtSetSensitive (widget_ids[RESOLUTION_OPTION_MENU],
			        INSENSITIVE);
		if (non_standard_res)
		    XtSetSensitive (widget_ids[RESOLUTION_TEXT], INSENSITIVE);
*/
		break;
	    case INCH_ID :
		new_units = INCHES;
/*
		XtSetSensitive (widget_ids[RESOLUTION_OPTION_MENU], SENSITIVE);
		if (non_standard_res)
		    XtSetSensitive (widget_ids[RESOLUTION_TEXT], SENSITIVE);
*/
		break;
	    case CENTIMETER_ID :
	    	new_units = CENTIMETERS;
/*
		XtSetSensitive (widget_ids[RESOLUTION_OPTION_MENU], SENSITIVE);
		if (non_standard_res)
		    XtSetSensitive (widget_ids[RESOLUTION_TEXT], SENSITIVE);
*/
		break;
	}
	if (new_units != units) {
	    if (XtSensitive (widget_ids[PIC_SHAPE_WIDTH_TEXT])) {
		wd_xmstr =
		    DXmCSTextGetString (widget_ids[PIC_SHAPE_WIDTH_TEXT]);
		if (wd_xmstr)
		{
		    wd_str = (char *)DXmCvtCStoOS (wd_xmstr, &bc, &status);
		    XmStringFree (wd_xmstr);
		    wd = atof (wd_str);
		    XtFree (wd_str);
		}
		else {
		    wd = 0;
		}
		ht_xmstr =
		    DXmCSTextGetString (widget_ids[PIC_SHAPE_HEIGHT_TEXT]);
		if (ht_xmstr)
		{
		    ht_str = (char *)DXmCvtCStoOS (ht_xmstr, &bc, &status);
		    XmStringFree (ht_xmstr);
		    ht = atof (ht_str);
		    XtFree( ht_str );
		}
		else {
		    ht = 0;
		}
	        Convert_To_Pixels (wd, ht, &new_wd, &new_ht);
	    }
	    else
		if (cur_size == PAGE_ID) {
		    new_wd = 8*cur_res;
		    new_ht = 10*cur_res;
		}
		else {				     /* cur_size = SCREEN_ID */
		    new_wd = DisplayWidth(disp, screen);
		    new_ht = DisplayHeight(disp, screen);
		}
	    units = new_units;
	    Fill_In_Dimension( new_wd, new_ht );
	}
    }
}


void Change_Resize_Crop_Or_Scale (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->reason == XmCR_VALUE_CHANGED)
        if (r->set)
            resize_crop_or_scale_new = *button_id;
}
