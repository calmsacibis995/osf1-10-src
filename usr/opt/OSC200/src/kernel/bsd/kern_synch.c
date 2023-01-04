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
static char	*sccsid = "@(#)$RCSfile: kern_synch.c,v $ $Revision: 4.3.14.3 $ (DEC) $Date: 1993/09/07 21:43:53 $";
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *
 *	Revision History:
 *
 * 03-May-91	Peter H. Smith
 *	Don't let the autonice code mess with POSIX fixed priority processes.
 */

#include <cputypes.h>
#include <cpus.h>
#include <mach_km.h>
#include <mach_assert.h>
#include <rt_sched.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/vm.h>
#include <sys/kernel.h>
#include <sys/table.h>

#include <kern/ast.h>
#include <sys/callout.h>
#include <kern/queue.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <mach/machine.h>
#include <kern/parallel.h>
#include <kern/processor.h>

#if     MACH_KM
#include <kern/kern_mon.h>
#endif 

#include <machine/cpu.h>
#include <vm/pmap.h>
#include <vm/vm_kern.h>

#ifdef	vax
#include <vax/mtpr.h>	/* XXX */
#endif

#include <kern/task.h>
#include <mach/time_value.h>
#include <sys/sched_mon.h>

extern int autonice; 

/*
 * Recompute process priorities, once a second
 */
