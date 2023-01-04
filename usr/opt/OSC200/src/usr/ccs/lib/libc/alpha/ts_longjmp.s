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
	.asciiz "@(#)$RCSfile: ts_longjmp.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:48:20 $"
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
 * C library -- thread-safe longjmp (acts just like _longjmp!)
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
 * thread-safe longjmp: see include file for actual code.
 * This module should be used by libc_r only. libc uses _longjmp.s.
 */

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <setjmp.h>
#include <pdsc.h>

LEAF(__ts_longjump_resume)
	.eflag	PDSC_FLAGS_EXCEPTION_FRAME

/*
 * Include work routine for longjmp_resume.
 */
#include "ljresume_incl.s"

END(__ts_longjump_resume)

#define LJ_FRAME_SIZE	16
/*
 * longjmp(jmp_buf, retval)
 */
NESTED(longjmp, LJ_FRAME_SIZE, ra)

/*
 * include work routine for longjmp.
 */
#include "_longjmp_incl.s"

END(longjmp)
