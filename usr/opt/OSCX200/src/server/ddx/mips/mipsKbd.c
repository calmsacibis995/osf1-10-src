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
 * $XConsortium: mipsKbd.c,v 1.6 91/07/18 22:58:29 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsKbd.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sysv/sys/termio.h>
#include <sysv/sys/kbd_ioctl.h>
#include <sysv/sys/buzzer.h>

#include "X.h"
#include "keysym.h"
#define  NEED_EVENTS
#include "Xproto.h"
#include "scrnintstr.h"
#include "inputstr.h"

#include "mips.h"
#include "mipsIo.h"
#include "mipsKbd.h"
#include "mipsMouse.h"

#if PIXIE
extern int	pixie;
#endif /* PIXIE */
extern int	DDXxled1;
extern int	DDXxled2;
extern int	DDXxled3;
static int	keybdLock = 0;	/* Current Lock key state */
static int	keyClick = 0;	/* KeyClick batching flag */

KeybdPriv keybdPriv = {
    -1,
    0,
    0,
    DEFAULT_KEYBOARD,
};

KeybdType_t keybdType[] = {
#ifdef XT_KEYBOARD
    { { xt_KeyMap, 0x01 + 8, 0x65 + 8, 2, },
	xt_ModMap, 8, xtKeybdEvent },
#endif /* XT_KEYBOARD */

#ifdef AT_KEYBOARD
    { { at_KeyMap, 0x01 + 8, 0x84 + 8, 2, },
	at_ModMap, 8, atKeybdEvent },
#endif /* AT_KEYBOARD */

#ifdef UNIX1_KEYBOARD
    { { unix1_KeyMap, 0x07 + 2, 0x84 + 2, 2, },
	unix1_ModMap, 2, unix1KeybdEvent },
#endif /* UNIX1_KEYBOARD */
};

int
getKeyCode(ks)
KeySym	ks;
{
    KeySymsPtr		pKS;
    int			i, size, kc;

    pKS = &keybdType[keybdPriv.type].keySyms;
    size = (pKS->maxKeyCode - pKS->minKeyCode) * pKS->mapWidth;

    for (i = 0; i < size; i++) {
	if (pKS->map[i] == ks) {
	    kc = (i / pKS->mapWidth) + pKS->minKeyCode;
	    return(kc);
	}
    }
    return(-1);
}

static
getKeyswtch()
{
    extern int		errno;
    int addr, keyswtch = 0;

    errno = 0;
    addr = koptKeyswtch();
    if (errno != 0) {
	Error("unable to get keyswtch");
	return(0);
    }

    if (addr & ~1) {
	int		memfd;
	u_char		value;
	char		*coref = "/dev/kmem";
	extern off_t lseek();

	if ((memfd = open(coref, O_RDONLY)) < 0) {
	    ErrorF("unable to open ");
	    Error(coref);
	}
	else {
	    addr &= 0x7fffffff;
	    if (lseek(memfd, (off_t) addr, L_SET) == (off_t) -1) {
		ErrorF("unable to seek to %#xx in ", addr);
		Error(coref);
	    }
	    else if (read(memfd, &value, (int)sizeof(value)) != (int)sizeof(value)) {
		ErrorF("unable to read keyswtch from ");
		Error(coref);
	    }
	    else
		keyswtch = (value == '1') ? 1 : 0;
	    (void) close(memfd);
	}
    }

    return(keyswtch);
}

