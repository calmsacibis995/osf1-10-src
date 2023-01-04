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
/* $XConsortium: mfbwindow.c,v 5.9 92/03/31 17:54:35 keith Exp $ */
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************
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

******************************************************************/

#include "X.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mfb.h"
#include "mistruct.h"
#include "regionstr.h"
#include "maskbits.h"

extern WindowPtr *WindowTable;

Bool
mfbCreateWindow(pWin)
    register WindowPtr pWin;
{
    register mfbPrivWin *pPrivWin;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = FALSE;
    pPrivWin->fastBorder = FALSE;

    return (TRUE);
}

/* This always returns true, because Xfree can't fail.  It might be possible
 * on some devices for Destroy to fail */
Bool 
mfbDestroyWindow(pWin)
    WindowPtr pWin;
{
    register mfbPrivWin *pPrivWin;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);

    if (pPrivWin->pRotatedBorder)
	mfbDestroyPixmap(pPrivWin->pRotatedBorder);
    if (pPrivWin->pRotatedBackground)
	mfbDestroyPixmap(pPrivWin->pRotatedBackground);
    return (TRUE);
}

/*ARGSUSED*/
Bool mfbMapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* (x, y) is the upper left corner of the window on the screen 
   do we really need to pass this?  (is it a;ready in pWin->absCorner?)
   we only do the rotation for pixmaps that are 32 bits wide (padded
or otherwise.)
   mfbChangeWindowAttributes() has already put a copy of the pixmap
in pPrivWin->pRotated*
*/

/*ARGSUSED*/
Bool 
mfbPositionWindow(pWin, x, y)
    register WindowPtr pWin;
    int x, y;
{
    register mfbPrivWin *pPrivWin;
    int	reset = 0;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    if (pWin->backgroundState == BackgroundPixmap && pPrivWin->fastBackground)
    {
	mfbXRotatePixmap(pPrivWin->pRotatedBackground,
			 pWin->drawable.x - pPrivWin->oldRotate.x);
	mfbYRotatePixmap(pPrivWin->pRotatedBackground,
			 pWin->drawable.y - pPrivWin->oldRotate.y);
	reset = 1;
    }

    if (!pWin->borderIsPixel && pPrivWin->fastBorder)
    {
	while (pWin->backgroundState == ParentRelative)
	    pWin = pWin->parent;
	mfbXRotatePixmap(pPrivWin->pRotatedBorder,
			 pWin->drawable.x - pPrivWin->oldRotate.x);
	mfbYRotatePixmap(pPrivWin->pRotatedBorder,
			 pWin->drawable.y - pPrivWin->oldRotate.y);
	reset = 1;
    }
    if (reset)
    {
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }

    /* This is the "wrong" fix to the right problem, but it doesn't really
     * cost very much.  When the window is moved, we need to invalidate any
     * RotatedPixmap that exists in any GC currently validated against this
     * window.
     */
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    /* Again, we have no failure modes indicated by any of the routines
     * we've called, so we have to assume it worked */
    return (TRUE);
}

