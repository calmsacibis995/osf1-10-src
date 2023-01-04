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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/color.c,v 1.1.2.3 92/12/11 08:33:46 devrcs Exp $";	/* BuildSystemHeader */
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
**   Jonathan Joseph, Aug 1989
**
**  ABSTRACT:
**
**   Creates color dialog box and handles changes
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
#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <DXm/DXmColor.h>
#include <Xm/DrawingA.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

static int cur_fg_color, cur_fg_color_row, cur_fg_color_col;
static int cur_fg_color_low_x, cur_fg_color_low_y;
static int cur_fg_color_high_x,	cur_fg_color_high_y;
static XmDrawingAreaWidget dummy_widget = 0;
static XmDrawingAreaWidget mix_window_widget = 0;

static Widget *dialog_boxes[] = {
    &main_widget,
    &brush_dialog, &color_mix_dialog, &color_dialog, &edit_pat_dialog,
    &line_dialog, &msgbox, &help_widget, &write_dialog, &read_dialog,
    &ai_error_caution_box, &pattern_dialog, &print_dialog, &pic_shape_dialog, 
    &scale_dialog, &zoom_dialog, &text_widget, &color_mix_apply_cb,
    &grid_size_dialog, &include_dialog, &default_question_dialog,
    &print_2_dialog
};    


void Init_Dialog_Boxes ()
{
    int i;

    for (i = 0; i < NUM_DIALOG_BOXES; i++) {
        (*dialog_boxes[i]) = 0;
    }
}


/* find the closest color in (ddif_colors) to (color) do not choose from */
/* color indexes listed in (used).  Retrun the index of the closest */
int Find_Closest_Color_2 (color, ddif_colors, num_ddif_colors, used, num_used)
    XColor *color;
    DDIF_COLOR_TYPE *ddif_colors;
    int num_ddif_colors;
    int *used;
    int num_used;
{
    int i, j;
    int ok;
    int closest = 100000000; /* infinity for my purposes */
    int closest_ind = MAX_COLORS;
    int tmp;

    for (i = 0; i < num_ddif_colors; i++) {
	ok = TRUE;
	for (j = 0; j < num_used; j++) {
	    if (i == used[j]) {
		ok = FALSE;
	    }
	}
	if (ok) {
/* see if there is a better color matching algorithm than this */
/* if ddif_colors[i] is closer than closest_ind, closest_ind = ddif_colors[i] */
	    tmp = (((color->red >> 8) - (ddif_colors[i].red >> 8)) *
		   ((color->red >> 8) - (ddif_colors[i].red >> 8))) +  
		  (((color->green >> 8) - (ddif_colors[i].green >> 8)) *
		   ((color->green >> 8) - (ddif_colors[i].green >> 8))) +
		  (((color->blue >> 8) - (ddif_colors[i].blue >> 8)) *
		   ((color->blue >> 8) - (ddif_colors[i].blue >> 8)));
	    if (tmp < closest) {
		closest = tmp;
		closest_ind = i;
	    }
	}
    }
    return (closest_ind);
}


/* R, G, B are in the range [0, 255] */
int Find_Closest_Color (R, G, B)
    int R, G, B;
{
    int i;
    int tmp;
    int closest = 100000000; /* infinity for my purposes */
    int closest_ind = MAX_COLORS;

    for (i = 0; i < num_colors; i++) {
	tmp = (R-(colormap[i].i_red>>8))*(R-(colormap[i].i_red>>8)) +
	      (G-(colormap[i].i_green>>8))*(G-(colormap[i].i_green>>8)) +
	      (G-(colormap[i].i_blue>>8))*(G-(colormap[i].i_blue>>8));
	if (tmp < closest) {
	    closest = tmp;
	    closest_ind = i;
	}
    }
    return (closest_ind);
}


/* special colors are those colors indexes which are used for window_fg, */
/* window_bg, window highlight color, window_border_color, and the cursor */
/* foreground and background colors (if applicable)  - If any of these */
/* indexes are the same as BlackPixel or WhitePixel - they are not counted */
/* as special */
void Init_Special_Colors ()
{
    int i, j, k, skip = FALSE;
    unsigned long tmp;

    if (visual_info->visual != XDefaultVisual (disp, screen)) {
	num_special_indexes = 0;
	return;
    }


/* if on an 8 plane GPX/VsII then remember cursor colors */
    if (gpx) {
	special_colors [CURSOR_FG_INDEX] = 255;
	special_colors [CURSOR_BG_INDEX] = 254;
    }
    else {
	special_colors [CURSOR_FG_INDEX] = MAX_COLORS;
	special_colors [CURSOR_BG_INDEX] = MAX_COLORS;
    }

    
    Get_Attribute (main_widget, XmNforeground, &tmp);
    if ((tmp == black_pixel) || (tmp == white_pixel))
	special_colors[WINDOW_FG_INDEX] = MAX_COLORS;
    else
	special_colors[WINDOW_FG_INDEX] = tmp;

    Get_Attribute (main_widget, XmNbackground, &tmp);
    if ((tmp == black_pixel) || (tmp == white_pixel))
	special_colors[WINDOW_BG_INDEX] = MAX_COLORS;
    else
	special_colors[WINDOW_BG_INDEX] = tmp;

    Get_Attribute (main_widget, XmNborder, &tmp);
    if ((tmp == black_pixel) || (tmp == white_pixel))
	special_colors[WINDOW_BORDER_INDEX] = MAX_COLORS;
    else
	special_colors[WINDOW_BORDER_INDEX] = tmp;

    Get_Attribute (main_widget, XmNhighlight, &tmp);
    if ((tmp == black_pixel) || (tmp == white_pixel))
	special_colors[WINDOW_HIGHLIGHT_INDEX] = MAX_COLORS;
    else
	special_colors[WINDOW_HIGHLIGHT_INDEX] = tmp;


    j = 0;
    for (i = 0; i < NUM_SPECIAL_COLORS; i++) {
	if (special_colors[i] != MAX_COLORS) {
	    for (k = 0; ((k < j) && (!skip)); k++) {
		if (special_indexes[k] == special_colors[i])
		    skip = TRUE;
	    }	
	    if (skip) {
		skip = FALSE;
	    }
	    else {
		special_indexes [j] = special_colors[i];
		j++;
	    }
	}
    }
    num_special_indexes = j;
}



