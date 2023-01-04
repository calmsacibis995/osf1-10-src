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
static char *rcsid = "@(#)$RCSfile: model_color.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:48:42 $";
#endif
/*
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

#include "../ws/ws.h"
#include "ffbvram.h"

#ifdef MCMAP
#include "../cmap/cmap.h"
#endif

struct model_colormap cmapModel;

extern int wsFd;


void model_write_color(index, red, green, blue)
    unsigned int index;
    unsigned short red;
    unsigned short green;
    unsigned short blue;
{
    cmapModel.palette[0][index] = blue >> 8;
    cmapModel.palette[1][index] = green >> 8;
    cmapModel.palette[2][index] = red >> 8;
}


void modelStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *pdefs;
{
    ScreenPtr pScreen;
    long cmapIndex = 0;
    long i;
    /* 
     * we will be evil, and note that the server and driver use the same
     * structure.
     */
    xColorItem	directCells[256];
    ws_color_map_data cd;

    pScreen = pmap->pScreen;

#ifdef MCMAP
    if (pScreen->rootDepth == 8)
#endif
    {
	wsScreenPrivate *pPrivScreen = (wsScreenPrivate *) 
	    pmap->pScreen->devPrivates[wsScreenPrivateIndex].ptr;
	if (pmap != pPrivScreen->pInstalledMap) return;
    }
#ifdef MCMAP
    else
    {
	cmapIndex = CMAP_NOT_INSTALLED;
	
	for (i=0;  i < pScreen->maxInstalledCmaps; i++)
	{
	    if (pmap == CMAPScrDevPriv(pScreen)->pInstalledCmaps[i]) 
	    {
		cmapIndex = i;
	    }
	}
	
	/*
	 * all static color types are provided freebie by bt463, so don't use
	 * up any palette entries for them.
	 */
	switch (cmapIndex)
	{
	 case CMAP_TRUECOLOR24:
	 case CMAP_TRUECOLOR12:
	 case CMAP_STATICGRAY8:
	 case CMAP_NOT_INSTALLED:
	    return;
	}
    }
#endif
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

    {
	long top;
	i = 256 * cmapIndex;
	top = i + cd.ncells;
	for( ; i < top; cd.cells++,i++)
	{
	    model_write_color(cd.cells->index,
			      cd.cells->red,
			      cd.cells->green,
			      cd.cells->blue);
	}
    }
}


                    
void model_init_colorramp(pScreen)
    ScreenPtr pScreen;
{
    ws_color_map_data cd;
    xColorItem        *cells;
    int 	      i;
    ws_video_control vc;

    cd.screen = WS_SCREEN(pScreen);
    cd.map = 0;				/* only one... */
    cd.start = 0;
    cd.ncells = 256;
    cells = (xColorItem *)ALLOCATE_LOCAL(cd.ncells * sizeof(xColorItem));
    cd.cells = (ws_color_cell *)cells;
    for(i = 0; i < 256; i++){
        cells->pixel = i;
	cells->red = (i << 8) | i;
	cells->green = (i << 8) | i;
	cells->blue =  (i << 8) | i;
        cells->flags = 0;
	cells++;	
    }
    if (ioctl(wsFd, WRITE_COLOR_MAP, &cd) == -1) {
	ErrorF("WRITE_COLOR_MAP: failed to write color ramp\n");
    }
    DEALLOCATE_LOCAL(cells);

    /* toggling video cleans colormap */
    vc.screen = WS_SCREEN(pScreen);
    vc.control = SCREEN_OFF;
    if (ioctl(wsFd, VIDEO_ON_OFF, &vc) < 0)
	    ErrorF("VIDEO_ON_OFF: failed to turn screen off.\n");
    vc.control = SCREEN_ON;
    if (ioctl(wsFd, VIDEO_ON_OFF, &vc) < 0)
	ErrorF("VIDEO_ON_OFF: failed to turn screen on.\n");
}

/*
 * HISTORY
 */
/* HACKed up version of ..... 
 * ws_color.c - device specific color routines, stored in screen
 * 
 * Author:	Jim Gettys
 * 		Digital Equipment Corporation
 * 		Cambridge Research Laboratory
 * Date:	Sat January 24 1990
 */
