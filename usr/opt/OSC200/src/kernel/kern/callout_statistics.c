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
static char	*sccsid = "@(#)$RCSfile: callout_statistics.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:23:09 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * File:	callout_statistics.c
 * Purpose:
 *	Code for call-out statistics gathering.
 */

#include <mach_debug.h>
#include <mach_co_info.h>
#include <mach_co_stats.h>

#include <mach/kern_return.h>
#include <mach/vm_param.h>
#include <kern/task.h>
#include <kern/lock.h>
#include <sys/callout.h>
#include <machine/machparam.h>		/* for splhigh */
#include <vm/vm_map.h>
#include <mach/vm_prot.h>
#include <kern/ipc_globals.h>
#include <mach_debug/callout_statistics.h>
#include <vm/vm_kern.h>

#if	MACH_DEBUG && MACH_CO_INFO
/*
 *	Routine:	host_callout_info [exported, user]
 *	Purpose:
 *		Return the contents of the call-out queue.
 */
kern_return_t
host_callout_info(target_task, infoArray, infoCnt)
	task_t target_task;
	callout_info_array_t *infoArray;
	unsigned int *infoCnt;
{
	unsigned int actual;	/* this many queue elements */
	unsigned int space;	/* space for this many elements */

	vm_size_t size;
	vm_offset_t addr;

#ifdef	lint
	target_task++;
#endif	lint

	/* initial guess for amount of memory to allocate */
	size = page_size;

	for (;;) {
		register callout_info_t *info;
		register struct callout *event;
		int s;

		/* allocate memory non-pageable, so don't fault
		   while holding locks */
		(void) vm_allocate(ipc_kernel_map, &addr, size, TRUE);
		(void) vm_map_pageable(ipc_kernel_map, addr, addr + size,
				VM_PROT_READ|VM_PROT_WRITE);

		info = (callout_info_t *) addr;
		space = size / sizeof(callout_info_t);
		actual = 0;

		s = splhigh();
		simple_lock(&callout_lock);

		for (event = calltodo.c_next;
		     (actual < space) && (event != 0);
		     actual++, event = event->c_next) {
			info[actual].coi_time = event->c_time;
			info[actual].coi_arg = event->c_arg;
			info[actual].coi_func = event->c_func;
		}

		simple_unlock(&callout_lock);
		splx(s);

		if (event == 0)
			break;

		/* free current memory block */
		(void) kmem_free(ipc_kernel_map, addr, size);

		/* go for another try, allowing for expansion */
		size = round_page(2 * actual * sizeof(callout_info_t));
	}

	if (actual == 0) {
		/* no members, so return null pointer and deallocate memory */
		*infoArray = 0;
		*infoCnt = 0;

		(void) kmem_free(ipc_kernel_map, addr, size);
	} else {
		vm_size_t size_used;
		vm_map_copy_t copied_in_info;

		*infoCnt = actual;

		size_used = round_page(actual * sizeof(callout_info_t));

		/* finished touching it, so make the memory pageable */
		(void) vm_map_pageable(ipc_kernel_map,
				       addr, addr + size_used, VM_PROT_NONE);

		/* the memory needs to be in copied-in form */
		(void) vm_map_copyin(ipc_kernel_map, addr, size_used, TRUE,
				&copied_in_info);
		*infoArray = (callout_info_array_t) copied_in_info;

		/* free any unused memory */
		if (size_used != size)
			kmem_free(
				ipc_kernel_map,
				addr + size_used,
				size - size_used);
	}

	return KERN_SUCCESS;
}
#endif	MACH_DEBUG && MACH_CO_INFO

#if	MACH_CO_STATS
callout_statistics_t callout_statistics;
int callout_statistics_fudge;

#if	MACH_DEBUG
/*
 *	Routine:	host_callout_statistics [exported, user]
 *	Purpose:
 *		Return the accumulated call-out statistics.
 */
kern_return_t
host_callout_statistics(target_task, statistics)
	task_t target_task;
	callout_statistics_t *statistics;
{
	register int s;

	s = splhigh();
	simple_lock(&callout_lock);

	*statistics = callout_statistics;

	simple_unlock(&callout_lock);
	splx(s);

	return KERN_SUCCESS;
}

/*
 *	Routine:	host_callout_statistics_reset [exported, user]
 *	Purpose:
 *		Reset the accumulated call-out statistics.
 */
kern_return_t
host_callout_statistics_reset(target_task)
	task_t target_task;
{
	register int s;

	s = splhigh();
	simple_lock(&callout_lock);

	callout_statistics_fudge = -callout_statistics.cos_current_size;
	callout_statistics.cos_num_timeout = 0;
	callout_statistics.cos_cum_timeout_size = 0;
	callout_statistics.cos_cum_timeout_pos = 0;

	callout_statistics.cos_num_untimeout = 0;
	callout_statistics.cos_num_untimeout_hit = 0;
	callout_statistics.cos_cum_untimeout_size = 0;
	callout_statistics.cos_cum_untimeout_pos = 0;

	callout_statistics.cos_num_softclock = 0;
	callout_statistics.cos_cum_softclock_size = 0;

	simple_unlock(&callout_lock);
	splx(s);

	return KERN_SUCCESS;
}
#endif	MACH_DEBUG
#endif	MACH_CO_STATS
