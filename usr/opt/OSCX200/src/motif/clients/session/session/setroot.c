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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "smdata.h"
#include "smconstants.h"
#include "smpatterns.h"
#include "smresource.h"

extern	Window WmRootWindow ();

static	unsigned long	*color_1 = 0;
static	unsigned long	*color_2 = 0;
static	unsigned int *color1_allocated = 0;
static  unsigned int *color2_allocated = 0;
static	int first = 1;

#define is_static(type) ((type == StaticColor)||(type == StaticGray) || (type == TrueColor))

extern	int	execute_display(changemask)
unsigned    int	changemask;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Set the display parameters to the user's choices.  This includes
**	screen saver and display background pattern.
**
**  FORMAL PARAMETERS:
**
**	changemask - A bit is set for each item that this routine
**		     handles.  If the bit is set, then the calls
**		     are made to set the display to the value.
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
int timeout,interval;

/* Screen saver settings */
if (((changemask & mdissaver) != 0) || ((changemask & mdissaverseconds) != 0))
    { 
    /* Enabled */
    if (windowsetup.saver)
	{
	 /* convert from minutes to seconds */
	 interval = (windowsetup.saver_seconds * 60);
	 timeout = interval;
        }
    else
	/* Disabled */
	{
	timeout = 0;
	interval = 0;
	}

    /* The X call which sets the screen saver timeout value */
    XSetScreenSaver(display_id, timeout, interval, PreferBlanking,
		    AllowExposures);
    }


/* Display background pattern and colors */
if (((changemask & mdispattern) != 0) || ((changemask & mdisforeground) != 0)
	    || ((changemask & mdisbackground) != 0))

    {
    unsigned	int numscreens,type,i;
    unsigned	int pattern_index[num_system_types];
    unsigned	int index_done[num_system_types];
    struct  color_data	screen_background[num_system_types];
    struct  color_data	screen_foreground[num_system_types];
    Window	*wmrootlist = 0;

    for (i=0; i< num_system_types; i++)
	{
	index_done[i] = 0;
	}
    pattern_index[system_color_type] = windowsetup.pattern_index;
    bcopy(&windowsetup.screen_background, &screen_background[system_color_type], 
		    sizeof(struct color_data));
    bcopy(&windowsetup.screen_foreground, &screen_foreground[system_color_type], 
		    sizeof(struct color_data));
    index_done[system_color_type] = 1;
    numscreens = XScreenCount(display_id);
    if (first)
	{
	first = 0;
	color_1 = (unsigned long *)XtMalloc(numscreens * sizeof(long));
	color_2 = (unsigned long *)XtMalloc(numscreens * sizeof(long));
	color1_allocated = (unsigned int *)XtMalloc(numscreens * sizeof(int));
	color2_allocated = (unsigned int *)XtMalloc(numscreens * sizeof(int));
	}
    wmrootlist = (Window *)XtMalloc(sizeof(Window)*numscreens);
    
    /* For each screen, set the background pattern*/
    for (i=0; i < numscreens; i++)
	{
	char	svalue[256];
	int	value[4];
	int	size;
	unsigned    int	type;
	Window	root,wmroot;

	color_1[i] = 0L;
	color_2[i] = 0L;
	color1_allocated[i] = 0;
	color2_allocated[i] = 0;

	root = XRootWindow(display_id, i);
	wmroot = WmRootWindow(display_id,root);
	wmrootlist[i] = WmRootWindow(display_id,wmroot);
	type = determine_system_color(display_id, i);
	if (index_done[type] == 0)
	    {
	    /* get the pattern resource */
	    size = sm_get_int_screen_resource(idispattern, type, value);
	    pattern_index[type] = value[0];
	    get_color_screen_resource(&screen_foreground[type], idisforeground,
			    i, type);
	    get_color_screen_resource(&screen_background[type], idisbackground,
			    i, type);
	    }
	
	if (pattern_index[type] == solid_background)
	    {
	    background_solid(&screen_background[type],
		    XRootWindow(display_id, i),
	    	    wmrootlist[i], i);
	    }
	if (pattern_index[type] == solid_foreground)
	    {
	    background_solid(&screen_foreground[type],
		    XRootWindow(display_id, i),
	    	    wmrootlist[i], i);
	    }
	else
	    {
	    background_pattern(pattern_index[type], XRootWindow(display_id, i),
	    	    wmrootlist[i], i);
	    }
	}
 	XtFree((char *)wmrootlist);
 }
