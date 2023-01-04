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
 *	@(#)$RCSfile: ast.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/20 14:00:23 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	kern/ast.h: Definitions for Asynchronous System Traps.
 *
 *	Revision History:
 *
 * 6-Apr-91	Ron Widyono
 *	Implement Kernel mode ASTs for preemption points.  Per-processor
 *	flag ast_mode[] indicates whether an AST request is User mode or 
 *	Kernel mode.  Conditionalized under RT_PREEMPT.
 *
 */

#ifndef	_KERN_AST_H_
#define _KERN_AST_H_

/*
 *	There are two types of AST's:
 *		1.  This thread must context switch [call thread_block()]
 *		2.  This thread must do something bizarre
 *			[call thread_halt_self()]
 *
 *	Type 2 ASTs are kept in a field in the thread which encodes the
 *	bizarre thing the thread must do.
 *
 *	The need_ast array (per processor) records whether ASTs are needed
 *	for each processor.  For now each processor only has access to its
 *	own cell in that array.  [May change when we record which 
 *	processor each thread is executing on.]
 *
 *	need_ast is initialized from the thread's ast field at context
 *	switch.  Type 1 ASTs are entered directly into the field
 *	by aston().  The actual values in need_ast do not matter, 
 *	an ast is required if it is non-zero.
 */

#include <cpus.h>
#include <hw_ast.h>
#include <rt_preempt.h>

#include <machine/cpu.h>

/*
 *	Bits for reasons
 */

#define	AST_ZILCH	0x0
#define AST_HALT	0x1
#define AST_TERMINATE	0x2
/*#define AST_PROFILE	0x4  For future use */

/*
 *	Machines with hardware support (e.g. vax) turn on HW_AST option.
 *	This causes all type 1 ast support to be pulled in from machine/ast.h.
 */

#if	HW_AST
#include <machine/ast.h>
#else	/* HW_AST */


union {
	long	test_ast_lwc;

	struct {
#ifdef __alpha
		int lwc_pending;
		int need_ast;
#else
		short lwc_pending;
		short need_ast;
#endif
	} flags;

} kernel_async_trap[NCPUS];


#if	RT_PREEMPT
extern int	ast_mode[NCPUS];
#endif

/*
 *	Type 1 ASTs
 */
#define	aston()	kernel_async_trap[cpu_number()].flags.need_ast = 1;

#define astoff() kernel_async_trap[cpu_number()].flags.need_ast = 0;

/*
 *      AST types
 */
#define USER_AST        0
#define KERNEL_AST      1

#endif	/* HW_AST */
/*
 *	Type 2 ASTs
 */
#define	thread_ast_set(thread, reason)	(thread)->ast |= (reason)
#define thread_ast_clear(thread, reason)	(thread)->ast &= ~(reason)
#define thread_ast_clear_all(thread)	(thread)->ast = AST_ZILCH

/*
 *	NOTE: if thread is the current thread, thread_ast_set should
 *	be followed by aston() 
 */

#if	HW_AST
/*
 *	machine/ast.h must define versions of these macros.
 */
#else	/* HW_AST */
/*
 *	Macros to propagate thread asts to need_ast at context switch and
 *	clock interrupts.  (Context switch replaces old ast requests,
 *	clock interrupt reflects new requests from thread to need_ast.
 *
 *	NOTE: cpu is always the current cpu.  It is in these macros
 *	solely to avoid recalculating it on machines where that may
 *	be expensive.
 */

#define	ast_context(thread, cpu)	kernel_async_trap[cpu_number()].flags.need_ast = (thread)->ast
#define	ast_propagate(thread, cpu)	kernel_async_trap[cpu_number()].flags.need_ast |= (thread)->ast
#define	ast_needed(cpu)		kernel_async_trap[cpu_number()].flags.need_ast	
#endif	/* HW_AST */

#endif	/* _KERN_AST_H_ */
