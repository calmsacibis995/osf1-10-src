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
 * @(#)$RCSfile: strcmp.s,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/10/05 21:00:22 $
 */

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern int strcmp(const char *s1, const char *s2);
 *
 *	a0	string 1 pointer (s1)
 *	a1	string 2 pointer (s2)
 *
 * Compare the string pointed to by the "s1" parameter to the string
 * pointed to by the "s2" parameter. The sign of any nonzero value returned
 * is determined by the sign resulting from the difference in integer
 * values of the first character-pair comparison (both converted to
 * "unsigned char") in which the characters are different.
 *
 * The return value is an integer whose value is greater than, equal to
 * or less than 0, according to whether the "s1" string is greater than,
 * equal to, or less than the "s2" string.
 */
	.align	4
WEAK_LEAF(strcmp,mbscmp)
	.prologue 0

	ldq_u	t0, 0(a0)		# get initial bytes of s1
	xor	a0, a1, t2		# see where s1 and s2 ptrs differ
	ldq_u	t1, 0(a1)		# get initial bytes of s2
	and	t2, 8-1, t2		# see if s1 and s2 alignments match
	and	a0, 8-1, t3		# get s1 alignment
	ldiq	v0, -1			# ret value for s1 < s2
	ldiq	t8, -1			# get 8 bytes of 0xff
	bne	t2, unaligned		# br if alignment mismatch
	bne	t3, initial		# go handle partial quadword

	# The next byte, to be compared, of both strings is on a quadword
	# boundary -- compare a full quadword at once. Continue
	# comparing quadwords until we find a mismatch or a null byte
	#
	#	a0	string 1 pointer (may not be aligned)
	#	a1	string 2 pointer (may not be aligned)
	#	t0	next 8 bytes of string 1
	#	t1	next 8 bytes of string 2
	#	v0	return value for s1 < s2

	.align	3
quads:	cmpbge	zero, t0, t2		# any null bytes in s1?
	xor	t0, t1, t3		# see if any bytes don't match
	or	t2, t3, t4		# find a null or mismatch?
	bne	t4, final		# br if so
	ldq_u	t0, 8(a0)		# get next 8 bytes of s1
	addq	a0, 8, a0		# advance s1 pointer
	ldq_u	t1, 8(a1)		# get next 8 bytes of s2
	addq	a1, 8, a1		# advance s2 pointer
	br	zero, quads		# continue

	# We've compared two quadwords and have either found a mismatch,
	# or a null-byte, or both. Determine which came first and then
	# do an unsigned comparison on the two bytes. Note that
	# string 1 and string 2 may have been swapped if the two
	# strings did not have the same alignment.
	#
	#	t0	final bytes of string 1
	#	t1	final bytes of string 2
	#	t2	bit mask of null bytes in string 1
	#	t3	xor of string 1 and string 2 bytes
	#	v0	return value for s1 < s2

	.align	3
	nop				# improve scheduling
final:	cmpbge	zero, t3, t3		# get mask of matches
	ornot	t2, t3, t2		# merge in null byte mask
	subq	zero, t2, t4		# flip all but first bit
	and	t4, t2, t2		# isolate bit for first mismatch
	cmpbge	t0, t1, t0		# get mask bytes where s1 >= s2
	and	t3, t2, t3		# set NE if final byte matches
	and	t0, t2, t0		# set NE if final s1 >= s2
	subq	zero, v0, t1		# get retval for s1 > s2
	cmovne	t0, t1, v0		# set retval if s1 >= s2
	cmovne	t3, 0, v0		# set retval (0) if s1 == s2
	RET

	# Both strings have the same alignment, but the first byte
	# is not on a quadword boundary. Compare the initial bytes
	# and then continue on with the aligned case. We compare the
	# initial bytes by forcing the initial bytes of string 1 and
	# string 2 to be 0xff, and then comparing the whole quadword.
	#
	#	a0	string 1 pointer
	#	a1	string 2 pointer
	#	t0	initial bytes of string 1
	#	t1	initial bytes of string 2
	#	t3	alignment of string 1
	#	t8	8 bytes of 0xff
	#	v0	return value for s1 < s2

	.align	3
initial:
	mskql	t8, a0, t8		# get mask of initial 0xff bytes
	or	t8, t0, t0		# set initial s1 bytes to 0xff
	or	t8, t1, t1		# set initial s2 bytes to 0xff
	br	zero, quads		# compare the quads and continue

	# The two string pointers do not have the same alignment. The
	# strategy is to get the string 1 pointer quadword aligned and
	# then make unaligned references to string 2. Beware that this
	# code has to be quite careful to avoid reading past the end of
	# either of the source strings.
	#
	#	a0	string 1 pointer
	#	a1	string 2 pointer
	#	t0	initial bytes of string 1
	#	t1	initial bytes of string 2
	#	t3	alignment of string 1
	#	t8	8 bytes of 0xff
	#	v0	return value for s1 < s2

	.align	3
unaligned:
	bne	t3, uinitial		# br if string 1 is unaligned

	# The next byte, to be compared, of string 1 is on a quadword
	# boundary -- compare a full quadword at once. Continue
	# comparing quadwords until we find a mismatch or a null byte.
	# Note that string 1 and string 2 may have been swapped by the
	# unaligned comparison code.
	#
	#	a0	string 1 pointer
	#	a1	string 2 pointer (may not be aligned)
	#	t0	next 8 bytes of string 1
	#	t1	next 8 bytes of string 2
	#	t8	8 bytes of 0xff
	#	v0	return value for s1 < s2

	.align	3
uquads:	extql	t8, a1, t8		# get mask of low s2 bytes
	nop				# so next label is aligned

10:	extql	t1, a1, t5		# get low bytes of s2
	ornot	t5, t8, t5		# set high bytes of s2 to 0xff
	ornot	t0, t8, t4		# set high bytes of s1 to 0xff
	cmpbge	zero, t4, t2		# any null bytes in s1?
	xor	t4, t5, t3		# see if any bytes don't match
	or	t2, t3, t6		# find a null or mismatch?
	addq	a1, 8, a1		# advance s2 pointer
	bne	t6, ufinal		# br if null or mismatch

	ldq_u	t1, 0(a1)		# get next 8 bytes of s2
	nop				# improve scheduling
	extqh	t1, a1, t5		# get high bytes of s2
	or	t5, t8, t5		# set low bytes of s2 to 0xff
	or	t0, t8, t4		# set low bytes of s1 to 0xff
	cmpbge	zero, t4, t2		# any null bytes in s1?
	xor	t4, t5, t3		# see if any bytes don't match
	or	t2, t3, t6		# find a null or mismatch?
	addq	a0, 8, a0		# advance s1 pointer
	bne	t6, ufinal		# br if null or mismatch

	ldq	t0, 0(a0)		# get next 8 s1 bytes
	br	zero, 10b		# continue till null or mismatch

	# We've compared two quadwords and have either found a mismatch,
	# or a null-byte, or both. Determine which came first and then
	# do an unsigned comparison on the two bytes. Note that string 1
	# and string 2 may have been swapped by the unaligned comparison code.
	#
	#	t2	bit mask of null bytes in string 1
	#	t3	xor of string 1 and string 2 bytes
	#	t4	final bytes of string 1
	#	t5	final bytes of string 2
	#	v0	return value for s1 < s2

	.align	3
	nop				# improve scheduling
ufinal:	bis	t4, t4, t0		# copy final string 1 bytes
	bis	t5, t5, t1		# copy final string 2 bytes
	br	zero, final		# continue with aligned version

	# The strings do not have the same alignment and the first byte
	# of string 1 is not on a quadword boundary. Compare the initial
	# bytes so that string 1 is quadword aligned, and then continue on
	# with the aligned case. We swap the string 1 and string 2
	# pointers as necessary so that s2 is the string with more
	# initial bytes.
	#
	#	a0	string 1 pointer
	#	a1	string 2 pointer
	#	t0	initial bytes of string 1
	#	t1	initial bytes of string 2
	#	t3	alignment of string 1
	#	t8	8 bytes of 0xff
	#	v0	return value for s1 < s2

	.align	3
uinitial:
	and	a1, 8-1, t4		# get s2 alignment
	cmpult	t4, t3, t3		# does s2 have more bytes till aligned?
	bis	a0, a0, t5		# start to swap s1 and s2 pointers
	bne	t3, 10f			# br if s2 has more

	bis	a1, a1, a0		# finish swaping s1 and s2 pointers
	bis	t5, t5, a1
	bis	t0, t0, t6		# swap s1 and s2 initial bytes
	bis	t1, t1, t0
	bis	t6, t6, t1
	subq	zero, v0, v0		# flip ret value for s1 < s2

10:	extql	t0, a0, t0		# get low bytes of s1
	extql	t1, a1, t1		# get low bytes of s2
	extqh	t8, a0, t4		# get mask of high bytes
	or	t0, t4, t0		# set high bytes of s1 to 0xff
	or	t1, t4, t1		# set high bytes of s2 to 0xff
	cmpbge	zero, t0, t2		# any null bytes in s1?
	xor	t0, t1, t3		# see if any bytes don't match
	or	t2, t3, t6		# find a null or mismatch?
	subq	zero, a0, t5		# negate s1 pointer
	bne	t6, final		# br if null or mismatch

	ldq_u	t0, 8(a0)		# get next 8 bytes of s1
	and	t5, 8-1, t5		# bytes to s1 is aligned
	addq	a0, t5, a0		# advance s1 pointer
	addq	a1, t5, a1		# advance s2 pointer
	ldq_u	t1, 0(a1)		# get next bytes of s2
	br	zero, uquads		# continue with s1 aligned case

END(strcmp) 
