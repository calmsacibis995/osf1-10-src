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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/*
Title:		xfbbanksw.c -- Bank switching module for TX
Author:		Edwin Goei
Created:	21 Mar 91

Purpose:
    Some machines like the 3max have an I/O address space limit which is
    smaller than the total I/O space of the TX board.  On these machines
    the TX card must be run in "bankswitched" mode to access all the
    memory on the board.  This module manages all of this.

Notes:
    For efficiency reasons, this module should not be called for machines
    that do not need bank switching, although it should still work.

    The implementation wraps both GC funcs and ops for 24-bit GCs in order
    to capture requests that draw into the 24-bit frame buffer.   In addition,
    it also wraps certain screen funcs that access the 24-bit frame buffer.

    The wrapper code here assumes that the last func to be called before any
    op will be ValidateGC.

    Window to window copies currently can use large pixmaps.  We should
    probably rewrite this to be more memory efficient.

Updates:
    17 Sep 91 (edg) = Added code to set the select plane of 8-bit
	StaticGray windows to depth 24.  All drawing is still done in 8 bits.
	This trick causes StaticGray windows to display using the decomposed
	hardware colormap.  See ropcolor.c for more info.
*/

#include "X.h"
#include "Xproto.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "xfbstruct.h"
#include "xfbbankswst.h"
#include "tfb.h"
#include "tfbdraw.h"
#include "fontstruct.h"
#include "cfb.h"

/* RasterOps screen regions
This stuff is RasterOps specific.  May want to put this in screen private
to generalize this mechanism for other hardware.
*/

#ifdef nomore

#define xfbBankLength 0x200000  /* length of a bank in bytes */
#define xfbBankRegionsN 3
static RegionPtr xfbBankRegions[xfbBankRegionsN];
/* List of rectangles that make up each region */
static int xfbBankRectsN[xfbBankRegionsN] = { 2, 3, 2 };
static xRectangle xfbBankRects[xfbBankRegionsN][3] = {
    { { 0, 0, 1280, 409 }, { 0, 409, 768, 1 }, { 0, 0, 0, 0 } },
    { { 768, 409, 512, 1 }, { 0, 410, 1280, 409 }, { 0, 819, 256, 1 } },
    { { 256, 819, 1024, 1 }, { 0, 820, 1280, 204 }, { 0, 0, 0, 0 } },
};

#endif nomore


#define xfbBankLength 0x200000  /* length of a bank in bytes */
#define xfbBankRegionsN 7
static RegionPtr xfbBankRegions[xfbBankRegionsN];
/* List of rectangles that make up each region */
static int xfbBankRectsN[xfbBankRegionsN] = { 1, 1, 1, 1, 1, 1, 1 };
static xRectangle xfbBankRects[xfbBankRegionsN][1] = {
    { { 0, 0, 1280, 409 } }, 
    { { 0, 409, 768, 1 } }, 
    { { 768, 409, 512, 1 } }, 
    { { 0, 410, 1280, 409 } }, 
    { { 0, 819, 256, 1 } },
    { { 256, 819, 1024, 1 } }, 
    { { 0, 820, 1280, 204 } }, 
};
static int xfbBanks[xfbBankRegionsN] = {0, 0, 1, 1, 1, 2, 2};

#define BANK_DEFAULT -1

