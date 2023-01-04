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
/*
 *	@(#)$RCSfile: proc.h,v $ $Revision: 4.3.16.8 $ (DEC) $Date: 1993/09/21 22:15:01 $
 */ 
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
 * OSF/1 Release 1.0.1
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
 * Copyright (c) 1982, 1986, 1989  Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)proc.h	3.3 (ULTRIX/OSF)	5/21/91
 */

#ifndef	_SYS_PROC_H_
#define _SYS_PROC_H_

#ifndef ASSEMBLER
#include <mach/boolean.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#ifdef	_KERNEL
#include <kern/lock.h>
#else	/* _KERNEL */
#ifndef	_KERN_LOCK_H_
#define _KERN_LOCK_H_
typedef int simple_lock_data_t;
#endif	/* _KERN_LOCK_H_ */
#endif	/* _KERNEL */

#include <sys/rt_limits.h>		  /* #if RT, used for psx4 timers*/
#include <sys/siginfo.h>

/*
 * One structure allocated per session.
 */
struct	session {
	struct	proc *s_leader;	/* session leader */
 	pid_t	s_id;		/* session ID */
	int	s_count;	/* ref cnt; pgrps in session */
	struct	vnode *s_ttyvp;	/* vnode of controlling terminal */
 	struct	pgrp *s_pgrps;	/* list of pgrp's in session */
 	struct	pgrp **s_fpgrpp;/* pointer to foreground pgrp pointer */
#ifdef _KERNEL
 	udecl_simple_lock_data(, s_lock)
 	udecl_simple_lock_data(, s_fpgrp_lock)
#endif /* _KERNEL */
};

/*
 * One structure allocated per process group.
 */
struct	pgrp {
	struct	pgrp *pg_hforw;	/* forward link in hash bucket */
	struct	proc *pg_mem;	/* pointer to pgrp members */
	struct	session *pg_session;	/* pointer to session */
        struct  pgrp    *pg_sessnxt;    /* next pgrp in session */
	pid_t	pg_id;		/* pgrp id */
	short	pg_jobc;	/* # procs qualifying pgrp for job control */
};

