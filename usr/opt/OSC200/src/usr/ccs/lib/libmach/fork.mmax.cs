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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
_sccsid:.asciz	"@(#)fork.c	5.2.M (Mach) 19-aug-86"
#endif not lint

#include "SYS.mmax.h"

	.globl	_mach_init

SYSCALL(fork)
	cmpqd	$0,r1	# parent since r1 == 0 in parent, 1 in child
	beq	.Lparent
	jsr	@_mach_init
	movqd	$0,r0
.Lparent:
	ret	$0	# pid = fork()
