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
**	c m a p
**
**  FACILITY:
**
**      ddx/cmap -- DECwindows Device Specific Code for multiple colormaps
**             
**  ABSTRACT:
**
**	This module contains device specific routines for 
**	handling colors and managing the colormaps. Many of the routines
**      are based on SPXgt's implementation of multiple colormaps.
**      
**     
**  AUTHOR(S):
**
**      Khaled Tabbara
*/
#include "cmap.h"
#include "../ws/ws.h"

extern wsFd;
extern TellLostMap();
extern TellGainedMap();
extern wsExpandDirectColors();



/*ROUTINE:cmapExpandDirectColors
**++
**      c m a p E x p a n d D i r e c t C o l o r s 
**
** FUNCTIONAL DESCRIPTION:
**
**     This routine is similar to wsExpandDirectColors().
**     Unlike wsExpandDirectColors(), this routine does NOT look
**     at the root window's depth. It does the right thing as far
**     as expanding DirectColor color cells is concerned regardless of what the
**     default visual is (root window depth).
**
** FORMAL PARAMETERS:
**
**     pmap   -  pointer to the colormap
**     ndef   -  number of "unexpanded" color cells
**     indefs -  pointer to the "unexpanded" color cells
**     outdefs - pointer to the location where the "expanded" color cells
**               should be stored...
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**--
*/
#define Cookie	0x1FDEC50B

int cmapExpandDirectColors(ColormapPtr pmap, int ndef, xColorItem *indefs, xColorItem *outdefs)
{
    int doCell[256];
    VisualPtr pVisual = pmap->pVisual;
    long nresult = 0;
    long red, green, blue;
    long offsetRed, offsetGreen, offsetBlue;
    long stepred, stepgreen, stepblue;
    long maxred, maxgreen, maxblue;

    switch (pVisual->nplanes)
    {
     case 8:
	/* When simulating DirectColor on PseudoColor hardware LUT, multiple
	 * entries of the colormap must be updated.  We actually don't know that
	 * the hardware doesn't support, eg. 3-3-2, but it's fairly uncommon,
	 * so assume that we have to fake it.
	 */
	offsetRed = 0;
	offsetGreen = 0;
	offsetBlue = 0;
	stepred = 1 << pVisual->offsetRed;
	stepgreen = 1 << pVisual->offsetGreen;
	stepblue = 1 << pVisual->offsetBlue;
	maxred = pVisual->redMask;
	maxgreen = pVisual->greenMask;
	maxblue = pVisual->blueMask;
	break;
     case 12:
     case 24:
     case 32:
	offsetRed = pVisual->offsetRed;
	offsetGreen = pVisual->offsetGreen;
	offsetBlue = pVisual->offsetBlue;
	stepred = pVisual->redMask;
	stepgreen = pVisual->greenMask;
	stepblue = pVisual->blueMask;
	maxred = pVisual->redMask;
	maxgreen = pVisual->greenMask;
	maxblue = pVisual->blueMask;
	break;
     default:
	ErrorF("cmapExpandDirectColors: unsupported DirectColor depth %d\n", 
	       pVisual->nplanes);
	return 0;
    }

    for ( ; ndef-- ; indefs++ ) {
	if (indefs->flags & DoRed)
	{
	    red = (indefs->pixel & pVisual->redMask) >> offsetRed;
	    for (green = 0; green <= maxgreen; green += stepgreen) {
		for (blue = 0; blue <= maxblue; blue += stepblue) {
		    doCell[red|blue|green] = Cookie;
		}
	    }
	}
	if (indefs->flags & DoGreen)
	{
	    int green = (indefs->pixel & pVisual->greenMask) >> offsetGreen;
	    for (red = 0; red <= maxred; red += stepred) {
		for (blue = 0; blue <= maxblue; blue += stepblue) {
		    doCell[red|blue|green] = Cookie;
		}
	    }
	}
	if (indefs->flags & DoBlue)
	{
	    int blue = (indefs->pixel & pVisual->blueMask) >> offsetBlue;
	    for (red = 0; red <= maxred; red += stepred) {
		for (green = 0; green <= maxgreen; green += stepgreen) {
		    doCell[red|blue|green] = Cookie;
		}
	    }
	}
    }
    for ( ndef = 0 ; ndef < 256 ; ndef++ ) {
	if (doCell[ndef] == Cookie)
	{
	    doCell[ndef] = 0;
	    outdefs->pixel = ndef;
	    outdefs->red = pmap->red[ndef].co.local.red;
	    outdefs->green = pmap->green[ndef].co.local.green;
	    outdefs->blue = pmap->blue[ndef].co.local.blue;
	    outdefs->flags = (DoRed | DoGreen | DoBlue);
	    outdefs++;
	    nresult++;
	}
    }
    return nresult;
}



