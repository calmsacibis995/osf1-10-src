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
static char *rcsid = "@(#)$RCSfile: dti_kbd.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/16 07:58:25 $";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
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
 ************************************************************************/

/*
 * dti_kbd.c
 *
 * Modification history
 *
 * 19-Feb-92 - R. Craig Peterson
 *
 *	- DEFINE'D some hardcoded LK400 values
 *	- Changed autorepeat code to generate metronomes instead of
 *	  repeating the character.
 *	- Added some comments about lk400 series emulation
 *
 * 28-Oct-91 - R. Craig Peterson
 *
 *	- Allows application to read device input directly
 *
 * 09-Oct-91 - R. Craig Peterson
 *
 *	- Fixes non-reuse of addresses.
 *	- Fixes application transmit ioctl.
 *
 * 30-Sep-91 - R. Craig Peterson
 *
 *	- Fixes timeout table overflow panic.
 *	- Fixes CTRL-ALT-DEL resetting machine
 *	- Fixes application interaction for feedback in re DTi signals, etc.
 *	- Removes some non-critical panics
 *	- Stops autorepeating modifier keycodes (viz. shift/ctrl)
 *
 * 18-Sep-91 - R. Craig Peterson
 *
 *	- Adds real keyclicks, bell, caps lock led, hold led, session
 *	  manager control of keyclick and bell volume, autorepeat in
 *	  single user mode.
 *	- Should always recognize mouse and keyboard on boot, should
 *	  re-use old addresses.
 *	- Stops server's mouse code from complaining about having to
 *	  assume a 3-button mouse (fakes it out)
 *	- Does not "randomly inject" characters in single-user mode
 *
 * 27-Aug-91 - R. Craig Peterson
 *
 *	Fix timeout panic on hot-plug, and occasional timeout panic with
 *	normal operation.  Some general cleanups.
 *
 * 13-Aug-91 - R. Craig Peterson
 *
 *	Some general cleanups.
 *
 * 06-Aug-91 - R. Craig Peterson
 *
 *	Initial version of code.  Not fully functional.
 *
 */

/*
 * Code to handle DTi keyboard management.
 *
 * Currently only handles the most generic cases.
 */

/* Note: lk400/lk200 series keyboard emulation is not perfect, nor fully
   implemented.  The autorepeat code does not behave exactly like the
   "real" keyboards.


   The emulator is built into the keyboard code and causes any
   keyboard on the bus which behaves like an lk501 to provide lk201
   events to the host and to act upon lk201 commands from the host.
   This emulator does not look at the keycodes but provides the
   following: up/down events; translation of LED, bell, and keyclick
   commands; and autorepeating and keyclick.  It should be noted that
   the emulation code does not provide full lk201 emulation, but only
   the portion that is currently used by the ws-driver/X-Server.

   The Serial.bus code does not provide full emulation of the LK201 or
   LK401 keyboards.  Namely:

	- LED commands are properly honored.

	- Most keyboard bell commands are properly honored.

	- Most keyboard click commands are properly honored.

	- Keyboard divisional commands for all but the alphabetic
	commands (division 1 - main keyboard area) are ignored.

	- Down/up events are properly generated when in down/up mode.
	Autorepeat down/up modes are not supported.

	- When in down only mode autorepeat is by default supported
	with no metronome support - key events are re-generated for
	autorepeats.

	- The code reports all up events in all modes.

	- The keyboard emulation code identifies itself (& provides
	status) with the following four-byte sequence: 0x01 0x00 0x00
	0x00.

   Various other special features, which have not been used by the
   server, are not supported.

   Specifically the following commands are supported:

	0x11 turn off LED(s)
	0x13 light LED(s)
	0x1b enable click, set volume
	0x23 enable bell, set volume
	0x89 inhibit kbd transmission
	0x8b resume kbd transmission
	0x99 disable keyclick
	0x9f sound keyclick
	0xa1 disable bell
	0xa7 sound bell
	0xab request keyboard ID (response with 0x01 0x00)
	0xd3 reinstate defaults

   The following are not supported:

	0xb9 disable CNTL keyclick
	0xbb enable CNTL keyclick
	0xc1 temporary autorepeat inhibit
	0xcb jump to test mode
	0xd9 change all autorepeat to down only
	0xe1 disable autorepeat across keyboard
	0xe3 enable autorepeat across keyboard
	0xe9 change lk400/lk200 modes (off)
	0xeb change lk400/lk200 modes (on)
	0xfd reinitiate keyboard

   If an unsupported command is received it is silently ignored.

   Keyboard bells, clicks, and LED changes are propagated to all
   keyboards attached to the bus.
 */

