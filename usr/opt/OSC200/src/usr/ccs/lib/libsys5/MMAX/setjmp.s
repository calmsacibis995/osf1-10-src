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
 *	@(#)$RCSfile: setjmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:19:08 $
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
/*	@(#)setjmp.s	3.1
 *	
 *	2/26/91
 * System V Compatibility library -- setjmp, longjmp
 *
 * longjmp(a,v)
 *	will generate a "return(v)" from the last call to
 *
 * setjmp(a)
 *	 by restoring r3-r7, fp, sb, sp, and pc from 'a' and doing a return.
 *
 */
	.file	"setjmp.s"

#include "SYS.h"

	.align  2
ENTRY(setjmp)
	movd	SP(4),r0
	movd	r3,0(r0)
	movd	r4,4(r0)
	movd	r5,8(r0)
	movd	r6,12(r0)
	movd	r7,16(r0)
	addr	0(fp),20(r0)	# fp
	addr	0(sb),24(r0)	# sb
	addr	SP(4),28(r0)	# sp - Caller pops the arg off
	movd	SP(0),32(r0)	# pc
	movqd	$0,40(r0)	# signal mask was not saved
	movqd	$0,r0
	EXIT
	ret	$0

	.align	2
ENTRY(longjmp)
	movd	SP(8),r0		# return(v)
	movd	SP(4),r1
	movd	0(r1),r3
	movd	4(r1),r4
	movd	8(r1),r5
	movd	12(r1),r6
	movd	16(r1),r7
	lprd	fp,20(r1)
	lprd	sb,24(r1)
	lprd	sp,28(r1)
	cmpqd   $0,r0
	bne	.L2
	movqd	$1,r0
.L2:
	movd	32(r1),r1   # pc
	jump	0(r1)
