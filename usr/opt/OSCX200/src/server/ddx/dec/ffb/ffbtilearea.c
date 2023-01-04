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
static char *rcsid = "@(#)$RCSfile: ffbtilearea.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:18:31 $";
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


/* 
 * Tile a list of spans with an arbitrary width tile
 */
#ifdef FFBTILEFILLSPANS
void FFBTILEFILLSPANS (pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr	    pDraw;
    GC		    *pGC;
    int		    nInit;	    /* number of spans to fill		     */
    DDXPointPtr     pptInit;	    /* pointer to start points		     */
    int		    *pwidthInit;    /* pointer to widths		     */
    int		    fSorted;
{
    int    	    dstAlign;	    /* Last few bits of destination ptr	     */
    int    	    srcAlign;	    /* last few bits of source ptr	     */
    int    	    shift;	    /* Mostly dstAlign-srcAlign		     */
    int    	    width;	    /* width of current span		     */
    int    	    widthDst;	    /* width of drawable		     */
    Pixel8 	    *pdstBase;	    /* pointer to start pixel of drawable    */
    Pixel8 	    *pdst;	    /* pointer to start pixel of row	     */
    Pixel8 	    *pd;	    /* pointer into middle of row	     */
    PixmapPtr       ptile;
    int		    tileWidth;  /* Width in pixels of tile		     */
    int		    tileHeight; /* Height of tile			     */
    int		    srcwidth;   /* Width of tile in bytes		     */

    Pixel8	    *psrcBase;  /* Pointer to tile bits		     */
    register Pixel8  *psrc;	    /* pointer to current byte in tile	     */
    long	    iy;	    /* Row offset into tile		     */
    int		    ix;	    /* Column offset into tile		     */
    int		    xorg, yorg; /* Drawable offset into tile	     */
    Pixel8	    *psrcLine;  /* pointer to line of tile to use	     */
    int		    w, srcw;
    cfbPrivGC	    *gcPriv;
    CommandWord	    ones = ffbCopyAll1;
    CommandWord	    mask, leftMask, rightMask;
    CommandWord     rotdepthSrc;
    ffbScreenPrivPtr    scrPriv;
    FFB	 	    ffb;
    int		    ymul, iymul;
    RegionPtr       prgnClip;
    BoxPtr	    pbox;
    long	    numRects;
    long	    xDraw,yDraw;
    long	    signbits = SignBits;

    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
   
    if (nInit == 0 | numRects == 0) return;

    xDraw = pDraw->x;
    yDraw = pDraw->y;
    DrawableBaseAndWidth(pDraw, pdstBase, widthDst);
    pdstBase += yDraw * widthDst + xDraw * FFBPIXELBYTES;

    ptile = pGC->tile.pixmap;
    psrcBase   = (Pixel8 *)(ptile->devPrivate.ptr);
    tileHeight = ptile->drawable.height;
    tileWidth  = ptile->drawable.width;
    srcwidth   = ptile->devKind;

    /* xorg and yorg (mod tile width and height) are the location in the
       tile for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       xorg and yorg to be below any possible x and y that we have to paint.
     */

    xorg = (pGC->patOrg.x % tileWidth) - tileWidth;
    yorg = (pGC->patOrg.y % tileHeight) - tileHeight;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);

    if (SCREENMEMORY(ptile)) {
	/* Tile is in offscreen ffb memory, so use screen/screen copy. */
	pdstBase = CYCLE_FB(pdstBase);
	FFB_SRC_ROTATEDEPTH(ptile, rotdepthSrc);
	FFBMODE(ffb, COPY | rotdepthSrc);
	for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--) {
	    REGION_SPANS_SETUP(pbox, nInit, pptInit, pwidthInit, xDraw, yDraw);
	    do { /* while spans */
	  	register long x, y;

	        WRITE_MEMORY_BARRIER();
                CLIP_SPANS(ppt,widthDst,pwidth,ul,lr,signbits,  /* in */
                       clipx1,clipx2,clipy1,clipy2,		/* in */
                       x,y,ymul,width,				/* out */
                       SPAN_COMPLETELY_CLIPPED);

	        pwidth++;
	        ppt++;
		n--;

	        /* Compute offsets into src. */
	        iy = (y - yorg) % tileHeight;
	        ix = (x - xorg) % tileWidth;

	        /* Compute left end of span. */
	        pdst = pdstBase + ymul + x * FFBPIXELBYTES;
    
	        psrcLine = psrcBase + iy * srcwidth;
	        psrc = psrcLine + ix * FFBSRCPIXELBYTES;
	        srcw = tileWidth - ix;

	        do { /* while pixels left to paint in span */
		    w = width;
		    if (w > srcw) { /* Ran out of tile bits */
		        w = srcw;
		    }
		    width -= w;

		    pd = pdst;
		    pdst += w * FFBPIXELBYTES;
    
		    /* Tiling is like a one-line bitblt */
		    CONJUGATE_FORWARD_ARGUMENTS(psrc,pd,srcAlign,dstAlign,shift,
					        w,leftMask,rightMask,0,0,
					        FFBCOPYALL1_SCRSCR,
					        FFBMASKEDCOPYPIXELSMASK_SCRSCR);

		    CYCLE_REGS(ffb);
		    FFBSHIFT(ffb, shift);
		    if (w <= FFBCOPYPIXELS_SCRSCR) {
		        /* The mask fits into a single word */
		        FFBWRITE(psrc, rightMask);
		        FFBWRITE(pd, leftMask & rightMask);
		    } else {
		        /* Mask requires multiple words */
		        COPY_MASKED_AND_UNMASKED(ffb,psrc,pd,shift,w,
					         leftMask,rightMask,
					         FFBCOPYBYTESDONE_SCRSCR,
					         FFBSRCCOPYBYTESDONE_SCRSCR,
					         FFBCOPYBYTESDONEUNMASKED,
					         FFBSRCCOPYBYTESDONEUNMASKED);
		    } /*if small copy else big copy */
		    pdst = CYCLE_FB_DOUBLE(pdst);
		    psrcLine = CYCLE_FB_DOUBLE(psrcLine);
		    psrc = psrcLine;
		    srcw = tileWidth;
	        } while (width > 0);
	    } while (n > 0);
	}
    } else {
	/* Tile is in main memory. */
        FFB_SRC_ROTATEDEPTH_MAINMEM(ptile, rotdepthSrc);
        FFBMODE(ffb, COPY | rotdepthSrc);
	for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--) {
            REGION_SPANS_SETUP(pbox, nInit, pptInit, pwidthInit, xDraw, yDraw);
 
	    do { /* while spans */
	        register long x, y;

                WRITE_MEMORY_BARRIER();
                CLIP_SPANS(ppt,widthDst,pwidth,ul,lr,signbits, /* in */
                       clipx1,clipx2,clipy1,clipy2,       /* in */
                       x,y,ymul,width,                        /* out */
                       SPAN_COMPLETELY_CLIPPED);

                pwidth++;
                ppt++;
                n--;
		/* Compute offsets into src. */
	        iy = (y - yorg) % tileHeight;
	        iymul = iy * srcwidth;

	        ix = (x - xorg) % tileWidth;
	        srcw = tileWidth - ix;

	        /* Compute left end of span. */
	        pdst = pdstBase + ymul + x * FFBPIXELBYTES;
    
	        psrcLine = psrcBase + iymul;
    
	        psrc = psrcLine + ix * FFBSRCPIXELBYTES;
    
	        do { /* while pixels left to paint in span */
		    w = width;
		    if (w > srcw) { /* Ran out of tile bits */
		        w = srcw;
		    }
		    width -= w;
		    pd = pdst;
		    pdst += w * FFBPIXELBYTES;
		    pdst = CYCLE_FB(pdst);
 
		    /* Tiling is like a one-line bitblt */
		    CONJUGATE_FORWARD_ARGUMENTS(psrc,pd,srcAlign,dstAlign,shift,
					        w,leftMask,rightMask,0,0,
					        FFBCOPYALL1,
					        FFBMASKEDCOPYPIXELSMASK);
		    CYCLE_REGS(ffb);
		    FFBSHIFT(ffb, shift);

		    if (w <= FFBCOPYPIXELS) {
		    /* The mask fits into a single word */
		        mask = leftMask & rightMask;
		        COPY_ONE_MEMSCR(ffb,psrc,pd,rightMask,mask,0,0);
		    } else {
		        /* Mask requires multiple words */
		        COPY_MULTIPLE_MEMSCR(ffb, psrc, pd, w, 0, 0,
					     leftMask, rightMask);
		    } /* if small copy else big copy */
		    psrc = psrcLine;
		    srcw = tileWidth;
	        } while (width > 0);
	    } while (n > 0);
	}
    } /* end if tile in screen else tile in main memory */
}
#endif /* FFBTILEFILLSPANS */



