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
 *	@(#)$RCSfile: ipc_pobj.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:24:41 $
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

#ifndef	_KERN_IPC_POBJ_H_
#define _KERN_IPC_POBJ_H_

#include <mach_km.h>

#include <kern/host.h>
#include <mach/port.h>
#include <kern/port_object.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <vm/vm_map.h>

extern void port_object_set();
extern int port_object_get();
extern port_object_type_t port_object_type();

extern task_t convert_port_to_task();
extern thread_t convert_port_to_thread();
extern vm_map_t convert_port_to_map();
extern port_t convert_task_to_port();
extern port_t convert_thread_to_port();

extern host_t convert_port_to_host();
extern host_t convert_port_to_host_priv();
extern processor_t convert_port_to_processor();
extern processor_set_t convert_port_to_pset();
extern processor_set_t convert_port_to_pset_name();
extern port_t convert_host_to_port();
extern port_t convert_processor_to_port();
extern port_t convert_pset_to_port();
extern port_t convert_pset_name_to_port();

#if	MACH_KM
#include <kern/kern_mon.h>

extern monitor_t convert_port_to_monitor();
extern port_t convert_monitor_to_port();
#endif	/* MACH_KM */
#endif	/* _KERN_IPC_POBJ_H_ */
