
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
static char	*sccsid = "@(#)$RCSfile: startup.c,v $ $Revision: 1.2.3.6 $ (DEC) $Date: 1992/07/08 08:48:02 $";
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
 * derived from startup.c	4.10	(ULTRIX)	11/15/90";
 */


/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Modification History: startup.c
 *
 * 27-Jun-91	Scott Cranston
 *	Changed optional compilation of binary error logging support strategy.
 *	  - added #include <uerf.h>
 *        - changed instances of '#ifdef _UERF' to '#if UERF'
 *
 * 5-Jun-91	Scott Cranston
 *      Added binary error logging support:
 *         - turned on 'appendflg'
 *         - initialized zone memory for ealloc() to use.
 *
 * 31-May-91 -- map (Mark Parenti)
 *	Add call to getbootctlr() to initialize arrays needed by 
 *	getsysinfo.
 *
 * 05-Oct-90 -- Burns
 *	Moved to OSF/1.
 *
 * 15-Feb-90 -- Robin
 *	Made a bug fix to allow the DS_5500 to size memory via the
 *	bitmap (same as the 5800).
 *
 * 19-Dec-89 -- Fred Canter
 *	Bug fix to last delta. Need 2 PTEs per buffer header not one.
 *
 * 18-Dec-89 -- burns
 *	Fixed check of nbuf against sysptsize to exclude the MAXMEM
 *	fudge factor which is incorrect for mips based systems.
 *
 * 12-Dec-89 -- burns
 *	Moved out sysptsize to param.c since it is now calculated
 *	based upon physmem from the config file. It needs to be
 *	set in the targetbuild directory, not in a binary file.
 *
 * 11-Nov-89 -- Randall Brown
 *	Moved when we call cninit() to after mapinit(), so that cninit() can
 *	do BADDADDR if it needs to.  Changed msize_bitmap to ignore the first
 *	2 long words if we are on a 3max.
 *
 * 10-Nov-89 -- jaw
 *	remove references to maxcpu.
 *
 * 16-Oct-89 -- Alan Frechette
 *	No longer dump out the buffer cache during a system crash
 *	for partial selective crash dumps. Save the PFN of the last 
 *	kernel page to dump out for the crashdump code. Readjusted
 *	the order of allocation so that the buffer cache is allocated
 *	last. The buffer cache must be the last allocated data for 
 *	this to work. 
 *
 * 13-Oct-89  -- gmm
 *	smp support
 *
 * 10-July-89 -- burns
 *	Added memory sizing via bitmap, and added afd's separate cpu
 *	switch troutines for sizing memory via badaddr probes or bitmap.
 *
 * 30-Jun-89 -- afd
 *	Call memsize routine (to size and clear memory) thru the cpu switch.
 *	Added 2 new memory sizing rotines: msize_baddr and msize_bitmap.
 *
 * 15-Jun-89 -- afd
 *	Call cpu_initialize() routine as soon as "cpup" is set up in startup().
 *  
 * 14-Jun-89 -- darrell
 *	Removed the splhigh in startup, as spl() cannot be called yet,
 *
 * 14 Jun 89 -- chet
 *	Make buffer header allocations based on maxcpu (uniprocessor
 *	or multiprocessor).
 *
 * 01-Jun-89 -- Kong
 *	Added a field in the "save_state" struct to store the value
 *	of "cpu".  This is to allow the ULTRIX installation program
 *	"finder" to determine the type of machine it is running on
 *	without doing a name list on the kernel.  "save_state" is
 *	initialised by "init_restartblk".
 *
 * 08-Jun-89 -- gmm
 *	Allocate (KMALLOC) the idle stack for boot cpu during startup
 *
 * 02-May-89 -- jaw, jmartin
 *	fix forkutl to work on mips.
 *
 * 20-May-89 -- Randall Brown
 *	added call to cninit(), so that the cons_swp is setup correctly.
 *	printf's can not be done before this call is executed.
 *
 * 17-Apr-89 -- afd
 *	Created "save_state" struct at the bottom of the ULTRIX startup stack.
 *	This struct contains a magic number and the address of "doadump".
 *	This allows people to force a memory dump from console mode when
 *	a system is hung.
 *
 * 07-Apr-89 -- afd
 *	Updated system_type routine.
 *	Don't need "system.h"; cpu & systype info is in cpuconf.h
 *	Added "cpuswitch_entry" routine to initialize the pointer into
 *	    the cpusw table (cpup).
 *
 * 07-Apr-89 -- kong
 *	Added global variable "qmapbase" which gets initialised to
 *	the physical address of the Qbus map registers.  Changed
 *	routine "mapinit" so that on a kn210, the last 32K bytes
 *	of physical memory is not used by the kernel, but is used
 *	by the Qbus as map registers.  Note that the Qbus map must
 *	sit on a 32Kb boundary.
 *
 * 24-Mar-89 -- burns
 *	Added ISIS specifics.
 *
 * 23-Mar-89 -- Kong
 *	Added variable "sysptsize" and initialize it to SYSPTSIZE.
 *	Some of the VAX i/o device drivers need it.
 *
 * 23-Feb-89 -- Kong
 *	Added variable "Physmem" so that the "sizer" program knows
 * 	the actual size of physical memory.
 *
 * 12-Jan-89 -- Kong
 *     .Added variable "cpu" whose value is defined in system.h.
 *	Determine on what we are running and properly initialize "cpu".
 *     .Based on the system type, initialize all the interrupt handler
 *	entry points in c0vec_tbl (in trap.c).  This way each different
 *	system can use different handlers.
 *     .Initialized the interrupt masks based on the type of system
 *	we're on.  This allows the different interrupt schemes of the
 *	different systems to use the same splxxx routines.
 *
 * 10 Nov 88 -- chet
 *	Add configurable buffer cache.
 *
 * 09-Nov-88 -- afd
 *	Get the "systype" word from the boot PROM, and save it in
 *	"cpu_systype".
 *	This is done in startup because it must be done before the first
 *	autoconfig printf so that the error log packet with config messages
 *	will have the systype in the header.
 *
 * 09-Sep-88 -- afd
 *	Cleanup startup msgs; no configuration printfs until after kmeminit(),
 *	  because the kernel printf routine does a km_alloc.
 *	Set & clear appendflg so that startup msgs get logged together.
 *	Set "printstate" to what level of printing is possible as the system
 *	  comes up.
 *	Call chk_cpe() after configuration is done (records cache parity errs).
 *
 */

