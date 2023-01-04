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
static char *rcsid = "@(#)$RCSfile: pcxal.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/11/02 15:31:31 $";
#endif

/* #define KBD_DEBUG */

#define DEBUG_PRINT_ERRORS
/* #define DEBUG_PRINT_INFO */
/* #define DEBUG_PRINT_MODE */
/* #define DEBUG_PRINT_SELECTCODE */
/* #define DEBUG_PRINT_INIT */
/* #define DEBUG_PRINT_EVENT */
/* #define DEBUG_PRINT_OVERRUN */

/* #define DPRINTF jprintf */
#define DPRINTF printf
/* #define DPRINTF xxputs */

#ifdef DEBUG_PRINT_ERRORS
#define ERR_PRINTF DPRINTF
#else
#define ERR_PRINTF
#endif /* DEBUG_PRINT_ERRORS */

/*
 * support for PCXAL and successor keyboards.
 * written by J. Estabrook, 
 * based on LK201 support by J. Gettys, from cfb original
 */

/* FIXME FIXME - can we delete all these includes? */
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
/* end FIXME FIXME - can we delete all these includes? */

#include <data/ws_data.c>
#include <data/pcxal_data.c>

#define MAKE_CNTRL(x) ((x) & 0x1f)

static int noclicks = 0;
static int pcxal_keyboard_enabled = 0; /* FIXME */

void pcxal_set_leds();
void pcxal_disable_keyboard();

extern void gpc_ctl_cmd();
extern void gpc_ctl_output();

extern int gpc_set_leds();

/*
 * Routine to get a character from PCXAL keyboard.
 * Uses POLLED mode I/O to get scancode from keyboard (via pcxal_getcode).
 * Used (indirectly) by the console "getc" routine entry which is
 * located in the data/cons_sw_data.c file.
 * (roughly equivalent to sccgetc from io/dec/tc/scc.c)
 */
pcxal_getc()
{
	int data, c, prefix;
	ws_keyboard_state *lp = &pcxal_softc[0];

#ifdef DEBUG_PRINT_INFO
DPRINTF("pcxal_getc: entry\n");
#endif /* DEBUG_PRINT_INFO */
	/*
         * Get a character from the keyboard,
         */

loop:
	data = gpc_ctl_getcode();
	if (data < 0)
		return(data);
	
	/*
	 * note current prefix state, clear old prefix state.
	 */
	prefix = KB_IS_PREFIX(lp);
	KB_CLR_PREFIX(lp);
	
	/*
         * Check for various keyboard errors
         */
	if (data == PK_POR) {
		pcxal_reset_keyboard(lp);
		return(0);
	}

/* FIXME FIXME - must be able to handle other non-standard returns */
/*               like: BAT Failure, ACK, RESEND, ECHO, Keyboard ID */
/*               and the others, since they *could* occur... :-(   */

#ifndef KBD_XLATE
	if (data == F0SEEN) {
		KB_SET_PREFIX(lp);
		goto loop;
	}
#else
	if (data == 0xe0 || data == 0xe1) /* ignore other prefixes for now */
		goto loop;

	if (data & 0x80) {
		prefix = 1;
		data &= 0x7f;
	}
#endif /* !KBD_XLATE */
	
	if (!data || data > PCXAL_MAX_KEYCODE)
	{
	  ERR_PRINTF("pcxal_getc: keycode not within normal range (0x%x)\n",
		     data);
	  goto loop;
	}

        /*
         * See if it's a state change key
         */
	switch (xlate[data].char_type)
	{		
	case CAPS:
		/* if current state is PREFIX, this is an UP event
		 * for the scancode just fetched, so ignore...
		 */
		if (prefix)
			goto loop;
		KB_TGL_CAPSLOCK(lp);	/* toggle */
		pcxal_set_leds(lp);
		goto loop;

	case SCROLL:
		/* if current state is PREFIX, this is an UP event
		 * for the scancode just fetched, so ignore...
		 */
		if (prefix)
			goto loop;
		KB_TGL_HOLD(lp);	/* toggle */
		pcxal_set_leds(lp);
		if (KB_IS_HOLD(lp))
			c = MAKE_CNTRL('S');
		else
			c = MAKE_CNTRL('Q');
		break;

	case NUMLOCK:
		/* if current state is PREFIX, this is an UP event
		 * for the scancode just fetched, so ignore...
		 */
		if (prefix)
			goto loop;
		KB_TGL_NUMLOCK(lp);	/* toggle */
		pcxal_set_leds(lp);
		goto loop;

	case LSHIFT:
	case RSHIFT:
#ifndef KBD_XLATE
		KB_TGL_SHIFT(lp);     /* toggle */
#else
		if (prefix)
			KB_CLR_SHIFT(lp);
		else
			KB_SET_SHIFT(lp);
#endif /* !KBD_XLATE */
		goto loop;

	case LCTRL:
	case RCTRL:
#ifndef KBD_XLATE
		KB_TGL_CNTRL(lp);     /* toggle */
#else
		if (prefix)
			KB_CLR_CNTRL(lp);
		else
			KB_SET_CNTRL(lp);
#endif /* !KBD_XLATE */
		goto loop;

	case LALT:
	case RALT:
		/*
		 * it doesn't matter if this is an UP or DOWN event,
		 *  simply toggle the current state...
		 */
#ifndef KBD_XLATE
		KB_TGL_ALT(lp);	      /* toggle */
#else
		if (prefix)
			KB_CLR_ALT(lp);
		else
			KB_SET_ALT(lp);
#endif /* !KBD_XLATE */
		goto loop;

	case ASCII:
	case NUMPAD:
	case FUNCTION:
		/* if current state is PREFIX, then this is an UP event
		 * for the scancode just fetched, so ignore...
		 */
		if (prefix)
			goto loop;
                /*
                 * test for control characters. If set, see if the character
                 * is eligible to become a control character.
                 */
		if (KB_IS_CNTRL(lp)) 
		{
			c = xlate[data].unshifted;
			if (c >= ' ' && c <= '~')
				c &= 0x1f;
                        if (c == MAKE_CNTRL('S')) {
                                KB_SET_HOLD(lp);
                                pcxal_set_leds(lp);
                        }
			else if (c == MAKE_CNTRL('Q')) {
                                KB_CLR_HOLD(lp);
                                pcxal_set_leds(lp);
                        }
		} 
                /*
                 * test for numeric keypad. If so, check NUMLOCK and
		 * load the appropriate character.
                 */
		else if (xlate[data].char_type == NUMPAD) {			
			if (KB_IS_NUMLOCK(lp))
				c = xlate[data].shifted;
			else
				c = xlate[data].unshifted;
		}
                /*
                 * test for CAPSLOCK or SHIFT state. If so,
		 * load the appropriate character, otherwise
		 * load the unshifted version.
                 */
		else if (KB_IS_CAPSLOCK(lp) || KB_IS_SHIFT(lp))
			c = xlate[data].shifted;
		else
			c = xlate[data].unshifted;
		break;	
	}
	lp->last = c;

        /*
         * Check for special function keys
	 * In this case, simply return NULL if special, since this
	 * routine is used to fetch single chars only.
         */
	if (c & 0x100)
	    return (0);
	else
	    return (c);
}

