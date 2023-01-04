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
static char *rcsid = "@(#)$RCSfile: u_mape_anon.c,v $ $Revision: 1.1.31.11 $ (DEC) $Date: 1993/12/08 22:31:32 $";
#endif
/*
 * Supports anonymous memory mapped regions for U & C maps
 */

#include <kern/assert.h>
#include <mach_ldebug.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <kern/zalloc.h>
#include <sys/buf.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <vm/vm_anon.h>
#include <vm/vm_swap.h>
#include <vm/vm_fault.h>
#include <vm/vm_control.h>
#include <vm/vm_tune.h>
#include <vm/vm_umap.h>
#include <vm/vm_vlock.h>
#include <vm/vm_vp.h>
#include <vm/vm_mmap.h>
#include <vm/vm_umap.h>
#include <vm/pmap.h>
#include <vm/heap_kmem.h>
#include <vm/vm_perf.h>
#include <vm/vm_pagelru.h>

extern int
	u_anon_fault(),
	u_anon_dup(),
	u_anon_unmap(),
	u_anon_msync(),
	u_anon_lockop(),
	u_anon_swap(),
	u_anon_core(),
	u_anon_control(),
	u_anon_protect(),
	u_anon_check_protect(),
	u_anon_kluster(),
	u_anon_copy(),
	u_anon_grow();

struct vm_map_entry_ops u_mape_anon_ops = {
	&u_anon_fault,		/* fault */
	&u_anon_dup,		/* dup */
	&u_anon_unmap,		/* unmap */
	&u_anon_msync,		/* msync */
	&u_anon_lockop,		/* lockop */
	&u_anon_swap,		/* swap */
	&u_anon_core,		/* corefile */
	&u_anon_control,	/* control */
	&u_anon_protect,	/* protect */
	&u_anon_check_protect,	/* check protection */
	&u_anon_kluster,	/* kluster */
	&u_anon_copy,		/* copy */
	&u_anon_grow,		/* grow */
};

u_anon_allocate(vm_size_t size, boolean_t shared, vm_object_t *obj)
{
	struct vm_anon_object *aop;
	kern_return_t ret;

	size = round_page(size);
	if ((ret = vm_object_allocate(OT_ANON, size,
		FALSE, (vm_object_t *) &aop)) != KERN_SUCCESS) return ret;

	aop->ao_flags |= (shared == FALSE) ? AF_PRIVATE : AF_SHARED;

	if (a_reserve(aop, atop(size)) == FALSE) {
		vm_object_deallocate(aop);
		return KERN_RESOURCE_SHORTAGE;
	}
	*obj = (vm_object_t) aop;
	return 0;
}

/*
 * If a backing object (obj) has been specified then
 * it is assumed a reference to it has already been taken.
 */

int
u_anon_create(vm_map_t map, vm_object_t obj, vm_offset_t args)
{
        register struct vp_mmap_args *ap = (struct vp_mmap_args *) args;
        struct vm_anon_object *aop;
        kern_return_t ret;

        /*
         * If an object is backing us then
         * the mapping has to be private.
         */

        if (obj && (ap->a_flags & MAP_PRIVATE) == 0) {
                ret = KERN_INVALID_ARGUMENT;
                goto delback;
        }

        if (ap->a_flags & MAP_FIXED) {
                vm_map_lock(map);
                ret = (*map->vm_delete_map)(map, *(ap->a_vaddr),
                        *(ap->a_vaddr) + ap->a_size, FALSE);
                vm_map_unlock(map);
                if (ret != KERN_SUCCESS) goto delback;
        }

        if (obj || (ap->a_flags & MAP_SHARED)) {
		register struct u_map_private *up = (struct u_map_private *) (map->vm_private);

		if ((ap->a_size + map->vm_size) > up->um_maxvas) {
			ret = KERN_RESOURCE_SHORTAGE;
			goto delback;
		}
                if ((ret = vm_object_allocate(OT_ANON, ap->a_size,
                        (caddr_t) ((obj && !(ap->a_prot & VM_PROT_WRITE) &&
                        (ap->a_flags & MAP_PRIVATE)) ? TRUE : FALSE),
                        (vm_object_t *) &aop)) != KERN_SUCCESS) goto delback;
                aop->ao_flags |=
                        (ap->a_flags & MAP_PRIVATE) ? AF_PRIVATE : AF_SHARED;
                if (obj) {
                        aop->ao_bobject = obj;
                        aop->ao_boffset = ap->a_offset;
                }

                /*
                 * If the backing object is potentially going to
                 * be written to then we reserve the anon space up
                 * front assuming we'll be doing COW very soon.
                 * For normal anon we also reserve now.
                 */

                if ((obj == VM_OBJECT_NULL) || (ap->a_prot & VM_PROT_WRITE)) {
                        if (a_reserve(aop, atop(ap->a_size)) == FALSE) {
                                vm_object_deallocate(aop);
                                return KERN_RESOURCE_SHORTAGE;
                        }
                }
        }
        else aop = (struct vm_anon_object *) 0;


        /*
         * Map the anon in.
         */

        ret = u_map_enter(map, ap->a_vaddr, ap->a_size, page_mask,
                (ap->a_flags & MAP_FIXED) ? FALSE : TRUE,
                aop, (vm_offset_t) 0, (ap->a_flags & MAP_SHARED) ? FALSE : TRUE,
                ap->a_prot, ap->a_maxprot,  VM_INHERIT_COPY);

        return ret;
delback:
        if (obj) vm_object_deallocate(obj);
        return ret;
}


/*
 * General fault handler for anonymous memory.
 */

kern_return_t
u_anon_fault(register vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t size,
	vm_prot_t access_type,
	vm_fault_t fault_type,
	vm_page_t *rpp)
{
	vm_page_t rp;
	register struct vpage *vp;
	struct vm_anon_object *aop;
	register struct vm_anon *ap;
	struct vm_anon **app;
	register vm_offset_t offset, klend, addr;
	alock_t lp;
	kern_return_t ret;
	int ivp;
	register int npages, i;
	int pcnt;
	vm_page_t plist[ANON_KLPAGES+1], *pp;
	vm_prot_t prot[ANON_KLPAGES];
	vm_size_t len;

	if ((va + size) > ep->vme_end) return KERN_INVALID_ADDRESS;

	if (fault_type == VM_UNWIRE) return u_anon_unwire(ep, va, size);

	aop = (struct vm_anon_object *) (ep->vme_object);

	if (ep->vme_private) {
		vp = (struct vpage *) (ep->vme_private) +
				atop(va - ep->vme_start);
		ivp = 1;
	}
	else {
		vp = &ep->vme_vpage;
		ivp = 0;
	}

	len = round_page(size);

	/*
	 * Validate the access to the region.
	 */

	for(npages = atop(len), i = 0; npages; npages--) {
		if ((access_type & (vp + i)->vp_prot) != access_type)
			return KERN_PROTECTION_FAILURE;
		i += ivp;
	}

	offset = ep->vme_offset + (va - ep->vme_start);
	npages = atop(len);
	addr = va;


	/*
	 * Everything above is guaranteed to be invariant
	 * because of the upper level locking strategy.
	 * Compute the first lock pointer.
	 */

	if (aop->ao_anon) {
		lp = aop->ao_klock + anon_kl(offset);
		klend = anon_klround(offset + PAGE_SIZE);
		lk_mlock(lp);
	}
	else lp = (alock_t) 0;

	while (npages) {

		pp = plist;
		*pp = VM_PAGE_NULL;
		if (fault_type == VM_WIRE)
			pcnt = MIN(npages, ANON_KLPAGES);
		else pcnt = ANON_KLPAGES;

		/*
		 * If we have an anon pointer
		 */

		if (aop->ao_anon &&
			(ap = *(app = aop->ao_anon + atop(offset)))) {

			ret = a_anon_getpage(aop, offset, ep,
					addr, lp, pp, pcnt, ap);
			if (ret != KERN_SUCCESS) goto failed;
		}

		/*
		 * Backed by another object.
		 */

		else if (aop->ao_bobject) {

			app = (struct vm_anon **) 0;
			prot[0] = VM_PROT_READ;
			ret = OOP_PAGEIN(aop->ao_bobject,
				aop->ao_boffset + offset, size,
				ep, addr, pp, pcnt, prot);
			if (ret != KERN_SUCCESS) goto failed;
		}
		else pcnt = 1;


		/*
		 * All pages of current interest are secure.
		 * More might be held but these are simply released.
		 */

		for (i = 0; i < pcnt && npages; i++) {
			 ret = u_anon_faultpage(ep, addr, offset, app,
				fault_type, (fault_type == VM_WIRE) ?
					vp->vp_prot : access_type,
					vp, plist, prot, &rp, lp);


			if (ret != KERN_SUCCESS) goto failed;
			else if (rp == VM_PAGE_NULL)
				if (i) break;
				else {
					ret = KERN_INVALID_ADDRESS;
					goto failed;
				}
			else if (fault_type == VM_PAGEGET) break;
			else npages--;

			size -= PAGE_SIZE;
			addr += PAGE_SIZE;
			offset += PAGE_SIZE;
			vp += ivp;
		}

		/*
		 * Release the remaining pages brought in
		 * that we don't want.  Avoid releasing
		 * the VM_PAGEGET page.
		 */

		pp = plist;
		while (*pp != VM_PAGE_NULL) {
			if (*pp != VM_PAGE_EMPTY &&
				(fault_type != VM_PAGEGET || rp != *pp)) {
				if (app) vm_page_release(*pp);
				else OOP_PAGECTL(aop->ao_bobject, pp,
					1, VMOP_RELEASE);
			}
			pp++;
		}

		assert(fault_type == VM_PAGEGET && rp != VM_PAGE_NULL);

		if (fault_type == VM_PAGEGET) {
			*rpp = rp;
			break;
		}

		/*
		 * At the end of this cluster.
		 */

		if (lp && npages && (offset == klend)) {
			lk_mpunlock(lp,m,p);
			lp++;
			lk_mlock(lp);
			klend += anon_klsize;
		}
	}

	if (lp) lk_mpunlock(lp,m,p);
	return KERN_SUCCESS;
failed:
	pp = plist;
	while (*pp != VM_PAGE_NULL) {
		if (*pp != VM_PAGE_EMPTY) {
			if (app) vm_page_release(*pp);
			else OOP_PAGECTL(aop->ao_bobject, pp, 1, VMOP_RELEASE);
		}
		pp++;
	}

	if (lp) lk_mpunlock(lp,m,p);

	/*
	 * If wiring we must unwire all pages already wired.
	 */

	if (fault_type == VM_WIRE) u_anon_unwire(ep, va, (addr - va));
	return ret;
}

