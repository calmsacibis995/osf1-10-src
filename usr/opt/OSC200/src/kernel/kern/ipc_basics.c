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
static char	*sccsid = "@(#)$RCSfile: ipc_basics.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/06/29 08:20:48 $";
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
 * File:	ipc_basics.c
 * Purpose:
 *	Primitive IPC functions.
 *
 *	The IPC module performs two basic management functions: port rights
 *	(including name translations), and message queueing.
 *
 *	Port rights/translations:
 *		We must manage a set of (task, port, name, rights) tuples,
 *		and perform the following functions:
 *			Verify and translate (task, name) to port [message send].
 *			Translate (task, port) to name [message receive];
 *			 if no such name exists, create a new name.
 *			Remove (task, port, *, rights) [port deallocation].
 *			Remove (task, *, *, *) [task destruction].
 *			Remove (*, port, *, *) [port destruction].
 *
 *		Synchronization is achieved through a translation lock on
 *		each task, and on each port.  Creation or destruction of
 *		a tuple requires the acquisition of both locks; 
 *		task translation locks are taken first, as most operations
 *		involve tasks.	So that destruction operations are guaranteed
 *		to terminate, each port and each task maintains an indication
 *		of livelihood; once declared dead, no further translations
 *		involving that port/task may be created.
 *
 *	Message queues:
 *		Once fully translated, a message may be queued on a port.
 *		In order to provide multiplexed message receipt, ports may
 *		be grouped into sets.  [Currently, the external interface
 *		only provides for one set per task -- the "enabled" port set.]
 *		Each set of ports (or singleton port) has its own queue
 *		containing messages for all of the member ports.
 *
 *		Synchronization consists of a lock on each port, and a lock
 *		on each message queue.  Queueing a message requires the
 *		acquisition of the port lock, to check for over-backlog
 *		conditions and to find the appropriate message queue; then
 *		the message queue is locked to perform the actual queueing.
 *		Dequeueing requires the locking of the message queue to
 *		do the actual dequeueing (at which time the message queue
 *		may be unlocked); then, the port in question is locked to
 *		peform necessary bookkeeping.
 *
 *		In order to move a port from one set to another, first it
 *		must be locked, then both the old and new message queues
 *		must be locked, and appropriate messages relocated.  Since
 *		this is the only operation causing more than one message queue
 *		to be locked, any consistent ordering of two message queue 
 *		locks is sufficient to prevent deadlock.
 *
 *	Interactions:
 *		An integral feature of port translation is the ability
 *		to transfer receive (or ownership, or both) rights during
 *		message translation.  Unfortunately, the message queueing
 *		system also needs to know the "receiver" for a given port
 *		at various times.
 *
 *		For this reason, separating the "translation" lock on a port
 *		from the "message queueing" lock on that port results in a
 *		minimal increase in parallelism -- most operations may involve
 *		using or changing the "receiver", and would hold both locks
 *		for most of their duration.
 *
 *		Therefore, only one lock is used for both translation and
 *		message queueing at a port.
 *
 *	Other locking issues:
 *		The IPC state of a thread is only manipulated by that thread
 *		itself when "awake", and only by the thread which will wake
 *		it up otherwise.  It is locked by the lock on the object
 *		upon which the thread is waiting: the message queue lock
 *		for msg_receive, or the port lock for msg_send.
 *
 *		The only interrupt-level access to the IPC system is
 *		via the network, and its uses are only msg_queue and
 *		port_reference.  Thus, only the port (data) and message
 *		queue locks need interrupt protection; IPC state changes
 *		are all done inside another lock, so it need not separately
 *		be protected.
 *
 *		Because the count of messages queued at a port is locked
 *		by the port lock, and messages may be removed from message
 *		queues without holding that lock, it is possible for the
 *		count to be larger than calculated by traversing the queue.
 *
 *	Port references:
 *		Ports are reference-counted, both for garbage collection
 *		and to detect when no (non-owner/receiver) senders remain.
 *
 *		Each port translation constitutes a reference; it constitutes
 *		a send right.  In order to consistently determine when no
 *		senders remain, an additional reference is taken when a task
 *		holds both receive and ownership rights.
 *
 *		Each port right contained in a message in transit requires
 *		a reference.  Again, "all" rights gains yet another reference.
 *		A message containing a port more than once acquires a
 *		reference for each use.
 *
 *		Port names kept in other kernel data structures should take
 *		another reference.  For example, the task data structure
 *		stores 	the names of the self, data, and notify ports; those
 *		ports must be referenced in order for the values stored in the
 *		task structure to remain valid.
 *
 *	Kernel interface:
 *		Messages directed to ports for which the kernel is the
 *		receiver are directly handled by the mach_server() interface.
 *		Note that because the request message is still in-transit,
 *		all of the port names used by the interface are the global
 *		port names, and have references associated with them.
 *
 *		Similarly, interface routines which return ports must acquire
 *		references for the reply message.
 *
 *	Revision History:
 *
 * 02-May-91	Peter H. Smith
 *	Only allow thread_go_and_switch to be called when BOTH the sender and
 *	receiver are running the POLICY_TIMESHARE policy.
 */

#include <mach_km.h>
#include <mach_net.h>
#include <mach_np.h>
#include <mach_ipc_xxxhack.h>
#include <mach_ipc_tcache.h>
#include <mach_ipc_wwa.h>
#include <mach_debug.h>
#ifdef	ibmrt
#include <mach_debug_ca.h>
#endif	
#include <rt_sched.h>

#include <kern/assert.h>

#include <kern/queue.h>
#include <mach/boolean.h>

#include <kern/zalloc.h>

#include <vm/vm_map.h>
#include <mach/vm_param.h>	/* for page_size */
#include <vm/vm_kern.h>		/* for vm_move */
#include <vm/vm_object.h>

