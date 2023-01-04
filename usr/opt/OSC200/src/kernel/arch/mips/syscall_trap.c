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
static char *rcsid = "@(#)$RCSfile: syscall_trap.c,v $ $Revision: 1.1.3.9 $ (DEC) $Date: 1992/10/13 15:37:02 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from trap.c	4.10	(ULTRIX)	12/19/90";
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/***********************************************************************
 *
 *	(Relevant) Modification History: trap.c
 *
 * 24-May-91        Alaa Zeineldine
 *  Added habitat support in syscall()
 *
 * 21-Apr-91	Ron Widyono
 *	Change kast_preemption, hack_preemption, original_preemption, 
 *	pp_count, ast_was_on to rt_preempt_async, rt_preempt_syscall, 
 *	rt_preempt_user, rt_preempt_pp.  Remove some syscall preemption 
 *	debugging.
 *	Remove kernel_ast.
 *
 * 19-Apr-91	Ron Widyono
 *	Always include parallel.h.
 *
 * 6-Apr-91	Ron Widyono
 *	Add syscall funneling calls.
 *	Process Kernel Mode ASTs--preemption point.
 *	Reset ASTs to User Mode when turning AST off.
 *
 * 04-Oct-89 jaw
 *	release all locks held when "longjmp" out of a syscall occurs.  
 *	This is a tempory fix until code in "soclose" is fixed to handle 
 *	interrupted system calls.
 *
 * 18-Nov-1988  afd (for rr)
 *	Added system call trace hooks.
 *
 ***********************************************************************/

#define TRACE(x)

#include "bin_compat.h"

#include <mach_assert.h>
#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#include <sys/secdefines.h>

#if SEC_BASE
#include <sys/security.h>
#include <kern/lock.h>
#include <sys/audit.h>
#endif /* SEC_BASE */

#include <machine/cpu.h>
#include <mach/exception.h>	  /* Machine-indep. exception defs */
#include <builtin/ux_exception.h> /* Unix sw exceptions under EXC_SOFTWARE */
#include <sys/systm.h>
#include <sys/habitat.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <kern/xpr.h> 
#include <kern/parallel.h>	/* unix_master, unix_release */
#if	RT_PREEMPT
#include <kern/ast.h>
#include <sys/preempt.h>
#if	RT_PREEMPT_DEBUG
#include <mach/time_value.h>
#include <mach/boolean.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/queue.h>
#endif	/* RT_PREEMPT */
#endif  /* RT_PREEMPT_DEBUG */

#include <machine/reg.h> 

#include <sys/syscall.h>

extern	int	hz;

extern	int	install_bp();

int nosys();

#if	RT_PREEMPT && RT_PREEMPT_DEBUG
extern int	rt_preempt_syscall;
extern int	rt_preempt_user;
extern int	rt_preempt_async;
extern int	rt_preempt_ast_set;
extern int	rt_preempt_pp;
#endif /* RT_PREEMPT && RT_PREEMPT_DEBUG */

#include <sys/table.h>				/* Statistic info */

#define	SYSCALLTRACE
#ifdef	SYSCALLTRACE
int syscalltracing = 0;
int syscallhalt = 0;
int syscallstop = 50;
#endif
#define NUMSYSCALLARGS 8
int syscalltrace = 0;

int scmax = LAST;		/* max number of system call counts */
int sccount[LAST - FIRST +1];		/* system call counts */

#if BIN_COMPAT
extern int bin_compat_debug;
#endif /BIN_COMPAT*/

