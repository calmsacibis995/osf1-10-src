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
 * $XConsortium: x386Init.c,v 1.2 91/08/20 15:39:58 gildea Exp $
 *
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
 */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "site.h"

#include "compiler.h"

#include "x386Procs.h"
#include "x386OSD.h"

#ifdef XTESTEXT1
#include "atKeynames.h"
extern int xtest_command_key;
#endif /* XTESTEXT1 */

extern ScrnInfoRec vga256InfoRec;
extern ScrnInfoRec vga16InfoRec;
extern ScrnInfoRec wgaInfoRec;
extern ScrnInfoRec xgaInfoRec;

ScrnInfoPtr x386Screens[] = {
  
  &vga256InfoRec,
#ifdef notyet
  &vga16InfoRec, 
  &wgaInfoRec,
  &xgaInfoRec,
#endif
};

int         x386MaxScreens = sizeof(x386Screens) / sizeof(ScrnInfoPtr);
x386InfoRec x386Info;
int         x386ScreenIndex;


/*
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *      That includes vt-manager setup, querying all possible devices and
 *      collecting the pixmap formats.
 */

void
InitOutput(pScreenInfo, argc, argv)
     ScreenInfo	*pScreenInfo;
     int     	argc;
     char    	**argv;
{
  int                    i, j, index, fd;
  struct vt_mode         VT;
  char                   vtname1[10],vtname2[10];
  static int             numFormats = 0;
  static PixmapFormatRec formats[MAXFORMATS];
  static unsigned long   generation = 0;
   

  if (serverGeneration == 1) {

    ErrorF("X386 Version 1.2 / X Windows System\n");
    ErrorF("(protocol Version %d, revision %d, vendor release %d)\n\n",
	   X_PROTOCOL, X_PROTOCOL_REVISION, VENDOR_RELEASE );

    /*
     * setup the virtual terminal manager
     */
    if ((fd = open("/dev/console",O_WRONLY,0)) <0) 
      FatalError("Cannot open /dev/console\n");

    if (ioctl(fd, VT_OPENQRY, &x386Info.vtno) < 0 || x386Info.vtno == -1) 
      FatalError("Cannot find a free VT\n");

    close(fd);

    sprintf(vtname1,"/dev/vc%02d",x386Info.vtno); /* ESIX */
    sprintf(vtname2,"/dev/vt%02d",x386Info.vtno); /* rest of the world */

    if ( (x386Info.consoleFd = open(vtname1, O_RDWR | O_NDELAY, 0)) < 0 &&
	 (x386Info.consoleFd = open(vtname2, O_RDWR | O_NDELAY, 0)) < 0 )
      FatalError("Cannot open %s (%s)\n",vtname2, vtname1);
    
    if (ioctl(x386Info.consoleFd, VT_GETMODE, &VT) < 0) 
      FatalError ("VT_GETMODE failed\n");
    
    signal(SIGUSR1, x386VTRequest);
    
    VT.mode = VT_PROCESS;
    VT.relsig = SIGUSR1;
    VT.acqsig = SIGUSR1;
    if (ioctl(x386Info.consoleFd, VT_SETMODE, &VT) < 0) 
      FatalError ("VT_SETMODE VT_PROCESS failed\n");
    
    if (ioctl(x386Info.consoleFd, KDSETMODE, KD_GRAPHICS) < 0)
      FatalError ("KDSETMODE KD_GRAPHICS failed\n");


    x386Config();

    /*
     * collect all possible formats
     */
    formats[0].depth = 1;
    formats[0].bitsPerPixel = 1;
    formats[0].scanlinePad = BITMAP_SCANLINE_PAD;
  
    for ( i=0; i < x386MaxScreens && x386Screens[i]->configured; i++ )
      { 
	/*
	 * add new pixmap format
	 */
	for ( j=0; j <= numFormats; j++ ) {
	  
	  if (formats[j].depth == x386Screens[i]->depth &&
	      formats[j].bitsPerPixel == x386Screens[i]->bitsPerPixel)
	    break; /* found */
	  
	  if ( j == numFormats ) {
	    formats[j].depth = x386Screens[i]->depth;
	    formats[j].bitsPerPixel = x386Screens[i]->bitsPerPixel;
	    formats[j].scanlinePad = BITMAP_SCANLINE_PAD;
	    numFormats++;
	    if ( numFormats > MAXFORMATS )
	      FatalError( "WSGO!! Too many formats! Exiting\n" );
	    
	    break; /* added */
	  }
	}
      }
  }

  /*
   * now force to get the VT
   */
  if (ioctl(x386Info.consoleFd, VT_ACTIVATE, x386Info.vtno) != 0)
    ErrorF("VT_ACTIVATE failed\n");
  if (ioctl(x386Info.consoleFd, VT_WAITACTIVE, x386Info.vtno) != 0)
    ErrorF("VT_WAITACTIVE failed\n");


  /*
   * Use the previous collected parts to setup pScreenInfo
   */
  pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
  pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
  pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
  pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
  pScreenInfo->numPixmapFormats = numFormats;
  for ( i=0; i < numFormats; i++ ) pScreenInfo->formats[i] = formats[i];

  if (generation != serverGeneration)
    {
      x386ScreenIndex = AllocateScreenPrivateIndex();
      generation = serverGeneration;
    }


  for ( i=0; i < x386MaxScreens && x386Screens[i]->configured; i++ )
    {    
      /*
       * On a server-reset, we have explicitely to remap all stuff ...
       * (At startuptime this is implicitely done by probing the device
       */
      if (serverGeneration != 1) (x386Screens[i]->EnterLeaveVT)(ENTER);
      index = AddScreen(x386Screens[i]->Init, argc, argv);
      if (index > -1)
	screenInfo.screens[index]->devPrivates[x386ScreenIndex].ptr
	  = (pointer)x386Screens[i];

      /*
       * Here we have to let the driver getting access of the VT. Note that
       * this doesn't mean that the graphics board may access automatically
       * the monitor. If the monitor is shared this is done in x386CrossScreen!
       */
      if (!x386Info.sharedMonitor) (x386Screens[i]->EnterLeaveMonitor)(ENTER);
    }

  RegisterBlockAndWakeupHandlers(x386Block, x386Wakeup, (void *)0);
}



