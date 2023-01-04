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
static char *rcsid = "@(#)$RCSfile: ffbpntarea.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:13:06 $";
#endif
/*
 */
#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "ffbpntarea.h"
#include "cfbsetsp.h"
#include "ffbblt.h"

#include "ffbtile8.h"

extern CommandWord ffbStippleAll1;

static int SignBits = 0x80008000;



/*****************************************************************************
 *
 * This file is intimately tied with Imakefile: selection of which routines to
 * compile into any particular .o is done with -D<NAME>=<RealName>.
 *
 *****************************************************************************/

#ifdef FFBPOLYFILLRECTSOLID
void ffbPolyFillRectSolid (pDraw, pGC, nrectFill, prectBase)
    DrawablePtr pDraw;
    GCPtr	pGC;
    long	nrectFill;		/* number of rectangles to fill	     */
    xRectangle	*prectBase;		/* Pointer to first rectangle to fill*/
{
    RegionPtr       prgnClip;
    long	    dstwidth;
    long	    xDraw, yDraw;
    Pixel8	    *pdstBase;
    Pixel8	    *pdst;		/* pointer to start pixel of row     */
    Pixel8	    *p;			/* pointer to pixels we're writing   */
    long	    numRects;		/* Number of clipping rectangles     */
    cfbPrivGC       *gcPriv;
    BoxPtr	    pbox;		/* pointer to one region clip box    */
    long	    width;		/* width of current rect	     */
    long	    h;			/* height of current rect	     */
    long	    align;		/* alignment of pdst		     */
    CommandWord     mask, leftMask, rightMask;
    long	    maxPixels;
    long	    w;
    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;
    long            signbits = SignBits;

    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
    if (nrectFill == 0 || numRects == 0) return;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;
    pdstBase += yDraw * dstwidth + xDraw * FFBPIXELBYTES;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    /* last ffb register written: planemask */

    FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,
		{ FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		  CYCLE_REGS(ffb); },
		/**/,
		FFBMODE(ffb,ffbFillMode));
    FFBDATA(ffb, ~0);
    
    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--) {

        REGION_RECTS_SETUP(pbox, nrectFill, prectBase, xDraw, yDraw);

        do { /*while nrectFill != 0 */
	    register long x, y;
	    int ymul;

	    CLIP_RECT(prect, dstwidth, ul, lr, signbits,
		      clipx1, clipx2, clipy1, clipy2,
		      x, y, ymul, width, h,
		      RECT_COMPLETELY_CLIPPED);
	    prect++;
	    nrect--;

	    pdstBase = CYCLE_FB(pdstBase);
	    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
	    align = (long)pdst & FFBBUSBYTESMASK;
	    pdst -= align;

	    if (width <= maxPixels ) {
		/* One block fill command per scan line */
		mask = FFBLOADBLOCKDATA(align, width);
		while (h != 0) {
		    FFBWRITE(pdst, mask);
		    pdst += dstwidth;
		    /* no CYCLE_FB b/c dstwidth > CPU_WB_BYTES */
		    h -= 1;
		};
	    } else {
		/* Mask requires multiple words */
		mask = FFBLOADBLOCKDATA(align, maxPixels);
		do {
		    p = pdst;
		    w = width;
		    while( w  > maxPixels) {
			FFBWRITE(p, mask);
			p += maxPixels * FFBPIXELBYTES;
			/* no CYCLE_FB b/c dstwidth > CPU_WB_BYTES */
			w -= maxPixels;
		    }
		    FFBWRITE(p, FFBLOADBLOCKDATA(align, w));
		    pdst += dstwidth;
		    /* I think CYCLE_FB() isn't needed here, since dstwidth
		       is always word-aligned, and fill alignment is 4-pixels
		       for 8-bit, and 1-pixel for 32-bit drawing, ie, 
		       also word-aligned */
		    h--;
		} while (h !=0);
	    } /*if skinny else fat rectangle*/
	} while (nrect != 0);
    } /*for pbox*/
}
#endif /*FFBPOLYFILLRECTSOLID*/


#ifdef FFBSOLIDFILLSPANS
void FFBSOLIDFILLSPANS (pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill	     */
    DDXPointPtr pptInit;		/* pointer to list of start points   */
    int		*pwidthInit;		/* pointer to list of n widths	     */
    int 	fSorted;
{
    register Pixel8	    *pdst;
    Pixel8		    *addrlBase;	/* pointer to start of dst	     */
    long		    nlwidth;	/* width in pixels of dst	     */
    cfbPrivGC		    *gcPriv;
    ffbScreenPrivPtr	    scrPriv;
    FFB			    ffb;
    long		    maxPixels;
    BoxPtr		    pbox;
    RegionPtr		    prgnClip;
    long		    numRects;	/* Number of clipping rectangles     */
    long		    xDraw, yDraw;
    long		    signbits = SignBits;

    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    if (nInit == 0 | numRects == 0) return;

    DrawableBaseAndWidth(pDraw, addrlBase, nlwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;
    addrlBase += yDraw * nlwidth + xDraw * FFBPIXELBYTES;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    /* last ffb register written: planemask */

    FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,
		{ FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		  CYCLE_REGS(ffb); },
		/**/,
		FFBMODE(ffb, ffbFillMode));
    FFBDATA(ffb, ~0);


    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_SPANS_SETUP(pbox, nInit, pptInit, pwidthInit, xDraw, yDraw);

	do
	{
	    register long x, y, w;
	    int ymul;

	    CLIP_SPANS(ppt,nlwidth,pwidth,ul,lr,signbits, /* in */
		       clipx1,clipx2,clipy1,clipy2,	  /* in */
		       x,y,ymul,w,			  /* out */
		       SPAN_COMPLETELY_CLIPPED);
	    pwidth++;
	    ppt++;
	    n--;
	    addrlBase = CYCLE_FB(addrlBase);
	    pdst = addrlBase + ymul + x * FFBPIXELBYTES;
	    FFBSOLIDSPAN(ffb, pdst, w, maxPixels);
	} while (n > 0);
    }
}
#endif /*FFBSOLIDFILLSPANS*/


/* 
 * Opaque stipple pstipple onto the destination drawable.  Unlike the stippled
 * rectangle cases, we don't have to worry about repeating horizontally and
 * vertically. 
 */
