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
 * $XConsortium: mipsMouse.c,v 1.5 91/07/18 22:58:46 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsMouse.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>
#include <sys/file.h>
#include <sysv/sys/termio.h>
#include <sysv/sys/uart_ioctl.h>
#include <sys/time.h>
#ifndef NOSIGNALS
#include <signal.h>
#endif NOSIGNALS

#include "X.h"
#define  NEED_EVENTS
#include "Xproto.h"
#include "scrnintstr.h"
#include "inputstr.h"
#include "mipointer.h"

#include "mips.h"
#include "mipsIo.h"
#include "mipsMouse.h"

MousePriv mousePriv = {
    -1,
    0,
    0,
    1200,
    0,
    0, 0,
    MIPS_MOUSE_DEFAULT,
    0,
    { {RAW_LEFT, 1},
      {RAW_MIDDLE, 2},
      {RAW_RIGHT, 3} },
};

#if PIXIE
extern int	pixie;
#endif /* PIXIE */

/* mouse statistics */
int	mserr = 0;

#ifdef X11R4

static long mipsEventTime();
static Bool mipsCursorOffScreen();
static void mipsCrossScreen();
extern void miPointerQueueEvent();

miPointerCursorFuncRec mipsPointerCursorFuncs = {
    mipsEventTime,
    mipsCursorOffScreen,
    mipsCrossScreen,
    miPointerQueueEvent,
};

volatile mouseQ_t	mouseQ[MOUSEQSIZE];
volatile mouseQ_t	*mouseQh = mouseQ;
volatile mouseQ_t	*mouseQt = mouseQ;

/* Motion buffer */
int		motionBufferSize = 100;
xTimecoord	*motionBuf = NULL;
xTimecoord	*motionTail = NULL;

static long
mipsEventTime(pScr)
ScreenPtr	pScr;
{
    return(lastEventTime);
}

#else /* X11R4 */

static Bool mipsCursorOffScreen();
static void mipsCrossScreen();
static void mipsWarpCursor();

miPointerScreenFuncRec mipsPointerScreenFuncs = {
    mipsCursorOffScreen,
    mipsCrossScreen,
    mipsWarpCursor,
};

static void
mipsWarpCursor (pScr, x, y)
ScreenPtr	pScr;
int		x, y;
{
#ifdef SYSV
    void	(*poll)();

    poll = sigset(SIGPOLL, SIG_HOLD);
#else /* SYSV */
    int		block;

    block = sigblock(sigmask(SIGIO));
#endif /* SYSV */
    miPointerWarpCursor (pScr, x, y);
#ifdef SYSV
    (void) sigset(SIGPOLL, poll);
#else /* SYSV */
    (void) sigsetmask(block);
#endif /* SYSV */
}

#endif /* X11R4 */

static Bool
mipsCursorOffScreen(pScr, x, y)
ScreenPtr	*pScr;
int		*x;
int		*y;
{
    int		i;

    if ((screenInfo.numScreens > 1) && ((*x < 0) || ((*pScr)->width <= *x))) {
	i = (*pScr)->myNum;
	if (*x < 0) {
	    if (i == 0) i = screenInfo.numScreens;
	    i--;
	    *pScr = screenInfo.screens[i];
	    *x += (*pScr)->width;
	}
	else {
	    *x -= (*pScr)->width;
	    i++;
	    if (i == screenInfo.numScreens) i = 0;
	    *pScr = screenInfo.screens[i];
	}
	return(TRUE);
    }
    return(FALSE);
}

/*ARGSUSED*/
static void
mipsCrossScreen(pScr, enter)
ScreenPtr	pScr;
Bool		enter;
{
}

