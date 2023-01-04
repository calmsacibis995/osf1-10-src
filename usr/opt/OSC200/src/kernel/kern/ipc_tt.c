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
static char	*rcsid = "@(#)$RCSfile: ipc_tt.c,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/04/06 21:05:37 $";
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
 * File:	ipc_tt.c
 * Purpose:
 *	Task and thread related IPC functions.

 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 */

#include <sys/secdefines.h>
#include <sys/types.h>
#include <mach_km.h>
#include <mach_ipc_tcache.h>
#include <mach_ipc_xxxhack.h>

#include <mach/boolean.h>
#include <mach/port.h>
#include <kern/queue.h>
#include <kern/lock.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/kern_obj.h>
#include <kern/kern_port.h>
#include <kern/kern_set.h>
#include <kern/ipc_copyout.h>
#include <kern/ipc_hash.h>
#include <kern/ipc_cache.h>
#include <kern/ipc_mports.h>
#include <kern/ipc_globals.h>
#include <kern/ipc_tt.h>
#include <sys/proc.h>		/* for port_secure, uggh */
#include <sys/user.h>		/* for port_secure, uggh */

#if	MACH_KM
#include <kern/kern_mon.h>
#endif	
#if     SEC_BASE
#include <sys/security.h>
#endif

#if	MACH_IPC_TCACHE
/*
 * Initializes the task's cache.
 * The task doesn't have to be locked if nobody can get at it.
 */
void
obj_cache_init(t)
	register task_t t;
{
	unsigned int i;

	for (i = 0; i < OBJ_CACHE_MAX; i++)
		obj_cache_clear(t, i);
}
#endif	

/*
 *	Routine:	ipc_task_init [exported]
 *	Purpose:
 *		Initialize this task's IPC state.
 */
void
ipc_task_init(t, parent)
	register task_t t;
	task_t parent;
{
	kern_port_t port;
	int i;

	t->ipc_privilege = FALSE;
	t->ipc_intr_msg = FALSE;

	t->ipc_next_name = 1;	/* start with 1 to skip PORT_NULL */

	obj_cache_init(t);

	queue_init(&t->ipc_translations);

	simple_lock_init(&t->ipc_translation_lock);

	t->ipc_active = TRUE;

	/*
	 *	The important fields (lock, active, translation queue)
	 *	must be initialized before this allocation.  This
	 *	is necessary when initializing kernel_task.
	 */
	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_task_init: kernel port allocate");
	port->port_references += 2; /* refs for saved pointers */
	port_unlock(port);
	t->task_self = (task_port_t) port;
	t->task_tself = (task_port_t) port;

	/* Task will be set as port object in ipc_task_enable. */

	if (port_alloc(t, &port) != KERN_SUCCESS)
		panic("ipc_task_init: notify port allocate");
	port->port_references++; /* ref for saved pointer */
	port_unlock(port);
	t->task_notify = (task_port_t) port;

	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
		t->ipc_ports_registered[i] = PORT_NULL;

#if	MACH_IPC_XXXHACK
	if (set_alloc(t, &t->ipc_enabled) != KERN_SUCCESS)
		panic("ipc_task_init: enabled set allocate");
	t->ipc_enabled->set_references++; /* take ref for the task */
	set_unlock(t->ipc_enabled);
#endif	

	if (parent != TASK_NULL) {
	    port_t port_temp;

	    /* Inherit registered ports from parent. */

	    (void) mach_ports_register(t, parent->ipc_ports_registered,
				       (unsigned int) TASK_PORT_REGISTER_MAX);

	    ipc_task_lock(parent);

	    /* Inherit exception port from parent. */

	    if (parent->exception_port != PORT_NULL)
		port_reference((kern_port_t) parent->exception_port);
	    t->exception_port = parent->exception_port;

	    /* Inherit bootstrap port from parent. */

	    if (parent->bootstrap_port != PORT_NULL)
		port_reference((kern_port_t) parent->bootstrap_port);
	    t->bootstrap_port = parent->bootstrap_port;

	    ipc_task_unlock(parent);

	    /* Copy out send rights for exception port. */

	    if (t->exception_port != PORT_NULL) {
		port_reference((kern_port_t) t->exception_port);
		object_copyout(t, (kern_obj_t) t->exception_port,
			       MSG_TYPE_PORT, &port_temp);
	    }

	    /* Copy out send rights for bootstrap port. */

	    if (t->bootstrap_port != PORT_NULL) {
		port_reference((kern_port_t) t->bootstrap_port);
		object_copyout(t, (kern_obj_t) t->bootstrap_port,
			       MSG_TYPE_PORT, &port_temp);
	    }
	} else {
	    t->exception_port = PORT_NULL;
	    t->bootstrap_port = PORT_NULL;
	}
}

