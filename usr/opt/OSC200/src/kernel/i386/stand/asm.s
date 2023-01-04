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
 *	@(#)$RCSfile: asm.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */


/			INTEL CORPORATION PROPRIETARY INFORMATION
/
/	This software is supplied under the terms of a license  agreement or 
/	nondisclosure agreement with Intel Corporation and may not be copied 
/	nor disclosed except in accordance with the terms of that agreement.
/
/	Copyright 1988 Intel Corporation
/ Copyright 1988, 1989 by Intel Corporation
/

	.file "asm.s"

#include "machine/asm.h"

BOOTSEG		=	0x100

CR0_PE_ON	=	0x1
CR0_PE_OFF	=	0xfffffffe

	.text

/
/ real_to_prot()
/ 	transfer from real mode to protected mode.
/

ENTRY(real_to_prot)
	/ guarantee that interrupt is disabled when in prot mode
	cli

	/ load the gdtr
	addr16
	data16
	lgdt	EXT(Gdtr)

	/ set the PE bit of CR0
	mov	%cr0, %eax

	data16
	or	$CR0_PE_ON, %eax
	mov	%eax, %cr0 

	/ make intrasegment jump to flush the processor pipeline and
	/ reload CS register
	data16
	ljmp	$0x08, $xprot

xprot:
	/ we are in USE32 mode now
	/ set up the protective mode segment registers : DS, SS, ES
	mov	$0x10, %eax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es

	ret

/
/ prot_to_real()
/ 	transfer from protected mode to real mode
/ 

ENTRY(prot_to_real)

	/ change to USE16 mode
	ljmp	$0x18, $x16

x16:
	/ clear the PE bit of CR0
	mov	%cr0, %eax
	data16
	and 	$CR0_PE_OFF, %eax
	mov	%eax, %cr0


	/ make intersegment jmp to flush the processor pipeline
	/ and reload CS register
	data16
	ljmp $BOOTSEG, $xreal


xreal:
	/ we are in real mode now
	/ set up the real mode segment registers : DS, SS, ES
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es

	data16
	ret

/
/ outb(port, byte)
/

ENTRY(outb)
	push	%ebp
	mov	%esp, %ebp
	push	%edx

	movw	8(%ebp), %dx
	movb	12(%ebp), %al
/	outb	(%dx)
	OUTB

	pop	%edx
	pop	%ebp
	ret

/
/ inb(port)
/

ENTRY(inb)
	push	%ebp
	mov	%esp, %ebp
	push	%edx

	movw	8(%ebp), %dx
	subw	%ax, %ax
/	inb	(%dx)
	INB

	pop	%edx
	pop	%ebp
	ret


/
/ halt()
/

ENTRY(halt)
	call	EXT(getchar)
	jmp	EXT(halt)
	hlt



/
/ startprog(phyaddr)
/	start the program on protected mode where phyaddr is the entry point
/

ENTRY(startprog)
	push	%ebp
	mov	%esp, %ebp

	/ push some number of args onto the stack
	movl	0x0c(%ebp), %eax	/ &argv
#if	soon
	movl	(%eax), %ecx		/ argc
	inc	%ecx			/ argc+1
	subl	$4, %eax		/ &argv[-1]

parg:
	pushl	(%eax, %ecx, 4)
	loop	parg
#else
	push	0x10(%eax)		/ argv[4] = esym
	push	0x4(%eax)		/ argv[1] = howto
	push	0x1c(%eax)		/ argv[7] = cnvmem
	push	0x20(%eax)		/ argv[8] = extmem
	push	0x8(%eax)		/ argv[2] = bootdev
#endif	soon
	
	mov	0x8(%ebp), %ecx		/ entry offset 
	mov	$0x28, %ebx		/ segment
	push	%ebx
	push	%ecx

	/ set up %ds and %es
	mov	$0x20, %ebx
	movw	%bx, %ds
	movw	%bx, %es

	lret


/
/ pcpy(src, dst, cnt)
/	where src is a virtual address and dst is a physical address
/

ENTRY(pcpy)
	push	%ebp
	mov	%esp, %ebp
	push	%es
	push	%esi
	push	%edi
	push	%ecx

	cld

	/ set %es to point at the flat segment
	mov	$0x20, %eax
	movw	%ax , %es

	mov	0x8(%ebp), %esi		/ source
	mov	0xc(%ebp), %edi		/ destination
	mov	0x10(%ebp), %ecx	/ count

	rep
	movsb

	pop	%ecx
	pop	%edi
	pop	%esi
	pop	%es
	pop	%ebp

	ret

/
/ pzero(dst, cnt)
/	where src is a virtual address and dst is a physical address
/

ENTRY(pzero)
	push	%ebp
	mov	%esp, %ebp
	push	%es
	push	%edi
	push	%ecx

	cld

	/ set %es to point at the flat segment
	mov	$0x20, %eax
	movw	%ax , %es
	movb	$0x0, %al

	mov	0x8(%ebp), %edi		/ destination
	mov	0xc(%ebp), %ecx		/ count

	rep
	stosb	%al

	pop	%ecx
	pop	%edi
	pop	%es
	pop	%ebp

	ret

ENTRY(_sp)
	mov	%esp, %eax
	ret
