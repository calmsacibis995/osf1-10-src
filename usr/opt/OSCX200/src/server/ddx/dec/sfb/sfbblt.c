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

#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "sfb.h"
#include "sfbblt.h"
#include "bitblt.h"

#undef SFBBUFFILL
/* This code very carefully has to avoid reading any data from the pixmap that
   doesn't actually exist.  It must also always write 2 extra words of data to
   the sfb buffer to make sure that the residue data gets advanced into the
   copy buffer (unless the mask already says write all 8 words).  The mask is
   always a contiguous group of 1's in the least significant bits, but may be
   completely 0.

   ||| Currently often (50%?) reads more data than needed.  I don't know what
   I'm going to do here eventually, so I've not updated anything in sfb.h. */

# define SFBBUFFILL(sfb, psrc, mask)			\
{							\
    register CommandWord mask_;				\
    register PixelWord a_, b_;				\
    if (mask) {						\
	a_ = ((PixelWord *)(psrc))[0];			\
	if ((mask) & 0xf0) {				\
	    b_ = ((PixelWord *)(psrc))[1];		\
	}						\
	SFBBUFWRITE(sfb, 0, a_, b_);			\
	mask_ = (mask) >> SFBPIXELALIGNMENT;		\
	if (mask_) {					\
	    a_ = ((PixelWord *)(psrc))[2];		\
	    if (mask_ & 0xf0) {				\
		b_ = ((PixelWord *)(psrc))[3];		\
	    }						\
	    SFBBUFWRITE(sfb, 2, a_, b_);		\
	    mask_ >>= SFBPIXELALIGNMENT;		\
	    if (mask_) {				\
		a_ = ((PixelWord *)(psrc))[4];		\
		if (mask & 0xf0) {			\
		    b_ = ((PixelWord *)(psrc))[5];	\
		}					\
		SFBBUFWRITE(sfb, 4, a_, b_);		\
		mask_ >>= SFBPIXELALIGNMENT;		\
		if (mask_) {				\
		    a_ = ((PixelWord *)(psrc))[6];	\
		    if (mask & 0xf0) {			\
			b_ = ((PixelWord *)(psrc))[7];	\
		    }					\
		}					\
		SFBBUFWRITE(sfb, 6, a_, b_);		\
	    } else {					\
		SFBBUFWRITE(sfb, 4, 0, 0);		\
	    }						\
	} else {					\
	    SFBBUFWRITE(sfb, 2, 0, 0);			\
	}						\
    } else {						\
	SFBBUFWRITE(sfb, 0, 0, 0);			\
    }							\
    CYCLE_REGS(sfb);					\
} /* SFBBUFFILL */

#ifdef ndef
/* Seems to be worse */
# define SFBBUFFILL(sfb, psrc, mask)			\
{							\
    register CommandWord mask_;				\
    register PixelWord a_, b_;				\
    if ((mask) & 0xffff0000) {				\
	/* At least 5 words */				\
	a_ = ((PixelWord *)(psrc))[0];			\
	b_ = ((PixelWord *)(psrc))[1];			\
	SFBBUFWRITE(sfb, 0, a_, b_);			\
	a_ = ((PixelWord *)(psrc))[2];			\
	b_ = ((PixelWord *)(psrc))[3];			\
	SFBBUFWRITE(sfb, 2, a_, b_);			\
	mask_ >>= 2*SFBPIXELALIGNMENT;			\
	a_ = ((PixelWord *)(psrc))[4];			\
	if (mask_ & 0xff00) {				\
	    /* At least 7 words */			\
	    b_ = ((PixelWord *)(psrc))[5];		\
	    SFBBUFWRITE(sfb, 4, a_, b_);		\
	    a_ = ((PixelWord *)(psrc))[6];		\
	    if (mask_ & 0xf000) {			\
		b_= ((PixelWord *)(psrc))[7];		\
	    }						\
	    SFBBUFWRITE(sfb, 6, a_, b_);		\
	} else {					\
	    /* 5 or 6 words */				\
	    if (mask_ & 0xf0) {				\
		b_= ((PixelWord *)(psrc))[5];		\
	    }						\
	    SFBBUFWRITE(sfb, 4, a_, b_);		\
	    SFBBUFWRITE(sfb, 6, 0, 0);			\
	}						\
    } else if ((mask) & 0xff00) {			\
	/* 3 or 4 words */				\
	a_ = ((PixelWord *)(psrc))[0];			\
	b_ = ((PixelWord *)(psrc))[1];			\
	SFBBUFWRITE(sfb, 0, a_, b_);			\
	a_ = ((PixelWord *)(psrc))[2];			\
	if ((mask) & 0xf000) {				\
	    b_ = ((PixelWord *)(psrc))[3];		\
	}						\
	SFBBUFWRITE(sfb, 2, a_, b_);			\
	SFBBUFWRITE(sfb, 4, 0, 0);			\
    } else if (mask) {					\
	/* 1 or 2 words */				\
	a_ = ((PixelWord *)(psrc))[0];			\
	if ((mask) & 0xf0) {				\
	    b_ = ((PixelWord *)(psrc))[1];		\
	}						\
	SFBBUFWRITE(sfb, 0, a_, b_);			\
	SFBBUFWRITE(sfb, 2, 0, 0);			\
    } else {						\
	/* 0 words */					\
	SFBBUFWRITE(sfb, 0, 0, 0);			\
    }							\
} /* SFBBUFFILL */
#endif