/*
 * Do the rest of this page fault.
 * The faults intended access has already
 * been verified as valid.  Also protection
 * changes can't be made while faulting because
 * of the upper level locking strategy.
 * Write to backing object are always COW conditions.
 * Write to anonymous memory with a refcnt > 1 are COW.
 */

int
u_anon_faultpage(vm_map_entry_t ep,
		vm_offset_t addr,
		vm_offset_t offset,
		struct vm_anon **app,
		vm_fault_t fault_type,
		vm_prot_t access,
		struct vpage *vp,
		register vm_page_t *pp,
		vm_prot_t *pprot,
		register vm_page_t *rp,
		alock_t lp)
{
	struct vm_anon_object *aop;
	register struct vm_anon *ap, *newap;
	vm_prot_t prot;
	int ret;
	extern int ubc_pages;

	aop = (struct vm_anon_object *) (ep->vme_object);
	prot = 0;
	newap = (struct vm_anon *) 0;

	if (app && !(ap = *app)) {
		ap = a_anon_pagezero_alloc(aop, offset, lp);
		pp[0] = VM_PAGE_EMPTY;
		pp[1] = VM_PAGE_NULL;
		*rp = ap->an_page;
	}

	/*
	 * Page came from anon.
	 */


	else if (app && ap) {
		register simple_lock_t alp;

		alp = a_lockaddr(ap);

		while (*pp != VM_PAGE_NULL) {
			if (*pp != VM_PAGE_EMPTY && *pp == ap->an_page) {
				*pp = VM_PAGE_EMPTY;
				break;
			}
			pp++;
		}

		if (*pp == VM_PAGE_NULL) {
			*rp = VM_PAGE_NULL;
			return 0;
		}

		vm_page_wait(ap->an_page);

                if (ap->an_page->pg_error) {
			vm_page_release(ap->an_page);
                        return KERN_FAILURE;
                }

		a_locklp(alp);
		if ((ap->an_refcnt > 1) &&
			((access & VM_PROT_WRITE) || fault_type == VM_WIRE)) {

			vpf_ladd(cowfaults,1);
			newap = a_anon_cowpage_alloc(aop, offset, lp, ap, app);
			if (newap != (struct vm_anon *) 0) {


				if (aop->ao_flags & AF_SHARED)
					pmap_page_protect(
						page_to_phys(ap->an_page),
						VM_PROT_NONE);
					
				vm_page_copy(ap->an_page, newap->an_page);
				if (ap->an_cowfaults < ANON_COWMAX)
					ap->an_cowfaults++;
				ap->an_refcnt--;
				*rp = newap->an_page;
				vm_page_release(ap->an_page);
				vm_pageout_activate(newap->an_page, FALSE);
			}
			else *rp = ap->an_page;
			a_unlocklp(alp);
		}
		else {
			if (ap->an_refcnt > 1) prot = VM_PROT_WRITE;
			*rp = ap->an_page;
			a_unlocklp(alp);
		}
	}

	/*
	 * Page list is from backing object
	 */

	else {
		vm_page_t *phead;

		phead = pp;

		while (*pp != VM_PAGE_NULL)
			if (*pp != VM_PAGE_EMPTY &&
			(*pp)->pg_offset == (offset + aop->ao_boffset)) break;
			else pp++;

		if (*pp == VM_PAGE_NULL) {
			*rp = VM_PAGE_NULL;
			return 0;
		}

		vm_page_wait(*pp);

		if ((*pp)->pg_error) {
			OOP_PAGECTL(aop->ao_bobject, pp, 1, VMOP_RELEASE);
			pp[0] = VM_PAGE_EMPTY;
			return KERN_FAILURE;
		}

		if (access & VM_PROT_WRITE) {
			vpf_ladd(cowfaults,1);
			app = aop->ao_anon + atop(offset);
			newap = a_anon_appage_alloc(aop, offset, lp, app);
			vm_page_copy(*pp, newap->an_page);
			OOP_PAGECTL(aop->ao_bobject, pp, 1, VMOP_RELEASE);
			*rp = newap->an_page;
		}
		else {
			*rp = *pp;
			prot = pprot[pp - phead] | VM_PROT_WRITE;
		}
		*pp = VM_PAGE_EMPTY;

		
	}

	if (fault_type != VM_PAGEGET) {
		if (fault_type == VM_WIRE && !vp->vp_plock) {
			if (wire_check(*rp)){
				if (!app) OOP_PAGECTL(aop->ao_bobject,
					rp, 1, VMOP_RELEASE);
				else vm_page_release(*rp);
				*rp = VM_PAGE_EMPTY;
				return KERN_RESOURCE_SHORTAGE;
			}
			pmap_pageable(vm_map_pmap(ep->vme_map),
				addr, addr + PAGE_SIZE, FALSE);
			pmap_enter(vm_map_pmap(ep->vme_map),
				addr, page_to_phys(*rp),
				vp->vp_prot & ~prot, TRUE, access);
			if (!app) OOP_PAGECTL(aop->ao_bobject,
					rp, 1, VMOP_RELEASE|VMOP_LOCK);
			else vm_page_wire(*rp, TRUE);
		}
		else {
			if (!vp->vp_plock)
				pmap_enter(vm_map_pmap(ep->vme_map),
					addr, page_to_phys(*rp),
					vp->vp_prot & ~prot, FALSE, access);
			if (!app) OOP_PAGECTL(aop->ao_bobject,
					rp, 1, VMOP_RELEASE);
			else vm_page_release(*rp);
		}
	}
	return 0;
}

/*
 * Unwire anon memory.
 * Any page with the vp_plock set is left alone.
 */

u_anon_unwire(register vm_map_entry_t ep,
	register vm_offset_t va,
	register vm_size_t size)
{
	register vm_offset_t offset, end, klend;
	struct vm_anon_object *aop;
	register struct vpage *vp;
	register struct vm_anon *ap;
	vm_offset_t start;
	alock_t lp;
	int ivp;

	aop = (struct vm_anon_object *)(ep->vme_object);
	offset = ep->vme_offset + (va - ep->vme_start);	

	if (ep->vme_private) {
		vp = (struct vpage *)(ep->vme_private) +
			atop(va - ep->vme_start);
		ivp = 1;
	}
	else {
		vp = &ep->vme_vpage;
		ivp = 0;
	}

	if (aop->ao_anon) {
		lp = aop->ao_klock + anon_kl(offset);
		klend = anon_klround(offset + PAGE_SIZE);
		lk_lock(lp);
	}
	else lp = (alock_t) 0;

	for (end = va + size; va < end;
			va += PAGE_SIZE, offset += PAGE_SIZE, vp += ivp) {
		if (lp && (offset == klend)) {
			lk_unlock(lp);
			lp++;
			lk_lock(lp);
			klend += anon_klsize;
		}

		if (vp->vp_plock) continue;


		/*
		 * Wired private memory.
		 */

		if (aop->ao_anon && (ap = *(aop->ao_anon + atop(offset)))) {
			a_lock(ap);
			vm_page_unwire(ap->an_page, TRUE);
			a_unlock(ap);
		}

		/*
		 * Wired backed memory.
		 */

		else if (aop->ao_bobject) {
			OOP_CONTROL(aop->ao_bobject,
				aop->ao_boffset + offset,
				1, VMOC_PAGEUNLOCK, 0);
		}

		pmap_change_wiring(vm_map_pmap(ep->vme_map), va, FALSE);
	}
	if(lp) lk_unlock(lp);
	pmap_pageable(ep->vme_map, start, start + size, TRUE);
	return 0;
}

	
/*
 * The address position indicates the type of pager
 * requesting the klustering.
 */

