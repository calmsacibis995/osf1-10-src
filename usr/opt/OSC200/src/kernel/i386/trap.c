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
static char	*sccsid = "@(#)$RCSfile: trap.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:20:18 $";
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

#include <cpus.h>
#include <mach_kdb.h>
#include <mach_rdb.h>

#include <i386/psl.h>
#include <i386/reg.h>
#include <i386/debug.h>
#include <i386/pmap.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <i386/trap.h>
#include <i386/fpreg.h>
#include <sys/acct.h>
#include <sys/kernel.h>
#include <kern/xpr.h>

/* #undef	SYSCALLTRACE */
#include <syscalltrace.h>

#include <kern/thread.h>
#include <kern/parallel.h>
#include <mach/thread_status.h>
#include <kern/task.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <mach/vm_param.h>
#include <mach/kern_return.h>
#include <kern/sched_prim.h>
#include <mach/exception.h>
#include <builtin/ux_exception.h>

#define USER	040		/* user-mode flag added to type */

struct	sysent	sysent[];
int	nsysent;

int syscalltrace = 0;

char	*trap_type[] = {
	"Divide error",
	"Debug trap",
	"NMI",
	"Breakpoint",
	"Overflow",
	"Bounds check",
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"(reserved)",
	"Invalid TSS",
	"Segmentation",
	"Stack bounds",
	"General protection",
	"Page fault",
	"(reserved)",
	"Coprocessor error"
};

#if	MACH_KDB
int	kdbinstalled = 1;
#else	/* MACH_KDB */
int	kdbinstalled = 0;
#endif  /* MACH_KDB */
#if	MACH_RDB 
int	rdbinstalled = 1;
#else	/* MACH_RDB */
int	rdbinstalled = 0;
#endif	/* MACH_RDB */

/*
 *  Need this accessible to the kernel debugger.
 */
int	TRAP_TYPES =	(sizeof trap_type / sizeof trap_type[0]);
int	show_pc = 0;
#if	MACH_KDB || MACH_RDB
extern int kdbactive;
extern int kdbtrapok;
extern int kdb_singlestep;
#endif	/* MACH_KDB || MACH_RDB */

#if MACH_KDB || MACH_RDB
int	kdb_user_debug;		/* flag to allow kernel debugging of user code */
#endif

/*
 *	Mach trap handler.  Called from locore when a processor trap
 *	occurs.
 */
