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
 * @(#)$RCSfile: envram_reg.h,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/19 21:55:21 $
 */

#ifndef _ENVRAM_REG_H_
#define _ENVRAM_REG_H_

/***************************************************************************
 *
 * envram.h
 *
 * Digital EISA NVRAM (DEC2500) presto driver
 *
 ***************************************************************************/


/* 
 * EISA NVRAM register definitions
 */

/* 
 * Define offsets to nvram device registers
 */ 

#define	ENVRAM_CSR 		0xc00	/* CSR */
#define	ENVRAM_BAT        	0xc04	/* Battery Disconnect */
#define ENVRAM_HIBASE		0xc08	/* Ext. Mem Config */
#define ENVRAM_CONFIG		0xc0c	/* EISA config reg */ 
#define ENVRAM_ID		0xc80	/* EISA ID reg */
#define ENVRAM_CTRL		0xc84	/* EISA control */
#define ENVRAM_DMA0		0xc88	/* DMA addr reg 0 */
#define ENVRAM_DMA1		0xc8c	/* DMA addr reg 1 */

/* PSGFIX: Need to work diag implementation
 * 
 * These are TC NVRAM Soft Diag reg defs - What for EISA?? 
 */

/* psgfix: wired up and ignored for power-on 
 * Diag soft register tells us if diags passed, and what size board 
 */
#define ENVRAM_DIAG_REGISTER	0x3f8    /* 1k - 1*/ 

#define BOARD_FAILED		0x00000008 /*bit is set if board passed diags*/
#define BOARD_SIZE		0x000000f0 /* Mask for board size */
#define ENVRAM_DIAG_RESVED	0x400	   /* The amount of space diags require
					      and assure 2K alignment for DMA*/
#define ENVRAM_OFFSET_RESVED	0x200	   /* Common space to both 4 and 
					      1 MB board where firmware 
					      passes data to driver */

/* 
 * Where firmware puts offset to cache last 32 bits in nvram 4mb space 
 */
#define ENVRAM_CACHE_OFFSET	0x400 /*PSGFIX - this my cookie location*/


/* 
 * CSR register bit mask definitions 
 */
#define SET_LED		0x0100	/* Turn LED on */
#define ENBL_BAT_INT	0x0200  /* Enable battery fail interrupt */
#define ENBL_PFAIL_INT	0x0400  /* Enable power fail interrupt */
#define BAT_FAIL	0x0800  /* Indicated Battery failure */
#define	BAT_DISCON	0x1000  /* Indicated status of disconnect circuit*/
#define	WRMEM		0x2000  /* Enable writes to ENVRAM memory */
#define	SET_DREQ	0x4000  /* Set DREQ for DMA */
#define DMA_CHAN_7        0X80  /* Channel 7 for DMA */
#define DMA_CHAN_5        0x40  /* Channel 5 for DMA */

/* 
 * Battery disconnect register bit mask defs 
 */
#define BAT_DISCON_BIT	0x0080  /* bit to hit with connect sequence */

/* 
 * EISA Control Register bit masks 
 */
#define EISA_ENABLE_BOARD  0x1  /* EISA config enable - makes memory visable */

/* 
 * EISA ID register bit mask   PSGFIX: What is it really?
 *
#define ENVRAM_ID_MASK	0x10a32500
 */
#define ENVRAM_ID_MASK	0x0025a310


/*
 * Define constants used for upper layer presto commuication
 */
#define ENVRAM_MAPPED 	 1	/* buffer is mapped */
#define ENVRAM_NOTMAPPED 0	/* buffer is not mapped */
#define ENVRAM_CACHED	 1	/* Use kseg space */
#define ENVRAM_NOTCAHCED 0	/* Use a cached space */

/*
 * Define allignment boundaries
 */
#define ENVRAM_XFER_SIZE 1024   /* Maximum DMA transfer size to NVRAM module */
#define ENVRAM_ALLIGN    8192   /* DMA allignment required */

#endif

