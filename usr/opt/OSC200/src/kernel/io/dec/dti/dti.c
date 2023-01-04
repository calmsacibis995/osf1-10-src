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
static char *rcsid = "@(#)$RCSfile: dti.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/04/16 07:58:01 $";
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
 * dti.c
 *
 * Modification history
 *
 * 19-Feb-92 - R. Craig Peterson
 *
 *	- Fix some crash and burns under load.
 *	- Go to splhigh before modifying ioasic parameters
 *	  in dti_init().
 *	- Don't print new device messages immediately after boot.
 *	- Clean up some of the out-of-buffer conditions.
 *	- Throw buffers from the done list onto the free list
 *	  when we get low on buffers in the free list.
 *	- Implemented crude flow control on DTI_PUT_MSG ioctl.
 *
 * 06-Nov-91 - R. Craig Peterson
 *
 *	- Fix to allow multiple applications to open
 *	  the dti device & use it simultaneously.  Only
 *	  one application can use the "feedback" queue
 *	  at time.
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
 * 22-Oct-91 - R. Craig Peterson
 *
 *	- Adds lk521 "keymaps" to low level driver.
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
 * This file implements the lower level hardware drivers and initial
 * unwrapping of DTi messages.
 *
 */

/* Note that the DMA code in these routines does not work! */

#include <io/dec/dti/dti_hdr.h>
#include "dti.h"

#ifdef DEBUG
u_char	*bp = (u_char *)0;	/* DEBUG */

int	dti_debug_level = 0;
#endif

struct controller *dti_info[NDTI];
struct dti_buf dti_free;	/* Free list of dti buffers */
struct dti_buf dti_down;	/* DTi buffers ready to transmit */
struct dti_buf dti_done;	/* Buffers xmitted, waiting for ack */
struct dti_buf *dti_input;	/* Current input buffer */
struct dti_buf *dti_output;	/* Current output buffer */
struct dti_stats dti_stats;	/* statistics */
struct dti_devices dti_devices[DTI_N_ADDR]; /* Info on each device */
struct dti_msg dti_mfree;	/* Message free list */
struct dti_msg *prev_mp = (struct dti_msg *)0;

extern int (*(dti_loc_mgr()))(), dti_poll_everybody(), dti_check_id_rply_to();
extern int (*(dti_kbd_mgr()))();

char *dti_hello_string;		/* The dti power-up message */

/* List must be terminated by a null entry */

struct dti_dev_managers	dti_dev_managers[DTI_MAX_MANAGERS] =
{
    { dti_loc_mgr },
    { dti_kbd_mgr },
    { 0 }			/* Must be preserved */
};

extern unsigned char dti_to_lk201[];

/* List must be terminated by a null entry */

struct dti_keymaps dti_keymaps[DTI_MAX_KEYMAPS] =
{
   { "LK501", dti_to_lk201 },
   { "LK501-??", dti_to_lk201 },
   { "LK501-AA", dti_to_lk201 },
   { "LK501-AB", dti_to_lk201 },
   { "LK501-AC", dti_to_lk201 },
   { "LK501-AD", dti_to_lk201 },
   { "LK501-AF", dti_to_lk201 },
   { "LK501-AG", dti_to_lk201 },
   { "LK501-AH", dti_to_lk201 },
   { "LK501-AI", dti_to_lk201 },
   { "LK501-AK", dti_to_lk201 },
   { "LK501-AL", dti_to_lk201 },
   { "LK501-AM", dti_to_lk201 },
   { "LK501-AN", dti_to_lk201 },
   { "LK501-AP", dti_to_lk201 },
   { "LK501-AQ", dti_to_lk201 },
   { "LK501-AS", dti_to_lk201 },
   { "LK501-AV", dti_to_lk201 },
   { "LK521", dti_to_lk201 },
   { "LK521-??", dti_to_lk201 },
   { "LK521-AA", dti_to_lk201 },
   { "LK521-AB", dti_to_lk201 },
   { "LK521-AC", dti_to_lk201 },
   { "LK521-AD", dti_to_lk201 },
   { "LK521-AF", dti_to_lk201 },
   { "LK521-AG", dti_to_lk201 },
   { "LK521-AH", dti_to_lk201 },
   { "LK521-AI", dti_to_lk201 },
   { "LK521-AK", dti_to_lk201 },
   { "LK521-AL", dti_to_lk201 },
   { "LK521-AM", dti_to_lk201 },
   { "LK521-AN", dti_to_lk201 },
   { "LK521-AP", dti_to_lk201 },
   { "LK521-AQ", dti_to_lk201 },
   { "LK521-AS", dti_to_lk201 },
   { "LK521-AV", dti_to_lk201 },
   { "DEFAULT", dti_to_lk201 },
   { "", 0 }			/* Must be preserved */
};

u_char *prev_cp;
static int	first_through = 1;

/* Indicates if there is a transmit timeout pending. These
   timeouts are started when a message has been formed that we
   want to transmit.  If there is no response from the I2C hardware
   indicating that the transmission has occured, or has failed, we
   assume that the hardware/firmware is faulty, and reset it. */
int	dti_xmt_to_pending = 0;

/* Indicates if the DTi code is initialized. */
int dti_initialized = 0;
/* Indicates that initialization is complete, and that a timeout
   period has elapsed after the initialization. */
int dti_init_done = 0;

/* This is an array of integers that is allocated in dti_init.  It
   contains feedback events that are going to be passed on up to
   an application through an ioctl. */
int *dti_feedback;

/* dti_awaiting_feedback indicates if an application has performed
   an ioctl requesting some sort of feedback.  If this is true then
   events should be placed into the dti_feedback array (queue).

   dti_feedback_mask is a bit field which indicates what events
   should be queued up in dti_feedback.

   dti_feedback_q_depth indicates which array element in dti_feedback
   is available for new information. */
int	dti_awaiting_feedback = 0, dti_feedback_mask = 0, dti_feedback_q_depth;
int	dti_awaiting_put_msg = 0;

/* dti_last_data_position is a pointer into the current input
   buffer which indicates where the last piece of data was
   received.  It is used to determine if data was received by
   the interrupt routines since the last timeout.
   See dti_input_timeout() */
u_char *dti_last_data_position = (u_char *)0;

caddr_t		dti_stub_csr[] = { 0 };

struct driver dtidriver =
{
    dti_probe,			/* see if a driver is really there */
    (int(*)())0,		/* see if a slave is there */
    dti_attach,			/* controller attach routine */
    (int(*)())0,		/* device attach routine */
    (int(*)())0,		/* fill csr/ba to start transfer */
    dti_stub_csr,		/* device csr addresses */
    (int(*)())0,		/* name of a device */
    (int(*)())0,		/* backpointers to driver structs */
    "dti",			/* name of controller */
    dti_info,			/* backptrs to controller structs */
    /* Forget the rest... */
    /* want exclusive use of bdp's */
    /* size of first csr area */
    /* address space of first csr area */
    /* size of second csr area */
    /* address space of second csr area */
};

extern int dti_handle_default(), dti_get_input_buf();
extern int dti_process_input(), dti_reset_all();

/* We'll get an interrupt at which point we set up a timer to go off
   immediately.  When it goes off we take a look and see if we've
   received any data since that last timeout.  If we haven't, then we've
   got a message - if we have, then we timeout again.  The higher level code
   (dti_process_input) breaks this buffer up into messages if necessary.
 */

dti_input_timeout(force)	/* force is ignored */
{
    long	x = splhigh();
    int	len, *ip, i, nz = 0;
    u_char	*cp;
    struct dti_buf	*p;

    untimeout(dti_input_timeout, 0);
    untimeout(dti_input_timeout, 1);
    
#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	cprintf("dti_input_timeout\n");
    }
#endif

    if (!dti_input)
