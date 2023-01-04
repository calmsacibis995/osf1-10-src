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
 *	@(#)$RCSfile: if_wd8003.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:56 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

/*
 * Western Digital Mach Ethernet driver
 * Copyright (c) 1990 OSF Research Institute 
 */
/*
  Copyright 1990 by Open Software Foundation,
Cambridge, MA.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both the copyright notice and this permission notice appear in
supporting documentation, and that the name of OSF or Open Software
Foundation not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

  OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/********************************************/
/* Defines for the NIC 8390 Lan Controller  */
/********************************************/


/*--- 8390 Registers ---*/
#define OFF_8390	0x10	/* offset of the 8390 chip */

/*--  page 0, rd --*/
#define CR	OFF_8390+0x00		/* Command Register	*/
#define CLDA0	OFF_8390+0x01		/* Current Local DMA Address 0 */
#define CLDA1	OFF_8390+0x02		/* Current Local DMA Address 1 */
#define BNRY	OFF_8390+0x03		/* Boundary Pointer */
#define TSR	OFF_8390+0x04		/* Transmit Status Register */
#define NCR	OFF_8390+0x05		/* Number of Collisions Register */
#define FIFO	OFF_8390+0x06		/* FIFO */
#define ISR	OFF_8390+0x07		/* Interrupt Status Register */
#define CRDA0	OFF_8390+0x08		/* Current Remote DMA Address 0 */
#define CRDA1	OFF_8390+0x09		/* Current Remote DMA Address 1 */
/* OFF_8390+0x0A is reserved */
/* OFF_8390+0x0B is reserved */
#define RSR	OFF_8390+0x0C		/* Receive Status Register */
#define CNTR0	OFF_8390+0x0D		/* Frame Alignment Errors */
#define CNTR1	OFF_8390+0x0E		/* CRC Errors */
#define CNTR2	OFF_8390+0x0F		/* Missed Packet Errors */

/*-- page 0, wr --*/
/*	CR	OFF_8390+0x00		   Command Register	*/
#define PSTART	OFF_8390+0x01		/* Page Start Register */
#define PSTOP	OFF_8390+0x02		/* Page Stop Register */
#define	BNDY	OFF_8390+0x03		/* Boundary Pointer	*/
#define TPSR	OFF_8390+0x04		/* Transmit Page Start Register */
#define TBCR0	OFF_8390+0x05		/* Transmit Byte Count Register 0*/
#define TBCR1	OFF_8390+0x06		/* Transmit Byte Count Register 1*/
/*	ISR	OFF_8390+0x07		   Interrupt Status Register	*/
#define RSAR0	OFF_8390+0x08		/* Remote Start Address Register 0 */
#define RSAR1	OFF_8390+0x09		/* Remote Start Address Register 1 */
#define RBCR0	OFF_8390+0x0A		/* Remote Byte Count Register 0 */
#define RBCR1	OFF_8390+0x0B		/* Remote Byte Count Register 1 */
#define RCR	OFF_8390+0x0C		/* Receive Configuration Register */
#define TCR	OFF_8390+0x0D		/* Transmit Configuration Register */
#define DCR	OFF_8390+0x0E		/* Data Configuration Register */
#define IMR	OFF_8390+0x0F		/* Interrupt Mask Register */

/*-- page 1, rd and wr */
/*	CR	OFF_8390+0x00		   Control Register	*/
#define PAR0	OFF_8390+0x01		/* Physical Address Register 0 */
#define PAR1	OFF_8390+0x02		/*			     1 */
#define PAR2	OFF_8390+0x03		/*			     2 */
#define PAR3	OFF_8390+0x04		/*			     3 */
#define PAR4	OFF_8390+0x05		/*			     4 */
#define PAR5	OFF_8390+0x06		/*			     5 */
#define CURR	OFF_8390+0x07		/* Current Page Register */
#define	MAR0	OFF_8390+0x08		/* Multicast Address Register 0	*/
#define MAR1	OFF_8390+0x09		/* 			      1	*/
#define MAR2	OFF_8390+0x0A		/*			      2 */
#define	MAR3	OFF_8390+0x0B		/*			      3 */
#define	MAR4	OFF_8390+0x0C		/*			      4 */
#define MAR5	OFF_8390+0x0D		/*			      5 */
#define MAR6	OFF_8390+0x0E		/*			      6 */
#define MAR7	OFF_8390+0x0F		/*			      7 */

/*-- page 2, rd --*/

/*-- page 2, wr --*/

/*-- Command Register CR description */
#define STP		0x01	/* stop; software reset */
#define STA		0x02	/* start */
#define TXP		0x04	/* transmit packet */
#define	RD0		0x08
#define	RD1		0x10
#define	RD2		0x20
#define RRD		0x08	/* remote DMA command - remote read */

#define RWR		0x10	/* remote DMA command - remote write */
#define SPK		0x18	/* remote DMA command - send packet */
#define ABR		0x20	/* remote DMA command - abrt/cmplt remote DMA */

#define PS0		0x00	/* register page select - 0 */
#define PS1		0x40	/* register page select - 1 */
#define PS2		0x80	/* register page select - 2 */

#define	PS0_STA		0x22	/* page select 0 with start bit maintained */
#define	PS1_STA		0x62	/* page select 1 with start bit maintained */
#define	PS2_STA		0x0A2	/* page select 2 with start bit maintained */

/*-- Interrupt Status Register ISR description */
#define PRX		0x01	/* packet received no error */
#define PTX		0x02	/* packet transmitted no error */
#define RXE		0x04	/* receive error */
#define TXE		0x08	/* transmit error */
#define OVW		0x10	/* overwrite warning */
#define CNT		0x20	/* counter overflow */
#define RDC		0x40	/* remote DMA complete */
#define RST		0x80	/* reset status */

/*-- Interrupt Mask Register IMR description */
#define PRXE		0x01	/* packet received interrupt enable */
#define PTXE		0x02	/* packet transmitted interrupt enable */
#define RXEE		0x04	/* receive error interrupt enable */
#define TXEE		0x08	/* transmit error interrupt enable */
#define OVWE		0x10	/* overwrite warning interrupt enable */
#define CNTE		0x20	/* counter overflow interrupt enable */
#define RDCE		0x40	/* DMA complete interrupt enable */

/*-- Data Configuration Register DCR description */
#define WTS		0x01	/* word transfer select */
#define BOS		0x02	/* byte order select */
#define LAS		0x04	/* long address select */
#define BMS		0x08	/* burst DMA select */
#define AINIT		0x10	/* autoinitialize remote */

#define FTB2		0x00	/* receive FIFO threshold select - 2 bytes */
#define FTB4		0x20	/* receive FIFO threshold select - 4 bytes */
#define FTB8		0x40	/* receive FIFO threshold select - 8 bytes */
#define FTB12		0x60	/* receive FIFO threshold select - 12 bytes */

/*-- Transmit Configuration Register TCR description */
#define MCRC		0x01	/* manual crc generation */
#define LB1		0x02	/* mode 1; internal loopback LPBK=0 */
#define LB2		0x04	/* mode 2; internal loopback LPBK=1 */
#define LB3		0x06	/* mode 3; internal loopback LPBK=0 */

#define ATD		0x08	/* auto transmit disable */
#define OFST		0x10	/* collision offset enable */

/*-- Transmit Status Register TSR description --*/
#define XMT		0x01	/* packet transmitted without error */
#define COL		0x04	/* transmit collided */
#define ABT		0x08	/* transmit aborted */
#define CRS		0x10	/* carrier sense lost - xmit not aborted */
#define FU		0x20	/* FIFO underrun */
#define CDH		0x40	/* CD heartbeat */
#define OWC		0x80	/* out of window collision - xmit not aborted */

/*-- Receive Configuration Register RCR description --*/
#define SEP		0x01	/* save error packets */
#define AR		0x02	/* accept runt packet */
#define AB		0x04	/* accept broadcast */
#define AM		0x08	/* accept multicast */
#define PRO		0x10	/* promiscuous physical */
#define MON		0x20	/* monitor mode */

/*--Receive Status Register RSR description --*/
#define RCV		0x01	/* packet received intact */
#define CRC		0x02	/* CRC error */
#define FAE		0x04	/* frame alignment error */
#define FO		0x08	/* FIFO overrun */
#define MPA		0x10	/* missed packet */
#define PHY		0x20	/* physical/multicast address */
#define DIS		0x40	/* receiver disable */
#define DFR		0x80	/* deferring */




/***********************************************************/
/*  Defines for the 583 chip.                              */
/***********************************************************/


/*--- 83c583 registers ---*/
#define MSR	0x00		/* memory select register */
#define ICR	0x01		/* interface configuration register */
#define IAR	0x02		/* io address register */
#define BIO	0x03		/* bios ROM address register */
#define IRR	0x04		/* interrupt request register */
#define GP1	0x05		/* general purpose register 1 */
#define IOD	0x06		/* io data latch */
#define GP2	0x07		/* general purpose register 2 */
#define LAR	0x08		/* LAN address register	*/
#define LAR2	0x09		/*			*/
#define LAR3	0x0A		/*			*/
#define LAR4	0x0B		/*			*/
#define LAR5	0x0C		/*			*/
#define LAR6	0x0D		/*			*/
#define LAR7	0x0E		/*			*/
#define LAR8	0x0F		/* LAN address register */

/********************* Register Bit Definitions **************************/
/* MSR definitions */
/* defined: #define RST	     0x80	        1 => reset */
#define MENB	0x40		/* 1 => memory enable */
#define SA18	0x20		/* Memory enable bits	*/
#define	SA17	0x10		/*	telling where shared	*/
#define	SA16	0x08		/*	mem is to start.	*/
#define SA15	0x04		/*	Assume SA19 = 1		*/
#define SA14	0x02		/*				*/
#define	SA13	0x01		/*				*/

/* ICR definitions */
#define	STR	0x80		/* Non-volatile EEPROM store	*/
#define	RCL	0x40		/* Recall I/O Address from EEPROM */
#define	RX7	0x20		/* Recall all but I/O and LAN address */
#define RLA	0x10		/* Recall LAN Address	*/
#define	MSZ	0x08		/* Shared Memory Size	*/
#define	DMAE	0x04		/* DMA Enable	*/
#define	IOPE	0x02		/* I/O Port Enable */
/* defined #define WTS	     0x01		 Word Transfer Select */

/* IAR definitions */
#define	IA15	0x80		/* I/O Address Bits	*/
/*	.		*/
/*	.		*/
/*	.		*/
#define	IA5	0x01		/*			*/

/* BIO definitions */
#define	RS1	0x80		/* BIOS size bit 1 */
#define	RS0	0x40		/* BIOS size bit 0 */
#define	BA18	0x20		/* BIOS ROM Memory Address Bits */
#define	BA17	0x10		/*				*/
#define	BA16	0x08		/*				*/
#define	BA15	0x04		/*				*/
#define BA14	0x02		/* BIOS ROM Memory Address Bits */
#define	WINT	0x01		/* W8003 interrupt	*/

/* IRR definitions */
#define	IEN	0x80		/* Interrupt Enable	*/
#define	IR1	0x40		/* Interrupt request bit 1	*/
#define	IR0	0x20		/* Interrupt request bit 0	*/
#define	AMD	0x10		/* Alternate mode	*/
#define AINT	0x08		/* Alternate interrupt	*/
#define BW1	0x04		/* BIOS Wait State Control bit 1	*/
#define BW0	0x02		/* BIOS Wait State Control bit 0	*/
#define OWS	0x01		/* Zero Wait State Enable	*/

/* GP1 definitions */

/* IOD definitions */

/* GP2 definitions */


/*************************************************************/
/*   Shared RAM buffer definitions                           */
/*************************************************************/


/**** Western digital node bytes ****/
#define	WD_NODE_ADDR_0	0x00
#define	WD_NODE_ADDR_1	0x00
#define	WD_NODE_ADDR_2	0xC0

/**** NIC definitions ****/
#define NIC_8003_SRAM_SIZE 0x2000       /* size of shared RAM buffer */
#define	NIC_HEADER_SIZE	4		/* size of receive header */
#define	NIC_PAGE_SIZE	0x100		/* each page of rcv ring is 256 byte */

/* #define NWD8003         1 */
#define ETHER_ADDR_SIZE	6	/* size of a MAC address */

#ifndef TRUE
#define TRUE		1
#endif	TRUE

#ifdef MACH
#define	HZ		100
#endif

#define	DSF_LOCK	1
#define DSF_RUNNING	2
#define	DSF_SETADDR	3

#define MOD_ENAL 1
#define MOD_PROM 2



/*****************************************************************************
 *                                                                           *
 *   Definitions for board ID.                                               *
 *                                                                           *
 *   note: board ID should be ANDed with the STATIC_ID_MASK                  *
 *         before comparing to a specific board ID                           *
 *	   The high order 16 bits correspond to the Extra Bits which do not  *
 *         change the boards ID.                                             *
 *                                                                           *
 *   Note: not all are implemented.  Rest are here for future enhancements...*
 *                                                                           *
 *****************************************************************************/


#define	STARLAN_MEDIA		0x00000001
#define	ETHERNET_MEDIA		0x00000002
#define	TWISTED_PAIR_MEDIA	0x00000003
#define	MICROCHANNEL		0x00000008
#define	INTERFACE_CHIP		0x00000010
#define	INTELLIGENT		0x00000020
#define	BOARD_16BIT		0x00000040
#define	RAM_SIZE_UNKNOWN	0x00000000	/* 000 => Unknown RAM Size */
#define	RAM_SIZE_RESERVED_1	0x00010000	/* 001 => Reserved */
#define	RAM_SIZE_8K		0x00020000	/* 010 => 8k RAM */
#define	RAM_SIZE_16K		0x00030000	/* 011 => 16k RAM */
#define	RAM_SIZE_32K		0x00040000	/* 100 => 32k RAM */
#define	RAM_SIZE_64K		0x00050000	/* 101 => 64k RAM */ 
#define	RAM_SIZE_RESERVED_6	0x00060000	/* 110 => Reserved */ 
#define	RAM_SIZE_RESERVED_7	0x00070000	/* 111 => Reserved */ 
#define	SLOT_16BIT		0x00080000
#define	NIC_690_BIT		0x00100000
#define	ALTERNATE_IRQ_BIT	0x00200000

#define	MEDIA_MASK		0x00000007
#define	RAM_SIZE_MASK		0x00070000
#define	STATIC_ID_MASK		0x0000FFFF

/* Word definitions for board types */
#define	WD8003E		ETHERNET_MEDIA
#define	WD8003EBT	WD8003E		/* functionally identical to WD8003E */
#define	WD8003S		STARLAN_MEDIA
#define	WD8003SH	WD8003S		/* functionally identical to WD8003S */
#define	WD8003WT	TWISTED_PAIR_MEDIA
#define	WD8003W		(TWISTED_PAIR_MEDIA | INTERFACE_CHIP)
#define	WD8003EB	(ETHERNET_MEDIA | INTERFACE_CHIP)
#define	WD8003ETA	(ETHERNET_MEDIA | MICROCHANNEL)
#define	WD8003STA	(STARLAN_MEDIA | MICROCHANNEL)
#define	WD8003EA	(ETHERNET_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003SHA	(STARLAN_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003WA	(TWISTED_PAIR_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8013EBT	(ETHERNET_MEDIA | BOARD_16BIT)
#define	WD8013EB	(ETHERNET_MEDIA | BOARD_16BIT | INTERFACE_CHIP)
#define	WD8023E		(ETHERNET_MEDIA | INTELLIGENT | INTERFACE_CHIP)


#define BID_SIXTEEN_BIT_BIT  0x01

#define REG_0            0x00
#define REG_1            0x01
#define REG_2            0x02

#define BOARD_ID_BYTE    0x0E
#define BID_REV_MASK     0x1E
#define BID_MSZ_583      0x08    /* memory size mask for 583 interface chip */
#define BID_RAM_SIZE_BIT 0x40


/***************************/
/* Specific to the 8013EBT */
/****************************/

/***** 83c583 registers *****/

#define BSR        0x01         /* Bus Size Register (read only) */
#define LAAR       0x05         /* LA Address Register */

/***** BSR Defs ****/

#define BUS16BIT   0x01         /* Bit 0 tells if the bus is 16 bit */


/**** LAAR Definitions ****/
#define	MEM16ENB	0x80		/* Enables 16bit shrd RAM for host */
#define	LAN16ENB	0x40		/* Enables 16bit shrd RAM for LAN */
#define SOFTINT         0x20            /* Enable interrupt from pc */
#define	LA23		0x10		/* Address lines for enabling */
#define	LA22		0x08		/*    shared RAM above 1Mbyte */
#define	LA21		0x04		/*    in host memory */
#define	LA20		0x02
#define	LA19		0x01



