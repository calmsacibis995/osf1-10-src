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
static char	*sccsid = "@(#)$RCSfile: trap.c,v $ $Revision: 1.2.3.9 $ (DEC) $Date: 1992/10/13 15:37:13 $";
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
/************************************************************************
 *
 *	(Relevant) Modification History: trap.c
 *
 * 07-Nov-91	Woodward
 *   change emulate instruction panic with a kill signal to the offending 
 *   process.
 *
 * 25-May-91    Scott Cranston
 *   Added 'savedefp' to make the exception frame pointer available in the
 *   namelist for dump analysis use by dbx and Cansta.
 *
 * 21-Apr-91	Ron Widyono
 *	Change kast_preemption, hack_preemption, original_preemption, pp_count,
 *	ast_was_on to rt_preempt_async, rt_preempt_syscall, rt_preempt_user,
 *	rt_preempt_pp.  Remove some syscall preemption debugging.
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
 * 14-Nov-89 -- sekhar
 * 	Fixes to turn profiling off when scale set to 1.
 *
 * 13-Oct-89 gmm
 *	smp changes. Access nofault, nofault_cause etc through cpudata.
 *	Changes for per processor tlbpids. Handle affinity correctly for
 *	system calls.
 *
 * 09-Jun-89 -- scott
 *	added audit support
 *
 * 09-Nov-1988  jaa
 *	allow process to turn on/off unaligned access messages
 *
 *************************************************************************/

int trap_debug = 0;


/*
#define TRACE(x) if (trap_debug) x
*/
#define TRACE(x)
#define	SHOW	0		/* warn about some strange user states */

#include <mach_nbc.h>
#include <mach_kdb.h>
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
#include <mach/exception.h>
#include <builtin/ux_exception.h>
#include <sys/param.h>			/* for CHECK_SIGNALS definition */
#include <sys/user.h>
#include <mach/machine/thread_status.h> /* for USER_REG defintion */
#include <sys/kernel.h>
#include <sys/proc.h>
#include <mach/vm_param.h>		/* need for round_page() */
#include <vm/vm_kern.h>
#include <sys/buf.h> 			/* B_READ, in install_bp */
#include <sys/ptrace.h>			/* for SIGTRAP defs */
#include <kern/parallel.h>		/* for unix_master,release def */
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
#endif
#endif

#include <machine/reg.h>
#include <machine/inst.h>
#include <machine/fpu.h>	/* fpc_csr in emulate_branch */
#include <procfs/procfs.h>

extern int machine_check();	/* Per-system machine check handler */
u_long trap_get_subcode();
/*
 * Save the exception frame pointer so it can be found in the kernel name list.
 * This is used by Canasta to obtain analysis information from the dump file.
 * The value of 'ep' is saved upon entering trap().  If the trap is
 * recovered from, the the saved value is reset to zero.  Canasta checks if
 * savedefp = 0 to see if there is an exception frame.
 */

u_int *savedefp = 0;


#if	RT_PREEMPT && RT_PREEMPT_DEBUG
extern int	rt_preempt_syscall;
extern int	rt_preempt_user;
extern int	rt_preempt_async;
extern int	rt_preempt_ast_set;
extern int	rt_preempt_pp;
#endif

/*
 * Exception handling dispatch table. 
 */
extern VEC_syscall(), VEC_cpfault(), VEC_trap(), VEC_int(), VEC_tlbmod();
extern VEC_tlbmiss(), VEC_breakpoint(), VEC_addrerr(), VEC_ibe(), VEC_dbe();
extern VEC_ill_instr();
extern VEC_unexp();
int  (*causevec[16])() = {
	/*  0: EXC_INT */		VEC_int,
	/*  1: EXC_MOD */		VEC_tlbmod,
	/*  2: EXC_RMISS */		VEC_tlbmiss,
	/*  3: EXC_WMISS */		VEC_tlbmiss,
	/*  4: EXC_RADE */		VEC_addrerr,
	/*  5: EXC_WADE */		VEC_addrerr,
	/*  6: EXC_IBE */		VEC_ibe,
	/*  7: EXC_DBE */		VEC_dbe,
	/*  8: EXC_SYSCALL */	 	VEC_syscall,
	/*  9: EXC_BREAK */		VEC_breakpoint,
	/* 10: EXC_II */		VEC_ill_instr,
	/* 11: EXC_CPU */		VEC_cpfault,
	/* 12: EXC_OV */		VEC_trap,
	/* 13: undefined */		VEC_unexp,
	/* 14: undefined */		VEC_unexp,
	/* 15: undefined */		VEC_unexp
};



int  (*c0vec_tbl[8])();
int	c0vec_tblsize = {sizeof(c0vec_tbl)};	/* Size in bytes */

/*
 * Interrupt statistic information
 */

#include <sys/table.h>					/* Statistic info */

int     c0vec_tbl_type[NC0VECS];
int	c0vec_tbl_type_size = {sizeof(c0vec_tbl_type)};	/* size in bytes */

/*
 * nofault dispatch table
 */
