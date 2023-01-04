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
 *	@(#)$RCSfile: mmax_ptrace.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:10 $
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
/*

 *
 */

#ifdef	KERNEL
#include <sysV/aouthdr.h>
#else	KERNEL
#include <aouthdr.h>
#endif	KERNEL

/*
 * Location of the users' stored
 * registers relative to R0.
 * Usage is ptrace_user.pt_ar0[XX].
 */

#define PT_R0_OFFSET	(8)
#define PT_AR0	(4)
#define PT_R0	(0)
#define PT_R1	(-1)
#define PT_R2	(-2)
#define PT_R3	(-3)
#define PT_R4	(-4)
#define PT_R5	(-5)
#define PT_R6	(-6)
#define PT_R7	(-7)

#define PT_FSR	(5)
#define PT_F0	(6)
#define PT_F1	(7)
#define PT_F2	(8)
#define PT_F3	(9)
#define PT_F4	(10)
#define PT_F5	(11)
#define PT_F6	(12)
#define PT_F7	(13)

#define PT_FP	(1)
#define PT_RFP	PT_FP
#define PT_SP	(-8)
#define PT_RSP	PT_SP
#define PT_PS	(3)
#define PT_RPS	PT_PS
#define PT_RPSRMOD	PT_RPS
#define PT_PC	(2)
#define PT_RPC	PT_PC
#define PT_MAXNAMLEN	255
#define PT_MAXARG	10

/* offsets for remote cdb as it interfaces to dbmon */
/* these are where the sb, usp, mmu regs begin */
/* These values are after ptrace_user, so PT_MAX is the 1st extra reg */
#define PT_MAX_OFFSET	532
#define PT_MAX		120

/*
 * Logical per process user structure that can be looked at by ptrace.
 * Nothing is actually stored here. It is only used to map ptrace requests
 * and to define the structure that goes at the beginning of core dumps.
 */
 
struct	ptrace_user {
	int	pt_sp;
	int	pt_r7;
	int	pt_r6;
	int	pt_r5;
	int	pt_r4;
	int	pt_r3;
	int	pt_r2;
	int	pt_r1;
	int	pt_r0;
	int	pt_fp;
	int	pt_pc;
	int	pt_psl;

	int	pt_dataptr;

	int	pt_fsr;	/* floating point status register */
	float	pt_f0;	/* floating point registers */
	float	pt_f1;
	float	pt_f2;
	float	pt_f3;
	float	pt_f4;
	float	pt_f5;
	float	pt_f6;
	float	pt_f7;

	char	pt_comm[PT_MAXNAMLEN + 1];	/* command name */
	int	pt_arg[PT_MAXARG];	/* arguments to current system call */

/*
	The address range fields  below have  the following interpretation:

	pt_dsize	= equivalent to the sum of aouthdr.dsize + 
			  aouthdr.bsize + (size of malloc'd storage) with 
			  aouthdr.dsize and aouthdr.bsize as defined in
			  aouthdr.h.
	pt_ssize	= size (in bytes) of the stack.  Stack start is
			  assumed to be USRSTACK - ssize.
 */

	long	pt_dsize;		/* data size (bytes) 		*/
	long	pt_ssize;		/* stack size (bytes) 		*/
	struct aouthdr pt_aouthdr;	/* the a.out header		*/
	long	pt_signal;		/* reason why he died		*/
	long	pt_usrstack;		/* top of stack address		*/
	long	pt_rev;			/* revision number (=2)		*/
	long	pt_subcode;		/* signal subcode		*/

	double	pt_d0;			/* the 8 64-bit FP regs.	*/
	double	pt_d1;			/* all 8 are real on the 32532.	*/
	double	pt_d2;			/* only d0,d2,d4,d6 are real    */
	double	pt_d3;			/* ... on the 32032 and 32332.	*/
	double	pt_d4;			/* d0 corresponds to (f0|f1),   */
	double	pt_d5;			/* ... d2 corresponds with 	*/
	double	pt_d6;			/* ... (f2|f3), etc.		*/
	double	pt_d7;			/* d1,d3,d5,d7 are always zero  */
					/* ... on the 32032 and 32332.  */
	long	pt_nmem;		/* Number of extra data regions	*/
	long	pt_memptr;		/* Offset of pt_mem_desc structs*/
};

struct pt_mem_desc {
	char   *pt_mem_va;		/* User address of region	*/
	int	pt_mem_size;		/* Size of region		*/
};

/*
	The ptrace_user structure is found at the beginning  of core files.
	The layout of a Multimax core file is as follows:


	+-------------------------------+ 0
	|	struct ptrace_user	|
	+-------------------------------+ sizeof(struct ptrace_user)
	/////////////////////////////////
	+-------------------------------+ <===== ptrace_user.pt_memptr
	| pt_mem_desc[ pt_nmem ]        |
	+-------------------------------+
	/////////////////////////////////
	+-------------------------------+ <===== ptrace_user.pt_dataptr
	|	data			|
	+-------------------------------+ ptrace_user.pt_dataptr + dsize
	|	stack			|
	+-------------------------------+ ptrace_user.pt_dataptr+dsize+ssize
	|	extra memory 0		| optional
	+-------------------------------+  ... + pt_mem_desc[0].pt_mem_size
	|	extra memory 1		| optional
	+-------------------------------+  ... + pt_mem_desc[1].pt_mem_size
	|       etc.			| optional
	+-------------------------------+

	Notice that the three or four components are contiguous and
	no padding or alignment is used.  The shared memory segment never
	appears on Mach because this abstraction falls far short of what
	is needed to represent Mach shared memory.

 */

/*
 * Ptrace request numbers.
 */
#define PT_TRACEME	0	/* Child's request to be traced */
#define PT_FIRST	1	/* First valid value */
#define PT_READ		1	/* Read from child's memory */
#define PT_OLDREAD	2	/* Artifact of PDP-11 history */
#define PT_READ_U	3	/* Read from child's u area */
#define PT_WRITE	4	/* Write to child's memory */
#define PT_OLDWRITE	5	/* Another artifact */
#define PT_WRITE_U	6	/* Write child's u area */
#define PT_SENDSIG	7	/* Send signal to child */
#define PT_KILL		8	/* Kill traced process */
#define PT_SINGLESTEP	9	/* Execute one instruction */
				/* additions for cdb follow  Unimplemented */
#define PT_ATTACH	10	/* Attach to a process */
#define PT_FREEPROC	11	/* Free process from debugger */
#define PT_GETPPID	12	/* Get processes parent process id */
#define PT_GETCOMM	13	/* Get command name from u */
#define PT_HWBPT	14	/* Set/clear hardware breakpoint */
#define PT_LAST		14	/* Last valid value */

/*
 * Structure used for passing in information about hardware breakpoints.
 */

struct ptrace_hwbpt {
	int	options;	/* Type of hardware breakpointing (0 == off) */
	caddr_t mask;		/* Mask of bits in address that don't matter. */
};

				/* Bit values for options field above. */
#define HWBPT_READ	0x01	/* Break on read. */
#define HWBPT_WRITE	0x02	/* Break on write. */

/*
 *	Offsets into ptrace_user structure
 */

#define PU_SP		0
#define PU_R7		4
#define PU_R6		8
#define PU_R5		12
#define PU_R4		16
#define PU_R3		20
#define PU_R2		24
#define PU_R1		28
#define PU_R0		32
#define PU_FP		36
#define PU_PC		40
#define PU_PSL		44
#define PU_DATAPTR	48
#define PU_FSR		52
#define PU_F0		56
#define PU_F1		60
#define PU_F2		64
#define PU_F3		68
#define PU_F4		72
#define PU_F5		76
#define PU_F6		80
#define PU_F7		84
#define PU_COMM		88
#define PU_ARG0		344
#define PU_ARG1		348
#define PU_ARG2		352
#define PU_ARG3		356
#define PU_ARG4		360
#define PU_ARG5		364
#define PU_ARG6		368
#define PU_ARG7		372
#define PU_ARG8		376
#define PU_ARG9		380
#define PU_DSIZE	384
#define PU_SSIZE	388
#define PU_AH_MAGIC	392
#define PU_AH_VSTAMP	394
#define PU_AH_TSIZE	396
#define PU_AH_DSIZE	400
#define PU_AH_BSIZE	404
#define PU_AH_MSIZE	408
#define PU_AH_MSTART	412
#define PU_AH_ENTRY	416
#define PU_AH_TSTART	420
#define PU_AH_DSTART	424
#define PU_AH_EMOD	428
#define PU_SIGNAL	432
#define PU_USRSTACK	436
#define	PU_REV			440
#define	PU_SUBCODE		444
#define PU_D0			448
#define PU_D1			456
#define PU_D2			464
#define PU_D3			472
#define PU_D4			480
#define PU_D5			488
#define PU_D6			496
#define PU_D7			504
#define	PU_NMEM			512
#define	PU_MEMPTR		516
/*#define PU_NEXTRA	440*/
/*#define PU_EXTSIZE	444*/
