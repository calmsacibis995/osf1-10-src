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
/* $XConsortium: XQuTextExt.c,v 11.18 91/01/06 11:47:37 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986, 1987	*/

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

#define NEED_REPLIES
#include "Xlibint.h"

#if NeedFunctionPrototypes
XQueryTextExtents (
    register Display *dpy,
    Font fid,
    register _Xconst char *string,
    register int nchars,
    int *dir,
    int *font_ascent,
    int *font_descent,
    register XCharStruct *overall)
#else
XQueryTextExtents (dpy, fid, string, nchars, dir, font_ascent, font_descent,
                   overall)
    register Display *dpy;
    Font fid;
    register char *string;
    register int nchars;
    int *dir;
    int *font_ascent, *font_descent;
    register XCharStruct *overall;
#endif
{
    register int i;
    register char *ptr;
    char *buf;
    xQueryTextExtentsReply rep;
    long nbytes;
    register xQueryTextExtentsReq *req;

    LockDisplay(dpy);
    nbytes = nchars << 1;
    if (! (buf = _XAllocScratch (dpy, (unsigned long) nbytes))) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    GetReq(QueryTextExtents, req);
    req->fid = fid;
    req->length += (nbytes + 3)>>2;
    req->oddLength = nchars & 1;
    for (ptr = buf, i = nchars; --i >= 0;) {
	*ptr++ = 0;
	*ptr++ = *string++;
    }
    Data (dpy, buf, nbytes);
    if (!_XReply (dpy, (xReply *)&rep, 0, xTrue)) {
        UnlockDisplay(dpy);
	SyncHandle();
	return (0);
    }
    *dir = rep.drawDirection;
    *font_ascent = cvtINT16toInt (rep.fontAscent);
    *font_descent = cvtINT16toInt (rep.fontDescent);
    overall->ascent = (short) cvtINT16toShort (rep.overallAscent);
    overall->descent = (short) cvtINT16toShort (rep.overallDescent);
    /* XXX bogus - we're throwing away information!!! */
    overall->width  = (short) cvtINT32toInt (rep.overallWidth);
    overall->lbearing = (short) cvtINT32toInt (rep.overallLeft);
    overall->rbearing = (short) cvtINT32toInt (rep.overallRight);
    UnlockDisplay(dpy);
    SyncHandle();
    return (1);
}
