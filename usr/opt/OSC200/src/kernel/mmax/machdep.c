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
static char	*sccsid = "@(#)$RCSfile: machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:13 $";
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
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <cpus.h>

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_kdb.h>

#include <mmax/reg.h>
#include <mmax/pte.h>
#include <mmax/psl.h>

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/map.h>
#include <sys/vm.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <ufs/inode.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/clist.h>
#include <sys/callout.h>
#include <sys/msgbuf.h>
#include <mach/machine.h>
#include <sys/time.h>
#include <ufs/ufsmount.h>
#include <ufs/fs.h>
#include <nfs/nfsnode.h>

#include <mmax/cpu.h>
#include <mmax/mtpr.h>

#include <kern/task.h>
#include <kern/thread.h>
#include <kern/lock.h>
#include <mach/boolean.h>
#include <kern/processor.h>
#include <kern/assert.h>

#include <vm/pmap.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <kern/zalloc.h>

#include <mmaxio/crqdefs.h>
#include <mmax/cpudefs.h>
#include <mmax/sccdefs.h>
#include <mmax/boot.h>
#include <mmax/isr_env.h>

extern	union boot	Boot;
extern	unsigned int	Bufpercentage;
extern	struct isr_que	*isr_q_slots;

#if	MMAX_DPC
/*
 * General counters, etc. used for keeping track of DPC slots
 * and DPC registers.
 */

dpcctl_t Dpc_ctl[NCPUS];
dpcprior_t Dpc_prior[NCPUS];
int Dpc_nmidiscnt[NCPUS];
#endif

extern int paniccpu;
extern int nchhashsize;

extern char	*shutl, *eshutl;
vm_offset_t	usrpt, eusrpt;

int	nbuf;
int	bufpages;

int	msgbuf_mapped = FALSE;

vm_map_t	buffer_map;

/*
 * Machine-dependent startup code
 */