extern baerror(), cerror(), adderr(), uerror(), uaerror(), softfp_adderr();
extern reviderror(), cstrerror(), softfp_insterr(), fixade_error();
extern rdnf_error();
int  (*nofault_pc[NF_NENTRIES])() = {
	/* unused */		0,
	/* NF_BADADDR */	baerror,
	/* NF_COPYIO */		cerror,
	/* NF_ADDUPC */		adderr,
	/* NF_FSUMEM */		uerror,
	/* NF_USERACC */	uaerror,
	/* NF_SOFTFP */		softfp_adderr,
	/* NF_REVID */		reviderror,
	/* NF_COPYSTR */	cstrerror,
	/* NF_SOFTFPI */	softfp_insterr,
	/* NF_FIXADE */		fixade_error,
	/* NF_INTR */		rdnf_error
};

/*
 * Used for decoding break instructions.  There is an old standing bug in the
 * assembler which encoded the break code right justified to bit 16 not to
 * bit 6 (that's why the BRK_SHIFT is 16 not 6 as would be obvious).
 */
#define BRK_MASK	0xfc00003f
#define BRK_SHIFT	16
#define BRK_SUBCODE(x)	(((x) & ~BRK_MASK) >> BRK_SHIFT)

/*
 * This must be declared as an int array to keep the assembler from
 * making it gp relative.
 */
extern int sstepbp[];


int 		nofault_cause;	  /* cause at time of nofault */
int 		nofault_badvaddr; /* badvaddr at time of nofault */


/*
 * TODO: Get parameters in agreement with locore.s
 */

#define USERFAULT 1


/*
 * A count of the number of unaligned access fixups
 * not reported via uprintf because the process does
 * not have a controlling terminal (e.g., X server).
 *
 * Also, a variable to poke if you want to redirect
 * the failed uprintf to the console. CAUTION,
 * for debug only (could choke the console with messages).
 */
int	unreported_unaligned_access_fixups = 0;
int	print_unaligned_access_fixups = 0;


/*
 * Called from locore.s (VEC_trap) to handle exception conditions
 */
