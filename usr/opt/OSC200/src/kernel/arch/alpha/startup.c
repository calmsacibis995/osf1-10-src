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
static char *rcsid = "@(#)$RCSfile: startup.c,v $ $Revision: 1.2.8.4 $ (DEC) $Date: 1993/10/07 17:29:30 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)startup.c	9.2	(ULTRIX/OSF)	10/29/91";
#endif	lint

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: ...msp/src/kernel/dec/machine/alpha/startup.c
 *
 * 26-Apr-91 -- afd
 *	Created this file for Alpha startup.
 */

#include <confdep.h>
#include <mach_kdb.h>
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif /* SEC_BASE */

#include <sys/param.h>
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
#include <machine/entrypt.h>

#include <rpc/types.h>
#include <nfs/nfs.h>
#include <nfs/rnode.h>

extern struct cpusw *cpup;
extern int numci, nummsi, do_mscp_poll_wait;

long do_virtual_tables = 1;	/* select how to allocate U*x tables */

#undef kmem_alloc
#undef kmem_free
#undef mem_alloc
#undef mem_free

/*
 * Declare these as initialized data so we can patch them.
 */
#ifdef	NBUF
long	nbuf = NBUF;
#else
long	nbuf = 0;
#endif
#ifdef	BUFPAGES
long	bufpages = BUFPAGES;
#else
long	bufpages = 0;
#endif
int	nswbuf = 0;	/* number of swap buffers */
int	bufdebug = 0;	/* buffer cache initialization messages */
int	clperbuf = 0;	/* one buf struct for every clperbuf page clusters */
			/* of cache */
int	pteperbuf = 2;	/* we need two 4K ptes for each 8K buf */


vm_map_t	buffer_map;

extern struct cpusw *cpup;	/* pointer to cpusw entry */
extern int cpu;
extern int appendflg;			/* for logging startup messages */
extern char version[];
extern int bufcache;

/*
 * Machine-dependent startup code.
 * We now have an initial u_area and kernel stack that we are running on
 * set up by remap_os.
 */

vm_offset_t buffers_end;

startup()
{
	vm_size_t       size;
	vm_offset_t     tmp_addr;
	kern_return_t   ret;
	register unsigned int i;
	int             base, residual;
	vm_object_t	object;

	/*
	 * Initialization message print.
	 */
	printf(version);
#define MEG	(1024*1024)
	printf("physical memory = %d.%d%d megabytes.\n", mem_size / MEG,
	       ((mem_size * 10) / MEG) % 10,
	       ((mem_size * 100) / MEG) % 10);

	/*
	 * Allocate the various U*x tables. Note that we
	 * do it here on Alpha because we want them virtual.
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
	buffer_map = kmem_suballoc(kernel_map,
				   &tmp_addr,	/* min */
				   &buffers_end,	/* max */
				   size,
				   TRUE);
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
	 * Print avail memory and size of buffer cache
	 */
	{
		register vm_size_t		nbytes;
		extern int			vm_page_free_count;
		extern struct scavenge_list	scavenge_info;

		nbytes = alpha_ptob(vm_page_free_count + scavenge_info.count);
		printf("available memory = %d.%d%d megabytes.\n", nbytes / MEG,
			((10 * nbytes) / MEG) % 10,
			((100 * nbytes) / MEG) % 10);
		nbytes = ptoa(bufpages);
		printf("using %d buffers containing %d.%d%d megabytes of memory\n",
		       nbuf,
			nbytes / MEG,
			((10 * nbytes) / MEG) % 10,
			((100 * nbytes) / MEG) % 10);
	}

	/*
	 * No thread context exists yet. Therefore we need to prevent
	 * badaddrs during configure from going thru trap.
	 */

	/*
	 * start any SYSAPS
	 */
	if (cpup->flags & SCS_START_SYSAPS)
 		scs_start_sysaps();

	/*
	 * Auto-config:
	 * On return from configure machine is at spl0()
	 */
	configure();
	if (cpup->flags & MSCP_POLL_WAIT && ( numci || nummsi )) {
		do_mscp_poll_wait++;
	}

#if	MACH_KDB
	kdb_enable();
#endif
	(*cpup->timer_action)();
}

