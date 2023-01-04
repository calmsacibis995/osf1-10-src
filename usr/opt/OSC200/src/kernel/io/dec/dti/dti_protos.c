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
static char *rcsid = "@(#)$RCSfile: dti_protos.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/16 07:58:44 $";
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
 * dti_protos.c
 *
 * Modification history
 *
 * 19-Feb-92
 *
 *	- Don't print new device messages immediately after boot.
 *
 * 28-Oct-91 - R. Craig Peterson
 *
 *	- Adds substantial comments to the code
 *	- Fixes problem of hot-plugged devices with
 *	  non-random serial #'s not being recognized until
 *	  the next device poll after the series of timeouts.
 *	- Fixes application transmit for > 4 bytes
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
 * Implements the device independent layers of the DTi protocol.
 */

#include <io/dec/dti/dti_hdr.h>

extern u_char	*bp;		/* DEBUG */

#ifdef DEBUG
extern int dti_debug_level;
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
extern int	dti_feedback_q_depth, dti_init_done;

extern int dti_xmt(), dti_get_caps(), dti_check_id_rply_to();

/* This routine is the default device handler.  When a device
   is first recognized, this handler is assigned to it.  Any
   messages comming from the device will be send to this
   handler.

   Arguments are standard for handlers.  type indicates what
   type of message this is (it could be an error or a normal
   message).  mp points to the message buffer that is associated
   with the message.
   */

dti_handle_default(type, mp)
register struct dti_msg *mp;
{
    int	msg_type, x, i;
    extern int	dti_send_id_request();

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	mprintf("dti_handle_default\n");
	cprintf("dti_handle_default\n");
    }
#endif

    switch(type)
    {
    case DTI_T_MESG:		/* A normal message */
	break;

    case DTI_T_ERR:		/* A transmission error occured */
	/* We have in stat one of:
	   DTI_I_ARBL DTI_I_NACK DTI_I_TIMO
	   */

	if (mp->stat == DTI_I_NACK)
	{
	    x = splhigh();
	    if (dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat &
		DTI_IS_IDREQ == 0)
	    {
		dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat |=
		    DTI_IS_IDREQ;
		splx(x);
		/* We've haven't sent an ID Request to him yet */
		dti_send_id_request(mp->msg_dest_addr);

		timeout(dti_check_id_rply_to,
			    mp->msg_dest_addr,
			    DTI_VERIFY_ALIVE_INTERVAL);
	    } else
		splx(x);
	    /* We'll just let the timeout take care of this guy.  We can't
	       be sure that the NAK really matches the message we got back.
	       The I2C firmware doesn't guarantee that we can... */
	}

	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);

	return;

    case DTI_T_UNKERR:		/* Somebody err'ed... Maybe us? */
	untimeout(dti_send_id_request, mp->msg_dest_addr);
	dti_reset_device(mp->msg_dest_addr);

	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);

	return;
    }

    msg_type = mp->type_len & 0x80;

    if (msg_type)		/* Is this an operation? */
    {
	switch(mp->msg_op_code)
	{
	case DTI_OP_ATTN:
	    if (*mp->msg_op_data != 0) /* Check stat byte */
	    {
		dti_stats.bad_device++;
		mprintf("DTi device reported failure\n");
	    }

	    dti_id_everybody();

	    break;

	case DTI_OP_IDRPLY:
	    /* Turn off re-xmit */
	    untimeout(dti_send_id_request, mp->msg_source_addr);

	    x = splhigh();
	    dti_devices[DTI_ADDR_TO_INDEX(mp->msg_dest_addr)].stat &= ~DTI_IS_IDREQ;
	    splx(x);
	    untimeout(dti_check_id_rply_to, mp->msg_source_addr);

	    dti_idrply(mp);
	    break;

	default:
#ifdef DEBUG
	    mprintf("Bad op code 0x%x at default addr 0x%x\n",
		    mp->msg_op_code, mp->msg_source_addr);
	    cprintf("Bad op code 0x%x at default addr 0x%x\n",
		    mp->msg_op_code, mp->msg_source_addr);
#endif
	    dti_reset_device(mp->msg_source_addr);
	    dti_stats.unexpect_op++;
	    break;
	}
    }
    else
    {
#ifdef DEBUG
	mprintf("Bad op code 0x%x at default addr 0x%x (type 0x%x)\n",
		mp->msg_op_code, mp->msg_source_addr, mp->type_len);
	cprintf("Bad op code 0x%x at default addr 0x%x (type 0x%x)\n",
		mp->msg_op_code, mp->msg_source_addr, mp->type_len);
#endif
	dti_stats.unexpect_op++;
    }
}

