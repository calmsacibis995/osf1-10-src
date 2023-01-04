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
 *	@(#)$RCSfile: ipc_prims.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:24:51 $
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

#ifndef	_KERN_IPC_PRIMS_H_
#define _KERN_IPC_PRIMS_H_

#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <kern/ipc_hash.h>
#include <kern/macro_help.h>

extern port_hash_t obj_entry_find();
extern port_hash_t obj_entry_lookup();
extern void obj_entry_change();
extern void obj_entry_remove();
extern void obj_entry_dealloc();
extern void obj_entry_destroy();
extern void obj_entry_insert();
extern void obj_entry_create();
extern port_hash_t obj_entry_make();

extern kern_return_t obj_alloc();
extern void obj_destroy_rights();

#define task_check_name(task, name)	\
		(obj_entry_lookup((task), (name)) != PORT_HASH_NULL)

#define task_check_rights(task, obj)	\
		(obj_entry_find((task), (obj)) != PORT_HASH_NULL)

extern void msg_queue_changed();

/*
 * extern void
 * obj_entry_lookup_macro(task, name, entry, notfound)
 *	task_t task;
 *	port_name_t name;
 *	port_hash_t &entry;
 *	code notfound;
 *
 * The task must be locked.  Upon normal return, the by-reference
 * parameter "entry" points to the translation entry found.
 * If no entry is found, the "notfound" code (which should be a
 * single complete statement) is executed; it should return/goto.
 */

#define obj_entry_lookup_macro(_task, name, entry, notfound) 		\
MACRO_BEGIN								\
	register port_hash_bucket_t *bucket;				\
									\
	bucket = &TL_table[port_hash_TL((_task), (name))];		\
	bucket_lock(bucket);						\
	(entry) = (port_hash_t) queue_first(&bucket->head);		\
									\
	for (;;) {							\
		if (queue_end(&bucket->head, (queue_entry_t) (entry))) {\
			bucket_unlock(bucket);				\
			notfound					\
		}							\
									\
		if (((entry)->task == (_task)) &&			\
		    ((entry)->local_name == (name))) {			\
			bucket_unlock(bucket);				\
			break;						\
		}							\
									\
		ipc_event(bucket_misses);				\
		(entry) = (port_hash_t) queue_next(&(entry)->TL_chain);	\
	}								\
MACRO_END

#endif	/* _KERN_IPC_PRIMS_H_ */
