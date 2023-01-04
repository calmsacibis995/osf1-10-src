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
 * Signed division
 *
 * On input:
 *	$23/t9  = return address (RA)
 *	$24/t10 = dividend (numerator) (A)
 *	$25/t11 = divisor (denominator) (B)
 *
 * On Return:
 *	$27/t12 = quotient (Q)
 *	All other registers, except $at and $gp, must be preserved
 */

#define ENTRY \
	.globl	__divq; \
	.ent	__divq; \
__divq: ; \
	.frame	sp, 8*8, RA; \
	.mask	(M_T0|M_T1|M_T2|M_T3|M_A0|M_T9|M_T10|M_T11), -8*8; \
	ldgp	gp, 0(pv); \
	lda	sp, -8*8(sp); \
	stq	RA, 0*8(sp); \
	stq	t0, 1*8(sp); \
	stq	t1, 2*8(sp); \
	stq	t2, 3*8(sp); \
	stq	t3, 4*8(sp); \
	stq	a0, 5*8(sp); \
	stq	A, 6*8(sp); \
	stq	B, 7*8(sp); \
	.prologue 1; \
	/* check for LONG_MIN / -1 */; \
	addq	B, 1, t0	/* 0 if B == -1; != 0 otherwise */; \
	bis	A, A, t1	/* copy dividend */; \
	cmovne	t0, 0, t1	/* replace w/ 0 if divisor != -1 */; \
	subqv	zero, t1, t1	/* trap if dividend = LONG_MIN */; \
	subq	zero, A, t0	/* negate dividend */; \
	cmovlt	A, t0, A	/* get absolute value of dividend */; \
	subq	zero, B, t1	/* negate divisor */; \
	cmovlt	B, t1, B	/* get absolute value of divisor */

#define RETURN \
	subq	zero, Q, AT; \
	ldq	A, 6*8(sp); \
	ldq	B, 7*8(sp); \
	xor	A, B, t0	/* compute sign of quotient */; \
	cmovlt	t0, AT, Q; \
	ldq	RA, 0*8(sp); \
	ldq	t0, 1*8(sp); \
	ldq	t1, 2*8(sp); \
	ldq	t2, 3*8(sp); \
	ldq	t3, 4*8(sp); \
	ldq	a0, 5*8(sp); \
	lda	sp, 8*8(sp); \
	ret	zero, (RA), 1

#include "divrem.s"
