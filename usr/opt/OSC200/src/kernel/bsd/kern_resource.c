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
static char *rcsid = "@(#)$RCSfile: kern_resource.c,v $ $Revision: 4.4.17.7 $ (DEC) $Date: 1993/09/22 18:28:13 $";
#endif 
/*
 * (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
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
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	Revision History:
 *
 * 04-Feb-92	Amy Kessler/Paula Long/Jeff Denham
 *    - Add new routines for get and setpriority that have extended
 * 	argument lists, containing the parameter structure version
 * 	number and the library-level thread-port value.
 *
 * 01-Oct-91	Peter H. Smith
 *    - Fix obvious bug in scheduler monitor get history code.
 *
 * 25-Jun-91	Peter H. Smith
 *    -	Incorporate OSF/1.0.1 fixes.  In donice(), change SEC_BASE security so
 *	that it is sufficient to have (cur_uid == chg_uid) in order to change
 *	priority.  Also, if SEC_ARCH is defined, return EACCES if caller does
 *	not have kill permission for the target process.
 *    - Make similar changes to the security for posix_setprio(), which was
 *	copied from donice().  The security for posix_setscheduler() does not
 *	need to be changed, because it is already more restrictive.
 *
 * 21-May-91	Peter H. Smith
 *    - Have posix_setscheduler return the old scheduling policy.  Draft 10
 *	requires that the old policy be returned.
 *
 * 03-May-91	Peter H. Smith
 *    - Change donice so that it silently ignores fixed priority threads.  This
 *	prevents users of the BSD setpriority() call from touching a fixed
 *	priority thread by accident.  Realtime applications must use the POSIX
 *	interfaces to affect fixed priority thread priorities.
 *    - Extend getpriority and setpriority to support POSIX-style priorities
 *	through new flags defined in mach/policy.h.  This is only used by the
 *	library routines.  Also piggyback the POSIX setscheduler functionality
 *	on these routines, to avoid adding extra syscalls.
 *    - Add scheduler monitor support (compiled out for production builds).
 *
 */

#include <rt_pml.h>
#include <rt_sched.h>

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <ufs/fs.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/vm.h>
#include <vm/vm_tune.h>

#include <kern/thread.h>
#include <mach/time_value.h>
#include <mach/mach_interface.h>
#include <mach/vm_param.h>
#include <vm/vm_umap.h>
#if	SEC_BASE
#include <sys/security.h>
#endif	

#include <sys/sched_mon.h>

#if RT_SCHED

#include <kern/port_object.h>	/* needed for port-to-pointer translate */
#include <kern/kern_port.h>	/* needed for port_unlock() macro */
#include <kern/queue.h>

/*
 * This occurrance of RT_SCHED is here since rr_interval is referred to
 * in  posix_setscheduler.
 * Pick up the global round-robin interval, for passing to thread_policy.
 * This is in units of milliseconds.  The interval is defined in
 * kernel/kern/sched_prim.c.
 */
extern int rt_sched_rr_interval;
extern thread_t convert_port_to_thread();
#endif /* RT_SCHED */

#if RT_SCHED_MON
/* Forward declarations */
static int schedmon_get_counts();
static int schedmon_get_history();
static int schedmon_get_ctrl();
static int schedmon_set_ctrl();
static int schedmon_clr_ctrl();
#endif /* RT_SCHED_MON */

#if RT_SCHED_MON
/* RT Scheduling monitoring structures. */
int rt_sched_hist_counts[RT_SCHED_HIST_EVTS + 1];
struct rt_sched_hist rt_sched_hist;
decl_simple_lock_data(,rt_sched_hist_lock);
#endif /* RT_SCHED_MON */

extern void task_get_rusage(struct rusage *, task_t);

/*
 * Resource controls and accounting.
 */

getpriority(curp, args, retval)
	struct proc *curp;
	void *args;
	long *retval;
{
	register struct args {
		long	which;
		long	who;
	} *uap = (struct args *) args;
	register struct proc *p;
	int low = PRIO_MAX + 1;
	register struct pgrp *pg;
	register uid_t tuid;

	ASSERT(syscall_on_master());
	switch (uap->which) {

	case PRIO_PROCESS:
		if (uap->who == 0)
			p = curp;
		else
			p = pfind(uap->who);
		if (p == 0)
			break;
		low = p->p_nice;
		break;

	case PRIO_PGRP:
		if (uap->who == 0)
			pg = curp->p_pgrp;
		else if ((pg = pgfind(uap->who)) == NULL)
			break;
		for (p = pg->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			if (p->p_nice < low)
				low = p->p_nice;
		}
		break;

	case PRIO_USER:
		if (uap->who == 0)
			uap->who = u.u_uid;
		for (p = allproc; p != NULL; p = p->p_nxt) {
			BM(PROC_LOCK(p));
			tuid = p->p_rcred->cr_uid;
			BM(PROC_UNLOCK(p));
			if (tuid == uap->who && p->p_nice < low)
				low = p->p_nice;
		}
		break;
	default:
		return (EINVAL);
	}
	if (low == PRIO_MAX + 1)
		return (ESRCH);

	*retval = low;
	return (0);
}

setpriority(curp, args, retval)
	struct proc *curp;
	void *args;
	long *retval;
{
	register struct args {
		long	which;
		long	who;
		long	prio;
	} *uap = (struct args *) args;
	register struct proc *p;
	int  found = 0, error = 0;
	register struct pgrp *pg;
	register uid_t tuid;

	ASSERT(syscall_on_master());


	switch (uap->which) {

	case PRIO_PROCESS:
		if (uap->who == 0)
			p = curp;
		else
			p = pfind(uap->who);
		if (p == 0)
			break;
		error = donice(curp, p, uap->prio);
		found++;
		break;

	case PRIO_PGRP:
		if (uap->who == 0)
			pg = curp->p_pgrp;
		else if ((pg = pgfind(uap->who)) == NULL)
			break;
		for (p = pg->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			error = donice(curp, p, uap->prio);
			found++;
		}
		break;

	case PRIO_USER:
		if (uap->who == 0)
			uap->who = u.u_uid;
		for (p = allproc; p != NULL; p = p->p_nxt) {
			BM(PROC_LOCK(p));
			tuid = p->p_rcred->cr_uid;
			BM(PROC_UNLOCK(p));
			if (tuid == uap->who) {
				error = donice(curp, p, uap->prio);
				found++;
			}
		}
		break;

	default:
		return (EINVAL);
	}
	if (found == 0)
		return (ESRCH);
	return (error);
}

