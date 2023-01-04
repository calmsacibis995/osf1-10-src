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
#define IDENT "X-003"
#define MODULE_NAME SFBGLYPH
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
** X-003        DMC0010         Dave Coleman                    29-Jan-1992
**              Correct increment of glyph scanline pointer.
**
** X-2		TLB0002		Tracy Bragdon		15-Nov-1991
**		Changes for R5 font structures
**
**--
**/

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/sfb/sfbglyph.c,v 1.1.7.2 1993/08/23 14:16:03 Robert_Lembree Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"

#ifdef MITR5
#include "dixfontstr.h"
#else	/* MITR5 */
#include "fontstr.h"
#endif	/* MITR5 */

#include "sfb.h"
#include "sfbglyph.h"
#include "cfbmskbits.h"
#include "sfbplygblt.h"

/*
    Actually paint unclipped variable-pitch glyphs onto the screen.  
    Separated out from sfbpglyblt.c so that terminal-emulator (fixed metric)
    versions could be put into assembly language.  Yummmmm.
*/

void sfbSplatGlyphs(pdstBase, widthDst, nglyph, ppci)
    register Pixel8	    *pdstBase;	/* first byte of current glyph in dst */
    register int	    widthDst;	/* width of dst in bytes	      */
    unsigned int	    nglyph;	/* Number of glyphs to paint	      */
    register CharInfoPtr    *ppci;	/* array of character info	      */
{
    register Pixel8	    *pdst;	/* pointer to current byte in dst     */
    register int	    align;	/* Alignment of pdst pointer	      */
    register int	    w;		/* width of glyph in bits	      */
    register int	    widthGlyph;	/* width of glyph in bytes	      */
    register int	    h;		/* height of glyph in bits	      */
    register Bits8	    *pglyph;	/* pointer to current row of glyph    */
    register CommandWord    glyphbits;  /* Actual bits of glyph row	      */
    register CharInfoPtr    pci;
    register CommandWord    glyphMask;  /* Mask to zero right junk bits       */
    register CommandWord    ones = sfbBusAll1;

    while (nglyph != 0) {
	pci = *ppci;

	pglyph = SFBGLYPHBITS(pci);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	h = pci->metrics.ascent + pci->metrics.descent;

	/* find pixel for top left bit of glyph */
	pdst = pdstBase - pci->metrics.ascent * widthDst 
	       + pci->metrics.leftSideBearing * SFBPIXELBYTES;

	/* Align pdst to word boundary.  Keep track of how many extra
	   pixels this adds...we compensate for these pixels by shifting
	   glyphbits left by this amount.  Since we shift in 0's, this
	   doesn't cause anything bad to get written. */
	align = ((int) pdst) & SFBALIGNMASK;
	pdst -= align;
	SFBBYTESTOPIXELS(align);

	if ( pglyph != (Bits8 *)NULL ) {

	  if (w <= 8) {
#if (8 + SFBALIGNMASK) > SFBSTIPPLEBITS
	    if (w + align <= SFBSTIPPLEBITS) {
#endif
		/* Shifted data fits in a single word, needs single write */
		if (h & 1) {
		    /* One row of the glyph's bits */
		    glyphbits = *pglyph;
		    /* w <= 8 here */
		    pglyph += GLYPHPADBYTES;
		    SFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    h--;
		}
		while (h > 0) {
		    register CommandWord glyphbits1;
		    /* Two rows of the glyph's bits */
		    glyphbits = ((unsigned short *)pglyph)[0];
		    glyphbits1 = ((unsigned short *)pglyph)[2];
		    SFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    SFBWRITE(pdst, glyphbits1 << align);
		    pdst += widthDst;

		    /* w <= 8 here */
		    pglyph += 2*GLYPHPADBYTES;
		    h -= 2;
		} 
#if 8 + SFBALIGN > SFBSTIPPLEBITS
	      } else {
		/* Shifted data fits in a single word, needs two writes. */
		int rightShift = SFBRIGHTSTIPPLESHIFT(align);
		do {
		    /* One row of the glyph's bits */
		    glyphbits = *pglyph;
		    pglyph += GLYPHPADBYTES;
		    SFBWRITE(pdst, glyphbits << align);
		    glyphbits >>= rightShift;
		    SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, glyphbits);
		    pdst += widthDst;
		    h--;
		} while (h > 0);
	      }
#endif
	  } else if (w + align <= SFBSTIPPLEBITS) {
	    /* Shifted data fits into a single word, needs a single write */
	    widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

#if GLYPHPADBYTES < 2
#define GLYPHBYTES_WIDTH2	2
#else	/* GLYPHPADBYTES == 4 or 2 */
#define GLYPHBYTES_WIDTH2	GLYPHPADBYTES
#endif	/* GLYPHPADBYTES */

	    if ((widthGlyph == 2) && (((int)pglyph & 1) == 0)) {
		/* what sleaze.  We can use 16-bit loads safely. */
		if (h & 1) {
		    /* One row of the glyph's bits */
		    glyphbits = ((unsigned short *)pglyph)[0];
		    pglyph += GLYPHBYTES_WIDTH2;
		    SFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    h--;
		}
		while (h > 0) {
		    /* Two row of the glyph's bits */
		    register CommandWord glyphbits1;
		    glyphbits = ((unsigned short *)pglyph)[0];
		    glyphbits1 = ((unsigned short *)pglyph)[1];
		    pglyph += 2 * GLYPHBYTES_WIDTH2;
		    SFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    SFBWRITE(pdst, glyphbits1 << align);
		    pdst += widthDst;
		    h -= 2;
		}
	    } else {
		/* use getleftbits because yucky byte-aligned glyphs. */
		glyphMask = SFBRIGHTBUSMASK(w, ones);
		do {
		    /* One row of the glyph's bits */
		    getleftbits(pglyph, w, glyphbits);
		    glyphbits &= glyphMask;
		    pglyph += widthGlyph;
		    SFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    h--;
	        } while (h > 0);
	    }
#if SFBBUSBITS + SFBALIGNMASK <= 2*SFBSTIPPLEBITS
	  } else {
#else
	  } else if (w + align <= 2 * SFBSTIPPLEBITS) {
#endif
	    /* Shifted data requires two writes. */
	    int rightShift = SFBRIGHTSTIPPLESHIFT(align);
	    widthGlyph = GLYPHWIDTHBYTESPADDED(pci);
	    glyphMask = SFBRIGHTBUSMASK(w, ones);
	    do {
		/* One row of the glyph's bits */
		getleftbits(pglyph, w, glyphbits);
		glyphbits &= glyphMask;
		pglyph += widthGlyph;
		SFBWRITE(pdst, glyphbits << align);
		/* And now the remaining bits */
		SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, glyphbits >> rightShift);
		pdst += widthDst;
		h--;
	    } while (h > 0);
#if SFBBUSBITS + SFBALIGNMASK > 2*SFBSTIPPLEBITS
	  } else {
	    /* Need more than two writes, very gross. */
	    int rightShift = SFBRIGHTSTIPPLESHIFT(align);
	    widthGlyph = GLYPHWIDTHBYTESPADDED(pci);
	    glyphMask = SFBRIGHTBUSMASK(w, ones);
	    do {
		Pixel8  *p;

		/* Fetch one row of the glyph's bits */
		getleftbits(pglyph, w, glyphbits);
		glyphbits &= glyphMask;
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
	}
	/* Move pdstBase to next character */
	pdstBase += pci->metrics.characterWidth * SFBPIXELBYTES;
	pdstBase = CYCLE_FB(pdstBase);
	
	/* Move ppci to next character info */
	ppci++;
	nglyph--;
    } /* while nglyph */
} /* sfbSplatGlyphs */
