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
static char *rcsid = "@(#)$RCSfile: dti_loc.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/01/28 11:57:51 $";
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
 * dti_loc.c
 *
 * Modification history
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
 * Locator (mouse/tablet) code.
 *
 * Support only currently exists for the VSXXX mouse
 *
 */

/*#include "../io/ws/vsxxx.h"
*/
#include <io/dec/ws/vsxxx.h>
#include <io/dec/dti/dti_hdr.h>

extern (*vs_gdkint)();
struct  mouse_report    current_rep;

extern u_char	*bp;		/* DEBUG */

#ifdef DEBUG
extern int dti_debug_level;
#define abs(x)	(x<0?-(x):x)
#endif

extern struct dti_buf dti_free;	/* Free list of dti buffers */
extern struct dti_buf dti_down;	/* DTi buffers ready to transmit */
extern struct dti_buf dti_done;	/* Buffers xmitted, waiting for ack */
extern struct dti_buf *dti_input; /* Current input buffer */
extern struct dti_buf *dti_output; /* Current output buffer */
extern struct dti_stats dti_stats; /* statistics */
extern struct dti_devices dti_devices[]; /* Info on each device */
extern struct dti_msg dti_mfree; /* Message free list */
extern struct dti_dev_managers	dti_dev_managers[];

extern int	*dti_feedback, dti_awaiting_feedback, dti_feedback_mask;
extern int	dti_feedback_q_depth;

extern int dti_xmt(), dti_get_caps(), dti_check_id_rply_to();

/* I'm a generic mouse manager, so I'll only respond when priority
   gets to 2. */

int
(*(dti_loc_mgr(pri, dd)))()
struct dti_devices	*dd;
{
    char *p;
    extern int	dti_mouse_mgr();

    if (pri < 2)
	return(NULL);

    /* We're looking for something of the form: prot(locator) */

    if (!dti_check_caps(dd->caps, "prot", "locator"))
	return(NULL);

#ifdef DEBUG
    if (dti_debug_level > 0)
	if (dti_check_caps(dd->caps, "type", "mouse"))
	    mprintf("Found mouse\n");
	else
	    mprintf("Found locator\n");
#endif
    return(dti_mouse_mgr);
}

dti_mouse_mgr(type, mp)
register struct dti_msg *mp;
{
    u_short	button_state;
    short	dx, dy;
    int		x;
    struct mouse_report *new_rep;

    switch(type)
    {
    case DTI_T_MESG:		/* A normal message */
	if (dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat & DTI_IS_IDREQ)
	{
#ifdef DEBUG
	    if (dti_debug_level > 0)
		mprintf("dti_dflt_kbd_mgr: resetting DTI_IS_IDREQ\n");
#endif
	    dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat &= ~DTI_IS_IDREQ;

	    untimeout(dti_check_id_rply_to, mp->msg_dest_addr);
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
		/* We've haven't sent an ID Request to him yet */
		dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat |= DTI_IS_IDREQ;
		splx(x);
		dti_send_id_request(mp->msg_dest_addr);

		timeout(dti_check_id_rply_to, mp->msg_dest_addr, DTI_VERIFY_ALIVE_INTERVAL);
	    } else
		splx(x);
	    /* We'll just let the timeout take care of this guy.  We can't be sure that
	       the NAK really matches the message we got back.  The I2C firmware doesn't
	       guarantee that we can... */
	}
#if 0
	else
	{
#ifdef DEBUG
	    if (dti_debug_level > 1)
		mprintf("dti_dflt_loc_mgr: retransmitting due to %d\n",
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
#if 0				/* TODO: The mouse doesn't always report polled
				   info correctly */
#ifdef DEBUG
	if (dti_debug_level > 2)
	    cprintf(" mu0x%x ", mp->msg_dest_addr);
#endif
	mp->msg_source_addr = mp->msg_dest_addr;
	mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 1);
	mp->msg_op_code = DTI_AP_LOC_POLL;
	mp->len = DTI_MESSAGE_OVHD + 1;
	dti_xmt(mp);
#else
	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);
#endif
	return;
    }

    /* We don't expect many op-code type messages */

    if (mp->type_len & 0x80)
    {
	if (mp->msg_op_code == DTI_OP_IDRPLY)
	{
#ifdef DEBUG
	    if (dti_debug_level)
		mprintf("loc still healty 0x%x\n", mp->msg_source_addr);
#endif
	    untimeout(dti_check_id_rply_to, mp->msg_source_addr);

	    dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)].stat &= ~DTI_IS_IDREQ;
	    return;
	}

#ifdef DEBUG
	mprintf("unknown mouse op code 0x%x\n",
		mp->msg_op_code);
#endif
	dti_stats.unexpect_op++;
#ifdef DEBUG
	if (dti_debug_level > 0)
	    dti_print_mp(mp, "failing message:");
#endif
	return;
    }

    if (!vs_gdkint)
	return;

    new_rep = &current_rep;

    button_state = (mp->msg_data[0] << 8) | (mp->msg_data[1]);
    dx = (mp->msg_data[2] << 8) | (mp->msg_data[3]); /* x movement */
    dy = (mp->msg_data[4] << 8) | (mp->msg_data[5]); /* y movement */

    if (dti_awaiting_feedback &&
	(dti_feedback_mask & DTI_TRACE_LOC))
    {
	DTI_ADD_FEEDBACK(DTI_TRACE_LOC | button_state);
	DTI_ADD_FEEDBACK(DTI_TRACE_LOC | (dx & DTI_FEEDBACK_MASK));
	DTI_ADD_FEEDBACK(DTI_TRACE_LOC | (dy & DTI_FEEDBACK_MASK));
    }

#ifdef DEBUG
    if (abs(dx) > 255 || abs(dy) > 255)
    {
	mprintf("mouse event out of range: x %d y %d (buttons 0x%x)\n",
		dx, dy, button_state);
	dti_print_mp(mp, "offending packet");
	cprintf("mouse event out of range: x %d y %d (buttons 0x%x)\n",
		dx, dy, button_state);
	if (dx > 255)
	    dx -= 255;
	if (dy > 255)
	    dy -= 255;
    }
#endif	
    new_rep->state = 0;		/* initialize state */

    if (button_state & DTI_AP_MB1)
	new_rep->state |= LEFT_BUTTON;

    if (button_state & DTI_AP_MB3)
	new_rep->state |= MIDDLE_BUTTON;

    if (button_state & DTI_AP_MB2)
	new_rep->state |= RIGHT_BUTTON;

    if (dx > 0)
    {
	new_rep->state |= X_SIGN;
	new_rep->dx = dx;
    }
    else {
	new_rep->state &= ~X_SIGN;
	new_rep->dx =  -(dx);
    }
    if (dy > 0)
    {
	new_rep->state |= Y_SIGN;
	new_rep->dy = dy;
    }
    else {
	new_rep->state &= ~Y_SIGN;
	new_rep->dy =  -(dy);
    }

#ifdef DEBUG
    if (dti_debug_level > 9)
    {
	cprintf("mouse event report: state = 0x%x, dx = %d, dy = %d\n", new_rep->state, new_rep->dx, new_rep->dy);
	mprintf("mouse event report: state = 0x%x, dx = %d, dy = %d\n", new_rep->state, new_rep->dx, new_rep->dy);
    }
#endif

    (*vs_gdkint)(0400);		/* 400 says line 1 */
    dti_stats.mouse_events++;
}