/*
 *	Routine: ipc_task_enable [exported]
 *
 *	Purpose: enable task operations via ipc.
 *
 *	Assumptions: ipc_task_init has been called.
 */
void
ipc_task_enable(t)
	task_t t;
{
	ipc_task_lock(t);
	if (t->ipc_active) {
		port_object_set((kern_port_t) t->task_self,
				PORT_OBJECT_TASK, (int) t);
		task_reference(t);
	}
	ipc_task_unlock(t);
}

/*
 *	Routine: ipc_task_disable [exported]
 *
 *	Purpose: disable task operations via ipc.
 *
 *	Assumptions: Caller holds a reference to task.
 */
void
ipc_task_disable(t)
	task_t t;
{
	kern_port_t myport;

	ipc_task_lock(t);
	if (t->ipc_active) {
		myport = (kern_port_t) t->task_self;
		port_lock(myport);
		if (myport->port_object.kp_type == PORT_OBJECT_TASK) {
			task_deallocate(t);
			port_object_set(myport, PORT_OBJECT_NONE, 0);
		}
		port_unlock(myport);
	}
	ipc_task_unlock(t);
}

/*
 *	Routine:	ipc_task_terminate [exported]
 *	Purpose:
 *		Shut down the given task's IPC access.
 *
 *	Assumptions:
 *		Task must be suspended (or current thread must
 *		be in the task).
 */
void
ipc_task_terminate(task)
	register task_t task;
{
	register int i;

	ipc_task_lock(task);
	if (!task->ipc_active) {
		ipc_task_unlock(task);
		return;
	}
	task->ipc_active = FALSE;
	obj_cache_terminate(task);
	ipc_task_unlock(task);

	/* Release references held by the kernel. */

	if ((kern_port_t) task->task_tself != KERN_PORT_NULL)
		port_release((kern_port_t) task->task_tself);

	if ((kern_port_t) task->task_notify != KERN_PORT_NULL)
		port_release((kern_port_t) task->task_notify);

	if (task->exception_port != PORT_NULL) {
		port_release((kern_port_t) task->exception_port);
		task->exception_port = PORT_NULL;
	}

	if (task->bootstrap_port != PORT_NULL) {
		port_release((kern_port_t) task->bootstrap_port);
		task->bootstrap_port = PORT_NULL;
	}

	ipc_task_lock(task);

#if	MACH_IPC_XXXHACK
	/* Release enabled port set. */

	set_release(task->ipc_enabled);
	task->ipc_enabled = KERN_SET_NULL; /* neatness */
#endif	

	/* Release registration references */

	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
		if (task->ipc_ports_registered[i] != PORT_NULL)
			port_release((kern_port_t)
				     task->ipc_ports_registered[i]);

	/* Eliminate rights held by this task. */

	while (!queue_empty(&task->ipc_translations)) {
		register port_hash_t entry =
			(port_hash_t) queue_first(&task->ipc_translations);

		switch (entry->type) {
		      case PORT_TYPE_SEND:
#if	MACH_IPC_XXXHACK
		      case PORT_TYPE_RECEIVE:
		      case PORT_TYPE_OWN:
#endif	
		      case PORT_TYPE_RECEIVE_OWN:
			ipc_task_unlock(task);
			(void) port_dealloc(task, (kern_port_t) entry->obj);
			ipc_task_lock(task);
			break;

		      case PORT_TYPE_SET:
			set_lock((kern_set_t) entry->obj);
			set_destroy(task, (kern_set_t) entry->obj, entry);
			ipc_task_lock(task);
			break;

		      default:
			panic("strange translation record");
		}
	}

	ipc_task_unlock(task);

	/* Release rights held by the kernel. */

	(void) port_dealloc(kernel_task, (kern_port_t) task->task_self);
	port_release((kern_port_t) task->task_self);
}