syscall(ep, code, sr, cause)
register u_int *ep;
register unsigned code;
{
	int opc;
	struct timeval syst;
	register int	error;
	register struct utask *utask;
	register struct task *task;
	register struct uthread	*uthread;
	register thread_t	t;
	register struct sysent *callp;
	struct args {
		int i[NUMSYSCALLARGS];
	} args;
	int rval[2];
	extern char *syscallnames[];
#if BIN_COMPAT
	unsigned long	habitat;
	struct compat_mod *cm_ptr;
#endif /*BIN_COMPAT*/

	t = current_thread();
#ifdef	ASSERTIONS
	/*
	 *	If this is a MACH-only thread, we cannot do U*X system
	 *	calls.
	 */
	if (u.u_procp == (struct proc *)0) {
	    thread_doexception(current_thread(), EXC_SOFTWARE,
		EXC_UNIX_BAD_SYSCALL, 0);	/* XXX */
	    return;
	}
#endif

	uthread = t->u_address.uthread;
	utask = t->u_address.utask;

 	if (utask->uu_prof.pr_scale > 1)
		syst  = utask->uu_ru.ru_stime;
	error = 0;

	opc = ep[EF_EPC];
	ep[EF_EPC] = opc + 4;		/* normal return executes next inst */

	{
	register regparams, nargs;
	register int *params;

	regparams = EF_A0;

	if (code == 0) {
		/*
		 * indirect system call (syscall), first param is
		 * sys call number
		 */
		code = (unsigned) ep[EF_A0];
		regparams++;
	}

	/*
	 * The following results in callp, cm_ptr, and habitat
	 * being set if they will be used. Any code changes must make
	 * sure they are not used un-assigned. Explicit initialization
	 * was removed to reduce system call overhead when not needed.
	 */
#if BIN_COMPAT
	/* 
	 * Check for a binary compatibility module,
	 * if not, then if code < sysent, we can safely assume
	 *	that it is a  base system call, since the habitat
	 *	number in the high order byte will guarantee that
	 *	a habitat system call will be >= 0x01000000. Which
	 *	should be larger than most reasonable sysent tables
	 * if code > nsysent, then check if it includes a habitat number
	 *	if it does process as a habitat syscall
	 *
	 * if not, then syscall is out of range.
	 */	

	/* case for a binary compatibility module */
	if (u.u_compat_mod) {
		cm_ptr = u.u_compat_mod;
		callp = (* cm_ptr->cm_syscall)(code);
		if (callp == (struct sysent *) 0) {
			callp = &sysent[0];	/* indir (illegal) */
			ep[EF_EPC] = opc;	/* pc = syscall inst */
		}
	}
	/* Case for base system call */
	else if (code < nsysent)
			callp = &sysent[code];

	/* case for syscall with a habitat set */
	else if (code & HABITAT_MASK) {
		habitat = (code >> HABITAT_SHIFT);
		code &= ~HABITAT_MASK;		/* turn off habitat index */
		if(habitat < MAXHABITATS){
			cm_ptr = habitats[habitat];

			/* If there is a pointer, call the syscall function.
			 * A NULL return indicates that code is out of range.
			 */
			if (cm_ptr) {
				callp = (* cm_ptr->cm_syscall)(code);
				if (callp == (struct sysent *) 0) {
					/* syscall not in habitat's range */
					callp = &sysent[0]; 
					ep[EF_EPC] = opc;
				}
			}
			else {
				/* habitats[habitat] is an empty slot */
				callp = &sysent[0]; 
				ep[EF_EPC] = opc; 
			}
		}
		else {
			/* habitat > MAXHABITATS */
			callp = &sysent[0];	/* indir (illegal) */
			ep[EF_EPC] = opc;	/* pc = syscall inst */
		}
	}
	else {
		callp = &sysent[0];	/* indir (illegal) */
		ep[EF_EPC] = opc;	/* pc = syscall inst */
	}
#else	/*BIN_COMPAT*/
	if (code >= nsysent) {
		callp = &sysent[0];	/* indir (illegal) */
		ep[EF_EPC] = opc;	/* just leave pc at the syscall inst */
	} else
		callp = &sysent[code];
#endif /*BIN_COMPAT*/

	params = (int *) &args;
	for (nargs = callp->sy_narg; nargs && regparams <= EF_A3; nargs--)
		*params++ = ep[regparams++];
	if (nargs) {
		if (error = copyin(ep[EF_SP]+4*sizeof(int), params,
		   		   nargs*sizeof(int)))
			goto out;
	}
	}

	rval[0] = 0;
	rval[1] = ep[EF_V1];

#if	MACH_ASSERT
	/*
	 * A stash to aid debugging.
	 */
	uthread->uu_spare[2] = uthread->uu_spare[1];	
#if BIN_COMPAT
	if (cm_ptr)
		uthread->uu_spare[1] = (long) cm_ptr->call_name[code];
	else
#endif /*BIN_COMPAT*/
	uthread->uu_spare[1] = (long) syscallnames[code];
#endif
	/*
	 * Acquire a fresh copy of the task's credentials
	 * for use over the duration of this system call.
	 */
	cr_threadinit(t);

#if	RT_PREEMPT
	/*
	 * If this is not a parallelized system call, funnel it
	 * to the master processor.
	 */
        if (callp->sy_parallel == 0)
                unix_master();
#endif

#ifdef	SYSCALLTRACE
	if (syscalltrace &&
	   (syscalltrace < 0 || syscalltrace == u_procp->p_pid))
		syscalltracing = 1;
	if (syscalltracing) {
		register int i;
		char *cp;

#if BIN_COMPAT
		if (cm_ptr)
		    printf("[P%d %s] %s.%s{%d}", u_procp->p_pid, 
			u.u_comm, cm_ptr->cm_name, 
			cm_ptr->call_name[code], code);
		 else
#endif /*BIN_COMPAT*/
		printf("[P%d %s] %s{%d}", u_procp->p_pid, 
			u.u_comm, syscallnames[code], code);
		cp = "(";
		for (i= 0; i < callp->sy_narg && i < 6; i++) {
			printf("%s%x", cp, args.i[i]);
			cp = ", ";
		}
		printf(")" );
	}
#endif

	XPR(XPR_SYSCALLS, ("syscall start: %c%c%c%c%c%c\n",
		u.u_comm[0], u.u_comm[1], u.u_comm[2],
		u.u_comm[3], u.u_comm[4], u.u_comm[5]));
#if BIN_COMPAT
	if (cm_ptr) 
	XPR(XPR_SYSCALLS, ("syscall start %s.%d: args=0x%x, 0x%x, 0x%x, 0x%x\n",
		cm_ptr->cm_name, code, args.i[0], args.i[1], args.i[2],
		args.i[3], args.i[4]));
	else
#endif /*BIN_COMPAT*/
	XPR(XPR_SYSCALLS, ("syscall start %d: args=0x%x, 0x%x, 0x%x, 0x%x\n",
		code, args.i[0], args.i[1], args.i[2],
		args.i[3], args.i[4]));

	/* This permits passing back the problem svc number
	 * (including habitat) to the signal handler for bad svcs */
	if(*callp->sy_call == &nosys)
		u.u_code = ((habitat << HABITAT_SHIFT) | code);

	/*
	 * Start of /proc code.
	 *
	 * If PRFS_SCENTER in pr_qflags is 0, skip the tracing section, and
	 * do the system call.
	 * There is at least one syscall that the user wants traced on entry,
	 * see if this is the one.
	 *
	 * If prismember(sp, flag) returns true, then
	 * set the task pr_flags field to PR_STOPPED | PR_ISTOP, set pr_why to
	 * PR_SYSENTRY, and pr_what to the current syscall number;
	 * wakeup any process that may be sleeping on us; and call
	 * thread_suspend() to stop.  On return from the debugger, if
	 * PRFS_ABORT is set and we are we are not tracing this system call
	 * on exit, abort - i.e. return EINTR before actually doing the
	 * system call.
	 */
 if(t->task->procfs.pr_qflags & PRFS_TRACING) {
	task = current_task();
	if ((PRFS_SCENTER & task->procfs.pr_qflags) ||
		(PRFS_SCENTER & t->t_procfs.pr_qflags)) {
		if(prismember(task->procfs.pr_sysenter.word, code)) {
			task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			task->procfs.pr_why = PR_SYSENTRY;
			task->procfs.pr_what = code;
			task->procfs.pr_tid = t;
			task->suspend_count++;
			task->user_stop_count++;
			wakeup(&(task->procfs));
			/* task_suspend(task); */
			thread_block(t);
			if(((task->procfs.pr_qflags & PRFS_ABORT) != 0) &&
				(!prismember(task->procfs.pr_sysexit.word, code))) {
				return(EINTR);
			}
			else if((task->procfs.pr_qflags & PRFS_ABORT) == 0)
			{ /* not aborted */
#if SEC_BASE
		           if (security_is_on) {
		                AUDIT_SETUP(callp - sysent, habitat);
		                error = (*callp->sy_call)(u.u_procp,&args,rval);
		                AUDIT_GENERATE_RECORD(&args, error, rval);
		           } else
#endif
				error = (*callp->sy_call)(u.u_procp,&args,rval);
			}
		}
		else if(prismember(t->t_procfs.pr_sysexit.word, code)) {
			t->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			t->t_procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			t->t_procfs.pr_why = PR_SYSENTRY;
			t->t_procfs.pr_what = code;
			task->procfs.pr_tid = t;
			wakeup(&(task->procfs));
			thread_suspend(t);
			thread_block(t);
			if((t->t_procfs.pr_qflags & PRFS_ABORT) != 0 &&
				!prismember(t->t_procfs.pr_sysexit.word,code)) {
				return(EINTR);
			}
			else if( (t->t_procfs.pr_qflags & PRFS_ABORT) != 0)
			{ /* not aborted */
#if SEC_BASE
			   if (security_is_on) {
				AUDIT_SETUP(callp - sysent, habitat);
				error = (*callp->sy_call)(u.u_procp,&args,rval);
				AUDIT_GENERATE_RECORD(&args, error, rval);
			   } else
#endif
				error = (*callp->sy_call)(u.u_procp,&args,rval);
			}
		}
		else { /* Not the one traced */
#if SEC_BASE
		        if (security_is_on) {
				AUDIT_SETUP(callp - sysent, habitat);
				error = (*callp->sy_call)(u.u_procp,&args,rval);
				AUDIT_GENERATE_RECORD(&args, error, rval);
		        } else
#endif
			error = (*callp->sy_call)(u.u_procp, &args, rval);
		}
	 }
	 else /* No calls traced */
	/* The actual system call is made here. */
#if SEC_BASE
		if (security_is_on) {
			AUDIT_SETUP(callp - sysent, habitat);
			error = (*callp->sy_call)(u.u_procp, &args, rval);
			AUDIT_GENERATE_RECORD(&args, error, rval);
		} else
#endif
		error = (*callp->sy_call)(u.u_procp, &args, rval);

#if BIN_COMPAT
	/* Display errors from syscalls */
	if(bin_compat_debug && error && cm_ptr)
		printf("(%s %d) syscall: %s{%d}, error %d, rval %d\n", 
		u.u_comm, u.u_procp->p_pid, cm_ptr->call_name[code], code,
		error, rval[0]);
#endif BIN_COMPAT

	/*
	 * If PRFS_SCEXIT in pr_qflags is 0, skip the tracing section.
	 * There is at least one syscall that the user wants traced on exit,
	 * see if this is the one.
	 *
	 * If prismember(task->procfs.pr_sysexit, code) returns true, then
	 * set the task pr_flags field to PR_STOPPED | PR_ISTOP, set pr_why to
	 * PR_SYSEEXIT, and pr_what to the current syscall number;
	 * wakeup any process that may be sleeping on us and call task_suspend()
	 * to stop.
	 */
	if ((PRFS_SCEXIT & task->procfs.pr_qflags) ||
		(PRFS_SCEXIT & t->t_procfs.pr_qflags)) {
		if(prismember(task->procfs.pr_sysexit.word, code)) {
			task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			task->procfs.pr_qflags &= ~(PRFS_ABORT);
			task->procfs.pr_why = PR_SYSEXIT;
			task->procfs.pr_what = code;
			task->procfs.pr_tid = t;
			task->suspend_count++;
			task->user_stop_count++;
			wakeup(&(task->procfs));
			/* task_suspend(task); */
			thread_block(t);
		}
		if(prismember(t->t_procfs.pr_sysexit.word, code)) {
			t->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			t->t_procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			t->t_procfs.pr_qflags &= ~(PRFS_ABORT);
			t->t_procfs.pr_why = PR_SYSEXIT;
			t->t_procfs.pr_what = code;
			task->procfs.pr_tid = t;
			wakeup(&(t->task->procfs));
			thread_suspend(t);
			thread_block(t);
		}
	}
	/* end of /proc code */
 }
 else {
	/* The actual system call is made here. */
#if SEC_BASE
		if (security_is_on) {
			AUDIT_SETUP(callp - sysent, habitat);
			error = (*callp->sy_call)(u.u_procp, &args, rval);
			AUDIT_GENERATE_RECORD(&args, error, rval);
		} else
#endif
		error = (*callp->sy_call)(u.u_procp, &args, rval);

#if BIN_COMPAT
	/* Display errors from syscalls */
	if(bin_compat_debug && error && cm_ptr)
		printf("(%s %d) syscall: %s{%d}, error %d, rval %d\n", 
		u.u_comm, u.u_procp->p_pid, cm_ptr->call_name[code], code,
		error, rval[0]);
#endif BIN_COMPAT

 }

#if BIN_COMPAT
	if ( ! cm_ptr)
#endif /*BIN_COMPAT*/
	if (code <= scmax)
		sccount[code - FIRST]++;     	/* count system calls */

	/*
	 * a3 is returned to user 0 if indicate no errors on syscall,
	 * non-zero otherwise
	 */
out:
	if (error == ERESTART) {
		ep[EF_EPC] = opc;
	}
	else if (error != EJUSTRETURN) {
		if (error) {
#ifdef	SYSCALLTRACE
			if (syscalltracing)
				printf(" => error=%d\n", error);
#endif
			ep[EF_V0] = error;
#if SEC_BASE
			if (callp == &sysent[AUDIT_SECURITY])
				ep[EF_V0] |= (AIP->si_error << SEC_ERRNO_SHIFT);
#endif
			ep[EF_A3] = 1;
		} else {
#ifdef	SYSCALLTRACE
			if (syscalltracing)
				printf(" => r0 = %x, r1 = %x\n",
					rval[0], rval[1]);
#endif
			ep[EF_V0] = rval[0];
			ep[EF_V1] = rval[1];
			ep[EF_A3] = 0;
		}
#ifdef	SYSCALLTRACE
		if (syscalltracing) {
			syscalltracing = 0;
			if (syscallhalt++ > syscallstop) {
				printf("%d syscalls.  syscalltrace@%x\n",
					syscallstop, &syscalltrace);
				gimmeabreak();
				syscallhalt = 0;
			}
		}
#endif
	}
	/* else if (error == EJUSTRETURN) */
		/* nothing to do */
done:
#if BIN_COMPAT
	if(cm_ptr) 
	    XPR(XPR_SYSCALLS,
		("syscall end: pid=%d, module=%s, code=%d, rval1=%d, rval2=%d\n"
		, u.u_procp->p_pid, cm_ptr->cm_name, code, 
		rval[0], rval[1]));
	else
#endif /*BIN_COMPAT*/
	XPR(XPR_SYSCALLS,
		("syscall end: pid=%d, code=%d, rval1=%d, rval2=%d\n",
		u.u_procp->p_pid, code, rval[0], rval[1]));

	{	
		register struct proc *p = utask->uu_procp;

		if (CHECK_SIGNALS(p, t, uthread)) {
			unix_master();
			if (p->p_cursig || issig())
				psig();
			unix_release();
		}
	}

#if	RT_PREEMPT
	if (t->unix_lock >= 0)
		unix_release();
#endif

#define SLOWER
#ifdef	SLOWER
	if (csw_needed(t, current_processor())) {
		utask->uu_ru.ru_nivcsw++;
#if	RT_PREEMPT && RT_PREEMPT_DEBUG
		rt_preempt_syscall++;
		if (ast_needed(cpu_number()))
		  rt_preempt_ast_set++;
#endif	/* RT_PREEMPT && RT_PREEMPT_DEBUG */
		thread_block();
	}
#endif	/* SLOWER */

	/*
	 * if single stepping this process, install breakpoints before
	 * returning to user mode.  Do this here rather than in trap()
	 * so single stepping will work when signals are delivered.
	 */
	if (t->pcb->pcb_sstep == PT_STEP)
		install_bp();

 	if (utask->uu_prof.pr_scale > 1) {
		int ticks;
		struct timeval *tv = &utask->uu_ru.ru_stime;

		/* Beware tick roundoff - hz is 64, 128, 256, etc */
		ticks = ((tv->tv_sec - syst.tv_sec) * hz) +
			(((tv->tv_usec - syst.tv_usec) * hz) / (1000*1000));
		if (ticks > 0) {
			simple_lock(&utask->uu_prof.pr_lock);
			addupc(ep[EF_EPC], &utask->uu_prof, ticks);
			simple_unlock(&utask->uu_prof.pr_lock);
		}
	}

#if	RT_PREEMPT
	if (t->unix_lock >= 0)
		unix_release_force();
#endif

	/*
	 * if error == EJUSTRETURN, then force full state restore
	 */
	return (error == EJUSTRETURN);
}


/*
 * nonexistent system call-- signal process (may want to handle it)
 * flag error if process won't see signal immediately
 * Q: should we do that all the time ??
 */
nosys(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{
	int error = 0;
	if (signal_disposition(SIGSYS) == SIG_IGN ||
	    signal_disposition(SIGSYS) == SIG_HOLD)
		error = EINVAL;
	thread_doexception(current_thread(), EXC_SOFTWARE,
		EXC_UNIX_BAD_SYSCALL, 0);
	return (error);
}

