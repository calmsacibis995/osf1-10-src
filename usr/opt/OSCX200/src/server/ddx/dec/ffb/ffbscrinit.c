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
static char *rcsid = "@(#)$RCSfile: ffbscrinit.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1994/01/04 17:49:13 $";
#endif
/*
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
#include "mibstore.h"
#include "dix.h"

#include "ffb.h"
#include "cfbscrinit.h"
#include "ffbscrinit.h"
#include "ffbwindow.h"
#include "ffbparams.h"
#include "ffbgc.h"
#include "ffbpntwin.h"
#include "ffbplane.h"
#include "ffbbstore.h"
#include "ffbbitblt.h"
#include "ffbgetsp.h"
#include "ffbpixmap.h"
#include "ffbbuf.h"
#include "ffbpolytext.h"

#include <sys/resource.h>
#ifdef SOFTWARE_MODEL
#include "model/defs.h"
#include "model/Module.h"
#include "ffbvram.h"
#include <stdio.h>
#include <errno.h>
#include <malloc.h>

extern int wsFd;

/*
 * stuff to get started with greater-than-8 depths
 */
#include "../ws/ws.h"

#include "../pxg/px_priv.h"
#include "Graphics_Mem.h"

#ifdef FFB_DEPTH_INDEPENDENT
Bool wrapperInited = FALSE;
int UsePxg = 0;
int setLogFlag;
int disableRootTile;
FILE *	    vramOut;
extern Bool ffb8_InitProc();
extern Bool ffb32_InitProc();
Pixel8 dma_buffer[8192];
#else
extern Bool wrapperInited;
extern Bool UsePxg;
extern FILE *vramOut;
extern int setLogFlag;
extern int disableRootTile;
extern char *pxgNames[];
extern Pixel8 dmaBuffer[];

char *pxgNames[] = {
    "PMAG-CA ", "PMAG-DA ", "PMAG-FA ", NULL
};

/* end stuff to get started with greater-than-8 depths */

History()
{
}

DebugInit()
{
}

#define NOEVENTS 0
int ticks;

Tick( )
{
  OneCycle( NOEVENTS );
  ++ticks;
}

unsigned char epromMem [ROMSIZE]; 
unsigned char paletteRAM [RAMDACS] [PALMASK + 1] [CHANNELS];

ExitSfb(n)
{
    exit(n);
}
#endif /*FFB_DEPTH_INDEPENDENT*/

extern void modelStoreColors();
extern int  logFlag;
extern FILE *cmdOut /* , *vramOut*/;

#endif /*SOFTWARE_MODEL*/

#if FFBPIXELBITS==32 && defined(MCMAP)
#include "../cmap/cmap.h"
#include "../cmap/bt463.h"
#endif

#ifndef VMS
#  ifndef SOFTWARE_MODEL
#    include <sys/types.h>
#    include <sys/workstation.h>
#    include "../ws/ws.h"
#  endif
#endif

#ifdef  FFB_DEPTH_INDEPENDENT
int              defaultVisualClass = -1;
int		 maxsize = 0;
int              ffbScreenPrivateIndex;
int              ffbGCPrivateIndex;
int              ffbWindowPrivateIndex;
unsigned long    ffbGeneration = 0;
#else
extern int       defaultVisualClass;
extern int	 maxsize;
extern unsigned long    ffbGeneration;

#endif

#ifndef VMS
#ifdef FFB_DEPTH_INDEPENDENT

