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
static char *rcsid = "@(#)$RCSfile: vgagblt.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/22 17:34:40 $";
#endif

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"font.h"
#include	"scrnintstr.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"pixmapstr.h"
#include	"mfb.h"
#include	"vga.h"
#include	"vgaprocs.h"

#ifdef DWDOS386
#include "alloca.h"
#endif /* DWDOS386 */


void
vgaPolyGlyphBlt( pDrawable, pGC, x, y, nglyph, ppci, pglyphBase )
     DrawablePtr pDrawable ;
     GC 		*pGC ;
     int 	x, y ;
     unsigned int nglyph ;
     CharInfoPtr *ppci ;		/* array of character info */
     unsigned char *pglyphBase ;	/* start of array of glyphs */
{
  unsigned char  *pglyph ;
  ExtentInfoRec  info ;	
  BoxRec         bbox ;	

  CharInfoPtr    pci ;
  int            w, h ;
  int		 first;

  StippleFuncPtr StippleFunc
    = ((DrawFuncs *)(pGC->pScreen->devPrivate))->StippleFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

  unsigned long fg = pGC->fgPixel;
  unsigned int alu = pGC->alu;
  unsigned long pm = pGC->planemask;

  x += pDrawable->x ;
  y += pDrawable->y ;

  QueryGlyphExtents(pGC->font, ppci, nglyph, &info) ;
  bbox.x1 = x + info.overallLeft ;
  bbox.x2 = x + info.overallRight ;
  bbox.y1 = y - info.overallAscent ;
  bbox.y2 = y + info.overallDescent ;

#ifdef SOFTWARE_CURSOR
  (*HideCursorInXYWHFunc)( bbox.x1, bbox.y1,
			  info.overallRight - info.overallLeft,
			  info.overallAscent + info.overallDescent ) ;
#endif
    
  switch ((*pGC->pScreen->RectIn)(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip, &bbox)) {
  case rgnOUT:
    break;
  case rgnIN:
    first = TRUE;
    while (nglyph-- ) {
      pci = *ppci++ ;
      pglyph = FONTGLYPHBITS(pglyphBase,pci) ;
      if ( ( w = GLYPHWIDTHPIXELS(pci) )
	  && ( h = GLYPHHEIGHTPIXELS(pci) ) ) {
/*	vgaDrawGlyph(pglyph,
		     x + pci->metrics.leftSideBearing,
		     y - pci->metrics.ascent,
		     w, h, fg);
*/
	(*StippleFunc)(pGC->pScreen, 0, 0, w, h, pglyph, 
		       GLYPHWIDTHBYTESPADDED(pci), 
		       x + pci->metrics.leftSideBearing,
		       y - pci->metrics.ascent,
		       first, alu, pm, fg);
        first = FALSE;
      }
      x += pci->metrics.characterWidth ;	/* update character origin */
    } /* while nglyph-- */
    break;
  case rgnPART:
    {
      TEXTPOS *ppos;
      RegionPtr cclip;
      int nbox;
      BoxPtr pbox;
      int xpos;		/* x position of char origin */
      int i;
      BoxRec clip;
      int leftEdge, rightEdge;
      int topEdge, bottomEdge;
      int glyphRow;		/* first row of glyph not wholly
				   clipped out */
      int glyphCol;		/* leftmost visible column of glyph */

      if(!(ppos = (TEXTPOS *)ALLOCATE_LOCAL(nglyph * sizeof(TEXTPOS))))
	return;

      xpos = x;

      for (i=0; i<nglyph; i++) {
	pci = ppci[i];
	  
	ppos[i].xpos = xpos;
	ppos[i].leftEdge = xpos + pci->metrics.leftSideBearing;
	ppos[i].rightEdge = xpos + pci->metrics.rightSideBearing;
	ppos[i].topEdge = y - pci->metrics.ascent;
	ppos[i].bottomEdge = y + pci->metrics.descent;
	ppos[i].widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	xpos += pci->metrics.characterWidth;
      }

      cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
      pbox = REGION_RECTS(cclip);
      nbox = REGION_NUM_RECTS(cclip);
      first = TRUE;

      for (; --nbox >= 0; pbox++) {
	clip.x1 = max(bbox.x1, pbox->x1);
	clip.y1 = max(bbox.y1, pbox->y1);
	clip.x2 = min(bbox.x2, pbox->x2);
	clip.y2 = min(bbox.y2, pbox->y2);
	if ((clip.x2<=clip.x1) || (clip.y2<=clip.y1))
	  continue;

	for(i=0; i<nglyph; i++) {
	  pci = ppci[i];

	  /* clip the left and right edges */
	  if (ppos[i].leftEdge < clip.x1)
	    leftEdge = clip.x1;
	  else
	    leftEdge = ppos[i].leftEdge;

	  if (ppos[i].rightEdge > clip.x2)
	    rightEdge = clip.x2;
	  else
	    rightEdge = ppos[i].rightEdge;

	  w = rightEdge - leftEdge;
	  if (w <= 0)
	    continue;

	  /* clip the top and bottom edges */
	  if (ppos[i].topEdge < clip.y1)
	    topEdge = clip.y1;
	  else
	    topEdge = ppos[i].topEdge;
	      
	  if (ppos[i].bottomEdge > clip.y2)
	    bottomEdge = clip.y2;
	  else
	    bottomEdge = ppos[i].bottomEdge;
	      
	  h = bottomEdge - topEdge;
	  if (h <= 0)
	    continue;

	  glyphRow = (topEdge - y) + pci->metrics.ascent;
	  glyphCol = (leftEdge - ppos[i].xpos) -
	    (pci->metrics.leftSideBearing);
	  
          pglyph = FONTGLYPHBITS(pglyphBase,pci) ;

	  (*StippleFunc)(pGC->pScreen, glyphCol, glyphRow, w, h,
			 pglyph, ppos[i].widthGlyph,
			 leftEdge, topEdge,
			 first, alu, pm, fg);
	  first = FALSE;
	} /* for each glyph */
      } /* while nbox-- */
      DEALLOCATE_LOCAL(ppos);
      break;
    }
  default:
    break;
  }

#ifdef SOFTWARE_CURSOR
  (*ShowCursorFunc)();
#endif

}


