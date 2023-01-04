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
#ifndef lint
static char rcs_id[] = "@(#)$RCSfile: icon.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:57:37 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */

/* Icon.c - Handle icon pixmaps. */

#include "decxmail.h"
#include <IntrinsicP.h>
/* #include <Convert.h> */
#include <Xm/AtomMgr.h>
/* #include <X11/DECWmHints.h> */
#include "DECWmHints.h"

typedef struct _PixmapInfoRec {
    char *name;
    int width;
    int height;
    char *data;
    Pixmap pixmap;
    Pixmap bitmap;
} PixmapInfoRec, *PixmapInfo;


#include "nomail.bit"
#include "newmail.bit"
#include "gray.bit"
#include "black.bit"
#include "compose.bit"
#include "viewmsg.bit"
#include "pick.bit"
#include "tocbutton.bit"
#include "currentbutton.bit"
#include "unopenedbutton.bit"
#include "shownbutton.bit"

PixmapInfoRec info[] = {
    {"nomail", nomail_32_width, nomail_32_height, (char *) nomail_32_bits, NULL, NULL},
    {"newmail", newmail_32_width, newmail_32_height, (char *) newmail_32_bits, NULL, NULL},
    {"compose", compose_32_width, compose_32_height, (char *) compose_32_bits, NULL, NULL},
    {"viewmsg", viewmsg_32_width, viewmsg_32_height, (char *) viewmsg_32_bits, NULL, NULL},
    {"pick", pick_32_width, pick_32_height, (char *) pick_32_bits, NULL, NULL},
    {"gray", gray_width, gray_height, (char *) gray_bits, NULL, NULL},
    {"black", black_width, black_height, (char *) black_bits, NULL, NULL},
    {"smallnomail", smallnomail_width, smallnomail_height,
	 (char *) smallnomail_bits, NULL, NULL},
    {"smallnewmail", smallnewmail_width, smallnewmail_height,
	 (char *) smallnewmail_bits, NULL, NULL},
    {"smallviewmsg", smallviewmsg_width, smallviewmsg_height,
	 (char *) smallviewmsg_bits, NULL, NULL},
    {"smallcompose", smallcompose_width, smallcompose_height,
	 (char *) smallcompose_bits, NULL, NULL},
    {"smallpick", smallpick_width, smallpick_height,
	 (char *) smallpick_bits, NULL, NULL},
    {"tocbutton", tocbutton_width, tocbutton_height,
	 (char *) tocbutton_bits, NULL, NULL},
    {"currentbutton", currentbutton_width, currentbutton_height,
	 (char *) currentbutton_bits, NULL, NULL},
    {"unopenedbutton", unopenedbutton_width, unopenedbutton_height,
	 (char *) unopenedbutton_bits, NULL, NULL},
    {"shownbutton", shownbutton_width, shownbutton_height,
	 (char *) shownbutton_bits, NULL, NULL},
};

static int icon_sizes[] = {16, 32, 50, 75};
static char *nomail_icon_bits[] = {smallnomail_bits,
			    nomail_32_bits,
			    nomail_50_bits,
			    nomail_75_bits};
static char *newmail_icon_bits[] = {smallnewmail_bits,
			    newmail_32_bits,
			    newmail_50_bits,
			    newmail_75_bits};
static char *compose_icon_bits[] = {smallcompose_bits,
			    compose_32_bits,
			    compose_50_bits,
			    compose_75_bits};
static char *viewmsg_icon_bits[] = {smallviewmsg_bits,
			    viewmsg_32_bits,
			    viewmsg_50_bits,
			    viewmsg_75_bits};
static char *pick_icon_bits[] = {smallpick_bits,
			    pick_32_bits,
			    pick_50_bits,
			    pick_75_bits};
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

    internalDecorationGeometry 	prop = 0;
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

    result = True;
    return (result);
}

