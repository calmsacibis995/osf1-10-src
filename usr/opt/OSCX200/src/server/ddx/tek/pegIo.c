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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/pegIo.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
/***********************************************************
Portions modified by Tektronix, Inc.  Copyright 1987 Tektronix, Inc.

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
#include "X.h"
#define  NEED_EVENTS
#include "Xproto.h"
#include "inputstr.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "pixmap.h"
#include "windowstr.h"
#include "regionstr.h"
#include "resource.h"

#include "mi.h"

#ifdef	UTEK
#include <box/keyboard.h>
#endif	/* UTEK */

#ifdef	UTEKV
#include "redwing/keyboard.h"
#endif	/* UTEKV */

#include "peg.h"

static void pegChangePointerControl();
static void pegChangeKeyboardControl();
static void xtlChangeKeyboardControl();
static void pegBell();
static void xtlBell();
extern int errno;
extern InitInfo	pegInfo;

Bool
pegSaveScreen(pScreen, on)
    ScreenPtr pScreen;
    int on;

{
    switch (on) {
    case SCREEN_SAVER_FORCER:
    case SCREEN_SAVER_OFF:
		pegInfo.screenIsSaved = SCREEN_SAVER_OFF;
		DisplayOn();
		break;
    case SCREEN_SAVER_ON:
    default:
		pegInfo.screenIsSaved = SCREEN_SAVER_ON;
		DisplayOff();
		break;
    }

    return TRUE;
}


/*
 * This routine is needed by a os/4.2 routine to implement screensaver.  It is
 * not documented in ddx.doc
 */
TimeSinceLastInputEvent()
{
        return (pegInfo.dsp->ds_ltime - pegInfo.dsp->ds_lastevent);
}

int
pegGetMotionEvents()
{
    return 0;
}

int
pegMouseProc(pDev, onoff, argc, argv)
    DevicePtr pDev;
    int onoff;
    int argc;
    char **argv;
{
    BYTE map[8];
    SvcCursorXY	upperLeft, lowerRight;

    debug4(("MouseProc(0x%x,%d,%d,0x%x)\n", pDev, onoff, argc, argv));

    switch (onoff)
    {
	case DEVICE_INIT: 
	    pegInfo.pPointer = pDev;
	    pDev->devicePrivate = (pointer) pegInfo.queue;
	    /*
	     * Mouse button map so that the leftmost mouse button comes out as
	     * 1, middle as 2 and the rightmost button as 3.  Any thing else
	     * comes out as 0 (the driver only gives us one button at a
	     * time).
	     */
	    map[0] = map[1] = map[2] = map[3] = map[4] = map[5] = map[6] =
		map[7] = 0;
	    map[Button_L] = 1;
	    map[Button_M] = 2;
	    map[Button_R] = 3;
	    InitPointerDeviceStruct(pegInfo.pPointer,
				    map,
				    8,
				    pegGetMotionEvents,
				    pegChangePointerControl,
				    0);
#ifndef XPEG_TANDEM
	    SetInputCheck(&pegInfo.queue->head, &pegInfo.queue->tail);
#endif /* XPEG_TANDEM */

	    /*
	     * Turn on driver.  Enable the mouse.
	     */
	    SetCursorMode(0);
	    MousePanOn(); /* this is only effective on a 4405 anyway */
	    JoyPanOff(); /* if there is panning... we do it */
	    upperLeft.x = upperLeft.y = 0;
	    lowerRight.x = pegInfo.scrWidth - 1;
	    lowerRight.y = pegInfo.scrHeight - 1;
	    SetCursorBounds(&upperLeft, &lowerRight);

	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    AddEnabledDevice(pegInfo.eventFd);
	    CursorEnable();

	    /*
	     * XXX - the driver has not issued an event yet, so the time of the
	     * last event is 0, and the time returned by
	     * TimeSinceLastInputEvent() will be very large, and
	     * WaitForSomething() will compute a negative
	     * time for select(), and select will return with an error... ad
	     * infinitum.  The bug in WaitForSomething should really be fixed.
	     * 2/10/87 Todd B.
	     */
	    pegInfo.dsp->ds_lastevent = pegInfo.dsp->ds_ltime;

	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
	    RemoveEnabledDevice(pegInfo.eventFd);
	    CursorDisable();
	    break;
	case DEVICE_CLOSE: 
	    break;
    }
    return Success;

}

/*
 * These are the "main row" keys.  These are repeatable.
 */
