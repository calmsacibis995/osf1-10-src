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
static char *rcsid = "@(#)$RCSfile: u_mape_vp.c,v $ $Revision: 1.1.15.6 $ (DEC) $Date: 1993/12/02 22:08:25 $";
#endif

/*
 * Supports memory mapped vnode regions
 */

#include <sys/unix_defs.h>
#include <sys/buf.h>
#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <vm/vm_object.h>
#include <vm/vm_swap.h>
#include <vm/vm_anon.h>
#include <vm/u_mape_seg.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vp_swap.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <vm/vm_tune.h>
#include <vm/vm_vp.h>
#include <vm/vm_mmap.h>
#include <vm/vm_umap.h>
#include <sys/mman.h>
#include <vm/vm_perf.h>
#include <vm/vm_pagelru.h>
#include <vm/vm_page.h>

extern int
	u_vp_fault(),
	u_vp_dup(),
	u_vp_unmap(),
	u_vp_msync(),
	u_vp_lockop(),
	u_vp_swap(),
	u_vp_core(),
	u_vp_control(),
	u_vp_protect(),
	u_vp_check_protect(),
	u_vp_kluster(),
	u_vp_copy(),
	u_vp_grow();

struct vm_map_entry_ops u_vp_mape_ops = {
	&u_vp_fault,		/* fault */
	&u_vp_dup,		/* dup */
	&u_vp_unmap,		/* unmap */
	&u_vp_msync,		/* msync */
	&u_vp_lockop,		/* lockop */
	&u_vp_swap,		/* swap */
	&u_vp_core,		/* corefile */
	&u_vp_control,		/* control */
	&u_vp_protect,		/* protect */
	&u_vp_check_protect,	/* check protection */
	&u_vp_kluster,		/* kluster */
	&u_vp_copy,		/* copy */
	&u_vp_grow,		/* grow */
};

extern boolean_t wire_check();
extern boolean_t u_seg_enabled;

u_vp_create(vm_map_t map,
	struct vm_vp_object *vop,
	vm_offset_t args)
{
	register struct vp_mmap_args *ap = (struct vp_mmap_args *) args;
	kern_return_t ret;
	struct vnode *vp;

	if ((u_seg_enabled == TRUE) &&
		((ap->a_prot & (VM_PROT_EXECUTE|VM_PROT_WRITE))
			== VM_PROT_EXECUTE) &&
		(u_seg_create(map, vop, ap) == KERN_SUCCESS))
			return KERN_SUCCESS;

	/*
	 * Take our reference to the vnode
	 */

	vp = vop->vo_vp;
	VREF(vp);

	if (ap->a_flags & MAP_PRIVATE)
		return u_anon_create(map, vop, (vm_offset_t) ap);

	if (ap->a_flags & MAP_FIXED) {
		vm_map_lock(map);
		ret = (*map->vm_delete_map)(map, *(ap->a_vaddr),
			*(ap->a_vaddr) + ap->a_size, FALSE);
		vm_map_unlock(map);
		if (ret != KERN_SUCCESS) {
			vrele(vp);
			return ret;
		}
	}

	ret = u_map_enter(map, ap->a_vaddr, round_page(ap->a_size), page_mask,
		(ap->a_flags & MAP_FIXED) ? FALSE : TRUE,
		vop, ap->a_offset, (ap->a_flags & MAP_SHARED) ? FALSE : TRUE,
		ap->a_prot, ap->a_maxprot,  VM_INHERIT_COPY);

	return ret;
}

