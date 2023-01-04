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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/pegEvent.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
/***********************************************************
Copyright 1987 by Tektronix, Beaverton, Oregon,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Tektronix or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

TEKTRONIX DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "keysym.h"
#include "inputstr.h"
#include "miscstruct.h"
#include "screenint.h"
#include "keynames_xtl.h"

#ifdef	UTEK
#include <box/keyboard.h>
#endif	/* UTEK */

#ifdef	UTEKV
#include "redwing/keyboard.h"
#endif	/* UTEKV */

#include "peg.h"

/*
 * Test Consortium Input Extension Variables
 */
#ifdef XTESTEXT1
SvcCursorXY LastMousePosition;
#endif /* XTESTEXT1 */

extern InitInfo pegInfo;
extern CARD16 keyModifiersList[];
extern int screenIsSaved;
static Bool	KanaShiftState;
static Bool	KanaLocked;
static Bool	readyToUnlock;
static KeyCode	InitialKanaLockKey;
static int	NumKanaLocks;
static int	NumKanaShifts;

#ifdef XDEBUG
static char *type[] = {
    "button",
    "mmotion",
    "tmotion",
    "timeout",
    "pan",
    "???",
    "???",
    "???",
};

static char *direction[] = {
    "kbtup",
    "kbtdown",
    "kbtraw",
    "???",
};

static char *device[] = {
    "???",
    "mouse",
    "dkb",
    "tablet",
    "aux",
    "console",
    "???",
    "???",
};
#endif /* XDEBUG */

static Event
*RepeatKey()
{
    static Event	ev;
    register		c = pegInfo.kv.timer[ KEYREPEAT ].key + MINKEYCODE,
			index;
    XDisplayState   	*dsp;

    /*
     * The protocol says that we should deliver up AND down events
     * for auto-repeating keys.  But we need not worry, because
     * ProcessKeyboardEvent() in dix/events.c delivers an extra up event
     * for us if it sees two down events.
     */
    debug10(("key 0x%x timeout", pegInfo.kv.timer[ KEYREPEAT ].key));
    if (KeyDepressed(c)) {
	dsp = pegInfo.dsp;
	ev.e_x = dsp->ds_x;
	ev.e_y = dsp->ds_y;
	ev.e_type = E_BUTTON;
	ev.e_device = E_DKB;
	ev.e_direction = E_KBTDOWN;
	ev.e_key = pegInfo.kv.timer[ KEYREPEAT ].key;
	ev.e_time = (dsp->ds_ltime%1000)/10;
	debug10(("\n"));
    } else {
	pegInfo.kv.timer[ KEYREPEAT ].key = -1;
	debug10(("... already up\n", c));
	return(NULL);
    }

    /*
     * figure out next timeout.
     */
    index = pegInfo.kv.timer[ KEYREPEAT ].delay;
    if (++index >= pegInfo.kv.nKeyDelays)
	index = pegInfo.kv.nKeyDelays - 1;
    pegInfo.kv.timer[ KEYREPEAT ].delay = index;
    pegInfo.kv.timer[ KEYREPEAT ].when =
	dsp->ds_ltime + pegInfo.kv.keyDelays[ index ];
    return(&ev);
}

