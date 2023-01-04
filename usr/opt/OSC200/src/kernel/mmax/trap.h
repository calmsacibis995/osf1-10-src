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
 *	@(#)$RCSfile: trap.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:37 $
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

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

/*
 *	Trap codes for ns32000
 */

#define T_AST		0
#define T_ABT_READ	1
#define T_ABT_WRITE	2
#define T_FPU		3
#define T_ILL		4
#define T_SVC		5
#define T_DVZ		6
#define T_FLG		7
#define T_BPT		8
#define T_TRC		9
#define T_UND		10
/*
 *	Notes:
 *		1.  T_SVC is not used: svc calls syscall() directly.
 *		2.  These correspond to the ns32000 Vector numbers,
 *			except that ABT was split into two cases, and
 *			AST is faked in software. [NMI and NVI aren't traps]
 */

#if	MMAX_XPC || MMAX_APC

#define T_BUSERR        16              /* bus error -- APC only */
#define T_ICURACE       17              /* icu race condition */
#define T_DUARTFLT      18              /* DUART Trap */
#define T_CASCADE       19              /* Illegal Cascade Fault */
#define T_UNDEFLT       20              /* Undefined trap - cannot happen */
#define	T_RBE		21		/* recoverable bus error -- XPC */
#define	T_NBE		22		/* non-recoverable bus error -- XPC */
#define	T_OVF		23		/* integer overflow trap -- XPC */
#define	T_DBG		24		/* debug register trap -- XPC */

/*  Software-defined flags for communicating status from abt handler
 *  in locore.s to trap() in trap.c
 */

#define TF_FLAGS        0xff000000      /* Flags field in type word */
#define TF_STATUS       0x00f00000      /* CPU Status bits at abort */
#define TFO_STATUS              20      /* Offset to CPU Stat bits  */
#define TF_TYPE         0x000fffff      /* Type value in type word  */

#define TFV_USER        0x80000000      /* User mode bit was on in ASR */
#define TFV_USER_BIT            31

#endif	MMAX_XPC || MMAX_APC
