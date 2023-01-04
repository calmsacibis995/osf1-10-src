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
 *	@(#)$RCSfile: sigaction.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:55:36 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */


/*
 *
 * 90/02/22 10:55 jd at osf
 *	Replaced SYS.h with machine/asm.h and syscall.h.
 *
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

/ Remember that with 4.3 semantics signals & longjumps are tied together!

/ The signal trampoline code is implemented by having sigvec pass to
/ the kernel the address of a signal cleanup routine (_sigreturn).
/ When delivering a signal the kernel sets up a stack frame that has
/ _sigreturn as the return address. The sigreturn system call takes
/ a pointer to a signal context as an argument and uses the contents
/ of the context structure to restore the processes state. 

#include	<machine/asm.h>
#include	<syscall.h>

#define SYS_sigaction 46

ENTRY(sigaction)
	movl	$46,%eax
	movl	$__sigreturn,%edx
/ we set the high order bit to signify that we're using the new signal
/ implementation (lance & tommy 16-mar-89)
/	orl	$0x80000000, %edx
	SVC
	jb 	EXT(cerror)
	ret

ENTRY(_sigreturn)
	addl	$12, %esp	/ remove args to user interrupt routine
/ the scpcopy field is now on top of the stack and is the argument to 
/ sigreturn (see machdep in kernel)
	call 	EXT(sigreturn)
	ret
