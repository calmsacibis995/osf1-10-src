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
static char *rcsid = "@(#)$RCSfile: ffbplane.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:12:25 $";
#endif
/*
 */
/*
 * HISTORY
 */

#include "X.h"
#include "Xprotostr.h"

#include "misc.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "regionstr.h"
#include "Xmd.h"
#include "servermd.h"
#include "ffb.h"
#include "ffbpntarea.h"
#include "ffbplane.h"
#include "cfbplane.h"


/* ffbCopyPlane strategy: 
 * First get a 1-bit deep pixmap representing the bits requested 
 * build a source clip
 * Use the bitmap we've built up as an opaque stipple for the destination 
 */

/* ||| Should steal R5 version (with our checks for overflow), esp. if can 
   just CALL clip computations, rather than the cheezy way Keith sets up
   some global variables. */

RegionPtr ffbCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane)
    DrawablePtr 	pSrcDrawable;
    DrawablePtr		pDstDrawable;
    GCPtr		pGC;
    int 		srcx, srcy;
    int 		width, height;
    int 		dstx, dsty;
    Bits32		bitPlane;
{
    PixmapPtr		pPixmap;
    BoxRec 		box;
    RegionPtr		prgnSrc, prgnExposed;
    cfbPrivGC		*gcPriv;
    int			dstxorg, dstyorg;
    FFB			ffb;

    gcPriv    = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    
    /* The comment below does not make sense to me given the code but I'm
       leaving it in in case it may help someone else out.  edg 9/28/90
    */
    /* Compute the source clip with the smallest possible extent.  The source
       clip excludes any area covered by children windows, and any area outside
       the window or pixmap boundaries.  The clip origin should be relative to
       (srcx, srcy), that is, (0, 0) in box represents the point (srcx, srcy).

       If the entire drawable is unclipped, then the computations below result
       in the rectangle:
	    x1 = pSrcDrawable->x + max(0, -srcx);
	    y1 = pSrcDrawable->y + max(0, -srcy);
	    x2 = pSrcDrawable->x
		    + min(width,  (int)pSrcDrawable->width  - srcx);
	    y2 = pSrcDrawable->y
		    + min(height, (int)pSrcDrawable->height - srcy);

       Note that by taking just the extent box of the source clip region,
       rather than paying attention to the entire clip, we may actually
       display shit on the screen, which will eventually be filled with the
       window background once miHandleClips zaps it.  Too bad.  I'm only
       really interested in CopyPlane from a 1-bit pixmap, which always has
       a single rectangle for the clip, and so doesn't have this problem.
*/

    if (pSrcDrawable->type != DRAWABLE_PIXMAP) {
        box.x1 = pSrcDrawable->x + srcx;
        box.y1 = pSrcDrawable->y + srcy;
	box.x2 = box.x1 + width;
        box.y2 = box.y1 + height;
        prgnSrc = (*pGC->pScreen->RegionCreate)(&box, 1);
	/* clip to visible drawable */
	if (pGC->subWindowMode == IncludeInferiors) {
	    RegionPtr prgnSrcClip;
	    prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
	    (*pGC->pScreen->Intersect) (prgnSrc, prgnSrc, prgnSrcClip);
	    (*pGC->pScreen->RegionDestroy) (prgnSrcClip);
	} else {
	    (*pGC->pScreen->Intersect)
		(prgnSrc, prgnSrc, &((WindowPtr)pSrcDrawable)->clipList);
	}
	(*pGC->pScreen->TranslateRegion)(prgnSrc, -box.x1, -box.y1);
	box = *(*pGC->pScreen->RegionExtents)(prgnSrc);
	(*pGC->pScreen->RegionDestroy)(prgnSrc);

/* ||| Suspect all these computation subject to 16-bit overflow.  Get CopyArea
   right, then steal it. */

    } else {
	/* Create box with origin (srcx, srcy) clipped to boundarys of pixmap */
	/* Clip upper left */
	box.x1 = 0;
	box.y1 = 0;
	if (srcx < pSrcDrawable->x)
	    box.x1 = pSrcDrawable->x - srcx;
	if (srcy < pSrcDrawable->y)
	    box.y1 = pSrcDrawable->y - srcy;

	/* Clip lower right */
	box.x2 = width;
        box.y2 = height;
	if (box.x2 > pSrcDrawable->x + pSrcDrawable->width - srcx)
	    box.x2 = pSrcDrawable->x + pSrcDrawable->width - srcx;
	if (box.y2 > pSrcDrawable->y + pSrcDrawable->height - srcy)
	    box.y2 = pSrcDrawable->y + pSrcDrawable->height - srcy;
    }

    if ((box.x2 > box.x1) && (box.y2 > box.y1)) {
	/* Clip to destination region */
	dstxorg = dstx + pDstDrawable->x;
	dstyorg = dsty + pDstDrawable->y;
        prgnSrc = (*pGC->pScreen->RegionCreate)(&box, 1);
	(*pGC->pScreen->TranslateRegion)(prgnSrc, dstxorg, dstyorg);
	(*pGC->pScreen->Intersect)(prgnSrc, prgnSrc, gcPriv->pCompositeClip);

	if (REGION_NUM_RECTS(prgnSrc) != 0) {
	    if (pSrcDrawable->bitsPerPixel == 1) {
		/* We can just use the source pixmap itself as a stipple */
		if (pDstDrawable->bitsPerPixel == 8)
		    ffb8OSPlane(pDstDrawable, pGC, (PixmapPtr) pSrcDrawable,
			      dstx-srcx, dsty-srcy, prgnSrc);
		else
		    ffb32OSPlane(pDstDrawable, pGC, (PixmapPtr) pSrcDrawable,
			      dstx-srcx, dsty-srcy, prgnSrc);
/* ||| Need 8-to-1 plane getter */
#ifdef ndef
	    /* Both 16, 32 bits/pixel support 8 bits/pixel pixmaps. */
	    } else if (pSrcDrawable->bitsPerPixel == 8) {
		/* Copy bits out of source pixmap */
		pPixmap = cfbGetPlane8(pSrcDrawable, ffs(bitPlane) - 1,
			       srcx + box.x1, srcy + box.y1,
			       box.x2 - box.x1, box.y2 - box.y1);
#if FFBBITSPERPIXEL == 16
	    } else if (pSrcDrawable->bitsPerPixel == 16) {
		/* Copy bits out of source pixmap */
		pPixmap = cfbGetPlane16(pSrcDrawable, ffs(bitPlane) - 1,
			       srcx + box.x1, srcy + box.y1,
			       box.x2 - box.x1, box.y2 - box.y1);
#elif FFBBITSPERPIXEL == 32
	    } else if (pSrcDrawable->bitsPerPixel == 32) {
		/* Copy bits out of source pixmap */
		pPixmap = cfbGetPlane32(pSrcDrawable, ffs(bitPlane) - 1,
			       srcx + box.x1, srcy + box.y1,
			       box.x2 - box.x1, box.y2 - box.y1);
#endif
		ffbOSPlane(pDstDrawable, pGC, pPixmap,
			      dstx-srcx, dsty-srcy, prgnSrc);
		(*pGC->pScreen->DestroyPixmap)(pPixmap);
#endif
	    } else {
		/* Have to copy bits out of pixmap the hard way */
		ffb = FFBSCREENPRIV(pDstDrawable->pScreen)->ffb;
		WRITE_MEMORY_BARRIER();
		FFBMODE(ffb, SIMPLE);
		FFBSYNC(ffb);
		pPixmap = cfbGetPlane(pSrcDrawable, FFSS(bitPlane) - 1,
			       srcx + box.x1, srcy + box.y1,
			       box.x2 - box.x1, box.y2 - box.y1);
		if (pDstDrawable->bitsPerPixel == 8)
		    ffb8OSPlane(pDstDrawable, pGC, pPixmap,
			      dstx + box.x1, dsty + box.y1, prgnSrc);
		else
		    ffb32OSPlane(pDstDrawable, pGC, pPixmap,
			      dstx + box.x1, dsty + box.y1, prgnSrc);
		(*pGC->pScreen->DestroyPixmap)(pPixmap);
	    }
	}
	(*pGC->pScreen->RegionDestroy)(prgnSrc);

    }
    prgnExposed = (RegionPtr)NULL;
    if (gcPriv->fExpose)
	prgnExposed = miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
			srcx, srcy, width, height, dstx, dsty, bitPlane);
    return prgnExposed;
}
