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
static char *rcsid = "@(#)$RCSfile: vgafillr.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:34:36 $";
#endif

#include "X.h"
#include "Xprotostr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "miscstruct.h"
#include "regionstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"


/* 
 *	Filled rectangles.
 *	Translate the rectangles, clip them, and call the helper function 
 *	in the GC.
 */

void
vgaPolyFillRectSolid(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    int xorg, yorg;
    int n;		/* spare counter */
    xRectangle *prect; /* temporary */
    RegionPtr prgnClip;
    BoxPtr pbox;	/* used to clip with */
    BoxRec boxClipped;
    BoxPtr pextent;
    mfbPrivGC	*priv;
    int numRects;
    int first;

    FillSolidFuncPtr FillSolidFunc
      = ((DrawFuncs *)(pGC->pScreen->devPrivate))->FillSolidFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

    unsigned int fg  = (unsigned int) pGC->fgPixel;
    unsigned int alu = pGC->alu;
    unsigned int pm  = (unsigned int) pGC->planemask;
    priv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    prgnClip = priv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
        prect = prectInit;
	n = nrectFill;
	while (n--) {
	  prect->x += xorg;
	  prect->y += yorg;
	  prect++;
	}
    }

    prect = prectInit;

    pextent = (*pGC->pScreen->RegionExtents)(prgnClip);

    first = TRUE;
    
    while (nrectFill--)
    {
	BoxRec box;
	int	x2, y2;

	/*
	 * clip the box to the extent of the region --
	 * avoids overflowing shorts and minimizes other
	 * computations
	 */

	box.x1 = prect->x;
	if (box.x1 < pextent->x1)
		box.x1 = pextent->x1;

	box.y1 = prect->y;
	if (box.y1 < pextent->y1)
		box.y1 = pextent->y1;

	x2 = (int) prect->x + (int) prect->width;
	if (x2 > pextent->x2)
		x2 = pextent->x2;
	box.x2 = x2;

	y2 = (int) prect->y + (int) prect->height;
	if (y2 > pextent->y2)
		y2 = pextent->y2;
	box.y2 = y2;

	prect++;

	if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    continue;

	switch((*pGC->pScreen->RectIn)(prgnClip, &box))
	{
	  case rgnOUT:
	    break;
	  case rgnIN:

#ifdef SOFTWARE_CURSOR
	    (*HideCursorInXYWHFunc)(box.x1, box.y1, box.x2 - box.x1,
				    box.y2 - box.y1);
#endif

	    (*FillSolidFunc)(pGC->pScreen, fg, alu, pm, box.x1, box.y1,
			     box.x2 - box.x1, box.y2 - box.y1, first);
	    first = FALSE;

#ifdef SOFTWARE_CURSOR
	    (*ShowCursorFunc)();
#endif

	    break;
	  case rgnPART:
	    pbox = REGION_RECTS(prgnClip);
	    n = numRects;

	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
	        boxClipped.x1 = max(box.x1, pbox->x1);
	        boxClipped.y1 = max(box.y1, pbox->y1);
	        boxClipped.x2 = min(box.x2, pbox->x2);
	        boxClipped.y2 = min(box.y2, pbox->y2);
		pbox++;

	        /* see if clipping left anything */
	        if(boxClipped.x1 < boxClipped.x2 && 
	           boxClipped.y1 < boxClipped.y2)
	        {

#ifdef SOFTWARE_CURSOR
		  (*HideCursorInXYWHFunc)(boxClipped.x1, boxClipped.y1,
					  boxClipped.x2 - boxClipped.x1,
					  boxClipped.y2 - boxClipped.y1);
#endif

		  (*FillSolidFunc)(pGC->pScreen, fg, alu, pm, boxClipped.x1, 
				   boxClipped.y1, boxClipped.x2 - boxClipped.x1,
				   boxClipped.y2 - boxClipped.y1,
				   first);
		  first = FALSE;

#ifdef SOFTWARE_CURSOR
		  (*ShowCursorFunc)();
#endif

	        }
	    }
	    break;
	}
    }
}


