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
	.asciiz "@(#)$RCSfile: fastcopy.s,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/27 14:18:03 $"
	.text

/*
 * Modification History: machine/fastcopy.s
 *
 * 25-Aug-92 -- tlh
 *	Created this file for Alpha support.
 */

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/thread.h>

#include "assym.s"

/*
 * extern int bcopy(char *source, char *destination, int length);
 *
 *	a0	source
 *	a1	destination
 *	a2	length
 *
 * Copy "length" characters from the string pointed to by the "source"
 * parameter into the location pointed to by the "destination" parameter.
 * Copying takes place as though the "length" number of characters from string
 * "source" are first copied into a temporary location having "length"
 * bytes that do not overlap either of the strings pointed to by "source"
 * and "destination". Then, "length" number of characters from the temporary
 * location are copied to the string pointed to by "destination". Consequently,
 * this operation is nondestructive and proceeds from left to right.
 *
 * Note that in this routine, all labels prefixed with "f_" are used
 * for forwards copies, and all labels prefixed with "b_" are used
 * for backwards copies.
 */
	.align	4
LEAF(bcopy)
	bis	zero,zero,v0		# return 0
	addl	a2, 0, a2		# ensure length is an int
	subq	a1, a0, t5		# get offset from src to dst
	cmpult	t5, a2, t5		# do src and dst overlap?
	ble	a2, done		# quit if nothing to be copied
	addq	a0, a2, a3		# get ending src pointer
	bne	t5, backwards		# overlap: do backwards copy
	ldq_u	t2, 0(a0)		# get initial src bytes
	xor	a0, a1, t0		# see how much of pointers match
	and	t0, 8-1, t0		# check for quadword match
	and	a1, 8-1, t1		# get dst alignment	
	bne	t0, f_unaligned		# go handle mismatched pointers
	bne	t1, f_initial		# go get quadword aligned

	# Forward copy where source and destination pointers have
	# the same alignment, and the next byte to copy starts on a
	# quadword boundary. Copy a quadword at a time, leaving the loop
	# with a non-zero number of bytes (1..8) remaining to be copied
	#
	#	a0	source pointer (may not be aligned)
	#	a1	destination pointer (may not be aligned)
	#	a2	length
	#	t2	quadword pointed to by a0

	.align	3
f_quads:
	subq	a2, 1, t0		# get byte count - 1
	bic	t0, 8-1, t0		# get quad count
	and	a2, 8-1, a2		# get final byte count mod 8
	beq	t0, f_final		# br if no quads to copy
10:	stq_u	t2, 0(a1)		# store next quadword
	addq	a1, 8, a1		# advance dst pointer
	ldq_u	t2, 8(a0)		# get next quadword
	subq	t0, 8, t0		# adjust quad count
	addq	a0, 8, a0		# advance src pointer
	bne	t0, 10b			# loop on quad count

	# Store the final 1..8 bytes to the destination string.
	#
	#	a1	destination pointer (may not be aligned)
	#	a2	remaining length mod 8
	#	t2	final quadword from source string

	.align	3
f_final:
	mskql	t2, a2, t4		# mask out trailing bytes
	bne	a2, 20f			# br if there are trailing bytes
	stq_u	t2, 0(a1)		# store final full quadword
	RET

20:	ldq_u	t3, 0(a1)		# get final dst quadword
	mskqh	t3, a2, t3		# zap all but trailing bytes
	or	t4, t3, t4		# merge src and dst bytes
	stq_u	t4, 0(a1)		# store merged bytes
	RET

	# Forward copy where the source and destination pointers have the
	# same alignment, but are not currently on a quadword boundary.
	# Copy the initial bytes so the pointers will be aligned.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	t1	alignment of destination pointer
	#	t2	low bytes of quadword pointed to by a0

	.align	3
f_initial:
	ldq_u	t3, 0(a1)		# get initial dst bytes
	addq	a2, t1, a2		# include initial bytes in count
	mskqh	t2, a0, t2		# mask out initial src bytes
	mskql	t3, a0, t3		# mask out all but initial dst bytes
	or	t2, t3, t2		# merge src and dst bytes
	br	zero, f_quads		# store merged quad and countinue

	# Forward copy where the source and destination pointers do not
	# have the same alignment. The strategy is to get the destination
	# pointer aligned, and then do unaligned loads from the source.
	# Beware that this code has to be quite careful to avoid reading
	# past the end of either the source or destination strings.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	t1	alignment of destination pointer
	#	t2	low bytes of quadword pointed to by a0

	.align	3