#include <kern/task.h>
#include <kern/thread.h>

#include <mach/message.h>
#include <mach/notify.h>

#include <kern/kern_obj.h>
#include <kern/kern_port.h>
#include <kern/kern_msg.h>
#include <kern/kern_set.h>

#include <machine/cpu.h>

#include <kern/sched_prim_macros.h>

#include <mach_debug/ipc_statistics.h>
#include <kern/ipc_hash.h>
#include <kern/ipc_cache.h>
#include <kern/ipc_signal.h>
#include <kern/ipc_prims.h>
#include <kern/ipc_globals.h>
#include <kern/ipc_kmesg.h>
#include <kern/ipc_basics.h>

#include <kern/macro_help.h>

#include <mach/policy.h>
#include <sys/sched_mon.h>

/*
 *	Routine:	adjust_timeout [internal]
 *	Purpose:
 *		Munges the timeout values the user provides to the
 *	IPC traps into something the scheduling code likes.
 *
 *	static void
 *	adjust_timeout(newwait, maxwait)
 *		int &newwait;
 *		msg_timeout_t maxwait;
 */
#define adjust_timeout(newwait, maxwait) 				\
MACRO_BEGIN								\
	(newwait) = ((maxwait) / timeout_scaling_factor) * 1000;	\
	(newwait) += (((maxwait) % timeout_scaling_factor) * 1000)	\
		/ timeout_scaling_factor;				\
	if ((newwait) < timeout_minimum)				\
		(newwait) = timeout_minimum;				\
	else                                                            \
		(newwait) += 1;                                         \
MACRO_END

/*
 *	Routine:	send_notification
 *	Purpose:
 *		Sends a notification to the specified task regarding
 *		some interesting event; all such events involve exactly
 *		one port.
 *	Conditions:
 *		No locks should be held on entry.
 */
void
send_notification(task, msg_id, name)
	register task_t task;		/* Who we're notifying */
	int msg_id;			/* What event occurred */
	port_name_t name;		/* What port is involved */
{
	register kern_msg_t kmsg;
	register notification_t	*notify;
	kern_port_t tnotify;

	ipc_event(notifications);

	ipc_task_lock(task);
	if (!task->ipc_active) {
		ipc_task_unlock(task);
		return;
	}

	tnotify = (kern_port_t) task->task_notify;
	if (tnotify == KERN_PORT_NULL) {
		ipc_task_unlock(task);
		return;
	}
	port_reference(tnotify);

	ipc_task_unlock(task);

	assert_static(sizeof(notification_t) <=
	       (KERN_MSG_SMALL_SIZE -
		sizeof(struct kern_msg) +
		sizeof(msg_header_t)));
	kern_msg_allocate_small(kmsg);
	notify = (notification_t *) &kmsg->kmsg_header;

	*notify = notification_template;
	notify->notify_header.msg_local_port = (port_t) tnotify;
	notify->notify_header.msg_id = msg_id;
	notify->notify_port = (port_t) name;

	(void) msg_queue(kmsg, SEND_ALWAYS, 0);
}


/*
 *	Routine:	send_complex_notification
 *	Purpose:
 *		Sends a notification to the specified port regarding
 *		some interesting event; all such events involve exactly
 *		one port.
 *
 *	Conditions:
 *		No locks should be held on entry.
 *		The caller should hold a ref for the dest; we will take
 *		another ref for the message.  The port in question should
 *		already be prepped for putting in a message.
 */
void
send_complex_notification(dest, msg_id, msg_type_name, port)
	register kern_port_t dest;	/* Who we're notifying */
	int msg_id;			/* What event occurred */
	int msg_type_name;		/* Type of rights */
	kern_port_t port;		/* The port involved */
{
	register kern_msg_t kmsg;
	register notification_t	*notify;

	ipc_event(notifications);

	assert_static(sizeof(notification_t) <=
	       (KERN_MSG_SMALL_SIZE -
		sizeof(struct kern_msg) +
		sizeof(msg_header_t)));
	kern_msg_allocate_small(kmsg);
	notify = (notification_t *) &kmsg->kmsg_header;

	assert(dest != KERN_PORT_NULL);
	port_reference(dest);

	*notify = complex_notification_template;
	notify->notify_header.msg_local_port = (port_t) dest;
	notify->notify_header.msg_id = msg_id;
	notify->notify_type.msg_type_name = msg_type_name;
	notify->notify_port = (port_t) port;

	(void) msg_queue(kmsg, SEND_ALWAYS, 0);
}

/*
 *	Routine:	memory_object_terminate_hack
 *	Purpose:
 *		Send a memory_object_terminate message,
 *		atomically consuming a reference
 *		for the memory object port.
 */

void
memory_object_terminate_hack(object, control, name)
	kern_port_t object;
	kern_port_t control;
	kern_port_t name;
{
	extern msg_option_t msg_send_from_kernel_options;

	register struct Request {
		msg_header_t Head;
		msg_type_t controlType;
		port_t control;
		msg_type_t nameType;
		port_t name;
	} *Request;
	register kern_msg_t kmsg;

	static msg_type_t Type = {
		/* msg_type_name = */		MSG_TYPE_PORT_ALL,
#ifdef	__alpha
		/* msg_type_size = */		64,
#else
		/* msg_type_size = */		32,
#endif
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	TRUE,
		/* msg_type_unused = */		0
	};

	assert_static(sizeof *Request <=
	       (KERN_MSG_SMALL_SIZE -
		sizeof(struct kern_msg) +
		sizeof(msg_header_t)));
	kern_msg_allocate_small(kmsg);
	Request = (struct Request *) &kmsg->kmsg_header;

	Request->Head.msg_unused = 0;
	Request->Head.msg_simple = FALSE;
	Request->Head.msg_size = sizeof *Request;
	Request->Head.msg_type = MSG_TYPE_NORMAL;
	Request->Head.msg_local_port = (port_t) object;
	Request->Head.msg_remote_port = PORT_NULL;
	Request->Head.msg_id = 2201;
	Request->controlType = Type;
	Request->control = (port_t) control;
	Request->nameType = Type;
	Request->name = (port_t) name;

	if (control != KERN_PORT_NULL)
		(void) object_copyin_from_kernel(kernel_task, (port_t) control,
						 MSG_TYPE_PORT_ALL, TRUE,
						 (kern_obj_t *) &control);

	if (name != KERN_PORT_NULL)
		(void) object_copyin_from_kernel(kernel_task, (port_t) name,
						 MSG_TYPE_PORT_ALL, TRUE,
						 (kern_obj_t *) &name);

	(void) msg_queue(kmsg, SEND_ALWAYS|msg_send_from_kernel_options, 0);
}

