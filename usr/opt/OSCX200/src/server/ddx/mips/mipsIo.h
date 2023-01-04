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
 * $XConsortium: mipsIo.h,v 1.4 91/07/18 22:58:26 keith Exp $
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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsIo.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
#ifndef __DDX_MIPSIO_H
#define	__DDX_MIPSIO_H

#define MAXEVENTS       100

/* Device capabilities */

#define	DEV_TIMESTAMP	0x01	/* Timestamps enabled */
#define DEV_BUZZER	0x02	/* Buzzer enabled */
#define DEV_COLORMAP	0x04	/* Colormap enabled */
#define DEV_LIGHTS	0x08	/* lights enabled */
#define DEV_ASYNC	0x10	/* async io enabled */
#define DEV_READ	0x20	/* readable */
#define DEV_INIT	0x40	/* initable */
#define DEV_BLANK	0x80	/* Blanking enabled */

/* cursor private structure */

typedef
struct	_cursorpriv {
    int		srcWidth;	/* width of source bitmap in bytes. */
    int		width;		/* width of savearea in bytes. */
    int		size;		/* size of savearea required. */
    int		fg, bg;
} CursorPriv;

extern int		errno;
extern int		isItTimeToYield;
extern int		sigIOfunc();
extern int		lastEventTime;
extern ScreenInfo	screenInfo;
extern volatile int	avoid;
extern int		screenIsSaved;
extern void		SaveScreens();

#endif /* __DDX_MIPSIO_H */
