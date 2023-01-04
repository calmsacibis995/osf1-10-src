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
 *	@(#)$RCSfile: user.h,v $ $Revision: 4.3.18.4 $ (DEC) $Date: 1993/05/22 17:58:58 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.
 * All Rights Reserved.
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef	_SYS_USER_H_
#define _SYS_USER_H_

#include <mach/boolean.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/namei.h>
#include <sys/ucred.h>
#include <sys/sem.h>
#include <machine/pcb.h>
#include <sys/audit.h>
#include <sys/siginfo.h>
#ifdef	_KERNEL
#include <sys/unix_defs.h>
#include <sys/security.h>
#else
#ifndef	_KERN_LOCK_H_
#define _KERN_LOCK_H_
typedef int simple_lock_data_t;			/* XXX */
#endif
#endif	/* _KERNEL */


/*
 * Per process structure containing data that
 * isn't needed in core when the process is swapped out.
 */

#define MAXCOMLEN	16		/* <= MAXNAMLEN, >= sizeof(ac_comm) */
#define MAXLOGNAME      12              /* >= UT_NAMESIZE */

struct flag_field {
	int	fi_flag;
#ifdef	_KERNEL
	udecl_simple_lock_data(,*fi_lock)
#endif
};

#define NOFILE_IN_U    64

#if	(defined(_KERNEL) || defined(SHOW_UTT))
/*
 *	Per-thread U area.
 *
 *	It is likely that this structure contains no fields that must be
 *	saved between system calls.
 */
struct uthread {
#ifdef __alpha
	long	*uu_ar0;		/* address of users saved R0 */
#else
	int	*uu_ar0;		/* address of users saved R0 */
#endif

/* namei & co. */
	struct unameicache {		/* last successful directory search */
		int nc_prevoffset;	/* offset at which last entry found */
		ino_t nc_inumber;	/* inum of cached directory */
		dev_t nc_dev;		/* dev of cached directory */
		time_t nc_time;		/* time stamp for cache entry */
	} uu_ncache;
	struct	nameidata uu_nd;
	long	uu_spare[8];

/* thread exception handling */
	int	uu_code;			/* ``code'' to trap */
	char uu_cursig;				/* p_cursig for exc. */
	sigset_t  uu_sig;			/* p_sig for exc. */

/* per thread signal state */
	sig_t	uu_tsignal[NSIG+1];	/* disposition of signals */
#ifdef  _KERNEL
	queue_head_t	uu_sigqueue;	/* queue of sigqueue structs */
	sigqueue_t	uu_curinfo;	/* p_curinfo for exc. */
#else
	void		*uu_sigqueue;
	void		*uu_curinfo;
#endif
#ifdef __alpha
/* per thread ieee floating point state */
	ulong_t	uu_ieee_fp_trap_pc;
	ulong_t	uu_ieee_fp_trigger_sum;
	uint_t	uu_ieee_fp_trigger_inst;

        /* the following ieee variables are set via hal_sysinfo(2) */
	ulong_t	uu_ieee_fp_control;
	ulong_t	uu_ieee_set_state_at_signal;
	ulong_t	uu_ieee_fp_control_at_signal;
	ulong_t	uu_ieee_fpcr_at_signal;
#endif /* __alpha */

/* audit information */
	int     uu_event;		   /* audit event             */
	int     uu_vno_indx;		   /* # of vno_{dev,num} used */
	dev_t   uu_vno_dev[AUD_VNOMAX];	   /* vnode dev's referenced by cur proc */
	ino_t   uu_vno_num[AUD_VNOMAX];	   /* vnode num's referenced by cur proc */
	uint_t  uu_vno_aud[AUD_VNOMAX];	   /* vnode audit-mode bits   */
	mode_t  uu_vno_mode[AUD_VNOMAX];   /* vnode mode flag         */
	u_int   uu_set_uids_snap;          /* snapshot of set_uids    */

};

struct utask_nd {
	struct vnode *utnd_cdir;	/* current directory */
	struct vnode *utnd_rdir;	/* root directory of current process */
#ifdef	_KERNEL
	udecl_simple_lock_data(,utnd_lock)
#endif
};

/*
 * Structures associated with the per-process open file table.
 */
struct ufile_state {
	struct	file *uf_ofile[NOFILE_IN_U];/* file structs of open files */
	char	uf_pofile[NOFILE_IN_U];	/* per-process flags of open files */
	int	uf_lastfile;		/* high-water mark of uf_ofile */
#ifdef	_KERNEL
	udecl_simple_lock_data(,uf_ofile_lock) 
#endif
	/*
	 * If greater than NOFILE_IN_U file descriptors are allocated,
	 * uf_ofile_of and uf_pofile_of are used to reference the KALLOC'ed
	 * buffers which store the additional entries.
	 */
	u_int   uf_of_count;
	struct  file    **uf_ofile_of; /* Pointer to KALLOC'ed buffer */
	char            *uf_pofile_of; /* Pointer to KALLOC'ed buffer */
};

/*
 *	Per-task U area - global process state.
 */
struct utask {
	struct	proc *uu_procp;		/* pointer to proc structure */
	char	uu_comm[MAXCOMLEN + 1];

/* 1.1 - processes and protection */
#if	ibmrt
	char	uu_calltype;	/* ROMP_DUALCALL 0 - old calling sequence */
#endif
	char    uu_logname[MAXLOGNAME]; /* login name, if available */
	int	uu_uswitch;		/* uswitch vaulues & flags */

/* 1.2 - memory management */
	size_t	uu_tsize;		/* text size (clicks) */
	size_t	uu_dsize;		/* data size (clicks) */
	size_t	uu_ssize;		/* stack size (clicks) */
	caddr_t	uu_text_start;		/* text starting address */
	caddr_t	uu_data_start;		/* data starting address */
	caddr_t	uu_stack_start;		/* stack starting address */
	caddr_t	uu_stack_end;		/* stack ending address */
	boolean_t uu_stack_grows_up;	/* stack grows at high end? */
	time_t	uu_outime;		/* user time at last sample */

/* 1.3 - signal management */
	sig_t	uu_signal[NSIG+1];	/* disposition of signals */
	sigset_t uu_sigmask[NSIG+1]; 	/* signals to be blocked */
#ifdef	i386
	int	(*uu_sigreturn)();
#endif
#ifdef	multimax
	int	(*uu_sigcatch)();	/* used as a way not to do tramp. */
#endif
#if	defined(balance) || defined(mips) || defined (__alpha)
	int	(*uu_sigtramp)();	/* signal trampoline code */
#endif
	sigset_t uu_sigonstack;		/* signals to take on sigstack */
	sigset_t uu_sigintr;		/* signals that interrupt syscalls */
	sigset_t uu_oldmask;		/* saved mask from before sigpause */
	sigset_t uu_sigresethand;	/* signals with old behavior */
	sigset_t uu_signodefer;		/* signals which don't mask self */
	stack_t  uu_sigstack; 		/* sp & on stack state variable */

/* 1.4 - descriptor management */
	struct ufile_state uu_file_state;	/* open file information */
#define UF_EXCLOSE 	0x1		/* auto-close on exec */
#define UF_MAPPED 	0x2		/* mapped from device */
	struct utask_nd uu_utnd;

	short	uu_cmask;		/* mask for file creation */

/* 1.5 - timing and statistics */
	struct	rusage uu_ru;		/* stats for this proc */
	struct	rusage uu_cru;		/* sum of stats for reaped children */
	long	uu_ioch;		/* # of chars read/written */
	struct	itimerval uu_timer[3];
	struct	timeval uu_start;
	struct	flag_field uu_acflag;

	struct uuprof {			/* profile arguments */
		simple_lock_data_t *pr_lock;	/* lock for thread updating */
		short	*pr_base;	/* buffer base */
		u_long pr_size;	/* buffer size */
		u_long pr_off;	/* pc offset */
		u_long pr_scale;	/* pc scaling */
	} uu_prof;

	u_short	uu_maxuprc;		/* max processes per UID (per tree) */
/* 1.6 - resource controls */
	struct	rlimit uu_rlimit[RLIM_NLIMITS];

/* sysv ipc */
	struct sem_undo *uu_semundo; 	/* semaphore undo structure */
	u_short		 uu_shmsegs;	/* # attached shared-memory-segments */

	u_char	uu_lflags;		/* process lock flags */
#define UL_TXTLOCK	1
#define UL_DATLOCK	2
#define UL_STKLOCK	4
#define UL_PROLOCK	8
#define UL_ALL_FUTURE	16

/* pointers and length to args and env */
        char		*uu_argp;
        char		*uu_envp;
        u_short		uu_arg_size;
        u_short		uu_env_size;
        
/* pointer to compatability module control block */
	struct compat_mod *uu_compat_mod;
        
/* pointer to chain of LMF actions to do at process exit */
	struct exit_actn *uu_exitp;    /* _LMF_ */
        
/* audit masks and control flag */
	u_int  uu_auditmask[AUDIT_INTMASK_LEN];	/* auditmask          */
	u_int  uu_audit_cntl;			/* audit control flag */
	u_int  uu_set_uids;			/* incr on uid change */

#ifdef	_KERNEL
	udecl_simple_lock_data(,uu_timer_lock) /* protects u_time[] */
	udecl_simple_lock_data(,uu_handy_lock) /* handy lock for misc. data */
#endif
};

#endif	/* (defined(_KERNEL) || defined(SHOW_UTT)) */

struct	user {
	struct	pcb u_pcb;
	struct	proc *u_procp;		/* pointer to proc structure */
#ifdef __alpha
	long	*u_ar0;			/* address of users saved R0 */
#else
	int	*u_ar0;			/* address of users saved R0 */
#endif
	char	u_comm[MAXCOMLEN + 1];
        
/* ssycall parameters, results and catches */
	int	u_arg[8];		/* arguments to current system call */
					/* now only used for signal */

/* 1.1 - processes and protection */
	char	u_logname[MAXLOGNAME];	/* login name, if available */
	int	u_uswitch;		/* uswitch values and flags */

/* 1.2 - memory management */
	size_t	u_tsize;		/* text size (clicks) */
	size_t	u_dsize;		/* data size (clicks) */
	size_t	u_ssize;		/* stack size (clicks) */
	caddr_t	u_text_start;		/* text starting address */
	caddr_t	u_data_start;		/* data starting address */
	caddr_t	u_stack_start;		/* stack starting address */
	caddr_t	u_stack_end;		/* stack ending address */
	int	u_stack_grows_up;	/* stack grows at high end? */
	time_t	u_outime;		/* user time at last sample */

/* 1.3 - signal management */
	sig_t	u_signal[NSIG+1];	/* disposition of signals */
	sigset_t u_sigmask[NSIG+1];	/* signals to be blocked */
	sigset_t u_sigonstack;		/* signals to take on sigstack */
	sigset_t u_sigintr;		/* signals that interrupt syscalls */
	sigset_t u_oldmask;		/* saved mask from before sigpause */
	int	u_code;			/* ``code'' to trap */
	stack_t u_sigstack;		/* sp & on stack state variable */
#define u_onstack	u_sigstack.ss_flags
#define u_sigsflags	u_sigstack.ss_flags
#define u_sigsp		u_sigstack.ss_sp
#define u_sigssz	u_sigstack.ss_size

/* 1.4 - descriptor management */
/* This information must exactly match ufile_state structure */
	struct	file *u_ofile[NOFILE_IN_U];/* file structs of open files */
	char	u_pofile[NOFILE_IN_U];	/* per-process flags of open files */
	int	u_lastfile;		/* high-water mark of u_ofile */
	/*
	 * If greater than NOFILE_IN_U file descriptors are allocated,
	 * uf_ofile_of and uf_pofile_of are used to reference the KALLOC'ed
	 * buffers which store the additional entries.
	 */
	u_int   u_of_count;
	struct  file    **u_ofile_of; /* Pointer to KALLOC'ed buffer */
	char            *u_pofile_of; /* Pointer to KALLOC'ed buffer */
#define UF_EXCLOSE 	0x1		/* auto-close on exec */
#define UF_MAPPED 	0x2		/* mapped from device */
				/* These definitions must match utask_nd */
	struct vnode *u_cdir;	/* current directory */
	struct vnode *u_rdir;	/* root directory of current process */
	short	u_cmask;		/* mask for file creation */

/* 1.5 - timing and statistics */
	struct	rusage u_ru;		/* stats for this proc */
	struct	rusage u_cru;		/* sum of stats for reaped children */
	struct	itimerval u_timer[3];
	int	u_XXX[3];
	struct	timeval u_start;
	struct	flag_field u_acflag;

	struct uprof {			/* profile arguments */
		short	*pr_base;	/* buffer base */
		u_long pr_size;	/* buffer size */
		u_long pr_off;	/* pc offset */
		u_long pr_scale;	/* pc scaling */
	} u_prof;
	u_short	u_maxuprc;		/* max processes per UID (per tree) */

/* 1.6 - resource controls */
	struct	rlimit u_rlimit[RLIM_NLIMITS];

/* namei & co. */
	struct nameicache {		/* last successful directory search */
		int nc_prevoffset;	/* offset at which last entry found */
		ino_t nc_inumber;	/* inum of cached directory */
		dev_t nc_dev;		/* dev of cached directory */
		time_t nc_time;		/* time stamp for cache entry */
	} u_ncache;
	struct	nameidata u_nd;
#define UL_TXTLOCK	1
#define UL_DATLOCK	2

/* compatability module pointer */
	struct compat_mod *u_compat_mod;

/* pointer to chain of LMF actions to do at process exit */
	struct exit_actn *u_exitp;     /* _LMF_ */

	int	u_stack[1];
};

/* u_error codes */
#include <sys/errno.h>

#ifdef	_KERNEL
#include <kern/thread.h>

#ifndef	u
#ifdef	multimax
extern struct u_address	active_uareas[NCPUS];
#define u       (active_uareas[cpu_number()])
#else	/* multimax */
#ifdef	balance
#define u	(*(struct u_address *) 0x40)
#else	/* balance */
#ifndef mips	/* need to know size of uthread & pcb before def. u */
#define u	(current_thread()->u_address)
#endif	/* mips */
#endif	/* balance */
#endif	/* multimax */
#endif	/* u */

#define u_pcb		uthread->uu_pcb
#define u_procp		utask->uu_procp
#define u_ar0		uthread->uu_ar0
#define u_comm		utask->uu_comm
#define u_calltype	utask->uu_calltype	/* ROMP_DUALCALL */
#define u_logname	utask->uu_logname
#define u_uswitch	utask->uu_uswitch
#define	u_utnd		utask->uu_utnd
#define	u_file_state	utask->uu_file_state
/*
 * These macros assume the FD table is locked!
 */
#define U_OFILE(fd, ufp) ((unsigned)(fd) < NOFILE_IN_U ? \
			(ufp)->uf_ofile[(fd)] : \
			(ufp)->uf_ofile_of[(fd) - NOFILE_IN_U])

