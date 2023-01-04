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
	.rdata
	.asciiz "@(#)$RCSfile: sigjmp.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:48:13 $"
	.text

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <setjmp.h>
#include <syscall.h>

/*
 * C library -- sigsetjmp, siglongjmp
 *
 *	siglongjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	sigsetjmp(a, m)
 * by restoring registers from the stack,
 * previous signal mask (if SAVEMASK = TRUE), and doing a return.
 *
 * NOTE: THIS MUST MATCH UP WITH SIGCONTEXT STRUCTURE, be sure constants
 * in setjmp.h and signal.h are consistent!
 * 
 * Whats happening here: sigsetjmp assumes that all process state except
 * the callee saved registers and the gp has been preserved by the
 * C calling sequence; therefore, sigsetjmp only saves the signal state
 * (sigmask and the signal flag), and the state that must be preserved
 * by the callee (callee saved regs, gp, sp, ra, callee save fp regs
 * and fpc_csr)  into a sigcontext struct. If sigsetjmp is called with
 * SAVEMASK argument of 0, it behaves like _setjmp, i.e., no signal
 * state is restored. This is accomplished by a Boolean in the jmp buf.
 *
 * On a siglongjmp, if the buffer is marked for a signal-mask restore
 * the jmp_buf is verified to be consistent, the appropriate
 * return value is dropped into the sigcontext, and a sigreturn system
 * call is performed to restore the signal state and restore the
 * callee saved register that were saved in the sigcontext by setjmp.
 * Otherwise, a _longjmp-style restore is done.
 */

SETJMPFRM	=	64

NESTED(sigsetjmp, SETJMPFRM, ra)

/*
 * Include work routine for sigsetjmp.
 */
#include "setjmp_incl.s"

END(sigsetjmp)