/*
 *	Routines from this point on need the ipc_thread_lock,
 *	to interlock references to thread->self.
 */

/*
 *	Routine:	ipc_thread_init [exported]
 *	Purpose:
 *		Initialize thread IPC state.
 */
void
ipc_thread_init(th)
	register thread_t th;
{
	kern_port_t port;

	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_thread_init: kernel port allocate");
	port->port_references += 2; /* refs for saved pointers */
	port_unlock(port);
	th->thread_self = (thread_port_t) port;
	th->thread_tself = (thread_port_t) port;

	if (port_alloc(th->task, &port) != KERN_SUCCESS)
		panic("ipc_thread_init: reply port allocate");
	port->port_references++; /* ref for saved pointer */
	port_unlock(port);
	th->thread_reply = (thread_port_t) port;

	/* Thread will be set as port object in ipc_thread_enable. */

	th->ipc_state = KERN_SUCCESS;
	simple_lock_init(&th->ipc_state_lock);
	th->exception_port = PORT_NULL;
	th->exception_clear_port = PORT_NULL;

	th->reply_port = PORT_NULL;
}

/*
 *	Routine: ipc_thread_enable [exported]
 *
 *	Purpose: enable thread operations via ipc.
 *
 *	Assumptions: ipc_thread_init has been called.
 */
void
ipc_thread_enable(th)
	thread_t th;
{
	ipc_thread_lock(th);
	if (th->thread_self != PORT_NULL) {
		port_object_set((kern_port_t) th->thread_self,
				PORT_OBJECT_THREAD, (int) th);
		thread_reference(th);
	}
	ipc_thread_unlock(th);
}


/*
 *	Routine: ipc_thread_disable [exported]
 *
 *	Purpose: disable thread operations via ipc.
 *
 *	Assumptions: Caller holds a reference to thread.
 */
void
ipc_thread_disable(th)
	thread_t th;
{
	kern_port_t myport;

	ipc_thread_lock(th);
	if (th->thread_self != PORT_NULL) {
		myport = (kern_port_t) th->thread_self;
		port_lock(myport);
		if(myport->port_object.kp_type == PORT_OBJECT_THREAD) {
			thread_deallocate(th);
			port_object_set(myport, PORT_OBJECT_NONE, 0);
		}
		port_unlock(myport);
	}
	ipc_thread_unlock(th);
}

/*
 *	Routine:	ipc_thread_terminate [exported]
 *	Purpose:
 *		Shut down thread IPC structures.
 *	Assumptions:
 *		Thread must be suspended (or be the current thread).
 */
