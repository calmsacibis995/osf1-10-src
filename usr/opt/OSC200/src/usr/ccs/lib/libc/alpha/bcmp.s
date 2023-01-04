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
 * @(#)$RCSfile: bcmp.s,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/01/19 14:04:41 $
 */

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern int bcmp(char *string1, char *string2, int length);
 *
 *	a0	string1
 *	a1	string2
 *	a2	length
 *
 * Compare the byte string pointed to by the "string1" parameter against
 * the string pointed to by the "string2" parameter. The "length"
 * parameter indicates how many bytes to compare.
 *
 * A zero is returned if the two strings match; otherwise a non-zero
 * value is returned.
 */
	.align	4
LEAF(bcmp)
	.prologue 0

	addl	a2, 0, a2		# ensure int length
	xor	a0, a1, t0		# see where src1 and src2 ptrs differ
	and	t0, 8-1, t0		# are low 3 bits the same?
	ble	a2, match		# quit now if nothing to compare
	ldq_u	t2, 0(a0)		# get initial bytes of src1
	and	a1, 8-1, t1		# get src2 alignment
	ldq_u	t3, 0(a1)		# get initial bytes of src2
	bne	t0, unaligned		# if not aligned handle specially
	bne	t1, initial		# compare up to QW boundary

	# The next byte to compare of both strings are on a quadword
	# boundary (although the pointers may not be quadword aligned),
	# so do the compare a quadword at a time. The number of quads
	# to compare has been calculated so that there is a non-zero number
	# of bytes to compare when the loop exits
	#
	#	a0	string1 pointer
	#	a1	string2 pointer
	#	a2	byte count
	#	t2	quadword pointed to by a0
	#	t3	quadword pointed to by a1

	.align	3
quads:	subq	a2, 1, t5		# get byte count - 1
	bic	t5, 8-1, t5		# get count of quads to compare
	subq	a2, t5, a2		# get bytes remaining after this loop
	beq	t5, final		# skip if no whole quads to compare
10:	xor	t2, t3, t0		# check for mismatches
	ldq_u	t2, 8(a0)		# get next 8 src1 bytes
	addq	a0, 8, a0		# advance src1 pointer
	ldq_u	t3, 8(a1)		# get next 8 src2 bytes
	subq	t5, 8, t5		# adjust byte count
	bne	t0, nomatch		# quit if a non-match found
	addq	a1, 8, a1		# advance src2 pointer
	bne	t5, 10b			# loop on quad count

	# Compare the bytes (1..8) of the final quadword.
	#
	#	a2	remaining byte count
	#	t2	last quadword of string1
	#	t3	last quadword of string2

	.align	3
final:	xor	t2, t3, v0		# check whole quad for matches
	extqh	v0, a2, v0		# ignore mismatches after end
	cmovne	v0, 1, v0		# make sure we return an int
	RET

	# Both strings have the same alignment, but they are not yet
	# quadword aligned. Compare the initial bytes to get us aligned.
	# We do the comparison by setting the initial bytes of both
	# quadwords to zero (so they compare equal), adjust the length
	# to include the initial bytes and then treat as if both
	# pointers were aligned.
	#
	#	a0	string1 pointer
	#	a1	string2 pointer
	#	a2	byte count
	#	t1	string2 alignment
	#	t2	low bytes of quadword pointed to by a0
	#	t3	low bytes of quadword pointed to by a1

	.align	3
initial:
	mskqh	t2, a0, t2		# zap bytes before src1
	mskqh	t3, a0, t3		# zap bytes before src2
	addq	a2, t1, a2		# bump count to include start of QW
	br	zero, quads		# go compare 'em

	# The two strings have different alignment. We handle this case
	# by getting the second string quadword aligned so we only have
	# to deal with one unaligned string. Beware that this code has to
	# be quite careful to avoid reading past the end of either the
	# source or destination strings.
	#
	#	a0	string1 pointer
	#	a1	string2 pointer
	#	a2	byte count
	#	t1	string2 alignment
	#	t2	low bytes of quadword pointed to by a0
	#	t3	low bytes of quadword pointed to by a1

	.align	3
unaligned:
	cmpule	a2, 8, t0		# is length <= 8?
	addq	a0, a2, a3		# pointer to just past end of src1
	bne	t0, usmall		# br if length <= 8
	bne	t1, uinitial		# br if src2 is not QW aligned

	# The next byte of string2 is on a quadword boundary (although the
	# pointer may not be quadword aligned), so do the compare a quadword
	# at a time but we are guaranteed that string1 is unaligned. The
	# number of quads to compare has been calculated so that there is
	# a non-zero number of bytes to compare when the loop exits
	#
	#	a0	string1 pointer
	#	a1	string2 pointer
	#	a2	byte count
	#	t2	low bytes of quadword pointed to by a0
	#	t3	quadword pointed to by a1

	.align	3
uquads:	subq	a2, 1, t5		# get byte count - 1
	bic	t5, 8-1, t5		# get count of quads to compare
	subq	a2, t5, a2		# get final byte count
	beq	t5, ufinal1		# skip if none

10:	ldq_u	t4, 7(a0)		# get high bytes of src1
	addq	a0, 8, a0		# advance src1 pointer
	extql	t2, a0, t0		# extract low bytes of src1
	extqh	t4, a0, t1		# extract high bytes of src1
	nop
	or	t0, t1, t0		# merge high and low bytes
	xor	t0, t3, t0		# see if src1 and src2 differ
	ldq	t3, 8(a1)		# get next 8 src2 bytes
	subq	t5, 8, t5		# adjust byte count
	bne	t0, nomatch		# quit if a non-match found
	addq	a1, 8, a1		# advance src2 pointer
	beq	t5, ufinal2		# get out if no more quads

	ldq_u	t2, 7(a0)		# get high bytes of src1
	addq	a0, 8, a0		# advance src1 pointer
	extql	t4, a0, t0		# extract low bytes of src1
	extqh	t2, a0, t1		# extract high bytes of src1
	nop
	or	t0, t1, t0		# merge high and low bytes
	xor	t0, t3, t0		# see if src1 and src2 differ
	ldq	t3, 8(a1)		# get next 8 src2 bytes
	subq	t5, 8, t5		# adjust byte count
	bne	t0, nomatch		# quit if a non-match found
	addq	a1, 8, a1		# advance src2 pointer
	bne	t5, 10b			# loop on quad count

	# Compare the bytes (1..8) of the final quadword.
	#
	#	a0	string1 pointer
	#	a2	remaining byte count
	#	t2	low bytes of last quadword of string1 (ufinal1 only)
	#	t3	last quadword of string2
	#	t4	low bytes of last quadword of string1 (ufinal2 only)

	.align	3
ufinal1:
	bis	t2, t2, t4		# copy low bytes of last quad
ufinal2:
	ldq_u	t2, -1(a3)		# get high bytes of src1
	extql	t4, a0, t4		# extract low bytes of src1
	extqh	t2, a0, t2		# extract high bytes of src1
	nop
	or	t2, t4, t2		# merge high and low bytes
	xor	t2, t3, v0		# set v0 to 0 if we match NE otherwise
	extqh	v0, a2, v0		# zap bytes beyond end of strings
	cmovne	v0, 1, v0		# make sure we return an int
	RET

	# string2 is not quadword aligned. Compare the initial bytes to
	# get string2 aligned.
	#
	#	a0	string1 pointer
	#	a1	string2 pointer
	#	a2	byte count
	#	t2	low bytes of quadword pointed to by a0
	#	t3	low bytes of quadword pointed to by a1

	.align	3
uinitial:
	subq	zero, a1, t0		# negate src2 pointer
	and	t0, 8-1, t0		# get bytes till src2 QW aligned
	ldq_u	t4, 7(a0)		# get high bytes of src1
	extql	t2, a0, t2		# get low bytes of src1
	extqh	t4, a0, t4		# get high bytes of src1
	or	t2, t4, t2		# merge high and low bytes

	insql	t2, a1, t2		# shift src1 to match src2 alignment
	mskqh	t3, a1, t3		# zap src2 bytes to ignore
	xor	t2, t3, t2		# see if src1 and src2 differ
	ldq_u	t3, 8(a1)		# get next src2 bytes
	addq	a0, t0, a0		# advance src1 pointer
	bne	t2, nomatch		# quit if a mismatch

	addq	a1, t0, a1		# advance src2 pointer
	subq	a2, t0, a2		# adjust length

	ldq_u	t2, 0(a0)		# get next src1 bytes
	br	zero, uquads		# go compare full quadwords

	# The two strings have different alignment and the length
	# is 8 bytes or less.
	#
	#	a0	string1 pointer
	#	a1	string2 pointer
	#	a2	byte count
	#	a3	pointer to one byte beyond end of string1
	#	t2	low bytes of quadword pointed to by a0
	#	t3	low bytes of quadword pointed to by a1

	.align	3
usmall:	ldq_u	t4, -1(a3)		# load high bytes of src1
	extql	t2, a0, t2		# get low bytes of src1
	extqh	t4, a0, t4		# get high bytes of src1
	or	t2, t4, t2		# merge high and low bytes of src1

	addq	a1, a2, a4		# get ending src2 pointer
	ldq_u	t5, -1(a4)		# load high bytes of src2
	extql	t3, a1, t3		# get low bytes of src2
	extqh	t5, a1, t5		# get high bytes of src2
	or	t3, t5, t3		# merge high and low bytes of src2

	xor	t2, t3, t2		# see if src1 and src2 differ
	extqh	t2, a2, v0		# force match in bytes beyond end
	nop
	cmovne	v0, 1, v0		# if NE, set to an int non-zero value
	RET

	# A mismatch was found; return a non-zero value.

	.align	3
nomatch:
	ldiq	v0, 1
	RET

	# The whole string was compared and now mismatches were found;
	# return zero.

	.align	3
match:
	ldiq	v0, 0
	RET

END(bcmp)
