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
static char *rcsid = "@(#)$RCSfile: kern_sig.c,v $ $Revision: 4.5.18.19 $ (DEC) $Date: 1994/01/20 16:24:05 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *
 *	Revision History:
 *
 * 31-Oct-91	Brian Stevens
 *	Modified core() to put the offending thread id into the core file
 *	header.
 *
 * 30-Oct-91	Brian Stevens
 *	Fixed core() to write the registers of each thread to the core file.
 *
 * 25-Jun-91	Peter H. Smith
 *	Incorporate OSF/1.0.1 changes:
 *	 - Remove SWITCH_ANSI conditional.  Always cast to int.
 *	 - In psignal(), take thread_list_lock at splsched, not splhigh.
 *	 - In issig(), take thread_list_lock at splsched, not splhigh.
 *	 - Make coreprint default to 0, not 1.
 *
 * 03-May-91	Peter H. Smith
 *	Modify psignal so that it does not try to change the priority of a
 *	fixed priority process.  Also, get rid of a hardcoded priority.
 */

#include "bin_compat.h"

#include <cputypes.h>
#include <mach_host.h>
#include <mach_host.h>
#include <sys/secdefines.h>
#include <rt_sched.h>
#include <rt_sched_rq.h>

#include <machine/reg.h>
#if	defined(vax) || defined(sun) || defined(i386)
#include <machine/psl.h>
#endif

#include <sys/param.h>
#include <sys/sched_mon.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <machine/vmparam.h>	/* Get USRSTACK */
#include <sys/acct.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/kernel.h>
#include <kern/sched.h>
#include <mach/vm_param.h>
#include <kern/thread.h>
#include <kern/parallel.h>
#include <machine/cpu.h>
#include <kern/sched_prim.h>
#include <kern/task.h>
#include <kern/queue.h>
#include <vm/vm_kern.h>
#if     SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif


#ifdef	multimax
#include <mmax/mmax_ptrace.h>
#endif

#include <sys/core.h>

#include <sys/siginfo.h>
#include <sys/uswitch.h>
#include <sys/habitat.h>

#define stopsigmask	(sigmask(SIGSTOP)|sigmask(SIGTSTP)| \
			sigmask(SIGTTIN)|sigmask(SIGTTOU))
#define defaultignmask	(sigmask(SIGCONT)|sigmask(SIGIO)|sigmask(SIGURG)| \
			sigmask(SIGCHLD)|sigmask(SIGWINCH)|sigmask(SIGINFO))
/*
 * Generalized interface signal handler.
 */

/*
 * Can process p send the signal signo to process q?
 */
#define	CANSIGNAL(p, q, signo)	cansignal(p, q, signo)

sigaction(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	signo;
		struct	sigaction *nsa;
		struct	sigaction *osa;
#if	defined(balance) || defined(mips) || defined(__alpha)
		int	(*sigtramp)();		/* signal trampoline code */
#endif
	} *uap = (struct args *) args;
	struct sigaction vec;
	register struct sigaction *sa;
	register int sig;
	int error;
	sigset_t bit;
#ifdef	i386
	u.u_sigreturn = (int (*)())u.u_ar0[EDX];
#endif

	sig = uap->signo;
	ASSERT(syscall_on_master());
#ifdef	multimax
	/*
	 * On the MMax the u area is not visible to the user tasks,
	 * therefore, the task must declare the address of its
	 * trampoline code before it attempts to catch signals.
	 * This is done by using the special signal SIGCATCHALL
	 */
	/* If this is the intermediate signal catcher then set it. */
	if (sig == SIGCATCHALL) {

		/* Copy the uap->nsa structure from user space to system
		 *	space.  Then set the return value and the signal
		 *	catcher.
		 */
		if (error = copyin ((caddr_t)uap->nsa, (caddr_t)&vec,
		    sizeof(vec)))
			return (error);
		*retval = (long)u.u_sigcatch;
		u.u_sigcatch = (void (*)) vec.sa_handler;
		return (0);
	}
#endif
	if (sig <= 0 || sig > NSIG)
		return (EINVAL);
	sa = &vec;
	if (uap->osa) {
		sa->sa_handler = signal_disposition(sig);
		sa->sa_mask = u.u_sigmask[sig];
		bit = sigmask(sig);
		sa->sa_flags = 0;
		if ((u.u_sigonstack & bit) != (sigset_t)0)
			sa->sa_flags |= SA_ONSTACK;
		if ((u.u_sigintr & bit) == (sigset_t)0)
			sa->sa_flags |= SA_RESTART;
		if ((p->p_siginfo & bit) != (sigset_t)0)
			sa->sa_flags |= SA_SIGINFO;
		if ((u.u_sigresethand & bit) != 0) 
		  sa->sa_flags |= SA_RESETHAND;
		if ((u.u_signodefer & bit) != 0)
		  sa->sa_flags |= SA_NODEFER;
		
		if (sig == SIGCHLD) {
			if (p->p_flag & SNOCLDSTOP)
				sa->sa_flags |= SA_NOCLDSTOP;
			if (p->p_flag & SNOCLDWAIT)
				sa->sa_flags |= SA_NOCLDWAIT;
		}
		if (error = copyout((caddr_t)sa, (caddr_t)uap->osa,
		    sizeof (vec)))
			return (error);
	}
	if (uap->nsa) {
		if (error = copyin((caddr_t)uap->nsa, (caddr_t)sa,
		    sizeof (vec)))
			return (error);
		if (sa->sa_handler != SIG_DFL && (sig == SIGKILL || sig == SIGSTOP))
			return(EINVAL);

#if	defined(mips) || defined(__alpha)
		/*
		 * check for unaligned pc on sighandler
		 */
		if (sa->sa_handler != SIG_IGN
		    && ((int)sa->sa_handler & (sizeof(int)-1))) {
			return (EINVAL);
		}
#endif /* mips or __alpha */
		setsigvec(p, sig, sa);
#if	defined(balance) || defined(mips) || defined(__alpha)
		/*
		 * On the Sequent Balance and Mips, struct user isn't
		 * visible to the executing thread; thus the trampoline
		 * code pointer is explicitly passed in when setting
		 * a signal handler.
		 */
		u.u_sigtramp = uap->sigtramp;
#endif
	}
	return (0);
}

/*
 * ssig is the target of the System V signal() system call, preserved
 * here for binary compatibility.
 *
 * Instead of setting obsolete P_SOUSIG and forcing old SV behavior for
 * all handlers from this point on without recourse by the user process,
 * set sigresethand and signodefer for the signal specified in this
 * invocation.  Only the signal specified is set to use old SV
 * behavior.  Signal handlers set up through the sigaction() system
 * call will get sigaction()/POSIX.1 behavior.
 */
ssig(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	signo;
		int	(*fun)();
#if	defined(balance) || defined(mips) || defined(__alpha)
		int	(*sigtramp)();		/* signal trampoline code */
#endif
	} *uap = (struct args *) args;
	register int a;
	sigset_t mask;
	struct sigaction act;
	register struct sigaction *sa = &act;
#ifdef	i386
	u.u_sigreturn = (int (*)())u.u_ar0[EDX];
#endif

	a = uap->signo;
	mask = sigmask(a);
	sa->sa_handler = (void (*)())uap->fun;

	if (a <= 0 || a > NSIG || 
	    (sa->sa_handler != SIG_DFL && (a == SIGKILL || a == SIGSTOP)) ||
	    (a == SIGCONT && sa->sa_handler == SIG_IGN))
		return (EINVAL);
	sa->sa_mask = 0;
	sa->sa_flags = 0;
	*retval = (long)signal_disposition(a);

	if (a == SIGCHLD) {
		if (sa->sa_handler == SIG_IGN) {
			/*
			 * If SIGCHLD is set to	SIG_IGN, then
			 * no zombies are created, and the parent
			 * can't wait on children.
			 */
			sa->sa_flags |= SA_NOCLDWAIT;
		}
		sa->sa_flags |= SA_NOCLDSTOP;
	}

	setsigvec(p, a, sa);

	/*
	 * Request that the handler be reset to SIG_DFL and that the
	 * current signal not be blocked while in the handler. Setting
	 * both of these flags provides the old System V signal
	 * behavior.
	 */
	u.u_sigresethand |= mask;
	u.u_signodefer |= mask;

#if	defined(balance) || defined(mips) || defined(__alpha)
		/*
		 * On the Sequent Balance and Mips, struct user isn't
		 * visible to the executing thread; thus the trampoline
		 * code pointer is explicitly passed in when setting
		 * a signal handler.
		 */
		u.u_sigtramp = uap->sigtramp;
#endif
	return (0);
}

setsigvec(p, sig, sa)
	register struct proc *p;
	long sig;
	register struct sigaction *sa;
{
	register sigset_t bit;

	ASSERT(syscall_on_master());
	bit = sigmask(sig);
	/*
	 * Change setting atomically.
	 */
	(void) splhigh();
	sig_lock_simple(p);
	signal_disposition(sig) = sa->sa_handler;
	u.u_sigmask[sig] = sa->sa_mask &~ sigcantmask;

	if ((sa->sa_flags & SA_RESTART) == 0)
		u.u_sigintr |= bit;
	else
		u.u_sigintr &= ~bit;
	if (sa->sa_flags & SA_ONSTACK)
		u.u_sigonstack |= bit;
	else
		u.u_sigonstack &= ~bit;
	if (sa->sa_flags & SA_RESETHAND)
		u.u_sigresethand |= bit;
	else
		u.u_sigresethand &= ~bit;
	if (sa->sa_flags & SA_NODEFER)
		u.u_signodefer |= bit;
	else
		u.u_signodefer &= ~bit;

	if (sig == SIGCHLD) {
		if (sa->sa_flags & SA_NOCLDSTOP)
			p->p_flag |= SNOCLDSTOP;
		else
			p->p_flag &= ~SNOCLDSTOP;
		if (sa->sa_flags & SA_NOCLDWAIT)
			p->p_flag |= SNOCLDWAIT;
		else
			p->p_flag &= ~SNOCLDWAIT;
	}

	/*
	 * Set bit in p_sigignore for signals that are set to SIG_IGN,
	 * and for signals set to SIG_DFL where the default is to ignore.
	 * However, don't put SIGCONT in p_sigignore,
	 * as we have to restart the process.
	 */
	if (sa->sa_handler == SIG_IGN ||
	   (bit & defaultignmask && sa->sa_handler == SIG_DFL)) {
		p->p_sig &= ~bit;		/* never to be seen again */

		/* LATER: may want to leverage this off of p->p_siginfo flag */
		sigq_remove_all(&p->p_sigqueue, sig);

		/*
		 *	If this is a thread signal, clean out the
		 *	threads as well.
		 */
		if (bit & threadmask) {
			register	queue_head_t	*list;
			register	thread_t	thread;

			list = &p->task->thread_list;
			simple_lock(&p->task->thread_list_lock);
			thread = (thread_t) queue_first(list);
			while (!queue_end(list, (queue_entry_t) thread)) {
				thread->u_address.uthread->uu_sig &= ~bit;
				sigq_remove_all(
				       &thread->u_address.uthread->uu_sigqueue,
				       sig);
				thread = (thread_t)
					queue_next(&thread->thread_list);
			}
			simple_unlock(&p->task->thread_list_lock);
		}
		if (sig != SIGCONT)
			p->p_sigignore |= bit;	/* easier in psignal */
		p->p_sigcatch &= ~bit;
	} else {
		p->p_sigignore &= ~bit;
		if (sa->sa_handler == SIG_DFL)
			p->p_sigcatch &= ~bit;
		else {
			p->p_sigcatch |= bit;
			if (sa->sa_flags & SA_SIGINFO)
				p->p_siginfo |= bit;
			else
				p->p_siginfo &= ~bit;
		}
	}
	sig_unlock(p);
	(void) spl0();
}