#ifdef DOTILE
/* 
 * Tile a list of spans, where
 * the tile width == 8.  (The tile has been replicated
 * if its width was a power of 2 smaller than 8.)
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
    register long	ixBUSBITS;  /* FFBBUSBITS - ix			    */
    cfbPrivGC		*gcPriv;
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    long		maxPixels;
    BoxPtr		pbox;
    RegionPtr		prgnClip;
    long		numRects;   /* Number of clipping rectangles	     */
    long		xDraw, yDraw;
    long		signbits = SignBits;

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

    pstipple = pGC->tile.pixmap;

    psrcBase      = (CommandWord *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg =  pGC->patOrg.x;
    yorg =  (pGC->patOrg.y % stippleHeight) - stippleHeight;

    /* one-time stuff for each variant */
    FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,
		/**/,
		/**/,
		FFBMODE(ffb,ffbFillMode));


    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_SPANS_SETUP(pbox, nInit, pptInit, pwidthInit, xDraw, yDraw);

	/*
	 * blockfill 8-pixel tile
	 */
	if (PowerOfTwo(stippleHeight))
	{
	    long stippleMask = stippleHeight-1;
	    long stippleWidth = pstipple->drawable.width;
	    long stride = SRCTILESTRIDE(pDraw,stippleWidth);

	    do {
		register long x, y, width;
		int ymul;

		CLIP_SPANS(ppt,dstwidth,pwidth,ul,lr,signbits, /* in	     */
			   clipx1,clipx2,clipy1,clipy2,	       /* in	     */
			   x,y,ymul,width,		       /* out	     */
			   SPAN_COMPLETELY_CLIPPED);
		pwidth++;
		ppt++;
		n--;

		ix = (x - xorg) & 7;
		iy = (y - yorg) & stippleMask;

		pdstBase = CYCLE_FB(pdstBase);
		p = pdstBase + ymul + x * FFBPIXELBYTES;
		FFBTILESPAN(pDraw,ffb,0,ix,iy,FFBMAXBSWPIXELS,
			    psrcBase,stippleWidth,stippleHeight,stride,
			    p,dstwidth,width);
	    } while (n > 0);
	}
	else
	{
	    long stippleWidth = pstipple->drawable.width;
	    long stride = SRCTILESTRIDE(pDraw,stippleWidth);
	    do {
		register long x, y, width;
		int ymul;

		CLIP_SPANS(ppt,dstwidth,pwidth,ul,lr,signbits, /* in	     */
			   clipx1,clipx2,clipy1,clipy2,	       /* in	     */
			   x,y,ymul,width,		       /* out	     */
			   SPAN_COMPLETELY_CLIPPED);
		pwidth++;
		ppt++;
		n--;

		ix = (x - xorg) & 7;
		iy = (y - yorg) % stippleHeight;

		pdstBase = CYCLE_FB(pdstBase);
		p = pdstBase + ymul + x * FFBPIXELBYTES;
		FFBTILESPAN(pDraw,ffb,0,ix,iy,FFBMAXBSWPIXELS,
			    psrcBase,stippleWidth,stippleHeight,stride,
			    p,dstwidth,width);
	    } while (n > 0);
	}
    }
}
#endif /*FFBTSSPANWORD*/