/*ARGSUSED*/
trap(r0ptr)
	int *r0ptr;
{
	register int	*locr0 = r0ptr;
	register struct proc *p;
	register thread_t th;
	register struct uthread *uthreadp;
	register struct utask	*utaskp;
	struct timeval	syst;
	
	vm_map_t	map;
	vm_offset_t	va;
	kern_return_t	result;
	
	int	exc_type, exc_code, exc_subcode;
	
	int	save_error;
	int     type;
	int     code;
	int	s;
	u_int   pc;


	th = current_thread();
	type = locr0[TRAPNO];
	code = locr0[ERR];
	pc = locr0[EIP];
	va = (vm_offset_t)_cr2();
	
#if	SYSCALLTRACE
if (syscalltrace < -1)
printf("Ktrap: type %x, code %x, pc %x, ar0 %x, va %x\n",
	type, code, pc, locr0, va);
#endif	/* SYSCALLTRACE */

#define I386_UMODE	((USERMODE(locr0[CS])) || (locr0[EFL] & EFL_VM))
#if	MACH_KDB
	if (!I386_UMODE || kdbactive)
#else	MACH_KDB
	if (!I386_UMODE)
#endif	/* MACH_KDB */
	{
	    /*
	     *	Trap in system mode.  Only page-faults are valid,
	     *	and then only in special circumstances.
	     */

	    switch (type) {
		case T_ENOEXTFLT:	/* software floating trap */
		case T_NOEXTFLT:
			fpnoextflt(locr0);
			return;
	    	case T_EXTOVRFLT:
			fpextovrflt(locr0);
			return;
	    	case T_EXTERRFLT:
			fpexterrflt(locr0);
			return;
			systemdebugtrap(type,locr0);
			panic("The kernel just tried to do a fp op.");
		case T_BPTFLT:
		case T_SGLSTP:
			systemdebugtrap(type,locr0);
			return;
			
		case T_PGFLT:
		case T_SEGNPFLT:
			if (va >= VM_MIN_KERNEL_ADDRESS) {
				result = KERN_FAILURE;
				if (th->task->kernel_vm_space) {
					map = th->task->map;
					result = kernel_fault_in(map,va,code);
				}
				if (result != KERN_SUCCESS) {
					map = kernel_map;
					result = kernel_fault_in(map,va,code);
				}
			 } else {
				map = th->task->map;
				result = vm_fault(map, trunc_page(va),
					  (code & I386_PTE_KRW)
					    ? VM_PROT_READ|VM_PROT_WRITE
					    : VM_PROT_READ,
					  FALSE);
			}
			if (result == KERN_SUCCESS)
				return;
			{
				extern int	ALLOW_FAULT_START,
						ALLOW_FAULT_END,
						FAULT_ERROR;
				vm_offset_t	recover = th->recover;
#if	MACH_KDB
				if (kdbtrapok) {
					locr0[EIP] = (int)&FAULT_ERROR;
					return;
				} else
#endif	/* MACH_KDB */
				if (recover == (vm_offset_t)0) {
					if ((vm_offset_t)pc >
					    (vm_offset_t)&ALLOW_FAULT_START
					    && ((vm_offset_t)pc <
						(vm_offset_t)&ALLOW_FAULT_END))
					{
						locr0[EIP] = (int)&FAULT_ERROR;
						return;
					}
				} else {
					/* ???	locr0[EFL] &= ~PSL_FPD; */
					locr0[EIP] = (int)recover;
					return;
				}
			}
			printf("kernel page fault: va = %x, err = %x\n", va,
			       code & 0x7);
			/* Unanticipated page-fault errors in kernel
			   should not happen. */
			/* fall through */
		default:
#if	MACH_KDB
                        if (kdbinstalled && kdb(type, locr0, 1))
                                return;
#endif	/* MACH_KDB */
#if MACH_RDB			
			if( rdbinstalled)
				db_kdb(type, locr0, 0);	/* pass to debugger */
#endif /* MAH_RDB */
			printf("trap type %d, code = %x, pc = %x, cr2 = %x\n",
			       type, code, pc, va);
			print_all_registers(locr0);
			if ((unsigned) type < TRAP_TYPES)
				panic(trap_type[type]);
			else
				panic("trap");
			return;
		}
	}
	
	/*
	 *	Trap from user mode.
	 */
	
	p = u.u_procp;
	
	if (p) {
		syst = u.u_ru.ru_stime;
		u.u_ar0 = locr0;
	}
	
	exc_code = 0;
	exc_subcode = 0;
	
	switch (type) {
	case T_AST:
		astoff();
		/*
		 *	Check for termination.
		 */
		while(thread_should_halt(th))
			thread_halt_self();

		utaskp = th->u_address.utask;
		simple_lock(&utaskp->uu_prof.pr_lock);
		if ((utaskp->uu_procp->p_flag & SOWEUPC) &&
		    utaskp->uu_prof.pr_scale) {
			addupc(pc, &utaskp->uu_prof, 1);
			utaskp->uu_procp->p_flag &= ~SOWEUPC;
		}
		simple_unlock(&utaskp->uu_prof.pr_lock);
		goto out;

	case T_DIVERR:
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_I386_DIV;
		break;
		
	case T_SGLSTP:
#if MACH_KDB || MACH_RDB
		if (kdb_user_debug) {
			systemdebugtrap(type,locr0);
			return;
		}
#endif
		exc_type = EXC_BREAKPOINT;
		exc_code = EXC_I386_SGL;
		break;
	case T_BPTFLT:
#if MACH_KDB || MACH_RDB
		if (kdb_user_debug) {
			systemdebugtrap(type,locr0);
			return;
		}
#endif
		exc_type = EXC_BREAKPOINT;
		exc_code = EXC_I386_BPT;
		break;
		
	case T_STKFLT:
		if (locr0[EFL] & EFL_VM) {
			exc_type = EXC_BAD_INSTRUCTION;
			exc_code = EXC_I386_STKFLT;
			break;
		}

	case T_NMIFLT:
	case T_DBLFLT:
	case T_INVTSSFLT:
		printf("USER trap type %d, code = %x, pc = %x\n",
		       type, code, pc);
#if	MACH_KDB
                kdb(type, locr0, 1);
#endif	/* MACH_KDB */
		if ((unsigned) type < TRAP_TYPES)
			panic(trap_type[type]);
		else
			panic("trap");
		return;
	case T_INTOFLT:
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_I386_INTO;
		break;
		
	case T_BOUNDFLT:
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_I386_BOUND;
		break;
		
	case T_GPFLT:
		if (!(locr0[EFL] & EFL_VM))
			uprintf("General Protection Fault: trap type %d, code = %x, pc = %x, cr2 = %x\n",
			       type, code, pc, va);
	case T_INVOPFLT:
		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_I386_INVOP;
		break;

	case T_ENOEXTFLT:	/* software floating trap */
	case T_NOEXTFLT:

		if (!(locr0[EFL] & EFL_VM)) {
			fpnoextflt(locr0);
			goto out;
		} else {
			exc_type = EXC_ARITHMETIC;
			exc_code = EXC_I386_NOEXT;
			break;
		}

	case T_EXTOVRFLT:
		fpextovrflt(locr0);
		goto out;
		
	case T_PGFLT:
	case T_SEGNPFLT:
		map = th->task->map;
		result = vm_fault(map, trunc_page(va), 
				  (code & I386_PTE_KRW)
				    ? VM_PROT_READ|VM_PROT_WRITE
				    : VM_PROT_READ,
				  FALSE);
		
		if (result == KERN_SUCCESS) {
			return;
		}
		exc_type = EXC_BAD_ACCESS;
		exc_code = result;
		exc_subcode = va;
		break;
		
	case T_EXTERRFLT:
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_I386_EXTERR;
		break;
		
	case T_ENDPERR:
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_I386_EMERR;
		break;
		
	default:
		printf("trap(user): unknown trap 0x%x\n", type);
#if	MACH_KDB
                (void) kdb(type, locr0, 1);
#endif
		return;
	}
	if (show_pc && bcmp(u.u_comm, "sh", 2))
		uprintf("Utrap: in %s type %x, code %x, pc %x, ar0 %x, va %x\n",
			u.u_comm, type, code, pc, locr0, va);
	thread_doexception(th,
			   exc_type, exc_code, exc_subcode);

    out:
	if (p) {
		if (CHECK_SIGNALS(p, th,th->u_address.uthread)) {
			unix_master();
			if (p->p_cursig || issig()) {
				psig();
			}
			unix_release();
		}
	}
		
        if(csw_needed(th,current_processor())) {
                u.u_ru.ru_nivcsw++;
                thread_block();
        }

	if (u.u_prof.pr_scale) {
		int	ticks;
		struct timeval *tv = &u.u_ru.ru_stime;
			
		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			 (tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks)
			addupc(locr0[EIP], &u.u_prof, ticks);
	}
}

