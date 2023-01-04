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

#ifndef lint
_sccsid:.asciiz	"@(#)fork.c	5.2.M (Mach) 10-mar-89"
#endif not lint

#include <mips/regdef.h>
#include <mips/asm.h>
#include <syscall.h>

SYSCALL(fork)
	beq	v1,zero,parent
	subu	sp,20		# need some temp stack space
	sw	ra,20(sp)
	jal	mach_init
	lw	ra,20(sp)
	addu	sp,20		# restore sp..
	move	v0,zero
parent:
	RET		# pid = fork()
.end fork
