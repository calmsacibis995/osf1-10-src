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
 * $XConsortium: mipsIo.c,v 1.5 91/07/18 22:58:24 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsIo.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 * mipsIo.c - input device routines.
 */
#ifdef SYSV
#include <sys/param.h>
#else /* SYSV */
#define HZ	100		/* 100 ticks/second of the clock */
#endif /* SYSV */
#include "X.h"
#define  NEED_EVENTS
#include "Xproto.h"
#include "scrnintstr.h"
#include "inputstr.h"
#include "mips.h"
#include "mipsIo.h"
#include "mipsMouse.h"

int		lastEventTime;
static int	ts_calibrate = 1;

extern void handleKeybd();
extern void handleMouse();

/* SIGIO handler */
#ifdef X11R4
volatile int mipsIOReady;

sigIOfunc()
{
    mipsIOReady = 1;
    isItTimeToYield++;
}
#else /* X11R4 */
sigIOfunc()
{
    extern DevicePtr pKeyboard, pPointer;

    handleKeybd(pKeyboard);
    handleMouse(pPointer);
}
#endif /* X11R4 */

void
ProcessInputEvents()
{
#ifdef X11R4
    DevicePtr	pMouse;
    DevicePtr	pKeybd;

    pMouse = LookupPointerDevice();
    pKeybd = LookupKeyboardDevice();
    mipsIOReady = 0;
    handleKeybd(pKeybd);
    handleMouse(pMouse);

    if (screenIsSaved == SCREEN_SAVER_ON)
	SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
#else /* X11R4 */
    mieqProcessInputEvents();
    miPointerUpdate();
#endif /* X11R4 */
}

TimeSinceLastInputEvent()
{
    int	elapsed;

    if (lastEventTime == 0) {
	lastEventTime = GetTimeInMillis();
	elapsed = 0;
	ts_calibrate = 1;	/* No events yet, then calibrate */
    }
    else if ((elapsed = GetTimeInMillis() - lastEventTime) < 0) {
	lastEventTime += elapsed;
	elapsed = 0;
	ts_calibrate = 1;	/* Event timestamp is in the future, then calibrate */
    }
    else if (elapsed > 60000) {
	ts_calibrate = 1;	/* Event timestamp is far in the past, then calibrate */
    }

    return(elapsed);
}

int
offsetTime(time)
int	time;
{
    static int	offset;
    int		mtime;
    int		rtime;

    mtime = time * (1000 / HZ);

    if (ts_calibrate) {
	ts_calibrate = 0;
	rtime = GetTimeInMillis();
	offset = rtime - mtime;
    }

    return(mtime + offset);
}
