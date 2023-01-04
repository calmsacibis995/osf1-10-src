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
static char	*sccsid = "@(#)$RCSfile: ipc_copyin.c,v $ $Revision: 4.3.4.2 $ (DEC) $Date: 1992/04/06 15:18:04 $";
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
 * File:	ipc_copyin.c
 * Purpose:
 *	msg_copyin and related functions.
 */

#include <mach_ipc_stats.h>
#include <mach_ipc_xxxhack.h>

#include <mach/boolean.h>
#include <sys/types.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/kern_port.h>
#include <kern/kern_set.h>
#include <kern/kern_msg.h>
#include <kern/ipc_kmesg.h>
#include <kern/ipc_hash.h>
#include <kern/ipc_prims.h>
#include <kern/ipc_cache.h>
#include <kern/ipc_globals.h>
#include <mach_debug/ipc_statistics.h>
#include <kern/ipc_copyin.h>
#include <mach/vm_param.h>

/*
 *	Routine:	port_clear_receiver [internal]
 *	Purpose:
 *		Move receive rights out of a task.  The new receiver specifies
 *			ipc_soft_task:	Right is in a queued message.
 *			TASK_NULL:	Right is dead; ownership is
 *					 in a queued message.
 *	Conditions:
 *		The port must be locked.
 *	Side effects:
 *		The port is moved out of any port set it might be in.
 *		Threads waiting to receive on the port in question are
 *		awakened.
 */
void
port_clear_receiver(port, task)
	kern_port_t port;
	task_t task;
{
	kern_set_t set;

	assert((task == ipc_soft_task) || (task == TASK_NULL));

	set = port->port_set;
	if (set != KERN_SET_NULL) {
		/* No threads receiving on port, but must remove from set. */

		set_lock(set);
		set_remove_member(set, port);
		set_check_unlock(set);
	} else {
		/* Else, wake up all receivers, indicating why. */

		msg_queue_changed(&port->port_messages, RCV_INVALID_PORT);
	}

	port->port_receiver = task;
	port->port_receiver_name = PORT_NULL;
}

#if	MACH_IPC_XXXHACK
/*
 *	Routine:	port_copyin_receive [internal]
 *	Purpose:
 *		Copy-in receive rights for the port.
 *	Conditions:
 *		Task and port locked throughout.
 */
void
port_copyin_receive(task, port)
	task_t task;
	kern_port_t port;
{
	assert(port->port_receiver == task);

	port_clear_receiver(port, ipc_soft_task);
}

/*
 *	Routine:	port_copyin_own [internal]
 *	Purpose:
 *		Copy-in ownership rights for the port.
 *	Conditions:
 *		Task and port locked throughout.
 */
void
port_copyin_own(task, port)
	task_t task;
	kern_port_t port;
{
	assert(port->port_owner == task);

	port->port_owner = ipc_soft_task;
}
#endif	/* MACH_IPC_XXXHACK */

/*
 *	Routine:	port_copyin_receive_own [internal]
 *	Purpose:
 *		Copy-in receive/ownership rights for the port.
 *	Conditions:
 *		Task and port locked throughout.
 */
void
port_copyin_receive_own(task, port)
	task_t task;
	kern_port_t port;
{
#if	MACH_IPC_XXXHACK
	assert(port->port_owner == task);
	port->port_owner = ipc_soft_task;
#endif	

	assert(port->port_receiver == task);
	port_clear_receiver(port, ipc_soft_task);
}

/*
 *	Routine:	object_copyin
 *	Purpose:
 *		Copy the given "rights" held by "task" under local "name"
 *		for a message in transit.  If "deallocate" is specified,
 *		all rights to this object are removed from the "task" after
 *		the copy is made.
 *
 *		References to the object in question are made for the message
 *		in transit.  The value returned is the global name of the
 *		object.
 *
 *		Returns FALSE for illegal operations.
 *
 *	Conditions:
 *		No locks held on entry or exit.
 */
