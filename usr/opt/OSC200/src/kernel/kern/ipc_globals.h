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
 *	@(#)$RCSfile: ipc_globals.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:23:57 $
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

#ifndef	_KERN_IPC_GLOBALS_H_
#define _KERN_IPC_GLOBALS_H_

#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/notify.h>
#include <kern/zalloc.h>
#include <kern/task.h>
#include <kern/ipc_hash.h>
#include <vm/vm_map.h>

typedef struct object_copyout_table {
	void (*destroy)(/* kern_obj_t obj */);
	int (*func)(/* task_t task, kern_obj_t obj, port_name_t name */);
	boolean_t nomerge;
	port_type_t result[PORT_TYPE_LAST];
} object_copyout_table_t;

typedef struct object_copyin_table {
	boolean_t illegal;
	boolean_t nodealloc;
	boolean_t dodealloc;
	port_name_t result;
	void (*func)(/* task_t task, kern_obj_t obj */);
} object_copyin_table_t;

extern zone_t kmsg_zone;
extern zone_t kmsg_zone_large;

extern zone_t port_zone;
extern zone_t port_zone_reserved;

extern zone_t set_zone;

extern task_t ipc_soft_task;
extern vm_map_t ipc_kernel_map;

#define IPC_DEBUG_BOGUS_KMSG	0x00000001
#define IPC_DEBUG_SEND_INT	0x00000002
#define IPC_DEBUG_1K_PORTS	0x00000004
#define IPC_DEBUG_KPORT_DIED	0x00000008
#define IPC_DEBUG_SET_REFS	0x00000010
#define IPC_DEBUG_PORT_REFS	0x00000020

extern unsigned int ipc_debug;

extern zone_t port_hash_zone;

extern port_hash_bucket_t *TP_table;
extern port_hash_bucket_t *TL_table;

extern notification_t notification_template;
extern notification_t complex_notification_template;

extern object_copyout_table_t
object_copyout_table[MSG_TYPE_LAST];

extern object_copyin_table_t
object_copyin_table[MSG_TYPE_LAST][PORT_TYPE_LAST];

extern unsigned int timeout_minimum;
extern unsigned int timeout_scaling_factor;

extern void ipc_bootstrap();
extern void ipc_init();

#endif	/* _KERN_IPC_GLOBALS_H_ */
