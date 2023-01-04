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
 * Crt0 for alpha standalone programs
 *
 */
#include <machine/regdef.h>
#include <machine/pal.h>

	.globl start
	.ent start
	.text
	.align 3
start:
	.set	noreorder
	bis	zero,zero,zero
	br	t0, 1f			# get addr of pad bytes preceding .quad
1:	ldgp	gp, 0(t0)		# load gp, making sure to skip pad bytes
	lda	sp,_stack		# init the stack
	ldiq	t0,8192
	addq	t0,sp,sp
	subq	sp,8,sp			# allocate space for the argument
	bis	a0,zero,a1		# called with 1 parameter
	bis	sp,zero,a0		# pass the address to main
	jsr	ra,main			# get to the main program

1:	call_pal PAL_halt
	br	ra,1b
	.end	start

	.lcomm	_stack,8192		# the stack