initKeybd()
{
    static int	swap = 1;

    if (keybdPriv.type == DEFAULT_KEYBOARD) {
	switch (mipsSysType) {
	    case RS2030:
#ifdef UNIX1_KEYBOARD
		keybdPriv.type = UNIX1_KEYBOARD; /* UNIX1 if nothing else is available */
#endif /* UNIX1_KEYBOARD */
#ifdef AT_KEYBOARD
		keybdPriv.type = AT_KEYBOARD;	/* AT if XT is not available */
#endif AT_KEYBOARD
#ifdef XT_KEYBOARD
		keybdPriv.type = XT_KEYBOARD;	/* XT if available */
#endif /* XT_KEYBOARD */
		break;

	    case RS3230:
#ifdef XT_KEYBOARD
		keybdPriv.type = XT_KEYBOARD;	/* XT if nothing else is available */
#endif /* XT_KEYBOARD */
#ifdef AT_KEYBOARD
		keybdPriv.type = AT_KEYBOARD;	/* AT if UNIX1 is not available */
#endif AT_KEYBOARD
#ifdef UNIX1_KEYBOARD
		keybdPriv.type = UNIX1_KEYBOARD; /* UNIX1 if available */
#endif /* UNIX1_KEYBOARD */
		break;
	}
    }

    if ((swap) && (getKeyswtch())) {
	CARD8		tmpMod;
	KeySym		tmpSym;
	int		kc1, kc2;
	KeySymsPtr	pKS;
	CARD8		*modMap;
	int		i;

	/* Swap XK_Caps_Lock and XK_Control_L */

	swap = 0;
	pKS = &keybdType[keybdPriv.type].keySyms;
	modMap = keybdType[keybdPriv.type].modMap;

	if ((kc1 = getKeyCode(XK_Caps_Lock)) >= 0) {
	    if ((kc2 = getKeyCode(XK_Control_L)) >= 0) {
		tmpMod = modMap[kc1];
		modMap[kc1] = modMap[kc2];
		modMap[kc2] = tmpMod;

		kc1 = (kc1 - pKS->minKeyCode) * pKS->mapWidth;
		kc2 = (kc2 - pKS->minKeyCode) * pKS->mapWidth;
		for (i = 0; i < pKS->mapWidth; i++) {
		    tmpSym = pKS->map[kc1];
		    pKS->map[kc1] = pKS->map[kc2];
		    pKS->map[kc2] = tmpSym;
		    kc1++;
		    kc2++;
		}
	    }
	}
    }
}

setLEDs(pPriv, cmd, on)
KeybdPrivPtr	pPriv;
int		cmd;
int		on;
{
    static int	keybdLights = 0;	/* Current LED value */

    if (pPriv->cap & DEV_LIGHTS) {
	switch (cmd) {
	    case LEDupdate:
		break;
	    case LEDreset:
		keybdLights = 0;
		break;
	    case LEDScrollLock:
		if (!DDXxled1) {
		    if (on)
			keybdLights |= KLSCRLLOCK;
		    else
			keybdLights &= ~KLSCRLLOCK;
		}
		break;
	    case LEDCapsLock:
		if (!DDXxled2) {
		    if (on)
			keybdLights |= KLCAPSLOCK;
		    else
			keybdLights &= ~KLCAPSLOCK;
		}
		break;
	    case LEDNumLock:
		if (!DDXxled3) {
		    if (on)
			keybdLights |= KLNUMLOCK;
		    else
			keybdLights &= ~KLNUMLOCK;
		}
		break;
	    case LED1:
		if (DDXxled1) {
		    if (on)
			keybdLights |= KLSCRLLOCK;
		    else
			keybdLights &= ~KLSCRLLOCK;
		}
		break;
	    case LED2:
		if (DDXxled2) {
		    if (on)
			keybdLights |= KLCAPSLOCK;
		    else
			keybdLights &= ~KLCAPSLOCK;
		}
		break;
	    case LED3:
		if (DDXxled3) {
		    if (on)
			keybdLights |= KLNUMLOCK;
		    else
			keybdLights &= ~KLNUMLOCK;
		}
		break;
	    default:
		return;
	}
	if ((ioctl(pPriv->fd, KTCSETLIGHTS, &keybdLights) < 0) && (errno != EBUSY))
	    pPriv->cap &= ~DEV_LIGHTS;
    }
}

/* ARGSUSED */
static void
mipsKeyClick(pKeybd)
DevicePtr	pKeybd;
{
    KeybdPrivPtr	pPriv;
    struct buzzer	buzz;
    int			i;

    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
    if (pPriv->cap & DEV_BUZZER) {
	if ((keyClick) && (((DeviceIntPtr) pKeybd)->kbdfeed->ctrl.click)) {
	    buzz.buzzer_time = 1;
	    buzz.buzzer_load = HZ_TO_LOAD(600);
	    buzz.buzzer_flags = 0;
	    if ((ioctl(pPriv->fd, KTCPRGMBUZZER, &buzz) < 0) && (errno != EBUSY)) {
		pPriv->cap &= ~DEV_BUZZER;
		Error("cannot ioctl(KTCPRGMBUZZER) keyboard");
	    }
	}
    }
    keyClick = 0;
}

