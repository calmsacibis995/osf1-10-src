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
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/lib/Xt/Getdechints.c,v 1.1.2.4 92/06/10 11:43:06 Dave_Hill Exp $";
#endif /* lint */

/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**	MASSACHUSSETTS INSTITUTE OF TECHNOLOGY, CAMBRIDGE, MASS.	    *
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
**  CORPORATION OR MIT.                                                     *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      < to be supplied >
**
**  ABSTRACT:
**
**      < to be supplied >
**
**  ENVIRONMENT:
**
**      < to be supplied >
**
**  MODIFICATION HISTORY:
**
**      < to be supplied >
**
**--
**/

#ifndef VMS
#include	<X11/Xlib.h>
#include	<X11/Xproto.h>
#include	<X11/Xatom.h>
#include	<X11/Xutil.h>
#include	<X11/Intrinsic.h>
#include	<stdio.h>
#else
#include	<decw$include/Xlib.h>
#include	<decw$include/Xproto.h>
#include	<decw$include/Xatom.h>
#include	<decw$include/Xutil.h>
#include 	<stdio>
#endif /* VMS */
#include "DECWmHints.h"
#include "DECWmHintsP.h"


Status
WmGetDecorationGeometry(dpy, atom, w, g)
	Display *dpy;
	Atom atom;
	Window w;
	WmDecorationGeometry g;
{
	internalDecorationGeometry prop = 0;
        Atom actual_type;
        int actual_format;
        unsigned long leftover;
        unsigned long nitems;
	if (XGetWindowProperty(dpy, w, atom,
	    0L, (long)WmNumDecorationGeometryElements,
	    False, atom, &actual_type, &actual_format,
            &nitems, &leftover, (unsigned char **)&prop)
            != Success) return (0);
	if ((actual_type != atom) ||
             (nitems < WmNumDecorationGeometryElements) ||
	     (actual_format != 32)) {
		if (prop != 0) XFree ((char *)prop);
	    	return(0);
		}
	g->title_font = prop->title_font;
	g->icon_font = prop->icon_font;
	g->border_width = prop->border_width;
	g->title_height = prop->title_height;
	g->non_title_width = prop->non_title_width;
	g->icon_name_width = prop->icon_name_width;
	g->iconify_width = prop->iconify_width;
	g->iconify_height = prop->iconify_height;
	XFree((char *)prop);
	return(1);
}

Status
WmGetDECWmHints(dpy, atom, w, g)
	Display *dpy;
	Atom atom;
	Window w;
	DECWmHints g;
{
internalDECWmHints prop = 0;
Atom actual_type;
int actual_format;
unsigned long leftover;
unsigned long nitems;

	if (XGetWindowProperty(dpy, w, atom,
	    0L, (long)WmNumDECWmHintsElements,
	    False, atom, &actual_type, &actual_format,
            &nitems, &leftover, (unsigned char **)&prop)
            != Success) return (0);
	if ((actual_type != atom) ||
             (nitems < WmNumDECWmHintsElements) || (actual_format != 32)) {
		if (prop != 0) XFree ((char *)prop);
		return(0);
		}
	g->value_mask = prop->value_mask;
	g->iconify_pixmap = prop->iconify_pixmap;
	g->icon_box_x = prop->icon_box_x;
	g->icon_box_y = prop->icon_box_y;
	g->tiled = prop->tiled;
	g->sticky = prop->sticky;
	g->no_iconify_button = prop->no_iconify_button;
	g->no_lower_button = prop->no_lower_button;
	g->no_resize_button = prop->no_resize_button;
	XFree((char *)prop);
	return(1);
}

Status
_XtWmGetIconState(dpy, atom, w, s)
	Display *dpy;
	Atom atom;
	Window w;
	WmIconState s;
{
internalWmIconState prop = 0;
Atom actual_type;
int actual_format;
unsigned long leftover;
unsigned long nitems;

	if (XGetWindowProperty(dpy, w, atom,
	    0L, (long)WmNumIconStateElements,
	    False, atom, &actual_type, &actual_format,
            &nitems, &leftover, (unsigned char **)&prop)
            != Success) return (0);
	if ((actual_type != atom) ||
             (nitems < WmNumIconStateElements) || (actual_format != 32)) {
		if (prop != 0) XFree ((char *)prop);
		return(0);
		}
	s->state = prop->state;
	XFree((char *)prop);
	return(1);
}

Status
WmGetIconBoxName(dpy, atom, w, name)
    register Display *dpy;
    Atom atom;
    Window w;
    char **name;
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;
    unsigned char *data = 0;
    if (XGetWindowProperty(dpy, w, atom, 0L, (long)BUFSIZ,
	False, XA_STRING, &actual_type, &actual_format, &nitems,
	&leftover, &data)
            != Success) {
        *name = 0;
        return(0);
        }
    if ( (actual_type == XA_STRING) &&  (actual_format == 8) ) {
        /* The data returned by XGetWindowProperty is guarranteed to
        contain one extra byte that is null terminated to make retrieveing
        string properties easy. */

        *name = (char *)data;
        return(1);
        }
    if (data) XFree ((char *)data);
    *name = 0;
    return(0);
}
