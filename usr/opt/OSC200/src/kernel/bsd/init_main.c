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
static char	*rcsid = "@(#)$RCSfile: init_main.c,v $ $Revision: 4.3.31.16 $ (DEC) $Date: 1993/11/02 15:30:56 $";
#endif
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
 * init_main.c
 *
 * Modification History:
 *
 * 04-Feb-92	Jeff Denham
 *	Add initialization for the POSIX.4 realtime timer and consolidate
 *	initialization for all realtime components.
 *
 * 27-Oct-91	Fred Canter
 *	Moved vm_initial_limit_{stack,data,core} to param.c so these
 *	limits can be established via the kernel config file.
 *
 * 27-Jun-91	Scott Cranston
 *    Change optional binary error logging support option compilation
 *    method...
 *	- added #include <uerf.h>
 *	- changed instances of '#ifdef _UERF' to '#if UERF'
 *
 * 10-june-91 Brian Harrigan
 *    Fixed rt_preempt_opt so that it will pick up its value at
 *    run time.
 * 
 * 6-June-91  Brian Stevens
 *    Fixed maxuprc declaration so that it will pick up its value at
 *    run-time (to allow binary configuration).
 *
 * 5-Jun-91	Scott Cranston
 *	Add declaration and initialization of error logging lock (lk_errlog)
 *      in global_lock_initialization().
 *
 * 3-Jun-91     Diane Lebel
 *	Added support for > 64 open file descriptors per process
 *
 * 30-May-91	Michael Schmitz
 *	Added code to declare and initialize the atomic_op_lock.
 *
 * 5-May-91	Ron Widyono
 *	Incorporate run-time option for kernel preemption (rt_preempt_enabled).
 *
 * 11-Apr-91     Lai-Wah
 *      Added P1003.4 required extensions.  
 *      Specifically <rt_timer.h> is now included and if RT_TIMER
 *      is defined  psx_driftrate is created and initalized to 0.
 *
 * 6-Apr-91	Ron Widyono
 *	Initialize simple lock counter and turn on lock counting for
 *	preemption points.  Conditionalized by RT_PREEMPT.
 *
 * 16-Jan-91	Fred Canter
 *	Include confdeg.h to correct missing dependency (MAXDSIZ) in
 *	DEFAULT config file.
 *
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

 */

#include <confdep.h>
#include "bin_compat.h"
#include "_lmf_.h"

#include <mach_device.h>
#include <mach_host.h>
#include <mach_km.h>
#include <mach_net.h>
#include <xpr_debug.h>
#include <cpus.h>
#include <mach_emulation.h>
#include <streams.h>
#include <rt_preempt.h>
#include <bsd_tty.h>		/* for define of BSD_TTY */

#include <cputypes.h>
#include <sys/secdefines.h>

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/clist.h>
#include <sys/dk.h>
#include <sys/table.h>
#include <sys/lock_types.h>
#include <sys/kernel.h>
#include <machine/reg.h>
#include <machine/cpu.h>
#include <machine/hal/cpuconf.h>

#include <kern/thread.h>
#include <kern/task.h>
#include <mach/machine.h>
#include <kern/timer.h>
#include <sys/version.h>
#include <machine/pmap.h>
#include <mach/vm_param.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <mach/boolean.h>
#include <kern/sched_prim.h>
#include <kern/thread_swap.h>
#include <kern/task_swap.h>
#include <kern/zalloc.h>
#include <kern/kern_obj.h>
#include <kern/kern_port.h>
#include <kern/ipc_copyout.h>
#include <mach/memory_object.h>
#include <mach/task_special_ports.h>
#include <builtin/ux_exception.h>
#include <sys/utsname.h>
#include <vm/vm_tune.h>
#include <rt_timer.h>
#include <rt_sem.h>

#ifdef	multimax
#include <mst.h>
#if	NMST
#include <mstopt.h>
#endif
#endif