Bool ffbInitProc(index, pScreen, argc, argv)
    int         index;
    ScreenPtr   pScreen;
    int         argc;
    char        **argv;
{
    int i;
    Bool (*func)() = ffb8_InitProc; /* hack for base ffb code */

#ifdef SOFTWARE_MODEL
    for ( i=1; i < argc; i++ ) {
	if (strcmp(argv[i], "-lflag") == 0) {
	    disableRootTile = 1;
	    setLogFlag = 1;
	}
	else if (strcmp(argv[i], "-sfb8") == 0 || strcmp(argv[i], "-ffb8") == 0) {
	    ErrorF("ffbInitProc: force 8-bit mode\n");
	    func = ffb8_InitProc;
	}
	else if (strcmp(argv[i], "-sfbq") == 0 ||
		 strcmp(argv[i], "-ffbq") == 0) {
	    ErrorF("ffbInitProc: disable root tile\n");
	    disableRootTile = 1;
	}
	else if (strcmp(argv[i], "-sfbdebug") == 0 ||
		 strcmp(argv[i], "-ffbdebug") == 0) {
	    extern ffbDebug;
	    ErrorF("ffbInitProc: enable debug\n");
	    ffbDebug = 1;
	}
    }
#endif
    return (*func)(index, pScreen, argc, argv);
}
#endif

extern Bool fbInitProc();
extern Bool (*wsAltScreenInitProc)();

Bool ffb_InitProc(index, pScreen, argc, argv)
    int		index;
    ScreenPtr   pScreen;
    int		argc;
    char	**argv;
{
    int	        i;

    if(screenArgs[index].flags & ARG_CLASS)
        defaultVisualClass = screenArgs[index].class;
                                         
    for ( i = 1; i < argc; i++){
        if (strcmp(argv[i], "-dblsize") == 0) {
	    i++;
	    maxsize = atoi(argv[i]);
	}
    }
 
    wsAltScreenInitProc = ffbScreenInit;
    if (!fbInitProc(index, pScreen, argc, argv))
	return FALSE;
#ifdef SOFTWARE_MODEL
    if (UsePxg){
	/* Initialize color map to linear ramp in all 3 guns */
        model_init_colorramp(pScreen);
    }
#endif

#if 0 /* Changed by USG */
    D_FFBDrawInit(pScreen, index);
#else
    D_GENERICDrawInit(pScreen, index);
#endif
#if 0
    /* for testing purposes with static server */
    DECStereoExtensionInit(argc, argv);
    ffbStereoInitProc(argc, argv);
#endif
    return TRUE;
}
#endif


/*
  The following definitions are taken from cfbscrinit.c and used for
  ffbSetupScreen and ffbFinishScreenInit, which were copied from the
  corresponding cfb routines.
*/

/* If this table is added/subtracted from you must also modify the
 * corresponding NUMVISUALSx values
 */

/*
  What is the reason for duplicate visuals?  GL uses duplicate visuals to distinguish
  between its own visuals with different sets of attributes, such as whether double_b
  or alpha_b is turned on.  Another way to do this would be to have GL expand the
  visuals per screen, creating however many duplicates it wants, and installing it.
 */
static VisualRec visuals[] = {
/* vid  class        bpRGB cmpE nplan rMask     gMask   bMask oRed oGreen oBlue */
    0,  PseudoColor, 8,    256,   8,    0,        0,      0,    0,    0, 0,
    0,  PseudoColor, 8,    256,   8,    0,        0,      0,    0,    0, 0,

    0,  DirectColor, 8,      8,   8, 0xe0,     0x1c,    0x3,    5,    2, 0,
    0,  GrayScale,   8,    256,   8,    0,        0,      0,    0,    0, 0,
    0,  StaticGray,  8,    256,   8,    0,        0,      0,    0,    0, 0,
    0,  StaticColor, 8,    256,   8, 0xe0,     0x1c,    0x3,    5,    2, 0,

    0,  TrueColor,   8,      8,   8, 0xe0,     0x1c,    0x3,    5,    2, 0,
    0,  TrueColor,   8,      8,   8, 0xe0,     0x1c,    0x3,    5,    2, 0,
    0,  TrueColor,   8,      8,   8, 0xe0,     0x1c,    0x3,    5,    2, 0,
    0,  TrueColor,   8,      8,   8, 0xe0,     0x1c,    0x3,    5,    2, 0,

#if FFBPIXELBITS==32
    0,  TrueColor,   8,     16,  12,  0xf00000, 0xf000, 0xf0,  20,   12, 4,
    0,  TrueColor,   8,     16,  12,  0xf00000, 0xf000, 0xf0,  20,   12, 4,
    0,  TrueColor,   8,     16,  12,  0xf00000, 0xf000, 0xf0,  20,   12, 4,
    0,  TrueColor,   8,     16,  12,  0xf00000, 0xf000, 0xf0,  20,   12, 4,

    0,  TrueColor,   8,    256,  24,  0xff0000, 0xff00, 0xff,  16,    8, 0,
    0,  TrueColor,   8,    256,  24,  0xff0000, 0xff00, 0xff,  16,    8, 0,
    0,  TrueColor,   8,    256,  24,  0xff0000, 0xff00, 0xff,  16,    8, 0,
    0,  TrueColor,   8,    256,  24,  0xff0000, 0xff00, 0xff,  16,    8, 0,

    0,  DirectColor, 8,    256,  24,  0xff0000, 0xff00, 0xff,  16,    8, 0,

    0,  PseudoColor, 8,    16,    4,    0,        0,      0,    0,    0, 0,
#endif
};

