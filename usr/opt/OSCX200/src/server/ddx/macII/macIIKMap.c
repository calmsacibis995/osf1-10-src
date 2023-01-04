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
/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include	"macII.h"
#include	"keysym.h"

static KeySym macIIMap[] = {
	XK_A,		NoSymbol,		/* 0x00 */
	XK_S,		NoSymbol,		/* 0x01 */
	XK_D,		NoSymbol,		/* 0x02 */
	XK_F,		NoSymbol,		/* 0x03 */
	XK_H,		NoSymbol,		/* 0x4 */
	XK_G,		NoSymbol,		/* 0x5 */
	XK_Z,		NoSymbol,		/* 0x6 */
	XK_X,		NoSymbol,		/* 0x7 */
	XK_C,		NoSymbol,		/* 0x8 */
	XK_V,		NoSymbol,		/* 0x9 */
	NoSymbol,	NoSymbol,		/* 0xa */
	XK_B,		NoSymbol,		/* 0xb */
	XK_Q,		NoSymbol,		/* 0xc */
	XK_W,		NoSymbol,		/* 0xd */
	XK_E,		NoSymbol,		/* 0xe */
	XK_R,		NoSymbol,		/* 0xf */
	XK_Y,		NoSymbol,		/* 0x10 */
	XK_T,		NoSymbol,		/* 0x11 */
	XK_1,		XK_exclam,		/* 0x12 */
	XK_2,		XK_at,			/* 0x13 */
	XK_3,		XK_numbersign,		/* 0x14 */
	XK_4,		XK_dollar,		/* 0x15 */
	XK_6,		XK_asciicircum,		/* 0x16 */
	XK_5,		XK_percent,		/* 0x17 */
	XK_equal,	XK_plus,		/* 0x18 */
	XK_9,		XK_parenleft,		/* 0x19 */
	XK_7,		XK_ampersand,		/* 0x1a */
	XK_minus,	XK_underscore,		/* 0x1b */
	XK_8,		XK_asterisk,		/* 0x1c */
	XK_0,		XK_parenright,		/* 0x1d */
	XK_bracketright,XK_braceright,		/* 0x1e */
	XK_O,		NoSymbol,		/* 0x1f */
	XK_U,		NoSymbol,		/* 0x20 */
	XK_bracketleft,	XK_braceleft,		/* 0x21 */
	XK_I,		NoSymbol,		/* 0x22 */
	XK_P,		NoSymbol,		/* 0x23 */
	XK_Return,	NoSymbol,		/* 0x24 */
	XK_L,		NoSymbol,		/* 0x25 */
	XK_J,		NoSymbol,		/* 0x26 */
	XK_quoteright,	XK_quotedbl,		/* 0x27 */
	XK_K,		NoSymbol,		/* 0x28 */
	XK_semicolon,	XK_colon,		/* 0x29 */
	XK_backslash,	XK_bar,			/* 0x2a */
	XK_comma,	XK_less,		/* 0x2b */
	XK_slash,	XK_question,		/* 0x2c */
	XK_N,		NoSymbol,		/* 0x2d */
	XK_M,		NoSymbol,		/* 0x2e */
	XK_period,	XK_greater,		/* 0x2f */
	XK_Tab,		NoSymbol,		/* 0x30 */
	XK_space,	NoSymbol,		/* 0x31 */
	XK_quoteleft,	XK_asciitilde,		/* 0x32 */
	XK_Delete,	NoSymbol,		/* 0x33 */
	NoSymbol,	NoSymbol,		/* 0x34 */
	XK_Escape,	NoSymbol,		/* 0x35 */
	XK_Control_L,	NoSymbol,		/* 0x36 */
	XK_Meta_L,	NoSymbol,		/* 0x37 */ /* apple cloverleaf */
	XK_Shift_L,	NoSymbol,		/* 0x38 */
	XK_Caps_Lock,	NoSymbol,		/* 0x39 */
	NoSymbol,	NoSymbol,		/* 0x3a */ /* option */
	XK_Left,	NoSymbol,		/* 0x3b */
	XK_Right,	NoSymbol,		/* 0x3c */
	XK_Control_R,	NoSymbol,		/* 0x3d */ /* down arrow key */
	XK_Meta_R,	NoSymbol,		/* 0x3e */ /* up arrow key */
	NoSymbol,	NoSymbol,		/* 0x3f */
	NoSymbol,	NoSymbol,		/* 0x40 */
	XK_KP_Decimal,	NoSymbol,		/* 0x41 */
	NoSymbol,	NoSymbol,		/* 0x42 */
	XK_KP_Multiply,	NoSymbol,		/* 0x43 */
	NoSymbol,	NoSymbol,		/* 0x44 */
	XK_KP_Add,	NoSymbol,		/* 0x45 */
	NoSymbol,	NoSymbol,		/* 0x46 */
	XK_Clear,	NoSymbol,		/* 0x47 */
	NoSymbol,	NoSymbol,		/* 0x48 */
	NoSymbol,	NoSymbol,		/* 0x49 */
	NoSymbol,	NoSymbol,		/* 0x4a */
	XK_KP_Divide,	NoSymbol,		/* 0x4b */
	XK_KP_Enter,	NoSymbol,		/* 0x4c */
	NoSymbol,	NoSymbol,		/* 0x4d */
	XK_KP_Subtract,	NoSymbol,		/* 0x4e */
	NoSymbol,	NoSymbol,		/* 0x4f */
	NoSymbol,	NoSymbol,		/* 0x50 */
	XK_KP_Equal,	NoSymbol,		/* 0x51 */
	XK_KP_0,	NoSymbol,		/* 0x52 */
	XK_KP_1,	NoSymbol,		/* 0x53 */
	XK_KP_2,	NoSymbol,		/* 0x54 */
	XK_KP_3,	NoSymbol,		/* 0x55 */
	XK_KP_4,	NoSymbol,		/* 0x56 */
	XK_KP_5,	NoSymbol,		/* 0x57 */
	XK_KP_6,	NoSymbol,		/* 0x58 */
	XK_KP_7,	NoSymbol,		/* 0x59 */
	NoSymbol,	NoSymbol,		/* 0x5a */
	XK_KP_8,	NoSymbol,		/* 0x5b */
	XK_KP_9,	NoSymbol,		/* 0x5c */
	NoSymbol,	NoSymbol,		/* 0x5d */
	NoSymbol,	NoSymbol,		/* 0x5e */
	NoSymbol,	NoSymbol,		/* 0x5f */
	XK_F5,		NoSymbol,		/* 0x60 */
	XK_F6,		NoSymbol,		/* 0x61 */
	XK_F7,		NoSymbol,		/* 0x62 */
	XK_F3,		NoSymbol,		/* 0x63 */
	XK_F8,		NoSymbol,		/* 0x64 */
	XK_F9,		NoSymbol,		/* 0x65 */
	NoSymbol,	NoSymbol,		/* 0x66 */
	XK_F11,		NoSymbol,		/* 0x67 */
	NoSymbol,	NoSymbol,		/* 0x68 */
	XK_F13,		XK_Print,		/* 0x69 */
	NoSymbol,	NoSymbol,		/* 0x6a */
	XK_F14,		XK_Pause,		/* 0x6b */
	NoSymbol,	NoSymbol,		/* 0x6c */
	XK_F10,		NoSymbol,		/* 0x6d */
	NoSymbol,	NoSymbol,		/* 0x6e */
	XK_F12,		NoSymbol,		/* 0x6f */
	/* I assume you can't generate 0x70 code from any keyboard.  MGC. */
	XK_Up,		NoSymbol,		/* 0x70 */ 
	XK_F15,		XK_Pause,		/* 0x71 */
	XK_Help,	XK_Insert,		/* 0x72 */
	XK_Home,	NoSymbol,		/* 0x73 */
	XK_Prior,	NoSymbol,		/* 0x74 */ /* Page Up */
	XK_Delete,	NoSymbol,		/* 0x75 */
	XK_F4,		NoSymbol,		/* 0x76 */
	XK_End,		NoSymbol,		/* 0x77 */
	XK_F2,		NoSymbol,		/* 0x78 */
	XK_Next,	NoSymbol,		/* 0x79 */ /* Page Down */
	XK_F1,		NoSymbol,		/* 0x7a */
	XK_Right,	NoSymbol,		/* 0x7b */
	XK_Left,	NoSymbol,		/* 0x7c */
	XK_Down,	NoSymbol,		/* 0x7d */
	NoSymbol,	NoSymbol,		/* 0x7e */
	NoSymbol,	NoSymbol,		/* 0x7f */ /* Soft Power */
};

KeySymsRec macIIKeySyms[] = {
    /*	map	   minKeyCode	maxKC	width */
    macIIMap,		0,	0x7e,	2,
};

#define	cT	(ControlMask)
#define	sH	(ShiftMask)
#define	lK	(LockMask)
#define	mT	(Mod1Mask)
static CARD8 macIImodmap[MAP_LENGTH] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  cT, mT,/* 30-3f */
    sH, lK, 0,  0,  0, cT, mT,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 40-4f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 50-5f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 60-6f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 70-7f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 80-8f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 90-9f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* a0-af */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* b0-bf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* c0-cf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* d0-df */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* e0-ef */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* f0-ff */
};

CARD8 *macIIModMap[] = {
	macIImodmap,
};
