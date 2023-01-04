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
**
** X-2		TLB0002		Tracy Bragdon		15-Nov-1991
**		Changes for R5 font structures; use FONT* macros
**		where possible
**
**--
**/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbteplygblt.c,v 1.1.3.5 93/01/22 16:04:43 Jim_Ludwig Exp $ */

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
#include "sfb.h"
#include "sfbteglyph.h"
#include "sfbplygblt.h"
#include "sfbteplygblt.h" 

/*

Specially fast PolyGlyphBlt and ImageGlyphBlt code for fixed-metric fonts of
width <= SFBBUSBITS.  We know that:

    4 <= width <= SFBBUSBITS
    leftBearing = 0
    rightBearing = character width
    font Ascent = character Ascent
    font Descent = character Descent
    and forward-moving font (character width > 0).

This file is compiled twice: once for PolyText, and once for ImageText.

Note that ImageText ignores the graphics function, and always uses GXcopy.

*/

#ifdef FASTCPU
#ifdef SFBIMAGETEXT
static VoidProc imageGlyphs[4] = {
    /*  1.. 8 */ sfbTEImageGlyphs8,
    /*  9..16 */ sfbTEImageGlyphs16,
    /* 17..24 */ sfbTEImageGlyphs32,
    /* 25..32 */ sfbTEImageGlyphs32
};
#define ChooseGlyphProc(charWidth)  (imageGlyphs[((charWidth)-1) / 8])
#else
static VoidProc splatGlyphs[4] = {
    /*  1.. 8 */ sfbTESplatGlyphs8,
    /*  9..16 */ sfbTESplatGlyphs16,
    /* 17..24 */ sfbTESplatGlyphs32,
    /* 25..32 */ sfbTESplatGlyphs32
};
#define ChooseGlyphProc(charWidth)  (splatGlyphs[((charWidth)-1) / 8])
#endif

#else
/* Slow CPU */
#ifdef SFBIMAGETEXT
static VoidProc imageGlyphs[32] = {
    /*  1 */    sfbTEImageGlyphs6,
    /*  2 */    sfbTEImageGlyphs6,
    /*  3 */    sfbTEImageGlyphs6,
    /*  4 */    sfbTEImageGlyphs6,
    /*  5 */    sfbTEImageGlyphs6,
    /*  6 */    sfbTEImageGlyphs6,

    /*  7 */    sfbTEImageGlyphs8,
    /*  8 */    sfbTEImageGlyphs8,

    /*  9 */    sfbTEImageGlyphs12,
    /* 10 */    sfbTEImageGlyphs12,
    /* 11 */    sfbTEImageGlyphs12,
    /* 12 */    sfbTEImageGlyphs12,

    /* 13 */    sfbTEImageGlyphs16,
    /* 14 */    sfbTEImageGlyphs16,
    /* 15 */    sfbTEImageGlyphs16,
    /* 16 */    sfbTEImageGlyphs16,

    /* 17 */    sfbTEImageGlyphs32, sfbTEImageGlyphs32, sfbTEImageGlyphs32,
    /* 20 */    sfbTEImageGlyphs32, sfbTEImageGlyphs32, sfbTEImageGlyphs32,
    /* 23 */    sfbTEImageGlyphs32, sfbTEImageGlyphs32, sfbTEImageGlyphs32,
    /* 26 */    sfbTEImageGlyphs32, sfbTEImageGlyphs32, sfbTEImageGlyphs32,
    /* 29 */    sfbTEImageGlyphs32, sfbTEImageGlyphs32, sfbTEImageGlyphs32,
    /* 32 */    sfbTEImageGlyphs32
};
#define ChooseGlyphProc(charWidth)  (imageGlyphs[(charWidth)-1])
#else
static VoidProc splatGlyphs[32] = {
    /*  1 */    sfbTESplatGlyphs6,
    /*  2 */    sfbTESplatGlyphs6,
    /*  3 */    sfbTESplatGlyphs6,
    /*  4 */    sfbTESplatGlyphs6,
    /*  5 */    sfbTESplatGlyphs6,
    /*  6 */    sfbTESplatGlyphs6,

    /*  7 */    sfbTESplatGlyphs8,
    /*  8 */    sfbTESplatGlyphs8,

    /*  9 */    sfbTESplatGlyphs12,
    /* 10 */    sfbTESplatGlyphs12,
    /* 11 */    sfbTESplatGlyphs12,
    /* 12 */    sfbTESplatGlyphs12,

    /* 13 */    sfbTESplatGlyphs16,
    /* 14 */    sfbTESplatGlyphs16,
    /* 15 */    sfbTESplatGlyphs16,
    /* 16 */    sfbTESplatGlyphs16,

    /* 17 */    sfbTESplatGlyphs32, sfbTESplatGlyphs32, sfbTESplatGlyphs32,
    /* 20 */    sfbTESplatGlyphs32, sfbTESplatGlyphs32, sfbTESplatGlyphs32,
    /* 23 */    sfbTESplatGlyphs32, sfbTESplatGlyphs32, sfbTESplatGlyphs32,
    /* 26 */    sfbTESplatGlyphs32, sfbTESplatGlyphs32, sfbTESplatGlyphs32,
    /* 29 */    sfbTESplatGlyphs32, sfbTESplatGlyphs32, sfbTESplatGlyphs32,
    /* 32 */    sfbTESplatGlyphs32
};
#define ChooseGlyphProc(charWidth)  (splatGlyphs[(charWidth)-1])
#endif
#endif /* FASTCPU ... else ... */


