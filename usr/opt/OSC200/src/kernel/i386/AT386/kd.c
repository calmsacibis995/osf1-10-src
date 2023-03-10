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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: kd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:04 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/* 
 *	Olivetti Mach Console driver v0.0
 *	Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989
 *	All rights reserved.
 *
 */ 

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1988 Intel Corporation
 *	All rights reserved.
 */


#include "mach_kdb.h"

#include <sys/types.h>
#include <sys/table.h>
#include <sys/param.h>
#include <sys/conf.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/kernel.h>
#include <vm/vm_kern.h>
#include <kern/queue.h>
#include <mach/vm_param.h>
#include <i386/AT386/cram.h>
#include <i386/AT386/kd.h>
#include <i386/AT386/kdsoft.h>
#include <i386/AT386/kdsoft.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>
#include <mach_rdb.h>

#include "blit.h"
#if	NBLIT > 0
#include <i386/AT386/blitvar.h>
#else
#define blit_present()	FALSE
#define blit_init()			/* nothing */
#endif


/*
#include "evc.h"
#if	NEVC > 0
int evc1init();
#else
#define evc1init()	FALSE
#endif
*/


#define DEBUG	1			/* export feep() */

/*
 * If KD_NEEDS_VM is turned on, then cnputc will drop characters on the
 * floor until the virtual memory code is initialized.
 * This is needed to access some graphics cards which live at high 
 * addresses.
 */

#if	NBLIT > 0
#define KD_NEEDS_VM	1		/* "Mars Needs Women" */
#else
#define KD_NEEDS_VM	0
#endif

#define DEFAULT		-1		/* see kd_atoi */

void kd_enqsc();			/* enqueues a scancode */

void timeout();
void ttinit();
void ttiocom();
void ttrstrt();
void ttywait();
void wakeup();


struct tty       kd_tty;
extern int	rebootflag;


static void charput(), charmvup(), charmvdown(), charclear(), charsetcursor();
static void kd_noopreset();
boolean_t kdcheckmagic();

/* 
 * These routines define the interface to the device-specific layer.
 * See kdsoft.h for a more complete description of what each routine does.
 */

void	(*kd_dput)()	= charput;	/* put attributed char */
void	(*kd_dmvup)()	= charmvup;	/* block move up */
void	(*kd_dmvdown)()	= charmvdown;	/* block move down */
void	(*kd_dclear)()	= charclear;	/* block clear */
void	(*kd_dsetcursor)() = charsetcursor;
void	(*kd_dreset)() = kd_noopreset;	/* prepare for reboot */
				/* set cursor position on displayed page */

/* forward declarations */
unsigned char kd_getdata(), state2leds();


/*
 * Globals used for both character-based controllers and bitmap-based
 * controllers.  Default is EGA.
 */

vm_offset_t kd_bitmap_start = (vm_offset_t)0xa0000; /* XXX - put in kd.h */
u_char 	*vid_start	= (u_char *)EGA_START; 
     				/* VM start of video RAM or frame buffer */
csrpos_t kd_curpos	= 0;	/* set indirectly by kd_setpos--see kdsoft.h */
short	kd_lines	= 25;
short	kd_cols		= 80;
char	kd_attr		= KA_NORMAL;	/* current attribute */

/* 
 * kd_state shows the state of the modifier keys (ctrl, caps lock, 
 * etc.)  It should normally be changed by calling set_kd_state(), so
 * that the keyboard status LEDs are updated correctly.
 */
int  	kd_state	= KS_NORMAL;
int	kb_mode		= KB_ASCII;	/* event/ascii */

/* 
 * Some keyboard commands work by sending a command, waiting for an 
 * ack (handled by kdintr), then sending data, which generates a 
 * second ack.  If we are in the middle of such a sequence, kd_ack
 * shows what the ack is for.
 * 
 * When a byte is sent to the keyboard, it is kept around in last_sent 
 * in case it needs to be resent.
 * 
 * The rest of the variables here hold the data required to complete
 * the sequence.
 * 
 * XXX - the System V driver keeps a command queue, I guess in case we
 * want to start a command while another is in progress.  Is this
 * something we should worry about?
 */
enum why_ack {NOT_WAITING, SET_LEDS, DATA_ACK};
enum why_ack	kd_ack	= NOT_WAITING;

u_char last_sent = 0;

u_char	kd_nextled	= 0;

/*
 * We don't provide any mutex protection for this flag because we know
 * that this module will have been initialized by the time multiple
 * threads are running.
 */
boolean_t kd_initialized 	= FALSE;	/* driver initialized? */

boolean_t kd_extended	= FALSE;

/* Array for processing escape sequences. */
#define	K_MAXESC	16
u_char	esc_seq[K_MAXESC];
u_char	*esc_spt	= (u_char *)0;

/*
 * This array maps scancodes to Ascii characters (or character
 * sequences). 
 * Each row corresponds to one key.  There are NUMOUTPUT bytes per key
 * state.  The states are ordered: Normal, SHIFT, CTRL, ALT,
 * SHIFT/ALT.
 */

