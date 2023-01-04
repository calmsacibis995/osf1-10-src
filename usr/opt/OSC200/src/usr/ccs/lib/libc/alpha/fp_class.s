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
/* @(#)fp_class.s	9.1 (ULTRIX) 8/7/90 */
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
 * 0001	Ken Lesniak, 03-Jul-1990
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
/* $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/fp_class.s,v 1.1.4.2 1993/06/11 19:34:09 Neil_OBrien Exp $ */

#include <machine/regdef.h>
#include <machine/asm.h>
#include <fp_class.h>
#include <machine/softfp.h>

/*
 * fp_class_d(d)	
 * double d;
 */
#define D_FRMSIZ 16
NESTED(fp_class_d, D_FRMSIZ, ra)
	ldgp	gp, 0(pv)
	lda	sp, -D_FRMSIZ(sp)
	.prologue	1

	# Get the double into an integer register
	stt	$f16, 0(sp)
	ldq	a2, 0(sp)

	# Break down the double into sign (a0), exponent (a1), fraction (a2),
	# and fraction shifted into msb (a3)
	srl	a2, DEXP_SHIFT, a1
	and	a1, DEXP_MASK, a1
	cmplt	a2, 0, a0
	sll	a2, 64-DFRAC_BITS, a3

	# Check for infinities and Nan's
	cmpeq	a1, DEXP_INF, v0
	beq	v0, 2f
	bne	a3, 1f
	ldiq	v0, FP_POS_INF
	cmovne	a0, FP_NEG_INF, v0
	lda	sp, D_FRMSIZ(sp)
	ret

	# Check to see if this is a signaling NaN or a quiet NaN
1:	ldiq	v0, FP_QNAN
	cmovge	a3, FP_SNAN, v0
	lda	sp, D_FRMSIZ(sp)
	ret

	# Check for zeroes and denorms
2:	bne	a1, 4f
	bne	a3, 3f
	ldiq	v0, FP_POS_ZERO
	cmovne	a0, FP_NEG_ZERO, v0
	lda	sp, D_FRMSIZ(sp)
	ret
3:	ldiq	v0, FP_POS_DENORM
	cmovne	a0, FP_NEG_DENORM, v0
	lda	sp, D_FRMSIZ(sp)
	ret

	# It is just a normal number
4:	ldiq	v0, FP_POS_NORM
	cmovne	a0, FP_NEG_NORM, v0
	lda	sp, D_FRMSIZ(sp)
	RET
END(fp_class_d)

/*
 * fp_class_f(f)	
 * float d;
 */
#define F_FRMSIZ 16
NESTED(fp_class_f, F_FRMSIZ, ra)
	ldgp	gp, 0(pv)
	lda	sp, -F_FRMSIZ(sp)
	.prologue	1

	# Get the float into an integer register
	sts	$f16, 0(sp)
	ldl	a2, 0(sp)

	# Break down the float into sign (a0), exponent (a1), fraction (a2),
	# and fraction shifted into msb (a3)
	srl	a2, SEXP_SHIFT, a1
	and	a1, SEXP_MASK, a1
	cmplt	a2, 0, a0
	sll	a2, 64-SFRAC_BITS, a3

	# Check for infinities and Nan's
	cmpeq	a1, SEXP_INF, v0
	beq	v0, 2f
	bne	a3, 1f
	ldiq	v0, FP_POS_INF
	cmovne	a0, FP_NEG_INF, v0
	lda	sp, F_FRMSIZ(sp)
	ret

	# Check to see if this is a signaling NaN or a quiet NaN
1:	ldiq	v0, FP_QNAN
	cmovge	a3, FP_SNAN, v0
	lda	sp, F_FRMSIZ(sp)
	ret

	# Check for zeroes and denorms
2:	bne	a1, 4f
	bne	a3, 3f
	ldiq	v0, FP_POS_ZERO
	cmovne	a0, FP_NEG_ZERO, v0
	lda	sp, F_FRMSIZ(sp)
	ret
3:	ldiq	v0, FP_POS_DENORM
	cmovne	a0, FP_NEG_DENORM, v0
	lda	sp, F_FRMSIZ(sp)
	ret

	# It is just a normal number
4:	ldiq	v0, FP_POS_NORM
	cmovne	a0, FP_NEG_NORM, v0
	lda	sp, F_FRMSIZ(sp)
	RET
END(fp_class_f)