#define U_OFILE_SET(fd, value, ufp) { \
                    if ((unsigned)(fd) < NOFILE_IN_U ) \
			(ufp)->uf_ofile[(fd)] = (value); \
                    else \
                        (ufp)->uf_ofile_of[(fd) - NOFILE_IN_U] = (value); \
}

#define U_POFILE(fd, ufp) ((unsigned)(fd) < NOFILE_IN_U ? \
			(ufp)->uf_pofile[(fd)] : \
			(ufp)->uf_pofile_of[(fd) - NOFILE_IN_U])

#define U_POFILE_SET(fd, value, ufp) { \
                    if ((unsigned)(fd) < NOFILE_IN_U ) \
			(ufp)->uf_pofile[(fd)] = (value); \
                    else \
                        (ufp)->uf_pofile_of[(fd) - NOFILE_IN_U] = (value); \
}

#ifdef __alpha
#define	u_ieee_fp_trigger_sum		uthread->uu_ieee_fp_trigger_sum
#define	u_ieee_fp_trigger_inst		uthread->uu_ieee_fp_trigger_inst
#define	u_ieee_fp_trap_pc		uthread->uu_ieee_fp_trap_pc
#define	u_ieee_fp_control		uthread->uu_ieee_fp_control
#define	u_ieee_set_state_at_signal	uthread->uu_ieee_set_state_at_signal
#define	u_ieee_fp_control_at_signal	uthread->uu_ieee_fp_control_at_signal
#define	u_ieee_fpcr_at_signal		uthread->uu_ieee_fpcr_at_signal
#endif /* __alpha */

