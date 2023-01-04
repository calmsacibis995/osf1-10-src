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
 * $XConsortium: mipsKbdUNIX1.c,v 1.5 91/07/18 22:58:39 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsKbdUNIX1.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 *	keysym mapping for the unix keyboard.
 */

#include <sys/types.h>
#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "keysym.h"
#include "mips.h"
#include "mipsKbd.h"

extern void specialKeybdEvent(), genKeybdEvent();

/* UNIX1 keyboard type */

#ifdef UNIX1_KEYBOARD
KeySym unix1_KeyMap[] = {
    XK_F1,		NoSymbol,	/* 0x07 F1 */
    XK_Escape,		NoSymbol,	/* 0x08 esc */
    NoSymbol,		NoSymbol,	/* 0x09     */
    NoSymbol,		NoSymbol,	/* 0x0a     */
    NoSymbol,		NoSymbol,	/* 0x0b     */
    NoSymbol,		NoSymbol,	/* 0x0c     */
    XK_Tab,		NoSymbol,	/* 0x0d tab */
    XK_quoteleft,	XK_asciitilde,	/* 0x0e ` ~ */
    XK_F2,		NoSymbol,	/* 0x0f F2  */
    NoSymbol,		NoSymbol,	/* 0x10     */
    XK_Control_L,	NoSymbol,	/* 0x11 CONTROL   */
    XK_Shift_L,		NoSymbol,	/* 0x12 SHIFT */
    NoSymbol,		NoSymbol,	/* 0x13     */
    XK_Caps_Lock,	NoSymbol,	/* 0x14 CAPS LOCK   */
    XK_Q,		NoSymbol,	/* 0x15 Q  */
    XK_1,		XK_exclam,	/* 0x16 1 ! */
    XK_F3,		NoSymbol,	/* 0x17 F3  */
    NoSymbol,		NoSymbol,	/* 0x18     */
    XK_Alt_L,		NoSymbol,	/* 0x19 Left ALT   */
    XK_Z,		NoSymbol,	/* 0x1a Z  */
    XK_S,		NoSymbol,	/* 0x1b S  */
    XK_A,		NoSymbol,	/* 0x1c A  */
    XK_W,		NoSymbol,	/* 0x1d W  */
    XK_2,		XK_at,		/* 0x1e 2 @ */
    XK_F4,		NoSymbol,	/* 0x1f F4  */
    NoSymbol,		NoSymbol,	/* 0x20     */
    XK_C,		NoSymbol,	/* 0x21 C  */
    XK_X,		NoSymbol,	/* 0x22 X  */
    XK_D,		NoSymbol,	/* 0x23 D  */
    XK_E,		NoSymbol,	/* 0x24 E  */
    XK_4,		XK_dollar,	/* 0x25 4 $ */
    XK_3,		XK_numbersign,	/* 0x26 3 # */
    XK_F5,		NoSymbol,	/* 0x27 F5  */
    NoSymbol,		NoSymbol,	/* 0x28     */
    XK_space,		NoSymbol,	/* 0x29 sp */
    XK_V,		NoSymbol,	/* 0x2a V  */
    XK_F,		NoSymbol,	/* 0x2b F  */
    XK_T,		NoSymbol,	/* 0x2c T  */
    XK_R,		NoSymbol,	/* 0x2d R  */
    XK_5,		XK_percent,	/* 0x2e 5 % */
    XK_F6,		NoSymbol,	/* 0x2f F6  */
    NoSymbol,		NoSymbol,	/* 0x30     */
    XK_N,		NoSymbol,	/* 0x31 N  */
    XK_B,		NoSymbol,	/* 0x32 B  */
    XK_H,		NoSymbol,	/* 0x33 H  */
    XK_G,		NoSymbol,	/* 0x34 G  */
    XK_Y,		NoSymbol,	/* 0x35 Y  */
    XK_6,		XK_asciicircum,	/* 0x36 6 ^ */
    XK_F7,		NoSymbol,	/* 0x37 F7  */
    NoSymbol,		NoSymbol,	/* 0x38     */
    XK_Alt_R,		NoSymbol,	/* 0x39 Right alt */
    XK_M,		NoSymbol,	/* 0x3a M  */
    XK_J,		NoSymbol,	/* 0x3b J  */
    XK_U,		NoSymbol,	/* 0x3c U  */
    XK_7,		XK_ampersand,	/* 0x3d 7 & */
    XK_8,		XK_asterisk,	/* 0x3e 8 * */
    XK_F8,		NoSymbol,	/* 0x3f F8  */
    NoSymbol,		NoSymbol,	/* 0x40     */
    XK_comma,		XK_less,	/* 0x41 , <  */
    XK_K,		NoSymbol,	/* 0x42 K  */
    XK_I,		NoSymbol,	/* 0x43 I  */
    XK_O,		NoSymbol,	/* 0x44 O  */
    XK_0,		XK_parenright,	/* 0x45 0 ) */
    XK_9,		XK_parenleft,	/* 0x46 9 ( */
    XK_F9,		NoSymbol,	/* 0x47 F9  */
    NoSymbol,		NoSymbol,	/* 0x48     */
    XK_period,		XK_greater,	/* 0x49 . >  */
    XK_slash,		XK_question,	/* 0x4a / ? */
    XK_L,		NoSymbol,	/* 0x4b L  */
    XK_semicolon,	XK_colon,	/* 0x4c ; : */
    XK_P,		NoSymbol,	/* 0x4d P  */
    XK_minus,		XK_underscore,	/* 0x4e - _ */
    XK_F10,		NoSymbol,	/* 0x4f F10 */
    NoSymbol,		NoSymbol,	/* 0x50     */
    NoSymbol,		NoSymbol,	/* 0x51     */
    XK_quoteright,	XK_quotedbl,	/* 0x52 ' " */
    NoSymbol,		NoSymbol,	/* 0x53     */
    XK_bracketleft,	XK_braceleft,	/* 0x54 [ { */
    XK_equal,		XK_plus,	/* 0x55 = + */
    XK_F11,		NoSymbol,	/* 0x56 F11 */
    XK_Linefeed,	NoSymbol,	/* 0x57 Linefeed */
    XK_Control_R,	NoSymbol,	/* 0x58 Right control */
    XK_Shift_R,		NoSymbol,	/* 0x59 SHIFT */
    XK_Return,		NoSymbol,	/* 0x5a RETURN */
    XK_bracketright,	XK_braceright,	/* 0x5b ] } */
    XK_backslash,	XK_bar,		/* 0x5c \ | */
    NoSymbol,		NoSymbol,	/* 0x5d     */
    XK_F12,		NoSymbol,	/* 0x5e F12 */
    XK_Break,		NoSymbol,	/* 0x5f KP_Break */
    XK_Down,		NoSymbol,	/* 0x60 down arrow  */
    XK_Left,		NoSymbol,	/* 0x61 left arrow  */
    XK_Menu,		NoSymbol,	/* 0x62 Menu */
    XK_Up,		NoSymbol,	/* 0x63 uparrow */
    XK_Delete,		NoSymbol,	/* 0x64 DEL */
    NoSymbol,		NoSymbol,	/* 0x65     */
    XK_BackSpace,	NoSymbol,	/* 0x66 bs  */
    NoSymbol,		NoSymbol,	/* 0x67     */
    NoSymbol,		NoSymbol,	/* 0x68     */
    XK_KP_1,		XK_End,		/* 0x69 1  */
    XK_Right,		NoSymbol,	/* 0x6a right arrow */
    XK_KP_4,		XK_Left,	/* 0x6b 4  */
    XK_KP_7,		XK_Home,	/* 0x6c 7  */
    XK_KP_Separator,	NoSymbol,	/* 0x6d ,  */
    NoSymbol,		NoSymbol,	/* 0x6e     */
    NoSymbol,		NoSymbol,	/* 0x6f     */
    XK_KP_0,		XK_Insert,	/* 0x70 0  */
    XK_KP_Decimal,	XK_Delete,	/* 0x71 .  */
    XK_KP_2,		XK_Down,	/* 0x72 2  */
    XK_KP_5,		NoSymbol,	/* 0x73 5  */
    XK_KP_6,		XK_Right,	/* 0x74 6  */
    XK_KP_8,		XK_Up,		/* 0x75 8  */
    XK_KP_F1,		NoSymbol,	/* 0x76 PF1 */
    XK_KP_F2,		NoSymbol,	/* 0x77 PF2 */
    NoSymbol,		NoSymbol,	/* 0x78     */
    XK_KP_Enter,	NoSymbol,	/* 0x79 KP_ENTER */
    XK_KP_3,		XK_Next,	/* 0x7a 3  */
    NoSymbol,		NoSymbol,	/* 0x7b     */
    XK_KP_F4,		NoSymbol,	/* 0x7c PF4 */
    XK_KP_9,		XK_Prior,	/* 0x7d 9  */
    XK_KP_F3,		NoSymbol,	/* 0x7e PF3 */
    NoSymbol,		NoSymbol,	/* 0x7f     */
    NoSymbol,		NoSymbol,	/* 0x80     */
    NoSymbol,		NoSymbol,	/* 0x81     */
    NoSymbol,		NoSymbol,	/* 0x82     */
    NoSymbol,		NoSymbol,	/* 0x83     */
    XK_KP_Subtract,	NoSymbol,	/* 0x84 -  */
};

CARD8 unix1_ModMap[MAP_LENGTH] = {
/* 00-0f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10-1f */	 0, 0, 0,MC,MS, 0,ML, 0, 0, 0, 0,M1, 0, 0, 0, 0,
/* 20-2f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 30-3f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,M1, 0, 0, 0, 0,
/* 40-4f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 50-5f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,MC,MS, 0, 0, 0, 0,
/* 60-6f */	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
unix1KeybdEvent(pKeybd, code)
DevicePtr	pKeybd;
u_char		code;
{
    static int		release = 0;
    u_char		kindex;

    /* Random 0xfa and 0xfe characters are being delivered */

    if ((code & 0xfa) == 0xfa)
	return;

    if (code == 0xf0)
	release = 1;
    else {
	kindex = code;
	if (kindex == 0x62) {
	    if (release)
		specialKeybdEvent();
	}

	kindex += keybdType[UNIX1_KEYBOARD].offset;
	genKeybdEvent(pKeybd, release, kindex);
	release = 0;
    }
}
#endif /* UNIX1_KEYBOARD */