static
setSpeed(fd, vmin, speed)
int	fd;
int	vmin;
int	speed;
{
    struct termio	logmode;

    logmode.c_iflag = IGNBRK|IGNPAR;
    logmode.c_oflag = 0;
    logmode.c_lflag = 0;
    logmode.c_line = 0;
    logmode.c_cc[VMIN] = vmin;
    logmode.c_cc[VTIME] = 0;
    switch (speed) {
	case 9600:
	    logmode.c_cflag = B9600|CS8|CSTOPB|CREAD|CLOCAL|HUPCL;
	    break;
	case 4800:
	    logmode.c_cflag = B4800|CS8|CSTOPB|CREAD|CLOCAL|HUPCL;
	    break;
	case 2400:
	    logmode.c_cflag = B2400|CS8|CSTOPB|CREAD|CLOCAL|HUPCL;
	    break;
	case 1200:
	default:
	    logmode.c_cflag = B1200|CS8|CSTOPB|CREAD|CLOCAL|HUPCL;
	    break;
    }
    if (ioctl(fd, TCSETAF, &logmode) < 0) {	/* set tty mode */
	Error("cannot ioctl(TCSETAF) mouse");
	return(-1);
    }
}

static
setBaud(fd, vmin, current, new)
int	fd;
int	vmin;
int	current;
int	new;
{
    setSpeed(fd, vmin, current);

#if PIXIE
    if (!pixie)
#endif /* PIXIE */
    {
	write(fd, "*", 1);
	switch (new) {
	    case 9600:
		write(fd, "q", 1);  /* Set baud rate to 9600 */
		break;
	    case 4800:
		write(fd, "p", 1);  /* Set baud rate to 4800 */
		break;
	    case 2400:
		write(fd, "o", 1);  /* Set baud rate to 2400 */
		break;
	    case 1200:
	    default:
		write(fd, "n", 1);  /* Set baud rate to 1200 */
		break;
	}

	{
	    struct timeval	wait;

	    wait.tv_sec = 0;
	    wait.tv_usec = 100000;

	    select(0, NULL, NULL, NULL, &wait);
	}
    }

    setSpeed(fd, vmin, new);
}

/* logfd is a real serial device... */

static int
loginit(pPriv)
MousePrivPtr	pPriv;
{
    int		vmin;
    char	c;

    if (pPriv->cap & DEV_INIT) {
	vmin = (pPriv->cap & DEV_TIMESTAMP) ? 40 : 5;
	vmin = 1;
	setBaud(pPriv->fd, vmin, 1200, pPriv->baud);
	setBaud(pPriv->fd, vmin, 2400, pPriv->baud);
	setBaud(pPriv->fd, vmin, 4800, pPriv->baud);
	setBaud(pPriv->fd, vmin, 9600, pPriv->baud);

	switch (pPriv->type) {
	default:
		write(pPriv->fd, "U", 1);	/* Five byte packed binary */

		if (pPriv->rate <= 0)
		    write(pPriv->fd, "O", 1);	/* Continuous */
		else if (125 <= pPriv->rate)
		    write(pPriv->fd, "N", 1);	/* 150 per second */
		else if (85 <= pPriv->rate)
		    write(pPriv->fd, "Q", 1);	/* 100 per second */
		else if (60 <= pPriv->rate)
		    write(pPriv->fd, "M", 1);	/* 70 per second */
		else if (42 <= pPriv->rate)
		    write(pPriv->fd, "R", 1);	/* 50 per second */
		else if (27 <= pPriv->rate)
		    write(pPriv->fd, "L", 1);	/* 35 per second */
		else if (15 <= pPriv->rate)
		    write(pPriv->fd, "K", 1);	/* 20 per second */
		else
		    write(pPriv->fd, "J", 1);	/* 10 per second */
		break;
	case MIPS_MOUSE_MOUSEMAN:
		sleep(2);
		write(pPriv->fd, "*U", 2);
		sleep(2);
		while ( read(pPriv->fd, &c, 1) == 1 )
			/* nothing */ ;
		break;
	}
    }
    return(0);
}

static int
accelMouse(d)
int		d;
{
    PtrCtrl *pctrl;

    pctrl = &mousePriv.ctrl;
    if (d > 0)
	return((d > pctrl->threshold) ? (d * pctrl->num) / pctrl->den : d);
    else
	return((-d > pctrl->threshold) ? (d * pctrl->num) / pctrl->den : d);
}

#ifdef X11R4
static void
enqueueMouse(time, dx, dy, bmask)
int	time;
int	dx, dy;
int	bmask;
{
    volatile mouseQ_t	*newQt;

    mouseQt->time = time;
    mouseQt->scrn = mousePriv.scrn;
    mouseQt->dx = accelMouse(dx);
    mouseQt->dy = accelMouse(dy);
    mouseQt->bmask = bmask;
    newQt = mouseQt + 1;
    if (newQt == (mouseQ + MOUSEQSIZE))
	newQt = mouseQ;
    if (newQt != mouseQh)
	mouseQt = newQt;
}

