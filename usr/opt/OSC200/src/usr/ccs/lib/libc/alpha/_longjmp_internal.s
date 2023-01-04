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
	.asciiz "@(#)$RCSfile: _longjmp_internal.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 19:47:22 $"
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
 * C library -- _longjmp_internal. Uncondition signal state restore,
 * called from siglongjmp.
 *
 * This routine exists as a separate routine existing only in libc to ensure
 * that the versions of longjmp that reside in libc_r and libsys5 --
 * which do the wrong thing -- are not called.
 *
 * See the include file for the actual code.
 */

#define LJ_FRAME_SIZE	16

#ifndef _NAME_SPACE_WEAK_STRONG
NESTED(_longjmp_internal, LJ_FRAME_SIZE, ra)
#else
OLDNESTED(_longjmp_internal, LJ_FRAME_SIZE, ra)
#endif

/*
 * Include work routine for longjmp.
 */
#include "longjmp_incl.s"

END(_longjmp_internal)
