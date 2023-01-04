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
static char	*rcsid = "@(#)$RCSfile: mach_process.c,v $ $Revision: 4.3.10.7 $ (DEC) $Date: 1993/12/17 01:14:48 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 198,7,8,9 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0.1
 */
/*
 * Modification History: mach_process.c
 *
 * 4-Dec-91  Brian Stevens
 *	Modified the ptrace request PT_CONTINUE to return the passed in data
 *	parameter upon success.  Modified the ptrace request PT_WRITE_I and
 *	PT_WRITE_D to return the passed in data parameter upon success.
 *	Both of these changes are neccessary to pass SVVS.
 *   
 *
 * 4-Nov-91  Paula Long
 *   Merged OSF V1.0.1 fix (bug number 1897). 
 *    The Mach version of ptrace omitted the resetting of the SWTED flag; it
 *    just needs to be restored.      
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
 *	@(#)sys_process.c	7.1 (Berkeley) 6/5/86
 */

#include <machine/reg.h>
#ifdef	ibmrt
#include <ca/scr.h>
#include <mach/ca/vm_param.h>		/* for KERNEL_STACK_SIZE */
#endif	ibmrt
#if	!defined(ibmrt) && !defined(mips)
#include <machine/psl.h>
#endif	!defined(ibmrt) && !defined(mips)

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/siginfo.h>

#include <kern/sched_prim.h>
#include <kern/task.h>
#include <kern/thread.h>

#include <vm/vm_map.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_kern.h>

#ifdef	sun
/* Use Sun version of ptrace */
#else	sun

#ifdef	vax
#define NIPCREG 16
int ipcreg[NIPCREG] =
	{R0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,AP,FP,SP,PC};
#define	PROGRAM_COUNTER		PC
#define	PROCESSOR_STATUS_WORD	PS
#endif	vax

#ifdef	i386
#define NIPCREG 16
int ipcreg[NIPCREG] =
	{EAX,EBX,ECX,EDX,ESI,EDI,EBP,EIP,ESP};
#define	PROGRAM_COUNTER		EIP
#define	PROCESSOR_STATUS_WORD	EFL
#define	PSL_USERSET		EFL_USERSET
#define	PSL_USERCLR		EFL_USERCLR
#define	PSL_T			EFL_TF
#endif	i386

#if	defined(mc68000) || defined(__mc68000)
#define NIPCREG 17
int ipcreg[NIPCREG] =
        {R0,R1,R2,R3,R4,R5,R6,R7,AR0,AR1,AR2,AR3,AR4,AR5,AR6,AR7,PC};
#define	PROGRAM_COUNTER		PC
#define	PROCESSOR_STATUS_WORD	PS
#endif	mc68000

#ifdef	ibmrt
#define NIPCREG 18	 /* allow modification of only these u area variables */
int ipcreg[NIPCREG] =
	{R0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15,IAR,MQ};
#define	PROGRAM_COUNTER		IAR
#define	PROCESSOR_STATUS_WORD	ICSCS
#define	PSL_USERSET		ICSCS_USERSET
#define	PSL_USERCLR		ICSCS_USERCLR
#define	PSL_T			ICSCS_INSTSTEP
#endif	ibmrt

#ifdef	balance
int	ipcreg[] = {R0,R1,R2,R3,R4,R5,R6,R7,FP,SP,PC};
#define NIPCREG	(sizeof(ipcreg)/sizeof(ipcreg[0]))
#define	PROGRAM_COUNTER		PC
#define	PROCESSOR_STATUS_WORD	PS
#endif	balance

#ifdef	multimax
/* 
 *	Multimax does this differently; see mmax/mmax_ptrace.c
 *	and mmax/mmax_ptrace.h for details.
 */
#define	get_ptrace_u mmax_get_ptrace_u
#define set_ptrace_u mmax_set_ptrace_u
#define	PROGRAM_COUNTER		PC
#define	PROCESSOR_STATUS_WORD	PS
#endif	multimax

#ifdef	mips
#define	PROGRAM_COUNTER		EF_EPC
#endif	mips

#ifdef	__alpha
#define	PROGRAM_COUNTER		EF_PC
#endif	__alpha

#define ALIGNED(addr,size)	(((unsigned)(addr)&((size)-1))==0)

/*
 * sys-trace system call.
 */