/* Send a poll to all possible devices to see who's home */
dti_poll_everybody(dummy)
{
    dti_id_everybody();
}

/* Resets all devices to default state */
dti_reset_all(dummy)
{
    int 	i, high;

    dti_reset_device(DTI_DEFAULT_ADDR);

    high = DTI_ADDR_TO_INDEX(DTI_DEFAULT_ADDR);

    for (i = 0; i < high; i++)
	dti_reset_device(DTI_INDEX_TO_ADDR(i));
}

/* Resets the device at the given address.  This will cause
   the device to go to its power-up state.
   */

dti_reset_device(addr)
u_int addr;
{
    struct dti_msg *mp;
    struct dti_devices *dd;
    int	x;

#ifdef DEBUG
    if (dti_debug_level > 0)
    {
	mprintf("dti_reset_device 0x%x\n", addr);
    }
#endif

    if ((unsigned)DTI_ADDR_TO_INDEX(addr) >= (unsigned)DTI_N_ADDR)
    {
#ifdef DEBUG
	mprintf("dti_reset_device: bad address 0x%x\n", addr);
#endif
	return;
    }

    dti_dflt_dd_entry(addr);

    x = splhigh();
    DTI_GETBUF(dti_mfree, mp);
    splx(x);

    if (!mp)
    {
	dti_stats.no_buffs++;
#ifdef DEBUG
	mprintf("dti_reset_device: (0x%x) dti_mfree empty\n");
#endif
	timeout(dti_reset_device, addr, hz / 2);
	return;
    }

    mp->msg_dest_addr = addr;
    mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 1);
    mp->msg_op_code = DTI_OP_RESET;
    mp->len = DTI_MESSAGE_OVHD + 1;
    dti_xmt(mp);
}

/* Default the entry in the dti_devices table for
   the device with the given address.  This causes
   the entry to be available for assignment to
   another device.

   This is typically done because the device at
   this address is no longer responding to id
   requests.
   */

dti_dflt_dd_entry(addr)
{
    struct dti_devices	*dd;
    int		x;

#ifdef DEBUG
    if (dti_debug_level > 0)
	mprintf("dti_dflt_dd_entry: defaulting addr 0x%x\n",
		addr);
#endif

    x = splhigh();

    dd = &dti_devices[DTI_ADDR_TO_INDEX(addr)];

    dd->addr = addr;
    dd->err_cnt = 0;

    if (addr == DTI_DEFAULT_ADDR)
    {
	dd->stat = DTI_IS_CNFG;
	dd->handler = dti_handle_default;
    }
    else
    {
	dd->stat = 0;
	dd->handler = 0;
    }

    splx(x);
}

/* Start getting the capabilities string from the
   device that is at the specified entry in the
   dti_devices table.
   */

dti_auto_caps(dd)
struct dti_devices *dd;
{
    int		x;
    struct dti_msg	*op;

    if (!(dd->stat & DTI_IS_CNFG))
    {
#ifdef DEBUG
	mprintf("dti_auto_caps: device not cnfg'ed\n");
#endif
	return;
    }

    x = splhigh();
    DTI_GETBUF(dti_mfree, op);
    splx(x);

    if (!op)
    {
	dti_stats.no_buffs++;
	timeout(dti_auto_caps, dd, hz / 4);
	return;
    }

    op->msg_source_addr = dd->addr;

    dti_get_caps(DTI_T_MESG, op);

    x = splhigh();
    DTI_RELBUF(dti_mfree, op);
    splx(x);
}

/* Send an identification request to the device at
   the specified address.  These are most commonly
   used as queries to see if the device is still
   there.
   */

