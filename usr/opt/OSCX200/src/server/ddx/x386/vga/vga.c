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
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/vga/vga.c,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */


#include "X.h"
#include "input.h"
#include "scrnintstr.h"
#include "mipointer.h"
#include "cursorstr.h"

#include "compiler.h"

#include "x386.h"
#include "x386Priv.h"
#include "x386OSD.h"
#include "vga.h"
#include "cfb.h"

extern void NoopDDA();

ScrnInfoRec vga256InfoRec = {
  FALSE,		/* Bool configured */
  -1,			/* int index */
  vgaProbe,		/* Bool (* Probe)() */
  vgaScreenInit,	/* Bool (* Init)() */
  vgaEnterLeaveVT,	/* void (* EnterLeaveVT)() */
  NoopDDA,		/* void (* EnterLeaveMonitor)() */
  NoopDDA,		/* void (* EnterLeaveCursor)() */
  vgaAdjustFrame,	/* void (* AdjustFrame)() */
  vgaSwitchMode,	/* void (* SwitchMode)() */
  8,			/* int depth */
  8,			/* int bitsPerPixel */
  PseudoColor,		/* int defaultVisual */
  -1, -1,		/* int virtualX,virtualY */
  -1, -1, -1, -1,	/* int frameX0, frameY0, frameX1, frameY1 */
  NULL,			/* char *vendor */
  NULL,			/* char *chipset */
  0,			/* int clocks */
  {0, },		/* int clock[MAXCLOCKS] */
  0,			/* int videoRam */
  240, 180,		/* int width, height */
  NULL,			/* DisplayModePtr modes */
};

pointer vgaOrigVideoState = NULL;
pointer vgaNewVideoState = NULL;
pointer vgaBase = NULL;
struct kd_memloc vgaDSC;

void (* vgaEnterLeaveFunc)();
void (* vgaInitFunc)();
void * (* vgaSaveFunc)();
void (* vgaRestoreFunc)();
void (* vgaAdjustFunc)();
void (* vgaSetReadFunc)();
void (* vgaSetWriteFunc)();
void (* vgaSetReadWriteFunc)();
int vgaMapSize;
int vgaSegmentSize;
int vgaSegmentShift;
int vgaSegmentMask;
void *vgaReadBottom;
void *vgaReadTop;
void *vgaWriteBottom;
void *vgaWriteTop =    (pointer)&writeseg; /* dummy for linking */
Bool vgaReadFlag;
Bool vgaWriteFlag;

int vgaIOBase;

static Bool saveFuncs = FALSE;
static void (* saveInitFunc)();
static void * (* saveSaveFunc)();
static void (* saveRestoreFunc)();
static void (* saveAdjustFunc)();
static void (* saveSetReadFunc)();
static void (* saveSetWriteFunc)();
static void (* saveSetReadWriteFunc)();

extern miPointerScreenFuncRec x386PointerScreenFuncs;

static vgaVideoChipPtr Drivers[] = {
  &PVGA1,
  &GVGA,
  &ET3000,
  &ET4000,
};

#define MAXDRIVERS (sizeof(Drivers)/sizeof(vgaVideoChipPtr))

/*
 * vgaProbe --
 *     probe and initialize the hardware driver
 */