u_anon_kluster(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register int pcnt,
	vm_offset_t *back,
	vm_offset_t *forward)
{
	register struct vm_anon_object *aop;
	register struct vm_anon **app, **center, **first, **last;
	register vm_offset_t offset, kl, noffset, i;
	boolean_t nonanon;
	int cluster;

	aop = (struct vm_anon_object *)(ep->vme_object);
	addr = trunc_page(addr);
	offset = ep->vme_offset + (addr - ep->vme_start);

	cluster = vm_page_free_count - vm_page_kluster;
	if (cluster <= 0 || (pcnt = MIN(pcnt, cluster)) == 1 ||
	   (pcnt = MIN(pcnt, anon_klpages)) == 1) return 1;
	pcnt--;

	if (!aop->ao_anon) {
		register vm_size_t regsize;

		regsize = ptoa(pcnt);
		assert(aop->ao_bobject != VM_OBJECT_NULL);
		offset += aop->ao_boffset;

		noffset = MIN((ep->vme_end - (addr + PAGE_SIZE)), regsize);
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

	/*
	 * Lowest offset in kluster
	 */

	if ((kl = anon_kltrunc(offset)) < ep->vme_offset)
		first = aop->ao_anon + atop(ep->vme_offset);
	else
		first = aop->ao_anon + atop(kl);

	/*
	 * Highest offset in kluster
	 */

	if ((kl = anon_klround(offset + PAGE_SIZE)) >
		((i = ep->vme_offset + ep->vme_end - ep->vme_start)))
		last = aop->ao_anon + atop(i);
	else
		last = aop->ao_anon + atop(kl);

	center = aop->ao_anon + atop(offset);

	/*
	 * There is an anon pointer or a backing object
	 */


	if (*center || aop->ao_bobject) {
		register vm_offset_t boffset;

		if (*center) {
			nonanon = FALSE;
			boffset = (vm_offset_t) 0;
		}
		else {
			nonanon = TRUE;
			boffset = aop->ao_boffset;
		}

		for (noffset = offset, app = center + 1, i = pcnt;
			app < last && i; i--, app++, noffset += PAGE_SIZE)
			if (*app) {
				if (nonanon == FALSE) continue;
				else break;
			}
			else if (nonanon == TRUE) continue;
			else break;

		if (i != pcnt) *forward = noffset + boffset;
		else *forward = offset + boffset;

		pcnt = i;
		for (noffset = offset, app = center - 1;
			app >= first && i; i--, app--, noffset -= PAGE_SIZE)
			if (*app) {
				if (nonanon == FALSE) continue;
				else break;
			}
			else if (nonanon == TRUE) continue;
			else break;

		if (i != pcnt) *back = noffset + boffset;
		else *back = offset + boffset;
	}
	else *forward = *back = offset;

	return 0;
}

/*
 * Dup the address space
 */

u_anon_dup(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	register vm_map_entry_t newep,
	vm_copy_t copy)
{
	register struct vm_anon_object *aop;
	vm_offset_t offset;
	struct vm_anon_object *naop;
	kern_return_t ret;

	switch (copy) {
	case VM_COPYMCOPY:
	case VM_COPYU:
		break;
	default:
		return KERN_INVALID_ARGUMENT;
	}

	if ((addr + size) > ep->vme_end) return KERN_INVALID_ADDRESS;
	aop = (struct vm_anon_object *) (ep->vme_object);


	offset = ep->vme_offset + (addr - ep->vme_start);

	switch (copy) {
	case VM_COPYU:
		ret = u_anon_dupu(ep, addr, aop, offset, newep, &naop);
		break;
	case VM_COPYMCOPY:
		ret = u_anon_dupmcopy(ep, addr, aop, offset,
			size, newep, &naop);
		break;
	}

	newep->vme_object = (vm_object_t) naop;

	if (ret != KERN_SUCCESS) return ret;
	else if (aop->ao_bobject) {
		naop->ao_bobject = aop->ao_bobject;
		naop->ao_boffset = aop->ao_boffset + offset;
		OOP_REFERENCE(aop->ao_bobject);
	}
	return KERN_SUCCESS;
}

u_anon_dupu(vm_map_entry_t ep,
	vm_offset_t addr,
	struct vm_anon_object *aop,
	register vm_offset_t offset,
	vm_map_entry_t newep,
	struct vm_anon_object **raop)
{
	register struct vm_anon *ap;
	register vm_offset_t  end, noffset, kloffset, nkloffset;
	register struct vpage *vp;
	register struct vm_anon_object *nop;
	register struct vm_anon **app, **napp;
	register vm_page_t pp;
	register struct vm_vlock **vlpp;
	kern_return_t ret;
	alock_t lp, nlp;
	vm_offset_t tmp;
	vm_size_t size;
	int vpi;
	
	if (aop->ao_flags & AF_SHARED) {
		vm_object_lock(aop);
		aop->ao_refcnt++;
		aop->ao_rescnt++;
		vm_object_unlock(aop);
		*raop = aop;
		return KERN_SUCCESS;
	}

	size = ep->vme_end - ep->vme_start;

	newep->vme_offset = 0;
	if (!aop->ao_anon){
		if ((ret = vm_object_allocate(OT_ANON, size, TRUE, raop))
		== KERN_SUCCESS){
			nop = *raop;
			nop->ao_flags |= AF_PRIVATE;
		}
		return ret;
	}		
	else if ((ret = vm_object_allocate(OT_ANON, size, FALSE, raop))
		!= KERN_SUCCESS) return ret;
	else if (a_reserve(*raop, atop(size)) != TRUE) {
		return KERN_RESOURCE_SHORTAGE;
	}

	nop = *raop;
	nop->ao_flags |= AF_PRIVATE;

	if (ep->vme_private) {
		vp = (struct vpage *)(ep->vme_private);
		vpi = 1;
	}
	else {
		vp = &ep->vme_vpage;
		vpi = 0;
	}

	vlpp = &((struct u_map_private *)(ep->vme_map->vm_private))->um_vlock;
	app = aop->ao_anon + atop(offset);
	napp = nop->ao_anon;
	kloffset = anon_kltrunc(offset);
	end = offset + size;
	lp = aop->ao_klock + anon_kl(kloffset);
	kloffset += anon_klsize;
	nkloffset = anon_klsize;
	nlp = nop->ao_klock;
	lk_mlock(lp);
	lk_mlock(nlp);

	for (noffset = 0; offset < end; addr += PAGE_SIZE,
	noffset += PAGE_SIZE, offset += PAGE_SIZE, app++, napp++, vp += vpi) {

		if (offset >= kloffset) {
			lk_mpunlock(lp,m,p);
			lp++;
			lk_mlock(lp);
			kloffset += anon_klsize;	
		}

		if (noffset >= nkloffset) {
			lk_mpunlock(nlp,m,p);
			nlp++;
			lk_mlock(nlp);
			nkloffset += anon_klsize;
		}

		if (!lp->akl_anon) {
			if (kloffset >= end) break;
			tmp = (kloffset - offset) - PAGE_SIZE;
			addr += tmp;
			offset += tmp;
			noffset += tmp;
			tmp = atop(tmp);
			app += tmp;
			napp += tmp;
			if (vpi) vp += tmp;
			continue;
		}


		if (!(ap = *app)) continue;

		a_lock(ap);

		/*
		 * If the page is wired we can't do COW
		 */

		if (vp->vp_plock || (*vlpp &&
			vl_kwire(ep, addr, addr + PAGE_SIZE))
		        || lw_is_wired(ep, addr, addr + PAGE_SIZE)
		        || pmap_coproc_page_is_busy(vm_map_pmap(ep->vme_map),
						      addr, addr + PAGE_SIZE)) {
			register struct vm_anon *newap;

			pp = ap->an_page;
			if (pp == VM_PAGE_NULL) panic("u_anon_dupu: page gone");
			if (vm_page_hold(pp) == TRUE) {
				a_unlock(ap);
				vm_page_wait(pp);
                                if (pp->pg_error) {
                                        a_free(*raop, atop(size));
                                        vm_object_deallocate(*raop);
                                        return KERN_FAILURE;
                                }

			}
			else a_unlock(ap);
			newap = a_anon_appage_alloc(nop, noffset, nlp, napp);
			vm_page_copy(pp, newap->an_page);
			vm_page_release(pp);
			vm_page_release(newap->an_page);
			continue;
		}

		if (ap->an_page &&
			(ap->an_cowfaults > vm_tune_value(cowfaults))) {

			pp = ap->an_page;
			vm_page_lock(pp);
			if (!pp->pg_busy) {
				register struct vm_anon *newap;

				pp->pg_hold++;
				vm_page_unlock(pp);
				a_unlock(ap);
				vpf_ladd(cowsteals,1);
				newap = a_anon_appage_alloc(nop, noffset,
					nlp, napp);
				vm_page_copy(pp, newap->an_page);
				vm_page_release(pp);
				pmap_enter(vm_map_pmap(newep->vme_map),
					addr, page_to_phys(newap->an_page),
					vp->vp_prot, FALSE, vp->vp_prot);
				vm_page_release(newap->an_page);
				continue;
			}
			else vm_page_unlock(pp);
		}

		/*
		 * Must set up for COW
		 */

		ap->an_refcnt++;
		nlp->akl_anon++;
		if (ap->an_refcnt == 2 && (pp = ap->an_page) != VM_PAGE_NULL) {
			if (vm_page_hold(pp) == TRUE) {
				a_unlock(ap);
				vm_page_wait(pp);
                                if (pp->pg_error) {
                                        a_free(*raop, atop(size));
                                        vm_object_deallocate(*raop);
                                        return KERN_FAILURE;
                                }
			}
			else a_unlock(ap);
			pmap_copy_on_write(page_to_phys(pp));
			vm_page_release(pp);
		}
		else a_unlock(ap);
		*napp = ap;
	}
	lk_mpunlock(lp,m,p);
	lk_mpunlock(nlp,m,p);
	return KERN_SUCCESS;
}

u_anon_dupmcopy(vm_map_entry_t ep,
	vm_offset_t addr,
	struct vm_anon_object *aop,
	register vm_offset_t offset,
	vm_size_t size,
	vm_map_entry_t newep,
	struct vm_anon_object **raop)
{
	register struct vm_anon *ap;
	register vm_offset_t  end, noffset, kloffset, nkloffset;
	register struct vm_anon_object *nop;
	register struct vm_anon **app, **napp;
	register alock_t lp, nlp;
	register vm_page_t pp;
	kern_return_t ret;
	vm_offset_t tmp;
	

	newep->vme_ops = &u_mape_anon_ops;

	if ((ret = vm_object_allocate(OT_ANON, size, FALSE, raop))
		!= KERN_SUCCESS) return ret;
	else if (a_reserve(*raop, atop(size)) != TRUE) {
		vm_object_deallocate(*raop);
		return KERN_RESOURCE_SHORTAGE;
	}

	nop = *raop;
	nop->ao_flags |= AF_PRIVATE;
	if (!aop->ao_anon) return 0;

	end = offset + size;
	app = aop->ao_anon + atop(offset);
	napp = nop->ao_anon;
	kloffset = anon_kltrunc(offset);
	lp = aop->ao_klock + anon_kl(kloffset);
	nlp = nop->ao_klock;
	kloffset += anon_klsize;
	nkloffset = anon_klsize;
	lk_mlock(lp);
	lk_mlock(nlp);

	for (noffset = 0; offset < end; addr += PAGE_SIZE,
		noffset += PAGE_SIZE, offset += PAGE_SIZE, app++, napp++) {

		if (offset >= kloffset) {
			lk_mpunlock(lp,m,p);
			lp++;
			lk_mlock(lp);
			kloffset += anon_klsize;	
		}

		if (noffset >= nkloffset) {
			lk_mpunlock(nlp,m,p);
			nlp++;
			lk_mlock(nlp);
			nkloffset += anon_klsize;
		}

		if (!lp->akl_anon) {
			if (kloffset >= end) break;
			tmp = (kloffset - offset) - PAGE_SIZE;
			addr += tmp;
			offset += tmp;
			noffset += tmp;
			tmp = atop(tmp);
			app += tmp;
			napp += tmp;
			continue;
		}


		if (!(ap = *app)) continue;

		a_lock(ap);

		/*
		 * If the page is wired we can't do COW
		 */

		pp = ap->an_page;

		if ((pp != VM_PAGE_NULL) && pp->pg_wire_count) {
			register struct vm_anon *newap;

			if (vm_page_hold(pp) == TRUE) {
				a_unlock(ap);
				vm_page_wait(pp);
				if (pp->pg_error) {
					a_free(*raop, atop(size));
					vm_object_deallocate(*raop);
                                        return KERN_FAILURE;
				}
			}
			else a_unlock(ap);
			newap = a_anon_appage_alloc(nop, noffset, nlp, napp);
			vm_page_copy(pp, newap->an_page);
			vm_page_release(pp);
			vm_page_release(newap->an_page);
		}
		else {
			/*
			 * Must set up for COW
			 */

			nlp->akl_anon++;
			ap->an_refcnt++;
			if ((pp != VM_PAGE_NULL)  &&
				(aop->ao_flags & AF_SHARED ||
				ap->an_refcnt == 2)) {
				if (vm_page_hold(pp) == TRUE) {
					a_unlock(ap);
					vm_page_wait(pp);
					if (pp->pg_error) {
						a_free(*raop, atop(size));
						vm_object_deallocate(*raop);
                                        	return KERN_FAILURE;
					}
				}
				else a_unlock(ap);
				pmap_copy_on_write(page_to_phys(pp));
				vm_page_release(pp);
			}
			else a_unlock(ap);
			*napp = ap;
		}	
	}
	lk_mpunlock(lp,m,p);
	lk_mpunlock(nlp,m,p);
	return KERN_SUCCESS;
}


/*
 * Unmap part or all of an address space.
 * We must acquire the anon kluster locks for
 * the region to be affected.
 */

u_anon_unmap(register vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t len)
{
	register struct vm_anon_object *aop;
	register struct vpage *vp;
	register vm_offset_t offset;
	int npages;

	if (!u_map_entry_delete_check(ep->vme_map, ep, addr, addr + len))
		return KERN_RESOURCE_SHORTAGE;

	aop = (struct vm_anon_object *) (ep->vme_object);


	/*
	 * Release potential wirings
	 */

	u_anon_lockop(ep, addr, len, VM_UNWIRE);

	/*
	 * MAP_PRIVATE readonly or shared and reference count not one
 	 */

	if (!aop->ao_anon ||
		(aop->ao_flags & AF_SHARED && aop->ao_refcnt != 1)) {
		u_map_entry_unmap(ep, addr, len);
		pmap_remove(vm_map_pmap(ep->vme_map), addr, addr + len);
		return KERN_SUCCESS;
	}

	vm_object_lock(aop);
	vm_object_wait(aop,SWAP);
	aop->ao_oflags |= OB_CHANGE;
	vm_object_unlock(aop);

	offset = ep->vme_offset + (addr - ep->vme_start);

	/*
	 * Acquire all locks in the region of interest
	 * to prevent further pageout or swapout activity.
	 */

	u_anon_klock(aop, offset, len, TRUE);

	npages = atop(len);

	/*
	 * Before freeing anon unload all
	 * translations to this address space.
	 */

	pmap_remove(vm_map_pmap(ep->vme_map), addr, addr + len);

	u_anon_free(aop, offset, npages);

	/*
	 * Release reserved anon space
	 */

	if (aop->ao_ranon) a_free(aop, npages);

	/*
	 * Unmapping makes growth impossible
	 */

	aop->ao_flags |= AF_NOGROW;


	u_anon_klock_free(aop, offset, len);

	/*
	 * Free the anon array part
	 */

	h_kmem_free((caddr_t) (aop->ao_anon + atop(offset)),
			npages * sizeof (aop->ao_anon));

	if (aop->ao_refcnt == 1 && ep->vme_start == addr &&
		ep->vme_end == (addr + len))
		aop->ao_anon = (struct vm_anon **) 0;

	u_map_entry_unmap(ep, addr, len);

	vm_object_lock(aop);
	aop->ao_oflags ^= OB_CHANGE;
	vm_object_wakeup(aop,CHANGE);
	vm_object_unlock(aop);

	return KERN_SUCCESS;
}


u_anon_msync(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	int flags)
{
	struct vm_anon_object *aop;
	register vm_offset_t offset, last;
	register boolean_t shared;
	register alock_t lp;
	vm_offset_t kloffset;
	register struct vm_anon **app;
	register boolean_t backed;
	

	aop = (struct vm_anon_object *) (ep->vme_object);
	if (aop->ao_bobject == VM_OBJECT_NULL) return KERN_SUCCESS;
	offset = ep->vme_offset + (addr - ep->vme_start);
	if (app = aop->ao_anon) app = app + atop(offset);
	if (aop->ao_flags & AF_SHARED) shared = TRUE;
	else shared = FALSE;
	
	if (shared) {
		kloffset = anon_kltrunc(offset);
		lp = aop->ao_klock + anon_kl(kloffset);
		lk_lock(lp);
	}

	/*
	 * Optimize by calling the OOP_CONTROL
	 * with ranges of offsets.
	 */

	for (backed = FALSE, last = offset; size;
		offset += PAGE_SIZE, size -= PAGE_SIZE) {

		if (shared == TRUE && offset >= kloffset && size > PAGE_SIZE) {
			if (backed == TRUE) {
				OOP_CONTROL(aop->ao_bobject, aop->ao_boffset +
				last, atop(offset - last),
				VMOC_MSYNC, flags);
				last = offset;
				backed = FALSE;
			}

			lk_unlock(lp);
			lp++;
			kloffset += anon_klsize;
			lk_lock(lp);
		}

		if (!app || !(*app++)) {
			backed = TRUE;
			continue;
		}

		/*
		 * Have an anon
		 */
		if (backed == TRUE) {
			OOP_CONTROL(aop->ao_bobject, aop->ao_boffset +
				last, atop(offset - last),
				VMOC_MSYNC, flags);
			last = offset;
			backed = FALSE;
		}
	}

	/*
	 * If the last anon cell isn't backed we missed it.
	 */

	if (backed == TRUE)
		OOP_CONTROL(aop->ao_bobject, aop->ao_boffset + last,
			atop (offset - last), VMOC_MSYNC, flags);
				
	if (shared == TRUE) lk_unlock(lp);
	return KERN_SUCCESS;
}

u_anon_lockop(vm_map_entry_t ep,
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
			ret = u_anon_fault(ep, start, 1,
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
				u_anon_fault(ep, start, 1,
				ep->vme_maxprot, wire, (vm_page_t *) 0);
		}
	}
	return ret;
}

