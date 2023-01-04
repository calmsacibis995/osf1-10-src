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
static char *rcsid = "@(#)$RCSfile: wgainit.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/22 17:35:37 $";
#endif

/*
 *	This module has the initialization code for the
 *	Compaq Qvision. Refer to the Qvision hardware guide
 *	for more details.
 *
 *	Written: 1-May-1993, Henry R. Tumblin
 */

#include <stdio.h>
#include "X.h"
#include "Xproto.h"

#include "servermd.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "windowstr.h"
#include "dixfont.h"
#include "mi.h"
#include "mistruct.h"
#include "mibstore.h"
#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"
#include "wga.h"
#include "wgaprocs.h"
#include "wgaio.h"
#include "osvideo.h"
#include "gcstruct.h"

#include "setmode.h"

#include <sys/workstation.h>
#include "ws.h"

extern int wsScreenPrivateIndex;

/*
 *	Static function prototypes
 */

static unsigned char pwgaHWInit(ScreenPtr pScreen);
static void ClearScreen(ScreenPtr pScreen);	/* Clear the frame buffer */
static Bool pwgaCloseScreen(int index, ScreenPtr pScreen);


/*
 * Translation table between X ALU and Compaq triton
 */

unsigned char mergexlate[16] = {
  0x0, /* GXclear        0 */  
  0x8, /* GXand          src AND dst */
  0x4, /* GXandReverse   src AND NOT dst */
  0xC, /* GXcopy         src */
  0x2, /* GXandInverted  (NOT src) AND dst */
  0xA, /* GXnoop         dst */
  0x6, /* GXxor          src XOR dst */
  0xE, /* GXor           src OR dst */
  0x1, /* GXnor          (NOT src) AND (NOT dst) */
  0x9, /* GXequiv        (NOT src) XOR dst */
  0x5, /* GXinvert       NOT dst */   
  0xD, /* GXorReverse    src OR (NOT dst) */
  0x3, /* GXcopyInverted NOT src */ 
  0xB, /* GXorInverted   (NOT src) OR dst */
  0x7, /* GXnand         (NOT src) OR (NOT dst) */
  0xF  /* GXset          1 */  
};

int vgaScreenPrivateIndex ;
int vgaGeneration;
int vgaScreenActive;

void initJensenVGA();			   /* Initialize pointers for I/O */
void outblk_init();			   /* Initialize outblk addresses */


/*
 *	Initialize the triton VGA for high-res(1024x768) graphics
 *	This routine will destroy any previous state left in the vga.
 *	This code was lifted from the Compaq sample code. All the initialization
 *	values are from the tables provided by Compaq.
 */

