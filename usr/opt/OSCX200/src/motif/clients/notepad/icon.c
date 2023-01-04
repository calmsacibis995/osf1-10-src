/* #module icon.c "V1-002" */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      Notepad -- DECwindows simple out-of-the-box editor.
**
**  AUTHOR:
**
**      Joel Gringorten  - November, 1987
**
**  ABSTRACT:
**
**      This module contains the icon bitmap.
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**
**
**      V1-001  JMG0001         Joel Gringorten            23-Oct-1987
**              Initial version.
**
**--
*/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader = "$Header: [icon.c,v 1.3 91/08/17 05:58:52 rmurphy Exp ]$";
#endif		/* BuildSystemHeader */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/DECWmHints.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "notepad.h"
#include "iconbit.h"

static int icon_sizes[] = {17, 32, 50, 75};
static char *icon_bits[] = {notepad_17_bits,
			    notepad_32_bits,
			    notepad_50_bits,
			    notepad_75_bits};
static Atom dwm_atom;

static Boolean IsXUIWMRunning (widget)
Widget widget;
{
    typedef unsigned long int INT32;
    typedef struct {
	INT32 title_font;
	INT32 icon_font;
	INT32 border_width;
	INT32 title_height;
	INT32 non_title_width;
	INT32 icon_name_width;
	INT32 iconify_width;
	INT32 iconify_height;
    } internalDecorationGeometryRec, *internalDecorationGeometry;

#define WmNumDecorationGeometryElements \
    (sizeof(internalDecorationGeometryRec)/sizeof(INT32))

    typedef struct {
	INT32 value_mask;
	INT32 iconify_pixmap;
	INT32 icon_box_x;
	INT32 icon_box_y;
	INT32 tiled;
	INT32 sticky;
	INT32 no_iconify_button;
	INT32 no_lower_button;
	INT32 no_resize_button;
    } internalDECWmHintsRec, *internalDECWmHints;

#define WmNumDECWmHintsElements (sizeof(internalDECWmHintsRec) / sizeof(INT32))

    static int			result = False;
    Screen			*scrn = XtScreen(widget);
    Display			*dpy = XtDisplay(widget);
    /*
     * variables for XGetWindowProperty calls.
     */
    static Atom	dec_geom = None;
    static Atom dec_hints = None;

    internalDecorationGeometry 	prop = 0;
    internalDECWmHints		hprop = 0;
    Atom			actual_type;
    int				actual_format;
    unsigned long		leftover;
    unsigned long		nitems;

    /*
     * Once we get this atom, we don't need to refetch it. Atoms are never
     * destroyed without restarting the server.
     */
    if (dec_geom == None)
	dec_geom = XmInternAtom(dpy, "DEC_WM_DECORATION_GEOMETRY", True);

    /*
     * If still None, then XUI WM can not have run yet.
     */
    if (dec_geom == None) {
	result = False;
	return (result);
    }

    /*
     * Check for undocumented property name to find out if the
     * XUI WM has been run on this server. Of course, this test doesn't
     * tell you if the window manager is still running - So, there is
     * room for improvement here.
     */

    XGetWindowProperty( dpy, RootWindowOfScreen(scrn),
			dec_geom, 0L, (long)WmNumDecorationGeometryElements,
			False, dec_geom, &actual_type, &actual_format,
			&nitems, &leftover, (unsigned char **) &prop);

    if (prop != 0) XFree((char *) prop);

    /* Check to see if a property with the given name exists.
     * The XUI WM is the only client that we know
     * about that sets a property with this name.  Therefore,
     * if the property exists, we assume that the XUI WM is running.
     */

    if ((actual_type != dec_geom) ||
	(nitems < WmNumDecorationGeometryElements) ||
	(actual_format != 32)) {
	result = False;
	return (result);
    }
    /*
     * If it hasn't bailed out yet, we still are not sure.
     * We now do a check for a different property.  This one will be on
     * our shell if the XUI WM has run since this application started. Once
     * it is there, it doesn't go away (as far as I know). Therefore, if
     * the previous result was True, we know that it will still be True and
     * we can skip this check.
     */
/* This doesn't seem to work... */
#if 0
    if (reuslt) return (result);
    else {
	/*
         * Check for cached atom.
	 */
	if (dec_hints == None) {
	    dec_hints = XmInternAtom(dpy, "DEC_WM_HINTS", True);
	}
	/*
	 * Still not there, we're outta here.
	 */
	if (dec_hints == None) {
	    result = False;
	    return (result);
	}

	/*
	 * Try to get the DEC WM HINTS property.
	 */
	XGetWindowProperty(dpy, XtWindow(widget), dec_hints, 0L,
			   (long) WmNumDECWmHintsElements, False,
			   dec_hints, &actual_type, &actual_format,
			   &nitems, &leftover, (unsigned char **) &hprop);
	/*
	 * Free up the actual property, we're not really interested in it,
	 * just it's existence.
	 */
	if (hprop) XFree((char *) hprop);

	/*
	 * Check whether we got it
	 */
	if ((actual_type != dec_hints) ||
	    (nitems < WmNumDECWmHintsElements) ||
	    (actual_format != 32)) {
	    result = False;
	    return (result);
	}
    }
#endif
    result = True;
    return (result);
}