/* Screen stuff */
static int  xfbBankScreenIndex;
static unsigned long xfbBankGeneration = 0;

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((xfbBankScreenPtr) \
    (pScreen)->devPrivates[xfbBankScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)


/* GC stuff */
static int  xfbBankGCIndex;

#define FUNC_PROLOGUE(pGC, pPriv) \
    ((pGC)->funcs = pPriv->wrapFuncs),\
    ((pGC)->ops = pPriv->wrapOps)

#define FUNC_EPILOGUE(pGC, pPriv) \
    ((pGC)->funcs = &xfbBankGCFuncs),\
    ((pGC)->ops = &xfbBankGCOps)

static void xfbBankValidateGC(), xfbBankChangeGC(), xfbBankCopyGC();
static void xfbBankDestroyGC(), xfbBankChangeClip(), xfbBankDestroyClip();
static void xfbBankCopyClip();

static GCFuncs xfbBankGCFuncs = {
    xfbBankValidateGC,
    xfbBankChangeGC,
    xfbBankCopyGC,
    xfbBankDestroyGC,
    xfbBankChangeClip,
    xfbBankDestroyClip,
    xfbBankCopyClip,
};

#define OP_INIT(pGC) \
    ScreenPtr pScreen = (pGC)->pScreen; \
    tfbPrivGCPtr pTfbPriv = (tfbPrivGCPtr) \
	(pGC)->devPrivates[tfbGCPrivateIndex].ptr; \
    xfbBankGCPtr pGCPriv = (xfbBankGCPtr) \
	(pGC)->devPrivates[xfbBankGCIndex].ptr; \
    RegionPtr pOrigCompositeClip; \
    RegionRec tmpReg; \
    int i; \
    GCFuncs *oldFuncs = (pGC)->funcs; \

#define OP_PROLOGUE(pGC) \
    (pGC)->ops = pGCPriv->wrapOps; \
    (pGC)->funcs = pGCPriv->wrapFuncs;

#define OP_EPILOGUE(pGC) \
    pGCPriv->wrapOps = (pGC)->ops; \
    (pGC)->ops = &xfbBankGCOps; \
    (pGC)->funcs = oldFuncs; \

#define OP_TOP_PART(pGC) \
    (*pScreen->RegionInit)(&tmpReg, NullBox, 0); \
    pOrigCompositeClip = pTfbPriv->pCompositeClip; \
    pTfbPriv->pCompositeClip = &tmpReg; \
    for (i = 0; i < xfbBankRegionsN; i++) { \
	(*pScreen->Intersect)(pTfbPriv->pCompositeClip, pOrigCompositeClip, \
	    xfbBankRegions[i]); \
	if (pScreen->RegionNotEmpty(&tmpReg)) { \
	    xfbBankSetBank(pScreen, i); \

#define OP_BOTTOM_PART(pGC) \
	} \
    } \
    xfbBankSetBank(pScreen, BANK_DEFAULT); \
    pTfbPriv->pCompositeClip = pOrigCompositeClip; \
    (*pScreen->RegionUninit)(&tmpReg); \

/* edg: may want to get rid of this macro */
#define OP_DEST_ONLY(pDrawable, pGC, statement) \
    if ((pDrawable)->type == DRAWABLE_WINDOW) { \
	OP_TOP_PART(pGC); \
	statement; \
	OP_BOTTOM_PART(pGC); \
    } else { \
	statement; \
    }

#define OP_SIMPLE(pDrawable, pGC, statement) \
    OP_INIT(pGC); \
    OP_PROLOGUE(pGC); \
    OP_DEST_ONLY(pDrawable, pGC, statement); \
    OP_EPILOGUE(pGC);

static void xfbBankFillSpans(), xfbBankSetSpans(), xfbBankPutImage();
static RegionPtr xfbBankCopyArea(), xfbBankCopyPlane();
static void xfbBankPolyPoint(), xfbBankPolylines(), xfbBankPolySegment();
static void xfbBankPolyRectangle(),xfbBankPolyArc(), xfbBankFillPolygon();
static void xfbBankPolyFillRect(), xfbBankPolyFillArc();
static int xfbBankPolyText8(), xfbBankPolyText16();
static void xfbBankImageText8(), xfbBankImageText16();
static void xfbBankImageGlyphBlt(),xfbBankPolyGlyphBlt();
static void xfbBankPushPixels(), xfbBankLineHelper();

static GCOps xfbBankGCOps = {
    xfbBankFillSpans,		xfbBankSetSpans,	xfbBankPutImage,	
    xfbBankCopyArea,		xfbBankCopyPlane,	xfbBankPolyPoint,
    xfbBankPolylines,		xfbBankPolySegment,	xfbBankPolyRectangle,
    xfbBankPolyArc,		xfbBankFillPolygon,	xfbBankPolyFillRect,
    xfbBankPolyFillArc,		xfbBankPolyText8,	xfbBankPolyText16,
    xfbBankImageText8,		xfbBankImageText16,	xfbBankImageGlyphBlt,
    xfbBankPolyGlyphBlt,	xfbBankPushPixels,	xfbBankLineHelper,
};


void
xfbBankSetBank(pScreen, bank)
    ScreenPtr	pScreen;
    int		bank;
{
#ifdef nomore
    ropSetBank(pScreen, bank);
    if (bank != BANK_DEFAULT) {
	tfbDrawSetFb24(pScreen,
	    ((xfbBankScreenPtr) pScreen->devPrivates[xfbBankScreenIndex].ptr)
	    ->mappedAdr - bank * xfbBankLength);
    }
#endif nomore
    ropSetBank(pScreen, xfbBanks[bank]);
    if (bank != BANK_DEFAULT) {
	tfbDrawSetFb24(pScreen,
	    ((xfbBankScreenPtr) pScreen->devPrivates[xfbBankScreenIndex].ptr)
	    ->mappedAdr - xfbBanks[bank] * xfbBankLength);
    }

}

/* Screen procs */

static Bool
xfbBankCloseScreen(i, pScreen)
    ScreenPtr	pScreen;
{
    xfbBankScreenPtr   pScreenPriv;

    pScreenPriv = (xfbBankScreenPtr)
	pScreen->devPrivates[xfbBankScreenIndex].ptr;

    xfbBankSetBank(pScreen, BANK_DEFAULT);
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->CreateGC = pScreenPriv->CreateGC;
    pScreen->PaintWindowBackground = pScreenPriv->PaintWindowBackground;
    pScreen->PaintWindowBorder = pScreenPriv->PaintWindowBorder;
    pScreen->CopyWindow = pScreenPriv->CopyWindow;

    xfree((pointer) pScreenPriv);
    return (*pScreen->CloseScreen)(i, pScreen);
}

#define RectToRegion(pScreen, prect, preg) { \
    BoxRec box; \
    box.x1 = (prect)->x; \
    box.y1 = (prect)->y; \
    box.x2 = box.x1 + (prect)->width; \
    box.y2 = box.y1 + (prect)->height; \
    (*(pScreen)->RegionInit)((preg), &box, 0); \
};

static void
xfbBankGetImage(pDrawable, sx, sy, w, h, format, planemask, pdstLine)
    DrawablePtr	    pDrawable;
    int		    sx, sy, w, h;
    unsigned int    format;
    unsigned long   planemask;
    pointer	    pdstLine;
{
    ScreenPtr pScreen = pDrawable->pScreen;
    int i, j;
    RegionRec tmpReg, srcReg;
    BoxRec box, *pbox;
    
    SCREEN_PROLOGUE(pScreen, GetImage);
    if (pDrawable->type == DRAWABLE_WINDOW && pDrawable->depth == 24) {
        box.x1 = pDrawable->x + sx;
        box.y1 = pDrawable->y + sy;
        box.x2 = box.x1 + w;
        box.y2 = box.y1 + h;
	(*pScreen->RegionInit)(&srcReg, &box, 0);
	for (i = 0; i < xfbBankRegionsN; i++) {
	    for (j = 0; j < xfbBankRectsN[i]; j++) {
		RectToRegion(pScreen, &xfbBankRects[i][j], &tmpReg);
		(*pScreen->Intersect)(&tmpReg, &tmpReg, &srcReg);
		if (pScreen->RegionNotEmpty(&tmpReg)) {
		    pbox = pScreen->RegionExtents(&tmpReg);
		    w = pbox->x2 - pbox->x1;
		    h = pbox->y2 - pbox->y1;
		    xfbBankSetBank(pScreen, i);
		    (*pScreen->GetImage)(pDrawable, pbox->x1 - pDrawable->x,
			pbox->y1 - pDrawable->y, w, h, format, planemask,
			pdstLine);
		    /* the following assumes alot about the pixmap format */
		    pdstLine += w * 4 * h;
		}
		(*pScreen->RegionUninit)(&tmpReg);
	    }
	}
	xfbBankSetBank(pScreen, BANK_DEFAULT);
	(*pScreen->RegionUninit)(&srcReg);
    } else {
	(*pScreen->GetImage)(pDrawable, sx, sy, w, h,
				  format, planemask, pdstLine);
    }
    SCREEN_EPILOGUE(pScreen, GetImage, xfbBankGetImage);
}

