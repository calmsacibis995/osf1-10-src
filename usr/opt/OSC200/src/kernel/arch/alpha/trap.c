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
static char *rcsid = "@(#)$RCSfile: trap.c,v $ $Revision: 1.2.30.20 $ (DEC) $Date: 1994/01/17 22:19:54 $";
#endif

#include <rt_preempt.h>

#include <mach/exception.h>
#include <mach/machine/thread_status.h>
#include <sys/kernel.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/buf.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <machine/trap.h>
#include <machine/softfp.h>
#include <machine/fpu.h>
#include <machine/local_ieee.h>
#include <vm/vm_kern.h>
#include <procfs/procfs.h>
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

u_long trap_get_subcode();
long trap_debug = 0;

/*
 * The following data structure is filled in if the kernel causes an invalid
 * memory access, just before it does some printf's and calls panic().  This
 * information is then available for crash analysis, either at the console
 * prompt, or through kdebug, or through a crash dump.  This data structure
 * is only referenced below, just before a "kernel memory fault" panic.
 */
struct {
	u_long fault_va;	/* faulting virtual address */
	u_long fault_pc;	/* pc of faulting instruction */
	u_long fault_ra;	/* ra contents at time of fault */
	u_long fault_sp;	/* sp contents at time of fault */
	u_long access;		/* access type (0:R, 1:W, -1:X) */
	u_long status;		/* processor status register */
	u_long cpunum;		/* cpu number (WHAMI reg) */
	u_long count;		/* kernel mem fault count */
	struct pcb *pcb;	/* pointer to current pcb */
	struct thread *thread;	/* pointer to current thread */
	struct task *task;	/* pointer to current task */
	struct proc *proc;	/* pointer to current proc */
} kernel_memory_fault_data;

/*
 * A count of the number of unaligned access fixups
 * not reported via uprintf because the process does
 * not have a controlling terminal (e.g., X server).
 *
 * Also, a variable to poke if you want to redirect
 * the failed uprintf to the console. CAUTION,
 * for debug only (could choke the console with messages).
 */
int     unreported_unaligned_access_fixups = 0;
int     print_unaligned_access_fixups = 0;

/*
 * Save the exception frame pointer so it can be found in the kernel name list.
 * This is used by Canasta to obtain analysis information from the dump file.
 * The value of 'ep' is saved upon entering trap().  If the trap is
 * recovered from, the the saved value is reset to zero.  Canasta checks if
 * savedefp = 0 to see if there is an exception frame.
 */

u_long *savedefp = 0L;
u_long nofault_cause;
u_long nofault_badvaddr;
extern int lww[];
extern long lww_event[];
extern lock_data_t profil_lock;

/*
 * nofault dispatch table	rjlfix
 *
 * The use of the nofault handler isn't completely implemented.
 * As each of the routines is added the entry can be added by removing
 * the comments below.
 */
extern cerror(), adderr(), uerror(), uaerror(), cstrerror();
extern emulator_handler();
extern lwerror(), lwunerror(), lwunerror_aud(), lwerror_ass();

int  (*nofault_pc[NF_NENTRIES])() = {
	/* unused */            0L,
	/* NF_BADADDR         baerror, */ 0L,
	/* NF_COPYIO  */      cerror,
	/* NF_ADDUPC  */      adderr, 
	/* NF_FSUMEM  */      uerror, 
	/* NF_USERACC */      uaerror,
	/* NF_SOFTFP */       emulator_handler,
	/* NF_REVID           reviderror, */ 0L,
	/* NF_COPYSTR */      cstrerror,
	/* NF_SOFTFPI         softfp_insterr, */ 0L,
	/* NF_SIGRETURN_COPYIN */ 0L,		   /* set on first use */
	/* NF_SENDSIG_COPYOUT*/ 0L,                /* set on first use */
	/* NF_LWERR */        lwerror,
	/* NF_LW_UNERR */     lwunerror,
	/* NF_LW_UNERR_AUD */ lwunerror_aud,
	/* NF_LWERR_ASS */    lwerror_ass
};

