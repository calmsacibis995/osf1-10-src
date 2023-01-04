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
static char	*sccsid = "@(#)$RCSfile: syscall_subr.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/05/22 17:40:47 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 03-May-91	Peter H. Smith
 *	When yielding, make sure the thread goes to the tail of the run
 *	queue.  Add assertions and remove hardcoded priorities for setting
 *	of sched_pri.  This may need revisiting...
 */

#include <mach_emulation.h>
#include <mach_sctimes.h>
#include <stat_time.h>
#include <cpus.h>
#include <rt_sched.h>
#include <rt_sched_rq.h>

#include <sys/types.h>
#include <kern/thread.h>
#include <mach/thread_switch.h>

#include <sys/user.h>
#include <sys/proc.h>

#include <mach/boolean.h>
#include <kern/ipc_pobj.h>
#include <kern/kern_port.h>
#include <mach/policy.h>
#include <kern/sched.h>
#include <kern/sched_prim_macros.h>
#include <kern/thread.h>
#if	MACH_EMULATION
#include <kern/syscall_emulation.h>
#endif
#include <machine/cpu.h>
#include <kern/processor.h>
#if	MACH_SCTIMES
#include <mach/port.h>
#include <vm/vm_kern.h>
#endif

/*
 *	Note that we've usurped the name "swtch" from its
 *	previous use.
 */

/*
 *	swtch and swtch_pri both attempt to context switch (logic in
 *	thread_block no-ops the context switch if nothing would happen).
 *	A boolean is returned that indicates whether there is anything
 *	else runnable.
 *
 *	This boolean can be used by a thread waiting on a
 *	lock or condition:  If FALSE is returned, the thread is justified
 *	in becoming a resource hog by continuing to spin because there's
 *	nothing else useful that the processor could do.  If TRUE is
 *	returned, the thread should make one more check on the
 *	lock and then be a good citizen and really suspend.
 */

void thread_depress_priority();
kern_return_t thread_depress_abort();

boolean_t swtch()
{
	register processor_t	myprocessor;
#if RT_SCHED
	register int s;
#endif /* RT_SCHED */

#if RT_SCHED
	/*
	 * If the thread is placed back on the head of the run queue, yielding
	 * won't have accomplished much.  Explicitly clear the processor's
	 * was_first_quantum flag.
	 *
	 * Note that we still have to get the current processor after blocking,
	 * because we may have moved to a different processor.
	 */
	s = splsched();
	current_processor()->was_first_quantum = FALSE;
#endif /* RT_SCHED */

	thread_block();

#if RT_SCHED
	splx(s);
#endif /* RT_SCHED */

	myprocessor = current_processor();
	return(myprocessor->runq.count > 0 ||
	       myprocessor->processor_set->runq.count > 0);
}

boolean_t  swtch_pri(pri)
	int pri;
{
	register boolean_t	result;
	register thread_t	thread = current_thread();
#ifdef	lint
	pri++;
#endif	

	/*
	 *	XXX need to think about depression duration.
	 *	XXX currently using min quantum.
	 */

#if	RT_SCHED
	if (thread->policy & (POLICY_FIFO|POLICY_RR)) {
		register processor_t	myprocessor;
		myprocessor = current_processor();
		result = myprocessor->runq.count > 0 ||
	       		myprocessor->processor_set->runq.count > 0;
		if (result) {
			assert_wait((vm_offset_t)swtch_pri, TRUE);
			thread_set_timeout(1);	/* shortest wait possible */
		}
	} else
#endif
		thread_depress_priority(thread, min_quantum);
	result = swtch();
	if (thread->depress_priority >= 0)
		thread_depress_abort(thread);
	return(result);
}

extern unsigned int timeout_scaling_factor;

/*
 *	thread_switch:
 *
 *	Context switch.  User may supply thread hint.
 *
 *	Fixed priority threads that call this get what they asked for
 *	even if that violates priority order.
 */
