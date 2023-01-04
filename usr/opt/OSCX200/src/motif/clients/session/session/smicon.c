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
#include "sm_icon.h"
#include "sm_iconify.h"
#include <X11/Vendor.h>

int IconInit()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Create each of the icon pixmaps out of the static bit data in
**	the include files.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**	sm_icon_bits - Bits which make up the key icon for the icon box
**	sm_iconify_bits - Bits which are the key icon for the title bar
**	sm_icon_reverse_bits - Reverse video bits for icon box
**	sm_iconify_reverse_bits - Reverse video bits for title bar
**
**  IMPLICIT OUTPUTS:
**
**	smdata.icon - The pixmap for icon box
**	smdata.iconify - The pixmap for the title bar
**	smdata.reverse_icon - The pixmap for the reverse video icon for box
**	smdata.reverse_iconify - The pixmap for the reverse video icon for title
**
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    smdata.icon = XCreatePixmapFromBitmapData(display_id, DefaultRootWindow(display_id),
                              sm_icon_bits,
                              (Dimension) sm_icon_width,
                              (Dimension) sm_icon_height,
			      (unsigned long)BlackPixel(display_id,screen), 
			      (unsigned long)WhitePixel(display_id,screen),
			      1);
    smdata.iconify = XCreatePixmapFromBitmapData(display_id, DefaultRootWindow(display_id),
                              sm_iconify_bits,
                              (Dimension) sm_iconify_width,
                              (Dimension) sm_iconify_height,
			      (unsigned long)BlackPixel(display_id,screen), 
			      (unsigned long)WhitePixel(display_id,screen),
			      (unsigned int)DefaultDepth(display_id,screen));
    smdata.reverse_icon = XCreatePixmapFromBitmapData(display_id, DefaultRootWindow(display_id),
                              sm_icon_reverse_bits,
                              (Dimension) sm_icon_width,
                              (Dimension) sm_icon_height,
			      (unsigned long)BlackPixel(display_id,screen), 
			      (unsigned long)WhitePixel(display_id,screen),
			      (unsigned int)DefaultDepth(display_id,screen));
    smdata.reverse_iconify = XCreatePixmapFromBitmapData(display_id, DefaultRootWindow(display_id),
                              sm_iconify_bits,
                              (Dimension) sm_iconify_width,
                              (Dimension) sm_iconify_height,
			      (unsigned long)WhitePixel(display_id,screen),
			      (unsigned long)BlackPixel(display_id,screen), 
			      (unsigned int)DefaultDepth(display_id,screen));
}

#define WmNormalState 0
#define WmIconicState 1

int property_handler( widget, closure, event )
Widget widget;
caddr_t closure;
XEvent *event;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	A property notify on the top level shell has been delivered.  This
**	will happen for several reasons.  One is that the user has either
**	iconified or de-iconified the session manager window.  We want
**	to keep track of this so that we can reverse video the icons if
**	the session manager is iconified and a new message comes in.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**	icon_state - set to either not_icon or am_icon 
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    XPropertyEvent *event_rcvd;
    Window window;
    Display *display;
    int s;
    Atom state_atom;
    Arg     arglist[3];

    event_rcvd = (XPropertyEvent*)event;

    /* Make sure it is property notify event */
    if ( (event_rcvd->type & 127) != PropertyNotify )
        return 0;
    display = XtDisplay( widget );
    window  = XtWindow(  widget );
    
    /* See if it is the icon state property change */
    state_atom = XInternAtom( display, "DEC_WM_ICON_STATE", FALSE );
    if ( event_rcvd->atom != state_atom )
        return 1;

    /* Get the icon state */
    WmGetIconState( display, state_atom, window, &s);

    switch (s) {
    case WmNormalState:
	{
	/* We have been de-iconified.  Reset the icon if necessary */
        icon_state = not_icon;
	if (icon_type == icon_reverse)
	    {
	    XtSetArg(arglist[0], XtNiconPixmap, smdata.icon);
	    XtSetValues(smdata.toplevel, arglist, 2);
	    icon_type = icon_standard;
	    }
        break;
	}
    case WmIconicState:
	{
	/* We were just made into an icon.  Store that in our global */
	icon_state = am_icon;
        break;
	}
    }
    return 1;
} 


Status WmGetIconState(dpy, atom, w, s)
	Display *dpy;
	Atom atom;
	Window w;
	int *s;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	find out if the session manager is an icon.   We can find
**	out by looking at a property on our window
**
**  FORMAL PARAMETERS:
**
**	dpy - Display connection
**	atom - The name of the property to look for
**	window - The window to get the property off of
**	s - Returns the icon state of the window
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
unsigned    long *prop = 0;
Atom actual_type;
int actual_format;
unsigned long leftover;
unsigned long nitems;

/* Get the property */
if (XGetWindowProperty(dpy, w, atom, 0L, (long)1, False, atom, &actual_type, 
		       &actual_format, &nitems, &leftover, 
			(unsigned char **)&prop) != Success) 
	return (0);

/* Make sure it is the right kind of property and in the right format */
if ((actual_type != atom) || (nitems < 1) || (actual_format != 32)) 
	{
	if (prop != 0) 
	    XFree ((char *)prop);
	return(0);
	}

/* Return the icon state. */
*s = *prop;
XFree((char *)prop);
return(1);
}