boolean_t
object_copyin(task, name, rights, deallocate, objp)
	task_t task;
	port_name_t name;
	unsigned int rights;
	boolean_t deallocate;
	kern_obj_t *objp;
{
	register kern_obj_t obj;
	register object_copyin_table_t *table;
	register port_hash_t entry;
	port_type_t type;

	assert(task != TASK_NULL);
	assert(! task->kernel_ipc_space);

	/* note name might be PORT_NULL, in which case we'll return FALSE */

	ipc_event(port_copyins);

	ipc_task_lock(task);
	if (!task->ipc_active) {
		ipc_task_unlock(task);
		return FALSE;
	}

	entry = obj_entry_lookup(task, name);
	if (entry == PORT_HASH_NULL) {
		ipc_task_unlock(task);
		return FALSE;
	}

	type = entry->type;
	assert(rights < MSG_TYPE_LAST);
	assert((0 <= type) && (type < PORT_TYPE_LAST));
	table = &object_copyin_table[rights][type];

	if (table->illegal || (deallocate && table->nodealloc)) {
		ipc_task_unlock(task);
		return FALSE;
	}

	obj = entry->obj;
	obj_lock(obj);

	if (!obj->obj_in_use) {
		obj_unlock(obj);
		ipc_task_unlock(task);
		return FALSE;
	}

	if (table->func)
		(*table->func)(task, obj);
	entry->type = table->result;

	/* if deallocation is desired or forced, remove the task's right */

	if (deallocate || table->dodealloc)
		obj_entry_destroy(task, obj, entry);

	ipc_task_unlock(task);

	/* take a reference for our caller */
	obj->obj_references++;
	obj_unlock(obj);

	*objp = obj;
	return TRUE;
}

/*
 *	Routine:	object_copyin_from_kernel [internal]
 *	Purpose:
 *		Hack to allow a kernel_ipc_space task to send rights
 *		in a message by using a pointer to the port,
 *		instead of a local name.
 *
 *		Returns FALSE for illegal operations.
 *	Conditions:
 *		Nothing locked.
 */
boolean_t
object_copyin_from_kernel(task, name, rights, deallocate, objp)
	register task_t task;
	port_name_t name;
	unsigned int rights;
	boolean_t deallocate;
	kern_obj_t *objp;
{
	register kern_obj_t obj = (kern_obj_t) name;
	register object_copyin_table_t *table;
	register port_hash_t entry;
	port_type_t type;

	assert(task != TASK_NULL);
	assert(obj != KERN_OBJ_NULL);
	assert(task->kernel_ipc_space);

	ipc_event(port_copyins);

	/* special case for MSG_TYPE_PORT: the task doesn't
	   even have to have rights for the port, if it doesn't
	   want to deallocate the rights. */

	if ((rights == MSG_TYPE_PORT) && !deallocate) {
		obj_reference(obj);
		return TRUE;
	}

	/* otherwise, the task actually has to have the
	   rights it is trying to send.  this code is like
	   object_copyin, except for the entry lookup. */

	ipc_task_lock(task);
	assert(task->ipc_active);

	entry = obj_entry_find(task, obj);
	if (entry == PORT_HASH_NULL) {
		ipc_task_unlock(task);
		return FALSE;
	}

	type = entry->type;
	assert(rights < MSG_TYPE_LAST);
	assert((0 <= type) && (type < PORT_TYPE_LAST));
	table = &object_copyin_table[rights][type];

	if (table->illegal || (deallocate && table->nodealloc)) {
		ipc_task_unlock(task);
		return FALSE;
	}

	assert(obj == entry->obj);
	obj_lock(obj);

	if (!obj->obj_in_use) {
		obj_unlock(obj);
		ipc_task_unlock(task);
		return FALSE;
	}

	if (table->func)
		(*table->func)(task, obj);
	entry->type = table->result;

	/* if deallocation is desired or forced, remove the task's right */

	if (deallocate || table->dodealloc)
		obj_entry_destroy(task, obj, entry);
	else
		assert(entry->type == PORT_TYPE_RECEIVE_OWN);

	ipc_task_unlock(task);

	/* take a reference for the message */
	obj->obj_references++;
	obj_unlock(obj);

	*objp = obj;
	return TRUE;
}

/*
 *	Routine:	object_copyin_cache [internal]
 *	Purpose:
 *		Copy given rights from a message header into the kernel.
 *		A "kernel_ipc_space" task is allowed to copy-in any port.
 *		If successful, the caller gets a ref for "*objp".
 *		Tries to cache the mapping.
 *	Conditions:
 *		The task is locked.
 */
boolean_t
object_copyin_cache(task, name, objp)
	task_t task;
	port_name_t name;
	kern_obj_t *objp;
{
	register port_hash_t entry;
	register kern_obj_t obj;

	assert(task != TASK_NULL);
	assert(name != PORT_NULL);

	ipc_event(port_copyins);

	if (!task->ipc_active)
		return FALSE;

	if (task->kernel_ipc_space) {
		obj = (kern_obj_t) name;
		obj_reference(obj);

		*objp = obj;
		return TRUE;
	}

	obj_entry_lookup_macro(task, name, entry, return FALSE;);

	if (!PORT_TYPE_IS_PORT(entry->type))
		return FALSE;

	obj = entry->obj;
	obj_reference(obj);
	obj_cache_set(task, name, obj);

	*objp = obj;
	return TRUE;
}

