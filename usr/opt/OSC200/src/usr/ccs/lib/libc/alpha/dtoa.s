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
/* @(#)dtoa.s	9.2 (ULTRIX) 1/7/91 */
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
 * 0001	Ken Lesniak, 09-Jul-1990
 *	Translation of mips version
 *
 ************************************************************************/

/* --------------------------------------------------- */
/* | Copyright (c) 1986 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/dtoa.s,v 1.1.8.3 1993/06/07 19:46:31 Thomas_Peterson Exp $ */

#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif

#include <machine/regdef.h>
#include <machine/asm.h>


#define SPACE 32	/* ascii space */
#define MINUS 45	/* ascii minus sign */
#define ZERO 48		/* ascii zero */

/*
 * _dtoa(buffer, ndigits, x, fflag)
 * Store sign, ndigits of x, and null in buffer.  Digits are in ascii.
 * 1 <= ndigits <= 17.  Return exponent.  If fflag set then generate
 * Fortran F-format; i.e. ndigits after decimal point.
 *	a0	buffer
 *	a1	ndigits
 *	$f18	x
 *	a3	fflag
 */
#define FRMSIZ 16
NESTED(_dtoa, FRMSIZ, ra)
	ldgp	gp, 0(pv)
	lda	sp, -FRMSIZ(sp)
	.prologue	1

	stt	$f18, 0(sp)		# copy "x" into integer reg
	ldq	a2, 0(sp)

	/* Copy sign to buffer */

	ldiq	t1, SPACE		# assume positive
	cmovlt	a2, MINUS, t1		# change sign if we assumed wrong
	ldq_u	t0, (a0)		# store the sign
	insbl	t1, a0, t1
	mskbl	t0, a0, t0
	bis	t0, t1, t1
	stq_u	t1, (a0)
	addq	a0, 1, a0		# update the buffer pointer

	sll	a2, 1, t2		# move the exp into the msbit

	/* Convert binary exponent into approximate decimal exponent */

	ldiq	t4, 0x4d104d42 >> 21	# log10(2) shifted so decimal point
					# ends up between bits 32/31 after
					# multiplication
	srl	t2, 32, t1		# get exp left justified in low 32 bits
	subq	t1, 1023 << 21, t1	# remove exp bias
	addq	t4, 1, t0		# round up log10(2) approximation
	cmovlt	t1, t0, t4		# if neg exp, use rounded up approx.
	mulq	t1, t4, v0		# get approximate decimal exponent
	sra	v0, 32, v0

	/* Check for special cases */

	srl	t2, 53, t8		# copy biased exp
	beq	t8, 10f			# br if +/- 0 or denormalized
	cmpeq	t8, 2047, t5
	bne	t5, 20f			# br if +/- infinity or NaN

	/* Convert to fixed point number in range 1..100 by multiplication */
	/* by appropriate power of 10. */

	sll	t2, 11, t2		# remove exponent
	srl	t2, 11, t2
	ldiq	t5, 1			# add in hidden bit
	sll	t5, 53, t5
	bis	t5, t2, t2

15:	subq	zero, v0, t0		# negate exponent
	jsr	a4, _tenscale		# perform 10**exp
	ldgp	gp, 0(a4)
	subq	t8, 1023+52, t8
	addq	t8, a5, t8
	subq	zero, t8, t8

	/* Extract first digit */

	srl	t1, t8, t6
	sll	t6, t8, t7
	subq	t1, t7, t1
	cmpult	t6, 10, t4		# first digit < 10?

	/* Handle F-format */

	beq	a3, 18f
	# Add corrected exponent+1 to digits after point to get
	# the number of digits to produce
	subq	t4, v0, t5		# ndigits - ((ld < 10) - v0) + 2
	subq	a1, t5, a1		# = ndigits + v0 + (ld >= 10) + 1
	addq	a1, 2, a1

	bgt	a1, 17f
	# ndigits <= 0
	xor	t4, 1, t5
	addq	v0, t5, v0
	blt	a1, 6f			# return null string if ndigits < 0
	# ndigits = 0, return "" if round-down, or "1" if round-up
	bne	t4, 19f			# br if first digit < 10
	cmpult	t6, 50, t5
	bne	t5, 6f
	cmpule	t6, 50, t5
	beq	t5, 7f
	br	zero, 55f

