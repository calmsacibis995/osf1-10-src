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
static char	*sccsid = "@(#)$RCSfile: ux_exception.c,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/07/07 19:43:17 $";
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
#include <sys/user.h>

#include <mach/boolean.h>
#include <mach/exception.h>
#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <kern/lock.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/ipc_pobj.h>
#include <kern/ipc_copyin.h>
#include <builtin/ux_exception.h>

#include <mach/exc.h>
#include <mach/mach_user_internal.h>

/*
 *	Unix exception handler.
 */

void	ux_exception();

decl_simple_lock_data(,	ux_handler_init_lock)
port_t			ux_exception_port;

/*
 *	Messages.  Receive message must be incredibly large just in case
 *	some bogon sends a truly enormous message to this port.
 */
static struct {
    msg_header_t	h;
    char		d[MSG_SIZE_MAX - (sizeof(msg_header_t))];
} exc_msg;

static struct {
    death_pill_t	pill;
    int			d[2];
} rep_msg;

port_t	ux_handler_task_self;
task_t	ux_handler_task;

void	ux_handler()
{
	register kern_return_t	r;
	port_name_t		ux_notify_port;
	port_name_t		ux_local_port;
	port_set_name_t		ux_set;
	extern void		task_name();

	ux_handler_task = current_task();
	ux_handler_task->kernel_vm_space = TRUE;
	current_thread()->ipc_kernel = TRUE;

	simple_lock(&ux_handler_init_lock);

	ux_handler_task_self = task_self();

	/*
	 *	Allocate a port set that we will receive on.
	 */
	r = port_set_allocate(ux_handler_task_self, &ux_set);
	if (r != KERN_SUCCESS)
		panic("ux_handler: port_set_allocate failed");

	/*
	 *	Allocate an exception port and use object_copyin to
	 *	translate it to the global name.  Put it into the set.
	 */
	r = port_allocate(ux_handler_task_self, &ux_local_port);
	if (r != KERN_SUCCESS)
		panic("ux_handler: port_allocate failed");
	r = port_set_add(ux_handler_task_self, ux_set, ux_local_port);
	if (r != KERN_SUCCESS)
		panic("ux_handler: port_set_add failed");
	ux_exception_port = ux_local_port;
	if (!object_copyin(ux_handler_task, ux_exception_port,
			   MSG_TYPE_PORT, FALSE,
			   (kern_obj_t *) &ux_exception_port))
		panic("ux_handler: object_copyin(ux_exception_port) failed");

	/*
	 *	Get a hold of our notify port.  Put it into the set.
	 */
	ux_notify_port = task_notify();
	r = port_set_add(ux_handler_task_self, ux_set, ux_notify_port);
	if (r != KERN_SUCCESS)
		panic("ux_handler: port_set_add failed");

	/*
	 *	Release kernel to continue.
	 */
	thread_wakeup((vm_offset_t)&ux_exception_port);
	simple_unlock(&ux_handler_init_lock);

	task_name("exception hdlr");

	/* Message handling loop. */

 	for (;;) {
		exc_msg.h.msg_local_port = ux_set;
		exc_msg.h.msg_size = sizeof(exc_msg);

		r = msg_receive(&exc_msg.h, MSG_OPTION_NONE, 0);
		if (r != RCV_SUCCESS) {
			printf("error code %d\n", r);
			panic("exception_handler: receive failed");
		}
			
		if (exc_msg.h.msg_local_port == ux_notify_port)
			/* ignore notifications */;
		else if (exc_msg.h.msg_local_port == ux_local_port) {
			register port_name_t	rep_port;

			(void) exc_server(&exc_msg.h, &rep_msg.pill.Head);
			rep_port = rep_msg.pill.Head.msg_remote_port;
			if ((rep_port != PORT_NULL) &&
			    (rep_msg.pill.RetCode != MIG_NO_REPLY)) {
				/* might fail if client died */
				/* XXX might block? */
				(void) msg_send(&rep_msg.pill.Head,
						MSG_OPTION_NONE, 0);

				/* XXX rep_port might be important */
				(void) port_deallocate(ux_handler_task_self,
						       rep_port);
			}
		} else
			panic("ux_handler: strange port");
	}
}

kern_return_t
catch_exception_raise(exception_port, thread_port, task_port,
	exception, code, subcode)
