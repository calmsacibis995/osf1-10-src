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
#define IDENT "X-004"
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
**
**  MODIFICATION HISTORY:
**
** X-004        DMC0011         Dave Coleman                    29-Jan-1992
**              Fix typecasting to correct arithmetic error.
**
** X-003	DMC0004 	Dave Coleman			20-Dec-1991
**		Add explicit typecaseting to avoid errors during
**		Ultrix compilation.
**
** X-2		TLB0002		Tracy Bragdon		15-Nov-1991
**		Changes for R5 font structures; use FONT* macros
**		where possible
**
**--
**/

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/sfb/sfbplygblt.c,v 1.1.5.2 1993/04/19 12:08:39 Don_Haney Exp $ */

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
#include "regionstr.h"
#include "miscstruct.h"

#include "sfb.h"
#include "sfbplygblt.h"
#include "sfbpntarea.h"
#include "cfbmskbits.h"

/*

This code works for variable-pitch fonts with glyphs where the
maximum width <= SFBBUSBITS bits.

The clipping calculations are done for worst-case fonts, with no assumptions
about the heights, widths, or bearings of the glyphs.  It is possible, for
example, for a font to be defined in which the next-to-last character in a font
would be clipped out, but the last one wouldn't.  The code below deals with
this.

This file is compiled twice: once for PolyText, and once for ImageText.

Note that ImageText ignores the graphics function, and always uses GXcopy.

*/

void SFBPOLYGLYPHBLT(pDrawable, pGC, x, y, nglyph, ppci)
    DrawablePtr     pDrawable;
    GCPtr	    pGC;
    int		    x, y;
    unsigned int    nglyph;
    CharInfoPtr     *ppci;		/* array of character info	      */
{
    BoxRec	    bbox;		/* string's bounding box	      */
    register CharInfoPtr pci;
    int		    widthDst;		/* width of dst in bytes	      */
    Pixel8	    *pdstBase;		/* first byte of current glyph in dst */
    register Pixel8 *pdst;		/* pointer to current byte in dst     */

    int		    w;			/* width of glyph in bits	      */
    int		    h;			/* height of glyph		      */
    int		    widthGlyph;		/* width of glyph, in bytes	      */
    register Bits8  *pglyph;		/* pointer to current row of glyph    */
    register CommandWord glyphbits;	/* Actual bits of glyph row	      */
    int overallLeft, overallRight, overallWidth, i;
    cfbPrivGC       *gcPriv;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

#ifdef SFBIMAGETEXT
    int xorg;
    int yorg;
    xRectangle backrect;		/* backing rectangle to paint, which is
					   defined by the FONT's ascent and
					   descent, and the CHARACTER widths.
					   NOT necessarily the same as the 
					   string's bounding box. */
    BoxRec backbox;			/* Same thing as a box */

    xorg = pDrawable->x;
    yorg = pDrawable->y;
#else
    x += pDrawable->x;
    y += pDrawable->y;
#endif

    DrawableBaseAndWidth(pDrawable, pdstBase, widthDst);

    /* Quickly compute a (possibly non-minimal) bounding box for string. */
    overallWidth = 0;
    if (-FONTMINBOUNDS(pGC->font, leftSideBearing) 
	<= FONTMINBOUNDS(pGC->font, characterWidth)) {
	/* No character can extend to the left of the previous character. */
	overallLeft = 0;
	if ((ppci[0]->metrics.leftSideBearing < 0))
	    overallLeft = ppci[0]->metrics.leftSideBearing;
	for (i = 0; i != nglyph-1; i++) {
	    overallWidth += ppci[i]->metrics.characterWidth;   

	}
	overallRight = overallWidth + ppci[nglyph-1]->metrics.rightSideBearing;
	overallWidth += ppci[nglyph-1]->metrics.characterWidth;  
    } else {
	/* Weird font, have to be more careful about box computations. */
	overallLeft = 0;
	overallRight = 0;
	for (i = 0; i <= nglyph-1; i++) {
	    int t;
	    t = overallWidth + ppci[i]->metrics.leftSideBearing;
	    if (overallLeft > t) overallLeft = t;
	    t = overallWidth + ppci[i]->metrics.rightSideBearing;
	    if (overallRight < t) overallRight = t;
	    overallWidth += ppci[i]->metrics.characterWidth;
	}
    }

#ifdef SFBIMAGETEXT
    backrect.x = x;
    backrect.width = overallWidth;
    backrect.y = y - FONTASCENT(pGC->font);
    backrect.height = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);

    x += xorg;
    y += yorg;
    backbox.x1 = x;
    backbox.y1 = backrect.y + yorg;
    backbox.x2 = backbox.x1 + backrect.width;
    backbox.y2 = backbox.y1 + backrect.height;
#endif
    bbox.x1 = x + overallLeft;
    bbox.x2 = x + overallRight;
    bbox.y1 = y - FONTASCENT(pGC->font);
    bbox.y2 = y + FONTDESCENT(pGC->font);

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);

    gcPriv      = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);

