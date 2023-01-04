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
#include "cmap.h"

unsigned long	cmapGeneration;
unsigned long	cmapScreenPrivateIndex;
unsigned long	cmapWindowPrivateIndex;

/*ROUTINE:cmapSetupScreen
**++
**      cmapSetupScreen
**
** FUNCTIONAL DESCRIPTION:
**
**      Initializes all the color/colormap func vectors/wrappers
**
** FORMAL PARAMETERS:
**
**      pScreen             -  a pointer to the screen
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
**      pScreen->(*func)() are modified
**
**--
*/
Bool cmapSetupScreen(ScreenPtr pScreen,
		     void (*WriteWindowTypeTable)(),
		     void (*FillWindowType)(),
		     void (*LoadColormap)(),
		     int  (*ExpandDirectColors)(),
		     void (*ClearOverlays)(),
		     void (*ClearToBackground)(),
		     long (*OverlayVisualIndex)(),
		     long (*CmapTypeToIndex)(),
		     Bool (*IsOverlayWindow)())
{
    cmapScrPrivPtr          pScrPriv;
    VisualPtr               pVisual;
    long                     i;
    
    if (cmapGeneration != serverGeneration) {
        cmapScreenPrivateIndex = AllocateScreenPrivateIndex();
        if (cmapScreenPrivateIndex < 0)
            return FALSE;
        cmapWindowPrivateIndex = AllocateWindowPrivateIndex();
        cmapGeneration = serverGeneration;
    }
    if (!AllocateWindowPrivate(pScreen, cmapWindowPrivateIndex,
                               sizeof(cmapWinPrivRec)))
        return FALSE;
    
    pScrPriv = (cmapScrPrivPtr) xalloc (sizeof(cmapScrPrivRec) +
					(pScreen->maxInstalledCmaps *
					 sizeof(ColormapPtr)));
    if (!pScrPriv)
        return FALSE;

    pScreen->devPrivates[cmapScreenPrivateIndex].ptr = (pointer) pScrPriv;

    pScrPriv->WriteWindowTypeTable = WriteWindowTypeTable;
    pScrPriv->FillWindowType = FillWindowType;
    pScrPriv->LoadColormap = LoadColormap;
    pScrPriv->ClearOverlays = ClearOverlays;
    pScrPriv->ClearToBackground = ClearToBackground;
    pScrPriv->OverlayVisualIndex = OverlayVisualIndex;
    pScrPriv->IsOverlayWindow = IsOverlayWindow;
    pScrPriv->CmapTypeToIndex = CmapTypeToIndex;

    /* take over these functions */

    pScreen->InstallColormap = cmapInstallColormap;
    pScreen->UninstallColormap = cmapUninstallColormap;
    pScreen->ValidateTree = cmapValidateTree;
    pScreen->ListInstalledColormaps = cmapListInstalledColormaps;
    pScreen->StoreColors = cmapStoreColors;

    /* wrappers */

    pScrPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = cmapCloseScreen;

    pScrPriv->CreateWindow = pScreen->CreateWindow;
    pScreen->CreateWindow = cmapCreateWindow;

    pScrPriv->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = cmapDestroyWindow;

    pScrPriv->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pScreen->ChangeWindowAttributes = cmapChangeWindowAttributes;

    pScrPriv->RealizeWindow = pScreen->RealizeWindow;
    pScreen->RealizeWindow = cmapRealizeWindow;

    pScrPriv->UnrealizeWindow = pScreen->UnrealizeWindow;
    pScreen->UnrealizeWindow = cmapUnrealizeWindow;

    pScrPriv->CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = cmapCopyWindow;

    pScrPriv->CreateColormap = pScreen->CreateColormap;
    pScreen->CreateColormap = cmapCreateColormap;

    pScrPriv->DestroyColormap = pScreen->DestroyColormap;
    pScreen->DestroyColormap = cmapDestroyColormap;

    /* additional information */

    pScrPriv->pClearUnmapWinList = NULL;
    pScrPriv->pClearUnmapWinReg = (* pScreen->RegionCreate)(NullBox,0);
    
    pScrPriv->lastInstalledCmapIndex = CMAP_NOT_INSTALLED;
    pScrPriv->pLastInstalledCmap = NULL;
    for (i=0; i < pScreen->maxInstalledCmaps; i++) {
	pScrPriv->pInstalledCmaps[i] = NULL;
    }

    return TRUE;
}


/*ROUTINE:cmapScreenClose
**++
**	c m a p S c r e e n C l o s e
**
** FUNCTIONAL DESCRIPTION:
**
**	Destroys/deallocates screen specific resources
**
** FORMAL PARAMETERS:
**
**      int index;		... screen private index     
**      ScreenPtr pScreen;	... screen structure pointer 
**
** IMPLICIT INPUTS:
**
**      None.
**
** IMPLICIT OUTPUTS:
**
**      None.
**               
** FUNCTION VALUE:         
**
**      True if successful
**
** SIDE EFFECTS:
**
**      Frees memory/processes/other resources...
**--
*/

Bool cmapCloseScreen(int index, ScreenPtr pScreen)
{
    cmapScrPrivPtr   pScrPriv = CMAPScrDevPriv(pScreen);
    
    TRACE(ErrorF("cmapScreenClose()\n"));

    /*
     **  Release the private data structure(s)
     */

    if (pScreen->devPrivate) 
    {
	xfree(pScreen->devPrivate);
	pScreen->devPrivate = NULL;
    }

    pScrPriv = CMAPScrDevPriv(pScreen);

    while (pScrPriv->pClearUnmapWinList)
    {   /*clear out the list of windows */
        cmapClearUnmapWinPtr ClearUnmapWinPtr;
        ClearUnmapWinPtr = pScrPriv->pClearUnmapWinList;
        pScrPriv->pClearUnmapWinList = ClearUnmapWinPtr->flink;
        if (pScrPriv->pClearUnmapWinList)
        {
            pScrPriv->pClearUnmapWinList->blink = ClearUnmapWinPtr->blink;
        }

        (* pScreen->RegionDestroy)(ClearUnmapWinPtr->pClearUnmapWinReg);
        xfree(ClearUnmapWinPtr);
    }

    xfree(pScrPriv);
    pScreen->devPrivates[cmapScreenPrivateIndex].ptr = NULL;

    return TRUE;
}

/*
 * HISTORY
 */