#ifdef DEBUG
    {
	dti_panic("input_timeout w/no input buffer\n");
	splx(x);
	return;
    }
#else
    {
	splx(x);
	dti_stats.no_ibuffs++;
	timeout(dti_get_input_buf, 1, hz / 8);
        return;
    }
#endif

#ifdef DMA
    NEEDS WORK!!!!

	if ((u_char *)*(DTI_RCV_DMA_PTR) == dti_last_data_position)
	{			/* We've got a live one */
	    len = (((int)(*(DTI_RCV_DMA_PTR) - RCV_BASE_ADDR(dti_input->buf))) >> DTI_ASIC_SHIFT) / sizeof(int);
	    cprintf("%d bytes total\n", len);
	    mprintf("%d bytes total ", len);

	    dti_input->error = 0;
	    dti_input->s = (u_char *)SHY_HALF_PAGE(dti_input->buf); /* Find where the data starts */
	    dti_input->len = len;
	    ip = (int *)dti_input->s;

	    for (i = 0; i < len; i++, ip++)
	    {
		if (*ip != 0xa5a5a5a5)
		    nz++;
		mprintf("0x%x ", *ip);
	    }
	    mprintf("\n");
	    cprintf("%d non-a5\n", nz);
	    mprintf("%d non-a5\n", nz);

	    dti_last_data_position = (u_char *)0;

	    *(DTI_RCV_DMA_PTR) = RCV_BASE_ADDR(dti_input->buf); /* Set our RCV base addr */
	}
	else
	{			/* Do it again, Sam */
	    cprintf("timeout again\n");

	    dti_last_data_position = (u_char *)*(DTI_RCV_DMA_PTR);
	    timeout(dti_input_timeout, 0, DTI_TIMEOUT_INTERVAL);
	}
#else
    if (dti_input->s == dti_last_data_position)
    {				/* We've got a live one */
	len = dti_input->len;

	dti_input->error = 0;

	cp = dti_input->s = dti_input->buf;

	p = dti_input;

	DTI_GETBUF(dti_free, dti_input);

	if (!dti_input)
	{
	    dti_stats.no_ibuffs++;

	    /* We're going to be dropping data until we get a buffer */

	    timeout(dti_get_input_buf, 1, hz / 8);
	}
	else
	{
	    dti_input->error = 0;
	    dti_input->s = dti_input->buf;
	    dti_input->len = 0;
	    dti_last_data_position = (u_char *)0;
	}
	if (p->len)
	{
	    p->s = p->buf;	/* Reset s for later processing */
	    splx(x);
	    dti_process_input(p);
	}
	else
	{
	    /* A zero length buf is probably allright.  'fer instance
	       if we receive a DTI_I_XMT indicating a successful xmission
	       the lower code will suck it out & not give it to us, and
	       if no other data comes in, we'll have a zero length buffer */

	    DTI_RELBUF(dti_free, p);
	    splx(x);
	}

#ifdef DEBUG
	if (dti_debug_level > 2 && len)
	{
	    strcpy(bp, "Received bytes:");

	    for (i = 0; i < len; i++, cp++)
	    {
		strcat(bp, " 0x");
		strncat(bp, hex_string + ((*cp & 0xf0) >> 4), 1);
		strncat(bp, hex_string + (*cp & 0x0f), 1);
	    }

	    mprintf("%s\n", bp);
	}
	else
	    if (dti_debug_level > 1)
		cprintf(".");
#endif
    }
    else
    {				/* Do it again, Sam */
#ifdef DEBUG
	if (dti_debug_level > 5)
	{
	    cprintf("timeout again\n");
	}
#endif

	dti_last_data_position = dti_input->s;
	splx(x);
	timeout(dti_input_timeout, 0, DTI_TIMEOUT_INTERVAL);
    }
#endif

}

#ifdef DMA
dtiintr()
{
    long	len, x;
    u_long	reg;

    x = splhigh();

    dti_stats.interrupts++;

    reg = *(SIR);

    /* How many bytes have we received? */
    len = (((int)(*(DTI_RCV_DMA_PTR) - RCV_BASE_ADDR(dti_input->buf))) >> DTI_ASIC_SHIFT) / sizeof(int);
    cprintf("%d ", len);

    if (reg & DTI_XMT_PEND)	/* Transmit Page End Interrupt */
    {
	reg &= ~DTI_XMT_PEND;
	cprintf("DTi Transmit Page End Intr\n");
    }

    if (reg & DTI_XMT_DMA_ERR)	/* Transmit DMA mem read error */
    {
	reg &= ~DTI_XMT_DMA_ERR;
	cprintf("DTi Transmit DMA mem read err.\n");
    }

    if (reg & DTI_RCV_HPINT)	/* Receive half page interrupt */
    {
	reg &= ~DTI_RCV_HPINT;
	cprintf("DTi Receive Half Page Intr\n");
    }

    if (reg & DTI_DMA_POR)	/* Receive DMA page overrun */
    {
	reg &= ~DTI_DMA_POR;
	cprintf("DTi Receive DMA Page Overrun\n");
    }

    *(SIR) = reg;

    if (!dti_last_data_position)
    {
	dti_last_data_position = (u_char *)*(DTI_RCV_DMA_PTR);
	timeout(dti_input_timeout, 0, DTI_TIMEOUT_INTERVAL);
    }

    splx(x);
}
#else
/* We've done a transmit & haven't received a reply.  Time to
   reset the I2C controller */

dti_xmt_to(type)
{
    int		x = splhigh(), i;
    u_long	reg;

    dti_xmt_to_pending = 1;

    if (type)
    {
#ifdef DEBUG
	if (dti_debug_level > 0)
	{
	    mprintf("Resetting I2C\n");
	}
#endif

	/* Disable rcv & xmt interrupts */

	reg = *(SIRM);
	reg &= ~DTI_RCV;
	reg &= ~DTI_XMT;
	*(SIRM) = reg;

	/* Reset the I2C chip */

	reg = *(SSR);
	reg |= DTI_RESET;
	*(SSR) = reg;

	dti_stats.hard_resets++;

	/* Run again in a second to enable the I2C chip */

	timeout(dti_xmt_to, 0, hz);

	dti_free_done_bufs();
    }
    else
    {
#ifdef DEBUG
	if (dti_debug_level > 5)
	{
	    mprintf("Enabling I2C\n");
	}
#endif

	first_through = 1;

	/* Enable rcv & xmt interrupts */

	reg = *(SIRM);
	reg |= DTI_RCV;
	reg |= DTI_XMT;
	*(SIRM) = reg;

	/* Enable DTi */

	reg = *(SSR);
	reg &= ~DTI_RESET;
	*(SSR) = reg;

	/* We're assuming that everyone else is ok, so we won't
	   zero out any structures, etc. */

	dti_set_i2c_software_sigs(DTI_SIG_MASK);
    }

    splx(x);
}

/* This routine is called when a transmit has been completed.
   This could mean that there was an error in transmission as
   well as a successful transmission.

   type indicates if there was a transmission error or not.
   */

dti_xmt_done(type)
{
    struct dti_buf	*p;
    int	x=splhigh();

    dti_xmt_to_pending = 0;
    untimeout(dti_xmt_to, 1);

    /* We got a good xmit, so let's yank the buffer from
       the done queue & put it in the free queue. */

    DTI_GETBUF(dti_done, p);

    if (!p)
    {
	splx(x);
#ifdef DEBUG
	if (dti_debug_level)
	    mprintf("dti_xmt_done: nothing on the done queue");
#endif
	return;
    }

    DTI_RELBUF(dti_free, p);
    
    splx(x);

#ifdef DEBUG
    if (dti_debug_level > 4)
	mprintf("xmt ack\n");
#endif
}

/* This is the primary interrupt routine.  This routine is called
   if there is a byte to read from the I2C controller, or if the
   I2C controller's transmit buffer has room.
   */

