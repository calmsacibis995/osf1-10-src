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
#ifdef VMS
#define IDENT "X-2"
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991 BY                  *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel
**
**--
**/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbpntarea.c,v 1.1.5.2 93/03/02 16:28:04 Don_Haney Exp $ */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "sfb.h"
#include "sfbpntarea.h"
#include "cfbsetsp.h"

#ifdef COMPILEEVERYTHING

extern CommandWord sfbStippleAll1;

void sfbPolyFillRectSolid(pDraw, pGC, nrectFill, prectBase)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		nrectFill;		/* number of rectangles to fill */
    xRectangle	*prectBase;		/* Pointer to first rectangle to fill */
{
    xRectangle      *prect;
    register int    nrect;
    RegionPtr       prgnClip;
    int		    dstwidth;
    int		    xDraw, yDraw;
    Pixel8	    *pdstBase;
    register Pixel8 *pdst;		/* pointer to start pixel of row    */
    register Pixel8 *p;			/* pointer to pixels we're writing  */
    register int    numRects;		/* Number of clipping rectangles    */
    BoxPtr	    pextent;
    cfbPrivGC       *gcPriv;
    register BoxPtr pbox;		/* pointer to one region clip box   */
    register int    width;		/* width of current rect	    */
    register int    h;			/* height of current rect	    */
    register int    align;		/* alignment of pdst		    */
    CommandWord	    ones = sfbStippleAll1;
    CommandWord     mask, leftMask, rightMask;
    int		    m;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
    if (nrectFill == 0 || numRects == 0) return;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    xDraw = pDraw->x;
    yDraw = pDraw->y;

    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--) {
	register int clipx1, clipx2, clipy1, clipy2;
	register int x, x2, y, y2;

	clipx1 = pbox->x1 - xDraw;
	clipy1 = pbox->y1 - yDraw;
	clipx2 = pbox->x2 - xDraw;
	clipy2 = pbox->y2 - yDraw;

	prect = prectBase;
	nrect = nrectFill;
	do { /* while nrect != 0 */
	    int ymul;
	    y = prect->y;
	    ymul = y * dstwidth;
	    x = prect->x;
	    width = prect->width;
	    h = prect->height;
	    x2 = x + width;
	    y2 = y + h;
	    if (x < clipx1 | y < clipy1 | x2 > clipx2 | y2 > clipy2
		| width == 0 | h == 0) {
		/* Ick, we have to clip at least partially */
		if (x < clipx1) x = clipx1;
		if (y < clipy1) {
		    y = clipy1;
		    ymul = y * dstwidth;
		}
		if (x2 > clipx2) x2 = clipx2;
		if (y2 > clipy2) y2 = clipy2;
		if (x >= x2 || y >= y2) {
		    goto COMPLETELY_CLIPPED;
		}
		width = x2 - x;
		h = y2 - y;
	    }
	    pdstBase = CYCLE_FB(pdstBase);

	    pdst = pdstBase + yDraw * dstwidth + ymul + (xDraw + x) * SFBPIXELBYTES;
	    align = (int)pdst & SFBALIGNMASK;
	    pdst -= align;
	    SFBBYTESTOPIXELS(align);
	    width += align;
	    leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	    rightMask = SFBRIGHTSTIPPLEMASK(width, ones);

	    if (width <= SFBSTIPPLEBITS) {
		/* Mask fits in one word.  Measurements show that it it fastest
		   to use a single write, and take the TLB misses. */
		mask = leftMask & rightMask;
		if (h & 1) {
		    SFBWRITE(pdst, mask);
		    pdst += dstwidth;
		    h--;
		}
		while (h != 0) {
		    SFBWRITE(pdst, mask);
		    pdst += dstwidth;
		    SFBWRITE(pdst, mask);
		    pdst += dstwidth;
		    h -= 2;
		};
	    } else {
		/* Mask requires multiple words */
		do {
		    p = pdst;
		    SFBWRITE(p, leftMask);
		    for (m = width - 2*SFBSTIPPLEBITS; m > 0;
			m -= SFBSTIPPLEBITS) {
			p += SFBSTIPPLEBYTESDONE;
			SFBWRITE(p, ones);
		    }
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE, rightMask);
		    pdst += dstwidth;
		    pdst = CYCLE_FB(pdst);
		    h--;
		} while (h !=0);
	    } /* if skinny else fat rectangle */

COMPLETELY_CLIPPED:
	    prect++;
	    nrect--;
	} while (nrect != 0);
    } /* for pbox */
}

void sfbSolidFillSpans(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    register Pixel8	    *pdst, *pdstLine;
    int			    y, lasty;
    register int	    w;
					/* next 3 parameters are post-clip */
    int			    n;		/* number of spans to fill */
    register DDXPointPtr    ppt;	/* pointer to list of start points */
    register int	    *pwidth;	/* pointer to list of n widths */
    Pixel8		    *addrlBase;	/* pointer to start of dst */
    int			    nlwidth;	/* width in pixels of dst */
    int			    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr		    pptFree;
    CommandWord		    ones = sfbStippleAll1;
    cfbPrivGC		    *gcPriv;
    sfbScreenPrivPtr	    scrPriv;
    SFB			    sfb;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    n = nInit * miFindMaxBand(gcPriv->pCompositeClip);
    pwidthFree = pwidth = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = ppt = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if (!ppt || !pwidth) {
	if (ppt) DEALLOCATE_LOCAL(ppt);
	if (pwidth) DEALLOCATE_LOCAL(pwidth);
	return;
    }
    n = miClipSpans(gcPriv->pCompositeClip,
		     pptInit, pwidthInit, nInit,
		     ppt, pwidth, fSorted);

    DrawableBaseAndWidth(pDraw, addrlBase, nlwidth);

    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    pdstLine = addrlBase;
    lasty = 0;      /* ||| Not sure how much diff this makes. */
    while (n--) {
	y = ppt->y;
	if (y == lasty + 1) {
	    pdstLine += nlwidth;
	} else if (y != lasty) {
	    pdstLine = addrlBase + y * nlwidth;
	}
	addrlBase = CYCLE_FB(addrlBase);
	pdstLine = CYCLE_FB(pdstLine);
	lasty = y;
	pdst = pdstLine + ppt->x * SFBPIXELBYTES;
	w = *pwidth;
	pwidth++;
	SFBSOLIDSPAN(sfb, pdst, w, ones);
	ppt++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


/* 
 * Opaque stipple pstipple onto the destination drawable.  Unlike the stippled
 * rectangle cases, we don't have to worry about repeating horizontally and
 * vertically. 
 */

void sfbOSPlane(pDraw, pGC, pstipple, xorg, yorg, prgn)
    DrawablePtr pDraw;
    GCPtr	pGC;
    PixmapPtr	pstipple;
    int		xorg, yorg;
    RegionPtr   prgn;
{
    int			nbox;
    BoxPtr		pbox;
    register int	width;      /* width of current rect in bytes	    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register Pixel8	*p;	    /* pointer to pixels we're writing      */

    int			stippleWidth; /* Width in bits of data in stipple   */
    int			stippleHeight;/* Height in bits of data in stipple  */
    int			srcWidth;   /* Stipple width in physical words      */
    CommandWord		*psrcBase;  /* Pointer to stipple bits		    */
    CommandWord		*psrc;      /* Pointer to word in stipple bits      */
    int			ix, iy;     /* Rectangle offset into stipple 	    */
    int			ixBUSBITS;  /* SFBBUSBITS - ix			    */
    int align;			    /* Pixel offsets for left alignment     */
    CommandWord		purebits, srcbits, *ps;
    CommandWord		ones = sfbStippleAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

    nbox = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);

    psrcBase	    = (CommandWord *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind / SFBBUSBYTES;
    stippleWidth    = pstipple->drawable.width;

    xorg += pDraw->x;
    yorg += pDraw->y;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, OPAQUESTIPPLE);

    do { /* Each rectangle */
	pdst = pdstBase + pbox->y1 * dstwidth + pbox->x1 * SFBPIXELBYTES;
	pdstBase = CYCLE_FB(pdstBase);
	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;

	ix = pbox->x1 - xorg;
	iy = pbox->y1 - yorg;

	psrc = psrcBase + (iy * srcWidth) + (ix / SFBBUSBITS);

	/* Align everything.  This may cause ix to wrap around to a negative
	   number, which nominally means we should back up psrc as well.  But
	   we won't: we don't actually need the source data, because it
	   will be masked out. */
	align = ((int) pdst) & SFBALIGNMASK;
	pdst -= align;

	SFBBYTESTOPIXELS(align);
	ix = (ix - align) & SFBBUSBITSMASK;
	ixBUSBITS = SFBBUSBITS - ix;
	width += align;
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(width, ones);
	
	if (width <= SFBSTIPPLEBITS) {
	    /* One word of data bits, one write to sfb. */
	    mask = leftMask & rightMask;
	    do { /* For each line of height */
		ps = psrc;
		/* Fetch source, unless we'd just throw it away because
		   ix wrapped to negative */
		if (align < ixBUSBITS) {
		    purebits = *ps++;
		}
		srcbits = purebits >> ix;
		if (width > ixBUSBITS) {
		    /* We need a few more bits */
		    purebits = *ps;
		    srcbits |= (purebits << ixBUSBITS);
		}
		SFBPIXELMASK(sfb, mask);
		SFBWRITEONEWORD(sfb, pdst, srcbits);
		psrc += srcWidth;
		pdst += dstwidth;
		pdst = CYCLE_FB(pdst);
		height--;
	    } while (height != 0);

#if SFBBUSBITS <= SFBSTIPPLEBITS
	} else {
	    /* Multiple words of data bits, each requires one write to sfb */
	    if (ix == 0) { /* 1/32 chance, but lets us optimize ix != 0 case */
		do { /* For each line of height */
		    ps = psrc;
		    p = pdst;
    
		    /* Ragged left edge */
		    purebits = *ps++;
		    SFBPIXELMASK(sfb, leftMask);
		    SFBWRITE(p, purebits);
		    p = CYCLE_FB(p);
		    /* We need a few more bits */
		    purebits = *ps++;
		    for (m = width - 2*SFBSTIPPLEBITS; m > 0;
			 m -= SFBSTIPPLEBITS) {
			p += SFBSTIPPLEBYTESDONE;
			SFBWRITE(p, purebits);
			purebits = *ps++;
		    }
		    /* Ragged right bits */
		    p = CYCLE_FB(p);
		    SFBPIXELMASK(sfb, rightMask);
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE, purebits);
		    psrc += srcWidth;
		    pdst += dstwidth;
		    pdst = CYCLE_FB(pdst);
		    height--;
		} while (height != 0);

	    } else { /* ix != 0, 31/32 chance */
		do { /* For each line of height */
		    ps = psrc;
		    p = pdst;
    
		    /* Fetch source, unless we'd just throw it away because
		       ix wrapped to negative */
		    if (align < ixBUSBITS) {
			purebits = *ps++;
		    }
		    srcbits = purebits >> ix;
		    /* We need a few more bits */
		    purebits = *ps++;
		    srcbits |= (purebits << ixBUSBITS);
		    SFBPIXELMASK(sfb, leftMask);
		    SFBWRITE(p, srcbits);
		    p = CYCLE_FB(p);
    
		    for (m = width - 2*SFBSTIPPLEBITS;
			 m > 0; m -= SFBSTIPPLEBITS) {
			p += SFBSTIPPLEBYTESDONE;
			srcbits = purebits >> ix;
			/* Get a few more bits, merge with srcbits */
			purebits = *ps++;
			srcbits |= (purebits << ixBUSBITS);
			SFBWRITE(p, srcbits);
		    }
		    /* Ragged right bits */
		    srcbits = purebits >> ix;
		    m += SFBSTIPPLEBITS;
		    if (m > ixBUSBITS) {
			/* Get a few more bits, merge with srcbits */
			purebits = *ps;
			srcbits |= (purebits << ixBUSBITS);
		    }
		    p = CYCLE_FB(p);
		    SFBPIXELMASK(sfb, rightMask);
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE, srcbits);
		    psrc += srcWidth;
		    pdst += dstwidth;
		    pdst = CYCLE_FB(pdst);
		    height--;
		} while (height != 0);
	    }

#else /* SFBBUSBITS > SFBSTIPPLEBITS, so must break up each word read. */

	} else if (width <= SFBBUSBITS) {
	    /* One word of data bits, multiple writes. */
	    do { /* For each line of height */
		ps = psrc;
		/* Fetch source, unless we'd just throw it away because
		   ix wrapped to negative */
		if (align < ixBUSBITS) {
		    purebits = *ps++;
		}
		srcbits = purebits >> ix;
		if (width > ixBUSBITS) {
		    /* We need a few more bits */
		    purebits = *ps;
		    srcbits |= (purebits << ixBUSBITS);
		}
		p = pdst;
		SFBPIXELMASK(sfb, leftMask);
		SFBWRITE(p, srcbits);
		p = CYCLE_FB(p);
# if SFBBUSBITS > 2 * SFBSTIPPLEBITS
		for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		    p += SFBSTIPPLEBYTESDONE;
		    srcbits >>= SFBSTIPPLEBITS;
		    SFBWRITE(p, srcbits);
		}
# endif
		p = CYCLE_FB(p);
		SFBPIXELMASK(sfb, rightMask);
		SFBWRITE(p+SFBSTIPPLEBYTESDONE, srcbits >> SFBSTIPPLEBITS);
		psrc += srcWidth;
		pdst += dstwidth;
		pdst = CYCLE_FB(pdst);
		height--;
	    } while (height != 0);
	    
	} else {
	    /* Multi-word data, multiple writes per word. */
	    do { /* For each line of height */
		ps = psrc;
 		/* Fetch source, unless we'd just throw it away because
		   ix wrapped to negative */
		if (align < ixBUSBITS) {
		    purebits = *ps++;
		}
		srcbits = purebits >> ix;
		/* We need a few more bits */
		purebits = *ps++;
		if (ix != 0) srcbits |= (purebits << ixBUSBITS);
		/* Write ragged left bits. */
		p = pdst;
		SFBPIXELMASK(sfb, leftMask);
		StippleEntireWord(p, srcbits);
		p = CYCLE_FB(p);
		/* Write middle words. */
		for (m = width - 2*SFBBUSBITS; m > 0; m -= SFBBUSBITS) {
		    p += SFBBUSBITS*SFBPIXELBYTES;
		    srcbits = purebits >> ix;
		    /* Get a few more bits, merge with srcbits if needed */
		    purebits = *ps++;
		    if (ix != 0) srcbits |= (purebits << ixBUSBITS);
		    StippleEntireWord(p, srcbits);
		}
		/* Ragged right bits */
		srcbits = purebits >> ix;
		m += SFBBUSBITS;
		if (m > ixBUSBITS) {
		    /* Get a few more bits, merge with srcbits always needed */
		    purebits = *ps;
		    srcbits |= (purebits << ixBUSBITS);
		}
# if SFBBUSBITS > 2 * SFBSTIPPLEBITS
		for (m = m - SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		    SFBWRITE(p + SFBBUSBITS*SFBPIXELBYTES, srcbits);
		    p += SFBSTIPPLEBYTESDONE;
		    srcbits >>= SFBSTIPPLEBITS;
		}
# else
		if (m > SFBSTIPPLEBITS) {
		    SFBWRITE(p + SFBBUSBITS*SFBPIXELBYTES, srcbits);
		    p += SFBSTIPPLEBYTESDONE;
		    srcbits >>= SFBSTIPPLEBITS;
		}
# endif
		p = CYCLE_FB(p);
		SFBPIXELMASK(sfb, rightMask);
		SFBWRITE(p + SFBBUSBITS*SFBPIXELBYTES, srcbits);
		psrc += srcWidth;
		pdst += dstwidth;
		pdst = CYCLE_FB(pdst);
		height--;
	    } while (height > 0);
#endif
	} /* end if narrow or wide rectangle */
	pbox++;
	nbox--;
    } while (nbox > 0);
}

