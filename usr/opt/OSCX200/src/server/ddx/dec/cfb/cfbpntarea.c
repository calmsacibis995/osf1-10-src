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
/****************************************************************************
**                                                                          *
**                  COPYRIGHT (c) 1988, 1989, 1990 BY                       *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
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

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbpntarea.c,v 1.1.2.2 92/01/07 12:50:49 Jim_Ludwig Exp $ */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"
#include "cfbpntarea.h"
#include "cfbfgbg.h"

/*
Compiled 3 times:

CFBSOLIDFILLAREA	 CFBTSFILLAREA	       CFBTSFILLAREA32         CFBFILL
--------------------------------------------------------------------------------
cfbSolidFillAreaCopy     cfbTSFillAreaCopy     cfbTSFillArea32Copy     DFCOPY   
cfbSolidFillAreaXor      cfbTSFillAreaXor      cfbTSFillArea32Xor      DFXOR
cfbSolidFillAreaGeneral  cfbTSFillAreaGeneral  cfbTSFillArea32General  DFGENERAL

*/

void
CFBSOLIDFILLAREA(pDraw, nrect, prect, fgandbits, fgxorbits, pGC)
    DrawablePtr		    pDraw;
    int			    nrect;
    xRectangle		    *prect;
    register Pixel32	    fgandbits;
    register Pixel32	    fgxorbits;
    GCPtr		    pGC;	/* Only for use in stipples */

{
    register int w;	    /* width of current rect			    */
    register int h;	    /* height of current rect			    */
    register int dstwidth;  /* width of drawable			    */
    register Pixel8 *pdstBase;/* pointer to start pixel of drawable	    */
    register Pixel8 *pdst;  /* pointer to start pixel of row		    */
    register Pixel8 *p;     /* pointer to pixels we're writing		    */
    register int align;	    /* alignment of pdst (0..3)			    */
    register int tw;	    /* temporary width holder			    */

    if (nrect == 0) return;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pdstBase = (Pixel8 *)
		    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    } else {
	pdstBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }

#ifdef COPYINASM
	cfbFillSolidRectCopy(
	    fgandbits, fgxorbits, pdstBase, dstwidth, prect, nrect);