kern_return_t
u_vp_fault(register vm_map_entry_t ep,
	vm_offset_t va,
	register vm_size_t size,
	vm_prot_t access_type,
	vm_fault_t fault_type,
	vm_page_t *rpp)
{
	kern_return_t ret;
	vm_prot_t prot[VP_PAGELIST];
	vm_page_t plist[VP_PAGELIST+1], *pl;
	register vm_page_t pp;
	register vm_prot_t *pprot;
	register vm_offset_t addr;
	register vm_offset_t offset, end;
	register int i;
	register struct vpage *vp;
	register struct vm_vp_object *vop;
	vm_offset_t target_addr;
	int ivp, rw;
	vm_size_t psize;
	int pcnt;
	extern int ubc_pages;

	if (fault_type == VM_UNWIRE) return u_vp_unwire(ep, va, size);

	if (ep->vme_private) {
		vp = (struct vpage *) (ep->vme_private) +
			atop(va - ep->vme_start);
		ivp = 1;
	}
	else {
		vp = &ep->vme_vpage;
		ivp = 0;
	}


	/*
	 * Validate the access to the region.
	 */

	if (ivp) {
		for(i = atop(round_page(size)); i;) {
			i--;
			if ((access_type & (vp + i)->vp_prot) != access_type)
				return KERN_PROTECTION_FAILURE;
		}
	}
	else if ((vp->vp_prot & access_type) != access_type)
		return KERN_PROTECTION_FAILURE;

	vop = (struct vm_vp_object *) (ep->vme_object);

	offset = ep->vme_offset + (va - ep->vme_start);
	end = offset + size;

	switch (access_type) {
	case VM_PROT_READ :
	case VM_PROT_EXECUTE :
	case VM_PROT_READ|VM_PROT_EXECUTE :
		rw = 1;
		break;
	default:
		rw = 0;
		break;
	}

	if(fault_type == VM_WIRE) {
	     pcnt = 1;
	     psize = PAGE_SIZE;
	     target_addr = addr + PAGE_SIZE;
	}
	else {
	     pcnt = (fault_type == VM_PAGEGET) ? 1 : VP_PAGELIST;
	     if (size > VP_PAGELISTSIZE) 
	           psize =  VP_PAGELISTSIZE;
	     else psize = size;
	}

	for (addr = va; offset < end; offset += psize) {
		plist[0] = VM_PAGE_NULL;

		if(fault_type == VM_WIRE)
		        target_addr = addr + PAGE_SIZE;

		if (rw) vpf_ladd(pgioreads,1);
		VOP_GETPAGE(vop->vo_vp, offset,
			psize, prot, plist,
			pcnt, ep, addr, rw, u.u_cred, ret);
		if (ret) goto failed;
		for (pprot = prot, pl = plist, pp = *pl;
			pp != VM_PAGE_NULL; pl++, pprot++, pp = *pl) {
			vm_page_wait(pp);
			if (pp->pg_error) {
				ret = KERN_FAILURE;
				goto failed;
			}
			if (pp->pg_offset >= offset &&
				pp->pg_offset < offset + psize) {

				if (fault_type == VM_PAGEGET &&
					pp->pg_offset == offset) {
					*rpp = pp;
				}
				else if (fault_type == VM_WIRE) {
					if (!vp->vp_plock) {
						if (wire_check(pp)){
							ret =
							KERN_RESOURCE_SHORTAGE;
							goto failed;
						}
						pmap_pageable(
						vm_map_pmap(ep->vme_map),
						addr, addr + PAGE_SIZE, FALSE);
						pmap_enter(
						vm_map_pmap(ep->vme_map),
						addr, page_to_phys(pp),
						vp->vp_prot & ~(*pprot), TRUE,
						access_type);
						ubc_wire(pl, 1, TRUE);
					}
					else ubc_page_release(pp, 0);
				}
				else {
					pmap_enter(vm_map_pmap(ep->vme_map),
						addr, page_to_phys(pp),
						vp->vp_prot & ~(*pprot), FALSE,
						access_type);
					ubc_page_release(pp, 0);
				}
				if (ivp) vp++;
				addr += PAGE_SIZE;
			}
			else ubc_page_release(pp, 0);
			*pl = VM_PAGE_EMPTY;
		}
		if (fault_type == VM_WIRE && addr != target_addr)
		        goto failed;
	}

	return 0;

	/*
	 * Release potentially held pages.
	 */

failed:
	pl = plist;
	while (*pl != VM_PAGE_NULL) {
		pp = *pl;
		if (pp != VM_PAGE_EMPTY) {
			vm_page_wait(pp);
			ubc_page_release(pp, 0);
		}
		*pl++ = VM_PAGE_EMPTY;
	}
	if (fault_type == VM_WIRE) u_vp_unwire(ep, va, addr - va);
	return ret;
}

