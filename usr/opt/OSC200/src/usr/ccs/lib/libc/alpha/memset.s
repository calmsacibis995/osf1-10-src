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
 * @(#)$RCSfile: memset.s,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/08 01:24:39 $
 */
#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <alpha/asm.h>
#include <alpha/regdef.h>

#define NBPC 64		/* number of bytes per chunk */

/*
 * extern void *memset(void *s, int c, size_t n);
 *
 *	a0	string address (s)
 *	a1	fill char (c)
 *	a2	length (n)
 *
 * Copy the value of the character specified by the "c" parameter
 * (which is converted to an unsigned char) into each of the first
 * "n" locations of the string pointed to by the "s" parameter.
 *
 * The string pointed to by the "s" parameter becomes the return value.
 */
	.align	4
LEAF(memset)
	.prologue 0

	bis	a0, a0, v0		# set return value
	beq	a2, done		# done if zero length
	and	a1, 255, a1		# isolate fill byte
	subq	zero, a0, t0		# negate dest pointer
	and	t0, 8-1, t0		# bytes till dest is aligned
	beq	a1, 10f			# take shortcut for 0 fill byte

	sll	a1, 8, t1		# shift left a byte
	or	a1, t1, a1		# form fill word
	sll	a1, 16, t1		# shift left 2 bytes
	or	a1, t1, a1		# form fill longword
	sll	a1, 32, t1		# shift left 4 bytes
	or	a1, t1, a1		# form fill quadword

10:	bic	a2, NBPC-1, t3		# number of chunks to copy
	beq	t0, chunks		# if aligned try for chunks

	# The destination is not quadword aligned. If the length is
	# large enough, fill up to the next quadword boundary

	cmpult	a2, t0, t1		# fill to next QW boundary?
	ldq_u	t2, 0(a0)		# get initial dest bytes
	insql	a1, a0, t4		# shift fill bytes to dst alignment
	bne	t1, partial		# go handle partial quad
	subq	a2, t0, a2		# adjust length
	bic	a2, NBPC-1, t3		# number of chunks to copy
	mskql	t2, a0, t2		# zap bytes to fill
	or	t4, t2, t2		# merge in fill bytes
	stq_u	t2, 0(a0)		# store the filled bytes
	addq	a0, t0, a0		# bump dest pointer

	# fill in NBPC-byte chunks

	.align	3
chunks:	and	a2, NBPC-8, t4		# number of single QWs to copy
	beq	t3, quads		# skip if no chunks to copy
20:	stq	a1, 0*8(a0)		# quad 0
	subq	t3, NBPC, t3		# decr chunk count
	stq	a1, 1*8(a0)		# quad 1
	stq	a1, 2*8(a0)		# quad 2
	stq	a1, 3*8(a0)		# quad 3
	stq	a1, 4*8(a0)		# quad 4
	stq	a1, 5*8(a0)		# quad 5
	stq	a1, 6*8(a0)		# quad 6
	stq	a1, 7*8(a0)		# quad 7
	addq	a0, NBPC, a0		# point to next chunk
	bne	t3, 20b			# loop on chunk count

	# fill a quadword at a time

	.align	3
quads:	and	a2, 8-1, t5		# number of bytes to fill
	beq	t4, bytes		# skip if no whole QWs left
30:	stq	a1, 0(a0)
	subq	t4, 8, t4		# decr quad*8 count
	addq	a0, 8, a0		# update dest pointer
	bne	t4, 30b			# loop on quad*8 count

	# fill partial part of last quadword

	.align	3
bytes:	bne	t5, 40f			# skip if bytes to fill
	RET
40:	ldq	t0, 0(a0)		# get next QW
	mskqh	t0, t5, t0		# zap bytes to be filled
	mskql	a1, t5, a1		# zap fill bytes to ignore
	or	a1, t0, t0		# merge in fill bytes
	stq	t0, 0(a0)		# store the filled bytes
done:	RET

	# fill a partial quadword, i.e. neither the starting or ending
	# bytes are not on a QW boundary

	.align	3
partial:
	ldiq	t0, -1			# get mask of all 0xff bytes
	mskql	t0, a2, t0		# make into mask of len 0xff bytes
	insql	t0, a0, t0		# shift to match alignment
	bic	t2, t0, t2		# zap bytes to be filled
	and	a1, t0, a1		# zap fill bytes to ignore
	or	a1, t2, t2		# merge in fill bytes
	stq_u	t2, 0(a0)		# store the filled bytes
	RET

END(memset)
