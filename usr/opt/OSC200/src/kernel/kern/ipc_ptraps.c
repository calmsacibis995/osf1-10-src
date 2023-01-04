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
static char	*sccsid = "@(#)$RCSfile: ipc_ptraps.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:58:42 $";
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
 * File:	ipc_ptraps.c
 * Purpose:
 *	Task & thread port traps.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <sys/types.h>
#include <sys/secdefines.h>
#include <kern/host.h>
#include <mach/port.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/kern_port.h>
#include <kern/ipc_copyout.h>
#include <kern/ipc_ptraps.h>

#include <sys/param.h>
#include <sys/user.h>
#if SEC_BASE
#include <sys/security.h>
#endif

/*
 *	Routines:	task_self, task_notify, thread_self, thread_reply
 *			[exported, trap]
 *	Purpose:
 *		Primitive traps that provide the currently-executing
 *		task/thread with one of its ports.
 */

#define TASK_TRAP(trap_name, port_to_get)				\
port_name_t								\
trap_name()								\
{									\
	register task_t self = current_thread()->task;			\
	register kern_port_t port;					\
	port_name_t name;						\
									\
	ipc_task_lock(self);						\
	port = (kern_port_t) self->port_to_get;				\
	if (port != KERN_PORT_NULL)					\
		port_reference(port);					\
	ipc_task_unlock(self);						\
									\
	if (port != KERN_PORT_NULL)					\
		object_copyout(self, &port->port_obj,			\
			       MSG_TYPE_PORT, &name);			\
	else								\
		name = PORT_NULL;					\
									\
	return name;							\
}

TASK_TRAP(task_self, task_tself)
TASK_TRAP(task_notify, task_notify)

#define THREAD_TRAP(trap_name, port_to_get)				\
port_name_t								\
trap_name()								\
{									\
	register thread_t self = current_thread();			\
	register kern_port_t port;					\
	port_name_t name;						\
									\
	ipc_thread_lock(self);						\
	port = (kern_port_t) self->port_to_get;				\
	if (port != KERN_PORT_NULL)					\
		port_reference(port);					\
	ipc_thread_unlock(self);					\
									\
	if (port != KERN_PORT_NULL)					\
		object_copyout(self->task, &port->port_obj,		\
			       MSG_TYPE_PORT, &name);			\
	else								\
		name = PORT_NULL;					\
									\
	return name;							\
}

THREAD_TRAP(thread_self, thread_tself)
THREAD_TRAP(thread_reply, thread_reply)

#define	HOST_TRAP(trap_name, port_to_get)				\
port_name_t								\
trap_name()								\
{									\
	register task_t self = current_thread()->task;			\
	register kern_port_t port;					\
	port_name_t name;						\
									\
	port = (kern_port_t) realhost.port_to_get;			\
	if (port != KERN_PORT_NULL)					\
		port_reference(port);					\
									\
	if (port != KERN_PORT_NULL)					\
		object_copyout(self, &port->port_obj,			\
			       MSG_TYPE_PORT, &name);			\
	else								\
		name = PORT_NULL;					\
									\
	return name;							\
}

HOST_TRAP(host_self, host_self)

#if SEC_BASE

#define	HOST_PRIV_TRAP(trap_name, port_to_get)				\
port_name_t								\
trap_name()								\
{									\
	register task_t self = current_thread()->task;			\
	register kern_port_t port;					\
	port_name_t name;						\
									\
	if (!privileged(SEC_ALLOWDACACCESS, 0)) {			\
		name = PORT_NULL;					\
	}								\
	else {								\
		port = (kern_port_t) realhost.port_to_get;		\
		if (port != KERN_PORT_NULL)				\
			port_reference(port);				\
									\
		if (port != KERN_PORT_NULL)				\
			object_copyout(self, &port->port_obj,		\
			       MSG_TYPE_PORT, &name);			\
		else							\
			name = PORT_NULL;				\
	}								\
	return name;							\
}

#else	!SEC_BASE

#define	HOST_PRIV_TRAP(trap_name, port_to_get)				\
port_name_t								\
trap_name()								\
{									\
	register task_t self = current_thread()->task;			\
	register kern_port_t port;					\
	port_name_t name;						\
									\
	if (suser(u.u_cred, &u.u_acflag)) {				\
		name = PORT_NULL;					\
	}								\
	else {								\
		port = (kern_port_t) realhost.port_to_get;		\
		if (port != KERN_PORT_NULL)				\
			port_reference(port);				\
									\
		if (port != KERN_PORT_NULL)				\
			object_copyout(self, &port->port_obj,		\
			       MSG_TYPE_PORT, &name);			\
		else							\
			name = PORT_NULL;				\
	}								\
	return name;							\
}

#endif	!SEC_BASE

HOST_PRIV_TRAP(host_priv_self, host_priv_self)
