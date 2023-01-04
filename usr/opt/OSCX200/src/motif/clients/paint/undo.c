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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/undo.c,v 1.1.2.2 92/12/11 08:36:17 devrcs Exp $";	/* BuildSystemHeader */
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
**   UNDO contains routines to undo and redo the last action
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

static int prv_no_distortion;

void Clear_UG ()
{
    int i;

/* clear the undo grid */
    while (UG_num_used > 0)
    {
	i = UG_used[UG_num_used - 1];
	if (UG_image[i] != NULL)
	{
#if 0
	    XtFree (UG_image[i]->data);
#endif
	    XDestroyImage (UG_image[i]);
	    UG_image[i] = NULL;
	}
        UG_num_used -= 1;
    }
}


void Find_Affected_Images (x, y, wd, ht, i_array, num_i)
    int x, y, wd, ht, i_array[], *num_i;
{
    int first_row, first_col, last_row, last_col, row, col, i;
    int min_x, min_y, max_x, max_y;

    *num_i = 0;
/* cropped to window
    max_x = MIN (pic_xorigin + pwindow_wd, x + wd);
    max_y = MIN (pic_yorigin + pwindow_ht, y + ht);
    x = MAX (pic_xorigin, x);
    y = MAX (pic_yorigin, y);
*/

    max_x = MIN ((int)picture_wd, x + wd) - 1;
    max_y = MIN ((int)picture_ht, y + ht) - 1;
    min_x = MAX (0, x);
    min_y = MAX (0, y);

    if ((max_x >= min_x) && (max_y >= min_y)) {
	row = first_row = min_y / UG_ht;
	col = first_col = min_x / UG_wd;
	last_row = max_y / UG_ht;
	last_col = max_x / UG_wd;

	while (row <= last_row) {
	    while (col <= last_col) {
		if (UG_image [(i = (row * UG_cols) + col)] == 0) {
		    i_array[*num_i] = i;
		    *num_i += 1;
		}
		col++;
	    }
	    row++;
	    col = first_col;
	}
    }
}



void Find_More_Affected_Rect (x, y, wd, ht, num_i, i_array)
    int x, y, wd, ht, *num_i, i_array[];
{
    if (y < 0) {
	i_array[*num_i] = UG_last + 1;
	*num_i += 1;
	UG_extra[1].x = PMX_TO_IX (x);
	UG_extra[1].y = PMY_TO_IY (y);
	UG_extra[1].wd = wd;
	UG_extra[1].ht = picture_y - UG_extra[1].y;
    }

    if (x < 0) {
	i_array[*num_i] = UG_last + 2;
	*num_i += 1;
	UG_extra[2].x = PMX_TO_IX (x);
	UG_extra[2].y = MAX (picture_y, PMY_TO_IY (y));
	UG_extra[2].wd = picture_x - UG_extra[2].x;
	UG_extra[2].ht = MIN (picture_y + picture_ht, PMY_TO_IY (y + ht)) - 
			 UG_extra[2].y;
    }

    if ((x + wd) > (int) picture_wd) {
	i_array[*num_i] = UG_last + 3;
	*num_i += 1;
	UG_extra[3].x = picture_x + picture_wd;
	UG_extra[3].y = MAX (picture_y, PMY_TO_IY (y));
	UG_extra[3].wd = PMX_TO_IX (x + wd) - UG_extra[3].x;
	UG_extra[3].ht = MIN (picture_y + picture_ht, PMY_TO_IY (y + ht)) -
                         UG_extra[3].y;
    }

    if ((y + ht) > (int) picture_ht) {
	i_array[*num_i] = UG_last + 4;
	*num_i += 1;
	UG_extra[4].x = PMX_TO_IX (x);
	UG_extra[4].y = picture_y + picture_ht;
	UG_extra[4].wd = wd;
	UG_extra[4].ht = PMX_TO_IX (y + ht) - UG_extra[4].y;
    }
}



/*
 *
 * ROUTINE:  Find_Affected_Area
 *
 * ABSTRACT: 
 *
 * Depending on the current action find the rectangular area in the picture
 * that was changed by the action.
 *
 */
void Find_Affected_Area (x, y, width, height, num_i, i_array)
    int *x, *y, *width, *height, *num_i, i_array[];
{
    int wd;
    int xmax, ymax;

    *num_i = 0;
    i_array[0] = UG_last;
    switch (current_action) {
	case LINE :
	    *num_i = 1;
/* crop to image */
	    xmax = shape_xmax + brush_wd;
	    ymax = shape_ymax + brush_wd;
	    *x = MAX (-picture_x, shape_xmin - brush_wd);
	    *y = MAX (-picture_y, shape_ymin - brush_wd);
	    *width = MIN (xmax, IX_TO_PMX (pimage_wd)) - *x;
	    *height= MIN (ymax, IY_TO_PMY (pimage_ht)) - *y;

	    Find_More_Affected_Rect (*x, *y, *width, *height, num_i, i_array);
	    break;

	case ARC :
	case RECTANGLE :
	case SQUARE :
	case ELLIPSE :
	case CIRCLE :
	    Find_Rectangle (points[0].x, points[0].y, points[1].x, points[1].y);
	    *num_i = 1;
/* crop to image */
	    xmax = rect_x + rect_wd + brush_wd;
	    ymax = rect_y + rect_ht + brush_wd;
	    *x = MAX (-picture_x, rect_x - brush_wd);
	    *y = MAX (-picture_y, rect_y - brush_wd);
	    *width = MIN (xmax, IX_TO_PMX (pimage_wd)) - *x;
	    *height= MIN (ymax, IY_TO_PMY (pimage_ht)) - *y;

	    Find_More_Affected_Rect (*x, *y, *width, *height, num_i, i_array);
	    break;

	case MOVE :
	case CROP :
	    *num_i = 1;
	    *x = 0;
	    *y = 0;
	    *width = picture_wd;
	    *height = picture_ht;
	    break;

	case SELECT_RECT :
	case SELECT_AREA :
	    *x = select_x;
	    *y = select_y;
	    *width = select_width;
	    *height = select_height;
	    break;

	case TEXT :
	    *num_i = 1;
/* crop to image */
	    xmax = shape_xmin + shape_wd;
	    ymax = shape_ymin + shape_ht;
	    *x = MAX (-picture_x, shape_xmin);
	    *y = MAX (-picture_y, shape_ymin);
	    *width = MIN (xmax, IX_TO_PMX (pimage_wd)) - *x;
	    *height= MIN (ymax, IY_TO_PMY (pimage_ht)) - *y;

	    Find_More_Affected_Rect (*x, *y, *width, *height, num_i, i_array);
	    break;

	case POLYGON :
	case STROKE :
	case SPRAYCAN : 
	    *x = points[0].x;
	    *y = points[0].y;
	    *width = 0;
	    *height = 0;
	    break;

	case PENCIL :
	    *num_i = 1;
	    *x = points[0].x;
	    *y = points[0].y;
	    *width = 1;
	    *height = 1;
	    i_array[0] = ((points[0].y / UG_ht) * UG_cols) + 
			  (points[0].x / UG_wd);
	    break;
	case ERASE :
	case BRUSH :
	    *x = points[0].x - brush_half_wd;
	    *y = points[0].y - brush_half_wd;
	    *width = brush_wd;
	    *height = brush_wd;
	    Find_Affected_Images (points[0].x - brush_half_wd, 
				  points[0].y - brush_half_wd, 
				  brush_wd, brush_wd, i_array, num_i);

	    break;

	case FLOOD :
	    *num_i = 1;
	    if (pdepth == 1) {
		*x = pic_xorigin;
		*y = pic_yorigin;
		*width = pwindow_wd;
		*height = pwindow_ht;
	    }
	    else {
		*x = points[0].x;
		*y = points[0].y;
		*width = 1;
		*height = 1;
		i_array[0] = ((points[0].y / UG_ht) * UG_cols) + 
			      (points[0].x / UG_wd);
	    }
	    break;
    }

/* cropped at window
    xmax = MIN (*x + *width, pic_xorigin + pwindow_wd);
    ymax = MIN (*y + *height, pic_yorigin + pwindow_ht);

    if( *x < pic_xorigin )
	*x = pic_xorigin;
    if( *y < pic_yorigin )
	*y = pic_yorigin;
*/

    xmax = MIN (*x + *width, (int) picture_wd);
    ymax = MIN (*y + *height, (int) picture_ht);

    if( *x < 0)
	*x = 0;
    if( *y < 0)
	*y = 0;
    *width = xmax - *x;
    *height = ymax - *y;
}



