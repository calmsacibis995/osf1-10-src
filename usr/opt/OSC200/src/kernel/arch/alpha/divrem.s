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
 * This code is the main "guts" of all the divide and remainder
 * RTL routines. It is intended to be included by a wrapper which
 * defines macros to provide entry and return code, and parameters
 * to indicate 32 vs 64-bit, divide vs remainder, etc.
 *
 * The algorithm used here is based on long division and a table for
 * approximating inverses as discussed in the paper "Division by a
 * Constant" by Mary Payne and Robert Gries. If the divisor inverse can
 * be constructed from the table without an error, the division is performed
 * as described in the paper with a multiplication and a shift.  
 *
 * If the inverse can not be found in the table, we improve the inverse with
 * a linear approximation, "I". A multiplication by "I" and a shift by
 * log2(y) is used to obtain an approximate quotient, "Q". Now, like long
 * division, the most significant bits are correct, therefore if we calculate
 * the remainder R = x-Q*y, R will be smaller than x, and R will contain
 * the true remainder and the error in Q, "e" multiplied by y.
 *
 * So if we do the same multiplication and shift, we will get an approxmation
 * for e. This is just long division, and it will finish when R, the remainder
 * is less than y.
 *
 * On input:
 *      $23/t9  = return address (RA)
 *      $24/t10 = dividend (numerator) (A)
 *      $25/t11 = divisor (denominator) (B)
 *
 * On Return:
 *      $27/t12 = quotient/remainder (Q)
 *
 * Register Usage:
 *	$at, $gp, t0..t3, a0 and t9/RA are destroyed; all others preserved
 *      (t0..t3, a0 and t9/RA are preserved by the wrapper)
 *
 * Macros defined by wrapper:
 *	ENTRY - code for procedure prolog, overflow checks, absolute
 *		value of operands, etc.
 *	THIRTYTWOBIT - defined if 32-bit division/remainder; 64-bit
 *		if not defined.
 *	UNSIGNED - defined if unsigned division/remainder; signed
 *		if not defined.
 *	REMAINDER - defined if remainder operation; division operation
 *		if not defined.
 *	RETURN - code for epilog
 */

#ifndef KERNEL
#include <alpha/pal.h>
#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <alpha/gentrap.h>
#else
#include <machine/pal.h>
#include <machine/regdef.h>
#include <machine/asm.h>
#include <machine/gentrap.h>
#endif
#include "divglobl.h"

	.text
	.set	noat
	.align	4
	ENTRY				# generate prolog and setup

	lda	t3, __divdat+TABLE_BIAS	# point to tables

	# Check for division by zero and cases where quotient can only
	# be zero or one
	#
	#	t3	pointer to tables
	#	A	numerator
	#	B	denominator
	#
	#  if (B==0) raise(div_by_zero);
	#  if ((B >> (size_uint64-1)) != 0) return A>=B;
	#  if (B>=A) return A==B; /* common case */
	#  shift= log2(B);

	beq	B, divzero		# signal divide by zero
	cmpule	A, B, t0		# denom >= numer?
#ifdef THIRTYTWOBIT
	addl	B, 0, t1		# sign-extend denom
	blt	t1, zero_one		# quot is 0 or 1 if neg denom
#else
	blt	B, zero_one		# quot is 0 or 1 if neg denom
#endif
	cmpbge	zero, B, t2		# get mask of null denom bytes
	bne	t0, zero_one		# quot is 0 or 1 if denom >= numer
	xor	t2, 0xff, t0		# get mask of non-null denom bytes
	s4addq	t0, t3, t0		# get table index
	ldl	AT, LOG2TAB(t0)		# offset of highest non-null byte
	extbl	B, AT, t0		# extract highest non-null byte
	s4addq	t0, t3, t0		# get table index
	ldl	t0, LOG2TAB(t0)		# offset of highest non-null bit
	s8addq	AT, t0, AT		# combine bit and byte offsets

	# Normalize the denominator (shift highest one bit into MSB) and
	# check to see if number of bits between the most-significant-one-bit
	# and the least-significant-one-bit is BIT_LENGTH+1 bits or less; if
	# so we can take a short cut
	#
	#	t3	pointer to tables
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#
	#  if ((shift-(bit_length+1) <= 0) ||
	#    ((B << (size_uint64-shift+(bit_length+1))) == 0))
	#      goto small;

	ornot	zero, AT, t0		# get (63-shift) in low 6 bits
	sll	B, t0, t0		# normalize denominator
	sll	t0, BIT_LENGTH+1, t1	# remove top bits from normalized denom
	bne	t1, large		# special case denoms with lots of bits

