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
 *	@(#)$RCSfile: strcpy.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:08:33 $
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
/*
 *	      Copyright (C) 1985 Encore Computer Corporation
 *
 *    ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 *    Corporation.  This software is made available solely pursuant to the
 *    terms  of a software  license agreement  which governs its use.  Un-
 *    authorized duplication, distribution or sale are strictly prohibited.
 *
 * Original Author: Jim Van Sciver		Created on: 1/14/86
 *
 * Description: strcpy
 *
 *    Strcpy copies string2 to string1.
 *
 *    The design goal for this implementation of strcpy has been to optimize
 *    for speed.  The most straight forward assembly language implementation
 *    would have used the movsb instruction however careful measurements showed
 *    that the startup time gave negative improvements for short strings.
 *
 *    This implementation unravels a byte comparision loop so that
 *    branches (which are very expensive) are only take when the comparision
 *    is over or every twenty characters.  This has been the fastest method
 *    with over 30% improvement over the original implementation in C.
 *
 *    Note that multiple movd instructions are twice as fast as movmd.
 */

#include "SYS.h"

	.file	"strcpy.s"
	.text
	.globl	_strcpy
	.globl	strcat_entry


/* Macro copies character at string position "x".
 * If it is zero it branches to ".diff".
 */
#define CPYBX(x) \
	cmpqb	$0,x(r2) ; \
	beq	.diff ## x


/* These branch destinations have been placed near their corresponding
 * branch instructions so that a two (versus three) byte displacement
 * will be used.  This has a measurable performance increase.  This scheme
 * is not used for the profiling version of libc.  Otherwise this code
 * would get counted as part of the previous routine.
 */
#ifndef PROF
.diff0:
	movqb	$0,0(r1)
	EXIT
	ret	$0

.diff1:
	movw	0(r2),0(r1)
	EXIT
	ret	$0

.diff2:
	movw	0(r2),0(r1)
	movqb   $0,2(r1)
	EXIT
	ret	$0

.diff3:
	movd	0(r2),0(r1)
	EXIT
	ret	$0

.diff4:
	movd	0(r2),0(r1)
	movqb   $0,4(r1)
	EXIT
	ret	$0

.diff5:
	movd	0(r2),0(r1)
	movw	4(r2),4(r1)
	EXIT
	ret	$0
#endif PROF

	.align	4
ENTRY(strcpy)
	movd	SP(4),r1		# r1 <- &string1.
	movd	SP(8),r2		# r2 <- &string2.
	movd	r1,r0			# r0 <- &string1 (This is returned.)

.same:
strcat_entry:
	CPYBX(0)
	CPYBX(1)
	CPYBX(2)
	CPYBX(3)
	CPYBX(4)
	CPYBX(5)
	CPYBX(6)
	CPYBX(7)
	CPYBX(8)
	CPYBX(9)
	CPYBX(10)
	CPYBX(11)
	CPYBX(12)
	CPYBX(13)
	CPYBX(14)
	CPYBX(15)
	CPYBX(16)
	CPYBX(17)
	CPYBX(18)
	CPYBX(19)

	movd	0(r2),0(r1)		# Do a block copy of
	movd	4(r2),4(r1)		# of string2 to string1.
	movd	8(r2),8(r1)
	movd	12(r2),12(r1)
	movd	16(r2),16(r1)
	addr	20(r1),r1		# increment &string1.
	addr	20(r2),r2		# increment &string2.
	br .same

.diff6:
	movd	0(r2),0(r1)
	movw	4(r2),4(r1)
	movqb   $0,6(r1)
	EXIT
	ret	$0

.diff7:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	EXIT
	ret	$0

.diff8:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movqb   $0,8(r1)
	EXIT
	ret	$0

.diff9:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movw	8(r2),8(r1)
	EXIT
	ret	$0

.diff10:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movw	8(r2),8(r1)
	movqb   $0,10(r1)
	EXIT
	ret	$0

.diff11:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	EXIT
	ret	$0

.diff12:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movqb   $0,12(r1)
	EXIT
	ret	$0

.diff13:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movw	12(r2),12(r1)
	EXIT
	ret	$0

.diff14:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movw	12(r2),12(r1)
	movqb   $0,14(r1)
	EXIT
	ret	$0

.diff15:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movd	12(r2),12(r1)
	EXIT
	ret	$0

.diff16:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movd	12(r2),12(r1)
	movqb   $0,16(r1)
	EXIT
	ret	$0

.diff17:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movd	12(r2),12(r1)
	movw	16(r2),16(r1)
	EXIT
	ret	$0

.diff18:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movd	12(r2),12(r1)
	movw	16(r2),16(r1)
	movqb   $0,18(r1)
	EXIT
	ret	$0
	
.diff19:
	movd	0(r2),0(r1)
	movd	4(r2),4(r1)
	movd	8(r2),8(r1)
	movd	12(r2),12(r1)
	movd	16(r2),16(r1)
	EXIT
	ret	$0


/* This section is only used for the profiled libc.
 */
#ifdef PROF
.diff0:
	movqb	$0,0(r1)
	EXIT
	ret	$0

.diff1:
	movw	0(r2),0(r1)
	EXIT
	ret	$0

.diff2:
	movw	0(r2),0(r1)
	movqb   $0,2(r1)
	EXIT
	ret	$0

.diff3:
	movd	0(r2),0(r1)
	EXIT
	ret	$0

.diff4:
	movd	0(r2),0(r1)
	movqb   $0,4(r1)
	EXIT
	ret	$0

.diff5:
	movd	0(r2),0(r1)
	movw	4(r2),4(r1)
	EXIT
	ret	$0
#endif PROF