/* Make it happen immediately*/
XFlush(display_id);
}

int 	background_pattern(pattern_index,root,wmroot,screennum)
unsigned    int	pattern_index;
Window	root;
Window wmroot;
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Makes the calls necearry to set the background display pattern
**	to one of the patterns selected from the pattern pallet.
**
**  FORMAL PARAMETERS:
**
**	pattern_index - The index into our global pattern array which
**		        contains the bitmap pattern.
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
Pixmap bitmap;
unsigned    int	save_color = 0;

/* Handle the server default */
if (pattern_index == default_pattern)
    {
    /* Set it on both the root and window manager root window */
    XSetWindowBackgroundPixmap(display_id, root, (Pixmap)ParentRelative);
    XSetWindowBackgroundPixmap(display_id, wmroot, (Pixmap)ParentRelative);
    XClearWindow(display_id, wmroot);
    }
else
    {
    /* Create the bitmap and set the background to that pattern */
    bitmap = XCreateBitmapFromData(display_id, root, 
		    pattern_bits[pattern_index],
		    gray_width, gray_height);
    SetBackgroundToBitmap(bitmap, gray_width, gray_height, root, wmroot,
		screennum, &save_color);
    }
}
  

int	background_solid(colordata,root,wmroot,screennum)
struct	color_data  *colordata;
Window	root;
Window	wmroot;
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Set the background to a solid pattern in the given
**	color.
**
**  FORMAL PARAMETERS:
**
**	A pointer to the color to be used in the solid pattern
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
unsigned    long    pixel;
unsigned    int	save_color = 0;
unsigned    int	status;

/* Convert the color to a pixel value */
status = ColorToPixel(colordata, &save_color, screennum, &pixel);
if (status != 0)
	{
	/* Save this pixel value and free the old one */
	free_and_save(&pixel, NULL, screennum);
	/* Set the pattern on both the root and window manager root */
	XSetWindowBackground(display_id, root, pixel);
	XSetWindowBackground(display_id, wmroot, pixel);
	XClearWindow(display_id, wmroot);
	}
}

int ColorToPixel(colordata, save_colors, screennum, pixel)
struct color_data    *colordata;
unsigned    int	*save_colors;
int screennum;
unsigned    long    *pixel;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a pixel value representing the current color.
**
**  FORMAL PARAMETERS:
**
**	colordata - A pointer to the color structure which will contain
**		    either an RGB or a named color
**	save_colors - Set to one if we allocated a color other than
**		      black or white.
**	pixel - A pointer to a long to store the allocated pixel value
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
    XColor scolor, ecolor;

/* color is specified by name.  Get a color cell */
if (colordata->by_name)
    {
    if (!colordata->name || !*colordata->name)
	return 0;
    if (!XAllocNamedColor(display_id, DefaultColormap(display_id, screennum), 
			colordata->name,
			  &scolor, &ecolor)) 
	{
	return(0);
	}

    }
else
    /* Color is specified by rgb, allocate it that way */
    {
    bcopy(&colordata->color, &ecolor, sizeof(XColor));
    if (!XAllocColor(display_id, DefaultColormap(display_id, screennum), &ecolor))
	{
	return(0);
	}
    }

/* If we are on a monochrome system and the color is black or white, then
   don't bother remembering that we allocated the color.  Otherwise,
   we need to remember that we allocated the color so that we can
   free it later */
if ((ecolor.pixel != BlackPixel(display_id, screennum)) &&
	(ecolor.pixel != WhitePixel(display_id, screennum)) &&
	(DefaultVisual(display_id, screennum)->class & 1))
	    *save_colors = 1;
/* Return the allocated pixel value */
*pixel = ecolor.pixel;
return(1);
}

