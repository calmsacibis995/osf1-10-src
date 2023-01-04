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
 *	@(#)$RCSfile: timer_macros.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:33 $
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


#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

/*
 *	Timer macros for mmax
 */

/*
 *	Traps:
 *
 *	TIME_TRAP_ENTRY  -- general timing for any trap entry
 *	TIME_TRAP_ENTRY_I -- above, but assume interrupts disabled
 *	TIME_TRAP_UENTRY  -- optimized for trap from user mode
 *	TIME_TRAP_UENTRY_I -- above, but assume interrupts disabled
 *	TIME_TRAP_EXIT -- general timing for any trap exit
 *	TIME_TRAP_UEXIT -- optimized for trap exit to user mode
 */

	.globl	_time_trap_uentry
	
#define TIME_TRAP_ENTRY(label)						\
	tbitw	$PSR_U_BIT,10(fp);	/* only change timer */		\
	bfc	label;			/* if from user mode */		\
	DISINT;				/* disable interrupts */	\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uentry;	/* change timer */		\
	ENBINT;								\
label:

#define TIME_TRAP_ENTRY_I(label)					\
	tbitw	$PSR_U_BIT,10(fp);	/* only change timer */		\
	bfc	label;			/* if from user mode */		\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uentry;	/* change timer */		\
label:

#define TIME_TRAP_UENTRY						\
	DISINT;				/* disable interrupts */	\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uentry;	/* change timer */		\
	ENBINT;

#define TIME_TRAP_UENTRY_I						\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uentry;	/* change timer */

	.globl	_time_trap_uexit
	
#if	MMAX_XPC || MMAX_APC

#define TIME_TRAP_EXIT(label)						\
	tbitw	$PSR_U_BIT,10(fp);	/* only change timer */		\
	bfc	label;			/* if to user mode */		\
	DISINT;				/* disable interrupts */	\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uexit;	/* change timer */		\
	ENBINT;								\
label:

#define TIME_TRAP_UEXIT							\
	DISINT;				/* disable interrupts */	\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uexit;	/* change timer */		\
	ENBINT;

#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC

#define TIME_TRAP_EXIT(label)						\
	tbitw	$PSR_U_BIT,10(fp);	/* only change timer */		\
	bfc	label;			/* if to user mode */		\
	DISINT;				/* disable interrupts */	\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uexit;	/* change timer */		\
label:

#define TIME_TRAP_UEXIT							\
	DISINT;				/* disable interrupts */	\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	jsr	_time_trap_uexit;	/* change timer */

#endif	MMAX_DPC

/*
 *	On this machine, timers are 4 words (doubles in assembler) long so
 *	it is necessary to shift a cpuid 2 bits to the left to get a
 *	timer offset in doubles
 */
#define TIMER_D_SHIFT	2

/*
 *	Interrupts are always locked out by any interrupt on Multimax
 */
	.globl	_time_int_entry

#define TIME_INT_ENTRY(new_timer)					\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	GETCPUID(r1);			/* calculate address */		\
	lshd	$TIMER_D_SHIFT,r1;	/*  of new timer */		\
	addr	new_timer[r1:d],r1;	/* as second arg */		\
	jsr	_time_int_entry;	/* do the timing */		\
	movd	r0,tos			/* returns old timer, push it */

	.globl	_time_int_exit

#define TIME_INT_EXIT							\
	movd	@SCCREG_FRCNT,r0;	/* timestamp */			\
	movd	tos,r1;			/* old timer from entry */	\
	jsr	_time_int_exit;		/* go back to it */
