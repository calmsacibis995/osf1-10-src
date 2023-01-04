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
static char *rcsid = "@(#)$RCSfile: ffbglyph.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:11:08 $";
#endif
/*
 */

#include "X.h"
#include "Xproto.h"
#include "misc.h"

#include "dixfontstr.h"

#include "ffb.h"
#include "ffbglyph.h"
#include "ffbplygblt.h"
#include "cfbmskbits.h"

#if GLYPHPADBYTES == 4
#   define GLYPHBITS31(pglyph)  (*((int *)(pglyph)))
#   define GLYPHBITS32(pglyph)  (*((unsigned int *)(pglyph)))
#else
Sorry, but right now this code won't work with GLYPHPADBYTES < 4.
#endif


/*
    Actually paint unclipped variable-pitch glyphs onto the screen.  
    Separated out from ffbpglyblt.c so that terminal-emulator (fixed metric)
    versions could be put into assembly language on MIPS.  Yummmmm.
*/
void ffbSplatGlyphs(pdstBase, widthDst, nglyph, ppci)
    register Pixel8	    *pdstBase;	/* first byte of current glyph in dst */
    register int	    widthDst;	/* width of dst in bytes	      */
    unsigned int	    nglyph;	/* Number of glyphs to paint	      */
    register CharInfoPtr    *ppci;	/* array of character info	      */
{
    register Pixel8	    *pdst;	/* pointer to current byte in dst     */
    register int	    align;	/* Alignment of pdst pointer	      */
    register int	    w;		/* width of glyph in bits	      */
    register int	    h;		/* height of glyph in bits	      */
    register Bits8	    *pglyph;	/* pointer to current row of glyph    */
    register CommandWord    glyphbits;  /* Actual bits of glyph row	      */
    register CharInfoPtr    pci;
    register CommandWord    glyphMask;  /* Mask to zero right junk bits       */
    register CommandWord    ones = ffbBusAll1;

    WRITE_MEMORY_BARRIER();
    while (nglyph != 0) {
	pci = *ppci;

	pglyph = FFBGLYPHBITS(pci);
	w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	h = pci->metrics.ascent + pci->metrics.descent;

	/* find pixel for top left bit of glyph */
	pdstBase = CYCLE_FB(pdstBase);
	pdst = pdstBase - pci->metrics.ascent * widthDst 
	       + pci->metrics.leftSideBearing * FFBPIXELBYTES;

	/* Align pdst to word boundary.  Keep track of how many extra
	   pixels this adds...we compensate for these pixels by shifting
	   glyphbits left by this amount.  Since we shift in 0's, this
	   doesn't cause anything bad to get written. */
	align = ((long) pdst) & FFBSTIPPLEALIGNMASK; 
	pdst -= align;
	FFBBYTESTOPIXELS(align);

/* |||	if ( pglyph == (Bits8 *)NULL ) continue;  */
	
	if (w + align <= FFBSTIPPLEBITS) {
	    /* Shifted data fits in a single word, needs single write */
	    if (w <= 31) {
		/* Can do superfast glyphbits load */
		if (h & 1) {
		    /* One row of the glyph's bits */
		    glyphbits = GLYPHBITS31(pglyph);
		    pglyph += GLYPHPADBYTES;
		    FFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    h--;
		}
		while (h > 0) {
		    register CommandWord glyphbits1;
		    Pixel8 *p1;
		    /* Two rows of the glyph's bits */
		    glyphbits = GLYPHBITS31(pglyph);
		    glyphbits1 = GLYPHBITS31(pglyph+GLYPHPADBYTES);
		    FFBWRITE(pdst, glyphbits << align);
		    p1 = pdst + widthDst;
		    FFBWRITE(p1, glyphbits1 << align);
		    pdst = p1 + widthDst;
		    pglyph += 2*GLYPHPADBYTES;
		    h -= 2;
		}
	    } else {
		/* Have to do slower glyphbits load */
		if (h & 1) {
		    /* One row of the glyph's bits */
		    glyphbits = GLYPHBITS32(pglyph);
		    pglyph += GLYPHPADBYTES;
		    FFBWRITE(pdst, glyphbits << align);
		    pdst += widthDst;
		    h--;
		}
		while (h > 0) {
		    register CommandWord glyphbits1;
		    Pixel8 *p;
		    /* Two rows of the glyph's bits */
		    glyphbits = GLYPHBITS32(pglyph);
		    glyphbits1 = GLYPHBITS32(pglyph+GLYPHPADBYTES);
		    FFBWRITE(pdst, glyphbits << align);
		    p = pdst + widthDst;
		    FFBWRITE(p, glyphbits1 << align);
		    pdst = p + widthDst;
		    pglyph += 2*GLYPHPADBYTES;
		    h -= 2;
		}
	    } /* if fast loads else slow loads */

	} else {
	    /* Shifted data requires two writes. */
	    int rightShift = FFBRIGHTSTIPPLESHIFT(align);
	    do {
		/* One row of the glyph's bits */
		glyphbits = GLYPHBITS32(pglyph);
		pglyph += GLYPHPADBYTES;
		FFBWRITE(pdst, glyphbits << align);
		/* And now the remaining bits */
		FFBWRITE(pdst+FFBSTIPPLEBYTESDONE, glyphbits >> rightShift);
		pdst += widthDst;
		h--;
	    } while (h > 0);
	}
	
	/* Move pdstBase to next character */
	pdstBase += pci->metrics.characterWidth * FFBPIXELBYTES;

	/* Move ppci to next character info */
	ppci++;
	nglyph--;
    } /* while nglyph */
} /* ffbSplatGlyphs */


/*
 * HISTORY
 */
