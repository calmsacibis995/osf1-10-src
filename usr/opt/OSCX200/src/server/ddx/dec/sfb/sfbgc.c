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
#define IDENT "X-5"
#define MODULE_NAME SFBGC
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991, 1992 BY            *
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
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**      This module creates, destroys, and validates Graphics Contexts
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**
**  CREATION DATE:     20-Nov-1991
**
**
**  MODIFICATION HISTORY:
**
**
** X-5		BIM0011		Irene McCartney		17-Jan-1992
**		Merge in latest changes from Joel
**
** X-4		TLB0005		Tracy Bragdon		13-Dec-1991
**		change default GCOp from cfbOpsTE6 to cfbTEOps to 
**		match our cfb
**
** X-3		TLB0003		Tracy Bragdon		3-Dec-1991
**		remove references to obsolete GC field (FillArea)
**
** X-2		TLB0002		Tracy Bragdon		15-Nov-1991
**		Changes for R5 font structures; use FONT* macros
**		where possible
**
**--
**/

#include <stdio.h>
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"

#ifdef MITR5
#include "dixfontstr.h"
#else
#include "fontstr.h"
#endif

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "mistruct.h"
#include "mibstore.h"

#include "sfb.h"
#include "cfbgc.h"
#include "cfbmfb.h"
#include "cfbpixmap.h"
#include "cfbbitblt.h"
#include "cfbmskbits.h"

#ifdef MITR5
#include "cfbdecline.h"
#else
#include "cfbline.h"
#endif

#include "sfbarc1.h"
#include "sfbbitblt.h"
#include "sfbfillarc.h"
#include "sfbfillrct.h"
#include "sfbgc.h"
#include "sfbline.h"
#include "sfbpixmap.h"
#include "sfbplane.h"
#include "sfbplygblt.h"
#include "sfbpntarea.h"
#include "sfbpoly.h"
#include "sfbpolypnt.h"
#include "sfbteplygblt.h"
#include "sfbwideline.h"
#include "sfbzerarc.h"

/* Standard set of funcs that every sfb gc points to */
static GCFuncs sfbFuncs = {
    sfbValidateGC,
    cfbChangeGC,
    cfbCopyGC,
    sfbDestroyGC,
    cfbChangeClip,
    cfbDestroyClip,
    cfbCopyClip,
};

/* Fairly common sets of ops that many gc's will point to.  All these guys
   require lineWidth = 0, fillStyle = Solid, etc.  See MatchCommon() below.

    We steal code from cfb for some routines, even when the drawable is in
    on-screen memory.  In particular, cfbPutImage goes through ops->CopyArea
    or ops->CopyPlane, so it works for both cfb and sfb.
*/


/* ||| Would save more space by putting in real GC hashing, and thus
   sharing ops records wherever possible.  This is a cheap hack to
   get at least some sharing. */

/* Standard ops with terminal emulator fonts of width 1..SFBBUSBITS */
static GCOps	sfbOpsTE = {
    sfbSolidFillSpans,		/* FillSpans	    */
    sfbSetSpans,		/* SetSpans	    */
    cfbPutImage,		/* PutImage	    */
    sfbCopyArea,		/* CopyArea	    */
    sfbCopyPlane,		/* CopyPlane	    */
    sfbPolyPoint,		/* PolyPoint	    */
    sfbLineSS,			/* Polyines	    */
    sfbSegmentSS,		/* PolySegment      */
    miPolyRectangle,		/* PolyRectangle    */
    sfbZeroPolyArc,		/* PolyArc	    */
    sfbFillPolygon,		/* FillPolygon      */
    sfbPolyFillRectSolid,	/* PolyFillRect     */
    sfbPolyFillArc,		/* PolyFillArc      */
    miPolyText8,		/* PolyText8	    */
    miPolyText16,		/* PolyText16       */
    miImageText8,		/* ImageText8       */
    miImageText16,		/* ImageText16      */
    sfbTEImageGlyphBlt,		/* ImageGlyphBlt    */
    sfbTEPolyGlyphBlt,          /* PolyGlyphBlt     */
    mfbPushPixels,		/* PushPixels       */
    (VoidProc)NULL,		/* LineHelper       */
};

