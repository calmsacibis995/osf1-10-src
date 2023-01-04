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
static char *rcsid = "@(#)$RCSfile: pcb.c,v $ $Revision: 1.2.13.2 $ (DEC) $Date: 1993/04/23 21:07:48 $";
#endif
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
 *	File:	alpha/pcb.c
 *	Author:	A. Forin
 *
 *	Copyright (C) 1989, A. Forin
 *
 *	alpha PCB management
 */

/*
 *
 *      Modification History: alha/pcb.c
 *
 * 24-May-91   afd
 *      ported mips version for alpha use
 *
 * 24-Jan-91   David Metsky
 *      osf supplied fix to default interrupt mask
 *
 */

#include <kern/thread.h>
#include <mach/thread_status.h>
#include <mach/vm_param.h>
#include <machine/machparam.h>
#include <machine/psl.h>

struct	u_address	U_ADDRESS;

/*
 *	pcb_init:
 *
 *	Initialize the pcb for a thread.
 *
 */
void
pcb_init(thread, ksp)
	register thread_t	thread;
	register vm_offset_t	ksp;
{
	register struct pcb			*pcb = thread->pcb;
	register struct alpha_saved_state	*saved_state;
	vm_offset_t				prefault;

	int	thread_bootstrap();
	pt_entry_t *	pmap_pte();

	bzero(pcb, sizeof(struct pcb));

	/*
	 *	Set up thread to start at user bootstrap.  The stack
	 *	pointer is setup to point to the frame that corresponds
	 *	to the user's state (thread_bootstrap will perform a
	 *	ret instruction to simulate returning from a trap).
	 *	The frame pointer is left 0, for debuggers.
	 *
	 */
	pcb->pcb_regs[PCB_PC] = (long) thread_bootstrap;

	/* Make sure the stack doesn't take write faults when it's active. */
	for (prefault = ksp;
	     prefault < ksp + KERNEL_STACK_SIZE;
	     prefault += ALPHA_PGBYTES)
		*(long *)prefault = 0;

	saved_state = (struct alpha_saved_state *)
				(ksp + KERNEL_STACK_SIZE
				- sizeof(struct alpha_saved_state));

	/*
	 * Setup the new kernel stack pointer
	 */
	pcb->pcb_ksp = (long)saved_state;
	pcb->pcb_regs[PCB_SP] = (long)saved_state;
	pcb->pcb_regs[PCB_PS] = SPLHIGH; /* kernel mode SPL high */

	/*
	 * Get the physical address of the pcb for context switching.
	 */
	svatophys(pcb, &pcb->pcb_paddr);

	/* 
	 * Get the page frame of the root page table from the task
	 * pmap.
	 */
	pcb->pcb_ptbr =
	     KSEG_TO_PHYS(vm_map_pmap(thread->task->map)->level1_pt) >> PGSHIFT;

	/*
	 * Propagate the PME bit if necessary
	 */
	if (current_thread() && (current_thread()->pcb->pcb_fen & PCB_PME_BIT))
		thread->pcb->pcb_fen = PCB_PME_BIT;

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
 *	Nothing to do on alpha.
 */
void
pcb_terminate(thread)
	struct thread *thread;
{
}


/*
 *	pcb_synch:
 *
 *	Stores registers that aren't saved by the kernel at context
 *	switch time into the pcb.  On alpha, nothing to do.
 *	
 */

pcb_synch(thread)
	thread_t thread;
{
	return(0);
}


/*
 *	pcb_fixup:
 *
 *	Fix up anything required to swap a thread back in.
 *	Currently, alpha just needs to re-compute the pcb_paddr.
 */
pcb_fixup(thread)
	struct thread *thread;
{
	(void)svatophys(thread->pcb,&thread->pcb->pcb_paddr);
}


/*
 *	initial_context:
 *
 *	Set up the context for the very first thread to execute
 *	(which is passed in as the parameter).
 */
void
initial_context(thread)
	thread_t	thread;
{
	active_threads[cpu_number()] = thread;
	pmap_activate(vm_map_pmap(thread->task->map), thread, cpu_number());
#if NCPUS > 1
	thread->pcb->pcb_cpu_number = mfpr_whami();
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
	long			mode;
{
	register struct pcb	*pcb;
	vm_offset_t		 sp;

	pcb = thread->pcb;

	pcb->pcb_regs[PCB_PC] = (long) start;

	reset_uu_ar0( thread );
}

/*
 *	thread_setstatus:
 *
 *	Set the status of the given thread.
 */

kern_return_t thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	long			flavor;
	thread_state_t		tstate;
	unsigned long		count;
{
	register struct alpha_thread_state	*state;
	register struct alpha_saved_state	*saved_state;


	switch (flavor) {

	case ALPHA_THREAD_STATE:

		if (count < ALPHA_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		state = (struct alpha_thread_state *) tstate;
		saved_state = USER_REGS(thread);

		saved_state->r0 = state->r0;
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
		if (thread == current_thread()) {
			mtpr_usp(state->r30);
		} else {
			thread->pcb->pcb_usp =  state->r30; 
		}
		saved_state->pc  = state->pc;
		saved_state->ps  = PSL_U;	/* user mode, spl 0 */

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
	long			flavor;
	thread_state_t		tstate;		/* pointer to OUT array */
	unsigned int		*count;		/* IN/OUT */
{
	register struct alpha_thread_state	*state;
	register struct alpha_saved_state	*saved_state;

	switch (flavor) {
	case THREAD_STATE_FLAVOR_LIST:
		if (*count < 2)
			return(KERN_INVALID_ARGUMENT);
		tstate[0] = ALPHA_THREAD_STATE;
		*count = 2;

		break;
	
	case ALPHA_THREAD_STATE:

		if (*count < ALPHA_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		state = (struct alpha_thread_state *) tstate;
		saved_state = USER_REGS(thread);

		state->r0 = saved_state->r0;
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

		if (thread == current_thread()) {
			state->r30=mfpr_usp();
		} else {
			state->r30 = thread->pcb->pcb_usp; 
		}
		state->pc  = saved_state->pc;
		state->ps  = saved_state->ps;

		*count = ALPHA_THREAD_STATE_COUNT;
		break;

	default:
		return(KERN_INVALID_ARGUMENT);
	}
	return(KERN_SUCCESS);
}

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>

/*
 *	thread_dup:
 *
 *	Duplicate the user's state of a thread.  This is only used to perform
 *	the Unix fork operation.
 */
thread_dup(parent, child)
	register thread_t	parent, child;
{
	register struct alpha_saved_state	*parent_state,
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
	child_state->r0 = proc[child->task->proc_index].p_pid;
	child_state->r20 = 1;
	child_state->r19 = 0;
	child->pcb->pcb_usp=mfpr_usp();
	/*
	 * If parent used floating point, copy its state in child.
	 */
	if (parent->pcb->pcb_ownedfp) {
		bcopy(parent->pcb->pcb_fpregs, child->pcb->pcb_fpregs,
							32 * sizeof(long));
	}

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
	thread->u_address.uthread->uu_ar0 = (long *) USER_REGS(thread);
}