/*
 *	Routine:	mach_msg
 *	Purpose:
 *		Perform appropriate Mach call based on previously
 *		recognized kernel message.  Returns reply message
 *		or NULL.
 *	Conditions:
 *		No locks held on entry or exit.
 */
kern_msg_t
mach_msg(kmsgptr)
	register kern_msg_t kmsgptr;
{
	register kern_msg_t out_msg;
	register kern_port_t dest_port;

	ipc_event(kernel);

	kern_msg_allocate_large(out_msg);

	if (!mach_server(&kmsgptr->kmsg_header, &out_msg->kmsg_header))
	if (!mach_host_server(&kmsgptr->kmsg_header, &out_msg->kmsg_header))
#if	MACH_NET
	if (!mach_net_server(&kmsgptr->kmsg_header, &out_msg->kmsg_header))
#endif	
#if	MACH_DEBUG
	if (!mach_debug_server(&kmsgptr->kmsg_header, &out_msg->kmsg_header))
#endif	
#if	defined(ibmrt) && MACH_DEBUG_CA
	if (!mach_debug_ca_server(&kmsgptr->kmsg_header, &out_msg->kmsg_header))
#endif	
#if	MACH_KM
	if (!monitor_server(&kmsgptr->kmsg_header, &out_msg->kmsg_header))
#endif	
		if (ipc_debug & IPC_DEBUG_BOGUS_KMSG)
			uprintf("msg_queue: bogus kernel message, id=%d\n", 
				kmsgptr->kmsg_header.msg_id);

	dest_port = (kern_port_t) out_msg->kmsg_header.msg_remote_port;

	if (dest_port == KERN_PORT_NULL) {
		kern_msg_free(out_msg);
		msg_destroy(kmsgptr);
		return KERN_MSG_NULL;
	} else {
		out_msg->kmsg_header.msg_local_port = (port_t) dest_port;
		out_msg->kmsg_header.msg_remote_port = PORT_NULL;

	 	/*
		 *	The reference to the reply port from the
		 *	original message (dest_port) is kept; we
		 *	must dispose of the reference to the
		 *	destination port (that represents the
		 *	kernel object) from the original message.
		 */

	    {
		register kern_port_t port =
			(kern_port_t) kmsgptr->kmsg_header.msg_local_port;

		assert(port != PORT_NULL);
		port_release_macro(port);
	    }

		/*
		 *	We've accounted for the ports in the
		 *	original message header; now we destroy
		 *	any other contents, and free the incoming
		 *	message
		 */

		if (kmsgptr->kmsg_header.msg_simple)
			kern_msg_free(kmsgptr);
		else {
			kmsgptr->kmsg_header.msg_local_port = PORT_NULL;
			kmsgptr->kmsg_header.msg_remote_port = PORT_NULL;
			msg_destroy(kmsgptr);
		}

		return out_msg;
	 }
}

/*
 *	Routine:	msg_queue [exported]
 *	Purpose:
 *		Queue the given kernel message on the appropriate
 *		port, using "option" and "send_timeout" to select
 *		termination conditions (see msg_send).
 *
 *		The message in question must already be fully
 *		translated, and contain references to ports as necessary.
 *
 *	Conditions:
 *		No locks held on entry or exit.
 *	Returns:
 *		An error value appropriate to msg_send.
 *		The message in question is either queued (SEND_SUCCESS,
 *		SEND_WILL_NOTIFY), or destroyed (all other return values).
 */