u_vp_unwire(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t size)
{
	register vm_offset_t offset, end;
	register struct vpage *vp;
	vm_page_t pp;
	register int vpi;
	register struct vm_vp_object *vop;

	offset = ep->vme_offset + (addr - ep->vme_start);
	end = offset + size;

	if (ep->vme_private) {
		vpi = 1;
		vp = (struct vpage *) (ep->vme_private) +
			atop(addr - ep->vme_start);
	}
	else {
		vpi = 0;
		vp = &ep->vme_vpage;
	}
	vop = (struct vm_vp_object *) (ep->vme_object);

	for (; offset < end;
		offset += PAGE_SIZE, vp += vpi, addr += PAGE_SIZE) {
		if (vp->vp_plock) continue;
		ubc_page_lookup(vop, offset, &pp);
		if (pp == VM_PAGE_NULL) panic("u_vp_unwire: page gone");
		pmap_change_wiring(vm_map_pmap(ep->vme_map), addr, FALSE);
		ubc_unwire(&pp, 1);
	}
	return 0;
}

u_vp_dup(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	register vm_map_entry_t newep,
	vm_copy_t copy)
{
	register struct vm_vp_object *vop;
	kern_return_t ret;

	switch (copy) {
	case VM_COPYMCOPY:
		return u_vp_dup_copy(ep, addr, size, newep);
	case VM_COPYU:
		break;
	default:
		return KERN_INVALID_ARGUMENT;
	}

	vop = (struct vm_vp_object *) (ep->vme_object);
	VREF(vop->vo_vp);
	return 0;
}

u_vp_dup_copy(register vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t size,
	vm_map_entry_t newep)
{
	register struct vm_vp_object *vop;
	struct vm_anon_object *aop;
	vm_offset_t offset;
	struct vm_uvp_object *obj;
	kern_return_t ret;

	if ((ret = vm_object_allocate(OT_ANON, size,
		(caddr_t) 0, (vm_object_t *) &aop)) != KERN_SUCCESS) return ret;

	if (a_reserve(aop, atop(size)) == FALSE) {
		vm_object_deallocate((vm_object_t) aop);
		return KERN_RESOURCE_SHORTAGE;
	}


	offset = ep->vme_offset + (addr - ep->vme_start);
	vop = (struct vm_vp_object *) (ep->vme_object);
	VREF(vop->vo_vp);
	aop->ao_flags =  AF_PRIVATE;
	aop->ao_bobject = (vm_object_t) vop;
	aop->ao_boffset = offset;
	newep->vme_ops = vm_object_to_vmeops(aop);
	newep->vme_offset = (vm_offset_t) 0;
	newep->vme_object = (vm_object_t) aop;
	return 0;
}

u_vp_unmap(register vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t len)
{
	if (!u_map_entry_delete_check(ep->vme_map, ep, addr, addr + len))
		return KERN_RESOURCE_SHORTAGE;
	u_vp_lockop(ep, addr, len, VM_UNWIRE);
	u_map_entry_unmap(ep, addr, len);
	pmap_remove(vm_map_pmap(ep->vme_map), addr, addr + len);
	return 0;
}

/*
 * vnode msync function
 * don't invalidate a locked page
 */