#define NUMVISUALS8     10

/* this is added for our temporary 12 bit root visual support */
#define TrueColor12 	NUMVISUALS8

#if FFBPIXELBITS == 32
#define NUMVISUALS12    4
#define NUMVISUALS24    5
#define NUMVISUALS4	1
#endif
 
static DepthRec depths[] = {
/* depth	numVid		vids */
     8,		NUMVISUALS8,    NULL,
#if FFBPIXELBITS == 32
    12,		NUMVISUALS12,   NULL,
    24,		NUMVISUALS24,   NULL,
     4,		NUMVISUALS4,	NULL,
#endif
};

#define NUMDEPTHS	((sizeof depths)/(sizeof depths[0]))


static miBSFuncRec ffbBSFuncRec = {
    ffbSaveAreas,
    ffbRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};


/*ARGSUSED*/
Bool ffbCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
#ifdef SOFTWARE_MODEL
    READ_MEMORY_BARRIER();
    MakeIdle();
    if (logFlag) {
	ErrorF("Closing log files\n");
	fclose (cmdOut);
	fclose (vramOut);
	logFlag = FALSE;
    }
    xfree(vram);
#endif

    xfree (FFBSCREENPRIV(pScreen));

#ifdef VMS
    return vmsScreenClose(index,pScreen);
#endif
}


#if FFBPIXELBITS==32
extern void ffb32ClearOverlays(WindowPtr pWin, RegionPtr pRegion);
extern Bool ffb32InitializeColormap(ColormapPtr pMap);
extern void ffbWriteWindowTypeTable(ScreenPtr pScreen, ColormapPtr pMap, long where);
extern void ffb32ClearToBackground(WindowPtr pWin);
#endif


/*
  The following two routines were copied from corresponding cfb routines.
  The only reason for this is that miScreenInit must be called with
  ffbBSFuncRec, which is passed by reference.
*/
Bool
ffbSetupScreen(pScreen)
    register ScreenPtr pScreen;
{
    if (!cfbAllocatePrivates(pScreen, (int *)0L, (int *)0L) )
	return FALSE;
    pScreen->defColormap = FakeClientID(0);
    /* let CreateDefColormap do whatever it wants for pixels */ 
    pScreen->blackPixel = pScreen->whitePixel = (Pixel) 0;
#ifdef VMS
    {
	extern cfbvmsQueryBestSize();
	pScreen->QueryBestSize = cfbvmsQueryBestSize;
    }
#else
    pScreen->QueryBestSize = mfbQueryBestSize;
#endif

    /* Set up ffb routines */
    pScreen->GetImage = ffbGetImage;
    pScreen->GetSpans = ffbGetSpans;
    pScreen->CreateGC = ffbCreateGC;
    pScreen->CreatePixmap = ffbCreatePixmap;
    pScreen->DestroyPixmap = ffbDestroyPixmap;
    pScreen->PaintWindowBackground = ffbPaintWindow;
    pScreen->PaintWindowBorder = ffbPaintWindow;
    pScreen->CopyWindow = ffbCopyWindow;
    pScreen->CloseScreen = ffbCloseScreen;
    pScreen->CreateWindow = ffbCreateWindow;
    pScreen->DestroyWindow = ffbIgnoreWindow;
    pScreen->PositionWindow = ffbIgnoreWindow;
    pScreen->ChangeWindowAttributes = ffbIgnoreWindow;
    pScreen->RealizeWindow = ffbIgnoreWindow;
    pScreen->UnrealizeWindow = ffbIgnoreWindow;

    /* Set up cfb routines */
    pScreen->RealizeFont = ffbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
#if FFBPIXELBITS==32 && defined(MCMAP)
    pScreen->CreateColormap = ffb32InitializeColormap;
#else
    pScreen->CreateColormap = cfbInitializeColormap;
#endif
    pScreen->DestroyColormap = NoopDDA;
    pScreen->ResolveColor = cfbResolveColor;
    pScreen->BitmapToRegion = mfbPixmapToRegion;

    mfbRegisterCopyPlaneProc (pScreen, CFB_NAME(CopyPlane));
    return TRUE;
}


