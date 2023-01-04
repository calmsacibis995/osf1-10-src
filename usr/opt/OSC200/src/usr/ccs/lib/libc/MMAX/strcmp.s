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
 *	@(#)$RCSfile: strcmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:08:29 $
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
 * Original Author: Jim Van Sciver		Created on: 1/12/86
 *
 * Description: strcmp
 *
 *    Strcmp compares its arguments and returns an integer greater than,
 *    equal to, or less than zero according as string1 is lexicographically
 *    greater than, equal to, or less that string2.
 *
 *    The design goal for this implementation of strcmp has been to optimize
 *    for speed.  One assumption was that most strings are less than twenty
 *    characters.  Since strcmp terminates at the end of either string or
 *    the first set of different characters the chances are good that we
 *    aren't dealing with two long, equal strings.
 *
 *    The most straight forward assembly language implementation would have
 *    used the cmpsb instruction however careful measurements showed that
 *    the startup time gave relatively small improvements for short strings.
 *    (It was very fast for strings greater than 100 characters, big deal.)
 *
 *    This implementation just unravels a byte comparision loop so that
 *    branches (which are very expensive) are only take when the comparision
 *    is over or every twenty characters.  This has been the fastest method
 *    with over 30% improvement over the original implementation in C.
 */


#include "SYS.h"


/* Macro compares characters at string position "x".  If they are
 * different it branches to ".diff".  If they are equal and zero
 * then the strings are equal and a branch to ".zero".
 */
#define CMPBX(x) \
	movb	x(r2),r0 ; \
	cmpb	r0,x(r1) ; \
	bne	.diff    ; \
	cmpqb	$0,r0    ; \
	beq	.zero

	.file	"strcmp.s"
	.text

	.globl	_strcmp
	.align	4


ENTRY(strcmp)
	movd	SP(8),r2		# r2 <- &string2
	movd	SP(4),r1		# r1 <- &string1
.same:
	CMPBX(0)
	CMPBX(1)
	CMPBX(2)
	CMPBX(3)
	CMPBX(4)
	CMPBX(5)
	CMPBX(6)
	CMPBX(7)
	CMPBX(8)
	CMPBX(9)
	CMPBX(10)
	CMPBX(11)
	CMPBX(12)
	CMPBX(13)
	CMPBX(14)
	CMPBX(15)
	CMPBX(16)
	CMPBX(17)
	CMPBX(18)
	CMPBX(19)
	CMPBX(20)

	addr	20(r1),r1		# increment &string1.
	addr	20(r2),r2		# increment &string2.
	br .same

.diff:	movqd	$1,r0			# Assume that string1 greater
	blo	.done			# Yup, it was
	negd	r0,r0			# Since not equal, must be less
	br	.done
.zero:
	movqd	$0,r0
.done:	EXIT
	ret	$0
