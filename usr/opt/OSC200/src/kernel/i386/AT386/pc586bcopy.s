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
 *	@(#)$RCSfile: pc586bcopy.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:50 $
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

	.text

	.file	"pc586bcopy.s"
/ pc586bcopy(p_from, p_to, count)
/ long *p_from, *p_to, count; /* count is in bytes */
/ this routine is used to copy data on and off of pc586 board
/ the pc586 board MUST be accessed on 16 bit boundries only
/ see pc586 hardware ref man
/ this routine copies in 4 byte increments and rounds down 


	.text

ENTRY(pc586bcopy)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	push	%edi
	movl	0x08(%ebp),%esi
	movl	0x0c(%ebp),%edi
	movl	0x10(%ebp),%ecx
	shrl	$2,%ecx
	orl	%ecx,%ecx
	jz	copy_done
	cld
	rep 
	movsl
copy_done:
	popl	%edi
	popl	%esi
	popl	%ebp
	ret


