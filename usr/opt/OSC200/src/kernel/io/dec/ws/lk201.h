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
 * @(#)$RCSfile: lk201.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/10/19 21:57:08 $
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

#ifndef _LK201_H_
#define _LK201_H_
/* 
 * command keycodes for Digital LK200/LK400 series keyboards.
 */

#define LK_POWER_UP	0x01		/* initial byte of power up code */
#define LK_UPDOWN	0x86		/* bits for setting lk201 modes */
#define LK_AUTODOWN	0x82
#define LK_DOWN		0x80
#define LK_DEFAULTS	0xd3		/* reset (some) default settings*/
#define LK_AR_ENABLE	0xe3		/* global auto repeat enable	*/
#define LK_CL_ENABLE	0x1b		/* keyclick enable		*/
#define LK_CL_DISABLE	0x99		/* keyclick disable		*/
#define LK_CL_SOUND	0x9f		/* sound keyclick		*/
#define LK_KBD_ENABLE	0x8b		/* keyboard enable		*/
#define LK_BELL_ENABLE	0x23		/* the bell			*/
#define LK_BELL_DISABLE 0xa1		/* disable bell entirely	*/
#define LK_LED_ENABLE	0x13		/* light led			*/
#define LK_LED_DISABLE	0x11		/* turn off led			*/
#define LK_RING_BELL	0xa7		/* ring keyboard bell		*/
#define LK_LED_WAIT	0x81		/* LK201 only			*/
#define LK_LED_COMPOSE	0x82		/* LK201 only			*/
#define LK_LED_NUMLOCK	0x82		/* LK443/444 only		*/
#define LK_LED_LOCK	0x84		/* all LKXXX			*/
#define LK_LED_HOLD	0x88		/* all LKXXX			*/
#define LK_LED_ALL	0x8f		/* all possible LEDs		*/
#define LK_KDOWN_ERROR	0x3d		/* key down on powerup error	*/
#define LK_POWER_ERROR	0x3e		/* keyboard failure on pwrup tst*/
#define LK_OUTPUT_ERROR 0xb5		/* keystrokes lost during inhbt */
#define LK_INPUT_ERROR	0xb6		/* garbage command to keyboard	*/
#define LK_LOWEST	0x55		/* lowest significant keycode	*/
					/*  this allows LK443 now...	*/
#define LK_DIV6_START	0xad		/* start of div 6		*/
#define LK_DIV5_END	0xb2		/* end of div 5			*/
#define LK_ENABLE_401	0xe9		/* turn on LK401 mode		*/
#define LK_MODE_CHANGE	0xba		/* mode change ack		*/


#define MIN_LK201_KEY            85	/* now allows 443/444		*/
#define MAX_LK201_KEY           251
#define LK201_GLYPHS_PER_KEY      2

/* the keys themselves */

#define KEY_443_ESC		 85
#define KEY_F1			 86
#define KEY_F2			 87
#define KEY_F3			 88
#define KEY_F4			 89
#define KEY_F5			 90
#define KEY_F6			100
#define KEY_F7			101
#define KEY_F8			102
#define KEY_F9			103
#define KEY_F10			104
#define KEY_F11			113
#define KEY_F12			114
#define KEY_F13			115
#define KEY_F14			116
#define KEY_HELP		124
#define KEY_MENU		125
#define KEY_F17			128
#define KEY_F18			129
#define KEY_F19			130
#define KEY_F20			131
#define KEY_FIND		138
#define KEY_INSERT_HERE		139
#define KEY_REMOVE		140
#define KEY_SELECT		141
#define KEY_PREV_SCREEN		142
#define KEY_NEXT_SCREEN		143
#define KEY_KP_0		146	/* key pad */
#define KEY_KP_PERIOD		148	/* key pad */
#define KEY_KP_ENTER		149	/* key pad */
#define KEY_KP_1		150	/* key pad */
#define KEY_KP_2		151	/* key pad */
#define KEY_KP_3		152	/* key pad */
#define KEY_KP_4		153	/* key pad */
#define KEY_KP_5		154	/* key pad */
#define KEY_KP_6		155	/* key pad */
#define KEY_KP_COMMA		156	/* key pad */
#define KEY_KP_7		157	/* key pad */
#define KEY_KP_8		158	/* key pad */
#define KEY_KP_9		159	/* key pad */
#define KEY_KP_HYPHEN		160
#define KEY_KP_PF1		161
#define KEY_KP_NUMLOCK		161	/* 443/444 */
#define KEY_KP_PF2		162
#define KEY_KP_PF3		163
#define KEY_KP_PF4		164
#define KEY_LEFT		167
#define KEY_RIGHT		168
#define KEY_DOWN		169
#define KEY_UP			170
#define KEY_SHIFT_R		171	/* LK401 */
#define KEY_ALT_L		172	/* LK401 */
#define KEY_COMPOSE_R		173	/* LK401 */
#define KEY_CTRL_R		173	/* 443/444 */
#define KEY_SHIFT		174
#define KEY_SHIFT_L		174
#define KEY_CTRL		175
#define KEY_LOCK		176
#define KEY_COMPOSE		177
#define KEY_APPLE		177
#define KEY_META		177
#define KEY_ALT_R		178	/* LK401 */
#define KEY_DELETE		188
#define KEY_RETURN		189
#define KEY_TAB			190
#define KEY_TILDE		191
#define KEY_TR_1		192	/* Top Row */
#define KEY_Q			193
#define KEY_A			194
#define KEY_Z			195
#define KEY_TR_2		197
#define KEY_W			198
#define KEY_S			199
#define KEY_X			200
#define KEY_LANGLE_RANGLE	201	/* xxx */
#define KEY_TR_3		203
#define KEY_E			204
#define KEY_D			205
#define KEY_C			206
#define KEY_TR_4		208
#define KEY_R			209
#define KEY_F			210
#define KEY_V			211
#define KEY_SPACE		212
#define KEY_TR_5		214
#define KEY_T			215
#define KEY_G			216
#define KEY_B			217
#define KEY_TR_6		219
#define KEY_Y			220
#define KEY_H			221
#define KEY_N			222
#define KEY_TR_7		224
#define KEY_U			225
#define KEY_J			226
#define KEY_M			227
#define KEY_TR_8		229
#define KEY_I			230
#define KEY_K			231
#define KEY_COMMA		232	/* xxx */
#define KEY_TR_9		234
#define KEY_O			235
#define KEY_L			236
#define KEY_PERIOD		237	/* xxx */
#define KEY_TR_0		239
#define KEY_P			240
#define KEY_SEMICOLON		242	/* xxx */
#define KEY_QMARK		243
#define KEY_PLUS		245	/* xxx */
#define KEY_RBRACE		246
#define KEY_VBAR		247	/* xxx */
#define KEY_UBAR		249	/* xxx */
#define KEY_LBRACE		250
#define KEY_QUOTE		251

#ifndef KEY_ESC
#define KEY_ESC KEY_F11
#endif

/*
 * Keycodes for special keys and functions
 */

#define SHIFT	0xae
#define SHIFT_RIGHT 0xab
#define LOCK	0xb0
#define REPEAT	0xb4
#define CNTRL	0xaf
#define ALLUP	0xb3
#define HOLD	0x56

#ifdef KERNEL
int lk201_getc();
int lk201_putc();
caddr_t lk201_init_closure();
void lk201_up_down_mode();
void lk201_reset_keyboard();
void lk201_ring_bell();
int lk201_set_keyboard_control();
void lk201_autorepeat();
void lk201_keyboard_event();
void lk201_keyboard_char();
void lk201_set_leds();
#endif

#endif
