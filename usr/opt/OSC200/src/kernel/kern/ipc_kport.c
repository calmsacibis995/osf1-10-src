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
static char	*sccsid = "@(#)$RCSfile: ipc_kport.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/04/06 15:18:34 $";
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
 * File:	ipc_kport.c
 * Purpose:
 *	IPC port functions.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <mach_ipc_xxxhack.h>
#include <mach_net.h>
#include <mach_xp.h>

#include <sys/types.h>
#include <mach/kern_return.h>
#include <kern/zalloc.h>
#include <kern/queue.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/msg_queue.h>
#include <kern/kern_port.h>
#include <mach/notify.h>
#include <kern/sched_prim_macros.h>
#include <kern/ipc_hash.h>
#include <kern/ipc_prims.h>
#include <kern/ipc_globals.h>
#include <kern/ipc_kmesg.h>
#include <mach_debug/ipc_statistics.h>
#if	MACH_XP
#include <vm/vm_object.h>
#endif	

/*
 *	Routine:	port_reference [exported]
 *	Purpose:
 *		Acquire a reference to the port in question, preventing
 *		its destruction.  Also comes in macro form.
 */
void
port_reference(port)
	kern_port_t port;
{
	port_reference_macro(port);
}

/*
 *	Routine:	port_release [exported]
 *	Purpose:
 *		Release a port reference.
 *		Also comes in macro form.
 */
void
port_release(port)
	kern_port_t port;
{
	port_release_macro(port);
}

/*
 *	Routine:	port_alloc [internal]
 *	Purpose:
 *		Allocate a new port, giving all rights to "task".
 *		Allocates and initializes the translation.
 *		Returns a locked port, with no ref for the caller.
 *	Conditions:
 *		No locks held on entry.  Port locked on exit.
 */
kern_return_t
port_alloc(task, portp)
	task_t task;
	kern_port_t *portp;
{
	kern_return_t kr;
	zone_t my_zone;
	port_hash_t entry;

	/* note we are inspecting ipc_privilege without having the task locked.
	   this should be OK, assuming the value of ipc_privilege doesn't
	   change after the task is initialized. */
	my_zone = task->ipc_privilege ? port_zone_reserved : port_zone;

	kr = obj_alloc(task, my_zone, PORT_TYPE_RECEIVE_OWN, &entry);
	if (kr == KERN_SUCCESS) {
		register kern_port_t port = (kern_port_t) entry->obj;

		port->port_receiver_name = entry->local_name;
		port->port_receiver = task;
#if	MACH_IPC_XXXHACK
		port->port_owner = task;
#endif	
		port->port_backup = KERN_PORT_NULL;

		port->port_message_count = 0;
		port->port_backlog = PORT_BACKLOG_DEFAULT;
		msg_queue_init(&port->port_messages);
		queue_init(&port->port_blocked_threads);

		port->port_set = KERN_SET_NULL;
		/* don't have to initialize port_brothers */

		port->port_object.kp_type = PORT_OBJECT_NONE;

		*portp = port;
		ipc_event(port_allocations);
	}

	return kr;
}

/*
 *	Routine:	port_destroy
 *	Purpose:
 *		Shut down a port; called after both receiver and owner die.
 *		Destroys all queued messages and port rights associated with
 *		the port, sending death messages as appropriate.  References
 *		to this port held by the kernel or in other messages in
 *		transit may still remain.
 *	Conditions:
 *		The port must be locked on entry, and have one spare
 *		reference; the reference and the lock will both be released.
 */
