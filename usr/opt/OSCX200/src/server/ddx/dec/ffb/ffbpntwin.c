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
static char *rcsid = "@(#)$RCSfile: ffbpntwin.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:13:24 $";
#endif
/*
 */

#include "X.h"
#include "Xprotostr.h"

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"

#include "ffb.h"
#include "ffbpntwin.h"
#include "ffbpntarea.h"
#include "cfbmskbits.h"

#include "ffbcopy.h"

#if FFBPIXELBITS==32 && defined(MCMAP)
#include "../cmap/cmap.h"
#include "../cmap/bt463.h"
#endif

extern long ffbWindowID(WindowPtr pWin);


static void ffbGenFillSolid(pWin, pRegion, srcpix, planemask, depth)
    WindowPtr       pWin;
    RegionPtr       pRegion;
    PixelWord	    srcpix;
    PixelWord	    planemask;
    int		    depth;
{
    int		    nbox;
    BoxPtr	    pbox;
    register int    x, y;
    register int    i;
    Pixel8	    *pdstBase, *pdst, *p;
    int		    dstwidth;
    register int    width, w;   /* width of current rect		     */
    register int    h;		/* height of current rect		     */
    register int    align;	/* alignment of pdst 0..FFBSTIPPLEALIGNMASK  */
    CommandWord	    ones = ffbStippleAll1;
    CommandWord	    mask, leftMask, rightMask;
    int		    m;
    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;

    nbox = REGION_NUM_RECTS(pRegion);
    if (nbox == 0) return;

    /* Set up ffb state */
    scrPriv = FFBSCREENPRIV(pWin->drawable.pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    ffb = scrPriv->ffb; 
    WRITE_MEMORY_BARRIER();
    FFBMODE(ffb, BLOCKFILL);
    if (depth == 8){
        FFBROP(ffb, GXcopy,  ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
	       PACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
    } else {
        FFBROP(ffb, GXcopy,  ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
	       TWENTYFOUR_BIT_DEST << DST_VISUAL_SHIFT);
    }

    FFBDATA(ffb, ~0);
    FFBLOADCOLORREGS(ffb, srcpix, depth);
    CYCLE_REGS(ffb);
    FFBPLANEMASK(scrPriv, ffb, planemask);

    pbox = REGION_RECTS(pRegion);

    pdstBase = (Pixel8 *)
	(((PixmapPtr)(pWin->drawable.pScreen->devPrivate))->devPrivate.ptr);
    dstwidth =(int)
	(((PixmapPtr)(pWin->drawable.pScreen->devPrivate))->devKind);

    do { /* For each region box */
	x = pbox->x1;
	y = pbox->y1;
	width = pbox->x2 - x;
	h = pbox->y2 - y;
	if (width == 0 || h == 0) {
	    ErrorF("Who the hell passed a 0-area rectangle to pntwin?\n");
	    abort();
	}
#ifdef MITR5
	/* ||| This shouldn't be necessary if everyone obeys the damn rules */
	if ( width && h ) {
#endif
	pdstBase = CYCLE_FB(pdstBase);
	pdst = pdstBase + y*dstwidth + x * FFBPIXELBYTES;

#define DUMPRECTANGLES  

#if defined(DUMPRECTANGLES) && defined(SOFTWARE_MODEL)
	(void) FillMemory(pdst, width, h, dstwidth, srcpix, planemask);
#else
        /* We only need to flush on this outer loop since the inner loops
         * always advance to the next scan line and are thus non-interferring
         * with themselves. It's only when the last box leaves off where
         * the next is starting that we have a concern. We don't need
         * to flush all the time, so keep this pointer around. We also
         * should not need to flush coming into this routine since the
         * FFBMODE should be flushing the write buffer to make sure that
         * the mode register is set when the writes start (remember that the
         * write buffer to io system writes are not ordered.. )
         */

	/* ||| This flush check stuff should be replaced by aliasing as
           needed, like after each rectangle. */
	align = (long)pdst & FFBBUSBYTESMASK;
	pdst -= align;
	if (width <= FFBMAXBSWPIXELS ) {
	    /* One block fill command per scan line */
	    mask = FFBLOADBLOCKDATA(align, width);
	    while (h != 0) {
		FFBWRITE(pdst, mask);
		pdst += dstwidth;
		h -= 1;
	    };
	} else {
	    /* Mask requires multiple words */
	    mask = FFBLOADBLOCKDATA(align, FFBMAXBSWPIXELS);
	    do {
		p = pdst;
		w = width;
		while( w  > FFBMAXBSWPIXELS) {
		    FFBWRITE(p, mask);
		    p += FFBMAXBSWPIXELS * FFBPIXELBYTES;
		    w -= FFBMAXBSWPIXELS;
		}
		FFBWRITE(p, FFBLOADBLOCKDATA(align, w));
		pdst += dstwidth;
		h--;
	    } while (h !=0);
	} /* if skinny else fat rectangle */
#endif /* DUMPRECTANGLES && SOFTWARE_MODEL */
#ifdef MITR5
 	}
#endif
	nbox--;
	pbox++;
    } while (nbox != 0);
}


static void ffbFillBoxSolid(pWin, pRegion, srcpix, clearOverlays)
    WindowPtr       pWin;
    RegionPtr       pRegion;
    PixelWord	    srcpix;
    Bool    	    clearOverlays;
{
    unsigned int    planemask;
    ffbBufDesc      *bdptr;

#if FFBPIXELBITS == 8
    Pixel8ToPixelWord(srcpix);
    ffbGenFillSolid(pWin, pRegion, srcpix, FFBBUSALL1, FFBPIXELBITS);
#else
    /* Always paint window background AND window id bits.  After all, we
       can do both pretty much for free. */
    bdptr = FFBBUFDESCRIPTOR(pWin);
    planemask = bdptr->planemask | WID_MASK;

    switch (pWin->drawable.depth) {
     case 4:
	srcpix = (srcpix << 24) & 0x0f000000;
	break;
     case 8:
	srcpix <<= (bdptr->wid >> 25);	/* ||| */
	srcpix &= 0x00ffffff;
	break;
     case 12:
	srcpix >>= ((bdptr->wid >> 28) & 0x4); /* ||| */
     case 24:
     case 32:
	srcpix &= 0x00ffffff;
     default:
	break;
    }

#ifdef MCMAP
    if (!FFBISOVERLAYWINDOW(pWin))
    {
	srcpix |= ffbWindowID(pWin);
	
	if (CMAPWinDevPriv(pWin)->windowResized) {
	    CMAPWinDevPriv(pWin)->windowResized = FALSE;
	}
	else if (clearOverlays) {
	    planemask |= FFB_OVRLY_PLANES;
	}
    }
#else
    srcpix |= ffbWindowID(pWin);
#endif

    ffbGenFillSolid(pWin, pRegion, srcpix, planemask, FFBPIXELBITS);
#endif
}
    


#if FFBPIXELBITS == 32
void ffb32PaintWindowID(WindowPtr pWin, RegionPtr pRegion, Bool clearOverlays)
{
    long mask;

    /*
    ** if this is an overlay window then return. We should not modify
    ** the pixel's window type for overlay windows. It should retain
    ** the window type field of the window that was created in the
    ** RGB planes.
    */
    if (FFBISOVERLAYWINDOW(pWin))
	return;

#ifdef MCMAP
    mask = 0;
    if (CMAPWinDevPriv(pWin)->windowResized) {
	CMAPWinDevPriv(pWin)->windowResized = FALSE;
    }
    else if (clearOverlays) {
	mask = FFB_OVRLY_PLANES;
    }
#endif

    ffbGenFillSolid(pWin, pRegion, ffbWindowID(pWin), WID_MASK | mask, FFBPIXELBITS);
}


void ffb32ClearOverlays(WindowPtr pWin, RegionPtr pRegion)
{
    ffbGenFillSolid(pWin, pRegion, 0, 0x0f000000, FFBPIXELBITS);
}
#endif



static GC fakeGC;

/* Tile window or border */
static void ffbFillBoxTile(pWin, pRegion, itile, clearOverlays)
    WindowPtr   pWin;
    RegionPtr   pRegion;
    PixmapPtr   itile;
    Bool    	clearOverlays;
{
    int		    nbox;	    /* number of boxes to fill */
    register BoxPtr pbox;	    /* pointer to list of boxes to fill */
    long	    x, y, autoWidMask, overlayMask = 0;
    int		    row;
    GCPtr	    pGC;
    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;
    VoidProc	    ffbTileProc;
    PixmapPtr       tile;

    nbox = REGION_NUM_RECTS(pRegion);
    if (nbox == 0) return;

    pbox = REGION_RECTS(pRegion);

#if FFBPIXELBITS==32 && defined(MCMAP)
    if (!FFBISOVERLAYWINDOW(pWin))
    {
	if (CMAPWinDevPriv(pWin)->windowResized) {
	    CMAPWinDevPriv(pWin)->windowResized = FALSE;
	}
	else if (clearOverlays) {
	    overlayMask = FFB_OVRLY_PLANES;
	}
    }
#endif

    /* Load info into fake GC. */
    pGC = &fakeGC;
    tile = itile;

    /* Figure out which painting routine to use */
    switch (tile->drawable.width) {
     case FFBBLOCKTILEPIXELS:
	/*
	 * use color registers and BLOCKFILL, 8-pixel patterns...
	 */
	ffbTileProc = ffbTileFillAreaWord;
	autoWidMask = WID_MASK;
	break;
     default:
	tile = (PixmapPtr) ffbExpandTile(tile);
	if (tile == (PixmapPtr)NULL) {
	    int	row;

	    tile = itile;
	    FFB_SELECTROW(tile->drawable.depth, tile->drawable.bitsPerPixel,
			  pWin->drawable.bitsPerPixel, row);
	    ffbTileProc = ffbCopyTab[row][_TILE_FILL_AREA];
	    autoWidMask = 0;
#	    if FFBPIXELBITS==32
	    ffb32PaintWindowID(pWin, pRegion, clearOverlays);
#	    endif
	} else {
	    ffbTileProc = ffbTileFillAreaWord;
	    autoWidMask = WID_MASK;
	}
    }

    /* Set up ffb state */
    scrPriv = FFBSCREENPRIV(pWin->drawable.pScreen);
    ffb = scrPriv->ffb;

#   if FFBPIXELBITS==32
    switch(pWin->drawable.depth) {
     case 8:
	if (autoWidMask)			/* ha! */
	    ffb32PaintWindowID(pWin, pRegion, clearOverlays);
	WRITE_MEMORY_BARRIER();
	pGC->planemask = FFB_8U_PLANEMASK;
	FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
	       UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
	break;
     case 12:
	WRITE_MEMORY_BARRIER();
	pGC->planemask = FFB_12_BUF0_PLANEMASK | autoWidMask | overlayMask;
	FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
	       TWELVE_BIT_DEST << DST_VISUAL_SHIFT);
	break;
     case 24:
	WRITE_MEMORY_BARRIER();
	pGC->planemask = FFB_24BIT_PLANEMASK | autoWidMask | overlayMask;
	FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
	       TWENTYFOUR_BIT_DEST << DST_VISUAL_SHIFT);
	break;
     default:
	abort();
    }
#   else /*8*/
    WRITE_MEMORY_BARRIER();
    pGC->planemask = FFB_8P_PLANEMASK;
    FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
	   PACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
#   endif /*32*/
    pGC->alu = GXcopy;
    pGC->tile.pixmap = tile;
    pGC->patOrg.x = 0;
    pGC->patOrg.y = 0;
    
    scrPriv->lastGC = pGC;

    CYCLE_REGS(ffb);
    FFBPLANEMASK(scrPriv, ffb, pGC->planemask);
    scrPriv->ffb = ffb;
    
    /* Turn screen-relative boxes into window-relative rectangles, and paint. */
    if (nbox == 1) {
	xRectangle      rect;

	x = pbox->x1;
	rect.x = x - pWin->drawable.x;
	y = pbox->y1;
	rect.y = y - pWin->drawable.y;
	rect.width = pbox->x2 - x;
	rect.height = pbox->y2 - y;
	(*ffbTileProc)((DrawablePtr)pWin, 1, &rect, pGC);
    } else {
	xRectangle      *xrects, *xr;
	int		i;
	int		wx, wy;

	xrects = (xRectangle *) ALLOCATE_LOCAL(nbox * sizeof(xRectangle));
	if (!xrects) return;
    
	wx = pWin->drawable.x;
	wy = pWin->drawable.y;
	for (i = nbox, xr = xrects; i != 0; i--, xr++, pbox++) {
	    x = pbox->x1;
	    xr->x = x - wx;
	    y = pbox->y1;
	    xr->y = y - wy;
	    xr->width = pbox->x2 - x;
	    xr->height = pbox->y2 - y;
	}
	(*ffbTileProc)((DrawablePtr)pWin, nbox, xrects, pGC);
	DEALLOCATE_LOCAL(xrects);
    }
    if (tile != itile) {
	CFB_NAME(DestroyPixmap)(tile);
    }
}


void ffbPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
/* #define CHEAT */  
#if defined(CHEAT) && defined(SOFTWARE_MODEL)
    void MakeIdle();
    MakeIdle();
    CFB_NAME(PaintWindow)(pWin, pRegion, what);
    return;
#endif CHEAT && SOFTWARE_MODEL

    switch (what) {
    case PW_BACKGROUND:
	switch (pWin->backgroundState) {
	case None:
	    return;

	case ParentRelative:
	    do {
		pWin = pWin->parent;
	    } while (pWin->backgroundState == ParentRelative);
	    (*pWin->drawable.pScreen->PaintWindowBackground)
		(pWin, pRegion, what);
	    return;

	case BackgroundPixmap:
	    ffbFillBoxTile((DrawablePtr)pWin, pRegion, pWin->background.pixmap, TRUE);
	    return;

	case BackgroundPixel:
	    ffbFillBoxSolid((DrawablePtr)pWin, pRegion, pWin->background.pixel, TRUE);
	    return;
    	}
    	break;

    case PW_BORDER:
	if (pWin->borderIsPixel) {
	    ffbFillBoxSolid ((DrawablePtr)pWin, pRegion, pWin->border.pixel, TRUE);
	} else {
	    /* The border tile origin is always the same as the background tile
	       origin. */
	    WindowPtr pWinOrigin;
	    for (pWinOrigin = pWin;
		 pWinOrigin->backgroundState == ParentRelative;
		 pWinOrigin = pWinOrigin->parent) {};
	    
	    ffbFillBoxTile((DrawablePtr)pWinOrigin, pRegion,
			   pWin->border.pixmap, TRUE);
	}
	return;
    }
}



#if FFBPIXELBITS==32 && defined(MCMAP)
void ffb32ClearToBackground(WindowPtr pWin, RegionPtr pRegion)
{
    /* inline: ffbPaintWindow */
    switch (pWin->backgroundState) {
	    
     case BackgroundPixmap:
	ffbFillBoxTile((DrawablePtr)pWin, pRegion, pWin->background.pixmap, FALSE);
	break;
	    
     case BackgroundPixel:
	ffbFillBoxSolid((DrawablePtr)pWin, pRegion, pWin->background.pixel, FALSE);
    }
}
#endif

/*
 * HISTORY
 */