/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 */
struct	proc {
	struct	proc *p_link;	/* linked list of running processes */
	struct	proc *p_rlink;
	struct	proc *p_nxt;	/* linked list of allocated proc slots */
	struct	proc **p_prev;		/* also zombies, and free proc's */
#ifdef	ibmrt
	short	p_usrpri;	/* user-priority based on p_cpu and p_nice */
	short	p_pri;		/* priority, negative is high */
	short	p_cpu;		/* cpu usage for scheduling */
#else
	char	p_usrpri;	/* user-priority based on p_cpu and p_nice */
	char	p_pri;		/* priority, negative is high */
	char	p_cpu;		/* cpu usage for scheduling */
#endif
	char	p_stat;
	char	p_time;		/* resident time for scheduling */
#ifdef	ibmrt
	short	p_nice;		/* nice for cpu usage */
#else
	char	p_nice;		/* nice for cpu usage */
#endif
	char	p_slptime;	/* time since last block */
	char	p_cursig;
	sigset_t p_sig;		/* signals pending to this process */
	sigset_t p_sigmask;	/* current signal mask */
	sigset_t p_sigignore;	/* signals being ignored */
	sigset_t p_sigcatch;	/* signals being caught by user */
	sigset_t p_siginfo;	/* signals expecting siginfo delivery */
	int	p_flag;
	uid_t	p_ruid;		/* real user id */
	uid_t	p_svuid;	/* saved effective user id */
	gid_t	p_rgid;		/* real group id */
	gid_t	p_svgid;	/* saved effective group id */
	struct	ucred *p_rcred;	/* user credentials (uid, gid, etc) */
	pid_t	p_pid;		/* unique process id */
	pid_t	p_ppid;		/* process id of parent */
	int	p_xstat;	/* exit information */
	struct	rusage *p_ru;	/* mbuf holding exit information */
	size_t 	p_rssize; 	/* current resident set size in clicks */
	size_t	p_maxrss;	/* copy of u.u_limit[MAXRSS] */
	size_t	p_swrss;	/* resident set size before last swap */
	swblk_t	p_swaddr;	/* disk address of u area when swapped */
	int	p_habitat;	/* habitat flag for process */
	int	p_cpticks;	/* ticks of cpu time */
	fixpt_t	p_pctcpu;	/* %cpu for this process during p_time */
	short	p_ndx;		/* proc index for memall (because of vfork) */
	short	p_idhash;	/* hashed based on p_pid for kill+exit+... */
	struct	proc *p_pptr;	/* pointer to process structure of parent */
	struct	proc *p_cptr;	/* pointer to youngest living child */
	struct	proc *p_osptr;	/* pointer to older sibling processes */
	struct	proc *p_ysptr;	/* pointer to younger siblings */
	struct 	pgrp *p_pgrp;	/* pointer to process group */
#define p_session p_pgrp->pg_session
#define p_pgid	p_pgrp->pg_id
	struct	proc *p_pgrpnxt; /* pointer to next process in process group */
	struct	itimerval p_realtimer;
	int	p_traceflag;	/* kernel trace points */
	int	p_uac;		/* unaligned access control flags */
	struct	vnode *p_tracep;/* trace to vnode */
#if defined(tahoe)
	int	p_ckey;		/* code cache key */
	int	p_dkey;		/* data cache key */
#endif
	dev_t	    p_logdev;	/* logged-in controlling device */
	struct task	*task;	/* corresponding task */
	struct utask	*utask; /* utask structure of corresponding task */
	struct thread	*thread;/* corresponding thread */
	simple_lock_data_t siglock;	/* multiple thread signal lock */
	boolean_t	sigwait;	/* indication to suspend */
#ifdef _KERNEL
	queue_head_t	p_strsigs;	/* Streams signals from I_SETSIG */
#endif
	struct thread	*exit_thread;	/* XXX Which thread is exiting?
					   XXX That thread does no signal
					   XXX processing, other threads
					   XXX must suspend. */
#ifdef	sun
	struct	proc *p_tptr;	/* pointer to process structure of tracer */
#endif
#ifdef i386
	void	*cxenix;	/* for Xenix compatibility */
#endif
#if	defined(_KERNEL) || defined(_POSIX_4SOURCE)
	psx4_tblock_t	*p_psx4_timer;	/* pointer to POSIX.4 timer array */
#else
	void		*p_psx4_timer;	/* else pointer to void */
#endif
	short	p_realtimer_coe;/* clear p_realtimer on exec */
	struct vnode 	*p_vnptr;  /* vnode in /proc fs if open via /proc */
	uid_t		p_auid;   /* audit id */
#ifdef	_KERNEL
	queue_head_t
		p_sigqueue;	/* queue of sigqueue structs */
	sigqueue_t
		p_curinfo;	/* current sigqueue struct */
#else
	void	*p_sigqueue;
	void	*p_curinfo;
#endif
	char p_wcode;		/* wait code to explain contents of p_xstat */
#ifdef	_KERNEL
	udecl_simple_lock_data(,p_lock)		/* general proc lock */
	udecl_simple_lock_data(,p_timer_lock)	/* protects realtimer */
#endif
	int	p_pr_qflags;	/* /proc quick flags TRACING and STOPEXEC */
};

#ifdef	_KERNEL
#include <kern/macro_help.h>

extern pid_t PID_RSRVD; /* tcgetpgrp() result when no foreground pgrp */
/*
 * Pid's which are less than zero or greater than PID_MAX are out of range.
 * In some cases, wait4/kill, negative pids are process group ids.  These
 * cases are handled appropriately.  The second half of this macro becomes
 * a no-op if PID_MAX is maximal for its type.  Note that the first half
 * of this macro is always evaluated because a pid_t must be a signed type.
 */
#define PID_INVALID(pid) ((pid_t)(pid) < 0 || (pid_t)(pid) > (pid_t)PID_MAX)

/*
 * Multiprocessor exclusion to a proc data structure.
 * Procs are annoying but must be kept around for backwards
 * compatibility.
 */