donice(curp, chgp, n)
	register struct proc *curp, *chgp;
	register long n;
{
	register thread_t th;
	int	pri;
	register uid_t cur_uid;
	register uid_t chg_uid;
	uid_t cur_ruid;
#ifdef	ibmrt
	short	chg_nice;
#else
	char	chg_nice;
#endif

	BM(PROC_LOCK(curp));
	cur_uid = curp->p_rcred->cr_uid;
	cur_ruid = curp->p_ruid;
	BM(PROC_UNLOCK(curp));
	BM(PROC_LOCK(chgp));
	chg_uid = chgp->p_rcred->cr_uid;
	chg_nice = chgp->p_nice;
	BM(PROC_UNLOCK(chgp));

#if	SEC_BASE
	if (cur_uid != chg_uid && cur_ruid != chg_uid &&
		(check_privileges ? (!privileged(SEC_OWNER,0))
				  : (cur_uid && cur_ruid)))

#else	
	if (cur_uid && cur_ruid && 
	    cur_uid != chg_uid && cur_ruid != chg_uid)
#endif
		return (EPERM);
#if SEC_ARCH
	if (!sec_can_kill(chgp))
		return (EACCES);
#endif
	if (n > PRIO_MAX)
		n = PRIO_MAX;
	if (n < PRIO_MIN)
		n = PRIO_MIN;
	th = chgp->thread;
#if RT_SCHED
	/* Ignore request to change priority of realtime thread */
	if (th->policy & (POLICY_RR|POLICY_FIFO)) {
	  RT_SCHED_HIST_SPL( RTS_donice, th, th->policy, n, 0, 0 );
	  return (0);
	}
	RT_SCHED_HIST_SPL( RTS_donice, th, th->policy, n, 1, 0 );
#endif /* RT_SCHED */
	pri = BASEPRI_USER + (n - PRIZERO);

        /* need to put calculated priority into the valid range for
         * user threads.  For non-realtime this is 0 to 31 and for
         * realtime it is 32 to 63.
         */
	if (pri < BASEPRI_HIGHEST) {
		pri = BASEPRI_HIGHEST;
	} else if (pri > BASEPRI_LOWEST) {
		pri = BASEPRI_LOWEST;
	}

	if ((n < chg_nice) &&
#if	SEC_BASE
            !privileged(SEC_LIMIT,0))
#else
	    suser(u.u_cred, &u.u_acflag))
#endif
		return (EACCES);
	else if (pri < th->max_priority)
		/*
		 *	Unix superuser gets to raise max priority.
		 */
		thread_max_priority(th, th->processor_set, pri, TRUE);
	PROC_LOCK(chgp);
	chgp->p_nice = n;
	PROC_UNLOCK(chgp);
	thread_priority(th, pri, TRUE, TRUE);
	return (0);
}

setrlimit(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	which;
		struct	rlimit *lim;
	} *uap = (struct args *) args;
	struct rlimit alim;
	register struct rlimit *alimp;
	int error;
	extern int open_max_hard;
	
	extern struct rlimit vm_initial_limit_stack;
	extern struct rlimit vm_initial_limit_data;
	extern struct rlimit vm_initial_limit_rss;

	ASSERT(syscall_on_master());
	if (uap->which >= RLIM_NLIMITS)
		return (EINVAL);
	alimp = &u.u_rlimit[uap->which];
	if (error = 
	    copyin((caddr_t)uap->lim, (caddr_t)&alim, sizeof (struct rlimit)))
		return (error);
	if (alim.rlim_cur > alimp->rlim_max || alim.rlim_max > alimp->rlim_max)

#if	SEC_BASE
		if (!privileged(SEC_LIMIT, EPERM))
			return (EPERM);
#else
		if (error = suser(u.u_cred, &u.u_acflag))
			return (error);
#endif	

	switch (uap->which) {

	case RLIMIT_NOFILE:
		if ((alim.rlim_cur != RLIM_INFINITY &&
			(alim.rlim_cur > open_max_hard || 
		    alim.rlim_cur > OPEN_MAX_SYSTEM)) ||
		    (alim.rlim_max != RLIM_INFINITY &&
		    (alim.rlim_max > open_max_hard || 
		    alim.rlim_max > OPEN_MAX_SYSTEM)))
				return(EINVAL);
		if (alim.rlim_cur == RLIM_INFINITY)
				alim.rlim_cur = open_max_hard;
		if (alim.rlim_max == RLIM_INFINITY)
				alim.rlim_max = open_max_hard;
		break;
	
	case RLIMIT_DATA:
		if (alim.rlim_cur > vm_initial_limit_data.rlim_max)
			alim.rlim_cur = vm_initial_limit_data.rlim_max;
		break;

	case RLIMIT_STACK:
		if (alim.rlim_cur > vm_initial_limit_stack.rlim_max)
			alim.rlim_cur = vm_initial_limit_stack.rlim_max;
		break;
	
	case RLIMIT_RSS:
		if (alim.rlim_cur > vm_initial_limit_rss.rlim_max)
			alim.rlim_cur = vm_initial_limit_rss.rlim_max;
		p->p_maxrss = alim.rlim_cur/NBPG;
		break;

	case RLIMIT_AS:
		if (alim.rlim_cur > vm_tune_value(maxvas))
			if (alim.rlim_cur == RLIM_INFINITY)
				alim.rlim_cur = vm_tune_value(maxvas);
			else
				return(EINVAL);
		if (alim.rlim_max > vm_tune_value(maxvas))
			if (alim.rlim_max == RLIM_INFINITY)
				alim.rlim_max = vm_tune_value(maxvas);
			else
				return(EINVAL);
		break;
	}

	*alimp = alim;
	return (error);
}