trap(ep, code, sr, cause)
	register u_int *ep;		/* exception frame ptr */
	register u_int code;		/* trap code (trap type) */
	u_int sr, cause;		/* status and cause regs */
{
	int             nofault_save, save_error, i, up_ret;
	int		exc_type, exc_code, exc_subcode;
	unsigned int	inst;
	struct uthread *uthread;

	kern_return_t   ret;
	vm_map_t	map;
	vm_offset_t     vaddr;
	thread_t	t;

	struct proc	*p;
	struct timeval	syst;
	task_t		task;

		int signo;
	        signo = 0;

        savedefp = ep;      /* make ep accessable from the dump file */

#ifdef	ASSERTIONS
	{
	int s = splhigh();
	splx(s | 1);
	if (!(s & 1))
		panic("trap with interrupts disabled");
	}
#endif
	t = current_thread();
	nofault_save = t->pcb->pcb_nofault;
	t->pcb->pcb_nofault = 0;

	p = u.u_procp;
	uthread = t->u_address.uthread;

	/*
	 * code to trace traps for /proc starts here
	 */

	/*
	 * If PRFS_FAULT in pr_qflags is 0, then skip this fault tracing code.
	 *
	 * Convert the trap code to a bit mask, and check it against the
	 * bit mask of traps to trace.  If the corresponding bit in the
	 * mask of traps to trace is set, set PR_STOPPED and PR_ISTOP in
	 * pr_flags; set pr_why to PR_FAULTED; set pr_what to the fault
	 * number; save the fault, code, in procfs_curflt; if the fault
	 * is an Instruction Bus Error, set PR_PCINVAL in pr_flags; call
	 * wakeup with our thread procfs address to notify any processes
	 * that may be waiting for us to stop; and then suspend ourselves.
	 * If procfs.pr_curflt is -1 then return - i.e. skip the rest of the
	 * trap code.
	 */
 if(t->task->procfs.pr_qflags & PRFS_TRACING) {
	u_int	*i_ep;
	i_ep = ep;	    	/* copy for trap_get_subcode */
	task = current_task();
	if ((PRFS_FAULT & t->t_procfs.pr_qflags) ||
		 (PRFS_FAULT & task->procfs.pr_qflags)){
		if(prismember(task->procfs.pr_flttrace.word, code)) {
			task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			task->procfs.pr_why = PR_FAULTED;
			task->procfs.pr_what = code;
			task->procfs.pr_curflt = code;
			task->procfs.pr_tid = t;
			task->procfs.pr_subcode =  trap_get_subcode(i_ep, code);
			task->suspend_count++;
			task->user_stop_count++;
			wakeup(task->procfs);
			/* task_suspend(task); */
			thread_block(t);
			if (t->task->procfs.pr_curflt == -1)
				goto out;
		}
		else if(prismember(t->t_procfs.pr_flttrace.word, code)) {
			t->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			t->t_procfs.pr_flags &= ~(PR_DSTOP |PR_PCINVAL);
			t->t_procfs.pr_why = PR_FAULTED;
			t->t_procfs.pr_what = code;
			t->t_procfs.pr_curflt = code;
			task->procfs.pr_tid = t;
			t->t_procfs.pr_subcode =  trap_get_subcode(i_ep, code);
			wakeup(task->procfs);
			thread_suspend(t);
			thread_block(t);
			if (t->t_procfs.pr_curflt == -1)
				goto out;
		}
	}
 }
	/* End of code for /proc */

	TRACE({printf("trap no. %d in %s mode. sr=x%x pc=x%x\n",
	       code >> 2, (USERMODE(sr)) ? "user" : "system", sr, ep[EF_EPC]);})


	if (!USERMODE(sr)) {		/* Kernel mode faults */

		switch (code) {
#if	RT_PREEMPT
		case SEXC_AST:
#if	RT_PREEMPT_DEBUG
			rt_preempt_pp++;
#endif
                        preemption_point_safe(rt_preempt_async);
                        ast_mode[cpu_number()] = USER_AST;
                        goto done;
#endif
		case EXC_WMISS:	/* from tlbmod() */
			cause = code;
			code = SEXC_SEGV;
			/* fall through */
		case SEXC_SEGV:
			vaddr = trunc_page(ep[EF_BADVADDR]);
			cause &= CAUSE_EXCMASK;
			TRACE({ printf("trap kSegv at x%x, nofault x%x\n",
				ep[EF_BADVADDR], nofault_save);})

			if ((vaddr >= VM_MIN_KERNEL_ADDRESS) &&
			    (!t->task->kernel_vm_space))
				map = kernel_map;
			else
				map = t->task->map;
			ret = vm_fault(map, vaddr,
				       (cause == EXC_WMISS)
				       		? VM_PROT_READ|VM_PROT_WRITE
						: VM_PROT_READ,
				       FALSE);

			if ((ret != KERN_SUCCESS) &&
			    (vaddr >= VM_MIN_KERNEL_ADDRESS)) {
				ret = vm_fault(kernel_map, vaddr,
				       (cause == EXC_WMISS)
				       		? VM_PROT_READ|VM_PROT_WRITE
						: VM_PROT_READ,
				       FALSE);
			    }
#if	MACH_NBC
			if (thread_should_halt(t))
				ret = KERN_FAILURE;
#endif
			if (ret == KERN_SUCCESS)
				goto done;
	
			goto chk_nofault;

		case EXC_DBE:
			/*
			 * interpret instruction to calculate faulting address
			 */
			ep[EF_BADVADDR] = ldst_addr(ep);
			/* fall through */
		case EXC_CPU:
		case EXC_IBE:
		case EXC_RADE:
		case EXC_WADE:
chk_nofault:
		/*
		 * This is the "nofault" handler.  Low level code sets
		 * "nofault" to appropriate values and this code dismisses the
		 * trap if it comes.  See for example badaddr() in locore.s
		 */
			if (nofault_save) {
				extern          (*nofault_pc[]) ();
				extern		parityerr;
	
				parityerr = 1;
				nofault_cause = cause;
				nofault_badvaddr = ep[EF_BADVADDR];
				i = nofault_save;
				nofault_save = 0;
				if (i < 1 || i >= NF_NENTRIES)
					panic("bad nofault");
				ep[EF_EPC] = (u_int) nofault_pc[i];
				goto done;
			}
/*  trap () commented out call to buserror in chk_nofault area */
#ifdef notdef
			if (code == EXC_IBE || code == EXC_DBE)
				/*
				 * Must do special processing on bus errors
				 * to clear the fault or we will loop in panics.
				 */
				buserror(ep);
#endif	/* notdef */
			/* fall through */
		default:
			goto fatal;
		}
	}
#ifdef	ASSERTIONS
	else if (nofault_save)
		panic("nofault set in user mode");
#endif

	code |= USERFAULT;

	if (p)
	    syst = u.u_ru.ru_stime;

				/* Note, that the code below that sets exc_code
				 * was used as a template by the function
				 * trap_get_subcode() - for use by /proc.
				 */
	exc_code = 0;
	exc_subcode = 0;
				/* User mode faults */
	switch (code) {

	case EXC_RMISS|USERFAULT:	/* from softfp nofault code */
#if	SHOW
		printf("trap Rmiss, ");
#endif
	case EXC_WMISS|USERFAULT:	/* from softfp nofault code */
#if	0/*SHOW*/
		printf("trap Wmiss, ");
#endif
		cause = code & ~USERFAULT;
		code = SEXC_SEGV|USERFAULT;
		/* fall through */

	case SEXC_SEGV|USERFAULT:
		cause &= CAUSE_EXCMASK;
		vaddr = trunc_page(ep[EF_BADVADDR]);
		TRACE({ printf("trap uSegv at %x, badva 0x%x\n",
			ep[EF_EPC], ep[EF_BADVADDR]);})

		map = t->task->map;
		ret = vm_fault(map, vaddr,
			       (cause == EXC_WMISS)
			       		? VM_PROT_READ|VM_PROT_WRITE
					: VM_PROT_READ,
				FALSE);

		if (ret == KERN_SUCCESS)
			goto done;

		exc_type = EXC_BAD_ACCESS;
		exc_code = ret;
		exc_subcode = ep[EF_BADVADDR];

		break;
	case SEXC_AST|USERFAULT:
		astoff();
#if	RT_PREEMPT
		ast_mode[cpu_number()] = USER_AST;
#endif
		while (thread_should_halt(current_thread()))
			thread_halt_self();

		if (p) {
 		    if ((p->p_flag & SOWEUPC) && u.u_prof.pr_scale > 1) {
			addupc(ep[EF_EPC], &u.u_prof, 1);
			p->p_flag &= ~SOWEUPC;
		    }
		}
		goto out;

	case EXC_II|USERFAULT:		/* Illegal instr */
#if	SHOW
		printf("trap Ill at %x\n", ep[EF_EPC]);
#endif
		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_MIPS_RESOPND;
		exc_subcode = cause;
		break;
		
	case SEXC_CPU|USERFAULT:	/* coproc unusable */
#if	SHOW
		printf("trap Copr at %x\n", ep[EF_EPC]);
#endif
		exc_type = EXC_SOFTWARE;
		exc_code = EXC_MIPS_SOFT_CPU;
		exc_subcode = cause;
		break;

	case EXC_OV|USERFAULT:		/* Overflow */
#if	SHOW
		printf("trap Ovf at %x\n", ep[EF_EPC]);
#endif
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_MIPS_FLT_OVERFLOW;
		exc_subcode = cause;
		break;

	case SEXC_ILL|USERFAULT:	/* Soft illegal instruction */
	case EXC_DBE|USERFAULT:		/* Data Bus Error */
		/*
		 * Interpret instruction to calculate faulting address
		 */
		ep[EF_BADVADDR] = ldst_addr(ep);
#if	SHOW
		printf("trap Dbe, ");
#endif
		/* fall through */

	case EXC_IBE|USERFAULT:		/* Instruction Bus Error */
#if	SHOW
		printf("trap Ibe at %x, vaddr %x\n", ep[EF_EPC], ep[EF_BADVADDR]);
#endif
		/*
		 * Call processor specific routine to handle trap error.
		 * "signo" is passed as a return parameter: it gets set to
		 *     SIGBUS if we want to terminate the user process.
		 * If the system needs to panic, it does so in the processor
		 *     specific routine.
		 */ 
		machine_check(ep, code, sr, cause, &signo);
		if (signo) {
		    exc_type = EXC_BAD_ACCESS;
		    exc_code = KERN_INVALID_ADDRESS;
		    exc_subcode = ep[EF_BADVADDR];
		}
		break;

	case EXC_RADE|USERFAULT:	/* Read Address Error */
		/* fall through */
	case EXC_WADE|USERFAULT:	/* Write Address Error */

		/* First thing we must do is check to see if this is a special
		 * type of kernel access exception.  Namely, did we just jump to
		 * a kernel address while in user mode.  If so, the EPC will be
		 * a kernel address not the address of the jump instruction that
		 * got us here.  So, we might as well punt right away.  Otherwise,
		 * we end up in the fixade routine and try to decode the instruction
		 * at the bogus kernel address.  At that point the error we would
		 * report depends upon what is at the kernel address and has nothing
		 * to do with the actual error.
		 */
		if (ep[EF_EPC] >= K0BASE) {
			i = 1;			/* Force a bad kernel access error. */
		}
		else {
			i = fixade(ep, cause);
		}

		if (i == 0) {
			/* success */
			up_ret = uprintf("Fixed unaligned %s access. PID %d (%s) at pc 0x%x, addr 0x%x\n",
				(code == (EXC_WADE|USERFAULT))?"write":"read",
				p->p_pid, u.u_comm, ep[EF_EPC], ep[EF_BADVADDR]);
			if (up_ret != KERN_SUCCESS) {
			    unreported_unaligned_access_fixups++;
			    if (print_unaligned_access_fixups) {
			        printf("Fixed unaligned %s access. PID %d (%s) at pc 0x%x, addr 0x%x\n",
				(code == (EXC_WADE|USERFAULT))?"write":"read",
				p->p_pid, u.u_comm, ep[EF_EPC], ep[EF_BADVADDR]);
			    }
			}
			break;
		} else if (i == 1) {
			/* kernel addr */
			uprintf("Bad (kernel) memory access, PID %d at pc 0x%x\n",
				p->p_pid, ep[EF_EPC]);
		} else {
			/* couldn't align it */
			uprintf("Unaligned %s access, PID %d (%s) at pc 0x%x, addr 0x%x\n",
				(code == (EXC_WADE|USERFAULT))?"write":"read",
				p->p_pid, u.u_comm, ep[EF_EPC], ep[EF_BADVADDR]);
		}
		exc_subcode = ep[EF_BADVADDR];
		if (!IS_KUSEG(exc_subcode)) {
			exc_type = EXC_BAD_INSTRUCTION;
			exc_code = EXC_MIPS_RESADDR;
		}
		else {
			exc_type = EXC_BAD_ACCESS;
			exc_code = KERN_INVALID_ADDRESS;
		}
		break;

	case EXC_BREAK|USERFAULT:		/* Breakpoint */

		exc_type = EXC_BREAKPOINT;
		vaddr = (vm_offset_t) ep[EF_EPC];
		if (ep[EF_CAUSE] & CAUSE_BD)
			vaddr += 4;
		inst = fuiword((caddr_t)vaddr);
		exc_code = BRK_SUBCODE(inst);
#if	SHOW
		printf("trap Brk at %x, code %x\n", vaddr, exc_code);
#endif
		exc_subcode = (inst == *sstepbp) ? CAUSESINGLE : CAUSEBREAK;
		t->pcb->trapcause = exc_subcode;
		if (exc_subcode == CAUSESINGLE)
			remove_bp();
		break;

	default:
		goto fatal;
	}

	/* Deliver the exception */
	thread_doexception( t, exc_type, exc_code, exc_subcode);

	/* Check for branch-delay emulation */
	if (t->pcb->pcb_bd_ra) {
		ep[EF_EPC] = t->pcb->pcb_bd_epc;
		ep[EF_CAUSE] = t->pcb->pcb_bd_cause;
		t->pcb->pcb_bd_ra = 0;
	}

out:
	if (p) {
	    if (CHECK_SIGNALS(p, t, uthread)) {
		unix_master();
		if (p->p_cursig || issig())
		    psig();
		unix_release();
	    }
	}

	if (csw_needed(t, current_processor())) {
	    u.u_ru.ru_nivcsw++;
#if	RT_PREEMPT && RT_PREEMPT_DEBUG
	    rt_preempt_user++;
#endif
	    thread_block();
	}

	/*
	 * If single stepping this process, install breakpoints before
	 * returning to user mode.
	 */
	if ((t->pcb->pcb_sstep == PT_STEP) && USERMODE(sr))
		install_bp();

	/* Profiling ? */
 	if (u.u_prof.pr_scale > 1) {
	    int	ticks;
	    struct timeval *tv = &u.u_ru.ru_stime;

	    /* Beware tick roundoff - hz is 64, 128, 256, etc */
	    ticks = ((tv->tv_sec - syst.tv_sec) * hz) +
		(((tv->tv_usec - syst.tv_usec) * hz) / (1000*1000));

	    if (ticks > 0)
		addupc(ep[EF_EPC], &u.u_prof, ticks);
	}
done:
	t->pcb->pcb_nofault = nofault_save;
	savedefp = 0;         /* reset if we recovered */
	return;
fatal:
	/*
	 * Kernel mode errors.
	 * They all panic, its just a matter of what we log
	 *     and what panic message we issue.
	 * Call processor specific routine to log and panic.
	 * If we return to here then continue (this will happen
	 *     when doing a memory dump and we get a cpu read ECC
	 *     error on the dump).
	 */ 
	machine_check(ep, code, sr, cause, &signo);
	splhigh();
 	if (code != SEXC_SEGV) {
 		dprintf("trap %d [%d]\n", code >> 2, code&USERFAULT);
 		dumpregs(ep);
 	} else {
 		dprintf("Ksegv pc = %x, va = %x, ep = %x\n",
			ep[EF_EPC], ep[EF_BADVADDR], ep);
 	}
#if	MACH_KDB
	if (kdb_trap(ep, 1))
		return;
#endif
	panic("trap: fatal error");
	halt_cpu();
	/* NOTREACHED */
}