#if	RT_PREEMPT && RT_PREEMPT_DEBUG
extern int	rt_preempt_syscall;
extern int	rt_preempt_user;
extern int	rt_preempt_async;
extern int	rt_preempt_ast_set;
extern int	rt_preempt_kpp;
#endif

#include <mach/machine/vm_param.h>
extern int emulate_null();
u_long convert_fp_fault();

trap(a0,a1,a2,code,exc_frame)
union {
	u_long exsum;
	u_long va;
	u_long fault_code;
} a0;
union {
	u_long regmask;
	u_long mmcsr;
	u_long opcode;
} a1;
union {
	u_long mmcause;
	u_long sd_reg;
} a2;
u_long	code;			/* trap type code */
u_long *exc_frame;		/* pointer to the exception frame */
{
	register thread_t	t;
	register struct pcb	*pcb;
	task_t			task;
	struct proc		*p;
	struct timeval syst;
	long	nofault_save;
	unsigned long	exc_type, exc_code, exc_subcode;
#ifdef rjlfix
	struct cpudata *pcpu = CURRENT_CPUDATA;	/* SMP */
#endif
	extern int printstate;
	int			i;
	int			s;
	time_value_t		stime_enter, stime_exit, utime;
	int			stime_enter_valid;

	savedefp = exc_frame;		/* for dumps analysis	*/

        if (U_ADDRESS.utask) 
                p = u.u_procp;
        else 
                p = NULL;

	if (t = current_thread()) {
 
		task = current_task();
		pcb = t->pcb;

		/*
		 * Save no fault state, we don't want to have nofault enabled
		 * while we try to process the fault!!
		 */
		nofault_save = pcb->pcb_nofault;
		pcb->pcb_nofault = 0L;

		/*
		 * code to trace traps for /proc starts here
		 */

		/*
		 * If PRFS_FAULT in pr_qflags is 0, then skip fault tracing code.
		 *
		 * Convert the trap code to a bit mask, and check it against the
		 * bit mask of traps to trace.  If the corresponding bit in the
		 * mask of traps to trace is set, set PR_STOPPED and PR_ISTOP in
		 * pr_flags; set pr_why to PR_FAULTED; set pr_what to the fault
		 * number; save the fault, code, in procfs_curflt; if the fault
		 * is an Instruction Bus Error, set PR_PCINVAL in pr_flags; call
		 * wakeup with our thread procfs address to notify any processes
		 * that may be waiting for us to stop; then suspend ourselves.
		 * If procfs.pr_curflt is -1 then return - i.e. skip the rest of
		 * the trap code.
		 */
		if (task->procfs.pr_qflags & PRFS_TRACING) {
		    if ((PRFS_FAULT & t->t_procfs.pr_qflags) ||
		    (PRFS_FAULT & task->procfs.pr_qflags)){

		    switch(code)
		    {
		    case T_IFAULT:
		        exc_subcode = a0.fault_code;
		        break;
		    case T_MMANG:
		        exc_subcode = a2.mmcause;
		        break;
		    default:
		        exc_subcode = -1;
		        break;
		    }

		    if(prismember(&task->procfs.pr_flttrace, code)) {
			unix_master();
			task->procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			task->procfs.pr_flags &= ~(PR_DSTOP | PR_PCINVAL);
			task->procfs.pr_why = PR_FAULTED;
			task->procfs.pr_what = code;
			task->procfs.pr_curflt = code;
			task->procfs.pr_tid = t;
			task->procfs.pr_subcode = exc_subcode;
			wakeup(task->procfs);
			task_suspend(task);
			unix_release();

			if (task->procfs.pr_curflt == -1)
				goto out;
		    }
		    else if(prismember(&t->t_procfs.pr_flttrace, code)) {
			t->t_procfs.pr_flags |= PR_STOPPED | PR_ISTOP;
			t->t_procfs.pr_flags &= ~(PR_DSTOP |PR_PCINVAL);
			t->t_procfs.pr_why = PR_FAULTED;
			t->t_procfs.pr_what = code;
			t->t_procfs.pr_curflt = code;
			task->procfs.pr_tid = t;
			t->t_procfs.pr_subcode =  exc_subcode;
			wakeup(task->procfs);
			thread_suspend(t);
			thread_block();
			if (t->t_procfs.pr_curflt == -1)
				goto out;
		    }
		  }
		}
		/* End of code for /proc */

	} else {
		/* can't get these if don't have thread -- not a normal case */
		task = NULL;
		pcb = NULL;
		nofault_save = 0;
	}

	/*
	 * Check process status at time of exception
	 */
	if (USERMODE(exc_frame[EF_PS])) {
		code += USERFAULT;
		if (u.u_prof.pr_scale > 1) {
			thread_read_times(t, &utime, &stime_enter);
			stime_enter_valid = 1;
		}
		else
			stime_enter_valid = 0;
	}
#ifdef rjlfix
	CURRENT_CPUDATA->cpu_trap++; /* SMP */
#endif
	exc_code = 0;
	exc_subcode = 0;
	switch (code) {
		kern_return_t	ret;
		vm_map_t	map;
		vm_prot_t	prot;
		char		*cp;

	default:			/* all others			*/
		printstate |= PANICPRINT;
		printf("trap type %x, status word=%16lx, pc=%16lx\n",
			code, exc_frame[EF_PS], exc_frame[EF_PC]);
		panic("trap");

	case T_IFAULT:
		switch (a0.fault_code) {

		/* 
		 * If we get pre_empted, we could get rescheduled and then
		 * come back in the kernel with fp off.  If we then try to
		 * restore the fp state, we would blow up unless we do the
		 * following....
		 *
		 * also, if we get an unaligned trap on the first fp 
		 * instruction we need to do this.
		 */
		case T_IFAULT_FEN:		/* turn on the fp */
			enable_fen(pcb);
			goto done;
		case T_IFAULT_OPDEC:	
		    /*
		     * We should only get here from the ieee emulation code.
		     * nofault will have been set.
		     */
		    if (nofault_save) {
			int i;
			u_long result;
			u_long *pexcsum = (u_long *)exc_frame[EF_A2];
			i = nofault_save;
			nofault_save = 0;
			 
			if(i != NF_SOFTFP)
				panic("bad nofault: T_IFAULT_OPDEC");
			ret = fp_infinity(exc_frame, &a0, &a1, 0, &result);
			if (ret == FP_FIXED_INFINITY) {
				exc_frame[EF_V0] = result;
				*pexcsum = 0;
			}
			else if (ret == FP_SIGNAL_USER) {
				exc_frame[EF_V0] = result;
				*pexcsum = a0.exsum;
			}
			else {
				uprintf("unknown OPDEC from ieee emulation\n");
				exc_frame[EF_V0] = -1;
				*pexcsum = 0;
			}

			exc_frame[EF_PC] = (u_long) nofault_pc[i];
			goto done;
		    }
		    goto kifault;

		case T_IFAULT_BPT:
		    /* fall through */
		default:
kifault:
		    printf("kernel inst fault=%d, status=%16lx, pc=%16lx\n",
			a0.fault_code, exc_frame[EF_PS], exc_frame[EF_PC]);
		    panic("trap: illegal instruction");
		}

	case T_IFAULT+USERFAULT:
		switch (a0.fault_code) {
		case T_IFAULT_BPT:
			if (pcb->pcb_ssi.ssi_cnt) {
				remove_bp();
			}
			exc_type = EXC_BREAKPOINT;
			exc_code = a0.fault_code;
			exc_subcode = EXC_ALPHA_BPT;
			break;
		case T_IFAULT_GENT:
			exc_type = EXC_SOFTWARE;
			exc_code =  exc_frame[EF_A0];
			exc_subcode = EXC_ALPHA_INST_TRAP;
			break;

		case T_IFAULT_BUGCK:
			exc_type = EXC_BREAKPOINT;
			exc_code = a0.fault_code;
			exc_subcode = EXC_ALPHA_BPT;
			break;

		case T_IFAULT_FEN:		/* turn on the fp */
			enable_fen(pcb);
			goto done;

		case T_IFAULT_OPDEC:
			ret = fp_infinity(exc_frame, &a0, &a1, 1, (caddr_t)0);
			if (ret == FP_FIXED_INFINITY) {
				exc_frame[EF_PC] += 4; /* incr to next inst. */
				goto out2;
			}
			else  if (ret == FP_SIGNAL_USER) {
				exc_code=ieee_handler(a0.exsum,a1.regmask,
								exc_frame);
				if (exc_code == 0)
					goto out2;
				if ((exc_code == FPE_ILLEGAL_SHADOW_TRAP) ||
				    (exc_code == SFP_FAILURE)) 
					exc_code = convert_fp_fault(a0.exsum);
				exc_frame[EF_PC] += 4;
				exc_type = EXC_ARITHMETIC;
				break;
			}
			else
				/* fall through */ ;
		default:
			uprintf("inst fault=%d, status word=%16lx, pc=%16lx\n",
			a0.fault_code, exc_frame[EF_PS], exc_frame[EF_PC]);
			exc_code = a0.fault_code & ~USERFAULT;
			exc_type = EXC_BAD_INSTRUCTION;
			break;
		}
		break;

	case T_ARITH+USERFAULT:
		if (pcb && pcb->pcb_ownedfp)
		{
			exc_code=ieee_handler(a0.exsum,a1.regmask,exc_frame);
			if (exc_code == 0)
				goto out2;
			if ((exc_code == FPE_ILLEGAL_SHADOW_TRAP) ||
			    (exc_code == SFP_FAILURE)) 
				exc_code = convert_fp_fault(a0.exsum);
		}
		exc_type = EXC_ARITHMETIC;
		break;

	case T_ARITH:
		if (nofault_save) {
			int i;
			u_long *pexcsum = (u_long *)exc_frame[EF_A2];
			i = nofault_save;
			nofault_save = 0;
			 
			if(i != NF_SOFTFP)
				panic("bad nofault: T_ARITH");
			*pexcsum = a0.exsum;
			exc_frame[EF_PC] = (u_long) nofault_pc[i];
			exc_frame[EF_V0] = -1;
			goto done;
		}
		printf("kernel arithmetic fault=%d, status=%16lx, pc=%16lx\n",
			a0.fault_code, exc_frame[EF_PS], exc_frame[EF_PC]);
		panic("kernel arithmetic fault");

	case T_MMANG:
		if((nofault_save == NF_LWERR) || (nofault_save == NF_LW_UNERR)) {
			int i;
			++lww_event[9];
			i = nofault_save;
			nofault_save = 0;
			exc_frame[EF_PC] = (u_long) nofault_pc[i];
			goto done;
		      }
		if (pcb && pcb->pcb_regs[PCB_PC]) {
			/* since we have a context, try the vm fault handler */
			if (a2.mmcause == MMF_READ)
				prot = VM_PROT_READ;
			else if (a2.mmcause == MMF_IFETCH)
				prot = VM_PROT_READ | VM_PROT_EXECUTE;
			else
				prot = VM_PROT_READ | VM_PROT_WRITE;
			/* try task's map only if appropriate */
			if (!task ||
			    (IS_SYS_VA(a0.va) && !task->kernel_vm_space))
				map = kernel_map;
			else
				map = task->map;
			ret = vm_fault(map, alpha_trunc_page(a0.va), prot);
			/* if first try failed, try again with kernel_map */
			if (ret != KERN_SUCCESS &&
			    IS_SYS_VA(a0.va) &&
			    map != kernel_map)
				ret = vm_fault(kernel_map,
					alpha_trunc_page(a0.va), prot);
#if	MACH_NBC
			if (thread_should_halt(t))
				ret = KERN_FAILURE;
#endif
			if (ret == KERN_SUCCESS)
				goto done;
		}

		/*
		 * This is the "nofault" handler.  Low level code sets
		 * "nofault" to appropriate values and this code dismisses the
		 * trap if it comes by returning to a handler.  It's the
		 * handler's responsibility to clean up if necessary and
		 * return an appropriate error code.
		 */
		if (nofault_save) {
			int i;
			/*
			 * Save these incase we do panic
			 */
			nofault_cause = a2.mmcause;
			nofault_badvaddr = a0.va;
			i = nofault_save;
			nofault_save = 0;
			/*
			 * Make sure the nofault code is within the proper
			 * range and the handler is implemented.
			 */
			if((i < 1 || i >= NF_NENTRIES) ||
			   (nofault_pc[i] == NULL) || (i == NF_SOFTFP))
				panic("bad nofault");
			exc_frame[EF_PC] = (u_long) nofault_pc[i];
			goto done;
		}

		/*
		 * If we get down to here, then we have a kernel memory
		 * fault.  We try to crash "gracefully", that is, store
		 * key information in a global data structure and print
		 * some of it as safely as we can, then panic.
		 */
		if (kernel_memory_fault_data.count++ == 0) {
			kernel_memory_fault_data.fault_va = a0.va;
			kernel_memory_fault_data.fault_pc = exc_frame[EF_PC];
			kernel_memory_fault_data.fault_ra = exc_frame[EF_RA];
			kernel_memory_fault_data.fault_sp =
						(u_long)exc_frame + EF_SIZE;
			kernel_memory_fault_data.access = a2.mmcause;
			kernel_memory_fault_data.status = exc_frame[EF_PS];
			kernel_memory_fault_data.cpunum = 0;/* for WHAMI reg */
			kernel_memory_fault_data.pcb = pcb;
			kernel_memory_fault_data.thread = t;
			kernel_memory_fault_data.task = task;
			kernel_memory_fault_data.proc = p;
		}
		switch (a2.mmcause) {
		case MMF_READ:
			cp = " read";
			break;
		case MMF_WRITE:
			cp = " write";
			break;
		case MMF_IFETCH:
			cp = " ifetch";
			break;
		default:
			cp = "";
			break;
		}
		printstate |= PANICPRINT; /* force output to be like panic */

		printf("\ntrap: invalid memory%s access from kernel mode\n\n",
				cp);
		printf("    faulting virtual address:     0x%016lx\n",
				a0.va);
		printf("    pc of faulting instruction:   0x%016lx\n",
				exc_frame[EF_PC]);
		printf("    ra contents at time of fault: 0x%016lx\n",
				exc_frame[EF_RA]);
		printf("    sp contents at time of fault: 0x%016lx\n\n",
				(u_long)exc_frame + EF_SIZE);
		panic("kernel memory fault");
		break;

	case T_MMANG+USERFAULT:
		if (a2.mmcause == MMF_READ)
			prot = VM_PROT_READ;
		else if (a2.mmcause == MMF_IFETCH)
			prot = VM_PROT_READ | VM_PROT_EXECUTE;
		else
			prot = VM_PROT_READ | VM_PROT_WRITE;
		ret = vm_fault(task->map, alpha_trunc_page(a0.va), prot);
		if (ret	== KERN_SUCCESS)
			goto out2;
		if (emulate_null() && (a2.mmcause == MMF_READ)
		    && (a0.va >= 0 && a0.va < VM_MIN_ADDRESS))
			if (map_zero_page_at_0() == 0) {
				ret = KERN_SUCCESS;
				goto out2;
			} else
				ret = KERN_RESOURCE_SHORTAGE;
		exc_type = EXC_BAD_ACCESS;
		exc_code = ret;
		exc_subcode = a0.va;
		break;

	case T_AST:
#if	RT_PREEMPT
#if	RT_PREEMPT_DEBUG
		rt_preempt_kpp++;
#endif
		preemption_point_safe_nospl0(rt_preempt_async);
		ast_mode[cpu_number()] = USER_AST;
		goto done;
#else	/* RT_PREEMPT */
		panic("kernel mode ast");
#endif	/* RT_PREEMPT */
		break;

	case T_AST+USERFAULT:
		astoff();
#if	RT_PREEMPT
		ast_mode[cpu_number()] = USER_AST;
#endif
		/*
		 * Check for termination.
		 */
		while(thread_should_halt(t))
			thread_halt_self();

		lock_write(&profil_lock);
		if(p && (p->p_flag & SOWEUPC) && u.u_prof.pr_scale > 1) {
			addupc(exc_frame[EF_PC], &u.u_prof, 1);
			p->p_flag &= ~SOWEUPC;
		}
		lock_done(&profil_lock);
		goto out;
	}
	/*
	 * Save first 3 args in pcb in case sendsig() is called, in which
	 * case they are provided to the user's signal catching routine
	 * in the sigcontext structure.  These args might also be useful
	 * in debugging kernel crashes.
	 */
	pcb->pcb_traparg_a0 = a0.va;
	pcb->pcb_traparg_a1 = a1.mmcsr;
	pcb->pcb_traparg_a2 = a2.mmcause;

	/* Deliver the exception */
	thread_doexception(t, exc_type, exc_code, exc_subcode);
out:
	if (p) {
		if (CHECK_SIGNALS(p, t, t->u_address.uthread)) {
			unix_master();
			if (p->p_cursig || issig())
				psig();
			unix_release();
	    	}
	}

	if (csw_needed(t, current_processor())) {
#if     RT_PREEMPT && RT_PREEMPT_DEBUG
                rt_preempt_user++;
#endif
	 	thread_block();
	}

	/*
	 * If single stepping this process, install breakpoints before
	 * returning to user mode.
	 */
	if (pcb->pcb_sstep == PT_STEP && USERMODE(exc_frame[EF_PS]))
		install_bp();
out2:
 	if (stime_enter_valid) {

		int ticks, secs, usecs;
	
		thread_read_times(t, &utime, &stime_exit);
	
		secs = stime_exit.seconds - stime_enter.seconds;
		usecs = stime_exit.microseconds - stime_enter.microseconds;

		ticks = (secs * hz) + ((usecs * hz) / (1000*1000));

		if (ticks > 0) {
			lock_write(&profil_lock);
			if (u.u_prof.pr_scale > 1) 
				addupc(exc_frame[EF_PC], &u.u_prof, ticks);
			lock_done(&profil_lock);
		}
	}
done:
	if (pcb)
		pcb->pcb_nofault = nofault_save;
	savedefp = 0L;
	return;
}

