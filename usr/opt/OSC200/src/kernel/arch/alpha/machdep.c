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
static char *rcsid = "@(#)$RCSfile: machdep.c,v $ $Revision: 1.2.31.18 $ (DEC) $Date: 1994/01/20 18:19:08 $";
#endif

#include <sys/types.h>
#include <machine/reg.h>
#include <mach/machine/thread_status.h>

#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <ufs/inode.h>
#include <sys/mount.h>
#include <ufs/ufsmount.h>
#include <ufs/fs.h>
#include <vm/vm_kern.h>
#include <kern/xpr.h>
#include <machine/cpu.h>
#include <machine/rpb.h>
#include <machine/vmparam.h>
#include <machine/pmap.h>
#include <sys/mtio.h>
#include <hal/cpuconf.h>
#include <dec/sas/mop.h>
#include <sys/fs_types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <sys/lock.h>
#include <io/common/devdriver.h>
#include <sys/sysinfo.h>
#include <sys/uswitch.h>
#include <sys/habitat.h>
#include <mach/exception.h>

long netblk_ldaddr;		/* kernel virt addr where we put net block */

extern struct timeval time;	/* system time for process correctable */
				/* error thresholding */
static int dumpsize = 0;	/* number of pages for savecore */
static int dont_dump = 0;	/* if nonzero, skip the dump */
int partial_dump = 1;		/* if nonzero, do a partial dump */
extern char bootedfile[];
extern dev_t dumpdev;
extern char version[];
extern char *panicstr;
extern struct msgbuf *pmsgbuf;
extern vm_offset_t blbufphysaddr;
extern struct binlog_bufhdr *blbuf;

/*
 * dump header for savecore
 */
static struct dumpinfo dumpinfo;

#define DUMPOFF_INCR ((2*1024*1024)/DEV_BSIZE) /* byte boundary for dumping */
#define DUMPOFF_MAX ((1024*1024*1204)/DEV_BSIZE) /* max dump offset */

extern int cpu;			/* System type, value defined in cpuconf.h */
extern struct cpusw *cpup;	/* Pointer to cpusw entry for this machine*/

/*****************************************************************************
 *
 *	This section has general alpha architecture routines
 *
 *****************************************************************************/

/*
 * NOTE: The following definitions must match the setjmp values,
 *       which must match the struct sigcontext offsets in order
 *       for it all to work.
 *       So, for the sake of reducing errors, the following
 *       values will be based from the setjmp.h values.
 *       JB_REGS = offset (in integers) to sc_regs array.
 *
 *       Wouldn't it be nice if genassym did it all based
 *       on struct sigcontext!
 */

/*
 * Must keep these old values for now, due to
 * cfe warnings on redefinition conflicts between
 * JB_ definitions in pcb.h, and JB_ definitions in setjmp.h.
 * pcb.h included by user.h.
 *
 * #include <machine/setjmp.h>
*/
/*
 * With the present calling sequence these are just the register numbers
 */
#define R_V0	0
#define R_A0	16
#define R_A1	17
#define R_A2	18
#define R_A3	19
#define R_A4	20
#define R_A5	21
#define R_SP	30

/* desired definitions
#define R_ZERO  JB_ZERO-JB_REGS
#define R_AT    JB_AT-JB_REGS
#define R_V0    JB_V0-JB_REGS
#define R_A0    JB_A0-JB_REGS
#define R_A1    JB_A1-JB_REGS
#define R_A2    JB_A2-JB_REGS
#define R_A3    JB_A3-JB_REGS
#define R_SP    JB_SP-JB_REGS
*/


#ifdef PGINPROF
put pagein profiler here...
#endif

/*
 * Clear registers on exec
 */
setregs(entry)
	long entry;
{
	struct pcb *pcb = current_thread()->pcb;
	register struct proc *p = u.u_procp;

	u.u_ar0[EF_RA] = 0;
	u.u_ar0[EF_PC] = (long)entry;
	pcb->pcb_ownedfp = 0;
	disable_fen();
	p->thread->pcb->pcb_sstep = 0;
	p->thread->pcb->pcb_ssi.ssi_cnt = 0;
}


/*
 * Send an interrupt to process.
 *
 * Stack is set up to allow sigcode in libc to call user signal handling
 * routine, followed by syscall to sigreturn routine below.
 * After sigreturn restores critcal registers, resets the signal mask
 * and the stack, it returns to user mode.
 */

/*
 * Exception stacks are aligned on 64 byte boundaries
 */
#define	STACK_ALIGN(x)	((unsigned long)(x) &~ ((long)(8*sizeof(long))-1))
#define	IMPRECISE_FP_TRAP 0
extern int (*nofault_pc[])();
volatile int sendsig_init = 0;

sendsig(p, sig, signalmask, siginfo_p)
     long (*p)(), sig;
     sigset_t signalmask;
     k_siginfo_t *siginfo_p;
{
	struct sigcontext *scp;
	long *srp;
	long *frp;
	int oonstack;
	sigset_t mask;
	int death_signal;
	unsigned long death_cause;
	register caddr_t stacktop;
	register k_siginfo_t *sip = NULL;
	register struct alpha_saved_state *saved_state;
	long usp;
	register struct pcb *pcb=current_pcb;
	long nofaultpc;

	/*
	 * Take snapshot of signal stack flags with SVR4 indicator
	 * cleared.
	 */
	oonstack = u.u_sigsflags;
	usp = mfpr_usp();

	/*
	 * Decide which stack the signal is to be taken on, and make
	 * sure its aligned and accessable.
	 *
	 * If either the SS_ONSTACK or the SS_DISABLE flag is
	 * set this test will fail.
	 */
	if (!oonstack && u.u_sigsp && (u.u_sigonstack & sigmask(sig))) {
		scp = (struct sigcontext *)STACK_ALIGN(u.u_sigsp) - 1;
		u.u_sigsflags |= SS_ONSTACK;
	} else
		scp = (struct sigcontext *)STACK_ALIGN(usp) - 1;

	if (siginfo_p) {
		/*
		 * Align address pointed to by scp, then subtract off
		 * a siginfo structure.
		 */
		sip = (k_siginfo_t *)STACK_ALIGN(scp) - 1;
		stacktop = (caddr_t) sip;
	} else {
		/* no siginfo, don't allocate space for it */
		stacktop = (caddr_t) scp;
	}

	/*
	 * Make sure this stack data will fit on a fixed-size stack.
	 * The copyout() below will probably work if this stack overruns
	 * into already mapped memory, but it will trash that memory.
	 * This applies only to signal stacks established through
	 * sigaltstack(), where the fixed stack size is known (nonzero
	 * in the test below).
	 */
	if (u.u_sigsflags & SS_ONSTACK)
		/*
		 * Make sure structures fit in signal stack.
		 * This makes sense only for SVR4, where you know the
		 * signal stack size. Having a nonzero u.u_sigssz is
		 * only possible for a user of sigaltstack().
		 */
		if (u.u_sigssz &&
		    ((caddr_t)stacktop <= (caddr_t)(u.u_sigsp - u.u_sigssz))) {
			u.u_sigsflags &= ~SS_ONSTACK;
			goto bad;	
		}


	if (sendsig_init == 0) {
		nofaultpc = getpc();
		/***** If a fault takes place, it returns HERE ****/
		if (sendsig_init == 0) {
			nofault_pc[NF_SENDSIG_COPYOUT] = (int (*)())nofaultpc;
			/*
			 * Need to make sure nofault setup before
			 * sendsig_init is set. ORDER is important.
			 */
			mb();
			sendsig_init = 1;
		}
		else {
			pcb->pcb_nofault = 0; /* clean-up after fault */
			goto bad;	
		}
	}
	pcb->pcb_nofault = NF_SENDSIG_COPYOUT;
	/***************************** NOFAULT section BEGIN ****************/ 
	/*  NOTE!  Any writes to locations pointed to by "scp" must be done */
 	/* 	   in this routine and must be done within this nofault     */
        /*         section.  These references are to user mode locations    */
	/*	   and could fault.    					    */

	saved_state = (struct alpha_saved_state *) &USER_REG(EF_V0);
	/*
	 * Copy signal stack size and base info into sigcontext for
	 * use by a ucontext_t, in case on comes into being.
	 * If we're already on the signal stack, reflect that in the
	 * stack values.
	 */
	if (oonstack & SS_ONSTACK) {
		scp->sc_ssize = u.u_sigssz;
		scp->sc_sbase = u.u_sigsp;
		if (u.u_stack_grows_up == FALSE)
			scp->sc_sbase -= scp->sc_ssize;
	} else {
		scp->sc_ssize = ctob(u.u_ssize);
		scp->sc_sbase = u.u_stack_end;
	}
		
	/*
	 * Save into the sigcontext the signal state and that portion
	 * of process state that we trash in order to transfer control
	 * to the signal trampoline code.
	 *
	 * The remainder of the process state is preserved by
	 * signal trampoline code that runs in user mode.
	 */
	scp->sc_onstack = oonstack;
	scp->sc_mask = signalmask;
	scp->sc_pc = saved_state->pc; 
	scp->sc_ps = saved_state->ps;
	scp->sc_regs[R_V0] = saved_state->r0;
	scp->sc_regs[R_A0] = saved_state->r16;
	scp->sc_regs[R_A1] = saved_state->r17;
	scp->sc_regs[R_A2] = saved_state->r18;
	scp->sc_regs[R_A3] = saved_state->r19;
	scp->sc_regs[R_A4] = saved_state->r20;
	scp->sc_regs[R_A5] = saved_state->r21;
	scp->sc_regs[R_SP] = usp;
	
	/*
	 * If this process has ever used the fp coprocessor, save
	 * its state into the sigcontext.
	 */
	scp->sc_ownedfp = pcb->pcb_ownedfp;
	if (pcb->pcb_ownedfp) {
		if (pcb->pcb_fen == 0)
			enable_fen(pcb);
		freg_save(pcb->pcb_fpregs);	/* dump fp to pcb */
		srp = &scp->sc_fpregs[0];
		frp = (long *)&pcb->pcb_fpregs[0];
		while (srp < &scp->sc_fpregs[32])
			*srp++ = *frp++;
		scp->sc_fpcr = _get_fpcr();
		if (sig == SIGFPE) {
			scp->sc_fp_control = u.u_ieee_fp_control;
			scp->sc_fp_trigger_sum = u.u_ieee_fp_trigger_sum;
			scp->sc_fp_trigger_inst = u.u_ieee_fp_trigger_inst;
			/*
			 * As per ieee spec:
			 *   For an imprecise exception, the sc_pc and the
			 *   trap_pc equal the pc detected by the hardware.
			 *   For a precise exception, the trap_pc is
			 *   equal the pc detected by the hardware and the 
			 *   sc_pc is the pc of the faulting fp instruction.
			 */
			scp->sc_fp_trap_pc = scp->sc_pc;
			if (u.u_ieee_fp_trigger_inst != IMPRECISE_FP_TRAP) 
				scp->sc_pc = u.u_ieee_fp_trap_pc;
		}
	}
	
	/*
	 * Pass the first 3 args from trap() to allow the user's
	 * signal handler to further analyze certain exceptions.
	 */
	scp->sc_traparg_a0 = pcb->pcb_traparg_a0;
	scp->sc_traparg_a1 = pcb->pcb_traparg_a1;
	scp->sc_traparg_a2 = pcb->pcb_traparg_a2;
	
	if (sip) {
		/*
		 * Copy k_siginfo structure onto stack.  Note that 
	  	 * enough room for siginfo_t is allowed, but that 
	  	 * the padding bytes at the end of siginfo_t are left
	  	 * as is, and k_siginfo_t is copied into this
	  	 * area.  k_siginfo_t and siginfo_t are identical
	  	 * except for the padding at the end of siginfo_t.
	  	 */
		*sip = *siginfo_p;
	}
	
	/***************************** NOFAULT section END ******************/ 
	pcb->pcb_nofault = 0;

	/*
	 * Set up registers to enter signal handler when resuming user process
	 */
	saved_state->r16 = sig; /* write A0 */
	if (sip) {
		/* register EF_A1 contains arg 2 of arglist passed to
		 * sigtramp() in user code.
		 */
		saved_state->r17 = (unsigned long)sip;
	} else if (sigmask(sig) & (sigmask(SIGFPE) | sigmask(SIGSEGV) |
				    sigmask(SIGILL) | sigmask(SIGBUS) |
				    sigmask(SIGTRAP))) {
		saved_state->r17 = u.u_code;
		u.u_code = 0;
	} else
		saved_state->r17 = 0L;

	saved_state->r18 = (unsigned long)scp;
	saved_state->r19 = (unsigned long)p;
	
 	mtpr_usp(STACK_ALIGN(stacktop)); 
	saved_state->pc = (unsigned long)u.u_sigtramp;
	return;

bad:

	uprintf(
	"sendsig: can't grow stack, pid %d, proc %s, sig %d, pc 0x%lx\n", 
		u.u_procp->p_pid, u.u_comm, sig, USER_REG(EF_PC));

	/*
	 * Process has trashed its stack; give it a SIGILL (SVR4 required)
	 * violation to halt it in its tracks if an SVR4 process. Otherwise
	 * give it the SIGSEGV seemingly required by the Alpha architecture.
	 */
	if (ISHAB_SVR4(u.u_procp->p_habitat)) {
		death_signal = SIGILL;
		death_cause = EXC_ALPHA_SOFT_STK;
	} else {
		death_signal = SIGSEGV;
		death_cause = EXC_ALPHA_SOFT_SEGV;
	}
	signal_disposition(death_signal) = SIG_DFL;
	mask = sigmask(death_signal);
	u.u_procp->p_sigignore &= ~mask;
	u.u_procp->p_sigcatch &= ~mask;
	u.u_procp->p_sigmask &= ~mask;
 	thread_doexception(current_thread(), EXC_SOFTWARE,
			   death_cause, (unsigned long) USER_REG(EF_PC));
}