#ifdef FFBOSPLANE
void FFBOSPLANE (pDraw, pGC, pstipple, xorg, yorg, prgn)
    DrawablePtr pDraw;
    GCPtr	pGC;
    PixmapPtr	pstipple;
    int		xorg, yorg;
    RegionPtr   prgn;
{
    long		nbox;
    BoxPtr		pbox;
    register long	width;      /* width of current rect in bytes	    */
    register long	height;     /* height of current rect		    */
    register long	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register Pixel8	*p;	    /* pointer to pixels we're writing      */

    long		stippleWidth; /* Width in bits of data in stipple   */
    long		stippleHeight;/* Height in bits of data in stipple  */
    long		srcWidth;   /* Stipple width in physical words      */
    CommandWord		*psrcBase;  /* Pointer to stipple bits		    */
    CommandWord		*psrc;      /* Pointer to word in stipple bits      */
    long		ix, iy;     /* Rectangle offset into stipple 	    */
    long		ixBUSBITS;  /* FFBBUSBITS - ix			    */
    long		align;	    /* Pixel offsets for left alignment     */
    CommandWord		purebits, srcbits, *ps;
    CommandWord		ones = ffbStippleAll1;
    CommandWord		mask, leftMask, rightMask;
    long		m,h;
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    cfbPrivGC		*gcPriv;
    FFBMode		lastMode;
    int                 canBlock;
 
    nbox = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);

    psrcBase	    = (CommandWord *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind / FFBBUSBYTES;
    stippleWidth    = pstipple->drawable.width;

    xorg += pDraw->x;
    yorg += pDraw->y;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    /* last ffb reg written: planemask */
    
    gcPriv = CFBGCPRIV(pGC);
    lastMode = COPY;	/* mode not used in this routine */
    canBlock = FFBGCBLOCK(pGC);

    /* If we're using background with BLOCKFILL then stipple,
       do all the BG's first */
    do { /*Each rectangle */
	int ymul = pbox->y1 * dstwidth;
        width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;
        if(canBlock && FFB_OS_WIDTH_CHECK(width)) {
	    CYCLE_REGS(ffb);
            FFBMODE(ffb, BLOCKFILL);
            FFBDATA(ffb, ~0);
            FFBLOADCOLORREGS(ffb, gcPriv->and, pDraw->depth); 
	    h = height;
	    pdstBase = CYCLE_FB_DOUBLE(pdstBase);
     	    pdst = pdstBase + ymul + pbox->x1 * FFBPIXELBYTES;
	    align = (long)pdst & FFBBUSBYTESMASK;
	    p = pdst-align;
	    mask = FFBLOADBLOCKDATA(align,width);
	    do { /*fill in the background */
	        FFBWRITE(p, mask);
	        h--;
		p += dstwidth;
	    } while (h != 0);
    	    CYCLE_REGS(ffb);
            FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE);
	    lastMode = TRANSPARENTBLOCKSTIPPLE;
            FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth); 
	    ix = pbox->x1 - xorg;
	    iy = pbox->y1 - yorg;
	    psrc = psrcBase + (iy * srcWidth) + (ix / FFBBUSBITS);
	    /* Align everything.  This may cause ix to wrap around to a negative
	       number, which nominally means we should back up psrc as well.  But
	       we won't: we don't actually need the source data, because it
	       will be masked out. */
	    align = ((long) pdst) & FFBSTIPPLEALIGNMASK;
	    p = (pdst -= align);
	    FFBBYTESTOPIXELS(align);
	    ix = (ix - align) & FFBBUSBITSMASK;
	    ixBUSBITS = FFBBUSBITS - ix;
	    width += align;
	    leftMask = FFBLEFTSTIPPLEMASK(align, ones);
	    rightMask = FFBRIGHTSTIPPLEMASK(width, ones);
	    if (ix == 0) {
		    /* 1/32 chance, but lets us optimize ix != 0 case */
	        do { /*For each line of height */
		    ps = psrc;
		    p = pdst;
    
		    /* Ragged left edge */
		    purebits = *ps++;
		    FFBWRITE(p, purebits & leftMask);
		    /* pre-load before entering loop */
		    purebits = *ps++;
		    for (m = width - 2*FFBSTIPPLEBITS; m > 0;
		        m -= FFBSTIPPLEBITS) {
			p += FFBSTIPPLEBYTESDONE;
			FFBWRITE(p, purebits);
			purebits = *ps++;
		    }  
		    /* Ragged right bits */
		    p += FFBSTIPPLEBYTESDONE;
		    FFBWRITE(p, purebits & rightMask);
		    psrc += srcWidth;
		    pdst += dstwidth;
		    height--;
		} while (height != 0);
	    } else { /*ix != 0, 31/32 chance */
	        do { /*For each line of height */
		    ps = psrc;
		    p = pdst;
    
		    srcbits = 0;
		    /* Fetch source, unless we'd just throw it away because
		       ix wrapped to negative */
		    if (align < ixBUSBITS) {
		        purebits = *ps++;
			srcbits = purebits >> ix;
		    }
		    /* We need a few more bits */
		    purebits = *ps++;
		    srcbits |= (purebits << ixBUSBITS);
		    FFBWRITE(p, srcbits & leftMask);

		    for (m = width - 2*FFBSTIPPLEBITS;
		         m > 0; m -= FFBSTIPPLEBITS) {
			p += FFBSTIPPLEBYTESDONE;
                        srcbits = purebits >> ix;
			/* Get a few more bits, merge with srcbits */
			purebits = *ps++;
			srcbits |= (purebits << ixBUSBITS);
			FFBWRITE(p, srcbits);
		    }
		    /* Ragged right bits */
		    srcbits = purebits >> ix;
		    m += FFBSTIPPLEBITS;
		    if (m > ixBUSBITS) {
		    /* Get a few more bits, merge with srcbits */
		        purebits = *ps;
			srcbits |= (purebits << ixBUSBITS);
		    }
		    p += FFBSTIPPLEBYTESDONE;
		    FFBWRITE(p, srcbits & rightMask);
		    psrc += srcWidth;
		    pdst += dstwidth;
		    height--;
		} while (height != 0);
	    } /*end if ix==0 else */
        } else { /*else if GCBLOCK && OS_WIDTH_CHECK */
	    if (lastMode != OPAQUESTIPPLE){
		lastMode = OPAQUESTIPPLE;
		CYCLE_REGS(ffb);
    	        FFBMODE(ffb, OPAQUESTIPPLE);
	    }
	    pdstBase = CYCLE_FB(pdstBase);
     	    pdst = pdstBase + ymul + pbox->x1 * FFBPIXELBYTES;
	    ix = pbox->x1 - xorg;
	    iy = pbox->y1 - yorg;
	    psrc = psrcBase + (iy * srcWidth) + (ix / FFBBUSBITS);
	    /* Align everything.  This may cause ix to wrap around to a negative
	       number, which nominally means we should back up psrc as well.
	       But we won't: we don't actually need the source data, because it
	       will be masked out. */
	    align = ((long) pdst) & FFBSTIPPLEALIGNMASK;
	    pdst -= align;

	    FFBBYTESTOPIXELS(align);
	    ix = (ix - align) & FFBBUSBITSMASK;
	    ixBUSBITS = FFBBUSBITS - ix;
	    width += align;
	    leftMask = FFBLEFTSTIPPLEMASK(align, ones);
	    rightMask = FFBRIGHTSTIPPLEMASK(width, ones);

	    if (width <= FFBSTIPPLEBITS) {
	        /* One word of data bits, one write to ffb. */
	        mask = leftMask & rightMask;
		CYCLE_REGS(ffb);
                FFBPERSISTENTPIXELMASK(ffb, mask); 
	        do { /*For each line of height */
		    ps = psrc;
		    srcbits = 0;
		    /* Fetch source, unless we'd just throw it away because
		        ix wrapped to negative */
		    if (align < ixBUSBITS) {
		        purebits = *ps++;
			srcbits = purebits >> ix;
		    }
		    if (width > ixBUSBITS) {
		        /* We need a few more bits */
		        purebits = *ps;
		        srcbits |= (purebits << ixBUSBITS);
		    }
		    FFBWRITEONEWORD(ffb, pdst, srcbits);
		    psrc += srcWidth;
		    pdst += dstwidth;
		    height--;
	        } while (height != 0);
	    } else {
	        if (ix == 0) {
		    /* 1/32 chance, but lets us optimize ix != 0 case */
		    /* nb: dstwidth can be such that p+FFBSTIPPLEBYTESDONE and
		     * pdst+dstwidth are in the same wb line, with an intervening
		     * FFBPIXELMASK.  not good?
		     */
		    do { /*For each line of height */
		        ps = psrc;
		        p = pdst;
    
		        /* Ragged left edge */

		        CYCLE_REGS(ffb); 
			FFBPIXELMASK(ffb, leftMask); 
		        purebits = *ps++;
		        FFBWRITE(p, purebits);
		        /* We need a few more bits */
		        purebits = *ps++;
		        for (m = width - 2*FFBSTIPPLEBITS; m > 0;
			    m -= FFBSTIPPLEBITS) {
			    p += FFBSTIPPLEBYTESDONE;
			    FFBWRITE(p, purebits);
			    purebits = *ps++;
		        }
			/* Ragged right bits */
			CYCLE_REGS(ffb);
		        FFBPIXELMASK(ffb, rightMask); 
		        FFBWRITE(p+FFBSTIPPLEBYTESDONE, purebits);
		        psrc += srcWidth;
		        pdst += dstwidth;
			pdst = CYCLE_FB(pdst);
		        height--;
		    } while (height != 0);
	        } else { /*ix != 0, 31/32 chance */
		    do { /*For each line of height */
		        ps = psrc;
		        p = pdst;
			CYCLE_REGS(ffb);
		        FFBPIXELMASK(ffb, leftMask); 
    
		        srcbits = 0;
		        /* Fetch source, unless we'd just throw it away because
		           ix wrapped to negative */
		        if (align < ixBUSBITS) {
			    purebits = *ps++;
			    srcbits = purebits >> ix;
		        }
		        /* We need a few more bits */
		        purebits = *ps++;
		        srcbits |= (purebits << ixBUSBITS);
		        FFBWRITE(p, srcbits);

		        for (m = width - 2*FFBSTIPPLEBITS;
			     m > 0; m -= FFBSTIPPLEBITS) {
			    p += FFBSTIPPLEBYTESDONE;
			    srcbits = purebits >> ix;
			    /* Get a few more bits, merge with srcbits */
			    purebits = *ps++;
			    srcbits |= (purebits << ixBUSBITS);
			    FFBWRITE(p, srcbits);
		        }
		        /* Ragged right bits */
		        srcbits = purebits >> ix;
		        m += FFBSTIPPLEBITS;
		        if (m > ixBUSBITS) {
			    /* Get a few more bits, merge with srcbits */
			    purebits = *ps;
			    srcbits |= (purebits << ixBUSBITS);
		        }
			CYCLE_REGS(ffb);
		        FFBPIXELMASK(ffb, rightMask); 
		        FFBWRITE(p+FFBSTIPPLEBYTESDONE, srcbits);
		        psrc += srcWidth;
		        pdst += dstwidth;
		        height--;
		    } while (height != 0);
	        }
	    } /*end if narrow or wide rectangle */
	} /*end if GCBLOCK && OS_WIDTH_CHECK */
	pbox++;
	nbox--;
    } while (nbox > 0);
    CYCLE_REGS(ffb);
    FFBPIXELMASK(ffb, FFBBUSALL1);
}
#endif /*FFBOSPLANE*/


/* 
 * Transparent or opaque stipple a list of rects
 */

/* ||| Right now we're just going to slime our way out of this, and base the ffb
   code heavily upon the existing cfb code.  This could be much improved by
   trying to paint things more like text does...paint up to a boundary, then
   paint full FFBBUSBITS words (regardless of how narrow the stipple is), then
   paint the final few bits. */
   
