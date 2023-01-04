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
 *	@(#)$RCSfile: strcat.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:08:26 $
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
 * Description: strcat
 *
 *    Strcat appends a copy of string2 to the end of string1.
 *
 *    The design goal for this implementation of strcat has been to optimize
 *    for speed.  This implementation first checks for the end of string1 by
 *    unraveling a byte comparision loop.  It then copies string2 to the end
 *    of string1 by jumping to the strcpy routine.  Strcpy then does the return
 *    to the calling routine.
 */
#include "SYS.h"

	.file	"strcat.s"
	.text
	.globl	_strcat
	.globl	strcat_entry


/* Macros checks character at string position "x".
 * If it is zero it branches to ".eofx" or ".eofqx".
 * One pair of macros uses the "addq" instruction instead of "addr".
 */
#define CMPBX(x) \
	cmpqb	$0,x(r1) ; \
	beq	.eof ## x

#define CMPQBX(x) \
	cmpqb	$0,x(r1) ; \
	beq	.eofq ## x

#define EOF(x) \
.eof ## x ## : \
	addr	x ## (r1),r1 ; \
	jump	strcat_entry

#define EOFQ(x) \
.eofq ## x ## : \
	addqd	$ ## x ## ,r1 ; \
	jump	strcat_entry


/* These branch destinations are placed here so the CMPQB(x) macro will use a
 * two byte branch displacement.  This has a measurable performance increase.
 * This scheme is not used for the profiling version of libc.  Otherwise this
 * code would get counted as part of the previous routine.
 */
#ifndef PROF
.eofq0:
	jump	strcat_entry
	EOFQ(1)
	EOFQ(2)
	EOFQ(3)
	EOFQ(4)
	EOFQ(5)
#endif PROF


	.align	4
ENTRY(strcat)
	movd	SP(4),r1		# r1 <- &string1.
	movd	SP(8),r2		# r2 <- &string2.
	movd	r1,r0			# r0 <- &string1 (This is returned.)

.nonzero:
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

	addr	20(r1),r1		# increment &string1.
	br .nonzero

/* These branch destinations are placed here so the CMPB(x)
 * macro will use a single byte branch displacement.
 */
	EOF(13)
	EOF(14)
	EOF(15)
	EOF(16)
	EOF(17)
	EOF(18)
	EOF(19)

/* These branch displacements are only used in
 * the profiling version of libc.
 */
#ifdef PROF
.eofq0:
	jump	strcat_entry
	EOFQ(1)
	EOFQ(2)
	EOFQ(3)
	EOFQ(4)
	EOFQ(5)
#endif PROF

/* These branch destinations couldn't be reached
 * by a single byte branch displacement.
 */
	EOFQ(6)
	EOFQ(7)
	EOF(8)
	EOF(9)
	EOF(10)
	EOF(11)
	EOF(12)