int sigreturn_init = 0;
/*
 * Routine to cleanup state after a signal has been taken.
 * Reset signal mask and stack state from context left by sendsig (above).
 */
sigreturn(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		struct sigcontext *sigcntxtp;
	} *uap = (struct args *) args;
	register struct sigcontext *srp;
	register long *frp;
	register long *ufrp;
	register struct alpha_saved_state *saved_state;
	int s;
	long nofaultpc;
	register struct pcb *pcb = current_pcb;
	long onstack, fp_trigger;

	srp = uap->sigcntxtp;
	
	if (((unsigned long)uap->sigcntxtp + sizeof(struct sigcontext)) 
					> VM_MAX_ADDRESS)
		return(EFAULT);

	s = splhigh();  /* no preemption, no TLB shootdown */
	if (sigreturn_init == 0) {
		nofaultpc = getpc();
		/***** If a fault takes place, it returns HERE ****/
		if (sigreturn_init ==0) {
			nofault_pc[NF_SIGRETURN_COPYIN] = (int (*)())nofaultpc;
			/*
			 * Need to make sure nofault setup before
			 * sigreturn_init is set. ORDER is important.
			 */
			mb();
			sigreturn_init = 1;
		}
		else {
			pcb->pcb_nofault = 0; /* clean-up after fault */
			splx(s); /* all safe now... */
			return(EFAULT);
		}
	}
	pcb->pcb_nofault = NF_SIGRETURN_COPYIN;
	/***************************** NOFAULT section BEGIN ****************/ 
	/*  NOTE!  Any reads from locations pointed to by "srp" must be     */
 	/* 	   done in this routine and must be done within this nofault*/
        /*         section.  These references are to user mode locations    */
	/*	   and could fault.    					    */
	onstack = srp->sc_onstack;
	fp_trigger = srp->sc_fp_trigger_inst;
	mb(); /* do previous reads first !!!! */

	/*
	 * Restore the stack that this thread was running
	 * on. They should really be thread specific,
	 * but if some fool thinks he can change them from a thread,
	 * we'll oblige him and put that back in the uarea.
	 *
	 * We only bother with this when a SVR4 ucontext was
	 * involved. If we were on the signal stack when this
	 * signal occurred, we don't bother taking in the stack
	 * values, because they are soon to written over by
	 * the the sigcontext from the earlier signal.
	 */
	if ((onstack & SS_STACKMAGIC) == SS_STACKMAGIC) {
		onstack &= ~SS_STACKMAGIC;
		if (!(onstack & SS_ONSTACK)) {
			u.u_stack_end = srp->sc_sbase;
			u.u_ssize = btoc(srp->sc_ssize);
		}
	}

	/*
	 * In some cases, the caller of sigreturn() (e.g., libexc)
	 * doesn't want the signal mask retored from sigcontext.
	 * If SS_NOMASK is set in sc_onstack, we'll skip that step.
	 */
	if ((onstack & SS_NOMASK) == 0)
		p->p_sigmask = srp->sc_mask & ~sigcantmask;
	u.u_sigsflags = onstack & _SSTACKFLAGS;

	/*
	 * Copy entire user process state from sigcontext into
	 * exception frame, a special exit from syscall insures
	 * that the entire exception frame gets restored.
	 *
	 * Sigcontext is in register order, the exception frame is perverted
	 * by the hardware portion of the exception frame.
	 */
        saved_state = (struct alpha_saved_state *) &USER_REG(EF_V0);

        saved_state->r0 = srp->sc_regs[0];
        saved_state->r1 = srp->sc_regs[1];
        saved_state->r2 = srp->sc_regs[2]; 
        saved_state->r3 = srp->sc_regs[3];
        saved_state->r4 = srp->sc_regs[4];
        saved_state->r5 = srp->sc_regs[5];
        saved_state->r6 = srp->sc_regs[6];
        saved_state->r7 = srp->sc_regs[7];
        saved_state->r8 = srp->sc_regs[8];
        saved_state->r9 = srp->sc_regs[9];
        saved_state->r10 = srp->sc_regs[10];
        saved_state->r11 = srp->sc_regs[11];
        saved_state->r12 = srp->sc_regs[12];
        saved_state->r13 = srp->sc_regs[13];
        saved_state->r14 = srp->sc_regs[14];
        saved_state->r15 = srp->sc_regs[15];
        saved_state->r16 = srp->sc_regs[16];
        saved_state->r17 = srp->sc_regs[17];
        saved_state->r18 = srp->sc_regs[18];
        saved_state->r19 = srp->sc_regs[19];
        saved_state->r20 = srp->sc_regs[20];
        saved_state->r21 = srp->sc_regs[21];
        saved_state->r22 = srp->sc_regs[22];
        saved_state->r23 = srp->sc_regs[23];
        saved_state->r24 = srp->sc_regs[24];
        saved_state->r25 = srp->sc_regs[25];
        saved_state->r26 = srp->sc_regs[26];
        saved_state->r27 = srp->sc_regs[27];
        saved_state->r28 = srp->sc_regs[28];
        saved_state->r29 = srp->sc_regs[29];
        mtpr_usp(srp->sc_regs[30]);
        saved_state->pc = srp->sc_pc;

	/*
	 * If this process has ever used the fp coprocessor,
	 * copy fp state from the sigcontext back to the pcb.
	 */
	if (current_pcb->pcb_ownedfp) {
		ufrp = &srp->sc_fpregs[0];
		frp = (long *)&current_pcb->pcb_fpregs[0];
		while (ufrp < &srp->sc_fpregs[32])
			*frp++ = *ufrp++;
/*
 * verhulst - does this need to be done here?????
 * I thought so but the ieee emulation test breaks if this is executed.
 *		current_pcb->pcb_fpcr = srp->sc_fpcr;
 *		u.u_ieee_fp_control = srp->sc_fp_control;
 */
		mtpr_fen(0);			/* disable fp */
	}
	/***************************** NOFAULT section END ******************/ 
	pcb->pcb_nofault =0;
	splx(s); /* all safe now... */

	/*
	 * The following fake error code is checked for by syscall() in order
	 * to indicate to the system call exception handling in locore.s that
	 * it needs to perform a full register restore before resuming user
	 * code execution.
	 */
	return (EJUSTRETURN);
}