#ifdef COMPILEEVERYTHING

/* Sort rectangles and points into the correct order to process them, so as
   to not destroy information if the source and destination overlap.  Returns
   FALSE if allocations fail. */
Bool SortRectsAndPoints(ppboxInit, ppptInit, nbox, xdir, ydir)
    BoxPtr		    *ppboxInit;
    DDXPointPtr		    *ppptInit;
    register int	    nbox;
    int			    xdir; /* 0: left to right, 1: right to left */
    int			    ydir; /* 0: top to bottom, 1: bottom to top */
{
    register BoxPtr	    pboxInit;
    register DDXPointPtr    pptInit;
    register BoxPtr	    pboxSorted;
    register DDXPointPtr    pptSorted;
    register int	    i, j, k, oldj, y1;

    /* Get pointers to existing data */
    pboxInit = *ppboxInit;
    pptInit  = *ppptInit;

    /* Create space for sorted data */
    pboxSorted = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
    if (!pboxSorted)
	return FALSE;
    pptSorted = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
    if (!pptSorted)
    {
	DEALLOCATE_LOCAL(pboxSorted);
	return FALSE;
    }
    *ppboxInit = pboxSorted;
    *ppptInit = pptSorted;

    if (ydir) {
        /* Walk source botttom to top */
	if (xdir) {
	    /* Bottom to top, and right to left.  Just reverse lists. */
	    for (i = 0; nbox != 0; /* increments in loop */) {
		nbox--;
		pboxSorted[i] = pboxInit[nbox];
		pptSorted [i] = pptInit [nbox];
		i++;
	    }
	} else {
	    /* Bottom to top, and left to right.  Reverse bands while keeping
	       order inside bands the same. */
	    for (i = 0, j = nbox-1; j >= 0; /* increments in loop */) {
		/* Find beginning of j band */
		y1 = pboxInit[j].y1;
		oldj = j;
		do {
		    j--;
		} while (j >= 0 && pboxInit[j].y1 == y1);
		/* Now copy the band */
		k = j;
		do {
		    k++;
		    pboxSorted[i] = pboxInit[k];
		    pptSorted[i] = pptInit[k];
		    i++;
		} while (k != oldj);
	    } /* end for */
	}
    } else { /* ydir == 0 */
	/* Top to bottom, but right to left.  (This routine isn't called if
	   top to bottom, left to right, so we don't have to worry about that.)
	   Reverse rectangle order inside bands, but maintain band order. */
	for (i = 0, j = 0; i != nbox; /* increments in loop */) {
	    /* Find end of j band */
	    y1 = pboxInit[i].y1;
	    oldj = j;
	    do {
		j++;
	    } while (j < nbox && pboxInit[j].y1 == y1);
	    /* Now reverse copy the band */
	    k = j;
	    do {
		k--;
		pboxSorted[i] = pboxInit[k];
		pptSorted[i] = pptInit[k];
		i++;
	    } while (k != oldj);
	} /* end for */
    } /* end if bottom to top else top to bottom */
    return TRUE;
} /* SortRectsAndPoints */


