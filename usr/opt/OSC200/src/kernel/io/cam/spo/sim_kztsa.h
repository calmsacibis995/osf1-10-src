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
 * @(#)$RCSfile: sim_kztsa.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/13 21:10:45 $
 */
/*
 *  This file contains KZTSA hardware specific definitions
 */
#define KZTSA_TEST_START	0xC0008	/* Start diagnostic */
#define	KZTSA_BASE_ADDR		0xD0040
#define KZTSA_CLR_TC_INTR	0xE0000	/* Clear KZTSA TC INT register */
#define KZTSA_TEST_STATUS	0xD000C	/* Status */
#define KZTSA_ADAP_ERR_LOG	0xD00C0	/* Adapter Error Log Data Register */
#define KZTSA_RESET_START	0xF0000 /* Power up reset */

/*
 * KZTSA does not support CDB as a pointer.  Therefore the host driver
 * will not support CDB byte greater than 12 bytes.  For CDB as a pointer
 * and less than 12 bytes, the CDB is copied to the CCB's CDB.
 */
#define KZTSA_CDB_MAX_LEN       12

typedef struct kztsa_regs {
	U32	amcsr;		/* Adapter Maintenance Control and Staus */
	u_char  pad[4];		/* SBZ */
	REGBITS abbr;    	/* Adatper Block Base Register */
	REGBITS dafqir; 	/* Driver-Adapter Free Queue IR */
	REGBITS dacqir; 	/* Driver-Adapter Command Queue IR */
	u_char 	pad2[32];	/* Padding */
	U32	asr;		/* Adapter Status Register */
	u_char	pad3[4];	/* SBZ */
	REGBITS afar;		/* Adapter Falling Address Register */
	REGBITS afpr;		/* Adapter Falling Parameter Register */
} KZTSA_REGS;

#define AMCSR	amcsr
#define ABBR	abbr.paddr
#define DAFQIR	dafqir.paddr
#define DACQIR	dacqir.paddr
#define ASR	asr
#define AFAR	afar.paddr
#define AFPR	afpr.paddr


typedef struct kztsa_err_log_regs {
	U32	err_log_1;
	U32	err_log_2;
	U32	err_log_3;
	U32	err_log_4;
	U32	err_log_5;
	U32	err_log_6;
	U32	err_log_7;
	U32	err_log_8;
	U32	err_log_9;
	U32	err_log_a;
} KZTSA_ERR_LOG_REGS;


#define SPO_GET_KZTSAREGS(sc)	\
	((KZTSA_REGS *)DENSE((caddr_t)((sc)->reg) + KZTSA_BASE_ADDR))

#define KZTSA_GET_DENSE_REG_ADDR(addr)	\
	(caddr_t)(DENSE((addr) + KZTSA_BASE_ADDR))

