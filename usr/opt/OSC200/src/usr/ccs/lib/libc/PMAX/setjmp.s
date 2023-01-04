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
 *	@(#)$RCSfile: setjmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:09:50 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * setjmp.s
 *
 *	Revision History:
 *
 * 25-Mar-91	Kim Peterson
 *	Shared library reorganization
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

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

#ifdef	LANGUAGE_C
#undef	LANGUAGE_C
#endif
#define	LANGUAGE_ASSEMBLY

#include <mips/regdef.h>
#include <mips/asm.h>
#include <setjmp.h>
#include <syscall.h>
#include <mips/fpu.h>

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

SETJMPFRM	=	32
	.set	noreorder

/*
 *  D A N G E R - W i l l  R o b i n s o n ! !   D a n g e r ! !
 *
 *	Because GAS doesn't know how to instruction reorder, this code is
 *	written knowing where the stalls are/would be.  Be very
 *	careful about changing anything!
 */

NESTED(setjmp, SETJMPFRM, zero)
	sw	sp,+JB_SP*4(a0)		# save sp before it is changed
	subu	sp,SETJMPFRM
	sw	ra,SETJMPFRM-4(sp)
	sw	a0,SETJMPFRM-8(sp)	# save jmp_buf ptr
	jal	sigblock		# find current sigmask
	 move	a0,zero			# executed in shadow of jmp
	lw	v1,SETJMPFRM-8(sp)	# Get jmpbuf address
	 move	a0,zero			# Zero => no signal stack
	li	a1,1
	sw	a1,+JB_SM*4(v1)		# record that mask is saved
	sw	v0,+JB_SIGMASK*4(v1)	# Store sigmask
	jal	sigstack		# sigstack(0,oss)
	 addu	a1,v1,+JB_ONSIGSTK*4	#  Load &oss in shadow

	lw	a0,SETJMPFRM-8(sp)	# Get jmpbuf
	lw	ra,SETJMPFRM-4(sp)	# Recover return address
	bltz	v0,botch		# Syscall failed
	 addu	sp,SETJMPFRM		# Pop argument list in shadow

	sw	ra,+JB_PC*4(a0)		# Save all register state
	sw	gp,+JB_GP*4(a0)
	sw	s0,+JB_S0*4(a0)
	sw	s1,+JB_S1*4(a0)
	sw	s2,+JB_S2*4(a0)
	sw	s3,+JB_S3*4(a0)
	sw	s4,+JB_S4*4(a0)
	sw	s5,+JB_S5*4(a0)
	sw	s6,+JB_S6*4(a0)
	sw	s7,+JB_S7*4(a0)
	cfc1	v0,fpc_csr		# One cycle delay to read v0
	 sw	fp,+JB_FP*4(a0)
	sw	v0,+JB_FPC_CSR*4(a0)	# Okay to save FP CoProc State
	s.d	$f20,+JB_F20*4(a0)
	s.d	$f22,+JB_F22*4(a0)
	s.d	$f24,+JB_F24*4(a0)
	s.d	$f26,+JB_F26*4(a0)
	s.d	$f28,+JB_F28*4(a0)
	s.d	$f30,+JB_F30*4(a0)
	li	v0,+JBMAGIC		# No delays for li
	sw	v0,+JB_MAGIC*4(a0)
	j	ra
	 move	v0,zero			# setjmp MUST return zero

	.set	reorder
.end setjmp



LEAF(longjmp)
	.set	noreorder
	lw	s0,+JB_SP*4(a0)
	 lw	v0,+JB_MAGIC*4(a0)
	bgtu	sp,s0,botch		# jmp_buf no longer on stack
	 nop
	bne	a1,$0,nonzero		# Not allowed to return zero
	 nop
	li	a1,+1			# Force non-zero return
nonzero:
	sw	a1,+JB_V0*4(a0)		# let sigreturn set v0
	bne	v0,+JBMAGIC,botch	# protect the naive

	/*
	 * sigreturn will restore signal state and all callee saved
	 * registers from sigcontext and return to next instruction
	 * after setjmp call, the C calling sequence will then restore
	 * the caller saved registers
	 */
	li	v0,SYS_sigreturn	# sigreturn(&sigcontext)
	.set	reorder
	syscall

/*
 * Instruction reordering is turned back on in this code!
 */
botch:
	jal	longjmperror
	jal	abort


.end longjmp
