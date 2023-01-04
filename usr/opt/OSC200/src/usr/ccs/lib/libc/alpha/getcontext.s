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
	.asciiz "@(#)$RCSfile: getcontext.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/30 21:37:53 $"
	.text

GTCTXFRM	=	32

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <setjmp.h>

NESTED(getcontext, GTCTXFRM, zero)
	ldgp	t0, 0(pv)
	stq	sp, (a0)		#save stack pointer at front of
					# ucontext before it changes
	lda	sp, -GTCTXFRM(sp)
	stq	gp, 16(sp)
	stq	a0, 8(sp)
	stq	ra, 0(sp)
	bis	t0, zero, gp		# gp needed for jsr
	jsr	ra, _getucontext
	ldgp	t0, 0(ra)
	ldq	ra, 0(sp)
	ldq	a0, 8(sp)
	ldq	gp, 16(sp)
	lda	sp, GTCTXFRM(sp)
	beq	v0, botch

	bis	v0, v0, a1
	stq	ra, 8*JB_PC(a1)
	stq	ra, 8*JB_RA(a1)
	stq	gp, 8*JB_GP(a1)
	bis	t0, zero, gp		# gp needed to get JB_MAGIC
	stq	sp, 8*JB_SP(a1)
	stq	s0, 8*JB_S0(a1)
	stq	s1, 8*JB_S1(a1)
	stq	s2, 8*JB_S2(a1)
	stq	s3, 8*JB_S3(a1)
	stq	s4, 8*JB_S4(a1)
	stq	s5, 8*JB_S5(a1)
	stq	s6, 8*JB_S6(a1)
	stt	$f2, 8*JB_F2(a1)
	stt	$f3, 8*JB_F3(a1)
	stt	$f4, 8*JB_F4(a1)
	stt	$f5, 8*JB_F5(a1)
	stt	$f6, 8*JB_F6(a1)
	stt	$f7, 8*JB_F7(a1)
	stt	$f8, 8*JB_F8(a1)
	stt	$f9, 8*JB_F9(a1)

	ldiq	v0, +JBMAGIC
	stq	v0, 8*JB_MAGIC(a1)
	bis	zero, zero, v0
	RET
botch:
	ldiq	v0, -1
	br	t0, 1f
1:
	ldgp	gp, 0(t0)		# GP needed for jsr
#ifndef _NAME_SPACE_WEAK_STRONG
	jsr	ra, longjmperror
#else
	jsr	ra, __longjmperror
#endif
	ldgp	gp, 0(ra)		# GP needed for jsr
	jsr	ra, abort
	RET				# should not get here
END(getcontext)
