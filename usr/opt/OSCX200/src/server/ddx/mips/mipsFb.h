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
 * $XConsortium: mipsFb.h,v 1.4 91/07/18 22:58:18 keith Exp $
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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsFb.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
#ifndef __DDX_MIPSFB_H
#define	__DDX_MIPSFB_H

/* private per-screen information */

typedef struct _MipsScreen {
	char type;		/* screen type */
#define		MIPS_SCRTYPE_DISABLED	0
#define		MIPS_SCRTYPE_COLOR	1
#define		MIPS_SCRTYPE_MONO	2
	char unit;		/* display unit number */
	char bitsPerPixel;	/* bits occupied */
	char depth;		/* bits actually present */
	short dpi;		/* resolution */

	short scr_width;	/* visible */
	short scr_height;	/* visible */
	unsigned char *fbnorm;	/* normal framebuffer mapping */
	unsigned char *fbcache;	/* cached framebuffer mapping */
	unsigned char *fbnocache; /* uncached framebuffer mapping */
	unsigned char *fbspec;	/* special framebuffer mapping */
	unsigned char *fbreg;	/* framebuffer registers */
	int fb_width;		/* pixels per scanline (fbnorm) */

	int cap;		/* device capabilities */
#define		MIPS_SCR_CURSOR	1	/* HW cursor */
#define		MIPS_SCR_PACKED	2	/* packed mode */
#define		MIPS_SCR_FILL	4	/* fill mode */
#define		MIPS_SCR_MASK	8	/* plane mask */

	Bool (*CloseScreen)();	/* wrapped screen close */

	void (*Blank)();	/* screen blank/unblank */
	void (*WriteCMap)();	/* colormap load */
	void (*Close)();	/* restore normal state */

	Bool (*RealizeCursor)(); /* HW cursor support */
	void (*SetCursor)();
	void (*MoveCursor)();
	void (*RecolorCursor)();
	int xhot, yhot;		/* cursor hot spot (device dependent) */

	ColormapPtr InstalledMap; /* current colormap for this screen */
} MipsScreenRec, *MipsScreenPtr;

extern MipsScreenRec mipsScreen[];

#define	MipsScreenToPriv(scr)	(&mipsScreen[(scr)->myNum])
#define	MipsScreenNumToPriv(n)	(&mipsScreen[(n)])

#endif /* __DDX_MIPSFB_H */
