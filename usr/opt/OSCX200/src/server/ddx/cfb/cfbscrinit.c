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
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/* $XConsortium: cfbscrinit.c,v 5.25 92/07/30 10:43:43 rws Exp $ */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "cfbmskbits.h"
#include "mibstore.h"

extern RegionPtr mfbPixmapToRegion();
extern RegionPtr cfbCopyPlane();
extern Bool mfbAllocatePrivates();


miBSFuncRec cfbBSFuncRec = {
    cfbSaveAreas,
    cfbRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};

Bool
cfbCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    int	    d;
    DepthPtr	depths = pScreen->allowedDepths;

    for (d = 0; d < pScreen->numDepths; d++)
	xfree (depths[d].vids);
    xfree (depths);
    xfree (pScreen->visuals);
#ifdef CFB_NEED_SCREEN_PRIVATE
    xfree (pScreen->devPrivates[cfbScreenPrivateIndex].ptr);
#else
    xfree (pScreen->devPrivate);
#endif
    return TRUE;
}

Bool
cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int	i;
    extern RegionPtr	(*cfbPuntCopyPlane)();

    if (!cfbAllocatePrivates(pScreen, (int *) 0L, (int *) 0L))
	return FALSE;
    pScreen->defColormap = FakeClientID(0);
    /* let CreateDefColormap do whatever it wants for pixels */ 
    pScreen->blackPixel = pScreen->whitePixel = (Pixel) 0;
    pScreen->QueryBestSize = mfbQueryBestSize;
    /* SaveScreen */
    pScreen->GetImage = cfbGetImage;
    pScreen->GetSpans = cfbGetSpans;
    pScreen->CreateWindow = cfbCreateWindow;
    pScreen->DestroyWindow = cfbDestroyWindow;
    pScreen->PositionWindow = cfbPositionWindow;
    pScreen->ChangeWindowAttributes = cfbChangeWindowAttributes;
    pScreen->RealizeWindow = cfbMapWindow;
    pScreen->UnrealizeWindow = cfbUnmapWindow;
    pScreen->PaintWindowBackground = cfbPaintWindow;
    pScreen->PaintWindowBorder = cfbPaintWindow;
    pScreen->CopyWindow = cfbCopyWindow;
    pScreen->CreatePixmap = cfbCreatePixmap;
    pScreen->DestroyPixmap = cfbDestroyPixmap;
    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->CreateGC = cfbCreateGC;
    pScreen->CreateColormap = cfbInitializeColormap;
    pScreen->DestroyColormap = NoopDDA;
#ifdef	STATIC_COLOR
    pScreen->InstallColormap = cfbInstallColormap;
    pScreen->UninstallColormap = cfbUninstallColormap;
    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
    pScreen->StoreColors = NoopDDA;
#endif
    pScreen->ResolveColor = cfbResolveColor;
    pScreen->BitmapToRegion = mfbPixmapToRegion;

    mfbRegisterCopyPlaneProc (pScreen, cfbCopyPlane);
    return TRUE;
}

cfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int	i, j;
#ifdef CFB_NEED_SCREEN_PRIVATE
    pointer oldDevPrivate;
#endif
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;

    rootdepth = 0;
    if (!cfbInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootdepth, &defaultVisual, 1<<(PSZ-1), 8))
	return FALSE;
#ifdef CFB_NEED_SCREEN_PRIVATE
    oldDevPrivate = pScreen->devPrivate;
#endif
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals,
			(miBSFuncPtr) 0L))
	return FALSE;
    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = cfbCloseScreen;
    /* init backing store here so we can overwrite CloseScreen without stepping
     * on the backing store wrapped version */
    miInitializeBackingStore (pScreen, &cfbBSFuncRec);
#ifdef CFB_NEED_SCREEN_PRIVATE
    pScreen->devPrivates[cfbScreenPrivateIndex].ptr = pScreen->devPrivate;
    pScreen->devPrivate = oldDevPrivate;
#endif
    return TRUE;
}

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
cfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;
    return cfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
}