17:	cmple	a1, 17, t5		# more than 17 digits?
	cmoveq	t5, 17, a1		# clamp at 17

18:	addq	a1, a0, a1		# stop point in buffer
	bne	t4, 3f			# br if first digit < 10

	/* log10 approximation was low by 1, so we got first "digit" > 9 */

	addq	v0, 1, v0
#if 1	/* is this necessary??? */
	ldiq	t7, ZERO
185:	subq	t6, 10, t6
	addq	t7, 1, t7
	cmpult	t6, 10, t4
	beq	t4, 185b
#else	/* or will this suffice?? */
	ldiq	t7, ZERO+1
	subq	t6, 10, t6
#endif
	ldq_u	t4, (a0)		# store extra digit
	mskbl	t4, a0, t4
	insbl	t7, a0, t7
	bis	t4, t7, t7
	stq_u	t7, (a0)
	addq	a0, 1, a0		# advance the buffer pointer
	cmpeq	a0, a1, t4
	beq	t4, 3f			# br if more room in buffer
19:	cmpult	t6, 5, t4
	bne	t4, 6f
	cmpule	t6, 5, t4
	beq	t4, 7f
	br	zero, 55f

3:	addq	t6, ZERO, t6		# convert to ascii digit
	ldq_u	t4, (a0)		# store the digit
	mskbl	t4, a0, t4
	insbl	t6, a0, t7
	bis	t4, t7, t7
	stq_u	t7, (a0)
	addq	a0, 1, a0		# advance the buffer pointer
	cmpeq	a0, a1, t4
	bne	t4, 5f			# br if buffer full

	/* Now produce digits by multiplying fraction by 10 and taking */
	/* integer part.  Actually multiply by 5 (it's easier) and take */
	/* integer part from decreasing bit positions. */

4:	srl	t0, 62, t3		# get carry of t0*4
	sll	t0, 2, t5		# multiply t0 by 4
	eqv	t5, 0, t4		# get carry from addition
	cmpult	t4, t0, t4
	addq	t3, t4, t4		# combine carries
	addq	t0, t5, t0		# (t0*4)+t0 = t0*5

	s4addq	t1, t1, t1		# multiply t1 by 5
	addq	t1, t4, t1		# add carry out of t0

	subq	t8, 1, t8
	srl	t1, t8, t6
	sll	t6, t8, t7
	subq	t1, t7, t1
	addq	t6, ZERO, t6		# convert to ascii digit
	ldq_u	t4, (a0)		# store digit
	mskbl	t4, a0, t4
	insbl	t6, a0, t7
	bis	t4, t7, t7
	stq_u	t7, (a0)
	addq	a0, 1, a0		# advance buffer pointer
	cmpeq	a0, a1, t4
	beq	t4, 4b			# continue if more room in buffer

	/* Digit production complete.  Now round */

5:	subq	t8, 1, t8
	srl	t1, t8, t7
	beq	t7, 6f

55:	bne	t2, 7f
	subq	t1, 1, t7
	bne	t1, 7f
	and	t7, t1, t7
	bne	t0, 7f
	and	t6, 1, t6
	bne	t7, 7f
	bne	t6, 7f

	/* Round down, i.e. done */

6:	ldq_u	t0, (a0)		# terminate the buffer
	mskbl	t0, a0, t0
	stq_u	t0, (a0)
	lda	sp, FRMSIZ(sp)
	RET

	/* Round up (in ascii!) */

7:	subq	a0, 1, t1
75:	ldq_u	t2, (t1)		# get previous digit
	extbl	t2, t1, t0
	mskbl	t2, t1, t2		# prepare for an eventual store
	cmpeq	t0, ZERO+9, t4		# br if digit '9'
	bne	t4, 8f
	cmpult	t0, ZERO, t4		# br if less than '0'
	bne	t4, 9f			# ' ' and '-' both < '0'
	addq	t0, 1, t0
	insbl	t0, t1, t0		# replace the digit
	bis	t0, t2, t0
	stq_u	t0, (t1)
	ldq_u	t0, (a0)		# terminate the buffer
	mskbl	t0, a0, t0
	stq_u	t0, (a0)
	lda	sp, FRMSIZ(sp)
	RET

8:	ldiq	t0, ZERO
	insbl	t0, t1, t0		# store a '0'
	bis	t2, t0, t0
	stq_u	t0, (t1)
	subq	t1, 1, t1		# backup buffer pointer
	br	zero, 75b

	# Tried to round into sign position