void
ipc_thread_terminate(th)
	register thread_t th;
{
	register thread_port_t self, tself, reply, exc;

	ipc_thread_lock(th);

	if ((self = th->thread_self) == PORT_NULL) {
		ipc_thread_unlock(th);
		return;
	}

	th->thread_self = PORT_NULL;
	tself = th->thread_tself;
	th->thread_tself = PORT_NULL;
	reply = th->thread_reply;
	th->thread_reply = PORT_NULL;
	exc = th->exception_port;
	th->exception_port = PORT_NULL;

	ipc_thread_unlock(th);

	/* Release references. */

	if (tself != PORT_NULL) {
		port_release((kern_port_t) tself);
	}
	if (exc != PORT_NULL) {
		port_release((kern_port_t) exc);
	}

	if (reply != PORT_NULL) {
		(void) port_dealloc(th->task, (kern_port_t) reply);
		port_release((kern_port_t) reply);
	}

	/*
	 *	exception_clear_port is a per thread port allocated in the
	 *	thread's task.  Code to dispose of it has been moved to
	 *	thread_exception_abort.
	 */
	
	if (th->exception_clear_port != PORT_NULL)
		thread_exception_abort(th);

	/*
	 *	Release kernel's right to the thread port;
	 *	deallocate in th->task as an optimization.
	 */

	(void) port_dealloc(th->task, (kern_port_t) self);
	(void) port_dealloc(kernel_task, (kern_port_t) self);
	port_release((kern_port_t) self);

	th->ipc_state = KERN_SUCCESS;
}

/*
 *	Routine:	port_secure
 *	Purpose:
 *		Determine whether the given task has the only
 *		access rights to the port in question.
 */
boolean_t
port_secure(kp, task, kernel_references)
	register kern_port_t kp;
	task_t task;
	int kernel_references;
{
	register port_hash_t entry;
	boolean_t result = TRUE;
	int count = 0;

	port_lock(kp);
	for (entry = (port_hash_t) queue_first(&kp->port_translations);
	     !queue_end(&kp->port_translations, (queue_entry_t) entry);
	     entry = (port_hash_t) queue_next(&entry->obj_chain)) {
		register task_t sender = entry->task;

		if ((sender == task) ||
		    (sender == kernel_task))
			count++;	/* an OK send right */
		else {
			struct proc *p = sender->u_address->uu_procp;
#if SEC_BASE
/* XXX should define a new privilege for Mach IPC XXX */
			if (security_is_on) {
                        if (ISBITSET(secinfo[p - proc].si_epriv, SEC_DEBUG))
			        count++; /* an OK send right */
                        else {
			        result = FALSE;
                                break;
			      }
			} else
#endif
			{
			PROC_LOCK(p);
			if (p->p_rcred->cr_uid == 0) {
				PROC_UNLOCK(p);
				count++; /* an OK send right */
			} else {
				PROC_UNLOCK(p);
				result = FALSE;
				break;
			}
			}
		}
	}
	if (kp->port_references > (count + kernel_references))
		result = FALSE;
	port_unlock(kp);
	return result;
}

/*
 *	Routine:	exc_port_secure
 *	Purpose:
 *		Determine if an exception port is secure.
 *		This is the case if either the ux_exception task
 *		is the receiver, or a uid=0 task is the receiver.
 */
boolean_t
exc_port_secure(port)
	register kern_port_t port;
{
	extern task_t ux_handler_task; /* internal ux_exception task */

	register task_t task;
	boolean_t result;

	if (port == KERN_PORT_NULL)
		return TRUE;

	port_lock(port);
	if (!port->port_in_use)
		result = TRUE;
	else if ((task = port->port_receiver) == ux_handler_task)
		result = TRUE;
	else if (task == ipc_soft_task)
		result = FALSE;
	else {
		struct proc *p = task->u_address->uu_procp;
#if SEC_BASE
/* XXX Should define a new privilege for Mach IPC XXX */
		if (security_is_on) {
                if (ISBITSET(secinfo[p - proc].si_epriv, SEC_DEBUG))
                        result = TRUE;
		else
		        result = FALSE;
		} else
#endif
		{
		PROC_LOCK(p);
		if (p->p_rcred->cr_uid == 0)
			result = TRUE;
		else
			result = FALSE;
		PROC_UNLOCK(p);
		}

	}
	port_unlock(port);

	return result;
}

