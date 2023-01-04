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
 *	@(#)$RCSfile: frexp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:13:06 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

#include <mips/regdef.h>
#include <mips/asm.h>
#include <fp_class.h>
#include <mips/softfp.h>
#define _KERNEL
#include <errno.h>

#define	FRAME_SIZE	24
#define	LOCAL_SIZE	0
#define	F12_OFFSET	FRAME_SIZE+4*0
#define	A2_OFFSET	FRAME_SIZE+4*2
#define	RA_OFFSET	FRAME_SIZE-LOCAL_SIZE-4*1

/*
 * double frexp(value, eptr)
 * double value; int *eptr;
 *   In assembly:
 *	value -- $f12 ($f13,$f12) as a double
 *	eptr  -- a2
 *   and the return values:
 *      return value -- $f0
 *	*eptr        -- 0(a2)
 *
 * Frexp() returns a double x such that x = 0 or 0.5 <= |x| < 1.0
 * and stores an integer n such that value = x * 2 ** n indirectly
 * through eptr.
 */
NESTED(frexp, FRAME_SIZE, ra)
	.mask	0x80000000, -(LOCAL_SIZE+4)

	subu	sp,FRAME_SIZE
	s.d	$f12,F12_OFFSET(sp)
	sw	a2,A2_OFFSET(sp)
	sw	ra,RA_OFFSET(sp)

	# get the class of the floating point value and switch on it.
	jal	fp_class_d
	sll	v0,2	
	lw	v0,jmp_tbl(v0)
	j	v0

	.rdata
jmp_tbl:
	.word	nan
	.word	nan
	.word	inf
	.word	inf
	.word	norm
	.word	norm
	.word	denorm
	.word	denorm
	.word	zeroval
	.word	zeroval
	.text
	
nan:	# For NaNs return the default quiet nan for both the return
	# value and the integer n.
	li	t1, EDOM
	sw	t1, errno
	li	a0,DQUIETNAN_LESS
	li	a1,DQUIETNAN_LEAST
	mtc1	a0,$f1
	mtc1	a1,$f0
	lw	a2,A2_OFFSET(sp)
	li	a0,WQUIETNAN_LEAST
	sw	a0,0(a2)
	j	ret

inf:	# For infinities return the infinity and return the maximum value
	# for the integer n.
	li	t1, EDOM
	sw	t1, errno
	l.d	$f0,F12_OFFSET(sp)
	lw	a2,A2_OFFSET(sp)
	li	a0,WORD_MAX
	sw	a0,0(a2)
	j	ret

norm:	# For normalized numbers return the value with the exponent changed
	# to -1.  The interger n gets the exponent of the value minus 1.
	l.d	$f0,F12_OFFSET(sp)
	mfc1	a0,$f1
	and	a1,a0,~(DEXP_MASK<<DEXP_SHIFT)
	or	a1,((DEXP_BIAS-1)<<DEXP_SHIFT)
	mtc1	a1,$f1
	lw	a2,A2_OFFSET(sp)
	srl	a0,DEXP_SHIFT
	and	a0,DEXP_MASK
	subu	a0,DEXP_BIAS-1
	sw	a0,0(a2)
	j	ret

denorm:	# For denormalized numbers return the value with the exponent changed
	# to -1 and the value normalized.  The interger n gets the exponent of
	# the value minus 1 adjusted for the denorms value.
	l.d	$f0,F12_OFFSET(sp)
	mfc1	a2,$f1
	mfc1	a3,$f0

	# set the exponent (a1) separate the fraction (a2,a3) and sign (a0)
	and	a0,a2,SIGNBIT
	li	a1,-DEXP_BIAS+1-1
	and	a2,DFRAC_MASK

	/*
	 * Renormalize the denormalized double value.
	 */
	/*
	 * The first step in this process is to determine where the first
	 * one bit is in the fraction (a2,a3).  After this series of tests
	 * the shift count to shift the fraction left so the first 1 bit is
	 * in the high bit will be in t9.  This sequence of code uses registers
	 * v0,v1 and t9 (it could be done with two but with reorginization this
	 * is faster).
	 */
	move	v0,a2
	move	t9,zero
	bne	a2,zero,1f
	move	v0,a3
	addu	t9,32
1:
	srl	v1,v0,16
	bne	v1,zero,1f
	addu	t9,16
	sll	v0,16
1:	srl	v1,v0,24
	bne	v1,zero,2f
	addu	t9,8
	sll	v0,8
2:	srl	v1,v0,28
	bne	v1,zero,3f
	addu	t9,4
	sll	v0,4
3:	srl	v1,v0,30
	bne	v1,zero,4f
	addu	t9,2
	sll	v0,2
4:	srl	v1,v0,31
	bne	v1,zero,5f
	addu	t9,1
5:
	/*
	 * Now that it is known where the first one bit is calculate the
	 * amount to shift the fraction to put the first one bit in the
	 * implied 1 position (also the amount to adjust the exponent by).
	 * Then adjust the exponent and shift the fraction.
	 */
	subu	t9,DFRAC_LEAD0S	# the calulated shift amount
	subu	a1,t9		# adjust the exponent
	blt	t9,32,1f
	subu	t9,32		# shift the fraction for >= 32 bit shifts
	sll	a2,a3,t9
	move	a3,zero
	b	2f
1:
	negu	v0,t9		# shift the fraction for < 32 bit shifts
	addu	v0,32
	sll	a2,t9
	srl	v1,a3,v0
	or	a2,v1
	sll	a3,t9
2:
	# Now put the normalized value together with a -1 exponent
	and	a2,~DIMP_1BIT
	or	a2,((DEXP_BIAS-1)<<DEXP_SHIFT)
	or	a2,a0
	mtc1	a2,$f1
	mtc1	a3,$f0

	# Now store the integer n
	lw	a2,A2_OFFSET(sp)
	sw	a1,0(a2)
	j	ret


zeroval: # For zeroes return zero for both the return value and the
	 # integer n.
	l.d	$f0,F12_OFFSET(sp)
	lw	a2,A2_OFFSET(sp)
	sw	zero,0(a2)
	# Continue on to common return

ret:	# Common return needed by picie
	lw	ra,RA_OFFSET(sp)
	addu	sp,FRAME_SIZE
	j	ra

END(frexp)