int ui_x (i)
    int i;
{
    if (i == UG_last)
	return (undo_x);
    if (i > UG_last)
	return (UG_extra[i - UG_last].x);
    else 
	return (UG_wd * (i % UG_cols));
}


int ui_y (i)
    int i;
{
    if (i == UG_last)
	return (undo_y);
    if (i > UG_last)
	return (UG_extra[i - UG_last].y);
    else 
	return (UG_ht * (i / UG_cols));
}


int ui_wd (i)
    int i;
{
    if (i == UG_last)
	return (undo_width);
    if (i > UG_last)
	return (UG_extra[i - UG_last].wd);
    else {
	if ((UG_xwd != 0) && ((i % UG_cols) == UG_cols - 1))
	    return (UG_xwd);
	else
	    return (UG_wd);
    }
}


int ui_ht (i)
    int i;
{
    if (i == UG_last)
	return (undo_height);
    if (i > UG_last)
	return (UG_extra[i - UG_last].ht);
    else {
	if ((UG_xht != 0) && ((i / UG_cols) == UG_rows - 1))
	    return (UG_xht);
	else
	    return (UG_ht);
    }
}


void Continue_Save_Picture_State (x, y, wd, ht)
    int x, y, wd, ht;
{    
    int i, j;
    int  num_i, *i_array;
    int umx = undo_x + undo_width, umy = undo_y + undo_height;

    i_array = (int *) XtCalloc (UG_rows * UG_cols - UG_num_used, sizeof(int));

    Find_Affected_Images (x, y, wd, ht, i_array, &num_i);

    for (i = 0; i < num_i; i++) {
	j = i_array[i];
	UG_image[j] = XGetImage (disp, picture, ui_x (j), ui_y (j), 
				 ui_wd (j), ui_ht (j), bit_plane, 
				 img_format);
	UG_used[UG_num_used] = j;
	UG_num_used++;
    }
    undo_x = MAX (0, MIN (x, undo_x));
    undo_y = MAX (0, MIN (y, undo_y));
    undo_width = MIN ((int) picture_wd, MAX (umx, x + wd)) - undo_x;
    undo_height = MIN ((int) picture_ht, MAX (umy, y + ht)) - undo_y;
    XtFree ((char *)i_array);
}

/*
 *
 * ROUTINE:  Save_Picture_State
 *
 * ABSTRACT: 
 *                                       
 *  The current action is recorded in case the user wants to undo it.
 *
 */  
void Save_Picture_State()
{    
int x, y, wd, ht, i, j;
int num_i, i_array[4];

    if (undo_available) {
	Clear_UG ();

	Find_Affected_Area( &x, &y, &wd, &ht, &num_i, i_array);
	undo_x = x;
	undo_y = y;
	undo_width = wd;
	undo_height = ht;
	undo_picture_x = picture_x;
	undo_picture_y = picture_y;

	for (i = 0; i < num_i; i++) {
	    j = i_array[i];
	    if (j <= UG_last) 
		UG_image[j] = XGetImage (disp, picture, ui_x (j), ui_y (j), 
					 ui_wd (j), ui_ht (j), bit_plane, 
					 img_format);
	    else
		UG_image[j] = XSubImage (picture_image, ui_x (j), ui_y (j),
					 ui_wd (j), ui_ht (j));
	    UG_used[UG_num_used] = j;
	    UG_num_used++;
	}

	Set_Undo_Button( current_action );
	undo_action = current_action;
    }
}


/*
 *
 * ROUTINE:  Undo_Change_Picture_Size
 *
 */
