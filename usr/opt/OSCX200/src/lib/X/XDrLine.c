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
/* $XConsortium: XDrLine.c,v 11.15 91/01/06 11:45:13 rws Exp $ */
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

#define wsize (SIZEOF(xPolySegmentReq) + WLNSPERBATCH * SIZEOF(xSegment))
#define zsize (SIZEOF(xPolySegmentReq) + ZLNSPERBATCH * SIZEOF(xSegment))

XDrawLine (dpy, d, gc, x1, y1, x2, y2)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x1, y1, x2, y2;
{
    register xSegment *segment;
#ifdef MUSTCOPY
    xSegment segmentdata;
    long len = SIZEOF(xSegment);

    segment = &segmentdata;
#endif /* not MUSTCOPY */

    LockDisplay(dpy);
    FlushGC(dpy, gc);

    {
    register xPolySegmentReq *req = (xPolySegmentReq *) dpy->last_req;

    /* if same as previous request, with same drawable, batch requests */
    if (
          (req->reqType == X_PolySegment)
       && (req->drawable == d)
       && (req->gc == gc->gid)
       && ((dpy->bufptr + SIZEOF(xSegment)) <= dpy->bufmax)
       && (((char *)dpy->bufptr - (char *)req) < (gc->values.line_width ?
						  wsize : zsize)) ) {
	 req->length += SIZEOF(xSegment) >> 2;
#ifndef MUSTCOPY
         segment = (xSegment *) dpy->bufptr;
	 dpy->bufptr += SIZEOF(xSegment);
#endif /* not MUSTCOPY */
	 }

    else {
	GetReqExtra (PolySegment, SIZEOF(xSegment), req);
	req->drawable = d;
	req->gc = gc->gid;
#ifdef MUSTCOPY
	dpy->bufptr -= SIZEOF(xSegment);
#else
	segment = (xSegment *) NEXTPTR(req,xPolySegmentReq);
#endif /* MUSTCOPY */
	}

    segment->x1 = x1;
    segment->y1 = y1;
    segment->x2 = x2;
    segment->y2 = y2;

#ifdef MUSTCOPY
    Data (dpy, (char *) &segmentdata, len);
#endif /* MUSTCOPY */

    UnlockDisplay(dpy);
    SyncHandle();
    }
}

