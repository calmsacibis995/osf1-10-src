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
/* $XConsortium: cfbglblt8.c,v 5.24 91/12/19 14:16:27 keith Exp $ */
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
*/

/*
 * Poly glyph blt.  Accepts an arbitrary font <= 32 bits wide, in Copy mode
 * only.
 */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"cfb.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"

#define BOX_OVERLAP(box1, box2, xoffset, yoffset) \
 	((box1)->x1 <= ((int) (box2)->x2 + (xoffset)) && \
 	 ((int) (box2)->x1 + (xoffset)) <= (box1)->x2 && \
	 (box1)->y1 <= ((int) (box2)->y2 + (yoffset)) && \
 	 ((int) (box2)->y1 + (yoffset)) <= (box1)->y2)

#define BOX_CONTAINS(box1, box2, xoffset, yoffset) \
 	((box1)->x1 <= ((int) (box2)->x1 + (xoffset)) && \
 	 ((int) (box2)->x2 + (xoffset)) <= (box1)->x2 && \
	 (box1)->y1 <= ((int) (box2)->y1 + (yoffset)) && \
 	 ((int) (box2)->y2 + (yoffset)) <= (box1)->y2)

#if defined(FOUR_BIT_CODE) || defined(WriteFourBits) && !defined(GLYPHROP)

#if GLYPHPADBYTES != 4
#define USE_LEFTBITS
#endif

#ifdef USE_LEFTBITS
typedef	unsigned char	*glyphPointer;
extern unsigned long endtab[];

#define GlyphBits(bits,width,dst)	getleftbits(bits,width,dst); \
					(dst) &= widthMask; \
					(bits) += widthGlyph;
#define GlyphBitsS(bits,width,dst,off)	GlyphBits(bits,width,dst); \
					dst = BitRight (dst, off);
#else
typedef unsigned int	*glyphPointer;

#define GlyphBits(bits,width,dst)	dst = (*bits++);
#define GlyphBitsS(bits,width,dst,off)	dst = BitRight(*bits++, off);
#endif

#ifdef GLYPHROP
#define cfbPolyGlyphBlt8	cfbPolyGlyphRop8
#define cfbPolyGlyphBlt8Clipped	cfbPolyGlyphRop8Clipped

#undef WriteFourBits
#define WriteFourBits(dst,pixel,bits)	RRopFourBits(dst,bits)

#endif

static void cfbPolyGlyphBlt8Clipped();

#if defined(HAS_STIPPLE_CODE) && !defined(GLYPHROP) && !defined(USE_LEFTBITS)
#define USE_STIPPLE_CODE
#endif

#if defined(__GNUC__) && !defined(GLYPHROP) && (defined(mc68020) || defined(mc68000) || defined(__mc68000__)) && !defined(USE_LEFTBITS)
#ifdef USE_STIPPLE_CODE
#undef USE_STIPPLE_CODE
#endif
#endif

#define DST_INC	    (sizeof(long) >> PWSH)

void
cfbPolyGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    register unsigned long  c;
#ifndef GLYPHROP
    register unsigned long  pixel;
#endif
    register unsigned long  *dst;
    register glyphPointer   glyphBits;
    register int	    xoff;

    FontPtr		pfont = pGC->font;
    CharInfoPtr		pci;
    unsigned long	*dstLine;
    unsigned long	*pdstBase;
    int			hTmp;
    int			bwidthDst;
    int			widthDst;
    int			h;
    int			ew;
    BoxRec		bbox;		/* for clipping */
    int			widthDiff;
    int			w;
    RegionPtr		clip;
    BoxPtr		extents;
#ifdef USE_LEFTBITS
    int			widthGlyph;
    unsigned long	widthMask;
#endif
#ifndef STIPPLE
#ifdef USE_STIPPLE_CODE
    void		(*stipple)();
    extern void		cfbStippleStack (), cfbStippleStackTE ();

    stipple = cfbStippleStack;
    if (FONTCONSTMETRICS(pfont))
	stipple = cfbStippleStackTE;
