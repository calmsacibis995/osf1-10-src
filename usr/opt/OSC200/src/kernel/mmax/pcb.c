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
static char	*sccsid = "@(#)$RCSfile: pcb.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:19 $";
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
 *	File:	mmax/pcb.c
 *
 *	MMax PCB management
 *
 */

#include <cpus.h>
#include <fast_csw.h>
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

#include <kern/thread.h>
#include <mach/thread_status.h>
#include <mach/kern_return.h>
#include <mach/vm_param.h>
#include <sys/types.h>		/* to get caddr_t */
#include <mmax/cpu.h>
#include <mmax/machparam.h>

#include <mmax/mtpr.h>

#include <mmax/pcb.h>
#include <mmax/psl.h>
#include <mmax/vmparam.h>

extern	int	OSmodpsr;	/* OS mod/psr pair saved in locore.s */

#ifdef	multimax
#if	MMAX_XPC
typedef	struct ns32532_thread_state	ns32xxx_thread_state;
#endif	MMAX_XPC
#if	MMAX_APC || MMAX_DPC
typedef	struct ns32000_thread_state	ns32xxx_thread_state;
#endif	MMAX_APC || MMAX_DPC
#else	multimax
typedef	struct ns32000_thread_state	ns32xxx_thread_state;
#endif	multimax

/*
 *	Set up thread pcb and top of kernel stack for thread start.
 *	All user threads start at thread_begin which trampolines back
 *	to user mode.  Kernel threads are modified by thread_start.
 */
void pcb_init(thread, ksp)
	struct thread 	*thread;
	vm_offset_t	ksp;
{
	struct pcb	*pcb = thread->pcb;
	register struct ns32000_saved_state	*saved_state;
	int	thread_begin();

	bzero((caddr_t)pcb, sizeof(struct pcb));

#if	FAST_CSW
#else	FAST_CSW
	pcb->pcb_pc = (int) thread_begin;
#if	MMAX_XPC || MMAX_APC || MMAX_DPC
	pcb->pcb_modpsr = OSmodpsr; /* set up in locore.s */
#endif	MMAX_XPC || MMAX_APC || MMAX_DPC
#if	!MMAX_DPC
	pcb->pcb_isrv = 0;	    /*  Insure interrupts initially on */
#endif	!MMAX_DPC
#endif	FAST_CSW
	saved_state = (struct ns32000_saved_state *)(ksp + KERNEL_STACK_SIZE
				- sizeof(struct ns32000_saved_state));
#if	FAST_CSW
	pcb->pcb_ssp = ((char *) saved_state) - sizeof(int);
	*((int *) pcb->pcb_ssp) = (int) thread_begin;
#else	FAST_CSW
	pcb->pcb_ssp = (char *) saved_state;
#endif	FAST_CSW
	pcb->pcb_fp = (int) &saved_state->fp;
	/*
	 *	Guarantee that the bootstrapped thread will be in user
	 *	mode (this psl assignment above executes the bootstrap
	 *	code in kernel mode) and set up the default mod register.
	 *	Note, these are the only user registers that we set.
	 *	All others are assumed to be random unless the user sets them.
	 */
	saved_state->psr = PSR_I | PSR_S | PSR_U;
#if	FAST_CSW
	saved_state->sp = USRSTACK;
#else	FAST_CSW
	pcb->pcb_usp = (char *) USRSTACK;  /* needed by init */
#endif	FAST_CSW
	saved_state->mod = 0x20;	/* default value */
}
 
/*
 *	Set up the context for the very first thread to execute
 *	(which is passed in as the parameter).
 */
void initial_context(thread)
	thread_t	thread;
{
	void	initial_uarea_context();

	active_threads[cpu_number()] = thread;
	initial_uarea_context(thread);
	pmap_activate(vm_map_pmap(thread->task->map), thread, cpu_number());
}

/*
 *
 *	thread_start:
 *
 *	Start a kernel thread at specified address.  Thread must be suspended.
 *
#if	FAST_CSW
 *	Kernel thread trampolines off of kernel_thread_begin() to reset
 *	interrupts correctly.  Context routine does a return to
 *	kernel_thread_begin(), which then returns to actual start.
#endif	FAST_CSW
 *
 */

