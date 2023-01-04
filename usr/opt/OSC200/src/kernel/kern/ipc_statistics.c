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
static char	*sccsid = "@(#)$RCSfile: ipc_statistics.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:05 $";
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
 * File:	ipc_statistics.c
 * Purpose:
 *	Code for IPC statistics gathering.
 */

#include <mach_debug.h>

#include <mach/vm_param.h>
#include <mach/kern_return.h>
#include <mach_debug/ipc_statistics.h>
#include <kern/task.h>
#include <kern/lock.h>

decl_simple_lock_data(,ipc_statistics_lock_data)
ipc_statistics_t ipc_statistics;

/*
 *	Routine:	ipc_stats_init [exported]
 *	Purpose:
 *		Initialize Mach IPC statistics gathering.
 */
void
ipc_stats_init()
{
	simple_lock_init(&ipc_statistics_lock_data);

	ipc_statistics.version = 77;
	ipc_statistics.messages = 0;
	ipc_statistics.complex = 0;
	ipc_statistics.kernel = 0;
	ipc_statistics.large = 0;
	ipc_statistics.current = 0;
	ipc_statistics.emergency = 0;
	ipc_statistics.notifications = 0;
	ipc_statistics.port_copyins = 0;
	ipc_statistics.port_copyouts = 0;
	ipc_statistics.port_copyin_hits = 0;
	ipc_statistics.port_copyin_miss = 0;
	ipc_statistics.port_allocations = 0;
	ipc_statistics.bucket_misses = 0;
	ipc_statistics.ip_data_grams = 0;
}

#if	MACH_DEBUG
#include <kern/ipc_hash.h>
#include <kern/ipc_globals.h>

/*
 *	Routine:	host_ipc_statistics [exported, user]
 *	Purpose:
 *		Return the accumulated IPC statistics.
 */
kern_return_t
host_ipc_statistics(task, statistics)
	task_t task;
	ipc_statistics_t *statistics;
{
	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	ipc_statistics_lock();
	*statistics = ipc_statistics;
	ipc_statistics_unlock();

	return KERN_SUCCESS;
}

/*
 *	Routine:	host_ipc_statistics_reset [exported, user]
 *	Purpose:
 *		Reset the accumulated IPC statistics.
 */
kern_return_t
host_ipc_statistics_reset(task)
	task_t task;
{
	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	ipc_statistics_lock();
	ipc_statistics.messages = 0;
	ipc_statistics.complex = 0;
	ipc_statistics.kernel = 0;
	ipc_statistics.large = 0;
	ipc_statistics.emergency = 0;
	ipc_statistics.notifications = 0;
	ipc_statistics.port_copyins = 0;
	ipc_statistics.port_copyouts = 0;
	ipc_statistics.port_copyin_hits = 0;
	ipc_statistics.port_copyin_miss = 0;
	ipc_statistics.port_allocations = 0;
	ipc_statistics.bucket_misses = 0;
	ipc_statistics.ip_data_grams = 0;
	ipc_statistics_unlock();

	return KERN_SUCCESS;
}

/*
 *	Routine:	host_ipc_bucket_sizes
 *	Purpose:
 *		Return the number of translations in each IPC bucket.
 */
kern_return_t
host_ipc_bucket_info(task, TLinfo, TLinfoCnt, TPinfo, TPinfoCnt)
	task_t task;
	ipc_bucket_info_array_t *TLinfo;
	unsigned int *TLinfoCnt;
	ipc_bucket_info_array_t *TPinfo;
	unsigned int *TPinfoCnt;
{
	vm_offset_t addr1, addr2;
	vm_size_t size;
	int num_buckets = PORT_HASH_COUNT;
	ipc_bucket_info_t *tlinfo, *tpinfo;
	int i;
	kern_return_t kr;

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	size = round_page(num_buckets * sizeof(ipc_bucket_info_t));

	/*
	 *	Allocate memory in the ipc_kernel_map, because
	 *	we need to touch it, and then move it to the ipc_soft_map
	 *	(where the IPC code expects to find it) when we're done.
	 *
	 *	Because we don't touch the memory with any locks held,
	 *	it can be left pageable.
	 */

	kr = vm_allocate(ipc_kernel_map, &addr1, size, TRUE);
	if (kr != KERN_SUCCESS)
		panic("host_ipc_bucket_info: vm_allocate");

	kr = vm_allocate(ipc_kernel_map, &addr2, size, TRUE);
	if (kr != KERN_SUCCESS)
		panic("host_ipc_bucket_info: vm_allocate");

	tlinfo = (ipc_bucket_info_t *) addr1;
	tpinfo = (ipc_bucket_info_t *) addr2;

	for (i = 0; i < num_buckets; i++) {
		register port_hash_bucket_t *tl = &TL_table[i];
		register port_hash_bucket_t *tp = &TP_table[i];
		register port_hash_t entry;
		int count;

		count = 0;
		bucket_lock(tl);
		for (entry = (port_hash_t) queue_first(&tl->head);
		     !queue_end(&tl->head, (queue_entry_t) entry);
		     entry = (port_hash_t) queue_next(&entry->TL_chain))
			count++;
		bucket_unlock(tl);
		tlinfo++->count = count;

		count = 0;
		bucket_lock(tp);
		for (entry = (port_hash_t) queue_first(&tp->head);
		     !queue_end(&tp->head, (queue_entry_t) entry);
		     entry = (port_hash_t) queue_next(&entry->TP_chain))
			count++;
		bucket_unlock(tp);
		tpinfo++->count = count;
	}

	/*
	 *	Move memory to ipc_soft_map.
	 */

	kr = vm_map_copyin(ipc_kernel_map, addr1,
		     size, TRUE,
		     (vm_map_copy_t *) TLinfo);
	if (kr != KERN_SUCCESS)
		panic("host_ipc_bucket_info: vm_map_copyin");

	kr = vm_map_copyin(ipc_kernel_map, addr2,
		     size, TRUE,
		     (vm_map_copy_t *) TPinfo);
	if (kr != KERN_SUCCESS)
		panic("host_ipc_bucket_info: vm_map_copyin");

	*TLinfoCnt = num_buckets;
	*TPinfoCnt = num_buckets;

	return KERN_SUCCESS;
}

#endif	MACH_DEBUG
