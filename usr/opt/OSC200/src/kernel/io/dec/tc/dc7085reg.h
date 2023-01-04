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
 *	@(#)$RCSfile: dc7085reg.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1993/07/14 18:17:29 $
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
 * derived from dc7085reg.h	4.2	(ULTRIX)	8/9/90
 */

/*
 * dc7085reg.h
 *
 * DC7085 SLU console driver
 *
 * Modification history
 *
 *  10-Oct-1991 - Mike Larson
 *	Fixed DS5100_DC_L2_XMIT definition.
 *
 *  4-Jul-1990 - Randall P. Brown
 *	Removed definitions for LK201 keyboard.
 *
 *  8-Dec-1989 - Randall P. Brown
 *	Removed the definitions for the tablet buttons.
 *
 * 11-Jul-1988 - Randall P. Brown
 *	Removed the dc_tty struct from the softc struct.  The dc_tty struct
 *	is declared by itself in dc7085.c
 *
 * 16-Dec-1988 - Randall P. Brown
 *	Added the struct for pdma, and included the pdma struct in the
 * 	dc_softc struct.
 *
 * 17-Nov-1988 - Randall P. Brown
 *	Cleaned up file so that names are consistent with spec.
 *
 *  7-Jul-1988 - rsp (Ricky Palmer)
 *	Created file. Contents based on ssreg.h file.
 *
 */

#ifndef _DC_REG_H_
#define _DC_REG_H_

/* Serial line registers */
struct dc_reg {
	u_short dz_csr;			/* SLU control status register	*/
	u_short pad1[3];
	union {				/* SLU read buffer/line param.	*/
		u_short dz_rbuf;
		u_short dz_lpr;
	} dz_rbuf_lpr;
	u_short pad2[3];
	u_short dz_tcr;			/* SLU transmitter control reg. */
	u_short pad3[3];
	union {				/* SLU modem status/txmt reg.	*/
	    u_short	dz_msr;		/* read only modem status reg.	*/
	    u_short	dz_tbuf_w;	/* bottom half of TDR 		*/
	    u_long	dz_tbuf_l;	/* on 3max we need to use	*/
	} dz_msr_tdr;			/* full word write to this reg. */
	u_char 	pad4;			/* due to a R3000 bug.          */
	u_char	dz_brk;			/* on 3max the upper 8 bits of 	*/
	                                /* the TDR are accessed at this */
	                                /* address			*/
};

#define dccsr		sc_regs->dz_csr
#define dcrbuf		sc_regs->dz_rbuf_lpr.dz_rbuf
#define dclpr		sc_regs->dz_rbuf_lpr.dz_lpr
#define dctcr		sc_regs->dz_tcr
#define dcmsr		sc_regs->dz_msr_tdr.dz_msr
#define dcbrk_tbuf	sc_regs->dz_msr_tdr.dz_tbuf_l
#define dctbuf		sc_regs->dz_msr_tdr.dz_tbuf_w  	/* transmit half word */
#define dcbrk		sc_regs->dz_brk

/* CPU specific configuration */
#define CONSOLEMAJOR		0  /* Console is on unit 0 	           */
#define NDCLINE 		4  /* Number of lines per DC chip          */
#define CONSOLE_UNIT		0  /* Console is on unit 0 	           */
#define DS5100_MAX_NDC 		3  /* mipsmate can have 3 DC7085's at most */
#define DS5100_CONSOLE_LINE  	0  /* mipsmate console is on line 0 of DC0 */
#define WS_DC_DIAG_LINE	 	3  /* non graphic diag. console for ws 	   */

/* Control status register definitions (dccsr) */
#define DC_OFF		0x00		/* Modem control off		*/
#define DC_MAINT	0x08		/* Maintenance			*/
#define DC_CLR		0x10		/* Reset dc7085 chip		*/
#define DC_MSE		0x20		/* Master Scan Enable		*/
#define DC_RIE		0x40		/* Receive IE */
#define DC_RDONE	0x80		/* Receiver done		*/
#define DC_TIE		0x4000		/* Trasmit IE */
#define DC_TRDY		0x8000		/* Transmit ready		*/

/* Line parameter register definitions (dclpr) */
#define BITS5		0x00		/* 5 bit char width		*/
#define BITS6		0x08		/* 6 bit char width		*/
#define BITS7		0x10		/* 7 bit char width		*/
#define BITS8		0x18		/* 8 bit char width		*/
#define TWOSB		0x20		/* two stop bits		*/
#define PENABLE		0x40		/* parity enable		*/
#define OPAR		0x80		/* odd parity			*/
#define DC_B50		0x000		/* 50 BPS speed			*/
#define DC_B75		0x100		/* 75 BPS speed			*/
#define DC_B110		0x200		/* 110 BPS speed		*/
#define DC_B134_5	0x300		/* 134.5 BPS speed		*/
#define DC_B150		0x400		/* 150 BPS speed		*/
#define DC_B300		0x500		/* 300 BPS speed		*/
#define DC_B600		0x600		/* 600 BPS speed		*/
#define DC_B1200	0x700		/* 1200 BPS speed		*/
#define DC_B1800	0x800		/* 1800 BPS speed		*/
#define DC_B2000	0x900		/* 2000 BPS speed		*/
#define DC_B2400	0xa00		/* 2400 BPS speed		*/
#define DC_B3600	0xb00		/* 3600 BPS speed		*/
#define DC_B4800	0xc00		/* 4800 BPS speed		*/
#define DC_B7200	0xd00		/* 7200 BPS speed		*/
#define DC_B9600	0xe00		/* 9600 BPS speed		*/
#define DC_B19200	0xf00		/* 19200 BPS speed		*/
#define DC_B38400	0xf00		/* 38400 BPS speed - see LED2	*/
#define DC_RE		0x1000		/* Receive enable		*/

/* Transmit Control Register (dctcr) */
#define DC_TCR_EN_0	0x1		/* enable transmit on line 0	*/
#define DC_TCR_EN_1	0x2		/* enable transmit on line 1	*/
#define DC_TCR_EN_2	0x4		/* enable transmit on line 2	*/
#define DC_TCR_EN_3	0x8		/* enable transmit on line 3	*/

/* CPU specific Transmit Control Register definitions */
#define DS3100_DC_L2_DTR	0x0400	/* pmax DTR on line 2 		*/	
#define DS3100_DC_L2_DSR	0x0200	/* pmax DSR on line 2 		*/	
#define DS3100_DC_L2_XMIT	DS3100_DC_L2_DSR  /* All modem leads ready */

#define DS3100_DC_L3_DTR	0x0800	/* pmax DTR on line 3 		*/	
#define DS3100_DC_L3_DSR	0x0001	/* pmax DSR on line 3 		*/	
#define DS3100_DC_L3_XMIT	DS3100_DC_L3_DSR  /* All modem leads ready */	

#define DS5000_DC_L2_DTR	0x0400	/* 3max DTR on line 2 		*/	
#define DS5000_DC_L2_RTS	0x0800	/* 3max RTS on line 2 		*/	
#define DS5000_DC_L3_DTR	0x0100	/* 3max DTR on line 3 		*/	
#define DS5000_DC_L3_RTS	0x0200	/* 3max RTS on line 3 		*/	

#define DS5100_DC_L2_SS		0x0200	/* mipsmate SS on line 2 	*/	
#define DS5100_DC_L2_DTR	0x0400	/* mipsmate DTR on line 2 	*/	
#define DS5100_DC_L2_RTS	0x0800	/* mipsmate RTS on line 2 	*/	

/* CPU specific Modem Status Register definitions */
#define DS3100_DC_L2_SS		0x0200	/* mipsmate ss on line 2 	*/	

#define DS5000_DC_L2_CTS	0x0100	/* 3max CTS on line 2 		*/	
#define DS5000_DC_L2_DSR	0x0200	/* 3max DSR on line 2 		*/	
#define DS5000_DC_L2_CD		0x0400	/* 3max CD on line 2 		*/	
#define DS5000_DC_L2_RI		0x0800	/* 3max RI on line 2 		*/	
#define DS5000_DC_L2_XMIT	(DS5000_DC_L2_CTS| DS5000_DC_L2_DSR| DS5000_DC_L2_CD)					/* All modem leads ready	*/

#define DS5000_DC_L3_CTS	0x0001	/* 3max CTS on line 3 		*/	
#define DS5000_DC_L3_DSR	0x0002	/* 3max DSR on line 3 		*/	
#define DS5000_DC_L3_CD		0x0004	/* 3max CD on line 3 		*/	
#define DS5000_DC_L3_RI		0x0008	/* 3max RI on line 3 		*/	
#define DS5000_DC_L3_XMIT	(DS5000_DC_L3_CTS| DS5000_DC_L3_DSR| DS5000_DC_L3_CD)					/* All modem leads ready	*/

#define DS5100_DC_L2_CTS	0x0001	/* mipsmate CTS on line 2 	*/	
#define DS5100_DC_L2_DSR	0x0002	/* mipsmate DSR on line 2 	*/	
#define DS5100_DC_L2_CD		0x0004	/* mipsmate CD on line 2 	*/	
#define DS5100_DC_L2_RI		0x0008	/* mipsmate CD on line 2 	*/	
#define DS5100_DC_L2_SI		0x0080	/* mipsmate CD on line 2 	*/	
#define DS5100_DC_L2_XMIT	(DS5100_DC_L2_CTS| DS5100_DC_L2_DSR| DS5100_DC_L2_CD)					/* All modem leads ready	*/

/* Receiver buffer register definitions (dcrbuf) */
#define DC_PE		0x1000		/* Parity error			*/
#define DC_FE		0x2000		/* Framing error		*/
#define DC_DO		0x4000		/* Data overrun error		*/
#define DC_DVAL		0x8000		/* Receive buffer data valid	*/

/* Line control status definitions (dclcs) */
#define DC_SR		0x08		/* Secondary Receive		*/
#define DC_CTS		0x10		/* Clear To Send		*/
#define DC_CD		0x20		/* Carrier Detect		*/
#define DC_RI		0x40		/* Ring Indicate		*/
#define DC_DSR		0x80		/* Data Set Ready		*/
#define DC_LE		0x100		/* Line Enable			*/
#define DC_DTR		0x200		/* Data Terminal Ready		*/
#define DC_BRK		0x400		/* Break			*/
#define DC_ST		0x800		/* Secondary Transmit		*/
#define DC_RTS		0x1000		/* Request To Send		*/

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

/* Line Prameter Register bits */
#define SER_KBD      000000
#define SER_POINTER  000001
#define SER_COMLINE  000002
#define SER_PRINTER  000003
#define SER_CHARW    000030
#define SER_STOP     000040
#define SER_PARENB   000100
#define SER_ODDPAR   000200
#define SER_SPEED    006000
#define SER_RXENAB   010000

/* CPU specific base address */
#define DS3100_DC_BASE   (struct dc_reg *)(PHYS_TO_K1(0x1c000000))
#define DS5000_DC_BASE   (struct dc_reg *)(PHYS_TO_K1(0x1fe00000))
#define DS5100_DC0_BASE  (struct dc_reg *)(PHYS_TO_K1(0x1c000000))
#define DS5100_DC1_BASE  (struct dc_reg *)(PHYS_TO_K1(0x15000000))
#define DS5100_DC2_BASE  (struct dc_reg *)(PHYS_TO_K1(0x15200000))

/* Pseudo DMA structure */
struct dcpdma {
    	char *p_mem;
	char *p_end;
};

/* DEV_SIZE is defined in devio.h in ULTRIX */
#ifndef DEV_SIZE
#define DEV_SIZE        0x08            /* Eight bytes */
#endif	/* DEV_SIZE */

/* Driver and data specific structure */
struct	dc_softc {
	struct	dcpdma dc_pdma[NDCLINE];/* peudo dma structure		*/
	volatile struct dc_reg *sc_regs; /* PMAX SLU registers		*/
	long	sc_flags[NDCLINE];	/* Flags (one per line)		*/
	long	sc_category_flags[NDCLINE]; /* Category flags (one per line)*/
	u_long	sc_softcnt[NDCLINE];	/* Soft error count total	*/
	u_long	sc_hardcnt[NDCLINE];	/* Hard error count total	*/
	char	sc_device[DEV_SIZE][NDCLINE]; /* Device type string	*/
};

/* Baud rate support status */
struct baud_support {
	u_short	baud_param;	/* How baud rate is represented by device */
	u_char  baud_support;	/* Set if baud rate is supported. */
};

#endif
