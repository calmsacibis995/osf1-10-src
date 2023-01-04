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
#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern size_t strlen(char *s);
 *
 *	a0	string address (s)
 *
 * Return the number of bytes in the string pointed to by the "s" parameter.
 * The string length value does not include the terminating null character.
 */
	.align	4
	.weakext NLstrdlen, strlen
	.weakext NLstrlen, strlen
LEAF(strlen)
	.prologue 0

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
