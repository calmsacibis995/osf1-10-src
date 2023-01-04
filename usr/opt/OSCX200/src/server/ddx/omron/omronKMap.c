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
 * $XConsortium: omronKMap.c,v 1.1 91/06/29 13:48:59 xguest Exp $
 *
 * Copyright 1991 by OMRON Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OMRON
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "omron.h"
#include "omronKbd.h"

#define cT      (ControlMask)
#define sH      (ShiftMask)
#define lK      (LockMask)
#define mT      (Mod1Mask)
#ifdef USE_KANA_SWITCH
#define m5      (Mod5Mask)
#endif

#ifndef USE_KANA_SWITCH
# ifdef USE_KEYCOMPATI
static CARD8 omronDefKeyModeMap[MAP_LENGTH] = {
/*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */    
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  cT, 0, sH, sH, lK, mT, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 40-4f */
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
# else
static CARD8 omronDefKeyModeMap[MAP_LENGTH] = {
/*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */    
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  cT, 0, sH, sH, lK, mT, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 40-4f */
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
    0,  0,  0,  0,  0,  0,  0,  0,  0,                             /* f0-ff */
};
# endif
#else
# ifdef USE_KEYCOMPATI
static CARD8 omronDefKeyModeMap[MAP_LENGTH] = {
/*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */    
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  cT, m5, sH, sH, lK, mT, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 40-4f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 50-5f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 60-6f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 70-7f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 80-8f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 90-9f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* a0-af */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* b0-bf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* c0-cf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* d0-df */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* e0-ef */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* f0-ff */
};
# else
static CARD8 omronDefKeyModeMap[MAP_LENGTH] = {
/*  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f  */    
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  cT, m5, sH, sH, lK, mT, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 40-4f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 50-5f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 60-6f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 70-7f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 80-8f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 90-9f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* a0-af */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* b0-bf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* c0-cf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* d0-df */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* e0-ef */
    0,  0,  0,  0,  0,  0,  0,  0,  0,                              /* f0-ff */
};
# endif
#endif

CARD8 *omronKeyModMap[] = {
#ifdef luna2
	omronDefKeyModeMap,
	omronDefKeyModeMap,
	omronDefKeyModeMap,
#endif
	omronDefKeyModeMap,
};

#define XK_henkan	XK_Kanji
#define XK_kakutei	XK_Execute

#ifndef USE_KANA_SWITCH
static KeySym omronDefKeyMap[] = {
/*      NormalShift             ShiftedShift            */
        XK_Tab,                 NoSymbol,               /* 0x9 -> min KeyCode */
        XK_Control_L,           NoSymbol,               /* 0xa */
        NoSymbol,               NoSymbol,               /* 0xb -> kana Key */
        XK_Shift_R,             NoSymbol,               /* 0xc */
        XK_Shift_L,             NoSymbol,               /* 0xd */
        XK_Caps_Lock,           NoSymbol,               /* 0xe */
        XK_Meta_L,              NoSymbol,               /* 0xf -> zenmen Key */
        XK_Escape,              NoSymbol,               /* 0x10 */
        XK_BackSpace,           NoSymbol,               /* 0x11 */
        XK_Return,              NoSymbol,               /* 0x12 */
        NoSymbol,               NoSymbol,               /* 0x13 -> Nothing */
        XK_space,               NoSymbol,               /* 0x14 */
        XK_Delete,              NoSymbol,               /* 0x15 */
        XK_henkan,              NoSymbol,               /* 0x16 -> Henkan Key */
        XK_kakutei,             NoSymbol,               /* 0x17 -> Kakutei Key */
        XK_F11,                 NoSymbol,               /* 0x18 */
        XK_F12,                 NoSymbol,               /* 0x19 */
        XK_F13,                 NoSymbol,               /* 0x1a */
        XK_F14,                 NoSymbol,               /* 0x1b */
        XK_Up,                  NoSymbol,               /* 0x1c */
        XK_Left,                NoSymbol,               /* 0x1d */
        XK_Right,               NoSymbol,               /* 0x1e */
        XK_Down,                NoSymbol,               /* 0x1f */
        NoSymbol,               NoSymbol,               /* 0x20 -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x21 -> Nothing */
        XK_1,                   XK_exclam,              /* 0x22 */
        XK_2,                   XK_quotedbl,            /* 0x23 */
        XK_3,                   XK_numbersign,          /* 0x24 */
        XK_4,                   XK_dollar,              /* 0x25 */
        XK_5,                   XK_percent,             /* 0x26 */
        XK_6,                   XK_ampersand,           /* 0x27 */
        XK_7,                   XK_quoteright,          /* 0x28 */
        XK_8,                   XK_parenleft,           /* 0x29 */
        XK_9,                   XK_parenright,          /* 0x2a */
        XK_0,                   NoSymbol,               /* 0x2b */
        XK_minus,               XK_equal,               /* 0x2c */
        XK_asciicircum,         XK_asciitilde,          /* 0x2d */
        XK_backslash,           XK_bar,                 /* 0x2e */
        NoSymbol,               NoSymbol,               /* 0x2f -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x30 -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x31 -> Nothing */
        XK_q,                   XK_Q,                   /* 0x32 */
        XK_w,                   XK_W,                   /* 0x33 */
        XK_e,                   XK_E,                   /* 0x34 */
        XK_r,                   XK_R,                   /* 0x35 */
        XK_t,                   XK_T,                   /* 0x36 */
        XK_y,                   XK_Y,                   /* 0x37 */
        XK_u,                   XK_U,                   /* 0x38 */
        XK_i,                   XK_I,                   /* 0x39 */
        XK_o,                   XK_O,                   /* 0x3a */
        XK_p,                   XK_P,                   /* 0x3b */
        XK_at,                  XK_quoteleft,           /* 0x3c */
        XK_bracketleft,         XK_braceleft,           /* 0x3d */
        NoSymbol,               NoSymbol,               /* 0x3e -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x3f -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x40 -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x41 -> Nothing */
        XK_a,                   XK_A,                   /* 0x42 */
        XK_s,                   XK_S,                   /* 0x43 */
        XK_d,                   XK_D,                   /* 0x44 */
        XK_f,                   XK_F,                   /* 0x45 */
        XK_g,                   XK_G,                   /* 0x46 */
        XK_h,                   XK_H,                   /* 0x47 */
        XK_j,                   XK_J,                   /* 0x48 */
        XK_k,                   XK_K,                   /* 0x49 */
        XK_l,                   XK_L,                   /* 0x4a */
        XK_semicolon,           XK_plus,                /* 0x4b */
        XK_colon,               XK_asterisk,            /* 0x4c */
        XK_bracketright,        XK_braceright,          /* 0x4d */
        NoSymbol,               NoSymbol,               /* 0x4e -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x4f -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x50 -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x51 -> Nothing */
        XK_z,                   XK_Z,                   /* 0x52 */
        XK_x,                   XK_X,                   /* 0x53 */
        XK_c,                   XK_C,                   /* 0x54 */
        XK_v,                   XK_V,                   /* 0x55 */
        XK_b,                   XK_B,                   /* 0x56 */
        XK_n,                   XK_N,                   /* 0x57 */
        XK_m,                   XK_M,                   /* 0x58 */
        XK_comma,               XK_less,                /* 0x59 */
        XK_period,              XK_greater,             /* 0x5a */
        XK_slash,               XK_question,            /* 0x5b */
        NoSymbol,               XK_underscore,          /* 0x5c */
        NoSymbol,               NoSymbol,               /* 0x5d -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x5e -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x5f -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x60 -> Nothing */
        XK_plus,                NoSymbol,               /* 0x61 */
        XK_minus,               NoSymbol,               /* 0x62 */
        XK_7,                   NoSymbol,               /* 0x63 */
        XK_8,                   NoSymbol,               /* 0x64 */
        XK_9,                   NoSymbol,               /* 0x65 */
        XK_4,                   NoSymbol,               /* 0x66 */
        XK_5,                   NoSymbol,               /* 0x67 */
        XK_6,                   NoSymbol,               /* 0x68 */
        XK_1,                   NoSymbol,               /* 0x69 */
        XK_2,                   NoSymbol,               /* 0x6a */
        XK_3,                   NoSymbol,               /* 0x6b */
        XK_0,                   NoSymbol,               /* 0x6c */
        XK_period,              NoSymbol,               /* 0x6d */
        XK_Return,              NoSymbol,               /* 0x6e */
        NoSymbol,               NoSymbol,               /* 0x6f */
        NoSymbol,               NoSymbol,               /* 0x70 */
        NoSymbol,               NoSymbol,               /* 0x71 */
        XK_F1,                  NoSymbol,               /* 0x72 */
        XK_F2,                  NoSymbol,               /* 0x73 */
        XK_F3,                  NoSymbol,               /* 0x74 */
        XK_F4,                  NoSymbol,               /* 0x75 */
        XK_F5,                  NoSymbol,               /* 0x76 */
        XK_F6,                  NoSymbol,               /* 0x77 */
        XK_F7,                  NoSymbol,               /* 0x78 */
        XK_F8,                  NoSymbol,               /* 0x79 */
        XK_F9,                  NoSymbol,               /* 0x7a */
        XK_F10,                 NoSymbol,               /* 0x7b */
        XK_asterisk,            XK_asterisk,            /* 0x7c */
        XK_slash,               XK_slash,               /* 0x7d */
        XK_equal,               XK_equal,               /* 0x7e */
        XK_comma,               XK_comma,               /* 0x7f */
/************************** Kana Code Table *****************************/
        XK_Tab,                 NoSymbol,               /* 0x80 */
        NoSymbol,               NoSymbol,               /* 0x81 */
        NoSymbol,               NoSymbol,               /* 0x82 */
        NoSymbol,               NoSymbol,               /* 0x83 */
        NoSymbol,               NoSymbol,               /* 0x84 */
        NoSymbol,               NoSymbol,               /* 0x85 */
        NoSymbol,               NoSymbol,               /* 0x86 */
        XK_Escape,              NoSymbol,               /* 0x87 */
        XK_BackSpace,           NoSymbol,               /* 0x88 */
        XK_Return,              NoSymbol,               /* 0x89 */
        NoSymbol,               NoSymbol,               /* 0x8a */
        XK_space,               NoSymbol,               /* 0x8b */
        XK_Delete,              NoSymbol,               /* 0x8c */
        XK_henkan,              NoSymbol,               /* 0x8d */ /* HENKAN */ 
        XK_kakutei,             NoSymbol,               /* 0x8e */ /* KAKUTEI */
        XK_F11,                 NoSymbol,               /* 0x8f */
        XK_F12,                 NoSymbol,               /* 0x90 */
        XK_F13,                 NoSymbol,               /* 0x91 */
        XK_F14,                 NoSymbol,               /* 0x92 */
        XK_Up,                  NoSymbol,               /* 0x93 */
        XK_Left,                NoSymbol,               /* 0x94 */
        XK_Right,               NoSymbol,               /* 0x95 */
        XK_Down,                NoSymbol,               /* 0x96 */
        NoSymbol,               NoSymbol,               /* 0x97 */
        NoSymbol,               NoSymbol,               /* 0x98 */
        XK_kana_NU,             NoSymbol,               /* 0x99 */
        XK_kana_HU,             NoSymbol,               /* 0x9a */
        XK_kana_A,              XK_kana_a,              /* 0x9b */
        XK_kana_U,              XK_kana_u,              /* 0x9c */
        XK_kana_E,              XK_kana_e,              /* 0x9d */
        XK_kana_O,              XK_kana_o,              /* 0x9e */
        XK_kana_YA,             XK_kana_ya,             /* 0x9f */
        XK_kana_YU,             XK_kana_yu,             /* 0xa0 */
        XK_kana_YO,             XK_kana_yo,             /* 0xa1 */
        XK_kana_WA,             XK_kana_WO,             /* 0xa2 */
        XK_kana_HO,             NoSymbol,               /* 0xa3 */
        XK_kana_HE,             NoSymbol,               /* 0xa4 */
        XK_prolongedsound,      NoSymbol,               /* 0xa5 */
        NoSymbol,               NoSymbol,               /* 0xa6 */
        NoSymbol,               NoSymbol,               /* 0xa7 */
        NoSymbol,               NoSymbol,               /* 0xa8 */
        XK_kana_TA,             NoSymbol,               /* 0xa9 */
        XK_kana_TE,             NoSymbol,               /* 0xaa */
        XK_kana_I,              XK_kana_i,              /* 0xab */
        XK_kana_SU,             NoSymbol,               /* 0xac */
        XK_kana_KA,             NoSymbol,               /* 0xad */
        XK_kana_N,              NoSymbol,               /* 0xae */
        XK_kana_NA,             NoSymbol,               /* 0xaf */
        XK_kana_NI,             NoSymbol,               /* 0xb0 */
        XK_kana_RA,             NoSymbol,               /* 0xb1 */
        XK_kana_SE,             NoSymbol,               /* 0xb2 */
        XK_voicedsound,         NoSymbol,               /* 0xb3 */
        XK_semivoicedsound,     XK_kana_openingbracket, /* 0xb4 */
        NoSymbol,               NoSymbol,               /* 0xb5 */
        NoSymbol,               NoSymbol,               /* 0xb6 */
        NoSymbol,               NoSymbol,               /* 0xb7 */
        NoSymbol,               NoSymbol,               /* 0xb8 */
        XK_kana_TI,             NoSymbol,               /* 0xb9 */
        XK_kana_TO,             NoSymbol,               /* 0xba */
        XK_kana_SHI,            NoSymbol,               /* 0xbb */
        XK_kana_HA,             NoSymbol,               /* 0xbc */
        XK_kana_KI,             NoSymbol,               /* 0xbd */
        XK_kana_KU,             NoSymbol,               /* 0xbe */
        XK_kana_MA,             NoSymbol,               /* 0xbf */
        XK_kana_NO,             NoSymbol,               /* 0xc0 */
        XK_kana_RI,             NoSymbol,               /* 0xc1 */
        XK_kana_RE,             NoSymbol,               /* 0xc2 */
        XK_kana_KE,             NoSymbol,               /* 0xc3 */
        XK_kana_MU,             XK_kana_closingbracket, /* 0xc4 */
        NoSymbol,               NoSymbol,               /* 0xc5 */
        NoSymbol,               NoSymbol,               /* 0xc6 */
        NoSymbol,               NoSymbol,               /* 0xc7 */
        NoSymbol,               NoSymbol,               /* 0xc8 */
        XK_kana_TU,             XK_kana_tu,             /* 0xc9 */
        XK_kana_SA,             NoSymbol,               /* 0xca */
        XK_kana_SO,             NoSymbol,               /* 0xcb */
        XK_kana_HI,             NoSymbol,               /* 0xcc */
        XK_kana_KO,             NoSymbol,               /* 0xcd */
        XK_kana_MI,             NoSymbol,               /* 0xce */
        XK_kana_MO,             NoSymbol,               /* 0xcf */
        XK_kana_NE,             XK_kana_comma,          /* 0xd0 */
        XK_kana_RU,             XK_kana_fullstop,       /* 0xd1 */
        XK_kana_ME,             XK_kana_middledot,      /* 0xd2 */
        XK_kana_RO,             NoSymbol,               /* 0xd3 */
        NoSymbol,               NoSymbol,               /* 0xd4 */
        NoSymbol,               NoSymbol,               /* 0xd5 */
        NoSymbol,               NoSymbol,               /* 0xd6 */
        NoSymbol,               NoSymbol,               /* 0xd7 */
        XK_plus,                NoSymbol,               /* 0xd8 */
        XK_minus,               NoSymbol,               /* 0xd9 */
        XK_7,                   NoSymbol,               /* 0xda */
        XK_8,                   NoSymbol,               /* 0xdb */
        XK_9,                   NoSymbol,               /* 0xdc */
        XK_4,                   NoSymbol,               /* 0xdd */
        XK_5,                   NoSymbol,               /* 0xde */
        XK_6,                   NoSymbol,               /* 0xdf */
        XK_1,                   NoSymbol,               /* 0xe0 */
        XK_2,                   NoSymbol,               /* 0xe1 */
        XK_3,                   NoSymbol,               /* 0xe2 */
        XK_0,                   NoSymbol,               /* 0xe3 */
        XK_period,              NoSymbol,               /* 0xe4 */
        XK_Return,              NoSymbol,               /* 0xe5 */
        NoSymbol,               NoSymbol,               /* 0xe6 */
        NoSymbol,               NoSymbol,               /* 0xe7 */
        NoSymbol,               NoSymbol,               /* 0xe8 */
        XK_F1,                  NoSymbol,               /* 0xe9 */
        XK_F2,                  NoSymbol,               /* 0xea */
        XK_F3,                  NoSymbol,               /* 0xeb */
        XK_F4,                  NoSymbol,               /* 0xec */
        XK_F5,                  NoSymbol,               /* 0xed */
        XK_F6,                  NoSymbol,               /* 0xee */
        XK_F7,                  NoSymbol,               /* 0xef */
        XK_F8,                  NoSymbol,               /* 0xf0 */
        XK_F9,                  NoSymbol,               /* 0xf1 */
        XK_F10,                 NoSymbol,               /* 0xf2 */
        XK_asterisk,            XK_asterisk,            /* 0xf3 */
        XK_slash,               XK_slash,               /* 0xf4 */
        XK_equal,               XK_equal,               /* 0xf5 */
        XK_comma,               XK_comma,               /* 0xf6  -> maxKeyCode */
};

#ifdef luna2 
static KeySym omronJisjisKeyMap[] = {        
/*      NormalShift             ShiftedShift            */
        XK_Tab,                 NoSymbol,               /* 0x9 -> min KeyCode */
        XK_Control_L,           NoSymbol,               /* 0xa */
        NoSymbol,               NoSymbol,               /* 0xb -> kana Key */
        XK_Shift_R,             NoSymbol,               /* 0xc */
        XK_Shift_L,             NoSymbol,               /* 0xd */
        XK_Caps_Lock,           NoSymbol,               /* 0xe */
        XK_Meta_L,              NoSymbol,               /* 0xf -> zenmen Key */
        XK_Escape,              NoSymbol,               /* 0x10 */
        XK_BackSpace,           NoSymbol,               /* 0x11 */
        XK_Return,              NoSymbol,               /* 0x12 */
        NoSymbol,               NoSymbol,               /* 0x13 -> Nothing */
        XK_space,               NoSymbol,               /* 0x14 */
        XK_Delete,              NoSymbol,               /* 0x15 */
        XK_henkan,              NoSymbol,               /* 0x16 -> Henkan Key */
        XK_kakutei,             NoSymbol,               /* 0x17 -> Kakutei Key */
        XK_F11,                 NoSymbol,               /* 0x18 */
        XK_F12,                 NoSymbol,               /* 0x19 */
        XK_F13,                 NoSymbol,               /* 0x1a */
        XK_F14,                 NoSymbol,               /* 0x1b */
        XK_Up,                  NoSymbol,               /* 0x1c */
        XK_Left,                NoSymbol,               /* 0x1d */
        XK_Right,               NoSymbol,               /* 0x1e */
        XK_Down,                NoSymbol,               /* 0x1f */
        osfXK_Insert,           NoSymbol,               /* 0x20 -> Insert */
        osfXK_Copy,             NoSymbol,               /* 0x21 -> Copy */
        XK_1,                   XK_exclam,              /* 0x22 */
        XK_2,                   XK_quotedbl,            /* 0x23 */
        XK_3,                   XK_numbersign,          /* 0x24 */
        XK_4,                   XK_dollar,              /* 0x25 */
        XK_5,                   XK_percent,             /* 0x26 */
        XK_6,                   XK_ampersand,           /* 0x27 */
        XK_7,                   XK_quoteright,          /* 0x28 */
        XK_8,                   XK_parenleft,           /* 0x29 */
        XK_9,                   XK_parenright,          /* 0x2a */
        XK_0,                   NoSymbol,               /* 0x2b */
        XK_minus,               XK_equal,               /* 0x2c */
        XK_asciicircum,         XK_asciitilde,          /* 0x2d */
        XK_backslash,           XK_bar,                 /* 0x2e */
        NoSymbol,               NoSymbol,               /* 0x2f -> Nothing */
        osfXK_Cut,              NoSymbol,               /* 0x30 -> Cut */
        osfXK_Paste,            NoSymbol,               /* 0x31 -> Paste */
        XK_q,                   XK_Q,                   /* 0x32 */
        XK_w,                   XK_W,                   /* 0x33 */
        XK_e,                   XK_E,                   /* 0x34 */
        XK_r,                   XK_R,                   /* 0x35 */
        XK_t,                   XK_T,                   /* 0x36 */
        XK_y,                   XK_Y,                   /* 0x37 */
        XK_u,                   XK_U,                   /* 0x38 */
        XK_i,                   XK_I,                   /* 0x39 */
        XK_o,                   XK_O,                   /* 0x3a */
        XK_p,                   XK_P,                   /* 0x3b */
        XK_at,                  XK_quoteleft,           /* 0x3c */
        XK_bracketleft,         XK_braceleft,           /* 0x3d */
        NoSymbol,               NoSymbol,               /* 0x3e -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x3f -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x40 -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x41 -> Nothing */
        XK_a,                   XK_A,                   /* 0x42 */
        XK_s,                   XK_S,                   /* 0x43 */
        XK_d,                   XK_D,                   /* 0x44 */
        XK_f,                   XK_F,                   /* 0x45 */
        XK_g,                   XK_G,                   /* 0x46 */
        XK_h,                   XK_H,                   /* 0x47 */
        XK_j,                   XK_J,                   /* 0x48 */
        XK_k,                   XK_K,                   /* 0x49 */
        XK_l,                   XK_L,                   /* 0x4a */
        XK_semicolon,           XK_plus,                /* 0x4b */
        XK_colon,               XK_asterisk,            /* 0x4c */
        XK_bracketright,        XK_braceright,          /* 0x4d */
        NoSymbol,               NoSymbol,               /* 0x4e -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x4f -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x50 -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x51 -> Nothing */
        XK_z,                   XK_Z,                   /* 0x52 */
        XK_x,                   XK_X,                   /* 0x53 */
        XK_c,                   XK_C,                   /* 0x54 */
        XK_v,                   XK_V,                   /* 0x55 */
        XK_b,                   XK_B,                   /* 0x56 */
        XK_n,                   XK_N,                   /* 0x57 */
        XK_m,                   XK_M,                   /* 0x58 */
        XK_comma,               XK_less,                /* 0x59 */
        XK_period,              XK_greater,             /* 0x5a */
        XK_slash,               XK_question,            /* 0x5b */
        NoSymbol,               XK_underscore,          /* 0x5c */
        NoSymbol,               NoSymbol,               /* 0x5d -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x5e -> Nothing */
        NoSymbol,               NoSymbol,               /* 0x5f -> Nothing */
        XK_Home,                NoSymbol,               /* 0x60 -> Home */
        XK_plus,                NoSymbol,               /* 0x61 */
        XK_minus,               NoSymbol,               /* 0x62 */
        XK_7,                   NoSymbol,               /* 0x63 */
        XK_8,                   NoSymbol,               /* 0x64 */
        XK_9,                   NoSymbol,               /* 0x65 */
        XK_4,                   NoSymbol,               /* 0x66 */
        XK_5,                   NoSymbol,               /* 0x67 */
        XK_6,                   NoSymbol,               /* 0x68 */
        XK_1,                   NoSymbol,               /* 0x69 */
        XK_2,                   NoSymbol,               /* 0x6a */
        XK_3,                   NoSymbol,               /* 0x6b */
        XK_0,                   NoSymbol,               /* 0x6c */
        XK_period,              NoSymbol,               /* 0x6d */
        XK_Return,              NoSymbol,               /* 0x6e */
        NoSymbol,               NoSymbol,               /* 0x6f */
        NoSymbol,               NoSymbol,               /* 0x70 */
        NoSymbol,               NoSymbol,               /* 0x71 */
        XK_F1,                  NoSymbol,               /* 0x72 */
        XK_F2,                  NoSymbol,               /* 0x73 */
        XK_F3,                  NoSymbol,               /* 0x74 */
        XK_F4,                  NoSymbol,               /* 0x75 */
        XK_F5,                  NoSymbol,               /* 0x76 */
        XK_F6,                  NoSymbol,               /* 0x77 */
        XK_F7,                  NoSymbol,               /* 0x78 */
        XK_F8,                  NoSymbol,               /* 0x79 */
        XK_F9,                  NoSymbol,               /* 0x7a */
        XK_F10,                 NoSymbol,               /* 0x7b */
        XK_asterisk,            XK_asterisk,            /* 0x7c */
        XK_slash,               XK_slash,               /* 0x7d */
        XK_equal,               XK_equal,               /* 0x7e */
        XK_comma,               XK_comma,               /* 0x7f */
/************************** Kana Code Table *****************************/
        XK_Tab,                 NoSymbol,               /* 0x80 */
        NoSymbol,               NoSymbol,               /* 0x81 */
        NoSymbol,               NoSymbol,               /* 0x82 */
        NoSymbol,               NoSymbol,               /* 0x83 */
        NoSymbol,               NoSymbol,               /* 0x84 */
        NoSymbol,               NoSymbol,               /* 0x85 */
        NoSymbol,               NoSymbol,               /* 0x86 */
        XK_Escape,              NoSymbol,               /* 0x87 */
        XK_BackSpace,           NoSymbol,               /* 0x88 */
        XK_Return,              NoSymbol,               /* 0x89 */
        NoSymbol,               NoSymbol,               /* 0x8a */
        XK_space,               NoSymbol,               /* 0x8b */
        XK_Delete,              NoSymbol,               /* 0x8c */
        XK_henkan,              NoSymbol,               /* 0x8d */ /* HENKAN */ 
        XK_kakutei,             NoSymbol,               /* 0x8e */ /* KAKUTEI */
        XK_F11,                 NoSymbol,               /* 0x8f */
        XK_F12,                 NoSymbol,               /* 0x90 */
        XK_F13,                 NoSymbol,               /* 0x91 */
        XK_F14,                 NoSymbol,               /* 0x92 */
        XK_Up,                  NoSymbol,               /* 0x93 */
        XK_Left,                NoSymbol,               /* 0x94 */
        XK_Right,               NoSymbol,               /* 0x95 */
        XK_Down,                NoSymbol,               /* 0x96 */
        NoSymbol,               NoSymbol,               /* 0x97 */
        NoSymbol,               NoSymbol,               /* 0x98 */
        XK_kana_NU,             NoSymbol,               /* 0x99 */
        XK_kana_HU,             NoSymbol,               /* 0x9a */
        XK_kana_A,              XK_kana_a,              /* 0x9b */
        XK_kana_U,              XK_kana_u,              /* 0x9c */
        XK_kana_E,              XK_kana_e,              /* 0x9d */
        XK_kana_O,              XK_kana_o,              /* 0x9e */
        XK_kana_YA,             XK_kana_ya,             /* 0x9f */
        XK_kana_YU,             XK_kana_yu,             /* 0xa0 */
        XK_kana_YO,             XK_kana_yo,             /* 0xa1 */
        XK_kana_WA,             XK_kana_WO,             /* 0xa2 */
        XK_kana_HO,             NoSymbol,               /* 0xa3 */
        XK_kana_HE,             NoSymbol,               /* 0xa4 */
        XK_prolongedsound,      NoSymbol,               /* 0xa5 */
        NoSymbol,               NoSymbol,               /* 0xa6 */
        NoSymbol,               NoSymbol,               /* 0xa7 */
        NoSymbol,               NoSymbol,               /* 0xa8 */
        XK_kana_TA,             NoSymbol,               /* 0xa9 */
        XK_kana_TE,             NoSymbol,               /* 0xaa */
        XK_kana_I,              XK_kana_i,              /* 0xab */
        XK_kana_SU,             NoSymbol,               /* 0xac */
        XK_kana_KA,             NoSymbol,               /* 0xad */
        XK_kana_N,              NoSymbol,               /* 0xae */
        XK_kana_NA,             NoSymbol,               /* 0xaf */
        XK_kana_NI,             NoSymbol,               /* 0xb0 */
        XK_kana_RA,             NoSymbol,               /* 0xb1 */
        XK_kana_SE,             NoSymbol,               /* 0xb2 */
        XK_voicedsound,         NoSymbol,               /* 0xb3 */
        XK_semivoicedsound,     XK_kana_openingbracket, /* 0xb4 */
        NoSymbol,               NoSymbol,               /* 0xb5 */
        NoSymbol,               NoSymbol,               /* 0xb6 */
        NoSymbol,               NoSymbol,               /* 0xb7 */
        NoSymbol,               NoSymbol,               /* 0xb8 */
        XK_kana_TI,             NoSymbol,               /* 0xb9 */
        XK_kana_TO,             NoSymbol,               /* 0xba */
        XK_kana_SHI,            NoSymbol,               /* 0xbb */
        XK_kana_HA,             NoSymbol,               /* 0xbc */
        XK_kana_KI,             NoSymbol,               /* 0xbd */
        XK_kana_KU,             NoSymbol,               /* 0xbe */
        XK_kana_MA,             NoSymbol,               /* 0xbf */
        XK_kana_NO,             NoSymbol,               /* 0xc0 */
        XK_kana_RI,             NoSymbol,               /* 0xc1 */
        XK_kana_RE,             NoSymbol,               /* 0xc2 */
        XK_kana_KE,             NoSymbol,               /* 0xc3 */
        XK_kana_MU,             XK_kana_closingbracket, /* 0xc4 */
        NoSymbol,               NoSymbol,               /* 0xc5 */
        NoSymbol,               NoSymbol,               /* 0xc6 */
        NoSymbol,               NoSymbol,               /* 0xc7 */
        NoSymbol,               NoSymbol,               /* 0xc8 */
        XK_kana_TU,             XK_kana_tu,             /* 0xc9 */
        XK_kana_SA,             NoSymbol,               /* 0xca */
        XK_kana_SO,             NoSymbol,               /* 0xcb */
        XK_kana_HI,             NoSymbol,               /* 0xcc */
        XK_kana_KO,             NoSymbol,               /* 0xcd */
        XK_kana_MI,             NoSymbol,               /* 0xce */
        XK_kana_MO,             NoSymbol,               /* 0xcf */
        XK_kana_NE,             XK_kana_comma,          /* 0xd0 */
        XK_kana_RU,             XK_kana_fullstop,       /* 0xd1 */
        XK_kana_ME,             XK_kana_middledot,      /* 0xd2 */
        XK_kana_RO,             NoSymbol,               /* 0xd3 */
        NoSymbol,               NoSymbol,               /* 0xd4 */
        NoSymbol,               NoSymbol,               /* 0xd5 */
        NoSymbol,               NoSymbol,               /* 0xd6 */
        NoSymbol,               NoSymbol,               /* 0xd7 */
        XK_plus,                NoSymbol,               /* 0xd8 */
        XK_minus,               NoSymbol,               /* 0xd9 */
        XK_7,                   NoSymbol,               /* 0xda */
        XK_8,                   NoSymbol,               /* 0xdb */
        XK_9,                   NoSymbol,               /* 0xdc */
        XK_4,                   NoSymbol,               /* 0xdd */
        XK_5,                   NoSymbol,               /* 0xde */
        XK_6,                   NoSymbol,               /* 0xdf */
        XK_1,                   NoSymbol,               /* 0xe0 */
        XK_2,                   NoSymbol,               /* 0xe1 */
        XK_3,                   NoSymbol,               /* 0xe2 */
        XK_0,                   NoSymbol,               /* 0xe3 */
        XK_period,              NoSymbol,               /* 0xe4 */
        XK_Return,              NoSymbol,               /* 0xe5 */
        NoSymbol,               NoSymbol,               /* 0xe6 */
        NoSymbol,               NoSymbol,               /* 0xe7 */
        NoSymbol,               NoSymbol,               /* 0xe8 */
        XK_F1,                  NoSymbol,               /* 0xe9 */
        XK_F2,                  NoSymbol,               /* 0xea */
        XK_F3,                  NoSymbol,               /* 0xeb */
        XK_F4,                  NoSymbol,               /* 0xec */
        XK_F5,                  NoSymbol,               /* 0xed */
        XK_F6,                  NoSymbol,               /* 0xee */
        XK_F7,                  NoSymbol,               /* 0xef */
        XK_F8,                  NoSymbol,               /* 0xf0 */
        XK_F9,                  NoSymbol,               /* 0xf1 */
        XK_F10,                 NoSymbol,               /* 0xf2 */
        XK_asterisk,            XK_asterisk,            /* 0xf3 */
        XK_slash,               XK_slash,               /* 0xf4 */
        XK_equal,               XK_equal,               /* 0xf5 */
        XK_comma,               XK_comma,               /* 0xf6  -> maxKeyCode */
};

static KeySym omronAsciijisKeyMap[] = {
/*      NormalShift             ShiftedShift            */
        XK_Tab,                 NoSymbol,               /* 0x9 -> min KeyCode */
        XK_Control_L,           NoSymbol,               /* 0xa */
        NoSymbol,               NoSymbol,               /* 0xb -> kana Key */
        XK_Shift_R,             NoSymbol,               /* 0xc */
        XK_Shift_L,             NoSymbol,               /* 0xd */
        XK_Caps_Lock,           NoSymbol,               /* 0xe */
        XK_Meta_L,              NoSymbol,               /* 0xf -> zenmen Key */
        XK_Escape,              NoSymbol,               /* 0x10 */
        XK_BackSpace,           NoSymbol,               /* 0x11 */
        XK_Return,              NoSymbol,               /* 0x12 */
        NoSymbol,               NoSymbol,               /* 0x13 -> Nothing */
        XK_space,               NoSymbol,               /* 0x14 */
        XK_Delete,              NoSymbol,               /* 0x15 */
        XK_henkan,              NoSymbol,               /* 0x16 -> Henkan Key */
        XK_kakutei,             NoSymbol,               /* 0x17 -> Kakutei Key */
        XK_F11,                 NoSymbol,               /* 0x18 */
        XK_F12,                 NoSymbol,               /* 0x19 */
        XK_F13,                 NoSymbol,               /* 0x1a */
        XK_F14,                 NoSymbol,               /* 0x1b */
        XK_Up,                  NoSymbol,               /* 0x1c */
        XK_Left,                NoSymbol,               /* 0x1d */
        XK_Right,               NoSymbol,               /* 0x1e */
        XK_Down,                NoSymbol,               /* 0x1f */
        osfXK_Insert,           NoSymbol,               /* 0x20 -> Insert */
        osfXK_Copy,             NoSymbol,               /* 0x21 -> Copy */
        XK_1,                   XK_exclam,              /* 0x22 */
        NoSymbol,               NoSymbol,               /* 0x23 */
        XK_3,                   XK_numbersign,          /* 0x24 */
        XK_4,                   XK_dollar,              /* 0x25 */
        XK_5,                   XK_percent,             /* 0x26 */
        NoSymbol,               NoSymbol,               /* 0x27 */
        NoSymbol,               NoSymbol,               /* 0x28 */
        NoSymbol,               NoSymbol,               /* 0x29 */
        NoSymbol,               NoSymbol,               /* 0x2a */
        NoSymbol,               NoSymbol,               /* 0x2b */
        NoSymbol,               NoSymbol,               /* 0x2c */
        NoSymbol,               NoSymbol,               /* 0x2d */
        XK_backslash,           XK_bar,                 /* 0x2e */
        XK_9,                   XK_parenleft,           /* 0x2f */
        osfXK_Cut,              NoSymbol,               /* 0x30 -> Cut */
        osfXK_Paste,            NoSymbol,               /* 0x31 -> Paste */
        XK_q,                   XK_Q,                   /* 0x32 */
        XK_w,                   XK_W,                   /* 0x33 */
        XK_e,                   XK_E,                   /* 0x34 */
        XK_r,                   XK_R,                   /* 0x35 */
        XK_t,                   XK_T,                   /* 0x36 */
        XK_y,                   XK_Y,                   /* 0x37 */
        XK_u,                   XK_U,                   /* 0x38 */
        XK_i,                   XK_I,                   /* 0x39 */
        XK_o,                   XK_O,                   /* 0x3a */
        XK_p,                   XK_P,                   /* 0x3b */
        NoSymbol,               NoSymbol,               /* 0x3c */
        NoSymbol,               NoSymbol,               /* 0x3d */
        XK_minus,               XK_underscore,          /* 0x3e */
        XK_equal,               XK_plus,                /* 0x3f */
        XK_2,                   XK_at,                  /* 0x40 */
        XK_6,                   XK_asciicircum,         /* 0x41 */
        XK_a,                   XK_A,                   /* 0x42 */
        XK_s,                   XK_S,                   /* 0x43 */
        XK_d,                   XK_D,                   /* 0x44 */
        XK_f,                   XK_F,                   /* 0x45 */
        XK_g,                   XK_G,                   /* 0x46 */
        XK_h,                   XK_H,                   /* 0x47 */
        XK_j,                   XK_J,                   /* 0x48 */
        XK_k,                   XK_K,                   /* 0x49 */
        XK_l,                   XK_L,                   /* 0x4a */
        NoSymbol,               NoSymbol,               /* 0x4b */
        NoSymbol,               NoSymbol,               /* 0x4c */
        NoSymbol,               NoSymbol,               /* 0x4d */
        XK_bracketleft,         XK_braceleft,           /* 0x4e */
        XK_bracketright,        XK_braceright,          /* 0x4f */
        XK_7,                   XK_ampersand,           /* 0x50 */
        XK_8,                   XK_asterisk,            /* 0x51 */
        XK_z,                   XK_Z,                   /* 0x52 */
        XK_x,                   XK_X,                   /* 0x53 */
        XK_c,                   XK_C,                   /* 0x54 */
        XK_v,                   XK_V,                   /* 0x55 */
        XK_b,                   XK_B,                   /* 0x56 */
        XK_n,                   XK_N,                   /* 0x57 */
        XK_m,                   XK_M,                   /* 0x58 */
        XK_comma,               XK_less,                /* 0x59 */
        XK_period,              XK_greater,             /* 0x5a */
        XK_slash,               XK_question,            /* 0x5b */
        NoSymbol,               XK_underscore,          /* 0x5c */
        XK_semicolon,           XK_colon,               /* 0x5d */
        XK_apostrophe,          XK_quotedbl,            /* 0x5e */
        XK_grave,               XK_asciitilde,          /* 0x5f */
        XK_Home,                NoSymbol,               /* 0x60 -> Home */
        XK_plus,                NoSymbol,               /* 0x61 */
        XK_minus,               NoSymbol,               /* 0x62 */
        XK_7,                   NoSymbol,               /* 0x63 */
        XK_8,                   NoSymbol,               /* 0x64 */
        XK_9,                   NoSymbol,               /* 0x65 */
        XK_4,                   NoSymbol,               /* 0x66 */
        XK_5,                   NoSymbol,               /* 0x67 */
        XK_6,                   NoSymbol,               /* 0x68 */
        XK_1,                   NoSymbol,               /* 0x69 */
        XK_2,                   NoSymbol,               /* 0x6a */
        XK_3,                   NoSymbol,               /* 0x6b */
        XK_0,                   NoSymbol,               /* 0x6c */
        XK_period,              NoSymbol,               /* 0x6d */
        XK_Return,              NoSymbol,               /* 0x6e */
        XK_0,                   XK_parenright,          /* 0x6f */
        NoSymbol,               NoSymbol,               /* 0x70 */
        NoSymbol,               NoSymbol,               /* 0x71 */
        XK_F1,                  NoSymbol,               /* 0x72 */
        XK_F2,                  NoSymbol,               /* 0x73 */
        XK_F3,                  NoSymbol,               /* 0x74 */
        XK_F4,                  NoSymbol,               /* 0x75 */
        XK_F5,                  NoSymbol,               /* 0x76 */
        XK_F6,                  NoSymbol,               /* 0x77 */
        XK_F7,                  NoSymbol,               /* 0x78 */
        XK_F8,                  NoSymbol,               /* 0x79 */
        XK_F9,                  NoSymbol,               /* 0x7a */
        XK_F10,                 NoSymbol,               /* 0x7b */
        XK_asterisk,            XK_asterisk,            /* 0x7c */
        XK_slash,               XK_slash,               /* 0x7d */
        XK_equal,               XK_equal,               /* 0x7e */
        XK_comma,               XK_comma,               /* 0x7f */
/************************** Kana Code Table *****************************/
        XK_Tab,                 NoSymbol,               /* 0x80 */
        NoSymbol,               NoSymbol,               /* 0x81 */
        NoSymbol,               NoSymbol,               /* 0x82 */
        NoSymbol,               NoSymbol,               /* 0x83 */
        NoSymbol,               NoSymbol,               /* 0x84 */
        NoSymbol,               NoSymbol,               /* 0x85 */
        NoSymbol,               NoSymbol,               /* 0x86 */
        XK_Escape,              NoSymbol,               /* 0x87 */
        XK_BackSpace,           NoSymbol,               /* 0x88 */
        XK_Return,              NoSymbol,               /* 0x89 */
        NoSymbol,               NoSymbol,               /* 0x8a */
        XK_space,               NoSymbol,               /* 0x8b */
        XK_Delete,              NoSymbol,               /* 0x8c */
        XK_henkan,              NoSymbol,               /* 0x8d */ /* HENKAN */ 
        XK_kakutei,             NoSymbol,               /* 0x8e */ /* KAKUTEI */
        XK_F11,                 NoSymbol,               /* 0x8f */
        XK_F12,                 NoSymbol,               /* 0x90 */
        XK_F13,                 NoSymbol,               /* 0x91 */
        XK_F14,                 NoSymbol,               /* 0x92 */
        XK_Up,                  NoSymbol,               /* 0x93 */
        XK_Left,                NoSymbol,               /* 0x94 */
        XK_Right,               NoSymbol,               /* 0x95 */
        XK_Down,                NoSymbol,               /* 0x96 */
        NoSymbol,               NoSymbol,               /* 0x97 */
        NoSymbol,               NoSymbol,               /* 0x98 */
        XK_kana_NU,             NoSymbol,               /* 0x99 */
        NoSymbol,               NoSymbol,               /* 0x9a */
        XK_kana_A,              XK_kana_a,              /* 0x9b */
        XK_kana_U,              XK_kana_u,              /* 0x9c */
        XK_kana_E,              XK_kana_e,              /* 0x9d */
        NoSymbol,               NoSymbol,               /* 0x9e */
        NoSymbol,               NoSymbol,               /* 0x9f */
        NoSymbol,               NoSymbol,               /* 0xa0 */
        NoSymbol,               NoSymbol,               /* 0xa1 */
        NoSymbol,               NoSymbol,               /* 0xa2 */
        NoSymbol,               NoSymbol,               /* 0xa3 */
        NoSymbol,               NoSymbol,               /* 0xa4 */
        XK_prolongedsound,      NoSymbol,               /* 0xa5 */
        XK_kana_YO,             XK_kana_yo,             /* 0xa6 */
        NoSymbol,               NoSymbol,               /* 0xa7 */
        NoSymbol,               NoSymbol,               /* 0xa8 */
        XK_kana_TA,             NoSymbol,               /* 0xa9 */
        XK_kana_TE,             NoSymbol,               /* 0xaa */
        XK_kana_I,              XK_kana_i,              /* 0xab */
        XK_kana_SU,             NoSymbol,               /* 0xac */
        XK_kana_KA,             NoSymbol,               /* 0xad */
        XK_kana_N,              NoSymbol,               /* 0xae */
        XK_kana_NA,             NoSymbol,               /* 0xaf */
        XK_kana_NI,             NoSymbol,               /* 0xb0 */
        XK_kana_RA,             NoSymbol,               /* 0xb1 */
        XK_kana_SE,             NoSymbol,               /* 0xb2 */
        NoSymbol,               NoSymbol,               /* 0xb3 */
        NoSymbol,               NoSymbol,               /* 0xb4 */
        XK_kana_HO,             NoSymbol,               /* 0xb5 */
        XK_kana_HE,             NoSymbol,               /* 0xb6 */
        XK_kana_FU,             NoSymbol,               /* 0xb7 */
        XK_kana_O,              XK_kana_o,              /* 0xb8 */
        XK_kana_TI,             NoSymbol,               /* 0xb9 */
        XK_kana_TO,             NoSymbol,               /* 0xba */
        XK_kana_SHI,            NoSymbol,               /* 0xbb */
        XK_kana_HA,             NoSymbol,               /* 0xbc */
        XK_kana_KI,             NoSymbol,               /* 0xbd */
        XK_kana_KU,             NoSymbol,               /* 0xbe */
        XK_kana_MA,             NoSymbol,               /* 0xbf */
        XK_kana_NO,             NoSymbol,               /* 0xc0 */
        XK_kana_RI,             NoSymbol,               /* 0xc1 */
        NoSymbol,               NoSymbol,               /* 0xc2 */
        NoSymbol,               NoSymbol,               /* 0xc3 */
        NoSymbol,               NoSymbol,               /* 0xc4 */
        XK_voicedsound,         XK_kana_openingbracket, /* 0xc5 */
        XK_semivoicedsound,     XK_kana_closingbracket, /* 0xc6 */
        XK_kana_YA,             XK_kana_ya,             /* 0xc7 */
        XK_kana_YU,             XK_kana_yu,             /* 0xc8 */
        XK_kana_TU,             XK_kana_tu,             /* 0xc9 */
        XK_kana_SA,             NoSymbol,               /* 0xca */
        XK_kana_SO,             NoSymbol,               /* 0xcb */
        XK_kana_HI,             NoSymbol,               /* 0xcc */
        XK_kana_KO,             NoSymbol,               /* 0xcd */
        XK_kana_MI,             NoSymbol,               /* 0xce */
        XK_kana_MO,             NoSymbol,               /* 0xcf */
        XK_kana_NE,             XK_kana_comma,          /* 0xd0 */
        XK_kana_RU,             XK_kana_fullstop,       /* 0xd1 */
        XK_kana_ME,             XK_kana_middledot,      /* 0xd2 */
        XK_kana_RO,             NoSymbol,               /* 0xd3 */
        XK_kana_RE,             NoSymbol,               /* 0xd4 */
        XK_kana_KE,             NoSymbol,               /* 0xd5 */
        XK_kana_MU,             NoSymbol,               /* 0xd6 */
        NoSymbol,               NoSymbol,               /* 0xd7 */
        XK_plus,                NoSymbol,               /* 0xd8 */
        XK_minus,               NoSymbol,               /* 0xd9 */
        XK_7,                   NoSymbol,               /* 0xda */
        XK_8,                   NoSymbol,               /* 0xdb */
        XK_9,                   NoSymbol,               /* 0xdc */
        XK_4,                   NoSymbol,               /* 0xdd */
        XK_5,                   NoSymbol,               /* 0xde */
        XK_6,                   NoSymbol,               /* 0xdf */
        XK_1,                   NoSymbol,               /* 0xe0 */
        XK_2,                   NoSymbol,               /* 0xe1 */
        XK_3,                   NoSymbol,               /* 0xe2 */
        XK_0,                   NoSymbol,               /* 0xe3 */
        XK_period,              NoSymbol,               /* 0xe4 */
        XK_Return,              NoSymbol,               /* 0xe5 */
        XK_kana_WA,             XK_kana_WO,             /* 0xe6 */
        NoSymbol,               NoSymbol,               /* 0xe7 */
        NoSymbol,               NoSymbol,               /* 0xe8 */
        XK_F1,                  NoSymbol,               /* 0xe9 */
        XK_F2,                  NoSymbol,               /* 0xea */
        XK_F3,                  NoSymbol,               /* 0xeb */
        XK_F4,                  NoSymbol,               /* 0xec */
        XK_F5,                  NoSymbol,               /* 0xed */
        XK_F6,                  NoSymbol,               /* 0xee */
        XK_F7,                  NoSymbol,               /* 0xef */
        XK_F8,                  NoSymbol,               /* 0xf0 */
        XK_F9,                  NoSymbol,               /* 0xf1 */
        XK_F10,                 NoSymbol,               /* 0xf2 */
        XK_asterisk,            XK_asterisk,            /* 0xf3 */
        XK_slash,               XK_slash,               /* 0xf4 */
        XK_equal,               XK_equal,               /* 0xf5 */
        XK_comma,               XK_comma,               /* 0xf6  -> maxKeyCode */
};

static KeySym omronAsciiKeyMap[] = {
/*      NormalShift             ShiftedShift            */
        XK_Tab,                 NoSymbol,               /* 0x9 -> min KeyCode */
        XK_Control_L,           NoSymbol,               /* 0xa */
        NoSymbol,               NoSymbol,               /* 0xb -> kana Key */
        XK_Shift_R,             NoSymbol,               /* 0xc */
        XK_Shift_L,             NoSymbol,               /* 0xd */
        XK_Caps_Lock,           NoSymbol,               /* 0xe */
        XK_Meta_L,              NoSymbol,               /* 0xf -> zenmen Key */
        XK_Escape,              NoSymbol,               /* 0x10 */
        XK_BackSpace,           NoSymbol,               /* 0x11 */
        XK_Return,              NoSymbol,               /* 0x12 */
        NoSymbol,               NoSymbol,               /* 0x13 -> Nothing */
        XK_space,               NoSymbol,               /* 0x14 */
        XK_Delete,              NoSymbol,               /* 0x15 */
        NoSymbol,               NoSymbol,               /* 0x16 -> Henkan Key */
        NoSymbol,               NoSymbol,               /* 0x17 -> Kakutei Key */
        XK_F11,                 NoSymbol,               /* 0x18 */
        XK_F12,                 NoSymbol,               /* 0x19 */
        XK_F13,                 NoSymbol,               /* 0x1a */
        XK_F14,                 NoSymbol,               /* 0x1b */
        XK_Up,                  NoSymbol,               /* 0x1c */
        XK_Left,                NoSymbol,               /* 0x1d */
        XK_Right,               NoSymbol,               /* 0x1e */
        XK_Down,                NoSymbol,               /* 0x1f */
        NoSymbol,               NoSymbol,               /* 0x20 */
        NoSymbol,               NoSymbol,               /* 0x21 */
        XK_1,                   XK_exclam,              /* 0x22 */
        NoSymbol,               NoSymbol,               /* 0x23 */
        XK_3,                   XK_numbersign,          /* 0x24 */
        XK_4,                   XK_dollar,              /* 0x25 */
        XK_5,                   XK_percent,             /* 0x26 */
        NoSymbol,               NoSymbol,               /* 0x27 */
        NoSymbol,               NoSymbol,               /* 0x28 */
        NoSymbol,               NoSymbol,               /* 0x29 */
        NoSymbol,               NoSymbol,               /* 0x2a */
        NoSymbol,               NoSymbol,               /* 0x2b */
        NoSymbol,               NoSymbol,               /* 0x2c */
        NoSymbol,               NoSymbol,               /* 0x2d */
        XK_backslash,           XK_bar,                 /* 0x2e */
        XK_9,                   XK_parenleft,           /* 0x2f */
        NoSymbol,               NoSymbol,               /* 0x30 */
        NoSymbol,               NoSymbol,               /* 0x31 */
        XK_q,                   XK_Q,                   /* 0x32 */
        XK_w,                   XK_W,                   /* 0x33 */
        XK_e,                   XK_E,                   /* 0x34 */
        XK_r,                   XK_R,                   /* 0x35 */
        XK_t,                   XK_T,                   /* 0x36 */
        XK_y,                   XK_Y,                   /* 0x37 */
        XK_u,                   XK_U,                   /* 0x38 */
        XK_i,                   XK_I,                   /* 0x39 */
        XK_o,                   XK_O,                   /* 0x3a */
        XK_p,                   XK_P,                   /* 0x3b */
        NoSymbol,               NoSymbol,               /* 0x3c */
        NoSymbol,               NoSymbol,               /* 0x3d */
        XK_minus,               XK_underscore,          /* 0x3e */
        XK_equal,               XK_plus,                /* 0x3f */
        XK_2,                   XK_at,                  /* 0x40 */
        XK_6,                   XK_asciicircum,         /* 0x41 */
        XK_a,                   XK_A,                   /* 0x42 */
        XK_s,                   XK_S,                   /* 0x43 */
        XK_d,                   XK_D,                   /* 0x44 */
        XK_f,                   XK_F,                   /* 0x45 */
        XK_g,                   XK_G,                   /* 0x46 */
        XK_h,                   XK_H,                   /* 0x47 */
        XK_j,                   XK_J,                   /* 0x48 */
        XK_k,                   XK_K,                   /* 0x49 */
        XK_l,                   XK_L,                   /* 0x4a */
        NoSymbol,               NoSymbol,               /* 0x4b */
        NoSymbol,               NoSymbol,               /* 0x4c */
        NoSymbol,               NoSymbol,               /* 0x4d */
        XK_bracketleft,         XK_braceleft,           /* 0x4e */
        XK_bracketright,        XK_braceright,          /* 0x4f */
        XK_7,                   XK_ampersand,           /* 0x50 */
        XK_8,                   XK_asterisk,            /* 0x51 */
        XK_z,                   XK_Z,                   /* 0x52 */
        XK_x,                   XK_X,                   /* 0x53 */
        XK_c,                   XK_C,                   /* 0x54 */
        XK_v,                   XK_V,                   /* 0x55 */
        XK_b,                   XK_B,                   /* 0x56 */
        XK_n,                   XK_N,                   /* 0x57 */
        XK_m,                   XK_M,                   /* 0x58 */
        XK_comma,               XK_less,                /* 0x59 */
        XK_period,              XK_greater,             /* 0x5a */
        XK_slash,               XK_question,            /* 0x5b */
        NoSymbol,               NoSymbol,               /* 0x5c XXX */
        XK_semicolon,           XK_colon,               /* 0x5d */
        XK_apostrophe,          XK_quotedbl,            /* 0x5e */
        XK_grave,               XK_asciitilde,          /* 0x5f */
        XK_Home,                NoSymbol,               /* 0x60 -> Home */
        NoSymbol,               NoSymbol,               /* 0x61 */
        NoSymbol,               NoSymbol,               /* 0x62 */
        NoSymbol,               NoSymbol,               /* 0x63 */
        NoSymbol,               NoSymbol,               /* 0x64 */
        NoSymbol,               NoSymbol,               /* 0x65 */
        NoSymbol,               NoSymbol,               /* 0x66 */
        NoSymbol,               NoSymbol,               /* 0x67 */
        NoSymbol,               NoSymbol,               /* 0x68 */
        NoSymbol,               NoSymbol,               /* 0x69 */
        NoSymbol,               NoSymbol,               /* 0x6a */
        NoSymbol,               NoSymbol,               /* 0x6b */
        NoSymbol,               NoSymbol,               /* 0x6c */
        NoSymbol,               NoSymbol,               /* 0x6d */
        NoSymbol,               NoSymbol,               /* 0x6e */
        XK_0,                   XK_parenright,          /* 0x6f */
        NoSymbol,               NoSymbol,               /* 0x70 */
        NoSymbol,               NoSymbol,               /* 0x71 */
        XK_F1,                  NoSymbol,               /* 0x72 */
        XK_F2,                  NoSymbol,               /* 0x73 */
        XK_F3,                  NoSymbol,               /* 0x74 */
        XK_F4,                  NoSymbol,               /* 0x75 */
        XK_F5,                  NoSymbol,               /* 0x76 */
        XK_F6,                  NoSymbol,               /* 0x77 */
        XK_F7,                  NoSymbol,               /* 0x78 */
        XK_F8,                  NoSymbol,               /* 0x79 */
        XK_F9,                  NoSymbol,               /* 0x7a */
        XK_F10,                 NoSymbol,               /* 0x7b */
};
#endif /* luna2 */
#else
static KeySym omronDefKeyMap[] = {
        XK_Tab,         NoSymbol,     XK_Tab,         NoSymbol,     /* 0x9 -> min KeyCode */
        XK_Control_L,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0xa */
        XK_Mode_switch, NoSymbol,     NoSymbol,       NoSymbol,     /* 0xb -> kana Key */
        XK_Shift_R,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0xc */
        XK_Shift_L,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0xd */
        XK_Caps_Lock,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0xe */
        XK_Meta_L,      NoSymbol,     NoSymbol,       NoSymbol,     /* 0xf -> zenmen Key */
        XK_Escape,      NoSymbol,     XK_Escape,      NoSymbol,     /* 0x10 */
        XK_BackSpace,   NoSymbol,     XK_BackSpace,   NoSymbol,     /* 0x11 */
        XK_Return,      NoSymbol,     XK_Return,      NoSymbol,     /* 0x12 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x13 -> Nothing */
        XK_space,       NoSymbol,     XK_space,       NoSymbol,     /* 0x14 */
        XK_Delete,      NoSymbol,     XK_Delete,      NoSymbol,     /* 0x15 */
        XK_henkan,      NoSymbol,     XK_henkan,      NoSymbol,     /* 0x16 -> Henkan Key */
        XK_kakutei,     NoSymbol,     XK_kakutei,     NoSymbol,     /* 0x17 -> Kakutei Key */
        XK_F11,         NoSymbol,     XK_F11,         NoSymbol,     /* 0x18 */
        XK_F12,         NoSymbol,     XK_F12,         NoSymbol,     /* 0x19 */
        XK_F13,         NoSymbol,     XK_F13,         NoSymbol,     /* 0x1a */
        XK_F14,         NoSymbol,     XK_F14,         NoSymbol,     /* 0x1b */
        XK_Up,          NoSymbol,     XK_Up,          NoSymbol,     /* 0x1c */
        XK_Left,        NoSymbol,     XK_Left,        NoSymbol,     /* 0x1d */
        XK_Right,       NoSymbol,     XK_Right,       NoSymbol,     /* 0x1e */
        XK_Down,        NoSymbol,     XK_Down,        NoSymbol,     /* 0x1f */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x20 -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x21 -> Nothing */
        XK_1,           XK_exclam,    XK_kana_NU,     NoSymbol,     /* 0x22 */
        XK_2,           XK_quotedbl,  XK_kana_HU,     NoSymbol,     /* 0x23 */
        XK_3,           XK_numbersign,XK_kana_A,      XK_kana_a,    /* 0x24 */
        XK_4,           XK_dollar,    XK_kana_U,      XK_kana_u,    /* 0x25 */
        XK_5,           XK_percent,   XK_kana_E,      XK_kana_e,    /* 0x26 */
        XK_6,           XK_ampersand, XK_kana_O,      XK_kana_o,    /* 0x27 */
        XK_7,           XK_quoteright,XK_kana_YA,     XK_kana_ya,   /* 0x28 */
        XK_8,           XK_parenleft, XK_kana_YU,     XK_kana_yu,   /* 0x29 */
        XK_9,           XK_parenright,XK_kana_YO,     XK_kana_yo,   /* 0x2a */
        XK_0,           NoSymbol,     XK_kana_WA,     XK_kana_WO,   /* 0x2b */
        XK_minus,       XK_equal,     XK_kana_HO,     NoSymbol,     /* 0x2c */
        XK_asciicircum, XK_asciitilde,XK_kana_HE,     NoSymbol,     /* 0x2d */
        XK_backslash,   XK_bar,       XK_prolongedsound, NoSymbol,  /* 0x2e */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x2f -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x30 -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x31 -> Nothing */
        XK_q,           XK_Q,         XK_kana_TA,     NoSymbol,     /* 0x32 */
        XK_w,           XK_W,         XK_kana_TE,     NoSymbol,     /* 0x33 */
        XK_e,           XK_E,         XK_kana_I,      XK_kana_i,    /* 0x34 */
        XK_r,           XK_R,         XK_kana_SU,     NoSymbol,     /* 0x35 */
        XK_t,           XK_T,         XK_kana_KA,     NoSymbol,     /* 0x36 */
        XK_y,           XK_Y,         XK_kana_N,      NoSymbol,     /* 0x37 */
        XK_u,           XK_U,         XK_kana_NA,     NoSymbol,     /* 0x38 */
        XK_i,           XK_I,         XK_kana_NI,     NoSymbol,     /* 0x39 */
        XK_o,           XK_O,         XK_kana_RA,     NoSymbol,     /* 0x3a */
        XK_p,           XK_P,         XK_kana_SE,     NoSymbol,     /* 0x3b */
        XK_at,          XK_quoteleft, XK_voicedsound, NoSymbol,     /* 0x3c */
        XK_bracketleft, XK_braceleft, XK_semivoicedsound, XK_kana_openingbracket, /* 0x3d */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x3e -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x3f -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x40 -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x41 -> Nothing */
        XK_a,           XK_A,         XK_kana_TI,     NoSymbol,     /* 0x42 */
        XK_s,           XK_S,         XK_kana_TO,     NoSymbol,     /* 0x43 */
        XK_d,           XK_D,         XK_kana_SHI,    NoSymbol,     /* 0x44 */
        XK_f,           XK_F,         XK_kana_HA,     NoSymbol,     /* 0x45 */
        XK_g,           XK_G,         XK_kana_KI,     NoSymbol,     /* 0x46 */
        XK_h,           XK_H,         XK_kana_KU,     NoSymbol,     /* 0x47 */
        XK_j,           XK_J,         XK_kana_MA,     NoSymbol,     /* 0x48 */
        XK_k,           XK_K,         XK_kana_NO,     NoSymbol,     /* 0x49 */
        XK_l,           XK_L,         XK_kana_RI,     NoSymbol,     /* 0x4a */
        XK_semicolon,   XK_plus,      XK_kana_RE,     NoSymbol,     /* 0x4b */
        XK_colon,       XK_asterisk,  XK_kana_KE,     NoSymbol,     /* 0x4c */
        XK_bracketright,XK_braceright,XK_kana_MU,     XK_kana_closingbracket,  /* 0x4d */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4e -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4f -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x50 -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x51 -> Nothing */
        XK_z,           XK_Z,         XK_kana_TU,     XK_kana_tu,   /* 0x52 */
        XK_x,           XK_X,         XK_kana_SA,     NoSymbol,     /* 0x53 */
        XK_c,           XK_C,         XK_kana_SO,     NoSymbol,     /* 0x54 */
        XK_v,           XK_V,         XK_kana_HI,     NoSymbol,     /* 0x55 */
        XK_b,           XK_B,         XK_kana_KO,     NoSymbol,     /* 0x56 */
        XK_n,           XK_N,         XK_kana_MI,     NoSymbol,     /* 0x57 */
        XK_m,           XK_M,         XK_kana_MO,     NoSymbol,     /* 0x58 */
        XK_comma,       XK_less,      XK_kana_NE,     XK_kana_comma,     /* 0x59 */
        XK_period,      XK_greater,   XK_kana_RU,     XK_kana_fullstop,  /* 0x5a */
        XK_slash,       XK_question,  XK_kana_ME,     XK_kana_middledot, /* 0x5b */
        NoSymbol,       XK_underscore,XK_kana_RO,     NoSymbol,     /* 0x5c */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x5d -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x5e -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x5f -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x60 -> Nothing */
        XK_plus,        NoSymbol,     XK_plus,        NoSymbol,     /* 0x61 */
        XK_minus,       NoSymbol,     XK_minus,       NoSymbol,     /* 0x62 */
        XK_7,           NoSymbol,     XK_7,           NoSymbol,     /* 0x63 */
        XK_8,           NoSymbol,     XK_8,           NoSymbol,     /* 0x64 */
        XK_9,           NoSymbol,     XK_9,           NoSymbol,     /* 0x65 */
        XK_4,           NoSymbol,     XK_4,           NoSymbol,     /* 0x66 */
        XK_5,           NoSymbol,     XK_5,           NoSymbol,     /* 0x67 */
        XK_6,           NoSymbol,     XK_6,           NoSymbol,     /* 0x68 */
        XK_1,           NoSymbol,     XK_1,           NoSymbol,     /* 0x69 */
        XK_2,           NoSymbol,     XK_2,           NoSymbol,     /* 0x6a */
        XK_3,           NoSymbol,     XK_3,           NoSymbol,     /* 0x6b */
        XK_0,           NoSymbol,     XK_0,           NoSymbol,     /* 0x6c */
        XK_period,      NoSymbol,     XK_period,      NoSymbol,     /* 0x6d */
        XK_Return,      NoSymbol,     XK_Return,      NoSymbol,     /* 0x6e */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x6f */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x70 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x71 */
        XK_F1,          NoSymbol,     XK_F1,          NoSymbol,     /* 0x72 */
        XK_F2,          NoSymbol,     XK_F2,          NoSymbol,     /* 0x73 */
        XK_F3,          NoSymbol,     XK_F3,          NoSymbol,     /* 0x74 */
        XK_F4,          NoSymbol,     XK_F4,          NoSymbol,     /* 0x75 */
        XK_F5,          NoSymbol,     XK_F5,          NoSymbol,     /* 0x76 */
        XK_F6,          NoSymbol,     XK_F6,          NoSymbol,     /* 0x77 */
        XK_F7,          NoSymbol,     XK_F7,          NoSymbol,     /* 0x78 */
        XK_F8,          NoSymbol,     XK_F8,          NoSymbol,     /* 0x79 */
        XK_F9,          NoSymbol,     XK_F9,          NoSymbol,     /* 0x7a */
        XK_F10,         NoSymbol,     XK_F10,         NoSymbol,     /* 0x7b */
        XK_asterisk,    XK_asterisk,  XK_asterisk,    XK_asterisk,  /* 0x7c */
        XK_slash,       XK_slash,     XK_slash,       XK_slash,     /* 0x7d */
        XK_equal,       XK_equal,     XK_equal,       XK_equal,     /* 0x7e */
        XK_comma,       XK_comma,     XK_comma,       XK_comma,     /* 0x7f */
};

#ifdef luna2 
static KeySym omronJisjisKeyMap[] = {
/*      NormalShift     ShiftedShift                */
        XK_Tab,         NoSymbol,     XK_Tab,         NoSymbol,     /* 0x9 -> min KeyCode */
        XK_Control_L,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0xa */
        XK_Mode_switch, NoSymbol,     NoSymbol,       NoSymbol,     /* 0xb -> kana Key */
        XK_Shift_R,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0xc */
        XK_Shift_L,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0xd */
        XK_Caps_Lock,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0xe */
        XK_Meta_L,      NoSymbol,     NoSymbol,       NoSymbol,     /* 0xf -> zenmen Key */
        XK_Escape,      NoSymbol,     XK_Escape,      NoSymbol,     /* 0x10 */
        XK_BackSpace,   NoSymbol,     XK_BackSpace,   NoSymbol,     /* 0x11 */
        XK_Return,      NoSymbol,     XK_Return,      NoSymbol,     /* 0x12 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x13 -> Nothing */
        XK_space,       NoSymbol,     XK_space,       NoSymbol,     /* 0x14 */
        XK_Delete,      NoSymbol,     XK_Delete,      NoSymbol,     /* 0x15 */
        XK_henkan,      NoSymbol,     XK_henkan,      NoSymbol,     /* 0x16 -> Henkan Key */
        XK_kakutei,     NoSymbol,     XK_kakutei,     NoSymbol,     /* 0x17 -> Kakutei Key */
        XK_F11,         NoSymbol,     XK_F11,         NoSymbol,     /* 0x18 */
        XK_F12,         NoSymbol,     XK_F12,         NoSymbol,     /* 0x19 */
        XK_F13,         NoSymbol,     XK_F13,         NoSymbol,     /* 0x1a */
        XK_F14,         NoSymbol,     XK_F14,         NoSymbol,     /* 0x1b */
        XK_Up,          NoSymbol,     XK_Up,          NoSymbol,     /* 0x1c */
        XK_Left,        NoSymbol,     XK_Left,        NoSymbol,     /* 0x1d */
        XK_Right,       NoSymbol,     XK_Right,       NoSymbol,     /* 0x1e */
        XK_Down,        NoSymbol,     XK_Down,        NoSymbol,     /* 0x1f */
        osfXK_Insert,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0x20 -> Insert */
        osfXK_Copy,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0x21 -> Copy */
        XK_1,           XK_exclam,    XK_kana_NU,     NoSymbol,     /* 0x22 */
        XK_2,           XK_quotedbl,  XK_kana_HU,     NoSymbol,     /* 0x23 */
        XK_3,           XK_numbersign,XK_kana_A,      XK_kana_a,    /* 0x24 */
        XK_4,           XK_dollar,    XK_kana_U,      XK_kana_u,    /* 0x25 */
        XK_5,           XK_percent,   XK_kana_E,      XK_kana_e,    /* 0x26 */
        XK_6,           XK_ampersand, XK_kana_O,      XK_kana_o,    /* 0x27 */
        XK_7,           XK_quoteright,XK_kana_YA,     XK_kana_ya,   /* 0x28 */
        XK_8,           XK_parenleft, XK_kana_YU,     XK_kana_yu,   /* 0x29 */
        XK_9,           XK_parenright,XK_kana_YO,     XK_kana_yo,   /* 0x2a */
        XK_0,           NoSymbol,     XK_kana_WA,     XK_kana_WO,   /* 0x2b */
        XK_minus,       XK_equal,     XK_kana_HO,     NoSymbol,     /* 0x2c */
        XK_asciicircum, XK_asciitilde,XK_kana_HE,     NoSymbol,     /* 0x2d */
        XK_backslash,   XK_bar,       XK_prolongedsound, NoSymbol,  /* 0x2e */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x2f -> Nothing */
        osfXK_Cut,      NoSymbol,     NoSymbol,       NoSymbol,     /* 0x30 -> Cut */
        osfXK_Paste,    NoSymbol,     NoSymbol,       NoSymbol,     /* 0x31 -> Paste */
        XK_q,           XK_Q,         XK_kana_TA,     NoSymbol,     /* 0x32 */
        XK_w,           XK_W,         XK_kana_TE,     NoSymbol,     /* 0x33 */
        XK_e,           XK_E,         XK_kana_I,      XK_kana_i,    /* 0x34 */
        XK_r,           XK_R,         XK_kana_SU,     NoSymbol,     /* 0x35 */
        XK_t,           XK_T,         XK_kana_KA,     NoSymbol,     /* 0x36 */
        XK_y,           XK_Y,         XK_kana_N,      NoSymbol,     /* 0x37 */
        XK_u,           XK_U,         XK_kana_NA,     NoSymbol,     /* 0x38 */
        XK_i,           XK_I,         XK_kana_NI,     NoSymbol,     /* 0x39 */
        XK_o,           XK_O,         XK_kana_RA,     NoSymbol,     /* 0x3a */
        XK_p,           XK_P,         XK_kana_SE,     NoSymbol,     /* 0x3b */
        XK_at,          XK_quoteleft, XK_voicedsound, NoSymbol,     /* 0x3c */
        XK_bracketleft, XK_braceleft, XK_semivoicedsound, XK_kana_openingbracket, /* 0x3d */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x3e -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x3f -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x40 -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x41 -> Nothing */
        XK_a,           XK_A,         XK_kana_TI,     NoSymbol,     /* 0x42 */
        XK_s,           XK_S,         XK_kana_TO,     NoSymbol,     /* 0x43 */
        XK_d,           XK_D,         XK_kana_SHI,    NoSymbol,     /* 0x44 */
        XK_f,           XK_F,         XK_kana_HA,     NoSymbol,     /* 0x45 */
        XK_g,           XK_G,         XK_kana_KI,     NoSymbol,     /* 0x46 */
        XK_h,           XK_H,         XK_kana_KU,     NoSymbol,     /* 0x47 */
        XK_j,           XK_J,         XK_kana_MA,     NoSymbol,     /* 0x48 */
        XK_k,           XK_K,         XK_kana_NO,     NoSymbol,     /* 0x49 */
        XK_l,           XK_L,         XK_kana_RI,     NoSymbol,     /* 0x4a */
        XK_semicolon,   XK_plus,      XK_kana_RE,     NoSymbol,     /* 0x4b */
        XK_colon,       XK_asterisk,  XK_kana_KE,     NoSymbol,     /* 0x4c */
        XK_bracketright,XK_braceright,XK_kana_MU, XK_kana_closingbracket, /* 0x4d */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4e -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4f -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x50 -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x51 -> Nothing */
        XK_z,           XK_Z,         XK_kana_TU,     XK_kana_tu,   /* 0x52 */
        XK_x,           XK_X,         XK_kana_SA,     NoSymbol,     /* 0x53 */
        XK_c,           XK_C,         XK_kana_SO,     NoSymbol,     /* 0x54 */
        XK_v,           XK_V,         XK_kana_HI,     NoSymbol,     /* 0x55 */
        XK_b,           XK_B,         XK_kana_KO,     NoSymbol,     /* 0x56 */
        XK_n,           XK_N,         XK_kana_MI,     NoSymbol,     /* 0x57 */
        XK_m,           XK_M,         XK_kana_MO,     NoSymbol,     /* 0x58 */
        XK_comma,       XK_less,      XK_kana_NE,     XK_kana_comma,     /* 0x59 */
        XK_period,      XK_greater,   XK_kana_RU,     XK_kana_fullstop,  /* 0x5a */
        XK_slash,       XK_question,  XK_kana_ME,     XK_kana_middledot, /* 0x5b */
        NoSymbol,       XK_underscore,XK_kana_RO,     NoSymbol,     /* 0x5c */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x5d -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x5e -> Nothing */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x5f -> Nothing */
        XK_Home,        NoSymbol,     NoSymbol,       NoSymbol,     /* 0x60 -> Home */
        XK_plus,        NoSymbol,     XK_plus,        NoSymbol,     /* 0x61 */
        XK_minus,       NoSymbol,     XK_minus,       NoSymbol,     /* 0x62 */
        XK_7,           NoSymbol,     XK_7,           NoSymbol,     /* 0x63 */
        XK_8,           NoSymbol,     XK_8,           NoSymbol,     /* 0x64 */
        XK_9,           NoSymbol,     XK_9,           NoSymbol,     /* 0x65 */
        XK_4,           NoSymbol,     XK_4,           NoSymbol,     /* 0x66 */
        XK_5,           NoSymbol,     XK_5,           NoSymbol,     /* 0x67 */
        XK_6,           NoSymbol,     XK_6,           NoSymbol,     /* 0x68 */
        XK_1,           NoSymbol,     XK_1,           NoSymbol,     /* 0x69 */
        XK_2,           NoSymbol,     XK_2,           NoSymbol,     /* 0x6a */
        XK_3,           NoSymbol,     XK_3,           NoSymbol,     /* 0x6b */
        XK_0,           NoSymbol,     XK_0,           NoSymbol,     /* 0x6c */
        XK_period,      NoSymbol,     XK_period,      NoSymbol,     /* 0x6d */
        XK_Return,      NoSymbol,     XK_Return,      NoSymbol,     /* 0x6e */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x6f */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x70 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x71 */
        XK_F1,          NoSymbol,     XK_F1,          NoSymbol,     /* 0x72 */
        XK_F2,          NoSymbol,     XK_F2,          NoSymbol,     /* 0x73 */
        XK_F3,          NoSymbol,     XK_F3,          NoSymbol,     /* 0x74 */
        XK_F4,          NoSymbol,     XK_F4,          NoSymbol,     /* 0x75 */
        XK_F5,          NoSymbol,     XK_F5,          NoSymbol,     /* 0x76 */
        XK_F6,          NoSymbol,     XK_F6,          NoSymbol,     /* 0x77 */
        XK_F7,          NoSymbol,     XK_F7,          NoSymbol,     /* 0x78 */
        XK_F8,          NoSymbol,     XK_F8,          NoSymbol,     /* 0x79 */
        XK_F9,          NoSymbol,     XK_F9,          NoSymbol,     /* 0x7a */
        XK_F10,         NoSymbol,     XK_F10,         NoSymbol,     /* 0x7b */
        XK_asterisk,    XK_asterisk,  XK_asterisk,    XK_asterisk,  /* 0x7c */ /* KANA new */
        XK_slash,       XK_slash,     XK_slash,       XK_slash,     /* 0x7d */
        XK_equal,       XK_equal,     XK_equal,       XK_equal,     /* 0x7e */
        XK_comma,       XK_comma,     XK_comma,       XK_comma,     /* 0x7f */
};

static KeySym omronAsciijisKeyMap[] = {
/*      NormalShift     ShiftedShift                */
        XK_Tab,         NoSymbol,     XK_Tab,         NoSymbol,     /* 0x9 -> min KeyCode */
        XK_Control_L,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0xa */
        XK_Mode_switch, NoSymbol,     NoSymbol,       NoSymbol,     /* 0xb -> kana Key */
        XK_Shift_R,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0xc */
        XK_Shift_L,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0xd */
        XK_Caps_Lock,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0xe */
        XK_Meta_L,      NoSymbol,     NoSymbol,       NoSymbol,     /* 0xf -> zenmen Key */
        XK_Escape,      NoSymbol,     XK_Escape,      NoSymbol,     /* 0x10 */
        XK_BackSpace,   NoSymbol,     XK_BackSpace,   NoSymbol,     /* 0x11 */
        XK_Return,      NoSymbol,     XK_Return,      NoSymbol,     /* 0x12 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x13 -> Nothing */
        XK_space,       NoSymbol,     XK_space,       NoSymbol,     /* 0x14 */
        XK_Delete,      NoSymbol,     XK_Delete,      NoSymbol,     /* 0x15 */
        XK_henkan,      NoSymbol,     XK_henkan,      NoSymbol,     /* 0x16 -> Henkan Key */
        XK_kakutei,     NoSymbol,     XK_kakutei,     NoSymbol,     /* 0x17 -> Kakutei Key */
        XK_F11,         NoSymbol,     XK_F11,         NoSymbol,     /* 0x18 */
        XK_F12,         NoSymbol,     XK_F12,         NoSymbol,     /* 0x19 */
        XK_F13,         NoSymbol,     XK_F13,         NoSymbol,     /* 0x1a */
        XK_F14,         NoSymbol,     XK_F14,         NoSymbol,     /* 0x1b */
        XK_Up,          NoSymbol,     XK_Up,          NoSymbol,     /* 0x1c */
        XK_Left,        NoSymbol,     XK_Left,        NoSymbol,     /* 0x1d */
        XK_Right,       NoSymbol,     XK_Right,       NoSymbol,     /* 0x1e */
        XK_Down,        NoSymbol,     XK_Down,        NoSymbol,     /* 0x1f */
        osfXK_Insert,   NoSymbol,     NoSymbol,       NoSymbol,     /* 0x20 -> Insert */
        osfXK_Copy,     NoSymbol,     NoSymbol,       NoSymbol,     /* 0x21 -> Copy */
        XK_1,           XK_exclam,    XK_kana_NU,     NoSymbol,     /* 0x22 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x23 */
        XK_3,           XK_numbersign,XK_kana_A,      XK_kana_a,    /* 0x24 */
        XK_4,           XK_dollar,    XK_kana_U,      XK_kana_u,    /* 0x25 */
        XK_5,           XK_percent,   XK_kana_E,      XK_kana_e,    /* 0x26 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x27 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x28 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x29 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x2a */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x2b */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x2c */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x2d */
        XK_backslash,   XK_bar,       XK_prolongedsound, NoSymbol,  /* 0x2e */
        XK_9,           XK_parenleft, XK_kana_YO,     XK_kana_yo,   /* 0x2f */
        osfXK_Cut,      NoSymbol,     NoSymbol,       NoSymbol,     /* 0x30 -> Cut */
        osfXK_Paste,    NoSymbol,     NoSymbol,       NoSymbol,     /* 0x31 -> Paste */
        XK_q,           XK_Q,         XK_kana_TA,     NoSymbol,     /* 0x32 */
        XK_w,           XK_W,         XK_kana_TE,     NoSymbol,     /* 0x33 */
        XK_e,           XK_E,         XK_kana_I,      XK_kana_i,    /* 0x34 */
        XK_r,           XK_R,         XK_kana_SU,     NoSymbol,     /* 0x35 */
        XK_t,           XK_T,         XK_kana_KA,     NoSymbol,     /* 0x36 */
        XK_y,           XK_Y,         XK_kana_N,      NoSymbol,     /* 0x37 */
        XK_u,           XK_U,         XK_kana_NA,     NoSymbol,     /* 0x38 */
        XK_i,           XK_I,         XK_kana_NI,     NoSymbol,     /* 0x39 */
        XK_o,           XK_O,         XK_kana_RA,     NoSymbol,     /* 0x3a */
        XK_p,           XK_P,         XK_kana_SE,     NoSymbol,     /* 0x3b */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x3c */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x3d */
        XK_minus,       XK_underscore,XK_kana_HO,     NoSymbol,     /* 0x3e */
        XK_equal,       XK_plus,      XK_kana_HE,     NoSymbol,     /* 0x3f */
        XK_2,           XK_at,        XK_kana_FU,     NoSymbol,     /* 0x40 */
        XK_6,           XK_asciicircum, XK_kana_O,    XK_kana_o,    /* 0x41 */
        XK_a,           XK_A,         XK_kana_TI,     NoSymbol,     /* 0x42 */
        XK_s,           XK_S,         XK_kana_TO,     NoSymbol,     /* 0x43 */
        XK_d,           XK_D,         XK_kana_SHI,    NoSymbol,     /* 0x44 */
        XK_f,           XK_F,         XK_kana_HA,     NoSymbol,     /* 0x45 */
        XK_g,           XK_G,         XK_kana_KI,     NoSymbol,     /* 0x46 */
        XK_h,           XK_H,         XK_kana_KU,     NoSymbol,     /* 0x47 */
        XK_j,           XK_J,         XK_kana_MA,     NoSymbol,     /* 0x48 */
        XK_k,           XK_K,         XK_kana_NO,     NoSymbol,     /* 0x49 */
        XK_l,           XK_L,         XK_kana_RI,     NoSymbol,     /* 0x4a */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4b */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4c */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x4d */
        XK_bracketleft, XK_braceleft, XK_voicedsound, XK_kana_openingbracket,     /* 0x4e */
        XK_bracketright,XK_braceright,XK_semivoicedsound, XK_kana_closingbracket, /* 0x4f */
        XK_7,           XK_ampersand, XK_kana_YA,     XK_kana_ya,   /* 0x50 */
        XK_8,           XK_asterisk,  XK_kana_YU,     XK_kana_yu,   /* 0x51 */
        XK_z,           XK_Z,         XK_kana_TU,     XK_kana_tu,   /* 0x52 */
        XK_x,           XK_X,         XK_kana_SA,     NoSymbol,     /* 0x53 */
        XK_c,           XK_C,         XK_kana_SO,     NoSymbol,     /* 0x54 */
        XK_v,           XK_V,         XK_kana_HI,     NoSymbol,     /* 0x55 */
        XK_b,           XK_B,         XK_kana_KO,     NoSymbol,     /* 0x56 */
        XK_n,           XK_N,         XK_kana_MI,     NoSymbol,     /* 0x57 */
        XK_m,           XK_M,         XK_kana_MO,     NoSymbol,     /* 0x58 */
        XK_comma,       XK_less,      XK_kana_NE,     XK_kana_comma,     /* 0x59 */
        XK_period,      XK_greater,   XK_kana_RU,     XK_kana_fullstop,  /* 0x5a */
        XK_slash,       XK_question,  XK_kana_ME,     XK_kana_middledot, /* 0x5b */
        NoSymbol,       XK_underscore,XK_kana_RO,     NoSymbol,     /* 0x5c */
        XK_semicolon,   XK_colon,     XK_kana_RE,     NoSymbol,     /* 0x5d */
        XK_apostrophe,  XK_quotedbl,  XK_kana_KE,     NoSymbol,     /* 0x5e */
        XK_grave,       XK_asciitilde,XK_kana_MU,     NoSymbol,     /* 0x5f */
        XK_Home,        NoSymbol,     NoSymbol,       NoSymbol,     /* 0x60 -> Home */
        XK_plus,        NoSymbol,     XK_plus,        NoSymbol,     /* 0x61 */
        XK_minus,       NoSymbol,     XK_minus,       NoSymbol,     /* 0x62 */
        XK_7,           NoSymbol,     XK_7,           NoSymbol,     /* 0x63 */
        XK_8,           NoSymbol,     XK_8,           NoSymbol,     /* 0x64 */
        XK_9,           NoSymbol,     XK_9,           NoSymbol,     /* 0x65 */
        XK_4,           NoSymbol,     XK_4,           NoSymbol,     /* 0x66 */
        XK_5,           NoSymbol,     XK_5,           NoSymbol,     /* 0x67 */
        XK_6,           NoSymbol,     XK_6,           NoSymbol,     /* 0x68 */
        XK_1,           NoSymbol,     XK_1,           NoSymbol,     /* 0x69 */
        XK_2,           NoSymbol,     XK_2,           NoSymbol,     /* 0x6a */
        XK_3,           NoSymbol,     XK_3,           NoSymbol,     /* 0x6b */
        XK_0,           NoSymbol,     XK_0,           NoSymbol,     /* 0x6c */
        XK_period,      NoSymbol,     XK_period,      NoSymbol,     /* 0x6d */
        XK_Return,      NoSymbol,     XK_Return,      NoSymbol,     /* 0x6e */
        XK_0,           XK_parenright,XK_kana_WA,     XK_kana_WO,   /* 0x6f */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x70 */
        NoSymbol,       NoSymbol,     NoSymbol,       NoSymbol,     /* 0x71 */
        XK_F1,          NoSymbol,     XK_F1,          NoSymbol,     /* 0x72 */
        XK_F2,          NoSymbol,     XK_F2,          NoSymbol,     /* 0x73 */
        XK_F3,          NoSymbol,     XK_F3,          NoSymbol,     /* 0x74 */
        XK_F4,          NoSymbol,     XK_F4,          NoSymbol,     /* 0x75 */
        XK_F5,          NoSymbol,     XK_F5,          NoSymbol,     /* 0x76 */
        XK_F6,          NoSymbol,     XK_F6,          NoSymbol,     /* 0x77 */
        XK_F7,          NoSymbol,     XK_F7,          NoSymbol,     /* 0x78 */
        XK_F8,          NoSymbol,     XK_F8,          NoSymbol,     /* 0x79 */
        XK_F9,          NoSymbol,     XK_F9,          NoSymbol,     /* 0x7a */
        XK_F10,         NoSymbol,     XK_F10,         NoSymbol,     /* 0x7b */
        XK_asterisk,    XK_asterisk,  XK_asterisk,    XK_asterisk,  /* 0x7c */ /* KANA new */
        XK_slash,       XK_slash,     XK_slash,       XK_slash,     /* 0x7d */
        XK_equal,       XK_equal,     XK_equal,       XK_equal,     /* 0x7e */
        XK_comma,       XK_comma,     XK_comma,       XK_comma,     /* 0x7f */
};

static KeySym omronAsciiKeyMap[] = {        
/*      NormalShift             ShiftedShift            */
        XK_Tab,                 NoSymbol,               /* 0x9 -> min KeyCode */
        XK_Control_L,           NoSymbol,               /* 0xa */
        NoSymbol,               NoSymbol,               /* 0xb -> kana Key */
        XK_Shift_R,             NoSymbol,               /* 0xc */
        XK_Shift_L,             NoSymbol,               /* 0xd */
        XK_Caps_Lock,           NoSymbol,               /* 0xe */
        XK_Meta_L,              NoSymbol,               /* 0xf -> zenmen Key */
        XK_Escape,              NoSymbol,               /* 0x10 */
        XK_BackSpace,           NoSymbol,               /* 0x11 */
        XK_Return,              NoSymbol,               /* 0x12 */
        NoSymbol,               NoSymbol,               /* 0x13 -> Nothing */
        XK_space,               NoSymbol,               /* 0x14 */
        XK_Delete,              NoSymbol,               /* 0x15 */
        NoSymbol,               NoSymbol,               /* 0x16 -> Henkan Key */
        NoSymbol,               NoSymbol,               /* 0x17 -> Kakutei Key */
        XK_F11,                 NoSymbol,               /* 0x18 */
        XK_F12,                 NoSymbol,               /* 0x19 */
        XK_F13,                 NoSymbol,               /* 0x1a */
        XK_F14,                 NoSymbol,               /* 0x1b */
        XK_Up,                  NoSymbol,               /* 0x1c */
        XK_Left,                NoSymbol,               /* 0x1d */
        XK_Right,               NoSymbol,               /* 0x1e */
        XK_Down,                NoSymbol,               /* 0x1f */
        NoSymbol,               NoSymbol,               /* 0x20 */
        NoSymbol,               NoSymbol,               /* 0x21 */
        XK_1,                   XK_exclam,              /* 0x22 */
        NoSymbol,               NoSymbol,               /* 0x23 */
        XK_3,                   XK_numbersign,          /* 0x24 */
        XK_4,                   XK_dollar,              /* 0x25 */
        XK_5,                   XK_percent,             /* 0x26 */
        NoSymbol,               NoSymbol,               /* 0x27 */
        NoSymbol,               NoSymbol,               /* 0x28 */
        NoSymbol,               NoSymbol,               /* 0x29 */
        NoSymbol,               NoSymbol,               /* 0x2a */
        NoSymbol,               NoSymbol,               /* 0x2b */
        NoSymbol,               NoSymbol,               /* 0x2c */
        NoSymbol,               NoSymbol,               /* 0x2d */
        XK_backslash,           XK_bar,                 /* 0x2e */
        XK_9,                   XK_parenleft,           /* 0x2f */
        NoSymbol,               NoSymbol,               /* 0x30 */
        NoSymbol,               NoSymbol,               /* 0x31 */
        XK_q,                   XK_Q,                   /* 0x32 */
        XK_w,                   XK_W,                   /* 0x33 */
        XK_e,                   XK_E,                   /* 0x34 */
        XK_r,                   XK_R,                   /* 0x35 */
        XK_t,                   XK_T,                   /* 0x36 */
        XK_y,                   XK_Y,                   /* 0x37 */
        XK_u,                   XK_U,                   /* 0x38 */
        XK_i,                   XK_I,                   /* 0x39 */
        XK_o,                   XK_O,                   /* 0x3a */
        XK_p,                   XK_P,                   /* 0x3b */
        NoSymbol,               NoSymbol,               /* 0x3c */
        NoSymbol,               NoSymbol,               /* 0x3d */
        XK_minus,               XK_underscore,          /* 0x3e */
        XK_equal,               XK_plus,                /* 0x3f */
        XK_2,                   XK_at,                  /* 0x40 */
        XK_6,                   XK_asciicircum,         /* 0x41 */
        XK_a,                   XK_A,                   /* 0x42 */
        XK_s,                   XK_S,                   /* 0x43 */
        XK_d,                   XK_D,                   /* 0x44 */
        XK_f,                   XK_F,                   /* 0x45 */
        XK_g,                   XK_G,                   /* 0x46 */
        XK_h,                   XK_H,                   /* 0x47 */
        XK_j,                   XK_J,                   /* 0x48 */
        XK_k,                   XK_K,                   /* 0x49 */
        XK_l,                   XK_L,                   /* 0x4a */
        NoSymbol,               NoSymbol,               /* 0x4b */
        NoSymbol,               NoSymbol,               /* 0x4c */
        NoSymbol,               NoSymbol,               /* 0x4d */
        XK_bracketleft,         XK_braceleft,           /* 0x4e */
        XK_bracketright,        XK_braceright,          /* 0x4f */
        XK_7,                   XK_ampersand,           /* 0x50 */
        XK_8,                   XK_asterisk,            /* 0x51 */
        XK_z,                   XK_Z,                   /* 0x52 */
        XK_x,                   XK_X,                   /* 0x53 */
        XK_c,                   XK_C,                   /* 0x54 */
        XK_v,                   XK_V,                   /* 0x55 */
        XK_b,                   XK_B,                   /* 0x56 */
        XK_n,                   XK_N,                   /* 0x57 */
        XK_m,                   XK_M,                   /* 0x58 */
        XK_comma,               XK_less,                /* 0x59 */
        XK_period,              XK_greater,             /* 0x5a */
        XK_slash,               XK_question,            /* 0x5b */
        NoSymbol,               NoSymbol,               /* 0x5c XXX */
        XK_semicolon,           XK_colon,               /* 0x5d */
        XK_apostrophe,          XK_quotedbl,            /* 0x5e */
        XK_grave,               XK_asciitilde,          /* 0x5f */
        XK_Home,                NoSymbol,               /* 0x60 -> Home */
        NoSymbol,               NoSymbol,               /* 0x61 */
        NoSymbol,               NoSymbol,               /* 0x62 */
        NoSymbol,               NoSymbol,               /* 0x63 */
        NoSymbol,               NoSymbol,               /* 0x64 */
        NoSymbol,               NoSymbol,               /* 0x65 */
        NoSymbol,               NoSymbol,               /* 0x66 */
        NoSymbol,               NoSymbol,               /* 0x67 */
        NoSymbol,               NoSymbol,               /* 0x68 */
        NoSymbol,               NoSymbol,               /* 0x69 */
        NoSymbol,               NoSymbol,               /* 0x6a */
        NoSymbol,               NoSymbol,               /* 0x6b */
        NoSymbol,               NoSymbol,               /* 0x6c */
        NoSymbol,               NoSymbol,               /* 0x6d */
        NoSymbol,               NoSymbol,               /* 0x6e */
        XK_0,                   XK_parenright,          /* 0x6f */
        NoSymbol,               NoSymbol,               /* 0x70 */
        NoSymbol,               NoSymbol,               /* 0x71 */
        XK_F1,                  NoSymbol,               /* 0x72 */
        XK_F2,                  NoSymbol,               /* 0x73 */
        XK_F3,                  NoSymbol,               /* 0x74 */
        XK_F4,                  NoSymbol,               /* 0x75 */
        XK_F5,                  NoSymbol,               /* 0x76 */
        XK_F6,                  NoSymbol,               /* 0x77 */
        XK_F7,                  NoSymbol,               /* 0x78 */
        XK_F8,                  NoSymbol,               /* 0x79 */
        XK_F9,                  NoSymbol,               /* 0x7a */
        XK_F10,                 NoSymbol,               /* 0x7b */
};
#endif
#endif

KeySymsRec omronKeySyms[] = {
		/*  map          minKeyCode   maxKeyCode  width */
#ifdef USE_KEYCOMPATI
# ifdef USE_KANA_SWITCH
#  ifdef luna2
		omronJisjisKeyMap,   0x9,         0x7f,   4,
		omronAsciiKeyMap,    0x9,         0x7b,   2,
		omronAsciijisKeyMap, 0x9,         0x7f,   4,
#  endif
		omronDefKeyMap,      0x9,         0x7f,   4
# else
#  ifdef luna2
		omronJisjisKeyMap,   0x9,         0xf6,   2,
		omronAsciiKeyMap,    0x9,         0x7b,   2,
		omronAsciijisKeyMap, 0x9,         0xf6,   2,
#  endif
		omronDefKeyMap,      0x9,         0xf6,   2
# endif
#else
# ifdef USE_KANA_SWITCH
#  ifdef luna2
		omronJisjisKeyMap,   0x10,        0x86,   4,
		omronAsciiKeyMap,    0x10,        0x82,   2,
		omronAsciijisKeyMap, 0x10,        0x86,   4,
#  endif
		omronDefKeyMap,      0x10,        0x86,   4
# else
#  ifdef luna2
		omronJisjisKeyMap,   0x10,        0xfd,   2,
		omronAsciiKeyMap,    0x10,        0x82,   2,
		omronAsciijisKeyMap, 0x10,        0xfd,   2,
#  endif
		omronDefKeyMap,      0x10,        0xfd,   2
# endif
#endif
};

#ifdef luna2
# ifdef USE_KEYCOMPATI
static unsigned char omronAsciiAutoRepeats[AREPBUFSZ] = {
	0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
# else
static unsigned char omronAsciiAutoRepeats[AREPBUFSZ] = {
	0x00, 0x00, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
# endif
#endif

#ifdef USE_KEYCOMPATI
# ifdef USE_KANA_SWITCH
static unsigned char omronDefAutoRepeats[AREPBUFSZ] = {
	0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
# else
static unsigned char omronDefAutoRepeats[AREPBUFSZ] = {
	0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00
};
# endif
#else
# ifdef USE_KANA_SWITCH
static unsigned char omronDefAutoRepeats[AREPBUFSZ] = {
	0x00, 0x00, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
# else
static unsigned char omronDefAutoRepeats[AREPBUFSZ] = {
	0x00, 0x00, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f
};
# endif
#endif

unsigned char *omronAutoRepeats[] = {
#ifdef luna2
	omronDefAutoRepeats,
	omronAsciiAutoRepeats,
	omronDefAutoRepeats,
#endif
	omronDefAutoRepeats
};
