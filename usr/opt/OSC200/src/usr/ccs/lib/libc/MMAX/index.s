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
 *	@(#)$RCSfile: index.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:02:51 $
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
 * Original Author: Jim Van Sciver		Created on: 1/13/86
 *
 * Description: index
 *
 *    Index returns a pointer to the first occurrence of a character in a
 *    string or zero if the character is not found.
 *
 *    The design goal for this implementation of index has been to optimize
 *    for speed.  The most straight forward assembly language implementation
 *    would have used the skpsb instruction however careful measurements showed
 *    that the startup time gave negative improvements for short strings.
 *
 *    This implementation just unravels a byte comparision loop so that
 *    branches (which are very expensive) are only take when the comparision
 *    is over or every twenty characters.  This has been the fastest method
 *    with over 30% improvement over the original implementation in C.
 */
#include "SYS.h"

	.file	"index.s"
	.text
	.globl	_index

/* Macros are destination when the character at string position
 * "x" ends the string.  The string size is returned in r0.
 */
#define MATCHX(x) \
.match ## x ## : \
	addr	x ## (r0),r0; \
	EXIT;	\
	ret	$0

#define MATCHQX(x) \
.matchq ## x ## : \
	addqd	$ ## x ## ,r0; \
	EXIT;	\
	ret	$0

#define CMPBX(x) \
	cmpb	r2,x(r1) ; \
	beq	.match ## x ; \
	cmpqb	$0,x(r1) ; \
	beq	.eos

#define CMPQBX(x) \
	cmpb	r2,x(r1) ; \
	beq	.matchq ## x ; \
	cmpqb	$0,x(r1) ; \
	beq	.eos



/* These branch destinations have been placed near their corresponding
 * branch instructions so that a two (versus three) byte displacement
 * will be used.  This has a measurable performance increase.  This scheme
 * is not used if profiling has been enabled so that the accounting will
 * be more accurate.
 */
#ifndef PROF
.eos:
	movqd	$0,r0			# r0 <- null.
	EXIT
	ret $0

.matchq0:
	EXIT
	ret $0

	MATCHQX(1)
	MATCHQX(2)
	MATCHQX(3)
#endif PROF

	.align	4
ENTRY(index)
	movd	SP(4),r1		# r1 <- &string
	movb	SP(8),r2		# r2 <- match character.
	movd	r1,r0			# r0 <- &string

.same:
	CMPQBX(0)
	CMPQBX(1)
	CMPQBX(2)
	CMPQBX(3)
	CMPQBX(4)
	CMPQBX(5)
	CMPQBX(6)
	CMPQBX(7)
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

	addr	20(r0),r0		# increment &matched character.
	addr	20(r1),r1		# increment &string.
	br .same

	MATCHX(18)
	MATCHX(19)
	MATCHX(20)

/* This section is only used if profiling is turned on.
 */
#ifdef PROF
.eos:
	movqd	$0,r0			# r0 <- null.
	EXIT
	ret $0

.matchq0:
	EXIT
	ret $0

	MATCHQX(1)
	MATCHQX(2)
	MATCHQX(3)
#endif PROF


/* These branch destinations were not 
 * within the two byte range.
 */
	MATCHQX(4)
	MATCHQX(5)
	MATCHQX(6)
	MATCHQX(7)
	MATCHX(8)
	MATCHX(9)
	MATCHX(10)
	MATCHX(11)
	MATCHX(12)
	MATCHX(13)
	MATCHX(14)
	MATCHX(15)
	MATCHX(16)
	MATCHX(17)