/* Standard ops with variable-pitch fonts of maximum width 1..SFBBUSBITS */
static GCOps	sfbOpsNonTE = {
    sfbSolidFillSpans,		/* FillSpans	    */
    sfbSetSpans,		/* SetSpans	    */
    cfbPutImage,		/* PutImage	    */
    sfbCopyArea,		/* CopyArea	    */
    sfbCopyPlane,		/* CopyPlane	    */
    sfbPolyPoint,		/* PolyPoint	    */
    sfbLineSS,			/* Polyines	    */
    sfbSegmentSS,		/* PolySegment      */
    miPolyRectangle,		/* PolyRectangle    */
    sfbZeroPolyArc,		/* PolyArc	    */
    sfbFillPolygon,		/* FillPolygon      */
    sfbPolyFillRectSolid,	/* PolyFillRect     */
    sfbPolyFillArc,		/* PolyFillArc      */
    miPolyText8,		/* PolyText8	    */
    miPolyText16,		/* PolyText16       */
    miImageText8,		/* ImageText8       */
    miImageText16,		/* ImageText16      */
    sfbImageGlyphBlt,		/* ImageGlyphBlt    */
    sfbPolyGlyphBlt,            /* PolyGlyphBlt     */
    mfbPushPixels,		/* PushPixels       */
    (VoidProc)NULL,		/* LineHelper       */
};

static GCOps *
MatchCommon (pGC, gcPriv)
    GCPtr	    pGC;
    cfbPrivGCPtr    gcPriv;
{

    if (   pGC->lineWidth == 0 
        && pGC->lineStyle == LineSolid
        && pGC->fillStyle == FillSolid
	&& pGC->font) {

	if (FONTMAXBOUNDS(pGC->font, rightSideBearing) -
	    FONTMINBOUNDS(pGC->font, leftSideBearing) <= SFBBUSBITS) {

	    int charWidth;
    
	    /* At this point, we know we'll definitely return one of the above
	       ops records */

	    charWidth = FONTMINBOUNDS(pGC->font, characterWidth);
	    if (TERMINALFONT(pGC->font) && charWidth >= 4) {
		return &sfbOpsTE;
	    } else {
		return &sfbOpsNonTE;
	    }
	}
    }
    return (GCOps *)NULL;
}

Bool sfbCreateGC(pGC)
    register GCPtr      pGC;
{
    sfbGCPrivPtr	sfbGCPriv;
    sfbScreenPrivPtr    scrPriv;

    if (pGC->depth == 1) {
	return (mfbCreateGC(pGC));
    }

    if (! cfbCreateGC(pGC)) {
	return FALSE;
    }

    /* Override a few of cfbCreateGC's decisions */
    pGC->ops = &sfbOpsTE;
    pGC->funcs = &sfbFuncs;
    sfbGCPriv = SFBGCPRIV(pGC);
    sfbGCPriv->lastValidateWasSFB = TRUE;
    /* Default dash pattern of X11 protocol is 4 on, 4 off */
    sfbGCPriv->dashLength  = 8;
    sfbGCPriv->dashAdvance = 0;		
    sfbGCPriv->dashPattern = 0x0f0f0f0f;
    scrPriv = SFBSCREENPRIV(pGC->pScreen);
    LOADSTATE(scrPriv, scrPriv->sfb, pGC);
    return TRUE;
}


void sfbDestroyGC(pGC)
    GC 			*pGC;
{
    sfbScreenPrivPtr    scrPriv;

    scrPriv = SFBSCREENPRIV(pGC->pScreen);
    if (scrPriv->lastGC = pGC) scrPriv->lastGC = (GCPtr)NULL;
    cfbDestroyGC(pGC);
}