static void
dequeueMouse()
{
    volatile mouseQ_t	*newQh;

    newQh = mouseQh + 1;
    if (newQh == (mouseQ + MOUSEQSIZE))
	newQh = mouseQ;
    mouseQh = newQh;
}
#endif /* X11R4 */

#ifdef X11R4
#define	mouseEvent(pm, dx, dy, bmask, time) \
	enqueueMouse((time), (dx), (dy), (bmask))
#endif /* X11R4 */

static int
charMouse(pMouse, code, time)
    DevicePtr pMouse;
    char code;
    int time;
{
    static signed char	msbuf[5];	/* mouse input buffer */
    static int		state = 0;

    if ((code & 0xf8) == LSYNCBIT) {
	/* Sync bit found */

	if (state) {
	    mouseEvent (pMouse, msbuf[1], -msbuf[2], msbuf[0], time);
	    mserr++;
	}
	state = 1;
	msbuf[0] = BUTTONMASK(code);
	msbuf[1] = 0;
	msbuf[2] = 0;
    }
    else {
	/* Sync bit not found */

	switch (state) {
	    case 0:	/* Waiting for sync byte */
		mserr++;
		break;
	    case 1:	/* Waiting for X0 */
		msbuf[state++] = (signed char) code;
		break;
	    case 2:	/* Waiting for Y0 */
		msbuf[state++] = (signed char) code;
		mouseEvent (pMouse, msbuf[1], -msbuf[2], msbuf[0], time);
		break;
	    case 3:	/* Waiting for X1 */
		msbuf[state++] = (signed char) code;
		break;
	    case 4:	/* Waiting for Y1 */
		msbuf[state] = (signed char) code;
		mouseEvent (pMouse, msbuf[1], -msbuf[2], msbuf[0], time);
		state = 0;
		break;
	}
    }
}

static
timestampMouse(pMouse, code)
DevicePtr	pMouse;
char		code;
{
    static int		state = -1;
    static char		data;
    static time_t	time;
    u_char		*ptime = (u_char *) &time;

    state++;
    switch (state) {
	case 0:	/* Looking for 1st sync byte */
	case 1:	/* Looking for 2nd sync byte */
	case 2:	/* Looking for 3rd sync byte */
	    if (((u_char) code) != UARTSYNCCHAR)
		state = -1;
	    break;
	case 3:	/* Looking for data */
	    data = code;
	    break;
	case 4:	/* Looking for 1st time byte */
	case 5:	/* Looking for 2nd time byte */
	case 6:	/* Looking for 3rd time byte */
	    ptime[state - 4] = (u_char) code;
	    break;
	case 7:	/* Looking for 4th time byte */
	    ptime[state - 4] = (u_char) code;
	    charMouse(pMouse, data, offsetTime(time));
	    state = -1;
	    break;
    }
}

void
handleMouse(pMouse)
DevicePtr	pMouse;
{
    int		nchar = 0;
    int		i;
    char	buf[MAXEVENTS];

#ifdef X11R4
#ifdef SYSV
    void	(*poll)();

    poll = sigset(SIGPOLL, SIG_HOLD);
#else /* SYSV */
    int		block;

    block = sigblock(sigmask(SIGIO));
#endif /* SYSV */
#else /* X11R4 */
    if (!mousePriv.cap & DEV_TIMESTAMP)
	lastEventTime = GetTimeInMillis();
#endif /* X11R4 */

    if ((mousePriv.fd >= 0) && (mousePriv.cap & DEV_READ)) {
	do {
	    if ((nchar = read(mousePriv.fd, buf, sizeof(buf))) <= 0)
		break;

	    if (mousePriv.cap & DEV_TIMESTAMP) {
		for (i = 0; i < nchar; ++i)
		    timestampMouse(pMouse, buf[i]);
	    }
	    else {
		for (i = 0; i < nchar; ++i)
		    charMouse(pMouse, buf[i], lastEventTime);
	    }
	} while (nchar == sizeof(buf));
    }

#ifdef X11R4
#ifdef SYSV
    (void) sigset(SIGPOLL, poll);
#else /* SYSV */
    (void) sigsetmask(block);
#endif /* SYSV */

    while (mouseQh != mouseQt) {
	mouse_event(pMouse, mouseQh);
	dequeueMouse();
    }
#endif /* X11R4 */
}