/* 
 * Tile a list of boxes with an arbitrary width tile
 */

void sfbTileFillArea(pDstDraw, nrect, prect, pGC)
    DrawablePtr		pDstDraw;
    int			nrect;
    xRectangle		*prect;
    GCPtr		pGC;
{
    register int	dstAlign;   /* Last few bits of destination ptr     */
    register int	srcAlign;   /* last few bits of source ptr	    */
    register int	shift;	    /* Mostly dstAlign-srcAlign		    */
    register int	width;      /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	widthDst;   /* width of drawable		    */
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

    Pixel8		*psrcLine;  /* pointer to line of tile to use */
    int			w, srcw;

    CommandWord		ones = sfbCopyAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

    if (nrect == 0) return;

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

    DrawableBaseAndWidth(pDstDraw, pdstBase, widthDst);
    CHECKSTATE(pDstDraw->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, COPY);

    if (SCREENMEMORY(ptile)) {
	/* Tile is in offscreen sfb memory, so use screen/screen copy. */
	/* Make pdstBase one cycle off from psrcBase so that
	 * there are no conflicts 
	 */
	/* We need to keep the src and dst apart by one alias.
	 * But we need to cycle psrc and pdst differently over time
	 * so we need to keep a global current alias/cycle
	 */
	CYCLE_FB_GLOBAL_DECL;
	do { /* Each rectangle */
	    /* Compute upper left corner in dst. */
	    pdst = pdstBase + (prect->y + pDstDraw->y) * widthDst
		   + (prect->x + pDstDraw->x) * SFBPIXELBYTES;
    
	    /* Compute offsets into src. */
	    iy = (prect->y - yorg) % tileHeight;
	    psrcLine = psrcBase + iy * srcwidth;
	    ix = (prect->x - xorg) % tileWidth;
    
	    width = prect->width;
	    height = prect->height;
    
	    if (width <= tileWidth - ix) {
		/* Easy case.  Tile is wide enough for rectangle. */
		srcAlign = ix & SFBALIGNMASK;
		dstAlign = (int)pdst & SFBALIGNMASK;
		shift = dstAlign - srcAlign;
		if (shift < 0) {
		    /* Prime pump. */
		    shift += SFBALIGNMENT;
		    dstAlign += SFBALIGNMENT;
		}
		SFBSHIFT(sfb, shift);
		psrcLine -= srcAlign;
		pdst -= dstAlign;
		SFBBYTESTOPIXELS(dstAlign);
		width += dstAlign;
		leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
		rightMask = SFBRIGHTCOPYMASK(width, ones);
    
		if (width <= SFBCOPYBITS) {
		    /* The mask fits into a single word */
		    mask = leftMask & rightMask;
		    /* Extend mask if it specifies a single memory access
		       to avoid a race condition in copy logic. */
		    if (dstAlign < SFBALIGNMENT) rightMask |= SFBRACECOPYMASK;
		    else rightMask |= (SFBRACECOPYMASK << SFBALIGNMENT);
		    psrc = psrcLine + ix * SFBPIXELBYTES;
		    do {  /* For each line of height */
			psrc = CYCLE_FB_GLOBAL(psrc);
			SFBWRITE(psrc, rightMask);
			pdst = CYCLE_FB_GLOBAL(pdst);
			SFBWRITE(pdst, mask);
			psrc += srcwidth;
			iy++;
			if (iy == tileHeight) {
			    iy = 0;
			    psrc = psrcBase + ix*SFBPIXELBYTES - srcAlign;
			}
			pdst += widthDst;
			height--;
		    } while (height > 0);
		} else {
		    /* Mask requires multiple words */
		    psrcLine += ix * SFBPIXELBYTES;
		    do {  /* For each line of height */
			Pixel8  *pd;
    
			psrc = psrcLine;
			pd = pdst;
			psrc = CYCLE_FB_GLOBAL(psrc);
			pd = CYCLE_FB_GLOBAL(pd);
			SFBWRITE(psrc, ones);
			SFBWRITE(pd, leftMask);
			for (m = width - 2*SFBCOPYBITS;
			    m > 0;
			    m -= SFBCOPYBITS) {
			    psrc += SFBCOPYBYTESDONE;
			    pd += SFBCOPYBYTESDONE;
			    SFBWRITE(psrc, ones);
			    SFBWRITE(pd, ones);
			}
			SFBWRITE(psrc+SFBCOPYBYTESDONE, rightMask);
			SFBWRITE(pd+SFBCOPYBYTESDONE, rightMask);
			psrcLine += srcwidth;
			iy++;
			if (iy == tileHeight) {
			    iy = 0;
			    psrcLine =
				psrcBase + ix*SFBPIXELBYTES - srcAlign;
			}
			pdst += widthDst;
			height--;
		    } while (height > 0);
		}
    
	    } else {
		/* Yuck.  We'll have to recycle tile across each scanline. */
		do {  /* For each line of height */
		    int     tw = width;
		    Pixel8 *pds = pdst;
	
		    psrc = psrcLine + ix * SFBPIXELBYTES;
		    srcw = tileWidth - ix;
	
		    do { /* while pixels left to paint in this line */
			Pixel8  *pd;
	
			w = tw;		/* Number of pixels left in scanline. */
			if (w > srcw) { /* Ran out of tile pixels. */
			    w = srcw;
			}
			tw -= w;
			pd = pds;
			pds += w * SFBPIXELBYTES;
	
			/* Tiling is like a one-line bitblt */
			srcAlign = (int)psrc & SFBALIGNMASK;
			dstAlign = (int)pd & SFBALIGNMASK;
			shift = dstAlign - srcAlign;
			if (shift < 0) {
			    /* Prime pump. */
			    shift += SFBALIGNMENT;
			    dstAlign += SFBALIGNMENT;
			}
			SFBSHIFT(sfb, shift);
			psrc -= srcAlign;
			pd -= dstAlign;
			SFBBYTESTOPIXELS(dstAlign);
			w += dstAlign;
			leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
			rightMask = SFBRIGHTCOPYMASK(w, ones);
			psrc = CYCLE_FB_GLOBAL(psrc);
			pd = CYCLE_FB_GLOBAL(pd);
			if (w <= SFBCOPYBITS) {
			    /* The mask fits into a single word */
			    mask = leftMask & rightMask;
			    /* Extend mask if it specifies a single memory
			       access to avoid a race condition in copy logic */
				if (dstAlign < SFBALIGNMENT)
				    rightMask |= SFBRACECOPYMASK;
				else
				    rightMask |=
					    (SFBRACECOPYMASK << SFBALIGNMENT);
			        SFBWRITE(psrc, rightMask);
			        SFBWRITE(pd, mask);
			} else {
			    /* Mask requires multiple words */
			    SFBWRITE(psrc, ones);
			    SFBWRITE(pd, leftMask);
			    for (m = w - 2*SFBCOPYBITS;
				m > 0;
				m -= SFBCOPYBITS) {
				psrc += SFBCOPYBYTESDONE;
				pd += SFBCOPYBYTESDONE;
				SFBWRITE(psrc, ones);
				SFBWRITE(pd, ones);
			    }
			    SFBWRITE(psrc+SFBCOPYBYTESDONE, rightMask);
			    SFBWRITE(pd+SFBCOPYBYTESDONE, rightMask);
			} /* if small copy else big copy */
			psrc = psrcLine;
			srcw = tileWidth;
		    } while (tw > 0);
		    psrcLine += srcwidth;
		    iy++;
		    if (iy == tileHeight) {
			iy = 0;
			psrcLine = psrcBase;
		    }
		    pdst += widthDst;
		    height--;
		} while (height > 0);
	    } /* if tile wide enough for rect else recycle through it */
	    prect++;
	    nrect--;
	} while (nrect > 0);
    } else {
	/* Tile is in main memory. */
	do { /* Each rectangle */
	    /* Compute upper left corner in dst. */
	    pdst = pdstBase + (prect->y + pDstDraw->y) * widthDst 
		   + (prect->x + pDstDraw->x) * SFBPIXELBYTES;
	    pdstBase = CYCLE_FB(pdstBase);

	    /* Compute offsets into src. */
	    iy = (prect->y - yorg) % tileHeight;
	    psrcLine = psrcBase + iy * srcwidth;
	    ix = (prect->x - xorg) % tileWidth;
    
	    width = prect->width;
	    height = prect->height;
    
	    if (width <= tileWidth - ix) {
		/* Easy case.  Tile is wide enough for rectangle. */
		srcAlign = (ix * SFBSLEAZEPIXELBYTES) & SFBSLEAZEALIGNMASK;
		dstAlign = (int)pdst & SFBALIGNMASK;
		shift = dstAlign - srcAlign*SFBSLEAZEMULTIPLIER;
		if (shift < 0) {
		    /* Prime pump. */
		    shift += SFBALIGNMENT;
		    dstAlign += SFBALIGNMENT;
		}
		SFBSHIFT(sfb, shift);
		psrcLine -= srcAlign;
		pdst -= dstAlign;
		SFBBYTESTOPIXELS(dstAlign);
		width += dstAlign;
		leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
		rightMask = SFBRIGHTCOPYMASK(width, ones);
    
		if (width <= SFBCOPYBITS) {
		    /* The mask fits into a single word */
		    mask = leftMask & rightMask;
		    psrc = psrcLine + ix * SFBSLEAZEPIXELBYTES;
		    do {  /* For each line of height */
			/* Read source words and stuff them into sfb buffer */
			SFBBUFFILL(sfb, psrc, rightMask);
			SFBWRITE_CYCLE(pdst, mask);
			psrc += srcwidth;
			iy++;
			if (iy == tileHeight) {
			    iy = 0;
			    psrc = psrcBase + ix*SFBSLEAZEPIXELBYTES - srcAlign;
			}
			pdst += widthDst;
			height--;
		    } while (height > 0);
		} else {
		    /* Mask requires multiple words */
		    psrcLine += ix * SFBSLEAZEPIXELBYTES;
		    do {  /* For each line of height */
			Pixel8  *pd;
    
			psrc = psrcLine;
			pd = pdst;
			SFBBUFFILLALL(sfb, psrc);
			SFBWRITE(pd, leftMask);
			for (m = width - 2*SFBCOPYBITS;
			    m > 0;
			    m -= SFBCOPYBITS) {
			    psrc += SFBSLEAZEBYTESDONE;
			    pd += SFBCOPYBYTESDONE;
			    SFBBUFFILLALL(sfb, psrc);
			    SFBWRITE(pd, ones);
			}
			SFBBUFFILL(sfb, psrc+SFBSLEAZEBYTESDONE, rightMask);
			SFBWRITE(pd+SFBCOPYBYTESDONE, rightMask);
			psrcLine += srcwidth;
			iy++;
			if (iy == tileHeight) {
			    iy = 0;
			    psrcLine =
				psrcBase + ix*SFBSLEAZEPIXELBYTES - srcAlign;
			}
			pdst += widthDst;
			pdst = CYCLE_FB(pdst);
			height--;
		    } while (height > 0);
		}
    
	    } else {
		/* Yuck.  We'll have to recycle tile across each scanline. */
		do {  /* For each line of height */
		    int     tw = width;
		    Pixel8 *pds = pdst;
	
		    psrc = psrcLine + ix * SFBSLEAZEPIXELBYTES;
		    srcw = tileWidth - ix;
	
		    do { /* while pixels left to paint in this line */
			Pixel8  *pd;
	
			w = tw;		/* Number of pixels left in scanline. */
			if (w > srcw) { /* Ran out of tile pixels. */
			    w = srcw;
			}
			tw -= w;
			pd = pds;
			pds += w * SFBPIXELBYTES;
		    	pds = CYCLE_FB(pds);
	
			/* Tiling is like a one-line bitblt */
			srcAlign = (int)psrc & SFBSLEAZEALIGNMASK;
			dstAlign = (int)pd & SFBALIGNMASK;
			shift = dstAlign - srcAlign*SFBSLEAZEMULTIPLIER;
			if (shift < 0) {
			    /* Prime pump. */
			    shift += SFBALIGNMENT;
			    dstAlign += SFBALIGNMENT;
			}
			SFBSHIFT(sfb, shift);
			psrc -= srcAlign;
			pd -= dstAlign;
			SFBBYTESTOPIXELS(dstAlign);
			w += dstAlign;
			leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
			rightMask = SFBRIGHTCOPYMASK(w, ones);
			if (w <= SFBCOPYBITS) {
			    /* The mask fits into a single word */
			    mask = leftMask & rightMask;
			    /* Read source words and stuff into sfb buffer */
			    SFBBUFFILL(sfb, psrc, rightMask);
			    SFBWRITE(pd, mask);
			} else {
			    /* Mask requires multiple words */
			    SFBBUFFILLALL(sfb, psrc);
			    SFBWRITE(pd, leftMask);
			    for (m = w - 2*SFBCOPYBITS;
				m > 0;
				m -= SFBCOPYBITS) {
				psrc += SFBSLEAZEBYTESDONE;
				pd += SFBCOPYBYTESDONE;
				SFBBUFFILLALL(sfb, psrc);
				SFBWRITE(pd, ones);
			    }
			    SFBBUFFILL(sfb, psrc+SFBSLEAZEBYTESDONE, rightMask);
			    SFBWRITE(pd+SFBCOPYBYTESDONE, rightMask);
			} /* if small copy else big copy */
			psrc = psrcLine;
			srcw = tileWidth;
		    } while (tw > 0);
		    psrcLine += srcwidth;
		    iy++;
		    if (iy == tileHeight) {
			iy = 0;
			psrcLine = psrcBase;
		    }
		    pdst += widthDst;
		    pdst = CYCLE_FB(pdst);
		    height--;
		} while (height > 0);
	    } /* if tile wide enough for rect else recycle through it */
	    prect++;
	    nrect--;
	} while (nrect > 0);
    } /* end if tile in sfb memory else in main memory */
}


