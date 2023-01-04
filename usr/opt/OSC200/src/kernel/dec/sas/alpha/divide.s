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
 * These divide routines are different than the versions used by the
 * kernel and libc since in this use the size of the code is the
 * most important aspect -- performance doesn't matter
 */

	.rdata
	.asciiz "@(#)$RCSfile: divide.s,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/01/19 15:27:40 $"
	.text

#include <machine/regdef.h>
#include <machine/asm.h>

/*
 * For the kernel we elect NOT to check for division by zero nor overflow.
 * If we wanted to make those checks, we'd need this symbol defined
 * from machine/gentrap.h: GEN_INTDIV.
 */
#define NOCHECK_DIVZERO 1
#define NOCHECK_OVERFLOW 1


	.text
	.set	noat		# so we don't accidently use $at

/*
 * Signed division.
 *
 * On input:
 *	$23 = return address
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *	Note that the high 32 bits of r24 and r25 are ignored.
 *
 * On Return:
 *	$27 = quotient
 *	All other registers, except gp, are preserved
 */

NESTED(__divl, 6*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x80001c, -6*8
	lda	sp, -6*8(sp)
	stq	$24, 4*8(sp)
	stq	$25, 3*8(sp)
	stq	$4, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

	addl	$24, 0, $24	# make sure dividend is in canonical form
	addl	$25, 0, $25	# make sure divisor is in canonical form

#ifndef NOCHECK_OVERFLOW
	# check for INT_MIN / -1
	addl	$25, 1, $2	# 0 if $25 == -1; != 0 otherwise
	bis	$24, $24, $3	# copy dividend
	cmovne	$2, 0, $3	# replace w/ 0 if divisor != -1
	sublv	zero, $3, $3	# trap if dividend = INT_MIN
#endif

	xor	$24, $25, $4	# compute sign of quotient
	cmplt	$24, 0, $2	# sign of dividend is sign of remainder
	bic	$4, 1, $4	# use low bit for remainder sign
	bis	$2, $4, $4	# merge in with quotient sign

	subl	zero, $24, $2	# negate dividend
	cmovlt	$24, $2, $24	# get absolute value of dividend
	subl	zero, $25, $2	# negate divisor
	cmovlt	$25, $2, $25	# get absolute value of divisor

	ldiq	$2, 32/4	# loop iterations

	sll	$25, 32, $25	# move divisor up to high 32 bits
	zap	$24, 0xf0, $24	# zero-extend dividend to 64 bits

10:	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	subq	$2, 1, $2	# any more iterations?
	bne	$2, 10b

	addl	$24, 0, $27	# get quotient into canonical form

	subl	zero, $27, $2	# negate quotient into a temp
	cmovlt	$4, $2, $27	# if quotient should be negative copy temp

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$4, 2*8(sp)
	ldq	$25, 3*8(sp)
	ldq	$24, 4*8(sp)
	lda	sp, 6*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__divl)

/*
 * Signed division.
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *	Note that the high 32 bits of r26 and r27 are ignored.
 *
 * On Return:
 *	$27 = quotient
 *	All other registers, except gp, are preserved
 */

NESTED(__reml, 6*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x80001c, -6*8
	lda	sp, -6*8(sp)
	stq	$24, 4*8(sp)
	stq	$25, 3*8(sp)
	stq	$4, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

	addl	$24, 0, $24	# make sure dividend is in canonical form
	addl	$25, 0, $25	# make sure divisor is in canonical form

#ifndef NOCHECK_OVERFLOW
	# check for INT_MIN / -1
	addl	$25, 1, $2	# 0 if $25 == -1; != 0 otherwise
	bis	$24, $24, $3	# copy dividend
	cmovne	$2, 0, $3	# replace w/ 0 if divisor != -1
	sublv	zero, $3, $3	# trap if dividend = INT_MIN
#endif

	xor	$24, $25, $4	# compute sign of quotient
	cmplt	$24, 0, $2	# sign of dividend is sign of remainder
	bic	$4, 1, $4	# use low bit for remainder sign
	bis	$2, $4, $4	# merge in with quotient sign

	subl	zero, $24, $2	# negate dividend
	cmovlt	$24, $2, $24	# get absolute value of dividend
	subl	zero, $25, $2	# negate divisor
	cmovlt	$25, $2, $25	# get absolute value of divisor

	ldiq	$2, 32/4	# loop iterations

	sll	$25, 32, $25	# move divisor up to high 32 bits
	zap	$24, 0xf0, $24	# zero-extend dividend to 64 bits

10:	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	subq	$2, 1, $2	# any more iterations?
	bne	$2, 10b

	sra	$24, 32, $27	# extract remainder in canonical form

	subl	zero, $27, $2	# negate remainder into a temp
	cmovlbs	$4, $2, $27	# if remainder should be negative copy temp

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$4, 2*8(sp)
	ldq	$25, 3*8(sp)
	ldq	$24, 4*8(sp)
	lda	sp, 6*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__reml)