#include <confdep.h>
#include <mach_kdb.h>
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif /* SEC_BASE */

#include <machine/reg.h>
#include <machine/cpu.h>
#include <hal/cpuconf.h>

#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/specdev.h>
#include <ufs/inode.h>
#include <sys/mount.h>
#include <ufs/ufsmount.h>
#include <sys/file.h>
#include <sys/clist.h>
#include <sys/callout.h>
#include <vm/vm_kern.h>
#if PROFILING
#include <sys/gprof.h>
#endif /* PROFILING */
#include <hal/entrypt.h>

#include <rpc/types.h>
#include <nfs/nfs.h>
#include <nfs/rnode.h>

#undef kmem_alloc
#undef kmem_free
#undef mem_alloc
#undef mem_free


extern struct cpusw *cpup;
extern int numci, nummsi, do_mscp_poll_wait;

int do_virtual_tables = 0;	/* select how to allocate U*x tables */
/*
 * Declare these as initialized data so we can patch them.
 */
#ifdef	NBUF
int	nbuf = NBUF;
#else
int	nbuf = 0;
#endif
#ifdef	BUFPAGES
int	bufpages = BUFPAGES;
#else
int	bufpages = 0;
#endif
extern	unsigned int	bufcache;

vm_map_t	buffer_map;

extern int	cpu;
u_int printstate = PROMPRINT;		/* what level of printf is available */



/*
 * Since the cache on the MipsCo. machines does not snoop the bus,
 * it is necessary to take care of DMA devices appropriately.
 * The simplest way to do it is by making the memory where we dma into
 * non-cachable.  Hence, the virtual range used for the buffers
 * is clearly delimited so that the pmap module can recognize it.
 *
 * Not nice, really, but flushing the cache on each disk transfer
 * does not make much more sense, does it ?
 */
vm_offset_t buffers_end;

/*
 * Machine-dependent startup code.
 */
