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

int	get_bwcolor_widget_state(data1, data2, change, mask1, mask2)
struct	color_data  *data1, *data2;
unsigned    int	*change;
unsigned    int	mask1,mask2;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Look at the color widget and fill in the data structures with
**	the current values of the widget.
**
**  FORMAL PARAMETERS:
**
**	data1 - A pointer to a structure which will be filled in with
**	        the colors selected in the widget.
**	data2 - A pointer to a structure which will be filled in with
**	        the colors selected in the widget.
**	change - returned 1 if the data changed for this widget
**	mask1, mask2 - The bit to set in the changed mask
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
unsigned    int	changed = 0;

if (system_color_type == black_white_system)
    {
    /* Get the setting for the foreground button */
    changed = get_bw_widget_state(data1);
    /* if it changed set the mask */
    if (changed != 0)
	*change = *change | mask1;
    /* make the background be the opposite of foreground */
    changed = get_opposite_data(data1, data2);
    if (changed != 0)
	*change = *change | mask2;
    }
else
    {
    /* get the first widget state and set the changed flag if necessary */
    changed = get_color_widget_state(data1);
    if (changed != 0)
	*change = *change | mask1;
    /* get the second widget state and set the changed flag if necessary */
    changed = get_color_widget_state(data2);
    if (changed != 0)
	*change = *change | mask2;
    }
return 1;
}


int	set_bwcolor_widget_state(data1, data2)
struct	color_data  *data1, *data2;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Set the color widget to the current data values.  This is
**	necessary when you manage a customize dialog box.
**
**  FORMAL PARAMETERS:
**
**	data1 - A pointer to a structure with the current data
**	data2 - A pointer to a structure with the current data
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
unsigned    int	status;

if (system_color_type == black_white_system)
    set_bw_widget_state(data1);
else
    {
    status = set_color_widget_state(data1);
    if (!status)
	{
	/* we couldn't allocate colors - display an error and set
           the widget insensitive */
	color_allocation_failure();
	XtSetSensitive(data1->widget_list[0], FALSE);
	XtSetSensitive(data2->widget_list[0], FALSE);
	}
    else
	{
	status = set_color_widget_state(data2);
	if (!status)
	    {
	    color_allocation_failure();
	    XtSetSensitive(data2->widget_list[0], FALSE);
	    }
	}
    }
}

/****************************************/
extern	int	get_color_resource(data,index)
struct	color_data  *data;
unsigned    int	index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a color resource from the database.  We have a table
**	of the resources we care about and the parameter "index"
**	tells us the resource name.  Color resouces can either
**	be specified as a string color name, or as an RGB hex value.
**	RGB values are specified by having the first character as a # sign.
**	We will maintain the representation of the data in that if the
**	user has specified it as a name, we will store it as a name, otherwise
**	we store it as an XCOLOR structure.  
**
**  FORMAL PARAMETERS:
**
**	data - A pointer to a structure which will be filled in with
**	       the respource value for color.
**	index - An index into the resource table
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
char	svalue[256];
int	size;
XColor	tempcolor;

/* get the resource value */
size = sm_get_string_resource(index, svalue);

/* determine whether it is an RGB value or a color name.  RGB values are
	preceded by a # sign */
if (svalue[0] == '#')
    {
    /* RGB value.  Get an xcolor for this RGB */
    data->by_name = 0;
    size = XParseColor(display_id, XDefaultColormap(display_id, screen),
			svalue, &tempcolor);
    if (size == 0)
	{
        data->by_name = 1;
	strcpy(data->name, svalue);
    	}
    else
	{
	/* leave pixel value of the color alone.  Otherwise, when we get
	    a new color from a resource file, we may overwrite the pixel
	    number that we have already allocated. */
	data->color.red = tempcolor.red;
	data->color.green = tempcolor.green;
	data->color.blue = tempcolor.blue;
	}
    }
else
    {
    /* Data was specified by name.  Store a copy of the string in our
       buffer */
    data->by_name = 1;
    strcpy(data->name, svalue);
    }
}

int	set_color_resource(data,index)
struct	color_data  *data;
unsigned    int	index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Store a color resource in the resource file.
**
**  FORMAL PARAMETERS:
**
**	data -  A pointer to the structure which specifies the
**	        resource color to save.
**	index - The index value into the resource table.
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
char	*value;
unsigned	int	ivalue,status;
char	astring[20];	

/* If it is a string value, just store the strings value */
if (data->by_name)
    {
    sm_put_resource(index, data->name);
    }
else
    {
    /* Convert the rgb value to a string that can be stored */
    status = rgb_to_str(data->color, astring);
    if (status == 1)
	{
	/* store the string */
	sm_put_resource(index, astring);
	}
    }
}

set_pattern_colors(data1, data2)
struct	color_data  *data1, *data2;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The screen background pattern box needs to change when the 
**	display foreground and/or background colors change.
**	This routine gets the pixel values of the foreground and 
**	background colors and changes the pattern box foreground
**	and background buttons to have the correct colors.
**
**  FORMAL PARAMETERS:
**
**	data1 - A pointer to a structure which has the foreground color data
**	data2 - A pointer to a structure which has the background color data
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
unsigned    long    fore_pixel;
unsigned    long    back_pixel;

/* get the foreground and background pixels based on screen type */
if (system_color_type == black_white_system)
    get_bw_screen_pixel(data1,&fore_pixel,&back_pixel);
else
    get_color_screen_pixel(data1,data2, &fore_pixel,&back_pixel);

/* now set the widget to the correct pixel values*/
set_widget_color(1, 1, fore_pixel,back_pixel);
}

/****************************************/
extern	int	get_color_screen_resource(data,index,screennum,type)
struct	color_data  *data;
unsigned    int	index;
int screennum;
unsigned    int	type;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a color resource from the database.  We have a table
**	of the resources we care about and the parameter "index"
**	tells us the resource name.  Color resouces can either
**	be specified as a string color name, or as an RGB hex value.
**	RGB values are specified by having the first character as a # sign.
**	We will maintain the representation of the data in that if the
**	user has specified it as a name, we will store it as a name, otherwise
**	we store it as an XCOLOR structure.  
**
**  FORMAL PARAMETERS:
**
**	data - A pointer to a structure which will be filled in with
**	       the respource value for color.
**	index - An index into the resource table
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
char	svalue[256];
int	size;
XColor	tempcolor;

/* get the resource value */
size = sm_get_screen_resource(def_table[index].name, type, svalue);

/* determine whether it is an RGB value or a color name.  RGB values are
	preceded by a # sign */
if (svalue[0] == '#')
    {
    /* RGB value.  Get an xcolor for this RGB */
    data->by_name = 0;
    size = XParseColor(display_id, XDefaultColormap(display_id, screennum),
			svalue, &tempcolor);
    if (size == 0)
	{
        data->by_name = 1;
	strcpy(data->name, svalue);
    	}
    else
	{
	/* leave pixel value of the color alone.  Otherwise, when we get
	    a new color from a resource file, we may overwrite the pixel
	    number that we have already allocated. */
	data->color.red = tempcolor.red;
	data->color.green = tempcolor.green;
	data->color.blue = tempcolor.blue;
	}
    }
else
    {
    /* Data was specified by name.  Store a copy of the string in our
       buffer */
    data->by_name = 1;
    strcpy(data->name, svalue);
    }
}
