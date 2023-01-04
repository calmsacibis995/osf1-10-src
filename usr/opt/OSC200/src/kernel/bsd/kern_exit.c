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
static char	*rcsid = "@(#)$RCSfile: kern_exit.c,v $ $Revision: 4.4.20.18 $ (DEC) $Date: 1993/12/17 01:14:03 $";
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
 * kern_exit.c
 *
 * Modification History:
 *
 * 24-Apr-1992  Marian Macartney
 *	Fix procdup error leg
 *
 * 04-Feb-1992  Jeff Denham
 *	Add call to POSIX.4 timer routine to tear down timers on exit().
 *
 * 30-Sep-1991  Paula Long
 *      Merged in OSF-V1.0.1 changes (Bug Number 1850).
 *      "SVVS3.0 expects the ac_stat field of the acct record to be 0 
 *       regardless of the exit(n) status.  It seems it is expecting the upper
 *       8 bits (not WEXITSTATUS) to be in the ac_stat field."
 *
 * 21-Aug-1991  Philip Cameron
 *	Added code for waitpid() (Ultrix 4.2 binary compatability only)
 *
 * 3-Jun-1991   Diane Lebel
 *	Added support for > 64 file descriptors per process.
 *
 * 16-Apr-91    Paula Long
 *      Fixed problem with the RT_TIMER conditional
 *
 * 4-Apr-91     Paula Long
 *      Add P1003.4 required extensions.  
 *      Specifically <rt_timer.h> is now included and if RT_TIMER
 *      is defined all pending Realtime timers are canceled on process exit
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
#include "_lmf_.h"

#include <cputypes.h>
#include <sys/secdefines.h>
#include <machine/reg.h>
#ifdef	ibmrt
#include <ca/scr.h>
#endif
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
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/vm.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/syslog.h>
#include <mach/kern_return.h>
#include <kern/task.h>
#include <kern/kalloc.h>
#include <vm/vm_map.h>
#include <kern/thread.h>
#include <kern/parallel.h>
#include <kern/sched_prim.h>
#include <mach/time_value.h>
#include <sys/audit.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <rt_timer.h>

#if RT
#include <sys/time.h>
#endif
#include <streams.h>    /* for streams_sigexit() */

#include <sys/uswitch.h>

/*
 * subroutine to process newly-orphaned process group
 */

void
orphan_pg(pgp)
     struct pgrp *pgp;
{
	struct proc *q;
	int count = 0;

	/* find out if there are any stopped procs in the group */

	for (q = pgp->pg_mem; q != NULL && count == 0; q = q->p_pgrpnxt) {
		if (q->task != TASK_NULL)
			count += q->task->user_stop_count;
	}
	if (count > 0) { /* there are, send SIGHUP/SIGCONT to all procs in group  */
		for (q = pgp->pg_mem; q != NULL;  q = q->p_pgrpnxt) {
			psignal(q, SIGHUP);
			psignal(q, SIGCONT);
		}
	}
}


/*
 * Exit system call: pass back caller's arg
 */