#include <io/dec/ws/vsxxx.h>
#include <io/dec/dti/dti_hdr.h>

#define DTI_NO_KEYS	255
#define DTI_KB_DOWN	0
#define DTI_KB_UP	1
#define DTI_KB_EMPTY	10
#define DTI_KB_OUTERR	11
#define DTI_KB_INERR	12
#define DTI_MAX_LK501_PARAMS	5 /* Maximum # of parameters for a
				     single LK501 command */

#define DTI_LK400_ALL_UPS	0xb3 /* LK401 ALL UPS code */
#define DTI_LK400_METRONOME	0xb4 /* LK401 Metronome code */
#define DTI_LK400_INPUT_ERR	0xb6 /* LK401 Input error code */
#define DTI_LK400_KBD_LCK_ACK	0xb7 /* LK401 Keyboard locked acknowledgement */

extern (*vs_gdkint)();

extern u_char	*bp;		/* DEBUG */

#ifdef DEBUG
extern int dti_debug_level;
#endif
extern int dti_initialized;

extern struct dti_buf dti_free;	/* Free list of dti buffers */
extern struct dti_buf dti_down;	/* DTi buffers ready to transmit */
extern struct dti_buf dti_done;	/* Buffers xmitted, waiting for ack */
extern struct dti_buf *dti_input; /* Current input buffer */
extern struct dti_buf *dti_output; /* Current output buffer */
extern struct dti_stats dti_stats; /* statistics */
extern struct dti_devices dti_devices[]; /* Info on each device */
extern struct dti_msg dti_mfree; /* Message free list */
extern struct dti_dev_managers	dti_dev_managers[];
extern struct dti_keymaps dti_keymaps[];

extern int dti_xmt(), dti_get_caps(), dti_check_id_rply_to();
extern int	*dti_feedback, dti_awaiting_feedback, dti_feedback_mask;
extern int	dti_feedback_q_depth;

static int	keyclick = 1, keyclick_vol = 3;
static int	bell = 1, bell_vol = 3;
static int	kbd_input_ok = 1, keyboard_mode = 0;
static int	autorpt_char = 0;
int led_state = 0;

struct dti_kb_local
{
    u_char	*keymap;
    u_char	state[DTI_NO_KEYS];
};

/* Do a "song and dance" on the keyboard.  Flashes the LEDs and
   rings the bell.  Shows that the keyboard has been recognized
   by the system. */