u_vp_msync(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	int flags)
{
	struct vm_vp_object *vop;
	register vm_offset_t offset, end, start;
	register struct vpage *vp;
	int vpi;
	int bflags;

	vop = (struct vm_vp_object *)(ep->vme_object);
	offset = ep->vme_offset + (addr - ep->vme_start);
	end = offset + size;

	if (ep->vme_private) {
		vpi = 1;
		vp = (struct vpage *) (ep->vme_private) +
			atop(addr - ep->vme_start);
	}
	else {
		vpi = 0;
		vp = &ep->vme_vpage;
	}

	bflags = (flags & MS_SYNC) ? 0 : B_ASYNC;
	bflags |= (flags & MS_INVALIDATE) ? B_FREE : 0;

	if (vpi == 0) {
		if (vp->vp_plock && bflags & B_FREE)
			return KERN_PAGELOCKED;
	} else for (start = offset; start < end; vp += vpi, start += PAGE_SIZE)
		if (vp->vp_plock  && bflags & B_FREE) return KERN_PAGELOCKED;

	return ubc_msync(vop->vo_vp, offset, round_page(size), bflags);
}

u_vp_lockop(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t len,
	vm_fault_t wire)
{
	register vm_offset_t start, end;
	register struct vpage *vp;
	register int vpi;
	kern_return_t ret;


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
		vp += atop(va - ep->vme_start);
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
			ret = u_vp_fault(ep, start, 1,
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
				u_vp_fault(ep, start, 1,
				vp->vp_prot, wire, (vm_page_t *) 0);
		}
	}
	return ret;
}

u_vp_swap(register vm_map_entry_t ep, int rw)
{
	/*
	 * The only thing we can do is to
	 * remove the translations.
	 */

	if (!rw)
		pmap_remove(vm_map_pmap(ep->vme_map),
			ep->vme_start, ep->vme_end);
	return 0;
}

/*
 * For now don't produce core dump information.
 * What should the real policy be ?
 */

u_vp_core(register vm_map_entry_t ep,
	register unsigned next,
	register char *vec,
	register int *vecsize)
{
	*vecsize = -1;
	return 0;
}

u_vp_control(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register vm_size_t size,
	vm_control_t control,
	int arg)
{
	kern_return_t ret;

	switch (control) {
	case VMC_INHERITANCE:
		if (u_map_entry_clip_check(ep->vme_map, ep,
			addr, addr + size) == FALSE)
				return KERN_RESOURCE_SHORTAGE;
		ep->vme_inheritance = arg;
		break;
	case VMC_KEEP_ON_EXEC:
		if (u_map_entry_clip_check(ep->vme_map, ep,
			addr, addr + size) == FALSE)
				return KERN_RESOURCE_SHORTAGE;
		ep->vme_keep_on_exec = arg;
		break;
	case VMC_SEM_WAKEUP:
	case VMC_SEM_SLEEP:
	{
		register struct vm_vp_object *vop;

		vop = (struct vm_vp_object *) (ep->vme_object);
		OOP_REFERENCE(vop);
		vm_mape_faultdone(ep);
		ret = msmem_control((vm_object_t) vop, addr, control);
		OOP_DEALLOCATE(vop);
		return ret;
	}
	default:
		return KERN_INVALID_ARGUMENT;
	}

	return u_map_entry_split(ep, addr, size);
	return KERN_SUCCESS;
}

u_vp_protect(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t size,
	vm_prot_t prot)
{
	register struct vpage *vp;
	kern_return_t ret;
	register vm_offset_t end;

	vp = (struct vpage *) (ep->vme_private);

	if (!vp) {
		if (ep->vme_start != addr || ep->vme_end != (addr + size)) {
			if (ret = u_vpage_init(ep, FALSE)) return ret;
			vp = (struct vpage *) (ep->vme_private);
		}
		else if (prot == ep->vme_protection) return 0;
		else {
			pmap_protect(vm_map_pmap(ep->vme_map),
					ep->vme_start, ep->vme_end, prot);
			ep->vme_protection = prot;
			return 0;
		}
	}

	vp += atop(addr - ep->vme_start);
	for (end = addr + size ; addr < end; addr += PAGE_SIZE, vp++)
		if (vp->vp_prot != prot) {
			pmap_protect(vm_map_pmap(ep->vme_map),
				addr, addr + PAGE_SIZE, prot);
			vp->vp_prot = prot;
		}
	return 0;
}