port_t		exception_port, thread_port, task_port;
unsigned long	exception, code, subcode;
{
    thread_t	thread;
    task_t	task;
    int		signal = 0;
    int		ret = KERN_SUCCESS;
    k_siginfo_t	siginfo;

#ifdef	lint
    exception_port++;
#endif	lint

    /*
     *	Convert local port names to structure pointers.  Have object_copyin
     *	deallocate our rights to the ports.  (it returns a reference).
     */
    if (!object_copyin(ux_handler_task, task_port,
		       MSG_TYPE_PORT, TRUE,
		       (kern_obj_t *) &task_port))
	return(KERN_INVALID_ARGUMENT);

    if (!object_copyin(ux_handler_task, thread_port,
		       MSG_TYPE_PORT, TRUE,
		       (kern_obj_t *) &thread_port)) {
	port_release(task_port);
	return(KERN_INVALID_ARGUMENT);
    }

    task = convert_port_to_task(task_port);
    thread = convert_port_to_thread(thread_port);

    /*
     *	Catch bogus ports
     */
    if (task != TASK_NULL && thread != THREAD_NULL) {
	    siginfo = zero_ksiginfo;

	    /*
	     *	Convert exception to unix signal and code.
	     */
	    ux_exception(exception, code, subcode, &signal,
			 &thread->u_address.uthread->uu_code, &siginfo);

	    /*
	     *	Send signal.
	     */
	    if (signal != 0) {
		/* ok to sleep in thread_psignal() when called from here */
		if (siginfo.si_signo == -1) {
			/*
			 * signo of -1 indicates there is no siginfo
			 * information for this exception, so don't
			 * pass a siginfo pointer to thread_psignal()
			 */
			thread_psignal(thread, signal, NULL);
		} else {
			siginfo.si_signo = signal;
			thread_psignal(thread, signal, &siginfo);
		}
	    }
    }
    else {
	    ret = KERN_INVALID_ARGUMENT;
    }

    /*
     *	Delete the references acquired in the convert routines.
     */
    if (task != TASK_NULL) 
	    task_deallocate(task);

    if (thread != THREAD_NULL)
	    thread_deallocate(thread);

    /*
     *	Delete the port references that came from port_copyin.
     */
    port_release(task_port);
    port_release(thread_port);

    return(ret);
}


boolean_t	machine_exception();

/*
 *	ux_exception translates a mach exception, code and subcode to
 *	a signal and u.u_code.  Calls machine_exception (machine dependent)
 *	to attempt translation first.
 */

void ux_exception(exception, code, subcode, ux_signal, ux_code, siginfo_p)
unsigned long	exception, code, subcode;
int	*ux_signal, *ux_code;
register k_siginfo_t *siginfo_p;
{
	/*
	 *	Try machine-dependent translation first.
	 */
	if (machine_exception(exception, code, subcode, ux_signal, 
			      ux_code, siginfo_p))
		return;
	
	/*
	 * The subcode is about always the address for the following signals.
	 * When not, this field won't be looked at in any case.
	 */
	siginfo_p->si_addr = (caddr_t) subcode;
	siginfo_p->si_errno = 0; /* set it here in case it is not set later */

	switch (exception) {
	case EXC_BAD_ACCESS:
		if (code == KERN_INVALID_ADDRESS)
			*ux_signal = SIGSEGV;
		else
			*ux_signal = SIGBUS;
		break;
	case EXC_BAD_INSTRUCTION:
	        *ux_signal = SIGILL;
		break;
	case EXC_ARITHMETIC:
	        *ux_signal = SIGFPE;
		break;
	case EXC_EMULATION:
		*ux_signal = SIGEMT;
		/* no siginfo for this signal */
		siginfo_p->si_signo = -1;
		break;
	case EXC_SOFTWARE:
		switch (code) {
		case EXC_UNIX_BAD_SYSCALL:
			*ux_signal = SIGSYS;
			break;
		case EXC_UNIX_BAD_PIPE:
		    	*ux_signal = SIGPIPE;
			break;
		case EXC_UNIX_ABORT:
			*ux_signal = SIGABRT;
			break;
		default:
			*ux_signal = SIGTRAP;
			break;
		}
		/* no siginfo for this signal */
		siginfo_p->si_signo = -1;
		break;
	case EXC_BREAKPOINT:
		*ux_signal = SIGTRAP;
		break;
	}
}
