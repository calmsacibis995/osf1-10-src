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
static char	*sccsid = "@(#)$RCSfile: trap.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:28 $";
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
 *	File extensively rewritten from vax/trap.c for ns32000.
 */ 
/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_idebug.h>
#include <cpus.h>
#include <mach_ldebug.h>

#include <mmax/psl.h>
#include <mmax/reg.h>
#include <mmax/pte.h>

#include <sys/unix_defs.h>

#include <sys/secdefines.h>

#if SEC_BASE
#include <sys/security.h>
#include <sys/lock.h>
#include <sys/audit.h>
#endif /* SEC_BASE */


#include <sys/param.h>
#include <kern/sched.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/acct.h>
#include <sys/kernel.h>
#include <kern/assert.h>

#ifndef	SYSCALLTRACE
#define SYSCALLTRACE
#endif
#ifdef	SYSCALLTRACE
#include <syscalls.c>
#endif

#include <mmax/cpu.h>
#include <mmax/mmu.h>
#include <mmax/mtpr.h>
#include <mmax/cpudefs.h>
#include <mmax/trap.h>
#if	MMAX_XPC || MMAX_APC
#include <mmax/icu.h>
#endif

#include <kern/ast.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <vm/pmap.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <mach/vm_param.h>
#include <mach/kern_return.h>
#include <kern/parallel.h>
#include <mach/exception.h>
#include <builtin/ux_exception.h>

#define USER	040		/* user-mode flag added to type */
#define TOCONS	0x1		/* for putchar() */
#define TOTTY	0x2
#define TOLOG	0x4

#if	MMAX_IDEBUG
#define	MMAX_IASSERT(clause)	ASSERT(clause)
#else
#define	MMAX_IASSERT(clause)
#endif

struct	sysent	sysent[];
int	nsysent;

#ifdef	SYSCALLTRACE
int syscalltrace = 0;
int syscalltracing = 0;
#endif

#define	MAXSYSCALLS	300			
int	scmax = MAXSYSCALLS;		/* max number of system call counts */
int	sccount[MAXSYSCALLS];		/* system call counts */

char	*trap_type[] = {
	"Ast trap",				/* T_AST */
	"Read fault",				/* T_ABT_READ */
	"Write fault",				/* T_ABT_WRITE */
	"Floating point trap",			/* T_FPU */
	"Illegal (privileged) instruction",	/* T_ILL */
	"System call",				/* T_SVC (not used) */
	"Divide by zero",			/* T_DVZ */
	"Flag trap",				/* T_FLG */
	"Breakpoint",				/* T_BPT */
	"Trace trap",				/* T_TRC */
	"Undefined Instruction",		/* T_UND */
#if	MMAX_XPC || MMAX_APC
	"Bus Error",				/* T_BUSERR -- APC only */
	"ICU Race",				/* T_ICURACE */
	"Unexpected DUART Trap",		/* T_DUARTFLT */
	"Illegal Cascade Fault",		/* T_CASCADE */
	"Undefined Trap",			/* T_UNDEFLT */
	"\"Recoverable\" Bus Error",		/* T_RBE -- XPC only */
	"Non-Recoverable Bus Error",		/* T_NBE -- XPC only */
	"Integer Overflow Trap",		/* T_OVF -- 532 */
	"Debug Trap",				/* T_DBG -- 532 */
#endif
};

#define TRAP_TYPES	(sizeof trap_type / sizeof trap_type[0])

int	trap_trace = 0;

int     mmu_fix_count = 0;

#ifdef	MMUBUG
/*
 *	MMU problem can cause a write fault to be reported as a read fault.
 *	As a workaround if the same read fault occurs twice in a row
 *	report it as a write fault.  Following data structure holds
 *	fault data.  Fault data is invalidated during translation buffer
 *	invalidation.
 */

struct mmubug_vm_fault	last_fault[NCPUS];

int     mmu_bug_count = 0;

/*
 *	Also, mmu somehow manages to report read access to get sb
 *	register (part of rett) as a write access.  Catch this by checking
 *	pc of fault.  master_rett and master_reti are labels attached to
 *	the rett and reti instructions in locore.s.
 *	These are also used to catch invalid mod tables in all cases.
 */
int     mmu_ret_count = 0;

#endif	/* MMUBUG */

extern int master_rett;
extern int master_reti;

#if	MMAX_XPC
#define	RDRMW	MSR_ST_T_RDRMW
#endif