dti_send_id_request(addr)
u_int addr;
{
    register struct dti_msg *mp;
    int	x;

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	mprintf("dti_send_id_request 0x%x\n", addr);
	cprintf("dti_send_id_request 0x%x\n", addr);
    }
#endif
    x = splhigh();
    DTI_GETBUF(dti_mfree, mp);
    splx(x);

    if (!mp)
    {
	dti_stats.no_buffs++;
	/* We couldn't get a buffer, so we'll try again in a bit */

	timeout(dti_send_id_request, addr, hz / 4);
	return;
    }

    mp->msg_dest_addr = (u_char)addr;
    mp->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 1);
    mp->msg_op_code = DTI_OP_IDREQ;
    mp->len = DTI_MESSAGE_OVHD + 1;

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	dti_print_mp(mp, "dti_send_id_request:"); /* DEBUG */
    }
#endif

    dti_xmt(mp);
}

/* Called in response to the receipt of an identification
   reply from a device.  This message is generated automatically
   by a device when it first comes on line, and is also
   generated in response to an id request.
   */

dti_idrply(mp)
struct dti_msg *mp;
{
    int		i, x;
    u_char	addr;
    struct dti_devices *dd;
    struct dti_msg *op;

    if (mp->msg_source_addr != DTI_DEFAULT_ADDR)
    {
	/* This must be from a probe to see if the device was alive.
	   The fact that we got here means that it is. */

	dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)].stat &= ~DTI_IS_IDREQ;
	return;
    }

    /* Find an available addr */

    x = splhigh();

    /* First, let's see if we've already configured this guy */

    for (i = 0, dd = dti_devices; i < DTI_N_ADDR - 1; i++, dd++)
    {
	if (dd->stat)
	    if (!dti_bcmp(dd->data, mp->msg_op_data, DTI_ID_MSG_SIZE))
	    {
		if (mp->msg_source_addr == DTI_DEFAULT_ADDR &&
		    dd->stat & DTI_IS_CONFIGED)
		{
		    /* Apparently, we've been hot-plugged! */
#ifdef DEBUG
		    if (dti_debug_level > 1)
			mprintf("dti: Hot plug detected\n");
#endif
		    if (dd->stat & DTI_IS_IDREQ)
			untimeout(dti_check_id_rply_to, dd->addr);

		    dti_dflt_dd_entry(dd->addr);
		}
		else
		{
		    /* We already know about this guy.  Let's see if he's
		       still at his old address. */
		    splx(x);
#ifdef DEBUG
		    if (dti_debug_level > 0)
		    {
			mprintf("dup idrply with 0x%x (cmp = %d, i=%d, dd = 0x%x) %s\n",
				dd->addr, dti_bcmp(dd->data, mp->msg_op_data, DTI_ID_MSG_SIZE),
				i, dd, mp->msg_op_data);
			mprintf("size = %d The dup: %s\n", DTI_ID_MSG_SIZE, dd->data);
			dti_print_mp(mp, "dup idrply");
		    }
#endif
		    x = splhigh();
		    if (dd->stat & DTI_IS_IDREQ == 0)
		    {
			dd->stat |= DTI_IS_IDREQ;
			splx(x);

			dti_send_id_request(dd->addr);
			timeout(dti_check_id_rply_to,
				DTI_INDEX_TO_ADDR(i),
				DTI_VERIFY_ALIVE_INTERVAL);
		    } else
			splx(x);
		    return;
		}
		
	    }
    }
    
    /* Find an empty spot. */

    for (i = 0, dd = dti_devices; i < DTI_N_ADDR - 1; i++, dd++)
	if (!dd->stat)
	    break;

    splx(x);

    addr = DTI_INDEX_TO_ADDR(i);

    if (addr >= DTI_DEFAULT_ADDR)
    {
	mprintf("No more DTi addresses available (Address %d out of range)\n", addr);
	cprintf("No more DTi addresses available (Address %d out of range)\n", addr);
	return;
    }

    dd->stat = DTI_IS_CNFG;
    dd->handler = dti_get_caps;
    dd->addr = addr;

    bcopy(mp->msg_op_data, dd->data, DTI_ID_MSG_SIZE); /* preserve an exact copy  */

    /* A bunch of HARD-CODED stuff: TODO */

    dd->proto_rev = mp->msg_op_data[0];
    strncpy(dd->module_rev, &mp->msg_op_data[1], 7);
    strncpy(dd->vendor_nm, &mp->msg_op_data[8], 8);
    strncpy(dd->module_nm, &mp->msg_op_data[16], 8);

    /* The cast to int is to stop compiler bitching about shift size */
    dd->dev_no =
	((int)mp->msg_op_data[24] << 24) |
	    ((int)mp->msg_op_data[25] << 16) |
		(mp->msg_op_data[26] << 8) |
		    mp->msg_op_data[27];