/* 
 * Tile a list of spans with an arbitrary width tile
 */

void sfbTileFillSpans(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr	    pDraw;
    GC		    *pGC;
    int		    nInit;	/* number of spans to fill		    */
    DDXPointPtr     pptInit;	/* pointer to start points		    */
    int		    *pwidthInit;/* pointer to widths			    */
    int		    fSorted;
{
    int		    n;		/* number of spans to fill after clipping   */
    DDXPointPtr     ppt;	/* pointer to start points after clipping   */
    int		    *pwidth;	/* pointer to widths after clipping	    */
    int		    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr	    pptFree;

    register int    dstAlign;   /* Last few bits of destination ptr	    */
    register int    srcAlign;   /* last few bits of source ptr		    */
    register int    shift;      /* Mostly dstAlign-srcAlign		    */
    register int    width;	/* width of current span		    */
    register int    widthDst;   /* width of drawable			    */
    register Pixel8 *pdstBase;  /* pointer to start pixel of drawable       */
    register Pixel8 *pdst;      /* pointer to start pixel of row	    */
    register Pixel8 *pd;	/* pointer into middle of row		    */

    PixmapPtr		ptile;
    int			tileWidth;  /* Width in pixels of tile		    */
    int			tileHeight; /* Height of tile			    */
    int			srcwidth;   /* Width of tile in bytes		    */

    Pixel8		*psrcBase;  /* Pointer to tile bits		    */
    register Pixel8     *psrc;      /* pointer to current byte in tile      */
    int			iy;	    /* Row offset into tile		    */
    int			ix;	    /* Column offset into tile		    */
    int			xorg, yorg; /* Drawable offset into tile	    */

    Pixel8		*psrcLine;  /* pointer to line of tile to use */
    int			w, srcw;

    cfbPrivGC		*gcPriv;
    CommandWord		ones = sfbCopyAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    n = nInit * miFindMaxBand(gcPriv->pCompositeClip);
    pwidthFree = pwidth = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = ppt = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if (!ppt || !pwidth) {
	if (ppt) DEALLOCATE_LOCAL(ppt);
	if (pwidth) DEALLOCATE_LOCAL(pwidth);
	return;
    }
    n = miClipSpans(gcPriv->pCompositeClip,
		     pptInit, pwidthInit, nInit, 
		     ppt, pwidth, fSorted);

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

    xorg = pDraw->x + (pGC->patOrg.x % tileWidth) - tileWidth;
    yorg = pDraw->y + (pGC->patOrg.y % tileHeight) - tileHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, widthDst);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, COPY);

    if (SCREENMEMORY(ptile)) {
	/* Tile is in offscreen sfb memory, so use screen/screen copy. */
	/* Make pdstBase one cycle off from psrcBase so that
	 * there are no conflicts 
	 */
	pdstBase = CYCLE_FB(pdstBase);
	while (n--) {
	    /* Compute left end of span. */
	    pdst = pdstBase + ppt->y * widthDst + ppt->x * SFBPIXELBYTES;
	    pdstBase = CYCLE_FB(pdstBase);
	    width = *pwidth;
    
	    /* Compute offsets into src. */
	    iy = (ppt->y - yorg) % tileHeight;
	    ix = (ppt->x - xorg) % tileWidth;
	    psrcLine = psrcBase + iy * srcwidth;
	    psrcLine = CYCLE_FB(psrcLine);
	    psrc = psrcLine + ix * SFBPIXELBYTES;
	    srcw = tileWidth - ix;
    
	    do { /* while pixels left to paint in span */
		w = width;
		if (w > srcw) { /* Ran out of tile bits */
		    w = srcw;
		}
		width -= w;
		pd = pdst;
		pdst += w * SFBPIXELBYTES;
    
		/* Tiling is like a one-line bitblt */
		srcAlign = (int)psrc & SFBALIGNMASK;
		dstAlign = (int)pd & SFBALIGNMASK;
		shift = dstAlign - srcAlign;
		if (shift < 0) {
		    /* Prime pump. */
		    shift += SFBALIGNMENT;
		    dstAlign += SFBALIGNMENT;
		}
		SFBSHIFT(sfb, shift);
		psrc -= srcAlign;
		pd -= dstAlign;
		psrc = CYCLE_FB(psrc);
		pd = CYCLE_FB(pd);
		SFBBYTESTOPIXELS(dstAlign);
		w += dstAlign;
		leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
		rightMask = SFBRIGHTCOPYMASK(w, ones);
		if (w <= SFBCOPYBITS) {
		    /* The mask fits into a single word */
		    mask = leftMask & rightMask;
		    /* Extend mask if it specifies a single memory
		       access to avoid a race condition in copy logic */
			if (dstAlign < SFBALIGNMENT) 
			    rightMask |= SFBRACECOPYMASK;
			else
			    rightMask |= (SFBRACECOPYMASK << SFBALIGNMENT);
		    /* Read source words and stuff them into sfb buffer */
		    SFBWRITE(psrc, rightMask);
		    SFBWRITE(pd, mask);
		} else {
		    /* Mask requires multiple words */
		    SFBWRITE(psrc, ones);
		    SFBWRITE(pd, leftMask);
		    for (m = w - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
			psrc += SFBCOPYBYTESDONE;
			pd += SFBCOPYBYTESDONE;
			SFBWRITE(psrc, ones);
			SFBWRITE(pd, ones);
		    }
		    
		    SFBWRITE(psrc+SFBCOPYBYTESDONE, rightMask);
		    SFBWRITE(pd+SFBCOPYBYTESDONE, rightMask);
		} /* if small copy else big copy */
		psrc = psrcLine;
		srcw = tileWidth;
	    } while (width > 0);
	    ppt++;
	    pwidth++;
	}
    } else {
	/* Tile is in main memory. */
	while (n--) {
	    /* Compute left end of span. */
	    pdst = pdstBase + ppt->y * widthDst + ppt->x * SFBPIXELBYTES;
	    pdstBase = CYCLE_FB(pdstBase);
	    width = *pwidth;
    
	    /* Compute offsets into src. */
	    iy = (ppt->y - yorg) % tileHeight;
	    ix = (ppt->x - xorg) % tileWidth;
	    psrcLine = psrcBase + iy * srcwidth;
    
	    psrc = psrcLine + ix * SFBSLEAZEPIXELBYTES;
	    srcw = tileWidth - ix;
    
	    do { /* while pixels left to paint in span */
		w = width;
		if (w > srcw) { /* Ran out of tile bits */
		    w = srcw;
		}
		width -= w;
		pd = pdst;
		pdst += w * SFBPIXELBYTES;
		pdst = CYCLE_FB(pdst);
    
		/* Tiling is like a one-line bitblt */
		srcAlign = (int)psrc & SFBSLEAZEALIGNMASK;
		dstAlign = (int)pd & SFBALIGNMASK;
		shift = dstAlign - srcAlign*SFBSLEAZEMULTIPLIER;
		if (shift < 0) {
		    /* Prime pump. */
		    shift += SFBALIGNMENT;
		    dstAlign += SFBALIGNMENT;
		}
		SFBSHIFT(sfb, shift);
		psrc -= srcAlign;
		pd -= dstAlign;
		SFBBYTESTOPIXELS(dstAlign);
		w += dstAlign;
		leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
		rightMask = SFBRIGHTCOPYMASK(w, ones);
		if (w <= SFBCOPYBITS) {
		    /* The mask fits into a single word */
		    mask = leftMask & rightMask;
		    /* Read source words and stuff them into sfb buffer */
		    SFBBUFFILL(sfb, psrc, rightMask);
		    SFBWRITE(pd, mask);
		} else {
		    /* Mask requires multiple words */
		    SFBBUFFILLALL(sfb, psrc);
		    SFBWRITE(pd, leftMask);
		    for (m = w - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
			psrc += SFBSLEAZEBYTESDONE;
			pd += SFBCOPYBYTESDONE;
			SFBBUFFILLALL(sfb, psrc);
			SFBWRITE(pd, ones);
		    }
		    
		    SFBBUFFILL(sfb, psrc+SFBSLEAZEBYTESDONE, rightMask);
		    SFBWRITE(pd + SFBCOPYBYTESDONE, rightMask);
		} /* if small copy else big copy */
		psrc = psrcLine;
		srcw = tileWidth;
	    } while (width > 0);
	    ppt++;
	    pwidth++;
	}
    } /* end if tile in screen else tile in main memory */
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

