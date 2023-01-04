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
/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * This file describes the Sun type 4 keyboard.
 */

static struct key Sun_type4_row0 [] = {
 {8,	"Stop",	"L1",	7, 7,	0,	XK_F11},
 {10,	"Again","L2",	7, 7,	0,	XK_F12},
 {0,	0,	0,	4, 7},
 {12,	"F1",	0,	7, 7,	0,	XK_F1},
 {13,	"F2",	0,	7, 7,	0,	XK_F2},
 {15,	"F3",	0,	7, 7,	0,	XK_F3},
 {17,	"F4",	0,	7, 7,	0,	XK_F4},
 {19,	"F5",	0,	7, 7,	0,	XK_F5},
 {21,	"F6",	0,	7, 7,	0,	XK_F6},
 {23,	"F7",	0,	7, 7,	0,	XK_F7},
 {24,	"F8",	0,	7, 7,	0,	XK_F8},
 {25,	"F9",	0,	7, 7,	0,	XK_F9},
 {14,	"F10",	0,	7, 7,	0,	XK_F10},
 {16,	"F11",	0,	7, 7,	0,	XK_F11},
 {18,	"F12",	0,	7, 7,	0,	XK_F12},
 {95,	"|",	"\\",	7, 7,	0,	XK_backslash,	XK_bar},
 {73,	"Delete",0,	14, 7,	0,	XK_Delete},
 {0,	0,	0,	4, 7},
 {28,	"Pause","R1",	7, 7,	0,	XK_F21,		XK_Pause},
 {29,	"PrSc",	"R2",	7, 7,	0,	XK_F22},
 {30,	"Scroll","Lock",7, 7,	0,	XK_F23},
 {105,	"Num",	"Lock", 7, 7,	0,	XK_Num_Lock,	XK_Num_Lock}
};

static struct key Sun_type4_row1 [] = {
 {32, "Props",	"L3",	7, 7,	0,	XK_F13},
 {33, "Undo",	"L4",	7, 7,	0,	XK_F14},
 {0,	0,	0,	4, 7},
 {36, "Esc",	0,	7, 7,	0,	XK_Escape},
 {37,	"!",	"1",	7, 7,	0,	XK_1,		XK_exclam},
 {38,	"@",	"2",	7, 7,	0,	XK_2,		XK_at},
 {39,	"#",	"3",	7, 7,	0,	XK_3,		XK_numbersign},
 {40,	"$",	"4",	7, 7,	0,	XK_4,		XK_dollar},
 {41,	"%",	"5",	7, 7,	0,	XK_5,		XK_percent},
 {42,	"^",	"6",	7, 7,	0,	XK_6,		XK_asciicircum},
 {43,	"&",	"7",	7, 7,	0,	XK_7,		XK_ampersand},
 {44,	"*",	"8",	7, 7,	0,	XK_8,		XK_asterisk},
 {45,	"(",	"9",	7, 7,	0,	XK_9,		XK_parenleft},
 {46,	")",	"0",	7, 7,	0,	XK_0,		XK_parenright},
 {47,	"_",	"-",	7, 7,	0,	XK_minus,	XK_underscore},
 {48,	"+",	"=",	7, 7,	0,	XK_equal,	XK_plus},
 {50, "Backspace",0,	14, 7,	0,	XK_BackSpace},
 {0,	0,	0,	4, 7},
 {52,	"=",	"R4",	7, 7,	0,	XK_F24,		XK_KP_Equal},
 {53,	"/",	"R5",	7, 7,	0,	XK_F25,		XK_KP_Divide},
 {54,	"*",	"R6",	7, 7,	0,	XK_F26,		XK_KP_Multiply},
 {78,	"-",	0,	7, 7,	0,	XK_KP_Subtract,	XK_KP_Subtract}
};

static struct key Sun_type4_row2 [] = {
 {56,	"Front","L5",		7, 7,	0,	XK_F15},
 {58,	"Copy",	"L6",		7, 7,	0,	XK_F16},
 {0,	0,	0,		4, 7},
 {60,	"Tab",	0,		10, 7,	0,	XK_Tab},
 {61,	"Q",	0,		7, 7,	0,	XK_Q},
 {62,	"W",	0,		7, 7,	0,	XK_W},
 {63,	"E",	0,		7, 7,	0,	XK_E},
 {64,	"R",	0,		7, 7,	0,	XK_R},
 {65,	"T",	0,		7, 7,	0,	XK_T},
 {66,	"Y",	0,		7, 7,	0,	XK_Y},
 {67,	"U",	0,		7, 7,	0,	XK_U},
 {68,	"I",	0,		7, 7,	0,	XK_I},
 {69,	"O",	0,		7, 7,	0,	XK_O},
 {70,	"P",	0,		7, 7,	0,	XK_P},
 {71,	"{",	"[",		7, 7,	0,	XK_bracketleft,	XK_braceleft},
 {72,	"}",	"]",		7, 7,	0,	XK_bracketright,XK_braceright},
 {0,	0,	0,		3, 7},
 {96,	"Return",0,		8, 14,	0,	XK_Return},
 {0,	0,	0,		4, 7},
 {75,	"7",	"Home",		7, 7,	0,	XK_F27,		XK_KP_7},
 {76,	"8",	"UpArrow",	7, 7,	0,	XK_Up,		XK_KP_8},
 {77,	"9",	"PgUp",		7, 7,	0,	XK_F29,		XK_KP_9},
 {132,	"+",	0,		7, 14,	0,	XK_KP_Add,	XK_KP_Add}
};

