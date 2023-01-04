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
 * @(#)$RCSfile: austrian_german_lk401ag.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/11/09 17:10:38 $
 */

/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * This file describes the DEC LK401 keyboard.
 * This file shares data with the DEC LK401 keyboard description.
 */


static struct key austrian_german_lk401ag_row1 [] = {
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

static struct key austrian_german_lk401ag_row2 [] = {
 {0,	0,	0,	5, 7},
 {0xbf, StrXK_degree, "^", 7, 7,	0,	XK_asciicircum, XK_degree, XK_notsign},
 {0xc0, "!", "1", 7, 7,	0,	XK_1, XK_exclam, XK_onesuperior, XK_exclamdown},
 {0xc5, "\"", "2", 7, 7,	0,	XK_2, XK_quotedbl, XK_twosuperior},
 {0xcb, StrXK_section, "3", 7, 7,	0,	XK_3, XK_section, XK_threesuperior, XK_sterling},
 {0xd0, "$", "4", 7, 7,	0,	XK_4, XK_dollar, XK_onequarter, XK_currency},
 {0xd6, "", "5", 7, 7,	0,	XK_5, XK_percent, XK_onehalf},
 {0xdb, "&", "6", 7, 7,	0,	XK_6, XK_ampersand, XK_threequarters},
 {0xe0, "/", "7", 7, 7,	0,	XK_7, XK_slash, XK_braceleft},
 {0xe5, "(", "8", 7, 7,	0,	XK_8, XK_parenleft, XK_bracketleft},
 {0xea, ")", "9", 7, 7,	0,	XK_9, XK_parenright, XK_bracketright, XK_plusminus},
 {0xef, "=", "0", 7, 7,	0,	XK_0, XK_equal, XK_braceright, XK_degree},
 {0xf9, "?", StrXK_ssharp, 7, 7,	0,	XK_ssharp, XK_question, XK_backslash, XK_questiondown},
 {0xf5, "`", "'", 7, 7,	0,	XK_apostrophe, XK_grave, DXK_cedilla_accent},
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
 
static struct key austrian_german_lk401ag_row3 [] = {
 {0,	0,	0,		5, 7},
 {190,	0, 	0,		12, 7,	0,	XK_Tab},
 {0xc1, "Q", "q", 7, 7,	0,	XK_q, XK_Q, XK_at},
 {0xc6, "W", "w", 7, 7,	0,	XK_w, XK_W},
 {0xcc, "E", "e", 7, 7,	0,	XK_e, XK_E},
 {0xd1, "R", "r", 7, 7,	0,	XK_r, XK_R, XK_paragraph, XK_registered},
 {0xd7, "T", "t", 7, 7,	0,	XK_t, XK_T},
 {0xdc, "Z", "z", 7, 7,	0,	XK_z, XK_Z, NoSymbol, XK_yen},
 {0xe1, "U", "u", 7, 7,	0,	XK_u, XK_U},
 {0xe6, "I", "i", 7, 7,	0,	XK_i, XK_I},
 {0xeb, "O", "o", 7, 7,	0,	XK_o, XK_O, XK_oslash, XK_Ooblique},
 {0xf0, "P", "p", 7, 7,	0,	XK_p, XK_P, XK_thorn, XK_THORN},
 {0xfa, StrXK_Udiaeresis, StrXK_udiaeresis, 7, 7,	0,	XK_udiaeresis, XK_Udiaeresis, DXK_diaeresis, DXK_ring_accent},
 {0xf6, "*", "+", 7, 7,	0,	XK_plus, XK_asterisk, DXK_tilde},
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

static struct key austrian_german_lk401ag_row4 [] = {
 {175,	0, 	0,		7, 7,	ControlMask,	XK_Control_L},
 {176,	0, 	0,		12, 7,	LockMask,	XK_Caps_Lock},
 {0xc2, "A", "a", 7, 7,	0,	XK_a, XK_A, XK_ae, XK_AE},
 {0xc7, "S", "s", 7, 7,	0,	XK_s, XK_S, XK_ssharp, XK_section},
 {0xcd, "D", "d", 7, 7,	0,	XK_d, XK_D, XK_eth, XK_ETH},
 {0xd2, "F", "f", 7, 7,	0,	XK_f, XK_F, NoSymbol, XK_ordfeminine},
 {0xd8, "G", "g", 7, 7,	0,	XK_g, XK_G},
 {0xdd, "H", "h", 7, 7,	0,	XK_h, XK_H},
 {0xe2, "J", "j", 7, 7,	0,	XK_j, XK_J},
 {0xe7, "K", "k", 7, 7,	0,	XK_k, XK_K, NoSymbol, XK_ampersand},
 {0xec, "L", "l", 7, 7,	0,	XK_l, XK_L},
 {0xf2, StrXK_Odiaeresis, StrXK_odiaeresis, 7, 7,	0,	XK_odiaeresis, XK_Odiaeresis, DXK_acute_accent},
 {0xfb, StrXK_Adiaeresis, StrXK_adiaeresis, 7, 7,	0,	XK_adiaeresis, XK_Adiaeresis, DXK_circumflex_accent},
 {0xf7, "'", "#", 7, 7,	0,	XK_numbersign, XK_apostrophe, DXK_grave_accent},
 {0,	0,	0,		24, 7},
/*LEFT*/
 {170,	0, 0,		8, 7,	0,	XK_Up},
 {0,	0,	0,		15, 7},
 {153,	0, 	0,		7, 7,	0,	XK_KP_4},
 {154,	0, 	0,		7, 7,	0,	XK_KP_5},
 {155,	0, 	0,		7, 7,	0,	XK_KP_6},
 {156,	0,	0,		7, 7,	0,	XK_KP_Separator}
};

static struct key austrian_german_lk401ag_row5 [] = {
 {174,	"Shift",0,		16, 7,	ShiftMask,		XK_Shift_L},
 {0xc9, ">", "<", 7, 7,	0,	XK_less, XK_greater, XK_bar, XK_brokenbar},
 {0xc3, "Y", "y", 7, 7,	0,	XK_y, XK_Y, XK_guillemotleft, XK_less},
 {0xc8, "X", "x", 7, 7,	0,	XK_x, XK_X, XK_guillemotright, XK_greater},
 {0xce, "C", "c", 7, 7,	0,	XK_c, XK_C, XK_cent, XK_copyright},
 {0xd3, "V", "v", 7, 7,	0,	XK_v, XK_V},
 {0xd9, "B", "b", 7, 7,	0,	XK_b, XK_B},
 {0xde, "N", "n", 7, 7,	0,	XK_n, XK_N},
 {0xe3, "M", "m", 7, 7,	0,	XK_m, XK_M, XK_mu, XK_masculine},
 {0xe8, ";", ",", 7, 7,	0,	XK_comma, XK_semicolon, NoSymbol, XK_multiply},
 {0xed, ":", ".", 7, 7,	0,	XK_period, XK_colon, XK_periodcentered, XK_division},
 {0xf3, "_", "-", 7, 7,	0,	XK_minus, XK_underscore, XK_hyphen},
/*SHIFT*/
 {171,	"Shift",0,		16, 7,		ShiftMask,	XK_Shift_R},
 {0,	0,	0,		10, 7},
 {167,"LeftArrow", 0,		8, 7,	0,	XK_Left},
 {169,"DownArrow", 0,		8, 7,	0,	XK_Down},
 {168,"RightArrow",0,		8, 7,	0,	XK_Right},
 {0,	0,	0,		7, 7,	0,	XK_KP_0},
 {150,	"1",	0,		7, 7,	0,	XK_KP_1},
 {151,	"2",	0,		7, 7,	0,	XK_KP_2},
 {152,	"3",	0,		7, 7,	0,	XK_KP_3},
 {149,"Enter",	0,		7, 14,	0,	XK_KP_Enter}
};

static struct key austrian_german_lk401ag_row6 [] = {
 {0,	0,	0,		9, 7},
 {177, "Compose", "Character",	12, 7,	Mod1Mask, XK_Multi_key},
 {172, "Alt",	"Function",	12, 7,	Mod2Mask, XK_Alt_L, XK_Meta_L},
 {212,	" ",	0,		48, 7,	0,	  XK_space},
 {178, "Alt",	"Function",	12, 7,	Mod2Mask, XK_Alt_R, XK_Meta_R},
 {173, "Compose","Character",	12, 7,	Mod1Mask, XK_Multi_key},
 {0,	0,	0,		45, 7},
 {146,	"0",	0,		14, 7,	0,	  XK_KP_0},
 {148,	".",	0,		7, 7,	0,	  XK_KP_Decimal}
};

static struct row austrian_german_lk401ag_rows [] = {
  { sizeof (austrian_german_lk401ag_row1) / sizeof (struct key), 7, austrian_german_lk401ag_row1 },
  { 0, 7, 0 },
  { sizeof (austrian_german_lk401ag_row2) / sizeof (struct key), 7, austrian_german_lk401ag_row2 },
  { sizeof (austrian_german_lk401ag_row3) / sizeof (struct key), 7, austrian_german_lk401ag_row3 },
  { sizeof (austrian_german_lk401ag_row4) / sizeof (struct key), 7, austrian_german_lk401ag_row4 },
  { sizeof (austrian_german_lk401ag_row5) / sizeof (struct key), 7, austrian_german_lk401ag_row5 },
  { sizeof (austrian_german_lk401ag_row6) / sizeof (struct key), 7, austrian_german_lk401ag_row6 }
};

static struct keyboard austrian_german_lk401ag = {
  "LK401 (Deutsch)",
  sizeof (austrian_german_lk401ag_rows) / sizeof (struct row),
  austrian_german_lk401ag_rows,
  6, 3, 3
};