thread_start(thread, start)
	thread_t	thread;
	void		(*start)();
{
	register struct pcb	*pcb;
#if	FAST_CSW
	int	kernel_thread_begin();
#endif	FAST_CSW
	
	pcb = thread->pcb;
#if	FAST_CSW
	pcb->pcb_fp = (thread->kernel_stack + KERNEL_STACK_SIZE);
	pcb->pcb_ssp = ((char *)pcb->pcb_fp) - sizeof(int);
	*((int *) pcb->pcb_ssp) = (int) start;
	pcb->pcb_ssp -= sizeof(int);
	*((int *) pcb->pcb_ssp) = (int) kernel_thread_begin;
#else	FAST_CSW
	pcb->pcb_pc = (int)start;
	pcb->pcb_ssp = (char *)(thread->kernel_stack + KERNEL_STACK_SIZE);
	pcb->pcb_fp = (int) pcb->pcb_ssp; /* for profiling */
#endif	FAST_CSW
}

/*
 *	pcb_synch synchronizes pcb with current thread state.  If thread
 *	is stopped, this is a no-op because save_context() did the required
 *	work.  If the thread is running, it must be on this processor.
 */

pcb_synch(thread)
thread_t	thread;
{
	if (thread != current_thread())
		return;
	else {
		float_sync(thread->pcb);
	}
}

/*
 *	thread_setstatus:
 *
 *	Set the status of the specified thread.
 */

thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		count;
{
	register struct ns32532_thread_state		*state;
	register struct ns32000_thread_state		*ostate;
	register struct ns32000_saved_state		*saved_state;
	register struct pcb	*pcb;

	/* flavor '0' is compatibility code of old interface */

	if (flavor != 0) {
		switch (flavor) {
		    case NS32000_THREAD_STATE:
			if (count < NS32000_THREAD_STATE_COUNT)
				return KERN_INVALID_ARGUMENT;
			break;
		    case NS32532_THREAD_STATE:
			if (count < NS32532_THREAD_STATE_COUNT)
				return KERN_INVALID_ARGUMENT;
			break;
		    default:
			return KERN_INVALID_ARGUMENT;
			break;
		}
	}

	state = (struct ns32532_thread_state *) tstate;
	saved_state = (struct ns32000_saved_state *)
			(thread->kernel_stack + KERNEL_STACK_SIZE -
				sizeof(struct ns32000_saved_state));
	pcb = (struct pcb *) thread->pcb;
	saved_state->sp = state->sp;
	saved_state->pc = state->pc;
	saved_state->psr |= state->psr & 0xff;
	saved_state->mod = state->mod;
	saved_state->r0 = state->r0;
	saved_state->r1 = state->r1;
	saved_state->r2 = state->r2;
	saved_state->r3 = state->r3;
	saved_state->r4 = state->r4;
	saved_state->r5 = state->r5;
	saved_state->r6 = state->r6;
	saved_state->r7 = state->r7;
	saved_state->fp = state->fp;
	/*
	 *	Floating point registers can be found only in pcb.  If
	 *	thread is not suspended this has no effect.
	 */
	pcb->pcb_fsr = state->fsr;
	if (flavor == NS32532_THREAD_STATE) {
		pcb->pcb_f0 = state->f0;
		pcb->pcb_f1 = state->f1;
		pcb->pcb_f2 = state->f2;
		pcb->pcb_f3 = state->f3;
		pcb->pcb_f4 = state->f4;
		pcb->pcb_f5 = state->f5;
		pcb->pcb_f6 = state->f6;
		pcb->pcb_f7 = state->f7;
	}
	else {
		ostate = (struct ns32000_thread_state *) tstate;
		pcb->pcb_f0.val[0] = ostate->f0;
		pcb->pcb_f0.val[1] = ostate->f1;
		pcb->pcb_f2.val[0] = ostate->f2;
		pcb->pcb_f2.val[1] = ostate->f3;
		pcb->pcb_f4.val[0] = ostate->f4;
		pcb->pcb_f4.val[1] = ostate->f5;
		pcb->pcb_f6.val[0] = ostate->f6;
		pcb->pcb_f6.val[1] = ostate->f7;
	}
	return(KERN_SUCCESS);
}

