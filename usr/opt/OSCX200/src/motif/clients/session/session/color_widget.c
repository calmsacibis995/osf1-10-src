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
#include <stdio.h>
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"
#include <Xm/PushB.h>

int color_select();
int red_scale();
int blue_scale();
int green_scale();
int color_action();
int color_cancel();

/* The following should be in smdata.c */

globaldef struct colorattr_di
    {
    Widget parent;
    Widget widget_list[10];
    XColor mix;
    int changed;
    struct color_data	*current_data;
    unsigned int failed;
    } colorsetup = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static	   unsigned int	cancelindex;

#define is_static() ((vtype == StaticColor)||(vtype == StaticGray) || (vtype == TrueColor))

/********************************************/
int	get_color_widget_state(data)
struct	color_data  *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the user hits OK or Apply from a Customize dialog
**	box which deals with color.  This will read the current color
**	for the button and store it in the data structure.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**	data - contains the widget id for the color button and the place
**	       to store the current color.
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
/* get the state of the widget and store it in the data structure.  also
    set the by_name value to 1 if storing a name and 0 if storing and
    xcolor value */

if (data->mix_changed)
    {
    /* The user has changed this color since the last APPLY.  We need
       to store the temporary color which is currently in data.mix into
       the permanent color for this button (data.color).  We don't
       need to allocate a new color cell here.  We will copy the .mix pixel
       value which we know is allocated.  The button is actually already
       shown in this color, so we don't need to free any "old" colors.  
       That was done when the user said OK from the color selection dialog
       box */
    bcopy(&data->mix, &data->color, sizeof(XColor));

    /* Remember that we no longer are storing a temporary color.  Clear out
       the mix data structure.   We also no longer are storing the color
       by name.  */
    data->mix_allocated = 0;
    data->by_name = 0;
    data->mix_changed = 0;
    return (1);
    }
else
    /* If we haven't changed the color, there is no work to do */
    return (0);
}


/**************************************/
int	set_color_widget_state(data)
struct	color_data  *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Set a color button to have the correct color.   This needs to
**	be done every time a dialog box is managed.   This entails
**	allocating a color cell with the correct RGB value.
**
**  FORMAL PARAMETERS:
**
**	data - The rgb value to display in the button.  Also we will
**	       change this data structure in this routine. 
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
Status status;
static Arg arglist[2];
/*
 * set the widget to look correct when the dialog is managed. 
 *
 * Allocate a read/write color cell to use as the label background pixel 
 * and store the starting color in it. 
 */
if (data->by_name)
    {
    /* turn it into an RGB value */
    status = XParseColor(display_id, XDefaultColormap(display_id, screen), 
	data->name, &data->color);
    }

/* initialize some parts of the structure */
data->color.flags = DoRed | DoGreen | DoBlue;
data->mix_changed = 0;
data->mix_allocated = 0;

if (system_color_type == gray_system)
    {
    /* on intensity system, get levels of gray by setting red=green=blue */
    data->color.blue = data->color.red;
    data->color.green = data->color.red;
    }

/* Allocate a read only color cell for this button */
status = XAllocColor(display_id, XDefaultColormap(display_id, screen), 
		     &data->color);

if (!status) 
    {
    /* woops, no more color cells left, display a message and set the
       button insensitive so that it cannot be selected again */
    color_allocation_failure();
    XtSetSensitive(data->widget_list[0], FALSE);
    return (0);
    }

/* Yes, we were able to allocate a color cell.  It could have been
   insensitive from a previous invokation, so set it sensitive and
   set its background pixel to the correct color pixel cell which is
   returned from XAllocColor */

XtSetSensitive(data->widget_list[0], TRUE);
XtSetArg(arglist[0], XmNbackground, data->color.pixel);
XtSetValues(data->widget_list[0], arglist, 1);

return(1);
}