void
handleKeybd(pKeybd)
DevicePtr	pKeybd;
{
    int			nchar = 0;
    int			i;
    u_char		buf[MAXEVENTS];
    KeybdPrivPtr	pPriv;

    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
    if (pPriv->cap & DEV_READ) {
	do {
	    if ((nchar = read(pPriv->fd, buf, sizeof(buf))) <= 0) {
		if ((nchar < 0) && (errno != EWOULDBLOCK)) {
		    pPriv->cap &= ~DEV_READ;
		    Error("error reading keyboard");
		}
		return;
	    }

	    if (pPriv->cap & DEV_TIMESTAMP) {
		for (i = 0; i < nchar; ++i)
		    timestampKeybd(pKeybd, buf[i]);
	    }
	    else {
		for (i = 0; i < nchar; ++i)
		    keybdType[pPriv->type].keybdEvent(pKeybd, buf[i]);
	    }
	    mipsKeyClick(pKeybd);
	} while (nchar == sizeof(buf));
    }
}

/*ARGSUSED*/
Bool
LegalModifier(key, pDev)
    BYTE key;
    DevicePtr pDev;
{
    return(TRUE);
}

static int
stinit(pPriv)
KeybdPrivPtr	pPriv;
{
    if (pPriv->cap & DEV_INIT) {
	struct termio   stmode;

	stmode.c_iflag = IGNBRK|IGNPAR;
	stmode.c_oflag = 0;
	stmode.c_cflag = B1200|CS8|CREAD|CLOCAL;
	stmode.c_lflag = 0;
	stmode.c_cc[VMIN] = 1;
	stmode.c_cc[VTIME] = 0;

	if (ioctl(pPriv->fd, TCSETAF, &stmode) < 0) {
	    pPriv->cap &= ~DEV_INIT;
	    Error("cannot ioctl(TCSETAF) keyboard");
	    return(-1);
	}
    }
    return(0);
}

/* ARGSUSED */
static void
mipsBell(loud, pKeybd)
int		loud;
DevicePtr	pKeybd;
{
    KeybdPrivPtr	pPriv;
    struct buzzer	buzz;
    KeybdCtrl		*pCtrl;

    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
    if (pPriv->cap & DEV_BUZZER) {
	pCtrl = &((DeviceIntPtr) pKeybd)->kbdfeed->ctrl;
	if (pCtrl->bell) {
	    buzz.buzzer_time = pCtrl->bell_duration / 10;
	    buzz.buzzer_load = (pCtrl->bell_pitch ? 
			       HZ_TO_LOAD(pCtrl->bell_pitch) : HZ_TO_LOAD(1));
	    buzz.buzzer_flags = 0;
	    if ((ioctl(pPriv->fd, KTCPRGMBUZZER, &buzz) < 0) && (errno != EBUSY)) {
		pPriv->cap &= ~DEV_BUZZER;
		Error("cannot ioctl(KTCPRGMBUZZER) keyboard");
	    }
	}
    }
}

/* ARGSUSED */
static void
mipsChangeKeybdCtrl(pKeybd, ctrl)
DevicePtr	pKeybd;
KeybdCtrl	*ctrl;
{
    KeybdPrivPtr	pPriv;

    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
    setLEDs(pPriv, LED1, (int) ctrl->leds & 1);
    setLEDs(pPriv, LED2, (int) ctrl->leds & 2);
    setLEDs(pPriv, LED3, (int) ctrl->leds & 4);
}

