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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/dec/qdss/qdgc.c,v 1.2 91/12/15 12:42:16 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/***********************************************************
COPYRIGHT 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "X.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mibstore.h"

#include "qd.h"
#include "qdprocs.h"
#include "qdgc.h"

#include "mi.h"

/*
 * The following procedure declarations probably should be in mi.h
 */
extern PixmapPtr mfbCreatePixmap();
extern void miRecolorCursor();
extern void miImageGlyphBlt();

extern void miClearToBackground();
extern void miSaveAreas();
extern Bool miRestoreAreas();
extern void miTranslateBackingStore();

extern void miPolyFillRect();
extern void qdPixFillRect();
extern void miPolyFillArc();

extern RegionPtr miCopyArea();
extern RegionPtr miCopyPlane();
extern void miPolyPoint();
extern void miPutImage();

extern void miMiter();
extern void miNotMiter();
extern void miZeroLine();
extern void miPolySegment();
extern void tlPolySegment();
extern void tlPolySegmentDashed();
extern void tlPolylinesDashed();
extern void miPolyRectangle();
extern void miPolyArc();
extern void miFillPolygon();

extern void miImageText8(), miImageText16();
extern int miPolyText8(), miPolyText16();
extern void  miPolyArc();
extern void  miFillPolyArc();
extern void  qdUnnaturalPushPixels();

extern void qdImageTextKerned();

#include "qdvalidate.h"

extern void qdSetSpansPix();
static GCOps SolidWinOps = {
    tlSolidSpans,
    qdSetSpansWin,
    qdWinPutImage,
    qdCopyAreaWin,
    qdCopyPlane,
    miPolyPoint,
    tlPolylines,
    tlPolySegment,
    miPolyRectangle,
    miZeroPolyArc,
    qdFillPolygon,
    tlSolidRects, /* dst */
    miPolyFillArc,
    tlPolyTextSolid, /* font, dst ? */
    miPolyText16,
    tlImageText, /* font, dst ? */
    miImageText16,
    miImageGlyphBlt,
    qdPolyGlyphBlt,
    qdPushPixels,
    miMiter, /* LineHelper */
    /* devPrivate.val implicitly set to 0 */
};
static GCOps SolidPixOps = {
    qdFSPixSolid,
    qdSetSpansPix,
    qdPixPutImage,
    qdCopyArea,
    qdCopyPlanePix,
    miPolyPoint,
    miZeroLine,
    miPolySegment,
    miPolyRectangle,
    miZeroPolyArc,
    miFillPolygon,
    qdPixFillRect,
    miPolyFillArc,
    qdPolyTextPix,
    miPolyText16,
    qdImageTextPix,
    miImageText16,
    miImageGlyphBlt,
    qdPolyGlyphBlt,
    qdPushPixels,
    miMiter, /* LineHelper */
    /* devPrivate.val implicitly set to 0 */
};

extern Bool qdNaturalSizePixmap();

struct _GC *InstalledGC;

void
qdChangeGC(pGC, pQ, maskQ)
     GCPtr pGC;
     GCInterestPtr pQ;
     BITS32 maskQ;
{
    if (maskQ & GCDashList) {
	/* calculate length of dash list */
	int length = 0;
	register unsigned char *pDash = (unsigned char *)pGC->dash;
	register int i = pGC->numInDashList;
	while (--i >= 0)
	    length += (unsigned int) *pDash++;
	if (pGC->numInDashList & 1)
	    length += length;
	GC_DASH_LENGTH(pGC) = length;
	GC_DASH_PLANE(pGC) = -1;
    }
}

void qdCopyGC();
static GCFuncs qdFuncs = {
    qdValidateGC,
    qdChangeGC,
    qdCopyGC,
    qdDestroyGC,
    qdChangeClip,
    qdDestroyClip,
    qdCopyClip,
};

