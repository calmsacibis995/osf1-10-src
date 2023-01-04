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
	.rdata
	.asciiz "@(#)$RCSfile: in_checksum.s,v $ $Revision: 1.1.2.7 $ (DEC) $Date: 1992/08/28 16:28:10 $"
	.text

#include <machine/asm.h>
#include <machine/regdef.h>
#include "assym.s"

/*
 * in_checksum( addr, len, prevcksum )
 *
 * Calculates a 16-bit ones-complement checksum 
 */

LEAF(in_checksum)
	bis	zero, a2, v0		# copy previous checksum
	beq	a1, 9f			# count exhausted?
	and	a0, 7, t0		# check for quadword alignment
	beq	t0, 2f			# if EQ, already aligned
	subq	zero, a0, t0		# compute # of bytes
	and	t0, 7, t0		#   to alignment
	cmpult	a1, t0, t1		# compute ultimate
	cmovne	t1, a1, t0		#   number of bytes to use
	ldq_u	t1, 0(a0)		# get correct quadword
	extql	t1, a0, t1		# extract valid bytes
	mskql	t1, t0, t1		# mask to required bytes
	blbc	a0, 1f			# odd byte alignment?
	sll	t1, 8, t1		# shift by one byte
1:	addq	t1, v0, t1		# accumulate checksum
	cmpult	t1, v0, v0		# compute overflow
	addq	t0, a0, a0		# move on the pointer
	subq	a1, t0, a1		# reduce remaining byte count
	addq	v0, t1, v0		# accumulate IP checksum
	beq	a1, 8f			# count exhausted?

2:	srl	a1, 6, t1		# get # of 64 byte blocks
	beq	t1, 4f			# if EQ, none
	and	a1, 63, a1		# residual byte count

3:	ldq	t2, 0(a0)		# get next quadword
	addq	a0, 64, a0		# point to next block
	ldq	t3, -56(a0)		# load rest of first cache line
	subq	t1, 1, t1		# reduce remaining block count

	addq	t2, v0, t2		# accumulate checksum
	ldq	t4, -48(a0)
	cmpult	t2, v0, t7		# compute overflow
	ldq	t5, -40(a0)
	ldq	t6, -32(a0)		# get quadword in next cache line
	addq	t3, t2, v0		# accumulate checksum
	cmpult	v0, t2, t8		# compute overflow
	addq	t4, v0, t4		# accumulate checksum
	cmpult	t4, v0, t9		# compute overflow
	addq	t5, t4, v0		# accumulate checksum
	cmpult	v0, t4, t10		# compute overflow
	addq	t7, t8, t0		# start accumulating overflows
	ldq	t7, -24(a0)		# load rest of second cache line
	addq	t9, t0, t0		# accumulate rest
	addq	t10, t0, t0		#   of the overflows
	ldq	t8, -16(a0)
	addq	t6, v0, t6		# accumulate checksum
	cmpult	t6, v0, t2		# compute overflow
	ldq	t9, -8(a0)
	addq	t7, t6, v0		# accumulate checksum
	cmpult	v0, t6, t3		# compute overflow
	addq	t8, v0, t8		# accumulate checksum
	cmpult	t8, v0, t4		# compute overflow
	addq	t9, t8, v0		# accumulate checksum
	cmpult	v0, t8, t5		# compute checksum
	addq	t2, t0, t0		# accumulate all
	addq	t3, t0, t0		#   the overflows - no chance
	addq	t4, t0, t0		#     of an overflow here
	addq	t5, t0, t0

	addq	t0, v0, t0		# add in the overflows
	cmpult	t0, v0, v0		# compute overflow
	addq	v0, t0, v0		# accumulate IP checksum
	bne	t1, 3b			# loop till done

4:	srl	a1, 3, t1		# get # of quadwords
	beq	t1, 6f			# if EQ, none
	and	a1, 7, a1		# residual byte count

5:	ldq	t0, 0(a0)		# get next quadword
	addq	a0, 8, a0		# move on pointer
	subq	t1, 1, t1		# reduce count
	addq	t0, v0, t0		# accumulate checksum
	cmpult	t0, v0, v0		# compute overflow
	addq	v0, t0, v0		# accumulate IP checksum
	bne	t1, 5b			# loop till done

6:	beq	a1, 8f			# count exhausted?
	ldq	t0, 0(a0)		# get next quadword
	mskql	t0, a1, t0		# mask out unwanted bytes
	addq	t0, v0, t0		# accumulate checksum
	cmpult	t0, v0, v0		# compute overflow
	addq	v0, t0, v0		# accumulate IP checksum

/*
 * Now we need to fold the 64-bit checksum into 16-bits. We do this in
 * 2 stages: 64 bits -> 32 bits, 32 bits -> 16 bits.
 */
8:	srl	v0, 32, t0		# get high 32-bits of checksum
	mskll	v0, 4, v0		# mask to lower 32-bits
	addq	t0, v0, v0		# form 32-bit checksum

	srl	v0, 16, t0		# get high 16/17-bits of checksum
	mskll	v0, 2, v0		# mask to lower 16-bits
	addq	t0, v0, v0		# form 16-bit checksum

	srl	v0, 16, t0		# do it again
	mskll	v0, 2, v0		#   in case of overflow
	addq	t0, v0, v0

	srl	v0, 16, t0		# one last time
	mskll	v0, 2, v0		#   in case of overflow
	addq	t0, v0, v0

9:	ret	zero, (ra)
	END(in_checksum)