msg_return_t
msg_queue(kmsg, option, tout)
	register kern_msg_t kmsg;
	msg_option_t option;
	msg_timeout_t tout;
{
	register kern_port_t kp;
	msg_return_t result = SEND_SUCCESS;

	kp = (kern_port_t) kmsg->kmsg_header.msg_local_port;

	/* msg_copyin checks for a null destination port */
	assert(kp != KERN_PORT_NULL);

	port_lock(kp);

#if	MACH_NP
	if (kp->port_in_use &&
	    (kp->port_external != (int **) 0) && 
	    (kp->port_external[MSG_QUEUE_OFF] != (int *) 0)) {
		kern_return_t result;

		port_unlock(kp);

		result = ((int (*)()) kp->port_external[MSG_QUEUE_OFF])
		    (kmsg, option, tout);
		if (result != SEND_KERNEL_REFUSED)
			return result;

		port_lock(kp);
	}
#endif	

	if (kp->port_receiver == kernel_task) {
		typedef struct {
    			msg_header_t	Head;
			msg_type_t	RetCodeType;
			kern_return_t	RetCode;
		} kern_reply_msg;

		register kern_msg_t out_msg;

		assert(kp->port_in_use);
		assert(kp->port_message_count == 0);
		assert(0 < kp->port_backlog);
		port_unlock(kp);

		out_msg = mach_msg(kmsg);
		if (out_msg != KERN_MSG_NULL) {
			/*
			 *	KERN_ABORTED means operation was interrupted
			 *	before it did anything.
			 */
			if (((kern_reply_msg *)
			     (&out_msg->kmsg_header))->RetCode
				== KERN_ABORTED) {
				    msg_destroy(out_msg);
				    return SEND_INTERRUPTED;
			}
			
			(void) msg_queue(out_msg, SEND_ALWAYS, 0);
		}
		return SEND_SUCCESS;
	}

	/* Before queueing message, need to initialize sender_entry.
	   The SEND_NOTIFY case in the while loop might modify this. */
	kmsg->sender_entry = PORT_HASH_NULL;

	/* Obey backlog, if appropriate. */

	while (kp->port_message_count >= kp->port_backlog) {
		register thread_t self = current_thread();

		if (option & SEND_ALWAYS)
			break;

		/* Check that we don't go to sleep on a dead port. */
		if (!kp->port_in_use) {
			result = SEND_INVALID_PORT;
			goto GiveUp;
		}

		if (!(option & SEND_TIMEOUT))
			tout = 0;

		/* If we've exhausted our timeout... */

		if (tout == 0) {
			/* Setup a notification, if able. */

			if (option & SEND_NOTIFY) {
				register task_t task = self->task;
				register port_hash_t entry;

				entry = obj_entry_find(task, (kern_obj_t) kp);
				if (entry == PORT_HASH_NULL) {
					result = SEND_INVALID_PORT;
					goto GiveUp;
				}

				if (entry->kmsg != KERN_MSG_NULL) {
					result = SEND_NOTIFY_IN_PROGRESS;
					goto GiveUp;
				}

				/* don't need to have task locked to modify
				   the special kmsg field.  don't need refs
				   because if the entry goes away, this
				   pointer will get cleaned up. */

				entry->kmsg = kmsg;
				kmsg->sender_entry = entry;

				result = SEND_WILL_NOTIFY;
				break;
			}

			/* Terminate with a timeout. */

			if (option & SEND_TIMEOUT) {
				result = SEND_TIMED_OUT;

			    GiveUp:
				port_unlock(kp);
	
				msg_destroy(kmsg);
				return result;
			}
		}

		/*
		 *	Assert that we're about to block;
		 *	put us on the port's wakeup queue, and
		 *	make note of our state.
		 */

		self->ipc_state = SEND_IN_PROGRESS;

		queue_enter(&kp->port_blocked_threads, self, thread_t, 
			    ipc_wait_queue);

		if (tout != 0) {
			int adj_tout;

			adjust_timeout(adj_tout, tout);
			thread_will_wait_with_timeout(self, adj_tout);
		} else
			thread_will_wait(self);

	 	port_unlock(kp);
		thread_block();
		port_lock(kp);

		/* Why did we wake up? */

		if (self->ipc_state == SEND_SUCCESS)
			continue;
		assert(self->ipc_state == SEND_IN_PROGRESS);

		/* Take ourselves off wakeup queue. */

		queue_remove(&kp->port_blocked_threads, self, thread_t, 
			     ipc_wait_queue);

		/*
		 *	Thread wakeup-reason field tells us why
		 *	the wait was interrupted.
		 */
		switch (self->wait_result) {
		    case THREAD_INTERRUPTED:
		    case THREAD_SHOULD_TERMINATE:
			/* Send was interrupted - exit. */

			result = SEND_INTERRUPTED;
			if (ipc_debug & IPC_DEBUG_SEND_INT)
				printf("msg_queue: send interrupted\n");
			goto GiveUp;

		    case THREAD_TIMED_OUT:
			/* Time-out expired. */

			tout = 0;
			break;

		    default:
			/* Unknown wakeup reason.  Try again. */
			break;
		}
	}

	/* Check that we don't deliver a message to a dead port. */

	if (!kp->port_in_use) {
		result = SEND_INVALID_PORT;
		goto GiveUp;
	}

    {
	register msg_queue_t *mq;
	register kern_set_t set;
	register queue_t bqueue;
	register thread_t t;

	set = kp->port_set;
	if (set != KERN_SET_NULL) {
		set_lock(set);
		kp->port_message_count++;
		set_unlock(set);
		PSIGNAL(kp->port_receiver,
			kmsg->kmsg_header.msg_type & MSG_TYPE_EMERGENCY);
		mq = &set->set_messages;
	} else {
		kp->port_message_count++;
		mq = &kp->port_messages;
	}
	assert(kp->port_message_count > 0);

	msg_queue_lock(mq);
	bqueue = &mq->blocked_threads;

	/* Can unlock the port now that the msg queue is locked and
	   we know the port is active.  While the msg queue is locked,
	   we have control of the kmsg, so the ref in it for the port
	   is still good.  If the msg queue is in a set (dead or alive),
	   then we're OK because the port is still a member of the set
	   and the set won't go away until the port is taken out, which
	   tries to lock the set's msg queue to remove the port's msgs. */

	port_unlock(kp);

	/* Look for a receiver for the message. */

	while ((queue_t) (t = (thread_t) bqueue->next) != bqueue) {
		/* Dequeue the sleeping receiver. */
		queue_remove(bqueue, t, thread_t, ipc_wait_queue);

		/* Check if the receiver can handle our message. */
		if (kmsg->kmsg_header.msg_size <= t->ipc_data.msize) {
#if RT_SCHED
			register thread_t self;
#endif /* RT_SCHED */
			t->ipc_state = RCV_SUCCESS;
			t->ipc_data.kmsg = kmsg;

			msg_queue_unlock(mq);

			/*
			 * XXX thread_run optimization can't be used
			 * XXX for fixed priority threads.
			 */
#if RT_SCHED
			/*
			 * Restrict this further.  The expression which was
			 * used before allowed handoff between a TIMESHARE
			 * and FIXEDPRI thread.  Change this to only allow
			 * handoff between two TIMESHARE threads.
			 */
			self = current_thread();
			if ((option & SEND_SWITCH) &&
			    ((self->policy & t->policy & POLICY_TIMESHARE))
			   ) {
			        RT_SCHED_HIST_SPL(RTS_tsswitch, self, 
						  self->policy, t, 0, 0);
				thread_go_and_switch(t);
			 }
#else /* RT_SCHED */
			if ((option & SEND_SWITCH) &&
			    ((current_thread()->policy & t->policy &
			     POLICY_FIXEDPRI) == 0)
			   )
				thread_go_and_switch(t);
#endif /* RT_SCHED */
			else
				thread_go(t);

			return result;
		}

		t->ipc_state = RCV_TOO_LARGE;
		t->ipc_data.msize = kmsg->kmsg_header.msg_size;
		thread_go(t);
	}

	/* Couldn't find a receiver, just enqueue the message. */

	enqueue(&mq->messages, (queue_entry_t) kmsg);
	msg_queue_unlock(mq);
    }

	return result;
}

