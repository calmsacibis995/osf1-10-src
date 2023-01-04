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
static char	*sccsid = "@(#)$RCSfile: exception.c,v $ $Revision: 4.3.4.2 $ (DEC) $Date: 1993/05/22 17:40:34 $";
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

#include <sys/param.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/port.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <sys/user.h>
#include <kern/kern_obj.h>
#include <kern/kern_port.h>

#include <mach/exc.h>
#include <kern/ipc_copyout.h>

#include <kern/parallel.h>		/* Only for u*x_master */

/*
 *	thread_doexception does all the dirty work in actually setting
 *	up to send the exception message.  
 *
 *	XXX Can only be called on current thread, because it may exit().
 */
 
void thread_doexception(thread, exception, code, subcode)
thread_t	thread;
unsigned long	exception, code, subcode;
{
	task_t		task;
	port_name_t	task_port, thread_port;
	thread_port_t	clear_port, exc_port;
	int		signal, junk;	/* XXX for exit() */
	kern_return_t	r;
	k_siginfo_t	siginfo;

	if (thread != current_thread())
		panic("thread_doexception: thread is NOT current_thread!");
	thread_port = thread_self();
	task_port = task_self();
	task = thread->task;

	/*
	 *	Allocate an exception_clear port if there isn't one
	 *	already.  Only done by the thread itself.
	 */
	ipc_thread_lock(thread);
	if (thread->exception_clear_port == PORT_NULL) {
		kern_port_t port;

		if (port_alloc(task, &port) != KERN_SUCCESS)
			panic("thread_doexception: alloc clear port");
		port->port_references++; /* ref for saved pointer */
		port_unlock(port);

		thread->exception_clear_port = (thread_port_t) port;
	}

	/*
	 *	Now translate global port name from structure to local
	 *	port name.  Need an extra reference for object_copyout.
	 */
	clear_port = thread->exception_clear_port;
	port_reference((kern_port_t) clear_port);
	object_copyout(task, (kern_obj_t) clear_port,
		       MSG_TYPE_PORT, &clear_port);

	/*
	 *	Set ipc_kernel flag because msg buffers for rpc are in
	 *	kernel space.
	 */
	if (thread->ipc_kernel) {
	        printf("\nException = 0x%x, Code = 0x%x, SubCode = 0x%x\n",
		       exception, code, subcode);
		panic("Kernel thread exception.");
	}
	thread->ipc_kernel = TRUE;

	/*
	 *	Try thread port first.
	 */
	exc_port = thread->exception_port;
	if (exc_port != PORT_NULL) {
	    port_reference(exc_port);
	}
	ipc_thread_unlock(thread);

	if (exc_port != PORT_NULL) {
	    /*
	     *	Translate global name from data structure to local name.
	     */
	    object_copyout(task, (kern_obj_t) exc_port,
			   MSG_TYPE_PORT, &exc_port);

	    if ((r = exception_raise(exc_port, clear_port, thread_port,
				task_port, exception, code, subcode))
		== KERN_SUCCESS) {
		    /*
		     *	Turn off ipc_kernel before returning
		     */
		    thread->ipc_kernel = FALSE;
		    return;
	    }

	    /*
	     *	If RCV_INVALID_PORT is returned, the receive right to
	     *	the clear port has vanished.  Most likely cause is
	     *	thread_exception_abort.  Assume whoever did this
	     *  knows what they're doing and return.  Make sure the
	     *	exception port field in the thread structure is
	     *	cleared (thread_exception_abort() does this).
	     */
	    if (r == RCV_INVALID_PORT) {
		thread->ipc_kernel = FALSE;
		if (thread->exception_clear_port != PORT_NULL) 
			thread_exception_abort();
		return;
	    }
	}

	simple_lock(&task->ipc_translation_lock);
	exc_port = task->exception_port;
	if (exc_port != PORT_NULL) {
	    port_reference(exc_port);
	}
	simple_unlock(&task->ipc_translation_lock);

	if (exc_port != PORT_NULL) {

	    /*
	     *	Translate global name from data structure to local name.
	     */
	    object_copyout(task, (kern_obj_t) exc_port,
			   MSG_TYPE_PORT, &exc_port);

	    if ((r = exception_raise(exc_port, clear_port, thread_port,
				task_port, exception, code, subcode))
		== KERN_SUCCESS) {
		    /*
		     *	Reset ipc_kernel flag before returning.
		     */
		    thread->ipc_kernel = FALSE;
		    return;
	    }

	    /*
	     *	See above comment on RCV_INVALID_PORT.
	     */
	    if (r == RCV_INVALID_PORT) {
		thread->ipc_kernel = FALSE;
		if (thread->exception_clear_port != PORT_NULL) 
			thread_exception_abort();
		return;
	    }
	}
	/*
	 *	Failed to send outgoing message, reset ipc_kernel flag.
	 */
	thread->ipc_kernel = FALSE;

	/*
	 *	If this thread is being terminated, cooperate.
	 */
	while (thread_should_halt(thread))
		thread_halt_self();

	/*
	 *	All else failed; terminate task.
	 */

#if	1
	/*
	 *	Unfortunately, the rest of this is U*X code, since
	 *	task_terminate doesn't work on a U*X process yet.
	 */

	unix_master();		/* rest of this code is u*x */

	uprintf("Exception %d %d %d; no exception port, terminating task\n",
		exception, code, subcode);  /* for debugging */
	bzero(&siginfo,sizeof( siginfo ));
	ux_exception(exception, code, subcode, &signal, &junk, &siginfo);
	/* exit really should be task_terminate(task) */
	exit(current_task()->u_address->uu_procp, signal);
#else	/* 1 */
	(void) task_terminate(current_task());
	thread_halt_self();
#endif	/* 1 */
	/*NOTREACHED*/
}

/*
 *	thread_exception_abort:
 *
 *	Abort the exception receive of a thread waiting in the kernel
 *	by deallocating the exception_clear_port.
 */
kern_return_t thread_exception_abort(thread)
register thread_t thread;
{
	register port_t	exc_clear;

	ipc_thread_lock(thread);
	exc_clear = thread->exception_clear_port;
	thread->exception_clear_port = PORT_NULL;
	ipc_thread_unlock(thread);

	if (exc_clear == PORT_NULL) {
		/*
		 *	Thread couldn't be waiting in an exception
		 *	because it didn't have a port to wait on!
		 */
		return(KERN_FAILURE);
	}

	/*
	 *	Get rid of the port as well as the reference to it
	 *	held by the thread structure.  Ok to use thread->task
	 *	because caller must have a reference to thread.
	 */
	(void) port_dealloc(thread->task, (kern_port_t) exc_clear);
	port_release((kern_port_t) exc_clear);

	return(KERN_SUCCESS);
}
