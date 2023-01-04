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
/*
jpl  Jun 92
    8-bit DirectColor	supported

edg  6 Aug 91
RasterOps (TX) card colormap management module.  This code talks to the
ws driver to install hardware colormaps and store colors in them.
Code was originally copied from ws_color.c and modified.

Updates: 18 Sep 91
The current version is really a total redesign based on a careful analysis
of all the possible types of colormaps that need to be supported and how
best to support them.  (Ask Ed Goei for a pointer to this analysis.)
The types of colormaps supported are:

    8-bit StaticGray
    8-bit GrayScale
    8-bit StaticColor
    8-bit PseudoColor
    8-bit TrueColor	[simulated in software]
    8-bit DirectColor	[not yet supported, but could be if we copy R5 code]
    24-bit TrueColor
    24-bit DirectColor

The TX board has essentially two identical hardware colormaps #0 and #1.
The select plane determines which colormap a particular pixel indexes into.
Currently, colormaps of the same type (ie. depth and visual class) as the
default colormap of the screen will be stored in #0 and any other colormap
will be stored in #1.  This avoids technicolor in the most common cases and
is not too difficult to implement.  For this to work, functions in
xfbwindow.c and xfbbanksw.c must set the select plane accordingly.

This is useful for a video conferencing application that uses StaticGray,
for example, because it allows a PsuedoColor map and a StaticGray map to be
installed simultaneously.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include "X.h"          /* required for DoRed ... */
#include "Xproto.h"     /* required for xColorItem */
#include "resource.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include <sys/workstation.h>


/* these are for screen privates */
static int ropColorScreenPrivIndex;
static unsigned long ropColorGeneration = 0;

typedef struct {
    ColormapPtr installedMaps[2];  /* installed hw maps */
    Bool (*CloseScreen)();
} ropColorPrivScreen;

#define NO_MAP ((ColormapPtr) 0)  /* means no map is installed */

#define LOG_MAXCELLS	8
/* code assumes both hardware maps have this many cells */
#define MAXCELLS	(1 << LOG_MAXCELLS)
#define INDEX_MASK	(MAXCELLS - 1)


#define IsDecomposed(pcmap) \
    (((pcmap)->class | DynamicClass) == DirectColor)

/* true if pcmap is a 24-bit depth decomposed colormap */
#define IsDecomposed24(pcmap) \
    (IsDecomposed(pcmap) && (pcmap)->pVisual->ColormapEntries == MAXCELLS)

/* Returns hardware colormap number to store pcmap into.  This should agree
    with xfbGetHardwareCmap() in xfbGetHardwareCmap().
*/
#define HardwareMap(pcmap) \
    (((pcmap)->pVisual->vid == (pcmap)->pScreen->rootVisual) ? 0 : 1)


/* these next two are DIX routines */
extern int      TellLostMap();
extern int      TellGainedMap();

extern int wsFd;
extern ws_screen_descriptor screenDesc[];


/*
Expand a list of pixel values into a list of indexes.
This should really go in ws_color.c since it is needed there also.
(Modified from fhsu's version of ws_color.c.)
*/
wsExpandPixelToIndexes(pcmap, ndef, indefs, outdefs)
    ColormapPtr pcmap;
    int         ndef;
    xColorItem  *indefs;
    ws_color_cell *outdefs;
{
#define DO_CELL_MAGIC 0xf00d
    int doCell[MAXCELLS];
    VisualPtr pVisual = pcmap->pVisual;
    int nresult = 0;

    /* mark at least all the cells we will modify (others may be marked
	already which is very unlikely but OK)
    */
    for ( ; ndef--; indefs++) {
	if (indefs->flags & DoRed) {
	    doCell[(indefs->pixel & pVisual->redMask) >> pVisual->offsetRed]
		= DO_CELL_MAGIC;
	}
	if (indefs->flags & DoGreen) {
	    doCell[(indefs->pixel & pVisual->greenMask) >> pVisual->offsetGreen]
		= DO_CELL_MAGIC;
	}
	if (indefs->flags & DoBlue) {
	    doCell[(indefs->pixel & pVisual->blueMask) >> pVisual->offsetBlue]
		= DO_CELL_MAGIC;
	}
    }

    /* read out RGB values corresponding to all marked cells */
    for (ndef = 0 ; ndef < MAXCELLS; ndef++) {
	if (doCell[ndef] == DO_CELL_MAGIC) {
	    outdefs->index = ndef;
	    outdefs->red = pcmap->red[ndef].co.local.red;
	    outdefs->green = pcmap->green[ndef].co.local.green;
	    outdefs->blue = pcmap->blue[ndef].co.local.blue;
	    outdefs++;
	    nresult++;
	}
    }
    return (nresult);
}


