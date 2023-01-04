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
static char	*sccsid = "@(#)$RCSfile: ipc_host.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:57:58 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	kern/ipc_host.c
 *
 *	Routines to implement host ports.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <sys/types.h>
#include <kern/host.h>
#include <kern/kern_port.h>
#include <mach/message.h>
#include <kern/port_object.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/ipc_host.h>

#include <machine/machparam.h>

/*
 *	ipc_host_init: set up various things.
 */

void ipc_host_init()
{
	kern_port_t	port;
	/*
	 *	Allocate and set up the two host ports.
	 */
	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_host_init: host port allocate");
	port->port_references++;
	port_unlock(port);
	realhost.host_self = (port_t) port;
	port_object_set(port, PORT_OBJECT_HOST, &realhost);

	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_host_init: host priv port allocate");
	port->port_references++;
	port_unlock(port);
	realhost.host_priv_self = (port_t) port;
	port_object_set(port, PORT_OBJECT_HOST_PRIV, &realhost);

	/*
	 *	Set up ipc for default processor set.
	 */
	ipc_pset_init(&default_pset);
	ipc_pset_enable(&default_pset);

	/*
	 *	And for master processor
	 */
	ipc_processor_init(master_processor);
	ipc_processor_enable(master_processor);
}
/*
 *	ipc_processor_init:
 *
 *	Initialize ipc access to processor by allocating port.
 */

void
ipc_processor_init(processor)
processor_t	processor;
{
	kern_port_t	port;

	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_processor_init: port allocate");
	port->port_references++;
	port_unlock(port);
	processor->processor_self = (pset_port_t) port;
}

/*
 *	ipc_processor_enable:
 *
 *	Enable ipc control of processor by setting port object.
 */
void
ipc_processor_enable(processor)
processor_t	processor;
{
	kern_port_t	myport;

	myport = (kern_port_t) processor->processor_self;
	port_lock(myport);
	port_object_set(myport, PORT_OBJECT_PROCESSOR, processor);
	port_unlock(myport);
}

/*
 *	ipc_processor_disable:
 *
 *	Disable ipc control of processor by clearing port object.
 */
void
ipc_processor_disable(processor)
processor_t	processor;
{
	kern_port_t	myport;

	if ((myport = (kern_port_t)processor->processor_self) ==
	    KERN_PORT_NULL) {
		return;
	}
	port_lock(myport);
	port_object_set(myport, PORT_OBJECT_NONE, 0);
	port_unlock(myport);
}
	
/*
 *	ipc_processor_terminate:
 *
 *	Processor is off-line.  Destroy ipc control port.
 */
void
ipc_processor_terminate(processor)
processor_t	processor;
{
	kern_port_t	myport;
	int	s;

	s = splsched();
	processor_lock(processor);
	if (processor->processor_self == PORT_NULL) {
		processor_unlock(processor);
		(void) splx(s);
		return;
	}

	myport = (kern_port_t) processor->processor_self;
	processor->processor_self = PORT_NULL;
	processor_unlock(processor);
	(void) splx(s);

	port_release(myport);
	(void)port_dealloc(kernel_task, myport);
}
	
/*
 *	ipc_pset_init:
 *
 *	Initialize ipc control of a processor set by allocating its ports.
 */

void
ipc_pset_init(pset)
processor_set_t	pset;
{
	kern_port_t	port;

	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_pset_init: pset port allocate");
	port->port_references++;
	port_unlock(port);
	pset->pset_self = (pset_port_t) port;

	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_pset_init: name port allocate");
	port->port_references++;
	port_unlock(port);
	pset->pset_name_self = (pset_port_t) port;
}

/*
 *	ipc_pset_enable:
 *
 *	Enable ipc access to a processor set.
 */
void
ipc_pset_enable(pset)
processor_set_t	pset;
{
	register kern_port_t	myport;

	pset_lock(pset);
	if (pset->active) {
		myport = (kern_port_t)pset->pset_self;
		port_lock(myport);
		port_object_set(myport, PORT_OBJECT_PSET, pset);
		port_unlock(myport);
		myport = (kern_port_t) pset->pset_name_self;
		port_lock(myport);
		port_object_set(myport, PORT_OBJECT_PSET_NAME, pset);
		port_unlock(myport);
		pset->ref_count += 2;
	}
	pset_unlock(pset);
}

/*
 *	ipc_pset_disable:
 *
 *	Disable ipc access to a processor set by clearing the port objects.
 *	Ok to just decrement ref_count because caller holds a reference.
 */
void
ipc_pset_disable(pset)
processor_set_t	pset;
{
	kern_port_t	myport;

	pset_lock(pset);
	myport = (kern_port_t) pset->pset_self;
	port_lock(myport);
	if (myport->port_object.kp_type == PORT_OBJECT_PSET) {
		port_object_set(myport, PORT_OBJECT_NONE, 0);
		pset->ref_count -= 1;
	}
	port_unlock(myport);
	myport = (kern_port_t) pset->pset_name_self;
	port_lock(myport);
	if (myport->port_object.kp_type == PORT_OBJECT_PSET_NAME) {
		port_object_set(myport, PORT_OBJECT_NONE, 0);
		pset->ref_count -= 1;
	}
	port_unlock(myport);
	pset_unlock(pset);
}

/*
 *	ipc_pset_terminate:
 *
 *	Processor set is dead.  Deallocate the ipc control structures.
 */
void
ipc_pset_terminate(pset)
processor_set_t	pset;
{

	port_release((kern_port_t) pset->pset_self);
	port_release((kern_port_t) pset->pset_name_self);
	port_dealloc(kernel_task, (kern_port_t) pset->pset_self);
	port_dealloc(kernel_task, (kern_port_t) pset->pset_name_self);
}

/*
 *	processor_set_default, processor_set_default_priv:
 *
 *	Return ports for manipulating default_processor set.  MiG code
 *	differentiates between these two routines.
 */
kern_return_t
processor_set_default(host, pset)
host_t	host;
processor_set_t	*pset;
{

	if (host == HOST_NULL)
		return(KERN_INVALID_ARGUMENT);

	*pset = &default_pset;
	return(KERN_SUCCESS);
}

kern_return_t
xxx_processor_set_default_priv(host, pset)
host_t	host;
processor_set_t	*pset;
{

	if (host == HOST_NULL)
		return(KERN_INVALID_ARGUMENT);

	*pset = &default_pset;
	return(KERN_SUCCESS);
}
