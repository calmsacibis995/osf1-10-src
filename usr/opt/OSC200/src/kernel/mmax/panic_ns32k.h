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
 *	@(#)$RCSfile: panic_ns32k.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:16 $
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
 * Mach Operating System
 * Copyright (c) 1989 Encore Computer Corporation
 */

#if	MMAX_XPC || MMAX_APC || MMAX_DPC

#include "sys/types.h"

/*
 * Panic Save area, where registers go on a crash:
 *        usp, r0-r7, sp, fp, pc, psl/mod, misc.
 * N.B.:  this arrangement isn't idle, the usp through ssp must match
 * the order in which the chip lays down the registers on a trap/interrupt
 * and in which a BUILD_FRAME (locore.s) places registers on the stack.
 */

#define	PSA_PANIC_VERSION	1

#define	PSA_DPC_TYPE		1
#define	PSA_APC_TYPE		2
#define	PSA_XPC_TYPE		3

typedef struct psa {
	/* Panic Save Area Identification -- Always at Beginning, Please */
	u_int	psa_version;
	u_int	psa_size;
	u_int	psa_cputype;

	/* Integer CPU State */
#define	psa_frame_base	psa_usp
	u_int	psa_usp;
	u_int	psa_r7;
	u_int	psa_r6;
	u_int	psa_r5;
	u_int	psa_r4;
	u_int	psa_r3;
	u_int	psa_r2;
	u_int	psa_r1;
	u_int	psa_r0;
	u_int	psa_fp;
	u_int	psa_pc;
	u_int	psa_psr;
	u_int	psa_ssp;
#define	psa_frame	psa_ssp
	u_int	psa_sb;
	u_int	psa_intbase;
	u_int	psa_cfg;		/* only available on 532 */

	/* Memory Management Information */
	u_int	psa_ptb0;		/* page table base (kernel) */
	u_int	psa_ptb1;		/* page table base (user) */

					/* 532-specific */
	u_int	psa_mcr;		/* memory management control */
	u_int	psa_msr;		/* memory management status */
	u_int	psa_tear;		/* translation exception address */
	u_int	psa_bpc;		/* breakpoint program counter */
	u_int	psa_car;		/* compare address */
	u_int	psa_dcr;		/* debug condition */
	u_int	psa_dsr;		/* debug status */

					/* 332-specific */
#define	psa_few		psa_mcr		/* memory management control */
#define	psa_asr		psa_msr		/* memory management  */
/*	psa_tear	psa_tear	/* translation exception address */
#define	psa_bear	psa_dsr		/* bus error address */
#define	psa_bar		psa_bpc		/* breakpoint address */
#define	psa_bmr		psa_car		/* breakpoint mask */
#define	psa_bdr		psa_dcr		/* breakpoint data */

					/* 032-specific */
/*	psa_msr		psa_msr		/* memory management status */
#define	psa_eia		psa_tear	/* error/invalidate address */
#define	psa_bpr0	psa_bpc		/* breakpoint register 0 */
#define	psa_bpr1	psa_car		/* breakpoint register 1 */
#define	psa_bcnt	psa_dcr		/* breakpoint count */

	/* Floating Point State */
	u_int	psa_fsr;		/* floating point status register */
	double	psa_f0;			/* l0 = (f1, f0) on 532 */
	double	psa_f1;
	double	psa_f2;			/* l2 = (f3, f2) on 532 */
	double	psa_f3;
	double	psa_f4;			/* l4 = (f5, f4) on 532 */
	double	psa_f5;
	double	psa_f6;			/* l6 = (f7, f6) on 532 */
	double	psa_f7;

	/* Interrupt Controller State */
					/* Master ICU -- XPC and APC */
	u_short	psa_icu_ipnd;		/* Pending interrupts */
	u_short	psa_icu_imsk;		/* Masked intverrupts */
	u_short	psa_icu_isrv;		/* In service interrupts */
					/* Slave ICU -- APC only */
	u_short	psa_sicu_ipnd;		/* Pending interrupts (slave ICU) */
	u_short	psa_sicu_imsk;		/* Masked interrupts (slave ICU) */
	u_short	psa_sicu_isrv;		/* In service interrupts (slave ICU) */

	/* Pending Vectors */
#define	PSA_VBMAX	16		/* Beware of changes in VB fifo len */
	u_char	psa_vbfifo[PSA_VBMAX];	/* Pending vectors */

	/* Miscellaneous */
	u_int	psa_cpu_reg_csr;	/* CPU's CSR register */
	u_int	psa_cpu_reg_err;	/* CPU's error status, XPC or APC */
	u_int	psa_timestamp;		/* timestamp to track cpu check-in */
} psa_t;

#endif	MMAX_XPC || MMAX_APC || MMAX_DPC
