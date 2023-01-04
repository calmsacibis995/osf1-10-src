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
	.asciiz "@(#)$RCSfile: longjmp.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:49:07 $"
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
#include <pdsc.h>

/*
 * Sys 5 library -- longjmp
 *
 *	longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 */

/*
 * longjmp: see include file in libc/alpha for actual code.
 */

LEAF(__sys5_longjump_resume)

/*
 * Include work file from __longjmp_resume.
 */
#include "ljresume_incl.s"

END(__sys5_longjump_resume)

#define LJ_FRAME_SIZE	16
/*
 * longjmp(jmp_buf, retval)
 */
NESTED(longjmp, LJ_FRAME_SIZE, ra)

/*
 * Include work file from _longjmp from libc.
 */
#include "_longjmp_incl.s"

END(longjmp)

