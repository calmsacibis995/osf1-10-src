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
static char *rcsid = "@(#)$RCSfile: vm_cmap.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 18:59:10 $";
#endif
#include <vm/vm_map.h>
#include <vm/vm_umap.h>
#include <vm/vm_kern.h>
#include <vm/vm_lock.h>

/*
 * This code supports the kernel copy map technology.
 * We want C submaps to be treated the same way as U submaps.
 * Most of the C submap operation handles are identical to the U map
 * ones.  There two major differences are:
 *	1) There is no support of map deallocation,
 *		inherit, keep_on_exec, map_exec
 *	    The last three serve no purpose for copy maps and should
 *          be caught for errors.
 *	2) The IPC code has used vm_map_pageable extensively to prevent
 *         faults while doing certain operations.  It is already considered
 * 	   dangerous to allow a deletion of VAS while kernel wirings are
 *	   held.  So rather than change all of the IPC code the 
 *	   vm_map_pageable calls are being vectored to u_map_lock_vas.
 *	   This could create one of two problems which will have to be
 *	   tracked:
 *		Is the wiring being done on anything but private memory ?
 *		For example if the wiring is being done on VAS created
 *		by a copyout the access is a write then data consistency
 *		problems will arise because COW won't be abided by.
 *
 *		Is the wiring to the copy map a real wiring for something
 *		like physical I/O ?
 *
 *	    If your worried about any of the above issues, then feel free
 *	    to fix the IPC code's utilization of vm_map_pageable.  After doing
 *	    that change this code by replacing/removing c_map_wire with
 *	    u_map_wire.
 */

extern int
	c_map_invalid(),
	u_map_fault(),
	c_map_wire(),
	u_map_allocate(),
	u_map_enter(),
	u_map_protect(),
	c_map_invalid(),
	c_map_invalid(),
	c_map_invalid(),
	u_map_delete(),
	u_map_check_protection(),
	u_map_copy_overwrite(),
	u_map_copyout(),
	u_map_copyin();

struct vm_map_ops c_map_ops = {
	&c_map_invalid,
	&u_map_fault,
	&c_map_wire,
	&u_map_allocate,
	&u_map_enter,
	&u_map_protect,
	&c_map_invalid,
	&c_map_invalid,
	&c_map_invalid,
	&u_map_delete,
	&u_map_check_protection,
	&u_map_copy_overwrite,
	&u_map_copyout,
	&u_map_copyin,
};



/*
 * Allocate a copy map
 */

vm_map_t
kmem_csuballoc(vm_map_t parent,
	vm_offset_t *min,
	vm_offset_t *max,
	vm_size_t size)
{
	register vm_map_t submap;
	vm_map_entry_t entry;

	if (parent != kernel_map) panic("kmem_csuballoc: not kernel_map");
	submap = kmem_suballoc(parent, min, max, size, TRUE);

	vm_map_lock(kernel_map);
	if (!vm_map_lookup_entry(kernel_map, vm_map_min(submap), &entry))
		panic("kmem_csuballoc: entry not found in kernel_map");
	entry->vme_copymap = 1;
	submap->vm_ops = &c_map_ops;
	submap->vm_copy_map = 1;
	vm_map_unlock(kernel_map);

	if (u_map_create(submap) != KERN_SUCCESS) 
		panic("kmem_csuballoc: u_map_create failed");
	submap->vm_umap = 0;
	return submap;	
};


c_map_invalid()
{
	panic("c_map_invalid: invalid map operation");
}

kern_return_t
c_map_wire(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	vm_prot_t access_type)
{
	return u_map_lockvas(map, start, end,
		(access_type == VM_PROT_NONE) ? 
			VML_UNLOCK_RANGE :
			VML_LOCK_RANGE);
}

kern_return_t
c_map_allocate_nonpaged(vm_map_t map,
	vm_offset_t *addrp,
	vm_size_t size,
	vm_prot_t prot,
	boolean_t anywhere)
{
	kern_return_t ret;

	ret = vm_allocate(map, addrp, size, anywhere);
	if (ret == KERN_SUCCESS)
		ret = vm_map_pageable(map, *addrp, *addrp + size, prot);
	return ret;
}
