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
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $XConsortium: ws_color.c,v 1.5 92/04/06 18:19:34 keith Exp $ */

/* 
 * ws_color.c - device specific color routines, stored in screen
 * 
 * Author:	Jim Gettys
 * 		Digital Equipment Corporation
 * 		Cambridge Research Laboratory
 * Date:	Sat January 24 1990
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/devio.h>
#include "X.h"          /* required for DoRed ... */
#include "Xproto.h"     /* required for xColorItem */

#include "misc.h"       /* required for colormapst.h */
#include "resource.h"
#include "scrnintstr.h"
#include "colormapst.h"
/* XXX */
#include <sys/workstation.h>

#include "ws.h"



/* these next two are DIX routines */
extern int      TellLostMap();
extern int      TellGainedMap();

extern int wsFd;

#define DoElement(mask) { 						\
    register unsigned int i; 						\
    for ( i = 0 ; i < nresult ; i++ ) { 				\
	if (outdefs[i].pixel == index)	 				\
	    break; 							\
	if (i == nresult) {						\
	    nresult++;							\
	    outdefs[i].pixel = index;					\
	    outdefs[i].flags |= (mask);					\
	    outdefs[i].red = pmap->red[index].co.local.red;		\
	    outdefs[i].green = pmap->green[index].co.local.green;	\
	    outdefs[i].blue = pmap->blue[index].co.local.blue;		\
	}								\
    }									\
}

#define AddElement(mask) { 		\
    pixel = red | green | blue; 	\
    for (i = 0; i < nresult; i++) 	\
  	if (outdefs[i].pixel == pixel) 	\
    	    break; 			\
    if (i == nresult) 			\
    { 					\
   	nresult++; 			\
	outdefs[i].pixel = pixel; 	\
	outdefs[i].flags = 0; 		\
    } 					\
    outdefs[i].flags |= (mask); 	\
    outdefs[i].red = pmap->red[red >> pVisual->offsetRed].co.local.red; \
    outdefs[i].green = pmap->green[green >> pVisual->offsetGreen].co.local.green; \
    outdefs[i].blue = pmap->blue[blue >> pVisual->offsetBlue].co.local.blue; \
}