#define uu_cdir		uu_utnd.utnd_cdir
#define uu_rdir		uu_utnd.utnd_rdir

#define u_nd		uthread->uu_nd
#define u_spare		uthread->uu_spare
#define u_cdir		utask->uu_cdir
#define u_rdir		utask->uu_rdir
#define u_cred		u_nd.ni_cred
#define	u_ruid		u_procp->p_ruid
#define	u_rgid		u_procp->p_rgid
#define u_uid		u_cred->cr_uid
#define u_gid		u_cred->cr_gid
#define u_ngroups	u_cred->cr_ngroups
#define u_groups	u_cred->cr_groups

#define u_tsize		utask->uu_tsize
#define u_dsize		utask->uu_dsize
#define u_ssize		utask->uu_ssize
#define u_text_start	utask->uu_text_start
#define u_data_start	utask->uu_data_start
#define u_stack_start	utask->uu_stack_start
#define u_stack_end	utask->uu_stack_end
#define u_stack_grows_up utask->uu_stack_grows_up
#define u_outime	utask->uu_outime

#define u_signal	utask->uu_signal
#define u_tsignal	uthread->uu_tsignal
#ifdef	i386
#define	u_sigreturn	utask->uu_sigreturn
#endif
#ifdef	multimax
#define u_sigcatch	utask->uu_sigcatch
#endif
#if	defined(balance) || defined(mips) || defined (__alpha)
#define u_sigtramp	utask->uu_sigtramp
#endif
#define u_sigmask	utask->uu_sigmask
#define u_sigonstack	utask->uu_sigonstack
#define u_sigintr	utask->uu_sigintr
#define u_oldmask	utask->uu_oldmask
#define u_code		uthread->uu_code
#define u_sigstack	utask->uu_sigstack