#if	MMAX_APC
#define	RDRMW	ASR_ST_T_RDRMW
unsigned long	lastber_pc[NCPUS];
#endif



/*
 * Called from the trap handler when a processor trap occurs.
 */
/*ARGSUSED*/
trap(type, code, sp, r7, r6, r5, r4, r3, r2, r1, r0, fp, pc, psl)
	unsigned code;
	int sp, type;
	int pc, psl;
{
	int	*locr0 = ((int *)&r0);
	int 	i;
#if	MMAX_XPC || MMAX_APC
	unsigned usrflg;
	int cpustatus;
#endif
	struct proc *p;
	struct timeval	syst;
	vm_offset_t	recover;
	kern_return_t	ret;
	vm_map_t	map;
	int		exception = 0;
	int		exc_code = 0;
	int	 	exc_subcode = 0;
	int		sig;
#if	!MMAX_APC
	register int	cpuid;
#endif
	register thread_t	th;
	register struct uthread 	*uthreadp;
	register struct utask	*utaskp;
#if	MACH_LDEBUG 
	int	old_lock;
	int	old_addr[MAX_LOCK];
	struct slock_debug old_slck;
	int	cpu;
#endif

	MMAX_IASSERT(icu_ints_on());
	MMAX_IASSERT(ints_on());

	th = current_thread();
	if (th != THREAD_NULL) {
		uthreadp = th->u_address.uthread;
		utaskp = th->u_address.utask;
		syst = utaskp->uu_ru.ru_stime;
	
		if (USERMODE(locr0[PS])) {
			type |= USER;
			uthreadp->uu_ar0 = locr0;
		}
#if	MACH_LDEBUG
		old_lock = th->lock_count;
		for (i = 0; i < MAX_LOCK; i++)
			old_addr[i] = th->lock_addr[i];
#endif
	}
#if	MACH_LDEBUG
	cpu = cpu_number();
	old_slck = slck_dbg[cpu];
#endif
	if(trap_trace)
		printf("trap type %d, code = %x, pc = %x\n", type, code, pc);

#if	MMAX_XPC || MMAX_APC
	usrflg = type;		/* Save flags from abt handler */
	cpustatus = (type & TF_STATUS) >> TFO_STATUS;
	type &= TF_TYPE;	/* Keep just read/write indicator */
#endif

	switch (type) {

	default:
		type &= ~USER;
		if ((unsigned)type < TRAP_TYPES)
			panic(trap_type[type]);
		printf("trap:  unexpected trap type %d\n", type);
		panic("trap");

	case T_ILL+USER:	/* Illegal instr. trap */
	case T_UND+USER:	/* Undefined instr. trap */
		exception = EXC_BAD_INSTRUCTION;
		exc_code = type & ~USER;
		break;

	case T_FLG+USER:	/* flag trap */
		exception = EXC_SOFTWARE;
		exc_code = EXC_NS32K_FLG;
		break;

	case T_AST:
	case T_AST+USER:
		astoff();
		/*
		 *	Check for termination.
		 */
		while(thread_should_halt(th))
			thread_halt_self();

		simple_lock(&utaskp->uu_prof.pr_lock);
		if ((utaskp->uu_procp->p_flag & SOWEUPC) &&
		    utaskp->uu_prof.pr_scale) {
			addupc(pc, &utaskp->uu_prof, 1);
			utaskp->uu_procp->p_flag &= ~SOWEUPC;
		}
		simple_unlock(&utaskp->uu_prof.pr_lock);
		break;

	case T_FPU+USER:
		if (code == EXC_NS32K_FPU_ILLEGAL ||
		    code == EXC_NS32K_FPU_INVALID)
			exception = EXC_BAD_INSTRUCTION;
		else
			exception = EXC_ARITHMETIC;
		exc_code = EXC_NS32K_FPU;
		exc_subcode = code;
		break;

	case T_DVZ+USER:
		exception = EXC_ARITHMETIC;
		exc_code = EXC_NS32K_DVZ;
		break;

	case T_ABT_READ:
	case T_ABT_READ+USER:
	case T_ABT_WRITE:
	case T_ABT_WRITE+USER:
		/*
		 *	Figure out which map to fault on.
		 */
		if (th == THREAD_NULL) {
			/*
			 *	Page fault very early in boot sequence.
			 *	Must be on kernel map.
			 */
			map = kernel_map;
		}
		else {
#if	MMAX_XPC || MMAX_APC
			if ((usrflg & TFV_USER) != 0) {
				map = th->task->map; /* User */
			}
			else {
				if (th->task->kernel_vm_space)
					map = th->task->map; /* Kernel task */
				else
					map = kernel_map; /* Kernel */
			}
#endif	
#if	MMAX_DPC
			if ((code & EIA_PTB1) != 0) {
				map = th->task->map;
			}
			else {
				if (th->task->kernel_vm_space)
					map = th->task->map;
				else
					map = kernel_map;
			}
#endif
		}

#if	MMAX_XPC || MMAX_APC
		/*
		 *	If this is a fault for the first part of a RMW
		 *      cycle, then it's really a write fault.
		 */

		if (((type & ~USER) == T_ABT_READ) && (cpustatus == RDRMW)) {
		        mmu_fix_count++;
			type &= USER; /* save this bit, clear fault type */
			type |= T_ABT_WRITE; /* make it a write fault */
		}
#endif

#ifdef MMUBUG
		/*
		 *	If this is a read fault and is identical to the
		 *	last fault on this cpu, make it a write fault.
		 */
		cpuid = cpu_number();
		if (((type & ~USER) == T_ABT_READ) &&
		    LAST_FAULT_IDENTICAL(cpuid, map, type, code) ) {
		        mmu_bug_count++;
			type &= USER; /* save this bit, clear fault type */
			type |= T_ABT_WRITE; /* make it a write fault */
		}

		/*
		 *	Save this fault in last_fault[cpuid]
		 */
		LAST_FAULT_SAVE(cpuid, map, type, code);
#endif
Retryfault:

		MMAX_IASSERT(icu_ints_on());
		MMAX_IASSERT(ints_on());

		exc_code = vm_fault(map,
#if	MMAX_XPC || MMAX_APC
			trunc_page((vm_offset_t) code),
#endif
#if	MMAX_DPC
			trunc_page((vm_offset_t) (code & ~EIA_PTB1)),
#endif
			(type & ~USER) == T_ABT_READ ? VM_PROT_READ :
				VM_PROT_READ|VM_PROT_WRITE, FALSE);
		/*
		 *	If a thread was in a kernel page fault and
		 *	needs to be interrupted/terminated, pretend
		 *	fault failed to kick off cleanup code.
		 */

		if (thread_should_halt(th) && th->recover != NULL)
			exc_code = KERN_FAILURE;
		
		if (exc_code != KERN_SUCCESS) {
			MMAX_IASSERT(icu_ints_on());
			MMAX_IASSERT(ints_on());
			/*
			 *	Check for unusual kernel faults.
			 */
			if ((type & USER) == 0 && th != THREAD_NULL) {
			    /*
			     *	Kernel tasks may fault on either
			     *	their own map or the kernel_map.
			     *	If own map fails, try kernel.
			     */
			    if (th->task->kernel_vm_space &&
				map != kernel_map) {
				    map = kernel_map;
				    goto Retryfault;
			    }
			    /*
			     *	If bad fault was in copyin/copyout routines,
			     *	we want to return control to the failure
			     *	return address in th->recover.
			     */
			    if (th->recover != NULL) {
				    locr0[PC] = th->recover;
#if	MACH_LDEBUG
				    goto lockck;
#else
				    return;
#endif
			    }
			    /*
			     *	If rett or reti to user mode faults,
			     *	user may be in big trouble.
			     */
			    if ((pc == (int)&master_rett) ||
				(pc == (int)&master_reti)) {
#ifdef	MMUBUG
				    /*
				     *	If this is a write access to
				     *	load sb, retry as a read.
				     */
				    if ((exc_code == KERN_PROTECTION_FAILURE)
					&& ((type & ~USER) == T_ABT_WRITE)) {
					    mmu_ret_count++;
					    type &= USER;
					    type |= T_ABT_READ;
					    goto Retryfault;
				    }
#endif
				    /*
				     *	User loses big; without a mod table,
				     *	execution in user mode is impossible!
				     */
				    sig = SIGSEGV;
				    if (thread_signal_disposition(sig))
					uthreadp->uu_tsignal[sig] = SIG_DFL;
				    else
					utaskp->uu_signal[sig] = SIG_DFL;
				    sig = sigmask(sig);
				    p = utaskp->uu_procp;
				    p->p_sigignore &= ~sig;
				    p->p_sigcatch &= ~sig;
				    p->p_sigmask &= ~sig;
			    }
			}
			exception = EXC_BAD_ACCESS;
			exc_subcode = code;
			break;
		}
#if	MACH_LDEBUG
		goto lockck;
#else
		return;
#endif

	case T_BPT+USER:	/* bpt instruction fault */
	case T_TRC+USER:	/* trace trap */
		locr0[PS] &= ~(PSR_T << 16);
		exception = EXC_BREAKPOINT;
		exc_code = type & ~USER;
		break;

#if	MMAX_APC
	case T_BUSERR:		/* Bus error - possibly recoverable */
		{
		int cpuid;
		apcerr_t err_sts;
		int fatal;
		cpuid = cpu_number();
		err_sts = * (apcerr_t *) APCREG_ERR;
		if (err_sts .f.e_rdbus)
			fatal = 0;
		else if (!err_sts.f.e_lock)
			fatal = 1;
		else if ((unsigned long)locr0[PC] != lastber_pc[cpuid]) {
			lastber_pc[cpuid] = (unsigned long)locr0[PC];
			fatal = 0;
		} else
			fatal = 1;

		printf ("*** Bus Error ***   Status = 0x%x\n", err_sts.l);
		if (fatal)
		  panic ("Bus Error");
		  /* NOTREACHED */
#if	MACH_LDEBUG 
		goto lockck;
#else
		return;
#endif
		}

	case T_BUSERR + USER:	/* Bus error - possibly recoverable */
		{
		apcerr_t err_sts;
		err_sts = * (apcerr_t *) APCREG_ERR;
		if (!err_sts.f.e_rdbus) {
			unix_master();
			psignal(utaskp->uu_procp, SIGKILL);
			unix_release();
		}
		printf("*** Bus Error   Status = 0x%x", err_sts.l);
		break;
		}
#endif	/* MMAX_APC */

#if	MMAX_XPC
	case T_OVF+USER:
		exception = EXC_ARITHMETIC;
		exc_code = EXC_NS32K_FPU;
		exc_subcode = EXC_NS32K_FPU_INTOVF;
		break;
#endif
	}

	if (th == THREAD_NULL) {
		printf("Null thread: ");
		panic(trap_type[type]);
	}
	/*
	 *	Send exception if there is one, and process signals.
	 */
	if (exception != 0) {
		locr0[PS] &= ~(PSR_P << 16);	/* cancel pending trace */
		thread_doexception(th, exception, exc_code,
			exc_subcode);
	}

	p = utaskp->uu_procp;
	if (CHECK_SIGNALS(p, th, uthreadp)) {
		unix_master();
		if (p->p_cursig || issig())
			psig();
		unix_release();
	}

	p->p_pri = p->p_usrpri;

	if (USERMODE(locr0[PS])) {
		/*
		 *	If trapped from user mode, handle possible
		 *	rescheduling.
		 */
		if (csw_needed(th,current_processor())) {
			utaskp->uu_ru.ru_nivcsw++;
			thread_block();
		}
	}
		
	if (utaskp->uu_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &utaskp->uu_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks) {
			simple_lock(&utaskp->uu_prof.pr_lock);
			addupc(locr0[PC], &utaskp->uu_prof, ticks);
			simple_unlock(&utaskp->uu_prof.pr_lock);
		}
	}
