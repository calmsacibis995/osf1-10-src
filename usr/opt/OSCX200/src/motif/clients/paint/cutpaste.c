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
 * Change image_id in Copy_To_Clipboard() to long
 */
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: cutpaste.c,v 1.1.2.3 92/12/11 08:34:03 devrcs Exp $";
#endif		/* BuildSystemHeader */
/* jj-port
  #ifndef ULTRIX
  #module CUTPASTE "V1-000"
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
**   This module handles cutting, copying and pasting to the clipboard
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      11/18/88
**      pad image when cutting and pasting to the clipboard
**
**--
**/           
#include "paintrefs.h"
#include <Xm/CutPasteP.h>

extern Force_Return_On_Failure();

/*
 *
 * ROUTINE:  Clipboard_Copy_To_Callback
 *
 * ABSTRACT: 
 *                                
 *   ISL action routine to copy DDIF stream piece by piece to the clipboard.
 *
 */
long Clipboard_Copy_To_Callback (bufptr, buflen, usrparam)
    char *bufptr;
    unsigned long buflen;
    int usrparam;
{
    int st;
    int dummy1;
    int dummy2;

    st = XmClipboardCopy (disp, pwindow, item_id, "DDIF", 
			  bufptr, buflen, dummy1, &dummy2);
    if (st == ClipboardSuccess)
	return (1);
    else 
	return (0);
}


/*
 *
 * ROUTINE:  Clipboard_Copy_From_Callback
 *
 * ABSTRACT: 
 *                                
 *   ISL action routine to copy DDIF stream piece by piece from the clipboard.
 *
 */
long Clipboard_Copy_From_Callback (bufptr, buflen, bytcnt, usrparam)
    char *bufptr;
    unsigned long buflen;
    unsigned long *bytcnt;
    int usrparam;
{
    int st;
    int pid;

    st = XmClipboardRetrieve (disp, pwindow, "DDIF", bufptr, buflen,
			       bytcnt, &pid);
    if ((st == ClipboardSuccess) || (st == ClipboardTruncate))
	return (1);
    else 
	return (0);
}


/*
 *
 * ROUTINE:  Copy_To_Clipboard
 *
 * ABSTRACT: 
 *                                
 *   put the selected area on the clipboard
 *
 */
int Copy_To_Clipboard (time)
    Time time;
{
    unsigned char* pix;
    int save_file_color;
    long image_id;
    int st;
    int status;
    XImage *ximage;
    GC gc_copy;
    char title[80];
    int i, j;
    int bits_per_pixel;
    unsigned long context;
    XmString tmp_xmstr;
    long bc, stat;

    if (Get_UIL_Value (title, "T_DECPAINT") != SUCCESS)
	DRM_Error ("Could not fetch value from UID file");

    tmp_xmstr = DXmCvtOStoCS ("title", &bc, &stat);
    st = XmClipboardStartCopy (disp, pwindow, tmp_xmstr,
			       time, NULL, NULL, &item_id);
    XmStringFree (tmp_xmstr);

    if (st != ClipboardSuccess) {
	switch (st) {
	    case ClipboardLocked :
		return (K_CLIPBOARD_LOCKED);
		break;
	    default :
		return (K_FAILURE);
		break;
	}
    }
/* Get the image of the copymap */

/* if select area then turn the 0's outside the select into picture_bg */
    if (!select_rectangle) {
	gc_copy = Get_GC (GC_PD_FUNCTION);
	XSetFunction (disp, gc_copy, GXxor);
	XSetForeground (disp, gc_copy, picture_bg);
	XFillPolygon (disp, copymap, gc_copy, orig_hi_points,
		      orig_num_hipts, Complex, CoordModeOrigin);
	XFillRectangle (disp, copymap, gc_copy, 0, 0, select_width, 
			select_height);
    }

    ximage = XGetImage (disp, copymap, 0, 0, select_width, select_height, 
			bit_plane, img_format);

/* return copymap to its normal form */
    if (!select_rectangle) {
	XFillRectangle (disp, copymap, gc_copy, 0, 0, select_width,
                        select_height);
	XFillPolygon (disp, copymap, gc_copy, orig_hi_points,
		      orig_num_hipts, Complex, CoordModeOrigin);
	XSetForeground (disp, gc_copy, picture_fg);
    }

    save_file_color = file_color;

/* pick file color associated with display - BW, GRAYSCALE, COLOR */
    switch (visual_info->class) {
        case StaticGray :
            file_color = SAVE_BW;
            break;
        case GrayScale :
            file_color = SAVE_GRAY;
            break;
        case StaticColor :
        case PseudoColor :
        case TrueColor :
        case DirectColor :
            file_color = SAVE_COLOR;
            break;
    }

    bits_per_pixel = (pdepth == 1) ? 1 : 8;
/* if non-bitonal image, check to see if it's all black and white anyway */
    if (bits_per_pixel != 1) {
	for (i = 0; i < select_height; i++) {
/* start pix to point to first pixel in scanline */
	    pix = (unsigned char *) ximage->data + i * ximage->bytes_per_line;
	    for (j = 0; j < select_width; j++) {
		if ((*pix != colormap[BLACK].pixel) &&
		    (*pix != colormap[WHITE].pixel)) {
		    break;
		}
		pix++;
	    }
	    if (j < select_width) {
		break;
	    }
	}
	if ((i == select_height) && (j == select_width)) {
	    file_color = SAVE_BW;
	}
    }

    status = Write_DDIF ((XImage *)ximage, (long *)&image_id);

    if (status != K_SUCCESS) {
	XmClipboardCancelCopy (disp, pwindow, item_id);
	return (status);
    }

    context = ImgCreateDDIFStream (ImgK_ModeExport, 0, 0, 0,
				      Clipboard_Copy_To_Callback, 0);

    ImgExportDDIFFrame (image_id, NULL, context, 0, 0, 0, 0, 0);

    ImgDeleteDDIFStream (context);

    st = XmClipboardEndCopy (disp, pwindow, item_id);
    st = XmClipboardUnlock (disp, pwindow, TRUE);
    XDestroyImage (ximage);
    ImgDeleteFrame (image_id);
/* reinstate the original file color */
    file_color = save_file_color;

    return (K_SUCCESS);
}