#endif
#endif
    
    x += pDrawable->x;
    y += pDrawable->y;

    /* compute an approximate (but covering) bounding box */
    bbox.x1 = 0;
    if ((ppci[0]->metrics.leftSideBearing < 0))
	bbox.x1 = ppci[0]->metrics.leftSideBearing;
    h = nglyph - 1;
    w = ppci[h]->metrics.rightSideBearing;
    while (--h >= 0)
	w += ppci[h]->metrics.characterWidth;
    bbox.x2 = w;
    bbox.y1 = -FONTMAXBOUNDS(pfont,ascent);
    bbox.y2 = FONTMAXBOUNDS(pfont,descent);

    clip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    extents = &clip->extents;

    if (!clip->data) 
    {
	if (!BOX_CONTAINS(extents, &bbox, x, y))
	{
	    if (BOX_OVERLAP (extents, &bbox, x, y)) 
		cfbPolyGlyphBlt8Clipped(pDrawable, pGC, x, y, 
		     nglyph, ppci, pglyphBase);
	    return;
	}
    }
    else
    {
    	/* check to make sure some of the text appears on the screen */
    	if (!BOX_OVERLAP (extents, &bbox, x, y))
	    return;
    
    	bbox.x1 += x;
    	bbox.x2 += x;
    	bbox.y1 += y;
    	bbox.y2 += y;
    
    	switch ((*pGC->pScreen->RectIn)(clip, &bbox))
    	{
      	  case rgnPART:
	    cfbPolyGlyphBlt8Clipped(pDrawable, pGC, x, y, 
		nglyph, ppci, pglyphBase);
      	  case rgnOUT:
	    return;
    	}
    }

#ifdef GLYPHROP
    cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