unsigned char	key_map[NUMKEYS][WIDTH_KMAP] = {
{NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC},
{K_ESC,NC,NC, K_ESC,NC,NC, K_ESC,NC,NC, K_ESC,NC,NC, K_ESC,NC,NC},
{K_ONE,NC,NC, K_BANG,NC,NC, K_ONE,NC,NC, 0x1b,0x4e,0x31, 0x1b,0x4e,0x21},
{K_TWO,NC,NC, K_ATSN,NC,NC, K_NUL,NC,NC, 0x1b,0x4e,0x32, 0x1b,0x4e,0x40},
{K_THREE,NC,NC, K_POUND,NC,NC, K_THREE,NC,NC, 0x1b,0x4e,0x33, 0x1b,0x4e,0x23},
{K_FOUR,NC,NC, K_DOLLAR,NC,NC, K_FOUR,NC,NC, 0x1b,0x4e,0x34, 0x1b,0x4e,0x24},
{K_FIVE,NC,NC, K_PERC,NC,NC, K_FIVE,NC,NC, 0x1b,0x4e,0x35, 0x1b,0x4e,0x25},
{K_SIX,NC,NC, K_CARET,NC,NC, K_RS,NC,NC, 0x1b,0x4e,0x36, 0x1b,0x4e,0x5e},
{K_SEVEN,NC,NC, K_AMPER,NC,NC, K_SEVEN,NC,NC, 0x1b,0x4e,0x37, 0x1b,0x4e,0x26},
{K_EIGHT,NC,NC, K_ASTER,NC,NC, K_EIGHT,NC,NC, 0x1b,0x4e,0x38, 0x1b,0x4e,0x2a},
{K_NINE,NC,NC, K_LPAREN,NC,NC, K_NINE,NC,NC, 0x1b,0x4e,0x39,0x1b,0x4e,0x28},
{K_ZERO,NC,NC, K_RPAREN,NC,NC, K_ZERO,NC,NC, 0x1b,0x4e,0x30,0x1b,0x4e,0x29},
{K_MINUS,NC,NC, K_UNDSC,NC,NC, K_US,NC,NC, 0x1b,0x4e,0x2d, 0x1b,0x4e,0x5f},
{K_EQL,NC,NC, K_PLUS,NC,NC, K_EQL,NC,NC, 0x1b,0x4e,0x3d, 0x1b,0x4e,0x2b},
{K_BS,NC,NC, K_BS,NC,NC, K_BS,NC,NC, K_BS,NC,NC, K_BS,NC,NC},
{K_HT,NC,NC, K_GS,NC,NC, K_HT,NC,NC, K_HT,NC,NC, K_GS,NC,NC},
{K_q,NC,NC, K_Q,NC,NC, K_DC1,NC,NC, 0x1b,0x4e,0x71, 0x1b,0x4e,0x51},
{K_w,NC,NC, K_W,NC,NC, K_ETB,NC,NC, 0x1b,0x4e,0x77, 0x1b,0x4e,0x57},
{K_e,NC,NC, K_E,NC,NC, K_ENQ,NC,NC, 0x1b,0x4e,0x65, 0x1b,0x4e,0x45},
{K_r,NC,NC, K_R,NC,NC, K_DC2,NC,NC, 0x1b,0x4e,0x72, 0x1b,0x4e,0x52},
{K_t,NC,NC, K_T,NC,NC, K_DC4,NC,NC, 0x1b,0x4e,0x74, 0x1b,0x4e,0x54},
{K_y,NC,NC, K_Y,NC,NC, K_EM,NC,NC, 0x1b,0x4e,0x79, 0x1b,0x4e,0x59},
{K_u,NC,NC, K_U,NC,NC, K_NAK,NC,NC, 0x1b,0x4e,0x75, 0x1b,0x4e,0x55},
{K_i,NC,NC, K_I,NC,NC, K_HT,NC,NC, 0x1b,0x4e,0x69, 0x1b,0x4e,0x49},
{K_o,NC,NC, K_O,NC,NC, K_SI,NC,NC, 0x1b,0x4e,0x6f, 0x1b,0x4e,0x4f},
{K_p,NC,NC, K_P,NC,NC, K_DLE,NC,NC, 0x1b,0x4e,0x70, 0x1b,0x4e,0x50},
{K_LBRKT,NC,NC, K_LBRACE,NC,NC, K_ESC,NC,NC, 0x1b,0x4e,0x5b, 0x1b,0x4e,0x7b},
{K_RBRKT,NC,NC, K_RBRACE,NC,NC, K_GS,NC,NC, 0x1b,0x4e,0x5d, 0x1b,0x4e,0x7d},
{K_CR,NC,NC, K_CR,NC,NC, K_CR,NC,NC, K_CR,NC,NC, K_CR,NC,NC},
{K_SCAN,K_CTLSC,NC, K_SCAN,K_CTLSC,NC, K_SCAN,K_CTLSC,NC, K_SCAN,K_CTLSC,NC, 
	 K_SCAN,K_CTLSC,NC},
{K_a,NC,NC, K_A,NC,NC, K_SOH,NC,NC, 0x1b,0x4e,0x61, 0x1b,0x4e,0x41},
{K_s,NC,NC, K_S,NC,NC, K_DC3,NC,NC, 0x1b,0x4e,0x73, 0x1b,0x4e,0x53},
{K_d,NC,NC, K_D,NC,NC, K_EOT,NC,NC, 0x1b,0x4e,0x65, 0x1b,0x4e,0x45},
{K_f,NC,NC, K_F,NC,NC, K_ACK,NC,NC, 0x1b,0x4e,0x66, 0x1b,0x4e,0x46},
{K_g,NC,NC, K_G,NC,NC, K_BEL,NC,NC, 0x1b,0x4e,0x67, 0x1b,0x4e,0x47},
{K_h,NC,NC, K_H,NC,NC, K_BS,NC,NC, 0x1b,0x4e,0x68, 0x1b,0x4e,0x48},
{K_j,NC,NC, K_J,NC,NC, K_LF,NC,NC, 0x1b,0x4e,0x6a, 0x1b,0x4e,0x4a},
{K_k,NC,NC, K_K,NC,NC, K_VT,NC,NC, 0x1b,0x4e,0x6b, 0x1b,0x4e,0x4b},
{K_l,NC,NC, K_L,NC,NC, K_FF,NC,NC, 0x1b,0x4e,0x6c, 0x1b,0x4e,0x4c},
{K_SEMI,NC,NC, K_COLON,NC,NC, K_SEMI,NC,NC, 0x1b,0x4e,0x3b, 0x1b,0x4e,0x3a},
{K_SQUOTE,NC,NC,K_DQUOTE,NC,NC,K_SQUOTE,NC,NC,0x1b,0x4e,0x27,0x1b,0x4e,0x22},
{K_GRAV,NC,NC, K_TILDE,NC,NC, K_RS,NC,NC, 0x1b,0x4e,0x60, 0x1b,0x4e,0x7e},
{K_SCAN,K_LSHSC,NC, K_SCAN,K_LSHSC,NC, K_SCAN,K_LSHSC,NC, K_SCAN,K_LSHSC,NC,
	 K_SCAN,K_LSHSC,NC},
{K_BSLSH,NC,NC, K_PIPE,NC,NC, K_FS,NC,NC, 0x1b,0x4e,0x5c, 0x1b,0x4e,0x7c},
{K_z,NC,NC, K_Z,NC,NC, K_SUB,NC,NC, 0x1b,0x4e,0x7a, 0x1b,0x4e,0x5a},
{K_x,NC,NC, K_X,NC,NC, K_CAN,NC,NC, 0x1b,0x4e,0x78, 0x1b,0x4e,0x58},
{K_c,NC,NC, K_C,NC,NC, K_ETX,NC,NC, 0x1b,0x4e,0x63, 0x1b,0x4e,0x43},
{K_v,NC,NC, K_V,NC,NC, K_SYN,NC,NC, 0x1b,0x4e,0x76, 0x1b,0x4e,0x56},
{K_b,NC,NC, K_B,NC,NC, K_STX,NC,NC, 0x1b,0x4e,0x62, 0x1b,0x4e,0x42},
{K_n,NC,NC, K_N,NC,NC, K_SO,NC,NC, 0x1b,0x4e,0x6e, 0x1b,0x4e,0x4e},
{K_m,NC,NC, K_M,NC,NC, K_CR,NC,NC, 0x1b,0x4e,0x6d, 0x1b,0x4e,0x4d},
{K_COMMA,NC,NC, K_LTHN,NC,NC, K_COMMA,NC,NC, 0x1b,0x4e,0x2c, 0x1b,0x4e,0x3c},
{K_PERIOD,NC,NC, K_GTHN,NC,NC, K_PERIOD,NC,NC,0x1b,0x4e,0x2e,0x1b,0x4e,0x3e},
{K_SLASH,NC,NC, K_QUES,NC,NC, K_SLASH,NC,NC, 0x1b,0x4e,0x2f, 0x1b,0x4e,0x3f},
{K_SCAN,K_RSHSC,NC, K_SCAN,K_RSHSC,NC, K_SCAN,K_RSHSC,NC, K_SCAN,K_RSHSC,NC, 
	 K_SCAN,K_RSHSC,NC},
{K_ASTER,NC,NC, K_ASTER,NC,NC, K_ASTER,NC,NC, 0x1b,0x4e,0x2a,0x1b,0x4e,0x2a},
{K_SCAN,K_ALTSC,NC, K_SCAN,K_ALTSC,NC, K_SCAN,K_ALTSC,NC, K_SCAN,K_ALTSC,NC, 
	 K_SCAN,K_ALTSC,NC},
{K_SPACE,NC,NC, K_SPACE,NC,NC, K_NUL,NC,NC, K_SPACE,NC,NC, K_SPACE,NC,NC},
{K_SCAN,K_CLCKSC,NC, K_SCAN,K_CLCKSC,NC, K_SCAN,K_CLCKSC,NC, 
	 K_SCAN,K_CLCKSC,NC, K_SCAN,K_CLCKSC,NC},
{K_F1, K_F1S, K_F1, K_F1, K_F1S},
{K_F2, K_F2S, K_F2, K_F2, K_F2S},
{K_F3, K_F3S, K_F3, K_F3, K_F3S},
{K_F4, K_F4S, K_F4, K_F4, K_F4S},
{K_F5, K_F5S, K_F5, K_F5, K_F5S},
{K_F6, K_F6S, K_F6, K_F6, K_F6S},
{K_F7, K_F7S, K_F7, K_F7, K_F7S},
{K_F8, K_F8S, K_F8, K_F8, K_F8S},
{K_F9, K_F9S, K_F9, K_F9, K_F9S},
{K_F10, K_F10S, K_F10, K_F10, K_F10S},
{K_SCAN,K_NLCKSC,NC, K_SCAN,K_NLCKSC,NC, K_SCAN,K_NLCKSC,NC, 
	 K_SCAN,K_NLCKSC,NC, K_SCAN,K_NLCKSC,NC},
{K_SCRL, K_NUL,NC,NC, K_SCRL, K_SCRL, K_NUL,NC,NC},
{K_HOME, K_SEVEN,NC,NC, K_HOME, K_HOME, 0x1b,0x4e,0x37},
{K_UA, K_EIGHT,NC,NC, K_UA, K_UA, 0x1b,0x4e,0x38},
{K_PUP, K_NINE,NC,NC, K_PUP, K_PUP, 0x1b,0x4e,0x39},
{0x1b,0x5b,0x53, K_MINUS,NC,NC, 0x1b,0x5b,0x53,0x1b,0x5b,0x53,0x1b,0x4e,0x2d},
{K_LA, K_FOUR,NC,NC, K_LA, K_LA, 0x1b,0x4e,0x34},
{0x1b,0x5b,0x47,K_FIVE,NC,NC,0x1b,0x5b,0x47, 0x1b,0x5b,0x47, 0x1b,0x4e,0x35},
{K_RA, K_SIX,NC,NC, K_RA, K_RA, 0x1b,0x4e,0x36},
{0x1b,0x5b,0x54,K_PLUS,NC,NC, 0x1b,0x5b,0x54, 0x1b,0x5b,0x54, 0x1b,0x4e,0x2b},
{K_END, K_ONE,NC,NC, K_END, K_END, 0x1b,0x4e,0x31},
{K_DA, K_TWO,NC,NC, K_DA, K_DA, 0x1b,0x4e,0x32},
{K_PDN, K_THREE,NC,NC, K_PDN, K_PDN, 0x1b,0x4e,0x33},
{K_INS, K_ZERO,NC,NC, K_INS, K_INS, 0x1b,0x4e,0x30},
{K_DEL,NC,NC, K_PERIOD,NC,NC, K_DEL,NC,NC, K_DEL,NC,NC, 0x1b,0x4e,0x2e},
{NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC},
{NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC},
{NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC,NC},
{K_F11, K_F11S, K_F11, K_F11, K_F11S},
{K_F12, K_F12S, K_F12, K_F12, K_F12S}
};


/*
 * Globals used only for character-based controllers.
 */

short	kd_index_reg	= EGA_IDX_REG;
short	kd_io_reg	= EGA_IO_REG;


/*
 * Globals used only for bitmap-based controllers.  See kdsoft.h for
 * an explanation of what some of these variables are used for.
 */

u_char	*font_start	= 0;		/* starting addr of font */

short	fb_width	= 0;		/* bits in frame buffer scan line */
short	fb_height	= 0;		/* scan lines in frame buffer*/
short	char_width	= 0;		/* bit width of 1 char */
short	char_height	= 0;		/* bit height of 1 char */
short	chars_in_font	= 0;
short	cursor_height	= 0;		/* bit height of cursor */

/* These initial values are simply guesses. */
u_char	char_black	= 0;
u_char	char_white	= 0xff;

short	xstart		= 0;
short	ystart		= 0;

short	char_byte_width	= 0;		/* char_width/NBBY */
short	fb_byte_width	= 0;		/* fb_width/NBBY */
short	font_byte_width	= 0;		/* num bytes in 1 scan line of font */

/*
 * Switch for poll vs. interrupt.
 */
int	kd_pollc = 0;



