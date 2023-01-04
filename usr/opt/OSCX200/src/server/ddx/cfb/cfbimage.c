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
/* $XConsortium: cfbimage.c,v 1.9 91/12/19 18:36:36 keith Exp $ */

#include "X.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "servermd.h"

extern void mfbGetImage();

void
cfbPutImage(pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		depth, x, y, w, h;
    int		leftPad;
    unsigned int format;
    char 	*pImage;
{
    PixmapRec	FakePixmap;
    int		bitsPerPixel;

    if ((w == 0) || (h == 0))
	return;

    if (format != XYPixmap)
    {
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDraw->pScreen;
    	FakePixmap.drawable.depth = depth;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w+leftPad;
    	FakePixmap.drawable.height = h;
	if (format == XYBitmap)
	{
	    FakePixmap.drawable.bitsPerPixel = 1;
	    FakePixmap.devKind = BitmapBytePad(FakePixmap.drawable.width);
	}
	else
	{
	    FakePixmap.drawable.bitsPerPixel = BitsPerPixel(depth);
	    FakePixmap.devKind = 
		PixmapBytePad(FakePixmap.drawable.width, depth);
	}
    	FakePixmap.refcnt = 1;
        FakePixmap.devPrivate.ptr = (pointer)pImage;

    	((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = FALSE;
	if (format == ZPixmap)
	    (void)(*pGC->ops->CopyArea)(&FakePixmap, pDraw, pGC, leftPad, 0,
					w, h, x, y);
	else
	    (void)(*pGC->ops->CopyPlane)(&FakePixmap, pDraw, pGC, leftPad, 0,
					 w, h, x, y, 1L);
	((cfbPrivGC*)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->fExpose = TRUE;

    }
    else
    {
	unsigned long	oldFg, oldBg, gcv[3];
	unsigned long	oldPlanemask;
	unsigned long	i;
	long		bytesPer;

	depth = pGC->depth;
	oldPlanemask = pGC->planemask;
	oldFg = pGC->fgPixel;
	oldBg = pGC->bgPixel;
	gcv[0] = ~0L;
	gcv[1] = 0L;
	DoChangeGC(pGC, GCForeground | GCBackground, gcv, 0);
	bytesPer = (long)h * BitmapBytePad(w + leftPad);

	for (i = 1 << (depth-1); i != 0; i >>= 1, pImage += bytesPer)
	{
	    if (i & oldPlanemask)
	    {
	        gcv[0] = i;
	        DoChangeGC(pGC, GCPlaneMask, gcv, 0);
	        ValidateGC(pDraw, pGC);
	        (*pGC->ops->PutImage)(pDraw, pGC, 1, x, y, w, h, leftPad,
			         XYBitmap, pImage);
	    }
	}
	gcv[0] = oldPlanemask;
	gcv[1] = oldFg;
	gcv[2] = oldBg;
	DoChangeGC(pGC, GCPlaneMask | GCForeground | GCBackground, gcv, 0);
    }
}

void
cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer	pdstLine;
{
    PixmapRec	FakePixmap;
    BoxRec box;
    DDXPointRec ptSrc;
    RegionRec rgnDst;

    if ((w == 0) || (h == 0))
	return;
    if (pDrawable->bitsPerPixel == 1)
    {
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	return;
    }
    if (format == ZPixmap)
    {
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDrawable->pScreen;
    	FakePixmap.drawable.depth = pDrawable->depth;
    	FakePixmap.drawable.bitsPerPixel = pDrawable->bitsPerPixel;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w;
    	FakePixmap.drawable.height = h;
    	FakePixmap.devKind = PixmapBytePad(w, pDrawable->depth);
    	FakePixmap.refcnt = 1;

        FakePixmap.devPrivate.ptr = (pointer)pdstLine;

	if ((planeMask & PMSK) != PMSK)
	    bzero((char *)FakePixmap.devPrivate.ptr, FakePixmap.devKind * h);
        ptSrc.x = sx + pDrawable->x;
        ptSrc.y = sy + pDrawable->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        (*pDrawable->pScreen->RegionInit)(&rgnDst, &box, 1);
	cfbDoBitblt(pDrawable, (DrawablePtr)&FakePixmap, GXcopy, &rgnDst,
		    &ptSrc, planeMask);
        (*pDrawable->pScreen->RegionUninit)(&rgnDst);

    }
    else
    {
#if PSZ == 8
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDrawable->pScreen;
    	FakePixmap.drawable.depth = 1;
    	FakePixmap.drawable.bitsPerPixel = 1;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w;
    	FakePixmap.drawable.height = h;
    	FakePixmap.devKind = BitmapBytePad(w);
    	FakePixmap.refcnt = 1;

        FakePixmap.devPrivate.ptr = (pointer)pdstLine;

        ptSrc.x = sx + pDrawable->x;
        ptSrc.y = sy + pDrawable->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        (*pDrawable->pScreen->RegionInit)(&rgnDst, &box, 1);
	cfbCopyImagePlane (pDrawable, (DrawablePtr)&FakePixmap, GXcopy, &rgnDst,
		    &ptSrc, planeMask);
        (*pDrawable->pScreen->RegionUninit)(&rgnDst);

#else
	miGetImage (pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
#endif
    }
}
