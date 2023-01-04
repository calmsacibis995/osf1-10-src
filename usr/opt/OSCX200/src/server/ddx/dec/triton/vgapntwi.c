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
static char *rcsid = "@(#)$RCSfile: vgapntwi.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:34:48 $";
#endif

#include "X.h"

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "vga.h"
#include "vgaprocs.h"

/*
 *	Static function prototypes
 */

static void vgaPaintWindowSolid(WindowPtr pWin, RegionPtr pRegion, int what);
static void vgaPaintWindowTile(WindowPtr pWin, RegionPtr pRegion, int what);


/**********************************************************************
  The window background specified by pWin->backgroundState can be one of:
  {None, ParentRelative, BackgroundPixel, BackgroundPixmap}

  The window border specified by pWin->borderIsPixel indicates whether
  the border is pixel or a pixmap.
  
  If the background or border is a pixel, the pWin->background or
  pWin->border union contains the pixel value.  If the background or
  border is a pixmap, the pWin->background or pWin->border union
  contains the PixmapPtr.

  For a ParentRelative background, the background tile origin always
  aligns with the parent's background tile origin.  Otherwise, the
  background tile origin is always the window origin.  The border tile
  origin is always the same as the background tile origin.
**********************************************************************/


void
vgaPaintWindow(pWin, pRegion, what)
     WindowPtr  pWin;
     RegionPtr  pRegion;
     int        what;
{
  WindowPtr	pBgWin;
  
  switch (what) {
  case PW_BACKGROUND:
    switch (pWin->backgroundState) {
    case None:
      return;
    case ParentRelative:
      do {
        pWin = pWin->parent;
      } while (pWin->backgroundState == ParentRelative);
      (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, pRegion, what);
      return;
    case BackgroundPixmap:
      vgaPaintWindowTile(pWin, pRegion, what);
      return;
    case BackgroundPixel:
      vgaPaintWindowSolid(pWin, pRegion, what);
      return;
    }
    break;
  case PW_BORDER:
    for (pBgWin = pWin; 
	 pBgWin->backgroundState == ParentRelative;
	 pBgWin = pBgWin->parent);
    if (pBgWin->borderIsPixel) {
      vgaPaintWindowSolid(pBgWin, pRegion, what);
      return;
    }
    else {
      vgaPaintWindowTile(pBgWin, pRegion, what);
      return;
    }
  }
}


static void
vgaPaintWindowSolid(pWin, pRegion, what)
     WindowPtr  pWin;
     RegionPtr  pRegion;
     int        what;
{
  int            nBox;
  BoxPtr         pBox;
  int            xDst, yDst, wDst, hDst;
  unsigned int   pixel;
  unsigned int   allplanes;
  int		 first;
  
  FillSolidFuncPtr FillSolidFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->FillSolidFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->ShowCursorFunc;

  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

  allplanes = (1 << pWin->drawable.depth) - 1;

  if ((nBox = REGION_NUM_RECTS(pRegion))==0)
    return;
  pBox = REGION_RECTS(pRegion);

  if (what == PW_BACKGROUND)
    pixel = (unsigned int) pWin->background.pixel;
  else
    pixel = (unsigned int) pWin->border.pixel;

  first = TRUE;
  for (; nBox--; pBox++) {
    xDst = pBox->x1;
    yDst = pBox->y1;
    wDst = pBox->x2 - xDst;
    hDst = pBox->y2 - yDst;
    if (wDst>0 && hDst>0) {
#ifdef SOFTWARE_CURSOR
      (*HideCursorInXYWHFunc)(xDst, yDst, wDst, hDst);
#endif
      (*FillSolidFunc)(pWin->drawable.pScreen, pixel, GXcopy, allplanes, xDst, 
			yDst, wDst, hDst, first);
      first = FALSE;
#ifdef SOFTWARE_CURSOR
      (*ShowCursorFunc)();
#endif
    }
  } /* End of loop for each box */
}


static void
vgaPaintWindowTile(pWin, pRegion, what)
    WindowPtr   pWin;
    RegionPtr   pRegion;
    int         what;
{
  int            nBox;
  BoxPtr         pBox;
  PixmapPtr      pTile;
  int            tileWidth, tileHeight;
  IMAGE_PTR	psrcBase;
  long            widthSrc;
  int            tileOrgX, tileOrgY;
  unsigned int   allplanes;
  int		 first;

  DrawColorImageFuncPtr DrawColorImageFunc = 
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->DrawColorImageFunc;

  ReplicateScansFuncPtr ReplicateScansFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->ReplicateScansFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->ShowCursorFunc;

  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pWin->drawable.pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

  allplanes = (1 << pWin->drawable.depth) - 1;

  if ((nBox = REGION_NUM_RECTS(pRegion))==0)
    return;
  pBox = REGION_RECTS(pRegion);

  if (what == PW_BACKGROUND)
    pTile = pWin->background.pixmap;
  else
    pTile = pWin->border.pixmap;
  
  tileWidth = pTile->drawable.width;
  tileHeight = pTile->drawable.height;
  psrcBase = (IMAGE_PTR) pTile->devPrivate.ptr;
  widthSrc = (long) pTile->devKind;
  tileOrgX = (pWin->drawable.x % tileWidth) - tileWidth;
  tileOrgY = (pWin->drawable.y % tileHeight) - tileHeight;
  
  for (; nBox--; pBox++) {
    int heightRep;
    int doRep = 0;
    int y = pBox->y1;
    int widthSave = pBox->x2 - pBox->x1;
    int height = pBox->y2 - pBox->y1;
    int xOffSrcSave = (pBox->x1 - tileOrgX) % tileWidth;
    int yOffSrc = (pBox->y1 - tileOrgY) % tileHeight;
    int widthRemSave = tileWidth - xOffSrcSave;
    int heightRem = tileHeight - yOffSrc;
#ifdef SOFTWARE_CURSOR
    (*HideCursorInXYWHFunc)(pBox->x1, pBox->y1, widthSave, height);
#endif

    if ((height > tileHeight) && (doRep == 0)) {
      heightRep = height - tileHeight;
      height = tileHeight;
      doRep = 1;
    }
    first = TRUE;
    while (height > 0) {
      int x = pBox->x1;
      int width = widthSave;
      int xOffSrc = xOffSrcSave;
      int widthRem = widthRemSave;
      while (width > 0) {
        (*DrawColorImageFunc)(pWin->drawable.pScreen, x, y,
                              min(width, widthRem),
                              min(height, heightRem),
                              psrcBase + (yOffSrc * widthSrc) + xOffSrc,
                              widthSrc, GXcopy, allplanes, first);
        first = FALSE;
	x += widthRem;
        width -= widthRem;
        xOffSrc = 0;
        widthRem = tileWidth;
      }
      y += heightRem;
      height -= heightRem;
      yOffSrc = 0;
      heightRem = tileHeight;
    }
    if (doRep) {
      (*ReplicateScansFunc)(pWin->drawable.pScreen, pBox->x1, pBox->y1, 
			    widthSave, tileHeight, heightRep, allplanes);
    }
#ifdef SOFTWARE_CURSOR 
    (*ShowCursorFunc)();
#endif
  } /* End of loop for each box */
}
