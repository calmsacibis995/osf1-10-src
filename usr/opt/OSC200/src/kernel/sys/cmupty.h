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
 *	@(#)$RCSfile: cmupty.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:57:02 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#ifndef	_SYS_CMUPTY_H_
#define _SYS_CMUPTY_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ttyloc.h>
#include <kern/queue.h>

#define NPTYL	5		/* maximum of 16 (4-bit mask) */

#define OPENMSG	  1
#define CLOSEMSG  2
#define STATEMSG  3
#define IOCTLMSG  4
#define WRITEMSG  5
#define CCMSG	  6
#define ATTACHMSG 7

#define OPENREPLY  50
#define CLOSEREPLY 51
#define IOCTLREPLY 52

#define IOCTLDATA  75
#define READDATA   76

struct ptymsg
{
    unsigned char pt_msg;		/* message identifier */
    unsigned char pt_arg;		/* message argument */
    unsigned char pt_aux;		/* auxiliary message argument */
    unsigned char pt_line;		/* line generating message */
};

struct ptyreply
{
    unsigned char pt_reply;		/* reply identifier */
    unsigned char pt_error;		/* reply error code */
    unsigned char pt_len;		/* reply non-data length */
    unsigned char pt_line;		/* line to receive reply */
};


#define PTYLBITS	(4)		/* bits in ioctl line */
#define PTYLMASK	(017)		/* PTYLBITS of line mask */

/*
 *  Note: The PTYLBITS definition above limits the number of PTY
 *  ioctl calls to 16 (the remaining 4 bits in the low byte of
 *  the code).
 */
#define PIOCXIOC	_IO  ('P', ( 0<<PTYLBITS))
#define PIOCSEOF	_IO  ('P', ( 1<<PTYLBITS))
#define PIOCSSIG	_IOW ('P', ( 3<<PTYLBITS), int)
#define PIOCCONN	_IOR ('P', ( 4<<PTYLBITS), struct ptymsg)
#define PIOCSIM		_IO  ('P', ( 5<<PTYLBITS))	/* obsolete */
#define PIOCNOSIM	_IO  ('P', ( 6<<PTYLBITS))	/* obsolete */
#define PIOCCCMSG	_IO  ('P', ( 7<<PTYLBITS))	/* obsolete */
#define PIOCNOCCMSG	_IO  ('P', ( 8<<PTYLBITS))	/* obsolete */
#define PIOCENBS	_IOW ('P', ( 9<<PTYLBITS), int)
#define PIOCMBIS	_IOW ('P', (10<<PTYLBITS), int)
#define PIOCMBIC	_IOW ('P', (11<<PTYLBITS), int)
#define PIOCSLOC	_IOW ('P', (12<<PTYLBITS), struct ttyloc)
#define PIOCMGET	_IOR ('P', (13<<PTYLBITS), int)
#define PIOCDGET	_IOR ('P', (14<<PTYLBITS), int)

#ifdef	_KERNEL
typedef struct sgttyb	ptioctlbuf_t;

struct ptyctrl
{
    int pt_state;			/* pty control state bits */
    int pt_ostate;			/* previous tty state bits */
    int pt_orcc;			/* previous raw character count */
    char pt_openbuf;			/* open reply buffer */
    char pt_closebuf;			/* close reply buffer */
    short pt_buflen;			/* ioctl reply length */
    int pt_cmdbuf;			/* ioctl cmd word (must be here) */
    ptioctlbuf_t pt_ioctlbuf;		/* ioctl reply buffer */
    struct ptyctrl *pt_cpty;		/* master multiplex pty */
    struct ptyctrl *pt_ctrl[NPTYL];	/* multiplex line pointers */
    struct proc *pt_sigp;		/* signal process pointer */
    u_short pt_pid;			/* PID of signal process */
    u_short pt_sign;			/* signal number to send */
    unsigned char pt_next;		/* next line for multiplex read */
    unsigned char pt_high;		/* highest line assigned */
    dev_t pt_mdev;			/* device number of pty */
    dev_t pt_ldev;			/* device number of control file */
    struct queue_entry pt_selq;		/* Queue of waiting threads */
};

#define OPENFLG	    0x1			/* waiting for open reply */
#define OPENINPROG  0x2			/* open in progress */
#define CLOSEFLG    0x4			/* waiting for close reply */
#define CLOSEINPROG 0x8			/* close in progress */
#define IOCTLFLG    0x10		/* waiting for ioctl reply */
#define IOCTLINPROG 0x20		/* ioctl in progress */
#define CCFLG	    0x40		/* character count */
#define ATTACHFLG   0x80		/* newly attached */
#endif	/* _KERNEL */

#define PTYNOIOCTL  0x100000		/* disable all ioctl() forwarding */
#define PTYDETHUP   0x200000		/* send HANGUP on detach condition */
#define PTYLOGGEDIN 0x400000		/* application terminal "logged-in" */
#define PTYDETACHED 0x800000		/* application terminal "detached" */
#define PTYNEWSIG   0x1000000		/* return 1 byte on new signals */
#define PTYHOLDSIG  0x2000000		/* keep signal enabled after sending */
#define PTYNOBLOCK  0x4000000		/* do not block on control reads */
#define PTYCCMSG    0x8000000		/* generate character count messages */
#define PTYSIM	    0x10000000		/* force read to fill buffer */

#ifdef	_KERNEL
#define PTYEOF	    0x20000000		/* force read to return EOF */
#define OUTPUTWAIT  0x40000000		/* pty output wait */
#define PTYINUSE    0x80000000		/* pty control in use */

/*
 *  State (mode) bit masks.
 *
 *  RW (read/write) can be examined and changed by the control process.
 *  RO (read-only) can only be examined by the control process.
 */
#define PTYRWMODES (PTYNEWSIG|PTYHOLDSIG|PTYNOBLOCK|PTYCCMSG|PTYSIM|PTYDETHUP|PTYNOIOCTL)
#define PTYROMODES (PTYLOGGEDIN)


/*
 *  Read will return control or data information.
 */

#define ptywread(cp, tp)	\
	(ptywxread(cp, tp) || ptywdread(tp))

/*
 *  Read will return control information.
 */

#define ptywxread(cp, tp)	\
	((cp)->pt_state&(ATTACHFLG|OPENFLG|IOCTLFLG|CLOSEFLG|CCFLG) || \
	 ((tp)->t_state != (cp)->pt_ostate))

/*
 *  Read will return data bytes.
 */
#define ptywdread(tp)		\
	(((tp)->t_outq.c_cc > 0) && \
	 !((tp)->t_state&(TS_TIMEOUT|TS_BUSY|TS_TTSTOP)))

#endif	/* _KERNEL */
#endif	/* _SYS_CMUPTY_H_ */
