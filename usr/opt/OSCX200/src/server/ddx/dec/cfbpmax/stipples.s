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
 * $XConsortium: stipples.s,v 1.3 90/11/30 15:24:42 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * MIPS assembly code for optimized text rendering.
 *
 * Other stippling could be done in assembly, but the payoff is
 * not nearly as large.  Mostly because large areas are heavily
 * optimized already.
 */

#ifdef MIPSEL
# define BitsR		sll
# define BitsL		srl
# define FourBits(dest,bits)	and	dest, bits, 0xf
#else
# define BitsR	srl
# define BitsL	sll
# define FourBits(dest,bits)	srl	dest, bits, 28
#endif

/* reordering instructions would be fatal here */
	.set	noreorder

#define StippleCases(addr,value,done,do_l1,do_l2,o)	\
	do_l2			/* 0 */	;\
	nop				;\
	done				;\
	nop				;\
					\
	do_l2			/* 1 */	;\
	sb	value, o+0(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 2 */	;\
	sb	value, o+1(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 3 */	;\
	sh	value, o+0(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 4 */	;\
	sb	value, o+2(addr)	;\
	done				;\
	nop				;\
					\
	sb	value, o+0(addr)/* 5 */	;\
	do_l1				;\
	sb	value, o+2(addr)	;\
	nop				;\
					\
	sb	value, o+1(addr)/* 6 */	;\
	do_l1				;\
	sb	value, o+2(addr)	;\
	nop				;\
					\
	do_l2			/* 7 */	;\
	swl	value, o+2(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* 8 */	;\
	sb	value, o+3(addr)	;\
	done				;\
	nop				;\
					\
	sb	value, o+0(addr)/* 9 */	;\
	do_l1				;\
	sb	value, o+3(addr)	;\
	nop				;\
					\
	sb	value, o+1(addr)/* a */	;\
	do_l1				;\
	sb	value, o+3(addr)	;\
	nop				;\
					\
	sh	value, o+0(addr)/* b */	;\
	do_l1				;\
	sb	value, o+3(addr)	;\
	nop				;\
					\
	do_l2			/* c */	;\
	sh	value, o+2(addr)	;\
	done				;\
	nop				;\
					\
	sb	value, o+0(addr)/* d */	;\
	do_l1				;\
	sh	value, o+2(addr)	;\
	nop				;\
					\
	do_l2			/* e */	;\
	swr	value, o+1(addr)	;\
	done				;\
	nop				;\
					\
	do_l2			/* f */	;\
	sw	value, o+0(addr)	;\
	done				;\
	nop

/*
 * This macro uses registers:
 *	2	temp
 *	8	switch table base, must contain the address of $label
 */
/* variables */

#define SBase		$8
#define STemp		$2
#define CatComma(a,b)	a, b

#define Stipple(addr,bits,value,label,done,l1,l2)	\
	FourBits(STemp, bits)		;\
	sll	STemp, STemp, 4		;\
	addu	STemp, STemp, SBase	;\
	j	STemp			;\
	BitsL	bits,4			;\
$l1:					;\
	bnez	bits,$l2		;\
	nop				;\
	done				;\
	nop				;\
$l2:					;\
	addu	addr, addr, 4		;\
	FourBits(STemp, bits)		;\
	sll	STemp, STemp, 4		;\
	addu	STemp, STemp, SBase	;\
	j	STemp			;\
	BitsL	bits, 4			;\
$label:					;\
	StippleCases(addr,value,done,b $l1,CatComma(bnez bits, $l2),0)

#ifdef INCLUDE_UNUSED_FUNCTIONS
/*
 * This function isn't used, but exists to illustrate the
 * usage of the basic code
 */
	.text	
	.align	2
/*
 * stippleone(addr, bits, value)
 *             4     5     6
 */
/* arguments */
#define addr	$4
#define bits	$5
#define value	$6

	.globl	stippleone
	.ent	stippleone 2
stippleone:
	.frame	$sp, 0, $31
	la	SBase, $100
	Stipple(addr,bits,value,100,j $31,101,102)
	j	$31
	nop
	.end	stippleone
#undef addr
#undef bits
#undef value

#endif /* INCLUDE_UNUSED_FUNCTIONS */

/*
 * stipplestack(addr, stipple, value, stride, Count, Shift)
 *               4       5       6      7     16(sp) 20(sp)
 *
 *  Apply successive 32-bit stipples starting at addr, addr+stride, ...
 *
 *  Used for text rendering, but only when no data could be lost
 *  when the stipple is shifted left by Shift bits
 */
/* arguments */
#define addr	$4
#define stipple	$5
#define value	$6
#define stride	$7
#define Count	16($sp)
#define Shift	20($sp)

/* local variables */
#define count	$14
#define shift	$13
#define atemp	$12
#define bits	$11

	.globl	stipplestack
	.ent	stipplestack 2
stipplestack:
	.frame	$sp, 0, $31
	lw	count, Count
	lw	shift, Shift
	la	SBase,$201
$200:
	lw	bits, 0(stipple)
	move	atemp, addr
	BitsR	bits, bits, shift
	Stipple(atemp,bits,value,201,b $204,202,203)
$204:
	addu	count, count, -1
	addu	addr, addr, stride
	bnez	count, $200
	addu	stipple, stipple, 4
	j	$31

	.end	stipplestack

/*
 * Used when the stipple is > 28 pixels wide to avoid troubles with the
 * shift.  Cannot be used when the shift amount is zero.
 */

/* additional local variables */
#define tbits	$10
#define TBase	$9
#define mshift	$15

	.globl	stipplestackwide
	.ent	stipplestackwide 2
stipplestackwide:
	.frame	$sp, 0, $31
	lw	count, Count
	la	SBase,$251
	la	TBase, $260
	lw	shift, Shift
	li	mshift, 32
	subu	mshift, mshift, shift
$250:
	lw	bits, 0(stipple)
	move	atemp, addr
	BitsR	tbits, bits, shift
	Stipple(atemp,tbits,value,251,b $254,252,253)
$254:
	BitsL	tbits, bits, mshift
	sll	tbits, tbits, 4
	addu	STemp, TBase, tbits
	j	STemp
	addu	count, count, -1
$260:
	StippleCases(addr,value,nop,b $264,b $264,28)
$264:
	addu	addr, addr, stride
	bnez	count, $250
	addu	stipple, stipple, 4
	j	$31

	.end	stipplestackwide

#undef addr
#undef stipple
#undef value
#undef stride
#undef Count
#undef Shift
#undef count
#undef shift
#undef atemp
#undef bits

#ifdef INCLUDE_UNUSED_FUNCTIONS
/*
 * stipplespan32(addr,bits,value, leftmask,rightmask,nlw)
 *                $4   $5    $6       $7     16($sp) 20($sp) 		
 *
 * Fill a span with bits from a 32-bit stipple.
 *
 * This could be used by span filling code, but the performance
 * gain is not really all that significant
 */
/* arguments */
#define addr		$4
#define bits		$5
#define value		$6
#define leftmask	$7
#define Rightmask	16($sp)
#define Nlw		20($sp)
/* local variables */
#define rightmask	$14
#define nlw		$13
#define btemp		$12
#define atemp		$11

	.globl	stipplespan32
	.ent	stipplespan32 2
stipplespan32:
	.frame	$sp, 0, $31

	lw	nlw,20($sp)
	la	SBase, $402

	/*
 	 * Compute left edge stipple bits and
 	 * jump into the middle of the loop
	 */

	beqz	leftmask,$405
	and	btemp,bits,leftmask
	b	$401
	nop
	/* while (nlw--) */
$400:
	move	btemp,bits
$401:
	move	atemp, addr
	addu	addr, addr, 32
	Stipple(atemp,btemp,value,402,b $405,403,404)
$405:
	bnez	nlw,$400
	addu	nlw,nlw,-1

	/* And do the right edge as well */
	lw	rightmask,Rightmask
	la	SBase, $406
	and	btemp, bits, rightmask

	Stipple(addr,btemp,value,406,j $31,407,408)

	j	$31
	nop
	.end	stipplespan32

#undef addr
#undef bits
#undef value
#undef leftmask
#undef Rightmask
#undef Nlw
#undef rightmask
#undef nlw
#undef btemp
#undef atemp

#endif /* INCLUDE_UNUSED_FUNCTIONS */
