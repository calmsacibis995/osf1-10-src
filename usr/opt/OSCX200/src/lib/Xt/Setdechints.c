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
#else
#include	<decw$include/Xlib.h>
#include	<decw$include/Xproto.h>
#include	<decw$include/Xatom.h>
#include	<decw$include/Xutil.h>
#endif /* VMS */
#include "DECWmHints.h"
#include "DECWmHintsP.h"


void
WmSetDecorationGeometry(dpy, atom, w, g)
	Display *dpy;
	Atom atom;
	Window w;
	WmDecorationGeometry g; 
{
internalDecorationGeometryRec prop;

	prop.title_font = g->title_font;
	prop.icon_font = g->icon_font;
	prop.border_width = g->border_width;
	prop.title_height = g->title_height;
	prop.non_title_width = g->non_title_width;
	prop.icon_name_width = g->icon_name_width;
	prop.iconify_width = g->iconify_width;
	prop.iconify_height = g->iconify_height;
	XChangeProperty (dpy, w, atom, atom, 32,
	    PropModeReplace, (unsigned char *) &prop,
	    WmNumDecorationGeometryElements);
}

void
WmSetDECWmHints(dpy, atom, w, wmhints)
	Display *dpy;
	Atom atom;
	Window w;
	DECWmHints wmhints; 
{
internalDECWmHintsRec prop;

	prop.value_mask = wmhints->value_mask;
	prop.iconify_pixmap = wmhints->iconify_pixmap;
	prop.icon_box_x = wmhints->icon_box_x;
	prop.icon_box_y = wmhints->icon_box_y;
	prop.tiled = wmhints->tiled;
	prop.sticky = wmhints->sticky;
	prop.no_iconify_button = wmhints->no_iconify_button;
	prop.no_lower_button = wmhints->no_lower_button;
	prop.no_resize_button = wmhints->no_resize_button;
	XChangeProperty (dpy, w, atom, atom, 32,
	    PropModeReplace, (unsigned char *) &prop, WmNumDECWmHintsElements);
}

void
WmSetIconState(dpy, atom, w, s)
	Display *dpy;
	Atom atom;
	Window w;
	WmIconState s; 
{
internalWmIconStateRec prop;

	prop.state = s->state;
	XChangeProperty (dpy, w, atom, atom, 32,
	    PropModeReplace, (unsigned char *) &prop, WmNumIconStateElements);
}

void
WmSetIconBoxName(dpy, atom, w, name)
    register Display *dpy;
    Atom atom;
    Window w;
    char *name;

{
    XChangeProperty(dpy, w, atom, XA_STRING,
                8, PropModeReplace, (unsigned char *)name, strlen(name));
}

