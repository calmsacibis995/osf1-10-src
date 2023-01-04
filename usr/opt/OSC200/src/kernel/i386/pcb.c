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
static char	*sccsid = "@(#)$RCSfile: pcb.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:18 $";
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
 *	File:	i386/pcb.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young, David Golub
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr., Michael Wayne Young,
 *					David Golub
 *
 *	80386 PCB management
 */

#include <i386/pcb.h>
#include <i386/reg.h>
#include <i386/psl.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <kern/thread.h>
#include <mach/thread_status.h>
#include <mach/vm_param.h>

#include <machine/fpreg.h>

#ifndef	NO_U_ADDRESS
struct	u_address	U_ADDRESS;
#endif	NO_U_ADDRESS

void pcb_init(thread, ksp)
	register thread_t	thread;
	register vm_offset_t	ksp;
{
	int				*jbp;
	register struct pcb		*pp = thread->pcb;
	register struct tss386		*pcb = &thread->pcb->pcb_tss;
	register struct i386_saved_state *saved_state;

	int	thread_bootstrap();

	bzero((caddr_t)pcb, sizeof(struct pcb));

	/*
	 *	Set t_cr3 to physical address of the TSS
	 */
	pcb->t_cr3 = kvtophys(thread->task->map->pmap->cr3);
	pcb->t_ldt = LDTSEL;
	pp->pcb_fpvalid = 0;
	ldt_init(thread->task->map->pmap->ldt, (char *)&pp->pcb_fpvalid,
		 sizeof (struct fpstate) + sizeof (int));


	/*
	 *	Set up thread to start at user bootstrap.  The frame
	 *	pointer is setup to point to the frame that corresponds
	 *	to the user's state (thread_bootstrap will perform a
	 *	ret instruction to simulate returning from a trap).
	 *
	 */
	pcb->t_eip = (int) thread_bootstrap;
	ksp += KERNEL_STACK_SIZE;
	/*
	 * leave room on kernel stack for optional 5 words of V86
	 * mode state
	 */
	pcb->t_esp0 = ksp - sizeof(struct i386_v86_info);
/*	pcb->t_esp0 = ksp;*/
	pcb->t_ss0 = KDSSEL;
	pcb->t_ebp = pcb->t_esp0;
	pcb->t_esp = pcb->t_esp0;
	pcb->t_cs = KCSSEL;
	pcb->t_ds = KDSSEL;
	pcb->t_es = KDSSEL;
	pcb->t_ss = KDSSEL;
	pcb->t_bitmapbase = sizeof (struct tss386);
	/*
	 *	need to define a label_t structure for
	 *	setjmp/longjmp
	 */
	jbp = thread->pcb->pcb_context;
	jbp[3] = pcb->t_ebp;
	jbp[4] = ksp - sizeof(struct i386_saved_state) - sizeof(int);
	jbp[5] = pcb->t_eip;
	jbp[6] = 0;			/* initial spl priority */
	/*
	 *	Guarantee that the bootstrapped thread will be in user
	 *	mode (this psl assignment above executes the bootstrap
	 *	code in kernel mode).  Note, this is the only user register
	 *	that we set.  All others are assumed to be random unless
	 *	the user sets them.
	 */
	saved_state = (struct i386_saved_state *)ksp - 1;
	saved_state->efl = EFL_IF;
	saved_state->cs = USER_CS;
	saved_state->ds = USER_DS;
	saved_state->fs = USER_DS;
	saved_state->es = USER_DS;
	saved_state->gs = USER_DS;
	saved_state->v86.oldgs = 0x00;
	saved_state->v86.oldfs = 0x00;
	saved_state->v86.oldds = 0x00;
	saved_state->v86.oldes = 0x00;

}

/* 
 * Initialize a Local Descriptor Table.  "fpstart" and "fpsize" 
 * define an array of bytes.  The comments in seg.h say that this 
 * array is used by the fp emulator.
 */
void ldt_init(ldt, fpstart, fpsize)
	register struct fakedesc *ldt;
	char *fpstart;
	int fpsize;			/* number of bytes */
{
	extern struct fakedesc scall_dscr;
	extern struct fakedesc sigret_dscr;
	register struct fakedesc *tp, *dp;

	ldt[seltoi(USER_SCALL)] = scall_dscr;
	ldt[seltoi(USER_SIGCALL)] = sigret_dscr;
	/* code segment */
	tp = &ldt[seltoi(USER_CS)];
	sdbase(tp, VM_MIN_ADDRESS);
	sdlimit(tp, VM_MAX_ADDRESS - VM_MIN_ADDRESS - 1);
	sdaccess(tp, UTEXT_ACC1, TEXT_ACC2);
	/* data segment */
	dp = &ldt[seltoi(USER_DS)];
	*dp = *tp;
	sdaccess(dp, UDATA_ACC1, DATA_ACC2);
	/* ??? */
	dp = &ldt[seltoi(USER_ZERO)];
	sdbase(dp, VM_MIN_ADDRESS);
	sdlimit(dp, VM_MIN_ADDRESS + sizeof(int));
	sdaccess(dp, UDATA_ACC1, ZERO_ACC2);
	/* segment for use by floating-point emulator (?) */
	dp = &ldt[seltoi(USER_FP)];
	sdbase(dp, fpstart);
	sdlimit(dp, fpsize);
	sdaccess(dp, UDATA_ACC1, DATA_ACC2);
}

