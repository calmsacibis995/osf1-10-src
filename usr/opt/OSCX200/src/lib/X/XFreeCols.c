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
/* $XConsortium: XFreeCols.c,v 11.10 91/01/06 11:45:44 rws Exp $ */
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

XFreeColors(dpy, cmap, pixels, npixels, planes)
register Display *dpy;
Colormap cmap;
unsigned long *pixels; /* LISTofCARD32 */
int npixels;
unsigned long planes; /* CARD32 */
{
    register xFreeColorsReq *req;
    register long nbytes;

    LockDisplay(dpy);
    GetReq(FreeColors, req);
    req->cmap = cmap;
    req->planeMask = planes;

    /* on the VAX, each pixel is a 32-bit (unsigned) integer */
    req->length += npixels;

    nbytes = npixels << 2;		/* watch out for macros... */
#if ( defined (__osf__) && defined(__alpha) )
    {
	/* sizeof(CARD32) != sizeof(long) */
	int i;
	CARD32 value;
	for ( i = 0; i < npixels; i++ )
	{
	    value = pixels[i];
	    Data32 (dpy, &value, sizeof(CARD32));
	}
    }
#else
    Data32 (dpy, (long *) pixels, nbytes);
#endif
    UnlockDisplay(dpy);
    SyncHandle();
}