void
vgaImageGlyphBlt( pDrawable, pGC, x, y, nglyph, ppci, pglyphBase )
     DrawablePtr pDrawable ;
     GC 		*pGC ;
     int 	x, y ;
     unsigned int nglyph ;
     CharInfoPtr *ppci ;		/* array of character info */
     unsigned char *pglyphBase ;	/* start of array of glyphs */
{
  RegionPtr      cclip;
  BoxPtr         pbox;
  int            nbox;
  CharInfoPtr    pci ;
  ExtentInfoRec  info ;	
  BoxRec         bbox ;
  BoxRec         backbox ;
  int            w, h ;
  int            ax, ay, zx, zy, zw, zh; /* as in A to Z */
  unsigned char  *pglyph ;
  int		 first;

  FillSolidFuncPtr FillSolidFunc
    = ((DrawFuncs *)(pGC->pScreen->devPrivate))->FillSolidFunc;

  StippleFuncPtr StippleFunc
    = ((DrawFuncs *)(pGC->pScreen->devPrivate))->StippleFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

  unsigned long fg = pGC->fgPixel;
  unsigned long bg = pGC->bgPixel;
  unsigned long pm = pGC->planemask;

  cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))
    ->pCompositeClip;
  if ((nbox = REGION_NUM_RECTS(cclip))==0)
    return ;

  x += pDrawable->x ;
  y += pDrawable->y ;

  /* Find string's bounding box */
  QueryGlyphExtents(pGC->font, ppci, nglyph, &info) ;
  bbox.x1 = x + info.overallLeft ;
  bbox.x2 = x + info.overallRight ;
  bbox.y1 = y - info.overallAscent ;
  bbox.y2 = y + info.overallDescent ;

  /* Find background paint rectangle, in general not same as bounding box */
  backbox.x1 = x;
  backbox.y1 = y - info.fontAscent;
  backbox.x2 = x + info.overallWidth;
  backbox.y2 = y + info.fontDescent;

  /* Is the glyph string being drawn right-to-left or bottom-to-top? */
  if (backbox.x1 > backbox.x2) {
    ax = backbox.x1;
    backbox.x1 = backbox.x2;
    backbox.x2 = ax;
  }
  if (backbox.y1 > backbox.y2) {
    ay = backbox.y1;
    backbox.y1 = backbox.y2;
    backbox.y2 = ay;
  }

#ifdef SOFTWARE_CURSOR
  /* Hide cursor in max extent of bounding box and background rectangle */
  ax = min(bbox.x1, backbox.x1);
  ay = min(bbox.y1, backbox.y1);
  zx = max(bbox.x2, backbox.x2);
  zy = max(bbox.y2, backbox.y2);

  (*HideCursorInXYWHFunc)( ax, ay, zx-ax, zy-ay );