/*
 * Not supported yet.
 */

u_anon_swap(register vm_map_entry_t ep,
	int rw)
{
	panic("u_anon_swap: not supported");
}

u_anon_core(register vm_map_entry_t ep,
	register unsigned next,
	register char *vec,
	register int *vecsize)
{
#ifdef	COMPRESSED_CORE

	register struct vm_anon_object *aop;
	register struct vm_anon **app;
	register vm_offset_t start, end;
	register int i;

	aop = (struct vm_anon_object *) (ep->vme_object);	
	if ((next >= atop(ep->vme_end - ep->vme_start)) || !aop->ao_anon) {
		*vecsize = -1;
		return 1;
	}
	app = aop->ao_anon + next + atop(ep->vme_offset);
	start = ep->vme_start + atop(next);
	end = ep->vme_end;
	for (i = 0; i < *vecsize && start < end;
		i++, start += PAGE_SIZE, app++, vec++)
		if (*app) *vec = TRUE;
		else *vec = FALSE;
	
	*vecsize = i;
#else	/* COMPRESSED_CORE */
	/*
	 * If the entry has write protection, core() will try to write
	 * it to disk.  If pages in this entry has differing
	 * protections (vme_private is allocated) make sure they have
	 * read access.  It is okay to allow read access as no threads
	 * are active at this point since the process is about to
	 * terminate.
	 */
	if (ep->vme_private && (ep->vme_protection & VM_PROT_WRITE)) {
		register vm_offset_t addr = ep->vme_start;
		register struct vpage *vp;

		vp = (struct vpage *)ep->vme_private;
		for (; addr < ep->vme_end; addr += PAGE_SIZE, vp++) {
			if (!(vp->vp_prot & VM_PROT_READ)) {
				vm_map_lock(ep->vme_map);
				(void)u_anon_protect(ep, addr, PAGE_SIZE,
						     vp->vp_prot|VM_PROT_READ);
				vm_map_unlock(ep->vme_map);
			}
		}
	}
#endif
	return 0;
}

