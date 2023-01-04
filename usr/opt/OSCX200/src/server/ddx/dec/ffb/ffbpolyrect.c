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
static char *rcsid = "@(#)$RCSfile: ffbpolyrect.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:15:47 $";
#endif
/*
 */
#include "X.h"
#include "Xprotostr.h"
#include "pixmapstr.h"
#include "gcstruct.h"       
#include "windowstr.h"
#include "miscstruct.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "ffb.h"
#include "ffbfill.h"
#include "ffbpolyrect.h"

static int SignBits = 0x80008000;

void ffbPolyRectangle(pDraw, pGC, nrects, prectBase)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		nrects;			/* number of rectangles  */
    xRectangle	*prectBase;		/* Pointer to first rectangle  */
{
    cfbPrivGC	*gcPriv;
    ffbGCPrivPtr ffbGCPriv;
    RegionPtr	prgnClip;
    int 	numRects;
    int		xorg,yorg;
    BoxPtr	pClipRect;
    xRectangle  *prect;
    Pixel8	*addrlBase;
    Pixel8	*pdst;
    int		nlwidth;
    ffbScreenPrivPtr scrPriv;
    FFB		ffb;
    long	yx1, yx2, dimension, ul, lr, y, x, width, height;
    long        clipx1, clipx2, clipy1, clipy2;
    int		ymul,ymul2;
    int		i;  
    int		fillmode;
    long	maxPixels;
    int		signbits = SignBits;
    DDXPointRec rectangle[5];
    long	dy;
    long	data;
    unsigned long ones = ~0; 

    gcPriv = CFBGCPRIV(pGC);
    ffbGCPriv = FFBGCPRIV(pGC);
    prgnClip = gcPriv->pCompositeClip;
    prect = prectBase;

    numRects = REGION_NUM_RECTS(prgnClip);
    if (nrects == 0 | numRects == 0) return;

    if (numRects == 1) {
	/* Very fast clipping */
	BoxRec tbox;		/* Translated clip rectangle */

	xorg = pDraw->x;
	yorg = pDraw->y; 
	pClipRect = REGION_RECTS(prgnClip);
	tbox.x1 = clipx1 = pClipRect->x1 - xorg;
	tbox.y1 = clipy1 = pClipRect->y1 - yorg;
	ul = (clipy1 << 16) | (clipx1 & 0xffff);
	tbox.x2 = clipx2 = pClipRect->x2 - xorg;
	tbox.y2 = clipy2 = pClipRect->y2 - yorg;
	lr = (clipy2-1 << 16) | (clipx2-1 & 0xffff);

	DrawableBaseAndWidth(pDraw, addrlBase, nlwidth);
	addrlBase += yorg * nlwidth + xorg * FFBPIXELBYTES;

        WRITE_MEMORY_BARRIER();
        CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
	/* last ffb register written: planemask */
 	FFBFILLMODE(pDraw, pGC, TRUE, maxPixels,/**/,/**/,fillmode=ffbFillMode);
	FFBLOADCOLORREGS(ffb, gcPriv->xor, pDraw->depth);
        FFBDATA(ffb, ~0);
	FFBBRESWIDTH(ffb, nlwidth/FFBPIXELBYTES);

	do { /* while nrects != 0 */
	    yx1 = ((int *) prect)[0];
	    dimension = ((int *) prect)[1];
	    yx2 = yx1 + dimension;
	    width = wShort(dimension);
	    height = hShort(dimension);

	    if ((((yx1 - ul) | (lr - yx2) | yx1 | dimension) & signbits)
		| (width == 0) | (height == 0)) {
		/* Yuck, let's be sleazy and let line code do clipping */
		x = prect->x;
		y = prect->y;
		rectangle[0].x = x;
		rectangle[0].y = y;
		rectangle[1].x = x + width;
		rectangle[1].y = y;
		rectangle[2].x = x + width;
		rectangle[2].y = y + height;
		rectangle[3].x = x;
		rectangle[3].y = y + height;
		rectangle[4].x = x;
		rectangle[4].y = y;
		CYCLE_REGS(ffb);
		FFBMODE(ffb, TRANSPARENTLINE);
		ffbLineS1(ffb, addrlBase, nlwidth, rectangle, 5-1, &tbox,
		    fillmode, maxPixels);
		WRITE_MEMORY_BARRIER();

	    } else {
		/* We can optimize because the rectangle fits within the
		   clip rect. So we're going to do the horizontal lines
		   and then the vertical lines. This saves us from 
		   changing modes as often as if we did horiz, vert, horiz,
		   vert. */
		y = yShort(yx1);
		ymul = y * nlwidth;
		CYCLE_REGS(ffb);
	        FFBMODE(ffb, fillmode);

		x = xShort(yx1);
		addrlBase = CYCLE_FB(addrlBase);
		pdst = addrlBase + ymul + x * FFBPIXELBYTES; 
		/* prime the multiplication pump by starting the next * before 
		   calling SOLIDSPAN */
		ymul2 = height * nlwidth;
		FFBSOLIDSPAN(ffb, pdst, width + 1, maxPixels);
		FFBSOLIDSPAN(ffb, pdst + ymul2, width + 1, maxPixels);

		dy = height - 1;
		if (dy > 0) {
		    CYCLE_REGS(ffb);
		    FFBMODE(ffb, TRANSPARENTLINE);
		    pdst += nlwidth; 
		    FFBADDRESS(ffb, pdst);
		    data = FFBLINEDXDY(0, dy);
		    FFBSLP3(ffb, data);
		    dy -= FFBLINEBITS;
		    while (dy > 0) {
			CYCLE_REGS(ffb);
			FFBCONTINUE(ffb,ones);
			dy -= FFBLINEBITS;
		    }
		    pdst += width * FFBPIXELBYTES; 
		    CYCLE_REGS(ffb);
		    FFBADDRESS(ffb, pdst);
		    FFBSLP3(ffb, data);
		    dy = height - 1 - FFBLINEBITS;
		    while (dy > 0) {
			CYCLE_REGS(ffb);
			FFBCONTINUE(ffb,ones);
			dy -= FFBLINEBITS;
		    }
		}
	    }
	    prect++;
	    nrects--;
	} while (nrects != 0);
    } else {
	for (i=0; i<nrects; i++) 
	{
	    x = prect->x;
	    y = prect->y;
	    width = prect->width;
	    height = prect->height;
	    rectangle[0].x = x;
	    rectangle[0].y = y;
	    rectangle[1].x = x + width;
	    rectangle[1].y = y;
	    rectangle[2].x = x + width;
	    rectangle[2].y = y + height;
	    rectangle[3].x = x;
	    rectangle[3].y = y + height;
	    rectangle[4].x = x;
	    rectangle[4].y = y;
            (*pGC->ops->Polylines)(pDraw, pGC, CoordModeOrigin, 5, rectangle);
	    prect++;
	}
    }
}

/*
 * HISTORY
 */