Set_Color_Mix_Display_Window ()
{
    Arg al[5];

    XtSetArg(al[0], XmNwidth, 0);
    XtSetArg(al[1], XmNheight, 0);
    XtSetArg(al[2], XmNborderWidth, 0);

    if (dummy_widget == 0) {
	dummy_widget = 
	    (XmDrawingAreaWidget) XmCreateDrawingArea (main_widget, "DummyColorWindow", al, 3);
    }
    if (mix_window_widget == 0) {
	Get_Attribute (widget_ids[COLOR_MIX_WIDGET], DXmNdisplayWindow,
		       &mix_window_widget);
    }

    if (paint_colormap == default_colormap) {
	Set_Attribute (widget_ids[COLOR_MIX_WIDGET], DXmNdisplayWindow,
		       mix_window_widget);
    }
    else {
	Set_Attribute (widget_ids[COLOR_MIX_WIDGET], DXmNdisplayWindow,
		       dummy_widget);
    }	
}



void Set_Colormap_On_Widgets ()
{
    Widget w;
    int i;

    Set_Attribute (toplevel, XmNcolormap, paint_colormap);
    if (XtWindow (toplevel) != 0) {
	XSetWindowColormap (disp, XtWindow (toplevel), paint_colormap);
	
	if (color_mix_dialog != 0) {
	    if (paint_colormap != default_colormap) {
		Set_Color_Mix_Display_Window ();
	    }
	}

	for (i = 0; i < NUM_DIALOG_BOXES; i++) {
	    w = *dialog_boxes[i];
	    if (w != 0) {
		Set_Attribute (XtParent (w), XmNcolormap, paint_colormap);
		XSetWindowColormap (disp, XtWindow (XtParent (w)),
				    paint_colormap);
	    }
	}
    }
}


/* create a private colormap */
Create_Private_Colormap ()
{
    int i, j, k, cmap_size;
    int skip = FALSE;
    unsigned long pixels[256];
    XColor colors[256], color;

    cmap_size = visual_info->colormap_size;

/* create a new colormap and alloc all of its color cells */
    paint_colormap = XCreateColormap (disp, XDefaultRootWindow (disp),
				      visual_info->visual,
				      AllocAll);
/* store all colors from into new colormap */
    for (i = 0; i < cmap_size; i++) {
	colors[i].pixel = i;
	colors[i].flags = DoRed | DoGreen | DoBlue;
    }
    XQueryColors (disp, default_colormap, colors, cmap_size);
    XStoreColors (disp, paint_colormap, colors, cmap_size);

/* free up the default colormap */
    for (i = 0; i < num_colors; i++)
	pixels[i] = colormap[i].pixel;
    XFreeColors (disp, default_colormap, pixels, num_colors, 0);

/* Make pixels array contain 0's for unused colormap indexes, 1's for used */
/* colormap indexes */
    for (i = 0; i < cmap_size; i++)
	pixels[i] = 0;
    for (i = 0; i < num_colors; i++)
	pixels[colormap[i].pixel] = 1;

/* Set unused colormap entries to the pixel values they will be bound to */
/* skip over the special colors - set them last */
    j = num_colors;
    for (i = 0; i < cmap_size; i++) {
	if (!pixels[i]) {
	    for (k = 0; ((k < num_special_indexes) && !skip); k++) {
		if (i == special_indexes[k]) {
		    skip = TRUE;
		}
	    }
	    if (skip) {
		skip = FALSE;
	    }
	    else {
		colormap[j].pixel = i;
		j++;
	    }
	}
    }

/* Set remainder of indexes to special color indexes */
    for (i = num_special_indexes - 1; i >= 0; i--) {
	colormap[j].pixel = special_indexes[i];
	j++;
    }

/* make sure all dialog boxes pick up new colormap */
    Set_Colormap_On_Widgets ();
}