/*
 * Delay routine for all Alpha processors...
 * 
 * This routine uses the 32 bit process cycle counter register to accurately
 * time small delays.  The 64 bit system cycle counter is not used,
 * since it isn't present in all flavors of PAL.
 */
alpha_delay(n)
	int n;		/* number of micro-seconds to delay for */
{
	register u_long end_scc, current_scc;
	register u_long now, previous;

	extern u_int scc();

	previous = scc();
	current_scc = previous;

	/* rpb_counter is nr of ticks per second */
	end_scc = current_scc + ((long)n * rpb->rpb_counter) / 1000000L;

	while (current_scc < end_scc) {
		now = scc();
		current_scc = current_scc - previous + now;
		if (now < previous)
		  current_scc += (1L << 32); /* 32 bit counter wrapped */
		previous = now;
	}
}

int	waittime = -1;
int	shutting_down = 0;

boot(reason, howto)
int reason, howto;
{
        register struct mount *mp, *nmp;
	struct fs *fsp;
	long rs;

	extern int cpu;
	extern struct rpb_percpu *percpu;

        /*
         * "shutting_down" is used by device drivers to determine
         * that the system is shutting down.
         */
        shutting_down = 1;
        
	if ((howto&RB_NOSYNC)==0 && waittime < 0 && bfreelist[0].b_forw) {
		/*
		 *  Force an accurate time into the root file system super-
		 *  block.
		 */
		if (rootfs && !(rootfs->m_flag & M_RDONLY) &&
		    mounttab && rootfs == mounttab->um_mountp &&
		    (fsp = mounttab[0].um_fs) && fsp != (struct fs *)1)
			fsp->fs_fmod = 1;

		waittime = 0;
		/*
		 * Unmount local filesystems when not panicing
		 */
                if (reason != RB_PANIC && rootfs) {
			int error;

			for (mp = rootfs->m_next; mp != rootfs; mp = nmp) {
				nmp = mp->m_next;
				if ((mp->m_flag & M_LOCAL) == 0)
					continue;
				(void) dounmount((struct vnode *)0, mp, 0);
			}
			VFS_SYNC(rootfs, MNT_NOWAIT, error);
		}

		(void) splnet();	/* block software interrupts */
		printf("syncing disks... ");
		{
			int ind, nbusy;

			/*
			 * Write out locally mounted file system blocks
			 * asynchronously
			 */
			mp = rootfs;
			do {
				if ((mp->m_flag & M_LOCAL) == 0) {
					mp = mp->m_next;
					continue;
				}
				mntflushbuf(mp, 0);
				mp = mp->m_next;
			} while (mp != rootfs);

			/*
			 * Wait for all writes to all locally
			 * mounted file systems.
			 */
			for (ind = 0; ind < 20; ind++) {
				nbusy = 0;
				mp = rootfs;
				do {
					if ((mp->m_flag & M_LOCAL) == 0) {
						mp = mp->m_next;
						continue;
					}
					nbusy += mntbusybuf(mp);
					mp = mp->m_next;
				} while (mp != rootfs);

				if (nbusy == 0)
					break;
				printf("%d ", nbusy);
				/*
				 * in case there are any pending ubc
				 * async completions to handle, since
				 * we won't be starting this lwc anymore.
				 */
				ubc_async_iodone_lwc();
				DELAY(40000 * ind);
			      }
			if (nbusy)
				printf("failed\n");
			else
				printf("done\n");
		}
		/*
		 * If we've been adjusting the clock, the todr
		 * will be out of synch; adjust it now.
		 */
		resettodr();
	}
	(void) splhigh();			/* extreme priority */
	if (current_pcb)
		(void) save_context_all();
	rs = (percpu->rpb_state & ~(STATE_HALT_MASK | STATE_RC));
	percpu->rpb_state = rs;
	if (howto & RB_HALT) {
		printf("halting.... (transferring to monitor)\n\n");
		percpu->rpb_state = rs | STATE_HALT;
		dont_dump = 1;
	}
	else if (reason != RB_PANIC) {
		printf("rebooting.... (transferring to monitor)\n\n");
		percpu->rpb_state = rs | STATE_WARM_BOOT;
		dont_dump = 1;
	}
	dumpsys();
	drvr_shutdown();
	halt();
	/* NOTREACHED */
}

char boottype[10];
struct netblk netblk;
static int forcebootp = 0; /* ==1 to force a network boot */
int netbootdebug=0;       /* debug flag */
char netdevice[80];
static int bootp_errno;
#ifdef mips
#define mopnetboot ( (!strncmp(bootdev, "mop", 3)) || \
	    	     (!strncmp(&bootdev[2], "mop", 3)) )
#define bootpnetboot ( (!strncmp(bootdev, "tftp", 4)) || \
	    	     (!strncmp(&bootdev[2], "tftp", 4)) ) 
#endif /* mips */
#ifdef __alpha
#define mopnetboot   ( (!strncmp(bootdev, "MOP", 3)) || \
			(!strncmp(bootdev, "mop", 3)) )
#define bootpnetboot ( (!strncmp(bootdev, "BOOTP", 5)) || \
			 (!strncmp(bootdev, "bootp", 5)) ) 
#endif /* __alpha */
extern char *getbootdev();

#define NETBLK_LDADDR PHYS_TO_KSEG(netblk_ldaddr)

/*
 * called from init_main to see if a network boot has occurred.  If so,
 * available information is read into a local copy of the netblk
 * structure.  As per original design the changing of 'roottype' is what
 * triggers init_main to assume a diskless network environment.
 */
netbootchk()
{
	struct netblk *netblk_ptr;
	extern int roottype;
	extern int swaptype;
	extern int dumptype;
	extern int swapsize;
	char *bootdev;
	char *bootstring;
	char *boottypep;
	int i;

	/*
	 * Note: network boot can be for diskless or Standalone 
	 * ULTRIX.  The caller (init_main.c) checks roottype to
	 * sense the difference.
	 */
	
	boottype[0] = '\0';
	boottypep = &boottype[0];

	bootdev = (char *)getbootdev();
	i=0;
	while(bootdev[i] != NULL) {      /* Save for network crashing */
	        netdevice[i] = bootdev[i];
		i++;
	}
	netdevice[i] = bootdev[i];

	if (forcebootp > 0 || mopnetboot || bootpnetboot)
	    { 
		int i; char *j;

		printf("Booted from Network Interface (%s)\n", bootdev);

	  	if (mopnetboot) {
			/*
		 	 * Take a picture of the netblk structure in case
		 	 * it gets tromped on by someone other than the
		 	 * kernel.
		 	 */
			 bcopy((char *)NETBLK_LDADDR, (char *)&netblk,
				 sizeof (struct netblk));
			 /*
		 	 * Clear out that which was just copied so it isn't
		 	 * hanging around for a subsequent boot.  This will
		 	 * guarantee that NETBLK_LDADDR points to real data
		 	 * pertaining to this boot.
		 	 */
			 j = (char *)NETBLK_LDADDR;
			 for (i = 0; i < sizeof (struct netblk); i++)
				 j[i] = 0;
			/*
			 * If in debug mode, dump the contents of the netblk.
			 */
			if (netbootdebug) {
				netblk_ptr = &netblk;
				printf("netbootchk: Dumping netblk contents\n");
				printf("netblk_ptr->srvname=<%s>\n", 
					netblk_ptr->srvname);
				printf("netblk_ptr->srvipaddr=<0x%x>\n", 
					netblk_ptr->srvipadr);
				printf("netblk_ptr->cliname=<%s>\n", 
					netblk_ptr->cliname);
				printf("netblk_ptr->cliipaddr=<0x%x>\n", 
					netblk_ptr->cliipadr);
				printf("netblk_ptr->brdcst=<0x%x>\n", 
					netblk_ptr->brdcst);
				printf("netblk_ptr->netmsk=<0x%x>\n", 
					netblk_ptr->netmsk);
				printf("netblk_ptr->swapfs=<0x%x>\n", 
					netblk_ptr->swapfs);
				printf("netblk_ptr->rootfs=<0x%x>\n", 
					netblk_ptr->rootfs);
				printf("netblk_ptr->swapsz=<0x%x>\n", 
					netblk_ptr->swapsz);
				printf("netblk_ptr->dmpflg=<0x%x>\n", 
					netblk_ptr->dmpflg);
				printf("netblk_ptr->rootdesc=<%s>\n", 
					netblk_ptr->rootdesc);
				printf("netblk_ptr->swapdesc=<%s>\n", 
					netblk_ptr->swapdesc);
			}
		}

		netblk_ptr = &netblk;
		getboottype(boottypep, bootdev);
		if (boottype[0] == '\0') {
			printf("netload:  unsupported boottype \n");
		} else
		/* for tftp boots, send a BOOTP request packet */
		if (forcebootp > 0 || bootpnetboot) {
			if (bootp_info(boottype) < 0)
				printf("bootp: request failed, errno=%d\n",
					bootp_errno);
		}
		/* TODO: fill in netblk_ptr with rootfs */

		/*** no diskless support yet
		if (netblk_ptr->rootfs == GT_NFS) {
			roottype= (int) netblk_ptr->rootfs;
			swaptype= (int) netblk_ptr->swapfs;
			swapsize= ((int) netblk_ptr->swapsz) * 1024;
			if (netblk_ptr->dmpflg != -1)
			     dumptype= ((int) netblk_ptr->dmpflg) * 1024;
		}
		***/
	}
}

