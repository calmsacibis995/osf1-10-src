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
static char *rcsid = "@(#)$RCSfile: ffbgc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:10:01 $";
#endif
/*
 */

#include <stdio.h>
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"

#include "dixfontstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "mistruct.h"
#include "mibstore.h"

#include "ffb.h"
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

#include "ffbarc1.h"
#include "ffbbitblt.h"
#include "ffbcopy.h"
#include "ffbfillarc.h"
#include "ffbfillrct.h"
#include "ffbgc.h"
#include "ffbline.h"
#include "ffbpixmap.h"
#include "ffbplane.h"
#include "ffbplygblt.h"
#include "ffbpntarea.h"
#include "ffbpoly.h"
#include "ffbpolypnt.h"
#include "ffbpolyrect.h"
#include "ffbpolytext.h"
#include "ffbteplygblt.h"
#include "ffbwideline.h"
#include "ffbzerarc.h"


/* Standard set of funcs that every ffb gc points to */
static GCFuncs ffbFuncs = {
    ffbValidateGC,
    CFB_NAME(ChangeGC),
    cfbCopyGC,
    CFB_NAME(DestroyGC),
    CFB_NAME(ChangeClip),
    CFB_NAME(DestroyClip),
    CFB_NAME(CopyClip),
};

/* Fairly common sets of ops that many gc's will point to.  All these guys
   require lineWidth = 0, fillStyle = Solid, etc.  See MatchCommon() below.

    We steal code from cfb for some routines, even when the drawable is in
    on-screen memory.  In particular, cfbPutImage goes through ops->CopyArea
    or ops->CopyPlane, so it works for both cfb and ffb.
*/


/* ||| Would save more space by putting in real GC hashing, and thus
   sharing ops records wherever possible.  This is a cheap hack to
   get at least some sharing. */

/* Standard ops with terminal emulator fonts of width 1..FFBBUSBITS */
static GCOps	ffbOpsTE = {
    ffbSolidFillSpans,		/* FillSpans	    */
    ffbSetSpans,		/* SetSpans	    */
    CFB_NAME(PutImage),		/* PutImage	    */
    ffbCopyArea,		/* CopyArea	    */
    ffbCopyPlane,		/* CopyPlane	    */
    ffbPolyPoint,		/* PolyPoint	    */
    ffbLineSS,			/* Polylines	    */
    ffbSegmentSS,		/* PolySegment      */
    ffbPolyRectangle,		/* PolyRectangle    */
    ffbZeroPolyArc,		/* PolyArc	    */
    ffbFillPolygon,		/* FillPolygon      */
    ffbPolyFillRectSolid,	/* PolyFillRect     */
    ffbPolyFillArc,		/* PolyFillArc      */
    ffbPolyText8,		/* PolyText8	    */
    ffbPolyText16,		/* PolyText16       */
    ffbImageText8,		/* ImageText8       */
    ffbImageText16,		/* ImageText16      */
    ffbTEImageGlyphBlt,		/* ImageGlyphBlt    */
    (VoidProc)ffbTEPolyGlyphBlt,/* PolyGlyphBlt     */
    mfbPushPixels,		/* PushPixels       */
    (VoidProc)NULL,		/* LineHelper       */
};

