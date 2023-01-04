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
#define IDENT "X-007"
#define MODULE_NAME SFBBITBLT
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
*****************************************************************************
*/
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**      This module does bitblts for the SFB hardware
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**
**  CREATION DATE:     20-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-007        DMC0011         Dave Coleman                    29-Jan-1992
**              Use correct parameters for cfbDoBitblt routines.
**
** X-006	BIM0011		Irene McCartney			27-Jan-1992
**		Merge Joel's latest changes.
**
** X-005	TLB0007 	Tracy Bragdon 			21-Jan-1992
**		Add missing parameters to cfbDoBitblt* calls
**
** X-004	DMC0003 	Dave Coleman			20-Dec-1991
**		Correct capitalization of cfbDoBitblt*
**
** X-3		TLB0003		Tracy Bragdon			03-Dec-1991
**		change references to GC field for R5
**
** X-2		BIM0009		Irene McCartney			02-Dec-1991
**		Change name of Bitblt routines for MIT R5 server
**		Add edit history
**--
**/

#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "servermd.h"

#include "sfb.h"
#include "cfbmskbits.h"
#include "bitblt.h"
#include "sfbbitblt.h"
#include "sfbblt.h"
#include "cfbpixmap.h"

#ifdef MITR5
static VoidProc copyAreaMemMem[4] = {
    cfbDoBitbltCopy,	    cfbDoBitbltCopy,
    cfbDoBitbltXor,	    cfbDoBitbltGeneral
};
#else
static VoidProc copyAreaMemMem[4] = {
    cfbBitbltCopy,	    cfbBitbltCopySPM,
    cfbBitbltXor,	    cfbBitbltGeneral
};
#endif

static VoidProc copyAreaScrMem[4] = {
    sfbBitbltScrMemCopy,    sfbBitbltScrMemCopySPM,
    sfbBitbltScrMemXor,	    sfbBitbltScrMemGeneral
};

#ifdef MITR5
/*
  The following definition is copied from ddx/dec/cfb/cfb.h and is
  used for the R4 implementation of reducing raster ops for the bitblt
  routines.
*/
/*
  The following definitions are taken from the MIT R4 version of
  ddx/dec/cfb/cfbgc.c and is used for the sfb bitblt routines.  The
  ddx/cfb bitblt routines in MITR5 use a different method of reducing
  rop operations, but the sfb routines still use the R4 implementation.
*/
typedef struct {
    DXop	    fillRop;
    DXop	    copyRop;
    Pixel32	    andbits1;
    Pixel32	    xorbits1;
    Pixel32	    andbits2;
    Pixel32	    xorbits2;
} RopToDXop;

static RopToDXop ropToDXop[16] = {
/* X11 rop      fillrop      copyrop     andbits1, xorbits1, andbits2, xorbits2
---------------------------------------------------------------------------*/
/* clear   */	{DXcopy,     DXgeneral,      0,        0,        0,        0},
/* and     */   {DXgeneral,  DXgeneral,     -1,        0,        0,        0},
/* andRev  */	{DXgeneral,  DXgeneral,     -1,        0,       -1,        0},
/* copy    */	{DXcopy,     DXcopy,         0,        0,       -1,        0},

/* andInv  */	{DXgeneral,  DXgeneral,     -1,       -1,        0,        0},
/* noop    */	{DXxor,      DXxor,          0,       -1,        0,        0},
/* xor     */	{DXxor,      DXxor,          0,       -1,       -1,        0},
/* or      */	{DXgeneral,  DXgeneral,     -1,       -1,       -1,        0},

/* nor     */	{DXgeneral,  DXgeneral,     -1,       -1,       -1,        -1},
/* equiv   */	{DXxor,      DXxor,          0,       -1,       -1,        -1},
/* invert  */	{DXxor,      DXgeneral,      0,       -1,        0,        -1},
/* orRev   */   {DXgeneral,  DXgeneral,     -1,       -1,        0,        -1},

/* copyInv */	{DXcopy,     DXgeneral,      0,        0,       -1,        -1},
/* orInv   */	{DXgeneral,  DXgeneral,     -1,        0,       -1,        -1},
/* nand    */	{DXgeneral,  DXgeneral,     -1,        0,        0,        -1},
/* set     */	{DXcopy,     DXgeneral,      0,        0,        0,        -1}
};
#endif	/* MITR5 */

