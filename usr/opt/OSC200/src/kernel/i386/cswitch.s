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
 *	@(#)$RCSfile: cswitch.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:17:07 $
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
  *  Copyright 1988, 1989 by Intel Corporation
  */

#include <i386/asm.h>

	.text

#define	O_EDI	0
#define	O_ESI	4
#define	O_EBX	8
#define	O_EBP	12
#define	O_ESP	16
#define O_EIP	20
#define	O_IPL	24

ENTRY(save_context)
	/*
	 * We would like to do an "fwait" here so that any pending
	 * floating point exceptions happen before the context switch.
	 * Unfortunately, at least on an AT, this can cause the system
	 * to hang.
	 */
	call	EXT(get_pcb_context)
	movl	%eax,%edx		/ address of save area
	movl	EXT(curr_ipl), %eax
	movl	%eax, O_IPL(%edx)	/ save ipl level for longjmp
	call	EXT(splhi)
	movl	%edi, O_EDI(%edx)
	movl	%esi, O_ESI(%edx)
	movl	%ebx, O_EBX(%edx)
	movl	%ebp, O_EBP(%edx)
	movl	%esp, O_ESP(%edx)
	movl	(%esp), %ecx		/ %eip (return address)
	movl	%ecx, O_EIP(%edx)
	subl	%eax, %eax		/ retval <- 0
	ret

ENTRY(setjmp)
	movl	4(%esp), %edx		/ address of save area
	movl	EXT(curr_ipl), %eax
	movl	%eax, O_IPL(%edx)	/ save ipl level for longjmp
	movl	%edi, O_EDI(%edx)
	movl	%esi, O_ESI(%edx)
	movl	%ebx, O_EBX(%edx)
	movl	%ebp, O_EBP(%edx)
	movl	%esp, O_ESP(%edx)
	movl	(%esp), %ecx		/ %eip (return address)
	movl	%ecx, O_EIP(%edx)
	subl	%eax, %eax		/ retval <- 0
	ret

ENTRY(longjmp)
	call	EXT(splhi)
	movl	4(%esp), %edx		/ address of save area
	movl	O_EDI(%edx), %edi
	movl	O_ESI(%edx), %esi
	movl	O_EBX(%edx), %ebx
	movl	O_EBP(%edx), %ebp
	movl	O_ESP(%edx), %esp
	movl	O_EIP(%edx), %ecx	/ %eip (return address)
#if	STACK_LIMIT_CHECK
/ following code loads the limit field of the kernel stack selector
/ descriptor with the current thread's kernel stack base. This prevents
/ writing into the last page of the kernel stack (you had better have at
/ least two pages of kernel stack!). when the stack overflows a double
/ fault will happen (actually you will get a stack segment exception, but
/ will then get a double fault because the kernel stack isn't usable).
	movl	EXT(active_threads),%eax
	movl	THREAD_STACK(%eax),%eax	/ get stack base
	shrl	$12,%eax		/ drop page part
	movw	%ax,EXT(gdt)+KSSSEL
	shrl	$16,%eax		/ get high order bits
	orb	$0x80+0x40,%al		/ set D + G
	movb	%al,EXT(gdt)+6+KSSSEL
	movw	$KSSSEL,%ax
	movw	%ax,%ss			/ access new descriptor
#endif	/* STACK_LIMIT_CHECK */
	pushl	%ecx
	pushl	O_IPL(%edx)
	call	EXT(splx)		/ restore ipl level
	popl	%edx
	popl	%ecx
	movl	$1, %eax		/ ret val <- 1
	addl	$4, %esp		/ ret address
	jmp	*%ecx			/ indirect

/ENTRY(setjmp)
/	movl	4(%esp), %edx		/ address of save area
/	movl	EXT(curr_ipl), %eax
/	movl	%eax, O_IPL(%edx)	/ save ipl level for longjmp
/	movl	%edi, O_EDI(%edx)
/	movl	%esi, O_ESI(%edx)
/	movl	%ebx, O_EBX(%edx)
/	movl	%ebp, O_EBP(%edx)
/	movl	%esp, O_ESP(%edx)
/	movl	(%esp), %ecx		/ %eip (return address)
/	movl	%ecx, O_EIP(%edx)
/	subl	%eax, %eax		/ retval <- 0
/	ret
/
/ENTRY(longjmp)
/	call	EXT(splhi)
/	movl	4(%esp), %edx		/ address of save area
/	movl	O_EDI(%edx), %edi
/	movl	O_ESI(%edx), %esi
/	movl	O_EBX(%edx), %ebx
/	movl	O_EBP(%edx), %ebp
/	movl	O_ESP(%edx), %esp
/	movl	O_EIP(%edx), %eax	/ %eip (return address)
/	movl	%eax, 0(%esp)
/	pushl	O_IPL(%edx)
/	call	EXT(splx)		/ restore ipl level
/	popl	%edx
/	popl	%eax			/ ret addr != 0
/	jmp	*%eax			/ indirect

ENTRY(kdbsetjmp)
	movl	4(%esp), %edx		/ address of save area
	movl	%edi, O_EDI(%edx)
	movl	%esi, O_ESI(%edx)
	movl	%ebx, O_EBX(%edx)
	movl	%ebp, O_EBP(%edx)
	movl	%esp, O_ESP(%edx)
	movl	(%esp), %ecx		/ %eip (return address)
	movl	%ecx, O_EIP(%edx)
	subl	%eax, %eax		/ retval <- 0
	ret

ENTRY(kdblongjmp)
	movl	8(%esp), %eax		/ return value
	movl	4(%esp), %edx		/ address of save area
	movl	O_EDI(%edx), %edi
	movl	O_ESI(%edx), %esi
	movl	O_EBX(%edx), %ebx
	movl	O_EBP(%edx), %ebp
	movl	O_ESP(%edx), %esp
	movl	O_EIP(%edx), %ecx		/ %eip (return address)
	addl	$4, %esp		/ pop ret adr
	jmp	*%ecx			/ indirect


#define T_EIP	32
#define T_EAX	T_EIP+8
#define T_ECX	T_EAX+4
#define T_EDX	T_ECX+4
#define T_EBX	T_EDX+4
#define T_ESP	T_EBX+4
#define T_EBP	T_ESP+4
#define T_ESI	T_EBP+4
#define T_EDI	T_ESI+4
#define T_ES	T_EDI+4
#define T_CS	T_ES+4
#define T_SS	T_CS+4
#define T_DS	T_SS+4
#define T_FS	T_DS+4
#define T_GS	T_FS+4

ENTRY(save_all_regs)
	pushl	%eax			/ squirrel away eax and edx
	pushl	%edx			/ because this routine uses them
	call	EXT(get_pcb_tss)
	movl	%eax,%edx
	movl	%ecx, T_ECX(%edx)
	movl	%ebx, T_EBX(%edx)
	movl	%esp, T_ESP(%edx)
	movl	%ebp, T_EBP(%edx)
	movl	%esi, T_ESI(%edx)
	movl	%edi, T_EDI(%edx)
	movl	%es, T_ES(%edx)
	movl	%cs, T_CS(%edx)
	movl	%ss, T_SS(%edx)
	movl	%ds, T_DS(%edx)
	movl	%fs, T_FS(%edx)
	movl	%gs, T_GS(%edx)
	movl	%ebx, T_EBX(%edx)
	movl	(%esp), %ecx		/ %eip (return address)
	movl	%ecx, T_EIP(%edx)
	popl	%eax
	movl	%eax, T_EAX(%edx)
	movl	%edx,%eax		/ %eax <- offset
	popl	%edx
	movl	%edx, T_EDX(%eax)
	ret
