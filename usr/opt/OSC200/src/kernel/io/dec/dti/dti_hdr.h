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
 * @(#)$RCSfile: dti_hdr.h,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1992/07/07 17:52:58 $
 */

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
 * dti_hdr.h
 *
 * Modification history
 *
 * 11-Feb-92 - R. Craig Peterson
 *
 *	- Changed MODIFIER macro so that the Up Arrow isn't interpreted
 *	  as a modifier key.
 *
 * 28-Oct-91 - R. Craig Peterson
 *
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
 * Header file for the DESKTOPinterconnect code
 */

#ifndef _dti_h_
#  define _dti_h_

#ifdef KERNEL

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/vmmac.h>
#include <sys/clist.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <io/common/devio.h>
#include <sys/exec.h>
#include <sys/proc.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <hal/scb.h>
#include <machine/cpu.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <hal/kn02ca.h>

#endif

#undef DEBUG

#define	cprintf	printf
#define mprintf printf

/*
 * MAXine baseboard register structure
 *
 * Control/status flags are in IO ASIC general-purpose registers -
 * SSR = system support register
 * SIR = system interrupt register
 * SIRM = system interrupt mask register
 *
 * RESET bit	at SSR[ bit 5 ] set to 1 to reset 83c751
 *
 * TXINT bit	at SIR[ bit 1 ] set to 1 when xmit buffer is empty
 * RXINT bit	at SIR[ bit 0 ] set to 1 when recv data available
 *
 * TXMASK bit	at SIRM[ bit 1 ] set to 1 to ENABLE Tx intr.
 * RXMASK bit	at SIRM[ bit 0 ] set to 1 to ENABLE Rx intr.
 *
 */

/* pointers (in KSEG1) to system registers */
#define SSR		((u_long *)PHYS_TO_K1(KN02CA_SSR_ADDR))
#define SIR		((u_long *)PHYS_TO_K1(KN02CA_SIR_ADDR))
#define SIRM		((u_long *)PHYS_TO_K1(KN02CA_SIRM_ADDR))

#define DTI_RESET	(1<<5)	/* reset the h/w */

/* Used to get a DTi buffer */
#define DTI_GETBUF(l,p)	{if ((l).next == &(l)) { (p) = 0; } else \
			 { (l).stat--; (p) = (l).prev; (l).prev = (p)->prev; ((p)->prev)->next = (p)->next;}}

/* Used to release a DTi buffer back to the pool */
#define DTI_RELBUF(l,p)	{(l).stat++; (p)->next = (l).next; ((l).next)->prev = (p); (l).next = (p); (p)->prev = &(l);}

#ifdef DMA
#define DTI_ASIC_SHIFT	3	/* How many bits to left shift because of TC hack */

#define DTI_XMT_DMA_PTR	((u_long *)PHYS_TO_K1(0x1c040050))
#define DTI_RCV_DMA_PTR	((u_long *)PHYS_TO_K1(0x1c040060))
#define DTI_DMA_SLOT	((u_long *)PHYS_TO_K1(0x1c040190))

#define DTI_RCV_DMA	(1<<28)	/* Enable/Disable receive DMA */
#define DTI_XMT_DMA	(1<<29)	/* Enable/Disable transmit DMA */

#define DTI_XMT_PEND	(1<<27) /* DTi Transmit Page End Intr */
#define DTI_XMT_DMA_ERR	(1<<26)	/* DTi Transmit DMA mem read err. */
#define DTI_RCV_HPINT	(1<<25)	/* DTi Receive Half Page Intr */
#define DTI_DMA_POR	(1<<24)	/* DTi Receive DMA Page Overrun	*/

/* This macro is used for the receive interrupt code.  The receive
   interrupt is generated when the dma pointer crosses a 2k page
   boundary.  In order to ensure that we hit such a boundary fairly
   soon, we need to take our pointer to a 4k segment that we've allocated,
   slam it down to a 4K boundary, and then bump it up one word shy of the
   next 2K boundary.  We then pass this address to the DMA RCV routines
   who will give us an interrupt after the first byte received. (The DMA
   routines expect & provide 1 byte/word.) */

#define SHY_HALF_PAGE(x)	((int)(x) + 0x7fc)

/* What is the base addr we want to use for receive? */
#define RCV_BASE_ADDR(x)	(SHY_HALF_PAGE((x)) << DTI_ASIC_SHIFT)

#else
#define DTI_DATA_REG	((u_long *)PHYS_TO_K1(0x1c280000+0x20000)) /* DTi Controller Data Register */
#define DTI_XMT		(1<<1)	/* DTi Transmit Complete Interrupt */
#define DTI_RCV		(1)	/* DTi Receive Buffer Full */
#endif

extern int cpu;

