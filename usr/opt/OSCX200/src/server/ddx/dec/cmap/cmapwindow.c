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
**	p v x W i n d o w
**
**  FACILITY:
**
**      ddxpvx -- DECwindows Device Specific Code for PixelVision
**             
**  ABSTRACT:
**
**	This module contains device specific routines for creating, mapping,
**	unmapping, changing, and destroying windows.
**                           
**  AUTHOR(S):
**
**      Khaled Tabbara (re-used from MIT CFB code)
*/

/*
**  Include file list
*/
#include	<sys/types.h>
#include	"cmap.h"

/*
** Module Specific Prototypes/externs...
*/

extern  WindowPtr         *WindowTable;


#define OVERLAY_PROPERTY_STRING  "SERVER_OVERLAY_VISUALS"


/*ROUTINE:cmapCreateWindow
**++
**      p v x C r e a t e W i n d o w 
**
** FUNCTIONAL DESCRIPTION:
**
**      This routine is called by DIX when a new window has been created.
**      It initializes all the members in the dev priv.
**
**
** FORMAL PARAMETERS:
**
**      pWin  - pointer to the window that has been created
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
**      Always returns TRUE
**
** SIDE EFFECTS:     
**
**      None.
**
**--
*/
Bool cmapCreateWindow(WindowPtr pWin)
{
    cmapWinPrivPtr  pWinPriv;
    cmapPrivPtr	    pmapPriv;
    WindowPtr       pWinWithOpt;
    ColormapPtr     pMap;
    Atom            property;
    CARD32          propertyData[4];
    ScreenPtr       pScreen;
    long            i;
    cmapScrPrivPtr  pScrPriv;

    /*
     * the DIX allocates the space for the window's dev private
     * so don't Xalloc anything , just initialize it.
     */
    TRACE(ErrorF("cmapCreateWindow()\n"));

    /*
     * if this is the root window then we need to hang the 
     * SERVER_OVERLAY_VISUALS property on it. The data associated with 
     * the property contains the visual ID, pixel type (transparent),
     * and the transparent pixel value (0).
     *
     * We cannot do this in cmapScreenInit (or cmapInitColor) because the
     * root window does not exist at that time. 
     */
    pScreen = pWin->drawable.pScreen;
    pScrPriv = CMAPScrDevPriv(pScreen);

    if (pWin->parent == NULL)		/* is this the root window (no parent) */
    {
	/*
	 * create and hang the overlay visual property on the root window
	 */

	if ( (i = (* pScrPriv->OverlayVisualIndex)(pScreen)) == -1)
	{
	    FatalError("cmapCreateWindow: did not find overlay visual\n");
	}
	
        /*
	 * This is how the property data is structured and interpreted
	 *
	 *     SERVER_OVERLAY_VISUALS {
	 *         overlay_visual: VISUALID
	 *         transparent_type: {None, TransparentPixel, TransparentMask}
	 *         value: CARD32
	 *         layer: CARD32
	 *     }
	 */

	propertyData[0] = pScreen->visuals[i].vid; /*  Visual ID          */
	propertyData[1] = 1;		/*  TransparentPixel   */
	propertyData[2] = 0;		/*  trans. pixel is 0  */
	propertyData[3] = CMAP_LAYER_OVERLAY; /*  layer is 1         */
	
	/*
	 * get an atom for this property. This atom is new and we want
	 * it to be created (so we specify TRUE for the last param).
	 */
         
	property = MakeAtom(OVERLAY_PROPERTY_STRING, 
			    strlen(OVERLAY_PROPERTY_STRING),
			    TRUE);

	ChangeWindowProperty(pWin,
			     property,
			     property,
			     32,
			     PropModeReplace,
			     4,
			     propertyData,
			     FALSE);
    }
    
    pWinPriv = CMAPWinDevPriv(pWin);
    
    pWinPriv->flink = NULL;
    pWinPriv->blink = NULL;
    pWinPriv->pWin  = pWin;
    pWinPriv->numKidsInOtherLayer = 0;
    pWinPriv->windowMoved = FALSE;
    pWinPriv->windowResized = FALSE;

    pWinWithOpt = pWin;
    while(pWinWithOpt->optional == NULL)
    {
	pWinWithOpt = pWinWithOpt->parent;
    }
	
    pMap = (ColormapPtr) LookupIDByType(pWinWithOpt->optional->colormap, 
					RT_COLORMAP);
    pWinPriv->pMap = pMap;

    /*
    ** if the window has a colormap then insert it into its colormap's
    ** queue...if it doesn't have a colormap now then it will eventually
    ** get one when pvxChangeWindowAttributes is called and the window
    ** will be inserted into the queue then.
    */

    if (pMap && (pWin->drawable.class != InputOnly))
    {
        cmapPrivPtr pMapPriv = CMAPDevPriv(pMap);

        pWinPriv->flink = pMapPriv->windowQFlink;
        pWinPriv->blink = (cmapWinPrivPtr) pMapPriv;

        pMapPriv->windowQFlink->blink = pWinPriv;
        pMapPriv->windowQFlink = pWinPriv;
    }

    pWinPriv->layer = CMAP_LAYER_NORMAL;
    if ((* pScrPriv->IsOverlayWindow)(pWin))
    {
	pWinPriv->layer = CMAP_LAYER_OVERLAY;
    }
    else if ((pWin->drawable.depth == 0) && 
	     (pWin->drawable.class == InputOnly))
    {
	/*
	 * for InputOnly windows, we'll just init the layer field
	 * so that cmapValidateTree will not think that this window
	 * is in a different layer than its parent. If we'd left layer
	 * un-initialized, then it'll be unpredictable.
	 */

	pWinPriv->layer = CMAPWinDevPriv(pWin->parent)->layer;
    }
    
    return (Bool) (* pScrPriv->CreateWindow)(pWin);
}


