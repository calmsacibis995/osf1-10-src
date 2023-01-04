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
 * @(#)$RCSfile: pcxal.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/02 15:31:38 $
 */

#ifndef _PCXAL_H_
#define _PCXAL_H_

#define KBD_XLATE	/* use keyboard interface to do translate to ASCII */

/* 
 * command keycodes for Digital PCXAL series keyboards.
 */

/* keyboard controller registers (PRI:10-4) */
#define PK_STAT			0x64	/* status register  - RO */
#define PK_CMD			0x64	/* command register - WO */
#define PK_IB			0x60	/* input buffer     - WO */
#define PK_OB			0x60	/* output buffer    - RO */

/* status register, PS2 format (not pc-at format) page 6-202 (PRI:10-6) */
#define	PK_STAT_OBF		0x01	/* Output Buffer Full 		*/
#define	PK_STAT_IBF		0x02	/* Input Buffer Full 		*/
#define	PK_STAT_SYS		0x04	/* System Flag	 		*/
#define	PK_STAT_CD		0x08	/* Command/Data Flag 		*/
#define	PK_STAT_KBEN		0x10	/* Keyboard Enable 		*/
#define	PK_STAT_ODS		0x20	/* Output buffer Source 	*/
#define	PK_STAT_GTO		0x40	/* General Time Out 		*/
#define	PK_STAT_PERR		0x80	/* Parity Error 		*/	

/* mode register, PS2 format (not pc-at format) page 6-199 (PRI:10-4)   */
#define PK_MODE_EKI		0x01	/* Enable Keyboard Interrupt 	*/
#define PK_MODE_EMI		0x02	/* Enable Mouse Interrupt 	*/
#define PK_MODE_SYS		0x04	/* System Flag 			*/
#define PK_MODE_DKB		0x10	/* Disable Keyboard 		*/
#define PK_MODE_DMS		0x20	/* Disable Mouse 		*/
#define PK_MODE_KCC		0x40	/* Keycode Conversion 		*/

/* keyboard controller commands, page 6-203 (PRI:10-9) */
#define PK_CTL_RDMODE	0x20	/* Read Mode Register */
#define PK_CTL_WRMODE	0x60	/* Write Mode Register */
#define	PK_CTL_TEST1	0xAA	/* Self Test */
#define	PK_CTL_TEST2	0xAB	/* KB Interface Test */
#define	PK_CTL_DISABLE	0xAD	/* Disable KB */
#define	PK_CTL_ENABLE	0xAE	/* Enable KB */

#define	PK_CTL_RDOUT	0xD0	/* Read Output Port */
#define	PK_CTL_WROUT	0xD1	/* Write Output Port */

#define	PK_CTL_MDISABLE	0xA7	/* Disable Mouse */
#define	PK_CTL_MENABLE	0xA8	/* Enable Mouse */
#define PK_CTL_MTEST	0xA9	/* Mouse Test */
#define PK_CTL_WRMOUSE	0xD4	/* Write to Mouse */

/* 
 * Found definitions for these in the Keyboard chapter of the
 * "IBM Personal System/2 Hardware Interface Technical Reference -
 * Common Interfaces"
 */
/*
 * Commands sent to the keyboard.  (See page 21)
 */
#define	PK_SETLEDS	0xED
#define	PK_SELECTCODE	0xF0
#define PK_SETRATE	0xF3
#define	PK_ENABLE	0xF4
#define	PK_DISABLE	0xF5
#define	PK_DEFAULTS	0xF6
#define	PK_SETALL_MB	0xF8
#define	PK_SETALL_MBT	0xFA
#define	PK_SETKEY_MB	0xFC
#define	PK_RESET	0xFF

/*
 * Commands sent from the keyboard to the system.  (These are actually
 * responses.)  (See page 26.)
 */
#define	PK_ACK		0xFA
#define	PK_RESEND	0xFE
#define PK_POR		0xAA	/* Power-On Reset Completion */
#define PK_OVR		0x00	/* Over Run Condition */

/* Keyboard LED bits
 * This is described in the keyboard spec page 29.  The 3 LEDS are accessed
 * as follows:
 *      BIT     FUNCTION
 *       0      Scroll lock indicator
 *       1      Num lock indicator
 *       2      Caps lock indicator
 *      7-3     reserved, must be 0
 */
#define PK_LED_HOLD	0x01
#define PK_LED_NUMLOCK	0x02
#define PK_LED_LOCK	0x04


/* mouse commands - see logitech spec page 2-11 */
/* Logitech Mouse spec - Technical Reference and Programmer's Guide */
#define	PM_RESET	0xFF	/* Set to default parameters.  (incremental
				 * stream mode, 1:1 scaling, report rate of
				 * 100.  Following this command the mouse is
				 * disabled. */
#define PM_RESEND	0xFE	/* Resend last transmission */
#define	PM_ENABLE	0xF4	/* Start the transmission */

#define PM_ACK		0xFA	/* Acknowledge from mouse */
/* end mouse */

/* rtc - control register 1 */
/* The rtc register is used to control peripheral chip selects that are
 * not included in control register 0.
 */
#define	RTCA		0x0170	/* address select register */
#define	RTCD		0x0171	/* data register */
/* These defines are associated with the rtc register.  They are used to
 * initialize the keyboard mode.  See 6-207 for a complete description
 * of what the 6A represents.
 */
#define	RTCCR1		0x6A	/* address of control register #1 */
#define	ATKBD		0x02	/* enable AT keyboard flag */

/* prefix */
#define	F0SEEN		0x00f0
#define	E0SEEN		0x00e0

#ifndef KBD_XLATE
#define PCXAL_MAX_KEYCODE	0x87	/* max "normal" keycode */
					/* inc. Japanese kbd, PCXAJ-AA */
#else
#define PCXAL_MAX_KEYCODE	0x58	/* max "normal" keycode */
#endif /* !KBD_XLATE */

#define PCXAL_GLYPHS_PER_KEY    2

/* the keys themselves */

#define RAW_KEY_F1		0x07	/* must be above 7 for X */

/* first, the top row */
#define KEY_ESC			0x08
#define KEY_F1			0x09	/* must be above 7 for X */
#define KEY_F2			0x0f
#define KEY_F3			0x17
#define KEY_F4			0x1f
#define KEY_F5			0x27
#define KEY_F6			0x2f
#define KEY_F7			0x37
#define KEY_F8			0x3f
#define KEY_F9			0x47
#define KEY_F10			0x4f
#define KEY_F11			0x56
#define KEY_F12			0x5e
#define KEY_PRINT_SCREEN	0x57
#define KEY_SCROLL_LOCK		0x5f
#define KEY_PAUSE		0x62

/* editing keypad */
#define KEY_INSERT		0x67
#define KEY_HOME		0x6e
#define KEY_PAGE_UP		0x6f
#define KEY_DELETE		0x64
#define KEY_END			0x65
#define KEY_PAGE_DOWN		0x6d

/* numeric keypad */
#define KEY_KP_0		0x70
#define KEY_KP_PERIOD		0x71
#define KEY_KP_ENTER		0x79
#define KEY_KP_1		0x69
#define KEY_KP_2		0x72
#define KEY_KP_3		0x7a
#define KEY_KP_4		0x6b
#define KEY_KP_5		0x73
#define KEY_KP_6		0x74
#define KEY_KP_PLUS		0x7c
#define KEY_KP_7		0x6c
#define KEY_KP_8		0x75
#define KEY_KP_9		0x7d
#define KEY_KP_NUMLOCK		0x76
#define KEY_KP_SLASH		0x77
#define KEY_KP_STAR		0x7e
#define KEY_KP_HYPHEN		0x84

/* arrow (cursor) keys */
#define KEY_LEFT		0x61
#define KEY_RIGHT		0x6a
#define KEY_UP			0x63
#define KEY_DOWN		0x60

/* modifier keys */
#define KEY_SHIFT_L		0x12
#define KEY_SHIFT_R		0x59
#define KEY_ALT_L		0x19
#define KEY_ALT_R		0x39
#define KEY_CTRL_L		0x11
#define KEY_CTRL_R		0x58
#define KEY_CAPS_LOCK		0x14

/* misc keys */
#define KEY_BACKSPACE		0x66
#define KEY_ENTER		0x5a
#define KEY_TAB			0x0d

/* typewriter keys - top row */
#define KEY_TILDE		0x0e
#define KEY_TILDE_102J		0x53
#define KEY_TR_1		0x16
#define KEY_TR_2		0x1e
#define KEY_TR_3		0x26
#define KEY_TR_4		0x25
#define KEY_TR_5		0x2e
#define KEY_TR_6		0x36
#define KEY_TR_7		0x3d
#define KEY_TR_8		0x3e
#define KEY_TR_9		0x46
#define KEY_TR_0		0x45
#define KEY_UBAR		0x4e
#define KEY_UBAR_J		0x51
#define KEY_PLUS		0x55

/* typewriter keys - 2nd row */
#define KEY_Q			0x15
#define KEY_W			0x1d
#define KEY_E			0x24
#define KEY_R			0x2d
#define KEY_T			0x2c
#define KEY_Y			0x35
#define KEY_U			0x3c
#define KEY_I			0x43
#define KEY_O			0x44
#define KEY_P			0x4d
#define KEY_LBRACE		0x54
#define KEY_RBRACE		0x5b
#define KEY_VBAR		0x5c
#define KEY_VBAR_J		0x5d
#define KEY_VBAR_102		0x13

/* typewriter keys - 3rd row */
#define KEY_A			0x1c
#define KEY_S			0x1b
#define KEY_D			0x23
#define KEY_F			0x2b
#define KEY_G			0x34
#define KEY_H			0x33
#define KEY_J			0x3b
#define KEY_K			0x42
#define KEY_L			0x4b
#define KEY_SEMICOLON		0x4c
#define KEY_QUOTE		0x52

/* typewriter keys - 4th row */
#define KEY_Z			0x1a
#define KEY_X			0x22
#define KEY_C			0x21
#define KEY_V			0x2a
#define KEY_B			0x32
#define KEY_N			0x31
#define KEY_M			0x3a
#define KEY_COMMA		0x41
#define KEY_PERIOD		0x49
#define KEY_QMARK		0x4a

/* typewriter keys - bottom row */
#define KEY_SPACE		0x29

/* Japanese keyboard (PCXAJ-AA) support */
#define KEY_MUHENKAN		0x85
#define KEY_HENKAN		0x86
#define KEY_HIRAGANA		0x87


#ifndef KEY_ESC
#define KEY_ESC KEY_F11
#endif

#ifdef KERNEL

caddr_t pcxal_init_closure();
void pcxal_init_keyboard();
void pcxal_reset_keyboard();
void pcxal_enable_keyboard();
int pcxal_set_keyboard_control();
void pcxal_ring_bell();
void pcxal_keyboard_event();
void pcxal_keyboard_char();
void pcxal_autorepeat();

#endif

/**************************************/
/* originally from io/dec/isa/xlate.h */
/**************************************/

#define UNKNOWN         0
#define SCROLL          1
#define CAPS            2
#define LSHIFT          3
#define LCTRL           4
#define RSHIFT          5
#define RCTRL           6
#define IGNORE          7
#define LALT            8
#define RALT            9
#define FUNCTION        10
#define NUMLOCK         11
#define NUMPAD          12
#define ASCII           13

typedef struct {
        short    unshifted;
        short    shifted;
        short    char_type;
} xlate_t;

extern xlate_t xlate[];

#endif /* _PCXAL_H_ */