void Undo_Change_Picture_Size ()
{
    int tmp;
    int p_wd, p_ht;
    int old_pxo = PMX_TO_IX (pic_xorigin), old_pyo = PMY_TO_IY (pic_yorigin);
    XImage *tmp_image;
    Arg args[5];
    int argcnt;
    int rezoom = FALSE;

    if (zoomed) {
	End_Zoom ();
	rezoom = TRUE;
    }

    tmp_image = UG_image[UG_last];
    UG_image[UG_last] = picture_image;
    picture_image = tmp_image;

    tmp = pimage_wd;
    pimage_wd = undo_width;
    undo_width = tmp;

    tmp = pimage_ht;
    pimage_ht = undo_height;
    undo_height = tmp;

/* Reset the picture origin and picture size */
        p_wd = MIN (pimage_wd, max_picture_wd);
        p_ht = MIN (pimage_ht, max_picture_ht);
/* get new scroll window width/height jj-10/11/88 */
	Find_Pwindow_Size ();

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

/* Re-initialize zoom window if necessary */
    if (rezoom) {
	Begin_Zoom (pic_xorigin + MIN ((int) pwindow_wd, pimage_wd) / 2,
		    pic_yorigin + MIN ((int) pwindow_ht, pimage_ht) / 2 ); 
    }

    Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
    Update_Pos_Window_Image_Size ();

    Set_Redo_Button();
    Update_Pic_Shape_Dialog_Fields (FALSE);
}



/*
 *
 * ROUTINE:  Undo_Easy
 *
 */
void Undo_Easy()
{
    int i, j;
    XImage *tmp_image, *tmp2_image;
    int simple;
    int new_UG_num_used = 0;

    simple = (undo_picture_x == picture_x) && (undo_picture_y == picture_y);
/* make sure picture pixmap is in same place as undo grid */
    if (!simple) {
	if (zoomed) {
	    zoom_xorigin -= pic_xorigin;
	    zoom_yorigin -= pic_yorigin; 
	}

	Boundary_Move_Picture (undo_picture_x, undo_picture_y);
	Position_Pixmap (undo_picture_x, undo_picture_y);
	pic_xorigin = undo_x + undo_width/2 - pwindow_wd/2;
	if (pic_xorigin + (int) pwindow_wd > (int) picture_wd)
	    pic_xorigin = picture_wd - pwindow_wd; 
	pic_xorigin = MAX (pic_xorigin, 0);
	pic_yorigin = undo_y + undo_height/2 - pwindow_ht/2;
	if (pic_yorigin + (int) pwindow_ht > (int) picture_ht)
	    pic_yorigin = picture_ht - pwindow_ht; 
	pic_yorigin = MAX (pic_yorigin, 0);
	Reset_SD_Stipple_Origin (- picture_x - pic_xorigin,
				 - picture_y - pic_yorigin);
/*	Reset_PD_Clip_Rect (); */
	if (zoomed) {
	    zoom_xorigin += pic_xorigin;
	    zoom_yorigin += pic_yorigin; 
	}

/* reset_sliders */
        Set_Attribute (widget_ids[PAINT_H_SCROLL_BAR], XmNvalue, picture_x +
                       pic_xorigin);
        Set_Attribute (widget_ids[PAINT_V_SCROLL_BAR], XmNvalue, picture_y +
                       pic_yorigin);
/* reset the position window */
	Update_Pos_Window_Window_Pos (FALSE);
    }

    tmp_image = XGetImage (disp, picture, undo_x, undo_y, undo_width,
			   undo_height, bit_plane, img_format);
    for (i = 0; i < UG_num_used; i++) {
	j = UG_used[i];
	if (j <= UG_last)
	{
	    MY_XPutImage
	    (
		disp, picture, Get_GC(GC_PD_COPY), UG_image[j], 0, 0,
		ui_x(j), ui_y(j), ui_wd(j), ui_ht(j)
	    );
#if 0
	    XtFree (UG_image[j]->data);
#endif
	    XDestroyImage (UG_image[j]);
	    UG_image[j] = NULL;
	}
	else
	{
	    tmp2_image = XSubImage (picture_image, ui_x (j), ui_y (j),
				    ui_wd (j), ui_ht (j));
	    Merge_Image (0, 0, ui_wd (j), ui_ht (j), ui_x (j), ui_y (j),
			 UG_image[j], picture_image, ImgK_Src);
#if 0
	    XtFree (UG_image[j]->data);
#endif
	    XDestroyImage (UG_image[j]);
	    UG_image[j] = tmp2_image;
	    UG_used[new_UG_num_used] = j;
	    ++new_UG_num_used;
	}
    }
    UG_image[UG_last] = tmp_image;	
    UG_used[new_UG_num_used] = UG_last;
    ++new_UG_num_used;
    UG_num_used = new_UG_num_used;
    if (simple)
	Refresh_Picture (undo_x, undo_y, undo_width, undo_height);
    else 
	Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
    Increase_Change_Pix (undo_x, undo_y, undo_width, undo_height);
    Set_Redo_Button();
}


/*
 *
 * ROUTINE:  Undo_Easy
 *
 * ABSTRACT: If there has been no scrolling, simply swap the undomap with the 
 *	     picture pixmap.  Otherwise a little more work is required.
 * 
 */        
void XUndo_Easy()
{
    Pixmap temp = picture;

    if ((undo_picture_x == picture_x) && (undo_picture_y == picture_y)) {
	picture = undomap;
	undomap = temp;
	Refresh_Picture (undo_x, undo_y, undo_width, undo_height);
    }
    else {
/* if the user has scrolled, copy the undomap into the picture and update the
   undomap from the image */
	XCopyArea (disp, undomap, picture, Get_GC(GC_PD_COPY), 0, 0, picture_wd,
		   picture_ht, 0, 0);
	MY_XPutImage (disp, undomap, Get_GC(GC_PD_COPY), picture_image, 
		      undo_picture_x + undo_x, undo_picture_y + undo_y,
		      undo_x, undo_y, undo_width, undo_height);
	picture_x = undo_picture_x;
	picture_y = undo_picture_y;
	pic_xorigin = (picture_wd - pwindow_wd) / 2;
	pic_yorigin = (picture_ht - pwindow_ht) / 2;
/* reset the slider values */
        Set_Attribute (widget_ids[PAINT_H_SCROLL_BAR], XmNvalue, picture_x +
                       pic_xorigin);
        Set_Attribute (widget_ids[PAINT_V_SCROLL_BAR], XmNvalue, picture_y +
                       pic_yorigin);
	Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
    }
    Set_Redo_Button();
}


/*
 *
 * ROUTINE:  Undo_Paste
 *
 * ABSTRACT: 
 *                                       
 *  Undo/Redo a pasteto the clipboard
 *
 */  
void Undo_Paste (time)
    Time time;
{
    if (Undo_Button_State())
	Paste (time);
    else {
	if (!deselected) {
	    DeSelect (TRUE);
	}
	Restore_Image (undo_move_map, orig_select_x, orig_select_y, 
		       0, 0, orig_select_wd, orig_select_ht,
		       Get_GC(GC_PD_COPY), TRUE);
	Set_Redo_Button();
    }
}



