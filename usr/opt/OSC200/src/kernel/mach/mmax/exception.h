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
 *	@(#)$RCSfile: exception.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:33:52 $
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
 *	Codes and subcodes for ns32000 exceptions.
 */

#ifndef	_MACH_MMAX_EXCEPTION_H_
#define _MACH_MMAX_EXCEPTION_H_

/*
 *	EXC_BAD_INSTRUCTION
 */

#define EXC_NS32K_FPU			3
#define EXC_NS32K_ILL			4
#define EXC_NS32K_UND			10

/*
 *	EXC_ARITHMETIC
 */

#define EXC_NS32K_FPU			3
#define EXC_NS32K_DVZ			6

/*
 *	FPU subcodes
 */

#define EXC_NS32K_FPU_UNDERFLOW 	1
#define EXC_NS32K_FPU_OVERFLOW		2
#define EXC_NS32K_FPU_DVZ		3
#define EXC_NS32K_FPU_ILLEGAL 		4
#define EXC_NS32K_FPU_INVALID		5
#define EXC_NS32K_FPU_INEXACT		6
#define EXC_NS32K_FPU_OPERAND		7
#define	EXC_NS32K_FPU_INTOVF		8

/*
 *	EXC_SOFTWARE
 */

#define EXC_NS32K_FLG			7

/*
 *	EXC_BREAKPOINT
 */

#define EXC_NS32K_BPT			8
#define EXC_NS32K_TRC			9

#endif	_MACH_MMAX_EXCEPTION_H_