/*
 * Consumers: set_bp(), remove_bp()
 */
write_utext(addr, val)
	u_int *addr;
	u_int val;
{
        vm_offset_t start_addr;
        int ret;

        start_addr = trunc_page(((vm_offset_t)addr));
        if (vm_map_protect( current_thread()->task->map,
                start_addr, start_addr + PAGE_SIZE,
                VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE, FALSE))
                return 0;

        ret = suiword(addr, val);

        if (vm_map_protect( current_thread()->task->map,
                start_addr, start_addr + PAGE_SIZE,
                VM_PROT_READ | VM_PROT_EXECUTE, FALSE))
                return 0;

        if (ret < 0)
                return 0;

        user_flush(addr, sizeof(val), 0x1);

        return 1;
}
 

/*
 * install_bp -- install breakpoints to implement single stepping
 */
install_bp()
{
	unsigned inst;
	unsigned target_pc;

#if	SHOW
	printf("install_bp ");
#endif

	if (current_thread()->pcb->pcb_ssi.ssi_cnt)
		printf("install_bp: can't install multiple breakpoints!!\n");
	/*
	 * If user can't access where his pc points, we give up.
	 * He'll be getting a SIGSEGV shortly anyway!
	 */
	if (!useracc(USER_REG(EF_EPC), sizeof(int), B_READ))
		return;
	inst = fuiword(USER_REG(EF_EPC));
	if (isa_branch(inst)) {
		target_pc = branch_target(inst, USER_REG(EF_EPC), 1);
		/*
		 * Can't single step self-branches, so just wait
		 * until they fall through
		 */
		if (target_pc != USER_REG(EF_EPC))
			set_bp(target_pc);
		set_bp(USER_REG(EF_EPC)+8);
	} else
		set_bp(USER_REG(EF_EPC)+4);
	/*
	 * only install breakpoints once!
	 */
	current_thread()->pcb->pcb_sstep = 0;
}

