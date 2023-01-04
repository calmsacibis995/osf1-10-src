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
 *	@(#)$RCSfile: cerror.s,v $ $Revision: 1.2.7.3 $ (DEC) $Date: 1993/06/08 01:23:45 $
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

#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#define _USE__SETERRNO
#endif

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <syscall.h>

#ifdef _THREAD_SAFE
#ifndef _USE__SETERRNO
.globl	seterrno
#else
.globl	__seterrno
#endif
FRMSIZE	=	(1+1)*8		/* 1 "homed" arguments + 1 ra register */
#else
.extern	errno 4
#endif

#ifndef _THREAD_SAFE
LEAF(_cerror)			/* This is the target of a branch--not called
				   across a shared library boundary. */
#else
NESTED(_cerror,FRMSIZE,ra)
#endif
	br	t0, 1f
1:
	ldgp	gp, 0(t0)

#ifdef _THREAD_SAFE
	lda	sp, -FRMSIZE(sp)
	stq	ra, 0(sp)
	.prologue	1
        bis     v0, zero,a0    	/* move v0 (errno) into a0) */
#ifndef _USE__SETERRNO
	jsr	ra, seterrno
#else
	jsr	ra, __seterrno
#endif
	ldgp	gp, 0(ra)
	ldq	ra, 0(sp)
	lda	sp, FRMSIZE(sp)
#else
	.prologue	1
	stl	v0,errno
#endif
	ldiq	v0,-1
	RET
END(_cerror)
