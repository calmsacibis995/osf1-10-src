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
 * TEGblt - ImageText expanded glyph fonts only.  For
 * 8 bit displays, in Copy mode with no clipping.
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
*/

/* $XConsortium: cfbteblt8.c,v 5.18 92/05/04 16:34:02 rws Exp $ */

#if PSZ == 8

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

/*
 * this code supports up to 5 characters at a time.  The performance
 * differences between 4 and 5 is usually small (~7% on PMAX) and
 * frequently negative (SPARC and Sun3), so this file is compiled
 * only once for now.  If you want to use the other options, you'll
 * need to hack cfbgc.c as well.
 */

#ifndef NGLYPHS
#define NGLYPHS 4
#define DO_COMMON
#endif

#ifdef DO_COMMON
#define CFBTEGBLT8 cfbTEGlyphBlt8
#endif

/*
 * On little-endian machines (or where fonts are padded to 32-bit
 * boundaries) we can use some magic to avoid the expense of getleftbits
 */

#if ((BITMAP_BIT_ORDER == LSBFirst && NGLYPHS >= 4) || GLYPHPADBYTES == 4)

#if GLYPHPADBYTES == 1
typedef unsigned char	*glyphPointer;
#define USE_LEFTBITS
#endif

#if GLYPHPADBYTES == 2
typedef unsigned short	*glyphPointer;
#define USE_LEFTBITS
#endif

#if GLYPHPADBYTES == 4
typedef unsigned int	*glyphPointer;
#endif

#define GetBitsL       c = BitLeft (*leftChar++, lshift)
#define NGetBits1S(r)   c = BitRight(*char1++ r, xoff1)
#define NGetBits1L(r)   GetBitsL | BitRight(*char1++ r, xoff1)
#define NGetBits1U(r)   c = *char1++ r
#define NGetBits2S(r)   NGetBits1S(| BitRight(*char2++ r, widthGlyph))
#define NGetBits2L(r)   NGetBits1L(| BitRight(*char2++ r, widthGlyph))
#define NGetBits2U(r)   NGetBits1U(| BitRight(*char2++ r, widthGlyph))
#define NGetBits3S(r)   NGetBits2S(| BitRight(*char3++ r, widthGlyph))
#define NGetBits3L(r)   NGetBits2L(| BitRight(*char3++ r, widthGlyph))
#define NGetBits3U(r)   NGetBits2U(| BitRight(*char3++ r, widthGlyph))
#define NGetBits4S(r)   NGetBits3S(| BitRight(*char4++ r, widthGlyph))
#define NGetBits4L(r)   NGetBits3L(| BitRight(*char4++ r, widthGlyph))
#define NGetBits4U(r)   NGetBits3U(| BitRight(*char4++ r, widthGlyph))
#define NGetBits5S(r)   NGetBits4S(| BitRight(*char5++ r, widthGlyph))
#define NGetBits5L(r)   NGetBits4L(| BitRight(*char5++ r, widthGlyph))
#define NGetBits5U(r)   NGetBits4U(| BitRight(*char5++ r, widthGlyph))
#define GetBits1S   c = BitRight(*char1++, xoff1)
#define GetBits1L   GetBitsL | BitRight(*char1++, xoff1)
#define GetBits1U   c = *char1++
#define GetBits2S   NGetBits1S(| BitRight(*char2++, widthGlyph))
#define GetBits2L   NGetBits1L(| BitRight(*char2++, widthGlyph))
#define GetBits2U   NGetBits1U(| BitRight(*char2++, widthGlyph))
#define GetBits3S   NGetBits2S(| BitRight(*char3++, widthGlyph))
#define GetBits3L   NGetBits2L(| BitRight(*char3++, widthGlyph))
#define GetBits3U   NGetBits2U(| BitRight(*char3++, widthGlyph))
#define GetBits4S   NGetBits3S(| BitRight(*char4++, widthGlyph))
#define GetBits4L   NGetBits3L(| BitRight(*char4++, widthGlyph))
#define GetBits4U   NGetBits3U(| BitRight(*char4++, widthGlyph))
#define GetBits5S   NGetBits4S(| BitRight(*char5++, widthGlyph))
#define GetBits5L   NGetBits4L(| BitRight(*char5++, widthGlyph))
#define GetBits5U   NGetBits4U(| BitRight(*char5++, widthGlyph))

#else

typedef unsigned int	*glyphPointer;

#define USE_LEFTBITS
#define ALL_LEFTBITS

#define GetBitsL    WGetBitsL
#define GetBits1S   WGetBits1S
#define GetBits1L   WGetBits1L
#define GetBits1U   WGetBits1U