static char mainrow_PEG[] = {
    KBBackSpace,KBTab,      KBLineFeed, KBReturn,
    KBSpaceBar, KBQuote,    KBComma,    KBHyphon,
    KBPeriod,   KBSlash,    KB0,        KB1,
    KB2,        KB3,        KB4,        KB5,        
    KB6,        KB7,        KB8,        KB9,        
    KBSemiColon,KBEqual,    KBA,        KBB,    
    KBC,        KBD,        KBE,        KBF,        
    KBG,        KBH,        KBI,        KBJ,        
    KBK,        KBL,        KBM,        KBN,        
    KBO,        KBP,        KBQ,        KBR,        
    KBS,        KBT,        KBU,        KBV,        
    KBW,        KBX,        KBY,        KBZ,        
    KBBraceL,   KBBackSlash,KBBraceR,   KBBar,  
    KBRubOut,   KBJoyRight, KBJoyUp,    KBJoyLeft,  
    KBJoyDown
};
static char mainrow_XTL[] = {
    KEY_BackSpace,KEY_Tab,      KEY_Linefeed, KEY_Return,
    KEY_Space,    KEY_Quote,    KEY_Comma,    KEY_Minus,
    KEY_Period,   KEY_Slash,    KEY_0,        KEY_1,
    KEY_2,        KEY_3,        KEY_4,        KEY_5,        
    KEY_6,        KEY_7,        KEY_8,        KEY_9,        
    KEY_SemiColon,KEY_Equal,    KEY_a,        KEY_b,    
    KEY_c,        KEY_d,        KEY_e,        KEY_f,        
    KEY_g,        KEY_h,        KEY_i,        KEY_j,        
    KEY_k,        KEY_l,        KEY_m,        KEY_n,        
    KEY_o,        KEY_p,        KEY_q,        KEY_r,        
    KEY_s,        KEY_t,        KEY_u,        KEY_v,        
    KEY_w,        KEY_x,        KEY_y,        KEY_z,        
    KEY_LBrace,   KEY_VertBar,  KEY_RBrace,   KEY_Tilde,  
    KEY_RubOut,   KEY_Cursor_R, KEY_Cursor_U, KEY_Cursor_L,  
    KEY_Cursor_D
};

int
pegKeybdProc(pDev, onoff, argc, argv)
    DevicePtr pDev;
    int onoff;
    int argc;
    char **argv;
{
    KeySymsRec keySyms;
    CARD8 modMap[MAP_LENGTH];
    int	i;

    switch (onoff)
    {
	case DEVICE_INIT: 
	    pegInfo.pKeyboard = pDev;
	    pDev->devicePrivate = (pointer) pegInfo.queue;
#ifdef	UTEK
	    if (KEYIDSTYLE(pegInfo.softp) ==  KB_STYLE_VT200)
#endif	/* UTEK */
#ifdef	UTEKV
	    if (KEYIDSTYLE(pegInfo.dsp) ==  KB_STYLE_VT200)
#endif	/* UTEKV */
	      {
		/* XTL style */
		gfbXTLKBMappings(&keySyms, modMap);
		InitKeyboardDeviceStruct(
			pegInfo.pKeyboard, &keySyms, modMap, xtlBell,
			xtlChangeKeyboardControl);
		for (i=0; i<sizeof(mainrow_XTL); i++) {
		    MakeRawKeyRepeatable(mainrow_XTL[ i ]);
		}
	      }
#ifdef	UTEK
	    else if (KEYIDSTYLE(pegInfo.softp) ==  KB_STYLE_YELLOW_JACKET)
#endif	/* UTEK */
#ifdef	UTEKV
	    else if (KEYIDSTYLE(pegInfo.dsp) ==  KB_STYLE_YELLOW_JACKET)
#endif	/* UTEKV */
	      {
			/* 4207 YELLOW JACKET style */
		GetPEGKBMappings(&keySyms, modMap);
		InitKeyboardDeviceStruct(
			pegInfo.pKeyboard, &keySyms, modMap, xtlBell,
			pegChangeKeyboardControl);
		for (i=0; i<sizeof(mainrow_XTL); i++) {
		    MakeRawKeyRepeatable(mainrow_XTL[ i ]);
		}
	    } else {
		/* default to Pegasus style */
		GetPEGKBMappings(&keySyms, modMap);
		InitKeyboardDeviceStruct(
			pegInfo.pKeyboard, &keySyms, modMap, pegBell,
			pegChangeKeyboardControl);
		for (i=0; i<sizeof(mainrow_PEG); i++) {
		    MakeRawKeyRepeatable(mainrow_PEG[ i ]);
		}
	    }
	    Xfree((pointer)keySyms.map);
	    ((DeviceIntPtr)pDev)->kbdfeed->ctrl.autoRepeat = True;
	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    AddEnabledDevice(pegInfo.eventFd);
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
/*	    RemoveEnabledDevice(pegInfo.eventFd);  */
	    break;
	case DEVICE_CLOSE: 
	    break;
    }
    return Success;
}