/*
 *
 * ROUTINE:  Cut()
 *
 * ABSTRACT: 
 *                                
 *   put the selected area on the clipboard
 *
 */
void Cut (time)
    Time time;
{
    int status;

    Set_Cursor_Watch (pwindow);
    status = Copy_To_Clipboard (time);
    switch (status) {
	case K_SUCCESS :
	    Stop_Highlight_Blink ();
	    Restore_Image (undo_move_map, PMX_TO_IX(select_x),
			   PMY_TO_IY(select_y), 0, 0, select_width,
			   select_height, Get_GC(GC_PD_COPY), TRUE);
	    undo_action = CUT;
	    DeSelect (TRUE);
	    break;
	case K_CLIPBOARD_LOCKED :
	    Display_Message ("T_COPY_CLIPBOARD_LOCKED");
	    break;
	default :
	    Display_Message ("T_NO_COPY_TO_CLIPBOARD");
	    break;
    }
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Copy()
 *
 * ABSTRACT: 
 *                                
 *   copy the selected area to the clipboard
 *
 */
void Copy (time)
    Time time;
{
    int status;
	
    Set_Cursor_Watch (pwindow);
    if (undo_action != COPY) {
	prv_undo_action = undo_action;
	Save_Toggle_State ();
    }
    status = Copy_To_Clipboard (time);
    switch (status) {
	case K_SUCCESS :
	    undo_action = COPY;
	    Set_Undo_Button (COPY);
	    break;
	case K_CLIPBOARD_LOCKED :
	    Display_Message ("T_COPY_CLIPBOARD_LOCKED");
	    break;
	default :
	    Display_Message ("T_NO_COPY_TO_CLIPBOARD");
	    break;
    }
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Set_Select
 *
 * ABSTRACT: 
 *                                
 *   Set up Select area from paste
 *
 */
void Set_Select(wd, ht)
{
	int i;

	select_window = pwindow;

/* Calculate select_x, select_y - should be centered in picture */
	orig_select_wd = select_width = wd;
	orig_select_ht = select_height = ht;
	if (picture_wd < pwindow_wd)
		select_x = picture_wd/2 - select_width/2;
	else
		select_x = PWX_TO_IX(pwindow_wd/2 - select_width/2);

	if (picture_ht < pwindow_ht)
		select_y = picture_ht/2 - select_height/2;
	else
		select_y = PWY_TO_IY(pwindow_ht/2 - select_height/2);

	prv_select_x = orig_select_x = PMX_TO_IX (select_x);
	prv_select_y = orig_select_y = PMY_TO_IY (select_y);

	moved_xdist = 0;
	moved_ydist = 0;
		 
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
        orig_num_hipts = num_hipts;
	if (orig_hi_points != NULL)
	{
	    XtFree ((char *)orig_hi_points);
	    orig_hi_points = NULL;
	}
        orig_hi_points = (XPoint *) XtMalloc (sizeof (XPoint) * num_hipts);
        for (i = 0; i < num_hipts; i++) {
            orig_hi_points[i].x = hi_points[i].x - IX_TO_PWX(select_x);
            orig_hi_points[i].y = hi_points[i].y - IY_TO_PWY(select_y);
        }

	select_region = XPolygonRegion (points, numpts, EvenOddRule);
        orig_select_region = XPolygonRegion (orig_hi_points, orig_num_hipts,
                                             EvenOddRule);

	numpts = 0;
	orig_select_rect = select_rectangle = TRUE;
	first_move = FALSE;
	select_on = TRUE;
	prv_num_hipts = 0;

	Create_Undo_Move_Map ();
/* if couldn't create undo_move_map - bail out */
        if (undo_move_map == 0) {
            Free_Select_Pixmaps ();
            return;
        }

/* if necessary, create the clip_mask */
	if (!opaque_fill) {
            clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
                                       select_width, select_height, 1);
            if (clip_mask == 0) {
                Free_Select_Pixmaps ();
                return;
            }
            Create_Clip_Mask  ();
	}

        Restore_From_Image (undo_move_map, orig_select_x, orig_select_y, 0, 0,
                            select_width, select_height, Get_GC(GC_PD_COPY));

	Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), TRUE);

	Start_Highlight_Blink();
	moved_xdist = 0;
        moved_ydist = 0;
	moved_only = TRUE;
	no_distortion = TRUE;
	deselected = FALSE;
	Start_Motion_Events(ifocus);
}