/*ROUTINE:cmapDestroyWindow
**++
**      p v x D e s t r o y W i n d o w 
**
** FUNCTIONAL DESCRIPTION:
**
**      This routine is called by DIX when a window is about to be destroyed.
**      It performs any cleanup work that's necessary.
**
** FORMAL PARAMETERS:
**
**      pWin    - pointer to the window that is about to be destroyed
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
**      Always returns TRUE.
**
** SIDE EFFECTS:     
**
**      Removes the window from its colormap's window queue 
**      
**
**--
**/
Bool cmapDestroyWindow(WindowPtr pWin)
{
    cmapWinPrivPtr   pWinPriv;
    
    TRACE(ErrorF("cmapDestroyWindow()\n"));

    /*
     * The DIX frees the space for the window's dev private
     * so don't Xfree anything, just take the window out of the 
     * colormap's window queue (if its in one)
     */

    pWinPriv = CMAPWinDevPriv(pWin);
    
    if ((pWinPriv->flink != NULL) && (pWinPriv->pMap))
    {
        pWinPriv->blink->flink = pWinPriv->flink;
        pWinPriv->flink->blink = pWinPriv->blink;
    }
      
    /*
     * the DIX will take care of freeing the SERVER_OVERLAY_VISUALS
     * property (it calls DeleteAllWindowProperties). So there's no
     * need for the DDX to anything in that regard.
     */

    return (Bool) (* CMAPScrDevPriv(pWin->drawable.pScreen)->DestroyWindow)(pWin);
}


/*ROUTINE:cmapMapWindow
**++
**     p v x R e a l i z e W i n d o w 
**
** FUNCTIONAL DESCRIPTION:
**
**      This routine is called by DIX whenever a window is mapped.
**      It then checks if the window is in a different layer that its parent
**      and increments the numKidsInOtherLayer member in the parent's dev priv.
**      This check could have been done in cmapCreateWindow, but it is better
**      to do it here, because windows that exist but are unmapped do NOT
**      really matter to  window tree validation or to cmapCopyWindow. I.E.
**      its okay to be weird if we can't see you.
**
** FORMAL PARAMETERS:
**
**      pWindow         - pointer to the window that is being mapped
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
**      None.
**
** SIDE EFFECTS:     
**
**      May increment the numKidsInOtherLayer member in the parent window's 
**      dev priv.
**
**--
**/
Bool cmapRealizeWindow(WindowPtr pWin)
{

    cmapWinPrivPtr   pWinPriv, pParentPriv;
    
    TRACE(ErrorF("cmapRealizeWindow()\n"));

    if (pWin->parent)		/* don't bother if this is the root window */
    {    
	pWinPriv = CMAPWinDevPriv(pWin);
        pParentPriv = CMAPWinDevPriv(pWin->parent);

	if (InDifferentLayers(pWinPriv, pParentPriv))
	{
	    pParentPriv->numKidsInOtherLayer++;
	    pParentPriv->numKidsInOtherLayer &= ~UnmappedKidsInOtherLayer;
	}
    }
	
    return (Bool) (* CMAPScrDevPriv(pWin->drawable.pScreen)->RealizeWindow)(pWin);
}