static void
pegChangeKeyboardControl(pDevice, ctrl)
    DevicePtr pDevice;
    KeybdCtrl *ctrl;
{
    /* ctrl->click: not possible */

    /* ctrl->bell: the DIX layer handles the base volume for the bell */

    /* ctrl->bell_pitch: can't set now */

    /* ctrl->bell_duration: as far as I can tell, you can't set this  */

    /* ctrl->leds: We have only one LED, but it is owned by ddx... */

    /* ctrl->autoRepeat: We need do nothing here... dix maintains this */

    /* ctrl->autoRepeats: Force on the joypad, if necessary */
    if (pegInfo.kv.panEnabled) {
	MakeRawKeyRepeatable(KBJoyLeft);
	MakeRawKeyRepeatable(KBJoyRight);
	MakeRawKeyRepeatable(KBJoyUp);
	MakeRawKeyRepeatable(KBJoyDown);
    }
}


/*
 *	NAME
 *		setXTLBellVolume - Set bell volume on XTL Keyboard
 *
 *	SYNOPSIS
 */
static void
setXTLBellVolume(volume)
    int volume;		/* in: volume (0-100)		*/
/*
 *	DESCRIPTION
 *		This routines sets the bell volume level on the XTL
 *		Keyboard.
 *
 *	RETURNS
 *		None
 */
{
    assert(volume >= 0 && volume <= 100);

    if (pegInfo.bells.fd < 0) {
	/*
	 * Unable to open /dev/bell
	 */
	return;
    }

    /*
     * XTL keyboard has only 3 distinct bell volumes.
     * Volume is a value between 0 and 100 with 100 being loudest.
     */
    if ( volume < 34 ) {
	/*
	 * Make ioctl() call to /dev/bell to set bell volume to SOFT
	 */
	if (ioctl(pegInfo.bells.fd, BELL_KEYBD_BELL_ON_SOFT, (char*)0) == -1) {
#ifdef XDEBUG
	    ErrorF("ioctl() returns error %d for BELL_KEYBD_BELL_ON_SOFT\n",
		    errno);
#endif /* XDEBUG */
	}
    } else if (volume < 68 ) {
	/*
	 * Make ioctl() call to /dev/bell to set bell volume to MEDIUM
	 */
	if (ioctl(pegInfo.bells.fd, BELL_KEYBD_BELL_ON_MED, (char*)0) == -1) {
#ifdef XDEBUG
	    ErrorF("ioctl() returns error %d for BELL_KEYBD_BELL_ON_MED\n",
		    errno);
#endif /* XDEBUG */
	}
    } else {
	/*
	 * Make ioctl() call to /dev/bell to set bell volume to LOUD
	 */
	if (ioctl(pegInfo.bells.fd, BELL_KEYBD_BELL_ON_LOUD, (char*)0) == -1) {
#ifdef XDEBUG
	    ErrorF("ioctl() returns error %d for BELL_KEYBD_BELL_ON_LOUD\n",
		    errno);
#endif /* XDEBUG */
	}
    }
}



/*
 *	NAME
 *		xtlChangeKeyboardControl - Changes Keyboard Controls
 *			for XTL keyboard
 *
 *	SYNOPSIS
 */
static void
xtlChangeKeyboardControl(pDevice, pKbCtrl)
    DevicePtr pDevice;	/* in: pointer to keyboard device	*/
    KeybdCtrl *pKbCtrl;	/* in: pointer to kb control structure	*/