#include <sys/mbuf.h>

/*
 * UDP port numbers for BOOTP server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

#define BOOTREQUEST     1
#define BOOTREPLY       2

/*
 * Bootstrap Protocol (BOOTP).  RFC 951.
 */
struct bootp {
        unsigned char   bp_op;          /* packet opcode type */
        unsigned char   bp_htype;       /* hardware addr type */
        unsigned char   bp_hlen;        /* hardware addr length */
        unsigned char   bp_hops;        /* gateway hops */
        unsigned int    bp_xid;         /* transaction ID */
        unsigned short  bp_secs;        /* seconds since boot began */
        unsigned short  bp_unused;
        struct in_addr  bp_ciaddr;      /* client IP address */
        struct in_addr  bp_yiaddr;      /* 'your' IP address */
        struct in_addr  bp_siaddr;      /* server IP address */
        struct in_addr  bp_giaddr;      /* gateway IP address */
        unsigned char   bp_chaddr[16];  /* client hardware address */
        unsigned char   bp_sname[64];   /* server host name */
        unsigned char   bp_file[128];   /* boot file name */
	unsigned char   bp_vend[64];    /* vendor-specific area */
};

/* globals */
#define Printf printf
int bootpq_maxlen = IFQ_MAXLEN;
int bootp_active=0;     /* for ether_read(), ==1 we are currently using bootp */
struct ifqueue bootpq;  /* bootp input packet queue; see ether_read(); */

/* locals */
static int dst_port_addr, my_port_addr;
static struct bootp sendbuf, recvbuf;

/*
 * Broadcast a bootp packet to find out our IP address and
 * the server IP address.  Return with the netblk filled in,
 * or -1 on error.
 */
static bootp_info(bp)
	char *bp;	/* string name of interface to use (ie. "ln0") */
{
	char *ntoa();
	struct netblk *netblk_ptr;		/* ptr to netblk */
	register struct in_ifaddr *ia;		/* for ifp search */
	struct ifnet *ifp;			/* ptr to ifnet */
	char devname[10];			/* device name (only) */
	int devunit;				/* device unit # */
	struct ifdevea ifd;			/* for SIOCRPHYSADDR ioctl */
	struct in_ifaddr ifa;			/* for SIOCSIFADDR */
	struct sockaddr sa;			/* for SIOCSIFADDR */
	int i, s, found, error, cmd, count;
	struct mbuf *m;
	int sts=0, retrys=5;

	/* initialize bootpq */
	bootpq.ifq_head = bootpq.ifq_tail = (struct mbuf *)0;
	bootpq.ifq_len = 0;
	bootpq.ifq_drops = 0;
	bootpq.ifq_maxlen = bootpq_maxlen;
	bootp_active = 1;	/* this hooks into ether_read() */
	netblk_ptr = &netblk;
	/*
	 * Set our addresses for bootp transaction.
	 */
	dst_port_addr = htons(IPPORT_BOOTPS);
	my_port_addr = htons(IPPORT_BOOTPC);

	bootp_errno = 0;
	i=0;
	while(*bp != NULL) {	/* copy device name, then zap the unit number */
		devname[i++] = *bp++;
	}
	devunit = devname[--i] - '0';	/* XXX assumes single digit unit # */
	devname[i] = NULL;

	if (netbootdebug>0) Printf("bootp: checking ifnet for %s, unit %d\n",
		devname, devunit);
	/* find the ifp of this device so we can call the driver directly */
	found=0;
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if ((strcmp (ifp->if_name, devname))==0) {
			if (ifp->if_unit == devunit) {
				found=1;
				break;
			} else {
				if (netbootdebug>0) Printf("bootp: found name %s but wrong unit (%d)\n",
				ifp->if_name, ifp->if_unit);
			}
		} else {
			if (netbootdebug>0) Printf("bootp: name %s doesn't match\n", ifp->if_name);
		}
	}
	if (found>0) {
		if (netbootdebug>0) Printf("bootp: found ifp OK, %s%d\n", ifp->if_name, ifp->if_unit);
	} else {
		/* hardwire to line 0, unit 0 if search fails */
		ifp = ifnet;
		Printf("bootp: ifnet entry for %s%d not found, using %s%d\n",
			devname, devunit, ifp->if_name, ifp->if_unit);
	}

	/* get hardware address */
	if (ifp == 0 || ifp->if_ioctl == 0) {
		Printf("bootp: no ifp->if_ioctl() for device %s%d\n",
			ifp->if_name, ifp->if_unit);
		bootp_errno = ENXIO;
		return(-1);
	}
	cmd=SIOCRPHYSADDR;
	if (netbootdebug>0) Printf("bootp: calling *ifp->if_ioctl(SIOCRPHYSADDR)\n");
	error = (*ifp->if_ioctl)(ifp, cmd, &ifd);
	if (error != 0) {
		if (netbootdebug>0)
			Printf("bootp: non-zero return from ifp->if_ioctl(SIOCRPHYSADDR)\n");
		/* keep going... */
	}
	if (netbootdebug>0)
		Printf("bootp: hardware addr=%s\n",
			ether_sprintf(ifd.default_pa));

	/*
	 * Set the network interface up and running, using INET
	 * addr 0.0.0.0, so we can send/receive BOOTP packets.
	 */
	bzero((char *) &ifa, sizeof(struct in_ifaddr));
	bzero((char *) &sa, sizeof(struct sockaddr));
	ifa.ia_ifa.ifa_addr = &sa;
	sa.sa_family = AF_INET;
	ifp->if_addrlist = &ifa.ia_ifa;
	cmd = SIOCSIFADDR;
	if (netbootdebug>0)
		Printf("bootp: calling *ifp->if_ioctl(SIOCSIFADDR)\n");
	error = (*ifp->if_ioctl)(ifp, cmd, &ifa);
	if (error != 0) {
		if (netbootdebug>0)
			Printf("bootp: non-zero return from ifp->if_ioctl(SIOCSIFADDR)\n");
		/* keep going... */
	}

	/*
	 * Generate the BOOTP request message.
	 */
	bzero((char *)&sendbuf, sizeof(sendbuf));
	sendbuf.bp_op = BOOTREQUEST;
	sendbuf.bp_hlen = 6;
	/* sendbuf.bp_xid = getrand(1024,0xff00); */
	sendbuf.bp_xid = 19338; /* XXX: not random */

	sendbuf.bp_ciaddr.s_addr = 0;
	if (ifp->if_type == IFT_ISO88025)	/* convert address for token ring */
	{
		haddr_convert(ifd.default_pa);
		sendbuf.bp_htype = ARPHRD_802;
	}
	else
	    sendbuf.bp_htype = ARPHRD_ETHER;
	/* XXX: 6 is hardwired to Ethernet */
	bcopy((char *)ifd.default_pa, (char *)sendbuf.bp_chaddr, 6);

	/*
	 * Send BOOTP request and get a response.
	 *
	 * Repeat transaction attempt up to 5 times.
	 *   Send the request.
	 *   Read response.
	 *     If error, retry.
	 *     If success, return.
	 *     If no response, repeat until timeout, then retry.
	 */
	while (retrys--) {
		count=0; /* error count */
		sendbuf.bp_xid++;
		if (netbootdebug>0) Printf("bootp: trying %d\n",(retrys+1));
		if ((sts=bootp_write(ifp, &sendbuf, sizeof (sendbuf))) != 0) {
			Printf("bootp: network write failed, errno=%d\n", bootp_errno);
			continue;
		}
		sts=0;
		while (sts == 0) {
			sts = bootp_read(ifp, &recvbuf, sizeof (recvbuf));
			if (sts > 0) {
				netblk_ptr->cliipadr = ntohl(recvbuf.bp_yiaddr.s_addr);
				netblk_ptr->srvipadr = ntohl(recvbuf.bp_siaddr.s_addr);

				/* future: fill in real names here */
				bcopy("SERVER", netblk_ptr->srvname, 6);
				netblk_ptr->srvname[6] = NULL;
				bcopy("CLIENT", netblk_ptr->cliname, 6);
				netblk_ptr->cliname[6] = NULL;
				netblk_ptr->brdcst = (unsigned int)-1; /* all 1's */
				netblk_ptr->netmsk = 0; /* force default */
				if (netbootdebug>0) {
					Printf("bootp: initialized netblk..\n");
					Printf("       client IP is %s\n", ntoa(htonl(netblk_ptr->cliipadr)));
					Printf("       server IP is %s\n", ntoa(htonl(netblk_ptr->srvipadr)));
				}
				bootp_active = 0; /* done */
				ifp->if_addrlist = NULL;
				s = splimp();
				for(;;) {
					/* drain input queue */
					IF_DEQUEUE(&bootpq, m);
					if (m == 0)
						break; /* done */
					m_freem(m);
				}
				splx(s);
				return(0); /* success */
			}
			if (sts == 0) {
				/* returned without reading any input packets.
				 * Go around the loop trying "retry" times.
				 */
				sts++;
			}
			if (sts == -1) {
				/* error, not our port, length error, etc. */
				sts++; /* == 0 */
				if (count++ > 12) /* allow 12 "error" packets */
					sts++; /* == 1, goto retry */
			}
		} /* end while (sts == 0) */
	} /* end while (retrys--) */

	/* fallthrough is error case */
	bootp_active = 0;
	ifp->if_addrlist = NULL; /* cleanup */
	s = splimp();
	for(;;) {
		/* drain input queue */
		IF_DEQUEUE(&bootpq, m);
		if (m == 0)
			break; /* done */
		m_freem(m);
	}
	splx(s);
	bootp_errno=ETIMEDOUT;
	return(-1);
}
/*
 * bootp_write
 *
 * Note that eh is setup to point to an odd halfword.  This then causes the
 * ip header and udp headers to be word aligned.
 * return 0 on success, -1 on error and errno set appropriately.
 */
