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
 *	@(#)$RCSfile: thread_status.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1992/03/18 17:49:29 $
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
 *	File:	mips/thread_status.h
 *
 *	This file contains the structure definitions for the thread
 *	state as applied to Mips processors.
 *
 */

#ifndef	_MACH_MIPS_THREAD_STATE_
#define	_MACH_MIPS_THREAD_STATE_

/*
 *	Two basic structures are defined:
 *
 *	mips_thread_state	this is the structure that is exported
 *				to user threads for use in status/mutate
 *				calls.  This structure should never
 *				change.
 *
 *	mips_saved_state	this structure corresponds to the state
 *				of the user registers as saved on the
 *				stack upon kernel entry.  This structure
 *				is used internally only.  Since this
 *				structure may change from version to
 *				version, it is hidden from the user.
 *
 * 	Other definitions (flavors) cover alternate register sets as
 *	needed.  For instance, for the FP coprocessor.
 */

#define	MIPS_THREAD_STATE	(1)
#define MIPS_FLOAT_STATE	(2)

struct mips_thread_state {
	int	r1;		/* at:  assembler temporary */
	int	r2;		/* v0:  return value 0 */
	int	r3;		/* v1:  return value 1 */
	int	r4;		/* a0:  argument 0 */
	int	r5;		/* a1:  argument 1 */
	int	r6;		/* a2:  argument 2 */
	int	r7;		/* a3:  argument 3 */
	int	r8;		/* t0:  caller saved 0 */
	int	r9;		/* t1:  caller saved 1 */
	int	r10;		/* t2: caller saved 2 */
	int	r11;		/* t3: caller saved 3 */
	int	r12;		/* t4: caller saved 4 */
	int	r13;		/* t5: caller saved 5 */
	int	r14;		/* t6: caller saved 6 */
	int	r15;		/* t7: caller saved 7 */
	int	r16;		/* s0: callee saved 0 */
	int	r17;		/* s1: callee saved 1 */
	int	r18;		/* s2: callee saved 2 */
	int	r19;		/* s3: callee saved 3 */
	int	r20;		/* s4: callee saved 4 */
	int	r21;		/* s5: callee saved 5 */
	int	r22;		/* s6: callee saved 6 */
	int	r23;		/* s7: callee saved 7 */
	int	r24;		/* t8: code generator 0 */
	int	r25;		/* t9: code generator 1 */
	int	r26;		/* k0: kernel temporary 0 */
	int	r27;		/* k1: kernel temporary 1 */
	int	r28;		/* gp: global pointer */
	int	r29;		/* sp: stack pointer */
	int	r30;		/* fp: frame pointer */
	int	r31;		/* ra: return address */
	int	mdlo;		/* low mult result */
	int	mdhi;		/* high mult result */
	int	pc;		/* user-mode PC */
};

#define	MIPS_THREAD_STATE_COUNT	(sizeof(struct mips_thread_state)/sizeof(int))


struct mips_float_state {
	int	r0;	/* general floating point registers */
	int	r1;
	int	r2;
	int	r3;
	int	r4;
	int	r5;
	int	r6;
	int	r7;
	int	r8;
	int	r9;
	int	r10;
	int	r11;
	int	r12;
	int	r13;
	int	r14;
	int	r15;
	int	r16;
	int	r17;
	int	r18;
	int	r19;
	int	r20;
	int	r21;
	int	r22;
	int	r23;
	int	r24;
	int	r25;
	int	r26;
	int	r27;
	int	r28;
	int	r29;
	int	r30;
	int	r31;
	int	csr;	/* floating point control and status reg */
	int	eir;	/* floating point exception instruction reg */
};

#define	MIPS_FLOAT_STATE_COUNT	(sizeof(struct mips_float_state)/sizeof(int))

#ifdef	KERNEL
struct mips_saved_state {
	int	arg0;		/* arg save for c calling seq */
	int	arg1;		/* arg save for c calling seq */
	int	arg2;		/* arg save for c calling seq */
	int	arg3;		/* arg save for c calling seq */
	int	r1;		/* at:  assembler temporary */
	int	r2;		/* v0:  return value 0 */
	int	r3;		/* v1:  return value 1 */
	int	r4;		/* a0:  argument 0 */
	int	r5;		/* a1:  argument 1 */
	int	r6;		/* a2:  argument 2 */
	int	r7;		/* a3:  argument 3 */
	int	r8;		/* t0:  caller saved 0 */
	int	r9;		/* t1:  caller saved 1 */
	int	r10;		/* t2: caller saved 2 */
	int	r11;		/* t3: caller saved 3 */
	int	r12;		/* t4: caller saved 4 */
	int	r13;		/* t5: caller saved 5 */
	int	r14;		/* t6: caller saved 6 */
	int	r15;		/* t7: caller saved 7 */
	int	r16;		/* s0: callee saved 0 */
	int	r17;		/* s1: callee saved 1 */
	int	r18;		/* s2: callee saved 2 */
	int	r19;		/* s3: callee saved 3 */
	int	r20;		/* s4: callee saved 4 */
	int	r21;		/* s5: callee saved 5 */
	int	r22;		/* s6: callee saved 6 */
	int	r23;		/* s7: callee saved 7 */
	int	r24;		/* t8: code generator 0 */
	int	r25;		/* t9: code generator 1 */
	int	r26;		/* k0: kernel temporary 0 */
	int	r27;		/* k1: kernel temporary 1 */
	int	r28;		/* gp: global pointer */
	int	r29;		/* sp: stack pointer */
	int	r30;		/* fp: frame pointer */
	int	r31;		/* ra: return address */
	int	sr;		/* status register */
	int	mdlo;		/* low mult result */
	int	mdhi;		/* high mult result */
	int	badvaddr;	/* bad virtual address */
	int	cause;		/* cause register */
	int	epc;		/* program counter */
	int	sys1;		/* system specific save 1 */
};

#include <machine/pcb.h>

#define	USER_REGS(_th)	((struct mips_saved_state *)	\
					((_th)->kernel_stack	\
					 + KERNEL_STACK_SIZE \
					 - sizeof(struct uthread) \
					 - sizeof(struct pcb )	\
					) - 1)
#define	USER_REG(x)		((unsigned int*)USER_REGS(current_thread()))[(x)]

#endif	/* KERNEL */

#endif	/* _MACH_MIPS_THREAD_STATE_ */
