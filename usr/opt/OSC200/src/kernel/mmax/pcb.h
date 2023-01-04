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
 *	@(#)$RCSfile: pcb.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:22 $
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
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */
#ifndef	_PCB_
#define _PCB_

#ifdef	KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax/pmap.h>
#include <sys/types.h>
#endif	KERNEL

/*
 * MMAX process control block
 */

struct pcb
{
	char	*pcb_usp;	/* User stack pointer */
	char	*pcb_ssp;	/* System stack pointer */
	int	pcb_r0; 
	int	pcb_r1; 
	int	pcb_r2; 
	int	pcb_r3; 
	int	pcb_r4; 
	int	pcb_r5; 
	int	pcb_r6; 
	int	pcb_r7; 
	int	pcb_fp; 
	int	pcb_pc; 	/* program counter */
	int	pcb_modpsr; 	/* program status register and mod register */
#if	MMAX_XPC || MMAX_APC
	short   pcb_isrv;	/* ISRV Register in ICU (interrupt state)   */
#endif	MMAX_XPC || MMAX_APC
	quad	pcb_f0;
	quad	pcb_f1;
	quad	pcb_f2;
	quad	pcb_f3;
	quad	pcb_f4;
	quad	pcb_f5;
	quad	pcb_f6;
	quad	pcb_f7;
	int	pcb_fsr;	/* FPU status register */
	struct pt_entry	*pcb_ptbr;
	int	pcb_sigc[5];
#if	MMAX_XPC
	int	pcb_dcr;	/* Debug Condition Register */
	int	pcb_dsr;	/* Debug Status Register */
	int	pcb_car;	/* Compare Address Register */
	int	pcb_bpc;	/* Breakpoint Program Counter */
#endif	MMAX_XPC
};

#ifdef	KERNEL
void	pcb_init();

/*
 * Shutdown any state associated with a thread's pcb.
 */
#define pcb_terminate(thread)
#endif	KERNEL

#endif	_PCB_