#ifdef SFBIMAGETEXT
    /* UNCLEAN CODE.

       sfbPolyFillRectSolid expects foreground and rop already set on the
       sfb.  We'll paint the background rectangle using bgPixel in GXcopy mode,
       restore foreground, paint the text, and finally restore the rop. 
    */

    /* Clear out the bounding box with the background pixel */
    if (((*pGC->pScreen->RectIn) (gcPriv->pCompositeClip, &backbox)) == rgnOUT){
	    return;
    }
    /* Call sfbPolyFillRectSolid  */
    SFBFOREGROUND(sfb, pGC->bgPixel);
    SFBROP(sfb, GXcopy);
    sfbPolyFillRectSolid(pDrawable, pGC, 1, &backrect);
    SFBFOREGROUND(sfb, pGC->fgPixel);
#endif

    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    /* Switch on bounding box completely covered, completely visible, or
       (yuch) partially visible. */
    switch ((*pGC->pScreen->RectIn)(gcPriv->pCompositeClip, &bbox)) {
      case rgnOUT:
	break;

      case rgnIN:
	/* Box is completely visible.  Just splat string onto the screen. */
        pdstBase += widthDst * y + x * SFBPIXELBYTES;
	sfbSplatGlyphs(pdstBase, widthDst, nglyph, ppci);
	break;

      case rgnPART:
	/* Box is partially visible.  Construct an array of information about
	   each glyph, then compute runs of completely unclipped characters.
	   Display each run with the fast SFBSPLATGLYPHS, and poke along with
	   slow code for each partially clipped character.
	*/
      {
	RegionRec	    region;     /* Intersection of clip and bbox    */
	TEXTPOS		    *anchor;    /* first unclipped char		    */
	int		    anchori;    /* index of first unclipped char    */
	TEXTPOS		    *ppos;      /* array of character info	    */
	register TEXTPOS    *tpos;      /* current character info	    */
	int		    nbox;       /* number of clip rectangles	    */
	BoxPtr		    pbox;       /* current clip rectangle	    */
	int		    xpos;	/* x position of char origin */
	int i;
	BoxRec		    clip;       /* clip bounds for current char     */
	int		    leftEdge, rightEdge;
	int		    topEdge, bottomEdge;
	int		    glyphRow;   /* first row of glyph not clipped out */
	int		    glyphCol;   /* leftmost visible column of glyph */
	register CommandWord glyphMask;  /* Mask for junk and clippedbits */

	if(!(ppos = (TEXTPOS *)ALLOCATE_LOCAL(nglyph * sizeof(TEXTPOS)))) {
	    break;
	}

	/* Compute intersection of clip region and bbox to get small region. */
	(*pGC->pScreen->RegionInit)(&region, &bbox, 1);
	(*pGC->pScreen->Intersect)(&region, &region, gcPriv->pCompositeClip);
	
	pdstBase += widthDst * y + x * SFBPIXELBYTES;
	xpos = x;

	/* Compute info for each character */
	for (tpos = ppos, i=0; i<nglyph; tpos++, i++) {
	    pci = ppci[i];

	    tpos->xpos = xpos;
	    tpos->leftEdge = xpos + pci->metrics.leftSideBearing;
	    tpos->rightEdge = xpos + pci->metrics.rightSideBearing;
	    tpos->topEdge = y - pci->metrics.ascent;
	    tpos->bottomEdge = y + pci->metrics.descent;
	    tpos->pdstBase = (unsigned long *) pdstBase;
	    tpos->widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	    xpos += pci->metrics.characterWidth;
	    pdstBase += pci->metrics.characterWidth * SFBPIXELBYTES;
	}

	pbox = REGION_RECTS(&region);
	nbox = REGION_NUM_RECTS(&region);

	for (; --nbox >= 0; pbox++) {
	    Bool clipTopOrBottom;

	    clip.x1 = max(bbox.x1, pbox->x1);
	    clip.y1 = max(bbox.y1, pbox->y1);
	    clip.x2 = min(bbox.x2, pbox->x2);
	    clip.y2 = min(bbox.y2, pbox->y2);

	    if ((clip.x2<=clip.x1) || (clip.y2<=clip.y1))
		continue;

	    /* Clip rectangle at least partially overlaps the string */
	    if (clip.y1 == bbox.y1 && clip.y2 == bbox.y2) {
		clipTopOrBottom = FALSE;
	    } else {
		clipTopOrBottom = TRUE;
	    }

	    anchor = ppos;
	    anchori = 0;

	    for (tpos = ppos, i=0; i != nglyph; tpos++, i++) {
		int align;

		pci = ppci[i];

		/* Is the complete character visible? */
		if (clip.x1 <= tpos->leftEdge &&
		    clip.x2 >= tpos->rightEdge &&
		    clip.y1 <= tpos->topEdge &&
		    clip.y2 >= tpos->bottomEdge) {
		    /* Display it with SFBSPLATGLYPHS later. */
		    continue;
		}

#define SplatUnclippedGlyphs()						    \
{									    \
    int length = tpos - anchor;						    \
    if (length > 0) {							    \
	sfbSplatGlyphs(anchor->pdstBase, widthDst, length, &ppci[anchori]); \
    }									    \
} /* SplatUnclippedGlyphs */

		/* Character partially visible.  Display all unclipped
		   characters so far, and reset the anchor counters. */
		SplatUnclippedGlyphs();
		anchor = tpos + 1;
		anchori = i + 1;

		/* clip the left and right edges */
		leftEdge = tpos->leftEdge;
		if (leftEdge < clip.x1)
		    leftEdge = clip.x1;

		rightEdge = tpos->rightEdge;
		if (rightEdge > clip.x2)
		    rightEdge = clip.x2;

		w = rightEdge - leftEdge;
		if (w <= 0) {
		    continue;
		}
		/* clip the top and bottom edges */
		topEdge = tpos->topEdge;
		bottomEdge = tpos->bottomEdge;
		if (clipTopOrBottom) {
		    if (topEdge < clip.y1)
			topEdge = clip.y1;
		    if (bottomEdge > clip.y2)
			bottomEdge = clip.y2;
		    if (topEdge >= bottomEdge) {
			continue;
		    }
		}
		h = bottomEdge - topEdge;
		if ( h <= 0) continue;

		glyphRow = (topEdge - y) + pci->metrics.ascent;
		widthGlyph = tpos->widthGlyph;
		pglyph = SFBGLYPHBITS(pci);
		if ( pglyph == (Bits8 *)NULL ) continue;

		pglyph += glyphRow * widthGlyph;

		pdst = (Pixel8 *) tpos->pdstBase
			- ((y-topEdge) * widthDst)
			+ (leftEdge - tpos->xpos) * SFBPIXELBYTES;

		/* Align pdst to SFB boundary.  Keep track of how many
		   extra pixels this adds...we compensate for these
		   pixels by shifting glyphbits left by this amount.  
		   Since we shift in 0's, this doesn't cause anything
		   bad to get written. */
		align = ((int) pdst) & SFBALIGNMASK;
		pdst -= align;
		SFBBYTESTOPIXELS(align);
		glyphCol =
		    (leftEdge - tpos->xpos) - pci->metrics.leftSideBearing;

		/* glyphMask masks out unwanted bits fetched, as well as
		   low bits that are clipped by the right edge */
		w = rightEdge - tpos->leftEdge;
		glyphMask = SFBRIGHTBUSMASK(w, SFBBUSALL1);
		
		if (w + align <= SFBSTIPPLEBITS) {
		    /* Everything can be done in one write. */
		    do {
			/* Fetch one row of the glyph's bits */
			getleftbits(pglyph, w, glyphbits);
			glyphbits &= glyphMask;
			/* Shift bits screen left to clip left side */
			glyphbits >>= glyphCol;
			SFBWRITE(pdst, glyphbits << align);
			pdst += widthDst;
			pglyph += widthGlyph;
			h--;
		    } while (h > 0);
#if SFBBUSBITS + SFBALIGNMASK <= 2 * SFBSTIPPLEBITS
		} else {
#else
		} else if (w + align <= 2 * SFBSTIPPLEBITS) {
#endif
		    /* Need two or more writes. */
		    int rightShift = SFBRIGHTSTIPPLESHIFT(align);
		    do {
			/* Fetch one row of the glyph's bits */
			getleftbits(pglyph, w, glyphbits);
			glyphbits &= glyphMask;
			/* Shift bits screen left to clip left side */
			glyphbits >>= glyphCol;
			SFBWRITE(pdst, glyphbits << align);
			glyphbits >>= rightShift;
			SFBWRITE(pdst+SFBSTIPPLEBYTESDONE,	glyphbits);
			pdst += widthDst;
			pglyph += widthGlyph;
			h--;
		    } while (h > 0);
#if SFBBUSBITS + SFBALIGNMASK > 2 * SFBSTIPPLEBITS
		} else {
		    /* Need more than two writes, very gross. */
		    int rightShift = SFBRIGHTSTIPPLESHIFT(align);
		    do {
			Pixel8  *p;

			/* Fetch one row of the glyph's bits */
			getleftbits(pglyph, w, glyphbits);
			glyphbits &= glyphMask;
			/* Shift bits screen left to clip left side */
			glyphbits >>= glyphCol;
			SFBWRITE(pdst, glyphbits << align);
			glyphbits >>= rightShift;
			p = pdst + SFBSTIPPLEBYTESDONE;
			while (glyphbits != 0) {
			    SFBWRITE(p, glyphbits);
			    p += SFBSTIPPLEBYTESDONE;
			    glyphbits >>= SFBSTIPPLEBITS;
			}
			pdst += widthDst;
			pglyph += widthGlyph;
			h--;
		    } while (h > 0);
#endif
		}
	    } /* for each glyph */

	    /* Now display any unclipped chars at end of string */
	    SplatUnclippedGlyphs();
	} /* while nbox-- */
	(*pGC->pScreen->RegionUninit)(&region);
	DEALLOCATE_LOCAL(ppos);
	break;
      }
      default:
	break;
    }
#ifdef SFBIMAGETEXT
    SFBROP(sfb, pGC->alu);
#endif
}