/* 
 * Set a list of spans with a list of pixels
 */

void sfbSetSpans(pDraw, pGC, psrc, pptInit, pwidthInit, nInit, fSorted)
    DrawablePtr	    pDraw;
    GC		    *pGC;
    Pixel8	    *psrc;
    DDXPointPtr     pptInit;	/* pointer to start points		    */
    int		    nInit;	/* number of spans to fill		    */
    int		    *pwidthInit;/* pointer to widths			    */
    int		    fSorted;
{
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    SFBFLUSHMODE(sfb, SIMPLE);
    /* Who gives a (*&?  No one calls this anyway. */
    cfbSetSpans(pDraw, pGC, psrc, pptInit, pwidthInit, nInit, fSorted);
}


#endif /* COMPILEEVERYTHING */


#ifdef DOOPAQUESTIPPLE
#define SFBPARTWRITE(sfb, pdst, srcbits, mask) {	\
    SFBPIXELMASK(sfb, mask);				\
    SFBWRITE(pdst, srcbits);				\
}
#define SFBPARTWRITEONEWORD(sfb, pdst, srcbits, mask) { \
    SFBPIXELMASK(sfb, mask);				\
    SFBWRITEONEWORD(sfb, pdst, srcbits);		\
}
#define SFBPARTWRITECYCLE(pdst)				\
    pdst = CYCLE_FB(pdst)
#else
#define SFBPARTWRITE(sfb, pdst, srcbits, mask) {	\
    SFBWRITE(pdst, (srcbits) & (mask));			\
}
#define SFBPARTWRITEONEWORD(sfb, pdst, srcbits, mask) { \
    SFBWRITEONEWORD(sfb, pdst, (srcbits) & (mask));	\
}
#define SFBPARTWRITECYCLE(pdst)	
#endif


#ifndef COMPILEEVERYTHING
/* 
 * Transparent or opaque stipple a list of rects
 */

/* ||| Right now we're just going to slime our way out of this, and base the sfb
   code heavily upon the existing cfb code.  This could be much improved by
   trying to paint things more like text does...paint up to a boundary, then
   paint full SFBBUSBITS words (regardless of how narrow the stipple is), then
   paint the final few bits. */
   
