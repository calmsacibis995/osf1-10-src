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
 *	@(#)$RCSfile: fp_class.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:13:00 $
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

/*
 * fp_class_d(d)	
 * double d;
 */
LEAF(fp_class_d)
	# Get the double from f12
	mfc1	a2,$f13
	mfc1	a3,$f12

	# Break down the double into sign, exponent and fraction
	srl	a1,a2,DEXP_SHIFT
	move	a0,a1
	and	a1,DEXP_MASK
	and	a0,SIGNBIT>>DEXP_SHIFT
	and	a2,DFRAC_MASK

	# Check for infinities and Nans
	bne	a1,DEXP_INF,4f
	bne	a2,zero,2f
	bne	a3,zero,2f
	bne	a0,zero,1f
	li	v0,FP_POS_INF
	j	ra
1:
	li	v0,FP_NEG_INF
	j	ra
2:	# Check to see if this is a signaling NaN
	and	a0,a2,DSNANBIT_MASK
	beq	a0,zero,3f
	li	v0,FP_SNAN
	j	ra
3:	li	v0,FP_QNAN
	j	ra
4:
	# Check for zeroes and denorms
	bne	a1,zero,8f
	bne	a2,zero,6f
	bne	a3,zero,6f
	bne	a0,zero,5f
	li	v0,FP_POS_ZERO
	j	ra
5:	li	v0,FP_NEG_ZERO
	j	ra
6:	bne	a0,zero,7f
	li	v0,FP_POS_DENORM
	j	ra
7:	li	v0,FP_NEG_DENORM
	j	ra
8:
	# It is just a normal number
	bne	a0,zero,9f
	li	v0,FP_POS_NORM
	j	ra
9:	li	v0,FP_NEG_NORM
	j	ra
END(fp_class_d)

/*
 * fp_class_f(f)	
 * float d;
 */
LEAF(fp_class_f)
	# Get the float from f12
	mfc1	a2,$f12

	# Break down the float into sign, exponent and fraction
	srl	a1,a2,SEXP_SHIFT
	move	a0,a1
	and	a1,SEXP_MASK
	and	a0,SIGNBIT>>SEXP_SHIFT
	and	a2,SFRAC_MASK

	# Check for infinities and Nans
	bne	a1,SEXP_INF,4f
	bne	a2,zero,2f
	bne	a0,zero,1f
	li	v0,FP_POS_INF
	j	ra
1:
	li	v0,FP_NEG_INF
	j	ra
2:	# Check to see if this is a signaling NaN
	and	a0,a2,SSNANBIT_MASK
	beq	a0,zero,3f
	li	v0,FP_SNAN
	j	ra
3:	li	v0,FP_QNAN
	j	ra
4:
	# Check for zeroes and denorms
	bne	a1,zero,8f
	bne	a2,zero,6f
	bne	a0,zero,5f
	li	v0,FP_POS_ZERO
	j	ra
5:	li	v0,FP_NEG_ZERO
	j	ra
6:	bne	a0,zero,7f
	li	v0,FP_POS_DENORM
	j	ra
7:	li	v0,FP_NEG_DENORM
	j	ra
8:
	# It is just a normal number
	bne	a0,zero,9f
	li	v0,FP_POS_NORM
	j	ra
9:	li	v0,FP_NEG_NORM
	j	ra
END(fp_class_f)
