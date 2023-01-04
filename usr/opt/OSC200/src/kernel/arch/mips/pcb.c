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
static char	*sccsid = "@(#)$RCSfile: pcb.c,v $ $Revision: 1.2.4.7 $ (DEC) $Date: 1992/07/08 08:42:13 $";
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
 * derived from pcb.c	2.2     (ULTRIX/OSF) 1/16/91";
 */

/*
 *	File:	mips/pcb.c
 *	Author:	A. Forin
 *
 *	Copyright (C) 1989, A. Forin
 *
 *	Mips PCB management
 */

/*
 * pcb.c
 *
 *      Modification History:
 *
 * 14-Jan-91   David Metsky
 *      osf supplied fix to default interrupt mask
 *
 */

#include <kern/thread.h>
#include <mach/thread_status.h>
#include <machine/reg.h>


/*
 * rpbfix: put in comment
 */
u_int sr_usermask = (SR_IMASK0|SR_IEP|SR_KUP);   /* disable IEc, enable IEp, prev umode */
u_int sr_kernelmask = (SR_IMASK0|SR_IEP|SR_IEC); /* enable IEc, enable IEp */

/*
 *	pcb_init:
 *
 *	Initialize the pcb for a thread.  For Mips,
 *	also initializes the coprocessor(s).
 *
 */
void pcb_init(thread, ksp)
	register thread_t	thread;
	register vm_offset_t	ksp;
{
	register struct pcb	*pcb = thread->pcb;
	register struct mips_saved_state		*saved_state;

	int	thread_bootstrap();

	bzero(pcb, sizeof(struct pcb));

	/*
	 *	Set up thread to start at user bootstrap.  The stack
	 *	pointer is setup to point to the frame that corresponds
	 *	to the user's state (thread_bootstrap will perform a
	 *	ret instruction to simulate returning from a trap).
	 *	The frame pointer is left 0, for debuggers.
	 *
	 */
	pcb->pcb_regs[PCB_PC] = (int) thread_bootstrap;

	saved_state = (struct mips_saved_state *)
				(((int)pcb) 
				- sizeof(struct mips_saved_state));

	saved_state->sr = sr_usermask;
	pcb->pcb_regs[PCB_SP] = ((int) PCB_WIRED_ADDRESS) -EF_SIZE;
	pcb->pcb_kstack = 1;	/* nonzero for kernel stack */

	pcb->u_address.uthread = thread->u_address.uthread;
	pcb->u_address.utask = thread->u_address.utask;
	/*
	 * The floating point coprocessor is reloaded as
	 * needed from the pcb.  Zeroing the pcb is enough.
	 * Same applies to other coprocessors, if any.
	 */

	reset_uu_ar0( thread );
}

/*
 *  	pcb_terminate:
 *
 *	Shutdown any state associated with a thread's pcb.
 *	Also, release any coprocessor(s) register state.
 */
void pcb_terminate(thread)
struct thread *thread;
{
	checkfp(thread,1);
}


/*
 *	pcb_synch:
 *
 *	Stores registers that aren't saved by the kernel at context
 *	switch time into the pcb.  On Mips, this only means the
 *	coprocessor(s) registers.  Other tricks apply to partial
 *	frame save/restore.
 *	
 */

pcb_synch(thread)
thread_t thread;
{
	checkfp(thread,0);
}

/*
 *	initial_context:
 *
 *	Set up the context for the very first thread to execute
 *	(which is passed in as the parameter).
 */
void initial_context(thread)
	thread_t	thread;
{
	struct pcb *pcb=thread->pcb;

	active_threads[cpu_number()] = thread;
	pmap_activate(vm_map_pmap(thread->task->map), thread, cpu_number());

	pcb->u_address.uthread = active_threads[cpu_number()]->u_address.uthread;
	pcb->u_address.utask   = active_threads[cpu_number()]->u_address.utask;
	pcb->pcb_regs[PCB_SR] = sr_kernelmask;
	pcb->pcb_kstack = 1;/* we're on the kernel stack */
#if NCPUS > 1
	thread->pcb->pcb_cpu_number = cpuident();	
#endif
	reset_uu_ar0( thread );
}