static void
ropStoreColors(pcmap, ndef, pdefs)
    ColormapPtr pcmap;
    int         ndef;
    xColorItem  *pdefs;
{
    xColorItem	directCells[MAXCELLS];
    ws_color_map_data cd;
    ropColorPrivScreen *screenPriv = (ropColorPrivScreen *)
	pcmap->pScreen->devPrivates[ropColorScreenPrivIndex].ptr;

    cd.map = HardwareMap(pcmap);

    if (pcmap != screenPriv->installedMaps[cd.map]) return;

    if (IsDecomposed(pcmap)) {
	/* is this if correct? compare to ws_color.c... jpl */
	if (pcmap->pVisual->ColormapEntries == 8) {
	    /* support for 8-bit DirectColor goes here, see
		cfbExpandDirectColor() in R5
	    */
	    /*return;*/
	    cd.ncells = wsExpandDirectColors(pcmap, ndef, pdefs, directCells);
	    cd.cells = (ws_color_cell *)directCells;
	} else {
	    ws_color_cell indexCells[MAXCELLS];

	    cd.ncells = wsExpandPixelToIndexes(pcmap, ndef, pdefs, indexCells);
	    cd.cells = indexCells;
	}
    } else {
	/* color cell index == pixel, for this case */
	cd.cells = (ws_color_cell *) pdefs;
	cd.ncells = ndef;
    }

    cd.screen = wsPhysScreenNum(pcmap->pScreen);
    cd.start = 0;

    /* XXX older versions of the driver do not support 24-bit depth maps.
	Remove this code later!
    */
    {
	extern int txDebugOption;
	if ((txDebugOption & 0x2) && cd.map == 1) {
#if defined(__alpha) && defined(__osf1)
	    ErrorF("ropcolor doesn't support 24-plane\n");
#endif
	    return;
	}
    }

    if (ioctl(wsFd, WRITE_COLOR_MAP, &cd) == -1) {
	ErrorF("error writing color map\n");
    }
}


#define _DUPRGB(C,V) ( (( (C) << (V)->offsetRed   ) & (V)->redMask  )\
		      |(( (C) << (V)->offsetGreen ) & (V)->greenMask)\
		      |(( (C) << (V)->offsetBlue  ) & (V)->blueMask ) )

/*
Notes: If we have an 8 entry (ie. 8-bit) decomposed map, then get all 256
pixel values so we can fake a TrueColor visual with an undecomposed
hardware colormap.
*/
static void
ropInstallColormap(pcmap)
    ColormapPtr	pcmap;
{
    Pixel	pixels[MAXCELLS];
    xrgb	rgbs[MAXCELLS];
    ws_color_cell wsCells[MAXCELLS];
    ws_color_map_data cd;
    int         i;
    int		hwmap;  /* hardware map number */
    ColormapPtr *installedMaps = ((ropColorPrivScreen *)
	pcmap->pScreen->devPrivates[ropColorScreenPrivIndex].ptr)
	->installedMaps;

    hwmap = HardwareMap(pcmap);

    if (pcmap == installedMaps[hwmap]) return;

    if (installedMaps[hwmap] != NO_MAP) {
        WalkTree(pcmap->pScreen, TellLostMap, &installedMaps[hwmap]->mid);
    }

    installedMaps[hwmap] = pcmap;

    if (IsDecomposed24(pcmap)) {
	for (i = 0; i < MAXCELLS; i++) {
	    pixels[i] = _DUPRGB(i, pcmap->pVisual);
	}
    } else {
	for (i = 0; i < MAXCELLS; i++) {
	    pixels[i] = i;
	}
    }

    QueryColors(pcmap, MAXCELLS, pixels, rgbs);

    /* fill in ws_color_cell array: */
    for (i = 0; i < MAXCELLS; i++) {
	/* change pixel to index for decomposed24bit case, else just copy */
	wsCells[i].index = pixels[i] & INDEX_MASK;

	wsCells[i].red = rgbs[i].red;
	wsCells[i].green = rgbs[i].green;
	wsCells[i].blue = rgbs[i].blue;
    }

    cd.screen = wsPhysScreenNum(pcmap->pScreen);
    cd.map = hwmap;
    cd.start = 0;
    cd.ncells = MAXCELLS;
    cd.cells = wsCells;

    /* XXX older versions of the driver do not support 24-bit depth maps.
	Remove this code later!
    */
    {
	extern int txDebugOption;
	if ((txDebugOption & 0x2) && cd.map == 1) {
	    WalkTree(pcmap->pScreen, TellGainedMap, &pcmap->mid);
	    return;
	}
    }

#ifdef DEBUG
    printf("ropInstallColormap(): cd.map = %d, map id = %d\n", cd.map,
	pcmap->mid);
#endif /* DEBUG */

    if (ioctl(wsFd, WRITE_COLOR_MAP, &cd) == -1) {
	ErrorF("error writing color map\n");
    }

    WalkTree(pcmap->pScreen, TellGainedMap, &pcmap->mid);
}

