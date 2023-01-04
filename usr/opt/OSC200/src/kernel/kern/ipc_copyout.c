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
static char	*sccsid = "@(#)$RCSfile: ipc_copyout.c,v $ $Revision: 4.3.10.2 $ (DEC) $Date: 1993/11/23 16:32:07 $";
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
 * File:	ipc_copyout.c
 * Purpose:
 *	msg_copyout and related functions.
 */

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
#include <kern/ipc_copyout.h>

#if	MACH_IPC_XXXHACK
/*
 *	Routine:	port_destroy_receive [internal]
 *	Purpose:
 *		Destroy receive rights that were in transit.
 *
 *		Allow for possibility that the receive rights are
 *		no longer in the message.
 *	Conditions:
 *		The port is locked; we have a ref to release.
 */
void
port_destroy_receive(port)
	kern_port_t port;
{
	if (port->port_receiver == ipc_soft_task)
		port->port_receiver = TASK_NULL;

	if ((port->port_receiver == TASK_NULL) &&
	    (port->port_owner == TASK_NULL))
		port_destroy(port);
	else {
		port->port_references--;
		/* No need for port_check_unlock, because a receiver or
		   owner exists, so there is some other outstanding ref. */
		port_unlock(port);
	}
}

/*
 *	Routine:	port_destroy_own [internal]
 *	Purpose:
 *		Destroy ownership rights that were in transit.
 *
 *		Allow for possibility that the ownership rights are
 *		no longer in the message.
 *	Conditions:
 *		The port is locked; we have a ref to release.
 */
void
port_destroy_own(port)
	kern_port_t port;
{
	if (port->port_owner == ipc_soft_task)
		port->port_owner = TASK_NULL;

	if ((port->port_receiver == TASK_NULL) &&
	    (port->port_owner == TASK_NULL))
		port_destroy(port);
	else {
		port->port_references--;
		/* No need for port_check_unlock, because a receiver or
		   owner exists, so there is some other outstanding ref. */
		port_unlock(port);
	}
}
#endif	/* MACH_IPC_XXXHACK */

/*
 *	Routine:	port_destroy_receive_own [internal]
 *	Purpose:
 *		Destroy receive/ownership rights that were in transit.
 *
 *		Allow for possibility that the receive/ownership rights are
 *		no longer in the message.
 *	Conditions:
 *		The port is locked; we have a ref to release.
 */
void
port_destroy_receive_own(port)
	kern_port_t port;
{
	if (port->port_receiver == ipc_soft_task)
		port->port_receiver = TASK_NULL;
#if	MACH_IPC_XXXHACK
	if (port->port_owner == ipc_soft_task)
		port->port_owner = TASK_NULL;
#endif	

#if	MACH_IPC_XXXHACK
	if ((port->port_receiver == TASK_NULL) &&
	    (port->port_owner == TASK_NULL))
#else
	if (port->port_receiver == TASK_NULL)
#endif	
		port_destroy(port);
	else {
		port->port_references--;
		/* No need for port_check_unlock, because a receiver or
		   owner exists, so there is some other outstanding ref. */
		port_unlock(port);
	}
}

#if	MACH_IPC_XXXHACK
/*
 *	Routine:	port_copyout_receive [internal]
 *	Purpose:
 *		Transfer receive rights from a message in transit
 *		to the task in question.
 *	Conditions:
 *		The port is locked throughout.
 */
int
port_copyout_receive(task, port, name)
	task_t task;
	kern_port_t port;
	port_name_t name;
{
	assert(port->port_receiver == ipc_soft_task);
	port->port_receiver = task;
	port->port_receiver_name = name;

	if (port->port_owner == TASK_NULL) {
		/* if ownership rights were dead, this task gets them too,
		   and a notification gets sent. */
		port->port_owner = task;
		return NOTIFY_OWNERSHIP_RIGHTS;
	}

	return 0;
}

