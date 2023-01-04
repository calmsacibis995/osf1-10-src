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
 *	@(#)$RCSfile: bios.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:42 $
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

	.file	"bios.s"

#include "small.h"

#include "machine/asm.h"
	.text

/ biosread(dev, cyl, head, sec)
/	Read one sector from disk into the internal buffer "intbuf" which
/	is the first 512 bytes of the boot loader.
/ BIOS call "INT 0x13 Function 0x2" to read sectors from disk into memory
/	Call with	%ah = 0x2
/			%al = number of sectors
/			%ch = cylinder
/			%cl = sector
/			%dh = head
/			%dl = drive (0x80 for hard disk, 0x0 for floppy disk)
/			%es:%bx = segment:offset of buffer
/	Return:		
/			%al = 0x0 on success; err code on failure

ENTRY(biosread)
	push	%ebp
	mov	%esp, %ebp

	push	%ebx
	push	%ecx
	push	%edx
	push	%es

	movb	0x10(%ebp), %dh	/ head

	movw	0x0c(%ebp), %cx
	xchgb	%ch, %cl	/ cylinder; the highest 2 bits of cyl is in %cl
	rorb	$2, %cl
	movb	0x14(%ebp), %al
	orb	%al, %cl
	incb	%cl		/ sector; sec starts from 1, not 0

	movb	0x8(%ebp), %dl	/ device

	xor	%ebx, %ebx	/ offset -- 0
				/ prot_to_real will set %es to BOOTSEG

	call	EXT(prot_to_real)	/ enter real mode

	movb	$0x2, %ah	/ subfunction

	movb	$0x1, %al	/ number of sectors -- one

	sti
	int	$0x13
	cli

	mov	%eax, %ebx	/ save return value

	data16
	call	EXT(real_to_prot) / back to protected mode

	xor	%eax, %eax
	movb	%bh, %al	/ return value in %ax

	pop	%es
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp

	ret


/ putc(ch)
/ BIOS call "INT 10H Function 0Eh" to write character to console
/	Call with	%ah = 0x0e
/			%al = character
/			%bh = page
/			%bl = foreground color ( graphics modes)


ENTRY(putc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx
	push	%ecx		/ save ECX

	movb	0x8(%ebp), %cl

	call	EXT(prot_to_real)

	data16
	mov	$0x1, %ebx	/ %bh=0, %bl=1 (blue)
	movb	$0xe, %ah
	movb	%cl, %al

	sti

	int	$0x10		/ display a byte

	cli

	data16
	call	EXT(real_to_prot)

	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret


/ getc()
/ BIOS call "INT 16H Function 00H" to read character from keyboard
/	Call with	%ah = 0x0
/	Return:		%ah = keyboard scan code
/			%al = ASCII character

ENTRY(getc)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx		/ save %ebx

	call	EXT(prot_to_real)

	movb	$0x0, %ah
	
	sti

	int	$0x16

	cli

	movb	%al, %bl	/ real_to_prot uses %eax

	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	movb	%bl, %al

	pop	%ebx
	pop	%ebp
	ret

#ifndef	SMALL
/ ischar()
/       if there is a character pending, return it; otherwise return 0
/ BIOS call "INT 16H Function 01H" to check whether a character is pending
/	Call with	%ah = 0x1
/	Return:
/		If key waiting to be input:
/			%ah = keyboard scan code
/			%al = ASCII character
/			Zero flag = clear
/		else
/			Zero flag = set

ENTRY(ischar)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx

	call	EXT(prot_to_real)		/ enter real mode

	xor	%ebx, %ebx

	movb	$0x1, %ah

	sti

	int	$0x16

	cli

	data16
	jz	nochar

	movb	%al, %bl

nochar:
	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	movb	%bl, %al

	pop	%ebx
	pop	%ebp
	ret
#endif	SMALL

/
/ get_diskinfo():  return a word that represents the
/	max number of sectors and  heads and drives for this device
/

ENTRY(get_diskinfo)
	push	%ebp
	mov	%esp, %ebp
	push	%es
	push	%ebx
	push	%ecx
	push	%edx

	movb	0x8(%ebp), %dl		/ diskinfo(drive #)
	call	EXT(prot_to_real)	/ enter real mode

	movb	$0x8, %ah		/ ask for disk info

	sti
	int	$0x13
	cli

	data16
	call	EXT(real_to_prot)	/ back to protected mode

	xor	%eax, %eax

/	form a longword representing all this gunk
	movb	%dh, %ah		/ # heads
	andb	$0x3f, %cl		/ mask of cylinder gunk
	movb	%cl, %al		/ # sectors

di_ret:
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%es
	pop	%ebp
	ret

/
/ memsize(i) :  return the memory size in KB. i == 0 for conventional memory,
/		i == 1 for extended memory
/	BIOS call "INT 12H" to get conventional memory size
/	BIOS call "INT 15H, AH=88H" to get extended memory size
/		Both have the return value in AX.
/

ENTRY(memsize)
	push	%ebp
	mov	%esp, %ebp
	push	%ebx

	mov	8(%ebp), %ebx

	call	EXT(prot_to_real)		/ enter real mode

	cmpb	$0x1, %bl

	data16
	je	xext
	
	sti

	int	$0x12

	cli

	data16
	jmp	xdone

xext:
	movb	$0x88, %ah

	sti

	int	$0x15

	cli

xdone:
	mov	%eax, %ebx

	data16
	call	EXT(real_to_prot)

	xor	%eax, %eax
	mov	%ebx, %eax

	pop	%ebx
	pop	%ebp
	ret
