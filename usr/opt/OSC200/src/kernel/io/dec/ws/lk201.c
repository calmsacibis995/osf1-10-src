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
static char	*sccsid = "@(#)$RCSfile: lk201.c,v $ $Revision: 1.2.10.5 $ (DEC) $Date: 1993/11/23 21:32:09 $";
#endif 
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
 *			Copyright (c) 1989 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************

/*
 * support for lk201 and successor keyboards.
 * written by J. Gettys, from cfb original
 */

#include  <sys/types.h>
#include  <sys/errno.h>
/* #include  <sys/smp_lock.h> */
#include  <sys/tty.h>
#include  <sys/time.h>
#include  <sys/proc.h>
#include  <sys/param.h>
#include  <sys/user.h>
#include  <sys/exec.h>
/*#include  <sys/kmalloc.h>*/
#include  <sys/file.h>
/* #include  <sys/ipc.h> */
/* #include  <sys/shm.h> */
#include  <sys/conf.h>
#include  <io/common/devio.h>
#include  <sys/time.h>
#include  <sys/kernel.h>
#include  <sys/workstation.h>
#include  <sys/inputdevice.h>
#include  <sys/wsdevice.h>
#include  <io/dec/ws/lk201.h>
#include  <io/dec/tc/slu.h>

#include  <data/lk201_data.c>

#define DEBUG_PRINT_ERRORS

/*
 * Keyboard translation and font tables
 */
#define CHAR_S	0xc7
#define CHAR_Q	0xc1

static int noclicks = 0;
static unsigned int last_putc1, last_putc2;

extern char *special_keys[];

/* divisions that do not default correctly need to be fixed. */
struct lk_divdefaults {
	unsigned char division;
	unsigned char command;
} lk_divdefaults[] = {
	{ 5,  LK_AUTODOWN },
	{ 9,  LK_AUTODOWN },
	{ 10, LK_AUTODOWN },
	{ 11, LK_AUTODOWN },
	{ 12, LK_AUTODOWN },
	{ 13, LK_AUTODOWN },
};
	
#define KBD_INIT_DEFAULTS sizeof(lk_divdefaults)/sizeof(struct lk_divdefaults)

/* XXX this has to go into softc structure... */
extern ws_keyboard_state lk201_softc[];

short lk201_kbdinitstring[] = {		/* reset any random keyboard stuff */
	LK_CL_ENABLE,			/* keyclick */
	0x84,				/* keyclick volume */
	LK_KBD_ENABLE,			/* the keyboard itself */
/*	LK_BELL_ENABLE,			/* keyboard bell */
/*	0x84,				/* bell volume */
	LK_LED_DISABLE,			/* keyboard leds */
	LK_LED_ALL };
#define KBD_INIT_LENGTH	sizeof(lk201_kbdinitstring)/sizeof(short)

/*
 * Routine to get a character from LK201.
 */
lk201_getc(data)
	u_short	data;
{
	int	c;
	ws_keyboard_state *lp = &lk201_softc[0];
	/*
         * Get a character from the keyboard,
         */

loop:
        /*
         * Check for various keyboard errors
         */

	if (data == LK_POWER_UP) {
		lk201_reset_keyboard(lp);
		return;
	}
	if (data == LK_POWER_ERROR || data == LK_INPUT_ERROR 
	 			   || data == LK_OUTPUT_ERROR) 	{
		printf("lk201: keyboard error, code = %x\n", data);
		return(0);
	}
	if (data < LK_LOWEST) return(0);
        /*
         * See if its a state change key
         */
	switch (data) {
	case LOCK:
		KB_TGL_CAPSLOCK(lp);	/* toggle */
		lk201_set_leds(lp);
		data = (*slu.kbd_getc)();
		goto loop;
	case SHIFT_RIGHT:
	case SHIFT:
		KB_TGL_SHIFT(lp);
		data = (*slu.kbd_getc)();
		goto loop;
	case CNTRL:
		KB_TGL_CNTRL(lp);
		data = (*slu.kbd_getc)();
		goto loop;
	case ALLUP:
		KB_CLR_CNTRL(lp);
		KB_CLR_SHIFT(lp);
		data = (*slu.kbd_getc)();
		goto loop;
	case REPEAT:
		c = lp->last;
		break;
	default:
                /*
                 * Test for control characters. If set, see if the character
                 * is eligible to become a control character.
                 */
		if (KB_IS_CNTRL(lp)) {
		    c = q_key[data];
		    if (c >= ' ' && c <= '~')
			c &= 0x1f;
		} 
		else if (KB_IS_CAPSLOCK(lp) || KB_IS_SHIFT(lp))
			    c = q_shift_key[data];
		       else
			    c = q_key[data];
		break;	
	}
	lp->last = c;

        /*
         * Check for special function keys
         */
/* FIXME FIXME FIXME */
/* shouldn't this be a test for 0x100???? */
/* I'm changing it for now... */
/*	if (c & 0x80) */
	if (c & 0x100)
	    return (0);
	else
	    return (c);
}