#define DTI_ADDR_TO_INDEX(a)	(((a) - 0x52) / 2) /* convert i2c address into a table index */
#define DTI_INDEX_TO_ADDR(a)	(((a) * 2) + 0x52) /* convert table index into an i2c address */

#define DTI_DISABLE_XMIT()	{u_long sirmreg; \
				 sirmreg = *(SIRM); \
				 sirmreg &= ~DTI_XMT; \
				 *(SIRM) = sirmreg; }

#define DTI_ENABLE_XMIT()	{u_long sirmreg; \
				 sirmreg = *(SIRM); \
				 sirmreg |= DTI_XMT; \
				 *(SIRM) = sirmreg; }

/* There are some assumptions about the default address such as:
       - it is the highest address
       - all devices initialize themselves to this address
 */
#define DTI_DEFAULT_ADDR	0x6e /* Default DTi address */
#define DTI_N_ADDR	15	/* How many assignable addresses are there?
				   (include default addr) */
#define DTI_HOST_ADDR	0x50

#define DTI_TYPE_DATA	0
#define DTI_TYPE_CTRL_STAT 1
#define DTI_LEN_MASK	0x7f	/* Mask to find len in DTi message */
#define DTI_TYPE_LEN(t, l)	(((t & 0x1) << 7) | (l & DTI_LEN_MASK))

#define DTI_MESSAGE_OVHD	2 /* Overhead of a DTi message */

#define DTI_TIMEOUT_INTERVAL	(2) /* How many ticks to wait for data to complete */
#define DTI_VERIFY_ALIVE_INTERVAL	(hz * 2) /* How long to wait between
						     an idrequest & idreply */
#define DTI_POLL_DFLT_INTERVAL	(hz * 5) /* How often to poll default address */
#define DTI_POLL_ACT_INTERVAL	(hz * 5) /* How often to poll all "active" addresses */

#define DTI_IO_POOL_SIZE	256 /* How many dti input/output buffers to allocate? */
#define DTI_MSG_POOL_SIZE	128 /* How many dti messages to set aside */
#define DTI_MAX_MANAGERS	64 /* How many device managers max can there be? */
#define DTI_MAX_KEYMAPS		128 /* How many keymaps max can there be? */

#define DTI_MAX_TO_ERRS		32 /* How many times can we get a time-out on
				       a single device during configuration? */

#define DTI_ID_MSG_SIZE		28 /* The size of an ID message */
#define DTI_MAX_HELO_SIZE	100 /* How big can the hello message be? */

#ifdef DMA
#define DTI_BUF_SIZE	(4 * 1024) /* 4K is magic to the ASIC */
#else
#define DTI_BUF_SIZE	256
#endif

#define DTI_CAP_SIZE	(4 * 1024) /* Space in which to store capabilities string */

#define KM_DTIBUF	KM_DEVBUF
#define KM_DTI_FLAGS	(KM_NOW_CL_CO_CA|KM_NOCACHE)

/* DTi Interface command encoding */

#define DTI_I_START	0xf9	/* Sync or I2C START */
#define DTI_I_RCV	0xfa	/* Receive-Okay signal */
#define DTI_I_XMT	0xfb	/* Transmit-Okay signal */
#define DTI_I_ESC	0xf8	/* Escape (Control-Introducer character) */
/* All the following must be preceded by the escape character */
#define DTI_I_ARBL	0x41	/* (A) Arbitration loss */
#define DTI_I_CKSM	0x45	/* (E) Checksum error */
#define DTI_I_DBUG	0x5a	/* (Z) Set debug flags */
#define DTI_I_HELO	0x48	/* (H) Hello, I'm ready */
#define DTI_I_NACK	0x4e	/* (N) Negative Ack */
#define DTI_I_OVFL	0x4f	/* (O) Overflow signal */
#define DTI_I_SRAW	0x52	/* (R) SRAW */
#define DTI_I_STOP	0x50	/* (P) I2C STOP */
#define DTI_I_TIMO	0x54	/* (T) I2C DTi_Timeout */
#define DTI_I_SET_ADDR	0x31	/* (1) Set I2C Address register to xx */
#define DTI_I_SET_I2CM	0x32	/* (2) Set I2C Mode register to xx */
#define DTI_I_SET_HSTM	0x33	/* (3) Set Host-Mode register to xx */
#define DTI_I_RESET	0x58	/* (X) Ack pending reset, or notify of pending reset*/
#define DTI_I_f8	0xe8	/* f8 data character */
#define DTI_I_f9	0xe9	/* f9 data character */
#define DTI_I_fa	0xea	/* fa data character */
#define DTI_I_fb	0xeb	/* fb data character */

#define DTI_SIG_MASK	0x23	/* Signal mask for HSTM control sequence */

