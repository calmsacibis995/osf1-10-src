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
 *	@(#)$RCSfile: rindex.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:06:09 $
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
 * Original Author: Jim Van Sciver		Created on: 2/2/86
 *
 * Description: rindex
 *
 *    Rindex returns the address of the last match character in a string
 *    or zero if no match is found.
 *
 *    The design goal for this implementation of rindex has been to optimize
 *    for speed.  This implementation unravels a byte comparision loop that
 *    searches for the end of string.  It then branches to another unraveled
 *    byte comparision loop that tries to match the given character.
 *    Branches (which are very expensive) are only take when a comparision
 *    is over or every twenty characters.  This has been the fastest method
 *    with over 40% improvement over the original implementation in C.
 */
#include "SYS.h"

	.file	"rindex.s"
	.text

/* Check for matched character and branch if match.
 * Then check for beginning of string and branch if beginning.
 */
#define CMP(x) \
.cmp ## x ## :\
	cmpb	r2, ## x ## (r1);\
	beq	.match ## x ## 

/* Increment string address, test character 
 * for end of string, and branch if end.
 */
#define EOS(x) \
	cmpqb	$0, ## x ## (r1);\
	beq	.cmp ## x ## 

/* r0 <- address of matched character.
 * Then return to the comparisions.
 */
#define MATCH(x) \
.match ## x ## :\
	addr	x ## (r1),r0;\
	EXIT;	\
	ret	$0


	.align	4
ENTRY(rindex)
	movd	SP(4),r1		# r1 <- string address.
	movb	SP(8),r2		# r2 <- match character.
	movd	r1,r0			# r0 <- string address.

/* First, find the end of the string.
 */
.eos_loop:
	EOS(0)
	EOS(1)
	EOS(2)
	EOS(3)
	EOS(4)
	EOS(5)
	EOS(6)
	EOS(7)
	EOS(8)
	EOS(9)
	EOS(10)
	EOS(11)
	EOS(12)
	EOS(13)
	EOS(14)
	EOS(15)
	EOS(16)
	EOS(17)
	EOS(18)
	EOS(19)
	addr	20(r1),r1		# Increment string address.
	br .eos_loop


/* Now, return to the beginning of the string
 * looking for the match character.
 */
.cmp_loop:
	CMP(19)
	CMP(18)
	CMP(17)
	CMP(16)
	CMP(15)
	CMP(14)
	CMP(13)
	CMP(12)
	CMP(11)
	CMP(10)
	CMP(9)
	CMP(8)
	CMP(7)
	CMP(6)
	CMP(5)
	CMP(4)
	CMP(3)
	CMP(2)
	CMP(1)
	CMP(0)

	cmpd	r0,r1			# Check string addresses.
	beq	.no_match		# Branch if back to beginning.
	addr	-20(r1),r1		# Decrement string address.
	br .cmp_loop

/* No match if back to string beginning.
 * R0 has zero, the return value.
 */
.no_match:
	movqd	$0,r0
	EXIT
	ret	$0


/* Here if match, save address in r0 and return.
 */
	MATCH(0)
	MATCH(1)
	MATCH(2)
	MATCH(3)
	MATCH(4)
	MATCH(5)
	MATCH(6)
	MATCH(7)
	MATCH(8)
	MATCH(9)
	MATCH(10)
	MATCH(11)
	MATCH(12)
	MATCH(13)
	MATCH(14)
	MATCH(15)
	MATCH(16)
	MATCH(17)
	MATCH(18)
	MATCH(19)
	MATCH(20)