/*
 * cnputc:
 *
 *	This function is used to put characters out to the console,
 *	one at a time, for things like printf().  It does CR/NL
 *	mapping.
 *
 * input	: character 'c' to be put on the screen.
 * 
 * output	: character  from input is put on the screen.
 *	
 */

cnputc(c)

u_char	c;

{
	int	i;

#if	KD_NEEDS_VM
	if (kernel_map == 0 || kernel_pmap == 0 || kernel_pmap->cr3 == 0)
		return;
#endif
	kdinit();
	switch(c) {
	case K_LF:
		kd_putc(K_CR);
		kd_putc(K_LF);
		break;
	case K_HT:
		for (i = 8 - (CURRENT_COLUMN(kd_curpos) % 8); i > 0; i--)
			kd_putc(' ');
		break;
	default:
		kd_putc(c);
		break;
	}
	return;
}


/* 
 * cngetc:
 * 
 *	Get one character using polling, rather than interrupts.  Used 
 *	by the kernel debugger.  Note that Caps Lock is ignored.
 *	Normally this routine is called with interrupts already 
 *	disabled, but there is code in place so that it will be more 
 *	likely to work even if interrupts are turned on.
 */

cngetc()
{
	unsigned char	c;
	unsigned char	scancode;
	unsigned int	char_idx;
	boolean_t	up;

	kdinit();
	kd_extended = FALSE;
	for ( ; ; ) {
		while (!(inb(K_STATUS) & K_OBUF_FUL))
				;
		up = FALSE;
		scancode = inb(K_RDWR);
		if (scancode & K_UP) {
			up = TRUE;
			scancode &= ~K_UP;
		}
		if (scancode < NUMKEYS) {
			/* Lookup in map, then process. */
			char_idx = kdstate2idx(kd_state, kd_extended);
			c = key_map[scancode][char_idx];
			if (c == K_SCAN) {
				c = key_map[scancode][++char_idx];
				kd_state = do_modifier(kd_state, c, up);
			} else if (!up) {
				/* regular key-down */
				if (c == K_CR)
					c = K_LF;
				return(c & 0177);
			}
		}
	}
}


#ifdef	DEBUG
/* 
 * feep:
 *
 *	Ring the bell for a short time. 
 *	Warning: uses outb(). You may prefer to use kd_debug_put.
 */

feep()

{
	int i;

	kd_bellon();
	for (i = 0; i < 50000; ++i)
		;
	kd_belloff();
}

pause()

{
	int i;

	for (i = 0; i < 50000; ++i)
		;
}

/* 
 * Put a debugging character on the screen.
 * LOC=0 means put it in the bottom right corner, LOC=1 means put it 
 * one column to the left, etc.
 */
kd_debug_put(loc, c)

int	loc;
char	c;

{
	csrpos_t pos = ONE_PAGE - (loc+1) * ONE_SPACE;

	(*kd_dput)(pos, c, KA_NORMAL);
}
#endif /* DEBUG */


/*
 * cnpollc:
 *
 *	I think this routine is supposed to switch the keyboard
 *	between polled mode and interrupt-driven mode.  I don't know
 *	if that's important for the AT keyboard.
 */

extern int	mouse_in_use;
int		old_mouse_in_use;
int		old_kb_mode;

cnpollc(on)
boolean_t on;
{
	if (on) {
	    /* switch into X */
	    old_mouse_in_use = mouse_in_use;
	    old_kb_mode = kb_mode;
	    mouse_in_use = 0;
	    kb_mode = KB_ASCII;
	    X_kdb_enter();

	    kd_pollc++;
	}
	else {
	    --kd_pollc;

	    /* switch out of X */
	    X_kdb_exit();
	    mouse_in_use = old_mouse_in_use;
	    kb_mode = old_kb_mode;
	}
}



/*
 * kdopen:
 *
 *	This opens the console driver and sets up the tty and other
 *	rudimentary stuff including calling the line discipline for
 *	setting up the device independent stuff for a tty driver.
 *
 * input:	device number 'dev', and flag
 *
 * output:	device is opened and setup
 *
 */

/*ARGSUSED*/
kdopen(dev, flag)

int	dev;
int	flag;

{
	struct 	tty	*tp;
	int	kdstart();
	int	o_pri;

	tp = &kd_tty;
	if (!(tp->t_state & TS_ISOPEN)) {
		queue_init(&tp->t_selq);
		ttychars(tp);
                tp->t_addr = (caddr_t)tp;
		tp->t_oproc = kdstart;
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_cflag = CS8|CREAD;
		tp->t_line = 0;
		tp->t_state = TS_ISOPEN|TS_CARR_ON;
		tp->t_ospeed = tp->t_ispeed = TTYDEF_SPEED;
		ttsetwater(tp);
		kdinit();
	}
	o_pri = SPLKD();	
	tp->t_state |= TS_CARR_ON;
	(*linesw[tp->t_line].l_open)(dev, tp, flag);
	splx(o_pri);
	return(0);
}


/*
 * kdclose:
 *
 *	This function merely executes the device independent code for
 *	closing the line discipline.
 *
 * input:	device number 'dev', and flag
 * 
 * output:	device is closed
 *
 */

/*ARGSUSED*/
kdclose(dev, flag)

int	dev;
int	flag;

{
	struct	tty	*tp;

	tp = &kd_tty;
	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);
	return;

}


/*
 * kdread:
 *
 *	This function executes the device independent code to read from
 *	the tty.
 *
 * input:	device number 'dev'
 *
 * output:	characters are read from tty clists
 *
 */

/*ARGSUSED*/
kdread(dev, uio, flag)

int	dev;
struct	uio	*uio;
int flag;

{
	struct	tty	*tp;
  
	tp = &kd_tty;
	tp->t_state |= TS_CARR_ON;
	return((*linesw[kd_tty.t_line].l_read)(tp, uio, flag));
}


/*
 * kdwrite:
 *
 *	This function does the device independent write action for this
 *	console (tty) driver.
 *
 * input:	device number 'dev'
 *
 * output:	characters are written to tty clists
 *
 */

/*ARGSUSED*/
kdwrite(dev, uio,flag)

int	dev;
struct	uio	*uio;
int flag;

{
	return((*linesw[kd_tty.t_line].l_write)(&kd_tty, uio,flag));
}

/* 
 * Mmap.
 */

/*ARGSUSED*/
int
kdmmap(dev, off, prot)
	dev_t dev;
	off_t off;
	int prot;
{
	if ((u_int) off >= (128*1024))
		return(-1);

	/* Get page frame number for the page to be mapped. */
	return(i386_btop(kd_bitmap_start+off));
}

/*
 * kdioctl:
 *
 *	This function executes the device independent code to manipulate 
 *	the tty, plus it provides ioctl's that are specific to the
 *	console. 
 *
 * input:	device number 'dev', command 'cmd', pointer to arguments 
 *		'addr', mode
 *
 * output:	dependent and independent tty manipulation
 *
 */

/*ARGSUSED*/
int
kdioctl(dev, cmd, addr, mode)

int	dev;
int	cmd;
caddr_t	addr;
int	mode;

{
	register    int	err = -1;
	struct	    tty *tp = &kd_tty;

	/* 
	 * First look for an ioctl that is specific to the kd driver.  
	 * If we don't find one (err == -1), try the line discipline
	 * ioctl's and then the generic tty ioctl's.
	 */

	switch(cmd) {
        case TCSBREAK:
	case TIOCSBRK:
	case TIOCCBRK:
	case TIOCSDTR:
	case TIOCCDTR:
		err = 0;
		break;

	case KDGSTATE:
		*addr = kd_state;
		err = 0;
		break;

	case KDGKBENT:			/* get entry from key_map */
		err = kdgetkbent((struct kbentry *)addr);
		break;

	case KDSKBENT:			/* set entry in key_map */
		err = kdsetkbent((struct kbentry *)addr, mode);
		break;

	case KDSETBELL:
		err = kdsetbell(*addr, mode);
		break;
	}
	if (err >= 0)
		return(err);

	if ((err = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr, mode)) >= 0)
		return(err);

	if ((err = ttioctl(tp, cmd, addr, mode)) >= 0)
		return(err);

	return(ENOTTY);
}


/* 
 * kdsetbell:
 * 
 *	Turn the bell on or off.  Returns error code, if given bogus 
 *	on/off value.
 */

int 
kdsetbell(val, flags)

int	val;				/* on or off */
int	flags;				/* flags set for console */

{
	int err = 0;

	if ((flags & (FWRITE | FREAD)) != (FWRITE | FREAD))
		return(EACCES);

	if (val == KD_BELLON)
		kd_bellon();
	else if (val == KD_BELLOFF)
		kd_belloff();
	else
		err = EINVAL;

	return(err);
}


/* 
 * kdgetkbent:
 * 
 *	Get entry from key mapping table.  Returns error code, if any.
 */

int
kdgetkbent(kbent)

struct kbentry *	kbent;

{
	u_char *cp;
	int o_pri = SPLKD();		/* probably superfluous */

	cp = &key_map[kbent->kb_index][CHARIDX(kbent->kb_state)];
	kbent->kb_value[0] = *cp++;
	kbent->kb_value[1] = *cp++;
	kbent->kb_value[2] = *cp;
	(void)splx(o_pri);
	return(0);
}


/* 
 * kdsetkbent:
 * 
 *	Set entry in key mapping table.  Return error code, if any.
 */

int
kdsetkbent(kbent, flags)

struct kbentry *	kbent;
int	flags;				/* flags set for console */