/* dti_devices.stat defines */
#define DTI_IS_CNFG	(1<<0)	/* Device is configured with address */
#define DTI_IS_CAPREQ	(1<<1)	/* CapReq is in progress */
#define DTI_IS_CONFIGED	(1<<2)	/* Device is fully configured */
#define DTI_IS_IDREQ	(1<<3)	/* Sent an IdReq to see if dev is alive */

/* dti_buf and dti_msg .stat defines */
#define DTI_ST_REXMT	(1<<0)	/* Retransmit this buffer on error */

/* OP CODES */
#define DTI_OP_SIG	0xa0	/* Signal sent to computer */
#define DTI_OP_R0	0xc0	/* Vendor reserved command */
#define DTI_OP_R1	0xc1	/* Vendor reserved command */
#define DTI_OP_R2	0xc2	/* Vendor reserved command */
#define DTI_OP_R3	0xc3	/* Vendor reserved command */
#define DTI_OP_ATTN	0xe0	/* Attention */
#define DTI_OP_IDRPLY	0xe1	/* Id Reply */
#define DTI_OP_CAPRPLY	0xe3	/* Capabilities reply */
#define DTI_OP_RESET	0xf0	/* Device reset */
#define DTI_OP_IDREQ	0xf1	/* IdRequest */
#define DTI_OP_ASSGNADDR 0xf2	/* Assign address */
#define DTI_OP_CAPREQ	0xf3	/* Capabilities request */

/* Keyboard Application OP Codes */
#define DTI_AP_KB_CLICK	0x1	/* keyclick on keyboard */
#define DTI_AP_KB_BELL	0x2	/* bell on keyboard */
#define DTI_AP_KB_LED	0x3	/* leds on keyboard */
#define DTI_AP_KB_POLL	0x4	/* Poll keyboard for state */

/* Mouse Codes */
#define DTI_AP_LOC_SAMP_INT	0x1 /* Sampling Interval */
#define DTI_AP_LOC_POLL	0x2	/* Poll device for status */
#define	DTI_AP_MB1	(1<<0)	/* left button */
#define DTI_AP_MB2	(1<<1)	/* right button */
#define DTI_AP_MB3	(1<<2)	/* middle button */

/* Types for Device Managers to understand */
#define DTI_T_MESG	0	/* A normal message */
#define DTI_T_ERR	1	/* This message (from you) is bad/had an error */
#define DTI_T_UNKERR	2	/* An unknown error has occured.  Check the
				   status of the device mp's dest_addr points to.
				   Free mp when you're done with it. */

/* ioctl cmds */
#define DTI_GET_CAPS_CHUNK	116
#define DTI_FEEDBACK_Q_SIZE	64

#define DTI_ADD_FEEDBACK(x)	{if (dti_feedback_q_depth < DTI_FEEDBACK_Q_SIZE)\
				     dti_feedback[dti_feedback_q_depth++] = x; else\
					 {dti_feedback[DTI_FEEDBACK_Q_SIZE - 1] = -1;\
					      dti_stats.feedback_overflow++;};wakeup(dti_feedback);}

struct dti_caps
{
    int		index;
    int		pos;
    char	string[DTI_GET_CAPS_CHUNK];
};

/* Number of entries in dti_devices table */
#define DTI_GET_DEV_SIZE	_IOR('a', 1, int)
/* Copy table into user address space */
#define DTI_GET_DEV_TBL		_IOWR('a', 2, struct dti_devices) 
/* Send provided dti message */
#define DTI_PUT_MSG		_IOW('a', 3, struct dti_caps) /* Note:
				     This should be dti_msg, but the size
				     is too great for the BSD way of doing
				     things.  This shorter version will
				     still allow an entire legal packet
				     to be sent. */
/* Get caps string len */
#define DTI_GET_CAPS_LEN	_IOWR('a', 4, int)
/* Get caps string at position */
#define DTI_GET_CAPS_POS	_IOWR('a', 5, struct dti_caps)
/* Get info about keyboard type feedback */
#define DTI_SET_FEEDBACK_MASK	_IOW('a', 6, int)
#define DTI_FEEDBACK_MASK	0xffffff
#define DTI_INTERCEPT_LED	(1<<24)	/* LED change & status */
#define DTI_INTERCEPT_CLICK	(1<<25)	/* Key Click & volume */
#define DTI_INTERCEPT_BELL	(1<<26)	/* Bell sound & volume */
#define DTI_TRACE_SIGNAL	(1<<27)	/* Signal from device such as Reset, Halt, or Attn */
#define DTI_TRACE_LOC		(1<<28)	/* Give locator events */
#define DTI_TRACE_KBD		(1<<29)	/* Give keyboard events */
#define DTI_TRACE_KBD_SVR	(1<<30)	/* Give kbd events sent to server */
#define DTI_TRACE_DEV_CHNG	(1<<31)	/* A new device has been detected, or an
					   old one went away */