/*ROUTINE:cmapGetVisualIndex
**++
**      c m a p G e t V i s u a l I n d e x 
**
** FUNCTIONAL DESCRIPTION:
**
**     This is a convenience function. It searches thru the list of     
**     visuals for the screen and looks for one that matches the
**     class and depth parameters. If it finds one, then it returns
**     the index of the visual in the pScreen->visuals array. If there
**     is no match then it returns -1.
**
** FORMAL PARAMETERS:
**
**      pScreen             -  a pointer to the screen
**      class               -  class of desired visual (e.g. PseudoColor)
**      depth               -  depth of desired visual (e.g. 4)
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**--
*/
long cmapGetVisualIndex(ScreenPtr pScreen, short class, short depth)
{
    long visualIndex;
    long i;
    
    visualIndex = -1;
    
    for (i = 0; i < pScreen->numVisuals; i++)
    {
	if ((pScreen->visuals[i].class == class) &&
	    (pScreen->visuals[i].nplanes == depth))
	{
	    visualIndex = i;
	    break;
	}
    }
    TRACE(ErrorF("cmapGetVisualIndex(%d,%d) -> %d\n",
		 class, depth, visualIndex));

    if (visualIndex == -1)
    {
	FatalError("cmapGetVisualIndex: didn't find visual class %d depth %d\n",
		   class, depth);
    }

    return (visualIndex);
}



/*ROUTINE:cmapCreateColormap
**++
**      c m a p C r e a t e C o l o r m a p  
**
** FUNCTIONAL DESCRIPTION:
**
**      brief description of what it does.
**
** FORMAL PARAMETERS:
**
**      p1              -  ...
**	p2              -  ...
**	...etc
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
Bool cmapCreateColormap(ColormapPtr pMap)
{
    cmapPrivPtr     pMapPriv;
    unsigned long   i;
    VisualPtr       pVisual;
    unsigned long   lim, maxent, shift;
    cmapScrPrivPtr  pScrPriv;

    pMapPriv = (cmapPrivPtr) Xalloc(sizeof(cmapPrivRec));
    
    if (!pMapPriv) {
        ErrorF("cmapCreateColormap: could not Xalloc memory for cmapPriv\n");
	return FALSE;
    }
    TRACE(ErrorF("cmapCreateColormap()\n"));

    pMap->devPriv = (pointer) pMapPriv;

    pMapPriv->windowQFlink = (cmapWinPrivPtr) pMapPriv;
    pMapPriv->windowQBlink = (cmapWinPrivPtr) pMapPriv;
    pMapPriv->pMap = pMap;
    pMapPriv->tag = CMAP_NOT_INSTALLED;

    pVisual = pMap->pVisual;
    pScrPriv = CMAPScrDevPriv(pMap->pScreen);

    if (pVisual->class & DynamicClass)
    {
	return (Bool) (* pScrPriv->CreateColormap)(pMap);
    }

    /*
    ** if we get here then the colormap is static, so we have to 
    ** initialize it.
    */

    switch (pVisual->class)
    {
     case TrueColor:

	/*
	 ** These colormaps are always installed so let's store
	 ** the window tag now in pMapPriv->tag.
	 */

	if (pVisual->nplanes == 24) {
	    pMapPriv->tag = CMAP_TRUECOLOR24;
	}
	else if (pVisual->nplanes == 12) {
	    pMapPriv->tag = CMAP_TRUECOLOR12;
	}
	break;

     case StaticGray:
	if (pVisual->nplanes == 8) {
	    pMapPriv->tag = CMAP_STATICGRAY8;
	}
	break;

     case StaticColor:
	break;

     default:
	ErrorF("cmapCreateColormap: What is the class???\n");
    }

    return (Bool) (* pScrPriv->CreateColormap)(pMap);
}