ptrace(curp, args, retval)
	struct proc *curp;
	void *args;
	ulong_t *retval;
{
	register struct args {
		long	req;
		long	pid;
		ulong_t	*addr;
		ulong_t	data;
	} *uap = (struct args *) args;

	register struct proc *p;
	ulong_t		*locr0;
	int		error = 0;
	int s;

	/*
	 *	Intercept and deal with "please trace me" request.
	 */
	if (uap->req <= 0) {
		curp->p_flag |= STRC;
		return (0);
	}

	/*
	 *	Locate victim, and make sure it is traceable.
	 */
	p = pfind(uap->pid);
	if (p == 0 || p->task->user_stop_count == 0 ||
	    p->p_stat != SSTOP || p->p_ppid != curp->p_pid ||
	    !(p->p_flag & STRC))
		return (ESRCH);

	/*
	 * Set p_wcode to CLD_TRAPPED to show that the process has be
	 * traced on.
	 */
	p->p_wcode = CLD_TRAPPED;

	/*
	 *	Mach version of ptrace executes request directly here,
	 *	thus simplifying the interaction of ptrace and signals.
	 */

	switch (uap->req) {

	case PT_READ_I:
	case PT_READ_D:
		error = ptrace_read_data(p,
				(vm_offset_t)uap->addr,
				sizeof(long),
				(caddr_t)retval);
		break;



	case PT_WRITE_I:
	case PT_WRITE_D:
		error = ptrace_write_data(p,
				(vm_offset_t)uap->addr,
				sizeof(long),
				(caddr_t)&uap->data,
				(caddr_t)retval,
				(uap->req == PT_WRITE_I));
		*retval = uap->data;
		break;

	case PT_READ_U:
		/*
		 *	Read victim's U-area or registers.
		 *	Offsets are into BSD kernel stack, and must
		 *	be faked to match MACH.
		 */
#if	defined(multimax) || defined(mips) || defined(__alpha)
		if (get_ptrace_u(uap->addr, retval, p->thread) != 0)
			goto errout;
		break;
#else	defined(multimax) || defined(mips) || defined(__alpha)
	    {
		register int	i, off;
		struct user	fake_uarea;

		i = (int)uap->addr;

		if (i < 0 || i >= ctob(UPAGES))
			goto errout;

#ifdef	ibmrt
		if (!ALIGNED(i, sizeof(int)))
			goto errout;
		/*
		 *	U-area and kernel stack are swapped
		 */
		i -= (ctob(UPAGES) - sizeof(struct user));
		off = i + ctob(UPAGES);
		if (i >= 0) {
#else	ibmrt
		off = i;
		if (i < sizeof(struct user)) {
#endif	ibmrt
		    /*
		     *	We want data from the U area.  Fake it up,
		     *	then pull out the desired int.
		     */
		    bzero((caddr_t)&fake_uarea, sizeof(struct user));
		    fake_u(&fake_uarea, p->thread);
		    *retval = *(long *)(((caddr_t)&fake_uarea) + i);
		}
		else {
		    /*
		     *	Assume we want data from the kernel stack, most
		     *	likely the user's registers.
		     *
		     */
		    *retval = *(long *)(
			((caddr_t)p->thread->kernel_stack)
			+ (KERNEL_STACK_SIZE - ctob(UPAGES))
			+ off);
		}
		break;
	    }
#endif	defined(multimax) || defined(mips) || defined(__alpha)

	case PT_WRITE_U:
		/*
		 *	Write victim's registers.
		 *	Offsets are into BSD kernel stack, and must
		 *	be faked to match MACH.
		 */
#if	defined(multimax) || defined(mips) || defined(__alpha)
		if (set_ptrace_u(uap->addr, uap->data, p->thread, retval) != 0)
			goto errout;
		break;
#else	defined(multimax) || defined(mips) || defined(__alpha)
	    {
		register int	i, off;
		register int	*reg_addr;

		i = (int)uap->addr;
#ifdef	ibmrt
		i -= (ctob(UPAGES) - sizeof(struct user));
		off = i - ctob(UPAGES);
#else	ibmrt
		off = i;
#endif	ibmrt
		/*
		 *	Write one of the user's registers.
		 *	Convert the offset (in old-style Uarea/kernel stack)
		 *	into the corresponding offset into the saved
		 *	register set.
		 */
		reg_addr = (int *)(((caddr_t)p->thread->kernel_stack)
				+ (KERNEL_STACK_SIZE - ctob(UPAGES))
				+ off);

		locr0 = p->thread->u_address.uthread->uu_ar0;

		for (i = 0; i < NIPCREG; i++)
			if (reg_addr == &locr0[ipcreg[i]])
				goto ok;

		if (reg_addr == &locr0[PROCESSOR_STATUS_WORD]) {
			uap->data = (uap->data | PSL_USERSET) & ~PSL_USERCLR;
#ifdef	vax
			/* special problems with compatibility mode */
			if (uap->data & PSL_CM)
			    uap->data &= ~(PSL_FPD|PSL_DV|PSL_FU|PSL_IV);
#endif	vax
		} else {
		    goto errout;
		}

	ok:
		*reg_addr = uap->data;
		break;
	    }
#endif	defined(multimax) || defined(mips)

	case PT_KILL:
		/*
		 *	Tell child process to kill itself after it
		 *	is resumed by adding NSIG to p_cursig. [see issig]
		 */
		s = splhigh();
		p->p_cursig += NSIG;
		splx(s);
		goto resume;

	case PT_STEP:			/* single step the child */
	case PT_CONTINUE:		/* continue the child */
		locr0 = (ulong_t *)p->thread->u_address.uthread->uu_ar0;
		if ((int)uap->addr != 1) {
#ifdef	mips
			if (!ALIGNED(uap->addr, sizeof(int)))
				goto errout;
#endif	mips
			locr0[PROGRAM_COUNTER] = (long)uap->addr;
		}

		if ((unsigned)uap->data > NSIG)
			goto errout;
		*retval = uap->data;

		if (sigmask(p->p_cursig) & threadmask)
			p->thread->u_address.uthread->uu_cursig = 0;
		p->p_cursig = uap->data;	/* see issig */

		if (p->thread->u_address.uthread->uu_curinfo &&
		    (p->p_cursig !=
		     p->thread->u_address.uthread->uu_curinfo->
		        siginfo.si_signo))
			/*
			 * This sigqueue structure is for a 
			 * different signal than what is now in
			 * p_cursig, so get rid of it.
			 */
			SIGQ_FREE(p->thread->u_address.uthread->uu_curinfo);
		if (sigmask(uap->data) & threadmask)
			p->thread->u_address.uthread->uu_cursig = uap->data;

#if defined(mips) || defined(__alpha)
		p->thread->pcb->pcb_sstep = uap->req;
#else
		if (uap->req == PT_STEP) 
			locr0[PROCESSOR_STATUS_WORD] |= PSL_T;
#endif	/* mips || __alpha */

	resume:
		p->p_stat = SRUN;
		if (p->thread && p->p_cursig)
			clear_wait(p->thread, THREAD_INTERRUPTED, TRUE);
		thread_release(p->thread);
		break;
		
	default:
	errout:
		error = EIO;
	}
	return (error);
}
#endif	sun

/*
 *	Convenient routines to read/write process address spaces
 *
 *	XXX If anyone feels like doing some public service,
 *	XXX these routines should be reimplemented using only
 *	XXX the exported Mach interface calls.
 */

int ptrace_read_data(p, address, amount, resulting_data)
	struct proc	*p;
	vm_offset_t	address;
	vm_size_t	amount;
	caddr_t		resulting_data;
{
	vm_map_t	victim_map;
	vm_offset_t	start_addr, end_addr,
			kern_addr, offset;
	vm_size_t	size;
	vm_map_copy_t	copy_result;
	int		result = 0;

	/*
	 *	Read victim's memory
	 */

#ifdef	mips
	if (!ALIGNED((int *) address, amount))
		return(EIO);
#endif	mips

	victim_map = p->task->map;
	vm_map_reference(victim_map);
	if (vm_map_copyin(
		victim_map,
		address,
		amount,
		FALSE,
		&copy_result)
	    != KERN_SUCCESS) {
	    	vm_map_deallocate(victim_map);
		return(EIO);
	}
	vm_map_deallocate(victim_map);
	if (vm_map_copyout(kernel_copy_map, &kern_addr, copy_result)
	    != KERN_SUCCESS) {
		vm_map_copy_discard(copy_result);
		result = EIO;
	}
	else {
	    /*
	     *	Read the data from the copy in the kernel map.
	     *	Use eblkcpy to avoid alignment restrictions and
	     *  handle any errors in vm_fault().
	     */
            if (eblkcpy((caddr_t) kern_addr, resulting_data, amount)) 
                result = EIO;
	}

	/*
	 *	Discard the kernel's copy.
	 */
	(void) vm_deallocate(kernel_copy_map, kern_addr, amount);
	return(result);
}


int ptrace_write_data(p, address, amount, new_data, old_data, flush)
	struct proc	*p;
	vm_offset_t	address;
	vm_size_t	amount;
	caddr_t		new_data;
	caddr_t		old_data;
	boolean_t	flush;
{
	vm_map_t	victim_map;
	vm_offset_t	start_addr, end_addr,
			kern_addr, offset;
	vm_size_t	size;
	vm_map_copy_t	copy_result;
	boolean_t	change_protection;
	int		save_protection;
	int		result = 0;

#if	!defined(mips) && defined(lint)
	if (flush) old_data++;
#endif	!defined(mips) && defined(lint)

	/*
	 *	Write victim's memory
	 */
#ifdef	mips
	if (!ALIGNED(address, amount))
		return(EIO);
#endif	mips

	start_addr = trunc_page(address);
	end_addr = round_page(address + amount);
	size = end_addr - start_addr;

	victim_map = p->task->map;

	/*
	 *	Allocate some pageable memory in the kernel map,
	 *	and copy the victim's memory to it.
	 */
	vm_map_reference(victim_map);

	/*
	 *	Obtain write access to the page.
	 */
	change_protection = FALSE;
	if (!vm_map_check_protection(victim_map, start_addr, end_addr,
	    						VM_PROT_WRITE)) {
		change_protection = TRUE;
		save_protection = 0;
		if (vm_map_check_protection(victim_map, start_addr, end_addr,
	    						VM_PROT_READ)) 
			save_protection |= VM_PROT_READ;
		if (vm_map_check_protection(victim_map, start_addr, end_addr,
	    						VM_PROT_EXECUTE))
			save_protection |= VM_PROT_EXECUTE;

		if (vm_map_protect(victim_map, start_addr, end_addr,
					VM_PROT_ALL, FALSE) != KERN_SUCCESS) {
	    		vm_map_deallocate(victim_map);
			return(EIO);
		}
	}

	if (vm_map_copyin(victim_map, start_addr, size, FALSE, &copy_result)
	    != KERN_SUCCESS) {
        	if (change_protection) {
            		(void) vm_map_protect(victim_map, start_addr, end_addr,
                                                	save_protection, FALSE);
        	}
	    	vm_map_deallocate(victim_map);
		return(EIO);
	}

	if (vm_map_copyout(kernel_copy_map, &kern_addr, copy_result)
	    != KERN_SUCCESS) {
        	if (change_protection) {
            		(void) vm_map_protect(victim_map, start_addr, end_addr,
                                                	save_protection, FALSE);
        	}
	    	vm_map_deallocate(victim_map);
		vm_map_copy_discard(copy_result);
		return(EIO);
	}
        /*
         *      Change the data in the copy in the kernel map.
         *      Use eblkcpy to avoid alignment restrictions and handle
         *      any errors in vm_fault().
         */

        offset = (vm_offset_t)address - start_addr;
#ifdef  mips
        if (eblkcpy((caddr_t) (kern_addr + offset), old_data, amount)) {
                vm_map_deallocate(victim_map);
                (void) vm_deallocate(kernel_pageable_map, kern_addr, amount);
                return(EIO);
        }
#endif  /*mips*/
        if (eblkcpy(new_data, (caddr_t) (kern_addr + offset), amount)) {
                vm_map_deallocate(victim_map);
                (void) vm_deallocate(kernel_pageable_map, kern_addr, amount);
                return(EIO);
        }

	/*
	 *	Copy it back to the victim.
	 */

	if (vm_map_copyin(kernel_copy_map, kern_addr, size, TRUE, &copy_result) != KERN_SUCCESS) {
        	if (change_protection) {
            		(void) vm_map_protect(victim_map, start_addr, end_addr,
                                                	save_protection, FALSE);
        	}
	    	vm_map_deallocate(victim_map);
		vm_deallocate(kernel_copy_map, kern_addr, amount);
		return(EIO);
	}

	if (vm_map_copy_overwrite(
		victim_map,
		start_addr,
		copy_result,
		FALSE /* XXX interruptible */)
	    != KERN_SUCCESS)
		result = EIO;

	vm_map_copy_discard(copy_result);

	/*
	 *	Re-protect the victim's memory.
	 */
	if (change_protection) {
	    (void) vm_map_protect(victim_map, start_addr, end_addr,
						save_protection, FALSE);
	}
	/*
	 * Flush Icache, as we might have changed the victim's code.
	 * We always flush because the new dbx uses data write to
	 * modify text (why?).
	 */
#ifdef notdef
	if (flush)
#endif /* notdef */
#ifdef mips
		ptrace_user_flush(victim_map, start_addr, amount);
#else
		imb();
#endif

    	vm_map_deallocate(victim_map);

	/*
	 *	Discard the kernel's copy.
	 */
	(void) vm_deallocate(kernel_copy_map, kern_addr, size);

	return(result);

}