dtiintr()
{
    u_long	x, c, reg;
    static int	last_char_esc = 0;

    x = splhigh();

    dti_stats.interrupts++;

    reg = *(SIR);

    if (reg & DTI_RCV)		/* Byte Received */
    {
	if (first_through)	/* Throw away very first byte - it is trash */
	{
	    first_through = 0;
	    c = *(DTI_DATA_REG);
	    splx(x);
	    return;
	}

	/* The data is written as 0xfa00fa00 in the word, and
	   fa is the byte we're interested in. */

	c = (u_long)((*(DTI_DATA_REG) & 0xff00) >> 8);

	dti_stats.bytes_in++;

	if (c == DTI_I_XMT)
	    dti_xmt_done(DTI_I_XMT);
	else if (dti_input)
	{
	    *(dti_input->s) = (u_char)c;

	    dti_input->s++;
	    dti_input->len++;

	    if (dti_input->len >= DTI_BUF_SIZE)	/* We're out of room */
	    {
#ifdef DEBUG
		mprintf("dtiintr: Out of buffer space\n");
#endif
		last_char_esc = 0;
		dti_last_data_position = dti_input->s;
		dti_input_timeout(1);
	    }
	    
	    if (last_char_esc &&
		c != DTI_I_ESC)
	    {
		if (c == DTI_I_HELO) /* The hello msg is DTI_I_RCV terminated */
		{
		    last_char_esc = 0;
		}
		else
		{
		    untimeout(dti_input_timeout, 0);
#if 1
		    dti_last_data_position = dti_input->s;
		    dti_input_timeout(1);
#else
		    timeout(dti_input_timeout, 1, 0);
#endif
		}
		
	    }

	    if (!dti_last_data_position)
	    {
		dti_last_data_position = dti_input->s;
		timeout(dti_input_timeout, 0, DTI_TIMEOUT_INTERVAL);
	    }

	    /* If we've received something interesting, let's force the higher
	       code to take a look at it. */

	    switch (c)
	    {
	    case DTI_I_ESC:
		last_char_esc++;
		break;

	    case DTI_I_START:
	    case DTI_I_RCV:
		untimeout(dti_input_timeout, 0);
#if 1
		dti_last_data_position = dti_input->s;
		dti_input_timeout(1);
#else
		timeout(dti_input_timeout, 1, 0);
#endif
		break;

	    default:
		last_char_esc = 0;
		break;
	    }
	}
	else
	{
	    dti_stats.no_ibuffs++;
	    dti_get_input_buf(0);
	}
    }

    if (reg & DTI_XMT)		/* Transmit Complete */
    {
	/* Check if there's anything to do */

	if (dti_output)
	{
	    /* Write it out */

	    dti_stats.bytes_out++;

	    *(DTI_DATA_REG) = (*(dti_output->s) & 0xff) << 8;

	    /* There is a problem with the I2C chip not transmitting data &
	       not informing us that the data was indeed transmitted.
	       Therefore we set up a timer to go off in two seconds,
	       at which time we will force a hard reset on the I2C controller.
	       When we receive a DTI_I_XMT acknowledging the transmit we
	       will reset the timeout (or when we transmit again). */

	    if (!dti_xmt_to_pending)
	    {
		timeout(dti_xmt_to, 1, hz * 10);
		dti_xmt_to_pending = 1;
	    }

	    dti_output->len--;
	    dti_output->s++;

	    /* If there's no more chars in this buffer, get the
	       next.  If there's no more chars at all, turn off
	       xmit interrupt 'cuz we don't care any more. */

	    if (!dti_output->len)
		dti_prepare_for_xmit();
	}
	else
	{
	    DTI_DISABLE_XMIT();
	}
    }

    splx(x);
}

/* This routine is called if there was some sort of error that would
   effect the transmission.  It clears out the current transmit
   buffer (dti_output), and tells everybody that an error occured.

   reason is the error.
   */

dti_abort_output(reason)
{
    struct dti_buf *p;
    int	x;

    x = splhigh();
    DTI_GETBUF(dti_done, p);
    splx(x);

    if (p)
    {
	dti_inform_of_err(p, reason);
	x = splhigh();
	DTI_RELBUF(dti_free, p);
	splx(x);
    }
    else
    {
	if (dti_output)
	{
	    x = splhigh();

	    dti_output->len = 0;
	    dti_inform_of_err(dti_output, reason);
	    DTI_RELBUF(dti_free, dti_output);
	    DTI_GETBUF(dti_down, dti_output);
	    splx(x);
	}
    }

    dti_prepare_for_xmit();
}

/* This routine informs the various device handlers of
   an error.  We cannot be sure that this dti_buf we've
   been handed is actually the cause of the error, but
   we inform handlers of the error so that they can poll
   their devices which have state to see if something
   has changed.
   */

dti_inform_of_err(p, reason)
struct dti_buf	*p;
{
    struct dti_devices *dd;
    struct dti_msg	*mp;
    int		addr, x;
    int		(*handler)();

    addr = p->om.msg_dest_addr;

    if ((unsigned)DTI_ADDR_TO_INDEX(addr) >= (unsigned)DTI_N_ADDR)
    {
#ifdef DEBUG
	if (dti_debug_level)
	    mprintf("dti_inform_of_err: bad address 0x%x\n", addr);
#endif
	return;
    }

    dd = &dti_devices[DTI_ADDR_TO_INDEX(addr)];

    x = splhigh();
    handler = dd->handler;
    splx(x);

    if (!dd->stat ||
	!handler)		/* Not configured */
	return;

    x = splhigh();
    DTI_GETBUF(dti_mfree, mp);
    splx(x);

    if (!mp)
    {
	dti_stats.no_buffs++;
	return;
    }

    bcopy(&p->om, mp, sizeof(struct dti_msg));

    mp->stat = reason;

    (*handler)(DTI_T_ERR, mp);
}

/* This routine makes sure there is data ready to be transmitted
   in dti_output.  It may take a buffer ready for transmission from
   dti_down and put it into dti_output if there is nothing to be
   transmitted currently pointed to by dti_output.  Once it has
   determined if there is anything available, it will enable, or
   disable the transmit interrupts depending on the availability
   of data to transmit.
   */

dti_prepare_for_xmit()
{
    int x = splhigh();

    if (dti_output)
    {
	/* If we've just completed transmitting a buffer, put it on the done
	   list so we can inform the higher layers of code if there was an error.
	   */

	if (!dti_output->len)
	{
	    DTI_RELBUF(dti_done, dti_output);
	    DTI_GETBUF(dti_down, dti_output);
	}
    }
    else
	DTI_GETBUF(dti_down, dti_output);

    while (dti_output && !dti_output->len)
    {
	/* No data in this one, so let it go & get the next one. */

	DTI_RELBUF(dti_free, dti_output);
	DTI_GETBUF(dti_down, dti_output);
    }

    if (dti_output)
    {
	DTI_ENABLE_XMIT();
    }
    else
    {
	DTI_DISABLE_XMIT();
    }
    splx(x);
}

#endif

/* This routine takes an input buffer and breaks it up into its
   component messages.  It also parses all of the escape conventions
   that may be in the buffer.  Once a message has been created
   it passes it up to another routine which will give it to the
   handler for that address.
   */

dti_process_input(p)
struct dti_buf *p;
{
    struct dti_msg *mp;
    register u_char	*cp, *ip, *ep, *ecp;
    int		x;
    static int	had_overflow_error = 0;

    /* We may have data laying around from the last service... */

    if (prev_mp)
    {
	mp = prev_mp;
	cp = prev_cp;		/* Char Pointer for Message */
    }
    else
    {
	x = splhigh();
        DTI_GETBUF(dti_mfree, mp);
	splx(x);

        if (!mp)
	{
	    dti_stats.no_buffs++;
	    x = splhigh();
	    DTI_RELBUF(dti_free, p);
	    splx(x);
	    return;
	}

        cp = &mp->msg_start;	/* Char Pointer for Message */
    }

