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
/* $XConsortium: mdepthinit.c,v 1.3 92/07/30 10:42:32 rws Exp $ */

/*

Copyright 1992 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "gcstruct.h"
#include "mibstore.h"

extern int defaultColorVisualClass;

#define BitsPerPixel(d) (\
    (1 << PixmapWidthPaddingInfo[d].padBytesLog2) * 8 / \
    (PixmapWidthPaddingInfo[d].padRoundUp+1))

Bool
mcfbCreateGC(pGC)
    GCPtr   pGC;
{
    switch (BitsPerPixel(pGC->depth)) {
    case 1:
	return mfbCreateGC (pGC);
    case 8:
	return cfbCreateGC (pGC);
    case 16:
	return cfb16CreateGC (pGC);
    case 32:
	return cfb32CreateGC (pGC);
    }
    return FALSE;
}

void
mcfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    unsigned long	*pdstStart;	/* where to put the bits */
{
    switch (BitsPerPixel(pDrawable->depth)) {
    case 1:
	mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 8:
	cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 16:
	cfb16GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 32:
	cfb32GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    }
    return;
}

void
mcfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer	pdstLine;
{
    switch (BitsPerPixel(pDrawable->depth)) 
    {
    case 1:
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 8:
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 16:
	cfb16GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 32:
	cfb32GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    }
}

Bool
mcfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int depth;			/* depth of root */
    int	bpp;			/* bits per pixel of root */
{
    extern int		cfbWindowPrivateIndex;
    extern int		cfbGCPrivateIndex;
    int			wpi, gpi;

    switch (bpp) {
    case 8:
	cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	wpi = cfbWindowPrivateIndex;
	gpi = cfbGCPrivateIndex;
	break;
    case 16:
	cfb16SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	wpi = cfbWindowPrivateIndex;
	gpi = cfbGCPrivateIndex;
	break;
    case 32:
	cfb32SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	wpi = cfbWindowPrivateIndex;
	gpi = cfbGCPrivateIndex;
	break;
    default:
	return FALSE;
    }
    if (bpp != 8)
	cfbAllocatePrivates (pScreen, &wpi, &gpi);
    if (bpp != 16)
	cfb16AllocatePrivates (pScreen, &wpi, &gpi);
    if (bpp != 32)
	cfb32AllocatePrivates (pScreen, &wpi, &gpi);
    pScreen->CreateGC = mcfbCreateGC;
    pScreen->GetImage = mcfbGetImage;
    pScreen->GetSpans = mcfbGetSpans;
    return TRUE;
}

extern int  cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;

mcfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel */
    int depth;			/* depth of screen */
{
    int		i;
    pointer	oldDevPrivate;
    VisualPtr	visuals;
    int		nvisuals;
    DepthPtr	depths;
    int		ndepths;
    VisualID	defaultVisual;
    int		rootdepth;
    miBSFuncPtr	bsFuncs;
    extern miBSFuncRec	cfbBSFuncRec, cfb16BSFuncRec, cfb32BSFuncRec;
    extern Bool		cfbCloseScreen(), cfb16CloseScreen(), cfb32CloseScreen();

    rootdepth = depth;
    if (!cfbInitVisuals(&visuals, &depths, &nvisuals, &ndepths, &rootdepth, &defaultVisual, 1 << (bpp - 1), 8))
	return FALSE;
    rootdepth = depth;
    oldDevPrivate = pScreen->devPrivate;
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals,
			(miBSFuncPtr) 0))
	return FALSE;
    switch(bpp)
    {
    case 8:
	pScreen->CloseScreen = cfbCloseScreen;
	bsFuncs = &cfbBSFuncRec;
	break;
    case 16:
	pScreen->CloseScreen = cfb16CloseScreen;
	pScreen->devPrivates[cfb16ScreenPrivateIndex].ptr = pScreen->devPrivate;
	pScreen->devPrivate = oldDevPrivate;
	bsFuncs = &cfb16BSFuncRec;
	break;
    case 32:
	pScreen->CloseScreen = cfb32CloseScreen;
	pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr = pScreen->devPrivate;
	pScreen->devPrivate = oldDevPrivate;
	bsFuncs = &cfb32BSFuncRec;
	break;
    }
    miInitializeBackingStore (pScreen, bsFuncs);
    return TRUE;
}


/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
mcfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel */
    int depth;			/* depth of screen */
{
    if (!mcfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth))
	return FALSE;
    return mcfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth);
}