extern int do_mscp_poll_wait;
extern void mscp_poll_wait();
extern void ux_handler();
extern struct cpusw *cpup;
extern int vm_managed_pages;

#if	NCPUS > 1
#include <kern/processor.h>
#endif

#if     MACH_KM
#include <kern/kern_mon.h>
#endif 

long	cp_time[CPUSTATES];
int	dk_ndrive;
int	dk_busy;
long	dk_time[DK_NDRIVE];
long	dk_seek[DK_NDRIVE];
long	dk_xfer[DK_NDRIVE];
long	dk_wds[DK_NDRIVE];
long	dk_wpms[DK_NDRIVE];

long	tk_nin;
long	tk_nout;

dev_t	rootdev;		/* device of the root */
struct vnode *rootvp;		/* vnode of root filesystem */
dev_t	dumpdev;		/* device to take dumps on */
daddr_t	dumplo;			/* offset into dumpdev */
int	show_space;
int	hostid;
char	hostname[MAXHOSTNAMELEN];
int	hostnamelen;
char	domainname[MAXDOMNAMELEN];
int	domainnamelen;
int	networkboot = 0;

struct	timeval boottime;
struct	timeval time;
struct	timezone tz;			/* XXX */
int	hz;
int	phz;				/* alternate clock's frequency */
int	tick;
time_t	lbolt;				/* awoken once a second */

thread_t idle_thread_ptrs[NCPUS];	/* used for debugging crash dumps */
thread_t vm_pageout_thread_ptr, reaper_thread_ptr;	/* use these with */
thread_t swapin_thread_ptr, swapout_thread_ptr;		/* the dbx command */
thread_t action_thread_ptr, sched_thread_ptr;		/* "thread set xxx" */
thread_t acctwatch_thread_ptr;

int	cmask = CMASK;
extern vm_offset_t (*mountroot)();

/*
 *	Defaults for all processes
 */
extern int maxuprc;		/* default maximum proccesses per user */
#if	UNIX_LOCKS
#include <sys/time.h>
#include <kern/lock.h>
#include <sys/file.h>
#include <sys/select.h>
#endif

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *	     - process 2 to page out
 */
task_t		first_task;
thread_t	first_thread;