int Paste_Into_Select (tmp_image)
    XImage *tmp_image;
{
    if (copymap != 0)
	DeSelect(TRUE);

    if (copymap_image != NULL)
    {
#if 0
	XtFree (copymap_image->data);
#endif
	XDestroyImage (copymap_image);
	copymap_image = NULL;
    }

    copymap_image = tmp_image;

    copymap  = Create_Pdepth_Pixmap (picture_bg, copymap_image->width,
				     copymap_image->height);
/* if couldn't create copymap - bail out */
    if (copymap == 0) {
	copymap_image = 0;
	Free_Select_Pixmaps ();
	Set_Cursor (pwindow, current_action);
	return (K_FAILURE);
    }

    MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), copymap_image,
		  0, 0, 0, 0, copymap_image->width,
		  copymap_image->height);
	
/* Set up the select area from the paste */
    ifocus = picture_widget;
/* must be before Set_Select!!! */
    Set_Select_Icon (SELECT_RECT);
    Set_Select (copymap_image->width, copymap_image->height);
    Increase_Change_Pix (select_x, select_y, select_width, select_height);
    return (K_SUCCESS);
}


/*
 *
 * ROUTINE:  Paste
 *
 * ABSTRACT: 
 *                                
 *   Paste from the clipboard
 *
 */
void Paste (time)
    Time time;
{
    int st;
    int status;
    XImage *tmp_image;
    int picture_altered;
    unsigned long context;
    unsigned long buflen = 0;
    XImage *Read_DDIF();
    int prv_num_colors = num_colors;
    int tmp_undo_action;

    Set_Cursor_Watch (pwindow);

    st = XmClipboardStartRetrieve (disp, pwindow, time);
    if (st != ClipboardSuccess) {
	switch (st) {
	    case ClipboardLocked :
		Display_Message ("T_PASTE_CLIPBOARD_LOCKED");
		break;
	    default :
		Display_Message ("T_PASTE_FAILED");
		break;
	}
	Set_Cursor (pwindow, current_action);
	return;
    }

/* see if there is any data */
    st = XmClipboardInquireLength (disp, pwindow, "DDIF", &buflen);
    if ((buflen == 0) || (st != ClipboardSuccess)) {
	st = XmClipboardEndRetrieve (disp, pwindow);    
	st = XmClipboardUnlock (disp, pwindow, TRUE);
	Display_Message ("T_PASTE_NO_DATA");
	Set_Cursor (pwindow, current_action);
	return;
    }

    ISL_ERROR_HANDLER_SETUP

    context = ImgCreateDDIFStream (ImgK_ModeImport, 0, 0, 0,
				      Clipboard_Copy_From_Callback, 0);

    if (failure_occurred)
    {
	ISL_RECOVER_FROM_ERROR
	st = XmClipboardEndRetrieve (disp, pwindow);    
	st = XmClipboardUnlock (disp, pwindow, TRUE);
	Display_Message ("T_PASTE_FAILED");
	Set_Cursor (pwindow, current_action);
	return;
    }


    tmp_image = Read_DDIF (&status, &picture_altered, context);

    ImgDeleteDDIFStream (context);

    switch (status) {
	case K_SUCCESS :
	    if (entering_text)
		End_Text ();
	    tmp_undo_action = undo_action;
	    undo_action = PASTE; /* in case user undo's the paste */
	    st = Paste_Into_Select (tmp_image);
	    if (st == K_SUCCESS) {
/* if had to alter the picture - put up a message */
		if (picture_altered) {
		    Display_Message ("T_PASTE_IMAGE_ALTERED");
		}
	    }
	    else {
		undo_action = tmp_undo_action;
		XDestroyImage (tmp_image);
		Display_Message ("T_NO_MEM_FOR_PASTE");
	    }
	    break;
	case K_IMAGE_TOO_BIG :
	    Display_Message ("T_IMG_TOO_BIG_FOR_PASTE");
	    break;
	default :
	    Display_Message ("T_PASTE_FAILED");
	    break;
    }

    if ((status != K_SUCCESS) || (st != K_SUCCESS))
	Clear_Color_Table (prv_num_colors);
    else {
	Set_Undo_Button(PASTE); /* set undo select */
	Set_Edit_Buttons (SENSITIVE);
	if (color_dialog) {
	    if (XtIsManaged (color_dialog)) {
		Redraw_Color_Dialog_Window (0, 0, COLOR_WINDOW_WD,
					    COLOR_WINDOW_HT);
	    }
	}
    }

    st = XmClipboardEndRetrieve (disp, pwindow);    
    st = XmClipboardUnlock (disp, pwindow, TRUE);
    Set_Cursor (pwindow, current_action);
}