#ifdef FFBSTIPPLEAREA
void FFBSTIPPLEAREA(pDraw, pGC, nrectFill, prectBase)
    DrawablePtr		pDraw;
    int			nrectFill;
    xRectangle		*prectBase;
    GCPtr		pGC;
{
    register long	width, w;	/* width of current rect	     */
    register long	height;		/* height of current rect	     */
    register long	dstwidth;	/* width of drawable		     */
    register Pixel8	*pdstBase;	/* pointer to start pixel of drawable*/
    register Pixel8	*pdst;		/* pointer to start pixel of row     */
    register Pixel8	*p;		/* pointer to pixels we're writing   */

    PixmapPtr		pstipple;
    long		stippleWidth;	/* Width in bits of data in stipple  */
    long		stippleHeight;	/* Height in bits of data in stipple */
    long		srcWidth;	/* Stipple width in physical words   */
    CommandWord		*psrcBase;	/* Pointer to stipple bits	     */
    CommandWord		*psrc;		/* Pointer to word in stipple bits   */
    long		xorg, yorg;	/* Drawable offset into stipple	     */
    long		ix, iy;		/* Rectangle offset into stipple     */
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    RegionPtr       	prgnClip;
    long	    	xDraw, yDraw;
    long	    	numRects;	/* Number of clipping rectangles     */
    BoxPtr	    	pbox;		/* pointer to one region clip box    */
    long            	signbits = SignBits;
#ifdef DOOPAQUESTIPPLE
    CommandWord		ones = ffbStippleAll1;
    CommandWord		leftMask, rightMask, mask;
    long		align;
    int			canBlock = FFBGCBLOCK(pGC);
#else
    CommandWord		ones = ffbBusAll1;
#endif
    cfbPrivGC           *gcPriv;
    FFBMode	        lastMode;

    gcPriv          = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    if (nrectFill == 0 | numRects == 0) return;

    pstipple	    = pGC->stipple;
    psrcBase	    = (CommandWord *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind / FFBBUSBYTES;
    stippleWidth    = pstipple->drawable.width;
    stippleHeight   = pstipple->drawable.height;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = (pGC->patOrg.x % stippleWidth) - stippleWidth;
    yorg = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    lastMode = COPY;	/* mode not used in this routine */
    p = (pdstBase += yDraw * dstwidth + xDraw * FFBPIXELBYTES);

#ifdef DOTRANSPARENTSTIPPLE
    if (FFBGCBLOCK(pGC)) {
	FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE);
	FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
#endif
	for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
	{
	    REGION_RECTS_SETUP(pbox, nrectFill, prectBase, xDraw, yDraw);

	    do { /*Each rectangle */
		register long x, y;
		int ymul;
		CommandWord *psrcStart;

		CLIP_RECT(prect, dstwidth, ul, lr, signbits, /* in */
			  clipx1, clipx2, clipy1, clipy2, /* in */
			  x, y, ymul, width, height, /* out */
			  RECT_COMPLETELY_CLIPPED);
		prect++;
		nrect--;

#ifdef DOOPAQUESTIPPLE
		if (canBlock && FFB_OS_WIDTH_CHECK(width)) {
		    long h;
		    ix = (x - xorg) % stippleWidth;
		    iy = (y - yorg) % stippleHeight;
		    psrcStart = psrcBase + (iy * srcWidth);
		    /* change mode for background */
		    CYCLE_REGS(ffb);
		    FFBMODE(ffb, BLOCKFILL);
		    FFBDATA(ffb, ~0);
		    FFBLOADCOLORREGS(ffb, gcPriv->and, pDraw->depth); 
		    h = height;
		    /* one for background, one for foreground */
		    pdstBase = CYCLE_FB_DOUBLE(pdstBase);
		    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
		    align = (long)pdst & FFBBUSBYTESMASK;
		    mask = FFBLOADBLOCKDATA(align,width);
		    p = pdst-align;
		    /* fill in the background */
		    do {
			FFBWRITE(p, mask);
			p += dstwidth;
		    } while (--h > 0);
		    CYCLE_REGS(ffb);
		    lastMode = TRANSPARENTBLOCKSTIPPLE;
		    FFBMODE(ffb, lastMode);
		    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		    /* 32-bit writes not enabled, but if it is, then there needs to be a cycle_fb here, to go with the
		       cycle_fb_double above, since there's none in the loop below; and either the cycle_fb above should
		       be cycle_fb_bleh, or the cycle_fb for finishing off rect also cycle_fb(pdstbase). */
#else
		    iy = (y - yorg) % stippleHeight;
		    psrcStart = psrcBase + (iy * srcWidth);
		    ix = (x - xorg) % stippleWidth;
		    pdstBase = CYCLE_FB(pdstBase);
		    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
#endif
		    /*
		     * this whole loop is doing transparent stipples, so it should
		     * be the same for OS and TS, since OS had the background filled
		     * already.
		     */
#if 0 && LONG_BIT==64
		    /* this version is optimized to remove extra sfb+ writes (ie, 2 per 32 pixels).
		     * it's not enabled, until we get a developer's sandbox set up.  also, this version
		     * depends on a change to ffbPadPixmap/ffbValidateGC wrt special 64-bit padding.
		     * this code should go into stipple-span also, but we'll replicate it there when it's
		     * enabled, rather than pollute this file with #if 0'd code.
		     */
		    {
			long align = (long)pdst & FFBSTIPPLEALIGNMASK;
			FFBBYTESTOPIXELS(align);

			do { /*For each line of height */
			    register long w; 	     /* rect width left */
			    register long ixBUSBITS; /* useful bits from current word */
			    register long srcw;
			    register CommandWord srcbits, newbits;
			    register CommandWord hangover = 0; /* leftover bits from previous read */
			    long tix;		     /* bits already included in previous write */

			    psrc = psrcStart + (ix / FFBBUSBITS);
			    srcw = stippleWidth - ix;
			    w = width;
			    p = pdst;

			    /*
			     * we guarantee that srcbits will have FFBBUSBITS of valid data 
			     * in it, always.  consequently, alignment never changes...
			     */
			    p -= align;

			    tix = ix & FFBBUSBITSMASK; /* unuseful bits from 1st word */
			    newbits = psrc[0] >> tix; /* prime the loop */
			    ixBUSBITS = FFBBUSBITS - tix; /* useful bits so far */

			    for (;;)
			    {
				if (ixBUSBITS >= srcw) {
				    /* Oops, ran out of stipple bits...*/
				    if (srcw <= 0 /*&& srcw > -32*/) {
					/*
					 * already at or past the official end of stipple, so re-prime the
					 * pump with stuff from the beginning...
					 * 
					 */
					srcbits = psrcStart[0];
					psrc = psrcStart +1;
					newbits = psrc[0];
					tix = (-srcw) /*% stippleWidth*/;
					ixBUSBITS = FFBBUSBITS - tix; /* useful bits so far */
					srcbits = (srcbits >> tix) | (newbits << ixBUSBITS);
					srcw = stippleWidth - tix - FFBBUSBITS;
				    }
				    else {
					long six;
					/*
					 * need tix bits from 1st word of stipple scanline, starting from bit 'six'.
					 * six+tix <= FFBBUSBITS, so don't worry about needing 2nd word.
					 */
					srcbits = newbits;
					psrc = psrcStart;
					newbits = psrc[0];
					six = (ixBUSBITS-srcw) /*% stippleWidth*/;
					srcbits |= (newbits >> six) << ixBUSBITS;
					srcw = stippleWidth - six;
					/* now reset parameters for beginning of stipple */
					tix = six & FFBBUSBITSMASK; /* unuseful bits from this word */
					ixBUSBITS = FFBBUSBITS - tix; /* useful bits so far */
				    }
				}
				else {
				    srcbits = newbits;
				    newbits = psrc[1];
				    srcbits |= newbits << ixBUSBITS;
				    psrc++;
				    srcw -= FFBBUSBITS;
				}
				newbits >>= tix;

				if (FFBBUSBITS >= w) {
				    /* Oops, ran out of rectangle */
				    srcbits &= FFBRIGHTBUSMASK(w, ones);
				    FFBWRITE(p, (srcbits << align) | hangover);
				    if (w+align > FFBSTIPPLEBITS) {
					/* Finish off srcbits */
					FFBWRITE(p+FFBSTIPPLEBYTESDONE,
						 srcbits >> FFBRIGHTSTIPPLESHIFT(align));
					pdst = CYCLE_FB(pdst);
				    }
				    break; /*loop terminator*/
				}

				/* Paint srcbits */
				FFBWRITE(p, (srcbits << align) | hangover);
				hangover = srcbits >> FFBRIGHTSTIPPLESHIFT(align);

				w -= FFBBUSBITS;
				p += FFBSTIPPLEBYTESDONE;

			    } /*end for loop */
			    iy++;
			    psrcStart += srcWidth;
			    if (iy == stippleHeight) {
				iy = 0;
				psrcStart = psrcBase;
			    }
			    pdst += dstwidth;
			    height--;
			} while (height > 0);
		    }
#else
		    do { /*For each line of height */
			register int w, tix, ixBUSBITS, srcw, align;
			register CommandWord srcbits;

			psrc = psrcStart + (ix / FFBBUSBITS);
			srcw = stippleWidth - ix;
			w = width;
			p = pdst;
			tix = ix & FFBBUSBITSMASK;
			srcbits = (*psrc) >> tix;
			ixBUSBITS = FFBBUSBITS - tix;

			for (;;) {
			    p = CYCLE_FB(p);
			    if (ixBUSBITS > srcw) {
				/* Oops, ran out of stipple bits. */
				ixBUSBITS = srcw;
				srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
			    }
			    if (ixBUSBITS > w) {
				/* Oops, ran out of rectangle */
				ixBUSBITS = w;
				srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
			    }
    
			    align = (long) p & FFBSTIPPLEALIGNMASK;
			    p -= align;
			    FFBBYTESTOPIXELS(align);

			    /* Paint srcbits */
			    FFBWRITE(p, srcbits << align);
			    if (ixBUSBITS+align > FFBSTIPPLEBITS) {
				/* Finish off srcbits */
				FFBWRITE(p+FFBSTIPPLEBYTESDONE,
					 srcbits >> FFBRIGHTSTIPPLESHIFT(align));
			    }
			    w -= ixBUSBITS;
			    if (w == 0) break; /* LOOP TERMINATOR */

			    p += (ixBUSBITS+align) * FFBPIXELBYTES;
			    srcw -= ixBUSBITS;

			    ixBUSBITS = FFBBUSBITS;
			    psrc++;
			    if (srcw == 0) {
				srcw = stippleWidth;
				psrc = psrcStart;
			    }
			    srcbits = *psrc;
			} /*end for loop */
			iy++;
			psrcStart += srcWidth;
			if (iy == stippleHeight) {
			    iy = 0;
			    psrcStart = psrcBase;
			}
			pdst += dstwidth;
			height--;
		    } while (height > 0);
#endif
#ifdef DOOPAQUESTIPPLE
		} else { /*else FFB_OS_WIDTH_CHECK */
		    if (lastMode != OPAQUESTIPPLE) {
			lastMode = OPAQUESTIPPLE;
			CYCLE_REGS(ffb);
			FFBMODE(ffb, OPAQUESTIPPLE);
		    }
		    /* fall thru to normal stippling */
#else
	    } while (nrect > 0);
	} /* for pbox */
    } else { /* ~FFBGCBLOCK(pGC) */
	FFBMODE(ffb, TRANSPARENTSTIPPLE);
	for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
	{
	    REGION_RECTS_SETUP(pbox, nrectFill, prectBase, xDraw, yDraw);
	    do {
		register long x, y;
		int ymul;
		CommandWord *psrcStart;

		CLIP_RECT(prect,dstwidth, ul,lr,signbits,
			  clipx1,clipx2,clipy1,clipy2,
			  x, y, ymul, width,height,
			  RECT_COMPLETELY_CLIPPED);
		prect++;
		nrect--;
#endif
		    /* normal stippling */

		    ix = (x - xorg) % stippleWidth;
		    iy = (y - yorg) % stippleHeight;

		    psrcStart = psrcBase + (iy * srcWidth);

		    pdstBase = CYCLE_FB(pdstBase);
		    pdst = pdstBase + ymul + x * FFBPIXELBYTES;

		    do { /*For each line of height */
			register int w, tix, ixBUSBITS, srcw, align;
			register CommandWord srcbits;

			psrc = psrcStart + (ix / FFBBUSBITS);
			srcw = stippleWidth - ix;
			w = width;
			p = pdst;
			tix = ix & FFBBUSBITSMASK;
			srcbits = (*psrc) >> tix;
			ixBUSBITS = FFBBUSBITS - tix;

			for (;;) {
			    p = CYCLE_FB(p);
			    if (ixBUSBITS > srcw) {
				/* Oops, ran out of stipple bits. */
				ixBUSBITS = srcw;
#ifdef DOTRANSPARENTSTIPPLE
				srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
			    }
			    if (ixBUSBITS > w) {
				/* Oops, ran out of rectangle */
				ixBUSBITS = w;
#ifdef DOTRANSPARENTSTIPPLE
				srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
			    }
    
			    align = (long) p & FFBSTIPPLEALIGNMASK;
			    p -= align;
			    FFBBYTESTOPIXELS(align);

			    /* Paint srcbits */
#ifdef DOOPAQUESTIPPLE
			    {
				long tw = ixBUSBITS + align;
				leftMask = FFBLEFTSTIPPLEMASK(align, ones);
				rightMask = FFBRIGHTSTIPPLEMASK(tw, ones);
				CYCLE_REGS(ffb);
				if (tw <= FFBSTIPPLEBITS) {
				    FFBPIXELMASK(ffb, leftMask & rightMask);
				    FFBWRITE(p, srcbits << align);
				} else {
				    FFBPIXELMASK(ffb, leftMask);
				    FFBWRITE(p, srcbits << align);
				    CYCLE_REGS(ffb);
				    FFBPIXELMASK(ffb, rightMask);
				    FFBWRITE(p+FFBSTIPPLEBYTESDONE,
					     srcbits >> FFBRIGHTSTIPPLESHIFT(align));
				}
			    }
#else
			    FFBWRITE(p, srcbits << align);
			    if (ixBUSBITS+align > FFBSTIPPLEBITS) {
				/* Finish off srcbits */
				FFBWRITE(p+FFBSTIPPLEBYTESDONE,
					 srcbits >> FFBRIGHTSTIPPLESHIFT(align));
			    }
#endif
			    w -= ixBUSBITS;
			    if (w == 0) break; /* LOOP TERMINATOR */

			    p += (ixBUSBITS+align) * FFBPIXELBYTES;
			    srcw -= ixBUSBITS;

			    ixBUSBITS = FFBBUSBITS;
			    psrc++;
			    if (srcw == 0) {
				srcw = stippleWidth;
				psrc = psrcStart;
			    }
			    srcbits = *psrc;
			} /*end for loop */

			iy++;
			psrcStart += srcWidth;
			if (iy == stippleHeight) {
			    iy = 0;
			    psrcStart = psrcBase;
			}
			pdst += dstwidth;
			height--;
		    } while (height > 0);
#ifdef DOOPAQUESTIPPLE
		} /*end OS_WIDTH_CHECK*/
	    } while (nrect > 0);
    } /*end for pbox*/
#else /* DOTRANSPARENTSTIPPLE */
	    } while (nrect > 0);
	} /* for pbox */
    } /* if FFBGCBLOCK(pGC) ... else ... */
