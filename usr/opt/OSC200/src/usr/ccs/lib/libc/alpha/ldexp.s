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

#include <machine/regdef.h>
#include <machine/asm.h>

/*
 * double ldexp (value, exp)
 * double value;
 * int exp;
 *
 * Ldexp returns value*2**exp, if that result is in range.
 * If underflow occurs, it returns zero.  If overflow occurs,
 * it returns a value of appropriate sign and largest
 * possible magnitude.  In case of either overflow or underflow,
 * errno is set to ERANGE.  Note that errno is not modified if
 * no error occurs.
 *
 *	f16	value
 *	a1	exp
 */


#if 0
/* ??? some how should be able to load infinity directly so we avoid */
/* ??? multiple copies (pick one of the below methods...) */
#define INFINITY 0x0h0x7ff
#define INFINITY 0x7ff0000000000000
#else
	.sdata
infinity:
	.quad	0x7ff0000000000000
#endif

#define	FRMSIZ 16	/* one temp plus padding to octaword align */

	.text

NESTED(ldexp, FRMSIZ, ra)
	ldgp	gp, 0(pv)		# setup gp
	lda	sp, -FRMSIZ(sp)		# create stack frame
	.prologue	1
	cpys	$f16, $f16, $f0		# move "value" to retval
	stt	$f16, 0(sp)		# copy "value" to int reg...
	ldq	t0, 0(sp)		#  using temp location
	sll	t0, 1, t1		# remove sign bit
	srl	t1, 53, t1		# isolate exponent
	sll	t0, 12, t0		# move fraction to msb
	addq	t1, a1, t2		# add on exponent
	beq	t1, 80f			# br if "value" denormalized or zero
10:	ble	t2, 50f			# br if result is denormalized
	cmplt	t2, 2047, t3		# check for exponent overflow
	beq	t3, 20f			# br if infinity or NaN
	sll	t2, 52, t2		# move exp into exponent field
	srl	t0, 12, t0		# restore fraction to LSB
	bis	t0, t2, t0		# merge in exponent
	stq	t0, 0(sp)		# copy result to retval...
	ldt	$f0, 0(sp)		#  using temp location
	cpys	$f16, $f0, $f0		# merge in sign of "value"
	lda	sp, FRMSIZ(sp)		# remove stack frame
	RET

/*
 * Return +/- infinity or NaN
 *
 *	t0 = frac (shifted to MSB)
 *	t1 = original exponent
 *	f0, f16 = original "value"
 */

20:
	cmpeq	t1, 2047, t1		# could "value" have been Nan?
	beq	t1, 30f			# br if not
	bne	t0, 40f			# br if non-zero ("value" was Nan)
#if 0
30:	ldit	$f0, INFINITY		# load positive infinity
30:	ldiq	$f0, INFINITY		# load positive infinity
#else
30:	ldt	$f0, infinity		# load positive infinity
#endif
	cpys	$f16, $f0, $f0		# merge in sign of "value"
40:	lda	sp, FRMSIZ(sp)		# remove stack frame
        ldiq    v0,34                   # ERANGE
        jmp      zero,_cerror            # set errno,(cerror does an RET)
	RET

/*
 * Return denormalized value
 *
 *	t0 = frac (shifted to MSB)
 *	t2 = result exponent
 *	f0, f16 = original "value"
 */

50:	cmple	t2, -52, t1		# will there be any fraction left?
	bne	t1, 70f			# br if not and return zero
	srl	t0, 12, t0		# restore fraction to LSB
	ldiq	t1, 1			# turn on hidden bit
	sll	t1, 52, t1
	bis	t0, t1, t0
	subq	t2, 1, t2

	# Shift by amount of underflow
	sll	t0, t2, t1		# move bits shifted out to msb
	subq	zero, t2, t2		# get positive shift count
	srl	t0, t2, t0		# shift fraction by exponent
	bge	t1, 60f			# br if no rounding required

	# Round up
	addq	t0, 1, t0
	sll	t1, 1, t1
	bne	t1, 60f
	bic	t0, 1, t0

60:	beq	t0, 70f			# br if result is zero
	stq	t0, 0(sp)		# copy result to retval...
	ldt	$f0, 0(sp)		#  using temp location
	cpys	$f16, $f0, $f0		# merge in sign of "value"
	lda	sp, FRMSIZ(sp)		# remove stack frame
	RET

/*
 * Return zero
 *
 *	f0, f16 = original "value"
 */

70:
	cpys	$f16, $f31, $f0		# set retval to zero with same
					# sign as "value"
	lda	sp, FRMSIZ(sp)		# remove stack frame
        ldiq    v0,34                   # ERANGE
        jmp      zero,_cerror            # set errno,(cerror does an RET)
	RET

/*
 * Input value was denormalized or zero
 *
 *	t0 = frac (shifted to MSB)
 *	t2 = result exponent
 *	f0, f16 = original "value"
 */

80:	beq	t0, 40b			# zero returns zero
	blt	t0, 100f		# br if frac already normalized

	# Normalize by removing leading zero bits
90:	sll	t0, 1, t0		# shift fraction left a bit
	subq	t2, 1, t2		# adjust final exp
	bgt	t0, 90b

100:	sll	t0, 1, t0		# remove hidden bit
	br	zero, 10b		# go see what we ended up with
END(ldexp)