rexit(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long     rval;
	} *uap = (struct args *) args;

	return (exit(p, W_EXITCODE(uap->rval, 0)));
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(p, rv)
	register struct proc *p;
	int rv;
{
	register int i;
	register struct proc *q, *nq;
	register int x;
	struct rusage *ru;
	time_value_t		sys_time, user_time;
	register struct	timeval	*utvp, *stvp;
	task_t			task = current_task();
	thread_t		thread;
	queue_head_t		*list;
	int			s;
	struct file		*f;
	int			signal_init = 0, init_signaled = 0;
        extern int              mlil;	/* see sys_sysinfo.c */
	extern char		swap_kill_message[];
	k_siginfo_t siginfo;

#ifdef	PGINPROF
	vmsizmon();
#endif

	/* New for /proc */
	/*
	 * If there is a valid vnode/procnode save the exit status for the
	 * process in the procfs inode for the folks who have it open.
	 * Then stop the process to make a post mortem easier.
	 */
	if(p->p_vnptr) {
		 register struct procnode *prcnd;

		prcnd = VNTOPN(p->p_vnptr);
		prcnd->prc_exitval = rv;
		p->task->procfs.pr_why = PR_DEAD;
		p->task->procfs.pr_what = rv;
		if(p->task->procfs.pr_qflags & PRFS_STOPTERM) {
			unix_master();
			p->task->procfs.pr_flags |=
				(PR_STOPPED | PR_ISTOP | PR_PCINVAL);
			wakeup(&p->task->procfs);
			task_suspend(p->task);
			unix_release();
		}
	}

	if(p->task->procfs.pr_exvp != NULLVP)
	{
		vrele(p->task->procfs.pr_exvp);
		p->task->procfs.pr_exvp = NULLVP;
	}

	bzero((struct procfs*)&(task->procfs), sizeof (struct procfs));
	p->p_pr_qflags = NULL;
	/* End of /proc code */

        /*
         * Inform Capacity Limitation.
         */
        if((p->p_flag & SLOGIN) != 0) {
            if (mlil > 0) mlil--;
            p->p_flag &= ~SLOGIN;
        } 

	/*
	 *	Since exit can be called from psig or syscall, we have
	 *	to worry about the potential race.  sig_lock_to_exit
	 *	causes any thread in this task encountering a sig_lock
	 *	anywhere (including here) to immediately suspend permanently.
	 */
	if (current_thread() != p->exit_thread) {
		sig_lock(p);
		sig_lock_to_exit(p);
	}
	/*
	 *	Halt all threads in the task, except for the current
	 *	thread.
	 */
	(void) task_halt(task);

	/*
	 *	Set SWEXIT just to humor ps.
	 */
#if	MACH
	p->p_flag &= ~STRC;
#else
	p->p_flag &= ~(STRC|SULOCK);
#endif
	p->p_flag |= SWEXIT;
	p->p_sigignore = ~(sigset_t)0;
	p->p_sig = (sigset_t)0;
	p->p_cpticks = (sigset_t)0;
	p->p_pctcpu = (sigset_t)0;
	for (i = 0; i < NSIG; i++)
		signal_disposition(i) = (void (*)()) SIG_IGN;
	untimeout(realitexpire, (caddr_t)p);
    
#if RT_TIMER
	exitrttimers(p);
#endif

#if BIN_COMPAT
	/* no more syscalls so release compatability module */
	if(u.u_compat_mod) {
		cm_terminate(u.u_compat_mod);
		u.u_compat_mod = 0;
	}
#endif /*BIN_COMPAT*/

	p->p_habitat = USW_OSF1;

#if _LMF_
        /*
         * If this process has a chain of exit actions (hanging from
         * the u_exitp of the user struct), process the chain.
         * (This feature is implemented in a general way, but is used
         * only by LMF (License Management Facility).)
         */
	if(u.u_exitp) {
	        lmf_exit_actn();
	}
#endif /*_LMF_*/

	vm_delete_wirings(task->map);

	/*
	 * If this process was killed due to lack of swap space, write
	 * appropriate message to user's controlling terminal if possible.
	 */
	if (p->p_flag & SSWPKIL)
		uprintf(swap_kill_message, p->p_pid);

	/*
	 * No other threads in this task are active; no need to lock.
	 */
	for (i = 0; i <= u.u_lastfile; i++) {
		f = U_OFILE(i, &u.u_file_state);
		U_POFILE_SET(i, 0, &u.u_file_state);
		if (f != NULL && f != U_FD_RESERVED) {
			U_OFILE_SET(i, NULL, &u.u_file_state);
			(void)closef(f);
		}
		if (f == U_FD_RESERVED)
			printf("exit: stumbled on reserved fd\n");
	}
	if (u.u_file_state.uf_ofile_of) {
		kfree(u.u_file_state.uf_ofile_of,
			u.u_file_state.uf_of_count * sizeof(struct file *));
		kfree(u.u_file_state.uf_pofile_of, u.u_file_state.uf_of_count);
		u.u_file_state.uf_ofile_of = NULL;
		u.u_file_state.uf_pofile_of = NULL;
	}
	if (SESS_LEADER(p)) {
              	int retval, error;
                int *retvalp = &retval;
                struct vnode *vp;
                struct session *session = p->p_session;

		SESS_LOCK(session);
		session->s_leader = (struct proc *)NULL;
		if (vp = session->s_ttyvp) {
			struct pgrp *fpgrp;
                        session->s_ttyvp = (struct vnode *)NULL;
                        if (session->s_fpgrpp) {
                                if (fpgrp = *session->s_fpgrpp)
                                        PG_REF(fpgrp);
                                session->s_fpgrpp = (struct pgrp **)NULL;
                        } else {
                                fpgrp = (struct pgrp *)NULL;
                        }
                        SESS_UNLOCK(session);


                        /*
                         * send SIGHUP to all processes in foreground pgrp.
                         */
                        p->p_flag &= ~SCTTY; /* excl. current proc from HUPS */
                        if (fpgrp) {
                                pgsignal(fpgrp, SIGHUP, 1);
				PG_UNREF(fpgrp);
                        }

                        /*
                         * clearalias closes the device associated with
                         * the controlling terminal.
                         */
                        clearalias(vp, 0);
			vrele(vp);
		} else
			SESS_UNLOCK(session);
	}
	vrele(u.u_cdir);
	if (u.u_rdir)
		vrele(u.u_rdir);
	u.u_rlimit[RLIMIT_FSIZE].rlim_cur = RLIM_INFINITY;
/*...rjg.. do it later..*/
#ifdef ORIG_CODE
	acct(rv);
#endif
#if SEC_ARCH
	if (security_is_on) sp_clear_tags();
#endif

	/*
	 * Clean up aio state, if aio ever occurred in this process.
	 */
	if (p->p_flag & SAIO)
		(void) aioexit(p);

	semexit();
#if     STREAMS
        streams_sigexit(p);
#endif
#ifdef i386
	clx_exit();		/* Xenix clean up */
#endif
	AUDIT_CALL ( SYS_exit, 0, (int *)0, rv, AUD_HDR|AUD_RES, (char *)0 );

	if (*p->p_prev = p->p_nxt)		/* off allproc queue */
		p->p_nxt->p_prev = p->p_prev;
	if (p->p_nxt = zombproc)		/* onto zombproc */
		p->p_nxt->p_prev = &p->p_nxt;
	p->p_prev = &zombproc;
	zombproc = p;
	p->p_stat = SZOMB;

	pidunhash(p);

	if (p->p_pid == 1) {
		printf("init exited with %d\n", WEXITSTATUS(rv));
		if (u.u_data_start == 0) {
			printf("Can't exec /etc/init\n");
			for (;;)
				;
		} else
			panic("init died");
	}

	p->p_xstat = rv;

        /*
         * Allocate an rusage to hold this process's resource
         * utilization statistics (for wait), and initialize to the contents
         * of the rusage in the U-area.  No locks needed for u_ru.
         */
        ru = (struct rusage *)kalloc(sizeof (struct rusage));
        p->p_ru = ru;
        *p->p_ru = u.u_ru;

	/*
	 *	The task_halt() above stopped all the threads except this
	 *	one in their tracks, so it's ok to get their times here.
	 */
	utvp = &ru->ru_utime;
	utvp->tv_sec = 0;
	utvp->tv_usec = 0;

	stvp = &ru->ru_stime;
	stvp->tv_sec = 0;
	stvp->tv_usec = 0;

	list = &task->thread_list;
	task_lock(task);
	thread = (thread_t) queue_first(list);
	s = splsched();
	while (!queue_end(list, (queue_entry_t) thread)) {
		
		thread_read_times(thread, &user_time, &sys_time);

		utvp->tv_sec += user_time.seconds;
		utvp->tv_usec += user_time.microseconds;
		stvp->tv_sec += sys_time.seconds;
		stvp->tv_usec += sys_time.microseconds;

		thread = (thread_t) queue_next(&thread->thread_list);
	}
	splx(s);

	/*
	 *  Add in time from terminated threads.
	 */
	utvp->tv_sec += task->total_user_time.seconds;
	utvp->tv_usec += task->total_user_time.microseconds;
	stvp->tv_sec += task->total_system_time.seconds;
	stvp->tv_usec += task->total_system_time.microseconds;
	task_unlock(task);

        /*
         * Get page fault statistics and memory utilization from task structure.
         * Copy back to uarea for acct() to report correct times
         */
	task_get_rusage(ru, task);

        u.u_ru = *p->p_ru;
        acct(rv);                       /* write out accouning record */

        ruadd(p->p_ru, &u.u_cru);       /* add in children's usage */

	if (p->p_cptr)		/* only need this if any child is S_ZOMB */
		wakeup((caddr_t)&proc[1]);
	if (PGRP_JOBC(p)) {
		p->p_pgrp->pg_jobc--;
		if (p->p_pgrp->pg_jobc == 0) /* process group is now orphaned */
			orphan_pg(p->p_pgrp);
	}
					
	for (q = p->p_cptr; q != NULL; q = nq) {
		if (PGRP_JOBC(q)) {
			q->p_pgrp->pg_jobc--;
			if (q->p_pgrp->pg_jobc == 0)  /* group is orphaned */
				orphan_pg(q->p_pgrp);
		}
		nq = q->p_osptr;
		if (nq != NULL)
			nq->p_ysptr = NULL;
		if (proc[1].p_cptr)
			proc[1].p_cptr->p_ysptr = q;
		q->p_osptr = proc[1].p_cptr;
		q->p_ysptr = NULL;
		proc[1].p_cptr = q;

		q->p_pptr = &proc[1];
		q->p_ppid = 1;
		if (q->p_stat == SZOMB && p->p_pptr != &proc[1]) {
			signal_init = init_signaled = 1;
			psignal(&proc[1], SIGCHLD);
		}

		/*
		 * Traced processes are killed
		 * since their existence means someone is screwing up.
		 */
		if (q->p_flag&STRC) {
			q->p_flag &= ~STRC;
			q->sigwait = FALSE;
			psignal(q, SIGKILL);
		}
		/*
		 * Protect this process from future
		 * tty signals, clear TSTP/TTIN/TTOU if pending.
		 */
		(void) spgrp(q);
	}
	p->p_cptr = NULL;

	/*
	 * Set p_wcode to the CLD_ code describing what
	 * caused process to exit.  Set it to a CLD_
	 * describing either a regular exit, a coredump 
	 * signal, or a regular signal.
	 */
	p->p_wcode = WIFEXITED(rv) ? CLD_EXITED :
			WCOREDUMP(rv) ? CLD_DUMPED : CLD_KILLED;
	siginfo.si_signo = SIGCHLD;
	siginfo.si_code = p->p_wcode;
	siginfo.si_pid = p->p_pid;
	siginfo.si_status = WAITID_STATUS(siginfo.si_code, rv);
	psignal_info(p->p_pptr, SIGCHLD, &siginfo);

	p->task = TASK_NULL;
	p->thread = THREAD_NULL;

	/*
	 * Now that process no longer exists except as a zombie,
	 * can safely remove all sigqueue structures.
	 * thread_deallocate() will handle deallocating
	 * thread sigqueue structures.  Just do the proc
	 * structure sigqueues here.
	 */
	sigq_remove_all(&p->p_sigqueue, ALL_SIGQ_SIGS);

	/*
	 * Handle the SNOCLDWAIT flag here.
	 * This controls whether the parent can wait successfully for
	 * the dead process. If set, the child is handed immediately
	 * to init. If this is the last child in the parent's list
	 * it is awakened to get its ECHILD error. A SIGCHLD is always sent,
	 * regardless of this flag setting.
	 */
	if (p->p_pptr->p_flag & SNOCLDWAIT) {
		/*
		 * Parent doesn't care about its children.
		 * Therefore, immediately give this
		 * exiting process to init so the parent's
		 * wait for it will fail with ECHILD
		 */
		struct proc *pp = p->p_pptr, *initp = &proc[1];

		/* Unlink this process from its siblings */
		if (p->p_osptr)
			p->p_osptr->p_ysptr = p->p_ysptr;
		if (p->p_ysptr)
			p->p_ysptr->p_osptr = p->p_osptr;
		
		/* Unlink this process from its parent */
		if (pp->p_cptr == p)
			pp->p_cptr = p->p_osptr;
		
		/* Link it to init */
		p->p_pptr = initp;
		p->p_ppid = 1;
		
		/* Link it to init's child list */
		p->p_osptr = initp->p_cptr;
		p->p_ysptr = NULL;
		if (initp->p_cptr)
			initp->p_cptr->p_ysptr = p;
		initp->p_cptr = p;
		signal_init = 1;
		if (pp->p_cptr == NULL)
			wakeup((caddr_t)pp);
	} else
		wakeup((caddr_t)p->p_pptr);

	if (signal_init) {
		if (!init_signaled)
			psignal(&proc[1], SIGCHLD);
		wakeup((caddr_t)&proc[1]);
	}
	(void) task_terminate(current_task());
	thread_halt_self();
	/*NOTREACHED*/
}


