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
/* @(#)tenscale.s	9.2 (ULTRIX) 10/10/90 */
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

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/tenscale.s,v 1.1.6.3 1993/06/08 01:25:52 David_Lindner Exp $ */

#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif
#include <machine/regdef.h>
#include <machine/asm.h>

#if !as1_fixed
_ptenexp = 624			# byte offset to here from _ptentables
_ptenround = 496		# byte offset to here from _ptentables
_pten = 104			# byte offset to here from _ptentables
#endif

/* t1:t0 = t2 * 10^t0, return binary exponent of 10^t0 in a5. */
/* t2 unchanged */
/* t3 bashed */
/* return address in a4 */

#define FRMSIZ 16

NESTED(_tenscale, FRMSIZ, a4)
	.save_ra	a4
	ldgp	gp, 0(pv)
	lda	sp, -FRMSIZ(sp)
	.prologue	1

	.set	noat
	lda	$at, _ptentables	# get pointer to power of 10 tables

	blt	t0, 10f			# br if negative exp
	subq	t0, 27, t0
	bgt	t0, 10f			# br if exp > 27

	# exponent was in range 0..27

	s8addq	t0, $at, t3		# make exp into table offset
	ldq	t3, _pten+27*8(t3)	# get power of ten
	s4addq	t0, $at, a5		# make exp into table offset
	ldl	a5, _ptenexp+27*4(a5)	# get binary exponent
	mulq	t2, t3, t0		# do the scaling
	umulh	t2, t3, t1
	lda	sp, FRMSIZ(sp)
	ret	zero, (a4), 1

/* Exponent is < 0 or > 27 */

10:	sra	t0, 4, t3
	s4addq	t3, $at, t3
	ldl	t3, _ptenround(t3)
	sll	t0, 1, t1
	sll	t3, t1, t3
	sra	t3, 63, t3
	stl	t3, 0(sp)

	blt	t0, 20f			# br if negative exp
	ldiq	t1, 27-1
11:	addq	t1, 1, t1
	subq	t0, 28, t0
	bge	t0, 11b

12:	s4addq	t1, $at, a5
	ldl	a5, _ptenexp(a5)
	s8addq	t1, $at, t3
	ldq	t3, _pten(t3)

	s4addq	t0, $at, t1
	ldl	t1, _ptenexp+28*4(t1)
	addq	a5, t1, a5
	addq	a5, 1, a5
	s8addq	t0, $at, t1
	ldq	t1, _pten+28*8(t1)
	mulq	t1, t3, t0
	umulh	t1, t3, t1

	blt	t1, 13f
	sll	t1, 1, t1
	srl	t0, 63, t3
	bis	t3, t1, t1
	sll	t0, 1, t0
	subq	a5, 1, a5

13:	ldl	t3, 0(sp)
	srl	t0, 63, t0
#if 1	/* I'm not sure why the following needs to be this way. ??? Ken */
	addl	t0, t3, t3
	addl	t1, t3, t3
	zap	t3, 0xf0, t3
	zap	t1, 0x0f, t1
	bis	t1, t3, t3
#else
	addq	t0, t3, t3
	addq	t1, t3, t3
#endif
	mulq	t2, t3, t0
	umulh	t2, t3, t1
	lda	sp, FRMSIZ(sp)
	ret	zero, (a4), 1

20:	ldiq	t1, 0
21:	subq	t1, 1, t1
	addq	t0, 28, t0
	blt	t0, 21b
	subq	t0, 28, t0
	br	zero, 12b
	.set	at
END(_tenscale)

.rdata
.align 3
_ptentables:			# base address of tables
	.quad 0xe1afa13afbd14d6e /* -364 */
	.quad 0xe3e27a444d8d98b8 /* -336 */
	.quad 0xe61acf033d1a45df /* -308 */
	.quad 0xe858ad248f5c22ca /* -280 */
	.quad 0xea9c227723ee8bcb /* -252 */
	.quad 0xece53cec4a314ebe /* -224 */
	.quad 0xef340a98172aace5 /* -196 */
	.quad 0xf18899b1bc3f8ca2 /* -168 */
	.quad 0xf3e2f893dec3f126 /* -140 */
	.quad 0xf64335bcf065d37d /* -112 */
	.quad 0xf8a95fcf88747d94 /* -84 */
	.quad 0xfb158592be068d2f /* -56 */
	.quad 0xfd87b5f28300ca0e /* -28 */
