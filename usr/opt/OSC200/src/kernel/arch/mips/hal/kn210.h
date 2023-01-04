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
 *	@(#)$RCSfile: kn210.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:13:47 $
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
 * derived from kn210.h	2.1	(ULTRIX)	10/12/89       
 */

/*
 * Revision History:
 * 16-Jan-89	Kong
 *	Created the file.
 */

/* Program Interval timer bit definition */
#define	TCR_RUN		0x00000001
#define	TCR_XFR		0x00000010
#define	TCR_INT		0x00000080
#define	TCR_ERR		0x80000000
#define	TCR_STP		0x00000004


/* MIPs Kseg 1 address of 10mS timer control/status register */
#define TCSR	PHYS_TO_K1(0x10084010)
#define	TCSR_IE		0x00000040	/* Enable timer interrupts */

/* MIPs Kseg 1 address of error interrupt status register */
#define	ISR	PHYS_TO_K1(0x10084000)
#define	ISR_FPU	0x00000008		/* FPU error interrupt 	*/
#define	ISR_PWF 0x00000004		/* Power fail interrupt	*/
#define	ISR_CERR 0x00000002		/* CQBIC or CMCTL error	*/
#define	ISR_WEAR 0x00000001		/* Write error interrupt*/

/* MIPs Kseg 1 address of interrupt vector read registers */
#define	VRR0	PHYS_TO_K1(0x16000050)	/* IRQ0	*/
#define	VRR1	PHYS_TO_K1(0x16000054)	/* IRQ1	*/
#define	VRR2	PHYS_TO_K1(0x16000058)	/* IRQ2	*/
#define	VRR3	PHYS_TO_K1(0x1600005c)	/* IRQ3	*/

#define	KN210LANCE_ADDR 0x10084400	/* physical addr of lance registers */
#define	KN210SSC_ADDR	0x10140000	/* physical addr of SSC reg set	    */
#define	LANCE_OFFSET	0xd4		/* SCB offset for network interrupts*/
#define	KN210QBUSREG	0x10087800	/* phys addr of Qbus map reg - 0x800*/
#define KN210MSIREG_ADDR 0x10084600	/* physical addr of MSI registers   */
#define	KN210SIIBUF_ADDR 0x10100000	/* physical addr of MSI buffer RAM  */
#define	KN210QMAPBASEREG 0x10080010	/* phys addr of QBus map base reg   */
#define KN210DSER	PHYS_TO_K1(0x10080004)
#define KN210WEAR	PHYS_TO_K1(0x17000000)
#define	KN210QBEAR	PHYS_TO_K1(0x10080008)
#define	KN210DEAR	PHYS_TO_K1(0x1008000c)
#define KN210CBTCR	PHYS_TO_K1(0x10140020)

/* Main memory csrs */
struct memcsr {
	/* memcsr0 through memcsr15 are memory configuration registers */
	unsigned	memcsr0;
	unsigned	memcsr1;
	unsigned	memcsr2;
	unsigned	memcsr3;
	unsigned	memcsr4;
	unsigned	memcsr5;
	unsigned	memcsr6;
	unsigned	memcsr7;
	unsigned	memcsr8;
	unsigned	memcsr9;
	unsigned	memcsr10;
	unsigned	memcsr11;
	unsigned	memcsr12;
	unsigned	memcsr13;
	unsigned	memcsr14;
	unsigned	memcsr15;
	/* memcsr16 is memory error status register */
	unsigned	memcsr16;
	/* memcsr17 is memory control and diagnostic status register */
	unsigned	memcsr17;
};
#define MEMCSR	PHYS_TO_K1(0x10080100)	/* Kseg 1 addr of memory csrs */

/*
 * Bits in memcsr0-15
 */
#define MEM_BNKENBLE	0x80000000	/* <31> Bank Enable */
#define	MEM_BNKUSAGE	0x00000003	/* <1:0> Bank Usage */