thread_t setup_main()
/*
 *	first_addr contains the first available physical address
 *	running in virtual memory on the interrupt stack
 *
 *	returns initial thread to run
 */
{
	extern vm_offset_t	virtual_avail;
	vm_offset_t		end_stack, cur_stack;
	int			i;
	extern void	initial_context();
	extern void	vm_mem_init();
	extern void	kdebug_vtop_init();
	extern void	sigqueue_init();
#if	MACH_NET
	extern void	mach_net_init();
#endif
	extern char	version_version[], version_release[];
	static struct ucred first_creds;

	rqinit();
	sched_init();
#ifndef __alpha
	vm_mem_init(); /* Alpha called this from ../arch/alpha/alpha_init.c */
	kdebug_vtop_init();
#endif /* __alpha */
        kmeminit();                     /* BSD malloc */
	init_timers();
	lwc_init();

#if	XPR_DEBUG
	xprbootstrap();
#endif

	startup(virtual_avail);

	machine_info.max_cpus = NCPUS;
	machine_info.memory_size = mem_size;
	machine_info.avail_cpus = 0;
	machine_info.major_version = KERNEL_MAJOR_VERSION;
	machine_info.minor_version = KERNEL_MINOR_VERSION;

#ifdef __alpha
	/* this will need to be reworked later for NCPUS > 1 */
#else
	/*
	 *	Create stacks for other processors (the first
	 *	processor up uses a preallocated stack).
	 */

	cur_stack = kmem_alloc(kernel_map, NCPUS*INTSTACK_SIZE);
	end_stack = cur_stack + round_page(NCPUS*INTSTACK_SIZE);
	for (i = 0; i < NCPUS; i++) {
		if (machine_slot[i].is_cpu) {
			if (i != master_cpu) {
				interrupt_stack[i] = cur_stack;
				cur_stack += INTSTACK_SIZE;
			}
			else {
				/*
				 * Master cpu uses system intstack,
				 */
#ifndef	ibmrt
				interrupt_stack[i] = (vm_offset_t) &intstack[0];
#endif
			}
		}
		else {
			interrupt_stack[i] = (vm_offset_t) 0;
		}
	}

	/*
	 *	Free up any stacks we really didn't need.
	 */

	cur_stack = round_page(cur_stack);
	if (end_stack != cur_stack)
		kmem_free(kernel_map, cur_stack, end_stack - cur_stack);
#endif /* __alpha */

	/*
	 *	Initialize the task and thread subsystems.
	 */

	/*
	 * This is a convenient place to do this.  This
	 * keeps us from including user.h in thread.c
	 */
	{
		extern int threadmax;
		extern struct zone *u_zone;
		u_zone = zinit(sizeof(struct utask),
			threadmax * sizeof(struct utask),
			10 * sizeof(struct utask),
			"u-areas");
	}
	ipc_bootstrap();
#if     MACH_KM
        /*
	 *  Initialize some kernel monitoring structures.
	 *  This must be placed before cpu_up() because
	 *  the all_monitor's queue is touched there.
	 */
	monitor_init();
#endif

	cpu_up(master_cpu);	/* signify we are up */
#if	MACH_NET
	mach_net_init();
#endif
	sigqueue_init();
	task_init();
	thread_init();
	thread_swapper_init();
	task_swapper_init();
	ipc_init();
	kern_prot_init();
#if	MACH_HOST
	pset_sys_init();
#endif

	/*
	 *	initialize static version info in utsname struct
	 */

	bcopy(version_version, utsname.version, sizeof(utsname.version));
        utsname.version[sizeof(utsname.version)-1] = '\0';
        bcopy(version_release, utsname.release, sizeof(utsname.release));
        utsname.release[sizeof(utsname.release)-1] = '\0';

        
	/*
	 *  initialize the binary error logging
	 */
	if (binlog_init() < 0)
	      printf("Binary event logger disabled, initialization failure\n");


	/*
	 *	Create proc[0]'s u area.
	 */

	if (task_create(TASK_NULL, FALSE, &first_task) != KERN_SUCCESS)
	    panic("Can't create task for first process");
 
	bzero((caddr_t) first_task->u_address,
              (unsigned) sizeof(struct utask));

	first_task->proc_index = 0;
	first_task->ipc_privilege = TRUE;
	first_task->kernel_ipc_space = TRUE;
	first_task->kernel_vm_space = TRUE;

	/*
	 * The first thread and task need some credentials,
	 * appropriate pointers, miscellaneous locks initialized (sigh).
	 * N.B.  The first_creds structure has an arbitrarily high
	 * reference count to prevent it from being released back into
	 * the creds zone, from whence it never came.
	 */
	first_task->u_address->uu_procp = &proc[0];
	proc[0].p_rcred = &first_creds;
	first_creds.cr_ref = 100;
	first_creds.cr_uid = 0;
	first_creds.cr_gid = 0;
	first_creds.cr_ngroups = 1;
	first_creds.cr_groups[0] = 0;
/*
 * In SVR4 a credential structure is called a cred.  In OSF/1, it is
 * called a ucred.  A cred contains everything in a ucred as well as the
 * real and saved user and group ids.  According to the DDI/DKI Reference
 * Manual, a driver can examine all of the fields defined in a cred. In
 * order to support the DDI/DKI interfaces, it was necessary to add these
 * fields to the OSF/1 ucred. 
 *
 * In OSF/1, the real and saved  user and group ids were stored only in
 * the proc structure. Code was added to make sure that the extended 
 * ucred contains the real and saved user and group ids.
 */

	first_creds._cr_ruid = 0;
	first_creds._cr_rgid = 0;
	first_creds._cr_suid = 0;
	first_creds._cr_sgid = 0;

	CR_LOCK_INIT(&first_creds);
	PROC_LOCK_INIT(&proc[0]);

	if (thread_create(first_task, &first_thread) != KERN_SUCCESS) {
		panic("setup_main: can't create the first thread");
	}
	initial_context(first_thread);
	proc[0].task = first_task;
	proc[0].thread = first_thread;
	proc[0].utask  = first_task->u_address;
	first_thread->state = TH_RUN;
	first_thread->user_stop_count = 0;
	first_thread->suspend_count = 0;
	first_thread->ipc_kernel = TRUE;
	(void) thread_resume(first_thread);

	/*
	 *	Tell the pmap system that our cpu is using the kernel pmap
	 */
	PMAP_ACTIVATE(kernel_pmap, first_thread, cpu_number());

	/*
	 *	Return to assembly code to start the first process.
	 */

	return(first_thread);
}