#if	MACH_LDEBUG
lockck:
	if (th != THREAD_NULL) {
		ASSERT(th->lock_count == old_lock);
		for (i = 0; i < MAX_LOCK; i++)
			ASSERT(th->lock_addr[i] == old_addr[i]);
	}
	cpu = cpu_number();
	for (i = 0; i < MAX_LOCK; i++) {
		if (slck_dbg[cpu].addr[i] != old_slck.addr[i]) {
		printf("trap: slck_dbg[%d].addr[%d] (%d) != old.addr (%d)\n",
			cpu, i, slck_dbg[cpu].addr[i], old_slck.addr[i]);
		panic("trap: slck debug addr");
		}
	}
	if (slck_dbg[cpu].count != old_slck.count) {
		printf("trap: slck_dbg[%d].count (%d) != old.count (%d)\n",
			cpu, slck_dbg[cpu].count, old_slck.count);
		panic("trap: slck debug count");
	}
			
	return;
#endif
}

#define NUMSYSCALLARGS 8

/*
 * Called from the trap handler when a system call occurs.
 */
/*ARGSUSED*/
syscall(type, code, sp, r7, r6, r5, r4, r3, r2, r1, r0, fp, pc, psl)
	int	*r1;
	caddr_t	pc;
	int	psl;
	register unsigned code;
{
	int	 *locr0 = ((int *)&r0);
	caddr_t params;
	int i;
	struct sysent *callp;
	struct proc *p;
	caddr_t opc;
	struct timeval syst;
	thread_t	th;
	register struct uthread 	*uthreadp;
	register struct utask	*utaskp;
	struct args {
		int i[NUMSYSCALLARGS];
	} args;
	int rval[2];
	register int error;


	MMAX_IASSERT(icu_ints_on());
	MMAX_IASSERT(ints_on());

	LASSERT(slck_dbg[cpu_number()].count == 0);
	LASSERT(current_thread()->lock_count == 0);

	th = current_thread();
	uthreadp = th->u_address.uthread;
	utaskp = th->u_address.utask;
	p = utaskp->uu_procp;

	code = r0;		/* R0 has syscall number on Multimax.*/
	syst = utaskp->uu_ru.ru_stime;
	if (!USERMODE(psl))
		panic ("Kernal mode syscall");
	uthreadp->uu_ar0 = locr0;
#ifdef	SYSCALLTRACE
	if (syscalltrace && (syscalltrace < 0 || syscalltrace == p->p_pid))
		syscalltracing = 1;
#endif
	if (code == 139) {			/* XXX 4.2 COMPATIBILITY */
#ifdef	SYSCALLTRACE
		if (syscalltracing) {
			printf ("Pid %d: osigcleanup\n",
				p->p_pid);
		}
#endif
#if SEC_BASE
		audit_setup(callp - sysent);
#endif /* SEC_BASE */
		unix_master();			/* XXX 4.2 COMPATIBILITY */
		error = osigcleanup();		/* XXX 4.2 COMPATIBILITY */
		goto done;			/* XXX 4.2 COMPATIBILITY */
	}					/* XXX 4.2 COMPATIBILITY */

	params = (caddr_t) r1;
	error = 0;
	opc = pc;
	pc += 1;

	if (code == 0) {
		/*
		 * Indirect system call, first param is sys call number
		 */
		code = fuword(params);
		params += NBPW;
	}
	callp = (code >= nsysent) ? &sysent[63] : &sysent[code];

#if SEC_BASE
	audit_setup(callp - sysent);
#endif /* SEC_BASE */

	if ((i = callp->sy_narg * sizeof (int)) &&
	    (error = copyin(params, &args, (u_int)i)) != 0) {
		locr0[R0] = error;
		locr0[PS] |= (PSR_C << 16);	/* carry bit */
		goto done;
	}
	rval[0] = 0;
	rval[1] = locr0[R1];

	/*
	 * Acquire a fresh copy of the credentials for
	 * the duration of the system call.
	 */
	cr_threadinit(th);

	if (callp->sy_parallel == 0)
		unix_master();

	MMAX_IASSERT(icu_ints_on());
	MMAX_IASSERT(ints_on());

#ifdef	SYSCALLTRACE
	if (syscalltracing)
		print_syscall(code, callp, (int *)&args);
#endif
	error = (*callp->sy_call)(p, &args, rval);

#ifdef	SYSCALLTRACE
	if (syscalltracing)
		syscalltracing = 0;
#endif

	if (code < scmax)
		sccount[code]++;		/* count system calls */

#if SEC_BASE
	 AUDIT_GENERATE_RECORD(&args, error, rval);
#endif

	MMAX_IASSERT(icu_ints_on());
	MMAX_IASSERT(ints_on());

	if (error == ERESTART)
		pc = opc;
	else if (error != EJUSTRETURN) {
		if (error) {
			locr0[R0] = error;
			locr0[PS] |= (PSR_C << 16);	/* carry bit */
#if SEC_BASE
			/*
			 * security() passes back a secondary error code
			 * that allows for more meaningful error messages
			 * than just "permission denied" etc.
			 */

			if (callp == &sysent[AUDIT_SECURITY])
			    locr0[R0] |= (AIP->si_error << SEC_ERRNO_SHIFT);
#endif
		} else {
			locr0[R0] = rval[0];
			locr0[R1] = rval[1];
			locr0[PS] &= ~(PSR_C << 16);
		}
	} 
	/* else if (error == EJUSTRETURN) */
		/* nothing to do */
done:
	p = utaskp->uu_procp;
	if (CHECK_SIGNALS(p, th, uthreadp)) {
		unix_master();
		if (p->p_cursig || issig())
			psig();
		unix_release();
	}
	p->p_pri = p->p_usrpri;

	/*
	 *	It is OK to go back on a slave.  139 test is for osigcleanup
	 *	which must execute on master, but doesn't use full-blown
	 *	handler (callp is never set up).
	 */
	if (th->unix_lock >= 0)
		unix_release();

	if (csw_needed(th, current_processor())) {
		utaskp->uu_ru.ru_nivcsw++;
		thread_block();
	}
	if (utaskp->uu_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &utaskp->uu_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks)
			addupc(locr0[PC], &utaskp->uu_prof, ticks);
	}
