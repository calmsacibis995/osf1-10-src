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
#define IDENT "X-007"
#define MODULE_NAME SFBSCRINIT
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991, 1992 BY                  *
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
*****************************************************************************
*/
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**      This module does screen initialization for the SFB hardware
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**
**  CREATION DATE:     20-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-007	DMC0013		Dave Coleman			29-Jan-1992
**		Compile sfbInitProc routine only for VMS.
**		Do not call cfbScreenClose for non-VMS
**
** X-6		BIM0011		Irene McCartney			27-Jan-1992
**		Merge latest changes from Joel
**
** X-5		BIM0010		Irene McCartney			23-Jan-1992
**		Put the code to initialize backing store back in to be 
**		only if we're building a non MITR5 server.
**
** X-4		FGK1048		Fred Kleinsorge			20-Jan-1992
**		Make it work.  Remove the init of backing store - this is done
**		in the MI init.  Also change the two CFB calls to a single call
**		to the init logic (which does these two calls and is the 
**		standard entry point).
**
** X-3		TLB0006		Tracy Bragdon			07-Jan-1992
**		Changes from Dave's sources for OSF/1
**
** X-2		BIM0009		Irene McCartney			02-Dec-1991
**		Change to close SFB screen open and close routines instead 
**		of CFB.
**		Add edit history
**--
**/


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

#include "sfb.h"
#include "cfbscrinit.h"
#include "sfbscrinit.h"
#include "sfbwindow.h"
#include "sfbparams.h"
#include "sfbgc.h"
#include "sfbpntwin.h"
#include "sfbplane.h"
#include "sfbbstore.h"
#include "sfbbitblt.h"
#include "sfbgetsp.h"
#include "sfbpixmap.h"

#include <stdio.h>

#ifdef SOFTWARE_MODEL
extern int logFlag;
FILE *cmdFile, *vramFile;
History()
{
}
DebugInit()
{
}
#endif

#if defined(ultrix) || defined(__osf__)
#include <sys/workstation.h>
#include "ws.h"

extern int wsScreenPrivateIndex;

extern Bool fbInitProc();
extern Bool (*wsAltScreenInitProc)();

Bool sfbInitProc(index, pScreen, argc, argv)
    int		index;
    ScreenPtr   pScreen;
    int		argc;
    char	**argv;
{
    wsAltScreenInitProc = sfbScreenInit;
    fbInitProc(index, pScreen, argc, argv);
}

#endif 

#ifdef MITR5
/*
  The following definitions are taken from cfbscrinit.c and used for
  sfbSetupScreen and sfbFinishScreenInit, which were copied from the
  corresponding cfb routines.
*/
#include "cfbmskbits.h"

#define _BP 8
#define _RZ ((PSZ + 2) / 3)
#define _RS 0
#define _RM ((1 << _RZ) - 1)
#define _GZ ((PSZ - _RZ + 1) / 2)
#define _GS _RZ
#define _GM (((1 << _GZ) - 1) << _GS)
#define _BZ (PSZ - _RZ - _GZ)
#define _BS (_RZ + _GZ)
#define _BM (((1 << _BZ) - 1) << _BS)
#define _CE (1 << _RZ)

static VisualRec visuals[] = {
/* vid  class        bpRGB cmpE nplan rMask gMask bMask oRed oGreen oBlue */
#ifndef STATIC_COLOR
    0,  PseudoColor, _BP,  1<<PSZ,   PSZ,  0,   0,   0,   0,   0,   0,
    0,  DirectColor, _BP, _CE,       PSZ,  _RM, _GM, _BM, _RS, _GS, _BS,
    0,  GrayScale,   _BP,  1<<PSZ,   PSZ,  0,   0,   0,   0,   0,   0,
    0,  StaticGray,  _BP,  1<<PSZ,   PSZ,  0,   0,   0,   0,   0,   0,
#endif
    0,  StaticColor, _BP,  1<<PSZ,   PSZ,  _RM, _GM, _BM, _RS, _GS, _BS,
    0,  TrueColor,   _BP, _CE,       PSZ,  _RM, _GM, _BM, _RS, _GS, _BS
};

