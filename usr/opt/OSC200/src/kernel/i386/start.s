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
 *	@(#)$RCSfile: start.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:56 $
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

#include <i386/asm.h>
#include <mach/vm_param.h>

/*
 *  Copyright 1988, 1989 by Intel Corporation
 *
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 */
	
	.globl	EXT(cnvmem)
	.globl	EXT(extmem)
	.globl	EXT(boottype)
	.globl	EXT(boothowto)
#ifdef PS2
	.globl	EXT(abios_info)
#endif /* PS2 */
#ifdef	wheeze
#else	wheeze
	.globl	EXT(esym)
#endif	wheeze
	.globl	EXT(pstart)
	.globl	EXT(gdt)
	.globl	EXT(idt)
	.globl	EXT(kpde)
	.globl	EXT(kpte)
	.globl	EXT(scall_dscr)
	.globl	EXT(sigret_dscr)

#ifdef PS2
/ PS2 requires higher limits because the abios support requires the ability to create
/ new descriptors on the fly to pass data addresses to abios.
	.set	GDTLIM, [8\*768-1]
	.set	IDTLIM, [8\*256-1]
	.set    GDTSZ,   768
#else
#ifdef	wheeze
	.set	GDTLIM, [8\*96-1]
	.set	IDTLIM, [8\*256-1]
#else	wheeze
	.set	GDTLIM, (Times(8,96)-1)
	.set	IDTLIM, (Times(8,256)-1)
#endif	wheeze
	.set    GDTSZ,   96
#endif PS2
	.set    IDTSZ,   256
	.set	PAGEBIT, 0x80000000
	.set	MASK,	 0x0FFFFFFF	/ mask the top nibble
	.set	KV,	 0xC0000000

	.text
DATA(intstack)
DATA(df_stack)
	.set .,. + KERNEL_STACK_SIZE

Entry(pstart)

#ifdef LOCORE_DEBUG
	mov	$'A',%ebx	/ output some A's
	call	output
#endif

	/ Retrieve the parameters passed from boot
	pop	%eax
	mov	$EXT(boottype), %ebx
	and	$MASK, %ebx
	mov	%eax, (%ebx)

	pop	%eax
	mov	$EXT(extmem), %ebx
	and	$MASK, %ebx
	mov	%eax, (%ebx)

	pop	%eax
	mov	$EXT(cnvmem), %ebx
	and	$MASK, %ebx
	mov	%eax, (%ebx)

#ifdef	OLD_BOOT
	xorl	%eax, %eax
	movb	$0, %al
#else	OLD_BOOT
	pop	%eax
#endif	OLD_BOOT
	mov	$EXT(boothowto), %ebx
	and	$MASK, %ebx
	mov	%eax, (%ebx)
#ifdef PS2
/ get the abios pointer and store into abios_info.
	pop	%eax
	mov	$EXT(abios_info), %ebx
	and	$MASK, %ebx
	mov	%eax, (%ebx)
#endif /* PS2 */

#ifdef	wheeze
#else	wheeze
/*	leal	EXT(end), %eax*/
	pop	%eax
	orl	$KV, %eax
	mov	$EXT(esym), %ebx
	and	$MASK, %ebx
	mov	%eax, (%ebx)
#endif	wheeze

	/ Turn off clock interrupt.
	/ This is due to a bug in p0init which enables interrupt
	/ before picinit.
	mov	$0x21, %edx		/ XXX - magic number
	INB
	orb	$1, %al			/ XXX - magic number
	OUTB

#ifdef LOCORE_DEBUG
	mov	$'B',%ebx
	call	output
#endif /* LOCORE_DEBUG */

	/ Rearrange GDT
	mov	$EXT(gdt), %eax
	and	$MASK, %eax

	mov	$GDTLIM, %ecx

	mov	$GDTdscr, %ebx
	and	$MASK, %ebx
	movw	%cx, (%ebx)

	call	munge_table

	/ Rearrange IDT
	mov	$EXT(idt), %eax
	and	$MASK, %eax

	mov	$IDTLIM, %ecx

	call	munge_table

	/ Rearrange call gate for system call (scall_dscr)
	mov	$EXT(scall_dscr), %eax
	and	$MASK, %eax

	mov	$1, %ecx

	call	munge_table

	/ Rearrange call gate for signal return  (sigret_dscr)
	mov	$EXT(sigret_dscr), %eax
	and	$MASK, %eax

	mov	$1, %ecx

	call	munge_table

#ifdef LOCORE_DEBUG
	mov	$'C',%ebx
	call	output
#endif /* LOCORE_DEBUG */

	/ Fix up the 1st, 3 giga and last entries in the page directory
	mov	$EXT(kpde), %ebx
	and	$MASK, %ebx

	mov	$EXT(kpte), %eax	
	and	$0xffff000, %eax
	or	$0x1, %eax

	mov	%eax, (%ebx)
	mov	%eax, 3072(%ebx)	/ 3 giga -- C0000000

	mov	$EXT(kpde), %edx
	and	$MASK, %edx

	/ Load IDTR
	mov	$IDTdscr, %eax
	and	$MASK, %eax

	lidt	(%eax)

#ifdef LOCORE_DEBUG
	mov	$'D',%ebx
	call	output
#endif /* LOCORE_DEBUG */

	/ Load GDTR
	mov	$GDTdscr, %eax
	and	$MASK, %eax

	lgdt	(%eax)

#ifdef LOCORE_DEBUG
	mov	$'E',%ebx
	call	output
#endif /* LOCORE_DEBUG */

	/ turn PG on
	mov	%cr0, %eax
	or	$PAGEBIT, %eax
	mov	%eax, %cr0

	mov	%edx, %cr3

#ifdef LOCORE_DEBUG
	mov	$'F',%ebx
	call	output
#endif /* LOCORE_DEBUG */

	ljmp	$KTSSSEL, $0x0

/ *********************************************************************
/	munge_table:
/		This procedure will 'munge' a descriptor table to
/		change it from initialized format to runtime format.
/		Assumes:
/			%eax -- contains the base address of table.
/			%ecx -- contains size of table.
/ *********************************************************************

munge_table:
	mov	%eax, %ebx
	add	%ebx, %ecx
moretable:
	cmp	%ebx, %ecx
	jl	donetable		/ Have we done every descriptor??
	movb	7(%ebx), %al	/ Find the byte containing the type field
	testb	$0x10, %al	/ See if this descriptor is a segment
	jne	notagate
	testb	$0x04, %al	/ See if this destriptor is a gate
	je	notagate
				/ Rearrange a gate descriptor.
	movw	6(%ebx), %eax	/ Type (etc.) lifted out
	movw	4(%ebx), %edx	/ Selector lifted out.
	movw	%eax, 4(%ebx)	/ Type (etc.) put back
	movw	2(%ebx), %eax	/ Grab Offset 16..31
	movw	%edx, 2(%ebx)	/ Put back Selector
	movw	%eax, 6(%ebx)	/ Offset 16..31 now in right place
	jmp	descdone

notagate:			/ Rearrange a non gate descriptor.
	movw	4(%ebx), %edx	/ Limit 0..15 lifted out
	movb	%al, 5(%ebx)	/ type (etc.) put back
	movw	2(%ebx), %eax	/ Grab Base 16..31
	movb	%al, 4(%ebx)	/ put back Base 16..23
	movb	%ah, 7(%ebx)	/ put back Base 24..32
	movw	(%ebx), %eax	/ Get Base 0..15
	movw	%eax, 2(%ebx)	/ Base 0..15 now in right place
	movw	%edx, (%ebx)	/ Limit 0..15 in its proper place

descdone:
	addl	$8, %ebx	/ Go for the next descriptor
	jmp	moretable

donetable:
	ret

	.align	8
GDTdscr:
	Value  GDTLIM
	.long	EXT(gdt)

	.align	8
IDTdscr:
	Value	IDTLIM
	.long	EXT(idt)

#ifdef LOCORE_DEBUG
/ output the character in %ebx to the screen
output:
/ put some characters on the screen to verify that boot worked
/ we output 2000 charactesr to b8000 and b0000 just to make sure
/ that we hit the screen.
	pusha
	mov	$0xb8000,%eax
	mov	$2000,%ecx	/ fill 25 * 80 bytes of screen 
1:	movb	%ebx,(%eax)
	movb	$7,1(%eax)
	add	$2,%eax
	loop	1b

	mov	$0xb0000,%eax
	mov	$2000,%ecx	/ fill 25 * 80 bytes of screen 
1:	movb	%ebx,(%eax)
	movb	$7,1(%eax)
	add	$2,%eax
	loop	1b

/ cause a 1 second delay
	mov	$1000000,%ecx
1:
	loop	1b
	popa
	ret

/ end of loop to put characters on the screen
#endif /* LOCORE_DEBUG */
