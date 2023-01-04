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
	.asciiz "@(#)$RCSfile: longjmp.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:50 $"
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

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <setjmp.h>
#include <syscall.h>

/*
 * longjmp: see include file for actual code.
 * This module should be used by libc only. libc_r uses ts_longjmp.s.
 */

#define LJ_FRAME_SIZE	16

#ifndef _NAME_SPACE_WEAK_STRONG
NESTED(longjmp, LJ_FRAME_SIZE, ra)
#else
OLDNESTED(longjmp, LJ_FRAME_SIZE, ra)
#endif

/*
 * Include work routine for longjmp.
 */
#include "longjmp_incl.s"

END(longjmp)
