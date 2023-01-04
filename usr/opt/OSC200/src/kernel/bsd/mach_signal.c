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
static char	*sccsid = "@(#)$RCSfile: mach_signal.c,v $ $Revision: 4.3.6.4 $ (DEC) $Date: 1993/12/17 01:14:51 $";
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
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <kern/sched.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <sys/siginfo.h>

/*
 * Send the specified exception signal to the specified thread.
 *
 * NOTE: unlike its full-blown counterpart, this is completely parallel!
 */
thread_psignal(sig_thread, sig, siginfo_p)
	register thread_t	sig_thread;
	register long sig;
	register k_siginfo_t *siginfo_p;
{
	register struct proc *p;
	register task_t		sig_task;
	sigset_t mask;
	register sigqueue_t sigqueue;
	int s;
	
	if ((unsigned long)sig > NSIG)
		return;

	mask = sigmask(sig);
	if ((mask & threadmask) == 0) {
		printf("signal = %d\n");
		panic("thread_psignal: signal is not an exception!");
	}

	sig_task = sig_thread->task;
	p = &proc[sig_task->proc_index];

	/*
	 *	Forget ignored signals UNLESS process is being traced. (XXX)
	 */
	if ((p->p_sigignore & mask) && (p->p_flag & STRC) == 0 )
		return;

	/*
	 * if no siginfo is queued up for this signal type yet, and
	 * there is something to queue, then allocate a sigqueue
	 * structure and queue it up to uthread structure.
	 * It is sufficient to check the signal posted bit array
	 * to see whether a sigqueue structure should be queued.
	 * It is possible that the previously posted instance of
	 * this signal did not have any siginfo information to
	 * queue, and that no sigqueue for this signal type has
	 * been queued, but in this case it is appropriate that
	 * no siginfo information be delivered for this signal.
	 */
	 sigqueue = NULL;
	 if (siginfo_p && !(sig_thread->u_address.uthread->uu_sig & mask)) {
		/*
		 * Allocate a sigqueue structure. This routine can
		 * always sleep.
		 */
		sigqueue = SIGQ_ALLOC();

		/*
		 * If didn't allocate sigqueue - just don't queue up
		 * any siginfo information.
		 */
		if (sigqueue)
			sigqueue->siginfo = *siginfo_p;
	}

	/*
	 *	This is an exception signal - deliver directly to thread.
	 */
	sig_lock_simple(p);
	/*
	 * Attach sigqueue to thread structure uu_sigqueue queue.
	 * Make sure no one else has posted this signal type to this 
	 * thread in the meantime.  We have a lock, so it is ok to
	 * manipulate the sigqueue queue.
	 */
	if (sigqueue && !(sig_thread->u_address.uthread->uu_sig & mask)) {
		sigq_enqueue_tail(&sig_thread->u_address.uthread->uu_sigqueue,
				  sigqueue);
		sigqueue = NULL;
	}
	sig_thread->u_address.uthread->uu_sig |= mask;
	sig_unlock(p);
	SIGQ_FREE(sigqueue);
}
