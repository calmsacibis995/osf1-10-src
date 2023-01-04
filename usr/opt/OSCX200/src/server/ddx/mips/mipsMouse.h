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
 * $XConsortium: mipsMouse.h,v 1.4 91/07/18 22:58:50 keith Exp $
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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsMouse.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
#ifndef __DDX_MIPSMOUSE_H
#define	__DDX_MIPSMOUSE_H

#define NBUTTONS	3

#define RAW_LEFT	4
#define RAW_MIDDLE	2
#define RAW_RIGHT	1

/* mouse device name */

#define	MOUSEDEV	"/dev/mouse"

/* motion buffer */
#ifdef X11R4
#define MOUSEQSIZE      100

typedef struct {
    int		time;
    int		scrn;
    int		dx, dy;
    int		bmask;
} mouseQ_t;
#endif /* X11R4 */

/* macros for i/o with mouse device */

#define LPACKETSIZE	5
#define LSYNCBIT	0x80
#define BUTTONMASK(x)	((~(x)) & 0x07)

/* mouse private structure */

typedef
struct  _mousePriv {
    int		fd;		/* File descriptor of the mouse */
    short	unit;		/* which mouse /dev/tty(unit) */
    short	cap;		/* Device capabilities */
    short	baud;		/* Baud rate */
    short	rate;		/* Sampling rate */
    INT16	rootX, rootY;	/* Current mouse position */
    char	type;		/* mouse type */
#define	MIPS_MOUSE_DEFAULT	0
#define	MIPS_MOUSE_MOUSEMAN	1
    char	buttonstate;	/* Which buttons are down */
    struct {
	char mask;
	char val;
    } buttonmap[NBUTTONS];	/* Conversion between mouse and cursor buttons */
    int		scrn;		/* What screen is the cursor on */
    PtrCtrl	ctrl;		/* Pointer control */
} MousePriv, *MousePrivPtr;

/* keyboard and mouse defines */

extern MousePriv	mousePriv;

#endif /* __DDX_MIPSMOUSE_H */
