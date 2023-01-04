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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/crop.c,v 1.1.2.2 92/12/11 08:33:58 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module CROP "V1-000"
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
**   This module contains routines that do image cropping
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

/*
 *
 * ROUTINE:  Crop
 *
 * ABSTRACT: 
 *
 * Crop the selected piece
 *
 */
void Crop()
{
    int i, x, y;
    int bits_per_pixel, bits_per_line, extra_bits, image_bytes;
    char *image_data;
    GC gc_solid, gc_mask;
    Arg args[5];
    int argcnt;

    if ((select_width < MIN_PICTURE_WD) || (select_height < MIN_PICTURE_HT)) {
	Display_Message ("T_INVALID_SIZE");
	return;
    }

/*
    if ((select_width > pimage_wd) || (select_height > picture_ht))
	Display_Message ("T_CROP_TOO_LARGE");
*/

/* if the whole thing was selected, a crop won't do anything.  Just return */
    if ((pimage_wd == select_width) && (pimage_ht == select_height))
	return;


/* create a new picture image */
/* make sure there is memory to allocate new image */
    bits_per_pixel = (pdepth == 1) ? 1 : 8;
    bits_per_line = pimage_wd * bits_per_pixel;
    if (extra_bits = bits_per_line % 32)
	bits_per_line += 32 - extra_bits;
    image_bytes = (bits_per_line / 8) * pimage_ht;
    image_data = (char *) XtCalloc (image_bytes, sizeof (char));

    if (image_data == 0) {
	Display_Message ("T_NO_MEM_FOR_CROP");
	return;
    }

    if (zoomed)
	Zoom_Off();

    Set_Cursor_Watch (pwindow);
    Stop_Highlight_Blink();

    Restore_Image (undo_move_map, PMX_TO_IX(select_x), PMY_TO_IY(select_y),
		   0, 0, select_width, select_height, Get_GC (GC_PD_COPY),
		   FALSE);

/* fill the undo move map with background (for copy select to image) */
    XSetForeground (disp, Get_GC (GC_PD_SOLID), picture_bg);
    XFillRectangle (disp, undo_move_map, Get_GC (GC_PD_SOLID), 0, 0,
		    select_width, select_height );

    Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y,
		  picture, ImgK_Src); 

/* Change picture width and height */
    prv_picture_wd = picture_wd;
    prv_picture_ht = picture_ht;
    picture_wd = MIN (select_width, max_picture_wd);
    picture_ht = MIN (select_height, max_picture_ht);
    prv_pic_xorigin = pic_xorigin;
    prv_pic_yorigin = pic_yorigin;
    pic_xorigin = 0;
    pic_yorigin = 0;

    prv_pimage_wd = pimage_wd;
    prv_pimage_ht = pimage_ht;
    pimage_wd = select_width;
    pimage_ht = select_height;    
    prv_picture_x = picture_x;  
    prv_picture_y = picture_y;
    picture_x = 0;
    picture_y = 0;

/* stuff */
    Clear_UG ();

    XFreePixmap (disp, picture);
    picture = 0;

    Create_Pixmaps (picture_bg, picture_wd, picture_ht);
    if (exiting_paint)
	return;

    UG_image [UG_last] = picture_image;
    UG_used[0] = UG_last;
    UG_num_used = 1;

/*
    undo_x = 0;
    undo_y = 0;
    undo_width = prv_pimage_wd;
    undo_height = prv_pimage_ht;
*/

/* fill the new image data with background pixels */
/* might only be good under VAX *???* */
    if (picture_bg_byte != 0) {
	memset (image_data, picture_bg_byte, image_bytes);
    }

    picture_image =
	XCreateImage (disp, visual_info->visual,
                      (bits_per_pixel == 1) ? 1 : pdepth,
		       img_format, 0, image_data,
                       pimage_wd, pimage_ht, 32, 0);
    picture_image->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
    picture_image->byte_order = NATIVE_BYTE_ORDER;         /*ram*/


    Find_Pwindow_Size ();

    Copy_Select_To_Image (0, 0, TRUE);
    XFreePixmap (disp, undo_move_map);
    undo_move_map = 0;

/* reset the border window */
/*
    XtSetArg (args[0], XmNwidth, picture_wd);
    XtSetArg (args[1], XmNheight, picture_ht);
    XtSetValues (widget_ids[BORDER_WINDOW], args, 2);
*/

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
    XtSetArg (args[argcnt], XmNvalue, 0);
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
    XtSetArg (args[argcnt], XmNvalue, 0);
    argcnt++;
    XtSetValues (widget_ids[PAINT_V_SCROLL_BAR], args, argcnt);

    Increase_Change_Pix (0, 0, picture_wd, picture_ht);

    Set_Undo_Button (CROP);

    Update_Pos_Window_Image_Size ();

    DeSelect (FALSE);
    Update_Pic_Shape_Dialog_Fields (FALSE);
}