/*
 *	Set up the context for the very first thread to execute
 *	(which is passed in as the parameter).
 */
void initial_context(thread)
	thread_t	thread;
{
	extern int system_bootstrap();
	register struct i386_saved_state		*saved_state;
	register struct tss386		*pcb = &thread->pcb->pcb_tss;

	saved_state = (struct i386_saved_state *)
				(thread->kernel_stack + KERNEL_STACK_SIZE -
					sizeof(struct i386_saved_state));
	pcb->t_esp0 = (int) saved_state - sizeof(struct i386_v86_info);
/*	pcb->t_esp0 = (int) saved_state;*/
	pcb->t_ss0 = KDSSEL;
	pcb->t_ebp = pcb->t_esp0;
	pcb->t_esp = pcb->t_esp0;
	pcb->t_cs = KCSSEL;
	pcb->t_eip = (int) system_bootstrap;
	pcb->t_cr3 = _cr3();
	pcb->t_ds = KDSSEL;
	pcb->t_es = KDSSEL;
	pcb->t_ss = KDSSEL;
	pcb->t_bitmapbase = sizeof (struct tss386);	/* debatable */

	active_threads[cpu_number()] = thread;
/*
	bcopy(thread,CURRENT_THREAD,sizeof int);
*/
	pmap_activate(vm_map_pmap(thread->task->map), thread, cpu_number());
	maptss(thread->pcb);
#ifndef	NO_U_ADDRESS
	load_context_data();
#endif	NO_U_ADDRESS
}

/*
 *	Set up the stack to look like a system call, have load_init_program
 *	load in /etc/init, and return to user mode
 */
void init_regs(ar0)
int *ar0;
{

	/*
	 *	thread_start set up the initial stack pointer to have
	 *	room for the trap save registers
	 */
	u.u_ar0 = ar0;
	ar0[EFL] = PS_IE;
	ar0[EIP] = 0;
	ar0[ES] = USER_DS;
	ar0[FS] = 0;
	ar0[GS] = 0;
	ar0[CS] = USER_CS;
	ar0[DS] = USER_DS;
	ar0[SS] = USER_DS;
	load_init_program();
	spl0();
	return;
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
	int			*jbp, sp;

	sp = thread->kernel_stack + KERNEL_STACK_SIZE
			- sizeof(struct i386_saved_state);
	/*
	 *	need to define a label_t structure for
	 *	setjmp/longjmp
	 */
	jbp = thread->pcb->pcb_context;
	jbp[3] = sp;
	jbp[4] = sp - sizeof(int);
	jbp[5] = (int)start;
	jbp[6] = 0;
}

#ifndef	NO_U_ADDRESS
load_context_data()
{
	U_ADDRESS.uthread = active_threads[cpu_number()]->u_address.uthread;
	U_ADDRESS.utask   = active_threads[cpu_number()]->u_address.utask;
}
#endif	NO_U_ADDRESS

set_iopl(flag)
{
	register thread_t thread = current_thread();
	register struct i386_saved_state *saved_state = USER_REGS(thread);

	if (flag)
		saved_state->efl |= EFL_IOPL;
	else
		saved_state->efl &= ~EFL_IOPL;
}


/*
 *	thread_setstatus:
 *
 *	Set the status of the specified thread.
 */

kern_return_t thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		count;
{
	kern_return_t set_thread_state(), set_float_state();

	/* flavor '0' is compatibility code for old interface. */
	switch (flavor) {
	case 0:
	case i386_THREAD_STATE:
		return(set_thread_state(thread, tstate, count));
		break;
	case i386_FLOAT_STATE:
		return(set_float_state(thread, tstate, count));
		break;
	default:
		return(KERN_INVALID_ARGUMENT);
	}
}

/* 
 * 	set_thread_state:
 * 	
 * 	Set the thread state for the specified thread.
 *
 *    For the i386, the user is not allowed to set the segment
 *    registers or the stack pointer used by the kernel.
 *    XXX - we should also think about what we're willing to let the
 *          user do to the flags register.
 */

