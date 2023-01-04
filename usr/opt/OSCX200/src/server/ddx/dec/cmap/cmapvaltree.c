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
**	p v x v a l t r e e 
**
**  FACILITY:
**
**      ddxcmap -- DECwindows Device Specific Code for PixelVision
**             
**  ABSTRACT:
**
**	This module contains the routines for window tree validation.
**      (i.e. calculate clip list and exposed regions for marked windows).
**
**	The main routine is cmapValidateTree (pScreen->ValidateTree).
**      The code is based on mivaltree.c (Revision 1.2.2.2  92/05/01).
**      Changes were made to window tree validation in the areas of computing
**      clip lists and exposed regions to support windows in the overlay layer.
**      
**      For background on Window Tree Validation see the book 
**      "The X Window System Server" by Israel & Fortune. For background on
**      on overlays see the paper "How Not to Implement Overlays in X" by
**      Todd Newman (published in the X Resource Issue #1, Winter 1992).
**      
**
**      FRIENDLY WARNING: It is very easy to fix something in here and
**                        break something else. Even the simplest of changes
**                        should be heavily tested.
**
**  AUTHOR:
**
**      Khaled Tabbara (code is based heavily on mivaltree.c)
**      
*/

/*

General notes on how things are supposed to work:

-- There are two layers: normal layer and overlay layer.  

-- Windows in the same layer clip and interfere with each other as usual.

-- Child Windows in different layers than their parent do NOT clip 
   (rendering wise) the parent.

-- Sibling windows clip other siblings regardless of layer.

-- Child windows do not clip parent window if the child and parent
   are in different layers.

-- Windows in the normal layer interfere with windows in the overlay
   layer.  This means that a change to a normal window may cause Expose
   events to the overlay windows that are beneath it in the stacking
   order. And, ideally,  a change to an overlay window does NOT generate an
   expose event to the normal window beneath. But there are cases where we need
   to send an expose event to repair the normal parent eventhough the child 
   that changed was an overlay window:

   cmapCopyWindow (called when a window is to be moved) moves *all* the
   layers and not just the layer that the window happens to be in. It is
   expensive to figure out what layers the window and all its descendants are
   in. Moving all the planes and control bits (RGB and DCC) ensures that if 
   window and/or its descendants use private colormaps then they will display 
   correctly in their new location). This also ensures that if the window
   to be moved is normal and it has overlay children then those children are
   moved too ! So the simplest and most straightforward thing to do is to
   move both the RGB and DCC (which covers the normal planes, overlay planes,
   and window tag bits).


   So whenever an overlay window (with a normal parent) is moved then
   we mark it (by setting windowMoved in its dev priv to TRUE). This
   means that *next* time this window changes  (moved, unmapped, or resized) 
   we should send an expose event so the client will repair the region in 
   the parent that is no longer occupied by the overlay child. windowMoved
   is cleared after we've set the valdata->after.exposed region in the parent.

   When an overlay window is moved or unmapped then clearing the overlays
   is taken care of by the cmapCopyWindow and cmapUnmapWindow routines,
   respectively. There's no specific DDX routine for resize operations, so in
   that case we'll need to clear the overlays in cmapValidateTree.

   If the child and parent are in the same layer or if the child is in the
   normal layer and its parent is in the overlay layer then it is business
   as usual as far as exposures.

*/ 
#include	"cmap.h"

extern int    miShapedWindowIn ();

extern void miTreeObscured(WindowPtr pParent);

/*
** function prototypes for functions in this module
*/

void cmapQuickComputeClips (WindowPtr pParent, ScreenPtr pScreen, 
			    RegionPtr universe, VTKind kind, 
			    RegionPtr exposed);

long cmapQuickValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind);

void cmapComputeClips (WindowPtr pParent, ScreenPtr pScreen, 
		       RegionPtr universe,VTKind kind,RegionPtr exposed);

/* borrowed from mivaltree.c */
#define HasParentRelativeBorder(w) (!(w)->borderIsPixel && \
				    HasBorder(w) && \
				    (w)->backgroundState == ParentRelative)
static void
miTreeObscured(pParent)
    register WindowPtr pParent;
{
    register WindowPtr pChild;
    register int    oldVis;

    pChild = pParent;
    while (1)
    {
	if (pChild->viewable)
	{
	    oldVis = pChild->visibility;
	    if (oldVis != (pChild->visibility = VisibilityFullyObscured) &&
		((pChild->eventMask | wOtherEventMasks(pChild)) & VisibilityChangeMask))
		SendVisibilityNotify(pChild);
	    if (pChild->firstChild)
	    {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pParent))
	    pChild = pChild->parent;
	if (pChild == pParent)
	    break;
	pChild = pChild->nextSib;
    }
}