static void
xfbBankGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr	pDrawable;
    int		wMax;
    DDXPointPtr	ppt;
    int		*pwidth;
    int		nspans;
    unsigned int *pdstStart;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    
    SCREEN_PROLOGUE(pScreen, GetSpans);
    if (pDrawable->type == DRAWABLE_WINDOW && pDrawable->depth == 24) {
	/* The following code for bank switching 24-bit windows is RasterOps
	    specific.  Also, it does not assume spans are sorted.
	*/

#define InRegion0 \
    (ppt[k].y < 409 || ppt[k].y == 409 && ppt[k].x + pwidth[k] <= 768)
#define OnBorder01 \
    (ppt[k].y == 409 && ppt[k].x < 768 && ppt[k].x + pwidth[k] > 768)
#define InRegion1 \
    (ppt[k].y > 409 && ppt[k].y < 819 \
    || ppt[k].y == 819 && ppt[k].x + pwidth[k] <= 256 \
    || ppt[k].y == 409 && ppt[k].x >= 768)
#define OnBorder12 \
    (ppt[k].y == 819 && ppt[k].x < 256 && ppt[k].x + pwidth[k] > 256)
#define InRegion2 \
    (ppt[k].y > 819 || ppt[k].y == 819 && ppt[k].x >= 256)

	DDXPointRec pt2;
	int w1, w2, k, start, dstInc;

	for (k = 0; k < nspans;) {
	    if (InRegion0) {
		xfbBankSetBank(pScreen, 0);
		start = k;
		dstInc = 0;
		do {
		    dstInc += pwidth[k];
		    k++;
		} while (InRegion0 && k < nspans);
		(*pScreen->GetSpans)(pDrawable, wMax, &ppt[start],
		    &pwidth[start], k-start, pdstStart);
		pdstStart += dstInc;
	    } else if (OnBorder01) {
		/* get left half of span: */
		xfbBankSetBank(pScreen, 0);
		w1 = 768 - ppt[k].x;
		(*pScreen->GetSpans)(pDrawable, wMax, &ppt[k],
		    &w1, 1, pdstStart);
		pdstStart += w1;

		/* get right half of span: */
		xfbBankSetBank(pScreen, 1);
		pt2.y = ppt[k].y;
		pt2.x = 768;
		w2 = pwidth[k] - w1;
		(*pScreen->GetSpans)(pDrawable, wMax, &pt2,
		    w2, 1, pdstStart);
		pdstStart += w2;
		k++;
	    } else if (InRegion1) {
		xfbBankSetBank(pScreen, 1);
		start = k;
		dstInc = 0;
		do {
		    dstInc += pwidth[k];
		    k++;
		} while (InRegion1 && k < nspans);
		(*pScreen->GetSpans)(pDrawable, wMax, &ppt[start],
		    &pwidth[start], k-start, pdstStart);
		pdstStart += dstInc;
	    } else if (OnBorder12) {
		/* get left half of span: */
		xfbBankSetBank(pScreen, 1);
		w1 = 256 - ppt[k].x;
		(*pScreen->GetSpans)(pDrawable, wMax, &ppt[k],
		    &w1, 1, pdstStart);
		pdstStart += w1;

		/* get right half of span: */
		xfbBankSetBank(pScreen, 2);
		pt2.y = ppt[k].y;
		pt2.x = 256;
		w2 = pwidth[k] - w1;
		(*pScreen->GetSpans)(pDrawable, wMax, &pt2,
		    w2, 1, pdstStart);
		pdstStart += w2;
		k++;
	    } else {  /* span must be in region 2 */
		xfbBankSetBank(pScreen, 2);
		start = k;
		dstInc = 0;
		do {
		    dstInc += pwidth[k];
		    k++;
		} while (InRegion2 && k < nspans);
		(*pScreen->GetSpans)(pDrawable, wMax, &ppt[start],
		    &pwidth[start], k-start, pdstStart);
		pdstStart += dstInc;
	    }
	}
	xfbBankSetBank(pScreen, BANK_DEFAULT);
#undef InRegion0
#undef OnBorder01
#undef InRegion1
#undef OnBorder12
#undef InRegion2

    } else {
	(*pScreen->GetSpans)(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
    }
    SCREEN_EPILOGUE(pScreen, GetSpans, xfbBankGetSpans);
}

static Bool
xfbBankCreateGC(pGC)
    GCPtr   pGC;
{
    ScreenPtr	pScreen = pGC->pScreen;
    xfbBankGCPtr pGCPriv = (xfbBankGCPtr) pGC->devPrivates[xfbBankGCIndex].ptr;
    Bool	ret;

    SCREEN_PROLOGUE(pScreen, CreateGC);
    
    if ((ret = (*pScreen->CreateGC)(pGC)) && pGC->depth == 24) {
	/* wrap funcs and ops */
	pGCPriv->wrapFuncs = pGC->funcs;
	pGCPriv->wrapOps = pGC->ops;
	pGC->funcs = &xfbBankGCFuncs;
	pGC->ops = &xfbBankGCOps;
    }

    SCREEN_EPILOGUE(pScreen, CreateGC, xfbBankCreateGC);
    return (ret);
}

void
xfbBankPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    RegionRec tmpReg;
    int i;
    void (*paintWindow)();

    if (what == PW_BORDER) {
	SCREEN_PROLOGUE(pScreen, PaintWindowBorder);
	paintWindow = pScreen->PaintWindowBorder;
    } else {
	SCREEN_PROLOGUE(pScreen, PaintWindowBackground);
	paintWindow = pScreen->PaintWindowBackground;
    }
    if (pWin->drawable.depth == 24) {
	(*pScreen->RegionInit)(&tmpReg, NullBox, 0);
	for (i = 0; i < xfbBankRegionsN; i++) {
	    (*pScreen->Intersect)(&tmpReg, pRegion, xfbBankRegions[i]);
	    if (pScreen->RegionNotEmpty(&tmpReg)) {
		xfbBankSetBank(pScreen, i);
		(*paintWindow)(pWin, &tmpReg, what);
	    }
	}
	(*pScreen->RegionUninit)(&tmpReg);
	xfbBankSetBank(pScreen, BANK_DEFAULT);
    } else {
	(*paintWindow)(pWin, pRegion, what);
    }
    if (what == PW_BORDER) {
	SCREEN_EPILOGUE(pScreen, PaintWindowBorder, xfbBankPaintWindow);
    } else {
	SCREEN_EPILOGUE(pScreen, PaintWindowBackground, xfbBankPaintWindow);
    }
}