/*ROUTINE:cmapUnmapWindow
**++
**     p v x U n r e a l i z e W i n d o w 
**
** FUNCTIONAL DESCRIPTION:
**
**      This routine is called by DIX whenever a window is unmapped.
**      If the window is in a different layer than its parent then
**      it decrements numKidsInOtherLayer in the parent's dev priv.
**      And, if this is an overlay window it clears overlay planes 
**      in borderClip region.
**
** FORMAL PARAMETERS:
**
**      pWin         - pointer to the window that is being mapped
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
**      None.
**
** SIDE EFFECTS:     
**
**      May decrement the numKidsInOtherLayer member in the parent window's 
**      dev priv.
**
**--
**/
Bool cmapUnrealizeWindow(WindowPtr pWin)
{
    cmapWinPrivPtr   pWinPriv, pParentPriv;
    cmapScrPrivPtr   pScrPriv;

    TRACE(ErrorF("cmapUnmapWindow()\n"));

    /*
     * this routine is NOT called by DIX if this is the root window.
     * so pWin->parent can never be NULL in this context. So don't
     * bother checking, just use it.
     */
    pParentPriv = CMAPWinDevPriv(pWin->parent);

    pWinPriv = CMAPWinDevPriv(pWin);
    pScrPriv = CMAPScrDevPriv(pWin->drawable.pScreen);

    if (InDifferentLayers(pWinPriv, pParentPriv))
    {
	/*
	 * decrement the count and set bit 31 if the count reaches 0.
	 * If bit 31 is set, this is used as a flag to cmapValidateTree
	 * to help it distinguish between:
	 *
	 * 1) Windows that never had any children in another layer.
	 *    Or windows that had kids in another layer but those kids
	 *    were unmapped before now.
	 *
	 *
	 * 2) Windows that have just had their last kid in another layer
	 *    unmapped.
	 *
	 *
	 * For case 1) we want to call the QuickValidateTree routine.
	 * For case 2) we want to use  cmapValidateTree.
	 *
	 * cmapValidateTree will take care of clearing bit 31.
	 *
	 */
	if (pParentPriv->numKidsInOtherLayer > 0)
	{
            --pParentPriv->numKidsInOtherLayer;
        }

        if (pParentPriv->numKidsInOtherLayer == 0)
        {
            pParentPriv->numKidsInOtherLayer |= UnmappedKidsInOtherLayer;
        }
    }
    
    /*
     * if this is an overlay window then we need to clear the
     * the overlay pixels so that the normal layer will be "exposed". 
     * This is because the window(s) in the normal layer that are underneath
     * this window (in the stacking order) will NOT get Expose events 
     *(which is one of the reasons we use overlays in the first place).
     *
     * This operation is delayed until we are in cmapValidateTree. If we
     * cleared the window now, then the backing store routines will be saving
     * the cleared window and not the original contents.
     */
    /*
    ** we needed to make this a linked list due to there being recursive ones
    */

    if ((pWinPriv->layer == CMAP_LAYER_OVERLAY))
    {
        cmapClearUnmapWinPtr newpClearUnmapWin = (cmapClearUnmapWinPtr) Xalloc(sizeof(cmapClearUnmapWin));
        ScreenPtr pScreen = pWin->drawable.pScreen;

        newpClearUnmapWin->flink = CMAPScrDevPriv(pScreen)->pClearUnmapWinList;
        if (newpClearUnmapWin->flink)
        {
            CMAPScrDevPriv(pScreen)->pClearUnmapWinList->blink = newpClearUnmapWin;
        }
        newpClearUnmapWin->blink = NULL;
        newpClearUnmapWin->pClearUnmapWin = pWin;
        newpClearUnmapWin->pClearUnmapWinReg = (* pScreen->RegionCreate)(NullBox,0);
        (* pScreen->RegionCopy)(newpClearUnmapWin->pClearUnmapWinReg,
                                &pWin->borderClip);
        CMAPScrDevPriv(pScreen)->pClearUnmapWinList = newpClearUnmapWin;
    }

    return (Bool) (* pScrPriv->UnrealizeWindow)(pWin);
}


