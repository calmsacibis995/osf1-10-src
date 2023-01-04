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
 *	@(#)$RCSfile: alloca.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:45:49 $
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
 * alloca(size) - allocate size bytes on the stack, and return the
 *                value of the starting address of this space (aligned
 *                on a 4-byte boundary)
 *
 * This version of alloca works with the current C compiler.  If a
 * procedure writes into any of registers %ebx, %edi, or %esi, the
 * compiler arranges for their values to be pushed on the stack on
 * procedure entry and generates corresponding pop instructions to
 * be executed just before returning.  This strategy, of course, fails
 * if the stack pointer is moved by the procedure (by calling alloca,
 * for example).  This version of alloca allocates the required space
 * on the stack, but then copies the three values that were on the top
 * of the stack to the current top-of-stack, so that the pop
 * instructions (if any) work correctly.
 *
 * 1/26/89 - Thomas A. Joseph - Olivetti Research Center, Menlo Park, CA
 *
 * The copyright notice for the original alloca follows.
 *
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS

#endif LIBC_SCCS

#include <machine/asm.h>

ENTRY(alloca)
	popl	%ecx		/ Save program counter
	popl	%eax		/ Get size
	movl	%esp,%edx	/ Save current stack pointer
	subl	%eax,%esp	/ Allocate space on stack
	andl	$0xfffffffc,%esp /Align on 4-byte boundary
	leal	(%esp),%eax	/ Store return value
	pushl	8(%edx)		/ Copy three values from old
	pushl	4(%edx)		/   top-of-stack to new
	pushl	(%edx)		/   top-of-stack
	pushl	%eax		/ Push argument back on stack
				/   (will be pop'ed in caller)
	jmp	*%ecx		/ Jump back to calling routine