/*
Copy (x1,x2) from boxes in pRegion to ppt array, translating by (dx, dy)
*/
static void
MkBitbltPoints(ppt, pRegion, dx, dy)
    DDXPointPtr ppt;
    RegionPtr pRegion;
    int dx, dy;
{
    BoxPtr pbox;
    int i, nbox;

    pbox = REGION_RECTS(pRegion);
    nbox = REGION_NUM_RECTS(pRegion);
    for (i = nbox; --i >= 0; ppt++, pbox++) {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }
}

extern void tfbDoBitbltCopy();  /* look in tfbblt.c */
extern void tfbDoBitblt();  /* look in tfbblt.c */
extern WindowPtr *WindowTable;

/*
Do a 24 bit CopyWindow with bank switching.  Use a temporary pixmap and
copy the data twice.
*/
static void
xfbBankCopyWindow24(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DDXPointPtr pptBitbltSrc;
    int dx, dy;
    int i;
    WindowPtr pWinRoot= WindowTable[pScreen->myNum];
    PixmapPtr pPixmap;
    RegionRec tmpReg;
    BoxPtr pSrcBB, pDstBB;  /* src and dst region bounding boxes (extents) */

    /* make prgnSrc cover the region that we need to copy */
    dx = pWin->drawable.x - ptOldOrg.x;
    dy = pWin->drawable.y - ptOldOrg.y;
    (*pScreen->TranslateRegion)(prgnSrc, dx, dy);
    (*pScreen->Intersect)(prgnSrc, &pWin->borderClip, prgnSrc);
    (*pScreen->TranslateRegion)(prgnSrc, -dx, -dy);

    pSrcBB = (*pScreen->RegionExtents)(prgnSrc);
    pPixmap = (*pScreen->CreatePixmap)(pScreen, pSrcBB->x2 - pSrcBB->x1,
	pSrcBB->y2 - pSrcBB->y1, 24);
    (*pScreen->RegionInit)(&tmpReg, NullBox, 0);

    for (i = 0; i < xfbBankRegionsN; i++) {
	(*pScreen->Intersect)(&tmpReg, prgnSrc, xfbBankRegions[i]);
	if (pScreen->RegionNotEmpty(&tmpReg)) {
	    xfbBankSetBank(pScreen, i);
	    if (!(pptBitbltSrc = (DDXPointPtr) ALLOCATE_LOCAL(
		REGION_NUM_RECTS(&tmpReg) * sizeof(DDXPointRec)))) {
		(*pScreen->RegionUninit)(&tmpReg);
		(*pScreen->DestroyPixmap)(pPixmap);
		return;
	    }
	    MkBitbltPoints(pptBitbltSrc, &tmpReg, 0, 0);
	    (*pScreen->TranslateRegion)(&tmpReg, -pSrcBB->x1, -pSrcBB->y1);
	    tfbDoBitbltCopy((DrawablePtr) pWinRoot, (DrawablePtr) pPixmap,
		GXcopy, &tmpReg, pptBitbltSrc, ~0L);
	    DEALLOCATE_LOCAL(pptBitbltSrc);
	}
    }

    (*pScreen->TranslateRegion)(prgnSrc, dx, dy);
    /* now prgnSrc is really the destination region! */
    pDstBB = (*pScreen->RegionExtents)(prgnSrc);
    for (i = 0; i < xfbBankRegionsN; i++) {
	(*pScreen->Intersect)(&tmpReg, prgnSrc, xfbBankRegions[i]);
	if (pScreen->RegionNotEmpty(&tmpReg)) {
	    xfbBankSetBank(pScreen, i);
	    if (!(pptBitbltSrc = (DDXPointPtr) ALLOCATE_LOCAL(
		REGION_NUM_RECTS(&tmpReg) * sizeof(DDXPointRec)))) {
		(*pScreen->RegionUninit)(&tmpReg);
		(*pScreen->DestroyPixmap)(pPixmap);
		return;
	    }
	    MkBitbltPoints(pptBitbltSrc, &tmpReg, -pDstBB->x1, -pDstBB->y1);
	    tfbDoBitbltCopy((DrawablePtr) pPixmap, (DrawablePtr) pWinRoot,
		GXcopy, &tmpReg, pptBitbltSrc, ~0L);
	    DEALLOCATE_LOCAL(pptBitbltSrc);
	}
    }

    (*pScreen->RegionUninit)(&tmpReg);
    (*pScreen->DestroyPixmap)(pPixmap);
    xfbBankSetBank(pScreen, BANK_DEFAULT);
}

void 
xfbBankCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    int depth, hwCmap;

    SCREEN_PROLOGUE(pScreen, CopyWindow);

    xfbFindDepthAndHwCmap(pWin, &depth, &hwCmap);
    if (hwCmap != -1) {
	/* easy case: windows all have the same hardware cmap */
	ropSetSelect(pWin->drawable.pScreen, &pWin->borderClip, hwCmap);
    } else {
	xfbCopyWindowSetCmaps(pWin);
    }
    if (depth == 8) {
	/* no bank switching needed */
	cfbCopyWindow(pWin, ptOldOrg, prgnSrc);
    } else {
	/* do bank switched CopyWindow */
	xfbBankCopyWindow24(pWin, ptOldOrg, prgnSrc);
    }
    SCREEN_EPILOGUE(pScreen, CopyWindow, xfbBankCopyWindow);
}


/* GC funcs */

