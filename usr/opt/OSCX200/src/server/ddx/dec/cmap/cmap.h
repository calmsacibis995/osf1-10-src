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
#ifndef _CMAP_H_
#define _CMAP_H_

#include "X.h"
#include "Xmd.h"
#include "gc.h"
#include "gcstruct.h"
#include "pixmap.h"
#include "scrnintstr.h"
#include "region.h"
#include "regionstr.h"
#include "colormap.h"
#include "colormapst.h"
#include "miscstruct.h"
#include "servermd.h"
#include "validate.h"
#include "windowstr.h"
#include "fontstruct.h"
#include "mi.h"
#include "mibstore.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/devio.h>
#include <sys/workstation.h>

typedef struct _cmapWinPriv {
    struct _cmapWinPriv *flink;		/* ptr to next wind on the colormap's queue */
    struct _cmapWinPriv *blink;		/* ptr to prev wind on the colormap's queue */
    WindowPtr            pWin;		/* window pointer */
    ColormapPtr          pMap;
    CARD16               pid;		/* this is either 0x0000 or 0x8000          */
    CARD32               numKidsInOtherLayer;
    long                 layer;
    Bool                 windowMoved;
    Bool    	    	 windowResized;
} cmapWinPrivRec, *cmapWinPrivPtr;

typedef struct _cmapClearUnmapWin
{
    struct _cmapClearUnmapWin  *flink;  /* ptr to next wind on the ClearUnmap's queue */
    struct _cmapClearUnmapWin  *blink;  /* ptr to prev wind on the ClearUnmap's queue */
    WindowPtr    pClearUnmapWin;
    RegionPtr    pClearUnmapWinReg;
} cmapClearUnmapWin, *cmapClearUnmapWinPtr;

#define CMAPWinDevPriv(_pWin)						      \
    ((cmapWinPrivPtr)((_pWin)->devPrivates[cmapWindowPrivateIndex].ptr))

#define CMAP_LAYER_NORMAL      0
#define CMAP_LAYER_OVERLAY     1

#define UnmappedKidsInOtherLayer 0x80000000

#define InSameLayer(_pPriv1, _pPriv2)					      \
    ((_pPriv1)->layer == (_pPriv2)->layer)

#define InDifferentLayers(_pPriv1, _pPriv2)				      \
    ((_pPriv1)->layer != (_pPriv2)->layer)

#define InNormalInOverlay(_pPriv1, _pPriv2)				      \
    (( (_pPriv1)->layer == CMAP_LAYER_NORMAL) &&			      \
     ( (_pPriv2)->layer == CMAP_LAYER_OVERLAY) )


typedef struct _cmapScrPriv {
    /* support functions provided by ddx */
    void    	(*WriteWindowTypeTable)();
    void    	(*FillWindowType)();
    void    	(*LoadColormap)();
    int    	(*ExpandDirectColors)();
    void    	(*ClearOverlays)();
    void    	(*ClearToBackground)();
    long	(*OverlayVisualIndex)();
    long	(*CmapTypeToIndex)();
    Bool	(*IsOverlayWindow)();
    /* wrapped functions */
    Bool	(*CloseScreen)();
    Bool	(*CreateWindow)();
    Bool	(*DestroyWindow)();
    Bool	(*ChangeWindowAttributes)();
    Bool	(*RealizeWindow)();
    Bool	(*UnrealizeWindow)();
    void	(*CopyWindow)();
    Bool	(*CreateColormap)();
    void	(*DestroyColormap)();
    /* additional information */
    RegionPtr	pClearUnmapWinReg;
    cmapClearUnmapWin *pClearUnmapWinList;
    long        lastInstalledCmapIndex; /* index into pInstalledCmaps[] */
    ColormapPtr pLastInstalledCmap;
    ColormapPtr pInstalledCmaps[1];	/* must be last! */
} cmapScrPrivRec, *cmapScrPrivPtr;

#define CMAPScrDevPriv( _pScr )						      \
    ((cmapScrPrivPtr) ((_pScr)->devPrivates[cmapScreenPrivateIndex].ptr))


typedef struct _cmapPriv
{
    cmapWinPrivPtr  windowQFlink;	/* Queue head for windows which  */
    cmapWinPrivPtr  windowQBlink;	/* use this colormap */
    ColormapPtr     pMap;		/* colormap pointer */
    long            tag;		/* one of the CMAP_ constants... */
    pointer	    ddxPriv;
} cmapPrivRec, *cmapPrivPtr;