startup(firstaddr)
	caddr_t	firstaddr;
{
	register int unixsize;
	register unsigned i;
	register struct pte *pte;
	int mapaddr, j;
	register caddr_t v;
	int base, residual;
	extern char etext;
	extern slave_stacks;
	vm_size_t	size;
	kern_return_t	ret;
	vm_offset_t	trash_offset;
	u_int		vnode_size;

	/*
	 * Good {morning,afternoon,evening,night}.
	 */

	panic_init();

	/*
	 *  Initialize the kernel debugger.  Cause a breakpoint trap to the
	 *  debugger before proceding any further if the proper option bit was
	 *  specified in the boot flags.
	 */
	if (boothowto&RB_HALT && boothowto&RB_DEBUG)
	    bpt("startup");

	printf("\n%s\n", version);
#define MEG	(1024*1024)
	printf("physical memory = %d.%d%d megabytes.\n", mem_size/MEG,
		((mem_size%MEG)*10)/MEG,
		((mem_size%(MEG/10))*100)/MEG);

	/*
	 * Allocate space for system data structures.
	 * The first available real memory address is in "firstaddr".
	 * The first available kernel virtual address is in "v".
	 * As pages of kernel virtual memory are allocated, "v" is incremented.
	 * As pages of memory are allocated and cleared,
	 * "firstaddr" is incremented.
	 * An index into the kernel page table corresponding to the
	 * virtual memory address maintained in "v" is kept in "mapaddr".
	 */
	/*
	 *	It is possible that someone has allocated some stuff
	 *	before here, (like vcons_init allocates the unibus map),
	 *	so, we look for enough space to put the dynamic data
	 *	structures, then free it with the assumption that we
	 *	can just get it back later (at the same address).
	 */
	firstaddr = (caddr_t) round_page(firstaddr);
	/*
	 *	Between the following find, and the next one below
	 *	we can't cause any other memory to be allocated.  Since
	 *	below is the first place we really need an object, it
	 *	will cause the object zone to be expanded, and will
	 *	use our memory!  Therefore we allocate a dummy object
	 *	here.  This is all a hack of course.
	 */
	ret = vm_map_find(kernel_map, vm_object_allocate(0), (vm_offset_t) 0, &firstaddr,
				8*1024*1024, TRUE);	/* size big enough for worst case */
	if (ret != KERN_SUCCESS) {
		printf("startup: no space for dynamic structures.\n");
		panic("startup");
	}
	vm_map_remove(kernel_map, firstaddr, firstaddr + 8*1024*1024);
	v = firstaddr;
#define valloc(name, type, num) \
	(name) = (type *)(v); (v) = (caddr_t)((name)+(num));\
	if (show_space) \
		printf("name = %d(0x%x) bytes @%x, %d cells @ %d bytes\n",\
		 num*sizeof(type), num*sizeof(type), name, num, sizeof (type))
#define valloclim(name, type, num, lim) \
	(name) = (type *)(v); (v) = (caddr_t)((lim) = ((name)+(num)));\
	if (show_space) \
		printf("name = %d(0x%x) bytes @%x, %d cells @ %d bytes\n",\
		 num*sizeof(type), num*sizeof(type), name, num, sizeof (type))
#define vround() (v = (caddr_t) ( ( ((int) v) + (NBPG-1)) & ~(NBPG-1)))
#define vquadround() (v = (caddr_t) ( ( ((int) v) + (8-1)) & ~(8-1)))

	vn_maxprivate = max(sizeof(struct inode),
			    sizeof(struct nfsnode));
	vnode_size = ((u_int)(sizeof(struct vnode) + vn_maxprivate -4));
	vnode = (struct vnode *)(v);
	vnodeNVNODE = (struct vnode *)
		((u_int) vnode + nvnode * vnode_size);
	(v) = (caddr_t)vnodeNVNODE;

	valloclim(file, struct file, nfile, fileNFILE);
	valloclim(proc, struct proc, nproc, procNPROC);
	valloc(cfree, struct cblock, nclist);
	valloc(callout, struct callout, ncallout);
	valloc(namecache, struct namecache, nchsize);
#ifndef ISR_THREADS_TEST
	valloc(isr_q_slots, struct isr_que, Boot.b_size_isrqueue);
#else
	valloc(isr_q_slots, struct isr_que, B_SIZE_ISRQUEUE);
#endif
	nchsz = rndup(nchsz, MINNCHSZ);
	valloc(nchash, struct nchash, nchsz);
	inohsz = rndup(inohsz, MININOHSZ);
	valloc(ihead, struct ihead, inohsz);
	bufhsz = rndup(bufhsz, MINBUFHSZ);
	valloc(bufhash, struct bufhd, bufhsz);
	spechsz = rndup(spechsz, MINSPECHSZ);
	valloc(speclisth, struct spechash, spechsz);
	valloc(mounttab, struct ufsmount, nmount);
	
	/*
	 * Calculate buffer size.  This scheme uses the
	 *  Encore SYSPARAM file to pass in the percentage
	 *  of memory we want to use for buffer space.
	 */
	if (Bufpercentage >= 50)
		printf("Warning:  Buffer cache will consume %d%% of available memory\n", Bufpercentage);
	bufpages = atop((mem_size/100) * Bufpercentage);
	nbuf = ptoa(bufpages) / (8 * 1024);

	if (bufpages > nbuf * (MAXBSIZE / page_size))
		bufpages = nbuf * (MAXBSIZE / page_size);
	valloc(buf, struct buf, nbuf);

	/*
	 * Clear space allocated thus far, and make r/w entries
	 * for the space in the kernel map.
	 */
	v = (caddr_t) round_page(v);
	size = (vm_size_t) (v - firstaddr);
	ret = vm_map_find(kernel_map, vm_object_allocate(size),
			(vm_offset_t) 0, &firstaddr, size, FALSE);
	if (ret != KERN_SUCCESS) {
		panic("startup: unable to allocate kernel data structures");
	}
	vm_map_pageable(kernel_map, firstaddr, v, VM_PROT_READ|VM_PROT_WRITE);
	/* set firstaddr to the next address to be allocated */
	firstaddr = v;

	/*
	 * Now allocate buffers proper.  They are different than the above
	 * in that they usually occupy more virtual memory than physical.
	 */
	valloc(buffers, char, MAXBSIZE * nbuf);
	base = bufpages / nbuf;
	residual = bufpages % nbuf;
	if (show_space) {
		printf("bufpages = %d, nbuf = %d, base = %d, residual = %d\n",
				bufpages, nbuf, base, residual);
	}
	/*
	 *	Allocate virtual memory for buffer pool.
	 */
	v = (caddr_t) round_page(v);
	size = (vm_size_t) (v - firstaddr);
	buffer_map = kmem_suballoc(kernel_map, &firstaddr,
		&trash_offset /* max */, size, TRUE);
	ret = vm_map_find(buffer_map, vm_object_allocate(size), (vm_offset_t) 0,
				&firstaddr, size, FALSE);
	if (ret != KERN_SUCCESS) {
		panic("startup: unable to allocate buffer pool");
	}

	for (i = 0; i < nbuf; i++) {
		vm_size_t	thisbsize;
		vm_offset_t	curbuf;

		/*
		 * First <residual> buffers get (base+1) physical pages
		 * allocated for them.  The rest get (base) physical pages.
		 *
		 * The rest of each buffer occupies virtual space,
		 * but has no physical memory allocated for it.
		 */

		thisbsize = page_size*(i < residual ? base+1 : base);
		curbuf = (vm_offset_t)buffers + i * MAXBSIZE;
		vm_map_pageable(buffer_map, curbuf, curbuf+thisbsize,
			VM_PROT_READ|VM_PROT_WRITE);
	}


	/*
	 * Initialize callouts
	 */
	callfree = callout;
	for (i = 1; i < ncallout; i++)
		callout[i-1].c_next = &callout[i];

	/*
	 * Initialize memory allocator and swap
	 * and user page table maps.
	 *
	 * THE USER PAGE TABLE MAP IS CALLED ``kernelmap''
	 * WHICH IS A VERY UNDESCRIPTIVE AND INCONSISTENT NAME.
	 */
	{
		register int	nbytes;
		extern int	vm_page_free_count;

		nbytes = ptoa(vm_page_free_count);
		printf("available memory = %d.%d%d megabytes.\n", nbytes/MEG,
			((nbytes%MEG)*10)/MEG,
			((nbytes%(MEG/10))*100)/MEG);
		nbytes = ptoa(bufpages);
		printf("using %d buffers containing %d.%d%d megabytes of memory\n",
			nbuf,
			nbytes/MEG,
			((nbytes%MEG)*10)/MEG,
			((nbytes%(MEG/10))*100)/MEG);
	}

	/* do init here so ethernet driver can get mbufs */
	mbinit();

	user_pt_map = kmem_suballoc(kernel_map, &usrpt, &eusrpt,
		8*MEG, FALSE);

	/*
	 * Configure the system.
	 */

	configure();
}

