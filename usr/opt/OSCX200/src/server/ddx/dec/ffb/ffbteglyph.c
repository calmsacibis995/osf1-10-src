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
static char *rcsid = "@(#)$RCSfile: ffbteglyph.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:17:58 $";
#endif
/*
 */

#include "X.h"
#include "Xproto.h"
#include "misc.h"

#include "dixfontstr.h"

#include "ffb.h"
#include "ffbteglyph.h"

/*

Paint unclipped fixed-metric glyphs onto the screen.  Separated out
from ffbpglyblt.c so it could be put into assembly language.  Yummmmm.

The philosophy here is to try to paint a bunch of glyphs at a time, so that we
always have FFBBUSBITS of glyph information, and thus minimize writes out to the
TURBOChannel.  At the beginning of the string, we have to deal with the left
alignment, which restricts the number of glyphs we can use on the scanline.
For the rest of the string, we have to deal with whatever residue is left over
>from the previous glyph.  In both cases, we have to deal with the possibility
that there are fewer glyphs left to paint than we'd like.

We also try to keep several glyph pointers in registers.  The number of
glyphs we are guaranteed to load each time through the loop depends upon the
maximum glyph width handled.

This file is compiled a few times, in order to pick up glyph infomation as
painlessly as possible:

FFBTESPLATGLYPHS	glyph widths handled
--------------------------------------------
ffbTESplatGlyphs8	     4-8
ffbTESplatGlyphs16	     9-16
ffbTESplatGlyphs32	    17-32

*/

#if GLYPHPADBYTES == 4
#   if GLYPHWIDTH <= 31
#       define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph)     \
{									\
    newbits = *((int *)(pglyph));					\
    pglyph += GLYPHPADBYTES;						\
} /* GETGLYPHBITS */
#   else
#       define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph)     \
{									\
    newbits = *((unsigned int *)(pglyph));				\
    pglyph += GLYPHPADBYTES;						\
} /* GETGLYPHBITS */
#   endif

#else /* GLYPHPADBYTES is some yucky small number */
#   if GLYPHWIDTH <= 8
#       define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = *(pglyph);						\
    pglyph += GLYPHPADBYTES;						\
} /* GETGLYPHBITS */

#   elif GLYPHWIDTH <= 16
#       define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = *((unsigned short *) (pglyph));				\
    pglyph += 2;  							\
} /* GETGLYPHBITS */

#   elif GLYPHWIDTH <= 32
#       define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = Ulw(pglyph) & (glyphMask);				\
    pglyph += widthGlyph;						\
} /* GETGLYPHBITS */
#   endif
#endif

#ifdef Bits64
#   define BitsWord    Bits64
#else
#   define BitsWord    Bits32
#endif

#if NGLYPHS * GLYPHWIDTH + FFBSTIPPLEALIGNMASK > FFBBUSBITS
#   define FFBWRITEGLYPH(pdst, glyphbits)				    \
{									    \
    FFBWRITE(pdst, glyphbits);						    \
    FFBWRITE(pdst + FFBBUSBITS * FFBPIXELBYTES, (glyphbits) >> FFBBUSBITS); \
}
#else
#   define FFBWRITEGLYPH(pdst, glyphbits) FFBWRITE(pdst, glyphbits)
#endif

#ifdef FFBIMAGETEXT
void FFBTESPLATGLYPHS(pdstBase, widthDst, nglyph, ppci, ffb)
    Pixel8	    *pdstBase;	/* first byte of current glyph in dst */
    int		    widthDst;	/* width of dst in bytes	      */
    int		    nglyph;	/* Number of glyphs to paint	      */
    CharInfoPtr     *ppci;	/* array of character info	      */
    FFB		    ffb;
#else
void FFBTESPLATGLYPHS(pdstBase, widthDst, nglyph, ppci)
    Pixel8	    *pdstBase;	/* first byte of current glyph in dst */
    int		    widthDst;	/* width of dst in bytes	      */
    int		    nglyph;	/* Number of glyphs to paint	      */
    CharInfoPtr    *ppci;	/* array of character info	      */
#endif


/*
Paint the same number of characters each interation; the number of characters
is fixed at the maximum allowed under the worst alignment contraints.  That is,
NGLYPHS = (WORDBITS-FFBSTIPPLEALIGNMASK)/GLYPHWIDTH.

Then paint the last few characters in the string.

*/


