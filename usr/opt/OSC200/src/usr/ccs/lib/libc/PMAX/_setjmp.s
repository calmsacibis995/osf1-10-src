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
 *	@(#)$RCSfile: _setjmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:09:40 $
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
 * _setjmp.s
 *
 *	Revision History:
 *
 * 28-Apr-91	Fred Canter
 *	Undo LANGUAGE_C hack.
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

/*
 * C library -- _setjmp, _longjmp
 *
 *	_longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	_setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 */

#include <mips/regdef.h>
#include <mips/asm.h>
#include <setjmp.h>
#include <mips/fpu.h>

LEAF(_setjmp)

/*
 * DANGER - instruction reordering is OFF.  All branches and load delays
 *		must be manually filled here!!!
 */
	.set	noreorder
	sw	ra,4*JB_PC(a0)
	sw	sp,4*JB_SP(a0)
	sw	s0,4*JB_S0(a0)
	sw	s1,4*JB_S1(a0)
	sw	s2,4*JB_S2(a0)
	sw	s3,4*JB_S3(a0)
	sw	s4,4*JB_S4(a0)
	sw	s5,4*JB_S5(a0)
	sw	s6,4*JB_S6(a0)
	sw	s7,4*JB_S7(a0)
	sw	zero,4*JB_SM(a0)
	cfc1	v0,fpc_csr		# 1 cycle delay here
	 sw	fp,4*JB_FP(a0)
	sw	v0,+JB_FPC_CSR*4(a0)	# Now store
	s.d	$f20,+JB_F20*4(a0)
	s.d	$f22,+JB_F22*4(a0)
	s.d	$f24,+JB_F24*4(a0)
	s.d	$f26,+JB_F26*4(a0)
	s.d	$f28,+JB_F28*4(a0)
	s.d	$f30,+JB_F30*4(a0)
	li	v0,+JBMAGIC
	sw	v0,+JB_MAGIC*4(a0)
	j	ra
	 move	v0,zero
	.set	reorder

.end _setjmp

/*
 * _longjmp(jmp_buf, retval)
 */
LEAF(_longjmp)
	.set	noreorder
	bne	a1,zero,nonzero		# longjmp MUST not return 0
	 lw	v0,+JB_MAGIC*4(a0)	# Get jmpbuf magic cookie
	li	a1,+1			# So change 0 to non-zero
nonzero:
	bne	v0,+JBMAGIC,botch	# protect the naive
	 nop				# Do not fill this shadow
					#  it makes debugging easier
	lw	ra,4*JB_PC(a0)		# Restore setjmp return address
	lw	sp,4*JB_SP(a0)
	lw	s0,4*JB_S0(a0)
	lw	s1,4*JB_S1(a0)
	lw	s2,4*JB_S2(a0)
	lw	s3,4*JB_S3(a0)
	lw	s4,4*JB_S4(a0)
	lw	s5,4*JB_S5(a0)
	lw	s6,4*JB_S6(a0)
	lw	s7,4*JB_S7(a0)
	lw	fp,4*JB_FP(a0)
	l.d	$f20,+JB_F20*4(a0)
	l.d	$f22,+JB_F22*4(a0)
	l.d	$f24,+JB_F24*4(a0)
	l.d	$f26,+JB_F26*4(a0)
	l.d	$f28,+JB_F28*4(a0)
	lw	v0,+JB_FPC_CSR*4(a0)	# Get FP Coproc status
	l.d	$f30,+JB_F30*4(a0)
	ctc1	v0,fpc_csr		# Wait to restore
	 j	ra
	 move	v0,a1			# Load in branch shadow
	.set	reorder

/*
 * Instruction reordering back on now, it's safe to breathe
 */
botch:
	jal	longjmperror
	jal	abort
.end _longjmp