/*
 * Initialize signal state for process 0;
 * set to ignore signals that are ignored by default.
 */
siginit(p)
	struct proc *p;
{

	p->p_sigignore = defaultignmask &~ sigmask(SIGCONT);
	queue_init(&p->p_sigqueue);
	p->p_habitat = USW_OSF1;
}

/*
 * Reset signals for an exec of the specified process.
 */
execsigs(p)
	register struct proc *p;
{
	register int nc;
	sigset_t mask;
	int s;

#ifndef	multimax
	ASSERT(syscall_on_master());
#endif
	s = splhigh();
	/*
	 * Reset caught signals.  Held signals remain held
	 * through p_sigmask (unless they were caught,
	 * and are now ignored by default).
	 */
	while (p->p_sigcatch) {
		nc = ffs((long)p->p_sigcatch);
		mask = sigmask(nc);
		p->p_sigcatch &= ~mask;
		if (mask & defaultignmask) {
			if (nc != SIGCONT)
				p->p_sigignore |= mask;
			p->p_sig &= ~mask;
			/*
			 * Clear out all sigqueues of this signal
			 * type from the proc structure.
			 * LATER: tie this to siginfo mask?
			 */
			sigq_remove_all(&p->p_sigqueue, nc);

			/*
			 * If this is a thread signal, clear it
			 * from the current thread as well.
		         */

			if (mask & threadmask) {
				u.u_sig &= ~mask;
				/*
				 * clear out all sigqueue structures
				 * of this signal type.  Assume thread
				 * specifiec siginfo structures for
				 * threads other than the current thread
				 * have been removed already, because
				 * all other threads were already
				 * removed.
				 */
				sig_lock_simple(p);
				sigq_remove_all(&u.u_sigqueue, nc);
				sig_unlock(p);
			}
		}
		signal_disposition(nc) = SIG_DFL;
	}
	p->p_siginfo = 0;
	splx(s);

	/*
	 * Reset stack state to the user stack.
	 * Clear set of signals caught on the signal stack.
	 */
	u.u_sigresethand = 0;
	u.u_signodefer = 0;
	/* LATER: how about SNOCLDSTOP? */
	p->p_flag &= ~SNOCLDWAIT;

	/*
	 * Do not inherit habitat.  If the exec'ed process is really
	 * an SVR4 process, it will call uswitch() from crt0 to set
	 * this field and get SVR4 behavior.
	 */
	p->p_habitat = USW_OSF1;
	u.u_uswitch = USW_OSF1;
	u.u_sigssz = 0;
	u.u_sigsflags = SS_DISABLE;
	u.u_sigsp = NULL;
}

/*
 * Manipulate signal mask.
 * Note that we receive new mask, not pointer,
 * and return old mask as return value;
 * the library stub does the rest.
 */
sigprocmask(p, args, retval)
	register struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long		how;		/* real type: 'int' */
		sigset_t	mask;
	} *uap = (struct args *) args;
	int error = 0;
	sigset_t mask = uap->mask;

	ASSERT(syscall_on_master());
	*retval = p->p_sigmask;
	(void) splhigh();

	switch (uap->how) {
	case SIG_BLOCK:
		p->p_sigmask |= mask &~ sigcantmask;
		break;

	case SIG_UNBLOCK:
		p->p_sigmask &= ~mask;
		break;

	case SIG_SETMASK:
		p->p_sigmask = mask &~ sigcantmask;
		break;
	
	default:
		error = EINVAL;
		break;
	}
	(void) spl0();
	return (error);
}

sigpending(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;

{

	*retval = p->p_sig;
	return (0);
}

#if COMPAT_43
/*
 * Generalized interface signal handler, 4.3-compatible.
 */
osigvec(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	signo;
		struct	sigvec *nsv;
		struct	sigvec *osv;
#if	defined(balance) || defined(mips) || defined(__alpha)
		int	(*sigtramp)();		/* signal trampoline code */
#endif
	} *uap = (struct args *) args;
	struct sigvec vec;
	register struct sigvec *sv;
	register int sig;
	int error;
	sigset_t bit;
	struct sigaction sigact;
	register struct sigaction *sa = &sigact;
#ifdef	i386
	u.u_sigreturn = (int (*)())u.u_ar0[EDX];
#endif

	ASSERT(syscall_on_master());
	sig = uap->signo;
#ifdef	multimax
	/*
	 * On the MMax the u area is not visible to the user tasks,
	 * therefore, the task must declare the address of its
	 * trampoline code before it attempts to catch signals.
	 * This is done by using the special signal SIGCATCHALL
	 */
	/* If this is the intermediate signal catcher then set it. */
	if (sig == SIGCATCHALL) {

		/* Copy the uap->nsv structure from user space to system
		 *	space.  Then set the return value and the signal
		 *	catcher.
		 */
		if (error = copyin ((caddr_t)uap->nsv, (caddr_t)&vec,
		    sizeof(vec)))
			return (error);
		*retval = (long)u.u_sigcatch;
		u.u_sigcatch = (void (*))vec.sv_handler;
		return (0);
	}
#endif
	if (sig <= 0 || sig > NSIG) {
		return (EINVAL);
	}
	sv = &vec;
	if (uap->osv) {
		sv->sv_handler = signal_disposition(sig);
		sv->sv_mask = u.u_sigmask[sig];
		bit = sigmask(sig);
		sv->sv_flags = 0;
		if ((u.u_sigonstack & bit) != 0)
			sv->sv_flags |= SV_ONSTACK;
		if ((u.u_sigintr & bit) != 0)
			sv->sv_flags |= SV_INTERRUPT;
		if (p->p_flag & SNOCLDSTOP)
			sv->sv_flags |= SA_NOCLDSTOP;
		if (error = copyout((caddr_t)sv, (caddr_t)uap->osv,
                    sizeof (vec)))
			return (error);
	}
	if (uap->nsv) {
		if (error = copyin((caddr_t)uap->nsv, (caddr_t)sv,
                    sizeof (vec)))
			return (error);
		if (sv->sv_handler != SIG_DFL && (sig == SIGKILL || sig == SIGSTOP))
			return(EINVAL);
		if (sig == SIGCONT && sv->sv_handler == SIG_IGN) {
			return (EINVAL);
		}
#if	defined(mips) || defined(__alpha)
		/*
		 * check for unaligned pc on sighandler
		 */
		if (sv->sv_handler != SIG_IGN
		    && ((int)sv->sv_handler & (sizeof(int)-1))) {
			return (EINVAL);
		}
#endif /* mips or __alpha */
		sv->sv_flags ^= SA_RESTART; /* opposite of SV_INTERRUPT */
		sa->sa_flags = sv->sv_flags;
		sa->sa_handler = sv->sv_handler;
		sa->sa_mask = sv->sv_mask;
		setsigvec(p, sig, sa);
#if	defined(balance) || defined(mips) || defined(__alpha)
		/*
		 * On the Sequent Balance and Mips, struct user isn't
		 * visible to the executing thread; thus the trampoline
		 * code pointer is explicitly passed in when setting
		 * a signal handler.
		 */
		u.u_sigtramp = uap->sigtramp;
#endif
	}
	return (0);
}

osigblock(p, args, retval)
	register struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long	mask;
	} *uap = (struct args *) args;

	ASSERT(syscall_on_master());
	(void) splhigh();
	*retval = p->p_sigmask;
	p->p_sigmask |= uap->mask &~ sigcantmask;
	(void) spl0();
	return (0);
}

osigsetmask(p, args, retval)
	register struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long	mask;
	} *uap = (struct args *) args;

	ASSERT(syscall_on_master());
	(void) splhigh();
	*retval = p->p_sigmask;
	p->p_sigmask = uap->mask &~ sigcantmask;
	(void) spl0();
	return (0);
}

#endif	/* COMPAT_43 */

/*
 * Suspend process until signal, providing mask to be set
 * in the meantime.  Note nonstandard calling convention:
 * libc stub passes mask, not pointer, to save a copyin.
 */
sigsuspend(p, args, retval)
	register struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		sigset_t	mask;
	} *uap = (struct args *) args;

	ASSERT(syscall_on_master());
	/*
	 * When returning from sigsuspend, we want
	 * the old mask to be restored after the
	 * signal handler has finished.  Thus, we
	 * save it here and mark the proc structure
	 * to indicate this (should be in u.).
	 */
	u.u_oldmask = p->p_sigmask;
	p->p_flag |= SOMASK;
	p->p_sigmask = uap->mask &~ sigcantmask;
	(void) tsleep((caddr_t)&u, PSLEP | PCATCH, "pause", 0);
	/* always return EINTR rather than ERESTART... */
	return (EINTR);
}

sigstack(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		struct	sigstack *nss;
		struct	sigstack *oss;
	} *uap = (struct args *) args;
	struct sigstack ss;
	int error=0;

	ASSERT(syscall_on_master());

	/*
	 * If caller wants old signal stack state, translate from SYS V
	 * stack_t in utask to local BSD struct sigstack before copying
	 * it out.
	 */
	if (uap->oss) {
		ss.ss_onstack = u.u_sigsflags & SS_ONSTACK;
		ss.ss_sp = u.u_sigsp;
		if ((error = copyout((caddr_t)&ss,
			     (caddr_t)uap->oss, sizeof (struct sigstack))))
			return (error);
	}

	if (uap->nss && (error = copyin((caddr_t)uap->nss, (caddr_t)&ss,
            sizeof (ss))) == 0)	{
		if (ss.ss_onstack) 
			u.u_sigsflags = SS_ONSTACK;
		else
			/* Turn off default ss_disable flag */
			u.u_sigsflags = 0;
		u.u_sigsp = ss.ss_sp;
		u.u_sigssz = 0;
	}
	return error;
}