/* What operations can be done multiple times on a src/dst pixel combination
and still give the same result in the dst pixel? */

Bool8 idempotent[16] = {
    TRUE,       /* clear	*/
    TRUE,       /* and		*/
    FALSE,      /* andReverse   */
    TRUE,       /* copy		*/
    
    TRUE,       /* andInverted  */
    TRUE,       /* noop		*/
    FALSE,      /* xor		*/
    TRUE,       /* or		*/
    
    FALSE,      /* nor		*/
    FALSE,      /* equiv	*/
    FALSE,      /* invert       */
    FALSE,      /* orReverse    */
    
    TRUE,       /* copyInverted */
    TRUE,       /* orInverted   */
    FALSE,      /* nand		*/
    TRUE	/* set		*/
};

void
sfbValidateGC(pGC, changes, pDraw)
    register GC     *pGC;
    Mask	    changes;
    DrawablePtr     pDraw;
{
    register cfbPrivGCPtr gcPriv;
    sfbGCPrivPtr    sfbGCPriv;
    GCOps	    *ops;
    int		    height;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;
    
    /* flags for changing the proc vector */
    int		    new_line, new_text, new_fillspans, new_fill;

    sfbGCPriv = SFBGCPRIV(pGC);
    /* First compute new dash pattern if needed.  Do this regardless of who
       did last validation and who should do this validation. */
    if (changes & GCDashList) {
	/* See if dash pattern is smaller than 16 bits total.  If so, replicate
	   it out to fill a Bits32, and use that rather than always computing
	   the pattern on the fly. */
	int		i, dashLength;
	Bits32		dashPattern;
	Bits8		*pDash;
	int		major, minor, which;

	dashLength = 0;
	i = pGC->numInDashList;
	pDash = pGC->dash;
	do {
	    i--;
	    dashLength += pDash[i];
	} while (i != 0);
	if (pGC->numInDashList & 1) {
	    /* The dash pattern has an odd number of dashes, so it'll really
	       take twice as many bits to represent. */
	    dashLength *= 2;
	}
	sfbGCPriv->dashLength = dashLength;
	if (dashLength <= 16) {
	    /* Create the bit pattern. */
	    sfbGCPriv->dashAdvance = 16 % dashLength;
	    major = 0;
	    minor = pDash[0];
	    which = EVENDASH;
	    GetDashBits(dashLength, dashPattern, pDash, pGC->numInDashList,
		major, minor, which);
	    dashPattern >>= (16 - dashLength);
	    dashPattern &= SFBRIGHTBUSMASK(dashLength, SFBBUSALL1);
	    do {
		dashPattern |= (dashPattern << dashLength);
		dashLength *= 2;
	    } while (dashLength < 32);
	    sfbGCPriv->dashPattern = dashPattern;
	}
    }

    if (pDraw->type != DRAWABLE_WINDOW && MAINMEMORY((PixmapPtr)pDraw)){
	/* Let's let cfb do validation.  If sfb did the most recent validation,
	   then we need to start off gc ops with a clean slate, and force cfb
	   to recompute EVERYTHING.  */
	if (sfbGCPriv->lastValidateWasSFB) {
	    sfbGCPriv->lastValidateWasSFB = FALSE;
	    /* Toss existing pGC->ops and reset to standard cfb ops */
	    if (pGC->ops->devPrivate.val) {
		cfbDestroyOps(pGC->ops);
	    }
#ifdef MITR5
	    pGC->ops = &cfbTEOps;
#else
	    pGC->ops = &cfbOpsTE6;
#endif
	    changes = ~0;
	    /* If tiling is set, and the tile is in sfb memory, dump it out to
	      main memory; I'd really rather avoid doing new routines for that
	      direction, and there are speed problems as well. */
	    if (pGC->fillStyle == FillTiled && !pGC->tileIsPixel &&
		SCREENMEMORY(pGC->tile.pixmap)) {
		pGC->tile.pixmap = sfbScreenToMainPixmap(pGC->tile.pixmap);
	    }
	}

	cfbValidateGC(pGC, changes, pDraw);
	/* Override a few decisions.  Be careful if cfbValidate chose
	   one of the standard cfbOps records. */
	/* ||| Here we are just messing up sharing of ops records again.  Should
	   put in real caching. */
	if (!pGC->ops->devPrivate.val) {
	    pGC->ops = cfbCreateOps(pGC->ops);
	}
	pGC->ops->CopyArea = sfbCopyArea;
	scrPriv = SFBSCREENPRIV(pGC->pScreen);
	scrPriv->lastGC = (GCPtr)NULL;
	return;
    }

    if (!sfbGCPriv->lastValidateWasSFB) {
	sfbGCPriv->lastValidateWasSFB = TRUE;
	if (pGC->ops->devPrivate.val)
	    cfbDestroyOps(pGC->ops);
	(pGC->ops) = &sfbOpsTE;
	changes = ~0;
    }
    gcPriv = (cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr);
    cfbComputeClips(pGC, gcPriv, changes, pDraw);

    /*
     * Figure out which values have changed in the GC, and plug in (possibly)
     * new procedures accordingly.
     */

    /* Set everyone FALSE to begin with. */
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fill = FALSE;

    /*
     * Accumulate a list of which procedures might have to change due to
     * changes in the GC.  In some cases (e.g.
     * changing one 16 bit tile for another) we might not really need
     * a change, but the code is being paranoid.  This sort of batching
     * wins if any pair of items both change the same thing. 
     * Note that we've already dealt with
     * changes to GCSubwindowMode, GCClipXOrigin, GCClipYOrigin, GCClipMask.
     */

#if SFBPIXELBITS < SFBBUSBITS || defined(SLEAZOID32)
    if (changes & GCPlaneMask) {
	PixelToPixelWord(pGC->planemask);
    }
   
    if (changes & GCForeground) {
	PixelToPixelWord(pGC->fgPixel);
    }
   
    if (changes & GCBackground) {
	PixelToPixelWord(pGC->bgPixel);
    }
#endif
   
    if (changes & (GCLineWidth | GCLineStyle | GCDashList)) {
	new_line = TRUE;
    }

    if (changes & (GCFillStyle)) {
	new_text = TRUE;
	new_fillspans = TRUE;
	new_line = TRUE;
	new_fill = TRUE;
    }

    if (changes & (GCTile)) {
	if (!pGC->tileIsPixel) {
	    int width = (int)pGC->tile.pixmap->drawable.width * SFBPIXELBITS;
	    PixmapPtr ntile;

	    if ((width <= SFBBUSBITS) && !(width & (width - 1))) {
		ntile = cfbCopyPixmap(pGC->tile.pixmap);
		if (ntile != (PixmapPtr)NULL) {
		    cfbDestroyPixmap(pGC->tile.pixmap);
		    (void) sfbPadPixmap(ntile);
		    pGC->tile.pixmap = ntile;
		}
	    }
	    new_fillspans = TRUE;
	    new_fill = TRUE;
	}
    }

    if (changes & (GCStipple)) {
	if (pGC->stipple != NullPixmap) {
	    int width = pGC->stipple->drawable.width;
	    PixmapPtr nstipple;

	    if ((width <= SFBBUSBITS) && !(width & (width - 1))) {
		nstipple = cfbCopyPixmap(pGC->stipple);
		if (nstipple != (PixmapPtr)NULL) {
		    cfbDestroyPixmap(pGC->stipple);
		    (void) sfbPadPixmap(nstipple);
		    pGC->stipple = nstipple;
		}
	    }
	    new_fillspans = TRUE;
	    new_fill = TRUE;
	}
    }

    if (changes & (GCFont)) {
	new_text = TRUE;
    }

    /* Note that we don't have to set any Booleans if any of the
     * following change:
     *
     * GCFillRule
     * GCJoinStyle
     * GCTileStipXOrigin
     * GCTileStipYOrigin
     * GCGraphicsExposures
     * GCDashOffset
     * GCArcMode
     */


    /* Load hardware state with current gc state. */
    scrPriv = SFBSCREENPRIV(pGC->pScreen);
    sfb = scrPriv->sfb;
    LOADSTATE(scrPriv, sfb, pGC);

    /* Deal with the changes we've collected */

    if (new_line || new_text || new_fillspans || new_fill) {
	if (ops = MatchCommon(pGC, gcPriv)) {
	    if (pGC->ops->devPrivate.val)
		cfbDestroyOps(pGC->ops);
	    pGC->ops = ops;
	    return;
	}
	if (!pGC->ops->devPrivate.val) {
	    pGC->ops = cfbCreateOps(pGC->ops);
	}
    }

    /* We're only here if there are changes, and we aren't using a pretty
       standard ops record. */

    ops = pGC->ops;

    if (new_line) {
	ops->PolySegment = miPolySegment;
	ops->PolyArc = miPolyArc;
	if (pGC->lineStyle == LineSolid) {
	    if (pGC->lineWidth == 0) {
		if (pGC->fillStyle == FillSolid) {
		    ops->Polylines   = sfbLineSS;
		    ops->PolySegment = sfbSegmentSS;
		    ops->PolyArc     = sfbZeroPolyArc;
		} else {
		    ops->Polylines   = miZeroLine;
		}
	    } else {
		if (pGC->lineWidth == 1) {
		    ops->PolyArc = sfbPolyArc1;
		}
	    }
	} else { /* LineOnOffDash, LineDoubleDash */
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid) {
		if (sfbGCPriv->dashLength <= 16) {
		    ops->Polylines   = sfbDashLine16;
		    ops->PolySegment = sfbDashSegment16;
		} else {
		    ops->Polylines   = sfbDashLine;
		    ops->PolySegment = sfbDashSegment;
		}
		ops->PolyArc     = miZeroPolyArc;
            }else {
               /* miWideDash supposedly even handles 0-width correctly */
               ops->Polylines       = miWideDash;
	    }
	}
    }

    /* sfbWideLine and sfbWideDash depend upon the raster op and the number of
       clip rectangles.  It's easiest to always test for the right conditions
       to allow the sfb-specific versions than to junk in with new_line. */

    if (pGC->lineWidth > 0) {
	if (pGC->lineStyle == LineSolid) {
	    ops->Polylines = miWideLine;
	} else {
	    ops->Polylines = miWideDash;
	}
	if (idempotent[pGC->alu] && pGC->fillStyle == FillSolid
	    && REGION_NUM_RECTS(gcPriv->pCompositeClip) == 1) {
	    if (pGC->lineStyle == LineSolid) {
		ops->Polylines = sfbWideLine;
	    } else {
		ops->Polylines = sfbWideDash;
	    }
	}
    }