/*
 *	Macro:		wakeup_waiting_sender
 *	Purpose:
 *		Wakes up a thread waiting to send to a port.
 *	Conditions:
 *		The port is locked.
 */
#define wakeup_waiting_sender(port) 				\
MACRO_BEGIN							\
	register thread_t sender_to_wake;			\
								\
	queue_remove_first(&(port)->port_blocked_threads,	\
			   sender_to_wake, thread_t,		\
			   ipc_wait_queue);			\
	sender_to_wake->ipc_state = SEND_SUCCESS;		\
	thread_go(sender_to_wake);				\
MACRO_END

/*
 *	Routine:	msg_dequeue [exported]
 *	Purpose:
 *		Dequeues a kernel message from a queue.
 *
 *		If msg_dequeue returns RCV_SUCCESS then kmsgptr
 *		is set to the dequeued kmsg.  If it returns
 *		RCV_TOO_LARGE, then kmsgptr is set the size of
 *		the overly-large kmsg.
 *
 *	Conditions:
 *		The msg-queue is locked; no ports, sets, or tasks
 *		are locked.  The queue will be unlocked.
 *
 *		The msg-queue is part of a port or port set.
 *		Our caller must hold a reference for the port or set
 *		to keep the queue from being deallocated out
 *		from under us.  Furthermore, we don't want to get
 *		stuck waiting in a dead queue and never woken up.
 *		So, our caller has to lock the port/set, check that
 *		it is active, lock the queue, and unlock the port/set.
 *		The port/set destruction code locks the port/set,
 *		marks it deactive, locks the queue, and wakes up any
 *		waiting threads.
 *
 *		XXX problems with timeout handling
 */
msg_return_t
msg_dequeue(message_queue, max_size, option, tout, kmsgptr)
	msg_queue_t *message_queue;
	msg_size_t max_size;
	msg_option_t option;
	msg_timeout_t tout;
	kern_msg_t *kmsgptr;
{
	register kern_msg_t kmsg;

    {
	register msg_queue_t *mq = message_queue;
	register queue_t mqueue = &mq->messages;

	for (;;) {
		register thread_t self;
		register queue_t bqueue;

		kmsg = (kern_msg_t) mqueue->next;
		if ((queue_t) kmsg != mqueue) {
			/* There is a kmsg at the front of the queue.
			   Check space requirements. */

			if (kmsg->kmsg_header.msg_size > max_size) {
				* (msg_size_t *) kmsgptr =
					kmsg->kmsg_header.msg_size;
				msg_queue_unlock(mq);
				return RCV_TOO_LARGE;
			}

			/* Dequeue the kmesg and quit looping.
			   The fast dequeue needs queue_head at
			   the front of the kmsg. */

			(mqueue->next = kmsg->queue_head.next)->prev = mqueue;
			break;
		}

		/* Will have to sleep for a message. */
		self = current_thread();
		self->ipc_state = RCV_IN_PROGRESS;
		self->ipc_data.msize = max_size;

		if (option & RCV_TIMEOUT) {
			int adj_tout;

			if (tout == 0) {
				msg_queue_unlock(mq);
				return RCV_TIMED_OUT;
			}

			adjust_timeout(adj_tout, tout);
			thread_will_wait_with_timeout(self, adj_tout);
		} else
			thread_will_wait(self);

		bqueue = &mq->blocked_threads;
		queue_enter(bqueue, self, thread_t, ipc_wait_queue);

		msg_queue_unlock(mq);
		thread_block();
		msg_queue_lock(mq);

		/* Quick check for success. */
		if (self->ipc_state == RCV_SUCCESS) {
			/* Pick up the message that was handed to us. */

			kmsg = self->ipc_data.kmsg;
			break;
		}

		switch (self->ipc_state) {
		    case RCV_TOO_LARGE:
			/* Return the too-large message's size. */

			* (msg_size_t *) kmsgptr = self->ipc_data.msize;
			/* fall through */

		    case RCV_PORT_CHANGE:
		    case RCV_INVALID_PORT:
			/* Something bad happened to the port/set. */

			msg_queue_unlock(mq);
			return self->ipc_state;

		    case RCV_IN_PROGRESS:
			/*
			 *	Awakened for other than IPC completion.
			 *	Remove ourselves from the waiting queue,
			 *	then check the wakeup cause.
			 */
			queue_remove(bqueue, self, thread_t, ipc_wait_queue);

			switch (self->wait_result) {
			    case THREAD_INTERRUPTED:
			    case THREAD_SHOULD_TERMINATE:
				/* Interrupt - exit receive. */

				msg_queue_unlock(mq);
				return RCV_INTERRUPTED;

			    case THREAD_TIMED_OUT:
				/* Timeout expired. */

				msg_queue_unlock(mq);
				return RCV_TIMED_OUT;
			}

			/* Unknown wakeup cause.  Try again. */
			break;

		    default:
			panic("msg_dequeue: strange ipc_state");
		}
	}

	/* At this point, we have a kmsg and can unlock the msg queue. */

	assert(kmsg->kmsg_header.msg_size <= max_size);
	msg_queue_unlock(mq);
    }

    {
	register kern_port_t kp;
	register port_hash_t entry;
	register int mcount;

	kp = (kern_port_t) kmsg->kmsg_header.msg_local_port;
	port_lock(kp);

	/* If the port is in a set, the set must be locked
	   to increment port_message_count.  But don't bother
	   looking at kp->port_set if the locking calls are nops. */

#if	MACH_SLOCKS
    {
	register kern_set_t set = kp->port_set;

	if (set == KERN_SET_NULL)
		mcount = --kp->port_message_count;
	else {
		set_lock(set);
		mcount = --kp->port_message_count;
		set_unlock(set);
	}
    }
#else	
	mcount = --kp->port_message_count;
#endif

	assert(mcount >= 0);

#if	MACH_IPC_WWA
	/* We received one message from this port, so wakeup one sender. */

	if (!queue_empty(&kp->port_blocked_threads))
		wakeup_waiting_sender(kp);
#else
	/* No messages queued at this port, so wakeup all waiting senders. */

	if (!queue_empty(&kp->port_blocked_threads) &&
	    (mcount == 0))
		do
			wakeup_waiting_sender(kp);
		while (!queue_empty(&kp->port_blocked_threads));
#endif	

	/* Must have port locked to access sender_entry. */

	entry = kmsg->sender_entry;
	if (entry == PORT_HASH_NULL)
		port_unlock(kp);
	else {
		task_t task = entry->task;
		port_name_t name = entry->local_name;

		assert(entry->kmsg == kmsg);
		assert(entry->obj == (kern_obj_t) kp);

		/* Take a task ref, so it won't disappear on us. */
		task_reference(task);

		/* Clear the SEND_NOTIFY flag; don't need to have
		   the task locked because it is special. */
		entry->kmsg = KERN_MSG_NULL;

		/* Need to unlock port before sending notification. */
		port_unlock(kp);

		send_notification(task, NOTIFY_MSG_ACCEPTED, name);
		task_deallocate(task);
	}
    }

	*kmsgptr = kmsg;
	return RCV_SUCCESS;
}