openKeybd()
{
    if (keybdPriv.fd < 0) {
	if ((keybdPriv.fd = open(KEYDEV, O_RDWR|O_NDELAY)) >= 0) {
#ifndef SYSV
	    int		flags;

	    if ((flags = fcntl(keybdPriv.fd, F_GETFL, flags)) == -1)
		Error("cannot fcntl(F_GETFL) keyboard");
	    flags |= FNDELAY;
	    if (fcntl(keybdPriv.fd, F_SETFL, flags) == -1)
		Error("cannot fcntl(F_SETFL) keyboard");
#endif /* SYSV */
	    keybdPriv.cap = -1;
	}
	else {
	    keybdPriv.cap = 0;
	    Error("cannot open keyboard");
#if NETWORK_KEYBD
	    keybdPriv.fd = TCPkeybd();
#endif /* NETWORK_KEYBD */
	}
#if MESSED_UP_IOP
	if (mipsSysType == RS2030)
	    keybdPriv.cap &= ~DEV_LIGHTS;	/* IOP is messed up */
#endif
    }
}

static keybdAsync(pPriv, set)
KeybdPrivPtr	pPriv;
Bool	set;
{
#if PIXIE
    if (pixie)
	return;
#endif /* PIXIE */
    if (pPriv->cap & DEV_ASYNC) {
	if (mipsStreamAsync(pPriv->fd, set) < 0) {
	    pPriv->cap &= ~DEV_ASYNC;
	    Error("cannot ioctl(I_SETSIG) keyboard");
	}
    }
}

int
mipsKeybdProc(pKeybd, onoff, argc, argv)
DevicePtr	pKeybd;
int		onoff, argc;
char		*argv[];
{
    extern int		stinit();
    KeybdPrivPtr	pPriv;

    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
    switch (onoff) {
	case DEVICE_INIT:
	    pKeybd->on = FALSE;
	    pKeybd->devicePrivate = (pointer) &keybdPriv;
	    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
	    openKeybd();
	    InitKeyboardDeviceStruct(pKeybd, &(keybdType[pPriv->type].keySyms),
		keybdType[pPriv->type].modMap, mipsBell, mipsChangeKeybdCtrl);
	    break;
	case DEVICE_ON:
	    if (pPriv->fd >= 0) {
		if (pPriv->cap & DEV_TIMESTAMP) {
		    if (ioctl(pPriv->fd, KTCSETTIMESTAMP, 0) < 0) {
			pPriv->cap &= ~DEV_TIMESTAMP;
			Error("cannot ioctl(KTCSETTIMESTAMP) keyboard");
		    }
		}
		if (stinit(pPriv) < 0)
		    ErrorF("cannot initialize keyboard\n");
		keybdAsync(pPriv, TRUE);
		setLEDs(pPriv, LEDreset, 0);
		AddEnabledDevice(pPriv->fd);
	    }
	    keybdLock = 0;
	    pKeybd->on = TRUE;
	    break;
	case DEVICE_CLOSE:
	case DEVICE_OFF:
	    pKeybd->on = FALSE;
	    if (pPriv->fd >= 0)
		RemoveEnabledDevice(pPriv->fd);
	    break;
    }
    return(Success);
}

static
timestampKeybd(pKeybd, code)
DevicePtr	pKeybd;
u_char		code;
{
    static int		state = -1;
    static u_char	data;
    static time_t	time;
    u_char		*ptime = (u_char *) &time;
    KeybdPrivPtr	pPriv;

    state++;
    switch (state) {
	case 0:	/* Looking for 1st sync byte */
	case 1:	/* Looking for 2nd sync byte */
	case 2:	/* Looking for 3rd sync byte */
	    if (code != KBDSYNCCHAR)
		state = -1;
	    break;
	case 3:	/* Looking for data */
	    data = code;
	    break;
	case 4:	/* Looking for 1st time byte */
	case 5:	/* Looking for 2nd time byte */
	case 6:	/* Looking for 3rd time byte */
	    ptime[state - 4] = code;
	    break;
	case 7:	/* Looking for 4th time byte */
	    ptime[state - 4] = code;
	    lastEventTime = offsetTime((int) time);
	    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
	    keybdType[pPriv->type].keybdEvent(pKeybd, data);
	    state = -1;
	    break;
    }
}

