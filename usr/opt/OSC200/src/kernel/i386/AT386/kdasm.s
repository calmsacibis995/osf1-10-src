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
 *	@(#)$RCSfile: kdasm.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:31 $
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
 * Some inline code to speed up major block copies to and from the
 * screen buffer.
 * 
 * Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989.
 *  All rights reserved.
 *
 * orc!eugene	28 Oct 1988
 *
 */


 
#include <i386/asm.h>

/*
 * Function:	kd_slmwd()
 *
 *	This function "slams" a word (char/attr) into the screen memory using
 *	a block fill operation on the 386.
 *
 */

#define start 0x08(%ebp)
#define count 0x0c(%ebp)
#define value 0x10(%ebp)

ENTRY(kd_slmwd)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi
	pushl	%ecx
	pushl	%eax
	pushl	start
	popl	%edi
	movl	count, %ecx
	movw	value, %ax
	rep
	stosw
	popl	%eax
	popl	%ecx
	popl	%edi

/* why twice %mem	start;	mem	count;	mem	value; */

	pushl	%edi
	pushl	%ecx
	pushl	%eax
	pushl	start
	popl	%edi
	movl	count, %ecx
	movw	value, %ax
	rep
	stosw
	popl	%eax
	popl	%ecx
	popl	%edi

	leave
	ret
#undef start
#undef count
#undef value

/*
 * "slam up"
 */

#define from  0x08(%ebp)
#define to    0x0c(%ebp)
#define count 0x10(%ebp)
ENTRY(kd_slmscu)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi
	pushl	%edi
	pushl	%ecx
	pushl	from
	pushl	to
	popl	%edi
	popl	%esi
	movl	count, %ecx
	cmpl	%edi, %esi
	rep
	movsw
	popl	%ecx
	popl	%edi
	popl	%esi

	leave
	ret

/*
 * "slam down"
 */
ENTRY(kd_slmscd)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi
	pushl	%edi
	pushl	%ecx
	pushl	from
	pushl	to
	popl	%edi
	popl	%esi
	movl	count, %ecx
	cmpl	%edi, %esi
	std
	rep
	movsw
	cld
	popl	%ecx
	popl	%edi
	popl	%esi

	leave
	ret
#undef from
#undef to
#undef count
