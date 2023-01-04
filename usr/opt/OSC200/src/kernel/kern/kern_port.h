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
 *	@(#)$RCSfile: kern_port.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:32 $
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
 * Kernel internal structure associated with a port.
 *
 */

#ifndef	_KERN_KERN_PORT_H_
#define _KERN_KERN_PORT_H_

#include <mach_ipc_xxxhack.h>

#include <mach/port.h>
#include <kern/kern_obj.h>
#include <kern/task.h>
#include <kern/msg_queue.h>
#include <kern/queue.h>
#include <kern/port_object.h>
#include <kern/kern_set.h>

typedef struct kern_port {
	struct kern_obj port_obj;

	task_t		port_receiver;
				/* Task holding receive rights */
	port_name_t	port_receiver_name;
				/* Receiver's local name for port */
#if	MACH_IPC_XXXHACK
	task_t		port_owner;
				/* Task holding ownership */
#endif	
	struct kern_port *port_backup;
				/* "Send rights" to a backup port */

	int		port_message_count;
				/* Optimistic number of queued messages */
	int		port_backlog;
				/* Queue limit before blocking */
	msg_queue_t	port_messages;
				/* Queued messages, if not in set */
	queue_chain_t	port_blocked_threads;
				/* Senders waiting to complete */

	port_object_t	port_object;
				/* Kernel object I represent */

	kern_set_t	port_set;
				/* The set I belong to (else NULL) */
	queue_chain_t	port_brothers;
				/* List of all members of that set */
} port_data_t, *kern_port_t;

#define port_data_lock		port_obj.obj_data_lock
#define port_in_use		port_obj.obj_in_use
#define port_references		port_obj.obj_references
#define port_home_zone		port_obj.obj_home_zone
#define port_translations	port_obj.obj_translations

#define		KERN_PORT_NULL	((kern_port_t) 0)

#define port_lock(port)		obj_lock(&(port)->port_obj)
#define port_lock_try(port)	obj_lock_try(&(port)->port_obj)
#define port_unlock(port)	obj_unlock(&(port)->port_obj)
#define port_check_unlock(port)	obj_check_unlock(&(port)->port_obj)
#define port_free(port)		obj_free(&(port)->port_obj)

#define port_reference_macro(port)	obj_reference(&(port)->port_obj)
#define port_release_macro(port)	obj_release(&(port)->port_obj)

extern void port_reference();
extern void port_release();
extern kern_return_t port_alloc();
extern void port_destroy();
extern kern_return_t port_dealloc();

#endif	/* _KERN_KERN_PORT_H_ */
