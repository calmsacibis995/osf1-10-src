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
 * Unsigned remainder
 *
 * On input:
 *	$23/t9  = return address (RA)
 *	$24/t10 = dividend (numerator) (A)
 *	$25/t11 = divisor (denominator) (B)
 *	Note that the high 32 bits of r24 and r25 are ignored.
 *
 * On Return:
 *	$27/t12 = remainder (Q)
 *	All other registers, except $at and $gp, must be preserved
 */

#define ENTRY \
	.globl	__remlu; \
	.ent	__remlu; \
__remlu: ; \
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
	zap	A, 0xf0, A; \
	zap	B, 0xf0, B

#define UNSIGNED 1

#define THIRTYTWOBIT 1

#define REMAINDER 1

#define RETURN \
	addl	Q, 0, Q; \
	ldq	RA, 0*8(sp); \
	ldq	t0, 1*8(sp); \
	ldq	t1, 2*8(sp); \
	ldq	t2, 3*8(sp); \
	ldq	t3, 4*8(sp); \
	ldq	a0, 5*8(sp); \
	ldq	A, 6*8(sp); \
	ldq	B, 7*8(sp); \
	lda	sp, 8*8(sp); \
	ret	zero, (RA), 1

#include "divrem.s"
