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
	.asciiz "@(#)$RCSfile: setjmp_incl.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:48:06 $"
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
 *   module that wants a version of setjmp that saves signal state.
 *   Both setjmp and sigsetjmp can be built from this file. If
 *   __BUILD_SETJMP is defined, you get setjmp (always save signal
 *   state). Otherwise, you get sigsetjmp, which checks a1 to see
 *   whether the state should be saved.
 */

	ldgp	t0, 0(pv)
	subq	sp, SETJMPFRM, sp
	stq	gp, 16(sp)		# saving old gp
	bis	t0, zero, gp		# and readying gp for setjmp
	stq	ra, 0(sp)
	.mask		0x04000000, -SETJMPFRM
	.prologue	1
#ifdef	__BUILD_SETJMP
	br	zero, 1f		# jump ahead to save flag
#endif
	bne	a1, 1f			# test savemask argument
	ldq	a1, +JB_FLAGS*8(a0)	# clear savemask bit
	bic	a1, 1, a1
	stq	a1, +JB_FLAGS*8(a0)
	br	zero, 2f
1:	ldq	a1, +JB_FLAGS*8(a0)	# set savemask bit
	bis	a1, 1, a1
	stq	a1, +JB_FLAGS*8(a0)
	stq	a0, 8(sp)		# save jmp_buf ptr
	bis	zero, zero, a0
#ifndef _NAME_SPACE_WEAK_STRONG
	jsr	ra, sigblock		# find current sigmask
#else
	jsr	ra, __sigblock		# find current sigmask
#endif
	ldgp	gp, 0(ra)
	ldq	t1, 8(sp)
	stq	v0, +JB_SIGMASK*8(t1)
	bis	zero, zero, a0
	addq	sp, 24, a1
#ifndef _NAME_SPACE_WEAK_STRONG
	jsr	ra, sigstack
#else
	jsr	ra, __sigstack
#endif
	ldgp	gp, 0(ra)
	ldl	a1, 32(sp)		# load onstackk portion of sigstack
	ldq	a0, 8(sp)		# restore ptr to sigcontext
	stq	a1, +JB_ONSIGSTK*8(a0)	# store onstack
	ldq	ra, 0(sp)
	ldq	t0, 16(sp)
2:	addq	sp, SETJMPFRM, sp
	stq	ra, +JB_PC*8(a0)
	stq	ra, +JB_RA*8(a0)
	stq	t0, +JB_GP*8(a0)
	stq	sp, +JB_SP*8(a0)
	stq	s0, +JB_S0*8(a0)
	stq	s1, +JB_S1*8(a0)
	stq	s2, +JB_S2*8(a0)
	stq	s3, +JB_S3*8(a0)
	stq	s4, +JB_S4*8(a0)
	stq	s5, +JB_S5*8(a0)
	stq	s6, +JB_S6*8(a0)
	stt	$f2, +JB_F2*8(a0)
	stt	$f3, +JB_F3*8(a0)
	stt	$f4, +JB_F4*8(a0)
	stt	$f5, +JB_F5*8(a0)
	stt	$f6, +JB_F6*8(a0)
	stt	$f7, +JB_F7*8(a0)
	stt	$f8, +JB_F8*8(a0)
	stt	$f9, +JB_F9*8(a0)
	ldiq	v0, +JBMAGIC
	stq	v0, +JB_MAGIC*8(a0)
	bis	zero, zero, v0
	RET
END(sigsetjmp)