/*
 * SVR4 sigaltstack() system call.  This differs from sigstack() in
 * that it sets and returns the size of the stack as well as the
 * location of stack.  You can also mark the signalstack as invalid
 * with the SS_DISABLE flag.
 *
 * NOTE: sigaltstack has a hidden third arguemnt to support getcontext().
 */
sigaltstack(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		stack_t *nss;
		stack_t *oss;
		long	base_info;
	} *uap = (struct args *) args;
	stack_t ss;
	int error;

	ASSERT(syscall_on_master());

	/*
	 * Note that the utask area has been modified to use an
	 * SVR4 stack_t structure instead of a 4BSD sigstack
	 * structure, so we can copy right into the utask structure.
	 */
	if (uap->oss) {

		/*
		 * If the 3rd arg is true, then were being called from
		 * _sigaltstack in libsys5 to request process stack info
		 * for getcontext(). But -- if we're already on the
		 * sigstack, then we want that information back instead.
		 */
		if (uap->base_info && !(u.u_sigsflags & SS_ONSTACK)) {
			ss.ss_flags = u.u_sigsflags;
			ss.ss_sp = u.u_stack_end;
			ss.ss_size = ctob(u.u_ssize);
		} else {
			ss = u.u_sigstack;
			if (u.u_stack_grows_up == FALSE)
				/*
				 * Remove adjustment for stack growth
				 * direction before copying out.
				 */
				ss.ss_sp -= ss.ss_size;
		}
		if (error = copyout((caddr_t)&ss, (caddr_t)uap->oss,
				    sizeof (ss)))
			return (error);
	}
	if (uap->nss) {
		/*
		 * can't modify signal stack if currently running 
		 * on it.
		 */
		if (u.u_sigsflags & SS_ONSTACK)
			return EPERM;

		if (error = copyin((caddr_t)uap->nss, (caddr_t)&ss,
				   sizeof (ss)))
			return error;

		switch (ss.ss_flags) {
		case SS_DISABLE:
			break;
		case 0:
			if (ss.ss_size < MINSIGSTKSZ)
				/* stack size too small */
				return ENOMEM;
			break;
		default:
			/* only the disable flag is valid */
			return EINVAL;
		}

		if (u.u_stack_grows_up == FALSE)
			/*
			 * Adjust for stack growth direction.
			 * In other words,
			 */
			ss.ss_sp += ss.ss_size;
		u.u_sigstack = ss;
	}
	return error;
}

kill(cp, args, retval)
	register struct proc *cp;
	void *args;	
	long *retval;
{
	register struct args {
		long	pid;
		long	signo;
	} *uap = (struct args *) args;
	register struct proc *p;
	register k_siginfo_t	siginfo;

	ASSERT(syscall_on_master());

	if ((unsigned) uap->signo > NSIG)
		return (EINVAL);
	
	if (uap->pid > 0) {
		/* kill single process */
		p = pfind(uap->pid);
		if (p == 0)
			return (ESRCH);

		if (!CANSIGNAL(cp, p, uap->signo))
			return (EPERM);
#if     SEC_ARCH
                 if (!sec_can_kill(p))
			return (EACCES);
#endif
#if     SEC_BASE
		if (p->p_flag & SSYS)
			return (0);
#endif
#if	SEC_MAC || SEC_NCAV
                AUDSTUB_PROC_LEVELS(p);
#endif
		if (uap->signo) {
			siginfo.si_signo = uap->signo;
			siginfo.si_code = SI_USER;	/* from user */
			siginfo.si_errno = 0;
			siginfo.si_pid = cp->p_pid;
			siginfo.si_uid = cp->p_ruid;
			psignal_info(p, uap->signo, &siginfo);
		}
		return (0);
	}
	switch (uap->pid) {
	case -1:		/* broadcast signal */
		return (killpg1(cp, uap->signo, 0, 1));
	case 0:			/* signal own process group */
		return (killpg1(cp, uap->signo, 0, 0));
	default:		/* negative explicit process group */
		return (killpg1(cp, uap->signo, -uap->pid, 0));
	}
	/* NOTREACHED */
}

#if COMPAT_43
okillpg(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	pgid;
		long	signo;
	} *uap = (struct args *) args;

	ASSERT(syscall_on_master());
	if ((unsigned) uap->signo > NSIG)
		return (EINVAL);
	return (killpg1(p, uap->signo, uap->pgid, 0));
}
#endif

killpg1(cp, signo, pgid, all)
	register struct proc *cp;
	long signo, pgid, all;
{
	register struct proc *p;
	struct pgrp *pgrp;
	int f = 0, error = ESRCH;
	k_siginfo_t siginfo;

	ASSERT(syscall_on_master());

	/*
	 * Set up for later use.
	 */
	siginfo.si_signo = signo;
	siginfo.si_code = SI_USER;		/* from user */
	siginfo.si_errno = 0;
	siginfo.si_pid = cp->p_pid;
	siginfo.si_uid = cp->p_ruid;

	if (all)	
		/*
		 * broadcast
		 */
		for (p = allproc; p != NULL; p = p->p_nxt) {
			if (p->p_ppid == 0 || p->p_flag&SSYS ||
			    !CANSIGNAL(cp, p, signo))
				continue;
			if ((signo == SIGKILL || signo == SIGSTOP)
			    && p == u.u_procp)
				continue;
#if	SEC_ARCH
                        if (!sec_can_kill(p))
				continue;
#if     SEC_MAC || SEC_NCAV
                        AUDSTUB_PROC_LEVELS(p);
#endif
#endif
			f++;
			if (signo)
				psignal_info(p, signo, &siginfo);
		}
	else {
		if (pgid == 0)		
			/*
			 * zero pgid means send to my process group.
			 */
			pgrp = u.u_procp->p_pgrp;
		else {
			pgrp = pgfind(pgid);
			if (pgrp == NULL)
				return (ESRCH);
		}
		for (p = pgrp->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			if (p->p_ppid == 0 || p->p_flag&SSYS ||
			!CANSIGNAL(cp, p, signo))
				continue;
#if	SEC_ARCH
                        if (!sec_can_kill(p)) {
				error = EACCES;
				continue;
			}
#if     SEC_MAC || SEC_NCAV
                        AUDSTUB_PROC_LEVELS(p);
#endif
#endif
			f++;
			if (signo)
				psignal_info(p, signo, &siginfo);
		}
	}
	return (f ? 0 : error);
}

/*
 * Send the specified signal to
 * all processes with 'pgid' as
 * process group.
 */
gsignal(pgid, sig)
	long pgid, sig;
{
	struct pgrp *pgrp;

	ASSERT(syscall_on_master());
	if (pgid && (pgrp = pgfind(pgid)))
		pgsignal(pgrp, sig, 0);
}

/*
 * Send sig to all all members of the process group
 * If checktty is 1, limit to members which have a controlling
 * terminal.
 */

pgsignal(pgrp, sig, checkctty)
	struct pgrp *pgrp;
	long sig, checkctty;
{
	register struct proc *p;

	ASSERT(syscall_on_master());
	if (pgrp)
		for (p = pgrp->pg_mem; p != NULL; p = p->p_pgrpnxt)
			if (checkctty == 0 || p->p_flag&SCTTY)
				psignal(p, sig);
}

/*
 * For debugging: this keeps track of bogus attempts to sigq_alloc()
 * from interrupt level. This value should alway remain zero....
 */
int sigalloc_atintr;

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
	register struct proc *p;
	register long sig;
{
	psignal_info(p, sig, NULL);
}