#ifdef DEBUG
    mprintf("0x%x: %c | %s | %s | %s | %d\n", addr,
	    dd->proto_rev, dd->module_rev, dd->vendor_nm,
	    dd->module_nm, dd->dev_no);
#endif

    /* We'll now build an assign address message by changing the op
       code, adding the address to the end of the message, and adjusting
       the length based on the original length */

    x = splhigh();
    DTI_GETBUF(dti_mfree, op);
    splx(x);

    if (!op)
    {
	dti_stats.no_buffs++;
	return;
    }

    bcopy(mp, op, sizeof(struct dti_msg));

    op->msg_op_data[28] = addr;
    op->msg_op_code = DTI_OP_ASSGNADDR;
    op->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT,
				(mp->type_len & DTI_LEN_MASK) + 1);
    op->len += 1;

    dti_xmt(op);

    if (dti_awaiting_feedback &&
	(dti_feedback_mask & DTI_TRACE_DEV_CHNG))
	DTI_ADD_FEEDBACK(DTI_TRACE_DEV_CHNG | 0x100 | addr);

    /* Let's get his capabilities string */
    timeout(dti_auto_caps, dd, hz / 8);
}

/* For some reason bcmp wasn't giving the expected results.  Here's
   yet another implementation. */

dti_bcmp(s1, s2, len)
register char *s1, *s2;
register int len;
{
    while (len--)
	if (*s1++ != *s2++)
	    return (1);

    return (0);
}

/* This is the timeout routine for the get capabilities
   functionality.  If the device hasn't sent the requested
   portion within the timeout period, we start things
   over again.

   dd is a pointer to the dti_devices entry for this device.
   */

dti_get_caps_to(dd)
struct dti_devices *dd;
{
    struct dti_msg	*op;
    int	x;

#ifdef DEBUG
    if (dti_debug_level > 2)
    {
	mprintf("dti_get_caps_to\n");
	cprintf("dti_get_caps_to\n");
    }
#endif

    if (dd->err_cnt++ > DTI_MAX_TO_ERRS)
    {
#ifdef DEBUG
	if (dti_debug_level > 0)
	    mprintf("dti_get_caps_to: exceeded DTI_MAX_TO_ERRS (%d) threshold\n",
		    DTI_MAX_TO_ERRS);
#endif
	dti_reset_device(dd->addr);
	return;
    }

    x = splhigh();
    DTI_GETBUF(dti_mfree, op);
    splx(x);

    if (!op)
    {
	dti_stats.no_buffs++;
#ifdef DEBUG
	mprintf("dti_get_caps_to: no free dti_bufs\n");
#endif
	return;
    }

    op->msg_dest_addr = dd->addr;
    op->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 3);
    op->msg_op_code = DTI_OP_CAPREQ;
    op->msg_op_data[0] = (dd->caplen & 0xff00) >> 8;
    op->msg_op_data[1] = dd->caplen & 0xff;
    op->len = DTI_MESSAGE_OVHD + 3;

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	dti_print_mp(op, "dti_get_caps_to:"); /* DEBUG */
    }
#endif

    dti_xmt(op);

    /* Force a re-transmit if no party heard from */
    timeout(dti_get_caps_to, dd, hz / 30);
}

/* The get capabilities handler.  This handler is used for
   devices that are in the process of providing their
   capabilities strings.

   Standard handler arguments.
   */

dti_get_caps(type, mp)
struct dti_msg *mp;
{
    struct dti_msg	*op;
    struct dti_devices	*dd;
    int			len, offset, x;

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	cprintf("dti_get_conf called\n");
	mprintf("dti_get_conf called\n");
    }
