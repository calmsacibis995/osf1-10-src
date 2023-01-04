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

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern int ffs(int index);
 *
 *	a0	index
 *
 * Find the first bit set in "index" (from LSB to MSB) and return the
 * index of that bit. Bits are numbered starting with 1. A return value
 * of 0 indicates that there were no bits set in "index".
 */
	.align	4
LEAF(ffs)
	.prologue 0

	addl	a0, 0, a0		# force index to be an int
	ldiq	v0, 1			# indexes are 1-based

	subq	zero, a0, t0		# complement all but first bit
	beq	a0, 20f			# quit now if no bits set

	and	a0, t0, t0		# clear all but first bit
	blbs	a0, 10f			# take an easy out

	extwl	t0, 2, t1		# get high word
	addq	v0, 16, t2		# adjust index assuming in high word
	cmovne	t1, t1, t0		# if yes, replace bits
	cmovne	t1, t2, v0		# if yes, replace index

	extbl	t0, 1, t1		# get high byte of low word
	addq	v0, 8, t2		# adjust index assuming in high byte
	cmovne	t1, t1, t0		# if yes, replace bits
	cmovne	t1, t2, v0		# if yes, replace index

	and	t0, 0x0f, t2		# null in bytes 0..3?
	addq	v0, 4, t1		# adjust index assuming no
	cmoveq	t2, t1, v0		# if no, replace index

	and	t0, 0x33, t3		# null in bytes 0..1, or 4..5?
	addq	v0, 2, t1		# adjust index assuming no
	cmoveq	t3, t1, v0		# if no, replace index

	and	t0, 0x55, t4		# null in bytes 0, 2, 4 of 6
	addq	v0, 1, t1		# adjust index assuming no
	cmoveq	t4, t1, v0		# if no, replace index

10:	RET

	.align	3
20:	bis	zero, zero, v0		# no bits set
	RET

END(ffs)