/*
 * The number of bits between the most-signficant-one-bit (MSOB) and the
 * least-significant-one-bit (LSOB) is BIT_LENGTH+1 or less which means
 * we can guarantee our approximation to 1/B is actually exact.
 * As a side effect, this path also handles all cases of dividing
 * by a power of two (since the number of bits between MSOB and LSOB
 * is one!).
 *
 *	A	numerator
 *	B	denominator
 *	AT	log2 of denominator (shift)
 *	t0	normalized denominator
 *	t3	pointer to tables
 */

	#  if ((B - 1) & B) return A >> shift; /* power of two */
	#  if (shift>(bit_length+1)) {
	#    j= integer(B >> (shift-(bit_length+1)));
	#  } else {
	#    j= integer(B << ((bit_length+1)-shift));
	#  }

	subq	B, 1, t1		# flip divisor bits up to LS-one-bit
	and	B, t1, t2		# set non-zero if not power of 2
	srl	t0, 63-(BIT_LENGTH+1), t12 # shift signif divisor bits to LSB
	beq	t2, pow_2		# br if divisor is power of 2

	#  j= j-(bit_value+bit_value);

 	subq	t12, BIT_VALUE+BIT_VALUE, t12 # clear MS-one-bit

	#  inverse = inv[j >> 1];

	bic	t12, 1, a0		# (j>>1)*2
	s8addq	a0, t3, a0		# &__divdat+(j*16)
	ldq	t9, INV(a0)

	#  if (j & 1) inverse -= inv_m[j >> 1];

	ldq	a0, INV_M(a0)
	subq	t9, a0, a0
	cmovlbs	t12, a0, t9

	#  switch(inv_flag[j]) {
	#  case 1:
	#    /* code option 1 for rounded dividend */
	#    a= A+1;
	#    if (a == 0)
	#      c= inverse >> shift;
	#    else
	#      c= __UMULH(a,inverse) >> shift;
	#    r= A-(B*c);
	#    break;
	#
	#  case 0:
	#    /* code option 0 for rounded inv */
	#    c= __UMULH(A,inverse) >> shift;
	#    r= A-(B*c);
	#    break;
	#  }

	addq	t12, t3, t2		# &inv_flag+j
	ldq_u	t0, INV_FLAG(t2)
	extbl	t0, t2, t2

	addq	A, t2, t1
	umulh	t1, t9, t3
#if !defined(THIRTYTWOBIT) && defined(UNSIGNED)
	cmoveq	t1, t9, t3
#endif

	srl	t3, AT, Q
#ifdef REMAINDER
	mulq	Q, B, t0
	subq	A, t0, Q
#endif
	RETURN

/*
 * Handle cases when divisor is a power of 2
 *
 *	A	numerator
 *	B	denominator
 *	AT	log2 of denominator (shift)
 *	t1	B-1
 */
	.align	3
pow_2:
#ifdef REMAINDER
	#  r = A & (B-1);

	and	A, t1, Q
#else
	#  c = A >> shift;

	srl	A, AT, Q
#endif
	RETURN

/*
 * The quotient of A/B is guaranteed to be between 0 or 1
 *
 *	A	numerator
 *	B	denominator
 */
	.align	3
zero_one:
#ifdef REMAINDER
	cmpule	B, A, t0
	subq	A, B, Q
	cmoveq	t0, A, Q
#else
	cmpule	B, A, Q
#endif
	RETURN

/*
 * The number of bits between the most-significant-one-bit and the
 * least-significant-one-bit greather than BIT_LENGTH+1 which means
 * we only have an approximation to 1/divisor. Refine the inverse
 * until it is accurate enough.
 */
	.align	3
large:
	# From the most significant BIT_LENGTH+K_BIT_LENGTH bits of the
	# normalized denominator, extract the low K_BIT_LENGTH bits and
	# next BIT_LENGTH-1 bits
	#
	#	t3	pointer to tables
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t0	normalized denominator
	#
	#  if (shift>bit_length+k_bit_length) {
	#    k= B >> (shift-bit_length-k_bit_length);
	#  } else {
	#    k= B << (bit_length+k_bit_length-shift);
	#  }
	#  j= (k >> k_bit_length)-bit_value;
	#  k= k & k_bit_mask;

	srl	t0, 63-(BIT_LENGTH+K_BIT_LENGTH), t12 # shift signif bits to LSB
	srl	t12, K_BIT_LENGTH, t0	# extract high bits
 #	subq	t0, BIT_VALUE, t0	# folded into mem refs
#if K_BIT_LENGTH <= 8
	and	t12, K_BIT_MASK, t12	# extract low bits
#elif K_BIT_LENGTH <= 16
	ldiq	t2, K_BIT_MASK		# get low bits mask
	and	t12, t2, t12		# extract low bits
#else
	sll	t12, 64-K_BIT_LENGTH, t12 # extract low bits
	srl	t12, 64-K_BIT_LENGTH, t12
#endif

	# Get an approximation of 1/denominator
	#
	#	t3	pointer to tables
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t0	high bits of signif denom bits (j)
	#	t12	low bits of signif denom bits (k)
	#
	#if k_bit_length > bit_length
	#  inverse= inv[j] - (((inv_m[j] >> k_bit_length-bit_length) * k) 
	#	>> (bit_length-1));
	#else
	#  inverse= inv[j] - ((inv_m[j] * k) >> (k_bit_length-1));
	#endif

	addq	t0, t0, t0		# scale index (j) by 2
	s8addq	t0, t3, t1		# get pointer into __divdat
	ldq	t2, INV-(16*BIT_VALUE)(t1) # inv[j]
	ldq	t0, INV_M-(16*BIT_VALUE)(t1) # inv_m[j]