f_unaligned:
	cmpule	a2, 8, t0		# is length >= 8?
	addq	a0, a2, a3		# determine end point of src
	bne	t0, f_usmall		# if length <=8 handle specially
	bne	t1, f_uinitial		# br if dst is not QW aligned

	# Forward copy where the next byte to copy will be stored
	# on a quadword boundary. Copy a quadword at a time, leaving the loop
	# with a non-zero number of bytes (1..8) remaining to be copied
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a0

	.align	3
f_uquads:
	subq	a2, 1, t0		# get length - 1
	bic	t0, 8-1, t0		# get quadword count
	and	a2, 8-1, a2		# get final byte count mod 8
	beq	t0, f_ufinal1		# br if no full quads to copy

10:	ldq_u	t3, 7(a0)		# get high bytes of src
	addq	a0, 8, a0		# advance src pointer
	extql	t2, a0, t4		# get low bytes of src
	nop
	extqh	t3, a0, t5		# get high bytes of src
	subq	t0, 8, t0		# adjust quad count
	or	t4, t5, t5		# merge high and low src bytes
	stq	t5, 0(a1)		# store the merged bytes
	addq	a1, 8, a1		# advance dst pointer
	beq	t0, f_ufinal2		# br if no quads to copy

	ldq_u	t2, 7(a0)		# get high bytes of next src QW
	addq	a0, 8, a0		# advance src pointer
	extql	t3, a0, t4		# get low bytes of src
	nop
	extqh	t2, a0, t5		# get high bytes of src
	subq	t0, 8, t0		# adjust quad count
	or	t4, t5, t5		# merge high and low src bytes
	stq	t5, 0(a1)		# store the merged bytes
	addq	a1, 8, a1		# advance dst pointer
	bne	t0, 10b			# loop on the quad count
	#br	zero, f_ufinal1

	# Store the final 1..8 bytes to the destination string.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	remaining length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of final source quadword (f_ufinal1 only)
	#	t3	low bytes of final source quadword (f_ufinal2 only)

	.align	3
f_ufinal1:
	bis	t2, t2, t3		# copy low bytes of src
f_ufinal2:
	ldq_u	t2, -1(a3)		# get high bytes of next src QW
	extql	t3, a0, t3		# get low bytes of src
	extqh	t2, a0, t2		# get high bytes of src
	or	t2, t3, t2		# merge high and low bytes
	br	zero, f_final		# continue with aligned routine

	# Forward copy where the destination pointer is not currently on
	# a quadword boundary, and the length is >= 8. Copy the initial
	# bytes so the destination pointer will be aligned.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a0

	.align	3
f_uinitial:
	subq	zero, a1, t0		# negate dst pointer
	and	t0, 8-1, t0		# bytes till dst is QW aligned

	ldq_u	t3, 7(a0)		# get high bytes of src
	extql	t2, a0, t2		# get low bytes of initial src QW
	extqh	t3, a0, t3		# get high bytes of initial src QW
	or	t2, t3, t5		# merge high and low src bytes
	insql	t5, a1, t5		# shift src QW to match dst alignment

	ldq_u	t6, 0(a1)		# get initial dst bytes
	mskql	t6, a1, t6		# zap all but initial dst bytes
	or	t5, t6, t5		# merge src and dst bytes
	stq_u	t5, 0(a1)		# store merged bytes

	addq	a0, t0, a0		# advance src pointer
	addq	a1, t0, a1		# advance dst pointer
	subq	a2, t0, a2		# adjust length

	ldq_u	t2, 0(a0)		# get low bytes of next src QW
	br	zero, f_uquads		# copy quadwords

	# Forward copy where the source and destination pointers do not
	# have the same alignment, and the length is 8 bytes or less
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a0

	.align	3