/* 
 * Handle a debug trap from system mode.  This can happen in one of 3 
 * ways:
 * 
 * 1 - A user process did a system call while single-stepping.  Our
 *     action is to set a flag in the pcb so that the user process
 *     will return to single-stepping once the system call is
 *     finished.
 * 2 - Someone is single-step debugging using kdb.  Our action is to 
 *     call kdb and let it deal with it.
 * 3 - We have hit a kernel breakpoint (set by kdb).  Again, our
 *     action is to call kdb.
 * 
 * For the first two cases, we can tell from dr6 that the CPU was 
 * single-stepping.  We distinguish case 1 from case 2 by checking a 
 * flag that kdb has set if it was single-stepping.  Cases 1 and 3 can 
 * happen at any time, so there aren't any global flags we can set to 
 * identify either of those cases.
 */

systemdebugtrap(type,locr0)
	int *locr0;			/* pointer to register stack */
{
	int dr6 = _dr6();
	extern int system_call;

	if (dr6 & DBG_BS) {
#if	MACH_KDB
		/* single stepping */
		if (kdb_singlestep) {
			kdb(T_SGLSTP, locr0, 0);
		} else
#endif
                {
			/* 
			 * Additional sanity check.  The trap should 
			 * have happened near the system call entry 
			 * point in locore.
			 */
			if (locr0[EIP] < (int)&system_call ||
			    locr0[EIP] > ((int)&system_call + 10))
				panic("bogus single step trap\n");
			current_thread()->pcb->pcb_flags |=
				PSF_SINGLESTEP;
			locr0[EFL] &= ~EFL_TF;
		}
	} else
#if	MACH_KDB
        if (kdbinstalled)
		kdb(type, locr0, 1); /* kernel bkpt. */
	else
#endif
#if	MACH_RDB
        if (rdbinstalled)
		db_kdb(type, locr0, 0); /* kernel bkpt. */
        else
#endif
		panic("breakpoint with no kernel debugger.");
}