static KeySym
keyCodeToKeySym(curKeySyms, keyCode)
KeySymsPtr	curKeySyms;
KeyCode		keyCode;
{
    if ((keyCode < curKeySyms->minKeyCode) ||
	(keyCode > curKeySyms->maxKeyCode))
	return(NoSymbol);

    return(curKeySyms->map[(keyCode - curKeySyms->minKeyCode) *
	curKeySyms->mapWidth]);
}

#ifdef X11R4
#define	mieqEnqueue(event) ((*pKeybd->processInputProc)((event), pKeybd, 1))
#endif /* X11R4 */

void
genKeybdEvent(pKeybd, release, kindex)
DevicePtr	pKeybd;
int		release;
u_char		kindex;
{
    static u_char	kindexRepeat = 0x100;
    xEvent		kevent;
    KeySym		ks;
    MousePrivPtr	pMPriv;
    KeybdCtrl		*pCtrl;
    KeybdPrivPtr	pPriv;
    u_char		mod;

    pPriv = (KeybdPrivPtr) pKeybd->devicePrivate;
    pCtrl = &((DeviceIntPtr) pKeybd)->kbdfeed->ctrl;

    kevent.u.keyButtonPointer.time = lastEventTime;

    ks = keyCodeToKeySym(&((DeviceIntPtr) pKeybd)->key->curKeySyms, kindex);
    mod = ((DeviceIntPtr) pKeybd)->key->modifierMap[kindex];
    if (release) {
	kindexRepeat = 0x100;
	if (mod & LockMask)
	    return;
	switch (ks) {
	    case XK_Num_Lock:
	    case XK_Pause:
		/* No KeyRelease for these keys */
		return;
	}
	kevent.u.u.type = KeyRelease;
	kevent.u.u.detail = kindex;
	mieqEnqueue(&kevent);
    }
    else {	/* operate */
	if (kindexRepeat == kindex) {
	    /* autorepeat case */

	    if (pCtrl->autoRepeat != AutoRepeatModeOn)
		return;	/* No autorepeat */

	    if (mod)
		return;
	    switch (ks) {
		case XK_Num_Lock:
		case XK_Pause:
		    /* No autorepeat */
		    return;
		    break;
	    }
	    kevent.u.u.type = KeyRelease;
	    kevent.u.u.detail = kindex;
	    keyClick = 1;
	    mieqEnqueue(&kevent);
	    kevent.u.u.type = KeyPress;
	    mieqEnqueue(&kevent);
	    kevent.u.u.type = KeyRelease;
	    mieqEnqueue(&kevent);
	    kevent.u.u.type = KeyPress;
	    mieqEnqueue(&kevent);
	}
	else {
	    /* Non-autorepeat case */

	    kevent.u.u.type = KeyPress;
	    if (mod & LockMask) {
		keybdLock ^= KLCAPSLOCK;
		if ((keybdLock & KLCAPSLOCK) == 0) {
		    kevent.u.u.type = KeyRelease;
		    setLEDs(pPriv, LEDCapsLock, 0);
		}
		else
		    setLEDs(pPriv, LEDCapsLock, 1);
	    }
	    else {
		switch (ks) {
		    case XK_Num_Lock:
			keybdLock ^= KLNUMLOCK;
			if ((keybdLock & KLNUMLOCK) == 0) {
			    kevent.u.u.type = KeyRelease;
			    setLEDs(pPriv, LEDNumLock, 0);
			}
			else
			    setLEDs(pPriv, LEDNumLock, 1);
			break;
		    case XK_Pause:
			keybdLock ^= KLSCRLLOCK;
			if ((keybdLock & KLSCRLLOCK) == 0) {
			    kevent.u.u.type = KeyRelease;
			    setLEDs(pPriv, LEDScrollLock, 0);
			}
			else
			    setLEDs(pPriv, LEDScrollLock, 1);
			break;
		}
	    }
	    kevent.u.u.detail = kindex;
	    keyClick = 1;
	    mieqEnqueue(&kevent);
	    kindexRepeat = kindex;
	}
    }
}

void
specialKeybdEvent()
{
#if MEM_DEBUG || MEM_STATS
    if ((keybdLock & KLNUMLOCK) == 0)
	listMem();
    else
	checkpointMem();
#endif
#if PIXIE
    if (pixie)
	exit(0);
#endif /* PIXIE */
}