/*
 * InitInput --
 *      Initialize all supported input devices...what else is there
 *      besides pointer and keyboard? Two DeviceRec's are allocated and
 *      registered as the system pointer and keyboard devices.
 */

void
InitInput(argc, argv)
     int     	  argc;
     char    	  **argv;
{
  x386Info.vtRequestsPending = FALSE;
  x386Info.inputPending = FALSE;
#ifdef XTESTEXT1
  xtest_command_key = KEY_SysReqest + MIN_KEYCODE;
#endif /* XTESTEXT1 */

  x386Info.pKeyboard = AddInputDevice(x386Info.kbdProc, TRUE); 
  x386Info.pPointer =  AddInputDevice(x386Info.mseProc, TRUE);
  RegisterKeyboardDevice(x386Info.pKeyboard); 
  RegisterPointerDevice(x386Info.pPointer); 

  miRegisterPointerDevice(screenInfo.screens[0], x386Info.pPointer);
  mieqInit (x386Info.pKeyboard, x386Info.pPointer);
}



/*
 * ddxGiveUp --
 *      Device dependent cleanup. Called by by dix before normal server death.
 *      For SYSV386 we must switch the terminal back to normal mode. No error-
 *      checking here, since there should be restored as much as possible.
 */

void
ddxGiveUp()
{
  struct vt_mode   VT;

  ioctl(x386Info.consoleFd, VT_ACTIVATE, x386Info.vtno);
  ioctl(x386Info.consoleFd, VT_WAITACTIVE, 0);
  ioctl(x386Info.consoleFd, KDSETMODE, KD_TEXT);  /* Back to text mode  ... */
  if (ioctl(x386Info.consoleFd, VT_GETMODE, &VT) != -1)
    {
      VT.mode = VT_AUTO;
      ioctl(x386Info.consoleFd, VT_SETMODE, &VT); /* set default vt handling */
    }

  close(x386Info.consoleFd);                    /* make the vt-manager happy */
}



/*
 * AbortDDX --
 *      DDX - specific abort routine.  Called by AbortServer(). The attempt is
 *      made to restore all original setting of the displays. Also all devices
 *      are closed.
 */

void
AbortDDX()
{
  int i;

  /*
   * try to deinitialize all input devices
   */
  if (x386Info.pPointer) (x386Info.mseProc)(x386Info.pPointer, DEVICE_CLOSE);
  if (x386Info.pKeyboard) (x386Info.kbdProc)(x386Info.pKeyboard, DEVICE_CLOSE);

  /*
   * try to restore the original video state
   */
  for ( i=0; i < screenInfo.numScreens; i++ )
    (X386SCRNINFO(screenInfo.screens[i])->EnterLeaveVT)( LEAVE );

  /*
   * This is needed for a abnormal server exit, since the normal exit stuff
   * MUST also be performed (i.e. the vt must be left in a defined state)
   */
  ddxGiveUp();
}



/*
 * ddxProcessArgument --
 *	Process device-dependent command line args. Returns 0 if argument is
 *      not device dependent, otherwise Count of number of elements of argv
 *      that are part of a device dependent commandline option.
 */

/* ARGSUSED */
int
ddxProcessArgument (argc, argv, i)
     int argc;
     char *argv[];
     int i;
{
  return 0;
}


/*
 * ddxUseMsg --
 *	Print out correct use of device dependent commandline options.
 *      Maybe the user now knows what really to do ...
 */

void
ddxUseMsg()
{
  ErrorF("\n");
  ErrorF("\n");
  ErrorF("Device Dependent Usage\n");
  ErrorF("\n");
}