static void
xfbBankValidateGC(pGC, stateChanges, pDrawable)
    GCPtr   	  pGC;
    unsigned long stateChanges;
    DrawablePtr   pDrawable;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pGC->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ValidateGC)(pGC, stateChanges, pDrawable);

    /* save funcs and ops as Validate may have changed them */
    pPriv->wrapFuncs = pGC->funcs;
    pPriv->wrapOps = pGC->ops;

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbBankChangeGC(pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pGC->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ChangeGC)(pGC, mask);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbBankCopyGC(pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pGCDst->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pGCDst, pPriv);

    (*pGCDst->funcs->CopyGC)(pGCSrc, mask, pGCDst);

    FUNC_EPILOGUE(pGCDst, pPriv);
}

static void
xfbBankDestroyGC(pGC)
    GCPtr   pGC;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pGC->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->DestroyGC)(pGC);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbBankChangeClip(pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pGC->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbBankDestroyClip(pGC)
    GCPtr	pGC;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pGC->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->DestroyClip)(pGC);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbBankCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    xfbBankGCPtr pPriv = (xfbBankGCPtr) pgcDst->devPrivates[xfbBankGCIndex].ptr;

    FUNC_PROLOGUE(pgcDst, pPriv);

    (*pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    FUNC_EPILOGUE(pgcDst, pPriv);
}


/* Ops are below */

static void
xfbBankFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    OP_SIMPLE(pDrawable, pGC,
    (*pGC->ops->FillSpans)(pDrawable, pGC, nInit, pptInit,
	pwidthInit, fSorted));
}

static void
xfbBankSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			*psrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->SetSpans)(pDrawable, pGC, psrc, ppt, pwidth,
	nspans, fSorted));
}

static void
xfbBankPutImage(pDrawable, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int		  depth;
    int	    	  x;
    int	    	  y;
    int	    	  w;
    int	    	  h;
    int	    	  format;
    char    	  *pBits;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PutImage)(pDrawable, pGC,
	depth, x, y, w, h, leftPad, format, pBits));
}

/*
CopyArea for pixmap to window case is much like the other simple ops.
*/
static void
xfbBankCopyAreaPixWin(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    PixmapPtr	  pSrc;
    WindowPtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    OP_TOP_PART(pGC);
    /* copy area but don't generate any exposures */
    pTfbPriv->fExpose = FALSE;
    (void) (*pGC->ops->CopyArea)((DrawablePtr) pSrc, (DrawablePtr) pDst, pGC,
	srcx, srcy, w, h, dstx, dsty);
    pTfbPriv->fExpose = TRUE;
    OP_BOTTOM_PART(pGC);
    OP_EPILOGUE(pGC);
}

