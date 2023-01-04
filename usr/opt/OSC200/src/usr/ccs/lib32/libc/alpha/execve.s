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
 * @(#)$RCSfile: execve.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/13 22:20:24 $
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

NESTED(execve, 16, ra)
	lda	sp, -16(sp)
	stq	fp, 0(sp)
	bis	sp, sp, fp

	bis	a1, a1, t1
10:	ldl	t0, 0(a1)
	addl	a1, 4, a1
	bne	t0, 10b
	subl	t1, a1, a1
	bis	a2, a2, t2
20:	ldl	t0, 0(a2)
	addl	a2, 4, a2
	bne	t0, 20b
	subl	t2, a2, a2

	addl	a1, a2, t0
	addl	t0, t0, t0
	addl	t0, 15, t0
	bic	t0, 15, t0
	subq	sp, t0, sp

	bis	sp, sp, t3
	bis	t3, t3, a1
30:	ldl	t0, 0(t1)
	addl	t1, 4, t1
	stq	t0, 0(t3)
	addq	t3, 8, t3
	bne	t0, 30b
	bis	t3, t3, a2
40:	ldl	t0, 0(t2)
	addl	t2, 4, t2
	stq	t0, 0(t3)
	addq	t3, 8, t3
	bne	t0, 40b

	ldiq	v0, SYS_execve
	CHMK()
	bne	a3, 100f

	bis	fp, fp, sp
	ldq	fp, 0(sp)
	lda	sp, 16(sp)
	RET

100:	br	zero, _cerror

END(execve)