#ifdef	COMPAT_43
/*
 * The compatibility interface wait() is patched through to the
 * general purpose function waitf(). The options and rusage fields are
 * derived from register values that depend on the machine type.
 * N.B. 4.3 reno sets uap->status to 0 unconditionally for compat code;
 *	we're not, so keep an eye on this.
 */

#if	!defined(__alpha) && !defined(mips)
owait(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	pid;
		int	*status;
		long	options;
		struct	rusage_dev *rusage;
		long	compat;
	} *uap = (struct args *) args;

	uap->pid = WAIT_ANY;
	uap->compat = 1;
	uap->status = 0;
#ifdef	multimax
	/*
	 *	All wait calls use the same interface.
	 */
	uap->rusage = (struct rusage_dev *)fuword(u.u_ar0[R1] + 8);
	uap->options = fuword(u.u_ar0[R1] + 4);
#else	/* multimax */

#ifdef	ibmrt
	if ((u.u_ar0[ICSCS] & ICSCS_HOKEY) != ICSCS_HOKEY) {
		uap->options = 0;
		uap->rusage = (struct rusage_dev *)0;
	}
	else {
		uap->rusage = (struct rusage_dev *)u.u_ar0[R4];	 /* as per C linkage */
		uap->options = u.u_ar0[R3];     /* as per C linkage */
	}