static void get_icon_size(w, rheight, rwidth)
Widget w;
int *rheight, *rwidth;
{
    XIconSize icon_size_hints, *i_sizes;
    int numsizes, currsize;
    int height, width;

    width = 64;
    height = 64;
    currsize = 0;

    if (XGetIconSizes(XtDisplay(w), XRootWindowOfScreen(XtScreen(w)),
		&i_sizes, &numsizes)) {
	int i = 1;
	/*
	 * Look for largest allowable icon size
	 */
	while (i < numsizes) {
	    if (i_sizes[i].max_width >= i_sizes[currsize].max_width &&
	        i_sizes[i].max_height >= i_sizes[currsize].max_height) {
		currsize = i;
	    }
	}
	if (i_sizes[currsize].max_width <= 0 ||
	    i_sizes[currsize].max_height <= 0) {
	    *rwidth = width;
	    *rheight = height;
	} else {
	    *rwidth = i_sizes[currsize].max_width;
	    *rheight = i_sizes[currsize].max_height;
	}
	XFree((char *)i_sizes);
    } else {
	*rwidth = width;
	*rheight = height;
    }
}

static void SetIconify(w, closure, event)
Widget w;
caddr_t closure;
XEvent *event;
{
    DECWmHintsRec   dwm_hint;

    if (event->type == ReparentNotify || event->type == MapNotify) {
	dwm_hint.value_mask = DECWmIconifyPixmapMask;
	dwm_hint.iconify_pixmap = iconifyPixmap;
	XChangeProperty(XtDisplay(toplevel), XtWindow(toplevel),
			dwm_atom, dwm_atom, 32, PropModeReplace, 
			(unsigned char *)&dwm_hint, 9);
	XtRemoveEventHandler
	(
	    toplevel,
	    StructureNotifyMask,
	    False,
	    (XtEventHandler)SetIconify,
	    NULL
	);
    }
}
static void WindowMapped(w, closure, event)
Widget w;
caddr_t closure;
XEvent *event;
{
    if (event->type == MapNotify) {
	XtRemoveEventHandler
	(
	    toplevel,
	    StructureNotifyMask,
	    False,
	    (XtEventHandler)WindowMapped,
	    NULL
	);
        XtAddEventHandler
	(
	    toplevel,
	    StructureNotifyMask,
	    False,
	    (XtEventHandler)SetIconify,
	    NULL
	);
    }
}
static void SetIconifyIcon()
{

    /*
     * Check for XUI window manager
     */
    if (!IsXUIWMRunning (toplevel)) return;

    /*
     * Look for dxwm and do iconify pixmap if possible
     */

    dwm_atom = XmInternAtom (XtDisplay(toplevel), "DEC_WM_HINTS", True);
    if (dwm_atom != None) {
	iconifyPixmap =  XCreateBitmapFromData(
				CurDpy, DefaultRootWindow(CurDpy),
                                icon_bits[0],
                                (Dimension) icon_sizes[0],
                                (Dimension) icon_sizes[0]);
	XtAddEventHandler
	(
	    toplevel,
	    StructureNotifyMask,
	    False,
	    (XtEventHandler)WindowMapped,
	    NULL
	);
    }
}
void IconInit()
{
    int	    t_height, t_width;
    int	    height, width;
    int	    size_index;

    /*
     * Get the maximum supported icon size
     */

    get_icon_size(toplevel, &t_height, &t_width);    
    for (size_index = XtNumber(icon_sizes) - 1; size_index > 0; size_index--) {
	if ((t_height >= icon_sizes[size_index]) &&
	    (t_width  >= icon_sizes[size_index])) break;
    }

    height = width = icon_sizes[size_index];
    NotepadPixmap = XCreateBitmapFromData(
			CurDpy, DefaultRootWindow(CurDpy),
			icon_bits[size_index],
			icon_sizes[size_index],
			icon_sizes[size_index]);
    SetIconifyIcon();
}
