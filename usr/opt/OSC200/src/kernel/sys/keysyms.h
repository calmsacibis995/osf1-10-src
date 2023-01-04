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
 * @(#)$RCSfile: keysyms.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/02 15:31:41 $
 */
/*
 * keysyms defined on default keyboard
 * This really should just be an include of /usr/include/X11/keysym.h
 * But this isn't in the base system (at least at the moment), so
 * we have to kludge around it.
 */

#ifndef _SYS_KEYSYMS_H_
#define _SYS_KEYSYMS_H_

#define XK_F1			0xFFBE
#define XK_F2			0xFFBF
#define XK_F3			0xFFC0
#define XK_F4			0xFFC1
#define XK_F5			0xFFC2
#define XK_F6			0xFFC3
#define XK_F7			0xFFC4
#define XK_F8			0xFFC5
#define XK_F9			0xFFC6
#define XK_F10			0xFFC7
#define XK_F11			0xFFC8
#define XK_L1			0xFFC8
#define XK_F12			0xFFC9
#define XK_L2			0xFFC9
#define XK_F13			0xFFCA
#define XK_L3			0xFFCA
#define XK_F14			0xFFCB
#define XK_L4			0xFFCB
#define XK_F15			0xFFCC
#define XK_L5			0xFFCC
#define XK_F16			0xFFCD
#define XK_L6			0xFFCD
#define XK_F17			0xFFCE
#define XK_L7			0xFFCE
#define XK_F18			0xFFCF
#define XK_L8			0xFFCF
#define XK_F19			0xFFD0
#define XK_L9			0xFFD0
#define XK_F20			0xFFD1
#define XK_L10			0xFFD1
#define XK_F21			0xFFD2
#define XK_R1			0xFFD2
#define XK_F22			0xFFD3
#define XK_R2			0xFFD3
#define XK_F23			0xFFD4
#define XK_R3			0xFFD4
#define XK_F24			0xFFD5
#define XK_R4			0xFFD5
#define XK_F25			0xFFD6
#define XK_R5			0xFFD6
#define XK_F26			0xFFD7
#define XK_R6			0xFFD7
#define XK_F27			0xFFD8
#define XK_R7			0xFFD8
#define XK_F28			0xFFD9
#define XK_R8			0xFFD9
#define XK_F29			0xFFDA
#define XK_R9			0xFFDA
#define XK_F30			0xFFDB
#define XK_R10			0xFFDB
#define XK_F31			0xFFDC
#define XK_R11			0xFFDC
#define XK_F32			0xFFDD
#define XK_R12			0xFFDD
#define XK_R13			0xFFDE
#define XK_F33			0xFFDE
#define XK_F34			0xFFDF
#define XK_R14			0xFFDF
#define XK_F35			0xFFE0
#define XK_R15			0xFFE0

#define XK_Help			0xFF6A	/* Help, ? */
#define XK_Menu			0xFF67
#define XK_Find			0xFF68	/* Find, search */
#define XK_Insert		0xFF63	/* Insert, insert here */
#define DXK_Remove		0x1000FF00
#define XK_Select		0xFF60	/* Select, mark */
#define XK_Prior		0xFF55	/* Prior, previous, Page Up */
#define XK_Next			0xFF56	/* Next, Page Down */
#define XK_Print		0xFF61	/* Print */
#define XK_Home			0xFF50	/* Home */
#define XK_End			0xFF57	/* End */
#define XK_Num_Lock		0XFF7F	/* numeric lock */

/* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

#define XK_KP_Space		0xFF80	/* space */
#define XK_KP_Tab		0xFF89
#define XK_KP_Enter		0xFF8D	/* enter */
#define XK_KP_F1		0xFF91	/* PF1, KP_A, ... */
#define XK_KP_F2		0xFF92
#define XK_KP_F3		0xFF93
#define XK_KP_F4		0xFF94
#define XK_KP_Equal		0xFFBD	/* equals */
#define XK_KP_Multiply		0xFFAA
#define XK_KP_Add		0xFFAB
#define XK_KP_Separator		0xFFAC	/* separator, often comma */
#define XK_KP_Subtract		0xFFAD
#define XK_KP_Decimal		0xFFAE
#define XK_KP_Divide		0xFFAF

#define XK_KP_0			0xFFB0
#define XK_KP_1			0xFFB1
#define XK_KP_2			0xFFB2
#define XK_KP_3			0xFFB3
#define XK_KP_4			0xFFB4
#define XK_KP_5			0xFFB5
#define XK_KP_6			0xFFB6
#define XK_KP_7			0xFFB7
#define XK_KP_8			0xFFB8
#define XK_KP_9			0xFFB9

#define XK_Left			0xFF51	/* Move left, left arrow */
#define XK_Up			0xFF52	/* Move up, up arrow */
#define XK_Right		0xFF53	/* Move right, right arrow */
#define XK_Down			0xFF54	/* Move down, down arrow */

/* Modifiers */

#define XK_Shift_L		0xFFE1	/* Left shift */
#define XK_Shift_R		0xFFE2	/* Right shift */
#define XK_Control_L		0xFFE3	/* Left control */
#define XK_Control_R		0xFFE4	/* Right control */
#define XK_Caps_Lock		0xFFE5	/* Caps lock */
#define XK_Shift_Lock		0xFFE6	/* Shift lock */

#define XK_Meta_L		0xFFE7	/* Left meta */
#define XK_Meta_R		0xFFE8	/* Right meta */
#define XK_Alt_L		0xFFE9	/* Left alt */
#define XK_Alt_R		0xFFEA	/* Right alt */
#define XK_Super_L		0xFFEB	/* Left super */
#define XK_Super_R		0xFFEC	/* Right super */
#define XK_Hyper_L		0xFFED	/* Left hyper */
#define XK_Hyper_R		0xFFEE	/* Right hyper */

/* International & multi-key character composition */

#define XK_Multi_key		0xFF20  /* Multi-key character compose */

/* Japanese keyboard support */

#define XK_Kanji                0xFF21  /* Kanji, Kanji convert */
#define XK_Muhenkan             0xFF22  /* Cancel Conversion */
#define XK_Henkan_Mode          0xFF23  /* Start/Stop Conversion */
#define XK_Henkan               0xFF23  /* Alias for Henkan_Mode */
#define XK_Romaji               0xFF24  /* to Romaji */
#define XK_Hiragana             0xFF25  /* to Hiragana */
#define XK_Katakana             0xFF26  /* to Katakana */
#define XK_Hiragana_Katakana    0xFF27  /* Hiragana/Katakana toggle */
#define XK_Zenkaku              0xFF28  /* to Zenkaku */
#define XK_Hankaku              0xFF29  /* to Hankaku */
#define XK_Zenkaku_Hankaku      0xFF2A  /* Zenkaku/Hankaku toggle */
#define XK_Touroku              0xFF2B  /* Add to Dictionary */
#define XK_Massyo               0xFF2C  /* Delete from Dictionary */
#define XK_Kana_Lock            0xFF2D  /* Kana Lock */
#define XK_Kana_Shift           0xFF2E  /* Kana Shift */
#define XK_Eisu_Shift           0xFF2F  /* Alphanumeric Shift */
#define XK_Eisu_toggle          0xFF30  /* Alphanumeric toggle */

/*
 *  Latin 1
 *  Byte 3 = 0
 */
#define XK_space               0x020
#define XK_exclam              0x021
#define XK_quotedbl            0x022
#define XK_numbersign          0x023
#define XK_dollar              0x024
#define XK_percent             0x025
#define XK_ampersand           0x026
#define XK_apostrophe          0x027
#define XK_quoteright          0x027	/* deprecated */
#define XK_parenleft           0x028
#define XK_parenright          0x029
#define XK_asterisk            0x02a
#define XK_plus                0x02b
#define XK_comma               0x02c
#define XK_minus               0x02d
#define XK_period              0x02e
#define XK_slash               0x02f
#define XK_0                   0x030
#define XK_1                   0x031
#define XK_2                   0x032
#define XK_3                   0x033
#define XK_4                   0x034
#define XK_5                   0x035
#define XK_6                   0x036
#define XK_7                   0x037
#define XK_8                   0x038
#define XK_9                   0x039
#define XK_colon               0x03a
#define XK_semicolon           0x03b
#define XK_less                0x03c
#define XK_equal               0x03d
#define XK_greater             0x03e
#define XK_question            0x03f
#define XK_at                  0x040
#define XK_A                   0x041
#define XK_B                   0x042
#define XK_C                   0x043
#define XK_D                   0x044
#define XK_E                   0x045
#define XK_F                   0x046
#define XK_G                   0x047
#define XK_H                   0x048
#define XK_I                   0x049
#define XK_J                   0x04a
#define XK_K                   0x04b
#define XK_L                   0x04c
#define XK_M                   0x04d
#define XK_N                   0x04e
#define XK_O                   0x04f
#define XK_P                   0x050
#define XK_Q                   0x051
#define XK_R                   0x052
#define XK_S                   0x053
#define XK_T                   0x054
#define XK_U                   0x055
#define XK_V                   0x056
#define XK_W                   0x057
#define XK_X                   0x058
#define XK_Y                   0x059
#define XK_Z                   0x05a
#define XK_bracketleft         0x05b
#define XK_backslash           0x05c
#define XK_bracketright        0x05d
#define XK_asciicircum         0x05e
#define XK_underscore          0x05f
#define XK_grave               0x060
#define XK_quoteleft           0x060	/* deprecated */
#define XK_a                   0x061
#define XK_b                   0x062
#define XK_c                   0x063
#define XK_d                   0x064
#define XK_e                   0x065
#define XK_f                   0x066
#define XK_g                   0x067
#define XK_h                   0x068
#define XK_i                   0x069
#define XK_j                   0x06a
#define XK_k                   0x06b
#define XK_l                   0x06c
#define XK_m                   0x06d
#define XK_n                   0x06e
#define XK_o                   0x06f
#define XK_p                   0x070
#define XK_q                   0x071
#define XK_r                   0x072
#define XK_s                   0x073
#define XK_t                   0x074
#define XK_u                   0x075
#define XK_v                   0x076
#define XK_w                   0x077
#define XK_x                   0x078
#define XK_y                   0x079
#define XK_z                   0x07a
#define XK_braceleft           0x07b
#define XK_bar                 0x07c
#define XK_braceright          0x07d
#define XK_asciitilde          0x07e

/*
 * TTY Functions, cleverly chosen to map to ascii, for convenience of
 * programming, but could have been arbitrary (at the cost of lookup
 * tables in client code.
 */

#define XK_BackSpace		0xFF08	/* back space, back char */
#define XK_Tab			0xFF09
#define XK_Linefeed		0xFF0A	/* Linefeed, LF */
#define XK_Clear		0xFF0B
#define XK_Return		0xFF0D	/* Return, enter */
#define XK_Pause		0xFF13	/* Pause, hold */
#define XK_Scroll_Lock		0xFF14
#define XK_Escape		0xFF1B
#define XK_Delete		0xFFFF	/* Delete, rubout */

/* this table defines which keycodes have modifier bits associated with them */

/* Key masks. Used as modifiers to GrabButton and GrabKey,
   results of QueryPointer,
   state in various key-, mouse-, and button-related events. */

#ifndef ShiftMask
#define ShiftMask		(1<<0)
#define LockMask		(1<<1)
#define ControlMask		(1<<2)
#define Mod1Mask		(1<<3)
#define Mod2Mask		(1<<4)
#define Mod3Mask		(1<<5)
#define Mod4Mask		(1<<6)
#define Mod5Mask		(1<<7)
#endif


#endif
