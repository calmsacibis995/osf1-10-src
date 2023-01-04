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
static char *rcsid = "@(#)$RCSfile: ffbpolytext.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/21 19:51:32 $";
#endif
/*
 */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"misc.h"
#include	"gcstruct.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"bitmap.h"


int
ffbBitmapGetGlyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    BitmapFontPtr  bitmapFont;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr *encoding;
    CharInfoPtr pDefault;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    encoding = bitmapFont->encoding;
    pDefault = bitmapFont->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;
    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols && (pci = encoding[c]))
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols && (pci = encoding[c]))
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols &&
		    (pci = encoding[r * numCols + c]))
		*glyphs++ = pci;
	    else if (pDefault)
		*glyphs++ = pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}

int
ffbBitmapGetGlyphs8All(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    BitmapFontPtr  bitmapFont;
    CharInfoPtr     *encoding;
    CharInfoPtr     glyph0, glyph1;
    unsigned char   *last;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    encoding = bitmapFont->encoding;
    last = chars + count;
    
    if (count & 1) {
	*glyphs = encoding[*chars];
	glyphs++;
	chars++;
    }
    while (chars != last) {
	glyph0 = encoding[chars[0]];
	glyph1 = encoding[chars[1]];
	glyphs[0] = glyph0;
	glyphs[1] = glyph1;
	glyphs += 2;
	chars += 2;
    }
    *glyphCount = count;
    return Successful;
}

int
ffbBitmapGetGlyphs8(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    BitmapFontPtr   bitmapFont;
    CharInfoPtr     *encoding;
    unsigned char   *last;
    CharInfoPtr     glyph0, glyph1;
    CharInfoPtr     *glyphBase;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    encoding = bitmapFont->encoding;
    glyphBase = glyphs;
    last = chars + count;
    
    if (count & 1) {
	glyph0 = encoding[*chars];
	*glyphs = glyph0;
	glyphs += (glyph0 != NULL);
	chars++;
    }
    while (chars != last) {
	glyph0 = encoding[chars[0]];
	glyph1 = encoding[chars[1]];
	*glyphs = glyph0;
	glyphs += (glyph0 != NULL);
	*glyphs = glyph1;
	glyphs += (glyph1 != NULL);
	chars += 2;
    }
    *glyphCount = glyphs - glyphBase;
    return Successful;
}

/*ARGSUSED*/
Bool
ffbRealizeFont( pScreen, font)
    ScreenPtr	pScreen;
    FontPtr	font;
{
    BitmapFontPtr   bitmapFont;
    int		    i;
    CharInfoPtr     *encoding;
    CharInfoPtr     pDefault;

    font->get_glyphs = ffbBitmapGetGlyphs;

    bitmapFont = (BitmapFontPtr) font->fontPrivate;
    if (font->info.firstCol == 0 && font->info.lastCol == 255 &&
	font->info.lastRow == 0) {
	/* 8-bit font, encoding array has 256 pointers */
	if (font->info.allExist) {
	    /* All 256 characters have valid pointers in encoding array */
	    font->get_glyphs = ffbBitmapGetGlyphs8All;
	} else if (pDefault = bitmapFont->pDefault) {
	    /* We can make all 256 characters have valid pointers */
	    encoding = bitmapFont->encoding;
	    for (i = 0; i < 256; i++) {
		if (encoding[i] == NULL) {
		    encoding[i] = pDefault;
		}
	    }
	    font->info.allExist = TRUE;
	    font->get_glyphs = ffbBitmapGetGlyphs8All;
	} else {
	    /* Bummer, have to use slower GetGlyphs routine */
	    font->get_glyphs = ffbBitmapGetGlyphs8;
	}
    } /* if can use fast 8-bit glyph getter */
    return TRUE;
}


int ffbmiPolyGlyphBlt(pDrawable, pGC, x, y, n, ppci, pglyphBase)
    DrawablePtr     pDrawable;
    GCPtr	    pGC;
    int		    x, y;
    unsigned int    n;
    CharInfoPtr     *ppci;		/* array of character info	      */
    unsigned char   *pglyphBase;       	/* start of array of glyphs */
{
    register CharInfoPtr *ci;
    int w;

    if (n == 0) return 0;

    miPolyGlyphBlt(pDrawable, pGC, x, y, n, ppci, pglyphBase);
    /* Now compute overallWidth, and return that */
    if (TERMINALFONT(pGC->font)) {
	w = n * ppci[0]->metrics.characterWidth;
    } else {
	w = 0;
	ci = ppci + n;
	if (n & 1) {
	    w = ci[-1]->metrics.characterWidth;
	    ci--;
	}
	while (ci != ppci)  {
	    w += ci[-1]->metrics.characterWidth
	       + ci[-2]->metrics.characterWidth;
	    ci -= 2;
	}
    }
    return w;
}


typedef int (*IntProc)();

int
ffbPolyText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    FontPtr font = pGC->font;
    CharInfoPtr *charinfo;
    unsigned long n;
    int w;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr ))))
	return x ;

    (*font->get_glyphs)(font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    w = (*((IntProc)(pGC->ops->PolyGlyphBlt)))(
	  pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));

    DEALLOCATE_LOCAL(charinfo);
    return x+w;
}


int
ffbPolyText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    FontPtr font = pGC->font;
    CharInfoPtr *charinfo;
    unsigned long n;
    int w;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr ))))
	return x ;
    (*font->get_glyphs)
	    (font, (unsigned long)count, (unsigned char *)chars,
	    (FONTLASTROW(font) == 0) ? Linear16Bit : TwoD16Bit,
	    &n, charinfo);
    w = (*((IntProc)(pGC->ops->PolyGlyphBlt)))(
	  pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));

    DEALLOCATE_LOCAL(charinfo);
    return x+w;
}


void
ffbImageText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    CharInfoPtr *charinfo;
    unsigned long n;
    FontPtr font = pGC->font;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr))))
	return;
    (*font->get_glyphs)(font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
    DEALLOCATE_LOCAL(charinfo);
}


void
ffbImageText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    CharInfoPtr *charinfo;
    unsigned long n;
    FontPtr font = pGC->font;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr))))
	return;
    (*font->get_glyphs)(font, (unsigned long)count, (unsigned char *)chars,
	      (FONTLASTROW(font) == 0) ? Linear16Bit : TwoD16Bit,
	      &n, charinfo);
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
    DEALLOCATE_LOCAL(charinfo);
}

/*
 * HISTORY
 */

/*
 * HISTORY
 */