/*
 *	Routine:	port_copyout_own [internal]
 *	Purpose:
 *		Transfer ownership rights from a message in transit
 *		to the task in question.
 *	Conditions:
 *		The port is locked throughout.
 */
int
port_copyout_own(task, port, name)
	task_t task;
	kern_port_t port;
	port_name_t name;
{
#ifdef	lint
	name++;
#endif	

	assert(port->port_owner == ipc_soft_task);
	port->port_owner = task;

	if (port->port_receiver == TASK_NULL) {
		/* if receive rights were dead, this task gets them too,
		   and a notification gets sent. */
		port->port_receiver = task;
		return NOTIFY_RECEIVE_RIGHTS;
	}

	return 0;
}
#endif	/* MACH_IPC_XXXHACK */

/*
 *	Routine:	port_copyout_receive_own [internal]
 *	Purpose:
 *		Transfer receive/ownership rights from a message in transit
 *		to the task in question.
 *	Conditions:
 *		The port is locked throughout.
 */
int
port_copyout_receive_own(task, port, name)
	task_t task;
	kern_port_t port;
	port_name_t name;
{
	assert(port->port_receiver == ipc_soft_task);
	port->port_receiver = task;
	port->port_receiver_name = name;

#if	MACH_IPC_XXXHACK
	assert(port->port_owner == ipc_soft_task);
	port->port_owner = task;
#endif	

	return 0;
}

/*
 *	Routine:	object_destroy [internal]
 *	Purpose:
 *		Delete the rights denoted by "rights" for the object "obj"
 *		from a message in transit.
 *	Conditions:
 *		No locks held on entry or exit.
 *		Releases an object reference (in the message); may destroy
 *		the object.
 */
void
object_destroy(obj, rights)
	kern_obj_t obj;
	unsigned int rights;
{
	register object_copyout_table_t *table;

	assert(obj != KERN_OBJ_NULL);

	ipc_event(port_copyouts);

	assert(rights < MSG_TYPE_LAST);
	table = &object_copyout_table[rights];

	obj_lock(obj);
	if (table->destroy && obj->obj_in_use)
		/* consumes ref & unlocks obj */
		(*table->destroy)(obj);
	else {
		obj->obj_references--;
		obj_check_unlock(obj);
	}
}

/*
 *	Routine:	object_copyout [internal]
 *	Purpose:
 *		Transfer the rights denoted by "rights" for the object "obj"
 *		from a message in transit to the task in question.
 *		Return the task's local name in "*namep".
 *	Conditions:
 *		No locks held on entry or exit.
 *		Releases an object reference (in the message); may
 *		add a reference for a new right.
 */
void
object_copyout(task, obj, rights, namep)
	task_t task;
	kern_obj_t obj;
	unsigned int rights;
	port_name_t *namep;
{
	register object_copyout_table_t *table;
	port_hash_t entry;
	port_name_t name;
	port_type_t type;
	int msg_id;

	assert(task != TASK_NULL);
	assert(obj != KERN_OBJ_NULL);

	ipc_task_lock(task);
	obj_lock(obj);

	assert(rights < MSG_TYPE_LAST);
	table = &object_copyout_table[rights];

	entry = obj_entry_make(task, obj, table->nomerge);
	if (entry == PORT_HASH_NULL) {
		/* this is a rare case, so don't mind unlocking and
		   letting object_destroy do all the work.  it will
		   consume our ref. */

		obj_unlock(obj);
		ipc_task_unlock(task);
		object_destroy(obj, rights);
		*namep = PORT_NULL;
		return;
	}

	type = entry->type;
	*namep = name = entry->local_name;
	msg_id = 0;

	/* need to have both task & object locked while
	   manipulating the entry. */

	assert((0 <= type) && (type < PORT_TYPE_LAST));
	entry->type = table->result[type];
	assert(entry->type != PORT_TYPE_NONE);

	/* Would drop task lock here, except for hack below. */

	if (table->func) {
		msg_id = (*table->func)(task, obj, name);

		/* Hack to take care of way notification
		   generation should change the entry's type. */
		if (msg_id != 0)
			entry->type = PORT_TYPE_RECEIVE_OWN;
	}

	/* There must be some other reference, for a translation,
	   so no need for obj_check_unlock. */
	obj->obj_references--;
	obj_unlock(obj);

	ipc_task_unlock(task);

	if (msg_id != 0)
		send_notification(task, msg_id, name);

	ipc_event(port_copyouts);
}