static void
pan4405()
{
    register XDisplayState  *dsp = pegInfo.dsp;
    register down;
#ifdef	UTEK
    register x, y, deltax, deltay;
    SvcCursorXY	newViewPort;
#endif	/* UTEK */

    int delay;

    /*
     * First, decide what keys are being repeated.
     */
#ifdef	UTEK
    debug11(("%x: pan(x,y)=(%s,%s) xydelay=(%d,%d)",
	dsp->ds_ltime / dsp->ds_ltimeinc,
	JoyEWDepressed ? "down" : "up",
	JoyNSDepressed ? "down" : "up",
	pegInfo.kv.timer[ JOYEWREPEAT ].delay,
	pegInfo.kv.timer[ JOYNSREPEAT ].delay));
#endif	/* UTEK */

    down = JoyDepressed;

    if (!down && pegInfo.kv.panStop) {
	pegInfo.kv.timer[ JOYEWREPEAT ].key = -1;
	pegInfo.kv.timer[ JOYEWREPEAT ].delay = 0;
	pegInfo.kv.timer[ JOYNSREPEAT ].key = -1;
	pegInfo.kv.timer[ JOYNSREPEAT ].delay = 0;
	debug11(("\n"));
	return;
    }

    if ((down & JOYEWMASK) == 0) {
	if ((pegInfo.kv.timer[ JOYEWREPEAT ].delay
	  -= pegInfo.kv.panInertia) < 0) {
	    pegInfo.kv.timer[ JOYEWREPEAT ].key = -1;
	    pegInfo.kv.timer[ JOYEWREPEAT ].delay = 0;
	} else {
	    if (pegInfo.kv.timer[ JOYEWREPEAT ].key == KBJoyRight)
		down |= JOYRIGHT_BIT;
	    else
		down |= JOYLEFT_BIT;
	}
    }

    if ((down & JOYNSMASK) == 0) {
	if ((pegInfo.kv.timer[ JOYNSREPEAT ].delay
	  -= pegInfo.kv.panInertia) < 0) {
	    pegInfo.kv.timer[ JOYNSREPEAT ].key = -1;
	    pegInfo.kv.timer[ JOYNSREPEAT ].delay = 0;
	} else {
	    if (pegInfo.kv.timer[ JOYNSREPEAT ].key == KBJoyUp)
		down |= JOYUP_BIT;
	    else
		down |= JOYDOWN_BIT;
	}
    }

#ifdef	UTEK
    /*
     * Now see if the X panning can change.
     */
    GetViewport(&newViewPort);
    if (down & (JOYLEFT_BIT|JOYRIGHT_BIT)) {
	/*
	 * Figure out the next timeout ASAP.
	 */
	delay = pegInfo.kv.timer[ JOYEWREPEAT ].delay;
	if (JoyEWDepressed) {
	    if (++pegInfo.kv.timer[ JOYEWREPEAT ].delay
	      >= pegInfo.kv.nPanDelays)
		pegInfo.kv.timer[ JOYEWREPEAT ].delay =
			pegInfo.kv.nPanDelays - 1;
	}
	pegInfo.kv.timer[ JOYEWREPEAT ].when =
	    dsp->ds_ltime + pegInfo.kv.panDelays[ delay ];

	/*
	 * ... and then compute delta x.
	 */
	deltax = pegInfo.kv.panDeltaX[ pegInfo.kv.timer[ JOYEWREPEAT ].delay ];
	if (down & JOYLEFT_BIT)
	    deltax = -deltax;
	x = newViewPort.x + deltax;
	if (x < 0)
	    x = 0;
	else if (x > dsp->ds_panxmax)
	    x = dsp->ds_panxmax;
	if (newViewPort.x != x) {
	    deltax = x - newViewPort.x;
	    newViewPort.x = x;
	} else {
	    deltax = 0;
	    pegInfo.kv.timer[ JOYEWREPEAT ].key = -1;
	    pegInfo.kv.timer[ JOYEWREPEAT ].delay = 0;
	}
    } else {
	x = newViewPort.x;
	deltax = 0;
    }
#endif	/* UTEK */

#ifdef	UTEKV
    /*
     * Now see if the X axis panning buttons can change.
     */
    if (down & (JOYLEFT_BIT|JOYRIGHT_BIT)) {
	/*
	 * Figure out the next timeout ASAP.
	 */
	delay = pegInfo.kv.timer[ JOYEWREPEAT ].delay;
	if (JoyEWDepressed) {
	    if (++pegInfo.kv.timer[ JOYEWREPEAT ].delay
	      >= pegInfo.kv.nPanDelays)
		pegInfo.kv.timer[ JOYEWREPEAT ].delay =
			pegInfo.kv.nPanDelays - 1;
	}
	pegInfo.kv.timer[ JOYEWREPEAT ].when =
	    dsp->ds_ltime + pegInfo.kv.panDelays[ delay ];
    }
#endif	/* UTEKV */

#ifdef	UTEK
    /*
     * ... and see if the Y panning can change.
     */
    if (down & (JOYUP_BIT|JOYDOWN_BIT)) {
	/*
	 * Figure out the next timeout ASAP.
	 */
	delay = pegInfo.kv.timer[ JOYNSREPEAT ].delay;
	if (JoyNSDepressed) {
	    if (++pegInfo.kv.timer[ JOYNSREPEAT ].delay
	     >= pegInfo.kv.nPanDelays)
		pegInfo.kv.timer[ JOYNSREPEAT ].delay =
			pegInfo.kv.nPanDelays - 1;
	}
	pegInfo.kv.timer[ JOYNSREPEAT ].when =
	    dsp->ds_ltime + pegInfo.kv.panDelays[ delay ];

	/*
	 * And compute delta y.
	 */
	deltay = pegInfo.kv.panDeltaY[ pegInfo.kv.timer[ JOYNSREPEAT ].delay ];
	if (down & JOYUP_BIT)
	    deltay = -deltay;
	y = newViewPort.y + deltay;
	if (y < 0)
	    y = 0;
	else if (y > dsp->ds_panymax)
	    y = dsp->ds_panymax;
	if (newViewPort.y != y) {
	    deltay = y - newViewPort.y;
	    newViewPort.y = y;
	} else {
	    deltay = 0;
	    pegInfo.kv.timer[ JOYNSREPEAT ].key = -1;
	    pegInfo.kv.timer[ JOYNSREPEAT ].delay = 0;
	    if (deltax == 0)
		return;
	}
    } else {
	deltay = 0;
    }

    debug11((" pan-->(%d,%d)", newViewPort.x, newViewPort.y));
    SetViewport(&newViewPort);
#endif	/* UTEK */

#ifdef	UTEKV
    /*
     * Now see if the Y axis panning buttons can change.
     */
    if (down & (JOYUP_BIT|JOYDOWN_BIT)) {
	/*
	 * Figure out the next timeout ASAP.
	 */
	delay = pegInfo.kv.timer[ JOYNSREPEAT ].delay;
	if (JoyNSDepressed) {
	    if (++pegInfo.kv.timer[ JOYNSREPEAT ].delay
	     >= pegInfo.kv.nPanDelays)
		pegInfo.kv.timer[ JOYNSREPEAT ].delay =
			pegInfo.kv.nPanDelays - 1;
	}
	pegInfo.kv.timer[ JOYNSREPEAT ].when =
	    dsp->ds_ltime + pegInfo.kv.panDelays[ delay ];
    }
#endif	/* UTEK */
}

