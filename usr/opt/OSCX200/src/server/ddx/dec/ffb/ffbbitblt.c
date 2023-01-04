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
static char *rcsid = "@(#)$RCSfile: ffbbitblt.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/13 17:32:46 $";
#endif
/*
 */

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

#include "ffb.h"
#include "cfbmskbits.h"
#include "bitblt.h"
#include "ffbbitblt.h"
#include "ffbblt.h"      /* see ffbcopy.[ch] for declaration of ffbCopyTab */
#include "cfbpixmap.h"
#include "ffbcpu.h"
#include "ffbfill.h"

#define EXTENTCHECKINTERSECT(r1x1,r1x2,r2x1,r2x2,r1y1,r1y2,r2y1,r2y2) \
      (!( ((r1x2) <= (r2x1))  | \
          ((r1x1) >= (r2x2))  | \
          ((r1y2) <= (r2y1))  | \
          ((r1y1) >= (r2y2)) ) )

static BoxRec EmptyBox = {0, 0, 0, 0};
static RegDataRec EmptyData = {0, 0};
    
#ifdef MITR5
static VoidProc copyAreaMemMem[2][4] = {
    {
	cfbDoBitbltCopy,    cfbDoBitbltCopy,
	cfbDoBitbltXor,     cfbDoBitbltGeneral
    }, {
	cfb32DoBitbltCopy,  cfb32DoBitbltCopy,
	cfb32DoBitbltXor,   cfb32DoBitbltGeneral
    }
};
#else
static VoidProc copyAreaMemMem[2][4] = {
    {
	cfbBitbltCopy,      cfbBitbltCopySPM,
	cfbBitbltXor,       cfbBitbltGeneral
    }, {
	cfb32BitbltCopy,    cfb32BitbltCopy,
	cfb32BitbltXor,     cfb32BitbltGeneral
    }
};
#endif

#ifdef MITR5
/*
  The following definition is copied from ddx/dec/cfb/cfb.h and is
  used for the R4 implementation of reducing raster ops for the bitblt
  routines.
*/

/*
  The following definitions are taken from the MIT R4 version of
  ddx/dec/cfb/cfbgc.c and is used for the ffb bitblt routines.  The
  ddx/cfb bitblt routines in MITR5 use a different method of reducing
  rop operations, but the ffb routines still use the R4 implementation.
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
/* invert  */	{DXxor,      DXxor,          0,       -1,        0,        -1},
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

