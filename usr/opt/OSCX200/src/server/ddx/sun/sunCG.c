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
/*
 * $XConsortium: sunCG.c,v 1.4 91/08/23 16:11:04 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * sunCG.c
 *
 * Routines for managing a sun colormap
 */

#include    "sun.h"

#include    <sys/mman.h>
#include    <pixrect/memreg.h>
#include    "colormapst.h"
#include    "resource.h"
#include    <struct.h>

extern int TellLostMap(), TellGainedMap();

static void
sunCGUpdateColormap(pScreen, index, count, rmap, gmap, bmap)
    ScreenPtr	pScreen;
    int		index, count;
    u_char	*rmap, *gmap, *bmap;
{
    struct fbcmap sunCmap;

    sunCmap.index = index;
    sunCmap.count = count;
    sunCmap.red = &rmap[index];
    sunCmap.green = &gmap[index];
    sunCmap.blue = &bmap[index];

#ifdef SUN_WINDOWS
    if (sunUseSunWindows()) {
	static Pixwin *pw = 0;

	if (! pw) {
	    if ( ! (pw = pw_open(windowFd)) )
		FatalError( "sunCGUpdateColormap: pw_open failed\n" );
	    pw_setcmsname(pw, "X.V11");
	}
	pw_putcolormap(
	    pw, index, count, &rmap[index], &gmap[index], &bmap[index]);
    }
#endif SUN_WINDOWS

    if (ioctl(sunFbs[pScreen->myNum].fd, FBIOPUTCMAP, &sunCmap) < 0) {
	perror("sunCGUpdateColormap");
	FatalError( "sunCGUpdateColormap: FBIOPUTCMAP failed\n" );
    }
}


/*-
 *-----------------------------------------------------------------------
 * sunCGInstallColormap --
 *	Install given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Existing map is uninstalled.
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
sunCGInstallColormap(cmap)
    ColormapPtr	cmap;
{
    SetupScreen(cmap->pScreen);
    register int i;
    register Entry *pent;
    register VisualPtr pVisual = cmap->pVisual;
    u_char	  rmap[256], gmap[256], bmap[256];

    if (cmap == pPrivate->installedMap)
	return;
    if (pPrivate->installedMap)
	WalkTree(pPrivate->installedMap->pScreen, TellLostMap,
		 (pointer) &(pPrivate->installedMap->mid));
    if ((pVisual->class | DynamicClass) == DirectColor) {
	for (i = 0; i < 256; i++) {
	    pent = &cmap->red[(i & pVisual->redMask) >>
			      pVisual->offsetRed];
	    rmap[i] = pent->co.local.red >> 8;
	    pent = &cmap->green[(i & pVisual->greenMask) >>
				pVisual->offsetGreen];
	    gmap[i] = pent->co.local.green >> 8;
	    pent = &cmap->blue[(i & pVisual->blueMask) >>
			       pVisual->offsetBlue];
	    bmap[i] = pent->co.local.blue >> 8;
	}
    } else {
	for (i = 0, pent = cmap->red;
	     i < pVisual->ColormapEntries;
	     i++, pent++) {
	    if (pent->fShared) {
		rmap[i] = pent->co.shco.red->color >> 8;
		gmap[i] = pent->co.shco.green->color >> 8;
		bmap[i] = pent->co.shco.blue->color >> 8;
	    }
	    else {
		rmap[i] = pent->co.local.red >> 8;
		gmap[i] = pent->co.local.green >> 8;
		bmap[i] = pent->co.local.blue >> 8;
	    }
	}
    }
    pPrivate->installedMap = cmap;
    (*pPrivate->UpdateColormap) (cmap->pScreen, 0, 256, rmap, gmap, bmap);
    WalkTree(cmap->pScreen, TellGainedMap, (pointer) &(cmap->mid));
}

/*-
 *-----------------------------------------------------------------------
 * sunCGUninstallColormap --
 *	Uninstall given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	default map is installed
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
sunCGUninstallColormap(cmap)
    ColormapPtr	cmap;
{
    SetupScreen(cmap->pScreen);
    if (cmap == pPrivate->installedMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    ColormapPtr defMap = (ColormapPtr) LookupIDByType(defMapID,
							      RT_COLORMAP);

	    if (defMap)
		(*cmap->pScreen->InstallColormap)(defMap);
	    else
	        ErrorF("sunCG: Can't find default colormap\n");
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunCGListInstalledColormaps --
 *	Fills in the list with the IDs of the installed maps
 *
 * Results:
 *	Returns the number of IDs in the list
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
sunCGListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr	pScreen;
    Colormap	*pCmapList;
{
    SetupScreen(pScreen);
    *pCmapList = pPrivate->installedMap->mid;
    return (1);
}


/*-
 *-----------------------------------------------------------------------
 * sunCGStoreColors --
 *	Sets the pixels in pdefs into the specified map.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
static void
sunCGStoreColors(pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*pdefs;
{
    SetupScreen(pmap->pScreen);
    u_char	rmap[256], gmap[256], bmap[256];
    xColorItem	expanddefs[256];
    register int i;

    if (pmap != pPrivate->installedMap)
	return;
    if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
	ndef = cfbExpandDirectColors(pmap, ndef, pdefs, expanddefs);
	pdefs = expanddefs;
    }
    while (ndef--) {
	i = pdefs->pixel;
	rmap[i] = pdefs->red >> 8;
	gmap[i] = pdefs->green >> 8;
	bmap[i] = pdefs->blue >> 8;
	(*pPrivate->UpdateColormap) (pmap->pScreen, i, 1, rmap, gmap, bmap);
	pdefs++;
    }
}

sunCGScreenInit (pScreen)
    ScreenPtr	pScreen;
{
#ifndef STATIC_COLOR
    extern Bool	FlipPixels;
    SetupScreen (pScreen);
    pScreen->InstallColormap = sunCGInstallColormap;
    pScreen->UninstallColormap = sunCGUninstallColormap;
    pScreen->ListInstalledColormaps = sunCGListInstalledColormaps;
    pScreen->StoreColors = sunCGStoreColors;
    pPrivate->UpdateColormap = sunCGUpdateColormap;
    if (FlipPixels)
    {
	pScreen->whitePixel = 1;
	pScreen->blackPixel = 0;
    }
#endif
}