/*
rgnBlt could be improved by intersecting it with rgnDst, ie. compositeClip
*/
static void
xfbBankCopyAreaWinPix(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    WindowPtr	  pSrc;
    PixmapPtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    extern RegionPtr miHandleExposures();
    ScreenPtr pScreen = pGC->pScreen;
    BoxRec box;
    RegionRec rgnTmp, rgnSrc;
    RegionRec rgnBlt;		/* part of source region to copy */
    BoxPtr pBltBB;		/* rgnBlt bounding box (extents) */
    RegionPtr prgnSrcClip;	/* may be a new region, or just a pointer */
    Bool freeSrcClip = FALSE;
    int i;
    WindowPtr pWinRoot= WindowTable[pScreen->myNum];
    DDXPointPtr pptBitbltSrc;

    box.x1 = pSrc->drawable.x + srcx;
    box.y1 = pSrc->drawable.y + srcy;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;
    (*pScreen->RegionInit)(&rgnSrc, &box, 0);
    if (pGC->subWindowMode == IncludeInferiors) {
	prgnSrcClip = NotClippedByChildren(pSrc);
	freeSrcClip = TRUE;
    } else {
	prgnSrcClip = &pSrc->clipList;
    }
    (*pScreen->RegionInit)(&rgnBlt, NullBox, 0);
    (*pScreen->Intersect)(&rgnBlt, &rgnSrc, prgnSrcClip);
    (*pScreen->RegionUninit)(&rgnSrc);
    if (freeSrcClip)
	(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
    pBltBB = (*pScreen->RegionExtents)(&rgnBlt);

    (*pScreen->RegionInit)(&rgnTmp, NullBox, 0);
    for (i = 0; i < xfbBankRegionsN; i++) {
	(*pScreen->Intersect)(&rgnTmp, &rgnBlt, xfbBankRegions[i]);
	if (pScreen->RegionNotEmpty(&rgnTmp)) {
	    xfbBankSetBank(pScreen, i);
	    if (!(pptBitbltSrc = (DDXPointPtr) ALLOCATE_LOCAL(
		REGION_NUM_RECTS(&rgnTmp) * sizeof(DDXPointRec)))) {
		(*pScreen->RegionUninit)(&rgnTmp);
		(*pScreen->RegionUninit)(&rgnBlt);
		return;
	    }
	    MkBitbltPoints(pptBitbltSrc, &rgnTmp, 0, 0);
	    (*pScreen->TranslateRegion)(&rgnTmp, -box.x1, -box.y1);
	    tfbDoBitblt((DrawablePtr) pWinRoot, (DrawablePtr) pDst,
		pGC->alu, &rgnTmp, pptBitbltSrc, pGC->planemask);
	    DEALLOCATE_LOCAL(pptBitbltSrc);
	}
    }
    xfbBankSetBank(pScreen, BANK_DEFAULT);

    (*pScreen->RegionUninit)(&rgnTmp);
    (*pScreen->RegionUninit)(&rgnBlt);
}

/*
Similar to WinPix case, uses temporary pixmap and then calls PixWin case.
Could be much faster.
*/
static void
xfbBankCopyAreaWinWin(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    WindowPtr	  pSrc;
    WindowPtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    extern RegionPtr miHandleExposures();
    ScreenPtr pScreen = pGC->pScreen;
    BoxRec box;
    RegionRec rgnTmp, rgnSrc;
    RegionRec rgnBlt;		/* part of source region to copy */
    BoxPtr pBltBB;		/* rgnBlt bounding box (extents) */
    RegionPtr prgnSrcClip;	/* may be a new region, or just a pointer */
    Bool freeSrcClip = FALSE;
    int i;
    PixmapPtr pPixmap;
    WindowPtr pWinRoot= WindowTable[pScreen->myNum];
    DDXPointPtr pptBitbltSrc;

    box.x1 = pSrc->drawable.x + srcx;
    box.y1 = pSrc->drawable.y + srcy;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;
    (*pScreen->RegionInit)(&rgnSrc, &box, 0);
    if (pGC->subWindowMode == IncludeInferiors) {
	prgnSrcClip = NotClippedByChildren(pSrc);
	freeSrcClip = TRUE;
    } else {
	prgnSrcClip = &pSrc->clipList;
    }
    (*pScreen->RegionInit)(&rgnBlt, NullBox, 0);
    (*pScreen->Intersect)(&rgnBlt, &rgnSrc, prgnSrcClip);
    (*pScreen->RegionUninit)(&rgnSrc);
    if (freeSrcClip)
	(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
    pBltBB = (*pScreen->RegionExtents)(&rgnBlt);
    pPixmap = (*pScreen->CreatePixmap)(pScreen, pBltBB->x2 - pBltBB->x1,
	pBltBB->y2 - pBltBB->y1, 24);

    /* Essentially does the same thing as xfbBankCopyAreaWinPix() : */
    (*pScreen->RegionInit)(&rgnTmp, NullBox, 0);
    for (i = 0; i < xfbBankRegionsN; i++) {
	(*pScreen->Intersect)(&rgnTmp, &rgnBlt, xfbBankRegions[i]);
	if (pScreen->RegionNotEmpty(&rgnTmp)) {
	    xfbBankSetBank(pScreen, i);
	    if (!(pptBitbltSrc = (DDXPointPtr) ALLOCATE_LOCAL(
		REGION_NUM_RECTS(&rgnTmp) * sizeof(DDXPointRec)))) {
		(*pScreen->RegionUninit)(&rgnTmp);
		(*pScreen->DestroyPixmap)(pPixmap);
		(*pScreen->RegionUninit)(&rgnBlt);
		return;
	    }
	    MkBitbltPoints(pptBitbltSrc, &rgnTmp, 0, 0);
	    (*pScreen->TranslateRegion)(&rgnTmp, -pBltBB->x1, -pBltBB->y1);
	    tfbDoBitblt((DrawablePtr) pWinRoot, (DrawablePtr) pPixmap,
		pGC->alu, &rgnTmp, pptBitbltSrc, pGC->planemask);
	    DEALLOCATE_LOCAL(pptBitbltSrc);
	}
    }
    xfbBankSetBank(pScreen, BANK_DEFAULT);

    xfbBankCopyAreaPixWin(pPixmap, pDst, pGC, 0, 0,
	pBltBB->x2 - pBltBB->x1, pBltBB->y2 - pBltBB->y1,
	dstx + pBltBB->x1 - box.x1, dsty + pBltBB->y1 - box.y1);

    (*pScreen->RegionUninit)(&rgnTmp);
    (*pScreen->DestroyPixmap)(pPixmap);
    (*pScreen->RegionUninit)(&rgnBlt);
}

static RegionPtr
xfbBankCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    RegionPtr winExposed;

    if (pDst->type == DRAWABLE_WINDOW) {
	if (pSrc->type == DRAWABLE_WINDOW) {
	    extern int txDebugOption;
	    if (txDebugOption & 0x1) {
		winExposed = miCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h,
		    dstx, dsty);
		return (winExposed);
	    } else {
		xfbBankCopyAreaWinWin((WindowPtr) pSrc, (WindowPtr) pDst, pGC,
		    srcx, srcy, w, h, dstx, dsty);
	    }
	} else {
	    xfbBankCopyAreaPixWin((PixmapPtr) pSrc, pDst, pGC, srcx, srcy,
		w, h, dstx, dsty);
	}
    } else {
	if (pSrc->type == DRAWABLE_WINDOW) {
	    xfbBankCopyAreaWinPix((WindowPtr) pSrc, (PixmapPtr) pDst, pGC,
		srcx, srcy, w, h, dstx, dsty);
	} else {
	    /* simple, no bank switching, just call down */
	    OP_INIT(pGC);
	    OP_PROLOGUE(pGC);
	    winExposed = (*pGC->ops->CopyArea)(pSrc, pDst, pGC, srcx, srcy,
		w, h, dstx, dsty);
	    OP_EPILOGUE(pGC);
	    return (winExposed);
	}
    }
    return (miHandleExposures(pSrc, pDst, pGC, srcx, srcy, w, h,
	dstx, dsty, (unsigned long) 0));
}

static RegionPtr
xfbBankCopyPlane(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    register GC   *pGC;
    int     	  srcx,
		  srcy;
    int     	  w,
		  h;
    int     	  dstx,
		  dsty;
    unsigned long  plane;
{
    RegionPtr winExposed;

    if (pDst->type == DRAWABLE_WINDOW) {
	if (pSrc->type == DRAWABLE_WINDOW) {
	    winExposed = miCopyPlane(pSrc, pDst, pGC, srcx, srcy, w, h,
		dstx, dsty, plane);
	    return (winExposed);
	} else {
	    OP_INIT(pGC);
	    OP_PROLOGUE(pGC);
	    OP_TOP_PART(pGC);
	    /* try to avoid calling miHandleExposures() until later */
	    pTfbPriv->fExpose = FALSE;
	    (void) (*pGC->ops->CopyPlane)(pSrc, pDst, pGC, srcx, srcy, w, h,
		dstx, dsty, plane);
	    pTfbPriv->fExpose = TRUE;
	    OP_BOTTOM_PART(pGC);
	    OP_EPILOGUE(pGC);
	}
    } else {
	if (pSrc->type == DRAWABLE_WINDOW) {
	    winExposed = miCopyPlane(pSrc, pDst, pGC, srcx, srcy, w, h,
		dstx, dsty, plane);
	    return (winExposed);
	} else {
	    /* simple, no bank switching, just call down */
	    OP_INIT(pGC);
	    OP_PROLOGUE(pGC);
	    winExposed = (*pGC->ops->CopyPlane)(pSrc, pDst, pGC,
		srcx, srcy, w, h, dstx, dsty, plane);
	    OP_EPILOGUE(pGC);
	    return (winExposed);
	}
    }
    return (miHandleExposures(pSrc, pDst, pGC, srcx, srcy, w, h,
	dstx, dsty, plane));
}

static void
xfbBankPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PolyPoint)(pDrawable, pGC, mode, npt, pptInit));
}

