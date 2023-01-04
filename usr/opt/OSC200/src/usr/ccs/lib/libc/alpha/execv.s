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
 *	@(#)$RCSfile: execv.s,v $ $Revision: 1.2.6.4 $ (DEC) $Date: 1994/01/06 14:14:36 $
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

#ifndef _NAME_SPACE_WEAK_STRONG
	.globl	environ
#else
	.globl	__environ
#endif

FRMSIZE		= 16		/* ra saved and padding */

NESTED(execv, FRMSIZE, ra)
	ldgp	gp, 0(pv)
	lda	sp, -FRMSIZE(sp)

	stq	ra, 0(sp)
	.mask		0x04000000, -FRMSIZE
	.prologue	1
#ifndef _NAME_SPACE_WEAK_STRONG
	ldq	a2,environ
	jsr	ra, execve
#else
	ldq	a2,__environ
	jsr	ra, __execve
#endif
	ldgp	gp, 0(ra)

	ldq	ra,0(sp)

	lda	sp, FRMSIZE(sp)
	RET			# execv(file, argv)
END(execv)
