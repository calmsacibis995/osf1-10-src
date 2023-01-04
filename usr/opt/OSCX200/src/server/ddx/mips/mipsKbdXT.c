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
 * $XConsortium: mipsKbdXT.c,v 1.5 91/07/18 22:58:41 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsKbdXT.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 *	keysym mapping for the XT keyboard.
 */

#include <sys/types.h>
#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "keysym.h"
#include "mips.h"
#include "mipsKbd.h"

extern void specialKeybdEvent(), genKeybdEvent();

/* PC/XT keyboard type */

#ifdef XT_KEYBOARD
KeySym xt_KeyMap[] = {
    XK_Escape,		NoSymbol,	/* 0x01 esc(arbitrary mapping) */
    XK_1,		XK_exclam,	/* 0x02 1 ! */
    XK_2,		XK_at,		/* 0x03 2 @@ */
    XK_3,		XK_numbersign,	/* 0x04 3 # */
    XK_4,		XK_dollar,	/* 0x05 4 $ */
    XK_5,		XK_percent,	/* 0x06 5 % */
    XK_6,		XK_asciicircum,	/* 0x07 6 ^ */
    XK_7,		XK_ampersand,	/* 0x08 7 & */
    XK_8,		XK_asterisk,	/* 0x09 8 * */
    XK_9,		XK_parenleft,	/* 0x0a 9 ( */
    XK_0,		XK_parenright,	/* 0x0b 0 ) */
    XK_minus,		XK_underscore,	/* 0x0c - _ */
    XK_equal,		XK_plus,	/* 0x0d = + */
    XK_BackSpace,	NoSymbol,	/* 0x0e bs  */
    XK_Tab,		NoSymbol,	/* 0x0f tab */
    XK_Q,		NoSymbol,	/* 0x10 Q  */
    XK_W,		NoSymbol,	/* 0x11 W  */
    XK_E,		NoSymbol,	/* 0x12 E  */
    XK_R,		NoSymbol,	/* 0x13 R  */
    XK_T,		NoSymbol,	/* 0x14 T  */
    XK_Y,		NoSymbol,	/* 0x15 Y  */
    XK_U,		NoSymbol,	/* 0x16 U  */
    XK_I,		NoSymbol,	/* 0x17 I  */
    XK_O,		NoSymbol,	/* 0x18 O  */
    XK_P,		NoSymbol,	/* 0x19 P  */
    XK_bracketleft,	XK_braceleft,	/* 0x1a [ { */
    XK_bracketright,	XK_braceright,	/* 0x1b ] } */
    XK_Return,		NoSymbol,	/* 0x1c RETURN */
    XK_Control_L,	NoSymbol,	/* 0x1d CONTROL */
    XK_A,		NoSymbol,	/* 0x1e A  */
    XK_S,		NoSymbol,	/* 0x1f S  */
    XK_D,		NoSymbol,	/* 0x20 D  */
    XK_F,		NoSymbol,	/* 0x21 F  */
    XK_G,		NoSymbol,	/* 0x22 G  */
    XK_H,		NoSymbol,	/* 0x23 H  */
    XK_J,		NoSymbol,	/* 0x24 J  */
    XK_K,		NoSymbol,	/* 0x25 K  */
    XK_L,		NoSymbol,	/* 0x26 L  */
    XK_semicolon,	XK_colon,	/* 0x27 ; : */
    XK_quoteright,	XK_quotedbl,	/* 0x28 ' " */
    XK_quoteleft,	XK_asciitilde,	/* 0x29 ` ~ */
    XK_Shift_L,		NoSymbol,	/* 0x2a SHIFT */
    XK_backslash,	XK_bar,		/* 0x2b \ | */
    XK_Z,		NoSymbol,	/* 0x2c z  */
    XK_X,		NoSymbol,	/* 0x2d x  */
    XK_C,		NoSymbol,	/* 0x2e c  */
    XK_V,		NoSymbol,	/* 0x2f v  */
    XK_B,		NoSymbol,	/* 0x30 b  */
    XK_N,		NoSymbol,	/* 0x31 n  */
    XK_M,		NoSymbol,	/* 0x32 m  */
    XK_comma,		XK_less,	/* 0x33 , < */
    XK_period,		XK_greater,	/* 0x34 . > */
    XK_slash,		XK_question,	/* 0x35 / ? */
    XK_Shift_R,		NoSymbol,	/* 0x36 SHIFT */
    XK_KP_Multiply,	NoSymbol,	/* 0x37 *  */
    XK_Alt_L,		NoSymbol,	/* 0x38 Left ALT   */
    XK_space,		NoSymbol,	/* 0x39 sp */
    XK_Caps_Lock,	NoSymbol,	/* 0x3a CAPS LOCK   */
    XK_F1,		NoSymbol,	/* 0x3b F1  */
    XK_F2,		NoSymbol,	/* 0x3c F2  */
    XK_F3,		NoSymbol,	/* 0x3d F3  */
    XK_F4,		NoSymbol,	/* 0x3e F4  */
    XK_F5,		NoSymbol,	/* 0x3f F5  */
    XK_F6,		NoSymbol,	/* 0x40 F6  */
    XK_F7,		NoSymbol,	/* 0x41 F7  */
    XK_F8,		NoSymbol,	/* 0x42 F8  */
    XK_F9,		NoSymbol,	/* 0x43 F9  */
    XK_F10,		NoSymbol,	/* 0x44 F10 */
    XK_Num_Lock,	NoSymbol,	/* 0x45 Num Lock */
    XK_Pause,		NoSymbol,	/* 0x46 Scroll Lock */
    XK_KP_7,		NoSymbol,	/* 0x47 7  */
    XK_KP_8,		NoSymbol,	/* 0x48 8  */
    XK_KP_9,		NoSymbol,	/* 0x49 9  */
    XK_KP_Subtract,	NoSymbol,	/* 0x4a -  */
    XK_KP_4,		NoSymbol,	/* 0x4b 4  */
    XK_KP_5,		NoSymbol,	/* 0x4c 5  */
    XK_KP_6,		NoSymbol,	/* 0x4d 6  */
    XK_KP_Add,		NoSymbol,	/* 0x4e +  */
    XK_KP_1,		NoSymbol,	/* 0x4f 1  */
    XK_KP_2,		NoSymbol,	/* 0x50 2  */
    XK_KP_3,		NoSymbol,	/* 0x51 3  */
    XK_KP_0,		NoSymbol,	/* 0x52 0  */
    XK_KP_Decimal,	NoSymbol,	/* 0x53 .  */
    XK_Print,		NoSymbol,	/* 0x54 Print */
    XK_Prior,		NoSymbol,	/* 0x55 Prev Screen  */
    XK_Next,		NoSymbol,	/* 0x56 Next Screen  */
    XK_F11,		NoSymbol,	/* 0x57 F11 */
    XK_F12,		NoSymbol,	/* 0x58 F12 */
    XK_Up,		NoSymbol,	/* 0x59 uparrow */
    XK_Left,		NoSymbol,	/* 0x5a left arrow  */
    XK_Down,		NoSymbol,	/* 0x5b down arrow  */
    XK_Right,		NoSymbol,	/* 0x5c right arrow */
    XK_Delete,		NoSymbol,	/* 0x5d DEL */
    XK_KP_Enter,	NoSymbol,	/* 0x5e ENTER */
    XK_Control_R,	NoSymbol,	/* 0x5f Right control */
    XK_Alt_R,		NoSymbol,	/* 0x60 Right alt */
    XK_Insert,		NoSymbol,	/* 0x61 Insert */
    XK_KP_Divide,	NoSymbol,	/* 0x62 KP_Divide */
    XK_Home,		NoSymbol,	/* 0x63 KP_Home */
    XK_End,		NoSymbol,	/* 0x64 KP_End */
    XK_Break,		NoSymbol,	/* 0x65 KP_Break */
};