/* CopyArea

    clip the source rectangle to the source's available bits.  (this
avoids copying unnecessary pieces that will just get exposed anyway.)
this becomes the new shape of the destination.
    clip the destination region to the composite clip in the
GC.  this requires translating the destination region to (dstx, dsty).
    build a list of source points, one for each rectangle in the
destination.  this is a simple translation.
    go do the multiple rectangle copies
    do graphics exposures
*/

extern Bool8 sfbIdempotent[];

RegionPtr
sfbCopyArea(pSrcDraw, pDstDraw, pGC, srcx, srcy, width, height, dstx, dsty)
    register DrawablePtr pSrcDraw;
    register DrawablePtr pDstDraw;
    GC *pGC;
    int srcx, srcy;
    int width, height;
    int dstx, dsty;
{
    RegionPtr		    prgnDst, prgnExposed = NULL;
    DDXPointPtr		    pptSrc;
    register DDXPointPtr    ppt;
    register BoxPtr	    pbox;
    int			    numRects;
    register int	    dx;
    register int	    dy;
    xRectangle		    origSource;
    DDXPointRec		    origDest;
    cfbPrivGC		    *gcPriv;

#ifdef MITR5
    DXop	DXcopyRop;
    Pixel32	planemask;
    RopToDXop	*ropDXop;
#endif	/* MITR5 */

    if ((width == 0) || (height == 0)) return (RegionPtr) NULL;

    if (pDstDraw->depth == 1)
	return mfbCopyArea(pSrcDraw, pDstDraw, pGC, srcx, srcy, width, height, 
			   dstx, dsty);

    origSource.x = srcx;
    origSource.y = srcy;
    origSource.width = width;
    origSource.height = height;
    origDest.x = dstx;
    origDest.y = dsty;

    if (pDstDraw->type == DRAWABLE_WINDOW) {
        if (!((WindowPtr)pDstDraw)->realized) {
	    return (RegionPtr) NULL;
        }
    }
    dstx += pDstDraw->x;
    dsty += pDstDraw->y;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);

    /* Clip the source and translate it to destination coordinates */

    if ((pSrcDraw != pDstDraw) && pSrcDraw->pScreen->SourceValidate)
    {
	(*pSrcDraw->pScreen->SourceValidate)
	    (pSrcDraw, srcx, srcy, width, height);
    }

    srcx += pSrcDraw->x;
    srcy += pSrcDraw->y;

    if (pSrcDraw->type == DRAWABLE_PIXMAP) {
	BoxRec box;
	register int x1, x2, y1, y2;

	x1 = srcx;
	y1 = srcy;
	x2 = x1 + width;
	y2 = y1 + height;

	/* Shrink as necessary to fit pixmap boundaries */
	if (x1 < pSrcDraw->x)
	    x1 = pSrcDraw->x;
	if (y1 < pSrcDraw->y)
	    y1 = pSrcDraw->y;
	if (x2 > pSrcDraw->x + (int) pSrcDraw->width)
	    x2 = pSrcDraw->x + (int) pSrcDraw->width;
	if (y2 > pSrcDraw->y + (int) pSrcDraw->height)
	    y2 = pSrcDraw->y + (int) pSrcDraw->height;

	/* Translate to destination coordinates, and regionify.  */
	dx = srcx - dstx;
	dy = srcy - dsty;
	
	if (x2 > x1 && y2 > y1) {
	    box.x1 = x1 - dx;
	    box.y1 = y1 - dy;
	    box.x2 = x2 - dx;
	    box.y2 = y2 - dy;
	    prgnDst = (*pGC->pScreen->RegionCreate)(&box, 1);
	} else {
	    prgnDst = (*pGC->pScreen->RegionCreate)((RegionPtr)NULL, 1);
	}
    } else {   
	BoxRec      box;
	RegionPtr   prgnSrcClip;	 /* may be a new region or a copy */
	Bool	    realSrcClip = FALSE; /* TRUE if we create a src clip */
	register int x2, y2;

        if (pGC->subWindowMode == IncludeInferiors) {
            if ((pSrcDraw == pDstDraw) &&
		(pGC->clientClipType == CT_NONE)) {
                prgnSrcClip = gcPriv->pCompositeClip;
            } else {   
                prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDraw);
                realSrcClip = TRUE;
            }
        } else {
            prgnSrcClip = &((WindowPtr)pSrcDraw)->clipList;
        }
	box.x1 = srcx;
	box.y1 = srcy;
	x2 = srcx + width;
	y2 = srcy + height;
	/* Check for overflow beyond coordinate system */
	if (x2 > 32767) x2 = 32767;
	if (y2 > 32767) y2 = 32767;
	box.x2 = x2;
	box.y2 = y2;
    
	prgnDst = (*pGC->pScreen->RegionCreate)(&box, 1);
	(*pGC->pScreen->Intersect)(prgnDst, prgnDst, prgnSrcClip);
	if (realSrcClip)
	    (*pGC->pScreen->RegionDestroy)(prgnSrcClip);

	/* Translate to destination coordinates */
	dx = srcx - dstx;
	dy = srcy - dsty;
	(*pGC->pScreen->TranslateRegion)(prgnDst, -dx, -dy);
    }    

    /* clip the shape of the dst to the destination composite clip */
    (*pGC->pScreen->Intersect)(prgnDst, prgnDst, gcPriv->pCompositeClip);

    numRects = REGION_NUM_RECTS(prgnDst);
    if (numRects) {
	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						    sizeof(DDXPointRec)))) {
	    (*pGC->pScreen->RegionDestroy)(prgnDst);
	    return (RegionPtr)NULL;
	}
	pbox = REGION_RECTS(prgnDst);
	ppt = pptSrc;
	for ( ; numRects != 0; numRects--, pbox++, ppt++) {
	    ppt->x = pbox->x1 + dx;
	    ppt->y = pbox->y1 + dy;
	}
    
	if (pDstDraw->type == DRAWABLE_WINDOW 
	    || SCREENMEMORY((PixmapPtr)pDstDraw)) {
	    sfbScreenPrivPtr    scrPriv;
	    SFB			sfb;

	    CHECKSTATE(pDstDraw->pScreen, scrPriv, sfb, pGC);
	    if (pSrcDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pSrcDraw)) {
		sfbBitbltScrScr(pSrcDraw, pDstDraw, prgnDst, pptSrc);
	    } else {
		sfbBitbltMemScr(pSrcDraw, pDstDraw, prgnDst, pptSrc);
	    }
	} else {
	    VoidProc    copyProc;
	    /* No need to check state...destination is main memory. */
#ifdef MITR5
    ropDXop = &ropToDXop[pGC->alu];
    DXcopyRop = ropDXop->copyRop;
#endif	/* MITR5 */
	    if (pSrcDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pSrcDraw)) {
#ifdef MITR5
       		copyProc = copyAreaScrMem[DXcopyRop];
		(*copyProc)	(pSrcDraw, pDstDraw, prgnDst, pptSrc,
				ropDXop->andbits1, ropDXop->xorbits1,
				ropDXop->andbits2, ropDXop->xorbits2);
	    } else {
   	   	copyProc = copyAreaMemMem[DXcopyRop];
		(*copyProc)	(pSrcDraw, pDstDraw, pGC->alu,
				prgnDst, pptSrc, ~0L);
   	    }