Bool
qdCreateGC(pGC)
    GCPtr pGC;
{
    QDPrivGCPtr		qdPriv;
    mfbPrivGCPtr	mfbPriv;
    pGC->ops = &SolidWinOps;
    pGC->funcs = &qdFuncs;
    pGC->funcs->devPrivate.ptr = NULL;
    pGC->miTranslate = 0;    /* all qd output routines do window translation */
    pGC->clipOrg.x = pGC->clipOrg.y = 0;
    pGC->clientClip = (pointer)NULL;
    pGC->clientClipType = CT_NONE;
    qdPriv = (QDPrivGCPtr)pGC->devPrivates[qdGCPrivateIndex].ptr;
    mfbPriv = (mfbPrivGCPtr)pGC->devPrivates[mfbGCPrivateIndex].ptr;
    qdPriv->mask = QD_NEWLOGIC;
    qdPriv->ptresult = (unsigned char *) NULL;
    mfbPriv->freeCompClip = FALSE;
    qdPriv->lastDest = DRAWABLE_WINDOW;
    qdPriv->GCstate = VSFullReset;
    qdPriv->dashLength = 8;
    qdPriv->dashPlane = -1; /* invalid plane number */
    return TRUE;
}

void
qdDestroyGC( pGC)
GCPtr	pGC;
{
    QDPrivGCPtr qdPriv = (QDPrivGCPtr)pGC->devPrivates[qdGCPrivateIndex].ptr;
    mfbPrivGCPtr mfbPriv =
	(mfbPrivGCPtr)pGC->devPrivates[mfbGCPrivateIndex].ptr;

    if (qdPriv->ptresult)
	Xfree(qdPriv->ptresult);

    if (mfbPriv->freeCompClip)
	(*pGC->pScreen->RegionDestroy)(mfbPriv->pCompositeClip);

    if (pGC->ops->devPrivate.val)
	Xfree(pGC->ops);
    if (InstalledGC == pGC) InstalledGC = NULL;
}


void
qdDestroyClip( pGC)
    GCPtr	pGC;
{
    if ( pGC->clientClipType == CT_NONE)
	return;
    else if (pGC->clientClipType == CT_PIXMAP)
	qdDestroyPixmap((PixmapPtr)(pGC->clientClip));
    else
	(*pGC->pScreen->RegionDestroy)( (RegionPtr)pGC->clientClip);
    pGC->clientClip = (pointer)NULL;
    pGC->clientClipType = CT_NONE;
}

RegionPtr
qdBitmapToRegion(pPix)
    PixmapPtr pPix;
{
    tlCancelPixmap(pPix);
    return mfbPixmapToRegion(pPix);
}

void
qdChangeClip( pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    qdDestroyClip(pGC);
    if(type == CT_PIXMAP)
    {
	tlCancelPixmap(pvalue);
	pGC->clientClip = (pointer) mfbPixmapToRegion(pvalue);
	(*pGC->pScreen->DestroyPixmap)(pvalue);
    }
    else if (type == CT_REGION)
    {
	pGC->clientClip = pvalue;
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) miRectsToRegion(nrects, pvalue, type);
	Xfree(pvalue);
    }
    pGC->clientClipType = (type != CT_NONE && pGC->clientClip) ? CT_REGION :
								 CT_NONE;
    pGC->stateChanges |= GCClipMask;
}

void
qdCopyGC( pgcSrc, maskQ, pgcDst)
    GCPtr		pgcDst;
    int                 maskQ;
    GCPtr		pgcSrc;
{
}

void
DoNothing()
{
}

int
IntDoNothing()
{
}

/*
 * Install a vector to mi code for polygon filling, so that the set of pixels
 * touched conforms to the protocol specification.
 * The default is to use the fast dragon polygons, violating the protocol
 * specification.
 */
slowPolygons()	/* called from qdScreenInit */
{
    SolidWinOps.FillPolygon = miFillPolygon;
}

/*
 * Clipping conventions
 *	if the drawable is a window
 *	    CT_REGION ==> pCompositeClip really is the composite
 *	    CT_other ==> pCompositeClip is the window clip region
 *	if the drawable is a pixmap
 *	    CT_REGION ==> pCompositeClip is the translated client region
 *		clipped to the pixmap boundary
 *	    CT_other ==> pCompositeClip is the pixmap bounding box
 */