psignal_info(p, sig, siginfo_p)
	register struct proc *p;
	register int sig;
	register k_siginfo_t *siginfo_p;
{
	register k_siginfo_t child_siginfo;
	register sigqueue_t sigqueue;
	register sigset_t tmpstopmask, stopsig;
	register int s;
	register sig_t action;
	register thread_t	sig_thread;
	register task_t		sig_task;
	register thread_t	cur_thread;
	sigset_t mask;
#if RT_SCHED
	register boolean_t twiddle;	/* Used below for monitoring. */
#endif /* RT_SCHED */

	ASSERT(syscall_on_master());

	if ((unsigned)sig > NSIG || sig == 0)
		panic("psignal sig");
	mask = sigmask(sig);

	/*
	 *	We will need the task pointer later.  Grab it now to
	 *	check for a zombie process.  Also don't send signals
	 *	to kernel internal tasks.
	 */
	if (((sig_task = p->task) == TASK_NULL) || sig_task->kernel_vm_space)
		return;

	/*
	 * If proc is traced, always give parent a chance.
	 */
	if((p->p_flag & STRC) ||
	   ((p->p_pr_qflags & PRFS_STOPEXEC) && sig == SIGTRAP))
		action = SIG_DFL;
	else {
		/*
		 * If the signal is being ignored,
		 * then we forget about it immediately.
		 * (Note: we don't set SIGCONT in p_sigignore,
		 * and if it is set to SIG_IGN,
		 * action will be SIG_DFL here.)
		 */
		if (p->p_sigignore & mask)
			return;
		if (p->p_sigmask & mask)
			action = SIG_HOLD;
		else if (p->p_sigcatch & mask)
			action = SIG_CATCH;
		else {
			if (p->p_pgrp->pg_jobc == 0 && (sig == SIGTTIN ||
			    sig == SIGTTOU || sig == SIGTSTP))
				return;
			action = SIG_DFL;
		}
	}

	/*
	 * If there is siginfo information, then allocate a sigqueue
	 * structure.
	 */
	sigqueue = NULL;
	if (siginfo_p) {
		/*
		 * Allocate a sigqueue structure.
		 */
		sigqueue = SIGQ_ALLOC();

		/*
		 * if couldn't allocate sigqueue then don't queue
		 * up any siginfo information -- this appears to
		 * be an acceptable behavior in SVR4.
		 */
		if (sigqueue)
			/* structure copy... */
			sigqueue->siginfo = *siginfo_p;
	}

	/*
	 * Post signal and queue sigqueue if none is posted yet for
	 * this signal type.  Attach sigqueue to proc structure
	 * p_sigqueue queue.
	 *
	 * It is sufficient to check the signal posted bit array
	 * to see whether a sigqueue structure should be queued.
	 * It is possible that the previously posted instance of this
	 * signal did not have any siginfo inforamtion to queue,
	 * and that no sigqueue for this signal type has been queued,
	 * but in this case it is appropriate that no siginfo
	 * information be delivered for the currently posted signal.
	 *
	 * Note that if a thread-specific signal is queued to the
	 * proc structure via this routine, then its siginfo information
	 * will only be delivered if there isn't also a sigqueue
	 * structure for this signal on the thread specific queue for
	 * the first thread in the process.  This sigqueue structure
	 * will just be tossed by issig() during delivery of the signal
	 * and sigqueue structure in the thread's signal pending queue.
	 *
	 * Set splhigh() so a second invocation of psignal() doesn't
	 * queue another signal of the same type, or mess up the
	 * p_sigqueue data structure.
	 */
	s = splhigh();
	if (!(p->p_sig & mask)) {
		p->p_sig |= mask;
		if (sigqueue) {
			sigq_enqueue_tail(&p->p_sigqueue, sigqueue);
			sigqueue = NULL;
		}
	}

	/* if couldn't queue sigqueue, free it now */
	SIGQ_FREE(sigqueue);

	switch (sig) {
	case SIGTERM:
		if ((p->p_flag&STRC) || action != SIG_DFL)
			break;
		/* fall into ... */

	case SIGCONT:
		/*
		 * for /proc, set PR_JOBCONTROL in the pr_what field of the
		 * task->procfs struct.
		 */
		p->task->procfs.pr_why = PR_JOBCONTROL;
		p->task->procfs.pr_what = SIGCONT;
		p->task->procfs.pr_flags &= ~PR_STOPPED;
		/* end of /proc code */

		/* get set of currently queued stop sigs */
		tmpstopmask = p->p_sig & stopsigmask;
		p->p_sig &= ~stopsigmask;
		/* remove sigqueue structures and sig bit for stop signals */
		while (tmpstopmask) {
			stopsig = ffs(tmpstopmask);
			tmpstopmask &= ~(sigmask(stopsig));
			sigq_remove_all(&p->p_sigqueue, stopsig);
		}
		break;
		
	case SIGSTOP:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
		/*
		 * for /proc, set PR_JOBCONTROL in the pr_what field of the
		 * task->procfs struct.
		 */
		p->task->procfs.pr_why = PR_JOBCONTROL;
		p->task->procfs.pr_what = sig;
		p->task->procfs.pr_flags |= PR_STOPPED;
		/* end of /proc code */

		p->p_sig &= ~sigmask(SIGCONT);
		/* remove SIGCONT sigqueue structures */
		sigq_remove_all(&p->p_sigqueue, SIGCONT);
		break;
	}

	/*
	 * Defer further processing for signals which are held,
	 * except that stopped processes must be continued by SIGCONT.
	 */
	if (action == SIG_HOLD && (sig != SIGCONT || p->p_stat != SSTOP)) {
		splx(s);
		return;
	}

	/*
	 *	Deliver the signal to the first thread in the task. This
	 *	allows single threaded applications which use signals to
	 *	be able to be linked with multithreaded libraries.  We have
	 *	an implicit reference to the current_thread, but need
	 *	an explicit one otherwise.  The thread reference keeps
	 *	the corresponding task data structures around too.  This
	 *	reference is released by thread_deallocate_interrupt
	 *	because psignal() can be called from interrupt level).
	 *	NOTE: We used to go to splsched(), but since the signal
	 *	queue operations need splhigh(), that's where we go now.
	 */

	cur_thread = current_thread();
	/*
	 *	This is a mess.  The thread_list_lock is a special
	 *	lock that excludes insert and delete operations
	 *	on the task's thread list for our benefit (can't
	 *	grab task lock because we might be at interrupt
	 *	level).  Check if there are any threads in the
	 *	task.  If there aren't, sending it a signal
	 *	isn't going to work very well, so just return.
	 */
	simple_lock(&sig_task->thread_list_lock);
	if (queue_empty(&sig_task->thread_list)) {
		simple_unlock(&sig_task->thread_list_lock);
		(void) splx(s);
		return;
	}
	sig_thread = (thread_t) queue_first(&sig_task->thread_list);
	if (sig_thread != cur_thread)
		thread_reference(sig_thread);
	simple_unlock(&sig_task->thread_list_lock);

	/*
	 *	SIGKILL priority twiddling moved here from above because
	 *	it needs sig_thread.  Could merge it into large switch
	 *	below if we didn't care about priority for tracing
	 *	as SIGKILL's action is always SIG_DFL.
	 */
#if RT_SCHED
	/*
	 * Don't mess with the priority of a fixed priority process.  The nice
	 * value for a fixed priority process still reflects the base priority,
	 * so an explicit policy check is needed.  If a process is pegged at a
	 * low priority, the application _wants_ it to starve.  This introduces
	 * a priority inversion problem, but that should be addressed by the
	 * application designer.
	 *
	 * Changing max_priority here doesn't impact the use of that field for
	 * security, because the process is about to go away anyway.
	 */
	twiddle = ((sig == SIGKILL) && (p->p_nice > PRIZERO)
		   && (sig_thread->policy & ~(POLICY_FIFO|POLICY_RR)));
	RT_SCHED_HIST(RTS_kill, sig_thread, sig_thread->policy,
		      p->p_nice, twiddle, 0);
	if (twiddle)
#else /* RT_SCHED */
	if ((sig == SIGKILL) && (p->p_nice > PRIZERO))
#endif /* RT_SCHED */
	{
		p->p_nice = PRIZERO;
		thread_max_priority(sig_thread, sig_thread->processor_set,
			BASEPRI_USER, TRUE);
		thread_priority(sig_thread, BASEPRI_USER, FALSE, TRUE);
	}

	if ((p->p_flag & STRC) ||
	   ((p->p_pr_qflags & PRFS_STOPEXEC) && sig == SIGTRAP)) {
		/*
		 *	Process is traced - wake it up (if not already
		 *	stopped) so that it can discover the signal in
		 *	issig() and stop for the parent.
		 */
		if (p->p_stat != SSTOP) {
			/*
			 *	Wake it up to get signal
			 */
			goto run;
		}
		goto out;
	}
	else if (action != SIG_DFL) {
		/*
		 *	User wants to catch the signal.
		 *	Wake up the thread, but don't un-suspend it
		 *	(except for SIGCONT).
		 */
		if (sig == SIGCONT) {
			(void) task_resume(sig_task);
			/*
			 *	Process will be running after 'run'
			 */
			p->p_stat = SRUN;
			p->p_wcode = CLD_CONTINUED;
			p->p_xstat = sig;
			/*
			 * SVR4 parent expects to be sent a SIGCHLD if
			 * child receives a SIGCONT, but OSF/1 doesn't.
			 */
			if (ISHAB_SVR4(p->p_pptr->p_habitat)) {
				/* register info for waitfxwhat -() */
				if (!(p->p_pptr->p_flag & SNOCLDSTOP)) {
					child_siginfo.si_signo = SIGCHLD;
					child_siginfo.si_pid = p->p_pid;
					child_siginfo.si_code = CLD_CONTINUED;
					child_siginfo.si_status = sig;
					child_siginfo.si_errno = 0;
					psignal_info(p->p_pptr, SIGCHLD, 
						     &child_siginfo);
				}
			}
			wakeup((caddr_t)p->p_pptr);
		}
		goto run;
	}
	else {
		/*
		 *	Default action - varies
		 */

		if (mask & stopsigmask) {
			/*
			 * These are the signals which by default
			 * stop a process.
			 *
			 * Don't clog system with children of init
			 * stopped from the keyboard.
			 */
			if (sig != SIGSTOP && p->p_pptr == &proc[1]) {
				psignal_info(p, SIGKILL, NULL);
				p->p_sig &= ~mask;
				/*
				 * Finished handling this signal -- about
				 * to be killed, in fact -- so get rid of
				 * associated sigqueue.
				 */
				sigq_remove_all(&p->p_sigqueue, sig);
				goto out;
			}
			/*
			 *	Stop the task.
			 */
			if ((sig_thread->state & TH_RUN) == 0) {
				/*
				 *	If task hasn't already been stopped by
				 *	a signal, stop it.
				 */
				p->p_sig &= ~mask;

				/*
				 * Default signal action for stop 
				 * signals is being performed now, 
				 * so it's time to free the sigqueue 
				 * struct for this signal.
				 */
				sigq_remove_all(&p->p_sigqueue, sig);

				if (sig_task->user_stop_count == 0) {
					/*
					 * p_cursig must not be set, because
					 * it will be psig()'d if it is not
					 * zero, and the signal is being
					 * handled here.  But save the signal
					 * in p_stopsig so WUNTRACED
					 * option to wait can find it.
					 */

					/*
					 * No longer use p_stopsig proc field.
					 * Signal saved in p_xstat instead for
					 * waitf().
					 */
					p->p_wcode = CLD_STOPPED;
					p->p_xstat = sig;

					if ((p->p_pptr->p_flag & SNOCLDSTOP) == 0) {
						child_siginfo.si_signo = SIGCHLD;
						child_siginfo.si_pid = p->p_pid;
						child_siginfo.si_code = CLD_STOPPED;
						child_siginfo.si_status = sig;
						child_siginfo.si_errno = 0;
						psignal_info(p->p_pptr,
							     SIGCHLD, 
							     &child_siginfo);

					}	
					stop(p);
				}
				goto out;
			}
			else {
				if ((p == u.u_procp) && (p->p_stat != SZOMB))
#ifdef	vax
#include <vax/mtpr.h>
#endif
					aston();
				goto out;
			}
		}

		switch (sig) {
			/*
			 * Signals ignored by default have been dealt
			 * with already, since their bits are on in
			 * p_sigignore.
			 */

		case SIGKILL:
			/*
			 * Kill signal always sets process running and
			 * unsuspends it.
			 */
			while (sig_task->user_stop_count > 0)
				(void) task_resume(sig_task);
			/*
			 *	Process will be running after 'run'
			 */
			p->p_stat = SRUN;

			/*
			 * Break it out of user wait, as well.
			 */
			while (sig_thread->user_stop_count > 0)
				(void) thread_resume(sig_thread);

			/*
			 * Clear system wait if possible.  The
			 * THREAD_SHOULD_TERMINATE is overkill, but
			 * saves us from potentially buggy code elsewhere.
			 */
			clear_wait(sig_thread, THREAD_SHOULD_TERMINATE, FALSE);
#if	MACH_HOST
			/*
			 * Make sure it can run.
			 */
			if (sig_thread->processor_set->empty)
				thread_assign(sig_thread, &default_pset);
#endif
			/*
			 * If we're delivering the signal to some other
			 * thread, that thread might be stuck in an
			 * exception.  Break it out.  Can't call
			 * thread_exception_abort from high spl, but
			 * SIGKILL can't be sent from interrupt level, so
			 * it's ok to drop spl.  Can call thread_deallocate
			 * for same reason.
			 */
			splx(s);
			if (sig_thread != cur_thread) {
				thread_exception_abort(sig_thread);
				thread_deallocate(sig_thread);
			}
			return;

		case SIGCONT:
			/*
			 * Let the process run.  If it's sleeping on an
			 * event, it remains so.
			 */
			(void) task_resume(sig_task);
			p->p_stat = SRUN;

			/*
			 * SVR4 parent expects to be sent a SIGCHLD if
			 * child receives a SIGCONT, but OSF/1 doesn't.
			 */

			p->p_wcode = CLD_CONTINUED;
			p->p_xstat = sig;
			if (ISHAB_SVR4(p->p_pptr->p_habitat)) {
				/* register wait info */
				if (!(p->p_pptr->p_flag & SNOCLDSTOP)) {
					child_siginfo.si_signo = SIGCHLD;
					child_siginfo.si_pid = p->p_pid;
					child_siginfo.si_code = CLD_CONTINUED;
					child_siginfo.si_status = sig;
					child_siginfo.si_errno = 0;
					psignal_info(p->p_pptr, SIGCHLD, 
						     &child_siginfo);
				}
			}
			wakeup((caddr_t)p->p_pptr);
			goto out;

		default:
			/*
			 * All other signals wake up the process, but don't
			 * resume it.
			 */
			goto run;
		}
	}
	/*NOTREACHED*/
run:
	/*
	 *	BSD used to raise priority here.  This has been broken
	 *	for ages and nobody's noticed.  Code deleted. -dlb
	 */

	/*
	 *	Wake up the thread if it is interruptible.
	 */
	clear_wait(sig_thread, THREAD_INTERRUPTED, TRUE);
out:
	splx(s);
	if (sig_thread != cur_thread)
		thread_deallocate_interrupt(sig_thread);
}

