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
 *	@(#)$RCSfile: syscall_sw.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:33:24 $
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
 * syscall_sw.h
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */

/* 
 * derived from syscall_sw.h	2.1	(ULTRIX/OSF)	12/3/90
 */
#ifndef	_MACH_MIPS_SYSCALL_SW_H_
#define	_MACH_MIPS_SYSCALL_SW_H_	1

#include <machine/regdef.h>

/*
 * The Unix kernel expects arguments to be passed with the normal C calling
 * sequence, and v0 should contain the system call number.
 * On Mach we pass all the arguments in registers, the trap number is in v0
 * and the return value is placed in v0.  There are no awful hacks for
 * returning multiple values from a trap.
 *
 * Performance: a trap with up to 4 args takes 4 cycles in user mode,
 * with an unfortunate and unavoidable nop instruction and no memory
 * accesses. Any arg after the fourth takes 1 more cycle to load
 * from the cache (which cannot possibly miss) into a register.
 */

/*
 * A simple trap is one with up to 4 args. Args are passed to us
 * in registers, and we keep them there.
 */
#define simple_kernel_trap(trap_name, trap_number)	 \
	.globl	trap_name;	 			 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

#define kernel_trap_0(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_1(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_2(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_3(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_4(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

/*
 * A trap with more than 4 args requires popping of args
 * off the stack, where they are placed by the compiler.
 */
#define kernel_trap_5(trap_name, trap_number)	 	 \
	.globl	trap_name; 				 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	lw	t0,16(sp);				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

#define kernel_trap_6(trap_name, trap_number)	 	 \
	.globl	trap_name; 				 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	lw	t0,16(sp);				 \
	lw	t1,20(sp);				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

/*
 * There are no Mach traps with more than 6 args.
 * If that changes just add more macros, using
 * registers t2-t7.  And fix the kernel.
 */

#ifdef	__STDC__
#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_##nargs(trap_name,trap_number)
#else
#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_/**/nargs(trap_name,trap_number)
#endif	/* __STDC__ */

#endif	_MACH_MIPS_SYSCALL_SW_H_