/*
 *	Routine:	msg_copyin
 *	Purpose:
 *		Copy a user message into a kernel message structure,
 *		returning that new kernel message in "var_kmsgptr".
 *
 *		Port rights are transferred as appropriate; the
 *		kernel message structure uses only global port names
 *		and holds port references.
 *
 *		Memory for in-transit messages is kept in one of two maps:
 *			ipc_kernel_map:	arrays of ports
 *			ipc_soft_map:	all else
 *	Conditions:
 *		No locks held on entry or exit.
 *	Errors:
 *		Error values appropriate to msg_send are returned; only
 *		for SEND_SUCCESS will a valid kernel message be returned.
 */
msg_return_t
msg_copyin(task, msgptr, msg_size, kmsgptr)
	register task_t task;
	msg_header_t *msgptr;
	msg_size_t msg_size;
	kern_msg_t *kmsgptr;
{
	register kern_msg_t kmsg;
	port_t lp, rp;
	kern_obj_t dest_port, reply_port;

	assert(task != TASK_NULL);

	if (msg_size < sizeof(msg_header_t))
		return SEND_MSG_TOO_SMALL;

	/* Allocate a new kernel message. */

	if (msg_size > (KERN_MSG_SMALL_SIZE -
			sizeof(struct kern_msg) +
			sizeof(msg_header_t))) {

		/* Enforce the maximum message size. */

		if (msg_size > MSG_SIZE_MAX)
			return SEND_MSG_TOO_LARGE;

		kern_msg_allocate_large(kmsg);
	} else
		kern_msg_allocate_small(kmsg);

	/* Copy in the message. */

	if (current_thread()->ipc_kernel) {
		bcopy((caddr_t) msgptr,
		      (caddr_t) &kmsg->kmsg_header,
		      msg_size);
	} else {
		if (copyin((caddr_t) msgptr,
			   (caddr_t) &kmsg->kmsg_header,
			   msg_size)) {
			kern_msg_free(kmsg);
			return SEND_INVALID_MEMORY;
		}
	}

#if	MACH_IPC_STATS
	ipc_event(messages);
	if (kmsg->kmsg_header.msg_type & MSG_TYPE_EMERGENCY)
		ipc_event(emergency);
#endif	

	/* Copied-in size may not be correct. */
	kmsg->kmsg_header.msg_size = msg_size;

	/* Translate ports in header from user's name to global name. */

	lp = kmsg->kmsg_header.msg_local_port;
	rp = kmsg->kmsg_header.msg_remote_port;

	ipc_task_lock(task);

	if (rp == PORT_NULL) {
		ipc_task_unlock(task);
		kern_msg_free(kmsg);
		return SEND_INVALID_PORT;
	} else {
		register kern_port_t _dest_port;

		obj_cache_copyin(task, rp, dest_port,
			{
				ipc_task_unlock(task);
				kern_msg_free(kmsg);
				return SEND_INVALID_PORT;
			});

		/*
		 *	Check whether this message is destined for the
		 *	kernel.
		 */

		_dest_port = (kern_port_t) dest_port;
		port_lock(_dest_port);
		if (_dest_port->port_receiver == kernel_task)
			kmsg->kernel_message = TRUE;
		port_unlock(_dest_port);
	}

	if (lp == PORT_NULL)
		reply_port = KERN_OBJ_NULL;
	else
		obj_cache_copyin(task, lp, reply_port,
			{
				ipc_task_unlock(task);
				obj_release(dest_port);
				kern_msg_free(kmsg);
				return SEND_INVALID_PORT;
			});

	ipc_task_unlock(task);

	kmsg->kmsg_header.msg_local_port  = (port_t) dest_port;
	kmsg->kmsg_header.msg_remote_port = (port_t) reply_port;

	if (! kmsg->kmsg_header.msg_simple) {
		register caddr_t saddr;
		caddr_t endaddr;
		boolean_t is_simple;
		boolean_t (*copyinf)();
		vm_map_t map = task->map;

		ipc_event(complex);
		is_simple = TRUE;

		if (task->kernel_ipc_space)
			copyinf = object_copyin_from_kernel;
		else
			copyinf = object_copyin;

		saddr = (caddr_t) (&kmsg->kmsg_header + 1);
		endaddr = (((caddr_t) &kmsg->kmsg_header) +
			   kmsg->kmsg_header.msg_size);

		while (saddr < endaddr) {
			unsigned int tn;
			unsigned long ts;
			register long elts;
			unsigned long numbytes;
			boolean_t is_port;

			kern_obj_t *port_list;
			register msg_type_long_t *tp =
				(msg_type_long_t *) saddr;
			boolean_t dealloc =
				tp->msg_type_header.msg_type_deallocate;
			caddr_t staddr = saddr;

			if (tp->msg_type_header.msg_type_longform) {
				tn = tp->msg_type_long_name;
				ts = tp->msg_type_long_size;
				elts = tp->msg_type_long_number;
				saddr += sizeof(msg_type_long_t);
			} else {
				tn = tp->msg_type_header.msg_type_name;
				ts = tp->msg_type_header.msg_type_size;
				elts = tp->msg_type_header.msg_type_number;
				saddr += sizeof(msg_type_t);
			}

			numbytes = ((elts * ts) + 7) >> 3;

			/*
			 *	Determine whether this item requires more
			 *	data than is left in the message body.
			 */

			if ((endaddr - saddr) <
			    (tp->msg_type_header.msg_type_inline ?
			     numbytes : sizeof(caddr_t))) {
				kmsg->kmsg_header.msg_size =
					staddr - (caddr_t) &kmsg->kmsg_header;
				msg_destroy(kmsg);
				return SEND_MSG_TOO_SMALL;
			}

			is_port = MSG_TYPE_PORT_ANY(tn);

			/*
			 *	Check that the size field for ports
			 *	is correct.  The code that examines
			 *	port_list relies on this.
			 */

			if (is_port && (ts != sizeof(port_t)*NBBY)) {
				kmsg->kmsg_header.msg_size =
					staddr - (caddr_t) &kmsg->kmsg_header;
				msg_destroy(kmsg);
				return SEND_INVALID_PORT;
			}
#ifdef	__alpha
			if (MSG_TYPE_ALIGN_64(tn,tp)) {
				/*
				 * saddr must be properly aligned
				 */
				saddr = (caddr_t)((vm_offset_t)(saddr+7)&(~7L));
			}
#endif

			/*
			 *	Copy data in from user's space, if
			 *	out-of-line, advance to the next item.
			 */

			if (tp->msg_type_header.msg_type_inline) {
				port_list = (kern_obj_t *)saddr;
				saddr += ((numbytes+3) & (~0x3)) ;
			} else {
				is_simple = FALSE;
				if (move_msg_data(kmsg, tn)) {
					vm_map_copy_t	copy_addr;

					if (vm_map_copyin(map,
						    * (vm_offset_t *) saddr,
						    (vm_size_t) numbytes, dealloc,
						    &copy_addr) != KERN_SUCCESS) {

					 BadVMCopy: ;

						kmsg->kmsg_header.msg_size =
							staddr - (caddr_t) &kmsg->kmsg_header;
						msg_destroy(kmsg);
						return SEND_INVALID_MEMORY;
					}

					/*
					 *	Must copy port names out to kernel VM so that
					 *	they can be translated.
					 */

					if (is_port) {
						if (vm_map_copyout(
							ipc_kernel_map,
							(vm_offset_t *) saddr,
							copy_addr)
						    != KERN_SUCCESS)
							goto BadVMCopy;
						port_list = * (kern_obj_t **) saddr;
					} else {
						*(vm_offset_t *)saddr = 
						    (vm_offset_t) copy_addr;
					}
				} else
					assert(!is_port);

				saddr += sizeof (caddr_t) ;
			}

			/*
			 *	Translate ports in this item from user's name
			 *	to global.
			 */

			if (is_port) {
				is_simple = FALSE;

				if ((elts > 1024) &&
				    (ipc_debug & IPC_DEBUG_1K_PORTS))
	uprintf("msg_copyin: passing more than 1K ports in one item!\n");

				while (--elts >= 0) {
					kern_obj_t obj = *port_list;
					port_name_t name = (port_name_t) obj;

					/*
					 *	Note that any of the
					 *	references to port_list
					 *	may encounter a page fault,
					 *	so those accesses are all
					 *	done here, rather than passing
					 *	an address to the port_list
					 *	cell to the copyin function.
					 */

					if ((name != PORT_NULL) &&
					    ! (*copyinf)(task, name,
							 tn, dealloc,
							 &obj)) {
						*port_list = KERN_OBJ_INVALID;
						msg_destroy(kmsg);
						return SEND_INVALID_PORT;
					}

					*port_list = obj;
					port_list++;
				}
			}
		}
		kmsg->kmsg_header.msg_simple = is_simple;
	}

	*kmsgptr = kmsg;
	return SEND_SUCCESS;
}