f_usmall:
	ldq_u	t3, -1(a3)		# load high bytes of src
	extql	t2, a0, t2		# get low bytes of src
	extqh	t3, a0, t3		# get high bytes of src
	or	t2, t3, t2		# merge high and low bytes of src
	insqh	t2, a1, t3		# get high src bytes with dst alignment
	insql	t2, a1, t2		# get low src bytes with dst alignment

	ldiq	t4, -1			# get mask of 8 0xff bytes
	mskql	t4, a2, t5		# mask out bytes to ignore
	cmovne	t5, t5, t4		# fix mask for count of 8
	insqh	t4, a1, t5		# get high mask bytes with dst alignment
	insql	t4, a1, t4		# get low mask bytes with dst alignment

	addq	a1, a2, a4		# get end of dst string + 1
	ldq_u	t6, 0(a1)		# get low dst bytes
	ldq_u	t7, -1(a4)		# get high dst bytes

	bic	t6, t4, t6		# mask out low dst bytes to ignore
	bic	t7, t5, t7		# mask out high dst bytes to ignore
	and	t2, t4, t2		# mask out low src bytes to ignore
	and	t3, t5, t3		# mask out high src bytes to ignore
	or	t2, t6, t2		# merge low bytes
	or	t3, t7, t3		# merge high bytes

	stq_u	t3, -1(a4)		# store merged high bytes
	stq_u	t2, 0(a1)		# store merged low bytes
	RET

	# The source and the destination overlap in such a way that
	# performing a normal forward copy would not produce the
	# desired results. So, we copy the string backwards. Since
	# this doesn't occur often, and the penalty is not that great,
	# this code always assumes the worst case that the source
	# and the destination pointers do not have the same alignment.
	# Beware that this code has to be quite careful to avoid
	# reading past the end of either the source or destination strings.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length

	.align	3
backwards:
	cmpule	a2, 8, t0		# is length >= 8?
	addq	a0, a2, a3		# point past end of src
	addq	a1, a2, a4		# point past end of dst
	bne	t0, b_small		# br if length <= 8
	and	a4, 7, t4		# get bytes till dst is QW aligned
	bne	t4, b_initial		# br if not already aligned

	# Backward copy where the next byte to copy will be stored
	# on a quadword boundary. Copy a quadword at a time. Unlike
	# the forward copy we can exit this loop with no bytes to
	# copy.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	source pointer (from end)
	#	a4	destination pointer (from end)

	.align	3
b_quads:
	bic	a2, 7, t4		# get QWs * 8 to move
	and	a2, 7, a2		# adjust length
	nop				# improve scheduling
	beq	t4, b_final		# skip if none

10:	ldq_u	t0, -8(a3)		# get low bytes of src QW
	subq	a4, 8, a4		# adjust dst pointer
	ldq_u	t1, 7-8(a3)		# get high bytes of src QW
	subq	a3, 8, a3		# adjust src pointer
	extql	t0, a3, t0		# extract low bytes
	extqh	t1, a3, t1		# extract high bytes
	subq	t4, 8, t4		# adjust length
	or	t0, t1, t0		# merge low and high bytes
	stq	t0, 0(a4)		# store the merge bytes
	bne	t4, 10b			# loop on QW count

	# Store the final 0..7 bytes to the destination string.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	remaining length

	.align	3
b_final:
	bne	a2, 20f			# skip if any bytes remain
	RET

20:	ldq_u	t0, 0(a0)		# get low bytes of src
	ldq_u	t1, 7(a0)		# get high bytes of src
	ldq_u	t2, 0(a1)		# get dst QW
	extql	t0, a0, t0		# extract low bytes
	extqh	t1, a0, t1		# extract high bytes
	or	t0, t1, t0		# merge low and high bytes
	insql	t0, a1, t0		# position to match dst alignment
	mskql	t2, a1, t2		# clear ignored bytes from dst
	or	t2, t0, t2		# merge in src bytes
	stq_u	t2, 0(a1)		# store merged bytes
	RET

	# Backward copy where the destination pointer is not currently on
	# a quadword boundary. Copy the initial bytes so the destination
	# pointer will be aligned.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	source pointer (from end)
	#	a4	destination pointer (from end)
	#	t4	bytes till dst is QW aligned

	.align	3