#define u_onstack	u_sigstack.ss_flags
#define u_sigsflags	u_sigstack.ss_flags
#define u_sigsp		u_sigstack.ss_sp
#define u_sigssz	u_sigstack.ss_size

#define	u_sigresethand	utask->uu_sigresethand
#define	u_signodefer	utask->uu_signodefer

#define u_lastfile	u_file_state.uf_lastfile
#if	UNIX_LOCKS
#define	u_ofile_lock	u_file_state.uf_ofile_lock
#define	u_timer_lock	utask->uu_timer_lock
#define u_handy_lock    utask->uu_handy_lock
#endif
#define u_cmask		utask->uu_cmask

#define u_ru		utask->uu_ru
#define u_cru		utask->uu_cru
#define u_ioch		utask->uu_ioch
#define u_timer		utask->uu_timer
#define u_XXX		utask->uu_XXX
#define u_start		utask->uu_start
#define u_acflag	utask->uu_acflag

#define u_prof		utask->uu_prof

#define u_maxuprc	utask->uu_maxuprc

#define u_lflags	utask->uu_lflags

#define u_rlimit	utask->uu_rlimit

#define u_ncache	uthread->uu_ncache

#define u_sig		uthread->uu_sig
#define u_cursig	uthread->uu_cursig
#define	u_sigqueue	uthread->uu_sigqueue
#define	u_curinfo	uthread->uu_curinfo

