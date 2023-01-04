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
 * @(#)$RCSfile: conv.s,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/01/28 17:28:54 $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */

#include <asm.h>
#include <regdef.h>

.sdata
infinity:
	.word 0x7ff00000
infinity_s:
	.asciiz "Infinity"
nan_s:
	.asciiz "NaN"

.rdata
.align 3
	.word 0xfbd14d6e, 0xe1afa13a /* -364 */
	.word 0x4d8d98b8, 0xe3e27a44 /* -336 */
	.word 0x3d1a45df, 0xe61acf03 /* -308 */
	.word 0x8f5c22ca, 0xe858ad24 /* -280 */
	.word 0x23ee8bcb, 0xea9c2277 /* -252 */
	.word 0x4a314ebe, 0xece53cec /* -224 */
	.word 0x172aace5, 0xef340a98 /* -196 */
	.word 0xbc3f8ca2, 0xf18899b1 /* -168 */
	.word 0xdec3f126, 0xf3e2f893 /* -140 */
	.word 0xf065d37d, 0xf64335bc /* -112 */
	.word 0x88747d94, 0xf8a95fcf /* -84 */
	.word 0xbe068d2f, 0xfb158592 /* -56 */
	.word 0x8300ca0e, 0xfd87b5f2 /* -28 */
_pten:
	.word 0x00000000, 0x80000000 /* 0 */
	.word 0x00000000, 0xa0000000 /* 1 */
	.word 0x00000000, 0xc8000000 /* 2 */
	.word 0x00000000, 0xfa000000 /* 3 */
	.word 0x00000000, 0x9c400000 /* 4 */
	.word 0x00000000, 0xc3500000 /* 5 */
	.word 0x00000000, 0xf4240000 /* 6 */
	.word 0x00000000, 0x98968000 /* 7 */
	.word 0x00000000, 0xbebc2000 /* 8 */
	.word 0x00000000, 0xee6b2800 /* 9 */
	.word 0x00000000, 0x9502f900 /* 10 */
	.word 0x00000000, 0xba43b740 /* 11 */
	.word 0x00000000, 0xe8d4a510 /* 12 */
	.word 0x00000000, 0x9184e72a /* 13 */
	.word 0x80000000, 0xb5e620f4 /* 14 */
	.word 0xa0000000, 0xe35fa931 /* 15 */
	.word 0x04000000, 0x8e1bc9bf /* 16 */
	.word 0xc5000000, 0xb1a2bc2e /* 17 */
	.word 0x76400000, 0xde0b6b3a /* 18 */
	.word 0x89e80000, 0x8ac72304 /* 19 */
	.word 0xac620000, 0xad78ebc5 /* 20 */
	.word 0x177a8000, 0xd8d726b7 /* 21 */
	.word 0x6eac9000, 0x87867832 /* 22 */
	.word 0x0a57b400, 0xa968163f /* 23 */
	.word 0xcceda100, 0xd3c21bce /* 24 */
	.word 0x401484a0, 0x84595161 /* 25 */
	.word 0x9019a5c8, 0xa56fa5b9 /* 26 */
	.word 0xf4200f3a, 0xcecb8f27 /* 27 */
	.word 0xcfe20766, 0xd0cf4b50 /* 55 */
	.word 0x2aabd62c, 0xd2d80db0 /* 83 */
	.word 0xc1d1ea96, 0xd4e5e2cd /* 111 */
	.word 0x9292d603, 0xd6f8d750 /* 139 */
	.word 0x28069da4, 0xd910f7ff /* 167 */
	.word 0xe9d0696a, 0xdb2e51bf /* 195 */
	.word 0x6b947519, 0xdd50f199 /* 223 */
	.word 0xbd342cf7, 0xdf78e4b2 /* 251 */
	.word 0xbbd26451, 0xe1a63853 /* 279 */
	.word 0x63a198e5, 0xe3d8f9e5 /* 307 */

	.word 0x000000cc
	.word 0x00000000
	.word 0x00000014
	.word 0x45010004
	.word 0x01000000
	.word 0x030cc0cc
	.word 0x30011010
	.word 0x00040444
	.word 0x030300c0
	.word 0x3c3c0c03
	.word 0xcf30cf0c
	.word 0x00c00c00
	.word 0x00000000
	.word 0x00005001
	.word 0x44000010
	.word 0x00100400
	.word 0x00040100
	.word 0x00140001
	.word 0x10000000
	.word 0x00300300
	.word 0x00333c30
	.word 0xcf00303c
_ptenround:
	.word 0x00000000
	.word 0x00000000
	.word 0x00000000
	.word 0x00003000
	.word 0x03000000
	.word 0x00050010
	.word 0x11554000
	.word 0x04000401
	.word 0x44400100
	.word 0x04001104
	.word 0x04100000
	.word 0x00000000
	.word 0x000cf000
	.word 0x00000003
	.word 0x00cc30c0
	.word 0x0030c004
	.word 0x44010141
	.word 0x40000001
	.word 0x40000000


	.half -1210 /* -364 */
	.half -1117 /* -336 */
	.half -1024 /* -308 */
	.half -931 /* -280 */
	.half -838 /* -252 */
	.half -745 /* -224 */
	.half -652 /* -196 */
	.half -559 /* -168 */
	.half -466 /* -140 */
	.half -373 /* -112 */
	.half -280 /* -84 */
	.half -187 /* -56 */
	.half -94 /* -28 */
_ptenexp:
	.half 0 /* 0 */
	.half 3 /* 1 */
	.half 6 /* 2 */
	.half 9 /* 3 */
	.half 13 /* 4 */
	.half 16 /* 5 */
	.half 19 /* 6 */
	.half 23 /* 7 */
	.half 26 /* 8 */
	.half 29 /* 9 */
	.half 33 /* 10 */
	.half 36 /* 11 */
	.half 39 /* 12 */
	.half 43 /* 13 */
	.half 46 /* 14 */
	.half 49 /* 15 */
	.half 53 /* 16 */
	.half 56 /* 17 */
	.half 59 /* 18 */
	.half 63 /* 19 */
	.half 66 /* 20 */
	.half 69 /* 21 */
	.half 73 /* 22 */
	.half 76 /* 23 */
	.half 79 /* 24 */
	.half 83 /* 25 */
	.half 86 /* 26 */
	.half 89 /* 27 */
	.half 182 /* 55 */
	.half 275 /* 83 */
	.half 368 /* 111 */
	.half 461 /* 139 */
	.half 554 /* 167 */
	.half 647 /* 195 */
	.half 740 /* 223 */
	.half 833 /* 251 */
	.half 926 /* 279 */
	.half 1019 /* 307 */

/* x = _atod (buffer, ndigit, exp) */
/* Convert ndigits from buffer.  Digits in binary (0..9).
   1 <= ndigits <= 17.  -324 <= exp <= 308 */
/* Return double value. */
.text
NESTED(_atod,16,ra)
	addiu	sp,sp,-16
	sw	ra,0(sp)

	/* Convert digits to 64-bit integer * 2 (* 2 so that multiplication
	   by 10^N leaves binary point between registers). */
	addu	a1, a0
	li	t4, 0
	li	t5, 0
	b	11f
10:
	/* Multiply by 5 */
	srl	t7, t4, 30
	sll	t1, t4, 2
	not	t6, t1
	sltu	t6, t6, t4
	addu	t0, t7, t6
	addu	t4, t1

	sll	t1, t5, 2
	addu	t5, t1
	addu	t5, t0

11:	/* Add digit */
	lbu	t2, (a0)
	addu	a0, 1
	not	t3, t4
	sltu	t3, t3, t2
	addu	t4, t2
	addu	t5, t3

	/* Multiply by 2 */
	sll	t5, 1
	srl	t7, t4, 31
	or	t5, t7
	sll	t4, 1

	bne	a0, a1, 10b

	/* Dispatch zero */
	bne	t4, 0, 12f
	beq	t5, 0, 50f
12:

	/* Multiply 64-bit integer by 10^N, leaving binary point
	   between t2/t1. */
	move	t0, a2
	jal	_tenscale

	/* Normalize */

	/* Word normalize */
	bne	t3, 0, 20f
	move	t3, t2		# hi word zero, shift up by 32
	move	t2, t1
	move	t1, t0
	li	t0, 0
	subu	v1, 32		# adjust for 32-bit shift

	/* Find normalization shift */
20:	srl	a2, t3, 16
	li	t6, 16+8
	bne	a2, 0, 21f
	subu	t6, 16
21:	srl	a2, t3, t6
	addu	t6, 4
	bne	a2, 0, 22f
	subu	t6, 8
22:	srl	a2, t3, t6
	addu	t6, 2
	bne	a2, 0, 23f
	subu	t6, 4
23:	srl	a2, t3, t6
	addu	t6, 1
	bne	a2, 0, 24f
	subu	t6, 2
24:	srl	a2, t3, t6
	xor	a2, 1
	subu	t6, a2

	addu	v1, t6
	addu	v1, 1023+32	# 1023 = bias, 32 to compensate for binary
				# point after t2 instead of t3
	bgtz	v1, 29f
	/* will produce denorm */
	subu	t6, v1		# exponent pins at 0
	addu	t6, 1
	li	v1, 1		# 1 instead of 0 to compensate for hidden bit
				# subtraction below
	blt	t6, 52, 29f
	move	t0, t1
	move	t1, t2
	move	t2, t3
	li	t3, 0
	subu	t6, 32
29:

	/* Normalize */
	subu	a3, t6, 20	# normalize so that msb is bit 20 (hidden bit)
	beq	a3, 0, 26f
	negu	t7, a3
	bltz	a3, 25f
	/* quad-word right shift */
	srl	t0, a3		# necessary only for round to even done below
	sll	a2, t1, t7	# ditto
	or	t0, a2		# ditto
	srl	t1, a3
	sll	a2, t2, t7
	or	t1, a2
	srl	t2, a3
	sll	a2, t3, t7
	or	t2, a2
	srl	t3, a3
	b	26f
25:	/* quad-word left shift */
	sll	t3, t7
	srl	a2, t2, a3
	or	t3, a2
	sll	t2, t7
	srl	a2, t1, a3
	or	t2, a2
	sll	t1, t7
	srl	a2, t0, a3
	or	t1, a2
	sll	t0, t7		# necessary only for round to even done below

26:	/* Round */
	bgez	t1, 27f
	sll	t1, 1
	bne	t1, 0, 28f
	bne	t0, 0, 28f
	and	t0, t2, 1
	beq	t0, 0, 27f
28:
	addu	t2, 1
	bne	t2, 0, 27f
	addu	t3, 1
	sll	a3, t3, 31-21	# overflow into bit 21?
	bgez	a3, 27f
	addu	v1, 1
	srl	t3, 1		# t3/t2 is 200000/00000000, so no need
				# for dw shift
27:	bgeu	v1, 2047, 40f
	subu	v1, 1		# subtract out hidden bit
	sll	v1, 20
	addu	t3, v1
	mtc1	t2, $f0
	mtc1	t3, $f1
	b	55f
	
40:	lwc1	$f1, infinity
	mtc1	$0, $f0
	b	55f

50:	/* zero */
	mtc1	$0, $f0
	mtc1	$0, $f1
55:
	lw	ra,0(sp)
	addiu	sp,sp,16
	j	ra
.end _atod

/* Multiply 64-bit integers in t5/t4 and t7/t6 to produced 128-bit
   product in t3/t2/t1/t0.  Little-endian register order.  Bashes
   a3/a2.  t5/t4 and r7/t6 unchanged.  63 cycles. */
.ent _dwmultu
_dwmultu:
	.frame	sp, 0, ra
	multu	t4, t6		# x0 * y0
	## 10 cycle interlock
	mflo	t0		# lo(x0 * y0)
	mfhi	t1		# hi(x0 * y0)
	not	a3, t1
	## 1 nop
	multu	t5, t6		# x1 * y0
	## 10 cycle interlock
	mflo	a2		# lo(x1 * y0)
	mfhi	t2		# hi(x1 * y0)
	sltu	a3, a3, a2	# carry(hi(x0 * y0) + lo(x1 * y0))
	addu	t1, a2		# hi(x0 * y0) + lo(x1 * y0)
	multu	t4, t7		# x0 * y1
	add	t2, a3		# hi(x1 * y0) + carry
	not	a3, t1
	## 8 cycle interlock
	mflo	a2		# lo(x0 * y1)
	mfhi	t3		# hi(x0 * y1)
	sltu	a3, a3, a2	# carry((hi(x0 * y0) + lo(x1 * y0)) + lo(x0 * y1))
	addu	t1, a2		# hi(x0 * y0) + lo(x1 * y0) + lo(x0 * y1)
	multu	t5, t7		# x1 * y1
	add	t2, a3		# hi(x1 * y0) + carry + carry
	not	a2, t2
	sltu	a2, a2, t3	# carry(hi(x1 * y0) + hi(x0 * y1))
	addu	t2, t3		# hi(x1 * y0) + hi(x0 * y1))
	not	a3, t2
	## 5 cycle interlock
	mfhi	t3		# hi(x1 * y1)
	add	t3, a2		# hi(x1 * y1) + carry(hi(x1 * y0) + hi(x0 * y1))
	mflo	a2		# lo(x1 * y1)
	sltu	a3, a3, a2	# carry((hi(x1 * y0) + hi(x0 * y1)) + lo(x1 * y1))
	add	t3, a3
	addu	t2, a2		# hi(x1 * y0) + hi(x0 * y1) + lo(x1 * y1)
	j	ra
.end _dwmultu

/* t3/t2/t1/t0 = t5/t4 * 10^t0, return binary exponent of 10^t0 in v1. */
/* Bashes a3/a2. */

.ent _tenscale
_tenscale:
	bltz	t0, 10f
	subu	t0, 27
	bgtz	t0, 10f
	sll	t0, 1
	lh	v1, _ptenexp+27*2(t0)
	sll	t0, 2
	ld	t6, _pten+27*8(t0)
	j	_dwmultu

10:	subu	sp, 16
	sd	t4, 0(sp)
	sw	ra, 8(sp)
	sra	t3, t0, 4
	sll	t3, 2
	lw	t3, _ptenround(t3)
	sll	t2, t0, 1
	sll	t3, t2
	sra	t3, 30
	sw	t3, 12(sp)
	bltz	t0, 20f
	li	t1, (27-1)*2
11:	addu	t1, 1*2
	subu	t0, 28
	bgez	t0, 11b
12:	lh	v1, _ptenexp(t1)
	sll	t1, 2
#ifdef MIPSEL
	ld	t6, _pten(t1)
#else
	lw	t6, _pten+0(t1)
	lw	t7, _pten+4(t1)
#endif
	sll	t0, 1
	lh	t2, _ptenexp+28*2(t0)
	addu	v1, t2
	addu	v1, 1
	sll	t0, 2
#ifdef MIPSEL
	ld	t4, _pten+28*8(t0)
#else
	lw	t4, _pten+0+28*8(t0)
	lw	t5, _pten+4+28*8(t0)
#endif
	jal	_dwmultu
	bltz	t3, 13f
	sll	t3, 1
	srl	t0, t2, 31
	or	t3, t0
	sll	t2, 1
	srl	t0, t1, 31
	or	t2, t0
	sll	t1, 1
	subu	v1, 1
13:	lw	t0, 12(sp)
	srl	t1, 31
	addu	t6, t2, t1
	addu	t6, t0
	move	t7, t3
	ld	t4, 0(sp)
	lw	ra, 8(sp)
	addu	sp, 16
	j	_dwmultu

20:	li	t1, 0
21:	subu	t1, 1*2
	addu	t0, 28
	bltz	t0, 21b
	subu	t0, 28
	b	12b
.end _tenscale


/* _dtoa (buffer, ndigit, x, fflag) */
/* Store sign, ndigits of x, and null in buffer.  Digits are in ascii.
   1 <= ndigits <= 17.  Return exponent.  If fflag set then generate
   Fortran F-format; i.e. ndigits after decimal point. */
NESTED(_dtoa,16,ra)
	addiu	sp,sp,-16
	sw	ra,0(sp)

	/* Sign */
	li	t1, 32
#if MIPSEL
	bgez	a3, 1f
	li	t1, 45
1:	sll	t5, a3, 1
	srl	t0, a2, 31
	or	t5, t0
	sll	t4, a2, 1
#else
	bgez	a2, 1f
	li	t1, 45
1:	sll	t5, a2, 1
	srl	t0, a3, 31
	or	t5, t0
	sll	t4, a3, 1
#endif
	sb	t1, (a0)
	addu	a0, 1

	/* log10 approximation */
	li	t2, 0x4D104D42 >> 21	# log10(2), shifted so . between hi/lo
	subu	t1, t5, 1023 << 21
	bgez	t1, 2f
	addu	t2, 1
2:	mult	t1, t2
	srl	t8, t5, 21
	beq	t8, 0, 10f
	beq	t8, 2047, 20f

	/* Convert to fixed point number in range [1,100) by multiplication
	   by appropriate power of 10. */
	sll	t5, 11
	srl	t5, 11
	or	t5, 1 << 21
	mfhi	v0			# approximate exponent

15:	neg	t0, v0
	jal	_tenscale
	subu	t8, 1023+20
	addu	t8, v1
	neg	t8

	/* Extract first digit */
	srl	a2, t3, t8
	sll	a3, a2, t8
	subu	t3, a3
	sltu	t4, a2, 10		# first digit > 9?

	/* Handle F-format */
	lw	t5, 32(sp)		# fflag
	beq	t5, 0, 18f
	# add corrected exponent+1 to digits after point to get
	# the number of digits to produce
	subu	t5, t4, v0		# ndigit - ((ld < 10) - v0) + 2
	subu	a1, t5			# = ndigit + v0 + (ld >= 10) + 1
	addu	a1, 2
			
	bgtz	a1, 17f
	## ndigit <= 0
	xor	t5, t4, 1
	addu	v0, t5
	bltz	a1, 6f			# return null string if ndigit < 0
	## ndigit = 0, return "" if round-down, or "1" if round-up
	bne	t4, 0, 19f		# first digit < 10
	bltu	a2, 50, 6f
	bgtu	a2, 50, 7f
	b	55f
17:	ble	a1, 17, 18f		# no more than 17 digits
	li	a1, 17
18:
	addu	a1, a0			# a1: stop point in buffer

	bne	t4, 0, 3f
	/* log10 approximation was low by 1, so we got first "digit" > 9 */
	addu	v0, 1
#if 1	/* is this necessary?? */
	divu	a3, a2, 10
	remu	a2, 10
	addu	a3, 48
#else	/* or will this suffice?? */
	li	a3, 48+1
	subu	a2, 10
#endif
	sb	a3, (a0)		# store extra digit
	addu	a0, 1
	bne	a0, a1, 3f
19:	bltu	a2, 5, 6f
	bgtu	a2, 5, 7f
	b	55f
3:	addu	a2, 48			# store digit
	sb	a2, (a0)
	addu	a0, 1
	beq	a0, a1, 5f		# that may be it

	/* Now produce digits by multiplying fraction by 10 and taking
	   integer part.  Actually multiply 5 (it's easier) and take
	   integer part from decreasing bit positions. */
4:	srl	t7, t0, 30
	sll	t5, t0, 2
	not	t6, t5
	sltu	t6, t6, t0
	addu	t4, t7, t6
	addu	t0, t5

	srl	t7, t1, 30
	sll	t5, t1, 2
	not	t6, t5
	sltu	t6, t6, t1
	addu	t1, t5
	addu	t7, t6
	not	t6, t4
	sltu	t6, t6, t1
	addu	t1, t4
	addu	t4, t7, t6

	srl	t7, t2, 30
	sll	t5, t2, 2
	not	t6, t5
	sltu	t6, t6, t2
	addu	t2, t5
	addu	t7, t6
	not	t6, t4
	sltu	t6, t6, t2
	addu	t2, t4
	addu	t4, t7, t6

	sll	t5, t3, 2
	addu	t3, t5
	addu	t3, t4

	subu	t8, 1
	srl	a2, t3, t8
	sll	a3, a2, t8
	subu	t3, a3
	addu	a2, 48
	sb	a2, (a0)
	addu	a0, 1
	bne	a0, a1, 4b

5:	/* digit production complete.  now round */
	subu	t8, 1
	srl	a3, t3, t8
	beq	a3, 0, 6f
55:
	bne	t2, 0, 7f
	subu	a3, t3, 1
	bne	t1, 0, 7f
	and	a3, t3
	bne	t0, 0, 7f
	and	a2, 1
	bne	a3, 0, 7f
	bne	a2, 0, 7f

6:	/* round down, i.e. done */
	sb	$0, (a0)
	b	99f

7:	/* round up (in ascii!) */
	subu	t1, a0, 1
75:
.set noreorder
	lbu	t0, (t1)
	beq	t0, 48+9, 8f
	bltu	t0, 48, 9f		/* ' ' and '-' both < '0' */
	 addu	t0, 1
.set reorder
	sb	t0, (t1)
	sb	$0, (a0)
	b	99f
8:	li	t0, 48
	sb	t0, (t1)
	subu	t1, 1
	b	75b
9:	/* tried to round into sign position */
	addu	v0, 1
	lw	t5, 32(sp)		# fflag
	li	t0, 48+1
	beq	t5, 0, 91f
	li	t4, 48
	sb	t4, 0(a0)
	addu	a0, 1
91:	sb	t0, 1(t1)
	sb	$0, (a0)
	b	99f
	
10:	/* output 0 or denorm */
	bne	t5, 0, 12f
	bne	t4, 0, 12f
	/* zero */
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
	 * lw	t0, 32(sp)
	 * addu	a1, t0
	 */
	li	t2, 48
	blez	a1, 40f
	addu	a1, a0
11:	sb	t2, (a0)
	addu	a0, 1
	bne	a0, a1, 11b
40:	sb	$0, (a0)
	li	v0, 0
	b	99f

12:	/* denorm */
	/* normalize with slow but small loop (denorm speed is unimportant) */
	li	t8, -1022
	li	t2, 1 << 21
13:	subu	t8, 1
	sll	t5, 1
	srl	t0, t4, 31
	or	t5, t0
	sll	t4, 1
	and	t0, t5, t2
	beq	t0, 0, 13b
14:	/* log10 approximation */	
	sll	t1, t8, 20
	addu	t1, t5
	subu	t1, t2
	li	t2, (0x4D104D42 >> 20) + 1
	mult	t1, t2
	addu	t8, 1023
	mfhi	v0
	b	15b

20:	/* output +-Infinity or Nan */
	addu	a1, a0
	la	t0, nan_s
	bne	t4, 0, 22f
	sll	t1, t5, 11
	bne	t1, 0, 22f
	la	t0, infinity_s
22:	lbu	t1, (t0)
	addu	a0, 1
	sb	t1, -1(a0)
	beq	t1, 0, 23f
	addu	t0, 1
	bne	a0, a1, 22b
23:	li	v0, 0x80000000
99:
	lw	ra,0(sp)
	addiu	sp,sp,16
	j	ra
.end _dtoa