RegionPtr
ffbCopyArea(pSrcDraw, pDstDraw, pGC, srcx, srcy, width, height, dstx, dsty)
    DrawablePtr pSrcDraw;
    DrawablePtr pDstDraw;
    GC 		*pGC;
    int 	srcx, srcy;
    int 	width, height;
    int 	dstx, dsty;
{
    RegionPtr		    prgnExposed = NULL;
    RegionRec		    rgnDst;
    DDXPointPtr		    pptSrc;
    register DDXPointPtr    ppt;
    register BoxPtr	    pbox;
    int			    numRects;
    register int	    dx;
    register int	    dy;
    xRectangle		    origSource;
    DDXPointRec		    origDest;
    cfbPrivGC		    *gcPriv;
    ScreenPtr		    pScreen;
#ifdef MITR5
    DXop		    DXcopyRop;
    Pixel32		    planemask;
    RopToDXop		    *ropDXop;
#endif	/* MITR5 */

    if ((width == 0) || (height == 0)) return (RegionPtr) NULL;

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
    pScreen = pGC->pScreen;

    /* Clip the source and translate it to destination coordinates */

    if ( pSrcDraw->pScreen->SourceValidate && (pSrcDraw != pDstDraw))
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
	
	/* Region Init */
	     rgnDst.extents = *(&box);
             rgnDst.data = (RegDataPtr)NULL;
	} else {
	     (&rgnDst)->extents = EmptyBox;
             (&rgnDst)->data =  &EmptyData;
	}
    } else {   
	BoxRec      box;
	RegionPtr   prgnSrcClip;	 /* may be a new region or a copy */
	Bool	    realSrcClip = FALSE; /* TRUE if we create a src clip */
	long 	    x2, y2;

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

	x2 = srcx + width;
	y2 = srcy + height;
	/* Check for overflow beyond coordinate system */
	if (x2 > 32767) x2 = 32767;
	if (y2 > 32767) y2 = 32767;

/* 	Region Init */
	(&rgnDst)->data = (RegDataPtr)NULL;
/*	Intersect */
	{
 	long yx,r2x1,r2x2,r2y1,r2y2;
    
	yx = ((int *)&(prgnSrcClip->extents))[0];
    	r2y1 = yShort(yx);
    	r2x1 = xShort(yx);
    	yx = ((int *)&(prgnSrcClip->extents))[1];
    	r2y2 = yShort(yx);
    	r2x2 = xShort(yx);
	if (  REGION_NIL(prgnSrcClip) || !EXTENTCHECKINTERSECT(srcx,x2,r2x1,r2x2,srcy,y2,r2y1,r2y2)){
            ((int *)&(rgnDst.extents))[0] = ((int *)&(rgnDst.extents))[1] = (srcy << 16) | srcx;
            rgnDst.data = &EmptyData;
        } else if (!prgnSrcClip->data){
            long yx2;
            yx = max(srcx, r2x1);
            yx2 = max(srcy, r2y1);
            ((int *)&(rgnDst.extents))[0] = (yx2 << 16) | yx;
            yx = min(x2, r2x2);
            yx2 = min(y2, r2y2);
            ((int *)&(rgnDst.extents))[1] = (yx2 << 16) | yx;
        } else {
            ((int *)&(rgnDst.extents))[0] = (srcy << 16) | srcx;
            ((int *)&(rgnDst.extents))[1] = (y2 << 16) | x2;
	    (*pScreen->Intersect)(&rgnDst, &rgnDst, prgnSrcClip);
        }
	}
	if (realSrcClip)
	    (*pScreen->RegionDestroy)(prgnSrcClip);

	/* Translate to destination coordinates */
	dx = srcx - dstx;
	dy = srcy - dsty;
	(*pScreen->TranslateRegion)(&rgnDst, -dx, -dy);
    }    

    /* clip the shape of the dst to the destination composite clip */
    (*pScreen->Intersect)(&rgnDst, &rgnDst, gcPriv->pCompositeClip);

    numRects = REGION_NUM_RECTS(&rgnDst);
    if (numRects) {
	VoidProc  copyProc;
	int	  row;

	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						    sizeof(DDXPointRec)))) {
	    (*pScreen->RegionUninit)(&rgnDst);	    
	    return (RegionPtr)NULL;
	}
	pbox = REGION_RECTS(&rgnDst);
	ppt = pptSrc;
	for ( ; numRects != 0; numRects--, pbox++, ppt++) {
	     ppt->x = pbox->x1 + dx;
	     ppt->y = pbox->y1 + dy;
	}
    
	/* decide which depth copier to use */
	FFB_SELECTROW(pSrcDraw->depth, pSrcDraw->bitsPerPixel,
		      pDstDraw->bitsPerPixel, row);
    
	/* decide what flavor copier for this depth to use */
	if (pDstDraw->type == DRAWABLE_WINDOW 
	    || SCREENMEMORY((PixmapPtr)pDstDraw)) {
	    /* we know the hardware can handle rops, so we don't need */
	    /* special rop variants                                   */
	    ffbScreenPrivPtr    scrPriv;
	    FFB			ffb;

	    WRITE_MEMORY_BARRIER();
	    CHECKSTATE(pDstDraw->pScreen, pDstDraw, scrPriv, ffb, pGC);
	    if (pSrcDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pSrcDraw)) {

		copyProc = ffbCopyTab[row][_SCREEN_SCREEN];
	    } else { /* not screen to screen */
		copyProc = ffbCopyTab[row][_MEM_SCREEN];
	    }
	    (*copyProc)(pSrcDraw, pDstDraw, &rgnDst, pptSrc);
	} else {
	    /* No need to check state...destination is main memory. */
#ifdef MITR5
    	    ropDXop = &ropToDXop[pGC->alu];
    	    DXcopyRop = ropDXop->copyRop;
#endif	/* MITR5 */
	    if (pSrcDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pSrcDraw)) {

#ifdef MITR5
		if (DXcopyRop == DXcopy) {
		    ffbGCPrivPtr        ffbGCPriv;
		    CommandWord 	reqPlaneMask;
		    
		    switch (row) {
		     case _PACKED_TO_PACKED:
		     case _UNPACKED_TO_PACKED:
			reqPlaneMask = 0xffffffff;
			break;
		     case _THIRTYTWO_BITS_DEEP:
		     case _TWELVE_BITS_DEEP:
			reqPlaneMask = 0x00ffffff;
			break;
		     default:
			abort();
		    }
		    ffbGCPriv = FFBGCPRIV(pGC);

		    if (reqPlaneMask == ffbGCPriv->planemask) {
			copyProc = ffbCopyTab[row][DXcopy];
		    } else {
			copyProc = ffbCopyTab[row][DXcopySPM];
		    }
		} else {
		    copyProc = ffbCopyTab[row][DXcopyRop];
		}
		(*copyProc) (pSrcDraw, pDstDraw, &rgnDst, pptSrc,
			     ropDXop->andbits1, ropDXop->xorbits1,
			     ropDXop->andbits2, ropDXop->xorbits2);

#else /* not MITR5 */
                copyProc = ffbCopyTab[row][gcPriv->copyRop];
		(*copyProc)	(pSrcDraw, pDstDraw, &rgnDst, pptSrc,
				 gcPriv->andbits1, gcPriv->xorbits1,
				 gcPriv->andbits2, gcPriv->xorbits2);
#endif
	    } else {
		/* use cfb code */
#ifdef MITR5
		copyProc = 
		    copyAreaMemMem[pDstDraw->bitsPerPixel>8][DXcopyRop];
		(*copyProc)	(pSrcDraw, pDstDraw, GXcopy,
				&rgnDst, pptSrc, ~0L);
#else /* not MITR5 */
		copyProc = 
		    copyAreaMemMem[pDstDraw->bitsPerPixel>8)][gcPriv->copyRop];
		(*copyProc) (pSrcDraw, pDstDraw, &rgnDst, pptSrc,
			     gcPriv->andbits1, gcPriv->xorbits1,
			     gcPriv->andbits2, gcPriv->xorbits2);