int color_select (widget, tag, reason)
Widget widget;
struct color_data *tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Puts up a modal dialog box with some sliders which allow the user
**	to select an RGB combination.  The current color created from
**	the selection of an RGB will be displayed in a "mix" window.
**
**  FORMAL PARAMETERS:
**
**	widget - A widget to use as the parent of this dialog box
**	tag - The color data to use as the initial settings for the
**	      sliders and the mix window color
**	reason - Not used.
**
**  IMPLICIT INPUTS:
**
**	Heavily dependent on the global color structure defined at
**	the top of this routine.
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
static  MrmRegisterArg reglist[] = {
    {"colormix_id", (caddr_t) &colorsetup.widget_list[0]},
    {"colorred_id", (caddr_t) &colorsetup.widget_list[1]},
    {"colorgreen_id", (caddr_t) &colorsetup.widget_list[2]},
    {"colorblue_id", (caddr_t) &colorsetup.widget_list[3]},
    {"colorgrey_id", (caddr_t) &colorsetup.widget_list[1]},
    {"ColorOkCallback", (caddr_t) color_action},   
    {"ColorCancelCallback", (caddr_t) color_cancel},   
    {"ColorRedCallback", (caddr_t) red_scale},   
    {"ColorGreenCallback", (caddr_t) green_scale},   
    {"ColorBlueCallback", (caddr_t) blue_scale},
};
static int reglist_num = (sizeof reglist / sizeof reglist [0]);
int okindex,  num_children;
Status status;
Arg arglist[5];
Position x,y;
char	title[20];
char	temp[10];

  if (widget == pointsetup.foreground.widget_list[0])
    tag = &pointsetup.foreground;
  else if (widget == pointsetup.background.widget_list[0])
    tag = &pointsetup.background;
  else if (widget == windowsetup.screen_foreground.widget_list[0])
    tag = &windowsetup.screen_foreground;
  else if (widget == windowsetup.screen_background.widget_list[0])
    tag = &windowsetup.screen_background;
  else if (widget == windowsetup.foreground.widget_list[0])
    tag = &windowsetup.foreground;
  else if (widget == windowsetup.background.widget_list[0])
    tag = &windowsetup.background;
  else if (widget == windowsetup.activetitle.widget_list[0])
    tag = &windowsetup.activetitle;
#ifdef XUI
  else if (widget == windowsetup.inactivetitle.widget_list[0])
    tag = &windowsetup.inactivetitle;
#endif

colorsetup.failed = 0;

if (system_color_type == gray_system)
    strcpy(title, "GrayAttributes");
else
    strcpy(title, "ColorAttributes");

/* set the current tag so the callbacks can get to it */
/* this is easier than xtremovecallback and xtaddcallback to get
    the correct data address passed to the callback */
colorsetup.current_data = tag;

MrmRegisterNames (reglist, reglist_num);
MrmFetchWidgetOverride(s_DRMHierarchy, title, XtParent(widget), title,
		NULL, 0, &colorsetup.parent, &drm_dummy_class);

/* create the color mix window */
if (colorsetup.current_data->mix_allocated)
    {
    /* If we have already changed this color before, then take the color
       data from the place where we stored that color - tag.mix */
    bcopy(&tag->mix, &colorsetup.mix, sizeof(XColor));
    /* On intensity system red=green=blue */
    if (system_color_type == gray_system)
	{
	colorsetup.mix.blue=colorsetup.mix.red;
	colorsetup.mix.green=colorsetup.mix.red;
	}
    }
else
    {
    /* We haven't changed this color yet, take the color from the tag.color
       field. */
    bcopy(&tag->color, &colorsetup.mix, sizeof(XColor));
    if (system_color_type == gray_system)
	{
	colorsetup.mix.blue=colorsetup.mix.red;
	colorsetup.mix.green=colorsetup.mix.red;
	}
    }

/* We have to deal with static color systems and dynamic color systems
   differently.  On static systems, we can't allocate a read/write color
   cell.   We can only allocate READ color cells. This includes
   TRUECOLOR machines like the FIREFOX hardware */
