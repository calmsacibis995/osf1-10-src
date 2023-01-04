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
 *	@(#)$RCSfile: mips_ptrace.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:16:43 $
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
 * derived from mips_ptrace.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*
 * register number definitions for PT_READ_U's and PT_WRITE_U's
 */
#define GPR_BASE	0			/* general purpose regs */
#define	NGP_REGS	32			/* number of gpr's */

#define FPR_BASE	(GPR_BASE+NGP_REGS)	/* fp regs */
#define	NFP_REGS	32			/* number of fp regs */

#define	SIG_BASE	(FPR_BASE+NFP_REGS)	/* sig handler addresses */
#define	NSIG_HNDLRS	32			/* number of signal handlers */

#define SPEC_BASE	(SIG_BASE+NSIG_HNDLRS)	/* base of spec purpose regs */
#define PC		SPEC_BASE+0		/* program counter */
#define	CAUSE		SPEC_BASE+1		/* cp0 cause register */
#define MMHI		SPEC_BASE+2		/* multiply high */
#define MMLO		SPEC_BASE+3		/* multiply low */
#define FPC_CSR		SPEC_BASE+4		/* fp csr register */
#define FPC_EIR		SPEC_BASE+5		/* fp eir register */
#define TRAPCAUSE	SPEC_BASE+6		/* multiplex SIGTRAP cause */
#define TRAPINFO	SPEC_BASE+7		/* associated info to SIGTRAP */
#define NSPEC_REGS	8			/* number of spec registers */
#define NPTRC_REGS	(SPEC_BASE + NSPEC_REGS)

/*
 * causes for SIGTRAP
 */
#define CAUSEEXEC	1		/* traced process exec'd */
#define CAUSEFORK	2		/* traced process fork'd */
#define CAUSEWATCH	3		/* traced process hit a watchpoint */
#define CAUSESINGLE	4		/* traced process executed 1 instr */
#define CAUSEBREAK	5		/* traced process hit a breakpoint */
#define	CAUSETRACEON	6		/* initial trap after PTRC_TRACEON */