#ifndef OSF_BOOT

/*
 * Unsigned division.
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *	Note that the high 32 bits of r25 and r27 are ignored.
 *
 * On Return:
 *	$27 = quotient
 *	All other registers, except gp, must be preserved
 */

NESTED(__divlu, 4*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x80100c, -4*8
	lda	sp, -4*8(sp)
	stq	$24, 3*8(sp)
	stq	$25, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

	ldiq	$2, 32/4	# set iteration count

	sll	$25, 32, $25	# move divisor up to high 32 bits
	zap	$24, 0xf0, $24	# zero-extend dividend to 64 bits

10:	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	subq	$2, 1, $2	# any more iterations?
	bne	$2, 10b

	addl	$24, 0, $27	# get quotient into canonical form

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$25, 2*8(sp)
	ldq	$24, 3*8(sp)
	lda	sp, 4*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__divlu)

/*
 * Signed division.
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *
 * On Return:
 *	$27 = quotient
 *	All other registers, except gp, must be preserved
 */

NESTED(__divq, 6*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x0280003c, -6*8
	lda	sp, -6*8(sp)
	stq	$24, 5*8(sp)
	stq	$25, 4*8(sp)
	stq	$5, 3*8(sp)
	stq	$4, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

#ifndef NOCHECK_OVERFLOW
	# check for LONG_MIN / -1
	addq	$25, 1, $2	# 0 if $25 == -1; != 0 otherwise
	bis	$24, $24, $3	# copy dividend
	cmovne	$2, 0, $3	# replace w/ 0 if divisor != -1
	subqv	zero, $3, $3	# trap if dividend = LONG_MIN
#endif

	xor	$24, $25, $4	# compute sign of quotient
	cmplt	$24, 0, $2	# sign of dividend is sign of remainder
	bic	$4, 1, $4	# use low bit for remainder sign
	bis	$2, $4, $4	# merge in with quotient sign

	subq	zero, $24, $2	# negate dividend
	cmovlt	$24, $2, $24	# get absolute value of dividend
	subq	zero, $25, $2	# negate divisor
	cmovlt	$25, $2, $25	# get absolute value of divisor

	ldiq	$3, 0		# zero-extend dividend to 128 bits

	ldiq	$5, 64/4	# loop iterations

10:	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	subq	$5, 1, $5	# see if more iterations to go
	bne	$5, 10b

	subq	zero, $24, $2	# negate quotient into a temp
	cmovlt	$4, $2, $24	# if quotient should be negative copy temp
	bis	$24, $24, $27

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$4, 2*8(sp)
	ldq	$5, 3*8(sp)
	ldq	$25, 4*8(sp)
	ldq	$24, 5*8(sp)
	lda	sp, 6*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__divq)

/*
 * Unsigned division.
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *
 * On Return:
 *	$27 = quotient
 *	All other registers, except gp, must be preserved
 */

NESTED(__divqu, 4*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x80001c, -4*8
	lda	sp, -4*8(sp)
	stq	$24, 3*8(sp)
	stq	$4, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

	ldiq	$4, 64/4	# set iteration count

	ldiq	$3, 0		# zero-extend dividend to 128 bits

10:	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	subq	$4, 1, $4	# any more iterations?
	bne	$4, 10b

	bis	$24, $24, $27

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$4, 2*8(sp)
	ldq	$24, 3*8(sp)
	lda	sp, 4*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__divqu)

/*
 * Unsigned remainder.
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *	Note that the high 32 bits of r25 and r27 are ignored.
 *
 * On Return:
 *	$27 = remainder
 *	All other registers, except gp, must be preserved
 */

NESTED(__remlu, 4*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x80100c, -4*8
	lda	sp, -4*8(sp)
	stq	$24, 3*8(sp)
	stq	$25, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

	ldiq	$2, 32/4	# set iteration count

	sll	$25, 32, $25	# move divisor up to high 32 bits
	zap	$24, 0xf0, $24	# zero-extend dividend to 64 bits

10:	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	addq	$24, $24, $24	# shift dividend left a bit

	cmpule	$25, $24, $3	# is dividend >= divisor?
	addq	$24, $3, $24	# set quotient bit if dividend >= divisor
	subq	$24, $25, $3	# subtract divisor from dividend...
	cmovlbs	$24, $3, $24	# ...if dividend >= divisor

	subq	$2, 1, $2	# any more iterations?
	bne	$2, 10b

	sra	$24, 32, $27	# extract remainder in canonical form

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$25, 2*8(sp)
	ldq	$24, 3*8(sp)
	lda	sp, 4*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__remlu)