#else
    do { /* Each rectangle */
	pdst = pdstBase + prect->y * dstwidth + prect->x;
	w = prect->width;
	h = prect->height;

	align = prect->x & 3;
	tw = align + w - 4;

	if (tw <= 0) {
	    /* Narrow rectangle, fits inside single destination word */
	    if (tw == 0) {
		/* Fits into high pixels of word */
		do {
		    CFBFILLRIGHT(pdst, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    h--;
		} while (h != 0);

	    } else if (align == 0) {
		/* Fits into low pixels of word, move pdst past highest byte */
		pdst += w;
		pdst -= 1;
		do {
		    CFBFILLLEFT(pdst, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    h--;
		} while (h != 0);

	    } else if (w == 1) {
		/* One byte wide */
		do {
		    CFBFILL(pdst, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    h--;
		} while (h != 0);
	    
	    } else {
		/* w = 2, align = 1 */
		do {
		    CFBFILL(pdst, fgandbits, fgxorbits);
		    CFBFILL(pdst+1, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    h--;
		} while (h != 0);
	    }

	} else {
	    /* At least two (partial) words of dst to write to */
	    int leftpixels = 4 - align;
	    tw -= 4;
	    if (tw <= 0) {
		/* Exactly two (partial) words to write to */
		p = pdst + w - 1;
		do {
		    /* Do left word (1-4 pixels) */
		    CFBFILLRIGHT(pdst, fgandbits, fgxorbits);
		    /* Do right word (1-4 pixels) */
		    CFBFILLLEFT(p, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    p += dstwidth;
		    h--;
		} while (h != 0);
	    } else if (tw <= 4) {
		do {
		    /* Do left alignment (1-4 pixels) */
		    CFBFILLRIGHT(pdst, fgandbits, fgxorbits);
		    /* Do 4 middle pixels */
		    CFBFILL((Pixel32 *) (pdst + leftpixels),
			fgandbits, fgxorbits);
		    /* Do right alignment (1-4 pixels) */
		    CFBFILLLEFT(pdst+w-1, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    h--;
		} while (h != 0);
	    } else {
		do {
		    /* Do left alignment (1-4 pixels) */
		    CFBFILLRIGHT(pdst, fgandbits, fgxorbits);
		    /* Do middle pixels until 1-4 left */
		    p = pdst + leftpixels;
		    w = tw;
		    do {
			CFBFILL((Pixel32 *) p, fgandbits, fgxorbits);
			p += 4;
			w -= 4;
		    } while (w > 0);
		    /* Do right alignment (1-4 pixels) */
		    CFBFILLLEFT(p+w+3, fgandbits, fgxorbits);
		    pdst += dstwidth;
		    h--;
		} while (h != 0);
	    }
	}
	    
        prect++;
	nrect--;
    } while (nrect != 0);
#endif
}


/* 
 * Transparent stipple a list of rects
 */

void
CFBTSFILLAREA(pDraw, nrect, prect, fgandbits, fgxorbits, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    register Pixel32    fgandbits;
    register Pixel32    fgxorbits;
    GCPtr		pGC;
{
    register int	width, w;   /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register Pixel8	*p;	    /* pointer to pixels we're writing      */

    PixmapPtr		pstipple;
    int			stippleWidth; /* Width in bits of data in stipple   */
    int			stippleHeight;/* Height in bits of data in stipple  */
    int			srcWidth;   /* Stipple width in physical words      */
    Bits32		*psrcBase;  /* Pointer to stipple bits		    */
    Bits32		*psrc;      /* Pointer to word in stipple bits      */
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Rectangle offset into stipple 	    */


    if (nrect == 0) return;

    pstipple	    = pGC->stipple;
    psrcBase	    = (Bits32 *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind >> 2;
    stippleWidth    = pstipple->drawable.width;
    stippleHeight   = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       xorg and yorg to be below any possible x and y that we have to paint.
     */

    xorg = pDraw->x + (pGC->patOrg.x % stippleWidth) - stippleWidth;
    yorg = pDraw->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pdstBase = (Pixel8 *)
		    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    } else {
	pdstBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }

    do { /* Each rectangle */
	width = prect->width;
	height = prect->height;
	pdst = pdstBase + prect->y * dstwidth + prect->x;
	ix = (prect->x - xorg) % stippleWidth;
	iy = (prect->y - yorg) % stippleHeight;

	do { /* For each line of height */
	    register int w, tix, x32, xrem32, srcw;
	    register Bits32 srcbits;

	    w = width;
	    p = pdst;
	    tix = ix;
	    x32 = tix & 31;
	    psrc = psrcBase + (iy * srcWidth) + (tix >> 5);
	    srcbits = (*psrc) >> x32;
	    xrem32 = 32-x32;
	    srcw = stippleWidth - tix;

	    for (;;) {
		register Pixel8 *tp;

		if (xrem32 > srcw) {
		    /* Oops, ran out of stipple bits. */
		    xrem32 = srcw;
		    srcbits &= ((1 << xrem32) - 1);
		}
		if (xrem32 > w) {
		    /* ran out of rectangle, so mask out high stipple bits */
		    xrem32 = w;
		    srcbits &= ((1 << xrem32) - 1);
		}
    
		tp = p;
		/* Align srcbits to word boundary */
		while ((int) tp & 3) {
		    if (srcbits & 1) {
			CFBFILL(tp, fgandbits, fgxorbits);
		    }
		    srcbits >>= 1;
		    tp++;
		}
		SplatAllFGBits(tp, srcbits, fgandbits, fgxorbits);
		w -= xrem32;

		if (w == 0) break; /* LOOP TERMINATOR */

		p += xrem32;
		tix += xrem32;
		srcw -= xrem32;

		xrem32 = 32;
		psrc++;
		if (srcw == 0) {
		    tix = 0;
		    srcw = stippleWidth;
		    psrc = psrcBase + (iy * srcWidth);
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



/* 
 * Transparent stipple a list of rects, stipple width == 32
 * (Stipple may have been replicated if power of 2 < 32 bits)
 */

void
CFBTSFILLAREA32(pDraw, nrect, prect, fgandbits, fgxorbits, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    register Pixel32    fgandbits;
    register Pixel32    fgxorbits;
    GCPtr		pGC;
{
    register int	width;      /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register int	align;      /* alignment of pdst (0..3)		    */

    PixmapPtr		pstipple;
    int			stippleHeight;/* Height in bits of data in stipple  */
    Bits32		*psrcBase;  /* Pointer to base of stipple bits      */
    Bits32		*psrc;
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Rectangle offset into stipple 	    */
    register int	ix32;       /* 32 - ix				    */

/*
The trick here is to take advantage of stipple patterns that can be replicated
to a width of 32 bits.  Then we can rotate each row of the stipple by a good
amount, and always use the rotated 32 bits as is.  We solve initial alignment
problems by backing up each row to a word boundary, and substituting 0-3 bits
of 0's at the bottom of the stipple the very first time through the loop.  We
solve ending alignment problems by substituting 0's for the high 1-31 bits of
the stipple.
*/

    if (nrect == 0) return;

    pstipple      = pGC->stipple;
    psrcBase      = (Bits32 *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       yorg to be below any possible y that we have to paint.  xorg uses
       masking to compute mod, so we don't have to worry about it being < 0.
     */

    xorg = pDraw->x + pGC->patOrg.x;
    yorg = pDraw->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;
    if (pDraw->type == DRAWABLE_WINDOW) {
	pdstBase = (Pixel8 *)
	    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    } else {
	pdstBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }

#ifdef COPYINASM
	cfbFillStippledRectCopy(
	    psrcBase, stippleHeight, fgandbits, fgxorbits, xorg, yorg, 
	    pdstBase, dstwidth, prect, nrect);
#else	
    do { /* Each rectangle */
	/* Compute upper left corner.  Offset back so pdst is word-aligned. */
	pdst = pdstBase + prect->y * dstwidth + prect->x;

	width = prect->width;
	height = prect->height;


	/* Compute offsets into stipple. */
	ix = (prect->x - xorg);
	iy = (prect->y - yorg) % stippleHeight;

	align = (int) pdst & 3;
	width += align;
	pdst -= align;
	ix -= align;

	if (width <= 99) {
	    /* Fairly wide rectangle */
    
	    ComputeRotateAmounts(ix, ix32, ix);
	    psrc = psrcBase + iy;
	    do { /* For each line of height */
		register int	    w;
		register Bits32     srcbits, tsrcbits;
		register Pixel8     *p;
    
		w = width;
		p = pdst;
		srcbits = *psrc;
		/* Do one-time alignment of srcbits for entire scan line */
		srcbits = (srcbits << ix32) | (srcbits >> ix);
    
		/* We bumped back p to beginning of word, so make sure bottom
		   bits of word are 0. */
		tsrcbits = (srcbits >> align) << align;
		
		for (;;) {
		    register Pixel8 *tp;
    
		    if (w < 32) {
			/* right end of rectangle, mask out high stipple bits */
			tsrcbits &= ((1 << w) - 1);
		    }
	
		    tp = p;
		    SplatAllFGBits(tp, tsrcbits, fgandbits, fgxorbits);
		    w -= 32;
    
		    if (w <= 0) break; /* LOOP TERMINATOR */
    
		    p += 32;
		    tsrcbits = srcbits;
		} /* end loop */
    
		iy++;
		psrc++;
		if (iy == stippleHeight) {
		    iy = 0;
		    psrc = psrcBase;
		}
		pdst += dstwidth;
		height--;
	    } while (height > 0);

	} else {
	    /* Really wide rectangle.  For each 4 bits, get into proper branch
	       of case statement, then loop punching the appropriate pixels
	       across the dst.  We know we'll loop at least 3 times. */

	    register int    w;
	    register Bits32 srcbits, tsrcbits;
	    register Pixel8 *p, *tp;
	    int		    i, w3;

	    /* Compute slightly different offsets into stipple from above */
	    ComputeRotateAmounts(ix, ix32, ix + 4);

	    w3 = width & 3;
	    width = width - (96+4) - w3;

	    do  { /* For each line of height */
		srcbits = psrcBase[iy];
		iy++;
		if (iy == stippleHeight) {
		    iy = 0;
		}
		/* Do one-time alignment of srcbits for entire scan line */
		srcbits = (srcbits << ix32) | (srcbits >> ix);
    
		/* Ragged left alignment */
		tsrcbits = (srcbits >> (28 + align)) << align;
		Splat4FGBits(pdst, tsrcbits, fgandbits, fgxorbits);

		p = pdst + 4;
		/* Ragged right alignment */
		if (w3) {
		    tsrcbits = srcbits >> (width & 0x1c);
		    tsrcbits &= ((1 << w3) - 1);
		    Splat4FGBits(p+width+96, tsrcbits, fgandbits, fgxorbits);
		}

		/* Now cycle through each 4 bits, splating the pixels as many
		   times as possible.  We know that we splat each 4 bits at
		   least three times. */

#define MYCFBFILL(tp)   CFBFILL(tp, fgandbits, fgxorbits)

#define BitLoop(body)   \
{			\
    body;		\
    tp += 32;		\
    body;		\
    tp += 32;		\
    body;		\
    tp += 32;		\
    while (w != 0) {	\
	body;		\
	tp += 32;	\
	w--;		\
    };			\
} /* BitLoop */

		for (i = 28; i >= 0 && srcbits; i -=4) {
		    w = (width + i) >> 5;
		    tp = p;
		    switch (srcbits & 0xf) {
			case 0:
			    break;
			case 1:
			    BitLoop (MYCFBFILL(tp));
			    break;
			case 2:
			    BitLoop (MYCFBFILL(tp+1));
			    break;
			case 3:
			    BitLoop (MYCFBFILL((short *) tp));
			    break;
			case 4:
			    BitLoop (MYCFBFILL(tp+2));
			    break;
			case 5:
			    BitLoop (MYCFBFILL(tp);
				     MYCFBFILL(tp+2))
			    break;
			case 6:
			    BitLoop (MYCFBFILL(tp+1);
				     MYCFBFILL(tp+2));
			    break;
			case 7:
			    BitLoop (MYCFBFILL((short *) tp);
				     MYCFBFILL(tp+2));
			    break;
			case 8:
			    BitLoop (MYCFBFILL(tp+3));
			    break;
			case 9:
			    BitLoop (MYCFBFILL(tp);
				     MYCFBFILL(tp+3));
			    break;
			case 10:
			    BitLoop (MYCFBFILL(tp+1);
				     MYCFBFILL(tp+3));
			    break;
			case 11:
			    BitLoop (MYCFBFILL((short *) tp);
				     MYCFBFILL(tp+3));
			    break;
			case 12:
			    BitLoop (MYCFBFILL((short *) (tp+2)));
			    break;
			case 13:
			    BitLoop (MYCFBFILL(tp);
				     MYCFBFILL((short *) (tp+2)));
			    break;
			case 14:
			    BitLoop (MYCFBFILL(tp+1);
				     MYCFBFILL((short *) (tp+2)));
			    break;
			case 15:
			    BitLoop (MYCFBFILL((int *) tp));
			    break;
		    } /* switch */

		    p += 4;
		    srcbits >>= 4;
		} /* for i */
		
		pdst += dstwidth;
		height--;
	    } while (height > 0);
	}


	prect++;
	nrect--;
    } while (nrect > 0);
#endif COPYINASM
}

/* 
 * Opaque stipple a list of rects
 */

void
CFBOSFILLAREA(pDraw, nrect, prect, fgandbits, fgxorbits, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    register Pixel32    fgandbits;
    register Pixel32    fgxorbits;
    GCPtr		pGC;
{
    register int	width, w;   /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register Pixel8	*p;	    /* pointer to pixels we're writing      */

    PixmapPtr		pstipple;
    int			stippleWidth; /* Width in bits of data in stipple   */
    int			stippleHeight;/* Height in bits of data in stipple  */
    int			srcWidth;   /* Stipple width in physical words      */
    Bits32		*psrcBase;  /* Pointer to stipple bits		    */
    Bits32		*psrc;      /* Pointer to word in stipple bits      */
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Rectangle offset into stipple 	    */

    register RopBits	*osfgbgmap; /* Map from 4 bits into 4 pixels	    */
    register RopBits    *ropbits;

    if (nrect == 0) return;

    pstipple	    = pGC->stipple;
    psrcBase	    = (Bits32 *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind >> 2;
    stippleWidth    = pstipple->drawable.width;
    stippleHeight   = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       xorg and yorg to be below any possible x and y that we have to paint.
     */

    xorg = pDraw->x + (pGC->patOrg.x % stippleWidth) - stippleWidth;
    yorg = pDraw->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pdstBase = (Pixel8 *)
	    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    } else {
	pdstBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }

#ifdef MITR5
    osfgbgmap = NULL;
printf ("***** Error: cfbGetOSFGBGMap should be obsolete in MIT R5\n");
#else
    osfgbgmap = cfbGetOSFGBGMap(pGC);
#endif	/* MITR5 */
    do { /* Each rectangle */
	width = prect->width;
	height = prect->height;
	pdst = pdstBase + prect->y * dstwidth + prect->x;

	ix = (prect->x - xorg) % stippleWidth;
	iy = (prect->y - yorg) % stippleHeight;

	do { /* For each line of height */
	    register int w, tix, x32, xrem32, srcw;
	    register Bits32 srcbits;
	    register int    align;

	    w = width;
	    p = pdst;
	    tix = ix;
	    x32 = tix & 31;
	    psrc = psrcBase + (iy * srcWidth) + (tix >> 5);
	    srcbits = (*psrc) >> x32;
	    xrem32 = 32-x32;
	    srcw = stippleWidth - tix;

	    for (;;) {
		if (xrem32 > srcw) {
		    /* Oops, ran out of stipple bits. */
		    xrem32 = srcw;
		}
		if (xrem32 > w) {
		    /* Oops, ran out of rectangle */
		    xrem32 = w;
		}
    
		w -= xrem32;
		tix += xrem32;
		srcw -= xrem32;

		ropbits = &osfgbgmap[srcbits & 15];
		align = ((int) p) & 3;
		if (align + xrem32 <= 4) {
		    /* Narrow rectangle, fits inside single destination word */
		    Pixel32 andbits, xorbits;
		    andbits = ropbits->andbits;
		    xorbits = ropbits->xorbits;
		    while (xrem32 > 0) {
			CFBFILL(p, andbits, xorbits);
			andbits >>= 8;
			xorbits >>= 8;
			p++;
			xrem32--;
		    }

		} else {
		    /* Wider than single destination word */

		    /* First align p to word */
		    CFBFILLRIGHT(p, ropbits->andbits, ropbits->xorbits);
		    align = 4 - align;
		    srcbits >>= align;
		    p += align;
		    xrem32 -= align;

		    /* Now write rest of srcbits */
		    if (xrem32 >= 28) {
			FirstFourOSBits(p, 0, srcbits, osfgbgmap);
			NextFourOSBits(p,  4, srcbits, osfgbgmap);
			NextFourOSBits(p,  8, srcbits, osfgbgmap);
			NextFourOSBits(p, 12, srcbits, osfgbgmap);
			NextFourOSBits(p, 16, srcbits, osfgbgmap);
			NextFourOSBits(p, 20, srcbits, osfgbgmap);
			NextFourOSBits(p, 24, srcbits, osfgbgmap);
			p += 28;
			xrem32 &= 3;
		    } else {
			SetupOSBits(srcbits);
			while (xrem32 >= 4) {
			    NextFourOSBits(p, 0, srcbits, osfgbgmap);
			    p += 4;
			    xrem32 -= 4;
			}
		    }
		    if (xrem32 > 0) {
			RaggedRightOSBits(p, xrem32, srcbits, osfgbgmap);
			p += xrem32;
		    }
		}

		if (w == 0) break; /* LOOP TERMINATOR */

		xrem32 = 32;
		psrc++;
		if (srcw == 0) {
		    tix = 0;
		    srcw = stippleWidth;
		    psrc = psrcBase + (iy * srcWidth);
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


/* 
 * Opaque stipple a list of rects, stipple width == 32
 * (Stipple may have been replicated if power of 2 < 32 bits)
 */

void
CFBOSFILLAREA32(pDraw, nrect, prect, fgandbits, fgxorbits, pGC)
    DrawablePtr		pDraw;
    int			nrect;
    xRectangle		*prect;
    register Pixel32    fgandbits;
    register Pixel32    fgxorbits;
    GCPtr		pGC;
{
    register int	width;      /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register int	align;      /* alignment of pdst (0..3)		    */
    register int	align4;     /* 4 - align			    */

    PixmapPtr		pstipple;
    int			stippleHeight;/* Height in bits of data in stipple  */
    Bits32		*psrcBase;  /* Pointer to stipple bits		    */
    int			xorg, yorg; /* Drawable offset into stipple	    */
    int			ix, iy;     /* Rectangle offset into stipple 	    */
    register int	ix32;       /* 32 - ix				    */
    register int	w;	    /* Temp width counter		    */
    register Pixel8     *p;	    /* Temp dest pointer		    */
    Bits32		*psrc;      /* Pointer to stipple bits for scanline */
    register Bits32     srcbits;    /* Actual source bits for scanline      */

    register RopBits	*osfgbgmap; /* Map from 4 bits into 4 pixels	    */
    register RopBits    *ropbits;

/*
The trick here is to take advantage of stipple patterns that can be replicated
to a width of 32 bits.  Then we can rotate each row of the stipple by a good
amount, and always use the rotated 32 bits as is.  We must use CFBFILLRIGHT
at the left edge to align things, and CFBFILLLEFT at the right edge.
*/

    if (nrect == 0) return;

    pstipple      = pGC->stipple;
    psrcBase      = (Bits32 *)(pstipple->devPrivate.ptr);
    stippleHeight = pstipple->drawable.height;

    /* xorg and yorg (mod stipple width and height) are the location in the
       stipple for the upper left corner of the drawable.  Because the C %
       operator is weird about negative dividends, we mathemagically offset
       yorg to be below any possible y that we have to paint.  xorg uses
       masking to compute mod, so we don't have to worry about it being < 0.
     */

    xorg = pDraw->x + pGC->patOrg.x;
    yorg = pDraw->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pdstBase = (Pixel8 *)
	    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    } else {
	pdstBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }

#ifdef MITR5
    osfgbgmap = NULL;
printf ("***** Error: cfbGetOSFGBGMap should be obsolete in MIT R5\n");
#else
    osfgbgmap = cfbGetOSFGBGMap(pGC);
#endif	/* MITR5 */
    
    do { /* Each rectangle */
	/* Compute upper left corner. */
	pdst = pdstBase + prect->y * dstwidth + prect->x;
	align = (int) pdst & 3;
	width = prect->width;
	height = prect->height;
	iy = (prect->y - yorg) % stippleHeight;
	psrc = psrcBase + iy;

	if (align + width <= 4) {
	    /* Very skinny rectangle */
	    ComputeRotateAmounts(ix, ix32, prect->x - xorg);
	    do { /* For each line of height */
		Pixel32 andbits, xorbits;

		srcbits = *psrc;
		/* Align srcbits */
		srcbits = (srcbits << ix32) | (srcbits >> ix);
		ropbits = &osfgbgmap[srcbits & 0xf];
		andbits = ropbits->andbits;
		xorbits = ropbits->xorbits;
		p = pdst;
		w = width;
		do {
		    CFBFILL(p, andbits, xorbits);
		    p++;
		    w--;
		    andbits >>= 8;
		    xorbits >>= 8;
		} while (w > 0);
		iy++;
		psrc++;
		if (iy == stippleHeight) {
		    iy = 0;
		    psrc = psrcBase;
		}
		pdst += dstwidth;
		height--;
	    } while (height > 0);

	} else if (isPMAX || width + align <= 99) {
	    /* Fairly wide rectangle */
	    /* Compute offsets into stipple. */
	    align4 = 4 - align;
	    ComputeRotateAmounts(ix, ix32, prect->x + align4 - xorg);
	    width -= align4;

	    do { /* For each line of height */
		register Bits32     tsrcbits;
    
		srcbits = *psrc;
		/* Do one-time alignment of srcbits for entire scan line */
		srcbits = (srcbits << ix32) | (srcbits >> ix);
    
		/* We bumped up aligned stipple to next word, so write the very
		   highest bits of the stipple for ragged left alignment */
		ropbits = &osfgbgmap[srcbits >> (28 + align)];
		CFBFILLRIGHT(pdst, ropbits->andbits, ropbits->xorbits);
		p = pdst + align4;
    
		/* Now write 32 pixels per loop as long as possible */
		w = width - 32;
		while (w > 0) {
		    tsrcbits = srcbits;
		    FirstFourOSBits(p, 0, tsrcbits, osfgbgmap);
		    NextFourOSBits(p,  4, tsrcbits, osfgbgmap);
		    NextFourOSBits(p,  8, tsrcbits, osfgbgmap);
		    NextFourOSBits(p, 12, tsrcbits, osfgbgmap);
		    NextFourOSBits(p, 16, tsrcbits, osfgbgmap);
		    NextFourOSBits(p, 20, tsrcbits, osfgbgmap);
		    NextFourOSBits(p, 24, tsrcbits, osfgbgmap);
		    LastFourOSBits(p, 28, tsrcbits, osfgbgmap);
		    w -= 32;
		    p += 32;
		}
		w += 32 - 4;
		while (w > 0) {
		    ropbits = &osfgbgmap[srcbits & 0xf];
		    CFBFILL((Pixel32 *) p, ropbits->andbits, ropbits->xorbits);
		    srcbits >>= 4;
		    p += 4;
		    w -= 4;
		}
		/* Write final 1-4 bytes */
		srcbits <<= (-w);
		ropbits = &osfgbgmap[srcbits & 0xf];
		CFBFILLLEFT(p+w+3, ropbits->andbits, ropbits->xorbits);
    
		iy++;
		psrc++;
		if (iy == stippleHeight) {
		    iy = 0;
		    psrc = psrcBase;
		}
		pdst += dstwidth;
		height--;
	    } while (height > 0);

	} else {
	    /* Really wide rectangle.  Get 4 pixels for each four bits, and
	       paint across dst multiple times with those pixels.  We know
	       we write each whole word in main loop at least 3 times. */

	    int     i, w3;
	    Pixel8  *tp;
	    Bits32  tsrcbits;
	    Pixel32 andbits, xorbits;

	    /* Compute offsets into stipple. */
	    align4 = 4 - align;
	    ComputeRotateAmounts(ix, ix32, prect->x + align4 - xorg);
	    width = width - align4 - 96;
	    w3 = width & 3;
	    
	    do { /* For each line of height */
		srcbits = psrcBase[iy];
		/* Do one-time alignment of srcbits for entire scan line */
		srcbits = (srcbits << ix32) | (srcbits >> ix);
    
		/* Ragged left alignment */
		ropbits = &osfgbgmap[srcbits >> (28 + align)];
		CFBFILLRIGHT(pdst, ropbits->andbits, ropbits->xorbits);
    
		p = pdst + align4;
		/* Write  bits of stipple for ragged right alignment */
		if (w3) {
		    tsrcbits = srcbits >> (width & 0x1c);
		    tsrcbits <<= (7 - w3);
		    ropbits =
			(RopBits *) ((Pixel8 *) osfgbgmap + (tsrcbits & 0x78));
		    CFBFILLLEFT(p+width+(96-1),
			ropbits->andbits, ropbits->xorbits);
		}

		/* Now cycle through each 4 bits, painting the pixels as many
		   times as possible.  We know that we paint each 4 bits at
		   least three times. */
		ropbits = &osfgbgmap[srcbits & 0xf];
		andbits = ropbits->andbits;
		xorbits = ropbits->xorbits;
		srcbits >>= 1;
		for (i = 28; i >= 0; i -=4) {
		    CFBFILL((Pixel32 *) p, andbits, xorbits);
		    CFBFILL((Pixel32 *) (p+32),	andbits, xorbits);
		    CFBFILL((Pixel32 *) (p+64),	andbits, xorbits);
		    w = (width + i) >> 5;
		    tp = p+96;
		    while (w != 0) {
			CFBFILL((Pixel32 *) tp, andbits, xorbits);
			tp += 32;
			w--;
		    };
		    p += 4;
		    ropbits =
			(RopBits *) ((Pixel8 *) osfgbgmap + (srcbits & 0x78));
		    andbits = ropbits->andbits;
		    xorbits = ropbits->xorbits;
		    srcbits >>= 4;
		}
		
		iy++;
		if (iy == stippleHeight) {
		    iy = 0;
		}
		pdst += dstwidth;
		height--;
	    } while (height > 0);
	}
	prect++;
	nrect--;
    } while (nrect > 0);
}

/* 
 * Opaque stipple pstipple onto the destination drawable.  Unlike the stippled
 * rectangle cases, we don't have to worry about repeating horizontally and
 * vertically. 
 */

void CFBOSPLANE(pDraw, pGC, pstipple, xorg, yorg, prgn)
    DrawablePtr pDraw;
    GCPtr	pGC;
    PixmapPtr	pstipple;
    int		xorg, yorg;
    RegionPtr   prgn;
{
    int			nbox;
    BoxPtr		pbox;
    Pixel32		savePlanemask; /* Current planemask */
    register int	width, w;   /* width of current rect		    */
    register int	height;     /* height of current rect		    */
    register int	dstwidth;   /* width of drawable		    */
    register Pixel8	*pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8	*pdst;      /* pointer to start pixel of row	    */
    register Pixel8	*p;	    /* pointer to pixels we're writing      */

    int			stippleWidth; /* Width in bits of data in stipple   */
    int			stippleHeight;/* Height in bits of data in stipple  */
    int			srcWidth;   /* Stipple width in physical words      */
    Bits32		*psrcBase;  /* Pointer to stipple bits		    */
    Bits32		*psrc;      /* Pointer to word in stipple bits      */
    int			ix, iy;     /* Rectangle offset into stipple 	    */

    register RopBits	*osfgbgmap; /* Map from 4 bits into 4 pixels	    */
    register RopBits    *ropbits;

    nbox = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);

    psrcBase	    = (Bits32 *)(pstipple->devPrivate.ptr);
    srcWidth	    = pstipple->devKind >> 2;
    stippleWidth    = pstipple->drawable.width;

    xorg += pDraw->x;
    yorg += pDraw->y;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pdstBase = (Pixel8 *)
	    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);
    } else {
	pdstBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	dstwidth = (int)(((PixmapPtr)pDraw)->devKind);
    }

#ifdef MITR5
    osfgbgmap = NULL;
printf ("***** Error: cfbGetOSFGBGMap should be obsolete in MIT R5\n");
#else
    osfgbgmap = cfbGetOSFGBGMap(pGC);
#endif	/* MITR5 */
    savePlanemask = cfbpmaxSetPlaneMask(pGC->planemask,pGC->pScreen);

    do { /* Each rectangle */
	int align, align4;      /* Pixel offsets for left alignment     */
	int ax, ax32;		/* Bit offsets for left alignment       */
	int mx, mx32;		/* Bit offsets for middle words	*/
	register int w;
	register Bits32 purebits, srcbits, *ps;

	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;
	pdst = pdstBase + pbox->y1 * dstwidth + pbox->x1;

	ix = pbox->x1 - xorg;
	iy = pbox->y1 - yorg;
	psrc = psrcBase + (iy * srcWidth) + (ix >> 5);
	align = ((int) pdst) & 3;
	align4 = 4 - align;
	ax = ix & 31;
	ax32 = 32 - ax;
	mx = (ix + align4) & 31;
	mx32 = 32 - mx;

	if (align + width <= 4) {
	    /* Very narrow rectangle, so paint one pixel at a time */
	    do { /* For each line of height */
		/* Get srcbits from one or two words of source bitmap */
		Pixel32 andbits, xorbits;

		srcbits = (*psrc) >> ax;
		if (width > ax32) {
		    srcbits |= (psrc[1] << ax32);
		}
		ropbits = &osfgbgmap[srcbits & 15];
		andbits = ropbits->andbits;
		xorbits = ropbits->xorbits;
		p = pdst;
		w = width;
		do {
		    CFBFILL(p, andbits, xorbits);
		    andbits >>= 8;
		    xorbits >>= 8;
		    p++;
		    w--;
		} while (w != 0);
		psrc += srcWidth;
		pdst += dstwidth;
		height--;
	    } while (height > 0);

    } else {
	    /* Rectangle wider than single destination word */
	    do { /* For each line of height */
    
		p = pdst + align4;
		ps = psrc;
		w = width - align4;
    
		/* Do leftmost 1-4 bits */
		purebits = *ps++;
		srcbits = purebits >> ax;
		if (align4 >= ax32) {
		    /* We're looking for a few (more) good bits.  Note that we
		       fetch purebits even if align4 == ax32.  These bits won't
		       be used here, but keep loop below consistent.  We know
		       it's okay to prefetch purebits because this rectangle
		       is wider than a single destination word. */
		    purebits = *ps++;
		    srcbits |= (purebits << ax32);
		}
		ropbits = &osfgbgmap[srcbits & 15];
		CFBFILLRIGHT(pdst, ropbits->andbits, ropbits->xorbits);

		/* Inner loop of 32 bits per iteration */
		w -= 32;
		while (w > 0) {
		    srcbits = purebits >> mx;
		    /* Get a few more bits, merge with srcbits if needed */
		    /* Since the real w > 32, we know w > mx32 */
		    purebits = *ps++;
		    if (mx != 0) srcbits |= (purebits << mx32);
    
		    /* Now write srcbits in 8 easy steps */
		    FirstFourOSBits(p, 0, srcbits, osfgbgmap);
		    NextFourOSBits(p,  4, srcbits, osfgbgmap);
		    NextFourOSBits(p,  8, srcbits, osfgbgmap);
		    NextFourOSBits(p, 12, srcbits, osfgbgmap);
		    NextFourOSBits(p, 16, srcbits, osfgbgmap);
		    NextFourOSBits(p, 20, srcbits, osfgbgmap);
		    NextFourOSBits(p, 24, srcbits, osfgbgmap);
		    LastFourOSBits(p, 28, srcbits, osfgbgmap);
		    p += 32;
		    w -= 32;
		} /* end while w > 0 */

		/* Final 1-32 bits */
		srcbits = purebits >> mx;
		w += 32;
		if (w > mx32) {
		    /* Get a few more bits, merge with srcbits always needed */
		    purebits = *ps;
		    srcbits |= (purebits << mx32);
		}

		w -= 4;
		if (w > 0) {
		    /* Can do more than 4 bits after main loop */
		    FirstFourOSBits(p, 0, srcbits, osfgbgmap);
		    w -= 4;
		    while (w > 0) {
			p += 4;
			NextFourOSBits(p, 0, srcbits, osfgbgmap);
			w -= 4;
		    };
		    /* Final 1-4 bits */
		    srcbits <<= -w;
		    srcbits &= 0x78;
		    ropbits = (RopBits *)((Pixel8 *)osfgbgmap + srcbits);
		    CFBFILLLEFT(p+w+8-1, ropbits->andbits, ropbits->xorbits);
		} else {
		    /* Only 1-4 bits left after main loop */
		    srcbits <<= (3 - w);
		    srcbits &= 0x78;
		    ropbits = (RopBits *)((Pixel8 *)osfgbgmap + srcbits);
		    CFBFILLLEFT(p+w+4-1, ropbits->andbits, ropbits->xorbits);
		}

		psrc += srcWidth;
		pdst += dstwidth;
		height--;
	    } while (height > 0);
	} /* end if narrow or wide rectangle */
	pbox++;
	nbox--;
    } while (nbox > 0);
    (void) cfbpmaxSetPlaneMask(savePlanemask,pGC->pScreen);
}