/* 
 * Handle a page fault from system mode for a kernel page.  This gets 
 * a bit hairy because of copy-on-reference.
 * 
 * First treat the fault as a write fault.  If that works, we've
 * faulted in the page and we're done.  If that fails because of a
 * protection violation and the fault was really a read fault, fault
 * in the page as a read fault.  If this succeeds, call a special pmap 
 * routine to enable access to the page.
 * 
 * XXX The loop is in case another processor (on a multiprocessor)
 * steals the page before we can mark it present.  Note that this code
 * hasn't actually been tested on a multiprocessor.
 */
kern_return_t
kernel_fault_in(map, va, code)
	vm_map_t map;
	vm_offset_t va;
	int code;
{
	kern_return_t result;
	extern int cor_upgrades;	/* see pmap.c */

	va = trunc_page(va);

	if ((result = vm_fault(map, va, VM_PROT_READ|VM_PROT_WRITE,
			       FALSE)) == KERN_SUCCESS) {
		if (!(code & I386_PTE_KRW))
			cor_upgrades++;
		return(KERN_SUCCESS);
	}
	if (result != KERN_PROTECTION_FAILURE || (code & I386_PTE_KRW))
		return(result);

	/* It was a read fault, and we can't treat it as a write fault. */
	for (;;) {
		if ((result = vm_fault(map, va, VM_PROT_READ, FALSE))
		    != KERN_SUCCESS)
			return(result);
		if (pmap_mark_valid(va))
			break;
	}

	return(KERN_SUCCESS);
}
			
#define NUMSYSCALLARGS 8
/*
 * Called from locore when a system call occurs.
 */

#define MAXSYSCALLS     300
int scmax = MAXSYSCALLS;                /* max number of system call counts */
int sccount[MAXSYSCALLS];               /* system call counts */


