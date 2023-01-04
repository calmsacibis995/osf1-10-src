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
 *	@(#)$RCSfile: trap.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:20:21 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */


/*
 * I386 Trap type values
 */

#define T_DIVERR	0	/* divide by 0 eprror		*/
#define T_SGLSTP	1	/* single step			*/
#define T_NMIFLT	2	/* NMI				*/
#define T_BPTFLT	3	/* breakpoint fault		*/
#define T_INTOFLT	4	/* INTO overflow fault		*/
#define T_BOUNDFLT	5	/* BOUND instruction fault	*/
#define T_INVOPFLT	6	/* invalid opcode fault		*/
#define T_NOEXTFLT	7	/* extension not available fault*/
#define T_DBLFLT	8	/* double fault			*/
#define T_EXTOVRFLT	9	/* extension overrun fault	*/
#define T_INVTSSFLT	10	/* invalid TSS fault		*/
#define T_SEGNPFLT	11	/* segment not present fault	*/
#define T_STKFLT	12	/* stack fault			*/
#define T_GPFLT		13	/* general protection fault	*/
#define T_PGFLT		14	/* page fault			*/
#define T_EXTERRFLT	16	/* extension error fault	*/
#define T_ENDPERR	33	/* emulated extension error flt	*/
#define T_ENOEXTFLT	32	/* emulated ext not present	*/
#define T_AST		34	/* handle an ast event */

/*
 * Trap type values
 */

/* The first three constant values are known to the real world <signal.h> */
/* Well, I wonder what we do? */
/*#define	T_RESADFLT	0		/* reserved addressing fault */
/*#define	T_PRIVINFLT	1		/* privileged instruction fault */
/*#define	T_RESOPFLT	2		/* reserved operand fault */
/* End of known constants */
#define T_READ_FAULT	14		/* read fault */
#define T_WRITE_FAULT	15		/* write fault */
#define T_KDB_ENTRY	16		/* force entry to kernel debugger */

/* 
 * List of printable strings for trap types.  TRAP_TYPES is the number 
 * of elements in the array.  It may be smaller than the list of trap 
 * types given above.
 */
#ifndef  ASSEMBLER
extern char *trap_type[];
extern int TRAP_TYPES;
#endif   ASSEMBLER
