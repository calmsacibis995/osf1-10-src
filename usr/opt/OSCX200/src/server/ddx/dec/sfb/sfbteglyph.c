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
#define IDENT "X-3"
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
** X-3		TLB0004		Tracy Bragdon		11-Dec-1991
**		Conditionally compile reference to Ulw (unaligned longword)
**		Revisit later when we know what to do for Alpha/VMS
**
** X-2		TLB0002		Tracy Bragdon		15-Nov-1991
**		Changes for R5 font structures
**
**--
**/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbteglyph.c,v 1.1.3.10 93/01/22 16:02:48 Jim_Ludwig Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"

#ifdef MITR5
#include "dixfontstr.h"
#else
#include "fontstr.h"
#endif

#include "sfb.h"
#include "sfbteglyph.h"
#include "sfbdivtable.h"

/*

Paint unclipped fixed-metric glyphs onto the screen.  Separated out
from sfbpglyblt.c so it could be put into assembly language.  Yummmmm.

The philosophy here is to try to paint a bunch of glyphs at a time, so that we
always have SFBBUSBITS of glyph information, and thus minimize writes out to the
TURBOChannel.  At the beginning of the string, we have to deal with the left
alignment, which restricts the number of glyphs we can use on the scanline.
For the rest of the string, we have to deal with whatever residue is left over
from the previous glyph.  In both cases, we have to deal with the possibility
that there are fewer glyphs left to paint than we'd like.

We also try to keep several glyph pointers in registers.  The number of
glyphs we are guaranteed to load each time through the loop depends upon the
maximum glyph width handled.

This file is compiled a few times, in order to pick up glyph infomation as
painlessly as possible:

SFBTESPLATGLYPHS	glyph widths handled
--------------------------------------------
sfbTESplatGlyphs8	     4-8
sfbTESplatGlyphs16	     9-16
sfbTESplatGlyphs32	    17-32

*/

/* NOTE: only handling GLYPHPADBYTES of 1,2, and 4, not 8 or 0 */
#if GLYPHWIDTH <= 8
#define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {		\
    newbits = *(pglyph);						\
    pglyph += GLYPHPADBYTES;						\
} /* GETGLYPHBITS */
#define GETLASTRESIDUEBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = *(pglyph);						\
    pglyph += widthGlyph;						\
} /* GETLASTRESIDUEBITS */

#elif GLYPHWIDTH <= 16

#if GLYPHPADBYTES < 4
#define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {		\
    newbits = *((unsigned short *) (pglyph));				\
    pglyph += 2;							\
} /* GETGLYPHBITS */
#else /* GLYPHPADBYTES == 4 */
#define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {		\
    newbits = *((unsigned short *) (pglyph));				\
    pglyph += GLYPHPADBYTES;						\
} /* GETGLYPHBITS */
#endif /* GLYPHPADBYTES */
#define GETLASTRESIDUEBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = *((unsigned short *) (pglyph));				\
    pglyph += widthGlyph;						\
} /* GETLASTRESIDUEBITS */

#elif GLYPHWIDTH <= 32

#if defined(MITR5) || defined(VMS)   
	/* fix later : this will cause unaligned fault on alpha */
#if GLYPHPADBYTES < 4
#define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {		\
    newbits = *((unsigned int *) (pglyph)) & (glyphMask); 		\
    pglyph += widthGlyph;						\
} /* GETGLYPHBITS */
#else /* GLYPHPADBYTES == 4 */
#define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {		\
    newbits = *((unsigned int *) (pglyph)) & (glyphMask); 		\
    pglyph += GLYPHPADBYTES;						\
} /* GETGLYPHBITS */
#endif /* GLYPHPADBYTES */
#define GETLASTRESIDUEBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = *((unsigned int *) (pglyph)) & (glyphMask); 		\
    pglyph += widthGlyph;						\
} /* GETLASTRESIDUEBITS */

#else	/*  not MITR5 or VMS */
#define GETGLYPHBITS(pglyph, newbits, glyphMask, widthGlyph) {		\
    newbits = Ulw(pglyph) & (glyphMask);				\
    pglyph += widthGlyph;						\
} /* GETGLYPHBITS */
#define GETLASTRESIDUEBITS(pglyph, newbits, glyphMask, widthGlyph) {	\
    newbits = Ulw(pglyph) & (glyphMask);				\
    pglyph += widthGlyph;						\
} /* GETLASTRESIDUEBITS */

