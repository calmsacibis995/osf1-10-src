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
 * @(#)$RCSfile: cfbpmaxcolor.c,v $ $Revision: 1.2.5.2 $ (DEC) $Date: 1993/08/03 00:04:34 $
 */
/* 
 * cfbpmaxcolor.c - device specific color routines, stored in screen
 * 
 */

/*
 *  */



static char rcs_ident[] = "@(#)$RCSfile: cfbpmaxcolor.c,v $ $Revision: 1.2.5.2 $ (DEC) $Date: 1993/08/03 00:04:34 $";

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/devio.h>
#include <io/tc/pmioctl.h>
#include <io/tc/dc7085reg.h>
#include "X.h"          /* required for DoRed ... */
#include "Xproto.h"     /* required for xColorItem */

#include "misc.h"       /* required for colormapst.h */
#include "resource.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "cfbpmaxcolor.h"

#define NOMAPYET        (ColormapPtr) 1

static ColormapPtr pInstalledMap = NOMAPYET;

/* these next two are DIX routines */
extern int      TellLostMap();
extern int      TellGainedMap();

void
cfbpmaxStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *pdefs;
{
    int		idef;
    ColorMap	map;
    xColorItem	directDefs[256];

    if (pmap != pInstalledMap)
	return;

    if ((pmap->pVisual->class | DynamicClass) == DirectColor)
    {
	ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
	pdefs = directDefs;
    }
    for(idef = 0; idef < ndef; idef++)
    {
	map.Map = 0;
	map.index = (short) pdefs[idef].pixel;
	map.Entry.red = pdefs[idef].red >>8;
	map.Entry.green = pdefs[idef].green>>8;
	map.Entry.blue = pdefs[idef].blue>>8;
	ioctl(fdPM, QIOSETCMAP, &map);
    }

}

void
cfbpmaxInstallColormap(pcmap)
	ColormapPtr	pcmap;
{
    int         entries;
    Pixel *     ppix;
    xrgb *      prgb;
    xColorItem *defs;
    int         i;

    if ( pcmap == pInstalledMap)
        return;

    if ((pcmap->pVisual->class | DynamicClass) == DirectColor)
	entries = (pcmap->pVisual->redMask |
		   pcmap->pVisual->greenMask |
		   pcmap->pVisual->blueMask) + 1;
    else
	entries = pcmap->pVisual->ColormapEntries;

    ppix = (Pixel *)ALLOCATE_LOCAL( entries * sizeof(Pixel));
    prgb = (xrgb *)ALLOCATE_LOCAL( entries * sizeof(xrgb));
    defs = (xColorItem *)ALLOCATE_LOCAL(entries * sizeof(xColorItem));

    if ( pInstalledMap != NOMAPYET)
        WalkTree( pcmap->pScreen, TellLostMap, &pInstalledMap->mid);
    pInstalledMap = pcmap;
    for ( i=0; i<entries; i++)
        ppix[i] = i;
    QueryColors( pcmap, entries, ppix, prgb);
    for ( i=0; i<entries; i++) /* convert xrgbs to xColorItems */
    {
        defs[i].pixel = ppix[i];
        defs[i].red = prgb[i].red;
        defs[i].green = prgb[i].green;
        defs[i].blue = prgb[i].blue;
        defs[i].flags =  DoRed|DoGreen|DoBlue;
    }
    cfbpmaxStoreColors( pcmap, entries, defs);
    WalkTree(pcmap->pScreen, TellGainedMap, &pcmap->mid);

    DEALLOCATE_LOCAL(ppix);
    DEALLOCATE_LOCAL(prgb);
    DEALLOCATE_LOCAL(defs);
}


void
cfbpmaxUninstallColormap(pcmap)
    ColormapPtr pcmap;
{
    /*  Replace installed colormap with default colormap */

    ColormapPtr defColormap;

    if ( pcmap != pInstalledMap)
        return;

    defColormap = (ColormapPtr) LookupIDByType( pcmap->pScreen->defColormap,
                        RT_COLORMAP);

    if (defColormap == pInstalledMap)
        return;

    (*pcmap->pScreen->InstallColormap) (defColormap);
}

int
cfbpmaxListInstalledColormaps( pscr, pcmaps)
    ScreenPtr   pscr;
    Colormap *  pcmaps;
{
    *pcmaps = pInstalledMap->mid;
    return 1;
}

