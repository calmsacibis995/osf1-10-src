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
/* $XConsortium: XAllCells.c,v 11.19 91/01/06 11:44:03 rws Exp $ */
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

Status XAllocColorCells(dpy, cmap, contig, masks, nplanes, pixels, ncolors)
register Display *dpy;
Colormap cmap;
Bool contig;
unsigned int ncolors; /* CARD16 */
unsigned int nplanes; /* CARD16 */
unsigned long *masks; /* LISTofCARD32 */ /* RETURN */
unsigned long *pixels; /* LISTofCARD32 */ /* RETURN */
{

    Status status;
    xAllocColorCellsReply rep;
    register xAllocColorCellsReq *req;
    LockDisplay(dpy);
    GetReq(AllocColorCells, req);

    req->cmap = cmap;
    req->colors = ncolors;
    req->planes = nplanes;
    req->contiguous = contig;

    status = _XReply(dpy, (xReply *)&rep, 0, xFalse);

    if (status) {
#if ( defined (__osf__) && defined(__alpha) )
        {
	    /* sizeof(long) != sizeof(CARD32) */
	    int i;
	    CARD32 value;
	    for ( i = 0; i < rep.nPixels; i++ )
	    {
	        _XRead32 (dpy, &value, 4);
	        pixels[i] = (long) value;
	    }
	    for ( i = 0; i < rep.nMasks; i++ )
	    {
	        _XRead32 (dpy, &value, 4);
	        masks[i] = (long) value;
	    }
        }
#else
        {
	    _XRead32 (dpy, (long *) pixels, 4L * (long) (rep.nPixels));
	    _XRead32 (dpy, (long *) masks, 4L * (long) (rep.nMasks));
        }
#endif
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return(status);
}