u_anon_control(register vm_map_entry_t ep,
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
	case VMC_SEM_SLEEP:
	case VMC_SEM_WAKEUP:
	{
		register struct vm_anon_object *aop;

		aop = (struct vm_anon_object *) (ep->vme_object);
		if (aop->ao_bobject || ((aop->ao_flags & AF_SHARED) == 0)) {
			vm_mape_faultdone(ep);
			return KERN_INVALID_ADDRESS;
		}
		OOP_REFERENCE(aop);
		vm_mape_faultdone(ep);
		ret = msmem_control((vm_object_t) aop, addr, control);
		OOP_DEALLOCATE(aop);
		return ret;
	}
	default:
		return KERN_INVALID_ARGUMENT;
	}	

	u_map_entry_split(ep, addr, size);
	return KERN_SUCCESS;
}

/*
 * Protection changes aren't allowed
 * while there is wiring.
 */

u_anon_protect(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t size,
	vm_prot_t prot)
{
	register struct vm_anon_object *aop;
	register struct vm_anon **app, *ap;
	register struct vpage *vp;
	register vm_offset_t offset, end;
	vm_offset_t start;
	register int vpi;
	vm_prot_t oldprot;
	kern_return_t ret;
	boolean_t needlocks;
	alock_t lp;
	vm_offset_t kloffset;

	aop = (struct vm_anon_object *) (ep->vme_object);
	start = addr;

	if (ep->vme_private) {
		boolean_t changeprot;

		changeprot = FALSE;
		vp = (struct vpage *) (ep->vme_private);
		vp += atop(addr - ep->vme_start);
		vpi = 1;
		for (end = addr + size; addr < end; addr += PAGE_SIZE, vp++) {
			if (vp->vp_prot != prot) {
				if (vp->vp_plock || vl_kwire(ep, 
					addr, addr + PAGE_SIZE)
				        || lw_is_wired(ep, addr, addr + PAGE_SIZE))
				              return KERN_PAGELOCKED;
				else changeprot = TRUE;
			}
		}
		if (changeprot == FALSE) return KERN_SUCCESS;
		else addr = start;
	}
	else {
		vp = &ep->vme_vpage;
		vpi = 0;
		if (vp->vp_prot == prot) return KERN_SUCCESS;
		else if (vp->vp_plock || vl_kwire(ep, addr, addr + size)
			 || lw_is_wired(ep, addr, addr + PAGE_SIZE))
			return KERN_PAGELOCKED;
	}

	offset = ep->vme_offset + (addr - ep->vme_start);

	if (aop->ao_bobject && (prot & VM_PROT_WRITE)) {
		if (aop->ao_flags & AF_SHARED) {
			vm_object_lock(aop);
			if (!aop->ao_ranon &&
				(a_reserve(aop, atop(aop->ao_size)) == FALSE)) {
				vm_object_unlock(aop);
				return KERN_RESOURCE_SHORTAGE;
			}

			vm_object_unlock(aop);
		}
		else if (!aop->ao_anon) {
			struct vm_anon_object *naop;

			assert(aop->ao_anon == (struct vm_anon **) 0);
			if (size == (ep->vme_end - ep->vme_start) &&
				aop->ao_refcnt == 1) {
				if (a_reserve(aop, atop(size)) == FALSE)
					return KERN_RESOURCE_SHORTAGE;
				if (ep->vme_offset) {
					aop->ao_boffset += ep->vme_offset;
					ep->vme_offset = 0;
				}
				aop->ao_size = size;
				u_anon_oop_create(aop, FALSE);
				ep->vme_protection = prot;
				return 0;
			}

			if (!u_map_entry_clip_check(ep->vme_map, ep, addr,
				addr + size)) return KERN_RESOURCE_SHORTAGE;
			if (ret = vm_object_allocate(OT_ANON, size,
				FALSE, &naop)) return ret;
			else if (a_reserve(naop, atop(naop->ao_size)) == FALSE){
				vm_object_deallocate((vm_object_t) naop);
				return KERN_RESOURCE_SHORTAGE;
			}
			u_map_entry_split(ep, addr, size);
			vm_object_reference(aop->ao_bobject);
			naop->ao_flags |= AF_PRIVATE;
			naop->ao_bobject = aop->ao_bobject;
			naop->ao_boffset = aop->ao_boffset;
			vm_object_deallocate(aop);
			naop->ao_boffset += ep->vme_offset;
			ep->vme_offset = 0;
			ep->vme_object = (vm_object_t) naop;
			ep->vme_protection = prot;
		        if (ep->vme_private) {
			    vp = (struct vpage *) (ep->vme_private);
			    addr =  ep->vme_start;
			    for (end = ep->vme_end; addr<end; addr += PAGE_SIZE,
								  vp += vpi) 
				vp->vp_prot = prot;
			}
			return 0;
		}
	}

	if (!vpi) {
		if (ep->vme_start != addr || ep->vme_end != (addr + size)) {
			if ((ret = u_vpage_init(ep, TRUE)) != KERN_SUCCESS)
				return ret;
			vpi = 1;	
			vp = (struct vpage *) (ep->vme_private);
		}
	}
	else vp = (struct vpage *) (ep->vme_private);

	/*
	 * If its shared and this op is enabling write
	 * we have to accomplish it without missing someones COW setup.
	 */

	needlocks = ((aop->ao_flags & AF_SHARED) && (prot & VM_PROT_WRITE))
			? TRUE : FALSE;
	
	if (needlocks) {
		kloffset = anon_kltrunc(offset);
		lp = aop->ao_klock + anon_kl(kloffset);
		lk_lock(lp);
		kloffset += anon_klsize;	
	}
	

	if (app = aop->ao_anon) app += atop(offset);

	if (!vpi) {
		oldprot  = vp->vp_prot;
		vp->vp_prot = prot;
	}
	else vp += atop(addr - ep->vme_start);

	for (end = addr + size; addr < end;
		addr += PAGE_SIZE, offset += PAGE_SIZE, vp += vpi) {
		
			
		if (needlocks && (offset >= kloffset)) {
			lk_unlock(lp);
			lp++;
			lk_lock(lp);
			kloffset += anon_klsize;
		}
		
		if (vpi) {
			oldprot = vp->vp_prot;
			vp->vp_prot = prot;
		}

		if (!app || !(ap = *app++)) {
			 if (aop->ao_bobject &&
				(((vp->vp_prot & ~VM_PROT_WRITE) & oldprot)
					!= oldprot))
				pmap_protect(vm_map_pmap(ep->vme_map),
					addr, addr + PAGE_SIZE,
					vp->vp_prot & ~VM_PROT_WRITE);
		}
		else {
			a_lock(ap);
			if (ap->an_refcnt > 1) {
				if ((vp->vp_prot & ~VM_PROT_WRITE) != oldprot)
					pmap_protect(vm_map_pmap(ep->vme_map),
						addr, addr + PAGE_SIZE,
						vp->vp_prot & ~VM_PROT_WRITE);
			}
			else if (vp->vp_prot != oldprot)
				pmap_protect(vm_map_pmap(ep->vme_map),
					addr, addr + PAGE_SIZE,
					vp->vp_prot);
			a_unlock(ap);
		}
	}

	if (needlocks) lk_unlock(lp);
	return KERN_SUCCESS;
}