#define	PROC_LOCK(p)		usimple_lock(&(p)->p_lock)
#define	PROC_UNLOCK(p)		usimple_unlock(&(p)->p_lock)
#define	PROC_LOCK_INIT(p)	usimple_lock_init(&(p)->p_lock)

/*
 * The proc timer lock must always be held at splhigh.  Furthermore, there
 * is a "natural" lock ordering between the proc timer lock and the time
 * lock:  always take the proc timer lock FIRST.  Otherwise, well, ....
 */
#define	PROC_TIMER_LOCK_INIT(p)	usimple_lock_init(&(p)->p_timer_lock)
#define	PROC_TIMER_LOCK(p)	usimple_lock(&(p)->p_timer_lock)
#define	PROC_TIMER_UNLOCK(p)	usimple_unlock(&(p)->p_timer_lock)

/*
 * Interface to other struct-specific locks.
 */
#define	SESS_LOCK(s)		usimple_lock(&(s)->s_lock)
#define	SESS_UNLOCK(s)		usimple_unlock(&(s)->s_lock)
#define	SESS_LOCK_INIT(s)	usimple_lock_init(&(s)->s_lock)

#define	SESS_FPGRP_LOCK(s)	usimple_lock(&(s)->s_fpgrp_lock)
#define	SESS_FPGRP_UNLOCK(s)	usimple_unlock(&(s)->s_fpgrp_lock)
#define	SESS_FPGRP_LOCK_INIT(s)	usimple_lock_init(&(s)->s_fpgrp_lock)
/*
 *	Signal lock has the following states and corresponding actions
 *	that the locker must take:
 *
 *	Locked (siglock) - simple lock acquires the lock when free.
 *	Unlocked (sigwait = 0 && exit_thread == 0)  simple lock.
 *	Waiting (sigwait != 0) - Drop siglock after acquiring it, and
 *		call thread_block().  Thread that set the lock to
 *		wait has done a task_suspend().
 *	Exiting (exit_thread != 0) - The thread in exit_thread is going to
 *		call exit().  If we're not that thread, permanently stop
 *		in favor of that thread.  If we're that thread, immediately
 *		bail out (no signal processing is permitted once we're
 *		committed to exit) and indicate that signals should not be
 *		processed.  If we have been asked to halt, bail out and
 *		indicate that signals should be processed (to clean up any
 *		saved state).
 *
 *	The logic for this is in the sig_lock_or_return macro.
 */

/*
 *	Try to grab signal lock.  If we are already exiting,
 *	execute 'false_return'.  If some other thread is exiting,
 *	hold.  If we must halt, execute 'true_return'.
 */
#define sig_lock_or_return(p, false_return, true_return)	\
MACRO_BEGIN							\
	simple_lock(&(p)->siglock);				\
	while ((p)->sigwait || (p)->exit_thread) {		\
	    simple_unlock(&(p)->siglock);			\
	    if ((p)->exit_thread) {				\
		if (current_thread() == (p)->exit_thread) {	\
		    /*						\
		     *	Already exiting - no signals.		\
		     */						\
		    false_return;				\
		}						\
		else {						\
		    /*						\
		     *	Another thread has called exit -	\
		     *	stop (until terminate request).		\
		     */						\
		    thread_hold(current_thread());		\
		}						\
	    }							\
	    thread_block();					\
	    if (thread_should_halt(current_thread())) {		\
		/*						\
		 *	Terminate request - clean up.		\
		 */						\
		true_return;					\
	    }							\
	    simple_lock(&(p)->siglock);				\
	}							\
MACRO_END

/*
 *	Try to grab signal lock.  Return from caller if
 *	we must halt or task is exiting.
 */
#define sig_lock(p)		sig_lock_or_return(p, return, return)

#define sig_lock_simple(p)	simple_lock(&(p)->siglock)

#define sig_unlock(p)		simple_unlock(&(p)->siglock)

#define sig_lock_to_wait(p)			\
MACRO_BEGIN					\
	(p)->sigwait = TRUE; 			\
	simple_unlock(&(p)->siglock);		\
