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
/* $XConsortium: cfbwindow.c,v 5.13 92/02/11 15:04:25 keith Exp $ */
/* $XConsortium: cfbwindow.c,v 5.14 92/03/13 16:20:59 eswu Exp $ */
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
#include "cfb.h"
#include "mistruct.h"
#include "regionstr.h"
#include "cfbmskbits.h"

extern WindowPtr *WindowTable;

Bool
cfbCreateWindow(pWin)
    WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    pPrivWin->pRotatedBorder = NullPixmap;
    pPrivWin->pRotatedBackground = NullPixmap;
    pPrivWin->fastBackground = FALSE;
    pPrivWin->fastBorder = FALSE;

#ifdef PIXMAP_PER_WINDOW
    /* Setup pointer to Screen pixmap */
    pWin->devPrivates[frameWindowPrivateIndex].ptr =
	(pointer) cfbGetScreenPixmap(pWin->drawable.pScreen);
#endif

    return TRUE;
}

Bool
cfbDestroyWindow(pWin)
    WindowPtr pWin;
{
    cfbPrivWin *pPrivWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

    if (pPrivWin->pRotatedBorder)
	cfbDestroyPixmap(pPrivWin->pRotatedBorder);
    if (pPrivWin->pRotatedBackground)
	cfbDestroyPixmap(pPrivWin->pRotatedBackground);
    return(TRUE);
}

/*ARGSUSED*/
Bool
cfbMapWindow(pWindow)
    WindowPtr pWindow;
{
    return(TRUE);
}

/* (x, y) is the upper left corner of the window on the screen 
   do we really need to pass this?  (is it a;ready in pWin->absCorner?)
   we only do the rotation for pixmaps that are 32 bits wide (padded
or otherwise.)
   cfbChangeWindowAttributes() has already put a copy of the pixmap
in pPrivWin->pRotated*
*/
/*ARGSUSED*/
Bool
cfbPositionWindow(pWin, x, y)
    WindowPtr pWin;
    int x, y;
{
    cfbPrivWin *pPrivWin;
    int setxy = 0;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);
    if (pWin->backgroundState == BackgroundPixmap && pPrivWin->fastBackground)
    {
	cfbXRotatePixmap(pPrivWin->pRotatedBackground,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBackground,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	setxy = 1;
    }

    if (!pWin->borderIsPixel &&	pPrivWin->fastBorder)
    {
	while (pWin->backgroundState == ParentRelative)
	    pWin = pWin->parent;
	cfbXRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.y - pPrivWin->oldRotate.y);
	setxy = 1;
    }
    if (setxy)
    {
	pPrivWin->oldRotate.x = pWin->drawable.x;
	pPrivWin->oldRotate.y = pWin->drawable.y;
    }
    return (TRUE);
}

/*ARGSUSED*/
Bool
cfbUnmapWindow(pWindow)
    WindowPtr pWindow;
{
    return (TRUE);
}

/* UNCLEAN!
   this code calls the bitblt helper code directly.

   cfbCopyWindow copies only the parts of the destination that are
visible in the source.
*/


void 
cfbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    RegionRec rgnDst;
    register BoxPtr pbox;
    register int dx, dy;
    register int i, nbox;
    WindowPtr pwinRoot;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    (* pWin->drawable.pScreen->RegionInit) (&rgnDst, NullBox, 0);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(&rgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(&rgnDst);
    nbox = REGION_NUM_RECTS(&rgnDst);
    if(!nbox || !(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
    {
	(* pWin->drawable.pScreen->RegionUninit) (&rgnDst);
	return;
    }
    ppt = pptSrc;

    for (i = nbox; --i >= 0; ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    cfbDoBitbltCopy((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		GXcopy, &rgnDst, pptSrc, ~0L);
    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionUninit) (&rgnDst);
}



/* swap in correct PaintWindow* routine.  If we can use a fast output
routine (i.e. the pixmap is paddable to 32 bits), also pre-rotate a copy
of it in devPrivates[cfbWindowPrivateIndex].ptr.
*/
Bool
cfbChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    unsigned long mask;
{
    register unsigned long index;
    register cfbPrivWin *pPrivWin;
    int width;
    WindowPtr	pBgWin;

    pPrivWin = (cfbPrivWin *)(pWin->devPrivates[cfbWindowPrivateIndex].ptr);

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
	cfbXRotatePixmap(pPrivWin->pRotatedBorder,
		      pWin->drawable.x - pPrivWin->oldRotate.x);
	cfbYRotatePixmap(pPrivWin->pRotatedBorder,
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
	    }
	    else if (pWin->backgroundState == ParentRelative)
	    {
		pPrivWin->fastBackground = FALSE;
		/* Rotate border to match parent origin */
		if (pPrivWin->pRotatedBorder) {
		    for (pBgWin = pWin->parent;
			 pBgWin->backgroundState == ParentRelative;
			 pBgWin = pBgWin->parent);
		    cfbXRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.x - pPrivWin->oldRotate.x);
		    cfbYRotatePixmap(pPrivWin->pRotatedBorder,
				  pBgWin->drawable.y - pPrivWin->oldRotate.y);
		}
	    }
	    else if (((width = (pWin->background.pixmap->drawable.width * PSZ)) <= LONG_BIT) &&
		     !(width & (width - 1)))
	    {
		cfbCopyRotatePixmap(pWin->background.pixmap,
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
	    if (((width = (pWin->border.pixmap->drawable.width * PSZ)) <= LONG_BIT) &&
		!(width & (width - 1)))
	    {
		for (pBgWin = pWin;
		     pBgWin->backgroundState == ParentRelative;
		     pBgWin = pBgWin->parent);
		cfbCopyRotatePixmap(pWin->border.pixmap,
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
    return (TRUE);
}