#define GetBits2S   GetBits1S Get1Bits (char2, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff2);
#define GetBits2L   GetBits1L Get1Bits (char2, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff2);
#define GetBits2U   GetBits1U Get1Bits (char2, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff2);

#define GetBits3S   GetBits2S Get1Bits (char3, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff3);
#define GetBits3L   GetBits2L Get1Bits (char3, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff3);
#define GetBits3U   GetBits2U Get1Bits (char3, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff3);

#define GetBits4S   GetBits3S Get1Bits (char4, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff4);
#define GetBits4L   GetBits3L Get1Bits (char4, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff4);
#define GetBits4U   GetBits3U Get1Bits (char4, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff4);

#define GetBits5S   GetBits4S Get1Bits (char5, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff5);
#define GetBits5L   GetBits4L Get1Bits (char5, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff5);
#define GetBits5U   GetBits4U Get1Bits (char5, tmpSrc) \
		    c |= BitRight(tmpSrc, xoff5);

#endif

#ifdef USE_LEFTBITS
extern unsigned long endtab[];

#define IncChar(c)  (c = (glyphPointer) (((char *) c) + glyphBytes))

#define Get1Bits(ch,dst)    glyphbits (ch, widthGlyph, glyphMask, dst); \
			    IncChar (ch);

#define glyphbits(bits,width,mask,dst)	getleftbits(bits,width,dst); \
					dst &= mask;

#define WGetBitsL   Get1Bits(leftChar,c); \
		    c = BitLeft (c, lshift);
#define WGetBits1S  Get1Bits (char1, c) \
		    c = BitRight (c, xoff1);
#define WGetBits1L  WGetBitsL Get1Bits (char1, tmpSrc) \
		    c |= BitRight (tmpSrc, xoff1);
#define WGetBits1U  Get1Bits (char1, c)

#else
#define WGetBitsL   GetBitsL
#define WGetBits1S  GetBits1S
#define WGetBits1L  GetBits1L
#define WGetBits1U  GetBits1U
#endif

#if NGLYPHS == 2
# define GetBitsNS GetBits2S
# define GetBitsNL GetBits2L
# define GetBitsNU GetBits2U
# define LastChar char2
#ifndef CFBTEGBLT8
# define CFBTEGBLT8 cfbTEGlyphBlt8x2
#endif
#endif
#if NGLYPHS == 3
# define GetBitsNS GetBits3S
# define GetBitsNL GetBits3L
# define GetBitsNU GetBits3U
# define LastChar char3
#ifndef CFBTEGBLT8
# define CFBTEGBLT8 cfbTEGlyphBlt8x3
#endif
#endif
#if NGLYPHS == 4
# define GetBitsNS GetBits4S
# define GetBitsNL GetBits4L
# define GetBitsNU GetBits4U
# define LastChar char4
#ifndef CFBTEGBLT8
# define CFBTEGBLT8 cfbTEGlyphBlt8x4
#endif
#endif
#if NGLYPHS == 5
# define GetBitsNS GetBits5S
# define GetBitsNL GetBits5L
# define GetBitsNU GetBits5U
# define LastChar char5
#ifndef CFBTEGBLT8
# define CFBTEGBLT8 cfbTEGlyphBlt8x5
#endif
#endif

/* another ugly giant macro */
#define SwitchEm    switch (ew) \
		    { \
		    case 0: \
		    	break; \
		    case 1: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 \
			    Loop \
		    	} \
		    	break; \
		    case 2: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) \
			    Loop \
		    	} \
		    	break; \
		    case 3: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) Step StoreBits(2) \
			    Loop \
		    	} \
		    	break; \
		    case 4: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) Step \
			    StoreBits(2) Step StoreBits(3) \
			    Loop \
		    	} \
		    	break; \
		    case 5: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) Step \
			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) \
			    Loop \
		    	} \
		    	break; \
		    case 6: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) Step \
 			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) Step StoreBits(5) \
			    Loop \
		    	} \
		    	break; \
		    case 7: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) Step \
			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) Step StoreBits(5) Step \
			    StoreBits(6) \
			    Loop \
		    	} \
		    	break; \
		    case 8: \
		    	while (hTmp--) { \
			    GetBits; \
			    StoreBits0 FirstStep StoreBits(1) Step \
			    StoreBits(2) Step StoreBits(3) Step \
			    StoreBits(4) Step StoreBits(5) Step \
			    StoreBits(6) Step StoreBits(7) \
			    Loop \
		    	} \
		    	break; \
		    }

