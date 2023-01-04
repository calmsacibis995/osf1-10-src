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
/* $XConsortium: ClientWin.c,v 1.3 89/12/10 10:26:35 rws Exp $ */

/* 
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>

static Window TryChildren();

/* Find a window with WM_STATE, else return win itself, as per ICCCM */

Window XmuClientWindow (dpy, win)
    Display *dpy;
    Window win;
{
    Atom WM_STATE;
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data;
    Window inf;

    WM_STATE = XInternAtom(dpy, "WM_STATE", True);
    if (!WM_STATE)
	return win;
    XGetWindowProperty(dpy, win, WM_STATE, 0, 0, False, AnyPropertyType,
		       &type, &format, &nitems, &after, &data);
    if (type)
	return win;
    inf = TryChildren(dpy, win, WM_STATE);
    if (!inf)
	inf = win;
    return inf;
}

static
Window TryChildren (dpy, win, WM_STATE)
    Display *dpy;
    Window win;
    Atom WM_STATE;
{
    Window root, parent;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    Atom type = None;
    int format;
    unsigned long nitems, after;
    unsigned char *data;
    Window inf = 0;

    if (!XQueryTree(dpy, win, &root, &parent, &children, &nchildren))
	return 0;
    for (i = 0; !inf && (i < nchildren); i++) {
	XGetWindowProperty(dpy, children[i], WM_STATE, 0, 0, False,
			   AnyPropertyType, &type, &format, &nitems,
			   &after, &data);
	if (type)
	    inf = children[i];
    }
    for (i = 0; !inf && (i < nchildren); i++)
	inf = TryChildren(dpy, children[i], WM_STATE);
    if (children) XFree((char *)children);
    return inf;
}