if (is_static())
    {
    /* allocate a read only color cell for static color systems */
    status = XAllocColor(display_id, XDefaultColormap(display_id, screen), 
	                 &colorsetup.mix);
    }
else
    {
    /* allocate a read/write color cell for non static color systems.  this
    is done for two reasons: 1. Don't have to allocate and free while the
    sliders are moving. 2. Alleviates possibility of getting an error in
    the slider routines trying to allocate a new color.  There were lots
    of toolkit bugs associated with trying to take down the modal color
    dialog box when a color allocation failed.  The sliders had garbage
    in them, an toolkit warning message was printed, and the slider 
    callbacks were called after the modal box was down, if the slider
    had been dragged.  Events were being queued up and still delivered
    after the box was down.
    */
    status = XAllocColorCells(display_id, XDefaultColormap(display_id, screen), 
    			      0, 0, 0, &colorsetup.mix.pixel, 1);
    }

if (!status) 
    {
    color_allocation_failure();
    return;
    }
else
    {
    if (!is_static())
	{
	/* for the read/write color cell, store the original color */
	colorsetup.mix.flags = DoRed | DoGreen | DoBlue;
	XStoreColor(display_id, XDefaultColormap(display_id, screen), 
	            &colorsetup.mix);
	}
    }

num_children = 0;
/* set the background resource */
XtSetArg(arglist[0], XmNbackground, colorsetup.mix.pixel);
XtSetValues(colorsetup.widget_list[num_children++], arglist, 1); 

/* set the RGB scales */
XtSetArg(arglist[0], XmNvalue, (0.5 + (colorsetup.mix.red * 100) / 65535.0));
XtSetValues(colorsetup.widget_list[num_children++], arglist, 1);

if (system_color_type == color_system)
    {
    XtSetArg(arglist[0], XmNvalue,
	     (0.5 + (colorsetup.mix.green * 100) / 65535.0));
    XtSetValues(colorsetup.widget_list[num_children++], arglist, 1);
	
    XtSetArg(arglist[0], XmNvalue,
    	     (0.5 + (colorsetup.mix.blue * 100) / 65535.0));
    XtSetValues(colorsetup.widget_list[num_children++], arglist, 1);
    }

XtManageChild(colorsetup.parent);

/* initialize the changed flag */
colorsetup.changed = 0;
}

int color_action (widget, tag, reason)
Widget  *widget;
struct color_data *tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called on an OK from the color selection dialog box.
**	If the data has changed, we need to store it somewhere.
**
**  FORMAL PARAMETERS:
**
**	widget - The widget ID of the OK button
**	tag - A pointer to the color data for the button that was pushed
**	      which originally brought up the color selection box.  We will
**	      be storing the data from the mix window into this structure.
**	reason - ButtonPushed.  Not used.
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
Time	time = 0;
Widget	parent;
static Arg cbackpixel[] = {
    {XmNbackground, 0},
    };
unsigned    int	status;

/*
 * If the user changed the selected color, store it into the colormap entry 
 * used for the background of the label widget on the main dialog box. 
 */

