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
 *	@(#)$RCSfile: ipc_hash.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:24:03 $
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

#ifndef	_KERN_IPC_HASH_H_
#define _KERN_IPC_HASH_H_

#include <mach/port.h>
#include <kern/queue.h>
#include <kern/task.h>
#include <kern/kern_obj.h>
#include <kern/kern_msg.h>
#include <kern/macro_help.h>

/*
 *	Port rights and translation data structures:
 *
 *	Entries representing valid (task, port, local name) tuples
 *	are hashed by (task, port) and by (task, local name);
 *	additionally, each port and task has a chain of all tuples
 *	containing that port/task.
 *
 *	All the fields except for kmsg are locked by both the task
 *	and object.  Both need to be locked for modifications; locking
 *	either gets read access.  The object controls the kmsg field.
 */

typedef struct port_hash {
	queue_chain_t	TP_chain;	/* Chain for (task, object) table */
	queue_chain_t	TL_chain;	/* Chain for (task, name) table */

	task_t		task;		/* The owning task */
	queue_chain_t	task_chain;	/* Chain for "all same task" */

	port_name_t	local_name;	/* The task's name for us */
	port_type_t	type;		/* The type of capability */

	kern_obj_t	obj;		/* Corresponding internal object */
	queue_chain_t	obj_chain;	/* All entries for an object */

	/* special field: only locked by object */
	kern_msg_t	kmsg;		/* Used for SEND_NOTIFY */
} *port_hash_t;

#define PORT_HASH_NULL	((port_hash_t) 0)

/*
 *	The hash tables themselves
 */

#define PORT_HASH_COUNT		(1 << 11)

typedef struct port_hash_bucket {
	queue_head_t head;
	decl_simple_lock_data(,lock)
} port_hash_bucket_t;


#define port_hash_TP(task,global) \
	( ( ((int)(task)) + (((int)(global)) >> 5) ) & (PORT_HASH_COUNT-1))
#define port_hash_TL(task,local) \
	( (((int)(task)) >> 2) + (int)(local) ) & (PORT_HASH_COUNT - 1)

extern port_hash_bucket_t *TP_table;
extern port_hash_bucket_t *TL_table;

#define bucket_lock_init(bucket)	simple_lock_init(&(bucket)->lock)
#define bucket_lock(bucket)		simple_lock(&(bucket)->lock)
#define bucket_unlock(bucket)		simple_unlock(&(bucket)->lock)

#define BUCKET_ENTER(bucket, entry, chain) \
MACRO_BEGIN								\
	bucket_lock(bucket); 						\
	queue_enter(&(bucket)->head, (entry), port_hash_t, chain); 	\
	bucket_unlock(bucket); 						\
MACRO_END

#define BUCKET_REMOVE(bucket, entry, chain) \
MACRO_BEGIN								\
	bucket_lock(bucket); 						\
	queue_remove(&(bucket)->head, (entry), port_hash_t, chain);	\
	bucket_unlock(bucket); 						\
MACRO_END

#define PORT_TYPE_IS_PORT(type)	((type) != PORT_TYPE_SET)
#define PORT_TYPE_IS_SET(type)	((type) == PORT_TYPE_SET)

#endif	/* _KERN_IPC_HASH_H_ */