#endif

    switch(type)
    {
    case DTI_T_MESG:		/* A normal message */
	if (dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)].stat & DTI_IS_IDREQ)
	{
#ifdef DEBUG
	    if (dti_debug_level > 0)
		mprintf("dti_dflt_kbd_mgr: resetting DTI_IS_IDREQ\n");
#endif
	    untimeout(dti_check_id_rply_to, mp->msg_source_addr);
	    dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)].stat &= ~DTI_IS_IDREQ;
	}
	break;

    case DTI_T_ERR:		/* A transmission error occured */
	/* We have in stat one of:
	   DTI_I_ARBL DTI_I_NACK DTI_I_TIMO
	   */

	/* We'll let the dti_get_caps_to take care of things since we
	   can't trust the NACK */

    case DTI_T_UNKERR:		/* Somebody err'ed... Maybe us? */
	/* We've already got a timer going, so if he doesn't reply
	   we'll kick him anyways.  We'll just ignore this potential
	   error. */

	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);

	return;
    }

    /* We've already given this device an address.  Now the user has
       done something with the device, so we have to get its
       characteristics, and then hand them up to a higher layer */

    x = splhigh();
    DTI_GETBUF(dti_mfree, op);
    splx(x);

    if (!op)
    {
	dti_stats.no_buffs++;
#ifdef DEBUG
	mprintf("dti_get_caps: no free dti_bufs\n");
#endif
	return;
    }

    /* The addr has already been checked, so we don't need to re-verify it here. */

    dd = &dti_devices[DTI_ADDR_TO_INDEX(mp->msg_source_addr)];

    /* Clear old timeout */
    untimeout(dti_get_caps_to, dd);

    dd->err_cnt = 0;

    if (dd->stat & DTI_IS_CAPREQ)
    {
	if (mp->msg_op_code != DTI_OP_CAPRPLY)
	{
	    if (mp->msg_op_code != DTI_OP_IDRPLY)
	    {
		/* We're going to restart the caprequest sequence.
		   We'll set up the pointers so they will cause the
		   routines further down to restart the process. */

		*dd->caps = '\000';
		dd->caplen = 0;
		offset = 1;
	    }
	}
	else
	{
	    len = (mp->type_len & DTI_LEN_MASK) - DTI_MESSAGE_OVHD - 1;

	    offset = (mp->msg_op_data[0] << 8) + mp->msg_op_data[1];

#ifdef DEBUG
	    if (dti_debug_level > 5)
	    {
		mprintf("in_len = %d, offset = %d, caplen = 0x%x\n",
			len, offset, dd->caplen);
	    }
#endif
	}

	if (offset == dd->caplen)
	{
	    /* We got what we're looking for */

	    if (len == 0)
	    {
		dd->caps[offset] = '\000';

		/* No data indicates we've hit the end of the string */
		dd->stat &= ~DTI_IS_CAPREQ;
		dd->stat |= DTI_IS_CONFIGED;

#ifdef DEBUG
		mprintf("0x%x %c %s %s %s %d caps: %s\n",
			dd->addr, dd->proto_rev, dd->module_rev, dd->vendor_nm,
			dd->module_nm, dd->dev_no, dd->caps);
#endif
		/* Only print out a message that we've found a device
		   if the device shows up after we've been running for
		   a little bit.  This should stop messages from appearing
		   on the console when we're booting, but should still
		   generate them for new devices the user plugs in some
		   time after we boot. */

		if (dti_init_done)
		    cprintf("0x%x %c %s %s %s %d\n",
			    dd->addr, dd->proto_rev, dd->module_rev, dd->vendor_nm,
			    dd->module_nm, dd->dev_no);

		if (dti_awaiting_feedback &&
		    (dti_feedback_mask & DTI_TRACE_DEV_CHNG))
		    DTI_ADD_FEEDBACK(DTI_TRACE_DEV_CHNG | 0x200 | dd->addr);

		x = splhigh(x);
		DTI_RELBUF(dti_mfree, op);
		splx(x);

		dti_find_driver(mp, dd);

		return;
	    }

	    if (offset + len >= DTI_CAP_SIZE)
		mprintf("dti: (0x%x) capabilities string too long!\n",
			dd->addr);
	    else
	    {
		/* We assume there are no NULLs in the string */
		/* the -2 on the end compensates for the offset in the buffer */

		bcopy(&mp->msg_op_data[2], &dd->caps[offset], len);

#ifdef DEBUG
		if (dti_debug_level > 5)
		    mprintf("caps: %s\n", dd->caps);
#endif
		dd->caplen += len;
	    }
	
	}
	else if (offset == 0)
	{
	    *dd->caps = '\000';
	    dd->caplen = 0;
	}
    }
    else
    {
	dd->stat |= DTI_IS_CAPREQ;
	dd->caplen = 0;

	/* No need to grab another 4k if we've already got it! */
	if (!dd->caps)
	   {
/*	    KM_ALLOC(dd->caps, u_char *, DTI_CAP_SIZE, KM_DEVBUF, KM_NOW_CL_CO_CA);
*/
	   dd->caps = (u_char *) kalloc (DTI_CAP_SIZE);
	   if (!dd->caps)
	      dti_panic("dti_get_caps: couldn't get memory for capabilities string\n");
	   else
	      bzero (dd->caps, sizeof(DTI_CAP_SIZE));
	   }
	*dd->caps = '\000';
	dd->caplen = 0;
    }

    op->msg_dest_addr = mp->msg_source_addr;
    op->type_len = DTI_TYPE_LEN(DTI_TYPE_CTRL_STAT, 3);
    op->msg_op_code = DTI_OP_CAPREQ;
    op->msg_op_data[0] = (dd->caplen & 0xff00) >> 8;
    op->msg_op_data[1] = dd->caplen & 0xff;
    op->len = DTI_MESSAGE_OVHD + 3;

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	dti_print_mp(op, "dti_get_caps:"); /* DEBUG */
    }
