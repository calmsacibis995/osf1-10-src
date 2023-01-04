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
 * nvtcreg.h
 *
 * TC NVRAM presto driver
*/

#ifndef _NVTCREG_H_
#define _NVTCREG_H_


/* NVRAM registers */

struct nvtcreg {

	volatile u_int reg;
};

struct nvtc_reg {
  struct nvtcreg *nvtc_csr;
  struct nvtcreg *nvtc_dar;
  struct nvtcreg *nvtc_bat;
  struct nvtcreg *nvtc_mar;
  struct nvtcreg *nvtc_bcr;
};

#define NVRAM_REG volatile struct nvtc_reg
/* 
 * Define offsets to nvram device registers and NV RAM.
 */ 

#define	NVTC_CSR 		0x000100000
#define	NVTC_DAR        	0x000100004
#define NVTC_BAT		0x000100008
#define NVTC_MAR		0x00010000C
#define NVTC_BCR		0x000100010

#define TCNVRAM_DIAG_REGISTER	0x3f8			/* Diag register tells us if diags passed, and what size board */
#define BOARD_PASSED		0x00000008		/* If this bit is set board passed diags */
#define BOARD_SIZE		0x000000f0 		/* Mask for board size */

#define TCNVRAM_DIAG_RESVED	0x400			/* The amount of space diags require & assure 2K alignment for DMA */
#define TCNVRAM_OFFSET_RESVED	0x200			/* Common space to both 4 and 1 MB board firmware passes data to driver */
#define TCNVRAM_CACHE_OFFSET	0x400000+0x400000-4	/* Where firmware puts offset to cache last 32 bits in nvram 4mb space */


/* CSR register bit definitions */
#define	DMA_GO		0x00000001
#define ANTIHOG		0x00000002
#define DMA_DONE        0x00000080
#define BURST_128_BYTES	0x00000800
#define ENBL_PARITY	0x00001000
#define ENBL_BAT_INT	0x00002000
#define ENBL_DONE_INT	0x00004000
#define ENBL_ERR_INT	0x00008000
#define	BAT_DISCON	0x00100000
#define BAT_FAIL	0x00400000
#define TC_ERROR	0x01000000
#define	MEM_ERROR	0x02000000
#define TC_PAR_ERR	0x04000000
#define TC_PROTCAL_ERR	0x08000000        
#define ERROR_SUM	0x80000000

/*
 * Define constants used for upper layer presto commuication
 */
#define NVTC_MAPPED 	1		/* buffer is mapped */
#define NVTC_NOTMAPPED	0		/* buffer is not mapped */
#define NVTC_CACHED	1		/* Use kseg space */
#define NVTC_NOTCAHCED	0		/* Use a cached space */

#endif