static void
ropUninstallColormap(pcmap)
    ColormapPtr pcmap;
{
    ColormapPtr defColormap;
    int hwmap;
    ropColorPrivScreen *screenPriv = (ropColorPrivScreen *)
	pcmap->pScreen->devPrivates[ropColorScreenPrivIndex].ptr;

    hwmap = HardwareMap(pcmap);

    if (pcmap != screenPriv->installedMaps[hwmap]) return;

    defColormap = (ColormapPtr) LookupIDByType(pcmap->pScreen->defColormap,
	RT_COLORMAP);

    /* if there is no default cmap to replace the map being uninstalled ... */
    if (hwmap != HardwareMap(defColormap)) {
	screenPriv->installedMaps[hwmap] = NO_MAP;
	return;
    }

    if (defColormap == screenPriv->installedMaps[hwmap]) return;

    (*pcmap->pScreen->InstallColormap) (defColormap);
}

static int
ropListInstalledColormaps(pscr, pcmaps)
    ScreenPtr   pscr;
    Colormap *  pcmaps;
{
    int i, n;
    ropColorPrivScreen *screenPriv = (ropColorPrivScreen *)
	pscr->devPrivates[ropColorScreenPrivIndex].ptr;

    n = 0;
    for (i = 0; i < 2; i++) {
	if (screenPriv->installedMaps[i] != NO_MAP) {
	    pcmaps[n++] = screenPriv->installedMaps[i]->mid;
	}
    }
    return (n);
}

static Bool
ropColorCloseScreen(index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    ropColorPrivScreen *screenPriv = (ropColorPrivScreen *)
	pScreen->devPrivates[ropColorScreenPrivIndex].ptr;

    pScreen->CloseScreen = screenPriv->CloseScreen;

    xfree(screenPriv);
    return ((*pScreen->CloseScreen)(index, pScreen));
}

Bool
ropColorInit(pScreen)
    register ScreenPtr pScreen;
{
    ropColorPrivScreen *screenPriv;

    if (ropColorGeneration != serverGeneration) {
	if ((ropColorScreenPrivIndex = AllocateScreenPrivateIndex()) < 0) {
	    return (FALSE);
	}
	ropColorGeneration = serverGeneration;
    }

    /* install colormap handling procs */
    pScreen->StoreColors = ropStoreColors;
    pScreen->InstallColormap = ropInstallColormap;
    pScreen->UninstallColormap = ropUninstallColormap;
    pScreen->ListInstalledColormaps = ropListInstalledColormaps;

    screenPriv = (ropColorPrivScreen *) xalloc(sizeof(ropColorPrivScreen));
    if (!screenPriv) {
	return (FALSE);
    }
    screenPriv->installedMaps[0] = NO_MAP;
    screenPriv->installedMaps[1] = NO_MAP;
    pScreen->devPrivates[ropColorScreenPrivIndex].ptr = (pointer) screenPriv;

    /* wrap CloseScreen so we can clean up when screen is closed */
    screenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = ropColorCloseScreen;

    return (TRUE);
}