u_vp_check_protect(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register vm_size_t size,
	register vm_prot_t prot)
{
	register struct vpage *vp;

	vp = (struct vpage *) (ep->vme_private);
	if (!vp) {
		if ((ep->vme_vpage.vp_prot & prot) != prot) return FALSE;
		else return TRUE;
	}
	vp += atop (addr - ep->vme_start);
	for (;size; size -= PAGE_SIZE, vp++)
		if ((vp->vp_prot & prot) != prot) return FALSE;
	return TRUE;
}

u_vp_kluster(register vm_map_entry_t ep,
	register vm_offset_t addr,
	int pcnt,
	vm_offset_t *back,
	vm_offset_t *forward)
{
	register vm_offset_t offset, noffset;
	register vm_size_t regsize;
	int cluster;

	offset = ep->vme_offset + (addr - ep->vme_start);
	addr = trunc_page(addr);

	cluster = vm_page_free_count - vm_page_kluster;
	if (cluster <= 0 || (pcnt = MIN(pcnt, cluster)) == 1) return 1;
	regsize = ptoa(pcnt - 1);

	noffset  = MIN(regsize, (ep->vme_end - (addr + PAGE_SIZE)));
	if (noffset) {
		*forward = offset + noffset;
		noffset = regsize - noffset;
	}
	else {
		noffset = regsize;
		*forward = offset;
	}

	if (noffset) {
		noffset = MIN((addr - ep->vme_start), noffset);
		*back = offset - noffset;
	}
	else *back = offset;
	return 0;
}

u_vp_copy(vm_map_entry_t ep, vm_copy_op_t op)
{
	panic("u_vp_copyout: illegal operation");
}

u_vp_grow(vm_map_entry_t ep,
	vm_prot_t prot,
	register vm_size_t size,
	as_grow_t direction)
{
	if (ep->vme_private		/* Don't add to a complicated entry. */
	    || ep->vme_plock		/* Don't add to a locked entry. */
	    || (ep->vme_protection != prot)) /* Need a different protection. */
		return KERN_NO_SPACE;

	if (direction == AS_GROWUP)
		ep->vme_end += size;
	else {
		ep->vme_start -= size;
		ep->vme_offset -= size;
	}
	return KERN_SUCCESS;
}

/*
 * For vnode object management
 */

u_vp_oop_badop()
{
	panic("u_vp_oop_badop: vnode object operation not supported");
}

kern_return_t
u_vp_oop_pagesteal()
{
        return KERN_FAILURE;
}


extern int
	u_vp_oop_reference(),
	u_vp_oop_deallocate(),
	u_vp_oop_pagein(),
	u_vp_oop_control(),
	u_vp_oop_pagecontrol();

/*
 * For vnode mmapers.
 */

struct vm_object_ops u_vp_oop = {
	OOP_KLOCK_TRY_K
	&u_vp_oop_badop,		/* lock try */
	OOP_UNKLOCK_K
	&u_vp_oop_badop,		/* unlock */
	&u_vp_oop_reference,		/* reference */
	&u_vp_oop_deallocate,		/* deallocate */
	&u_vp_oop_pagein,		/* pagein */
	OOP_PAGEOUT_K
	&u_vp_oop_badop,		/* pageout */
	OOP_SWAP_K
	&u_vp_oop_badop,		/* swapout */
	&u_vp_oop_control,		/* control */
	&u_vp_oop_pagecontrol,		/* page control */
	&u_vp_oop_pagesteal,		/* page_steal */
};


struct vm_object_config vp_object_conf = {
	(int (*)()) 0,
	(int (*)()) 0,
	sizeof (struct vm_vp_object),
	&u_vp_oop,
	&u_vp_mape_ops,
};