/*
 *	Routine:	msg_send [exported]
 */
msg_return_t
msg_send(msgptr, option, tout)
	msg_header_t *msgptr;
	msg_option_t option;
	msg_timeout_t tout;
{
	msg_size_t msg_size;
	msg_return_t mr;

	assert(current_thread()->ipc_kernel);

	msg_size = msgptr->msg_size;

	while ((mr = msg_send_trap(msgptr, option, msg_size, tout))
				== SEND_INTERRUPTED) {
		while(thread_should_halt(current_thread()))
			thread_halt_self();

		if (option & SEND_INTERRUPT)
			break;
	}
	return mr;
}

#if	MACH_IPC_XXXHACK
/*
 *	Routine:	msg_send_old [obsolete trap]
 */
msg_return_t
msg_send_old(msgptr, option, tout)
	msg_header_t *msgptr;
	msg_option_t option;
	msg_timeout_t tout;
{
	msg_size_t msg_size;

	if (current_thread()->ipc_kernel)
		msg_size = msgptr->msg_size;
	else {
		if (copyin((caddr_t) &msgptr->msg_size,
			   (caddr_t) &msg_size,
			   sizeof(msg_size_t)))
			return SEND_INVALID_MEMORY;
	}

	return msg_send_trap(msgptr, option, msg_size, tout);
}
#endif	

/*
 *	Routine:	msg_send_trap [exported, trap]
 */
msg_return_t
msg_send_trap(msgptr, option, size, tout)
	msg_header_t *msgptr;
	msg_option_t option;
	msg_size_t size;
	msg_timeout_t tout;
{
	kern_msg_t new_kmsg;
	register msg_return_t result;

	result = msg_copyin(current_thread()->task, msgptr, size, &new_kmsg);
	if (result == SEND_SUCCESS)
		result = msg_queue(new_kmsg, option & SEND_USER, tout);

	return result;
}

/*
 *	Routine:	msg_send_from_kernel [exported]
 */
msg_return_t
msg_send_from_kernel(msgptr, option, tout)
	msg_header_t *msgptr;
	msg_option_t option;
	msg_timeout_t tout;
{
	kern_msg_t new_kmsg;
	register msg_return_t result;
	boolean_t old_ipc_kernel;
	register boolean_t *ipc_kernel_state = &current_thread()->ipc_kernel;
	extern msg_option_t msg_send_from_kernel_options;

	old_ipc_kernel = *ipc_kernel_state;
	*ipc_kernel_state = TRUE;
	result = msg_copyin(kernel_task, msgptr, msgptr->msg_size, &new_kmsg);
	*ipc_kernel_state = old_ipc_kernel;

	if (result == SEND_SUCCESS)
		result = msg_queue(
			    new_kmsg,
			    option | SEND_ALWAYS | msg_send_from_kernel_options,
			    tout);
	else
	 	printf("msg_send_from_kernel: bad message, %d\n", result);

	return result;
}
msg_option_t msg_send_from_kernel_options = SEND_SWITCH;

/*
 *	Routine:	msg_receive [exported]
 */