/*
 *	DESCRIPTION
 *		Changes keyboard control as specified in the KeybdCtrl
 *		structure (i.e., autorepeat, keyclick volume, bell
 *		volume, bell duration, LEDs.
 *
 *	RETURNS
 *		None
 *
 */
{
    int lbval = 0;	/* low-byte value for bell duration ioctl */

    /*
     * pKbCtrl->autoRepeat: We need do nothing here... dix maintains this
     */

    /*
     * pKbCtrl->autoRepeats: Force on the joypad, if necessary
     */
    if (pegInfo.kv.panEnabled) {
	MakeRawKeyRepeatable(KEY_Cursor_R);
	MakeRawKeyRepeatable(KEY_Cursor_U);
	MakeRawKeyRepeatable(KEY_Cursor_L);
	MakeRawKeyRepeatable(KEY_Cursor_D);
    }

    /*
     * pKbCtrl->click:
     */
    if (pegInfo.bells.fd > 0 && 0 <= pKbCtrl->click && pKbCtrl->click <= 100) {
	if (pKbCtrl->click == 0) {
	    /*
	     * Make ioctl() call to /dev/bell to:
	     *      Turn KeyClick OFF
	     */
	    if (ioctl(pegInfo.bells.fd, BELL_KEYBD_CLICK_OFF, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for BELL_KEYBD_CLICK_OFF\n",
		    errno);
#endif /* XDEBUG */
	    }
	} else if (pKbCtrl->click < 34) {
	    /*
	     * Make ioctl() call to /dev/bell to:
	     *      Set KeyClick volume to SOFT
	     */
	    if (ioctl(pegInfo.bells.fd, BELL_KEYBD_CLICK_ON_SOFT, (char*)0)
		== -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for BELL_KEYBD_CLICK_ON_SOFT\n",
		    errno);
#endif /* XDEBUG */
	    }
	} else if (pKbCtrl->click < 68) {
	    /*
	     * Make ioctl() call to /dev/bell to:
	     *      Set KeyClick volume to MEDIUM
	     */
	    if (ioctl(pegInfo.bells.fd, BELL_KEYBD_CLICK_ON_MED, (char*)0)
		== -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for BELL_KEYBD_CLICK_ON_MED\n",
		    errno);
#endif /* XDEBUG */
	    }
	} else if (pKbCtrl->click <= 100) {
	    /*
	     * Make ioctl() call to /dev/bell to:
	     *      Set KeyClick volume to LOUD
	     */
	    if (ioctl(pegInfo.bells.fd, BELL_KEYBD_CLICK_ON_LOUD, (char*)0)
		== -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for BELL_KEYBD_CLICK_ON_LOUD\n",
		    errno);
#endif /* XDEBUG */
	    }
	}
    }

    /*
     * pKbCtrl->bell:
     *	The DIX layer handles the base volume for the bell, however, ...
     *  the DIX stored base volume level is applied only with a call
     *  to xtlBell().  Ringing of the bell through any other means
     *  (e.g., echo "^G") would result in a volume as set by the
     *  last xtlBell() call.  Therefore, we set the base volume here!
     */
    setXTLBellVolume(pKbCtrl->bell);

    /*
     * pKbCtrl->bell_pitch:
     *	not possible on XTL keyboard.
     */

    /*
     * pKbCtrl->bell_duration:
     *
     * The Kernel stores the bell duration that is set via ioctl().
     *
     * For the XTL keyboard, the duration (pKbCtrl->bell_duration) is
     * established by the low-order byte (lbval) of the "RING BELL" command
     * (4X hex) and equals the decimal value of lbval times 40 milliseconds
     * (pKbCtrl->bell_duration=lbval*40 msec).
     * If the bell_duration >= 0 then convert to an lbval.
     * Note that if duration <= 0 then the command 40 (Key Click 2.5 msec) will
     * be executed.
     */

    if (pegInfo.bells.fd > 0) {
        /*
         * Was able to open /dev/xdev
         */
	if (pKbCtrl->bell_duration > (0xf * 40)) {
	    lbval = 0xf;
	} else if (pKbCtrl->bell_duration > 0) {
	    lbval = (int)(pKbCtrl->bell_duration/40) & 0xf;
	}
	if (ioctl(pegInfo.bells.fd, BELL_KEYBD_BELL_DURATION_SET,
	  (char *)(&lbval)) == -1 ) {
#ifdef XDEBUG
	    ErrorF("ioctl() returns error %d for BELL_KEYBD_BELL_DURATION_SET\n",
		errno);
#endif /* XDEBUG */
	}
    }

    /*
     * pKbCtrl->leds:
     *
     * The XTL keyboard has 3 LEDs that are worth manipulating;  these are
     *     name         X LED #
     *     ----------   -------
     *     Hold Screen = LED 1
     *     Compose     = LED 2
     *     Wait        = LED 3
     *     Tek         = LED 4
     *     Xmt         = LED 5
     *     Rcv         = LED 6
     * The Lock LED is reserved to be used by the system to indicate when the
     * Lock key is considered on.
     */

    if (pegInfo.eventFd > 0) {
        /*
         * Was able to open /dev/xdev
         */

	/*
	 * HOLD SCREEN
	 */

	if (pKbCtrl->leds & 0x1) {
	    if (ioctl(pegInfo.eventFd, KEYBD_HOLD_LED_ON, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_HOLD_LED_ON\n", errno);
#endif /* XDEBUG */
	    }
	} else {
	    if (ioctl(pegInfo.eventFd, KEYBD_HOLD_LED_OFF, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_HOLD_LED_OFF\n", errno);
#endif /* XDEBUG */
	    }
	}

	/*
	 * COMPOSE
	 */
	 if (pKbCtrl->leds & 0x2) {
	    if (ioctl(pegInfo.eventFd, KEYBD_COMPOSE_LED_ON, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_COMPOSE_LED_ON\n", errno);
#endif /* XDEBUG */
	    }
	 } else {
	    if (ioctl(pegInfo.eventFd, KEYBD_COMPOSE_LED_OFF, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_COMPOSE_LED_OFF\n", errno);
#endif /* XDEBUG */
	    }
	 }

	 /*
	  * WAIT
	  */
	 if (pKbCtrl->leds & 0x4) {
	    if (ioctl(pegInfo.eventFd, KEYBD_WAIT_LED_ON, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_WAIT_LED_ON\n", errno);
#endif /* XDEBUG */
	    }
	 } else {
	    if (ioctl(pegInfo.eventFd, KEYBD_WAIT_LED_OFF, (char*)0)) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_WAIT_LED_OFF\n", errno);
#endif /* XDEBUG */
	    }
	 }

	/*
	 * TEK
	 */
	 if (pKbCtrl->leds & 0x8) {
	    if (ioctl(pegInfo.eventFd, KEYBD_TEK_LED_ON, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_TEK_LED_ON\n", errno);
#endif /* XDEBUG */
	    }
	 } else {
	    if (ioctl(pegInfo.eventFd, KEYBD_TEK_LED_OFF, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_TEK_LED_OFF\n", errno);
#endif /* XDEBUG */
	    }
	 }

	/*
	 * XMT
	 */
	 if (pKbCtrl->leds & 0x10) {
	    if (ioctl(pegInfo.eventFd, KEYBD_XMT_LED_ON, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_XMT_LED_ON\n", errno);
#endif /* XDEBUG */
	    }
	 } else {
	    if (ioctl(pegInfo.eventFd, KEYBD_XMT_LED_OFF, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_XMT_LED_OFF\n", errno);
#endif /* XDEBUG */
	    }
	 }

	/*
	 * RCV
	 */
	 if (pKbCtrl->leds & 0x20) {
	    if (ioctl(pegInfo.eventFd, KEYBD_RCV_LED_ON, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_RCV_LED_ON\n", errno);
#endif /* XDEBUG */

	    }
	 } else {
	    if (ioctl(pegInfo.eventFd, KEYBD_RCV_LED_OFF, (char*)0) == -1) {
#ifdef XDEBUG
		ErrorF("ioctl() returns error %d for KEYBD_RCV_LED_OFF\n", errno);
#endif /* XDEBUG */
	    }
	 }
    }
}

static void
pegBell(loud, pDevice)
    int loud;
    DevicePtr pDevice;
{
    int i;

    assert(loud >= 0 && loud <= 100);

    /*
     * We have only 8 distinct bell volumes, named 0-7
     */
    i = loud / 14;

    if (pegInfo.bells.fd > 0 && pegInfo.bells.len[ i ])
	write(pegInfo.bells.fd,
		pegInfo.bells.str[ i ],
		pegInfo.bells.len[ i ]);
}

/*
 *	NAME
 *		xtlBell - ring bell on XTL Keyboard
 *
 *	SYNOPSIS
 */
static void
xtlBell(volume, pDevice)
    int volume;		/* in: volume (0-100)		*/
    DevicePtr pDevice;	/* in: ptr to keyboard device	*/
/*
 *	DESCRIPTION
 *		This routines sets the bell volume level then rings
 *		the bell.  DIX has already applied the base volume
 *		level to the client requested adjustment level.
 *
 *	RETURNS
 *		None
 */
{
    assert(volume >= 0 && volume <= 100);

    if (pegInfo.bells.fd < 0) {
        /*
         * Unable to open /dev/bell
         */
        return;
    }

    /*
     * Set the XTL keyboard volume before ringing bell.
     */
    setXTLBellVolume(volume);

    /*
     * Finally, RING THE BELL!!
     */
    write(pegInfo.bells.fd, "", 0);

    /*
     * Reset the XTL keyboard volume to base volume.
     */
    volume = ((DeviceIntPtr)pDevice)->kbdfeed->ctrl.bell;
    setXTLBellVolume(volume);
}

static void
pegChangePointerControl(pDevice, ctrl)
    DevicePtr pDevice;
    PtrCtrl   *ctrl;
{
    SvcCursorSpeed new;
    int acceleration;

    if (!(new.multiplier = ctrl->num / ctrl->den))
	new.multiplier = 1;	/* watch for den > num */
    new.threshold = ctrl->threshold;
    new.divisor = 1;
    if (SetCursorSpeed(&new) < 0)
	Error("SetCursorSpeed");
}

    
Bool
pegSetCursorPosition( pScr, newx, newy, generateEvent)
    ScreenPtr	pScr;
    int		newx;
    int		newy;
    Bool	generateEvent;
{
    xEvent	motion;
    int		xlimit, ylimit;
    SvcCursorXY new;
    static Bool	firstTime = True;

    if (firstTime) {
#ifdef	UTEK
	if (CPU_BOARD(pegInfo.softp) == HC_CPU_4405PLUS) {
	    debug4(("first-time cursor position shifted from (%d,%d) to ",
		newx, newy));
	    newx = SCREEN_X(pegInfo.softp) / 2;
	    newy = SCREEN_Y(pegInfo.softp) / 2;
	}
#endif	/* UTEK */
	firstTime = False;
    }
    xlimit = pegInfo.scrWidth-1;
    ylimit = pegInfo.scrHeight-1;
    new.x = newx;
    new.y = newy;

    /*
     * Insure the position is still visible
     */
    if (new.x < 0)
	new.x = 0;
    else if (new.x > xlimit)
	new.x = xlimit;

    if (new.y < 0)
	new.y = 0;
    else if (new.y > ylimit)
	new.y = ylimit;

    debug4(("SetCursorPosition(%d,%d)-->(%d,%d), %sgenerate an event\n",
	newx, newy, new.x, new.y, generateEvent ? "" : "don't"));
#ifdef  XTESTEXT1
    LastMousePosition.x = new.x;
    LastMousePosition.y = new.y;
#endif  /* XTESTEXT1 */
    /*
     * Use ioctl to move cursor without causing an event to get
     * generated.  If this OS does not have this ioctl, just use
     * SetCursorPoint which may or may not actually generate an event
     */
    if (ioctl(pegInfo.eventFd, CE_NEW_MOUSE_POSITION_NO_EVT,
	     (char *)(&new)) == -1) {

	if (SetCursorPoint(&new) < 0)
	    Error("SetCursorPoint");
    }
    /*
     * XXX - these don't get updated 'till the next vertical sync, so
     * we could force them here.  But we don't because that suppresses an
     * event being generated (very useful because of the 4405 hack above).
     *
     * pegInfo.dsp->ds_x = new.x;
     * pegInfo.dsp->ds_y = new.y;
     */

    if (generateEvent)
    {
	if (pegInfo.queue->head != pegInfo.queue->tail)
	    ProcessInputEvents();
	motion.u.keyButtonPointer.rootX = newx;
	motion.u.keyButtonPointer.rootY = newy;
	motion.u.keyButtonPointer.time = pegInfo.lastEventTime;
	motion.u.u.type = MotionNotify;
	(*pegInfo.pPointer->processInputProc) (&motion, pegInfo.pPointer, 1);
    }

    return TRUE;
}

/*
 *	NAME
 *		pegRecolorCursor - Change cursor color
 *
 *	SYNOPSIS
 */
void
pegRecolorCursor( pScr, pCurs, displayed)
    ScreenPtr   pScr;
    CursorPtr   pCurs;
    Bool        displayed;	/* TRUE if this cursor is currently displayed */
/*
 *	DESCRIPTION
 *		Dix is telling us that the colors in the pCurs structure
 *		have changed.  Since we are not storing color info in the
 *		devPrivate field of pCurs we do not need to update that
 *		structure.  We must change the color of the current
 *		cursor if parameter "displayed" is TRUE.
 *
 *	RETURNS
 *		None
 *
 */
{
    /*
     * Calling DisplayCursor does a little more work than is necessary but
     * makes the process simpler to code.  All we really need to do is call
     * SetCursorColor.
     */
    if (displayed)
	(* pScr->DisplayCursor)( pScr, pCurs);

}

Bool
pegDisplayCursor(pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
{
    SvcCursorFormPtr	new;
    SvcCursorColor	colors;

#ifdef XDEBUG
    if (xflg_debug & 0x10) {
	x_debug("DisplayCursor %dX%d, hot=(%d,%d)\n",
	    pCurs->width, pCurs->height,
	    pCurs->xhot, pCurs->yhot);
	x_debug("cursor colors=<%x %x %x><%x %x %x>\n",
	    pCurs->foreRed,
	    pCurs->foreGreen,
	    pCurs->foreBlue,
	    pCurs->backRed,
	    pCurs->backGreen,
	    pCurs->backBlue);
    }
#endif /* XDEBUG */

    if (pegInfo.depth == 1) {
	unsigned short r, g, b;

	/*
	 * We are assuming here that if depth is one (-mono option or a
	 * 4316/4406+), then there is only one visual: the 0th one for the
	 * screen.  Otherwise we must call the screen ListInstalledColormaps()
	 * and lookup the visual associated with it.
	 */
	r = pCurs->foreRed;
	g = pCurs->foreGreen;
	b = pCurs->foreBlue;
	(*pScr->ResolveColor)(&r, &g, &b, &pScr->visuals[0]);
	colors.redFore = r;
	colors.greenFore = g;
	colors.blueFore = b;

	r = pCurs->backRed;
	g = pCurs->backGreen;
	b = pCurs->backBlue;
	(*pScr->ResolveColor)(&r, &g, &b, &pScr->visuals[0]);
	colors.redBack = r;
	colors.greenBack = g;
	colors.blueBack = b;
    } else {
#ifdef NOTDEF
	if (!FGammaCorrectionDisabled) {
	    DisplayLookUpGammaCorrectedColor(
		    (unsigned short) pCurs->foreRed,
		    (unsigned short) pCurs->foreGreen,
		    (unsigned short) pCurs->foreBlue,
		    &colors.redFore,
		    &colors.greenFore,
		    &colors.blueFore);
	    DisplayLookUpGammaCorrectedColor(
		    (unsigned short) pCurs->backRed,
		    (unsigned short) pCurs->backGreen,
		    (unsigned short) pCurs->backBlue,
		    &colors.redBack,
		    &colors.greenBack,
		    &colors.blueBack);
	}
 	else
#endif
 	{
	    colors.redFore   = (short) pCurs->foreRed;
	    colors.greenFore = (short) pCurs->foreGreen;
	    colors.blueFore  = (short) pCurs->foreBlue;
	    colors.redBack   = (short) pCurs->backRed;
	    colors.greenBack = (short) pCurs->backGreen;
	    colors.blueBack  = (short) pCurs->backBlue;
	}
    }
    if (SetCursorColor(&colors) < 0)
	Error("SetCursorColor");

    /*
     * load the cursor
     */
    new = (SvcCursorFormPtr) pCurs->devPriv[pScr->myNum];
    new->xoff = pCurs->bits->xhot;
    new->yoff = pCurs->bits->yhot;
    if (SetCursorSourceAndMask(new))
	Error("SetCursorSourceAndMask");
}

void
pegPointerNonInterestBox( pScr, pBox)
    ScreenPtr	pScr;
    BoxPtr	pBox;
{

    debug5(("PointerNonInterestBox: Box = x1 %d y1 %d x2 %d y2 %d\n",
	pBox->x1, pBox->y1, pBox->x2, pBox->y2));
    pegInfo.dsp->ds_box.bottom = pBox->y2;
    pegInfo.dsp->ds_box.top = pBox->y1;
    pegInfo.dsp->ds_box.left = pBox->x1;
    pegInfo.dsp->ds_box.right = pBox->x2;
}

void
pegConstrainCursor( pScr, pBox)
    ScreenPtr	pScr;
    BoxPtr	pBox;
{
    SvcCursorXY	upperLeft, lowerRight;

    debug4(("ConstrainCursor(0x%x)@(%d,%d),(%d,%d)\n", pScr,
	pBox->x1, pBox->y1, pBox->x2, pBox->y2));

    pegInfo.constraintBox = *pBox;
#ifdef XPEG_TANDEM
    if (!pegTandem)
#endif /* XPEG_TANDEM */
    {
	upperLeft.x = pBox->x1;
	upperLeft.y = pBox->y1;
	lowerRight.x = pBox->x2;
	lowerRight.y = pBox->y2;
	SetCursorBounds(&upperLeft, &lowerRight);
    }
}

void
pegCursorLimits( pScr, pCurs, pHotBox, pTopLeftBox)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
    BoxPtr	pHotBox;
    BoxPtr	pTopLeftBox;	/* return value */
{
    pTopLeftBox->x1 = max( pHotBox->x1, 0);
    pTopLeftBox->y1 = max( pHotBox->y1, 0);
    pTopLeftBox->x2 = min( pHotBox->x2, pegInfo.width);
    pTopLeftBox->y2 = min( pHotBox->y2, pegInfo.height);
}

/*
 * Copy data in (pegasus) bitmap format to fill a 64 x 64 bitmap.
 * Pad with zeros.
 */
static void
CopyCursorImage(src, dest, width, height)
    long *src, *dest;
    int width, height;
{
    int x, y;
    long edgeMask;

    /*
     * We know pegasus bitmaps have four bytes per scanline if width <= 32,
     * eight bytes per scanline if 32 < width <= SvcCursorSize (64).
     * Set all unused bits to zero, only mask when needed.
     */
    if (width < 32) {
	edgeMask = 0xffffffff << (32 - width);
	for (y = 0; y < height; y++) {
	    *dest++ = src[y] & edgeMask;
	    *dest++ = 0;
	}
    } else if (width == 32) {
	for (y = 0; y < height; y++) {
	    *dest++ = src[y];
	    *dest++ = 0;
	}
    } else if (width < SvcCursorSize) {
	edgeMask = 0xffffffff << (SvcCursorSize - width);
	for (y = 0; y < 2 * height; y += 2) {
	    *dest++ = src[y];
	    *dest++ = src[y+1] & edgeMask;
	}
    } else {
	for (y = 0; y < 2 * height; y += 2) {
	    *dest++ = src[y];
	    *dest++ = src[y+1];
	}
    }
    /* zero the remaining lines */
    for (y = height; y < SvcCursorSize; y++) {
	*dest++ = 0;
	*dest++ = 0;
    }
}

Bool
pegRealizeCursor( pScr, pCurs)
    ScreenPtr pScr;
    CursorPtr	pCurs;	/* a SERVER-DEPENDENT cursor */
{
    SvcCursorForm	*data;

    data = (SvcCursorForm *)Xalloc(sizeof(SvcCursorForm));
    if (!data)
	return(FALSE);

    if (pCurs->bits->height < SvcCursorSize
     || pCurs->bits->width < SvcCursorSize) {
	data->src = (unsigned long *)Xalloc(SvcCursorDataSize * sizeof(long));
	data->mask = (unsigned long *)Xalloc(SvcCursorDataSize * sizeof(long));
	if (!data->src || !data->mask)
	    return(FALSE);
	CopyCursorImage((long *)pCurs->bits->source, data->src,
		pCurs->bits->width, pCurs->bits->height);
	CopyCursorImage((long *)pCurs->bits->mask, data->mask,
		pCurs->bits->width, pCurs->bits->height);
    } else {
	data->src = (unsigned long *)pCurs->bits->source;
	data->mask = (unsigned long *)pCurs->bits->mask;
    }

    pCurs->devPriv[pScr->myNum] = (pointer)data;

    return(TRUE);  /* Failure can happen only in Xalloc() */
}

Bool
pegUnrealizeCursor( pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
{
    SvcCursorForm	*data = (SvcCursorForm *)pCurs->devPriv[pScr->myNum];

    if (data->src != (unsigned long *)pCurs->bits->source) {
	Xfree(data->src);
	Xfree(data->mask);
    }
    Xfree(data);

    return(TRUE);
}

/*
 *	NAME
 *		pegQueryBestSize - return best sizes for cursor and patterns
 *
 *	SYNOPSIS
 */
void
pegQueryBestSize(class, pwidth, pheight)
    int class;		/* in: CursorShape, TileShape, or StippleShape */
    short *pwidth;	/* in/out: width */
    short *pheight;	/* in/out: height */
/*
 *	DESCRIPTION
 *		Return best sizes for cursors, tiles, and stiples in
 *		response to client requests.  For CursorShape, return
 *		the maximum width and height for cursors that we can
 *		handle.  For TileShape and StippleShape, start with the
 *		suggested values in *pwidth and  *pheight and modify them
 *		in place to be optimal values that are greater than or
 *		equal to the suggested values.
 *
 *	RETURNS
 *		None
 *
 */
{

    switch(class) {
    case CursorShape:
	*pwidth = SvcCursorSize;
	*pheight = SvcCursorSize;
	break;

    /*
     * Return nearest, multiple of 32 not less than width
     * Do nothing for height
     */
    case TileShape:
    case StippleShape:
	if (*pwidth <= 32)
	    *pwidth = 32;
	else
	    *pwidth = (*pwidth + 31) & ~0x1f;

	    /* We don't care what height they use */
	    break;
    }
}
