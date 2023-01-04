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
 *	@(#)$RCSfile: crt0.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:58:47 $
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
 * $ Header: crt0.s,v 10.3 89/07/24 12:16:38 mbj Exp $
 * C runtime startoff with optional monitoring
 */

#ifdef MCRT0
.file	"mcrt0.s"
#include "SYS.h"
#else
.file	"crt0.s"
#endif MCRT0

	.set	sigcatchall,0x400	/* Return signal handler plant */

.globl	modstart
.globl	modsize
.globl	start
.globl	_main
.globl	_exit
.globl	__exit
.globl	_environ
.globl	__auxv
.globl	__ldr_crt0_request
.globl	__ldr_present
.globl	_sigvec
.globl	sigentry
#ifdef MCRT0
.globl	_monstartup
.globl	_monitor
.globl	____Argv
#else
.globl	mcount; mcount: ret $0; .align 4	/* In case any -p .os */
#endif MCRT0

/*	C language startup routine */

modstart:
	.double	p_glbl,0,0xf00000,0
	.set	modsize,0x10

start:
	addr	@modstart,r0
	cmpd	r0,$0xffff
	bhi	.L1
	lprw	mod,r0		#  mod register
	addr	@p_glbl,r0
	lprd	sb,r0		#  sb register
.L1:
	adjspb	$12
	movd	12(sp),0(sp)	#  argc
	addr	16(sp),r0
	movd	r0,4(sp)	#  argv
#ifdef MCRT0
	movd	r0,@____Argv	#  prog name for profiling
#endif MCRT0
.L2:
	movd	r0,r1
	addqd	$4,r0
	cmpqd	$0,0(r1)	#  null args term ?
	bne	.L2
	movd	r0,8(sp)	#  env
	movd	@__pic__environ,r1 # note pic reference
	movd	r0,0(r1)
#	movd	r0,@_environ

.L3:
	movd	r0,r1		# Find and set up auxv by searching for end
	addqd	$4,r0		#   of environment variables.  Pass auxv
	cmpqd	$0,0(r1)	#   as fourth argument to main and store it
	bne	.L3		#   in _auxv.
	movd	r0,12(sp)
	movd	r0,@__auxv

#ifdef MCRT0
	addr	@_etext,tos
	addr	@_eprol,tos
	jsr	@_monstartup
	adjspb	$-8
#endif MCRT0

	movqd	$0,tos		# setup intermediate signal handler
	addr	@sv,tos
	movzwd	$sigcatchall,tos
	movd	@__pic__sigvec,r0
	jsr	0(r0)
#	jsr	@_sigvec
	adjspb	$-12
#if	MACH
	movd	@_mach_init_routine,r0	# is there
	cmpqd	$0,r0			#   a mach_init
	beq	.L4			#     routine ??
	jsr	0(r0)			# if yes, call it.
.L4:
	movd	@__pthread_init_routine,r0
	cmpqd	$0,r0
	beq	.L5
	jsr	0(r0)
.L5:
#endif	MACH
	jsr	@_main

	movd	@__ldr_present,r1	# If _ldr_present is set, then
	cmpqd	$0,r1			#   main() returns an entry point
	beq	.L6			#   in r0.  Restore argc to
	movd	0(sp),12(sp)		#   eventual tos and adjust sp back
	adjspb	$-12			#   to original value.  Then jump to
	jump	0(r0)			#   entry point which should never
					#   return.
.L6:
	adjspb	$-16
	movd	r0,tos
#if	MACH
	movd	@__pthread_exit_routine,r0
	cmpqd	$0,r0
	beq	.L7
	jsr	0(r0)
	br	.L8
.L7:
#endif	MACH
	movd	@__pic__exit,r1
	jsr	0(r1)
#	jsr	@_exit
#if	MACH
.L8:
#endif	MACH
	movd	@__pic___exit,r1
	jsr	0(r1)
#	jsr @__exit			# insurance

	.data
	.align	4		# sigvec arg
sv:
	.double	sigentry
	.double	0
	.double	0

	.comm	p_glbl,4
	.comm	__auxv,4
	.comm	__ldr_present,4
__pic__environ:
	.double	_environ
__pic__sigvec:
	.double	_sigvec
__pic__exit:
	.double	_exit
__pic___exit:
	.double	__exit
__ldr_crt0_request:
	.double	1
#if	MACH
	.comm	_mach_init_routine,4
	.comm	__pthread_init_routine,4
	.comm	__pthread_exit_routine,4
#endif	MACH

#ifdef MCRT0
	.text
__exit:
	movqd	$0,tos
	jsr	@_monitor
	adjspb	$-4
	br	@_real_exit

	PSEUDO(real_exit,exit)
#endif MCRT0