Bool
vgaProbe()
{
  int            i, j;
  DisplayModePtr pMode, pEnd;

  for (i=0; i < MAXDRIVERS; i++)

    if ((Drivers[i]->ChipProbe)())
      {
	ErrorF("VGA256: %s (mem: %dk clocks:",
	       vga256InfoRec.chipset,
	       vga256InfoRec.videoRam);
	for (j=0; j < vga256InfoRec.clocks; j++)
	  ErrorF(" %2d", vga256InfoRec.clock[j]);
	ErrorF(")\n");

	vgaEnterLeaveFunc = Drivers[i]->ChipEnterLeave;
	vgaInitFunc = Drivers[i]->ChipInit;
	vgaSaveFunc = Drivers[i]->ChipSave;
	vgaRestoreFunc = Drivers[i]->ChipRestore;
	vgaAdjustFunc = Drivers[i]->ChipAdjust;
	vgaSetReadFunc = Drivers[i]->ChipSetRead;
	vgaSetWriteFunc = Drivers[i]->ChipSetWrite;
	vgaSetReadWriteFunc = Drivers[i]->ChipSetReadWrite;
	vgaMapSize = Drivers[i]->ChipMapSize;
	vgaSegmentSize = Drivers[i]->ChipSegmentSize;
	vgaSegmentShift = Drivers[i]->ChipSegmentShift;
	vgaSegmentMask = Drivers[i]->ChipSegmentMask;
	vgaReadBottom = (pointer)Drivers[i]->ChipReadBottom;
	vgaReadTop = (pointer)Drivers[i]->ChipReadTop;
	vgaWriteBottom = (pointer)Drivers[i]->ChipWriteBottom;
	vgaWriteTop = (pointer)Drivers[i]->ChipWriteTop;

	if (vga256InfoRec.virtualX > 0 &&
	    vga256InfoRec.virtualX * vga256InfoRec.virtualY >
	    vga256InfoRec.videoRam * 1024)
	  {
	    ErrorF("Too less memory for virtual resolution\n");
	    return(FALSE);
	  }


	pMode = pEnd = vga256InfoRec.modes;
	do {
	  x386LookupMode(pMode, &vga256InfoRec);
	  if (pMode->HDisplay * pMode->VDisplay > vga256InfoRec.videoRam*1024)
	    {
	      ErrorF("Too less memory for all resolutions\n");
	      return(FALSE);
	    }
	  pMode = pMode->next;
	}
	while (pMode != pEnd);
	
	return TRUE;
      }
  
  return FALSE;
}


/*
 * vgaScreenInit --
 *      Attempt to find and initialize a VGA framebuffer
 *      Most of the elements of the ScreenRec are filled in.  The
 *      video is enabled for the frame buffer...
 */

Bool
vgaScreenInit (index, pScreen, argc, argv)
    int            index;        /* The index of pScreen in the ScreenInfo */
    ScreenPtr      pScreen;      /* The Screen to initialize */
    int            argc;         /* The number of the Server's arguments. */
    char           **argv;       /* The arguments themselves. Don't change! */
{
  if (vgaBase == NULL) {
#ifdef SCO
    /*
     * To map the video-memory, we use the MAPCONS ioctl. First the screen
     * must be in graphics mode, hence the SW_CG640x350 (older SVR3.2 have
     * no VGA support, thus a EGA mode here !!!
     */
    if (ioctl(x386Info.consoleFd, SW_CG640x350, 0) != 0 ||
         (int)(vgaBase=(pointer)ioctl(x386Info.consoleFd, MAPCONS, 0)) == -1 )
      FatalError("failed to map the video memory\n");
#else
    vgaBase = (pointer)(((uint)xalloc(0x11000) & ~0xFFF) + 0x1000);
    vgaDSC.vaddr    = (char*)vgaBase;
    vgaDSC.physaddr = (char*)0xA0000;
    vgaDSC.length   = 0x10000;
    vgaDSC.ioflg   = 1;
    if (ioctl(x386Info.consoleFd, KDMAPDISP, &vgaDSC) < 0)
      FatalError("failed to map the video memory\n");
#endif
    vgaReadBottom  = (void *)((uint)vgaReadBottom + (uint)vgaBase); 
    vgaReadTop     = (void *)((uint)vgaReadTop + (uint)vgaBase); 
    vgaWriteBottom = (void *)((uint)vgaWriteBottom + (uint)vgaBase); 
    vgaWriteTop    = (void *)((uint)vgaWriteTop + (uint)vgaBase); 
  }

  (vgaInitFunc)(vga256InfoRec.modes);
  vgaOrigVideoState = (pointer)(vgaSaveFunc)(vgaOrigVideoState);
  (vgaRestoreFunc)(vgaNewVideoState);
  (vgaAdjustFunc)(vga256InfoRec.frameX0, vga256InfoRec.frameY0);

  /*
   * Inititalize the dragon to color display
   */
  if (!cfbScreenInit(pScreen,
		     (pointer) VGABASE,
		     vga256InfoRec.virtualX,
		     vga256InfoRec.virtualY,
		     75, 75,
		     vga256InfoRec.virtualX))
    return(FALSE);

    pScreen->CloseScreen = vgaCloseScreen;
    pScreen->SaveScreen = vgaSaveScreen;
    pScreen->InstallColormap = vgaInstallColormap;
    pScreen->UninstallColormap = vgaUninstallColormap;
    pScreen->ListInstalledColormaps = vgaListInstalledColormaps;
    pScreen->StoreColors = vgaStoreColors;
  
  miDCInitialize (pScreen, &x386PointerScreenFuncs);
  return (cfbCreateDefColormap(pScreen));

}