b_initial:
	subq	a3, t4, a3		# adjust src pointer
	subq	a4, t4, a4		# adjust dst pointer
	ldq_u	t1, 0(a3)		# get low bytes of src
	subq	a2, t4, a2		# adjust length
	ldq_u	t2, 7(a3)		# get high bytes of src
	ldq	t3, 0(a4)		# get dst QW
	extql	t1, a3, t1		# extract low bytes of src
	extqh	t2, a3, t2		# extract high bytes of src
	or	t1, t2, t1		# merge low and high bytes

	mskqh	t3, t4, t3		# clear bytes to ignore in dst
	mskql	t1, t4, t1		# clear bytes to ignore in src
	or	t1, t3, t1		# merge src and dst bytes
	stq	t1, 0(a4)		# store merged bytes
	br	zero, b_quads

	# Backward copy where the length is 8 bytes or less.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	source pointer (from end)
	#	a4	destination pointer (from end)

b_small:
	ldq_u	t2, 0(a0)		# get low bytes of src
	br	zero, f_usmall		# handle like forward case

done:	RET

END(bcopy)

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

/*
 * extern int strlen(char *s);
 *
 *	a0	string address (s)
 *
 * Return the number of bytes in the string pointed to by the "s" parameter.
 * The string length value does not include the terminating null character.
 */
	.align	4
LEAF(strlen)
	ldq_u	t0, 0(a0)		# get first quad
	ldiq	t1, -1			# get a quad of all 0xff bytes
	insqh	t1, a0, t1		# build mask of bytes to ignore
	bic	a0, 7, v0		# get aligned string pointer
	bis	t0, t1, t0		# set ignored bytes of first QW to 0xff
	cmpbge	zero, t0, t1		# any null bytes?
	bne	t1, 20f			# br if so

	.align	3
10:	ldq	t0, 8(v0)		# get next 8 bytes
	addq	v0, 8, v0		# advance string pointer
	cmpbge	zero, t0, t1		# any null bytes?
	beq	t1, 10b			# keep looping if not

	.align	3
20:	subq	zero, t1, t2		# complement bits to left of first 1-bit
	blbs	t1, 30f			# exit if null in first byte

	and	t1, t2, t1		# remove all but first 1-bit

	and	t1, 0x0f, t2		# null in bytes 0..3?
	addq	v0, 4, t0		# adjust pointer assuming no
	cmoveq	t2, t0, v0		# if no, replace pointer
	and	t1, 0x33, t3		# null in bytes 0..1, or 4..5?
	addq	v0, 2, t0		# adjust pointer assuming no
	cmoveq	t3, t0, v0		# if no, replace pointer
	and	t1, 0x55, t4		# null in bytes 0, 2, 4 of 6
	addq	v0, 1, t0		# adjust pointer assuming no
	cmoveq	t4, t0, v0		# if no, replace pointer

30:	subq	v0, a0, v0		# get length of string
	RET
END(strlen)


#define NBPC 64		/* number of bytes per chunk to zero */

/*
 * extern int bzero(char *string, int length);
 *
 *	a0	string
 *	a1	length
 *
 * Zero "length" bytes starting at the location specified by "string"
 * parameter.
 */
	.align	4
LEAF(bzero)
	bis	zero,zero,v0		# return 0
	addl	a1, 0, a1		# ensure int length
	subq	zero, a0, t0		# negate dest pointer
	and	t0, 8-1, t0		# bytes till dest is aligned
	ble	a1, zdone		# quit if nothing to copy
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
	beq	t3, zquads		# skip if no chunks to copy
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
zquads:	and	a1, 8-1, t5		# number of bytes to zero
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
zdone:	RET

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

/*
 * extern int kdebug_bcopy(char *source, char *destination, int length);
 *
 *	a0	source
 *	a1	destination
 *	a2	length
 *
 * Copy "length" characters from the string pointed to by the "source"
 * parameter into the location pointed to by the "destination" parameter.
 * Copying takes place as though the "length" number of characters from string
 * "source" are first copied into a temporary location having "length"
 * bytes that do not overlap either of the strings pointed to by "source"
 * and "destination". Then, "length" number of characters from the temporary
 * location are copied to the string pointed to by "destination". Consequently,
 * this operation is nondestructive and proceeds from left to right.
 *
 * Note that in this routine, all labels prefixed with "f_" are used
 * for forwards copies, and all labels prefixed with "b_" are used
 * for backwards copies.
 */
	.align	4
