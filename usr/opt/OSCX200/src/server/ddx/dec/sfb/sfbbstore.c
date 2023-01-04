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
#define IDENT "X-2"
#define MODULE_NAME SFBBSTORE
#include "module_ident.h"
#endif
/***********************************************************

Copyright 1987 by the Regents of the University of California
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  
******************************************************************/

/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991 BY                        *
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
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel.
**
**--
**/


/* $Header: */

#include    "X.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"
#include    "mibstore.h"

#include    "sfb.h"
#include    "sfbbstore.h"
#include    "sfbblt.h"


/*-
 *-----------------------------------------------------------------------
 * sfbSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap. This is very simple to do, since
 *	sfbBitbltXxxYyy is designed for this very thing. The region to save is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the screen
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */

void
sfbSaveAreas(pPixmap, prgnSave, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnSave; 	/* Region to save (pixmap-relative) */
    int	    	  	xorg;	    	/* X origin of region */
    int	    	  	yorg;	    	/* Y origin of region */
{
    register DDXPointPtr pPt;
    DDXPointPtr		pPtsInit;
    register BoxPtr	pBox;
    register int	i;
    DrawablePtr		pSrcDraw;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

    i = REGION_NUM_RECTS(prgnSave);
    pPtsInit = (DDXPointPtr)ALLOCATE_LOCAL(i * sizeof(DDXPointRec));
    if (!pPtsInit)
	return;

    pBox = REGION_RECTS(prgnSave);
    pPt = pPtsInit;
    for (; i != 0; i--) {
	pPt->x = pBox->x1 + xorg;
	pPt->y = pBox->y1 + yorg;
	pPt++;
	pBox++;
    }

    pSrcDraw = (DrawablePtr)pPixmap->drawable.pScreen->devPrivate;

    if (SCREENMEMORY(pPixmap)) {
	scrPriv = SFBSCREENPRIV(pSrcDraw->pScreen);
	scrPriv->lastGC = (GCPtr)NULL;
	sfb = scrPriv->sfb;
	SFBROP(sfb, GXcopy);
	SFBPLANEMASK(sfb, SFBBUSALL1);
	sfbBitbltScrScr(pSrcDraw, (DrawablePtr)pPixmap, prgnSave, pPtsInit);
    } else {
	sfbBitbltScrMemCopy(pSrcDraw,
		(DrawablePtr)pPixmap, prgnSave, pPtsInit, 0, 0, -1, 0);
    }
    DEALLOCATE_LOCAL(pPtsInit);
}

/*-
 *-----------------------------------------------------------------------
 * sfbRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. This is very simple to do, since
 *	sfbBitbltXxxYyy is designed for this very thing.  The region to 
 *      restore is already destination-relative and we're given the offset to
 *	the window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the pixmap
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
sfbRestoreAreas(pPixmap, prgnRestore, xorg, yorg)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnRestore; 	/* Region to restore (screen-relative)*/
    int	    	  	xorg;	    	/* X origin of window */
    int	    	  	yorg;	    	/* Y origin of window */
{
    register DDXPointPtr pPt;
    DDXPointPtr		pPtsInit;
    register BoxPtr	pBox;
    register int	i;
    DrawablePtr		pDstDraw;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

    i = REGION_NUM_RECTS(prgnRestore);
    pPtsInit = (DDXPointPtr)ALLOCATE_LOCAL(i*sizeof(DDXPointRec));
    if (!pPtsInit)
	return;
    
    pBox = REGION_RECTS(prgnRestore);
    pPt = pPtsInit;
    for (; i != 0; i--) {
	pPt->x = pBox->x1 - xorg;
	pPt->y = pBox->y1 - yorg;
	pPt++;
	pBox++;
    }

    pDstDraw = (DrawablePtr)pPixmap->drawable.pScreen->devPrivate;
    scrPriv = SFBSCREENPRIV(pDstDraw->pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    sfb = scrPriv->sfb;
    SFBROP(sfb, GXcopy);
    SFBPLANEMASK(sfb, SFBBUSALL1);
    if (SCREENMEMORY(pPixmap)) {
	sfbBitbltScrScr((DrawablePtr)pPixmap, pDstDraw, prgnRestore, pPtsInit);
    } else {
	sfbBitbltMemScr((DrawablePtr)pPixmap, pDstDraw, prgnRestore, pPtsInit);
    }
    DEALLOCATE_LOCAL(pPtsInit);
}