/*
 * Returns true if the current
 * process has a signal to process.
 * The signal to process is put in p_cursig.
 * This is asked at least once each time a process enters the
 * system (though this can usually be done without actually
 * calling issig by checking the pending signal masks.)
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
issig()
{
	register struct proc *p;
	register int sig;
	sigset_t sigbits, mask;
	thread_t initial_thread;
	thread_t cur_thread;
	int s;
	k_siginfo_t child_siginfo;

	p = u.u_procp;			/* XXX */
	/*
	 *	This must be called on master cpu
	 */
	if (cpu_number() != master_cpu)
		panic("issig not on master");

	/*
	 *	Try for the signal lock.
	 *	If we already have it, return FALSE: don't handle any signals.
	 *	If we must halt, return TRUE to clean up our state.
	 */
	sig_lock_or_return(p, return(FALSE), return(TRUE));
	for (;;) {
		/*
		 *	In a multi-threaded task it is possible for
		 *	one thread to interrupt another's issig(); psig()
		 *	sequence.  In this case, the thread signal may
		 *	be left in u.u_cursig.  We recover here
		 *	by getting it out and starting over.
		 */
		if (u.u_cursig != 0) {
			u.u_sig |= sigmask(u.u_cursig);
			u.u_cursig = 0;
			SIGQ_REQUEUE(&u.u_sigqueue, u.u_curinfo);
			
		}
		sigbits = (u.u_sig | p->p_sig) &~ p->p_sigmask;
		if (p->p_flag&SVFORK)
			sigbits &= ~stopsigmask;

		/*
		 * Only allow delivery of process signals (asynchronous)
		 * to the initial thread. This is the first thread in
		 * the tasks thread list.
		 *
		 * Note: if a synchronous (thread) signal is posted via kill,
		 * *any* thread passing through this code can take the signal.
		 */
		s = splhigh();
		simple_lock(&p->task->thread_list_lock);
		initial_thread = (thread_t)queue_first(&p->task->thread_list);
		simple_unlock(&p->task->thread_list_lock);
		cur_thread = current_thread();
		for (;;) {
			if (sigbits == 0)
				break;
			sig = ffs((long)sigbits);
			mask = sigmask(sig);
			if (mask & threadmask) {
				u.u_sig &= ~mask;
				u.u_cursig = sig;
				/*
				 * If no sigqueue, u.u_curinfo is set to NULL
				 */
				SIGQ_DEQUEUE(&u.u_sigqueue, sig, u.u_curinfo);
				/*
				 * Remove any sigqueue for this signal from
				 * the proc structure sigqueue queue.  The 
				 * thread sigqueue has precedence.
				 */
				if (u.u_curinfo)
					sigq_remove_all(&p->p_sigqueue, sig);
				break;
			} else {
				if (cur_thread != initial_thread)
					sigbits &= ~mask;
				else {
					break;
				}
			}
		}
		if (sigbits == 0) {
			splx(s);
			break;
		}

		/*
		 * If not sigqueue entry was obtained from the thread
		 * queue, try from the process queue.
		 */
		if (u.u_curinfo == NULL)
			SIGQ_DEQUEUE(&p->p_sigqueue, sig, u.u_curinfo);

		p->p_sig &= ~mask;		/* take the signal! */

		/* Done with all interrupt-sensitive operations. */
		splx(s);

		/*
		 * If the signal is being ignored, look for the next one.
		 * It has now been removed from p->p_sig, and will never
		 * be seen again. Note that this is a change from the
		 * 4.3BSD behavior, where ignored signals were excluded
		 * from sigbits above, and thus remained pending. This would
		 * mean that if the action for such a signal were changed
		 * later (possibly MUCH later), the old signal would
		 * suddenly get delivered. --rsc
		 */
		if (mask & p->p_sigignore && (p->p_flag&STRC) == 0) {
			SIGQ_FREE(u.u_curinfo);
			/* p->p_cursig not set yet */
			u.u_cursig = 0;
			splx(s);
			continue;	/* only if STRC was on when posted */
		}

		p->p_cursig = sig;

		if ((p->p_flag & STRC) && ((p->p_flag&SVFORK) == 0)) {
			register int	hold;
			register task_t	task;

			/*
			 * If traced, always stop, and stay
			 * stopped until released by the parent.
			 */

			/* Check the new proc structure field
			 * to see if being traced by /proc.  If so,
			 * then call proc_signal() to notify tracing
			 * process.  Always assume process is being
			 * ptraced().
			 *
			 * Not supporting /proc in this implementation.
			 * So don't need this code
			 *
			 * if (p->p_procsig & mask )
			 *	proc_signal(p,sig);
			 */

			/*
			 * SVR4 does not expect a SIGCHLD at this point,
			 * although it does return a CLD_TRAPPED to the
			 * parent through waitid().  The SVR4 documentation
			 * also mentions that no SIGCHLD is sent.
			 *
			 * Select whether to send the SIGCHLD or not
			 * based on the parent's habitat mechanism.
			 */
			if (!ISHAB_SVR4(p->p_pptr->p_habitat)) {
				child_siginfo.si_signo = SIGCHLD;
				child_siginfo.si_pid = p->p_pid;
				child_siginfo.si_code = CLD_TRAPPED;
				child_siginfo.si_status = sig;
				child_siginfo.si_errno = 0;
				psignal_info(p->p_pptr, SIGCHLD,
					     &child_siginfo);
			}

			/* wait info */
			p->p_wcode = CLD_TRAPPED;
			p->p_xstat = sig;

			/*
			 *	New ptrace() has no procxmt.  Only action
			 *	executed on behalf of parent is exit.
			 *	This is requested via a large p_cursig.
			 *	sig_lock_to_wait causes other threads
			 *	that try to handle signals to stop for
			 *	debugger.
			 */
			p->thread = current_thread();
#ifdef	i386
#else
			pcb_synch(p->thread);
#endif
			/*
			 *	XXX Have to really stop for debuggers;
			 *	XXX stop() doesn't do the right thing.
			 *	XXX Inline the task_suspend because we
			 *	XXX have to diddle Unix state in the
			 *	XXX middle of it.
			 */
			task = p->task;
			hold = FALSE;
			task_lock(task);
			if ((task->user_stop_count)++ == 0)
				hold = TRUE;
			task_unlock(task);

			if (hold) {
				(void) task_hold(task);
				sig_lock_to_wait(p);
				(void) task_dowait(task, TRUE);
				thread_hold(current_thread());
			}
			else {
				sig_lock_to_wait(p);
			}
			p->p_stat = SSTOP;
			wakeup((caddr_t)p->p_pptr);
			thread_block();
			sig_wait_to_lock(p);
			/*
			 * The bump of the suspend count is needed because 
			 * task_resume() doesn't ignore the current thread 
			 * as task_suspend() does.  Increasing the count
			 * ensures that the current thread's suspend count will
			 * be correct after the task_resume().
			*/
			thread_lock(cur_thread);
			cur_thread->suspend_count++;
			thread_unlock(cur_thread);
			task_resume(task);

			/*
			 *	We get here only if task
			 *	is continued or killed.  Kill condition
			 *	is signaled by adding NSIG to p_cursig.
			 *	Pass original p_cursig as exit value in
			 *	this case.
			 */
			if (p->p_cursig > NSIG) {
				/* If a system call is in progress, return to
				 * the system call so that specific cleanup
				 * can be performed.  syscall() will later
				 * look at p_cursig and call exit() if > NSIG.
				 */
				if (p->p_flag & SSYSCALL &&
				    p->task->thread_count == 1) {
					clear_wait(current_thread(),
						   THREAD_INTERRUPTED, FALSE);
					sig_lock_to_exit(p);
					return (TRUE);
				}
				/*
				 *	Wait event may still be outstanding;
				 *	clear it, since sig_lock_to_exit will
				 *	wait.
				 */
				clear_wait(current_thread(),
					THREAD_INTERRUPTED,
					FALSE);
				sig_lock_to_exit(p);
				exit(p, p->p_cursig - NSIG);
			}

			/*
			 *	We may have to quit
			 */
			if (thread_should_halt(current_thread())) {
				p->p_cursig = 0;
				SIGQ_FREE(u.u_curinfo);
				u.u_cursig = 0;
				sig_unlock(p);
				return(TRUE);
			}

			/*
			 * If the traced bit got turned off,
			 * then put the signal taken above back into p_sig
			 * and go back up to the top to rescan signals. This
			 * ensures that p_sig* and u_signal are consistent.
			 */
			if ((p->p_flag&STRC) == 0) {
				if (mask & threadmask) {
					/*
					 * sig_lock was already taken
					 * at the start of issig().
					 */
					u.u_sig |= mask;
					SIGQ_REQUEUE(&u.u_sigqueue,
						     u.u_curinfo);
					u.u_cursig = 0;
				} else {
					s = splhigh();
					p->p_sig |= mask;
					SIGQ_REQUEUE(&p->p_sigqueue,
						     u.u_curinfo);
					splx(s);
				}
				p->p_cursig = 0;
				continue;
			}

			/*
			 * If parent wants us to take the signal,
			 * then it will leave it in p->p_cursig;
			 * otherwise we just look for signals again.
			 */
			sig = p->p_cursig;
			if (sig == 0)
				continue;

			/*
			 * If signal is being masked put it back
			 * into p_sig and look for other signals.
			 */
			mask = sigmask(sig);
			if (p->p_sigmask & mask) {
				s = splhigh();
				if (mask & threadmask) {
					/* sig_lock is already taken */
					u.u_sig |= mask;
					SIGQ_REQUEUE(&u.u_sigqueue,
						     u.u_curinfo);
					u.u_cursig = 0;
				} else {
					s = splhigh();
					p->p_sig |= mask;
					SIGQ_REQUEUE(&p->p_sigqueue,
						     u.u_curinfo);
					splx(s);
				}
				p->p_cursig = 0;
				continue;
			}
		}

		switch ((int)signal_disposition(sig)) {
		case (int) SIG_DFL:
			/*
			 * Don't take default actions on system processes.
			 */
			if (p->p_ppid == 0) {
				u.u_cursig = 0;
				p->p_cursig = 0;
				SIGQ_FREE(u.u_curinfo);
				break;
			}
			if (mask & stopsigmask) {
				if (p->p_flag&STRC)
					continue;
				/*
				 * Insert test here for process in orphaned
				 * process group -- signal should be discarded
				 */
				/* register wait info before sending signal */
				p->p_wcode = CLD_STOPPED;
				p->p_xstat = sig;

				if ((p->p_pptr->p_flag & SNOCLDSTOP) == 0) {
					child_siginfo.si_signo = SIGCHLD;
					child_siginfo.si_pid = p->p_pid;
					child_siginfo.si_code = CLD_STOPPED;
					child_siginfo.si_status = sig;
					child_siginfo.si_errno = 0;
					psignal_info(p->p_pptr, SIGCHLD,
						     &child_siginfo);
				}

				/*
				 * Default signal action for stop signals is 
				 * being performed now, so time to clear
				 * p->p_cursig and free the sigqueue struct 
				 * for this signal.
				 *
				 * All stop signals are proc structure signals,
				 * so don't need to worry about cleaning out
				 * u.u_cursig/info.
				 */
				p->p_cursig = 0;
				SIGQ_FREE(u.u_curinfo);
				stop(p);
				sig_lock_to_wait(p);
				thread_block();
				sig_wait_to_lock(p);
				/*
				 *	We may have to quit
				 */
				if (thread_should_halt(current_thread())) {
					sig_unlock(p);
					return(TRUE);
				}
				continue;
			} else if (mask & defaultignmask) {
				/*
				 * Except for SIGCONT, shouldn't get here,
				 * since ignored signals were masked out earlier.
				 * Default action is to ignore; drop it.
				 */

				/*
				 * Performing a signal ignore action for 
				 * signal, so time to clear p->p_cursig 
				 * and free the sigqueue struct 
				 * for this signal.
				 */
				p->p_cursig = 0;
				SIGQ_FREE(u.u_curinfo);
				u.u_cursig = 0;
				continue;		/* == ignore */
			} else {
				goto send;
			}
			/*NOTREACHED*/

		case ((int)SIG_IGN):
			/*
			 * Default signal action being performed now, 
			 * so time to clear p->p_cursig and free the 
			 * sigqueue struct for this signal.
			 */
			p->p_cursig = 0;
			SIGQ_FREE(u.u_curinfo);
			u.u_cursig = 0;
			continue;

		default:
			/*
			 * This signal has an action, let
			 * psig process it.
			 */
			goto send;
		}
		/*NOTREACHED*/
	}
	/*
	 * Didn't find a signal to send.
	 */

	/*
	 * At this point, u_ and p_cursig and u_curinfo should
	 * have already been cleared out, because the signal was handled
	 * here in issig() (e.g., it is being ignored, etc.).  Check
	 * that this is true and cleanup if not.
	 */
	if (p->p_cursig || u.u_cursig || u.u_curinfo) {
		p->p_cursig = 0;
		SIGQ_FREE(u.u_curinfo);
		u.u_cursig = 0;
	}
	sig_unlock(p);
	return (0);

