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
/* $XConsortium: XDrArc.c,v 11.15 91/01/06 11:45:10 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/* Note to future maintainers:  XDrawArc does NOT batch successive PolyArc
   requests into a single request like XDrawLine, XDrawPoint, etc.
   We don't do this because X_PolyArc applies the GC's join-style if
   the last point in one arc coincides with the first point in another.
   The client wouldn't expect this and would have no easy way to defeat it. */
   
#include "Xlibint.h"

XDrawArc(dpy, d, gc, x, y, width, height, angle1, angle2)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y; /* INT16 */
    unsigned int width, height; /* CARD16 */
    int angle1, angle2; /* INT16 */
{
    register xPolyArcReq *req;
    register xArc *arc;
#ifdef MUSTCOPY
    xArc arcdata;
    long len = SIZEOF(xArc);

    arc = &arcdata;
#endif /* MUSTCOPY */

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReqExtra (PolyArc, SIZEOF(xArc), req);

    req->drawable = d;
    req->gc = gc->gid;

#ifndef MUSTCOPY
    arc = (xArc *) NEXTPTR(req,xPolyArcReq);
#endif /* MUSTCOPY */

    arc->x = x;
    arc->y = y;
    arc->width = width;
    arc->height = height;
    arc->angle1 = angle1;
    arc->angle2 = angle2;

#ifdef MUSTCOPY
    dpy->bufptr -= SIZEOF(xArc);
    Data (dpy, (char *) arc, len);
#endif /* MUSTCOPY */

    UnlockDisplay(dpy);
    SyncHandle();
}
