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
#include <Xm/Xm.h>
static Boolean first = True;

Boolean DXIsXUIWMRunning
#if defined (VAXC) || defined (__STDC__) || defined (CAUGHT_FUNCPROTO)
(Widget widget, Boolean check)
#else
(widget, check)
Widget widget;
Boolean check;
#endif
{
    typedef unsigned long int   INT32;
    typedef struct
    {
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

#define WmNumDECWmHintsElements (sizeof(internalDECWmHintsRec)/sizeof(INT32))

    static int		result = False;
    Screen		*scrn = XtScreen(widget);
    Display		*dpy = XtDisplay(widget);
    /*
    ** Variables for XGetWindowProperty calls.
    */
    static Atom			dec_geom = None;
    static Atom			dec_hints = None;

    internalDecorationGeometry	prop = 0;
    internalDECWmHints		hprop = 0;
    Atom			actual_type;
    int				actual_format;
    unsigned long		leftover;
    unsigned long		nitems;

    if (!first && !check) return(result);
    first = False;
    /*
    ** Once we get this atom, we don't need to refetch it.  Atoms are never
    ** destroyed without restarting the server.
    */
    if (dec_geom == None)
    {
	dec_geom = XmInternAtom(dpy, "DEC_WM_DECORATION_GEOMETRY", True);
    }

    /*
    ** If still None, then XUI WM can not have run yet.
    */
    if (dec_geom == None)
    {
	result = False;
	return (result);
    }

    /*
    ** Check for an undocumented property name to find out if the
    ** XUI WM has been run on this server.  Of course, this test doesn't
    ** tell you if the window manager is still running - So, there is
    ** room for improvment here.
    */
    XGetWindowProperty
    (
	dpy,
	RootWindowOfScreen(scrn),
	dec_geom,
	0L,
	(long)WmNumDecorationGeometryElements,
	False,
	dec_geom,
	&actual_type,
	&actual_format,
	&nitems,
	&leftover,
	(unsigned char **)&prop
    );
    if (prop != 0) XFree ((char *)prop);

    /*
    ** Check to see a property with the given name exists.
    ** The XUI WM is the only client we know
    ** about that sets a property with this name.  Therefore,
    ** if the property exists, we assume the XUI WM is running.
    */
    if ((actual_type != dec_geom) ||
	(nitems < WmNumDecorationGeometryElements) ||
	(actual_format != 32))
    {
	result = False;
	return (result);
    }
    result = True;
    return (result);

}