/*ROUTINE:cmapCopyWindow
**++
**	p v x C o p y W i n d o w
**
** FUNCTIONAL DESCRIPTION:
**
**      cmapCopyWindow is called by DIX when a window is moved. This routine
**      is the one that actually moves (copies) the window to its new
**      location on the screen taking into consideration the window
**      hierarchy (i.e. window clipping and regions...)
**
** FORMAL PARAMETERS:
**
**      pWin            - pointer to the window
**	oldOrigin       - old location of the upper left corner
**	pSrcRegion      - old region it is coming from
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
**      None.
**
** SIDE EFFECTS:     
**
**      pOlRegion is modified.
**
**--
**/
void cmapCopyWindow(WindowPtr pWin, DDXPointRec oldOrigin, RegionPtr pOldRegion)
{

    ScreenPtr           pScreen;
    DDXPointPtr         pPoint;
    RegionPtr           pDstRegion;
    RegionPtr           pDiffRegion;
    register BoxPtr     pBox;
    register long       dx, dy;
    register long       i, nBox;
    WindowPtr           pRootWin;
    cmapWinPrivPtr      pWinPriv;
    Bool                copyNormalLayer = TRUE;
    unsigned int        *ordering;
    long                ycrd,j,yMin,yMax,xMin,xMax;
    BoxRec              *pCurBox;
    WindowPtr           pAnc;		/* pointer to ancestor window */
    

    TRACE(ErrorF("cmapCopyWindow: old = (%d,%d) new = (%d,%d) \n",
		 oldOrigin.x, oldOrigin.y, pWin->drawable.x, pWin->drawable.y));

    pScreen = pWin->drawable.pScreen;
    
    pWinPriv = CMAPWinDevPriv(pWin);
    
    /*
     * if this window is in the overlay layer and its parent is in the
     * normal layer then we need to subtract the window's new borderClip 
     * region from its old. The result is stored in the pDiffRegion.
     * We then clear the overlays in this region after we've moved the window.
     */

    if (InNormalInOverlay(CMAPWinDevPriv(pWin->parent), pWinPriv))
    {
        pDiffRegion = (* pScreen->RegionCreate)(NULL, 1);
        (* pScreen->Subtract)(pDiffRegion, pOldRegion, &pWin->borderClip);

        if (pWinPriv->numKidsInOtherLayer == 0)
        {
            copyNormalLayer = FALSE;
        }

        /*
	 * set this flag so that if this window is moved/resize again then
	 * we have to send an expose event to underlying parent region.
	 * This is cleared in cmapValidaTree. (see cmapvaltree.c for more info)
	 * 
	 */

        pWinPriv->windowMoved = TRUE;
    }
    else 
    {
        pDiffRegion = NULL;
    }

    /*
     * if the window being moved is in the overlay layer, then we need
     * set the windowMoved flag in all its ancestor overlay windows (if any)
     * up to (but not including) a normal ancestor. This insures that we
     * send an expose event to the normal ancestor when its overlay child
     * is unmapped.
     */

    if (pWinPriv->layer == CMAP_LAYER_OVERLAY)
    {
	for (pAnc=pWin->parent;
	     (pAnc && (CMAPWinDevPriv(pAnc)->layer == CMAP_LAYER_OVERLAY));
	     pAnc = pAnc->parent)
	{
	    CMAPWinDevPriv(pAnc)->windowMoved = TRUE;
	}
    }

    (* CMAPScrDevPriv(pScreen)->CopyWindow)(pWin, oldOrigin, pOldRegion);

    if (pDiffRegion)
    {
        (* CMAPScrDevPriv(pScreen)->ClearOverlays)(pRootWin, pDiffRegion);
        (* pScreen->RegionDestroy)(pDiffRegion);
    }
}


/*ROUTINE:cmapChangeWindowAttributes
**++
**      p v x C h a n g e W i n d o w A t t r i b u t e s 
**
** FUNCTIONAL DESCRIPTION:
**
**      The DIX layer calls this routine when the attributes of a window
**      have changed. This routineupdates and device-dependent resources
**      for the window that depend on the window's attributes. 
**
** FORMAL PARAMETERS:
**
**      pWin            - pointer to the window whose attributes have changed.
**	mask            - a mask the specifies which attributes have changed.
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
**      return TRUE on success (always)
**
** SIDE EFFECTS:     
**
**      May modify the new and old colormaps' window queues.
**
**--
**/
Bool cmapChangeWindowAttributes(WindowPtr pWin, unsigned long mask)
{

    RegionPtr        pClipRegion;
    cmapWinPrivPtr   pWinPriv;
    ColormapPtr      pMap;
    cmapPrivPtr	     pMapPriv;
    WindowPtr        pWinWithOpt;
    

    TRACE(ErrorF("cmapChangeWindowAttributes()\n"));

    /*
     * Check if the colormap changed
     */

    if (mask & CWColormap)
    {
	mask &= ~CWColormap;

	/*
	 * remove the window from its current colormap's
	 * window queue.  Do this only if the current colormap is still
         * around; if it has been deleted (i.e. pWinPriv->pMap is NULL)
         * then don't do the operation.
	 */

	pWinPriv = CMAPWinDevPriv(pWin);

	if((pWinPriv->flink != NULL) && (pWinPriv->pMap))
	{
	    pWinPriv->blink->flink = pWinPriv->flink;
	    pWinPriv->flink->blink = pWinPriv->blink;
	}
	
	/*
	 * queue the window on its new colormap's window queue.
	 */

	pWinWithOpt = pWin;
	
	while(pWinWithOpt->optional == NULL)
	{
	    pWinWithOpt = pWinWithOpt->parent;
	}

	pMap = (ColormapPtr) LookupIDByType(pWinWithOpt->optional->colormap, 
					    RT_COLORMAP);
	pMapPriv = CMAPDevPriv(pMap);

	pWinPriv->flink = pMapPriv->windowQFlink;
	pWinPriv->blink = (cmapWinPrivPtr) pMapPriv;
	
	pMapPriv->windowQFlink->blink = pWinPriv;
	pMapPriv->windowQFlink = pWinPriv;

	pWinPriv->pMap = pMap;
	
	/*
	 * if the colormap is installed then update the window type
	 * in the exposed region of the window.
	 */

	if (pMapPriv->tag != CMAP_NOT_INSTALLED)
	{
	    pClipRegion = &(pWin->clipList);
	    
	    if (!REGION_NIL(pClipRegion))
	    {
		(* CMAPScrDevPriv(pWin->drawable.pScreen)->FillWindowType)
		    (pWin, pClipRegion, FALSE);
	    }
	}
    }

    if (mask) {
	return (Bool)
	    (* CMAPScrDevPriv(pWin->drawable.pScreen)->ChangeWindowAttributes)
		(pWin, mask);
    }

    return (TRUE);
}