getrlimit(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	which;
		struct	rlimit *rlp;
	} *uap = (struct args *) args;
	ASSERT(syscall_on_master());
	if (uap->which >= RLIM_NLIMITS)
		return (EINVAL);
	return (copyout((caddr_t)&u.u_rlimit[uap->which], (caddr_t)uap->rlp,
	    sizeof (struct rlimit)));
}

getrusage(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	who;
		struct	rusage *rusage;
	} *uap = (struct args *) args;
	struct rusage		ru_ret;
	time_value_t		sys_time, user_time,
				proc_sys_time, proc_user_time;
	register struct timeval	*tvp;
        task_t                  task = current_task();
        thread_t                thread;
        queue_head_t            *list;
        int                     s;


	switch (uap->who) {

	case RUSAGE_SELF:

 	       	list = &task->thread_list;

		task_lock(task);
        	s = splsched();
 		
 	       /*
         	*   Get time from terminated threads
         	*/

		proc_user_time = task->total_user_time;
		proc_sys_time = task->total_system_time;

		/*
		 *  Add time from active threads
		 */

  	     	thread = (thread_t) queue_first(list);
        	while (!queue_end(list, (queue_entry_t) thread)) {

			thread_read_times(thread, &user_time, &sys_time);

			time_value_add(&proc_user_time, &user_time);
			time_value_add(&proc_sys_time, &sys_time);

                	thread = (thread_t) queue_next(&thread->thread_list);
        	}

        	splx(s);
        	task_unlock(task);

		U_HANDY_LOCK();
		ru_ret = u.u_ru;
		U_HANDY_UNLOCK();

		tvp = &ru_ret.ru_utime;
        	tvp->tv_sec = proc_user_time.seconds;
        	tvp->tv_usec = proc_user_time.microseconds;
		tvp = &ru_ret.ru_stime;
        	tvp->tv_sec = proc_sys_time.seconds;
        	tvp->tv_usec = proc_sys_time.microseconds;

		/*
		 * Get pagefault statistics from task. Fills in 
		 * ru_minflt, ru_majflt, ru_maxrss, ru_idrss.
		 */
		task_get_rusage(&ru_ret, current_thread()->task);
		break;

	case RUSAGE_CHILDREN:
		U_HANDY_LOCK();
		ru_ret = u.u_cru;
		U_HANDY_UNLOCK();
		break;

	default:
		return (EINVAL);
	}
	return (copyout((caddr_t)&ru_ret, (caddr_t)uap->rusage,
	    sizeof (struct rusage)));
}

ruadd(ru, ru2)
	register struct rusage *ru, *ru2;
{
	register long *ip, *ip2;
	register int i;

	timevaladd(&ru->ru_utime, &ru2->ru_utime);
	timevaladd(&ru->ru_stime, &ru2->ru_stime);
	if (ru->ru_maxrss < ru2->ru_maxrss)
		ru->ru_maxrss = ru2->ru_maxrss;
	ip = &ru->ru_first; ip2 = &ru2->ru_first;
	for (i = &ru->ru_last - &ru->ru_first; i > 0; i--)
		*ip++ += *ip2++;
}

/*
 * Get pagefault statistics from task. Fills in 
	 * ru_minflt, ru_majflt, ru_maxrss, ru_idrss. Does not modify
 * any other fields.
 */

int ru_debug = 0;

void
task_get_rusage(struct rusage *ru_retp, task_t task)
{
	register struct rusage * ru = &u.u_ru;
	events_info_data_t	events;
	unsigned int		events_info_count;
	int			shift = PAGE_SHIFT - 10;  /* 2^10 == 1KB */
	thread_t       		thread;

	thread = current_thread();

	events_info_count = TASK_EVENTS_INFO_COUNT;
	task_info(task, TASK_ALL_EVENTS_INFO, (task_info_t)&events,
		&events_info_count);
	ru_retp->ru_minflt = events.faults - events.pageins;
	ru_retp->ru_majflt = events.pageins;

	ru_retp->ru_nswap = task->swap_nswap;

	if(ru_debug)
	printf("maxrss=0x%lx, ixrss=0x%lx, idrss=0x%lx, isrss=0x%lx\n",
		ru->ru_maxrss, ru->ru_ixrss, ru->ru_idrss, ru->ru_isrss);

	/* max resident set size in kilobytes */
	ru_retp->ru_maxrss = ru->ru_maxrss << shift;

	/* Integral resident set sizes measured in KB/seconds */
	ru_retp->ru_ixrss = (ru->ru_ixrss <<shift)/hz; 	/* text */
	ru_retp->ru_idrss = (ru->ru_idrss <<shift)/hz; 	/* data */
	ru_retp->ru_isrss = (ru->ru_isrss <<shift)/hz;  /* stack */

}

#if RT_SCHED
/* Additional P1003.4 getpriority routine. 
 * The following system calls can take additional arguments,
 * the sched_param version number ,a pointer to a thread structure,
 * and a null parameter whose use is yet to be decided.
 * Callers can now access an extended sched param structure
 * which may hold quantum information.  They also can now specify
 * a thread other than the main thread of a process.
 */

