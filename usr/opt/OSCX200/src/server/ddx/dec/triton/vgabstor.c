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
static char *rcsid = "@(#)$RCSfile: vgabstor.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/22 17:34:29 $";
#endif

#include    "mfb.h"
#include    "X.h"
#include    "mibstore.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"
#include    "vga.h"
#include    "vgaprocs.h"


#if defined (DWDOS286)
typedef unsigned char huge * BITMAP_UNIT_PTR;
#else
typedef unsigned char * BITMAP_UNIT_PTR;
#endif

/*-
 *-----------------------------------------------------------------------
 * vgaSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap.  The region to save is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the screen
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */
void
vgaSaveAreas(pPixmap, prgnSave, xorg, yorg)
     PixmapPtr	  pPixmap;  	/* Backing pixmap */
     RegionPtr	  prgnSave; 	/* Region to save (pixmap-relative) */
     int	  xorg;	    	/* X origin of region */
     int	  yorg;	    	/* Y origin of region */
{
  BoxPtr          pbox;
  int             nbox;
  BITMAP_UNIT_PTR pdstBase;
  long            widthDst;
    
  ReadColorImageFuncPtr ReadColorImageFunc =
    ((DrawFuncs *)(pPixmap->drawable.pScreen->devPrivate))->ReadColorImageFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pPixmap->drawable.pScreen->devPrivate))->ShowCursorFunc;
  
  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pPixmap->drawable.pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif
  
  pbox = REGION_RECTS(prgnSave);
  nbox = REGION_NUM_RECTS(prgnSave);

  pdstBase = (BITMAP_UNIT_PTR)(pPixmap->devPrivate.ptr);
  widthDst = (long)pPixmap->devKind;

  while (nbox-- > 0) {
    int xDst = pbox->x1;
    int xSrc = xDst + xorg;
    int wDst = pbox->x2 - xDst;
    int yDst = pbox->y1;
    int ySrc = yDst + yorg;
    int hDst = pbox->y2 - yDst;

#ifdef SOFTWARE_CURSOR
    (*HideCursorInXYWHFunc)(xSrc, ySrc, wDst, hDst);
#endif

    (*ReadColorImageFunc)(pPixmap->drawable.pScreen, xSrc, ySrc, wDst, hDst,
			  0xffffffff, pdstBase + (yDst * widthDst) + xDst,
			  (int)widthDst);

#ifdef SOFTWARE_CURSOR
    (*ShowCursorFunc)();
#endif

    pbox++;
  }
}

/*-
 *-----------------------------------------------------------------------
 * vgaRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. The region to restore is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the pixmap
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
vgaRestoreAreas(pPixmap, prgnRestore, xorg, yorg)
     PixmapPtr	  pPixmap;  	/* Backing pixmap */
     RegionPtr	  prgnRestore; 	/* Region to restore (screen-relative)*/
     int	  xorg;	    	/* X origin of window */
     int	  yorg;	    	/* Y origin of window */
{
  BoxPtr          pbox;
  int             nbox;
  BITMAP_UNIT_PTR psrcBase;
  long            widthSrc;
  int		  first;
    
  DrawColorImageFuncPtr DrawColorImageFunc =
    ((DrawFuncs *)(pPixmap->drawable.pScreen->devPrivate))->DrawColorImageFunc;

#ifdef SOFTWARE_CURSOR
  ShowCursorFuncPtr ShowCursorFunc =
    ((DrawFuncs *)(pPixmap->drawable.pScreen->devPrivate))->ShowCursorFunc;
  
  HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
    ((DrawFuncs *)(pPixmap->drawable.pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif
  
  pbox = REGION_RECTS(prgnRestore);
  nbox = REGION_NUM_RECTS(prgnRestore);

  psrcBase = (BITMAP_UNIT_PTR)(pPixmap->devPrivate.ptr);
  widthSrc = (long)pPixmap->devKind;

  first = TRUE;
  while (nbox-- > 0) {
    int xDst = pbox->x1;
    int xSrc = xDst - xorg;
    int wDst = pbox->x2 - xDst;
    int yDst = pbox->y1;
    int ySrc = yDst - yorg;
    int hDst = pbox->y2 - yDst;

#ifdef SOFTWARE_CURSOR
    (*HideCursorInXYWHFunc)(xDst, yDst, wDst, hDst);
#endif

    (*DrawColorImageFunc)(pPixmap->drawable.pScreen, xDst, yDst, wDst, hDst,
			  psrcBase + (ySrc * widthSrc) + xSrc,
			  (int)widthSrc, GXcopy, ~0, first);
    first = FALSE;

#ifdef SOFTWARE_CURSOR
    (*ShowCursorFunc)();
#endif

    pbox++;
  }
}