/*ROUTINE:cmapQuickComputeClips
**++
**	c m a p Q u i c k C o m p u t e C l i p s
**
** FUNCTIONAL DESCRIPTION:
**
**     Recompute the clipList, borderClip, exposed and borderExposed
**     regions for pParent and its children. Only viewable windows are
**     taken into account. This routine is recursive; it calls itself to
**     to handle the window's descendants.
**
** FORMAL PARAMETERS:
**
** 
**     pParent   - pointer to the window for which we need to compute 
**                 clipping regions.
**
**     pScreen   - pointer to the screen.
**
**     universe  - the area of the screen that may be used for the window
**                 and its inferiors.
**
**     kind      - specifies the reason for the validation.
**
**     exposed   - pointer to a region that may be used for intermediate
**                 calculations
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
**      clipList, borderClip, exposed and borderExposed are altered.
**      A VisibilityNotify event may be generated on the parent window.
**
**--
*/
void cmapQuickComputeClips (WindowPtr pParent, ScreenPtr pScreen, 
			    RegionPtr universe, VTKind kind, 
			    RegionPtr exposed)
{
    int			dx, dy;
    RegionRec		childUniverse;
    register WindowPtr	pChild;
    int     	  	oldVis, newVis;
    BoxRec		borderSize;
    RegionRec		childUnion;
    Bool		overlap;
    RegionPtr		borderVisible;
    Bool		resized;

    TRACE(ErrorF("cmapQuickComputeClips()\n"));
    /*
     * Figure out the new visibility of this window.
     * The extent of the universe should be the same as the extent of
     * the borderSize region. If the window is unobscured, this rectangle
     * will be completely inside the universe (the universe will cover it
     * completely). If the window is completely obscured, none of the
     * universe will cover the rectangle.
     */

    borderSize.x1 = pParent->drawable.x - wBorderWidth(pParent);
    borderSize.y1 = pParent->drawable.y - wBorderWidth(pParent);
    dx = (int) pParent->drawable.x + (int) pParent->drawable.width + 
	wBorderWidth(pParent);

    if (dx > 32767)
	dx = 32767;

    borderSize.x2 = dx;

    dy = (int) pParent->drawable.y + (int) pParent->drawable.height + 
	wBorderWidth(pParent);

    if (dy > 32767)
	dy = 32767;

    borderSize.y2 = dy;

    oldVis = pParent->visibility;
    switch ((* pScreen->RectIn) (universe, &borderSize)) 
    {
     case rgnIN:
	newVis = VisibilityUnobscured;
	break;
     case rgnPART:
	newVis = VisibilityPartiallyObscured;
#ifdef SHAPE
	{
	    RegionPtr   pBounding;

	    if ((pBounding = wBoundingShape (pParent)))
	    {
		switch (miShapedWindowIn (pScreen, universe, pBounding,
					  &borderSize,
					  pParent->drawable.x,
					  pParent->drawable.y))
		{
		 case rgnIN:
		    newVis = VisibilityUnobscured;
		    break;
		 case rgnOUT:
		    newVis = VisibilityFullyObscured;
		    break;
		}
	    }
	}
#endif
	break;
     default:
	newVis = VisibilityFullyObscured;
	break;
    }
    pParent->visibility = newVis;
    if (oldVis != newVis &&
	((pParent->eventMask|wOtherEventMasks(pParent))&VisibilityChangeMask))
	SendVisibilityNotify(pParent);

    dx = pParent->drawable.x - pParent->valdata->before.oldAbsCorner.x;
    dy = pParent->drawable.y - pParent->valdata->before.oldAbsCorner.y;

    /*
     * avoid computations when dealing with simple operations
     */

    switch (kind) {
     case VTMap:
     case VTStack:
     case VTUnmap:
	break;
     case VTMove:
	if ((oldVis == newVis) &&
	    ((oldVis == VisibilityFullyObscured) ||
	     (oldVis == VisibilityUnobscured)))
	{
	    pChild = pParent;
	    while (1)
	    {
		if (pChild->viewable)
		{
		    if (pChild->visibility != VisibilityFullyObscured)
		    {
			(* pScreen->TranslateRegion) (&pChild->borderClip,
						      dx, dy);
			(* pScreen->TranslateRegion) (&pChild->clipList,
						      dx, dy);
			pChild->drawable.serialNumber = NEXT_SERIAL_NUMBER;
			if (pScreen->ClipNotify)
			    (* pScreen->ClipNotify) (pChild, dx, dy);

		    }
		    if (pChild->valdata)
		    {
			(* pScreen->RegionInit) 
			    (&pChild->valdata->after.borderExposed,
			     NullBox, 
			     0);

			if (HasParentRelativeBorder(pChild))
			{
			    (* pScreen->Subtract)
				(&pChild->valdata->after.borderExposed,
				 &pChild->borderClip, 
				 &pChild->winSize);
			}
			(* pScreen->RegionInit) 
			    (&pChild->valdata->after.exposed,
			     NullBox, 
			     0);
		    }

		    if (pChild->firstChild)
		    {
			pChild = pChild->firstChild;
			continue;
		    }
		}
		while (!pChild->nextSib && (pChild != pParent))
		    pChild = pChild->parent;

		if (pChild == pParent)
		    break;

		pChild = pChild->nextSib;
	    }
	    return;
	}
	/* fall through */
     default:
    	/*
	 * To calculate exposures correctly, we have to translate the old
	 * borderClip and clipList regions to the window's new location so 
	 * there is a correspondence between pieces of the new and old 
	 * clipping regions.
	 */

    	if (dx || dy) 
    	{
	    /*
	     * We translate the old clipList because that will be exposed 
	     * or copied if gravity is right.
	     */

	    (* pScreen->TranslateRegion) (&pParent->borderClip, dx, dy);
	    (* pScreen->TranslateRegion) (&pParent->clipList, dx, dy);
    	} 
	break;
    }

    borderVisible = pParent->valdata->before.borderVisible;
    resized = pParent->valdata->before.resized;

    (* pScreen->RegionInit) (&pParent->valdata->after.borderExposed, 
			     NullBox, 
			     0);

    (* pScreen->RegionInit) (&pParent->valdata->after.exposed, NullBox, 0);

    /*
     * Since the borderClip must not be clipped by the children, we do
     * the border exposure first...
     *
     * 'universe' is the window's borderClip. To figure the exposures, remove
     * the area that used to be exposed from the new.
     * This leaves a region of pieces that weren't exposed before.
     */

    if (HasBorder (pParent))
    {
    	if (borderVisible)
    	{
	    /*
	     * when the border changes shape, the old visible portions
	     * of the border will be saved by DIX in borderVisible --
	     * use that region and destroy it
	     */
	    (* pScreen->Subtract) (exposed, universe, borderVisible);
	    (* pScreen->RegionDestroy) (borderVisible);
    	}
    	else
    	{
	    (* pScreen->Subtract) (exposed, universe, &pParent->borderClip);
    	}

	if (HasParentRelativeBorder(pParent) && (dx || dy))
	{
	    (* pScreen->Subtract)
		(&pParent->valdata->after.borderExposed,
		 universe,
		 &pParent->winSize);
	}
	else
	{
	    (* pScreen->Subtract) (&pParent->valdata->after.borderExposed,
				   exposed, &pParent->winSize);
	}
	

    	(* pScreen->RegionCopy) (&pParent->borderClip, universe);
    
    	/*
	 * To get the right clipList for the parent, and to make doubly sure
	 * that no child overlaps the parent's border, we remove the parent's
	 * border from the universe before proceeding.
	 */
    
    	(* pScreen->Intersect) (universe, universe, &pParent->winSize);
    }
    else
    {
    	(* pScreen->RegionCopy) (&pParent->borderClip, universe);
    }
    
    if ((pChild = pParent->firstChild) && pParent->mapped)
    {
	(*pScreen->RegionInit) (&childUniverse, NullBox, 0);
	(*pScreen->RegionInit) (&childUnion, NullBox, 0);

	if ((pChild->drawable.y < pParent->lastChild->drawable.y) ||
	    ((pChild->drawable.y == pParent->lastChild->drawable.y) &&
	     (pChild->drawable.x < pParent->lastChild->drawable.x)))
	{
	    for (; pChild; pChild = pChild->nextSib)
	    {
		if (pChild->viewable)
		    (* pScreen->RegionAppend)(&childUnion,&pChild->borderSize);
	    }
	}
	else
	{
	    for (pChild = pParent->lastChild; pChild; pChild = pChild->prevSib)
	    {
		if (pChild->viewable)
		    (* pScreen->RegionAppend)(&childUnion,&pChild->borderSize);
	    }
	}
	(* pScreen->RegionValidate)(&childUnion, &overlap);

	for (pChild = pParent->firstChild;
	     pChild;
	     pChild = pChild->nextSib)
 	{
	    if (pChild->viewable) {
		/*
		 * If the child is viewable, we want to remove its extents
		 * from the current universe, but we only re-clip it if
		 * it's been marked.
		 */

		if (pChild->valdata) {
		    /*
		     * Figure out the new universe from the child's
		     * perspective and recurse.
		     */

		    (* pScreen->Intersect) (&childUniverse,
					    universe,
					    &pChild->borderSize);

		    cmapComputeClips (pChild, pScreen, &childUniverse, kind,
				     exposed);
		}
		/*
		 * Once the child has been processed, we remove its extents
		 * from the current universe, thus denying its space to any
		 * other sibling.
		 */

		if (overlap)
		{
		    (* pScreen->Subtract) 
			(universe, universe, &pChild->borderSize);
		}
	    }
	}
	if (!overlap)
	{
	    (* pScreen->Subtract) (universe, universe, &childUnion);
	}
	(* pScreen->RegionUninit) (&childUnion);
	(* pScreen->RegionUninit) (&childUniverse);
    }					/* if any children */

    /*
     * 'universe' now contains the new clipList for the parent window.
     *
     * To figure the exposure of the window we subtract the old clip from the
     * new, just as for the border.
     */

    if (oldVis == VisibilityFullyObscured ||
	oldVis == VisibilityNotViewable)
    {
	(* pScreen->RegionCopy) (&pParent->valdata->after.exposed,
				 universe);
    }
    else if (newVis != VisibilityFullyObscured &&
	     newVis != VisibilityNotViewable)
    {
    	(* pScreen->Subtract) (&pParent->valdata->after.exposed,
			       universe, &pParent->clipList);
    }

    /*
     * One last thing: backing storage. We have to try to save what parts of
     * the window are about to be obscured. We can just subtract the universe
     * from the old clipList and get the areas that were in the old but aren't
     * in the new and, hence, are about to be obscured.
     */

    if (pParent->backStorage && !resized)
    {
	(* pScreen->Subtract) (exposed, &pParent->clipList, universe);
	(* pScreen->SaveDoomedAreas)(pParent, exposed, dx, dy);
    }
    
    /* HACK ALERT - copying contents of regions, instead of regions */
    {
	RegionRec   tmp;

	tmp = pParent->clipList;
	pParent->clipList = *universe;
	*universe = tmp;
    }

#ifdef NOTDEF
    (* pScreen->RegionCopy) (&pParent->clipList, universe);
#endif

    pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    if (pScreen->ClipNotify)
	(* pScreen->ClipNotify) (pParent, dx, dy);
}


