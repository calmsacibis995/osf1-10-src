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
/*******************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

************************************************************************/
/* $XConsortium: mipolytext.c,v 5.2 91/01/27 13:01:31 keith Exp $ */
/*
 * mipolytext.c - text routines
 *
 * Author:	haynes
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Thu Feb  5 1987
 */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"misc.h"
#include	"gcstruct.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"

int
miPolyText(pDraw, pGC, x, y, count, chars, fontEncoding)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char 	*chars;
    FontEncoding fontEncoding;
{
    register CharInfoPtr *charinfo;
    unsigned long n, i;
    int w;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr ))))
	return x ;
    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      fontEncoding, &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) w += charinfo[i]->metrics.characterWidth;
    if (n != 0)
        (*pGC->ops->PolyGlyphBlt)(
	    pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));

    DEALLOCATE_LOCAL(charinfo);
    return x+w;
}


int
miPolyText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    register CharInfoPtr *charinfo, *ci;
    unsigned long n, i;
    int w;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr ))))
	return x ;
    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    w = 0;
    if (n != 0) {
      if (TERMINALFONT(pGC->font)) {
	  w = n * charinfo[0]->metrics.characterWidth;
      } else {
	    ci = charinfo + n;
	    if (n & 1) {
		w = ci[-1]->metrics.characterWidth;
		ci--;
	    }
	    while (ci != charinfo)  {
		w += ci[-1]->metrics.characterWidth
		   + ci[-2]->metrics.characterWidth;
		ci -= 2;
	    }
      }
      (*pGC->ops->PolyGlyphBlt)(
	  pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));
    }

    DEALLOCATE_LOCAL(charinfo);
    return x+w;
}


int
miPolyText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    register CharInfoPtr *charinfo, *ci;
    unsigned long n, i;
    int w;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr ))))
	return x ;
    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      (FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
	      &n, charinfo);
    w = 0;
    if (n != 0) {
      if (TERMINALFONT(pGC->font)) {
          w = n * charinfo[0]->metrics.characterWidth;
      } else {
	    ci = charinfo + n;
	    if (n & 1) {
		w = ci[-1]->metrics.characterWidth;
		ci--;
	    }
	    while (ci != charinfo)  {
		w += ci[-1]->metrics.characterWidth
		   + ci[-2]->metrics.characterWidth;
		ci -= 2;
	    }
      }
      (*pGC->ops->PolyGlyphBlt)(
	  pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));
    }


    DEALLOCATE_LOCAL(charinfo);
    return x+w;
}


int
miImageText(pDraw, pGC, x, y, count, chars, fontEncoding)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int 	x, y;
    int 	count;
    char 	*chars;
    FontEncoding fontEncoding;
{
    register CharInfoPtr *charinfo;
    unsigned long n, i;
    FontPtr font = pGC->font;
    int w;

    if(!(charinfo = (CharInfoPtr *)ALLOCATE_LOCAL(count*sizeof(CharInfoPtr))))
	return x;
    GetGlyphs(font, (unsigned long)count, (unsigned char *)chars,
	      fontEncoding, &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) w += charinfo[i]->metrics.characterWidth;
    if (n !=0 )
        (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
    DEALLOCATE_LOCAL(charinfo);
    return x+w;
}


void
miImageText8(pDraw, pGC, x, y, count, chars)
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
    GetGlyphs(font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    if (n !=0 )
        (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
    DEALLOCATE_LOCAL(charinfo);
}


void
miImageText16(pDraw, pGC, x, y, count, chars)
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
    GetGlyphs(font, (unsigned long)count, (unsigned char *)chars,
	      (FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
	      &n, charinfo);
    if (n !=0 )
        (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
    DEALLOCATE_LOCAL(charinfo);
}