/*
 * Macro to convert a signal number to a signal mask.
 */
#define MASK(s)		(1<<((s)-1))

/*
 * Definition of the signal frame that the user will see.
 */

struct	sigframe {
	int	sf_signum;		/* Signal number.	     */
	int	sf_code;		/* Signal code.		     */
	struct	sigcontext *sf_scp;	/* Signal context pointer.   */
	int	sf_handler;		/* Signal handler.	     */
};



/*****************************************************************************
 *
 * NAME:
 *	sendsig -- Send a signal to a user process.
 *
 * DESCRIPTION:
 *
 * u_sigcatch contains the function address of the signal catcher.
 *
 * ARGUMENTS:
 *	funcadr		- Address of signal handler in user space
 *	sig		- Signal number
 *	sig_mask	- Old signal mask to be restored.
 */

sendsig (funcadr, sig, sig_mask)
register u_int	funcadr;		/* Address of the signal handler.   */
	 int	sig;			/* The signal number to send.	    */
	 int	sig_mask;		/* The old signal mask.		    */
{
	 struct	sigcontext scp;		/* Saved context.		    */
register struct	sigcontext *scp_ptr;	/* Pointer to above. (user space)   */
	 char	    *old_sp;		/* Old user stack pointer.	    */
	 struct	sigframe   fp;		/* Signal frame.		    */
register struct	sigframe   *fp_ptr;	/* Pointer to above. (user space)   */
register int	    *regs;		/* Pointer to the saved registers.  */
	 int	    old_onstack;	/* Saved u.u_onstack value.	    */
	 int	    newpc;		/* New user pc.			    */
	 int	    descptr;		/* Signal handler descriptor.	    */

/*
 * Initialize the various pointers and values.
 */
	ASSERT(syscall_on_master());
	regs = u.u_ar0;
	old_onstack = u.u_onstack;
	old_sp = (char *) regs[SP];
	scp_ptr = (struct sigcontext *) old_sp - 1;
/*
 * If there is not a signal on the user signal stack and the user wants this
 * signal to go on the signal stack, then set the signal frame pointer to the
 * stack and set the status of the stack as used.  Otherwise use the current
 * user stack.
 */
	if (!old_onstack && (u.u_sigonstack & MASK(sig))) {
		fp_ptr = (struct sigframe *)u.u_sigsp - 1;
		u.u_onstack = 1;
	}
	else
		fp_ptr = (struct sigframe *)scp_ptr - 1;
/*
 * If the user stack needs to be grown and it can be grown then do so.
 */
	if (!old_onstack && (int)fp_ptr <= USRSTACK - ctob(u.u_ssize))
		grow ((unsigned)fp_ptr);
	if (!u.u_onstack && (int)scp_ptr <= USRSTACK - ctob(u.u_ssize))
		grow ((unsigned)scp_ptr);
/*
 * Build the signal frame and copy it out to the user area.  If the signal is
 * a SIGILL or a SIGFPE then the code is the u.u_code else it is 0.
 */
	fp.sf_signum = sig;
	fp.sf_scp = scp_ptr;
	fp.sf_handler = funcadr;
	if (sig == SIGILL || sig == SIGFPE) {
		fp.sf_code = u.u_code;
		u.u_code = 0;
	}
	else
		fp.sf_code = 0;

	if (copyout (&fp, fp_ptr, sizeof (struct sigframe)))
		goto bad;
/*
 * Build the sigcontext frame on the user stack.
 */
	scp.sc_onstack = old_onstack;
	scp.sc_mask = sig_mask;
/*
 * 	If the executable is COFF format, then the restored stack pointer
 *	is the current user stack pointer.  Despite what it may say in
 *	sigentry (in libc), osigcleanup returns directly to the interrupted
 *	user code.
 */
	scp.sc_sp = (int) old_sp;
	scp.sc_pc = regs[PC];
	scp.sc_ps = regs[PSR];
	scp.sc_r0 = regs[R0];
	scp.sc_r1 = regs[R1];
	scp.sc_r2 = regs[R2];
	if (copyout (&scp, scp_ptr, sizeof (struct sigcontext)))
		goto bad;

	newpc = (int) u.u_sigcatch;
/*
 * Set the return stack and program counter.  This will cause the system to
 * return to the signal handler instead of the interrupted code.
 */
	regs[SP] = (int) fp_ptr;		/* set new stack pointer */
	regs[PC] = newpc;			/* set new pc */

	return (0);

bad:
/*
 * Here if the signal cannot be sent.  Cause the process to fail with an
 * illegal instruction.
 */
	signal_disposition(SIGILL) = SIG_DFL;
	sig = MASK (SIGILL);
	u.u_procp->p_sigignore &= ~sig;
	u.u_procp->p_sigcatch  &= ~sig;
	u.u_procp->p_sigmask   &= ~sig;
	psignal(u.u_procp, SIGILL);
}