static void get_icon_size(w, rheight, rwidth)
Widget w;
int *rheight, *rwidth;
{
    XIconSize *i_sizes;
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
    Pixmap	    iconifyPixmap = (Pixmap) closure;

    if (event->type == ReparentNotify || event->type == MapNotify) {
	dwm_hint.value_mask = DECWmIconifyPixmapMask;
	dwm_hint.iconify_pixmap = iconifyPixmap;
	XChangeProperty(XtDisplay(w), XtWindow(w),
			dwm_atom, dwm_atom, 32, PropModeReplace, 
			(unsigned char *)&dwm_hint, 9);
	XtRemoveEventHandler(w, StructureNotifyMask, False,
			    (XtEventHandler) SetIconify, closure);
    }
}
static void WindowMapped(w, closure, event)
Widget w;
caddr_t closure;
XEvent *event;
{
    if (event->type == ReparentNotify || event->type == MapNotify) {
	XtRemoveEventHandler(w, StructureNotifyMask, False,
				(XtEventHandler)WindowMapped, closure);
        XtAddEventHandler(w, StructureNotifyMask, False,
				(XtEventHandler)SetIconify, closure);
    }
}
void SetIconifyIcon(w, iconifyPixmap)
Widget w;
Pixmap iconifyPixmap;
{

    if (iconifyPixmap == (Pixmap) NULL) return;
    /*
     * Check for XUI window manager
     */
    if (!IsXUIWMRunning (w)) return;

    /*
     * Look for dxwm and do iconify pixmap if possible
     */

    dwm_atom = XmInternAtom (XtDisplay(w), "DEC_WM_HINTS", True);
    if (dwm_atom != None) {
	XtAddEventHandler(w, StructureNotifyMask, False,
			 (XtEventHandler)WindowMapped, (caddr_t) iconifyPixmap);
    }
}

static void CvtStringToPixmap(args, num_args, fromVal, toVal)
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
{
    Screen *screen;
    char *from = (char *) fromVal->addr;
    register int i;
    if (*num_args != 1)
	XtErrorMsg("cvtStringToPixmap", "wrongParameters", "decxmailError",
		   "String to pixmap conversion needs screen argument",
		   NULL, NULL);
    screen = *((Screen **) args[0].addr);
    for (i=0 ; i<XtNumber(info) ; i++) {
	if (strcmp(from, info[i].name) == 0) {
	    if (debug && info[i].pixmap)
		XtWarningMsg("alreadyConverted", "icon", "decxmailError",
			     "Already converted this string to a pixmap!",
			     NULL, NULL);
	    info[i].pixmap = XCreatePixmapFromBitmapData(
					DisplayOfScreen(screen),
					RootWindowOfScreen(screen),
					info[i].data,
					(Dimension) info[i].width,
					(Dimension) info[i].height,
					BlackPixelOfScreen(screen),
					WhitePixelOfScreen(screen),
					DefaultDepthOfScreen(screen));
	    toVal->size = sizeof(Pixmap);
	    toVal->addr = (caddr_t) &(info[i].pixmap);
	    return;
	}
    }
    XtStringConversionWarning(from, "Pixmap");
}

static void CvtStringToBitmap(args, num_args, fromVal, toVal)
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
{
    Screen *screen;
    char *from = (char *) fromVal->addr;
    register int i;
    if (*num_args != 1)
	XtErrorMsg("cvtStringToBitmap", "wrongParameters", "decxmailError",
		   "String to bitmap needs screen argument",
		   NULL, NULL);
    screen = *((Screen **) args[0].addr);
    for (i=0 ; i<XtNumber(info) ; i++) {
	if (strcmp(from, info[i].name) == 0) {
	    if (debug && info[i].bitmap)
		XtWarningMsg("alreadyConverted", "icon", "decxmailError",
			     "Already converted this string to a bitmap!",
			     NULL, NULL);
	    info[i].bitmap = XCreateBitmapFromData(
					DisplayOfScreen(screen),
					RootWindowOfScreen(screen),
					info[i].data,
					(Dimension) info[i].width,
					(Dimension) info[i].height);
	    toVal->size = sizeof(Pixmap);
	    toVal->addr = (caddr_t) &(info[i].bitmap);
	    return;
	}
    }
    XtStringConversionWarning(from, "Bitmap");
}

