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
static char *rcsid = "@(#)$RCSfile: ffbarc1.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:06:40 $";
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
#include "ffb.h"
#include "ffbarc1.h"
#include "ffbstate.h"

#include "ffbarc1bits.c" /* this is an alias */

/* If line width = 1, fill style solid, look for small circles and paint them
   using precomputed bitmaps. */

extern void miPolyArc();

#define FULLCIRCLE (360 * 64)

void ffbPolyArc1(pDraw, pGC, narcs, parcs)
    register DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc	*arc;
    int		i, diameter;
    BoxRec	box;
    RegionPtr   cclip;
    Pixel8      *addrb;
    int		nlwidth;
    int		xorg, yorg;
    cfbPrivGC   *gcPriv;
    ffbScreenPrivPtr scrPriv;
    FFB		ffb;

    xorg = pDraw->x;
    yorg = pDraw->y;
    DrawableBaseAndWidth(pDraw, addrb, nlwidth);
    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (FFBGCBLOCK(pGC)) {
	FFBMODE(ffb, TRANSPARENTBLOCKSTIPPLE);
	FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
    } else {
	FFBMODE(ffb, TRANSPARENTSTIPPLE);
    }
  
    cclip = gcPriv->pCompositeClip;

    for (arc = parcs, i = narcs; i > 0; arc++, i--) {
	addrb = CYCLE_FB_DOUBLE(addrb);
	diameter = (int)arc->width;
	if (diameter == arc->height && diameter < 32
	 && (arc->angle2 >= FULLCIRCLE || arc->angle2 <= -FULLCIRCLE)) {
	    box.x1 = arc->x + xorg;
	    box.y1 = arc->y + yorg;
	    box.x2 = box.x1 + diameter + 1;
	    box.y2 = box.y1 + diameter + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN) {
		int align, rightShift;
		Pixel8 *pdst, *pdst2;
		CommandWord *psrc;
		CommandWord srcbits, srcbits2;

		pdst = addrb + box.y1 * nlwidth + box.x1 * FFBPIXELBYTES;
		psrc = width1Circles[diameter];
		align = (long)pdst & FFBSTIPPLEALIGNMASK;
		pdst -= align;
		pdst2 = CYCLE_FB(pdst) + diameter*nlwidth;
		FFBBYTESTOPIXELS(align);
		if (diameter + align < FFBSTIPPLEBITS) {
		    while (diameter > 0) {
			srcbits = (psrc[0] << align);
			FFBWRITE(pdst, srcbits);
			FFBWRITE(pdst2, srcbits);
			pdst += nlwidth;
			pdst2 -= nlwidth;
			psrc++;
			diameter -= 2;
		    }
		    if (diameter == 0) {
			FFBWRITE(pdst, psrc[0] << align);
		    }
		} else {
		    rightShift = FFBSTIPPLEBITS - align;
		    while (diameter > 0) {
			srcbits = psrc[0];
			srcbits2 = srcbits >> rightShift;
			srcbits <<= align;
			FFBWRITE(pdst, srcbits);
			FFBWRITE(pdst+FFBSTIPPLEBYTESDONE, srcbits2);
			FFBWRITE(pdst2, srcbits);
			FFBWRITE(pdst2+FFBSTIPPLEBYTESDONE, srcbits2);
			pdst += nlwidth;
			pdst2 -= nlwidth;
			psrc++;
			diameter -= 2;
		    }
		    if (diameter == 0) {
			srcbits = psrc[0];
			FFBWRITE(pdst, srcbits << align);
			FFBWRITE(pdst+FFBSTIPPLEBYTESDONE,
			    srcbits >> rightShift);
		    }
		}
		continue;
	    } /* end if use stipple code */
	}
	miPolyArc(pDraw, pGC, i, arc);
	return;
    } /* end for */
}

/*
 * HISTORY
 */
/* $XConsortium: cfbzerarc.c,v 5.16 89/11/25 15:22:46 rws Exp $ */