#endif	
#ifdef	balance
	if ((u.u_ar0[MODPSR] & (PSR_C<<PSRADJ)) == 0) {
		uap->options = 0;
		uap->rusage = (struct rusage_dev *)0;
	}
	else {
		uap->rusage = (struct rusage_dev *)u.u_ar0[R2];
		uap->options = u.u_ar0[R1];
	}
#endif	
#ifdef	vax
	if ((u.u_ar0[PS] & PSL_ALLCC) != PSL_ALLCC) {
		uap->options = 0;
		uap->rusage = (struct rusage_dev *)0;
	}
	else {
		uap->rusage = (struct rusage_dev *)u.u_ar0[R1];
		uap->options = u.u_ar0[R0];
	}
#endif	
#ifdef	sun
	if ((u.u_ar0[PS] & PSL_ALLCC) != PSL_ALLCC) {
		uap->options = 0;
		uap->rusage = (struct rusage_dev *)0;
	}
	else {
		uap->rusage = (struct rusage_dev *)u.u_ar0[R1];
		uap->options = u.u_ar0[R0];
	}
#endif	
#ifdef	__hp_osf
	if ((u.u_ar0[PS] & PSL_ALLCC) != PSL_ALLCC) {
		uap->options = 0;
		uap->rusage = (struct rusage_dev *)0;
	}
	else {
		uap->rusage = (struct rusage_dev *)u.u_ar0[R1];
		uap->options = u.u_ar0[R0];
	}