if (colorsetup.changed)
    {
    if (!is_static())
	{
	/* free the read/write color cell  used in the mix window */
	XFreeColors(display_id, XDefaultColormap(display_id, screen), 
		&colorsetup.mix.pixel, (int) 1, (unsigned long) 0);

	/* if there was an old mix color allocated, then free it */
	if  (colorsetup.current_data->mix_allocated)
	    {
	    XFreeColors(display_id, XDefaultColormap(display_id, screen), 
		&colorsetup.current_data->mix.pixel, 1, (unsigned long) 0);
	    }
	else
	    {
	    /* free the original button color */
	    XFreeColors(display_id, XDefaultColormap(display_id, screen), 
		&colorsetup.current_data->color.pixel, 1, (unsigned long) 0);
	    }
	/* now copy the new color selected into the mix structure */
	bcopy(&colorsetup.mix, &colorsetup.current_data->mix, sizeof(XColor));
	/* allocate a color cell for this new color. We want it to be
           read only at this point.  */
	status = XAllocColor(display_id, XDefaultColormap(display_id, screen), 
	    		     &colorsetup.current_data->mix);
	if (!status) 
	    {
	    colorsetup.failed = 1;
	    color_allocation_failure();
	    
	    XtDestroyWidget(colorsetup.parent);
	    return;
	    }
       }
    else
	{
	/* don't free anything, and just copy the color data to mix.  The
           server doesn't allow us to free color cells on a static system.
           We get a X error event.  */
	bcopy(&colorsetup.mix, &colorsetup.current_data->mix, sizeof(XColor));
	}

    /* Mark that we now have a color which differs from the original one when
       the dialog was managed.  We can't just overwrite the original one 
       because the user can still hit CANCEL from the customize dialog and
       in that case we want to throw away any changes they made, including
       the color which was just selected */
    colorsetup.current_data->mix_allocated = 1;
    colorsetup.current_data->mix_changed = 1;
    /* This will set the color of the button in the customize dialog box */
    cbackpixel[0].value = colorsetup.current_data->mix.pixel;
    XtSetValues(colorsetup.current_data->widget_list[0], cbackpixel, 1);

    /* if we were dealing with the screen fore/back colors, update the
       pattern selection buttons which have foreground and background
       color in them.  */
    if (colorsetup.current_data == (&windowsetup.screen_foreground))
	set_widget_color(1, 0, colorsetup.current_data->mix.pixel, 0);
    
    if (colorsetup.current_data == (&windowsetup.screen_background))
	set_widget_color(0, 1, 0, colorsetup.current_data->mix.pixel);
    }
else
    {
    /* if the color didn't change, we still need to free the read/write
       color cell from the mix window.   Not on static systems, because
       the server will give a warning */
    if (!is_static())
	{
	XFreeColors(display_id, XDefaultColormap(display_id, screen), 
	    &colorsetup.mix.pixel, (int) 1, (unsigned long) 0);
	}
    }
XtDestroyWidget(colorsetup.parent);
}

int color_cancel (widget, tag, reason)
Widget  *widget;
struct color_data *tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the CANCEL button is hit from the Color selection
**	dialog box.  We need to free some color cells here.
**
**  FORMAL PARAMETERS:
**
**	widget - The widget id of the CANCEL button
**	tag - A pointer to the color data that contains all of the
**	      information concerning the color we are changeing
**	reason - Not used.
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
/* reset this variable */
colorsetup.changed = 0;

/* Free the read/write color allocated for the color dialog */
if (!is_static())
    XFreeColors(display_id, XDefaultColormap(display_id, screen), 
	&colorsetup.mix.pixel, (int) 1, (unsigned long) 0);

XtDestroyWidget(colorsetup.parent);
}

int red_scale (widget, tag, reason)
Widget  *widget;
struct color_data *tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the red scale in the color selection dialog box is
**	moved.  We need to change the color in the mix window.
**
**  FORMAL PARAMETERS:
**
**	widget - widget id of the red slider
**	tag - the color data that we are manipulating
**	reason - not used
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
int value;
unsigned    int	status;
int areason = XmCR_ACTIVATE;
int temp;
static Arg arglist[2];

/* We get called multiple times.  If we have had problems allocating
   a color, then this flag will be set and we just want to return */

if (colorsetup.failed) return;

/* Get the 0-100 value of the slider */
XmScaleGetValue(widget, &value);
/* Turn that into a number between 0 and 65535 - RGB percentage */
temp = (value * 65535) / 100;

/* If it didn't really change, just return */
if (temp == colorsetup.mix.red) return;

/* otherwise, save the new number */
colorsetup.mix.red = temp;

if (system_color_type == gray_system)
    {
    /* If we are on an intensity system, set the green and blue to be
       the same value */
    colorsetup.mix.green = temp;
    colorsetup.mix.blue = temp;
    }

/* When we allocate a color cell we need to set these flags so XLIB will
   look at all values */