#endif

  first = TRUE;

  for ( pbox = REGION_RECTS(cclip) ; nbox-- ; pbox++ ) {
    ax = max( pbox->x1, backbox.x1 );
    ay = max( pbox->y1, backbox.y1 );
    zx = min( pbox->x2, backbox.x2 );
    zy = min( pbox->y2, backbox.y2 );
    if ( ( (zw = zx - ax) > 0) && ( (zh = zy - ay) > 0 ) ) {
      (*FillSolidFunc)(pGC->pScreen, bg, GXcopy, pm, ax, ay, zw, zh, first );
      first = FALSE;
    }
  }

  first = TRUE;

  switch ((*pGC->pScreen->RectIn)(cclip, &bbox)) {
  case rgnOUT:
    break;
  case rgnIN:
    while (nglyph-- ) {
      pci = *ppci++ ;
      pglyph = FONTGLYPHBITS(pglyphBase,pci) ;
/*      pglyph = (unsigned char *)pci->bits;*/
      if ( ( w = GLYPHWIDTHPIXELS(pci) )
	  && ( h = GLYPHHEIGHTPIXELS(pci) ) ) {
/*	vgaDrawGlyph(pglyph,
		     x + pci->metrics.leftSideBearing,
		     y - pci->metrics.ascent,
		     w, h, fg);
*/
	(*StippleFunc)(pGC->pScreen, 0, 0, w, h, pglyph, 
		       GLYPHWIDTHBYTESPADDED(pci),
		       x + pci->metrics.leftSideBearing,
		       y - pci->metrics.ascent,
		       first, GXcopy, pm, fg);
        first = FALSE;
      }	
      x += pci->metrics.characterWidth ;	/* update character origin */
    } /* while nglyph-- */
    break;
  case rgnPART:
    {
      TEXTPOS *ppos;
      int xpos;		/* x position of char origin */
      int i;
      BoxRec clip;
      int leftEdge, rightEdge;
      int topEdge, bottomEdge;
      int glyphRow;		/* first row of glyph not wholly
				   clipped out */
      int glyphCol;		/* leftmost visible column of glyph */

      if(!(ppos = (TEXTPOS *)ALLOCATE_LOCAL(nglyph * sizeof(TEXTPOS))))
	return;

      xpos = x;

      for (i=0; i<nglyph; i++) {
	pci = ppci[i];
	  
	ppos[i].xpos = xpos;
	ppos[i].leftEdge = xpos + pci->metrics.leftSideBearing;
	ppos[i].rightEdge = xpos + pci->metrics.rightSideBearing;
	ppos[i].topEdge = y - pci->metrics.ascent;
	ppos[i].bottomEdge = y + pci->metrics.descent;
	ppos[i].widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	xpos += pci->metrics.characterWidth;
      }

      pbox = REGION_RECTS(cclip);
      nbox = REGION_NUM_RECTS(cclip);

      for (; --nbox >= 0; pbox++) {
	clip.x1 = max(bbox.x1, pbox->x1);
	clip.y1 = max(bbox.y1, pbox->y1);
	clip.x2 = min(bbox.x2, pbox->x2);
	clip.y2 = min(bbox.y2, pbox->y2);
	if ((clip.x2<=clip.x1) || (clip.y2<=clip.y1))
	  continue;

	for(i=0; i<nglyph; i++) {
	  pci = ppci[i];

	  /* clip the left and right edges */
	  if (ppos[i].leftEdge < clip.x1)
	    leftEdge = clip.x1;
	  else
	    leftEdge = ppos[i].leftEdge;

	  if (ppos[i].rightEdge > clip.x2)
	    rightEdge = clip.x2;
	  else
	    rightEdge = ppos[i].rightEdge;

	  w = rightEdge - leftEdge;
	  if (w <= 0)
	    continue;

	  /* clip the top and bottom edges */
	  if (ppos[i].topEdge < clip.y1)
	    topEdge = clip.y1;
	  else
	    topEdge = ppos[i].topEdge;
	      
	  if (ppos[i].bottomEdge > clip.y2)
	    bottomEdge = clip.y2;
	  else
	    bottomEdge = ppos[i].bottomEdge;
	      
	  h = bottomEdge - topEdge;
	  if (h <= 0)
	    continue;

	  glyphRow = (topEdge - y) + pci->metrics.ascent;
	  glyphCol = (leftEdge - ppos[i].xpos) -
	    (pci->metrics.leftSideBearing);
	  
          pglyph = FONTGLYPHBITS(pglyphBase,pci) ;

	  (*StippleFunc)(pGC->pScreen, glyphCol, glyphRow, w, h,
			 pglyph, ppos[i].widthGlyph,
			 leftEdge, topEdge,
			 first, GXcopy, pm, fg);
          first = FALSE;

	} /* for each glyph */
      } /* while nbox-- */
      DEALLOCATE_LOCAL(ppos);
      break;
    }
  default:
    break;
  }

#ifdef SOFTWARE_CURSOR
  (*ShowCursorFunc)();
#endif

}

