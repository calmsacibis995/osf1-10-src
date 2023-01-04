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
 *	@(#)$RCSfile: exception.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:33:01 $
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
 *	Codes and subcodes for 80386 exceptions.
 */

/*
 *	EXC_BAD_INSTRUCTION
 */

#ifndef	_MACH_I386_EXCEPTION_H_
#define _MACH_I386_EXCEPTION_H_

#define EXC_I386_INVOP			1

/*
 *	EXC_ARITHMETIC
 */

#define EXC_I386_DIV			1
#define EXC_I386_INTO			2
#define EXC_I386_NOEXT			3
#define EXC_I386_EXTOVR			4
#define EXC_I386_EXTERR			5
#define EXC_I386_EMERR			6
#define EXC_I386_BOUND			7

/*
 *	EXC_SOFTWARE
 */

/*
 *	EXC_BAD_ACCESS
 */

/*
 *	EXC_BREAKPOINT
 */

#define EXC_I386_SGL			1
#define EXC_I386_BPT			2

#define EXC_I386_DIVERR		0	/* divide by 0 eprror		*/
#define EXC_I386_SGLSTP		1	/* single step			*/
#define EXC_I386_NMIFLT		2	/* NMI				*/
#define EXC_I386_BPTFLT		3	/* breakpoint fault		*/
#define EXC_I386_INTOFLT	4	/* INTO overflow fault		*/
#define EXC_I386_BOUNDFLT	5	/* BOUND instruction fault	*/
#define EXC_I386_INVOPFLT	6	/* invalid opcode fault		*/
#define EXC_I386_NOEXTFLT	7	/* extension not available fault*/
#define EXC_I386_DBLFLT		8	/* double fault			*/
#define EXC_I386_EXTOVRFLT	9	/* extension overrun fault	*/
#define EXC_I386_INVTSSFLT	10	/* invalid TSS fault		*/
#define EXC_I386_SEGNPFLT	11	/* segment not present fault	*/
#define EXC_I386_STKFLT		12	/* stack fault			*/
#define EXC_I386_GPFLT		13	/* general protection fault	*/
#define EXC_I386_PGFLT		14	/* page fault			*/
#define EXC_I386_EXTERRFLT	16	/* extension error fault	*/
#define EXC_I386_ENDPERR	33	/* emulated extension error flt	*/
#define EXC_I386_ENOEXTFLT	32	/* emulated ext not present	*/

#endif	_MACH_I386_EXCEPTION_H_