/*
 * Display an appropriate unaligned access message (called from locore.s).
 */
afault_print(va, type, regno, pc, ps, ra)
long va, type, regno, pc, ps, ra;
/*ARGSUSED*/
{
	static char *inst_names[] = {
		"ldf", "ldg", "lds", "ldt",
		"stf", "stg", "sts", "stt",
		"ldl", "ldq", "ldl_l", "ldq_l",
		"stl", "stq", "stl_c", "stq_c"
	};

	/*
	 * If this is a non-kernel access print the pid and progname
	 */
	if (USERMODE(ps)) {
		uprintf(
	"Unaligned access pid=%d <%s> va=%lx pc=%lx ra=%lx type=%s\n",
			u.u_procp->p_pid, u.u_comm,
			va, pc, ra, inst_names[type]);
	} else {
		printf(
	"Unaligned kernel access va=%lx pc=%lx ra=%lx type=%s\n",
			va, pc, ra, inst_names[type]);
	}

}

/*
 * Do appropriate unaligned access signal handling (called from locore.s).
 */
afault_signal(va, opcode, regno)
long va, opcode, regno;
{
	struct thread *t;
	struct pcb *pcb;

	if ((t = current_thread()) && (pcb = t->pcb)) {
		pcb->pcb_traparg_a0 = va;
		pcb->pcb_traparg_a1 = opcode;
		pcb->pcb_traparg_a2 = regno;
		/*
		 * NOTE: Use of KERN_FAILURE will result in a SIGBUS signal.
		 */
		thread_doexception(t, EXC_BAD_ACCESS, KERN_FAILURE, va);
	}
}