MACRO_END

#define sig_wait_to_lock(p)			\
MACRO_BEGIN					\
	simple_lock(&(p)->siglock); 		\
	(p)->sigwait = FALSE;			\
MACRO_END

/*
 *	sig_lock_to_exit() also shuts down all other threads except the
 *	current one.  There is no sig_exit_to_lock().  The sig_lock is
 *	left in exit state and is cleaned up by exit().
 */

#define sig_lock_to_exit(p)				\
MACRO_BEGIN						\
	(p)->exit_thread = current_thread();		\
	simple_unlock(&(p)->siglock);			\
	(void) task_hold(current_task());		\
	(void) task_dowait(current_task(), FALSE);	\
MACRO_END
#endif	/* _KERNEL */


#define PIDHSZ		64
#define PIDHASH(pid)	((pid) & (PIDHSZ - 1))

#ifdef	_KERNEL
extern pid_t	pidhash[PIDHSZ];
extern struct	proc *pfind();
struct	pgrp *pgrphash[PIDHSZ];
struct 	pgrp *pgfind();		/* find process group by id */
extern struct	proc *proc, *procNPROC;	/* the proc table itself */
extern struct	proc *freeproc, *zombproc, *allproc;
			/* lists of procs in various states */
extern int	nproc;

#define NQS	32		/* 32 run queues */
extern struct	prochd {
	struct	proc *ph_link;	/* linked list of running processes */
	struct	proc *ph_rlink;
} qs[NQS];

#define SESS_LEADER(p)	((p)->p_session->s_leader == (p))
#define PGRP_JOBC(p)	(((p)->p_pgrp != (p)->p_pptr->p_pgrp) && \
			((p)->p_session == (p)->p_pptr->p_session))
#define PCTCPU_SCALE	1000	/* scaling for p_pctcpu */
#endif	/* _KERNEL */

/* stat codes */
/*
 *	MACH uses only NULL, SRUN, SZOMB, and SSTOP.
 */
#define SSLEEP	1		/* awaiting an event */
#define SWAIT	2		/* (abandoned state) */
#define SRUN	3		/* running */
#define SIDL	4		/* intermediate state in process creation */
#define SZOMB	5		/* intermediate state in process termination */
#define SSTOP	6		/* process being traced */

/* flag codes */
#define SLOAD	0x00000001	/* in core */
#define SSYS	0x00000002	/* swapper or pager process */
#define SSYSCALL \
		0x00000004	/* process is preforming a system call */
#define STRC	0x00000010	/* process is being traced */
#define SOMASK	0x00000200	/* restore old mask after taking signal */
#define SWEXIT	0x00000400	/* working on exiting */
#define SPHYSIO	0x00000800	/* doing physical i/o */
#define SVFORK	0x00001000	/* process resulted from vfork() */
#define SPAGV	0x00008000	/* init data space on demand, from vnode */
#define SSEQL	0x00010000	/* user warned of sequential vm behavior */
#define SUANOM	0x00020000	/* user warned of random vm behavior */
#define STIMO	0x00040000	/* timing out during sleep */
#define SCNTD   0x00080000      /* process was continued */
#define SOWEUPC	0x00200000	/* owe process an addupc() call at next ast */
#define SLOGIN  0x00400000      /* mark process as a login for Capacity Limitation */
#define SCTTY	0x00800000	/* has a controlling terminal */
#define SXONLY	0x02000000	/* process image read protected	*/
#define SAIO	0x08000000	/* process performed asych i/o */
#define SSWPKIL	0x10000000	/* process killed due to lack of swap space */
#define	SNOCLDWAIT \
		0x20000000	/* no SIGCHLD when children die */
#define	SNOCLDSTOP \
		0x40000000	/* no SIGCHLD when children stop */
#define SEXEC	0x80000000	/* process called exec */

