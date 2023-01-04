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
 * $XConsortium: mips3230.h,v 1.4 91/07/18 22:58:07 keith Exp $
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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mips3230.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
#ifndef __DDX_MIPS3230_H
#define	__DDX_MIPS3230_H

/*
 * RS3230 frame buffer description
 */

#ifdef USE_CHAR
typedef unsigned char	DACBITS
#define PAD_TO_DACBITS(var) unsigned char var[3];
#else
typedef unsigned int	DACBITS;
#define PAD_TO_DACBITS(var) 
#endif

/* Brooktree Bt459 RAMDAC registers */
struct bt459 {
	PAD_TO_DACBITS(pad0)
	volatile DACBITS	adlo;
	unsigned char pad1[4096 - sizeof (DACBITS)];
	volatile DACBITS	adhi;
	unsigned char pad2[4096 - sizeof (DACBITS)];
	volatile DACBITS	ctrl;
	unsigned char pad3[4096 - sizeof (DACBITS)];
	volatile DACBITS	cmap;
	unsigned char pad4[4096 - sizeof (DACBITS)];
};

#define	BT459_SETADDR(r, a)	((r)->adhi = (a) >> 8, (r)->adlo = (a))

/* Bt459 addresses */
#define	BT459_CMAPRAM	0x000
#define	BT459_CURSCOL1	0x181
#define	BT459_CURSCOL2	0x182
#define	BT459_CURSCOL3	0x183
#define	BT459_ID	0x200
#define		BT459_ID_ID		0x4a	/* ID register value */
#define	BT459_CR0	0x201
#define	BT459_CR1	0x202
#define	BT459_CR2	0x203
#define		BT459_CR2_XCURS		2	/* X mode cursor */
#define	BT459_READMASK	0x204
#define	BT459_BLINKMASK	0x206
#define	BT459_CURSCMD	0x300
#define		BT459_CURSCMD_CURSEN	0xc0	/* cursor enable */
#define	BT459_CURSXLO	0x301
#define	BT459_CURSXHI	0x302
#define	BT459_CURSYLO	0x303
#define	BT459_CURSYHI	0x304
#define	BT459_CURSRAM	0x400

#define	BT459_CURSMAX	64	/* cursor size */
#define	BT459_CURSBYTES	(64 * 64 * 2 / 8)

/* cursor constants */
#define	BT459_CURSK_P	52
#define	BT459_CURSK_H	(388 - 4)
#define	BT459_CURSK_V	(30 - 1)

/* cursor address correction -- add result to x, y */
#define	BT459_CURSFIXX(xhot)	(BT459_CURSK_H - BT459_CURSK_P + 31 - (xhot))
#define	BT459_CURSFIXY(yhot)	(BT459_CURSK_V - 32 + 31 - (yhot))

/* color frame buffer registers */
struct rs3230c_reg {
	struct bt459 ramdac;
	char pad0[7];
	volatile unsigned char xserver;
#define		RS3230C_XSERVER_UNBLANK	1
#define		RS3230C_XSERVER_PACKED	2
#define		RS3230C_XSERVER_FILL	4
	char pad1[4095];
	volatile unsigned char kernel;
#define		RS3230C_KERNEL_HBLANK	0x10
#define		RS3230C_KERNEL_VBLANK	0x20
#define		RS3230C_KERNEL_IDMASK	0xc0
	char pad2[4088];
	volatile unsigned int mask;
	char pad3[4092];
};

/* color frame buffer definitions */
#define	RS3230C_VISW	1280	/* visible pixels per scan line */
#define	RS3230C_VISH	1024	/* scan lines */
#define	RS3230C_BPSL	4096	/* bytes per scan line (as mapped) */

/* unpacked mode frame buffer */
struct rs3230c_ufb {
	struct rs3230c_line {
		char vis[RS3230C_VISW];
		char invis[RS3230C_BPSL / 2 - RS3230C_VISW];
		char dup[RS3230C_BPSL / 2];
	} line[RS3230C_VISH];
};

/* packed mode frame buffer */
struct rs3230c_pfb {
	char fb[32][1024][64];
};

/* packed mode address macros */
#define	RS3230C_PYSHIFT	6
#define	RS3230C_PYMASK	(1023 << RS3230C_PYSHIFT)

#define	RS3230C_PXADDR(x)	((x) & 63 | (((x) >> 6) << 16))
#define	RS3230C_PYADDR(y)	((y) << RS3230C_PYSHIFT)
#define	RS3230C_PADDR(x, y)	(RS3230C_PXADDR(x) | RS3230C_PYADDR(y))


/* mono frame buffer definitions */
#define	RS3230M_VISW	1152
#define	RS3230M_VISH	900
#define	RS3230M_BPSL	(RS3230M_VISW / 8)

/* mono frame buffer */
struct rs3230m_fb {
	struct rs3230m_line {
		char vis[RS3230M_BPSL];
	} line[RS3230M_VISH];
};

#endif /* __DDX_MIPS3230_H */
