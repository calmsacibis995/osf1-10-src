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
 *	@(#)$RCSfile: syscall_sw.h,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/09/03 19:26:23 $
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

#ifndef	_MACH_SYSCALL_SW_H_
#define _MACH_SYSCALL_SW_H_

/*
 *	The machine-dependent "syscall_sw.h" file should
 *	define a macro for
 *		kernel_trap(trap_name, trap_number, arg_count)
 *	which will expand into assembly code for the
 *	trap.
 *
 *	N.B.: When adding calls, do not put spaces in the macros.
 */

#include <mach/machine/syscall_sw.h>

/*
 *	These trap numbers should be taken from the
 *	table in "../kern/syscall_sw.c".
 */

kernel_trap(task_self,-10,0)
kernel_trap(thread_reply,-11,0)
kernel_trap(task_notify,-12,0)
kernel_trap(thread_self,-13,0)
kernel_trap(msg_send_old,-14,3)
kernel_trap(msg_receive_old,-15,3)
kernel_trap(msg_rpc_old,-16,5)
kernel_trap(msg_send_trap,-20,4)
kernel_trap(msg_receive_trap,-21,5)
kernel_trap(msg_rpc_trap,-22,6)

kernel_trap(lw_wire,-30,3)
kernel_trap(lw_unwire,-31,1)

kernel_trap(htg_unix_syscall,-52,3);

kernel_trap(inode_swap_preference,-40,3)
kernel_trap(init_process,-41,0)
kernel_trap(map_fd,-43,5)

kernel_trap(host_self,-55,1)
kernel_trap(host_priv_self,-56,1)
kernel_trap(swtch_pri,-59,1)
kernel_trap(swtch,-60,0)
kernel_trap(thread_switch,-61,3)

#if	MACH_SCTIMES
kernel_trap(mach_sctimes_0,-70,0)
kernel_trap(mach_sctimes_1,-71,1)
kernel_trap(mach_sctimes_2,-72,2)
kernel_trap(mach_sctimes_3,-73,3)
kernel_trap(mach_sctimes_4,-74,4)
kernel_trap(mach_sctimes_5,-75,5)
kernel_trap(mach_sctimes_6,-76,6)

kernel_trap(mach_sctimes_7,-77,0)
kernel_trap(mach_sctimes_8,-78,6)

kernel_trap(mach_sctimes_9,-79,1)
kernel_trap(mach_sctimes_10,-80,2)
kernel_trap(mach_sctimes_11,-81,2)

kernel_trap(mach_sctimes_port_alloc_dealloc,-82,1)
#endif	/* MACH_SCTIMES */

#endif	/* _MACH_SYSCALL_SW_H_ */