caddr_t lk201_init_closure (address)
	caddr_t address;
{
	return address;
}
/*
 *
 * Use the console line drivers putc routine
 * Should be in cfb module....
 */

lk201_putc( c )
char c;
{
    /* send the char to the keyboard using line 0 of the serial line driver */
    (*slu.kbd_putc)(c);
    last_putc2 = last_putc1;
    last_putc1 = c;
}

/*
 * init an LK201.
 */
void lk201_up_down_mode(lp)
	ws_keyboard_state *lp;
{
	register int i;

	if (KB_IS_INRESET(lp)) return;
	KB_SET_INRESET(lp);
	lk201_putc(LK_DEFAULTS);
	lk201_putc(LK_ENABLE_401);	
	for (i = 1; i < 15; i++) {
		DELAY(10000);
		lk201_putc(LK_UPDOWN | (i << 3));
	}
	KB_CLR_INRESET(lp);
	KB_SET_UPDNMODE(lp);	/* now in up/down mode */
}

/*
 * reset an LK201.
 */
void lk201_reset_keyboard(lp)
	ws_keyboard_state *lp;
{
	register int i;
	static int init_done = 0;

	/* only the *VERY* first time, and assume no interrupts enabled */
	if (!init_done) {
		(*slu.kbd_init)();	/* moved here from ws_cons_init */
		DELAY(10000);
		lk201_putc(0xab);	/* Request keyboard ID */
		i = (*slu.kbd_getc)();	/* get first byte */
		switch (i) {
		  case 1:
			lp->kp->hardware_type = KB_LK201;
			lp->kp->name = lk201_name;
			break;
		  default:
#ifdef DEBUG_PRINT_ERRORS
printf("unrecognized keyboard ID (0x%x), defaulting to LK401\n", i);
#endif /* DEBUG_PRINT_ERRORS */
		  case 2:
			lp->kp->hardware_type = KB_LK401;
			lp->kp->name = lk401_name;
			break;
		  case 3:
			lp->kp->hardware_type = KB_LK443;
			lp->kp->name = lk443_name;
			lp->kp->definition = &lk443_definition;
			lp->kp->modifiers = lk443_modifiers;
			break;
		  case 4:
			lp->kp->hardware_type = KB_LK421;
			lp->kp->name = lk421_name;
			break;
		}
		i = (*slu.kbd_getc)();	/* get second byte (ignored) */
		init_done = 1;
	}
	
	if (KB_IS_REPEATING(lp)) {
	    untimeout(lk201_autorepeat, lp);
	    KB_CLR_REPEATING(lp);
	}

	if (KB_IS_INRESET(lp))
	    return;
	KB_SET_INRESET(lp);

#ifdef __alpha
	lp->timeout = 250;
	lp->interval = 50;
#endif

	lk201_putc(LK_DEFAULTS);

	for (i = 0; i < KBD_INIT_DEFAULTS; i++) {
		lk201_putc(lk_divdefaults[i].command | 
			(lk_divdefaults[i].division << 3));
		DELAY(1000);
	}
	for (i = 0; i < KBD_INIT_LENGTH; i++)
		lk201_putc(lk201_kbdinitstring[i]);

	KB_CLR_UPDNMODE(lp);	/* now in regular mode */
	KB_CLR_INRESET(lp);
}

