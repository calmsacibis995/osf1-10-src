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
 *	@(#)$RCSfile: assert.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:22:52 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

/*	assert.h	4.2	85/01/21	*/

/*
 * Handy assert macro.
 */

#ifndef	_KERN_ASSERT_H_
#define _KERN_ASSERT_H_

#include <mach_assert.h>

#ifdef	multimax
#include <mmax_debug.h>
#include <mmax_idebug.h>
/*
 * The ASSERT macro passes the string representation of the failing
 * assertion to assfail to be printed out.  The assert macro, used by
 * CMU, doesn't try to pass the string representation because CMU
 * frequently uses multi-line assertions without escaping the newlines,
 * which would cause preprocessor errors by placing newlines inside
 * string constants.
 */
#if	!lint && (MMAX_DEBUG || MACH_LDEBUG || MMAX_IDEBUG)
#define ASSERT(EX)	((EX) ? 0 : assfail("EX", __FILE__, __LINE__))
#define	ASSCMU(EX)	((EX) ? 0 : assfail("", __FILE__, __LINE__))
#else
#define ASSERT(x)
#define	ASSCMU(x)
#endif	/* !lint && (MMAX_DEBUG || MACH_LDEBUG || MMAX_IDEBUG) */
#if	MACH_ASSERT
#define	assert(ex)		((ex) ? 0 : assfail("", __FILE__, __LINE__))
#define	assert_static(ex)	ASSCMU(ex)
#else
#define	assert(ex)
#define	assert_static(ex)
#endif
#else	/* multimax */

#include <kern/macro_help.h>

#if	MACH_ASSERT
#define ASSERT(ex)	assert(ex)
#define assert(ex)							\
MACRO_BEGIN								\
	if (!(ex)) {							\
		printf("Assertion failed: file: \"%s\", line: %d test: %s\n", \
		       __FILE__, __LINE__, "ex");			\
		Debugger("assertion failure");				\
	}								\
MACRO_END

#ifdef	lint
#define assert_static(x)
#else
#define assert_static(x)	assert(x)
#endif

#else	/* MACH_ASSERT */
#define ASSERT(ex)
#define assert(ex)
#define assert_static(ex)
#endif	/* MACH_ASSERT */
#endif	/* multimax */

#endif	/* _KERN_ASSERT_H_ */
