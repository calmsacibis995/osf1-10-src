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
 *	@(#)$RCSfile: frexp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:48:11 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
     double frexp(value, eptr)
     double value;
     int *eptr;

     The frexp subroutine returns the mantissa of a double value
     as a double quantity, x, of magnitude less than one and
     greater than or equal to 0.5 (0.5 <= |x| < 1) and stores an
     integer n such that value = x*2**n indirectly through eptr.

     One exception: if the given value is 0, then x and *eptr are made
     zero. 
*/

#include <machine/asm.h>

m1:	.long	-1

	.align	ALIGN

ENTRY(frexp)
	/ First, check for zero.
	fldl	4(%esp)
	fldz
	fcomp	%st(1)		/ and pop off the zero
	fnstsw	%ax
	sahf
	jne	LBf(notzero,0)

	movl	12(%esp), %eax
	movl	$0,(%eax)	/ *exp = 0
	ret			/ return 0.0 (ie, the argument we were given)

LB(notzero,0):
	fxtract			/ significand at top of stack, then exp.
				/ abs(significand) is between 1 and 2

	fxch	%st(1)		/ put exponent at top of stack, then sig.
	fld1
	faddp	%st, %st(1)	/ exp = exp+1 (multiply by 2)
 	movl	12(%esp), %eax
	fistpl	(%eax)		/ *eptr = exp

	fildl	m1
	fxch	%st(1)
	fscale			/ divide by 2
	fstp	%st(1)
	ret
