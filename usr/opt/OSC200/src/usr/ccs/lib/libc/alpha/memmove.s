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
 * @(#)$RCSfile: memmove.s,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/08 01:24:37 $
 */
#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern void *memmove(void *s1, const void *s2, size_t n);
 *
 *	a0	destination string pointer (s1)
 *	a1	source string pointer (s2)
 *	a2	length in bytes (n)
 *
 * Copy "n" characters from the string pointed to by the "s2" parameter
 * into the location pointed to by the "s1" parameter. Copying takes
 * place as though the "n" number of characters from string "s2"
 * are first copied into a temporary location having "n" bytes that
 * do not overlap either of the strings pointed to by "s1" and "s2".
 * Then, "n" number of characters from the temporary location are copied
 * to the string pointed to by "s1". Consequently, this operation
 * is nondestructive and proceeds from left to right.
 *
 * The return value is a pointer to the string pointed to by the
 * "s1" parameter.
 *
 * Note that in this routine, all labels prefixed with "f_" are used
 * for forwards copies, and all labels prefixed with "b_" are used
 * for backwards copies.
 */
	.align	4
LEAF(memmove)
	.prologue 0

	bis	a0, a0, v0		# set return value
	beq	a2, done		# quit if nothing to be copied
	subq	a0, a1, t5		# get offset from src to dst
	cmpult	t5, a2, t5		# do src and dst overlap?
	addq	a1, a2, a4		# get ending src pointer
	bne	t5, backwards		# overlap: do backwards copy
	ldq_u	t2, 0(a1)		# get initial src bytes
	xor	a1, a0, t0		# see how much of pointers match
	and	t0, 8-1, t0		# check for quadword match
	and	a0, 8-1, t1		# get dst alignment	
	bne	t0, f_unaligned		# go handle mismatched pointers
	bne	t1, f_initial		# go get quadword aligned

	# Forward copy where source and destination pointers have
	# the same alignment, and the next byte to copy starts on a
	# quadword boundary. Copy a quadword at a time, leaving the loop
	# with a non-zero number of bytes (1..8) remaining to be copied
	#
	#	a0	destination pointer (may not be aligned)
	#	a1	source pointer (may not be aligned)
	#	a2	length
	#	t2	quadword pointed to by a1

	.align	3
f_quads:
	subq	a2, 1, t0		# get byte count - 1
	bic	t0, 8-1, t0		# get quad count
	and	a2, 8-1, a2		# get final byte count mod 8
	beq	t0, f_final		# br if no quads to copy
10:	stq_u	t2, 0(a0)		# store next quadword
	addq	a0, 8, a0		# advance dst pointer
	ldq_u	t2, 8(a1)		# get next quadword
	subq	t0, 8, t0		# adjust quad count
	addq	a1, 8, a1		# advance src pointer
	bne	t0, 10b			# loop on quad count

	# Store the final 1..8 bytes to the destination string.
	#
	#	a0	destination pointer (may not be aligned)
	#	a2	remaining length mod 8
	#	t2	final quadword from source string

	.align	3
f_final:
	mskql	t2, a2, t4		# mask out trailing bytes
	bne	a2, 20f			# br if there are trailing bytes
	stq_u	t2, 0(a0)		# store final full quadword
	RET

20:	ldq_u	t3, 0(a0)		# get final dst quadword
	mskqh	t3, a2, t3		# zap all but trailing bytes
	or	t4, t3, t4		# merge src and dst bytes
	stq_u	t4, 0(a0)		# store merged bytes
	RET

	# Forward copy where the source and destination pointers have the
	# same alignment, but are not currently on a quadword boundary.
	# Copy the initial bytes so the pointers will be aligned.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	t1	alignment of destination pointer
	#	t2	low bytes of quadword pointed to by a1

	.align	3
f_initial:
	ldq_u	t3, 0(a0)		# get initial dst bytes
	addq	a2, t1, a2		# include initial bytes in count
	mskqh	t2, a1, t2		# mask out initial src bytes
	mskql	t3, a1, t3		# mask out all but initial dst bytes
	or	t2, t3, t2		# merge src and dst bytes
	br	zero, f_quads		# store merged quad and countinue

	# Forward copy where the source and destination pointers do not
	# have the same alignment. The strategy is to get the destination
	# pointer aligned, and then do unaligned loads from the source.
	# Beware that this code has to be quite careful to avoid reading
	# past the end of either the source or destination strings.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	t1	alignment of destination pointer
	#	t2	low bytes of quadword pointed to by a1

	.align	3
f_unaligned:
	cmpule	a2, 8, t0		# is length >= 8?
	addq	a1, a2, a4		# determine end point of src
	bne	t0, f_usmall		# if length <=8 handle specially
	bne	t1, f_uinitial		# br if dst is not QW aligned

	# Forward copy where the next byte to copy will be stored
	# on a quadword boundary. Copy a quadword at a time, leaving the loop
	# with a non-zero number of bytes (1..8) remaining to be copied
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	a4	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a1

	.align	3
f_uquads:
	subq	a2, 1, t0		# get length - 1
	bic	t0, 8-1, t0		# get quadword count
	and	a2, 8-1, a2		# get final byte count mod 8
	beq	t0, f_ufinal1		# br if no full quads to copy

10:	ldq_u	t3, 7(a1)		# get high bytes of src
	addq	a1, 8, a1		# advance src pointer
	extql	t2, a1, t4		# get low bytes of src
	nop
	extqh	t3, a1, t5		# get high bytes of src
	subq	t0, 8, t0		# adjust quad count
	or	t4, t5, t5		# merge high and low src bytes
	stq	t5, 0(a0)		# store the merged bytes
	addq	a0, 8, a0		# advance dst pointer
	beq	t0, f_ufinal2		# br if no quads to copy

	ldq_u	t2, 7(a1)		# get high bytes of next src QW
	addq	a1, 8, a1		# advance src pointer
	extql	t3, a1, t4		# get low bytes of src
	nop
	extqh	t2, a1, t5		# get high bytes of src
	subq	t0, 8, t0		# adjust quad count
	or	t4, t5, t5		# merge high and low src bytes
	stq	t5, 0(a0)		# store the merged bytes
	addq	a0, 8, a0		# advance dst pointer
	bne	t0, 10b			# loop on the quad count
	#br	zero, f_ufinal1

	# Store the final 1..8 bytes to the destination string.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	remaining length
	#	a4	pointer to first byte beyond source string
	#	t2	low bytes of final source quadword (f_ufinal1 only)
	#	t3	low bytes of final source quadword (f_ufinal2 only)

	.align	3
f_ufinal1:
	bis	t2, t2, t3		# copy low bytes of src
f_ufinal2:
	ldq_u	t2, -1(a4)		# get high bytes of next src QW
	extql	t3, a1, t3		# get low bytes of src
	extqh	t2, a1, t2		# get high bytes of src
	or	t2, t3, t2		# merge high and low bytes
	br	zero, f_final		# continue with aligned routine

	# Forward copy where the destination pointer is not currently on
	# a quadword boundary, and the length is >= 8. Copy the initial
	# bytes so the destination pointer will be aligned.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	a4	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a1

	.align	3
f_uinitial:
	subq	zero, a0, t0		# negate dst pointer
	and	t0, 8-1, t0		# bytes till dst is QW aligned

	ldq_u	t3, 7(a1)		# get high bytes of src
	extql	t2, a1, t2		# get low bytes of initial src QW
	extqh	t3, a1, t3		# get high bytes of initial src QW
	or	t2, t3, t5		# merge high and low src bytes
	insql	t5, a0, t5		# shift src QW to match dst alignment

	ldq_u	t6, 0(a0)		# get initial dst bytes
	mskql	t6, a0, t6		# zap all but initial dst bytes
	or	t5, t6, t5		# merge src and dst bytes
	stq_u	t5, 0(a0)		# store merged bytes

	addq	a1, t0, a1		# advance src pointer
	addq	a0, t0, a0		# advance dst pointer
	subq	a2, t0, a2		# adjust length

	ldq_u	t2, 0(a1)		# get low bytes of next src QW
	br	zero, f_uquads		# copy quadwords

	# Forward copy where the source and destination pointers do not
	# have the same alignment, and the length is 8 bytes or less
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	a4	pointer to first byte beyond source string
	#	t2	low bytes of quadword pointed to by a1

	.align	3
f_usmall:
	ldq_u	t3, -1(a4)		# load high bytes of src
	extql	t2, a1, t2		# get low bytes of src
	extqh	t3, a1, t3		# get high bytes of src
	or	t2, t3, t2		# merge high and low bytes of src
	insqh	t2, a0, t3		# get high src bytes with dst alignment
	insql	t2, a0, t2		# get low src bytes with dst alignment

	ldiq	t4, -1			# get mask of 8 0xff bytes
	mskql	t4, a2, t5		# mask out bytes to ignore
	cmovne	t5, t5, t4		# fix mask for count of 8
	insqh	t4, a0, t5		# get high mask bytes with dst alignment
	insql	t4, a0, t4		# get low mask bytes with dst alignment

	addq	a0, a2, a3		# get end of dst string + 1
	ldq_u	t6, 0(a0)		# get low dst bytes
	ldq_u	t7, -1(a3)		# get high dst bytes

	bic	t6, t4, t6		# mask out low dst bytes to ignore
	bic	t7, t5, t7		# mask out high dst bytes to ignore
	and	t2, t4, t2		# mask out low src bytes to ignore
	and	t3, t5, t3		# mask out high src bytes to ignore
	or	t2, t6, t2		# merge low bytes
	or	t3, t7, t3		# merge high bytes

	stq_u	t3, -1(a3)		# store merged high bytes
	stq_u	t2, 0(a0)		# store merged low bytes
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
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length

	.align	3
backwards:
	cmpule	a2, 8, t0		# is length >= 8?
	addq	a1, a2, a4		# point past end of src
	addq	a0, a2, a3		# point past end of dst
	bne	t0, b_small		# br if length <= 8
	and	a3, 7, t4		# get bytes till dst is QW aligned
	bne	t4, b_initial		# br if not already aligned

	# Backward copy where the next byte to copy will be stored
	# on a quadword boundary. Copy a quadword at a time. Unlike
	# the forward copy we can exit this loop with no bytes to
	# copy.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	a3	destination pointer (from end)
	#	a4	source pointer (from end)

	.align	3
b_quads:
	bic	a2, 7, t4		# get QWs * 8 to move
	and	a2, 7, a2		# adjust length
	nop				# improve scheduling
	beq	t4, b_final		# skip if none

10:	ldq_u	t0, -8(a4)		# get low bytes of src QW
	subq	a3, 8, a3		# adjust dst pointer
	ldq_u	t1, 7-8(a4)		# get high bytes of src QW
	subq	a4, 8, a4		# adjust src pointer
	extql	t0, a4, t0		# extract low bytes
	extqh	t1, a4, t1		# extract high bytes
	subq	t4, 8, t4		# adjust length
	or	t0, t1, t0		# merge low and high bytes
	stq	t0, 0(a3)		# store the merge bytes
	bne	t4, 10b			# loop on QW count

	# Store the final 0..7 bytes to the destination string.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	remaining length

	.align	3
b_final:
	bne	a2, 20f			# skip if any bytes remain
	RET

20:	ldq_u	t0, 0(a1)		# get low bytes of src
	ldq_u	t1, 7(a1)		# get high bytes of src
	ldq_u	t2, 0(a0)		# get dst QW
	extql	t0, a1, t0		# extract low bytes
	extqh	t1, a1, t1		# extract high bytes
	or	t0, t1, t0		# merge low and high bytes
	insql	t0, a0, t0		# position to match dst alignment
	mskql	t2, a0, t2		# clear ignored bytes from dst
	or	t2, t0, t2		# merge in src bytes
	stq_u	t2, 0(a0)		# store merged bytes
	RET

	# Backward copy where the destination pointer is not currently on
	# a quadword boundary. Copy the initial bytes so the destination
	# pointer will be aligned.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	a3	destination pointer (from end)
	#	a4	source pointer (from end)
	#	t4	bytes till dst is QW aligned

	.align	3
b_initial:
	subq	a4, t4, a4		# adjust src pointer
	subq	a3, t4, a3		# adjust dst pointer
	ldq_u	t1, 0(a4)		# get low bytes of src
	subq	a2, t4, a2		# adjust length
	ldq_u	t2, 7(a4)		# get high bytes of src
	ldq	t3, 0(a3)		# get dst QW
	extql	t1, a4, t1		# extract low bytes of src
	extqh	t2, a4, t2		# extract high bytes of src
	or	t1, t2, t1		# merge low and high bytes

	mskqh	t3, t4, t3		# clear bytes to ignore in dst
	mskql	t1, t4, t1		# clear bytes to ignore in src
	or	t1, t3, t1		# merge src and dst bytes
	stq	t1, 0(a3)		# store merged bytes
	br	zero, b_quads

	# Backward copy where the length is 8 bytes or less.
	#
	#	a0	destination pointer
	#	a1	source pointer
	#	a2	length
	#	a3	destination pointer (from end)
	#	a4	source pointer (from end)

b_small:
	ldq_u	t2, 0(a1)		# get low bytes of src
	br	zero, f_usmall		# handle like forward case

done:	RET

END(memmove)