#else
    pixel = ((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->xor;
#endif

    cfbGetTypedWidthAndPointer (pDrawable, bwidthDst, pdstBase, char, unsigned long)

    widthDst = bwidthDst / sizeof(long);
    while (nglyph--)
    {
	pci = *ppci++;
	glyphBits = (glyphPointer) FONTGLYPHBITS(pglyphBase,pci);
	xoff = x + pci->metrics.leftSideBearing;
	dstLine = pdstBase +
	          (y - pci->metrics.ascent) * widthDst + (xoff >> PWSH);
	x += pci->metrics.characterWidth;
	if (hTmp = pci->metrics.descent + pci->metrics.ascent)
	{
	    xoff &= PIM;
#ifdef STIPPLE
	    STIPPLE(dstLine,glyphBits,pixel,bwidthDst,hTmp,xoff);
#else
#ifdef USE_STIPPLE_CODE
	    (*stipple)(dstLine,glyphBits,pixel,bwidthDst,hTmp,xoff);
#else
#ifdef USE_LEFTBITS
	    w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	    widthGlyph = PADGLYPHWIDTHBYTES(w);
	    widthMask = endtab[w];
#endif
	    do {
	    	dst = dstLine;
	    	dstLine = (unsigned long *) (((char *) dstLine) + bwidthDst);
	    	GlyphBits(glyphBits, w, c)
	    	WriteFourBits(dst, pixel, GetFourBits(BitRight(c,xoff)));
	    	dst += DST_INC;
	    	c = BitLeft(c,sizeof(long)-xoff);
	    	while (c)
	    	{
		    WriteFourBits(dst, pixel, GetFourBits(c));
		    NextFourBits(c);
		    dst += DST_INC;
	    	}
	    } while (--hTmp);
#endif /* USE_STIPPLE_CODE else */
#endif /* STIPPLE else */
	}
    }
}

static void
cfbPolyGlyphBlt8Clipped (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    register unsigned long  c;
#ifndef GLYPHROP
    register unsigned long  pixel;
#endif
    register unsigned long  *dst;
    register glyphPointer   glyphBits;
    register int	    xoff;
    unsigned long	    c1;

    CharInfoPtr		pci;
    FontPtr		pfont = pGC->font;
    unsigned long	*dstLine;
    unsigned long	*pdstBase;
    unsigned long	*cTmp, *clips;
#if LONG_BIT == 64
    unsigned int	* clips32 = (unsigned int *)NULL;
#endif /* LONG_BIT */
    int			maxAscent, maxDescent;
    int			minLeftBearing;
    int			hTmp;
    int			widthDst;
    int			bwidthDst;
    int			ew;
    int			xG, yG;
    BoxPtr		pBox;
    int			numRects;
    int			widthDiff;
    int			w;
    RegionPtr		pRegion;
    int			yBand;
#ifdef USE_LEFTBITS
    int			widthGlyph;
    unsigned long	widthMask;
#endif

#ifdef GLYPHROP
    cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
#else
    pixel = ((cfbPrivGCPtr) pGC->devPrivates[cfbGCPrivateIndex].ptr)->xor;
#endif
    
    cfbGetTypedWidthAndPointer (pDrawable, bwidthDst, pdstBase, char, unsigned long)

    widthDst = bwidthDst / sizeof(long);
    maxAscent = FONTMAXBOUNDS(pfont,ascent);
    maxDescent = FONTMAXBOUNDS(pfont,descent);
    minLeftBearing = FONTMINBOUNDS(pfont,leftSideBearing);

    pRegion = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;

    pBox = REGION_RECTS(pRegion);
    numRects = REGION_NUM_RECTS (pRegion);
    while (numRects && pBox->y2 <= y - maxAscent)
    {
	++pBox;
	--numRects;
    }
    if (!numRects || pBox->y1 >= y + maxDescent)
	return;
    yBand = pBox->y1;
    while (numRects && pBox->y1 == yBand && pBox->x2 <= x + minLeftBearing)
    {
	++pBox;
	--numRects;
    }
    if (!numRects)
	return;
    clips = (unsigned long *)ALLOCATE_LOCAL ((maxAscent + maxDescent) *
					     sizeof (unsigned long));
    while (nglyph--)
    {
	pci = *ppci++;
	glyphBits = (glyphPointer) FONTGLYPHBITS(pglyphBase,pci);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	xG = x + pci->metrics.leftSideBearing;
	yG = y - pci->metrics.ascent;
	x += pci->metrics.characterWidth;
	if (hTmp = pci->metrics.descent + pci->metrics.ascent)
	{
	    dstLine = pdstBase + yG * widthDst + (xG >> PWSH);
	    xoff = xG & PIM;
#ifdef USE_LEFTBITS
	    w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	    widthGlyph = PADGLYPHWIDTHBYTES(w);
	    widthMask = endtab[w];
#endif
	    switch (cfb8ComputeClipMasks32 (pBox, numRects, xG, yG, w, hTmp, clips))
 	    {
	    case rgnPART:
#ifdef USE_LEFTBITS
	    	cTmp = clips;
	    	do {
	    	    dst = dstLine;
	    	    dstLine = (unsigned long *) (((char *) dstLine) + bwidthDst);
	    	    GlyphBits(glyphBits, w, c)
		    c &= *cTmp++;
		    if (c)
		    {
	    	    	WriteFourBits(dst, pixel, GetFourBits(BitRight(c,xoff)));
	    	    	c = BitLeft(c,4 - xoff);
	    	    	dst += DST_INC;
	    	    	while (c)
	    	    	{
		    	    WriteFourBits(dst, pixel, GetFourBits(c));
		    	    NextFourBits(c);
		    	    dst += DST_INC;
	    	    	}
		    }
	    	} while (--hTmp);
	    	break;
#else /* USE_LEFT_BITS */
	    	{
		    int h;
    
		    h = hTmp;
		    do
		    {
			--h;
		    	clips[h] = clips[h] & (unsigned long)(glyphBits[h]);
		    } while (h);
	    	}
#if LONG_BIT == 32
	    	glyphBits = (unsigned int *)clips;
#else /* LONG_BIT == 64 */
		{
		    register int i;
		    clips32 = (unsigned int *)ALLOCATE_LOCAL(
			(maxAscent + maxDescent) *sizeof(unsigned int));
		    for ( i=0; i < (maxAscent+maxDescent); i++ )
			clips32[i] = (unsigned int)clips[i];
		}
	    	glyphBits = (glyphPointer)clips32;
#endif /* LONG_BIT */
	    	/* fall through */
#endif /* USE_LEFT_BITS */
	    case rgnIN:
#ifdef STIPPLE
	    	STIPPLE(dstLine,glyphBits,pixel,bwidthDst,hTmp,xoff);
#else
#ifdef USE_STIPPLE_CODE
	    	cfbStippleStackTE(dstLine,glyphBits,pixel,bwidthDst,hTmp,xoff);
#else
	    	do {
	    	    dst = dstLine;
	    	    dstLine = (unsigned long *) (((char *) dstLine) + bwidthDst);
	    	    GlyphBits(glyphBits, w, c)
		    if (c)
		    {
	    	    	WriteFourBits(dst, pixel, GetFourBits(BitRight(c,xoff)));
	    	    	c = BitLeft(c,sizeof(long)-xoff);
	    	    	dst += DST_INC;
	    	    	while (c)
	    	    	{
		    	    WriteFourBits(dst, pixel, GetFourBits(c));
		    	    NextFourBits(c);
		    	    dst += DST_INC;
	    	    	}
		    }
	    	} while (--hTmp);
#endif /* USE_STIPPLE_CODE else */
#endif /* STIPPLE else */
#if LONG_BIT == 64
    		if ( clips32 != (unsigned int *)NULL ) {
			DEALLOCATE_LOCAL (clips32);
			clips32 = (unsigned int *)NULL;
		}
#endif /* LONG_BIT */
	    	break;
	    }
	}
    }
    DEALLOCATE_LOCAL (clips);
}

#endif /* FOUR_BIT_CODE */