static int bootp_write(ifp, buf, cnt)
	struct ifnet *ifp;
	char *buf;
	int cnt;
{
	struct ip *iph;
	struct udphdr *udph;
	char *data;
	static unsigned short ip_id = 0;
	struct mbuf *m, *mp;
	int s;
	struct sockaddr_in dst;

	bootp_errno = 0;
	s = splimp();
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m) {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
			Printf("bootp_write: no mbuf clusters!\n");
			bootp_errno = ENOBUFS;
			goto bad;
		}
		m->m_len = MCLBYTES;
		m->m_pkthdr.len = MCLBYTES;
	} else {
		Printf("bootp_write: no mbufs!\n");
		bootp_errno = ENOBUFS;
		goto bad;
	}
	splx(s);

	iph = mtod(m, struct ip *);
	udph = (struct udphdr *)((char *)iph + sizeof (struct ip));
	data = (char*)udph + sizeof(struct udphdr);

	/*
	 * Copy data to buffer.
	 */
	bcopy(buf, data, cnt);

	m->m_len = cnt + sizeof(struct ip) + sizeof (struct udphdr);
	m->m_pkthdr.len = m->m_len; /* needed? */
	m->m_flags |= M_BCAST;	/* so arpresolve won't spit us out */

	/*
	 * Construct IP header.
	 */
	bzero((char *)iph, sizeof(struct ip));
	iph->ip_vhl = (sizeof(struct ip)>>2 | IPVERSION<<4);
	iph->ip_len = htons(sizeof(struct udphdr) + sizeof(struct ip) + cnt);
	iph->ip_id = htons(ip_id++);
	iph->ip_off = htons(IP_DF);
	iph->ip_ttl = MAXTTL;
	iph->ip_p = IPPROTO_UDP;
	iph->ip_src.s_addr = 0;
	iph->ip_dst.s_addr = INADDR_BROADCAST;
	iph->ip_sum = 0;
	iph->ip_sum = in_cksum(m, sizeof(struct ip));

	/*
	 * Construct UDP header.  (No checksum.)
	 */
	udph->uh_sport = my_port_addr;
	udph->uh_dport = dst_port_addr;
	udph->uh_ulen  = htons(cnt + sizeof(struct udphdr));
	udph->uh_sum = 0;

	/* our dummy IP address is still 0.0.0.0
	 */
	bzero((char *) &dst, sizeof(struct sockaddr_in));
	dst.sin_family = AF_INET;

	(*ifp->if_output)(ifp, m, &dst, (char *)0);

	return(0);

bad:
	m_freem(m);
	splx(s);
	return(bootp_errno);
}

/*
 * bootp_read
 * Read a BOOTP packet from the network.  Return length of buffer read,
 * or return -1 if error (packet not for us, etc.)
 */
static int bootp_read(ifp, buf, cnt)      /* NOTE: ifp not used */
	struct ifnet *ifp;
	char *buf;
	int cnt;
{
	struct ip *iph;
	struct udphdr *udph;
	struct mbuf *m;
	int s;
	int sts = 0;
	int count = 0;


again:
	s = splimp();
	IF_DEQUEUE(&bootpq, m);
	splx(s);
	if (m == 0) {
		if (count++ > 10) {
			/* nothing came in for 5 seconds */
			if (netbootdebug>0)
				Printf("bootp_read: IF_DEQUEUE bootpq is empty\n");
			return(0);
		}
		DELAY(500000);  /* 1/2 sec */
		goto again;
	}
	if (netbootdebug>0)
		Printf("bootp: got %d bytes at addr 0x%lx\n", m->m_len, m->m_data);
	sts = m->m_pkthdr.len;

	if ((m->m_len < sizeof (struct ip) + sizeof(struct udphdr)) &&
		(m = m_pullup(m, sizeof(struct ip) + sizeof(struct udphdr))) == 0) {
		  goto drop; /* packet too short */
	}

	iph = (struct ip*)(mtod(m, char *));
	udph = (struct udphdr*)((char*)iph + sizeof(struct ip));

	/*
	 * Validate IP data...
	 *   Check the header checksum.
	 *   Check that length is le than packet size.
	 *   Check that it is a udp packet.
	 */
	if (in_cksum(m, sizeof(struct ip))) {
		if (netbootdebug>0) Printf("bootp_read: IP in_cksum failed\n");
		goto drop;
	}

	if (ntohs(iph->ip_len) > sts) {
		if (netbootdebug > 0)
		    Printf("bootp_read: IP header len > readsize\n");
		goto drop;
	}
	if (iph->ip_p != IPPROTO_UDP) {
		if (netbootdebug > 0)
		    Printf("bootp_read: IP header proto not UDP/IP\n");
		goto drop;
	}

	/*
	 * Validate UDP data...
	 *  Length checks.
	 *  Check that it is to us.
	 *  Record where it came from.
	 */
	sts -= sizeof (struct ip);
	udph->uh_ulen = ntohs(udph->uh_ulen);
	if (udph->uh_ulen > sts) {
		if (netbootdebug > 0)
		    Printf("bootp_read: UDP header len > readsize\n");
		goto drop;
	}
	sts = udph->uh_ulen - sizeof(struct udphdr) ;

	/* make sure packet is for us */
	if (udph->uh_dport != my_port_addr ) {
		if (netbootdebug > 0)
		    Printf("bootp_read: UDP dest port not BOOTPC\n");
		goto drop;
	}
	if (netbootdebug>0) Printf("bootp_read: got bootp reply, size=%d\n", sts);

	if (sts > cnt) {
		if (netbootdebug>0)
			Printf("bootp_read: got bootp packet too large for bootp buffer\n");
		goto drop;
	}

	/*
	 * Copy data to caller buffer, skipping the IP/UDP header.
	 */
	m_copydata(m, sizeof(struct ip) + sizeof(struct udphdr), sts, buf);
	m_freem(m);
	return(sts);
drop:
	m_freem(m);
	return(-1);
}

static
csum_write(fd, blks, bytes, blk_off)
    u_int fd;
    char *blks;
    int bytes;
    u_int blk_off;
{
    u_int status;
    int i;
    static char csum_blk[DBSIZE * DEV_BSIZE];

    bcopy(blks, csum_blk, bytes);
    for (i = 0; i < bytes; i++)
        dumpinfo.csum += csum_blk[i];

    status = prom_write(fd, csum_blk, bytes, blk_off);
    if (status != bytes) {
	return 0;
    }
    return bytes;
}

#define ONE_MEGABYTE    0x00100000

/*
 *  Name: promio_dump
 *
 *  Abstract:	This routine will do the dumping to the dump device. All IO is
 *		done by using console callbacks which are common across all
 *		ALPHA platforms. They are defined in the SRM.
 *
 *  Inputs: dump_req 
 *
 *  Outputs: dump file written to dump partition
 *
 *  Return                                             
 *  Values:
 *	ESUCCESS - dump file written successfully
 *	EIO - a prom_write did not complete as expected
 */
u_int	promio_dump(dump_req)
	struct dump_request *dump_req;

{
	u_int	status = 0;	/* return status from console callbacks */
	u_int   blk_off = 0;	/* offset into the dump device for writing */
	u_int	dump_io = 0;	/* dump dev channel number for prom callbacks */
	char	buf[DEV_BSIZE]; /* temporary buffer for header and addresses */
	u_int	psize;		/* size of the dump partition */
	u_int	i;		/* loop index */

	/*
	 * open the dump device
	 */
 	if ((dump_io = prom_open(dump_req->device_name, 2)) < 0) {
 		dprintf("DUMP: can't open dump device\n");
		return(EIO);
	}

	/*
	 * clear out any remnant dump headers on the dump device which
	 * could confuse savecore
	 */
	bzero(buf, sizeof(buf));
	psize=(*bdevsw[major(dump_req->dump_dev)].d_psize)(dump_req->dump_dev);
	blk_off = dump_req->blk_offset - dumplo;
	for (i = 0; i < psize; i += DUMPOFF_INCR) {
	    prom_write(dump_io, buf, DEV_BSIZE, blk_off + i);
	}

	/*
	 * Write out the dump header, leaving the last character off the
	 * signature so that savecore can recognize an aborted dump.  The
	 * first block with the full signature will be copied after the
	 * dump completes.
	 */
	dumpinfo.partial_dump = partial_dump;
	dumpinfo.csum = 0;
	dumpinfo.addr[X_TIME] = &time;
	dumpinfo.addr[X_DUMPSIZE] = &dumpsize;
	dumpinfo.addr[X_VERSION] = version;
	dumpinfo.addr[X_PANICSTR] = &panicstr;
	dumpinfo.addr[X_MSGBUF] = &pmsgbuf;
	dumpinfo.addr[X_BLBUFPADR] = &blbufphysaddr;
	dumpinfo.addr[X_BLBUF] = &blbuf;
	dumpinfo.addr[X_BOOTEDFILE] = bootedfile;
	bcopy(DUMPINFO_SIGNATURE,dumpinfo.signature,sizeof(DUMPINFO_SIGNATURE));
	dumpinfo.signature[sizeof(DUMPINFO_SIGNATURE)-2] = '\0';
	bzero(buf, sizeof(buf));
	bcopy(&dumpinfo, buf, sizeof(dumpinfo));