#define u_semundo	utask->uu_semundo
#define u_shmsegs	utask->uu_shmsegs

#define u_argp          utask->uu_argp
#define u_envp          utask->uu_envp
#define u_arg_size      utask->uu_arg_size
#define u_env_size      utask->uu_env_size
#define u_compat_mod    utask->uu_compat_mod
#define u_exitp         utask->uu_exitp           /* _LMF_ */

#define u_event		uthread->uu_event
#define u_vno_indx	uthread->uu_vno_indx
#define u_vno_dev	uthread->uu_vno_dev
#define u_vno_num	uthread->uu_vno_num
#define u_vno_aud	uthread->uu_vno_aud
#define u_vno_mode	uthread->uu_vno_mode
#define u_auditmask	utask->uu_auditmask
#define u_audit_cntl	utask->uu_audit_cntl
#define u_set_uids	utask->uu_set_uids
#define u_set_uids_snap	uthread->uu_set_uids_snap

#define	U_HANDY_LOCK()		usimple_lock(&u.u_handy_lock)
#define	U_HANDY_UNLOCK()	usimple_unlock(&u.u_handy_lock)
#define	U_HANDY_LOCK_INIT(up)	usimple_lock_init(&(up)->uu_handy_lock)

#define	UTND_LOCK(utndp)	usimple_lock(&(utndp)->utnd_lock)
#define	UTND_UNLOCK(utndp)	usimple_unlock(&(utndp)->utnd_lock)
#define	UTND_LOCK_INIT(utndp)	usimple_lock_init(&(utndp)->utnd_lock)

#define	FLAG_LOCK(flagp)	usimple_lock((flagp)->fi_lock)
#define	FLAG_UNLOCK(flagp)	usimple_unlock((flagp)->fi_lock)

/*
 * The U_TIMER_LOCK must always be taken before the time lock.  Or else!
 * The U_TIMER_LOCK must always be held at splhigh.
 */
#define	U_TIMER_LOCK()		usimple_lock(&u.u_timer_lock)
#define	U_TIMER_UNLOCK()	usimple_unlock(&u.u_timer_lock)
#define	U_TIMER_LOCK_INIT(p)	usimple_lock_init(&(p)->uu_timer_lock)

#endif	/* _KERNEL */

#endif	/* _SYS_USER_H_ */