/*
 *
 * ROUTINE:  Undo_Include
 *
 * ABSTRACT: 
 *                                       
 *  Undo/Redo an include
 *
 */  
void Undo_Include ()
{
    if (Undo_Button_State()) {
	strcpy (temp_file, include_file_name);
	Include_File ();
	if (exiting_paint)
            return;
    }
    else {
	if (!deselected) {
	    DeSelect (TRUE);
	}
	Restore_Image (undo_move_map, orig_select_x, orig_select_y, 
		       0, 0, orig_select_wd, orig_select_ht,
		       Get_GC(GC_PD_COPY), TRUE);
	Set_Redo_Button();
    }
}


/*
 *
 * ROUTINE:  Undo_Copy
 *
 * ABSTRACT: 
 *                                       
 *  Undo/Redo a copy to the clipboard.  Note that only undo/redo copy
 *  to clipboard is only done when an area is selected.
 *
 */  
void Undo_Copy (time)
    Time time;
{
    if (! Undo_Button_State()) {
	XmClipboardUndoCopy( disp, pwindow );
	Set_Redo_Button();
    }
    else
    	Copy (time);
}




/*
 *
 * ROUTINE:  Undo_Crop
 *
 * ABSTRACT: 
 *                                       
 *  Undo/Redo crop
 *
 */  
void Undo_Crop()
{
    int x, y;
    int tmp;
    XImage *tmp_image;
    int i;
    Arg args[5];
    int argcnt;
    int recrop;
    int cpx, cpy, cpwd, cpht;

    if (recrop = Undo_Button_State()) {
	Stop_Highlight_Blink();
	Restore_Image (undo_move_map, PMX_TO_IX(select_x), PMY_TO_IY(select_y),
                       0, 0, select_width, select_height, Get_GC (GC_PD_COPY),
                       FALSE);
	XFreePixmap (disp, undo_move_map);
        undo_move_map = 0;
	DeSelect (FALSE);
    }
    Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y, picture,
		  ImgK_Src);

    XFreePixmap (disp, picture);
    picture = 0;

    tmp_image = picture_image;
    picture_image = UG_image[UG_last];
    UG_image[UG_last] = tmp_image;

    tmp = picture_wd;
    picture_wd = prv_picture_wd;
    prv_picture_wd = tmp;

    tmp = picture_ht;
    picture_ht = prv_picture_ht;
    prv_picture_ht = tmp;

    tmp = pimage_wd;
    pimage_wd = prv_pimage_wd;
    prv_pimage_wd = tmp;

    tmp = pimage_ht;
    pimage_ht = prv_pimage_ht;
    prv_pimage_ht = tmp;
    
    Create_Pixmaps (picture_bg, picture_wd, picture_ht);
    if (exiting_paint)
	return;
    Find_Pwindow_Size ();

    if (recrop) {
	picture_x = 0;
	picture_y = 0;
	pic_xorigin = 0;
	pic_yorigin = 0;
    }
    else {
	x = orig_select_x + orig_select_wd/2 - pwindow_wd/2;
	if (x < 0)
	    x = 0;
	if (x + (int) pwindow_wd > pimage_wd)
	    x = pimage_wd - pwindow_wd;
	y = orig_select_y + orig_select_ht/2 - pwindow_ht/2;
	if (y < 0)
	    y = 0;
	if (y + (int) pwindow_ht > pimage_ht)
	    y = pimage_ht - pwindow_ht;

	Find_Pixmap_Position (x, y, &picture_x, &picture_y);
	pic_xorigin = x - picture_x;
	pic_yorigin = y - picture_y;
    }

    MY_XPutImage (disp, picture, Get_GC(GC_PD_COPY), picture_image, picture_x,
		  picture_y, 0, 0, picture_wd, picture_ht);

    if (!recrop) {
	undo_move_map = Create_Pdepth_Pixmap (0, orig_select_wd,
					      orig_select_ht);
	undo_move_wd = orig_select_wd;
	undo_move_ht = orig_select_ht;
	Restore_From_Image (undo_move_map, orig_select_x, orig_select_y,
			    0, 0, orig_select_wd, orig_select_ht,
			    Get_GC (GC_PD_COPY));

	select_x = IX_TO_PMX (orig_select_x);
	select_y = IY_TO_PMY (orig_select_y);
	select_width = orig_select_wd;
	select_height = orig_select_ht;
	select_rectangle = orig_select_rect;

	num_hipts = orig_num_hipts;
	if (hi_points != NULL)
	{
	    XtFree ((char *)hi_points);
	    hi_points = NULL;
	}
	hi_points = (XPoint *) XtMalloc (sizeof(XPoint) * num_hipts);
	for( i=0; i < num_hipts; ++i ){
	    hi_points[i].x = orig_hi_points[i].x + IX_TO_PWX(select_x);
	    hi_points[i].y = orig_hi_points[i].y + IY_TO_PWY(select_y);
	}
	select_region = XPolygonRegion (orig_hi_points, orig_num_hipts,
					EvenOddRule);
	XOffsetRegion (select_region, select_x, select_y);

	copymap = Create_Pdepth_Pixmap (0, orig_select_wd, orig_select_ht);
	MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), copymap_image, 0, 0,
		      0, 0, orig_select_wd, orig_select_ht);

	if ((!select_rectangle) || (!opaque_fill)) {
	    clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
				       select_width, select_height, 1);
	    Create_Clip_Mask ();
	}

	select_on = TRUE;
	Copy_Select_To_Image (orig_select_x, orig_select_y, FALSE);
	Start_Highlight_Blink();
	Start_Motion_Events (ifocus);
	Set_Edit_Buttons (SENSITIVE);
    
	Increase_Change_Pix (select_x, select_y, select_width, select_height);
    }
    Set_Redo_Button ();

/* reset the border window */
/*
    argcnt = 0;
    XtSetArg (args[argcnt], XmNwidth, picture_wd);
    argcnt++;
    XtSetArg (args[argcnt], XmNheight, picture_ht);
    argcnt++;
    XtSetValues (widget_ids[BORDER_WINDOW], args, argcnt);
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

    Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
    Update_Pos_Window_Image_Size ();
    Update_Pic_Shape_Dialog_Fields (FALSE);
}




/*
 *
 * ROUTINE:  Undo_Trans_After_Deselect
 *
 */