/* Standard ops with variable-pitch fonts of maximum width 1..FFBBUSBITS */
static GCOps	ffbOpsNonTE = {
    ffbSolidFillSpans,		/* FillSpans	    */
    ffbSetSpans,		/* SetSpans	    */
    CFB_NAME(PutImage),		/* PutImage	    */
    ffbCopyArea,		/* CopyArea	    */
    ffbCopyPlane,		/* CopyPlane	    */
    ffbPolyPoint,		/* PolyPoint	    */
    ffbLineSS,			/* Polylines	    */
    ffbSegmentSS,		/* PolySegment      */
    ffbPolyRectangle,		/* PolyRectangle    */
    ffbZeroPolyArc,		/* PolyArc	    */
    ffbFillPolygon,		/* FillPolygon      */
    ffbPolyFillRectSolid,	/* PolyFillRect     */
    ffbPolyFillArc,		/* PolyFillArc      */
    ffbPolyText8,		/* PolyText8	    */
    ffbPolyText16,		/* PolyText16       */
    ffbImageText8,		/* ImageText8       */
    ffbImageText16,		/* ImageText16      */
    ffbImageGlyphBlt,		/* ImageGlyphBlt    */
    (VoidProc)ffbPolyGlyphBlt,  /* PolyGlyphBlt     */
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
	    FONTMINBOUNDS(pGC->font, leftSideBearing) <= FFBBUSBITS) {

	    int charWidth;
    
	    /* At this point, we know we'll definitely return one of the above
	       ops records */

	    charWidth = FONTMINBOUNDS(pGC->font, characterWidth);
	    if (TERMINALFONT(pGC->font) && charWidth >= 4) {
		return &ffbOpsTE;
	    } else {
		return &ffbOpsNonTE;
	    }
	}
    }
    return (GCOps *)NULL;
}

Bool ffbCreateGC(pGC)
    register GCPtr      pGC;
{
    ffbGCPrivPtr	ffbGCPriv;
    ffbScreenPrivPtr    scrPriv;

    if (pGC->depth == 1) {
	return (mfbCreateGC(pGC));
    }

    if (! CFB_NAME(CreateGC)(pGC)) {
	return FALSE;
    }

    /* Override a few of cfbCreateGC's decisions */
    pGC->ops = &ffbOpsTE;
    pGC->funcs = &ffbFuncs;
    pGC->miTranslate = 0;   /* it's faster for us to do the translation */
    ffbGCPriv = FFBGCPRIV(pGC);
    ffbGCPriv->lastValidateWasFFB = TRUE;
    /* Default dash pattern of X11 protocol is 4 on, 4 off */
    ffbGCPriv->dashLength  = 8;
    ffbGCPriv->dashAdvance = 0;		
    ffbGCPriv->dashPattern = 0x0f0f0f0f;
    scrPriv = FFBSCREENPRIV(pGC->pScreen);
#if FFBPIXELBITS==32
    switch(pGC->depth) {
     case 4:
	ffbGCPriv->planemask = FFB_OVRLY_PLANEMASK;
	break;
     case 8:
	ffbGCPriv->planemask = FFB_8U_PLANEMASK;
	break;
     case 12:
	ffbGCPriv->planemask = FFB_12_BUF0_PLANEMASK | FFB_12_BUF1_PLANEMASK;
	break;
     case 24:
	ffbGCPriv->planemask = FFB_24BIT_PLANEMASK;
	break;
    }
#else
    ffbGCPriv->planemask = FFB_8P_PLANEMASK; 
#endif
    scrPriv->lastGC = (GCPtr)NULL;
    return TRUE;
}


void ffbDestroyGC(pGC)
    GC 			*pGC;
{
    ffbScreenPrivPtr    scrPriv;

    scrPriv = FFBSCREENPRIV(pGC->pScreen);
    if (scrPriv->lastGC = pGC) scrPriv->lastGC = (GCPtr)NULL;
    CFB_NAME(DestroyGC)(pGC);
}

#ifdef FFB_DEPTH_INDEPENDENT

/* What operations can be done multiple times on a src/dst pixel combination
and still give the same result in the dst pixel? */

