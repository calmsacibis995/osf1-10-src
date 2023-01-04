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
 * @(#)$RCSfile: isp.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/08/18 19:17:42 $
 */

#ifndef _HAL_ISP_H_
#define _HAL_ISP_H_
/***************************************************************************/
/*                                                                         */
/* MODULE NAME: isp.h							   */
/* 									   */ 
/* LOCATION:	.../src/kernel/arch/alpha/hal				   */
/* 									   */ 
/* DESCRIPTION:								   */
/* 	Contains definitions for Integrated System Peripheral, ISP.	   */
/* 									   */ 
/* STRUCTURES:								   */
/* 									   */
/* 									   */ 
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/* Intel 82357 ISP defines. Note that register addresses are defined in    */
/* eisa.h because they are standard well known eisa addresses specified by */
/* the EISA specification.						   */
/*-------------------------------------------------------------------------*/

#define	NDMA_CHAN	8

#define DMA_CH0		0
#define DMA_CH1		1
#define DMA_CH2		2
#define DMA_CH3		3
#define DMA_CH4		4
#define DMA_CH5		5
#define DMA_CH6		6
#define DMA_CH7		7

#define ISP_CT_ENABLE	0x0
#define ISP_CT_DISABLE	0x4
#define ISP_PRI_FIXED	0x0
#define ISP_PRI_ROTATE	0x10
#define	ISP_DREQ_HIGH	0x0
#define	ISP_DACK_LOW	0x0
#define	ISP_DACK_HIGH	0x80

#define ISP_VERIF_XFER	0x0
#define ISP_WRITE_XFER	0x4
#define ISP_READ_XFER	0x8
#define ISP_AINIT_DIAB	0x00
#define ISP_AINIT_ENAB	0x10
#define	ISP_ADDR_INC	0x00
#define	ISP_ADDR_DEC	0x20
#define ISP_DEMAND	0x00
#define ISP_SINGLE	0x40
#define ISP_BLOCK	0x80
#define ISP_CASCADE	0xC0

#define ISP_CHAIN_OFF	0x00
#define ISP_CHAIN_ON	0x04
#define ISP_CHAIN_NRDY	0x00
#define ISP_CHAIN_RDY	0x08
#define ISP_CHAIN_IRQ	0x00
#define ISP_CHAIN_TC	0x10

#define	ISP_CLR_MASK	0x00
#define	ISP_SET_MASK	0x04

#define	ISP_CH0_MASK	0x1
#define	ISP_CH1_MASK	0x2
#define	ISP_CH2_MASK	0x4
#define	ISP_CH3_MASK	0x8
#define	ISP_CH4_MASK	0x1
#define	ISP_CH5_MASK	0x2
#define	ISP_CH6_MASK	0x4
#define	ISP_CH7_MASK	0x8
#define ISP_CH_MASK_ALL	0x0F



struct	dma_softc
   {
   struct  controller	*ownerp;	/* Pointer to ctlr structure of */
					/* current owner. */
   short		 mode_reg;	/* Base mode register setting. */
   short		 emode_reg;	/* Base extended mode reg. setting. */
   int			 allocated;	/* When set indicates channel is */
					/* allocated. */
   int			 has_chain;	/* When set indicates > 1 ba,bc */
					/* pair to use for transfer. */
   sglist_t		 sglistp;	/* Pointer to buffer address, count */
					/* pairs. */
   };

extern  struct  dma_softc	softc[];

#endif