/*
 *	Routine:	msg_copyout
 *	Purpose:
 *		Transfer the specified kernel message to user space.
 *		Includes transferring port rights and out-of-line memory
 *		present in the message.
 *	Conditions:
 *		No locks held on entry or exit.
 *		The kernel message is destroyed.
 *	Returns:
 *		Values returned correspond to those for msg_receive.
 */		
msg_return_t
msg_copyout(task, msgptr, kmsg)
	register task_t task;
	register msg_header_t *msgptr;
	register kern_msg_t kmsg;
{
	register kern_port_t dest_port, reply_port;
	register port_name_t name;
	kern_return_t ret;

	assert(task != TASK_NULL);

	/*
	 *	Translate ports in the header.
	 */

	dest_port = (kern_port_t) kmsg->kmsg_header.msg_local_port;
	reply_port = (kern_port_t) kmsg->kmsg_header.msg_remote_port;

	assert(dest_port != KERN_PORT_NULL);
	port_lock(dest_port);
	if (dest_port->port_receiver == task) {
		assert(dest_port->port_in_use);

		name = dest_port->port_receiver_name;
	} else {
		/* This is a very rare case: just after one thread dequeues
		   a message, another thread gives away or destroys the
		   receive right.  Give the bozo PORT_NULL. */
		name = PORT_NULL;
	}
	dest_port->port_references--;
	port_check_unlock(dest_port);
	kmsg->kmsg_header.msg_local_port = name;

	if (reply_port == KERN_PORT_NULL)
		name = PORT_NULL;
	else {
		register port_hash_t entry;

		/* Do an object_copyout of MSG_TYPE_PORT/reply_port inline. */

		ipc_task_lock(task);
		port_lock(reply_port);

		entry = obj_entry_make(task, (kern_obj_t) reply_port, FALSE);
		if (entry == PORT_HASH_NULL)
			name = PORT_NULL;
		else {
			if (entry->type == PORT_TYPE_NONE)
				entry->type = PORT_TYPE_SEND;
			name = entry->local_name;
			obj_cache_set(task, name, (kern_obj_t) reply_port);
		}

		reply_port->port_references--;
		port_check_unlock(reply_port);
		ipc_task_unlock(task);
	}
	kmsg->kmsg_header.msg_remote_port = name;

	/* If non-simple, translate port rights and memory to receiver. */

	if (!kmsg->kmsg_header.msg_simple) {
		register caddr_t saddr;
		register caddr_t endaddr;

		saddr = (caddr_t)(&kmsg->kmsg_header + 1);
		endaddr = (((caddr_t) &kmsg->kmsg_header) + 
			   kmsg->kmsg_header.msg_size);

		while (saddr < endaddr) {
			register msg_type_long_t *tp =
				(msg_type_long_t *) saddr;
			unsigned int tn;
			unsigned int ts;
			vm_size_t numbytes;
			boolean_t is_port;
			long elts;

			if (tp->msg_type_header.msg_type_longform) {
				elts = tp->msg_type_long_number;
				tn = tp->msg_type_long_name;
				ts = tp->msg_type_long_size;
				saddr += sizeof(msg_type_long_t);
			} else {
				tn = tp->msg_type_header.msg_type_name;
				ts = tp->msg_type_header.msg_type_size;
				elts = tp->msg_type_header.msg_type_number;
				saddr += sizeof(msg_type_t);
			}
			numbytes = ((elts * ts) + 7) >> 3;

#ifdef	__alpha
			if (MSG_TYPE_ALIGN_64(tn,tp)) {
				/*
				 * saddr must be properly aligned
				 */
				saddr = (caddr_t)((vm_offset_t)(saddr+7)&(~7L));
			}
#endif
			/* Translate ports */

			if (is_port = MSG_TYPE_PORT_ANY(tn)) {
				register port_name_t *obj_list;

				if (tp->msg_type_header.msg_type_inline)
					obj_list = (port_name_t *) saddr;
				else
					obj_list = * ((port_name_t **) saddr);

				while (--elts >= 0) {
					register kern_obj_t obj;

					obj = (kern_obj_t) *obj_list;
					assert(obj != KERN_OBJ_INVALID);

					if (obj != KERN_OBJ_NULL)
						object_copyout(task, obj, tn,
							       obj_list);
					obj_list++;
				}
			}

			/*
			 *	Move data, if necessary;
			 *	advance to the next data item.
			 */

			if (tn == MSG_TYPE_INTERNAL_MEMORY) {
				vm_object_t object = * (vm_object_t *) saddr;

				* (vm_offset_t *) saddr = 0;
				if (ret = vm_map_find(task->map, object, 0,
						(vm_offset_t *) saddr,
						numbytes,
						TRUE) != KERN_SUCCESS) {
					if(ret == KERN_MAPENTRIES_LIMIT)
					    uprintf("msg_copyout: can not copy data, map entries limit reached\n");
					else 
					    uprintf("msg_copyout: can not copy data, not enough memory\n");
					kern_msg_free(kmsg);
					return RCV_NOT_ENOUGH_MEMORY;
				}

				assert(tp->msg_type_header.msg_type_longform);
				tp->msg_type_long_name = MSG_TYPE_INTEGER_8;

				assert(! tp->msg_type_header.msg_type_inline);
				saddr += sizeof(caddr_t);
			} else if (tp->msg_type_header.msg_type_inline) {
				saddr += ( (numbytes + 3) & (~0x3) );
			} else {
				if (! fast_pager_data(kmsg)) {
					if (is_port) {
						vm_map_copy_t	new_addr;

					    if(ret = vm_map_copyin(ipc_kernel_map,
							*(vm_offset_t *)saddr,
							numbytes,
							TRUE,
							&new_addr) != KERN_SUCCESS) {
						*(vm_offset_t *)saddr = (vm_offset_t)0;
					    	uprintf("msg_copyout: can not copy data, vm_map_copyin failed\n");
						kern_msg_free(kmsg);
						return RCV_NOT_ENOUGH_MEMORY;
					    }
					    *(vm_offset_t *)saddr = (vm_offset_t) new_addr;
					}

					if (ret = vm_map_copyout(
						task->map,
						(vm_offset_t *) saddr,
						*(vm_map_copy_t *) saddr)
					    != KERN_SUCCESS) {
						vm_map_copy_discard(*(vm_map_copy_t *) saddr);
					    if(ret == KERN_MAPENTRIES_LIMIT)
					    	uprintf("msg_copyout: can not copy data, map entries limit reached\n");
					    else 
					        uprintf("msg_copyout: can not copy data, not enough memory\n");
						* (vm_offset_t *) saddr = 0;
						kern_msg_free(kmsg);
						return RCV_NOT_ENOUGH_MEMORY;
					}
				}

				saddr += sizeof (caddr_t);
			}
		}
	}

	/* Copy out the message header and body, all at once. */

	if (current_thread()->ipc_kernel)
		(void) bcopy((caddr_t) &kmsg->kmsg_header, 
			     (caddr_t) msgptr, 
			      kmsg->kmsg_header.msg_size);
	else if (copyout((caddr_t) &kmsg->kmsg_header, 
			  (caddr_t) msgptr, 
			   kmsg->kmsg_header.msg_size)) {
		kern_msg_free(kmsg);
		return RCV_INVALID_MEMORY;
	}

	kern_msg_free(kmsg);
	return RCV_SUCCESS;
}