#endif

    dti_xmt(op);

    /* Force a re-transmit if no party heard from */
    timeout(dti_get_caps_to, dd, hz / 30);
}

#ifdef DEBUG
/* Prints the contents of a dti message */
dti_print_mp(mp, msg)
struct dti_msg	*mp;
char *msg;
{
    int		i, len;
    u_char	*cp;

    strcpy(bp, msg);

    len = mp->len;

    for (i = 0, cp = &mp->msg_start; i < len; i++, cp++)
    {
	strcat(bp, " 0x");
	strncat(bp, hex_string + ((*cp & 0xf0) >> 4), 1);
	strncat(bp, hex_string + (*cp & 0x0f), 1);
    }

    mprintf("%s\n", bp);
}
#endif

/* Given an an address from mp, and a dd entry, call of the
   the handler's manager routines to see if anyone would
   like to lay claim on this device.  Once a handler has
   claimed the device, assign it to the device.
   */

dti_find_driver(mp, dd)
struct dti_msg	*mp;
struct dti_devices	*dd;
{
    struct dti_dev_managers	*dm;
    int		i, pri;
    int		dev_index;
    struct dti_devices *dp;
    int		(*fp)();

    dev_index = DTI_ADDR_TO_INDEX(mp->msg_source_addr);

    dp = &dti_devices[dev_index];

#ifdef DEBUG
    if (dti_debug_level > 5)
	mprintf("dti_find_driver called\n");
#endif

    for (pri = 0; pri < 3; pri++)
	for (i = 0, dm = dti_dev_managers;
	     i < DTI_MAX_MANAGERS && dm->probe;
	     i++, dm++)
	    if (dm->probe)
		if (fp = (*dm->probe)(pri, dd))
		{
		    dp->handler = fp;
		    return;
		}

    mprintf("DTi: Unrecognized device: %s %s\n",
	    dd->data, dd->caps);
}

/* This routine is called as a timeout and checks to make sure
   that the addressed device has replied to an idrequest it was
   sent. */