void
port_destroy(kp)
	register kern_port_t kp;
{
	register kern_msg_t kmsg;
	register queue_t dead_queue;
	kern_port_t backup;
	thread_t thread_to_wake;

	/* Check that both receive & ownership rights are dead. */

	assert(kp->port_in_use);
	assert(kp->port_receiver == TASK_NULL);
#if	MACH_IPC_XXXHACK
	assert(kp->port_owner == TASK_NULL);
#endif	

	/* First check for a backup port.  */

	backup = kp->port_backup;
	if (backup != KERN_PORT_NULL) {
		/* Clear the backup port, and transfer kp's ref to ourself. */
		kp->port_backup = KERN_PORT_NULL;

		/* Need to send kp away in a notification.
		   The notification will get the receive/owner rights.
		   We transfer our ref to the notification. */

		kp->port_receiver = ipc_soft_task;
#if	MACH_IPC_XXXHACK
		kp->port_owner = ipc_soft_task;
#endif	
		port_unlock(kp);

		send_complex_notification(backup, NOTIFY_PORT_DESTROYED,
					  MSG_TYPE_PORT_ALL, kp);

		port_release(backup);
		return;
	}

	/* Mark the port as no longer in use. */

	kp->port_in_use = FALSE;
	port_unlock(kp);

	/* Throw away all of the messages. */

	dead_queue = &kp->port_messages.messages;
	while ((kmsg = (kern_msg_t) dequeue_head(dead_queue)) !=
	    KERN_MSG_NULL) {
		register port_hash_t entry;

		assert((kern_port_t) kmsg->kmsg_header.msg_local_port == kp);

		/* Must have port locked to check msg-accepted flag. */
		port_lock(kp);

		entry = kmsg->sender_entry;
		if (entry != PORT_HASH_NULL) {
			assert(entry->obj == (kern_obj_t) kp);
			assert(entry->kmsg == kmsg);

			/* don't need to have the task locked,
			   because the kmsg field is special. */
			entry->kmsg = KERN_MSG_NULL;
		}

		port_unlock(kp);

		kern_msg_destroy(kmsg);
	}

	/* Poke all sleeping senders. */

	while (!queue_empty(&kp->port_blocked_threads)) {
		queue_remove_first(&kp->port_blocked_threads, thread_to_wake, 
				   thread_t, ipc_wait_queue);
		thread_to_wake->ipc_state = SEND_SUCCESS;
		thread_go(thread_to_wake);
	}

	/* Eliminate send rights from all tasks. */
	port_lock(kp);
	obj_destroy_rights((kern_obj_t) kp);

#if	MACH_NET || MACH_XP
	port_unlock(kp);
	switch (kp->port_object.kp_type) {
#if	MACH_NET
		case PORT_OBJECT_NET:
			netipc_ignore(PORT_NULL, (port_t) kp);
			break;
#endif	
#if	MACH_XP
		case PORT_OBJECT_PAGER:
			vm_object_destroy((port_t) kp);
			break;
#endif	
	}
	port_lock(kp);
#endif	

	/* Release the reference provided by the caller. */

	kp->port_references--;
	if (ipc_debug & IPC_DEBUG_PORT_REFS)
		if (kp->port_references != 0)
			printf("port_destroy: refs = %d, zone = %08x\n",
			       kp->port_references, kp->port_home_zone);
	port_check_unlock(kp);
}

/*
 *	Routine:	port_dealloc [exported, internal]
 *	Purpose:
 *		Delete port rights to "kp" from this "task".
 *	Conditions:
 *		No locks on entry or exit.  Assumes task is valid.
 *	Side effects:
 *		Port may be "destroyed" or rights moved.
 */
