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
/*
 * Fill 32 bit stippled rectangles for 8 bit frame buffers
 */
/*
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Keith Packard, MIT X Consortium

*/

/* $XConsortium: cfbrctstp8.c,v 1.14 91/12/19 18:36:29 keith Exp $ */

#if PSZ == 8

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfb8bit.h"

void
cfb8FillRectOpaqueStippled32 (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;	/* number of boxes to fill */
    register BoxPtr pBox;	/* pointer to list of boxes to fill */
{
    unsigned long   *src;
    int stippleHeight;

    int nlwDst;		/* width in longwords of the dest pixmap */
    int w;		/* width of current box */
    register int h;	/* height of current box */
    unsigned long startmask;
    unsigned long endmask;	/* masks for reggedy bits at either end of line */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    register int nlw;			/* loop version of nlwMiddle */
    unsigned long *dstLine;
    register unsigned long *dst;	/* pointer to bits we're writing */
    unsigned long *dstTmp;
    int y;				/* current scan line */

    unsigned long *pbits;/* pointer to start of pixmap */
    register unsigned long bits;	/* bits from stipple */
    int	rot, lastStop, i;
    register unsigned long  xor, and;
    cfbPrivGCPtr	    devPriv;
    PixmapPtr		    stipple;
    int	    wEnd;

    devPriv = ((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr);
    stipple = devPriv->pRotatedPixmap;

    cfb8CheckOpaqueStipple(pGC->alu, pGC->fgPixel, pGC->bgPixel, pGC->planemask);

    stippleHeight = stipple->drawable.height;
    src = (unsigned long *)stipple->devPrivate.ptr;

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pbits)

    while (nBox--)
    {
	w = pBox->x2 - pBox->x1;
	h = pBox->y2 - pBox->y1;
	y = pBox->y1;
	dstLine = pbits + (pBox->y1 * nlwDst) + 
		((pBox->x1 & ~(sizeof(long)-1)) >> PWSH);
	if (((pBox->x1 & PIM) + w) <= PPW)
	{
	    maskpartialbits(pBox->x1, w, startmask);
	    nlwMiddle = 0;
	    endmask = 0;
	}
	else
	{
	    maskbits (pBox->x1, w, startmask, endmask, nlwMiddle);
	}
	rot = (pBox->x1 & ((LONG_BIT-1) & ~(sizeof(long)-1)));
	pBox++;
	y = y % stippleHeight;
	if (cfb8StippleRRop == GXcopy)
	{
	    if (w < LONG_BIT*2)
	    {
	    	while (h--)
	    	{
	    	    bits = src[y];
	    	    y++;
	    	    if (y == stippleHeight)
		    	y = 0;
	    	    if (rot)
		    	RotBitsLeft(bits,rot);
		    dst = dstLine;
	    	    dstLine += nlwDst;
	    	    if (startmask)
	    	    {
		    	*dst = *dst & ~startmask | 
				GetFourPixels (bits) & startmask;
		    	dst++;
		    	RotBitsLeft (bits, LONG_BIT/8);
	    	    }
		    nlw = nlwMiddle;
	    	    while (nlw--)
	    	    {
			*dst++ = GetFourPixels(bits);
		    	RotBitsLeft (bits, LONG_BIT/8);
	    	    }
	    	    if (endmask)
	    	    {
			*dst = *dst & ~endmask | 
			       GetFourPixels (bits) & endmask;
	    	    }
	    	}
	    }
	    else
	    {
		wEnd = 7 - (nlwMiddle & 7);
		nlwMiddle = (nlwMiddle >> 3) + 1;
	    	while (h--)
	    	{
		    bits = src[y];
		    y++;
		    if (y == stippleHeight)
			y = 0;
	    	    if (rot != 0)
			RotBitsLeft (bits, rot);
	    	    dstTmp = dstLine;
	    	    dstLine += nlwDst;
		    if (startmask)
		    {
			*dstTmp = *dstTmp & ~startmask | 
			          GetFourPixels (bits) & startmask;
			dstTmp++;
			RotBitsLeft (bits, LONG_BIT/8);
		    }
		    w = 7 - wEnd;
		    while (w--)
		    {
			nlw = nlwMiddle;
			dst = dstTmp;
			dstTmp++;
			xor = GetFourPixels (bits);
			while (nlw--)
			{
			    *dst = xor;
			    dst += 8;
			}
			NextFourBits (bits);
		    }
		    nlwMiddle--;
		    w = wEnd + 1;
		    if (endmask)
		    {
			dst = dstTmp + (nlwMiddle << 3);
			*dst = (*dst & ~endmask) | 
			       GetFourPixels (bits) & endmask;
		    }
		    while (w--)
		    {
			nlw = nlwMiddle;
			dst = dstTmp;
			dstTmp++;
			xor = GetFourPixels (bits);
			while (nlw--)
			{
			    *dst = xor;
			    dst += 8;
			}
			NextFourBits (bits);
		    }
		    nlwMiddle++;
		}
	    }
	}
	else
	{
	    while (h--)
	    {
	    	bits = src[y];
	    	y++;
	    	if (y == stippleHeight)
		    y = 0;
	    	if (rot)
		    RotBitsLeft(bits,rot);
		dst = dstLine;
	    	dstLine += nlwDst;
	    	if (startmask)
	    	{
		    xor = GetFourBits(bits);
		    *dst = MaskRRopPixels(*dst, xor, startmask);
		    dst++;
		    RotBitsLeft (bits, LONG_BIT/8);
	    	}
		nlw = nlwMiddle;
	    	while (nlw--)
	    	{
		    RRopFourBits(dst, GetFourBits(bits));
		    dst++;
		    RotBitsLeft (bits, LONG_BIT/8);
	    	}
	    	if (endmask)
	    	{
		    xor = GetFourBits(bits);
		    *dst = MaskRRopPixels(*dst, xor, endmask);
	    	}
	    }
	}
    }
}


cfb8FillRectTransparentStippled32 (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;	/* number of boxes to fill */
    BoxPtr 	    pBox;	/* pointer to list of boxes to fill */
{
    int		    x, y, w, h;
    int		    nlwMiddle, nlwDst, nlwTmp;
    unsigned long   startmask, endmask;
    register unsigned long   *dst;
    unsigned long   *dstLine, *pbits, *dstTmp;
    unsigned long   *src;
    register unsigned long   xor;
    register unsigned long   bits, mask;
    int		    rot;
    int		    wEnd;
    cfbPrivGCPtr    devPriv;
    PixmapPtr	    stipple;
    int		    stippleHeight;
    register int    nlw;
    
    devPriv = ((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr);
    stipple = devPriv->pRotatedPixmap;
    src = (unsigned long *)stipple->devPrivate.ptr;
    stippleHeight = stipple->drawable.height;

    cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pbits)

    while (nBox--)
    {
	x = pBox->x1;
    	w = pBox->x2 - x;
	if (((x & PIM) + w) <= PPW)
	{
	    maskpartialbits(x, w, startmask);
	    endmask = 0;
	    nlwMiddle = 0;
	}
	else
	{
	    maskbits (x, w, startmask, endmask, nlwMiddle);
	}
	rot = (x & ((LONG_BIT-1) & ~(sizeof(long)-1)));
    	y = pBox->y1;
    	dstLine = pbits + (y * nlwDst) + (x >> PWSH);
    	h = pBox->y2 - y;
	pBox++;
	y %= stippleHeight;
	if (cfb8StippleRRop == GXcopy)
	{
	    xor = devPriv->xor;
	    if (w < LONG_BIT*2)
	    {
	    	while (h--)
	    	{
		    bits = src[y];
		    y++;
		    if (y == stippleHeight)
			y = 0;
	    	    if (rot != 0)
			RotBitsLeft (bits, rot);
	    	    dst = dstLine;
	    	    dstLine += nlwDst;
	    	    if (startmask)
	    	    {
		    	mask = cfb8PixelMasks[GetFourBits(bits)];
		    	*dst = (*dst & ~(mask & startmask)) | 
			       (xor & (mask & startmask));
		    	dst++;
		    	RotBitsLeft (bits, LONG_BIT/8);
	    	    }
		    nlw = nlwMiddle;
	    	    while (nlw--)
	    	    {
		    	WriteFourBits (dst,xor,GetFourBits(bits))
		    	dst++;
		    	RotBitsLeft (bits, LONG_BIT/8);
	    	    }
	    	    if (endmask)
	    	    {
		    	mask = cfb8PixelMasks[GetFourBits(bits)];
		    	*dst = (*dst & ~(mask & endmask)) | 
			       (xor & (mask & endmask));
	    	    }
	    	}
	    }
	    else
	    {
		wEnd = 7 - (nlwMiddle & 7);
		nlwMiddle = (nlwMiddle >> 3) + 1;
	    	while (h--)
	    	{
		    bits = src[y];
		    y++;
		    if (y == stippleHeight)
			y = 0;
	    	    if (rot != 0)
			RotBitsLeft (bits, rot);
	    	    dstTmp = dstLine;
	    	    dstLine += nlwDst;
		    if (startmask)
		    {
		    	mask = cfb8PixelMasks[GetFourBits(bits)];
		    	*dstTmp = (*dstTmp & ~(mask & startmask)) |
		       	       (xor & (mask & startmask));
		    	dstTmp++;
		    	RotBitsLeft (bits, LONG_BIT/8);
		    }
		    w = 7 - wEnd;
		    while (w--)
		    {
			nlw = nlwMiddle;
			dst = dstTmp;
			dstTmp++;
#if defined(__GNUC__) && defined(mc68020)
			mask = cfb8PixelMasks[GetFourBits(bits)];
			xor = xor & mask;
			mask = ~mask;
			while (nlw--)
			{
			    *dst = (*dst & mask) | xor;
			    dst += 8;
			}
			xor = devPriv->xor;
#else
#define SwitchBitsLoop(body) \
	while (nlw--)	\
	{		\
	    body	\
	    dst += 8;	\
	}
			SwitchFourBits(dst, xor, GetFourBits(bits));
#undef SwitchBitsLoop
#endif
			NextFourBits (bits);
		    }
		    nlwMiddle--;
		    w = wEnd + 1;
		    if (endmask)
		    {
			mask = cfb8PixelMasks[GetFourBits(bits)];
			dst = dstTmp + (nlwMiddle << 3);
			*dst = (*dst & ~(mask & endmask)) |
			       (xor &  (mask & endmask));
		    }
		    while (w--)
		    {
			nlw = nlwMiddle;
			dst = dstTmp;
			dstTmp++;
#if defined(__GNUC__) && defined(mc68020)
			mask = cfb8PixelMasks[GetFourBits(bits)];
			xor = xor & mask;
			mask = ~mask;
			while (nlw--)
			{
			    *dst = (*dst & mask) | xor;
			    dst += 8;
			}
			xor = devPriv->xor;
#else
#define SwitchBitsLoop(body) \
	while (nlw--)	\
	{		\
	    body	\
	    dst += 8;	\
	}
			SwitchFourBits(dst, xor, GetFourBits(bits));
#undef SwitchBitsLoop
#endif
			NextFourBits (bits);
		    }
		    nlwMiddle++;
		}
	    }
	}
	else
	{
	    while (h--)
	    {
		bits = src[y];
		y++;
		if (y == stippleHeight)
		    y = 0;
	    	if (rot != 0)
		    RotBitsLeft (bits, rot);
	    	dst = dstLine;
	    	dstLine += nlwDst;
	    	if (startmask)
	    	{
		    xor = GetFourBits(bits);
		    *dst = MaskRRopPixels(*dst, xor, startmask);
		    dst++;
		    RotBitsLeft (bits, LONG_BIT/8);
	    	}
		nlw = nlwMiddle;
	    	while (nlw--)
	    	{
		    RRopFourBits(dst, GetFourBits(bits));
		    dst++;
		    RotBitsLeft (bits, LONG_BIT/8);
	    	}
	    	if (endmask)
	    	{
		    xor = GetFourBits(bits);
		    *dst = MaskRRopPixels(*dst, xor, endmask);
	    	}
	    }
	}
    }
}


void
cfb8FillRectStippledUnnatural (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;
    register BoxPtr pBox;
{
    unsigned long   *pdstBase;	/* pointer to start of bitmap */
    unsigned long   *pdstLine;	/* current destination line */
    int		    nlwDst;	/* width in longwords of bitmap */
    PixmapPtr	    pStipple;	/* pointer to stipple we want to fill with */
    int		    nlwMiddle;
    register int    nlw;
    int		    x, y, w, h, xrem, xSrc, ySrc;
    int		    stwidth, stippleWidth;
    int		    stippleHeight;
    register unsigned long  bits, inputBits;
    register int    partBitsLeft;
    int		    nextPartBits;
    int		    bitsLeft, bitsWhole;
    register unsigned long    *pdst;	/* pointer to current word in bitmap */
    unsigned long   *srcTemp, *srcStart;
    unsigned long   *psrcBase;
    unsigned long   startmask, endmask;

    if (pGC->fillStyle == FillStippled)
	cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
    else
	cfb8CheckOpaqueStipple (pGC->alu, pGC->fgPixel, pGC->bgPixel, pGC->planemask);

    if (cfb8StippleRRop == GXnoop)
	return;

    /*
     *  OK,  so what's going on here?  We have two Drawables:
     *
     *  The Stipple:
     *		Depth = 1
     *		Width = stippleWidth
     *		Words per scanline = stwidth
     *		Pointer to pixels = pStipple->devPrivate.ptr
     */

    pStipple = pGC->stipple;

    stwidth = pStipple->devKind / sizeof(long);
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;
    psrcBase = (unsigned long *) pStipple->devPrivate.ptr;

    /*
     *	The Target:
     *		Depth = PSZ
     *		Width = determined from *pwidth
     *		Words per scanline = nlwDst
     *		Pointer to pixels = addrlBase
     */

    xSrc = pDrawable->x;
    ySrc = pDrawable->y;

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pdstBase)

    /* this replaces rotating the stipple. Instead we just adjust the offset
     * at which we start grabbing bits from the stipple.
     * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
     * so that iline and xrem always stay within the stipple bounds.
     */

    xSrc += (pGC->patOrg.x % stippleWidth) - stippleWidth;
    ySrc += (pGC->patOrg.y % stippleHeight) - stippleHeight;

    bitsWhole = stippleWidth;

    while (nBox--)
    {
	x = pBox->x1;
	y = pBox->y1;
	w = pBox->x2 - x;
	h = pBox->y2 - y;
	pBox++;
	pdstLine = pdstBase + y * nlwDst + (x >> PWSH);
	y = (y - ySrc) % stippleHeight;
	srcStart = psrcBase + y * stwidth;
	xrem = ((x & ~(sizeof(long)-1)) - xSrc) % stippleWidth;
	if (((x & PIM) + w) < PPW)
	{
	    maskpartialbits (x, w, startmask);
	    nlwMiddle = 0;
	    endmask = 0;
	}
	else
	{
	    maskbits (x, w, startmask, endmask, nlwMiddle);
	}
	while (h--)
	{
    	    srcTemp = srcStart + (xrem >> MFB_PWSH);
    	    bitsLeft = stippleWidth - (xrem & ~MFB_PIM);
	    NextUnnaturalStippleWord
	    NextSomeBits (inputBits, (xrem & MFB_PIM));
	    partBitsLeft -= (xrem & MFB_PIM);
	    NextUnnaturalStippleBits
	    nlw = nlwMiddle;
	    pdst = pdstLine;
	    if (startmask)
	    {
		*pdst = MaskRRopPixels(*pdst,bits,startmask);
		pdst++;
		NextUnnaturalStippleBits
	    }
	    while (nlw--)
	    {
		*pdst = RRopPixels(*pdst,bits);
		pdst++;
		NextUnnaturalStippleBits
	    }
	    if (endmask)
		*pdst = MaskRRopPixels(*pdst,bits,endmask);
	    pdstLine += nlwDst;
	    y++;
	    srcStart += stwidth;
	    if (y == stippleHeight)
	    {
		y = 0;
		srcStart = psrcBase;
	    }
	}
    }
}

#endif