/* Unused flags */
#if !MACH
#define SLOCK	0x00000004	/* process being swapped out */
#define SSWAP	0x00000008	/* save area flag */
#define SWTED	0x00000020	/* another tracing flag */
#define SULOCK	0x00000040	/* user settable lock in core */
#define SPAGE	0x00000080	/* process in page wait state */
#define SKEEP	0x00000100	/* another flag to prevent swap out */
#define SVFDONE	0x00002000	/* another vfork flag */
#define SNOVM	0x00004000	/* no vm, parent in a vfork() */
#define SACTIVE	0x00080000	/* process is executing */
#define	SOUSIG	0x00100000	/* using old signal mechansim */
#define	SPTECHG	0x01000000	/* pte's for process have changed */
#define SIDLE	0x04000000	/* is an idle process */
#define SKTR	0x20000000	/* pass kernel tracing flags to children */
#endif

/*
 * Caller must lock p to ensure consistent snapshot of state.
 */
#define PROC_ACTIVE(p)  (((p)->p_stat != 0) && ((p)->p_stat != SZOMB) && \
           !((p)->p_flag & SWEXIT))

/*
 * the following hackery is to allow use of the OSF/1 R1.2 ||ized
 * proc mgmt code w/o having all the OSF/1 R1.2 code in "place"
 */
#if (NCPUS == 1)
#define PROCREL_READ_LOCK()     
#define PROCREL_WRITE_LOCK()    
#define PROCREL_UNLOCK()        
#define PROCREL_LOCK_INIT()   
#define PID_LOCK()              
#define PID_UNLOCK()            
#define PID_LOCK_INIT()         
#define UIDHASH_LOCK()          
#define UIDHASH_UNLOCK()        
#define UIDHASH_LOCK_INIT()     

#define GIDHASH_LOCK()          
#define GIDHASH_UNLOCK()        
#define GIDHASH_LOCK_INIT()     

#define PGRPHASH_READ_LOCK()    
#define PGRPHASH_WRITE_LOCK()   
#define PGRPHASH_UNLOCK()       
#define PGRPHASH_LOCK_INIT()    

#define PGRP_READ_LOCK(pg)      
#define PGRP_WRITE_LOCK(pg)     
#define PGRP_WRITE_TO_READ(pg)  
#define PGRP_UNLOCK(pg)         
#define PGRP_LOCK_INIT(pg)      

#define PGRP_REFCNT_LOCK(pg)            
#define PGRP_REFCNT_UNLOCK(pg)          
#define PGRP_REFCNT_LOCK_INIT(pg)       

#define P_REF(p) PROC_ACTIVE(p)           
#define P_UNREF         
#define PG_REF          
#define PG_UNREF        
/*
 * for non BOGUS MEMORY machines 
 * define the PROC_INTR_LOCK as PROC_LOCK
 */


#define PROC_INTR_VAR(v)
#define PROC_INTR_LOCK(p, s)    PROC_LOCK(p)
#define PROC_INTR_UNLOCK(p, s)  PROC_UNLOCK(p)

#define PGRP_READ_LOCK(pg)      
#define PGRP_WRITE_LOCK(pg)     
#define PGRP_WRITE_TO_READ(pg)  
#define PGRP_UNLOCK(pg)         
#define PGRP_LOCK_INIT(pg)      

#define PGRP_REFCNT_LOCK(pg)            
#define PGRP_REFCNT_UNLOCK(pg)          
#define PGRP_REFCNT_LOCK_INIT(pg)       

#define PROC_SDATA_LOCK(p)      
#define PROC_SDATA_UNLOCK(p)    
#define PROC_SDATA_LOCK_INIT(p) 
#define PROC_SDATAUP_LOCK(p)    
#define PROC_SDATAUP_UNLOCK(p)  
#endif /* NCPU == 1 */
  
#endif /* ASSEMBLER */

/* p_uac codes */
#define UAC_NOPRINT    0x00000001	/* Don't report unaligned fixups */
#define UAC_NOFIX      0x00000002	/* Don't fix unaligned errors */
#define UAC_SIGBUS     0x00000004	/* Notify unaligned trap by SIGBUS */

#endif	/* _SYS_PROC_H_ */