void SFBTEPOLYGLYPHBLT(pDrawable, pGC, x, y, nglyph, ppci)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    int			x, y;
    unsigned int	nglyph;
    CharInfoPtr		*ppci;		/* array of character info	    */
{
    BoxRec		bbox;		/* string's bounding box	    */
    int			widthDst;	/* width of dst in bytes	    */
    Pixel8		*pdstBase;	/* first byte of current glyph in dst */
    register Pixel8     *pdst;		/* pointer to current byte in dst   */

    int			charAscent;
    int			charDescent;
    int			charWidth;
    cfbPrivGC		*gcPriv;
    VoidProc		paintGlyphs;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

    if (nglyph == 0) return;

    x += pDrawable->x;
    y += pDrawable->y;
    
    DrawableBaseAndWidth(pDrawable, pdstBase, widthDst);

     charAscent = FONTASCENT(pGC->font);
     charWidth = FONTMINBOUNDS(pGC->font, rightSideBearing);
     charDescent = FONTDESCENT(pGC->font);

    /* Make pdstBase point to upper left bit of first glyph */
    pdstBase += (widthDst * (y-charAscent)) + x * SFBPIXELBYTES;

    bbox.x1 = x;
    bbox.x2 = x + (nglyph * charWidth);
    bbox.y1 = y - charAscent;
    bbox.y2 = y + charDescent;

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
#ifdef SFBIMAGETEXT
    SFBROP(sfb, GXcopy);
    SFBMODE(sfb, OPAQUESTIPPLE);
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
    paintGlyphs = ChooseGlyphProc(charWidth);

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    switch ((*pGC->pScreen->RectIn)(gcPriv->pCompositeClip, &bbox)) {
      case rgnOUT:
	break;

      case rgnIN:
#ifdef SFBIMAGETEXT
	(*paintGlyphs) (pdstBase, widthDst, nglyph, ppci, sfb);
#else
	(*paintGlyphs) (pdstBase, widthDst, nglyph, ppci);
#endif
	break;

      case rgnPART:
	x -= pDrawable->x;
	y -= pDrawable->y;
	/* ||| This is so sleazy it's paintful.  I should do a fixed-metric
	   version of clipping, which would be a lot faster. */
#ifdef SFBIMAGETEXT
	sfbImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci);
#else
	sfbPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci);
#endif
	break;

      default:
	break;
    }
#ifdef SFBIMAGETEXT
    SFBROP(sfb, pGC->alu);
#endif
}
