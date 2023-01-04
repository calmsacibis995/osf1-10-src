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
static char	*sccsid = "@(#)$RCSfile: vm_unix.c,v $ $Revision: 4.4.15.5 $ (DEC) $Date: 1993/09/29 12:56:06 $";
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
 * derived from vm_unix.c	2.1	(ULTRIX/OSF)	12/3/90";
 *
 *
 *	Revision History:
 *
 * 24-Jun-91	Peter H. Smith
 *	Merge in OSF/1.0.1 changes:
 *	  - Clear (child) thread->u_address.utask->uu_flags in procdup().
 *
 * 04-Jun-91	Diane Lebel
 *	Added support for > 64 per-process file descriptors.
 *
 * 02-May-91	Peter H. Smith
 *	Modify procdup() to cause inheritance of policy and quantum (stored
 *	in sched_data accross fork().
 */

#include <cputypes.h>
#include <mach_debug.h>
#include <mach_emulation.h>
#include <rt_sched.h>

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <kern/task.h>
#include <kern/thread.h>
#include <mach/time_value.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_umap.h>
#include <kern/parallel.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/vm.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/trace.h>
#include <sys/map.h>
#include <sys/kernel.h>
#include <sys/mman.h>
#include <sys/sched_mon.h>

#include <vm/vm_kern.h>
#include <mach/port.h>
#include <kern/kern_obj.h>
#include <kern/ipc_pobj.h>
#include <kern/ipc_copyout.h>
#include <kern/kern_port.h>
#include <mach/memory_object.h>


/*
 * XXX	NOTE: The following three routines are still here for historical
 *		reasons.  All uses of them should be replaced as follows:
 *
 *		useracc + vslock -> vm_map_pageable.
 *		useracc (by itself) - Should be replaced because its
 *			results are unreliable (memory could vanish
 *			immediately after call).  Use copyin/copyout to
 *			access user space.
 *		vslock -> vm_map_pageable.
 *		vsunlock -> vm_map_pageable.
 *		
 *	vm_map_pageable incorporates protection check logic which ensures that
 *	the specified accesses to the memory will not cause page faults
 *	if vm_map_pageable returns KERN_SUCCESS.
 */

useracc(addr, len, prot)
	caddr_t	addr;
	u_int	len;
	int	prot;
{
	return (vm_map_check_protection(
			current_task()->map,
			trunc_page(addr), round_page(addr+len),
			prot == B_READ ? VM_PROT_READ : VM_PROT_WRITE));
}


int
vslock( addr, len )
    caddr_t	addr;
    int		len;
{
    kern_return_t status;

    status = vm_map_pageable( current_task()->map, trunc_page(addr),
			round_page(addr+len), VM_PROT_READ|VM_PROT_WRITE);

    return (kern_return_xlate(status));
}

int
vsunlock( addr, len, dirtied )
    caddr_t	addr;
    int		len;
    int		dirtied;
{
    kern_return_t status;

#ifdef  lint
	dirtied++;
#endif

    status = vm_map_pageable( current_task()->map, trunc_page(addr),
			round_page(addr+len), VM_PROT_NONE );

    return (kern_return_xlate(status));
}

int
kern_return_xlate( status )
    int		status;
{
    int stat;

    switch (status) {

	case KERN_SUCCESS:		stat = 0; break;
	case KERN_INVALID_ADDRESS:	stat = EFAULT; break;
	case KERN_PROTECTION_FAILURE:	stat = EACCES; break;
	case KERN_NO_SPACE:		stat = ENOMEM; break;
	case KERN_INVALID_ARGUMENT:	stat = EINVAL; break;
	case KERN_NO_ACCESS:		stat = EACCES; break;
	case KERN_MEMORY_FAILURE:	stat = ENOMEM; break;
	case KERN_MEMORY_ERROR:		stat = ENOMEM; break;
	default:			stat = EIO; break;
    }

    return (stat);
}


#if	defined(sun3) || defined(sun4) || balance || defined(i386) || defined(mips) || defined(__alpha)
/* These architectures have faster assembly-language versions */
#else	/* defined(sun3,sun4,balance,i386,mips,__alpha) */
subyte(addr, byte)
	caddr_t addr;
	char byte;
{
	return (copyout((caddr_t) &byte, addr, sizeof(char)) == 0 ? 0 : -1);
}

suibyte(addr, byte)
	caddr_t addr;
	char byte;
{
	return (copyout((caddr_t) &byte, addr, sizeof(char)) == 0 ? 0 : -1);
}

int fubyte(addr)
	caddr_t addr;
{
	char byte;

	if (copyin(addr, (caddr_t) &byte, sizeof(char)))
		return(-1);
	return((unsigned) byte);
}

int fuibyte(addr)
	caddr_t addr;
{
	char byte;

	if (copyin(addr, (caddr_t) &byte, sizeof(char)))
		return(-1);
	return((unsigned) byte);
}

suword(addr, word)
	caddr_t addr;
	int word;
{
	return (copyout((caddr_t) &word, addr, sizeof(int)) == 0 ? 0 : -1);
}

int fuword(addr)
	caddr_t addr;
{
	int word;

	if (copyin(addr, (caddr_t) &word, sizeof(int)))
		return(-1);
	return(word);
}

#ifndef	vax
/* suiword and fuiword are the same as suword and fuword, respectively */

suiword(addr, word)
	caddr_t addr;
	int word;
{
	return (copyout((caddr_t) &word, addr, sizeof(int)) == 0 ? 0 : -1);
}

int fuiword(addr)
	caddr_t addr;
{
	int word;

	if (copyin(addr, (caddr_t) &word, sizeof(int)))
		return(-1);
	return(word);
}
#endif	/* vax */
#endif	/* defined(sun3,sun4,balance,i386,mips,__alpha) */

thread_t procdup(child, parent)
	struct proc *child, *parent;
{
	thread_t	thread;
	task_t		task;
 	kern_return_t	result;
	struct ufile_state *parent_ufp, *child_ufp;
#if RT_SCHED
	register thread_t parent_thread;
	register int	s;
#endif /* RT_SCHED */

	result = task_create(parent->task, TRUE, &task);
	if(result != KERN_SUCCESS) {
	    printf("fork/procdup: task_create failed. Code: 0x%x\n", result);
	    return(THREAD_NULL);
	}
	child->task = task;

	/* XXX Cheat to get proc pointer into task structure */
	task->proc_index = child - proc;

	/*
	 * Copy parent's u area into new task
	 */
	bcopy((caddr_t) parent->task->u_address,
	      (caddr_t) task->u_address,
	      (unsigned) sizeof(struct utask));

	task->u_address->uu_procp = child;

	/*
	 * If parent u_area has an overflow buffer for open file
	 * descriptors, make one for the child too.
	 */
	if (parent->task->u_address->uu_file_state.uf_of_count) {
        	parent_ufp = &parent->task->u_address->uu_file_state;
		child_ufp = &task->u_address->uu_file_state;

		child_ufp->uf_ofile_of = (struct file **)
                        kalloc(parent_ufp->uf_of_count * sizeof(struct file *));
		if ((caddr_t)child_ufp->uf_ofile_of == NULL) {
			printf("procdup could not alloc child's ofile_of\n");
                        (void) task_terminate (task);
                        return(THREAD_NULL);
		}
		child_ufp->uf_pofile_of = (char *) 
			kalloc(parent_ufp->uf_of_count);
		if ((caddr_t)child_ufp->uf_pofile_of == NULL) {
                        printf("procdup could not alloc child's pofile_of\n");
                        kfree(child_ufp->uf_ofile_of,
			      parent_ufp->uf_of_count * sizeof(struct file *));
			(void) task_terminate (task);
			return(THREAD_NULL);
		}

		bcopy((caddr_t)parent_ufp->uf_ofile_of,
                      (caddr_t)child_ufp->uf_ofile_of,
                      parent_ufp->uf_of_count * sizeof(struct file *));
		bcopy((caddr_t)parent_ufp->uf_pofile_of,
                      (caddr_t)child_ufp->uf_pofile_of,
                      parent_ufp->uf_of_count);
	}

	result = thread_create(task, &thread);
	if(result != KERN_SUCCESS) {
	    printf("fork/procdup: thread_create failed. Code: 0x%x\n", result);
	    if (task->u_address->uu_file_state.uf_of_count) {
		   kfree(child_ufp->uf_ofile_of,
			 child_ufp->uf_of_count * sizeof(struct file *));
		   kfree(child_ufp->uf_pofile_of, child_ufp->uf_of_count);
	    }
	    (void) task_terminate (task);
	    return(THREAD_NULL);
	}
	child->thread = thread;
#if RT_SCHED
	/*
	 * Copy base priority, policy, and policy data to child.  The
	 * other scheduling fields are initialized by thread_create.
	 */
	parent_thread = current_thread();
	thread->priority = parent_thread->priority;
	thread->policy = parent_thread->policy;
	thread->sched_data = parent_thread->sched_data;
	RT_SCHED_HIST_SPL(RTS_fork, parent_thread, 
			  (parent_thread->policy|thread->policy),
			  thread, thread->policy, thread->priority);
#else /* RT_SCHED */
	thread->priority = current_thread()->priority;
#endif /* RT_SCHED */
	/*
	 *	Don't need to lock thread here because it can't
	 *	possibly execute and no one else knows about it.
	 */
	compute_priority(thread, TRUE);
	/*
	 * thread_create called uarea_init, which initialized some per-thread
	 * fields, including credentials.  We're about to undo that work,
	 * but we need to do a little cleanup first.  Then we can 
	 * call uarea_init to put back what we took away.
	 * It isn't pretty, but...
	 */
	ASSERT(thread->u_address.uthread->uu_nd.ni_cred != NOCRED);
	crfree(thread->u_address.uthread->uu_nd.ni_cred);

	bcopy((caddr_t) parent->thread->u_address.uthread,
	      (caddr_t) thread->u_address.uthread,
	      (unsigned) sizeof(struct uthread));
	bzero((caddr_t) &thread->u_address.utask->uu_ru,
			sizeof(struct rusage));
	bzero((caddr_t) &thread->u_address.utask->uu_cru,
			sizeof(struct rusage));
	thread->u_address.utask->uu_outime = 0;
	thread->u_address.utask->uu_semundo = NULL;
 	thread->u_address.utask->uu_lflags = 0;

#if	MACH_EMULATION	
	if (task->eml_dispatch = parent->task->eml_dispatch) {
		task->eml_dispatch->disp_count =
			parent->task->eml_dispatch->disp_count;
		task->eml_dispatch->eml_ref++;
	    }
#endif
	uarea_init(thread);
	return(thread);
}

#ifndef	vax
chgprot(_addr, prot)
	caddr_t		_addr;
	vm_prot_t	prot;
{
	vm_offset_t	addr = (vm_offset_t) _addr;

	return(vm_map_protect(current_task()->map,
				trunc_page(addr),
				round_page(addr + 1),
				prot, FALSE) == KERN_SUCCESS);
}
#endif

kern_return_t	unix_pid(t, x)
	task_t	t;
	int	*x;
{
	if (t == TASK_NULL) {
		*x = -1;
		return(KERN_FAILURE);
	} else {
		*x =  proc[t->proc_index].p_pid;
		return(KERN_SUCCESS);
	}
}

/*
 *	Routine:	task_by_unix_pid
 *	Purpose:
 *		Get the task port for another "process", named by its
 *		process ID on the same host as "target_task".
 *
 *		Only permitted to privileged processes, or processes
 *		with the same user ID.
 */
kern_return_t	task_by_unix_pid(target_task, pid, t)
	task_t		target_task;
	int		pid;
	task_t		*t;
{
	uid_t		uid, uid1, ruid, svuid;
	struct proc	*p, *p1;
	kern_return_t kr = KERN_SUCCESS;

	if ((target_task == TASK_NULL) ||
	    (target_task->proc_index == 0))
		return KERN_INVALID_ARGUMENT;

	/*
	 * Until proc manipulations are parallelized.  XXX
	 */
	unix_master();

	cr_threadinit(current_thread());

	if (pid == -1) {
#if	SEC_BASE
		if (privileged(SEC_DEBUG, EPERM)
			&& (target_task == current_task()))
#else 
		if (!(suser(u.u_cred, &u.u_acflag))
			&& (target_task == current_task()))
#endif
			*t = kernel_task;
		else
			kr = KERN_PROTECTION_FAILURE;
	} else if (pid == 0) {
#if	SEC_BASE
                if (privileged(SEC_DEBUG, EPERM)
			&& (target_task == current_task()))
#else
		if (!(suser(u.u_cred, &u.u_acflag))
			&& (target_task == current_task()))
#endif
			*t = proc[0].task;
		else
			kr = KERN_PROTECTION_FAILURE;
	} else if (((p = pfind(pid)) != (struct proc *) 0) &&
		   (p->p_stat != SZOMB) && (p->p_stat != SIDL)) {
		BM(PROC_LOCK(p));
		uid = p->p_rcred->cr_uid;
                ruid = p->p_ruid;
                svuid = p->p_svuid;
		BM(PROC_UNLOCK(p));
		p1 = proc + target_task->proc_index;
		PROC_LOCK(p1);
		uid1 = p1->p_rcred->cr_uid;
		PROC_UNLOCK(p1);
#if	SEC_BASE
                if (((uid == uid1) && (uid == ruid) && (uid == svuid))
			|| privileged(SEC_DEBUG, EPERM))
#else
		if (((uid == uid1) && (uid == ruid) && (uid == svuid))
		    	|| (!(suser(u.u_cred, &u.u_acflag))))
#endif
			*t = p->task;
		else
			kr = KERN_PROTECTION_FAILURE;
	} else
		kr = KERN_INVALID_ARGUMENT;

	unix_release();
	return kr;
}


/*
 *	fake_u:
 *
 *	fake a u-area structure for the specified thread.  Only "interesting"
 *	fields are filled in.
 */
fake_u(up, thread)
	register struct user	*up;
	register thread_t	thread;
{
	register struct utask	*utask;
	register struct uthread	*uthread;
	time_value_t	sys_time, user_time;
	register int	s, i;

	utask = thread->u_address.utask;
	uthread = thread->u_address.uthread;
#undef	u_pcb
	up->u_pcb = *(thread->pcb);
#ifdef	__hp_osf
	/* make these be offsets from the start of the kernel stack */
	{
	vm_offset_t offset = thread->kernel_stack + (KERNEL_STACK_SIZE - ctob(UPAGES));
	(vm_offset_t) up->u_pcb.user_regs  -= offset;
#undef	u_ar0
	up->u_ar0 = (int*) up->u_pcb.user_regs;
	(vm_offset_t) up->u_pcb.float_regs -= offset;
	(vm_offset_t) up->u_pcb.super_regs -= offset;
	}
#endif	/* __hp_osf */
#ifdef	vax
	/*	HACK HACK HACK	- keep adb happy	*/
	up->u_pcb.pcb_ksp = (up->u_pcb.pcb_ksp & 0x0FFFFFFF) | 0x70000000;
	/*	HACK HACK HACK	- keep adb happy	*/
#endif
#undef	u_comm
	bcopy(utask->uu_comm, up->u_comm, sizeof(up->u_comm));
#undef  u_nd
	up->u_nd = uthread->uu_nd;
#if	0				/* XXX */
#undef  u_cred
	up->u_cred = utask->uu_cred;
#endif
#undef	u_tsize
	up->u_tsize = utask->uu_tsize;
#undef	u_dsize
	up->u_dsize = utask->uu_dsize;
#undef	u_ssize
	up->u_ssize = utask->uu_ssize;
#undef	u_data_start
	up->u_data_start = utask->uu_data_start;
#undef	u_text_start
	up->u_text_start = utask->uu_text_start;
#undef	u_signal
	for (i = 0; i < NSIG; i++) {
		if (thread_signal_disposition(i))
			up->u_signal[i] = uthread->uu_tsignal[i];
		else
			up->u_signal[i] = utask->uu_signal[i];
	}
#undef	u_code
	up->u_code = uthread->uu_code;
#undef	u_procp
	up->u_procp = utask->uu_procp;
#undef	u_arg
	up->u_arg[0] = utask->uu_procp->p_cursig;
#undef	u_ru
	up->u_ru = utask->uu_ru;
#undef	u_acflag
        up->u_acflag = utask->uu_acflag;
#undef	u_cmask
        up->u_cmask = utask->uu_cmask;
#undef	u_start
        up->u_start = utask->uu_start;
#undef	u_logname
        bcopy(utask->uu_logname, up->u_logname, MAXLOGNAME - 1);
        
	/*
	 *	Times aren't in uarea any more.
	 */
	s = splsched();
	thread_lock(thread);
	thread_read_times(thread, &user_time, &sys_time);
	thread_unlock(thread);
	splx(s);
	up->u_ru.ru_stime.tv_sec = sys_time.seconds;
	up->u_ru.ru_stime.tv_usec = sys_time.microseconds;
	up->u_ru.ru_utime.tv_sec = user_time.seconds;
	up->u_ru.ru_utime.tv_usec = user_time.microseconds;
#undef	u_cru
	up->u_cru = utask->uu_cru;
	up->u_ru.ru_majflt = thread->thread_events.faults;
}

/*
 *	vm_keep_on_exec:
 *
 *	Set the keep-on-exec attribute of the specified vm region
 *	to "true".  This means the region will be kept in the address
 *	space when the task exec's.  Implements UNIX mmap(MAP_INHERIT)
 *	semantic.
 */

kern_return_t vm_keep_on_exec(addr, len)
	caddr_t	addr;
	u_int	len;
{
	return(vm_map_keep_on_exec(current_task()->map, trunc_page(addr),
				   round_page(addr+len), TRUE));
}


/*
 *	vm_exec:
 *
 *	Called to deallocate the current task's address space during exec.
 *	All regions except those marked keep_on_exec are deallocated;
 *	keep_on_exec regions are preserved.  The "ignore_koe" flag causes
 *	all regions (even those marked keep-on-exec) to be deallocated;
 *	this is used, for example, when exec'ing a setuid program.
 */

kern_return_t vm_exec(ignore_koe)
	boolean_t ignore_koe;
{
	vm_map_t	my_map;
	kern_return_t	rc;

	my_map = current_task()->map;

	((struct u_map_private *) (my_map->vm_private))->um_unload_all = 1;
	if (ignore_koe)
		rc = vm_map_delete(my_map, vm_map_min(my_map),
			vm_map_max(my_map), FALSE);
	else
		rc = vm_map_exec(my_map, vm_map_min(my_map), vm_map_max(my_map));
	((struct u_map_private *) (my_map->vm_private))->um_unload_all = 0;
	return(rc);
}



/*
 * vm_offset_t
 * vm_alloc_kva(vm_size_t size):
 *
 * Allocate a range of kernel virtual address space for use by
 * (for example) map_pva_kva(). No pages are allocated, but any
 * meta-information (ptepages, etc.) that the machine requires 
 * so that pmap_enter will function.  Faulting on this address space
 * or doing anything else will result in certain disaster.
 */

vm_offset_t
vm_alloc_kva(register vm_size_t size)
{
	vm_offset_t kvaddr=0;

	if (k_map_allocate_va(kernel_map, VM_OBJECT_NULL, &kvaddr, 
		(size = round_page(size)), TRUE) == KERN_SUCCESS) {
		pmap_pageable(pmap_kernel(), kvaddr, kvaddr + size, FALSE);
		return kvaddr;
	}
	else return (vm_offset_t) 0;
}

/*
 * vm_offset_t
 * map_pva_kva(struct proc *p, vm_offset_t va, vm_size_t size, vm_offset_t kva)
 *
 * map a specified range of user process virtual addresses
 * the given kernel virtual address. The kernel address space
 * must have been previously allocated via alloc_kva() or
 * kmem_alloc_pageable().
 * Since the process VA may not be aligned, the space allocated
 * must usually by one page large than count bytes.
 *
 * The process virtual address space from vaddr to vaddr+count
 * must be wired prior to calling this routine, by vm_map_pageable().
 * For example, a disk driver called from physio() will have been
 * properly initialized.
 *
 * The kernel VA corresponding to the process VA is returned
 * (including any offset into the first page.)
 */
vm_offset_t
map_pva_kva(p, vaddr, size, kvaddr)
struct proc *p;
vm_offset_t vaddr;
vm_size_t size;
vm_offset_t kvaddr;
{
	vm_offset_t kva;
	vm_offset_t o;
	vm_offset_t physaddr;
	pmap_t pmap = p->task->map->vm_pmap;

	if(!page_aligned(kvaddr))	/* kernel space must be aligned */
		panic("map_pva_kva kvaddr");

	o = vaddr - trunc_page(vaddr);	/* compute offset into page */
	vaddr -= o;			/* round down to page boundary */
	kva = kvaddr + o;	/* compute first kernel virtual address */
	size += o;		/* account for offset into first page */
	size = round_page(size);	/* will map whole pages */

	while (size) {
		physaddr = pmap_extract(pmap, vaddr);
		if (!physaddr)
			panic("map_pva_kva not resident");
		pmap_map(kvaddr, physaddr, physaddr+PAGE_SIZE, 
				VM_PROT_READ|VM_PROT_WRITE);
		size -= PAGE_SIZE;
		vaddr += PAGE_SIZE;
		kvaddr += PAGE_SIZE;
		physaddr += PAGE_SIZE;
	}
	/*
	 * return pointer to beginning of (vaddr,vaddr+size) 
	 * in kernel virtual space.
	 */
	return(kva);
}

/*
 * Semaphore initialization
 */

#define	NSMEM_LOCK	16

lock_t msmem_lock;
int nmsmem_lock = 0;
vm_offset_t msmem_hashmask;
vm_size_t msmem_shift;
extern zone_t vm_object_zone;

#define	msmem_lockp(OBJ)						\
	((((vm_offset_t) (OBJ) >> msmem_shift) & msmem_hashmask) + msmem_lock)

msmem_init()
{
	register int i, j;
	register vm_size_t osize;

	if (!nmsmem_lock) nmsmem_lock = NSMEM_LOCK;

	for (i = 1; i < nmsmem_lock; i <<= 1);
	nmsmem_lock = i;
	msmem_hashmask = i - 1;
	msmem_lock = (lock_t) kalloc(i * sizeof (lock_data_t));
	if (msmem_lock == (lock_t) 0)
		panic("msmem_init: failed to allocate lock array");
	for (j = 0; j < i; j++) lock_init(&msmem_lock[j], TRUE);
	osize = vm_object_zone->elem_size;
	for (i = 1; (1 << i) < osize; i++);
	msmem_shift = i;
}

msmem_control(vm_object_t obj,
	msemaphore *usem,
	vm_control_t control)
{
	register lock_t lp;
	kern_return_t ret;
	msemaphore sem;

	lp = msmem_lockp(obj);
	lock_write(lp);

	switch (control) {
	case VMC_SEM_SLEEP:
		sem.msem_wanted = TRUE;	
		if (copyout((caddr_t) &sem.msem_wanted, 
			(caddr_t) &usem->msem_wanted, 
			sizeof(sem.msem_wanted))) {
			ret = KERN_INVALID_ADDRESS;
			goto done;
		}
		else if (copyin((caddr_t) &usem->msem_state, 
			(caddr_t) &sem.msem_state,
			sizeof(sem.msem_state))) {
			ret = KERN_INVALID_ADDRESS;
			goto done;
		}
		else if (sem.msem_state == 0) {
			ret = KERN_SUCCESS;
			goto done;
		}
		else {
			vm_object_lock(obj);
			vm_object_assert_wait(obj,SEM,TRUE);
			vm_object_unlock(obj);
			lock_done(lp);
			thread_block();
			if (current_thread()->wait_result != THREAD_AWAKENED)
				return THREAD_INTERRUPTED;
			else return KERN_SUCCESS;
		}

	case VMC_SEM_WAKEUP:
		if (copyin((caddr_t) &usem->msem_wanted, 
			(caddr_t) &sem.msem_wanted, sizeof(sem.msem_wanted))) {
			ret = KERN_INVALID_ADDRESS;
			goto done;
		}
		else if (sem.msem_wanted != -1) {
			sem.msem_wanted = FALSE;	
			if (copyout((caddr_t) &sem.msem_wanted,
				    (caddr_t) &usem->msem_wanted,
				     sizeof(sem.msem_wanted))) {
				ret = KERN_INVALID_ADDRESS;
				goto done;
			}
		}
		vm_object_lock(obj);
		vm_object_wakeup(obj,SEM);
		vm_object_unlock(obj);
		lock_done(lp);
		return KERN_SUCCESS;
	}

done:
	lock_done(lp);
	return ret;
}
