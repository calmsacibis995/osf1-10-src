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
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "X.h"
#include "Xproto.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "qd.h"
#include "qdgc.h"
#include "servermd.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#define SFEncodedFontPtr        FontPtr
#define SFEncodedFontRec        FontRec
#define SFCharSetPtr            FontInfoPtr
#define SFCharSetRec            FontInfoRec


void
qdImageTextPix( pDraw, pGC, x0, y0, nChars, pStr)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x0, y0;
    int		nChars;
    char *	pStr;
{
    CHECK_MOVED(pGC, pDraw);
    if (QD_PIX_DATA((PixmapPtr)pDraw) == NULL) {
	/* make dummy window and use that as the drawable */
	SETUP_PIXMAP_AS_WINDOW(pDraw, pGC);
	/*
	 * tlImageText is somewhat faster than qdImageTextKerned,
	 * but in the interest of simplicity, we just go for the more
	 * general case, without testing to see if that is needed.
	 */
	qdImageTextKerned(pDraw, pGC, x0, y0, nChars, pStr);
	CLEANUP_PIXMAP_AS_WINDOW(pGC);
    }
    else
	miImageText8( pDraw, pGC, x0, y0, nChars, pStr);
}

int
qdPolyTextPix( pDraw, pGC, x0, y0, nChars, pStr)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x0, y0;
    int 	nChars;
    char *	pStr;
{
    CHECK_MOVED(pGC, pDraw);
    if (QD_PIX_DATA((PixmapPtr)pDraw) == NULL) {
	int width;
	/* make dummy window and use that as the drawable */
	SETUP_PIXMAP_AS_WINDOW(pDraw, pGC);
	width = tlPolyText( pDraw, pGC, x0, y0, nChars, pStr);
	CLEANUP_PIXMAP_AS_WINDOW(pGC);
	return width;
    }
    else
	return miPolyText8( pDraw, pGC, x0, y0, nChars, pStr);
}

qdImageTextKerned( pDraw, pGC, x0, y0, nChars, pStr)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x0, y0;
    int 	nChars;
    char *	pStr;
{
    xRectangle rect;
    register unsigned char *p;
    register int i, w;
    SFEncodedFontPtr pFont= pGC->font;
    int chfirst = FONTFIRSTCOL(pFont);
    int chlast = FONTLASTCOL(pFont);
    unsigned long n;
    int		ic;
#ifdef NEW_FONTS
    register CharInfoPtr *ppCI = pFont->ppCI - chfirst;
#else
    register CharInfoPtr *pci;
#endif /* NEW_FONTS */
    int saveAlu = pGC->alu;
    int saveFgPixel = pGC->fgPixel;

    if(!(pci = (CharInfoPtr *)ALLOCATE_LOCAL(nChars*sizeof(CharInfoPtr)))) {
	DEALLOCATE_LOCAL(chars);
	return(-1);
    }
/*
	Choose Linear8Bit encoding by clues in qdgc.c,
	fonts/lib/font/bitmap/bitmap.c
*/
    GetGlyphs(pFont, nChars, pStr, Linear8Bit, &n, pci);

    /* calculate width of string pStr */
    for (w = 0, i = nChars, p = (unsigned char*)pStr; --i >= 0; ) {
	register ch = *p++;
	if (ch < chfirst || ch > chlast) {
	    ch=	FONTDEFAULTCH(pFont);
	    if (ch < chfirst || ch > chlast) continue;
		/* SKK: else default characterWidth.... */
	}
#ifdef NEW_FONTS
	w += ppCI[ch]->metrics.characterWidth;
#else
	w += pci[i]->metrics.characterWidth;
#endif /* NEW_FONTS */
    }
    DEALLOCATE_LOCAL(pci);

    rect.x = x0;
    rect.y = y0 - FONTASCENT(pFont);
    rect.width = w;
    rect.height = FONTASCENT(pFont)+FONTDESCENT(pFont);
    pGC->alu = GXcopy;
    pGC->fgPixel = pGC->bgPixel;
    tlSolidRects(pDraw, pGC, 1, &rect);
    pGC->fgPixel = saveFgPixel;
    tlPolyTextSolid( pDraw, pGC, x0, y0, nChars, pStr);
    pGC->alu = saveAlu;
}

void
qdPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */

{
    int width, height;
    int nbyLine;			/* bytes per line of padded pixmap */
    SFEncodedFontRec 	*pfont;
    register int i;
    register int j;
    unsigned char *pbits;		/* buffer for PutImage */
    register unsigned char *pb;		/* temp pointer into buffer */
    register CharInfoPtr pci;		/* currect char info */
    register unsigned char *pglyph;	/* pointer bits in glyph */
    int gWidth, gHeight;		/* width and height of glyph */
    register int nbyGlyphWidth;		/* bytes per scanline of glyph */
    int nbyPadGlyph;			/* server padded line of glyph */
    QDPixRec dummyPixmap[1];

    if ((pDrawable->type == DRAWABLE_WINDOW) &&
	(pGC->miTranslate))
    {
	x += pGC->lastWinOrg.x;
	y += pGC->lastWinOrg.y;
    }

    pfont=	pGC->font;
    width = FONTMAXBOUNDS(pfont,rightSideBearing) - 
		FONTMINBOUNDS(pfont,leftSideBearing);
    height = FONTASCENT(pfont)+FONTDESCENT(pfont);

    nbyLine = PixmapBytePad(width, 1);
    pbits = (unsigned char *)ALLOCATE_LOCAL(height*nbyLine);
    if (!pbits)
        return ;

    dummyPixmap->pixmap.drawable.type = DRAWABLE_PIXMAP;
    dummyPixmap->pixmap.drawable.depth = 1;
    dummyPixmap->pixmap.drawable.pScreen = pDrawable->pScreen;
    QDPIX_X(dummyPixmap) = 0;
    QDPIX_Y(dummyPixmap) = 0;
    dummyPixmap->planes = 0;

    while(nglyph--) {
	pci = *ppci++;
	pglyph = FONTGLYPHBITS(0,pci);
	gWidth = GLYPHWIDTHPIXELS(pci);
	gHeight = GLYPHHEIGHTPIXELS(pci);
	nbyGlyphWidth = GLYPHWIDTHBYTESPADDED(pci);
	nbyPadGlyph = PixmapBytePad(gWidth, 1);

	for (i=0, pb = pbits; i<gHeight; i++, pb = pbits+(i*nbyPadGlyph))
	    for (j = 0; j < nbyGlyphWidth; j++)
		*pb++ = *pglyph++;

	QDPIX_WIDTH(&dummyPixmap->pixmap) = gWidth;
	QDPIX_HEIGHT(&dummyPixmap->pixmap) = gHeight;
	dummyPixmap->pixmap.devKind = PixmapBytePad(gWidth, 1);
	QD_PIX_DATA(&dummyPixmap->pixmap) = pbits;
	qdPushPixels(pGC, &dummyPixmap->pixmap, pDrawable,
		     gWidth, gHeight,
		     x + pci->metrics.leftSideBearing,
		     y - pci->metrics.ascent);
	/* hack to prevent CopyPixmapFromOffscreen */
	QD_PIX_DATA(&dummyPixmap->pixmap) = (unsigned char *)1;
	tlCancelPixmap(dummyPixmap);
	x += pci->metrics.characterWidth;
    }
    DEALLOCATE_LOCAL(pbits);
}