#endif									

#endif

/* I hate C.  I especially hate C type declarations. */
typedef Bits8 *Bits8Ptr;

static Bits8 fakeResidueBits[4] = {
    0, 0, 0, 0
};

#ifdef SFBIMAGETEXT
void SFBTESPLATGLYPHS(pdstBase, widthDst, nglyph, ppci, sfb)
    register Pixel8	    *pdstBase;	/* first byte of current glyph in dst */
    register int	    widthDst;	/* width of dst in bytes	      */
    int			    nglyph;	/* Number of glyphs to paint	      */
    register CharInfoPtr    *ppci;	/* array of character info	      */
    register SFB	    sfb;
#else
void SFBTESPLATGLYPHS(pdstBase, widthDst, nglyph, ppci)
    register Pixel8	    *pdstBase;	/* first byte of current glyph in dst */
    register int	    widthDst;	/* width of dst in bytes	      */
    int			    nglyph;	/* Number of glyphs to paint	      */
    register CharInfoPtr    *ppci;	/* array of character info	      */
#endif


#ifdef FASTCPU

{
    register Pixel8	    *pdst;	/* pointer to current byte in dst     */
    register Pixel8	    *pdstEnd;   /* where to stop current glyph group  */
    register int	    width;	/* width of glyph in bits	      */
    int			    height;	/* height of glyph in bits	      */
    Bits8Ptr		    pglyphArray[SFBBUSBITS]; /* List of pglyph ptrs   */
    register Bits8Ptr       *ppglyphs;	/* Pointer into pglyphArray	      */
    register Bits8Ptr       *ppglyphsEnd, *ppglyphsStart;
    register Bits8	    *pGlyphResidue;
    int			    residueWidthGlyph;
    register CommandWord    glyphbits;  /* Actual bits of glyph row	      */
    register CommandWord    newbits;
    int			    residue, newresidue;
    int			    rightShift;
    register int	    n, tn, i, m;
    int			    threshhold;
    register int	    nwidth;     /* n * width			      */
    register CharInfoPtr    pci;
#ifdef SFBIMAGETEXT
    CommandWord		    leftMask, rightMask;
#endif

/* Whenever possible, we'll load glyph pointers into registers, rather than the
   more general (and slower) pglyphArray table. */
#if GLYPHWIDTH <= 8
    register Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3;
#elif GLYPHWIDTH <= 16
    register Bits8	    *pglyph0, *pglyph1;
#elif GLYPHWIDTH <= 32
    register CommandWord    glyphMask;  /* Mask to zero right junk bits       */
    register int	    widthGlyph;	/* width of glyph in bytes	      */
    register Bits8	    *pglyph0;
#endif

    pci = *ppci;
    height = pci->metrics.ascent + pci->metrics.descent;
    width = pci->metrics.characterWidth;

#if GLYPHWIDTH > 16
    glyphMask = SFBRIGHTBUSMASK(width, SFBBUSALL1);
    widthGlyph = PADGLYPHWIDTHBYTES(width);
#endif

/****************************************************************
Phase 1: Paint as many glyphs as we can, given that we also have to shift
         them left because alignment constraints.  Leave pdstBase aligned s.t.
   (1) pdstBase & SFBALIGNMASK == 0 (pdstBase is SFB-aligned)
   (2) 0..width-1 bits left to paint in each row of glyph
*****************************************************************/
 
    /* Align pdstBase, compute ending pointer */
    newresidue = (int)pdstBase & SFBALIGNMASK;
    pdstBase -= newresidue;
    pdstEnd = pdstBase + height * widthDst;
    SFBBYTESTOPIXELS(newresidue);
#ifdef SFBIMAGETEXT
    leftMask = SFBLEFTSTIPPLEMASK(newresidue, SFBSTIPPLEALL1);
#endif

    /* Compute tn and new residue using adds and subtracts to avoid slow 
       integer divide. */
    /* We know tn >= SFBBUSBITS/GLYPHWIDTH */
    tn = SFBBUSBITS/GLYPHWIDTH;
    nwidth = SFBBUSBITS/GLYPHWIDTH * width;
    residue = nwidth + newresidue - SFBBUSBITS;
    while (residue < 0) {
	tn++;
	nwidth += width;
	residue += width;
    }

    if (tn > nglyph) {
	/* Uh-oh.  Too few characters to paint full 32 bits of data. */
	/* We slime out by making this look like there's residue data. */
#ifdef SFBIMAGETEXT
	nwidth = newresidue + nglyph*width;
#endif
	residue = newresidue;
	residueWidthGlyph = 0;
	pGlyphResidue = fakeResidueBits;
	goto PAINTFIRSTANDLASTCHARS;
    }

    /* Fill pglyphArray array with pointers to glyph bitmaps. */
    ppglyphsEnd = pglyphArray + tn;
#if GLYPHWIDTH <= 8
    pglyph0 = SFBGLYPHBITS(ppci[0]);
    pglyph1 = SFBGLYPHBITS(ppci[1]);
    pglyph2 = SFBGLYPHBITS(ppci[2]);
    pglyph3 = SFBGLYPHBITS(ppci[3]);
    ppci += 4;
    ppglyphsStart = pglyphArray + 4;
#elif GLYPHWIDTH <= 16
    pglyph0 = SFBGLYPHBITS(ppci[0]);
    pglyph1 = SFBGLYPHBITS(ppci[1]);
    ppci += 2;
    ppglyphsStart = pglyphArray + 2;
#elif GLYPHWIDTH <= 32
    pglyph0 = SFBGLYPHBITS(ppci[0]);
    ppci += 1;
    ppglyphsStart = pglyphArray + 1;
#endif
    ppglyphs = ppglyphsStart;
    while (ppglyphs != ppglyphsEnd) {
	*ppglyphs = SFBGLYPHBITS(ppci[0]);
	ppglyphs++;
	ppci++;
    };

    /* Now for each scanline load up a word full of bits and paint it */
    pdst = pdstBase;
    do {
	/* Load up glyphbits with info from glyphs */
	glyphbits = 0;
	ppglyphs = ppglyphsEnd;
	while (ppglyphs != ppglyphsStart) {
	    ppglyphs--;
	    GETGLYPHBITS(*ppglyphs, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	}
#if GLYPHWIDTH <= 8
	GETGLYPHBITS(pglyph3, newbits, glyphMask, widthGlyph);
	glyphbits = (glyphbits << width) | newbits;
	GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
	glyphbits = (glyphbits << width) | newbits;
#endif
#if GLYPHWIDTH <= 16
	GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
	glyphbits = (glyphbits << width) | newbits;
#endif
	GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	glyphbits = (glyphbits << width) | newbits;

#ifdef SFBIMAGETEXT
	SFBPIXELMASK(sfb, leftMask);
	/* Note: only if STIPPLEBITS=BUSBITS does this next write work
	 * correctly. You need some cycling of addresses otherwise..
	 * TBDone
	 */
#endif
	glyphbits <<= newresidue;
	StippleEntireWord(pdst, glyphbits);
	pdst += widthDst;
    } while (pdst != pdstEnd);

    pdstBase += SFBBUSBITS*SFBPIXELBYTES;
    pdstEnd += SFBBUSBITS*SFBPIXELBYTES;
    nglyph -= tn;

    /****************************************************************
    Phase 2: Paint groups of glyphs so as to always paint SFBBUSBITS.
       Leave loop with conditions:
       (1) pdstBase & SFBALIGNMASK == 0    (pdstBase is SFB-aligned)
       (2) 0..width-1 bits left to paint in each row of last glyph painted
    *****************************************************************/

    if (nglyph != 0) {
	/* Set up all parameters for middle of string.  We have residue number
	   of bits left over from the current glyph, where residue is in the
	   range [0..width-1]. */
    
	n = divTable[width].ndiv;
	nwidth = divTable[width].nwidth;
	threshhold = SFBBUSBITS - nwidth;
    
	do {
	    pGlyphResidue = SFBGLYPHBITS(ppci[-1]);
	    tn = n + (residue < threshhold);
	    newresidue = residue + nwidth;
	    if (tn != n) newresidue += width;
	    rightShift = width - residue;
	    if (tn > nglyph) {
		/* Yuck.  This'll be the last iteration for sure. */
		goto PAINTLASTCHARS;
	    }

	    /* Fill pglyphArray array with pointers to glyph bitmaps. */
	    ppglyphsEnd = pglyphArray + tn;
#if GLYPHWIDTH <= 8
    	    pglyph0 = SFBGLYPHBITS(ppci[0]);
    	    pglyph1 = SFBGLYPHBITS(ppci[1]);
    	    pglyph2 = SFBGLYPHBITS(ppci[2]);
    	    pglyph3 = SFBGLYPHBITS(ppci[3]);
	    ppci += 4;
	    ppglyphsStart = pglyphArray + 4;
#elif GLYPHWIDTH <= 16
    	    pglyph0 = SFBGLYPHBITS(ppci[0]);
    	    pglyph1 = SFBGLYPHBITS(ppci[1]);
	    ppci += 2;
	    ppglyphsStart = pglyphArray + 2;
#elif GLYPHWIDTH <= 32
    	    pglyph0 = SFBGLYPHBITS(ppci[0]);
	    ppci += 1;
	    ppglyphsStart = pglyphArray + 1;
#endif
	    ppglyphs = ppglyphsStart;
	    while (ppglyphs != ppglyphsEnd) {
		*ppglyphs = SFBGLYPHBITS(ppci[0]);
		ppglyphs++;
		ppci++;
	    };
     
	    /* Now for each scanline load up a word full of bits and paint it */
	    pdst = pdstBase;
	    do {
		/* Load up glyphbits with info from glyphs */
		glyphbits = 0;
		ppglyphs = ppglyphsEnd;
		while (ppglyphs != ppglyphsStart) {
		    ppglyphs--;
		    GETGLYPHBITS(*ppglyphs, newbits, glyphMask, widthGlyph);
		    glyphbits = (glyphbits << width) | newbits;
		}
#if GLYPHWIDTH <= 8
		GETGLYPHBITS(pglyph3, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
		GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
#endif
#if GLYPHWIDTH <= 16
		GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
#endif
		GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
		GETGLYPHBITS(pGlyphResidue, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << residue) | (newbits >> rightShift);
		StippleEntireWord(pdst, glyphbits);
		pdst += widthDst;
	    } while (pdst != pdstEnd);
    
	    residue = newresidue & SFBBUSBITSMASK;
	    pdstBase += SFBBUSBITS*SFBPIXELBYTES;
	    pdstEnd += SFBBUSBITS*SFBPIXELBYTES;
	    nglyph -= tn;
	} while (nglyph != 0);
    } /* if nglyph != 0 */

    if (residue != 0) {
	/* Paint the last few bits of the last glyph in the string.  We know
	   that we're already aligned at the left end, so the right end is the
	   only place we need a mask in ImageText mode. */
	pGlyphResidue = SFBGLYPHBITS(ppci[-1]);
	rightShift = width - residue;
#ifdef SFBIMAGETEXT
	rightMask = SFBRIGHTSTIPPLEMASK(residue, SFBSTIPPLEALL1);
#endif
#if GLYPHWIDTH > SFBSTIPPLEBITS
	residue -= SFBSTIPPLEBITS;
	if (residue <= 0) {
#endif
	    do {
		GETGLYPHBITS(pGlyphResidue, glyphbits, glyphMask, widthGlyph);
#ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, rightMask);
#endif
		glyphbits >>= rightShift;
		SFBWRITE(pdstBase, glyphbits);
		pdstBase += widthDst;
	    } while (pdstBase != pdstEnd);
#if GLYPHWIDTH > SFBSTIPPLEBITS
	} else {
	    do {
		int m;

		GETGLYPHBITS(pGlyphResidue, glyphbits, glyphMask, widthGlyph);
		glyphbits >>= rightShift;
		pdst = pdstBase;
# ifdef SFBIMAGETEXT
		m = residue;
		do {
		    SFBWRITE(pdst, glyphbits);
		    pdst += SFBSTIPPLEBYTESDONE;
		    glyphbits >>= SFBSTIPPLEBITS;
		    m -= SFBSTIPPLEBITS;
		} while (m > 0);
 		pdst = CYCLE_FB(pdst);
		SFBPIXELMASK(sfb, rightMask);
		SFBWRITE(pdst, glyphbits);
# else
		while (glyphbits != 0) {
		    SFBWRITE(pdst, glyphbits);
		    pdst += SFBSTIPPLEBYTESDONE;
		    glyphbits >>= SFBSTIPPLEBITS;
		}
# endif
		pdstBase += widthDst;
	    } while (pdstBase != pdstEnd);
	}
#endif

    } /* if residue != 0 */
    return;


PAINTLASTCHARS:
    /* Have to paint last few characters in string. */
#ifdef SFBIMAGETEXT
    nwidth = residue + nglyph*width;
    leftMask = SFBSTIPPLEALL1;
#endif
#if GLYPHPADBYTES == 1
#  if GLYPHWIDTH <= 8
    residueWidthGlyph = 1;
#  elif GLYPHWIDTH <= 16
    residueWidthGlyph = 2;
#  elif GLYPHWIDTH <= 32
    residueWidthGlyph = widthGlyph;
#  endif
#elif GLYPHPADBYTES == 2
#  if GLYPHWIDTH <= 16
    residueWidthGlyph = 2;
#  elif GLYPHWIDTH <= 32
    residueWidthGlyph = 4;
#  endif
#elif GLYPHPADBYTES == 4
    residueWidthGlyph = 4;
#endif /* GLYPHPADBYTES */

    pGlyphResidue = SFBGLYPHBITS(ppci[-1]);


PAINTFIRSTANDLASTCHARS:
#ifdef SFBIMAGETEXT
    rightMask = SFBRIGHTSTIPPLEMASK(nwidth, SFBSTIPPLEALL1);
#endif
    rightShift = width - residue;
#if 2*GLYPHWIDTH <= SFBSTIPPLEBITS
    if (nglyph == 1) {
#else
    if (nglyph == 1 && nwidth <= SFBSTIPPLEBITS) {
#endif
	/* Special case for residue bits + one complete glyph, everything
	   fits into a single write. */
	/* Fill pglyphArray array with pointers to glyph bitmaps. */
	SFBFLSHREGS();
	pglyph0 = SFBGLYPHBITS(ppci[0]);

	/* Now paint the character */
	do {
	    GETGLYPHBITS(pglyph0, glyphbits, glyphMask, widthGlyph);
	    GETLASTRESIDUEBITS(pGlyphResidue, newbits, 
			    glyphMask, residueWidthGlyph);
	    glyphbits = (glyphbits << residue) | (newbits >> rightShift);
#ifdef SFBIMAGETEXT
	    SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
	    SFBWRITE(pdstBase, glyphbits);
	    pdstBase += widthDst;
	} while (pdstBase != pdstEnd);
    } else {
	/* Can't fit into one write, possibly more than one character. */
	/* Fill pglyphArray array with pointers to glyph bitmaps. */
	ppglyphsEnd = pglyphArray + nglyph;
	pglyph0 = SFBGLYPHBITS(ppci[0]);
	ppci += 1;
	ppglyphsStart = pglyphArray + 1;
	ppglyphs = ppglyphsStart;
	while (ppglyphs != ppglyphsEnd) {
	    *ppglyphs = SFBGLYPHBITS(ppci[0]);
	    ppglyphs++;
	    ppci++;
	};
	
	do {
	    /* Load up glyphbits with info from glyphs */
	    glyphbits = 0;
	    ppglyphs = ppglyphsEnd;
	    while (ppglyphs != ppglyphsStart) {
		ppglyphs--;
		GETGLYPHBITS(*ppglyphs, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
	    }
	    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETLASTRESIDUEBITS(pGlyphResidue, newbits, glyphMask, 
				residueWidthGlyph);
	    glyphbits = (glyphbits << residue) | (newbits >> rightShift);

#if SFBBUSBITS > SFBSTIPPLEBITS
	    if (nwidth <= SFBSTIPPLEBITS) {
#endif
#ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
		SFBWRITE(pdstBase, glyphbits);

#if SFBBUSBITS > SFBSTIPPLEBITS
	    } else {
		pdst = pdstBase;
# ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, leftMask);
		SFBWRITE(pdst, glyphbits);
 		pdst = CYCLE_FB(pdst);
		for (m = nwidth - 2*SFBSTIPPLEBITS; m > 0;
		     m -= SFBSTIPPLEBITS) {
		    glyphbits >>= SFBSTIPPLEBITS;
		    pdst += SFBSTIPPLEBYTESDONE;
		    SFBWRITE(pdst, glyphbits);
		}
 		pdst = CYCLE_FB(pdst);
		SFBPIXELMASK(sfb, rightMask);
		glyphbits >>= SFBSTIPPLEBITS;
		SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, glyphbits);
# else
		while (glyphbits != 0) {
		    SFBWRITE(pdst, glyphbits);
		    pdst += SFBSTIPPLEBYTESDONE;
		    glyphbits >>= SFBSTIPPLEBITS;
		}
# endif
	    }
#endif
	    pdstBase += widthDst;
	} while (pdstBase != pdstEnd);
    }
} /* SFBTESPLATGLYPHS */




#else

/*

Slower CPU, use entirely different strategy.  Always paint the same number of
characters, which is fixed at the maximum allowed under the worst alignment
contraints.  That is, NGLYPHS = (SFBBUSBITS-SFBALIGNMASK)/GLYPHWIDTH.

Then paint the last few characters in the string.

*/


{
    register Pixel8	    *pdst;	/* pointer to current byte in dst     */
    register Pixel8	    *pdstE;     /* where to stop aligned	      */
    register Pixel8	    *pdstEnd;   /* where to stop unaligned	      */
    register int	    width;	/* width of glyph in bits	      */
    int			    height;	/* height of glyph in bits	      */
    register int	    nwidth;     /* NGLYPHS * width		      */
    register CommandWord    glyphbits;  /* Actual bits of glyph row	      */
    register CommandWord    newbits;
    CommandWord		    ones = sfbStippleAll1;
    int			    align;
    register CharInfoPtr    pci;
#ifdef SFBIMAGETEXT
    CommandWord		    leftMask, rightMask;
#endif

#if NGLYPHS == 1
    register CommandWord    glyphMask;  /* Mask to zero right junk bits       */
    register int	    widthGlyph;	/* width of glyph in bytes	      */
    register Bits8	    *pglyph0;
#elif NGLYPHS == 2
    register Bits8	    *pglyph0, *pglyph1;
#elif NGLYPHS == 3
    register Bits8	    *pglyph0, *pglyph1, *pglyph2;
#elif NGLYPHS == 4
    register Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3;
#elif NGLYPHS == 5
    register Bits8	    *pglyph0, *pglyph1, *pglyph2, *pglyph3, *pglyph4;
#endif

    pci = *ppci;
    height = pci->metrics.ascent + pci->metrics.descent;
    pdstEnd = pdstBase + height * widthDst;
    width = pci->metrics.characterWidth;
    nwidth = NGLYPHS * width;

#if GLYPHWIDTH > 16
    glyphMask = SFBRIGHTBUSMASK(width, SFBBUSALL1);
    widthGlyph = PADGLYPHWIDTHBYTES(width);
#endif

    nglyph -= NGLYPHS;
    while (nglyph >= 0) {
	/* Load up glyph pointers */
	pglyph0 = SFBGLYPHBITS(ppci[0]);

#if NGLYPHS >= 2
	pglyph1 = SFBGLYPHBITS(ppci[1]);
#endif
#if NGLYPHS >= 3
	pglyph2 = SFBGLYPHBITS(ppci[2]);
#endif
#if NGLYPHS >= 4
	pglyph3 = SFBGLYPHBITS(ppci[3]);
#endif
#if NGLYPHS >= 5
	pglyph4 = SFBGLYPHBITS(ppci[4]);
#endif
	ppci += NGLYPHS;

	/* Now paint bits from NGLYPHS on each scanline */
	align = (int)pdstBase & SFBALIGNMASK;
	pdst = pdstBase - align;
	pdstE = pdstEnd - align;
	SFBBYTESTOPIXELS(align);

#ifdef SFBIMAGETEXT
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(align + nwidth, ones);
#endif

#if (SFBBUSBITS <= SFBSTIPPLEBITS)
	do {
#if NGLYPHS == 1
	    GETGLYPHBITS(pglyph0, glyphbits, glyphMask, widthGlyph);
#elif NGLYPHS  == 2
	    GETGLYPHBITS(pglyph1, glyphbits, glyphMask, widthGlyph);
	    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#elif NGLYPHS == 3
	    GETGLYPHBITS(pglyph2, glyphbits, glyphMask, widthGlyph);
	    GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#elif NGLYPHS == 4
	    GETGLYPHBITS(pglyph3, glyphbits, glyphMask, widthGlyph);
	    GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#elif NGLYPHS == 5
	    GETGLYPHBITS(pglyph4, glyphbits, glyphMask, widthGlyph);
	    GETGLYPHBITS(pglyph3, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
	    GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
	    glyphbits = (glyphbits << width) | newbits;
#endif
#ifdef SFBIMAGETEXT
	    SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
	    SFBWRITE(pdst, glyphbits << align);
	    pdst += widthDst;
	} while (pdst != pdstE);

#elif
need multi-word cases here
#endif
	pdstBase += nwidth * SFBPIXELBYTES;
	pdstEnd += nwidth * SFBPIXELBYTES;
	nglyph -= NGLYPHS;
    } /* while (nglyph >= 0) */

#if NGLYPHS > 1
    nglyph += NGLYPHS;
    align = (int)pdstBase & SFBALIGNMASK;
    pdst = pdstBase - align;
    pdstE = pdstEnd - align;
    SFBBYTESTOPIXELS(align);
#ifdef SFBIMAGETEXT
	nwidth = nglyph * width;
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(align + nwidth, ones);
#endif
#if SFBBUSBITS <= SFBSTIPPLEBITS
    switch (nglyph) {
	case 0:
	    /* We're done. */
	    break;

	case 1:
	    /* Load up glyph pointers */
	    pglyph0 = SFBGLYPHBITS(ppci[0]);
	    do {
		GETGLYPHBITS(pglyph0, glyphbits, glyphMask, widthGlyph);
#ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
		SFBWRITE(pdst, glyphbits << align);
		pdst += widthDst;
	    } while (pdst != pdstE);
	    break;

#if NGLYPHS > 2
	case 2:
	    /* Load up glyph pointers */
    	    pglyph0 = SFBGLYPHBITS(ppci[0]);
    	    pglyph1 = SFBGLYPHBITS(ppci[1]);
	    do {
		GETGLYPHBITS(pglyph1, glyphbits, glyphMask, widthGlyph);
		GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
#ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
		SFBWRITE(pdst, glyphbits << align);
		pdst += widthDst;
	    } while (pdst != pdstE);
	    break;
#endif
#if NGLYPHS > 3
	case 3:
	    /* Load up glyph pointers */
    	    pglyph0 = SFBGLYPHBITS(ppci[0]);
    	    pglyph1 = SFBGLYPHBITS(ppci[1]);
    	    pglyph2 = SFBGLYPHBITS(ppci[2]);
	    do {
		GETGLYPHBITS(pglyph2, glyphbits, glyphMask, widthGlyph);
		GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
		GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
#ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
		SFBWRITE(pdst, glyphbits << align);
		pdst += widthDst;
	    } while (pdst != pdstE);
	    break;
#endif
#if NGLYPHS > 4
	case 4:
	    /* Load up glyph pointers */
    	    pglyph0 = SFBGLYPHBITS(ppci[0]);
    	    pglyph1 = SFBGLYPHBITS(ppci[1]);
    	    pglyph2 = SFBGLYPHBITS(ppci[2]);
    	    pglyph3 = SFBGLYPHBITS(ppci[3]);
	    do {
		GETGLYPHBITS(pglyph3, glyphbits, glyphMask, widthGlyph);
		GETGLYPHBITS(pglyph2, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
		GETGLYPHBITS(pglyph1, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
		GETGLYPHBITS(pglyph0, newbits, glyphMask, widthGlyph);
		glyphbits = (glyphbits << width) | newbits;
#ifdef SFBIMAGETEXT
		SFBPIXELMASK(sfb, leftMask & rightMask);
#endif
		SFBWRITE(pdst, glyphbits << align);
		pdst += widthDst;
	    } while (pdst != pdstE);
	    break;
#endif
    } /* switch */
#endif
#endif /* NGLYPHS > 1 */
}
#endif /* FASTCPU ... #else ... */