#define	NUMVISUALS	((sizeof visuals)/(sizeof visuals[0]))

static  VisualID VIDs[NUMVISUALS];

static DepthRec depths[] = {
/* depth	numVid		vids */
    1,		0,		NULL,
    8,		NUMVISUALS,	VIDs
};

#define NUMDEPTHS	((sizeof depths)/(sizeof depths[0]))

extern int cfbWindowPrivateIndex;
extern int cfbGCPrivateIndex;

#endif	/* MITR5 */

int sfbScreenPrivateIndex;
int sfbGCPrivateIndex;
static unsigned long sfbGeneration = 0;

miBSFuncRec sfbBSFuncRec = {
    sfbSaveAreas,
    sfbRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};

/*ARGSUSED*/
Bool sfbCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
#ifdef SOFTWARE_MODEL
    MakeIdle();
    if (logFlag) {
	fprintf(stderr, "Closing log files\n");
	fclose (cmdFile);
	fclose (vramFile);
/*	logFlag = FALSE; */
    }
#endif

    xfree (SFBSCREENPRIV(pScreen));

#ifdef VMS
    return cfbvmsScreenClose(index,pScreen);
#else    

#ifndef MITR5
    return cfbCloseScreen(index, pScreen);
#else	/* #ifndef MITR5 */
/* This routine is called by wsScreenClose, don't loop back to it */
/*    return wsScreenClose(index, pScreen);
*/
#endif	/* #ifndef MITR5 */

#endif
}

/*
  The following two routines were copied from corresponding cfb routines.
  The only reason for this is that miScreenInit must be called with
  sfbBSFuncRec, which is passed by reference.
*/
Bool
sfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int	i;

    if (sfbGeneration != serverGeneration)
    {
	/*  Set up the visual IDs */
	for (i = 0; i < NUMVISUALS; i++) {
	    visuals[i].vid = FakeClientID(0);
	    VIDs[i] = visuals[i].vid;
	}
    }
    if (!mfbAllocatePrivates(pScreen,
			     &cfbWindowPrivateIndex, &cfbGCPrivateIndex))
	return FALSE;
    if (!AllocateWindowPrivate(pScreen, cfbWindowPrivateIndex,
			       sizeof(cfbPrivWin)) ||
	!AllocateGCPrivate(pScreen, cfbGCPrivateIndex, sizeof(cfbPrivGC)))
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

    /* Set up sfb routines */
    pScreen->GetImage = sfbGetImage;
    pScreen->GetSpans = sfbGetSpans;
    pScreen->CreateGC = sfbCreateGC;
    pScreen->CreatePixmap = sfbCreatePixmap;
    pScreen->DestroyPixmap = sfbDestroyPixmap;
    pScreen->PaintWindowBackground = sfbPaintWindow;
    pScreen->PaintWindowBorder = sfbPaintWindow;
    pScreen->CopyWindow = sfbCopyWindow;
    pScreen->CloseScreen = sfbCloseScreen;

    /* Set up cfb routines */
    pScreen->CreateWindow = cfbCreateWindow;
    pScreen->DestroyWindow = cfbDestroyWindow;
    pScreen->PositionWindow = cfbPositionWindow;
    pScreen->ChangeWindowAttributes = cfbChangeWindowAttributes;
    pScreen->RealizeWindow = cfbMapWindow;
    pScreen->UnrealizeWindow = cfbUnmapWindow;
    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
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

sfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int	i;
    int defaultVisualClass = wsDefaultColorVisualClass(pScreen);

    if (defaultVisualClass < 0)
    {
	i = 0;
    }
    else
    {
	for (i = 0;
	     (i < NUMVISUALS) && (visuals[i].class != defaultVisualClass);
	     i++)
	    ;
	if (i >= NUMVISUALS)
	    i = 0;
    }
    return miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			8, NUMDEPTHS, depths,
			visuals[i].vid, NUMVISUALS, visuals,
			&sfbBSFuncRec);
}

#ifdef VMS
Bool sfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
      sfb,total_bytes