/*
 * Call psig() on unix master (called from locore.s).
 */
afault_psig()
{
	unix_master();
	psig();
	unix_release();
}

/*
 * Handle a non-recoverable unaligned lock access fault (called from locore.s).
 */
afault_error(ps)
long ps;
{
	/*
	 * If this is a non-kernel access kill the process.
	 */
	if(USERMODE(ps)) {
		register struct proc *p = u.u_procp;
		register sigset_t mask = sigmask(SIGABRT);

		unix_master();
		signal_disposition(SIGABRT) = SIG_DFL;
		p->p_sigignore &= ~mask;
		p->p_sigcatch &= ~mask;
		p->p_sigmask &= ~mask;
		p->p_sig = sigmask(SIGABRT);
		p->p_cursig = SIGABRT;
		u.u_sig = (sigset_t)0;
		u.u_cursig = 0;
		psig();			/* Bye */
		unix_release();
	} else
		panic("Unaligned lock access");

	/*NOTREACHED*/
}

/*****************************************************************************/
/******************* ptrace single step support routines *********************/
/*									     */
/*	I don't think this code belongs here, rather it should be in 	     */
/*	arch/alpha/alpha_ptrace.c.  It is here because that's how they 	     */
/*	did it on mips and it makes maintainance easier to have the same     */
/*	functions in the same files across architectures.		     */
/*****************************************************************************/