send:
	/*
	 * Let psig process the signal.
	 */
	sig_unlock(p);
	return (sig);
}

/*
 * Put the argument process into the stopped
 * state and notify the parent via wakeup.
 * Signals are handled elsewhere.
 */
stop(p)
	register struct proc *p;
{

	ASSERT(syscall_on_master());
	/*
	 *	Call special task_suspend routine,
	 *	because this routine is called from interrupts
	 *	(psignal) and cannot sleep.
	 */
	(void) task_suspend_nowait(p->task);	/*XXX*/

	p->p_stat = SSTOP;
	wakeup((caddr_t)p->p_pptr);
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 * The signal bit has already been cleared by issig,
 * and the current signal number stored in p->p_cursig.
 */
psig()
{
	register struct proc *p = u.u_procp;
	register int sig;
	sigset_t mask, block_mask, returnmask;
	register sig_t action;
	register thread_t th = current_thread();
	int s;
	register sigqueue_t sigqueue;
	register k_siginfo_t *siginfo_p;
	int ssig = NULL;

	/* 
	 * Traced process marked for exit.
	 */
	if ((unsigned long)p->p_cursig > NSIG)
		return;

	/*
	 *	This must be called on master cpu
	 */
	if (cpu_number() != master_cpu)
		panic("psig not on master");

	/*      Keep repeating this as long as there are signals that
	 *	need delivering
	 */
	do {
		/*
		 *	Try for the signal lock.  Don't proceed further if we
		 *	are already supposed to halt.
		 */
		sig_lock(p);
		sig = p->p_cursig;

		/*
		 * The /proc signal tracing code goes here
		 */

	     /* if PR_DSTOP is set, it means we got here from a fault, and
	      * that PIOCRUN with PRSTOP set was invoked.  We stop on the
	      * signal; the next PIOCRUN causes the "normal" signal behavior.
	      */
	     if(p->p_pr_qflags & PRFS_TRACING) {
		 register task_t t = current_task();

		/* first, clear the indication that the current signal was
		 * forced by PIOCSSIG.
		 */
		ssig = NULL;

		/* if the signal was forced by PIOCSSIG, do not trace on it;
		 * set ssig to indicate that the signal was forced; and
		 * clear the pr_ssig field in the task procfs structure.
		 */
		 if(t->procfs.pr_ssig == sig) {
			ssig = 1;
			t->procfs.pr_ssig = NULL;
		 }
		 else if((PR_DSTOP & t->procfs.pr_flags) )
		 {
			unix_master();
			t->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			t->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			t->procfs.pr_why = PR_REQUESTED;
			t->procfs.pr_what = sig;
			t->procfs.pr_tid = th;
			wakeup(&(t->procfs));
			sig_unlock(p);
			task_suspend(t);
			unix_release();
			sig_lock(p);
		 }

		/*
		 * Check the task and thread signal tracing flag
		 */
		 else if((t->procfs.pr_qflags & PRFS_SIGNAL) ||
			   (th->t_procfs.pr_qflags & PRFS_SIGNAL)) {

			/*
			 * Check if the current signal is to be traced at the
			 * process level, by anding p->p_cursig with the
			 * process signal trace mask. Actually, set
			 * sig = p->p_cursig, the original code will reload it.
			 * If it is to be traced at the task level, give up
			 * the signal lock set pr_flags, set pr_why, set
			 * pr_what, wakeup processes, stop the thread, relock
			 * on return and give up the signal lock
			 */
			if (prismember(&(t->procfs.pr_sigtrace), sig)) {
				unix_master();
				t->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
				t->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
				t->procfs.pr_why = PR_SIGNALLED;
				t->procfs.pr_what = sig;
				t->procfs.pr_tid = th;
				wakeup(&t->procfs);
				sig_unlock(p);
				task_suspend(t);
				unix_release();
				sig_lock(p);
#ifdef mips
				if (th->pcb->trapcause == CAUSEEXEC)
					th->pcb->trapcause = 0;
#endif /* mips */
			}
			/*
			 * If the signal is not to be traced at the process
			 * level, check if it is a thread level signal that
			 * this thread should trace.
			 * convert the signal to a bit mask
			 * if the signal is for this thread, and it is to be
			 * traced set pr_flags, set pr_why, set pr_what, wakeup
			 * processes, stop the thread, and relock on return 
			 */
			if (prismember(&(th->t_procfs.pr_sigtrace),sig)) {
				th->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
				th->t_procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
				th->t_procfs.pr_why = PR_SIGNALLED;
				th->t_procfs.pr_what = sig;
				t->procfs.pr_tid = th;
				wakeup(&t->procfs);
				thread_suspend(th);
				sig_unlock(p);
				thread_block();
				sig_lock(p);
			}
			/*
			 * At this point, the debugger can stop the signal
			 * from being sent to the current thread by doing an
			 * ioctl to clear p_cursig and uu_cursig. So reget
			 * sig from cursig.
			 */
		  }
		/* If we were stopped on a signal trace, and PIOCSSIG was used
		 * to change the current signal, set ssig and clear the
		 * indication that the signal was forced by PIOCSSIG.
		 */
		if( p->p_cursig != (char)sig) {
			ssig = 1;
			t->procfs.pr_ssig = NULL;
		}
			sig = p->p_cursig;
		}
		/*
		 * This is the end of the new /proc signal tracing code.
		 */

		mask = sigmask(sig);

		/*
		 *	If another thread got here first (sig == 0) or this is
		 *	a thread signal for another thread, bail out.
		 */
		if ((sig == 0) || ((mask & threadmask) && (sig != u.u_cursig))) 
		{
			sig_unlock(p);
			/*
			 * If we should halt, do so now.
			 */
			while(thread_should_halt(th))
				thread_halt_self();
			continue;
		}
		
		sigqueue = u.u_curinfo;
		if ((p->p_siginfo & mask) && sigqueue)
			siginfo_p = &sigqueue->siginfo;
		else
			siginfo_p = NULL;

		/* sig holds signal, sigqueue hold sigqueue */
		p->p_cursig = 0;
		u.u_curinfo = NULL;

		if (sigmask(sig) & threadmask)
			u.u_cursig = 0;

		action = signal_disposition(sig);
		if (action != SIG_DFL) {
			if (action == SIG_IGN || ((p->p_sigmask & mask)
						  && !ssig))
				panic("psig action");
			/*
			 * Set the new mask value and also defer further
			 * occurences of this signal (unless we're simulating
			 * the old signal facilities).
			 */

			s = splhigh();
			/*
			 * If SA_NODEFER was set for this signal, set up to
			 * not block this signal while in the handler.
			 * Otherwise, copy mask to block_mask for later
			 * use.
			 */
			if (u.u_signodefer & mask)
				block_mask = 0;
			else
				block_mask = mask;

			/*
			 * If SA_RESETHAND is set for this signal, clear
			 * out all the handler state for this signal.
			 * NOTE: do not unblock signal in handler -- that
			 * functionality is reserved for SA_NODEFER below.
			 */
			if (u.u_sigresethand & mask && sig != SIGILL
			    && sig != SIGTRAP && sig != SIGPWR) {
				/*
				 * Reset this signal handler back 
				 * to the default.
				 */
				signal_disposition(sig) = SIG_DFL;
				/*
				 * Clear out remaining signal state.
				 */
				p->p_sigcatch &= ~mask;
				p->p_siginfo &= ~mask;
				u.u_sigresethand &= ~mask;
				u.u_signodefer &= ~mask;
				u.u_sigonstack &= ~mask;
				u.u_sigintr |= mask;
				/*
				 * If signal is SIGCHLD must clear process
				 * actions for that signal.
				 */
				if (sig == SIGCHLD)
					p->p_flag &= ~(SNOCLDWAIT|SNOCLDSTOP);
			}

			/*
			 * Special case: user has done a sigpause.  Here the
			 * current mask is not of interest, but rather the
			 * mask from before the sigpause is what we want 
			 * restored after the signal processing is completed.
			 */
			if (p->p_flag & SOMASK) {
				returnmask = u.u_oldmask;
				p->p_flag &= ~SOMASK;
			} else
				returnmask = p->p_sigmask;

			p->p_sigmask |= u.u_sigmask[sig] | block_mask;

			/*
			 *	Fix up the signal state and unlock before
			 *	we send the signal.
			 */
			sig_unlock(p);
			u.u_ru.ru_nsignals++;
			(void) spl0();
			sendsig(action, sig, returnmask, siginfo_p);
			SIGQ_FREE(sigqueue);
			return;
		}
		U_HANDY_LOCK();
		u.u_acflag.fi_flag |= AXSIG;
		U_HANDY_UNLOCK();
		SIGQ_FREE(sigqueue);
		switch (sig) {
			/*
			 *  The new signal code for multiple threads makes it
			 *  possible for a multi-threaded task to get here (a
			 *  thread that didn't originally process a "stop"
			 *  signal notices that cursig is set), therefore, we
			 *  must handle this.
			 */
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
		case SIGSTOP:
			sig_unlock(p);
			continue;
			
		case SIGILL:
		case SIGIOT:
		case SIGBUS:
		case SIGQUIT:
		case SIGTRAP:
		case SIGEMT:
		case SIGFPE:
		case SIGSEGV:
		case SIGSYS:
		case SIGXCPU:
		case SIGXFSZ:
			u.u_sig = sig;
			/*
			 *	Indicate that we are about to exit.
			 *	disables all further signal processing for p.
			 */
			sig_lock_to_exit(p);
			if (core() == 0)
				sig |= WCOREFLAG;
			break;
		default:
			sig_lock_to_exit(p);
		}
		exit(p, W_EXITCODE(0, sig));
		/* NOTREACHED */
	} while (ISSIG (p));
}

#ifdef	i386
#	define USER_STACK_POINTER	UESP
#else
#if defined(mips) || defined(__alpha)
#	define USER_STACK_POINTER	EF_SP
#else
#	define	USER_STACK_POINTER	SP
#endif /* mips or alpha */
#endif /* i386 */
/*
 * Create a core image on the file "core"
 * If you are looking for protection glitches,
 * there are probably a wealth of them here
 * when this occurs to a suid command.
 *
 */

int coreprint = 0;	/* XXX Just for debugging */

core()
{
	register struct vnode *vp;
	struct vattr vattr;
	int error = 0;
	register struct nameidata *ndp = &u.u_nd;
	struct proc	        *p = u.u_procp;
	enum vtype	        type;
	struct ucred	        *cred;
	register vm_map_entry_t	me;
	struct core_filehdr     filehdr;
	struct core_scnhdr      scnh;
	struct core_regs        core_regs;
	uint                    scn_foff;   /* section header file offset */
	uint                    data_foff;  /* data file offset */
	int                     nscns;
	int                     i;
	vm_offset_t             tsp;		/* thread's stack pointer */
	register task_t         task;
	register queue_head_t   *list;
	register thread_t       thread, cur_thread, prev_thread;
#ifdef __alpha
	int			pgbytes = ALPHA_PGBYTES;
#else
	int			pgbytes = MIPS_PGBYTES;
#endif /* __alpha */
	int			vecsize;
	char			vec[1];

	ASSERT(syscall_on_master());
	if (coreprint)	{ /* XXX Just for debugging */
#if BIN_COMPAT
	      if(u.u_compat_mod) {
		struct compat_mod *cm_ptr = u.u_compat_mod;
	
		printf("(%s) pid %d uid %d -  bad %s svc %d - dumped core\n",
			u.u_comm, u.u_procp -> p_pid, p->p_ruid, 
			cm_ptr->cm_name, u.u_code);
	      } else
#endif /*BIN_COMPAT*/
	    if( u.u_sig == SIGSYS )
		printf("(%s) pid %d uid %d -  bad svc %d - dumped core\n",
			u.u_comm, u.u_procp -> p_pid, p->p_ruid, u.u_code);
	    else
		printf("(%s) pid %d uid %d dumped core\n",
			u.u_comm, u.u_procp -> p_pid, p->p_ruid);
	}

	PROC_LOCK(p);
	if (p->p_flag & SXONLY) {
		PROC_UNLOCK(p);
		return (-1);	/* Expected error code is discarded by caller */
	}
	crfree(u.u_cred);
	u.u_cred = p->p_rcred;
	crhold(p->p_rcred);
	p->p_rcred->cr_uid = p->p_ruid;
	p->p_rcred->cr_gid = p->p_rgid;
	PROC_UNLOCK(p);

	if (ctob(UPAGES+u.u_dsize+u.u_ssize) >=
	    u.u_rlimit[RLIMIT_CORE].rlim_cur)
		return (-1);   /* Expected error code is discarded by caller */
	/*
	 *	Make sure all registers, etc. are in pcb so they get
	 *	into core file.
	 */
	task = current_task();
	cur_thread = current_thread();
	pcb_synch(cur_thread);

	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = "core";
	error = vn_open(ndp, FCREAT|FWRITE, 0600);
	if (error)
		return (error);
	cred = u.u_cred;

	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VREG) {
		error = EFAULT;
		goto out;
	}
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error || vattr.va_nlink != 1) {
		if (error == 0)
			error = EFAULT;
		goto out;
	}

	vattr_null(&vattr);
	vattr.va_size = 0;
	VOP_SETATTR(vp, &vattr, cred, error);
	U_HANDY_LOCK();
	u.u_acflag.fi_flag |= ACORE;
	U_HANDY_UNLOCK();

	for(i = 0; i < MAXCOMLEN && u.u_comm[i] != 0; i++)
		filehdr.name[i] = u.u_comm[i];
	filehdr.name[i] = 0;
	filehdr.nscns = 0;
	bcopy("Core", filehdr.magic, 4);
	filehdr.version = CORE_VERS;
	filehdr.nthreads = task->thread_count;
	filehdr.signo = u.u_sig;
	filehdr.tid = cur_thread->thread_self;
	if(error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&filehdr, 
			sizeof(struct core_filehdr), (off_t)0, 
			UIO_SYSSPACE, IO_UNIT, 
			ndp->ni_cred, (int *)0))
		goto out;

	nscns = ((pgbytes - sizeof(struct core_filehdr)) /
			    sizeof(struct core_scnhdr)) - 1;

	scn_foff = sizeof(struct core_filehdr);

	/* exc frame, pc, fp regs and csr */
	scnh.scntype = SCNREGS;
	scnh.c_u.tid = cur_thread->thread_self;
	scnh.vaddr = (void *)0;
	scnh.size = sizeof(struct core_regs);
	bcopy((vm_offset_t)(cur_thread)->kernel_stack 
	/*
	 * changes to location of exception frame due to 
	 * new kstack orientation. 
	 */
#ifdef mips
			+KERNEL_STACK_START_OFFSET
#else
			+KERNEL_STACK_SIZE
#endif /* mips */
			- EF_SIZE, 
	      core_regs.ef_regs, EF_SIZE);
