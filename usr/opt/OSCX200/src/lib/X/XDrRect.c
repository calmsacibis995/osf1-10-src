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
/* $XConsortium: XDrRect.c,v 11.15 91/01/06 11:45:18 rws Exp $ */
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

#include "Xlibint.h"

/* precompute the maximum size of batching request allowed */

#define wsize (SIZEOF(xPolyRectangleReq) + WRCTSPERBATCH * SIZEOF(xRectangle))
#define zsize (SIZEOF(xPolyRectangleReq) + ZRCTSPERBATCH * SIZEOF(xRectangle))

XDrawRectangle(dpy, d, gc, x, y, width, height)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y; /* INT16 */
    unsigned int width, height; /* CARD16 */
{
    xRectangle *rect;
#ifdef MUSTCOPY
    xRectangle rectdata;
    long len = SIZEOF(xRectangle);

    rect = &rectdata;
#endif /* MUSTCOPY */

    LockDisplay(dpy);
    FlushGC(dpy, gc);

    {
    register xPolyRectangleReq *req = (xPolyRectangleReq *) dpy->last_req;

    /* if same as previous request, with same drawable, batch requests */
    if (
          (req->reqType == X_PolyRectangle)
       && (req->drawable == d)
       && (req->gc == gc->gid)
       && ((dpy->bufptr + SIZEOF(xRectangle)) <= dpy->bufmax)
       && (((char *)dpy->bufptr - (char *)req) < (gc->values.line_width ?
						  wsize : zsize)) ) {
	 req->length += SIZEOF(xRectangle) >> 2;
#ifndef MUSTCOPY
         rect = (xRectangle *) dpy->bufptr;
	 dpy->bufptr += SIZEOF(xRectangle);
#endif /* not MUSTCOPY */
	 }

    else {
	GetReqExtra(PolyRectangle, SIZEOF(xRectangle), req);
	req->drawable = d;
	req->gc = gc->gid;
#ifdef MUSTCOPY
	dpy->bufptr -= SIZEOF(xRectangle);
#else
	rect = (xRectangle *) NEXTPTR(req,xPolyRectangleReq);
#endif /* MUSTCOPY */
	}

    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;

#ifdef MUSTCOPY
    Data (dpy, (char *) rect, len);	/* subtracted bufptr up above */
#endif /* MUSTCOPY */

    }
    UnlockDisplay(dpy);
    SyncHandle();
}