/*ROUTINE:cmapDestroyColormap
**++
**      n a m e 
**
** FUNCTIONAL DESCRIPTION:
**
**      brief description of what it does.
**
** FORMAL PARAMETERS:
**
**      p1              -  ...
**	p2              -  ...
**	...etc
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
void cmapDestroyColormap(ColormapPtr pMap)
{
    cmapPrivPtr  pMapPriv;
    cmapWinPrivPtr   pWinPriv;
    cmapWinPrivPtr   pLastWinPriv=NULL;

    TRACE(ErrorF("cmapDestroyColormap()\n"));
    /*
    ** Check if the colormap is installed and uninstall it.
    */

    pMapPriv = CMAPDevPriv(pMap);
    
    if (pMapPriv->tag != CMAP_NOT_INSTALLED) {
	cmapUninstallColormap(pMap);
    }
    
    /*
    ** we need to NULL the pMap member in all the window privates
    ** that use this colormap. So we traverse the colormap's window
    ** private list and do just that. This guards against accidentally
    ** using the pMap from the window private after the colormap was
    ** deleted. This could happen if an evil client frees a window's
    ** colormap and then we get a request to paint the window's background
    ** and we try to find the window tag from the colormap pointer that we
    ** store in the window's dev private.
    */
	
    for (pWinPriv = pMapPriv->windowQFlink;
	 pWinPriv != (cmapWinPrivPtr) pMapPriv;
	 pWinPriv = pWinPriv->flink)
    {
	pWinPriv->pMap = NULL;
        pLastWinPriv = pWinPriv;
    }

    if (pLastWinPriv) pLastWinPriv->flink = NULL;

    (* CMAPScrDevPriv(pMap->pScreen)->DestroyColormap)(pMap);

    Xfree (pMapPriv);
    pMap->devPriv = NULL;
}
    


/*ROUTINE:cmapListInstalledColormaps
**++
**      n a m e 
**
** FUNCTIONAL DESCRIPTION:
**
**      brief description of what it does.
**
** FORMAL PARAMETERS:
**
**      p1              -  ...
**	p2              -  ...
**	...etc
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
int cmapListInstalledColormaps(ScreenPtr pScreen, Colormap *pMaps)
{
    long                 i;
    long                 j;
    cmapScrPrivPtr       pScrPriv;
    
    TRACE(ErrorF("cmapListInstalledColormaps()\n"));

    pScrPriv = CMAPScrDevPriv(pScreen);

    for (i=0, j=0; i < pScreen->maxInstalledCmaps; ++i) {
	if (pScrPriv->pInstalledCmaps[i] != NULL) {
	    pMaps[j] = pScrPriv->pInstalledCmaps[i]->mid;
	    j++;
	}
    }
    
    /*
    ** return the number of colormaps installed
    */

    return (j);
}



/*ROUTINE:cmapInstallColormap
**++
**      c m a p I n s t a l l C o l o r m a p 
**
** FUNCTIONAL DESCRIPTION:
**
**      This routine does all the work required to put the colormap
**      in the 
**
** FORMAL PARAMETERS:
**
**      p1              -  ...
**	p2              -  ...
**	...etc
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
void cmapInstallColormap(ColormapPtr pMap)
{

    ScreenPtr           pScreen;
    cmapScrPrivPtr       pScrPriv;
    cmapPrivPtr      pMapPriv;
    cmapWinPrivPtr       flink;
    WindowPtr           pWin;
    long                 i;
    Bool                wroteWindowTypeTable;
    long                mid;
    long                cmapId;
    RegionPtr           pClipRegion;    
    VisualPtr           pVisual;
    long                 cmapSize;
    xColorItem         *colors;
    Pixel              *pPix;
    xrgb               *pRGB;
    ColormapPtr        *pInstalledCmaps;
    
    pScreen = pMap->pScreen;
    pScrPriv =  CMAPScrDevPriv(pScreen);
    
    /*
    ** return if the colormap is already installed
    */

    for (i=0; i < pScreen->maxInstalledCmaps; i++) {
	if (pMap == pScrPriv->pInstalledCmaps[i]) {
	    TRACE(ErrorF("cmapInstallColormap(noop)\n"));
	    return;
	}
    }
    TRACE(ErrorF("cmapInstallColormap()\n"));

    pMapPriv = CMAPDevPriv(pMap);

    (* pScrPriv->LoadColormap)(pMap, &wroteWindowTypeTable);

    /*
    ** Now that it is actually installed in the hardware, tell
    ** everyone who cares.
    */

    cmapId = pMap->mid;
    WalkTree(pScreen, TellGainedMap, (char *) &cmapId);

    /* 
    ** Check if the window queue is empty.
    **
    ** If not empty then write the window type field of 
    ** all the windows that are in the colormap's queue.
    **
    ** we only need to do this if we had written a value to the
    ** the window type table (i.e. if wroteWindowTypeTable is TRUE)
    */

    if (wroteWindowTypeTable)
    {
	for (flink = pMapPriv->windowQFlink;
	     flink != (cmapWinPrivPtr) pMapPriv;
	     flink = flink->flink)
	{
	    pWin = flink->pWin;
	    pClipRegion = &(pWin->clipList);
	    
	    if (!REGION_NIL(pClipRegion)) {
		(*pScrPriv->FillWindowType)(pWin, pClipRegion, FALSE);
	    }
        }
    }
}