extern int ipcreg[];				/* defined in alpha_ptrace.c */

#define JMP_INST	(unsigned)0x68000000
#define BR_INST		(unsigned)0xc0000000
#define FBEQ_INST	(unsigned)0xc4000000
#define FBLT_INST	(unsigned)0xc8000000
#define FBLE_INST	(unsigned)0xcc000000
#define BSR_INST	(unsigned)0xd0000000
#define FBNE_INST	(unsigned)0xd4000000
#define FBGE_INST	(unsigned)0xd8000000
#define FBGT_INST	(unsigned)0xdc000000
#define BLBC_INST	(unsigned)0xe0000000
#define BEQ_INST	(unsigned)0xe4000000
#define BLT_INST	(unsigned)0xe8000000
#define BLE_INST	(unsigned)0xec000000
#define BLBS_INST	(unsigned)0xf0000000
#define BNE_INST	(unsigned)0xf4000000
#define BGE_INST	(unsigned)0xf8000000
#define BGT_INST	(unsigned)0xfc000000

#define OP_MASK		(unsigned)0xfc000000

unsigned long branch_target();

set_bp(addr)
unsigned *addr;
{
	register struct ssi_bp *ssibp;
	register struct pcb *pcb = current_thread()->pcb;
	extern int sstepbp[];


#ifdef	SHOW
	printf("set_bp %lx\n", addr);
#endif

	if (pcb->pcb_ssi.ssi_cnt >= 2) {
		printf("set_bp: too many break points set\n");
		return;
	}

	ssibp = &pcb->pcb_ssi.ssi_bp[pcb->pcb_ssi.ssi_cnt];
	ssibp->bp_addr = addr;
	/*
	 * Assume that if the fuiword fails, the write_utext will also
	 */
	ssibp->bp_inst = fuiword(addr);
	if (write_utext(addr, *sstepbp) == 0)
		pcb->pcb_ssi.ssi_cnt++;
#ifdef	SHOW
	else printf("utext failed!\n");
#endif
}

