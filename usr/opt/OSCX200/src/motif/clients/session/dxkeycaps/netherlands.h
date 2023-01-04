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
 * @(#)$RCSfile: netherlands.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1994/01/24 19:20:34 $
 */
/* 
**	xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
**
** This file describes the 102-key DEC PC keyboard.
** By Steven W. Orr <steveo@world.std.com>.
*/

static struct key netherlands_row1 [] = {
 {0x8, 0, 0, 8, 8,	0,	XK_Escape},
 {0,	0,	0,	8, 8},
 {0x9, 0, 0, 8, 8,	0,	XK_F1},
 {0xf, 0, 0, 8, 8,	0,	XK_F2},
 {0x17, 0, 0, 8, 8,	0,	XK_F3},
 {0x1f, 0, 0, 8, 8,	0,	XK_F4},
 {0,	0,	0,	4, 8},
 {0x27, 0, 0, 8, 8,	0,	XK_F5},
 {0x2f, 0, 0, 8, 8,	0,	XK_F6},
 {0x37, 0, 0, 8, 8,	0,	XK_F7},
 {0x3f, 0, 0, 8, 8,	0,	XK_F8},
 {0,	0,	0,	4, 8},
 {0x47, 0, 0, 8, 8,	0,	XK_F9},
 {0x4f, 0, 0, 8, 8,	0,	XK_F10},
 {0x56, 0, 0, 8, 8,	0,	XK_F11},
 {0x5e, 0, 0, 8, 8,	0,	XK_F12},
 {0,	0,	0,	4, 8},
 {0x57, 0, 0, 8, 8,	0,	XK_Print, XK_Sys_Req},
 {0x5f, 0, 0, 8, 8,	0,	XK_Scroll_Lock},
 {0x62, 0, 0, 8, 8,	0,	XK_Pause},
};

static struct key netherlands_row2 [] = {
 {0xe, StrXK_section, "@", 8, 8,	0,	XK_at, XK_section, XK_notsign, XK_at, XK_section, XK_notsign},
 {0x16, "!", "1", 8, 8,	0,	XK_1, XK_exclam, XK_onesuperior},
 {0x1e, "\"", "2", 8, 8,	0,	XK_2, XK_quotedbl, XK_twosuperior},
 {0x26, "#", "3", 8, 8,	0,	XK_3, XK_numbersign, XK_threesuperior},
 {0x25, "$", "4", 8, 8,	0,	XK_4, XK_dollar, XK_onequarter},
 {0x2e, "", "5", 8, 8,	0,	XK_5, XK_percent, XK_onehalf},
 {0x36, "&", "6", 8, 8,	0,	XK_6, XK_ampersand, XK_threequarters},
 {0x3d, "_", "7", 8, 8,	0,	XK_7, XK_underscore, XK_sterling},
 {0x3e, "(", "8", 8, 8,	0,	XK_8, XK_parenleft, XK_braceleft},
 {0x46, ")", "9", 8, 8,	0,	XK_9, XK_parenright, XK_braceright},
 {0x45, "'", "0", 8, 8,	0,	XK_0, XK_apostrophe},
 {0x4e, "?", "/", 8, 8,	0,	XK_slash, XK_question, XK_backslash},
 {0x55, "~", StrXK_degree, 8, 8,	0,	XK_degree, XK_asciitilde, XK_cedilla},
/*DEL*/
 {0x66, 0, 0, 16, 8,	0,	XK_Delete},
 {0,	0,	0,	4, 8},
 {0x67, 0, 0, 8, 8,	0,	XK_Insert},
 {0x6e, 0, 0, 8, 8,	0,	XK_Home},
 {0x6f, 0, 0, 8, 8,	0,	XK_Prior},
 {0,	0,	0,	4, 8},
 {0x76, 0, 0, 8, 8,	0,	XK_Num_Lock, XK_KP_F1},
 {0x77, 0, 0, 8, 8,	0,	XK_KP_Divide, XK_KP_F2},
 {0x7e, 0, 0, 8, 8,	0,	XK_KP_Multiply, XK_KP_F3},
 {0x84, 0, 0, 8, 8,	0,	XK_KP_Subtract, XK_KP_F4},
};

static struct key netherlands_row3 [] = {
 {0xd, 0, 0, 12, 8,	0,	XK_Tab},
 {0x15, 0, "Q", 8, 	8,	0,	XK_Q},
 {0x1d, 0, "W", 8, 	8,	0,	XK_W},
 {0x24, 0, "E", 8, 	8,	0,	XK_E},
 {0x2d, "R", "r", 8, 	8,	0,	XK_r, XK_R, XK_paragraph},
 {0x2c, 0, "T", 8, 	8,	0,	XK_T},
 {0x35, 0, "Y", 8, 	8,	0,	XK_Y},
 {0x3c, 0, "U", 8, 	8,	0,	XK_U},
 {0x43, 0, "I", 8, 	8,	0,	XK_I},
 {0x44, 0, "O", 8, 	8,	0,	XK_O},
 {0x4d, 0, "P", 8, 	8,	0,	XK_P},
 {0x54, "^", StrXK_diaeresis, 8,	8,	0,	XK_diaeresis, XK_asciicircum},
 {0x5b, StrXK_brokenbar, "*", 8,	8,	0,	XK_asterisk, XK_brokenbar},
 {0,    0,  0, 	2, 	7},
 {0x5a, 0, 0, 10, 16,	0,	XK_Return},
 {0,	0,	0,	4, 	7},
/*KPDEL*/
 {0x64, 0, 0, 8, 8,	0,	XK_Delete},
 {0x65, 0, 0, 8, 8,	0,	XK_End},
 {0x6d, 0, 0, 8, 8,	0,	XK_Next},
 {0,	0,	0,	4, 8},
 {0x6c, 0, 0, 8, 8,	0, XK_KP_7, XK_Home},
 {0x75, 0, 0, 8, 8,	0, XK_KP_8, XK_Up},
 {0x7d, 0, 0, 8, 8,	0, XK_KP_9, XK_Prior},
 {0x7c, 0, 0, 8, 16,	0,	XK_KP_Add},
};

static struct key netherlands_row4 [] = {
 {0x14, 0, 0, 14, 8,	LockMask,	XK_Caps_Lock},
 {0x1c, 0, "A", 8, 	8,	0,	XK_A},
 {0x1b, "S", "s", 8, 	8,	0,	XK_s, XK_S, XK_ssharp},
 {0x23, 0, "D", 8, 	8,	0,	XK_D},
 {0x2b, 0, "F", 8, 	8,	0,	XK_F},
 {0x34, 0, "G", 8, 	8,	0,	XK_G},
 {0x33, 0, "H", 8, 	8,	0,	XK_H},
 {0x3b, 0, "J", 8, 	8,	0,	XK_J},
 {0x42, 0, "K", 8, 	8,	0,	XK_K},
 {0x4b, 0, "L", 8, 	8,	0,	XK_L},
 {0x4c, StrXK_plusminus, "+", 8,	8,	0,	XK_plus, XK_plusminus},
 {0x52, "`", StrXK_acute, 8,	8,	0,	XK_acute, XK_grave},
 {0x53, ">", "<", 8,	8,	0,	XK_less, XK_greater},
 {0,	0,	0,	42,	7},
/*LEFT*/
 {0x6b, 0, 0, 8, 	8,	0, XK_KP_4, XK_Left},
 {0x73, 0, 0, 8, 	8,	0,	XK_KP_5},
 {0x74, 0, 0, 8, 	8,	0, XK_KP_6, XK_Right},
};

static struct key netherlands_row5 [] = {
 {0x12, 0, 0, 10, 8,	ShiftMask,	XK_Shift_L},
 {0x13, "[", "]", 8, 	8,	0,		XK_bracketright, XK_bracketleft, XK_brokenbar},
 {0x1a, "Z", "z", 8, 	8,	0,		XK_z, XK_Z, XK_guillemotleft},
 {0x22, "X", "x", 8, 	8,	0,		XK_x, XK_X, XK_guillemotright},
 {0x21, "C", "c", 8, 	8,	0,		XK_c, XK_C, XK_cent},
 {0x2a, 0, "V", 8, 	8,	0,		XK_V},
 {0x32, 0, "B", 8, 	8,	0,		XK_B},
 {0x31, 0, "N", 8, 	8,	0,		XK_N},
 {0x3a, "M", "m", 8, 	8,	0,		XK_m, XK_M, XK_mu},
 {0x41, ";", ",", 8, 	8,	0,		XK_comma, XK_semicolon},
 {0x49, ":", ".", 8, 	8,	0,		XK_period, XK_colon, XK_abovedot},
 {0x4a, "=", "-", 8, 	8,	0,		XK_minus, XK_equal},
/*SHIFT*/
 {0x59, 0, 0, 22, 8,	ShiftMask,	XK_Shift_R},
 {0,	0,	0,	12, 8},
 {0x63, 0, 0, 8, 	8,	0,		XK_Up},
 {0,	0,	0,	12, 8},
 {0x69, 0, 0, 8, 	8,	0, XK_KP_1, XK_End},
 {0x72, 0, 0, 8, 	8,	0, XK_KP_2, XK_Down},
 {0x7a, 0, 0, 8, 	8,	0, XK_KP_3, XK_Next},
 {0x79, 0, 0, 8, 16,	0,	XK_KP_Enter},
};

static struct key netherlands_row6 [] = {
 {0x11, 0, 0, 12, 8,	ControlMask,	XK_Control_L},
 {0,	0,	0,	8, 	8},
 {0x19, 0, 0, 12, 8,	Mod1Mask,	XK_Alt_L, XK_Meta_L},
 {0x29, 0, " ", 57, 8,	0,		XK_space},
 {0x39, 0, 0, 12, 8,	Mod3Mask,	XK_Mode_switch},
 {0,	0,	0,	7, 	8},
 {0x58, 0, 0, 12, 8,	ControlMask,	XK_Control_R},
 {0,	0,	0,	4, 	8},
 {0x61, 0, 0, 8, 	8,	0,		XK_Left},
 {0x60, 0, 0, 8, 	8,	0,		XK_Down},
 {0x6a, 0, 0, 8, 	8,	0,		XK_Right},
 {0,	0,	0,	4, 	8},
 {0x70, 0, 0, 16, 8,	0,		XK_KP_0, XK_Insert},
 {0x71, 0, 0, 8, 	8,	0,		XK_KP_Decimal, XK_Delete},
};

static struct row netherlands_rows [] = {
  { sizeof (netherlands_row1) / sizeof (struct key), 8, netherlands_row1 },
  { 0, 8, 0 },
  { sizeof (netherlands_row2) / sizeof (struct key), 8, netherlands_row2 },
  { sizeof (netherlands_row3) / sizeof (struct key), 8, netherlands_row3 },
  { sizeof (netherlands_row4) / sizeof (struct key), 8, netherlands_row4 },
  { sizeof (netherlands_row5) / sizeof (struct key), 8, netherlands_row5 },
  { sizeof (netherlands_row6) / sizeof (struct key), 8, netherlands_row6 },
};

static struct keyboard netherlands = {
  "PCXAL (Nederlands)",
  sizeof (netherlands_rows) / sizeof (struct row),
  netherlands_rows,
  6, 3, 3
};