CARD8 xt_ModMap[MAP_LENGTH] = {
/* 00-0f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10-1f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20-2f */	 0, 0, 0, 0, 0,MC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 30-3f */	 0, 0,MS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,MS, 0,
/* 40-4f */	M1, 0,ML, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 50-5f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 60-6f */	 0, 0, 0, 0, 0, 0, 0,MC,M1, 0, 0, 0, 0, 0, 0, 0,
/* 70-7f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 80-8f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 90-9f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* a0-af */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* b0-bf */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* c0-cf */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* d0-df */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e0-ef */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* f0-ff */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void
xtKeybdEvent(pKeybd, code)
DevicePtr	pKeybd;
u_char		code;
{
    int			release;
    static int		translate = 0;
    u_char		kindex;

    /* Random 0xfa and 0xfe characters are being delivered */

    if ((code & 0xfa) == 0xfa)
	return;

    if (code == 0xe0)
	translate = 1;
    else if (code == 0xe1)
	translate = 2;
    else {
	kindex = code & 0x7f;
	release = code & 0x80;
	if (translate == 1) {
	    translate = 0;
	    switch (kindex) {
		case 0x1c:
		    kindex = 0x5e;	/* KP_Enter */
		    break;
		case 0x1d:
		    kindex = 0x5f;	/* Control_R */
		    break;
		case 0x35:
		    kindex = 0x62;	/* KP_Divide */
		    break;
		case 0x37:
		    kindex = 0x54;	/* Print */
		    break;
		case 0x38:
		    kindex = 0x60;	/* Alt_R */
		    break;
		case 0x46:
		    kindex = 0x65;	/* Break */
		    break;
		case 0x47:
		    kindex = 0x63;	/* Home */
		    break;
		case 0x48:
		    kindex = 0x59;	/* Up arrow */
		    break;
		case 0x49:
		    kindex = 0x55;	/* Prior/Page Up */
		    break;
		case 0x4b:
		    kindex = 0x5a;	/* Left arrow */
		    break;
		case 0x4d:
		    kindex = 0x5c;	/* Right arrow */
		    break;
		case 0x4f:
		    kindex = 0x64;	/* End */
		    break;
		case 0x50:
		    kindex = 0x5b;	/* Down arrow */
		    break;
		case 0x51:
		    kindex = 0x56;	/* Next/Page Down */
		    break;
		case 0x52:
		    kindex = 0x61;	/* Insert */
		    break;
		case 0x53:
		    kindex = 0x5d;	/* Delete */
		    break;
		case 0x2a:
		case 0xaa:
		default:
		    return;
	    }
	}
	else if (translate == 2) {
	    translate = (kindex == 0x1d) ? 3 : 0;
	    return;
	}
	else if (translate == 3) {
	    translate = 0;
	    if (kindex == 0x45) {
		kindex = 0x65;
		if (release)
		    specialKeybdEvent();
	    }
	    else
		return;
	}

	kindex += keybdType[XT_KEYBOARD].offset;
	genKeybdEvent(pKeybd, release, kindex);
    }
}
#endif /* XT_KEYBOARD */