#else	/* MITR5 */
	   	copyProc = copyAreaScrMem[gcPriv->copyRop];
	    } else {               
		copyProc = copyAreaMemMem[gcPriv->copyRop];
	    }
	    (*copyProc)	(pSrcDraw, pDstDraw, prgnDst, pptSrc,
		gcPriv->andbits1, gcPriv->xorbits1,
		gcPriv->andbits2, gcPriv->xorbits2);
#endif	/* MITR5 */
	}
	DEALLOCATE_LOCAL(pptSrc);
    }

    if (gcPriv->fExpose)
	prgnExposed = miHandleExposures(pSrcDraw, pDstDraw, pGC,
			  origSource.x, origSource.y,
			  (int) origSource.width, (int) origSource.height,
			  origDest.x, origDest.y, (unsigned long) 0);

    (*pGC->pScreen->RegionDestroy)(prgnDst);
    return prgnExposed;
}

/* sfbPutImage -- public entry for the PutImage request
 * Here we benefit from knowing the format of the bits pointed to by pImage,
 * even if we don't know how pDraw represents them.  
 * Three different strategies are used depending on the format 
 * XYBitmap Format:
 *	kludge up a bitmap and call CopyPlane
 * XYPixmap format:
 *	what we're called with is a series of XYBitmaps, but we only want 
 *	each XYPixmap to update 1 plane, instead of updating all of them.
 * 	we set the foreground color to be all 1s and the background to all 0s
 *	then for each plane, we set the plane mask to only effect that one
 *	plane and do the same as XYBitmap format
 * ZPixmap format:
 *	kludge up a pixmap and call CopyArea
 */