dti_kbd_sd(arg)
{
    struct dti_msg	*mp;
    u_char	addr;
    char	state;
    int		x;

    addr = arg & 0xff;
    state = arg >> 8;

    x = splhigh();
    DTI_GETBUF(dti_mfree, mp);
    splx(x);

    if (!mp)
    {
	dti_stats.no_buffs++;
	return;
    }

    mp->msg_dest_addr = addr;
    mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 2);
    mp->len = DTI_MESSAGE_OVHD + 2;

    switch(state)
    {
    case 0:
	mp->msg_op_code = DTI_AP_KB_BELL;
	mp->msg_op_data[0] = 5;		/* Bell volume */
	break;
    case 1:
	mp->msg_op_code = DTI_AP_KB_LED;
	mp->msg_op_data[0] = 1<<2 | 1<<0;
	break;
    case 2:
	mp->msg_op_code = DTI_AP_KB_LED;
	mp->msg_op_data[0] = 1<<3 | 1<<1;
	break;
    case 3:
	mp->msg_op_code = DTI_AP_KB_LED;
	mp->msg_op_data[0] = 1<<2 | 1<<0;
	break;
    case 4:
	mp->msg_op_code = DTI_AP_KB_LED;
	mp->msg_op_data[0] = 1<<3 | 1<<1;
	break;
    case 5:
	mp->msg_op_code = DTI_AP_KB_LED;
	mp->msg_op_data[0] = 0;
	break;
    }

    if (state != 5)
	timeout(dti_kbd_sd, ((state + 1) << 8) | addr, hz / 2);

    dti_xmt(mp);
}

int
(*(dti_kbd_mgr(pri, dd)))()
struct dti_devices	*dd;
{
    char *p;
    struct dti_msg	*op;
    struct dti_keymaps	*kmp;	/* Key Map Pointer */
    extern int	dti_dflt_kbd_mgr();

    if (pri < 2)
	return(NULL);

    /* We're looking for something of the form: prot(locator) */

    if (!dti_check_caps(dd->caps, "prot", "keyb"))
	return(NULL);

#ifdef DEBUG
    if (dti_debug_level > 0)
    {
	if (dti_check_caps(dd->caps, "type", "keyb"))
	{
	    mprintf("Found keyboard\n");
	    cprintf("Found keyboard\n");
	}
	else
	{
	    mprintf("keyboard type/proto don't match\n");
	    cprintf("keyboard type/proto don't match\n");
	}
    }
#endif

    /* Make sure we have a table of key states, etc. */

    if (dd->mgr)
	bzero(dd->mgr, sizeof(struct dti_kb_local));
    else
    {
/*	KM_ALLOC(dd->mgr, u_char *, sizeof(struct dti_kb_local), 
		 KM_DEVBUF, KM_DTI_FLAGS);
*/
	dd->mgr = (u_char *) kalloc (sizeof(struct dti_kb_local));
	if (!dd->mgr)
	{
	    dti_panic("No room for device state table\n");
	    return(NULL);
	}
	else
	    bzero (dd->mgr, sizeof(struct dti_kb_local));
    }

    /* Certain key values from the keyboard are actually status reports */

    ((struct dti_kb_local *)dd->mgr)->state[0] = DTI_KB_EMPTY;
    ((struct dti_kb_local *)dd->mgr)->state[1] = DTI_KB_OUTERR;
    ((struct dti_kb_local *)dd->mgr)->state[2] = DTI_KB_INERR;

    /* Let's see if we can find the keymap. */

    for (kmp = dti_keymaps; *kmp->keymap_name; kmp++)
    {
	if (dti_check_caps(dd->caps, "keymap", kmp->keymap_name) ||
	    !strcmp(kmp->keymap_name, "DEFAULT"))
	{
	    if (!strcmp(kmp->keymap_name, "DEFAULT"))
	    {
		cprintf("dti: (0x%x) Couldn't find keymap, using %s keymap (lk501); capabilities = %s\n",
			dd->addr, kmp->keymap_name, dd->caps);
		mprintf("dti: (0x%x) Couldn't find keymap, using %s keymap (lk501); capabilities = %s\n",
			dd->addr, kmp->keymap_name, dd->caps);
	    }
#ifdef DEBUG
	    mprintf("0x%x using keymap %s\n", dd->addr, kmp->keymap_name);
#endif
	    ((struct dti_kb_local *)dd->mgr)->keymap = kmp->keymap;
	    break;
	}
    }

    /* Do a song and dance! */

    dti_kbd_sd(dd->addr);

    /* Send power-up "self-test" codes to host */