#endif
#ifdef	i386
	if ((u.u_ar0[EFL] & EFL_ALLCC) != EFL_ALLCC) {
		uap->options = 0;
		uap->rusage = (struct rusage_dev *)0;
	}
	else {
		uap->rusage = (struct rusage_dev *)u.u_ar0[EDX];
		uap->options = u.u_ar0[ECX];
	}
#endif	
#endif	/* multimax */

	return (waitf(p, uap, retval));
}

#else	/* mips or alpha */
/*
 * Here, the "wait" call is really "wait3", with 3 arguments. It is
 * transformed into a call to waitf() by copying the arguments out of
 * uap.
 */

owait(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		union wait *status;
		long	options;
		struct	rusage_dev *rusage;
	} *uap = (struct args *) args;
	struct args1 {
		long	pid;
		int	*status;
		long	options;
		struct	rusage_dev *rusage;
		long	compat;
	} uap1;
	uap1.pid = WAIT_ANY;
	uap1.status = (int *) uap->status;
	uap1.options = uap->options;
	uap1.rusage = uap->rusage;
	uap1.compat = 1;

	/*
	 * wait() must set WEXITED and WTRAPPED in uap->options
	 * before calling waitf().
	 */
	uap1.options |= (WEXITED|WTRAPPED);

	return (waitf(p, &uap1, retval));
}