boolean_t
u_anon_check_protect(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	register vm_prot_t prot)
{
	if (ep->vme_private) {
		register struct vpage *vp;

		vp = (struct vpage *) (ep->vme_private) +
			atop(addr - ep->vme_start);
		for (; size; size -= PAGE_SIZE, vp++)
			if ((vp->vp_prot & prot) != prot) return FALSE;
		return TRUE;
	}
	else {
		if ((ep->vme_protection & prot) != prot) return FALSE;
		else return TRUE;
	}
}

/*
 * This routine currently has no specific value.
 * Should lazy evaluation or some other state between
 * the object and map be evaluated, then this is where
 * it should occur.
 */

u_anon_copy(register vm_map_entry_t ep, vm_copy_op_t op)
{
	return KERN_SUCCESS;
}

/*
 * Attempt to grow this anon region with more
 * anon memory.  We can't grow it if a hole was
 * previously punched in the middle of it.
 * This could cause the object to appear in more
 * than one map entry which makes it impossible
 * to adjust the offset when growing down.
 */

u_anon_grow(vm_map_entry_t ep,
	vm_prot_t prot,
	register vm_size_t size,
	as_grow_t direction)
{
	register struct vm_anon_object *aop;
	register struct vm_anon **app;
	register alock_t lp, nlp, llp;
	alock_t newlp;
	register int i, j, k, oklsize;
	vm_size_t oldsize;
	int vpageinc, nlock_size;
	kern_return_t ret;

	aop = (struct vm_anon_object *)(ep->vme_object);

	if (aop->ao_bobject || (aop->ao_flags & (AF_SHARED|AF_NOGROW)))
		return KERN_NO_SPACE;

	/*
	 * May have to adjust vpage
	 * but don't grow for expansion.
	 */

	if (ep->vme_private || ep->vme_plock) {
		if (!ep->vme_private) {
			ret = u_vpage_init(ep, TRUE);
			if (ret != KERN_SUCCESS) return ret;
		}
		vpageinc = atop(size);	
		ret = u_vpage_reserve(ep->vme_map, vpageinc);
		if (ret != KERN_SUCCESS) return ret;
	}
	else if (ep->vme_protection == prot) vpageinc = 0;
	else return KERN_NO_SPACE;


	vm_object_lock(aop);
	vm_object_wait(aop,SWAP);
	aop->ao_oflags |= OB_CHANGE;
	vm_object_unlock(aop);

	nlock_size = 0;

	if (u_anon_klock_try(aop, ep->vme_offset, aop->ao_size) == FALSE) {
		ret = KERN_NO_SPACE;
		goto failed;
	}

	oldsize = aop->ao_size;
	j = atop(size);
	oklsize = anon_kl(anon_klround(ep->vme_offset + oldsize));

	/*
	 * Determine whether the lock array will grow.
	 */

	if (direction == AS_GROWUP) {
		i = anon_kl(anon_klround(ep->vme_offset + oldsize + size));
		if (i != aop->ao_nklock) nlock_size = i;
	}
	else {
		if (size > ep->vme_offset) {
			i = oklsize +
				anon_kl(anon_klround(size - ep->vme_offset));
			nlock_size = i;
		}
	}

	if (nlock_size) {
		newlp = (alock_t) h_kmem_zalloc_memory(nlock_size *
				sizeof (struct anon_klock), FALSE);
		if (newlp == (alock_t) 0) goto failed;
	}

	/*
	 * Attempt to grow the anon array and fix it up.
	 */

	if ((ret = a_grow(aop, size, atop(ep->vme_offset), direction, FALSE))
			!= KERN_SUCCESS)  goto failed;

	/*
	 * Have all resources and now comitted to expansion.
	 */

	if (direction == AS_GROWUP) {
		if (i == aop->ao_nklock) {
			lp = aop->ao_klock + i - 1;
			assert((lp->akl_pages + j) <= anon_pagesinkl);
			lp->akl_pages += j;
			u_anon_klock(aop, ep->vme_offset, aop->ao_size, FALSE);
			ep->vme_end += size;
			goto finish;
		}
		aop->ao_nklock = nlock_size;
		nlp = newlp;
		llp = aop->ao_klock + oklsize - 1;
		for (lp = aop->ao_klock; lp <= llp; i--, lp++, nlp++) {
			*nlp = *lp;
			lk_lock_init(nlp);
		}
		if (llp->akl_pages < anon_pagesinkl) {
			k = MIN((anon_pagesinkl - llp->akl_pages), j);
			(nlp - 1)->akl_pages += k;
			j -= k;
		}
		while (i--) {
			lk_lock_init(nlp);
			nlp->akl_pages += MIN(anon_pagesinkl, j);
			j -= nlp->akl_pages;
			nlp++;
		}
		ep->vme_end += size;
		h_kmem_free(aop->ao_klock,
			oklsize * sizeof (struct anon_klock));
		aop->ao_klock = newlp;
	}

	/*
	 * Growing down
	 */

	else {
		if (size <= ep->vme_offset) {
			lp = aop->ao_klock;
			lp->akl_pages += j;
			ep->vme_offset -= size;
			aop->ao_anon -= atop(ep->vme_offset);
			u_anon_klock(aop, ep->vme_offset, aop->ao_size, FALSE);
		}
		else {
			vm_offset_t newoffset;
			long adjust;

			aop->ao_nklock = nlock_size;
			nlp = newlp;
		
			nlp +=  i - 1;
			lp = aop->ao_klock + oklsize - 1;
			for (llp = aop->ao_klock; lp >= llp; lp--, nlp--) {
				*nlp = *lp;
				lk_lock_init(nlp);
			}
			if (ep->vme_offset) {
				k = MIN(j, (anon_pagesinkl - llp->akl_pages));
				(nlp + 1)->akl_pages += k;
				j -= k;
			}

			for(k = i - oklsize; k--; nlp--) {
				nlp->akl_pages = MIN(j, anon_pagesinkl);
				j -= nlp->akl_pages;
				lk_lock_init(nlp);
			}
			nlp++;
			newoffset = ptoa(anon_pagesinkl - nlp->akl_pages);
			adjust = (i - oklsize) << anon_klshift;
			aop->ao_rbase += adjust;
			aop->ao_anon -= atop(newoffset);
			ep->vme_offset = newoffset;
			h_kmem_free(aop->ao_klock,
				oklsize * sizeof (struct anon_klock));
			aop->ao_klock = newlp;
		}
		ep->vme_start -= size;
	}

finish:
	if (vpageinc) {
		register struct vpage *vp;

		u_vpage_expand(&ep->vme_private, atop(oldsize),
			vpageinc, (direction == AS_GROWUP) ? 0 : vpageinc);

		vp = (struct vpage *)(ep->vme_private);
		if (direction == AS_GROWUP) vp += atop(oldsize);
		for (; vpageinc--; vp++) vp->vp_prot = prot;
	}
	vm_object_lock(aop);
	aop->ao_oflags ^= OB_CHANGE;
	vm_object_wakeup(aop,CHANGE);
	vm_object_unlock(aop);
	return ret;

failed:
	vm_object_lock(aop);
	aop->ao_oflags ^= OB_CHANGE;
	vm_object_wakeup(aop,CHANGE);
	vm_object_unlock(aop);
	u_anon_klock(aop, ep->vme_offset, aop->ao_size, FALSE);
	if (vpageinc) u_vpage_free_reserve(ep->vme_map, vpageinc);
	if (nlock_size && (newlp != (alock_t) 0))
		h_kmem_free((caddr_t) newlp,
			nlock_size * sizeof (struct anon_klock));
	return KERN_RESOURCE_SHORTAGE;
}


/*
 * Free all the locks possible
 */

u_anon_klock_free(register struct vm_anon_object *aop,
	register vm_offset_t offset,
	register vm_size_t len)
{
	register vm_offset_t klstart, klend, end;
	register alock_t lp, elp;


	klstart = anon_kltrunc(offset);
	lp = aop->ao_klock + anon_kl(klstart);

	/*
	 * end not contained in the same region as start
	 */

	if ((end = offset + len) > (klstart + anon_klsize)) {
		lp->akl_pages -= atop((klstart + anon_klsize) - offset);
		if (lp->akl_pages) lk_unlock(lp);

		klend = anon_kltrunc(end - PAGE_SIZE);
		elp = aop->ao_klock + anon_kl(klend);
		elp->akl_pages -= atop(end - klend);
		if (elp->akl_pages) lk_unlock(elp);

		for (lp = lp + 1; lp < elp; lp++) lp->akl_pages = 0;
	}

	/*
	 * end and start in the same lock region
	 */

	else {
		lp->akl_pages -= atop(len);
		if (lp->akl_pages) lk_unlock(lp);
	}

	return;
}

u_anon_klock(register struct vm_anon_object *aop,
	register vm_offset_t start,
	register vm_size_t size,
	boolean_t lock)
{
	register vm_offset_t end;
	register alock_t lp;
	
	end = anon_klround(start + size);
	start = anon_kltrunc(start);
	for (lp = aop->ao_klock + anon_kl(start); start < end;
		start += anon_klsize, lp = aop->ao_klock + anon_kl(start))
		if (lock == TRUE) {
			lk_lock(lp);
		}
		else lk_unlock(lp);
}

