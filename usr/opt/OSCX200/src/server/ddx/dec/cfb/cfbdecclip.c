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
#ifdef VMS
#define IDENT "X-1"
#define MODULE_NAME CFBDECCLIP
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991 BY                  *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

#include "X.h"
#include "gcstruct.h"
#include "cfb.h"
#include "windowstr.h"
#include "scrnintstr.h"

static void
CheckOverflow(region)
RegionPtr region;
{
    register int i;
    register BoxPtr pBox;

    if (!region->data) {
        if (region->extents.x2 <
        		region->extents.x1)
            region->extents.x2 = 32767;
        if (region->extents.y2 <
        		region->extents.y1)
            region->extents.y2 = 32767;
    } else {
        for (i=0, pBox = REGION_BOXPTR(region);
    		i < REGION_NUM_RECTS(region);
    		i++, pBox++) {
            if (pBox->x2 < pBox->x1)
    	  pBox->x2 = 32767;
            if (pBox->y2 < pBox->y1)
    	  pBox->y2 = 32767;
        }
    }
}

void cfbComputeClips(pGC, gcPriv, changes, pDrawable)
    register GC     *pGC;
    register cfbPrivGCPtr gcPriv;
    Mask	    changes;
    DrawablePtr     pDrawable;
{
    WindowPtr       pWin;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;
    if (pDrawable->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr) pDrawable;
    } else {
	pWin = (WindowPtr) NULL;
    }

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & 
	    (GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode))
    ||(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))) {
	ScreenPtr pScreen = pGC->pScreen;

	if (pWin) {
	    RegionPtr   pregWin;
	    Bool        freeTmpClip, freeCompClip;

	    if (pGC->subWindowMode == IncludeInferiors) {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = TRUE;
	    }
	    else {
		pregWin = &pWin->clipList;
		freeTmpClip = FALSE;
	    }
	    freeCompClip = gcPriv->freeCompClip;

	    /*
	     * if there is no client clip, we can get by with just keeping
	     * the pointer we got, and remembering whether or not should
	     * destroy (or maybe re-use) it later.  this way, we avoid
	     * unnecessary copying of regions.  (this wins especially if
	     * many clients clip by children and have no client clip.) 
	     */
	    if (pGC->clientClipType == CT_NONE) {
		if (freeCompClip)
		    (*pScreen->RegionDestroy) (gcPriv->pCompositeClip);
		gcPriv->pCompositeClip = pregWin;
		gcPriv->freeCompClip = freeTmpClip;
	    }
	    else {
		/*
		 * we need one 'real' region to put into the composite
		 * clip. if pregWin the current composite clip are real,
		 * we can get rid of one. if pregWin is real and the
		 * current composite clip isn't, use pregWin for the
		 * composite clip. if the current composite clip is real
		 * and pregWin isn't, use the current composite clip. if
		 * neither is real, create a new region. 
		 */

		(*pScreen->TranslateRegion)(pGC->clientClip,
					    pDrawable->x + pGC->clipOrg.x,
					    pDrawable->y + pGC->clipOrg.y);
		CheckOverflow(pGC->clientClip);

		if (freeCompClip)
		{
		    (*pGC->pScreen->Intersect)(gcPriv->pCompositeClip,
					       pregWin, pGC->clientClip);
		    if (freeTmpClip)
			(*pScreen->RegionDestroy)(pregWin);
		}
		else if (freeTmpClip)
		{
		    (*pScreen->Intersect)(pregWin, pregWin, pGC->clientClip);
		    gcPriv->pCompositeClip = pregWin;
		}
		else
		{
		    gcPriv->pCompositeClip = (*pScreen->RegionCreate)(NullBox,
								       0);
		    (*pScreen->Intersect)(gcPriv->pCompositeClip,
					  pregWin, pGC->clientClip);
		}
		gcPriv->freeCompClip = TRUE;
		(*pScreen->TranslateRegion)(pGC->clientClip,
					    -(pDrawable->x + pGC->clipOrg.x),
					    -(pDrawable->y + pGC->clipOrg.y));
						  
	    }
	}			/* end of composite clip for a window */
	else {
	    BoxRec      pixbounds;

	    /* XXX should we translate by drawable.x/y here ? */
	    pixbounds.x1 = 0;
	    pixbounds.y1 = 0;
	    pixbounds.x2 = pDrawable->width;
	    pixbounds.y2 = pDrawable->height;

	    if (gcPriv->freeCompClip)
		(*pScreen->RegionReset)(gcPriv->pCompositeClip, &pixbounds);
	    else {
		gcPriv->freeCompClip = TRUE;
		gcPriv->pCompositeClip = (*pScreen->RegionCreate)(&pixbounds,
								   1);
	    }

	    if (pGC->clientClipType == CT_REGION)
	    {
		(*pScreen->TranslateRegion)(gcPriv->pCompositeClip,
					    -pGC->clipOrg.x, -pGC->clipOrg.y);
		CheckOverflow(pGC->clientClip);
		(*pScreen->Intersect)(gcPriv->pCompositeClip,
				      gcPriv->pCompositeClip,
				      pGC->clientClip);
		(*pScreen->TranslateRegion)(gcPriv->pCompositeClip,
					    pGC->clipOrg.x, pGC->clipOrg.y);
	    }
	}			/* end of composute clip for pixmap */
    }
}

