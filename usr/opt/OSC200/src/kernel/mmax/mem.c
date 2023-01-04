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
static char	*sccsid = "@(#)$RCSfile: mem.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:45 $";
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

/*
 * Memory special file
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/uio.h>

#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>

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

mmrw(dev, uio)
    dev_t dev;
    struct uio *uio;
{
    register int o;
    register u_int c, v;
    register struct iovec *iov;
    int error = 0;
    vm_offset_t	where;
    vm_offset_t	start_addr, end_addr;
    vm_prot_t	protection;
    vm_offset_t	pstart, pend;
    extern vm_offset_t	virtual_avail;


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

/* 
 * Minor device 0 is physical memory.
 */
	    case 0:
		v = trunc_page(uio->uio_offset);
		if (uio->uio_offset >= mem_size)
		    goto fault;

		where = vm_map_min(kernel_map);
		if (vm_map_find(kernel_map, VM_OBJECT_NULL,
		    (vm_offset_t) 0, &where, PAGE_SIZE, TRUE) != KERN_SUCCESS){
		    goto fault;
		}
		pmap_enter(vm_map_pmap(kernel_map), where, v,
				VM_PROT_READ|VM_PROT_WRITE, FALSE);
		pmap_update();
		o = uio->uio_offset - v;
		c = min(PAGE_SIZE - o, (u_int)iov->iov_len);
		error = uiomove((caddr_t) (where + o), c, uio);
		vm_map_remove(kernel_map, where, where + PAGE_SIZE);
		continue;


/* 
 * Minor device 1 is kernel memory.
 */
	    case 1:
#define	KMEM_MAX_WIRE_PAGES	(16)
#define	KMEM_MAX_WIRE_BYTES	(KMEM_MAX_WIRE_PAGES * PAGE_SIZE)
		/*
		 *  Even though only trusted programs should read
		 *  and write /dev/kmem, give the kernel an opportunity
		 *  to restrict the scope of the I/O to prevent user error
		 *  from harming the system.  (Don't want someone accidentally
		 *  wiring down the entire address space.)  Long requests will
		 *  be handled KMEM_MAX_WIRE_PAGES at a time.
		 */
		c = MIN(iov->iov_len, KMEM_MAX_WIRE_BYTES);

		/*
		 *  Duplicate the essence of VM_MAP_RANGE_CHECK.
		 *  Reject bogus requests.
		 */
		start_addr = trunc_page(uio->uio_offset);
		end_addr = round_page(uio->uio_offset + c);
		if (start_addr < vm_map_min(kernel_map) ||
		    end_addr > vm_map_max(kernel_map) ||
		    start_addr > end_addr)
			goto fault;

		/*
		 *  Some areas of the kernel (e.g., text+data) in effect
		 *  have been conveniently pre-wired for us; page faults
		 *  are not allowed on these areas so we must avoid
		 *  calling vm_map_pageable() on them.  We have already
		 *  disallowed requests for addresses below the lowest
		 *  address in the kernel map so now we clip the requested
		 *  region at the last known pre-wired kernel address.
		 */
		pstart = MAX(start_addr, virtual_avail);
		pend = MAX(end_addr, virtual_avail);

		/*
		 *  We must be able to wire down the region in question
		 *  and read from or write to it as the case may be.
		 *  Optimize for the case where we're operating completely
		 *  on pre-wired memory, as will frequently happen with
		 *  kmem users examining statically-allocated structures.
		 *  After the copy is complete, unwire the region.
		 */
		protection = (uio->uio_rw == UIO_WRITE ? 
				VM_PROT_WRITE : VM_PROT_READ);
		if (pend - pstart > 0)
			if (vm_map_pageable(kernel_map, pstart, pend,
					    protection) != KERN_SUCCESS)
				goto fault;

		if (pstart < virtual_avail)
			if (vm_map_check_protection(kernel_map, start_addr,
						    end_addr, protection)
			    == FALSE)
				goto fault;

		if (uio->uio_rw == UIO_WRITE)
			error = copyin(iov->iov_base,
				       (caddr_t) uio->uio_offset, (int) c);
		else
			error = copyout((caddr_t) uio->uio_offset,
					iov->iov_base, (int) c);

		/*
		 *  Not only do we not need to break early if error is set,
		 *  we *must* not break before we unwire the region.
		 */
		if (pend - pstart > 0)
			if (vm_map_pageable(kernel_map, pstart, pend,
					    VM_PROT_NONE) != KERN_SUCCESS)
				panic("mmrw:  couldn't remap");

		break;

/* 
 * Minor device 2 is EOF/RATHOLE.
 */
	    case 2:
		if (uio->uio_rw == UIO_READ)
		    return (0);
		c = iov->iov_len;
		break;

/* 
 * All other minor device numbers return an error.
 */
	    default:
		goto fault;

	}
	if (error)
	    break;
	iov->iov_base += c;
	iov->iov_len -= c;
	uio->uio_offset += c;
	uio->uio_resid -= c;
    }

    return (error);
fault:
    return (EFAULT);
}
