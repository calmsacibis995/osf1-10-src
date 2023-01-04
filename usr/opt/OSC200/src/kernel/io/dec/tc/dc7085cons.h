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
 *	@(#)$RCSfile: dc7085cons.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1993/07/14 18:17:24 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * derived from dc7085cons.h	4.5	(ULTRIX)	9/25/88
 */

/*
 * dc7085cons.h
 *
 * DC7085 SLU console driver
 *
 * Modification history
 *
 *  7-Jul-1988 - rsp (Ricky Palmer)
 *	Created file. Contents based on ssreg.h file.
 *
 */
#ifndef _DC_CONS_H_
#define _DC_CONS_H_

#define sscsr		sc_regs->sercsr
#define ssdtr		sc_regs->sertcr.c[1]
#define ssmsr		sc_regs->sermsr_tdr.c[1]
#define ssbrk		ssmsr
#define ssrbuf		sc_regs->serrbuf_lpr
#define sslpr		sc_regs->serrbuf_lpr
#define sstcr		sc_regs->sertcr.c[0]
#define sswtcr		sc_regs->sertcr.w
#define sstbuf		sc_regs->sermsr_tdr.c[0]
#define sswtbuf		sc_regs->sermsr_tdr.w

#define CONSOLEMAJOR	0
#define SSMAJOR 0
#define NSSLINE 4

/* Interrupt controller register bits */
#define SINT_SR 0200			/* Serial line recv/silo full	*/
#define SINT_ST 0100			/* Serial line transmitter done */

/* Control status register definitions (sscsr) */
#define SS_OFF		0x00		/* Modem control off		*/
#define SS_CLR		0x10		/* Reset ss			*/
#define SS_MSE		0x20		/* Master Scan Enable		*/
#define SS_RDONE	0x80		/* Receiver done		*/
#define SS_SAE		0x1000		/* Silo Alarm Enable		*/
#define SS_SA		0x2000		/* Silo Alarm			*/
#define SS_TIE		0x4000		/* Trasmit IE */
#define SS_RIE		0x0040		/* Receive IE */
#define SS_TRDY		0x8000		/* Transmit ready		*/
#define SS_ON		SS_DTR		/* Modem control on		*/

/* Line parameter register definitions (sslpr) */
#define BITS5		0x00		/* 5 bit char width		*/
#define BITS6		0x08		/* 6 bit char width		*/
#define BITS7		0x10		/* 7 bit char width		*/
#define BITS8		0x18		/* 8 bit char width		*/
#define TWOSB		0x20		/* two stop bits		*/
#define PENABLE		0x40		/* parity enable		*/
#define OPAR		0x80		/* odd parity			*/
#define SS_B4800	0xc00		/* 4800 BPS speed		*/
#define SS_B9600	0xe00		/* 9600 BPS speed		*/
#define SS_RE		0x1000		/* Receive enable		*/

#define	SER_KBD      000000
#define	SER_POINTER  000001
#define	SER_COMLINE  000002
#define	SER_PRINTER  000003
#define	SER_CHARW    000030	
#define	SER_STOP     000040
#define	SER_PARENB   000100
#define	SER_ODDPAR   000200
#define	SER_SPEED    006000
#define	SER_RXENAB   010000

/* Receiver buffer register definitions (ssrbuf) */
#define SS_PE		0x1000		/* Parity error			*/
#define SS_FE		0x2000		/* Framing error		*/
#define SS_DO		0x4000		/* Data overrun error		*/
#define SS_DVAL		0x8000		/* Receive buffer data valid	*/

/* Line control status definitions (sslcs) */
#define SS_SR		0x08		/* Secondary Receive		*/
#define SS_CTS		0x10		/* Clear To Send		*/
#define SS_CD		0x20		/* Carrier Detect		*/
#define SS_RI		0x40		/* Ring Indicate		*/
#define SS_DSR		0x80		/* Data Set Ready		*/
#define SS_LE		0x100		/* Line Enable			*/
#define SS_DTR		0x200		/* Data Terminal Ready		*/
#define SS_BRK		0x400		/* Break			*/
#define SS_ST		0x800		/* Secondary Transmit		*/
#define SS_RTS		0x1000		/* Request To Send		*/

/* DM lsr definitions */
#define SML_LE		0x01		/* Line enable			*/
#define SML_DTR		0x02		/* Data terminal ready		*/
#define SML_RTS		0x04		/* Request to send		*/
#define SML_ST		0x08		/* Secondary transmit		*/
#define SML_SR		0x10		/* Secondary receive		*/
#define SML_CTS		0x20		/* Clear to send		*/
#define SML_CAR		0x40		/* Carrier detect		*/
#define SML_RNG		0x80		/* Ring				*/
#define SML_DSR		0x100		/* Data set ready, not DM bit	*/

/* ssdtr bits */
#define SS_RRTS 0x1			/* REAL request to send bit	*/
#define SS_RDTR 0x4			/* REAL data terminal ready	*/

/* ssmsr bits */
#define SS_RCTS 0x1			/* REAL clear to send bit	*/
#define SS_RDSR 0x2			/* REAL data set ready bit	*/
#define SS_RCD	0x4			/* REAL carrier detect bit	*/
#define SS_XMIT (SS_RDSR|SS_RCD|SS_RCTS)/* Ready to transmit & rec.	*/
#define SS_NODSR (SS_RCD|SS_RCTS)	/* Instead of SS_XMIT		*/

#ifdef _KERNEL
/* Serial line registers */
struct cn_reg {
	u_short sercsr;			/* SLU control status register	*/
	u_short pad1[3];
	u_short serrbuf_lpr;		/* SLU read buffer/line param.	*/
	u_short pad2[3];
	union {
		u_char	c[2];
		u_short w;
	} sertcr;			/* SLU transmitter control reg. */
	u_short pad3[3];
	union	{
		u_char	c[2];
		u_short w;
	} sermsr_tdr;			/* SLU modem status/txmt reg.	*/
};

/* Driver and data specific structure */
/***** ss_tty must be first *****/
struct	ss_softc {
	struct	tty ss_tty[NSSLINE];	/* Tty structure		*/
	volatile struct cn_reg *sc_regs; /* PMAX SLU registers		*/
	long	sc_flags[NSSLINE];	/* Flags (one per line)		*/
	u_long	sc_softcnt[NSSLINE];	/* Soft error count total	*/
	u_long	sc_hardcnt[NSSLINE];	/* Hard error count total	*/
};

#ifndef	ASSEMBLER
extern struct ss_softc cons;
#endif	/* ASSEMBLER */
#endif	/* _KERNEL */


#endif