install_bp() 
{
	unsigned int inst;
	unsigned long target_pc;

#ifdef	SHOW
	printf("install_bp\n");
#endif
	if (current_thread()->pcb->pcb_ssi.ssi_cnt)
		printf("install_bp: can't install multiple breakpoints!!\n");
	/*
	 * If user can't access where his pc points, we give up.
	 * He'll be getting a SIGSEGV shortly anyway!
	 */
	if (!useracc(USER_REG(EF_PC), sizeof(int), B_READ))
		return;
	inst = fuiword(USER_REG(EF_PC));
	if (isa_branch(inst)) {
		target_pc = branch_target(inst, USER_REG(EF_PC));
		if (target_pc != USER_REG(EF_PC))
			set_bp(target_pc);
	}
	set_bp(USER_REG(EF_PC)+4);
	/*
	 * only install breakpoints once!
	 */
	current_thread()->pcb->pcb_sstep = 0;
}

remove_bp() 
{
	register struct ssi_bp *ssibp;
	register struct pcb *pcb = current_thread()->pcb;

#ifdef	SHOW
	printf("remove_bp (%d) breakpoints at ", pcb->pcb_ssi.ssi_cnt);
#endif
	while (pcb->pcb_ssi.ssi_cnt > 0) {
		pcb->pcb_ssi.ssi_cnt--;
		ssibp = &pcb->pcb_ssi.ssi_bp[pcb->pcb_ssi.ssi_cnt];
#ifdef	SHOW
	printf("%lx ", ssibp->bp_addr);
#endif
		if (write_utext(ssibp->bp_addr, ssibp->bp_inst)) {
			uprintf("couldn't remove breakpoint\n");
			continue;
		}
	}
#ifdef SHOW
	printf("\n");
#endif
	/* 
	 * After a breakpoint, the PC is past the instruction.  We set the
	 * PC back one instruction to execute the "real" instruction.
	 */
	USER_REG(EF_PC) -= 4;
}

