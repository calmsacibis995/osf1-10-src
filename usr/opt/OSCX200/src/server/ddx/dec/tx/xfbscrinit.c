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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <sys/types.h>
#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "xfbstruct.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "tfbdraw.h"*/
#include "mibstore.h"
#include "xfbxorfix.h"
#include "xfbbanksw.h"
#include "sys/workstation.h"

#ifdef DECWINDOWS
#include "cfbcmap.h"  /* exists in dec/cfb only, not needed for MIT cfb */
#endif DECWINDOWS

extern RegionPtr mfbPixmapToRegion();
extern RegionPtr xfbCopyPlane();
extern Bool mfbAllocatePrivates();

extern int defaultColorVisualClass;

#define CFBPSZ 8  /* cfb pixel size */
#define _BP 8
#define _RZ ((CFBPSZ + 2) / 3)
#define _RS 0
#define _RM ((1 << _RZ) - 1)
#define _GZ ((CFBPSZ - _RZ + 1) / 2)
#define _GS _RZ
#define _GM (((1 << _GZ) - 1) << _GS)
#define _BZ (CFBPSZ - _RZ - _GZ)
#define _BS (_RZ + _GZ)
#define _BM (((1 << _BZ) - 1) << _BS)
#define _CE (1 << _RZ)

static VisualRec visuals[] = {
/* vid  class        bpRGB cmpE nplan rMask gMask bMask oRed oGreen oBlue */
    0,  PseudoColor, _BP,  1<<CFBPSZ,   CFBPSZ,  0,   0,   0,   0,   0,   0,
    0,  DirectColor, _BP, _CE,       	_RZ,  	_RM, _GM, _BM, _RS, _GS, _BS,
    0,  GrayScale,   _BP,  1<<CFBPSZ,   CFBPSZ,  0,   0,   0,   0,   0,   0,
    0,  StaticGray,  _BP,  1<<CFBPSZ,   CFBPSZ,  0,   0,   0,   0,   0,   0,
    0,  StaticColor, _BP,  1<<CFBPSZ,   CFBPSZ,  _RM, _GM, _BM, _RS, _GS, _BS,
    0,  TrueColor,   _BP, _CE,       _RZ,  _RM, _GM, _BM, _RS, _GS, _BS,
    /* 24-bit depth visuals: */
    0,  TrueColor,   _BP, 1<<8,       8,  255, 255<<8, 255<<16, 0, 8, 16,
    0,  DirectColor, _BP, 1<<8,       8,  255, 255<<8, 255<<16, 0, 8, 16
};

#define	NUMVISUALS	((sizeof visuals)/(sizeof visuals[0]))
#define	NUMVISUALS24	2
#define	NUMVISUALS8	NUMVISUALS - NUMVISUALS24

static  VisualID VIDs[NUMVISUALS];

static DepthRec depths[] = {
/* depth	numVid		vids */
    1,		0,		NULL,
    8,		NUMVISUALS8,	VIDs,
    24,		NUMVISUALS24,	&VIDs[NUMVISUALS8]
};

#define NUMDEPTHS	((sizeof depths)/(sizeof depths[0]))

static unsigned long xfbGeneration = 0;

int xfbWindowPrivateIndex;
int xfbGCPrivateIndex;

#include "ws.h"

extern int wsScreenPrivateIndex;

/* mi backing store support functions */
miBSFuncRec xfbBSFuncRec = {
    xfbSaveAreas,
    xfbRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};


static Bool
xfbCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    cfb32DrawClose(pScreen);
    cfbDrawClose(pScreen);
    return TRUE;
}

