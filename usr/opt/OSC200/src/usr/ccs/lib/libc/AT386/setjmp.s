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
 *	@(#)$RCSfile: setjmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:54:26 $
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS

#endif LIBC_SCCS


/*
 * C library -- setjmp, longjmp
 *
 *	longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	setjmp(a)
 * by restoring registers from the stack,
 * The previous value of the signal mask is
 * restored.
 *
 */

#include <machine/asm.h>

#ifdef	wheeze
#define MOV	movw
#else
#define MOV	mov
#endif

ENTRY(setjmp)
	movl	4(%esp),%ecx		/ fetch buffer
	subl	$8, %esp
	pushl	%esp
	pushl	$0
	call	EXT(sigstack)
	movl	12(%esp), %eax		/ save onstack value
	movl	%eax, (%ecx)
	addl	$16, %esp
	pushl	$0			/ call sigblock(0) to obtain
	call	EXT(sigblock)		/ current value of signal mask
	movl	%eax, 4(%ecx)
	popl	%eax
	movl	%esp, %eax
	movl	%ecx, %esp
	addl	$56, %esp
/	lea	56(%ecx), %esp
	pusha
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%eax, %esp
	popl	%edx			/ get caller's instruction pointer
	movl    %edx, 64(%ecx)		/ save instruction pointer
	MOV	%cs, 68(%ecx)
	pushf				/ save flags
	popl	72(%ecx)
	movl    %esp, 36(%ecx)		/ correct saved stack pointer value
        movl	%esp, 76(%ecx)		/ save stack pointer of caller in UESP
	movl	$0, 80(%ecx)
	MOV	%ss, 80(%ecx)
	movl	$1, 84(%ecx)		/ signal mask was saved
	xor	%eax,%eax
        jmp     *%edx

ENTRY(longjmp)
	call	EXT(_fpinit)		/ reset coprocessor
	movl	4(%esp), %eax		/ address of jmp_buf (saved context)
	movl	8(%esp), %edx		/ return value
	orl	%edx,%edx		/ Check for attempt to return zero
	 jne	LBf(notzero,0)
	inc	%edx			/ Force to non-zero return value

LB(notzero,0):
	movl	%edx, 52(%eax)		/ return value into saved context
	movl	$103, %eax		/ sigreturn system call
	SVC
	addl	$8, %esp
	call	EXT(longjmperror)
