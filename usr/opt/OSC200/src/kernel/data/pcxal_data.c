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
 * @(#)$RCSfile: pcxal_data.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1993/11/17 17:27:47 $
 */

#include <sys/types.h>
#include <kern/queue.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/keysyms.h>
#include <io/dec/ws/pcxal.h>

#define MCTRL(c) ((c) & 0x1f)

#ifdef BINARY

extern char pcxal_keycodes[];
extern char pcxal_keysyms[];
extern ws_keycode_modifiers pcxal_modifiers[];
extern ws_keyboard_definition pcxal_definition;
extern ws_keyboard_state pcxal_softc[];
extern ws_keyboard pcxal_keyboard;
extern xlate_t xlate[];

extern unsigned int gfx_mode_autorepeats[];
extern unsigned int tty_mode_autorepeats[];

#ifdef KBD_XLATE
extern int kk_indices[];
extern xlate_t kkundo_table[];
extern xlate_t kk_table[];
#endif /* KBD_XLATE */

#else /* BINARY */

/*
 * Note that if a keycode appears additional times, it defines further
 * symbols on the the same keycode.  DDX translates this to the appropriate
 * data structure.  All this is to save bytes in the kernel.
 * WARNING: keycodes and keysym tables must be EXACTLY in sync!
 */
unsigned char pcxal_keycodes[] = {
    KEY_ESC,
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
    KEY_F11,
    KEY_F12,
    KEY_PRINT_SCREEN,
    KEY_SCROLL_LOCK,
    KEY_PAUSE,

    KEY_INSERT,
    KEY_HOME,
    KEY_PAGE_UP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGE_DOWN,

    KEY_KP_0,
    KEY_KP_PERIOD,
    KEY_KP_ENTER,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_PLUS,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_NUMLOCK,
    KEY_KP_SLASH,
    KEY_KP_STAR,
    KEY_KP_HYPHEN,

    KEY_LEFT,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_UP,

    KEY_SHIFT_L,
    KEY_SHIFT_R,
    KEY_CTRL_L,
    KEY_CTRL_R,
    KEY_CAPS_LOCK,
    KEY_ALT_L,
    KEY_ALT_L,
    KEY_ALT_R,
    KEY_ALT_R,

    KEY_BACKSPACE,
    KEY_ENTER,
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

    KEY_VBAR_102,
    KEY_VBAR_102,

    KEY_TILDE_102J,
    KEY_TILDE_102J,

    KEY_UBAR_J,
    KEY_UBAR_J,

    KEY_VBAR_J,
    KEY_VBAR_J,

    KEY_MUHENKAN,

    KEY_HENKAN,

    KEY_HIRAGANA,
};

unsigned int pcxal_keysyms[] = {
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
    XK_F11,
    XK_F12,
    XK_Print,
    XK_Scroll_Lock,
    XK_Pause,

    XK_Insert,
    XK_Home,
    XK_Prior,
    XK_Delete,
    XK_End,
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
    XK_KP_Add,
    XK_KP_7,
    XK_KP_8,
    XK_KP_9,
    XK_Num_Lock,
    XK_KP_Divide,
    XK_KP_Multiply,
    XK_KP_Subtract,

    XK_Left,
    XK_Right,
    XK_Down,
    XK_Up,

    XK_Shift_L,
    XK_Shift_R,
    XK_Control_L,
    XK_Control_R,
    XK_Caps_Lock,
    XK_Alt_L,
    XK_Meta_L,
    XK_Alt_R,
    XK_Meta_R,

    XK_BackSpace,
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

    XK_backslash,
    XK_bar,

    XK_numbersign,
    XK_asciitilde,

    XK_backslash,
    XK_underscore,

    XK_backslash,
    XK_bar,

    XK_Muhenkan,

    XK_Henkan,

    XK_Hiragana_Katakana,

};

ws_keycode_modifiers pcxal_modifiers[] = {
    { KEY_CAPS_LOCK, LockMask},
    { KEY_SHIFT_L, ShiftMask},
    { KEY_SHIFT_R, ShiftMask},
    { KEY_CTRL_L, ControlMask},
    { KEY_CTRL_R, ControlMask},
    { KEY_ALT_L, Mod1Mask},
    { KEY_ALT_R, Mod1Mask},
    { KEY_KP_NUMLOCK, Mod4Mask},
};

ws_keyboard_definition pcxal_definition = {
	0,
	PCXAL_GLYPHS_PER_KEY,		/* beware of this constant!!! */
	sizeof (pcxal_keysyms) / sizeof (unsigned int),
	sizeof (pcxal_modifiers) / sizeof (ws_keycode_modifiers),
	3
};

extern ws_keyboard 	keyboard;

#define NPCXAL 1

char pcxal_name[] = "PCXAL";

/* private keyboard data */
ws_keyboard_state pcxal_softc[NPCXAL] = {
    NULL,
    NULL,
    250, /* timeout  */
    50,  /* interval */
    &keyboard,
    0,   /* status */
    {0,0,0,0,0,0,0,0}, /* keys */
    NULL, NULL, 0
    };

/* pcxal-specific public keyboard data */
ws_keyboard pcxal_keyboard = {
    KB_PCXAL,				/* by default */
    0,					/* no axis data */
    (caddr_t) pcxal_softc,		/* private data */
    pcxal_name,				/* name */
    pcxal_init_closure,			/* init_closure */
    NULL,				/* ioctl */
    pcxal_init_keyboard,		/* init keyboard */
    pcxal_reset_keyboard,		/* reset state */
    pcxal_enable_keyboard,		/* enable interrupts */
    NULL,				/* disable */
    pcxal_set_keyboard_control,		/* set_keyboard_control */
    NULL,				/* get_keyboard_info */
    pcxal_ring_bell,			/* ring_bell */
    pcxal_keyboard_event,		/* process_keyboard_event */
    pcxal_keyboard_char,		/* process_keyboard_char */
/* FIXME FIXME FIXME FIXME */
    {	0,			/* device_number */
	0,			/* flags */
	0,			/* click */
	50,			/* bell */
	440,			/* bell_pitch (middle A :-) */
	100,			/* bell_duration (100 milliseconds) */
	1,			/* autorep on, except for modifiers,
					   return */
	  {
	      0, 0, 0, 0, 0, 0, 0, 0,	/* autorepeats - depends on mode */
	  },
	  0},				/* leds */
/* end FIXME FIXME FIXME FIXME */
    &pcxal_definition,
    pcxal_modifiers,
    pcxal_keysyms,
    pcxal_keycodes,
};

unsigned int gfx_mode_autorepeats[8] = {
	0xfde9ffff, 0xfdffffff, 0x78ffffff, 0xfdbfffff, 
	0xffffffff, 0xdfffffff, 0xffffffff, 0xffffffff,
};

unsigned int tty_mode_autorepeats[8] = {
	      0xcfffffff, 0xfabffbff, 0xffffff9f, 0xffffffff, 
	      0xffffffff, 0xdfffffff, 0xffffffff, 0xffffffff,
};

/****************************************/
/* originally from io/dec/isa/kbdscan.h */
/****************************************/

/* Some Key Defs */
/* Function keys */

/*
   NOTE: use the same mask as the LK201 table
   to indicate special scancodes like function
   keys, editing keys, numeric keypad keys.
   Also, try to keep the same special key indices
   for the PCXAL keyboard as for LK201, when
   possible. This means there are some holes
   in the indices defined below...

   The indices come from data/ws_data.c, the
   "special_keys" table.

   Original entries are in the range 0x01-0x2d
   Start new entries at offset 0x30
*/

#define SPEC_MASK	0x100

#define	F1		(SPEC_MASK|0x01)
#define	F2		(SPEC_MASK|0x02)
#define	F3		(SPEC_MASK|0x03)
#define	F4		(SPEC_MASK|0x04)
#define	F5		(SPEC_MASK|0x05)
#define	F6		(SPEC_MASK|0x06)
#define	F7		(SPEC_MASK|0x07)
#define	F8		(SPEC_MASK|0x08)
#define	F9		(SPEC_MASK|0x09)
#define	F10		(SPEC_MASK|0x0a)

#define	INS		(SPEC_MASK|0x13)	/* insert */
#define	END		(SPEC_MASK|0x15)	/* select */
#define	PGUP		(SPEC_MASK|0x16)	/* prev */
#define	PGDWN		(SPEC_MASK|0x17)	/* next */

#define	LEFT		(SPEC_MASK|0x1c)
#define	RIGHT		(SPEC_MASK|0x1d)
#define	DOWN		(SPEC_MASK|0x1e)
#define	UP		(SPEC_MASK|0x1f)

#define	DELE		(0x7f)			/* not special */

/* New Keys for PCXAL */
#define	F11		(SPEC_MASK|0x30)	/* new */
#define	F12		(SPEC_MASK|0x31)	/* new */

#define	HOME		(SPEC_MASK|0x32)	/* new */
#define	MIDDLE		(SPEC_MASK|0x33)	/* new */

/* Others */
#define	PRTSC		(SPEC_MASK|0x34)	/* new */
#define	PAUSE		(SPEC_MASK|0x35)	/* new */

#if 0
/* FIXME FIXME - don't know where/if these are needed */
#define	ALTDWN		(SPEC_MASK|0x36)	/* new */
#define	ALTUP		(SPEC_MASK|0x37)	/* new */
#endif /* 0 */

/**************************************/
/* originally from io/dec/isa/xlate.c */
/**************************************/

xlate_t xlate[] = {

#ifndef KBD_XLATE
/*
 * Index by scan code from a 101 key
 * AT style keyboard. The keyboard driver maps the
 * keycodes from the 84 key keyboard into the
 * keycodes from the 101 key keyboard.
 *
 * NOTE: the scancodes used here are those generated by the keyboard
 *       when it is in scanmode 3.
 *
 */

 /* unshifted  shifted  type    */	/* Scan, key number		*/
 { 	0,	0,	UNKNOWN	},	/* 0x00, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x01, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x02, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x03, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x04, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x05, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x06, Key num 0 		*/
 { 	F1,	F1,	FUNCTION},	/* 0x07, Key num 112 		*/
 { 	033,	033,	ASCII	},	/* 0x08, Key num 110, ESC 	*/
 { 	0,	0,	UNKNOWN	},	/* 0x09, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x0a, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x0b, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x0c, Key num 0 		*/
 { 	'\t',	'\t',	ASCII	},	/* 0x0D, Key num 16 		*/
 { 	'`',	'~',	ASCII	},	/* 0x0E, Key num 1 		*/
 { 	F2,	F2,	FUNCTION},	/* 0x0F, Key num 113 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x10, Key num 0 		*/
 { 	0,	0,	LCTRL	},	/* F0 0x11, Key num 58 		*/
 { 	0,	0,	LSHIFT	},	/* F0 0x12, Key num 44 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x13, Key num 0 		*/
 { 	0,	0,	CAPS	},	/* F0 0x14, Key num 30 		*/
 { 	'q',	'Q',	ASCII	},	/* 0x15, Key num 17 		*/
 { 	'1',	'!',	ASCII	},	/* 0x16, Key num 2 		*/
 { 	F3,	F3,	FUNCTION},	/* 0x17, Key num 114 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x18, Key num 0 		*/
 { 	0,	0,	LALT	},	/* F0 0x19, Key num 60 		*/
 { 	'z',	'Z',	ASCII	},	/* 0x1A, Key num 46 		*/
 { 	's',	'S',	ASCII	},	/* 0x1B, Key num 32 		*/
 { 	'a',	'A',	ASCII	},	/* 0x1C, Key num 31 		*/
 { 	'w',	'W',	ASCII	},	/* 0x1D, Key num 18 		*/
 { 	'2',	'@',	ASCII	},	/* 0x1E, Key num 3 		*/
 { 	F4,	F4,	FUNCTION},	/* 0x1F, Key num 115 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x20, Key num 0 		*/
 { 	'c',	'C',	ASCII	},	/* 0x21, Key num 48 		*/
 { 	'x',	'X',	ASCII	},	/* 0x22, Key num 47 		*/
 { 	'd',	'D',	ASCII	},	/* 0x23, Key num 33 		*/
 { 	'e',	'E',	ASCII	},	/* 0x24, Key num 19 		*/
 { 	'4',	'$',	ASCII	},	/* 0x25, Key num 5 		*/
 { 	'3',	'#',	ASCII	},	/* 0x26, Key num 4 		*/
 { 	F5,	F5,	FUNCTION},	/* 0x27, Key num 116 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x28, Key num 0 		*/
 { 	' ',	' ',	ASCII	},	/* 0x29, Key num 61 		*/
 { 	'v',	'V',	ASCII	},	/* 0x2A, Key num 49 		*/
 { 	'f',	'F',	ASCII	},	/* 0x2B, Key num 34 		*/
 { 	't',	'T',	ASCII	},	/* 0x2C, Key num 21 		*/
 { 	'r',	'R',	ASCII	},	/* 0x2D, Key num 20 		*/
 { 	'5',	'%',	ASCII	},	/* 0x2E, Key num 6 		*/
 { 	F6,	F6,	FUNCTION},	/* 0x2F, Key num 117 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x30, Key num 0 		*/
 { 	'n',	'N',	ASCII	},	/* 0x31, Key num 51 		*/
 { 	'b',	'B',	ASCII	},	/* 0x32, Key num 50 		*/
 { 	'h',	'H',	ASCII	},	/* 0x33, Key num 36 		*/
 { 	'g',	'G',	ASCII	},	/* 0x34, Key num 35 		*/
 { 	'y',	'Y',	ASCII	},	/* 0x35, Key num 22 		*/
 { 	'6',	'^',	ASCII	},	/* 0x36, Key num 7 		*/
 { 	F7,	F7,	FUNCTION},	/* 0x37, Key num 118 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x38, Key num 0 		*/
 { 	0,	0,	RALT	},	/* 0x39, Key num 62 		*/
 { 	'm',	'M',	ASCII	},	/* 0x3A, Key num 52 		*/
 { 	'j',	'J',	ASCII	},	/* 0x3B, Key num 37 		*/
 { 	'u',	'U',	ASCII	},	/* 0x3C, Key num 23 		*/
 { 	'7',	'&',	ASCII	},	/* 0x3D, Key num 8 		*/
 { 	'8',	'*',	ASCII	},	/* 0x3E, Key num 9 		*/
 { 	F8,	F8,	FUNCTION},	/* 0x3F, Key num 119 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x40, Key num 0 		*/
 { 	',',	'<',	ASCII	},	/* 0x41, Key num 53 		*/
 { 	'k',	'K',	ASCII	},	/* 0x42, Key num 38 		*/
 { 	'i',	'I',	ASCII	},	/* 0x43, Key num 24 		*/
 { 	'o',	'O',	ASCII	},	/* 0x44, Key num 25 		*/
 { 	'0',	')',	ASCII	},	/* 0x45, Key num 11 		*/
 { 	'9',	'(',	ASCII	},	/* 0x46, Key num 10 		*/
 { 	F9,	F9,	FUNCTION},	/* 0x47, Key num 120 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x48, Key num 0 		*/
 { 	'.',	'>',	ASCII	},	/* 0x49, Key num 54 		*/
 { 	'/',	'?',	ASCII	},	/* 0x4A, Key num 55 		*/
 { 	'l',	'L',	ASCII	},	/* 0x4B, Key num 39 		*/
 { 	';',	':',	ASCII	},	/* 0x4C, Key num 40 		*/
 { 	'p',	'P',	ASCII	},	/* 0x4D, Key num 26 		*/
 { 	'-',	'_',	ASCII	},	/* 0x4E, Key num 12 		*/
 { 	F10,	F10,	FUNCTION},	/* 0x4F, Key num 121 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x50, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x51, Key num 0 		*/
 { 	'\'',	'\"',	ASCII	},	/* 0x52, Key num 41 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x53, Key num 0 		*/
 { 	'[',	'{',	ASCII	},	/* 0x54, Key num 27 		*/
 { 	'=',	'+',	ASCII	},	/* 0x55, Key num 13 		*/
 { 	F11,	F11,	FUNCTION},	/* 0x56, Key num 122 		*/
 { 	PRTSC,	PRTSC,	FUNCTION},	/* 0x57, Key num 124 		*/
 { 	0,	0,	RCTRL	},	/* 0x58, Key num 64 		*/
 { 	0,	0,	RSHIFT	},	/* F0 0x59, Key num 57 		*/
 { 	'\r',	'\r',	ASCII	},	/* 0x5A, Key num 43 		*/
 { 	']',	'}',	ASCII	},	/* 0x5B, Key num 28 		*/
 { 	'\\',	'|',	ASCII	},	/* 0x5C, Key num 29 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x5d, Key num 0 		*/
 { 	F12,	F12,	FUNCTION},	/* 0x5E, Key num 123 		*/
 { 	0,	0,	SCROLL	},	/* 0x5F, Key num 125 		*/
 { 	DOWN,	DOWN,	NUMPAD	},	/* 0x60, Key num 84 		*/
 { 	LEFT,	LEFT,	NUMPAD	},	/* 0x61, Key num 79 		*/
 { 	PAUSE,	PAUSE,	FUNCTION},	/* 0x62, Key num 126 		*/
 { 	UP,	UP,	NUMPAD	},	/* 0x63, Key num 83 		*/
 { 	DELE,	DELE,	NUMPAD	},	/* 0x64, Key num 76 		*/
 { 	END,	END,	NUMPAD	},	/* 0x65, Key num 81 		*/
 { 	MCTRL('H'), MCTRL('H'),	ASCII }, /* 0x66, Key num 15 		*/
 { 	INS,	INS,	NUMPAD	},	/* 0x67, Key num 75 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x68, Key num 0 		*/
 { 	END,	'1',	NUMPAD	},	/* 0x69, Key num 93 		*/
 { 	RIGHT,	RIGHT,	NUMPAD	},	/* 0x6A, Key num 89 		*/
 { 	LEFT,	'4',	NUMPAD	},	/* 0x6B, Key num 92 		*/
 { 	HOME,	'7',	NUMPAD	},	/* 0x6C, Key num 91 		*/
 { 	PGDWN,	PGDWN,	NUMPAD	},	/* 0x6D, Key num 86 		*/
 { 	HOME,	HOME,	NUMPAD	},	/* 0x6E, Key num 80 		*/
 { 	PGUP,	PGUP,	NUMPAD	},	/* 0x6F, Key num 85 		*/
 { 	INS,	'0',	NUMPAD	},	/* 0x70, Key num 99 		*/
 { 	DELE,	'.',	NUMPAD	},	/* 0x71, Key num 104 		*/
 { 	DOWN,	'2',	NUMPAD	},	/* 0x72, Key num 98 		*/
 { 	MIDDLE,	'5',	NUMPAD	},	/* 0x73, Key num 97 		*/
 { 	RIGHT,	'6',	NUMPAD	},	/* 0x74, Key num 102 		*/
 { 	UP,	'8',	NUMPAD	},	/* 0x75, Key num 96 		*/
 { 	0,	0,	NUMLOCK	},	/* 0x76, Key num 90 		*/
 { 	'/',	'/',	ASCII	},	/* 0x77, Key num 95 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x78, Key num 0 		*/
 { 	'\r',	'\r',	ASCII  },	/* F0 0x79, Key num 108 	*/
 { 	PGDWN,	'3',	NUMPAD	},	/* 0x7A, Key num 103 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x7b, Key num 0 		*/
 { 	'+',	'+',	ASCII	},	/* 0x7C, Key num 106 		*/
 { 	PGUP,	'9',	NUMPAD	},	/* 0x7D, Key num 101 		*/
 { 	'*',	'*',	ASCII	},	/* 0x7E, Key num 100 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x7f, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x80, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x81, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x82, Key num 0 		*/
 { 	0,	0,	UNKNOWN	},	/* 0x83, Key num 0 		*/
 { 	'-',	'-',	ASCII	},	/* 0x84, Key num 105 		*/

#else /* !KBD_XLATE */

 /* unshifted  shifted  type    */	/* Scan, key number		*/
 { 	0,	0,	UNKNOWN	},	/* 0x00, Key num 0 		*/
 { 	033,	033,	ASCII	},	/* 0x01, Key num 110, ESC 	*/
 { 	'1',	'!',	ASCII	},	/* 0x02, Key num 2 		*/
 { 	'2',	'@',	ASCII	},	/* 0x03, Key num 3 		*/
 { 	'3',	'#',	ASCII	},	/* 0x04, Key num 4 		*/
 { 	'4',	'$',	ASCII	},	/* 0x05, Key num 5 		*/
 { 	'5',	'%',	ASCII	},	/* 0x06, Key num 6 		*/
 { 	'6',	'^',	ASCII	},	/* 0x07, Key num 7 		*/
 { 	'7',	'&',	ASCII	},	/* 0x08, Key num 8 		*/
 { 	'8',	'*',	ASCII	},	/* 0x09, Key num 9 		*/
 { 	'9',	'(',	ASCII	},	/* 0x0a, Key num 10 		*/
 { 	'0',	')',	ASCII	},	/* 0x0b, Key num 11 		*/
 { 	'-',	'_',	ASCII	},	/* 0x0c, Key num 12 		*/
 { 	'=',	'+',	ASCII	},	/* 0x0d, Key num 13 		*/
 {	DELE,   DELE,	ASCII   },	/* 0x0e, Key num 15 		*/
 { 	'\t',	'\t',	ASCII	},	/* 0x0f, Key num 16 		*/
 { 	'q',	'Q',	ASCII	},	/* 0x10, Key num 17 		*/
 { 	'w',	'W',	ASCII	},	/* 0x11, Key num 18 		*/
 { 	'e',	'E',	ASCII	},	/* 0x12, Key num 19 		*/
 { 	'r',	'R',	ASCII	},	/* 0x13, Key num 20 		*/
 { 	't',	'T',	ASCII	},	/* 0x14, Key num 21 		*/
 { 	'y',	'Y',	ASCII	},	/* 0x15, Key num 22 		*/
 { 	'u',	'U',	ASCII	},	/* 0x16, Key num 23 		*/
 { 	'i',	'I',	ASCII	},	/* 0x17, Key num 24 		*/
 { 	'o',	'O',	ASCII	},	/* 0x18, Key num 25 		*/
 { 	'p',	'P',	ASCII	},	/* 0x19, Key num 26 		*/
 { 	'[',	'{',	ASCII	},	/* 0x1a, Key num 27 		*/
 { 	']',	'}',	ASCII	},	/* 0x1b, Key num 28 		*/
 { 	'\r',	'\r',	ASCII	},	/* 0x1c, Key num 43, RET	*/
 { 	0,	0,	LCTRL	},	/* 0x1d, Key num 58 		*/
 { 	'a',	'A',	ASCII	},	/* 0x1e, Key num 31 		*/
 { 	's',	'S',	ASCII	},	/* 0x1f, Key num 32 		*/
 { 	'd',	'D',	ASCII	},	/* 0x20, Key num 33 		*/
 { 	'f',	'F',	ASCII	},	/* 0x21, Key num 34 		*/
 { 	'g',	'G',	ASCII	},	/* 0x22, Key num 35 		*/
 { 	'h',	'H',	ASCII	},	/* 0x23, Key num 36 		*/
 { 	'j',	'J',	ASCII	},	/* 0x24, Key num 37 		*/
 { 	'k',	'K',	ASCII	},	/* 0x25, Key num 38 		*/
 { 	'l',	'L',	ASCII	},	/* 0x26, Key num 39 		*/
 { 	';',	':',	ASCII	},	/* 0x27, Key num 40 		*/
 { 	'\'',	'\"',	ASCII	},	/* 0x28, Key num 41 		*/
 { 	'`',	'~',	ASCII	},	/* 0x29, Key num 1 		*/
 { 	0,	0,	LSHIFT	},	/* 0x2a, Key num 44 		*/
 { 	'\\',	'|',	ASCII	},	/* 0x2b, Key num 42 		*/
 { 	'z',	'Z',	ASCII	},	/* 0x2c, Key num 46 		*/
 { 	'x',	'X',	ASCII	},	/* 0x2d, Key num 47 		*/
 { 	'c',	'C',	ASCII	},	/* 0x2e, Key num 48 		*/
 { 	'v',	'V',	ASCII	},	/* 0x2f, Key num 49 		*/
 { 	'b',	'B',	ASCII	},	/* 0x30, Key num 50 		*/
 { 	'n',	'N',	ASCII	},	/* 0x31, Key num 51 		*/
 { 	'm',	'M',	ASCII	},	/* 0x32, Key num 52 		*/
 { 	',',	'<',	ASCII	},	/* 0x33, Key num 53 		*/
 { 	'.',	'>',	ASCII	},	/* 0x34, Key num 54 		*/
 { 	'/',	'?',	ASCII	},	/* 0x35, Key num 55 		*/
 { 	0,	0,	RSHIFT	},	/* 0x36, Key num 57 		*/
 { 	'*',	'*',	ASCII	},	/* 0x37, Key num 100 		*/
 { 	0,	0,	LALT	},	/* 0x38, Key num 60 		*/
 { 	' ',	' ',	ASCII	},	/* 0x39, Key num 61 		*/
 { 	0,	0,	CAPS	},	/* 0x3a, Key num 30 		*/
 { 	F1,	F1,	FUNCTION},	/* 0x3b, Key num 112 		*/
 { 	F2,	F2,	FUNCTION},	/* 0x3c, Key num 113 		*/
 { 	F3,	F3,	FUNCTION},	/* 0x3d, Key num 114 		*/
 { 	F4,	F4,	FUNCTION},	/* 0x3e, Key num 115 		*/
 { 	F5,	F5,	FUNCTION},	/* 0x3f, Key num 116 		*/
 { 	F6,	F6,	FUNCTION},	/* 0x40, Key num 117 		*/
 { 	F7,	F7,	FUNCTION},	/* 0x41, Key num 118 		*/
 { 	F8,	F8,	FUNCTION},	/* 0x42, Key num 119 		*/
 { 	F9,	F9,	FUNCTION},	/* 0x43, Key num 120 		*/
 { 	F10,	F10,	FUNCTION},	/* 0x44, Key num 121 		*/
 { 	0,	0,	NUMLOCK	},	/* 0x45, Key num 90 		*/
 { 	0,	0,	SCROLL	},	/* 0x46, Key num 125 		*/
 { 	HOME,	'7',	NUMPAD	},	/* 0x47, Key num 91 		*/
 { 	UP,	'8',	NUMPAD	},	/* 0x48, Key num 96 		*/
 { 	PGUP,	'9',	NUMPAD	},	/* 0x49, Key num 101 		*/
 { 	'-',	'-',	ASCII	},	/* 0x4a, Key num 105 		*/
 { 	LEFT,	'4',	NUMPAD	},	/* 0x4b, Key num 92 		*/
 { 	MIDDLE,	'5',	NUMPAD	},	/* 0x4c, Key num 97 		*/
 { 	RIGHT,	'6',	NUMPAD	},	/* 0x4d, Key num 102 		*/
 { 	'+',	'+',	ASCII	},	/* 0x4e, Key num 106 		*/
 { 	END,	'1',	NUMPAD	},	/* 0x4f, Key num 93 		*/
 { 	DOWN,	'2',	NUMPAD	},	/* 0x50, Key num 98 		*/
 { 	PGDWN,	'3',	NUMPAD	},	/* 0x51, Key num 103 		*/
 { 	INS,	'0',	NUMPAD	},	/* 0x52, Key num 99 		*/
 { 	DELE,	'.',	NUMPAD	},	/* 0x53, Key num 104 		*/
 { 	'4',	'4',	ASCII	},	/* 0x54, Key num 0 FIXME ??54??	*/
 { 	'5',	'5',	UNKNOWN	},	/* 0x55, Key num 0 FIXME ??55??	*/
 { 	'\\',	'|',	ASCII	},	/* 0x56, Key num 45 (UK)	*/
 { 	F11,	F11,	FUNCTION},	/* 0x57, Key num 122 		*/
 { 	F12,	F12,	FUNCTION},	/* 0x58, Key num 123 		*/

#endif /* !KBD_XLATE */
};

#ifdef KBD_XLATE
int kk_indices[] =
{
0x01,
0x0e,
0x29,
0x3a,
0x57,

0x00 /* END OF TABLE */
};

xlate_t kkundo_table[] =
{
 { 	033,	033,	ASCII	},	/* 0x01, Key num 110, ESC 	*//* ESC change to `~ */
 { 	MCTRL('H'), MCTRL('H'),	ASCII }, /* 0x0e, Key num 15 		*//* BSP change to DELE */
 { 	'`',	'~',	ASCII	},	/* 0x29, Key num 1 		*//* `~ change to ESC */
 { 	0,	0,	CAPS	},	/* 0x3a, Key num 30 		*//* CAPS change to LCTRL */
 { 	F11,	F11,	FUNCTION},	/* 0x57, Key num 122 		*/

 { 	0,	0,	UNKNOWN	}	/* END OF TABLE			*/
};

xlate_t kk_table[] =
{
 { 	'`',	'~',	ASCII	},	/* 0x01, Key num 110, ESC 	*//* ESC change to `~ */
 { 	DELE,	DELE,	ASCII	},	/* 0x0e, Key num 15 		*//* BSP change to DELE */
 { 	033,	033,	ASCII	},	/* 0x29, Key num 1 		*//* `~ change to ESC */
 { 	0,	0,	LCTRL	},	/* 0x3a, Key num 30 		*//* CAPS change to LCTRL */
 { 	033,	033,	ASCII	},	/* 0x57, Key num 122 		*//* F11 change to ESC */

 { 	0,	0,	UNKNOWN	}	/* END OF TABLE			*/
};
#endif /* KBD_XLATE */

#endif /* BINARY */
