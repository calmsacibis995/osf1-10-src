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
static char *rcsid = "@(#)$RCSfile: vgaspans.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/22 17:34:54 $";
#endif

#include "X.h"
#include "misc.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"

#ifdef DWDOS386
#include "alloca.h"
#endif /* DWDOS386 */

#if defined (DWDOS286)
typedef unsigned char huge * BITMAP_UNIT_PTR;
#else
typedef unsigned char * BITMAP_UNIT_PTR;
#endif

void
vgaSolidPixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr       pGC ;
    int         nInit ;                 /* number of spans to fill */
    DDXPointPtr pptInit ;               /* pointer to list of start points */
    int         *pwidthInit ;           /* pointer to list of n widths */
    int         fSorted ;
{
                                /* next three parameters are post-clip */
    int n;                      /* number of spans to fill */
    DDXPointPtr ppt;            /* pointer to list of start points */
    int *pwidth;                /* pointer to list of n widths */

    int *pwidthFree;            /* copies of the pointers to free */
    DDXPointPtr pptFree;

    BITMAP_UNIT_PTR pdstBase, pdst;
    long widthDst;
    int width;

    unsigned char fg        = (unsigned char) pGC->fgPixel;
    unsigned char alu       = pGC->alu;
    unsigned char pm        = (unsigned char) pGC->planemask;
    unsigned char npm       = ~pm;
    unsigned char allplanes = (unsigned char)((1 << pGC->depth) - 1);
    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree) {
      if (pptFree) DEALLOCATE_LOCAL(pptFree);
      if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
      return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
                    pptInit, pwidthInit, nInit,
                    ppt, pwidth, fSorted);
    
    pdstBase = (BITMAP_UNIT_PTR)(((PixmapPtr)pDrawable)->devPrivate.ptr);
    widthDst = (long)((PixmapPtr)pDrawable)->devKind;

    if (alu == GXcopy && (pm & allplanes) == allplanes) {
      while (n-- > 0) {
        pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
        width = *pwidth;
        while (width-- > 0) {
          *pdst++ = fg;
        }
        pwidth++;
        ppt++;
      }
    }
    else if (alu == GXxor && (pm & allplanes) == allplanes) {
      while (n-- > 0) {
        pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
        width = *pwidth;
        while (width-- > 0) {
          *pdst++ ^= fg;
        }
        pwidth++;
        ppt++;
      }
    }
    else if (alu == GXinvert) {
      while (n-- > 0) {
        pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
        width = *pwidth;
        while (width-- > 0) {
          *pdst++ ^= pm;
        }
        pwidth++;
        ppt++;
      }
    }
    else {
      while (n-- > 0) {
        pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
        width = *pwidth;
        while (width-- > 0) {
          unsigned char result;
          DoRop(result, alu, fg, *pdst);
          *pdst++ = (*pdst & npm) | (result & pm);
        }
        pwidth++;
        ppt++;
      }
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void
vgaStipplePixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr       pGC ;
    int         nInit ;                 /* number of spans to fill */
    DDXPointPtr pptInit ;               /* pointer to list of start points */
    int         *pwidthInit ;           /* pointer to list of n widths */
    int         fSorted ;
{
                                /* next three parameters are post-clip */
    int n;                      /* number of spans to fill */
    DDXPointPtr ppt;            /* pointer to list of start points */
    int *pwidth;                /* pointer to list of n widths */

    int *pwidthFree;            /* copies of the pointers to free */
    DDXPointPtr pptFree;

    PixmapPtr pStipple;
    BITMAP_UNIT_PTR pdstBase, pdst;
    long widthDst;
    unsigned char *psrcBase, *psrc, *psrcT;
    unsigned char src, mask;
    int widthSrc;
    int stippleWidth, stippleHeight, stipOrgX, stipOrgY;
    int xOffSrc, widthRem, width;

    unsigned char fg        = (unsigned char) pGC->fgPixel;
    unsigned char bg        = (unsigned char) pGC->bgPixel;
    unsigned char alu       = pGC->alu;
    unsigned char pm        = (unsigned char) pGC->planemask;
    unsigned char npm       = ~pm;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree) {
      if (pptFree) DEALLOCATE_LOCAL(pptFree);
      if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
      return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
                    pptInit, pwidthInit, nInit,
                    ppt, pwidth, fSorted);
    
    pdstBase = (BITMAP_UNIT_PTR)(((PixmapPtr)pDrawable)->devPrivate.ptr);
    widthDst = (long)((PixmapPtr)pDrawable)->devKind;

    pStipple = pGC->stipple;
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;
    psrcBase = pStipple->devPrivate.ptr;
    widthSrc = pStipple->devKind;
    stipOrgX = (pGC->patOrg.x % stippleWidth) - stippleWidth;
    stipOrgY = (pGC->patOrg.y % stippleHeight) - stippleHeight;

    if (pGC->fillStyle == FillStippled) {
      while (n-- > 0) {
        width = *pwidth;
        pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
        psrc = psrcBase + (((ppt->y - stipOrgY) % stippleHeight) * widthSrc);
        xOffSrc = (ppt->x - stipOrgX) % stippleWidth;
        widthRem = stippleWidth - xOffSrc;
        psrcT = psrc + (xOffSrc >> 3);
        src = *psrcT;
        mask = (unsigned char)(0x01 << (xOffSrc & 0x7));
        while (width-- > 0) {
          if (src & mask) {
            unsigned char result;
            DoRop(result, alu, fg, *pdst);
            *pdst = (*pdst & npm) | (result & pm);
          }
	  pdst++;
          if (--widthRem==0) {
	    /* Backup source pointer to beginning of stipple scanline */
            widthRem = stippleWidth;
            psrcT = psrc;
            src = *psrcT;
            mask = 0x01;
          }
          else if ((mask<<=1)==0) {
	    /* Fetch the next stipple source byte */
            psrcT++;
            src = *psrcT;
            mask = 0x01;
          }
        }
        pwidth++;
        ppt++;
      }
    }
    else /* FillOpaqueStippled */ {
      while (n-- > 0) {
        width = *pwidth;
        pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
        psrc = psrcBase + (((ppt->y - stipOrgY) % stippleHeight) * widthSrc);
        xOffSrc = (ppt->x - stipOrgX) % stippleWidth;
        widthRem = stippleWidth - xOffSrc;
        psrcT = psrc + (xOffSrc >> 3);
        src = *psrcT;
        mask = (unsigned char)(0x01 << (xOffSrc & 0x7));
        while (width-- > 0) {
          if (src & mask) {
            unsigned char result;
            DoRop(result, alu, fg, *pdst);
            *pdst++ = (*pdst & npm) | (result & pm);
          }
          else {
            unsigned char result;
            DoRop(result, alu, bg, *pdst);
            *pdst++ = (*pdst & npm) | (result & pm);
          }
          if (--widthRem==0) {
            widthRem = stippleWidth;
            psrcT = psrc;
            src = *psrcT;
            mask = 0x01;
          }
          else if ((mask<<=1)==0) {
            psrcT++;
            src = *psrcT;
            mask = 0x01;
          }
        }
        pwidth++;
        ppt++;
      }
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void
vgaTilePixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr       pGC ;
    int         nInit ;                 /* number of spans to fill */
    DDXPointPtr pptInit ;               /* pointer to list of start points */
    int         *pwidthInit ;           /* pointer to list of n widths */
    int         fSorted ;
{
                                /* next three parameters are post-clip */
    int n;                      /* number of spans to fill */
    DDXPointPtr ppt;            /* pointer to list of start points */
    int *pwidth;                /* pointer to list of n widths */

    int *pwidthFree;            /* copies of the pointers to free */
    DDXPointPtr pptFree;

    PixmapPtr pTile;
    BITMAP_UNIT_PTR pdstBase, pdst;
    long widthDst;
    unsigned char *psrcBase, *psrc, *psrcT;
    int widthSrc;
    int tileWidth, tileHeight, tileOrgX, tileOrgY;
    int xOffSrc, widthRem, width;

    unsigned char alu       = pGC->alu;
    unsigned char pm        = (unsigned char) pGC->planemask;
    unsigned char npm       = ~pm;
    unsigned char allplanes = (unsigned char)((1 << pGC->depth) - 1);

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree) {
      if (pptFree) DEALLOCATE_LOCAL(pptFree);
      if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
      return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
                    pptInit, pwidthInit, nInit,
                    ppt, pwidth, fSorted);
    
    pdstBase = (BITMAP_UNIT_PTR)(((PixmapPtr)pDrawable)->devPrivate.ptr);
    widthDst = (long)((PixmapPtr)pDrawable)->devKind;

    pTile = pGC->tile.pixmap;
    tileWidth = pTile->drawable.width;
    tileHeight = pTile->drawable.height;
    psrcBase = pTile->devPrivate.ptr;
    widthSrc = pTile->devKind;
    tileOrgX = (pGC->patOrg.x % tileWidth) - tileWidth;
    tileOrgY = (pGC->patOrg.y % tileHeight) - tileHeight;

    if (alu == GXcopy && (pm & allplanes) == allplanes) {
      while (n-- > 0) {
	width = *pwidth;
	pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
	psrc = psrcBase + (((ppt->y - tileOrgY) % tileHeight) * widthSrc);
	xOffSrc = (ppt->x - tileOrgX) % tileWidth;
	widthRem = tileWidth - xOffSrc;
	psrcT = psrc + xOffSrc;
	while (width-- > 0) {
	  *pdst++ = *psrcT++;
	  if (--widthRem==0) {
	    /* Backup source pointer to beginning of tile scanline */
	    widthRem = tileWidth;
	    psrcT = psrc;
	  }
	}
	pwidth++;
	ppt++;
      }
    }
    else {
      while (n-- > 0) {
	width = *pwidth;
	pdst = pdstBase + ((ppt->y * widthDst) + ppt->x);
	psrc = psrcBase + (((ppt->y - tileOrgY) % tileHeight) * widthSrc);
	xOffSrc = (ppt->x - tileOrgX) % tileWidth;
	widthRem = tileWidth - xOffSrc;
	psrcT = psrc + xOffSrc;
	while (width-- > 0) {
	  unsigned char result;
	  DoRop(result, alu, *psrcT++, *pdst);
	  *pdst++ = (*pdst & npm) | (result & pm);
	  if (--widthRem==0) {
	    /* Backup source pointer to beginning of tile scanline */
	    widthRem = tileWidth;
	    psrcT = psrc;
	  }
	}
	pwidth++;
	ppt++;
      }
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void 
vgaSolidWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr       pGC ;
    int         nInit ;                 /* number of spans to fill */
    DDXPointPtr pptInit ;               /* pointer to list of start points */
    int         *pwidthInit ;           /* pointer to list of n widths */
    int         fSorted ;
{
                                /* next three parameters are post-clip */
    int n;                      /* number of spans to fill */
    DDXPointPtr ppt;            /* pointer to list of start points */
    int *pwidth;                /* pointer to list of n widths */

    int *pwidthFree;            /* copies of the pointers to free */
    DDXPointPtr pptFree;
    int first;

    FillSolidFuncPtr FillSolidFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->FillSolidFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInSpansFuncPtr HideCursorInSpansFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInSpansFunc;
#endif

    unsigned int fg        = (unsigned int) pGC->fgPixel;
    unsigned int alu       = pGC->alu;
    unsigned int pm        = (unsigned int) pGC->planemask;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree) {
      if (pptFree) DEALLOCATE_LOCAL(pptFree);
      if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
      return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
                    pptInit, pwidthInit, nInit,
                    ppt, pwidth, fSorted);
#ifdef SOFTWARE_CURSOR    
    (*HideCursorInSpansFunc)(ppt, pwidth, n);
#endif
    first = TRUE;
    while (n-- > 0) {
      if (*pwidth) {
        (*FillSolidFunc)(pGC->pScreen, fg, alu, pm, ppt->x, ppt->y, 
			 *pwidth, 1, first);
	first = FALSE;
      }
      pwidth++;
      ppt++;
    }
#ifdef SOFTWARE_CURSOR
    (*ShowCursorFunc)();
#endif
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

void
vgaStippleWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr       pGC ;
    int         nInit ;                 /* number of spans to fill */
    DDXPointPtr pptInit ;               /* pointer to list of start points */
    int         *pwidthInit ;           /* pointer to list of n widths */
    int         fSorted ;
{
                                /* next three parameters are post-clip */
    int n;                      /* number of spans to fill */
    DDXPointPtr ppt;            /* pointer to list of start points */
    int *pwidth;                /* pointer to list of n widths */

    int *pwidthFree;            /* copies of the pointers to free */
    DDXPointPtr pptFree;

    PixmapPtr pStipple;
    unsigned char *psrcBase;
    int widthSrc;
    int stippleWidth, stippleHeight, stipOrgX, stipOrgY;
    int xOffSrc, yOffSrc, widthRem, width;
    int first;

    OpaqueStippleFuncPtr OpaqueStippleFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInSpansFuncPtr HideCursorInSpansFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInSpansFunc;
#endif

    unsigned int fg        = (unsigned int) pGC->fgPixel;
    unsigned int bg        = (unsigned int) pGC->bgPixel;
    unsigned int alu       = pGC->alu;
    unsigned int pm        = (unsigned int) pGC->planemask;

    if (pGC->fillStyle == FillStippled) {
      OpaqueStippleFunc = (OpaqueStippleFuncPtr)
        (((DrawFuncs *)(pGC->pScreen->devPrivate))->StippleFunc);
    }
    else {
      OpaqueStippleFunc =
        ((DrawFuncs *)(pGC->pScreen->devPrivate))->OpaqueStippleFunc;
    }

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree) {
      if (pptFree) DEALLOCATE_LOCAL(pptFree);
      if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
      return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
                    pptInit, pwidthInit, nInit,
                    ppt, pwidth, fSorted);
    
    pStipple = pGC->stipple;
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;
    psrcBase = pStipple->devPrivate.ptr;
    widthSrc = pStipple->devKind;

    stipOrgX = pDrawable->x + (pGC->patOrg.x % stippleWidth) - stippleWidth;
    stipOrgY = pDrawable->y + (pGC->patOrg.y % stippleHeight) - stippleHeight;
#ifdef SOFTWARE_CURSOR
    (*HideCursorInSpansFunc)(ppt, pwidth, n);
#endif
    first = TRUE;
    while (n-- > 0) {
      int x = ppt->x;
      int y = ppt->y;
      width = *pwidth;
      xOffSrc = (x - stipOrgX) % stippleWidth;
      yOffSrc = (y - stipOrgY) % stippleHeight;
      widthRem = stippleWidth - xOffSrc;
      if (width <= widthRem) {
        (*OpaqueStippleFunc)(pGC->pScreen, xOffSrc, yOffSrc, width, 1, 
			     psrcBase, widthSrc, x, y, first, alu, pm, fg, bg);
        first = FALSE;
      }
      else {
        (*OpaqueStippleFunc)(pGC->pScreen, xOffSrc, yOffSrc, widthRem, 1, 
			     psrcBase, widthSrc, x, y, first, alu, pm, fg, bg);
        first = FALSE;
	x += widthRem;
        width -= widthRem;
        do {
          (*OpaqueStippleFunc)(pGC->pScreen, 0, yOffSrc, 
			       min(width, stippleWidth), 1, psrcBase, widthSrc,
                               x, y, first, alu, pm, fg, bg);
          first = FALSE;
	  x += stippleWidth;
          width -= stippleWidth;
        } while (width > 0);
      }
      pwidth++;
      ppt++;
    }
#ifdef SOFTWARE_CURSOR
    (*ShowCursorFunc)();
#endif
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


void
vgaTileWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr       pGC ;
    int         nInit ;                 /* number of spans to fill */
    DDXPointPtr pptInit ;               /* pointer to list of start points */
    int         *pwidthInit ;           /* pointer to list of n widths */
    int         fSorted ;
{
                                /* next three parameters are post-clip */
    int n;                      /* number of spans to fill */
    DDXPointPtr ppt;            /* pointer to list of start points */
    int *pwidth;                /* pointer to list of n widths */

    int *pwidthFree;            /* copies of the pointers to free */
    DDXPointPtr pptFree;

    PixmapPtr pTile;
    unsigned char *psrcBase;
    int widthSrc;
    int tileWidth, tileHeight, tileOrgX, tileOrgY;
    int xOffSrc, yOffSrc, widthRem, width;
    int first;

    DrawColorImageFuncPtr DrawColorImageFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->DrawColorImageFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInSpansFuncPtr HideCursorInSpansFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInSpansFunc;
#endif

    unsigned int alu       = pGC->alu;
    unsigned int pm        = (unsigned int) pGC->planemask;

    n = nInit * miFindMaxBand(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree) {
      if (pptFree) DEALLOCATE_LOCAL(pptFree);
      if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
      return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
                    pptInit, pwidthInit, nInit,
                    ppt, pwidth, fSorted);
    
    pTile = pGC->tile.pixmap;
    tileWidth = pTile->drawable.width;
    tileHeight = pTile->drawable.height;
    psrcBase = pTile->devPrivate.ptr;
    widthSrc = pTile->devKind;

    tileOrgX = pDrawable->x + (pGC->patOrg.x % tileWidth) - tileWidth;
    tileOrgY = pDrawable->y + (pGC->patOrg.y % tileHeight) - tileHeight;
#ifdef SOFTWARE_CURSOR
    (*HideCursorInSpansFunc)(ppt, pwidth, n);
#endif
    first = TRUE;
    while (n-- > 0) {
      int x = ppt->x;
      int y = ppt->y;
      width = *pwidth;
      xOffSrc = (x - tileOrgX) % tileWidth;
      yOffSrc = (y - tileOrgY) % tileHeight;
      widthRem = tileWidth - xOffSrc;
      if (width <= widthRem) {
        (*DrawColorImageFunc)(pGC->pScreen, x, y, width, 1,
                              psrcBase + (yOffSrc * widthSrc) + xOffSrc,
                              widthSrc, alu, pm, first);
        first = FALSE;
      }
      else {
        (*DrawColorImageFunc)(pGC->pScreen, x, y, widthRem, 1,
                              psrcBase + (yOffSrc * widthSrc) + xOffSrc,
                              widthSrc, alu, pm, first);
        first = FALSE;
	x += widthRem;
        width -= widthRem;
        do {
          (*DrawColorImageFunc)(pGC->pScreen, x, y, min(width, tileWidth), 1,
                                psrcBase + (yOffSrc * widthSrc),
                                widthSrc, alu, pm, first);
          first = FALSE;
	  x += tileWidth;
          width -= tileWidth;
        } while (width > 0);
      }
      pwidth++;
      ppt++;
    }
#ifdef SOFTWARE_CURSOR
    (*ShowCursorFunc)();
#endif
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}


/*
**  vgaGetSpans - Given a list of points and widths, copy the pixels into
**  a destination buffer.  The destination is always a pixmap but the
** source may be a pixmap or a window.
*/  
void vgaGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pDstStart)
	DrawablePtr   pDrawable;   /* drawable from which to get bits */
	int           wMax;        /* largest value of all *pwidths */
	DDXPointPtr   ppt;         /* points to start copying from */
	int           *pwidth;     /* list of number of bits to copy */
	int           nspans;      /* number of scanlines to copy */
	unsigned char *pDstStart;  /* where to put the bits */
	{
	BITMAP_UNIT_PTR pSrcBase, pSrc;
	long widthSrc;
	unsigned char *pDst = pDstStart;

	ReadColorImageFuncPtr ReadColorImageFunc =
	    ((DrawFuncs *)(pDrawable->pScreen->devPrivate))->ReadColorImageFunc;

#ifdef SOFTWARE_CURSOR
	ShowCursorFuncPtr ShowCursorFunc =
	    ((DrawFuncs *)(pDrawable->pScreen->devPrivate))->ShowCursorFunc;
  
	HideCursorInSpansFuncPtr HideCursorInSpansFunc =
	    ((DrawFuncs *)(pDrawable->pScreen->devPrivate))->HideCursorInSpansFunc;
#endif

	if (pDrawable->type == DRAWABLE_PIXMAP)
		{
		pSrcBase = (BITMAP_UNIT_PTR)
		    (((PixmapPtr)pDrawable)->devPrivate.ptr);
		widthSrc = (long)((PixmapPtr)pDrawable)->devKind;

		while (nspans-- > 0)
			{
			int width = *pwidth;
			pSrc = pSrcBase + ((ppt->y * widthSrc) + ppt->x);
			while (width-- > 0)
				{
				*pDst++ = *pSrc++;
				}
			pwidth++;
			ppt++;
			}
		}
	else
		{
		/* Source is a window */
#ifdef SOFTWARE_CURSOR
		(*HideCursorInSpansFunc)(ppt, pwidth, nspans);
#endif
		while (nspans-- > 0)
			{
			(*ReadColorImageFunc) (pDrawable->pScreen, ppt->x, 
				    ppt->y, *pwidth, 1, 0xffffffff, pDst, 0);

			pDst += *pwidth;
			pwidth++;
			ppt++;
			}
#ifdef SOFTWARE_CURSOR
		(*ShowCursorFunc)();
#endif
		}

	return;
	}

/*
  vgaSetSpans -
  The only routines that use SetSpans are miCopyArea, miOpqStipDrawable,
  miGetImage, miPutImage
*/  
void vgaSetSpans(pDrawable, pGC, psrc, pptInit, pwidthInit, nInit, fSorted)
	DrawablePtr   pDrawable;
	GCPtr         pGC;
	unsigned char *psrc;
	DDXPointPtr   pptInit;
	int           *pwidthInit;
	int           nInit;
	int           fSorted;
	{
	BITMAP_UNIT_PTR pDstBase, pDst;
	long widthDst;
	unsigned char *pSrc = psrc;

                              /* next three parameters are post-clip */
	int n;                      /* number of spans to fill */
	DDXPointPtr ppt;            /* pointer to list of start points */
	int *pwidth;                /* pointer to list of n widths */
	DDXPointPtr pptFree;
	int *pwidthFree;            /* copies of the pointers to free */
	int first;

	DrawColorImageFuncPtr DrawColorImageFunc =
	    ((DrawFuncs *)(pDrawable->pScreen->devPrivate))->DrawColorImageFunc;

#ifdef SOFTWARE_CURSOR
	ShowCursorFuncPtr ShowCursorFunc =
	    ((DrawFuncs *)(pDrawable->pScreen->devPrivate))->ShowCursorFunc;
  
	HideCursorInSpansFuncPtr HideCursorInSpansFunc =
	    ((DrawFuncs *)(pDrawable->pScreen->devPrivate))->HideCursorInSpansFunc;
#endif

	unsigned char alu       = pGC->alu;
	unsigned long pm        = pGC->planemask;

	n = nInit * miFindMaxBand(((mfbPrivGC *)
	    (pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
	pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
	pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree)
		{
		if (pptFree)
			DEALLOCATE_LOCAL(pptFree);
		if (pwidthFree)
			DEALLOCATE_LOCAL(pwidthFree);
		return;
		}

	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(((mfbPrivGC *)
	    (pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip,
            pptInit, pwidthInit, nInit, ppt, pwidth, fSorted);
    
	if (pDrawable->type == DRAWABLE_PIXMAP)
		{
		/* Destination is a pixmap */
		pDstBase = (BITMAP_UNIT_PTR)
		    (((PixmapPtr)pDrawable)->devPrivate.ptr);
		widthDst = (long)((PixmapPtr)pDrawable)->devKind;

		while (n-- > 0)
			{
			int width = *pwidth;
			pDst = pDstBase + ((ppt->y * widthDst) + ppt->x);
			while (width-- > 0)
				{
				unsigned char result;
				DoRop(result, alu, *pSrc++, *pDst);
				*pDst++ = (*pDst & ~pm) | (result & pm);
				}
			pwidth++;
			ppt++;
			}
		}
	else
		{
		/* Destination is a window */
#ifdef SOFTWARE_CURSOR
		(*HideCursorInSpansFunc)(ppt, pwidth, n);
#endif
		first = TRUE;
		while (n-- > 0)
			{
			(*DrawColorImageFunc) (pDrawable->pScreen, ppt->x, 
				ppt->y, *pwidth, 1, pSrc, 0, alu, pm, first);

			first = FALSE;
			pSrc += *pwidth;
			pwidth++;
			ppt++;
			}
#ifdef SOFTWARE_CURSOR
		(*ShowCursorFunc)();
#endif
		}

	DEALLOCATE_LOCAL(pptFree);
	DEALLOCATE_LOCAL(pwidthFree);
	return;
	}

