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
 *	@(#)$RCSfile: cpu.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:12 $
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
#ifndef	_CPU_
#define _CPU_

#ifdef	KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#endif

#if	MMAX_XPC || MMAX_APC || MMAX_DPC
#include <machine/cpudefs.h>
#endif	MMAX_XPC || MMAX_APC || MMAX_DPC

#define CPU_NUMBER() cpu_number()

#ifdef	KERNEL
int	master_cpu;
int	*cpu_number_addr;
#endif	KERNEL
/*
 *	The following is an optimized version of getcpuid() from 
 *	dpcdefs.h.
 */

#if	MMAX_XPC
/*
 * Must mask off the right bits, no happy overflow situation as on APC.
 * Eventually optimize by using local memory.
 */
#define	cpu_number()		(*XPCREG_CSR & (XPCCSR_CPUID | XPCCSR_SLOTID))
#endif	MMAX_XPC

#if	MMAX_APC
/* Original version --
 *
 * #define getcpuid() (*APCREG_CSR & (APCCSR_CPUID | APCCSR_SLOTID))
 *
 * Optimized version of getcpuid() --
 *
 *       Non-relevent bits oflow order byte guaranteed to be zero
 */

#define cpu_number()              ((*((unsigned char *)0xffffff54)))
#endif	MMAX_APC

#if	MMAX_DPC
#define	DPCREG_STS		0xfffffffc
#define DPCSTS_PROCID		0x3d
#define cpu_number() ((*(long *)DPCREG_STS) & DPCSTS_PROCID)
#endif	MMAX_DPC

#endif	_CPU_