/*
 * Tile a list of rectangles, where
 * the tile width == FFBBUSBITS.  (The tile has been replicated
 * if its width was a power of 2 smaller than 8.)
 */

#ifdef FFBTSAREAWORD

void FFBTSAREAWORD (pDraw, nrect, prect, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    GCPtr		pGC;
{
    register long	width;      /* width of current rect		     */
    register long	height;     /* height of current rect		     */
    register long	dstwidth;   /* width of drawable		     */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable    */
    register Pixel8	*pdst;      /* pointer to start pixel of row	     */

    PixmapPtr		ptile;
    long		tileHeight; /* Height in bits of data in tile	     */
    long		tileWidth;  /* Width of data in tile		     */
    long		tileStride; /* CommandWord adjusted stride	     */
    CommandWord		*psrcBase;  /* Pointer to base of tile bits	     */
    long		xorg, yorg; /* Drawable offset into tile	     */
    long		ix, iy;     /* Rectangle offset into tile	     */
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    long		wid;

    /*
      We use this code for tiles that are (or are replicated to) 8 pixels
      wide.  In this case we load the tile pixels into the color registers, then
      act like we're filling a solid area.  We are guaranteed that really small
      tiles, like the ones used here, are never allocated to ffb osm, but always
      reside in main memory.

      In 32-bit mode, it is possible that we get called with: FFBPIXELBITS == 32
      and FFBSRCPIXELBITS == 8.  That is, packed8 src to unpacked8 dst.  So, we
      shouldn't march along the src w/ such big steps...

      Note that the drawable wid (if applicable) has been set through the
      planemask already, so we just blat 0xf into the the wid field 
      (if necessary).
      */
    if (nrect == 0) return;

    ptile      = pGC->tile.pixmap;
    psrcBase   = (CommandWord *)(ptile->devPrivate.ptr);
    tileHeight = ptile->drawable.height;
    tileWidth  = ptile->drawable.width;
    tileStride = SRCTILESTRIDE(pDraw,tileWidth);

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = (pGC->patOrg.x);
    yorg = (pGC->patOrg.y % tileHeight) - tileHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    wid = FFBBUFDESCRIPTOR(pDraw)->wid;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    FFBMODE(ffb, BLOCKFILL);
    FFBDATA(ffb, ~0);

    pdstBase += pDraw->y * dstwidth + pDraw->x * FFBPIXELBYTES;
    
    if (PowerOfTwo(tileHeight))
    {
	long tileMask = tileHeight-1;

	do { /*Each rectangle*/
	    long ss = ((int *)prect)[0];
	    long x, y = yShort(ss);
	    int ymul = y * dstwidth;

	    x = xShort(ss);
	    ss = ((int *)prect)[1];
	    width = wShort(ss);
	    height = hShort(ss);

	    /* Compute offsets into tile. */
	    ix = (x - xorg);
	    iy = (y - yorg) & tileMask;

	    prect++;
	    nrect--;

	    /* Compute upper left corner. */
	    pdstBase = CYCLE_FB(pdstBase);
	    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
	
	    FFBTILERECT(pDraw, ffb, wid, ix, iy,
			FFBMAXBSWPIXELS, psrcBase,
			tileWidth, tileHeight, tileStride,
			pdst, dstwidth, width, height);
	} while (nrect > 0);
    }
    else
    {
	do { /*Each rectangle*/
	    long ss = ((int *)prect)[0];
	    long x, y = yShort(ss);
	    int ymul = y * dstwidth;

	    x = xShort(ss);
	    ss = ((int *)prect)[1];
	    width = wShort(ss);
	    height = hShort(ss);

	    /* Compute offsets into tile. */
	    ix = (x - xorg);
	    iy = (y - yorg) % tileHeight;

	    prect++;
	    nrect--;

	    /* Compute upper left corner. */
	    pdstBase = CYCLE_FB(pdstBase);
	    pdst = pdstBase + ymul + x * FFBPIXELBYTES;
	
	    FFBTILERECT(pDraw, ffb, wid, ix, iy,
			FFBMAXBSWPIXELS, psrcBase,
			tileWidth, tileHeight, tileStride,
			pdst, dstwidth, width, height);
	} while (nrect > 0);
    } /*PowerOfTwo*/
}
#endif /*FFBTSAREAWORD*/



#ifdef FFBTILEAREAWORD
void FFBTILEAREAWORD (pDraw, pGC, nrectFill, prectBase)
    DrawablePtr		pDraw;
    int			nrectFill;
    xRectangle		*prectBase;
    GCPtr		pGC;
{
    register long	width;      /* width of current rect		     */
    register long	height;     /* height of current rect		     */
    register long	dstwidth;   /* width of drawable		     */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable    */
    register Pixel8	*pdst;      /* pointer to start pixel of row	     */

    PixmapPtr		ptile;
    long		tileHeight; /* Height in bits of data in tile	     */
    long		tileWidth;  /* Width of data in tile		     */
    long		tileStride; /* CommandWord adjusted stride	     */
    CommandWord		*psrcBase;  /* Pointer to base of tile bits	     */
    long		xorg, yorg; /* Drawable offset into tile	     */
    long		ix, iy;     /* Rectangle offset into tile	     */
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    
    RegionPtr prgnClip;
    long xDraw, yDraw;
    long numRects;		    /* Number of clipping rectangles	     */
    BoxPtr pbox;		    /* pointer to one region clip box	     */
    long signbits = SignBits;
    cfbPrivGC *gcPriv;
    long wid;

    /*
      We use this code for tiles that are (or are replicated to) 8 pixels
      wide.  In this case we load the tile pixels into the color registers, then
      act like we're filling a solid area.  We are guaranteed that really small
      tiles, like the ones used here, are never allocated to ffb osm, but always
      reside in main memory.

      In 32-bit mode, it is possible that we get called with: FFBPIXELBITS == 32
      and FFBSRCPIXELBITS == 8.  That is, packed8 src to unpacked8 dst.  So, we
      shouldn't march along the src w/ such big steps...

      Note that the drawable wid (if applicable) has been set through the
      planemask already, so we just blat 0xf into the the wid field 
      (if necessary).
      */

    gcPriv = CFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;
    numRects = REGION_NUM_RECTS(prgnClip);

    if (numRects == 0 | nrectFill == 0) return;

    ptile      = pGC->tile.pixmap;
    psrcBase   = (CommandWord *)(ptile->devPrivate.ptr);
    tileHeight = ptile->drawable.height;
    tileWidth  = ptile->drawable.width;
    tileStride = SRCTILESTRIDE(pDraw,tileWidth);
    wid = FFBBUFDESCRIPTOR(pDraw)->wid;    

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = (pGC->patOrg.x);
    yorg = (pGC->patOrg.y % tileHeight) - tileHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    FFBMODE(ffb, BLOCKFILL);
    FFBDATA(ffb, ~0);
    
    pdstBase += yDraw * dstwidth + xDraw * FFBPIXELBYTES;

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--)
    {
	REGION_RECTS_SETUP(pbox, nrectFill, prectBase, xDraw, yDraw);

	if (PowerOfTwo(tileHeight))
	{
	    long heightMask = tileHeight-1;

	    do { /*Each rectangle */
		register long x, y;
		int ymul;

		CLIP_RECT(prect, dstwidth, ul, lr, signbits,
			  clipx1, clipx2, clipy1, clipy2,
			  x, y, ymul, width, height,
			  RECT_COMPLETELY_CLIPPED);
		prect++;
		nrect--;

		/* Compute offsets into tile. */
		ix = (x - xorg);
		iy = (y - yorg) & heightMask;

		/* Compute upper left corner. */
		pdstBase = CYCLE_FB(pdstBase);
		pdst = pdstBase + ymul + x * FFBPIXELBYTES;

		FFBTILERECT(pDraw, ffb, wid, ix, iy,
			    FFBMAXBSWPIXELS, psrcBase,
			    tileWidth, tileHeight, tileStride,
			    pdst, dstwidth, width, height);
	    } while (nrect > 0);
	}
	else
	{
	    do { /*Each rectangle */
		register long x, y;
		int ymul;

		CLIP_RECT(prect, dstwidth, ul, lr, signbits,
			  clipx1, clipx2, clipy1, clipy2,
			  x, y, ymul, width, height,
			  RECT_COMPLETELY_CLIPPED);
		prect++;
		nrect--;

		/* Compute offsets into tile. */
		ix = (x - xorg);
		iy = (y - yorg) % tileHeight;

		/* Compute upper left corner. */
		pdstBase = CYCLE_FB(pdstBase);
		pdst = pdstBase + ymul + x * FFBPIXELBYTES;

		FFBTILERECT(pDraw, ffb, wid, ix, iy,
			    FFBMAXBSWPIXELS, psrcBase,
			    tileWidth, tileHeight, tileStride,
			    pdst, dstwidth, width, height);
	    } while (nrect > 0);
	}
    }
}
#endif /*FFBTILEAREAWORD*/
#endif /*DOTILE*/