#ifdef MITR5
    if (new_text && (pGC->font != (FontPtr)NULL)) {
#else
    if (new_text && (pGC->font != (EncodedFontPtr)NULL)) {
#endif
	/*
	   Fast font code can, in general, be used only for fonts with glyph
	   width <= SFBBUSBITS.  Note that ImageText ignores alu rop and fill
	   style.
	*/
	   
	/* (||| We should have max glyph width, which isn't the
	   same as the more pessimistic calculation here.  It's unlikely that
	   any single glyph spans this distance.) */

        ops->PolyGlyphBlt  = miPolyGlyphBlt;
	ops->ImageGlyphBlt = miImageGlyphBlt;

	if ((FONTMAXBOUNDS(pGC->font, rightSideBearing) -
             FONTMINBOUNDS(pGC->font, leftSideBearing)) <= SFBBUSBITS
            && FONTMINBOUNDS(pGC->font, characterWidth) >= 0) {
	    /* Choose ImageGlyphBlt */
	    if (TERMINALFONT(pGC->font)) {
		/* Use special code for terminal emulator fonts */
		ops->ImageGlyphBlt = sfbTEImageGlyphBlt;
	    } else {
                ops->ImageGlyphBlt = sfbImageGlyphBlt;
	    }
	    /* Choose PolyGlyphBlt */
	    if (pGC->fillStyle == FillSolid ||
		(pGC->fillStyle == FillOpaqueStippled &&
		 pGC->fgPixel == pGC->bgPixel)) {

	        if (TERMINALFONT(pGC->font)) {
		    /* Use special code for terminal emulator fonts */
		    ops->PolyGlyphBlt = sfbTEPolyGlyphBlt;
		} else {
		    ops->PolyGlyphBlt = sfbPolyGlyphBlt;
		}
	    }
        }
    }    

    if (new_fillspans) {
	/* Which FillSpans procedure, which FillArea rectangle helper? */
	ops->PolyFillRect = sfbPolyFillRect;
	switch (pGC->fillStyle) {
	case FillSolid:
	    ops->PolyFillRect = sfbPolyFillRectSolid;
	    ops->FillSpans    = sfbSolidFillSpans;
	    break;
	case FillTiled:
	    if (!pGC->tileIsPixel) {
		if (pGC->tile.pixmap->drawable.width * SFBPIXELBITS 
		    == SFBBUSBITS) {
		    ops->FillSpans = sfbTileFillSpansWord;
		    height = pGC->tile.pixmap->drawable.height;
		    if (height & (height - 1)) {
			PRIVFILLAREA = sfbTileFillAreaWord;
		    } else { /* height = 2^n */
			ops->PolyFillRect = sfbTileFillAreaWord2;
		    }
		} else {
		    ops->FillSpans   = sfbTileFillSpans;
		    PRIVFILLAREA = sfbTileFillArea;
		}
	    }
	    break;
	case FillStippled:
	    if (pGC->stipple->drawable.width == SFBBUSBITS) {
		ops->FillSpans   = sfbTSFillSpansWord;
		height = pGC->stipple->drawable.height;
		if (height & (height - 1)) {
		    PRIVFILLAREA = sfbTSFillAreaWord;
		} else { /* height = 2^n */
		    ops->PolyFillRect = sfbTSFillAreaWord2;
		}
	    } else {
		ops->FillSpans   = sfbTSFillSpans;
		PRIVFILLAREA = sfbTSFillArea;
	    }
	    break;
	case FillOpaqueStippled:
	    if (pGC->fgPixel == pGC->bgPixel) {
		ops->PolyFillRect = sfbPolyFillRectSolid;
		ops->FillSpans    = sfbSolidFillSpans;
	    } else {
		if (pGC->stipple->drawable.width == SFBBUSBITS) {
		    ops->FillSpans   = sfbOSFillSpansWord;
		    height = pGC->stipple->drawable.height;
		    if (height & (height - 1)) {
			PRIVFILLAREA = sfbOSFillAreaWord;
		    } else { /* height = 2^n */
			ops->PolyFillRect = sfbOSFillAreaWord2;
		    }
		} else {
		    ops->FillSpans   = sfbOSFillSpans;
		    PRIVFILLAREA = sfbOSFillArea;
		}
	    }
	    break;
	default:
	    FatalError("sfbValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fill) {
	/* Which polygon code? */
	if ((pGC->fillStyle == FillSolid) ||
 		(pGC->fillStyle == FillOpaqueStippled &&
		 pGC->fgPixel == pGC->bgPixel) ||
		 (pGC->stipple == (PixmapPtr)NULL)) {
	    ops->FillPolygon = sfbFillPolygon;
	    ops->PolyFillArc = sfbPolyFillArc;
	} else {
	    ops->FillPolygon = miFillPolygon;
	    ops->PolyFillArc = miPolyFillArc;
	}
    }
} /* sfbValidateGC */