    ip = p->s;			/* Input Pointer */
    ep = ip + p->len;		/* End Pointer (input buffer) */
    ecp = &mp->msg_end;		/* End Char (Message) Pointer */

    for ( ; ip < ep; ip++)
    {
	if (cp >= ecp)		/* Message Full */
	{
	    /* This condition should truly never happen.  We'll throw
	       away the message & hope someone will recover... */
#ifdef DEBUG
	    mprintf("dti_process_input: message too big\n");
#endif
	    goto throw_away;
	}

	switch(*ip)
	{
	case DTI_I_START:
	    /*
	      START
	      If sent by the Host, generates an I2C repeated-START if
	      message-transmit is in progress, or is ignored if between
	      message frames.  Also used to regain synchronization after
	      certain errors (all characters up to the next START from
	      the host will be discarded).

	      Will only be received by the Host if an I2C repeated-START
	      occurs within an already-open message frame.  Note that
	      if a I2C repeated-START occurs immediately after the
	      checksum of an ODB message, no START will be sent to the
	      Host, since the RCV signal (or CKSM in the case of bad
	      checksum) sent to the Host has already closed the frame.
	      */
	    continue;

	case DTI_I_RCV:
	    /*
	      RCV is received by the Host to signal that a message with
	      valid checksum has been received completely (this need not
	      correspond to an I2C STOP condition).  This signal closes
	      any open frame.
	      */

	    /* TODO?  At this point we may have a message that is by DTi
	       standards incomplete.  For instance, if we rely on the
	       len field in the message we may find an invalid byte that
	       may tell us we want more data than we really do, etc.

	       We here make a compromise.  If we haven't received enough
	       data to fill up to the data portion, we'll wait around until
	       we get some more from the next bunch comming up from the chip
	       level.  Otherwise we assume that the timeout from the higher
	       code has taken care of breaking up the message appropriately.
	     */

	    if (cp == &mp->msg_start)
		continue;

	    /* If we've had an overflow error the following frame is likely
	       to not be in tact.  At that point we set the had_overflow_error
	       flag.  We'll therefore throw away the current frame (the one
	       following the overflow error). */

	    if (had_overflow_error)
	    {
		had_overflow_error = 0;
		cp = &mp->msg_start;
		continue;
	    }

	    if (cp < mp->msg_data)
	    {
		dti_stats.short_message++;
#ifdef DEBUG
		if (dti_debug_level > 1)
		{
		    cprintf("short message len %d last byte 0x%x\n",
			    cp - &mp->msg_start, *(cp - 1));
		    mprintf("short message len %d last byte 0x%x\n",
			    cp - &mp->msg_start, *(cp - 1));
		}
#endif
	    }

	    mp->len = cp - &mp->msg_start;
	    mp->stat = 0;	/* Indicate packet ok */

	    dti_process_message(mp);

	    cp = &mp->msg_start;
	    continue;

	case DTI_I_XMT:
	    /*
	      XMT is received by the Host to indicate that a message has
	      been successfully transmitted onto the I2C bus.  This allows
	      the Host to tell when a message has actually been sent,
	      as opposed to being buffered in the Output FIFO.
	      */
	    /* TODO */
	    dti_xmt_done(DTI_I_XMT);
#ifdef DEBUG
	    if (dti_debug_level > 3)
	    {
		mprintf("xmit success\n");
		cprintf("xmit success\n");
	    }
#endif
	    continue;

	case DTI_I_ESC:
esc_loop:
	    ip++;
	    switch(*ip)
	    {
	    case DTI_I_ESC:
		goto esc_loop;

	    case DTI_I_f8:	/* 0xe8 */
		*cp++ = 0xf8;
		continue;

	    case DTI_I_f9:	/* 0xe9 */
		*cp++ = 0xf9;
		continue;

	    case DTI_I_fa:	/* 0xea */
		*cp++ = 0xfa;
		continue;

	    case DTI_I_fb:	/* 0xeb */
		*cp++ = 0xfb;
		continue;

	    case DTI_I_STOP:
		/*
		  (I2C STOP, possibly premature.)

		  STOP is sent to the Host as the normal means of closing all
		  RAW-I2C frames.  STOP is also sent to close ODB frames, but
		  in this case it indicates the frame was shorter than expected
		  (from the frame's length field).
		  */
#ifdef DEBUG
		if (dti_debug_level > 2)
		{
		    mprintf("rcv stop\n");
		    cprintf("rcv stop\n");
		}
		else
		    if (dti_debug_level)
			cprintf("P");
#endif
		dti_stats.stop_on_rcv++;

		dti_inform_mgrs_unk_err();

		/* Throw away the message */

		cp = &mp->msg_start;
		continue;

	    case DTI_I_OVFL:
		/*
		  OVFL will be sent to the Host when the Controller's Input
		  FIFO overflows.  To ensure that the Host notices that Overflow
		  has occured, the Controller will discard all received bytes
		  until the Host has accepted the OVFL signal (emptying the
		  Input FIFO).  Note that the Controller MUST continue to
		  receive and acknowledge all messages (even though they are
		  discarded before reaching the Host) so that ODB Hardware
		  Signals can be detected and the Host interrupted or reset.
		  */
#ifdef DEBUG
		if (dti_debug_level > 2)
		{
		    mprintf("I2C overflow\n");
		    cprintf("I2C overflow\n");
		}
		else
		    if (dti_debug_level)
			cprintf("O");
#endif
		dti_stats.i_overflow++;

		dti_inform_mgrs_unk_err();

		/* Throw away the message */

		cp = &mp->msg_start;
		had_overflow_error++;

		continue;

	    case DTI_I_CKSM:
		/*
		  A message received with an invalid checksum should be ignored.
		  The CPU will signal the host indicating a checksum error was detected.
		  */
#ifdef DEBUG
		if (dti_debug_level > 2)
		{
		    mprintf("cksum err\n");
		    cprintf("cksum err\n");
		}
		else
		    if (dti_debug_level)
			cprintf("C");
#endif
		dti_stats.cksum_err++;

		dti_inform_mgrs_unk_err();

		/* Throw away the message */

		cp = &mp->msg_start;
		continue;

	    case DTI_I_ARBL:
		/*
		  Arbitration loss during transmit
		  */
#ifdef DEBUG
		if (dti_debug_level > 2)
		{
		    mprintf("arbl\n");
		    cprintf("arbl\n");
		}
		else
		    if (dti_debug_level)
			cprintf("A");
#endif
		dti_stats.arb_loss++;

		dti_inform_mgrs_unk_err();

		dti_abort_output(DTI_I_ARBL);
		continue;

	    case DTI_I_NACK:
		/*
		  A byte was not acknowledged during transmit
		  */
#ifdef DEBUG
		if (dti_debug_level > 2)
		{
		    mprintf("nak\n");
		    cprintf("nak\n");
		}
		else
		    if (dti_debug_level)
			cprintf("N");
#endif

		dti_xmt_done(DTI_I_NACK);

		/* No need to inform mgrs about this error, since it
		   is associated with a specific known address (the
		   one we're currently transmitting to). */

		/* Throw away this output buffer, someone doesn't like it */

		dti_abort_output(DTI_I_NACK);

		dti_stats.nack++;
		continue;

	    case DTI_I_TIMO:
		/*
		  TIMO indicates the 83c751 I2C watchdog timer expired during
		  transmission of a message frame.

		  We have no idea if this was on input or output.
		  */
#ifdef DEBUG
		if (dti_debug_level > 2)
		{
		    mprintf("timeout\n");
		    cprintf("timeout\n");
		}
		else
		    if (dti_debug_level)
			cprintf("T");
#endif
		dti_stats.timeout++;

		dti_inform_mgrs_unk_err();

		/* Throw away the message */

		cp = &mp->msg_start;

		dti_abort_output(DTI_I_TIMO);
		continue;

	    case DTI_I_RESET:
		dti_ack_i2c_reset(0);
		if (dti_awaiting_feedback &&
		    (dti_feedback_mask & DTI_TRACE_SIGNAL))
		{
		    DTI_ADD_FEEDBACK(DTI_TRACE_SIGNAL | 0);
		    wakeup(dti_feedback);
		}
		continue;

	    case DTI_I_DBUG:	/* Indicates an I2C controller firmware error */
		mprintf("dti: unknown I2C firmware error reported\n");
		cprintf("dti: unknown I2C firmware error reported\n");
		continue;

	    case DTI_I_HELO:
	    {
		/*
		  HELO status ROM-cksum ROM-id RCV

		  Sent to the Host to signal that the Controller has completed
		  it's power-on/reset self-tests.

		  Field		Description
		  -----		------------------------------------------
		  status	(1 char) self-test status (0 = okay, !0 = err)
		  ROM-cksum	(1 char) ROM checksum computed
		  ROM-id	(up to the RCV) firmware identification info.

		  ``status'' is zero in a successful self-test, non-zero values
		  indicate the particular error.

		  ``ROM-cksum'' is included to aide it determining the correct
		  `fudge' value to store in the ROM the obtain a zero checksum.

		  ``ROM-id'' is a ASCII string giving the revision id. of the
		  Controller firmware, terminated by the RCV signal.

		  *** NOTE *** Unlike arguments of all other commands/signals,
		  the ``ROM-id'' portion of HELO's arguments is
		  subject to byte-stuffing.
		  */
		char	*buf;
		int	cnt;

		buf = (char *) kalloc (DTI_MAX_HELO_SIZE);
		if (!buf)
		    panic("dti: can't alloc for dti_hello_string\n");

		dti_hello_string = buf;

		strcpy(buf, "I2C status ");

		++ip;		/* Status */

		if (*ip)
		{
		    strcat(buf, "failed:0x");
		    strncat(buf, hex_string + ((*ip & 0xf0) >> 4), 1);
		    strncat(buf, hex_string + (*ip & 0x0f), 1);
		}
		else
		    strcat(buf, "passed");

		++ip;		/* Checksum */

		if (*ip)
		{
		    strcat(buf, " checksum failed:0x");
		    strncat(buf, hex_string + ((*ip & 0xf0) >> 4), 1);
		    strncat(buf, hex_string + (*ip & 0x0f), 1);
		}
		else
		    strcat(buf, " checksum passed");

		strcat(buf, " I2C firmware rev. ");

		++ip;

		cnt = 0;

		while (cnt++ < DTI_MAX_HELO_SIZE &&
		       *ip != DTI_I_RCV)
		    strncat(buf, ip++, 1);

		strcat(buf, "\n");

#ifdef DEBUG
		cprintf(buf);
		mprintf(buf);
#endif
		continue;
	    }

	    case DTI_I_SRAW:
		mprintf("Raw I2C packet received - not supported\n");
		cprintf("Raw I2C packet received - not supported\n");
		goto throw_away;
	    default:
		mprintf("dti: Bad control sequence\n");
		cprintf("dti: Bad control sequence\n");
		goto throw_away;
	    }
	}
	*cp++ = *ip;
    }