/*****************************************************************************
 *
 * NAME:
 *	osigcleanup
 *
 * DESCRIPTION:
 * 	cleans state after a signal.  This includes reseting the
 *	users stack pointer to the interrupted process', reseting the signal
 *	mask, and reseting the u.u_onstack flag.
 *
 */
osigcleanup ()
{
	 struct	sigcontext	scp;
register struct sigcontext	*scp_ptr;
register struct	sigframe	*sfp;
register int *regs = u.u_ar0;
	 int error;

	/* Go through the signal frame to get the signal context from the
	 *	users address space.
	 */
	sfp = (struct sigframe *)regs[SP];
	scp_ptr = (struct sigcontext *)fuword (&(sfp->sf_scp));
	if (error = copyin (scp_ptr, &scp, sizeof (struct sigcontext)))
		return (error);

	/* Restore the signal context.
	 */
	u.u_onstack = scp.sc_onstack & 01;
	u.u_procp->p_sigmask =
		scp.sc_mask & ~sigcantmask;
	regs[SP] = scp.sc_sp;		/* set new stack pointer */
	regs[R0] = scp.sc_r0;
	regs[R1] = scp.sc_r1;
	regs[R2] = scp.sc_r2;
/*
 * Insure that the restored psr has user mode, user stack and interrupts
 * enabled.
 */
	scp.sc_ps |= ((PSR_U | PSR_S | PSR_I) << 16);
	regs[PSR] = scp.sc_ps;
	regs[PC] = scp.sc_pc;
	return (0);
}

/*
 * System call to cleanup state after a signal
 * has been taken.  Reset signal mask and
 * stack state from context left by sendsig (above).
 * Return to previous pc and psl as specified by
 * context left by sendsig. Check carefully to
 * make sure that the user has not modified the
 * psl to gain improper priviledges or to cause
 * a machine fault.
 */
sigreturn(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		struct sigcontext *sigcntxp;
	} *uap = (struct args *) args;
	 struct	sigcontext	scp;
	register struct sigcontext	*scp_ptr;
	register int *regs = u.u_ar0;

	/* Go through the signal frame to get the signal context from the
	 *	users address space.
	 */
	scp_ptr = uap->sigcntxp;
	if (copyin (scp_ptr, &scp, sizeof (struct sigcontext)))
		return (EINVAL);

	u.u_onstack = scp.sc_onstack & 01;
	u.u_procp->p_sigmask = scp.sc_mask & ~sigcantmask;

	regs[SP] = scp.sc_sp;		/* set new stack pointer */
	regs[R0] = scp.sc_r0;
	regs[R1] = scp.sc_r1;
	regs[R2] = scp.sc_r2;
/*
 * Insure that the restored psr has user mode, user stack and interrupts
 * enabled.
 */
	scp.sc_ps |= ((PSR_U | PSR_S | PSR_I) << 16);
	regs[PSR] = scp.sc_ps;
	regs[PC] = scp.sc_pc;
	return (EJUSTRETURN);
}


/*
 * Set registers on exec
 */
setregs(entry, mod)
	u_long entry;
{
	vm_map_t	map;
	vm_offset_t	addr;

	mod &= 0xffff;

	/* allocate a page for MOD table if one not already present */
	map = current_task()->map;
	if (!vm_map_check_protection(map, (vm_offset_t)mod, (vm_size_t)16,
				     VM_PROT_READ)) {
		addr = trunc_page(mod);
		(void)vm_allocate(map, &addr, PAGE_SIZE, FALSE);
	}

	u.u_ar0[PC] = entry;
	u.u_ar0[PS] = (u.u_ar0[PS] & 0xffff0000) | (mod & 0xffff);
}

