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
static char *rcsid = "@(#)$RCSfile: mem.c,v $ $Revision: 1.2.4.12 $ (DEC) $Date: 1992/12/10 13:42:16 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)mem.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif	lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
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
 * Memory special file: /dev/mem and /dev/kmem
 */

/*
 * Modification History: machine/alpha/mem.c
 *
 *  2-Apr-92 -- tlh
 *	Revised port to Alpha/OSF to exploit OSFPAL and flat KSEG space.
 *
 * 20-Jun-91 -- afd
 *	Ported to OSF (mach vm).  kernacc is no longer needed.
 *
 * 28-Feb-91 -- afd
 *	Fixed some int/long variables and fixed the code for minor
 *	device 0 (physical memory) for Alpha.
 *
 *  4-Feb-91 -- jmartin
 *	kernacc
 *
 * 08-Oct-90 -- afd
 *	Created this file for Alpha support.
 */
#include <sys/param.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/systm.h>
#include <sys/vm.h>
#include <sys/uio.h>

#include <mach/vm_param.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_mmap.h>

#define DEV_ZERO_MINOR	4

mmread(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	uio->uio_rw = UIO_READ;
	return (mmrw(dev, uio));
}

mmwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	uio->uio_rw = UIO_WRITE;
	return (mmrw(dev, uio));
}

unsigned int zero_page_pfn;	/* page frame number of a page of zeros */
struct vnode zero_page_vnode;
struct vnode * zero_page_vp = &zero_page_vnode;

void
init_zero_page()
{
	extern struct vnode *zero_page_vp;
	vm_offset_t	phys;
			 
	if (pmap_svatophys(vm_kern_zero_page, &phys) == KERN_SUCCESS)
		zero_page_pfn = (unsigned int)(phys >> PGSHIFT);
	else
		panic("init_zero_page: vm_kern_zero_page not mapped");
	/*
	 * Initialize a dummy vnode to keep the u_mape_dev routines
	 * happy when a process mapped to /dev/zero array exits
	 * or unmaps the memory.  (Cribbed from aio_sysinit().)
	 */
	zero_page_vp->v_usecount = 1;
	VN_LOCK_INIT(zero_page_vp);
	VN_BUFLISTS_LOCK_INIT(zero_page_vp);
	VN_OUTPUT_LOCK_INIT(zero_page_vp);
	VN_AUX_LOCK_INIT(zero_page_vp);
	zero_page_vp->v_mount = 0;
	zero_page_vp->v_type = VBAD;
	zero_page_vp->v_numoutput = 0;
	zero_page_vp->v_object= VM_OBJECT_NULL;
}

int
map_zero_page_at_0()
{
	extern int mem_no;
	extern struct vnode *zero_page_vp;
	extern int mmmap();
	struct vp_mmap_args zero_page;
	vm_offset_t addr = (vm_offset_t)0;

	zero_page.a_dev = makedev(mem_no, DEV_ZERO_MINOR);
	zero_page.a_mapfunc = mmmap;
	zero_page.a_offset = (vm_offset_t)0;
	zero_page.a_vaddr = &addr;
	zero_page.a_size = VM_MIN_ADDRESS;
	zero_page.a_prot = VM_PROT_READ;
	zero_page.a_maxprot = VM_PROT_READ;
	zero_page.a_flags = MAP_FIXED;
	if (u_dev_create(current_task()->map, zero_page_vp, &zero_page))
		return ENOMEM;
	else
		return 0;
}

void
u_dev_permit_zero_page_at_0(dev, addr, map)
	dev_t		dev;
	vm_offset_t	addr;
	vm_map_t	map;
{
	extern int mem_no;

	if (major(dev) == mem_no && minor(dev) == DEV_ZERO_MINOR
	    && addr < VM_MIN_ADDRESS)
		map->vm_min_offset = 0;
}

int
mmmap(dev_t dev, vm_offset_t offset, vm_prot_t prot)
{
	extern unsigned int	zero_page_pfn;
	extern int		consmem;

	switch (minor(dev)) {
	      default:
		 return -1;

	       case 0:
		 return (pmap_valid_page(offset)
			 || ((prot & VM_PROT_WRITE == 0)
			     && (offset < alpha_ptob(consmem))))
			 ? (int)(offset >> PGSHIFT)
			 : -1;

	       case DEV_ZERO_MINOR:
		 return ((prot & VM_PROT_WRITE) ? -1 : (int)zero_page_pfn);
	 }
}

