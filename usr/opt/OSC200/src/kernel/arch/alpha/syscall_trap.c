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
static char *rcsid = "@(#)$RCSfile: syscall_trap.c,v $ $Revision: 1.1.16.9 $ (DEC) $Date: 1993/11/15 15:21:04 $";
#endif
#include "bin_compat.h"
#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#include <machine/cpu.h>
#include <mach/exception.h>       /* Machine-indep. exception defs */
#include <mach/time_value.h>
#include <builtin/ux_exception.h> /* Unix sw exceptions under EXC_SOFTWARE */
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/habitat.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <kern/xpr.h>
#include <kern/parallel.h>      /* unix_master, unix_release */
#include <mach/exception.h>
#include <mach/alpha/thread_status.h>
#ifdef ASSERTIONS
#include <builtin/ux_exception.h>
#endif
#include <machine/psl.h>
#include <machine/reg.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/param.h>
#include <kern/xpr.h>
#include <kern/parallel.h>
#include <sys/syscall.h>
#if     RT_PREEMPT
#include <kern/ast.h>
#include <sys/preempt.h>
#if     RT_PREEMPT_DEBUG
#include <mach/time_value.h>
#include <mach/boolean.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/queue.h>
#endif  /* RT_PREEMPT */
#endif  /* RT_PREEMPT_DEBUG */

#ifdef	SYSCALLTRACE
int syscalltracing = 0;
int syscallhalt = 0;
int syscallstop = 50;
#endif
long syscalltrace = 0;

int scmax = LAST;               /* max number of system call counts */
int sccount[LAST - FIRST +1];           /* system call counts */

int nosys();				/* forward declaration */

#if     RT_PREEMPT && RT_PREEMPT_DEBUG
extern int      rt_preempt_syscall;
extern int      rt_preempt_user;
extern int      rt_preempt_async;
extern int      rt_preempt_ast_set;
extern int      rt_preempt_pp;
#endif /* RT_PREEMPT && RT_PREEMPT_DEBUG */

extern lock_data_t profil_lock;

#ifdef KTRACE
int ktpid=0;		/* force it into initialized data	*/
#endif /* KTRACE */

/*
 * system calls
 */