static void
SetupTimeout()
{
    register XDisplayState  *dsp = pegInfo.dsp;
    register    i, when = 0;

    for (i=0; i<N_TIMERS; i++) {
	if (pegInfo.kv.timer[ i ].key >= 0) {
	    if (pegInfo.kv.timer[ i ].when < dsp->ds_ltime)
		pegInfo.kv.timer[ i ].when = dsp->ds_ltime;
	    if (when == 0 || pegInfo.kv.timer[ i ].when < when)
		when = pegInfo.kv.timer[ i ].when;
	}
    }

    dsp->ds_timeout = when;
}

static void
SetupRepeat(c)
    register    c;
{
    register XDisplayState  *dsp = pegInfo.dsp;
    Bool	somethingRepeated = False;
    register	mappedKey;

    debug10(("repeat 0x%x\n", c));
    switch(c) {
    case KBJoyLeft:
    case KBJoyRight:
	if (! pegInfo.kv.panEnabled)
		goto normalRepeat;
	if (pegInfo.kv.timer[ JOYEWREPEAT ].key == c) {
	    debug10(("0x%x already repeating\n",
		c, pegInfo.kv.timer[ JOYEWREPEAT ].key));
	    break;
	}
	pegInfo.kv.joyState |= JoyKeyToMask(c);
	pegInfo.kv.timer[ JOYEWREPEAT ].delay = 0; /* an index */
	pegInfo.kv.timer[ JOYEWREPEAT ].key = c;
	pegInfo.kv.timer[ JOYEWREPEAT ].when = dsp->ds_ltime;
	somethingRepeated = True;
#ifdef XPEG_TANDEM
	if (pegTandem)
	    break;
#endif /* XPEG_TANDEM */
	pan4405();
	break;
    case KBJoyUp:
    case KBJoyDown:
	if (! pegInfo.kv.panEnabled)
		goto normalRepeat;
	if (pegInfo.kv.timer[ JOYNSREPEAT ].key == c) {
	    debug10(("0x%x already repeating\n",
		c, pegInfo.kv.timer[ JOYNSREPEAT ].key));
	    break;
	}
	pegInfo.kv.joyState |= JoyKeyToMask(c);
	pegInfo.kv.timer[ JOYNSREPEAT ].delay = 0; /* an index */
	pegInfo.kv.timer[ JOYNSREPEAT ].key = c;
	pegInfo.kv.timer[ JOYNSREPEAT ].when = dsp->ds_ltime;
	somethingRepeated = True;
#ifdef XPEG_TANDEM
	if (pegTandem)
	    break;
#endif /* XPEG_TANDEM */
	pan4405();
	break;
    default:
normalRepeat:
	if (pegInfo.kv.timer[ KEYREPEAT ].key == c) {
	    debug10(("0x%x already repeating\n",
		c, pegInfo.kv.timer[ KEYREPEAT ].key));
	    break;
	}

	/*
	 * If there is already a key repeating, then push it on the stack.
	 */
	if (pegInfo.kv.timer[ KEYREPEAT ].key >= 0) {
	    pegInfo.kv.keyStack[ pegInfo.kv.keyTOS++ ] =
		pegInfo.kv.timer[ KEYREPEAT ].key;
	    pegInfo.kv.timer[ KEYREPEAT ].key = -1;
	}
	/*
	 * The protocol says it is desirable that keys being
	 * used as modifiers not auto-repeat, reguardless of
	 * their auto-repeat setting.
	 */
	mappedKey = c+MINKEYCODE;
	if (! AutoRepeatOn()
	 || ! KeyRepeatable(mappedKey)
	 || KeyIsModifier(mappedKey))
	    break;

	pegInfo.kv.timer[ KEYREPEAT ].key = c;
	pegInfo.kv.timer[ KEYREPEAT ].delay = 0; /* an index */
	pegInfo.kv.timer[ KEYREPEAT ].when =
	    dsp->ds_ltime + pegInfo.kv.keyDelays[ 0 ];
	somethingRepeated = True;
	break;
    }

    if (somethingRepeated)
	SetupTimeout();
}

