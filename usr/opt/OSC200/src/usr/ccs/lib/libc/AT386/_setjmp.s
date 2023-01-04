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
 *	@(#)$RCSfile: _setjmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:45:17 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * C library -- _setjmp, _longjmp
 *
 *	_longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	_setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 *
 */

#include <machine/asm.h>

ENTRY(_setjmp)
	movl	4(%esp),%ecx		/ fetch buffer
	movl	%ebx,0(%ecx)
	movl	%esi,4(%ecx)
	movl	%edi,8(%ecx)
	movl	%ebp,12(%ecx)		/ save frame pointer of caller
	popl	%edx
	movl	%esp,16(%ecx)		/ save stack pointer of caller
	movl	%edx,20(%ecx)		/ save pc of caller
	movl	$0,84(%ecx)		/ signal mask was not saved
	xorl	%eax,%eax
        jmp     *%edx

ENTRY(_longjmp)
	call	EXT(_fpinit)		/ reset coprocessor
	movl	8(%esp),%eax		/ return(v)
	movl	4(%esp),%ecx		/ fetch buffer
	movl	0(%ecx),%ebx
	movl	4(%ecx),%esi
	movl	8(%ecx),%edi
	movl	12(%ecx),%ebp
	movl	16(%ecx),%esp
	orl	%eax,%eax		/ Don't permit return of zero
	jnz	LBf(notzero,0)
	incl	%eax

LB(notzero,0):
	jmp	*20(%ecx)