	blk_off = dump_req->blk_offset;
	status = prom_write(dump_io, buf, DEV_BSIZE, blk_off);
	if (status == DEV_BSIZE) 
		blk_off++;
	else {
	        dprintf("DUMP: I/O error: bn = %d, ", blk_off);
		dprintf("status = %d\n", status);
		prom_close(dump_io);
  	        return(EIO);
	}     

        if (partial_dump) {
                vm_offset_t *blocks;
                register int count, i, total, num, bc;
 
                total = 0;
                bc = ctob(1);
                num = DEV_BSIZE/sizeof(vm_offset_t);
                blocks = (vm_offset_t *)buf;
                while((count = get_next_page(blocks, num)) != 0) {
                       	for(i=count; i<num; i++) 
				blocks[i] = 0;
			status = csum_write(dump_io, blocks, DEV_BSIZE, blk_off);
			if (status == DEV_BSIZE) 
 				blk_off++;
 	                else {
	 	                dprintf("DUMP: dump i/o error: bn = %d, ", blk_off);
	    			dprintf("status = %d\n", status);
				prom_close(dump_io);
        			return(EIO);
			}
	                for(i=0; i<count; i++) {
 				status = csum_write(dump_io, blocks[i], bc, blk_off);   
				if (status == bc) 
 					blk_off += ctod(1);
 	                        else {
 	                        	dprintf("DUMP: dump i/o error: bn = %d, ",blk_off);
	    				dprintf("status = %d\n", status);
					prom_close(dump_io);
					return(EIO);
 	                        }
 	                        if(((total + i) * NBPG) % ONE_MEGABYTE == 0)
 	                        	dprintf(".");
 	                }
 	                total += count;
		}
 	        if (total != dump_req->page_count) {
 	        	dprintf("DUMP: Mismatched dump size : Expected %d got %d\n",
 	                         dump_req->page_count, total);
			prom_close(dump_io);
			return(EIO);
		}
 	} else { /* else we do a full dump */
               	char *start;
	        register u_int page_count;
			
		page_count = ctod(dump_req->page_count);
		start = (char *)PHYS_TO_KSEG(0);

 	        while (page_count) {
 			register u_int blk,bc;
 
 	                blk = page_count > DBSIZE ? DBSIZE : page_count;
 	                bc  = blk * DEV_BSIZE;
 			status = 0;
 	
 	                status = csum_write(dump_io, start, bc, blk_off);
 	                if (status == bc) {
 	                	start += bc;
 	                        page_count -= blk;
 	                        blk_off += blk;
 	                } else {
	                        dprintf("DUMP: dump i/o error: bn = %d, ", blk_off);
	    			dprintf("status = %d\n", status);
				prom_close(dump_io);
				return(EIO);
                        }
                        if ((vm_offset_t)start % ONE_MEGABYTE == 0) 
                                dprintf(".");
 	                }
	}

	/*
	 * Put the entire signature into the dump header, as a marker of a
	 * successfully completed dump.
	 */
	blk_off = dump_req->blk_offset;
	bcopy(DUMPINFO_SIGNATURE,dumpinfo.signature,sizeof(DUMPINFO_SIGNATURE));
	bzero(buf, sizeof(buf));
	bcopy(&dumpinfo, buf, sizeof(dumpinfo));
	status = prom_write(dump_io, buf, DEV_BSIZE, blk_off);
	if (status != DEV_BSIZE) {
		dprintf("dump i/o error: bn = %d, ", blk_off);
		dprintf("status = %d\n", status);
		prom_close(dump_io);
   	        return(EIO);
	}    
	dprintf("succeeded\n");
	prom_close(dump_io);
	return(ESUCCESS);
}

/*
 *  Name: dump_chkdev
 *
 *  Abstract:	This routine checks to see if the device/partition will hold
 * 		the size of the dump we have requested. Make sure the device is
 *		alive, and any other device specific checks we need to make.
 *
 *  Inputs: dump_req 
 *
 *  Outputs: None
 *
 *  Return Values:
 *	ENOSYS - Success, and dump will be done via console call backs.
 *	ENOSPC - The dump size is too big to fit in requested disk partition
 *	ENOXIO - Device does not exist or is not alive.
 */
dump_chkdev(dump_req)
	struct dump_request *dump_req;

{
	register dev_t dev;
	daddr_t part_size, blkcnt;

	dev = dump_req->dump_dev;
	part_size = (*bdevsw[major(dev)].d_psize)(dev);
	dprintf("DUMP: partition size = %d blocks\n",part_size);

	blkcnt = ctod(dump_req->page_count);
	if (blkcnt + 1 > part_size) { /* +1 accounts for dumpinfo header added to dump */
		dprintf("DUMP: Dump device too small, requested dump size = %d, actual space available = %d.\n",blkcnt,part_size);
		return(ENOSPC);
	}

	if ((dump_req->device == 0) || (dump_req->device->alive == 0)) {
		dprintf("DUMP: Can't Open %s Dump Device 0x%x\n", dump_req->protocol, dev);
		return (ENXIO);
	}

	/* If its a SCSI device we need scsi id for the device */
	if (strcmp("rz", dump_req->device->dev_name) == 0) {
		strcpy(dump_req->protocol, "SCSI");
		dump_req->unit = dump_req->device->logunit - ((int)dump_req->device->logunit/8) * 8;
	} else {
		strcpy(dump_req->protocol, "MSCP");
		dump_req->unit = dump_req->device->logunit;
	}

	return(ENOSYS);
}


/*
 *  Name: dumpsys
 *
 *  Abstract:   This routine dumps memory to the dump device.  It is either
 * 		called from panic above or from doadump in locore.s (on an
 * 		error halt).
 *
 *  Inputs: Globals: dumpdev - the dev_t of the device to dump to
 *		     partial_dump - boolean for partial dumps
 *
 */
dumpsys()
{
	struct	dump_request dump_req;	    /* Pointer to dump info block */
	u_int status;			    /* Return status */
	extern int printstate;

	if (dont_dump)
		return;
	if (dumpdev == NODEV) {
		dprintf("DUMP: No dump device.\n");
		return;
	}
	if (dumpsize++)
		return;		/* handle recursive call to dumpsys */
	if (partial_dump)
		dumpsize = num_kernel_pages(); /* number of pages for dump */
	else
		dumpsize = physmem; /* number of pages of physical memory */
	if (!dumpsize)
		return;		/* avoid pageless dumps */

	/*
	 * Compute the dump offset on a 2MB boundary.  Not dumping to the
	 * beginning of the dump increases our chances of not swapping on
	 * the dump before it can be saved.  We also subtract a block from the
	 * offset to save room for the header.
	 */
	dumplo = (*bdevsw[major(dumpdev)].d_psize)(dumpdev)-(ctod(dumpsize)+1);
	dumplo = dumplo > DUMPOFF_MAX ? DUMPOFF_MAX : dumplo;
	dumplo = dumplo < 0 ? 0 : dumplo / DUMPOFF_INCR * DUMPOFF_INCR;

	/*
	 * If console is a graphics device, force printf messages directly
	 * to screen. This is needed here for the case when we manually start
	 * the dump routine from console mode.
	 */
	printstate |= PANICPRINT;

	dprintf(
	"\r\nDUMP: Attempting %s dump to dev 0x%x, offset %d, numpages %d.\r\n",
		(partial_dump) ? "partial" : "full",
		dumpdev, dumplo, dumpsize);

	dump_req.page_count  = dumpsize;
	dump_req.dump_dev = dumpdev;
	dump_req.blk_offset = dumplo;

	status = (*bdevsw[major(dumpdev)].d_dump)(&dump_req);

	if (status == ENOSYS)	/* If status is ENOSYS than IO has NOT been done in the driver */ 
		status = dump_chkdev(&dump_req);
	else 			
		return;

	/* If there wasn't enough space and we were doing a full dump try a parital dump before giving up */
	if (status == ENOSPC && !partial_dump) {		
		dprintf("DUMP: Dump device too small for full dump.\n");
		dump_req.page_count = num_kernel_pages();
		dumpsize = dump_req.page_count;
		if (dump_req.page_count) {
			dprintf("DUMP: Attempting partial dump of %d pages.\n",dump_req.page_count);
			partial_dump = 1;
			/*
			 * When we revert to partials, we don't offset the dump.
			 * This should be fixed in Gold.
			 */
			dump_req.blk_offset = dumplo;
			status = (*bdevsw[major(dumpdev)].d_dump)(&dump_req);   
			if (status == ENOSYS)
				status = dump_chkdev(&dump_req);
			else
				return;
		}
	}
	if (status == ENOSYS) {	/* Now we write out to the device using console callbacks. refer to the SRM. */
		u_int dump_io;

		dprintf("DUMP: Block offset %d\n",dump_req.blk_offset);
		dprintf("DUMP: Block count %d\n", ctod(dump_req.page_count));

		trans_dumpdev(&dump_req);	/* get the device string from the system specific call */
		dprintf("DUMP: %s\n",dump_req.device_name);
		status = promio_dump(&dump_req);

	} else if (status == ENOSPC && partial_dump) 
		dprintf("DUMP: Dump device too small for partial dump.\n");
	else {
		switch (status) {
			case ENXIO:
		       		dprintf("DUMP: device bad\n");
				break;

			case EFAULT:
				dprintf("DUMP: device not ready\n");
				break;

			case EINVAL:
				dprintf("DUMP: area improper\n");
				break;

			case EIO:
				dprintf("DUMP: i/o error\n");
				break;

		}
	}
	return;
}
/* 
 * Handle common configure operations and
 * call the machine dependent configure routine.
 */
