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
static char *rcsid = "@(#)$RCSfile: ffbpolypnt.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:14:29 $";
#endif
/*
 */
#include "X.h"
#include "Xprotostr.h"

#include "pixmapstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "ffbpolypnt.h"

#ifdef FFB_DEPTH_INDEPENDENT
int FFSS(word)
    unsigned word;
{
    int retval = 0;
    if (word != 0) {
	while ((word & 0xff) == 0) {
	    retval += 8;
	    word >>= 8;
	}
	retval++;
	while ((word & 0x1) == 0) {
	    retval++;
	    word >>= 1;
	}
    }
    return retval;
}
#endif


#define u_long unsigned long

void ffbPolyPoint(pDraw, pGC, mode, npt, pptInit)
    DrawablePtr 	pDraw;
    GCPtr 		pGC;
    int 		mode;		/* Origin or Previous */
    int			npt;
    xPoint		*pptInit;
{
    register int	dstwidth;   /* width of drawable		    */
    register int	shift, shift2;
    register Pixel8     *pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8     *pdst;      /* pointer to point pixel 		    */
    cfbPrivGC		*gcPriv;
    RegionPtr		prgnClip;
    BoxPtr		pbox;
    int			nbox;
    register xPoint	*ppt, *pptEnd;
    register u_long	pntul, pntlr;
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    int			signbits = 0x80008000;
#ifdef PARTIALWRITES
    PixelWord		foreground;
#endif

    if (npt == 0) return;

    gcPriv      = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    prgnClip    = gcPriv->pCompositeClip;
    nbox	= REGION_NUM_RECTS(prgnClip);
    if (nbox == 0) return;
    pbox	= REGION_RECTS(prgnClip);

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    /* last ffb register written: planemask */

#ifdef PARTIALWRITES
    FFBMODE(ffb, SIMPLE);
    foreground = pGC->fgPixel;
#else
    FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif

    pdstBase += pDraw->y * dstwidth + pDraw->x * FFBPIXELBYTES;
    pptEnd = pptInit + npt;

    /* make pointlist origin relative */
    if (mode == CoordModePrevious) {
	register int    offsetx = 0;
	register int    offsety = 0;

        ppt = pptInit;
	do {
	    ppt->x = (offsetx += ppt->x);
	    ppt->y = (offsety += ppt->y);
	    ppt++;
	} while (ppt != pptEnd);
    }

    shift = FFSS(dstwidth) - 1;

    /*
     * FB cycling for points: assume that the list of points is not disjoint.
     * As long as there are no conflicts, it doesn't matter which order the
     * points get written.  Do something similar to the clipul/cliplr scheme.
     * Worse case: concentric points, which is 1/2 as bad as doing the no-brainer
     * scheme of cycling FB every point (degenerate rectangle).  Note that cycling
     * FB is not cheap -- it trashes the TLB (trash-loaded bins)... this favors
     * a fast CPU (eg, AXP, where TLB/p.cache is relatively small).
     */

    if ((dstwidth & (dstwidth - 1)) == 0) {
	/* Oh so gross.  We can use a shift instead of a multiply */
	do { /* While clip boxes */
	    register int clipul, cliplr;
	    register int x, y, yx;
    
	    yx     = ((int *)&pDraw->x)[0];
	    yx     -= (yx & 0x8000) << 1;
	    clipul = ((int *)pbox)[0] - yx;
	    cliplr = ((int *)pbox)[1] - yx - 0x00010001;

	    ppt = pptInit;
	    do {
		x = ppt->x;
		y = ppt->y;
		yx = ((int *)ppt)[0];
		y <<= shift;
		if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		    CYCLE_AND_SET_FB(pdstBase);
		    pdst = pdstBase + y + x * FFBPIXELBYTES;
		    FFBPOINT(pdst, foreground);
		}
		ppt++;
	    } while (ppt != pptEnd);
	    pbox++;
	    nbox--;
	} while (nbox > 0);
	return;
    }

    shift2 = dstwidth - (1 << shift);
    if ((shift2 & (shift2 - 1)) == 0) {
	/* Even grosser: We can use two shifts and an add. */
	shift2 = FFSS(shift2) - 1;
	do { /* While clip boxes */
	    register int clipul, cliplr;
	    register int x, y, yx;
    
	    yx     = ((int *)&pDraw->x)[0];
	    yx     -= (yx & 0x8000) << 1;
	    clipul = ((int *)pbox)[0] - yx;
	    cliplr = ((int *)pbox)[1] - yx - 0x00010001;

	    ppt = pptInit;
	    do {
		x = ppt->x;
		y = ppt->y;
		yx = ((int *)ppt)[0];
		if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		    CYCLE_AND_SET_FB(pdstBase);
		    pdst = pdstBase + (y << shift) + (y << shift2) +
				       x * FFBPIXELBYTES;
		    FFBPOINT(pdst, foreground);
		}
		ppt++;
	    } while (ppt != pptEnd);
	    pbox++;
	    nbox--;
	} while (nbox > 0);
	return;
    }

    /* dstwidth not a convenient power of two */
    do { /* While clip boxes */
	register int clipul, cliplr;
	register int yx, ymul;
	long	 x, y;

	yx     = ((int *)&pDraw->x)[0];
	yx     -= (yx & 0x8000) << 1;
	clipul = ((int *)pbox)[0] - yx;
	cliplr = ((int *)pbox)[1] - yx - 0x00010001;

	ppt = pptInit;
	do {
	    yx = ((int *)ppt)[0];
	    y = yx >> 16;
	    ymul = y * dstwidth;
	    ppt++;
	    x = yx & 0xffff;
	    if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		CYCLE_AND_SET_FB(pdstBase);
		pdst = pdstBase + ymul + x * FFBPIXELBYTES;
		FFBPOINT(pdst, foreground);
	    }
	} while (ppt != pptEnd);
	pbox++;
	nbox--;
    } while (nbox > 0);
}

/*
 * HISTORY
 */
/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbpolypnt.c,v 1.1.2.2 1993/11/19 21:14:29 Robert_Lembree Exp $ */

