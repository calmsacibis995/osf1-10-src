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
static char *rcsid = "@(#)$RCSfile: u_mape_dev.c,v $ $Revision: 1.1.12.2 $ (DEC) $Date: 1993/09/22 14:54:11 $";
#endif
/*
 * Supports memory mapped regions
 */

#include <mach_ldebug.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <kern/zalloc.h>
#include <sys/vp_swap.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <vm/vm_fault.h>
#include <vm/vm_control.h>
#include <vm/vm_tune.h>
#include <vm/vm_umap.h>
#include <vm/vm_vlock.h>
#include <vm/pmap.h>
#include <vm/u_mape_dev.h>
#include <vm/vm_anon.h>
#include <vm/vm_mmap.h>

extern int
	u_dev_fault(),
	u_dev_dup(),
	u_dev_unmap(),
	u_dev_lockop(),
	u_dev_core(),
	u_dev_control(),
	u_dev_protect(),
	u_dev_check_protect(),
	u_dev_kluster(),
	u_dev_copy(),
	u_dev_grow(),
	u_dev_noop();

struct vm_map_entry_ops u_mape_dev_ops = {
	&u_dev_fault,		/* fault */
	&u_dev_dup,		/* dup */
	&u_dev_unmap,		/* unmap */
	&u_dev_noop,		/* msync */
	&u_dev_lockop,		/* lockop */
	&u_dev_noop,		/* swap */
	&u_dev_core,		/* corefile */
	&u_dev_control,		/* control */
	&u_dev_protect,		/* protect */
	&u_dev_check_protect,	/* check protection */
	&u_dev_kluster,		/* kluster */
	&u_dev_copy,		/* copy */
	&u_dev_grow,		/* grow */
};

/*
 * Create the private map entry information
 */

#ifdef __alpha
extern void u_dev_permit_zero_page_at_0();
#else
#define u_dev_permit_zero_page_at_0(dev, addr, map)
#endif

int
u_dev_create(vm_map_t map, struct vnode *vp, vm_offset_t args)
{
	register struct vp_mmap_args *ap = (struct vp_mmap_args *) args;
	struct u_dev_object *dp;
	kern_return_t ret;

	if (ap->a_flags & MAP_FIXED) {
		vm_map_lock(map);
		ret = (*map->vm_delete_map)(map, *(ap->a_vaddr), 
			*(ap->a_vaddr) + ap->a_size, FALSE);
		vm_map_unlock(map);
		if (ret != KERN_SUCCESS) return ret;
	}

	if ((ret = vm_object_allocate(OT_DEVMAP, 
		ap->a_size, (caddr_t) 0, &dp)) != KERN_SUCCESS) return ret;

	dp->ud_mapfunc = ap->a_mapfunc;
	dp->ud_offset = ap->a_offset;
	dp->ud_dev = ap->a_dev;
	VREF(vp);
	dp->ud_vp = vp;
	u_dev_permit_zero_page_at_0(ap->a_dev, *ap->a_vaddr, map);
	ret = u_map_enter(map, ap->a_vaddr, ap->a_size, page_mask,
		(ap->a_flags & MAP_FIXED) ? FALSE : TRUE, 
		dp, (vm_offset_t) 0, FALSE,
		ap->a_prot, ap->a_maxprot,  VM_INHERIT_COPY);
	return ret;
}

/*
 * Wiring fault 
 *	- kernel wirings and unwirings enter with the map write locked.
 *	- normal faults enter with a fault lock already taken.
 */