/* Wait for a feedback event to happen */
#define DTI_GRAB_FEEDBACK	_IOR('a', 7, int)
/* Set HSTM mask (see DTI_I_SET_HSTM) */
#define DTI_SET_HSTM_MASK	_IOW('a', 8, int)

/* Is this key a modifier (viz. shift, ctrl, alt)? */
#define MODIFIER(c)		(c >= 0xab && c <= 0xb2)

static char hex_string[] = "0123456789abcdef";

extern int	dti_probe(), dti_attach();
extern char	*strstrl(), *dti_check_caps(), *strcat(), *strncat();

struct dti_msg
{
    struct dti_msg *next;
    struct dti_msg *prev;
    struct dti_devices *dd;
    int		stat;
    int		len;		/* Length, used for transmission */
    union
    {
	u_char	start;		/* Pointer to the start of the msg */
	u_char	source;		/* Source address */
	u_char	dest;		/* Destination address */
    } addr;
    u_char	type_len;	/* Message type and len */
    union
    {
	struct
	{
	    u_char	dta[128]; /* Data */
	} p0;
	struct
	{
	    u_char	op_cde; /* Operation code */
	    u_char	dta[127]; /* Data */
	} p1;
    } d;
    u_char	msg_end;
#define msg_start	addr.start
#define msg_source_addr	addr.source
#define msg_dest_addr	addr.dest
#define msg_data	d.p0.dta
#define msg_op_code	d.p1.op_cde
#define msg_op_data	d.p1.dta
};

struct dti_buf
{
    struct dti_buf *next;	/* Pointer to next buffer */
    struct dti_buf *prev;	/* Pointer to previous buffer */
    int		stat;
    int		error;		/* Passed up from lower layer,
				   indicates an error occurred. */
    u_char	*s;		/* Pointer to the start of the data
				   for the next protocol layer */
    int		size;		/* The size of the data buffer -DON'T MODIFY!!*/
    int		len;		/* Length of data stored here */
    u_char	*buf;		/* The data itself */
    struct dti_msg om;		/* The original message */
};

struct dti_devices
{
    int		stat;		/* see dti_devices.stat defines */
    int		addr;		/* easy in to get the address */
    int		err_cnt;	/* Used by state machines */
    int		(*handler)();	/* function that will handle data from this device */
    u_char	*caps;		/* Pointer to capabilities string */
    short	caplen;		/* Used by caprequest to keep track of data rcv'ed */
    u_char	*mgr;		/* Reserved for device manager's use */
    int		dev_no;		/* The serial# of the device (negative if random) */
    /* The following are NULL terminated strings (may have trailing spaces) */
    char	proto_rev;
    char	module_rev[8];
    char	vendor_nm[9];
    char	module_nm[9];

    char	data[DTI_ID_MSG_SIZE]; /* An exact copy of the data buffer */
};

struct dti_dev_managers
{
    /* probe is a pointer to a function that returns a pointer to a function
       that returns an integer.  probe is called given a pointer to a dti_devices
       entry containing information about a device that has become available (along
       with a priority counter).  probe is then expected to return NULL if it
       doesn't recognize the device (considering the priority), or it returns
       the address of a function to which it wants data received from the device
       to be passed. */

    int		(*(*probe)())();
};

struct dti_keymaps
{
    char	*keymap_name;
    u_char	*keymap;
};

struct dti_stats
{
    u_long	bytes_in;	/* Total number of bytes in */
    u_long	bytes_out;	/* Total number of bytes out */
    u_long	messages_in;	/* Total messages */
    u_long	messages_out;	/* Total messages */
    u_long	interrupts;	/* Total number of interrupts */
    u_long	key_events;	/* Total number of keyboard events */
    u_long	mouse_events;	/* Total number of mouse events */
    u_long	no_ibuffs;	/* Error - no input buffers available */
    u_long	no_buffs;	/* General error - DTI_GETBUF failed */
    u_long	unknown_addr;	/* Messages from unknown/unexpected addresses */
    u_long	unexpect_op;	/* Unexpected op code */
    u_long	bad_device;	/* How many self-test error reports have we had? */
    u_long	stop_on_rcv;	/* A stop was received on an ODB message */
    u_long	i_overflow;	/* An input overflow has occured on the I2C h/w */
    u_long	cksum_err;	/* A message with a bad checksum has appeared */
    u_long	arb_loss;	/* An arbitration loss has occured */
    u_long	nack;		/* A byte was not acknowledged during transmit */
    u_long	timeout;	/* An I2C timeout has occured on rcv or xmt */
    u_long	short_message;	/* A RCV came in & we didn't have a decent message */
    u_long	polls_due_to_err; /* How many times did we cause devs to be
				     polled because we received an error? */
    u_long	hard_resets;	/* How many times have we reset the I2C? */
    u_long	feedback_overflow; /* How many times has the feedback queue overflowed */
};

#endif