/*
 * Signed division.
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *
 * On Return:
 *	$27 = remainder
 *	All other registers, except gp, must be preserved
 */

NESTED(__remq, 6*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x0280003c, -6*8
	lda	sp, -6*8(sp)
	stq	$24, 5*8(sp)
	stq	$25, 4*8(sp)
	stq	$5, 3*8(sp)
	stq	$4, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

#ifndef NOCHECK_OVERFLOW
	# check for LONG_MIN / -1
	addq	$25, 1, $2	# 0 if $25 == -1; != 0 otherwise
	bis	$24, $24, $3	# copy dividend
	cmovne	$2, 0, $3	# replace w/ 0 if divisor != -1
	subqv	zero, $3, $3	# trap if dividend = LONG_MIN
#endif

	xor	$24, $25, $4	# compute sign of quotient
	cmplt	$24, 0, $2	# sign of dividend is sign of remainder
	bic	$4, 1, $4	# use low bit for remainder sign
	bis	$2, $4, $4	# merge in with quotient sign

	subq	zero, $24, $2	# negate dividend
	cmovlt	$24, $2, $24	# get absolute value of dividend
	subq	zero, $25, $2	# negate divisor
	cmovlt	$25, $2, $25	# get absolute value of divisor

	ldiq	$3, 0		# zero-extend dividend to 128 bits

	ldiq	$5, 64/4	# loop iterations

10:	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	subq	$5, 1, $5	# see if more iterations to go
	bne	$5, 10b

	subq	zero, $3, $24	# copy negated remainder to return reg
	cmovlbc	$4, $3, $24	# if positive remainder then overwrite
	bis	$24, $24, $27

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$4, 2*8(sp)
	ldq	$5, 3*8(sp)
	ldq	$25, 4*8(sp)
	ldq	$24, 5*8(sp)
	lda	sp, 6*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__remq)

/*
 * Unsigned remainder
 *
 * On input:
 *	$24 = dividend (numerator)
 *	$25 = divisor (denominator)
 *
 * On Return:
 *	$27 = remainder
 *	All other registers, except gp, must be preserved
 */

NESTED(__remqu, 4*8, $23)
	/*
	 * ???? KJL: This prologue is incorrect. A stack frame procedure
	 * must save its return address. We lie here and set the
	 * appropriate mask bit so the code assembles without error.
	 * This will work since this code will never be in the call
	 * chain during a stack unwind. The code should be fixed to
	 * interchange the usage of $2 with $23.
	 */
	.mask	0x80001c, -4*8
	lda	sp, -4*8(sp)
	stq	$24, 3*8(sp)
	stq	$4, 2*8(sp)
	stq	$3, 1*8(sp)
	stq	$2, 0*8(sp)
	.prologue 0

#ifndef NOCHECK_DIVZERO
	beq	$25, 20f		# die if divisor is zero
#endif

	ldiq	$4, 64/4	# set iteration count

	ldiq	$3, 0		# zero-extend dividend to 128 bits

10:	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	cmplt	$24, 0, $2	# predict carry-out of low-dividend shift
	addq	$24, $24, $24	# shift low-dividend left
	addq	$3, $3, $3	# shift high-dividend left
	bis	$3, $2, $3	# merge in carry-out of low-dividend

	cmpule	$25, $3, $2	# is dividend >= divisor?
	addq	$24, $2, $24	# set quotient bit if dividend >= divisor
	subq	$3, $25, $2	# subtract divisor from dividend...
	cmovlbs	$24, $2, $3	# ...if dividend >= divisor

	subq	$4, 1, $4	# any more iterations?
	bne	$4, 10b

	bis	$3, $3, $27	# get remainder

	ldq	$2, 0*8(sp)
	ldq	$3, 1*8(sp)
	ldq	$4, 2*8(sp)
	ldq	$24, 3*8(sp)
	lda	sp, 4*8(sp)
	ret	zero, ($23), 1

#ifndef NOCHECK_DIVZERO
20:	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap	# punch out the user
#endif

END(__remqu)

#endif /* OSF_BOOT */