void
vgaPolyFillRectStipple(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    int xorg, yorg;
    int n;		/* spare counter */
    xRectangle *prect; /* temporary */
    RegionPtr prgnClip;
    BoxPtr pbox;	/* used to clip with */
    BoxRec boxClipped;
    BoxPtr pextent;
    mfbPrivGC	*priv;
    int numRects;
    int first;

    OpaqueStippleFuncPtr OpaqueStippleFunc;
    ReplicateScansFuncPtr ReplicateScansFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif
  
    unsigned int fg  = (unsigned int) pGC->fgPixel;
    unsigned int bg  = (unsigned int) pGC->bgPixel;
    unsigned int alu = pGC->alu;
    unsigned int pm  = (unsigned int) pGC->planemask;

    PixmapPtr pStipple = pGC->stipple;
    int stippleWidth = pStipple->drawable.width;
    int stippleHeight = pStipple->drawable.height;
    unsigned char *psrcBase = pStipple->devPrivate.ptr;
    int widthSrc = pStipple->devKind;
    int stipOrgX = pDrawable->x +(pGC->patOrg.x % stippleWidth)-stippleWidth;
    int stipOrgY = pDrawable->y +(pGC->patOrg.y % stippleHeight)-stippleHeight;

    if (pGC->fillStyle == FillStippled) {
      OpaqueStippleFunc = (OpaqueStippleFuncPtr)
	(((DrawFuncs *)(pGC->pScreen->devPrivate))->StippleFunc);
    }
    else {
      OpaqueStippleFunc =
	((DrawFuncs *)(pGC->pScreen->devPrivate))->OpaqueStippleFunc;
      ReplicateScansFunc =
        ((DrawFuncs *)(pGC->pScreen->devPrivate))->ReplicateScansFunc;

    }

    priv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    prgnClip = priv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
        prect = prectInit;
	n = nrectFill;
	while (n--) {
	  prect->x += xorg;
	  prect->y += yorg;
	  prect++;
	}
    }

    prect = prectInit;

    pextent = (*pGC->pScreen->RegionExtents)(prgnClip);

    while (nrectFill--)
    {
	BoxRec box;
	int	x2, y2;

	/*
	 * clip the box to the extent of the region --
	 * avoids overflowing shorts and minimizes other
	 * computations
	 */

	box.x1 = prect->x;
	if (box.x1 < pextent->x1)
		box.x1 = pextent->x1;

	box.y1 = prect->y;
	if (box.y1 < pextent->y1)
		box.y1 = pextent->y1;

	x2 = (int) prect->x + (int) prect->width;
	if (x2 > pextent->x2)
		x2 = pextent->x2;
	box.x2 = x2;

	y2 = (int) prect->y + (int) prect->height;
	if (y2 > pextent->y2)
		y2 = pextent->y2;
	box.y2 = y2;

	prect++;

	if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    continue;

	switch((*pGC->pScreen->RectIn)(prgnClip, &box))
	{
	  case rgnOUT:
	    break;

	  case rgnIN:
	    {
	      int heightRep;
	      int doRep = 0;
              int y = box.y1;
	      int widthSave = box.x2 - box.x1;
	      int height = box.y2 - box.y1;
	      int xOffSrcSave = (box.x1 - stipOrgX) % stippleWidth;
	      int yOffSrc = (box.y1 - stipOrgY) % stippleHeight;
	      int widthRemSave = stippleWidth - xOffSrcSave;
	      int heightRem = stippleHeight - yOffSrc;

#ifdef SOFTWARE_CURSOR
	      (*HideCursorInXYWHFunc)(box.x1, box.y1, widthSave, height);
#endif

              if ( (alu == GXcopy)
                && (height>stippleHeight)
                && (pGC->fillStyle!=FillStippled) )
              {
		heightRep = height - stippleHeight;
		height = stippleHeight;
		doRep = 1;
              }

	      first = TRUE;
	      while (height > 0) {
		int x = box.x1;
		int width = widthSave;
		int xOffSrc = xOffSrcSave;
		int widthRem = widthRemSave;
		while (width > 0) {
		  (*OpaqueStippleFunc)(pGC->pScreen, xOffSrc, yOffSrc, 
				       min(width, widthRem),
				       min(height, heightRem),
				       psrcBase, widthSrc,
				       x, y, first, alu, pm, fg, bg);
		  first = FALSE;
		  x += widthRem;
		  width -= widthRem;
		  xOffSrc = 0;
		  widthRem = stippleWidth;
		}
		y += heightRem;
		height -= heightRem;
		yOffSrc = 0;
		heightRem = stippleHeight;
	      }

              if (doRep)
              {
		(*ReplicateScansFunc)(pGC->pScreen, box.x1, box.y1, widthSave,
                        stippleHeight, heightRep, pm);
              }

#ifdef SOFTWARE_CURSOR
	      (*ShowCursorFunc)();
#endif

	    }
	    break;

	  case rgnPART:
	    pbox = REGION_RECTS(prgnClip);
	    n = numRects;

	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
	        boxClipped.x1 = max(box.x1, pbox->x1);
	        boxClipped.y1 = max(box.y1, pbox->y1);
	        boxClipped.x2 = min(box.x2, pbox->x2);
	        boxClipped.y2 = min(box.y2, pbox->y2);
		pbox++;

	        /* see if clipping left anything */
	        if(boxClipped.x1 < boxClipped.x2 && 
	           boxClipped.y1 < boxClipped.y2)
	        {
                  int heightRep;
	          int doRep = 0;
                  int y = boxClipped.y1;
		  int widthSave = boxClipped.x2 - boxClipped.x1;
		  int height = boxClipped.y2 - boxClipped.y1;
		  int xOffSrcSave = (boxClipped.x1 - stipOrgX) % stippleWidth;
		  int yOffSrc = (boxClipped.y1 - stipOrgY) % stippleHeight;
		  int widthRemSave = stippleWidth - xOffSrcSave;
		  int heightRem = stippleHeight - yOffSrc;

#ifdef SOFTWARE_CURSOR
		  (*HideCursorInXYWHFunc)(boxClipped.x1, boxClipped.y1,
					  widthSave, height);
#endif

                  if ( (alu == GXcopy)
                    && (height > stippleHeight)
                    && (pGC->fillStyle != FillStippled) )
                  {
		    heightRep = height - stippleHeight;
		    height = stippleHeight;
                    doRep = 1;
                  }

		  first = TRUE;
		  while (height > 0) {
		    int x = boxClipped.x1;
		    int width = widthSave;
		    int xOffSrc = xOffSrcSave;
		    int widthRem = widthRemSave;
		    while (width > 0) {
		      (*OpaqueStippleFunc)(pGC->pScreen, xOffSrc, yOffSrc, 
					   min(width, widthRem),
					   min(height, heightRem),
					   psrcBase, widthSrc,
					   x, y, first, alu, pm, fg, bg);
		      first = FALSE;
		      x += widthRem;
		      width -= widthRem;
		      xOffSrc = 0;
		      widthRem = stippleWidth;
		    }
		    y += heightRem;
		    height -= heightRem;
		    yOffSrc = 0;
		    heightRem = stippleHeight;
		  }
		  
		  if (doRep)
                  {
		    (*ReplicateScansFunc)(pGC->pScreen, boxClipped.x1, 
			boxClipped.y1, widthSave, stippleHeight, heightRep, pm);
                  }

#ifdef SOFTWARE_CURSOR
		  (*ShowCursorFunc)();
#endif

		}
	      }
	    break;
	}
    }
}