static unsigned char
pwgaHWInit(ScreenPtr pScreen)
{
  unsigned char nPlanes=8;
  int ibMode=2,ibMonClass=2;
  int width, height, i;
  int maphi,maplo;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);
  
   /* check for valid mode and monitor class */
   outpw (GC_INDEX,ENV_REG_0+(5<<8));    /* Unlock regs */

   /* turn video off */
   inp( 0x3da);                   /* reset latch */
   outp( ATTR_INDEX, 0x00);

   /* set the sequencer */
   for (i = 0; i < SEQ_CNT; i++)
      {                           /* synchronous seq reset, load seq regs */
      outp( SEQ_INDEX, i);
      outp( SEQ_DATA, abSeq[ibMode][i]);
      };
   outp( SEQ_INDEX, 0x00);
   outp( SEQ_DATA, 0x03);         /* restart the sequencer */

   /* unlock extended graphics registers */
   outp( GC_INDEX, 0x0f);
   outp( GC_DATA, 0x05);

   /* set Adv VGA mode (set bit 0 of Ctrl Reg 0) */
   outp( GC_INDEX, 0x40);
   outp( GC_DATA, 0x01);

   /* fix sequencer pixel mask for 8 bits */
   WGAUpdatePixelMask(pShadow, 0xff);

   /* set BitBLT enable (unlocks other Triton extended registers) */
   outp( GC_INDEX, 0x10);
   outp( GC_DATA, 0x08);

   /* set Triton mode, set bits per pixel */
   WGAUpdateCtrlReg1(pShadow, abCtrlReg1[ibMode]);

   /* load Misc Output reg */
   outp( MISC_OUTPUT, abMiscOut[ibMonClass][ibMode]);

   /* load DAC Cmd regs */
   outp( DAC_CMD_0, 0x02);                  /* 8-bit DAC */
   outp( DAC_CMD_1, abDacCmd1[ibMode]);     /* bits per pixel */
   outp( DAC_CMD_2, 0x23);                  /* set PortSel mask, X cursor */

   /* load CRTC parameters */
   outp( CRTC_INDEX, 0x11);
   outp( CRTC_DATA, 0x00);        /* unlock CRTC regs 0-7 */
   for (i = 0; i < CRTC_CNT; i++)
      {
      outp( CRTC_INDEX, i);
      outp( CRTC_DATA, abCrtc[ibMonClass][ibMode][i]);
      }
   outp( GC_INDEX, 0x42);
   outp( GC_DATA, abOverflow1[ibMode]);
   outp( GC_INDEX, 0x51);
   outp( GC_DATA, abOverflow2[ibMonClass][ibMode]);

   /* load overscan color (black) */
   outp( CO_COLOR_WRITE, 0x00);
   outp( CO_COLOR_DATA, 0x00);    /* red component */
   outp( CO_COLOR_DATA, 0x00);    /* green component */
   outp( CO_COLOR_DATA, 0x00);    /* blue component */

   /* load attribute regs */
   inp( 0x3da);                   /* reset latch */
   for (i = 0; i < ATTR_CNT; i++)
      {
      outp( ATTR_INDEX, i);
      outp( ATTR_DATA, abAttr[ibMode][i]);
      }

   /* load graphics regs */
   for (i = 0; i < GRFX_CNT; i++)
      {
      outp( GC_INDEX, i);
      outp( GC_DATA, abGraphics[ibMode][i]);
      }

   ClearScreen(pScreen);

   /* turn video on */
   inp( 0x3da);                   /* reset latch */
   outp( ATTR_INDEX, 0x20);

   /* Setup defaults for BLT engine */

   outpwz (SRC_PITCH, MAX_SCANLINE_DWORDS);
   outpwz (DEST_PITCH, MAX_SCANLINE_DWORDS);
   outpz (BLT_CMD_1, XY_SRC_ADDR | XY_DEST_ADDR);

   return nPlanes;
}



/*
 *	ClearScreen() - Clear the frame memory
 */

static void ClearScreen(ScreenPtr pScreen)
{
   vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
   wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

   /* set datapath source for pattern-to-screen BitBLT */
   WGAUpdateDataPathCtrl(pShadow, ROPSELECT_NO_ROPS | PIXELMASK_ONLY |
			 PLANARMASK_NONE_0XFF | SRC_IS_PATTERN_REGS );

   /* set pattern register for blank pattern */
   outp( PREG_4, 0x00);
   outp( PREG_5, 0x00);
   outp( PREG_6, 0x00);
   outp( PREG_7, 0x00);
   outp( PREG_0, 0x00);
   outp( PREG_1, 0x00);
   outp( PREG_2, 0x00);
   outp( PREG_3, 0x00);

   /* set up BitBLT registers and start engine */
   outpi( DEST_ADDR_LO, 0x00, 0x00);
   outpw( BITMAP_WIDTH, 1024);
   outpw( BITMAP_HEIGHT, 1023);
   outp( BLT_CMD_1, XY_SRC_ADDR |
                    XY_DEST_ADDR );
   outp( BLT_CMD_0, FORWARD |
                    START_BLT );

   /* wait for idle h/w */
   while (inp( CTRL_REG_1) & GLOBAL_BUSY_BIT);

}



/*
 *	pwgaSuspendScreen() - Turn the video off
 */
static void
pwgaSuspendScreen()
{
/* Turn off the video	*/

   inp( 0x3da);                   /* reset latch */
   outp( ATTR_INDEX, 0x00);

  VideoClose(); /* Reset the VGA that is being passed through the triton */
}



/*
 *	pwgaResumeScreen() - Reset the screen and install the colormap
 */
static void
pwgaResumeScreen(ScreenPtr pScreen)
{
  pwgaHWInit(pScreen);
  vgaRefreshInstalledColormap();
}



/*
 *	pwgaCloseScreen() - Turn off the video
 */
static Bool
pwgaCloseScreen(index, pScreen)
     int index;
     ScreenPtr pScreen;
{
/* Turn off the video	*/

   inp( 0x3da);                   /* reset latch */
   outp( ATTR_INDEX, 0x00);
  /*
   * Now the triton will mirror changes made to the VGA DAC
   */
  VideoClose(); /* Reset the VGA that is being passed through the triton */
  return TRUE;
}