/*
General purpose function to do equivalent of XMatchVisualInfo().
*/
static VisualPtr
xfbMatchVisual(pScreen, depth, visualClass)
    ScreenPtr pScreen;
    int depth, visualClass;
{
    int i, j;
    DepthPtr pDepth;
    VisualPtr pVisual;
    unsigned long vid;

    pDepth = pScreen->allowedDepths;
    for (i = 0; i < pScreen->numDepths; i++) {
	if (pDepth->depth == depth) {
	    for (j = 0; j < pDepth->numVids; j++) {
		vid = pDepth->vids[j];
		for (pVisual = pScreen->visuals; pVisual->vid != vid; pVisual++)
		    ;
		if (pVisual->class == visualClass) {
		    return (pVisual);
		}
	    }
	    return (NULL);
	}
	pDepth++;
    }
    return (NULL);
}

Bool
xfbScreenInit(pScreen, fb8, xsize, ysize, dpix, dpiy, width, fb24,
    defDepth, defVisualClass, useBankSw, useXorFix, useBStore)
    ScreenPtr pScreen;
    pointer fb8;		/* pointer to 8-bit depth framebuffer */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    pointer fb24;		/* pointer to 24-bit depth framebuffer */
    int defDepth;		/* root depth */
    int defVisualClass;		/* root visual class */
    Bool useBankSw;		/* whether bank switching is turned on */
    Bool useXorFix;		/* whether XorFix is turned on */
    Bool useBStore;		/* whether mi backing store is init-ed */
{
    int	i;
    VisualPtr pVisual;

#ifdef DPS
    {   /* XXX temp crock until server can do per-screen extension init */
	extern void cfbCreateDDXMarkProcs();
	XMIRegisterDDXMarkProcsProcs(pScreen, cfbCreateDDXMarkProcs);
    }
#endif DPS

    if (xfbGeneration != serverGeneration)
    {
	/*  Set up the visual IDs */
	for (i = 0; i < NUMVISUALS; i++) {
	    visuals[i].vid = FakeClientID(0);
	    VIDs[i] = visuals[i].vid;
	}
	xfbGeneration = serverGeneration;
    }

    /* init mfb drawing routines: */
    if (!mfbAllocatePrivates(pScreen,
			     &xfbWindowPrivateIndex, &xfbGCPrivateIndex))
	return FALSE;

    /* init cfb drawing routines: */
    if (!cfbDrawInit(pScreen, fb8, xsize, ysize, width,
	xfbWindowPrivateIndex, xfbGCPrivateIndex)) {
	return FALSE;
    }

    /* init tfb drawing routines (alloc window and GC privates also) */
    if (!cfb32DrawInit(pScreen, fb24, xsize, ysize, width,
	xfbWindowPrivateIndex, xfbGCPrivateIndex)) {
	return FALSE;
    }

    /* dts * (inch/dot) * (25.4 mm / inch) = mm */
    pScreen->width = xsize;
    pScreen->height = ysize;
    pScreen->mmWidth = ((xsize * 254) + (dpix * 5)) / (dpix * 10);
    pScreen->mmHeight = ((ysize * 254) + (dpiy * 5)) / (dpiy * 10);
    pScreen->numDepths = NUMDEPTHS;
    pScreen->allowedDepths = depths;

    pScreen->minInstalledCmaps = 1;
    pScreen->maxInstalledCmaps = 2;
    pScreen->backingStoreSupport = Always;
    pScreen->saveUnderSupport = NotUseful;
    /* let CreateDefColormap do whatever it wants */ 
    pScreen->blackPixel = pScreen->whitePixel = (Pixel) 0;

    /* cursmin and cursmax are device specific */

    pScreen->numVisuals = NUMVISUALS;
    pScreen->visuals = visuals;

    /* anything that xfb doesn't know about is assumed to be done
       elsewhere.  (we put in no-op only for things that we KNOW
       are really no-op.
    */
    pScreen->CreateWindow = xfbCreateWindow;
    pScreen->DestroyWindow = xfbDestroyWindow;
    pScreen->PositionWindow = xfbPositionWindow;
    pScreen->ChangeWindowAttributes = xfbChangeWindowAttributes;
    pScreen->RealizeWindow = xfbMapWindow;
    pScreen->UnrealizeWindow = xfbUnmapWindow;

    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->CloseScreen = xfbCloseScreen;
    pScreen->QueryBestSize = mfbQueryBestSize;
    pScreen->GetImage = xfbGetImage;
    pScreen->GetSpans = xfbGetSpans;
    pScreen->SourceValidate = (void (*)()) 0;
    pScreen->CreateGC = xfbCreateGC;
    pScreen->CreatePixmap = xfbCreatePixmap;
    pScreen->DestroyPixmap = xfbDestroyPixmap;
    pScreen->ValidateTree = miValidateTree;

    pScreen->ResolveColor = cfbResolveColor;

    pScreen->RegionCreate = miRegionCreate;
    pScreen->RegionInit = miRegionInit;
    pScreen->RegionCopy = miRegionCopy;
    pScreen->RegionDestroy = miRegionDestroy;
    pScreen->RegionUninit = miRegionUninit;
    pScreen->Intersect = miIntersect;
    pScreen->Inverse = miInverse;
    pScreen->Union = miUnion;
    pScreen->Subtract = miSubtract;
    pScreen->RegionReset = miRegionReset;
    pScreen->TranslateRegion = miTranslateRegion;
    pScreen->RectIn = miRectIn;
    pScreen->PointInRegion = miPointInRegion;
    pScreen->WindowExposures = xfbWindowExposures;
    pScreen->PaintWindowBackground = xfbPaintWindow;
    pScreen->PaintWindowBorder = xfbPaintWindow;
    pScreen->CopyWindow = xfbCopyWindow;
    pScreen->ClearToBackground = miClearToBackground;
    pScreen->ClipNotify = (void (*)()) 0;

    pScreen->RegionNotEmpty = miRegionNotEmpty;
    pScreen->RegionEmpty = miRegionEmpty;
    pScreen->RegionExtents = miRegionExtents;
    pScreen->RegionAppend = miRegionAppend;
    pScreen->RegionValidate = miRegionValidate;
    pScreen->BitmapToRegion = mfbPixmapToRegion;
    pScreen->RectsToRegion = miRectsToRegion;
    pScreen->SendGraphicsExpose = miSendGraphicsExpose;

    pScreen->BlockHandler = NoopDDA;
    pScreen->WakeupHandler = NoopDDA;
    pScreen->blockData = (pointer)0;
    pScreen->wakeupData = (pointer)0;

    pScreen->CreateColormap = cfbInitializeColormap;
    pScreen->DestroyColormap = NoopDDA;

    pScreen->defColormap = FakeClientID(0);
    
    mfbRegisterCopyPlaneProc (pScreen, cfbCopyPlane);

    if (pVisual = xfbMatchVisual(pScreen, defDepth, defVisualClass)) {
	pScreen->rootDepth = defDepth;
	pScreen->rootVisual = pVisual->vid;
    } else {
	pScreen->rootDepth = 8;
	pScreen->rootVisual = visuals[0].vid;
    }

    /* bank switched segment is mapped to fb8 address */
    if (useBankSw && !xfbBankSwitchInit(pScreen, fb8)) {
	ErrorF("xfbBankSwitchInit() failed\n");
	return (FALSE);
    }

    if (useXorFix && !xfbXorFixInit(pScreen)) {
	ErrorF("xfbXorFixInit() failed\n");
	return (FALSE);
    }

    if (useBStore) {
	if (useBankSw) {
	    /* init bank switched bstore here when we get it to work */
	} else {
	    miInitializeBackingStore(pScreen, &xfbBSFuncRec);
	}
    }

    {	
        wsScreenPrivate *wsp = (wsScreenPrivate *)pScreen->devPrivates[wsScreenPrivateIndex].ptr;
        D_GENERICDrawInit(pScreen, wsp->screenDesc->screen);
    }

/* do this later
    mfbRegisterCopyPlaneProc (pScreen, xfbCopyPlane);
*/
#ifdef MITSHM
    ShmRegisterFbFuncs(pScreen);
#endif

    return (TRUE);
}

