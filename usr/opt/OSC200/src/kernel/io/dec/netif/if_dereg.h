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
 * @(#)$RCSfile: if_dereg.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/06/24 22:41:20 $
 */


/*
 * Header file for DEPCA DE422 network adapter driver.
 */
#ifndef _DEPCA_REG_	/* Multiple include protection */
#define _DEPCA_REG_

#ifdef 0
/* Tim -turns out this is not needed */
/*
 * The DEPCA does slot-dependent addressing to determine the mapping
 * of IO registers.  The DEPCA can reside at these EISA Slot Addresses:
 * They are calculated as ((0x1000 * Slot#) + 0xc00)
 *
#define	DEPCA_SLOT_1_BASE	0x1c00		/* EISA Slot 1 */
#define	DEPCA_SLOT_2_BASE	0x2c00		/* EISA Slot 2 */
#define	DEPCA_SLOT_3_BASE	0x3c00		/* EISA Slot 3 */
#define	DEPCA_SLOT_4_BASE	0x4c00		/* EISA Slot 4 */
#define	DEPCA_SLOT_5_BASE	0x5c00		/* EISA Slot 5 */
#define	DEPCA_SLOT_6_BASE	0x6c00		/* EISA Slot 6 */
#define	DEPCA_SLOT_7_BASE	0x7c00		/* EISA Slot 7 */
#endif /* 0 */

/*
 * Register offsets from the base:
 */
#define NICSR_OFFSET	0xC00
#define RDP_OFFSET	0xC04
#define RAP_OFFSET	0xC06
#define HIBASE_OFFSET	0xC08
#define ADP_OFFSET	0xC0C
#define NI_ROM_OFFSET	ADP_OFFSET	/* Apparent naming oops in spec */
#define CONFIG_OFFSET	0xC0C
#define ID_OFFSET	0xC80
#define CONTROL_OFFSET	0xC84

/* Tim -turns out this is not needed */
/*
 * The EISA DEPCA contains the following registers at the specified offset:
 * Here the L/W/B specifies access size where B=8 bits, W=16 bits, L=32 bits.
 * R/W specifies access mode, R=read, W=write
 *
 * Register	Offset	L/W/B		R/W
 * ---------------------------------------------------------------------
 * nicsr	0x0	W		R&W
 * rdp		0x4	W		R&W
 * rap		0x6	W		R&W
 * hibase	0x8	B		  W
 * adp		0xC	B		R  
 * config	0xC	W		  W
 * id		0x80	L&W&B		R  
 * control	0x84	B		R&W
 *
 * Most of the registers is 16 bits in length yet they are spaced
 * as if they were 32 bits in length.  For this reason a pad of
 * 16 bits is inserted between each register.
 *
 * Just to make things confusing, the "id" register can be accessed in
 * either 8, 16, or 32 bit quantities.
 *
 * The comment field lists the access mode, R=read, W=write.
 */
struct de_reg {
	volatile unsigned short de_nicsr;	    /* Control & Status	  RW  */
	volatile unsigned short pad1;	            
	volatile unsigned short de_rdp;	    	    /* Register Data Port RW  */
	volatile unsigned short de_rap;	    	    /* Register Addr Port RW  */
	volatile unsigned char de_hibase;	    /* Extmem base addr    W  */
	volatile unsigned char pad2;
	volatile unsigned short pad3;
	union {
		volatile unsigned char de_adp;	    /* Ethernet Addr-ROM  R   */
		volatile unsigned short de_config;  /* EISA Configuration  W  */
	} de_adp_config;
	unsigned char pad4[114];
	union {
		volatile unsigned char de_id_b;	    /* Byte access, 8-bit R   */
		volatile unsigned short de_id_w;    /* "word" acc, 16bit  R   */
		volatile unsigned int de_id_l;      /* "long" acc, 32bit  R   */
	} de_id;
	volatile unsigned char de_control;	    /* EISA Control	  RW  */
};

/*
 * Register Bit Definitions:
 */

/*
 * nicsr (Network Interface Control & Status) register
 * 16 bits R&W, Bits [15:9] Must be Zero
 */
#define NICSR_LED	0x1		/* Diag Led control, 1=on, 0=off */
#define NICSR_IEN	0x2		/* Interrupt Enable              */
#define NICSR_IM	0x4		/* Interrupt Mask		 */
#define NICSR_128KB	0x8		/* 128KB Huge Mode Enable	 */
#define NICSR_REMOTE	0x10		/* Remote Enable		 */
#define NICSR_BUF	0x20		/* Buffer Size			 */
#define NICSR_BS	0x40		/* Bank Select			 */
#define NICSR_SHE	0x80		/* Shadow Enable		 */

/*
 * rdp (LANCE Register Data Port) register
 * 16 bits R&W
 */

/*
 * rap (LANCE Register Address Port) register
 * 16 bits R&W
 */
#define RAP_CSR0	0x0		/* Lance CSR 0 at this "logical" addr */
#define RAP_CSR1	0x1		/* Lance CSR 1 at this "logical" addr */
#define RAP_CSR2	0x2		/* Lance CSR 2 at this "logical" addr */
#define RAP_CSR3	0x3		/* Lance CSR 3 at this "logical" addr */

/*
 * hibase (Extended Memory Base Address) register
 * 8 bits Write-only, Bits [7:4] Must be Zero
 * Bits [3:0] contains the 16MB boundary specification.
 */

/*
 * adp (Extended Address-ROM Data Port) register
 * 8 bits Read-only
 */

/*
 * config (EISA Configuration) register
 * 16 bits Write-only
 */
#define CONFIG_PADR17	0x1		/* Programs Buffer Memory Addr bit 17 */
#define CONFIG_PADR16	0x2		/* Programs Buffer Memory Addr bit 16 */
#define CONFIG_BUF	0x4		/* Select Buf mem window size         */
#define CONFIG_I5	0x8		/* Lance interrupts on IRQ5	      */
#define CONFIG_I9	0x10		/* Lance interrupts on IRQ9	      */
#define CONFIG_I10	0x20		/* Lance interrupts on IRQ10	      */
#define CONFIG_I11	0x40		/* Lance interrupts on IRQ11	      */
#define CONFIG_REMOTE	0x80		/* Remote Boot Enabled		      */
#define CONFIG_TIMEOUT	0x100		/* Remote Boot Timeout Period	      */

/*
 * id (EISA Identification) register
 * 32 bits Read-only
 * If read as a longword the register should contain the following value
 * which is a compressed identification code of "DEC4220"
 */
#define ID_DEC4220 	0x2042A310

/*
 * control (EISA Control) register
 * 8 bits R&W
 * Bits [7:1] are reserved/not implemented.  They are considered read-only.
 * Therefore this register contains only 1 useful bit.
 */
#define CONTROL_EISA_ENABLE	0x1	/* 1=board enable, 0=disable	*/

#define EISA_STUFF
#ifdef EISA_STUFF
/*---------------------------------------------------------------------*/
/*
 * This is a bunch of EISA code stolen from Gary.  It will be placed
 * somewhere else later.  For now hack it in this way.  Doesn't belong here.
 *
 * NOTE: these defines are likely to change then they are "officially"
 * added to the code base.
 */

/******************************************
*	Defines for EISA address translation.
*	When things work these should go somewhere else.
*******************************************/

#define	EISA_ADDR_MASK	0x1ffffff
#define EISA		((long)1<<33)
#define EISA_IO		((long)1<<32)
#define EISA_BYTE	((long)0)		/* 8 bits */
#define EISA_WORD	((long)1<<5)		/* 16 bits */
#define EISA_TRIB	((long)2<<5)
#define EISA_LONG	((long)3<<5)		/* 32 bits */
#define EISA_ADDR_SHIFT	7
#define HAER		0x1d0000000
#define EISA_ID_OFFSET	0x0c80
#define EISA_BCR_OFFSET	0X0C84
/*---------------------------------------------------------------------*/
#endif /* EISA_STUFF */

/*---------------------------------------------------------------------*/
/*
 * Wrapper macros to access the DEPCA registers in EISA IO space.
 * Unlike the previous section, this DOES belong in this file.
 *
 * NOTE: these defines are likely to change when the EISA bus support
 * code is added.
 */
#define EISA_DEPCA_ADDR( csrbase, depca_offset, bcount )		\
	eisa_to_cpu_ioaddr(csrbase | depca_offset, bcount)
#define EISA_DEPCA_IO8( csrbase, depca_offset )			\
	*(unsigned char *)PHYS_TO_KSEG(EISA_DEPCA_ADDR(csrbase,depca_offset,EISA_BYTE))
#define EISA_DEPCA_IO16( csrbase, depca_offset )			\
	*(unsigned short *)PHYS_TO_KSEG(EISA_DEPCA_ADDR(csrbase,depca_offset,EISA_WORD))
#define EISA_DEPCA_IO32( csrbase, depca_offset )			\
	*(unsigned int *)PHYS_TO_KSEG(EISA_DEPCA_ADDR(csrbase,depca_offset,EISA_LONG))

/*---------------------------------------------------------------------*/

#endif /* _DEPCA_REG_  - Must be last line - multi-include protection */