allocate_unix_tables (phys_start, virtually) 
	vm_offset_t    *phys_start;	/* IN/OUT */
	long		virtually;	/* IN */
{
	vm_offset_t	base_addr;
	vm_offset_t	v;
	vm_size_t	size;
	vm_offset_t	tmp_addr;
	vm_size_t	vnode_size;

	/*
	 * Allocate space for system data structures.
	 *
	 * We can do it two ways: physically or virtually.
	 *
	 * If the virtual memory system has already been set up,
	 * we cannot bypass it to allocate memory as the old code does.
	 * We therefore make two passes over the table allocation code.
	 * The first pass merely calculates the size needed for the various
	 * data structures.  The second pass allocates the memory and then
	 * sets the actual addresses.  The code must not change any of the
	 * allocated sizes between the two passes.  If we want things
	 * allocated physically the same procedure is applied, only the
	 * way memory is allocated is different.
	 */
	base_addr = 0;
	for (;;) {
		v = base_addr;
#define	valloc(name, type, num) \
	    (name) = (type *)v; v = (vm_offset_t)((name)+(num))
#define	valloclim(name, type, num, lim) \
	    (name) = (type *)v; v = (vm_offset_t)((lim) = ((name)+(num)))

		vn_maxprivate = max(sizeof(struct inode),
				    sizeof(struct rnode));
		vnode_size = (vm_size_t)(((struct vnode *)0)->v_data +
								vn_maxprivate);
		vnode = (struct vnode *)(v);
		vnodeNVNODE = (struct vnode *)
				((vm_offset_t)vnode + nvnode * vnode_size);
		(v) = (vm_offset_t)vnodeNVNODE;
	
		valloclim(file, struct file, nfile, fileNFILE);
		valloclim(proc, struct proc, nproc, procNPROC);
#if	SEC_BASE
		valloc(secinfo, struct security_info, nproc);
#endif
		valloc(cfree, struct cblock, nclist);
		valloc(callout, struct callout, ncallout);
		valloc(namecache, struct namecache, nchsize);

#if PROFILING
                {   extern char *eprol, *etext, *s_lowpc;
                    extern u_long s_textsize;
                    extern struct phdr phdr;
#if PROFTYPE == 4
                    extern long tolimit;
#endif /* PROFTYPE == 4 */
		    u_long lstart = (u_long)&eprol;
		    u_long letext = (u_long)&etext;
                    s_lowpc = phdr.lowpc = (char *)ROUNDDOWN(lstart,
                        (long)(HISTFRACTION*sizeof(HISTCOUNTER)));
                    phdr.highpc = (char *)ROUNDUP(letext,
                        (long)(HISTFRACTION*sizeof(HISTCOUNTER)));
                    s_textsize = (u_long)phdr.highpc - (u_long)phdr.lowpc;
                    phdr.pc_bytes = (long)(s_textsize / (u_long)(HISTFRACTION));
		    phdr.pc_bytes += sizeof(long)-phdr.pc_bytes%sizeof(long);
                    valloc(phdr.pc_buf,char,phdr.pc_bytes);
#if PROFTYPE == 2 || PROFTYPE == 3
                    phdr.bb_bytes = s_textsize / BB_SCALE;
		    phdr.bb_bytes += sizeof(long)-phdr.bb_bytes%sizeof(long);
                    valloc(phdr.bb_buf,char,phdr.bb_bytes);
#endif /* PROFTYPE == (2 or 3) */
#if PROFTYPE == 4
                    phdr.froms_bytes = s_textsize / (long)(HASHFRACTION);
		    phdr.froms_bytes += sizeof(long)-phdr.froms_bytes%sizeof(long);
                    valloc(phdr.froms_buf,char,phdr.froms_bytes);
                    tolimit = s_textsize * ARCDENSITY / 100;
                    if (tolimit < MINARCS) { tolimit = MINARCS; }
                    if (tolimit > 65534) { tolimit = 65534; }
                    phdr.tos_bytes = tolimit * sizeof(struct tostruct);
		    phdr.tos_bytes += sizeof(long)-phdr.tos_bytes%sizeof(long);
                    valloc(phdr.tos_buf,char,phdr.tos_bytes);
#endif /* PROFTYPE == 4 */
                }
#endif /* PROFILING */

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
		 * Use 10% of memory for buffers, regardless. Since these
		 * pages are virtual-size pages (larger than physical page
		 * size), use only one page per buffer.
		 *
		 * If bufpages is non zero, the user has specified
		 * bufpages on the boot line and we don't want to over-ride.
		 */
		if ((bufcache <= 100) && (bufpages == 0))
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
				panic("startup: can't do tables physically");
			}
			base_addr = (vm_offset_t)tmp_addr;
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
