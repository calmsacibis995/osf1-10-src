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
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/* $XConsortium: init.c,v 1.5 92/04/06 18:18:58 keith Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/devio.h>

#include "X.h"

#include "scrnintstr.h"
#include "servermd.h"

#include "input.h"
/* XXX */
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include "ws.h"

extern void wsMouseProc();
extern void wsKeybdProc();
extern void wsClick();
extern void wsChangePointerControl();

extern KeybdCtrl defaultKeyboardControl;
ws_event_queue	*queue;

static int NumFormats;
static int bitsPerDepth[33];

ws_screen_descriptor screenDesc[MAXSCREENS];

Bool
commandLineMatch( argc, argv, pat, pmatch)
    int         argc;           /* may NOT be changed */
    char *      argv[];         /* may NOT be changed */
    char *      pat;
{
    int         ic;

    for ( ic=0; ic<argc; ic++)
        if ( strcmp( argv[ic], pat) == 0)
            return TRUE;
    return FALSE;
}

Bool
commandLinePairMatch( argc, argv, pat, pmatch)
    int         argc;           /* may NOT be changed */
    char *      argv[];         /* may NOT be changed */
    char *      pat;
    char **     pmatch;         /* RETURN */
{
    register int         ic;

    for ( ic=0; ic<argc; ic++)
        if ( strcmp( argv[ic], pat) == 0) {
            *pmatch = argv[ ic+1];
            return TRUE;
	}
    return FALSE;
}

ws_descriptor wsinfo;
int wsFd;
int ws_cpu;
int forceDepth;

int wsScreenPrivateIndex;
/* the following filth is forced by a broken dix interface */

InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char **argv;
{
    int i, j;
    int lsn = 0;/* logical server screen number, should match what goes
		 * into AddScreen
		 */
    int psn;	/* physical Screen Number */
    static int inited = FALSE;
    static int ma = 4;
    static int mt = 4;
    static PtrCtrl ctrl;
    static int  clicklevel;

    if (!inited) {
	char *clickvolume;
	char *mouseAcceleration;
	char *mouseThreshold;
	char *forceD;
	ws_keyboard_control control;
        inited = TRUE;
    
        if ((wsFd = open("/dev/mouse",  O_RDWR, 0)) < 0) {
		ErrorF("couldn't open device\n");
		exit (1);
	}

	if (ioctl (wsFd, GET_WORKSTATION_INFO, &wsinfo) != 0) {
		ErrorF("GET_WORKSTATION_INFO failed \n");
		exit(1);
	}

	control.device_number = wsinfo.console_keyboard;
	if (ioctl(wsFd, GET_KEYBOARD_CONTROL, &control) == -1) {
		ErrorF("GET_KEYBOARD_CONTROL failed\n");
		exit(1);
	}
	if (!commandLineMatch(argc, argv, "-c"))
    		defaultKeyboardControl.click = control.click;
	defaultKeyboardControl.bell = control.bell;
	defaultKeyboardControl.bell_pitch = control.bell_pitch;
	defaultKeyboardControl.bell_duration = control.bell_duration;
	defaultKeyboardControl.autoRepeat = control.auto_repeat;
	defaultKeyboardControl.leds = control.leds;
	bcopy (control.autorepeats, defaultKeyboardControl.autoRepeats, 32);
	
    /* turn off cursors on additional screens initially */
/* 
 * jmg - have to do a lot of other stuff here dealing with visuals
 *  and depths 
 */
       /*
	* deal with arguments.  Note we don't bother until we've successfully
	* opened the device.
	*/

	if (commandLinePairMatch( argc, argv, "c", &clickvolume))
		sscanf( clickvolume, "%d", &clicklevel);
	if (commandLinePairMatch( argc, argv, "-a", &mouseAcceleration))
		sscanf( mouseAcceleration, "%d", &ma);
	if (commandLinePairMatch( argc, argv, "-t", &mouseThreshold))
		sscanf( mouseThreshold, "%d", &mt);
	if (commandLinePairMatch( argc, argv, "-forceDepth", &forceD))
		sscanf ( forceD, "%d", &forceDepth);
    }
    for (i = 1; i <= 32; i++)
	bitsPerDepth[i] = 0;

    for (psn = 0; psn < wsinfo.num_screens_exist; psn++)
    {
	ws_screen_descriptor	screeninfo;
	ws_depth_descriptor	depthinfo;
	
	screeninfo.screen = psn;
	ioctl (wsFd, GET_SCREEN_INFO, &screeninfo);
	for (j = 0; j < screeninfo.allowed_depths; j++) 
	{
	    depthinfo.screen = psn;
	    depthinfo.which_depth = j;
	    ioctl (wsFd, GET_DEPTH_INFO, &depthinfo);
/*
	    if (forceDepth)
		depthinfo.depth = forceDepth;
*/
	    if (bitsPerDepth[depthinfo.depth] &&
		bitsPerDepth[depthinfo.depth] != depthinfo.bits_per_pixel)
	    {
		FatalError ("Screens with mismatching bpp for depth %d\n",
			    depthinfo.depth);
	    }
	    bitsPerDepth[depthinfo.depth] = depthinfo.bits_per_pixel;
	}
    }
    if (!bitsPerDepth[1])
	bitsPerDepth[1] = 1;
#define INCLUDE_ALL_CFB
#ifdef INCLUDE_ALL_CFB
    j = 0;
    for (i = 1; i <= 32; i++)
	if (bitsPerDepth[i])
	    j |= 1 << (bitsPerDepth[i] - 1);
    if (!(j & (1 << 3)))
	bitsPerDepth[4] = 8;
    if (!(j & (1 << 7)))
	bitsPerDepth[8] = 8;
    if (!(j & (1 << 11)))
	bitsPerDepth[12] = 32;
    if (!(j & (1 << 23)))
	bitsPerDepth[24] = 32;
    if (!(j & (1 << 31)))
      bitsPerDepth[32] = 32;
#endif    
    NumFormats = 0;
    for (i = 1; i <= 32; i++)
    {
	if (j = bitsPerDepth[i]) {
	    if (NumFormats >= MAXFORMATS)
		FatalError ("MAXFORMATS is too small for this machine\n");
	    screenInfo->formats[NumFormats].depth = i;
	    screenInfo->formats[NumFormats].bitsPerPixel = j;
	    screenInfo->formats[NumFormats].scanlinePad = BITMAP_SCANLINE_PAD;
	    NumFormats++;
	}
    }

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
    screenInfo->numPixmapFormats = NumFormats;

    ctrl.num = ma;
    ctrl.den = 1;
    ctrl.threshold = mt;
    wsChangePointerControl( (DevicePtr) NULL, &ctrl);
    wsClick(clicklevel);
    if (commandLineMatch(argc, argv, "-c")) wsClick(0);
    wsScreenPrivateIndex = AllocateScreenPrivateIndex();
    ws_cpu = wsinfo.cpu;
    for(psn = 0; psn < wsinfo.num_screens_exist; psn++) {
	int j;

	/* Don't initialize this screen if it has already been
	 * disabled or if there has been an OnlyScreen specified
	 * and we aren't it.
	 */
	if ( !(wsOnlyScreen == -1 || psn == wsOnlyScreen) ) continue;
	if ( wsOnlyScreen == -1 && wsDisabledScreens[psn] == 1 ) continue;
	
	/* Outside of this routine, until the wsScreenPrivate
	 * is set up, screenDesc has to be referenced and the
	 * only handle things have is the index as AddScreen
	 * sees things, which is the logical screen index.
	 * However, the logical screen index will change after
	 * initialization, so don't reference screenDesc directly
	 * after screen initialization, reference it through the
	 * pscreen ws private data 
	 */
	screenDesc[lsn].screen = psn;

	if (ioctl(wsFd,	 GET_SCREEN_INFO, &screenDesc[lsn]) == -1) {
	    ErrorF("GET_SCREEN_INFO failed\n");
	    exit(1);
	}

	if (lsn >= MAXSCREENS) {
	    ErrorF ("Server configured for %d screens, can't configure screen %d\n", MAXSCREENS, psn);
	    break;
	}

	j = LoadAndInitDeviceModule(screenDesc[lsn].moduleID, argc, argv);
	/*
	 * LoadAndInitDeviceModule either returns -1 (error) 
	 * or # of screens installed
	 * (current index).  screenDesc[] is exactly parallel to
	 * screenInfo.screens[] since the initProc() arg will be called
	 * with AddScreen's index, which in turn gets used for screenDesc[].
	 */
	if (j == -1) {
		ErrorF("Could not AddScreen, ID = %s\n",
			screenDesc[lsn].moduleID);
		wsDisabledScreens[psn] = 1;
	} else
	    lsn = j+1;
    }
#ifdef XINPUT
    ExtInitOutput(screenInfo, argc, argv);
#endif
    wsInputOutputFinish(wsinfo.num_screens_exist);
}


void
InitInput(argc, argv)
    int argc;
    char *argv[];
{
    DevicePtr p, k;
    static int inited = FALSE;
    
    p = AddInputDevice(wsMouseProc, TRUE);

    k = AddInputDevice(wsKeybdProc, TRUE);

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);

#ifdef XINPUT
    ExtInitInput(argc, argv);
#endif

    if (!inited) {
	inited = TRUE;

	if (ioctl(wsFd,  GET_AND_MAP_EVENT_QUEUE, &queue) == -1)  {
		ErrorF("GET_AND_MAP_EVENT_QUEUE failed\n");
		exit(1);
	}
    }
    SetTimeSinceLastInputEvent ();
}