/*ROUTINE:QuickValidateTree
**++
**      Q u i c k V a l i d a t e T r e e 
**
** FUNCTIONAL DESCRIPTION:
**
**
**     
**     Notes:
**
**     This routine assumes that all affected windows have been marked
**     (valdata created) and their winSize and borderSize regions
**     adjusted to correspond to their new positions. The borderClip and
**     clipList regions should not have been touched.
**
**     The top-most level is treated differently from all lower levels
**     because pParent is unchanged. For the top level, we merge the
**     regions taken up by the marked children back into the clipList
**     for pParent, thus forming a region from which the marked children
**     can claim their areas. For lower levels, where the old clipList
**     and borderClip are invalid, we can't do this and have to do the
**     extra operations done in cmapComputeClips, but this is much faster
**     e.g. when only one child has moved...
**
**
** FORMAL PARAMETERS:
**
**      pParent         -  Parent window of the window subtree to validate
**	pChild          -  First child of pParent that was affected;
**                         this is the changed window itself.
**	kind            -  what kind of configuration caused call
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
**      Always returns 1.
**
** SIDE EFFECTS:     
**
**      The cliplist, borderClip, exposed, and borderExposed regions for
**      each marked window are altered.
**
**--
*/
long cmapQuickValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind)
{
    RegionRec	  	totalClip;	/* Total clipping region available to
					 * the marked children. pParent's clipList
					 * merged with the borderClips of all
					 * the marked children. */
    RegionRec	  	childClip;	/* The new borderClip for the current
					 * child */
    RegionRec		childUnion;	/* the space covered by borderSize for
					 * all marked children */
    RegionRec		exposed;	/* For intermediate calculations */
    register ScreenPtr	pScreen;
    register WindowPtr	pWin;
    Bool		overlap;
    int			viewvals;
    Bool		forward;

    TRACE(ErrorF("cmapQuickValidateTree()\n"));

    pScreen = pParent->drawable.pScreen;
    if (pChild == NullWindow)
	pChild = pParent->firstChild;


    (*pScreen->RegionInit) (&childClip, NullBox, 0);
    (*pScreen->RegionInit) (&exposed, NullBox, 0);

    /*
     * compute the area of the parent window occupied
     * by the marked children + the parent itself.  This
     * is the area which can be divied up among the marked
     * children in their new configuration.
     */

    (*pScreen->RegionInit) (&totalClip, NullBox, 0);
    viewvals = 0;

    if ((pChild->drawable.y < pParent->lastChild->drawable.y) ||
	((pChild->drawable.y == pParent->lastChild->drawable.y) &&
	 (pChild->drawable.x < pParent->lastChild->drawable.x)))
    {
	forward = TRUE;
	for (pWin = pChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->valdata)
	    {
		(* pScreen->RegionAppend) (&totalClip, &pWin->borderClip);
		if (pWin->viewable)
		    viewvals++;
	    }
	}
    }
    else
    {
	forward = FALSE;
	pWin = pParent->lastChild;
	while (1)
	{
	    if (pWin->valdata)
	    {
		(* pScreen->RegionAppend) (&totalClip, &pWin->borderClip);
		if (pWin->viewable)
		    viewvals++;
	    }
	    if (pWin == pChild)
		break;
	    pWin = pWin->prevSib;
	}
    }
    (* pScreen->RegionValidate)(&totalClip, &overlap);

    /*
     * Now go through the children of the root and figure their new
     * borderClips from the totalClip, passing that off to xxxComputeClips
     * to handle recursively. Once that's done, we remove the child
     * from the totalClip to clip any siblings below it.
     */

    overlap = TRUE;
    if (kind != VTStack)
    {
	(* pScreen->Union) (&totalClip, &totalClip, &pParent->clipList);
	if (viewvals > 1)
	{
	    /*
	     * precompute childUnion to discover whether any of them
	     * overlap.  This seems redundant, but performance studies
	     * have demonstrated that the cost of this loop is
	     * lower than the cost of multiple Subtracts in the
	     * loop below.
	     */

	    (*pScreen->RegionInit) (&childUnion, NullBox, 0);

	    if (forward)
	    {
		for (pWin = pChild; pWin; pWin = pWin->nextSib)
		    if (pWin->valdata && pWin->viewable)
			(* pScreen->RegionAppend) (&childUnion,
						   &pWin->borderSize);
	    }
	    else
	    {
		pWin = pParent->lastChild;
		while (1)
		{
		    if (pWin->valdata && pWin->viewable)
			(* pScreen->RegionAppend) (&childUnion,
						   &pWin->borderSize);
		    if (pWin == pChild)
			break;
		    pWin = pWin->prevSib;
		}
	    }

	    (*pScreen->RegionValidate)(&childUnion, &overlap);

	    if (overlap)
		(*pScreen->RegionUninit) (&childUnion);
	}
    }

    for (pWin = pChild;
	 pWin != NullWindow;
	 pWin = pWin->nextSib)
    {
	if (pWin->viewable) 
	{
	    if (pWin->valdata) 
	    {
		(* pScreen->Intersect) (&childClip,
					&totalClip,
 					&pWin->borderSize);

		cmapComputeClips (pWin, pScreen, &childClip, kind, &exposed);

		if (overlap)
		{
		    (* pScreen->Subtract) (&totalClip,
				       	   &totalClip,
				       	   &pWin->borderSize);
		}

	    } 
	    else if (pWin->visibility == VisibilityNotViewable) 
	    {
		miTreeObscured(pWin);
	    }
	} 
	else 
	{
	    if (pWin->valdata) 
	    {
		(* pScreen->RegionEmpty)(&pWin->clipList);
		(* pScreen->RegionEmpty)(&pWin->borderClip);
		pWin->valdata = (ValidatePtr)NULL;
	    }
	}
    }

    (* pScreen->RegionUninit) (&childClip);

    if (!overlap)
    {
	(*pScreen->Subtract)(&totalClip, &totalClip, &childUnion);
	(*pScreen->RegionUninit) (&childUnion);
    }

    (* pScreen->RegionInit) (&pParent->valdata->after.exposed, NullBox, 0);
    (* pScreen->RegionInit) (&pParent->valdata->after.borderExposed,NullBox,0);

    /*
     * each case below is responsible for updating the
     * clipList and serial number for the parent window
     */

    switch (kind) 
    {
     case VTStack:
	break;
     default:
	/*
	 * totalClip contains the new clipList for the parent. Figure out
	 * exposures and obscures as per xxxComputeClips and reset the parent's
	 * clipList.
	 */

	(* pScreen->Subtract) (&pParent->valdata->after.exposed,
			       &totalClip, &pParent->clipList);

	/* fall through */

     case VTMap:
	if (pParent->backStorage) 
	{
	    (* pScreen->Subtract) (&exposed, &pParent->clipList, &totalClip);
	    (* pScreen->SaveDoomedAreas)(pParent, &exposed, 0, 0);
	}
	
	(* pScreen->RegionCopy) (&pParent->clipList, &totalClip);

	pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	break;
    }

    (* pScreen->RegionUninit) (&totalClip);
    (* pScreen->RegionUninit) (&exposed);

    if (pScreen->ClipNotify)
	(*pScreen->ClipNotify) (pParent, 0, 0);

    return (1);
}


