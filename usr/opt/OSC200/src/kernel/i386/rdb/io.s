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
 *	@(#)$RCSfile: io.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:25 $
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#include "i386/asm.h"
#include "i386/reg.h"
	.globl	EXT(in)
	.globl	EXT(out)
#define	TMP1 %edx
#define	TMP1W %dx
	.text
EXT(in):	movl	4(%esp),%edx
	xor	%eax,%eax
	INB
	ret
EXT(out):	movl	4(%esp),%edx	/ port
	movl	8(%esp),%eax	/ value
	OUTB
	.globl	EXT(_mfsr)
EXT(_mfsr):
	mov	4(%esp),%eax	/ register number
	and	$7,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	movw	%es,%ax		/ getcs
	ret
	.align	ALIGN
	movw	%cs,%ax		/ getcs
	ret
	.align	ALIGN
	movw	%ss,%ax
	ret
	.align	ALIGN
	movw	%ds,%ax
	ret
	.align	ALIGN
	movw	%fs,%ax
	ret
	.align	ALIGN
	movw	%gs,%ax
	ret
	.align	ALIGN
	movw	%gs,%ax
	ret
	.align	ALIGN
	movw	%gs,%ax
	ret

	/ mtsr(register,value)
	.globl	EXT(_mtsr)
EXT(_mtsr):
	mov	8(%esp),TMP1	/ get value
	mov	4(%esp),%eax	/ register number
	and	$7,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	movw	TMP1W,%es
	ret
	.align	ALIGN
	movw	TMP1W,%cs	/ better not really do this!
	ret
	.align	ALIGN
	movw	TMP1W,%ss
	ret
	.align	ALIGN
	movw	TMP1W,%ds
	ret
	.align	ALIGN
	movw	TMP1W,%fs
	ret
	.align	ALIGN
	movw	TMP1W,%gs
	ret
	.align	ALIGN
	movw	TMP1W,%gs
	ret
	.align	ALIGN
	movw	TMP1W,%gs
	ret

	.globl	EXT(mfcr)
EXT(mfcr):
	mov	4(%esp),%eax	/ register number
	and	$3,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	mov	%cr0,%eax	/ get cr0
	ret
	.align	ALIGN
	mov	%cr0,%eax	/ no cr1
	ret
	.align	ALIGN
	mov	%cr2,%eax	/ get cr2
	ret
	.align	ALIGN
	mov	%cr3,%eax	/ get cr3
	ret
	.align	ALIGN
	.globl	EXT(mfdr)
EXT(mfdr):
	mov	4(%esp),%eax	/ register number
	and	$7,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	mov	%db0,%eax	/ get dr0
	ret
	.align	ALIGN
	mov	%db1,%eax	/ get dr1
	ret
	.align	ALIGN
	mov	%db2,%eax	/ get dr2
	ret
	.align	ALIGN
	mov	%db3,%eax	/ get dr3
	ret
	.align	ALIGN
	mov	%db3,%eax	/ get dr4
	ret
	.align	ALIGN
	mov	%db3,%eax	/ get dr5
	ret
	.align	ALIGN
	mov	%db6,%eax	/ get dr6
	ret
	.align	ALIGN
	mov	%db7,%eax	/ get dr7
	ret
	.align	ALIGN

	.globl	EXT(ltr)
EXT(ltr):
	mov	4(%esp),%eax
	ltr	%ax
	ret

	.globl	EXT(str)
EXT(str):
	str	%ax
	ret

/ sgdt(&limit) (addr is 2 bytes beyond limit)
	.globl	EXT(sgdt)
EXT(sgdt):	mov	4(%esp),%eax
	sgdt	0(%eax)
	ret

	.globl	EXT(sldt)
EXT(sldt):
	xor	%eax,%eax
	sldt	%eax
	ret

/ lgdt(&limit) (addr is 2 bytes beyond limit)
	.globl	EXT(lgdt)
EXT(lgdt):	mov	4(%esp),%eax
	lgdt	0(%eax)
	ret
/ sidt(&limit) (addr is 2 bytes beyond limit)
	.globl	EXT(sidt)
EXT(sidt):	mov	4(%esp),%eax
	sidt	0(%eax)
	ret

/ lidt(&limit) (addr is 2 bytes beyond limit)
	.globl	EXT(lidt)
EXT(lidt):	mov	4(%esp),%eax
	lidt	0(%eax)
	ret
	.globl	EXT(mfgr)
EXT(mfgr):
	mov	4(%esp),%eax	/ register number
	and	$7,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	mov	%eax,%eax
	ret
	.align	ALIGN
	mov	%ecx,%eax
	ret
	.align	ALIGN
	mov	%edx,%eax
	ret
	.align	ALIGN
	mov	%ebx,%eax
	ret
	.align	ALIGN
	mov	%esp,%eax
	ret
	.align	ALIGN
	mov	%ebp,%eax
	ret
	.align	ALIGN
	mov	%esi,%eax
	ret
	.align	ALIGN
	mov	%edi,%eax
	ret

	/ get the return address
	.globl	EXT(mfip)
EXT(mfip):	mov	0(%esp),%eax
	ret
	/ get the flags
	.globl	EXT(mff)
EXT(mff):
	pushf
	pop	%eax
	ret


/ assume that first two parameters are the appropriate 
/ arguments for iret(cs,eip)
	.globl	EXT(iret)
EXT(iret):
	pop	%eax		/ get real return address
	iret			/ return thru the parameters
#ifdef _AIX
/ note that we fall thru to breakint so that after the iret
/ returns us to a task we will start execution at the right
/ spot

/ entered when we get an exception thru the debugger's tss
/ we map the tss selector back into the original interrupt
/ number so we can tell why we were entered
	.globl	EXT(breakint)
	.globl	EXT(debug_stack)
EXT(breakint):
	popl	TMP1		/ get possible error code
	mov	debug_stack,%esp	/get stack pointer
	str	%ax		/ get task selector 
	sub	dtss_s,%eax	/ get offset from base
	shr	$3,%eax
	push	TMP1		/ push error code
	push	%eax		/ push interrupt # 
	call	EXT(trap)		/ invoke debugger trap handler
	addl	$8,%esp		/ pop stack (not really required)
	iret			/ return to caller
#endif /* AIX */

	.globl	EXT(int3)
EXT(int3):	int	$3
	ret

/ stack 
/	0	return address
/	4	first parameter (segment) 
/	6	segment portion of first parameter
/	8	second parameter (offset)
	.globl	EXT(debugger)
EXT(debugger):
#ifdef PS2
	lcall	*6(%esp)	/call thru argument (task gate)
#endif
	ret			/ return to caller

	.globl	EXT(enable)
EXT(enable):
	sti
	ret

	.globl EXT(disable)
EXT(disable):
	cli
	ret

	.globl	EXT(call_done)
	.align	ALIGN
/ code copied onto the stack for a call in program context
EXT(call_done):
	add	$12,%esp	/ (3 bytes) we provide 3 args
	int	$1		/ (2 bytes) and enter the debugger again
	.byte	0,0,0		/ (fill)

	.globl	EXT(tlb_flush)
	.align	ALIGN
EXT(tlb_flush):
	movl	%cr3,%eax
	movl	%eax,%cr3
	ret

/ the following are assocated with debugexception which expects to be
/ entered after mov instruction has generated an exception.
	.globl	EXT(_hfetch)
EXT(_hfetch):
	movl	%esp,TMP1
	movl	4(%esp),%eax
	movzwl	(%eax),%eax	/ expect exception here
1:
	movl	TMP1,%esp
	ret
	
	.globl	EXT(_wfetch)
EXT(_wfetch):
	movl	%esp,TMP1
	movl	4(%esp),%eax
	movl	(%eax),%eax	/ expect exception here
1:
	movl	TMP1,%esp
	ret
	
	.globl	EXT(_bfetch)
EXT(_bfetch):
	movl	%esp,TMP1
	movl	4(%esp),%eax
	movzbl	(%eax),%eax	/ expect exception here
1:
	movl	TMP1,%esp
	ret
	
/ if an exception happens just set the address to the return instruction
/ and set eax to -1.
/ we expect the stack to have: eflags, cs, address, error-code
/ (we only except to get the following exceptions: 8, 10, 11, 12, 13, 14
	.globl	EXT(debugexception)
EXT(debugexception):
	movl	$-1,%eax	/ value to return
	addl	$4,%esp		/ get rid of error code
	movl	$1b,(%esp)
	iret			/ return from interrupt
	.globl	EXT(mtcr)
EXT(mtcr):
	mov	8(%esp),TMP1	/ value
	mov	4(%esp),%eax	/ register number
	and	$3,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	mov	TMP1,%cr0	/ set cr0
	ret
	.align	ALIGN
	mov	TMP1,%cr0	/ no cr1
	ret
	.align	ALIGN
	mov	TMP1,%cr2	/ set cr2
	ret
	.align	ALIGN
	mov	TMP1,%cr3	/ set cr3
	ret
	.align	ALIGN
	.globl	EXT(mtdr)
/ mtdr(reg,value)
EXT(mtdr):
	mov	8(%esp),TMP1	/ value
	mov	4(%esp),%eax	/ register number
	and	$7,%eax		/ get down to 3 bits
	shl	$2,%eax		/ times 4
	add	$1f,%eax
	jmp	*%eax
	.align	ALIGN
1:
	mov	TMP1,%db0	/ set dr0
	ret
	.align	ALIGN
	mov	TMP1,%db1	/ set dr1
	ret
	.align	ALIGN
	mov	TMP1,%db2	/ set dr2
	ret
	.align	ALIGN
	mov	TMP1,%db3	/ set dr3
	ret
	.align	ALIGN
	mov	TMP1,%db3	/ set dr4
	ret
	.align	ALIGN
	mov	TMP1,%db3	/ set dr5
	ret
	.align	ALIGN
	mov	TMP1,%db6	/ set dr6
	ret
	.align	ALIGN
	mov	TMP1,%db7	/ set dr7
	ret
	.align	ALIGN

/ build up a pseudo-tss for the debugger when "called"
	.globl	EXT(set_tss)
EXT(set_tss):
	push	%eax
	mov	8(%esp),%eax	/ get the argument

	mov	%gs, Times(GS,4)(%eax)
	mov	%fs, Times(FS,4)(%eax)
	mov	%es, Times(ES,4)(%eax)
	mov	%ds, Times(DS,4)(%eax)

	mov	%edi, Times(EDI,4)(%eax)
	mov	%esi, Times(ESI,4)(%eax)
	mov	%ebp, Times(EBP,4)(%eax)
	mov	%esp, Times(ESP,4)(%eax)
	mov	%ebx, Times(EBX,4)(%eax)
	mov	%edx, Times(EDX,4)(%eax)
	mov	%ecx, Times(ECX,4)(%eax)

	pop	Times(EAX,4)(%eax)		/ get saved eax
	push	Times(EAX,4)(%eax)		/ save it again

	movl	$0, Times(TRAPNO,4)(%eax)
	movl	$0, Times(ERR,4)(%eax)

	push	4(%esp)
	pop	Times(EIP,4)(%eax)	/ return address

	mov	%cs,Times(CS,4)(%eax)

	pushf
	pop	Times(EFL,4)(%eax)		/eflags

	add	$12, Times(ESP,4)(%eax)		/ ignore saved eax, return, arg 1

	pop	%eax
	ret

/	called with the "tss" that contains the information to do a return
	.globl	EXT(db_return)
EXT(db_return):
	mov	4(%esp), %esp		/ load stack from the argument
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa
	addl	$0x08, %esp
	iret

/ set the trace bit
	.globl	EXT(set_tf)
EXT(set_tf):
	pushf
	orl	$0x100,(%esp)
	popf			/ into flags
	ret