/*
 * Pop the previous key being repeated off the stack and start
 * repeating it.
 * We are guarenteed that pegInfo.kv.keyTOS > 0.
 */
static void
PopKeyRepeat(c)
    register    c;
{
    int i;

    /*
     * First, we purge the key stack of the key that just
     * got released... it may not be the top of stack.
     */
    debug10(("poprepeat stack: "));
    for (i=0; i<pegInfo.kv.keyTOS; i++)
	if (pegInfo.kv.keyStack[ i ] == c) {
	    pegInfo.kv.keyStack[ i ] = -1;
	    debug10((" (drop 0x%x)", c));
	} else
	    debug10((" 0x%x", pegInfo.kv.keyStack[ i ]));

    /*
     * If the key just released is NOT the one being repeated, we simply
     * let the repeat continue...
     */
    if (pegInfo.kv.timer[ KEYREPEAT ].key != -1) {
	if (pegInfo.kv.timer[ KEYREPEAT ].key == c)
	    pegInfo.kv.timer[ KEYREPEAT ].key = -1; /* cancel the repeat */
	else {
	    debug10((" allow 0x%x to continue\n",
		pegInfo.kv.timer[ KEYREPEAT ].key));
	    return;
	}
    }

    /*
     * Now peruse from TOS down, looking for a key to repeat.
     */
    do {
	c = pegInfo.kv.keyStack[ --pegInfo.kv.keyTOS ];
	if (c >= 0) {
	    SetupRepeat(c);
	    return;
	}
    } while (pegInfo.kv.keyTOS > 0);
    debug10((" stack now empty\n"));
}

static void
SetLockLED(on)
    Bool on;
{
    if (on)
	ioctl(pegInfo.eventFd, CE_SHIFTLOCKON, 0);
    else
	ioctl(pegInfo.eventFd, CE_SHIFTLOCKOFF, 0);
}

void
SetComposeLED(on)
    Bool on;
{
    if (on || (KanaShiftState == True))
	/* if we are in kana mode, keep ComposeLED on when Compose goes off */
	ioctl(pegInfo.eventFd, KEYBD_COMPOSE_LED_ON, 0);
    else
	ioctl(pegInfo.eventFd, KEYBD_COMPOSE_LED_OFF, 0);
}

#ifdef	UTEKV

#ifndef	NBPC
#define	NBPC	(4096)	/* number of bytes per click (page) */
#endif	/* NBPC */

#ifndef	NCPS
#define	NCPS	(1024)	/* number of clicks (pages) per segment */
#endif	/* NCPS */

#ifndef	NBPS
#define	NBPS	(NBPC*NCPS)	/* number of bytes per segment */
#endif	/* NBPS */

typedef struct _eventqueueFAKE {
	unsigned long offset;		/* input event buffer KVA - offset */
	int size;			/* size of event buffer */
	int head;			/* index into events */
	int tail;			/* index into events */
} EventQueueFAKE;