/*
 *	Sets the name for the given task.
 */
void task_name(s)
	char		*s;
{
	int		length = strlen(s);

	bcopy(s, u.u_comm,
		length >= sizeof(u.u_comm) ? sizeof(u.u_comm) :
			length + 1);
}

/* To allow these values to be patched, they're globals here */
/* Note: moved to param.c (set from the kernel config file) */
extern struct rlimit vm_initial_limit_stack;
extern struct rlimit vm_initial_limit_data;
extern struct rlimit vm_initial_limit_core;
extern struct rlimit vm_initial_limit_rss;
extern struct rlimit vm_initial_limit_vas;

main()
{
	register int i;
	register struct proc *p;
	register struct pgrp *pg;
	int s, ret;
	port_t		dummy_port;
	thread_t	th;
	extern 	int	numcpus;
	extern 	int	open_max_hard, open_max_soft;
	extern void	idle_thread(), init_task(), vm_pageout();
	extern void	reaper_thread(), swapin_thread(), swapout_thread();
	extern void	task_swapper_thread_loop();
	extern void	sched_thread();
	extern void	file_table_init();
  	extern void	scsiisr_init();
  	extern void	dsaisr_init();
	extern void     kd_thread_start();
#if	NCPUS > 1
	extern void	action_thread();
#endif
#if	UNIX_LOCKS
#ifdef	multimax
	extern void	slcintr_thread();
#endif
	extern void	psignal_thread();
#endif
	extern void	acctwatch_thread();
	extern thread_t	newproc();
	thread_t	exc_th;
	void		global_lock_initialization();
	extern	void	realtime_init();
#ifdef	multimax
	void		mmax_initialization();
#endif
	/*
	 * set up system process 0 (swapper)
	 */
	p = &proc[0];
	/*
	 *	Now in thread context, switch to thread timer.
	 */
	s = splhigh();
	timer_switch(&current_thread()->system_timer);
	splx(s);
	p->p_stat = SRUN;
	p->p_flag |= SLOAD|SSYS;
	p->p_nice = PRIZERO;
	simple_lock_init(&p->siglock);
	p->sigwait = FALSE;
	p->exit_thread = THREAD_NULL;
	p->p_auid = AUID_INVAL;
	u.u_procp = p;
	uarea_init(current_thread());
	uarea_lock_init(current_thread()->u_address.utask);
#if	defined(vax) || defined(ns32000)
	/*
	 * These assume that the u. area is always mapped 
	 * to the same virtual address. Otherwise must be
	 * handled when copying the u. area in newproc().
	 */
	ndinit(&u.u_nd, 0);
#endif
	u.u_cmask = cmask;
	/*
	 * No need for the file descriptor overflow table;
	 * set to NULL
	 */
	u.u_file_state.uf_ofile_of = NULL;
	u.u_file_state.uf_pofile_of = NULL;
	u.u_file_state.uf_of_count = 0;
	u.u_lastfile = -1;

	for (i = 0; i < sizeof(u.u_rlimit)/sizeof(u.u_rlimit[0]); i++)
		u.u_rlimit[i].rlim_cur = u.u_rlimit[i].rlim_max = 
		    RLIM_INFINITY;
	u.u_maxuprc = maxuprc;

	u.u_rlimit[RLIMIT_STACK] = vm_initial_limit_stack;
	u.u_rlimit[RLIMIT_DATA] = vm_initial_limit_data;
	u.u_rlimit[RLIMIT_CORE] = vm_initial_limit_core;
	u.u_rlimit[RLIMIT_RSS].rlim_cur = ((vm_initial_limit_rss.rlim_cur*vm_managed_pages)/100)*NBPG;
	u.u_rlimit[RLIMIT_RSS].rlim_max = ((vm_initial_limit_rss.rlim_max*vm_managed_pages)/100)*NBPG;
	u.u_rlimit[RLIMIT_AS] = vm_initial_limit_vas;

	/*
	 * Sanity check max number of file descriptors
	 * a process can open.
	 */
	if (open_max_hard < NOFILE_IN_U)
		open_max_hard = NOFILE_IN_U;
	else if (open_max_hard > OPEN_MAX_SYSTEM)
		open_max_hard = OPEN_MAX_SYSTEM;
	if (open_max_soft < NOFILE_IN_U)
		open_max_soft = NOFILE_IN_U;
	else if (open_max_soft > open_max_hard)
		open_max_soft = open_max_hard;
	u.u_rlimit[RLIMIT_NOFILE].rlim_cur = open_max_soft;
	u.u_rlimit[RLIMIT_NOFILE].rlim_max = open_max_hard;

 	/*
	 *	Allocate a kernel submap for pageable memory
	 *	for temporary copying (table(), execve()).
	 * 	Also a submap for Mach copyin/copyout technology.
	 */
	{
	    vm_offset_t	min, max;

	    kernel_pageable_map = kmem_suballoc(kernel_map, &min, &max,
				(numcpus+4)*round_page(NCARGS), TRUE);
		kernel_pageable_map->vm_wait_for_space = TRUE;

		kernel_copy_map = kmem_csuballoc(kernel_map, &min, &max,
				vm_tune_value(csubmapsize));
		kernel_copy_map->vm_wait_for_space = TRUE;
	}
	kallocinit();
  	scsiisr_init();
	kd_thread_start();

	/*
	 * Initialize device switch tables.
	 */
	devsw_init();

        /*
         * Get vnodes for swapdev, argdev, and rootdev.
	 *
	 *
	 * We initialize the nfs hooks in case vfsinit calls nfs_init.
         */
	nfs_hooks_init();
	vfsinit();
	if (bdevvp(rootdev, &rootvp))
		panic("can't setup bdevvp of rootdev");

#if	SEC_BASE
	sec_init();
#endif
        /*
         * Setup credentials
         */
        p->p_rcred = crget();
        p->p_rcred->cr_ngroups = 1;

	/*
	 * set up process group and session structures
	 */

	pgrphash[0] = (struct pgrp *)kalloc(sizeof (struct pgrp));
	if ((pg = pgrphash[0]) == NULL)
		panic("no space to craft zero'th process group");
        PGRP_LOCK_INIT(pg);
        PGRP_REFCNT_LOCK_INIT(pg);
	pg->pg_id = 0;
	pg->pg_hforw = (struct pgrp *)NULL;
	pg->pg_mem = p;
	pg->pg_jobc = 0;
	pg->pg_sessnxt = (struct pgrp *)NULL;
	p->p_pgrp = pg;
	p->p_pgrpnxt = (struct proc *)NULL;
        {
        struct session *session;
        session = (struct session *)kalloc(sizeof (struct session));
        if (session == NULL)
                panic("no space to craft zero'th session");
        SESS_LOCK_INIT(session);
        SESS_FPGRP_LOCK_INIT(session);
	session->s_count = 1;
	session->s_leader = (struct proc *)NULL;
	session->s_id = 0;
	session->s_ttyvp = (struct vnode *)NULL;
        session->s_pgrps = pg;
        session->s_fpgrpp = (struct pgrp **)NULL;
	pg->pg_session = session;
	}

	softclock_init();	/* bsd/kern_clock.c */
	startrtclock();
#ifdef	vax
#include <kg.h>
#if	NKG > 0
	startkgclock();
#endif
#endif	/* vax */

        /*
         * Start BSD malloc service thread.
         */
        kmeminit_thread(0);

	/*
	 * Initialize tables, protocols, and set up well-known inodes.
	 */
#if	BSD_TTY
	cinit();
#endif
	netinit();
#if	STREAMS
	pse_init();
#endif	

	pqinit();
	file_table_init();
	global_lock_initialization();
#if	multimax
	mmax_initialization();
#endif
	/*
	 *	Create kernel idle cpu processes.  This must be done
 	 *	before a context switch can occur (and hence I/O can
	 *	happen in the bio_init() call).
	 */
	u.u_rdir = NULL;
	u.u_cdir = NULL;

	for (i = 0; i < NCPUS; i++) {
		if (machine_slot[i].is_cpu == FALSE)
			continue;
		if (thread_create(first_task, &th) != KERN_SUCCESS) {
			panic("main: can't create idle thread");
		}
		thread_bind(th, cpu_to_processor(i));
		thread_start(th, idle_thread, THREAD_SYSTEMMODE);
		(void) thread_resume(th);
		idle_thread_ptrs[i] = th;
	}
	bio_init();
#if	UNIX_LOCKS && defined(multimax)
	/* this thread must be started early in the game */
	(void) kernel_thread (first_task, slcintr_thread);
#endif
#ifdef	PROFILING
	kmstartup();
#endif

#if	SEC_ARCH
	spdinit();
#endif

#if	defined(sun3) || defined(sun4)
	consconfig();	/* configure Sun console */
#endif

#if _LMF_
        /*
         * Initialize SMM (System Marketing Model) for LMF module 
         * This goes here to ensure that the clock is running on a DS 3100
         */
	set_lmf_smm();
#endif /*_LMF_*/
/* kick off timeout driven events by calling first time */
	recompute_priorities();
 	schedcpu();

/* wait for DSA disks to configure if possible on CPU */
	if (cpup->flags & MSCP_POLL_WAIT)
		mscp_poll_wait(5);

	if (do_mscp_poll_wait)
		mscp_poll_wait(5);

  	dsaisr_init();

	/*
	 * Grab all the boot messages from the syslog buffer and put them in
	 * the binary error log in a single record.  At this point all the
	 * messages from the system booting are the only things there, so grab
	 * everything and log a 'startup' class event record.
	 */
	log_startup();

	netbootchk();

	/*
	 * Mount the root filesystem
	 */
	vfs_mountroot();

	boottime = time;
	first_task->u_address->uu_start = boottime;

	/* SystemV ipc */
	msginit();
	seminit();
	shminit();

	/*
	 * Call REALTIME initialization package.
	 */
	realtime_init();

#if	MACH_EMULATION
	eml_init();
#endif
	/*
	 * Init record/file locking
	 */
	flckinit();

	/*
	 * make init process
	 */

	siginit(&proc[0]);

#if	XPR_DEBUG
	xprinit();
#endif

	/*
	 * Initialize the swap I/O subsystem and the ubc.
	 */

	vm_swap_init();
	ubc_init();

	/*
	 * Initial task.  Do it here so that pid will be 1.
	 */
	th = newproc(0);

#if	MACH_DEVICE
	{ /*device_server*/
	thread_t	device_io_th;
	extern void	device_loop();
	extern void	device_init();

	device_io_th = newproc(0);
	device_io_th->ipc_kernel = TRUE;
	device_io_th->task->kernel_vm_space = TRUE;
	device_io_th->task->ipc_privilege = TRUE;
	device_init(device_io_th->task);
	thread_swappable(device_io_th, FALSE);
	thread_start(device_io_th, device_loop, THREAD_SYSTEMMODE);
	(void) thread_resume(device_io_th);	
	/*device_server*/ }
#endif

#if BIN_COMPAT
	/*
	 *	Initialize the compatability module support
	 */
	cm_init();
#endif /*BIN_COMPAT*/

#if _LMF_
        /*
         * Initialize lock for license cache of LMF module 
         */
        lmf_init();
#endif /*_LMF_*/

	/*
	 *	Default exception server
	 */

	simple_lock_init(&ux_handler_init_lock);
	ux_exception_port = PORT_NULL;
	exc_th = newproc(0);
	exc_th->task->kernel_vm_space = TRUE;
	thread_start(exc_th, ux_handler, THREAD_SYSTEMMODE);
	(void) thread_resume(exc_th);

	/*
	 * Setup exception port for init.  This will be inherited by
	 * children of init (all processes).
	 */
	simple_lock(&ux_handler_init_lock);
	if (ux_exception_port == PORT_NULL) 
		thread_sleep((vm_offset_t)&ux_exception_port,
			simple_lock_addr(ux_handler_init_lock), FALSE);
	else
		simple_unlock(&ux_handler_init_lock);
	(void) task_set_exception_port(th->task, ux_exception_port);
	port_reference((kern_port_t) ux_exception_port);
	object_copyout(th->task, (kern_obj_t) ux_exception_port,
		       MSG_TYPE_PORT, &dummy_port);

	/*
	 *	After calling start_init,
	 *	machine-dependent code must
	 *	set up stack as though a system
	 *	call trap occurred, then call
	 *	load_init_program.
	 */

	thread_start(th, init_task, THREAD_SYSTEMMODE);
	(void) thread_resume(th);

	/*
	 *	task swapper thread
	 */
	{
	thread_t	task_swapper_thread;

        if (thread_create(first_task, &task_swapper_thread) != KERN_SUCCESS)
                panic("setup_main: can't create the task_swapper_thread");
	task_swapper_thread->ipc_kernel = TRUE;
        task_swapper_thread->max_priority = BASEPRI_HIGHEST;
        task_swapper_thread->priority = BASEPRI_SYSTEM;
        task_swapper_thread->sched_pri = BASEPRI_SYSTEM;
	task_swapper_thread->vm_privilege = TRUE;
        thread_swappable(task_swapper_thread, FALSE);
        thread_start(task_swapper_thread, task_swapper_thread_loop, THREAD_SYSTEMMODE);
        (void) thread_resume(task_swapper_thread);
	}

	/*
	 *	Kernel daemon threads that don't need their own tasks
	 */

	vm_pageout_thread_ptr = kernel_thread(first_task, vm_pageout);
	reaper_thread_ptr = kernel_thread(first_task, reaper_thread);
	swapin_thread_ptr = kernel_thread(first_task, swapin_thread);
	swapout_thread_ptr = kernel_thread(first_task, swapout_thread);
#if	NCPUS > 1
	action_thread_ptr = kernel_thread(first_task, action_thread);
#endif
	sched_thread_ptr = kernel_thread(first_task, sched_thread);
#if	UNIX_LOCKS
	(void) kernel_thread(first_task, psignal_thread);
#endif
	acctwatch_thread_ptr = kernel_thread(first_task, acctwatch_thread);
	/*
	 * initialize ISR threads (to support non-parallelized device drivers)
	 */
#ifdef	multimax
	isr_env_init();
#if	NMST
#if	!MSTOPT_DYNAMIC
	/*
	 * Simulate dynamic loading of tape driver by calling
	 * its configure function like kmodcall would.  Note
	 * this must be done, when using current driver, or
	 * driver will *not* be usable, even through static
	 * devsw entry.
	 */
	mst_callconfigure();
#endif
#endif
#endif
	u.u_procp->p_flag |= SLOAD|SSYS;
	task_name("kernel idle");
	(void) thread_terminate(current_thread());
	thread_halt_self();
	/*NOTREACHED*/
}