kern_return_t
u_dev_fault(register vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t size,
	vm_prot_t access_type,
	vm_fault_t wiring,
	vm_page_t *pp)
{
	register struct u_dev_object *dp = 
		(struct u_dev_object *) (ep->vme_object);
	register struct vpage *vp;
	register int vpi, i, npages;
	register vm_offset_t addr;
	vm_offset_t offset, physaddr;
	boolean_t wired;
	kern_return_t ret;
	int pf;

	if (wiring == VM_WIRE && dp->ud_flags & UDF_NOWIRING) 
		return KERN_INVALID_ADDRESS;

	/*
	 * Compute the vpage
	 */

	if (ep->vme_private) {
		vp = (struct vpage *)(ep->vme_private) + 
			atop(va - ep->vme_start);
		vpi = 1;
	}
	else {
		vp = &ep->vme_vpage;
		vpi = 0;
	}

	for(npages = atop(round_page(size)), i = 0; npages; npages--) {
		if ((access_type & (vp + i)->vp_prot) != access_type)
			return KERN_PROTECTION_FAILURE;
		i += vpi;
	}

	/*
	 * Handle the faults
	 */

	offset = va - ep->vme_start;
	for (addr = va; addr < (va + size) ; 
		addr += PAGE_SIZE, offset += PAGE_SIZE, vp += vpi) {
		if (wiring != VM_NOWIRE) {
			if (vp->vp_plock) continue;
			else if (wiring == VM_UNWIRE) {
				pmap_pageable(vm_map_pmap(ep->vme_map), 
					addr, addr + PAGE_SIZE, TRUE);
				pmap_change_wiring(vm_map_pmap(ep->vme_map),
					addr, FALSE);
				continue;
			}
			else {
				pmap_pageable(vm_map_pmap(ep->vme_map), 
					addr, addr + PAGE_SIZE, FALSE);
				wired = TRUE;
			}
		}
		else wired = FALSE;
		pf = (*dp->ud_mapfunc)(dp->ud_dev, 
				ep->vme_offset + offset + dp->ud_offset, 
				vp->vp_prot);
		if (pf == -1) {
			ret = KERN_INVALID_ADDRESS;
			goto failed;
		}
		else physaddr = pmap_phys_address(pf);

		/*
		 * Fabricate a page to satisfy page get requests
		 * for the Mach copy_overwrite mechanism.
		 */

		if (wiring == VM_PAGEGET) {
			*pp = (vm_page_t) kalloc(sizeof (struct vm_page));
			if (*pp == VM_PAGE_NULL) return KERN_RESOURCE_SHORTAGE;
			else {
				(*pp)->pg_phys_addr = physaddr;
				(*pp)->pg_object = (vm_object_t) dp;
				(*pp)->pg_hold = 1;
			}
			break;
		}
		else pmap_enter(vm_map_pmap(ep->vme_map),
			addr, physaddr,	vp->vp_prot, wired, access_type);
	}

	return KERN_SUCCESS;

failed:
	
	/*
	 * If wiring, undo what was done.
	 */

	if (wiring == VM_WIRE && !vp->vp_plock) {
		pmap_pageable(vm_map_pmap(ep->vme_map), va, addr - va, FALSE);
		while (va < addr) {
			pmap_change_wiring(vm_map_pmap(ep->vme_map),
					va, FALSE);
			va += PAGE_SIZE;
		}
			
	}
	return ret;
}

/*
 * Dup the device.
 */

kern_return_t
u_dev_dup(vm_map_entry_t ep,
	vm_offset_t start,
	register vm_size_t size,
	vm_map_entry_t newep,
	vm_copy_t copy)
{
	register struct u_dev_object *dp, *dp2;
	struct u_dev_object *obj;
	kern_return_t ret;

	switch (copy) {
	case VM_COPYU:
		break;
	case VM_COPYMCOPY:
		return u_dev_mdup(ep, start, size, newep);
	default:
		return KERN_INVALID_ARGUMENT;
	}

	if ((ret = vm_object_allocate(OT_DEVMAP, size, (caddr_t) 0, &obj)) 
		!= KERN_SUCCESS) return ret;
	dp = (struct u_dev_object *) ep->vme_object;

	dp2 = obj;

	newep->vme_offset = start - ep->vme_start;
	newep->vme_object = (vm_object_t) dp2;
	dp2->ud_vp = dp->ud_vp;
	VREF(dp->ud_vp);
	dp2->ud_mapfunc = dp->ud_mapfunc;
	dp2->ud_dev = dp->ud_dev;

	return ret;
}

/*
 * Mach dup of device memory.
 * Mach has a page structure for each device memory page.
 * We only allocate them when silly Mach Technology requires
 * such a gross and burdensome operation.  The only
 * place this is currently required is in ptrace.  Of course
 * if Mach read, write or copy calls are required then we 
 * satisfy them here as well.  Why all of this ?  We can't
 * do COW on device memory.  
 */

kern_return_t
u_dev_mdup(register vm_map_entry_t ep, 
	vm_offset_t start, 
	vm_size_t size, 
	register vm_map_entry_t newep)
{
	struct vm_anon_object *aop;
	kern_return_t ret;
	vm_page_t srcp, dstp;
	register vm_offset_t end;

	if ((ret = u_anon_allocate(size, FALSE, &aop)) != KERN_SUCCESS)
		return ret;

	/*
	 * Fill all required fields to take a fault.
	 * No translations are loaded because of VM_PAGEGET operations.
	 * The map which references ep is the owning map of the newep
	 * while VM_PAGEGET operations are done.  This may change
	 * during the vm_copyout but is currently of no consequence.
	 */

	newep->vme_object = (vm_object_t) aop;
	newep->vme_ops = vm_object_to_vmeops(aop);
	newep->vme_offset = (vm_offset_t) 0;
	newep->vme_map = ep->vme_map;

	for (end = start + size; start < end; start += PAGE_SIZE) {
		ret = (*ep->vme_fault)(ep, start, PAGE_SIZE,
			VM_PROT_READ, VM_PAGEGET, &srcp);
		if (ret != KERN_SUCCESS) goto failed;
		ret = (*newep->vme_fault)(newep, start, PAGE_SIZE,
			VM_PROT_READ, VM_PAGEGET, &dstp);
		if (ret != KERN_SUCCESS) {
			OOP_PAGECTL(srcp->pg_object, &srcp, 1, VMOP_RELEASE);
			goto failed;
		}
		vm_page_copy(srcp, dstp);
		OOP_PAGECTL(srcp->pg_object, &srcp, 1, VMOP_RELEASE);
		OOP_PAGECTL(dstp->pg_object, &dstp, 1, VMOP_RELEASE);
	}
	return KERN_SUCCESS;
failed:
	vm_object_deallocate(aop);
	return ret;
}

