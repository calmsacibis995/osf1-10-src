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
static char	*rcsid = "@(#)$RCSfile: kern_fork.c,v $ $Revision: 4.4.21.8 $ (DEC) $Date: 1993/12/07 21:47:02 $";
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
 * kern_fork.c
 *
 * Modification History:
 *
 * 04-Feb-92	Jeff Denham
 *	Now clear POSIX.4 timers by zeroing the tblock pointer in proc.
 *
 * 20-Sep-91	Jeff Denham
 *	Clear of POSIX.4 timers on a fork.
 *
 * 6-June-91  Brian Stevens
 *    Changed reference to MAXUPRC to u.u_maxuprc so that it will pick up its
 *    value at run-time (to allow binary configuration).
 *
 * 5-May-91	Ron Widyono
 *	Fix PROC_TIMER_LOCK bug in RT_TIMER.
 *
 *16-Apr-91     Paula Long
 *      Fixed for condtional for RT_TIMER
 *
 * 4-Apr-91     Paula Long
 *      Add P1003.4 required extensions.  
 *      Specifically <rt_timer.h> is now included and if RT_TIMER
 *      is defined all Realtime timers are cleared for the 
 *      forked process.
 *
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
 */
 
#include "bin_compat.h"
#include <sys/secdefines.h>
#include <cputypes.h>

#include "_lmf_.h"

#include <machine/reg.h>
#if	!defined(ibmrt) && !defined(mips)
#include <machine/psl.h>
#endif

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <sys/vm.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <kern/thread.h>
#if	UNIX_LOCKS
#include <sys/lock_types.h>
#endif
#if     SEC_BASE
#include <sys/security.h>
#endif
#include <rt_timer.h>
#if RT
#include <sys/time.h>
#endif

#include <rt_pml.h>

thread_t	newproc(), procdup();

/*
 * fork system call.
 */
fork(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return(fork1(p, args, retval, 0));
}

vfork(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return(fork1(p, args, retval, 1));
}

