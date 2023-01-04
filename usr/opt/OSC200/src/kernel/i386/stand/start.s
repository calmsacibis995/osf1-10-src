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
 *	@(#)$RCSfile: start.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:15:28 $
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

#include	"floppy.h"
#include	"machine/asm.h"

#define	DEBUG	1

	.file	"start.s"

BOOTSEG		=	0x100		/ boot will be loaded at 4k 
BOOTSTACK	=	0xf000		/ boot stack

#ifndef	FLOPPY	
/ partition table in sector 0
PARTSTART	=	0x1be	/ starting address of partition table
NUMPART		=	4	/ number of partitions in partition table
PARTSZ		=	16	/ each partition table entry is 16 bytes
BOOT_IND	=	0	/ offset of boot indicator in partition table
BEG_HEAD	=	1	/ offset of beginning head
BEG_SEC		= 	2	/ offset of beginning sector
BEG_CYL		=	3	/ offset of beginning cylinder
BOOTABLE	=	0x80	/ value of boot_ind, means bootable partition

LOADSZ		=	29	/ size of unix boot
#endif

SIGNATURE	=	0xaa55

	.text	


ENTRY(boot1)
/ ENTRY(intbuf)

	/ boot1 is loaded at 0x0:0x7c00
	/ ljmp to the next instruction to set up %cs
	data16
	ljmp $0x7c0, $start
/ x
start:
	/ set up %ds
	mov	%cs, %ax
	mov	%ax, %ds

	/ set up %ss and %esp
	data16
	mov	$BOOTSEG, %eax
	mov	%ax, %ss
	data16
	mov	$BOOTSTACK, %esp

	/set up %es
	mov	%ax, %es

	/ load the boot loader (boot1+boot2) into $BOOTSEG:0
	data16
	call	LCL(loadboot)

	/ ljmp to the second stage boot loader (boot2).
	/ After ljmp, %cs is BOOTSEG and boot1 (512 bytes) will be used
	/ as an internal buffer "intbuf".

	addr16
	data16
	movl	bootdev, %edx

/	data16
/	ljmp	$BOOTSEG, $EXT(boot2)
	data16
	.byte	0xea
	.long	EXT(boot2)
	.word	BOOTSEG


LCL(loadboot):

#ifdef	FLOPPY
	/ read sector 0 into BOOTSEG:0 by using BIOS call (INT 13H 02H)
	/	%ah=0x2		%al=0x1
	/	%ch=0x0		%cl=0x1
	/	%dh=0x0		%dl=0x0
	/	%es=BOOTSEG	%bx=0

	/ floppy disk, load the first two tracks which contain the boot code.
	/ load the first track

	data16
	call	getinfo		/ get drive info

	data16
	call	countsecs	/ figure how many sectors are really there

/	xor	%edx, %edx	/ %dl = 0, %dh = 0
	.byte	0x33
	.byte	0xd2

	data16
	mov	$0x1, %ecx	/ %cl = 1, %ch = 0 
				/ sector starts with 1 not 0

	addr16
	movb	spt, %al
	movb	$0x2, %ah

	xor	%ebx, %ebx	/ %bx = 0

	int	$0x13

	data16
	jb	read_error

	/ load the second track
	data16
	mov	$0x100, %edx	/ drive = 0, head = 1

	data16
	mov	$0x1, %ecx	/ sector = 1, cylinder = 0 

	movb	$0x2, %ah

	xor	%ebx, %ebx	/ load the second track into subsequent memory
	addr16
	movb	spt, %bl
	sal	$0x9, %bx

	int	$0x13

	data16
	jb	read_error

	data16
	ret


bootdrive: .byte 0		/ drive id of boot device
spt:	.word	0		/ sectors/track (one-based)
tpc:	.word	0		/ tracks/cylinder (zero-based)

getinfo:
	push	%ax
	push	%bx
	push	%cx
	push	%dx
	push	%es

/	reset the disk system
	movb	$0x0, %ah
	movb	$0x0, %dl
	int	$0x13

/	get the boot drive id
	movb	$0x33, %ah
	movb	$0x05, %al
	int	$0x21
	addr16
	movb	%dl, bootdrive

/ get some drive parameters
	movb	$0x8, %ah
/ still have boot drive id
	int	$0x13
	data16
	jnb	ok
/	trouble
	movb	$0x1, %dh		/ assume 2 heads, 15 trk/sec
	movb	$0xf, %cl
	movb	$0x2, %bl		/ => type 2

ok:

/ get max # tpc
	addr16
	movb	%dh, tpc
/ get max # of spt
	andb	$0x03f, %cl
	addr16
	movb	%cl, spt

#if	DEBUG
	movb	%cl, %al
	data16
	call	outhex
#endif	DEBUG

#if	DEBUG
	addr16
	movb	%bl, %al
	data16
	call	outhex
#endif	DEBUG

/	make a minor # out of the drive type
	xor	%ecx,%ecx
	movb	%bl, %cl			/ get the drive type
	data16
	mov	$fltype, %eax

	addr16
	movl	bootdev, %edx
	addr16					/ index into minor # table
	movb 	(%eax,%ecx,1), %dh		/ move into the partition spot
	addr16
	movl	%edx, bootdev

	pop	%es
	pop	%dx
	pop	%cx
	pop	%bx
	pop	%ax

	data16
	ret

fltype:
	.byte	0x0
	.byte	0x09		/ 5.25" 360kb
	.byte	0x03		/ 5.25" 1.2M
	.byte	0x01		/ 3.5" 720kb
	.byte	0x01		/ 3.5" 1.44M
	.byte	0x00


/ countsecs: count the real number of sectors on this piece of media
/	( 18spt max disks can be formatted at 9spt, and other
/	such horrors)

countsecs:
	push	%bx
	push	%cx
	push	%dx
	push	%es
	push	%di

	push	%ax

#if	DEBUG
	movb	$0x0a, %al
	data16
	call	outhex
#endif	DEBUG	

/	start with the max
	addr16
	movb	spt, %cl

ver:
	push	%cx

/	loop through until you successfully verify a cylinder
	movb	$0x04, %ah	/ verify sector
	movb	$0x01, %al
	movb	$0x00, %ch	/ cylinder (sector is in cl)

	xor	%edx, %edx
	addr16
	movb	bootdrive, %dl
	int	$0x13

	pop	%cx
	data16
	jnb	sec_ok
	data16
	loop	ver

/	couldn't find anything
	data16
	jmp	read_error

sec_ok:
	addr16
	movb	%cl, spt

#if	DEBUG
	movb	%cl, %al
	data16
	call	outhex
#endif	DEBUG

	pop	%ax
	pop	%di
	pop	%es
	pop	%dx
	pop	%cx
	pop	%bx

	data16
	ret

#else


	/ read sector 0 into BOOTSEG:0 by using BIOS call (INT 13H 02H)
	/	%ah=0x2		%al=0x1
	/	%ch=0x0		%cl=0x1
	/	%dh=0x0		%dl=0x80
	/	%es=BOOTSEG	%bx=0

	xor	%ebx, %ebx	/ %bx = 0

	data16
	mov	$0x201, %eax

	data16
	mov	$0x1, %ecx

	data16
	mov	$0x80, %edx

	int	$0x13

	data16
	jb	read_error


	/ find the bootable partition
	data16
	mov	$PARTSTART, %ebx

	data16
	mov	$NUMPART, %ecx

again:
	addr16
	movb    %es:BOOT_IND(%ebx), %al		/ get boot indicator
	cmpb	$BOOTABLE, %al			/ bootable or not?
	data16
	je	found

	data16
	add	$PARTSZ, %ebx

	data16
	loop	again

	data16
	mov	$enoboot, %esi

	data16
	jmp	boot_exit

found:

	/ BIOS call "INT 0x13 Function 0x2" to read sectors
	/	%ah = 0x2
	/	%al = number of sectors
	/	%ch = cylinder
	/	%cl = sector
	/	%dh = head
	/	%dl = drive (0x80 for hard disk, 0x0 for floppy disk)
	/	%es:%bx = segment:offset of buffer

	movb	$0x80, %dl	/ hard disk

	addr16
	movb	%es:BEG_HEAD(%ebx), %dh

	addr16
	movb	%es:BEG_SEC(%ebx), %cl

	addr16
	movb	%es:BEG_CYL(%ebx), %ch

	movb	$0x2, %ah

	movb	$LOADSZ, %al

	xor	%ebx, %ebx	/ %bx = 0

	int	$0x13

	data16
	jb	read_error

	data16
	ret
#endif

/
/	outhex: print a hex digit
/	%al: digit to be printed

outhex:
	push	%ax

	sar	$0x4, %al	/ top nibble
	data16
	call	outnib

	pop	%ax		/ low nibble (outnib masks)
	data16
	call	outnib

	data16
	ret

outnib:
	push	%ax
	push	%bx
	push	%dx

	andb	$0xf, %al
	cmpb	$0xa, %al
	data16
	jb	oh_digit

	data16
	add	$0x61-0xa, %al
	data16
	jmp	oh_print

oh_digit:
	data16
	add	$0x30, %al

oh_print:
	movb	%al, %dl
	data16
	mov	$0x1, %ebx	/ %bh=0, %bl=1 (blue)
	movb	$0xe, %ah
	int	$0x10

	pop	%dx
	pop	%bx
	pop	%ax
	data16
	ret
	

/
/	message: write the error message in %ds:%esi to console
/

message:
	push	%ax
	push	%bx

	/ Use BIOS "int 10H Function 0Eh" to write character in teletype mode
	/	%ah = 0xe	%al = character
	/	%bh = page	%bl = foreground color ( graphics modes)

	data16
	mov	$0x1, %ebx	/ %bh=0, %bl=1 (blue)
	cld

nextb:
	lodsb			/ load a byte into %al

	cmpb	$0x0, %al
	data16
	je	done

	movb	$0xe, %ah

	int	$0x10		/ display a byte

	data16
	jmp	nextb

done:
	pop	%bx
	pop	%ax
	data16
	ret			


bootpart:	.long	0

#ifdef	FLOPPY
bootdev:	.long	0x0001		/ slice<<8 | device
#else
bootdev:	.long	0x0000
#endif	FLOPPY


/
/	read_error
/

read_error:

	data16
	mov	$eread, %esi

/
/	boot_exit: write error message and halt
/

boot_exit:
	data16
	call	message				/ display error message

	hlt


/
/	error messages

eread:	String		"boot1: Read error\r\n\0"
enoboot: String		"boot1: No bootable partition\r\n\0"

/ the last 2 bytes in the sector 0 contain the signature
	. = EXT(boot1) + 0x1fe
	.value	SIGNATURE

