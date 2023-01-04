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
 *	@(#)$RCSfile: setjmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:07:01 $
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
 * C library -- setjmp, longjmp
 *
 *	  longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	   setjmp(a)
 * by restoring r3-r7, fp, sb, sp, pc, and the signal mask from 'a'
 * and doing a return.
 * NOTE: As it is, longjmp only restores the signal mask. It does not
 * 	 restore the signal-stack or the notion of whether we're on the
 *	 signal stack.  This should be remedied.
 */
	.file	"setjmp.s"
	.globl	_sigblock
	.globl	_sigsetmask

#include "SYS.h"

	.align  2
ENTRY(setjmp)
	movqd	$0,TOS		# Push a zero
	jsr	_sigblock	# Get signal mask
	adjspb	$-4
	movd	SP(4),r1	# Get 'a' - above $0 and pc on stack
	movd	r3,0(r1)
	movd	r4,4(r1)
	movd	r5,8(r1)
	movd	r6,12(r1)
	movd	r7,16(r1)
	addr	0(fp),20(r1)	# fp
	addr	0(sb),24(r1)	# sb
	addr	SP(4),28(r1)	# sp
	movd	SP(0),32(r1)	# pc
	movd	r0,36(r1)	# signal mask
	movqd	$1,40(r1)	# signal mask was saved
	movqd   $0,r0
	EXIT
	ret	$0

	.align  2
ENTRY(longjmp)
	movd	SP(4),r1		# Restore signal mask
	movd	36(r1),TOS
	jsr	_sigsetmask
	adjspb	$-4
	movd	SP(4),r1		# Now restore state
	movd	SP(8),r0                # return(v)
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
	movqd   $1,r0
.L2:
	movd	32(r1),r1   # pc
	jump	0(r1)