Pixmap GetPixmapNamed(str)
char *str;
{
    XrmValue from, to;

/* PJS - PMAX Doesn't like this NULL dereference: nor do I... */
    if (str == NULL) return NULL;
    from.addr = str;
    from.size = strlen(str) + 1;
    XtConvert(toplevel, XtRString, &from, RMailPixmap, &to);
    if (to.addr == NULL) return NULL;
    return *((Pixmap *) to.addr);
}


Pixmap GetBitmapNamed(str)
char *str;
{
    XrmValue from, to;

    if (str == NULL) return NULL;
    from.addr = str;
    from.size = strlen(str) + 1;
    XtConvert(toplevel, XtRString, &from, RMailBitmap, &to);
    if (to.addr == NULL) return NULL;
    return *((Pixmap *) to.addr);
}


void IconInit()
{
    static XtConvertArgRec screenConvertArg[] = {
      {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
    };
    int	    t_height, t_width;
    int	    size_index;
    Screen  *screen = DefaultScreenOfDisplay(theDisplay);
    /*
     * Get the maximum supported icon size
     */

    get_icon_size(toplevel, &t_height, &t_width);    
    for (size_index = XtNumber(icon_sizes) - 1; size_index > 0; size_index--) {
	if ((t_height >= icon_sizes[size_index]) &&
	    (t_width  >= icon_sizes[size_index])) break;
    }

    XtAddConverter(XtRString, RMailPixmap, CvtStringToPixmap,
		   screenConvertArg, XtNumber(screenConvertArg));
    
    XtAddConverter(XtRString, RMailBitmap, CvtStringToBitmap,
		   screenConvertArg, XtNumber(screenConvertArg));
    

    info[0].width = info[0].height = icon_sizes[size_index];
    info[0].data = nomail_icon_bits[size_index];
    info[1].width = info[1].height = icon_sizes[size_index];
    info[1].data = newmail_icon_bits[size_index];
    info[2].width = info[2].height = icon_sizes[size_index];
    info[2].data = compose_icon_bits[size_index];
    info[3].width = info[3].height = icon_sizes[size_index];
    info[3].data = viewmsg_icon_bits[size_index];
    info[4].width = info[4].height = icon_sizes[size_index];
    info[4].data = pick_icon_bits[size_index];
    NoMailPixmap = XCreateBitmapFromData(theDisplay, DefaultRootWindow(theDisplay),
			      nomail_icon_bits[size_index],
			      (Dimension) icon_sizes[size_index],
			      (Dimension) icon_sizes[size_index]);
    NewMailPixmap =XCreateBitmapFromData(theDisplay, DefaultRootWindow(theDisplay),
			      newmail_icon_bits[size_index],
			      (Dimension) icon_sizes[size_index],
			      (Dimension) icon_sizes[size_index]);
    NoMailSmallPixmap = XCreateBitmapFromData(theDisplay, DefaultRootWindow(theDisplay),
				   smallnomail_bits,
				   (Dimension) smallnomail_width,
				   (Dimension) smallnomail_height);
    NewMailSmallPixmap = XCreateBitmapFromData(theDisplay, DefaultRootWindow(theDisplay),
				   smallnewmail_bits,
				   (Dimension) smallnewmail_width,
				   (Dimension) smallnewmail_height);
    GrayPixmap = XCreatePixmapFromBitmapData(
			    theDisplay, DefaultRootWindow(theDisplay),
			    gray_bits,
			    (Dimension) gray_width,
			    (Dimension) gray_height,
			    BlackPixelOfScreen(screen),
			    WhitePixelOfScreen(screen),
			    DefaultDepthOfScreen(screen));
}