syscall(ep, code)
	u_long  *ep;                    /* pointer to the exception frame */
	u_long   code;                  /* system call number (0 for indir) */
{
	time_value_t		stime_enter, stime_exit, utime;
	int			stime_enter_valid;
	register int		 error;
	register struct utask	*utask;
	register struct task	*task;
	register struct uthread	*uthread;
	register thread_t	 t;
        register struct sysent	*callp;

	struct args {
		long i[NUMSYSCALLARGS];
	} args;
	long rval[2];
	int indir;
	extern char *syscallnames[];
#if BIN_COMPAT
	/*
	 *  Leave these initializations in; they are negligible.
	 *  The maintenance cost of making sure they get initialized
	 *  somewhere else before being used is a greater headache.
	 */
        unsigned long   habitat = 0L;
        struct compat_mod *cm_ptr = (struct compat_mod *)0;
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

 	if (u.u_prof.pr_scale > 1) {
		thread_read_times(t, &utime, &stime_enter);
		stime_enter_valid = 1;
	}
	else
		stime_enter_valid = 0;

	error = 0;

	if (indir = (code == 0)) {
		/*
		 * indirect system call (syscall), first param is
		 * sys call number, others will get shifted down
		 */
		code = ep[EF_A0];
	}

#if BIN_COMPAT_not
	/* We've got to get rid of compatibility and keep habitats. */
	if (cm_ptr = u.u_compat_mod) {
		if (!(callp = (* cm_ptr->cm_syscall)(code))) {
			callp = &sysent[0];
		}
	} else
#endif	/* BIN_COMPAT_not */
	/* case for syscall without a habitat set */
	if (code < nsysent)
		callp = &sysent[code];
#if BIN_COMPAT
	/* case for syscall with a habitat set */
	else if ((habitat = hbval(code))
		 && ((code &= ~(u_long)HABITAT_MASK),
		     (habitat < MAXHABITATS))
		 && (cm_ptr = habitats[habitat])
		 && (callp = (* cm_ptr->cm_syscall)(code)))
		;	/* if callp is now set, nothing left to do  */
#endif	/* BIN_COMPAT */
	else {
		callp = &sysent[0];	/* indir (illegal) */
	}

	/*
	 * User's arguments aren't contiguous on the stack,
	 * new calling sequence has 6 argument registers
	 * and more would be on the user's stack.
	 */
	if (!indir) {
		switch (callp->sy_narg) {
		case 6:
			args.i[5] = ep[EF_A5];
		case 5:
			args.i[4] = ep[EF_A4];
		case 4:
			args.i[3] = ep[EF_A3];
		case 3:
			args.i[2] = ep[EF_A2];
		case 2:
			args.i[1] = ep[EF_A1];
		case 1:
			args.i[0] = ep[EF_A0];
		case 0:
			break;
		default:
			panic("syscall: unexpected syscall arguments");
			break;
		}
	} else {
		switch (callp->sy_narg) {
		case 6:
			args.i[5] = fuqword(mfpr_usp());
		case 5:
			args.i[4] = ep[EF_A5];
		case 4:
			args.i[3] = ep[EF_A4];
		case 3:
			args.i[2] = ep[EF_A3];
		case 2:
			args.i[1] = ep[EF_A2];
		case 1:
			args.i[0] = ep[EF_A1];
		case 0:
			break;
		default:
			panic("syscall: unexpected indir syscall arguments");
			break;
		}
	}

	rval[0] = 0;
	rval[1] = 0;

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
        if (uthread->uu_nd.ni_cred != utask->uu_procp->p_rcred)
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
		printf("[P%ld %s] %s{%ld}", u_procp->p_pid,
			u.u_comm, syscallnames[code], code);
		cp = "(";
		for (i= 0; i < callp->sy_narg && i < 6; i++) {
			printf("%s0x%16lx", cp, args.i[i]);
			cp = ", ";
		}
		if (i) printf(")");
	}
#endif

	XPR(XPR_SYSCALLS, ("syscall start: %c%c%c%c%c%c\n",
		u.u_comm[0], u.u_comm[1], u.u_comm[2],
		u.u_comm[3], u.u_comm[4], u.u_comm[5]));
#if BIN_COMPAT
        if (cm_ptr)
        XPR(XPR_SYSCALLS, ("syscall start %s.%ld: args=0x%16lx, 0x%16lx, 0x%16lx, 0x%16lx\n",
                cm_ptr->cm_name, code, args.i[0], args.i[1], args.i[2],
                args.i[3], args.i[4]));
        else
#endif /*BIN_COMPAT*/
	XPR(XPR_SYSCALLS, ("syscall start %ld: args=0x%16lx, 0x%16lx, 0x%16lx, 0x%16lx, 0x%16lx\n",
		code, args.i[0], args.i[1], args.i[2],
		args.i[3], args.i[4]));


	/*
	 * The proc struct is the wrong place for this flag because multiple
	 * threads can be executing syscalls at different/overlapping times. 
	 * However, at this point in the cycle (close to final product) we
	 * could not afford to add a flag field to the uthread struct. 
	 * At a point in the future we must really address debugging a
	 * multi-threaded application (ptrace is the reason for this flag).
	 */
	u.u_procp->p_flag |= SSYSCALL;

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
	long ncode;					/*gets code | habitat */
	task = current_task();
	/* ncode is set in this manner, because "habitat" was not always
	 * initialized (to speedup syscall performance), and there is no
	 * guarantee that its initialization will never be removed.
	 */
#ifdef BIN_COMPAT
	if(cm_ptr)
		ncode = (code | (habitat << HABITAT_SHIFT));
	else
