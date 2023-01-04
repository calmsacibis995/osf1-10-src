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
	.asciiz "@(#)$RCSfile: _longjmp_incl.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:15 $"
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
 * C library -- _longjmp
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
 *   module that wants a version of longjmp that doesn't restore signal
 *   state.
 */

	# ldgp needed for +JBMAGIC and for 'botch'
	ldgp	gp, 0(pv)
	lda	sp, -LJ_FRAME_SIZE(sp)
	.mask	0x04000000, -LJ_FRAME_SIZE
	stq	ra, 0(sp)

	.prologue	1
	ldq	v0, 8*JB_MAGIC(a0)
	cmpeq	v0, +JBMAGIC, v0	# protect the naive
	beq	v0, botch
#if defined(_THREAD_SAFE)
	jsr	ra, __ts_longjump_resume
#elif defined(SVR4_LIB_SYS5)
	jsr	ra, __sys5_longjump_resume
#else
	jsr	ra, __longjump_resume
#endif /* defined(_THREAD_SAFE) */

botch:
	br	t0, 1f
1:
	ldgp	gp, 0(t0)		# gp needed for jsr to longjmperror
#ifndef _USE__LONGJMPERROR
	jsr	ra, longjmperror
#else
	jsr	ra, __longjmperror
#endif
        ldgp    gp, 0(ra)               # gp needed for jsr to abort
	jsr	ra, abort
	RET				# should not get here