#endif  /* MITR5 */
	    }
	}
	DEALLOCATE_LOCAL(pptSrc);

    }

    if (gcPriv->fExpose)
	prgnExposed = miHandleExposures(pSrcDraw, pDstDraw, pGC,
			  origSource.x, origSource.y,
			  (int) origSource.width, (int) origSource.height,
			  origDest.x, origDest.y, (unsigned long) 0);

    (*pScreen->RegionUninit)(&rgnDst);

    /*
     * Since there's latency between time we issue a command to move data
     * and the time the data is moved, we can't simply return.  For now
     * we ensure that the hardware is actually completed all operations.
     */
    FFBSYNC(FFBSCREENPRIV(pScreen)->ffb);	     

    return prgnExposed;
}

/* ffbPutImage -- public entry for the PutImage request
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
#ifdef FFB_DEPTH_INDEPENDENT
void
ffbPutImage(pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage)
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
	if(depth == 8)
            pFakePixmap = (PixmapPtr) cfbCreatePixmap(pDraw->pScreen, w+leftPad, h, 8);
	else
            pFakePixmap = (PixmapPtr) cfb32CreatePixmap(pDraw->pScreen, w+leftPad, h, 32);
	break;
    }
    if (!pFakePixmap) {
	ErrorF( "ffbPutImage can't make temp pixmap\n");
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

    /*
     * Since there's latency between time we issue a command to move data
     * and the time the data is moved, we can't simply return.  For now
     * we ensure that the hardware is actually completed all operations.
     */
    FFBSYNC(FFBSCREENPRIV(pGC->pScreen)->ffb);	     

}
#endif /* FFB_DEPTH_INDEPENDENT */
void ffbGetImage(pDraw, sx, sy, w, h, format, planeMask, pdstLine)
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
    FFB			ffb;
    VoidProc  		copyProc;
    int       		row;

    if ((w == 0) || (h == 0))
	return;
    if (pDraw->bitsPerPixel == 1) {
	mfbGetImage(pDraw, sx, sy, w, h, format, planeMask, pdstLine);
	return;
    }

    ffb = FFBSCREENPRIV(pDraw->pScreen)->ffb;
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

        /* decide which depth copier to use */
        FFB_SELECTROW(pDraw->depth, pDraw->bitsPerPixel,
                      FakePixmap.drawable.bitsPerPixel, row);

	switch (pDraw->depth) {
	    case 4:
/* ||| 		planeMask &= FFB_OVRLY_PLANEMASK; */
		planeMask &= 0xf;
		/* Fall through */
	    case 8:
		Pixel8ToPixelWord(planeMask);
		break;
	    case 12:
		planeMask &= FFB_12_BUF0_PLANEMASK;
		break;
	    case 24:
		planeMask &= FFB_24BIT_PLANEMASK;
		break;
	} /* switch */
	if (planeMask != FFBBUSALL1) {
	    /* ||| When we get a new scr->mem copier (maybe also new mem->mem
	       copier), don't bother zeroing dst, as planemask will be anded
	       with source data directly before writing dst.  (GetImage ZFormat
	       planemask doesn't have normal planemask semantics...0 bits in
	       mask just mean put 0 bits in destination.)
	       */
	    bzero((char *)pdstLine, FakePixmap.devKind * h);
	    if (pDraw->type == DRAWABLE_WINDOW 
		    || SCREENMEMORY((PixmapPtr)pDraw)) {
		/* pdraw is the source; getImage dest is always Mem */
#		define _COPY_SPM 1
		copyProc = ffbCopyTab[row][_COPY_SPM];
		(*copyProc)(pDraw, (DrawablePtr)&FakePixmap, &rgnDst, &ptSrc,
			    (PixelWord)0, (PixelWord)~planeMask,
			    (PixelWord)planeMask, (PixelWord)0);
	    } else {

                /* both src and dst in Mem */
#ifdef MITR5

/* DMC - Parameters have changed for R5 routines - this will require
   more extensive changes, but fix the parameters for now
*/
	    if (pDraw->bitsPerPixel == 8) {
		cfbDoBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, GXcopy,
		    &rgnDst, &ptSrc, planeMask);
	    } else {
		cfb32DoBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, GXcopy,
		    &rgnDst, &ptSrc, planeMask);
	    }