    if (vs_gdkint)
    {
	if (dti_awaiting_feedback &&
	    (dti_feedback_mask & DTI_TRACE_KBD_SVR))
	{
	    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | 0x01);
	    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | 0x00);
	    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | 0x00);
	    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | 0x00);
	}

	(*vs_gdkint)(0x01);	/* KBID */
	(*vs_gdkint)(0x00);	/* KBID */
	(*vs_gdkint)(0x00);	/* Success */
	(*vs_gdkint)(0x00);	/* No keys down */
    }    

    return(dti_dflt_kbd_mgr);
}

dti_kbd_autorpt(dummy)
{
    if (autorpt_char && !MODIFIER(autorpt_char))
    {
	if (vs_gdkint)
	{
	    if (dti_awaiting_feedback &&
		(dti_feedback_mask & DTI_TRACE_KBD_SVR))
		DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | DTI_LK400_METRONOME);
	    (*vs_gdkint)(DTI_LK400_METRONOME);
	}
	dti_keyclick(autorpt_char, 0);
	timeout(dti_kbd_autorpt, 0, hz / 32);
    }
}

dti_dflt_kbd_mgr(type, mp)
register struct dti_msg *mp;
{
    int		len, i, x;
    register u_char	*cp, *p;
    register u_char	*sp;	/* State table pointer */
    struct dti_kb_local	*kbp;	/* keyboard info pointer */
    extern u_char	dti_to_lk201[];

    switch(type)
    {
    case DTI_T_MESG:		/* A normal message */
	if (dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)].stat & DTI_IS_IDREQ)
	{
#ifdef DEBUG
	    if (dti_debug_level > 0)
		mprintf("dti_dflt_kbd_mgr: resetting DTI_IS_IDREQ\n");
#endif
	    dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)].stat &= ~DTI_IS_IDREQ;
	    untimeout(dti_check_id_rply_to, mp->msg_source_addr);
	}
	
	break;

    case DTI_T_ERR:		/* A transmission error occured */
	/* We have in stat one of:
	   DTI_I_ARBL DTI_I_NACK DTI_I_TIMO
	   */

	if (mp->stat == DTI_I_NACK)
	{
	    x = splhigh();
	    if (dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat & DTI_IS_IDREQ == 0)
	    {
		dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat |= DTI_IS_IDREQ;
		splx(x);
		/* We've haven't sent an ID Request to him yet */
		dti_send_id_request(mp->msg_dest_addr);

		timeout(dti_check_id_rply_to, mp->msg_dest_addr, DTI_VERIFY_ALIVE_INTERVAL);
	    } else
		splx(x);
	    /* We'll just let the timeout take care of this guy.  We can't be sure that
	       the NAK really matches the message we got back.  The I2C firmware doesn't
	       guarantee that we can... */
	}
#if 0				/* Things aren't stable enough to be able
				   to be absolutely sure we need to re-transmit.
				   Perhaps at some future date. */
	else
	{
#ifdef DEBUG
	    if (dti_debug_level > 1)
		mprintf("dti_dflt_kbd_mgr: retransmitting due to %d\n",
			type);
#endif
	    dti_xmt(mp);
	}
#endif
	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);
	return;

    case DTI_T_UNKERR:		/* Somebody err'ed... Maybe us? */
#ifdef DEBUG
	if (dti_debug_level > 2)
	    cprintf(" ku0x%x ", mp->msg_dest_addr);