void SFBSTIPPLEAREA(pDraw, nrect, prect, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    GCPtr		pGC;
{
    register int	width, w;   /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register Pixel8	*p, *tp;    /* pointer to pixels we're writing      */

    PixmapPtr		pstipple;
    int			stippleWidth; /* Width in bits of data in stipple   */
    int			stippleHeight;/* Height in bits of data in stipple  */
    int			srcWidth;   /* Stipple width in physical words      */
    CommandWord		*psrcBase;  /* Pointer to stipple bits		    */
    CommandWord		*psrc;      /* Pointer to word in stipple bits      */
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Rectangle offset into stipple 	    */
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;
#ifdef DOOPAQUESTIPPLE
    CommandWord		ones = sfbStippleAll1;
    CommandWord		leftMask, rightMask;
    int			tw;
#endif
#ifdef DOTRANSPARENTSTIPPLE
    CommandWord		ones = sfbBusAll1;
#endif

    if (nrect == 0) return;

    pstipple	    = pGC->stipple;
    psrcBase	    = (CommandWord *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind / SFBBUSBYTES;
    stippleWidth    = pstipple->drawable.width;
    stippleHeight   = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       xorg and yorg to be below any possible x and y that we have to paint.
     */

    xorg = (pGC->patOrg.x % stippleWidth) - stippleWidth;
    yorg = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);

    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef DOOPAQUESTIPPLE
    SFBMODE(sfb, OPAQUESTIPPLE);
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
    pdstBase += pDraw->y * dstwidth + pDraw->x * SFBPIXELBYTES;

    do { /* Each rectangle */
	width = prect->width;
	height = prect->height;
	pdst = pdstBase + prect->y * dstwidth + prect->x * SFBPIXELBYTES;
	pdstBase = CYCLE_FB(pdstBase);
	ix = (prect->x - xorg) % stippleWidth;
	iy = (prect->y - yorg) % stippleHeight;

	do { /* For each line of height */
	    register int w, tix, ixBUSBITS, srcw, align;
	    register CommandWord srcbits;
	    CommandWord	    *psrcStart;

	    psrcStart = psrcBase + (iy * srcWidth);
	    psrc = psrcStart + (ix / SFBBUSBITS);
	    srcw = stippleWidth - ix;
	    w = width;
	    p = pdst;
	    tix = ix & SFBBUSBITSMASK;
	    srcbits = (*psrc) >> tix;
	    ixBUSBITS = SFBBUSBITS - tix;

	    for (;;) {
		p = CYCLE_FB(p);

		if (ixBUSBITS > srcw) {
		    /* Oops, ran out of stipple bits. */
		    ixBUSBITS = srcw;
#ifdef DOTRANSPARENTSTIPPLE
		    srcbits &= SFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
		}
		if (ixBUSBITS > w) {
		    /* Oops, ran out of rectangle */
		    ixBUSBITS = w;
#ifdef DOTRANSPARENTSTIPPLE
		    srcbits &= SFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
		}
    
		align = (int) p & SFBALIGNMASK;
		p -= align;
		SFBBYTESTOPIXELS(align);

		/* Paint srcbits */
#if SFBBUSBITS <= SFBSTIPPLEBITS
# ifdef DOOPAQUESTIPPLE
		tw = ixBUSBITS + align;
		leftMask = SFBLEFTSTIPPLEMASK(align, ones);
		rightMask = SFBRIGHTSTIPPLEMASK(tw, ones);
		if (tw <= SFBSTIPPLEBITS) {
		    SFBPIXELMASK(sfb, leftMask & rightMask);
		    SFBWRITE(p, srcbits << align);
		} else {
		    SFBPIXELMASK(sfb, leftMask);
		    SFBWRITE(p, srcbits << align);
		    p = CYCLE_FB(p);
		    SFBPIXELMASK(sfb, rightMask);
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE,
			srcbits >> SFBRIGHTSTIPPLESHIFT(align));
		}
# else
		SFBWRITE(p, srcbits << align);
		if (ixBUSBITS+align > SFBSTIPPLEBITS)  {
		    /* Finish off srcbits */
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE, 
			srcbits >> SFBRIGHTSTIPPLESHIFT(align));
		}
# endif
#else /* SFBBUSBITS > SFBSTIPPLEBITS */
# ifdef DOOPAQUESTIPPLE
		tw = ixBUSBITS + align;
		leftMask = SFBLEFTSTIPPLEMASK(align, ones);
		rightMask = SFBRIGHTSTIPPLEMASK(tw, ones);
		if (tw <= SFBSTIPPLEBITS) {
		    SFBPIXELMASK(sfb, leftMask & rightMask);
		    SFBWRITE(p, srcbits << align);
		} else if (tw <= 2*SFBSTIPPLEBITS) {
		    SFBPIXELMASK(sfb, leftMask);
		    SFBWRITE(p, srcbits << align);
		    p = CYCLE_FB(p);
		    SFBPIXELMASK(sfb, rightMask);
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE, 
			srcbits >> SFBRIGHTSTIPPLESHIFT(align));
		} else {
		    SFBPIXELMASK(sfb, leftMask);
		    SFBWRITE(p, srcbits << align);
		    p = CYCLE_FB(p);
		    srcbits >>= SFBRIGHTSTIPPLESHIFT(align);
		    tp = p + SFBSTIPPLEBYTESDONE;
		    tp = CYCLE_FB(tp);
		    tw -= 2*SFBSTIPPLEBITS;
		    do {
			SFBWRITE(tp, srcbits);
			srcbits >>= SFBSTIPPLEBITS;
			tp += SFBSTIPPLEBYTESDONE;
			tw -= SFBSTIPPLEBITS;
		    } while (tw > 0);
		    tp = CYCLE_FB(tp);
		    SFBPIXELMASK(sfb, rightMask);
		    SFBWRITE(tp, srcbits);
		}
# else
		SFBWRITE(p, srcbits << align);
		/* We take a simpler approach here, because we know that
		   SFBRIGHTSTIPPLESHIFT cannot return 0, as it can above if
		   SFBSTIPPLEBITS == SFBBUSBITS and MODULOSHIFTS. */
		srcbits >>= SFBRIGHTSTIPPLESHIFT(align);
		if (srcbits != 0) {
		    tp = p + SFBSTIPPLEBYTESDONE;
		    do {
			SFBWRITE(tp, srcbits);
			srcbits >>= SFBSTIPPLEBITS;
			tp += SFBSTIPPLEBYTESDONE;
		    } while (srcbits != 0);
		}
# endif
#endif
		w -= ixBUSBITS;

		if (w == 0) break; /* LOOP TERMINATOR */

		p += (ixBUSBITS+align) * SFBPIXELBYTES;
		srcw -= ixBUSBITS;

		ixBUSBITS = SFBBUSBITS;
		psrc++;
		if (srcw == 0) {
		    srcw = stippleWidth;
		    psrc = psrcStart;
		}
		srcbits = *psrc;
	    } /* end loop */

	    iy++;
	    if (iy == stippleHeight) iy = 0;
	    pdst += dstwidth;
	    height--;
	} while (height > 0);
	prect++;
	nrect--;
    } while (nrect > 0);
}