startup()
{
	extern int      icache_size, dcache_size;
	vm_size_t       size;
	vm_offset_t     tmp_addr;
	kern_return_t   ret;
	register unsigned int i;
	int             base, residual;
	extern VEC_nofault(), VEC_trap(); /* OHMS hack */
	extern  (*causevec[])(); /* OHMS hack */
	vm_object_t object;


	/*
	 * Initialization message print.
	 */
	printf(version);
	printf("cache sizes: %d inst %d data\n", icache_size, dcache_size);
#define MEG	(1024*1024)
	printf("physical memory = %d bytes (%d.%d%d MB).\n", mem_size, mem_size / MEG,
	       ((mem_size % MEG) * 10) / MEG,
	       ((mem_size % (MEG / 10)) * 100) / MEG);

	/*
	 * If not done already, allocate the various U*x tables. Note that we
	 * do it here only if we want them virtual.
	 */

	if (do_virtual_tables)
		allocate_unix_tables((vm_offset_t *) 0, TRUE);

	/*
	 * Now allocate buffers proper.  They are different than the above in
	 * that they usually occupy more virtual memory than physical.
	 */

	base = bufpages / nbuf;
	residual = bufpages % nbuf;

	/*
	 * Allocate virtual memory for buffer pool.
	 */
	size = round_page((vm_size_t) (MAXBSIZE * nbuf));
	buffer_map = kmem_suballoc(kernel_map, &tmp_addr, &buffers_end,
				   size, TRUE);
	buffers = (char *) tmp_addr;
	vm_object_allocate(OT_NULL, 
		vm_map_max(buffer_map) - vm_map_min(buffer_map), 
		(caddr_t) 0, &object);

	k_map_allocate_va(buffer_map, object, &tmp_addr, size, TRUE);

	for (i = 0; i < nbuf; i++) {
		vm_size_t       thisbsize;
		vm_offset_t     curbuf;

		/*
		 * First <residual> buffers get (base+1) physical pages
		 * allocated for them.  The rest get (base) physical pages.
		 *
		 * The rest of each buffer occupies virtual space, but has no
		 * physical memory allocated for it.
		 */

		thisbsize = PAGE_SIZE * (i < residual ? base + 1 : base);
		curbuf = (vm_offset_t) buffers + i * MAXBSIZE;
		k_mem_allocate(buffer_map, curbuf, thisbsize);
	}


	/*
	 * Initialize callouts
	 */
	callfree = callout;
	for (i = 1; i < ncallout; i++)
		callout[i - 1].c_next = &callout[i];

	/*
	 * Tell 'em how much it's left
	 */
	{
		register int    nbytes;
		extern int      vm_page_free_count;

		nbytes = ptoa(vm_page_free_count);
		printf("available memory = %d bytes (%d.%d%d MB).\n", nbytes, nbytes / MEG,
		       ((nbytes % MEG) * 10) / MEG,
		       ((nbytes % (MEG / 10)) * 100) / MEG);
		nbytes = ptoa(bufpages);
		printf("using %d buffers containing %d bytes (%d.%d%d MB) of memory\n",
		       nbuf,
		       nbytes,
		       nbytes / MEG,
		       ((nbytes % MEG) * 10) / MEG,
		       ((nbytes % (MEG / 10)) * 100) / MEG);
	}




	causevec[EXC_DBE>>CAUSE_EXCSHIFT] = VEC_nofault;

	/* Start system sysaps (system applications - MSCP/TMSCP/etc.) */
	if (cpup->flags & SCS_START_SYSAPS) 
 		scs_start_sysaps();
	
	configure();		/* auto config.  */
	if (cpup->flags & MSCP_POLL_WAIT && ( numci || nummsi )) {
		do_mscp_poll_wait++;
	}
	getbootctlr();		/* initialize boot controller and consmagic */
	
 	printf("");

/* TOOD: 	(*(cpup->timer_action)); */
	/*
	 * on return from configure machine is at spl0()
	 * Start logging of parity errors and initialize restart block
	 */
	causevec[EXC_DBE>>CAUSE_EXCSHIFT] = VEC_trap;

#if	MACH_KDB
	kdb_enable();
#else
	init_restartblk();
#endif
	chk_cpe();
}

init_restartblk()
{
	register struct restart_blk *rb = (struct restart_blk *)RESTART_ADDR;
/* TOOD: 	register struct save_state *sst = (struct save_state *)SST_ADDR; */
	register int i, sum;
	extern int doadump();

	rb->rb_magic = RESTART_MAGIC;
	rb->rb_occurred = 0;
	rb->rb_restart = doadump;

	sum = 0;
	for (i = 0; i < RESTART_CSUMCNT; i++)
		sum += ((int *)rb->rb_restart)[i];

	rb->rb_checksum = sum;
	/*
	 * Save state for ULTRIX
	 */
/* TODO: 	sst->sst_magic = SST_MAGIC; */
/* TODO: 	sst->sst_dump = doadump; */
/* TODO: 	sst->cpu = cpu; */
}