/* 
 * Tile a list of boxes with an arbitrary width tile
 */
#ifdef FFBTILEFILLAREA
void FFBTILEFILLAREA (pDstDraw, nrect, prect, pGC)
    DrawablePtr		pDstDraw;
    int			nrect;
    xRectangle		*prect;
    GCPtr		pGC;
{
    /* pairs allocated to accomodate even/odd scanline processing */
    int        		dstAlign[2];/* Last few bits of destination ptr     */
    int        		srcAlign[2];/* last few bits of source ptr          */
    int        		shift[2];   /* Mostly dstAlign-srcAlign             */
    int        		width[2];   /* width of current rect                */
    Pixel8     		*psrcLine[2];/* pointers into tile		    */
    Pixel8     		*pdstLine[2];/* pointers into destination 	    */
    CommandWord         mask[2], leftMask[2], rightMask[2];

    register int        h;     	    /* height of current rect               */
    register int        height;     /* height of current rect               */
    register int	widthDst;   /* width of drawable		    */
    register int	widthSrc;   /* width of tile			    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */

    PixmapPtr		ptile;
    int			tileWidth;  /* Width in pixels of tile		    */
    int			tileHeight; /* Height of tile			    */
    int			srcwidth;   /* Width of tile in bytes		    */

    Pixel8		*psrcBase;  /* Pointer to tile bits		    */
    register Pixel8     *psrc;      /* pointer to current byte in tile      */
    int			iy;	    /* Row offset into tile		    */
    int			ix;	    /* Column offset into tile		    */
    int			xorg, yorg; /* Drawable offset into tile	    */

    int                 w, srcw;
    int			wSav, wDav;
    int                 wS, wD;                 /* for next even/odd line   */

    CommandWord		ones = ffbCopyAll1;
    CommandWord		rotdepthSrc;
    int			m;
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;

    if (nrect == 0) return;

    ptile = pGC->tile.pixmap;
    psrcBase   = (Pixel8 *)(ptile->devPrivate.ptr);
    tileHeight = ptile->drawable.height;
    tileWidth  = ptile->drawable.width;
    widthSrc = srcwidth   = ptile->devKind;

    /* xorg and yorg bleh bleh bleh (see above) */

    xorg = (pGC->patOrg.x % tileWidth) - tileWidth;
    yorg = (pGC->patOrg.y % tileHeight) - tileHeight;

    DrawableBaseAndWidth(pDstDraw, pdstBase, widthDst);
    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDstDraw->pScreen, pDstDraw, scrPriv, ffb, pGC);

    pdstBase += pDstDraw->y * widthDst + pDstDraw->x * FFBPIXELBYTES;

    /* we advance 2 sets of pointers, so double the strides */
    wS = widthSrc << 1;
    wD = widthDst << 1;
    wSav = widthSrc;
    wDav = widthDst;

    if (SCREENMEMORY(ptile)) {
        /* Tile is in offscreen ffb memory, so use screen/screen copy. */
	/* Put dst and src in different frame buffer aliases */
        pdstBase = CYCLE_FB(pdstBase);
	FFB_SRC_ROTATEDEPTH(ptile, rotdepthSrc);
        FFBMODE(ffb, COPY | rotdepthSrc);
	do { /* Each rectangle */
            /* Compute offsets into src. */
            iy = (prect->y - yorg) % tileHeight;
            ix = (prect->x - xorg) % tileWidth;

	    h = prect->height;
    
	    pdstLine[0] = pdstBase + prect->y * widthDst;
	    psrcLine[0] = psrcBase + iy * widthSrc;
 
	    if (prect->width <= tileWidth - ix) {
		/* Fast case.  Tile is wide enough for rectangle. */
		int i = 0;
		long offset[2];

		pdstLine[1] = CYCLE_FB_DOUBLE(pdstLine[0] + widthDst);
		psrcLine[1] = CYCLE_FB_DOUBLE(psrcLine[0] + widthSrc);
		offset[0] = offset[1] = prect->x;
		do { /* Until rectangle completely painted */
		    width[0] = width[1] = prect->width;
		    CONJUGATE_FORWARD_ARGUMENTS(psrcLine[0],pdstLine[0],
				  srcAlign[0],dstAlign[0],shift[0],
				  width[0],leftMask[0],rightMask[0],
				  ix, offset[0], FFBCOPYALL1_SCRSCR,
				  FFBMASKEDCOPYPIXELSMASK_SCRSCR);
		    CONJUGATE_FORWARD_ARGUMENTS(psrcLine[1],pdstLine[1],
				  srcAlign[1],dstAlign[1],shift[1],
				  width[1],leftMask[1],rightMask[1],
				  ix, offset[1], FFBCOPYALL1_SCRSCR,
				  FFBMASKEDCOPYPIXELSMASK_SCRSCR);
    
		    Assert(shift[0] == shift[1], "Shifts unaligned");
		    CYCLE_REGS(ffb);
		    FFBSHIFT(ffb, shift[0]);
		    if ((width[0] <= FFBCOPYPIXELS_SCRSCR) &&
			(width[1] <= FFBCOPYPIXELS_SCRSCR)) {
			/* The mask fits into a single word */
			mask[0] = leftMask[0] & rightMask[0];
			mask[1] = leftMask[1] & rightMask[1];
    
			do {  /* For each line until rect or tile height */
			    FFBWRITE(psrcLine[i], rightMask[i]);
			    FFBWRITE(pdstLine[i], mask[i]);
			    psrcLine[i] += wS;
			    pdstLine[i] += wD;
			    h--;
			    iy++;
			    i ^= 1;
			} while (h > 0 && iy != tileHeight);
		    } else {
			/* At least even or odd row is multiple words/row */
			do {  /* For each line of height */
			    if (width[i] <= FFBCOPYPIXELS_SCRSCR) {
				FFBWRITE(psrcLine[i], rightMask[i]);
				FFBWRITE(pdstLine[i],
					rightMask[i] & leftMask[i]);
			    } else {
				COPY_MASKED_AND_UNMASKED(ffb, psrcLine[i], 
				    pdstLine[i], shift[i], width[i],
				    leftMask[i], rightMask[i],
				    FFBCOPYBYTESDONE_SCRSCR,
				    FFBSRCCOPYBYTESDONE_SCRSCR,
				    FFBCOPYBYTESDONEUNMASKED,
				    FFBSRCCOPYBYTESDONEUNMASKED);
			    }
			    psrcLine[i] += wS;
			    pdstLine[i] += wD;
			    iy++;
			    h--;
			    i ^= 1;
			} while (h > 0 && iy != tileHeight);
		    } /* end if narrow else wide */
		    if (h != 0) {
			/* Who knows what the deal is now?  If tile is
			   an odd height ((tileHeight & 1) == 1), then
			   the alignment relationship between source and
			   destination will fundamentally change.  And i may be
			   0 or 1.  Echh.  Restore the world, and recompute all
			   copy parameters. */
			iy = 0;
			if (i) {
			    psrcBase = CYCLE_FB_DOUBLE(psrcBase);
			    pdstLine[0] = CYCLE_FB_DOUBLE(pdstLine[0]);
			    pdstLine[1] = CYCLE_FB_DOUBLE(pdstLine[1]);
			}
			psrcLine[0] = psrcBase;
			psrcLine[1] = psrcBase + widthSrc;
			offset[0] = (long)pdstLine[i] + dstAlign[i];
			offset[1] = (long)pdstLine[i^1] + dstAlign[i^1];
			pdstLine[0] = pdstLine[1] = (Pixel8 *)NULL;
			i = 0;
		    }
		} while (h > 0);
		/* Leave pdstBase and psrcBase in nice aliases */
		if (i) {
		    /* Oops, did an odd number of scanlines last rectangle;
		       put pdstBase and psrcBase into other alias set. */
		    pdstBase = CYCLE_FB_DOUBLE(pdstBase);
		    psrcBase = CYCLE_FB_DOUBLE(psrcBase);
		}
	    } else {
		/* Yuck.  We'll have to recycle tile across each scanline. */
		int i = 0;
		int j = 0;
		pdstLine[0] += prect->x * FFBPIXELBYTES;
		do {  /* For each line of height */
		    int     tw = prect->width;
		    Pixel8  *pds = pdstLine[0];

		    psrc = psrcLine[0] + ix * FFBPIXELBYTES;
		    srcw = tileWidth - ix;

		    do { /* while pixels left to paint in this line */
			Pixel8  *pd;
	
			w = tw;		/* Number of pixels left in scanline. */
			if (w > srcw) { /* Ran out of tile pixels. */
			    w = srcw;
			}
			tw -= w;
			pd = pds;
			pds = CYCLE_FB_DOUBLE(pds + w * FFBPIXELBYTES);
			j ^= 1;
	
			CONJUGATE_FORWARD_ARGUMENTS(psrc, pd, srcAlign[0],
			    dstAlign[0], shift[0], w, leftMask[0], rightMask[0],
			    0, 0, FFBCOPYALL1_SCRSCR,
			    FFBMASKEDCOPYPIXELSMASK_SCRSCR);
			
			CYCLE_REGS(ffb);
			FFBSHIFT(ffb, shift[0]);
			if (w <= FFBCOPYPIXELS_SCRSCR) {
			    /* The mask fits into a single word */
			    FFBWRITE(psrc, rightMask[0]);
			    FFBWRITE(pd, rightMask[0] & leftMask[0]);
			} else {
			    /* Mask requires multiple words */
			    COPY_MASKED_AND_UNMASKED(ffb, psrc, pd, shift[0],
				    w, leftMask[0], rightMask[0],
				    FFBCOPYBYTESDONE_SCRSCR,
				    FFBSRCCOPYBYTESDONE_SCRSCR,
				    FFBCOPYBYTESDONEUNMASKED,
				    FFBSRCCOPYBYTESDONEUNMASKED);

			} /* if small copy else big copy */
			psrc = psrcLine[0];
			srcw = tileWidth;
		    } while (tw > 0);
		    i ^= 1;
		    iy++;
		    if (iy == tileHeight) {
			iy = 0;
			if (i) {
			    psrcBase = CYCLE_FB_DOUBLE(psrcBase);
			    i = 0;
			}
			psrcLine[0] = psrcBase;
		    } else {
			psrcLine[0] = CYCLE_FB_DOUBLE(psrcLine[0] + srcwidth);
		    }
		    pdstLine[0] += widthDst;
		    h--;
		} while (h > 0);
		if (j) {
		    pdstBase = CYCLE_FB_DOUBLE(pdstBase);
		}
		if (i) {
		    psrcBase = CYCLE_FB_DOUBLE(psrcBase);
		}
	    } /* if tile wide enough for rect else recycle through it */
	    prect++;
	    nrect--;
	} while (nrect > 0);
    } else {
	/* the K suffix is just a way to get dbx to pay attention... */
	register int        dstAlignK;   /* Last few bits of destination ptr */
	register int        srcAlignK;   /* last few bits of source ptr      */
	register int        shiftK;      /* Mostly dstAlignK-srcAlignK       */
	register int        widthK,wSav; /* width of current rect            */
	CommandWord         maskK, leftMaskK, rightMaskK;
	Pixel8		    *psrcLineK, *psrcLK, *psrcK;
	Bool                doneFirstSet; /* even/odd scanline processing */
	int                 wS, wD;           /* for next even/odd line   */
	Pixel8		    *pdstLineK, *pdst;
	int		    numS, savIy;

	/* Tile is in main memory. */
        FFB_SRC_ROTATEDEPTH_MAINMEM(ptile, rotdepthSrc);
        FFBMODE(ffb, COPY | rotdepthSrc);

       	do { /* Each rectangle */
            wSav = prect->width;
            h = prect->height;
            /* process even numbered scanlines separately from odd */
            doneFirstSet = 0;

            /*
             * Need 2x widths and .5 h to implement even/odd scanline sets.
             */
            numS = (h >> 1) + (h & 1);
            wS = widthSrc << 1;
            wD = widthDst << 1;

            /* Compute offsets into src. */
            savIy = iy = (prect->y - yorg) % tileHeight;
            ix = (prect->x - xorg) % tileWidth;

	    pdstBase = CYCLE_FB(pdstBase);

         StartOfSet_MemScr:
            widthK = wSav;
	    iy = savIy + doneFirstSet;

	    pdstLineK = pdstBase + (prect->y * widthDst) + 
		                      prect->x * FFBPIXELBYTES;
            psrcLineK = psrcBase + iy * widthSrc + ix * FFBSRCPIXELBYTES;
	    if (doneFirstSet) {
		pdstLineK += widthDst;
		if (iy >= tileHeight) {
		    psrcLineK = psrcBase + ix * FFBSRCPIXELBYTES;
		    iy = 0;
		}
	    }
	    savIy = iy;
    
	    if (widthK <= tileWidth - ix) {
		/* Easy case.  Tile is wide enough for rectangle. */
		CONJUGATE_FORWARD_ARGUMENTS(psrcLineK,pdstLineK,
					    srcAlignK,dstAlignK,
					    shiftK,widthK,
					    leftMaskK,rightMaskK,0,0,
					    FFBCOPYALL1,FFBMASKEDCOPYPIXELSMASK);
		CYCLE_REGS(ffb);
		FFBSHIFT(ffb, shiftK);

		psrcLK = psrcLineK - iy * widthSrc; /* in case we wrap tile */

		if (widthK <= FFBCOPYPIXELS) {
		    /* The mask fits into a single word */
		    maskK = leftMaskK & rightMaskK;
		    do {  /* For each line of height */
			CYCLE_REGS(ffb);
			COPY_ONE_MEMSCR(ffb,psrcLineK,pdstLineK,
					rightMaskK,maskK,wS,wD);
			iy += 2;
			if (iy >= tileHeight) {
			    iy -= tileHeight;
			    psrcLineK = psrcLK;
			    if (iy) {
				psrcLineK += widthSrc;
			    }
			}
			numS--;
		    } while (numS > 0);
		} else {
		    /* Mask requires multiple words */
		    do {  /* For each line of height */
			/* XXX: this macro depends in turn on ffbbuffillall();
			   the way this clause was originally written,
			   ffbbuffill() was used instead --- there may be a
			   performance impact */
			CYCLE_REGS(ffb);
                        COPY_MULTIPLE_MEMSCR(ffb, psrcLineK, pdstLineK, widthK,
					    wS, wD, leftMaskK, rightMaskK);
			iy += 2;
			if (iy >= tileHeight) {
			    iy -= tileHeight;
			    psrcLineK = psrcLK;
			    if (iy) {
				psrcLineK += widthSrc;
			    }
			}
			numS--;
		    } while (numS > 0);
		}
    
	    } else {
		/* Yuck.  We'll have to recycle tile across each scanline. */
		do {  /* For each line of height */
		    int     tw = widthK;
		    Pixel8 *pds = pdstLineK;
		    Pixel8 *pS;
	
		    pS = psrc = psrcLineK;
		    pS -= iy * widthSrc;
		    srcw = tileWidth - ix;
	
		    do { /* while pixels left to paint in this line */
	
			w = tw;		/* Number of pixels left in scanline. */
			if (w > srcw) { /* Ran out of tile pixels. */
			    w = srcw;
			}
			tw -= w;
			pdst = pds;
                        pds += w * FFBPIXELBYTES;
			pds = CYCLE_FB(pds);

			CONJUGATE_FORWARD_ARGUMENTS(psrc,pdst,
						    srcAlignK,dstAlignK,shiftK,
						    w,leftMaskK,rightMaskK,
						    0,0,FFBCOPYALL1,
						    FFBMASKEDCOPYPIXELSMASK);
			
			CYCLE_REGS(ffb);
			FFBSHIFT(ffb, shiftK);

			if (w <= FFBCOPYPIXELS) {
			    /* The mask fits into a single word */
			    maskK = leftMaskK & rightMaskK;
			    COPY_ONE_MEMSCR(ffb,psrc,pdst,rightMaskK,maskK,
					    0,0);
			} else {
			    /* Mask requires multiple words */
			    COPY_MULTIPLE_MEMSCR(ffb, psrc, pdst, w, 0, 0,
						 leftMaskK, rightMaskK)
			} /* if small copy else big copy */
			psrc = psrcLineK - ix * FFBSRCPIXELBYTES;
			srcw = tileWidth;
		    } while (tw > 0);
		    pdstLineK = CYCLE_FB_CURR(pdstLineK, pds);
		    psrcLineK += wS;
		    iy += 2;
		    if (iy >= tileHeight) {
			iy -= tileHeight;
			psrcLineK = pS;
			if (iy) {
			    psrcLineK += widthSrc;
			}
		    }
		    pdstLineK += wD;
		    numS--;
		} while (numS > 0);
		pdstBase = CYCLE_FB_CURR(pdstBase, pdstLineK);
	    } /* if tile wide enough for rect else recycle through it */

            doneFirstSet ^= 1;              /* toggle sets */
            numS = h >> 1;
            if (doneFirstSet && numS) {
                goto StartOfSet_MemScr;
            } else {
		prect++;
		nrect--;
	    }
	} while (nrect > 0);
    } /* end if tile in ffb memory else in main memory */
}
#endif /*FFBTILEFILLAREA*/

/*
 * HISTORY
 */