Bool8 ffbIdempotentALU[16] = {
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

int ffbMaxFillPixels[2][16] = {
    {
	 FFBMAXFPWPIX8,  FFBMAXFPRPIX8,  FFBMAXFPRPIX8,  FFBMAXFPWPIX8,
	 FFBMAXFPRPIX8,  FFBMAXFPWPIX8,  FFBMAXFPRPIX8,  FFBMAXFPRPIX8,
	 FFBMAXFPRPIX8,  FFBMAXFPRPIX8,  FFBMAXFPRPIX8,  FFBMAXFPRPIX8,
	 FFBMAXFPWPIX8,  FFBMAXFPRPIX8,  FFBMAXFPRPIX8,  FFBMAXFPWPIX8,
    },
    {
	 FFBMAXFPWPIX32,  FFBMAXFPRPIX32,  FFBMAXFPRPIX32,  FFBMAXFPWPIX32,
	 FFBMAXFPRPIX32,  FFBMAXFPWPIX32,  FFBMAXFPRPIX32,  FFBMAXFPRPIX32,
	 FFBMAXFPRPIX32,  FFBMAXFPRPIX32,  FFBMAXFPRPIX32,  FFBMAXFPRPIX32,
	 FFBMAXFPWPIX32,  FFBMAXFPRPIX32,  FFBMAXFPRPIX32,  FFBMAXFPWPIX32,
    }
};

#else

extern Bool8 ffbIdempotentALU[];
extern int ffbMaxFillPixels[];

#endif
void
ffbValidateGC(pGC, changes, pDraw)
    register GC     *pGC;
    Mask	    changes;
    DrawablePtr     pDraw;
{
    register cfbPrivGCPtr gcPriv;
    ffbGCPrivPtr    ffbGCPriv;
    GCOps	    *ops;
    int		    height;     /* stipple/tile height */
    int		    width;      /* stipple/tile width */
    PixelWord       pixel;
    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;
    ffbBufDPtr	    bdptr; /* for hardware state */

    /* flags for changing the proc vector */
    int		    new_line, new_text, new_fillspans, new_fill,new_rrop;

#if FFBPIXELBITS==32
    if(pDraw->bitsPerPixel == 8)
        {
        ffb8ValidateGC(pGC, changes, pDraw);
        return;
        }
#endif


    ffbGCPriv = FFBGCPRIV(pGC);
    scrPriv = FFBSCREENPRIV(pGC->pScreen);

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
	ffbGCPriv->dashLength = dashLength;
	if (dashLength <= 16) {
	    /* Create the bit pattern. */
	    ffbGCPriv->dashAdvance = 16 % dashLength;
	    major = 0;
	    minor = pDash[0];
	    which = EVENDASH;
	    GetDashBits(dashLength, dashPattern, pDash, pGC->numInDashList,
		major, minor, which);
	    dashPattern >>= (16 - dashLength);
	    dashPattern &= FFBRIGHTBUSMASK(dashLength, FFBBUSALL1);
	    do {
		dashPattern |= (dashPattern << dashLength);
		dashLength *= 2;
	    } while (dashLength < 32);
	    ffbGCPriv->dashPattern = dashPattern;
	}
    }

    if (pDraw->type != DRAWABLE_WINDOW && MAINMEMORY((PixmapPtr)pDraw)){
	/* Let's let cfb do validation.  If ffb did the most recent validation,
	   then we need to start off gc ops with a clean slate, and force cfb
	   to recompute EVERYTHING.  */
	if (ffbGCPriv->lastValidateWasFFB) {
	    ffbGCPriv->lastValidateWasFFB = FALSE;
	    pGC->miTranslate = 1;
	    /* Toss existing pGC->ops and reset to standard cfb ops */
	    if (pGC->ops->devPrivate.val) {
		CFB_NAME(DestroyOps)(pGC->ops);
	    }
#ifdef MITR5
	    pGC->ops = &cfbTEOps;
#else
	    pGC->ops = &cfbOpsTE6;
#endif
	    changes = ~0;
	    /* If tiling is set, and the tile is in ffb memory, dump it out to
	      main memory; I'd really rather avoid doing new routines for that
	      direction, and there are speed problems as well. */
	    if (pGC->fillStyle == FillTiled && !pGC->tileIsPixel &&
		SCREENMEMORY(pGC->tile.pixmap)) {
		pGC->tile.pixmap = ffbScreenToMainPixmap(pGC->tile.pixmap);
	    }
	}

	CFB_NAME(ValidateGC)(pGC, changes, pDraw);
	/* Override a few decisions.  Be careful if cfbValidate chose
	   one of the standard cfbOps records. */
	/* ||| Here we are just messing up sharing of ops records again.  Should
	   put in real caching. */
	if (!pGC->ops->devPrivate.val) {
	    pGC->ops = CFB_NAME(CreateOps)(pGC->ops);
	}
	pGC->ops->CopyArea = ffbCopyArea;
	scrPriv->lastGC = (GCPtr)NULL;
	return;
    }

    if (!ffbGCPriv->lastValidateWasFFB) {
	ffbGCPriv->lastValidateWasFFB = TRUE;
	pGC->miTranslate = 0;
	if (pGC->ops->devPrivate.val)
	    CFB_NAME(DestroyOps)(pGC->ops);
	(pGC->ops) = &ffbOpsTE;
	changes = ~0;
    }
    gcPriv = (cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (pDraw->type != DRAWABLE_WINDOW && SCREENMEMORY((PixmapPtr)pDraw)){

	if (FFBPARENTWINDOW(pDraw)) {
	    /* parent window field of ffbpixmaprec is non-null iff
	       this is a packed back buffer, in which case we need
	       a composite clip that matches it's parent window; so
	       compute the composite clip using the parent window. */
	    cfbComputeClips(pGC, gcPriv, changes, FFBPARENTWINDOW(pDraw));
	} else {
	    /* not a packed back buffer */
	    cfbComputeClips(pGC, gcPriv, changes, pDraw);
	}
    } else {
	cfbComputeClips(pGC, gcPriv, changes, pDraw);
    }


    /*
     * Figure out which values have changed in the GC, and plug in (possibly)
     * new procedures accordingly.
     */

    /* Set everyone FALSE to begin with. */
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fill = FALSE;
    new_rrop = FALSE;
    /*
     * Accumulate a list of which procedures might have to change due to
     * changes in the GC.  In some cases (e.g.
     * changing one 16 bit tile for another) we might not really need
     * a change, but the code is being paranoid.  This sort of batching
     * wins if any pair of items both change the same thing. 
     * Note that we've already dealt with
     * changes to GCSubwindowMode, GCClipXOrigin, GCClipYOrigin, GCClipMask.
     */

#if FFBPIXELBITS == 8
    if (changes & GCPlaneMask) {
	Pixel8ToPixelWord(pGC->planemask);
        ffbGCPriv->planemask =  pGC->planemask; 
    }
   
    if (changes & GCForeground) {
	Pixel8ToPixelWord(pGC->fgPixel);
    }
   
    if (changes & GCBackground) {
	Pixel8ToPixelWord(pGC->bgPixel);
    }
#else
   switch(pDraw->depth) {
        case 8:
            if (changes & GCPlaneMask){
		Pixel8ToPixelWord(pGC->planemask);
		ffbGCPriv->planemask = pGC->planemask;
	    }
            if (changes & GCForeground) {
		Pixel8ToPixelWord(pGC->fgPixel);
	    }
            if (changes & GCBackground) {
		Pixel8ToPixelWord(pGC->bgPixel);
	    }
            break;
        case 12:
            if (changes & GCPlaneMask) {
                pixel = pGC->planemask & FFB_12_BUF0_PLANEMASK;
                ffbGCPriv->planemask = pixel | (pixel >> 4);
	    }
            if (changes & GCForeground) {
                pixel = pGC->fgPixel & FFB_12_BUF0_PLANEMASK;
                pGC->fgPixel = pixel | (pixel >> 4);
            }
            if (changes & GCBackground) {
                pixel = pGC->bgPixel & FFB_12_BUF0_PLANEMASK;
                pGC->bgPixel = pixel | (pixel >> 4);
            }
            break;
        case 24:
            if (changes & GCPlaneMask)
                ffbGCPriv->planemask = pGC->planemask & FFB_24BIT_PLANEMASK;
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

    if(changes & (GCFunction | GCForeground | GCBackground)){
	new_rrop = TRUE;
    }

    if (changes & (GCTile)) {
	if (!pGC->tileIsPixel) {
	    PixmapPtr ntile = (PixmapPtr) ffbExpandTile(pGC->tile.pixmap);
	    if (ntile != (PixmapPtr)NULL) {
		CFB_NAME(DestroyPixmap)(pGC->tile.pixmap);
		pGC->tile.pixmap = ntile;
	    }
	    new_fillspans = TRUE;
	    new_fill = TRUE;
	}
    }

    if (changes & (GCStipple)) {
	if (pGC->stipple != NullPixmap) {
	    PixmapPtr nstipple;
	    width = pGC->stipple->drawable.width;
	    if ((width <= FFBBUSBITS && PowerOfTwo(width))
#ifdef __osf__
#if 0 && LONG_BIT==64
		|| (pGC->stipple->drawable.bitsPerPixel == 1)
#endif
		)
	    {
		/*
		 * since ffbPadPixmap() no longer modifies the stipple width...
		 * there's no reason to make a copy of it.  however, eventually,
		 * we want to allow it to special-64 pad (for stipplearea).
		 */
		(void) ffbPadPixmap(pGC->stipple);
	    }
#else /*!osf*/
		)
	    {
		nstipple = CFB_NAME(CopyPixmap)(pGC->stipple);
		if (nstipple != (PixmapPtr)NULL) {
		    CFB_NAME(DestroyPixmap)(pGC->stipple);
		    (void) CFB_NAME(PadPixmap)(nstipple);
		    pGC->stipple = nstipple;
		}
	    }
#endif
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

    if(new_rrop) {
	int old_rrop;

	old_rrop = gcPriv->rop;

	/* check the rop for clear/invert/copy/set. If one of these 4 we want to
	 * set the rop to copy and xor to be the "foreground" color. 
	 */
	switch (pGC->alu) {
	 case GXclear:
	    gcPriv->xor = 0;
	    gcPriv->and = 0;
	    gcPriv->rop = GXcopy;
	    break;
	 case GXcopyInverted:
	    gcPriv->xor = ~pGC->fgPixel;
	    gcPriv->and = ~pGC->bgPixel;
	    gcPriv->rop = GXcopy;
	    break;
	 case GXcopy:
	    gcPriv->xor = pGC->fgPixel;
	    gcPriv->and = pGC->bgPixel;
	    gcPriv->rop = GXcopy;
	    break;
	 case GXset:
	    gcPriv->xor = ~0;
	    gcPriv->and = ~0;
	    gcPriv->rop = GXcopy;
	    break;
	 default:
	    gcPriv->rop = pGC->alu;
	    break;
	}

	/* if the last rop was copy we don't need to redo the funtion pointers unless something
	 * else changed.
	 */

	if (old_rrop == gcPriv->rop)     
	    new_rrop = FALSE;
	else{
	    new_fillspans = TRUE;       
	    new_fill = TRUE;
	}
    }
    /* Even though the rop hasn't changed, if this gc is switching between
     * 8-bit packed/unpacked on 32, then dis/enable block mode according to
     * derived rop.
     *
     * Can't use BLOCK fill or stipple modes for 8-bit packed visuals on
     * a 32-bit system, so let it be known that this is so... */

    ffbGCPriv->canBlock = (gcPriv->rop == GXcopy &&
			   (scrPriv->fbdepth == 8 || pDraw->bitsPerPixel != 8));


    /* Load hardware state with current gc state. */
    scrPriv = FFBSCREENPRIV(pGC->pScreen);
    ffb = scrPriv->ffb;

    WRITE_MEMORY_BARRIER();
    LOADSTATE(pDraw, scrPriv, ffb, pGC);

    /* Deal with the changes we've collected */


    {
	Bool	changeOps = FALSE;
	int	depthSpec;

	if (pDraw->depth == 8) {
	    if (pDraw->bitsPerPixel != 8) {
		depthSpec = UNPACKED_EIGHT_DEST;
	    } else {
		depthSpec = PACKED_EIGHT_DEST;
	    }
	} else if (pDraw->depth == 12) {
	    depthSpec = TWELVE_BIT_DEST;
	} else {
	    depthSpec = TWENTYFOUR_BIT_DEST;
	}
	    
	changeOps = depthSpec != FFBGCPRIV(pGC)->depthSpec;
	changeOps |= new_line || new_text || new_fillspans || new_fill || new_rrop;
	if (changeOps) {
	    FFBGCPRIV(pGC)->depthSpec = depthSpec;
	    if (ops = MatchCommon(pGC, gcPriv)) {
		if (pGC->ops->devPrivate.val)
		    CFB_NAME(DestroyOps)(pGC->ops);
		pGC->ops = ops;
		return;
	    }
	    if (!pGC->ops->devPrivate.val) {
		pGC->ops = CFB_NAME(CreateOps)(pGC->ops);
	    }
	}
    }

    ops = pGC->ops;

    if (new_line) {
	ops->PolySegment = miPolySegment;
	ops->PolyArc = miPolyArc;
	ops->PolyRectangle = miPolyRectangle;
	if (pGC->lineStyle == LineSolid) {
	    if (pGC->lineWidth == 0) {
		if (pGC->fillStyle == FillSolid) {
		    ops->Polylines   = ffbLineSS;
		    ops->PolySegment = ffbSegmentSS;
		    ops->PolyArc = ffbZeroPolyArc;
		    ops->PolyRectangle = ffbPolyRectangle;
		} else {
		    ops->Polylines   = miZeroLine;
		}
	    } else {
		if (pGC->lineWidth == 1) {
		    ops->PolyArc = ffbPolyArc1;
		}
	    }
	} else { /* LineOnOffDash, LineDoubleDash */
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid) {
		if (ffbGCPriv->dashLength <= 16) {
		    ops->Polylines   = ffbDashLine16;
		    ops->PolySegment = ffbDashSegment16;
		} else {
		    ops->Polylines   = ffbDashLine;
		    ops->PolySegment = ffbDashSegment;
		}
		ops->PolyArc	     = miZeroPolyArc;
	    } else {
		/* miWideDash supposedly even handles 0-width correctly */
		ops->Polylines       = miWideDash;
	    }
	}
    }

    /* ffbWideLine and ffbWideDash depend upon the raster op and the number of
       clip rectangles.  It's easiest to always test for the right conditions
       to allow the ffb-specific versions than to junk in with new_line. */

    if (pGC->lineWidth > 0) {
	if (pGC->lineStyle == LineSolid) {
	    ops->Polylines = miWideLine;
	} else {
	    ops->Polylines = miWideDash;
	}
	if (ffbIdempotentALU[pGC->alu] && pGC->fillStyle == FillSolid
	    && REGION_NUM_RECTS(gcPriv->pCompositeClip) == 1) {
	    if (pGC->lineStyle == LineSolid) {
		ops->Polylines = ffbWideLine;
	    } else {
		ops->Polylines = ffbWideDash;
	    }
	}
    }

    if (new_text && (pGC->font != (FontPtr)NULL)) {
	/*
	   Fast font code can, in general, be used only for fonts with glyph
	   width <= FFBBUSBITS.  Note that ImageText ignores alu rop and fill
	   style.
	*/
	   
	/* (||| We should have max glyph width, which isn't the
	   same as the more pessimistic calculation here.  It's unlikely that
	   any single glyph spans this distance.) */


        ops->PolyGlyphBlt  = (VoidProc)ffbmiPolyGlyphBlt;
	ops->ImageGlyphBlt = miImageGlyphBlt;

	if ((FONTMAXBOUNDS(pGC->font, rightSideBearing) -
             FONTMINBOUNDS(pGC->font, leftSideBearing)) <= FFBBUSBITS
            && FONTMINBOUNDS(pGC->font, characterWidth) >= 0) {
	    /* Choose ImageGlyphBlt */
	    if (TERMINALFONT(pGC->font)) {

		/* Use special code for terminal emulator fonts */
		ops->ImageGlyphBlt = ffbTEImageGlyphBlt;
	    } else {
                ops->ImageGlyphBlt = ffbImageGlyphBlt;
	    }
	    /* Choose PolyGlyphBlt */
	    if (pGC->fillStyle == FillSolid ||
		(pGC->fillStyle == FillOpaqueStippled &&
		 pGC->fgPixel == pGC->bgPixel)) {

	        if (TERMINALFONT(pGC->font)) {
		    
		/* Use special code for terminal emulator fonts */
		    ops->PolyGlyphBlt = (VoidProc)ffbTEPolyGlyphBlt;
		} else {
		    ops->PolyGlyphBlt = (VoidProc)ffbPolyGlyphBlt;
		}
	    }
        }
    }    

    if (new_fillspans) {
	/* Which FillSpans procedure, which FillArea rectangle helper? */
	ops->PolyFillRect = ffbPolyFillRect;
	switch (pGC->fillStyle) {
	case FillSolid:
	    ops->FillSpans    = ffbSolidFillSpans;
	    ops->PolyFillRect = ffbPolyFillRectSolid;
	    break;
	case FillTiled:
	    if (!pGC->tileIsPixel) {
		int row;
		PixmapPtr tile = pGC->tile.pixmap;
		if (tile->drawable.width == FFBBLOCKTILEPIXELS &&
                    FFBGCBLOCK(pGC)) {
		    ops->FillSpans = ffbTileSpansWord;
		    ops->PolyFillRect = ffbTilePolyAreaWord;
                } else {
                    int row;

                    FFB_SELECTROW(tile->drawable.depth,
                                  tile->drawable.bitsPerPixel,
                                  pDraw->bitsPerPixel,
                                  row);
                    ffbGCPriv->FillArea = ffbCopyTab[row][_TILE_FILL_AREA];
		    FFB_SELECTROW(tile->drawable.depth, 
				  tile->drawable.bitsPerPixel,
			          pDraw->bitsPerPixel, row);
		    ops->FillSpans   = ffbCopyTab[row][_TILE_FILL_SPANS];
                }
	    }
	    break;
	case FillStippled:
	    width = pGC->stipple->drawable.width;
	    if (width <= FFBBUSBITS && PowerOfTwo(width)) {
		ops->FillSpans    = ffbTSSpansWord;
		height = pGC->stipple->drawable.height;
		if (PowerOfTwo(height)) {
		    ops->PolyFillRect = ffbTSFillAreaWord2;
		} else {
		    ops->PolyFillRect = ffbTSFillAreaWord;
		}
	    } else {
		ops->FillSpans   = ffbTSFillSpans;
		ops->PolyFillRect = ffbTSFillArea;
	    }
	    break;
	case FillOpaqueStippled:
	    if (pGC->fgPixel == pGC->bgPixel) {
		ops->PolyFillRect = ffbPolyFillRectSolid;
		ops->FillSpans    = ffbSolidFillSpans;
	    } else {
		width = pGC->stipple->drawable.width;
		if (width <= FFBBUSBITS && PowerOfTwo(width)) {
		    ops->FillSpans    = ffbOSSpansWord;
		    height = pGC->stipple->drawable.height;
		    if (PowerOfTwo(height)) {
			ops->PolyFillRect = ffbOSFillAreaWord2;
		    } else {
			ops->PolyFillRect = ffbOSFillAreaWord;
		    }
		} else {
		    ops->FillSpans   = ffbOSFillSpans;
		    ops->PolyFillRect = ffbOSFillArea;
		}
	    }
	    break;
	default:
	    FatalError("ffbValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fill) {
	/* Which polygon code? */
	if ((pGC->fillStyle == FillSolid) ||
 		(pGC->fillStyle == FillOpaqueStippled &&
		 pGC->fgPixel == pGC->bgPixel) ||
		 (pGC->stipple == (PixmapPtr)NULL)) {
	    ops->FillPolygon = ffbFillPolygon;
	    ops->PolyFillArc = ffbPolyFillArc;
	} else {
	    ops->FillPolygon = miFillPolygon;
	    ops->PolyFillArc = miPolyFillArc;
	}
    }

} /* ffbValidateGC */

/*
 * HISTORY
 */