allocate_unix_tables( phys_start, virtually )
	vm_offset_t    *phys_start;	/* IN/OUT */
	int		virtually;	/* IN */
{
	caddr_t         base_addr;
	register caddr_t v;
	vm_size_t       size;
	vm_offset_t     tmp_addr;
	u_int		vnode_size;

	/*
	 * Allocate space for system data structures.
	 *
	 * We can do it two ways: physically or virtually.
	 *
	 * If the virtual memory
	 * system has already been set up, we cannot bypass it to allocate
	 * memory as the old code does.  We therefore make two passes over
	 * the table allocation code.  The first pass merely calculates the
	 * size needed for the various data structures.  The second pass
	 * allocates the memory and then sets the actual addresses.  The code
	 * must not change any of the allocated sizes between the two passes.
	 * If we want things allocated physically the same procedure is
	 * applied, only the way memory is allocated is different.
	 */
	base_addr = 0;
	for (;;) {
		v = base_addr;
#define	valloc(name, type, num) \
	    (name) = (type *)v; v = (caddr_t)((name)+(num))
#define	valloclim(name, type, num, lim) \
	    (name) = (type *)v; v = (caddr_t)((lim) = ((name)+(num)))

		vn_maxprivate = max(sizeof(struct inode),
				    sizeof(struct rnode));
		vnode_size = ((u_int)(sizeof(struct vnode) + vn_maxprivate -4));
		vnode = (struct vnode *)(v);
		vnodeNVNODE = (struct vnode *)
				((u_int) vnode + nvnode * vnode_size);
		(v) = (caddr_t)vnodeNVNODE;
	
		valloclim(file, struct file, nfile, fileNFILE);
		valloclim(proc, struct proc, nproc, procNPROC);
#if	SEC_BASE
		valloc(secinfo, struct security_info, nproc);
#endif
		valloc(cfree, struct cblock, nclist);
		valloc(callout, struct callout, ncallout);
		valloc(namecache, struct namecache, nchsize);
		/*
		 * The next few structures are hash chains that depend
		 * on being a power of two.  The roundup() function
		 * enforces that.
		 */
		nchsz = rndup(nchsz, MINNCHSZ);
		valloc(nchash, struct nchash, nchsz);
		bufhsz = rndup(bufhsz, MINBUFHSZ);
		valloc(bufhash, struct bufhd, bufhsz);
		spechsz = rndup(spechsz, MINSPECHSZ);
		valloc(speclisth, struct spechash, spechsz);
		inohsz = rndup(inohsz, MININOHSZ);
		valloc(ihead, struct ihead, inohsz);
		valloc(mounttab, struct ufsmount, nmount);

		/*
		 * Use as much memory as the config file asks for. Since these
		 * pages are virtual-size pages (larger than physical page
		 * size), use only one page per buffer.
		 */

		bufpages = atop((mem_size / 100) * bufcache);
		if (bufpages == 0)
			bufpages = atop(mem_size / 10);
		if (nbuf == 0) {
			if ((nbuf = bufpages) < 16)
				nbuf = 16;
		}
		if (bufpages > nbuf * (MAXBSIZE / PAGE_SIZE))
			bufpages = nbuf * (MAXBSIZE / PAGE_SIZE);

		valloc(buf, struct buf, nbuf);

		/*
		 * Clear space allocated thus far, and make r/w entries for
		 * the space in the kernel map.
		 */
		if (base_addr == 0) {
			/*
			 * Size has been calculated; allocate memory.
			 */
			size = (vm_size_t) (v - base_addr);
			if (virtually) {
				tmp_addr = kmem_alloc(kernel_map, round_page(size));
				if (tmp_addr == 0)
					panic("startup: no room for tables");
			} else {
				tmp_addr = PHYS_TO_K0(*phys_start);
				*phys_start += size;
			}
			base_addr = (caddr_t) tmp_addr;
		} else {
			/*
			 * Memory has been allocated.  Make sure that table
			 * size has not changed.
			 */
			if ((vm_size_t) (v - base_addr) != size)
				panic("startup: table size inconsistent");
			break;
		}
	}
}


