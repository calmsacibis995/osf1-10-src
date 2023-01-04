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

#include <alpha/asm.h>
#include <alpha/regdef.h>

/*
 * extern unsigned int ntohl(unsigned int netlong);
 *
 *	a0	32-bit integer in host-byte order (netlong)
 *
 * Convert a 32-bit integer from Internet network-byte order (big endian)
 * to host-byte order (little endian).
 *
 * The return value contains a 32-bit value in host-byte order.
 *
 * NOTE: This routine and htonl are identical. If you make a change here,
 * please make it in htonl as well.
 */
	.align	4
LEAF(ntohl)
	.prologue 0

	# the comments on each instruction show what's in each byte of
	# the dst register after the instruction executes. Each
	# character of the string corresponds to one byte of the register,
	# with the left most character corresponding to the MSB.
	# The characters have the following meaning:
	#	x	a "don't know" byte
	#	a-d	arbitrary data byte
	#	s	sign-extended byte (0 or 0xff)
	#	0	a zero byte

	# a0				# xxxxabcd
	extbl	a0, 3, t0		# 0000000a
	insbl	a0, 3, t1		# 0000d000
	sll	a0, 8, t2		# xxxabcd0
	zapnot	t2, 4, t2		# 00000c00
	srl	a0, 8, t3		# 0xxxxabc
	zapnot	t3, 2, t3		# 000000b0
	or	t0, t1, v0		# 0000d00a
	or	t2, v0, v0		# 0000dc0a
	addl	t3, v0, v0		# ssssdcba
	RET
END(ntohl)
