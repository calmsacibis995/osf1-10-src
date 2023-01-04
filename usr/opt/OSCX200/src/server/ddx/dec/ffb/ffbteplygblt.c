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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ffbteplygblt.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:18:09 $";
#endif
/*
 */

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"

#include "dixfontstr.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "ffb.h"
#include "ffbteglyph.h"
#include "ffbplygblt.h"

#include "ffbteplygblt.h" 

/*

Specially fast PolyGlyphBlt and ImageGlyphBlt code for fixed-metric fonts of
width <= FFBBUSBITS.  We know that:

    4 <= width <= FFBBUSBITS
    leftBearing = 0
    rightBearing = character width
    font Ascent = character Ascent
    font Descent = character Descent
    and forward-moving font (character width > 0).

This file is compiled twice: once for PolyText, and once for ImageText.

Note that ImageText ignores the graphics function, and always uses GXcopy.

ImageText for 8-bit packed visuals on 24-plane systems always uses
OPAQUESTIPPLE mode, as BLOCKFILL mode isn't available.

ImageText for all other visuals first clears the background using BLOCKFILL
mode, then uses the PolyText terminal emulator routines to splatter the
foreground text onto the background rectangle.

*/

#ifdef Bits64

#   ifdef FFBIMAGETEXT
static VoidProc imageGlyphs[32] = {
    /*  1 */    ffbTEImageGlyphs6,
    /*  2 */    ffbTEImageGlyphs6,
    /*  3 */    ffbTEImageGlyphs6,
    /*  4 */    ffbTEImageGlyphs6,
    /*  5 */    ffbTEImageGlyphs6,
    /*  6 */    ffbTEImageGlyphs6,

    /*  7 */    ffbTEImageGlyphs8,
    /*  8 */    ffbTEImageGlyphs8,

    /*  9 */    ffbTEImageGlyphs10,
    /* 10 */    ffbTEImageGlyphs10,

    /* 11 */    ffbTEImageGlyphs15,
    /* 12 */    ffbTEImageGlyphs15,
    /* 13 */    ffbTEImageGlyphs15,
    /* 14 */    ffbTEImageGlyphs15,
    /* 15 */    ffbTEImageGlyphs15,

    /* 16 */    ffbTEImageGlyphs16,

    /* 17 */    ffbTEImageGlyphs20, 
    /* 18 */    ffbTEImageGlyphs20, 
    /* 19 */    ffbTEImageGlyphs20,
    /* 20 */    ffbTEImageGlyphs20, 

    /* 21 */    ffbTEImageGlyphs30, ffbTEImageGlyphs30, ffbTEImageGlyphs30,
    /* 24 */    ffbTEImageGlyphs30, ffbTEImageGlyphs30, ffbTEImageGlyphs30,
    /* 27 */    ffbTEImageGlyphs30, ffbTEImageGlyphs30, ffbTEImageGlyphs30,
    /* 30 */    ffbTEImageGlyphs30,

    /* 31 */    ffbTEImageGlyphs32,
    /* 32 */    ffbTEImageGlyphs32
};
#   endif

static VoidProc splatGlyphs[32] = {
    /*  1 */    ffbTESplatGlyphs6,
    /*  2 */    ffbTESplatGlyphs6,
    /*  3 */    ffbTESplatGlyphs6,
    /*  4 */    ffbTESplatGlyphs6,
    /*  5 */    ffbTESplatGlyphs6,
    /*  6 */    ffbTESplatGlyphs6,

    /*  7 */    ffbTESplatGlyphs8,
    /*  8 */    ffbTESplatGlyphs8,

    /*  9 */    ffbTESplatGlyphs10,
    /* 10 */    ffbTESplatGlyphs10,

    /* 11 */    ffbTESplatGlyphs15,
    /* 12 */    ffbTESplatGlyphs15,
    /* 13 */    ffbTESplatGlyphs15,
    /* 14 */    ffbTESplatGlyphs15,
    /* 15 */    ffbTESplatGlyphs15,

    /* 16 */    ffbTESplatGlyphs16,

    /* 17 */    ffbTESplatGlyphs20, 
    /* 18 */    ffbTESplatGlyphs20, 
    /* 19 */    ffbTESplatGlyphs20,
    /* 20 */    ffbTESplatGlyphs20, 

    /* 21 */    ffbTESplatGlyphs30, ffbTESplatGlyphs30, ffbTESplatGlyphs30,
    /* 24 */    ffbTESplatGlyphs30, ffbTESplatGlyphs30, ffbTESplatGlyphs30,
    /* 27 */    ffbTESplatGlyphs30, ffbTESplatGlyphs30, ffbTESplatGlyphs30,
    /* 30 */    ffbTESplatGlyphs30,

    /* 31 */    ffbTESplatGlyphs32,
    /* 32 */    ffbTESplatGlyphs32
};