extern Bool colorSaveScreen(ScreenPtr pScreen, int on);

static DrawFuncs pwgaDrawFuncs = {
  pwgaSetPalette,
  pwgaBlit,
  pwgaReplicateScans,
  pwgaFillSolid,
  pwgaDrawColorImage,
  pwgaReadColorImage,
  pwgaOpaqueStipple,
  pwgaStipple,
  pwgaBresS,
  pwgaBresOnOff,
#ifdef SOFTWARE_CURSOR
  pwgaBresDouble,
  pwgaDrawCursor,
  pwgaUndrawCursor,
  pwgaSetNewCursor,
  colorCursorInit,
  colorShowCursor,
  colorHideCursorInXYWH,
  colorHideCursorInSpans,
  colorHideCursorInLine
#else
  pwgaBresDouble
#endif
};

static miBSFuncRec pwgaBSFuncRec = {
  vgaSaveAreas,
  vgaRestoreAreas,
  (void (*)()) 0,
  (PixmapPtr (*)()) 0,
  (PixmapPtr (*)()) 0
};


/*
  The list of visuals supported by the triton screen
  (only PseudoColor for now)
*/
static VisualRec visuals4[] = {
  {0, PseudoColor, 6, 16, 4, 0, 0, 0, 0, 0, 0}
};

static VisualRec visuals8[] = {
  {0, PseudoColor, 6, 256, 8, 0, 0, 0, 0, 0, 0}
};

#define NUMVISUALS      ((sizeof visuals4)/(sizeof visuals4[0]))

/*
  The list of VisualID's of the supported visuals
*/
static VisualID VIDs[NUMVISUALS];

/*
  The list of supported depths and visuals supported at each depth.
  Pixmaps are supported for each depth.
  Windows are supported for each depth with at least one visual.
*/
static DepthRec depths4[] = {
  /* depth      numVid          vids */
  {1,           0,              NULL},
  {4,           NUMVISUALS,     VIDs}
};

static DepthRec depths8[] = {
  /* depth      numVid          vids */
  {1,           0,              NULL},
  {8,           NUMVISUALS,     VIDs}
};

#define NUMDEPTHS       ((sizeof depths4)/(sizeof depths4[0]))
  

extern GCOps **colorGCOps;