#define MAXSPECIAL	0xba	
#define BASEKEY		0x41
#define MINSPECIAL	0xb3
#define IsBitOn(ptr, bit) \
	(((unsigned int *) (ptr))[(bit)>>5] & (1 << ((bit) & 0x1f)))

/*
 * This routine processes keyboard characters when graphics IS on.
 * Called (indirectly) from wskint during its interrupt processing.
 */
void
lk201_keyboard_event(closure, queue, ch, p)
	caddr_t closure;
	ws_event_queue *queue;
	register int ch;
	ws_pointer *p;
{
	ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register unsigned int key, bits;
	register int idx;
	int type;

	key = ch = ch & 0xff;
	if (key > MAXSPECIAL || (key >= BASEKEY && key < MINSPECIAL))  {
		idx = key >> 5;
		key &= 0x1f;
		key = 1 << key;
		    if ((lp->keys[idx] ^= key) & key) {
			register ws_keyboard_control *kc = &lp->kp->control;
			lp->last = ch;
			if (kc->auto_repeat && IsBitOn (kc->autorepeats, ch)) {
			    if (KB_IS_REPEATING(lp)) {
				KB_CLR_REPEATING(lp);
				untimeout(lk201_autorepeat, lp);
			    }
			    timeout(lk201_autorepeat, lp, lp->timeout);
			    KB_SET_REPEATING(lp);
			}
			lp->p = p;
			lp->queue = queue;
			type = BUTTON_DOWN_TYPE;
		    }
		    else {
			type = BUTTON_UP_TYPE;
			idx = 7;
			do {	/* last better be sensible */
			    if (bits = lp->keys[idx]) {
				lp->last = (idx << 5) | (ffs(bits) - 1);
				break;
			    }
			} while (--idx >= 0);
		    }
		    done:
		    ws_enter_keyboard_event(queue, ch, p, type);
	}
	else {
		KB_CLR_REPEATING(lp);
		untimeout(lk201_autorepeat, lp);
		switch (key)	{
		    case REPEAT: 
		  	printf("metronome error\n");
			break;
		    case ALLUP: 
			idx = 7;
			type = BUTTON_UP_TYPE;
			do {
			    if (bits = lp->keys[idx])   {
				lp->keys[idx] = 0;
				key = 0;
				do {
				    if (bits & 1)   {
				        ws_enter_keyboard_event
					   (queue, ((idx << 5)| key), p, type);
				    }
				    key++;
				} while (bits >>= 1);
			    }
			} while (--idx >= 0);
			break;
		    case LK_POWER_UP:
		        lk201_up_down_mode(lp);
			break;
		    case LK_INPUT_ERROR:
		    case LK_OUTPUT_ERROR:
		    case LK_POWER_ERROR: 
			printf("\nlk201: keyboard error, code=%x last=%x/%x",
			       key, last_putc1, last_putc2);
		        return;
		}
	}
}

void lk201_autorepeat (closure)
	caddr_t closure;
{
	register ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register ws_keyboard_control *kc = &lp->kp->control;
	int s = spltty();
	if (kc->auto_repeat  && IsBitOn (kc->autorepeats, lp->last) &&
	    KB_IS_REPEATING(lp)) {
		timeout(lk201_autorepeat, lp, lp->interval);
		if(noclicks == 0)
		    lk201_putc(LK_CL_SOUND);	
		/*
		 * DIX converts a single repeated down to a up/down pair, so
		 * why do twice as much work as needed in the kernel?
		 */
		ws_enter_keyboard_event(lp->queue, lp->last, lp->p, 
			BUTTON_DOWN_TYPE);
		ws_wakeup_any_pending(lp->queue);
	}
	else KB_CLR_REPEATING(lp);
	splx(s);
}
void lk201_ring_bell (closure) 
	caddr_t closure;
{	
	register ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	lk201_putc(LK_RING_BELL);	
	return;
}

