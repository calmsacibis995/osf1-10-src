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
static char *rcsid = "@(#)$RCSfile: ffb463.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:05:09 $";
#endif
/*
 */
#ifdef VMS
#include "ffb.h"
long ffbWindowID(WindowPtr pWin)
{
return;
}
#else
#include "ffb.h"
#include "../cmap/cmap.h"


#if FFBPIXELBITS==32
Bool ffb32InitializeColormap(ColormapPtr pMap)
{
    VisualPtr pVisual = pMap->pVisual;
    /*
     ** if this is an overlay visual then we need to allocate the
     ** first entry in the colormap (0). This is the invisible pixel
     ** and it is "owned" by the server. If a client wants
     ** to know about or use this pixel then it can get the 
     ** SERVER_OVERLAY_VISUALS property from the root window.
     **
     **
     ** The main reason we do this is that if an unsuspecting client uses
     ** the 4-plane PseudoColor visual and the client does not know or care 
     ** about overlays then it may try to use 0 for a real color and it will 
     ** not get what it expects (the hardware treats 0 as invisible). Hence, we
     ** pre-allocate 0 so that when the client allocates other colors it will
     ** never get 0 for a pixel value.
     **
     ** Another reason is that if this is the root window's colormap
     ** (i.e. server running in overlays), then 0 will probably be 
     ** allocated for BlackPixel or WhitePixel (see cmapInitColor). So to 
     ** avoid that and to make color allocation start from pixel 1 we do 
     ** the following...
     **
     ** Pixel 0 should also be marked private so that other clients don't
     ** try and share it.
     */

    if ((pVisual->class == PseudoColor) && (pVisual->nplanes == 4))
    {
	/*
	 ** the code below can be done by calling AllocCP as follows:
	 **
	 ** AllocCP(pMap, pMap->red, 1, 0, TRUE, &pixel, &mask)
	 **
	 ** Unfortunately, AllocCP is defined as a static function in 
	 ** server/dix/colormap.c So we'll have to do it inline here.
	 ** 
	 ** Its only two lines, so its not that bad...
	 **
	 */

	pMap->red->refcnt = AllocPrivate;
	pMap->red->fShared = FALSE;
    }
    return (Bool) CFB_NAME(InitializeColormap)(pMap);
}
#endif



long ffbWindowID(WindowPtr pWin)
{
#if FFBPIXELBITS==32 && defined(MCMAP)
    ColormapPtr pMap;
    long wid;

    if (pWin->drawable.type != DRAWABLE_WINDOW)
	return 0;

    pMap = CMAPWinDevPriv(pWin)->pMap;
    wid = FFBBUFDESCRIPTOR(pWin)->wid;

    switch (wid)
    {
     case WID_8:
     case WID_8_B1:
     case WID_8_B2:
	if (CMAPDevPriv(pMap)->tag == CMAP_SECOND_PALETTE) {
	    wid |= WID_8_C1;
	}
	break;
     case WID_24:
	if (CMAPDevPriv(pMap)->tag == CMAP_FIRST_PALETTE) {
	    wid |= WID_24_C0;
	}
	else if (CMAPDevPriv(pMap)->tag == CMAP_SECOND_PALETTE) {
	    wid |= WID_24_C1;
	}
    }
    return wid;
#else
    return 0;
#endif
}



void ffbWriteWindowTypeTable(ScreenPtr pScreen, ColormapPtr pMap, long where)
{
#if FFBPIXELBITS==32 && defined(MCMAP)
    /*
     * nothing to do, since driver preloads all ffb
     * window types, and private types not supported.
     */
#else
    abort();
#endif
}
#endif
/*
 * HISTORY
 */
