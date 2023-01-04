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
static char *rcsid = "@(#)$RCSfile: ffbfillrct.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:09:41 $";
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
#include "ffbfillrct.h"

/* 
    Filled rectangles.
    Translate and clip rectangles, then call the helper function in the GC.
*/

static int SignBits = 0x80008000;

void ffbPolyFillRect(pDrawable, pGC, nrectFill, prectBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill;		/* number of rectangles to fill */
    register xRectangle	*prectBase;	/* Pointer to first rectangle to fill */
{
    register xRectangle *prect;
    register int	nrects;
    RegionPtr       prgnClip;
    register int    numRects;		/* Number of clipping rectangles */
    BoxPtr	    pextent;
    cfbPrivGC       *gcPriv;
    ffbGCPrivPtr    ffbGCPriv;
    int		    xorg, yorg;
    long	    ul, lr, yx1, yx2, dimension;
    void	    (* pfn) ();
    int		    signbits = SignBits;


    gcPriv = CFBGCPRIV(pGC);
    ffbGCPriv = FFBGCPRIV(pGC);
    pfn = ffbGCPriv->FillArea;
    prgnClip = gcPriv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);
    if (nrectFill == 0 | numRects == 0) return;

    xorg = pDrawable->x;
    yorg = pDrawable->y;

    if (numRects == 1) {
	/* Very fast clipping */
	long clipx1, clipx2, clipy1, clipy2;
	long x1, x2, y1, y2;
	long width, height;

	/* We want to clip our rectangles to fit within the single clip
	   rectangle.  If rectangle already fits, do nothing but offset x, y.
	   If rectangle partially fits, write back clipped data.  If rectangle
	   is completely out of the clip rectangle, then write all rectangles
	   so far, and restart pointers.  This allows us to do the most common
	   case--rectangle completely visible--as fast as possible */
	
	pextent = REGION_RECTS(prgnClip);
	clipx1 = pextent->x1 - xorg;
	clipy1 = pextent->y1 - yorg;
	ul = (clipy1 << 16) | (clipx1 & 0xffff);
	clipx2 = pextent->x2 - xorg;
	clipy2 = pextent->y2 - yorg;
	lr = (clipy2 << 16) | (clipx2 & 0xffff);

	prect = prectBase;
	do { /* while nrectFill != 0 */
	    yx1 = ((int *) prect)[0];
	    dimension = ((int *) prect)[1];
	    yx2 = yx1 + dimension;
	    width = wShort(dimension);
	    height = hShort(dimension);
	    
	    if ((((yx1 - ul) | (lr - yx2) | yx1 | dimension) & signbits) 
		| (width == 0) | (height == 0)) {
		/* Ick, we have to clip at least partially */
		x1 = xShort(yx1);
		y1 = yShort(yx1);
		x2 = x1 + width;
		y2 = y1 + height;
		if (x1 < clipx1) x1 = clipx1;
		if (y1 < clipy1) y1 = clipy1;
		if (x2 > clipx2) x2 = clipx2;
		if (y2 > clipy2) y2 = clipy2;

		if (x1 < x2 && y1 < y2) {
		    ((int *)prect)[0] = (y1 << 16) | x1;
		    ((int *)prect)[1] = ((y2-y1) << 16) | (x2-x1);
		} else {
		    /* Really ick.  This rectangle goes to 0.  Write out all
		    the rectangle we have so far, if any. */
		    nrects = prect - prectBase;
		    if (nrects > 0) {
			(*pfn) (pDrawable, nrects, prectBase, pGC);
		    }
		    prectBase = prect + 1;
		}
	    }
	    prect++;
	    nrectFill--;
	} while (nrectFill != 0);

	/* Now paint the remaining rectangles all at once. */
	nrects = prect - prectBase;
	if (nrects > 0) {
	    (*pfn) (pDrawable, nrects, prectBase, pGC);
	}

    } else { /* Complex region, yuch */
    /* ||| Should redo complex clipping of rectangles to be much more efficient.
    */
	register xRectangle *pClipped;
	xRectangle	    *pClippedBase;
	register int n;		/* spare counter */

	/* pClippedBase points to list of clipped rects, and is constructed
	   anew for EACH rectangle in prect.  Yuch.  Since each rectangle
	   is clipped to 0..numRects rectangles, we never need more
	   boxes than numRects. */
	pClippedBase =
	    (xRectangle *) ALLOCATE_LOCAL(numRects * sizeof(xRectangle));
	if (!pClippedBase) {
	    return;
	}

	prect = prectBase;
	pextent = (*pGC->pScreen->RegionExtents)(prgnClip);
	do {
	    BoxRec box;
	    int	x2, y2, width, height;
	    register BoxPtr pbox;	/* pointer to one region clip box */
    
	    /*
	     * clip the box to the extent of the region --
	     * avoids overflowing shorts and minimizes other
	     * computations
	     */
    
	    prect->x += xorg;
	    prect->y += yorg;
    
	    box.x1 = prect->x;
	    if (box.x1 < pextent->x1)  box.x1 = pextent->x1;
    
	    box.y1 = prect->y;
	    if (box.y1 < pextent->y1)  box.y1 = pextent->y1;
    
	    x2 = (int) prect->x + (int) prect->width;
	    if (x2 > pextent->x2)      x2 = pextent->x2;
	    box.x2 = x2;
    
	    y2 = (int) prect->y + (int) prect->height;
	    if (y2 > pextent->y2)      y2 = pextent->y2;
	    box.y2 = y2;
    
	    prect++;
    
	    if ((box.x1 < x2) && (box.y1 < y2)) {
		/* clip the rectangle to each box in the clip region
		   this is logically equivalent to calling Intersect()
		*/
    
		n = numRects;
		pClipped = pClippedBase;
		pbox = REGION_RECTS(prgnClip);
		while (n--) {
		    pClipped->x = max(box.x1, pbox->x1);
		    pClipped->y = max(box.y1, pbox->y1);
		    width = min(box.x2, pbox->x2) - pClipped->x;
		    height = min(box.y2, pbox->y2) - pClipped->y;
		    pbox++;
    
		    /* see if clipping left anything */
		    if (width > 0 && height > 0) {
			pClipped->width = width;
			pClipped->height = height;
			pClipped->x -= xorg;    /* Totally kludged right now */
			pClipped->y -= yorg;
			pClipped++;
		    }
		}
		(*pfn)(pDrawable, pClipped-pClippedBase, pClippedBase, pGC);
	    }
	    nrectFill--;
	} while (nrectFill != 0);
	DEALLOCATE_LOCAL(pClippedBase);
    }
}

/*
 * HISTORY
 */