/*
 *	Routine:	msg_destroy
 *	Purpose:
 *		Cleans up the specified kernel message, destroying
 *		port rights and deallocating out-of-line memory.
 *		The kernel message is freed.
 *	Conditions:
 *		No locks held on entry or exit.
 */		
void
msg_destroy(kmsg)
	register kern_msg_t kmsg;
{
	register kern_port_t port;

	/* Release the port references in the header. */

	port = (kern_port_t) kmsg->kmsg_header.msg_remote_port;
	if (port != KERN_PORT_NULL)
		port_release_macro(port);

	port = (kern_port_t) kmsg->kmsg_header.msg_local_port;
	if (port != KERN_PORT_NULL)
		port_release_macro(port);

	/* If non-simple, destroy port rights and memory. */

	if (!kmsg->kmsg_header.msg_simple) {
		register caddr_t saddr;
		register caddr_t endaddr;

		saddr = (caddr_t)(&kmsg->kmsg_header + 1);
		endaddr = (((caddr_t) &kmsg->kmsg_header) + 
			   kmsg->kmsg_header.msg_size);

		while (saddr < endaddr) {
			register msg_type_long_t *tp =
				(msg_type_long_t *) saddr;
			long elts;
			unsigned int tn;
			unsigned int ts;
			vm_size_t numbytes;
			boolean_t is_port;

			if (tp->msg_type_header.msg_type_longform) {
				elts = tp->msg_type_long_number;
				tn = tp->msg_type_long_name;
				ts = tp->msg_type_long_size;
				saddr += sizeof(msg_type_long_t);
			} else {
				tn = tp->msg_type_header.msg_type_name;
				ts = tp->msg_type_header.msg_type_size;
				elts = tp->msg_type_header.msg_type_number;
				saddr += sizeof(msg_type_t);
			}
			numbytes = ((elts * ts) + 7) >> 3;

#ifdef	__alpha
			if (MSG_TYPE_ALIGN_64(tn,tp)) {
				/*
				 * saddr must be properly aligned
				 */
				saddr = (caddr_t)((long)(saddr+7) & (~7L)) ;
			}
#endif
			/* Destroy port rights */

			if (is_port = MSG_TYPE_PORT_ANY(tn)) {
				register port_t *port_list;

				if (tp->msg_type_header.msg_type_inline)
					port_list = (port_t *) saddr;
				else
					port_list = * ((port_t **) saddr);

				while (--elts >= 0) {
					register port_t pobj;

					pobj = *port_list++;
					if ((kern_obj_t)pobj==KERN_OBJ_INVALID)
						goto done;

					if ((kern_obj_t)pobj != KERN_OBJ_NULL)
						object_destroy(pobj, tn);
				}
			}

			/*
			 *	Deallocate data, if necessary;
			 *	advance to the next data item.
			 */

			if (tn == MSG_TYPE_INTERNAL_MEMORY) {
				assert(tp->msg_type_header.msg_type_longform);

				vm_object_deallocate(* (vm_object_t *) saddr);

				assert(! tp->msg_type_header.msg_type_inline);
				saddr += sizeof(caddr_t);
			} else if (tp->msg_type_header.msg_type_inline) {
				saddr += ( (numbytes + 3) & (~0x3) );
			} else {
				if (! fast_pager_data(kmsg)) {
					if (is_port) {
						(void) vm_deallocate(
							ipc_kernel_map,
							* (vm_offset_t *) saddr,
							numbytes);
					} else
						vm_map_copy_discard(*(vm_map_copy_t *) saddr);
				}

				saddr += sizeof (caddr_t);
			}
		}
	}

      done:
	kern_msg_free(kmsg);
}