#endif
}
#endif /*FFBSTIPPLEAREA*/


/*
 * Fill spans with transparent or opaque stipples that aren't FFBBUSBITS wide
 */
#ifdef FFBSTIPPLESPAN
void FFBSTIPPLESPAN(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDraw;
    GC		*pGC;
    int		nInit;		/* number of spans to fill		    */
    DDXPointPtr pptInit;	/* pointer to start points		    */
    int		*pwidthInit;	/* pointer to widths			    */
    int		fSorted;
{
    CommandWord     srcbits;
    CommandWord	    *psrcStart;
    CommandWord	    *psrc;
    register Pixel8 *p;			/* pointer to current byte in bitmap */
    register long   ix, ixBUSBITS, srcw, align;
    long	    iy;			/* first line of stipple to use	     */
    Pixel8	    *addrlBase;		/* pointer to start of bitmap	     */
    long	    nlwidth;		/* width in longwords of bitmap	     */
    PixmapPtr       pStipple;		/* pointer to stipple we want to fill with*/
    long	    xorg, yorg;
    long	    stwidth;		/* Width of stipple in words	     */
    long	    stippleWidth;	/* Width in bits of data in stipple  */
    long	    stippleHeight;	/* Height in bits of data in stipple */
    cfbPrivGC       *gcPriv;
    ffbScreenPrivPtr scrPriv;
    FFBMode	    lastMode;
    FFB		    ffb;
#ifdef DOOPAQUESTIPPLE
    CommandWord     ones = ffbStippleAll1;
    CommandWord	    leftMask, rightMask;
    long	    tw,mask;
    Pixel8 	    *pdst;
    int		    canBlock = FFBGCBLOCK(pGC);
#endif
#ifdef DOTRANSPARENTSTIPPLE
    CommandWord     ones = ffbBusAll1;
#endif
    BoxPtr  	    pbox;
    RegionPtr	    prgnClip;
    long    	    numRects;		/* Number of clipping rectangles     */
    long    	    signbits = SignBits;
    long    	    xDraw, yDraw;

    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    if (nInit == 0 | numRects == 0) return;

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

    stwidth = pStipple->devKind / FFBBUSBYTES;
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;

    /*
     *	The Target:
     *		Depth = FFBPIXELBITS
     *		Width = determined from *pwidth
     *		Words per scanline = nlwidth
     *		Pointer to pixels = addrlBase
     */

    DrawableBaseAndWidth(pDraw, addrlBase, nlwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = (pGC->patOrg.x % stippleWidth) - stippleWidth;
    yorg = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    lastMode = COPY;

    addrlBase += yDraw * nlwidth + xDraw * FFBPIXELBYTES;

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_SPANS_SETUP(pbox, nInit, pptInit, pwidthInit, xDraw, yDraw);

#ifdef DOTRANSPARENTSTIPPLE
	if (FFBGCBLOCK(pGC)) {
	    FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE);
	    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth); 
#endif
	    do {
		register long x, y, w;
		int ymul, iymul;

		CLIP_SPANS(ppt,nlwidth,pwidth,ul,lr,signbits, /* in	     */
			   clipx1,clipx2,clipy1,clipy2,	      /* in	     */
			   x,y,ymul,w,			      /* out	     */
			   SPAN_COMPLETELY_CLIPPED);
		pwidth++;
		ppt++;
		n--;
#ifdef DOOPAQUESTIPPLE
		if (canBlock && FFB_OS_WIDTH_CHECK(w)){
/* ||| Don't think this should cycle double here, really */
		    iy = (y - yorg) % stippleHeight;
		    iymul = iy * stwidth;
		    addrlBase = CYCLE_FB(addrlBase);
#else
		    iy = (y - yorg) % stippleHeight;
		    iymul = iy * stwidth;
		    addrlBase = CYCLE_FB(addrlBase);
#endif
		    ix = (x - xorg) % stippleWidth;
		    srcw = stippleWidth - ix;
		    ix &= FFBBUSBITSMASK;
		    ixBUSBITS = FFBBUSBITS - ix;
		    p = addrlBase + ymul + x * FFBPIXELBYTES;
		    psrcStart = 
			(CommandWord *) pStipple->devPrivate.ptr + iymul;
		    psrc = psrcStart + (ix / FFBBUSBITS);
		    srcbits = (*psrc) >> ix;
#ifdef DOOPAQUESTIPPLE
		    CYCLE_REGS(ffb);
		    FFBMODE(ffb, BLOCKFILL);
		    FFBDATA(ffb, ~0);
		    FFBLOADCOLORREGS(ffb, gcPriv->and, pDraw->depth); 
		    /* fill in the background */
		    align = (long)p & FFBBUSBYTESMASK;  
		    pdst = p - align;
		    FFBWRITE(pdst, FFBLOADBLOCKDATA(align, w));
		    CYCLE_REGS(ffb);
		    FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE);
		    lastMode = TRANSPARENTBLOCKSTIPPLE;
		    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth); 
		    /* Cycle FB because we just wrote block data */
		    p = CYCLE_FB(p);
		    addrlBase = CYCLE_FB(addrlBase);
#endif
		    for (;;) {
			if (ixBUSBITS > srcw) {
			    /* Oops, ran out of stipple bits. */
			    ixBUSBITS = srcw;
			    srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
			}
			if (ixBUSBITS > w) {
			    /* Oops, ran out of rectangle */
			    ixBUSBITS = w;
			    srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
			}
			/* Paint srcbits aligned to FFB boundary */
			align = (long) p & FFBSTIPPLEALIGNMASK;
			p -= align;
			FFBBYTESTOPIXELS(align);
			/* Paint srcbits */
			FFBWRITE(p, srcbits << align);
			if (ixBUSBITS+align > FFBSTIPPLEBITS) {
			    /* Finish off srcbits */
			    FFBWRITE(p+FFBSTIPPLEBYTESDONE,
				     srcbits >> FFBRIGHTSTIPPLESHIFT(align));
			}
			w -= ixBUSBITS;
			if (w == 0) break; /* LOOP TERMINATOR */

			/* If we ran out of src, next write may be to same
			   word. */
			p = CYCLE_FB(p);
			addrlBase = CYCLE_FB(addrlBase);

			p += (ixBUSBITS+align) * FFBPIXELBYTES;
			srcw -= ixBUSBITS;

			ixBUSBITS = FFBBUSBITS;
			psrc++;
			if (srcw == 0) {
			    srcw = stippleWidth;
			    psrc = psrcStart;
			}
			srcbits = *psrc;
		    } /*end for loop */
# ifdef DOOPAQUESTIPPLE
		} else { /*cantblock || < width_check*/
		    if (lastMode != OPAQUESTIPPLE){
			lastMode = OPAQUESTIPPLE;
			CYCLE_REGS(ffb);
			FFBMODE(ffb, OPAQUESTIPPLE);
		    }
		    /* fall thru to normal OS */
#else
		} while (n > 0);
	    } else { /*cantblock*/
		FFBMODE(ffb, TRANSPARENTSTIPPLE);
		do {
		    register long x, y, w;
		    int ymul, iymul;
		    
		    CLIP_SPANS(ppt,nlwidth,pwidth,ul,lr,signbits, /* in	     */
			       clipx1,clipx2,clipy1,clipy2,	  /* in	     */
			       x,y,ymul,w,			  /* out     */
			       SPAN_COMPLETELY_CLIPPED);
		    pwidth++;
		    ppt++;
		    n--;
#endif 
		    iy = (y - yorg) % stippleHeight;
		    iymul = iy * stwidth;
		    ix = (x - xorg) % stippleWidth;

		    addrlBase = CYCLE_FB(addrlBase);
		    srcw = stippleWidth - ix;

		    ix &= FFBBUSBITSMASK;
		    ixBUSBITS = FFBBUSBITS - ix;
		    /* ref. ymul */
		    p = addrlBase + ymul + x * FFBPIXELBYTES;
		    /* ref. iymul */
		    psrcStart =
			(CommandWord *) pStipple->devPrivate.ptr + iymul;
		    psrc = psrcStart + (ix / FFBBUSBITS);
		    srcbits = (*psrc) >> ix;

		    for (;;) {
			if (ixBUSBITS > srcw) {
			    /* Oops, ran out of stipple bits. */
			    ixBUSBITS = srcw;
#ifdef DOTRANSPARENTSTIPPLE
			    srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
			}
			if (ixBUSBITS > w) {
			    /* Oops, ran out of rectangle */
			    ixBUSBITS = w;
#ifdef DOTRANSPARENTSTIPPLE
			    srcbits &= FFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
			}
			/* Paint srcbits aligned to FFB boundary */
			align = (long) p & FFBSTIPPLEALIGNMASK;
			p -= align;
			FFBBYTESTOPIXELS(align);
			/* Paint srcbits */
# ifdef DOOPAQUESTIPPLE
			tw = ixBUSBITS + align;
			leftMask = FFBLEFTSTIPPLEMASK(align, ones);
			rightMask = FFBRIGHTSTIPPLEMASK(tw, ones);
			CYCLE_REGS(ffb);
			if (tw <= FFBSTIPPLEBITS) {
			    FFBPIXELMASK(ffb, leftMask & rightMask);
			    FFBWRITE(p, srcbits << align);
			} else {
			    FFBPIXELMASK(ffb, leftMask);
			    FFBWRITE(p, srcbits << align);
			    CYCLE_REGS(ffb);
			    FFBPIXELMASK(ffb, rightMask);
			    FFBWRITE(p+FFBSTIPPLEBYTESDONE,
				     srcbits >> FFBRIGHTSTIPPLESHIFT(align));
			}
# else
			FFBWRITE(p, srcbits << align);
			if (ixBUSBITS+align > FFBSTIPPLEBITS) {
			    /* Finish off srcbits */
			    FFBWRITE(p+FFBSTIPPLEBYTESDONE,
				     srcbits >> FFBRIGHTSTIPPLESHIFT(align));
			}
# endif
			w -= ixBUSBITS;
			if (w == 0) break; /* LOOP TERMINATOR */

			p += (ixBUSBITS+align) * FFBPIXELBYTES;
			/* If ran out of source, may paint same address twice,
			   or if wrote PIXELMASK may reach back. */
			p = CYCLE_FB(p);
			addrlBase = CYCLE_FB(addrlBase);
			srcw -= ixBUSBITS;
			ixBUSBITS = FFBBUSBITS;
			psrc++;
			if (srcw == 0) {
			    srcw = stippleWidth;
			    psrc = psrcStart;
			}
			srcbits = *psrc;
		    } /*end for loop */
#ifdef DOOPAQUESTIPPLE
		} /*end canblock...*/
#endif
	    } while (n > 0);