caddr_t
pcxal_init_closure (address)
    caddr_t address;
{
	return address;
}

/*
 * Initialize a PCXAL keyboard.
 *
 * Called from wsopen (in io/dec/ws/ws_device.c) during /dev/mouse open.
 * Also called from pcxal_keyboard_event, when Power On Reset processing
 * finishes the Basic Assurance Test (BAT).
 *
 * Usage pattern indicates that this is the routine called to initialize
 * the keyboard for/during graphics mode (Xserver use).
 *
 * The assumption is that the keyboard "defaults" will be set, followed by
 * keyboard scanning enabled, and the LEDS cleared (since status is "init").
 */
void
pcxal_init_keyboard(lp)
    ws_keyboard_state *lp;
{
	register int i;
	int s, err, data;
	
	if (KB_IS_REPEATING(lp)) {
	    untimeout(pcxal_autorepeat, lp);
	    KB_CLR_REPEATING(lp);
	}

	if (KB_IS_INRESET(lp))
	    return;
	KB_SET_INRESET(lp);

	s = spltty();
	
	err = pcxal_bot_init(lp);
	if (err) 
	{
		ERR_PRINTF("pcxal_bot_init unsuccessful.\n");
		splx(s);
		return;		/* FIXME FIXME - no return value? */
	}

        /* Flush any pending input. */
        while (gpc_input() != -1)
                continue;

	/*
	 * now DISABLE the keyboard to stop it scanning...
	 */
        if (gpc_output(PK_DISABLE) != PK_ACK) {
		ERR_PRINTF("pcxal_init_keyboard: keyboard disable failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
                DPRINTF("pcxal_init_keyboard: keyboard disable passed.\n");
#endif /* DEBUG_PRINT_INIT */

        /*
	 * Switch the keyboard to scan-code set 3.
         */
        if (gpc_output(PK_SELECTCODE) != PK_ACK) {
		ERR_PRINTF("pcxal_init_keyboard: select code failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
		DPRINTF("pcxal_init_keyboard: select code ACK received.\n");
#endif /* DEBUG_PRINT_INIT */

        /* Send out a 3 for scan code 3 */
	/* then send out SET DEFAULTS */
	/* then send out SET ALL KEYS to Make/Break */
        if (gpc_output_noverify(0x03) == PK_ACK) {
                if (gpc_output(PK_DEFAULTS) != PK_ACK ||
                    gpc_output(PK_SETALL_MB) != PK_ACK)
                {
			ERR_PRINTF("pcxal_init_keyboard: setup failed.\n");
			splx(s);
                        return;
                }
#ifdef DEBUG_PRINT_INIT
                else
                        DPRINTF("pcxal_init_keyboard: setup passed.\n");
#endif /* DEBUG_PRINT_INIT */
        }
	else 
	{
		ERR_PRINTF("pcxal_init_keyboard: scancode 3 not ACKed.\n");
		splx(s);
		return;
	}

        /*
         * Change MODE reg for no conversion of keycodes
         */
        gpc_ctl_cmd(PK_CTL_RDMODE);
        data = gpc_ctl_input();
        data &= ~(PK_MODE_KCC);
        gpc_ctl_cmd(PK_CTL_WRMODE);
        gpc_ctl_output(data);

	/*
	 * now ENABLE the keyboard to set it scanning...
	 */
        if (gpc_output(PK_ENABLE) != PK_ACK) {
		ERR_PRINTF("pcxal_init_keyboard: keyboard enable failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
                DPRINTF("pcxal_init_keyboard: keyboard enable passed.\n");
#endif /* DEBUG_PRINT_INIT */

	splx(s);

	/*
	 * Init the autorepeats array with gfx mode version.
	 */
	for (i = 0; i < 8; i++)
		lp->kp->control.autorepeats[i] = gfx_mode_autorepeats[i];

	/*
	 * clear the state and the LEDs
	 * FIXME FIXME should be more efficient...
	 */
	KB_CLR_PREFIX(lp);
	KB_CLR_SHIFT(lp);
	KB_CLR_CNTRL(lp);
	KB_CLR_HOLD(lp);
	KB_CLR_NUMLOCK(lp);
	KB_CLR_CAPSLOCK(lp);
	KB_CLR_ALT(lp);
	pcxal_set_leds(lp);

	KB_CLR_INRESET(lp);
}

/*
 * Reset a PCXAL keyboard.
 *
 * Called from ws_cons_init (in io/dec/ws/ws_device.c) during boot-up
 *  console initialization.
 * Called from wsclose (in io/dec/ws/ws_device.c) during /dev/mouse close.
 * Also called from pcxal_keyboard_char and pcxal_getc, when Power On Reset
 *  (POR) processing finishes with the Basic Assurance Test (BAT), and sends
 *  the BAT completion command (0xAA).
 *
 * Usage pattern indicates that this is the routine called to initialize
 * the keyboard for/during text mode ("glass tty" use).
 *
 * The assumption is that the keyboard "defaults" will be set, followed by
 * enabling of CLICK with volume, keyboard scanning enabled, and the LEDS
 * cleared (since status is "reset").
 */
void
pcxal_reset_keyboard(lp)
    ws_keyboard_state *lp;
{
	register int i;
	int s, err, data;

	if (KB_IS_REPEATING(lp)) {
	    untimeout(pcxal_autorepeat, lp);
	    KB_CLR_REPEATING(lp);
	}

	if (KB_IS_INRESET(lp))
		return;
	KB_SET_INRESET(lp);

	s = spltty();

	err = pcxal_bot_init(lp);

	if (err) 	/* probably no keyboard... */
	{
		ERR_PRINTF("PCXAL reset_keyboard unsuccessful.\n");
		splx(s);
		return;
	}
#ifdef DEBUG_PRINT_INIT
	else
		DPRINTF("pcxal_reset_keyboard completed successfully.\n");
#endif /* DEBUG_PRINT_INIT */

        /* Flush any pending input. */
        while (gpc_input() != -1)
                continue;

	/*
	 * now DISABLE the keyboard to stop it scanning...
	 */
        if (gpc_output(PK_DISABLE) != PK_ACK) {
		ERR_PRINTF("pcxal_reset_keyboard: disable failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
                DPRINTF("pcxal_reset_keyboard: disable passed.\n");
#endif /* DEBUG_PRINT_INIT */

	/*
	 * Switch it to scan-code set 2.
	 * First, send the command, then send out a 2 for scan code 2.
         */
        if (gpc_output(PK_SELECTCODE) != PK_ACK) {
		ERR_PRINTF("pcxal_reset_keyboard: select code failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
		DPRINTF("pcxal_reset_keyboard: select code ACK received.\n");
#endif /* DEBUG_PRINT_INIT */

        if (gpc_output_noverify(0x02) != PK_ACK)
	{
		ERR_PRINTF("pcxal_reset_keyboard: scancode 2 not ACKed.\n");
		splx(s);
		return;
	}

	/*
	 * Set Typematic Rate/Delay
	 */
        if (gpc_output(PK_SETRATE) != PK_ACK) 
	{
		ERR_PRINTF("pcxal_reset_keyboard: set rate/delay failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
		DPRINTF("pcxal_reset_keyboard: set rate/delay ACK received.\n");
#endif /* DEBUG_PRINT_INIT */

        /* Send out a 0x04 for delay=250msecs and rate=20/sec */
        if (gpc_output_noverify(0x04) != PK_ACK) {
		ERR_PRINTF("pcxal_reset_keyboard: rate/delay not ACKed.\n");
		splx(s);
		return;
	}

        /*
         * Change MODE reg to set KCC for conversion of keycodes
	 *
	 * NOTE: this is required by the console firmware, although there's
	 * no documentation as to what really happens. It seems to cause the
	 * keyboard to return scacode set *1* keycodes, even though the board
	 * is set to scancode set 2 (either explicitly or by RESET).
         */
        gpc_ctl_cmd(PK_CTL_RDMODE);
        data = gpc_ctl_input();
        data |= PK_MODE_KCC;
        gpc_ctl_cmd(PK_CTL_WRMODE);
        gpc_ctl_output(data);

	/*
	 * now ENABLE the keyboard to set it scanning...
	 */
        if (gpc_output(PK_ENABLE) != PK_ACK) {
		ERR_PRINTF("pcxal_reset_keyboard: enable failed.\n");
		splx(s);
		return;
        }
#ifdef DEBUG_PRINT_INIT
        else
                DPRINTF("pcxal_reset_keyboard: enable passed.\n");
#endif /* DEBUG_PRINT_INIT */

/* FIXME FIXME - keyboard CLICK must be started here... */

	splx(s);

	/*
	 * Init the autorepeats array with tty mode version.
	 */
	for (i = 0; i < 8; i++)
		lp->kp->control.autorepeats[i] = tty_mode_autorepeats[i];

	/*
	 * Clear the state and the LEDs.
	 * FIXME FIXME should be more efficient...
	 */
	KB_CLR_PREFIX(lp);
	KB_CLR_SHIFT(lp);
	KB_CLR_CNTRL(lp);
	KB_CLR_HOLD(lp);
	KB_CLR_NUMLOCK(lp);
	KB_CLR_CAPSLOCK(lp);
	KB_CLR_ALT(lp);
	pcxal_set_leds(lp);

	KB_CLR_INRESET(lp);
}

/*
 * BOT (beginning of time) keyboard init code.
 *
 * NOTE: should be run BEFORE interrupts are enabled, and
 * it will be run only once.
 */
int
pcxal_bot_init(lp)
    ws_keyboard_state *lp;
{
	int d, s;
	static int pcxal_bot_init_done = 0;

	if (pcxal_bot_init_done) return(0);

        /* Flush any pending input. */
        while (gpc_input() != -1)
                continue;

#ifdef DEBUG_PRINT_MODE
        gpc_ctl_cmd(PK_CTL_RDMODE);
        s = gpc_ctl_input();
	DPRINTF("pcxal_bot_init: initial MODE 0x%x.\n", s);
#endif /* DEBUG_PRINT_MODE */

	/* init keyboard to PS/2 style */
	gpc_init_keyboard();

        /*
         * Test the keyboard interface in the VTI chip.
         * This seems to be the only way to get it going.
         * If the test is successful a x55 is placed in the input buffer.
         */
        gpc_ctl_cmd(PK_CTL_TEST1);
        if ((s = gpc_ctl_input()) != 0x55) {
		ERR_PRINTF("pcxal_bot_init: keyboard failed self test.\n");
		return(-1);
        }
#ifdef DEBUG_PRINT_INIT
        else
                 DPRINTF("pcxal_bot_init: keyboard passed self test.\n");
#endif /* DEBUG_PRINT_INIT */

        /*
         * Perform a keyboard interface test.  This causes the controller
         * to test the keyboard clock and data lines.  The results of the
         * test are placed in the input buffer.
         */
        gpc_ctl_cmd(PK_CTL_TEST2);
        if ((s = gpc_ctl_input()) != 0x00) {
#ifdef DEBUG_PRINT_ERRORS
                 DPRINTF("pcxal_bot_init: interface test failure.\n");
                 switch (s) {
                        case 1: DPRINTF("Keyboard clock line stuck low.\n");
                                break;
                        case 2: DPRINTF("Keyboard clock line stuck high.\n");
                                break;
                        case 3: DPRINTF("Keyboard data line stuck low.\n");
                                break;
                        case 4: DPRINTF("Keyboard data line stuck high.\n");
                                break;
                        default: DPRINTF("Unknown kbd error %d\n",s);
                                break;
                 };
#endif /* DEBUG_PRINT_ERRORS */
                 return(-1);
        }
#ifdef DEBUG_PRINT_INIT
        else
                 DPRINTF("pcxal_bot_init: passed interface test.\n");
#endif /* DEBUG_PRINT_INIT */

        /* Enable the keyboard by allowing the keyboard clock to run. */
        gpc_ctl_cmd(PK_CTL_ENABLE);

#ifdef DEBUG_PRINT_SELECTCODE
        if (gpc_output(PK_SELECTCODE) != PK_ACK) {
                 DPRINTF("pcxal_bot_init: select code failed.\n");
                 return(-1);
        }
        if (gpc_output_noverify(0x00) == PK_ACK) {
		s = gpc_input();
		DPRINTF("pcxal_bot_init: default select code %d.\n", s);
	}
	else
		DPRINTF("pcxal_bot_init: default select code no ACK.\n");
#endif /* DEBUG_PRINT_SELECTCODE */

        /*
         * Reset keyboard. If the read times out
         * then the assumption is that no keyboard is
         * plugged into the machine.
         * This defaults the keyboard to scan-code set 2.
         */
        if (gpc_output(PK_RESET) != PK_ACK) {
		ERR_PRINTF("pcxal_bot_init: reset kbd failed, no ACK.\n");
		return(-1);
        }
#ifdef DEBUG_PRINT_INIT
        else
                 DPRINTF("pcxal_bot_init: reset kbd ACK received.\n");
#endif /* DEBUG_PRINT_INIT */

        if (gpc_input() != 0xAA) {
		ERR_PRINTF("pcxal_bot_init: reset kbd failed, not 0xAA.\n");
		return(-1);
        }
#ifdef DEBUG_PRINT_INIT
        else
                 DPRINTF("pcxal_bot_init: reset kbd 0xAA received.\n");
#endif /* DEBUG_PRINT_INIT */

        /*
	 * FIXME FIXME - comment is inaccurate...
         * Disable keyboard interrupts, operate in "real" mode,
         * Enable keyboard (by clearing the disable keyboard bit),
         * No conversion of keycodes, disable mouse interrupt.
         */
        gpc_ctl_cmd(PK_CTL_RDMODE);
        s = gpc_ctl_input();
        s &= ~(/*PK_MODE_EKI|PK_MODE_EMI|*/PK_MODE_SYS|PK_MODE_DKB|PK_MODE_KCC);
#ifdef KBD_XLATE
	s |= PK_MODE_KCC;
#endif /* KBD_XLATE */
        gpc_ctl_cmd(PK_CTL_WRMODE);
        gpc_ctl_output(s);

#ifdef DEBUG_PRINT_MODE
DPRINTF("pcxal_bot_init: wrote MODE 0x%x.\n", s);
#endif /* DEBUG_PRINT_MODE */

	pcxal_bot_init_done = 1;
	return(0);
}

#define IsBitOn(ptr, bit) \
	(((unsigned int *) (ptr))[(bit)>>5] & (1 << ((bit) & 0x1f)))

/*
 * This routine processes keyboard characters when graphics IS on.
 * Called (indirectly) from wskint during its interrupt processing.
 */
void
pcxal_keyboard_event(closure, queue, ch, p)
	caddr_t closure;
	ws_event_queue *queue;
	register int ch;
	ws_pointer *p;
{
	ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register unsigned int key, bits;
	register int idx;
	int type;
	int prefix;

	key = (ch &= 0xff);

	/*
	 * check for active "prefix" (UP indicator)
	 */
	prefix = KB_IS_PREFIX(lp);
	KB_CLR_PREFIX(lp);

/* FIXME FIXME - this may need some work??? */

	switch (key) 
	{
/* FIXME FIXME - must be able to handle other non-standard returns */
/*               like: BAT Failure, ACK, RESEND, ECHO, Keyboard ID */
/*               and the others, since they *could* occur... :-(   */
		

	case F0SEEN: 
		KB_SET_PREFIX(lp);
		return;

	case 0xff:	/* FIXME FIXME - possible in scancode set 3? */
	case PK_OVR:
		KB_CLR_REPEATING(lp);
		untimeout(pcxal_autorepeat, lp);
#ifdef DEBUG_PRINT_OVERRUN
DPRINTF("pcxal_keyboard_event: Overrun character (0x%x) detected.\n", key);
#endif /* DEBUG_PRINT_OVERRUN */
		break;

	case PK_POR:
		KB_CLR_REPEATING(lp);
		untimeout(pcxal_autorepeat, lp);
		pcxal_init_keyboard(lp);
		break;

	default:
		/* first, do any necessary translations... */
		/* keycodes are required by X to be 7 < keycode < 256 */
		if (key == RAW_KEY_F1) ch = key = KEY_F1;

		/* now process the keycode */
		idx = key >> 5;
		key &= 0x1f;
		key = 1 << key;

		lp->keys[idx] ^= key;	/* update state array for yuks :-) */

		if (!prefix) {
			register ws_keyboard_control *kc = &lp->kp->control;
			lp->last = ch;
			if (kc->auto_repeat && IsBitOn(kc->autorepeats, ch))
			{
			    if (KB_IS_REPEATING(lp)) {
				KB_CLR_REPEATING(lp);
				untimeout(pcxal_autorepeat, lp);
			    }
			    timeout(pcxal_autorepeat, lp, lp->timeout);
			    KB_SET_REPEATING(lp);
			}
			lp->p = p;
			lp->queue = queue;
			type = BUTTON_DOWN_TYPE;
		}
		else {
			KB_CLR_REPEATING(lp);	/* FIXME - more needed? */
			untimeout(pcxal_autorepeat, lp);
			type = BUTTON_UP_TYPE;
		}
#ifdef DEBUG_PRINT_EVENT
DPRINTF("pcxal_keyboard_event: keycode 0x%x %s\n", ch, prefix ? "UP" : "DN");
#endif /* DEBUG_PRINT_EVENT */
		ws_enter_keyboard_event(queue, ch, p, type);
		break;
	}
}

void
pcxal_autorepeat(closure)
    caddr_t closure;
{
	register ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register ws_keyboard_control *kc = &lp->kp->control;
	int s = spltty();

	if (kc->auto_repeat &&
	    IsBitOn(kc->autorepeats, lp->last) &&
	    KB_IS_REPEATING(lp))
	{
		timeout(pcxal_autorepeat, lp, lp->interval);
#ifdef FIXME
		if (noclicks == 0)
		    (*lp->kb_putc)(LK_CL_SOUND);	
#endif /* FIXME */

		/*
		 * DIX converts a single repeated down to a up/down pair, so
		 * why do twice as much work as needed in the kernel?
		 */
#ifdef DEBUG_PRINT_EVENT
DPRINTF("pcxal_autorepeat: keycode 0x%x %s\n", lp->last, "DN");
#endif /* DEBUG_PRINT_EVENT */
		ws_enter_keyboard_event(lp->queue, lp->last, lp->p, 
			BUTTON_DOWN_TYPE);
		ws_wakeup_any_pending(lp->queue);
	}
	else
		KB_CLR_REPEATING(lp);
	splx(s);
}

void
pcxal_ring_bell(closure, kp)
    caddr_t closure;
    ws_keyboard *kp;
{	
	if (!kp->control.bell || !kp->control.bell_duration)
		return;

	ring_bell(kp->control.bell_pitch, kp->control.bell_duration);

	return;
}

int
pcxal_set_keyboard_control(closure, kp, wskp)
    caddr_t closure;
    ws_keyboard *kp;
    ws_keyboard_control *wskp;
{
	register ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register int flags = wskp->flags;

	noclicks = 1;

	if (flags & WSKBKeyClickPercent) {
		kp->control.click = wskp->click;
#ifdef FIXME
		/* SORRY, can only keep track of it... */
		if (wskp->click == 0)
			(*lp->kb_putc)(LK_CL_DISABLE);	
		else {
			register int volume;
			volume = (7 - ((wskp->click / 14) & 7)) | 0x80;
			(*lp->kb_putc)(LK_CL_ENABLE);
			(*lp->kb_putc)(volume);
		}
#endif /* FIXME */
	}

	/* deal with bell pitch if possible */
	if (flags & WSKBBellPitch) {
		kp->control.bell_pitch = wskp->bell_pitch;
	}
	
	/* deal with bell duration if possible */
	if (flags & WSKBBellDuration) {
		kp->control.bell_duration = wskp->bell_duration;
	}
	
	/* deal with bell volume if possible */
	if (flags & WSKBBellPercent) {
		/* SORRY, can only keep track of it... */
		kp->control.bell = wskp->bell;
	}
	if (flags & WSKBAutoRepeatMode) {
		kp->control.auto_repeat = wskp->auto_repeat;
	}
	if (flags & WSKBAutoRepeats) {
		bcopy(wskp->autorepeats, kp->control.autorepeats, 32);
	}
	if (flags & WSKBLed) {
		kp->control.leds = wskp->leds; /* FIXME? record as-is... */

		/*
		 * only check for those LEDs we actually have...
		 */
		if (wskp->leds & WSKBLed_NumLock)
			KB_SET_NUMLOCK(lp);
		else
			KB_CLR_NUMLOCK(lp);
		if (wskp->leds & WSKBLed_CapsLock)
			KB_SET_CAPSLOCK(lp);
		else
			KB_CLR_CAPSLOCK(lp);
		if (wskp->leds & WSKBLed_ScrollLock)
			KB_SET_HOLD(lp);
		else
			KB_CLR_HOLD(lp);

		pcxal_set_leds(lp);
	}
	noclicks = 0;
	return 0;
}

/*
 * Called to update the 3 LED's based on the keyboard status fields.
 * This routine gets called when any of the following keys are typed:
 * ^S/^Q, scroll lock key, caps lock key, num lock key.
 *
 * NOTE: this routine could be called from interrupt level, so we need to
 *       post the setting of the LEDs to a later/lower-IPL time...
 */
void
pcxal_set_leds(lp)
ws_keyboard_state *lp;
{
	int new_leds = 0;
	
	if (KB_IS_CAPSLOCK(lp)) new_leds |= PK_LED_LOCK;
	if (KB_IS_HOLD(lp)) new_leds |= PK_LED_HOLD;
	if (KB_IS_NUMLOCK(lp)) new_leds |= PK_LED_NUMLOCK;
	
	if (pcxal_keyboard_enabled)
		gpc_set_leds(new_leds);
	else
	{
		(void) gpc_output(PK_SETLEDS);
		(void) gpc_output(new_leds);
	}
}

/*
 * This routine processes keyboard characters when graphics is NOT on.
 * Called (indirectly) from wskint during its interrupt processing.
 */
void
pcxal_keyboard_char(lp, data)
    ws_keyboard_state *lp;
    u_short data;
{
	register ws_keyboard_control *kc = &lp->kp->control;
	register u_short c;
	register int prefix;
	struct tty *tp;
	void pcxal_autorepeat_char();

	tp = &slu.slu_tty[0];

	/*
	 * note current prefix state, clear old prefix state.
	 */
	prefix = KB_IS_PREFIX(lp);
	KB_CLR_PREFIX(lp);
	
	/*
	 * Check for various keyboard errors
	 */
#ifdef FIXME
/* disable this test, since PK_POR == 0xAA, which is LSHIFT UP code !! :-( */
	if (data == PK_POR) {
		KB_CLR_REPEATING(lp);
		untimeout(pcxal_autorepeat_char, lp);
		pcxal_reset_keyboard(lp);
		return;
	}
#endif /* FIXME */
	
/* FIXME FIXME - must be able to handle other non-standard returns */
/*               like: BAT Failure, ACK, RESEND, ECHO, Keyboard ID */
/*               and the others, since they *could* occur... :-(   */

	/*
	 * Check for Overrun Condition character
	 */
	if (data == 0x00 || data == 0xff) 
	{
		KB_CLR_REPEATING(lp);
#ifdef DEBUG_PRINT_OVERRUN
DPRINTF("pcxal_keyboard_char: Overrun character (0x%x) detected.\n", data);
#endif /* DEBUG_PRINT_OVERRUN */
		return;
	}
	
	/*
	 * Check for prefix code
	 */
#ifndef KBD_XLATE
	if (data == F0SEEN) {
		KB_SET_PREFIX(lp);
		goto finish_up;
	}
#else
	if (data == 0xe0 || data == 0xe1) /* ignore prefixes for now */
		goto finish_up;

	if (data & 0x80) {
		prefix = 1;
		data &= 0x7f;
	}
#endif /* !KBD_XLATE */

	if (!data || data > PCXAL_MAX_KEYCODE)
	{
	    ERR_PRINTF("pcxal_keyboard_char: keycode out of range (0x%x)\n",
		       data);
	    return;
	}
		
	/*
	 * Process according to key type
	 */
	switch (xlate[data].char_type)
	{

	case CAPS:
		/*
		 * If the PREFIX was seen, this is an UP event
		 * for the scancode just fetched, so ignore...
		 * This is because this key is "latched",
		 * and to do this we just handle DOWN strokes.
		 */
		if (prefix)
			return;
		KB_TGL_CAPSLOCK(lp);	/* toggle */
		pcxal_set_leds(lp);
		break;
		
	case SCROLL:
		/*
		 * If the PREFIX was seen, this is an UP event
		 * for the scancode just fetched, so ignore...
		 * This is because this key is "latched",
		 * and to do this we just handle DOWN strokes.
		 */
		if (prefix)
			return;
		if (!KB_IS_HOLD(lp)) {
			if ((tp->t_state & TS_TTSTOP) == 0)  {
			    	c = MAKE_CNTRL('S');
				KB_SET_HOLD(lp);
				pcxal_set_leds(lp);
			} 
			else
				c = MAKE_CNTRL('Q');
		}
		else {
			c = MAKE_CNTRL('Q');
			KB_CLR_HOLD(lp);
			pcxal_set_leds(lp);
		}
		(*linesw[tp->t_line].l_rint)(c, tp);
		break;
		
	case NUMLOCK:
		/*
		 * If the PREFIX was seen, this is an UP event
		 * for the scancode just fetched, so ignore...
		 * This is because this key is "latched",
		 * and to do this we just handle DOWN strokes.
		 */
		if (prefix)
			return;
		KB_TGL_NUMLOCK(lp);   /* toggle */
		pcxal_set_leds(lp);
		break;
		
	case LSHIFT:
	case RSHIFT:
		/*
		 * it doesn't matter if this is an UP or DOWN event,
		 *  simply toggle the current state...
		 */
#ifndef KBD_XLATE
		KB_TGL_SHIFT(lp);     /* toggle */
#else
		if (prefix)
			KB_CLR_SHIFT(lp);
		else
			KB_SET_SHIFT(lp);
#endif /* !KBD_XLATE */

		break;
		
	case LCTRL:
	case RCTRL:
		/*
		 * it doesn't matter if this is an UP or DOWN event,
		 *  simply toggle the current state...
		 */
#ifndef KBD_XLATE
		KB_TGL_CNTRL(lp);     /* toggle */
#else
		if (prefix)
			KB_CLR_CNTRL(lp);
		else
			KB_SET_CNTRL(lp);
#endif /* !KBD_XLATE */

		break;
		
	case LALT:
	case RALT:
		/*
		 * it doesn't matter if this is an UP or DOWN event,
		 *  simply toggle the current state...
		 */
#ifndef KBD_XLATE
		KB_TGL_ALT(lp);	      /* toggle */
#else
		if (prefix)
			KB_CLR_ALT(lp);
		else
			KB_SET_ALT(lp);
#endif /* !KBD_XLATE */

		break;
		
	case ASCII:
	case NUMPAD:
	case FUNCTION:
		
		/* if current state is "prefix", then this is an UP event
		 * for the scancode just fetched, so ignore...
		 */
		if (prefix)
#ifndef KBD_XLATE
			return;
#else
		{
			data |= 0x80; /* OR back in prefix bit */
			break; /* finish up like prefix char */
		}
#endif /* !KBD_XLATE */
		
#ifndef KBD_XLATE
		/*
		 * do some repeat processing... :-) :-)
		 */
		if (kc->auto_repeat && IsBitOn(kc->autorepeats, data) &&
		    (lp->last != data))
		{
			if (KB_IS_REPEATING(lp)) {
				KB_CLR_REPEATING(lp);
				untimeout(pcxal_autorepeat_char, lp);
			}
			timeout(pcxal_autorepeat_char, lp, lp->timeout);
			KB_SET_REPEATING(lp);
		}
#endif /* !KBD_XLATE */

		lp->last = data;
		
#ifdef KBD_XLATE
		/*
		 * Test for "magic" state and process special keys
		 */
		if (KB_IS_CNTRL(lp) && KB_IS_ALT(lp) && KB_IS_SHIFT(lp)) 
		{
			xlate_t *kp = NULL;
			
			if (xlate[data].unshifted == /*HOME*/0x132)
				kp = kk_table;
			else if (xlate[data].unshifted == /*END*/0x115)
				kp = kkundo_table;
			if (kp != NULL) 
			{
				int i = 0;
				while (kk_indices[i])
				{
					xlate[kk_indices[i++]] = *kp++;
				}
				return;
			}
		}
#endif /* KBD_XLATE */

		/*
		 * Test for control characters. If set, see if the 
		 * character is eligible to become a control character.
		 */
		if (KB_IS_CNTRL(lp)) {
			c = xlate[data].unshifted;
			if (c >= ' ' && c <= '~')
				c &= 0x1f;
                        if (c == MAKE_CNTRL('S')) {
                                KB_SET_HOLD(lp);
                                pcxal_set_leds(lp);
                        }
			else if (c == MAKE_CNTRL('Q')) {
                                KB_CLR_HOLD(lp);
                                pcxal_set_leds(lp);
                        }
		} 
		
                /*
                 * test for numeric keypad. If so, check NUMLOCK and
		 * load the appropriate character.
                 */
		else if (xlate[data].char_type == NUMPAD) {
			if (KB_IS_NUMLOCK(lp))
				c = xlate[data].shifted;
			else
				c = xlate[data].unshifted;
		}
		
                /*
                 * test for CAPSLOCK or SHIFT state. If so,
		 * load the appropriate character, otherwise
		 * load the unshifted version.
                 */
		else if (KB_IS_CAPSLOCK(lp) || KB_IS_SHIFT(lp))
			c = xlate[data].shifted;
		else
			c = xlate[data].unshifted;
		
		/*
		 * Check for special function keys
		 */
		if (c & 0x100) {
			register char *string;
			
			c &= 0x7f;
			if ((c < special_keys_size) &&
			    (string = special_keys[c]))
			{
			    while (*string)
				(*linesw[tp->t_line].l_rint) (*string++, tp);
			}
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
			pcxal_set_leds(lp);
		}
		/*
		 * all "normal" cases exit here...
		 */
		return;

	case UNKNOWN:
		ERR_PRINTF("pcxal_keyboard_char: char_type UNKNOWN\n");
		break;

	case IGNORE:
		ERR_PRINTF("pcxal_keyboard_char: char_type IGNORE\n");
		break;

	default:
		ERR_PRINTF("pcxal_keyboard_char: char_type DEFAULT\n");
		break;

		
	}
		
	/*
	 * All non-repeating cases come here via "break";
	 *  they all need to clear REPEATING status and
	 *  reset the timeout if REPEATING is currently
	 *  going on...
	 * Also, "last" is set to the scancode, so that
	 *  we get closure on the last repeat, if any.
	 */
finish_up:

	if (KB_IS_REPEATING(lp)) {
		KB_CLR_REPEATING(lp);
		untimeout(pcxal_autorepeat_char, lp);
	}
	lp->last = data;      /* so next down can be same */
	return;
}

void
pcxal_autorepeat_char(closure)
    caddr_t closure;
{
	register ws_keyboard_state *lp = (ws_keyboard_state *)closure;
	register ws_keyboard_control *kc = &lp->kp->control;
	int s = spltty();

	if (kc->auto_repeat && IsBitOn(kc->autorepeats, lp->last) &&
	    KB_IS_REPEATING(lp))
	{
		timeout(pcxal_autorepeat_char, lp, lp->interval);

		/*
		 * must send another down of the current key, just as if we
		 *  had gotten an interrupt for it...
		 * NOTE: key-click will be done by pcxal_keyboard_char.
		 */
		pcxal_keyboard_char(lp, lp->last);
	}
	else
		KB_CLR_REPEATING(lp);
	splx(s);
}

/*
 * routine to enable keyboard interrupts
 */
void
pcxal_enable_keyboard()
{
	int s, data;
	
	if (pcxal_keyboard_enabled) return; /* FIXME */

#ifdef DEBUG_PRINT_ENTRY
DPRINTF("pcxal_enable_keyboard entry.\n");
#endif /* DEBUG_PRINT_ENTRY */

	s = spltty();
	
        gpc_ctl_cmd(PK_CTL_RDMODE);
        data = gpc_ctl_input();
        data |= (PK_MODE_EKI | PK_MODE_EMI); /* must enable mouse too... */
        gpc_ctl_cmd(PK_CTL_WRMODE);
        gpc_ctl_output(data);

	splx(s);

#ifdef DEBUG_PRINT_MODE
DPRINTF("pcxal_enable_keyboard: wrote MODE 0x%x.\n", s);
#endif /* DEBUG_PRINT_MODE */

	pcxal_keyboard_enabled = 1; /* FIXME */
}

/*
 * routine to disable keyboard interrupts
 */
void
pcxal_disable_keyboard()
{
	int s, data;
	
#ifdef KBD_DEBUG
DPRINTF("pcxal_disable_keyboard entry.\n");
#endif /* KBD_DEBUG */

/* FIXME FIXME - may need to disable keyboard at controller first?? */
/* FIXME FIXME - that's because these commands may cause interrupts!! */

	s = spltty();

        gpc_ctl_cmd(PK_CTL_RDMODE);
        data = gpc_ctl_input();
        data &= ~(PK_MODE_EKI | PK_MODE_EMI); /* disable mouse too... */
        gpc_ctl_cmd(PK_CTL_WRMODE);
        gpc_ctl_output(data);

	splx(s);

#ifdef DEBUG_PRINT_MODE
DPRINTF("pcxal_disable_keyboard: wrote MODE 0x%x.\n", s);
#endif /* DEBUG_PRINT_MODE */

}