/*
 * Unmap all or part of a user region
 * We release any vpage locks.
 */

kern_return_t
u_dev_unmap(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t len)
{
	/*
	 * If this operaration is going to split then ascertain 
	 * whether the map can accomodate the expansion.
	 */

	if (!u_map_entry_delete_check(ep->vme_map, ep, va, va + len))
		return KERN_RESOURCE_SHORTAGE;
 

	/*
	 * Release potential wirings
	 */

	u_dev_lockop(ep, va, len, VM_UNWIRE);

	pmap_remove(vm_map_pmap(ep->vme_map), va, va + len);
	u_map_entry_unmap(ep, va, len);
	return KERN_SUCCESS;
}

/*
 * Lock the device memory
 */

kern_return_t
u_dev_lockop(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t len,
	vm_fault_t wire)
{
	register vm_offset_t start, end;
	register struct vpage *vp;
	register int vpi;
	kern_return_t ret;


	/*
	 * If not convering the entire entry and no vpage array check
	 */

	vp = (struct vpage *) (ep->vme_private);

	if (!vp && (va != ep->vme_start || (va + len) != ep->vme_end)) { 

		/*
		 * If unwiring and never wired just return
		 */

		if (wire == VM_UNWIRE && !ep->vme_plock) return KERN_SUCCESS;
		else if (ret = u_vpage_init(ep, FALSE)) return ret;
		else vp = (struct vpage *) (ep->vme_private);
	}

	if (vp) {
		vpi = 1;
		vp += atop(ep->vme_start - va);
	}
	else {
		vp = &ep->vme_vpage;
		if (vp->vp_plock && wire == VM_WIRE) return KERN_SUCCESS;
		else if (!vp->vp_plock && wire == VM_UNWIRE) 
			return KERN_SUCCESS;
		vpi = 0;
	}

	for (ret = KERN_SUCCESS, start = va, end = va + len; start < end; 
		vp += vpi, start += PAGE_SIZE) {

		if (wire == VM_UNWIRE) {
			if (vp->vp_plock) vp->vp_plock = 0;
			else if (vpi) continue;
		}
		if (vl_kwire(ep, start, start + PAGE_SIZE) == FALSE)
			ret = u_dev_fault(ep, start, 1, 
				vp->vp_prot, wire, (vm_page_t *) 0);
		if (ret != KERN_SUCCESS) goto failed;
		if (wire == VM_WIRE && vpi) vp->vp_plock = 1;
	}

	if (!vpi && wire == VM_WIRE) vp->vp_plock = 1;

	return KERN_SUCCESS;
failed:
	if (wire == VM_WIRE) {
		wire = VM_UNWIRE;
		end = start;
		if (vpi) vp = (struct vpage *) (ep->vme_private) +
			atop(va - ep->vme_start);
		for (start = va; start < end; start += PAGE_SIZE, vp += vpi) {
			vp->vp_plock = 0;
			if (vl_kwire(ep, start, start + PAGE_SIZE) == FALSE) 
				 u_dev_fault(ep, start, 1, 
						ep->vme_maxprot, wire,
						(vm_page_t *) 0);
		}
	}
	return ret;
}

/*
 * No corefile information is produced
 */

u_dev_core(vm_map_entry_t ep,
	unsigned next, 
	char *vec, 
	int *vecsize)
{
	*vecsize = -1;
	return;
}

/*
 * Can't think of any valid reason for a control operation.
 */

kern_return_t
u_dev_control(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t len,
	vm_control_t control)
{
	return KERN_INVALID_ARGUMENT;
}

/*
 * Change the protection.
 */