#ifdef VMS
Bool ffbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, 
	bits_per_pixel, depth, ffbInfo, total_bytes)
#else
Bool ffbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, 
	bits_per_pixel, depth, ffbInfo)
#endif
    ScreenPtr 	pScreen;
    pointer 	pbits;			/* pointer to screen bitmap */
    int 	xsize, ysize;		/* in pixels */
    int 	dpix, dpiy;		/* dots per inch */
    int 	width;			/* pixel width of frame buffer */
    int		bits_per_pixel;		/* new with cfbflex */
    int		depth;			/* new with cfbflex */
    FFBInfo 	ffbInfo;		/* Regs ptr, shared planemask, ... */
#ifdef VMS
    int		total_bytes;		/* Total bytes available */
#endif
{
    FFB				ffb;	/* base of command registers */
    ffbScreenPrivPtr 		scrPriv;
    PixmapPtr			pPixmap, ffbCreatePixmap();
    int 			screenSize, memSize, size;
    volatile pointer 		base;
#ifdef SOFTWARE_MODEL
    extern ws_screen_descriptor screenDesc[];
    ws_depth_descriptor 	depthDesc;
#endif
    int 			i,j;
    int				iRootVisual = -1;
    VisualID    		*pVids;
    int 			visual_index = 0;

#ifdef SOFTWARE_MODEL
    /* Set up all parameters as if the driver had done it */
    READ_MEMORY_BARRIER();		/* _really_ a model artifact	    */
 
   /* Diagnostics/driver normally responsible for DEEP reg */
    ffb = (FFB)FIRSTREG_ADDRESS;
#   if FFBPIXELBITS==32 
    FFBDEEP(ffb, (7 << 2) | 3); /* Addr mask 32 MB, 32-bit system */
#   else
    FFBDEEP(ffb, (3 << 2) | 0); /* Addr mask 16 MB, 8-bit system */
#   endif

    width = FFB_SCANLINE_PAD(width);    /* To get 1284-wide frame buffer.   */

#   if FFBPIXELBITS==8
    depth = 8;				/* This is for the PXG.		    */
#   endif

    screen_width = xsize;
    screen_height = ysize;
    phys_screen_width = width;

    /* Set up vram and screen pointer approriate to HX or PXG */
    UsePxg = 0;
    sfbVRAM = pbits - FFBSTARTPIXELS * FFBPIXELBYTES;
    
    for (j = 0; pxgNames[j] != NULL; j++) {
	if (strcmp(screenDesc[pScreen->myNum].moduleID, pxgNames[j]) == 0) {
	    /* we're using pxg as an output device */
	    depthDesc.screen = pScreen->myNum;
	    depthDesc.which_depth = 0;
            if (ioctl(wsFd, GET_DEPTH_INFO, &depthDesc) < 0) {
		ErrorF("GET_DEPTH_INFO failed for PXG\n");
		exit(errno);
	    }
	    UsePxg = 1;
            pxInfoPtr = (pxInfo *)depthDesc.pixmap;
	    break;
	}
    }
    /*
     * malloc enough for 2 Mbytes for an 8 bit system or 8Mbytes for a 
     * 32 bit system. 
     */
    vram_bytes = 0x200000 * (depth == 8 ? 1 : 4);
    vram = (pointer)xalloc(vram_bytes);
    if (vram == (pointer)NULL) {
	ErrorF("malloc failed for vram\n");
	exit(errno);
    }
	    
    /* Driver starts screen at FB base address + 4KP offset */
    pbits = (pointer)(FFBMODEL_VRAMBASE + FFBSTARTPIXELS * FFBPIXELBYTES);

    ffbInfo = (FFBInfo)xalloc(sizeof(FFBInfoRec));
    ffbInfo->fb_alias_increment = (vm_offset_t) CYCLE_FB_INC;
    ffbInfo->option_base = (vm_offset_t) 0;
    ffbInfo->virtual_dma_buffer = (vm_offset_t) dma_buffer;
    ffbInfo->physical_dma_buffer = (vm_offset_t) dma_buffer;

    /* Start up model and logging if desired */
    if (!wrapperInited) {
	wrapperInited = TRUE;
	WrapperInit();
    }

    if (setLogFlag) {
	logFlag = TRUE;
	setLogFlag = FALSE;
    }

    if (logFlag) {
	ErrorF("Opening log files\n");
	cmdOut = popen("compress -c > ffbcmds.log.Z", "w");
	if (cmdOut == (FILE *)NULL) {
	    ErrorF("can't open ffbcmds.log.Z\n");
	    exit (1);
	}
	vramOut = popen("compress -c > ffbvram.log.Z", "w");
	if (vramOut == (FILE *)NULL) {
	    ErrorF("can't open ffbvram.log.Z\n");
	    exit (1);
	}
	/* Specify how many bytes of frame buffer memory to simulate */
	fprintf(cmdOut, "3 %x\n\n", vram_bytes);
    }

#endif /* SOFTWARE_MODEL */

    /* Set up data from ffbInfo */
    if (ffbInfo->fb_alias_increment != (vm_offset_t) CYCLE_FB_INC) {
	FatalError(
	    "Server alias increment does not match driver increment\n");
    }
    ffb = (FFB)(((Bits8 *)ffbInfo->option_base) + FIRSTREG_ADDRESS);

#ifndef SOFTWARE_MODEL
{
    unsigned int horiz;
    horiz = ffb->horiz_ctl;
    horiz |= ((unsigned)1 << 31);     /* Disable last 4 pixel display */
    /* ||| width is wrong from driver if ROM sets up 1284 as of 19 Aug 93 */
    width = (horiz & 0x1ff) * 4;
    if (width != FFB_SCANLINE_PAD(width)) {
	/* Make sure that server has what it really wants. */
	ErrorF("Server scanline padding disagrees with ROM, adjusting\n");
	if (width % 8 == 0) {
	    /* Hardware is 1280, e.g. but we want 1284 */
	    horiz -= (1 << 21);     /* Back porch - 1 */
	    horiz += 1;		    /* Active + 1 */
	} else {
	    /* Hardware is 1284 e.g. but we want 1280 */
	    horiz += (1 << 21);
	    horiz -= 1;
	}
	width = FFB_SCANLINE_PAD(width);
    }
    ffb->horiz_ctl = horiz;
}
#endif

#if defined(SOFTWARE_MODEL) || !defined(VMS)
    if (!ffbInitDMA()) return FALSE;
#endif

#ifndef VMS
    /* Let's try to give the server a higher priority than applications */
    {
	int result;
	extern int errno;
	result = setpriority(PRIO_PROCESS, getpid(), -2);
	if (result != 0)
	    ErrorF("Couldn't reset priority, errno = %d\n", errno);
    }
#endif

    if (depth == 24) {
        if (defaultVisualClass == -1)
            defaultVisualClass = TrueColor;
        switch (defaultVisualClass) {
         case PseudoColor:
            iRootVisual = 0;
            break;
	 case GrayScale:
            iRootVisual = 2;
            break;
	 case StaticGray:
            iRootVisual = 3;
            break;
	 case StaticColor:
            iRootVisual = 4;
            break;
	 case TrueColor12:
            iRootVisual = 6;
            break;
	 case TrueColor:
            iRootVisual = 7;
            break;
	 case DirectColor:
            iRootVisual = 8;
            break;
	 default:
            ErrorF("ffbScreenInit: Illegal root visual for 24 planes\n");
            return(FALSE);
            break;
        }				/* end switch */

    } else if (depth == 8) {
	if (defaultVisualClass == -1)
            defaultVisualClass = PseudoColor;
	switch (defaultVisualClass) {
	 case PseudoColor:
            iRootVisual = 0;
            break;
	 case DirectColor:
            iRootVisual = 1;
            break;
	 case GrayScale:
            iRootVisual = 2;
            break;
	 case StaticGray:
            iRootVisual = 3;
            break;
	 case StaticColor:
            iRootVisual = 4;
            break;
	 case TrueColor:
            iRootVisual = 5;
            break;
	 default:
            ErrorF("ffbScreenInit: Illegal root visual for 8 planes \n");
            return(FALSE);
            break;
        }				/* end switch */
    }					/* end if depth */


   
    depth = visuals[iRootVisual].nplanes;

#ifndef MCMAP
    /* |||
     * it's kind of dangerous to put this before miScreenInit(), since the latter
     * may overwrite pScreen->func pointers that are set up in ffbSetupScreen().
     * hence, cmapSetupScreen() is called after miScreenInit().
     * |||
     * This is left here, so as not to change anything before 8-plane ships 11/93.
     */
    if (!ffbSetupScreen(pScreen))
	return FALSE;
#endif

    for (i = 0; i < NUMDEPTHS; i++) {
	if (depths[i].vids == NULL) {
	    /* should only be done once! */
	    depths[i].vids = pVids = (VisualID *) xalloc(sizeof (VisualID) *
							 depths[i].numVids);
	    for (j = 0; j < depths[i].numVids; j++) {
		pVids[j] = visuals[j+visual_index].vid = FakeClientID(0);
	    }
	}
	visual_index += depths[i].numVids;
    }  
    if (!miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
		     depth, NUMDEPTHS, depths,
		     visuals[iRootVisual].vid,
		     visual_index,
		     visuals, &ffbBSFuncRec))
	return FALSE;

