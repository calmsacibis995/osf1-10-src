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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ipc_globals.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/11/05 15:22:19 $";
#endif 
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
 * File:	ipc_globals.c
 * Purpose:
 *	Define & initialize Mach IPC global variables.
 */

#include <mach_ipc_stats.h>
#include <mach_ipc_xxxhack.h>

#include <mach/boolean.h>
#include <mach/port.h>
#include <kern/task.h>
#include <kern/zalloc.h>
#include <mach/notify.h>
#include <kern/mach_param.h>
#include <kern/kern_port.h>
#include <kern/kern_set.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <kern/ipc_hash.h>
#include <kern/ipc_copyin.h>
#include <kern/ipc_copyout.h>
#include <kern/ipc_globals.h>

zone_t kmsg_zone;
zone_t kmsg_zone_large;

zone_t port_zone;
zone_t port_zone_reserved;

zone_t set_zone;

task_t ipc_soft_task;
vm_map_t ipc_kernel_map;

unsigned int ipc_debug = 0;

zone_t port_hash_zone;

port_hash_bucket_t *TP_table;
port_hash_bucket_t *TL_table;

notification_t notification_template;
notification_t complex_notification_template;

object_copyout_table_t object_copyout_table[MSG_TYPE_LAST];

object_copyin_table_t object_copyin_table[MSG_TYPE_LAST][PORT_TYPE_LAST];

unsigned int timeout_minimum = 1;
unsigned int timeout_scaling_factor;

int kmsg_large_max_num = 1024;
int kmsg_large_alloc_num = 1;

int kmsg_max_num = 4096;
int kmsg_alloc_num = 32;

extern int port_hash_max_num;
int port_hash_alloc_num = 256;

extern int port_max_num;
int port_alloc_num = 128;

extern int set_max_num;
int set_alloc_num = 32;

extern int port_reserved_max_num;
int port_reserved_alloc_num = 128;

/*
 *	Routine:	object_copyout_init [internal]
 *	Purpose:
 *		Called to initialize object_copyout_table.
 */
void
object_copyout_init()
{
	int mt;
	port_type_t pt;

#define init(mt, _destroy, _func, _nomerge)		\
MACRO_BEGIN						\
	object_copyout_table[mt].destroy = (_destroy);	\
	object_copyout_table[mt].func = (_func);	\
	object_copyout_table[mt].nomerge = (_nomerge);	\
MACRO_END

	for (mt = 0; mt < MSG_TYPE_LAST; mt++)
		init(mt, 0, 0, FALSE);

	init(MSG_TYPE_PORT, 0, 0, FALSE);
#if	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT_RECEIVE,
	     port_destroy_receive, port_copyout_receive, FALSE);
	init(MSG_TYPE_PORT_OWNERSHIP,
	     port_destroy_own, port_copyout_own, FALSE);
#endif	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT_ALL,
	     port_destroy_receive_own, port_copyout_receive_own, FALSE);

#undef	init

#define init(mt, pt, _result)					\
MACRO_BEGIN							\
	object_copyout_table[mt].result[pt] = (_result);	\
MACRO_END

	for (mt = 0; mt < MSG_TYPE_LAST; mt++)
		for (pt = 0; pt < PORT_TYPE_LAST; pt++)
			init(mt, pt, PORT_TYPE_NONE);

	init(MSG_TYPE_PORT, PORT_TYPE_NONE, PORT_TYPE_SEND);
	init(MSG_TYPE_PORT, PORT_TYPE_SEND, PORT_TYPE_SEND);
#if	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT, PORT_TYPE_RECEIVE, PORT_TYPE_RECEIVE);
	init(MSG_TYPE_PORT, PORT_TYPE_OWN, PORT_TYPE_OWN);
#endif	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT, PORT_TYPE_RECEIVE_OWN, PORT_TYPE_RECEIVE_OWN);

#if	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT_RECEIVE, PORT_TYPE_NONE, PORT_TYPE_RECEIVE);
	init(MSG_TYPE_PORT_RECEIVE, PORT_TYPE_SEND, PORT_TYPE_RECEIVE);
	init(MSG_TYPE_PORT_RECEIVE, PORT_TYPE_OWN, PORT_TYPE_RECEIVE_OWN);

	init(MSG_TYPE_PORT_OWNERSHIP, PORT_TYPE_NONE, PORT_TYPE_OWN);
	init(MSG_TYPE_PORT_OWNERSHIP, PORT_TYPE_SEND, PORT_TYPE_OWN);
	init(MSG_TYPE_PORT_OWNERSHIP, PORT_TYPE_RECEIVE, PORT_TYPE_RECEIVE_OWN);