int	waittime = -1;
int	nmi_panic = 0;

boot(paniced, arghowto)
	int paniced, arghowto;
{
	register int howto;
	register int devtype;
	register int i, halted;
	register processor_t	ourprocessor;
	extern	struct lock toy_lock;
	static	panic_record = 0;

#ifdef	lint
	howto = 0; devtype = 0;
	printf("howto %d, devtype %d\n", arghowto, devtype);
#endif
	if ((arghowto&RB_PANIC) && panic_record++ != 0) {
		printf ("** Multi-Panic, skipping sync and dump **\n");
		goto multi_panic;
	}
	howto = arghowto;
	if ((howto&RB_NOSYNC)==0 && nmi_panic == 0 &&
		waittime < 0 && bfreelist[0].b_forw) {
		/*
		 *  Force an accurate time into the root file system super-
		 *  block.
		 */
		mounttab[0].um_fs->fs_fmod = 1;		/* XXX */
		waittime = 0;
		(void) splnet();
		printf("syncing disks... ");
		/*
		 * Release inodes held by texts before update.
		 * Need comparable routine for MACH_XP case.  XXX
		vm_object_cache_clear();
		 */
		

		sync((struct proc *)NULL, (void *)NULL, (int *)NULL);

		{
		  register struct buf *bp;
		  int iter, nbusy;
		  int obusy = 0;
		  register int i;
		  register int locked;

		  for (iter = 0; iter < 20; iter++) {
			nbusy = 0;
			for (bp = &buf[nbuf]; --bp >= buf; ) {
				BUF_LOCK_TRY(bp, locked);
				if (locked) {
					BUF_UNLOCK(bp);
					continue;
				}
				/* XXX cheat!!! XXX */
				if (!(bp->b_flags & B_INVAL))
					nbusy++;
			}
			if (nbusy == 0)
				break;
			printf("%d ", nbusy);
		        if (nbusy != obusy)
				iter = 0;
			obusy = nbusy;
			/*
			 * Small delay, linearly increasing with iteration.
			 */
			i = 40000 * iter;
			while(i--)
				;
		  }
		}
		printf("done\n");
		/*
		 * If we've been adjusting the clock, the todr
		 * will be out of synch; adjust it now.
		 *
		 * Re-initialize the lock in case some other thread
		 * was actually using the lock at panic time.  Assume
		 * current thread is the only one active.
		 */
		lock_init(&toy_lock, TRUE);
		TIME_LOCK_INIT();
		resettodr();
	}

	if (paniced == RB_PANIC) dumpsys();

	splhigh();	/* extreme priority */
	ourprocessor = current_processor();
	halted = TRUE;
	for (i = 0; i < NCPUS; i++) {
	    register int j,k;

#define LOOPCOUNT 1000
#define WAITCOUNT 1000

	    if ((processor_ptr[i]->state != PROCESSOR_OFF_LINE) &&
		processor_ptr[i] != ourprocessor) {
		processor_shutdown(processor_ptr[i]);

		j = 0;
		while ( machine_slot[i].running && j < LOOPCOUNT) {

		    for (k=0; k < WAITCOUNT; k++);

		    j++;
		}

		if (machine_slot[i].running) {
		    printf("Processor %d not halted\n",i);
		    halted = FALSE;
		}
	    }
	}

	if (halted == TRUE)
	    printf("\nSystem halted.\n");
	else
	    bpt("boot");

multi_panic:
	if (howto & RB_HALT)
		halt_cpu();
	else {
		scc_reboot();
		halt_cpu();      /* In case scc_reboot finishes before */
	}

	/*NOTREACHED*/
}


int	dumpmag = 0x8fca0101;	/* magic number for savecore */
int	dumpsize = 0;		/* also for savecore */
/*
 * Doadump comes here after turning off memory management and
 * getting on the dump stack, either when called above, or by
 * the auto-restart code.
 */