kern_return_t
set_thread_state(thread, tstate, count)
	thread_t		thread;
	thread_state_t		tstate;
	unsigned int		count;
{
	register struct i386_thread_state	*state;
	register struct i386_saved_state	*saved_state;

	if (count < i386_THREAD_STATE_COUNT)
		return(KERN_INVALID_ARGUMENT);

	state = (struct i386_thread_state *) tstate;
	saved_state = USER_REGS(thread);

	saved_state->eax = state->eax;
	saved_state->ebx = state->ebx;
	saved_state->ecx = state->ecx;
	saved_state->edx = state->edx;
	saved_state->ebp = state->ebp;
	saved_state->edi = state->edi;
	saved_state->esi = state->esi;
/*
 */
	saved_state->uesp = state->uesp;
	saved_state->ds = USER_DS;
	saved_state->es = USER_DS;
	saved_state->fs = USER_DS;
	saved_state->gs = USER_DS;
	saved_state->ss = USER_DS;
	/*
	 * massive grossness -- the first time you go into
	 * v86 mode, the kernel stack pointer is offset by
	 * the extra amount generated by v86 mode interrupts
	 * so that v86 and non v86 threads can share the same
	 * state structure.  adjust the delta when switching
	 * into and out of v86 mode
	 */
	if (!(state->efl & EFL_VM)) {
		saved_state->ss = USER_DS;
		saved_state->cs = USER_CS;
		if (saved_state->efl & EFL_VM) {
			printf("turning off v86 mode\n");
			thread->pcb->pcb_tss.t_esp0 -= sizeof(struct i386_v86_info);
		}
	} else {
		saved_state->v86.oldds = state->ds;
		saved_state->v86.oldes = state->es;
		saved_state->v86.oldfs = state->fs;
		saved_state->v86.oldgs = state->gs;
		saved_state->ss = state->ss & 0xffff;
		saved_state->cs = state->cs & 0xffff;
		if (!(saved_state->efl & EFL_VM)) {
			/* turning on v86 mode */
			printf("turning on v86 mode\n");
			/* eeeeck */
			thread->pcb->pcb_tss.t_esp0 += sizeof(struct i386_v86_info);
			thread->pcb->pcb_tss.t_bitmapbase = sizeof (struct tss386);
		}
	}

/*	saved_state->uesp = state->uesp;*/
	saved_state->efl = state->efl | EFL_IF; /* ensure interrupts on */
	saved_state->eip = state->eip;

	return(KERN_SUCCESS);
}

/* 
 * 	set_float_state:
 * 	
 * 	Set the floating point coprocessor state for the specified 
 * 	thread.
 *
 *    XXX - there should probably be some checks in here, at least 
 *    enough to ensure that the user doesn't crash the system.
 */

kern_return_t
set_float_state(thread, tstate, count)
	thread_t		thread;
	thread_state_t		tstate;
	unsigned int		count;
{
	struct i386_float_state	*state;
	struct i387_state	*pcbstate, *userstate;

	state = (struct i386_float_state *)tstate;
	if (count < i386_FLOAT_STATE_COUNT)
		return(KERN_INVALID_ARGUMENT);
	if (fp_kind == FP_NO)
		return(KERN_FAILURE);

	/* 
	 * If the named thread has ownership of the fp unit, throw out
	 * the old fp state and give up ownership of the fp unit.
	 */
	if (fp_thread == thread)
		fp_unowned();

	pcbstate = (struct i387_state *)thread->pcb->pcb_fps.state;
	userstate = (struct i387_state *)state->hw_state;
	thread->pcb->pcb_fpvalid = state->initialized;

	bcopy(userstate->stack, pcbstate->stack, sizeof(userstate->stack));

	/* Ensure that reserved parts of the environment are 0. */
	bzero((char *)&pcbstate->env, sizeof(struct i387_env));
	pcbstate->env.control = userstate->env.control;
	pcbstate->env.status = userstate->env.status;
	pcbstate->env.tags = userstate->env.tags;
	pcbstate->env.ip = userstate->env.ip;
	pcbstate->env.cs_sel = userstate->env.cs_sel;
	pcbstate->env.opcode = userstate->env.opcode;
	pcbstate->env.data = userstate->env.data;
	pcbstate->env.data_sel = userstate->env.data_sel;

	thread->pcb->pcb_fps.status = 0;

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
	kern_return_t get_thread_state(), get_float_state();

	/* flavor '0' is compatibility code for old interface. */
	switch (flavor) {
	case 0:
	case i386_THREAD_STATE:
		return(get_thread_state(thread, tstate, count));
		break;
	case i386_FLOAT_STATE:
		return(get_float_state(thread, tstate, count));
		break;
	default:
		return(KERN_INVALID_ARGUMENT);
	}
}