9:	addq	v0, 1, v0
	ldiq	t0, ZERO+1
	beq	a3, 91f
	ldiq	t4, ZERO
	ldq_u	t7, (a0)
	mskbl	t7, a0, t7
	insbl	t4, a0, t4
	bis	t4, t7, t7
	stq_u	t7, (a0)
	addq	a0, 1, a0
91:	ldq_u	t7, 1(t1)
	lda	t1, 1(t1)
	mskbl	t7, t1, t7
	insbl	t0, t1, t0
	bis	t0, t7, t7
	stq_u	t7, (t1)
	ldq_u	t0, (a0)		# store new final null
	mskbl	t0, a0, t0
	stq_u	t0, (a0)
	lda	sp, FRMSIZ(sp)
	RET

	/* Output 0 or denorm */

10:	bne	t2, 12f			# br if non-zero (denorm)

	/* Zero */

	/*
	 * Neither 4.3BSD nor SunOS 3.something-or-other
	 * output an extra zero for f-format (from fcvt)
	 * compared to e-format (ecvt) for zero.
	 * That is, they don't seem to believe that a zero
	 * is 0.0000 and requires a digit preceeding
	 * the decimal -- at least in this context.
	 *
	 * This code is removed for compatibility since what the
	 * "real" answer is isn't obvious to me.
	 * The code is left here in case it turns out that this
	 * is a mistake and should be "un"-repaired.
	 *
	 * addq	a1, a3, a1
	 */
	ble	a1, 40f
	ldiq	t2, ZERO
	addq	a1, a0, a1
11:	ldq_u	t4, (a0)
	mskbl	t4, a0, t4
	insbl	t2, a0, t0
	bis	t0, t4, t0
	stq_u	t0, (a0)
	addq	a0, 1, a0
	cmpeq	a0, a1, t4
	beq	t4, 11b
40:	ldq_u	t0, (a0)
	mskbl	t0, a0, t0
	stq_u	t0, (a0)
	ldil	v0, 0
	lda	sp, FRMSIZ(sp)
	RET

	/* denorm */
	/* Normalize with slow but small loop (denorm speed is unimportant) */

12:	ldiq	t8, -1022		# initialize biased exponent
	ldiq	t4, 1			# get hidden bit mask
	sll	t4, 53, t4
13:	subq	t8, 1, t8		# adjust the exponent for shift
	sll	t2, 1, t2		# multiply fraction by 2
	and	t4, t2, t0		# isolate hidden bit
	beq	t0, 13b			# br if still not set

	/* log10(2) approximation */

	sll	t8, 52, t1		# shift exponent into place
	addq	t1, t2, t1		# combine with fraction
	subq	t1, t4, t1		# remove hidden bit
	sra	t1, 32, t1		# left justify exp in low 32 bits
	ldiq	t4, (0x4d104d42 >> 20) + 1 # log10(2) approximation
	mulq	t1, t4, v0		# get decimal exponent
	sra	v0, 32, v0		# throw out fractional part
	addq	t8, 1023, t8
	br	zero, 15b

	/* Output +/- Infinity or NaN */

20:	addq	a1, a0, a1		# stop point in buffer
	lda	t0, nan_inf		# assume NaN
	sll	t2, 11, t1		# shift fraction into bit 63
	addq	t0, 4, t3		# point at infinity string
	cmoveq	t1, t3, t0		# zero fraction: infinity
22:	ldq_u	t1, 0(t0)		# get next character from string
	extbl	t1, t0, t1
	addq	t0, 1, t0		# advance string pointer
	ldq_u	t2, 0(a0)		# get buffer quadword
	mskbl	t2, a0, t2		# merge next character in...
	insbl	t1, a0, t1
	addq	a0, 1, a0		# advance buffer pointer
	bis	t1, t2, t2
	stq_u	t2, -1(a0)		# stored merged buffer quadword
	cmoveq	t1, a1, a0		# force end when null byte is moved
	cmpeq	a0, a1, t4		# see if at stop point
	beq	t4, 22b			# br if not
	ldil	v0, 0x80000000
	lda	sp, FRMSIZ(sp)
	RET
END(_dtoa)

	.rdata
nan_inf:
	.ascii	"Nan\0Infinity\0"