#endif	/* mips or alpha */

wait4(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	pid;
		int	*status;
		long	options;
		struct	rusage_dev *rusage;
		long	compat;
	} *uap = (struct args *) args;

	uap->compat = 0;

	/*
	 * wait4() must set WEXITED and WTRAPPED in uap->options
	 * before calling the version of waitf() that supports
	 * SVR4 wait behavior.
	 */
	uap->options |= (WEXITED|WTRAPPED);
	return (waitf(p, uap, retval));
}
#else	/* COMPAT_43 */

#define waitf	wait4

#endif	/* COMPAT_43 */

/*
 * Wait system call, actually the target of waitpid() and wait4(), as
 * well as being the function that actually does the work for the 4.3
 * compatibility interfaces.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 *
 * In order to address the "logged-in" terminal problem for window
 * managers, telnet servers, etc. which are not created initially by
 * init but which we want init to clean up after, we add the concept
 * of the "login device" associated with a process.  The login
 * program executes a special ioctl() call to distinguish the top
 * process in the tree and record the controlling device with which
 * it is associated.  The new wait() option, WLOGINDEV, (used by
 * init) indicates that the returned resource usage record should
 * include the device number of the controlling terminal for the
 * top level process (this will be NODEV for any other process).
 * This way, init need not create terminal listeners for all
 * potential login terminals but can still know enough to clean
 * things up after such processes (which it has inherited) do exit.
 */