/* 
 * 	get_thread_state:
 * 	
 * 	Get the state for the specified thread.
 */

kern_return_t
get_thread_state(thread, tstate, count)
	register thread_t	thread;
	thread_state_t		tstate;		/* pointer to OUT array */
	unsigned int		*count;		/* IN/OUT */
{
	register struct i386_thread_state	*state;
	register struct i386_saved_state	*saved_state;

	if (*count < i386_THREAD_STATE_COUNT) {
		return(KERN_INVALID_ARGUMENT);
	}

	state = (struct i386_thread_state *) tstate;
	saved_state = USER_REGS(thread);

	state->eax = saved_state->eax;   
	state->ebx = saved_state->ebx;
	state->ecx = saved_state->ecx;
	state->edx = saved_state->edx;
	state->esp = saved_state->esp;
	state->ebp = saved_state->ebp;
	state->edi = saved_state->edi;
	state->esi = saved_state->esi;
	if (saved_state->efl & EFL_VM) {
		state->ds = saved_state->v86.oldds;
		state->es = saved_state->v86.oldes;
		state->fs = saved_state->v86.oldfs;
		state->gs = saved_state->v86.oldgs;
	} else {
		state->ds = saved_state->ds;
		state->es = saved_state->es;
		state->fs = saved_state->fs;
		state->gs = saved_state->gs;
	}
	state->ss = saved_state->ss;
	state->uesp = saved_state->uesp;
	state->efl = saved_state->efl;
	state->cs = saved_state->cs;
	state->eip = saved_state->eip;

	*count = i386_THREAD_STATE_COUNT;
	return(KERN_SUCCESS);
}

/* 
 * 	get_float_state:
 * 	
 * 	Get the floating point coprocessor state for the specified 
 * 	thread.
 */

kern_return_t
get_float_state(thread, tstate, count)
	register thread_t	thread;
	thread_state_t		tstate;		/* pointer to OUT array */
	unsigned int		*count;		/* IN/OUT */
{
	register struct i386_float_state	*state;
	struct i387_state	*pcbstate, *userstate;

	state = (struct i386_float_state *)tstate;
	if (*count < i386_FLOAT_STATE_COUNT)
		return(KERN_INVALID_ARGUMENT);

	/* Make sure we've got the latest fp state info */
	pcb_synch(thread);

	state->fpkind = fp_kind;
	state->initialized = thread->pcb->pcb_fpvalid;
	state->exc_status = thread->pcb->pcb_fps.status;
	pcbstate = (struct i387_state *)thread->pcb->pcb_fps.state;
	userstate = (struct i387_state *)state->hw_state;

	bcopy(pcbstate->stack, userstate->stack, sizeof(userstate->stack));

	/* Ensure that reserved parts of the environment are 0. */
	bzero((char *)&userstate->env, sizeof(struct i387_env));
	userstate->env.control = pcbstate->env.control;
	userstate->env.status = pcbstate->env.status;
	userstate->env.tags = pcbstate->env.tags;
	userstate->env.ip = pcbstate->env.ip;
	userstate->env.cs_sel = pcbstate->env.cs_sel;
	userstate->env.opcode = pcbstate->env.opcode;
	userstate->env.data = pcbstate->env.data;
	userstate->env.data_sel = pcbstate->env.data_sel;

	*count = i386_FLOAT_STATE_COUNT;
	return(KERN_SUCCESS);
}

/*
 *	thread_dup:
 *
 *	Duplicate the user's state of a thread.  This is only used to perform
 *	the Unix fork operation.
 */
thread_dup(parent, child)
	register thread_t	parent, child;
{
	register struct i386_saved_state   *parent_state,
					   *child_state;

	parent_state = ((struct i386_saved_state *)
			 (parent->u_address.uthread->uu_ar0));
	child_state = USER_REGS(child);
	*child_state = *parent_state;

	child_state->eax = proc[child->task->proc_index].p_pid;
	child_state->edx = 1;
	child_state->efl &= ~EFL_CF;
}

/*
 *	pcb_terminate
 *
 *	Free up any machine specific pcb resources.
 *	If allocatable ldt's are created for 286 emulation this
 *	would be the place to free them.
 */
pcb_terminate(th)
	thread_t th;
{
	/*
	 * give up the fp unit
	 */
	if (th == fp_thread)
		fp_unowned();
}

/* 
 * 	pcb_synch
 * 	
 * 	Make sure the pcb holds all the info from the hardware.  
 */

void
pcb_synch(th)
	thread_t th;
{
	if (!th)
		panic("null thread in pcb_synch");
	if (fp_thread == th)
		fpsave();
}