    x = splhigh();
    DTI_RELBUF(dti_free, p);
    splx(x);

    prev_mp = mp;
    prev_cp = cp;
    return;

throw_away:
    x = splhigh();

    DTI_RELBUF(dti_free, p);

    splx(x);

    prev_mp = mp;
    prev_cp = &mp->msg_start;
}

/* This device takes a message that has just been received and
   parsed (mp), and hands it on upto the handler for that device.
   */

dti_process_message(mp)
struct dti_msg *mp;
{
    register int dev_index, x;
    register struct dti_devices *dd;
    int		(*handler)();

    if (!mp->len)
	return;

    dti_stats.messages_in++;

    dev_index = DTI_ADDR_TO_INDEX(mp->msg_source_addr);

    if (dev_index < 0 || dev_index >= DTI_N_ADDR)
    {
#ifdef DEBUG
	if (dti_debug_level)
	    mprintf("dti_process_message: bad address 0x%x\n", mp->msg_source_addr);
#endif
    }
    else
    {
	dd = &dti_devices[dev_index];
	mp->dd = dd;		/* Keep it around for later */

	x = splhigh();
	handler = dd->handler;
	splx(x);

	/* Was there an ID request outstanding for this device? */
	if (dd->stat & DTI_IS_IDREQ)
	{
	    untimeout(dti_check_id_rply_to, mp->msg_source_addr);
	    dd->stat &= ~DTI_IS_IDREQ;
	}

	if (dd->stat & DTI_IS_CNFG &&
	    handler)
	    handler(DTI_T_MESG, mp);
	else
	{
#ifdef DEBUG
	    mprintf("Message from unknown device 0x%x\n", mp->msg_source_addr);
#endif
	    dti_reset_device(mp->msg_source_addr);
	    dti_stats.unknown_addr++;
	}
    }
}

/* This routine is called when there has been an error that has occured
   somewhere on the DTi.  Most of the time it is impossible to associate
   the device that had/caused the error.  We therefore inform all device
   managers (handlers) that an unknown error has occured.  At that point
   they are expected to retreive the state of the device (if it is state
   oriented such as a keyboard or mouse buttons).

   If they've got some sort of timeouts going in the event of a device
   non-response, they can just let their timeout expire.

   The entire decision on what to do is up to the device manager.
   */

dti_inform_mgrs_unk_err()
{
    struct dti_devices *dd;
    struct dti_msg	*mp;
    int		i, x;
    int		(*handler)();

    dti_xmt_done(0);

#ifdef DEBUG
    if (dti_debug_level > 0)
	cprintf("U");
#endif

    dti_stats.polls_due_to_err++;

    for (dd = dti_devices, i = 0;
	 i < DTI_N_ADDR;
	 dd++, i++)
    {
	x = splhigh();
	handler = dd->handler;
	splx(x);

	if (!handler)		/* Any handler? */
	    continue;

	x = splhigh();
	DTI_GETBUF(dti_mfree, mp);
	splx(x);

	if (!mp)
	{
	    dti_stats.no_buffs++;
	    return;
	}

	mp->msg_dest_addr = dd->addr;
	mp->dd = dd;

	(*handler)(DTI_T_UNKERR, mp);
    }
}

/* This routine is called by a timeout whenever there are no
   buffers left on the dti_free queue, and such a buffer is
   needed for the input data buffer. */

dti_get_input_buf(dummy)
{
    int x;

    x = splhigh();

    untimeout(dti_get_input_buf, 1);

    dti_free_done_bufs();

    if (!dti_input)
    {
	DTI_GETBUF(dti_free, dti_input);
	if (!dti_input)
	{
	    dti_stats.no_ibuffs++;
	    timeout(dti_get_input_buf, 1, 1);
	}
    }
    
    splx(x);
}

dti_free_done_bufs()
{
    int x;
    struct dti_buf	*p;

    /* Check if there are any buffers laying around on the done list
       and reclaim them (pick them up next pass) */

    x = splhigh();

    for (; ; )
    {
	DTI_GETBUF(dti_done, p);
	if (!p)
	    break;
	DTI_RELBUF(dti_free, p);
    }
    splx(x);
}

#ifdef DMA
DMA XMIT NOT SUPPORTED
#else

/* This routine is called when a message has been created
   which need to be transmitted.  The message is checked
   for basic validity, escape characters are added for
   any special characters, and a transmit buffer is
   built.  This buffer is then added to the transmit
   queue, and transmissions are readied.

   mp is a pointer to the message to be transmitted.
   */