#endif	MACH_IPC_XXXHACK

	init(MSG_TYPE_PORT_ALL, PORT_TYPE_NONE, PORT_TYPE_RECEIVE_OWN);
	init(MSG_TYPE_PORT_ALL, PORT_TYPE_SEND, PORT_TYPE_RECEIVE_OWN);

#undef	init
}

/*
 *	Routine:	object_copyin_init [internal]
 *	Purpose:
 *		Called to initialize object_copyin_table.
 */
void
object_copyin_init()
{
	int mt;
	port_type_t pt;

#define init(mt, pt, _illegal, _nodealloc, _dodealloc, _result, _func)	\
MACRO_BEGIN								\
	object_copyin_table[mt][pt].illegal = (_illegal);		\
	object_copyin_table[mt][pt].nodealloc = (_nodealloc);		\
	object_copyin_table[mt][pt].dodealloc = (_dodealloc);		\
	object_copyin_table[mt][pt].result = (_result);			\
	object_copyin_table[mt][pt].func = (_func);			\
MACRO_END

	for (mt = 0; mt < MSG_TYPE_LAST; mt++)
		for (pt = 0; pt < PORT_TYPE_LAST; pt++)
			init(mt, pt, TRUE, FALSE, FALSE, PORT_TYPE_NONE, 0);

	init(MSG_TYPE_PORT, PORT_TYPE_SEND,
	     FALSE, FALSE, FALSE,
	     PORT_TYPE_SEND, 0);

#if	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT, PORT_TYPE_RECEIVE,
	     FALSE, TRUE, FALSE,
	     PORT_TYPE_RECEIVE, 0);

	init(MSG_TYPE_PORT, PORT_TYPE_OWN,
	     FALSE, TRUE, FALSE,
	     PORT_TYPE_OWN, 0);
#endif	MACH_IPC_XXXHACK

	init(MSG_TYPE_PORT, PORT_TYPE_RECEIVE_OWN,
	     FALSE, TRUE, FALSE,
	     PORT_TYPE_RECEIVE_OWN, 0);

#if	MACH_IPC_XXXHACK
	init(MSG_TYPE_PORT_RECEIVE, PORT_TYPE_RECEIVE,
	     FALSE, FALSE, FALSE,
	     PORT_TYPE_SEND, port_copyin_receive);

	init(MSG_TYPE_PORT_RECEIVE, PORT_TYPE_RECEIVE_OWN,
	     FALSE, TRUE, FALSE,
	     PORT_TYPE_OWN, port_copyin_receive);

	init(MSG_TYPE_PORT_OWNERSHIP, PORT_TYPE_OWN,
	     FALSE, FALSE, FALSE,
	     PORT_TYPE_SEND, port_copyin_own);

	init(MSG_TYPE_PORT_OWNERSHIP, PORT_TYPE_RECEIVE_OWN,
	     FALSE, TRUE, FALSE,
	     PORT_TYPE_RECEIVE, port_copyin_own);
#endif	MACH_IPC_XXXHACK

	init(MSG_TYPE_PORT_ALL, PORT_TYPE_RECEIVE_OWN,
	     FALSE, FALSE, FALSE,
	     PORT_TYPE_SEND, port_copyin_receive_own);

#undef	init
}

/*
 *	Routine:	ipc_bootstrap [exported]
 *	Purpose:
 *		Initialize IPC structures needed even before
 *		the "kernel task" can be initialized
 */
