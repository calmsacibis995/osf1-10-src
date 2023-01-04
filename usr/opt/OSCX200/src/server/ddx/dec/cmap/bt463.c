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
**	b t 4 6 3
**
**  FACILITY:
**
**      ddx/bt463 -- DECwindows Device Specific Code for bt463 VDAC
**             
**  ABSTRACT:
**
**	This module contains device specific routines for 
**	handling the bt463 driver.
**     
**  AUTHOR(S):
**
**      Khaled Tabbara
*/
#include "cmap.h"


void bt463LoadColormap(ColormapPtr pMap, Bool *doWriteWindowTypeTable)
{
    VisualPtr		pVisual;
    ScreenPtr		pScreen;
    long		cmapSize, mid, where, i;
    ColormapPtr		*pInstalledCmaps;
    cmapScrPrivPtr	pScrPriv;
    Bool		doWritePalette;
    xColorItem         *colors;
    Pixel              *pPix;
    xrgb               *pRGB;

    *doWriteWindowTypeTable = doWritePalette = FALSE;

    pVisual = pMap->pVisual;
    pScreen = pMap->pScreen;
    pScrPriv = CMAPScrDevPriv(pScreen);

    cmapSize = pVisual->ColormapEntries;
    
    mid = -1;
    where = CMAP_NOT_INSTALLED;

    if ((pVisual->class == TrueColor) && (pVisual->nplanes == 24))
    {
	where = CMAP_TRUECOLOR24;
    }
    else if ((pVisual->class == TrueColor) && (pVisual->nplanes == 12))
    {
	where = CMAP_TRUECOLOR12;
    }	
    else if ((pVisual->class == StaticGray) && (pVisual->nplanes == 8))
    {
	where = CMAP_STATICGRAY8;
    }
    else if ((pVisual->class == PseudoColor) && (pVisual->nplanes == 4))
    {
	doWritePalette = TRUE;

	where = CMAP_OVERLAY_PALETTE;

	if (pScrPriv->pInstalledCmaps[CMAP_OVERLAY_PALETTE] != NULL)
	{
	    mid = (pScrPriv->pInstalledCmaps[CMAP_OVERLAY_PALETTE])->mid;
	}
    }
    else
    {
	*doWriteWindowTypeTable = doWritePalette = TRUE;

	pInstalledCmaps = pScrPriv->pInstalledCmaps;
	
        if (pInstalledCmaps[CMAP_FIRST_PALETTE] == NULL)
        {
            where = CMAP_FIRST_PALETTE;
        }
        else if (pInstalledCmaps[CMAP_SECOND_PALETTE] == NULL)
        {
            where = CMAP_SECOND_PALETTE;
        }
    }

    if (where == CMAP_NOT_INSTALLED)
    {
	/*
	 ** No empty slots found. So we'll have to swap out a colormap.
	 ** Keep the default map around.
	 */

	if (pScrPriv->lastInstalledCmapIndex == CMAP_FIRST_PALETTE)
	{
	    if (pScrPriv->pInstalledCmaps[CMAP_SECOND_PALETTE]->mid !=
		pScreen->defColormap)
	    {
		where = CMAP_SECOND_PALETTE;
	    }
	    else
	    {
		where = CMAP_FIRST_PALETTE;
	    }
	}
	else
	{
	    if (pScrPriv->pInstalledCmaps[CMAP_FIRST_PALETTE]->mid !=
		pScreen->defColormap)
	    {
		where = CMAP_FIRST_PALETTE;
	    }
	    else
	    {
		where = CMAP_SECOND_PALETTE;
	    }
	}
	mid = (pScrPriv->pInstalledCmaps[where])->mid;
    }
    
    if (*doWriteWindowTypeTable)
    {
	(* pScrPriv->WriteWindowTypeTable)(pScreen, pMap, where);
    }

    /*
     ** check if we had to swap out a colormap. If true then 
     ** notify all who care...
     */

    if (mid != -1)
    {
	WalkTree(pScreen, TellLostMap, &mid);
	CMAPDevPriv(pScrPriv->pInstalledCmaps[where])->tag = 
	    CMAP_NOT_INSTALLED;
    }

    pScrPriv->lastInstalledCmapIndex = where;
    pScrPriv->pLastInstalledCmap = pMap;
    pScrPriv->pInstalledCmaps[where] = pMap;
    
    /*
     ** The Bt463 provides the TrueColor24, TrueColor12, and StaticGray8 
     ** colormaps for free (i.e. they bypass the LUT). So we don't really 
     ** need to load anything into the colormap Palette RAM for those cases.
     */

    if (doWritePalette)
    {
	pPix = (Pixel *) ALLOCATE_LOCAL( cmapSize * sizeof(Pixel));
	pRGB = (xrgb *) ALLOCATE_LOCAL( cmapSize * sizeof(xrgb));
	colors = (xColorItem *) ALLOCATE_LOCAL(cmapSize * sizeof(xColorItem));
	
	
	if (pVisual->class == DirectColor)
	{
	    for (i=0; i < cmapSize; i++)
	    {
		pPix[i] = ((i << pVisual->offsetRed) |
			   (i << pVisual->offsetGreen) |
			   (i << pVisual->offsetBlue));
	    }
	}
	else
	{
	    for (i=0; i < cmapSize; i++)
	    {
		pPix[i] = i;
	    }
	}

	QueryColors(pMap, cmapSize, pPix, pRGB);

	for (i = 0; i < cmapSize; i++)
	{
	    colors[i].pixel = pPix[i];
	    colors[i].red = pRGB[i].red;
	    colors[i].green = pRGB[i].green;
	    colors[i].blue = pRGB[i].blue;
	    colors[i].flags = DoRed | DoGreen | DoBlue;
	}
	
	(* pScreen->StoreColors)(pMap, cmapSize, colors);

	DEALLOCATE_LOCAL(pPix);
	DEALLOCATE_LOCAL(pRGB);
	DEALLOCATE_LOCAL(colors);
    }
    
    /*
     ** take care of the colormap's private field
     */

    CMAPDevPriv(pMap)->tag = where;

    /*
     ** doWriteWindowTypeTable will be FALSE if the visual was 
     ** 24-plane TrueColor, 8-plane StaticGray, 4-plane Overlay PseudoColor
     ** and 12-plane TrueColor.
     */
}


long bt463CmapTypeToIndex(long cmapType)
{
    /* freebie colormaps -> -1 (don't touch) */
    if ((cmapType == CMAP_NOT_INSTALLED) || 
        (cmapType == CMAP_TRUECOLOR24) ||
        (cmapType == CMAP_TRUECOLOR12) ||
        (cmapType == CMAP_STATICGRAY8))
    {
	return -1;
    }
    return cmapType;
}



long bt463OverlayVisualIndex(ScreenPtr pScreen)
{
    return (long) cmapGetVisualIndex(pScreen, PseudoColor, 4);
}



Bool bt463IsOverlayWindow(WindowPtr pWin)
{
    return (Bool) (pWin->drawable.depth == 4);
}

/*
 * HISTORY
 */
