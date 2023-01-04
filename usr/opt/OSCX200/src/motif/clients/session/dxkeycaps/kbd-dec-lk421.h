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
 * @(#)$RCSfile: kbd-dec-lk421.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/11/09 17:16:09 $
 */
/* xkeycaps
 *
 * This file describes the DEC LK421 keyboard.
 */

static struct key DEC_LK421_row0 [] = {
 {0,	0,	0,	2, 7},
 {86,	"F1",	NULL,	7, 7,	0L,	XK_F1},
 {87,	"F2",	NULL,	7, 7,	0L,	XK_F2},
 {88,	"F3",	NULL,	7, 7,	0L,	XK_F3},
 {89,	"F4",	NULL,	7, 7,	0L,	XK_F4},
 {90,	"F5",	NULL,	7, 7,	0L,	XK_F5},
 {0,	NULL,	NULL,	5, 7},
 {100,	"F6",	NULL,	7, 7,	0L,	XK_F6},
 {101,	"F7",	NULL,	7, 7,	0L,	XK_F7},
 {102,	"F8",	NULL,	7, 7,	0L,	XK_F8},
 {103,	"F9",	NULL,	7, 7,	0L,	XK_F9},
 {104,	"F10",	NULL,	7, 7,	0L,	XK_F10},
 {0,	NULL,	NULL,	6, 7},
 {167,"LeftArrow", NULL,	8, 7,	0,	XK_Left},
 {169,"DownArrow", NULL,	8, 7,	0,	XK_Down},
 {170,"UpArrow",   NULL,	8, 7,	0,	XK_Right},
 {168,"RightArrow",NULL,	8, 7,	0,	XK_Right},
};

static struct key DEC_LK421_row1 [] = {
 {0,	0,	0,	2, 7},
 {191,	"ESC",	0,	11, 7,	0,	XK_Escape},
 {192,	"!",	"1",	7, 7,	0,	XK_1,		XK_exclam},
 {197,	"@",	"2",	7, 7,	0,	XK_2,		XK_at},
 {203,	"#",	"3",	7, 7,	0,	XK_3,		XK_numbersign},
 {208,	"$",	"4",	7, 7,	0,	XK_4,		XK_dollar},
 {214,	"%",	"5",	7, 7,	0,	XK_5,		XK_percent},
 {219,	"^",	"6",	7, 7,	0,	XK_6,		XK_asciicircum},
 {224,	"&",	"7",	7, 7,	0,	XK_7,		XK_ampersand},
 {229,	"*",	"8",	7, 7,	0,	XK_8,		XK_asterisk},
 {234,	"(",	"9",	7, 7,	0,	XK_9,		XK_parenleft},
 {239,	")",	"0",	7, 7,	0,	XK_0,		XK_parenright},
 {249,	"_",	"-",	7, 7,	0,	XK_minus,	XK_underscore},
 {245,	"+",	"=",	7, 7,	0,	XK_equal,	XK_plus},
 {188,	"<X|",	0,	18, 7,	0,	XK_Delete},
};
 
static struct key DEC_LK421_row2 [] = {
 {0,	0,	0,	2, 7},
 {190,	"Tab",	0,	15, 7,	0,	XK_Tab},
 {193,	"Q",	0,	7, 7,	0,	XK_Q},
 {198,	"W",	0,	7, 7,	0,	XK_W},
 {204,	"E",	0,	7, 7,	0,	XK_E},
 {209,	"R",	0,	7, 7,	0,	XK_R},
 {215,	"T",	0,	7, 7,	0,	XK_T},
 {220,	"Y",	0,	7, 7,	0,	XK_Y},
 {225,	"U",	0,	7, 7,	0,	XK_U},
 {230,	"I",	0,	7, 7,	0,	XK_I},
 {235,	"O",	0,	7, 7,	0,	XK_O},
 {240,	"P",	0,	7, 7,	0,	XK_P},
 {250,	"{",	"[",	7, 7,	0,	XK_bracketleft,	XK_braceleft},
 {246,	"}",	"]",	7, 7,	0,	XK_bracketright,XK_braceright},
 {247,	"|",	"\\",	7, 7,	0,	XK_backslash,	XK_bar},
 {201,	"~",	"`",	7, 7,	0,	XK_grave,	XK_asciitilde},
};

static struct key DEC_LK421_row3 [] = {
 {175,	"Ctrl",	0,	17, 7,	ControlMask,	XK_Control_L},
 {194,	"A",	0,	7, 7,	0,	XK_A},
 {199,	"S",	0,	7, 7,	0,	XK_S},
 {205,	"D",	0,	7, 7,	0,	XK_D},
 {210,	"F",	0,	7, 7,	0,	XK_F},
 {216,	"G",	0,	7, 7,	0,	XK_G},
 {221,	"H",	0,	7, 7,	0,	XK_H},
 {226,	"J",	0,	7, 7,	0,	XK_J},
 {231,	"K",	0,	7, 7,	0,	XK_K},
 {236,	"L",	0,	7, 7,	0,	XK_L},
 {242,	":",	";",	7, 7,	0,	XK_semicolon,	XK_colon},
 {251,	"\"",	"'",	7, 7,	0,	XK_apostrophe,	XK_quotedbl},
 {189,	"Return",0,	19, 7,	0,	XK_Return},
};

static struct key DEC_LK421_row4 [] = {
 {174,	"Shift",0,	20, 7,	ShiftMask,		XK_Shift_L},
 {195,	"Z",	0,	7, 7,	0,	XK_Z},
 {200,	"X",	0,	7, 7,	0,	XK_X},
 {206,	"C",	0,	7, 7,	0,	XK_C},
 {211,	"V",	0,	7, 7,	0,	XK_V},
 {217,	"B",	0,	7, 7,	0,	XK_B},
 {222,	"N",	0,	7, 7,	0,	XK_N},
 {227,	"M",	0,	7, 7,	0,	XK_M},
 {232,	"<",	",",	7, 7,	0,	XK_comma,	XK_less},
 {237,	">",	".",	7, 7,	0,	XK_period,	XK_greater},
 {243,	"?",	"/",	7, 7,	0,	XK_slash,	XK_question},
 {171,	"Shift",0,	23, 7,		ShiftMask,	XK_Shift_R},
};

static struct key DEC_LK421_row5 [] = {
 {0,	NULL,	NULL,		1, 7},
 {0,	"Ext-",	"end",		7, 7,	0L,		0}, /* extend */
 {177, 	"Com-", "pose",		7, 7,	Mod1Mask, XK_Multi_key, XK_Meta_L},
 {172, 	"Alt",	"Function",	13, 7,	Mod2Mask, XK_Alt_L},
 {212,	" ",	NULL,		51, 7,	0L,	  XK_space},
 {178, 	"Alt",	"Function",	12, 7,	Mod2Mask, XK_Alt_R},
 {173, 	"Compose","Character",	12, 7,	Mod1Mask, XK_Multi_key, XK_Meta_R},
};

static struct row DEC_LK421_rows [] = {
  { sizeof (DEC_LK421_row0) / sizeof (struct key), 7, DEC_LK421_row0 },
  { sizeof (DEC_LK421_row1) / sizeof (struct key), 7, DEC_LK421_row1 },
  { sizeof (DEC_LK421_row2) / sizeof (struct key), 7, DEC_LK421_row2 },
  { sizeof (DEC_LK421_row3) / sizeof (struct key), 7, DEC_LK421_row3 },
  { sizeof (DEC_LK421_row4) / sizeof (struct key), 7, DEC_LK421_row4 },
  { sizeof (DEC_LK421_row5) / sizeof (struct key), 7, DEC_LK421_row5 }
};

static struct keyboard DEC_LK421 = {
  "LK421",
  sizeof (DEC_LK421_rows) / sizeof (struct row),
  DEC_LK421_rows,
  6, 3, 3
};