waitf(q, args, retval)
	register struct proc *q;
	void *args;
	long retval[];
{
	register struct args {
		long	pid;		/* real type: 'pid_t' */
		int	*status;
		long	options;
		struct	rusage_dev *rusage;
#if	COMPAT_43
		long	compat;
#endif
	} *uap = (struct args *) args;
	register f;
	register struct proc *p;
	int status, error = 0;
	/*
	 * for waitid(), uap->status is a pointer to a siginfo
	 * structure
	 */
	siginfo_t siginfo;

	if (uap->pid == 0)
		uap->pid = -q->p_pgid;

#if	COMPAT_43
	if (!(uap->compat))
#endif
		if (uap->options &~ (WUNTRACED|WNOHANG|WLOGINDEV|WCONTINUED|
				     WNOWAIT|WTRAPPED|WEXITED|WSIGINFO))
			return (EINVAL);
loop:
	f = 0;
	for (p = q->p_cptr; p; p = p->p_osptr) {
		if (uap->pid != WAIT_ANY &&
		    p->p_pid != uap->pid && p->p_pgid != -uap->pid)
			continue;
#if SEC_BASE
		/*
		 * If we are init (pid 1) don't count unkillable processes.
		 * This is to avoid waiting forever for policy deamons
	   	 * to teminate as a result of a signal.
	    	 */
		if (!security_is_on 
			|| q->p_pid != 1 || ((p->p_flag & SSYS) == 0))
#endif
		f++;
		if (p->task == TASK_NULL) {
			if (!(uap->options & WEXITED)) {
				/* not looking for an exited task */
				continue;
			}

			if (p->p_wcode != CLD_EXITED
			    && p->p_wcode != CLD_KILLED
			    && p->p_wcode != CLD_DUMPED) {
				/* XXX This should never happen */
				/* XXX print a warning to the console */
				printf("WARNING: p_wcode value of exited child not set to CLD_EXITED or CLD_KILLED or CLD_DUMPED\n");
			}

			retval[0] = p->p_pid;
#if	COMPAT_43
			if (uap->compat)
				retval[1] = p->p_xstat;
#endif
			if (uap->options & WSIGINFO) {
				retval[0] = 0;
			}

			if (uap->status) {
				if (uap->options & WSIGINFO) {
					/*
					 * waitid system call expects
					 * a siginfo structure to be
					 * filled in.
					 */
					siginfo.si_signo = SIGCHLD;
					siginfo.si_code = p->p_wcode;
					siginfo.si_status =
						WAITID_STATUS(p->p_wcode,
							      p->p_xstat);
					siginfo.si_pid = p->p_pid;
					if (error = copyout((caddr_t)&siginfo,
							    (caddr_t)uap->status,
							    sizeof(siginfo)))
						return(error);
				} else {
					status = p->p_xstat;
					if (error = copyout((caddr_t)&status,
							    (caddr_t)uap->status,
							    sizeof(status)))
						return (error);
				}
			}
			if (uap->rusage && (error = copyout((caddr_t)p->p_ru,
			    (caddr_t)uap->rusage,
			    (uap->options&WLOGINDEV) ? 
			     sizeof(struct rusage_dev) : sizeof(struct rusage))))
				return (error);

			/* leave process in a waitable state */
			if (uap->options & WNOWAIT) {
				return(0);
                        }

			pgrm(p);			/* off pgrp */
			p->p_xstat = 0;
			p->p_wcode = 0;
			if (p->p_ru) {
				ruadd(&u.u_cru, p->p_ru);
				kfree(p->p_ru, sizeof (struct rusage));
				p->p_ru = 0;
			}
			p->p_ru = 0;
			p->p_stat = NULL;
			p->p_pid = 0;
			p->p_ppid = 0;
			if (*p->p_prev = p->p_nxt)	/* off zombproc */
				p->p_nxt->p_prev = p->p_prev;
			crfree(p->p_rcred);		/* kill off old creds */
			p->p_nxt = freeproc;		/* onto freeproc */
			freeproc = p;
			if (q = p->p_ysptr)
				q->p_osptr = p->p_osptr;
			if (q = p->p_osptr)
				q->p_ysptr = p->p_ysptr;
			if ((q = p->p_pptr)->p_cptr == p)
				q->p_cptr = p->p_osptr;
			p->p_pptr = 0;
			p->p_ysptr = 0;
			p->p_osptr = 0;
			p->p_cptr = 0;
			p->p_sig = (sigset_t)0;
			p->p_sigcatch = (sigset_t)0;
			p->p_sigignore = (sigset_t)0;
			p->p_sigmask = (sigset_t)0;
		        /*p->p_pgrp = 0;*/	/* pgrm() already did this */
			p->p_flag = 0;
			p->p_cursig = 0;
			return (0);
		}

		/*
		 * Zombie children (dead children) were already taken
		 * care of above.  Only get here if dealing with a 
		 * child which has been stopped or continued for some 
		 * reason.
		 */

		/*
		 * WTRAPPED is automatically set in wait() and waitpid().
		 * SWTED bit in p_flags is no longer used to indicate
		 * whether wait should report on this event.  Instead
		 * p_wcode is set when there is an event, and it is
		 * unset by waitf() when the event should not be reported
		 * on again (that is, when WNOWAIT is not set in the
		 * wait options field).
		 *
		 * If traced process received any signal,
		 * 	or if process received stop signal,
		 * 	or if process received a SIGCONT this test succeeds.
		 */
		if ((p->task->user_stop_count > 0 && p->p_stat == SSTOP &&
		     ((p->p_wcode == CLD_TRAPPED && uap->options & WTRAPPED
			&& p->p_flag & STRC) ||
		      (p->p_wcode == CLD_STOPPED && uap->options & WUNTRACED))
		    ) ||
		    (p->p_wcode == CLD_CONTINUED && uap->options & WCONTINUED) 
		  ) {
			if (uap->options & WSIGINFO)
				retval[0] = 0;
			else
				retval[0] = p->p_pid;

#if	COMPAT_43
			if (uap->compat) {
				retval[1] = W_STOPCODE(p->p_xstat);
				error = 0;
			}
#endif

			if (uap->status) {
				if (uap->options & WSIGINFO) {
					siginfo.si_signo = SIGCHLD;
					siginfo.si_code = p->p_wcode;
					siginfo.si_status = p->p_xstat;
					siginfo.si_pid = p->p_pid;
					error = copyout((caddr_t)&siginfo,
					  (caddr_t)uap->status, 
					  sizeof(siginfo));
				} else {
					if (p->p_wcode == CLD_CONTINUED) {
						status = _W_CONTINUED;
					} else {
						status = W_STOPCODE(p->p_xstat);
					}
					error = copyout((caddr_t)&status,
							(caddr_t)uap->status,
							sizeof(status));
				}
			} else
				error = 0;

			/*
			 * The SWTED bit in p_flags is no longer used 
			 * to indicate whether this event has already
			 * been waited for.  Instead, setting p_wcode to
			 * 0 will indicate not to wait for this event.
			 * It is now also possible to wait on the same 
			 * event more than once if the WNOWAIT option 
			 * is requested.
			 */
			if (!(uap->options & WNOWAIT))
				p->p_wcode = 0;
			return (error);
		}
        }
	if (f == 0)
		return (ECHILD);
	if (uap->options & WNOHANG) {
		if (uap->options & WSIGINFO) {
			siginfo.si_signo = 0;
			siginfo.si_code = 0;
			siginfo.si_status = 0;
			siginfo.si_pid = 0;
			error = copyout((caddr_t)&siginfo,
					(caddr_t)uap->status, sizeof(siginfo));
			retval[0] = 0;
			return (error);
		} else {
			retval[0] = 0;
			return (0);
		}
	}
	if (error = tsleep((caddr_t)q, PWAIT | PCATCH, "wait", 0)) {
		return (error);
	}
	goto loop;
}