/*
   In the copies below, we may need to read more data from the source
   than we write to the destination, in order to get the pump primed at
   the beginning of the copy, or to get the pump drained at the end of
   the copy.  We do this by (1) alway using a starting mask that includes
   the first word of the source, even though the destination mask says not
   to write the first word, and by (2) using an ending mask based upon the
   aligned destination width, which may be a little longer than what we'd
   get computing the mask based upon the source.
 */

void sfbBitbltScrScr(pSrcDraw, pDstDraw, prgnDst, pptSrc)
    DrawablePtr	pSrcDraw;
    DrawablePtr	pDstDraw;
    RegionPtr	prgnDst;
    DDXPointPtr pptSrc;
{
    int			xdir;		/* 0: left to right, 1: right to left */
    int			ydir;		/* 0: top to bottom, 1: bottom to top */
    register int	dstAlign;	/* Last few bits of destination ptr */
    register int	srcAlign;       /* last few bits of source ptr      */
    register int	shift;		/* Mostly dstAlign-srcAlign	    */
    register Pixel8	*psrc;		/* pointer to current src longword  */
    register Pixel8	*pdst;		/* pointer to current dst longword  */
    register int	width, w;	/* width to blt			    */
    register int	h;		/* height to blt		    */
    register BoxPtr	pbox;		/* current box to blt to	    */
    BoxPtr		pboxInit;       /* starting box of region	    */
    int 		nbox;

    Pixel8		*psrcBase, *pdstBase;	/* start of src, dst	    */
    int			 widthSrc, widthDst;	/* add to get to same position
    						   in next line		    */
    DDXPointPtr		ppt;			/* source location	    */
    Bool		careful;		/* Use sorted rects?	    */
    Pixel8		*psrcLine;		/* Current source scanline  */
    Pixel8		*pdstLine; 		/* Current dest scanline    */
    CommandWord		ones = sfbCopyAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    volatile SFB	sfb;
    
    /* Have to be careful about overlapping copies if pDstDraw = pSrcDraw, or
       if both are windows. */
    DrawableBaseAndWidthPlus(pSrcDraw, psrcBase, widthSrc,
	careful = TRUE  /* if window */,
	careful = FALSE /* if pixmap */);

    DrawableBaseAndWidthPlus(pDstDraw, pdstBase, widthDst,
	/* Nothing */   /* if window */,
	careful = FALSE /* if pixmap */);

    if (pSrcDraw == pDstDraw) careful = TRUE;

    pboxInit = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    if (nbox == 0) return;

    /* Decide what direction to do copies in, so as not to lose data if the
       source and destination overlap. */
    xdir = 0;
    ydir = 0;
    if (careful) {
	xdir = (pptSrc->x < pboxInit->x1);
	ydir = (pptSrc->y < pboxInit->y1);
	if (nbox == 1  ||  (xdir | ydir) == 0) {
	    /* Process rectangle(s) in existing order. */
	    careful = FALSE;
	} else {
	    /* Yuck, gotta rearrange them. */
	    if (!SortRectsAndPoints(&pboxInit, &pptSrc, nbox, xdir, ydir))
		return;
	}
	if (ydir != 0) {
	    /* Walk source bottom to top */
	    widthSrc = -widthSrc;
	    widthDst = -widthDst;
	}
    }

    sfb = SFBSCREENPRIV(pDstDraw->pScreen)->sfb;
    SFBMODE(sfb, COPY);

    pbox = pboxInit;
    ppt = pptSrc;

    /* 
     * Put pdst and psrc one cycle off.
     */
    psrcBase = CYCLE_FB(psrcBase); 
    do { 
	width = pbox->x2 - pbox->x1; 
	h = pbox->y2 - pbox->y1; 
	psrcBase = CYCLE_FB(psrcBase);
	pdstBase = CYCLE_FB(pdstBase);
	if (ydir == 0) { 
	    psrcLine = psrcBase + (ppt->y * widthSrc); 
	    pdstLine = pdstBase + (pbox->y1 * widthDst); 
	} else { 
	    /* we negated widthSrc and widthDst earlier */
	    psrcLine = psrcBase - ((ppt->y+h-1) * widthSrc); 
	    pdstLine = pdstBase - ((pbox->y2-1) * widthDst); 
	} 

	if (xdir == 0) { 
	    /* Forward copy */
	    psrcLine += ppt->x * SFBPIXELBYTES; 
	    pdstLine += pbox->x1 * SFBPIXELBYTES; 
	    srcAlign = (int)psrcLine & SFBALIGNMASK;
	    dstAlign = (int)pdstLine & SFBALIGNMASK;
	    shift = dstAlign - srcAlign;
	    if (shift < 0) {
		/* Ooops.  First source word has less data in it than we need
		   to write to destination, so first word written to internal
		   sfb copy buffer will be junk that just primes the pump.
		   Adjust shift and dstAlign to reflect this fact. */
		shift += SFBALIGNMENT;
		dstAlign += SFBALIGNMENT;
	    }
	    SFBSHIFT(sfb, shift);
	    psrcLine -= srcAlign;
	    pdstLine -= dstAlign;
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
		do {
		    SFBWRITE(psrcLine, rightMask);
		    SFBWRITE(pdstLine, mask);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    h--;
		} while (h != 0);
	    } else {
		/* Mask requires multiple words */
		do {
		    psrc = psrcLine;
		    pdst = pdstLine;
		    SFBWRITE(psrc, ones);
		    SFBWRITE(pdst, leftMask);
		    for (m = width - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
			psrc += SFBCOPYBYTESDONE;
			pdst += SFBCOPYBYTESDONE;
			SFBWRITE(psrc, ones);
			SFBWRITE(pdst, ones);
		    }
		    WBFLUSH();
		    SFBWRITE(psrc+SFBCOPYBYTESDONE, rightMask);
		    SFBWRITE(pdst+SFBCOPYBYTESDONE, rightMask);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    /* Note: since we may be drawing up to the end
		     * of a scan line (width ~= widthsomething)
		     * the first address at the top of the loop
		     * might be within ALPHA_WBUF_BYTES of the last
		     * address since pixmaps are only padded to 8 bytes.
		     */
		    psrcLine = CYCLE_FB(psrcLine);
		    pdstLine = CYCLE_FB(pdstLine);
		    h--;
		} while (h != 0);
	    } /* if small copy else big copy */

	} else {
	    /* Backward copy */
	    psrcLine += (ppt->x + width - 1) * SFBPIXELBYTES;
	    pdstLine += (pbox->x2 - 1) * SFBPIXELBYTES;
	    srcAlign = (int)psrcLine & SFBALIGNMASK;
	    dstAlign = (int)pdstLine & SFBALIGNMASK;
	    shift = dstAlign - srcAlign;
	    if (shift >= 0) {
		/* Ooops.  First source word has less data in it than we 
		   need to write to destination, so first word written to
		   internal sfb copy buffer will be junk that just primes
		   the pump.  Adjust shift and dstAlign to reflect this
		   fact.  (Note that if shift == 0, we execute this code,
		   which results in a wasted read of 8 bytes, but that's
		   by far the easiest thing to do here.)  */
		shift -= SFBALIGNMENT;
		dstAlign -= SFBALIGNMENT;
	    }
	    SFBSHIFT(sfb, shift);
	    psrcLine -= srcAlign;
	    pdstLine -= dstAlign;
	    SFBBYTESTOPIXELS(dstAlign);
	    width += SFBPIXELALIGNMASK - dstAlign;
	    rightMask = SFBBACKRIGHTCOPYMASK(dstAlign);
	    leftMask = SFBBACKLEFTCOPYMASK(width);
	    if (width <= SFBCOPYBITS) {
		/* The mask fits into a single word */
		mask = leftMask & rightMask;
		/* Can't use a mask that only involves a single memory access,
		   because it will trigger a race condition in copy logic. */
		if (dstAlign >= 0) leftMask |= SFBRACECOPYMASK;
		else leftMask |= (SFBRACECOPYMASK << SFBALIGNMENT);
		do {
		    SFBWRITE(psrcLine, leftMask);
		    SFBWRITE(pdstLine, mask);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    h--;
		} while (h != 0);
	    } else {
		/* Mask requires multiple words */
		do {
		    psrc = psrcLine;
		    pdst = pdstLine;
		    SFBWRITE(psrc, ones);
		    SFBWRITE(pdst, rightMask);
		    for (m = width - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
			psrc -= SFBCOPYBYTESDONE;
			pdst -= SFBCOPYBYTESDONE;
			SFBWRITE(psrc, ones);
			SFBWRITE(pdst, ones);
		    }
		    WBFLUSH();
		    SFBWRITE(psrc - SFBCOPYBYTESDONE, leftMask);
		    SFBWRITE(pdst - SFBCOPYBYTESDONE, leftMask);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    psrcLine = CYCLE_FB(psrcLine);
		    pdstLine = CYCLE_FB(pdstLine);
		    h--;
		} while (h != 0);
	    } /* if small copy else big copy */
	} /* end if forward copy else backward copy */

	pbox++;
	ppt++;
	nbox--;
    } while (nbox != 0);
    if (careful) {
	xfree(pboxInit);
	xfree(pptSrc);
    }
}