/*
 * called by install_bp
 *
 */
static
set_bp(addr)
unsigned *addr;
{
	register struct ssi_bp *ssibp;
	register struct pcb *pcb = current_thread()->pcb;

#if	SHOW
	printf("set_bp %x ", addr);
#endif

	ssibp = &pcb->pcb_ssi.ssi_bp[pcb->pcb_ssi.ssi_cnt];
	ssibp->bp_addr = addr;
	/*
	 * Assume that if the fuiword fails, the write_utext will also
	 */
	ssibp->bp_inst = fuiword(addr);
	if (write_utext(addr, *sstepbp))
		pcb->pcb_ssi.ssi_cnt++;
#if	SHOW
	else printf("utext failed!\n");
#endif

}

/*
 * remove_bp -- remove single step breakpoints from current process
 *		used by breakpoint code in trap()
 */
remove_bp()
{
	register struct ssi_bp *ssibp;
	register struct pcb *pcb = current_thread()->pcb;

#if	SHOW
	printf("remove_bp (%d) ", pcb->pcb_ssi.ssi_cnt);
#endif
	while (pcb->pcb_ssi.ssi_cnt > 0) {
		pcb->pcb_ssi.ssi_cnt--;
		ssibp = &pcb->pcb_ssi.ssi_bp[pcb->pcb_ssi.ssi_cnt];
#if	SHOW
	printf("%x\n", ssibp->bp_addr);
#endif
		if (!write_utext(ssibp->bp_addr, ssibp->bp_inst)) {
			uprintf("couldn't remove breakpoint\n");
			continue;
		}
	}
}

