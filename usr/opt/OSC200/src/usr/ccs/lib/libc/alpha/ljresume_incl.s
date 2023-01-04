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
	.asciiz "@(#)$RCSfile: ljresume_incl.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:43 $"
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
 * NOTE:
 *   This file is not assembled directly, but is included by another
 *   module that need the longjmp_resume functionality. 
 */

	.eflag	PDSC_FLAGS_EXCEPTION_FRAME
	bis	a0, zero, a2		# for traceback
	ldq	ra, 8*JB_RA(a0)
	ldq	sp, 8*JB_SP(a0)
	ldq	gp, 8*JB_GP(a0)
	ldq	s0, 8*JB_S0(a0) 
	ldq	s1, 8*JB_S1(a0)
	ldq	s2, 8*JB_S2(a0)
	ldq	s3, 8*JB_S3(a0)
	ldq	s4, 8*JB_S4(a0)
	ldq	s5, 8*JB_S5(a0)
	ldq	s6, 8*JB_S6(a0)
	ldt	$f2, 8*JB_F2(a0)
	ldt	$f3, 8*JB_F3(a0)
	ldt	$f4, 8*JB_F4(a0)
	ldt	$f5, 8*JB_F5(a0)
	ldt	$f6, 8*JB_F6(a0)
	ldt	$f7, 8*JB_F7(a0)
	ldt	$f8, 8*JB_F8(a0)
	ldt	$f9, 8*JB_F9(a0)
	bis	zero, a1, v0		# return retval
	cmoveq	v0, 1, v0		# zero becomes 1
	RET
	.prologue	0
	bis	zero,zero,zero		#no-op