colorsetup.mix.flags = DoRed | DoGreen | DoBlue;

if (!is_static())
    {
    /* For a Pseudo color machine, store the new color into the read/write
       color cell which was allocated for this mix window */
    colorsetup.mix.flags = DoRed | DoGreen | DoBlue;
    XStoreColor(display_id, XDefaultColormap(display_id, screen), &colorsetup.mix);
    }
else
    {
    /* for a static machine, we can't have read/write cells.  Allocate a
       new cell with the correct color.   Note that we don't free the old
       one because the server will send back an error.  Can't free color
       cells on a static system */
    status = XAllocColor(display_id, XDefaultColormap(display_id, screen), 
			 &colorsetup.mix);
    if (!status) 
	{
	/* If we can't alloc a cell, then take the box down! */
	colorsetup.failed = 1;
	color_allocation_failure();
	XtCallCallbacks(colorsetup.widget_list[cancelindex], 
		XmNactivateCallback, &areason);
	return;
	}
    }

/* Now change the mix window widget to have the new background color */
XtSetArg(arglist[0], XmNbackground, colorsetup.mix.pixel);
XtSetValues(colorsetup.widget_list[0], arglist, 1); 

/* And mark that the user has changed the color */
colorsetup.changed = 1;
}

int green_scale (widget, tag, reason)
Widget  *widget;
struct color_data *tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the green scale in the color selection dialog box is
**	moved.  We need to change the color in the mix window.
**
**  FORMAL PARAMETERS:
**
**	widget - widget id of the green slider
**	tag - the color data that we are manipulating
**	reason - not used
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
int value;
unsigned    int	status;
static Arg arglist[2];
int areason = XmCR_ACTIVATE;
int temp;

/* We get called multiple times.  If we have had problems allocating
   a color, then this flag will be set and we just want to return */
if (colorsetup.failed) return;

/* Get the 0-100 value of the slider */
XmScaleGetValue(widget, &value);
/* Turn that into a number between 0 and 65535 - RGB percentage */
temp = (value * 65535) / 100;

/* If it didn't really change, just return */
if (temp == colorsetup.mix.green) return;

/* otherwise, save the new number */
colorsetup.mix.green = temp;
/* When we allocate a color cell we need to set these flags so XLIB will
   look at all values */
colorsetup.mix.flags = DoRed | DoGreen | DoBlue;

if (!is_static())
    {
    /* For a Pseudo color machine, store the new color into the read/write
       color cell which was allocated for this mix window */
    colorsetup.mix.flags = DoRed | DoGreen | DoBlue;
    XStoreColor(display_id, XDefaultColormap(display_id, screen), &colorsetup.mix);
    }
else
    {
    /* for a static machine, we can't have read/write cells.  Allocate a
       new cell with the correct color.   Note that we don't free the old
       one because the server will send back an error.  Can't free color
       cells on a static system */
    status = XAllocColor(display_id, XDefaultColormap(display_id, screen), 
			 &colorsetup.mix);
    if (!status) 
	{
	/* If we can't alloc a cell, then take the box down! */
	colorsetup.failed = 1;
	color_allocation_failure();
	XtCallCallbacks(colorsetup.widget_list[cancelindex], 
		XmNactivateCallback, &areason);
	return;
	}
    }

/* Now change the mix window widget to have the new background color */
XtSetArg(arglist[0], XmNbackground, colorsetup.mix.pixel);
XtSetValues(colorsetup.widget_list[0], arglist, 1); 

/* And mark that the user has changed the color */
colorsetup.changed = 1;
}

int blue_scale (widget, tag, reason)
Widget  *widget;
struct color_data *tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the blue scale in the color selection dialog box is
**	moved.  We need to change the color in the mix window.
**
**  FORMAL PARAMETERS:
**
**	widget - widget id of the blue slider
**	tag - the color data that we are manipulating
**	reason - not used
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
int value;
unsigned    int	status;
static Arg arglist[2];
int areason = XmCR_ACTIVATE;
int temp;