/*ROUTINE:cmapUninstallColormap
**++
**      c m a p U n i n s t a l l C o l o r m a p 
**
** FUNCTIONAL DESCRIPTION:
**
**      Uninc
**
** FORMAL PARAMETERS:
**
**      p1              -  ...
**	p2              -  ...
**	...etc
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
void cmapUninstallColormap(ColormapPtr pMap)
{
    long                i;
    long                cmapId   = pMap->mid;
    ScreenPtr	    	pScreen  = pMap->pScreen;
    cmapScrPrivPtr      pScrPriv = CMAPScrDevPriv(pScreen); 
    cmapPrivPtr	    	pMapPriv = CMAPDevPriv(pMap);

    TRACE(ErrorF("cmapUninstallColormap()\n"));

    /*
    ** notify all who care
    */

    if(CLIENT_ID(cmapId) !=  0)
    {
	WalkTree(pScreen, TellLostMap, &cmapId);
    }
    
    /*
    ** find the colormap in the "installed cmaps" array and set that entry
    ** to NULL. Then mark the colormap (in its dev private) as
    ** being NOT_INSTALLED.
    */

    for (i = 0; i < pScreen->maxInstalledCmaps; i++) {
	if (pScrPriv->pInstalledCmaps[i] == pMap) {
	    pScrPriv->pInstalledCmaps[i] = NULL;
	    pMapPriv->tag = CMAP_NOT_INSTALLED;

	}
    }
    
    if (pMap == pScrPriv->pDefCmap)
    {
	pScrPriv->pDefCmap = NULL;
    }
}



/*ROUTINE:cmapStoreColors
**++
**      n a m e 
**
** FUNCTIONAL DESCRIPTION:
**
**      brief description of what it does.
**
** FORMAL PARAMETERS:
**
**      p1              -  ...
**	p2              -  ...
**	...etc
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** RETURN VALUE:         
**
**      None.
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
void cmapStoreColors(ColormapPtr pMap,
		     long nData,
		     xColorItem *pData)
{
    ScreenPtr          pScreen;
    long               cmapIndex;
    long    	       i;
    ws_color_map_data  cmap_data;
    xColorItem         directCells[256];
    cmapScrPrivPtr     pScrPriv;
    
    pScreen = pMap->pScreen;
    pScrPriv = CMAPScrDevPriv(pScreen);

    /*
     ** return if the colormap is not installed or 
     ** not changeable (static)
     */

    cmapIndex = CMAP_NOT_INSTALLED;
    
    for (i=0;  i < pScreen->maxInstalledCmaps; i++)
    {
	if (pMap == pScrPriv->pInstalledCmaps[i])
	{
	    cmapIndex = i;
	}
    }

    /*
     * cmapIndex may need some massaging, since different options allocate
     * maps differently in hardware maps.
     */
    if ((cmapIndex == CMAP_NOT_INSTALLED) ||
	(cmapIndex = (* pScrPriv->CmapTypeToIndex)(cmapIndex)) < 0)
    {
	TRACE(ErrorF("cmapStoreColors(noop)\n"));
	return;
    }

    cmap_data.screen = WS_SCREEN(pScreen);
    cmap_data.map = cmapIndex;
    cmap_data.start = 0;

    if (pMap->class == DirectColor)
    {
	if (pMap->pVisual->ColormapEntries > 256)
	{
	    FatalError("cmapStoreColors: too many visual colormap entries");
	}
	
	cmap_data.ncells = (* pScrPriv->ExpandDirectColors)(pMap, nData, pData, directCells);
	cmap_data.cells = (ws_color_cell *) directCells;
    }
    else
    {
	cmap_data.ncells = nData;
	cmap_data.cells = (ws_color_cell *) pData;
    }
    
    if (ioctl(wsFd, WRITE_COLOR_MAP, &cmap_data) != 0)
    {
	perror("cmapStoreColors");
	FatalError("WRITE_COLOR_MAP failed\n");
    }
    TRACE(ErrorF("cmapStoreColors()\n"));
}

/*
 * HISTORY
 */