rt_getprio(curp, args, retval)
	struct proc *curp;
	void *args;
	long *retval;
{
	register struct args {
		long	which;			/* real type: 'int' */
		long	process_id;		/* real type: 'int' */
		long	data;			/* real type: 'int' */
		long	sched_param_version;	/* real type: 'int' */
		port_t	thread_port;
	} *uap = (struct args *) args;
	register struct proc *p;
	/*
	 * INT_MAX is above the range of legal priorities.
	 * The value assigned to low here is tested near the
	 * the end of the procedure.
	*/
	register thread_t th, curth = current_thread();
	int low = INT_MAX;
	register struct pgrp *pg;
	register uid_t tuid;
        port_t port;
	register int which = uap->which, process_id = uap->process_id;

	ASSERT(syscall_on_master());
      
	/* 
	 * Extension -- split the which parameter into a byte indicating
	 * which (0xFF), and byte indicating policy requests (0xFF00).
	 */

	switch (which & PRIO_WHICH) {
	/*
	 * Extension:  Add code to handle requests for PRIO_POSIX, and also
	 * PRIO_POSIX|PRIO_POLICY.  This last is a hack to avoid adding
	 * another syscall, and might be inappropriate.
	 */
	case PRIO_POSIX:
		if (process_id == 0)
			p = curp;
		else
			p = pfind(process_id);
		if (p == 0) 
			break;

	/*
	 * Determine which thread to work on. If the thread port argument
	 * contains 0, then use the main/first thread in the process.
	 * Otherwise, attempt to translate the port ID into a thread pointer,
	 * and bail out if the translation fails. If the target thread isn't
	 * the current thread, take a reference on the thread before unlocking
	 * its port. This will keep the thread around for the duration of the
	 * system call without requiring draconian locking measures.
	 *
	 * Note: a thread port argument is considered valid only if it is
	 * a port in the same process as the caller -- no fiddling with or
	 * peeking at threads in other tasks. Note also that this scheme
	 * depends on a thread port in the library never being 0. This is
	 * probably a pretty safe assumption.
 	 */
		if (uap->thread_port == 0) {
			task_lock(p->task);
			th = (thread_t)
			    queue_first((queue_head_t *)&p->task->thread_list);
			if (th == THREAD_NULL) {
				task_unlock(p->task);
				break;
			}
			if (th != curth)
				thread_reference(th);
			task_unlock(p->task);
		} else {
			if (curp != p) break;
			if (!port_translate(p->task, uap->thread_port, &port)) 
				break;
			if (port_object_type(port) == PORT_OBJECT_THREAD) {
				th = (thread_t) port_object_get(port);
				if (th != curth)
					thread_reference(th);
				port_unlock(port);
			} else {
				port_unlock(port);
				break;
			}
		}

		/* 
		 * If PRIO_POLICY or a particular policy is specified,
		 * the caller wants policy information. Otherwise
		 * he wants POSIX priority information.
		 */
		if (which & PRIO_POLICY) {
			low = th->policy;
		} else {
			if (th->depress_priority < 0)
				low = th->priority;
			else
				low = th->depress_priority;
			low = (NRQS - 1) - low;
			RT_SCHED_HIST_SPL( RTS_getprio, th, th->policy, low, 1, 0);
		}
		if (th != curth)
			thread_deallocate(th);
    		break;
	default:
		/*
		 * Need to return -1 if an error occurred, to match the POSIX
		 * interface definition.  BSD users clear and check errno, so
		 * this won't break them.
		 */
		*retval = -1;
		return (EINVAL);
	} /* end of case */
	/* (Always valid -- see explanation above) */
	if (low == INT_MAX) {
		*retval = -1;
		return (ESRCH);
	}

	*retval = low;
	return (0);

}  /* end of rt_getprio */

/* Additional P1003.4 setpriority routine. 
 * The use of a sched_param_version number allows callers 
 * to set values in an extended sched_param structure. 
 * The call can specify a thread whose priority or
 * policy they would like to change. The extra data parameter
 * could be used to supply quantum information in the future. 
 * 
 */

rt_setprio(curp, args, retval)
	struct proc *curp;
	void *args;
	long *retval;
{
	register struct args {
		long	which;			/* real type: 'int' */
		long	process_id;		/* real type: 'int' */
		long	prio;			/* real type: 'int' */
		long	data;			/* real type: 'int' */
		long	sched_param_version;	/* real type: 'int' */
		port_t  thread_port;
	} *uap = (struct args *) args;
	register struct proc *p;
	register struct pgrp	*pg;
	register uid_t tuid;
	register thread_t th, curth = current_thread();
        port_t port;
	int found = 0, error = 0, is_thread = 0;
	register int which = uap->which, prio = uap->prio,
			process_id = uap->process_id;

	ASSERT(syscall_on_master());

	/* 
	 * Extension -- split the which parameter into a byte indicating
	 * which (0xFF), and byte indicating policy requests (0xFF00).
	 */
	switch (which&PRIO_WHICH) {
	case PRIO_POSIX:
		if (process_id == 0)
			p = curp;
		else
			p = pfind(process_id);
		if (p == 0)
			break;

	/*
	 * Determine which thread to work on. If the thread port argument
	 * contains 0, then use the main/first thread in the process.
	 * Otherwise, attempt to translate the port ID into a thread pointer,
	 * and bail out if the translation fails. If the target thread isn't
	 * the current thread, take a reference on the thread before unlocking
	 * its port. This will keep the thread around for the duration of the
	 * system call without requiring draconian locking measures.
	 *
	 * Note: a thread port argument is considered valid only if it is
	 * a port in the same process as the caller -- no fiddling with or
	 * peeking at threads in other tasks. Note also that this scheme
	 * depends on a thread port in the library never being 0. This is
	 * probably a pretty safe assumption.
 	 */
		if (uap->thread_port == 0) {
			task_lock(p->task);
			th = (thread_t)
			    queue_first((queue_head_t *)&p->task->thread_list);
			if (th == THREAD_NULL) {
				task_unlock(p->task);
				break;
			}
			if (th != curth)
				thread_reference(th);
			task_unlock(p->task);
			found++;
		} else {
			if (curp != p) break; /* thread op in same task only */
			if (!port_translate(p->task, uap->thread_port, &port))
				break;
			if (port_object_type(port) == PORT_OBJECT_THREAD) {
				th = (thread_t) port_object_get(port);
				if (th != curth)
					thread_reference(th);
				port_unlock(port);
			} else {
				port_unlock(port);
				break;
			}
			is_thread++;
			found++;
		}

	/* 
         * If PRIO_POLICY is true, the caller wants to set policy as well
	 * as priority information. Otherwise just set priority.
 	 */
 		if (which & PRIO_POLICY)
			error = posix_setscheduler(curp, p, prio, retval,
			   (which & PRIO_POLICY) >> PRIO_POLICY_SHIFT,
						   th, is_thread);
		else
			error = posix_setprio(curp, p, prio, retval,
					      th, is_thread, TRUE);

		/* 
		 * If the running thread is being modified and 
		 * it has lowered its priority below runq.low
		 * then set up to yield the processor by setting
		 * first_quantum and was_first_quantum to false.
		 * The first will cause the switch, the second will
		 * cause the thread to go to the end of the run queue.
		 */

		if (!error && (th == curth)) {
			register int s = splsched();
			register processor_t myprocessor = current_processor();
			if (myprocessor->processor_set->runq.low <=
				th->sched_pri) {
				myprocessor->first_quantum = FALSE;
				myprocessor->was_first_quantum = FALSE;
			}
			splx(s);
		}
		if (th != curth)
			thread_deallocate(th);
		break;

	default:
		return (EINVAL);
	}	/* end of case */
	if (found == 0)
		return (ESRCH);

	return (error);

} /* end of rt_setprio*/