/*
 * called by install_bp()
 */
#if	!MACH_KDB
static
#endif
isa_branch(inst)
{
	union mips_instruction i;

	i.word = inst;
	switch (i.j_format.opcode) {
	case spec_op:
		switch (i.r_format.func) {
		case jr_op:
		case jalr_op:
			return(1);
		}
		return(0);

	case bcond_op:
	case j_op:	case jal_op:	case beq_op:
	case bne_op:	case blez_op:	case bgtz_op:
		return(1);

	case cop0_op:
	case cop1_op:
	case cop2_op:
	case cop3_op:
		switch (i.r_format.rs) {
		case bc_op:
			return(1);
		}
		return(0);
	}
	return(0);
}

#define	REGVAL(x)	((x)?USER_REG((x)+EF_AT-1):0)

/*
 * called by install_bp()
 */
#if	!MACH_KDB
static
#endif
branch_target(inst, pc, use_epc)
{
	union mips_instruction i;

	i.word = inst;
	switch (i.j_format.opcode) {
	case spec_op:
		switch (i.r_format.func) {
		case jr_op:
		case jalr_op:
#if	MACH_KDB
			if (use_epc==0)
				return kdbgetreg_val(i.r_format.rs);
			else
#endif
				return(REGVAL(i.r_format.rs));
		}
		break;

	case j_op:
	case jal_op:
		return( ((pc+4)&~((1<<28)-1)) | (i.j_format.target<<2) );

	case bcond_op:
	case beq_op:
	case bne_op:
	case blez_op:
	case bgtz_op:
		return(pc+4+(i.i_format.simmediate<<2));

	case cop0_op:
	case cop1_op:
	case cop2_op:
	case cop3_op:
		switch (i.r_format.rs) {
		case bc_op:
			return(pc+4+(i.i_format.simmediate<<2));
		}
		break;
	}
	panic("branch_target");
}

/*
 * used in DBE handling in trap()
 */
ldst_addr(ep)
register u_int *ep;
{
	register u_int *pc;
	union mips_instruction i;
	int base;

	pc = (u_int *)ep[EF_EPC];
	if (ep[EF_CAUSE] & CAUSE_BD)
		pc++;
	i.word = IS_KUSEG(pc) ? fuiword(pc) : *pc;	/* might tlbmiss */
	if (i.j_format.opcode == spec_op &&
	    i.j_format.target == tas_op) {
		/*
		 * Special software test-and-set.
		 * The only valid register is a0
		 */
		return ep[EF_A0];
	}
	if (i.i_format.opcode < lb_op) {
		splhigh();
		dumpregs(ep);
		panic("DBE not on load or store");
	}
	base = (i.i_format.rs == 0) ? 0 : ep[EF_AT + i.i_format.rs - 1];
	return (base + i.i_format.simmediate);
}

	
/*
 * Handles nofault exceptions early on in system initialization
 * before trap is usable.
 */
trap_nofault(ep, code, sr, cause)
register u_int *ep;
register u_int code;
u_int sr, cause;
{
	register int i;

	switch(code) {

	case EXC_DBE:
		/* nofault handler */
		if (PCB_WIRED_ADDRESS->pcb_nofault) {
			extern (*nofault_pc[])();
			i = PCB_WIRED_ADDRESS->pcb_nofault;
			PCB_WIRED_ADDRESS->pcb_nofault = 0;
			if (i < 1 || i >= NF_NENTRIES)
				panic("bad nofault");
			ep[EF_EPC] = (u_int)nofault_pc[i];
			return;
		}
		/* fall through to default */

	default:
		splhigh();
		dumpregs(ep);
		panic("trap_nofault");

	}
}

