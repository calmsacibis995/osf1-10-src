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
 * @(#)$RCSfile: kn121.h,v $ $Revision: 1.1.6.6 $ (DEC) $Date: 1993/11/15 21:09:06 $
 */
/*
 * OLD HISTORY
 * Revision 1.1.2.5  1993/03/08  21:13:36  Gary_Dupuis
 * 	Added defines used for IO address translation and IO access.
 * 	[1993/03/08  20:23:45  Gary_Dupuis]
 *
 * Revision 1.1.2.4  92/11/03  14:46:11  Timothy_Burke
 * 	Commented how THETA_VTI_BASE is derived.
 * 	[92/11/03  14:45:28  Timothy_Burke]
 * 
 * Revision 1.1.2.3  92/10/28  16:46:34  Paul_Grist
 * 	Replace old version with first pass of the real thing
 * 	[92/10/28  15:58:20  Paul_Grist]
 * 
 * Revision 1.1.2.2  92/10/12  07:44:10  Gary_Dupuis
 * 	Initial insertion into a live pool.
 * 	[92/10/09  09:53:13  Gary_Dupuis]
 * 
 */

#ifndef _HAL_KN121_H_
#define _HAL_KN121_H_
/*
 * Modification History: kn121.h   JENSEN ALPHA PC/EISA system
 *
 * 01-Oct-92 -- Paul Grist
 * 	Clean version of jensen header file created from old versions.
 *      
 */

/*
 * Jensen Platform specific SCB assignments
 */

/*
 *  EV4 I/O assignements
 */
#define KN121_SCB_COMMA		0x900		/* Console ACE serial port */
#define KN121_SCB_COMMB		0x920		/* Optional ACE serail port */
#define KN121_SCB_KEYBD		0x980		/* PC Keyboard */
#define KN121_SCB_MOUSE		0x990		/* PC Mouse */

/* 
 * Local I/O Assignment 
 */
#define	KN121_SCB_LP		0x810		 /* Parallel Printer (IRQ1) */


/* 
 *   Jensen 82357 PIC IRQ and SCB assignment definitions  
 *
 * These are the SCB assignments for the Jensen,  becuase the IRQ
 * to SCB mapping is standard,  Jensen uses the generic irq_to_scb
 * mapping provided in pic.h and pic.c.
 *
 * This comment is here for completeness
 *
 *
 * KN121_SCB_TIMER	0x800		 i82357 Timer (IRQ0) 
 * KN121_SCB_LP		0x810		 Parallel Printer (IRQ1) 
 *                                       IRQ2 is cascade 
 * KN121_SCB_EISA_IRQ3	0x830		 EISA IRQ3 from PIC 
 * KN121_SCB_EISA_IRQ4	0x840		 EISA IRQ4 from PIC 
 * KN121_SCB_EISA_IRQ5	0x850		 EISA IRQ5 from PIC 
 * KN121_SCB_EISA_IRQ6	0x860		 EISA IRQ6 from PIC 
 * KN121_SCB_EISA_IRQ7	0x870		 EISA IRQ7 from PIC 
 *                                       IRQ 8 Not connected 
 * KN121_SCB_EISA_IRQ9	0x890		 EISA IRQ9 from PIC 
 * KN121_SCB_EISA_IRQ10	0x8A0		 EISA IRQ10 from PIC 
 * KN121_SCB_EISA_IRQ11	0x8B0		 EISA IRQ11 from PIC 
 * KN121_SCB_EISA_IRQ12	0x8C0		 EISA IRQ12 from PIC 
 * KN121_SCB_EISA_IRQ13	0x8D0		 IRQ13 is for DMA chaining 
 * KN121_SCB_EISA_IRQ14	0x8E0		 EISA IRQ14 from PIC 
 * KN121_SCB_EISA_IRQ15	0x8F0		 EISA IRQ15 from PIC 
 *
 *
 */



/*
 * Error handling SCB offset values (architected in SRM).
 *
 */
#define SYS_CORR_ERR	0x620	/* System correctable error */
#define PROC_CORR_ERR	0x630	/* Processor correctable error */
#define SYS_MCHECK	0x660	/* System machine check */
#define PROC_MCHECK	0x670	/* Processor machine check */


/*
 * 	VTI Combo chip definitions
 *
 *
 * Jensen local I/O addresses for the VTI combo chip are 
 * decoded as follows:
 * 			cA[33:32] = 01
 * 			cA[31] = 1
 * 			cA[30] = 1
 * 			cA[29:28] = 00
 *
 * Throwing this all together you get cA[33:28] = 011100 which equals 0x1C
 * shifted by the appropriate number of bits.
 */

/*
 * This defines how many bits the local port addr must be shifted to match
 * the appropriate address encoding for local i/o
 */
#define KN121_SHIFT	9	/* Local I/O swizzle shift value */
#define KN121_VTI_BASE	0x1C0000000L  /* Base address of Combo chip */
#define KN121_RTC_RAP_BASE 0x170  /* RTC Register Address Port */
#define KN121_RTC_RDP_BASE 0x171  /* RTC Register Data Port */

/* 
 *	Defines used for IO address translation and access.
 */
#define	KN121_SYSMAP_MASK 0xffffffff00000000L /* Upper 32-bits of 
						 address/io_handle */
#define	KN121_LOW_ADDR_MASK  0x01ffffffL  /* Selects low 25 bits of the 
					     device address. */
#define	KN121_HIGH_ADDR_MASK 0xfe000000	  /* Selects bits 25:32 of the */
					  /* device address. */
#define KN121_EISA_ALIGN_MASK 0xfffffffffffffe7f /* aligns swizzeled address
					            to long word boundary */
#define KN121_EISA_BUS_MASK 0x3f0000000   /* selects bus specific portion of
				             eisa address */
#define KN121_EISA_BIT  0x0200000000L	  /* A cpu address with this
					     bit on indicates an EISA addr */
#define KN121_EISA_IO_BIT 0x0100000000L	  /* If set indicate an EISA */
					  /* IO cycle. Not set indicates a 
					     memory cycle. */
#define KN121_EISA_STRIDE	0x200           /* Stride for swizzled address*/
#define KN121_EISA_MEM          0x200000000L	/* base eisa memory address */
#define KN121_EISA_IO           0x300000000L    /* base eisa io address */

#define KN121_IO (KN121_EISA_BIT | KN121_EISA_IO_BIT)
#define KN121_EISA_IO_SHIFT	32		/* Bit location of IO flag. */
#define KN121_EISA_ADDR_SHIFT	7		/* Shift required to go from
						   device address to cpu addr*/
#define	KN121_WIDTH_MASK	0x060L		/* Masks of the byte width 
						   in a translated IO addr */
#define KN121_WIDTH_SHIFT	5
#define KN121_OFFSET_MASK	0x0180L		/* Mask of byte offset in
						   translated IO access.   */
#define KN121_HAE	     	0x1d0000000L	/* Location of Host Address 
						   Extension Register */
#define KN121_HAE_SHIFT		25		/* shift required to go from 
						   io_handle_t to hae reg */
#define KN121_HAE_MASK          0x7f		/* selects hae */

#endif
