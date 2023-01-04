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
	.asciiz "@(#)$RCSfile: _longjmp.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:10 $"
	.text

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/_longjmp.s,v 1.1.2.2 1993/08/24 19:47:10 Jeff_Denham Exp $ */

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
/* we need to disable the alternate LEAF and NESTED macros from asm.h for
 * the name space protected symbols, yet allow a call to __longjmperror
 * if _NAME_SPACE_WEAK_STRONG was originally defined
 */
#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#define _USE__LONGJMPERROR
#endif

/*
 * See include files for actual working code.
 */

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <setjmp.h>
#include <pdsc.h>

/*
 * If this is being built for libc_r, don't create this entry point.
 */
#ifndef _THREAD_SAFE
LEAF(__longjump_resume)

/*
 * Include work file from __longjmp_resume.
 */
#include "ljresume_incl.s"

END(__longjump_resume)
#endif /* _THREAD_SAFE */


#define LJ_FRAME_SIZE	16

/*
 * _longjmp(jmp_buf, retval)
 */
NESTED(_longjmp, LJ_FRAME_SIZE, ra)

/*
 * Include work file from _longjmp.
 */
#include "_longjmp_incl.s"

END(_longjmp)
