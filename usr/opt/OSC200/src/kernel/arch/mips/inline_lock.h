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
 *	@(#)$RCSfile: inline_lock.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:15:09 $
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
/* 
 * derived from inline_lock.h	2.1	(ULTRIX/OSF)	12/3/90
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	For RT_PREEMPT, we have our own lock macros.
 *
 */

/*
 * Define inline functions to be used with compilers that can benefit
 * from this. For the PMAX we use the simple default locks.
 */
#if	__GNUC__
#if	(MACH_LDEBUG || RT_PREEMPT)
/*
 * When debugging simple locks on the PMAX we use the standard
 * functions in lock.c. Otherwise we can use the inlined functions.
 * By defining _NO_INLINE_LOCKS here we tell lock.h to not declare the standard
 * inline functions. This causes us to use the functions in lock.c.
 */
#define _NO_INLINE_LOCKS 1
#endif	/* MACH_LDEBUG */
#endif	/* __GNUC__ */