/*
 *	thread_start:
 *
 *	Start a thread at the specified routine.  The thread must
 *	be in a suspended state.
 */

thread_start(thread, start, mode)
	register thread_t	thread;
	void			(*start)();
	int			mode;
{
	register struct pcb	*pcb;
	vm_offset_t		 sp;

	pcb = thread->pcb;

	pcb->pcb_regs[PCB_PC] = (int) start;

	if (mode == THREAD_USERMODE)
		pcb->pcb_regs[PCB_SR] = sr_usermask;
	else
		pcb->pcb_regs[PCB_SR] = sr_kernelmask;

	reset_uu_ar0( thread );


}


/*
 *	thread_setstatus:
 *
 *	Set the status of the given thread.
 */

kern_return_t thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		count;
{
	register struct mips_thread_state	*state;
	register struct mips_saved_state	*saved_state;


	switch (flavor) {

	case MIPS_THREAD_STATE:

		if (count < MIPS_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		state = (struct mips_thread_state *) tstate;
		saved_state = USER_REGS(thread);

		saved_state->r1 = state->r1;
		saved_state->r2 = state->r2;
		saved_state->r3 = state->r3;
		saved_state->r4 = state->r4;
		saved_state->r5 = state->r5;
		saved_state->r6 = state->r6;
		saved_state->r7 = state->r7;
		saved_state->r8 = state->r8;
		saved_state->r9 = state->r9;
		saved_state->r10 = state->r10;
		saved_state->r11 = state->r11;
		saved_state->r12 = state->r12;
		saved_state->r13 = state->r13;
		saved_state->r14 = state->r14;
		saved_state->r15 = state->r15;
		saved_state->r16 = state->r16;
		saved_state->r17 = state->r17;
		saved_state->r18 = state->r18;
		saved_state->r19 = state->r19;
		saved_state->r20 = state->r20;
		saved_state->r21 = state->r21;
		saved_state->r22 = state->r22;
		saved_state->r23 = state->r23;
		saved_state->r24 = state->r24;
		saved_state->r25 = state->r25;
		saved_state->r26 = state->r26;
		saved_state->r27 = state->r27;
		saved_state->r28 = state->r28;
		saved_state->r29 = state->r29;
		saved_state->r30 = state->r30;
		saved_state->r31 = state->r31;
		saved_state->mdlo  = state->mdlo;
		saved_state->mdhi  = state->mdhi;
		saved_state->epc = state->pc;

		break;

	case MIPS_FLOAT_STATE:

		if (count < MIPS_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		thread->pcb->pcb_ownedfp = 1;
		checkfp(thread,0);	/* make sure we'll reload fpu */

		/* NOTE: this stmt knows about the pcb reg layout */
		bcopy(tstate, thread->pcb->pcb_fpregs, sizeof(struct mips_float_state));

		break;
	default:
		return(KERN_INVALID_ARGUMENT);
	}

	return(KERN_SUCCESS);

}

/*
 *	thread_getstatus:
 *
 *	Get the status of the specified thread.
 */

kern_return_t thread_getstatus(thread, flavor, tstate, count)
	register thread_t	thread;
	int			flavor;
	thread_state_t		tstate;		/* pointer to OUT array */
	unsigned int		*count;		/* IN/OUT */
{
	register struct mips_thread_state	*state;
	register struct mips_saved_state	*saved_state;

	switch (flavor) {
	case THREAD_STATE_FLAVOR_LIST:
		if (*count < 2)
			return(KERN_INVALID_ARGUMENT);
		tstate[0] = MIPS_THREAD_STATE;
		tstate[1] = MIPS_FLOAT_STATE;
		*count = 2;

		break;
	
	case MIPS_THREAD_STATE:

		if (*count < MIPS_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		state = (struct mips_thread_state *) tstate;
		saved_state = USER_REGS(thread);

		state->r1 = saved_state->r1;
		state->r2 = saved_state->r2;
		state->r3 = saved_state->r3;
		state->r4 = saved_state->r4;
		state->r5 = saved_state->r5;
		state->r6 = saved_state->r6;
		state->r7 = saved_state->r7;
		state->r8 = saved_state->r8;
		state->r9 = saved_state->r9;
		state->r10 = saved_state->r10;
		state->r11 = saved_state->r11;
		state->r12 = saved_state->r12;
		state->r13 = saved_state->r13;
		state->r14 = saved_state->r14;
		state->r15 = saved_state->r15;
		state->r16 = saved_state->r16;
		state->r17 = saved_state->r17;
		state->r18 = saved_state->r18;
		state->r19 = saved_state->r19;
		state->r20 = saved_state->r20;
		state->r21 = saved_state->r21;
		state->r22 = saved_state->r22;
		state->r23 = saved_state->r23;
		state->r24 = saved_state->r24;
		state->r25 = saved_state->r25;
		state->r26 = saved_state->r26;
		state->r27 = saved_state->r27;
		state->r28 = saved_state->r28;
		state->r29 = saved_state->r29;
		state->r30 = saved_state->r30;
		state->r31 = saved_state->r31;
		state->mdlo  = saved_state->mdlo;
		state->mdhi  = saved_state->mdhi;
		state->pc  = saved_state->epc;

		*count = MIPS_THREAD_STATE_COUNT;
		break;

	case MIPS_FLOAT_STATE:

		if (*count < MIPS_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		checkfp(thread,0);
		/* NOTE: this knows about the pcb reg layout */
		bcopy(thread->pcb->pcb_fpregs, tstate, sizeof(struct mips_float_state));

		*count = MIPS_FLOAT_STATE_COUNT;

		return(KERN_SUCCESS);

		break;

	default:
		return(KERN_INVALID_ARGUMENT);
	}
	return(KERN_SUCCESS);
}

#include <sys/proc.h>
#include <machine/fpu.h>

/*
 *	thread_dup:
 *
 *	Duplicate the user's state of a thread.  This is only used to perform
 *	the Unix fork operation.
 */
thread_dup(parent, child)
	register thread_t	parent, child;
{
	register struct mips_saved_state	*parent_state,
						*child_state;

	reset_uu_ar0( child );
	parent_state = USER_REGS(parent);
	child_state  = USER_REGS(child);
	*child_state = *parent_state;

	/*
	 * Following U*x rules, return the triple [pid,1,noerror] in
	 * the child.  The parent gets back to U*x, where it will
	 * return [pid,0,noerror].
	 */
	child_state->r2 = proc[child->task->proc_index].p_pid;
	child_state->r3 = 1;
	child_state->r7 = 0;

	/*
	 * If parent used coprocessor(s) must copy their state
	 * in child.
	 */
	if (parent->pcb->pcb_ownedfp) {
		/* Save state in pcb, disable coproc for parent */
		checkfp(parent, 0);
		bcopy(parent->pcb->pcb_fpregs, child->pcb->pcb_fpregs,
			32 * sizeof(int));
		child->pcb->pcb_fpc_csr = parent->pcb->pcb_fpc_csr & ~FPCSR_EXCEPTIONS;
		child->pcb->pcb_fpc_eir = parent->pcb->pcb_fpc_eir;
		/*
		 * If the parent used any coprocessor must disable
		 * its use in the child.
		 * Child may or may not use coprocessor(s).
		 */
		disable_fp(child);
	}
	
	child->pcb->pcb_kstack = 1;

	/* start /proc changes */
	/* If the stop-on-thread-create bit is set, stop the task */
	if(parent->task->procfs.pr_qflags & PRFS_STOPTCR) {
		parent->task->procfs.pr_flags |=
		  (PR_STOPPED | PR_ISTOP | PR_PCINVAL);
		parent->task->procfs.pr_why = PRFS_STOPTCR;
		parent->task->procfs.pr_what = (long )parent;
		wakeup(&parent->task->procfs);
		thread_suspend(parent);
		thread_block(parent);
	}
	if (parent->t_procfs.pr_flags & PR_FORK)
	    child->t_procfs = parent->t_procfs;
	else {
	    bzero(&child->t_procfs, sizeof(struct t_procfs) );
	}
	/* end /proc changes */

	/* XXX remove breakpoints in child */
}


reset_uu_ar0( thread )
	register thread_t thread;
{
	thread->u_address.uthread->uu_ar0 = (int *) USER_REGS(thread);
}