void Undo_Trans_After_Deselect()
{
    XImage *ximage;
    int i;
    int x, y, sx, sy, dx, dy;
    int wd, ht;
    int tmp_x, tmp_y, tmp;
    Pixmap tmp_pix;
    int simple;
    int inside = FALSE, intersect = FALSE;
    int ix, iy, iwd = 0, iht = 0;

    simple = ((orig_select_x == undo_picture_x) &&
	      (orig_select_y == undo_picture_y) &&
	      (orig_select_wd == undo_width) &&
	      (orig_select_ht == undo_height) && no_distortion);

    if ((undo_width != 0) && (undo_height != 0)) {
	intersect = Intersecting_Rectangles (picture_x, picture_y,
					     picture_wd, picture_ht,
					     undo_picture_x + undo_x,
					     undo_picture_y + undo_y,
					     undo_width, undo_height);
	inside = Inside_Rectangle (undo_picture_x + undo_x, 
				   undo_picture_y + undo_y, undo_width,
				   undo_height, picture_x, picture_y,
				   picture_wd, picture_ht);

	if (intersect && !inside) {
	    Rectangle_Intersect (0, 0, picture_wd, picture_ht,
				 IX_TO_PMX (undo_picture_x + undo_x),
				 IY_TO_PMY (undo_picture_y + undo_y),
				 undo_width, undo_height,
				 &ix, &iy, &iwd, &iht);

	    Update_Image (ix, iy, iwd, iht, picture_x, picture_y, picture, 
			  ImgK_Src);
	}

	if (inside) {
	    ix = IX_TO_PMX (undo_picture_x + undo_x);
	    iy = IY_TO_PMY (undo_picture_y + undo_y);
	    iwd = undo_width;
	    iht = undo_height;	    
	}

	x = undo_picture_x + undo_x;
	y = undo_picture_y + undo_y;
	if (inside) {
	    ximage = XGetImage (disp, picture, x - picture_x, y - picture_y,
				undo_width, undo_height, bit_plane, img_format);
	}
	else {
	    ximage = XSubImage (picture_image, x, y, undo_width, undo_height);
	}
	UG_image[UG_last] = ximage;
	UG_used[0] = UG_last;
	UG_num_used = 1;
    }					
    
    if (!simple) {
	if ((undo_width != 0) && (undo_height != 0)) {
	    Restore_Image (undo_move_map, undo_picture_x, undo_picture_y,
			   undo_x, undo_y, undo_width, undo_height, 
			   Get_GC (GC_PD_COPY), FALSE);

	    if ((iwd != 0) && (iht !=0 ))
		Increase_Change_Pix (ix, iy, iwd, iht);

	    XFreePixmap (disp, undo_move_map);
	    undo_move_map = 0;
	}
	undo_move_map = Create_Pdepth_Pixmap (0, orig_select_wd,
					      orig_select_ht);
	undo_move_wd = orig_select_wd;
	undo_move_ht = orig_select_ht;
	Restore_From_Image (undo_move_map, orig_select_x, orig_select_y,
			    0, 0, orig_select_wd, orig_select_ht,
			    Get_GC (GC_PD_COPY));
    }

    prv_select_x = orig_select_x;
    prv_select_y = orig_select_y;
    select_x = IX_TO_PMX (orig_select_x);
    select_y = IY_TO_PMY (orig_select_y);
    select_width = orig_select_wd;
    select_height = orig_select_ht;
    select_rectangle = orig_select_rect;

    num_hipts = orig_num_hipts;
    if (hi_points != NULL)
    {
	XtFree ((char *)hi_points);
	hi_points = NULL;
    }
    hi_points = (XPoint *) XtMalloc (sizeof(XPoint) * num_hipts);
    for( i=0; i < num_hipts; ++i ){
	hi_points[i].x = orig_hi_points[i].x + IX_TO_PWX(select_x);
	hi_points[i].y = orig_hi_points[i].y + IY_TO_PWY(select_y);
    }
    select_region = XPolygonRegion (orig_hi_points, orig_num_hipts,
				    EvenOddRule);
    XOffsetRegion (select_region, select_x, select_y);

    copymap = Create_Pdepth_Pixmap (0, orig_select_wd, orig_select_ht);
    MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), copymap_image, 0, 0,
		  0, 0, orig_select_wd, orig_select_ht);

    if ((!select_rectangle) || (!opaque_fill)) {
	clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
                                   select_width, select_height, 1);
	Create_Clip_Mask ();
    }

    if ((undo_width != 0) && (undo_height !=0)) {
	undo_picture_x = x;
	undo_picture_y = y;
	undo_x = 0;
	undo_y = 0;
	Refresh_Picture (IX_TO_PMX (x), IY_TO_PMY (y), undo_width, undo_height);
    }
    select_on = TRUE;
    Copy_Select_To_Image (orig_select_x, orig_select_y, TRUE);
    Start_Highlight_Blink();
    Start_Motion_Events (ifocus);
    Set_Edit_Buttons (SENSITIVE);
    Set_Redo_Button ();

    Increase_Change_Pix (select_x,select_y, select_width, select_height);

    prv_no_distortion = no_distortion;
    no_distortion = TRUE;
    moved_only = TRUE;
    Set_Select_Icon (select_rectangle ? SELECT_RECT : SELECT_AREA);
}

/*
 *
 * ROUTINE:  Redo_Trans_After_Deselect
 *
 */