void sfbBitbltMemScr(pSrcDraw, pDstDraw, prgnDst, ppt)
    DrawablePtr	pSrcDraw;
    DrawablePtr	pDstDraw;
    RegionPtr	prgnDst;
    register DDXPointPtr ppt;
{
    register int	dstAlign;	/* Last few bits of destination ptr */
    register int	srcAlign;       /* last few bits of source ptr      */
    register int	shift;		/* Mostly dstAlign-srcAlign	    */
    register Pixel8	*psrc;		/* pointer to current src longword  */
    register Pixel8	*pdst;		/* pointer to current dst longword  */
    register int	width, w;	/* width to blt			    */
    register int	h;		/* height to blt		    */
    register BoxPtr	pbox;		/* current box to blt to	    */
    int 		nbox;
    int			srcx, dstx;

    Pixel8		*psrcBase, *pdstBase;	/* start of src, dst	    */
    int			 widthSrc, widthDst;	/* add to get to same position
    						   in next line		    */
    Pixel8		*psrcLine;		/* Current source scanline  */
    Pixel8		*pdstLine; 		/* Current dest scanline    */
    CommandWord		ones = sfbCopyAll1;
    CommandWord		mask, leftMask, rightMask, srcRightMask;
    int			m;
    SFB			sfb;

    /* We know that the source is memory, and thus a pixmap.  We know that the
       destination is on the screen.  So we know that source and destination
       can't overlap. */

    sfb = SFBSCREENPRIV(pDstDraw->pScreen)->sfb;

    psrcBase = (Pixel8 *)(((PixmapPtr)pSrcDraw)->devPrivate.ptr);
    widthSrc = (int)(((PixmapPtr)pSrcDraw)->devKind);
	
    DrawableBaseAndWidth(pDstDraw, pdstBase, widthDst);

    nbox = REGION_NUM_RECTS(prgnDst);
    if (nbox == 0) return;
    pbox = REGION_RECTS(prgnDst);

    srcx = ppt->x * SFBSLEAZEPIXELBYTES;
    dstx = pbox->x1 * SFBPIXELBYTES;

#ifndef SOFTWARE_MODEL
#if defined(mips) 
    if ((srcx ^ dstx) & SFBBUSBYTESMASK) {
#endif
#endif
	/* Unaligned copy: use copy logic on sfb */
	SFBMODE(sfb, COPY);
	do { 
	    width = pbox->x2 - pbox->x1;

	    h = pbox->y2 - pbox->y1; 
	    srcx = ppt->x * SFBSLEAZEPIXELBYTES;
	    psrcLine = psrcBase + (ppt->y * widthSrc) + srcx;
	    srcAlign = srcx & SFBSLEAZEALIGNMASK;
	    dstx = pbox->x1 * SFBPIXELBYTES;
	    pdstLine = pdstBase + (pbox->y1 * widthDst) + dstx; 
    	    pdstBase = CYCLE_FB(pdstBase);
	    dstAlign = dstx & SFBALIGNMASK;

	    shift = dstAlign - srcAlign*SFBSLEAZEMULTIPLIER;
	    if (shift < 0) {
		/* Ooops.  First source word has less data in it than we need
		   to write to destination, so first word written to internal
		   sfb copy buffer will be junk that just primes the pump.
		   Adjust shift and dstAlign to reflect this fact. */
		shift += SFBALIGNMENT;
		dstAlign += SFBALIGNMENT;
	    }
	    SFBSHIFT(sfb, shift);
	    psrcLine -= srcAlign;
	    pdstLine -= dstAlign;
	    SFBBYTESTOPIXELS(dstAlign);
	    width += dstAlign;
	    leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
	    rightMask = SFBRIGHTCOPYMASK(width, ones);
	    srcRightMask = rightMask >> shift;
	    if (width <= SFBCOPYBITS) {
		/* The mask fits into a single word */
		mask = leftMask & rightMask;
		do {
		    /* Read source words and stuff them into sfb buffer */
		    SFBBUFFILL(sfb, psrcLine, srcRightMask);
		    SFBWRITE(pdstLine, mask);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    h--;
		} while (h != 0);
	    } else {
		/* Mask requires multiple words */
		do {
		    psrc = psrcLine;
		    pdst = pdstLine;
		    SFBBUFFILLALL(sfb, psrc);
		    SFBWRITE(pdst, leftMask);
		    for (m = width - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
			psrc += SFBSLEAZEBYTESDONE;
			pdst += SFBCOPYBYTESDONE;
			SFBBUFFILLALL(sfb, psrc);
			SFBWRITE(pdst, ones);
		    }
		    SFBBUFFILL(sfb, psrc+SFBSLEAZEBYTESDONE, srcRightMask);
		    SFBWRITE(pdst+SFBCOPYBYTESDONE, rightMask);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    pdstLine = CYCLE_FB(pdstLine);
		    h--;
		} while (h != 0);
	    } /* if small copy else big copy */
    
	    pbox++;
	    ppt++;
	    nbox--;
	} while (nbox != 0);
#if defined(mips) && !defined(SOFTWARE_MODEL)
/* ||| Don't know how to effectively deal with ragged edges, bytes, on anything
   but MIPS. */
/* NOTE: this code has not been alphabitized....*/
    } else {
	/* We know that src and dst will always be aligned, though there may be
	   a bit of residue at the left and right ends. */
	SFBMODE(sfb, SIMPLE);
	do { 
	    int		w;
	    PixelWord   sB, sC, sD, sE;	/* source temps */
    
	    width = pbox->x2 - pbox->x1;
	    h = pbox->y2 - pbox->y1; 
	    psrcLine =
		psrcBase + ppt->y * widthSrc + ppt->x * SFBSLEAZEPIXELBYTES; 
	    pdstLine =
		pdstBase + pbox->y1 * widthDst + pbox->x1 * SFBPIXELBYTES; 
	    if (width <= SFBBUSBYTES/SFBPIXELBYTES) {
		/* Narrow strip, do one pixel at a time */
		do {
		    w = width;
		    psrc = psrcLine;
		    pdst = pdstLine;
		    do {
			sB = ((OnePixel *) psrc)[0];
			SFBPIXELWRITE(pdst, sB);
			psrc += SFBPIXELBYTES;
			pdst += SFBPIXELBYTES;
			w -= 1;
		    } while (w != 0);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    h--;
		} while (h != 0);
	    } else {
		int	    cAlign;
    
		srcAlign = (int)psrcLine & SFBBUSBYTESMASK;
		cAlign = SFBBUSBYTES - srcAlign;
		width = width * SFBPIXELBYTES - cAlign;
		do {
		    LoadWordRight(sB, psrcLine);
		    SFBSTOREWORDRIGHT(sB, pdstLine);
		    /* Loops go faster if compare to 0 */
		    psrc = psrcLine + cAlign; 
		    pdst = pdstLine + cAlign;
		    w = width - 4 * SFBBUSBYTES;
		    while (w > 0) {
			sB = ((PixelWord *) psrc)[0];
			sC = ((PixelWord *) psrc)[1];
			sD = ((PixelWord *) psrc)[2];
			sE = ((PixelWord *) psrc)[3]; 
			SFBWRITE(pdst + 0 * SFBBUSBYTES, sB);
			SFBWRITE(pdst + 1 * SFBBUSBYTES, sC);
			SFBWRITE(pdst + 2 * SFBBUSBYTES, sD);
			SFBWRITE(pdst + 3 * SFBBUSBYTES, sE);
			pdst += 4 * SFBBUSBYTES;
			psrc += 4 * SFBBUSBYTES;
			w -= 4 * SFBBUSBYTES;
		    }
		    w += (4 - 1) * SFBBUSBYTES;
		    while (w > 0) {
			sB = ((PixelWord *) psrc)[0];
			SFBWRITE(pdst, sB);
			psrc += SFBBUSBYTES;
			pdst += SFBBUSBYTES;
			w -= SFBBUSBYTES;
		    }
		    LoadWordLeft(sB, psrc + w + 3); 
		    SFBSTOREWORDLEFT(sB, pdst + w + 3);
		    psrcLine += widthSrc;
		    pdstLine += widthDst;
		    h -= 1;
		} while (h != 0);
	    } /* if narrow else wide aligned copy */
	    pbox++;
	    ppt++;
	    nbox--;
	} while (nbox != 0);
    } /* if unaligned else aligned copy */
#endif
}