#else
Bool sfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, 
	bits_per_pixel, depth, sfb
#endif /* VMS */
)
    ScreenPtr 	pScreen;
    pointer 	pbits;			/* pointer to screen bitmap */
    int 	xsize, ysize;		/* in pixels */
    int 	dpix, dpiy;		/* dots per inch */
    int 	width;			/* pixel width of frame buffer */
#ifndef VMS
    int		bits_per_pixel;		/* new with cfbflex */
    int		depth;			/* new with cfbflex */
#endif
    SFB 	sfb;			/* base of command registers */
#ifdef VMS
    int               total_bytes;            /* Total bytes available */
#endif
{
    sfbScreenPrivPtr scrPriv;
    int screenSize, memSize, size;
    volatile pointer base;

#ifdef MITR5

    if (!sfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;
    if (!sfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;

#else

    if (!cfbMostScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;

#endif

#ifdef SOFTWARE_MODEL
    ((PixmapPtr)(pScreen->devPrivate))->devKind *= SFBPIXELBYTES;
    vram = (BYTES *) (((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr);
    WrapperInit();
    InitVRAM();
    if (logFlag) {
	fprintf(stderr, "Opening log files\n");
	if ((cmdFile = fopen("sfbcmds.log", "w")) == (FILE *)NULL) {
	    fprintf (stderr, "can't open sfbcmds.log\n");
	    exit (1);
	}
	if ((vramFile = fopen("sfbvram.log", "w")) == (FILE *)NULL) {
	    fprintf (stderr, "can't open sfbvram.log\n");
	    exit (1);
	}
    }
#endif

#ifndef __osf__ /* just to make sure everyone understands this is not osf */
#ifdef DPS
    {   /* XXX temp crock until server can do per-screen extension init */
	extern void cfbCreateDDXMarkProcs();
	XMIRegisterDDXMarkProcsProcs(pScreen, cfbCreateDDXMarkProcs);
    }
#endif DPS
#endif

    if (sfbGeneration != serverGeneration) {
	sfbScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (sfbScreenPrivateIndex < 0)
	    return FALSE;
	sfbGCPrivateIndex = AllocateGCPrivateIndex();
	sfbGeneration = serverGeneration;
    }
    scrPriv = (sfbScreenPrivPtr) xalloc (sizeof (sfbScreenPrivRec));
    if (!scrPriv)
	return FALSE;
    pScreen->devPrivates[sfbScreenPrivateIndex].ptr = (pointer) scrPriv;
#ifdef SOFTWARE_MODEL
   scrPriv->sfb = NULL;
#else
    scrPriv->sfb = sfb;
#endif

#ifdef VMS
    {
	/* Some sfb functions assume that the frame buffer is mapped on an 8 meg
	 * boundary (when drawing lines and segments). 
	 * The VMS driver does not map the frame buffer to an 8 meg boundary, so
	 * determine the offset to the previous boundary and save in the scrPriv
	 * for later use...
	 */
	extern int cfbvms_bitmap_addr[MAXSCREENS*2]; /* video bitmap address */
#	define _8MB 0x800000
	scrPriv->offset = - (cfbvms_bitmap_addr[pScreen->myNum*2] % _8MB);
    }
    /* Let's find out how much sfb memory we have.  */
    base = ((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr;
    memSize = total_bytes;
#else
    /* Let's find out how much sfb memory we have.  */
    base = ((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr
	- (SFBSTARTPIXELS * SFBPIXELBYTES);
    memSize = 2 * SFBPIXELBYTES * 1024 * 1024;
#endif  /* VMS */

    SFBPLANEMASK(sfb, SFBBUSALL1);
    SFBROP(sfb, GXcopy);
    SFBFLUSHMODE(sfb, SIMPLE);


#ifndef MITR5
#define mess_me_up_alot
#endif

#ifdef mess_me_up_alot
    /* ||| This code doesn't seem to detect how much memory there is.  The
       damn sfb is always coming saying that it has lots.  This may be because
       the ROMs or the driver are setting the thing's address space too small,
       by setting the address ignore bits (or whatever those bits are that mask
       of 1 or 2 bits off the top of the TURBOchannel address) incorrectly. */
       
    if (ws_cpu != DS_5000) {    /* Can't directly address > 2Mbytes on 3max */
	PixelWord data;
	SFBDEEPREAD(sfb, data);
	data = (~data) & 0xf;
	printf("deep data = %lx\n", data);
	SFBDEEP(sfb, 0xc | data);
	while (memSize < 4 * 1024 * 1024) {
	    /* Try probing memory */
	    SFBWRITE(base + memSize, 0x5555);
	    SFBWRITE(base + memSize + SFBBUSBYTES, 0xAAAA);
	    SFBREAD(base + memSize, data);
	    if (data != 0x5555) break;
	    SFBREAD(base + memSize + SFBBUSBYTES, data);
	    if (data != 0xAAAA) break;
	    memSize += 2 * SFBPIXELBYTES * 1024 * 1024;
	}
    } 
    printf("memsize = %ld\n", memSize);
#endif

    /* Set up offscreen memory free list.  We have SFBSTARTPIXELS at the 
       beginning, but can't use the first SFBALIGNMENT bytes.  
           (VMS uses the space prior to the onscreen memory for the 
	    console window; we cannot count on using this space, 
	    although some is available for scratch (like when we 
	    move a window off the screen).
       We can't have the next ysize * width pixels, as those are visible on the 
       screen.  Finally, we have all the memory left past the screen. */
    scrPriv->firstScreenMem = base;
    scrPriv->lastScreenMem = base + memSize;

    scrPriv->avail.size = 0;
    scrPriv->avail.link = (FreeElement *)NULL;
    
#ifndef VMS
    sfbScreenFree(pScreen, base + SFBALIGNMENT,
	(SFBSTARTPIXELS * SFBPIXELBYTES) - SFBALIGNMENT);
#endif

    screenSize = ysize * width * SFBPIXELBYTES;
    base = ((PixmapPtr)(pScreen->devPrivate))->devPrivate.ptr + screenSize;
    size = memSize - screenSize 
#ifndef VMS
	- SFBSTARTPIXELS*SFBPIXELBYTES
#endif
   	 ;
    sfbScreenFree(pScreen, base, size);

    scrPriv->lastGC = (GCPtr)NULL;
    
    if (!AllocateGCPrivate(pScreen, sfbGCPrivateIndex, sizeof(sfbGCPrivRec))) {
	return FALSE;
    }

#ifndef MITR5
    /* Override a few decisions that cfb made. */

    pScreen->GetImage = sfbGetImage;
    pScreen->GetSpans = sfbGetSpans;
    pScreen->CreateGC = sfbCreateGC;
    pScreen->CreatePixmap = sfbCreatePixmap;
    pScreen->DestroyPixmap = sfbDestroyPixmap;
    pScreen->PaintWindowBackground = sfbPaintWindow;
    pScreen->PaintWindowBorder = sfbPaintWindow;
    pScreen->CloseScreen = sfbCloseScreen;
    pScreen->CopyWindow = sfbCopyWindow;

    miInitializeBackingStore (pScreen, &sfbBSFuncRec);
#endif

/* ||| Not until we do fast 8-1 CopyPlane code (NOT the same as 8-1-8, which
    we should do too!) in cfb and sfb.
    mfbRegisterCopyPlaneProc (pScreen, sfbCopyPlane);
*/


   {
        wsScreenPrivate *wsp = (wsScreenPrivate *)
          pScreen->devPrivates[wsScreenPrivateIndex].ptr;

        D_GENERICDrawInit(pScreen, wsp->screenDesc->screen);
    }

    return TRUE;
}

#ifndef MITR5

extern int (*fbScreenInitProc)();
extern int cfbScreenInit();

Bool sfbInitProc(index, pScreen, argc, argv)
    int         index;
    ScreenPtr   pScreen;
    int         argc;
    char        **argv;
{
    fbScreenInitProc = sfbScreenInit;
    fbInitProc(index, pScreen, argc, argv);
    fbScreenInitProc = cfbScreenInit;
}

#endif	/* !MITR5 */