LEAF(kdebug_bcopy)
	bis	zero,zero,v0		# return 0
	addl	a2, 0, a2		# ensure length is an int
	subq	a1, a0, t5		# get offset from src to dst
	cmpult	t5, a2, t5		# do src and dst overlap?
	ble	a2, kdebug_done		# quit if nothing to be copied
	addq	a0, a2, a3		# get ending src pointer
	bne	t5, kdebug_backwards	# overlap: do backwards copy
	ldq_u	t2, 0(a0)		# get initial src bytes
	xor	a0, a1, t0		# see how much of pointers match
	and	t0, 8-1, t0		# check for quadword match
	and	a1, 8-1, t1		# get dst alignment	
	bne	t0, kdebug_f_unaligned	# go handle mismatched pointers
	bne	t1, kdebug_f_initial	# go get quadword aligned

	# Forward copy where source and destination pointers have
	# the same alignment, and the next byte to copy starts on a
	# quadword boundary. Copy a quadword at a time, leaving the loop
	# with a non-zero number of bytes (1..8) remaining to be copied
	#
	#	a0	source pointer (may not be aligned)
	#	a1	destination pointer (may not be aligned)
	#	a2	length
	#	t2	quadword pointed to by a0

	.align	3
kdebug_f_quads:
	subq	a2, 1, t0		# get byte count - 1
	bic	t0, 8-1, t0		# get quad count
	and	a2, 8-1, a2		# get final byte count mod 8
	beq	t0, kdebug_f_final	# br if no quads to copy
10:	stq_u	t2, 0(a1)		# store next quadword
	addq	a1, 8, a1		# advance dst pointer
	ldq_u	t2, 8(a0)		# get next quadword
	subq	t0, 8, t0		# adjust quad count
	addq	a0, 8, a0		# advance src pointer
	bne	t0, 10b			# loop on quad count

	# Store the final 1..8 bytes to the destination string.
	#
	#	a1	destination pointer (may not be aligned)
	#	a2	remaining length mod 8
	#	t2	final quadword from source string

	.align	3
kdebug_f_final:
	mskql	t2, a2, t4		# mask out trailing bytes
	bne	a2, 20f			# br if there are trailing bytes
	stq_u	t2, 0(a1)		# store final full quadword
	RET

20:	ldq_u	t3, 0(a1)		# get final dst quadword
	mskqh	t3, a2, t3		# zap all but trailing bytes
	or	t4, t3, t4		# merge src and dst bytes
	stq_u	t4, 0(a1)		# store merged bytes
	RET

	# Forward copy where the source and destination pointers have the
	# same alignment, but are not currently on a quadword boundary.
	# Copy the initial bytes so the pointers will be aligned.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	t1	alignment of destination pointer
	#	t2	low bytes of quadword pointed to by a0

	.align	3
kdebug_f_initial:
	ldq_u	t3, 0(a1)		# get initial dst bytes
	addq	a2, t1, a2		# include initial bytes in count
	mskqh	t2, a0, t2		# mask out initial src bytes
	mskql	t3, a0, t3		# mask out all but initial dst bytes
	or	t2, t3, t2		# merge src and dst bytes
	br	zero, kdebug_f_quads	# store merged quad and countinue

	# Forward copy where the source and destination pointers do not
	# have the same alignment. The strategy is to get the destination
	# pointer aligned, and then do unaligned loads from the source.
	# Beware that this code has to be quite careful to avoid reading
	# past the end of either the source or destination strings.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	t1	alignment of destination pointer
	#	t2	low bytes of quadword pointed to by a0

	.align	3
kdebug_f_unaligned:
	cmpule	a2, 8, t0		# is length >= 8?
	addq	a0, a2, a3		# determine end point of src
	bne	t0, kdebug_f_usmall	# if length <=8 handle specially
	bne	t1, kdebug_f_uinitial	# br if dst is not QW aligned

	# Forward copy where the next byte to copy will be stored
	# on a quadword boundary. Copy a quadword at a time, leaving the loop
	# with a non-zero number of bytes (1..8) remaining to be copied
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a0

	.align	3
kdebug_f_uquads:
	subq	a2, 1, t0		# get length - 1
	bic	t0, 8-1, t0		# get quadword count
	and	a2, 8-1, a2		# get final byte count mod 8
	beq	t0, kdebug_f_ufinal1	# br if no full quads to copy