#if	UNIX_LOCKS && NCPUS > 1
	unix_release_force();
#endif
	LASSERT(slck_dbg[cpu_number()].count == 0);
	LASSERT(current_thread()->lock_count == 0);
}

MMAX_load_init_program(dummy1, dummy2, sp, r7, r6, r5, r4, r3, r2, r1, r0, fp, pc, psl)
	int sp, r0;
	int pc, psl;
{
	int	*locr0 = ((int *)&r0);

	u.u_ar0 = locr0;
	load_init_program();
}

/*
 * nonexistent system call-- signal process (may want to handle it)
 * flag error if process won't see signal immediately
 * Q: should we do that all the time ??
 */
nosys()
{
	int error = 0;
	if (signal_disposition(SIGSYS) == SIG_IGN ||
	    signal_disposition(SIGSYS) == SIG_HOLD)
		error = EINVAL;
	thread_doexception(current_thread(), EXC_SOFTWARE,
		EXC_UNIX_BAD_SYSCALL, 0);
	return (error);
}

/*
 * hw_isr - handle all the ISRs caused by the hardware on the XPC/APC.
 *
 * ARGUMENTS:
 *
 * type - the interrupt type (the interrupt number).
 *
 * istkp - pointer to the interrupt stack frame at the time the trap
 *	occurred, as set by trap.s.
 *
 * USAGE:
 *	This routine is called to handle all the hardware-related
 *	interrupts.  Since MACH has no error logging facility, an
 *	appropriate message is printed on the console, and then
 *      either the system panics (fatal error) or we return and
 *      continue (non-fatal errors).
 */

