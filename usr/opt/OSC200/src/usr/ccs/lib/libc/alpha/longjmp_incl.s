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
	.asciiz "@(#)$RCSfile: longjmp_incl.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:55 $"
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

/*
 * C library -- setjmp, longjmp
 *
 *	longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	setjmp(a)
 * by restoring registers from the stack,
 * previous signal mask, and doing a return.
 *
 * NOTE: THIS MUST MATCH UP WITH SIGCONTEXT STRUCTURE, be sure constants
 * in setjmp.h and signal.h are consistent!
 * 
 * Whats happening here: setjmp assumes that all process state except
 * the callee saved registers and the gp has been preserved by the
 * C calling sequence; therefore, setjmp only saves the signal state
 * (sigmask and the signal flag), and the state that must be preserved
 * by the callee (callee saved regs, gp, sp, ra, callee save fp regs
 * and fpc_csr)  into a sigcontext struct.
 *
 * On a longjmp, the jmp_buf is verified to be consistent, the appropriate
 * return value is dropped into the sigcontext, and a sigreturn system
 * call is performed to restore the signal state and restore the
 * callee saved register that were saved in the sigcontext by setjmp.
 */

/*
 * NOTE:
 *   This file is not assembled directly, but is included by another
 *   module that wants a version of longjmp that restores signal state.
 */

	ldgp	gp, 0(pv)
	lda	sp, -LJ_FRAME_SIZE(sp)		# create stack frame
	stq	ra, 0(sp)			# so botch's can traceback
	.mask		0x04000000, -LJ_FRAME_SIZE
	.prologue	1

	/*
	 * This test breaks when jumping off an alternate signal stack.
	 * So, it's simply not safe to make this test any more.
	 *
	 * ldq	s0, +JB_SP*8(a0)
	 * cmpult	s0, sp, v0
	 * bne	v0, botch		# jmp_buf no longer on stack
	 */
	ldq	v0, +JB_MAGIC*8(a0)
	cmpeq	v0, +JBMAGIC, v0
	beq	v0, botch		# protect the naive
	cmoveq	a1, 1, a1		# zero becomes 1
	stq	a1, +JB_V0*8(a0)	# let sigreturn set v0
	/*
	 * sigreturn will restore signal state and all callee saved
	 * registers from sigcontext and return to next instruction
	 * after setjmp call, the C calling sequence will then restore
	 * the caller saved registers
	 */
	ldiq	v0, SYS_sigreturn	# sigreturn(&sigcontext)
	CHMK()

botch:
#ifndef _NAME_SPACE_WEAK_STRONG
	jsr	ra, longjmperror
#else
	jsr	ra, __longjmperror
#endif
	ldgp	gp, 0(ra)
	jsr	ra, abort
	RET				# Should never reach here
