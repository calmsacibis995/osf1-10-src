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
static char *rcsid = "@(#)$RCSfile: ffbfillarc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:09:23 $";
#endif
/*
 */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mifillarc.h"
#include "ffb.h"
#include "ffbfillarc.h"
#include "ffbcirclebits.h"

extern void miPolyFillArc();

#define FILLSPAN(ffb, addr, xl, xr, maxPixels)	    \
{						    \
    register int w_;				    \
    register Pixel8 *pdst_;			    \
    w_ = (xr - xl + 1);				    \
    if (w_ > 0) {				    \
	pdst_ = (addr) + xl * FFBPIXELBYTES;	    \
	FFBSOLIDSPAN(ffb, pdst_, w_, maxPixels);    \
    }						    \
} /* FILLSPAN */

#define FILLSLICESPANS(ffb, flip, addr, maxPixels)  \
{						    \
    if (!flip) {				    \
	FILLSPAN(ffb, addr, xl, xr, maxPixels);     \
    } else {					    \
	xc = xorg - x;				    \
	FILLSPAN(ffb, addr, xc, xr, maxPixels);     \
	xc += slw - 1;				    \
	FILLSPAN(ffb, CYCLE_FB(addr), xl, xc, maxPixels);    \
    }						    \
} /* FILLSLICESPAN */

