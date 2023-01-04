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
 *	@(#)$RCSfile: asm.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:07:36 $
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
 * derived from asm.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/*  asm.h 2.1 */

/*
 * asm.h -- cpp definitions for assembler files
 */

#ifdef	_KERNEL
#include <mach_assert.h>
#if	MACH_ASSERT
#define ASSERTIONS
#endif
#endif

/*
 * Notes on putting entry pt and frame info into symbol table for debuggers
 *
 *	.ent	name,lex-level	# name is entry pt, lex-level is 0 for c
 * name:			# actual entry point
 *	.frame	fp,framesize,saved_pc_reg
 *				# fp -- register which is pointer to base
 *				#	of previous frame, debuggers are special
 *				#	cased if "sp" to add "framesize"
 *				#	(sp is usually used)
 *				# framesize -- size of frame
 *				#	the expression:
 *				#		new_sp + framesize == old_sp
 *				#	should be true
 *				# saved_pc_reg -- either a register which
 *				#	contains callers pc or $0, if $0
 *				#	saved pc is assumed to be in
 *				#	(fp)+framesize-4
 *
 * Notes regarding multiple entry points: Use "LEAF" for the first (main)
 * entry point and "XLEAF" for alternate (additional) entry points. The
 * "XLEAF"s must be nested within a "LEAF" and a ".end".
 * LEAF(copyseg)		# declare main entry point
 * XLEAF(copypage)		# declare alternate entry point
 */
/*
 * LEAF -- declare leaf routine
 */
#define	LEAF(x)						\
	.globl	x;					\
	.ent	x,0;					\
x:;							\
	.frame	sp,0,ra

/*
 * XLEAF -- declare alternate entry to leaf routine
 */
#define	XLEAF(x)					\
	.globl	x;					\
	.aent	x,0;					\
x:

/*
 * VECTOR -- declare exception routine entry
 */
#define	VECTOR(x, regmask)				\
	.globl	x;					\
	.ent	x,0;					\
x:;							\
	.frame	sp,EF_SIZE,$0;				\
	.mask	regmask,-(EF_SIZE-(EF_RA*4))


/*
 * NESTED -- declare nested routine entry point
 */
#define	NESTED(x, fsize, rpc)				\
	.globl	x;					\
	.ent	x,0;					\
x:;							\
	.frame	sp,fsize, rpc

/*
 * XNESTED -- declare alternate entry point to nested routine
 */
#define	XNESTED(x)					\
	.globl	x;					\
	.aent	x,0;					\
x:

/*
 * END -- mark end of procedure
 */
#define	END(proc)					\
	.end	proc

/*
 * IMPORT -- import external symbol
 */
#define	IMPORT(sym, size)				\
	.extern	sym,size

/*
 * ABS -- declare absolute symbol
 */
#define	ABS(x, y)					\
	.globl	x;					\
x	=	y

/*
 * EXPORT -- export definition of symbol
 */
#define	EXPORT(x)					\
	.globl	x;					\
x:

/*
 * BSS -- allocate space in bss
 */
#define	BSS(x,y)		\
	.comm	x,y

/*
 * LBSS -- allocate static space in bss
 */
#define	LBSS(x,y)		\
	.lcomm	x,y

/*
 * SYSCALL -- standard system call sequence
 * The kernel expects arguments to be passed with the normal C calling
 * sequence.  v0 should contain the system call number.  On return from
 * the kernel mode, a3 will be 0 to indicate no error and non-zero to
 * indicate an error; if an error occurred v0 will contain an errno.
 */
#ifndef __STDC__
#ifdef	HABITAT_INDEX
#define	SYSCALL(x)					\
LEAF(x);						\
	li      t6, HABITAT_NSTD_CALL(x);		\
	li	t5, HABITAT_BASE;			\
	sub     t7, t6, t5;				\
	li	t5, HABITAT_INDEX;			\
	or	v0, t7, t5;				\
	syscall;					\
	beq	a3,zero,9f;				\
	j	_cerror;				\
9:
#else
#define	SYSCALL(x)					\
LEAF(x);						\
	li	v0,SYS_/**/x;				\
	syscall;					\
	beq	a3,zero,9f;				\
	j	_cerror;				\
9:
#endif	/* HABITAT_INDEX */

#else
#ifdef	HABITAT_INDEX
#define	SYSCALL(x)					\
LEAF(x);						\
	li      t6, HABITAT_STD_CALL(x);		\
	li	t5, HABITAT_BASE;			\
	sub     t7, t6, t5;				\
	li	t5, HABITAT_INDEX;			\
	or	v0, t7, t5;				\
	syscall;					\
	beq	a3,zero,9f;				\
	j	_cerror;				\
9:
#else
#define	SYSCALL(x)					\
LEAF(x);						\
	li	v0,SYS_ ## x;				\
	syscall;					\
	beq	a3,zero,9f;				\
	j	_cerror;				\
9:
#endif	/* HABITAT_INDEX */
#endif	/* __STDC__ */

/*
 * PSEUDO -- system call sequence for syscalls that are variations of other
 * system calls
 */
#ifndef __STDC__
#define	PSEUDO(x,y)					\
LEAF(x);						\
	li	v0,SYS_/**/y;				\
	syscall

#else
#define	PSEUDO(x,y)					\
LEAF(x);						\
	li	v0,SYS_ ## y;				\
	syscall

#endif

#define	CALL(y)						\
	jal	y

#define	RET						\
	j	ra

/*
 * The following macros reserve the usage of the local label '9'
 */
#define	PANIC(msg)					\
	sw	zero,waittime;				\
	la	a0,9f;					\
	jal	panic;					\
	MSGX(msg)

#define	PRINTF(msg)					\
	la	a0,9f;					\
	jal	printf;					\
	MSGX(msg)

#define	MSGX(msg)					\
	.rdata;						\
9:	.asciiz	msg;					\
	.text

/*
 * register mask bit definitions
 */
#define	M_EXCFRM	0x00000001
#define	M_AT		0x00000002
#define	M_V0		0x00000004
#define	M_V1		0x00000008
#define	M_A0		0x00000010
#define	M_A1		0x00000020
#define	M_A2		0x00000040
#define	M_A3		0x00000080
#define	M_T0		0x00000100
#define	M_T1		0x00000200
#define	M_T2		0x00000400
#define	M_T3		0x00000800
#define	M_T4		0x00001000
#define	M_T5		0x00002000
#define	M_T6		0x00004000
#define	M_T7		0x00008000
#define	M_S0		0x00010000
#define	M_S1		0x00020000
#define	M_S2		0x00040000
#define	M_S3		0x00080000
#define	M_S4		0x00100000
#define	M_S5		0x00200000
#define	M_S6		0x00400000
#define	M_S7		0x00800000
#define	M_T8		0x01000000
#define	M_T9		0x02000000
#define	M_K0		0x04000000
#define	M_K1		0x08000000
#define	M_GP		0x10000000
#define	M_SP		0x20000000
#define	M_FP		0x40000000
#define	M_RA		0x80000000