kern_return_t thread_switch(thread_name, option, option_time)
int	thread_name, option, option_time;
{
    register thread_t		cur_thread = current_thread();
    kern_port_t	port;

    /*
     *	Process option.
     */
    switch (option) {
	case SWITCH_OPTION_NONE:
	    /*
	     *	Nothing to do.
	     */
	    break;

	case SWITCH_OPTION_DEPRESS:
	    /*
	     *	Depress priority for given time.
	     */
	    thread_depress_priority(cur_thread, option_time);
	    break;

	case SWITCH_OPTION_WAIT:
	    thread_will_wait_with_timeout(cur_thread,
		(1000*option_time)/timeout_scaling_factor);
	    break;

	default:
	    return(KERN_INVALID_ARGUMENT);
    }
    
    /*
     *	Check and act on thread hint if appropriate.
     */
    if (thread_name != 0 &&
	port_translate(cur_thread->task, thread_name, &port)) {
	    /*
	     *	Get corresponding thread.
	     */
	    if (port_object_type(port) == PORT_OBJECT_THREAD) {
		register thread_t thread;
		register int s;

		thread = (thread_t) port_object_get(port);
		/*
		 *	Check if the thread is in the right pset. Then
		 *	pull it off its run queue.  If it
		 *	doesn't come, then it's not eligible.
		 */
		s = splsched();
		thread_lock(thread);
		if ((thread->processor_set == cur_thread->processor_set)
		    && (rem_runq(thread) != RUN_QUEUE_NULL)) {
			/*
			 *	Hah, got it!!
			 */
			thread_unlock(thread);
			(void) splx(s);
			port_unlock(port);
			if (thread->policy == POLICY_FIXEDPRI) {
			    register processor_t	myprocessor;

			    myprocessor = current_processor();
			    myprocessor->quantum = thread->sched_data;
			    myprocessor->first_quantum = FALSE;
			}
			thread_run(thread);
			/*
			 *  Restore depressed priority			 
			 */
			if (cur_thread->depress_priority >= 0)
				thread_depress_abort(cur_thread);

			return(KERN_SUCCESS);
		}
		thread_unlock(thread);
		(void) splx(s);
	    }
	    port_unlock(port);
    }

    /*
     *	No handoff hint supplied, or hint was wrong.  Call thread_block() in
     *	hopes of running something else.  If nothing else is runnable,
     *	thread_block will detect this.  WARNING: thread_switch with no
     *	option will not do anything useful if the thread calling it is the
     *	highest priority thread (can easily happen with a collection
     *	of timesharing threads).
     */
#if RT_SCHED
    /*
     * If the thread is placed back on the head of the run queue, yielding
     * won't have accomplished much.  Explicitly clear the processor's
     * was_first_quantum flag to keep it from going to the head.
     */

    {
      register int s;

      s = splsched();
      current_processor()->was_first_quantum = FALSE;
      thread_block();
      splx(s);
    }
#else /* RT_SCHED */
    thread_block();
#endif /* RT_SCHED */

    /*
     *  Restore depressed priority			 
     */
    if (cur_thread->depress_priority >= 0)
	thread_depress_abort(cur_thread);
    return(KERN_SUCCESS);
}

/*
 *	thread_depress_priority
 *
 *	Depress thread's priority to lowest possible for specified period.
 *	Intended for use when thread wants a lock but doesn't know which
 *	other thread is holding it.  As with thread_switch, fixed
 *	priority threads get exactly what they asked for.  Users access
 *	this by the SWITCH_OPTION_DEPRESS option to thread_switch.
 */