/*ROUTINE:cmapComputeClips
**++
**	c m a p C o m p u t e C l i p s
**
** FUNCTIONAL DESCRIPTION:
**
**     Recompute the clipList, borderClip, exposed and borderExposed
**     regions for pParent and its children. Only viewable windows are
**     taken into account. This routine is recursive; it calls itself to
**     to handle the window's descendants.
**
** FORMAL PARAMETERS:
**
** 
**     pParent   - pointer to the window for which we need to compute 
**                 clipping regions.
**
**     pScreen   - pointer to the screen.
**
**     universe  - the area of the screen that may be used for the window
**                 and its inferiors.
**
**     kind      - specifies the reason for the validation.
**
**     exposed   - pointer to a region that may be used for intermediate
**                 calculations
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
**      clipList, borderClip, exposed and borderExposed are altered.
**      A VisibilityNotify event may be generated on the parent window.
**
**--
*/
void cmapComputeClips (WindowPtr pParent, ScreenPtr pScreen, RegionPtr universe,
		       VTKind kind, RegionPtr exposed)
{
    int			dx, dy;
    RegionRec		childUniverse;
    register WindowPtr	pChild;
    int     	  	oldVis, newVis;
    BoxRec		borderSize;
    RegionRec		childUnion;
    Bool		overlap;
    RegionPtr		borderVisible;
    Bool		resized;
    RegionPtr           pOverExpose = NULL;
    RegionRec           t1, t2;
    WindowPtr           pSib;
    WindowPtr           pWin;
    cmapWinPrivPtr      pParentPriv;
    
    TRACE(ErrorF("cmapComputeClips()\n"));

    pParentPriv = CMAPWinDevPriv(pParent);

    if (pParentPriv->numKidsInOtherLayer == 0)
    {
	cmapQuickComputeClips(pParent, pScreen, universe, kind, exposed);
	return;
    }
    else
    {
        if (pParent->valdata->before.resized &&
            (pParentPriv->layer == CMAP_LAYER_NORMAL))
        {
            pParentPriv->windowResized = TRUE;
        }
    }
    
    /* 
    ** see pvxUnmapWindow if you want to know what UnmappedKidsInOtherLayer
    ** means.
    */
/*    if (pParentPriv->numKidsInOtherLayer == UnmappedKidsInOtherLayer)
**    {
**	pParentPriv->numKidsInOtherLayer = 0;
**    }
*/

    pParentPriv->numKidsInOtherLayer &= ~UnmappedKidsInOtherLayer;

    /*
     * Figure out the new visibility of this window.
     * The extent of the universe should be the same as the extent of
     * the borderSize region. If the window is unobscured, this rectangle
     * will be completely inside the universe (the universe will cover it
     * completely). If the window is completely obscured, none of the
     * universe will cover the rectangle.
     */

    borderSize.x1 = pParent->drawable.x - wBorderWidth(pParent);
    borderSize.y1 = pParent->drawable.y - wBorderWidth(pParent);
    dx = (int) pParent->drawable.x + (int) pParent->drawable.width + 
	wBorderWidth(pParent);

    if (dx > 32767)
	dx = 32767;

    borderSize.x2 = dx;

    dy = (int) pParent->drawable.y + (int) pParent->drawable.height + 
	wBorderWidth(pParent);

    if (dy > 32767)
	dy = 32767;

    borderSize.y2 = dy;

    oldVis = pParent->visibility;
    switch ((* pScreen->RectIn) (universe, &borderSize)) 
    {
     case rgnIN:
	newVis = VisibilityUnobscured;
	break;
     case rgnPART:
	newVis = VisibilityPartiallyObscured;
#ifdef SHAPE
	{
	    RegionPtr   pBounding;

	    if ((pBounding = wBoundingShape (pParent)))
	    {
		switch (miShapedWindowIn (pScreen, universe, pBounding,
					  &borderSize,
					  pParent->drawable.x,
					  pParent->drawable.y))
		{
		 case rgnIN:
		    newVis = VisibilityUnobscured;
		    break;
		 case rgnOUT:
		    newVis = VisibilityFullyObscured;
		    break;
		}
	    }
	}
#endif
	break;
     default:
	newVis = VisibilityFullyObscured;
	break;
    }
    pParent->visibility = newVis;
    if (oldVis != newVis &&
	((pParent->eventMask|wOtherEventMasks(pParent))&VisibilityChangeMask))
	SendVisibilityNotify(pParent);

    dx = pParent->drawable.x - pParent->valdata->before.oldAbsCorner.x;
    dy = pParent->drawable.y - pParent->valdata->before.oldAbsCorner.y;

    /*
     * avoid computations when dealing with simple operations
     */

    switch (kind) {
     case VTMap:
     case VTStack:
     case VTUnmap:
	break;
     case VTMove:
	if ((oldVis == newVis) &&
	    ((oldVis == VisibilityFullyObscured) ||
	     (oldVis == VisibilityUnobscured)))
	{
	    pChild = pParent;
	    while (1)
	    {
		if (pChild->viewable)
		{
		    if (pChild->visibility != VisibilityFullyObscured)
		    {
			(* pScreen->TranslateRegion) (&pChild->borderClip,
						      dx, dy);
			(* pScreen->TranslateRegion) (&pChild->clipList,
						      dx, dy);
			pChild->drawable.serialNumber = NEXT_SERIAL_NUMBER;
			if (pScreen->ClipNotify)
			    (* pScreen->ClipNotify) (pChild, dx, dy);

		    }
		    if (pChild->valdata)
		    {
			(* pScreen->RegionInit) 
			    (&pChild->valdata->after.borderExposed,
			     NullBox, 
			     0);

			if (HasParentRelativeBorder(pChild))
			{
			    (* pScreen->Subtract)
				(&pChild->valdata->after.borderExposed,
				 &pChild->borderClip, 
				 &pChild->winSize);
			}
			(* pScreen->RegionInit) 
			    (&pChild->valdata->after.exposed,
			     NullBox, 
			     0);
		    }

		    if (pChild->firstChild)
		    {
			pChild = pChild->firstChild;
			continue;
		    }
		}
		while (!pChild->nextSib && (pChild != pParent))
		    pChild = pChild->parent;

		if (pChild == pParent)
		    break;

		pChild = pChild->nextSib;
	    }
	    return;
	}
	/* fall through */
     default:
    	/*
	 * To calculate exposures correctly, we have to translate the old
	 * borderClip and clipList regions to the window's new location so 
	 * there is a correspondence between pieces of the new and old 
	 * clipping regions.
	 */

    	if (dx || dy) 
	{
	    /*
	     * We translate the old clipList because that will be exposed 
	     * or copied if gravity is right.
	     */

	    (* pScreen->TranslateRegion) (&pParent->borderClip, dx, dy);
	    (* pScreen->TranslateRegion) (&pParent->clipList, dx, dy);
	} 
	break;
    }

    /*
     *  this will handle the case for UnmapSubwindows.
     *  if any of the children of this window fit the description
     *  (i.e parent in overlay 
     */

    if (kind == VTUnmap)
    {
	for (pWin = pParent->firstChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->valdata && CMAPWinDevPriv(pWin)->windowMoved)
	    {
		CMAPWinDevPriv(pWin)->windowMoved = 0;
		
		if (pOverExpose)
		{
		    (* pScreen->RegionAppend)(pOverExpose, &pWin->borderClip);
		}
		else
		{
		    pOverExpose = (* pScreen->RegionCreate)(NullBox,0);
		    (* pScreen->RegionCopy)(pOverExpose, &pWin->borderClip);
		}
	    }
	}
    }

    borderVisible = pParent->valdata->before.borderVisible;
    resized = pParent->valdata->before.resized;

    (* pScreen->RegionInit) (&pParent->valdata->after.borderExposed, 
			     NullBox, 
			     0);

    (* pScreen->RegionInit) (&pParent->valdata->after.exposed, NullBox, 0);

    /*
     * Since the borderClip must not be clipped by the children, we do
     * the border exposure first...
     *
     * 'universe' is the window's borderClip. To figure the exposures, remove
     * the area that used to be exposed from the new.
     * This leaves a region of pieces that weren't exposed before.
     */

    if (HasBorder (pParent))
    {
	if (borderVisible)
	{
	    /*
	     * when the border changes shape, the old visible portions
	     * of the border will be saved by DIX in borderVisible --
	     * use that region and destroy it
	     */
	    (* pScreen->Subtract) (exposed, universe, borderVisible);
	    (* pScreen->RegionDestroy) (borderVisible);
	}
	else
	{
	    (* pScreen->Subtract) (exposed, universe, &pParent->borderClip);
	}

	if (HasParentRelativeBorder(pParent) && (dx || dy))
	{
	    (* pScreen->Subtract)
		(&pParent->valdata->after.borderExposed,
		 universe,
		 &pParent->winSize);
	}
	else
	{
	    (* pScreen->Subtract) (&pParent->valdata->after.borderExposed,
				   exposed, &pParent->winSize);
	}
	

	(* pScreen->RegionCopy) (&pParent->borderClip, universe);
    
	/*
	 * To get the right clipList for the parent, and to make doubly sure
	 * that no child overlaps the parent's border, we remove the parent's
	 * border from the universe before proceeding.
	 */
    
	(* pScreen->Intersect) (universe, universe, &pParent->winSize);
    }
    else
    {
	(* pScreen->RegionCopy) (&pParent->borderClip, universe);
    }
    
    
    if ((pChild = pParent->firstChild) && pParent->mapped)
    {
	(*pScreen->RegionInit) (&childUniverse, NullBox, 0);
	(*pScreen->RegionInit) (&childUnion, NullBox, 0);

	if ((pChild->drawable.y < pParent->lastChild->drawable.y) ||
	    ((pChild->drawable.y == pParent->lastChild->drawable.y) &&
	     (pChild->drawable.x < pParent->lastChild->drawable.x)))
	{
	    for (; pChild; pChild = pChild->nextSib)
	    {
		if (pChild->viewable)
		{
		    if (InSameLayer(CMAPWinDevPriv(pParent), CMAPWinDevPriv(pChild)))
			(* pScreen->RegionAppend)(&childUnion, &pChild->borderSize);
	        }
	    }
	}
	else
	{
	    for (pChild = pParent->lastChild; pChild; pChild = pChild->prevSib)
	    {
		if (pChild->viewable)
		{
		    if (InSameLayer(CMAPWinDevPriv(pParent), CMAPWinDevPriv(pChild)))
			(* pScreen->RegionAppend)(&childUnion, &pChild->borderSize);
	        }
	    }
	}
	(* pScreen->RegionValidate)(&childUnion, &overlap);

	for (pChild = pParent->firstChild;
	     pChild;
	     pChild = pChild->nextSib)
	{
	    if (pChild->viewable) {
		/*
		 * If the child is viewable, we want to remove its extents
		 * from the current universe, but we only re-clip it if
		 * it's been marked.
		 */

		if (pChild->valdata) {
		    /*
		     * Figure out the new universe from the child's
		     * perspective and recurse.
		     */

		    (* pScreen->Intersect) (&childUniverse,
					    universe,
					    &pChild->borderSize);

		    cmapComputeClips (pChild, pScreen, &childUniverse, kind,
				      exposed);
		}
		/*
		 * Once the child has been processed, we remove its extents
		 * from the current universe, thus denying its space to any
		 * other sibling.
		 */

		if (overlap)
		{
		    (* pScreen->Subtract) 
			(universe, universe, &pChild->borderSize);
		}
	    }
	}
	if (!overlap)
	{
	    (* pScreen->Subtract) (universe, universe, &childUnion);
	}
	(* pScreen->RegionUninit) (&childUnion);
	(* pScreen->RegionUninit) (&childUniverse);
    }					/* if any children */

    /*
     * 'universe' now contains the new clipList for the parent window.
     *
     * To figure the exposure of the window we subtract the old clip from the
     * new, just as for the border.
     */

    /*
     * we need to add the areas for the children of the parent that are
     * in a different layer. 
     */

    if (oldVis == VisibilityFullyObscured ||
	oldVis == VisibilityNotViewable)
    {
	(* pScreen->RegionCopy) (&pParent->valdata->after.exposed,
				 universe);
    }
    else if (newVis != VisibilityFullyObscured &&
	     newVis != VisibilityNotViewable)
    {
	(* pScreen->Subtract) (&pParent->valdata->after.exposed,
			       universe, &pParent->clipList);
    }

    if (pOverExpose)
    {
	(* pScreen->RegionValidate)(pOverExpose, &overlap);
	
	(* pScreen->Intersect)(pOverExpose, pOverExpose, universe);
	
	(* pScreen->RegionAppend)(&pParent->valdata->after.exposed,
				  pOverExpose);
	
	(* pScreen->RegionDestroy)(pOverExpose);
    }
    
    /*
     * One last thing: backing storage. We have to try to save what parts of
     * the window are about to be obscured. We can just subtract the universe
     * from the old clipList and get the areas that were in the old but aren't
     * in the new and, hence, are about to be obscured.
     */

    if (pParent->backStorage && !resized)
    {
	(* pScreen->Subtract) (exposed, &pParent->clipList, universe);
	(* pScreen->SaveDoomedAreas)(pParent, exposed, dx, dy);
    }
    
    /*
     * Add the borderClip regions to the parent window's clipList if the
     * child is in a different layer than the parent. If there is overlap
     * between the children then we have to be careful no to add those 
     * parts of the child's clipRegion that overlap (on top) a sibling
     * if the sibling is in the same layer as the parent.
     * 
     */
    
    if (kind == VTMap)
    {
	(* pScreen->RegionInit) (&t1, NullBox, 0);
	(* pScreen->RegionInit) (&t2, NullBox, 0);
	
	for (pWin = pParent->firstChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->valdata && InDifferentLayers(CMAPWinDevPriv(pParent), CMAPWinDevPriv(pWin)))
	    {
		(* pScreen->RegionAppend)(universe, &pWin->borderClip);
		
		if (overlap)
		{
		    for (pSib = pWin->nextSib; pSib; pSib = pSib->nextSib)
		    {
			if (InSameLayer(CMAPWinDevPriv(pParent), CMAPWinDevPriv(pSib)))
			{
			    (* pScreen->Intersect)(&t1,
						   &pWin->borderClip,
						   &pSib->borderSize);
			    
			    (* pScreen->RegionAppend)(&t2, &t1);
			}
		    }
		}
	    }
	}

	if (overlap)
	{
	    (* pScreen->RegionValidate)(universe, &overlap);
	    (* pScreen->RegionValidate)(&t2, &overlap);
	    (* pScreen->Subtract)(universe, universe, &t2);
	}
	else
	{
	    (* pScreen->RegionValidate)(universe, &overlap);
	}
	    
	(* pScreen->RegionUninit)(&t1);
	(* pScreen->RegionUninit)(&t2);
    }
			  
    /* HACK ALERT - copying contents of regions, instead of regions */
    {
	RegionRec   tmp;

	tmp = pParent->clipList;
	pParent->clipList = *universe;
	*universe = tmp;
    }

