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
 * $XConsortium: mips2030.h,v 1.3 91/09/22 10:52:13 rws Exp $
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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mips2030.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
#ifndef __DDX_MIPS2030_H
#define	__DDX_MIPS2030_H

/*
 * RS2030 frame buffer description
 */

/* Brooktree Bt458 RAMDAC registers */
struct bt458 {
	char pad0[3];
	volatile unsigned char addr;
	char pad1[3];
	volatile unsigned char cmap;
	char pad2[3];
	volatile unsigned char ctrl;
	char pad3[3];
	volatile unsigned char omap;
};

/* Bt458 addresses */
#define	BT458_READMASK	4
#define	BT458_BLINKMASK	5
#define	BT458_CMD	6
#define		BT458_CMD_MUX5	0x80	/* enable 5:1 multiplexing */
#define		BT458_CMD_RAMEN	0x40	/* enable color map RAM */
#define		BT458_CMD_BR1	0x20	/* blink rate */
#define		BT458_CMD_BR0	0x10	/* blink rate */
#define		BT458_CMD_BE1	0x08	/* OL1 blink enable */
#define		BT458_CMD_BE0	0x04	/* OL0 blink enable */
#define		BT458_CMD_OL1	0x02	/* OL1 display enable */
#define		BT458_CMD_OL0	0x01	/* OL0 display enable */
#define	BT458_TEST	7

/* frame buffer registers */
struct rs2030_reg {
	short intclr;		/* 0x0000 retrace interrupt clear */
	char pad0[0x80 - 2];
	short blank;		/* 0x0080 blank screen */
	char pad1[0x1000 - 0x80 - 2];
	short unblank;		/* 0x1000 unblank screen */
	char pad2[0xff00 - 0x1000 - 2];
	struct bt458 ramdac;	/* 0xff00 RAMDAC */
};

#define	RS2030_VISW	1280	/* visible pixels per scan line */
#define	RS2030_VISH	1024	/* scan lines */
#define	RS2030_BPSL	2048	/* bytes per scan line */

struct rs2030_fb {
	struct rs2030_line {
		char vis[RS2030_VISW];
		char invis[RS2030_BPSL - RS2030_VISW];
	} line[RS2030_VISH];
};

#endif /* __DDX_MIPS2030_H */