#define	HW_SOFT			0x1
#define	HW_IMPOSSIBLE		0x2
#define	HW_PANIC		0x4

#if	MMAX_XPC
	static char *herr_str[] = {
		"Protocol/Logic Error",
		"Write Data Parity Error/No Read Data Back",
		"Request Not Accepted/STALL",
		"No Grant"
	};

	static char *serr_str[] = {
		"Protocol/Logic Error",
		"Write Data Parity Error",
		"Retry",
		"No Error"
	};

/* ARGSUSED */
hw_isr(type, istkp, sp, r7, r6, r5, r4, r3, r2, r1, r0, fp, pc, psl)
long type;
{
	xpcerr_t	error;

	error.l = GETCPUERR;
	switch(type) {
	    case ICU_HARD_NBI_BIT:
		hw_log(HW_IMPOSSIBLE|HW_PANIC, "Hard NanoBus Error",
			 herr_str[error.f.e_hrd_err_stat],
			 type, error, pc, psl);
		break;
	    case ICU_POWERFAIL_BIT:
		hw_log(HW_PANIC, "Power Failure", 0, type, error, pc, psl);
		break;
	    case ICU_DESTSEL_PAR_BIT:
		hw_log(HW_SOFT, "DESTSEL Parity Error", 0, type, error,
		       pc, psl);
		break;
	    case ICU_BTAG_BIT:
		hw_log(HW_SOFT, "BTAG State Error", 0, type, error, pc, psl);
		break;
	    case ICU_BTAG_CACHE_BIT:
		hw_log(HW_SOFT, "BTAG/Cache Inconsistency", 0, type, error,
		       pc, psl);
		break;
	    case ICU_RCV_ADDR_PAR_BIT:
		hw_log(HW_SOFT, "Receive Address Parity Error", 0,
		       type, error, pc, psl);
		break;
	    case ICU_CACHE_PAR_BIT:
		cache_par_isr(type, pc, psl);
		break;
	    case ICU_BTAG_TAG_PAR_BIT:
		hw_log(HW_SOFT, "BTAG Tag Parity Error", 0,
		       type, error, pc, psl);
		break;
	    case ICU_SOFT_NBI_BIT:
		hw_log(HW_SOFT, "Soft NanoBus Error",
		       serr_str[error.f.e_sft_err_stat], type, error, pc, psl);
		break;
	    case ICU_VB_ERR_BIT:
		vecbus_err_isr (pc);
		break;
	    default:
		hw_log(HW_IMPOSSIBLE|HW_PANIC, "Unknown Fatal Interrupt",
		       0, type, error, pc, psl);
		break;
	}
}