boolean_t
u_anon_klock_try(register struct vm_anon_object *aop,
	register vm_offset_t start,
	register vm_size_t size)
{
	register vm_offset_t end, first;
	register alock_t lp;
	
	end = anon_klround(start + size);
	first = start = anon_kltrunc(start);
	for (lp = aop->ao_klock + anon_kl(start); start < end;
		start += anon_klsize, lp = aop->ao_klock + anon_kl(start))
		if (lk_lock_try(lp) == FALSE) goto failed;
	return TRUE;
failed:
	for (lp = aop->ao_klock + anon_kl(first); first < start;
		first += anon_klsize, lp = aop->ao_klock + anon_kl(first))
		lk_unlock(lp);
	return FALSE;
}

/*
 * Free anon at offset in object
 * Returns the number of anon freed.
 * This assume the object kluster lock is held
 * to prevent pageout or swapout activity within the
 * region of interest.
 */

int
u_anon_free(struct vm_anon_object *aop,
	register vm_offset_t offset,
	int npages)
{
	register struct vm_anon *ap;
	register vm_page_t pp;
	struct vm_anon **app;
	register int afreed;
	register alock_t lp;
	register vm_offset_t klnext, tmp, loffset;

	/*
	 * No anon array
	 */

	if (aop->ao_anon == (struct vm_anon **) 0) return 0;
	assert((offset + ptoa(npages)) <= aop->ao_size);

	afreed = 0;
	loffset = offset + ptoa(npages);
	app = aop->ao_anon + atop(offset);
	lp = aop->ao_klock + anon_kl(anon_kltrunc(offset));
	klnext = anon_klround(offset + PAGE_SIZE);
	do {
		if (offset >= klnext) {
			if (offset >= loffset) break;
			klnext += anon_klsize;
			lp++;
		}

		if (!lp->akl_anon) {
			tmp = klnext - offset;
			offset += tmp;
			if (offset >= loffset) break;
			tmp = atop(tmp);
			npages -= tmp;
			app += tmp;
			lp++;
			klnext += anon_klsize;
			continue;
		}

		if (ap = *app) {
			afreed++;
			*app = (struct vm_anon *) 0;
			a_lock(ap);
			pp = ap->an_page;
			if (ap->an_refcnt > 1) {
				ap->an_refcnt--;
				if (pp && pp->pg_owner == aop) {
					vm_page_remove_object(&lp->akl_pagelist,
						pp);
					lp->akl_rpages--;
					pp->pg_owner = NULL;
				}
				a_unlock(ap);
			}
			else {
				if (pp) {
					if (pp->pg_busy) {
						pp->pg_hold++;
						a_unlock(ap);
						vm_page_wait(pp);
						a_lock(ap);
						pp->pg_hold = 0;
					}
					if (vm_pageout_remove(pp, TRUE, FALSE)
						== FALSE) 
						panic("u_anon_free: page busy");
					a_unlock(ap);
					vm_anon_page_free(aop, offset, ap);
				}
				else a_unlock(ap);
				a_anon_free(ap);
			}
			lp->akl_anon--;
		}
		app++;
		offset += PAGE_SIZE;
		npages--;
	} while (npages);
	return afreed;
}


/*
 * anon object management
 */

int
u_anon_oop_create(register struct vm_anon_object *aop,
	caddr_t private)
{
	register int npages, nlock;
	register alock_t lp;
	boolean_t nonanon;

	nonanon = (boolean_t) private;

	if (nonanon == TRUE) return 0;

	aop->ao_crefcnt = 1;
	npages = atop(aop->ao_size);

	/*
	 * Allocate the anon array
	 */

	aop->ao_anon = (struct vm_anon **)
		h_kmem_zalloc(npages * sizeof (aop->ao_anon));

	/*
	 * Alloc the anon lock array and initialize it.
	 */

	nlock = anon_kl(anon_klround(aop->ao_size));
	aop->ao_klock = (alock_t) h_kmem_zalloc(nlock *
				sizeof (struct anon_klock));
	aop->ao_nklock = nlock;
	lp = aop->ao_klock;

	while (nlock--) {
		lk_lock_init(lp);
		lp->akl_pages = (npages >= anon_pagesinkl) ?
				anon_pagesinkl : npages;
		lp++;
		npages -= anon_pagesinkl;
	}

	vm_object_swapon(aop);
	return KERN_SUCCESS;
}


#ifndef	lk_lock_try

boolean_t
lk_lock_try(register alock_t lp)
{
	if (!usimple_lock_try(&lp->akl_slock)) return FALSE;
	else if (lp->akl_lock) {
		usimple_unlock(&lp->akl_slock);
		return FALSE;
	}
	else {
		lp->akl_lock = 1;
		usimple_unlock(&lp->akl_slock);
		return TRUE;
	}
}

#endif	/* !lk_lock_try */

boolean_t
lk_plock_try(register alock_t lp)
{
	if (!usimple_lock_try(&lp->akl_slock)) return FALSE;
	else if (lp->akl_lock) {
		if (lp->akl_plock || !lp->akl_mlock) {
			lk_sunlock(lp);
			return FALSE;
		}
	}
	else lp->akl_lock = 1;
	lp->akl_plock = 1;
	lk_sunlock(lp);
	return TRUE;
}

boolean_t
u_anon_oop_lock_try(register struct vm_anon_object *aop,
	register vm_offset_t offset)
{
	register alock_t lp;

	if (!vm_object_lock_try(aop)) return FALSE;
	else if (aop->ao_oflags & OB_CHANGE) {
		vm_object_unlock(aop);
		return FALSE;
	}

	/*
	 * Take the relative base
	 * and compute a real offset
	 */

	offset = (vm_offset_t) ((long) offset + aop->ao_rbase);

	lp = aop->ao_klock + anon_kl(offset);

	if (!lk_plock_try(lp)) {
		vm_object_unlock(aop);
		return FALSE;
	}
	else {
		vm_object_unlock(aop);
		return TRUE;
	}
}


int
u_anon_oop_unlock(register struct vm_anon_object *aop,
	register vm_offset_t offset)
{
	register alock_t lp;

	offset = (vm_offset_t) ((long) offset + aop->ao_rbase);
	lp = aop->ao_klock + anon_kl(offset);
	lk_mpunlock(lp,p,m);
	return;
}

u_anon_oop_reference(register struct vm_anon_object *aop)
{
	vm_object_lock(aop);
	aop->ao_refcnt++;
	aop->ao_rescnt++;
	aop->ao_crefcnt++;
	vm_object_unlock(aop);
}

u_anon_oop_deallocate(register struct vm_anon_object *aop)
{
	register struct vm_anon *app;
	register int npages;

	vm_object_lock(aop);
	aop->ao_refcnt--;
	aop->ao_rescnt--;
	aop->ao_crefcnt--;

	if (aop->ao_refcnt) {
		vm_object_unlock(aop);
		return 0;
	}
	if (aop->ao_oflags & OB_SWAPON) vm_object_swapoff(aop);
	vm_object_unlock(aop);	


	/*
	 * If any anon array and anon allocated then free each one
	 * The anon is freed before the reservation is updated.
	 * This will eliminate inconsistencies in what swap
	 * space is actually available.
	 */

	if (aop->ao_anon) {
		register int npages;

		npages = atop(aop->ao_size);
		u_anon_free(aop, (vm_offset_t) 0, npages);
		h_kmem_free(aop->ao_anon, npages * sizeof (aop->ao_anon));
	}

	if (aop->ao_klock)
		h_kmem_free(aop->ao_klock,
			aop->ao_nklock * sizeof (struct anon_klock));

	/*
	 * If any anon reserved then free it.
	 */

	if (aop->ao_ranon) a_free(aop, aop->ao_ranon);

	/*
	 * If there is a backing object we must
	 * remove our reference by deallocating it.
	 */

	if (aop->ao_bobject) vm_object_deallocate(aop->ao_bobject);

	vm_object_free(aop);
}

#if	VM_ANON_TRACK
static char anon_ownerbad[] = "page owner not valid";
long	anon_pageout_trace_index;
#define	ANON_PAGEOUT_TRACE_SIZE	3*1024
struct anon_pageout_trace {
	struct vm_anon *at_ap;
	struct vm_anon_object *at_aop;
	struct vm_page *at_pp;
} anon_pageout_trace[ANON_PAGEOUT_TRACE_SIZE];

#define	APAGEOUT_TRACE(AP,AOP,PP) {					\
	anon_pageout_trace[anon_pageout_trace_index].at_ap = (AP);	\
	anon_pageout_trace[anon_pageout_trace_index].at_aop = (AOP);	\
	anon_pageout_trace[anon_pageout_trace_index].at_pp = (PP);	\
	if (++anon_pageout_trace_index >= ANON_PAGEOUT_TRACE_SIZE)	\
		anon_pageout_trace_index = 0;				\
}

#define	ACHECK_OWNER(PP,AOP,AP)						\
	if ((PP)->pg_owner != (AOP) || (AP)->an_page != (PP))		\
 		panic(anon_ownerbad);					\
	else APAGEOUT_TRACE(AP,AOP,PP)
#else

#define	ACHECK_OWNER(PP,AOP,AP)

#endif	/* VM_ANON_TRACK */

/*
 * Anon pageout.  We enter with the kluster lock
 * taken.   Attempt to pageout other pages.
 */