#ifdef DOTRANSPARENTSTIPPLE
	}
#endif
    } /*end pbox*/
}
#endif /*FFBSTIPPLESPAN*/


/* 
 * Transparent stipple, opaque stipple a list of rectangles, where
 * the stipple/tile width == FFBBUSBITS.  (The stipple/tile has been replicated
 * if its width was a power of 2 smaller than FFBBUSBITS bits.)
 */

#ifdef FFBTSAREAWORD
#ifndef DOTILE

#define StartStippleLoop(psrc, srcbits, ix, ixBUSBITS)			   \
{									   \
    srcbits = *psrc;							   \
    /* Do alignment of srcbits */					   \
    srcbits = (srcbits << ixBUSBITS) | (srcbits >> ix);			   \
} /* StartStippleLoop */

/* Physical pixmap width is LONG_BIT bits */
#define EndStippleLoop(pdst, dstwidth, psrcBase, psrc, iy, stippleHeight,  \
    height)								   \
{									   \
    pdst += dstwidth;							   \
    iy++;								   \
    psrc += LONG_BIT/FFBBUSBITS;					   \
    if (iy == stippleHeight) {						   \
	iy = 0;								   \
	psrc = psrcBase;						   \
    }									   \
    height--;								   \
} /* EndStippleLoop */