static void saveDummy() {}

/*
 * vgaEnterLeaveVT -- 
 *      grab/ungrab the current VT completely.
 */

void
vgaEnterLeaveVT(enter)
     Bool enter;
{
  if (enter)
    {
      vgaInitFunc = saveInitFunc;
      vgaSaveFunc = saveSaveFunc;
      vgaRestoreFunc = saveRestoreFunc;
      vgaAdjustFunc = saveAdjustFunc;
      vgaSetReadFunc = saveSetReadFunc;
      vgaSetWriteFunc = saveSetWriteFunc;
      vgaSetReadWriteFunc = saveSetReadWriteFunc;
      
#ifndef SCO
      ioctl(x386Info.consoleFd, KDMAPDISP, &vgaDSC);
#endif
      (vgaEnterLeaveFunc)(ENTER);
      vgaOrigVideoState = (pointer)(vgaSaveFunc)(vgaOrigVideoState);
      (vgaRestoreFunc)(vgaNewVideoState);

      saveFuncs = FALSE;
    }
  else
    {
      (vgaSaveFunc)(vgaNewVideoState);
      /*
       * We come here in many cases, but one is special: When the server aborts
       * abnormaly. Therefore there MUST be a check whether vgaOrigVideoState
       * is valid or not.
       */
      if (vgaOrigVideoState)
	(vgaRestoreFunc)(vgaOrigVideoState);

      (vgaEnterLeaveFunc)(LEAVE);
#ifndef SCO
      ioctl(x386Info.consoleFd, KDUNMAPDISP, 0);
#endif

      saveInitFunc = vgaInitFunc;
      saveSaveFunc = vgaSaveFunc;
      saveRestoreFunc = vgaRestoreFunc;
      saveAdjustFunc = vgaAdjustFunc;
      saveSetReadFunc = vgaSetReadFunc;
      saveSetWriteFunc = vgaSetWriteFunc;
      saveSetReadWriteFunc = vgaSetReadWriteFunc;
      
      vgaInitFunc = saveDummy;
      vgaSaveFunc = (void * (*)())saveDummy;
      vgaRestoreFunc = saveDummy;
      vgaAdjustFunc = saveDummy;
      vgaSetReadFunc = saveDummy;
      vgaSetWriteFunc = saveDummy;
      vgaSetReadWriteFunc = saveDummy;
      
      saveFuncs = TRUE;
    }
}



/*
 * vgaCloseScreen --
 *      called to ensure video is enabled when server exits.
 */

Bool
vgaCloseScreen()
{
  /*
   * Hmm... The server may shut down even if it is not running on the
   * current vt. Let's catch this case here.
   */
  if (x386VTSema) vgaEnterLeaveVT(LEAVE);
  return(TRUE);
}



/*
 * vgaAdjustFrame --
 *      Set a new viewport
 */
void
vgaAdjustFrame(x, y)
     int x, y;
{
  (vgaAdjustFunc)(x, y);
}


/*
 * vgaSwitchMode --
 *     Set a new display mode
 */
void
vgaSwitchMode(mode)
     DisplayModePtr mode;
{
  (vgaInitFunc)(mode);
  (vgaRestoreFunc)(vgaNewVideoState);
}