dumpregs(ep)
unsigned *ep;
{
	extern struct reg_desc cause_desc[], sr_desc[];
	/*
	 * Dump out other items of interest
	 */
	dprintf("Faulting PC: 0x%x\n", ep[EF_EPC]);
	dprintf("Cause register: %R\n", ep[EF_CAUSE], cause_desc);
	dprintf("Status register: %R\n", ep[EF_SR], sr_desc);
	dprintf("Bad Vaddress: 0x%x\n", ep[EF_BADVADDR]);
	dprintf("Stack Pointer: 0x%x\n", ep[EF_SP]);
#if	MACH_KDB	/* Not very helpful if have kdb on hand */
	dprintf("\n");
#else
	dumpstack(ep);
#endif
}

dumpstack(ep)
unsigned *ep;
{
	static int dumpstack_once = 0;
	register int i;
	unsigned ts;

	if (dumpstack_once)
		return;
	dumpstack_once++;

	if((unsigned)ep & 0x3) {
		dprintf("ep at odd address (x%x)\n", ep);
		ep = (unsigned *)(((unsigned)ep + 0x3) & 0x3);
	}
	ts = round_page(ep);
	while((unsigned)ep < (ts-4*4)) {
 		dprintf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x%s\n",
 			ep,*ep,*(ep+1),*(ep+2),*(ep+3),
 			(((*ep)&0x0fff0000) == 0x0bad0000) ||
 			(((*(ep+1))&0x0fff0000) == 0x0bad0000) ||
 			(((*(ep+2))&0x0fff0000) == 0x0bad0000) ||
 			(((*(ep+3))&0x0fff0000) == 0x0bad0000)  ?
 				" <=" : "");
		ep += 4;
		DELAY(300000);
	}
	/* print the last few stack addresses (no more than 4 left) */
	if((unsigned)ep < (ts-4)) {
		dprintf("0x%x\t0x%x",ep,*ep);
prloop:
		ep++;
		if((unsigned)ep < ts) {
			dprintf("\t0x%x",*ep);
			goto prloop;
		}
		dprintf("\n");
	}
}



/*
 * Masks and constants for the rs field of "coprocessor instructions" 
 * (25-21) which are branch on coprocessor condition instructions.
 */
#define	COPz_BC_MASK	0x1a
#define COPz_BC		0x08

/*
 * Masks and constants for the rt field of "branch on coprocessor 
 * condition instructions" (20-16).
 */
#define	COPz_BC_TF_MASK	0x01
#define	COPz_BC_TRUE	0x01
#define	COPz_BC_FALSE	0x00

#define	PC_JMP_MASK	0xf0000000

/*
 * emulate_branch is used by fp_intr() and fixade() to calculate the 
 * resulting pc of a branch instruction.  It is passed a pointer to 
 * the exception frame, the branch instruction and the floating point 
 * control and status register.
 * The routine returns the resulting pc.  This routine will panic() if it
 * is called with a non-branch instruction or one it does not know how to
 * emulate.
 */
unsigned
emulate_branch(ep, instr, fpc_csr)
u_int *ep;
unsigned instr;
union fpc_csr fpc_csr;
{
    union mips_instruction cpu_instr;
    long condition;
    long rs, rt;

	cpu_instr.word = instr;

	/*
	 * The values for the rs and rt registers are taken from the exception
	 * frame and since there is space for the 4 argument save registers and
	 * doesn't save register zero this is accounted for (the +3).
	 */
	if(cpu_instr.r_format.rs == 0)
	    rs = 0;
	else
	    rs = ep[cpu_instr.r_format.rs + 3];
	if(cpu_instr.r_format.rt == 0)
	    rt = 0;
	else
	    rt = ep[cpu_instr.r_format.rt + 3];

	switch(cpu_instr.i_format.opcode){

	case spec_op:
	    switch(cpu_instr.r_format.func){
	    case jalr_op:
		/* r31 has already been updated by the hardware */
	    case jr_op:
		return(rs);
	    }
	    break;

	case jal_op:
	    /* r31 has already been updated by the hardware */
	case j_op:
	    return(((ep[EF_EPC] + 4) & PC_JMP_MASK) |
		   (cpu_instr.j_format.target << 2));

	case beq_op:
	    condition = rs == rt;
	    goto conditional;

	case bne_op:
	    condition = rs != rt;
	    goto conditional;

	case blez_op:
	    condition = rs <= 0;
	    goto conditional;

	case bgtz_op:
	    condition = rs > 0;
	    goto conditional;

	case bcond_op:
	    switch(cpu_instr.r_format.rt){
	    case bltzal_op:
		/* r31 has already been updated by the hardware */
	    case bltz_op:
		condition = rs < 0;
		goto conditional;

	    case bgezal_op:
		/* r31 has already been updated by the hardware */
	    case bgez_op:
		condition = rs >= 0;
		goto conditional;
	    }
	    break;

	case cop1_op:
	    if((cpu_instr.r_format.rs & COPz_BC_MASK) == COPz_BC){
		if((cpu_instr.r_format.rt & COPz_BC_TF_MASK) == COPz_BC_TRUE)
		    condition = fpc_csr.fc_struct.condition;
		else
		    condition = !(fpc_csr.fc_struct.condition);
		goto conditional;
	    }

	}
	/*
	 * For all other instructions (including branch on co-processor 2 & 3)
	 * hit an unknown branch.  Kill user process and continue. 
	 */
	 psignal(u.u_procp, SIGILL);


conditional:
	if(condition)
	    return(ep[EF_EPC] + 4 + (cpu_instr.i_format.simmediate << 2));
	else
	    return(ep[EF_EPC] + 8);
}