#else /* No 64-bit integers */

#   ifdef FFBIMAGETEXT
static VoidProc imageGlyphs[32] = {
    /*  1 */    ffbTEImageGlyphs7,
    /*  2 */    ffbTEImageGlyphs7,
    /*  3 */    ffbTEImageGlyphs7,
    /*  4 */    ffbTEImageGlyphs7,
    /*  5 */    ffbTEImageGlyphs7,
    /*  6 */    ffbTEImageGlyphs7,
    /*  7 */    ffbTEImageGlyphs7,

    /*  8 */    ffbTEImageGlyphs8,

    /*  9 */    ffbTEImageGlyphs14,
    /* 10 */    ffbTEImageGlyphs14,
    /* 11 */    ffbTEImageGlyphs14,
    /* 12 */    ffbTEImageGlyphs14,
    /* 13 */    ffbTEImageGlyphs14,
    /* 14 */    ffbTEImageGlyphs14,

    /* 15 */    ffbTEImageGlyphs16,
    /* 16 */    ffbTEImageGlyphs16,

    /* 17 */    ffbTEImageGlyphs32, ffbTEImageGlyphs32, ffbTEImageGlyphs32,
    /* 20 */    ffbTEImageGlyphs32, ffbTEImageGlyphs32, ffbTEImageGlyphs32,
    /* 23 */    ffbTEImageGlyphs32, ffbTEImageGlyphs32, ffbTEImageGlyphs32,
    /* 26 */    ffbTEImageGlyphs32, ffbTEImageGlyphs32, ffbTEImageGlyphs32,
    /* 29 */    ffbTEImageGlyphs32, ffbTEImageGlyphs32, ffbTEImageGlyphs32,
    /* 32 */    ffbTEImageGlyphs32
};
#   endif

static VoidProc splatGlyphs[32] = {
    /*  1 */    ffbTESplatGlyphs7,
    /*  2 */    ffbTESplatGlyphs7,
    /*  3 */    ffbTESplatGlyphs7,
    /*  4 */    ffbTESplatGlyphs7,
    /*  5 */    ffbTESplatGlyphs7,
    /*  6 */    ffbTESplatGlyphs7,
    /*  7 */    ffbTESplatGlyphs7,

    /*  8 */    ffbTESplatGlyphs8,

    /*  9 */    ffbTESplatGlyphs14,
    /* 10 */    ffbTESplatGlyphs14,
    /* 11 */    ffbTESplatGlyphs14,
    /* 12 */    ffbTESplatGlyphs14,
    /* 13 */    ffbTESplatGlyphs14,
    /* 14 */    ffbTESplatGlyphs14,

    /* 15 */    ffbTESplatGlyphs16,
    /* 16 */    ffbTESplatGlyphs16,

    /* 17 */    ffbTESplatGlyphs32, ffbTESplatGlyphs32, ffbTESplatGlyphs32,
    /* 20 */    ffbTESplatGlyphs32, ffbTESplatGlyphs32, ffbTESplatGlyphs32,
    /* 23 */    ffbTESplatGlyphs32, ffbTESplatGlyphs32, ffbTESplatGlyphs32,
    /* 26 */    ffbTESplatGlyphs32, ffbTESplatGlyphs32, ffbTESplatGlyphs32,
    /* 29 */    ffbTESplatGlyphs32, ffbTESplatGlyphs32, ffbTESplatGlyphs32,
    /* 32 */    ffbTESplatGlyphs32
};

#endif /* 64 bit integers else only 32 bit integers */

#define ChooseGlyphProc(table, charWidth)  (table[(charWidth)-1])


#ifdef FFBIMAGETEXT
void
#else
int
#endif
FFBTEPLYGLYPHBLT(pDrawable, pGC, x, y, nglyph, ppci)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			x, y;
    unsigned int	nglyph;
    CharInfoPtr		*ppci;		/* array of character info	    */
{
    BoxRec		bbox;		/* string's bounding box	    */
    int			widthDst;	/* width of dst in bytes	    */
    Pixel8		*pdstBase;	/* first byte of current glyph in dst */
    register Pixel8     *pdst;		/* pointer to current byte in dst   */
    Pixel8		*p;
    int			charAscent;
    int			charDescent;
    int			charWidth;
    int			overallWidth;
    cfbPrivGC		*gcPriv;
    VoidProc		paintGlyphs;
    long		width, height;
    long		align;
    CommandWord		mask;
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    ffbBufDPtr		pbufD;
    ffbGCPrivPtr 	ffbGCPriv;   
 
#ifdef FFBIMAGETEXT
    if (nglyph == 0) return;
#else
    if (nglyph == 0) return 0;
#endif

    x += pDrawable->x;
    y += pDrawable->y;
    
    DrawableBaseAndWidth(pDrawable, pdstBase, widthDst);

     charAscent = FONTASCENT(pGC->font);
     charWidth = FONTMINBOUNDS(pGC->font, rightSideBearing);
     charDescent = FONTDESCENT(pGC->font);

    /* Make pdstBase point to upper left bit of first glyph */
    pdstBase += (widthDst * (y-charAscent)) + x * FFBPIXELBYTES;

    overallWidth = nglyph * charWidth;
    bbox.x1 = x;
    bbox.x2 = x + overallWidth;
    bbox.y1 = y - charAscent;
    bbox.y2 = y + charDescent;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDrawable->pScreen, pDrawable, scrPriv, ffb, pGC);
    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
