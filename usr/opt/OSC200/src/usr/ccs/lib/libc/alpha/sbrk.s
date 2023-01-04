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
#undef SYS_brk	    /* remove old defintion */
#define SYS_brk SYS_obreak /* use obreak sys call for break */

	.globl	_end
	.globl	_minbrk
	.globl	_curbrk

/*.sdata*/.data
_minbrk:.quad	_end
_curbrk:.quad	_end

	.text

#ifdef _THREAD_SAFE
#ifndef _NAME_SPACE_WEAK_STRONG
LEAF(_unlocked_sbrk)
#else
OLDLEAF(_unlocked_sbrk)
#endif
#else
LEAF(sbrk)
#endif
	ldgp	gp, 0(pv)

	ldq	t0, _curbrk
	addq	a0, t0, a0
	ldiq	v0, SYS_brk
	CHMK()
	beq	a3, 1f
	jmp	zero, _cerror
1:
	ldq	v0,_curbrk
	stq	a0,_curbrk		# update to new curbrk
	RET
#ifdef _THREAD_SAFE
END(_unlocked_sbrk)
#else
END(sbrk)
#endif
