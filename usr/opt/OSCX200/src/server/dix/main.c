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
/* $XConsortium: main.c,v 5.20 92/08/21 19:29:46 rws Exp $ */

#include "X.h"
#include "Xproto.h"
#include "input.h"
#include "scrnintstr.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "extension.h"
#include "colormap.h"
#include "cursorstr.h"
#include "font.h"
#include "opaque.h"
#include "servermd.h"
#include "site.h"
#include <stdio.h>

#if LONG_BIT == 64
extern int defaultScreenSaverTime;
extern int defaultScreenSaverInterval;
#else /* LONG_BIT == 32 */
extern long defaultScreenSaverTime;
extern long defaultScreenSaverInterval;
#endif /* LONG_BIT */
extern int defaultScreenSaverBlanking;
extern int defaultScreenSaverAllowExposures;

void ddxGiveUp();

extern char *display;
char *ConnectionInfo;
xConnSetupPrefix connSetupPrefix;

extern WindowPtr *WindowTable;
extern FontPtr defaultFont;

extern void SetInputCheck();
extern void InitProcVectors();
extern void InitEvents();
extern void InitExtensions();
extern void DefineInitialRootWindow();
extern void QueryMinMaxKeyCodes();
extern Bool InitClientResources();
static Bool CreateConnectionBlock();
extern Bool CreateGCperDepthArray();
extern Bool CreateDefaultStipple();
extern void ResetWellKnownSockets();
extern void ResetWindowPrivates();
extern void ResetGCPrivates();
static void FreeScreen();
static void ResetScreenPrivates();

PaddingInfo PixmapWidthPaddingInfo[33]; /* supports depth up to 32 */
#if LONG_BIT == 64 && BITMAP_SCANLINE_PAD == 64
/* add padding info for 32-bit interface. PutImage and GetImage will
 * work on 32-bit padding while the rest of the server will work
 * on 64-bit padding.
 */
PaddingInfo PixmapWidthPaddingInfoProto[33];
#endif /* LONG_BIT */
int connBlockScreenStart;

static int restart = 0;

void
NotImplemented()
{
    FatalError("Not implemented");
}