dti_xmt(mp)
register struct dti_msg *mp;
{
    struct dti_buf *p;
    int		i;
    u_int	x;
    u_char	*cp, *op;

    if (!mp->len)
    {
#ifdef DEBUG
	if (dti_debug_level > 0)
	    mprintf("dti_xmt: Zero len buf\n");
#endif
	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);
	return;
    }

    x = splhigh();
    DTI_GETBUF(dti_free, p);
    splx(x);

    if (!p)
    {
	dti_stats.no_buffs++;
	x = splhigh();
	DTI_RELBUF(dti_mfree, mp);
	splx(x);
	dti_free_done_bufs();
	return;
    }

    dti_stats.messages_out++;

    p->error = 0;
    p->s = p->buf;
    p->len = 0;

    /* Keep around the original message */
    bcopy(mp, &p->om, sizeof(struct dti_msg));

    op = p->buf;
    cp = &mp->msg_start;

    /* Always shove a start in front of messages so we're
       sure they make it out. (A start is needed to acknowledge
       an error received from the I2C controller.   Sending
       a START in front of every message makes the acknowledgement
       easier (we don't explicitly have to acknowledge errors).
       */

    *op++ = DTI_I_START;
    p->len++;

    while (mp->len--)
    {
	switch (*cp)
	{
	case DTI_I_START:	/* 0xf9 */
	    *op++ = DTI_I_ESC;
	    *op++ = DTI_I_f9;
	    p->len += 2;
	    break;
	case DTI_I_RCV:		/* 0xfa */
	    *op++ = DTI_I_ESC;
	    *op++ = DTI_I_fa;
	    p->len += 2;
	    break;
	case DTI_I_XMT:		/* 0xfb */
	    *op++ = DTI_I_ESC;
	    *op++ = DTI_I_fb;
	    p->len += 2;
	    break;
	case DTI_I_ESC:		/* 0xf8 */
	    *op++ = DTI_I_ESC;
	    *op++ = DTI_I_f8;
	    p->len += 2;
	    break;
	default:
	    *op++ = *cp;
	    p->len++;
	    break;
	}
	cp++;
    }

#ifdef DEBUG
    if (dti_debug_level > 2)
    {
	strcpy(bp, "Transmit:");

	for (i = 0, cp = p->s; i < p->len; i++, cp++)
	{
	    strcat(bp, " 0x");
	    strncat(bp, hex_string + ((*cp & 0xf0) >> 4), 1);
	    strncat(bp, hex_string + (*cp & 0x0f), 1);
	}

	mprintf("(%d) %s\n", dti_stats.bytes_out, bp);
    }
    else
	if (dti_debug_level > 1)
	    cprintf(",");
#endif

    x = splhigh();

    DTI_RELBUF(dti_down, p);	/* Put the xmit buffer onto the
				   transmit queue. */
    DTI_RELBUF(dti_mfree, mp);

    splx(x);

    dti_prepare_for_xmit();	/* Get transmissions going */
}
#endif

/* Called at system initialization */

dti_probe(vbaddr, ui)
caddr_t vbaddr;
struct device *ui;
{
#ifdef DEBUG
    if (dti_debug_level > 5)
    {
	dprintf("dti_probe called\n");
    }
#endif
    dti_init();
    return (-1);
}

/* Called at system initialization */

dti_attach(ui)
struct device *ui;
{
    if (cpu != DS_MAXINE)
    {
	mprintf("Unknown DTi type/unit(%d), not init'ing - baseboard DTi init failed\n",
		ui->unit);
	cprintf("Unknown DTi type/unit(%d), not init'ing - baseboard DTi init failed\n",
		ui->unit);
	return;
    }

    dti_init();
}

/* This routine is called some time after (~30 seconds) the initialization
   of the dti code is complete.  This is used for new device messages
   on the console, so that they are only printed well after the
   system has been initialized. */

dti_init_timeout(dummy)
{
    dti_init_done = 1;
}

/* Perform DTi specific initializations.  Memory is allocated, and
   variables are initialized to an appropriate state.
   */

dti_init()
{
    static int	init_in_progress = 0;
    int	i;
    long	x;
    u_long	reg;
    struct dti_buf *p;
    struct dti_msg *mp;

    if (dti_initialized ||
	init_in_progress)
	return;

    init_in_progress++;

    if (cpu != DS_MAXINE)
    {
	mprintf("Unknown cpu type for DTi\n");
	cprintf("Unknown cpu type for DTi\n");
	return;
    }

    dti_feedback = (int *) kalloc (DTI_FEEDBACK_Q_SIZE * sizeof(int));
    
    if (!dti_feedback)
	panic("dti: couldn't kalloc for feedback queue\n");

#if 0
    /* Initialize our device table */

    dti_devices = (struct dti_devices *) kalloc (DTI_N_ADDR * sizeof(struct dti_devices));

    if (!dti_devices)
	panic("dti: couldn't kalloc for dti_devices\n");
#endif

    /* Initialize our two-way linked lists */

    dti_free.next = dti_free.prev = &dti_free;
    dti_down.next = dti_down.prev = &dti_down;
    dti_done.next = dti_done.prev = &dti_done;
    dti_mfree.next = dti_mfree.prev = &dti_mfree;

#ifdef DEBUG
    bp = (u_char *) kalloc (8*1024);
#endif

    /* Get memory for our pools */

    for (i = 0; i < DTI_IO_POOL_SIZE; i++)
    {
	p = (struct dti_buf *) kalloc (sizeof(struct dti_buf));
	if (!p)
	    dti_panic("init dti_free pool w/kalloc");
	p->buf = (u_char *) kalloc (DTI_BUF_SIZE);
	bzero (p->buf, DTI_BUF_SIZE);
	p->error = 0;
	p->s = p->buf;
	p->size = DTI_BUF_SIZE;
	p->len = 0;
	DTI_RELBUF(dti_free, p); /* Put the buffer into the pool */
    }

    for (i = 0; i < DTI_MSG_POOL_SIZE; i++)
    {
	mp = (struct dti_msg *) kalloc (sizeof(struct dti_msg));
	if (!mp)
	    dti_panic("Can't kalloc memory for dti_mfree\n");
	else
	    bzero (mp, sizeof(struct dti_msg));

	DTI_RELBUF(dti_mfree, mp);
    }

    DTI_GETBUF(dti_free, dti_input);

    if (!dti_input)
	panic("dti_init: no free dti_bufs\n");

    /* Enable the I2C controller */

    x = splhigh();

    reg = *(SSR);
    reg &= ~DTI_RESET;		/* Turn off the reset bit */
    *(SSR) = reg;

    splx(x);

#ifdef DMA
    /* Program the slot register */

    reg = *(DTI_DMA_SLOT);	/* read/write */
    cprintf("*DTI_DMA_SLOT = 0x%x\t", reg);
    *(DTI_DMA_SLOT) = 0xa;
    cprintf("0xa written\n");

    /* Set up our DMA receive base address */

    cprintf("dti_input->buf = 0x%x, half page addr = 0x%x, setting RCV_DMA to 0x%x\n",
	    dti_input->buf, SHY_HALF_PAGE(dti_input->buf), RCV_BASE_ADDR(dti_input->buf));

    mprintf("dti_input->buf = 0x%x, half page addr = 0x%x, setting RCV_DMA to 0x%x\n",
	    dti_input->buf, SHY_HALF_PAGE(dti_input->buf), RCV_BASE_ADDR(dti_input->buf));

    *(DTI_RCV_DMA_PTR) = RCV_BASE_ADDR(dti_input->buf); /* Set our RCV base addr */
    cprintf("wrote DTI_RCV_DMA_PTR\n");

    {
	int *ip = (int *)dti_input->buf, i;

	for (i = 0; i < 1024; i++, ip++)
	    *ip = 0xa5a5a5a5;
    }

    /* Enable RCV DMA's and interrupts */

    reg = *(SSR);
    cprintf("*SSR = 0x%x\t", reg);
    reg |= DTI_RCV_DMA;
    cprintf("0x%x\t",reg);
    *(SSR) = reg;
    cprintf("written\n");
#else
    /* Disable transmit interrupts, enable receive */
    x = splhigh();

    reg = *(SIRM);
    reg &= ~DTI_XMT;
    reg |= DTI_RCV;
    *(SIRM) = reg;

    splx(x);
#endif

    dti_set_i2c_software_sigs(DTI_SIG_MASK);

    dti_dflt_dd_entry(DTI_DEFAULT_ADDR);

    dti_initialized++;
    init_in_progress = 0;

    dti_reset_all(0);		/* Reset all devices */

    timeout(dti_poll_everybody, 0, hz * 4); /* Start polling all addresses */
    timeout(dti_init_timeout, 0, hz * 30); /* set dti_init_done flag */
}