configure()
{
	extern int cold;
	extern int lvprobe();

	cold = 1;

#ifndef __alpha
	/*
 	 * Initialize PROM environment entries.
	 */
	hwconf_init();

	/*
	 * Initialize interrupt handler kernel framework.
	 * Initialize loadable driver framework.
	 */
	handler_init();
	loadable_driver_init();
#endif /* !__alpha */

	/*
	 * Call machine dependent code to handle machine specific
	 * operations
	 */
	machine_configure();

	/*
	 * Configure logical volume manager(s).
	 */
	lvprobe(0);

	/*
	 * Configure swap area and related system
	 * parameter based on device(s) used.
	 */
	setconf();
#ifndef __alpha
	swapconf();

	/*
	 * Safe to take interrupts now.
	 */
	splnone();
#endif /* !__alpha */

	cold = 0;
	return (0);
}

/*
 * Delay for n microseconds
 * Call through the system switch to specific delay routine.
 */
microdelay(usecs)
	int usecs;
{
	(*(cpup->microdelay))(usecs);
}

write_utext(addr, val)
u_long *addr;
u_int val;
{
	vm_offset_t start_addr;
	int ret;
	vm_prot_t write_prot;

	start_addr = trunc_page(((vm_offset_t)addr));

	write_prot = 0;
        if (vm_map_check_protection(current_thread()->task->map, start_addr,
			  start_addr + PAGE_SIZE, VM_PROT_WRITE))
		write_prot = VM_PROT_WRITE;

	if (vm_map_protect( current_thread()->task->map,
		start_addr, start_addr + PAGE_SIZE,
		VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE, FALSE))
		return 0;

	ret = suiword(addr, val);

	if (vm_map_protect( current_thread()->task->map,
		start_addr, start_addr + PAGE_SIZE,
		VM_PROT_READ | VM_PROT_EXECUTE | write_prot, FALSE))
		return 0;

	if (ret < 0)
		return EIO;

	imb();

	return 0;
}

/*****************************************************************************
 *
 *      This section has smp related routines
 *
 *****************************************************************************/

slave_config()
{
        /* called from kern/slave.c */
}

/*
 * This atoi came with minor mods from libc.
 * It's intended use is for those
 * few cases where the conversion of a string to
 * an integer in the kernel is required like
 * generic boot code. Please do not abuse it.
 */
atoi(s1)
register char *s1;
{
        register int n;
        register int f;

        n = 0;
        f = 0;
        for(;;s1++) {
		switch(*s1) {
                case ' ':
                case '\t':
                        continue;
                case '-':
                        f++;
                case '+':
                        s1++;
                }
                break;
	}
	while(*s1 >= '0' && *s1 <= '9')
                n = n*10 + *s1++ - '0';
        return(f ? -n : n);
}

itoa(number, string)	/* integer to ascii routine */
int number;
char string[];
{
	int a, b, c;
	int i, sign;
	if (( sign = number) < 0)
		number = number * -1;
	i = 0;
	do {
		string[i++] = number % 10 + '0';
	} while ((number /= 10) > 0);
	if (sign < 0)
		string[i++] = '-';
	string[i] = '\0';
	/* flip the string in place */
	for (b = 0, c = strlen(string)-1; b < c; b++, c--) {
		a = string[b];
		string[b] = string[c];
		string[c] = a;
	}
	return( strlen(string) );
}

static char *
ntoa(in)	/* internet address to printable ascii string routine */
struct in_addr in;
{
        register unsigned char *p;
        register char *b;
        static char buf[20];
        int i;

        p = (unsigned char *)(&in);
        b = buf;
        for (i=0; i<4; i++) {
                if (i) *b++ = '.';
                if (*p > 99) {
                        *b++ = '0' + (*p / 100);
                        *p %= 100;
                }
                if (*p > 9) {
                        *b++ = '0' + (*p / 10);
                        *p %= 10;
                }
                *b++ = '0' + *p;
                p++;
        }
        *b++ = 0;
        return(buf);
}


/*
 * allocate global state variable for system correctable error
 * (sce) handling.
 * 
 * sce_period ('Tp') represents the period for which sce
 * (processor correctable error) statistics are maintained.  It is
 * initialized by sce_init().
 *
 * sce_last ('Tl') represents the time at which the last sce occurred.
 * It is initialized to zero.
 *
 * sce_first ('To') represents the time at which the sce statistical
 * period began. 
 *
 * sce_threshold ('Ft') represents the error threshold at which sce
 * reporting is disabled.
 *
 * sce_warn ('Fw') represents the error threshold at which a warning
 * is issued.
 *
 * sce_count ('Fc') represents the statistical number of errors which
 * have occurred in the present period.
 *
 * mces_state contains the expected state of the mces read/write bits.
 */

long sce_period;	/* 'Tps' - static */
long sce_last;		/* 'Tls' */
long sce_first;		/* 'Tos' */
long sce_threshold;	/* 'Fts' - static */
long sce_warn;		/* 'Fws' - static */
long sce_count;		/* 'Fcs' */

#define Tps sce_period
#define Txs sce_now
#define Tls sce_last
#define Tos sce_first
#define Fts sce_threshold
#define Fws sce_warn
#define Fcs sce_count

/*
 * mces_sce_init(threshold, period, warn):
 *
 *   inputs:
 *	threshold: the number of errors that can occur in a 'period'
 *	 	before sce reporting is disabled.
 *
 *	period: the length of time in time.tv_sec units in which
 *		'threshold' errors can occur before sce error
 *		reporting is disabled. 
 *
 *	warn: the number of errors which can occur in a 'period'
 *		before a warning is given... 
 *		(functionality not implemented) 
 *
 *	mcse_state: implicit input of global variable
 *
 *   outputs:
 *	Adjusted inputs. Potential MCES state and mces_state if
 * 	threshold is exceeded.
 *
 *   environment:
 *	mces_state:
 *	  the present state of the MCES read/write bits.
 * 	  This should be used to 'or' with w1c bits when clearing
 *	  specific error bits.
 *	
 *   abstract:
 * 	intialize environment required by mces_sce_xxx routines. It is 
 *	expected that this routine will be called from the platform
 *	specific error handler initialization code.
 *
 */
mces_sce_init(threshold, period, warn)
	long threshold;
	long period;
	long warn;
{
	long Txs = time.tv_sec; 	/* present time */

	Fts = threshold;
	Fws = warn;
	Fcs = 0;

	Tps = period;
	Txs = time.tv_sec;
	Tls = Txs;
	Tos = Txs;
	
	mces_sce_enable();
}


/*
 * mces_pce_handle:
 *   inputs:	(see #defines above)
 *	Tps 	- implicit. Global containing period (in
 *		  time.tv_sec units) over which thresholding
 *		  is maintained.
 *      Tls	- implicit. global containing timestamp of
 *		  last pce.
 *	Tos	- implicit. global containing start time of
 *		  present threshold period.
 *	Fcs	- implicit. global containing error counter
 *		  for threshold period.
 *	Fts	- implicit. global continaing the maximum
 *		  value for Fc before disabling pce reporting.
 */

mces_sce_handle()
{
	long sce_now = time.tv_sec; 	/* 'Txs' present time */
/*
 * 'Tps' represents the period for which sce (system correctable
 * error) statistics are maintained.  It is contained in sce_period,
 * which is initialized by mces_sce_init().
 *
 * 'Txs' represents the time at which the present sce occurred and is
 * taken from the time.tv_sec.
 *
 * 'Tls' represents the time at which the last sce occurred. It is
 * initialized to the present time, and set to 'Txs' after all sce
 * handling is complete.
 *
 * 'Tos' represents the time at which the first sce for the present
 *      period occurred.
 *  - 'Tos' is set to the present time at initialization.
 *  - 'Tos' is set to 'Txs' when 'Txs' - 'Tls' > 'Tps'
 *  - 'Tos' is also adjusted when 'Txs' - 'Tos' > 'Tps' to 'Txs' - 'Tps'.
 *
 * 'Fts' represents the error threshold at which sce reporting is
 * disabled.
 * 
 * 'Fcs' represents the pce frequency. 
 *  - It is incremented each time a sce occurs. 
 *  - 'Fcs' is set to zero whenever 'Txs' - 'Tls' > 'Tps'. 
 *  - 'Fc' is decremented when 'Txs' - 'Tos' > 'Tps' by the integer
 * 	   portion of ((('Txs' - 'Tps') - 'Tos')/('Fts'/'Tps'))
 */
	if ((Txs - Tls) > Tps)
	{
		/* More than a period has gone by since the last pce */
		/* reset start of period to now and count to 0. */

		Tos = Txs;	/* set 'Tos' to 'Txs' */
		Fcs = 0;		/* set 'Fcs' to zero */
	}

	if ((Txs - Tos) > Tps)
	{
		/* More than a period has gone by since the first sce */
		/* Adjust start of period and frequency accordingly */

		/* adjust the frequency statistic downward to account */
		/* for (mean) errors in time before the adjusted 'To' */
		Fcs = (((Fcs - (((Txs - Tps) - Tos)/(Fts/Tps))) > 0)
		      ? (Fcs - (((Txs - Tps) - Tos)/(Fts/Tps)))
		      : 0);

		/* adjust 'To' to a period ago */
		Tos = Txs - Tps;
	}

	Tls = Txs;	/* set 'Tls' to 'Txs' */
	Fcs ++;		/* increment 'Fcs' */

	/* turn off pce reporting if the threshold has been crossed */
	if (Fcs >= Fts)
	{
		mces_sce_disable();
		printf ("WARNING: too many System corrected errors. Reporting suspended.\n\r");
	}
}
/*
 * allocate global state variable for processor correctable error
 * (pce) handling.
 * 
 * pce_period ('Tp') represents the period for which pce
 * (processor correctable error) statistics are maintained.  It is
 * initialized by pce_init().
 *
 * pce_last ('Tl') represents the time at which the last pce occurred.
 * It is initialized to zero.
 *
 * pce_first ('To') represents the time at which the pce statistical
 * period began. 
 *
 * pce_threshold ('Ft') represents the error threshold at which pce
 * reporting is disabled.
 *
 * pce_warn ('Fw') represents the error threshold at which a warning
 * is issued.
 *
 * pce_count ('Fc') represents the statistical number of errors which
 * have occurred in the present period.
 *
 * mces_state contains the expected state of the mces read/write bits.
 */