void FFBTSAREAWORD (pDraw, pGC, nrectFill, prectBase)
    DrawablePtr		pDraw;
    int			nrectFill;
    xRectangle		*prectBase;
    GCPtr		pGC;
{
    register long	width;      /* width of current rect		    */
    register long	height;     /* height of current rect		    */
    register long	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register long	align;      /* alignment of pdst (0..FFBSTIPPLEALIGNMASK)  */
    register Pixel8     *p;	    /* Temp dest pointer		    */

    PixmapPtr		pstipple;
    long		stippleHeight;/* Height in bits of data in stipple  */
    CommandWord		*psrcBase;  /* Pointer to base of stipple bits	    */
    CommandWord		*psrc;      /* Pointer to stipple bits for scanline */
    long		xorg, yorg; /* Drawable offset into stipple	    */
    long		ix, iy;     /* Rectangle offset into stipple 	    */
    register long	ixBUSBITS;  /* FFBBUSBITS - ix			    */
    CommandWord		srcbits;    /* Actual source bits for scanline      */
    CommandWord		ones = ffbStippleAll1;
    CommandWord		mask, leftMask, rightMask;
    long		maxPixels;
    long		w;
    long		h; 
    long		m;
    ffbScreenPrivPtr    scrPriv;
    cfbPrivGCPtr 	gcPriv;
    FFBMode		fillmode, stipplemode, lastmode;
    FFB			ffb;
    long	    xDraw, yDraw;
    long            signbits = SignBits;
    long	    numRects;		/* Number of clipping rectangles    */
    BoxPtr	    pbox;		/* pointer to one region clip box   */
    RegionPtr       prgnClip;

    /*
      The trick here is to take advantage of stipple patterns that can be 
      replicated to a width of FFBBUSBITS bits.  Then we can rotate each row of
      the stipple by a good amount, and always use the rotated bits as is.

      For transparent stipples, we use 0 bits at the left and right ragged
      edges.  For opaque stipples, we us the pixel mask register on the FFB.

      */

    gcPriv   = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    if (nrectFill == 0 | numRects == 0) return;

    pstipple      = pGC->stipple;
    psrcBase      = (CommandWord *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = pGC->patOrg.x;
    yorg = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);

#ifdef DOTRANSPARENTSTIPPLE
    FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,
		FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		stipplemode = TRANSPARENTBLOCKSTIPPLE,
		stipplemode = TRANSPARENTSTIPPLE,
		fillmode = ffbFillMode);
#else
    FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,
		stipplemode = TRANSPARENTBLOCKSTIPPLE,
		stipplemode = TRANSPARENTSTIPPLE,
		fillmode = ffbFillMode);
#endif

    lastmode = COPY;    /* Some mode that is never used in this routine */

    pdstBase += yDraw * dstwidth + xDraw * FFBPIXELBYTES;

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_RECTS_SETUP(pbox, nrectFill, prectBase, xDraw, yDraw);
	
#ifdef DOOPAQUESTIPPLE
	/* one-time stuff per mode */
	if (fillmode == BLOCKFILL)
	{
	    xRectangle *tprect = prectBase;
	    int tnrect = nrectFill;

	    lastmode = BLOCKFILL;
	    FFBMODE(ffb, lastmode);
	    FFBLOADCOLORREGS(ffb, gcPriv->and, pDraw->depth);
	    FFBDATA(ffb, ~0);

	    do { /*rect*/
		register long x, y;
		int ymul;

		CLIP_RECT(tprect, dstwidth, ul, lr, signbits,
			  clipx1, clipx2, clipy1, clipy2,
			  x, y, ymul, width, height,
			  tprect++; if (--tnrect > 0) continue; break);
		tprect++;
		tnrect--;

		if ( FFB_OS_WIDTH_CHECK(width))
		{
		    pdstBase = CYCLE_FB(pdstBase);
		    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
		    align = (long)pdst & FFBSTIPPLEALIGNMASK;
		    FFBBYTESTOPIXELS(align);
		    align = (long)pdst & FFBBUSBYTESMASK;
		    pdst -= align;
		    mask = FFBLOADBLOCKDATA(align, FFBMAXBSWPIXELS);
		    do { /*height*/
			p = pdst;
			w = width;
			while (w > FFBMAXBSWPIXELS) {
			    FFBWRITE(p, mask);
			    w -= FFBMAXBSWPIXELS;
			    p += FFBMAXBSWPIXELS * FFBPIXELBYTES;
			}
			FFBWRITE(p, FFBLOADBLOCKDATA(align, w));
			pdst += dstwidth;
			height--;
		    } while (height > 0);
		}
	    } while (tnrect > 0);
	    CYCLE_REGS(ffb);
	    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
	}
#endif

	do { /*Each rectangle */
	    register long x, y;
	    int ymul;

	    CLIP_RECT(prect, dstwidth, ul, lr, signbits,
		      clipx1, clipx2, clipy1, clipy2,
		      x, y, ymul, width, height,
		      RECT_COMPLETELY_CLIPPED);
	    prect++;
	    nrect--;

	    /* Compute offsets into stipple. */
	    ix = (x - xorg);
	    iy = (y - yorg) % stippleHeight;

	    psrc = psrcBase + iy * (LONG_BIT/FFBBUSBITS);

	    CYCLE_REGS(ffb);

	    /* Compute upper left corner. */
	    pdstBase = CYCLE_FB(pdstBase);
	    pdst = pdstBase + ymul + x * FFBPIXELBYTES;

	    /* Set up pattern alignment */
	    align = (long)pdst & FFBSTIPPLEALIGNMASK;
	    FFBBYTESTOPIXELS(align);
	    ix -= align;
	    ComputeRotateAmounts(ix, ixBUSBITS, ix);

	    if (!FFB_OS_WIDTH_CHECK(width))
	    {
		pdst -= align * FFBPIXELBYTES;
		leftMask = FFBLEFTSTIPPLEMASK(align, ones);
		rightMask = FFBRIGHTSTIPPLEMASK((width + align), ones);
		if ((width + align) <= FFBSTIPPLEBITS)
		{
		    mask = leftMask & rightMask;
#ifdef DOOPAQUESTIPPLE
		    {
			if (lastmode != OPAQUESTIPPLE) {
			    lastmode = OPAQUESTIPPLE;
			    FFBMODE(ffb, lastmode);
			}
			FFBPERSISTENTPIXELMASK(ffb, mask); 
			do { /*for each line of height */
			    StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
			    FFBWRITE(pdst, srcbits);
			    EndStippleLoop(pdst, dstwidth, psrcBase, psrc, 
					   iy, stippleHeight, height);
			} while ( height != 0);
		    }
#endif
#ifdef DOTRANSPARENTSTIPPLE
		    {
			if (lastmode != stipplemode) {
			    lastmode = stipplemode;
			    FFBMODE(ffb, lastmode);
			}
			do { /*for each line of height */
			    StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
			    FFBWRITE(pdst, srcbits & mask);
			    EndStippleLoop(pdst, dstwidth, psrcBase, psrc, 
					   iy, stippleHeight, height);
			} while ( height != 0);
		    }
#endif
		}
		else
		{   /*width not super narrow but narrow enough for stipple mode */
		    if (pGC->fillStyle == FillOpaqueStippled) {
			if (lastmode != OPAQUESTIPPLE) {
			    lastmode = OPAQUESTIPPLE;
			    FFBMODE(ffb, lastmode);
			}
			do { /*for each line of height */
			    StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
			    p = pdst;
			    CYCLE_REGS(ffb);
			    FFBPIXELMASK(ffb, leftMask);
			    FFBWRITE(p, srcbits);
			    for (m = width - 2*FFBSTIPPLEBITS; m > 0; m -= FFBSTIPPLEBITS) {
				p += FFBSTIPPLEBYTESDONE;
				FFBWRITE(p, srcbits);
			    }
			    CYCLE_REGS(ffb);
			    FFBPIXELMASK(ffb, rightMask);
			    FFBWRITE(p+FFBSTIPPLEBYTESDONE, srcbits);
			    EndStippleLoop(pdst, dstwidth,
					   psrcBase, psrc, iy, stippleHeight, height);
			} while (height != 0);
		    }
		}
	    }
	    else
	    { /*not narrow enough for stipple mode, use fill mode */
		if (lastmode != fillmode) {
		    lastmode = fillmode;
		    FFBMODE(ffb, lastmode);
		}
	    
		/* Offset pdst so aligned for fill operation. */
		align = (long) pdst & FFBBUSBYTESMASK;
		pdst -= align;

		if (width <= maxPixels) {
		    mask = FFBLOADBLOCKDATA(align,width);
		    /* One write per scan line */
		    do { /*For each line of height */
			StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
			CYCLE_REGS(ffb);
			FFBDATA(ffb, srcbits);
			FFBWRITE(pdst, mask);
			EndStippleLoop(pdst, dstwidth,
				       psrcBase, psrc, iy, stippleHeight, height);
		    } while (height != 0);
		} else {
		    /* Multi-word mask */
		    mask = FFBLOADBLOCKDATA(align, maxPixels);
		    do { /*For each line of height */
			StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
			p = pdst;
			w = width;
			CYCLE_REGS(ffb);
			FFBDATA(ffb, srcbits);
			while (w > maxPixels) {
			    FFBWRITE(p, mask);
			    p += maxPixels * FFBPIXELBYTES;
			    w -= maxPixels;
			}
			FFBWRITE(p, FFBLOADBLOCKDATA(align,  w));
			EndStippleLoop(pdst, dstwidth, 
				       psrcBase, psrc, iy, stippleHeight, height);
		    } while (height != 0);
		} /*if skinny else fat rectangle */
	    } /*if super skinny else fat rectangle */
	} while (nrect > 0);
    }
    CYCLE_REGS(ffb);
    FFBPIXELMASK(ffb, FFBBUSALL1);
}
#endif /*DOTILE*/
#endif /*FFBTSAREAWORD*/


