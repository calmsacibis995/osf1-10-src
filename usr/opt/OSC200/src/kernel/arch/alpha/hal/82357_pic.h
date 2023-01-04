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
 * @(#)$RCSfile: 82357_pic.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/02 15:25:43 $
 */

#ifndef _ALPHA_HAL_PIC_H_
#define _ALPHA_HAL_PIC_H_
/*****************************************************************************
 *
 *	82357_pic.h
 *
 * 	82357 Programmable Interrupt Controller code.
 *	To be used by systems that use the 82357
 *	PIC chip for I/O interrupts or other
 *	purposes.
 *
 *
 ****************************************************************************/

/***********************************
 *
 *  Intel 82357-compatible PIC defs
 *
 ***********************************/


/* 
 * Edge/Level register controls PIC interrupt expected triggering
 */
#define PIC_ELR1   0x4d0		/* CTLR1 Edge/Level register */
#define PIC_ELR2   0x4d1		/* CTLR2 Edge/Level register */

/*
 * Operation Control Words are used to issue commands to PIC
 *
 */
#define CTRL2_OCW1   0xa1	/* CTLR2 Operation Command Word 1 addr */
#define CTRL2_OCW2   0xa0	/* CTLR2 Operation Command Word 2 addr */
#define CTRL2_OCW3   0xa0	/* CTLR2 Operation Command Word 3 addr */

#define CTRL1_OCW1   0x21	/* CTLR1 Operation Command Word 1 addr */
#define CTRL1_OCW2   0x20	/* CTLR1 Operation Command Word 2 addr */
#define CTRL1_OCW3   0x20	/* CTLR1 Operation Command Word 3 addr */

/*
 * Non Maskable Interrupt (NMI) status and enable registers
 */
#define NMI_CTRL     0x61       /* NMI Status and Control */
#define NMI_ECTRL    0X461      /* NMI Extended Satus and Control */

/*
 * NMI Enable definitions
 */
#define NMI_PARITY   0x0         /* NMI Parity Enable */
#define NMI_IOCHK    0x0         /* NMI I/O Check Enable */
#define NMI_FAILSAFE 0x400       /* NMI Failsafe Enable */
#define NMI_TIMEOUT  0x800       /* NMI Timeout Enable */

/*
 * Error Defines returned from pal
 */
#define PARITY_ERR   20          /* Parity Error */
#define IOCHECK_ERR  21          /* I/O Check Error */
#define TIMEOUT_ERR  22          /* Timeout Error */
#define SLAVE_ERR    23          /* Slave Timeout Error */
#define FAILSAFE_ERR 24          /* Failsafe Timer Error */
#define SOFT_NMI_ERR 25          /* Soft NMI Error */

/* 
 * OWC2 command definitions  (Operation Command Word 2) 
 */
#define NON_SPEC_EOI  0x20	/* Non-Specific End-of-intr mode*/
#define SPEC_EOI      0x60	/* Specific End-of-intr mode */
#define ROTATE_NS     0xa0	/* Rotate Priority on Non-specific EOI */
#define ROTATE_AUTO_S 0x80	/* Rotate Priority in Auto EOI mode - SET */
#define ROTATE_AUTO_C 0x0	/* Rotate Priority in Auto EOI mode - CLR */
#define ROTATE_SPEC   0xe0	/* Rotate Priority on specific EOI */
#define SET_PRIOR     0xc0	/* Set priority command */
#define NOP           0x40	/* No Operation */


/* 
 * OWC3 command definitions   (Operation Command Word 3) 
 */
#define READ_IR       0x02	/* Read Interrupt Request Register */
#define READ_IS       0x03	/* Read In-Service Register */
#define RESET_SPMASK  0x80	/* Reset Special mask mode */
#define SET_SPMASK    0xc0	/* Set Special mask mode */


/* 
 *   Standard 82357 PIC IRQ assignment definitions  
 */
#define PIC_IRQ0       0		/* timer */
#define PIC_IRQ1       1		/* IRQ1 */
#define PIC_IRQ2       2		/* Internal cascade */
#define PIC_IRQ3       3		/* IRQ3 */
#define PIC_IRQ4       4		/* IRQ4 */
#define PIC_IRQ5       5		/* IRQ5 */
#define PIC_IRQ6       6		/* IRQ6 */
#define PIC_IRQ7       7		/* IRQ7 */
#define PIC_IRQ8       8		/* Not connected */
#define PIC_IRQ9       9		/* IRQ9 */
#define PIC_IRQ10      10		/* IRQ10 */
#define PIC_IRQ11      11		/* IRQ11 */
#define PIC_IRQ12      12		/* IRQ12 */
#define PIC_IRQ13      13		/* DMA Chaining */
#define PIC_IRQ14      14		/* IRQ14 */
#define PIC_IRQ15      15		/* IRQ15 */


/*
 *  Standard 82357 PIC SCB assignments
 */
#define PIC_SCB_IRQ0		0x800		/* i82357 Timer (IRQ0) */
#define PIC_SCB_IRQ1		0x810		/* IRQ1 */
#define PIC_SCB_IRQ2		0x820	        /* IRQ2 is cascade */
#define PIC_SCB_IRQ3		0x830		/* IRQ3  */
#define PIC_SCB_IRQ4		0x840		/* IRQ4  */
#define PIC_SCB_IRQ5		0x850		/* IRQ5  */
#define PIC_SCB_IRQ6		0x860		/* IRQ6  */
#define PIC_SCB_IRQ7		0x870		/* IRQ7  */
#define PIC_SCB_IRQ8 		0x880           /* IRQ8  */
#define PIC_SCB_IRQ9		0x890		/* IRQ9  */
#define PIC_SCB_IRQ10		0x8A0		/* IRQ10 */
#define PIC_SCB_IRQ11		0x8B0		/* IRQ11 */
#define PIC_SCB_IRQ12		0x8C0		/* IRQ12 */
#define PIC_SCB_IRQ13		0x8D0		/* IRQ13 is for DMA chaining */
#define PIC_SCB_IRQ14		0x8E0		/* IRQ14 */
#define PIC_SCB_IRQ15		0x8F0		/* IRQ15 */


/********************************************************
 *
 *   End of 82357 PIC definitions
 *
 ********************************************************/

#endif