/*
 *	Routine:	task_secure
 *	Purpose:
 *		Determine whether the given task can be manipulated
 *		by tasks other than itself, via the IPC interface.
 */
boolean_t
task_secure(task)
	task_t task;
{
	thread_t thread;
	boolean_t result;

	result = (port_secure((kern_port_t) task->task_self, task, 2) &&
		  exc_port_secure((kern_port_t) task->exception_port));
	if (result) {
		task_lock(task);
		for (thread = (thread_t) queue_first(&task->thread_list);
		     !queue_end(&task->thread_list, (queue_entry_t) thread);
		     thread = (thread_t) queue_next(&thread->thread_list)) {
			if (!port_secure((kern_port_t) thread->thread_self,
					 task, 2) ||
			    !exc_port_secure((kern_port_t)
					     thread->exception_port)) {
				result = FALSE;
				break;
			}
		}
		task_unlock(task);
	}
	return result;
}

#if	MACH_KM

/*
 *	Routine:	ipc_monitor_init [exported]
 *	Purpose:
 *		Initialize monitor IPC state.
 *	Assumptions: Caller has a valid monitor.
 *
 *      Callers: (may be incomplete list) monitor_create()
 */
void
ipc_monitor_init(m)
	register
	monitor_t	m;
{
	kern_port_t	port;


	if (port_alloc(kernel_task, &port) != KERN_SUCCESS)
		panic("ipc_monitor_init: kernel port allocate");
	port->port_references++; /* ref for saved pointer */
	port_unlock(port);
        m->monitor_self = (port_t) port;
}

/*
 *	Routine: ipc_monitor_enable [exported]
 *
 *	Purpose: enable monitor operations via ipc.
 *
 *	Assumptions: ipc_monitor_init has been called and the 
 *		global state of the monitor is MONITOR_PAUSE.
 *
 *      Callers: (may be incomplete list) monitor_create()
 */

void
ipc_monitor_enable(m)
monitor_t	m;
{
    kern_port_t	port;

    if ((port = (kern_port_t) m->monitor_self) != KERN_PORT_NULL){
	port_lock(port);
	port_object_set(port, PORT_OBJECT_MONITOR, (int)m);
	port_unlock(port);
	m->ref_count++;
    } else {
	panic("ipc_monitor_enable():  null monitor port");
    }
}


/*
 *	Routine: ipc_monitor_disable [exported]
 *
 *	Purpose: disable monitor operations via ipc.
 *
 *	Assumptions: Caller has lock on monitor (more than one
 *		caller may try have access to this monitor).
 *
 *      Callers: (may be incomplete list) monitor_create()
 */

void
ipc_monitor_disable(m)
monitor_t	m;
{
    kern_port_t	myport;
    port_t	self;

    if (m->monitor_self != PORT_NULL){
	myport = (kern_port_t) m->monitor_self;
	port_lock(myport);
	if(myport->port_object.kp_type == PORT_OBJECT_MONITOR) {
	    m->ref_count--;
	    port_object_set(myport, PORT_OBJECT_NONE, 0);
	}
	port_unlock(myport);
    } else {
	panic("ipc_monitor_disable():  null monitor port");
    }

}

/*
 *	Routine:	ipc_monitor_terminate [exported]
 *	Purpose:
 *		Shut down monitor IPC structures.
 *	Assumptions:
 *		Caller holds a monitor lock.
 */
void
ipc_monitor_terminate(m)
	register
	monitor_t	m;
{
	register
	kern_port_t		self;


	if ((self = (kern_port_t) m->monitor_self) == KERN_PORT_NULL) {
	    return;
	}
	m->monitor_self = PORT_NULL;

	/*
	 *	Release references
	 */

	port_release(self);


	/*
	 *	Release kernel's right to the monitor port;
	 *	deallocate in current task as an optimization.
	 */

	(void) port_dealloc(current_task(), self);
	(void) port_dealloc(kernel_task, self);
}

#endif	/* MACH_KM */