void init_task()
{
	task_name("init");
#ifdef i386
	proc[1].cxenix = NULL;
#endif
	start_init();
}

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
cinit()
{
	register vm_offset_t ccp;
	register struct cblock *cp;

	ccp = (vm_offset_t)cfree;
	ccp = (ccp+CROUND) & ~CROUND;
	for(cp=(struct cblock *)ccp; cp < &cfree[nclist-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
		cfreecount += CBSIZE;
	}
}

void
global_lock_initialization()
{
	extern struct slock     accounting_lock;
	udecl_simple_lock_data(extern,cblock_freelist_lock)
	decl_simple_lock_data(extern,atomic_op_lock)
	decl_simple_lock_data(extern,consvp_lock)
	extern void             log_init();
	extern void		profil_lock_init();

#if UERF       /* binary error log support */
	decl_simple_lock_data(extern,lk_errlog)
#endif /* UERF */

#if	SER_COMPAT
	/* from ../bsd */
	static lock_data_t	default_uni_lock;
#endif
#if	MACH_LDEBUG
	extern int		check_locks;
	int	i;
#endif
#if	RT_PREEMPT
	extern int		rt_preempt_opt;
	extern int		rt_preempt_enabled;
	int	i;
#endif

	/* from ../bsd */
	TIME_LOCK_INIT();
	kern_prot_init();
	pty_initialization();
	log_init();
	usimple_lock_init(&accounting_lock);
#if	0
	lock_init2(&mount_lock, TRUE, LTYPE_MOUNT_TABLE);
#endif
	usimple_lock_init(&cblock_freelist_lock);
	lock_init2(&hostname_lock, TRUE, LTYPE_HOSTNAME);

#if UERF       /* binary error log support */
	simple_lock_init(&lk_errlog);           /* subr_log.c */
#endif  /* UERF */

	simple_lock_init(&atomic_op_lock);
	simple_lock_init(&consvp_lock);
	select_init();
#if	SER_COMPAT
	lock_init2(&default_uni_lock, TRUE, LTYPE_DEFAULT_UNI);
#endif

#if	MACH_LDEBUG
	check_locks = 1;
	check_lock_counts = 0;
	for (i = 0; i < NCPUS; i++) {
		slck_dbg[i].count = 0;
		bzero(&slck_dbg[i].addr[0], sizeof(int)*MAX_LOCK);
	}
#elif	RT_PREEMPT
	if (rt_preempt_opt) {
	    rt_preempt_enabled = 1;
	    for (i = 0; i < NCPUS; i++) {
		slock_count[i] = 0;
	    }
	}
#endif
	audlock_init();
	
	profil_lock_init();
}

/******************************************************************************
 * REALTIME INITIALIZATION: global data and initialization routines
 * for system startup.
 * The routines and global data should be found in the realtime module
 * itself, e.g., POSIX.4 timer data and initialization is found in
 * module bsd/kern_time.c
 *****************************************************************************/

/*
 * REALTIME global data/declarations
 */
extern void aio_sysinit();
#if	RT_TIMER
extern void rttimer_init();
#endif
#if	RT_SEM
extern void rtbsem_init();
#endif

/*
 * REALTIME initialization: realtime_init()
 */
void
realtime_init()
{
	aio_sysinit();
#if	RT_TIMER
	rttimer_init();
#endif
#if     RT_SEM
        rtbsem_init();
#endif

}

#if	multimax
static void
mmax_initialization()
{
	extern void	ms_cmdinit();

        extern struct buf       sccbuf; /* XXX */
	extern struct slock     console_lock; /* XXX */
	extern lock_data_t init_bdl_lock;

	ms_cmdinit();
        lock_init2(&sccbuf.b_lock, TRUE, LTYPE_BUF);
        event_clear(&sccbuf.b_iocomplete);
        usimple_lock_init(&console_lock);
        simple_lock_init(&init_bdl_lock);

}
#endif
