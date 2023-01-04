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
static char	*sccsid = "@(#)$RCSfile: ipc_mports.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:58:16 $";
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
 * File:	ipc_mports.c
 * Purpose:
 *	Code for mach_ports_* calls.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <mach/kern_return.h>
#include <mach/port.h>
#include <kern/task.h>
#include <kern/kern_port.h>
#include <mach/vm_param.h>
#include <kern/ipc_globals.h>
#include <kern/ipc_mports.h>

/*
 *	Routine:	mach_ports_register [exported, user]
 *	Purpose:
 *		Record the given list of ports in the target_task,
 *		to be passed down to its children.
 *
 *		Provides a means of getting valuable system-wide
 *		ports to future generations without using initial
 *		messages or other conventions that non-Mach applications
 *		might well ignore.
 *
 *		Children do not automatically acquire rights to these
 *		ports -- they must use mach_ports_lookup to pick them up.
 */
kern_return_t
mach_ports_register(target_task, init_port_set, init_port_set_Cnt)
	task_t target_task;
	task_port_t *init_port_set;
	unsigned int init_port_set_Cnt;
{
	task_port_t list[TASK_PORT_REGISTER_MAX];
	int i;
	kern_return_t result = KERN_INVALID_ARGUMENT;

	/* Verify that there weren't too many passed to us. */

	if (init_port_set_Cnt <= TASK_PORT_REGISTER_MAX) {

		/* Copy them into a local list, to avoid faulting. */

		for (i = 0; i < init_port_set_Cnt; i++)
			list[i] = init_port_set[i];
		for (; i < TASK_PORT_REGISTER_MAX; i++)
			list[i] = PORT_NULL;

		/* Mark each port as registered by this task. */

		ipc_task_lock(target_task);
		if (target_task->ipc_active) {
			for (i = 0; i < TASK_PORT_REGISTER_MAX; i++) {
				if (target_task->ipc_ports_registered[i] !=
						PORT_NULL)
					port_release((kern_port_t) target_task->
						ipc_ports_registered[i]);
				if (list[i] != PORT_NULL)
					port_reference((kern_port_t) list[i]);
				target_task->ipc_ports_registered[i] =
					list[i];
			}
			result = KERN_SUCCESS;
		}
		ipc_task_unlock(target_task);
	}

	return result;
}

/*
 *	Routine:	mach_ports_lookup [exported, user]
 *	Purpose:
 *		Retrieve the ports for the target task that were
 *		established by mach_ports_register (or by inheritance
 *		of those registered in an ancestor, if no mach_ports_register
 *		has been done explicitly on this task).
 */
kern_return_t
mach_ports_lookup(target_task, init_port_set, init_port_set_Cnt)
	task_t target_task;
	task_port_t **init_port_set;
	unsigned int *init_port_set_Cnt;
{
	kern_return_t result;
	task_port_t list[TASK_PORT_REGISTER_MAX];
	vm_size_t list_size = sizeof(list);

	*init_port_set = (task_port_t *) 0;
	*init_port_set_Cnt = 0;

	result = vm_allocate(ipc_kernel_map,
			     (vm_offset_t *) init_port_set, list_size,
			     TRUE);
	if (result == KERN_SUCCESS) {
		int i;

		/*
		 *	Copy the registered port list out right away
		 *	to avoid faulting while holding the lock.
		 */

		ipc_task_lock(target_task);
			for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
				if ((list[i] = target_task->ipc_active ?
					target_task->ipc_ports_registered[i] :
					PORT_NULL) != PORT_NULL)
						port_reference((kern_port_t) list[i]);
		ipc_task_unlock(target_task);

		/* Copy the ports into the array to return. */

		for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
			(*init_port_set)[i] = list[i];
		*init_port_set_Cnt = TASK_PORT_REGISTER_MAX;
	}

	/*
	 *	Note that this may return a barely-usable error code
	 *	if one of the vm_* calls fail.
	 */

	return result;
}