Window WmRootWindow (dpy, root)
    Display *dpy;
    Window root; 
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Find a window which is the same size as the root window which
**	is passed in.
**
**  FORMAL PARAMETERS:
**
**	dpy - the display connection
**	root - The window that gives the size we are looking for.
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
    Window parent;
    Window *child;
    unsigned int nchildren;
    XWindowAttributes rootatt, childatt;
    Window  newroot;

    /* Get the size of the root which is passed in */
    if (!XGetWindowAttributes (dpy, root, &rootatt)) 
	{
	return(0);
	}

    /* Look for all the children of that root and get their attributes */
    if (XQueryTree (dpy, root, &newroot, &parent, &child, &nchildren)) 
	{
	int i,j;
	for (i = 0; i < nchildren; i++) 
	    {
	    if (!XGetWindowAttributes (dpy, child[i], &childatt)) 
		{
		XFree((char *)child);
		return(0);
		}
	    /* If it is the same width/height as parent, return it */
	    if ((rootatt.width == childatt.width) &&
		(childatt.map_state == IsViewable) &&
		(rootatt.height == childatt.height))
		{
		newroot = child[i];
		XFree((char *)child);
		}
	    }
	    /* There weren't any children who were the same size, we must
               have the pseudo root, so return it.  New root will be the
	       root of the window we were passed as a parameter */
	    XFree((char *)child);
	    return newroot;
	} 
     else 
	{
	return(0);
	}
}

SetBackgroundToBitmap(bitmap, width, height, root, wmroot, screennum, save_colors)
    Pixmap bitmap;
    unsigned int width, height;
    Window root;
    Window wmroot;
    int	screennum;
    unsigned	int *save_colors;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the background to a particular pattern 
**
**  FORMAL PARAMETERS:
**
**	bitmap - The pattern to set the background to
**	width - The width of the pixmap
**	height - The height of the pixmap
**	save_colors - Set to one if we allocated some colors
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
    Pixmap pix;
    GC gc;
    XGCValues gc_init;
    unsigned	int status;

    /* Get the current foreground color */
    status  = ColorToPixel(&windowsetup.screen_foreground, save_colors, 
	    screennum, &gc_init.foreground);
    if (status == 0) return;

    /* Get the current background color */
    status  = ColorToPixel(&windowsetup.screen_background, save_colors, 
	    screennum, &gc_init.background);
    if (status == 0) return;

    /* Free old colors and save new colors */
    free_and_save(&gc_init.background, &gc_init.foreground, screennum);

    /* Create a GC and create a pixmap.  Then put the bit pattern passed
       in into the pixmap using the colors from the GC. */
    gc = XCreateGC(display_id, root, GCForeground|GCBackground, &gc_init);
    pix = XCreatePixmap(display_id, root, width, height,
			(unsigned int)DefaultDepth(display_id, screennum));
    XCopyPlane(display_id, bitmap, pix, gc, 0, 0, width, height, 0, 0, 
				(unsigned long)1);

    /* Set the root and window manager pseudo root to be that pixmap */
    XSetWindowBackgroundPixmap(display_id, root, pix);
    XSetWindowBackgroundPixmap(display_id, wmroot, pix);

    /* Free all the data we allocated */
    XFreeGC(display_id, gc);
    XFreePixmap(display_id, bitmap);
    XFreePixmap(display_id, pix);
    XClearWindow(display_id, wmroot);
}

int	free_and_save(firstcolor,secondcolor, screennum)
unsigned long	*firstcolor;
unsigned long	*secondcolor;
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Free old colors and save new pixel values in static global
**	variables.
**
**  FORMAL PARAMETERS:
**
**	firstcolor - One color to save
**	secondcolor - Second color to save
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
Visual  *type;
Screen  *screen;

screen = XScreenOfDisplay(display_id,screennum);
type = XDefaultVisualOfScreen(screen);

/* If we had an old color allocated.  Then free it.  Note that we can't
   free colors on a TRUECOLOR workstation.  The server gives an error */
if (color1_allocated[screennum])
    {
    if (!is_static(type->class))
	{
	XFreeColors(display_id, XDefaultColormap(display_id, screennum), 
		    &color_1[screennum], (int) 1, (unsigned long) 0);
	}
    }

/* If we had an old color allocated.  Then free it.  Note that we can't
   free colors on a TRUECOLOR workstation.  The server gives an error */
if (color2_allocated[screennum])
    {
    if (!is_static(type->class))
	{
	XFreeColors(display_id, XDefaultColormap(display_id, screennum), 
	&color_2[screennum], (int) 1, (unsigned long) 0);
	}
    }

/* Now save the first color and mark the flag as allocated */
color_1[screennum] = 0;
color1_allocated[screennum] = 0;
if (firstcolor != NULL)
    {
    color_1[screennum] = *firstcolor;
    color1_allocated[screennum] = 1;
    }

/* Now save the second color and mark the flag as allocated */
color_2[screennum] = 0L;
color2_allocated[screennum] = 0;
if (secondcolor != NULL)
    {
    color_2[screennum] = *secondcolor;
    color2_allocated[screennum] = 1;
    }
}