void Redo_Trans_After_Deselect()
{
    int simple;
    int inside = FALSE, intersect = FALSE;
    int ix, iy, iwd = 0, iht = 0;

    no_distortion = prv_no_distortion;
    simple = ((orig_select_x == undo_picture_x) &&
	      (orig_select_y == undo_picture_y) &&
	      (orig_select_wd == undo_width) &&
	      (orig_select_ht == undo_height) && no_distortion);

    Stop_Highlight_Blink();

    if( copymap != 0 )
        XFreePixmap( disp, copymap );
    copymap = 0;

    if (!simple) {
	Restore_Image (undo_move_map, orig_select_x, orig_select_y,
		       0, 0, select_width, select_height, Get_GC (GC_PD_COPY),
		       TRUE);
	XFreePixmap( disp, undo_move_map);
	undo_move_map = 0;
    }

    if (clip_mask != 0) {
        XFreePixmap (disp, clip_mask);
        XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
        clip_mask = 0;
    }

    if (hi_points != NULL)
    {
	XtFree ((char *)hi_points);
	hi_points = NULL;
	num_hipts = 0;
    }
    select_on = FALSE;
    Stop_Motion_Events(ifocus);

    if ((undo_width != 0) && (undo_height != 0)) {
	intersect = Intersecting_Rectangles (picture_x, picture_y,
					     picture_wd, picture_ht,
					     undo_picture_x, undo_picture_y,
					     undo_width, undo_height);
	inside = Inside_Rectangle (undo_picture_x, undo_picture_y, undo_width,
				   undo_height, picture_x, picture_y,
				   picture_wd, picture_ht);

	if (intersect && !inside) {
	    Rectangle_Intersect (0, 0, picture_wd, picture_ht,
				 IX_TO_PMX (undo_picture_x),
				 IY_TO_PMY (undo_picture_y), undo_width, 
				 undo_height, &ix, &iy, &iwd, &iht);
	}

	if (inside) {
	    ix = IX_TO_PMX (undo_picture_x);
	    iy = IY_TO_PMY (undo_picture_y);
	    iwd = undo_width;
	    iht = undo_height;
	}

	if (!simple) {
	    undo_move_map = Create_Pdepth_Pixmap (0, undo_width, undo_height);
	    undo_move_wd = undo_width;
	    undo_move_ht = undo_height;
	    Restore_From_Image (undo_move_map, undo_picture_x, undo_picture_y,
				0, 0, undo_width, undo_height,
				Get_GC (GC_PD_COPY));
	}

	if (!inside) {
	    Merge_Image (0, 0, undo_width, undo_height, undo_picture_x,
			 undo_picture_y, UG_image[UG_last], picture_image,
			 ImgK_Src);

	}
	if (intersect) {
	    MY_XPutImage (disp, picture, Get_GC(GC_PD_COPY), UG_image[UG_last],
			  0, 0, ix, iy, iwd, iht);
	    Increase_Change_Pix (ix, iy, iwd, iht);
	    Refresh_Picture (ix, iy, iwd, iht);
	}
	Clear_UG ();
    }


    Set_Edit_Buttons (INSENSITIVE);
    Set_Redo_Button ();
    Increase_Change_Pix (select_x,select_y, select_width, select_height);
}




/*
 *
 * ROUTINE:  Undo_Move
 *
 * ABSTRACT: Move is slightly different than the others.  While the
 * area is selected, just copy the selected area to the old place.  If
 * the moved area has been de-selected, use the undomap
 *           
 */
void Undo_Move()
{
int xchange, ychange;
int i;
int x, y;
int wd, ht;
int tmp_x, tmp_y;
Pixmap tmp_pix;

/* Re-set the values */
    if ((prv_select_x != PMX_TO_IX (select_x)) ||
	(prv_select_y != PMY_TO_IY (select_y))) {
	Stop_Highlight_Blink();
	Restore_Image (undo_move_map, PMX_TO_IX (select_x),
		       PMY_TO_IY (select_y), 0, 0, select_width,
		       select_height, Get_GC (GC_PD_COPY),
		       Get_GC (GC_PD_COPY), FALSE);

	Restore_From_Image (undo_move_map, prv_select_x, prv_select_y,
			    0, 0, select_width, select_height,
			    Get_GC (GC_PD_COPY));

	tmp_x = select_x;
	tmp_y = select_y;
	select_x = IX_TO_PMX (prv_select_x);
	select_y = IY_TO_PMY (prv_select_y);
	prv_select_x = PMX_TO_IX (tmp_x);
	prv_select_y = PMY_TO_IY (tmp_y);

	Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), 
			      FALSE);


/* jj-11/10/88 */
	xchange = select_x - tmp_x;
	ychange = select_y - tmp_y;
	for( i=0; i < num_hipts; ++i ){
	    hi_points[i].x += xchange;
	    hi_points[i].y += ychange;
	}    
	shape_xmin += xchange;
	shape_ymin += ychange;
	XOffsetRegion( select_region, xchange, ychange );
/* Update views */
	Refresh_Picture( select_x, select_y, select_width, select_height );
	Refresh_Picture( tmp_x , tmp_y, select_width, select_height );
	Start_Highlight_Blink();
    }
    Set_Redo_Button ();
}



/*
 *
 * ROUTINE:  Undo_Transformation
 *
 */
void Undo_Transformation()
{
int xchange, ychange;
int i;
int x, y;
int wd, ht;
int tmp_x, tmp_y, tmp;
Pixmap tmp_pix;
int simple;

/* Re-set the values */
    simple = (orig_select_x == PMX_TO_IX (select_x)) &&
	     (orig_select_y == PMY_TO_IY (select_y)) && no_distortion;

    if (clip_mask != 0) {
	XFreePixmap (disp, clip_mask);
	clip_mask = 0;
	XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
    }

    Stop_Highlight_Blink();
    UG_image[UG_last] = XGetImage (disp, copymap, 0, 0, select_width,
				   select_height, bit_plane, img_format);

    UG_used[0] = UG_last;
    UG_num_used = 1;
    if (!simple) {
	Restore_Image (undo_move_map, PMX_TO_IX (select_x),
		       PMY_TO_IY (select_y), 0, 0, select_width,
		       select_height, Get_GC (GC_PD_COPY),
		       Get_GC (GC_PD_COPY), FALSE);

	if ((select_width != orig_select_wd) || 
	    (select_height != orig_select_ht)) {
	    XFreePixmap (disp, copymap);
	    XFreePixmap (disp, undo_move_map);
	    copymap = Create_Pdepth_Pixmap (0, orig_select_wd, orig_select_ht);
	    undo_move_map = Create_Pdepth_Pixmap (0, orig_select_wd,
						  orig_select_ht);
	    undo_move_wd = orig_select_wd;
	    undo_move_ht = orig_select_ht;
	}
    }

    MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), copymap_image, 0, 0,
		  0, 0, orig_select_wd, orig_select_ht);
	
    if 	(!simple) {
	Restore_From_Image (undo_move_map, orig_select_x, orig_select_y,
			    0, 0, orig_select_wd, orig_select_ht,
			    Get_GC (GC_PD_COPY));


	tmp_x = select_x;
	tmp_y = select_y;
	select_x = IX_TO_PMX (orig_select_x);
	select_y = IY_TO_PMY (orig_select_y);
	prv_select_x = PMX_TO_IX (tmp_x);
	prv_select_y = PMY_TO_IY (tmp_y);

	prv_select_width = select_width;
	select_width = orig_select_wd;

	prv_select_height = select_height;
	select_height = orig_select_ht;

	prv_select_rectangle = select_rectangle;
	select_rectangle = orig_select_rect;	

	prv_num_hipts = num_hipts;
	num_hipts = orig_num_hipts;

	if (prv_hi_points != NULL)
	{
	    XtFree ((char *)prv_hi_points);
	    prv_hi_points = NULL;
	}
	prv_hi_points = hi_points;
	hi_points = (XPoint *) XtMalloc (sizeof(XPoint) * num_hipts);

	for( i=0; i < num_hipts; ++i ){
	    hi_points[i].x = orig_hi_points[i].x + IX_TO_PWX(select_x);
	    hi_points[i].y = orig_hi_points[i].y + IY_TO_PWY(select_y);
	}

	for( i=0; i < prv_num_hipts; ++i ){
	    prv_hi_points[i].x -= IX_TO_PWX(tmp_x);
	    prv_hi_points[i].y -= IY_TO_PWY(tmp_y);
	}    

	prv_select_region = select_region;
	XOffsetRegion (prv_select_region, -tmp_x, -tmp_y);
	select_region = XPolygonRegion (orig_hi_points, orig_num_hipts,
					EvenOddRule);
	XOffsetRegion (select_region, select_x, select_y);
    }

    if ((!select_rectangle) || (!opaque_fill)) {
	clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
                                   select_width, select_height, 1);
	Create_Clip_Mask ();
    }

    Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), FALSE);

