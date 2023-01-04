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
static char	*sccsid = "@(#)$RCSfile: counter.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:06 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1988 Carnegie-Mellon University
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
 *	mmax/counter.c: implement mmax-specific mapfrcounter syscall.
 */
 
#include <mach_xp.h>

#include <kern/task.h>
#include <kern/thread.h>
#include <mach/vm_param.h>
#include <mach/memory_object.h>
#include <mach/vm_inherit.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <mach/vm_prot.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/user.h>

#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>

/* #define	XM21 */

/*
 *	Function to tell device pager where page containing counter
 *	is in physical memory.  Ignores its arguments, as it will only
 *	be called indirectly from mapfrcounter().  Non-XP systems pass
 *	its result through ptob() to find the physical address (XXX), XP
 *	systems call pmap_phys_address() which passes its arg through ptoa().
 */

#define	CTR_ADDR	((vm_offset_t)	SCCREG_FRCNT)

int frcmapfun(dev, offset, prot)
dev_t		dev;
vm_offset_t	offset;
int		prot;
{
#ifdef	lint
	dev++; offset++; prot++;
#endif
	return(atop(trunc_page(CTR_ADDR)));
}

/*
 *	System call to map the counter.
 */

mapfrcounter(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	register struct args {
		int		*address;
		boolean_t	anywhere;
	} *uap = (struct args *) args;
	
	memory_object_t	pager;
	vm_offset_t	address;
	kern_return_t	result;
	int		error;
#ifdef	XM21
	vm_map_t	map = current_thread()->task->map;
	vm_object_t	vm_object_special();
#endif

	if(error = copyin((caddr_t)uap->address, &address, sizeof(vm_offset_t)))
		return (error);

	/*
	 *	Get a pager for the counter.  Device argument is
	 *	ignored by frcmapfun.
	 */

	pager = device_pager_create((dev_t)0, frcmapfun, PROT_READ,
		(vm_offset_t)0, PAGE_SIZE);

	if (uap->anywhere)
		address = VM_MIN_ADDRESS;
	else
		address = trunc_page(address);

	/*
	 *	Map the memory read-only with shared inheritance.
	 */
#ifdef	XM21
	result = vm_map_find(map, object, (vm_offset_t)0,
		&address, PAGE_SIZE, uap->anywhere);
	if (result != KERN_SUCCESS) {
		vm_object_deallocate(object);
	}
	else {
		result = vm_protect(map, address, PAGE_SIZE,
			TRUE, VM_PROT_READ);

		if (result == KERN_SUCCESS) {
			result = vm_inherit(map, address, PAGE_SIZE,
				VM_INHERIT_SHARE);
		}
		if (result != KERN_SUCCESS) {
			(void) vm_deallocate(map, address, PAGE_SIZE);
		}
	}
#else
	result = vm_map(current_thread()->task->map,
			&address, PAGE_SIZE, (vm_offset_t)0, uap->anywhere,
			pager, (vm_offset_t)0,
			FALSE,
			VM_PROT_READ, VM_PROT_READ,
			VM_INHERIT_SHARE);

#endif
	/*
	 *	Tell user where counter is or set failure code.
	 */
	if (result == KERN_SUCCESS) {
		address += CTR_ADDR - trunc_page(CTR_ADDR);
		error = copyout(&address, uap->address, sizeof(vm_offset_t));
	}
	else
		error = EFAULT;

#ifndef	XM21
	/*
	 *	Throw away reference acquired when pager was created.
	 */
	port_release(pager);
#endif
	return (error);
}
