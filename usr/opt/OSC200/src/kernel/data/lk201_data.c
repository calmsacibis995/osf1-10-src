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
 *	@(#)$RCSfile: lk201_data.c,v $ $Revision: 1.2.7.5 $ (DEC) $Date: 1993/11/17 17:27:44 $
 */ 
/*
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

#include <sys/types.h>
#include <kern/queue.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/keysyms.h>

#include <io/dec/ws/lk201.h>

#ifdef BINARY

extern ws_keyboard_state	lk201_softc[];
extern ws_keyboard		lk201_keyboard;
extern unsigned char		lk201_keycodes[];
extern unsigned int		lk201_keysyms[];
extern ws_keycode_modifiers	lk201_modifiers[];
extern ws_keyboard_definition	lk201_definition;
extern ws_keycode_modifiers	lk443_modifiers[];
extern ws_keyboard_definition	lk443_definition;

extern unsigned short q_key[], q_shift_key[];

extern char lk201_name[];
extern char lk401_name[];
extern char lk421_name[];
extern char lk443_name[];


#else /* BINARY */

/*
 * Note that if a keycode appears additional times, it defines further
 * symbols on the same keycode.  DDX translates this to the appropriate
 * data structure.  All this is to save bytes in the kernel.
 * WARNING: keycodes and keysym tables must be EXACTLY in sync!
 */
unsigned char lk201_keycodes[] = {
    KEY_443_ESC,	/* added for LK443/LK444 */
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_ESC,		/* escape is primary, due to previous stupidity...*/
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,

    KEY_HELP,
    KEY_MENU,

    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,

    KEY_FIND,
    KEY_INSERT_HERE,
    KEY_REMOVE,
    KEY_SELECT,
    KEY_PREV_SCREEN,
    KEY_NEXT_SCREEN,

    KEY_KP_0,
    KEY_KP_PERIOD,
    KEY_KP_ENTER,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_COMMA,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_HYPHEN,
    KEY_KP_PF1,
    KEY_KP_PF2,
    KEY_KP_PF3,
    KEY_KP_PF4,

    KEY_LEFT,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_UP,

    KEY_SHIFT,
    KEY_SHIFT_R,
    KEY_CTRL,
    KEY_LOCK,
    KEY_COMPOSE,
    KEY_COMPOSE_R,
    KEY_ALT_L,
    KEY_ALT_L,
    KEY_ALT_R,
    KEY_ALT_R,
    KEY_DELETE,
    KEY_RETURN,
    KEY_TAB,

    KEY_TILDE,
    KEY_TILDE,

    KEY_TR_1,
    KEY_TR_1,
    KEY_Q,
    KEY_A,
    KEY_Z,

    KEY_TR_2,
    KEY_TR_2,

    KEY_W,
    KEY_S,
    KEY_X,

    KEY_LANGLE_RANGLE,
    KEY_LANGLE_RANGLE,

    KEY_TR_3,
    KEY_TR_3,

    KEY_E,
    KEY_D,
    KEY_C,

    KEY_TR_4,
    KEY_TR_4,

    KEY_R,
    KEY_F,
    KEY_V,
    KEY_SPACE,

    KEY_TR_5,
    KEY_TR_5,

    KEY_T,
    KEY_G,
    KEY_B,

    KEY_TR_6,
    KEY_TR_6,

    KEY_Y,
    KEY_H,
    KEY_N,

    KEY_TR_7,
    KEY_TR_7,

    KEY_U,
    KEY_J,
    KEY_M,

    KEY_TR_8,
    KEY_TR_8,

    KEY_I,
    KEY_K,

    KEY_COMMA,
    KEY_COMMA,

    KEY_TR_9,
    KEY_TR_9,

    KEY_O,
    KEY_L,

    KEY_PERIOD,
    KEY_PERIOD,

    KEY_TR_0,
    KEY_TR_0,

    KEY_P,

    KEY_SEMICOLON,
    KEY_SEMICOLON,

    KEY_QMARK,
    KEY_QMARK,

    KEY_PLUS,
    KEY_PLUS,

    KEY_RBRACE,
    KEY_RBRACE,

    KEY_VBAR,
    KEY_VBAR,

    KEY_UBAR,
    KEY_UBAR,

    KEY_LBRACE,
    KEY_LBRACE,

    KEY_QUOTE,
    KEY_QUOTE,

};

unsigned int lk201_keysyms[] = {
    XK_Escape,
    XK_F1,
    XK_F2,
    XK_F3,
    XK_F4,
    XK_F5,
    XK_F6,
    XK_F7,
    XK_F8,
    XK_F9,
    XK_F10,
    XK_Escape,
    XK_F11,
    XK_F12,
    XK_F13,
    XK_F14,

    XK_Help,
    XK_Menu,

    XK_F17,
    XK_F18,
    XK_F19,
    XK_F20,

    XK_Find,
    XK_Insert,
    DXK_Remove,
    XK_Select,
    XK_Prior,
    XK_Next,

    XK_KP_0,
    XK_KP_Decimal,
    XK_KP_Enter,
    XK_KP_1,
    XK_KP_2,
    XK_KP_3,
    XK_KP_4,
    XK_KP_5,
    XK_KP_6,
    XK_KP_Separator,
    XK_KP_7,
    XK_KP_8,
    XK_KP_9,
    XK_KP_Subtract,
    XK_KP_F1,
    XK_KP_F2,
    XK_KP_F3,
    XK_KP_F4,

    XK_Left,
    XK_Right,
    XK_Down,
    XK_Up,

    XK_Shift_L,
    XK_Shift_R,
    XK_Control_L,
    XK_Caps_Lock,
    XK_Multi_key,
    XK_Multi_key,
    XK_Alt_L,
    XK_Meta_L,
    XK_Alt_R,
    XK_Meta_R,
    XK_Delete,
    XK_Return,
    XK_Tab,

    XK_quoteleft,
    XK_asciitilde,

    XK_1,
    XK_exclam,
    XK_Q,
    XK_A,
    XK_Z,

    XK_2,
    XK_at,

    XK_W,
    XK_S,
    XK_X,

    XK_less,
    XK_greater,

    XK_3,
    XK_numbersign,

    XK_E,
    XK_D,
    XK_C,

    XK_4,
    XK_dollar,

    XK_R,
    XK_F,
    XK_V,
    XK_space,

    XK_5,
    XK_percent,

    XK_T,
    XK_G,
    XK_B,

    XK_6,
    XK_asciicircum,

    XK_Y,
    XK_H,
    XK_N,

    XK_7,
    XK_ampersand,

    XK_U,
    XK_J,
    XK_M,

    XK_8,
    XK_asterisk,

    XK_I,
    XK_K,

    XK_comma,
    XK_less,

    XK_9,
    XK_parenleft,

    XK_O,
    XK_L,

    XK_period,
    XK_greater,

    XK_0,
    XK_parenright,

    XK_P,

    XK_semicolon,
    XK_colon,

    XK_slash,
    XK_question,

    XK_equal,
    XK_plus,

    XK_bracketright,
    XK_braceright,

    XK_backslash,
    XK_bar,

    XK_minus,
    XK_underscore,

    XK_bracketleft,
    XK_braceleft,

    XK_quoteright,
    XK_quotedbl,

};

/* 
 * derived from qfont.c	2.2	(ULTRIX/OSF)	12/4/90";
 */

/*
 * The following tables are used to translate LK201 key strokes
 * into ascii characters. The tables also support the special
 * keys, by providing a flag indicating when use of "special_keys"
 * table (data/ws_data.c) is necessary.
 */

unsigned short q_key[]={
	 0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*   0 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*   8 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00	/*  16 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  24 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  32 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  40 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  48 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  56 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  64 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  72 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x1b  ,0x101 ,0x102 /*  80 */
	,0x103 ,0x104 ,0x105 ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  88 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x106 ,0x107 ,0x108 ,0x109 /*  96 */
	,0x10a ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /* 104 */ 
	,0x00  ,0x1b  ,0x08  ,0x0a  ,0x10b ,0x00  ,0x00  ,0x00  /* 112 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x10c ,0x10d ,0x00  ,0x00  /* 120 */
	,0x10e ,0x10f ,0x110 ,0x111 ,0x00  ,0x00  ,0x00  ,0x00  /* 128 */
	,0x00  ,0x00  ,0x112 ,0x113 ,0x114 ,0x115 ,0x116 ,0x117 /* 136 */
	,0x00  ,0x00  ,0x120 ,0x00  ,0x121 ,0x122 ,0x123 ,0x124 /* 144 */
	,0x125 ,0x126 ,0x127 ,0x128 ,0x129 ,0x12a ,0x12b ,0x12c /* 152 */
	,0x12d ,0x118 ,0x119 ,0x11a ,0x11b ,0x00  ,0x00  ,0x11c /* 160 */
	,0x11d ,0x11e ,0x11f ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /* 168 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /* 176 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x7f  ,'\r'  ,0x09  ,'`'   /* 184 */
	,'1'   ,'q'   ,'a'   ,'z'   ,0x00  ,'2'   ,'w'   ,'s'   /* 192 */
	,'x'   ,'<'   ,0x00  ,'3'   ,'e'   ,'d'   ,'c'   ,0x00  /* 200 */
	,'4'   ,'r'   ,'f'   ,'v'   ,' '   ,0x00  ,'5'   ,'t'   /* 208 */
	,'g'   ,'b'   ,0x00  ,'6'   ,'y'   ,'h'   ,'n'   ,0x00  /* 216 */
	,'7'   ,'u'   ,'j'   ,'m'   ,0x00  ,'8'   ,'i'   ,'k'   /* 224 */
	,','   ,0x00  ,'9'   ,'o'   ,'l'   ,'.'   ,0x00  ,'0'   /* 232 */
	,'p'   ,0x00  ,';'   ,'/'   ,0x00  ,'='   ,']'   ,'\\'  /* 240 */
	,0x00  ,'-'   ,'['   ,'\''  ,0x00  ,0x00  ,0x00  ,0x00  /* 248 */
};

unsigned short q_shift_key[]={
	 0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*   0 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*   8 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00	/*  16 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  24 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  32 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  40 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  48 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  56 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  64 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  72 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x1b  ,0x101 ,0x102 /*  80 */
	,0x103 ,0x104 ,0x105 ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /*  88 */ 
	,0x00  ,0x00  ,0x00  ,0x00  ,0x106 ,0x107 ,0x108 ,0x109 /*  96 */
	,0x10a ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /* 104 */ 
	,0x00  ,0x1b  ,0x08  ,0x0a  ,0x10b ,0x00  ,0x00  ,0x00  /* 112 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x10c ,0x10d ,0x00  ,0x00  /* 120 */
	,0x10e ,0x10f ,0x110 ,0x111 ,0x00  ,0x00  ,0x00  ,0x00  /* 128 */
	,0x00  ,0x00  ,0x112 ,0x113 ,0x114 ,0x115 ,0x116 ,0x117 /* 136 */
	,0x00  ,0x00  ,0x120 ,0x00  ,0x121 ,0x122 ,0x123 ,0x124 /* 144 */
	,0x125 ,0x126 ,0x127 ,0x128 ,0x129 ,0x12a ,0x12b ,0x12c /* 152 */
	,0x12d ,0x118 ,0x119 ,0x11a ,0x11b ,0x00  ,0x00  ,0x11c /* 160 */
	,0x11d ,0x11e ,0x11f ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /* 168 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  ,0x00  /* 176 */
	,0x00  ,0x00  ,0x00  ,0x00  ,0x7f  ,'\r'  ,0x09  ,'~'   /* 184 */
	,'!'   ,'Q'   ,'A'   ,'Z'   ,0x00  ,'@'   ,'W'   ,'S'   /* 192 */
	,'X'   ,'>'   ,0x00  ,'#'   ,'E'   ,'D'   ,'C'   ,0x00  /* 200 */
	,'$'   ,'R'   ,'F'   ,'V'   ,' '   ,0x00  ,'%'   ,'T'   /* 208 */
	,'G'   ,'B'   ,0x00  ,'^'   ,'Y'   ,'H'   ,'N'   ,0x00  /* 216 */
	,'&'   ,'U'   ,'J'   ,'M'   ,0x00  ,'*'   ,'I'   ,'K'   /* 224 */
	,','   ,0x00  ,'('   ,'O'   ,'L'   ,'.'   ,0x00  ,')'   /* 232 */
	,'P'   ,0x00  ,':'   ,'?'   ,0x00  ,'+'   ,'}'   ,'|'   /* 240 */
	,0x00  ,'_'   ,'{'   ,'"'   ,0x00  ,0x00  ,0x00  ,0x00  /* 248 */
};


/* LK-series keyboard models */

char lk201_name[] = "LK201";
char lk401_name[] = "LK401";
char lk421_name[] = "LK421";
char lk443_name[] = "LK443";

/* for support of LK201/LK401/LK421 keyboards */

ws_keycode_modifiers lk201_modifiers[] = {
    { KEY_LOCK, LockMask},
    { KEY_SHIFT, ShiftMask},
    { KEY_SHIFT_R, ShiftMask},
    { KEY_CTRL, ControlMask},
    { KEY_ALT_L, Mod1Mask},
    { KEY_ALT_R, Mod1Mask},
    { KEY_COMPOSE, Mod2Mask},
    { KEY_COMPOSE_R, Mod2Mask},
};

ws_keyboard_definition lk201_definition = {
	0,
	LK201_GLYPHS_PER_KEY,		/* beware of this constant!!! */
	sizeof (lk201_keysyms) / sizeof (unsigned int),
	sizeof (lk201_modifiers) / sizeof (ws_keycode_modifiers),
	3
};

/* for support of LK443/LK444 keyboards */

ws_keycode_modifiers lk443_modifiers[] = {
    { KEY_LOCK, LockMask},
    { KEY_SHIFT, ShiftMask},
    { KEY_SHIFT_R, ShiftMask},
    { KEY_CTRL, ControlMask},
    { KEY_CTRL_R, ControlMask},
    { KEY_ALT_L, Mod1Mask},
    { KEY_ALT_R, Mod1Mask},
    { KEY_KP_NUMLOCK, Mod4Mask},
};

ws_keyboard_definition lk443_definition = {
	0,
	LK201_GLYPHS_PER_KEY,		/* beware of this constant!!! */
	sizeof (lk201_keysyms) / sizeof (unsigned int),
	sizeof (lk443_modifiers) / sizeof (ws_keycode_modifiers),
	3
};

/* private keyboard data */
extern ws_keyboard 	keyboard;
#define NLK201 1

ws_keyboard_state lk201_softc[NLK201] = {
    lk201_getc,
    lk201_putc,
    90/* was 75 in 4.L */,	/* timeout */
    9,				/* interval */
    &keyboard,
    0,				/* status */
    {0,0,0,0,0,0,0,0},		/* keys */
    NULL, NULL, 0
    };

/* lk201-specific public keyboard data */
extern void scc_enable_keyboard();	/* to enable keyboard interrupts */

ws_keyboard lk201_keyboard = {
    KB_LK401,				/* by default */
    0,					/* no axis data */
    (caddr_t) lk201_softc,		/* private data */
    lk401_name,				/* default name */
    lk201_init_closure,			/* init_closure */
    NULL,				/* ioctl */
    lk201_up_down_mode,			/* init keyboard */
    lk201_reset_keyboard,		/* reset (close to def lk201) state */
    scc_enable_keyboard,		/* enable interrupts */
    NULL,				/* can't disable */
    lk201_set_keyboard_control,		/* set_keyboard_control */
    NULL,				/* get_keyboard_info */
    lk201_ring_bell,			/* ring_bell */
    lk201_keyboard_event,		/* process_keyboard_event */
    lk201_keyboard_char,		/* process_keyboard_char */
    { 0, 0, 50, 50, 400, 100,
	  1,				/* autorep on, except for modifiers,
					   return */
	  {
	      0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
	      0xffffffff, 0xdff807ff, 0xffffffff, 0xffffffff,
	  },
	  0},				/* leds */
    &lk201_definition,
    lk201_modifiers,
    lk201_keysyms,
    lk201_keycodes,
};

#endif /* BINARY */