/*
 * mmrw and kernacc assume the following attributes of console/PAL memory:
 * 1) it starts at 0;
 * 2) it ends at alpha_ptob(consmem) (cf. ./pmap.c);
 * 3) it has no associated bit maps and no bad pages.
 */

mmrw(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register struct iovec *iov;
	register vm_size_t length;
	int error = 0;
	extern int consmem;

	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}

		switch (minor(dev)) {

			register vm_offset_t	start, end, addr, consend;
		/*
		 * minor device 0 is physical memory
		 */
		case 0:
			start = (vm_offset_t)uio->uio_offset;
			length = (vm_size_t)iov->iov_len;
			end = start + length;

			/* start < end <= "size of kseg space" */
			if (end <= start || end > SEG1_BASE - UNITY_BASE)
				goto fault;

			/* Read-only access to console/PAL */
			consend = alpha_ptob(consmem);
			if (start < consend && uio->uio_rw != UIO_READ)
				goto fault;

			/* Check for bad pages. */
			for (addr = start; addr < end; addr += NBPG)
				if (addr >= consend && !pmap_valid_page(addr))
					goto fault;

			/* Check the last page, if missed above. */
			if (addr >= consend
			    && (addr ^ (end - 1)) < NBPG
			    && !pmap_valid_page(addr))
				goto fault;

			error = uiomove(PHYS_TO_KSEG(start), length, uio);
			continue;

		/* 
		 * minor device 1 is kernel memory
		 */
		case 1:
			start = (vm_offset_t)uio->uio_offset;
			length = (vm_size_t)iov->iov_len;
			if (!kernacc(start, length, uio->uio_rw))
				goto fault;
			error = uiomove(start, length, uio);
			continue;

		/* 
		 * minor device 2 is EOF/RATHOLE
		 */
		case 2:
			if (uio->uio_rw == UIO_READ)
				return (0);
			length = (vm_size_t)iov->iov_len;
			break;
		
		/*
		 * Unknown minor device or UNIBUS
		 */
		case 3:
		default:
			error = ENXIO;
			break;

		/* 
		 * minor device zero.
		 */
		case DEV_ZERO_MINOR:
			length = MIN((vm_size_t)iov->iov_len, NBPG);
			if (uio->uio_rw != UIO_READ)
				return 0;
			start = (vm_offset_t)uio->uio_offset;
			if (start < 0 || start + length < start)
				goto fault;
			error = uiomove(vm_kern_zero_page, length, uio);
			continue;
		}
		if (error)
			break;
		iov->iov_base += length;
		iov->iov_len -= length;
		uio->uio_offset += length;
		uio->uio_resid -= length;
	}
	return (error);
fault:
	return (ENXIO);
}

/*
 * kernacc -- Check for kernel access
 */
static int
kernacc(base, len, rw)
	register vm_offset_t	base;
	register vm_size_t	len;
	enum uio_rw		rw;
{
	register vm_offset_t	last;
	extern vm_map_t		kernel_map;
	extern int		consmem;
	extern char		start[], etext[], end[];

	/*
	 * sanity check
	 */
	if (!len)
		return(1);

	/* Fail if not kseg or seg1 */
	if (!IS_SYS_VA(base))
		return 0;

	last = base + len - 1;

	/*
	 * Check for wrap around.
	 */
	if (last < base)
		return(0);

	/* kseg */
	if (IS_KSEG_VA(base)) {
		register vm_offset_t	consend;

		consend = PHYS_TO_KSEG(alpha_ptob(consmem));

		if (base < consend && rw != UIO_READ)
			return 0;		/* no write to console/PAL */

		while (base <= last && base < SEG1_BASE) {
			if (base >= consend
			    && !pmap_valid_page(KSEG_TO_PHYS(base)))
				return 0;	/* bad page */
			base += NBPG;
		}

		if (last < SEG1_BASE)
			if ((base ^ last) >= NBPG)
				return 1;	/* all pages good */
			else if (base >= consend
				 && !pmap_valid_page(KSEG_TO_PHYS(base)))
				return 0;	/* last page bad */
			else
				return 1;	/* last page good */
		else
			base = SEG1_BASE;	/* pages in kseg to check */
	}

	/* seg1 */
	if (base >= (vm_offset_t)((rw == UIO_READ) ? start : etext) &&
	    last < (vm_offset_t)end)
		return 1;
	return(vm_map_check_protection(kernel_map, trunc_page(base), 
				       round_page(last + 1), rw == UIO_READ ? 
						VM_PROT_READ : VM_PROT_WRITE));
}
