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
 * @(#)$RCSfile: british_lk201le_tw.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/11/09 17:11:11 $
 */
/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * This file describes the DEC LK201 keyboard.
 */

#ifndef NO_DEC_KEYSYMS
/* This file comes with the MIT distribution, but won't necessarily
   be present in all X implementations. */
# include <X11/DECkeysym.h>
#else
# define DXK_Remove 0x1000FF00
#endif

static struct key british_lk201le_tw_row0 [] = {
 {86,	0,	0,	7, 7,	0,	XK_F1},
 {87,	0,	0,	7, 7,	0,	XK_F2},
 {88,	0,	0,	7, 7,	0,	XK_F3},
 {89,	0,	0,	7, 7,	0,	XK_F4},
 {90,	0,	0,	7, 7,	0,	XK_F5},
 {0,	0,	0,	7, 7},
 {100,	0,	0,	7, 7,	0,	XK_F6},
 {101,	0,	0,	7, 7,	0,	XK_F7},
 {102,	0,	0,	7, 7,	0,	XK_F8},
 {103,	0,	0,	7, 7,	0,	XK_F9},
 {104,	0,	0,	7, 7,	0,	XK_F10},
 {0,	0,	0,	7, 7},
 {113,	0,	0,	7, 7,	0,	XK_Escape, XK_F11},
 {114,	0,	0,	7, 7,	0,	XK_F12},
 {115,	0,	0,	7, 7,	0,	XK_F13},
 {116,	0,	0,	7, 7,	0,	XK_F14},
 {0,	0,	0,	7, 7},
 {124,	0, 	0,	8, 7,	0,	XK_Help},
 {125,	0, 	0,	16, 7,	0,	XK_Menu},
 {0,	0,	0,	7, 7},
 {128,	0, 	0,	7, 7,	0,	XK_F17},
 {129,	0, 	0,	7, 7,	0,	XK_F18},
 {130,	0, 	0,	7, 7,	0,	XK_F19},
 {131,	0, 	0,	7, 7,	0,	XK_F20}
};

static struct key british_lk201le_tw_row2 [] = {
 {0,	0,	0,	5, 7},
 {191,	0,	0,	7, 7,	0,	XK_grave,	XK_asciitilde},
 {0xc0, "!", "1", 7, 7,	0,	XK_1, XK_exclam},
 {0xc5, "\"", "2", 7, 7,	0,	XK_2, XK_quotedbl},
 {0xcb, StrXK_sterling, "3", 7, 7,	0,	XK_3, XK_sterling},
 {0xd0, "$", "4", 7, 7,	0,	XK_4, XK_dollar},
 {0xd6, "", "5", 7, 7,	0,	XK_5, XK_percent},
 {0xdb, "&", "6", 7, 7,	0,	XK_6, XK_ampersand},
 {0xe0, "'", "7", 7, 7,	0,	XK_7, XK_apostrophe},
 {0xe5, "(", "8", 7, 7,	0,	XK_8, XK_parenleft},
 {0xea, ")", "9", 7, 7,	0,	XK_9, XK_parenright},
 {0xef, "=", "0", 7, 7,	0,	XK_0, XK_equal},
 {0xf9, "_", "-", 7, 7,	0,	XK_minus, XK_underscore},
 {0xf5, StrXK_onequarter, StrXK_onehalf, 7, 7,	0,	XK_onehalf, XK_onequarter},
/*DEL*/
 {188,	0, 	0,	11, 7,	0,	XK_Delete},
 {0,	0,	0,	12, 7},
 {138,	0, 	0,	8, 7,	0,	XK_Find},
 {139,	0,0, 8, 7,	0,	XK_Insert},
 {140, 0,0, 	8, 7,	0,	DXK_Remove},
 {0,	0,	0,	7, 7},
 {161,	0, 	0,	7, 7,	0,	XK_KP_F1},
 {162,	0, 	0,	7, 7,	0,	XK_KP_F2},
 {163,	0, 	0,	7, 7,	0,	XK_KP_F3},
 {164,	0, 	0,	7, 7,	0,	XK_KP_F4}
};
 
static struct key british_lk201le_tw_row3 [] = {
 {0,	0,	0,		5, 7},
 {190,	0, 	0,		12, 7,	0,	XK_Tab},
 {0xc1, 0, "Q", 7, 7,	0,	XK_Q},
 {0xc6, 0, "W", 7, 7,	0,	XK_W},
 {0xcc, 0, "E", 7, 7,	0,	XK_E},
 {0xd1, 0, "R", 7, 7,	0,	XK_R},
 {0xd7, 0, "T", 7, 7,	0,	XK_T},
 {0xdc, 0, "Y", 7, 7,	0,	XK_Y},
 {0xe1, 0, "U", 7, 7,	0,	XK_U},
 {0xe6, 0, "I", 7, 7,	0,	XK_I},
 {0xeb, 0, "O", 7, 7,	0,	XK_O},
 {0xf0, 0, "P", 7, 7,	0,	XK_P},
 {0xfa, StrXK_section, "@", 7, 7,	0,	XK_at, XK_section},
 {0xf6, "[", "]", 7, 7,	0,	XK_bracketright, XK_bracketleft},
 {0,	0,	0,		2, 7},
 {189,	0, 0,		8, 14,	0,	XK_Return},
 {0,	0,	0,		8, 7},
/*KPDEL*/
 {141,	0, 0,		8, 7,	0,	XK_Select},
 {142,	0, 	0, 	8, 7,	0,	XK_Prior},
 {143,	0, 	0, 	8, 7,	0,	XK_Next},
 {0,	0,	0,		7, 7},
 {157,	0, 	0,		7, 7,	0,	XK_KP_7},
 {158,	0, 	0,		7, 7,	0,	XK_KP_8},
 {159,	0, 	0,		7, 7,	0,	XK_KP_9},
 {160,	0,	0,		7, 7,	0,	XK_KP_Subtract}
};

static struct key british_lk201le_tw_row4 [] = {
 {175,	0, 	0,		7, 7,	ControlMask,	XK_Control_L},
 {176,	0, 	0,		12, 7,	LockMask,	XK_Caps_Lock},
 {0xc2, 0, "A", 7, 7,	0,	XK_A},
 {0xc7, 0, "S", 7, 7,	0,	XK_S},
 {0xcd, 0, "D", 7, 7,	0,	XK_D},
 {0xd2, 0, "F", 7, 7,	0,	XK_F},
 {0xd8, 0, "G", 7, 7,	0,	XK_G},
 {0xdd, 0, "H", 7, 7,	0,	XK_H},
 {0xe2, 0, "J", 7, 7,	0,	XK_J},
 {0xe7, 0, "K", 7, 7,	0,	XK_K},
 {0xec, 0, "L", 7, 7,	0,	XK_L},
 {0xf2, "+", ";", 7, 7,	0,	XK_semicolon, XK_plus},
 {0xfb, "*", ":", 7, 7,	0,	XK_colon, XK_asterisk},
 {247,	0,	0,		7, 7,	0,	XK_backslash,	XK_bar},
 {0,	0,	0,		24, 7},
/*LEFT*/
 {170,	0, 0,		8, 7,	0,	XK_Up},
 {0,	0,	0,		15, 7},
 {153,	0, 	0,		7, 7,	0,	XK_KP_4},
 {154,	0, 	0,		7, 7,	0,	XK_KP_5},
 {155,	0, 	0,		7, 7,	0,	XK_KP_6},
 {156,	0,	0,		7, 7,	0,	XK_KP_Separator}
};

static struct key british_lk201le_tw_row5 [] = {
 {174,	0, 0,		16, 7,	ShiftMask,		XK_Shift_L},
 {0xc9, ">", "<", 7, 7,	0,	XK_less, XK_greater},
 {0xc3, 0, "Z", 7, 7,	0,	XK_Z},
 {0xc8, 0, "X", 7, 7,	0,	XK_X},
 {0xce, 0, "C", 7, 7,	0,	XK_C},
 {0xd3, 0, "V", 7, 7,	0,	XK_V},
 {0xd9, 0, "B", 7, 7,	0,	XK_B},
 {0xde, 0, "N", 7, 7,	0,	XK_N},
 {0xe3, 0, "M", 7, 7,	0,	XK_M},
 {0xe8, 0, ",", 7, 7,	0,	XK_comma},
 {0xed, 0, ".", 7, 7,	0,	XK_period},
 {0xf3, "?", "/", 7, 7,	0,	XK_slash, XK_question},
/*SHIFT*/
 {171,	0, 0,		16, 7,		ShiftMask,	XK_Shift_R},
 {0,	0,	0,		10, 7},
 {167,0,  0,		8, 7,	0,	XK_Left},
 {169,0,  0,		8, 7,	0,	XK_Down},
 {168,0, 0,		8, 7,	0,	XK_Right},
 {0,	0,	0,		7, 7,	0,	XK_KP_0},
 {150,	0, 	0,		7, 7,	0,	XK_KP_1},
 {151,	0, 	0,		7, 7,	0,	XK_KP_2},
 {152,	0, 	0,		7, 7,	0,	XK_KP_3},
 {149,0, 	0,		7, 14,	0,	XK_KP_Enter}
};

static struct key british_lk201le_tw_row6 [] = {
 {0,	0,	0,		9, 7},
 {177, 0,0, 	16, 7,	Mod1Mask,	XK_Alt_L, XK_Meta_L},
 {212,	0,	0,		64, 7,	0,		XK_space},
 {0,	0,	0,		61, 7},
 {146,	0,	0,		14, 7,	0,		XK_KP_0},
 {148,	0,	0,		7, 7,	0,		XK_KP_Decimal}
};

static struct row british_lk201le_tw_rows [] = {
  { sizeof (british_lk201le_tw_row0) / sizeof (struct key), 7, british_lk201le_tw_row0 },
  { 0, 7, 0 },
  { sizeof (british_lk201le_tw_row2) / sizeof (struct key), 7, british_lk201le_tw_row2 },
  { sizeof (british_lk201le_tw_row3) / sizeof (struct key), 7, british_lk201le_tw_row3 },
  { sizeof (british_lk201le_tw_row4) / sizeof (struct key), 7, british_lk201le_tw_row4 },
  { sizeof (british_lk201le_tw_row5) / sizeof (struct key), 7, british_lk201le_tw_row5 },
  { sizeof (british_lk201le_tw_row6) / sizeof (struct key), 7, british_lk201le_tw_row6 }
};

static struct keyboard british_lk201le_tw = {
  "LK201 (UK)",
  sizeof (british_lk201le_tw_rows) / sizeof (struct row),
  british_lk201le_tw_rows,
  6, 3, 3
};
