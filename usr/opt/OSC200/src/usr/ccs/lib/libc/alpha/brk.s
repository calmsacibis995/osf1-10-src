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
#undef SYS_sbrk	    /* remove old defintion */
#define SYS_sbrk 17 /* use obreak sys call for break */

        .globl  _curbrk 8
        .globl  _minbrk 8

#ifndef _NAME_SPACE_WEAK_STRONG
#ifdef _THREAD_SAFE
LEAF(_unlocked_brk)
#else
LEAF(_brk)
#endif
#else
#ifdef _THREAD_SAFE
OLDLEAF(_unlocked_brk)
#else
OLDLEAF(_brk)
#endif
#endif
	ldgp	gp, 0(pv)
        br      zero, 1f
#ifdef _THREAD_SAFE
END(_unlocked_brk)
#else
END(_brk)
#endif

#ifdef _THREAD_SAFE
LEAF(unlocked_brk)
#else
LEAF(brk)
#endif
	ldgp	gp, 0(pv)

        ldq     v0, _minbrk
        cmpult  a0, v0, t0
        cmovne  t0, v0, a0
1:
        ldiq    v0,SYS_sbrk
	CHMK()
        bne     a3,err
        stq     a0,_curbrk
        bis	zero,zero,v0
        RET

err:
        br      gp, 2f;
2:
        ldgp    gp, 0(gp);
	jmp	zero,_cerror;
#ifdef _THREAD_SAFE
.end unlocked_brk
#else
.end brk
#endif