void
thread_depress_priority(thread, depress_time)
register thread_t thread;
int	depress_time;
{
    int		s;
    void	thread_depress_timeout();

    depress_time = (1000*depress_time)/timeout_scaling_factor;

    s = splsched();
    thread_lock(thread);

    /*
     *	If thread is already depressed, override previous depression.
     */
    if (thread->depress_priority >= 0) {
#if NCPUS > 1
	if (untimeout_try(thread_depress_timeout, thread) == FALSE) {
	    /*
	     *	Handle multiprocessor race condition.  Some other processor
	     *	is trying to timeout the old depress.  This should be rare.
	     */
	    thread_unlock(thread);
	    (void) splx(s);

	    /*
	     *	Wait for the timeout to do its thing.
	     */
	    while (thread->depress_priority >= 0)
	       continue;

	    /*
	     * Relock the thread and depress its priority.
	     */
	    s = splsched();
	    thread_lock(thread);

	    thread->depress_priority = thread->priority;
#if RT_SCHED_RQ
	    /*
	     * If sched_pri is changed while a thread is in a run queue, the
	     * run queue bitmask will be corrupted.  If this interface will
	     * be exposed to users, we need to do the priority depression by
	     * calling thread_priority or set_pri instead of stomping on the
	     * sched_pri field.
	     */
	    ASSERT(thread->runq == RUN_QUEUE_NULL);
#endif /* RT_SCHED_RQ */
	    /*
	     * RT_SCHED: Always valid -- replace hardcoded priorities with
	     * constants defined in kern/sched.h.
	     */
	    thread->priority = BASEPRI_LOWEST;
	    thread->sched_pri = BASEPRI_LOWEST;
	}
#else	/* NCPUS > 1 */
	untimeout(thread_depress_timeout, thread);
#endif	/* NCPUS > 1 */
    }
    else {
	/*
	 *	Save current priority, then set priority and
	 *	sched_pri to their lowest possible values.
	 */
	thread->depress_priority = thread->priority;
#if RT_SCHED_RQ
	/*
	 * If sched_pri is changed while a thread is in a run queue, the
	 * run queue bitmask will be corrupted.  If this interface will
	 * be exposed to users, we need to do the priority depression by
	 * calling thread_priority or set_pri instead of stomping on the
	 * sched_pri field.
	 */
	ASSERT(thread->runq == RUN_QUEUE_NULL);
#endif /* RT_SCHED_RQ */
	/*
	 * RT_SCHED: Always valid -- replace hardcoded priorities with
	 * constants defined in kern/sched.h.
	 */
	thread->priority = BASEPRI_LOWEST;
        thread->sched_pri = BASEPRI_LOWEST;
    }
    timeout(thread_depress_timeout, (caddr_t)thread, depress_time);

    thread_unlock(thread);
    (void) splx(s);

}	
    
/*
 *	thread_depress_timeout:
 *
 *	Timeout routine for priority depression.
 */
void
thread_depress_timeout(thread)
register thread_t thread;
{
    int s;

    s = splsched();
    thread_lock(thread);
    /*
     *	Make sure thread is depressed, then undepress it.
     */
    if (thread->depress_priority >= 0) {
	thread->priority = thread->depress_priority;
	thread->depress_priority = -1;
	compute_priority(thread, TRUE);
    }
    else {
	panic("thread_depress_timeout: thread not depressed!");
    }
    thread_unlock(thread);
    (void) splx(s);
}

/*
 *	thread_depress_abort:
 *
 *	Prematurely abort priority depression if there is one.
 */
kern_return_t
thread_depress_abort(thread)
register thread_t	thread;
{
    kern_return_t	ret = KERN_SUCCESS;
    int	s;

    if (thread == THREAD_NULL) {
	return(KERN_INVALID_ARGUMENT);
    }

    s = splsched();
    thread_lock(thread);
    
    /*
     *	Only restore priority if thread is depressed and we can
     *	grab the depress timeout off of the callout queue.
     */
    if (thread->depress_priority >= 0) {
#if	NCPUS > 1
	if (untimeout_try(thread_depress_timeout, thread)) {
#else
	untimeout(thread_depress_timeout, thread);
#endif
	    thread->priority = thread->depress_priority;
	    thread->depress_priority = -1;
	    compute_priority(thread, TRUE);
#if	NCPUS > 1
	}
	else {
	    ret = KERN_FAILURE;
	}
#endif
    }

    thread_unlock(thread);
    (void) splx(s);
    return(ret);
}