#endif /* COMPILEEVERYTHING */

#define MYCFBCOPY(src, pdst) \
    CFBCOPY(src, pdst, andbits1, xorbits1, andbits2, xorbits2)

void SFBBITBLTSCRMEM(pSrcDraw, pDstDraw, prgnDst, ppt,
	    andbits1, xorbits1, andbits2, xorbits2)
    DrawablePtr	pSrcDraw;
    DrawablePtr	pDstDraw;
    RegionPtr	prgnDst;
    register DDXPointPtr ppt;
    PixelWord	andbits1;
    PixelWord	xorbits1;
    PixelWord	andbits2;
    PixelWord	xorbits2;
{
    register int	dstAlign;	/* Last few bits of destination ptr */
    register int	srcAlign;       /* last few bits of source ptr      */
    register int	shift;		/* Mostly dstAlign-srcAlign	    */
    register Pixel8	*psrc;		/* pointer to current src longword  */
    register Pixel8	*pdst;		/* pointer to current dst longword  */
    register int	width, w;	/* width to blt			    */
    register int	h;		/* height to blt		    */
    register BoxPtr	pbox;		/* current box to blt to	    */
    int 		nbox;

    Pixel8		*psrcBase, *pdstBase;	/* start of src, dst	    */
    int			 widthSrc, widthDst;	/* add to get to same position
    						   in next line		    */
    Pixel8		*psrcLine;		/* Current source scanline  */
    Pixel8		*pdstLine; 		/* Current dest scanline    */
    CommandWord		ones = sfbCopyAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    SFB			sfb;

    sfb = SFBSCREENPRIV(pSrcDraw->pScreen)->sfb;
    /* ||| Decide if use sfb logic, or just cfb code */

#ifndef SOFTWARE_MODEL

#if LONG_BIT == 64 || defined(VMS)
    /* In the case of LONG_BIT, the cfb code is faster since it
     * blt's in quad words instead of just long words. 
     */
    {
#else 
    if (ws_cpu == DS_5000 ) {
#endif
	/* Barf.  We can't effectively use COPY mode to read from the screen,
	   because the 3MAX hardware performs read-around-write, and so may
	   try to read data from the sfb buffer register BEFORE the write to
	   fill those registers has gone out over the TURBOChannel.  So we'll
	   use cfb code instead.  Mike Nielsen assures me that the 3MAX is the
	   only machine that will do this on the MIPS side of the world; 3MAX+,
	   3min, etc. don't have this problem. */
	SFBFLUSHMODE(sfb, SIMPLE);
#ifdef MITR5	
#if LONG_BIT == 64
        CFBBITBLT(pSrcDraw, pDstDraw, GXcopy, prgnDst, ppt,
            (((unsigned long)andbits2) << 32) | ((unsigned long)andbits2));
#else
        CFBBITBLT(pSrcDraw, pDstDraw, GXcopy, prgnDst, ppt, andbits2);
#endif
#else	/* MITR5 */
	CFBBITBLT(pSrcDraw, pDstDraw, prgnDst, ppt,
	    andbits1, xorbits1, andbits2, xorbits2);
#endif	/* MITR5 */
	return;
    }
    SFBMODE(sfb, COPY);
#endif /* SOFTWARE_MODEL */

    /* NOTE: the following has been tested for alpha osf and works.
     * However, BUFDRAIN needs to be re-written to read quad words from
     * the buffers. Change the size of and and or bits in sfbbitblt.c,
     * have a second definition of read buf which casts the read as
     * a quad word, and use that. Alignment issues need to be dealt
     * with if dst isn't quad word aligned.
     * Since cfb is written to use quad words, we're using that instead.
     */
    /* We know that the destination is memory, and thus a pixmap.  We know that
       the source is on the screen.  So we know that source and destination
       can't overlap. */

    DrawableBaseAndWidth(pSrcDraw, psrcBase, widthSrc);
    pdstBase = (Pixel8 *)(((PixmapPtr)pDstDraw)->devPrivate.ptr);
    widthDst = (int)(((PixmapPtr)pDstDraw)->devKind);

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    if (nbox == 0) return;

    do { 
	width = pbox->x2 - pbox->x1; 
	h = pbox->y2 - pbox->y1; 
	psrcLine = psrcBase + (ppt->y * widthSrc); 
        psrcBase = CYCLE_FB(psrcBase);
	pdstLine = pdstBase + (pbox->y1 * widthDst); 

	psrcLine += ppt->x * SFBPIXELBYTES; 
	pdstLine += pbox->x1 * SFBSLEAZEPIXELBYTES;
	srcAlign = (int)psrcLine & SFBALIGNMASK;
	dstAlign = (int)pdstLine & SFBSLEAZEALIGNMASK;
	shift = dstAlign*SFBSLEAZEMULTIPLIER - srcAlign;
	if (shift < 0) {
	    /* Ooops.  First source word has less data in it than we need
	       to write to destination, so first word written to internal
	       sfb copy buffer will be junk that just primes the pump.
	       Adjust shift and dstAlign to reflect this fact. */
	    shift += SFBALIGNMENT;
	    dstAlign += SFBALIGNMENT/SFBSLEAZEMULTIPLIER;
	}
	SFBSHIFT(sfb, shift);
	psrcLine -= srcAlign;
	pdstLine -= dstAlign;
	SFBBYTESTOPIXELS(dstAlign);
	width += dstAlign;
	leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
	rightMask = SFBRIGHTCOPYMASK(width, ones);
	/* NOTE: SFBBUFDRAIN uses SFBFASTFLUSH which is a WBFLUSH
	 * so no flushing is needed here and no cycling
	 */
	if (width <= SFBCOPYBITS) {
	    /* The mask fits into a single word */
	    mask = leftMask & rightMask;
	    do {
		SFBWRITE(psrcLine, rightMask);
		SFBBUFDRAIN(sfb, pdstLine, mask);
		psrcLine += widthSrc;
		pdstLine += widthDst;
		h--;
	    } while (h != 0);
	} else {
	    /* Mask requires multiple words */
	    do {
		psrc = psrcLine;
		pdst = pdstLine;
		SFBWRITE(psrc, ones);
		SFBBUFDRAIN(sfb, pdst, leftMask);
		for (m = width - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
		    psrc += SFBCOPYBYTESDONE;
		    pdst += SFBSLEAZEBYTESDONE;
		    SFBWRITE(psrc, ones);
		    SFBBUFDRAINALL(sfb, pdst);
		}
		SFBWRITE(psrc+SFBCOPYBYTESDONE, rightMask);
		SFBBUFDRAIN(sfb, pdst+SFBSLEAZEBYTESDONE, rightMask);
		psrcLine += widthSrc;
		pdstLine += widthDst;
		psrcLine = CYCLE_FB(pdstLine);
		h--;
	    } while (h != 0);
	} /* if small copy else big copy */

	pbox++;
	ppt++;
	nbox--;
    } while (nbox != 0);
}