/*
 * This array encodes the answer to the question "what is the log base 2
 * of the number of pixels that fit in a scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
#if LONG_BIT == 32 
static int answer[6][3] = {
	/* pad   pad   pad */
	/*  8     16    32 */

	{   3,     4,    5 },	/* 1 bit per pixel */
	{   1,     2,    3 },	/* 4 bits per pixel */
	{   0,     1,    2 },	/* 8 bits per pixel */
	{   ~0,    0,    1 },	/* 16 bits per pixel */
	{   ~0,    ~0,   0 },	/* 24 bits per pixel */
	{   ~0,    ~0,   0 }	/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the first index for
 * the answer array above given the number of bits per pixel?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForBitsPerPixel[ 33 ] = {
	~0, 0, ~0, ~0,	/* 1 bit per pixel */
	1, ~0, ~0, ~0,	/* 4 bits per pixel */
	2, ~0, ~0, ~0,	/* 8 bits per pixel */
	~0,~0, ~0, ~0,
	3, ~0, ~0, ~0,	/* 16 bits per pixel */
	~0,~0, ~0, ~0,
	4, ~0, ~0, ~0,	/* 24 bits per pixel */
	~0,~0, ~0, ~0,
	5		/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the second index for
 * the answer array above given the number of bits per scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForScanlinePad[ 33 ] = {
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	0,  ~0, ~0, ~0,	/* 8 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	1,  ~0, ~0, ~0,	/* 16 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	2		/* 32 bits per scanline pad unit */
};
#else /* LONG_BIT */
static int answer[6][4] = {
	/* pad   pad   pad     pad*/
	/*  8     16    32    64 */

	{   3,     4,    5 ,   6 },	/* 1 bit per pixel */
	{   1,     2,    3 ,   4 },	/* 4 bits per pixel */
	{   0,     1,    2 ,   3 },	/* 8 bits per pixel */
	{   ~0,    0,    1 ,   2 },	/* 16 bits per pixel */
	{   ~0,    ~0,   0 ,   1 },	/* 24 bits per pixel */
	{   ~0,    ~0,   0 ,   1 }	/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the first index for
 * the answer array above given the number of bits per pixel?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForBitsPerPixel[ 33 ] = {
	~0, 0, ~0, ~0,	/* 1 bit per pixel */
	1, ~0, ~0, ~0,	/* 4 bits per pixel */
	2, ~0, ~0, ~0,	/* 8 bits per pixel */
	~0,~0, ~0, ~0,
	3, ~0, ~0, ~0,	/* 16 bits per pixel */
	~0,~0, ~0, ~0,
	4, ~0, ~0, ~0,	/* 24 bits per pixel */
	~0,~0, ~0, ~0,
	5		/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the second index for
 * the answer array above given the number of bits per scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForScanlinePad[ 65 ] = {
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	 0, ~0, ~0, ~0,	/* 8 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	 1, ~0, ~0, ~0,	/* 16 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	 2, ~0, ~0, ~0,	/* 32 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	 3		/* 64 bits per scanline pad unit */
};
#endif /* LONG_BIT */

dix_main(argc, argv)
    int		argc;
    char	*argv[];
{
    int		i, j, k;
#ifdef __alpha 
    int		alwaysCheckForInput[2];
#else
    long	alwaysCheckForInput[2];
#endif

    /* Notice if we're restart.  Probably this is because we jumped through
     * uninitialized pointer */
    if (restart)
	FatalError("server restarted. Jumped through uninitialized pointer?\n");
    else
	restart = 1;
    /* These are needed by some routines which are called from interrupt
     * handlers, thus have no direct calling path back to main and thus
     * can't be passed argc, argv as parameters */
    argcGlobal = argc;
    argvGlobal = argv;
    display = "0";
    ProcessCommandLine(argc, argv);

    alwaysCheckForInput[0] = 0;
    alwaysCheckForInput[1] = 1;
    while(1)
    {
	serverGeneration++;
        ScreenSaverTime = defaultScreenSaverTime;
	ScreenSaverInterval = defaultScreenSaverInterval;
	ScreenSaverBlanking = defaultScreenSaverBlanking;
	ScreenSaverAllowExposures = defaultScreenSaverAllowExposures;
	InitBlockAndWakeupHandlers();
	/* Perform any operating system dependent initializations you'd like */
	OsInit();		
	if(serverGeneration == 1)
	{
	    CreateWellKnownSockets();
	    InitProcVectors();
	    clients = (ClientPtr *)xalloc(MAXCLIENTS * sizeof(ClientPtr));
	    if (!clients)
		FatalError("couldn't create client array");
	    for (i=1; i<MAXCLIENTS; i++) 
		clients[i] = NullClient;
	    serverClient = (ClientPtr)xalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
	    InitClient(serverClient, 0, (pointer)NULL);
	}
	else
	    ResetWellKnownSockets ();
        clients[0] = serverClient;
        currentMaxClients = 1;

	if (!InitClientResources(serverClient))      /* for root resources */
	    FatalError("couldn't init server resources");

	SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);
	screenInfo.arraySize = MAXSCREENS;
	screenInfo.numScreens = 0;
	WindowTable = (WindowPtr *)xalloc(MAXSCREENS * sizeof(WindowPtr));
	if (!WindowTable)
	    FatalError("couldn't create root window table");

	/*
	 * Just in case the ddx doesnt supply a format for depth 1 (like qvss).
	 */
	j = indexForBitsPerPixel[ 1 ];
	k = indexForScanlinePad[ BITMAP_SCANLINE_PAD ];
	PixmapWidthPaddingInfo[1].padRoundUp = BITMAP_SCANLINE_PAD-1;
	PixmapWidthPaddingInfo[1].padPixelsLog2 = answer[j][k];
 	j = indexForBitsPerPixel[8]; /* bits per byte */
 	PixmapWidthPaddingInfo[1].padBytesLog2 = answer[j][k];

#if LONG_BIT == 64 && BITMAP_SCANLINE_PAD == 64
	/* Fake out protocol interface to make them believe
	 * we support padding of 32 instead of 64.
	 */
	j = indexForBitsPerPixel[ 1 ];
	k = indexForScanlinePad[ BITMAP_SCANLINE_PAD_PROTO ];
	PixmapWidthPaddingInfoProto[1].padRoundUp = BITMAP_SCANLINE_PAD_PROTO-1;
	PixmapWidthPaddingInfoProto[1].padPixelsLog2 = answer[j][k];
 	j = indexForBitsPerPixel[8]; /* bits per byte */
 	PixmapWidthPaddingInfoProto[1].padBytesLog2 = answer[j][k];
#endif /* LONG_BIT */

	InitAtoms();
	InitEvents();
	ResetScreenPrivates();
	ResetWindowPrivates();
	ResetGCPrivates();
	ResetFontPrivateIndex();
	InitOutput(&screenInfo, argc, argv);
	if (screenInfo.numScreens < 1)
	    FatalError("no screens found");
	InitExtensions(argc, argv);
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    if (!CreateGCperDepth(i))
		FatalError("failed to create scratch GCs");
	    if (!CreateDefaultStipple(i))
		FatalError("failed to create default stipple");
	    if (!CreateRootWindow(screenInfo.screens[i]))
		FatalError("failed to create root window");
	}
	InitInput(argc, argv);
	if (InitAndStartDevices() != Success)
	    FatalError("failed to initialize core devices");

	if(serverGeneration == 1)
  	    DisplayUseMsg(argc, argv);

	InitFonts ();
	if (SetDefaultFontPath(defaultFontPath) != Success)
	    ErrorF("failed to set default font path '%s'", defaultFontPath);
	if (!SetDefaultFont(defaultTextFont))
	    FatalError("could not open default font '%s'", defaultTextFont);
	if (!(rootCursor = CreateRootCursor(defaultCursorFont, 0)))
	    FatalError("could not open default cursor font '%s'",
		       defaultCursorFont);
	for (i = screenInfo.numScreens - 1; i >= 0; i--) 
	{
	    InitRootWindow(WindowTable[i]);
	    DefineInitialRootWindow(WindowTable[i]);
	}

	if (!CreateConnectionBlock())
	    FatalError("could not create connection block info");
	Dispatch();
	/* Now free up whatever must be freed */
	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
	CloseDownExtensions();
	FreeAllResources();
	CloseDownDevices();
	for (i = screenInfo.numScreens - 1; i >= 0; i--)
	{
	    FreeGCperDepth(i);
	    FreeDefaultStipple(i);
	    (* screenInfo.screens[i]->CloseScreen)(i, screenInfo.screens[i]);
	    FreeScreen(screenInfo.screens[i]);
	    screenInfo.numScreens = i;
	}
	xfree(WindowTable);

	FreeFonts ();

	LS_FreeMarkedLibraries();

	if (dispatchException & DE_TERMINATE)
	{
	    ddxGiveUp();
	    break;
	}

	xfree(ConnectionInfo);
    }
    exit(0);
}

static int padlength[4] = {0, 3, 2, 1};

static Bool
CreateConnectionBlock()
{
    xConnSetup setup;
    xWindowRoot root;
    xDepth	depth;
    xVisualType visual;
    xPixmapFormat format;
    unsigned long vid;
    int i, j, k,
        lenofblock,
        sizesofar = 0;
    char *pBuf;

    
    /* Leave off the ridBase and ridMask, these must be sent with 
       connection */

    setup.release = VENDOR_RELEASE;
    /*
     * per-server image and bitmap parameters are defined in Xmd.h
     */
    setup.imageByteOrder = screenInfo.imageByteOrder;
#if LONG_BIT == 64 && BITMAP_SCANLINE_PAD == 64
    if ( screenInfo.bitmapScanlineUnit > 32 )
    	setup.bitmapScanlineUnit  = 32;
    else
#endif 
    	setup.bitmapScanlineUnit  = screenInfo.bitmapScanlineUnit;
#if LONG_BIT == 64 && BITMAP_SCANLINE_PAD == 64
    if ( screenInfo.bitmapScanlinePad > 32 )
    	setup.bitmapScanlinePad = 32;
    else
#endif 
    	setup.bitmapScanlinePad = screenInfo.bitmapScanlinePad;
    setup.bitmapBitOrder = screenInfo.bitmapBitOrder;
    setup.motionBufferSize = NumMotionEvents();
    setup.numRoots = screenInfo.numScreens;
    setup.nbytesVendor = strlen(VENDOR_STRING); 
    setup.numFormats = screenInfo.numPixmapFormats;
    setup.maxRequestSize = MAX_REQUEST_SIZE;
    QueryMinMaxKeyCodes(&setup.minKeyCode, &setup.maxKeyCode);
    lenofblock = sizeof(xConnSetup) + 
            ((setup.nbytesVendor + 3) & ~3) +
	    (setup.numFormats * sizeof(xPixmapFormat)) +
            (setup.numRoots * sizeof(xWindowRoot));
    ConnectionInfo = (char *) xalloc(lenofblock);
    if (!ConnectionInfo)
	return FALSE;

    bcopy((char *)&setup, ConnectionInfo, sizeof(xConnSetup));
    sizesofar = sizeof(xConnSetup);
    pBuf = ConnectionInfo + sizeof(xConnSetup);

    bcopy(VENDOR_STRING, pBuf, (int)setup.nbytesVendor);
    sizesofar += setup.nbytesVendor;
    pBuf += setup.nbytesVendor;
    i = padlength[setup.nbytesVendor & 3];
    sizesofar += i;
    while (--i >= 0)
        *pBuf++ = 0;
    
    for (i=0; i<screenInfo.numPixmapFormats; i++)
    {
	format.depth = screenInfo.formats[i].depth;
	format.bitsPerPixel = screenInfo.formats[i].bitsPerPixel;
	if ( screenInfo.formats[i].scanlinePad > 32 )
	    format.scanLinePad = 32;
	else
	    format.scanLinePad = screenInfo.formats[i].scanlinePad;
	bcopy((char *)&format, pBuf, sizeof(xPixmapFormat));
	pBuf += sizeof(xPixmapFormat);
	sizesofar += sizeof(xPixmapFormat);
    }

    connBlockScreenStart = sizesofar;
    for (i=0; i<screenInfo.numScreens; i++) 
    {
	ScreenPtr	pScreen;
	DepthPtr	pDepth;
	VisualPtr	pVisual;

	pScreen = screenInfo.screens[i];
	root.windowId = WindowTable[i]->drawable.id;
	root.defaultColormap = pScreen->defColormap;
	root.whitePixel = pScreen->whitePixel;
	root.blackPixel = pScreen->blackPixel;
	root.currentInputMask = 0;    /* filled in when sent */
	root.pixWidth = pScreen->width;
	root.pixHeight = pScreen->height;
	root.mmWidth = pScreen->mmWidth;
	root.mmHeight = pScreen->mmHeight;
	root.minInstalledMaps = pScreen->minInstalledCmaps;
	root.maxInstalledMaps = pScreen->maxInstalledCmaps; 
	root.rootVisualID = pScreen->rootVisual;		
	root.backingStore = pScreen->backingStoreSupport;
	root.saveUnders = pScreen->saveUnderSupport != NotUseful;
	root.rootDepth = pScreen->rootDepth;
	root.nDepths = pScreen->numDepths;
	bcopy((char *)&root, pBuf, sizeof(xWindowRoot));
	sizesofar += sizeof(xWindowRoot);
	pBuf += sizeof(xWindowRoot);

	pDepth = pScreen->allowedDepths;
	for(j = 0; j < pScreen->numDepths; j++, pDepth++)
	{
	    lenofblock += sizeof(xDepth) + 
		    (pDepth->numVids * sizeof(xVisualType));
	    pBuf = (char *)xrealloc(ConnectionInfo, lenofblock);
	    if (!pBuf)
	    {
		xfree(ConnectionInfo);
		return FALSE;
	    }
	    ConnectionInfo = pBuf;
	    pBuf += sizesofar;            
	    depth.depth = pDepth->depth;
	    depth.nVisuals = pDepth->numVids;
	    bcopy((char *)&depth, pBuf, sizeof(xDepth));
	    pBuf += sizeof(xDepth);
	    sizesofar += sizeof(xDepth);
	    for(k = 0; k < pDepth->numVids; k++)
	    {
		vid = pDepth->vids[k];
		for (pVisual = pScreen->visuals;
		     pVisual->vid != vid;
		     pVisual++)
		    ;
		visual.visualID = vid;
		visual.class = pVisual->class;
		visual.bitsPerRGB = pVisual->bitsPerRGBValue;
		visual.colormapEntries = pVisual->ColormapEntries;
		visual.redMask = pVisual->redMask;
		visual.greenMask = pVisual->greenMask;
		visual.blueMask = pVisual->blueMask;
		bcopy((char *)&visual, pBuf, sizeof(xVisualType));
		pBuf += sizeof(xVisualType);
		sizesofar += sizeof(xVisualType);
	    }
	}
    }
    connSetupPrefix.success = xTrue;
    connSetupPrefix.length = lenofblock/4;
    connSetupPrefix.majorVersion = X_PROTOCOL;
    connSetupPrefix.minorVersion = X_PROTOCOL_REVISION;
    return TRUE;
}

static int  screenPrivateCount;

static void
ResetScreenPrivates()
{
    screenPrivateCount = 0;
}

/* this can be called after some screens have been created,
 * so we have to worry about resizing existing devPrivates
 */
int
AllocateScreenPrivateIndex()
{
    int		index;
    int		i;
    ScreenPtr	pScreen;
    DevUnion	*nprivs;

    index = screenPrivateCount++;
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	nprivs = (DevUnion *)xrealloc(pScreen->devPrivates,
				      screenPrivateCount * sizeof(DevUnion));
	if (!nprivs)
	{
	    screenPrivateCount--;
	    return -1;
	}
	pScreen->devPrivates = nprivs;
    }
    return index;
}