#ifdef MCMAP
    /* |||
     * it's kind of dangerous to put this before miScreenInit(), since the latter
     * may overwrite pScreen->func pointers that are set up in ffbSetupScreen().
     * hence, cmapSetupScreen() is called after miScreenInit().
     */
    if (!ffbSetupScreen(pScreen))
	return FALSE;

#if FFBPIXELBITS==32
    /* Set up multiple-colormap functions, rappers, etc */
    if (cmapSetupScreen(pScreen,
			ffbWriteWindowTypeTable,
			ffb32PaintWindowID,
			bt463LoadColormap,
			cmapExpandDirectColors,
			ffb32ClearOverlays,
			ffb32ClearToBackground,
			bt463OverlayVisualIndex,
			bt463CmapTypeToIndex,
			bt463IsOverlayWindow) == FALSE)
    {
	return FALSE;
    }

    /* don't need to preload the window type table, since driver will do that
     * for us; only when we allocate additional window types do we need to
     * update the bt463.  but, do need to create colormaps for all the freebie
     * types provided by the bt463...
     */
    {
	int vi;
	ColormapPtr pMap;

	vi = cmapGetVisualIndex(pScreen, TrueColor, 24);
	if (pScreen->visuals[vi].vid != pScreen->rootVisual)
	{
	    int cmapId = FakeClientID(0);

	    CreateColormap(cmapId,
			   pScreen,
			   &(pScreen->visuals[vi]),
			   &pMap,
			   AllocAll,
			   0);
	
	    (* pScreen->InstallColormap)(pMap);
	}

	vi = cmapGetVisualIndex(pScreen, TrueColor, 12);
	if (pScreen->visuals[vi].vid != pScreen->rootVisual)
	{
	    int cmapId = FakeClientID(0);

	    CreateColormap(cmapId,
			   pScreen,
			   &(pScreen->visuals[vi]),
			   &pMap,
			   AllocAll,
			   0);
	
	    (* pScreen->InstallColormap)(pMap);
	}

	vi = cmapGetVisualIndex(pScreen, StaticGray, 8);
	if (pScreen->visuals[vi].vid != pScreen->rootVisual)
	{
	    int cmapId = FakeClientID(0);

	    CreateColormap(cmapId,
			   pScreen,
			   &(pScreen->visuals[vi]),
			   &pMap,
			   AllocAll,
			   0);
	
	    (* pScreen->InstallColormap)(pMap);
	}
    }