#endif
	dti_update_leds(0);
	mp->msg_source_addr = mp->msg_dest_addr;
	mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 1);
	mp->msg_op_code = DTI_AP_KB_POLL;
	mp->len = DTI_MESSAGE_OVHD + 1;
	dti_xmt(mp);
	return;
    }

    /* We don't expect many op-code type messages */

    if (mp->type_len & 0x80)
    {
	if (mp->msg_op_code == DTI_OP_IDRPLY)
	{
	    /* We already reset the timeout up above. */
#ifdef DEBUG
	    if (dti_debug_level)
		mprintf("kbd still healty 0x%x\n", mp->msg_source_addr);
#endif
	    return;
	}

	if (mp->msg_op_code == DTI_OP_SIG)
	{
	    /* Values are 1=reset, 2=halt, 3=attn */
	    if (dti_awaiting_feedback &&
		(dti_feedback_mask & DTI_TRACE_SIGNAL))
	    {
		DTI_ADD_FEEDBACK(DTI_TRACE_SIGNAL | mp->msg_op_data[0]);
		wakeup(dti_feedback);
	    }
	    return;
	}
	

#ifdef DEBUG
	mprintf("unknown keyboard op code 0x%x\n",
		mp->msg_op_code);
#endif
	dti_stats.unexpect_op++;
#ifdef DEBUG
	if (dti_debug_level > 0)
	    dti_print_mp(mp, "failing message:");
#endif
	return;
    }

    if (!vs_gdkint ||		/* Is ws-driver hook initialized? */
	!kbd_input_ok)		/* Is keyboard input ok? */
	return;

    cp = mp->msg_data;
    len = mp->type_len & DTI_LEN_MASK;
    kbp = (struct dti_kb_local *)mp->dd->mgr;

    sp = kbp->state;

    if (dti_awaiting_feedback &&
	(dti_feedback_mask & DTI_TRACE_KBD))
    {
	DTI_ADD_FEEDBACK(DTI_TRACE_KBD | ((int)mp->msg_source_addr << 16) | len);
	for ( ; len; cp++, len--)
	    DTI_ADD_FEEDBACK(DTI_TRACE_KBD | *cp);
	cp = mp->msg_data;
	len = mp->type_len & DTI_LEN_MASK;
    }
	
    for ( ; len; cp++, len--)
    {
#ifdef DEBUG
	if (dti_debug_level > 5)
	    cprintf("*cp = 0x%x, sp[*cp] = 0x%x ", *cp, sp[*cp]);
#endif
	switch (sp[*cp])
	{
	case DTI_KB_DOWN:	/* Key down */
	    sp[*cp] = DTI_KB_UP;

	    if (dti_awaiting_feedback &&
		(dti_feedback_mask & DTI_TRACE_KBD_SVR))
		DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | (1 << 8) | kbp->keymap[*cp]);

	    (*vs_gdkint)(kbp->keymap[*cp]);
	    dti_stats.key_events++;

	    if (keyboard_mode < 2 && !MODIFIER(*cp))
	    {
		autorpt_char = kbp->keymap[*cp];
		untimeout(dti_kbd_autorpt, 0);
		timeout(dti_kbd_autorpt, 0, hz / 2);
	    }

	    dti_keyclick(*cp, mp->msg_source_addr);
	    continue;

	case DTI_KB_UP:		/* Key already down */
	case DTI_KB_UP + 1:	/* Already down */
	    sp[*cp] = DTI_KB_UP;
	    continue;

        case DTI_KB_EMPTY:	/* Empty state */
	    if (dti_awaiting_feedback &&
		(dti_feedback_mask & DTI_TRACE_KBD_SVR))
		DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | DTI_LK400_ALL_UPS);

	    (*vs_gdkint)(DTI_LK400_ALL_UPS); /* All ups */

	    if (keyboard_mode < 2) /* AutoRepeatDown */
	    {
		untimeout(dti_kbd_autorpt, 0);
		autorpt_char = 0;
	    }

	    dti_stats.key_events++;
	    bzero(sp, DTI_NO_KEYS);
	    sp[0] = DTI_KB_EMPTY;
	    sp[1] = DTI_KB_OUTERR;
	    sp[2] = DTI_KB_INERR;
	    return;

	case DTI_KB_OUTERR:	/* Output Error */
	    mprintf("kbd output error\n");
	    cprintf("kbd output error\n");
	    continue;

	case DTI_KB_INERR:	/* Input Error */
	    mprintf("kbd input error\n");
	    cprintf("kbd input error\n");
	    continue;
	}
    }

    /* Check to see which keys wern't in this message that
       have been in previous messages.  We can't trust the
       last message received because we may have missed one
       in the interveaning time & there's no way to tell...*/

    for (p = sp, i = 0;
	 i < DTI_NO_KEYS;
	 p++, i++)
    {
	if (!*p)
	    continue;

	if (*p == DTI_KB_UP)
	    (*p)++;
	else
	    if (*p == DTI_KB_UP + 1) /* Has been released */
	    {
		*p = DTI_KB_DOWN;
		if (keyboard_mode >= 2 || MODIFIER(*p))
		{
		    if (dti_awaiting_feedback &&
			(dti_feedback_mask & DTI_TRACE_KBD_SVR))
			DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | (2 << 8) | kbp->keymap[i]);

		    (*vs_gdkint)(kbp->keymap[i]);
		}
		
		dti_stats.key_events++;
		if (keyboard_mode < 2)	/* AutoRepeatDown */
		{
		    untimeout(dti_kbd_autorpt, 0);
		    autorpt_char = 0;
		}
	    }
    }
}