#endif
		ncode = code;

	if ((PRFS_SCENTER & task->procfs.pr_qflags) &&
	   prismember(&task->procfs.pr_sysenter, code))
	{
		unix_master();
		task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
		task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
		task->procfs.pr_why = PR_SYSENTRY;
		task->procfs.pr_what = ncode;
		task->procfs.pr_tid = t;
		wakeup(&(task->procfs));
		task_suspend(task);
		unix_release();

		if(((task->procfs.pr_qflags & PRFS_ABORT) != 0) &&
			(!prismember(&task->procfs.pr_sysexit, code))) {
			task->procfs.pr_qflags &= ~(PRFS_ABORT);
			u.u_procp->p_flag &= ~SSYSCALL;
			return(EINTR);
		}
		else if((task->procfs.pr_qflags & PRFS_ABORT) == 0)
		{ /* not aborted */
			if ( audswitch == 0 )
				error = (*callp->sy_call)(u.u_procp, &args, rval);
			else {
				u.u_vno_indx = 0;
				if ( callp != &sysent[0] ) {
#if BIN_COMPAT
					u.u_event = code | (habitat << HABITAT_SHIFT);
#else
					u.u_event = code;
#endif /* BIN_COMPAT */
				}
				else u.u_event = 0;
				u.u_set_uids_snap = u.u_set_uids;
				error = (*callp->sy_call)(u.u_procp, &args, rval);
				if ( callp->aud_param[NUMSYSCALLARGS] < SELF_AUDIT )
					AUDIT_CALL ( u.u_event, error, &args, rval[0], AUD_VHPR, (char *)0 );
			}
		}
	}
	else if( (PRFS_SCENTER & t->t_procfs.pr_qflags) &&
	        prismember(&t->t_procfs.pr_sysenter, code))
	{
		t->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
		t->t_procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
		t->t_procfs.pr_why = PR_SYSENTRY;
		t->t_procfs.pr_what = ncode;
		task->procfs.pr_tid = t;
		wakeup(&(task->procfs));
		thread_suspend(t);
		thread_block();
		if((t->t_procfs.pr_qflags & PRFS_ABORT) != 0 &&
			!prismember(&t->t_procfs.pr_sysexit,code)) {
			t->t_procfs.pr_qflags &= ~(PRFS_ABORT);
			u.u_procp->p_flag &= ~SSYSCALL;
			return(EINTR);
		}
		else if( (t->t_procfs.pr_qflags & PRFS_ABORT) != 0)
		{ /* not aborted */
			if ( audswitch == 0 )
				error = (*callp->sy_call)(u.u_procp, &args, rval);
			else {
				u.u_vno_indx = 0;
				if ( callp != &sysent[0] ) {
#if BIN_COMPAT
					u.u_event = code | (habitat << HABITAT_SHIFT);
#else
					u.u_event = code;
#endif /* BIN_COMPAT */
				}
				else u.u_event = 0;
				u.u_set_uids_snap = u.u_set_uids;
				error = (*callp->sy_call)(u.u_procp, &args, rval);
				if ( callp->aud_param[NUMSYSCALLARGS] < SELF_AUDIT )
					AUDIT_CALL ( u.u_event, error, &args, rval[0], AUD_VHPR, (char *)0 );
			}
		}
	}
	else /* Not the one traced */
	{
		if ( audswitch == 0 )
			error = (*callp->sy_call)(u.u_procp, &args, rval);
		else {
			u.u_vno_indx = 0;
			if ( callp != &sysent[0] ) {
#if BIN_COMPAT
				u.u_event = code | (habitat << HABITAT_SHIFT);
#else
				u.u_event = code;
#endif /* BIN_COMPAT */
			}
			else u.u_event = 0;
			u.u_set_uids_snap = u.u_set_uids;
			error = (*callp->sy_call)(u.u_procp, &args, rval);
			if ( callp->aud_param[NUMSYSCALLARGS] < SELF_AUDIT )
				AUDIT_CALL ( u.u_event, error, &args, rval[0], AUD_VHPR, (char *)0 );
		}
	}

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

	if ( (PR_DSTOP & task->procfs.pr_flags) || 
		(PRFS_SCEXIT & task->procfs.pr_qflags) ||
		(PRFS_SCEXIT & t->t_procfs.pr_qflags)) {
		if(error)
		{
			ep[EF_V0] = error;
			ep[EF_A3] = 1;
		}
		else
		{
			ep[EF_V0] = rval[0];
			ep[EF_A4] = rval[1];
			ep[EF_A3] = 0;
		}
		if ( (PR_DSTOP & task->procfs.pr_flags) )
		{
			unix_master();
			task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			task->procfs.pr_why = PR_REQUESTED;
			task->procfs.pr_what = ncode;
			task->procfs.pr_tid = t;
			wakeup(&(task->procfs));
			task_suspend(task);
			unix_release();
			/* if PRFS_ABORT is set, skip setting the user regs,
			 * this allows the user to "force" a return value
			 */
			if(task->procfs.pr_qflags & PRFS_ABORT)
			{
			    task->procfs.pr_qflags &= ~(PRFS_ABORT);
			    goto done;
			}

		}
		else if(prismember(&task->procfs.pr_sysexit, code)) {
			unix_master();
			task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			task->procfs.pr_why = PR_SYSEXIT;
			task->procfs.pr_what = ncode;
			task->procfs.pr_tid = t;
			wakeup(&(task->procfs));
			task_suspend(task);
			unix_release();
			/* if PRFS_ABORT is set, skip setting the user regs,
			 * this allows the user to "force" a return value
			 */
			if(task->procfs.pr_qflags & PRFS_ABORT)
			{
			    task->procfs.pr_qflags &= ~(PRFS_ABORT);
			    goto done;
			}
		}
		else if(prismember(&t->t_procfs.pr_sysexit, code)) {
			t->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			t->t_procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			t->t_procfs.pr_why = PR_SYSEXIT;
			t->t_procfs.pr_what = ncode;
			task->procfs.pr_tid = t;
			wakeup(&(t->task->procfs));
			thread_suspend(t);
			thread_block();
			/* if PRFS_ABORT is set, skip setting the user regs,
			 * this allows the user to "force" a return value
			 */
			if(t->t_procfs.pr_qflags & PRFS_ABORT)
			{
			    t->t_procfs.pr_qflags &= ~(PRFS_ABORT);
			    goto done;
			}
		}
	}
	/* end of /proc code */
 }
 else {
	/* The actual system call is made here. */
	if ( audswitch == 0 )
		error = (*callp->sy_call)(u.u_procp, &args, rval);
	else {
		u.u_vno_indx = 0;
		if ( callp != &sysent[0] ) {
#if BIN_COMPAT
			u.u_event = code | (habitat << HABITAT_SHIFT);
#else
			u.u_event = code;
#endif /* BIN_COMPAT */
		}
		else u.u_event = 0;
		u.u_set_uids_snap = u.u_set_uids;
		error = (*callp->sy_call)(u.u_procp, &args, rval);
		if ( callp->aud_param[NUMSYSCALLARGS] < SELF_AUDIT )
			AUDIT_CALL ( u.u_event, error, &args, rval[0], AUD_VHPR, (char *)0 );
	}

}