#ifdef __alpha
	/* save off current USP into exception frame for use by the 
	   debugger in analysis of programe coredump */
	core_regs.ef_regs[EF_SP] = mfpr_usp(); 
#endif /* __alpha */

	bcopy((vm_offset_t)(thread_pcb(cur_thread))->pcb_fpregs, 
	      core_regs.fp_regs, sizeof(core_regs.fp_regs));
#ifndef __alpha
	core_regs.fp_csr = (thread_pcb(cur_thread))->pcb_fpc_csr;
#endif
	/* write the exception frame and regs to the core */
	if(error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&core_regs, 
			   scnh.size, (scnh.scnptr = PAGE_SIZE), 
			   UIO_SYSSPACE, IO_UNIT,
			   ndp->ni_cred, (int *)0))
		goto out;

	/* now write excpt frame scn headr */
	if(error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&scnh,
			   sizeof(struct core_scnhdr), scn_foff,
			   UIO_SYSSPACE, IO_UNIT,
			   ndp->ni_cred, (int *)0))
		goto out;

	filehdr.nscns++;
	scnh.scnptr += sizeof (struct core_regs);
	scn_foff += sizeof (struct core_scnhdr);

	/*
	 * We have dumped the registers for the offending thread.
	 * Now dump the registers for the remaining threads in the task.
	 */
	list = &task->thread_list;
	prev_thread = THREAD_NULL;
	thread = (thread_t) queue_first(list);
	while (!queue_end(list, (queue_entry_t) thread)) {
		if (thread != cur_thread) {
			scnh.c_u.tid = thread->thread_self;

			bcopy((vm_offset_t)(thread)->kernel_stack + 
#ifdef mips
			      + KERNEL_STACK_START_OFFSET
#else
			      + KERNEL_STACK_SIZE
#endif /* mips */
			      - EF_SIZE,
			      core_regs.ef_regs, EF_SIZE);
#ifdef __alpha
			/*
			 * save off current USP into exception frame for use 
			 * by the debugger in analysis of program coredump
			 */
			core_regs.ef_regs[EF_SP] = thread->pcb->pcb_usp; 
#endif /* __alpha */
			bcopy((vm_offset_t)(thread_pcb(thread))->pcb_fpregs, 
			      core_regs.fp_regs, sizeof(core_regs.fp_regs));
#ifndef __alpha
			core_regs.fp_csr = (thread_pcb(thread))->pcb_fpc_csr;
#endif
			/* write the excp frame and regs to the core */
			if (error = vn_rdwr(UIO_WRITE, vp, 
					    (caddr_t)&core_regs, 
					    scnh.size, scnh.scnptr, 
					    UIO_SYSSPACE, IO_UNIT,
					    ndp->ni_cred, (int *)0))
				goto out;

			/* handle section header overflow */
			if (--nscns == 0) {
				if (error = write_ovfl_scn(vp, ndp,
							   &scn_foff,
							   &scnh.scnptr))
					break;
				filehdr.nscns++;
				nscns = (pgbytes / 
					 sizeof (struct core_scnhdr)) - 1;
			}
			/* write exception frame section header */
			if (error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&scnh,
					    sizeof(struct core_scnhdr), 
					    scn_foff,
					    UIO_SYSSPACE, IO_UNIT,
					    ndp->ni_cred, (int *)0))
				goto out;
			filehdr.nscns++;

			scnh.scnptr += sizeof (struct core_regs);
			scn_foff += sizeof (struct core_scnhdr);
		}
		thread = (thread_t) queue_next(&thread->thread_list);
	}

	/*
	 * core file format
	 * page 0	- headers
	 * page 1	- registers
	 * page 2...n	- data
	 *
	 * If there are enough threads, the register page might overflow,
	 * which would make the data region start at a page higher
	 * than .2
	 */
	/*
	 * changes to location of exception frame due to new kstack
	 * orientation. old value commented below:
	 *              data_foff = round_page(scnh.scnptr);
	 */
	data_foff = 2*PAGE_SIZE;
	/*
	 * Save the current thread's user stack pointer.  It is used to
	 * determine if a memory region contains the stack.
	 */