schedcpu()
{
	register thread_t th;
	uid_t uid;

	wakeup((caddr_t)&lbolt);

	if (autonice) {
        /* Autonice is now configurable. */	
	/*
	 *	Autonice code moved here from kern_clock.c
	 */
	  th = current_thread();
#if RT_SCHED
	  if ((th->policy == POLICY_TIMESHARE) && !(th->task->kernel_vm_space)) {
#else /* RT_SCHED */
	  if (!(th->task->kernel_vm_space)) {
#endif /* RT_SCHED */
	    register struct proc *p;

	    p = u.u_procp;
	    /*
	     * Important:  uid must be found through the per-thread
	     * credentials; using the proc structure's credentials
	     * would require taking the proc lock, not a good idea
	     * at interrupt level.
	     *
	     * Note sleazoid use of p_nice!		XXX
	     */
	    uid = u.u_uid;
	    if (uid && p->p_nice == PRIZERO) {
		time_value_t	user_time;

		timer_read(&(th->user_timer), &user_time);
		if (user_time.seconds > 10 * 60) {
		    RT_SCHED_HIST_SPL(RTS_autonice, th, th->policy, 
				      p->p_nice, 0, 0 );
		    p->p_nice = PRIZERO+4;
		    thread_priority(th, BASEPRI_USER + p->p_nice/2,
				    TRUE, TRUE);
		}
	    }
	  }
	}
	timeout(schedcpu, (caddr_t)0, hz);
}



/*
 * Internal MP sleep call.
 *
 * Suspends current thread until a wakeup is made on chan.
 * The process will then be made runnable.
 *
 * The thread sleeps at most timo/hz seconds (0 means no timeout).
 *
 * If pri includes PCATCH flag, signals are checked
 * before and after sleeping; otherwise, signals are not checked.
 *
 * Not setting PCATCH will cause the thread to sleep uninterruptibly.
 * This is different from 4.3 Reno, since pri is not used for scheduling
 * priority in OSF/1.  If PCATCH is not set, the return value will
 * always be zero.
 *
 * mpsleep returns 0 if awakened, and EWOULDBLOCK if the timeout expires.
 *
 * If PCATCH is set and a signal needs to be delivered,
 * ERESTART is returned if the current system call should be restarted
 * (if possible), and EINTR is returned if the system call should
 * be interrupted by the signal (return EINTR).
 * 
 * mpsleep also allows the caller to specify a lock to be unlocked before
 * it blocks.  This takes care of the MP problem with a normal sleep.
 * This lock can be simple, read, or write, as specified by the flags
 * parameter (MS_LOCK_SIMPLE, MS_LOCK_READ, MS_LOCK_WRITE).  The default
 * behavior is to re-lock the lock on success, and leave it unlocked on
 * errors.  If the MS_LOCK_ON_ERROR flag is set, then the lock is
 * re-locked on failure as well.
 *
 * This interface is not intended for general usage.  It is mostly for
 * use by sleep and tsleep, with some use by knowledgeable callers (e.g.
 * sosleep).
 */

mpsleep(chan, pri, wmesg, timo, lockp, flags)
	caddr_t	chan;
        long pri;
        char *wmesg;
        long timo;
	void *lockp;
	long flags;
{
	register struct proc *p = u.u_procp;
        register thread_t thread = current_thread();
	int catch = pri & PCATCH;
	int error = 0;
        
	if (chan)
		assert_wait((vm_offset_t)chan, (catch ? TRUE : FALSE));
	/*
	 * We may need to unlock a lock
	 */
	if (lockp) {
		/*
		 * Note:  the lock package macros should deal with this
		 *	  correctly if locking is not configured.
		 */
		if (flags & MS_LOCK_SIMPLE)
			simple_unlock((simple_lock_t) lockp);
		else {
			LASSERT(flags & (MS_LOCK_READ|MS_LOCK_WRITE));
			lock_done((lock_t) lockp);
		}
	}

	if (panicstr) {
		/*
		 * After a panic, just give interrupts a chance,
		 * then just return; don't run any other procs 
		 * or panic below, in case this is the idle process
		 * and already asleep.
		 * The splnet should be spl0 if the network was being used
		 * by the filesystem, but for now avoid network interrupts
		 * that might cause another panic.
		 */
		int s = splnet();
		/* Must clear any assert_wait, here or in caller */
		clear_wait(thread, THREAD_AWAKENED, FALSE);
		splx(s);
		goto out;
	}

	if (catch) {
		/*
		 * CHECK_SIGNALS does not need to be done on
		 * unix_master.  We only do the more expensive operation
		 * of issig after a quick check.
		 */
		if (CHECK_SIGNALS(p, thread, thread->u_address.uthread)) {
			unix_master();
			if (ISSIG(p)) {
				clear_wait(thread, THREAD_INTERRUPTED, TRUE);
				if (u.u_sigintr & sigmask(p->p_cursig))
					error = EINTR;
				else
					error = ERESTART;
				unix_release();
				goto out;
			}
			unix_release();
		}
	}
	if (timo)
		thread_set_timeout(timo);

        thread->wait_mesg = wmesg;

	thread_block();

        thread->wait_mesg = NULL;
	switch (thread->wait_result) {
		case THREAD_TIMED_OUT:
			error = EWOULDBLOCK;
			break;
		case THREAD_AWAKENED:
			/*
			 * Posix implies any signal should be delivered
			 * first, regardless of whether awakened due
			 * to receiving event.
			 */
			if (!catch)
				break;
			if (!CHECK_SIGNALS(p,thread,thread->u_address.uthread))
				break;
			/* else fall through */
		case THREAD_INTERRUPTED:
		case THREAD_SHOULD_TERMINATE:
			/*
			 * We're pretty sure we'll have to do issig, so
			 * don't bother with CHECK_SIGNALS, and just go
			 * for unix_master.
			 */
			if (catch) {
				unix_master();
				if (ISSIG(p)) {
					if (u.u_sigintr & sigmask(p->p_cursig))
						error = EINTR;
					else
						error = ERESTART;
				}
				unix_release();
			} 
			break;
	}
out:
	/*
	 * re-lock the lock if either no error, or MS_LOCK_ON_ERROR
	 * flag was set.
	 */
	if (lockp && (!error || (flags & MS_LOCK_ON_ERROR))) {
		if (flags & MS_LOCK_SIMPLE)
			simple_lock((simple_lock_t) lockp);
		else if (flags & MS_LOCK_READ) 
			lock_read((lock_t) lockp);
		else
			lock_write((lock_t) lockp);
	}
	return (error);
}

#if	MACH_ASSERT

/*
 * tsleep and sleep are macros (see param.h) if !MACH_ASSERT.
 */
/*
 * tsleep -- 
 * General purpose sleep call.
 * An interface to the mpsleep internal call.
 * Suspends current process until a wakeup is made on chan.
 * The process will then be made runnable.
 * Sleeps at most timo/hz seconds (0 means no timeout).
 * If pri includes PCATCH flag, signals are checked
 * before and after sleeping, else signals are not checked.
 * Returns 0 if awakened, EWOULDBLOCK if the timeout expires.
 * If PCATCH is set and a signal needs to be delivered,
 * ERESTART is returned if the current system call should be restarted
 * if possible, and EINTR is returned if the system call should
 * be interrupted by the signal (return EINTR).
 */

tsleep(chan, pri, wmesg, timo)
	caddr_t chan;
	long pri;
	char *wmesg;
	long timo;
{

	/*
	 * Catch callers who should be using sleep().
	ASSERT((pri & PCATCH) != 0 || (pri & ~PCATCH) > PZERO || timo);
	 */
	return (mpsleep(chan, pri, wmesg, timo, (void *) NULL, 0));
}

/*
 * sleep -- 
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process is rescheduled.
 * This function causes the process to sleep
 * uninterruptibly.
 * As a result, it must be called with pri <= PZERO.
 */

sleep(chan, pri)
	caddr_t chan;
	long pri;
{

	ASSERT((pri & PCATCH) == 0 && pri <= PZERO);
	(void) mpsleep(chan, pri, (char *)NULL, 0, (void *) NULL, 0);
}
#endif	/* MACH_ASSERT */


/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
	register caddr_t chan;
{
	int s;

	s = splhigh();
	thread_wakeup((vm_offset_t)chan);
	splx(s);
}

/*
 * Wake up the first process sleeping on chan.
 *
 * Be very sure that the first process is really
 * the right one to wakeup.
 */
wakeup_one(chan)
	register caddr_t chan;
{
	int s;

	s = splhigh();
	thread_wakeup_one((vm_offset_t)chan);
	splx(s);
}

/*
 * Initialize the (doubly-linked) run queues
 * to be empty.
 */
rqinit()
{
	register int i;

	for (i = 0; i < NQS; i++)
		qs[i].ph_link = qs[i].ph_rlink = (struct proc *)&qs[i];
	simple_lock_init(&callout_lock);
}

slave_start()
{
	register struct thread	*th;
	register int		mycpu;

	/*	Find a thread to execute */

	mycpu = cpu_number();

	splhigh();
	th = choose_thread(current_processor());
	if (th == NULL) {
		printf("Slave %d failed to find any threads.\n", mycpu);
		printf("Should have at least found idle thread.\n");
		halt_cpu();
	}

	/*
	 *	Show that this cpu is using the kernel pmap
	 */
	PMAP_ACTIVATE(kernel_pmap, th, mycpu);

	active_threads[mycpu] = th;

	if (th->task->kernel_vm_space == FALSE) {
		PMAP_ACTIVATE(vm_map_pmap(th->task->map), th, mycpu);
	}

	/*
	 *	Clock interrupt requires that this cpu have an active
	 *	thread, hence it can't be done before this.
	 */
	startrtclock();
	ast_context(th, mycpu);

#if     MACH_KM		
  if (th->monitor_obj != MONITOR_NULL) 
	monitor_new_thread(th, mycpu);	
#endif 

	load_context(th);
	/*NOTREACHED*/
}