cache_par_isr(type, pc, psl)
{
	xpcerr_t	error;
	char		buff[256], *err_p;
	void		sprintf();

	error.l = GETCPUERR;

	err_p = buff;
	*err_p = '\0';

	sprintf(err_p, "%s %s%s Cycle%s.\n",
		(int)(error.f.e_rdbus ? "External Read" : "Cache"),
		(int)(error.f.e_cpuerr ? "CPU or MMU" : "BTAG"),
		(int)(error.f.e_lock ? "" : " Interlock"),
		(int)(error.f.e_datape ? "" : " w/Data Parity Error"));
	if (error.f.e_cache_state_pe)
		sprintf(err_p + strlen(err_p),
			"\tCache State Parity Error, State: 0x%x;  Bank %d\n",
			(int)error.f.e_cache_state,
			(int)error.f.e_cache_addr);
	if (error.f.e_ctagpe != 3)
		sprintf(err_p + strlen(err_p),
			"\tCache Tag Parity Error on field(s)%s%s\n",
			(int)(error.f.e_ctagpe & 1 ? "" : " 0"),
			(int)(error.f.e_ctagpe & 2 ? "" : " 1"));
	sprintf(err_p + strlen(err_p),
		"\tCTAG Compare Status:  Bits 7:0-%sMatch, Bits 13:8-%sMatch",
		(int)(error.f.e_ctagcmp & 1 ? "No " : ""),
		(int)(error.f.e_ctagcmp & 2 ? "No " : ""));

	hw_log(HW_SOFT, "Cache Parity Error", err_p, type, error, pc, psl);
}

