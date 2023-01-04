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
 *	@(#)$RCSfile: ipc_tt.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:02 $
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

#ifndef	_KERN_IPC_TT_H_
#define _KERN_IPC_TT_H_

#include <mach_km.h>

#include <mach/boolean.h>

extern void ipc_task_init();
extern void ipc_task_enable();
extern void ipc_task_disable();
extern void ipc_task_terminate();

extern void ipc_thread_init();
extern void ipc_thread_enable();
extern void ipc_thread_disable();
extern void ipc_thread_terminate();

extern boolean_t task_secure();

#if	MACH_KM
extern void ipc_monitor_init();
extern void ipc_monitor_enable();
extern void ipc_monitor_disable();
extern void ipc_monitor_terminate();
#endif	/* MACH_KM */

#endif	/* _KERN_IPC_TT_H_ */