/*
 * POSIX_SETPRIO()/POSIX_SETSCHEDULER(): worker routines for rt_setprio()
 * These do the actual work of checking permissions and setting
 * poilcy and priority for a thread. The is_thread argument, if true,
 * means that this is an intraprocess thread operation, which has
 * different permission requirements than a process-level operation.
 * The comments below explain the differences.
 */

posix_setprio(curp, p, pri, retval, th, is_thread, may_reschedule)
  register struct proc *curp;
  register struct proc *p;
  long pri;
  long *retval;
  register thread_t th;
  int is_thread;
  int may_reschedule;
{
	register uid_t cur_uid, chg_uid;
	uid_t cur_ruid;
	register int n, mach_pri, old_pri, sts;
#ifdef	ibmrt
	short	chg_nice;
#else
	char	chg_nice;
#endif

	ASSERT(syscall_on_master());
	
	/* Check permissions. */
	BM(PROC_LOCK(curp));
	cur_uid = curp->p_rcred->cr_uid;
	cur_ruid = curp->p_ruid;
	BM(PROC_UNLOCK(curp));
	BM(PROC_LOCK(p));
	chg_uid = p->p_rcred->cr_uid;
	chg_nice = p->p_nice;
	BM(PROC_UNLOCK(p));

	/*
	 * FIX ME?
	 * As an optimization, consider combining this set of
	 * privilege checks with those below. Have fun!
	 */
#if	SEC_BASE
	if (cur_uid != chg_uid && cur_ruid != chg_uid &&
	    (check_privileges ? (!privileged(SEC_OWNER,0))
	     : (cur_uid && cur_ruid))) {
#else	
	if (cur_uid && cur_ruid &&
	    cur_uid != chg_uid && 
	    cur_ruid != chg_uid) {
#endif
		*retval = -1;
		return (EPERM);
	}
#if SEC_ARCH
	if (!sec_can_kill(chgp))
		return (EPERM);
#endif
		
	/* Check the priority against the allowed range */
	if (rt_sched_invalid_posix_pri(th->policy,pri)) {
		*retval = -1;
		return (EINVAL);
	}
	
	/*
	 * Convert the priority to its internal format, and calculate the
	 * corresponding "nice" value.
	 */
	mach_pri = (NRQS - 1) - pri;
	n = 2 * ( mach_pri - BASEPRI_USER );

#if SEC_BASE
	if (!privileged(SEC_LIMIT,0)) {
#else
	/*
	 * If this an appropriately privileged user, don't make any other
	 * checks. There should be no limits in the privileged case.
	 */
	if (suser(u.u_cred, &u.u_acflag)) {
#endif
		if (is_thread) {
		/*
		 * This is thread-only operation. Allow this without root
		 * privilege if the policy is TS or RR and priority
		 * doesn't exceed POSIX 19, or for FF if it doesn't exceed
		 * POSIX 18. Otherwise, return EPERM.
		 */
			register int policy = th->policy;
			if (((policy == POLICY_TIMESHARE
			      || policy == POLICY_RR)
			     && mach_pri < BASEPRI_USER) ||
			    (policy == POLICY_FIFO && mach_pri <= BASEPRI_USER)){
				*retval = -1;
				return EPERM;
			}
		} else {
			/* 
			 * If priority is being raised, see if it is legal.
			 * This is more complicated for time sharing tasks.
			 */
			if (((mach_pri < th->max_priority)
			     || ((th->policy == POLICY_TIMESHARE)
				 && (n < chg_nice) && (chg_nice - n > 1)))) {
				*retval = -1;
				return (EPERM);
                       }
		}
	}

	/*
	 * Get the old priority.  Note that this is not atomic with the
	 * setting of the priority if preemption is possible!
	 */
	old_pri = (NRQS - 1) - th->priority;
	
	/*
	 * Change the priority.  Also set the nice value and, if necessary,
	 * change the maximum priority.
	 */
	PROC_LOCK(p);
	p->p_nice = n;
	PROC_UNLOCK(p);

	if (mach_pri < th->max_priority) {
		sts = thread_max_priority(th, th->processor_set,
					  mach_pri, may_reschedule);
		if (sts != KERN_SUCCESS) {
			switch (sts) {
			case KERN_INVALID_ARGUMENT:
				sts = EINVAL;
				break;
			case KERN_FAILURE:
				/* Thread in transit accross processor set. */
				sts = EAGAIN;
				break;
			default:
				/*
				 * An unexpected error code was returned.
				 */
				panic("setpriority/thread_max_priority");
				break;
			}
			*retval = -1;
			return (sts);
		}
	}

	sts = thread_priority(th, mach_pri, FALSE, TRUE);
	if (sts != KERN_SUCCESS) {
		switch (sts) {
		case KERN_INVALID_ARGUMENT:
			sts = EINVAL;
			break;
		case KERN_FAILURE:
			sts = EPERM;
			break;
		default:
			/*
			 * An unexpected error code was returned.
			 */
			panic("setpriority/thread_priority");
			break;
		}
		*retval = -1;
		return (sts);
    }
    
	RT_SCHED_HIST_SPL(RTS_setprio, th, th->policy, 
			  (n << 16) | (pri & 0xFFFF), mach_pri, old_pri);
	*retval = 0;
	return (0);
}

posix_setscheduler(curp, p, pri, retval, policy, th, is_thread)
	register struct proc *curp;
	register struct proc *p;
	register long pri;
	long *retval;
	register long policy;
	register thread_t th;
	int is_thread;
{
	register uid_t cur_uid, chg_uid;
	uid_t cur_ruid;
	register int n, old_policy, sts, no_priv = 1;
	register long mach_pri;

	ASSERT(syscall_on_master());
	
	/* Check the scheduling policy (algorithm) */
	if (invalid_policy(policy)) {
		*retval = -1;
		return (EINVAL);
	}

	/*
	 * Check the priority against the allowed range for the 
	 * requested policy 
	 */
	if (rt_sched_invalid_posix_pri(policy,pri)) {
		*retval = -1;
		return (EINVAL);
	}

	/*
	 * Convert the priority to its internal format, and calculate the
	 * corresponding "nice" value.
	 */
	mach_pri = (NRQS - 1) - pri;
	n = 2 * ( mach_pri - BASEPRI_USER );

	/*
	 * If this an appropriately privileged user, don't make any other
	 * checks. There should be no limits in the privileged case.
	 */
#if SEC_BASE
	if (!privileged(SEC_LIMIT,0)) {
#else
	if (suser(u.u_cred, &u.u_acflag)) {
#endif
		if (is_thread) {
			/*
			 * This is thread-only operation. Allow this without
			 * root privilege if the policy is TS or RR and
			 * priority doesn't exceed POSIX 19, or for FF if
			 * it doesn't exceed POSIX 18. Otherwise, return EPERM.
			 */
			if (((policy == POLICY_TIMESHARE || policy == POLICY_RR)
			     && mach_pri < BASEPRI_USER) ||
			    (policy == POLICY_FIFO && mach_pri <= BASEPRI_USER)){
				*retval = -1;
				return EPERM;
			}
		} else {
			*retval = -1;
			return EPERM;
		}
	} else
		no_priv = 0;

	/*
	 * Save the old policy.  This will not be atomic if we unfunnel the
	 * the procedure.  The policy gets returned to the caller.
         */
	old_policy = th->policy << PRIO_POLICY_SHIFT;

	/*
	 * Change the policy and priority.  Also set the nice value and, 
	 * if necessary, change the maximum priority.
	 */
	PROC_LOCK(p);
	p->p_nice = n;
	PROC_UNLOCK(p);
	sts = thread_policy(th, policy, rt_sched_rr_interval);
	if (sts != KERN_SUCCESS) {
		switch (sts) {
		case KERN_INVALID_ARGUMENT:
			sts = EINVAL;
			break;
		case KERN_FAILURE:
			sts = EPERM;
			break;
		}
		*retval = -1;
		return (sts);
	}

	sts = KERN_SUCCESS;
	if (is_thread && no_priv)
		/*
		 * In the thread-specific case, slam the max_priority
		 * down to the approved max for the nonprived case.
		 * Closes holes for crossovers from the nonthread case.
		 * Do this only if the caller is not privileged.
		 */
		sts = thread_max_priority(th, th->processor_set,
					  th->policy == POLICY_FIFO ?
					  BASEPRI_USER+1 : BASEPRI_USER,
					  TRUE);
	else
		/*
		 * In the nonthreaded or prived case, push up the thread
		 * max priority if it's being exceeded. Otherwise,
		 * thread_priority() will fail.
		 */
		if (mach_pri < th->max_priority)
			sts = thread_max_priority(th, th->processor_set,
						  mach_pri, TRUE);

	if (sts != KERN_SUCCESS) {
		switch (sts) {
		case KERN_INVALID_ARGUMENT:
			sts = EINVAL;
			break;
		case KERN_FAILURE:
			/* Thread in transit accross processor set. */
			sts = EAGAIN;
			break;
		}
		*retval = -1;
		return (sts);
	}

	sts = thread_priority(th, mach_pri, FALSE, TRUE);
	if (sts != KERN_SUCCESS) {
		switch (sts) {
		case KERN_INVALID_ARGUMENT:
			sts = EINVAL;
			break;
		case KERN_FAILURE:
			sts = EPERM;
			break;
		}
		*retval = -1;
		return (sts);
	}

	RT_SCHED_HIST_SPL(RTS_setscheduler, th, th->policy,
			  (policy << 16) | (n & 0xFFFF), pri, mach_pri);
	*retval = old_policy;
	return (0);
}

#else /* RT_SCHED false, make stub entry points.*/

rt_getprio(curp, args, retval)
	struct proc *curp;
	void *args;
	long *retval;
{
	return (ENOSYS);
}

rt_setprio(curp, args, retval)
	struct proc *curp;
	void *args;
	long *retval;
{
	return (ENOSYS);
}	
#endif	/* RT_SCHED */



/* 
 * Interface for a scheduling event logger.
 */

#if RT_SCHED_MON
schedmon(curp, args, retval)
     struct proc *curp;
     void *args;
     long *retval;
{
  register struct args {
    long		func;
    long		len;
    caddr_t	buf;
    caddr_t	start;
    caddr_t	lost;
  } *uap = (struct args *) args;
  register int sts;

  switch ((int)uap->func) {
  case SCHEDMON_GET_COUNTS:
    sts = schedmon_get_counts(curp, retval, uap->len, uap->buf);
    break;
  case SCHEDMON_GET_HISTORY:
    sts = schedmon_get_history(curp, retval, uap->len, uap->buf, 
			       uap->start, uap->lost);
    break;
  case SCHEDMON_GET_CTRL:
    sts = schedmon_get_ctrl(curp, retval, uap->len, uap->buf, uap->start);
    break;
  case SCHEDMON_SET_CTRL:
    sts = schedmon_set_ctrl(curp, retval, uap->len, uap->buf, uap->start);
    break;
  case SCHEDMON_CLR_CTRL:
    sts = schedmon_clr_ctrl(curp, retval, uap->len, uap->buf, uap->start);
    break;
  default:
    *retval = -1;
    sts = EINVAL;
  }

  return sts;
}

static int schedmon_get_history(curp, retval, len, buf, ctxt, lost)
     struct proc *curp;
     long *retval;
     register int	len;	/* Length (in events) of user's buffer. */
     register caddr_t	buf;	/* Pointer to start of user's buffer. */
     register caddr_t	ctxt;	/* Pointer to user's context longword. */
     register caddr_t	lost;	/* Pointer to user's lost counter. */
{
  register int collected;	/* Events collected since last call. */
  register int events;		/* Events to report this call. */
  register int start;   	/* Starting index of new events. */
  register int size;		/* Size in bytes of segment to copy. */
  register int s;		/* Current spl level. */
  int count;			/* Updated count value (where to read next). */
  int events_lost;		/* Number of events lost. */

  /* Copy in the context value. */
  if(copyin(ctxt, (caddr_t)&count, sizeof(count))) {
    *retval = -1;
    return EFAULT;
  }

  /* Check the user's buffer length */
  if (len <= 0) {
    *retval = -1;
    return EINVAL;
  }

  /* 
   * Lock the event buffer to prevent further events.  Have to raise
   * spl since the lock is taken at interrupt level, too.
   */
  s = splsched();
  rt_sched_hist_lock();
  
  /* 
   * Get the number of events collected since the last sample.  Don't count the
   * current event, since duplicates may be able to overwrite it.  Also can't
   * treat the current event cell as if it still holds the oldest event in the
   * buffer, since may be merging newest data into it.
   * Set lost to reflect the number of events which were collected and 
   * which are no longer in the history buffer.
   */
  collected = rt_sched_hist.count - count - 1;
  if (collected >= RT_SCHED_HIST_LEN) {
    events_lost = collected - RT_SCHED_HIST_LEN + 1;
    collected = RT_SCHED_HIST_LEN - 1;
  }
  else {
    events_lost = 0;
  }

  /* Indicate how many events were lost. */
  if (copyout((caddr_t)&events_lost, lost, sizeof(events_lost))) {
    rt_sched_hist_unlock();
    splx(s);
    *retval = -2;
    return EFAULT;
  }

  /* 
   * Calculate number of events to report this time, and update the count
   * value.
   */
  events = (collected < len) ? collected : len;
  count += events_lost + events;
  if (copyout((caddr_t)&count, ctxt, sizeof(count))) {
    rt_sched_hist_unlock();
    splx(s);
    *retval = -3;
    return EFAULT;
  }
  
  /* 
   * If there are any events to report, collect them into the user's buffer.  
   * May have to wrap around to find all the entries.  When wrapping around,
   * it is convenient to let the variable start hold the value of the real
   * start - RT_SCHED_HIST_LEN.  That way, -start is the number of events
   * toward the end of the buffer, and start + collected is the number of
   * events toward the start of the buffer.  Note that we only want to copy
   * the earlier events, so we start at last-collected and only copy up to
   * last-collected+events.
   *
   * Don't collect the current event, since it may be able to be overwritten.
   *
   */
  if (events > 0) {
    start = rt_sched_hist.last - collected + 1;
    if (start < 0) {
      if (-start < events) {
	size = sizeof(struct rt_sched_hist_entry) * (-start);
	if (copyout((caddr_t)&rt_sched_hist.buf[start + RT_SCHED_HIST_LEN], 
		    buf, size)) {
	  rt_sched_hist_unlock();
	  splx(s);
	  *retval = -4;
	  return EFAULT;
	}
	buf += size;
	start = 0;
	events = start + events;
      }
      else {
	start += RT_SCHED_HIST_LEN;
      }
    }
    size = sizeof(struct rt_sched_hist_entry) * events;
    if (copyout((caddr_t)&rt_sched_hist.buf[start], buf, size)) {
      rt_sched_hist_unlock();
      splx(s);
      *retval = -5;
      return EFAULT;
    }
  }

  rt_sched_hist_unlock();
  splx(s);
  *retval = events;
  return 0;
}

static int schedmon_get_counts(curp, retval, len, buf)
     register struct proc 	*curp;
     register long 		*retval;
     register int 		len;
     register caddr_t		buf;
{
  register int s;

  if (len <= 0) {
    *retval = sizeof(rt_sched_hist_counts) / sizeof(rt_sched_hist_counts[0]);
    return 0;
  }
  else {
    len = len * sizeof(rt_sched_hist_counts[0]);
    if (sizeof(rt_sched_hist_counts) < len) {
      len = sizeof(rt_sched_hist_counts);
    }
  }

  /* 
   * Lock the event buffer to prevent further events.  Have to raise
   * spl since the lock is taken at interrupt level, too.
   */
  s = splsched();
  rt_sched_hist_lock();

  /* Copy the counters into the user buffer. */
  if (copyout((caddr_t)&rt_sched_hist_counts, buf, len)) {
    rt_sched_hist_unlock();
    splx(s);
    *retval = -1;
    return EFAULT;
  }
  
  rt_sched_hist_unlock();
  splx(s);
  *retval = len / sizeof(rt_sched_hist_counts[0]);
  return 0;
}

static int schedmon_get_ctrl(curp, retval, len, buf, start)
     register struct proc	*curp;
     register long 		*retval;
     register long		len;
     register caddr_t		buf;
     register long		start;
{
  register int s;

  /* Return the range of control values if no control info was requested */
  if (len == 0) {
    *retval = RT_SCHED_HIST_EVTS + 1;
    return 0;
  }
  else {
    if ((start < 0) || (start > RT_SCHED_HIST_EVTS + 1)) {
      *retval = -1;
      return EINVAL;
    }
    if ((start + len - 1) > RT_SCHED_HIST_EVTS) {
      len = (RT_SCHED_HIST_EVTS + 1) - start;
    }
  }

  /* 
   * Lock the event buffer to prevent further events.  Have to raise
   * spl since the lock is taken at interrupt level, too.
   */
  s = splsched();
  rt_sched_hist_lock();

  /* Copy the control values to the user's buffer */
  if (copyout((caddr_t)&rt_sched_hist.ctrl[start], buf, len)) {
    rt_sched_hist_unlock();
    splx(s);
    *retval = -1;
    return EFAULT;
  }
  
  rt_sched_hist_unlock();
  splx(s);
  *retval = len;
  return 0;
}

static int schedmon_set_ctrl(curp, retval, len, buf, start)
     register struct proc	*curp;
     register long 		*retval;
     register long		len;
     register caddr_t		buf;
     register long		start;
{
  register int s;
  register int no_privs;
  register int i;
  rt_sched_ctrl_t temp[RT_SCHED_HIST_EVTS+1];

  /* 
   * See if user has privileges to change control values.  For now, only
   * allow the superuser to change the controls.
   */
  ASSERT(syscall_on_master());
  BM(PROC_LOCK(curp));
  no_privs = (curp->p_rcred->cr_uid && curp->p_ruid);
  BM(PROC_UNLOCK(curp));
  if (no_privs) {
    *retval = -1;
    return EPERM;
  }

  /* Return the range of control values if no control info was requested */
  if (len == 0) {
    *retval = RT_SCHED_HIST_EVTS + 1;
    return 0;
  }
  else {
    if ((start < 0) || (start > RT_SCHED_HIST_EVTS + 1)) {
      *retval = -1;
      return EINVAL;
    }
    if ((start + len) > (RT_SCHED_HIST_EVTS + 1)) {
      len = (RT_SCHED_HIST_EVTS + 1) - start;
    }
  }

  /* 
   * Copy the user's control bits onto the kernel stack.  This
   * way we know that we can touch them.  Of course, we should worry
   * about the size of the kernel stack here, and we don't...
   */
  if (copyin(buf, (caddr_t)&temp[start], len)) {
    rt_sched_hist_unlock();
    splx(s);
    *retval = -1;
    return EFAULT;
  }
  
  /*
   * Lock the event buffer to prevent further events.  Have to raise
   * spl since the lock is taken at interrupt level, too.
   */
  s = splsched();
  rt_sched_hist_lock();

  /*
   * OR together the user's control values and the real control values.
   */
  for (i = start; i < start + len; i++) {
    rt_sched_hist.ctrl[i] |= temp[i];
  }

  rt_sched_hist_unlock();
  splx(s);
  *retval = len;
  return 0;
}

static int schedmon_clr_ctrl(curp, retval, len, buf, start)
     register struct proc	*curp;
     register long		*retval;
     register long		len;
     register caddr_t		buf;
     register long		start;
  {
  register int s;
  register int no_privs;
  register int i;
  rt_sched_ctrl_t temp[RT_SCHED_HIST_EVTS+1];

  /* 
   * See if user has privileges to change control values.  For now, only
   * allow the superuser to change the controls.
   */
  ASSERT(syscall_on_master());
  BM(PROC_LOCK(curp));
  no_privs = (curp->p_rcred->cr_uid & curp->p_ruid);
  BM(PROC_UNLOCK(curp));
  if (no_privs) {
    *retval = -1;
    return EPERM;
  }

  /* Return the range of control values if no control info was requested */
  if (len == 0) {
    *retval = RT_SCHED_HIST_EVTS + 1;
    return 0;
  }
  else {
    if ((start < 0) || (start > RT_SCHED_HIST_EVTS + 1)) {
      *retval = -1;
      return EINVAL;
    }
    if ((start + len) > (RT_SCHED_HIST_EVTS + 1)) {
      len = (RT_SCHED_HIST_EVTS + 1) - start;
    }
  }

  /* 
   * Copy the user's control bits onto the kernel stack.  This
   * way we know that we can touch them.  Of course, we should worry
   * about the size of the kernel stack here, and we don't...
   */
  if (copyin(buf, (caddr_t)&temp[start], len)) {
    rt_sched_hist_unlock();
    splx(s);
    *retval = -1;
    return EFAULT;
  }
  
  /*
   * Lock the event buffer to prevent further events.  Have to raise
   * spl since the lock is taken at interrupt level, too.
   */
  s = splsched();
  rt_sched_hist_lock();

  /*
   * Clear the particular bits specified.
   */
  for (i = start; i < start + len; i++) {
    rt_sched_hist.ctrl[i] &= ~temp[i];
  }

  rt_sched_hist_unlock();
  splx(s);
  *retval = len;
  return 0;
}
#endif /* RT_SCHED_MON */
