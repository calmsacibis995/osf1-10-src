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
/* @(#)atod.s	9.3 (ULTRIX) 1/7/91 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *
 *		Modification History
 *
 * 0001	Ken Lesniak, 12-Jul-1990
 *	Translation of mips version
 *
 ************************************************************************/

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/atod.s,v 1.1.8.3 1993/06/08 01:23:41 David_Lindner Exp $ */

#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <machine/regdef.h>
#include <machine/asm.h>


/*
 * x = _atod (buffer, ndigits, exp)
 * Convert ndigits from buffer.  Digits in binary (0..9).
 * 1 <= ndigits <= 17.  -324 <= exp <= 308
 * Return double value.
 *
 *	a0	buffer
 *	a1	ndigits
 *	a2	exp
 */
#define FRMSIZ 16
NESTED(_atod, FRMSIZ, ra)
	.save_ra	ra
	ldgp	gp, 0(pv)
	lda	sp, -FRMSIZ(sp)
	.prologue	1

	/* Convert digits to 64-bit integer * 2 (* 2 so that multiplication */
	/* by 10^N leaves binary point between registers). */

	ldiq	t2, 0		# initialize the fraction

	/* Multiply by 5 */

10:	s4addq	t2, t2, t2

	/* Add digit */

11:	ldq_u	t0, (a0)	# get next digit
	extbl	t0, a0, t0
	addq	a0, 1, a0	# advance buffer pointer
	addq	t0, t2, t2

	/* Multiply by 2 */

	sll	t2, 1, t2

	subq	a1, 1, a1	# adjust digit counter
	bne	a1, 10b		# continue if more digits

	/* Dispatch on zero */

	beq	t2, 50f

	/* Multiply 64-bit integer by 10^N, leaving binary point */
	/* between t1:t0. */

	bis	zero, a2, t0	# get exponent
	jsr	a4, _tenscale
	ldgp	gp, 0(a4)

	/* Normalize */

	/* 64-bit normalize */

	bne	t1, 20f		# br if high word non-zero
	bis	zero, t0, t1	# hi word zero, shift left 64 bits
	ldiq	t0, 0
	subq	a5, 64, a5	# adjust exp for 64 bit shift

	/* Find normalization shift */

20:	srl	t1, 32, a2
	ldiq	t2, 32+16
	cmoveq	a2, 16, t2
	srl	t1, t2, a2
	addq	t2, 8, t2
	subq	t2, 16, a1
	cmoveq	a2, a1, t2
	srl	t1, t2, a2
	addq	t2, 4, t2
	subq	t2, 8, a1
	cmoveq	a2, a1, t2
	srl	t1, t2, a2
	addq	t2, 2, t2
	subq	t2, 4, a1
	cmoveq	a2, a1, t2
	srl	t1, t2, a2
	addq	t2, 1, t2
	subq	t2, 2, a1
	cmoveq	a2, a1, t2
	srl	t1, t2, a2
	xor	a2, 1, a2
	subq	t2, a2, t2

	addq	a5, t2, a5
	addq	a5, 1023, a5	# bias exponent
	bgt	a5, 29f

	/* Will produce denorm */

	subq	t2, a5, t2	# exponent pins at 0
	addq	t2, 1, t2
	ldiq	a5, 1		# 1 instead of 0 to compensate for
				# hidden bit subtraction below

	/* Normalize */

29:	subq	t2, 52, a3	# normalize so that msb is bit 20 (hidden bit)
	beq	a3, 26f
	subq	zero, a3, t3
	blt	a3, 25f

	/* 128-bit right shift */

	srl	t0, a3, t0	# necessary only for round to even done below
	sll	t1, t3, a2	# ditto
	bis	t0, a2, t0	# ditto
	srl	t1, a3, t1
	br	zero, 26f

	/* 128-bit left shift */

25:	sll	t1, t3, t1
	srl	t0, a3, a2
	bis	t1, a2, t1
	sll	t0, t3, t0	# necessary only for round to even done below

	/* Round */

26:	bge	t0, 27f
	sll	t0, 1, t0
	bne	t0, 28f
	blbc	t1, 27f

28:	addq	t1, 1, t1
	sll	t1, 63-53, a3	# overflow into bit 53?
	bge	a3, 27f
	addq	a5, 1, a5
	srl	t1, 1, t1

27:	cmpult	a5, 2047, t3
	beq	t3, 40f
	subq	a5, 1, a5	# subtract out hidden bit
	sll	a5, 52, a5	# shift exponent into place
	addq	t1, a5, t1	# combine exponent and fraction
	stq	t1, 0(sp)	# transfer to floating register...
	ldt	$f0, 0(sp)
	lda	sp, FRMSIZ(sp)
	RET

40:	ldt	$f0, infinity
	lda	sp, FRMSIZ(sp)
        ldiq    v0,34           # ERANGE
        jmp      zero,_cerror    # cerror will set errno, (cerror does an RET)
	RET

	/* Zero */

50:	cpys	$f31, $f31, $f0
	lda	sp, FRMSIZ(sp)
	RET
END(_atod)

/*.rdata*/.sdata
.align 3
infinity:
	.quad	0x7ff0000000000000