#ifdef FAST_CONSTANT_OFFSET_MODE
#define StorePixels(o,p)    dst[o] = p
#define Loop		    dst += widthDst;
#else
#define StorePixels(o,p)    *dst++ = (p)
#define Loop		    dst += widthLeft;
#endif

#define Step		    NextFourBits(c);

#if (BITMAP_BIT_ORDER == MSBFirst)
#define StoreBits(o)	StorePixels(o,GetFourPixels(c));
#define FirstStep	Step
#else
#if LONG_BIT == 64
#define StoreBits(o)	StorePixels(o,*((unsigned long *) (((char *) cfb8Pixels) + (c & 0x7f8))));
#define FirstStep	c = BitLeft (c, 5);
#else
#define StoreBits(o)	StorePixels(o,*((unsigned long *) (((char *) cfb8Pixels) + (c & 0x3c))));
#define FirstStep	c = BitLeft (c, 2);
#endif
#endif

extern void cfbImageGlyphBlt8();

void
CFBTEGBLT8 (pDrawable, pGC, xInit, yInit, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	xInit, yInit;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    register unsigned long  c;
    register unsigned long  *dst;
    register unsigned long  leftMask, rightMask;
    register int	    hTmp;
    register int	    xoff1;
    register glyphPointer   char1;
    register glyphPointer   char2;
#if NGLYPHS >= 3
    register glyphPointer   char3;
#endif
#if NGLYPHS >= 4
    register glyphPointer   char4;
#endif
#if NGLYPHS >= 5
    register glyphPointer   char5;
#endif
#ifdef ALL_LEFTBITS
    int xoff2, xoff3, xoff4, xoff5;
#endif

    FontPtr		pfont = pGC->font;
    unsigned long	*dstLine;
    glyphPointer	oldRightChar;
    unsigned long	*pdstBase;
    glyphPointer	leftChar;
    int			widthDst, widthLeft;
    int			widthGlyph;
    int			h;
    int			ew;
    int			x, y;
    BoxRec		bbox;		/* for clipping */
    int			lshift;
    int			widthGlyphs;
#ifdef USE_LEFTBITS
    register unsigned long  glyphMask;
    register unsigned long  tmpSrc;
    register int	    glyphBytes;
#endif

    widthGlyph = FONTMAXBOUNDS(pfont,characterWidth);
    h = FONTASCENT(pfont) + FONTDESCENT(pfont);
    if (!h)
	return;
    x = xInit + FONTMAXBOUNDS(pfont,leftSideBearing) + pDrawable->x;
    y = yInit - FONTASCENT(pfont) + pDrawable->y;
    bbox.x1 = x;
    bbox.x2 = x + (widthGlyph * nglyph);
    bbox.y1 = y;
    bbox.y2 = y + h;

    switch ((*pGC->pScreen->RectIn)(
                ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip, &bbox))
    {
      case rgnPART:
	cfbImageGlyphBlt8(pDrawable, pGC, xInit, yInit, nglyph, ppci, pglyphBase);
      case rgnOUT:
	return;
    }

    if (!cfb8CheckPixels (pGC->fgPixel, pGC->bgPixel))
	cfb8SetPixels (pGC->fgPixel, pGC->bgPixel);

    leftChar = 0;

    cfbGetLongWidthAndPointer(pDrawable, widthDst, pdstBase)

#if NGLYPHS == 2
    widthGlyphs = widthGlyph << 1;
#else
#if NGLYPHS == 4
    widthGlyphs = widthGlyph << 2;
#else
    widthGlyphs = widthGlyph * NGLYPHS;
#endif
#endif

#ifdef USE_LEFTBITS
    glyphMask = endtab[widthGlyph];
    glyphBytes = GLYPHWIDTHBYTESPADDED(*ppci);
#endif

    pdstBase += y * widthDst;
#ifdef DO_COMMON
    if (widthGlyphs <= 32)
#endif
    	while (nglyph >= NGLYPHS)
    	{
	    nglyph -= NGLYPHS;
	    hTmp = h;
	    dstLine = pdstBase + (x / sizeof(long));

	    xoff1 = x & (sizeof(long)-1);
	    char1 = (glyphPointer) FONTGLYPHBITS(pglyphBase, *ppci++);
	    char2 = (glyphPointer) FONTGLYPHBITS(pglyphBase, *ppci++);
#ifdef ALL_LEFTBITS
	    xoff2 = xoff1 + widthGlyph;
#endif
#if NGLYPHS >= 3
	    char3 = (glyphPointer) FONTGLYPHBITS(pglyphBase, *ppci++);
#ifdef ALL_LEFTBITS
	    xoff3 = xoff2 + widthGlyph;
#endif
#endif
#if NGLYPHS >= 4
	    char4 = (glyphPointer) FONTGLYPHBITS(pglyphBase, *ppci++);
#ifdef ALL_LEFTBITS
	    xoff4 = xoff3 + widthGlyph;
#endif
#endif
#if NGLYPHS >= 5
	    char5 = (glyphPointer) FONTGLYPHBITS(pglyphBase, *ppci++);
#ifdef ALL_LEFTBITS
	    xoff5 = xoff4 + widthGlyph;
#endif
#endif
	    oldRightChar = LastChar;
	    dst = dstLine;
	    if (xoff1)
	    {
		ew = ((widthGlyphs - (sizeof(long) - xoff1)) / sizeof(long)) + 1;
#ifndef FAST_CONSTANT_OFFSET_MODE
		widthLeft = widthDst - ew;
#endif
	    	if (!leftChar)
	    	{
		    leftMask = cfbendtab[xoff1];
		    rightMask = cfbstarttab[xoff1];

#define StoreBits0	StorePixels (0,dst[0] & leftMask | \
				       GetFourPixels(c) & rightMask);
#define GetBits GetBitsNS

		    SwitchEm

#undef GetBits
#undef StoreBits0

	    	}
	    	else
	    	{
		    lshift = widthGlyph - xoff1;
    
#define StoreBits0  StorePixels (0,GetFourPixels(c));
#define GetBits GetBitsNL
    
		    SwitchEm
    
#undef GetBits
#undef StoreBits0
    
	    	}
	    }
	    else
	    {
#if NGLYPHS == 4
	    	ew = widthGlyph;    /* widthGlyphs >> 2 */
#else
	    	ew = widthGlyphs / sizeof(long);
#endif
#ifndef FAST_CONSTANT_OFFSET_MODE
		widthLeft = widthDst - ew;
#endif

#define StoreBits0  StorePixels (0,GetFourPixels(c));
#define GetBits	GetBitsNU

	    	SwitchEm

#undef GetBits
#undef StoreBits0

	    }
	    x += widthGlyphs;
	    leftChar = oldRightChar;
    	}
    while (nglyph--)
    {
	/* alignment based on destination, not font, so use long, not int */
	xoff1 = x & (sizeof(long)-1);
	char1 = (glyphPointer) FONTGLYPHBITS(pglyphBase, *ppci++);
	hTmp = h;
	dstLine = pdstBase + (x / sizeof(long));

	oldRightChar = char1;
	dst = dstLine;
	if (xoff1)
	{
	    ew = ((widthGlyph - (sizeof(long) - xoff1)) / sizeof(long)) + 1;
#ifndef FAST_CONSTANT_OFFSET_MODE
		widthLeft = widthDst - ew;
#endif
	    if (!leftChar)
	    {
		leftMask = cfbendtab[xoff1];
		rightMask = cfbstarttab[xoff1];

#define StoreBits0	StorePixels (0,dst[0] & leftMask | GetFourPixels(c) & rightMask);
#define GetBits	WGetBits1S

		SwitchEm
#undef GetBits
#undef StoreBits0

	    }
	    else
	    {
		lshift = widthGlyph - xoff1;

#define StoreBits0  StorePixels (0,GetFourPixels(c));
#define GetBits WGetBits1L

		SwitchEm
#undef GetBits
#undef StoreBits0

	    }
	}
	else
	{
	    ew = widthGlyph / sizeof(long);

#ifndef FAST_CONSTANT_OFFSET_MODE
		widthLeft = widthDst - ew;
#endif

#define StoreBits0  StorePixels (0,GetFourPixels(c));
#define GetBits	WGetBits1U

	    SwitchEm

#undef GetBits
#undef StoreBits0

	}
	x += widthGlyph;
	leftChar = oldRightChar;
    }
    /*
     * draw the tail of the last character
     */
    xoff1 = x & (sizeof(long)-1);
    if (xoff1)
    {
	rightMask = cfbstarttab[xoff1];
	leftMask = cfbendtab[xoff1];
	lshift = widthGlyph - xoff1;
	dst = pdstBase + (x / sizeof(long));

	hTmp = h;
	while (hTmp--)
	{
	    GetBitsL;
	    *dst = (*dst & rightMask) | (GetFourPixels(c) & leftMask);
	    dst += widthDst;
	}
    }
}
#endif /* PSZ == 8 */
