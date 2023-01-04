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
/****************************************************************************
**                                                                          *
**                  COPYRIGHT (c) 1988, 1989, 1990 BY                       *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

/*
Updated edg 18sep91:
    The select plane is painted in three places: CopyWindow, PaintWindow, and
    WindowExposures.  (For an analysis showing that this is sufficient ask
    Ed Goei.)  This is done by ropSetSelect().  The value written to the
    select plane depends on which hardware colormap a window's colormap will
    be installed into.  See ropcolor.c for more info.
*/

#include "X.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mistruct.h"
#include "regionstr.h"

#include "xfbstruct.h"
#include "cfb.h"

/*
Figure out which hardware colormap to use with this window.  This should
agree with HardwareMap() in ropcolor.c.
*/
#define xfbGetHardwareCmap(pWin) \
    ((wVisual(pWin) == pWin->drawable.pScreen->rootVisual) ? 0 : 1)

Bool
xfbCreateWindow(pWin)
    WindowPtr pWin;
{
    if (pWin->drawable.depth == 8) {
	return (cfbCreateWindow(pWin));
    } else {
	return (cfb32CreateWindow(pWin));
    }
}

Bool
xfbDestroyWindow(pWin)
    WindowPtr pWin;
{
    if (pWin->drawable.depth == 8) {
	return (cfbDestroyWindow(pWin));
    } else {
	return (cfb32DestroyWindow(pWin));
    }
}

Bool
xfbMapWindow(pWin)
    WindowPtr pWin;
{
    if (pWin->drawable.depth == 8) {
	return (cfbMapWindow(pWin));
    } else {
	return (cfb32MapWindow(pWin));
    }
}

Bool
xfbPositionWindow(pWin, x, y)
    WindowPtr pWin;
    int x, y;
{
    if (pWin->drawable.depth == 8) {
	return (cfbPositionWindow(pWin, x, y));
    } else {
	return (cfb32PositionWindow(pWin, x, y));
    }
}

Bool
xfbUnmapWindow(pWin)
    WindowPtr pWin;
{
    if (pWin->drawable.depth == 8) {
	return (cfbUnmapWindow(pWin));
    } else {
	return (cfb32UnmapWindow(pWin));
    }
}

/*
Look at pWin and all viewable children to see if they all have the same
depth and hardware colormap.
Outputs:
    pDepth:
	8 = all are depth 8
	24 = all are depth 24
	-1 = windows have different depths
    pHwCmap:
	if > 0 = hardware colormap number to use
	-1 = windows have different hardware colormaps
*/
void
xfbFindDepthAndHwCmap(pWin, pDepth, pHwCmap)
    WindowPtr pWin;
    int *pDepth;
    int *pHwCmap;
{
    WindowPtr pChild;
    int depth = pWin->drawable.depth;
    int hwCmap = xfbGetHardwareCmap(pWin);

    pChild = pWin;
    while (1) {
	if (pChild->viewable) {

	    if (depth != pChild->drawable.depth) {
		depth = -1;  /* windows are different depths */
	    }
	    if (hwCmap != xfbGetHardwareCmap(pChild)) {
		hwCmap = -1;  /* windows have different hardware cmaps */
	    }
	    if (depth < 0 && hwCmap < 0) {
		break;
	    }

	    if (pChild->firstChild) {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
    *pDepth = depth;
    *pHwCmap = hwCmap;
    return;
}

/*
Paint the appropriate select plane region corresponding to the window pWin.
The appropriate region is the clipped border plus the clipList region.
*/
static void
xfbCopyWindowSetCmap(pWin)
    WindowPtr pWin;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RegionPtr ptmpReg;
    
    ptmpReg = NotClippedByChildren(pWin);
    (*pScreen->Subtract)(ptmpReg, &pWin->borderClip, ptmpReg);
    (*pScreen->Union)(ptmpReg, ptmpReg, &pWin->clipList);
    ropSetSelect(pScreen, ptmpReg, xfbGetHardwareCmap(pWin));
    (*pScreen->RegionDestroy)(ptmpReg);
}

void
xfbCopyWindowSetCmaps(pWin)
    WindowPtr pWin;
{
    WindowPtr pChild;

    pChild = pWin;
    while (1) {
	if (pChild->viewable) {

	    xfbCopyWindowSetCmap(pChild);

	    if (pChild->firstChild) {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
}

void 
xfbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    int depth, hwCmap;

    xfbFindDepthAndHwCmap(pWin, &depth, &hwCmap);
    if (hwCmap != -1) {
	/* easy case: windows all have the same hardware cmap */
	ropSetSelect(pWin->drawable.pScreen, &pWin->borderClip, hwCmap);
    } else {
	xfbCopyWindowSetCmaps(pWin);
    }
    if (depth == 8) {
	cfbCopyWindow(pWin, ptOldOrg, prgnSrc);
    } else {
	cfb32CopyWindow(pWin, ptOldOrg, prgnSrc);
    }
}

Bool
xfbChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    unsigned long mask;
{
    if (pWin->drawable.depth == 8) {
	return (cfbChangeWindowAttributes(pWin, mask));
    } else {
	return (cfb32ChangeWindowAttributes(pWin, mask));
    }
}

void
xfbPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
    if (what == PW_BORDER) {
	ropSetSelect(pWin->drawable.pScreen, pRegion, xfbGetHardwareCmap(pWin));
    }
    if (pWin->drawable.depth == 8) {
	cfbPaintWindow(pWin, pRegion, what);
    } else {
	cfb32PaintWindow(pWin, pRegion, what);
    }
    return;
}

void 
xfbWindowExposures(pWin, prgn, other_exposed)
    WindowPtr pWin;
    RegionPtr prgn, other_exposed;
{
    ropSetSelect(pWin->drawable.pScreen, prgn, xfbGetHardwareCmap(pWin));
    miWindowExposures(pWin, prgn, other_exposed);
}