wsExpandDirectColors(pmap, ndef, indefs, outdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *indefs, *outdefs;
{
    register VisualPtr pVisual = pmap->pVisual;
    unsigned int nresult = 0;

    if (pmap->pScreen->rootDepth == 8) {
	int		minred, mingreen, minblue;
	register int    red, green, blue;
	int		maxred, maxgreen, maxblue;
	int		stepred, stepgreen, stepblue;
	register int    pixel;
	register int    i;

	stepred = 1 << pVisual->offsetRed;
	stepgreen = 1 << pVisual->offsetGreen;
	stepblue = 1 << pVisual->offsetBlue;
	maxred = pVisual->redMask;
	maxgreen = pVisual->greenMask;
	maxblue = pVisual->blueMask;

	/* When simulating DirectColor on PseudoColor hardware, multiple
	 * entries of the colormap must be updated */

	for ( ; ndef-- ; indefs++ ) {
	    if (indefs->flags & DoRed) {
		red = indefs->pixel & pVisual->redMask;
		for (green = 0; green <= maxgreen; green += stepgreen) {
		    for (blue = 0; blue <= maxblue; blue += stepblue) {
			AddElement (DoRed);
		    }
		}
	    }
	    if (indefs->flags & DoGreen) {
		green = indefs->pixel & pVisual->greenMask;
		for (red = 0; red <= maxred; red += stepred) {
		    for (blue = 0; blue <= maxblue; blue += stepblue) {
			AddElement (DoGreen);
		    }
		}
	    }
	    if (indefs->flags & DoBlue) {
		blue = indefs->pixel & pVisual->blueMask;
		for (red = 0; red <= maxred; red += stepred) {
		    for (green = 0; green <= maxgreen; green += stepgreen) {
			AddElement (DoBlue);
		    }
		}
	    }
	}
    }
    else if ((pmap->pScreen->rootDepth == 24) | 
        (pmap->pScreen->rootDepth == 12)) {

	if ((ndef < 23) && (pmap->pScreen->rootDepth != 12)) {
	    for ( ; ndef-- ; indefs++ ) {
		register unsigned int index;

		if (indefs->flags & DoRed) {
		    index = (indefs->pixel & pVisual->redMask) >> 
			pVisual->offsetRed;
		    DoElement(DoRed);
		}
		if (indefs->flags & DoGreen) {
		    index = (indefs->pixel & pVisual->greenMask) >> 
			pVisual->offsetGreen;
		    DoElement(DoGreen);
		}
		if (indefs->flags & DoBlue) {
		    index = (indefs->pixel & pVisual->blueMask) >> 
			pVisual->offsetBlue;
		    DoElement(DoBlue);
		}
	    }
	}
	else {
	    int doCell[256];		/* XXX */

	    for ( ; ndef-- ; indefs++ ) {
		if (indefs->flags & DoRed) {
		    doCell[(indefs->pixel & pVisual->redMask) >> 
			pVisual->offsetRed] = 0xdeadf00d;
		}
		if (indefs->flags & DoGreen) {
		    doCell[(indefs->pixel & pVisual->greenMask) >> 
			pVisual->offsetGreen] = 0xdeadf00d;
		}
		if (indefs->flags & DoBlue) {
		    doCell[(indefs->pixel & pVisual->blueMask) >> 
			pVisual->offsetBlue] = 0xdeadf00d;
		}
	    }
	    for ( ndef = 0 ; ndef < 256 ; ndef++ ) {
		if (doCell[ndef] == 0xdeadf00d) {
		    outdefs->pixel = ndef;
		    outdefs->red = pmap->red[ndef].co.local.red;
		    outdefs->green = pmap->green[ndef].co.local.green;
		    outdefs->blue = pmap->blue[ndef].co.local.blue;
		    outdefs->flags = (DoRed | DoGreen | DoBlue);
		    outdefs++;
		    nresult++;
		}
	    }
	}
    }
    else
	ErrorF("wsStoreColors: unsupported DirectColor depth %d\n", 
	    pmap->pScreen->rootDepth);
    return nresult;
}


void
wsStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *pdefs;
{
    /* 
     * we will be evil, and note that the server and driver use the same
     * structure.
     */
    xColorItem	directCells[256];
    ws_color_map_data cd;
    wsScreenPrivate *pPrivScreen = (wsScreenPrivate *) 
        pmap->pScreen->devPrivates[wsScreenPrivateIndex].ptr;

    if (pmap != pPrivScreen->pInstalledMap) return;

    cd.screen = WS_SCREEN(pmap->pScreen);
    cd.map = 0;		/* only one... */
    cd.start = 0;
    /*
     * Check to see if we need to convert DirectColor from
     * pixel value to LUT index.
     */
    if ((pmap->class | DynamicClass) == DirectColor) {
	if (pmap->pVisual->ColormapEntries > 256)
	    FatalError("wsStoreColors: too many visual colormap entries?");
	cd.ncells = wsExpandDirectColors(pmap, ndef, pdefs, directCells);
	cd.cells = (ws_color_cell *) directCells;
    }
    else {
	cd.ncells = ndef;
	cd.cells = (ws_color_cell *)pdefs;
    }

    if (ioctl(wsFd, WRITE_COLOR_MAP, &cd) == -1) {
	ErrorF("error writing color map\n");
	return;
    }
}

#define _DUPRGB(C,V) ( (( (C) << (V)->offsetRed   ) & (V)->redMask  )\
		      |(( (C) << (V)->offsetGreen ) & (V)->greenMask)\
		      |(( (C) << (V)->offsetBlue  ) & (V)->blueMask ) )

