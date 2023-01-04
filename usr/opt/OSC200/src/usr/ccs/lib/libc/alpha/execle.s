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
 *	@(#)$RCSfile: execle.s,v $ $Revision: 1.2.6.3 $ (DEC) $Date: 1993/06/08 01:23:50 $
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
#include <syscall.h>

FRMSIZE	=	(5+1)*8		/* 5 "homed" arguments + 1 ra register */

NESTED(execle, FRMSIZE, ra)

	ldgp	gp, 0(pv)

	lda	sp, -FRMSIZE(sp)

	stq	ra, 0(sp)
	.mask		0x04000000, -FRMSIZE
	.prologue	1
	stq	a1, 8(sp)		/* home all arguments */
	stq	a2, 16(sp)
	stq	a3, 24(sp)
	stq	a4, 32(sp)
	stq	a5, 40(sp)

	/* Loop through looking for execve's 3 parameter */

	lda	a2, 8(sp)
1:
	ldq	v0, 0(a2)
	addq	a2, 8, a2
	bne	v0, 1b

	/* Now know execv's 2nd and 3rd parameter */

	lda	a1, 8(sp)
	ldq	a2, 0(a2)

#ifndef _NAME_SPACE_WEAK_STRONG
	jsr	ra, execve
#else
	jsr	ra, __execve
#endif
	ldgp	gp, 0(ra)

	ldq	ra, 0(sp)
	lda	sp, FRMSIZE(sp)
	RET		# execle(file, arg1, arg2, ..., 0, env);
END(execle)