#endif /*32*/
#endif /*mcmap*/

/* overwrite the values computed by miScreenInit */

    ((PixmapPtr)pScreen->devPrivate)->drawable.bitsPerPixel = FFBPIXELBITS;
    ((PixmapPtr)pScreen->devPrivate)->devKind = width * FFBPIXELBYTES;


#ifdef SOFTWARE_MODEL
    if (UsePxg) {
	pScreen->StoreColors = modelStoreColors;
    }
#endif

#ifdef DPS
    {/* XXX temp crock until server can do per-screen extension init */
	extern void CFB_NAME(CreateDDXMarkProcs)();
	XMIRegisterDDXMarkProcsProcs(pScreen, CFB_NAME(CreateDDXMarkProcs));
    }
#endif DPS

    if (ffbGeneration != serverGeneration) {
	ffbScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (ffbScreenPrivateIndex < 0)
	    return FALSE;
	ffbGCPrivateIndex = AllocateGCPrivateIndex();
	ffbWindowPrivateIndex = AllocateWindowPrivateIndex();
	ffbGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, ffbGCPrivateIndex, sizeof(ffbGCPrivRec))) {
        return FALSE;
    }
    if (!AllocateWindowPrivate(pScreen, ffbWindowPrivateIndex,
                               sizeof(ffbBufferDescriptor))) {
        return FALSE;
    }

    scrPriv = (ffbScreenPrivPtr) xalloc (sizeof (ffbScreenPrivRec));
    if (!scrPriv)
	return FALSE;
    pScreen->devPrivates[ffbScreenPrivateIndex].ptr = (pointer) scrPriv;
    scrPriv->ffb = ffb;
    scrPriv->info = ffbInfo;
    scrPriv->fbdepth = FFBPIXELBITS;


