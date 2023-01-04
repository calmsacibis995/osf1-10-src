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
 *	@(#)$RCSfile: qdregs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:01 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
 *  AST Asynch Cluster Adapter Driver
 *  Copyright Ing. C. Olivetti & S.p.A. 1989
 *  All rights reserved.
 *
 */


/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/


#define MAXPORTS 8		/* max ports (4 boards) */

#define TURNON 1		/* modem enable */
#define TURNOFF 0		/* modem disable */
#define O_LOOPBACK 0x4000	/* special open-loopback mode */
#define	PROBE	0xA5		/* used for presence check */

/* uart register definitions */
#define RBR 0		/* receive buffer */
#define THR 0		/* xmit holding */
#define IER 1		/* interrupt enable */
#define IIR 2		/* interrupt ident */
#define LCR 3		/* line control */
#define MCR 4		/* modem control */
#define LSR 5		/* line status */
#define MSR 6		/* modem status */
#define SCR 7		/* scratch */
#define DLL 0		/* divisor latch low 8 bits */
#define DLM 1		/* divisor latch high 8 bits */

/* IER register bits */
#define ERBFI 0x1	/* enable receive */
#define ETBEI 0x2	/* enable xmit */
#define ELSI 0x4	/* enable line status */
#define EDSSI 0x8	/* enable modem status */

/* LCR register bits */
#define WLS0 0x1	/* word length select 0 */
#define WLS1 0x2	/* word length select 1 */
#define STB 0x4		/* 2 stop bits */
#define PEN 0x8		/* parity enable */
#define EPS 0x10	/* even parity select */
#define STICK 0x20	/* stick parity */
#define BREAK 0x40	/* set break */
#define DLAB 0x80	/* divisor latch access */

/* MCR register bits */
#define DTR 0x1		/* modem DTR */
#define RTS 0x2		/* modem RTS */
#define OUT1 0x4	/* disable interrupts */
#define OUT2 0x8	/* enable compatible interrupts (port 0 and 1 only) */
#define LOOP 0x10	/* enable loop back mode */

/* LSR register bits */
#define DR 0x1		/* receive data ready */
#define OE 0x2		/* overrun error */
#define PE 0x4		/* parity error */
#define FE 0x8		/* framing error */
#define BI 0x10		/* break interrupt */
#define THRE 0x20	/* xmit holding register empty */
#define TEMT 0x40	/* xmit empty */

/* MSR register bits */
#define DCTS 0x1	/* delta modem CTS */
#define DDSR 0x2	/* delta modem DSR */
#define TERI 0x4	/* trailing edge ring detector */
#define DDCD 0x8	/* delta modem DCD */
#define CTS 0x10	/* modem CTS */
#define DSR 0x20	/* modem DSR */
#define RI 0x40		/* modem RI */
#define DCD 0x80	/* modem DCD */

/* character configurations */
#define CBITS5 0x0		/* 5 bits */
#define CBITS6 WLS0		/* 6 bits */
#define CBITS7 WLS1		/* 7 bits */
#define CBITS8 (WLS0 | WlS1)	/* 8 bits */

#define STHRERR 0x0e
#define CTHRERR 0x7000

/* baud rate table of divisor latch values */
static int a8_speeds[] = {
	/* B0		*/	0,
	/* B50		*/	2304,
	/* B75		*/	1536,
	/* B110		*/	1047,
	/* B134		*/	857,
	/* B150		*/	768,
	/* B200		*/	0,
	/* B300		*/	384,
	/* B600		*/	192,
	/* B1200	*/	96,
	/* B1800	*/	64,
	/* B2400	*/	48,
	/* B4800	*/	24,
	/* B9600	*/	12,
	/* EXTA 19200	*/	6,
	/* EXTB 2000	*/	58
};