static void
xfbBankPolylines(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode;
    int	    	  npt;
    DDXPointPtr	  pptInit;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->Polylines)(pDrawable, pGC, mode, npt, pptInit));

#if 0  /* this stuff is for debugging: */
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    if ((pDrawable)->type == DRAWABLE_WINDOW) {
	(*pScreen->RegionInit)(&tmpReg, NullBox, 0);
	pOrigCompositeClip = pTfbPriv->pCompositeClip;
	pTfbPriv->pCompositeClip = &tmpReg;
	for (i = 0; i < xfbBankRegionsN; i++) {
	    (*pScreen->Intersect)(pTfbPriv->pCompositeClip, pOrigCompositeClip, 
		xfbBankRegions[i]);
	    if (pScreen->RegionNotEmpty(&tmpReg)) {
		xfbBankSetBank(pScreen, i);
		(*pGC->ops->Polylines)(pDrawable, pGC, mode, npt, pptInit);
	    }
	}
	xfbBankSetBank(pScreen, BANK_DEFAULT);
	pTfbPriv->pCompositeClip = pOrigCompositeClip;
	(*pScreen->RegionUninit)(&tmpReg);
    } else {
	(*pGC->ops->Polylines)(pDrawable, pGC, mode, npt, pptInit);
    }
    OP_EPILOGUE(pGC);
#endif
}

static void
xfbBankPolySegment(pDrawable, pGC, nseg, pSegs)
    DrawablePtr pDrawable;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PolySegment)(pDrawable, pGC, nseg, pSegs));
}

static void
xfbBankPolyRectangle(pDrawable, pGC, nrects, pRects)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PolyRectangle)(pDrawable, pGC, nrects, pRects));
}

static void
xfbBankPolyArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PolyArc)(pDrawable, pGC, narcs, parcs));
}

/*
Assumes pPtsDst array is large enough to hold count objects.
*/
#if 1
static void
xfbBankCopyPts(count, pPtsSrc, pPtsDst)
    int		count; 		/* number of points to copy */
    DDXPointPtr	pPtsSrc;  	/* Pointer to src */
    DDXPointPtr	pPtsDst;  	/* Pointer to dst */
{
    while (count--) {
	*pPtsDst++ = *pPtsSrc++;
    }
}
#elif 0
#define xfbBankCopyPts(count, pPtsSrc, pPtsDst) { \
    int	n = count; \
    DDXPointPtr	pSrc = pPtsSrc; \
    DDXPointPtr	pDst = pPtsDst; \
    while (n--) { \
	*pDst++ = *pSrc++; \
    } \
}
#else
#define xfbBankCopyPts(count, pPtsSrc, pPtsDst) \
    bcopy(pPtsSrc, pPtsDst, count * sizeof(DDXPointRec));
#endif

/*
Note: we copy pPts here because tfbFillPolygon(), based on MIT cfb,
assumes that it only gets called once and so it translates the points
passed to it.  This breaks bank switching if a polygon occupies 2 or
more bank regions by causing translation to occur more than once.  Copying
fixes this.  XXX Should MIT cfb be fixed instead?
*/
static void
xfbBankFillPolygon(pDrawable, pGC, shape, mode, count, pPts)
    DrawablePtr		pDrawable;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    if ((pDrawable)->type == DRAWABLE_WINDOW) {
	DDXPointPtr pPtsCopy;

	/* make a copy of pPts because tfbFillPolygon() changes it */
	pPtsCopy = (DDXPointPtr) xalloc(sizeof(DDXPointRec) * count);
	if (!pPtsCopy) {
	    OP_EPILOGUE(pGC);
	    return;
	}
	(*pScreen->RegionInit)(&tmpReg, NullBox, 0);
	pOrigCompositeClip = pTfbPriv->pCompositeClip;
	pTfbPriv->pCompositeClip = &tmpReg;
	for (i = 0; i < xfbBankRegionsN; i++) {
	    (*pScreen->Intersect)(pTfbPriv->pCompositeClip, pOrigCompositeClip, 
		xfbBankRegions[i]);
	    if (pScreen->RegionNotEmpty(&tmpReg)) {
		xfbBankCopyPts(count, pPts, pPtsCopy);
		xfbBankSetBank(pScreen, i);
		(*pGC->ops->FillPolygon)(pDrawable, pGC, shape, mode, count,
		    pPtsCopy);
	    }
	}
	xfbBankSetBank(pScreen, BANK_DEFAULT);
	pTfbPriv->pCompositeClip = pOrigCompositeClip;
	(*pScreen->RegionUninit)(&tmpReg);
	xfree((pointer) pPtsCopy);
    } else {
	(*pGC->ops->FillPolygon)(pDrawable, pGC, shape, mode, count, pPts);
    }
    OP_EPILOGUE(pGC);
}

/*
Assumes prectDst array is large enough to hold nrect rectangles.
*/
static void
xfbBankCopyRects(nrect, prectSrc, prectDst)
    int		nrect; 		/* number of rectangles to copy */
    xRectangle	*prectSrc;  	/* Pointer to src */
    xRectangle	*prectDst;  	/* Pointer to dst */
{
    while (nrect--) {
	*prectDst++ = *prectSrc++;
    }
}