void
wsInstallColormap(pcmap)
	ColormapPtr	pcmap;
{
    int         entries = pcmap->pVisual->ColormapEntries;
    Pixel *     ppix;
    xrgb *      prgb;
    xColorItem *defs;
    int         i;
    wsScreenPrivate *pPrivScreen = (wsScreenPrivate *) 
      pcmap->pScreen->devPrivates[wsScreenPrivateIndex].ptr;

    if (pcmap == pPrivScreen->pInstalledMap)  return;
    if ( pPrivScreen->pInstalledMap != NOMAPYET)
        WalkTree( pcmap->pScreen, TellLostMap, 
		 &pPrivScreen->pInstalledMap->mid);

    pPrivScreen->pInstalledMap = pcmap;

    /* if Turbochannel mfb (2 entry StaticGray), do not store any colors */
    if ((int) pcmap->devPriv != 2) {

	/* If we have an 8 entry TrueColor map, then get all 256 pixel
	    values so we can fake a TrueColor visual with an undecomposed
	    hardware colormap (ie. devPriv == 0).
	*/
	if (pcmap->devPriv == 0 && pcmap->class == TrueColor && entries == 8) {
	    entries = 256;
	}

	ppix = (Pixel *)ALLOCATE_LOCAL( entries * sizeof(Pixel));
	prgb = (xrgb *)ALLOCATE_LOCAL( entries * sizeof(xrgb));
	defs = (xColorItem *)ALLOCATE_LOCAL(entries * sizeof(xColorItem));
    
	/* devPriv == 1 means decomposed hardware colormap */
	if (((int) pcmap->devPriv == 1
	    && ((pcmap->class | DynamicClass) == DirectColor)) |
                (pcmap->pScreen->rootDepth == 12))
	    for( i = 0; i < entries; i++)
		   ppix[i] = _DUPRGB( i, pcmap->pVisual);
	else
	    for ( i=0; i<entries; i++)  ppix[i] = i;

	QueryColors( pcmap, entries, ppix, prgb);

	for ( i=0; i<entries; i++) { /* convert xrgbs to xColorItems */
	    defs[i].pixel = ppix[i] & 0xff;  	/* change pixel to index */
	    defs[i].red = prgb[i].red;
	    defs[i].green = prgb[i].green;
	    defs[i].blue = prgb[i].blue;
	    defs[i].flags =  DoRed|DoGreen|DoBlue;
	}
	(*pcmap->pScreen->StoreColors)( pcmap, entries, defs);
    
	DEALLOCATE_LOCAL(ppix);
	DEALLOCATE_LOCAL(prgb);
	DEALLOCATE_LOCAL(defs);
    }
    WalkTree(pcmap->pScreen, TellGainedMap, &pcmap->mid);
}


void
wsUninstallColormap(pcmap)
    ColormapPtr pcmap;
{
    /*  Replace installed colormap with default colormap */

    ColormapPtr defColormap;
    wsScreenPrivate *pPrivScreen = (wsScreenPrivate *) 
      pcmap->pScreen->devPrivates[wsScreenPrivateIndex].ptr;

    if ( pcmap != pPrivScreen->pInstalledMap) return;

    defColormap = (ColormapPtr) LookupIDByType( pcmap->pScreen->defColormap,
                        RT_COLORMAP);

    if (defColormap == pPrivScreen->pInstalledMap) return;

    (*pcmap->pScreen->InstallColormap) (defColormap);
}

int
wsListInstalledColormaps( pscr, pcmaps)
    ScreenPtr   pscr;
    Colormap *  pcmaps;
{
    wsScreenPrivate *pPrivScreen = (wsScreenPrivate *) 
    	pscr->devPrivates[wsScreenPrivateIndex].ptr;
    *pcmaps = pPrivScreen->pInstalledMap->mid;
    return 1;
}