void
sfbPutImage(pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr     pDraw;
    GCPtr	    pGC;
    int		    depth, x, y, w, h, leftPad;
    unsigned int    format;
    Pixel8	    *pImage;
{
    cfbPrivGC	    *gcPriv;
    PixelWord	    oldFg, oldBg;
    unsigned long   gcv[3];
    PixelWord	    oldPlanemask;
    unsigned long   i;
    pointer	    pbits;
    PixmapPtr       pFakePixmap;
    pointer	    ptrImage;

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    switch(format) {
      case XYBitmap:
        pFakePixmap = 
	    (PixmapPtr) mfbCreatePixmap(pDraw->pScreen, w+leftPad, h, 1);
	break;
      case XYPixmap:
        pFakePixmap = 
	    (PixmapPtr) mfbCreatePixmap(pDraw->pScreen, w+leftPad, h, 1);
	break;
      case ZPixmap:
        pFakePixmap = (PixmapPtr)
	   cfbCreatePixmap(pDraw->pScreen, w+leftPad, h, SFBPIXELBITS);
	break;
    }
    if (!pFakePixmap) {
	ErrorF( "sfbPutImage can't make temp pixmap\n");
	return;
    }
    pbits = pFakePixmap->devPrivate.ptr;
    gcPriv->fExpose = FALSE;
    pFakePixmap->devPrivate.ptr = (pointer)pImage;

    switch(format) {
      case XYBitmap:
        (void)(*pGC->ops->CopyPlane)
	    (pFakePixmap, pDraw, pGC, leftPad, 0, w, h, x, y, 1);
	break;

      case XYPixmap:
	depth = pGC->depth;
	oldPlanemask = pGC->planemask;
	oldFg = pGC->fgPixel;
	oldBg = pGC->bgPixel;
	gcv[0] = ~0;
	gcv[1] = 0;
	DoChangeGC(pGC, GCForeground | GCBackground, gcv, 0);

	ptrImage = pFakePixmap->devPrivate.ptr;
	for (i = 1 << (depth-1); i != 0; i >>= 1) {
	    if (i & oldPlanemask) {
	        gcv[0] = i;
	        DoChangeGC(pGC, GCPlaneMask, gcv, 0);
	        ValidateGC(pDraw, pGC);
		pFakePixmap->devPrivate.ptr = (pointer)ptrImage;
		(void)(*pGC->ops->CopyPlane)
		    (pFakePixmap, pDraw, pGC, leftPad, 0, w, h, x, y, 1);
	    }
	    ptrImage += h * PixmapBytePad(w, 1);
	}
	gcv[0] = oldPlanemask;
	gcv[1] = oldFg;
	gcv[2] = oldBg;
	DoChangeGC(pGC, GCPlaneMask | GCForeground | GCBackground, gcv, 0);
	break;

      case ZPixmap:
        (void)(*pGC->ops->CopyArea)
	    (pFakePixmap, pDraw, pGC, leftPad, 0, w, h, x, y);
	break;
    }
    gcPriv->fExpose = TRUE;
    pFakePixmap->devPrivate.ptr = pbits;
    (*pDraw->pScreen->DestroyPixmap)(pFakePixmap);
}

void sfbGetImage(pDraw, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDraw;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer	pdstLine;
{
    PixmapRec		FakePixmap;
    BoxRec 		box;
    DDXPointRec 	ptSrc;
    RegionRec 		rgnDst;
    SFB			sfb;

    if ((w == 0) || (h == 0))
	return;
    if (pDraw->bitsPerPixel == 1) {
	mfbGetImage(pDraw, sx, sy, w, h, format, planeMask, pdstLine);
	return;
    }

    sfb = SFBSCREENPRIV(pDraw->pScreen)->sfb;
    if (format == ZPixmap) {
    	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDraw->pScreen;
    	FakePixmap.drawable.depth = pDraw->depth;
    	FakePixmap.drawable.bitsPerPixel = pDraw->depth;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w;
    	FakePixmap.drawable.height = h;
    	FakePixmap.devKind = PixmapBytePad(w, pDraw->depth);
    	FakePixmap.refcnt = 1;
        FakePixmap.devPrivate.ptr = (pointer)pdstLine;
	ptSrc.x = sx + pDraw->x;
        ptSrc.y = sy + pDraw->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        (*pDraw->pScreen->RegionInit)(&rgnDst, &box, 1);

/* ||| If put in real 32 bits/pixel code, then need to determine if 
	depth != bitsPerPixel, and execute appropriate expansion/contraction
	code if not.  See the MX code for an example of this. */

	if ((planeMask & SFBPIXELALL1) != SFBPIXELALL1) {
	    bzero((char *)pdstLine, FakePixmap.devKind * h);
	    PixelToPixelWord(planeMask);
	    if (pDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pDraw)) {
		sfbBitbltScrMemCopySPM(pDraw, (DrawablePtr)&FakePixmap,&rgnDst,
		    &ptSrc, (PixelWord)0, (PixelWord)~planeMask, 
		    (PixelWord)planeMask, (PixelWord)0);
	    } else {
#ifdef MITR5
/* DMC - Parameters have changed for R5 routines - this will require
   more extensive changes, but fix the parameters for now
*/
		cfbDoBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, GXcopy,
		    &rgnDst, &ptSrc, planeMask);
#else	/* MITR5 */
		cfbBitbltCopySPM(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
		    &ptSrc, 0, ~planeMask, planeMask, 0);
#endif	/* MITR5 */
	    }
	} else {
	    if (pDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pDraw)) {
		sfbBitbltScrMemCopy(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
		    &ptSrc, 
		    (PixelWord)0, (PixelWord)0, (PixelWord)-1, (PixelWord)0);
	    } else {
#ifdef MITR5
/* DMC - Parameters have changed for R5 routines - this will require
   more extensive changes, but fix the parameters for now
*/
		cfbDoBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, GXcopy,
		    &rgnDst, &ptSrc, planeMask);
#else	/* MITR5 */
		cfbBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
		    &ptSrc, 0, -1, 0, 0);
#endif	/* MITR5 */
	    }
	}
	(*pDraw->pScreen->RegionUninit)(&rgnDst);
    } else { /* XYPixmap format */
	/* DIX has loop around this case to feed us a planemask with exactly
           one bit set.  Don't ask me why it doesn't let us do it. */
#if PSZ==8
	/* For SFB, pixel size is always 8, so miGetImage (below) will
	 * never be called. If it does get used in the future, sfb
	 * will need to be set up in SIMPLE mode in order for it to
	 * work correctly.
	 */
	FakePixmap.drawable.type = DRAWABLE_PIXMAP;
    	FakePixmap.drawable.class = 0;
    	FakePixmap.drawable.pScreen = pDraw->pScreen;
    	FakePixmap.drawable.depth = 1;
    	FakePixmap.drawable.bitsPerPixel = 1;
    	FakePixmap.drawable.id = 0;
    	FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    	FakePixmap.drawable.x = 0;
    	FakePixmap.drawable.y = 0;
    	FakePixmap.drawable.width = w;
    	FakePixmap.drawable.height = h;
    	FakePixmap.devKind = PixmapBytePad(w, 1);
    	FakePixmap.refcnt = 1;
        FakePixmap.devPrivate.ptr = (pointer)pdstLine;
        ptSrc.x = sx + pDraw->x;
        ptSrc.y = sy + pDraw->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        (*pDraw->pScreen->RegionInit)(&rgnDst, &box, 1);
#ifdef MITR5
	cfbCopyImagePlane (pDraw, (DrawablePtr)&FakePixmap, GXcopy, &rgnDst,
		&ptSrc, planeMask);
#else /* MITR5 */
	sfbCopyImagePlaneScrMem (pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
		    &ptSrc, 0, 0, -1, 0);
#endif /* MITR5 */
        (*pDraw->pScreen->RegionUninit)(&rgnDst);
#else
	miGetImage(pDraw, sx, sy, w, h, format, planeMask, pdstLine);
#endif
    }
}