/* Many of these may be unnecessary */
#include <sys/unix_defs.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <ufs/fs.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <ufs/dir.h>
#include <sys/file.h>

#include <mach/kern_return.h>
#include <kern/task.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <sys/mman.h>


map_fd(fd, offset, va, findspace, size)
	int		fd;
	vm_offset_t	offset;
	vm_offset_t	*va;
	boolean_t	findspace;
	vm_size_t	size;
{
	kern_return_t	result;
	vm_offset_t	user_addr;
	vm_offset_t retval;
	struct args {
		caddr_t	addr;	/* caddr_t */
		long	len;	/* size_t */
		long	prot;	/* int */
		long	flags;	/* int */
		long	fd;	/* int */
		long	pos;	/* off_t */
	} args;
	register struct args *ap = &args;

	if (size == 0)
		return(KERN_INVALID_ARGUMENT);

	if (findspace) {
		user_addr = VM_MIN_ADDRESS;
		ap->flags = MAP_PRIVATE;
	}
	else {
		if (copyin(va, &user_addr, sizeof(vm_offset_t))) {
			result = KERN_INVALID_ADDRESS;
			goto out;
		}
		ap->flags = MAP_PRIVATE | MAP_FIXED;
	}

	ap->addr = (caddr_t) user_addr;
	ap->len = size;
	ap->prot = VM_PROT_ALL;
	ap->fd = fd;
	ap->pos = offset;

	if (smmap(u.u_procp, ap, &retval)) return KERN_FAILURE;
	else if (findspace && copyout(&retval, va, sizeof(vm_offset_t))) {
		(void) vm_deallocate(current_task()->map, user_addr, size);
		result = KERN_INVALID_ADDRESS;
		goto out;
	}
	result = KERN_SUCCESS;
out:
	return result;

}


#if	MACH_EMULATION
/*
 *	honest-to-god_unix_syscall: 
 *
 *	Calls the indicated syscall, passing the given args and returning
 *	result(s), error and eosys in rv.  htg_unix_syscall() is intended
 *	to be machine independent which is why it doesn't peel arguments off
 *	the user's stack, return results in registers, etc.  It is intended
 *	to be a general way of providing kernel syscall services in an
 *	emulation environment (for when it absolutely, positively has to 
 *	be a syscall).
 */
htg_unix_syscall( code, argv, rv )
	unsigned	code;		/* syscall # */
	caddr_t		* argv;		/* arg vector */
	syscall_val_t 	* rv;
{
	extern 		int nsysent;
	struct sysent 	* callp;
	int		syscalltrace = 0;
	int		error;
	long		args[8];
	long		retval[2];

	if (code >= nsysent)
		code = 63;
	error = 0;
	callp = &sysent[code];

	/* copy in the user args */
	if (callp->sy_narg) {
		if (error = copyin(argv, (caddr_t)args, 
				   (callp->sy_narg)*NBPW))
			goto bad;
	}

	cr_threadinit(current_thread());

	if (syscalltrace) {
		register int i;
		char *cp;

		cp = "(";
		for (i= 0; i < callp->sy_narg; i++) {
			printf("%s0x%lx", cp, args[i]);
			cp = ", ";
		}
		if (i)
			putchar(')', 0);
		putchar('\n', 0);
	}

	/* actually call the routine */
	error = (*(callp->sy_call))(u.u_procp, args, retval);

	if (syscalltrace)
		 printf( "rc=0x%x val=0x%lx\n", error, *retval );
bad:
	/*
	 *  return the error bit, the eosys indicator (so the
	 *  caller can restart the syscall) and the two potential
	 *  return values
	 * XXX -- with new system call interface, who looks at these
	 *	  fields?  error may be ERESTART -- that's the clue to 
	 *	  restart the syscall.  I don't know how this is supposed
	 *	  to work.
	 */
	if (error) *retval = error;

	if ( copyout( retval, &rv->rv_val1, sizeof(rv->rv_val1) ) 
	    || copyout( &retval[1], &rv->rv_val2, sizeof(rv->rv_val2) ) )
		/* 
		 * we're kind of stuck if we fault copying out the error.
		 * we don't want to return unix errors directly, because mach syscalls
		 * return kern_return_t's.  So, the syscall will fail with some
		 * bogus errno in this case.
		 */
		error = EFAULT;	

	return( error ? KERN_FAILURE : KERN_SUCCESS );

}
#endif	/* MACH_EMULATION */

