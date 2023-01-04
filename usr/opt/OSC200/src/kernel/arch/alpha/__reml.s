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
 *	Note that the high 32 bits of r24 and r25 are ignored.
 *
 * On Return:
 *	$27/t12 = quotient (Q)
 *	All other registers, except $at and $gp, are preserved
 */

#define ENTRY \
	.globl	__reml; \
	.ent	__reml; \
__reml: ; \
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
	addl	A, 0, A		/* make sure dividend is in canonical form */; \
	addl	B, 0, B		/* make sure divisor is in canonical form */; \
	/* check for INT_MIN / -1 */; \
	addl	B, 1, t0	/* 0 if B == -1; != 0 otherwise */; \
	bis	A, A, t1	/* copy dividend */; \
	cmovne	t0, 0, t1	/* replace w/ 0 if divisor != -1 */; \
	sublv	zero, t1, t1	/* trap if dividend = INT_MIN */; \
	subq	zero, A, t0	/* negate dividend */; \
	cmovlt	A, t0, A	/* get 64-bit absolute value of dividend */; \
	subq	zero, B, t1	/* negate divisor */; \
	cmovlt	B, t1, B	/* get 64-bit absolute value of divisor */

#define REMAINDER 1

#define THIRTYTWOBIT 1

#define RETURN \
	addl	Q, 0, Q; \
	subl	zero, Q, AT; \
	ldq	A, 6*8(sp); \
	addl	A, 0, t0; \
	cmovlt	t0, AT, Q; \
	ldq	RA, 0*8(sp); \
	ldq	t0, 1*8(sp); \
	ldq	t1, 2*8(sp); \
	ldq	t2, 3*8(sp); \
	ldq	t3, 4*8(sp); \
	ldq	a0, 5*8(sp); \
	ldq	B, 7*8(sp); \
	lda	sp, 8*8(sp); \
	ret	zero, (RA), 1

#include "divrem.s"