/* 
 * Transparent stipple, or opaque stipple a list of spans, where
 * the stipple width == FFBBUSBITS.  (The stipple has been replicated
 * if its width was a power of 2 smaller than FFBBUSBITS bits.)
 */
#ifdef FFBTSSPANWORD
void FFBTSSPANWORD (pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr		pDraw;
    GCPtr		pGC;
    int			nInit;	    /* number of spans to fill		    */
    DDXPointPtr		pptInit;    /* list of start points		    */
    int			*pwidthInit;/* list of widths			    */
    int			fSorted;
{
    register long	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*p;	    /* pointer to start pixel of span	    */
	
    PixmapPtr		pstipple;
    long		stippleHeight;/* Height in bits of data in stipple  */
    CommandWord		*psrcBase;  /* Pointer to base of stipple bits	    */
    long		xorg, yorg; /* Drawable offset into stipple	    */
    long		ix, iy;     /* Span offset into stipple 	    */
    register int	ixBUSBITS;  /* FFBBUSBITS - ix			    */
    long		align;
    CommandWord		srcbits;    /* Actual source bits for scanline      */
    CommandWord		mask, leftMask, rightMask;
    cfbPrivGC		*gcPriv;
    ffbScreenPrivPtr    scrPriv;
    FFBMode		fillmode;
    FFB			ffb;
    long		maxPixels;
    BoxPtr  	    	pbox;
    RegionPtr	    	prgnClip;
    long    	    	numRects;   /* Number of clipping rectangles	     */
    long    	    	signbits = SignBits;
    long    	    	xDraw, yDraw;

    /*
      The trick here is to take advantage of stipple patterns that can be
      replicated to a width of FFBBUSBITS.  Then we can rotate each row of the
      stipple by a good amount, and always use the rotated bits as is.  We 
      solve initial alignment problems by backing up each row to a word 
      boundary, and substituting 0 bits at the bottom of the stipple the very
      first time through the loop.  We solve ending alignment problems by 
      substituting 0's for the high 1-31 bits of the stipple.
      */
    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    if (nInit == 0 | numRects == 0) return;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;
    pdstBase += yDraw * dstwidth + xDraw * FFBPIXELBYTES;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);

    pstipple = pGC->stipple;

    psrcBase      = (CommandWord *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = (pGC->patOrg.x);
    yorg = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    /* one-time stuff for each variant */
#ifdef DOTRANSPARENTSTIPPLE
    FFBFILLMODE(pDraw, pGC, FALSE, maxPixels,
		FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);CYCLE_REGS(ffb)
		,
		/* non-block */,
		FFBMODE(ffb,fillmode = ffbFillMode));
#else
    FFBFILLMODE(pDraw, pGC, FALSE, maxPixels,
		/* block OS starts out doing background */
		,
		/* non-block */,
		FFBMODE(ffb,fillmode = ffbFillMode));
#endif

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_SPANS_SETUP(pbox, nInit, pptInit, pwidthInit, xDraw, yDraw);

#ifdef DOOPAQUESTIPPLE
	if (fillmode == BLOCKFILL)
	{
	    /* solid fill background once, to avoid loading color
	       registers again and again and again per span... */
	    CYCLE_REGS(ffb);
	    FFBLOADCOLORREGS(ffb, gcPriv->and, pDraw->depth);
	    FFBDATA(ffb, ~0);
	    {
		long tn = n;
		DDXPointPtr tppt = ppt;
		int *tpwidth = pwidth;

		do {
		    register long x, y, w;
		    int ymul;
		    
		    CLIP_SPANS(tppt,dstwidth,tpwidth,ul,lr,signbits, /* in   */
			       clipx1,clipx2,clipy1,clipy2,	     /* in   */
			       x,y,ymul,w,			     /* out  */
			       tpwidth++; tppt++; if (--tn) continue; break);
		    tpwidth++;
		    tppt++;
		    tn--;
		    pdstBase = CYCLE_FB(pdstBase);
		    p = pdstBase + ymul + x * FFBPIXELBYTES;
		    FFBSOLIDSPAN(ffb, p, w, maxPixels);
		} while (tn > 0);
	    }
	    CYCLE_REGS(ffb);
	    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
	}
#endif
	/*
	 * stipple code.
	 */
	do { /*Each span*/
	    register long x, y, w;
	    int ymul;
	    
	    CLIP_SPANS(ppt,dstwidth,pwidth,ul,lr,signbits,   /* in	     */
		       clipx1,clipx2,clipy1,clipy2,	     /* in	     */
		       x,y,ymul,w,			     /* out	     */
		       SPAN_COMPLETELY_CLIPPED);
	    pwidth++;
	    ppt++;
	    n--;

	    CYCLE_REGS(ffb);

	    /* Compute left edge. */

	    /* Compute offsets into stipple. */
	    ix = x - xorg;
	    iy = (y - yorg) % stippleHeight;
	    srcbits = psrcBase[iy * (LONG_BIT/FFBBUSBITS)];

	    pdstBase = CYCLE_FB(pdstBase);
	    p = pdstBase + ymul + x * FFBPIXELBYTES;

	    /* Offset p back so FFB-aligned. */
	    align = (long)p & FFBSTIPPLEALIGNMASK;
	    FFBBYTESTOPIXELS(align);
	    ix -= align;

	    /* Do alignment of srcbits */
	    ComputeRotateAmounts(ix, ixBUSBITS, ix);
	    srcbits = (srcbits << ixBUSBITS) | (srcbits >> ix);

	    FFBDATA(ffb, srcbits);
	    FFBSOLIDSPAN(ffb, p, w, maxPixels);
	} while (n > 0);
    } /*end for pbox*/
}
#endif /*FFBTSSPANWORD*/


/* 
 * Transparent stipple or opaque stipple a list of rectangles, where
 * the stipple width == FFBBUSBITS.  (The stipple has been replicated
 * if its width was a power of 2 smaller than FFBBUSBITS bits.)  AND, the
 * height of the stipple is a power of 2.
 */

/* Physical pixmap width is LONG_BIT bits. */
#define StartStippleLoop2(psrcBase, iy, heightMask, srcbits, ix, ixBUSBITS) \
{									    \
    srcbits = *(CommandWord *)						    \
	    (psrcBase + (iy & heightMask) * (LONG_BIT/FFBBUSBITS));         \
    /* Do alignment of srcbits */					    \
    srcbits = (srcbits << ixBUSBITS) | (srcbits >> ix);			    \
} /* StartStippleLoop2 */

#define EndStippleLoop2(pdst, dstwidth, iy, height) \
{						    \
    pdst += dstwidth;				    \
    iy += FFBBUSBYTES;				    \
    height--;					    \
} /* EndStippleLoop2 */