kern_return_t
port_dealloc(task, kp)
	register task_t task;
	register kern_port_t kp;
{
	register port_hash_t entry;
	port_type_t pt;
#if	MACH_IPC_XXXHACK
	task_t otask;		/* other task involved */
	port_name_t name;
#endif	

	assert(task != TASK_NULL);

	if (kp == PORT_NULL)
		return KERN_SUCCESS;

	ipc_task_lock(task);

	/* Look for the port right. */

	entry = obj_entry_find(task, (kern_obj_t) kp);
	if (entry == PORT_HASH_NULL) {
		ipc_task_unlock(task);
		return KERN_SUCCESS;
	}

	/* Found it... remove the translation. */

	pt = entry->type;
	port_lock(kp);
	obj_entry_destroy(task, (kern_obj_t) kp, entry);
	ipc_task_unlock(task);

	/* If we hold special rights, there's more to do.
	   At this point, the port is still locked and
	   our caller has his ref for the port. */

	switch (pt) {
	    case PORT_TYPE_SEND:
		assert(kp->port_receiver != task);
#if	MACH_IPC_XXXHACK
		assert(kp->port_owner != task);
#endif	

		/* Not receiver or owner; nothing to do. */

		port_unlock(kp);
		break;

#if	MACH_IPC_XXXHACK
	    case PORT_TYPE_RECEIVE:
		assert(kp->port_receiver == task);
		assert(kp->port_owner != task);

		otask = kp->port_owner;

		if (otask == TASK_NULL) {
			/* We are receiver and ownership is dead.
			   Destroy the port. */

			port_clear_receiver(kp, TASK_NULL);
			kp->port_references++; /* for port_destroy */
			port_destroy(kp);
		} else if (otask == ipc_soft_task) {
			/* We are receiver and ownership is in transit.
			   Mark receiver as dead; notification will get
			   sent when ownership is picked up. */

			port_clear_receiver(kp, TASK_NULL);
			port_unlock(kp);
		} else {
			/* We are receiver and there is a real owner.
			   Send him a notification. */

			port_clear_receiver(kp, ipc_soft_task);
			kp->port_references++; /* for object_copyout */
			port_unlock(kp);
			object_copyout(otask, (kern_obj_t) kp,
				       MSG_TYPE_PORT_RECEIVE, &name);
			send_notification(otask, NOTIFY_RECEIVE_RIGHTS, name);
		}
		break;

	    case PORT_TYPE_OWN:
		assert(kp->port_receiver != task);
		assert(kp->port_owner == task);

		otask = kp->port_receiver;

		if (otask == TASK_NULL) {
			/* We are owner and receive right is dead.
			   Destroy the port. */

			kp->port_owner = TASK_NULL;
			kp->port_references++; /* for port_destroy */
			port_destroy(kp);
		} else if (otask == ipc_soft_task) {
			/* We are owner and receive right is in transit.
			   Mark owner as dead; notification will get
			   sent when receive right is picked up. */

			kp->port_owner = TASK_NULL;
			port_unlock(kp);
		} else {
			/* We are owner and there is a real receiver.
			   Send him a notification. */

			kp->port_owner = ipc_soft_task;
			kp->port_references++; /* for object_copyout */
			port_unlock(kp);
			object_copyout(otask, (kern_obj_t) kp,
				       MSG_TYPE_PORT_OWNERSHIP, &name);
			send_notification(otask, NOTIFY_OWNERSHIP_RIGHTS, name);
		}
		break;
#endif	/* MACH_IPC_XXXHACK */
	case PORT_TYPE_RECEIVE_OWN:
		assert(kp->port_receiver == task);
#if	MACH_IPC_XXXHACK
		assert(kp->port_owner == task);
#endif

		/* We are the receiver and owner.
		   Destroy the port. */

#if	MACH_IPC_XXXHACK
		kp->port_owner = TASK_NULL;
#endif
		port_clear_receiver(kp, TASK_NULL);

		kp->port_references++; /* for port_destroy */
		port_destroy(kp);
		break;

	    default:
		panic("port_dealloc: strange translation type");
	}

	return KERN_SUCCESS;
}

/*
 *	Routine:	port_lookup [external]
 *	Purpose:
 *		Converts a task/name pair to a port.
 *		If successful, the caller has a ref for the object.
 *
 *		Returns KERN_PORT_NULL for failure.
 *	Conditions:
 *		Nothing locked.
 */
kern_port_t
port_lookup(task, name)
	register task_t task;
	port_t name;
{
	register kern_obj_t obj = KERN_OBJ_NULL;
	register port_hash_t entry;

	assert(task != TASK_NULL);

	ipc_task_lock(task);
	if (!task->ipc_active)
		goto exit;

	entry = obj_entry_lookup(task, name);
	if (entry == PORT_HASH_NULL)
		goto exit;

	if (!PORT_TYPE_IS_PORT(entry->type))
		goto exit;

	obj = entry->obj;
	obj_reference(obj);

    exit:
	ipc_task_unlock(task);
	return (kern_port_t) obj;
}

#include <mach_kdb.h>

#if	MACH_KDB

#define printf	kdbprintf

/*
 *	Routine:	port_print
 */
void		port_print(kp)
	register
	kern_port_t	kp;
{
	extern int indent;

	printf("port 0x%X\n", kp);

	indent += 2;
	  ipc_obj_print(&kp->port_obj);
	  iprintf("receiver = 0x%X", kp->port_receiver);
	  printf(",receiver_name = 0x%X", kp->port_receiver_name);
#if	MACH_IPC_XXXHACK
	  printf(",owner = 0x%X\n", kp->port_owner);
#endif
	  iprintf("set = 0x%X", kp->port_set);
	  printf(",backup = 0x%X", kp->port_backup);
	  printf(",backlog = 0x%X\n", kp->port_backlog);
	  iprintf("message count = %D, queue = %X\n", kp->port_message_count,
	  				&kp->port_messages);
	  iprintf("object = 0x%X (type %D)\n",
		kp->port_object.kp_object,
		kp->port_object.kp_type);
	  indent -=2;
}
#endif	/* MACH_KDB */
