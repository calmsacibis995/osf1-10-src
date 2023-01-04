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
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/x386Procs.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */

#ifndef _X386PROCS_H
#define _X386PROCS_H

#include <X11/Xfuncproto.h>
#include "x386.h"
#include "x386Priv.h"

_XFUNCPROTOBEGIN


/* x386Config.c */

extern void x386Config();

extern void x386LookupMode(
#if NeedFunctionPrototypes
	DisplayModePtr,		/* target */
	ScrnInfoPtr		/* driver */
#endif 
);


/* x386Cursor.c */

extern void x386InitViewport(
#if NeedFunctionPrototypes
	ScrnInfoPtr		/* pScr */
#endif 
);

extern void x386SetViewport(
#if NeedFunctionPrototypes
	ScreenPtr,		/* pScreen */
	int,			/* x */
	int			/* y */
#endif 
);

extern void x386ZoomViewport(
#if NeedFunctionPrototypes
	ScreenPtr,		/* pScreen */
	int			/* zoom */
#endif 
);


/* x386Events.c */

extern void ProcessInputEvents();

extern void x386PostKbdEvent(
#if NeedFunctionPrototypes
	unsigned		/* key */
#endif 
);

extern void x386PostMseEvent(
#if NeedFunctionPrototypes
	int,			/* buttons */
	int,			/* dx */
	int			/* dy */
#endif
);

extern void x386Block(
#if NeedFunctionPrototypes
	pointer,		/* blockData */
	pointer,		/* pTimeout */
	long *			/* pReadmask */
#endif
);

extern void x386Wakeup(
#if NeedFunctionPrototypes
	pointer,		/* blockData */
	unsigned long,		/* err */
	long *			/* pReadmask */
#endif
);

extern void x386VTRequest(
#if NeedFunctionPrototypes
	int			/* signo */
#endif
);


/* x386Io.c */

extern void x386KbdLeds();

extern int  x386KbdProc(
#if NeedFunctionPrototypes
	DevicePtr,		/* pKeyboard */
	int			/* what */
#endif
);

extern void x386KbdEvents();

extern int  x386MseProc(
#if NeedFunctionPrototypes
	DevicePtr,		/* pPointer */
	int			/* what */
#endif
);

extern void x386MseEvents();

extern int  x386XqueKbdProc(
#if NeedFunctionPrototypes
	DevicePtr,		/* pKeyboard */
	int			/* what */
#endif
);

extern int  x386XqueMseProc(
#if NeedFunctionPrototypes
	DevicePtr,		/* pPointer */
	int			/* what */
#endif
);

extern void x386XqueEvents();



/* x386Kbd.c */

extern void x386KbdGetMapping(
#if NeedFunctionPrototypes
	KeySymsRec *,		/* pKeySyms */
	CARD8 *			/* pModMap */
#endif
);

_XFUNCPROTOEND

#endif /* _X386PROCS_H */