void
qdValidateGC( pGC, changes, pDrawable)
    register GCPtr	pGC;
    Mask		changes; /* this arg should equal pGC->stateChanges */
    DrawablePtr		pDrawable;
{
    mfbPrivGCPtr		devPriv;
    WindowPtr	pWin;
    RegionPtr	pReg;
    DDXPointRec	oldOrg;		/* origin of thing GC was last used with */
    unsigned long	procChanges = 0;	/* for mibstore */
    unsigned long	vdone = 0;	/* vector already validated */
	/* vdone is a field of VECMAX bits indicating validation complete */

    /* throw away offscreen pixmap if it about to be changed. */
    if (pDrawable->type == DRAWABLE_PIXMAP) {
	if (QDPIX_Y((QDPixPtr) pDrawable) == NOTOFFSCREEN)
	    tlConfirmPixmap((QDPixPtr)pDrawable);
	tlSinglePixmap((QDPixPtr)pDrawable);
    }

    oldOrg = pGC->lastWinOrg;

    if (InstalledGC == pGC) InstalledGC = NULL; /* overly conservative */

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr)pDrawable;
    }
    else
    {
	pWin = (WindowPtr)NULL;
    }
    devPriv = ((mfbPrivGCPtr) (pGC->devPrivates[mfbGCPrivateIndex].ptr));

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & 
	    (GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode))
    ||(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))) {
	if (pWin)
	{
	    int freeTmpClip, freeCompClip;
	    RegionPtr pregWin;	/* clip for this window, without client clip */

	    if (pGC->subWindowMode == IncludeInferiors)
	    {
	        pregWin = NotClippedByChildren( pWin);
		freeTmpClip = TRUE;
	    }
	    else   /* ClipByChildren */
	    {
	        pregWin = &pWin->clipList;
		freeTmpClip = FALSE;
	    }

	    freeCompClip = devPriv->freeCompClip;

	    /* 
	     * if there is no client clip, we can get by with
	     * just keeping the pointer we got, and remembering
	     * whether or not should destroy (or maybe re-use)
	     * it later.  this way, we avoid unnecessary copying
	     * of regions.  (this wins especially if many clients clip
	     * by children and have no client clip.)
	     */
	    if ( pGC->clientClipType == CT_NONE)
	    {
		if ( freeCompClip)
		    (*pGC->pScreen->RegionDestroy)( devPriv->pCompositeClip);
		devPriv->pCompositeClip = pregWin;
		devPriv->freeCompClip = freeTmpClip;
	    }
	    else	/* client clipping enabled */
	    {
		/*
		 * We need one 'real' region to put into the composite clip.
		 * If pregWin and the current composite clip 
		 *  are real, we can get rid of one.
		 * If pregWin is real and the current composite
		 *  clip isn't, use pregWin for the composite clip.
		 * If the current composite clip is real and
		 *  pregWin isn't, use the current composite clip.
		 * If neither is real, create a new region.
		 */
		miTranslateRegion(pGC->clientClip,
				  pDrawable->x + pGC->clipOrg.x,
				  pDrawable->y + pGC->clipOrg.y);
		if (!freeCompClip)
		    if (freeTmpClip)
			devPriv->pCompositeClip = pregWin;
		    else
			devPriv->pCompositeClip = miRegionCreate(NullBox, 0);
		miIntersect(
			    devPriv->pCompositeClip,
			    pregWin,
			    pGC->clientClip);
		if (freeCompClip && freeTmpClip)
		    (*pGC->pScreen->RegionDestroy)(pregWin);
                devPriv->freeCompClip = TRUE;
		miTranslateRegion(pGC->clientClip,
				  -(pDrawable->x + pGC->clipOrg.x),
				  -(pDrawable->y + pGC->clipOrg.y));
	    }
	} /* end of composite clip for a window */
	else	/* output to a pixmap */
	{
	    BoxRec pixbounds;

	    pixbounds.x1 = QDPIX_X((QDPixPtr)pDrawable);
	    pixbounds.y1 = QDPIX_Y((QDPixPtr)pDrawable);
	    pixbounds.x2 = pixbounds.x1 + QDPIX_WIDTH((PixmapPtr)pDrawable);
	    pixbounds.y2 = pixbounds.y1 + QDPIX_HEIGHT((PixmapPtr)pDrawable);

	    if (devPriv->freeCompClip)
	        miRegionReset( devPriv->pCompositeClip, &pixbounds);
	    else
	    {
		devPriv->freeCompClip = TRUE;
		devPriv->pCompositeClip = miRegionCreate(&pixbounds, 1);
	    }

	    if (pGC->clientClipType == CT_REGION)
	    {
		miTranslateRegion(pGC->clientClip,
				  pDrawable->x + pGC->clipOrg.x,
				  pDrawable->y + pGC->clipOrg.y);
		miIntersect(
			devPriv->pCompositeClip,
			devPriv->pCompositeClip,
			pGC->clientClip);
		miTranslateRegion(pGC->clientClip,
				  -(pDrawable->x + pGC->clipOrg.x),
				  -(pDrawable->y + pGC->clipOrg.y));
	    }
	} /* end of composite clip for pixmap */
    }

    /*
     * invalidate the version of this stipple in the off screen cache
     */


    if (pGC->stateChanges & GCStipple && pGC->stipple)
	tlSinglePixmap ((QDPixPtr)pGC->stipple);

    if ( pGC->stateChanges & GCTile && !pGC->tileIsPixel)
    	tlSinglePixmap ((QDPixPtr)pGC->tile.pixmap);
	
    /* VALIDATION */
    {
	QDPrivGCPtr qdPriv =
	    (QDPrivGCPtr)pGC->devPrivates[qdGCPrivateIndex].ptr;
	int fillStyle = FillSolid; 
		/* -1 if not natural size, else pGC->fillStyle */
	GCOps *ops;
	int new_fill = changes & GCFillStyle+GCTile+GCStipple;
	int new_line =
	    changes & GCLineWidth+GCLineStyle+GCJoinStyle+GCDashList; 
	int new_text = changes & GCFont;
	int fontKind = 0;
	if (new_text | new_fill) {
	    fontKind = QDCreateFont(pGC->pScreen,pGC->font);
	    if (fontKind == 2 && !pGC->ops->devPrivate.val)
	        new_text = 0;
	}

	if ( pGC->stateChanges &
	    (GCFunction|GCPlaneMask|GCForeground|GCBackground|GCFillStyle))
	    qdPriv->mask |= QD_NEWLOGIC;
	if (qdPriv->lastDest != pDrawable->type) {
	    ops = pDrawable->type == DRAWABLE_PIXMAP ? &SolidPixOps
		: &SolidWinOps;
	    if (pGC->ops->devPrivate.val) {
		new_fill = 1;
		new_line = 1;
		new_text = 1;
		*pGC->ops = *ops;
		pGC->ops->devPrivate.val = 1;
	    }
	    pGC->ops = ops;
	    qdPriv->lastDest = pDrawable->type;
	}
	if (new_fill | new_line | new_text) {
	    if (!pGC->ops->devPrivate.val) { /* allocate private ops */
		ops = (GCOps *) Xalloc (sizeof *ops);
		*ops = *pGC->ops;
		pGC->ops = ops;
		ops->devPrivate.val = 1;
	    }
	    ops = pGC->ops;
	    if (pGC->lineWidth != 0) {
	        ops->PolyArc = miPolyArc;
	    }
	    switch ( pGC->fillStyle) {
	      case FillSolid:
		fillStyle = FillSolid;
		break;
	      case FillOpaqueStippled:
	      case FillStippled:
		if (qdNaturalSizePixmap(pGC->stipple))
		    fillStyle = pGC->fillStyle;
		else
		    fillStyle = -1;
		break;
	      case FillTiled:
		fillStyle =
		    qdNaturalSizePixmap(pGC->tile) ? FillTiled : -1;
		break;
	    }
	    if (new_fill | new_line) {
		/* set op->FillSpans, op->PolyFillRect */
		if (pDrawable->type == DRAWABLE_WINDOW) {
		    static PFN WinFillSpanTab[5] = {
			qdWinFSOddSize,
			tlSolidSpans,
			tlTiledSpans,
			tlStipSpans,
			tlOpStipSpans};
		    static PFN WinPolyFillRectTab[5] = {
			qdPolyFillRectOddSize,
			tlSolidRects,
			tlTiledRects,
			tlStipRects,
			tlOpStipRects};
		    ops->FillSpans = WinFillSpanTab[fillStyle + 1];
		    ops->PolyFillRect = WinPolyFillRectTab[fillStyle + 1];
		}
		else {  /* PIXMAP */
		    static PFN PixFillSpanTab[4] = {
			qdFSPixSolid,
			qdFSPixTiled,
			qdFSPixStippleorOpaqueStip,
			qdFSPixStippleorOpaqueStip};
		    ops->FillSpans = PixFillSpanTab[pGC->fillStyle];
		    if (fillStyle >= 0)
		        ops->PolyFillRect = qdPixFillRect;
		    else
		        ops->PolyFillRect = miPolyFillRect ;
		}
		/* set op->PushPixels */
		if (fillStyle < 0) /* unnatural size */
		    ops->PushPixels = qdUnnaturalPushPixels;
		else
		    ops->PushPixels = qdPushPixels;
	    }
	    /* set ops->Polylines, ops->PolySegment */
	    if (new_line | new_fill) {
		if (pGC->lineStyle != LineSolid) {
		    static PFN DoubleDashLineFuncs[4] = {
			tlPolylinesDashed,	/* Solid */
			tlPolylines,		/* Tiled */
			miWideDash,		/* Stippled */
			tlPolylines,		/* OpaqueStippled */
		    };
		    if (pGC->lineWidth != 0 || fillStyle < FillSolid
		      || pDrawable->type != DRAWABLE_WINDOW
		      || GC_DASH_LENGTH(pGC) > 1024) {
			ops->Polylines = (PFN)miWideDash;
		        ops->PolySegment = miPolySegment;
		    }
		    else if (pGC->lineStyle == LineOnOffDash) {
			ops->Polylines = tlPolylinesDashed;
			ops->PolySegment = tlPolySegmentDashed;
		    }
		    else {
		        static PFN DoubleDashSegmentFuncs[4] = {
			    tlPolySegmentDashed,	/* Solid */
			    tlPolySegment,		/* Tiled */
			    miPolySegment,		/* Stippled */
			    tlPolySegment,		/* OpaqueStippled */
			};
			ops->Polylines = DoubleDashLineFuncs[fillStyle];
			ops->PolySegment = DoubleDashSegmentFuncs[fillStyle];
		    }
		}
		else if (pGC->lineWidth != 0) {
		    ops->Polylines = (PFN)miWideLine;
		    ops->PolySegment = miPolySegment;
		}
		else if ((fillStyle < FillSolid )
		  || (pDrawable->type != DRAWABLE_WINDOW)) {
		    ops->Polylines = miZeroLine;
		    ops->PolySegment = miPolySegment;
		}
		else {
		    ops->Polylines = tlPolylines;
		    ops->PolySegment = tlPolySegment;
		}
	    }

	    if (new_fill | new_text) {
		if (fontKind == 0) {
		    ops->PolyText8 = miPolyText8;
		    ops->ImageText8 = miImageText8;
		} else if (pDrawable->type == DRAWABLE_PIXMAP) {
		    ops->PolyText8 = qdPolyTextPix;
		    ops->ImageText8 = qdImageTextPix;
		} else {
		    ops->PolyText8 =
			fillStyle == 0 ? tlPolyTextSolid :
			fillStyle > 0 ? tlPolyText :
			miPolyText8;
		    if (fontKind > 1) 
		        ops->ImageText8 = tlImageText ;
		    else ops->ImageText8 =  qdImageTextKerned;
		}
	    }
	}
    }
}

void
qdCopyClip( pgcDst, pgcSrc)
    GCPtr	pgcDst, pgcSrc;
{
    RegionPtr	prgnNew;

    switch( pgcSrc->clientClipType)
    {
      case CT_PIXMAP:
	((PixmapPtr) pgcSrc->clientClip)->refcnt++;
	/* Fall through !! */
      case CT_NONE:
        qdChangeClip( pgcDst, pgcSrc->clientClipType, pgcSrc->clientClip, 0);
        break;
      case CT_REGION:
        prgnNew = (* pgcSrc->pScreen->RegionCreate)(NULL, 1);
        (* pgcSrc->pScreen->RegionCopy)(prgnNew,
                                       (RegionPtr)(pgcSrc->clientClip));
        qdChangeClip( pgcDst, CT_REGION, prgnNew, 0);
        break;
    }
}

void
qdClipMoved(pGC, x, y)
    GCPtr pGC;
    int x, y;
{
    int xDelta = x - pGC->lastWinOrg.x;
    int yDelta = y - pGC->lastWinOrg.y;
    pGC->lastWinOrg.x = x;
    pGC->lastWinOrg.y = y;
    miTranslateRegion(QDGC_COMPOSITE_CLIP(pGC), xDelta, yDelta);
}