/* notes only --- */
/* #define PHYS_HIGH_ADDR (0x60000000) * Servers Virtual Address "offset" from
				       * the physical address. An area
				       * between u and display board.
				       */
#endif	/* UTEKV */

long
GetTimeInMillis ()
{
    long   ret = 0;
#ifdef __STDC__
    volatile long  *addr;
#else
    long   *addr;
#endif

    if (pegInfo.dsp)
    {
	addr = (long *) &pegInfo.dsp->ds_ltime;
	ret = *addr;
	if (ret == 0)
	    ret = *addr;
    }
    return ret;
}

void
ProcessInputEvents ()
{
    register Event *ev;
    register u_long c, i;
    register EventQueue *queue = (EventQueue *)pegInfo.pPointer->devicePrivate;
    xEvent  x;
    u_long  nowInCentiSecs, nowInMilliSecs;
    u_long  eventInCentiSecs, eventInMilliSecs;
    int qLimit;
    int count;
    KeyCode keycode;
    KeySym *keysyms;
    int j;

#ifdef	UTEKV
	/*
	 * Event "queue" address corrected to point
	 * to the Kernal Virtual Address.
	 */
    register EventQueueFAKE *queueKVA;
    register EventQueue *queueSVA;
    register caddr_t sparePtr;		/* a spare pointer for address calc. */
    register caddr_t SVAbase;		/* server virtual address segment base*/
    register Event *evSVA;   /* server virtual address of -the- current event */

#endif	/* UTEKV */

#ifdef XPEG_TANDEM
    if (pegTandem)
	pegReadEvents(queue);
#endif /* XPEG_TANDEM */

    i = queue->head;
    qLimit = queue->size - 1;

    while (i != queue->tail) {
	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);

#ifdef	UTEK
	ev = &queue->events[i];
#endif	/* UTEKV */

#ifdef	UTEKV
	/*
	 * The servers mapping (Server Virtual Address - SVA) of the event
	 * queue is at 0x60000000 + the physical address of the shared
	 * region is typically 0x24a000 or 0x24ac000. Which is currently
	 * the same as the kernel virtual mapping. The event queue contains
	 * kernel virtual addresses (KVA) in the
	 * "EventQueue ds_q  entry -> Event *events" pointer.
	 *
	 * Sooo.. I must contruct a pointer to the queue and then calculate
	 * the address of the event by adding the 0x6...0 value to the
	 * KVA as though it were an offset.
	 */
#ifdef notdef
		/*
		 * Make a copy of the servers pointer
		 * to the dsp.ds_q entry.
		 *
		 * Now add the SVA offset to the
		 * segment base address to compute the
		 * "real" address.
		 */
	sparePtr = (caddr_t) (((unsigned long) queue)
				+ ((unsigned long) SVAbase));

	sparePtr = (caddr_t) (((unsigned long)
				sparePtr) & (~(NBPS-1))); /*prune segment*/
	SVAbase = (caddr_t) (((unsigned long) SVAbase)
				+ ((unsigned long) sparePtr));
#endif	/* notdef */

	SVAbase = (caddr_t) ((unsigned long)queue & (~(NBPS-1)));
				/* strip off the lower part of the
				 * address to get the segment address.
				 */

		/*
		 * Now fetch the KVA and prune the segment base address
		 * to the event queue kernel virtual address to compute
		 * the "real" address in SVA space.
		 */
	queueKVA = (EventQueueFAKE *) queue;	/* make "FAKE" queue ptr */

	sparePtr = (caddr_t) ((unsigned long) ((unsigned long) SVAbase) +
				((unsigned long) queueKVA->offset));
			/* add the KVA as though it were just an offset */

	sparePtr = (caddr_t) ((unsigned long) ((unsigned long) sparePtr)
					+ ((sizeof (Event)) * i));

	ev = (Event *)sparePtr;

#endif	/* UTEKV */


#ifndef MOTION_BUFFER_OFF
	if ((ev->e_type != E_MMOTION) &&
	  (pegInfo.motionQueue->mouseMoved == TRUE)) {
	    gfbSendMotionEvent(pegInfo.pPointer);
	}
#endif /* MOTION_BUFFER_OFF */

    anotherEvent:
	debug5(("(%d,%d) %s",
	    ev->e_x, ev->e_y, type[ ev->e_type&0x7 ]));

	x.u.keyButtonPointer.rootX = ev->e_x;
	x.u.keyButtonPointer.rootY = ev->e_y;
	/*
	 * The following silly looking code is because the old
	 * version of the driver only delivers 16 bits worth of
	 * centiseconds. We are supposed to be keeping time in
	 * terms of 32 bits of milliseconds.  Fortunately, this
	 * is available in shared space.
	 */
	nowInMilliSecs = pegInfo.dsp->ds_ltime;
	nowInCentiSecs = (nowInMilliSecs / 10);
	eventInCentiSecs = ev->e_time;
	eventInCentiSecs = (nowInCentiSecs & 0xFFFF0000) |
		           (eventInCentiSecs & 0x0000FFFF);
	if (eventInCentiSecs > nowInCentiSecs)
	    eventInCentiSecs -= 0x10000;
	x.u.keyButtonPointer.time = eventInCentiSecs * 10;
	switch (ev->e_type) {
	case E_BUTTON:
	    debug5((" @%d 0x%-2x %s %s\n",
		ev->e_time,
		ev->e_key,
		direction[ ev->e_direction&0x3 ],
		device[ ev->e_device&0x7 ]));
	    switch (ev->e_device) {
	    case E_MOUSE:
		x.u.u.type = (ev->e_direction == E_KBTUP) ?
		    ButtonRelease : ButtonPress;
		x.u.u.detail = ev->e_key;
#ifdef  XTESTEXT1
		/*
		 * If we are stealing input, then send info to the
		 * input extension.
		 */
		if (!on_steal_input ||
		    XTestStealKeyData(x.u.u.detail, x.u.u.type,
			    0,	/* dev_type: used for multiple input devices */
			    x.u.keyButtonPointer.rootX,
			    x.u.keyButtonPointer.rootY))
#endif  /* XTESTEXT1 */
		(*pegInfo.pPointer->processInputProc)(&x, pegInfo.pPointer, 1);
		break;

	    case E_AUX:
	    {
		register XDisplayState  *dsp;

		dsp = pegInfo.dsp;
		if ((pegInfo.kv.timer[ JOYNSREPEAT ].key >= 0
		 && pegInfo.kv.timer[ JOYNSREPEAT ].when <= dsp->ds_ltime)
		|| (pegInfo.kv.timer[ JOYEWREPEAT ].key >= 0
		 && pegInfo.kv.timer[ JOYEWREPEAT ].when <= dsp->ds_ltime))
		    pan4405();

		if (pegInfo.kv.timer[ KEYREPEAT ].key >= 0
		 && pegInfo.kv.timer[ KEYREPEAT ].when <= dsp->ds_ltime)
		    ev = RepeatKey();
		else
		    ev = NULL;

		SetupTimeout();
		if (ev)
		    goto anotherEvent;
		break;
	    }

	    case E_DKB:
		x.u.u.detail = ev->e_key + MINKEYCODE;
		c = ev->e_key;
		/*
		 * Hack attack!  If you hold down various key combinations,
		 * and then hit break, special things will happen...
		 */
		if (c == KBBreak && ev->e_direction == E_KBTDOWN) {
#ifdef XDEBUG
		    /*
		     * If you hold down the first four function keys
		     * and hit break, the server will exit.
		     */
		    if (RawKeyDepressed(KBF1)
		     && RawKeyDepressed(KBF2)
		     && RawKeyDepressed(KBF3)
		     && RawKeyDepressed(KBF4)) {
			debug6(("Bailing out\n"));
			exit(1);
		    }
		    /*
		     * If you hold down the left shift, the control key,
		     * a single letter and hit break, the debug bit
		     * corresponding to the letter (a=0, b=1, c=2, d=4, e=8...)
		     * will be toggled.
		     */
		    if (RawKeyDepressed(KBCtrl) && RawKeyDepressed(KBShiftL)) {
			int	key;
			x_debug("debug key... ");
			for (key=KBA+MINKEYCODE; key<=KBZ+MINKEYCODE; key++) {
			    if (KeyDepressed(key)) {
				key -= KBA+MINKEYCODE;
				xflg_debug = (xflg_debug & (~(1<<key)))
					| ((~xflg_debug) & (1<<key));
				x_debug("debug%d %s\n", key,
				    xflg_debug & (1<<key) ? "set" : "cleared");
				break;
			    }
			}
		    }
#endif /* XDEBUG */
		}
		switch (ev->e_direction) {
		case E_KBTUP:
		    x.u.u.type = KeyRelease;
		    if (pegInfo.kv.keyTOS > 0)
			PopKeyRepeat((int)c);
		    else
			pegInfo.kv.timer[ KEYREPEAT ].key = -1;
		    break;
		case E_KBTDOWN:
		    x.u.u.type = KeyPress;
		    if (pegInfo.kv.timer[ KEYREPEAT ].key != c)
			SetupRepeat((int)c);
		    break;
		}

		/*
		 * don't ever deliver joypad events unless panning is
		 * enabled.
		 */
		if (pegInfo.kv.panEnabled
		&& (c == KBJoyLeft
		 || c == KBJoyRight
		 || c == KBJoyUp
		 || c == KBJoyDown)) {
		    if (ev->e_direction == E_KBTUP)
			pegInfo.kv.joyState &= ~JoyKeyToMask(c);
		    break;
		}

		/*
		 * Implement a toggle for the caps lock led no matter
		 * what key is the caps lock. XXX this means no caps shift!
		 * If more than one lock key, we're in trouble!!!
		 */
		if (KeyIsModifier(x.u.u.detail) & LockMask) {
		    if (ev->e_direction == E_KBTDOWN)
			if (KeyDepressed(x.u.u.detail))
			    break; /* don't deliver the event */
			else
			    SetLockLED(True);
		    else /* E_KBTUP */
			if (pegInfo.kv.readyToUnlock) {
			    SetLockLED(False);
			    pegInfo.kv.readyToUnlock = False;
			} else {
			    pegInfo.kv.readyToUnlock = True;
			    break; /* don't deliver the event */
			}
		}

#ifdef SERVER_COMPOSE
		/* process Compose sequences; toss keycode if a match */
		/* Note that this bypasses the test code */
		if (CheckForCompose(x, (DeviceIntPtr) pegInfo.pKeyboard))
		    break;
#endif /* SERVER_COMPOSE */

#ifdef USE_KANA_STUFF
		/* process Katakana shift state */
		count = GetCurKeySymList(x.u.u.detail, &keysyms);
		for (j = 0; j < count; j++) {
		    switch (keysyms[j]) {
		    case XK_Kana_Lock:
		    /* its generic, so why not do it for everyone? */
		    case XK_Mode_switch:
			if (x.u.u.type == KeyPress) {
			    if (!NumKanaLocks++ && KanaLocked)
				readyToUnlock = True;
				/* unlock when NumKanaLocks goes to zero */
			    if (KanaLocked)
				/* trickery; toss all lock key events except
				   first/last press/release */
				goto big_break; /* ignore event; no chg */
			    else {
				/* act only when first of N locks goes down
				   and no shift is down.  Save initial 
				   Lock keycode for release */
				if (NumKanaLocks == 1) {
				    InitialKanaLockKey = x.u.u.detail;
				    KanaLocked = True;
				    if (!NumKanaShifts) {
					KanaShiftState = True;
					SetComposeLED(KanaShiftState);
			    /* Note that compose and kana are separate
			    states, and the LED is turned on for either. */
				    }
				}
			    }
			}
			else {
			    /* act only when the last of the locks pressed
			       while locked goes up, and no shift is down */
			    if (--NumKanaLocks)
				goto big_break; /* ignore event; no chg */
			    else {  /* all lock keys up */
				if (readyToUnlock) {
				    x.u.u.detail = InitialKanaLockKey;
				    KanaLocked = readyToUnlock = False;
				    if (!NumKanaShifts) {
					KanaShiftState = False;
					SetComposeLED(KanaShiftState);
				    }
				}
				else
				    goto big_break; /* ignore event; no chg */
			    }
			}
			goto little_break;
		    case XK_Kana_Shift:
			/* set/unset kana state to inverse of lock
			   when first/last shift released/depressed */
			if (x.u.u.type == KeyPress) {
			    if (!NumKanaShifts++) {
				if (KanaLocked)
				    KanaShiftState = False;
				else
				    KanaShiftState = True;
				SetComposeLED(KanaShiftState);
			    }
			}
			else	/* KeyRelease */
			    if (!--NumKanaShifts) {
				if (KanaLocked)
				    KanaShiftState = True;
				else
				    KanaShiftState = False;
				SetComposeLED(KanaShiftState);
			    }
			goto little_break;
		    default:	/* no Kana keysym, keep looking */
			break;
		    }
		}
/* ignore multiple Kana keysyms on one key */
little_break:

	    /* if Kana shift is on, convert kana keycodes to kana region. */
	    if (KanaShiftState) {
		keycode = x.u.u.detail + KANA_OFFSET;
		/* don't send Kana keycode if it has no keysym! */
		if (GetCurKeySymList(keycode, &keysyms))
		    x.u.u.detail = keycode;
	    }
#endif /* USE_KANA_STUFF */

#ifdef  XTESTEXT1
		/*
		 * If we are stealing input, then send info to the
		 * input extension.
		 */
		if (!on_steal_input ||
		    XTestStealKeyData(x.u.u.detail, x.u.u.type, 
			    0,	/* dev_type: useful for multiple keyboards */
			    x.u.keyButtonPointer.rootX,
			    x.u.keyButtonPointer.rootY))
#endif  /* XTESTEXT1 */
		(*pegInfo.pKeyboard->processInputProc)(&x, pegInfo.pKeyboard, 1);
		break;
	    }
	    break;

	case E_MMOTION:
	    debug5(("\n"));
	    x.u.u.type = MotionNotify;
#ifdef XTESTEXT1
        if (on_steal_input) {
	    XTestStealMotionData
		    (x.u.keyButtonPointer.rootX - LastMousePosition.x,
		    x.u.keyButtonPointer.rootY - LastMousePosition.y,
		    0,	/* dev_type: useful for multiple input devices */
		    x.u.keyButtonPointer.rootX,
		    x.u.keyButtonPointer.rootY);
	}
	LastMousePosition.x = x.u.keyButtonPointer.rootX;
	LastMousePosition.y = x.u.keyButtonPointer.rootY;
#endif  /* XTESTEXT1 */

#ifndef MOTION_BUFFER_OFF
	    gfbSaveMotionEvent(&x);
#else /* MOTION_BUFFER_OFF */
	    (*pegInfo.pPointer->processInputProc)(&x, pegInfo.pPointer, 1);
#endif /* MOTION_BUFFER_OFF */
	    break;
	}
big_break:

	if (i == qLimit)
	    i = queue->head = 0;
	else
	    i = ++queue->head;
    }