#ifdef __alpha
	tsp = trunc_page(mfpr_usp());
#else
	tsp = trunc_page(u.u_ar0[USER_STACK_POINTER]);
#endif

	for(me = vm_map_first_entry(task->map); 
	    me != vm_map_to_entry(task->map);
	    me = me->vme_next) {

		vecsize = 1;
		(*me->vme_core)(me, 0, vec, &vecsize);	
		if (vecsize == -1) continue;
		
		/* handle section header overflow */
		if(--nscns == 0) {
			if(error = write_ovfl_scn(vp, ndp,&scn_foff,&data_foff))
				break;
			filehdr.nscns++;
			nscns = (pgbytes / sizeof(struct core_scnhdr))- 1;
		}

		filehdr.nscns++;
		scnh.c_u.prot = me->vme_protection;
		scnh.vaddr = (void *)me->vme_start;
		scnh.size = me->vme_end - me->vme_start;

		/*
		 * text is RO, only write the section header.
		 *
		 * Also, if this region is backed by the core vnode, we
		 * might deadlock later.  Do not write it.  Instead,
		 * mark it as TEXT(for lack of a better identifier)
		 * so that core file readers will ignore this section.
		 */
		if((me->vme_protection & VM_PROT_WRITE) == 0 ||
						vp_is_bobject(me, vp)) {
			scnh.scntype = SCNTEXT;
			scnh.scnptr = 0;
		} else {
			/*
			 * If this region contains the current thread's
			 * stack pointer, identify it as a stack section.
			 * We do it only for the current thread as it is
			 * readily available.  There is no apparent benefit
			 * for doing this, so don't bother with other threads.
			 * Not only that, the other stacks may also lie in
			 * this entry (unlike OSF/1), and differentating
			 * between them is rather difficult.
			 */
			if(tsp < me->vme_end && tsp >= me->vme_start)
				scnh.scntype = SCNSTACK;
			else {
#if	0
				/*
				 * Not sure why we need this empty section.
				 * dbx doesn't care about the SCNDATA, so
				 * why do it?
				 */
				scnh.scntype = SCNDATA;
				scnh.scnptr = 0;
				
				/* handle section header overflow */
				if(--nscns == 0) {
					if(error = write_ovfl_scn(vp, ndp,
								  &scn_foff,
								  &data_foff))
						break;
					filehdr.nscns++;
					nscns = (pgbytes / 
						 sizeof(struct core_scnhdr))-1;
				}
				
				/* write the data section header */
				if(error = vn_rdwr(UIO_WRITE, vp,
						   (caddr_t)&scnh,
						   sizeof(struct core_scnhdr),
						   scn_foff, UIO_SYSSPACE,
						   IO_UNIT, ndp->ni_cred,
						   (int *)0))
					break;
				scn_foff += sizeof(struct core_scnhdr);
				filehdr.nscns++;
#endif
				/*** 
				 *** This has to be changed to run down
				 *** all of the regions in each data map entry 
				 *** and build a region header for each 
				 *** when (if) we are able to support regions..
				 ***/
				/* build the region header */
				scnh.scntype = SCNRGN;
			}
			scnh.scnptr = data_foff;
			/* write data out */
			if(error = vn_rdwr(UIO_WRITE, vp, (caddr_t)scnh.vaddr,
					   scnh.size, data_foff,
					   UIO_USERSPACE, IO_UNIT,
					   ndp->ni_cred, (int *)0))
				break;
			data_foff += scnh.size;
		}
		/* write the section header */
		if(error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&scnh,
				   sizeof(struct core_scnhdr), scn_foff,
				   UIO_SYSSPACE, IO_UNIT,
				   ndp->ni_cred, (int *)0))
			break;
		scn_foff += sizeof(struct core_scnhdr);
	}

	if(error == 0) {
		error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&filehdr, 
				sizeof(struct core_filehdr), (off_t)0, 
				UIO_SYSSPACE, IO_UNIT, 
				ndp->ni_cred, (int *)0);
	}
	if (error == 0)
		VOP_FSYNC(vp, 0, ndp->ni_cred, MNT_WAIT, error);
out:
	VOP_CLOSE(vp, FCREAT|FWRITE, u.u_cred, error);
	VN_LOCK(vp);
       	vp->v_wrcnt--;
	VN_UNLOCK(vp);
	/*
         * Unmount can race close as follows:
         *      - unmount flushes the buffer cache
         *      + active vnode writer writes into buf cache
         *      + active vnode writer closes vnode
         *      - unmount verifies no active vnodes on fs
         *      - unmount allows fs to be unmounted, leaving
         *      bogus buffer in memory.
         * By taking the associated mount structure's lookup
         * lock, the close will wait for the unmount to find
         * this active vnode.
         * N.B.  Only do this for vnodes attached to filesystems.
         */
        if (vp->v_mount != DEADMOUNT) {
                struct mount *mp = vp->v_mount;

                MOUNT_LOOKUP_START(mp);
                vrele(vp);
                MOUNT_LOOKUP_DONE(mp);
        } else
                vrele(vp);
	return (error);
}

write_ovfl_scn(vp, ndp, soff, doff)
	struct vnode *vp;
	struct nameidata *ndp;
	ulong   soff;
	ulong   doff;
{
	struct core_scnhdr ov;
	int error;

	ov.scntype = SCNOVFL;
	ov.c_u.prot = 0;
	ov.vaddr = (void *)0;
	ov.size = 0;

	if(doff % PAGE_SIZE)
		doff = round_page(doff);
	ov.scnptr = doff;
	if((error = vn_rdwr(UIO_WRITE, vp, (caddr_t)&ov, 
			   sizeof(struct core_scnhdr), soff,
			   UIO_SYSSPACE, IO_UNIT,ndp->ni_cred,(int *)0) == 0)){
		soff = doff;
		doff += PAGE_SIZE;
	}
	return(error);
}

cansignal(p, q, signo)
register struct proc *p;
register struct proc *q;
long signo;
{
	register struct ucred	*pcr;
	register struct ucred	*qcr;
	struct session 		*psess;
	struct session 		*qsess;
	uid_t			pruid, qruid, qsvuid;
	int			can;

	PROC_LOCK(p);
	pcr = p->p_rcred;
	psess = p->p_session;
	pruid = p->p_ruid;
	PROC_UNLOCK(p);
	PROC_LOCK(q);
	qsvuid = q->p_svuid;
	qsess = q->p_session;
	qruid = q->p_ruid;
	PROC_UNLOCK(q);
	/*
	 * N.B.  Session check should probably be under some
	 * as-yet-undefined pgrp lock.			XXXX
	 */
#if     SEC_BASE
	can = (pruid == qruid ||
	       pcr->cr_uid == qruid ||
	       pruid == qsvuid ||
	       pcr->cr_uid == qsvuid ||
	       ((signo) == SIGCONT &&
	       psess == qsess) ||
	       (check_privileges ? privileged(SEC_KILL, 0)
				 : (pcr->cr_uid == 0)));
#else
	can = (pcr->cr_uid == 0 ||
	       pruid == qruid ||
	       pcr->cr_uid == qruid ||
	       pruid == qsvuid ||
	       pcr->cr_uid == qsvuid ||
	       ((signo) == SIGCONT &&
	       psess == qsess));
#endif
	return(can);
}


#if	UNIX_LOCKS
/*
 * There are a few special cases where psignal must be called from
 * interrupt context.  Because psignal remains unparallelized, it takes
 * unix_master, which can't be done from interrupt context.  We use a
 * dedicated thread to process psignal calls for these special cases.
 *
 * All of these special cases arise from slave_hardclock, so none of
 * this code is needed on a uniprocessor.
 */

typedef struct psig_event {
	queue_chain_t	pe_chain;
	struct proc	*pe_proc;
	int		pe_sig;
} psig_event;

#define	NPSIGEVENTS	100
psig_event	psig_store[NPSIGEVENTS];
mpqueue_head_t	psig_queue;
mpqueue_head_t	psig_free_queue;


psignal_int(p, sig)
register struct proc *p;
register long sig;
{
	psig_event	*pe;

	mpdequeue1 (&psig_free_queue, &pe, QNOWAIT);
	if (pe == 0)
		panic("psignal_special:  no free psig events\n");
	pe->pe_proc = p;
	pe->pe_sig = sig;
	mpenqueue1 (&psig_queue, pe);
}

psignal_thread()
{
	thread_t		thread;
	psig_event		*pe;
	register struct proc	*p;
	register int		sig;

	thread = current_thread();
	thread_swappable(thread, FALSE);
	/*
	 * RT_SCHED: Replace hardcoded priority.  It is alright to overwrite
	 * sched_pri here, because the thread is not in a run queue at the
	 * time it is overwritten.
	 */
	thread->priority = thread->sched_pri = BASEPRI_PSIGNAL;
	thread->bound_processor = master_processor;
	thread_block();			/* come back on master processor */

	mpqueue_init(&psig_queue);
	mpqueue_init(&psig_free_queue);
	for (pe = psig_store; pe < &psig_store[NPSIGEVENTS]; ++pe)
		mpenqueue1 (&psig_free_queue, pe);

	for (;;) {
		mpdequeue1 (&psig_queue, &pe, QWAIT);
		p = pe->pe_proc;
		sig = pe->pe_sig;
		mpenqueue1 (&psig_free_queue, pe);
		psignal(p, sig);
	}
}
#endif
