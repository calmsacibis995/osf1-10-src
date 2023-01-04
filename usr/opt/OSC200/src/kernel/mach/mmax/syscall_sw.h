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
 *	@(#)$RCSfile: syscall_sw.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:33:58 $
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

#ifndef	_MACH_MMAX_SYSCALL_SW_H_
#define _MACH_MMAX_SYSCALL_SW_H_

/*
 *	This code must convert between the user procedure call protocol
 *	(all parameters on stack) and the kernel procedure call protocol
 *	(first two parameters in registers).  Just to make life interesting,
 *	r0 is reserved for the trap code number.  In hopes of optimizing
 *	calls with few arguments we put the first two arguments into r1 and
 *	r2 (in that order); the ACALL handler in locore.s gets them from
 *	there.  The remaining arguments are to be found starting at 12(sp)
 *	since the rsb address and first two arguments must be skipped at
 *	4 bytes each.
 */

#define kernel_trap(trap_name, trap_number, number_args) \
	.globl	_/**/trap_name ;\
_/**/trap_name: \
	movd	8(sp),r2	;\
	movd	4(sp),r1	;\
	addr	trap_number,r0	;\
	svc	;\
	ret	$0

#endif	_MACH_MMAX_SYSCALL_SW_H_