/* We get called multiple times.  If we have had problems allocating
   a color, then this flag will be set and we just want to return */
if (colorsetup.failed) return;

/* Get the 0-100 value of the slider */
XmScaleGetValue(widget, &value);
/* Turn that into a number between 0 and 65535 - RGB percentage */
temp = (value * 65535) / 100;

/* If it didn't really change, just return */
if (temp == colorsetup.mix.blue) return;

/* otherwise, save the new number */
colorsetup.mix.blue = temp;
/* When we allocate a color cell we need to set these flags so XLIB will
   look at all values */
colorsetup.mix.flags = DoRed | DoGreen | DoBlue;

if (!is_static())
    {
    /* For a Pseudo color machine, store the new color into the read/write
       color cell which was allocated for this mix window */
    colorsetup.mix.flags = DoRed | DoGreen | DoBlue;
    XStoreColor(display_id, XDefaultColormap(display_id, screen), &colorsetup.mix);
    }
else
    {
    /* for a static machine, we can't have read/write cells.  Allocate a
       new cell with the correct color.   Note that we don't free the old
       one because the server will send back an error.  Can't free color
       cells on a static system */
    status = XAllocColor(display_id, XDefaultColormap(display_id, screen), 
			 &colorsetup.mix);
    if (!status) 
	{
	/* If we can't alloc a cell, then take the box down! */
	colorsetup.failed = 1;
	color_allocation_failure();
	XtCallCallbacks(colorsetup.widget_list[cancelindex], 
		XmNactivateCallback, &areason);
	return;
	}
    }

/* Now change the mix window widget to have the new background color */
XtSetArg(arglist[0], XmNbackground, colorsetup.mix.pixel);
XtSetValues(colorsetup.widget_list[0], arglist, 1); 
/* And mark that the user has changed the color */
colorsetup.changed = 1;
}

int color_allocation_failure()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the xalloccolor call failes.  Notify the user
**	through a message box that color allocation has failed.
**
**  FORMAL PARAMETERS:
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
put_error(0, k_color_mapfull_msg);
fprintf(stderr, "XAllocColorCells failed\n"); 
}

int free_allocated_color(data)
struct color_data *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called from the customize dialog boxs when the dialog box is
**	removed from the screen.  It will free the colors allocated
**	for the buttons which allow the user to change colors.
**	It makes sure not to free colors on monochrome systems which
**	have not allocated colors, and on static color systems.  The
**	server will return XLIB errors if you free colors on a static
**	system.
**
**  FORMAL PARAMETERS:
**
**	data - a Pointer to the structure which hold information about
**	       color cells allocated.
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
if (system_color_type != black_white_system)
    {
    /* If we have a mix color allocated, we need to free it */
    if (data->mix_allocated) 
	{
	if (!is_static())
	    {
	    XFreeColors(display_id, XDefaultColormap(display_id, screen), 
	    &data->mix.pixel, (int) 1, (unsigned long) 0);
	    }
	data->mix_allocated = 0;
	}
    else
	/* otherwise, the color the button is displaying is stored in
	   data.color and we need to free that color */
	{
	if (!is_static())
	    {
	    XFreeColors(display_id, XDefaultColormap(display_id, screen), 
		&data->color.pixel, (int) 1, (unsigned long) 0);
	    }
	}
    }
}

get_color_screen_pixel(data1,data2,fore,back)
struct	color_data  *data1, *data2;
unsigned long	*fore,*back;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	this routine is called to determine which pixel values should
**	be used for the foreground and background boxes in the display
**	background pattern menu.  It simply returns the current pixel
**	values.
**
**  FORMAL PARAMETERS:
**
**	data1 - Address of display foreground color
**	data2 - Address of display background color
**	fore  - Pointer to a long to return the foreground pixel value
**	back - Pointer to a long to return the colormap pixel entry for
**	       the color currently allocated for background color.
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
*fore = data1->color.pixel;
*back = data2->color.pixel;
}