{
    Pixel8	    *pdst;	/* pointer to current byte in dst     */
    Pixel8	    *pdstE;     /* where to stop aligned	      */
    Pixel8	    *pdstEnd;   /* where to stop unaligned	      */
    int		    width;	/* width of glyph in bits	      */
    int		    height;	/* height of glyph in bits	      */
    int		    nwidth;     /* NGLYPHS * width		      */
    BitsWord	    glyphbits;  /* Actual bits of glyph row	      */
    BitsWord	    newbits, newbits1;
    CommandWord     ones = ffbStippleAll1;
    int		    align;
    CharInfoPtr     pci;
#ifdef FFBIMAGETEXT
    CommandWord		    leftMask, rightMask;
#endif

#if GLYPHWIDTH > 16
    BitsWord	    glyphMask;  /* Mask to zero right junk bits       */
    int		    widthGlyph;	/* width of glyph in bytes	      */
#endif

#if NGLYPHS == 1
    Bits8	    *pglyph0;
#elif NGLYPHS == 2
    Bits8	    *pglyph0, *pglyph1;
#elif NGLYPHS == 3
    Bits8	    *pglyph0, *pglyph1, *pglyph2;
#elif NGLYPHS == 4
    Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3;
#elif NGLYPHS == 5
    Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3, *pglyph4;
#elif NGLYPHS == 6	    
    Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3, *pglyph4, *pglyph5;
#elif NGLYPHS == 7
    Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3, *pglyph4, *pglyph5,
		    *pglyph6;
#elif NGLYPHS == 10
    Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3, *pglyph4, *pglyph5,
		    *pglyph6, *pglyph7, *pglyph8, *pglyph9;
#endif

    WRITE_MEMORY_BARRIER();
    pci = *ppci;
    height = pci->metrics.ascent + pci->metrics.descent;
    pdstEnd = pdstBase + height * widthDst;
    width = pci->metrics.characterWidth;
    nwidth = NGLYPHS * width;

#if GLYPHWIDTH > 16
    glyphMask = FFBRIGHTBUSMASK(width, FFBBUSALL1);
    widthGlyph = PADGLYPHWIDTHBYTES(width);
#endif

    nglyph -= NGLYPHS;
    while (nglyph >= 0) {
	/* Load up glyph pointers */
	pglyph0 = FFBGLYPHBITS(ppci[0]);

#if NGLYPHS >= 2
	pglyph1 = FFBGLYPHBITS(ppci[1]);
#endif
#if NGLYPHS >= 3
	pglyph2 = FFBGLYPHBITS(ppci[2]);
#endif
#if NGLYPHS >= 4
	pglyph3 = FFBGLYPHBITS(ppci[3]);
#endif
#if NGLYPHS >= 5
	pglyph4 = FFBGLYPHBITS(ppci[4]);
#endif
#if NGLYPHS >= 6
	pglyph5 = FFBGLYPHBITS(ppci[5]);
#endif
#if NGLYPHS >= 7
	pglyph6 = FFBGLYPHBITS(ppci[6]);
#endif
#if NGLYPHS >= 10
	pglyph7 = FFBGLYPHBITS(ppci[7]);
	pglyph8 = FFBGLYPHBITS(ppci[8]);
	pglyph9 = FFBGLYPHBITS(ppci[9]);
#endif
	ppci += NGLYPHS;

	/* Now paint bits from NGLYPHS on each scanline */
	align = (long)pdstBase & FFBSTIPPLEALIGNMASK;
	pdst = pdstBase - align;
	pdstE = pdstEnd - align;
	FFBBYTESTOPIXELS(align);

#ifdef FFBIMAGETEXT
	leftMask = FFBLEFTSTIPPLEMASK(align, ones);
	rightMask = FFBRIGHTSTIPPLEMASK(align + nwidth, ones);
	CYCLE_REGS(ffb);
	FFBPERSISTENTPIXELMASK(ffb, leftMask & rightMask);
#endif
	do {
#if NGLYPHS == 1
	    GETGLYPHBITS(pglyph0, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS  == 2
	    GETGLYPHBITS(pglyph1, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS == 3
	    GETGLYPHBITS(pglyph2, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS == 4
	    GETGLYPHBITS(pglyph3, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS == 5
	    GETGLYPHBITS(pglyph4, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS == 6
	    GETGLYPHBITS(pglyph5, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS == 7
	    GETGLYPHBITS(pglyph6, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS == 10
	    GETGLYPHBITS(pglyph9, glyphbits, glyphMask, widthGlyph);
#else
Oops, put something somewhere
#endif

#if NGLYPHS >= 10
	    GETGLYPHBITS(pglyph8, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph7, newbits1, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits1;
	    GETGLYPHBITS(pglyph6, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#endif
#if NGLYPHS >= 7
	    GETGLYPHBITS(pglyph5, newbits1, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits1;
#endif
#if NGLYPHS >= 6
	    GETGLYPHBITS(pglyph4, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#endif
#if NGLYPHS >= 5
	    GETGLYPHBITS(pglyph3, newbits1, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits1;
#endif
#if NGLYPHS >= 4
	    GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#endif
#if NGLYPHS >= 3
	    GETGLYPHBITS(pglyph1, newbits1, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits1;
#endif
#if NGLYPHS >= 2
	    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#endif
	    glyphbits <<= align;
	    FFBWRITEGLYPH(pdst, glyphbits);
	    pdst += widthDst;
	} while (pdst != pdstE);
	pdstBase += nwidth * FFBPIXELBYTES;
	pdstEnd += nwidth * FFBPIXELBYTES;
	nglyph -= NGLYPHS;
    } /* while (nglyph >= 0) */

#if NGLYPHS > 1
    nglyph += NGLYPHS;
    while (nglyph) {
	align = (long)pdstBase & FFBSTIPPLEALIGNMASK;
	pdst = pdstBase - align;
	pdstE = pdstEnd - align;
	FFBBYTESTOPIXELS(align);
#   ifdef FFBIMAGETEXT
	nwidth = nglyph * width;
	leftMask = FFBLEFTSTIPPLEMASK(align, ones);
	rightMask = FFBRIGHTSTIPPLEMASK(align + nwidth, ones);
	FFBPERSISTENTPIXELMASK(ffb, leftMask & rightMask);
#   endif

	switch (nglyph) {
	    case 1:
		/* Load up glyph pointers */
		pglyph0 = FFBGLYPHBITS(ppci[0]);
		do {
		    GETGLYPHBITS(pglyph0, glyphbits, glyphMask, widthGlyph);
		    glyphbits <<= align;
		    FFBWRITEGLYPH(pdst, glyphbits);
		    pdst += widthDst;
		} while (pdst != pdstE);
		nglyph -= 1;
		break;
    
#   if NGLYPHS > 2
	    case 2:
		/* Load up glyph pointers */
		pglyph0 = FFBGLYPHBITS(ppci[0]);
		pglyph1 = FFBGLYPHBITS(ppci[1]);	
		do {
		    GETGLYPHBITS(pglyph1, glyphbits, glyphMask, widthGlyph);
		    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		    glyphbits <<= align;
		    FFBWRITEGLYPH(pdst, glyphbits);
		    pdst += widthDst;
		} while (pdst != pdstE);
		nglyph -= 2;
		break;
#   endif
#   if NGLYPHS > 3
	    case 3:
		/* Load up glyph pointers */
		pglyph0 = FFBGLYPHBITS(ppci[0]);
		pglyph1 = FFBGLYPHBITS(ppci[1]);
		pglyph2 = FFBGLYPHBITS(ppci[2]);	
		do {
		    GETGLYPHBITS(pglyph2, glyphbits, glyphMask, widthGlyph);
		    GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		    GETGLYPHBITS(pglyph0, newbits1, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits1;
		    glyphbits <<= align;
		    FFBWRITEGLYPH(pdst, glyphbits);
		    pdst += widthDst;
		} while (pdst != pdstE);
		nglyph -= 3;
		break;
#   endif
#   if NGLYPHS > 4
	    case 4:
		/* Load up glyph pointers */
		pglyph0 = FFBGLYPHBITS(ppci[0]);
		pglyph1 = FFBGLYPHBITS(ppci[1]);
		pglyph2 = FFBGLYPHBITS(ppci[2]);
		pglyph3 = FFBGLYPHBITS(ppci[3]);	
		do {
		    GETGLYPHBITS(pglyph3, glyphbits, glyphMask, widthGlyph);
		    GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		    GETGLYPHBITS(pglyph1, newbits1, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits1;
		    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		    glyphbits <<= align;
		    FFBWRITEGLYPH(pdst, glyphbits);
		    pdst += widthDst;
		} while (pdst != pdstE);
		nglyph -= 4;
		break;
#   endif
#   if NGLYPHS > 5
	    default:
		/* Load up glyph pointers */
		pglyph0 = FFBGLYPHBITS(ppci[0]);
		pglyph1 = FFBGLYPHBITS(ppci[1]);
		pglyph2 = FFBGLYPHBITS(ppci[2]);
		pglyph3 = FFBGLYPHBITS(ppci[3]);	
		pglyph4 = FFBGLYPHBITS(ppci[4]);
		do {
		    GETGLYPHBITS(pglyph4, glyphbits, glyphMask, widthGlyph);
		    GETGLYPHBITS(pglyph3, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		    GETGLYPHBITS(pglyph2, newbits1, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits1;
		    GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		    GETGLYPHBITS(pglyph0, newbits1, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits1;
		    glyphbits <<= align;
		    FFBWRITEGLYPH(pdst, glyphbits);
		    pdst += widthDst;
		} while (pdst != pdstE);
#       ifndef IMAGETEXT
		nwidth = 5 * width;
#       endif
		pdstBase += nwidth * FFBPIXELBYTES;
		pdstEnd += nwidth * FFBPIXELBYTES;
		ppci += 5;
		nglyph -= 5;
		break;
#   endif
	} /* switch */
    } /* while nglyphs */
#endif /* NGLYPHS > 1 */
#ifdef FFBIMAGETEXT
    FFBPIXELMASK(ffb, ones);
#endif
}

/*
 * HISTORY
 */

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbteglyph.c,v 1.1.2.2 1993/11/19 21:17:58 Robert_Lembree Exp $ */