#ifdef FFBTSAREAWORD2
void FFBTSAREAWORD2 (pDraw, pGC, nrectFill, prectBase)
    DrawablePtr pDraw;
    GCPtr       pGC;
    int		nrectFill;      /* number of rectangles to fill		    */
    xRectangle  *prectBase;     /* Pointer to first rectangle to fill       */
{
    RegionPtr   prgnClip;
    long	dstwidth;       /* width of drawable			    */
    Pixel8      *pdstBase;      /* pointer to start pixel of drawable       */
    Pixel8	*pdst;		/* pointer to start pixel of row	    */
    Pixel8	*p;		/* Temp dest pointer			    */
    long	numRects;       /* Number of clipping rectangles	    */
    cfbPrivGC   *gcPriv;
    BoxPtr      pbox;		/* pointer to one region clip box	    */
    long	width;		/* width of current rect		    */
    long	height;		/* height of current rect		    */
    long	align;		/* alignment of pdst			    */
    PixmapPtr   pstipple;
    long	heightMask;     /* Height mask for stipple		    */
    Bits8       *psrcBase;      /* Pointer to base of stipple bits	    */
    long	xorg, yorg;     /* Drawable offset into stipple		    */
    long	ix, iy;		/* Rectangle offset into stipple 	    */
    long	ixBUSBITS;	/* FFBBUSBITS - ix			    */
    CommandWord srcbits;	/* Actual source bits for scanline	    */
    CommandWord ones = ffbStippleAll1;
    CommandWord mask, leftMask, rightMask;
    long	w;
    long	h;
    long	m;
    ffbScreenPrivPtr scrPriv;
    FFBMode	fillmode, stipplemode, lastmode;
    long	maxPixels;
    FFB		ffb;
    long	xDraw, yDraw;
    long        signbits = SignBits;

    /*
     * The trick here bleh bleh bleh... (see above)
     */

    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
    if (nrectFill == 0 | numRects == 0) return;

    pstipple = pGC->stipple;

    psrcBase   = (Bits8 *)(pstipple->devPrivate.ptr);
    heightMask = (pstipple->drawable.height-1) * FFBBUSBYTES;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = pGC->patOrg.x;
    yorg = pGC->patOrg.y;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);

    /* one-time stuff per mode */
#ifdef DOTRANSPARENTSTIPPLE
    FFBFILLMODE(pDraw, pGC, FALSE, maxPixels,
		FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		stipplemode = TRANSPARENTBLOCKSTIPPLE,
		stipplemode = TRANSPARENTSTIPPLE,
		fillmode = ffbFillMode);
#else
    FFBFILLMODE(pDraw, pGC, FALSE, maxPixels,
		stipplemode = TRANSPARENTBLOCKSTIPPLE,
		stipplemode = TRANSPARENTSTIPPLE,
		fillmode = ffbFillMode);
#endif
    lastmode = COPY;    /* Some mode that is never used in this routine */

    pdstBase += yDraw * dstwidth + xDraw * FFBPIXELBYTES;

    /*
     * need to decide between stipple and fill modes per rect, depending
     * on how fat it is...
     */

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_RECTS_SETUP(pbox, nrectFill, prectBase, xDraw, yDraw);

	do { /*while nrect != 0 */
	    register long x, y;
	    int ymul;

	    CLIP_RECT(prect, dstwidth, ul, lr, signbits,
		      clipx1, clipx2, clipy1, clipy2,
		      x, y, ymul, width, height,
		      RECT_COMPLETELY_CLIPPED);
	    prect++;
	    nrect--;

	    /* Compute offsets into stipple. */
	    ix = (x - xorg);
	    iy = (y - yorg) * FFBBUSBYTES;

	    CYCLE_REGS(ffb);

	    pdstBase = CYCLE_FB(pdstBase);
	    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
	    
	    align = (long) pdst & FFBSTIPPLEALIGNMASK;
	    FFBBYTESTOPIXELS(align);
	    ix -= align;
	    /* Compute alignment of srcbits */
	    ComputeRotateAmounts(ix, ixBUSBITS, ix);

	    if ( (width+align) <= FFBSTIPPLEBITS /*!FFB_OS_WIDTH_CHECK(width)*/) {
		width += align;
		pdst -= align * FFBPIXELBYTES;
		leftMask = FFBLEFTSTIPPLEMASK(align, ones);
		rightMask = FFBRIGHTSTIPPLEMASK(width, ones);
#if 0
	        if (width <= FFBSTIPPLEBITS)
#endif
		{
		    mask = leftMask & rightMask;
#ifdef DOOPAQUESTIPPLE
		    {
		        if (lastmode != OPAQUESTIPPLE) {
			    lastmode = OPAQUESTIPPLE;
			    FFBMODE(ffb, lastmode);
		        }
		        FFBPERSISTENTPIXELMASK(ffb, mask); 
		        do { /*for each line of height*/
			    StartStippleLoop2(psrcBase, iy, heightMask,
					      srcbits, ix, ixBUSBITS);
			    FFBWRITE(pdst, srcbits);
			    EndStippleLoop2(pdst, dstwidth, iy, height);
		        } while (height != 0);
		    }
#endif
#ifdef DOTRANSPARENTSTIPPLE
		    {
		        if (lastmode != stipplemode) {
			    lastmode = stipplemode;
			    FFBMODE(ffb, lastmode);
		        }
		        do { /*for each line of height*/
			    StartStippleLoop2(psrcBase, iy, heightMask,
					      srcbits, ix, ixBUSBITS);
			    FFBWRITE(pdst, srcbits & mask);
			    EndStippleLoop2(pdst, dstwidth, iy, height);
		        } while (height != 0);
		    } /*end if opaquestippled mode else */
#endif
	        }
#if 0
		else if (pGC->fillStyle == FillOpaqueStippled)
		{
		    if (lastmode != OPAQUESTIPPLE){
			lastmode = OPAQUESTIPPLE;         
			FFBMODE(ffb, lastmode);
		    }
		    do { /*for each line of height */
			StartStippleLoop2(psrcBase, iy, heightMask,
					  srcbits, ix, ixBUSBITS);
			p = pdst;
			CYCLE_REGS(ffb);
			FFBPIXELMASK(ffb, leftMask);
			FFBWRITE(p, srcbits);
			for (m = width - 2*FFBSTIPPLEBITS; m > 0; m -= FFBSTIPPLEBITS) {
			    p += FFBSTIPPLEBYTESDONE;
			    FFBWRITE(p, srcbits);
			}
			CYCLE_REGS(ffb);
			FFBPIXELMASK(ffb, rightMask);
			FFBWRITE(p + FFBSTIPPLEBYTESDONE, srcbits);
			EndStippleLoop2(pdst, dstwidth, iy, height);
		    } while (height != 0);
		} /*end width <= FFBSTIPPLEBITS */
#endif
	    }
	    else
	    {   /*not narrow enough to use stipple mode. Use fill mode */
		if (lastmode != fillmode) {
		    lastmode = fillmode;
		    FFBMODE(ffb, lastmode);
		}

		/* Offset pdst so aligned for fill operation. */
		align = (long) pdst & FFBBUSBYTESMASK;
		pdst -= align;

#ifdef DOOPAQUESTIPPLE
		if (fillmode == BLOCKFILL)
		{
		    /* First paint solid background */
		    FFBLOADCOLORREGS(ffb, gcPriv->and, pDraw->depth);
		    h = height;
		    p = CYCLE_FB_DOUBLE(pdst);
		    FFBDATA(ffb,~0);
		    if (width <= FFBMAXBSWPIXELS) {
			/* One write per scan line */
			mask = FFBLOADBLOCKDATA(align, width);
			do { /*For each line of height */
			    FFBWRITE(p, mask);
			    p += dstwidth;
			    h--;
			} while (h != 0);
		    } else {
			/* Multi-word mask */
			mask = FFBLOADBLOCKDATA(align, FFBMAXBSWPIXELS);
			do { /*For each line of height */
			    p = pdst;
			    w = width;
			    while (w > FFBMAXBSWPIXELS) {
				FFBWRITE(p, mask);
				p += FFBMAXBSWPIXELS * FFBPIXELBYTES;
				w -= FFBMAXBSWPIXELS;
			    }
			    FFBWRITE(p, FFBLOADBLOCKDATA(align, w));
			    h--;
			} while (h != 0);
		    } /*if skinny else fat rectangle */
		    CYCLE_REGS(ffb);
		    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		}
#endif
		if (width <= maxPixels) {
		    /* One write per scan line */
		    mask = FFBLOADBLOCKDATA(align, width);
		    do { /*For each line of height*/
			StartStippleLoop2(psrcBase, iy, heightMask, 
					  srcbits, ix, ixBUSBITS);
			CYCLE_REGS(ffb);
			FFBDATA(ffb, srcbits);
			FFBWRITE(pdst, mask);
			EndStippleLoop2(pdst, dstwidth, iy, height);
		    } while (height != 0);
		} else {
		    /* Multi-word mask */
		    mask = FFBLOADBLOCKDATA(align, maxPixels);
		    do { /*For each line of height*/
			StartStippleLoop2(psrcBase, iy, heightMask, srcbits,
			    ix, ixBUSBITS);
			p = pdst;
			w = width;
			CYCLE_REGS(ffb);
			FFBDATA(ffb, srcbits);
			while(w > maxPixels){
			    FFBWRITE(p, mask);
			    p += maxPixels * FFBPIXELBYTES;
			    w -= maxPixels;
			}
			FFBWRITE(p, FFBLOADBLOCKDATA(align, w));
			EndStippleLoop2(pdst, dstwidth, iy, height);
		    } while (height != 0);
		} /*if skinny else fat rectangle */
	    } /*if super skinny else fat rectangle */
	} while (nrect > 0);
    } /*for pbox*/
    CYCLE_REGS(ffb);
    FFBPIXELMASK(ffb, FFBBUSALL1);
}
#endif /*FFBTSAREAWORD2*/


/*
 * HISTORY
 */