#ifdef NOTDEF
    (* pScreen->RegionCopy) (&pParent->clipList, universe);
#endif

    pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    if (pScreen->ClipNotify)
    {
	(* pScreen->ClipNotify) (pParent, dx, dy);
    }
}


/*ROUTINE:cmapValidateTree
**++
**      p v x V a l i d a t e T r e e 
**
** FUNCTIONAL DESCRIPTION:
**
**     This routine is very similar to miValidateTree but with changes
**     for handling overlay windows. It recomputes the clip list for 
**     pParent and all its inferiors, taking into account the fact that
**     overlay windows do not clip and are not clipped by non-overlay windows.
**
**     
**     Notes:
**
**     This routine assumes that all affected windows have been marked
**     (valdata created) and their winSize and borderSize regions
**     adjusted to correspond to their new positions. The borderClip and
**     clipList regions should not have been touched.
**
**     The top-most level is treated differently from all lower levels
**     because pParent is unchanged. For the top level, we merge the
**     regions taken up by the marked children back into the clipList
**     for pParent, thus forming a region from which the marked children
**     can claim their areas. For lower levels, where the old clipList
**     and borderClip are invalid, we can't do this and have to do the
**     extra operations done in cmapComputeClips, but this is much faster
**     e.g. when only one child has moved...
**
**
** FORMAL PARAMETERS:
**
**      pParent         -  Parent window of the window subtree to validate
**	pChild          -  First child of pParent that was affected;
**                         this is the changed window itself.
**	kind            -  what kind of configuration caused call
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
**      Always returns 1.
**
** SIDE EFFECTS:     
**
**      The cliplist, borderClip, exposed, and borderExposed regions for
**      each marked window are altered.
**
**--
*/
int cmapValidateTree (
    WindowPtr	  	pParent,	/* Parent to validate */
    WindowPtr	  	pChild,		/* First child of pParent that was
					 * affected */
    VTKind    	  	kind)		/* What kind of configuration caused call */
{
    RegionRec	  	totalClip;	/* Total clipping region available to
					 * the marked children. pParent's clipList
					 * merged with the borderClips of all
					 * the marked children. */
    RegionRec	  	childClip;	/* The new borderClip for the current
					 * child */
    RegionRec		childUnion;	/* the space covered by borderSize for
					 * all marked children */
    RegionRec		exposed;	/* For intermediate calculations */
    register ScreenPtr	pScreen;
    register WindowPtr	pWin;
    Bool		overlap;
    int			viewvals;
    Bool		forward;
    cmapWinPrivPtr      pParentPriv;
    RegionPtr           pOverExpose = NULL;
    WindowPtr           pResizedWin = NULL;
    RegionPtr           pClearRegion = NULL;
    RegionPtr           pRegion = NULL;
    RegionPtr           pParentVizClip = NULL;
    RegionRec           t1, t2;
    WindowPtr           pSib;
    cmapScrPrivPtr	pScrPriv;
    
    TRACE(ErrorF("cmapValidateTree()\n"));

    pScreen = pParent->drawable.pScreen;
    pScrPriv = CMAPScrDevPriv(pScreen);

    pParentPriv = CMAPWinDevPriv(pParent);

    if (kind == VTUnmap)
    {
	cmapClearUnmapWinPtr ClearUnmapWin;

	while (ClearUnmapWin = pScrPriv->pClearUnmapWinList)
	{
	    pScrPriv->pClearUnmapWinList = ClearUnmapWin->flink;
	    if (pScrPriv->pClearUnmapWinList)
	    {
		pScrPriv->pClearUnmapWinList->blink = ClearUnmapWin->blink;
	    }
	    (* pScrPriv->ClearOverlays)(ClearUnmapWin->pClearUnmapWin, ClearUnmapWin->pClearUnmapWinReg);
	    (* pScreen->RegionDestroy)(ClearUnmapWin->pClearUnmapWinReg);
	    xfree(ClearUnmapWin);
	}
    }
    
    if (pParentPriv->numKidsInOtherLayer == 0) 
    {
	return (cmapQuickValidateTree(pParent,pChild,kind));
    }
    
    /* 
    ** see pvxUnmapWindow if you want to know what UnmappedKidsInOtherLayer
    ** means.
    */
    
    pParentPriv->numKidsInOtherLayer &= ~UnmappedKidsInOtherLayer;

    if (pChild == NullWindow)
	pChild = pParent->firstChild;
    
    if (kind == VTUnmap)
    {
	for (pWin = pChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->valdata && CMAPWinDevPriv(pWin)->windowMoved)
	    {
		CMAPWinDevPriv(pWin)->windowMoved = 0;

		if (pOverExpose)
		{
		    (* pScreen->RegionAppend)(pOverExpose,
					      &pWin->borderClip);
		}
		else 
		{
		    pOverExpose = (* pScreen->RegionCreate)(NullBox,0);
		    (* pScreen->RegionCopy)(pOverExpose, &pWin->borderClip);
		}
	    }
	}
    }
    else if (kind == VTOther)
    {
	for (pWin = pChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->valdata &&
		pWin->valdata->before.resized)
	    {
		pResizedWin = pWin;

                if (CMAPWinDevPriv(pWin)->numKidsInOtherLayer &&
                    (CMAPWinDevPriv(pWin)->layer == CMAP_LAYER_NORMAL))
                {
                    CMAPWinDevPriv(pWin)->windowResized = TRUE;
                }

		if (CMAPWinDevPriv(pWin)->windowMoved)
		{
		    pOverExpose = (* pScreen->RegionCreate)(NullBox, 0);
		    (* pScreen->RegionCopy)(pOverExpose, &pWin->borderClip);
                }
		else
		{
		    pClearRegion = (* pScreen->RegionCreate)(NullBox, 0);
		    (* pScreen->RegionCopy)(pClearRegion, &pWin->borderClip);
		}
		
 		break;
	    }
	    
	}		
    }

    (*pScreen->RegionInit) (&childClip, NullBox, 0);
    (*pScreen->RegionInit) (&exposed, NullBox, 0);

    /*
     * compute the area of the parent window occupied
     * by the marked children + the parent itself.  This
     * is the area which can be divied up among the marked
     * children in their new configuration.
     */

    (*pScreen->RegionInit) (&totalClip, NullBox, 0);
    viewvals = 0;

    if ((pChild->drawable.y < pParent->lastChild->drawable.y) ||
	((pChild->drawable.y == pParent->lastChild->drawable.y) &&
	 (pChild->drawable.x < pParent->lastChild->drawable.x)))
    {
	forward = TRUE;
	for (pWin = pChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->valdata)                 
	    {
		(* pScreen->RegionAppend) (&totalClip, &pWin->borderClip);
		if (pWin->viewable)
		    viewvals++;
	    }
	}
    }
    else
    {
	forward = FALSE;
	pWin = pParent->lastChild;
	while (1)
	{
	    if (pWin->valdata)
	    {
		(* pScreen->RegionAppend) (&totalClip, &pWin->borderClip);
		if (pWin->viewable)
		    viewvals++;
	    }
	    if (pWin == pChild)
		break;
	    pWin = pWin->prevSib;
	}
    }
    (* pScreen->RegionValidate)(&totalClip, &overlap);

    /*
     * Now go through the children of the root and figure their new
     * borderClips from the totalClip, passing that off to xxxComputeClips
     * to handle recursively. Once that's done, we remove the child
     * from the totalClip to clip any siblings below it.
     */

    overlap = TRUE;

    if (kind == VTMove || kind == VTOther)
    {
	pParentVizClip = (* pScreen->RegionCreate)(NullBox, 0);
	
	(* pScreen->RegionCopy)(pParentVizClip, &pParent->clipList);    
	
	for (pWin = pParent->firstChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->mapped && InDifferentLayers(pParentPriv, CMAPWinDevPriv(pWin)))
	    {
		(* pScreen->Subtract)(pParentVizClip, 
				      pParentVizClip,
				      &pWin->borderClip);
	    }
	}
    }

    if (kind != VTStack)
    {
	(* pScreen->Union) (&totalClip, 
			    &totalClip,
			    (pParentVizClip) ?
			    pParentVizClip : &pParent->clipList);
	if (viewvals > 1)
	{
	    /*
	     * precompute childUnion to discover whether any of them
	     * overlap.  This seems redundant, but performance studies
	     * have demonstrated that the cost of this loop is
	     * lower than the cost of multiple Subtracts in the
	     * loop below.
	     */

	    (*pScreen->RegionInit) (&childUnion, NullBox, 0);

	    if (forward)
	    {
		for (pWin = pChild; pWin; pWin = pWin->nextSib)
		    if (pWin->valdata && pWin->viewable)
			(* pScreen->RegionAppend) (&childUnion,
						   &pWin->borderSize);
	    }
	    else
	    {
		pWin = pParent->lastChild;
		while (1)
		{
		    if (pWin->valdata && pWin->viewable)
			(* pScreen->RegionAppend) (&childUnion,
						   &pWin->borderSize);
		    if (pWin == pChild)
			break;
		    pWin = pWin->prevSib;
		}
	    }

	    (*pScreen->RegionValidate)(&childUnion, &overlap);

	    if (overlap)
		(*pScreen->RegionUninit) (&childUnion);
	}
    }

    if (pParentVizClip)
    {
	(* pScreen->RegionDestroy)(pParentVizClip);
    }
	
    for (pWin = pChild;
	 pWin != NullWindow;
	 pWin = pWin->nextSib)
    {
	if (pWin->viewable) 
	{
	    if (pWin->valdata) 
	    {
		(* pScreen->Intersect) (&childClip,
					&totalClip,
 					&pWin->borderSize);

		cmapComputeClips (pWin, pScreen, &childClip, kind, &exposed);

		if (overlap)
		{
		    (* pScreen->Subtract) (&totalClip,
				       	   &totalClip,
				       	   &pWin->borderSize);
		}

	    } 
	    else if (pWin->visibility == VisibilityNotViewable) 
	    {
		miTreeObscured(pWin);
	    }
	} 
	else 
	{
	    if (pWin->valdata) 
	    {
		(* pScreen->RegionEmpty)(&pWin->clipList);
		(* pScreen->RegionEmpty)(&pWin->borderClip);
		pWin->valdata = (ValidatePtr)NULL;
	    }
	}
    }

    if (pResizedWin)
    {
        pRegion = (pClearRegion) ? pClearRegion : pOverExpose;
	
	(* pScreen->Subtract) (pRegion, 
			       pRegion, 
			       &pResizedWin->borderClip);

	if (!REGION_NIL(pRegion))
	{
	    (* pScrPriv->ClearOverlays)(pParent, pRegion);
	}
    }
	
    (* pScreen->RegionUninit) (&childClip);

    if (!overlap)
    {
	(*pScreen->Subtract)(&totalClip, &totalClip, &childUnion);
	(*pScreen->RegionUninit) (&childUnion);
    }

    (* pScreen->RegionInit) (&pParent->valdata->after.exposed, NullBox, 0);
    (* pScreen->RegionInit) (&pParent->valdata->after.borderExposed,NullBox,0);

    /*
     * each case below is responsible for updating the
     * clipList and serial number for the parent window
     */
    
    if ((kind != VTStack) && (kind != VTUnmap))
    {
	(* pScreen->RegionInit) (&t1, NullBox, 0);
	(* pScreen->RegionInit) (&t2, NullBox, 0);

	for (pWin = pParent->firstChild; pWin; pWin = pWin->nextSib)
	{
	    if (pWin->mapped && InDifferentLayers(pParentPriv, CMAPWinDevPriv(pWin)))
	    {          
		(* pScreen->RegionAppend)(&totalClip, &pWin->borderClip);

		if (overlap)
		{
		    for (pSib = pWin->nextSib; pSib; pSib = pSib->nextSib)
		    {
			if (InSameLayer(pParentPriv, CMAPWinDevPriv(pSib)))
			{
			    (* pScreen->Intersect)(&t1,
						   &pWin->borderClip,
						   &pSib->borderSize);
			    
			    (* pScreen->RegionAppend)(&t2, &t1);
			}
		    }
		}
	    }
	}

	if (overlap)
	{
	    (* pScreen->RegionValidate)(&totalClip, &overlap);
	    (* pScreen->RegionValidate)(&t2, &overlap);
	    (* pScreen->Subtract)(&totalClip, &totalClip, &t2);
	}
	else
	{
	    (* pScreen->RegionValidate)(&totalClip, &overlap);
	}
	
	(* pScreen->RegionUninit) (&t1);
	(* pScreen->RegionUninit) (&t2);	
    }

    switch (kind) 
    {
     case VTStack:
	break;
     default:
	/*
	 * totalClip contains the new clipList for the parent. Figure out
	 * exposures and obscures as per xxxComputeClips and reset the parent's
	 * clipList.
	 */

	(* pScreen->Subtract) (&pParent->valdata->after.exposed,
			       &totalClip, 
			       &pParent->clipList);

	if (pOverExpose)
	{
	    (* pScreen->RegionValidate)(pOverExpose, &overlap);
	    
	    (* pScreen->Intersect)(pOverExpose,
				   pOverExpose,
				   &totalClip);
	    
	    (* pScreen->RegionAppend)(&pParent->valdata->after.exposed,
				      pOverExpose);
	    
	    (* pScreen->RegionDestroy)(pOverExpose);
	}

	(* pScreen->RegionValidate)(&pParent->valdata->after.exposed,&overlap);

	/* fall through */

     case VTMap:

	if (pParent->backStorage) 
	{
	    (* pScreen->Subtract) (&exposed,&pParent->clipList,&totalClip);
	    (* pScreen->SaveDoomedAreas)(pParent, &exposed, 0, 0);
	}
	
	(* pScreen->RegionCopy) (&pParent->clipList, &totalClip);

	pParent->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	
	break;
    }

    (* pScreen->RegionUninit) (&totalClip);
    (* pScreen->RegionUninit) (&exposed);

    if (pScreen->ClipNotify)
	(*pScreen->ClipNotify) (pParent, 0, 0);

    return (1);
}


void cmapPrintRegion(RegionPtr pReg)
{
    BoxPtr  pBox;
    int     i;
    
    if (REGION_NIL(pReg))
    {
	ErrorF("region is NIL\n");
    }
    else
    {
	pBox = REGION_RECTS(pReg);
       
	for (i=0; i < REGION_NUM_RECTS(pReg); i++)
	{
	    ErrorF("Box[%d]: (%d,%d), (%d,%d)\n",i,pBox[i].x1,pBox[i].y1,
		   pBox[i].x2,pBox[i].y2);
	}
    }
}

/*
 * HISTORY
 */