/* The I2C controller allows us to intercept various "signals".  These
   signals indicate some sort of user intervention, such as <CTRL>-<ALT>-<DEL>
   combinations on a keyboard to reset the machine.  There are three
   levels of these signals, and various options can be set by the software.

   16. Host Mode Register (CI_HSTM).

	The CI_HSTM control sets 1-byte of flags specifying how the I2Ctlr
	handles the Host's RESET and HALT lines.

	I2C Mode Flag		If Set
	-------------		------
	bit 0 HisHalt		Sig(Halt) asserts DebugIntr
	bit 1 RisReset		Sig(Reset) initiates HostReset  *1
	bit 2 RisAttn		Sig(Attn) initiates HostReset   *1
	bit 3 RDoesNMI		HostReset is asserted via P1.1  *2
	bit 4 RDly0		LSB, select reset-pending delay
	bit 5 RDly1		MSB, select reset-pending delay
	bit 6 *RFU
	bit 7 *RFU

	RDly1	RDly0	Delay Before Hard Reset
	bit 5	bit 4	(seconds)
	-----	-----	-----------------------
	0	0	none (Hard Reset is immediate)
	0	1	1
	1	0	5
	1	1	10

	*RFU = Reserved for Future Use, these bits MUST be 0.

	*1 HostReset may be initiated by either or both Sig(ATTN) or
	   Sig(RESET).  If both RisReset and RisAttn are 0, HostReset is
	   disabled.

	*2 For debugging purposes, HostReset may be set to assert the 
	   DebugIntr instead of a HardReset by setting RDoesNMI.  If set,
	   this "redirection" would apply in all cases where a HostReset was
	   generated (ie immediate or delayed).

	Default value is 0x03:
		Sig(Halt) asserts DebugIntr (HALT),
		Sig(Reset) asserts HardReset (RESET) immediately.

	Examples:

	To allow keyboard Halt (C-A-RET), allow keyboard Reset (C-A-DEL),
	and allow software to handle Reset with 5 second grace period:
		0x23

	Same as above, but with Halt (C-A-RET) disabled:
		0x22

   */

dti_set_i2c_software_sigs(sig_mask)
{
    int	x;
    struct dti_buf *p;
    u_char	*op;

    x = splhigh();
    DTI_GETBUF(dti_free, p);
    splx(x);

    if (!p)
    {
	dti_free_done_bufs();
	dti_stats.no_buffs++;
	timeout(dti_set_i2c_software_sigs, sig_mask, hz / 4);
	return;
    }

    p->error = 0;
    op = p->s = p->buf;
    p->len = 0;

    bzero(&p->om, sizeof(struct dti_msg));

    /* Always shove a start in front of messages so we're
       sure they make it out. */

    *op++ = DTI_I_START;
    p->len++;

    *op++ = DTI_I_ESC;
    p->len++;

    *op++ = DTI_I_SET_HSTM;
    p->len++;

    *op++ = sig_mask;
    p->len++;

    x = splhigh();
    DTI_RELBUF(dti_down, p);
    splx(x);

    dti_prepare_for_xmit();
}

/* This routine is called to acknowledge the receipt of a
   reset pending message from the I2C controller.  If we
   do not acknowledge the message the machine will be
   reset.
   */

dti_ack_i2c_reset(dummy)
{
    int	x;
    struct dti_buf *p;
    u_char	*op;
    
    x = splhigh();
    DTI_GETBUF(dti_free, p);
    splx(x);

    if (!p)
    {
	dti_free_done_bufs();
	dti_stats.no_buffs++;
	timeout(dti_ack_i2c_reset, 0, hz / 4);
	return;
    }

    p->error = 0;
    op = p->s = p->buf;
    p->len = 0;

    bzero(&p->om, sizeof(struct dti_msg));

    /* Always shove a start in front of messages so we're
       sure they make it out. */

    *op++ = DTI_I_START;
    p->len++;

    *op++ = DTI_I_ESC;
    p->len++;

    *op++ = DTI_I_RESET;
    p->len++;

    splhigh();
    DTI_RELBUF(dti_down, p);
    splx(x);

    dti_prepare_for_xmit();
}

/* Open routine.  */

dtiopen(dev, flag)
dev_t	dev;
int	flag;
{
    return(0);
}

/* Close routine. */

dticlose(dev, flag)
dev_t	dev;
int	flag;
{
    dti_awaiting_put_msg = dti_awaiting_feedback = dti_feedback_mask = 0;
    dti_feedback_q_depth = 0;
    return(0);
}

/* Read routine.  Does nothing. */

dtiread(dev, uio)
dev_t	dev;
struct	uio	*uio;
{
    return(0);
}

/* Write routine.  Does nothing. */

dtiwrite(dev, uio)
dev_t	dev;
struct	uio	*uio;
{
    return(0);
}

/* The ioctl routine which allows all of the interface
   between the application and the driver.
   */