vecbus_err_isr(type, pc, psl)
{
	char	buf[256], *s;
	void	sprintf();
	u_short	vb_err;

	s = buf;
	*s = '\0';

	vb_err = *XPCVB_ERROR;

	if (vb_err & XPCVB_ERR_ILL_LMP_VCT)
		sprintf(s,"\tVector Bus Illegal Lamp Vector");
	if (vb_err & XPCVB_ERR_PARITY_ERR)
		sprintf(s + strlen(s),"\tVector Bus Parity Error");
	if (vb_err & XPCVB_ERR_OUT_OF_SYNC)
		sprintf(s + strlen(s),"\tVector Bus Out-of-Sync");
	if (vb_err & XPCVB_ERR_FIFO_OVERFLOW)
		sprintf(s + strlen(s),"\tVector Bus FIFO Overflow");
	hw_log(HW_SOFT, "Vector Bus Error", s, type, vb_err, pc, psl);
}
#endif	/* MMAX_XPC */

#if	MMAX_APC
hw_isr(type, istkp, sp, r7, r6, r5, r4, r3, r2, r1, r0, fp, pc, psl)
long type;
{
	int	error = *APCREG_ERR;

	switch(type) {
	    case ICU_VB_NAK_BIT:
		hw_log(HW_PANIC, "VectorBus NAK", 0, type, error, pc, psl);
		break;
	    case ICU_POWERFAIL_BIT:
		hw_log(HW_PANIC, "Power Failure", 0, type, error, pc, psl);
		break;
	    case ICU_HARD_NBI0_BIT:
	    case ICU_HARD_NBI1_BIT:
	    case ICU_HARD_NBI2_BIT:
	    case ICU_HARD_NBI3_BIT:
		hw_log(HW_PANIC, "Hard NanoBus", 0, type, error, pc, psl);
		break;
	    case ICU_DESTSEL_PAR_BIT:
		hw_log(HW_PANIC, "DESTSEL Parity Error", 0, type, error,
		       pc, psl);
		break;
	    case ICU_VB_PAR_BIT:
		hw_log(HW_PANIC, "VectorBus Parity Error", 0, type, error,
		       pc, psl);
		break;
	    case ICU_VB_OUT_OF_SYNC_BIT:
		hw_log(HW_PANIC, "VectorBus Out of Synch", 0, type, error,
		       pc, psl);
		break;
	    case ICU_SYSNMI_BIT:
	    case ICU_TIMER_M_BIT:
	    case ICU_VECBUS_BIT:
	    case ICU_DUART_BIT:
	    case ICU_TIMER_S_BIT:
	    default:
		hw_log(HW_IMPOSSIBLE|HW_PANIC, "Unrecognized Interrupt", 0,
		       type, error, pc, psl);
		break;
	    case ICU_SOFT_NBI0_BIT:
	    case ICU_SOFT_NBI1_BIT:
	    case ICU_SOFT_NBI2_BIT:
		hw_log(HW_SOFT, "Soft NanoBus", 0, type, error, pc, psl);
		break;
	    case ICU_RCV_ADDR_PAR4_BIT:
	    case ICU_RCV_ADDR_PAR0_BIT:
	    case ICU_RCV_ADDR_PAR1_BIT:
	    case ICU_RCV_ADDR_PAR2_BIT:
	    case ICU_RCV_ADDR_PAR3_BIT:
		hw_log(HW_SOFT, "Receive Address Parity", 0, type, error,
		       pc, psl);
		break;
	    case ICU_BTAG_PAR0_BIT:
	    case ICU_BTAG_PAR1_BIT:
		hw_log(HW_SOFT, "BTAG Parity Error", 0, type, error, pc, psl);
		break;
	    case ICU_CACHE_PAR_BIT:
		hw_log(HW_SOFT, "Cache Parity Error", 0, type, error, pc, psl);
		break;
	}
}
#endif	/* MMAX_APC */

