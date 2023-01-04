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
 * @(#)$RCSfile: strcat.s,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/08 01:25:42 $
 */
#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern char *strcat(char *s1, const char *s2);
 *
 *	a0	dst string pointer (s1)
 *	a1	src string pointer (s2)
 *
 * Append a copy of the string pointed to by the "s2" parameter,
 * including the terminating null character, to the string pointed
 * to by the "s1" parameter. The beginning character of the string
 * pointed to by the "s2" parameter overwrites the null character
 * at the end of the string pointed to by the "s1" parameter.
 * When operating on overlapping strings, the behavior is unreliable.
 *
 * The return value contains a pointer to the destination string (s1).
 */
	.align	4
LEAF(strcat)
	.prologue 0

	# locate the end of the dst string

	ldq_u	t2, 0(a0)		# get first dst quad
	ldiq	t0, -1			# mask of 8 0xff bytes
	mskql	t0, a0, t0		# mask of bytes before start of string
	bis	a0, a0, v0		# set return value
	or	t0, t2, t2		# set initial bytes to 0xff
	cmpbge	zero, t2, t0		# any null bytes?
	bic	a0, 8-1, a0		# align dst pointer
	bne	t0, 20f			# skip if we found a null byte

10:	ldq	t2, 8(a0)		# get next dst quad
	addq	a0, 8, a0		# advance dst pointer
	cmpbge	zero, t2, t0		# any null bytes?
	beq	t0, 10b			# continue scanning if not

20:	subq	zero, t0, t1		# flip bits left of first bit set
	and	t0, t1, t0		# isolate first bit set
	and	t0, 0x0f, t1		# null in bytes 0..3?
	addq	a0, 4, t5		# ready pointer assuming no
	cmoveq	t1, t5, a0		# if no, replace pointer
	and	t0, 0x33, t3		# null in bytes 0..1, or 4..5?
	addq	a0, 2, t2		# ready pointer assuming no
	cmoveq	t3, t2, a0		# if no, replace pointer
	and	t0, 0x55, t4		# null in bytes 0, 2, 4 of 6
	addq	a0, 1, t2		# ready pointer assuming no
	cmoveq	t4, t2, a0		# if no, replace pointer

	# copy the src string to the end of the dst string
	#
	#	a0	dst pointer
	#	a1	src pointer

	ldq_u	t0, 0(a1)		# get first src quad
	xor	a1, a0, t4		# see where src and dst differ
	and	t4, 8-1, t4		# only interested in low 3 bits
	and	a0, 8-1, t7		# get offset to first dst byte
	bne	t4, unaligned		# skip if unaligned
	cmpbge	zero, t0, t1		# any null bytes?
	bne	t7, initial		# copy initial bytes
	nop				# improve scheduling
	bne	t1, final		# br if found null byte

	# The src and dst pointers are quad aligned, so copy a
	# quadword at a time until we find one with a null byte
	#
	#	a0	dst pointer (not necessarily aligned)
	#	a1	src pointer (not necessarily aligned)
	#	t0	8 src bytes pointed to by a1

	.align	3
quads:	stq_u	t0, 0(a0)		# store next quad
	addq	a0, 8, a0		# advance dst pointer
	ldq_u	t0, 8(a1)		# get next src quad
	cmpbge	zero, t0, t1		# any null bytes?
	addq	a1, 8, a1		# advance src pointer
	beq	t1, quads		# continue looping if not

	# We've loaded a quad containing a null byte -- copy
	# only up to the null byte
	#
	#	a0	dst pointer
	#	t0	src quadword containing null byte
	#	t1	bit mask of null bytes within t0

	.align	3
final:	ldq_u	t2, 0(a0)		# get dst quad
	subq	zero, t1, t3		# flip bits left of first bit set
	xor	t1, t3, t1		# bit mask of bytes after null
	zap	t0, t1, t0		# zap src bytes to ignore
	zapnot	t2, t1, t2		# zap dst bytes to ignore
	nop				# improve scheduling
	or	t0, t2, t0		# merge src and dst bytes
	stq_u	t0, 0(a0)		# store the merged bytes
	RET

	# The src and dst pointers have the same alignment, but the
	# initial pointers are not quad aligned. Copy to the end of
	# the quad or to the null byte if there is one
	#
	#	a0	dst pointer
	#	a1	src pointer
	#	t0	src quadword containing null byte
	#	t1	bit mask of null bytes within t0
	#	t7	dst pointer alignment

	.align	3
initial:
	ldq_u	t2, 0(a0)		# get dst quad
	ldiq	t6, 255			# init bit mask to all bytes
	sll	t6, t7, t6		# get bit mask of relevant bytes
	and	t6, t1, t1		# ignore nulls before start of string
	subq	zero, t1, t3		# flip bits left of first bit set
	xor	t1, t3, t3		# bit mask of bytes after null
	ornot	t3, t6, t6		# bit mask of non-src bytes
	zap	t0, t6, t0		# zap src bytes to ignore
	zapnot	t2, t6, t2		# zap dst bytes to ignore
	nop
	or	t0, t2, t0		# merge src and dst bytes
	beq	t3, quads		# copy quads if we didn't find a null
	stq_u	t0, 0(a0)		# store the merged bytes
	RET	

	# The src and dst pointers do not have the same alignment.
	#
	#	a0	dst pointer
	#	a1	src pointer
	#	t0	initial source bytes
	#	t7	dst pointer alignment

	.align	3
unaligned:
	subq	zero, a0, t5		# negate dst pointer
	and	t5, 8-1, t5		# bytes till dst is aligned
	ldiq	t6, 255			# bit mask of 8 bytes all 1
	bne	t5, uinitial		# br if dst is not already aligned

	# The next dst byte is on a quadword boundary, so try to
	# copy a quadword at a time. Continue copying until we find
	# a source byte with a null.
	#
	#	a0	dst pointer
	#	a1	src pointer
	#	t0	initial source bytes
	#	t6	all-byte bit mask

	.align	3
uquads:	and	a1, 8-1, t7		# get src alignment
	srl	t6, t7, t6		# get mask of initial bytes

10:	extql	t0, a1, t8		# get low bytes of src
	cmpbge	zero, t8, t1		# any null bytes?
	and	t1, t6, t2		# only look at initial bytes
	bne	t2, ufinal		# br if we found a null

	ldq_u	t0, 8(a1)		# get next 8 src bytes
	addq	a1, 8, a1		# advance src pointer

	extqh	t0, a1, t9		# get high bytes of src
	or	t8, t9, t8		# merge high and low bytes
	cmpbge	zero, t8, t1		# any null bytes?
	bne	t1, ufinal		# br if we found a null

	stq	t8, 0(a0)		# store the next 8 bytes
	addq	a0, 8, a0		# advance the dst pointer
	br	zero, 10b		# continue till we find a null

	# We've loaded a source quadword that contains a null -- copy
	# only up to the null byte
	#
	#	a0	dst pointer
	#	t8	src quadword containing null byte
	#	t1	bit mask of null bytes within t0

	.align	3
ufinal:
	bis	t8, t8, t0		# copy src bytes containing null
	br	zero, final		# continue with aligned case

	# The src and dst pointers do not have the same alignment,
	# and the dst pointer is not quadword aligned. Copy the
	# initial bytes so that the dst pointer is quadword aligned,
	# and then copy a quadword at a time.
	#
	#	a0	dst pointer
	#	a1	src pointer
	#	t0	initial source bytes
	#	t5	bytes till dst is QW aligned
	#	t6	all-byte bit mask
	#	t7	dst pointer alignment

	.align	3
uinitial:
	ldq_u	t2, 0(a0)		# get dst quadword
	ldiq	t3, -1			# get 8 bytes of 0xff
	extql	t3, a0, t4		# get mask of the dst bytes
	extql	t3, a1, t3		# get mask of the low src bytes
	addq	a1, t5, a3		# get pointer past initial src bytes

	extql	t0, a1, t9		# get the low initial src bytes
	ornot	t9, t4, t9		# set non-dst bytes to 0xff
	ornot	t9, t3, t8		# set non-src bytes to 0xff
	cmpbge	zero, t8, t1		# any null bytes?
	bne	t1, upartial		# br if a null found

	mskql	t2, a0, t4		# zap initial dst bytes

	ldq_u	t0, -1(a3)		# get high bytes of initial src bytes
	extqh	t0, a1, t8		# extract the high src bytes
	or	t8, t9, t8		# merge the high and low src bytes
	cmpbge	zero, t8, t1		# any null bytes?
	insql	t8, a0, t3		# shift src to match dst alignment
	addq	a1, t5, a1		# advance the src pointer
	bne	t1, upartial		# br if a null found

	or	t4, t3, t2		# merge src and dst bytes
	stq_u	t2, 0(a0)		# store the merged bytes
	addq	a0, t5, a0		# advance the dst pointer

	ldq_u	t0, 0(a1)		# get the next low src bytes
	br	zero, uquads		# copy by quadword now

	# The src and dst pointers do not have the same alignment,
	# and the dst pointer is not quadword aligned, and the initial
	# bytes of the src contained a null byte. Merge the src
	# bytes with the dst bytes and exit.
	#
	#	a0	dst pointer
	#	t2	initial dst bytes
	#	t6	all-byte bit mask
	#	t7	dst pointer alignment
	#	t8	initial source bytes

	.align	3
upartial:
	insql	t8, a0, t8		# shift src to match dst alignment
	sll	t1, t7, t1		# shift null-byte mask to match src
	sll	t6, t7, t6		# get mask of initial src bytes
	subq	zero, t1, t4		# flip bits after first
	xor	t1, t4, t4		# bit mask of bytes after null
	ornot	t4, t6, t6		# mask of src bytes to be copied
	zap	t8, t6, t8		# zap src bytes to ignore
	zapnot	t2, t6, t2		# zap dst bytes to ignore
	or	t2, t8, t2		# merge src and dst bytes
	stq_u	t2, 0(a0)		# store the merged bytes
	RET

END(strcat)
