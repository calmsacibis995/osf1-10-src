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
 * @(#)$RCSfile: memchr.s,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/06/08 01:24:29 $
 */
#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern void *memchr(const void *s, int c, size_t n);
 *
 *	a0	string address (s)
 *	a1	char to search for (c)
 *	a2	length (n)
 *
 * Sequentially search the string at the location pointed to by the "s"
 * parameter until the character specified by the "c" parameter is
 * encountered, or the number of characters specified by the "n"
 * parameter have been examined.
 *
 * The return value is a pointer to the character when it is encountered;
 * otherwise a null pointer.
 */
	.align	4
LEAF(memchr)
	.prologue 0

	and	a1, 255, a1		# cast "c" to an unsigned char
	beq	a2, nomatch		# no match if zero length
	ldq_u	t0, 0(a0)		# get first bytes
	sll	a1, 8, t2		# shift "c" left a byte
	negq	a0, t1			# negate src pointer
	or	a1, t2, a1		# "c" repliclated twice
	sll	a1, 16, t2		# shift "c" left a word
	and	t1, 8-1, t1		# bytes till src is QW aligned
	or	a1, t2, a1		# "c" replicated 4 times
	sll	a1, 32, t2		# shift "c" left a longword
	or	a1, t2, a1		# "c" replicated 8 times
	bne	t1, initial		# go handle unaligned pointer

	# The source pointer is quadword aligned, so scan for the
	# search char a quadword at a time. The number of quads to
	# scan is calculated so that there is non-zero number of bytes
	# left to scan when the loop exits
	#
	#	a0	string address
	#	a1	char to search for (replicated 8 times)
	#	a2	length
	#	t0	quadword pointed to by a0

	.align	3
quads:	subq	a2, 1, t3		# get count - 1
	bic	t3, 8-1, t3		# get number of quads to check
	subq	a2, t3, a2		# get final count (always non-zero)
	beq	t3, final		# skip if no full quads to copy
10:	xor	a1, t0, t0		# set bytes that match to zero
	cmpbge	zero, t0, t0		# see if we matched the char
	subq	t3, 8, t3		# adjust quad count
	bne	t0, match		# break out if we matched
	ldq	t0, 8(a0)		# get next 8 bytes
	addq	a0, 8, a0		# adjust src pointer
	bne	t3, 10b			# continue if more quads

	# Scan the bytes (1..8) of the final (aligned) quadword.
	#
	#	a0	string address
	#	a1	char to search for (replicated 8 times)
	#	a2	length
	#	t0	quadword pointed to by a0

	.align	3
final:	xor	a1, t0, t0		# set bytes that match to zero
	cmpbge	zero, t0, t0		# see if we matched the char
	ldiq	t5, 255			# get 8-byte bit mask
	sll	t5, a2, t5		# bit mask of bytes to ignore
	bic	t0, t5, t0		# ignore matches past end of string
	beq	t0, nomatch		# skip if no matches

	# The search char was found within a quadword. Return with
	# v0 pointing to the address of the char
	#
	#	a0	string address of quadword containing search char
	#	t0	bit mask of bytes which matched search char

	.align	3
match:	negq	t0, t1			# flip all but lowest bit set
	and	t0, t1, t0		# isolate lowest bit set
	and	t0, 0x0f, t1		# "c" in bytes 0..3?
	addq	a0, 4, v0		# ready pointer assuming yes
	cmovne	t1, a0, v0		# if yes, replace pointer
	and	t0, 0x33, t3		# "c" in bytes 0..1, or 4..5?
	addq	v0, 2, t2		# ready pointer assuming no
	cmoveq	t3, t2, v0		# if no, replace pointer
	and	t0, 0x55, t4		# "c" in bytes 0, 2, 4 of 6
	addq	v0, 1, t2		# ready pointer assuming no
	cmoveq	t4, t2, v0		# if no, replace pointer
	RET

	# The string to search is not quadword aligned. Scan the initial
	# bytes so that the pointer will be aligned.
	#
	#	a0	string address
	#	a1	char to search for (replicated 8 times)
	#	a2	length
	#	t0	quadword pointed to by a0
	#	t1	# of bytes until string address is QW aligned

	.align	3
initial:
	and	a0, 8-1, t6		# get offset to first byte
	cmpult	a2, t1, t3		# scan to QW boundary?
	xor	a1, t0, t0		# set bytes that match to zero
	cmpbge	zero, t0, t0		# see if we matched the char
	bic	a0, 8-1, a0		# align src pointer
	ldiq	t5, 255			# get 8-byte bit mask
	sll	t5, t6, t5		# bit mask of bytes to ignore
	and	t0, t5, t0		# ignore matches before start of string
	bne	t3, partial		# skip if not scanning to QW boundary
	subq	a2, t1, a2		# adjust length
	bne	t0, match		# skip if we matched a char
	beq	a2, nomatch		# no match if no more bytes left
	ldq	t0, 8(a0)		# get next quad
	addq	a0, 8, a0		# advance src pointer
	br	zero, quads		# scan by quadword now

	# The string to search is not quadword aligned, and the length
	# is less than the number of bytes until the pointer would be
	# quadword aligned.
	#
	#	a0	string address
	#	a2	length
	#	t0	bit mask of bytes which matched search char
	#	t5	bit mask of which ignores the initial bytes

	.align	3
partial:
	sll	t5, a2, t5		# bit mask of bytes to ignore
	andnot	t0, t5, t0		# ignore matches past end of string
	bne	t0, match		# skip if we matched a char
	# fall into nomatch

	# The search char was not found before the count was exhausted.
	# Return a null pointer in v0.

	.align	3
nomatch:
	ldiq	v0, 0			# return null
	RET	

END(memchr)