dumpsys()
{
    long numblks;
    int start_addr;
    int i, error, sts, restartok;

#ifdef	notdef
	if ((minor(dumpdev)&07) != 1)
		return;
#endif

/*
 * Reset the SCC and all the EMCs
 * so that crash dump can operate properly.
 */

    printf("Restarting EMCs and SCC...");
    restartok = TRUE;
    (void)log_scc_attn(REQ_CRQ(SCC_SLOT, 0));	/* drain scc attn queue */
    for (i=0; i<NUM_SLOT; i++) {
	if (is_emc(Boot.boot_slotid[i])) {
	    sts = polled_warm_restart(i);
	    if (sts != 0) {
		printf("\nRestart of EMC in slot %d failed with code 0x%x\n",
		    i, sts);
		restartok = FALSE;
	    }
	}
    }
    sts = polled_warm_restart(SCC_SLOT);
    if ( sts != 0 ) {
	printf("\nRestart of SCC failed with code 0x%x\n", sts);
	restartok = FALSE;
    }
    if (restartok)  printf("\n");

    numblks = mem_size / DEV_BSIZE;
    start_addr = 0;
    printf("\ndumping %d blocks to dev 0x%x block %d from addr 0x%x\n",
	 numblks, dumpdev, dumplo, start_addr);
    printf("dump ");
    BDEVSW_DUMP(major(dumpdev), dumpdev, dumplo, start_addr,
		numblks, B_WRITE, error);

    switch(error) {
	case ENODEV:
		printf("device does not exist\n");
		break;

        case ENXIO:
                printf("device bad\n");
		break;

	case EFAULT:
		printf("device not ready\n");
		break;

	case EINVAL:
		printf("area improper\n");
		break;

	case EIO:
		printf("i/o error\n");
		break;

	default:
		printf("succeeded\n");
		break;
	}
}

/*
 * Return the best possible estimate of the time in the timeval
 * to which tvp points.  We do this by reading the interval count
 * register to determine the time remaining to the next clock tick.
 * We must compensate for wraparound which is not yet reflected in the time
 * (which happens when the ICR hits 0 and wraps after the splhigh(),
 * but before the mfpr(ICR)).  Also check that this time is no less than
 * any previously-reported time, which could happen around the time
 * of a clock adjustment.
 *
 *  Just for fun, we guarantee that the time
 * will be greater than the value obtained by a previous call.
 *
 * WHAT KIND OF FUN IS THIS?  Just for the hell of it, I'm NOT
 *	going to do this, geez!	jb
 */
microtime(tvp)
	register struct timeval *tvp;
{
	int s = splhigh();
	extern	unsigned int	Last_frcount[];
	unsigned int	last_count;
	u_int t;

	TIME_READ_LOCK();
	*tvp = time;
	TIME_READ_UNLOCK();
	t =  FRcounter;
	last_count = Last_frcount[master_cpu];
	if (t > last_count)
		t = t - last_count;
	else
		t = (int)t - (int)last_count;

	tvp->tv_usec += t;
	if (tvp->tv_usec > 1000000) {
		tvp->tv_sec++;
		tvp->tv_usec -= 1000000;
	}
	splx(s);
}

physstrat(bp, strat, prio)
	struct buf *bp;
	int (*strat)(), prio;
{
	int s;

	(*strat)(bp);
	LASSERT(BUF_LOCK_HOLDER(bp));
	(void) event_wait(&bp->b_iocomplete, FALSE, 0);
}

/*
.* startCPU - random code needed for starting up a new CPU
 *
.* ARGUMENTS:
 *
.* cpuid - processor number of this CPU (i.e. CPU being booted)
 *
.* USAGE: This routine is called late in the initialization of
 *	any CPU being brought up.
 *
.* ASSUMPTIONS: The CPU is running in virtual mode, but
 *	interrupts are still disabled.
 */

startCPU (cpuid)
register cpuid;
{
#if	MMAX_XPC
	init_vbga(cpuid);
#endif
#if	MMAX_XPC || MMAX_APC
	CPUicuenable();
#endif
	drain_vector_fifo();
	/*
	 * Read the error register to clear any latent error conditions.
	 */
	clrnmists();
#if	UNIX_LOCKS
	set_class(cpuid, MASTER_CLASS);
#else
	set_class(cpuid, cpuid == master_cpu ? MASTER_CLASS : SLAVE_CLASS);
#endif
#if	MMAX_APC
        /*
         * Initialize and load the APC control register.
         */
	*(APCREG_CTL) = APCCTL_INIT_VALUE;
#endif
#if	MMAX_DPC
	/*	Disable bus error NMIs, send NMIs to CPU a,
	 *	and disable the Extended Translation Buffer.
	 */
	Dpc_ctl[cpuid].l = DPCCTL_BUS_SOFT_ERR_DIS | DPCCTL_SWITCH_NMI_A |
				DPCCTL_ETLB_OFF;
	if ((boothowto & RB_MULTICPU) == 0)
		if (cpuid & (1<<0))	/* bit zero */
			Dpc_ctl[cpuid].l |= DPCCTL_BLOCK_A_REQ;
		else
			Dpc_ctl[cpuid].l |= DPCCTL_BLOCK_B_REQ;
	*((long *)DPCREG_CTL) = (Dpc_ctl[cpuid].L ^ DPCCTL_FIX);
#endif
}

