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
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"

#define	is_white(color) ((color.red == white) && (color.green == white) && (color.blue == white))

#define	is_black(color) ((color.red == black) && (color.green == black) && (color.blue == black))

#define set_white(color) (color.red = color.green = color.blue = white)

#define set_black(color) (color.red = color.green = color.blue = black)

int	get_bw_widget_state();
int	set_bw_widget_state();
int	get_opposite_data();


int	get_bw_widget_state(data)
struct	color_data  *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Determines if the user has selected the black button or the
**	white button.
**
**  FORMAL PARAMETERS:
**
**	color_data - pointer to a data structure to store color widget info
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**	data->name - changes to the string "white" or "black"
**	data->color - changed to the RGB FFFFFFFFFFFF or 000000000000
**
**  COMPLETION CODES:
**
**	Returns 1 if the data changed from its original value
**	Returns 0 if the data did not change.
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned    int	i,j;
unsigned    int	changed = 0;

/* get the current state of the widget */
i = XmToggleButtonGetState(data->widget_list[1]);

/* The black button is selected. */
if (i == 1)
    {
    /* make it black */
    if (data->by_name == 1)
	{
	/* The color data is in the string format */
	if (strcmp(data->name, "black") != 0)
	    {
	    changed = 1;
	    strcpy(data->name, "black");
	    }
	}
    else
	{
	/* The color data is in RGB format */	
        if (! is_black(data->color))
		    {
		    changed = 1;
		    set_black(data->color);
		    }
	}
    }
else
/* white was selected */
    {
    /* The color data is in the string format */
    if (data->by_name == 1)
	{
	if (strcmp(data->name, "white") != 0)
	    {
	    changed = 1;
	    strcpy(data->name, "white");
	    }
	}
    else
	{
	/* The color data is in RGB format */	
    	if (!is_white(data->color))
		    {
		    changed = 1;
		    set_white(data->color);
		    }
	}
    }
return (changed);
}

int	get_opposite_data(sourcedata, destdata)
struct	color_data  *sourcedata;
struct	color_data  *destdata;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	On a monochrome system, if foreground is white, then background
**	should be black.   This routine looks at the values in the
**	color structure sourcedata and sets destdata to be the opposite
**	color.
**
**  FORMAL PARAMETERS:
**
**	color_data - pointer to a data structure that contains first color
**	color_data - pointer to a data structure that will contain opposite
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**	color_data - pointer to a data structure that will contain opposite
**
**  COMPLETION CODES:
**
**	Returns 1 if the destination data changed from its original value
**	Returns 0 if the destination data did not change.
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned    int	source_is_white,dest_was_white;
unsigned    int	changed = 0;

/* colors can either be in the format of a string, or in the format of RGB.
   Deal with string data first */
/* find out what the destination current color is */
if (destdata->by_name)
    {
    if (strcmp(destdata->name, "white") == 0)
	{
	dest_was_white = 1;
	}
    else
	{
	dest_was_white = 0;
	}
    }
else
    {
    dest_was_white = is_white(destdata->color);
    }

/* find out what the source data current color is */
if (sourcedata->by_name)
    {
    if (strcmp(sourcedata->name, "white") == 0)
	{
	source_is_white = 1;
	}
    else
	{
	source_is_white = 0;
	}
    }
else
    {
    source_is_white = is_white(sourcedata->color);
    }



/* now determine if we have source=dest and if so, switch the dest to be
   the opposite of the source.  All the while maintain the representation
   of the color data in either the name or RGB format. */

if (source_is_white)
    {
    if (dest_was_white == 1)
	{
	if (destdata->by_name)
		{
		strcpy(destdata->name, "black");
		}
	else
		{
		set_black(destdata->color);
		}
	changed = 1;
	return(changed);
	}
    }
else
    if (dest_was_white == 0)
	{
	if (destdata->by_name)
		{
		strcpy(destdata->name, "white");
		}
	else
		{
		set_white(destdata->color);
		}
	changed = 1;
	return(changed);
	}
return(changed);
}



int	set_bw_widget_state(data)
struct	color_data  *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Set the widget to have the correct button highlighted.  If the
**	Data shows that black is specified, highlight the black button.
**
**  FORMAL PARAMETERS:
**
**	color_data - pointer to a data structure that contains color info
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/

{
unsigned    int	foreground_is_white;

if (data->by_name)
    {
    if (strcmp(data->name, "white") == 0)
	{
	foreground_is_white = 1;
	}
    else
	{
	foreground_is_white = 0;
	}
    }
else
    {
    if (is_white(data->color))
	{
	foreground_is_white = 1;
	}
    else
	{
	foreground_is_white = 0;
	}
    }

if (foreground_is_white == 1)
    {
	XmToggleButtonSetState(data->widget_list[2], 1, 0);
	XmToggleButtonSetState(data->widget_list[1],0, 0);
    }
else
	{
	XmToggleButtonSetState(data->widget_list[2], 0, 0);
	XmToggleButtonSetState(data->widget_list[1], 1, 0);
	}
}

get_bw_screen_pixel(data,fore,back)
struct	color_data  *data;
unsigned    long    *fore;
unsigned    long    *back;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Given the color data, return the foreground and background
**	pixel values for the foreground and background colors.
**
**  FORMAL PARAMETERS:
**
**	color_data - pointer to a data structure that contains colors
**	fore - pointer to return value for foreground pixel
**	back - pointer to return value for background pixel
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned    int	foreground_is_white;
if (data->by_name)
    {
    if (strcmp(data->name, "white") == 0)
	{
	foreground_is_white = 1;
	}
    else
	{
	foreground_is_white = 0;
	}
    }
else
    {
    if (is_white(data->color))
	{
	foreground_is_white = 1;
	}
    else
	{
	foreground_is_white = 0;
	}
    }

if (foreground_is_white == 1)
    {
    *fore = (unsigned long)WhitePixel(display_id,screen);
    *back = (unsigned long)BlackPixel(display_id,screen);
    }
else
    {
    *back = (unsigned long)WhitePixel(display_id,screen);
    *fore = (unsigned long)BlackPixel(display_id,screen);
    }
}