int
u_anon_oop_pageout(register struct vm_anon_object *aop,
	register vm_offset_t offset,
	vm_size_t size)
{
	register alock_t lp;
	vm_page_t pla[1];
	register vm_page_t pp, npp;
	register struct vm_anon **app, *ap;
	register int i;
	int pagecount = 0;
	struct vm_swap *swap_hint;

	offset = (vm_offset_t) ((long) offset + aop->ao_rbase);
	lp = aop->ao_klock + anon_kl(offset);
	app = aop->ao_anon;
	ap = *(app + atop(offset));
	a_lock(ap);
	pp = ap->an_page;
	if (vm_pageout_remove(pp, TRUE, TRUE) == FALSE) {
		vm_pageout_abort(pp);
		a_unlock(ap);
		lk_mpunlock(lp,p,m);
		return 0;
	}

	lk_slock(lp);
	if (!pp->pg_dirty) {
		vm_anon_page_free(aop, offset, ap);
		a_unlock(ap);
		lk_hmpunlock(lp,p,m);
		return 0;
	}
	else {
		/*
		 * Swap i/o load balancing.  Attempt to assign pages within
		 * this kluster lock to the same swap device to take full
		 * advantage of contiguous page optimizations in vm_swap_io().
		 * By encapsulating with swap_read_lock, we insure that
		 * swap_hint is valid even if device deallocation
		 * (ie. "swapoff") becomes available in the future.
		 */
		swap_read_lock();
		simple_lock(&vm_swap_circular_lock);
		swap_hint = vm_swap_circular;
		if (vm_swap_circular)
			vm_swap_circular = vm_swap_circular->vs_fl;
		simple_unlock(&vm_swap_circular_lock);


		if (!a_swap_lazy_alloc(ap, pp, FALSE, swap_hint)) {
			swap_read_unlock();
			vm_pageout_abort(pp);
			a_unlock(ap);
			lk_hmpunlock(lp,p,m);
			return 0;
		}
		pp->pg_owner = (struct vm_anon_object *) 0;
		lp->akl_rpages--;
		vm_page_remove_object(&lp->akl_pagelist, pp);
		lk_sunlock(lp);
		a_unlock(ap);
	}


	if (lp->akl_mlock) i = 0;
	else i = lp->akl_rpages;
	
	pla[0] = pp;
	pp->pg_pnext = VM_PAGE_NULL;
	AN_TRACK_WRITE(ap);
	pagecount = 1;

	if (i) for (pp = lp->akl_pagelist, size -= PAGE_SIZE; size && i--;
		pp = npp, size -= PAGE_SIZE) {
		npp = pp->pg_onext;
		offset =  (vm_offset_t) (pp->pg_roffset + aop->ao_rbase);
		ap = *(app + atop(offset));
		ACHECK_OWNER(pp,aop,ap);
		if (a_lock_try(ap) == FALSE) continue;
		else if (vm_pageout_remove(pp, FALSE, TRUE) == FALSE) {
			a_unlock(ap);
			continue;
		}
		else if (pp->pg_dirty == 0) {
			AN_TRACK_WRITTEN(ap);
			vm_anon_page_free(aop, offset, ap);
			a_unlock(ap);
			continue;
		}
		else if (!a_swap_lazy_alloc(ap, pp, FALSE, swap_hint)) {
			vm_pageout_abort(pp);
			a_unlock(ap);
			continue;
		}
		pp->pg_owner = (struct vm_anon_object *) 0;
		a_unlock(ap);
		AN_TRACK_WRITE(ap);
		vm_page_remove_object(&lp->akl_pagelist, pp);
		lp->akl_rpages--;
		vm_swap_sort_add(pla,pp);
		pagecount++;
	}
	swap_read_unlock();

	lk_mpunlock(lp,p,m);
	vm_swap_io(pla, B_WRITE);

	return pagecount;
}

/*
 * Object swapout
 * The object is prevented from changing
 * but there could still be other activity.
 */

u_anon_oop_swapout(register struct vm_anon_object *aop,
	int *pages)
{
	register alock_t lp, lpend;
	vm_page_t pla[1];
	register vm_page_t pp, npp;
	register struct vm_anon **app, *ap;
	register vm_offset_t offset;
	register int i;
	register boolean_t mlock;
	int pushes, error;
	struct vm_swap *swap_hint;

	error = KERN_SUCCESS;

	pushes = 0;
	for (i = 0, lp = aop->ao_klock; i < aop->ao_nklock; lp++, i++)
		if (!lk_plock_try(lp)) {
			for (lpend = lp, lp = aop->ao_klock; lp < lpend; lp++)
				lk_mpunlock(lp,p,m);
			goto failed;
		}

	app = aop->ao_anon;


	for (lpend = lp, lp = aop->ao_klock; lp < lpend; lp++) {
		/*
		 * Swap i/o load balancing.  Attempt to assign pages within
		 * this kluster lock to the same swap device to take full
		 * advantage of contiguous page optimizations in vm_swap_io().
		 * By encapsulating with swap_read_lock, we insure that
		 * swap_hint is valid even if device deallocation
		 * (ie. "swapoff") becomes available in the future.
		 */
		swap_read_lock();
		simple_lock(&vm_swap_circular_lock);
		swap_hint = vm_swap_circular;
		if (vm_swap_circular)
			vm_swap_circular = vm_swap_circular->vs_fl;
		simple_unlock(&vm_swap_circular_lock);

		pla[0] = VM_PAGE_NULL;
		if (lp->akl_mlock) {
			mlock = TRUE;
			lk_slock(lp);
		}
		else mlock = FALSE;
		for (i = lp->akl_rpages, pp = lp->akl_pagelist; i;
				pp = npp, i--) {
			npp = pp->pg_onext;
			offset =  (vm_offset_t)
				(pp->pg_roffset + aop->ao_rbase);
			ap = *(app + atop(offset));
			ACHECK_OWNER(pp,aop,ap);
			if (a_lock_try(ap) == FALSE) continue;
			else if (pp->pg_owner != aop ||
				vm_pageout_remove(pp, TRUE, TRUE) == FALSE) {
				a_unlock(ap);
				continue;
			}
			else if (pp->pg_dirty == 0) {
				AN_TRACK_WRITTEN(ap);
				vm_anon_page_free(aop, offset, ap);
				a_unlock(ap);
				continue;
			}
			else if (!a_swap_lazy_alloc(ap, pp, FALSE, swap_hint)) {
				vm_pageout_abort(pp);
				a_unlock(ap);
				error = KERN_RESOURCE_SHORTAGE;
				continue;
			}
			pp->pg_owner = (struct vm_anon_object *) 0;
			a_unlock(ap);
			AN_TRACK_WRITE(ap);
			vm_page_remove_object(&lp->akl_pagelist, pp);
			lp->akl_rpages--;
			pushes++;
			vm_swap_sort_add(pla, pp);
		}
		swap_read_unlock();

		if (mlock == TRUE) {
			lk_hmpunlock(lp,p,m);
		}
		else lk_mpunlock(lp,p,m);
		if (*pla != VM_PAGE_NULL) vm_swap_io(pla, B_WRITE|B_SWAP);
	}

failed:
	*pages = pushes;
	return error;
}


kern_return_t
u_anon_oop_pagesteal(struct vm_anon_object *aop, struct vm_page *pp)
{

	if (vm_object_lock_try(aop)) {

		vm_offset_t offset;
		register struct vm_anon *ap;

		/* take this page if it is not being used */

		VM_PAGE_QUEUES_REMOVE(pp);
		offset = (vm_offset_t) ((long) pp->pg_roffset + aop->ao_rbase);
		ap = *(aop->ao_anon + atop(offset));
		vm_anon_page_free(aop, offset, ap);
		vm_page_unlock_queues();
		vm_object_unlock(aop);
			/* tell the caller that a page was freed */
		return KERN_SUCCESS;

	} else {

			/* dont take the page if we couldnt grab the object lock */

		return KERN_FAILURE;

	}
}

u_anon_oop_bad()
{
	panic("u_anon_oop_bad: unsupported anon object operation");
}

boolean_t
wire_check(register vm_page_t pp)
{
	register int count;

	count = vm_page_inactive_count + vm_page_active_count +
		vm_page_free_count + ubc_pages + vm_page_wire_count;
	count = MAX(count,1);

	if (pp->pg_wire_count != 0) return FALSE;
	else if ((vm_page_wire_count * 100) >
			 count * vm_tune_value(syswiredpercent))
		return TRUE;
	else	return FALSE;
}

struct vm_object_ops u_anon_oop = {
	&u_anon_oop_lock_try,		/* lock try */
	&u_anon_oop_unlock,		/* unlock */
	&u_anon_oop_reference,		/* reference */
	&u_anon_oop_deallocate,		/* deallcoate */
	&u_anon_oop_bad,		/* pagein */
	&u_anon_oop_pageout,		/* pageout */
	&u_anon_oop_swapout,		/* swapout */
	&u_anon_oop_bad,		/* control */
	&u_anon_oop_bad,		/* page control */
	&u_anon_oop_pagesteal,		/* page steal */
};

	
struct vm_object_config anon_object_conf = {
	&u_anon_oop_create,
	(int (*)()) 0,
	sizeof (struct vm_anon_object),
	&u_anon_oop,
	&u_mape_anon_ops,
};