#else	/* MITR5 */
	    if (pDraw->bitsPerPixel == 8) {
		cfbBitbltCopySPM(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
		    &ptSrc, 0, ~planeMask, planeMask, 0);
	    } else {
		cfb32BitbltCopySPM(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
		    &ptSrc, 0, ~planeMask, planeMask, 0);
	    }
#endif	/* MITR5 */
	    }
	} else { 
	    /* plane mask is all 1's for ZPixmap */
	    if (pDraw->type == DRAWABLE_WINDOW 
		|| SCREENMEMORY((PixmapPtr)pDraw)) {
                /* pdraw is the source; getImage dest is always Mem */
#		define _COPY 0
		copyProc = ffbCopyTab[row][_COPY];
		(*copyProc)(pDraw, (DrawablePtr)&FakePixmap, &rgnDst, &ptSrc,
			    (PixelWord)0, (PixelWord)0, (PixelWord)-1, (PixelWord)0);
	    } else {

                /* both src and dst in Mem (ZPixmap and planemask all 1's) */
#ifdef MITR5
/* DMC - Parameters have changed for R5 routines - this will require
   more extensive changes, but fix the parameters for now
*/
/* ||| Should planeMask be ~0 at this point, which on 64-bit words could be
   different from FFBBUSALL1 */
		if (pDraw->bitsPerPixel == 8) {
		    cfbDoBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, GXcopy,
			&rgnDst, &ptSrc, planeMask);
		} else {
		    cfb32DoBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, GXcopy,
			&rgnDst, &ptSrc, planeMask);
		}
#else	/* MITR5 */
		if (pDraw->bitsPerPixel == 8) {
		    cfbBitbltCopy(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
			&ptSrc, 0, -1, 0, 0);
		} else {
		    cfb32BitbltCopy(pDraw, (DrawablePtr)&FakePixmap, &rgnDst,
			&ptSrc, 0, -1, 0, 0);
		}
#endif	/* MITR5 */
	    }
	}
	(*pDraw->pScreen->RegionUninit)(&rgnDst);
    } else { /* XYPixmap format */
	/* DIX has loop around this case to feed us a planemask with exactly
           one bit set.  Don't ask me why it doesn't let us do it. */

	FFBSYNC(ffb);
	if (pDraw->bitsPerPixel == 8) {
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
	    cfbCopyImagePlane (pDraw, (DrawablePtr)&FakePixmap, GXcopy, 
		&rgnDst, &ptSrc, planeMask);
	    (*pDraw->pScreen->RegionUninit)(&rgnDst);
	} else {
	    miGetImage(pDraw, sx, sy, w, h, format, planeMask, pdstLine);
	}
    }

    /*
     * Since there's latency between time we issue a command to move data
     * and the time the data is moved, we can't simply return.  For now
     * we ensure that the hardware is actually completed all operations.
     */
    FFBSYNC(ffb);

}

/*
 * HISTORY
 */