/*
 * clrnmists - internal routine used by startCPU() to clear NMI status
 *
 * Only exists because GHC tends to optimize out in-line attempts at
 * clearing the status...
 */

static
clrnmists()
{
#if	MMAX_XPC || MMAX_APC
	return (GETCPUERR);
#endif
#if	MMAX_DPC
	return(*((long *)DPCREG_NMI));
#endif
}

cnputc(c)
{
	slputc(c);
}

simple_lock_data_t	nmi_lock;
#if	MMAX_DPC
static long             lastnmi[NCPUS];
#endif

/*
.* nmi_trap - handle an NMI trap
 *
.* RETURNS:
 *	This routine returns a return value telling the low-level
 *	trap handler what action to take next. NMI_RESUME tells
 *	the low-level trap handler to resume as if nothing had
 *	happened. NMI_WAIT tells the low-level trap handler to
 *	spin its wheels and wait for a system reboot to occur.
 *	NMI_DEBUG tells the low-level trap handler to exit to
 *	the system debugger so it can handle the trap in its
 *	inimitable fashion. NMI_PANIC tells the low-level trap
 *	handler to make the system panic.
 *
.* USAGE:
 *	NMI traps have assorted uses that are dependent upon the
 *	architecture.
 *
 *	On the MULTIMAX NMIs are used to report all sorts of error
 *	conditions by the board hardware. They are also used to
 *	halt and synchronize all processors when a single processor
 *	hits a kernel breakpoint or kernel trace trap. They are
 *	also used to halt and synchronize all processors when
 *	the system panics. They are also used to halt a single
 *	processor by sending an NMI to it via the system console.
 *
 *	It is the job of this routine to differentiate between all the
 *	above cases, and take appropriate action.
 */

extern in_dbmon;
int panicnmiflag[NCPUS];

nmi_trap()
{
#if	MMAX_DPC
	dpcnmi_t nmi_sts;
#endif
	int cpuid;

	cpuid = getcpuid();

#if	MMAX_XPC || MMAX_APC
	/*
	 * If this is the first time through for this CPU and a panic
	 * is in progress, then make locore.s do the panic handling for the
	 * CPU (by passing the correct return code). Panic handling for the
	 * panicing CPU is to invoke debugger if present.  Panic handling
	 * for the non-boot  CPU is to halt the CPU and wait for the reboot.
	 */
	if(panicstr) {				/* PANIC */
		if (panicnmiflag[cpuid] == 0)
			panicnmiflag[cpuid] = 1;
		if (cpuid == paniccpu) {
			if ((boothowto & RB_DEBUG) == RB_DEBUG)
				return(NMI_DEBUG);
			else
				return(NMI_RESUME);
		}
		else {
#undef	boot_cpuid
			extern int boot_cpuid;
			if (cpuid == boot_cpuid)
				return(NMI_DEBUG);
			else
				return(NMI_WAIT);
		}
	}
	else {					/* No PANIC */
		if ((boothowto & RB_DEBUG) == RB_DEBUG)
			return(NMI_DEBUG);
		else
			return(NMI_PANIC);
	}
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
	/*
	 * On the DPC, read the NMI status register. This clears
	 * NMIs and determines further course of action.
	 */
	nmi_sts.l = ((*(long *)DPCREG_NMI ^ DPCNMI_FIX) & DPCNMI_MASK);
	lastnmi[cpuid] = nmi_sts.l;

	/*
	 * If this is the first time through for this CPU and a panic
	 * is in progress, then make locore.s do the panic handling for the
	 * CPU (by passing the correct return code). Panic handling for the
	 * panicing CPU is merely to resume. Panic handling for the non-boot
	 * CPU is to halt the CPU and wait for the reboot.
	 */
	if (panicstr) {
	    if (!panicnmiflag[cpuid]) {
		panicnmiflag[cpuid] = 1;
		if (cpuid == paniccpu)
		    return(NMI_RESUME);
		else
	 	    return(NMI_WAIT);
	    }
	}

	/*
	 * A system NMI that is NOT the first NMI after a panic is either a
	 * debug NMI or a kernel profiling NMI.
	 */
	if (nmi_sts.f.n_system_nmi) {
	    if ((boothowto & RB_DEBUG) == RB_DEBUG)
		return(NMI_DEBUG);
	    else
		return(NMI_PANIC);
	}

	/*
	 * If we got an error of some sort and we are currently within
	 * the confines of dbmon, then merely print out a message and
	 * continue...
	 */
	if (in_dbmon) {
		printf("NMI within dbmon. NMI status = 0x%x\n",nmi_sts.l);
		return(NMI_RESUME);
	}

	/*
	 * We got either a fatal NMI, a non-fatal NMI. For fatal NMIs log the
	 * error and make trap.s panic the system. For non-fatal NMIs just
	 * log the error and resume.
	 */
	if (nmi_sts.l & DPCNMI_HARDERR) {
		simple_lock(&nmi_lock);
		printf("Cpu %d: FATAL NMI received, status=0x%x.\n",
			getcpuid(),nmi_sts.l);
		simple_unlock(&nmi_lock);
		return(NMI_PANIC);
	} else if (nmi_sts.l) {
		simple_lock(&nmi_lock);
		printf("Cpu %d: Non-fatal NMI received, status=0x%x.\n",
			getcpuid(),nmi_sts.l);
		simple_unlock(&nmi_lock);
		return(NMI_RESUME);
	}

	/*
	 * We got an NMI that was neither fatal nor non-fatal. Just resume.
	 */
	return(NMI_RESUME);
#endif	/* MMAX_DPC */
}