isa_branch(inst)
unsigned int inst;
{
	switch (inst & OP_MASK) {
	case JMP_INST:
	case BR_INST:
	case FBEQ_INST:
	case FBLT_INST:
	case FBLE_INST:
	case BSR_INST:
	case FBNE_INST:
	case FBGT_INST:
	case FBGE_INST:
	case BLBC_INST:
	case BEQ_INST:
	case BLT_INST:
	case BLE_INST:
	case BLBS_INST:
	case BNE_INST:
	case BGE_INST:
	case BGT_INST:
		return 1;
		break;
	}

	return(0);
}

ulong_t
branch_target(inst, pc)
unsigned int inst;
unsigned long pc;
{
	ulong_t vaddr;

	if ((inst & OP_MASK) == JMP_INST) { /* jump instruction - SRM 3.3.1.1 */
		union {
			unsigned word;
			struct {	
				int disp	:16;
				unsigned rb	:5;
				unsigned ra	:5;
				unsigned opcode	:6;
			} format;
		} jmp_inst;
		ulong_t *locr0 =  (ulong_t *)USER_REGS(u.u_procp->thread);
		jmp_inst.word = inst;
		vaddr = locr0[ipcreg[jmp_inst.format.rb]] & ~(ulong_t)3;
		return vaddr;
	}
	else {				/* Branch instruction - SRM 3.3.2 */
		union {
			unsigned word;
			struct {	
				int disp	:21;
				unsigned ra	:5;
				unsigned opcode	:6;
			} format;
		} branch_inst;
		ulong_t offset;
		branch_inst.word = inst;
           	offset = branch_inst.format.disp << 2;
		vaddr = pc + 4 + offset;
		return (vaddr);
	}
}

/* 
 * Called when the ieee_handler detects an invalid trap shadow - meaning
 * that it couldn't detect the precise PC or fix up the result.  We return 
 * the error from the machine in a format that the software understands.
 * input = exception summary
 * output = exception code that machine_exception() understands.
 */
u_long
convert_fp_fault(exsum)
u_long exsum;
{
	u_long exc_code;

	switch (exsum & ~EXCSUM_SWC) { /* strip off software completion bit */
	case EXCSUM_DZE:
		exc_code = FPE_FLTDIV_FAULT;
		break;
	case EXCSUM_OVF:
		exc_code = FPE_FLTOVF_FAULT;
		break;
	case EXCSUM_UNF:
		exc_code = FPE_FLTUND_FAULT;
		break;
	case EXCSUM_INE:
		exc_code = FPE_INEXACT_FAULT;
		break;
	case EXCSUM_IOV:
		exc_code = FPE_INTOVF_FAULT;
		break;
	case EXCSUM_INV:	/* fall thru */
	default:		/* multiple errors ? */
		exc_code = FPE_INVALID_FAULT;
		break;
	}
	return (exc_code);
}