/*
Note: we copy prectInit here because tfbPolyFillRect(), based on MIT cfb,
assumes that it only gets called once and so it translates the rectangles
passed to it.  This breaks bank switching if a rectangle occupies 2 or
more bank regions by causing translation to occur more than once.  Copying
fixes this.  XXX Should MIT cfb be fixed instead?
*/
static void
xfbBankPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    if (pDrawable->type == DRAWABLE_WINDOW) {
	xRectangle *prectCopy;

	/* make a copy of prectInit because tfbPolyFillRect() changes it */
	prectCopy = (xRectangle *) xalloc(sizeof(xRectangle) * nrectFill);
	if (!prectCopy) {
	    OP_EPILOGUE(pGC);
	    return;
	}
	(*pScreen->RegionInit)(&tmpReg, NullBox, 0);
	pOrigCompositeClip = pTfbPriv->pCompositeClip;
	pTfbPriv->pCompositeClip = &tmpReg;
	for (i = 0; i < xfbBankRegionsN; i++) {
	    (*pScreen->Intersect)(pTfbPriv->pCompositeClip, pOrigCompositeClip, 
		xfbBankRegions[i]);
	    if (pScreen->RegionNotEmpty(&tmpReg)) {
		xfbBankCopyRects(nrectFill, prectInit, prectCopy);
		xfbBankSetBank(pScreen, i);
		(*pGC->ops->PolyFillRect)(pDrawable, pGC, nrectFill, prectCopy);
	    }
	}
	xfbBankSetBank(pScreen, BANK_DEFAULT);
	pTfbPriv->pCompositeClip = pOrigCompositeClip;
	(*pScreen->RegionUninit)(&tmpReg);
	xfree((pointer) prectCopy);
    } else {
	(*pGC->ops->PolyFillRect)(pDrawable, pGC, nrectFill, prectInit);
    }
    OP_EPILOGUE(pGC);
}

static void
xfbBankPolyFillArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PolyFillArc)(pDrawable, pGC, narcs, parcs));
}

/*
Note: the value returned by calling down should be exactly the same each
time so we just return the result of the last call down.
*/
static int
xfbBankPolyText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    int result;
    OP_SIMPLE(pDrawable, pGC,
	result = (*pGC->ops->PolyText8)(pDrawable, pGC, x, y, count, chars));
    return (result);
}

/*
Note: the value returned by calling down should be exactly the same each
time so we just return the result of the last call down.
*/
static int
xfbBankPolyText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    int	result;
    OP_SIMPLE(pDrawable, pGC,
	result = (*pGC->ops->PolyText16)(pDrawable, pGC, x, y, count, chars));
    return (result);
}

static void
xfbBankImageText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
#if 0
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->ImageText8)(pDrawable, pGC, x, y, count, chars));
#endif

    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    if ((pDrawable)->type == DRAWABLE_WINDOW) {
	(*pScreen->RegionInit)(&tmpReg, NullBox, 0);
	pOrigCompositeClip = pTfbPriv->pCompositeClip;
	pTfbPriv->pCompositeClip = &tmpReg;
	for (i = 0; i < xfbBankRegionsN; i++) {
	    (*pScreen->Intersect)(pTfbPriv->pCompositeClip, pOrigCompositeClip, 
		xfbBankRegions[i]);
	    if (pScreen->RegionNotEmpty(&tmpReg)) {
		xfbBankSetBank(pScreen, i);
		(*pGC->ops->ImageText8)(pDrawable, pGC, x, y, count, chars);
	    }
	}
	xfbBankSetBank(pScreen, BANK_DEFAULT);
	pTfbPriv->pCompositeClip = pOrigCompositeClip;
	(*pScreen->RegionUninit)(&tmpReg);
    } else {
	(*pGC->ops->ImageText8)(pDrawable, pGC, x, y, count, chars);
    }
    OP_EPILOGUE(pGC);
}

static void
xfbBankImageText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->ImageText16)(pDrawable, pGC, x, y, count, chars));
}

static void
xfbBankImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->ImageGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci,
	pglyphBase));
}

static void
xfbBankPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    OP_SIMPLE(pDrawable, pGC,
	(*pGC->ops->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph,
	    ppci, pglyphBase));
}

static void
xfbBankPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
    OP_SIMPLE(pDst, pGC,
	(*pGC->ops->PushPixels)(pGC, pBitMap, pDst, w, h, x, y));
}

static void
xfbBankLineHelper()
{
    FatalError("xfbBankLineHelper called\n");
}


/* Initialization */

/*
Inputs: mappedAdr = address of where bank switched memory is mapped to
*/
Bool
xfbBankSwitchInit(pScreen, mappedAdr)
    ScreenPtr	pScreen;
    pointer	mappedAdr;
{
    int i;
    xfbBankScreenPtr    pScreenPriv;

    if (xfbBankGeneration != serverGeneration) {
	xfbBankScreenIndex = AllocateScreenPrivateIndex();
	if (xfbBankScreenIndex < 0)
	    return (FALSE);
	xfbBankGCIndex = AllocateGCPrivateIndex();
	xfbBankGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, xfbBankGCIndex, sizeof(xfbBankGCRec)))
	return (FALSE);
    pScreenPriv = (xfbBankScreenPtr) xalloc(sizeof(xfbBankScreenRec));
    if (!pScreenPriv)
	return (FALSE);

    pScreenPriv->mappedAdr = mappedAdr;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->GetImage = pScreen->GetImage;
    pScreenPriv->GetSpans = pScreen->GetSpans;
    pScreenPriv->CreateGC = pScreen->CreateGC;
    pScreenPriv->PaintWindowBackground = pScreen->PaintWindowBackground;
    pScreenPriv->PaintWindowBorder = pScreen->PaintWindowBorder;
    pScreenPriv->CopyWindow = pScreen->CopyWindow;

    pScreen->CloseScreen = xfbBankCloseScreen;
    pScreen->GetImage = xfbBankGetImage;
    pScreen->GetSpans = xfbBankGetSpans;
    pScreen->CreateGC = xfbBankCreateGC;
    pScreen->PaintWindowBackground = xfbBankPaintWindow;
    pScreen->PaintWindowBorder = xfbBankPaintWindow;
    pScreen->CopyWindow = xfbBankCopyWindow;

    pScreen->devPrivates[xfbBankScreenIndex].ptr = (pointer) pScreenPriv;

    /* initialize clip regions for each bank */
    for (i = 0; i < xfbBankRegionsN; i++) {
	xfbBankRegions[i] = (*pScreen->RectsToRegion)(xfbBankRectsN[i],
	    xfbBankRects[i], CT_UNSORTED);
    }
    xfbBankSetBank(pScreen, BANK_DEFAULT);
    return (TRUE);
}