kern_return_t
u_dev_protect(vm_map_entry_t ep,
	register vm_offset_t va,
	register vm_size_t size,
	register vm_prot_t prot)
{
	register struct u_dev_object *dp;
	register struct vpage *vp;
	register vm_prot_t oprot;
	register vm_offset_t start;
	kern_return_t ret;

	dp = (struct u_dev_object *) (ep->vme_object);

	vp = (struct vpage *)(ep->vme_private);
	if (!vp && (ep->vme_start == va && ep->vme_end == (va + size))) {
		oprot = ep->vme_protection;

		ep->vme_protection = prot;

		if (oprot != ep->vme_protection) 
			pmap_protect(vm_map_pmap(ep->vme_map), va,
				va + size, ep->vme_protection);
		return KERN_SUCCESS;
	}
	else if (!vp) {
		if (ep->vme_protection == prot) return KERN_SUCCESS;
		else if ((ret = u_vpage_init(ep, TRUE)) != KERN_SUCCESS) 
			return ret;
	}


	vp += atop(va - ep->vme_start);


	for (start = va; size; start += PAGE_SIZE, vp++, size -= PAGE_SIZE) {
		oprot = vp->vp_prot;
		vp->vp_prot = prot;
		if (oprot != vp->vp_prot) 
			pmap_protect(vm_map_pmap(ep->vme_map), start,
				 start + PAGE_SIZE, vp->vp_prot);
	}

	return KERN_SUCCESS;
}

/*
 * Check the protection.
 */

boolean_t
u_dev_check_protect(register vm_map_entry_t ep,
	register vm_offset_t va,
	vm_size_t size,
	register vm_prot_t prot)
{
	register struct vpage *vp;
	register int nvp;

	vp = (struct vpage *) (ep->vme_private);
	
	if (vp) {
		vp += atop((va - ep->vme_start));
		nvp = atop(size);
	}
	else {
		nvp = 1;
		vp = &ep->vme_vpage;
	}
	for (; nvp--; vp++) if ((vp->vp_prot & prot) != prot) return FALSE;
	return TRUE;
}

/*
 * No klustering done for memory mapped devices.
 */

int
u_dev_kluster(vm_map_entry_t ep,
	vm_offset_t va,
	int pcnt,
	vm_offset_t *back, 
	vm_offset_t *forward)
{
	*forward = *back = ep->vme_offset + (va - ep->vme_start);
	return 0;
}

/*
 * Dup copy isn't supported.
 */

u_dev_copy(vm_map_entry_t ep, vm_copy_op_t op)
{
	return KERN_INVALID_ARGUMENT;
}

u_dev_grow(vm_map_entry_t ep,
	vm_prot_t prot,
	vm_size_t increase,
	as_grow_t direction)
{
	return KERN_NO_SPACE;
}

/*
 * This terminate handle is called for obj entry into a map failures
 * and when the reference count is <= 1
 * We only have to release the vp because the reset has already been done.
 */

int
u_dev_oop_terminate(register struct u_dev_object *dp)
{
	vrele(dp->ud_vp);
	vm_object_unlock((vm_object_t) dp);
	vm_object_free(dp);
}

/*
 * Routine to free page structures allocated
 * in u_dev_fault code.
 */

int
u_dev_oop_pagectl(register struct u_dev_object *dp,
	register vm_page_t *pl,
	register int pcnt,
	int flags)
{
	if (flags != VMOP_RELEASE) 
		panic("u_dev_oop_pagectl: illegal operation");
	for (;pcnt--; pl++) {
		if ((*pl)->pg_hold != 1) 
			panic("u_dev_oop_pagectl: hold count not one");
		kfree(*pl, sizeof (struct vm_page));
	}
	return KERN_SUCCESS;
}

int
u_dev_oop_bad()
{
	panic("u_dev_oop_bad: bad object operation");
}

kern_return_t
u_dev_oop_pagesteal()
{
	return KERN_FAILURE;
} 

/*
 * Object management stuff.
 */

struct vm_object_ops u_dev_oop = {
	&u_dev_oop_bad,			/* lock try */
	&u_dev_oop_bad,			/* unlock */
	&vm_ops_reference_def,		/* reference */
	&vm_ops_deallocate_def,		/* deallcoate */
	&u_dev_oop_bad,			/* pagein */
	&u_dev_oop_bad,			/* pageout */
	&u_dev_oop_bad,			/* swapout */
	&u_dev_oop_bad,			/* control */
	&u_dev_oop_pagectl,		/* page control */
	&u_dev_oop_pagesteal,		/* page steal */
};

/*
 * Configuration information
 */

struct vm_object_config dev_object_conf = {
	(int (*)()) 0,
	u_dev_oop_terminate,
	sizeof (struct u_dev_object),
	&u_dev_oop,
	&u_mape_dev_ops,
};
	
int
u_dev_noop()
{
	return 0;
}