#if as1_fixed
_pten = 104			# byte offset to here from _ptentables
#endif
	.quad 0x8000000000000000 /* 0 */
	.quad 0xa000000000000000 /* 1 */
	.quad 0xc800000000000000 /* 2 */
	.quad 0xfa00000000000000 /* 3 */
	.quad 0x9c40000000000000 /* 4 */
	.quad 0xc350000000000000 /* 5 */
	.quad 0xf424000000000000 /* 6 */
	.quad 0x9896800000000000 /* 7 */
	.quad 0xbebc200000000000 /* 8 */
	.quad 0xee6b280000000000 /* 9 */
	.quad 0x9502f90000000000 /* 10 */
	.quad 0xba43b74000000000 /* 11 */
	.quad 0xe8d4a51000000000 /* 12 */
	.quad 0x9184e72a00000000 /* 13 */
	.quad 0xb5e620f480000000 /* 14 */
	.quad 0xe35fa931a0000000 /* 15 */
	.quad 0x8e1bc9bf04000000 /* 16 */
	.quad 0xb1a2bc2ec5000000 /* 17 */
	.quad 0xde0b6b3a76400000 /* 18 */
	.quad 0x8ac7230489e80000 /* 19 */
	.quad 0xad78ebc5ac620000 /* 20 */
	.quad 0xd8d726b7177a8000 /* 21 */
	.quad 0x878678326eac9000 /* 22 */
	.quad 0xa968163f0a57b400 /* 23 */
	.quad 0xd3c21bcecceda100 /* 24 */
	.quad 0x84595161401484a0 /* 25 */
	.quad 0xa56fa5b99019a5c8 /* 26 */
	.quad 0xcecb8f27f4200f3a /* 27 */
	.quad 0xd0cf4b50cfe20766 /* 55 */
	.quad 0xd2d80db02aabd62c /* 83 */
	.quad 0xd4e5e2cdc1d1ea96 /* 111 */
	.quad 0xd6f8d7509292d603 /* 139 */
	.quad 0xd910f7ff28069da4 /* 167 */
	.quad 0xdb2e51bfe9d0696a /* 195 */
	.quad 0xdd50f1996b947519 /* 223 */
	.quad 0xdf78e4b2bd342cf7 /* 251 */
	.quad 0xe1a63853bbd26451 /* 279 */
	.quad 0xe3d8f9e563a198e5 /* 307 */

	.long 0x000000cc
	.long 0x00000000
	.long 0x00000014
	.long 0x45010004
	.long 0x01000000
	.long 0x030cc0cc
	.long 0x30011010
	.long 0x00040444
	.long 0x030300c0
	.long 0x3c3c0c03
	.long 0xcf30cf0c
	.long 0x00c00c00
	.long 0x00000000
	.long 0x00005001
	.long 0x44000010
	.long 0x00100400
	.long 0x00040100
	.long 0x00140001
	.long 0x10000000
	.long 0x00300300
	.long 0x00333c30
	.long 0xcf00303c
#if as1_fixed
_ptenround = 496		# byte offset to here from _ptentables
#endif
	.long 0x00000000
	.long 0x00000000
	.long 0x00000000
	.long 0x00003000
	.long 0x03000000
	.long 0x00050010
	.long 0x11554000
	.long 0x04000401
	.long 0x44400100
	.long 0x04001104
	.long 0x04100000
	.long 0x00000000
	.long 0x000cf000
	.long 0x00000003
	.long 0x00cc30c0
	.long 0x0030c004
	.long 0x44010141
	.long 0x40000001
	.long 0x40000000


/* Table to convert powers of ten (decimal exponent) to powers of two (binary */
/* exponent). The index into the table is a power of ten. The value at */
/* index is the highest power of two less than or equal to the power of ten */

	.long -1210 /* -364 */
	.long -1117 /* -336 */
	.long -1024 /* -308 */
	.long -931 /* -280 */
	.long -838 /* -252 */
	.long -745 /* -224 */
	.long -652 /* -196 */
	.long -559 /* -168 */
	.long -466 /* -140 */
	.long -373 /* -112 */
	.long -280 /* -84 */
	.long -187 /* -56 */
	.long -94 /* -28 */
#if as1_fixed
_ptenexp = 624			# byte offset to here from _ptentables
#endif
	.long 0 /* 0 */
	.long 3 /* 1 */
	.long 6 /* 2 */
	.long 9 /* 3 */
	.long 13 /* 4 */
	.long 16 /* 5 */
	.long 19 /* 6 */
	.long 23 /* 7 */
	.long 26 /* 8 */
	.long 29 /* 9 */
	.long 33 /* 10 */
	.long 36 /* 11 */
	.long 39 /* 12 */
	.long 43 /* 13 */
	.long 46 /* 14 */
	.long 49 /* 15 */
	.long 53 /* 16 */
	.long 56 /* 17 */
	.long 59 /* 18 */
	.long 63 /* 19 */
	.long 66 /* 20 */
	.long 69 /* 21 */
	.long 73 /* 22 */
	.long 76 /* 23 */
	.long 79 /* 24 */
	.long 83 /* 25 */
	.long 86 /* 26 */
	.long 89 /* 27 */
	.long 182 /* 55 */
	.long 275 /* 83 */
	.long 368 /* 111 */
	.long 461 /* 139 */
	.long 554 /* 167 */
	.long 647 /* 195 */
	.long 740 /* 223 */
	.long 833 /* 251 */
	.long 926 /* 279 */
	.long 1019 /* 307 */