/*ROUTINE:cmapClearToBackground
**++
**     c m a p C l e a r T o B a c k g r o u n d  
**
** FUNCTIONAL DESCRIPTION:
**
**
** FORMAL PARAMETERS:
**
**      pWindow         - pointer to the window that is being mapped
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
**      None.
**
** SIDE EFFECTS:     
**
**
**--
*/
void cmapClearToBackground(WindowPtr pWin, short x, short y, unsigned short w, unsigned short h, Bool generateExposures)
{
    BoxRec           box;
    RegionRec	     reg;
    RegionPtr        pBSReg = NullRegion;
    ScreenPtr	     pScreen;
    BoxPtr           extents;
    int	             x1, y1, x2, y2;

    /* compute everything using ints to avoid overflow */

    x1 = pWin->drawable.x + x;
    y1 = pWin->drawable.y + y;
    if (w)
        x2 = x1 + (int) w;
    else
        x2 = x1 + (int) pWin->drawable.width - (int) x;
    if (h)
        y2 = y1 + h;	
    else
        y2 = y1 + (int) pWin->drawable.height - (int) y;

    extents = &pWin->clipList.extents;
    
    /* clip the resulting rectangle to the window clipList extents.  This
     * makes sure that the result will fit in a box, given that the
     * screen is < 32768 on a side.
     */

    if (x1 < extents->x1)
	x1 = extents->x1;
    if (x2 > extents->x2)
	x2 = extents->x2;
    if (y1 < extents->y1)
	y1 = extents->y1;
    if (y2 > extents->y2)
	y2 = extents->y2;

    if (x2 <= x1 || y2 <= y1)
    {
	x2 = x1 = 0;
	y2 = y1 = 0;
    }

    box.x1 = x1;
    box.x2 = x2;
    box.y1 = y1;
    box.y2 = y2;

    pScreen = pWin->drawable.pScreen;
    (*pScreen->RegionInit) (&reg, &box, 1);

    if (pWin->backStorage)
    {
	/*
	 * If the window has backing-store on, call through the
	 * ClearToBackground vector to handle the special semantics
	 * (i.e. things backing store is to be cleared out and
	 * an Expose event is to be generated for those areas in backing
	 * store if generateExposures is TRUE).
	 */
	pBSReg = (* pScreen->ClearBackingStore)(pWin, x, y, w, h,
						generateExposures);
    }

    (* pScreen->Intersect)(&reg, &reg, &pWin->clipList);

    if (generateExposures)
    {
	(*pScreen->WindowExposures)(pWin, &reg, pBSReg);
    }
    else if (pWin->backgroundState != None)
    {
	if (pWin->backgroundState == ParentRelative)
	{
	    do
	    {
		pWin = pWin->parent;
	    }
	    while (pWin->backgroundState == ParentRelative);
	}

	(* CMAPScrDevPriv(pScreen)->ClearToBackground)(pWin, &reg);
    }

    (* pScreen->RegionUninit)(&reg);

    if (pBSReg)
    {
	(* pScreen->RegionDestroy)(pBSReg);
    }
}

/*
 * HISTORY
 */