int lk201_set_keyboard_control (closure, kp, wskp)
	caddr_t closure;
	ws_keyboard *kp;
	ws_keyboard_control *wskp;
{
	register ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register int flags = wskp->flags;
	noclicks = 1;
	if (flags & WSKBKeyClickPercent) {
		kp->control.click = wskp->click;
		if (wskp->click == 0)
			lk201_putc(LK_CL_DISABLE);	
		else {
			register int volume;
			volume = (7 - ((wskp->click / 14) & 7)) | 0x80;
			lk201_putc(LK_CL_ENABLE);
			lk201_putc(volume);
		}
	}
	/* can't deal with bell pitch or duration */
	if (flags & WSKBBellPercent) {
		register int loud;
		kp->control.bell = wskp->bell;
		if (kp->control.bell != 0) {
			loud = 7 - ((kp->control.bell / 14) & 7) | 0x80;
			lk201_putc(LK_BELL_ENABLE);
			lk201_putc(loud);
		}
		else
			lk201_putc(LK_BELL_DISABLE);
	}
	if (flags & WSKBAutoRepeatMode) {
		kp->control.auto_repeat = wskp->auto_repeat;
	}
	if (flags & WSKBAutoRepeats) {
		bcopy(wskp->autorepeats, kp->control.autorepeats, 32);
	}
	if (flags & WSKBLed) {
		kp->control.leds = wskp->leds;

		/*
		 * only check for those LEDs we actually have...
		 */
		if (wskp->leds & WSKBLed_NumLock)
			KB_SET_NUMLOCK(lp);
		else
			KB_CLR_NUMLOCK(lp);
		if (wskp->leds & WSKBLed_ScrollLock)
			KB_SET_HOLD(lp);
		else
			KB_CLR_HOLD(lp);
		if (wskp->leds & WSKBLed_CapsLock)
			KB_SET_CAPSLOCK(lp);
		else
			KB_CLR_CAPSLOCK(lp);
		/* these are for LK201 and LK443 */
		if (wskp->leds & WSKBLed_Wait)
			KB_SET_WAIT(lp);
		else
			KB_CLR_WAIT(lp);
		if (wskp->leds & WSKBLed_Compose)
			KB_SET_COMPOSE(lp);
		else
			KB_CLR_COMPOSE(lp);

		lk201_set_leds(lp);
	}
	noclicks = 0;
	return 0;
}

void
lk201_set_leds(lp)
    ws_keyboard_state *lp;
{
	int new_leds = 0x80;
	
	if (KB_IS_HOLD(lp)) new_leds |= LK_LED_HOLD;
	if (KB_IS_CAPSLOCK(lp)) new_leds |= LK_LED_LOCK;

	/*
	 * LK443/444 have a NUMLOCK LED which uses LED #2.
	 * LED #2 is also used as the COMPOSE LED for LK201 *AND*
	 *  some Japanese variants (LK401{JJ,BJ} and LK421JJ.
	 *
	 * NOTE: if a keyboard wants *BOTH* COMPOSE and NUMLOCK LEDs,
	 *   we might have problems, since they both currently use LED #2
	 *  (but not on the same keyboard, of course... :-).
	 */
	if (lp->kp->hardware_type == KB_LK443) {
		if (KB_IS_NUMLOCK(lp)) new_leds |= LK_LED_NUMLOCK;
	} else {
		if (KB_IS_COMPOSE(lp)) new_leds |= LK_LED_COMPOSE;
	}
	
	/* only LK201 has the WAIT LED */
	if (lp->kp->hardware_type == KB_LK201) {
		if (KB_IS_WAIT(lp)) new_leds |= LK_LED_WAIT;
	}

	lk201_putc(LK_LED_DISABLE);
	lk201_putc(LK_LED_ALL);

	lk201_putc(LK_LED_ENABLE); 
	lk201_putc(new_leds);
}