#ifndef MOTION_BUFFER_OFF
	/*
	 * At end of processing input buffer if mouseMoved, then send
	 * last motion event.
	 */
	if (pegInfo.motionQueue->mouseMoved == TRUE) {
	    gfbSendMotionEvent(pegInfo.pPointer);
    }
#endif /* MOTION_BUFFER_OFF */
}

#ifdef XPEG_TANDEM

static
pegReadEvents(queue)
    EventQueue *queue;
{
    int n, mask, index, cnt, red;
    Event   *ep;
    struct timeval  timeout;
    extern int  errno;
    extern int  pegDevFd;

    timeout.tv_usec = 100000;
    timeout.tv_sec = 0;

    errno = 0;
    for (cnt = 0; ; cnt++) {
	mask = 1<<pegDevFd;
	n = select(pegDevFd+1, &mask, NULL, NULL, &timeout);
	if (n == 0)
	    break;
	if (n < 0) {
	    fprintf(stderr, "pegReadEvents: select %d, errno =%d\n",
		n, errno);
	    exit(1);
	}
	ep = &queue->events[ queue->tail ];
	red = read(pegDevFd, ep, sizeof(Event));
	if (red < sizeof(Event)) {
	    fprintf(stderr, "pegReadEvents: red %d,errno=%d,sel=%d\n",
		red, errno, n);
	    exit(1);
	}
	/*
	 * fprintf(stderr, "server:(%d,%d),time=0x%x,type=%s,",
	 *  ep->e_x,
	 *  ep->e_y,
	 *  ep->e_time,
	 *  type[ ep->e_type & 0x7 ]);
	 * fprintf(stderr, "key=0x%x,dir=%s,dev=%s\n",
	 *  ep->e_key,
	 *  direction[ ep->e_direction & 0x3],
	 *  device[ ep->e_device & 0x7 ]);
	 */

	 index = queue->tail + 1;
	 if (index >= queue->size)
	    index = 0;
	 if (index != queue->head)
	    queue->tail = index;
    }
}

#endif /* XPEG_TANDEM */

void
InitKeybdState()
{
    KanaLocked = readyToUnlock = KanaShiftState = False;
    NumKanaLocks = NumKanaShifts = 0;
}