Bool
AllocateWindowPrivate(pScreen, index, amount)
    register ScreenPtr pScreen;
    int index;
    unsigned amount;
{
    unsigned oldamount;

    if (index >= pScreen->WindowPrivateLen)
    {
	unsigned *nsizes;
	nsizes = (unsigned *)xrealloc(pScreen->WindowPrivateSizes,
				      (index + 1) * sizeof(unsigned));
	if (!nsizes)
	    return FALSE;
	while (pScreen->WindowPrivateLen <= index)
	{
	    nsizes[pScreen->WindowPrivateLen++] = 0;
	    pScreen->totalWindowSize += sizeof(DevUnion);
	}
	pScreen->WindowPrivateSizes = nsizes;
    }
    oldamount = pScreen->WindowPrivateSizes[index];
    if (amount > oldamount)
    {
	pScreen->WindowPrivateSizes[index] = amount;
	pScreen->totalWindowSize += (amount - oldamount);
    }
    return TRUE;
}

Bool
AllocateGCPrivate(pScreen, index, amount)
    register ScreenPtr pScreen;
    int index;
    unsigned amount;
{
    unsigned oldamount;

    if (index >= pScreen->GCPrivateLen)
    {
	unsigned *nsizes;
	nsizes = (unsigned *)xrealloc(pScreen->GCPrivateSizes,
				      (index + 1) * sizeof(unsigned));
	if (!nsizes)
	    return FALSE;
	while (pScreen->GCPrivateLen <= index)
	{
	    nsizes[pScreen->GCPrivateLen++] = 0;
	    pScreen->totalGCSize += sizeof(DevUnion);
	}
	pScreen->GCPrivateSizes = nsizes;
    }
    oldamount = pScreen->GCPrivateSizes[index];
    if (amount > oldamount)
    {
	pScreen->GCPrivateSizes[index] = amount;
	pScreen->totalGCSize += (amount - oldamount);
    }
    return TRUE;
}

