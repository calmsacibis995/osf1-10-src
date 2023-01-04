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
	.asciiz "@(#)$RCSfile: _setjmp_incl.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:31 $"
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
 * C library -- _setjmp
 *
 *	_longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	_setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 */

/*
 * NOTE:
 *   This file is not assembled directly, but is included by another
 *   module that wants a version of setjmp that doesn't save signal state.
 */

	ldgp	t0, 0(pv)
	stq	ra, 8*JB_PC(a0)
	stq	ra, 8*JB_RA(a0)
	stq	gp, 8*JB_GP(a0)
	bis	t0, zero, gp		# and readying gp to get JB_MAGIC
	stq	sp, 8*JB_SP(a0)
	stq	s0, 8*JB_S0(a0)
	stq	s1, 8*JB_S1(a0)
	stq	s2, 8*JB_S2(a0)
	stq	s3, 8*JB_S3(a0)
	stq	s4, 8*JB_S4(a0)
	stq	s5, 8*JB_S5(a0)
	stq	s6, 8*JB_S6(a0)
	stt	$f2, 8*JB_F2(a0)
	stt	$f3, 8*JB_F3(a0)
	stt	$f4, 8*JB_F4(a0)
	stt	$f5, 8*JB_F5(a0)
	stt	$f6, 8*JB_F6(a0)
	stt	$f7, 8*JB_F7(a0)
	stt	$f8, 8*JB_F8(a0)
	stt	$f9, 8*JB_F9(a0)
	ldiq	v0, +JBMAGIC
	stq	v0, 8*JB_MAGIC(a0)
	bis	zero, zero, v0
	RET
