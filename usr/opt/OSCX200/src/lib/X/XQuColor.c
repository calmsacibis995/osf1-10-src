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
/* $XConsortium: XQuColor.c,v 11.17 91/01/06 11:47:28 rws Exp $ */
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

#define NEED_REPLIES
#include "Xlibint.h"

XQueryColor(dpy, cmap, def)
    register Display *dpy;
    Colormap cmap;
    XColor *def;	/* RETURN */
{
    xrgb color;
    xQueryColorsReply rep;
    register xQueryColorsReq *req;
    unsigned long val = def->pixel;	/* needed for macro below */

    LockDisplay(dpy);
    GetReqExtra(QueryColors, 4, req); /* a pixel (CARD32) is 4 bytes */
    req->cmap = cmap;

    OneDataCard32 (dpy, NEXTPTR(req,xQueryColorsReq), val);

    if (_XReply(dpy, (xReply *) &rep, 0, xFalse) != 0) {

	    _XRead(dpy, (char *)&color, (long) SIZEOF(xrgb));

	    def->red = color.red;
	    def->blue = color.blue;
	    def->green = color.green;
	    def->flags = DoRed | DoGreen | DoBlue;
        }
    UnlockDisplay(dpy);
    SyncHandle();
}
