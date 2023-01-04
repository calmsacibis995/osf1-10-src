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
/* --------------------------------------------------- */
/* | Copyright (c) 1986 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University and Alessandro Forin
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Run-time start up for C executables
 */

#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <alpha/pal.h>
#include <syscall.h>

/* External definitions */

	.globl	environ
	.globl	__Argc
	.globl	__Argv
	.globl	_auxv
	.globl	_ldr_present
	.globl	mach_init_routine
	.globl	_pthread_init_routine
	.globl	_pthread_exit_routine
	.globl	__crt0_doing_init
	.comm	environ, 8
	.comm	__Argc, 4
	.comm	__Argv, 8
	.comm	_auxv, 8
	.comm	_ldr_present, 4
	.comm	mach_init_routine, 8
	.comm	_pthread_init_routine, 8
	.comm	_pthread_exit_routine, 8
	.comm	errno, 4

/*
 * __ldr_data is reserved for use by the dynamic loader.
 * It must be the first thing in the data section
 * and it must be large enough to hold a pointer.
 *
 * __crt0_doing_init is a flag use to indicate to external routines
 * when crt0 is through with all initialization.
 *
 */
        .data
        .globl  __ldr_data
__ldr_data:
        .quad   1
__crt0_doing_init:
	.long	1

/*
 * Stack as provided by kernel after an exec looks like:
 *	STRINGS		strings making up args, environment and aux vector
 *	auxp[n]
 *	...
 *	auxp[0]
 *	0
 *	envp[n]		points into STRINGS
 *	...
 *	envp[0]		points into STRINGS
 *	0
 *	argv[n]		points into STRINGS
 *	...
 *	argv[0]		points into STRINGS
 * sp-> argc
 */

	.text

STARTFRM = 16		# return address and padding to octaword align

	.globl	__start
	.ent	__start, 0
__start:
	lda	sp, -STARTFRM(sp)
	stq	zero, STARTFRM-8(sp)	# return PC of 0 flags last stack frame
	.mask	0x04000000, -8
	.frame	sp, STARTFRM, ra

	br	t0, 10f			# get the current PC
10:	ldgp	gp, 0(t0)		# init gp
	
	/* Get all the arguments to main() and save them away */

	ldl	a0, STARTFRM(sp)	# pick up argc
	lda	a1, STARTFRM+8(sp)	# argv is right after argc
#ifdef _64BIT
	s8addq	a0, a1, a2		# skip over the arguments
	addq	a2, 8, a2		# envp follows terminating 0 word
	bis	a2, a2, a3		# copy envp
20:	ldq	t0, 0(a3)		# get pointer to next env string
	addq	a3, 8, a3		# advance pointer to next
	bne	t0, 20b			# loop till we find env terminator
	stl	a0, __Argc
	stq	a1, __Argv
	stq	a2, environ
	stq	a3, _auxv
#endif /*64BIT*/
/*
 * If this is the 32 bit compatabilty crt0. Then the input
 * must be compressed in to an array of longs (4 bytes) from the array
 * of quads (8 Bytes) passed in by the kernel.
 */
#ifdef _32BIT
	stl	a0, __Argc
	stq	a1, __Argv
	bis	a1, a1, a2
	beq	a0, 13f
12:	ldq	t0, 0(a1)
	addq	a1, 8, a1
	stl	t0, 0(a2)
	addq	a2, 4, a2
	subq	a0, 1, a0
	bne	a0, 12b
13:	stq	a2, environ
14:	ldq	t0, 0(a1)
	addq	a1, 8, a1
	stl	t0, 0(a2)
	addq	a2, 4, a2
	bne	t0, 14b
	stq	a2, _auxv
#endif     /*_32BIT*/

	ldq	pv, mach_init_routine
	beq	pv, 30f
	jsr	ra, (pv)
	ldgp	gp, 0(ra)
30:

#ifdef	MCRT0
	/* If profiling version, start up profiler */

	lda	a0, eprol
	lda	a1, etext
	jsr	ra, monstartup
	ldgp	gp, 0(ra)
#endif	/* MCRT0 */

	ldq	pv, _pthread_init_routine
	beq	pv, 40f
	jsr	ra, (pv)
	ldgp	gp, 0(ra)
40:
	lda	pv, __istart		# __istart defined by ld or loader
	beq	pv, 50f
	jsr	ra, (pv)
	ldgp	gp, 0(ra)
50:
	stl	zero, __crt0_doing_init	# mark end of crt0 inits
	stl	zero, errno		# start with no errors

	/* Re-load the arguments and call the main program */

	ldl	a0, __Argc
	ldq	a1, __Argv
	ldq	a2, environ
	ldq	a3, _auxv
	jsr	ra, main
	ldgp	gp, 0(ra)

	/*
	 * If _ldr_present is set, then we've been linked with the
	 * dynamic loader and have special work to do.
	 */

	ldl	t0, _ldr_present
	bne	t0, 60f

	/*
	 * If we're not part of the dynamic loader then we run any
	 * thread exit routines and go away
	 */

	stq	v0, 0(sp)		# save the return code from main()

	ldq	pv, _pthread_exit_routine
	beq	pv, 50f
	jsr	ra, (pv)
	ldgp	gp, 0(ra)
50:
	ldq	a0, 0(sp)		# get back the return code
 	jsr	ra, exit		# exit with return code
 	call_pal PAL_halt		# shouldn't ever get here
	/*
	 * If we're part of the dynamic loader, then the main() we called
	 * earlier was actually the dynamic loader's main(). If we get
	 * here, the loader has successfully performed the run-time link
	 * and returned the real main program's entry point in v0.
	 */

60:	lda	sp, STARTFRM(sp)	# prune back the stack
	jmp	zero, (v0)		# jump to __start in the main program
END(__start)

#ifdef MCRT0
/*
 * Profiling version requires special _exit to dump profiling info
 */
EXITFRM=16	# temp word and pad to octaword alignment
	.align	4
NESTED(_exit, EXITFRM, zero)
	ldgp	gp, 0(pv)
	stq	a0, 0(sp)
	ldiq	a0, 0
	jsr	ra, monitor
	ldgp	gp, 0(ra)
	ldq	a0, 0(sp)
	ldiq	v0, SYS_exit
	call_pal PAL_callsys
	call_pal PAL_halt
END(_exit)

#endif /* MCRT0 */

#ifdef CRT0
/*
 * Null moncontrol and mcount routines in case something profiled is
 * accidentally linked with non-profiled code
 */
LEAF(moncontrol)
	RET
END(moncontrol)

 # dummy substitute for _mcount
 
 # _mcount gets called in a funny, nonstandard sequence, so even the
 # dummy routine must fix up the stack and restore the return address

.set noat
.align	4
.globl _mcount
.ent _mcount
_mcount:
	lda	sp, 16(sp)
	bis	AT, AT, ra
	ret	zero, (AT), 1
.end _mcount

#endif /* CRT0 */

/*--------------------------------------------------------------------------*/
/*
/* eprol MUST BE THE LAST THING IN THE TEXT SEGMENT FOR CRT0.  COBOL
/* DEPENDS ON THIS ENTRY POINT BEING LOCATED JUST BEFORE THE TEXT FROM
/* THE USERS PROGRAM.
/*
/* NO TEXT SHOULD BE ADDED AFTER eprol !!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*--------------------------------------------------------------------------*/

.align 4	# prof likes having the bb-countable text doubleword aligned
.globl eprol
.ent eprol
.set noreorder
eprol:	nop
.set reorder
.end eprol