void
ffbPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr     pDraw;
    GCPtr	    pGC;
    int		    narcs;
    xArc	    *parcs;
{
    register xArc   *arc;
    register int    i;
    BoxRec	    box;
    RegionPtr       cclip;
    Pixel8	    *addrb;
    int		    nlwidth;
    int		    xdraworg, ydraworg;
    cfbPrivGC       *gcPriv;
    int		    rightShift;
    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;
    FFBMode	    fillMode, stippleMode, lastMode;
    long	    maxPixels;

    DrawableBaseAndWidth(pDraw, addrb, nlwidth);
    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    xdraworg = pDraw->x;
    ydraworg = pDraw->y;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,
		{ FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
		  stippleMode = TRANSPARENTBLOCKSTIPPLE; },
		stippleMode = TRANSPARENTSTIPPLE,
		fillMode = ffbFillMode);
    lastMode = COPY;    /* Mode that's never used in this routine */
    FFBDATA(ffb, ~0);

    cclip = gcPriv->pCompositeClip;

    for (arc = parcs, i = narcs; --i >= 0; arc++) {
	if (miFillArcEmpty(arc))
	    continue;
	addrb = CYCLE_FB(addrb);
	if (miCanFillArc(arc)) {
	    box.x1 = arc->x + xdraworg;
	    box.y1 = arc->y + ydraworg;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN) {

		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE)) {
		    int iscircle;
		    int x, y, e, ex;
		    int yk, xk, ym, xm, dx, dy, xorg, yorg;
		    miFillArcRec info;
		    Pixel8 *pdst, *addrlt, *addrlb;
		    register int xpos;
		    register int slw;
		
		    if (arc->width == arc->height && arc->width <= FFBBUSBITS) {
			int j, align;
			Pixel8 *pdst;
			CommandWord *psrc;
			CommandWord srcbits;

			j = arc->width;
			pdst = addrb + box.y1 * nlwidth + box.x1*FFBPIXELBYTES;
			psrc = solidCircles[j];

			/*
			 * drop back into stipple mode for a sec to handle
			 * this case...
			 */
			if (lastMode != stippleMode) {
		            lastMode = stippleMode;
			    CYCLE_REGS(ffb);
			    FFBMODE(ffb, lastMode);
			}
			align = (long)pdst & FFBSTIPPLEALIGNMASK;
			pdst -= align;
			FFBBYTESTOPIXELS(align);
			if (j + align <= FFBSTIPPLEBITS){
			    if (j & 1) {
				FFBWRITE(pdst, psrc[0] << align);
				pdst += nlwidth;
				psrc++;
				j--;
			    }
			    while (j > 0) {
				FFBWRITE(pdst, psrc[0] << align);
				pdst += nlwidth;
				FFBWRITE(pdst, psrc[1] << align);
				pdst += nlwidth;
				psrc += 2;
				j -= 2;
			    }
			} else {
			    rightShift = FFBSTIPPLEBITS - align;
			    while (j > 0) {
				srcbits = *psrc;
				FFBWRITE(pdst, srcbits << align);
				srcbits >>= rightShift;
				FFBWRITE(pdst+FFBSTIPPLEBYTESDONE, srcbits);
				pdst += nlwidth;
				psrc++;
				j--;
			    }
			}
			continue;
		    } /* if small circle we have a table lookup for */

		    if (lastMode != fillMode) {
			lastMode = fillMode;
			CYCLE_REGS(ffb);
			FFBMODE(ffb, lastMode);
		    }
		    addrlt = addrb;
		    miFillArcSetup(arc, &info);
		    MIFILLARCSETUP();
		    xorg += xdraworg;
		    yorg += ydraworg;
		    addrlb = addrlt;
		    addrlt += nlwidth * (yorg - y);
		    addrlb += nlwidth * (yorg + y + dy);
		    iscircle = (arc->width == arc->height);
		    while (y) {
			addrlt += nlwidth;
			addrlb -= nlwidth;
			if (iscircle) {
			    MIFILLCIRCSTEP(slw);
			} else {
			    MIFILLELLSTEP(slw);
			    if (!slw)
				continue;
			}
			xpos = xorg - x;
			pdst = addrlt + xpos * FFBPIXELBYTES;
			FFBSOLIDSPAN(ffb, pdst, slw, maxPixels); 
			if (miFillArcLower(slw)) {
			    pdst = addrlb + xpos * FFBPIXELBYTES;
			    FFBSOLIDSPAN(ffb, pdst, slw, maxPixels);
			}
		    } /* while y */

		} else {
		    /* Partial arc */
		    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
		    register int x, y, e, ex;
		    miFillArcRec info;
		    miArcSliceRec slice;
		    int xl, xr, xc;
		    int iscircle;
		    Pixel8 *addrlb, *addrlt;
		
		    if (lastMode != fillMode) {
			lastMode = fillMode;
			CYCLE_REGS(ffb);
			FFBMODE(ffb, lastMode);
		    }

		    addrlt = addrb;
		    miFillArcSetup(arc, &info);
		    miFillArcSliceSetup(arc, &slice, pGC);
		    MIFILLARCSETUP();
		    iscircle = (arc->width == arc->height);
		    xorg += xdraworg;
		    yorg += ydraworg;
		    addrlb = addrlt;
		    addrlt += nlwidth * (yorg - y);
		    addrlb += nlwidth * (yorg + y + dy);
		    slice.edge1.x += xdraworg;
		    slice.edge2.x += xdraworg;
		    while (y > 0)
		    {
			addrlt += nlwidth;
			addrlb -= nlwidth;
			if (iscircle)
			{
			    MIFILLCIRCSTEP(slw);
			}
			else
			{
			    MIFILLELLSTEP(slw);
			}
			MIARCSLICESTEP(slice.edge1);
			MIARCSLICESTEP(slice.edge2);
			if (miFillSliceUpper(slice))
			{
			    MIARCSLICEUPPER(xl, xr, slice, slw);
			    FILLSLICESPANS(ffb, slice.flip_top, addrlt, 
				    maxPixels);
			}
			if (miFillSliceLower(slice))
			{
			    MIARCSLICELOWER(xl, xr, slice, slw);
			    FILLSLICESPANS(ffb, slice.flip_bot, addrlb, 
				    maxPixels);
			}
		    }
		} /* if FULLCIRCLE ... else ... */
		/* Arc has been painted by some code above */
		continue;
	    } /* if unclipped arc */
	} /* if miCanFillArc(arc) */
	/* Generate spans, which will leave the Color and Foreground Registers
	   with fg, the Data register with ~0, and the Mode with fillMode. */
	miPolyFillArc(pDraw, pGC, 1, arc);
	WRITE_MEMORY_BARRIER();
	lastMode = fillMode;
    }
}

/*
 * HISTORY
 */