Bool
pwgaScreenInit(index, pScreen, argc, argv)
     int index;
     ScreenPtr pScreen;
     int argc;
     char **argv;
{
  int i;
  unsigned char nPlanes;
  VisualRec *visuals;
  DepthRec *depths;
  vgaScreenPrivPtr vgaPriv;
  wgaShadowRegPtr pShadow;
  static int maplo,maphi;
#ifdef SOFTWARE_CURSOR
  CursorInitFuncPtr CursorInitFunc;
#endif
  
  /*
   *	You must call wsScreenInit to setup all the cfb
   *	routines, and the screen private stuff. What a crock.
   *	The AddScreen call should change to check if this stuff
   *	is null and if so, don't bother with the wsScreenPrivate
   *	at ALL!!!!
   */
  
  wsAllocScreenInfo (index, pScreen);
  if (wsScreenInit(index, pScreen, argc, argv) == -1) 
    return FALSE;

  initJensenVGA(index);		/* Init pointers, etc. */
  
  if (nPlanes==4) {
    visuals = visuals4;
    depths = depths4;
  }
  else {
    visuals = visuals8;
    depths = depths8;
  }

  if (vgaGeneration != serverGeneration) {
    vgaScreenPrivateIndex = AllocateScreenPrivateIndex();
    if (vgaScreenPrivateIndex < 0)
      return FALSE;
    vgaGeneration = serverGeneration;
    mfbGCPrivateIndex = AllocateGCPrivateIndex();

    /*
     *	Set up the visual IDs 
     */
  
    for (i = 0; i < NUMVISUALS; i++) {
      visuals[i].vid = FakeClientID(0);
      VIDs[i] = visuals[i].vid;
    }
  }
  
  /* 
   *	Initialize Window and GC Privates
   *	VGA Window Privates are not used, see vgaWind.c
   *	VGA GC Privates are the same as MFB GC Privates
   */
  
  if (!AllocateGCPrivate(pScreen, mfbGCPrivateIndex, sizeof(mfbPrivGC)))
    return FALSE;

  /*
   *	Set up the device private area - This is required!
   */
  
  vgaPriv = (vgaScreenPrivPtr) xalloc (sizeof (vgaScreenPrivRec));
  if (!vgaPriv)
    return FALSE;

  pScreen->devPrivates[vgaScreenPrivateIndex].ptr = (pointer) vgaPriv;
  vgaPriv->vgaregs = (pointer) GetVgaAddress(index);
  vgaPriv->firstScreenMem = (pointer) GetFrameAddress(index);

  /*
   *	Set up register shadowing and invalidate the contents
   */

  pShadow = (wgaShadowRegPtr) xalloc(sizeof (wgaShadowRegRec));
  if (!pShadow)
    return FALSE;
  vgaPriv->avail = (pointer) pShadow;
  pShadow->CtrlReg1 	= 0xF110011F;
  pShadow->FGColor 	= 0xF110011F;
  pShadow->BGColor 	= 0xF110011F;
  pShadow->PlaneMask 	= 0xF110011F;
  pShadow->PixelMask 	= 0xF110011F;
  pShadow->Rop_A 	= 0xF110011F;
  pShadow->DataPathCtrl	= 0xF110011F;

  /*
   *	Save and restore the high address map around
   *	initializing the VGA. Resetting the sequencer
   *	on the triton resets this register(yuch!)
   */
  
  outp(VIRT_CTRLR_SEL, pScreen->myNum);
  vgaScreenActive = pScreen->myNum;

  nPlanes = pwgaHWInit(pScreen); 	/* Init the hardware */

  maphi = ((unsigned int) GetVgaPhysAddress())>>20;
  outpw (GC_INDEX,HI_ADDR_MAP+((maphi&0xff)<<8));
  outpw (GC_INDEX,HI_ADDR_MAP+1+(((maphi>>8)&0xff)<<8));

  /*
   *	 Initialize the Screen data values
   */

  pScreen->width = 1024;
  pScreen->height = 768 ;
  pScreen->mmWidth = (pScreen->width * 254L) / (75 * 10);
  pScreen->mmHeight = (pScreen->height * 254L) / (75 * 10);

  pScreen->numDepths = NUMDEPTHS;
  pScreen->rootDepth = nPlanes;
  pScreen->allowedDepths = depths;

  pScreen->minInstalledCmaps = 1;
  pScreen->maxInstalledCmaps = 1;
  pScreen->backingStoreSupport = Always;
  pScreen->saveUnderSupport = Always;

  /* blackPixel, whitePixel set by vgaCreateDefColormap */
  pScreen->blackPixel = 0;
  pScreen->whitePixel = 0;

  pScreen->numVisuals = NUMVISUALS;
  pScreen->visuals = visuals;
  
  pScreen->devPrivate = (pointer) &pwgaDrawFuncs;

  /*
    Random Screen Procedures
  */
  pScreen->CloseScreen = pwgaCloseScreen;
  pScreen->QueryBestSize = mfbQueryBestSize; /* MFB */

#ifndef TRITON
  pScreen->SaveScreen = colorSaveScreen;
#endif

  pScreen->GetImage = vgaGetImage;
  pScreen->GetSpans = vgaGetSpans;
  pScreen->PointerNonInterestBox = colorPointerNonInterestBox;
  pScreen->SourceValidate = (void (*)()) 0;
  
  /*
    Window Procedures
  */
  pScreen->CreateWindow = vgaCreateWindow;
  pScreen->DestroyWindow = vgaDestroyWindow;
  pScreen->PositionWindow = vgaPostionWindow;
  pScreen->ChangeWindowAttributes = vgaChangeWindowAttributes;
  pScreen->RealizeWindow = vgaMapWindow;
  pScreen->UnrealizeWindow = vgaUnmapWindow;
  pScreen->ValidateTree = miValidateTree;
  pScreen->PostValidateTree = (void (*)()) 0;
  pScreen->WindowExposures = miWindowExposures;
  pScreen->PaintWindowBackground = vgaPaintWindow;
  pScreen->PaintWindowBorder = vgaPaintWindow;
  pScreen->CopyWindow = vgaCopyWindow;
  pScreen->ClearToBackground = miClearToBackground;
  
  /*
    Pixmap procedures
  */
  pScreen->CreatePixmap = vgaCreatePixmap;
  pScreen->DestroyPixmap = vgaDestroyPixmap;
  
  /*
    Backing store procedures
  */
  pScreen->SaveDoomedAreas = (void (*)()) 0;
  pScreen->RestoreAreas = (RegionPtr (*)()) 0;
  pScreen->ExposeCopy = (void (*)()) 0;
  pScreen->TranslateBackingStore = (RegionPtr (*)()) 0;
  pScreen->ClearBackingStore = (RegionPtr (*)()) 0;
  pScreen->DrawGuarantee = (void (*)()) 0;
  
  /*
    Font procedures
  */
  pScreen->RealizeFont   = vgaRealizeFont;
  pScreen->UnrealizeFont = vgaUnrealizeFont;
  
  /*
    Cursor Procedures
  */

#ifdef SOFTWARE_CURSOR

  /* The following are not used in the Triton.  Use the ws-layer instead */
  pScreen->DisplayCursor     = colorDisplayCursor;
  pScreen->RecolorCursor     = colorRecolorCursor;
  pScreen->ConstrainCursor   = colorConstrainCursor;
  pScreen->CursorLimits      = colorCursorLimits;
  pScreen->RealizeCursor     = colorRealizeCursor;
  pScreen->UnrealizeCursor   = colorUnrealizeCursor;
  pScreen->SetCursorPosition = colorSetCursorPosition;

#endif

  /*
    GC procedures
  */
  pScreen->CreateGC = vgaCreateGC;
  
  /*
    Colormap procedures
  */
  pScreen->CreateColormap = vgaCreateColormap;
  pScreen->DestroyColormap = vgaDestroyColormap;
  pScreen->ResolveColor = vgaResolveColor;

#ifndef TRITON

  pScreen->InstallColormap = vgaInstallColormap;
  pScreen->UninstallColormap = vgaUninstallColormap;
  pScreen->ListInstalledColormaps = vgaListInstalledColormaps;
  pScreen->StoreColors = vgaStoreColors;

#endif

  /*
    Region procedures, all done by MI except BitmapToRegion
  */
  pScreen->RegionCreate = miRegionCreate;
  pScreen->RegionInit = miRegionInit;
  pScreen->RegionCopy = miRegionCopy;
  pScreen->RegionDestroy = miRegionDestroy;
  pScreen->RegionUninit = miRegionUninit;
  pScreen->Intersect = miIntersect;
  pScreen->Union = miUnion;
  pScreen->Subtract = miSubtract;
  pScreen->Inverse = miInverse;
  pScreen->RegionReset = miRegionReset;
  pScreen->TranslateRegion = miTranslateRegion;
  pScreen->RectIn = miRectIn;
  pScreen->PointInRegion = miPointInRegion;
  pScreen->RegionNotEmpty = miRegionNotEmpty;
  pScreen->RegionEmpty = miRegionEmpty;
  pScreen->RegionExtents = miRegionExtents;
  pScreen->RegionAppend = miRegionAppend;
  pScreen->RegionValidate = miRegionValidate;
  pScreen->BitmapToRegion = mfbPixmapToRegion; /* MFB */
  pScreen->RectsToRegion = miRectsToRegion;
  pScreen->SendGraphicsExpose = miSendGraphicsExpose;
  
  /*
    os layer procedures
  */
  pScreen->BlockHandler = NoopDDA;
  pScreen->WakeupHandler = NoopDDA;
  pScreen->blockData = (pointer) 0;
  pScreen->wakeupData = (pointer) 0;

  /*
    Create Default Colormap
  */
  pScreen->defColormap = FakeClientID(0);
  pScreen->rootVisual = visuals[0].vid;
  if (!vgaCreateDefColormap(pScreen))
    return FALSE;

  mfbRegisterCopyPlaneProc (pScreen, vgaCopyPlane);

  colorGCOps[0]->Polylines = wgaLineSS;
  colorGCOps[0]->PolySegment = wgaSegmentSS;

  WGAUpdateFGColor(pShadow, 1);
  WGAUpdateBGColor(pShadow, 0);

#ifdef SOFTWARE_CURSOR
  CursorInitFunc = ((DrawFuncs *)(pScreen->devPrivate))->CursorInitFunc;
  (*CursorInitFunc)(pScreen);
#endif

  miInitializeBackingStore(pScreen, &pwgaBSFuncRec);

  {
    wsScreenPrivate *wsp = (wsScreenPrivate *)
				pScreen->devPrivates[wsScreenPrivateIndex].ptr;

    D_GENERICDrawInit(pScreen, wsp->screenDesc->screen);
  }


  outblk_init();

  return TRUE;
}