/* clear the color table except for black and white */
void Clear_Color_Table (begin_color)
    int begin_color;
{
    int i;
    unsigned long pixels[256];
    unsigned long color;

/* free up the colors in the default colormap */
    if ((paint_colormap == default_colormap) && (num_colors > begin_color)) {
	for (i = begin_color; i < num_colors; i++) {
	    pixels[i - begin_color] = colormap[i].pixel;	
	}
	XFreeColors (disp, paint_colormap, pixels, num_colors - begin_color, 0);
    }
    if (color_dialog) {
        if (XtIsManaged (color_dialog)) {
/*
	    Get_Attribute (widget_ids[COLOR_DIALOG_WINDOW],
			   XmNbackground, &color);
	    XSetForeground (disp, Get_GC(GC_SD_SOLID), color);
*/
	    XSetForeground (disp, Get_GC(GC_SD_SOLID), colormap[WHITE].pixel);
	    for (i = begin_color; i < num_colors; i++) {
		XFillRectangle (disp, 
				XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
				Get_GC (GC_SD_SOLID),
				(i % NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1,
				(i / NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1,
				COLOR_SZ, COLOR_SZ);
		
	    }	    
	}
    }
    num_colors = begin_color;
    if (begin_color <= 2)
	Set_File_Color (SAVE_BW, FALSE);
}


void Update_File_Save_Color (entry)
    int entry;
{   
/* If added some color other than black or white,
 * update the file save color appropriately.
 */
    if (entry >= 2) {
	switch (visual_info->class) {
	    case GrayScale :
		if (file_color != SAVE_GRAY)
		    Set_File_Color (SAVE_GRAY, FALSE);
		break;
	    default :
		if (file_color != SAVE_COLOR) {
		    if ((colormap[entry].f_red == colormap[entry].f_green) &&
			(colormap[entry].f_green == colormap[entry].f_blue)) {
			if (file_color != SAVE_GRAY)
			    Set_File_Color (SAVE_GRAY, FALSE);
		    }
		    else {
			Set_File_Color (SAVE_COLOR, FALSE);
		    }
		}
		break;
	}
    }
}


/* If color is not in the colormap already, allocate it.
 * Return index of colormap entry corresponding to the color in entry.
 * if fl is TRUE then use float values to add the color - otherwise, use
 * integer values to set the color.
 * If can not allocate the color, return the value of the closest match in
 * entry. Retrun the status.
 */
int Add_Color (color, fl, entry)
    MyColor *color;
    int fl, *entry;
{
    int i;
    XColor xcolor;
    unsigned long pixels[2], dummy;
    float Convert_Color_I_To_F ();
    int R, G, B;

/* If color is already allocated then return the colormap entry index in 
 * entry .
 */
    if (fl) {
	R = color->f_red * 255.0 + 0.5;
	G = color->f_green * 255.0 + 0.5;
	B = color->f_blue * 255.0 + 0.5;
	for (i = 0; i < num_colors; i++) {
	    if ((colormap[i].f_red == color->f_red) &&
		(colormap[i].f_green == color->f_green) &&
		(colormap[i].f_blue == color->f_blue)) {	    
		*entry = i;	
		return (K_MATCH_COLOR);
	    }
	}
    }
    else {
	R = color->i_red >> 8;
	G = color->i_green >> 8;
	B = color->i_blue >> 8;
	for (i = 0; i < num_colors; i++) {
	    if (((colormap[i].i_red >> 8) == R) &&
		((colormap[i].i_green >> 8) == G) &&
		((colormap[i].i_blue >> 8) == B)) {	    
		*entry = i;	
		return (K_MATCH_COLOR);
	    }
	}
    }

/* if colormap is full, return failure */
    if (num_colors >= colormap_size) {
/* find closest color in colormap */
	*entry = Find_Closest_Color (R, G, B);
	return (K_CLOSEST_COLOR);
    }

/* set up X RGB values */
    if (fl) {
	xcolor.red = Convert_Color_F_To_I (color->f_red);
	xcolor.green = Convert_Color_F_To_I (color->f_green);
	xcolor.blue = Convert_Color_F_To_I (color->f_blue);
    }
    else {
	xcolor.red = color->i_red;
	xcolor.green = color->i_green;
	xcolor.blue = color->i_blue;
    }

/* if not a colormap device try to alloc the color */
    if (pdepth == 1) {
	if (!XAllocColor (disp, paint_colormap, &xcolor)) {
/* find closest color in colormap */
	    *entry = Find_Closest_Color (R, G, B);
	    return (K_CLOSEST_COLOR);
	}
	else {
	    colormap[num_colors].pixel = xcolor.pixel;
	}
    }
/* if a colormap device try to alloc the color cell */
    else {
	if (paint_colormap == default_colormap) {
	    if (!XAllocColorCells (disp, paint_colormap, 0, &dummy, 0, pixels, 1)) {
		Create_Private_Colormap ();
	    }
	    else {
		colormap[num_colors].pixel = pixels[0];
	    }
	}

	xcolor.pixel = colormap[num_colors].pixel;
	xcolor.flags = DoRed | DoGreen | DoBlue;

	XStoreColor (disp, paint_colormap, &xcolor);
    }

/* set colormap integer rgb values */    
    XQueryColor (disp, paint_colormap, &xcolor);
    colormap[num_colors].i_red = xcolor.red;
    colormap[num_colors].i_green = xcolor.green;
    colormap[num_colors].i_blue = xcolor.blue;

/* set colormap float rgb values */    
    if (fl) {
	colormap[num_colors].f_red = color->f_red;
        colormap[num_colors].f_green = color->f_green;
	colormap[num_colors].f_blue = color->f_blue;
    }
    else {
	colormap[num_colors].f_red = Convert_Color_I_To_F (xcolor.red);
        colormap[num_colors].f_green = Convert_Color_I_To_F (xcolor.green);
	colormap[num_colors].f_blue = Convert_Color_I_To_F (xcolor.blue);
    }

    ++num_colors;
    *entry = (num_colors - 1);
/* update the save file color if necessary */
    Update_File_Save_Color (*entry);
    return (K_NEW_COLOR);
}


/* black */
void Add_Black ()
{
    XColor xcolor;
    MyColor color;
    int color_entry;

    xcolor.pixel = black_pixel;
    XQueryColor (disp, default_colormap, &xcolor);
    if (visual_info->visual == XDefaultVisual (disp, screen)) {
	colormap[BLACK].i_red = xcolor.red;
	colormap[BLACK].i_green = xcolor.green;
	colormap[BLACK].i_blue = xcolor.blue;
	colormap[BLACK].pixel = xcolor.pixel;
	colormap[BLACK].f_red = 0.0;
	colormap[BLACK].f_green = 0.0;
	colormap[BLACK].f_blue = 0.0;
    }
    else {  
	color.i_red = xcolor.red;
        color.i_green = xcolor.green;
        color.i_blue = xcolor.blue;
	Add_Color (&color, FALSE, &color_entry);
    }
}


/* white */
int Add_White ()
{
    XColor xcolor;
    MyColor color;
    int color_entry;

    xcolor.pixel = white_pixel;
    XQueryColor (disp, default_colormap, &xcolor);
    if (visual_info->visual == XDefaultVisual (disp, screen)) {
	colormap[WHITE].i_red = xcolor.red;
	colormap[WHITE].i_green = xcolor.green;
	colormap[WHITE].i_blue = xcolor.blue;
	colormap[WHITE].pixel = xcolor.pixel;
	colormap[WHITE].f_red = 1.0;
	colormap[WHITE].f_green = 1.0;
	colormap[WHITE].f_blue = 1.0;
    }
    else {
	color.i_red = xcolor.red;
        color.i_green = xcolor.green;
        color.i_blue = xcolor.blue;
	Add_Color (&color, FALSE, &color_entry);
    }
}


void Init_Color_Table ()
{
    MyColor color;
    float r, g, b;
    int i;
    int color_entry;

    black_pixel = BlackPixel (disp, screen);
    white_pixel = WhitePixel (disp, screen);

/* Set up special colors and special indexes array */
    if (pdepth > 1)
	Init_Special_Colors ();


    Add_Black ();
    Add_White ();

    paint_color = BLACK;
    paint_bg_color = WHITE;

    num_colors = 2;

    if (pdepth == 1)
	return;
    if (visual_info->class == GrayScale)
	return;

/* red */
    color.f_red = 1.0;
    color.f_green = 0.0;
    color.f_blue = 0.0;
    Add_Color (&color, TRUE, &color_entry);

/* green */
    color.f_red = 0.0;
    color.f_green = 1.0;
    color.f_blue = 0.0;
    Add_Color (&color, TRUE, &color_entry);

/* blue */
    color.f_red = 0.0;
    color.f_green = 0.0;
    color.f_blue = 1.0;
    Add_Color (&color, TRUE, &color_entry);

/* yellow */
    color.f_red = 1.0;
    color.f_green = 1.0;
    color.f_blue = 0.0;
    Add_Color (&color, TRUE, &color_entry);

/* magenta */
    color.f_red = 1.0;
    color.f_green = 0.0;
    color.f_blue = 1.0;
    Add_Color (&color, TRUE, &color_entry);

/* cyan */
    color.f_red = 0.0;
    color.f_green = 1.0;
    color.f_blue = 1.0;
    Add_Color (&color, TRUE, &color_entry);

}


void Set_Color_Mix_BG_Color ()
{
    Arg al[5];

    XtSetArg (al[0], DXmNbackRedValue, 
	      Convert_Color_F_To_I (colormap[paint_bg_color].f_red));
    XtSetArg (al[1], DXmNbackGreenValue,
	      Convert_Color_F_To_I (colormap[paint_bg_color].f_green));
    XtSetArg (al[2], DXmNbackBlueValue,
	      Convert_Color_F_To_I (colormap[paint_bg_color].f_blue));
    XtSetValues (widget_ids[COLOR_MIX_WIDGET], al, 3);
}


void Set_Color_Mix_Orig_Color ()
{
    Arg al[5];

    XtSetArg (al[0], DXmNorigRedValue,
	      Convert_Color_F_To_I (colormap[paint_color].f_red));
    XtSetArg (al[1], DXmNorigGreenValue,
	      Convert_Color_F_To_I (colormap[paint_color].f_green));
    XtSetArg (al[2], DXmNorigBlueValue,
	      Convert_Color_F_To_I (colormap[paint_color].f_blue));
    XtSetValues (widget_ids[COLOR_MIX_WIDGET], al, 3);
}


void Set_Color_Mix_New_Color ()
{
    DXmColorMixSetNewColor ((DXmColorMixWidget)widget_ids[COLOR_MIX_WIDGET],
			    Convert_Color_F_To_I (colormap[paint_color].f_red),
			    Convert_Color_F_To_I (colormap[paint_color].f_green),
			    Convert_Color_F_To_I (colormap[paint_color].f_blue));
}


void Set_Foreground_Color (color)
    long color;
{
    picture_fg = color;

    XSetForeground (disp, Get_GC(GC_PD_COPY), picture_fg);

    XSetForeground (disp, Get_GC(GC_PD_OUTLINE), picture_fg);
    XSetForeground (disp, Get_GC(GC_SD_OUTLINE), colormap[paint_color].pixel);

    XSetForeground (disp, Get_GC(GC_PD_STROKE), picture_fg);
    XSetForeground (disp, Get_GC(GC_SD_STROKE), colormap[paint_color].pixel);

    XSetForeground (disp, Get_GC(GC_PD_ROUND_BRUSH), picture_fg);
    XSetForeground (disp, Get_GC(GC_SD_ROUND_BRUSH),
		    colormap[paint_color].pixel);

    XSetForeground (disp, Get_GC(GC_PD_SQUARE_BRUSH), picture_fg);
    XSetForeground (disp, Get_GC(GC_SD_SQUARE_BRUSH),
		    colormap[paint_color].pixel);

    XSetForeground (disp, Get_GC(GC_PD_FILL), picture_fg);
    XSetForeground (disp, Get_GC(GC_SD_FILL), colormap[paint_color].pixel);

    XSetForeground (disp, Get_GC(GC_PD_SPRAY), picture_fg);

    XSetForeground (disp, Get_GC(GC_PD_MASK), picture_fg);

    XSetForeground (disp, Get_GC(GC_PD_FLOOD), picture_fg);
}



void Set_Background_Color (color)
    long color;
{
    int i;
    picture_bg = color;

    picture_bg_byte = picture_bg;

    XSetBackground (disp, Get_GC(GC_PD_COPY), picture_bg);

    XSetBackground (disp, Get_GC(GC_PD_OUTLINE), picture_bg);
    XSetBackground (disp, Get_GC(GC_SD_OUTLINE), colormap[paint_bg_color].pixel);

    XSetBackground (disp, Get_GC(GC_PD_STROKE), picture_bg);
    XSetBackground (disp, Get_GC(GC_SD_STROKE), colormap[paint_bg_color].pixel);

    XSetBackground (disp, Get_GC(GC_PD_ROUND_BRUSH), picture_bg);
    XSetBackground (disp, Get_GC(GC_SD_ROUND_BRUSH),
		    colormap[paint_bg_color].pixel);

    XSetBackground (disp, Get_GC(GC_PD_SQUARE_BRUSH), picture_bg);
    XSetBackground (disp, Get_GC(GC_SD_SQUARE_BRUSH),
		    colormap[paint_bg_color].pixel);

    XSetBackground (disp, Get_GC(GC_PD_FILL), picture_bg);
    XSetBackground (disp, Get_GC(GC_SD_FILL), colormap[paint_bg_color].pixel);

    XSetBackground (disp, Get_GC(GC_PD_SPRAY), picture_bg);

    XSetBackground (disp, Get_GC(GC_PD_MASK), picture_bg);

    XSetBackground (disp, Get_GC(GC_PD_FLOOD), picture_bg);
}



void Redraw_Color_Dialog_Window (x, y, wd, ht)
    int x, y, wd, ht;
{
    int row_start, col_start, row_end, col_end;
    int i, j, entry;
    unsigned long color;

    i = row_start = y / (COLOR_SZ + 1);
    col_start = x / (COLOR_SZ + 1);
    row_end = (y + ht - 1) / (COLOR_SZ + 1);
    col_end = (x + wd - 1) / (COLOR_SZ + 1);

    if ((row_end >= row_start) && (col_end >= col_start)) {
	while (i <= row_end) {
	    j = col_start;
	    while (j <=	col_end) {
		entry = i * NUM_COLOR_COLS + j;
		if (entry < num_colors) {
		    XSetForeground (disp, Get_GC(GC_SD_SOLID),
				    colormap[entry].pixel);
		    XFillRectangle (disp, 
				    XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
				    Get_GC (GC_SD_SOLID),
				    j * (COLOR_SZ + 1) + 1,
				    i * (COLOR_SZ + 1) + 1,
				    COLOR_SZ, COLOR_SZ);
		}
		else {
/*
		    Get_Attribute (widget_ids[COLOR_DIALOG_WINDOW],
				   XmNforeground, &color);
		    XSetForeground (disp, Get_GC(GC_SD_SOLID), color);
*/
		    XSetForeground (disp, Get_GC(GC_SD_SOLID),
				    colormap[BLACK].pixel);
		    XDrawRectangle (disp, 
				    XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
				    Get_GC (GC_SD_SOLID),
				    j * (COLOR_SZ + 1) + 1,
				    i * (COLOR_SZ + 1) + 1,
				    COLOR_SZ - 1, COLOR_SZ - 1);
		}
		++j;
	    }
	    ++i;
	}
    }

/* draw the highlight around the current color */
/*
    Get_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNforeground, &color);
    XSetForeground (disp, Get_GC(GC_SD_SOLID), color);
*/
    XSetForeground (disp, Get_GC(GC_SD_SOLID), colormap[BLACK].pixel);
    XDrawRectangle (disp, XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
		    Get_GC (GC_SD_SOLID), cur_fg_color_low_x,
		    cur_fg_color_low_y, COLOR_SZ + 1, COLOR_SZ + 1);
/*
    Get_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNbackground, &color);
    XSetForeground (disp, Get_GC(GC_SD_SOLID), color);
*/
    XSetForeground (disp, Get_GC(GC_SD_SOLID), colormap[WHITE].pixel);
    XDrawRectangle (disp, XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
		    Get_GC (GC_SD_SOLID), cur_fg_color_low_x + 1,
		    cur_fg_color_low_y + 1, COLOR_SZ - 1, COLOR_SZ - 1);
}


void Refresh_Color_Dialog_Window (w, event, params, num_params)
    Widget w;
    XExposeEvent *event;
    char **params;
    int     num_params;
{
    XSetForeground (disp, Get_GC(GC_SD_SOLID), colormap[WHITE].pixel);
    XFillRectangle (disp, XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
		    Get_GC (GC_SD_SOLID), event->x, event->y,
		    event->width, event->height);
    Redraw_Color_Dialog_Window (event->x, event->y, event->width,
				event->height);
}


void Change_FG_Color (w, event, params, num_params)
    Widget  w;
    XButtonReleasedEvent *event;
    char **params;
    int     num_params;
{
    if (Finished_Action()) {
	if (cur_fg_color != paint_color) {
	    paint_color = cur_fg_color;
	    Set_Foreground_Color (colormap[paint_color].pixel);
	    if (color_mix_dialog) {
		if (XtIsManaged (color_mix_dialog)) {
		    Set_Color_Mix_New_Color ();
		    Set_Color_Mix_Orig_Color ();
		}
	    }
	    Set_Text_Color ();
	}
    }
}



New_Current_Color (color)
    int color;
{
    cur_fg_color = color;
    cur_fg_color_row = cur_fg_color / NUM_COLOR_COLS;
    cur_fg_color_col = cur_fg_color % NUM_COLOR_COLS;

    cur_fg_color_low_x = cur_fg_color_col * (COLOR_SZ + 1);
    cur_fg_color_low_y = cur_fg_color_row * (COLOR_SZ + 1);
    cur_fg_color_high_x = cur_fg_color_low_x + COLOR_SZ + 1;
    cur_fg_color_high_y = cur_fg_color_low_y + COLOR_SZ + 1;
}



Change_Cur_FG_Color (new_x, new_y)
    int new_x, new_y;
{
    int row, col, new_color;
    int x, y;
    unsigned long color;

/* if the color indeed changed */
    if ((new_x < cur_fg_color_low_x) ||
	(new_x > cur_fg_color_high_x) ||
	(new_y < cur_fg_color_low_y) ||
	(new_y > cur_fg_color_high_y)) {
/* find new color to highlight */
	row = new_y / (COLOR_SZ + 1);
	col = new_x / (COLOR_SZ + 1);
	new_color = row * NUM_COLOR_COLS + col;
	if (new_color >= num_colors) {
	    new_color = paint_color;
	}

	if (new_color != cur_fg_color) {
	    x = cur_fg_color_low_x;
	    y = cur_fg_color_low_y;

	    New_Current_Color (new_color);

/* erase old highlight */
/*
	    Get_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNbackground,
			   &color);
	    XSetForeground (disp, Get_GC(GC_SD_SOLID), color);
*/
	    XSetForeground (disp, Get_GC(GC_SD_SOLID), colormap[WHITE].pixel);
	    XDrawRectangle (disp, XtWindow (widget_ids[COLOR_DIALOG_WINDOW]), 
			    Get_GC (GC_SD_SOLID), x, y, 
			    COLOR_SZ + 1, COLOR_SZ + 1);
/* draw new highlight */
	    Redraw_Color_Dialog_Window (x, y, COLOR_SZ, COLOR_SZ);
	}
    }
}


void Drag_FG_Color (w, event, params, num_params)
    Widget  w;
    XMotionEvent *event;
    char **params;
    int     num_params;
{
    if (Finished_Action()) {
	if ((event->x >= 0) && (event->x <= COLOR_WINDOW_WD) &&
	    (event->y >= 0) && (event->y <= COLOR_WINDOW_HT)) {
	    Change_Cur_FG_Color (event->x, event->y);
	}
	else {
/* return to the original color */    
	    Change_Cur_FG_Color (
		(paint_color % NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1,
		(paint_color / NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1
	    );
/*
	    if (cur_fg_color != paint_color) {
		paint_color = cur_fg_color;
		Set_Foreground_Color (colormap[paint_color].pixel);
	    }
*/
	}
    }
}


void Clicked_On_FG_Color (w, event, params, num_params)
    Widget w;
    XButtonPressedEvent *event;
    char **params;
    int     num_params;
{
    if (Finished_Action() &&
	(event->x >= 0) && (event->x <= COLOR_WINDOW_WD) &&
	(event->y >= 0) && (event->y <= COLOR_WINDOW_HT)) {
	Change_Cur_FG_Color (event->x, event->y);
    }
}

Color_Mix_Add_Color (add)
    int add;
{
    unsigned short red, green, blue;
    MyColor color;
    XColor xcolor;
    float Convert_Color_I_To_F ();
    int color_entry;
    int status;

    DXmColorMixGetNewColor ((DXmColorMixWidget)widget_ids[COLOR_MIX_WIDGET], &red, &green, &blue);

    if (add) {
	color.f_red = Convert_Color_I_To_F (red);
	color.f_green = Convert_Color_I_To_F (green);
	color.f_blue = Convert_Color_I_To_F (blue);
        status = Add_Color (&color, TRUE, &color_entry);
        if (color_dialog) {
            if (XtIsManaged (color_dialog)) {
                Redraw_Color_Dialog_Window (
                    ((num_colors - 1) % NUM_COLOR_COLS) * (COLOR_SZ + 1),
                    ((num_colors - 1) / NUM_COLOR_COLS) * (COLOR_SZ + 1),
                    COLOR_SZ, COLOR_SZ);

                Change_Cur_FG_Color (
                    ((num_colors - 1) % NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1,
                    ((num_colors - 1) / NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1
                );
            }
        }
        paint_color = num_colors - 1;
        Set_Foreground_Color (colormap[paint_color].pixel);
	Set_Text_Color ();
    }
    else {
/* change color */
	xcolor.pixel = colormap[paint_color].pixel;
	xcolor.red = red;
	xcolor.green = green;
	xcolor.blue = blue;
	xcolor.flags = DoRed | DoGreen | DoBlue;
	
	XStoreColor (disp, paint_colormap, &xcolor);

/* set colormap integer rgb values */    
	XQueryColor (disp, paint_colormap, &xcolor);
	colormap[paint_color].i_red = xcolor.red;
	colormap[paint_color].i_green = xcolor.green;
	colormap[paint_color].i_blue = xcolor.blue;

	colormap[paint_color].f_red = Convert_Color_I_To_F (red);
	colormap[paint_color].f_green = Convert_Color_I_To_F (green);
	colormap[paint_color].f_blue = Convert_Color_I_To_F (blue);
/* update the save file color if necessary */
	Update_File_Save_Color (paint_color);
    }
}


void Color_Mix_Cancel (w, tag, r)
    Widget w;                                   /**/
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;           /**/

{
    if (Finished_Action()) {
        XtUnmanageChild(color_mix_dialog);
    }    
}


void Color_Mix_Apply_OK ()
{
	Color_Mix_Add_Color (TRUE);
	Set_Color_Mix_New_Color ();
	Set_Color_Mix_Orig_Color ();
}


void Color_Mix_Apply_Reply (w, reply, reason)
    Widget   w;
    int      *reply;
    int      reason;
{
    switch (*reply) {
	case COLOR_MIX_APPLY_YES_ID :
	    Color_Mix_Apply_OK ();
	    break;
	case COLOR_MIX_APPLY_NO_ID :
	    break;
	case COLOR_MIX_APPLY_HELP_ID :
	    return;
    }
    XtUnmanageChild (color_mix_apply_cb);
}


void Create_Color_Mix_Apply_CB ()
{
    int index;
    char label[40];
    int i;
    Widget w;

    Set_Cursor_Watch (pwindow);
/* If necessary fetch the cation box. */
    if (!color_mix_apply_cb) {
	if  (Fetch_Widget ("color_mix_apply_cb", main_widget,
			   &color_mix_apply_cb) != MrmSUCCESS)
	    DRM_Error ("can't fetch color mix apply caution box");
/* remove the help button */
/*
	w = XmMessageBoxGetChild (color_mix_apply_cb, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild (w);
*/
    }
    if (paint_colormap == default_colormap) {
	index = special_indexes[num_special_indexes - 1];
    }
    else {
	index = colormap[num_colors].pixel;
    }
    
    i = 0;
    while (special_colors[i] != index)
	i++;

    switch (i) {
	case CURSOR_FG_INDEX :
	    strcpy (label, "T_OVERWRITE_CURSOR_FG");
	    break;
	case CURSOR_BG_INDEX :
	    strcpy (label, "T_OVERWRITE_CURSOR_BG");
	    break;
	case WINDOW_FG_INDEX :
	    strcpy (label, "T_OVERWRITE_WINDOW_FG");
	    break;
	case WINDOW_BG_INDEX :
	    strcpy (label, "T_OVERWRITE_WINDOW_BG");
	    break;
	case WINDOW_BORDER_INDEX :
	    strcpy (label, "T_OVERWRITE_BORDER_COLOR");
	    break;
	case WINDOW_HIGHLIGHT_INDEX :
	    strcpy (label, "T_OVERWRITE_HIGHLIGHT_COLOR");
	    break;
    }

    Fetch_Set_Attribute (color_mix_apply_cb, XmNmessageString, label);
    XtManageChild (color_mix_apply_cb);
    Set_Cursor (pwindow, current_action);
}


void Color_Mix_Apply (w, tag, r)
    Widget w;                                   /**/
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;           /**/

{
    if (Finished_Action()) {
/* See if colormap is full */
	if (num_colors >= colormap_size) {
	    Display_Message ("T_COLOR_TABLE_FULL");
	}
/* first see if it will overwrite a special color */
	else {
	    if (num_colors >= visual_info->colormap_size - num_special_indexes) {
		Create_Color_Mix_Apply_CB ();
	    }
	    else {
		Color_Mix_Apply_OK ();
	    }
	}
    }    
}


void Color_Mix_OK (w, tag, r)
    Widget w;                                   /**/
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;           /**/

{
    if (Finished_Action()) {
	if (colormap[paint_color].pixel == black_pixel) {
	    Display_Message ("T_NO_CHANGE_BLACK");
	}
	else {
	    if (colormap[paint_color].pixel == white_pixel) {
		Display_Message ("T_NO_CHANGE_WHITE");
	    }
	    else {
		Color_Mix_Add_Color (FALSE);
		Set_Color_Mix_New_Color ();
		Set_Color_Mix_Orig_Color ();
	    }
	}
    }    
}


void Create_Color_Mix_Dialog ()
{
    Set_Cursor_Watch (pwindow);
    if (!color_mix_dialog) {
	if (Fetch_Widget ("color_mix_dialog", main_widget,
                          &color_mix_dialog) != MrmSUCCESS)
                DRM_Error ("can't fetch color mix dialog");
	if (paint_colormap != XDefaultColormap (disp, screen))
	    Set_Color_Mix_Display_Window ();
    }
    
    if (color_mix_dialog) {
	if (!XtIsManaged (color_mix_dialog)) {
	    Set_Color_Mix_BG_Color ();
	    Set_Color_Mix_New_Color ();
	    Set_Color_Mix_Orig_Color ();
	    XtManageChild (color_mix_dialog);
	}
	Pop_Widget (color_mix_dialog);
    }
    Set_Cursor (pwindow, current_action);
}

void Pickup_Color (x, y)
{
    int value, i;
    XImage *image;
    GC gc_return;

    image = XGetImage (disp, picture, x, y, 1, 1, bit_plane, img_format); 
    value = XGetPixel (image, 0, 0);
    XDestroyImage (image);

    i = 0;
    while (colormap[i].pixel != value)
	i++;


    if (color_dialog) {
        if (XtIsManaged (color_dialog)) {
	    Change_Cur_FG_Color (
		(i % NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1,
		(i / NUM_COLOR_COLS) * (COLOR_SZ + 1) + 1
	    );
	}
    }
    paint_color = i;
    Set_Foreground_Color (colormap[paint_color].pixel);

    if (color_mix_dialog) {
	if (XtIsManaged (color_mix_dialog)) {
	    Set_Color_Mix_New_Color ();
	    Set_Color_Mix_Orig_Color ();
	}
    }
    Set_Text_Color ();
}

void Dismiss_Color_Dialog (w, tag, r)
    Widget w;                                   /* pulldown menu */
    caddr_t tag;
    XmToggleButtonCallbackStruct *r;           /* just pick up the boolean */
{
    if (Finished_Action())
        XtUnmanageChild(color_dialog);
}



void Reset_Paint_Colors ()
{
    paint_color = BLACK;
    paint_bg_color = WHITE;

    Set_Foreground_Color (colormap[paint_color].pixel);

    if (color_dialog) {
	if (XtIsManaged (color_dialog)) {
	    Change_Cur_FG_Color (0, 0);	    
	}
    }

    if (color_mix_dialog) {
	if (XtIsManaged (color_mix_dialog)) {
	    Set_Color_Mix_New_Color ();
	    Set_Color_Mix_Orig_Color ();
	}
    }

    Set_Background_Color (colormap[paint_bg_color].pixel);
    Set_Text_Color ();
}



void Create_Color_Dialog()
{

    static int num_actions = 4;
    static XtActionsRec action_table [] =
    {
        {"Refresh_Color_Dialog_Window", (XtActionProc)Refresh_Color_Dialog_Window},
        {"Clicked_On_FG_Color", (XtActionProc)Clicked_On_FG_Color},
        {"Drag_FG_Color", (XtActionProc)Drag_FG_Color},
        {"Change_FG_Color", (XtActionProc)Change_FG_Color}
    };


    Set_Cursor_Watch (pwindow);
    if( !color_dialog ) {
/* add actions to the action table, fetch the widget and manage it */
	XtAddActions (action_table, num_actions);
	if (Fetch_Widget ("color_dialog_box", main_widget,
                          &color_dialog) != MrmSUCCESS)
                DRM_Error ("can't fetch color dialog");
                                                         
/* treg -> */
	if (visual_info->visual != XDefaultVisual (disp, screen)) {
 
	    Set_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNdepth, pdepth);
	    Set_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNcolormap,
			   paint_colormap);
	    Set_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNbackground,
			   colormap[WHITE].pixel);
	    Set_Attribute (widget_ids[COLOR_DIALOG_WINDOW], XmNborder,
			   colormap[BLACK].pixel);

	}
/* <- treg */

	Set_Attribute (widget_ids[COLOR_DIALOG_DISMISS_BUTTON],
		       XmNleftOffset,
		    - (XtWidth (widget_ids[COLOR_DIALOG_DISMISS_BUTTON]) / 2));

    }			
    if (color_dialog) {
	if (!XtIsManaged (color_dialog)) {
	    New_Current_Color (paint_color);
	    XtManageChild(color_dialog);
	}
	Pop_Widget (color_dialog);
    }
    Set_Cursor (pwindow, current_action);
}		


void Get_Default_Colormap ()
{
    Colormap tmp_paint_colormap;

    Clear_Color_Table (2);
/* reset the colormap to the default colormap */
    if ((paint_colormap != default_colormap) &&
	(visual_info->visual == XDefaultVisual (disp, screen)))
    {
	tmp_paint_colormap = paint_colormap;
        paint_colormap = default_colormap;
        Set_Colormap_On_Widgets ();
        XFreeColormap (disp, tmp_paint_colormap);
    }
    Reset_Paint_Colors ();
}