dti_keyclick(c, addr)
u_char c;
{
    struct dti_msg *mp;
    int	x;
    
    if (keyclick && !MODIFIER(c))
    {
	if (dti_awaiting_feedback &&
	    (dti_feedback_mask & DTI_INTERCEPT_CLICK))
	{
	    DTI_ADD_FEEDBACK(DTI_INTERCEPT_CLICK | keyclick_vol);
	    wakeup(dti_feedback);
	    return;
	}

	x = splhigh();
	DTI_GETBUF(dti_mfree, mp);
	splx(x);

	if (!mp)
	{
	    dti_stats.no_buffs++;
	    return;
	}

	if (addr)
	    mp->msg_dest_addr = addr;
	mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 2);
	mp->len = DTI_MESSAGE_OVHD + 2;
	mp->msg_op_code = DTI_AP_KB_CLICK;
	mp->msg_op_data[0] = keyclick_vol; /* click volume */

	if (addr)
	    dti_xmt(mp);
	else
	{
	    dti_xmit_match(mp, dti_dflt_kbd_mgr);
	    x = splhigh();
	    DTI_RELBUF(dti_mfree, mp);
	    splx(x);
	}
    }
}

	    
dti_kbd_init()
{
/*	OSF cannot deal with this when called early in the 
*	boot process (before pmap_bootstrap is called) because
*	dti_init makes calls to kalloc.

    dti_init();
*/
    dti_kbd_putc(-1);
    return(1);
}