/*ARGSUSED*/
syscall(locr0)
	register int *locr0;
{
	register unsigned code;
	register caddr_t params;		/* known to be r10 below */
	register int i;				/* known to be r9 below */
	register struct sysent *callp = &sysent[20];/* known to be getpid */
	register int	error;
	register struct uthread	*uthread;
	thread_t	t = current_thread();
	int		s;
	register struct proc *p;
	struct timeval syst;
	struct args {
		int j[NUMSYSCALLARGS];
	} args;
	int rval[2];

	code = (unsigned) locr0[EAX];
	syst = u.u_ru.ru_stime;
	uthread = t->u_address.uthread;
	if (!USERMODE(locr0[CS]))
		panic("syscall");
	uthread->uu_ar0 = locr0;
	if (t->pcb->pcb_flags & PSF_SINGLESTEP) {
		t->pcb->pcb_flags &= ~PSF_SINGLESTEP;
		locr0[EFL] |= EFL_TF;
	}

	if (code == 139) {			/* XXX 4.2 COMPATIBILITY */
		unix_master();
		error = osigcleanup();		/* XXX 4.2 COMPATIBILITY */
		goto done;			/* XXX 4.2 COMPATIBILITY */
	}					/* XXX 4.2 COMPATIBILITY */
	params = (caddr_t)locr0[UESP] + NBPW;
	error = 0;
	if (code == 0) {
		i = fuword(params);
		params += NBPW;
		code = (unsigned) i;
	}
	callp = (code >= nsysent) ? &sysent[63] : &sysent[code];

	if ((i = callp->sy_narg * sizeof (int)) &&
	    (error = copyin(params, (caddr_t)&args, (u_int)i)) != 0) {
		locr0[EAX] = error;
		locr0[EFL] |= EFL_CF;	/* carry bit */
		goto done;
	}
        rval[0] = 0;
        rval[1] = locr0[EDX];
#if	NCPUS > 1
	if (callp->sy_parallel == 0) {
		/*
		 *	System call must execute on master cpu
		 */
		if (t->unix_lock != -1)
			panic("syscall,unix_master(): unix_lock != -1");
		unix_master();		/* hold to master during syscall */
	}
#endif
	/*
	 * Acquire a fresh copy of the credentials for
	 * the duration of the system call.
	 */
	cr_threadinit(current_thread());
#if	SYSCALLTRACE
	if (syscalltrace && 
	    (syscalltrace == u.u_procp->p_pid || syscalltrace < 0)) {
#define i	ii
		register int i;
		char *cp;
		extern char *syscallnames[];

		if (code >= nsysent)
			uprintf("[P%d %s]0x%x",
				u.u_procp->p_pid,
				u.u_comm,
				code);
		else
			uprintf("[P%d %s]%s",
				u.u_procp->p_pid,
				u.u_comm,
				syscallnames[code]);
		cp = "(";
		for (i= 0; i < callp->sy_narg; i++) {
			uprintf("%s%x", cp, args.j[i]);
			cp = ", ";
		}
		if (i)
			cp = ")";
		else
			cp = "";
#undef	i
	}
#endif
	XPR(XPR_SYSCALLS, ("syscall start: %c%c%c%c%c%c\n",
		u.u_comm[0], u.u_comm[1], u.u_comm[2],
		u.u_comm[3], u.u_comm[4], u.u_comm[5]));
	XPR(XPR_SYSCALLS, 
	("syscall start %d: args=0x%x, 0x%x, 0x%x, 0x%x\n", 
		code, args.j[0], args.j[1], args.j[2],
		args.j[3], args.j[4]));

	error = (*callp->sy_call)(u.u_procp, &args, rval);

	if (code >= 0 && code < scmax)
		sccount[code]++;        /* count system calls */

	if (syscalltrace && 
	    (syscalltrace == u.u_procp->p_pid || syscalltrace < 0)) {
		if (error)
			uprintf(" = -1 [%d]\n", error);
		else
			uprintf(" = %x:%x\n", rval[0], rval[1]);
/*		DELAY(1*200000);*/
	}

	if (error == ERESTART)
		/* Back the pc up to before the lcall */
		locr0[EIP] -= 7;
	else if (error != EJUSTRETURN) {
		if (error) {
			locr0[EAX] = error;
			locr0[EFL] |= EFL_CF;	/* carry bit */
		} else {
			locr0[EAX] = rval[0];
			locr0[EDX] = rval[1];
			locr0[EFL] &= ~EFL_CF;
		}
	}
	/* else if (error == EJUSTRETURN) */
		/* nothing to do */
		
done:
	p = u.u_procp;
	XPR(XPR_SYSCALLS, 
		("syscall end: pid=%d, code=%d, rval1=%d, rval2=%d\n",
		u.u_procp->p_pid, code, rval[0], rval[1]));
	if (CHECK_SIGNALS(p, t, uthread)) {
		unix_master();
		if (p->p_cursig || issig()) {
			psig();
		}
		unix_release();
	}
	p->p_pri = p->p_usrpri;
#if	NCPUS > 1
	/*
	 *	It is OK to go back on a slave
	 */
	if (code == 139 || callp->sy_parallel == 0) { /***/

		unix_release();
		if (t->unix_lock != -1) {
			printf("syscall<%d>: released unix_lock D%d\n",
				u.u_procp-proc, t->unix_lock);
			panic("syscall: unix_lock != -1");
		}
	}
#endif

        if(csw_needed(current_thread(),current_processor())) {
                u.u_ru.ru_nivcsw++;
                thread_block();
	}

	if (u.u_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &u.u_ru.ru_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks) {
			simple_lock(&u.u_prof.pr_lock);
			addupc(locr0[EIP], &u.u_prof, ticks);
			simple_unlock(&u.u_prof.pr_lock);
		}
	}
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
	if (u.u_signal[SIGSYS] == SIG_IGN || u.u_signal[SIGSYS] == SIG_HOLD)
		error = EINVAL;
#if	MACH_EXCEPTION
	thread_doexception(current_thread(), EXC_SOFTWARE,
		EXC_UNIX_BAD_SYSCALL, 0);
#else	MACH_EXCEPTION
	psignal(p, SIGSYS);
#endif	/* MACH_EXCEPTION */
        return (error);
}

static int fpsw;

nofloat()
{
	_fninit();
	fpsw = _fstcw();
	fpsw &= ~(FPINV | FPZDIV | FPOVR | FPPC);
	fpsw |= FPSIG53 | FPIC;
	_fldcw(fpsw);
	return;
}


print_all_registers(sp)
int *sp;
{
	
	printf("\n");
	printf("SS %x, UESP %x, EFL %x, CS %x\n",sp[SS],
						 sp[UESP],
					 	 sp[EFL],
						 sp[CS]);
	printf("EIP %x, ERR, %x, TRAPNO %x, EAX %x\n",sp[EIP],
						      sp[ERR],
						      sp[TRAPNO],
						      sp[EAX]);
	printf("ECX %x, EDX %x, EBX %x, ESP %x EBP %x\n",sp[ECX],
							 sp[EDX],
							 sp[EBX],
							 sp[ESP],
							 sp[EBP]);
	printf("ESI %x, EDI %x, DS %x, ES %x\n",sp[ESI],
						sp[EDI],
						sp[DS],
						sp[ES]);
				
	printf("FS %x, GS %x\n",sp[FS],
				sp[GS]);
}