void
ipc_bootstrap()
{
	int i;
	msg_size_t large_size;

	large_size = (MSG_SIZE_MAX +
		      (sizeof(struct kern_msg) - sizeof(msg_header_t)));

	kmsg_zone_large = zinit((vm_size_t) large_size,
		(vm_size_t) (kmsg_large_max_num * large_size),
		(vm_size_t) round_page(kmsg_large_alloc_num * large_size),
		"large messages");

	kmsg_zone = zinit((vm_size_t) KERN_MSG_SMALL_SIZE,
		(vm_size_t) (kmsg_max_num * KERN_MSG_SMALL_SIZE),
		(vm_size_t) round_page(kmsg_alloc_num * KERN_MSG_SMALL_SIZE),
		"messages");

	port_hash_zone = zinit((vm_size_t) sizeof(struct port_hash),
		(vm_size_t) (port_hash_max_num * sizeof(struct port_hash)),
		(vm_size_t) round_page(port_hash_alloc_num *
				       sizeof(struct port_hash)),
		"port translations");

	port_zone = zinit((vm_size_t) sizeof(struct kern_port),
		(vm_size_t) (port_max_num * sizeof(struct kern_port)),
		(vm_size_t) round_page(port_alloc_num *
				       sizeof(struct kern_port)),
		"ports");
	zchange(port_zone, FALSE, FALSE, TRUE, FALSE);/* make it exhaustible */

	set_zone = zinit((vm_size_t) sizeof(struct kern_set),
		(vm_size_t) (set_max_num * sizeof(struct kern_set)),
		(vm_size_t) round_page(set_alloc_num *
				       sizeof(struct kern_set)),
		"sets");
	zchange(set_zone, FALSE, FALSE, TRUE, FALSE);/* make it exhaustible */

	port_zone_reserved = zinit((vm_size_t) sizeof(struct kern_port),
		(vm_size_t) (port_reserved_max_num * sizeof(struct kern_port)),
		(vm_size_t) round_page(port_reserved_alloc_num *
				       sizeof(struct kern_port)),
		"ports (reserved)");

	TP_table = (port_hash_bucket_t *) kmem_alloc(kernel_map,
		   (vm_size_t) (PORT_HASH_COUNT * sizeof(port_hash_bucket_t)));
	if (TP_table == (port_hash_bucket_t *) 0)
		panic("ipc_bootstrap: cannot create TP_table");

	for (i = 0; i < PORT_HASH_COUNT; i++) {
		queue_init(&TP_table[i].head);
		bucket_lock_init(&TP_table[i]);
	}

	TL_table = (port_hash_bucket_t *) kmem_alloc(kernel_map,
		   (vm_size_t) (PORT_HASH_COUNT * sizeof(port_hash_bucket_t)));
	if (TL_table == (port_hash_bucket_t *) 0)
		panic("ipc_bootstrap: cannot create TL_table");

	for (i = 0; i < PORT_HASH_COUNT; i++) {
		queue_init(&TL_table[i].head);
		bucket_lock_init(&TL_table[i]);
	}

#if	MACH_IPC_STATS
	ipc_stats_init();
#endif	MACH_IPC_STATS

	object_copyin_init();
	object_copyout_init();
}

/*
 *	Routine:	ipc_init [exported]
 *	Purpose:
 *		Called to initialize remaining data structures before
 *		any user traps are handled.
 */
void
ipc_init()
{
	register notification_t *n;
	vm_offset_t min, max;
	extern int hz;

	/* Create a template for notification messages. */

	n = &notification_template;
	n->notify_header.msg_local_port = PORT_NULL;
	n->notify_header.msg_remote_port = PORT_NULL;
	n->notify_header.msg_simple = TRUE;
	n->notify_header.msg_type = MSG_TYPE_EMERGENCY;
	n->notify_header.msg_id = 0;
	n->notify_header.msg_size = sizeof(notification_t);

	n->notify_type.msg_type_name = MSG_TYPE_PORT_NAME;
	n->notify_type.msg_type_inline = TRUE;
	n->notify_type.msg_type_deallocate = FALSE;
	n->notify_type.msg_type_longform = FALSE;
	n->notify_type.msg_type_number = 1;
	n->notify_type.msg_type_size = sizeof(port_t) * NBBY;

	/* Create a template for complex_notification messages. */

	n = &complex_notification_template;
	n->notify_header.msg_local_port = PORT_NULL;
	n->notify_header.msg_remote_port = PORT_NULL;
	n->notify_header.msg_simple = FALSE;
	n->notify_header.msg_type = MSG_TYPE_EMERGENCY;
	n->notify_header.msg_id = 0;
	n->notify_header.msg_size = sizeof(notification_t);

	n->notify_type.msg_type_name = 0;
	n->notify_type.msg_type_inline = TRUE;
	n->notify_type.msg_type_deallocate = FALSE;
	n->notify_type.msg_type_longform = FALSE;
	n->notify_type.msg_type_number = 1;
	n->notify_type.msg_type_size = sizeof(port_t) * NBBY;

	/* Compute the timeout scaling factor. usecs per tick */

	timeout_scaling_factor = (1000000 / hz);

	/* Create a task used to hold rights and data in transit. */

	if (task_create(TASK_NULL /* kernel_task */, FALSE, &ipc_soft_task)
					!= KERN_SUCCESS)
		panic("ipc_init");

	ipc_kernel_map = kmem_csuballoc(kernel_map, &min, &max,
				       1024 * 1024, TRUE);

	kernel_task->ipc_privilege = TRUE;
	kernel_task->kernel_ipc_space = TRUE;

	ipc_host_init();
}
