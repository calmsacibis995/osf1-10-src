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
 *	@(#)$RCSfile: reg.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:17:33 $
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
 * derived from reg.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/* This is all unnecessary, it duplicates thread_status.h and I
   should eventually flush it */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Location of the users' stored
 * registers in exception frame.
 * Usage is u.u_ar0[XX] or USER_REG(x).
 */

#define	EF_ARGSAVE0	0		/* arg save for c calling seq */
#define	EF_ARGSAVE1	1		/* arg save for c calling seq */
#define	EF_ARGSAVE2	2		/* arg save for c calling seq */
#define	EF_ARGSAVE3	3		/* arg save for c calling seq */
#define	EF_AT		4		/* r1:  assembler temporary */
#define	EF_V0		5		/* r2:  return value 0 */
#define	EF_V1		6		/* r3:  return value 1 */
#define	EF_A0		7		/* r4:  argument 0 */
#define	EF_A1		8		/* r5:  argument 1 */
#define	EF_A2		9		/* r6:  argument 2 */
#define	EF_A3		10		/* r7:  argument 3 */
#define	EF_T0		11		/* r8:  caller saved 0 */
#define	EF_T1		12		/* r9:  caller saved 1 */
#define	EF_T2		13		/* r10: caller saved 2 */
#define	EF_T3		14		/* r11: caller saved 3 */
#define	EF_T4		15		/* r12: caller saved 4 */
#define	EF_T5		16		/* r13: caller saved 5 */
#define	EF_T6		17		/* r14: caller saved 6 */
#define	EF_T7		18		/* r15: caller saved 7 */
#define	EF_S0		19		/* r16: callee saved 0 */
#define	EF_S1		20		/* r17: callee saved 1 */
#define	EF_S2		21		/* r18: callee saved 2 */
#define	EF_S3		22		/* r19: callee saved 3 */
#define	EF_S4		23		/* r20: callee saved 4 */
#define	EF_S5		24		/* r21: callee saved 5 */
#define	EF_S6		25		/* r22: callee saved 6 */
#define	EF_S7		26		/* r23: callee saved 7 */
#define	EF_T8		27		/* r24: code generator 0 */
#define	EF_T9		28		/* r25: code generator 1 */
#define	EF_K0		29		/* r26: kernel temporary 0 */
#define	EF_K1		30		/* r27: kernel temporary 1 */
#define	EF_GP		31		/* r28: global pointer */
#define	EF_SP		32		/* r29: stack pointer */
#define	EF_FP		33		/* r30: frame pointer */
#define	EF_RA		34		/* r31: return address */

#define	EF_SR		35		/* status register */
#define	EF_MDLO		36		/* low mult result */
#define	EF_MDHI		37		/* high mult result */
#define	EF_BADVADDR	38		/* bad virtual address */
#define	EF_CAUSE	39		/* cause register */
#define	EF_EPC		40		/* program counter */
#define EF_SYS1		41		/* system specific save 1 */

#define	EF_SIZE		(42*4)		/* size of exception frame */

/*
 * Macro to find user registers in core files
 * for use by debuggers, etc.
 * Note the USER_REG macro is located in mach/mips/thread_sataus.h
 */
#define	CORE_REG(reg, ubase)	\
	(((unsigned *)((unsigned)(ubase)+UPAGES*NBPG-EF_SIZE))[reg])