/* ARGSUSED */
static void
mipsChangePointerControl(pDevice, ctrl)
DevicePtr	pDevice;
PtrCtrl		*ctrl;
{
    mousePriv.ctrl = *ctrl;
}

#ifdef X11R4

/* ARGSUSED */
static int
mipsGetMotionEvents(dev, buff, start, stop)
DeviceIntPtr	dev;
xTimecoord	*buff;
CARD32		start, stop;
{
    xTimecoord	*ptc;
    int		count = 0;

    if (motionBuf) {
	ptc = motionTail;
	do {
	    ptc++;
	    if (ptc == (motionBuf + motionBufferSize))
		ptc = motionBuf;
	    if ((start <= ptc->time) && (ptc->time <= stop)) {
		*buff++ = *ptc;
		count++;
	    }
	}
	while ((ptc != motionTail) && (ptc->time <= stop));
    }

    return(count);
}

#endif /* X11R4 */

static mouseAsync(pPriv, set)
MousePrivPtr	pPriv;
Bool	set;
{
#if PIXIE
    if (pixie)
	return;
#endif /* PIXIE */
    if (pPriv->cap & DEV_ASYNC) {
	if (mipsStreamAsync(pPriv->fd, set) < 0) {
	    pPriv->cap &= ~DEV_ASYNC;
	    Error("cannot ioctl(I_SETSIG) mouse");
	}
    }
}

openMouse()
{
    if (mousePriv.fd < 0) {
	if ((mousePriv.fd = open(MOUSEDEV, O_RDWR|O_NDELAY)) >= 0) {
#ifndef SYSV
	    int		flags;

	    if ((flags = fcntl(mousePriv.fd, F_GETFL, flags)) == -1)
		Error("cannot fcntl(F_GETFL) mouse");
	    flags |= FNDELAY;
	    if (fcntl(mousePriv.fd, F_SETFL, flags) == -1)
		Error("cannot fcntl(F_SETFL) mouse");
#endif /* SYSV */
	    mousePriv.cap = -1;
	}
	else {
	    mousePriv.cap = 0;
	    Error("cannot open mouse");
	}
    }
}


/* ARGSUSED */
int
mipsMouseProc(pMouse, onoff, argc, argv)
DevicePtr	pMouse;
int		onoff, argc;
char		*argv[];
{
    BYTE		map[4];
    int			flags;
    MousePrivPtr	pPriv;
    extern int		loginit();

    pPriv = (MousePrivPtr) pMouse->devicePrivate;
    switch (onoff) {
	case DEVICE_INIT:
	    pMouse->devicePrivate = (pointer) &mousePriv;
	    pPriv = (MousePrivPtr) pMouse->devicePrivate;
	    openMouse();
	    pMouse->on = FALSE;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
#ifdef X11R4
	    if (motionBufferSize) {
		motionBuf = motionTail =
		    (xTimecoord *) Xalloc(motionBufferSize * sizeof(xTimecoord));
		if (motionBuf)
		    bzero(motionBuf, motionBufferSize * sizeof(xTimecoord));
	    }
	    InitPointerDeviceStruct(pMouse, map, 3, mipsGetMotionEvents,
		mipsChangePointerControl, motionBufferSize);
#else /* X11R4 */
	    InitPointerDeviceStruct(pMouse, map, 3, miPointerGetMotionEvents,
		mipsChangePointerControl, miPointerGetMotionBufferSize());
#endif /* X11R4 */
	    break;
	case DEVICE_ON:
	    if (pPriv->fd >= 0) {
		if (pPriv->cap & DEV_TIMESTAMP) {
		    if (ioctl(pPriv->fd, UTCSETTIMESTAMP, 0) < 0) {
			pPriv->cap &= ~DEV_TIMESTAMP;
			Error("cannot ioctl(UTCSETTIMESTAMP) mouse");
		    }
		}
		if (loginit(pPriv) < 0)
		    ErrorF("cannot initialize mouse\n");
		mouseAsync(pPriv, TRUE);
		AddEnabledDevice(pPriv->fd);
	    }
	    pMouse->on = TRUE;
	    break;
	case DEVICE_CLOSE:
	    pMouse->on = FALSE;
	    if (pPriv->fd >= 0) {
		RemoveEnabledDevice(pPriv->fd);
		(void) close(pPriv->fd);
	    }
	    pPriv->fd = -1;
	    pPriv->cap = 0;
#ifdef X11R4
	    if (motionBuf)
		Xfree(motionBuf);
	    motionBuf = motionTail = NULL;
#endif /* X11R4 */
	    break;
	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    if (pPriv->fd >= 0)
		RemoveEnabledDevice(pPriv->fd);
	    break;
    }
    return(Success);
}