#if K_BIT_LENGTH > BIT_LENGTH
	srl	t0, K_BIT_LENGTH-BIT_LENGTH, t0
	mulq	t0, t12, t0
	srl	t0, BIT_LENGTH-1, t0
#else
	mulq	t0, t12, t0
	srl	t0, K_BIT_LENGTH-1, t0
#endif
	subq	t2, t0, t12		# inverse

	# Get an approximation of quotient and the remainder
	#
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t12	approx of 1/denominator (inverse)
	#
	#  c= __UMULH(A,inverse);
	#  c = c >> shift;
	#  r= B*c;

	umulh	A, t12, a0		# quotient
	srl	a0, AT, a0
	mulq	a0, B, t9		# remainder

	# If sign of remainder differs from sign of numerator and true
	# remainder is larger than 64 bits, set 'minus' flag
	#
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t12	approx of 1/denominator (inverse)
	#	a0	quotient (c)
	#	t9	remainder (r)
	#
	# int minus = 0;
	#  if ((r >> (size_uint64-1) ) ^ (A >> (size_uint64-1) )) {
	#      if (__UMULH(B,c)!=0) minus= 1;
	#  }
	#  r= A-r;

	bis	zero, zero, t3		# init 'minus'
#ifndef THIRTYTWOBIT
	xor	A, t9, t0		# see where rem and numer differ
	bge	t0, 10f			# skip if numer and rem signs match
	umulh	B, a0, t3		# set 'minus' if rem bits > 64
10:
#endif
	subq	A, t9, t9

	#  if (c!=0) {	/* kjl: isn't this always true??? */
	#    if (r>A) minus= 1;
	#    if (minus) r= -r;

	beq	a0, 40f
	cmpult	A, t9, t0
	or	t0, t3, t3
	negq	t9, t1
	cmovne	t3, t1, t9

	# Continue refining the quotient
	#
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t12	approx of 1/denominator (inverse)
	#	t3	'minus' flag
	#	a0	quotient (c)
	#	t9	remainder (r)
	#
	#    if (r>=(B+B))
	#    do {
	#      a= __UMULH(r,inverse);
	#      a = a >> shift;
	#      if (minus) c-= a; else c+= a;
	#      a= a*B;
	#      r-= a;

	addq	B, B, t0
	cmpule	t0, t9, t0
	beq	t0, 30f
20:	umulh	t9, t12, t2
	srl	t2, AT, t2
	negq	t2, t0
	cmoveq	t3, t2, t0
	addq	a0, t0, a0
	mulq	t2, B, t1	
	subq	t9, t1, t9

#ifndef THIRTYTWOBIT
	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t12	approx of 1/denominator (inverse)
	#	t3	'minus' flag
	#	a0	quotient (c)
	#	t9	remainder (r)
	#
	#      if ((r >> (size_uint64-1)) != 0) {
	#	 minus= !minus;
	#        r= -r;
	#      }

	cmpeq	t3, 0, t0
	negq	t9, t2
	cmovlt	t9, t0, t3
	cmovlt	t9, t2, t9
#endif

	#	A	numerator
	#	B	denominator
	#	AT	log2 of denominator (shift)
	#	t12	approx of 1/denominator (inverse)
	#	t3	'minus' flag
	#	a0	quotient (c)
	#	t9	remainder (r)
	#
	#      if (B+B>r) a=0; /* break */
	#    } while (a!=0);

	addq	B, B, t0
	cmpult	t9, t0, t0
	cmovne	t0, 0, t1
	bne	t1, 20b
30:
	#  }
40:
	#  if (B<=r) {
	#    r-= B;
	#    if (minus) c--; else c++; /* kjl: don't need this for remainder */
	#  }

	cmpule	B, t9, t0
	beq	t0, 50f
	subq	t9, B, t9
	subq	a0, 1, t0
	addq	a0, 1, a0
	cmovne	t3, t0, a0
50:

	#	B	denominator
	#	t3	'minus' flag
	#	a0	quotient (c)
	#	t9	remainder (r)
	#
	#  if (r==0) minus=0;

	cmoveq	t9, 0, t3
#ifdef REMAINDER
	#  if (minus) r = B-r;
	#  return r;

	subq	B, t9, Q
	cmoveq	t3, t9, Q
#else
	#  if (minus) c--;
	#  return c;

	subq	a0, 1, Q
	cmoveq	t3, a0, Q
#endif
	RETURN

/*
 * Divisor is zero -- generate a signal
 */
	# .align 3 /* who cares if we save a cycle here... */
divzero:
	ldiq	a0, GEN_INTDIV
	call_pal PAL_gentrap

	# if the user continues from the gentrap, return a zero result

	ldiq	Q, 0
	RETURN

	.set	at
	.end