msg_return_t
msg_receive(header, option, tout)
	msg_header_t *header;
	msg_option_t option;
	msg_timeout_t tout;
{
	msg_size_t size;
	port_t name;
	msg_return_t mr;

	assert(current_thread()->ipc_kernel);

	size = header->msg_size;
	name = header->msg_local_port;

	while ((mr = msg_receive_trap(header, option, size, name, tout))
				== RCV_INTERRUPTED) {
		while(thread_should_halt(current_thread()))
			thread_halt_self();

		if (option & RCV_INTERRUPT)
			break;
	}
	return mr;
}

#if	MACH_IPC_XXXHACK
/*
 *	Routine:	msg_receive_old [obsolete trap]
 */
msg_return_t
msg_receive_old(header, option, tout)
	msg_header_t *header;
	msg_option_t option;
	msg_timeout_t tout;
{
	msg_size_t size;
	port_t name;

	if (current_thread()->ipc_kernel) {
		size = header->msg_size;
		name = header->msg_local_port;
	} else {
		msg_header_t usermsg;

		if (copyin((caddr_t) header,
			   (caddr_t) &usermsg,
			   sizeof(msg_header_t)))
			return RCV_INVALID_MEMORY;

		size = usermsg.msg_size;
		name = usermsg.msg_local_port;
	}

	return msg_receive_trap(header, option, size, name, tout);
}
#endif	

/*
 *	Routine:	msg_receive_trap [exported, trap]
 */
msg_return_t
msg_receive_trap(header, option, size, name, tout)
	msg_header_t *header;
	msg_option_t option;
	msg_size_t size;
	port_name_t name;
	msg_timeout_t tout;
{
	register msg_queue_t *mq;
	register task_t self = current_task();
	register kern_obj_t obj;
	register msg_return_t result;
	kern_msg_t kmsg;
#if	MACH_IPC_TCACHE
	unsigned int index;
#endif	

	/*
	 *	Find the global port name for the desired port,
	 *	and verify that the requesting task has receive rights.
	 */

	/* Need to have the task locked to do name->port lookups. */
	ipc_task_lock(self);

	if (name == PORT_NULL) {
		ipc_task_unlock(self);
		return RCV_INVALID_PORT;
#if	MACH_IPC_TCACHE
	} else if (self->obj_cache[index = obj_cache_index(name)].name
						== name) {
		register kern_port_t port;

		assert(self->ipc_active);
		assert(! self->kernel_ipc_space);

		obj = self->obj_cache[index].object;
		port = (kern_port_t) obj;
		port_lock(port);

		if ((port->port_receiver != self) ||
		    (port->port_set != KERN_SET_NULL)) {
			port_unlock(port);
			ipc_task_unlock(self);
			return RCV_INVALID_PORT;
		}

		mq = &port->port_messages;
#endif	
	} else if (!self->ipc_active) {
		ipc_task_unlock(self);
		return RCV_INVALID_PORT;
#if	MACH_IPC_XXXHACK
	} else if (name == PORT_ENABLED) {
		register kern_set_t set;

		set = self->ipc_enabled;
		obj = (kern_obj_t) set;
		assert(set != KERN_SET_NULL);
		set_lock(set);

		/* the set might be dead if the user deallocated it */
		if (!set->set_in_use) {
			set_unlock(set);
			ipc_task_unlock(self);
			return RCV_INVALID_PORT;
		}

		mq = &set->set_messages;
#endif	
	} else if (self->kernel_ipc_space) {
		register kern_port_t port;

		obj = (kern_obj_t) name;
		port = (kern_port_t) obj;
		port_lock(port);

		if ((port->port_receiver != self) ||
		    (port->port_set != KERN_SET_NULL)) {
			obj_unlock(obj);
			ipc_task_unlock(self);
			return RCV_INVALID_PORT;
		}

		mq = &port->port_messages;
	} else {
		register port_hash_t entry;

		obj_entry_lookup_macro(self, name, entry,
			{
				ipc_task_unlock(self);
				return RCV_INVALID_PORT;
			});

		obj = entry->obj;

		switch (entry->type) {
		    case PORT_TYPE_SEND:
#if	MACH_IPC_XXXHACK
		    case PORT_TYPE_OWN:
#endif	
			ipc_task_unlock(self);
			return RCV_INVALID_PORT;

#if	MACH_IPC_XXXHACK
		    case PORT_TYPE_RECEIVE:
#endif	
		    case PORT_TYPE_RECEIVE_OWN: {
			register kern_port_t port = (kern_port_t) obj;

			port_lock(port);

			assert(port->port_in_use);
			assert(port->port_receiver == self);

			if (port->port_set != KERN_SET_NULL) {
				port_unlock(port);
				ipc_task_unlock(self);
				return RCV_INVALID_PORT;
			}

			obj_cache_set(self, name, obj);
			mq = &port->port_messages;
			break;
		    }

		    case PORT_TYPE_SET: {
			register kern_set_t set = (kern_set_t) obj;

			set_lock(set);
			assert(set->set_in_use);
			assert(set->set_owner == self);

			mq = &set->set_messages;
			break;
		    }

		    default:
			panic("msg_receive_trap: strange translation type");
		}
	}

	ipc_task_unlock(self);
	assert(obj->obj_in_use);
	obj->obj_references++;
	msg_queue_lock(mq);
	obj_unlock(obj);

	result = msg_dequeue(mq, size, option, tout, &kmsg);
	/* mq is unlocked */
	if (result == RCV_SUCCESS)
		result = msg_copyout(self, header, kmsg);
	else if (result == RCV_TOO_LARGE) {
		if (current_thread()->ipc_kernel)
			header->msg_size = (msg_size_t) kmsg;
		else {
			if (copyout((caddr_t) &kmsg,
				    (caddr_t) &header->msg_size,
				    sizeof (msg_size_t)))
				result = RCV_INVALID_MEMORY;
		}
	}

	obj_release(obj);
	return result;
}