#ifdef X11R4

static
mouse_event(pMouse, pQ)
DevicePtr	pMouse;
mouseQ_t	*pQ;
{
    xEvent		mevent;
    int			i;
    int			nbuttons;
    MousePrivPtr	pPriv;

    lastEventTime = pQ->time;
    pPriv = (MousePrivPtr) pMouse->devicePrivate;

    /* a motion event? do cursor chase */

    if (pQ->dx || pQ->dy) {
	miPointerPosition(screenInfo.screens[0], &pPriv->rootX, &pPriv->rootY);
	pPriv->rootX += pQ->dx;
	pPriv->rootY += pQ->dy;
	miPointerDeltaCursor(screenInfo.screens[0], pQ->dx, pQ->dy, TRUE);

	if (motionBuf) {
	    motionTail++;
	    if (motionTail == (motionBuf + motionBufferSize))
		motionTail = motionBuf;
	    motionTail->time = lastEventTime;
	    motionTail->x = pPriv->rootX;
	    motionTail->y = pPriv->rootY;
	}
    }

    /* button change state? notify server. */

    if (nbuttons = pQ->bmask ^ pPriv->buttonstate) {
	mevent.u.keyButtonPointer.time = lastEventTime;
	for(i=0;i<NBUTTONS;i++)
	    if (pPriv->buttonmap[i].mask&nbuttons) {
		mevent.u.u.type =
		    (pQ->bmask & pPriv->buttonmap[i].mask) ?
		    ButtonPress : ButtonRelease;
		mevent.u.u.detail = pPriv->buttonmap[i].val;
		(*pMouse->processInputProc)(&mevent, pMouse, 1);
	    }
    }

    pPriv->buttonstate = pQ->bmask;
}

#else /* X11R4 */

static int
mouseAccel(pMouse, d)
DevicePtr	pMouse;
int		d;
{
    PtrCtrl *pctrl;

    pctrl = &((DeviceIntPtr) pMouse)->ptrfeed->ctrl;
    if (d > 0)
	return((d > pctrl->threshold) ? (d * pctrl->num) / pctrl->den : d);
    else
	return((-d > pctrl->threshold) ? (d * pctrl->num) / pctrl->den : d);
}

static
mouseEvent(pMouse, dx, dy, bmask, time)
DevicePtr	pMouse;
int		dx, dy, bmask;
{
    xEvent		mevent;
    int			i;
    int			nbuttons;
    MousePrivPtr	pPriv;

    lastEventTime = time;
    pPriv = (MousePrivPtr) pMouse->devicePrivate;

    /* a motion event? do cursor chase */

    if (dx || dy)
    {
	dx = mouseAccel (pMouse, dx);
	dy = mouseAccel (pMouse, dy);
	miPointerDeltaCursor (dx, dy, time);
    }

    /* button change state? notify server. */

    if (nbuttons = bmask ^ pPriv->buttonstate) {
	mevent.u.keyButtonPointer.time = lastEventTime;
	for(i=0;i<NBUTTONS;i++)
	    if (pPriv->buttonmap[i].mask&nbuttons) {
		mevent.u.u.type =
		    (bmask & pPriv->buttonmap[i].mask) ?
		    ButtonPress : ButtonRelease;
		mevent.u.u.detail = pPriv->buttonmap[i].val;
		mieqEnqueue (&mevent);
	    }
    }

    pPriv->buttonstate = bmask;
}

#endif /* X11R4 */