dti_kbd_putc(c)
int c;
{
    static int	awaiting_param = 0, parameters[DTI_MAX_LK501_PARAMS];
    static int	param_overflow = 0;
    int		x, cmd, division, mode, transmit = 0;
    struct dti_msg	*mp, *op;

    if (c == -1)
    {				/* Reset everything */
	awaiting_param = 0;
	param_overflow = 0;
	led_state = 0;
	bell_vol = 0;
	return;
    }
	
    if (!dti_initialized)
    {
#ifdef DEBUG
	cprintf("DTi not inited ");
#endif
	return;
    }

    /* Store away the current command/parameter */
    if (awaiting_param)
	parameters[awaiting_param++] = c & 0x7f;
    else
	parameters[awaiting_param++] = c & 0xff;

    /* Make sure we haven't overflowed */
    if (awaiting_param >= DTI_MAX_LK501_PARAMS)
    {
	param_overflow++;
	awaiting_param = 1;
    }

    /* Are there parameters to follow? */
    if (!(c & 0x80))
	return;			/* Yes, there are */

    /* Did the parameter table overflow? */
    if (param_overflow)
    {
	mprintf("dti: LK501 driver - keyboard parameter overflow, cmd 0x%x\n",
		parameters[0]);
#ifdef DEBUG
	cprintf("dti: LK501 driver - keyboard parameter overflow, cmd 0x%x\n",
		parameters[0]);
#endif
	param_overflow =  0;
	goto done;
    }

    cmd = parameters[0];

    if (cmd & 1)		/* Is this a mode setting? */
    {				/* Not a mode setting */
	switch (cmd)
	{
	case 0x11:		/* turn off LED(s) */
	    x = led_state;
	    led_state &= ~parameters[1];
	    if (x != led_state)
	        dti_update_leds(0);
#ifdef DEBUG
	    if (dti_debug_level)
	    mprintf("turn off LED 0x%x, state = 0x%x\n",
		    parameters[1], led_state);
#endif
	    break;

	case 0x13:		/* light LED(s) */
	    x = led_state;
	    led_state |= parameters[1];
	    if (x != led_state)
	        dti_update_leds(0);
#ifdef DEBUG
	    if (dti_debug_level)
	    mprintf("turn on LED 0x%x, state = 0x%x\n",
		    parameters[1], led_state);
#endif
	    break;
	case 0x1b:		/* enable click, set vol */
	    keyclick = 1;
	    keyclick_vol = 7 - parameters[1]; /* 0..7 (backasswards) */
	    break;

	case 0x23:		/* enable bell, set vol */
	    bell = 1;
	    bell_vol = 7 - parameters[1]; /* 0..7 (backasswards) */
	    break;

	case 0x89:		/* inhibit kbd xmission */
	    kbd_input_ok = 0;
	    if (vs_gdkint)
	    {
		if (dti_awaiting_feedback &&
		    (dti_feedback_mask & DTI_TRACE_KBD_SVR))
		    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | DTI_LK400_KBD_LCK_ACK);

		(*vs_gdkint)(DTI_LK400_KBD_LCK_ACK); /* kbd locked ack */
	    }
	    

#ifdef DEBUG
	    cprintf("keyboard disabled\n");
#endif
	    break;

	case 0x8b:		/* resume kbd xmission */
	    kbd_input_ok = 1;
#ifdef DEBUG
	    cprintf("keyboard enabled\n");
#endif
	    break;

	case 0x99:		/* disable keyclick */
	    keyclick = 0;
	    break;

	case 0x9f:		/* sound keyclick */
	    if (!keyclick)
		break;

	    dti_keyclick(0, 0);
	    break;

	case 0xa1:		/* disable bell */
	    bell = 0;
	    break;

	case 0xa7:		/* sound bell */
	    if (!bell)
		break;

	    if (dti_awaiting_feedback &&
		(dti_feedback_mask & DTI_INTERCEPT_BELL))
	    {
		DTI_ADD_FEEDBACK(DTI_INTERCEPT_BELL | bell_vol);
		wakeup(dti_feedback);
		return;
	    }

	    x = splhigh();
	    DTI_GETBUF(dti_mfree, mp);
	    splx(x);

	    if (!mp)
	    {
		dti_stats.no_buffs++;
		goto done;
	    }

	    mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 2);
	    mp->len = DTI_MESSAGE_OVHD + 2;
	    mp->msg_op_code = DTI_AP_KB_BELL;
	    mp->msg_op_data[0] = bell_vol; /* Bell volume */
	    transmit = 1;
	    break;

	case 0xab:		/* request keyboard ID */
	    if (vs_gdkint)
	    {
		if (dti_awaiting_feedback &&
		    (dti_feedback_mask & DTI_TRACE_KBD_SVR))
		{
		    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | 0x01);
		    DTI_ADD_FEEDBACK(DTI_TRACE_KBD_SVR | 0x00);
		}

		(*vs_gdkint)(0x01); /* KBID */
		(*vs_gdkint)(0x00); /* KBID */
	    }
	    break;

	case 0xb9:		/* disable CNTL keyclick */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;

	case 0xbb:		/* enable CNTL keyclick */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;
	
	case 0xc1:		/* temporary autorepeat inhibit */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;

	case 0xcb:		/* jump to test mode */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;

	case 0xd3:		/* reinstate defaults */
	    keyclick = 1;
	    keyclick_vol = 3;
	    bell = 1;
	    bell_vol = 3;
	    kbd_input_ok = 1;
	    keyboard_mode = 0;
	    break;

	case 0xd9:		/* cha all autorepeat to down only */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;

	case 0xe1:		/* disable autorepeat across kbd */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;

	case 0xe3:		/* enable autorepeat across kbd */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;

	case 0xe9:		/* Change LK400/LK200 modes (off) */
	case 0xeb:		/* Change LK400/LK200 modes (on) */