#ifdef SOFTWARE_MODEL
    base = vram;
    memSize = vram_bytes;

#else

#   ifdef VMS
/* ||| Is VMS driver going to pass in total_bytes?  Why can't we just use the
   same mechanism as OSF?  Note that total_bytes presumably is already
   decremented by the FFBSTARTPIXELS * FFBPIXELBYTES area. */
    base = ((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr;
    memSize = total_bytes;
#   else
    base = ((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr
	- (FFBSTARTPIXELS * FFBPIXELBYTES);
    /* Find out how much ffb memory we have from Deep register mask field.
	000 ->  2 megabytes
	001 ->  4 megabytes
	011 ->  8 megabytes
	111 -> 16 megabytes
    */
    READ_MEMORY_BARRIER();
    memSize = 0x200000 * (((FFBDEEPREAD(ffb) >> 2) & 7) + 1);
#   endif /*vms*/
#endif /*s/w model*/

/*

Set up offscreen memory free list.  We have FFBSTARTPIXELS at the beginning,
but can't use the first FFBCOPYSHIFTBYTES bytes.  (Copies to a pixmap can back
up the destination pointer by FFBCOPYSHIFTBYTES.)

(32-bit systems use the very first 1024 bytes of the FFBSTARTPIXELS area for
the cursor, so we don't want to free that part up.)

(VMS uses the space prior to the onscreen memory for the console window; we
cannot count on using this space, although some is available for scratch (like
when we move a window off the screen).

We can't have the next ysize * width pixels, as those are visible on the
screen.

Finally, we have all the memory left past the screen.

*/

    scrPriv->avail.size = 0;
    scrPriv->avail.next = (FreeElement *)NULL;
    scrPriv->avail.addr = (Pixel8 *)NULL;
    scrPriv->firstScreenMem = base;
    scrPriv->lastScreenMem = base + memSize;

#ifndef VMS
#   if FFBPIXELBITS == 8
    ffbScreenFree(pScreen, base + FFBCOPYSHIFTBYTES,
	FFBSTARTPIXELS*FFBPIXELBYTES - FFBCOPYSHIFTBYTES);
#   else
    ffbScreenFree(pScreen, base + FFBCURSORBYTES,
	FFBSTARTPIXELS*FFBPIXELBYTES - FFBCURSORBYTES);
#   endif
#endif

    screenSize = ysize * width * FFBPIXELBYTES; 
    base = ((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr + screenSize;
#ifdef VMS
    /* memSize from total_bytes, which didn't include VRAM before screen */
    size = memSize - screenSize;
#else
    size = memSize - screenSize - FFBSTARTPIXELS*FFBPIXELBYTES;
#endif

    ffbScreenFree(pScreen, base, size);

    scrPriv->lastGC = (GCPtr)NULL;
    
    pScreen->CloseScreen = ffbCloseScreen;

    /* 
     * The screen pixmap allocated above in the call to miScreenInit()
     * doesn't have the ffbplus-specific wrapper data structure.
     * Make an ffbplus-specific pixmap here (now that the pixmap
     * allocator has been initialized); copy the screen data into
     * the new pixmap record; and deallocate the old pixmap record.
     */
    pPixmap = ffbCreatePixmap(pScreen, 0, 0, depth);
    if (!pPixmap) {
	return FALSE;
    }
    bcopy((char *)(pScreen->devPrivate), (char *)pPixmap, sizeof(PixmapRec));
    xfree(pScreen->devPrivate);
    pScreen->devPrivate = (pointer)pPixmap;
#if FFBPIXELBITS==32
    /* the pixmap allocator always allocates packed pixmaps, and fills out
       the ffb-private buffer descriptor accordingly.  But this may be an
       8-bit screen on a 32-bit framebuffer.  So check for this. */
    if (depth == 8) {
        ffbBufDPtr  bdptr;
        bdptr = FFBBUFDESCRIPTOR(pPixmap);
	bdptr->visualDst = UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT;
	bdptr->visualSrc = UNPACKED_EIGHT_SRC << SRC_VISUAL_SHIFT;
    }
#endif
    /* ||| Not until we do fast 8-1 CopyPlane code (NOT the same as 8-1-8, which
       we should do too!) in cfb and ffb.
       mfbRegisterCopyPlaneProc (pScreen, ffbCopyPlane);
       */

#if FFBPIXELBITS==32 && !defined(SOFTWARE_MODEL)
    /*
     * finally, clear the frame buffer's overlay bits... inlined from
     * ClearOverlays()
     */
    {
	long dstwidth, mask, h;
	Pixel8 *pdst;

	WRITE_MEMORY_BARRIER();
	FFBMODE(ffb, BLOCKFILL);
	FFBROP(ffb, GXcopy, ROTATE_DESTINATION_3 << DST_ROTATE_SHIFT,
	       UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
	FFBDATA(ffb, ~0);
	FFBLOADCOLORREGS(ffb, 0, FFBPIXELBITS);
	CYCLE_REGS(ffb);
	FFBPLANEMASK(scrPriv, ffb, 0x0f000000);
	pdst = (Pixel8 *) (((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr);
	dstwidth =(long) (((PixmapPtr)(pScreen->devPrivate))->devKind);
	mask = FFBLOADBLOCKDATA(0, 1284/*|||*/);
	for ( h=1024; h != 0; h-- ) {
	    FFBWRITE(pdst, mask);
	    pdst += dstwidth;
	}
    }
#endif

    return TRUE;
}


/*
 * HISTORY
 */