void
vgaPolyFillRectTile(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    int xorg, yorg;
    int n;		/* spare counter */
    xRectangle *prect; /* temporary */
    RegionPtr prgnClip;
    BoxPtr pbox;	/* used to clip with */
    BoxRec boxClipped;
    BoxPtr pextent;
    mfbPrivGC	*priv;
    int numRects;
    int first;
    
    DrawColorImageFuncPtr DrawColorImageFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->DrawColorImageFunc;

    ReplicateScansFuncPtr ReplicateScansFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ReplicateScansFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

    unsigned int alu = pGC->alu;
    unsigned int pm  = (unsigned int) pGC->planemask;

    PixmapPtr pTile = pGC->tile.pixmap;
    int tileWidth = pTile->drawable.width;
    int tileHeight = pTile->drawable.height;
    unsigned char *psrcBase = pTile->devPrivate.ptr;
    int widthSrc = pTile->devKind;
    int stipOrgX = pDrawable->x +(pGC->patOrg.x % tileWidth)-tileWidth;
    int stipOrgY = pDrawable->y +(pGC->patOrg.y % tileHeight)-tileHeight;

    priv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    prgnClip = priv->pCompositeClip;

    numRects = REGION_NUM_RECTS(prgnClip);

    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
        prect = prectInit;
	n = nrectFill;
	while (n--) {
	  prect->x += xorg;
	  prect->y += yorg;
	  prect++;
	}
    }

    prect = prectInit;

    pextent = (*pGC->pScreen->RegionExtents)(prgnClip);

    while (nrectFill--)
    {
	BoxRec box;
	int	x2, y2;

	/*
	 * clip the box to the extent of the region --
	 * avoids overflowing shorts and minimizes other
	 * computations
	 */

	box.x1 = prect->x;
	if (box.x1 < pextent->x1)
		box.x1 = pextent->x1;

	box.y1 = prect->y;
	if (box.y1 < pextent->y1)
		box.y1 = pextent->y1;

	x2 = (int) prect->x + (int) prect->width;
	if (x2 > pextent->x2)
		x2 = pextent->x2;
	box.x2 = x2;

	y2 = (int) prect->y + (int) prect->height;
	if (y2 > pextent->y2)
		y2 = pextent->y2;
	box.y2 = y2;

	prect++;

	if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    continue;

	switch((*pGC->pScreen->RectIn)(prgnClip, &box))
	{
	  case rgnOUT:
	    break;

	  case rgnIN:
	    {
	      int heightRep;
	      int doRep = 0;
	      int y = box.y1;
	      int widthSave = box.x2 - box.x1;
	      int height = box.y2 - box.y1;
	      int xOffSrcSave = (box.x1 - stipOrgX) % tileWidth;
	      int yOffSrc = (box.y1 - stipOrgY) % tileHeight;
	      int widthRemSave = tileWidth - xOffSrcSave;
	      int heightRem = tileHeight - yOffSrc;

#ifdef SOFTWARE_CURSOR
	      (*HideCursorInXYWHFunc)(box.x1, box.y1, widthSave, height);
#endif

	      if (alu==GXcopy && height > tileHeight) {
		heightRep = height - tileHeight;
		height = tileHeight;
		doRep = 1;
	      }
	      first = TRUE;
	      while (height > 0) {
		int x = box.x1;
		int width = widthSave;
		int xOffSrc = xOffSrcSave;
		int widthRem = widthRemSave;
		while (width > 0) {
		  (*DrawColorImageFunc)(pGC->pScreen, x, y,
					min(width, widthRem),
					min(height, heightRem),
					psrcBase + (yOffSrc * widthSrc)
					+ xOffSrc,
					widthSrc, alu, pm, first);
		  first = FALSE;
		  x += widthRem;
		  width -= widthRem;
		  xOffSrc = 0;
		  widthRem = tileWidth;
		}
		y += heightRem;
		height -= heightRem;
		yOffSrc = 0;
		heightRem = tileHeight;
	      }
	      if (doRep) {
		(*ReplicateScansFunc)(pGC->pScreen, box.x1, box.y1, widthSave, 
				      tileHeight, heightRep, pm);
	      }

#ifdef SOFTWARE_CURSOR
	      (*ShowCursorFunc)();
#endif

	    }
	    break;

	  case rgnPART:
	    pbox = REGION_RECTS(prgnClip);
	    n = numRects;

	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
	        boxClipped.x1 = max(box.x1, pbox->x1);
	        boxClipped.y1 = max(box.y1, pbox->y1);
	        boxClipped.x2 = min(box.x2, pbox->x2);
	        boxClipped.y2 = min(box.y2, pbox->y2);
		pbox++;

	        /* see if clipping left anything */
	        if(boxClipped.x1 < boxClipped.x2 && 
	           boxClipped.y1 < boxClipped.y2)
	        {
		  int heightRep;
		  int doRep = 0;
		  int y = boxClipped.y1;
		  int widthSave = boxClipped.x2 - boxClipped.x1;
		  int height = boxClipped.y2 - boxClipped.y1;
		  int xOffSrcSave = (boxClipped.x1 - stipOrgX) % tileWidth;
		  int yOffSrc = (boxClipped.y1 - stipOrgY) % tileHeight;
		  int widthRemSave = tileWidth - xOffSrcSave;
		  int heightRem = tileHeight - yOffSrc;

#ifdef SOFTWARE_CURSOR
		  (*HideCursorInXYWHFunc)(boxClipped.x1, boxClipped.y1,
					  widthSave, height);
#endif
		  
		  if (alu==GXcopy && height > tileHeight) {
		    heightRep = height - tileHeight;
		    height = tileHeight;
		    doRep = 1;
		  }
		  first = TRUE;
		  while (height > 0) {
		    int x = boxClipped.x1;
		    int width = widthSave;
		    int xOffSrc = xOffSrcSave;
		    int widthRem = widthRemSave;
		    while (width > 0) {
		      (*DrawColorImageFunc)(pGC->pScreen, x, y,
					    min(width, widthRem),
					    min(height, heightRem),
					    psrcBase + (yOffSrc * widthSrc)
					    + xOffSrc,
					    widthSrc, alu, pm, first);
		      first = FALSE;
		      x += widthRem;
		      width -= widthRem;
		      xOffSrc = 0;
		      widthRem = tileWidth;
		    }
		    y += heightRem;
		    height -= heightRem;
		    yOffSrc = 0;
		    heightRem = tileHeight;
		  }
		  if (doRep) {
		    (*ReplicateScansFunc)(pGC->pScreen, boxClipped.x1, 
					  boxClipped.y1, widthSave, tileHeight,
					  heightRep, pm);
		  }

#ifdef SOFTWARE_CURSOR
		  (*ShowCursorFunc)();
#endif

		}
	      }
	    break;
	}
    }
}