/*
 * Fixade() is called to fix unaligned loads and stores.  It returns a
 * zero value if can fix it and a non-zero error code if it can't fix it.
 * Error codes are:
 *	1 == its a kernel access from user space.
 *	2 == we couldn't fixup the alignment.
 *
 * Fixade() modifies the destination register (general or
 * floating-point) for loads or the destination memory location for
 * stores.  Also the epc is advanced past the instruction (possibly to the
 * target of a branch).
 */
fixade(ep, cause)
register u_int *ep;
u_int cause;
{
    union mips_instruction inst, branch_inst;
    u_int addr, new_epc, word;
    int error;
    thread_t t = current_thread();


	if(cause & CAUSE_BD){
	    branch_inst.word = fuiword((caddr_t)ep[EF_EPC]);
	    inst.word = fuiword((caddr_t)(ep[EF_EPC] + 4));
	    if(branch_inst.i_format.opcode == cop1_op)
		checkfp(t, 0);
	    new_epc = emulate_branch(ep, branch_inst.word,
	    			     t->pcb->pcb_fpc_csr);
	}
	else{
	    inst.word = fuiword((caddr_t)ep[EF_EPC]);
	    new_epc = ep[EF_EPC] + 4;
	}

	addr = REGVAL(inst.i_format.rs) + inst.i_format.simmediate;

	/*
	 * The addresses of both the left and right parts of the reference
	 * have to be checked.  If either is a kernel address it is an
	 * illegal reference.
	 */
	if(addr >= K0BASE || addr+3 >= K0BASE)
	    return(1);

	error = 0;

	switch(inst.i_format.opcode){
	case lw_op:
	    error = uload_word(addr, &word);
	    if(inst.i_format.rt == 0)
		break;
	    else
	    	ep[inst.i_format.rt+3] = word;
	    break;
	case lh_op:
	    error = uload_half(addr, &word);
	    if(inst.i_format.rt == 0)
		break;
	    else
	    	ep[inst.i_format.rt+3] = word;
	    break;
	case lhu_op:
	    error = uload_uhalf(addr, &word);
	    if(inst.i_format.rt == 0)
		break;
	    else
	    	ep[inst.i_format.rt+3] = word;
	    break;
	case lwc1_op:
	    checkfp(t, 0);
	    error = uload_word(addr, &word);
	    t->pcb->pcb_fpregs[inst.i_format.rt] = word;
	    break;

	case sw_op:
	    error = ustore_word(addr, REGVAL(inst.i_format.rt));
	    break;
	case sh_op:
	    error = ustore_half(addr, REGVAL(inst.i_format.rt));
	    break;
	case swc1_op:
	    checkfp(t, 0);
	    error = ustore_word(addr, t->pcb->pcb_fpregs[inst.i_format.rt]);
	    break;

	default:
	    return(2);
	}
	
	if(error)
	    return(2);

	ep[EF_EPC] = new_epc;
	return(0);
}


#ifdef notdef
buserror(ep)
u_int *ep;
{
	/*
	 * attempt to scrub the error by writing it to zero
	 */
	wbadaddr(ep[EF_BADVADDR] &~ (sizeof(int)-1), sizeof(int), 0);
}
#endif	/* notdef */



/*
 * trap_get_subcode
 *
 * This determines the subcode for the following traps, and returns it in
 * procfs->pr_subcode.  The code mimics what is done in trap().  This function
 * was added for /proc.
 */

u_long
trap_get_subcode(ep, code)
u_int	*ep;			/* exception frame pointer */
u_int	code;			/* trap code */
{
	int		exc_subcode;
	vm_offset_t	vaddr;
	unsigned int	inst;

	switch (code) {


	case SEXC_SEGV:
		return( (u_long)KERN_SUCCESS);
		break;

	case EXC_II:				/* Illegal instr */
		return( (u_long)EXC_MIPS_RESOPND);
		break;
		
	case SEXC_CPU:				/* coproc unusable */
		return( (u_long)EXC_MIPS_SOFT_CPU);
		break;

	case EXC_OV:				/* Overflow */
		return( (u_long)EXC_MIPS_FLT_OVERFLOW);
		break;


	case EXC_RADE:				/* Read Address Error */
	case EXC_WADE:				/* Write Address Error */
		exc_subcode = ep[EF_BADVADDR];
		if (!IS_KUSEG(exc_subcode)) {
			return( (u_long)EXC_MIPS_RESADDR);
		}
		else {
			return( (u_long)KERN_INVALID_ADDRESS);
		}
		break;

	case EXC_BREAK:				/* Breakpoint */

		vaddr = (vm_offset_t) ep[EF_EPC];
		if (ep[EF_CAUSE] & CAUSE_BD)
			vaddr += 4;
		inst = fuiword((caddr_t)vaddr);
		return( (u_long)BRK_SUBCODE(inst));
		break;

	default:
		return( (u_long)0);
	}
/*NOTREACHED*/
}