{
	u_char *cp;
	int o_pri;

	if ((flags & (FWRITE | FREAD)) != (FWRITE | FREAD))
		return(EACCES);

	o_pri = SPLKD();
	cp = &key_map[kbent->kb_index][CHARIDX(kbent->kb_state)];
	*cp++ = kbent->kb_value[0];
	*cp++ = kbent->kb_value[1];
	*cp = kbent->kb_value[2];
	(void)splx(o_pri);
	return(0);
}


/*
 * kdintr:
 *
 *	This function is the interrupt code for the driver.  Since this is
 *	a special tty (console), interrupts are only for input, so we read in
 *	the character.  If in ascii mode, we then do the mapping translation 
 *	from the keyboard switch table and place the characters on the tty's 
 *	input switch table.  If in event mode, we create and queue a kd_event.
 *
 * input:	interrupt vector 'vec'
 *
 * output:	character or sequence is placed on appropriate queue
 *
 */

/*ARGSUSED*/
kdintr(unit)
int	unit;
{
	struct	tty	*tp;
	unsigned char	c;
	unsigned char	scancode;
	int	o_pri;
	int	char_idx;
	boolean_t	up = FALSE;		/* key-up event */

        if (kd_pollc)				/* kdb polling kdb */
                return(0);
        
	tp = &kd_tty;
	o_pri = SPLKD();
	scancode = inb(K_RDWR);
	if (scancode == K_EXTEND) {
		if (kb_mode != KB_EVENT)
			kd_extended = TRUE;
		goto done;
	} else if (scancode == K_RESEND) {
		kd_resend();
		goto done;
	} else if (scancode == K_ACKSC) {
		kd_handle_ack();
		goto done;
	} else if (kdcheckmagic(scancode)) {
		goto done;
	} else if (kb_mode == KB_EVENT) {
		kd_enqsc(scancode);
		goto done;
	} /* else... */

	if (scancode & K_UP) {
		up = TRUE;
		scancode &= ~K_UP;
	}
	if (scancode < NUMKEYS) {
		/* Lookup in map, then process. */
		char_idx = kdstate2idx(kd_state, kd_extended);
		c = key_map[scancode][char_idx];
		if (c == K_SCAN) {
			c = key_map[scancode][++char_idx];
			set_kd_state(do_modifier(kd_state, c, up));
		} else if (!up) {
			/* regular key-down */
			int max;	/* max index for char sequence */

			max = char_idx + NUMOUTPUT;
			char_idx++;
			if (!kd_extended) {
				if (kd_state&KS_CLKED) {
 					if (kd_isupper(c)) {
						c += ('a' - 'A');
						max = char_idx;
					}
					else if (kd_islower(c)) {
						c -= ('a' - 'A');
						max = char_idx;
					}
				}
				/*
				 * Notice that even if the keypad is remapped,
				 * NumLock only effects the keys that are
				 * physically part of the keypad.  Is this
				 * The Right Thing?
				 */
				if ((kd_state&KS_NLKED) &&
			    	    (((K_HOMESC) <= scancode) &&
			     	    (scancode <= (K_DELSC)))) {
					char_idx = CHARIDX(SHIFT_STATE);
					c = key_map[scancode][char_idx];
					max = char_idx + NUMOUTPUT;
					char_idx++;
				}
			}

			/* 
			 * here's where we actually put the char (or
			 * char sequence, for function keys) onto the
			 * input queue.
			 */
			for ( ; (c != K_DONE) && (char_idx <= max); 
			     c = key_map[scancode][char_idx++]) {	
				(*linesw[tp->t_line].l_rint)(c, tp);
			}
			kd_extended = FALSE;
		}
	}

 done:
	splx(o_pri);
	return(1);
}


/* 
 * kd_handle_ack:
 * 
 *	For pending commands, complete the command.  For data bytes, 
 *	drop the ack on the floor.
 */

kd_handle_ack()

{
	switch (kd_ack) {
	case SET_LEDS:
		kd_setleds2();
		kd_ack = DATA_ACK;
		break;
	case DATA_ACK:
		kd_ack = NOT_WAITING;
		break;
	case NOT_WAITING:
		printf("unexpected ACK from keyboard\n");
		break;
	default:
		panic("bogus kd_ack\n");
		break;
	}
}


/* 
 * kd_resend:
 *
 *	Resend a missed keyboard command or data byte.
 */

kd_resend()

{
	if (kd_ack == NOT_WAITING) 
		printf("unexpected RESEND from keyboard\n");
	else
		kd_senddata(last_sent);
}


/*
 * do_modifier:
 *
 *	Change keyboard state according to which modifier key and
 *	whether it went down or up.
 *
 * input:	the current state, the key, and the key's direction.  
 *		The key can be any key, not just a modifier key.
 * 
 * output:	the new state
 */
int
do_modifier(state, c, up)

int	state;
Scancode	c;
boolean_t	up;

{
	switch (c) {
	case (K_ALTSC):
		if (up)
			state &= ~KS_ALTED;
		else
			state |= KS_ALTED;
		kd_extended = FALSE;
		break;
#ifndef ORC
	case (K_CLCKSC):
#endif ORC
	case (K_CTLSC):
		if (up)
			state &= ~KS_CTLED;
		else
			state |= KS_CTLED;
		kd_extended = FALSE;
		break;
#ifdef ORC
	case (K_CLCKSC):
		if (!up)
			state ^= KS_CLKED;
		break;
#endif ORC
	case (K_NLCKSC):
		if (!up)
			state ^= KS_NLKED;
		break;
	case (K_LSHSC):
	case (K_RSHSC):
		if (up)
			state &= ~KS_SHIFTED;
		else
			state |= KS_SHIFTED;
		kd_extended = FALSE;
		break;
	}

	return(state);
}


/* 
 * kdcheckmagic:
 * 
 *	Check for magic keystrokes for invoking the debugger or 
 *	rebooting or ...
 *
 * input:	an unprocessed scancode
 * 
 * output:	TRUE if a magic key combination was recognized and 
 *		processed.  FALSE otherwise.
 *
 * side effects:	
 *		various actions possible, depending on which keys are
 *		pressed.  If the debugger is called, steps are taken 
 *		to ensure that the system doesn't think the magic keys 
 *		are still held down.
 */

boolean_t
kdcheckmagic(scancode)

Scancode	scancode;

{
	static int magic_state = KS_NORMAL; /* like kd_state */
	boolean_t up = FALSE;
	extern	int rebootflag;

	if (scancode & K_UP) {
		up = TRUE;
		scancode &= ~K_UP;
	}
	magic_state = do_modifier(magic_state, scancode, up);

	if ((magic_state&(KS_CTLED|KS_ALTED)) == (KS_CTLED|KS_ALTED)) {
		switch (scancode) {
#if	MACH_KDB || MACH_RDB
		case K_dSC:		/*  ctl-alt-d */
#if MACH_RDB
			int3();
#else
                        Bpt();
#endif
                        /* invoke debugger */
			/* Returned from debugger, so reset kbd state. */
			(void)SPLKD();
			magic_state = KS_NORMAL;
			if (kb_mode == KB_ASCII)
				set_kd_state(KS_NORMAL);
			else {
				kd_enqsc(K_ALTSC | K_UP);
				kd_enqsc(K_CTLSC | K_UP);
				kd_enqsc(K_dSC | K_UP);
			}
			return(TRUE);
			break;
#endif	MACH_KDB
		case K_DELSC:		/* ctl-alt-del */
			/* if rebootflag is on, reboot the system */
			if (rebootflag)
				kdreboot();
			break;
		}
	}
	return(FALSE);
}


/*
 * kdstate2idx:
 *
 *	Return the value for the 2nd index into key_map that 
 *	corresponds to the given state.
 */

int
kdstate2idx(state, extended)

int	state;				/* bit vector, not a state index */
boolean_t	extended;

{
	int state_idx = NORM_STATE;

	if ((!extended) && state != KS_NORMAL) {
		if ((state&(KS_SHIFTED|KS_ALTED)) == (KS_SHIFTED|KS_ALTED))
			state_idx = SHIFT_ALT;
		else if (state&KS_SHIFTED)
			state_idx = SHIFT_STATE;
		else if (state&KS_ALTED)
			state_idx = ALT_STATE;
		else if (state&KS_CTLED)
			state_idx = CTRL_STATE;
	}

	return (CHARIDX(state_idx));
}


/*
 * kdstart:
 *
 *	This function does the general processing of characters and other
 *	operations for the device driver.  The device independent portion of
 *	the tty driver calls this routine (it's setup in kdinit) with a
 *	given command.  That command is then processed, and control is passed
 *	back to the kernel.
 *
 * input:	tty pointer 'tp', and command to execute 'cmd'
 *
 * output:	command is executed
 *
 */

kdstart(tp)

struct	tty	*tp;

{
	int	o_pri;
	int	ch;
	unsigned char	c;
  
