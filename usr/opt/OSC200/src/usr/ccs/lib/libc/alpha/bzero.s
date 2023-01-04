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
 * @(#)$RCSfile: bzero.s,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/01/19 14:04:54 $
 */

#include <alpha/asm.h>
#include <alpha/regdef.h>

#define NBPC 64		/* number of bytes per chunk */

/*
 * extern void bzero(char *string, int length);
 *
 *	a0	string
 *	a1	length
 *
 * Zero "length" bytes starting at the location specified by "string"
 * parameter.
 */
	.align	4
LEAF(bzero)
	.prologue 0

	addl	a1, 0, a1		# ensure int length
	subq	zero, a0, t0		# negate dest pointer
	and	t0, 8-1, t0		# bytes till dest is aligned
	ble	a1, done		# quit if nothing to copy
	bic	a1, NBPC-1, t3		# number of chunks to copy
	beq	t0, chunks		# if aligned try for chunks

	# The destination is not quadword aligned. If the length is
	# large enough, zero up to the next quadword boundary

	cmpult	a1, t0, t1		# zero to next QW boundary?
	ldq_u	t2, 0(a0)		# get initial dest bytes
	bne	t1, partial		# go handle partial quad
	subq	a1, t0, a1		# adjust length
	bic	a1, NBPC-1, t3		# number of chunks to copy
	mskql	t2, a0, t2		# zero to next QW
	stq_u	t2, 0(a0)		# store the zeroed bytes
	addq	a0, t0, a0		# bump dest pointer

	# zero in NBPC-byte chunks

	.align	3
chunks:	and	a1, NBPC-8, t4		# number of single QWs to copy
	beq	t3, quads		# skip if no chunks to copy
10:	stq	zero, 0*8(a0)		# quad 0
	subq	t3, NBPC, t3		# decr chunk count
	stq	zero, 1*8(a0)		# quad 1
	stq	zero, 2*8(a0)		# quad 2
	stq	zero, 3*8(a0)		# quad 3
	stq	zero, 4*8(a0)		# quad 4
	stq	zero, 5*8(a0)		# quad 5
	stq	zero, 6*8(a0)		# quad 6
	stq	zero, 7*8(a0)		# quad 7
	addq	a0, NBPC, a0		# point to next chunk
	bne	t3, 10b			# loop on chunk count

	# zero a quadword at a time

	.align	3
quads:	and	a1, 8-1, t5		# number of bytes to zero
	beq	t4, bytes		# skip if no whole QWs left
20:	stq	zero, 0(a0)
	subq	t4, 8, t4		# decr quad*8 count
	addq	a0, 8, a0		# update dest pointer
	bne	t4, 20b			# loop on quad*8 count

	# zero partial part of last quadword

	.align	3
bytes:	bne	t5, 30f			# skip if bytes to zero
	RET
30:	ldq	t0, 0(a0)		# get next QW
	nop				# so mskqh and stq dual issue
	mskqh	t0, t5, t0		# zero remaining bytes
	stq	t0, 0(a0)		# store the zeroed bytes
done:	RET

	# zero a partial quadword, i.e. neither the starting or ending
	# bytes are on a QW boundary

	.align	3
partial:
	ldiq	t0, -1			# get mask of all 0xff bytes
	mskql	t0, a1, t0		# make into mask of len 0xff bytes
	nop				# so bic and stq_u dual issue
	insql	t0, a0, t0		# shift to match alignment
	bic	t2, t0, t2		# zero out the bytes
	stq_u	t2, 0(a0)		# store the zeroed bytes
	RET

END(bzero)