/*ARGSUSED*/
Bool 
mfbUnmapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   mfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
mfbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    RegionPtr prgnDst;
    register BoxPtr pbox;
    register int dx, dy;
    register int i, nbox;
    WindowPtr pwinRoot;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    prgnDst = (* pWin->drawable.pScreen->RegionCreate)(NULL, 1);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(prgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);
    if(!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
	return;
    ppt = pptSrc;

    for (i=nbox; --i >= 0; ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    mfbDoBitblt((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		GXcopy, prgnDst, pptSrc);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionDestroy)(prgnDst);
}



/* swap in correct PaintWindow* routine.  If we can use a fast output
routine (i.e. the pixmap is paddable to 32 bits), also pre-rotate a copy
of it in devPrivate.
*/
Bool
mfbChangeWindowAttributes(pWin, mask)
    register WindowPtr pWin;
    register unsigned long mask;
{
    register unsigned long index;
    register mfbPrivWin *pPrivWin;
    WindowPtr	pBgWin;

    pPrivWin = (mfbPrivWin *)(pWin->devPrivates[mfbWindowPrivateIndex].ptr);
    /*
     * When background state changes from ParentRelative and
     * we had previously rotated the fast border pixmap to match
     * the parent relative origin, rerotate to match window
     */
    if (mask & (CWBackPixmap | CWBackPixel) &&
	pWin->backgroundState != ParentRelative &&
	pPrivWin->fastBorder &&
	pPrivWin->oldRotate.x != pWin->drawable.x ||
	pPrivWin->oldRotate.y != pWin->drawable.y)
    {
	mfbXRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	mfbYRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }
    while(mask)
    {
	index = lowbit (mask);
	mask &= ~index;
	switch(index)
	{
	  case CWBackPixmap:
	      if (pWin->backgroundState == None)
	      {
		  pPrivWin->fastBackground = FALSE;
		  /* Rotate border to match parent origin */
		  if (pPrivWin->pRotatedBorder) {
		      for (pBgWin = pWin->parent;
			   pBgWin->backgroundState == ParentRelative;
			   pBgWin = pBgWin->parent);
		      mfbXRotatePixmap(pPrivWin->pRotatedBorder,
				    pBgWin->drawable.x - pPrivWin->oldRotate.x);
		      mfbYRotatePixmap(pPrivWin->pRotatedBorder,
				    pBgWin->drawable.y - pPrivWin->oldRotate.y);
		  }
	      }
	      else if (pWin->backgroundState == ParentRelative)
	      {
		  pPrivWin->fastBackground = FALSE;
	      }
	      else if ((pWin->background.pixmap->drawable.width <= PPW) &&
		       !(pWin->background.pixmap->drawable.width &
			 (pWin->background.pixmap->drawable.width - 1)))
	      {
		  mfbCopyRotatePixmap(pWin->background.pixmap,
				      &pPrivWin->pRotatedBackground,
				      pWin->drawable.x,
				      pWin->drawable.y);
		  if (pPrivWin->pRotatedBackground)
		  {
		      pPrivWin->fastBackground = TRUE;
		      pPrivWin->oldRotate.x = pWin->drawable.x;
		      pPrivWin->oldRotate.y = pWin->drawable.y;
		  }
		  else
		  {
		      pPrivWin->fastBackground = FALSE;
		  }
	      }
	      else
	      {
		  pPrivWin->fastBackground = FALSE;
	      }
	      break;

	  case CWBackPixel:
	      pPrivWin->fastBackground = FALSE;
	      break;

	  case CWBorderPixmap:
	      if ((pWin->border.pixmap->drawable.width <= PPW) &&
		  !(pWin->border.pixmap->drawable.width &
		    (pWin->border.pixmap->drawable.width - 1)))
	      {
		  for (pBgWin = pWin;
		       pBgWin->backgroundState == ParentRelative;
		       pBgWin = pBgWin->parent);
		  mfbCopyRotatePixmap(pWin->border.pixmap,
				      &pPrivWin->pRotatedBorder,
				      pBgWin->drawable.x,
				      pBgWin->drawable.y);
		  if (pPrivWin->pRotatedBorder)
		  {
		      pPrivWin->fastBorder = TRUE;
		      pPrivWin->oldRotate.x = pBgWin->drawable.x;
		      pPrivWin->oldRotate.y = pBgWin->drawable.y;
		  }
		  else
		  {
		      pPrivWin->fastBorder = FALSE;
		  }
	      }
	      else
	      {
		  pPrivWin->fastBorder = FALSE;
	      }
	      break;
	    case CWBorderPixel:
	      pPrivWin->fastBorder = FALSE;
	      break;
	}
    }
    /* Again, we have no failure modes indicated by any of the routines
     * we've called, so we have to assume it worked */
    return (TRUE);
}