#if !defined(FFBIMAGETEXT)
    if (FFBGCBLOCK(pGC)) {
	FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE); 
	FFBLOADCOLORREGS(ffb, gcPriv->xor, pDrawable->depth);
    } else {
	FFBMODE(ffb, TRANSPARENTSTIPPLE);
    }
#endif

    switch ((*pGC->pScreen->RectIn)(gcPriv->pCompositeClip, &bbox)) {
      case rgnOUT:
	break;

      case rgnIN:
#ifdef FFBIMAGETEXT
	pbufD = FFBBUFDESCRIPTOR(pDrawable);
	CYCLE_REGS(ffb);
	FFBROP(ffb, GXcopy, pbufD->rotateDst, pbufD->visualDst);
#   if FFBPIXELBITS == 32
	if (pDrawable->depth > 8) {
#   endif
	    /* Paint the background using BLOCKFILL, then transparent stipple
	       bits with BLOCKSTIPPLE. */

	    CYCLE_REGS(ffb);
	    FFBMODE(ffb, BLOCKFILL);
	    FFBDATA(ffb, ~0);
	    FFBLOADCOLORREGS(ffb, pGC->bgPixel, pDrawable->depth);
	    height = bbox.y2 - bbox.y1;
	    width = bbox.x2 - bbox.x1;
	    p = pdstBase;
	    align = (long)p & FFBBUSBYTESMASK;
	    p -= align;
	    if (width <= FFBMAXBSWPIXELS) {
		mask = FFBLOADBLOCKDATA(align, width);
		while (height != 0) {
		    FFBWRITE(p, mask);
		    p += widthDst;
		    height--;
		}
	    } else {
		long    w;
		Pixel8  *tp;
		mask = FFBLOADBLOCKDATA(align, FFBMAXBSWPIXELS);
		while (height != 0) {
		    tp = p;
		    w = width;
		    while (w > FFBMAXBSWPIXELS) {
			FFBWRITE(tp, mask);
			tp += FFBMAXBSWPIXELS * FFBPIXELBYTES;
			w -= FFBMAXBSWPIXELS;
		    }
		    FFBWRITE(tp, FFBLOADBLOCKDATA(align, w));
		    p += widthDst;
		    height--;
		}
	    }
	    CYCLE_REGS(ffb);
	    FFBLOADCOLORREGS(ffb, pGC->fgPixel, pDrawable->depth);
	    FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE);
	    paintGlyphs = ChooseGlyphProc(splatGlyphs, charWidth);
#   if FFBPIXELBITS == 32
	} else {
	    /* Use OPAQUESTIPPLE directly to paint foreground and background */
	    CYCLE_REGS(ffb);
	    FFBMODE(ffb, OPAQUESTIPPLE);
	    paintGlyphs = ChooseGlyphProc(imageGlyphs, charWidth);
	}
#   endif
	(*paintGlyphs) (pdstBase, widthDst, nglyph, ppci, ffb);
#else   /* PolyText */
	paintGlyphs = ChooseGlyphProc(splatGlyphs, charWidth);
	(*paintGlyphs) (pdstBase, widthDst, nglyph, ppci);
#endif
#ifdef FFBIMAGETEXT
	WRITE_MEMORY_BARRIER();
	FFBROP(ffb, pGC->alu, pbufD->rotateDst, pbufD->visualDst);
#endif
	break;

      case rgnPART:
	x -= pDrawable->x;
	y -= pDrawable->y;
	/* ||| This is so sleazy it's paintful.  I should do a fixed-metric
	   version of clipping, which would be a lot faster. */
#ifdef FFBIMAGETEXT
	ffbImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci);
#else
	ffbPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci);
#endif
	break;

      default:
	break;
    }
#ifndef FFBIMAGETEXT
    return overallWidth;
#endif
}

/*
 * HISTORY
 */