#define CMAPDevPriv(_pMap)						      \
    ((cmapPrivPtr)((_pMap)->devPriv))

#define CMAP_FIRST_PALETTE          0
#define CMAP_SECOND_PALETTE         1
#define CMAP_OVERLAY_PALETTE        2
#define CMAP_TRUECOLOR24            3
#define CMAP_STATICGRAY8            4
#define CMAP_TRUECOLOR12            5
#define CMAP_NOT_INSTALLED         -1
/*
**
** These numbers (0..5) are indicees into the PVX window type table.
** They are the numbers that are stored in the DCC's 3 window type bits.
** We can only use the first 6 since entries 7 and 8 are for the cursor colors.
**
** So a window that has a 24-plane TrueColor visual would have the value
** CMAP_TRUECOLOR24 in its DCC's window type bits, a window which uses
** the colormap that's installed in the first half of the palette would
** have CMAP_FIRST_PALETTE in its DCC's window type bits...and so on.
**
** Overlays do NOT have a special index. They are enabled all the time and
** and any time a NON-ZERO value is written into the overlay planes then
** the Bt463 will display the overlay-pixel and use the overlay palette.
** The Bt463 is configured such that the overlay palette is at a fixed
** starting address (512).
**
** CMAP_FIRST_PALETTE, CMAP_SECOND_PALETTE, and CMAP_OVERLAY_PALETTE
** are defined as 0, 1, and 2 so that if we multiply that by 256 we get
** the starting address of their respective colormaps. CMAP_FIRST_PALETTE
** and CMAP_SECOND_PALETTE are therefore used as window type bits in the DCC
** and also in calculating the starting address of the colormap (which is
** needed when we need to store colors in a certain colormap). 
**
*/
extern unsigned long cmapScreenPrivateIndex;
extern unsigned long cmapWindowPrivateIndex;

extern long cmapGetVisualIndex(ScreenPtr pScreen, short class, short depth);
extern Bool cmapCreateColormap(ColormapPtr pMap);
extern void cmapDestroyColormap(ColormapPtr pMap);
extern int  cmapListInstalledColormaps(ScreenPtr pScreen, Colormap *pMaps);
extern void cmapInstallColormap(ColormapPtr pMap);
extern void cmapUninstallColormap(ColormapPtr pMap);
extern void cmapStoreColors(ColormapPtr pMap, long nData, xColorItem *pData);
extern void cmapQuickComputeClips (WindowPtr pParent, ScreenPtr pScreen, 
				   RegionPtr universe, VTKind kind, 
				   RegionPtr exposed);
extern long cmapQuickValidateTree(WindowPtr pParent, WindowPtr pChild, VTKind kind);
extern void cmapComputeClips(WindowPtr pParent, ScreenPtr pScreen,
			     RegionPtr universe, VTKind kind, RegionPtr exposed);
extern int  cmapValidateTree (WindowPtr pParent, WindowPtr pChild, VTKind kind);
extern Bool cmapCreateWindow(WindowPtr pWin);
extern Bool cmapDestroyWindow(WindowPtr pWin);
extern Bool cmapRealizeWindow(WindowPtr pWindow);
extern Bool cmapUnrealizeWindow(WindowPtr pWindow);
extern void cmapCopyWindow(WindowPtr pWin, DDXPointRec oldOrigin, RegionPtr pOldRegion);
extern Bool cmapChangeWindowAttributes(WindowPtr pWin, unsigned long mask);
extern Bool cmapCloseScreen(int index, ScreenPtr pScreen);
extern int  cmapExpandDirectColors(ColormapPtr pmap, int ndef, xColorItem *indefs, xColorItem *outdefs);
extern void cmapClearToBackground(WindowPtr pWin, short x, short y, unsigned short w, unsigned short h, Bool generateExposures);
extern Bool cmapSetupScreen(ScreenPtr pScreen,
			    void (*WriteWindowTypeTable)(),
			    void (*FillWindowType)(),
			    void (*LoadColormap)(),
			    int  (*ExpandDirectColors)(),
			    void (*ClearOverlays)(),
			    void (*ClearToBackground)(),
			    long (*OverlayVisualIndex)(),
			    long (*CmapTypeToIndex)(),
			    Bool (*IsOverlayWindow)());

#define TRACE(S)	{ S; }
#endif
/*
 * HISTORY
 */