#if 0	    /* If we do the right thing & tell the server we can't change the
	       keyboard type, the server will bitch... */

	    if (vs_gdkint)
		(*vs_gdkint)(DTI_LK400_INPUT_ERR); /* No can do */
#endif
	    break;
	    
	case 0xfd:		/* reinitiate keyboard */
#ifdef DEBUG
	    mprintf("DTi kbd: 0x%x not implemented\n", cmd);
	    cprintf("DTi kbd: 0x%x not implemented\n", cmd);
#endif
	    break;
	default:
#ifdef DEBUG
	    mprintf("DTi kbd: unimplemented (len %d): 0x%x %x %x %x %x\n",
		    awaiting_param, parameters[0], parameters[1], parameters[2],
		    parameters[3], parameters[4]);
	    cprintf("DTi kbd: unimplemented (len %d): 0x%x %x %x %x %x\n",
		    awaiting_param, parameters[0], parameters[1], parameters[2],
		    parameters[3], parameters[4]);
#endif
	    break;
	}
    }
    else			/* Is a mode setting */
    {
	division = (cmd & 0x78) >> 3;
	mode =  (cmd & 6) >> 1;

	if (mode == 0 || mode == 1 || mode == 3)
	{
	    if (division == 1)	/* Main keyboard area */
	    {
		keyboard_mode = mode;
		untimeout(dti_kbd_autorpt, 0);
		autorpt_char = 0;
	    }
	}
	else
	{
	    mprintf("DTi: unsupported keyboard mode 0x%x, cmd = 0x%x\n",
		    mode, cmd);
#ifdef DEBUG
	    cprintf("DTi: unsupported keyboard mode 0x%x, cmd = 0x%x\n",
		    mode, cmd);
#endif
	}
    }
	
    /* Find the active keyboards */
    if (transmit)
    {
	
	dti_xmit_match(mp, dti_dflt_kbd_mgr);

	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);
    }
    

 done:
    
    awaiting_param = 0;
    return;
}

dti_kbd_getc()
{
#ifdef DEBUg
    cprintf("K");
#endif
    return(1);
}

dti_update_leds(dummy)
{
    static int awaiting_timeout = 0;
    int	x;
    struct dti_msg	*mp;

    if (dti_awaiting_feedback &&
	(dti_feedback_mask & DTI_INTERCEPT_LED))
    {
	DTI_ADD_FEEDBACK(DTI_INTERCEPT_LED | led_state);
	wakeup(dti_feedback);
	return;
    }

    x = splhigh();
    DTI_GETBUF(dti_mfree, mp);
    splx(x);

    if (!mp)
    {
	dti_stats.no_buffs++;
	if (!awaiting_timeout)
	    timeout(dti_update_leds, 0, hz / 8);
	awaiting_timeout = 1;
	return;
    }
    
    awaiting_timeout = 0;

    mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 2);
    mp->len = DTI_MESSAGE_OVHD + 2;
    mp->msg_op_code = DTI_AP_KB_LED;
    mp->msg_op_data[0] = led_state;

    dti_xmit_match(mp, dti_dflt_kbd_mgr);

    x = splhigh();
    DTI_RELBUF(dti_mfree, mp);
    splx(x);
}
