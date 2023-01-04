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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "X.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "servermd.h"

void
xfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer	pdstLine;
{
    if (pDrawable->depth == 8) {
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
    } else {
	cfb32GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
    }
}

void
xfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    unsigned int	*pdstStart;	/* where to put the bits */
{
    if (pDrawable->depth == 8) {
	cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
    } else {
	cfb32GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
    }
}

Bool
xfbCreateGC(pGC)
    register GCPtr      pGC;
{
    switch (pGC->depth) {
    case 1:
	return (mfbCreateGC(pGC));
    case 8:
	return (cfbCreateGC(pGC));
    case 24:
	return (cfb32CreateGC(pGC));
    default:
	ErrorF("xfbCreateGC: unsupported depth: %d\n", pGC->depth);
	return FALSE;
    }
}

PixmapPtr
xfbCreatePixmap(pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register PixmapPtr pPixmap;
    int size;

    if (depth != 1 && depth != 8 && depth != 24)
	return NullPixmap;

    /* Big pixmaps are pretty useless and will screw up server.  Probably we
       got such large numbers because someone passed in a negative height by
       accident, anyway. */
    if (width > 32767 || height > 32767)
	return NullPixmap;

    size = PixmapBytePad(width, depth);
    pPixmap = (PixmapPtr)xalloc(sizeof(PixmapRec) + (height * size));
    if (!pPixmap)
	return NullPixmap;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.class = 0;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.bitsPerPixel = BitsPerPixel(depth);
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;
    pPixmap->devKind = size;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = (pointer)(pPixmap + 1);
    return pPixmap;
}

Bool
xfbDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{

    if(--pPixmap->refcnt)
	return TRUE;
    xfree(pPixmap);
    return TRUE;
}