/*
 * This routine processes keyboard characters when graphics is NOT on.
 * Called (indirectly) from wskint during its interrupt processing.
 */
void
lk201_keyboard_char(lp, data)
ws_keyboard_state *lp;
u_short data;
{
	register u_short c;
	struct tty *tp;

	tp = &slu.slu_tty[0];

	/*
	 * Check for various keyboard errors
	 */
	if (data == LK_POWER_UP) {
		lk201_reset_keyboard(lp);
		return;
	}
	if (data == LK_POWER_ERROR || data == LK_OUTPUT_ERROR ||
	    data == LK_INPUT_ERROR) {
		if (!KB_IS_INRESET(lp))
                        printf("lk201: Keyboard Error, code=%x last=%x/%x\n",
			       data, last_putc1, last_putc2);
		return;
	}
	if (data < LK_LOWEST || data == LK_MODE_CHANGE) return;
	
	/*
	 * See if its a state change key
	 */
	switch (data)  {
	case LOCK:
		KB_TGL_CAPSLOCK(lp);	/* toggle */
		lk201_set_leds(lp);
		return;
	case SHIFT_RIGHT:
	case SHIFT:
		KB_TGL_SHIFT(lp);
		return;	
	case CNTRL:
		KB_TGL_CNTRL(lp);
		return;
	case ALLUP:
		KB_CLR_SHIFT(lp);
		KB_CLR_CNTRL(lp);
		return;
	case REPEAT:
		c = lp->last;
		break;
	case HOLD:
		/*
		 * "Hold Screen" key was pressed, we treat it as 
		 *  if ^s or ^q was typed.  
		 */
		if (!KB_IS_HOLD(lp)) {
			if ((tp->t_state & TS_TTSTOP) == 0)  {
			    	c = q_key[CHAR_S];
				KB_SET_HOLD(lp);
				lk201_set_leds(lp);
			} 
			else c = q_key[CHAR_Q];
		}
		else {
			c = q_key[CHAR_Q];
			KB_CLR_HOLD(lp);
			lk201_set_leds(lp);
		}
		if (c >= ' ' && c <= '~')
			c &= 0x1f;
		else if (c >= 0xA1 && c <= 0xFE)
			c &= 0x9F;
		(*linesw[tp->t_line].l_rint)(c, tp);
		return;
		
	default:
		
		/*
		 * Test for control characters. If set, see if the 
		 * character is elligible to become a control 
		 * character.
		 */
		if (KB_IS_CNTRL(lp)) {
			c = q_key[data];
			if (c >= ' ' && c <= '~') c &= 0x1f;
		} 
		else if (KB_IS_CAPSLOCK(lp) || KB_IS_SHIFT(lp))
			c = q_shift_key[data];
		else
			c = q_key[data];
		break;	
		
	}
	
	lp->last = c;
	
	/*
	 * Check for special function keys
	 */
	if (c & 0x100) {
		register char *string;
		
		string = special_keys[c & 0x7f];
		while (*string)
			(*linesw[tp->t_line].l_rint)(*string++, tp);
	} 
	else {
		if (tp->t_iflag & ISTRIP)	/* Strip to 7 bits. */
			c &= 0177;	
		else {			/* Take the full 8-bits */
			/*
			 * If ISTRIP is not set a valid character of 377
		 	 * is read as 0377,0377 to avoid ambiguity with
		 	 * the PARMARK sequence.
		 	 */ 
			if ((c == 0377) && (tp->t_line == TERMIODISC) &&
			    (tp->t_iflag & PARMRK))
				(*linesw[tp->t_line].l_rint)(0377,tp);
			
		}
	        (*linesw[tp->t_line].l_rint)(c, tp);
	}
	if (KB_IS_HOLD(lp) && ((tp->t_state & TS_TTSTOP) == 0))  {
		KB_CLR_HOLD(lp);
		lk201_set_leds(lp);
	}
}