/*
 *	vatopa -- convert virtual to physical address. Along with the address,
 *		the caller passes a length in virtual space. Vatopa determines
 *		how much of this length is in contiguous physical space and
 *		returns this information.  See below for details.
 *
 * ARGUMENTS:
 *	virt_addr:	Starting virtual address
 *	virt_len:	Length (in bytes) in virtual space
 *	&phys_addr:	Pointer to starting physical address (output)
 *	&phys_len:	Pointer to contiguous length in physical space (output)
 *	mproc:	process to translate for, kernel if 0.
 *
 * RETURN VALUE:
 *	0:	All the memory in the contiguous range is present.
 *	1:	The memory at virt_addr is not present in physical memory.
 */

vatopa (virt_addr, virt_len, phys_addr, phys_len, mproc)
u_int	virt_addr;		/* Starting virtual address */
u_int	virt_len;		/* Length to validate */
u_int	*phys_addr;		/* Starting physical address */
u_int	*phys_len;		/* Contiguous physical size */
struct proc *mproc;
{
vm_offset_t	this_addr, next_addr;	/* for finding max contig. */
u_int	bytes_in_page;		/* how much of this page is needed ? */
pmap_t	pmap;

	if (mproc == 0) {
		/* KERNEL */
		pmap = pmap_kernel();
	}
	else {
		/* USER */
		pmap = mproc->task->map->pmap;
	}

	this_addr = pmap_extract(pmap, virt_addr);
	if (this_addr == 0)
		return(1);
	*phys_addr = this_addr;
	bytes_in_page = min(PAGE_SIZE - (virt_addr % PAGE_SIZE), virt_len);
	*phys_len = virt_len;
	while (virt_len > 0) {
		virt_addr += bytes_in_page;
		virt_len -= bytes_in_page;
		next_addr = pmap_extract(pmap, virt_addr);
		if(next_addr != (this_addr + bytes_in_page))
			break;
		this_addr = next_addr;
		bytes_in_page = min(PAGE_SIZE, virt_len);
	}
	*phys_len -= virt_len;
	return (0);
}

dbg_printf(fmt, x1, x2)
{
	printf(fmt, x1, x2);
}

halt_cpu()
{
	machine_slot[getcpuid()].running = FALSE;
	for (;;);	/* infinite loop, don't want to halt on mmax */
}

/* ARGSUSED */
set_class(cpuid, which_class)
register	cpuid;
int		which_class;
{
#if	MMAX_XPC
	*XPCVB_CLASS = which_class;
#endif	MMAX_XPC
#if	MMAX_APC
	*APCREG_CSR = (which_class << APCCSR_CLASSSHFT);
#endif	MMAX_APC
#if	MMAX_DPC
	Dpc_prior[cpuid].f.p_pri = 0;
	Dpc_prior[cpuid].f.p_class = which_class;
	*((char *)DPCREG_PRIOR) = Dpc_prior[cpuid].c ^ DPCPRIOR_FIX;
#endif	MMAX_DPC
}


#if	MMAX_XPC
init_vbga(cpu_id)
int	cpu_id;
{
	u_short	vb_err_reg;

	*XPCVB_DIAG_CONTROL = XPCVB_DC_VBOFF;	/* turn off vbga */

	/* disable receipt of lamp vectors (only applies to CPU A) */
	if ((cpu_id & 1) == 0)
		*XPCVB_LAMP_RECEPTOR = 0;

	*XPCVB_CLASS = 0;		/* only directed vectors for now */

	drain_vector_fifo();

	/* Program this CPU's slot number into the VBGA and turn it on. */
	*XPCVB_DIAG_CONTROL = XPCVB_DC_NORMAL
				| (GETCPUSLOT << XPCVB_DC_SLOT_ID_SHFT);

	vb_err_reg = *XPCVB_ERROR;	/* clear the error status register */

	return (vb_err_reg);		/* trick the optimizer */
}
#endif	/* MMAX_XPC */

drain_vector_fifo()
{
	while (FIFO_NOT_EMPTY)		/* drain the vector fifo */
		read_fifo();
}
