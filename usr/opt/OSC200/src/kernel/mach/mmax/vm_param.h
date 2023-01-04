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
 *	@(#)$RCSfile: vm_param.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:34:02 $
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
 *	File:	mach/mmax/vm_param.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	NS32000 machine dependent virtual memory parameters.
 *	Most of the declarations are preceeded by NS32K_ (or ns32k_)
 *	which is OK because only Multimax specific code will be using
 *	them.  In almost all cases the declarations are the same as
 *	the VAX.  The names were changed to catch VAX parameter dependencies.
 */

#ifndef	_MACH_MMAX_VM_PARAM_H_
#define _MACH_MMAX_VM_PARAM_H_

#ifdef	KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#endif	KERNEL

#include <mach/mmax/vm_types.h>

#define BYTE_SIZE	8	/* byte size in bits */

#if	MMAX_XPC || MMAX_APC
#define NS32K_PGBYTES  4096     /* bytes per ns32k page */
#define NS32K_PGSHIFT    12     /* number of bits to shift for pages */
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define NS32K_PGBYTES	512	/* bytes per ns32k page */
#define NS32K_PGSHIFT	9	/* number of bits to shift for pages */
#endif	MMAX_DPC

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#if	MMAX_XPC || MMAX_APC || MMAX_DPC
#define	ns32k_btop(x)		(((unsigned)(x)) >> NS32K_PGSHIFT)
#define	ns32k_ptob(x)		((caddr_t)(((unsigned)(x)) << NS32K_PGSHIFT))
#endif	MMAX_XPC || MMAX_APC || MMAX_DPC

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#if	MMAX_XPC || MMAX_APC || MMAX_DPC
#define ns32k_round_page(x)	((((unsigned)(x)) + NS32K_PGBYTES - 1) & \
					~(NS32K_PGBYTES-1))
#define ns32k_trunc_page(x)	(((unsigned)(x)) & ~(NS32K_PGBYTES-1))
#endif	MMAX_XPC || MMAX_APC || MMAX_DPC


/*	VM_MAX_KERNEL_ADDRESS does not allow the SCC to be mapped.
 */
vm_offset_t	vm_max_kernel_address;

#if	MMAX_XPC || MMAX_APC
#define	VM_MIN_ADDRESS		((vm_offset_t) 0)
#define	VM_MAX_ADDRESS		((vm_offset_t) 0x7fffe000)
#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x0)
#define VM_MAX_KERNEL_ADDRESS	vm_max_kernel_address
#define	VM_MAX_KERNEL_SPACE	VM_MAX_ADDRESS
#define	VM_MIN_KERNEL_SPACE	((vm_offset_t) 0x4000000)
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define	VM_MIN_ADDRESS		((vm_offset_t) 0)
#define	VM_MAX_ADDRESS		((vm_offset_t) 0x1000000)
#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x0)
#define VM_MAX_KERNEL_ADDRESS	vm_max_kernel_address
#define	VM_MAX_KERNEL_SPACE	((vm_offset_t) 0xfc0000)
#define	VM_MIN_KERNEL_SPACE	((vm_offset_t) 0xfc0000)
#endif	MMAX_DPC

#define KERNEL_STACK_SIZE	(8*1024)
#define INTSTACK_SIZE		(3*512)

#if	MMAX_XPC || MMAX_APC || MMAX_DPC
/*
 *	Conversion between NS32000 pages and VM pages
 */

#define trunc_ns32k_to_vm(p)	(atop(trunc_page(ns32k_ptob(p))))
#define round_ns32k_to_vm(p)	(atop(round_page(ns32k_ptob(p))))
#define vm_to_ns32k(p)		(ns32k_btop(ptoa(p)))
#endif	MMAX_XPC || MMAX_APC || MMAX_DPC

#endif	_MACH_MMAX_VM_PARAM_H_