10:	ldq_u	t3, 7(a0)		# get high bytes of src
	addq	a0, 8, a0		# advance src pointer
	extql	t2, a0, t4		# get low bytes of src
	nop
	extqh	t3, a0, t5		# get high bytes of src
	subq	t0, 8, t0		# adjust quad count
	or	t4, t5, t5		# merge high and low src bytes
	stq	t5, 0(a1)		# store the merged bytes
	addq	a1, 8, a1		# advance dst pointer
	beq	t0, kdebug_f_ufinal2		# br if no quads to copy

	ldq_u	t2, 7(a0)		# get high bytes of next src QW
	addq	a0, 8, a0		# advance src pointer
	extql	t3, a0, t4		# get low bytes of src
	nop
	extqh	t2, a0, t5		# get high bytes of src
	subq	t0, 8, t0		# adjust quad count
	or	t4, t5, t5		# merge high and low src bytes
	stq	t5, 0(a1)		# store the merged bytes
	addq	a1, 8, a1		# advance dst pointer
	bne	t0, 10b			# loop on the quad count
	#br	zero, kdebug_f_ufinal1

	# Store the final 1..8 bytes to the destination string.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	remaining length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of final source quadword (f_ufinal1 only)
	#	t3	low bytes of final source quadword (f_ufinal2 only)

	.align	3
kdebug_f_ufinal1:
	bis	t2, t2, t3		# copy low bytes of src
kdebug_f_ufinal2:
	ldq_u	t2, -1(a3)		# get high bytes of next src QW
	extql	t3, a0, t3		# get low bytes of src
	extqh	t2, a0, t2		# get high bytes of src
	or	t2, t3, t2		# merge high and low bytes
	br	zero, kdebug_f_final	# continue with aligned routine

	# Forward copy where the destination pointer is not currently on
	# a quadword boundary, and the length is >= 8. Copy the initial
	# bytes so the destination pointer will be aligned.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a0

	.align	3
kdebug_f_uinitial:
	subq	zero, a1, t0		# negate dst pointer
	and	t0, 8-1, t0		# bytes till dst is QW aligned

	ldq_u	t3, 7(a0)		# get high bytes of src
	extql	t2, a0, t2		# get low bytes of initial src QW
	extqh	t3, a0, t3		# get high bytes of initial src QW
	or	t2, t3, t5		# merge high and low src bytes
	insql	t5, a1, t5		# shift src QW to match dst alignment

	ldq_u	t6, 0(a1)		# get initial dst bytes
	mskql	t6, a1, t6		# zap all but initial dst bytes
	or	t5, t6, t5		# merge src and dst bytes
	stq_u	t5, 0(a1)		# store merged bytes

	addq	a0, t0, a0		# advance src pointer
	addq	a1, t0, a1		# advance dst pointer
	subq	a2, t0, a2		# adjust length

	ldq_u	t2, 0(a0)		# get low bytes of next src QW
	br	zero, kdebug_f_uquads	# copy quadwords

	# Forward copy where the source and destination pointers do not
	# have the same alignment, and the length is 8 bytes or less
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a0

	.align	3
kdebug_f_usmall:
	ldq_u	t3, -1(a3)		# load high bytes of src
	extql	t2, a0, t2		# get low bytes of src
	extqh	t3, a0, t3		# get high bytes of src
	or	t2, t3, t2		# merge high and low bytes of src
	insqh	t2, a1, t3		# get high src bytes with dst alignment
	insql	t2, a1, t2		# get low src bytes with dst alignment

	ldiq	t4, -1			# get mask of 8 0xff bytes
	mskql	t4, a2, t5		# mask out bytes to ignore
	cmovne	t5, t5, t4		# fix mask for count of 8
	insqh	t4, a1, t5		# get high mask bytes with dst alignment
	insql	t4, a1, t4		# get low mask bytes with dst alignment

	addq	a1, a2, a4		# get end of dst string + 1
	ldq_u	t6, 0(a1)		# get low dst bytes
	ldq_u	t7, -1(a4)		# get high dst bytes

	bic	t6, t4, t6		# mask out low dst bytes to ignore
	bic	t7, t5, t7		# mask out high dst bytes to ignore
	and	t2, t4, t2		# mask out low src bytes to ignore
	and	t3, t5, t3		# mask out high src bytes to ignore
	or	t2, t6, t2		# merge low bytes
	or	t3, t7, t3		# merge high bytes

	stq_u	t3, -1(a4)		# store merged high bytes
	stq_u	t2, 0(a1)		# store merged low bytes
	RET

	# The source and the destination overlap in such a way that
	# performing a normal forward copy would not produce the
	# desired results. So, we copy the string backwards. Since
	# this doesn't occur often, and the penalty is not that great,
	# this code always assumes the worst case that the source
	# and the destination pointers do not have the same alignment.
	# Beware that this code has to be quite careful to avoid
	# reading past the end of either the source or destination strings.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length

	.align	3
