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
 *	@(#)$RCSfile: syscall_sw.h,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/07/14 16:11:29 $
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

#ifndef	_KERN_SYSCALL_SW_H_
#define _KERN_SYSCALL_SW_H_

#ifndef	ASSEMBLER

typedef struct {
	short		mach_trap_length;
	short		mach_trap_flags;
	int		(*mach_trap_function)();
} mach_trap_t;

extern mach_trap_t	mach_trap_table[];
extern int		mach_trap_count;

#ifdef	__alpha
#define MACH_TRAP(name, arg_count, flags)	\
		{ sizeof(long) * ((arg_count)+1), (flags), (int (*)())(name) }
#else
#define MACH_TRAP(name, arg_count, flags)	\
		{ sizeof(int) * ((arg_count)+1), (flags), (name) }
#endif
#define FF1(name,argno,flags)	MACH_TRAP(name, argno, 1<<flags)
#define FN(name, argno)		MACH_TRAP(name, argno, 0)

#endif	/* ASSEMBLER */

/*
 *	Flag values (actually bit numbers)
 */

#define ASETJMP		0
#define APSIG		2

#endif	/* _KERN_SYSCALL_SW_H_ */