fork1(p1, args, retval, isvfork)
	struct proc *p1;
	void *args;
	long retval[];
	long isvfork;
{
	register struct proc *p2;
	register a;
	thread_t	th = THREAD_NULL;
	int		s, error, not_allowed;
	register uid_t	puid;

	a = 0;
	not_allowed = 0;

	PROC_LOCK(p1);
	puid = p1->p_rcred->cr_uid;
	PROC_UNLOCK(p1);
#if     SEC_BASE
        /*
         * Check if process is allowed to exceed per-user proc limit or to
         * take last proc slot.  If so, don't bother to scan the proc table.
         * The second argument of -1 prevents this call from counting as a
         * use of privilege for purposes of auditing.  Use of the privilege
         * will be recorded by the second call below if it is actually needed.
         */
	not_allowed = (check_privileges ? (!privileged(SEC_LIMIT, -1))
						: (puid != 0));
#else
	if (puid != 0)
		not_allowed = 1;
#endif
	if (not_allowed) {
		for (p2 = allproc; p2; p2 = p2->p_nxt) {
			PROC_LOCK(p2);
			if (p2->p_rcred->cr_uid == puid)
				a++;
			PROC_UNLOCK(p2);
		}
		for (p2 = zombproc; p2; p2 = p2->p_nxt) {
			PROC_LOCK(p2);
			if (p2->p_rcred->cr_uid == puid)
				a++;
			PROC_UNLOCK(p2);
		}
	}
	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot.
	 */
	p2 = freeproc;
	if (p2==NULL)
		tablefull("proc");
#if     SEC_PRIV
	if (p2 == NULL || 
	   ((check_privileges ? (!privileged(SEC_LIMIT, 0)) : not_allowed)
		&& (p2->p_nxt == NULL || a > u.u_maxuprc)))
#else
	if (p2 == NULL || (not_allowed && (p2->p_nxt == NULL || a > u.u_maxuprc)))
#endif
	{
		retval[1] = 0;
		return (EAGAIN);
	}
	th = newproc(isvfork);
	if (th == THREAD_NULL) {
		retval[1] = 0;
		return (EAGAIN);
	}
	thread_dup(current_thread(), th);
	s = splhigh();
	TIME_READ_LOCK();
	th->u_address.utask->uu_start = time;
	TIME_READ_UNLOCK();
	splx(s);
	th->u_address.utask->uu_acflag.fi_flag = AFORK;

#if BIN_COMPAT
	/* keeps compatability module refernce count correct */
	if(th->u_address.utask->uu_compat_mod) 
		cm_newproc(th->u_address.utask->uu_compat_mod);
#endif /*BIN_COMPAT*/

#if _LMF_
	/* Starts child process with an empty chain of end actions */
	th->u_address.utask->uu_exitp = NULL;
#endif /*_LMF_*/

#if     RT_PML

	th->u_address.utask->uu_lflags &= ~(UL_ALL_FUTURE|UL_STKLOCK|
					    UL_DATLOCK|UL_TXTLOCK|UL_PROLOCK);
#endif

	retval[0] = p2->p_pid;
	retval[1] = 0;
	ASSERT(th->u_address.uthread->uu_nd.ni_cred != NOCRED);
	(void) thread_resume(th);
	return (0);
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
thread_t
newproc(isvfork)
	long isvfork;
{
	register struct proc *rpp, *rip;
	register int n;
	register struct file *fp;
	static int pidchecked = 0;
	static struct proc *idleproc = (struct proc *) 0;
	thread_t	th;
	int lastfile;
	struct ufile_state *ufp = &u.u_file_state;
        int i,s;

	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
	mpid++;
retry:
	if (mpid >= 30000) {
		mpid = 100;
		pidchecked = 0;
	}
	if (mpid >= pidchecked) {
		int doingzomb = 0;
		int doingidle = 0;

		pidchecked = 30000;
		/*
		 * Scan the proc table to check whether this pid
		 * is in use.  Remember the lowest pid that's greater
		 * than mpid, so we can avoid checking for a while.
		 */
		rpp = allproc;
again:
		for (; rpp != NULL; rpp = rpp->p_nxt) {
			if (rpp->p_pid == mpid || rpp->p_pgrp->pg_id == mpid) {
				mpid++;
				if (mpid >= pidchecked)
					goto retry;
			}
			if (rpp->p_pid > mpid && pidchecked > rpp->p_pid)
				pidchecked = rpp->p_pid;
			if (rpp->p_pgrp->pg_id > mpid &&
			    pidchecked > rpp->p_pgrp->pg_id)
				pidchecked = rpp->p_pgrp->pg_id;
		}
		if (!doingzomb) {
			doingzomb = 1;
			rpp = zombproc;
			goto again;
		}
		if (!doingidle) {
			doingidle = 1;
			rpp = idleproc;
			goto again;
		}
		
	}
	if ((rpp = freeproc) == NULL)
		panic("no procs");

	freeproc = rpp->p_nxt;			/* off freeproc */

	if (rpp->p_nxt = idleproc)		/* onto idleproc */	
		rpp->p_nxt->p_prev = &rpp->p_nxt;
	rpp->p_prev = &idleproc;
	idleproc = rpp;

	/*
	 * Make a proc table entry for the new process.
	 */
	rip = u.u_procp;
	rpp->p_stat = SIDL;
	/*
	 * Do not need to hold PROC_TIMER_LOCK for the child because no
	 * other threads know about the child yet.
	 */
	timerclear(&rpp->p_realtimer.it_value);
#if RT_TIMER
         /* 
          * P1003.4/D11 specifies timers shouldn't be inherited across
          * a fork, so clear them by making sure the timer base ptr is NULL.
          */
	rpp->p_psx4_timer = (psx4_tblock_t *)0;
#endif
	rpp->p_flag = SLOAD | (rip->p_flag & (SPAGV|SXONLY|SCTTY));
	rpp->p_ndx = rpp - proc;
	PROC_LOCK(rip);
	rpp->p_ruid = rip->p_ruid;
	rpp->p_rgid = rip->p_rgid;
	rpp->p_svuid = rip->p_svuid;
	rpp->p_svgid = rip->p_svgid;
	rpp->p_auid = rip->p_auid;
	PROC_UNLOCK(rip);
	rpp->p_pgrpnxt = rip->p_pgrpnxt;
	rip->p_pgrpnxt = rpp;
	rpp->p_pgrp = rip->p_pgrp;
	rpp->p_nice = rip->p_nice;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_pptr = rip;
	rpp->p_osptr = rip->p_cptr;
	rpp->p_uac = rip->p_uac;
	if (rip->p_cptr)
		rip->p_cptr->p_ysptr = rpp;
	rpp->p_ysptr = NULL;
	rpp->p_cptr = NULL;
	rip->p_cptr = rpp;
	rpp->p_time = 0;
	rpp->p_cpu = 0;
	rpp->p_sigmask = rip->p_sigmask;
	rpp->p_sigcatch = rip->p_sigcatch;
	rpp->p_sigignore = rip->p_sigignore;
	rpp->p_siginfo = rip->p_siginfo;

	/* take along any pending signals like stops? */
	queue_init(&rpp->p_sigqueue);
	/*
	 * Inherit the habitat values from the parent. Reset on exec.
	 */
	rpp->p_habitat = rip->p_habitat;
	rpp->p_rssize = 0;
	rpp->p_maxrss = rip->p_maxrss;
	rpp->p_slptime = 0;
	rpp->p_pctcpu = 0;
	rpp->p_cpticks = 0;
	rpp->p_logdev = NODEV;
#ifdef	balance
	save_fpu();		/* inherit FPU context across fork() */
#endif
	n = PIDHASH(rpp->p_pid);
	rpp->p_idhash = pidhash[n];
	pidhash[n] = rpp - proc;

	/*
	 * Increase reference counts on shared objects.
	 *
	 * This code breaks when the parent does an exit or close
	 * asynchronously with the fork.  Currently (06/08/88) exit and fork
	 * both require unix_master, so there may be no problem with exit.
	 * close does not require unix_master, so one thread in the parent
	 * task could close a file while another thread is calling fork.
	 * We will increase the file structure's reference count here but
	 * lose the file descriptor entry because of the close.  The child
	 * will never see a valid file descriptor and so the file structure
	 * will remain forever with a bogus reference count.  This code
	 * should be executed by the child.  (But consider that similar
	 * problems apply to other u-area file state, like cdir and rdir.)
	 * XXX
	 */
	U_FDTABLE_LOCK(ufp);
	for (n = 0; n <= ufp->uf_lastfile; n++) {
		if ((fp = U_OFILE(n, ufp)) == NULL || fp == U_FD_RESERVED)
			continue;
		FP_REF(fp);
	}
	U_FDTABLE_UNLOCK(ufp);
	/*
	 * increment reference counts on shared objects
	 */
	if (u.u_cdir)
		VREF(u.u_cdir);
	if (u.u_rdir)
		VREF(u.u_rdir);
	PROC_LOCK(rip);
	crhold(rip->p_rcred);
	rpp->p_rcred = rip->p_rcred;
	PROC_UNLOCK(rip);

	/*
	 * This begins the section where we must prevent the parent
	 * from being swapped.
	 */
#if	!MACH
	rip->p_flag |= SKEEP;
#endif
	simple_lock_init(&rpp->siglock);
	rpp->sigwait = FALSE;
	rpp->exit_thread = THREAD_NULL;
	th = procdup(rpp, rip);	/* child, parent */
	if (th == THREAD_NULL) {
		/*
		 * couldn't make new task/thread, must undo what we've done
		 */
		PROC_LOCK(rip);
		crfree(rip->p_rcred);
		PROC_UNLOCK(rip);
		if (u.u_cdir)
			VUNREF(u.u_cdir);
		if (u.u_rdir)
			VUNREF(u.u_rdir);
		U_FDTABLE_LOCK(ufp);
		for (n = 0; n <= ufp->uf_lastfile; n++) {
			if ((fp = U_OFILE(n, ufp)) == NULL || fp == U_FD_RESERVED)
				continue;
			FP_UNREF(fp);
		}
		U_FDTABLE_UNLOCK(ufp);

		rip->p_cptr = rpp->p_osptr;
		if (rip->p_cptr)
			rip->p_cptr->p_ysptr = NULL;
		pgrm(rpp);
		pidunhash(rpp);

		/*
		 * clear out a few things
		 */
		rpp->p_pid = 0;
		rpp->p_ppid = 0;
		rpp->p_ruid = 0;
		rpp->p_rgid = 0;
		rpp->p_svuid = 0;
		rpp->p_svgid = 0;
		rpp->p_auid = AUID_INVAL;
		rpp->p_pptr = 0;
		rpp->p_ysptr = 0;
		rpp->p_osptr = 0;
		rpp->p_cptr = 0;
		rpp->p_flag = 0;
		rpp->p_sig = (sigset_t)0;
		rpp->p_sigcatch = (sigset_t)0;
		rpp->p_sigignore = (sigset_t)0;
		rpp->p_sigmask = (sigset_t)0;
		rpp->p_cursig = 0;
		rpp->p_pgrp = 0;
                rpp->p_stat = NULL;
		if (*rpp->p_prev = rpp->p_nxt)	/* off idleproc */
			rpp->p_nxt->p_prev = rpp->p_prev;
		rpp->p_nxt = freeproc;		/* onto freeproc */
		freeproc = rpp;
		(void) spl0();
#if	!MACH
		rip->p_flag &= ~SKEEP;
#endif
		return(th);
	}
#if     SEC_BASE
	secinfo_dup(rip, rpp);
#endif
	uarea_lock_init(th->u_address.utask);
	ASSERT(th->u_address.uthread->uu_nd.ni_cred != NOCRED);
	/* Streams signals init */
	queue_init(&rpp->p_strsigs);
	rpp->utask = th->u_address.utask;
	/* New for /proc */
	/* If inherit-on-fork flag is set in the task procfs struct, dup it */
	if (rip->task->procfs.pr_flags & PR_FORK)
	{
	    rpp->task->procfs = rip->task->procfs;
	    rpp->task->procfs.pr_qflags &= ~PRFS_OPEN;
	    rpp->p_pr_qflags = rip->p_pr_qflags;
	}

	else {
	/* if the inherit flag is not set, clear all but the uid, gid, and perm
	    fields and set run on last close */
		uid_t tmp_uid;
		gid_t tmp_gid;
		struct vnode *tmp_vp;
		register int tmp_prot;

		/* save the original object file's UID, GID and permissions */
		tmp_uid = rip->task->procfs.pr_uid;
		tmp_gid = rip->task->procfs.pr_gid;
		tmp_prot = rip->task->procfs.pr_protect;
		tmp_vp = rip->task->procfs.pr_exvp;

		/* Clear the structure */
		bzero(&rpp->task->procfs, sizeof(struct procfs) );

		/* restore the original object file's UID, GID and permissions*/
		rpp->task->procfs.pr_uid = tmp_uid;
		rpp->task->procfs.pr_gid = tmp_gid;
		rpp->task->procfs.pr_protect = tmp_prot;
		rpp->task->procfs.pr_exvp = tmp_vp;
		rpp->p_pr_qflags = NULL;
	}

	if(rpp->task->procfs.pr_exvp != NULLVP)
		VREF(rpp->task->procfs.pr_exvp);

	/* End of /proc code */

	if (*rpp->p_prev = rpp->p_nxt)		/* off idleproc */
		rpp->p_nxt->p_prev = rpp->p_prev;

	/*
	 *	It is now safe to link onto allproc
	 */
	rpp->p_nxt = allproc;			/* onto allproc */
	rpp->p_nxt->p_prev = &rpp->p_nxt;	/*   (allproc is never NULL) */
	rpp->p_prev = &allproc;
	allproc = rpp;
	rpp->p_stat = SRUN;			/* XXX */
	(void) spl0();

	/*
	 * Cause child to take a non-local goto as soon as it runs.
	 * On older systems this was done with SSWAP bit in proc
	 * table; on VAX we use u.u_pcb.pcb_sswap so don't need
	 * to do rpp->p_flag |= SSWAP.  Actually do nothing here.
	 */
	/* rpp->p_flag |= SSWAP; */

	/*
	 * Now can be swapped.
	 */
#if	!MACH
	rip->p_flag &= ~SKEEP;
#endif


	return(th);
}

/*
 * Initialize per-thread uarea information.
 * There are two types of clients, those of thread_create() and
 * those of fork (newproc).
 *
 * NOTE:  there is a seemingly redundant call to this function in
 * 	  newproc() which is necessary because procdup() will undo
 *	  any work done here.
 */
uarea_init(th)
	register thread_t	th;
{
	register struct uthread *uthread = th->u_address.uthread;

	queue_init(&uthread->uu_sigqueue);
	uthread->uu_curinfo = 0;

	uthread->uu_nd.ni_iov = &uthread->uu_nd.ni_iovec;
	/*
	 * The nameidata structure in the thread must point into the user
	 * task structure so that current and root directories
	 * can be shared among all threads in a task.
	 * If we want to change this, much file system code must be changed
	 * to know where these entities are stored.  Currently, it assumes
	 * that it's in the nameidata struct.
	 */
	uthread->uu_nd.ni_utnd = &th->u_address.utask->uu_utnd;
	/*
	 * Initialize per-thread credentials.  This for two types of
	 * clients -- kernel threads (via thread_create()) and fork().
	 */
	uthread->uu_nd.ni_cred = NOCRED;
	cr_threadinit(th);
}

uarea_lock_init(utaskp)
	struct utask *utaskp;
{
#if	UNIX_LOCKS && MACH_SLOCKS
	/*
	 * The accounting flag should simply use the u-area handy lock.
	 */
	utaskp->uu_acflag.fi_lock = &utaskp->uu_handy_lock;
#endif
	U_FDTABLE_LOCK_INIT(&utaskp->uu_file_state);
	U_HANDY_LOCK_INIT(utaskp);
	U_TIMER_LOCK_INIT(utaskp);
	UTND_LOCK_INIT(&utaskp->uu_utnd);
	/*
	 * Init proc timer lock here rather than hack vm/vmunix_.c:procdup().
	 */
	PROC_TIMER_LOCK_INIT(utaskp->uu_procp);
}

uarea_zero(th)
	thread_t	th;
{
	bzero((caddr_t) th->u_address.uthread, sizeof(struct uthread));
}