static struct key Sun_type4_row3 [] = {
 {79,"Open",	"L7",		7, 7,	0,	XK_F17},
 {80,"Paste",	"L8",		7, 7,	0,	XK_F18},
 {0,	0,	0,		4, 7},
 {83,"Control",	0,		13, 7,	ControlMask,	XK_Control_L},
 {84,	"A",	0,		7, 7,	0,	XK_A},
 {85,	"S",	0,		7, 7,	0,	XK_S},
 {86,	"D",	0,		7, 7,	0,	XK_D},
 {87,	"F",	0,		7, 7,	0,	XK_F},
 {88,	"G",	0,		7, 7,	0,	XK_G},
 {89,	"H",	0,		7, 7,	0,	XK_H},
 {90,	"J",	0,		7, 7,	0,	XK_J},
 {91,	"K",	0,		7, 7,	0,	XK_K},
 {92,	"L",	0,		7, 7,	0,	XK_L},
 {93,	":",	";",		7, 7,	0,	XK_semicolon,	XK_colon},
 {94,	"\"",	"'",		7, 7,	0,	XK_apostrophe,	XK_quotedbl},
 {49,	"~",	"`",		7, 7,	0,	XK_grave,	XK_asciitilde},
 {0,	0,	0,		12, 7},
 {98,  "4",	"LeftArrow",	7, 7,	0,	XK_Left,	XK_KP_4},
 {99,  "5",	"R11",		7, 7,	0,	XK_F31,		XK_KP_5},
 {100,  "6",	"RightArrow",	7, 7,	0,	XK_Right,	XK_KP_6}
};

static struct key Sun_type4_row4 [] = {
 {102,	"Find",	"L9",		7, 7,	0,		XK_F19},
 {104,	"Cut",	"L10",		7, 7,	0,		XK_F20},
 {0,	0,	0,		4, 7},
 {106,	"Shift",0,		16, 7,	ShiftMask,	XK_Shift_L},
 {107,	"Z",	0,		7, 7,	0,		XK_Z},
 {108,	"X",	0,		7, 7,	0,		XK_X},
 {109,	"C",	0,		7, 7,	0,		XK_C},
 {110,	"V",	0,		7, 7,	0,		XK_V},
 {111,	"B",	0,		7, 7,	0,		XK_B},
 {112,	"N",	0,		7, 7,	0,		XK_N},
 {113,	"M",	0,		7, 7,	0,		XK_M},
 {114,	"<",	",",		7, 7,	0,		XK_comma, XK_less},
 {115,	">",	".",		7, 7,	0,		XK_period,XK_greater},
 {116,	"?",	"/",		7, 7,	0,		XK_slash, XK_question},
 {117,	"Shift",0,		12, 7,	ShiftMask,	XK_Shift_R},
 {118,	"Line ","Feed",		7, 7,	0,		XK_Linefeed},
 {0,	0,	0,		4, 7},
 {119,	"1",	"End",		7, 7,	0,	XK_R13,		XK_KP_1},
 {120,	"2",	"DownArrow",	7, 7,	0,	XK_Down,	XK_KP_2},
 {121,  "3",	"PgDn",		7, 7,	0,	XK_F35,		XK_KP_3},
 {97,"Enter",	0,		7, 14,	0,	XK_KP_Enter,	XK_KP_Enter}
};

static struct key Sun_type4_row5 [] = {
 {125,	"Help",	0,		14, 7,	0,		XK_Help},
 {0,	0,	0,		4, 7},
 {126,	"Caps","Lock",		7, 7,	LockMask,	XK_Caps_Lock},
 {26,	"Alt",	0,		7, 7,	0,		XK_Alt_L},
 {127,	"<>",	0,		7, 7,	Mod1Mask,	XK_Meta_L},
 {128,	" ",	0,		63, 7,	0,		XK_space},
 {129,	"<>",	0,		7, 7,	Mod1Mask,	XK_Meta_R},
 {74,   "Com ",	"pose",		7, 7,	0,		XK_Multi_key},
 {20,	"Alt",	"Graph",	7, 7},
 {0,	0,	0,		4, 7},
 {101,	"0",	"Ins",		14, 7,	0,	XK_Insert,	XK_KP_0},
 {57,	".",	"Del",		7, 7,	0,	XK_Delete,	XK_KP_Decimal}
};

static struct row Sun_type4_rows [] = {
  { sizeof (Sun_type4_row0) / sizeof (struct key), 7, Sun_type4_row0 },
  { sizeof (Sun_type4_row1) / sizeof (struct key), 7, Sun_type4_row1 },
  { sizeof (Sun_type4_row2) / sizeof (struct key), 7, Sun_type4_row2 },
  { sizeof (Sun_type4_row3) / sizeof (struct key), 7, Sun_type4_row3 },
  { sizeof (Sun_type4_row4) / sizeof (struct key), 7, Sun_type4_row4 },
  { sizeof (Sun_type4_row5) / sizeof (struct key), 7, Sun_type4_row5 },
};

static struct keyboard Sun_type4 = {
  "Sun type4",
  sizeof (Sun_type4_rows) / sizeof (struct row),
  Sun_type4_rows,
  6, 3, 3
};