kern_return_t	init_process()
/*
 *	Make the current process an "init" process, meaning
 *	that it doesn't have a parent, and that it won't be
 *	gunned down by kill(-1, 0).
 */
{
	register struct proc *p;

#if     SEC_BASE
	if (!privileged(SEC_SETPROCIDENT, EPERM)) {
#else
	if (suser(u.u_cred, &u.u_acflag) != 0) {
#endif
		AUDIT_CALL(SYS_setsid, EPERM, (char *)0, 0L,
			   AUD_HDR|AUD_RES, (char *)0);
		return(KERN_NO_ACCESS);
	}

	unix_master();
	p = u.u_procp;

	/*
	 *	Take us out of the sibling chain, and
	 *	out of our parent's child chain.
	 */

	if (p->p_osptr)
		p->p_osptr->p_ysptr = p->p_ysptr;
	if (p->p_ysptr)
		p->p_ysptr->p_osptr = p->p_osptr;
	if (p->p_pptr->p_cptr == p)
		p->p_pptr->p_cptr = p->p_osptr;
	p->p_pptr = p;
	p->p_ysptr = p->p_osptr = 0;
	pgmv (p, p->p_pid, 0);
	p->p_ppid = 0;

	unix_release();
	AUDIT_CALL(SYS_setsid, 0, (char *)0, p->p_pid,
		    AUD_HDR|AUD_RES, (char *)0);
	return(KERN_SUCCESS);
}