dtiioctl(dev, cmd, data, flag)
dev_t	dev;
int	cmd;
caddr_t	data;
int	flag;
{
    int	error = 0, x, pos, *ip = (int *)data;
    struct dti_msg *mp;
    struct dti_devices *dd;

    switch(cmd)
    {
	/* Get the size of the dti_devices table (how many
	   valid addresses are there?)
	   */
    case DTI_GET_DEV_SIZE:
	*ip = DTI_N_ADDR;
	break;

	/* Get a device table entry. */
    case DTI_GET_DEV_TBL:
	x = *ip;
	if (x >= DTI_N_ADDR || x < 0)
	{
	    error = EINVAL;
	    break;
	}

	bcopy(&dti_devices[x], data, sizeof(struct dti_devices));
	break;

	/* Transmit the given message */
    case DTI_PUT_MSG:
	/* Make sure that both the msg list and the transmit list
	   buffers are more than 1/3 full.  If there are too many
	   messages in either pool sleep for a tenth of a second
	   and check again.  Pretty crude, but it saves time
	   checking for sleeping processes on every DTI_PUTBUF
	   and then waking them up.  I don't expect this to happen
	   very often. */

	while (dti_mfree.stat < (DTI_MSG_POOL_SIZE / 3) ||
	       dti_free.stat < (DTI_IO_POOL_SIZE / 3))
	{
	    /* Not enough free space in buffers */
	    dti_awaiting_put_msg++;
	    timeout(wakeup, &dti_awaiting_put_msg, hz / 10);
	    sleep(&dti_awaiting_put_msg, PZERO+10);
	    dti_awaiting_put_msg--;
	}

	x = splhigh();
	DTI_GETBUF(dti_mfree, mp);
	splx(x);

	if (!mp)
	{
	    dti_stats.no_buffs++;
	    error = ENOBUFS;
	    break;
	}

	bcopy(data, mp, sizeof(struct dti_msg));

	dti_xmt(mp);
	break;

	/* Get the length of the caps string for a particular device */
    case DTI_GET_CAPS_LEN:
	x = *ip;
	if (x >= DTI_N_ADDR || x < 0)
	{
	    error = EINVAL;
	    break;
	}

	if (dti_devices[x].caps)
	    *ip = dti_devices[x].caplen;
	else
	    *ip = 0;
	break;

	/* Get a portion of the capabilities string. */
    case DTI_GET_CAPS_POS:
    {
	struct dti_caps *cp;

	cp = (struct dti_caps *)data;
	
	x = cp->index;
	pos = cp->pos;

	if (x >= DTI_N_ADDR || x < 0)
	{
	    error = EINVAL;
	    break;
	}

	dd = &dti_devices[x];

	if (pos > dd->caplen || x < 0)
	{
	    error = EINVAL;
	    break;
	}

	if (dd->caps)
	{
	    cp->pos = min(DTI_GET_CAPS_CHUNK, dd->caplen - pos + 1);
	    bcopy(dd->caps + pos, cp->string, cp->pos);
	}
	else
	{
	    cp->pos = 0;
	    cp->string[0] = '\000';
	}
	break;
    }

	/* Set the mask which indicates what kind of feedback
	   we want from the device.  Being able to directly
	   read the keyboard or the mouse is restricted to
	   the super user ony for security reasons.
	   */
    case DTI_SET_FEEDBACK_MASK:
	/* We only really want to have one process grabbing feedback
	   at a time.  This can be diffcult to control.  We have other
	   ioctl's that allow things which can certainly occur in a
	   serial fashion that we don't want to block in the open.

	   Doing it like this will cause an application to notice a
	   failure when it does its ioctl to set the feedback mask.
	   It does cause a side-effect that the application must
	   close & reopen the device if it wants to change the mask.
	   */

	if (dti_awaiting_feedback)
	{
	    error = ENXIO;
	    break;
	}

#if  SEC_BASE
	if ((!privileged(SEC_SYSATTR, 0)) &&
	    (*ip &= DTI_TRACE_LOC | DTI_TRACE_KBD |
		DTI_TRACE_KBD_SVR))
#else
	if ((suser(u.u_cred, &u.u_acflag) != 0) &&
	    (*ip &= DTI_TRACE_LOC | DTI_TRACE_KBD |
	     DTI_TRACE_KBD_SVR))
#endif	SEC_BASE
	    error = EACCES;
	else
	    dti_feedback_mask = *ip;
	break;

	/* Get the first entry from the feedback array. */
    case DTI_GRAB_FEEDBACK:
	x = splhigh();

	if (!dti_awaiting_feedback)
	    dti_awaiting_feedback++;

	while (dti_feedback_q_depth == 0)
	    sleep(dti_feedback, PZERO+10);

	*ip = dti_feedback[0];
	dti_feedback_q_depth--;

	for (pos = 0; pos < dti_feedback_q_depth; pos++)
	    dti_feedback[pos] = dti_feedback[pos + 1];

	splx(x);
	break;

	/* Set the software signal mask. */
    case DTI_SET_HSTM_MASK:
#ifdef  SEC_BASE
	if (!privileged(SEC_SYSATTR, 0))
#else
	if (suser(u.u_cred, &u.u_acflag) != 0)
#endif	SEC_BASE
	{
	    error = EPERM;
	    break;
	}

	dti_set_i2c_software_sigs(*ip);
	break;

    default:
#ifdef DEBUG
	cprintf("dtiioctl -default 0x%x\n", cmd);
#endif
	error = EINVAL;
    }
    return(error);
}

/* Dummy routine */
dtistop()
{
    return(0);
}

/* Dummy routine */
dtireset()
{
    return(0);
}

/* Dummy routine */
dti_tty()
{
    return(0);
}

/* Dummy routine */
dtiselect()
{
    return(0);
}

/* Implementation of the standard strcat routine */

char *
strcat(p, s)
register char *p, *s;
{
    char *start = p;

    while (*p)
	p++;

    while (*p++ = *s++)
	;
    return(start);
}

/* Implementation of the standard strncat routine */

char *
strncat(p, s, n)
register char *p, *s;
register int n;
{
    char *start = p;

    while (*p)
	p++;

    while (n--)
	*p++ = *s++;

    *p = '\000';
    return(start);
}

/* A modified strstr routine.  This can't be recursive as
   we're in the kernel */

char *
strstrl(s1, s2, limit)
register char *s1, *s2;
char	limit;
{
    register char	*save, *p2 = s2;

    while (*s1)
    {
	if (*s1 == limit)
	    return(NULL);

	if (*s1 == *p2)
	{
	    save = s1;

	    while (*s1 && *p2 && *s1 == *p2)
	    {
		s1++;
		p2++;
	    }

	    if (!*p2)
		return(save);

	    p2 = s2;
	    s1 = save + 1;
	}
	else
	    s1++;
    }
    return(0);
}

/* Used when something nasty happens in order to record some
   info about the state of the subsystem. */

dti_panic(s)
char	*s;
{
    long	reg;

    mprintf("dti_panic: %s", s);
#ifdef DEBUG
    cprintf("==> dti_panic: %s", s);
#endif

    mprintf("dti_mfree %d dti_free %d dti_down %d dti_done %d\n",
	    dti_mfree.stat, dti_free.stat, dti_down.stat, dti_done.stat);

    mprintf("bytes_in %d bytes_out %d messages_in %d messages_out %d interrupts %d no_ibuffs %d\n",
	    dti_stats.bytes_in, dti_stats.bytes_out, dti_stats.messages_in, dti_stats.messages_out,
	    dti_stats.interrupts, dti_stats.no_ibuffs); 

    mprintf("no_buffs %d unknown_addr %d unexpect_op %d bad_device %d stop_on_rcv %d i_overflow %d\n",
	    dti_stats.no_buffs, dti_stats.unknown_addr, dti_stats.unexpect_op, dti_stats.bad_device,
	    dti_stats.stop_on_rcv, dti_stats.i_overflow); 

    mprintf("cksum_err %d arb_loss %d nack %d timeout %d short_message %d polls_due_to_err %d hard_resets %d\n",
	    dti_stats.cksum_err, dti_stats.arb_loss, dti_stats.nack, dti_stats.timeout,
	    dti_stats.short_message, dti_stats.polls_due_to_err, dti_stats.hard_resets); 
    
    dti_xmt_to(1);
}

/*
 * DTi dummy routines.  These routines always return a one
 * to fake out the ws driver into thinking the mouse and
 * keyboard are ok.  In actuality we'll use the DTi (Desktop
 * BusInterconnect) keyboard and mouse via the X-extension.
 */

dti_mouse_init()
{
/*	OSF cannot deal with this when called early in the 
*	boot process (before pmap_bootstrap is called) because
*	dti_init makes calls to kalloc.

    dti_init();
*/
    return(1);
}

dti_mouse_putc(c)
int c;
{
#ifdef DEBUG
    if (!dti_initialized)
	dprintf("dti_mouse_putc: %c (0x%x)\n", c, c);
    else
	mprintf("dti_mouse_putc: %c (0x%x)\n", c, c);
#endif
    return(1);
}

/* This routine is called by the server/ws-driver to find
   out what kind of locator is attached.
   */

dti_mouse_getc()
{
    /* The fakeout string makes us look like a mouse */
    static char fakeout[4] = { 0, 2, 0, 0 };
    static char *fp = fakeout;
    
#if 0
/*	OSF cannot deal with this when called early in the 
 *	boot process (before pmap_bootstrap is called) because
 *	dti_init makes calls to kalloc.
 */
    if (!dti_initialized)
    {
#ifdef DEBUG
	cprintf("initing\n");
#endif
        dti_init();
    }
#endif

    if (fp > &fakeout[3])
	fp = fakeout;

    return(*fp++);
}