/*
 *	Routine:	msg_rpc [exported]
 */
msg_return_t
msg_rpc(header, option, rcv_size, send_timeout, rcv_timeout)
	msg_header_t *header;	/* in/out */
	msg_option_t option;
	msg_size_t rcv_size;
	msg_timeout_t send_timeout;
	msg_timeout_t rcv_timeout;
{
	msg_size_t send_size;
	msg_return_t mr;

	assert(current_thread()->ipc_kernel);

	send_size = header->msg_size;

	while ((mr = msg_rpc_trap(header, option, send_size, rcv_size,
				  send_timeout, rcv_timeout))
				== SEND_INTERRUPTED) {
		while (thread_should_halt(current_thread()))
			thread_halt_self();

		if (option & SEND_INTERRUPT)
			return mr;
	}
	if ((mr == RCV_INTERRUPTED) &&
	    !(option & RCV_INTERRUPT)) {
		port_name_t rcv_name = header->msg_local_port;

		do {
			mr = msg_receive_trap(header, option, rcv_size,
					      rcv_name, rcv_timeout);

			while (thread_should_halt(current_thread()))
				thread_halt_self();
		} while (mr == RCV_INTERRUPTED);
	}

	return mr;
}

#if	MACH_IPC_XXXHACK
/*
 *	Routine:	msg_rpc_old [obsolete trap]
 */
msg_return_t
msg_rpc_old(header, option, rcv_size, send_timeout, rcv_timeout)
	msg_header_t *header;	/* in/out */
	msg_option_t option;
	msg_size_t rcv_size;
	msg_timeout_t send_timeout;
	msg_timeout_t rcv_timeout;
{
	msg_size_t send_size;

	if (current_thread()->ipc_kernel)
		send_size = header->msg_size;
	else {
		if (copyin((caddr_t) &header->msg_size,
			   (caddr_t) &send_size,
			   sizeof(msg_size_t)))
			return SEND_INVALID_MEMORY;
	}

	return msg_rpc_trap(header, option, send_size, rcv_size,
			    send_timeout, rcv_timeout);
}
#endif	/* MACH_IPC_XXXHACK */

/*
 *	Routine:	msg_rpc_trap [exported, trap]
 */
msg_return_t
msg_rpc_trap(header, option, send_size, rcv_size, send_timeout, rcv_timeout)
	msg_header_t *header;	/* in/out */
	msg_option_t option;
	msg_size_t send_size;
	msg_size_t rcv_size;
	msg_timeout_t send_timeout;
	msg_timeout_t rcv_timeout;
{
	register msg_return_t result;
	register kern_port_t rp;
	register msg_queue_t *mq;
	register task_t self = current_task();
	kern_msg_t kmsg;

	result = msg_copyin(self, header, send_size, &kmsg);
	if (result != SEND_SUCCESS)
		return result;

	rp = (kern_port_t) kmsg->kmsg_header.msg_remote_port;
	if (rp == KERN_PORT_NULL) {
		/* We know the receive portion will fail, but must still
		   try the send portion first. */

		result = msg_queue(kmsg, (option & SEND_USER) | SEND_SWITCH,
				   send_timeout);
		if (result == SEND_SUCCESS)
			return RCV_INVALID_PORT;
		else
			return result;
	}

	/* Optimize rpc to kernel case: */

	if (kmsg->kernel_message) {
		register kern_msg_t out_msg;
		msg_size_t size;

		/* Here we simulate what msg_queue would do with the
		   request msg.  Because there is a reply port,
		   mach_msg will return a reply message. */

		out_msg = mach_msg(kmsg);
		assert(out_msg != KERN_MSG_NULL);
		assert(rp == (kern_port_t)
				out_msg->kmsg_header.msg_local_port);
		size = out_msg->kmsg_header.msg_size;

		/* Instead of calling msg_queue for the reply msg
		   and msg_dequeue on the reply port, check to see
		   if we can receive the reply directly (the normal case).
		   If not, queue the reply and jump to the receive code. */

		port_lock(rp);
		if ((rp->port_receiver != self) ||
		    (rp->port_message_count != 0) ||
		    (rp->port_set != KERN_SET_NULL) ||
		    (rcv_size < size)) {
			port_unlock(rp);
			port_reference(rp);
			(void) msg_queue(out_msg, SEND_ALWAYS, 0);
			goto receive_reply;
		}
		port_unlock(rp);

		return msg_copyout(self, header, out_msg);
	}

	/* Take a reference, so the reply port doesn't go away on us. */
	port_reference_macro(rp);

	result = msg_queue(kmsg, (option & SEND_USER) | SEND_SWITCH,
			   send_timeout);
     	if (result != SEND_SUCCESS) {
		port_release(rp);
		return result;
	}

receive_reply:

	/* Lock the reply port and check that we can receive from it. */

	port_lock(rp);
	if ((rp->port_receiver != self) ||
	    (rp->port_set != KERN_SET_NULL)) {
		rp->port_references--;
		port_check_unlock(rp);
		return RCV_INVALID_PORT;
	}

	assert(rp->port_in_use);
	mq = &rp->port_messages;
	msg_queue_lock(mq);
	port_unlock(rp);

	result = msg_dequeue(mq, rcv_size, option, rcv_timeout, &kmsg);
	/* mq is unlocked */
	if (result == RCV_SUCCESS)
		result = msg_copyout(self, header, kmsg);
	else if (result == RCV_TOO_LARGE) {
		if (current_thread()->ipc_kernel)
			header->msg_size = (msg_size_t) kmsg;
		else {
			if (copyout((caddr_t) &kmsg,
				    (caddr_t) &header->msg_size,
				    sizeof (msg_size_t)))
				result = RCV_INVALID_MEMORY;
		}
	}

	port_release_macro(rp);
	return result;
}