#if	MACH_SCTIMES
kern_return_t
mach_sctimes_0()
{
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_1(arg0)
{
#ifdef	lint
	arg0++;
#endif
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_2(arg0, arg1)
{
#ifdef	lint
	arg0++; arg1++;
#endif
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_3(arg0, arg1, arg2)
{
#ifdef	lint
	arg0++; arg1++; arg2++;
#endif
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_4(arg0, arg1, arg2, arg3)
{
#ifdef	lint
	arg0++; arg1++; arg2++; arg3++;
#endif
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_5(arg0, arg1, arg2, arg3, arg4)
{
#ifdef	lint
	arg0++; arg1++; arg2++; arg3++; arg4++;
#endif
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_6(arg0, arg1, arg2, arg3, arg4, arg5)
{
#ifdef	lint
	arg0++; arg1++; arg2++; arg3++; arg4++; arg5++;
#endif
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_7()
{
	return KERN_SUCCESS;
}

kern_return_t
mach_sctimes_8(arg0, arg1, arg2, arg3, arg4, arg5)
{
#ifdef	lint
	arg0++; arg1++; arg2++; arg3++; arg4++; arg5++;
#endif
	return KERN_SUCCESS;
}

vm_offset_t mach_sctimes_buffer = 0;
vm_size_t mach_sctimes_bufsize = 0;

kern_return_t
mach_sctimes_9(size)
	vm_size_t size;
{
	register kern_return_t kr;

	if (mach_sctimes_bufsize != 0)
		kmem_free(kernel_map, mach_sctimes_buffer,
			  mach_sctimes_bufsize);

	if (size == 0) {
		mach_sctimes_bufsize = 0;
		kr = KERN_SUCCESS;
	} else {
		mach_sctimes_buffer = kmem_alloc(kernel_map, size);
		if (mach_sctimes_buffer == 0) {
			mach_sctimes_bufsize = 0;
			kr = KERN_FAILURE;
		} else {
			mach_sctimes_bufsize = size;
			kr = KERN_SUCCESS;
		}
	}

	return kr;
}

kern_return_t
mach_sctimes_10(addr, size)
	char *addr;
	vm_size_t size;
{
	register kern_return_t kr;

	if (size > mach_sctimes_bufsize)
		kr = KERN_FAILURE;
	else if (copyin(addr, mach_sctimes_buffer, size))
		kr = KERN_FAILURE;
	else
		kr = KERN_SUCCESS;

	return kr;
}

kern_return_t
mach_sctimes_11(addr, size)
	char *addr;
	vm_size_t size;
{
	register kern_return_t kr;

	if (size > mach_sctimes_bufsize)
		kr = KERN_FAILURE;
	else if (copyout(mach_sctimes_buffer, addr, size))
		kr = KERN_FAILURE;
	else
		kr = KERN_SUCCESS;

	return kr;
}

kern_return_t
mach_sctimes_port_alloc_dealloc(times)
	int times;
{
	task_t self = current_task();
	int i;

	for (i = 0; i < times; i++) {
		port_name_t port;

		(void) port_allocate(self, &port);
		(void) port_deallocate(self, port);
	}

	return KERN_SUCCESS;
}
#endif	/* MACH_SCTIMES */