/*
 *	thread_getstatus:
 *
 *	Get the status of the specified thread.
 */

thread_getstatus(thread, flavor, tstate, count)
	register thread_t	thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		*count;
{
	register struct ns32532_thread_state		*state;
	register struct ns32000_thread_state		*ostate;
	register struct ns32000_saved_state		*saved_state;
	register struct pcb	*pcb;

	/* flavor '0' is compatiblity code for old interface. */
	if (flavor != 0) {
		switch (flavor) {
		    case NS32000_THREAD_STATE:
			if (*count < NS32000_THREAD_STATE_COUNT)
				return KERN_INVALID_ARGUMENT;
			break;
		    case NS32532_THREAD_STATE:
			if (*count < NS32532_THREAD_STATE_COUNT)
				return KERN_INVALID_ARGUMENT;
			break;
		    default:
			return KERN_INVALID_ARGUMENT;
			break;
		}
	}

	state = (struct ns32532_thread_state *) tstate;
	saved_state = (struct ns32000_saved_state *)
			(thread->kernel_stack + KERNEL_STACK_SIZE -
				sizeof(struct ns32000_saved_state));
	pcb = (struct pcb *) thread->pcb;

	state->sp = saved_state->sp;
	state->pc = saved_state->pc;
	state->psr = saved_state->psr;
	state->mod = saved_state->mod;
	state->r0 = saved_state->r0;
	state->r1 = saved_state->r1;
	state->r2 = saved_state->r2;
	state->r3 = saved_state->r3;
	state->r4 = saved_state->r4;
	state->r5 = saved_state->r5;
	state->r6 = saved_state->r6;
	state->r7 = saved_state->r7;
	state->fp = saved_state->fp;
	/*
	 *	Floating point registers can be found only in pcb.  If
	 *	thread is not suspended these values are bogus.
	 */
	state->fsr = pcb->pcb_fsr;
	if (flavor == NS32532_THREAD_STATE) {
		state->f0 = pcb->pcb_f0;
		state->f1 = pcb->pcb_f1;
		state->f2 = pcb->pcb_f2;
		state->f3 = pcb->pcb_f3;
		state->f4 = pcb->pcb_f4;
		state->f5 = pcb->pcb_f5;
		state->f6 = pcb->pcb_f6;
		state->f7 = pcb->pcb_f7;
	} else {
		ostate = (struct ns32000_thread_state *) tstate;
		ostate->f0 = pcb->pcb_f0.val[0];
		ostate->f1 = pcb->pcb_f0.val[1];
		ostate->f2 = pcb->pcb_f2.val[0];
		ostate->f3 = pcb->pcb_f2.val[1];
		ostate->f4 = pcb->pcb_f4.val[0];
		ostate->f5 = pcb->pcb_f4.val[1];
		ostate->f6 = pcb->pcb_f6.val[0];
		ostate->f7 = pcb->pcb_f6.val[1];
	}
	return(KERN_SUCCESS);
}

/*
 *	thread_dup:
 *
 *	Duplicate the user's state of a thread.  This is only used to perform
 *	the Unix fork operation.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <kern/task.h>

struct u_address active_uareas[NCPUS];

thread_dup(parent, child)
	register thread_t	parent, child;
{
	int		offset;
	struct ns32000_saved_state	*state;

	/*
	 *	Copy all registers to the new stack.
	 */
	offset = KERNEL_STACK_SIZE - sizeof(struct ns32000_saved_state);
	state = (struct ns32000_saved_state *)(child->kernel_stack + offset);
	bcopy((caddr_t)(parent->kernel_stack + offset), (caddr_t) state,
		sizeof(struct ns32000_saved_state));
	state->r0 = proc[child->task->proc_index].p_pid;
	state->r1 = 1;
	state->psr &= ~PSR_C;
	float_sync(child->pcb);
}

void initial_uarea_context(thread)
thread_t	thread;
{
	active_uareas[cpu_number()] = thread->u_address;
}