/* Update views */
    if (!simple) {
	Refresh_Picture( tmp_x , tmp_y, prv_select_width, prv_select_height );
    }
    Refresh_Picture( select_x, select_y, select_width, select_height );
    Start_Highlight_Blink();
    Set_Redo_Button ();
    Set_Select_Icon (select_rectangle ? SELECT_RECT : SELECT_AREA);
}



/*
 *
 * ROUTINE:  Redo_Transformation
 *
 */
void Redo_Transformation()
{
int xchange, ychange;
int i;
int x, y;
int wd, ht;
int tmp_x, tmp_y, tmp;
Pixmap tmp_pix;
Region tmp_region;
XPoint *tmp_pts;
int simple;

/* Re-set the values */
    simple = (prv_select_x == PMX_TO_IX (select_x)) &&
	     (prv_select_y == PMY_TO_IY (select_y)) && no_distortion;

    if (clip_mask != 0) {
	XFreePixmap (disp, clip_mask);
	clip_mask = 0;
	XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
    }

    Stop_Highlight_Blink();

    if (!simple) {
	Restore_Image (undo_move_map, PMX_TO_IX (select_x),
		       PMY_TO_IY (select_y), 0, 0, select_width,
		       select_height, Get_GC (GC_PD_COPY),
		       Get_GC (GC_PD_COPY), FALSE);

	if ((select_width != prv_select_width) || 
	    (select_height != prv_select_height)) {
	    XFreePixmap (disp, copymap);
	    XFreePixmap (disp, undo_move_map);
	    copymap = Create_Pdepth_Pixmap (0, prv_select_width, 
					    prv_select_height);
	    undo_move_map = Create_Pdepth_Pixmap (0, prv_select_width,
						  prv_select_height);
	    undo_move_wd = prv_select_width;
	    undo_move_ht = prv_select_height;
	}
	MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), UG_image[UG_last],
		      0, 0, 0, 0, prv_select_width, prv_select_height);
    }
    else {
	MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), UG_image[UG_last],
		      0, 0, 0, 0, select_width, select_height);
    }

    Clear_UG ();

    if (!simple) {
	Restore_From_Image (undo_move_map, prv_select_x, prv_select_y,
			    0, 0, prv_select_width, prv_select_height,
			    Get_GC (GC_PD_COPY));

	tmp_x = select_x;
	tmp_y = select_y;
	select_x = IX_TO_PMX (prv_select_x);
	select_y = IY_TO_PMY (prv_select_y);

	select_width = prv_select_width;
	select_height = prv_select_height;

	select_rectangle = prv_select_rectangle;

	tmp_pts = hi_points;
	hi_points = prv_hi_points;
	prv_hi_points = 0;
	if (tmp_pts != NULL)
	{
	    XtFree ((char *)tmp_pts);
	    tmp_pts = NULL;
	}
	num_hipts = prv_num_hipts;

	for( i=0; i < num_hipts; ++i ){
	    hi_points[i].x += IX_TO_PWX(select_x);
	    hi_points[i].y += IY_TO_PWY(select_y);
	}    

	tmp_region = select_region;
	select_region = prv_select_region;
	XDestroyRegion (tmp_region);
	XOffsetRegion (select_region, select_x, select_y);
    }

    if ((!select_rectangle) || (!opaque_fill)) {
	clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
                                   select_width, select_height, 1);
	Create_Clip_Mask ();
    }

    Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y), FALSE);


/* Update views */
    if (!simple) {
	Refresh_Picture (tmp_x , tmp_y, orig_select_wd, orig_select_ht);
    }
    Refresh_Picture (select_x, select_y, select_width, select_height);
    Start_Highlight_Blink();
    Set_Redo_Button ();
    Set_Select_Icon (select_rectangle ? SELECT_RECT : SELECT_AREA);
}




Undo_Selection_Transformations ()
{
/* if only moves done do quicker undo */
    if (!deselected){
	if (moved_only) 
	    Undo_Move ();
	else {
	    if (Undo_Button_State())
		Redo_Transformation();
	    else 
		Undo_Transformation();
	}
    }
    else {
	if (Undo_Button_State())
	    Redo_Trans_After_Deselect();
	else 
	    Undo_Trans_After_Deselect();
    }
}


/*
 *
 * ROUTINE:  Undo_Quick_Copy
 *
 * ABSTRACT: 
 *                                       
 *  Undo/Redo a quick copy.
 *
 */  
