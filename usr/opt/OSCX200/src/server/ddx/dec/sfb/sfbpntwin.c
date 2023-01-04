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
#define IDENT "X-2"
#define MODULE_NAME SFBPNTWIN
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
****************************************************************************/
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel 
**
**--
**/

#include "X.h"
#include "Xprotostr.h"

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"

#include "sfb.h"
#include "sfbpntwin.h"
#include "sfbpntarea.h"
#include "cfbmskbits.h"

static void sfbFillBoxSolid(pWin, pRegion, srcpix)
    WindowPtr       pWin;
    RegionPtr       pRegion;
    PixelWord	    srcpix;
{
    int		    nbox;
    BoxPtr	    pbox;
    register int    x, y;
    register int    i;
    Pixel8	    *pdstBase, *pdst, *p;
    int		    dstwidth;
    register int    width, w;   /* width of current rect		    */
    register int    h;		/* height of current rect		    */
    register int    align;	/* alignment of pdst (0..SFBALIGNMASK)	    */
    CommandWord	    ones = sfbStippleAll1;
    CommandWord	    mask, leftMask, rightMask;
    int		    m;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;
    GCPtr	    pGC;

    nbox = REGION_NUM_RECTS(pRegion);
    pbox = REGION_RECTS(pRegion);
    if (nbox == 0) return;

    pdstBase = (Pixel8 *)
	(((PixmapPtr)(pWin->drawable.pScreen->devPrivate))->devPrivate.ptr);
    dstwidth =(int)(((PixmapPtr)(pWin->drawable.pScreen->devPrivate))->devKind);

    /* Set up sfb state */
    scrPriv = SFBSCREENPRIV(pWin->drawable.pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    sfb = scrPriv->sfb;
    SFBPLANEMASK(sfb, SFBBUSALL1);
    SFBROP(sfb, GXcopy);
    PixelToPixelWord(srcpix);
    SFBFOREGROUND(sfb, srcpix);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    do { /* For each region box */
	x = pbox->x1;
	y = pbox->y1;
	width = pbox->x2 - x;
	h = pbox++->y2 - y;
	if ( !(width && h) ) continue;
	pdstBase = (Pixel8 *)CYCLE_FB(pdstBase);
	pdst = pdstBase + y*dstwidth + x * SFBPIXELBYTES;
#define DUMPRECTANGLES
#if defined(DUMPRECTANGLES) && defined(SOFTWARE_MODEL)
# ifdef SLEAZOID32
	(void) FillMemory(pdst, width, h, dstwidth, ExpandPixel(srcpix));
# else
	(void) FillMemory(pdst, width, h, dstwidth, srcpix);
# endif
#else
	align = (int)pdst & SFBALIGNMASK;
	pdst -= align;
	SFBBYTESTOPIXELS(align);
	width += align;
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(width, ones);
    
	if (width <= SFBSTIPPLEBITS) {
	    /* Mask fits in one word */
	    mask = leftMask & rightMask;
	    do {
		SFBWRITEONEWORD(sfb, pdst, mask);
		pdst += dstwidth;
		h--;
	    } while (h != 0);
	} else {
	    /* Mask requires multiple words */
	    do {
		p = pdst;
		SFBWRITE(p, leftMask);
		for (m = width - 2*SFBSTIPPLEBITS; m > 0; m -= SFBSTIPPLEBITS) {
		    p += SFBSTIPPLEBYTESDONE;
		    SFBWRITE(p, ones);
		}
		SFBWRITE(p+SFBSTIPPLEBYTESDONE, rightMask);
		pdst += dstwidth;
		pdst = CYCLE_FB(pdst);
		h--;
	    } while (h !=0);
	}
#endif /* DUMPRECTANGLES && SOFTWARE_MODEL */

    } while (--nbox > 0);
}

static GC fakeGC;

/* Tile window or border */
static void sfbFillBoxTile(pWin, pRegion, tile)
    WindowPtr   pWin;
    RegionPtr   pRegion;
    PixmapPtr   tile;
{
    register PixmapPtr pPixmap;
    int		    nbox;	    /* number of boxes to fill */
    register BoxPtr pbox;	    /* pointer to list of boxes to fill */
    int		    x, y;
    GCPtr	    pGC;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;
    VoidProc	    sfbTileProc;

    nbox = REGION_NUM_RECTS(pRegion);
    if (nbox == 0) return;
    pbox = REGION_RECTS(pRegion);

    /* Load info into fake GC. */
    pGC = &fakeGC;
    pGC->planemask = SFBBUSALL1;
    pGC->alu = GXcopy;
    pGC->tile.pixmap = tile;

    pGC->patOrg.x = pWin->drawable.x;
    pGC->patOrg.y = pWin->drawable.y;

    /* Get screen drawable (root window) */
    pPixmap = (PixmapPtr)(pWin->drawable.pScreen->devPrivate);

    /* Figure out which painting routine to use */
    sfbTileProc = sfbTileFillArea;
    if (tile->drawable.width == SFBBUSBITS / SFBPIXELBITS) {
	sfbTileProc = sfbTileFillAreaWord;
    }

    /* Set up sfb state */
    scrPriv = SFBSCREENPRIV(pWin->drawable.pScreen);
    scrPriv->lastGC = pGC;
    sfb = scrPriv->sfb;
    SFBPLANEMASK(sfb, SFBBUSALL1);
    SFBROP(sfb, GXcopy);
    
    /* Turn boxes into rectangles, and paint. */
    if (nbox == 1) {
	xRectangle      rect;

	rect.x = x = pbox->x1;
	rect.y = y = pbox->y1;
	rect.width = pbox->x2 - x;
	rect.height = pbox->y2 - y;
	(*sfbTileProc)(pPixmap, 1, &rect, pGC);
    } else {
	xRectangle      *xrects, *xr;
	int		i;

	xrects = (xRectangle *) ALLOCATE_LOCAL(nbox * sizeof(xRectangle));
	if (!xrects) return;
    
	for (i = nbox, xr = xrects; i != 0; i--, xr++, pbox++) {
	    xr->x = x = pbox->x1;
	    xr->y = y = pbox->y1;
	    xr->width = pbox->x2 - x;
	    xr->height = pbox->y2 - y;
	}
	(*sfbTileProc)(pPixmap, nbox, xrects, pGC);
	DEALLOCATE_LOCAL(xrects);
    }
}


void sfbPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
/*#define CHEAT*/
#if defined(CHEAT) && defined(SOFTWARE_MODEL)
    cfbPaintWindow(pWin, pRegion, what);
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
	    sfbFillBoxTile((DrawablePtr)pWin, pRegion, pWin->background.pixmap);
	    return;

	case BackgroundPixel:
	    sfbFillBoxSolid((DrawablePtr)pWin, pRegion, pWin->background.pixel);
	    return;
    	}
    	break;

    case PW_BORDER:
	if (pWin->borderIsPixel) {
	    sfbFillBoxSolid ((DrawablePtr)pWin, pRegion, pWin->border.pixel);
	} else {
	    /* The border tile origin is always the same as the background tile */
            WindowPtr	pBgWin;
            for (pBgWin = pWin;
                 pBgWin->backgroundState == ParentRelative;
                 pBgWin = pBgWin->parent);

	    sfbFillBoxTile((DrawablePtr)pBgWin, pRegion, pWin->border.pixmap);
	}
	return;
    }
}