kdebug_backwards:
	cmpule	a2, 8, t0		# is length >= 8?
	addq	a0, a2, a3		# point past end of src
	addq	a1, a2, a4		# point past end of dst
	bne	t0, kdebug_b_small	# br if length <= 8
	and	a4, 7, t4		# get bytes till dst is QW aligned
	bne	t4, kdebug_b_initial	# br if not already aligned

	# Backward copy where the next byte to copy will be stored
	# on a quadword boundary. Copy a quadword at a time. Unlike
	# the forward copy we can exit this loop with no bytes to
	# copy.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	source pointer (from end)
	#	a4	destination pointer (from end)

	.align	3
kdebug_b_quads:
	bic	a2, 7, t4		# get QWs * 8 to move
	and	a2, 7, a2		# adjust length
	nop				# improve scheduling
	beq	t4, kdebug_b_final	# skip if none

10:	ldq_u	t0, -8(a3)		# get low bytes of src QW
	subq	a4, 8, a4		# adjust dst pointer
	ldq_u	t1, 7-8(a3)		# get high bytes of src QW
	subq	a3, 8, a3		# adjust src pointer
	extql	t0, a3, t0		# extract low bytes
	extqh	t1, a3, t1		# extract high bytes
	subq	t4, 8, t4		# adjust length
	or	t0, t1, t0		# merge low and high bytes
	stq	t0, 0(a4)		# store the merge bytes
	bne	t4, 10b			# loop on QW count

	# Store the final 0..7 bytes to the destination string.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	remaining length

	.align	3
kdebug_b_final:
	bne	a2, 20f			# skip if any bytes remain
	RET

20:	ldq_u	t0, 0(a0)		# get low bytes of src
	ldq_u	t1, 7(a0)		# get high bytes of src
	ldq_u	t2, 0(a1)		# get dst QW
	extql	t0, a0, t0		# extract low bytes
	extqh	t1, a0, t1		# extract high bytes
	or	t0, t1, t0		# merge low and high bytes
	insql	t0, a1, t0		# position to match dst alignment
	mskql	t2, a1, t2		# clear ignored bytes from dst
	or	t2, t0, t2		# merge in src bytes
	stq_u	t2, 0(a1)		# store merged bytes
	RET

	# Backward copy where the destination pointer is not currently on
	# a quadword boundary. Copy the initial bytes so the destination
	# pointer will be aligned.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	source pointer (from end)
	#	a4	destination pointer (from end)
	#	t4	bytes till dst is QW aligned

	.align	3
kdebug_b_initial:
	subq	a3, t4, a3		# adjust src pointer
	subq	a4, t4, a4		# adjust dst pointer
	ldq_u	t1, 0(a3)		# get low bytes of src
	subq	a2, t4, a2		# adjust length
	ldq_u	t2, 7(a3)		# get high bytes of src
	ldq	t3, 0(a4)		# get dst QW
	extql	t1, a3, t1		# extract low bytes of src
	extqh	t2, a3, t2		# extract high bytes of src
	or	t1, t2, t1		# merge low and high bytes

	mskqh	t3, t4, t3		# clear bytes to ignore in dst
	mskql	t1, t4, t1		# clear bytes to ignore in src
	or	t1, t3, t1		# merge src and dst bytes
	stq	t1, 0(a4)		# store merged bytes
	br	zero, kdebug_b_quads

	# Backward copy where the length is 8 bytes or less.
	#
	#	a0	source pointer
	#	a1	destination pointer
	#	a2	length
	#	a3	source pointer (from end)
	#	a4	destination pointer (from end)

kdebug_b_small:
	ldq_u	t2, 0(a0)		# get low bytes of src
	br	zero, kdebug_f_usmall	# handle like forward case

kdebug_done:	RET

END(kdebug_bcopy)