#if	MMAX_XPC || MMAX_APC
hw_log(options, message, extra_message, type, error, pc, psl)
int	options;
char	*message;
char	*extra_message;
{
	int	cpuid;
	char	*p;

	cpuid = getcpuid();
	p = (options & HW_SOFT) ? "Warning about Hardware Condition" :
		(options & HW_IMPOSSIBLE) ? "\"Impossible\" Hardware Error" :
		(options & HW_PANIC) ? "Fatal Hardware Error" :
		"Hardware Error";

	printf("CPU %d:  %s:  %s\n", cpuid, p, message);
	if (extra_message)
		printf("\t%s\n", extra_message);
	printf("\ttype = 0x%x  error = 0x%x  pc = 0x%x  psl=0x%x\n",
	       type, error, pc, psl);
	if (options & HW_PANIC)
		panic("Hardware Error");
}
#endif	/* MMAX_XPC || MMAX_APC */

/*
 * Simple sprintf that understands only %% and %s options.
 */
void
sprintf(dest, fmt, args)
char	*dest, *fmt, *args;
{
	char	**stk_args, *string, *odest;

	odest = dest;
	stk_args = &args;
	for (; *fmt; ++fmt) {
		if (*fmt == '%') {
			if (*++fmt == '%') {
				*dest++ = '%';
				continue;
			}
			else
				if (*fmt != 's')
					panic("sprintf");
			string = *stk_args++;	/* assume stack direction */
			while (*string)
				*dest++ = *string++;
			continue;
		}
		*dest++ = *fmt;
	}
	*dest = '\0';
}

#if 0
blackflag(msg)
char	*msg;
{
	extern	unsigned int	mfree;
	extern	unsigned int	mbutl;
	extern	unsigned int	embutl;
	int	s;

	s = splimp();
	if(mfree && (mfree < mbutl || mfree > embutl))
		goto woops;

	mwalk(msg);
	if(((int)mfree & 0xff) != 0 && ((int)mfree & 0xff) != 0x80) {
woops:
		printf("mfree = 0x%x, mbutl = 0x%x, embutl = 0x%x\n", mfree, mbutl, embutl);
		if((int)msg < 100) {
			printf("blackflag %d\n", msg);
			panic("Blackflag");
		} else
			panic(msg);
	}
	splx(s);
}
#endif /* 0 */


#ifdef	SYSCALLTRACE
print_syscall(syscode, callp, args)
unsigned syscode;
struct sysent	*callp;
int	args[];
{
	register int i;
	char *cp;
	struct thread	*th;
	struct uthread	*uthreadp;
	struct utask	*utaskp;

	th = current_thread();
	uthreadp = th->u_address.uthread;
	utaskp = th->u_address.utask;

	printf ("Pid %d: ", utaskp->uu_procp->p_pid);
	if (syscode >= nsysent)
		printf("0x%x", syscode);
	else
		printf("%s", syscallnames[syscode]);
	cp = "(";
	for (i= 0; i < callp->sy_narg; i++) {
		printf("%s%x", cp, args[i]);
		cp = ", ";
	}
	if (i)
		putchar(')', 1, 0);
	putchar('\n', 1, 0);
}
#endif
