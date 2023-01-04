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
static char *rcsid = "@(#)$RCSfile: vgawind.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:34:59 $";
#endif

#include "X.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "regionstr.h"
#include "vga.h"
#include "vgaprocs.h"
  
#ifdef COMMENTS_ABOUT_WINDOW_FUNCTIONS

  The MFB window private structure, mfbPrivWin, is only used to
support special case painting of the window border and background
tiles.  In the special cases the fast{Border,Background} flags are set
TRUE and the pRotated{Border,Background} pointers are set to the
rotated border or background tiles.  The oldRotate field is updated
when the window postion changes and is used to calculate the amount to
re-rotate the the rotated tiles.

  The VGA paint window routines do not have any special cases.  It
does not cost significantly more to draw a tile with non-zero x and y
offsets than it does to draw the tile starting at its origin.  Only
height-of-the-tile scanlines need to be drawn before the
ReplicateScanlines helper can be used to finish the job.  Whatever
benefit using rotated tiles could have would be offset by the cost of
maintaining the rotated tiles every time the window moves and the
space of the window private structure (16 bytes) allocated for every
window.  AllocateWindowPrivate should not be called in the ScreenInit
because the window private is never used.

#endif /*  COMMENTS_ABOUT_WINDOW_FUNCTIONS */


/*
  The MFB version initializes the fast{Border,Background} flags to
  FALSE and the pRotated{Border,Background} pointers to NULL.
*/
Bool
vgaCreateWindow(pWin)
     WindowPtr pWin;
{
  return TRUE;
}


/*
  The MFB version frees the pRotated{Border,Background} tiles if they
  exist.
*/
Bool
vgaDestroyWindow(pWin)
     WindowPtr pWin;
{
  return TRUE;
}


/*
  The MFB version re-rotates the pRotated{Border,Background} tiles if
  they exist and updates oldRotate.
*/
Bool
vgaPostionWindow(pWin, x, y)
     WindowPtr pWin;
     int       x;
     int       y;
{
  return TRUE;
}


/*
  The MFB version checks for special PaintWindow cases when the
  changes mask includes any of {CWBackPixmap, CWBackPixel,
  CWBorderPixmap, CWBorderPixel}.
*/
Bool
vgaChangeWindowAttributes(pWin, mask)
     WindowPtr      pWin;
     unsigned long  mask;
{
  return TRUE;
}


/*
  The MFB versions of MapWindow (Realize) and UnmapWindow (Unrealize)
  do not do anything either.
*/
Bool
vgaMapWindow(pWin)
     WindowPtr pWin;
{
  return TRUE;
}

Bool
vgaUnmapWindow(pWin)
     WindowPtr pWin;
{
  return TRUE;
}