dti_check_id_rply_to(addr)
{
    if ((dti_devices[DTI_ADDR_TO_INDEX(addr)].stat & DTI_IS_IDREQ) &&
	dti_devices[DTI_ADDR_TO_INDEX(addr)].err_cnt++ > 5)
    {
#ifdef DEBUG
	if (dti_debug_level > 0)
	{
	    mprintf("check_id_rply timed out, addr 0x%x\n", addr);
	    cprintf("check_id_rply timed out, addr 0x%x\n", addr);
	}
#endif
	if (dti_awaiting_feedback &&
	    (dti_feedback_mask & DTI_TRACE_DEV_CHNG))
	    DTI_ADD_FEEDBACK(DTI_TRACE_DEV_CHNG | addr);

	dti_dflt_dd_entry(addr);
    }
}

/* Called to send everyone an id request.  This is to check to see
   if any devices have "disappeared" (been unplugged).
   */

dti_id_everybody()
{
    int 	i, to_interval = 0, x;
    register struct dti_devices *dd;

#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	mprintf("dti_id_everybody called\n");
	cprintf("dti_id_everybody called\n");
    }
#endif
	
    /* No need to repoll in a fraction of the active poll
       interval.  Cancel the poll & re-schedule it for
       the right number of ticks from now. */

    untimeout(dti_poll_everybody, 0);
    timeout(dti_poll_everybody, 0, DTI_POLL_ACT_INTERVAL);
    
    /* We'll send one to in-use addresses to make sure
       no one disappeared, and to the default address to see
       if anyone came on-line. */

    /* We have to be careful about overloading the I2C firmware,
       therefore we'll send out our resets slowly. */

    for (to_interval = 0, i = DTI_N_ADDR - 1;
	 i >= 0;
	 i--)
    {
	dd = &dti_devices[i];
	
#ifdef DEBUG
	if (dti_debug_level > 9)
	    mprintf("i = %d, dd->stat = %d, dd->stat & DTI_IS_IDREQ = %d dd->stat & DTI_IS_CONFIGED = %d\n",
		    i, dd->stat, dd->stat & DTI_IS_IDREQ, dd->stat & DTI_IS_CONFIGED);
#endif
	/* If this device is fully configured and someone hasn't just
	   started an ID request for it, we'll go ahead and schedule
	   one to be sent */

	x = splhigh();
	if (dd->stat & DTI_IS_CONFIGED)
	{
	    if (!(dd->stat & DTI_IS_IDREQ))
	    {
		dd->err_cnt = 0;
		dd->stat |= DTI_IS_IDREQ;
	    }
	    splx(x);
	    timeout(dti_send_id_request, DTI_INDEX_TO_ADDR(i), to_interval);
	    timeout(dti_check_id_rply_to,
			DTI_INDEX_TO_ADDR(i),
			DTI_VERIFY_ALIVE_INTERVAL + to_interval);
	    to_interval += hz  / 32;
	} else
	    splx(x);
    }
    timeout(dti_send_id_request, DTI_DEFAULT_ADDR, to_interval);
}

/* Check a capabilities string to see if the specific capability is
   found therein.  For instance it is used to check to see if the
   prot field contains keyb.

   Arguments are: capabilities string, capability, quality
   */

char *
dti_check_caps(s, c, q)
char	*s, *c, *q;
{
    char *p;

    if (p = strstrl(s, c, '\000'))
    {
	while (*p && *p != '(')
	    p++;

	if (*p && strstrl(p, q, ')'))
	    return(c);
    }

    return(NULL);
}

/* Transmit the data found in mp to every device which uses
   func as its handler.  Used primarily to transmit LED, bell,
   and click commands to keyboards.
   */

dti_xmit_match(mp, func)
struct dti_msg *mp;
int	(*func)();
{
    struct dti_msg *op;
    register struct dti_devices	*dd;
    int		i, x, count = 0;

    for (dd = dti_devices, i = 0; i < DTI_N_ADDR; dd++, i++)
	if ((dd->stat & DTI_IS_CONFIGED) && /* Is it configured & is its handler us? */
	    dd->handler == func)
	{
	    x = splhigh();
	    DTI_GETBUF(dti_mfree, op);
	    splx(x);

	    if (!op)
	    {
		dti_stats.no_buffs++;
		continue;
	    }

	    bcopy(mp, op, sizeof(struct dti_msg));

	    op->msg_dest_addr = dd->addr;
	    dti_xmt(op);
	    count++;
	}

    return(count);
}