void Undo_Quick_Copy ()
{
    int tmp_x, tmp_y;
    int i;

    if (!deselected) {
	if (! Undo_Button_State()) {
	    tmp_x = select_x;
	    tmp_y = select_y;
	    Stop_Highlight_Blink();
	    Restore_Image (undo_move_map, orig_select_x, orig_select_y, 0, 0,
			   select_width, select_height, Get_GC (GC_PD_COPY),
			   FALSE);
	    MY_XPutImage (disp, undo_move_map, Get_GC(GC_PD_COPY),
			  UG_image[UG_last], 0, 0, 0, 0, select_width,
			  select_height);
	    Clear_UG ();
	    prv_select_x = orig_select_x = qc_x;
	    prv_select_y = orig_select_y = qc_y;
	    select_x = IX_TO_PMX (qc_x);
	    select_y = IY_TO_PMY (qc_y);
	    for (i = 0; i < num_hipts; ++i) {
		hi_points[i].x += select_x - tmp_x;
		hi_points[i].y += select_y - tmp_y;
	    }
	    XOffsetRegion (select_region, select_x - tmp_x, select_y - tmp_y);
	    Set_Redo_Button();
	    Refresh_Picture (select_x, select_y, select_width, select_height);
	    Refresh_Picture (tmp_x , tmp_y, select_width, select_height);
	    Start_Highlight_Blink();
	    qc_redo = TRUE;
	}
	else {
	    Quick_Copy ();
	}
    }
    else {
	if (! Undo_Button_State()) {
/* Copy undo_move_map into picture at orig_select_x, orig_select_y */
	    Restore_Image (undo_move_map, orig_select_x, orig_select_y, 0, 0,
			   select_width, select_height, Get_GC(GC_PD_COPY),
			   TRUE);
/* Copy UG_image[UG_last] into the undo_move_map */
	    MY_XPutImage (disp, undo_move_map, Get_GC(GC_PD_COPY),
			  UG_image[UG_last], 0, 0, 0, 0, select_width,
			  select_height);
	    Increase_Change_Pix (IX_TO_PMX (orig_select_x), 
				 IY_TO_PMY (orig_select_y), 
				 select_width, select_height);
/* Set select variables for qc_x, qc_y */
	    select_x = IX_TO_PMX (qc_x);
	    select_y = IY_TO_PMY (qc_y);
	    prv_select_x = orig_select_x = qc_x;
	    prv_select_y = orig_select_y = qc_y;
	    qc_redo = TRUE;
	}
	else {
/* Copy undo_move_map into UG_image[UG_last] */
	    UG_image[UG_last] = XGetImage (disp, undo_move_map, 0, 0,
					   select_width, select_height,
					   bit_plane, img_format);
/* Copy from Image into undo_move_map */
	    select_x = PWX_TO_IX(pwindow_wd/2 - select_width/2);
	    select_y = PWY_TO_IY(pwindow_ht/2 - select_height/2);
	    prv_select_x = orig_select_x = PMX_TO_IX (select_x);
	    prv_select_y = orig_select_y = PMY_TO_IY (select_y);
	    Restore_From_Image (undo_move_map, orig_select_x, orig_select_y,
				0, 0, select_width, select_height, 
				Get_GC (GC_PD_COPY));
/* Set select variables for middle of screen */
	    qc_redo = FALSE;
	}

	Clear_UG ();

	num_hipts = orig_num_hipts;
	if (hi_points != NULL)
	{
	    XtFree ((char *)hi_points);
	    hi_points = NULL;
	}
	hi_points = (XPoint *) XtMalloc (sizeof(XPoint) * num_hipts);
	for (i=0; i < num_hipts; ++i) {
	    hi_points[i].x = orig_hi_points[i].x + IX_TO_PWX(select_x);
	    hi_points[i].y = orig_hi_points[i].y + IY_TO_PWY(select_y);	
	}

	select_region = XPolygonRegion (orig_hi_points, orig_num_hipts,
					EvenOddRule);
	XOffsetRegion (select_region, select_x, select_y);

/* Create the copymap via the copymap image */
	copymap = Create_Pdepth_Pixmap (0, orig_select_wd, orig_select_ht);
	MY_XPutImage (disp, copymap, Get_GC(GC_PD_COPY), copymap_image, 0, 0,
		      0, 0, orig_select_wd, orig_select_ht);

/* Create the clip mask if neccesary */
	if ((!select_rectangle) || (!opaque_fill)) {
	    clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
				       select_width, select_height, 1);
	    Create_Clip_Mask ();
	}


	moved_only = TRUE;
	no_distortion = TRUE;
	deselected = FALSE;
	select_on = TRUE;
	Copy_Select_To_Image (orig_select_x, orig_select_y, TRUE);
	Start_Motion_Events (ifocus);
	Set_Edit_Buttons (SENSITIVE);
	Increase_Change_Pix (select_x,select_y, select_width, select_height);
	Set_Redo_Button();
	Set_Select_Icon (select_rectangle ? SELECT_RECT : SELECT_AREA);
	Start_Highlight_Blink();
    }
}



/*
 *
 * ROUTINE:  Undo
 *
 * ABSTRACT: 
 *
 *  Restore the picture to the way it was before the last action was done.
 *
 */
void Undo (time)
    Time time;
{
    Pixmap tmp;
    int xchange, ychange;
    int i;

    Set_Cursor_Watch (pwindow);
    switch (undo_action) {
	case CUT :
	case MOVE :
	case SCALE :
	case CLEAR :
	case INVERT :
	    Undo_Selection_Transformations();
	    break;

	case COPY :
	    Undo_Copy (time);
	    break;

	case PASTE :
	    Undo_Paste (time);
	    break;

	case INCLUDE :
	    Undo_Include();
	    if (exiting_paint)
		return;
	    break;

	case CROP :
	    Undo_Crop();
	    if (exiting_paint)
		return;
	    break;

	case QUICK_COPY :
	    Undo_Quick_Copy ();
	    break;

	case LINE :
	case STROKE :                                    
	case RECTANGLE :
	case SQUARE :
	case ELLIPSE :
	case CIRCLE :
	case POLYGON :
	case ERASE :
	case CLEAR_WW :
	case BRUSH :
	case PENCIL :
	case TEXT :
	case FLOOD :
	case SPRAYCAN :
	case ARC :
	    Undo_Easy ();
	    break;

	case SELECT_RECT :
	case SELECT_AREA :
	    if (!deselected)
		DeSelect (TRUE);
	    else {
		if (Undo_Button_State())
		    Undo_Trans_After_Deselect();
		else 
		    Redo_Trans_After_Deselect();
	    }
	    break;

	case SCALE_PICTURE :
	case CHANGE_PICTURE_SIZE :
	case FULLVIEW_CROP :
	    Undo_Change_Picture_Size ();
	    if (exiting_paint)
		return;
	    break;
    }
    Set_Cursor (pwindow, current_action);
}               
  
                             