/*
	grow the array of screenRecs if necessary.
	call the device-supplied initialization procedure 
with its screen number, a pointer to its ScreenRec, argc, and argv.
	return the number of successfully installed screens.

*/

int
AddScreen(pfnInit, argc, argv)
    Bool	(* pfnInit)();
    int argc;
    char **argv;
{

    int i;
    int scanlinepad, format, depth, bitsPerPixel, j, k;
    ScreenPtr pScreen;
#ifdef DEBUG
    void	(**jNI) ();
#endif /* DEBUG */

    i = screenInfo.numScreens;
    if (i == MAXSCREENS)
	return -1;

    pScreen = (ScreenPtr) xalloc(sizeof(ScreenRec));
    if (!pScreen)
	return -1;

    bzero(pScreen, sizeof(ScreenRec));

    pScreen->devPrivates = (DevUnion *)xalloc(screenPrivateCount *
					      sizeof(DevUnion));
    if (!pScreen->devPrivates && screenPrivateCount)
    {
	xfree(pScreen);
	return -1;
    }

    bzero(pScreen->devPrivates, screenPrivateCount * sizeof(DevUnion));

    pScreen->myNum = i;
    pScreen->WindowPrivateLen = 0;
    pScreen->WindowPrivateSizes = (unsigned *)NULL;
    pScreen->totalWindowSize = sizeof(WindowRec);
    pScreen->GCPrivateLen = 0;
    pScreen->GCPrivateSizes = (unsigned *)NULL;
    pScreen->totalGCSize = sizeof(GC);
    pScreen->ClipNotify = (void (*)())NULL; /* for R4 ddx compatibility */
    
#ifdef DEBUG
    for (jNI = &pScreen->QueryBestSize; 
	 jNI < (void (**) ()) &pScreen->SendGraphicsExpose;
	 jNI++)
	*jNI = NotImplemented;
#endif /* DEBUG */

    /*
     * This loop gets run once for every Screen that gets added,
     * but thats ok.  If the ddx layer initializes the formats
     * one at a time calling AddScreen() after each, then each
     * iteration will make it a little more accurate.  Worst case
     * we do this loop N * numPixmapFormats where N is # of screens.
     * Anyway, this must be called after InitOutput and before the
     * screen init routine is called.
     */
    for (format=0; format<screenInfo.numPixmapFormats; format++)
    {
 	depth = screenInfo.formats[format].depth;
 	bitsPerPixel = screenInfo.formats[format].bitsPerPixel;
  	scanlinepad = screenInfo.formats[format].scanlinePad;
 	j = indexForBitsPerPixel[ bitsPerPixel ];
  	k = indexForScanlinePad[ scanlinepad ];
 	PixmapWidthPaddingInfo[ depth ].padPixelsLog2 = answer[j][k];
 	PixmapWidthPaddingInfo[ depth ].padRoundUp =
 	    (scanlinepad/bitsPerPixel) - 1;
 	j = indexForBitsPerPixel[ 8 ]; /* bits per byte */
 	PixmapWidthPaddingInfo[ depth ].padBytesLog2 = answer[j][k];
#if LONG_BIT == 64 && BITMAP_SCANLINE_PAD == 64
	/* Fake out protocol interface to make them believe
	 * we support padding of 32 instead of 64.
	 */
  	scanlinepad = 32;
 	j = indexForBitsPerPixel[ bitsPerPixel ];
  	k = indexForScanlinePad[ scanlinepad ];
 	PixmapWidthPaddingInfoProto[ depth ].padPixelsLog2 = answer[j][k];
 	PixmapWidthPaddingInfoProto[ depth ].padRoundUp =
 	    (scanlinepad/bitsPerPixel) - 1;
 	j = indexForBitsPerPixel[ 8 ]; /* bits per byte */
 	PixmapWidthPaddingInfoProto[ depth ].padBytesLog2 = answer[j][k];
#endif /* LONG_BIT */
    }
  
    /* This is where screen specific stuff gets initialized.  Load the
       screen structure, call the hardware, whatever.
       This is also where the default colormap should be allocated and
       also pixel values for blackPixel, whitePixel, and the cursor
       Note that InitScreen is NOT allowed to modify argc, argv, or
       any of the strings pointed to by argv.  They may be passed to
       multiple screens. 
    */ 
    pScreen->rgf = ~0L;  /* there are no scratch GCs yet*/
    WindowTable[i] = NullWindow;
    screenInfo.screens[i] = pScreen;
    screenInfo.numScreens++;
    if (!(*pfnInit)(i, pScreen, argc, argv))
    {
	FreeScreen(pScreen);
	screenInfo.numScreens--;
	return -1;
    }
    return i;
}

static void
FreeScreen(pScreen)
    ScreenPtr pScreen;
{
    xfree(pScreen->WindowPrivateSizes);
    xfree(pScreen->GCPrivateSizes);
    xfree(pScreen->devPrivates);
    xfree(pScreen);
}