#if BIN_COMPAT
        if ( ! cm_ptr)
#endif /*BIN_COMPAT*/
        if (code <= scmax)
                sccount[code - FIRST]++;        /* count system calls */

	/*
	 * a3 is returned to user -- 0 to indicate no errors on syscall,
	 * non-zero otherwise.
	 */
out:
	if (error == ERESTART) {
		ep[EF_PC] -= 4;
	}
	else if (error != EJUSTRETURN) {
		if (error) {
#ifdef	SYSCALLTRACE
			if (syscalltracing)
				printf(" => error=%d\n", error);
#endif
			ep[EF_V0] = error;
			ep[EF_A3] = 1;
		} else {
#ifdef	SYSCALLTRACE
			if (syscalltracing)
				printf(" => r0 = 0x%lx, r1 = 0x%lx\n",
					rval[0], rval[1]);
#endif
			ep[EF_V0] = rval[0];
			ep[EF_A4] = rval[1];
			ep[EF_A3] = 0;
		}
#ifdef	SYSCALLTRACE
		if (syscalltracing) {
			syscalltracing = 0;
			if (syscallhalt++ > syscallstop) {
				printf("%d syscalls.  syscalltrace@0x%16lx\n",
					syscallstop, &syscalltrace);
#ifdef alpha_notyet
				gimmeabreak();
#endif /* alpha_notyet */
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
                ("syscall end: pid=%ld, module=%s, code=%ld, rval1=%ld, rval2=%ld\n"
                , u.u_procp->p_pid, cm_ptr->cm_name, code,
                rval[0], rval[1]));
        else
#endif /*BIN_COMPAT*/
	XPR(XPR_SYSCALLS,
		("syscall end: pid=%ld, code=%ld, rval1=%ld, rval2=%ld\n",
		u.u_procp->p_pid, code, rval[0], rval[1]));

	{	
		register struct proc *p = utask->uu_procp;

		/*
		 * Indicate system call processing done.  If the process
		 * is being traced and marked for exit, issig() will
		 * terminate the process.
		 */
		p->p_flag &= ~SSYSCALL;
		if ((unsigned long)p->p_cursig > NSIG) {
			exit(p, p->p_cursig - NSIG);
		}
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
#if	RT_PREEMPT && RT_PREEMPT_DEBUG
		rt_preempt_syscall++;
		if (ast_needed(cpu_number()))
		  rt_preempt_ast_set++;
#endif
		thread_block();
	}
#endif

	/*
	 * if single stepping this process, install breakpoints before
	 * returning to user mode.  Do this here rather than in trap()
	 * so single stepping will work when signals are delivered.
	 */
	if (t->pcb->pcb_sstep == PT_STEP)
		install_bp();

 	if (stime_enter_valid) {

		int ticks, secs, usecs;
	
		thread_read_times(t, &utime, &stime_exit);
	
		secs = stime_exit.seconds - stime_enter.seconds;
		usecs = stime_exit.microseconds - stime_enter.microseconds;

		ticks = (secs * hz) + ((usecs * hz) / (1000*1000));

		if (ticks > 0) {
			lock_write(&profil_lock);
			if (u.u_prof.pr_scale > 1) 
				addupc(ep[EF_PC], &u.u_prof, ticks);
			lock_done(&profil_lock);
		}
	}

#if	RT_PREEMPT
	if (t->unix_lock >= 0)
		unix_release_force();

	/*
	 * If the slock_count has not returned to 0, coerce it there
	 * and then check whether the anomaly should crash the system.
	 */
	if (slock_count[cpu_number()] != 0)
		PREEMPTION_FIXUP();
#endif

#ifdef KTRACE
	/*
	 * This hack allows syscall tracing without using the trace file
	 * stuff.  Set ktpid with the pid you want to trace, it and all
	 * it's children will be traced.
	 */
	if( ktpid && (p->p_pid == ktpid || p->p_ppid == ktpid) ){
		int i;
		printf("pid=%ld <%s> %s ",
			p->p_pid, u.u_comm, syscallnames[code]);
		for( i=0 ; i< callp->sy_narg ; i++ )
			printf("0x%16lx ", u.u_arg[i]);
		printf("retv=0x%lx pc=0x%16lx ra=0x%16lx\n",
			error ? error : rval[0],
			ep[EF_PC], ep[EF_RA]);
	}
#endif /* KTRACE */

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
	long *retval;
{
	int error = 0;
	struct alpha_saved_state       *saved_state;

 
	/*
	 * fetch the syscall number out of the exception frame.
         */ 
	saved_state  = USER_REGS(current_thread());
	if (saved_state->r0 == 0) { 
		/* indirect call... code is in a0  */
		u.u_code = saved_state->r16;
	} else { 
		/* direct system call.... code in V0 */
		u.u_code = saved_state->r0;
	} 
	if (signal_disposition(SIGSYS) == SIG_IGN ||
	    signal_disposition(SIGSYS) == SIG_HOLD)
		error = EINVAL;
	thread_doexception(current_thread(), EXC_SOFTWARE,
		EXC_UNIX_BAD_SYSCALL, 0);
	return (error);
}