void 
vgaCopyWindow(pWin, ptOldOrg, prgnSrc)
     WindowPtr pWin;
     DDXPointRec ptOldOrg;
     RegionPtr prgnSrc;
{
  RegionPtr prgnDst;
  int dx, dy;
  BoxPtr pbox;
  int nbox;
  BoxPtr pboxBase, pboxNext, pboxTmp;
  int xSrc, ySrc, wSrc, hSrc, xDst, yDst;
  unsigned int allplanes;
  
  BlitFuncPtr BlitFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->BlitFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->ShowCursorFunc;
  
  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

  allplanes = (1 << pWin->drawable.depth) - 1;

  prgnDst = (* pWin->drawable.pScreen->RegionCreate)(NULL, 1);
  
  /*
    Dx and Dy are what you add to the old origin
    to get to the new origin
    */
  dx = pWin->drawable.x - ptOldOrg.x;
  dy = pWin->drawable.y - ptOldOrg.y;
  
  /*
    Translate the source region so its origin matches the origin of
    the clip region at the window's new position
    */
  (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, dx, dy);
  
  /*
    Find the portions of the source region that are visible at
    the new screen position
    */
  (* pWin->drawable.pScreen->Intersect)(prgnDst, &pWin->borderClip, prgnSrc);
  
  pbox = REGION_RECTS(prgnDst);
  nbox = REGION_NUM_RECTS(prgnDst);
  
  if ((nbox == 1) || ((dx <= 0) && (dy <= 0))) {
    /* Do bands top to bottom, left to right */
    while (nbox-- > 0) {
      xSrc = pbox->x1 - dx;
      ySrc = pbox->y1 - dy;
      wSrc = pbox->x2 - pbox->x1;
      hSrc = pbox->y2 - pbox->y1;
      xDst = pbox->x1;
      yDst = pbox->y1;
#ifdef SOFTWARE_CURSOR
      (*HideCursorInXYWHFunc)(xSrc, ySrc, wSrc, hSrc);
      (*HideCursorInXYWHFunc)(xDst, yDst, wSrc, hSrc);
#endif
      (*BlitFunc)(pWin->drawable.pScreen, xSrc, ySrc, wSrc, hSrc, xDst, yDst, 
		  GXcopy, allplanes);
      pbox++;
    }
  }
  else if ((dx > 0) && (dy > 0)) {
    /* Do bands bottom to top, right to left */
    pbox += (nbox-1);
    while (nbox-- > 0) {
      xSrc = pbox->x1 - dx;
      ySrc = pbox->y1 - dy;
      wSrc = pbox->x2 - pbox->x1;
      hSrc = pbox->y2 - pbox->y1;
      xDst = pbox->x1;
      yDst = pbox->y1;
#ifdef SOFTWARE_CURSOR
      (*HideCursorInXYWHFunc)(xSrc, ySrc, wSrc, hSrc);
      (*HideCursorInXYWHFunc)(xDst, yDst, wSrc, hSrc);
#endif
      (*BlitFunc)(pWin->drawable.pScreen, xSrc, ySrc, wSrc, hSrc, xDst, yDst, 
		  GXcopy, allplanes);
      pbox--;
    }
  }
  else if (dx > 0) {
    /* Do bands top to botton, right to left */
    pboxBase = pboxNext = pbox;
    while (pboxBase < pbox+nbox) {
      while ((pboxNext < pbox+nbox) &&
             (pboxNext->y1 == pboxBase->y1))
        pboxNext++;
      pboxTmp = pboxNext;
      while (pboxTmp != pboxBase) {
        pboxTmp--;
        xSrc = pboxTmp->x1 - dx;
        ySrc = pboxTmp->y1 - dy;
        wSrc = pboxTmp->x2 - pboxTmp->x1;
        hSrc = pboxTmp->y2 - pboxTmp->y1;
        xDst = pboxTmp->x1;
        yDst = pboxTmp->y1;
#ifdef SOFTWARE_CURSOR
        (*HideCursorInXYWHFunc)(xSrc, ySrc, wSrc, hSrc);
        (*HideCursorInXYWHFunc)(xDst, yDst, wSrc, hSrc);
#endif
        (*BlitFunc)(pWin->drawable.pScreen, xSrc, ySrc, wSrc, hSrc, xDst, yDst, 		    GXcopy, allplanes);
      }
      pboxBase = pboxNext;
    }
  }
  else {
    /* Do bands botton to top, left to right */
    pboxBase = pboxNext = pbox+nbox-1;
    while (pboxBase >= pbox) {
      while ((pboxNext >= pbox) &&
             (pboxBase->y1 == pboxNext->y1))
        pboxNext--;
      pboxTmp = pboxNext+1;
      while (pboxTmp <= pboxBase) {
        xSrc = pboxTmp->x1 - dx;
        ySrc = pboxTmp->y1 - dy;
        wSrc = pboxTmp->x2 - pboxTmp->x1;
        hSrc = pboxTmp->y2 - pboxTmp->y1;
        xDst = pboxTmp->x1;
        yDst = pboxTmp->y1;
#ifdef SOFTWARE_CURSOR
        (*HideCursorInXYWHFunc)(xSrc, ySrc, wSrc, hSrc);
        (*HideCursorInXYWHFunc)(xDst, yDst, wSrc, hSrc);
#endif
        (*BlitFunc)(pWin->drawable.pScreen, xSrc, ySrc, wSrc, hSrc, xDst, yDst, 		    GXcopy, allplanes);
        pboxTmp++;
      }
      pboxBase = pboxNext;
    }
  }
#ifdef SOFTWARE_CURSOR  
  (*ShowCursorFunc)();
#endif  
  (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}