	if (tp->t_state & TS_TTSTOP)
		return;
	for ( ; ; ) {
		tp->t_state &= ~TS_BUSY; 
		if (tp->t_state & TS_TTSTOP)
			break;
		o_pri = spltty();
		if ((tp->t_outq.c_cc <= 0) || (ch = getc(&tp->t_outq)) == -1) {
			splx(o_pri);
			break;
		}
		c = ch;
		if (c == (K_ESC)) {
			if (esc_spt == esc_seq) {
				*(esc_spt++)=(K_ESC);
				*(esc_spt) = '\0';
			}
			else {
				kd_putc((K_ESC));
				esc_spt = esc_seq;
			}
		}
		else {
			if (esc_spt - esc_seq) {
				*(esc_spt++) = c;
				*(esc_spt) = '\0';
				kd_parseesc();
			}
			else {
			        kd_putc(c);
			}
		}
		splx(o_pri);
	}
	o_pri = spltty();
	if (tp->t_outq.c_cc <= TTMINLOWAT) {
		if (tp->t_state & TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		select_wakeup(&tp->t_selq);
	}
	splx(o_pri);
	return;

}


/*
 * kdinit:
 *
 *	This code initializes the structures and sets up the port registers
 *	for the console driver.  
 *
 *	Each bitmap-based graphics card is likely to require a unique
 *	way to determine the card's presence.  The driver runs through
 *	each "special" card that it knows about and uses the first one
 *	that it finds.  If it doesn't find any, it assumes that an
 *	EGA-like card is installed.
 *
 * input	: None.	Interrupts are assumed to be disabled
 * output	: Driver is initialized
 *
 */

static ihandler_t kd_handler;

kdinit()

{
	void	kd_xga_init();
	unsigned char	k_comm;		/* keyboard command byte */
	ihandler_id_t *kd_handler_id;

	if (kd_initialized)
		return;

	esc_spt = esc_seq;
	kd_attr = KA_NORMAL;

	/*
	 * board specific initialization: set up globals and kd_dxxx
	 * pointers, and synch displayed cursor with logical cursor.
	 */

/*	if(!evc1init())*/
	if (blit_present())
		blit_init();
	else
		kd_xga_init();

	/* get rid of any garbage in output buffer */
	if (inb(K_STATUS) & K_OBUF_FUL)
		(void)inb(K_RDWR);

	kd_sendcmd(KC_CMD_READ);	/* ask for the ctlr command byte */
	k_comm = kd_getdata();
	k_comm &= ~K_CB_DISBLE;		/* clear keyboard disable bit */
	k_comm |= K_CB_ENBLIRQ;		/* enable interrupt */
	kd_sendcmd(KC_CMD_WRITE);	/* write new ctlr command byte */
	kd_senddata(k_comm);

	kd_initialized = TRUE;
	/* intpri[1] statically initialized to SPL6 */
        kd_handler.ih_level = 1;
        kd_handler.ih_handler = kdintr;
        kd_handler.ih_resolver = 0;
        kd_handler.ih_stats.intr_type = INTR_DEVICE;
        kd_handler.ih_stats.intr_cnt = 0;
	kd_handler.ih_hparam[0].intparam = 0;
        if ((kd_handler_id = handler_add(&kd_handler)) != NULL)
                handler_enable(kd_handler_id);
        else
                panic("Unable to add keyboard interrupt handler");
	set_kd_state(KS_NORMAL);
	return;
}


/*
 * kd_belloff:
 *
 *	This routine shuts the bell off, by sending the appropriate code
 *	to the speaker port.
 *
 * input	: None
 * output	: bell is turned off
 *
 */

kd_belloff()

{
	unsigned char status;

	status = (inb(K_PORTB) & ~(K_SPKRDATA | K_ENABLETMR2));
	outb(K_PORTB, status);
	return;
}


/*
 * kd_bellon:
 *
 *	This routine turns the bell on.
 *
 * input	: None
 * output	: bell is turned on
 *
 */

kd_bellon()

{
	unsigned char	status;

	/* program timer 2 */
	outb(K_TMRCTL, K_SELTMR2 | K_RDLDTWORD | K_TSQRWAVE | K_TBINARY);
	outb(K_TMR2, 1500 & 0xff);	/* LSB */
	outb(K_TMR2, (int)1500 >> 8);	/* MSB */

	/* start speaker - why must we turn on K_SPKRDATA? */
	status = (inb(K_PORTB)| K_ENABLETMR2 | K_SPKRDATA);
	outb(K_PORTB, status);
	return;
}



/*
 *
 * Function kd_putc():
 *
 *	This function simply puts a character on the screen.  It does some
 *	special processing for linefeed, carriage return, backspace and
 *	the bell.
 *
 * input	: character to be displayed
 * output	: character is displayed, or some action is taken
 *
 */

int sit_for_0 = 1;

kd_putc(ch)

u_char	ch;

{
        int i;
        
	if ((!ch) && sit_for_0)
		return;

	switch (ch) { 
	case K_LF:
		kd_down();
		break;
	case K_CR:
		kd_cr();
		break;
	case K_HT:
		i = 8 - (CURRENT_COLUMN(kd_curpos) & 7);
		while (i-- > 0)
			kd_right();
		break;
	case K_BS:
		kd_left();
		break;
	case K_BEL:
		/*
		 * Similar problem to K_BS here (behavior might depend
		 * on tty setting).  Also check LF and CR.
		 */
		kd_bellon();
		timeout(kd_belloff, 0, hz/8 );
		break;
        case K_FF:
                kd_cls();
		kd_home();
                break;
	default:
		(*kd_dput)(kd_curpos, ch, kd_attr);
		kd_right();
		break;
	}
	return;
}


/*
 * kd_setpos:
 *
 *	This function sets the software and hardware cursor position
 *	on the screen, using device-specific code to actually move and
 *	display the cursor.
 *
 * input	: position on (or off) screen to move the cursor to
 * output	: cursor position is updated, screen has been scrolled
 *		  if necessary to bring cursor position back onto
 *		  screen.
 *
 */

kd_setpos(newpos)

csrpos_t newpos;

{
	if (newpos > ONE_PAGE) {
		kd_scrollup();
		newpos = BOTTOM_LINE;
	}
	if (newpos < 0) {
		kd_scrolldn();
		newpos = 0;
	}

	(*kd_dsetcursor)(newpos);
}


/*
 * kd_scrollup:
 *
 *	This function scrolls the screen up one line using a DMA memory
 *	copy.
 *
 * input	: None
 * output	: lines on screen appear to be shifted up one line
 *
 */

kd_scrollup()

{
	csrpos_t to;
	csrpos_t from;
	int	count;

	/* scroll up */
	to = 0;
	from = ONE_LINE;
	count = (ONE_PAGE - ONE_LINE)/ONE_SPACE;
	(*kd_dmvup)(from, to, count);

	/* clear bottom line */
	to = BOTTOM_LINE;
	count = ONE_LINE/ONE_SPACE;
	(*kd_dclear)(to, count, kd_attr);
	return;
}


/*
 * kd_scrolldn:
 *
 *	Scrolls the characters on the screen down one line.
 *
 * input	: None
 * output	: Lines on screen appear to be moved down one line
 *
 */

kd_scrolldn()

{
	csrpos_t to;
	csrpos_t from;
	int	count;

	/* move down */
	to 	= ONE_PAGE - ONE_SPACE;
	from 	= ONE_PAGE - ONE_LINE - ONE_SPACE;
	count 	= (ONE_PAGE - ONE_LINE) / ONE_SPACE;
	(*kd_dmvdown)(from, to, count);

	/* clear top line */
	to	= 0;
	count	= ONE_LINE/ONE_SPACE;
	(*kd_dclear)(to, count, kd_attr);
	return;
	
}


/*
 * kd_parseesc:
 *
 *	This routine begins the parsing of an escape sequence.  It uses the
 *	escape sequence array and the escape spot pointer to handle 
 *	asynchronous parsing of escape sequences.
 *
 * input	: String of characters prepended by an escape
 * output	: Appropriate actions are taken depending on the string as
 *		  defined by the ansi terminal specification
 *
 */

kd_parseesc()

{
	u_char	*escp;

	escp = esc_seq + 1;		/* point to char following ESC */
	switch(*(escp)) {
	case 'c':
		kd_cls();
		kd_home();
		esc_spt = esc_seq;  /* reset spot in ESC sequence */
		break;
	case '[':
		escp++;
		kd_parserest(escp);
		break;
	case '\0':
		break;			/* not enough info yet	*/
	default:
		kd_putc(*escp);
		esc_spt = esc_seq;	/* inv sequence char, reset */
		break;
	}
	return;

}


/*
 * kd_parserest:
 *
 *	This function will complete the parsing of an escape sequence and
 *	call the appropriate support routine if it matches a character.  This
 *	function could be greatly improved by using a function jump table, and
 *	removing this bulky switch statement.
 *
 * input	: An string
 * output	: Appropriate action based on whether the string matches a
 *	 	  sequence acceptable to the ansi terminal specification
 *
 */

kd_parserest(cp)

u_char	*cp;

{
	int	number;
	csrpos_t newpos;

	cp += kd_atoi(cp, &number);
	switch(*cp) {
	case 'm':
		switch(number) {
		case DEFAULT:
		case 0:
			kd_attr = KA_NORMAL;
			break;
		case 7:
			kd_attr = KA_REVERSE;
			break;
		default:
			kd_attr = KA_NORMAL;
			break;
		}
		esc_spt = esc_seq;
		break;
	case '@':
		if (number == DEFAULT)
			kd_insch(1);
		else
			kd_insch(number);
		esc_spt = esc_seq;
		break;
	case 'H':
		kd_home();
		esc_spt = esc_seq;
		break;
	case 'A':
		if (number == DEFAULT)
			kd_up();
		else
			while (number--)
				kd_up();
		esc_spt = esc_seq;
		break;
	case 'B':
		if (number == DEFAULT)
			kd_down();
		else
			while (number--)
				kd_down();
		esc_spt = esc_seq;
		break;
	case 'C':
		if (number == DEFAULT)
			kd_right();
		else
			while (number--)
				kd_right();
		esc_spt = esc_seq;
		break;
	case 'D':
		if (number == DEFAULT)
			kd_left();
		else
			while (number--)
				kd_left();
		esc_spt = esc_seq;
		break;
	case 'E':
		kd_cr();
		if (number == DEFAULT)
			kd_down();
		else
			while (number--)
				kd_down();
		esc_spt = esc_seq;
		break;
	case 'F':
		kd_cr();
		if (number == DEFAULT)
			kd_up();
		else
			while (number--)
				kd_up();
		esc_spt = esc_seq;
		break;
	case 'G':
		if (number == DEFAULT)
			number = 0;
		else
			if (number > 0)
				--number;	/* because number is from 1 */
		kd_setpos(BEG_OF_LINE(kd_curpos) + number * ONE_SPACE);
		esc_spt = esc_seq;
		break;
	case ';':
		++cp;
		if (*cp == '\0')
			break;			/* not ready yet */
		if (number == DEFAULT)
			number = 0;
		else
			if (number > 0)
				--number;		/* numbered from 1 */
		newpos = (number * ONE_LINE);   /* setup row */
		cp += kd_atoi(cp, &number);
		if (*cp == '\0')
			break;			/* not ready yet */
		if (number == DEFAULT)
			number = 0;
		else if (number > 0)
			number--;
		newpos += (number * ONE_SPACE);	/* setup column */
		if (newpos < 0)
			newpos = 0;		/* upper left */
		if (newpos > ONE_PAGE)
			newpos = (ONE_PAGE - ONE_SPACE); 
		/* lower right */
		if (*cp == '\0')
			break;			/* not ready yet */
		if (*cp == 'H') {
			kd_setpos(newpos);
			esc_spt = esc_seq;	/* done, reset */
		}	
		else
			esc_spt = esc_seq;
		break;				/* done or not ready */
	case 'J':
		switch(number) {
		case DEFAULT:
		case 0:
			kd_cltobcur();	/* clears from current
					   pos to bottom.
					   */	
			break;
		case 1:
			kd_cltopcur();	/* clears from top to
					   current pos.
					   */	
			break;
		case 2:
			kd_cls();
			break;
		default:
			break;
		}
		esc_spt = esc_seq;		/* reset it */
		break;
	case 'K':
		switch(number) {
		case DEFAULT:
		case 0:
			kd_cltoecur();	/* clears from current
					   pos to eoln.
					   */	
			break;
		case 1:
			kd_clfrbcur();	/* clears from begin
					   of line to current
					   pos.
					   */
			break;
		case 2:
			kd_eraseln();	/* clear entire line */
			break;
		default:
			break;
		}
		esc_spt = esc_seq;
		break;
	case 'L':
		if (number == DEFAULT)
			kd_insln(1);
		else
			kd_insln(number);
		esc_spt = esc_seq;
		break;
	case 'M':
		if (number == DEFAULT)
			kd_delln(1);
		else
			kd_delln(number);
		esc_spt = esc_seq;
		break;
	case 'P':
		if (number == DEFAULT)
			kd_delch(1);
		else
			kd_delch(number);
		esc_spt = esc_seq;
		break;
	case 'S':
		if (number == DEFAULT)
			kd_scrollup();
		else
			while (number--)
				kd_scrollup();
		esc_spt = esc_seq;
		break;
	case 'T':
		if (number == DEFAULT)
			kd_scrolldn();
		else
			while (number--)
				kd_scrolldn();
		esc_spt = esc_seq;
		break;
	case 'X':
		if (number == DEFAULT)
			kd_erase(1);
		else
			kd_erase(number);
		esc_spt = esc_seq;
		break;	
	case '\0':
		break;			/* not enough yet */
	default:
		kd_putc(*cp);		/* show inv character */
		esc_spt = esc_seq;	/* inv entry, reset */
		break;
	}
	return;
}



/*
 * kd_atoi:
 *
 *	This function converts an ascii string into an integer, and
 *	returns DEFAULT if no integer was found.  Note that this is why
 *	we don't use the regular atio(), because ZERO is ZERO and not
 *	the DEFAULT in all cases.
 *
 * input	: string
 * output	: a number or possibly DEFAULT, and the count of characters
 *		  consumed by the conversion
 *
 */

int
kd_atoi(cp, nump)

u_char	*cp;
int	*nump;

{
	int	number;
	u_char	*original;

	original = cp;
	for (number = 0; ('0' <= *cp) && (*cp <= '9'); cp++)
		number = (number * 10) + (*cp - '0');
	if (original == cp)
		*nump = DEFAULT;
	else
		*nump = number;
	return(cp - original);
}


/*
 * kd_cls:
 *
 *	This function clears the screen with spaces and the current attribute.
 *
 * input	: None
 * output	: Screen is cleared
 *
 */

kd_cls()

{
	(*kd_dclear)(0, ONE_PAGE/ONE_SPACE, kd_attr);
	return;
}


/*
 * kd_home:
 *
 *	This function will move the cursor to the home position on the screen,
 *	as well as set the internal cursor position (kd_curpos) to home.
 *
 * input	: None
 * output	: Cursor position is moved
 *
 */

kd_home()

{
	kd_setpos(0);
	return;
}


/*
 * kd_up:
 *
 *	This function moves the cursor up one line position.
 *
 * input	: None
 * output	: Cursor moves up one line, or screen is scrolled
 *
 */

kd_up()

{
	if (kd_curpos < ONE_LINE)
		kd_scrolldn();
	else
		kd_setpos(kd_curpos - ONE_LINE);
	return;
}


/*
 * kd_down:
 *
 *	This function moves the cursor down one line position.
 *
 * input	: None
 * output	: Cursor moves down one line or the screen is scrolled
 *
 */

kd_down()

{
	if (kd_curpos >= (ONE_PAGE - ONE_LINE))
		kd_scrollup();
	else
		kd_setpos(kd_curpos + ONE_LINE);
	return;
}


/*
 * kd_right:
 *
 *	This function moves the cursor one position to the right.
 *
 * input	: None
 * output	: Cursor moves one position to the right
 *
 */

kd_right()

{
	if (kd_curpos < (ONE_PAGE - ONE_SPACE))
		kd_setpos(kd_curpos + ONE_SPACE);
	else {
		kd_scrollup();
		kd_setpos(BEG_OF_LINE(kd_curpos));
	}
	return;
}


/*
 * kd_left:
 *
 *	This function moves the cursor one position to the left.
 *
 * input	: None
 * output	: Cursor moves one position to the left
 *
 */

kd_left()

{
	if (0 < kd_curpos)
		kd_setpos(kd_curpos - ONE_SPACE);
	return;
}


/*
 * kd_cr:
 *
 *	This function moves the cursor to the beginning of the current
 *	line.
 *
 * input	: None
 * output	: Cursor moves to the beginning of the current line
 *
 */

kd_cr()

{
	kd_setpos(BEG_OF_LINE(kd_curpos));
	return;
}


/*
 * kd_cltobcur:
 *
 *	This function clears from the current cursor position to the bottom
 *	of the screen.
 *
 * input	: None
 * output	: Screen is cleared from current cursor postion to bottom
 *
 */

kd_cltobcur()

{
	csrpos_t start;
	int	count;

	start = kd_curpos;
	count = (ONE_PAGE - kd_curpos)/ONE_SPACE;
	(*kd_dclear)(start, count, kd_attr);
	return;
}


/*
 * kd_cltopcur:
 *
 *	This function clears from the current cursor position to the top
 *	of the screen.
 *
 * input	: None
 * output	: Screen is cleared from current cursor postion to top
 *
 */

kd_cltopcur()

{
	int	count;

	count = (kd_curpos + ONE_SPACE) / ONE_SPACE;
	(*kd_dclear)(0, count, kd_attr);
	return;
}


/*
 * kd_cltoecur:
 *
 *	This function clears from the current cursor position to eoln. 
 *
 * input	: None
 * output	: Line is cleared from current cursor position to eoln
 *
 */

kd_cltoecur()

{
	csrpos_t i;
	csrpos_t hold;

	hold = BEG_OF_LINE(kd_curpos) + ONE_LINE;
	for (i = kd_curpos; i < hold; i += ONE_SPACE) {
		(*kd_dput)(i, K_SPACE, kd_attr);
	}
}


/*
 * kd_clfrbcur:
 *
 *	This function clears from the beginning of the line to the current
 *	cursor position.
 *
 * input	: None
 * output	: Line is cleared from beginning to current position
 *
 */

kd_clfrbcur()

{
	csrpos_t i;

	for (i = BEG_OF_LINE(kd_curpos); i <= kd_curpos; i += ONE_SPACE) {
		(*kd_dput)(i, K_SPACE, kd_attr);
	}
}


/*
 * kd_delln:
 *
 *	This function deletes 'number' lines on the screen by effectively
 *	scrolling the lines up and replacing the old lines with spaces.
 *
 * input	: number of lines to delete
 * output	: lines appear to be deleted
 *
 */

kd_delln(number)

int	number;

{
	csrpos_t to;
	csrpos_t from;
	int	delbytes;		/* num of bytes to delete */
	int	count;			/* num of words to move or fill */

	if (number <= 0)
		return;

	delbytes = number * ONE_LINE;
	to = BEG_OF_LINE(kd_curpos);
	if (to + delbytes >= ONE_PAGE)
		delbytes = ONE_PAGE - to;
	if (to + delbytes < ONE_PAGE) {
		from = to + delbytes;
		count = (ONE_PAGE - from) / ONE_SPACE;
		(*kd_dmvup)(from, to, count);
	}

	to = ONE_PAGE - delbytes;
	count = delbytes / ONE_SPACE;
	(*kd_dclear)(to, count, kd_attr);
	return;
}


/*
 * kd_insln:
 *
 *	This function inserts a line above the current one by
 *	scrolling the current line and all the lines below it down.
 *
 * input	: number of lines to insert
 * output	: New lines appear to be inserted
 *
 */

kd_insln(number)

int	number;

{
	csrpos_t to;
	csrpos_t from;
	int	count;
	csrpos_t top;			/* top of block to be moved */
	int	insbytes;		/* num of bytes inserted */

	if (number <= 0)
		return;

	top = BEG_OF_LINE(kd_curpos);
	insbytes = number * ONE_LINE;
	if (top + insbytes > ONE_PAGE)
		insbytes = ONE_PAGE - top;
	to = ONE_PAGE - ONE_SPACE;
	from = to - insbytes;
	if (from > top) {
		count = (from - top + ONE_SPACE) / ONE_SPACE;
		(*kd_dmvdown)(from, to, count);
	}

	count = insbytes / ONE_SPACE;
	(*kd_dclear)(top, count, kd_attr);
	return;
}


/*
 * kd_delch:
 *
 *	This function deletes a number of characters from the current 
 *	position in the line.
 *
 * input	: number of characters to delete
 * output	: characters appear to be deleted
 *
 */

kd_delch(number)

int	number;

{
	int	count;			/* num words moved/filled */
	int	delbytes;		/* bytes to delete */
	register csrpos_t to;
	csrpos_t from;
	csrpos_t nextline;		/* start of next line */

	if (number <= 0)
		return;

	nextline = BEG_OF_LINE(kd_curpos) + ONE_LINE;
	delbytes = number * ONE_SPACE;
	if (kd_curpos + delbytes > nextline)
		delbytes = nextline - kd_curpos;
	if (kd_curpos + delbytes < nextline) {
		from = kd_curpos + delbytes;
		to = kd_curpos;
		count = (nextline - from) / ONE_SPACE;
		(*kd_dmvup)(from, to, count);
	}

	to = nextline - delbytes;
	count = delbytes / ONE_SPACE;
	(*kd_dclear)(to, count, kd_attr);
	return;

}


/*
 * kd_erase:
 *
 *	This function overwrites characters with a space starting with the
 *	current cursor position and ending in number spaces away.
 *
 * input	: number of characters to erase
 * output	: characters appear to be blanked or erased
 *
 */

kd_erase(number)

int	number;

{
	csrpos_t i;
	csrpos_t stop;

	stop = kd_curpos + (ONE_SPACE * number);	
	if (stop > BEG_OF_LINE(kd_curpos) + ONE_LINE)
		stop = BEG_OF_LINE(kd_curpos) + ONE_LINE;
	for (i = kd_curpos; i < stop; i += ONE_SPACE) {
		(*kd_dput)(i, K_SPACE, kd_attr);
	}
	return;
}


/*
 * kd_eraseln:
 *
 *	This function erases the current line with spaces.
 *
 * input	: None
 * output	: Current line is erased
 *
 */

kd_eraseln()

{
	csrpos_t i;
	csrpos_t stop;

	stop = BEG_OF_LINE(kd_curpos) + ONE_LINE;
	for (i = BEG_OF_LINE(kd_curpos); i < stop; i += ONE_SPACE) {	
		(*kd_dput)(i, K_SPACE, kd_attr);
	}
	return;
}


/*
 * kd_insch:
 *
 *	This function inserts a blank at the current cursor position
 *	and moves all other characters on the line over.
 *
 * input	: number of blanks to insert
 * output	: Blanks are inserted at cursor position
 *
 */

kd_insch(number)

int	number;

{
	csrpos_t to;
	csrpos_t from;
	int	count;
	csrpos_t nextline;		/* start of next line */
	int	insbytes;		/* num of bytes inserted */

	if (number <= 0)
		return;

	nextline = BEG_OF_LINE(kd_curpos) + ONE_LINE;
	insbytes = number * ONE_SPACE;
	if (kd_curpos + insbytes > nextline)
		insbytes = nextline - kd_curpos;

	to = nextline - ONE_SPACE;
	from = to - insbytes;
	if (from >= kd_curpos) {
		count = (from - kd_curpos + ONE_SPACE) / ONE_SPACE;
		(*kd_dmvdown)(from, to, count);
	}

	count = insbytes / ONE_SPACE;
	(*kd_dclear)(kd_curpos, count, kd_attr);
	return;
}


/*
 * kd_isupper, kd_islower:
 *
 *	Didn't want to include ctype.h because it brings in stdio.h, and
 *	only want to see if the darn character is uppercase or lowercase.
 *
 * input	: Character 'c'
 * output	: isuuper gives TRUE if character is uppercase, islower
 *		  returns TRUE if character is lowercase
 *
 */

kd_isupper(c)

u_char	c;

{
	if (('A' <= c) && (c <= 'Z'))
		return(TRUE);
	return(FALSE);
}

kd_islower(c)

u_char	c;

{
	if (('a' <= c) && (c <= 'z'))
		return(TRUE);
	return(FALSE);
}


/*
 * kd_senddata:
 *
 *	This function sends a byte to the keyboard RDWR port, but
 *	first waits until the input/output data buffer is clear before
 *	sending the data.  Note that this byte can be either data or a
 *	keyboard command.
 *
 */

kd_senddata(ch)

unsigned char	ch;

{
	while (inb(K_STATUS) & K_IBUF_FUL)
		continue;
	outb(K_RDWR, ch);
	last_sent = ch;
	return;
}


/*
 * kd_sendcmd:
 *
 *	This function sends a command byte to the keyboard command
 *	port, but first waits until the input/output data buffer is
 *	clear before sending the data.
 *
 */

kd_sendcmd(ch)

unsigned char	ch;

{
	while (inb(K_STATUS) & K_IBUF_FUL)
		continue;
	outb(K_CMD, ch);
	return;
}


/* 
 * kd_getdata:
 * 
 *	This function returns a data byte from the keyboard RDWR port, 
 *	after waiting until the port is flagged as having something to 
 *	read. 
 */

unsigned char
kd_getdata()

{
	while ((inb(K_STATUS) & K_OBUF_FUL) == 0)
		;
	return(inb(K_RDWR));
}


/* 
 * set_kd_state:
 * 
 *	Set kd_state and update the keyboard status LEDs.
 */

set_kd_state(newstate)

int newstate;

{
	kd_state = newstate;
	kd_setleds1(state2leds(newstate));
}


/* 
 * state2leds:
 * 
 *	Return a byte containing LED settings for the keyboard, given 
 *	a state vector.
 */

u_char
state2leds(state)

int	state;

{
	u_char result = 0;

	if (state & KS_NLKED)
		result |= K_LED_NUMLK;
	if (state & KS_CLKED)
		result |= K_LED_CAPSLK;
	return(result);
}


/* 
 * kd_setleds[12]:
 * 
 *	Set the keyboard LEDs according to the given byte.  
 */

kd_setleds1(val)

u_char val;

{
	if (kd_ack != NOT_WAITING) {
		printf("kd_setleds1: unexpected state (%d)\n", kd_ack);
		return;
	}

	kd_ack = SET_LEDS;
	kd_nextled = val;
	kd_senddata(K_CMD_LEDS);
}

kd_setleds2()

{
	kd_senddata(kd_nextled);
}


#ifdef notdef
/* 
 * cnsetleds:
 * 
 *	like kd_setleds[12], but not interrupt-based.
 *	Currently disabled because cngetc ignores caps lock and num 
 *	lock anyway.
 */

cnsetleds(val)

u_char val;

{
	kd_senddata(K_CMD_LEDS);
	(void)kd_getdata();		/* XXX - assume is ACK */
	kd_senddata(val);
}
#endif /* notdef */


kdreboot()

{
	(*kd_dreset)();
	kd_sendcmd(0xFE);		/* XXX - magic # */
}



/*
 * Code specific to EGA/CGA/VGA boards.  This code relies on the fact
 * that the "slam" functions take a word count and ONE_SPACE takes up
 * 1 word.
 */

#define	SLAMBPW	2			/* bytes per word for "slam" fcns */


/*
 * kd_xga_init:
 *
 *	Initialization specific to character-based graphics adapters.
 */
void
kd_xga_init()
{
	csrpos_t	xga_getpos();
	unsigned char	screen;

	outb(CMOS_ADDR, CMOS_EB);
	screen = inb(CMOS_DATA) & CM_SCRMSK;
	switch(screen) {
	case CM_EGA_VGA:
		/*
		 * Here we'll want to query to bios on the card
		 * itself, because then we can figure out what
		 * type we have exactly.  At this point we only
		 * know that the card is NOT CGA or MONO.  For
		 * now, however, we assume backwards compatability
		 * with 0xb8000 as the starting screen offset
		 * memory location for these cards.
		 *
		 */
		
		vid_start = (u_char *)phystokv(EGA_START);
		kd_index_reg = EGA_IDX_REG;
		kd_io_reg = EGA_IO_REG;
		kd_lines = 25;
		kd_cols = 80;
		kd_bitmap_start = 0xa0000; /* XXX - magic numbers */
		{		/* XXX - is there a cleaner way to do this? */
		    char *addr = (char *)phystokv(kd_bitmap_start);
		    int i;
		    for (i = 0; i < 200; i++)
			addr[i] = 0x00;
		}
		break;
	case CM_CGA_40:
		vid_start = (u_char *)phystokv(CGA_START);
		kd_index_reg = CGA_IDX_REG;
		kd_io_reg = CGA_IO_REG;
		kd_lines = 25;
		kd_cols = 40;
		break;
	case CM_CGA_80:
		vid_start = (u_char *)phystokv(CGA_START);
		kd_index_reg = CGA_IDX_REG;
		kd_io_reg = CGA_IO_REG;
		kd_lines = 25;
		kd_cols = 80;
		break;
	case CM_MONO_80:
		vid_start = (u_char *)phystokv(MONO_START);
		kd_index_reg = MONO_IDX_REG;
		kd_io_reg = MONO_IO_REG;
		kd_lines = 25;
		kd_cols = 80;
		break;
	default:
		printf("kd: unknown screen type, defaulting to EGA\n");
	}

	kd_setpos(xga_getpos());
}


/*
 * xga_getpos:
 *
 *	This function returns the current hardware cursor position on the
 *	screen, scaled for compatibility with kd_curpos.
 *
 * input	: None
 * output	: returns the value of cursor position on screen
 *
 */

csrpos_t
xga_getpos()

{
	unsigned char	low;
	unsigned char	high;
	short pos;

	outb(kd_index_reg, C_HIGH);
	high = inb(kd_io_reg);
	outb(kd_index_reg, C_LOW);
	low = inb(kd_io_reg);
	pos = (0xff&low) + ((unsigned short)high<<8);

	return(ONE_SPACE * (csrpos_t)pos);
}


/*
 * charput:
 *
 *	Put attributed character for EGA/CGA/etc.
 */

static void
charput(pos, ch, chattr)

csrpos_t pos;				/* where to put it */
char	ch;				/* the character */
char	chattr;				/* its attribute */

{
	*(vid_start + pos) = ch;
	*(vid_start + pos + 1) = chattr;
}


/*
 * charsetcursor:
 *
 *	Set hardware cursor position for EGA/CGA/etc.
 */
static void
charsetcursor(newpos)

csrpos_t newpos;

{
	short curpos;		/* position, not scaled for attribute byte */

    	curpos = newpos / ONE_SPACE;
    	outb(kd_index_reg, C_HIGH);
    	outb(kd_io_reg, (u_char)(curpos>>8));
    	outb(kd_index_reg, C_LOW);
    	outb(kd_io_reg, (u_char)(curpos&0xff));

	kd_curpos = newpos;
}


/*
 * charmvup:
 *
 *	Block move up for EGA/CGA/etc.
 */
static void
charmvup(from, to, count)

csrpos_t from, to;
int count;

{
	kd_slmscu(vid_start+from, vid_start+to, count);
}


/*
 * charmvdown:
 *
 *	Block move down for EGA/CGA/etc.
 */
static void
charmvdown(from, to, count)

csrpos_t from, to;
int count;

{
	kd_slmscd(vid_start+from, vid_start+to, count);
}


/*
 * charclear:
 *
 *	Fast clear for CGA/EGA/etc.
 */
static void
charclear(to, count, chattr)

csrpos_t to;
int	count;
char	chattr;

{
	kd_slmwd(vid_start+to, count, ((unsigned short)chattr<<8)+K_SPACE);
}


/* 
 * kd_noopreset:
 * 
 * 	No-op reset routine for kd_dreset.
 */

static void
kd_noopreset()
{
}



/*
 * Generic routines for bitmap devices (i.e., assume no hardware
 * assist).  Assumes a simple byte ordering (i.e., a byte at a lower
 * address is to the left of the byte at the next higher address).
 * For the 82786, this works anyway if the characters are 2 bytes
 * wide.  (more bubble gum and paper clips.)
 *
 * See the comments above about SLAMBPW.
 */

void	bmpch2bit(), bmppaintcsr();
u_char	*bit2fbptr();


/*
 * bmpput: Copy a character from the font to the frame buffer.
 */

void
bmpput(pos, ch, chattr)

csrpos_t pos;
char	ch, chattr;

{
	short xbit, ybit;		/* u/l corner of char pos */
	register u_char *to, *from;
	register short i, j;
	u_char mask = (chattr == KA_REVERSE ? 0xff : 0);

	if ((u_char)ch >= chars_in_font)
		ch = K_QUES;

	bmpch2bit(pos, &xbit, &ybit);
	to = bit2fbptr(xbit, ybit);
	from = font_start + ch * char_byte_width;
	for (i = 0; i < char_height; ++i) {
		for (j = 0; j < char_byte_width; ++j)
			*(to+j) = *(from+j) ^ mask;
		to += fb_byte_width;
		from += font_byte_width;
	}
}


/*
 * bmpcp1char: copy 1 char from one place in the frame buffer to
 * another.
 */

void
bmpcp1char(from, to)

csrpos_t from, to;

{
	short from_xbit, from_ybit;
	short to_xbit, to_ybit;
	register u_char *tp, *fp;
	register short i, j;

	bmpch2bit(from, &from_xbit, &from_ybit);
	bmpch2bit(to, &to_xbit, &to_ybit);

	tp = bit2fbptr(to_xbit, to_ybit);
	fp = bit2fbptr(from_xbit, from_ybit);

	for (i = 0; i < char_height; ++i) {
		for (j = 0; j < char_byte_width; ++j)
			*(tp+j) = *(fp+j);
		tp += fb_byte_width;
		fp += fb_byte_width;
	}
}


/*
 * bmpvmup: Copy a block of character positions upwards.
 */

void
bmpmvup(from, to, count)

csrpos_t from, to;
int	count;

{
	short from_xbit, from_ybit;
	short to_xbit, to_ybit;
	short i;

	bmpch2bit(from, &from_xbit, &from_ybit);
	bmpch2bit(to, &to_xbit, &to_ybit);

	if (from_xbit == xstart && to_xbit == xstart && count%kd_cols == 0) {
		/* fast case - entire lines */
		from_xbit = to_xbit = 0;
		bmppaintcsr(kd_curpos, char_black); /* don't copy cursor */
		count /= kd_cols;	/* num lines */
		count *= fb_byte_width * (char_height+cursor_height);
		kd_slmscu(bit2fbptr(from_xbit, from_ybit),
			  bit2fbptr(to_xbit, to_ybit), 
			  count/SLAMBPW);
		bmppaintcsr(kd_curpos, char_white);
	} else {
		/* slow case - everything else */
		for (i=0; i < count; ++i) {
			bmpcp1char(from, to);
			from += ONE_SPACE;
			to += ONE_SPACE;
		}
	}
}


/*
 * bmpmvdown: copy a block of characters down.
 */

void
bmpmvdown(from, to, count)

csrpos_t from, to;
int	count;

{
	short from_xbit, from_ybit;
	short to_xbit, to_ybit;
	short i;

	bmpch2bit(from, &from_xbit, &from_ybit);
	bmpch2bit(to, &to_xbit, &to_ybit);

	if (from_xbit == xstart + (kd_cols - 1) * char_width
	    && to_xbit == xstart + (kd_cols - 1) * char_width
	    && count%kd_cols == 0) {
		/* fast case - entire lines*/
		from_xbit = to_xbit = 8 * (fb_byte_width - 1);
					/* last byte on line */
		bmppaintcsr(kd_curpos, char_black); /* don't copy cursor */
		count /= kd_cols;	/* num lines */
		count *= fb_byte_width * (char_height+cursor_height);
		kd_slmscd(bit2fbptr(from_xbit, from_ybit),
			  bit2fbptr(to_xbit, to_ybit),
			  count/SLAMBPW);
		bmppaintcsr(kd_curpos, char_white);
	} else {
		/* slow case - everything else */
		for (i=0; i < count; ++i) {
			bmpcp1char(from, to);
			from -= ONE_SPACE;
			to -= ONE_SPACE;
		}
	}
}


/*
 * bmpclear: clear one or more character positions.
 */

void
bmpclear(to, count, chattr)

csrpos_t to;				/* 1st char */
int	count;				/* num chars */
char	chattr;				/* reverse or normal */

{
	register short i;
	u_short clearval;
	u_short clearbyte = (chattr == KA_REVERSE ? char_white : char_black);

	clearval = (u_short)clearbyte<<8 + clearbyte;
	if (to == 0 && count >= kd_lines * kd_cols) {
		/* fast case - entire page */
		kd_slmwd(vid_start, (fb_byte_width * fb_height)/SLAMBPW,
			 clearval);
	} else 
		/* slow case */
		for (i = 0; i < count; ++i) {
			bmpput(to, K_SPACE, chattr);
			to += ONE_SPACE;
		}
}


/*
 * bmpsetcursor: update the display and set the logical cursor.
 */

void
bmpsetcursor(pos)

csrpos_t pos;

{
	/* erase old cursor & paint new one */
	bmppaintcsr(kd_curpos, char_black);
	bmppaintcsr(pos, char_white);
	kd_curpos = pos;
}


/*
 * bmppaintcsr: paint cursor bits.
 */

void
bmppaintcsr(pos, val)

csrpos_t pos;
u_char	val;

{
	short xbit, ybit;
	register u_char *cp;
	register short line, byte;

	bmpch2bit(pos, &xbit, &ybit);
	ybit += char_height;		/* position at bottom of line */
	cp = bit2fbptr(xbit, ybit);
	for (line = 0; line < cursor_height; ++line) {
		for (byte = 0; byte < char_byte_width; ++byte)
			*(cp+byte) = val;
		cp += fb_byte_width;
	}
}


/*
 * bmpch2bit: convert character position to x and y bit addresses.
 * (0, 0) is the upper left corner.
 */

void
bmpch2bit(pos, xb, yb)

csrpos_t pos;
short	*xb, *yb;			/* x, y bit positions, u/l corner */

{
	register short xch, ych;

	xch = (pos / ONE_SPACE) % kd_cols;
	ych = pos / (ONE_SPACE * kd_cols);
	*xb = xstart + xch * char_width;
	*yb = ystart + ych * (char_height + cursor_height);
}


/*
 * bit2fbptr: return a pointer into the frame buffer corresponding to
 * the bit address (x, y).
 * Assumes that xb and yb don't point to the middle of a
 * byte.
 */

u_char *
bit2fbptr(xb, yb)

short	xb, yb;

{
	return(vid_start + yb * fb_byte_width + xb/8);
}
