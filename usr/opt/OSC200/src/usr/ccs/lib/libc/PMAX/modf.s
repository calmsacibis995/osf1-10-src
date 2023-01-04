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
 *	@(#)$RCSfile: modf.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:16:08 $
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
/* $ Header: modf.s,v 1.1 87/02/16 11:17:33 dce Exp $ */

#include <mips/regdef.h>

/*
 * modf(value, iptr) returns the signed fractional part of value
 * and stores the integer part indirectly through iptr.
 *
double modf(value, iptr)
    double value;
    double *iptr;
 */

/*.sdata*/.data
.align 3
one:	.double 1.0
#if MOXIE
maxint:	.double 36028797018963968.0	# 2**55
#else
maxint:	.double 4503599627370496.0	# 2**52
#endif

.text

.globl modf
.ent modf
modf:
	.frame	sp, 0, ra
#ifdef MOXIE
	mfc1	t0, $f12
	sll	t0, 16
#else
	mfc1	t0, $f13
#endif
	abs.d	$f0, $f12
	l.d	$f6, maxint
	c.lt.d	$f0, $f6
	bc1f	modf2
	add.d	$f2, $f0, $f6
	sub.d	$f2, $f6
	c.le.d	$f2, $f0
	bc1t	modf0
	l.d	$f6, one
	sub.d	$f2, $f6
modf0:
	bge	t0, 0, modf1
	neg.d	$f2
modf1:
	s.d	$f2, (a2)
	sub.d	$f0, $f12, $f2
	j	ret
modf2:
	s.d	$f12, (a2)
	mtc1	$0, $f0
	mtc1	$0, $f1
ret:
	j	ra
.end modf
