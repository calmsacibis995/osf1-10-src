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
static char	*sccsid = "@(#)$RCSfile: ipc_pobj.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/04/06 15:18:45 $";
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
 * File:	ipc_pobj.c
 * Purpose:
 *	Port object functions; task and thread conversion functions.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <mach_km.h>

#include <sys/types.h>
#include <kern/host.h>
#include <kern/kern_port.h>
#include <kern/port_object.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <vm/vm_map.h>
#include <kern/ipc_pobj.h>

#if	MACH_KM
#include <kern/kern_mon.h>
#endif	MACH_KM

#include <kern/macro_help.h>

#include <machine/machparam.h>

/*
 *	Port object set/get routines. [exported]
 *
 *	Each port may have an "object" associated with it, to provide
 *	quick lookup of that object for the kernel.
 */

void
port_object_set(port, type, object)
	port_t port;
	port_object_type_t type;
	int object;
{
#define port_object_set(port, type, object) 			\
	MACRO_BEGIN						\
	register kern_port_t _kp = (kern_port_t)(port);		\
	_kp->port_object.kp_type = (type);			\
	_kp->port_object.kp_object = (object);			\
	MACRO_END

	port_object_set(port, type, object);
}

int
port_object_get(port)
	port_t port;
{
#define port_object_get(port)	(((kern_port_t)(port))->port_object.kp_object)

	return port_object_get(port);
}

port_object_type_t
port_object_type(port)
	port_t		port;
{
#define port_object_type(port)	(((kern_port_t)(port))->port_object.kp_type)

	return port_object_type(port);
}

/*
 *	Conversion routines, from port to task/thread/map. [exported]
 */

task_t
convert_port_to_task(_port)
	port_t		_port;
{
	register task_t task;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return TASK_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_TASK) {
		task = (task_t) port_object_get(port);
		task_reference(task);
	} else
		task = TASK_NULL;

	port_unlock(port);
	return task;
}

thread_t
convert_port_to_thread(_port)
	port_t		_port;
{
	register thread_t th;
	kern_port_t	port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return THREAD_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_THREAD) {
		th = (thread_t) port_object_get(port);
		thread_reference(th);
	} else
		th = THREAD_NULL;

	port_unlock(port);
	return th;
}

#if	MACH_KM

/*
 *  	convert_port_to_monitor():
 *
 *	Used by kernel monitor calls.
 */
monitor_t	convert_port_to_monitor(_port)
	port_t			_port;
{
	register monitor_t	m;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return MONITOR_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_MONITOR) {
		m = (monitor_t) port_object_get(port);
		monitor_reference(m);
	}
	else {
		m = MONITOR_NULL;
	}

	port_unlock(port);
	return m;
}
#endif	MACH_KM


vm_map_t
convert_port_to_map(port)
	port_t		port;
{
	register task_t task;
	register vm_map_t map;

	task = convert_port_to_task(port);

	if (task == TASK_NULL)
		return VM_MAP_NULL;

	map = task->map;
	vm_map_reference(map);

	task_deallocate(task);
	return map;
}

port_t
convert_task_to_port(task)
	register task_t task;
{
	register kern_port_t result;

	ipc_task_lock(task);
	if (task->ipc_active) {
		result = (kern_port_t) task->task_self;
		if (result != KERN_PORT_NULL)
		   	port_reference(result);
	} else
		result = KERN_PORT_NULL;

	ipc_task_unlock(task);
	return (port_t) result;
}

port_t
convert_thread_to_port(thread)
	register thread_t thread;
{
	register kern_port_t result;

	ipc_thread_lock(thread);

	result = (kern_port_t) thread->thread_self;
	if (result != KERN_PORT_NULL)
	   	port_reference(result);

	ipc_thread_unlock(thread);
	return (port_t) result;
}


/*
 *	Conversion routines for host/processors/pset.
 */


host_t
convert_port_to_host(_port)
	port_t _port;
{
	register host_t host;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return HOST_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_HOST)
		host = (host_t) port_object_get(port);
	else
		host = HOST_NULL;

	port_unlock(port);
	return host;
}

host_t
convert_port_to_host_priv(_port)
	port_t _port;
{
	register host_t host;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return HOST_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_HOST_PRIV) {
		host = (host_t) port_object_get(port);
	} else
		host = HOST_NULL;

	port_unlock(port);
	return host;
}

processor_t
convert_port_to_processor(_port)
	port_t _port;
{
	register processor_t processor;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return PROCESSOR_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_PROCESSOR)
		processor = (processor_t) port_object_get(port);
	else
		processor = PROCESSOR_NULL;

	port_unlock(port);
	return processor;
}

processor_set_t
convert_port_to_pset(_port)
	port_t _port;
{
	register processor_set_t pset;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return PROCESSOR_SET_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_PSET) {
		pset = (processor_set_t) port_object_get(port);
		pset_reference(pset);
	} else
		pset = PROCESSOR_SET_NULL;

	port_unlock(port);
	return pset;
}

processor_set_t
convert_port_to_pset_name(_port)
	port_t _port;
{
	register processor_set_t pset;
	kern_port_t port = (kern_port_t) _port;

	if (port == KERN_PORT_NULL)
		return PROCESSOR_SET_NULL;

	port_lock(port);

	if (port_object_type(port) == PORT_OBJECT_PSET_NAME) {
		pset = (processor_set_t) port_object_get(port);
		pset_reference(pset);
	} else
		pset = PROCESSOR_SET_NULL;

	port_unlock(port);
	return pset;
}

port_t
convert_host_to_port(host)
	register host_t host;
{
	register kern_port_t result;

	result = (kern_port_t) host->host_self;
	if (result != KERN_PORT_NULL)
	   	port_reference(result);

	return (port_t) result;
}

port_t
convert_processor_to_port(processor)
register processor_t processor;
{
	register kern_port_t result;
	int	s;

	s = splsched();
	processor_lock(processor);

	result = (kern_port_t) processor->processor_self;
	if (result != KERN_PORT_NULL)
	   	port_reference(result);

	processor_unlock(processor);
	(void) splx(s);
	return (port_t) result;
}

port_t
convert_pset_to_port(pset)
	register processor_set_t pset;
{
	register kern_port_t result;

	pset_lock(pset);
	if (pset->active) {
		result = (kern_port_t) pset->pset_self;
		if (result != KERN_PORT_NULL)
		   	port_reference(result);
	}
	else
		result = KERN_PORT_NULL;
	pset_unlock(pset);
	return (port_t) result;
}

port_t
convert_pset_name_to_port(pset)
	register processor_set_t pset;
{
	register kern_port_t result;

	pset_lock(pset);
	if (pset->active) {
		result = (kern_port_t) pset->pset_name_self;
		if (result != KERN_PORT_NULL)
		   	port_reference(result);
	}
	else
		result = KERN_PORT_NULL;
	pset_unlock(pset);
	return (port_t) result;
}

#if	MACH_KM

/*
 *  	convert_monitor_to_port():
 *
 *	Used by kernel monitor calls.
 */
port_t		convert_monitor_to_port(m)
	register monitor_t	m;
{
	register kern_port_t	result;

    	/*
     	 * Check to see if the monitor is a valid monitor.
    	 */
    	if (m == MONITOR_NULL)
    	{
      	    result = KERN_PORT_NULL;
      	    return (port_t) result;
	}

	monitor_lock(m);
	if ((result = (kern_port_t) m->monitor_self) != KERN_PORT_NULL ) {
	   	port_reference(result);
	}
	monitor_unlock(m);
    	return (port_t) result;

}

#endif	MACH_KM