long pce_period;	/* 'Tp' - static */
long pce_last;		/* 'Tl' */
long pce_first;		/* 'To' */
long pce_threshold;	/* 'Ft' - static */
long pce_warn;		/* 'Fw' - static */
long pce_count;		/* 'Fc' */
long mces_state;

#define Tp pce_period
#define Tx pce_now
#define Tl pce_last
#define To pce_first
#define Ft pce_threshold
#define Fw pce_warn
#define Fc pce_count


/*
 * mces_pce_init(threshold, period, warn):
 *
 *   inputs:
 *	threshold: the number of errors that can occur in a 'period'
 *	 	before pce reporting is disabled.
 *
 *	period: the length of time in time.tv_sec units in which
 *		'threshold' errors can occur before pce error
 *		reporting is disabled. 
 *
 *	warn: the number of errors which can occur in a 'period'
 *		before a warning is given... 
 *		(functionality not implemented) 
 *
 *	mcse_state: implicit input of global variable
 *
 *   outputs:
 *	Adjusted inputs. Potential MCES state and mces_state if
 * 	threshold is exceeded.
 *
 *   environment:
 *	mces_state:
 *	  the present state of the MCES read/write bits.
 * 	  This should be used to 'or' with w1c bits when clearing
 *	  specific error bits.
 *	
 *   abstract:
 * 	intialize environment required by mces_pce_xxx routines. It is 
 *	expected that this routine will be called from the platform
 *	specific error handler initialization code.
 *
 */
mces_pce_init(threshold, period, warn)
	long threshold;
	long period;
	long warn;
{
	long Tx = time.tv_sec; 	/* present time */

	Ft = threshold;
	Fw = warn;
	Fc = 0;

	Tp = period;
	Tx = time.tv_sec;
	Tl = Tx;
	To = Tx;
	
	mces_pce_enable();
}


/*
 * mces_pce_handle:
 *   inputs:	(see #defines above)
 *	Tp 	- implicit. Global containing period (in
 *		  time.tv_sec units) over which thresholding
 *		  is maintained.
 *      Tl	- implicit. global containing timestamp of
 *		  last pce.
 *	To	- implicit. global containing start time of
 *		  present threshold period.
 *	Fc	- implicit. global containing error counter
 *		  for threshold period.
 *	Ft	- implicit. global continaing the maximum
 *		  value for Fc before disabling pce reporting.
 */

mces_pce_handle()
{
	long pce_now = time.tv_sec; 	/* 'Tx' present time */
/*
 * 'Tp' represents the period for which pce (processor correctable
 * error) statistics are maintained.  It is contained in pce_period,
 * which is initialized by mces_pce_init().
 *
 * 'Tx' represents the time at which the present pce occurred and is
 * taken from the time.tv_sec.
 *
 * 'Tl' represents the time at which the last pce occurred. It is
 * initialized to the present time, and set to 'Tx' after all pce
 * handling is complete.
 *
 * 'To' represents the time at which the first pce for the present
 *      period occurred.
 *  - 'To' is set to the present time at initialization.
 *  - 'To' is set to 'Tx' when 'Tx' - 'Tl' > 'Tp'
 *  - 'To' is also adjusted when 'Tx' - 'To' > 'Tp' to 'Tx' - 'Tp'.
 *
 * 'Ft' represents the error threshold at which pce reporting is
 * disabled.
 * 
 * 'Fc' represents the pce frequency. 
 *  - It is incremented each time a pce occurs. 
 *  - 'Fc' is set to zero whenever 'Tx' - 'Tl' > 'Tp'. 
 *  - 'Fc' is decremented when 'Tx' - 'To' > 'Tp' by the integer
 * 	   portion of ((('Tx' - 'Tp') - 'To')/('Ft'/'Tp'))
 */
	if ((Tx - Tl) > Tp)
	{
		/* More than a period has gone by since the last pce */
		/* reset start of period to now and count to 0. */

		To = Tx;	/* set 'To' to 'Tx' */
		Fc = 0;		/* set 'Fc' to zero */
	}

	if ((Tx - To) > Tp)
	{
		/* More than a period has gone by since the first pce */
		/* Adjust start of period and frequency accordingly */

		/* adjust the frequency statistic downward to account */
		/* for (mean) errors in time before the adjusted 'To' */
		Fc = (((Fc - (((Tx - Tp) - To)/(Ft/Tp))) > 0)
		      ? (Fc - (((Tx - Tp) - To)/(Ft/Tp)))
		      : 0);

		/* adjust 'To' to a period ago */
		To = Tx - Tp;
	}

	Tl = Tx;	/* set 'Tl' to 'Tx' */
	Fc ++;		/* increment 'Fc' */

	/* turn off pce reporting if the threshold has been crossed */
	if (Fc >= Ft)
	{
		mces_pce_disable();
		printf ("WARNING: too many corrected errors. Reporting suspended.\n\r");
	}
}

mces_pce_disable()
{
	mces_state |= MCES_DPC;
	mtpr_mces(mces_state);
	mb();
}

mces_pce_enable()
{
	mces_state &= ~MCES_DPC;
	mtpr_mces(mces_state);
	mb();
}

mces_sce_disable()
{
	mces_state |= MCES_DSC;
	mtpr_mces(mces_state);
	mb();
}

mces_sce_enable()
{
	mces_state &= ~MCES_DSC;
	mtpr_mces(mces_state);
	mb();
}

mces_sce_clear()
{
	mtpr_mces(mces_state | MCES_SCE);	/* write 1 to clear */
	mb();
}
mces_pce_clear()
{
	mtpr_mces(mces_state | MCES_PCE);	/* write 1 to clear */
	mb();
}

mces_mcheck_clear()
{
	mtpr_mces(mces_state | MCES_MCK);	/* write 1 to clear */
	mb();
}

write_instruction(address, instruction)
	vm_offset_t     address;
	unsigned int instruction;
{
	vm_offset_t phys;

	if (!IS_KSEG_VA(address)) {
		/* mapped kernel -- write at corresponding kseg space */
		svatophys(address, &phys);
		*(unsigned int *)PHYS_TO_KSEG(phys) = instruction;
	} else {
		/* kseg kernel */
		*(unsigned int *)address = instruction;
	}
	imb();
}

int swap_ipl();
enable_spls()
{
	write_instruction(&swap_ipl,0x35);  /* call_pal        swpipl */
}

/* A change was made to the HWRPB via an ECO and the following code tracks
 * a part of the ECO.  The HWRPB version must be 5 or higher or the ECO
 * was not implemented in the console firmware.  If its at rev 5 or greater
 * we can get the platform ascii string name from the HWRPB.  Thats what this
 * function does.  It checks the rev level and if the string is in the HWRPB
 * it returns the addtess of the string ... a pointer to the platform name.
 *
 * Returns:
 *	- Pointer to a ascii string if its in the HWRPB
 *	- Pointer to a "Unknown ..." string if the data is not in the HWRPB.
 */
char *
platform_string()
{
	struct rpb_dsr *dsr;
	extern struct rpb *rpb;
	static char    Unk_system_string[] = "Unknown System Type";


	/* Go to the console for the string pointer.
	 * If the rpb_vers is not 5 or greater the rpb
	 * is old and does not have this data in it.
	 */
	if(rpb->rpb_vers < 5 ){
		return (Unk_system_string);
	} else {
		/* The Dynamic System Recognision struct
		 * has the system platform name starting
		 * after the character count of the string.
		 */
		dsr =  ((struct rpb_dsr *) ((char *)rpb +
		       (rpb->rpb_dsr_off)));
		return ((char *)((char *) dsr + (dsr->rpb_sysname_off + sizeof(long))));
	}
}

/* To make it possible to ship platform support for new systems
 * without software changes, if all the support is there and its
 * simply a new configuration, we can now get licensing info
 * from the console via the HWRPB.  This routine will return:
 *	- The SMM value is its in the HWRPB.
 *	- Zero (0) if the value is not in the HWRPB, ie. its an
 *	  platform that has no knowledge of the new SMM value.
 */
long
get_platform_smm()
{
	struct rpb_dsr *dsr;
	extern struct rpb *rpb;
	/* If the rpb_vers is not 5 or greater the rpb
	 * is old and does not have this data in it.
	 */
	if(rpb->rpb_vers < 5 ){
		return (0);
	} else {
		/* The Dynamic System Recognision struct
		 * has the system smm as the first element.
		 */
		dsr =  ((struct rpb_dsr *) ((char *)rpb +
		       (rpb->rpb_dsr_off)));
		return ( dsr->rpb_smm);
	}

}