u_vp_oop_reference(register struct vm_vp_object *vop)
{
	VREF(vop->vo_vp);
}

u_vp_oop_deallocate(register struct vm_vp_object *vop)
{
	vrele(vop->vo_vp);
}

/*
 * The first prot element has the desired
 * access for the entire operation.
 */

u_vp_oop_pagein(register struct vm_vp_object *vop,
	vm_offset_t offset,
	vm_size_t size,
	vm_map_entry_t ep,
	vm_offset_t addr,
	vm_page_t *ppl,
	int pcnt,
	vm_prot_t *prot)
{
	int rw, status;


	if (*prot & VM_PROT_WRITE) rw = 0;
	else {
		vpf_ladd(pgioreads,1);
		rw = 1;
	}

	VOP_GETPAGE(vop->vo_vp, offset, size, prot, ppl,
		pcnt, ep, addr, rw, u.u_cred, status);
	return status;
}

u_vp_oop_control(register struct vm_vp_object *vop,
	vm_offset_t offset,
	int pcnt,
	vm_ocontrol_t op,
	int flags)
{
	switch (op) {
	case VMOC_NOOP:
	case VMOC_FREE:
		return 0;
	case VMOC_MSYNC:
	{
		int bflags;

		bflags = (flags & MS_SYNC)? 0: B_ASYNC;
		bflags |= (flags & MS_INVALIDATE)? B_FREE: 0;
		return ubc_msync(vop->vo_vp, offset, ptoa(pcnt), bflags);
	}
	case VMOC_PAGEUNLOCK:
	{
		vm_page_t pp;
		int uflag;
		register int count;

		for (uflag = B_CACHE, count = pcnt; count--;
			offset += PAGE_SIZE, uflag = B_CACHE) {
			ubc_page_lookup(vop, offset, &pp);
			if (uflag & B_NOCACHE)
				panic("u_vp_control: page not found ");
			ubc_unwire(&pp, 1);
		}
		return 0;
	}
	}
}

u_vp_oop_pagecontrol(register struct vm_uvp_object *vop,
	register vm_page_t *ppl,
	register int pcnt,
	int flags)
{
	switch (flags) {
	case VMOP_LOCK|VMOP_RELEASE:
		ubc_wire(ppl, pcnt, TRUE);
		break;
	case VMOP_LOCK:
		ubc_wire(ppl, pcnt, FALSE);
		break;
	case VMOP_RELEASE:
		while (pcnt--) ubc_page_release(*ppl++, 0);
		break;
	default:
		panic("u_vp_pagecontrol: illegal operation");
	}
	return 0;
}

/*
 * Place holder for now.
 */

extern struct vm_object_ops null_object_oops;
extern struct vm_map_entry_ops k_mape_ops_invalid;

struct vm_object_config vpmap_object_conf = {
	(int (*)()) 0,
	(int (*)()) 0,
	sizeof (struct vm_uvp_object),
	&null_object_oops,
	&k_mape_ops_invalid,
};

/*
 * Check if entry is backed by the given vnode.  Returns non-zero if vnode
 * is the backing object of entry.  Used by core() to avoid potential
 * deadlock.
 */
int
vp_is_bobject(me, vp)
struct vm_map_entry *me;
struct vnode	    *vp;
{
	struct vm_object *obj = me->vme_object;

	while(obj) {
		switch(vm_object_type(obj)) {
		case OT_ANON:
		case OT_SHM:
			obj = ((struct vm_anon_object *)obj)->ao_bobject;
			break;
		case OT_SEG:
			obj = (struct vm_object *)me->vme_seg->seg_vop;
			break;
		case OT_VP:
			return (((struct vm_vp_object *)obj)->vo_vp == vp);
		default:
			obj = VM_OBJECT_NULL;
			break;
		}
	}
	return 0;
}