/* Fill spans with transparent or opaque stipples that aren't SFBBUSBITS wide */
void SFBSTIPPLESPAN(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDraw;
    GC		*pGC;
    int		nInit;		/* number of spans to fill		    */
    DDXPointPtr pptInit;	/* pointer to start points		    */
    int		*pwidthInit;	/* pointer to widths			    */
    int		fSorted;
{
    int		    n;		/* number of spans to fill after clipping   */
    DDXPointPtr     ppt;	/* pointer to start points after clipping   */
    int		    *pwidth;	/* pointer to widths after clipping	    */
    CommandWord     srcbits;
    CommandWord	    *psrcStart;
    CommandWord	    *psrc;
    register Pixel8 *p, *tp;	/* pointer to current byte in bitmap	    */
    register int    w, ix, ixBUSBITS, srcw, align;
    int		    iy;	/* first line of stipple to use */
    Pixel8	    *addrlBase;	/* pointer to start of bitmap */
    int		    nlwidth;	/* width in longwords of bitmap */
    PixmapPtr       pStipple;	/* pointer to stipple we want to fill with */
    int		    width,  xorg, yorg;
    int		    stwidth;	/* Width of stipple in words		    */
    int		    stippleWidth;   /* Width in bits of data in stipple	    */
    int		    stippleHeight;  /* Height in bits of data in stipple    */  
    int		    *pwidthFree;    /* copies of the pointers to free */
    DDXPointPtr     pptFree;
    cfbPrivGC       *gcPriv;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;
#ifdef DOOPAQUESTIPPLE
    CommandWord     ones = sfbStippleAll1;
    CommandWord	    leftMask, rightMask;
    int		    tw;
#endif
#ifdef DOTRANSPARENTSTIPPLE
    CommandWord     ones = sfbBusAll1;
#endif

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    n = nInit * miFindMaxBand(gcPriv->pCompositeClip);
    pwidthFree = pwidth = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = ppt = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if (!ppt || !pwidth) {
	if (ppt) DEALLOCATE_LOCAL(ppt);
	if (pwidth) DEALLOCATE_LOCAL(pwidth);
	return;
    }
    n = miClipSpans(gcPriv->pCompositeClip,
		     pptInit, pwidthInit, nInit, 
		     ppt, pwidth, fSorted);

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

    stwidth = pStipple->devKind / SFBBUSBYTES;
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;

    /*
     *	The Target:
     *		Depth = SFBPIXELBITS
     *		Width = determined from *pwidth
     *		Words per scanline = nlwidth
     *		Pointer to pixels = addrlBase
     */

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       xorg and yorg to be below any possible x and y that we have to paint.
     */

    xorg = pDraw->x + (pGC->patOrg.x % stippleWidth) - stippleWidth;
    yorg = pDraw->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;

    DrawableBaseAndWidth(pDraw, addrlBase, nlwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef DOOPAQUESTIPPLE
    SFBMODE(sfb, OPAQUESTIPPLE);
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif

    while (n--) {
	w = *pwidth;
	p = addrlBase + (ppt->y * nlwidth) + ppt->x * SFBPIXELBYTES;
	addrlBase = CYCLE_FB(addrlBase);

	ix = (ppt->x - xorg) % stippleWidth;
	iy = (ppt->y - yorg) % stippleHeight;
	psrcStart = (CommandWord *) pStipple->devPrivate.ptr + (iy * stwidth);
	psrc = psrcStart + (ix / SFBBUSBITS);
	srcw = stippleWidth - ix;
	ix &= SFBBUSBITSMASK;
	srcbits = (*psrc) >> ix;
	ixBUSBITS = SFBBUSBITS - ix;

	for (;;) {
	    p = CYCLE_FB(p);
	    if (ixBUSBITS > srcw) {
		/* Oops, ran out of stipple bits. */
		ixBUSBITS = srcw;
#ifdef DOTRANSPARENTSTIPPLE
		srcbits &= SFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
	    }
	    if (ixBUSBITS > w) {
		/* Oops, ran out of rectangle */
		ixBUSBITS = w;
#ifdef DOTRANSPARENTSTIPPLE
		srcbits &= SFBRIGHTBUSMASK(ixBUSBITS, ones);
#endif
	    }
	    /* Paint srcbits aligned to SFB boundary */
	    align = (int) p & SFBALIGNMASK;
	    p -= align;
	    SFBBYTESTOPIXELS(align);

	    /* Paint srcbits */
#if SFBBUSBITS <= SFBSTIPPLEBITS
# ifdef DOOPAQUESTIPPLE
	    tw = ixBUSBITS + align;
	    leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	    rightMask = SFBRIGHTSTIPPLEMASK(tw, ones);
	    if (tw <= SFBSTIPPLEBITS) {
		SFBPIXELMASK(sfb, leftMask & rightMask);
		SFBWRITE(p, srcbits << align);
	    } else {
		SFBPIXELMASK(sfb, leftMask);
		SFBWRITE(p, srcbits << align);
		p = CYCLE_FB(p);
		SFBPIXELMASK(sfb, rightMask);
		SFBWRITE(p+SFBSTIPPLEBYTESDONE, 
		    srcbits >> SFBRIGHTSTIPPLESHIFT(align));
	    }
# else
	    SFBWRITE(p, srcbits << align);
	    if (ixBUSBITS+align > SFBSTIPPLEBITS)  {
		/* Finish off srcbits */
		SFBWRITE(p + SFBSTIPPLEBYTESDONE, 
			srcbits >> SFBRIGHTSTIPPLESHIFT(align));
	    }
# endif
#else /* SFBBUSBITS > SFBSTIPPLEBITS */
# ifdef DOOPAQUESTIPPLE
	    tw = ixBUSBITS + align;
	    leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	    rightMask = SFBRIGHTSTIPPLEMASK(tw, ones);
	    if (tw <= SFBSTIPPLEBITS) {
		SFBPIXELMASK(sfb, leftMask & rightMask);
		SFBWRITE(p, srcbits << align);
	    } else if (tw <= 2*SFBSTIPPLEBITS) {
		SFBPIXELMASK(sfb, leftMask);
		SFBWRITE(p, srcbits << align);
		p = CYCLE_FB(p);
		SFBPIXELMASK(sfb, rightMask);
		SFBWRITE(p+SFBSTIPPLEBYTESDONE,
		    srcbits >> SFBRIGHTSTIPPLESHIFT(align));
	    } else {
		SFBPIXELMASK(sfb, leftMask);
		SFBWRITE(p, srcbits << align);
		p = CYCLE_FB(p);
		srcbits >>= SFBRIGHTSTIPPLESHIFT(align);
		tp = p + SFBSTIPPLEBYTESDONE;
		tw -= 2*SFBSTIPPLEBITS;
		do {
		    SFBWRITE(tp, srcbits);
		    srcbits >>= SFBSTIPPLEBITS;
		    tp += SFBSTIPPLEBYTESDONE;
		    tw -= SFBSTIPPLEBITS;
		} while (tw > 0);
		tp = CYCLE_FB(tp);
		SFBPIXELMASK(sfb, rightMask);
		SFBWRITE(tp, srcbits);
	    }
# else
	    SFBWRITE(p, srcbits << align);
	    /* We take a simpler approach here, because we know that
	       SFBRIGHTSTIPPLESHIFT cannot return 0, as it can above if
	       SFBSTIPPLEBITS == SFBBUSBITS and MODULOSHIFTS. */
	    srcbits >>= SFBRIGHTSTIPPLESHIFT(align);
	    if (srcbits != 0) {
		tp = p + SFBSTIPPLEBYTESDONE;
		do {
		    SFBWRITE(tp, srcbits);
		    srcbits >>= SFBSTIPPLEBITS;
		    tp += SFBSTIPPLEBYTESDONE;
		} while (srcbits != 0);
	    }
# endif
#endif

	    w -= ixBUSBITS;

	    if (w == 0) break; /* LOOP TERMINATOR */

	    p += (ixBUSBITS+align) * SFBPIXELBYTES;
	    srcw -= ixBUSBITS;

	    ixBUSBITS = SFBBUSBITS;
	    psrc++;
	    if (srcw == 0) {
		srcw = stippleWidth;
		psrc = psrcStart;
	    }
	    srcbits = *psrc;
	} /* end for loop */
	ppt++;
	pwidth++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

#endif /* ndef COMPILEEVERYTHING */


/* 
 * Transparent stipple, opaque stipple, or tile a list of rectangles, where
 * the stipple/tile width == SFBBUSBITS.  (The stipple/tile has been replicated
 * if its width was a power of 2 smaller than SFBBUSBITS bits.)
 */

#define StartStippleLoop(psrc, srcbits, ix, ixBUSBITS)  \
{							\
    srcbits = *psrc;					\
    /* Do alignment of srcbits */			\
    srcbits = (srcbits << ixBUSBITS) | (srcbits >> ix); \
} /* StartStippleLoop */

/*
 * If the server is 64-bit based, all pixmaps are padded to 64-bit.
 * Incrementing through 32-bit wide pixmaps requires a 64-bit
 * increment or 2 32-bit increments instead of one 32-bit increment.
 */
#if LONG_BIT == 32
#define PIXMAP_Y_MULT 	1
#else /* LONG_BIT == 64 */
#define PIXMAP_Y_MULT 	2
#endif /* LONG_BIT */

#define EndStippleLoop(pdst, dstwidth, psrcBase, psrc, iy, stippleHeight,   \
    height)								    \
{							\
    pdst += dstwidth;					\
    iy++;						\
    psrc+=PIXMAP_Y_MULT;				\
    if (iy == stippleHeight) {				\
	iy = 0;						\
	psrc = psrcBase;				\
    }							\
    height--;						\
} /* EndStippleLoop */

void
SFBTILESTIPPLEAREAWORD(pDraw, nrect, prect, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    GCPtr		pGC;
{
    register int	width;      /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register int	align;      /* alignment of pdst (0..SFBALIGNMASK)  */
    register Pixel8     *p;	    /* Temp dest pointer		    */

    PixmapPtr		pstipple;
    int			stippleHeight;/* Height in bits of data in stipple  */
    CommandWord		*psrcBase;  /* Pointer to base of stipple bits	    */
    CommandWord		*psrc;      /* Pointer to stipple bits for scanline */
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Rectangle offset into stipple 	    */
    register int	ixBUSBITS;  /* SFBBUSBITS - ix			    */
    CommandWord		srcbits;    /* Actual source bits for scanline      */
    CommandWord		ones = sfbStippleAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

/*
The trick here is to take advantage of stipple patterns that can be replicated
to a width of SFBBUSBITS bits.  Then we can rotate each row of the stipple by a
good amount, and always use the rotated bits as is.

For transparent stipples and tiles, we use 0 bits at the left and right ragged
edges.  For opaque stipples, we us the pixel mask register on the SFB.

We also use this code for tiles that are (or are replicated) to SFBBUSBITS
wide.  In this case we load the tile pixels into the foreground register, then
act like we're filling a solid area.  We are guaranteed that really small
tiles, like the ones used here, are never allocated to sfb memory, but always
reside in main memory.

*/

    if (nrect == 0) return;

#ifdef DOTILE
    pstipple      = pGC->tile.pixmap;
#else
    pstipple      = pGC->stipple;
#endif
    psrcBase      = (CommandWord *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       yorg to be below any possible y that we have to paint.  xorg uses
       masking to compute mod, so we don't have to worry about it being < 0.
     */

    xorg = pGC->patOrg.x;
    yorg = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef DOOPAQUESTIPPLE
    SFBMODE(sfb, OPAQUESTIPPLE);
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
    pdstBase += pDraw->y * dstwidth + pDraw->x * SFBPIXELBYTES;

    do { /* Each rectangle */
	/* Compute upper left corner. */
	pdst = pdstBase + prect->y * dstwidth + prect->x * SFBPIXELBYTES;
	pdstBase = CYCLE_FB(pdstBase);
	width = prect->width;
	height = prect->height;

	/* Compute offsets into stipple. */
	ix = (prect->x - xorg);
	iy = (prect->y - yorg) % stippleHeight;

	psrc = psrcBase + PIXMAP_Y_MULT * iy;

	/* Offset pdst back so SFB-aligned. */
	align = (int) pdst & SFBALIGNMASK;
	pdst -= align;

	SFBBYTESTOPIXELS(align);
	ix -= align;
	width += align;
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(width, ones);

	/* Compute alignment of srcbits */
#ifdef DOTILE
	ComputeRotateAmounts(ix, ixBUSBITS, ix << 3);
#else
	ComputeRotateAmounts(ix, ixBUSBITS, ix);
#endif

	if (width <= SFBSTIPPLEBITS) {
	    /* One-word mask, one write to sfb. */
	    mask = leftMask & rightMask;
	    do { /* For each line of height */
		StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
#ifdef DOTILE
		SFBFOREGROUND(sfb, srcbits);
		SFBWRITEONEWORD(sfb, pdst, mask);
#else
		SFBPARTWRITEONEWORD(sfb, pdst, srcbits, mask);
#endif
		EndStippleLoop(pdst, dstwidth, 
		    psrcBase, psrc, iy, stippleHeight, height);
	    } while (height != 0);

#if (SFBBUSBITS <= SFBSTIPPLEBITS) || defined(DOTILE)
	} else {
	    /* Multi-word mask */
	    do { /* For each line of height */
		StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
		p = pdst;
# ifdef DOTILE
		SFBFOREGROUND(sfb, srcbits);
		SFBWRITE(pdst, leftMask);
		for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		    p += SFBSTIPPLEBYTESDONE;
		    SFBWRITE(p, SFBSTIPPLEALL1);
		}
		SFBWRITE(p+SFBSTIPPLEBYTESDONE, rightMask);
# else /* !DOTILE */
		SFBPARTWRITE(sfb, p, srcbits, leftMask);
		SFBPARTWRITECYCLE(p);
		for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		    p += SFBSTIPPLEBYTESDONE;
		    SFBWRITE(p, srcbits);
		}
		SFBPARTWRITECYCLE(p);
		SFBPARTWRITE(sfb, p+SFBSTIPPLEBYTESDONE, srcbits, rightMask);
# endif /* DOTILE */
		EndStippleLoop(pdst, dstwidth, 
		    psrcBase, psrc, iy, stippleHeight, height);
		pdst = CYCLE_FB(pdst);
	    } while (height != 0);

#else /* SFBBUSBITS > SFBSTIPPLEBITS && !DOTILE, so must break up srcbits. */

	} else if (width <= SFBBUSBITS) {
	    /* Multi-word mask, but at most one complete word of srcbits. */
	    do { /* For each line of height */
		StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
		p = pdst;
		SFBPARTWRITE(sfb, p, srcbits, leftMask);
		SFBPARTWRITECYCLE(p);
# if SFBBUSBITS > 2 * SFBSTIPPLEBITS
		for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		    srcbits >>= SFBSTIPPLEBITS;
		    p += SFBSTIPPLEBYTESDONE;
		    SFBWRITE(p, srcbits);
		}
# endif /* SFBBUSBITS > 2 * SFBSTIPPLEBITS */
		srcbits >>= SFBSTIPPLEBITS;
		SFBPARTWRITECYCLE(p);
		SFBPARTWRITE(sfb, p+SFBSTIPPLEBYTESDONE, srcbits, rightMask);
		EndStippleLoop(pdst, dstwidth, 
		    psrcBase, psrc, iy, stippleHeight, height);
		pdst = CYCLE_FB(pdst);
	    } while (height != 0);
	} else {
	    /* Multi-word mask, multiple repetitions of srcbits. */
	    do { /* For each line of height */
		StartStippleLoop(psrc, srcbits, ix, ixBUSBITS);
		p = pdst;
		m = width - SFBBUSBITS;
# ifdef DOOPAQUESTIPPLE
		SFBPIXELMASK(sfb, leftMask);
		StippleEntireWord(p, srcbits);
		p += SFBBUSBITS*SFBPIXELBYTES;
		p = CYCLE_FB(p);
		m -= SFBBUSBITS;
		while (m > 0) {
		    StippleEntireWord(p, srcbits);
		    p += SFBBUSBITS*SFBPIXELBYTES;
		    m -= SFBBUSBITS;
		}
#else /* DOOPAQUESTIPPLE */
		do {
		    StippleEntireWord(p, srcbits);
		    p += SFBBUSBITS*SFBPIXELBYTES;
		    m -= SFBBUSBITS;
		} while (m > 0);
#endif /* DOOPAQUESTIPPLE */
# if SFBBUSBITS <= 2 * SFBSTIPPLEBITS
		m = m + (SFBBUSBITS - SFBSTIPPLEBITS);
		if (m > 0) {
		    SFBWRITE(p, srcbits);
		    srcbits >>= SFBSTIPPLEBITS;
		    p += SFBSTIPPLEBYTESDONE;
		}
# else /* !SFBBUSBITS <= 2 * SFBSTIPPLEBITS */
		for (m = m + (SFBBUSBITS - SFBSTIPPLEBITS);
		     m > 0; m -= SFBSTIPPLEBITS) {
		    SFBWRITE(p, srcbits);
		    srcbits >>= SFBSTIPPLEBITS;
		    p += SFBSTIPPLEBYTESDONE;
		}
# endif /* SFBBUSBITS <= 2 * SFBSTIPPLEBITS */
		SFBPARTWRITECYCLE(p);
		SFBPARTWRITE(sfb, p, srcbits, rightMask);
		EndStippleLoop(pdst, dstwidth, 
		    psrcBase, psrc, iy, stippleHeight, height);
	    } while (height !=0);
#endif /* (SFBBUSBITS <= SFBSTIPPLEBITS) || defined(DOTILE) */
	} /* if skinny else fat rectangle */

	prect++;
	nrect--;
    } while (nrect > 0);
#ifdef DOTILE
    /* Restore foreground register */
    SFBFOREGROUND(sfb, pGC->fgPixel);
#endif
}


/* 
 * Transparent stipple, opaque stipple, or tile a list of spans, where
 * the stipple/tile width == SFBBUSBITS.  (The stipple/tile has been replicated
 * if its width was a power of 2 smaller than SFBBUSBITS bits.)
 */

void SFBTILESTIPPLESPANWORD(pDraw,  pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr		pDraw;
    GCPtr		pGC;
    int			nInit;	    /* number of spans to fill		    */
    DDXPointPtr		pptInit;    /* list of start points		    */
    int			*pwidthInit;/* list of widths			    */
    int			fSorted;
{
    int			n;	    /* post-clip number of spans to fill    */
    DDXPointPtr		ppt;	    /* post-clip list of start points       */
    int			*pwidth;    /* post-clip list of widths		    */
    int			*pwidthFree;/* copies of the pointers to free	    */
    DDXPointPtr		pptFree;
    register int	width;	    /* width of current span		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*p;	    /* pointer to start pixel of span	    */
    register int	align;      /* alignment of pdst (0..SFBALIGNMASK)  */

    PixmapPtr		pstipple;
    int			stippleHeight;/* Height in bits of data in stipple  */
    CommandWord		*psrcBase;  /* Pointer to base of stipple bits	    */
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Span offset into stipple 	    */
    register int	ixBUSBITS;  /* SFBBUSBITS - ix			    */
    CommandWord		srcbits;    /* Actual source bits for scanline      */
    int			m;
    CommandWord		ones = sfbStippleAll1;
    CommandWord		mask, leftMask, rightMask;
    cfbPrivGC		*gcPriv;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

/*
The trick here is to take advantage of stipple patterns that can be replicated
to a width of SFBBUSBITS.  Then we can rotate each row of the stipple by a good
amount, and always use the rotated bits as is.  We solve initial alignment
problems by backing up each row to a word boundary, and substituting 0 bits
at the bottom of the stipple the very first time through the loop.  We
solve ending alignment problems by substituting 0's for the high 1-31 bits of
the stipple.
*/

    if (nInit == 0) return;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    n = nInit * miFindMaxBand(gcPriv->pCompositeClip);
    pwidthFree = pwidth = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = ppt = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if (!ppt || !pwidth) {
	if (ppt) DEALLOCATE_LOCAL(ppt);
	if (pwidth) DEALLOCATE_LOCAL(pwidth);
	return;
    }
    n = miClipSpans(gcPriv->pCompositeClip,
		     pptInit, pwidthInit, nInit, 
		     ppt, pwidth, fSorted);

#ifdef DOTILE
    pstipple      = pGC->tile.pixmap;
#else
    pstipple      = pGC->stipple;
#endif
    psrcBase      = (CommandWord *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       yorg to be below any possible y that we have to paint.  xorg uses
       masking to compute mod, so we don't have to worry about it being < 0.
     */

    xorg = pDraw->x + pGC->patOrg.x;
    yorg = pDraw->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef DOOPAQUESTIPPLE
    SFBMODE(sfb, OPAQUESTIPPLE);
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif

    while (n > 0) { /* Each span */
	p = pdstBase + ppt->y * dstwidth + ppt->x * SFBPIXELBYTES;
	pdstBase = CYCLE_FB(pdstBase);

	/* Compute left edge. */
	width = *pwidth;

	/* Compute offsets into stipple. */
	ix = (ppt->x - xorg);
	iy = (ppt->y - yorg) % stippleHeight;
	srcbits = psrcBase[PIXMAP_Y_MULT * iy];

	/* Offset p back so SFB-aligned. */
	align = (int) p & SFBALIGNMASK;
	p -= align;

	SFBBYTESTOPIXELS(align);
	ix -= align;
	width += align;
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(width, ones);

	/* Do alignment of srcbits */
#ifdef DOTILE
	ComputeRotateAmounts(ix, ixBUSBITS, ix << 3);
#else
	ComputeRotateAmounts(ix, ixBUSBITS, ix);
#endif
	srcbits = (srcbits << ixBUSBITS) | (srcbits >> ix);

	if (width <= SFBSTIPPLEBITS) {
	    /* One-word mask */
	    mask = leftMask & rightMask;
#ifdef DOTILE
	    SFBFOREGROUND(sfb, srcbits);
	    SFBWRITEONEWORD(sfb, p, mask);
#else
	    SFBPARTWRITEONEWORD(sfb, p, srcbits, mask);
#endif
#if (SFBBUSBITS <= SFBSTIPPLEBITS) || defined(DOTILE)
	} else {
	    /* Multi-word mask */
# ifdef DOTILE
	    SFBFOREGROUND(sfb, srcbits);
	    SFBWRITE(p, leftMask);
	    for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		p += SFBSTIPPLEBYTESDONE;
		SFBWRITE(p, SFBSTIPPLEALL1);
	    }
	    SFBWRITE(p+SFBSTIPPLEBYTESDONE, rightMask);
# else /* !DOTILE */
	    SFBPARTWRITE(sfb, p, srcbits, leftMask);
	    SFBPARTWRITECYCLE(p);
	    for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		p += SFBSTIPPLEBYTESDONE;
		SFBWRITE(p, srcbits);
	    }
	    SFBPARTWRITECYCLE(p);
	    SFBPARTWRITE(sfb, p+SFBSTIPPLEBYTESDONE, srcbits, rightMask);
# endif /* DOTILE */

#else /* SFBBUSBITS > SFBSTIPPLEBITS && !DOTILE, so must break up srcbits. */

	} else if (width <= SFBBUSBITS) {
	    /* Multi-word mask, but at most one complete word of srcbits. */
	    SFBPARTWRITE(sfb, p, srcbits, leftMask);
	    SFBPARTWRITECYCLE(p);
# if SFBBUSBITS > 2 * SFBSTIPPLEBITS
	    for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		srcbits >>= SFBSTIPPLEBITS;
		p += SFBSTIPPLEBYTESDONE;
		SFBWRITE(p, srcbits);
	    }
# endif
	    srcbits >>= SFBSTIPPLEBITS;
	    SFBPARTWRITECYCLE(p);
	    SFBPARTWRITE(sfb, p+SFBSTIPPLEBYTESDONE, srcbits, rightMask);
	} else {
	    /* Multi-word mask, multiple repetitions of srcbits. */
	    m = width - SFBBUSBITS;
# ifdef DOOPAQUESTIPPLE
	    SFBPIXELMASK(sfb, leftMask);
	    StippleEntireWord(p, srcbits);
	    p = CYCLE_FB(p);
	    p += SFBBUSBITS*SFBPIXELBYTES;
	    m -= SFBBUSBITS;
	    while (m > 0) {
		StippleEntireWord(p, srcbits);
		p += SFBBUSBITS*SFBPIXELBYTES;
		m -= SFBBUSBITS;
	    } 
# else
	    do {
		StippleEntireWord(p, srcbits);
		p += SFBBUSBITS*SFBPIXELBYTES;
		m -= SFBBUSBITS;
	    } while (m > 0);
# endif
# if SFBBUSBITS <= 2 * SFBSTIPPLEBITS
	    m = m + (SFBBUSBITS - SFBSTIPPLEBITS);
	    if (m > 0) {
		SFBWRITE(p, srcbits);
		srcbits >>= SFBSTIPPLEBITS;
		p += SFBSTIPPLEBYTESDONE;
	    }
# else
	    for (m = m + (SFBBUSBITS - SFBSTIPPLEBITS);
		 m > 0; m -= SFBSTIPPLEBITS) {
		SFBWRITE(p, srcbits);
		srcbits >>= SFBSTIPPLEBITS;
		p += SFBSTIPPLEBYTESDONE;
	    }
# endif
#endif
	} /* if skinny else fatter else fattest span */

	ppt++;
	pwidth++;
	n--;
    } /* while (n > 0) */;
#ifdef DOTILE
    /* Restore foreground register */
    SFBFOREGROUND(sfb, pGC->fgPixel);
#endif
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


/* 
 * Transparent stipple, opaque stipple, or tile a list of rectangles, where
 * the stipple/tile width == SFBBUSBITS.  (The stipple/tile has been replicated
 * if its width was a power of 2 smaller than SFBBUSBITS bits.)  AND, the
 * height of the tile is a power of 2.
 */

#define StartStippleLoop2(psrcBase, iy, heightMask, srcbits, ix, ixBUSBITS) \
{									    \
    srcbits = *(CommandWord *)(psrcBase + PIXMAP_Y_MULT*(iy & heightMask)); \
    /* Do alignment of srcbits */					    \
    srcbits = (srcbits << ixBUSBITS) | (srcbits >> ix);			    \
} /* StartStippleLoop2 */

#define EndStippleLoop2(pdst, dstwidth, iy, height) \
{						    \
    pdst += dstwidth;				    \
    iy += SFBBUSBYTES;				    \
    height--;					    \
} /* EndStippleLoop2 */


void
SFBTILESTIPPLEAREAWORD2(pDraw, pGC, nrectFill, prectBase)
    DrawablePtr     pDraw;
    GCPtr	    pGC;
    int		    nrectFill;	    /* number of rectangles to fill */
    xRectangle      *prectBase;	    /* Pointer to first rectangle to fill */
{
    xRectangle      *prect;
    int		    nrect;
    RegionPtr       prgnClip;
    int		    dstwidth;	    /* width of drawable		    */
    Pixel8	    *pdstBase;	    /* pointer to start pixel of drawable   */
    Pixel8	    *pdst;	    /* pointer to start pixel of row	    */
    Pixel8	    *p;		    /* Temp dest pointer		    */
    int		    numRects;	    /* Number of clipping rectangles	    */
    BoxPtr	    pextent;
    cfbPrivGC       *gcPriv;
    int		    xDraw, yDraw;
    BoxPtr	    pbox;	    /* pointer to one region clip box       */
    int		    width;	    /* width of current rect		    */
    int		    height;	    /* height of current rect		    */
    int		    align;	    /* alignment of pdst (0..SFBALIGNMASK)  */

    PixmapPtr	    pstipple;
    int		    heightMask;     /* Height mask for stipple		    */
    Bits8	    *psrcBase;      /* Pointer to base of stipple bits	    */
    int		    xorg, yorg;     /* Drawable offset into stipple	    */
    int		    ix, iy;	    /* Rectangle offset into stipple 	    */
    int		    ixBUSBITS;  /* SFBBUSBITS - ix			    */
    CommandWord     srcbits;    /* Actual source bits for scanline      */
    CommandWord	    ones = sfbStippleAll1;
    CommandWord	    mask, leftMask, rightMask;
    int		    m;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

/*
The trick here is to take advantage of stipple patterns that can be replicated
to a width of SFBBUSBITS bits.  Then we can rotate each row of the stipple by a
good amount, and always use the rotated bits as is.

For transparent stipples and tiles, we use 0 bits at the left and right ragged
edges.  For opaque stipples, we us the pixel mask register on the SFB.

We also use this code for tiles that are (or are replicated) to SFBBUSBITS
wide.  In this case we load the tile pixels into the foreground register, then
act like we're filling a solid area.

*/

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
    if (nrectFill == 0 || numRects == 0) return;

#ifdef DOTILE
    pstipple   = pGC->tile.pixmap;
#else
    pstipple   = pGC->stipple;
#endif
    psrcBase   = (Bits8 *)(pstipple->devPrivate.ptr);
    heightMask = (pstipple->drawable.height-1) * SFBBUSBYTES;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Both xorg and yorg
       use masking to compute mod, so we don't have to worry about them < 0.
     */

    xorg = pGC->patOrg.x;
    yorg = pGC->patOrg.y;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef DOOPAQUESTIPPLE
    SFBMODE(sfb, OPAQUESTIPPLE);
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
    pdstBase += pDraw->y * dstwidth + pDraw->x * SFBPIXELBYTES;

    for (pbox = REGION_RECTS(prgnClip); numRects != 0; pbox++, numRects--) {
	register int clipx1, clipx2, clipy1, clipy2;
	register int x, x2, y, y2;

	clipx1 = pbox->x1 - pDraw->x;
	clipy1 = pbox->y1 - pDraw->y;
	clipx2 = pbox->x2 - pDraw->x;
	clipy2 = pbox->y2 - pDraw->y;

	prect = prectBase;
	nrect = nrectFill;
	do { /* while nrect != 0 */
	    int ymul;

	    y = prect->y;
	    ymul = y * dstwidth;
	    x = prect->x;
	    width = prect->width;
	    height = prect->height;
	    x2 = x + width;
	    y2 = y + height;

	    if (x < clipx1 | y < clipy1 | x2 > clipx2 | y2 > clipy2
		| width == 0 | height == 0) {
		/* Ick, we have to clip at least partially */
		if (x < clipx1) x = clipx1;
		if (y < clipy1) {
		    y = clipy1;
		    ymul = y * dstwidth;
		}
		if (x2 > clipx2) x2 = clipx2;
		if (y2 > clipy2) y2 = clipy2;
		if (x >= x2 || y >= y2) {
		    goto COMPLETELY_CLIPPED;
		}
		width = x2 - x;
		height = y2 - y;
	    }
	    /* Compute offsets into stipple. */
	    ix = (x - xorg);
	    iy = (y - yorg) * SFBBUSBYTES;
	    pdst = pdstBase + ymul + x * SFBPIXELBYTES;
	    pdstBase = CYCLE_FB(pdstBase);
	    align = (int) pdst & SFBALIGNMASK;
	    pdst -= align;
	    SFBBYTESTOPIXELS(align);
	    ix -= align;
	    width += align;
	    leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	    rightMask = SFBRIGHTSTIPPLEMASK(width, ones);

	    /* Compute alignment of srcbits */
#ifdef DOTILE
	    ComputeRotateAmounts(ix, ixBUSBITS, ix << 3);
#else
	    ComputeRotateAmounts(ix, ixBUSBITS, ix);
#endif
    
	    if (width <= SFBSTIPPLEBITS) {
		/* One-word mask, one write to sfb. */
		mask = leftMask & rightMask;
		do { /* For each line of height */
		    StartStippleLoop2(psrcBase, iy, heightMask, 
			srcbits, ix, ixBUSBITS);
#ifdef DOTILE
		    SFBFOREGROUND(sfb, srcbits);
		    SFBWRITEONEWORD(sfb, pdst, mask);
#else
		    SFBPARTWRITEONEWORD(sfb, pdst, srcbits, mask);
#endif
		    EndStippleLoop2(pdst, dstwidth, iy, height);
		} while (height != 0);
    
#if (SFBBUSBITS <= SFBSTIPPLEBITS) || defined(DOTILE)
	    } else {
		/* Multi-word mask */
		do { /* For each line of height */
		    StartStippleLoop2(psrcBase, iy, heightMask,
			srcbits, ix, ixBUSBITS);
		    p = pdst;
# ifdef DOTILE
		    SFBFOREGROUND(sfb, srcbits);
		    SFBWRITE(pdst, leftMask);
		    for (m = width - 2*SFBSTIPPLEBITS; m > 0;
			m -= SFBSTIPPLEBITS) {
			p += SFBSTIPPLEBYTESDONE;
			SFBWRITE(p, SFBSTIPPLEALL1);
		    }
		    SFBWRITE(p+SFBSTIPPLEBYTESDONE, rightMask);
# else /* DOLITTLE */
		    SFBPARTWRITE(sfb, p, srcbits, leftMask);
		    SFBPARTWRITECYCLE(p);
		    for (m = width - 2*SFBSTIPPLEBITS; m > 0;
			m -= SFBSTIPPLEBITS) {
			p += SFBSTIPPLEBYTESDONE;
			SFBWRITE(p, srcbits);
		    }
		    SFBPARTWRITECYCLE(p);
		    SFBPARTWRITE(sfb, p+SFBSTIPPLEBYTESDONE, srcbits, 
			rightMask);
# endif
		    EndStippleLoop2(pdst, dstwidth, iy, height);
		} while (height != 0);
    
#else /* SFBBUSBITS > SFBSTIPPLEBITS && !DOTILE, so must break up srcbits. */
    
	    } else if (width <= SFBBUSBITS) {
		/* Multi-word mask, but at most one complete word of srcbits. */
		do { /* For each line of height */
		    StartStippleLoop2(psrcBase, iy, heightMask, 
			srcbits, ix, ixBUSBITS);
		    p = pdst;
		    SFBPARTWRITE(sfb, p, srcbits, leftMask);
		    SFBPARTWRITECYCLE(p);
# if SFBBUSBITS > 2 * SFBSTIPPLEBITS
		    for (m = width - 2*SFBSTIPPLEBITS; m > 0;
			m -= SFBSTIPPLEBITS) {
			srcbits >>= SFBSTIPPLEBITS;
			p += SFBSTIPPLEBYTESDONE;
			SFBWRITE(p, srcbits);
		    }
# endif
		    srcbits >>= SFBSTIPPLEBITS;
		    SFBPARTWRITECYCLE(p);
		    SFBPARTWRITE(sfb, p+SFBSTIPPLEBYTESDONE, srcbits, 
			rightMask);
		    EndStippleLoop2(pdst, dstwidth, iy, height);
		} while (height != 0);
	    } else {
		/* Multi-word mask, multiple repetitions of srcbits. */
		do { /* For each line of height */
		    StartStippleLoop2(psrcBase, iy, heightMask, 
			srcbits, ix, ixBUSBITS);
		    p = pdst;
		    m = width - SFBBUSBITS;
# ifdef DOOPAQUESTIPPLE
		    SFBPIXELMASK(sfb, leftMask);
		    StippleEntireWord(p, srcbits);
		    p += SFBBUSBITS*SFBPIXELBYTES;
		    p = CYCLE_FB(p);
		    m -= SFBBUSBITS;
		    while (m > 0) {
			StippleEntireWord(p, srcbits);
			p += SFBBUSBITS*SFBPIXELBYTES;
			m -= SFBBUSBITS;
		    } 
# else
		    do {
			StippleEntireWord(p, srcbits);
			p += SFBBUSBITS*SFBPIXELBYTES;
			m -= SFBBUSBITS;
		    } while (m > 0);
# endif
# if SFBBUSBITS <= 2 * SFBSTIPPLEBITS
		    m = m + (SFBBUSBITS - SFBSTIPPLEBITS);
		    if (m > 0) {
			SFBWRITE(p, srcbits);
			srcbits >>= SFBSTIPPLEBITS;
			p += SFBSTIPPLEBYTESDONE;
		    }
# else
		    for (m = m + (SFBBUSBITS - SFBSTIPPLEBITS);
			 m > 0; m -= SFBSTIPPLEBITS) {
			SFBWRITE(p, srcbits);
			srcbits >>= SFBSTIPPLEBITS;
			p += SFBSTIPPLEBYTESDONE;
		    }
# endif
		    SFBPARTWRITECYCLE(p);
		    SFBPARTWRITE(sfb, p, srcbits, rightMask);
		    EndStippleLoop2(pdst, dstwidth, iy, height);
		} while (height !=0);
#endif
	    } /* if skinny else fat rectangle */

COMPLETELY_CLIPPED:
	    prect++;
	    nrect--;
	} while (nrect > 0);
    } /* for pbox */
#ifdef DOTILE
    /* Restore foreground register */
    SFBFOREGROUND(sfb, pGC->fgPixel);
#endif
}


