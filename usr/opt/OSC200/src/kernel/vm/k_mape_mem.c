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
static char *rcsid = "@(#)$RCSfile: k_mape_mem.c,v $ $Revision: 1.1.13.3 $ (DEC) $Date: 1993/12/02 22:05:47 $";
#endif
#include <mach_ldebug.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <kern/zalloc.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <vm/vm_anon.h>
#include <vm/vm_swap.h>
#include <vm/vm_fault.h>
#include <vm/vm_tune.h>
#include <vm/vm_kmap.h>
#include <vm/pmap.h>
#include <vm/heap_kmem.h>
#include <vm/vm_debug.h>
#include <vm/vm_perf.h>
#include <sys/buf.h>

/*
 * These are the default kernel map entry routine handlers.
 */

extern int
	k_mem_fault(),
	k_mem_unmap(),
	k_mem_lockop(),
	k_mem_prot(),
	k_mem_grow(),
	k_mem_badop();

extern boolean_t k_mem_check_prot();
extern struct vm_map_entry_ops k_mape_ops_invalid;

struct vm_map_entry_ops k_mape_ops_mem = {
	&k_mem_fault,		/* fault */
	&k_mem_badop,		/* dup */
	&k_mem_unmap,		/* unmap */
	&k_mem_badop,		/* msync */
	&k_mem_lockop,		/* lockop */
	&k_mem_badop,		/* swap */
	&k_mem_badop,		/* corefile */
	&k_mem_badop,		/* control */
	&k_mem_prot,		/* protect */
	&k_mem_check_prot,	/* check protection */
	&k_mem_badop,		/* kluster */
	&k_mem_badop,		/* copyout */
	&k_mem_grow,		/* grow */
};

/*
 * Kernel fault code for pkernel_object
 */

int
k_mem_fault(vm_map_entry_t ep,
	register vm_offset_t start,
	register vm_size_t size,
	vm_prot_t access,
	vm_fault_t wire,
	vm_page_t *pl)
{
	register vm_object_t pko = ep->vme_object;
	register vm_offset_t addr, offset;
	vm_offset_t end;
	struct vm_anon ***appp;
	register struct vm_anon **app, *ap;
	register vm_page_t pp;

	appp = (struct vm_anon ***) (ep->vme_private) +
		atop(start - ep->vme_start);
	offset = ep->vme_offset + (start - ep->vme_start);
		
	end = start + size;
	if (wire == VM_UNWIRE) {
		for (app = *appp, addr = start; size; addr += PAGE_SIZE, 
				appp++, app = *appp, offset += PAGE_SIZE,
					size -= PAGE_SIZE) {
			pmap_change_wiring(vm_map_pmap(ep->vme_map), 
				addr, FALSE);	
			if (ap = *app) {
				a_lock(ap);
				vm_page_unwire(ap->an_page, TRUE);
				a_unlock(ap);
			}
			else {
				vm_object_lock(pko);
				pp = vm_page_lookup(pko, offset);
				vm_page_unwire(pp, TRUE);
				vm_object_unlock(pko);
			}
		}
		pmap_pageable(vm_map_pmap(ep->vme_map), start, end, TRUE);
		return 0;
	}

	if (wire == VM_WIRE) {
		pmap_pageable(vm_map_pmap(ep->vme_map), start, end, FALSE);
	}
	else if ((access & ep->vme_protection) != access)
		return KERN_PROTECTION_FAILURE;

	for (app = *appp, addr = start; size; addr += PAGE_SIZE, 
		appp++, app = *appp, offset += PAGE_SIZE, size -= PAGE_SIZE) {
retry:
		vm_object_lock(pko);
		if (ap = *app) {
			vm_object_unlock(pko);
			a_lock(ap);
			if (pp = ap->an_page) {
				if (vm_page_hold(pp) == TRUE) {
					a_unlock(ap);
					vm_page_wait(pp);
					if (pp->pg_error)
						panic("k_mem_fault: IO error durring kernel pagein");	
				}
				else a_unlock(ap);
			}
			else {
				a_kpage_alloc(ap);
				vm_page_wait(pp = ap->an_page);
				if (pp->pg_error)
					panic("k_mem_fault: IO error durring kernel pagein");
			}
		}
		else {

			/*
			 * Finding the page in the pkernel_object
			 * implies that it isn't busy.
			 */

			pp = vm_page_lookup(pko, offset);
			if (pp == VM_PAGE_NULL) {
				pp = vm_zeroed_page_alloc(pko, offset);
				if (pp == VM_PAGE_NULL) {
					vm_object_unlock(pko);
					vm_wait();
					goto retry;
				}
				vpf_ladd(kzfod,1);
				pp->pg_hold = 1;
				pp->pg_app = app;
				vm_object_unlock(pko);
				vm_pageout_activate(pp, FALSE);
			} 
			else {
				if (vm_page_hold(pp) == TRUE) {
					vm_object_unlock(pko);
					vm_page_wait(pp);
					if (pp->pg_error)
						panic("k_mem_fault: IO error durring kernel pagein");	
				}
				else vm_object_unlock(pko);
			}
		}
		pmap_enter(vm_map_pmap(ep->vme_map), addr, page_to_phys(pp),
				ep->vme_protection, 
				(wire == VM_WIRE) ? TRUE : FALSE, access);
		if (wire == VM_WIRE) vm_page_wire(pp, TRUE);
		else vm_page_release(pp);
	}
	return 0;
}

/*
 * The kernel is allowed to delete space when
 * the wiring count is <= 1.
 */

int
k_mem_unmap(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t len)
{
	k_mem_entry_adjust(ep, addr, len, FALSE);
	if (ep->vme_kwire > 1) panic("k_mem_unmap: vme_kwire > 1");
	ep->vme_ops = &k_mape_ops_invalid;
	vm_map_unlock(ep->vme_map);
	k_mem_free_anon(ep);
	return 0;
}

/*
 * Enter with the map of ep map write locked.
 * We may acquire the fault lock to prevent further
 * non-faulting activity on this map entry.
 */

int
k_mem_lockop(register vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t len,
	vm_fault_t wiring)
{
	register vm_object_t obj;

	obj = ep->vme_object;
	k_mem_entry_adjust(ep, addr, len, FALSE);
	if (wiring == VM_UNWIRE) {
		if (ep->vme_kwire && (--(ep->vme_kwire) == 0)) {

			/*
			 * A non-paged kernel object is about to become 
			 * unwired for the first time.
			 */

			if (obj->ob_type == OT_KERNEL) {
				register vm_page_t pp;
				register vm_offset_t offset, last;
				register struct vm_anon ***appp, **app;

				/*
				 * Assert fault lock to prevent further
				 * activity.  Note faulting can't occur
				 * because the pages are wired.
				 */

				vm_mape_fault(ep);
				vm_map_unlock(ep->vme_map);

				offset = ep->vme_offset + 
					(addr - ep->vme_start);
				k_mem_grow_pageable(ep);
				appp = (struct vm_anon ***) ep->vme_private;
				vm_object_reference(pkernel_object);
				for(app = *appp, last = offset + len; 
					offset < last; 
					offset += PAGE_SIZE, 
					appp++, app = *appp) {

					vm_object_lock(obj);
					pp = vm_page_lookup(obj, offset);
					if (pp == VM_PAGE_NULL)
					panic("k_mem_lockop: page not found");
					vm_page_remove(pp);
					vm_object_unlock(obj);

					vm_object_lock(pkernel_object);
					vm_page_rename(pp, 
						pkernel_object, offset);
					vm_object_unlock(pkernel_object);
					pp->pg_app = app;
				} 
				vm_object_deallocate(obj);
				ep->vme_object = pkernel_object;
				k_mem_fault(ep, addr, len, 
					VM_PROT_DEFAULT, VM_UNWIRE,
					(vm_page_t *) 0);
				vm_mape_faultdone(ep);
				return 0;

			}

			/*
			 * A pageable object is no longer wired.
			 */

			else {
				vm_mape_fault(ep);
				vm_map_unlock(ep->vme_map);
				k_mem_fault(ep, addr, len,
					VM_PROT_DEFAULT, VM_UNWIRE, 
					(vm_page_t *) 0);
				vm_mape_faultdone(ep);
				return 0;
			}
		}
	}
	else if (ep->vme_kwire) {
		ep->vme_kwire++;
		vm_map_unlock(ep->vme_map);
		return 0;
	}
	else if (obj->ob_type == OT_PKERNEL) {
		kern_return_t ret;

		/*
		 * Take the fault lock so that the only additional
		 * activity is fault activity which can be handled
		 * in the fault code.
		 */

		vm_mape_fault(ep);
		ep->vme_kwire = 1;
		vm_map_unlock(ep->vme_map);
		ret = k_mem_fault(ep, addr, len, VM_PROT_READ|VM_PROT_WRITE, 
				VM_WIRE, (vm_page_t *) 0);
		vm_mape_faultdone(ep);
		return ret;
	}
	else panic("k_mem_lockop: attempting to wire with bad VM object");
}

int
k_mem_prot(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t size,
	vm_prot_t prot)
{

	if ((prot & ep->vme_protection) != prot) {
		k_mem_entry_adjust(ep, addr, size, FALSE);
		pmap_protect(ep->vme_map, ep->vme_start, addr + size, prot);
		ep->vme_protection = prot;
	}
	return 0;
}

boolean_t
k_mem_check_prot(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register vm_size_t size,
	register vm_prot_t prot)
{
	if ((ep->vme_protection & prot) != prot) return FALSE;
	else return TRUE;
}

/*
 * Don't kluster for the kernel.
 */

int
k_kem_kluster(register vm_map_entry_t ep,
	vm_offset_t addr,
	int pcnt,
	vm_offset_t *back, 
	vm_offset_t *forward)
{
	*back = *forward = ep->vme_offset + (addr - ep->vme_start);
	return 0;
}

/*
 * We enter with the map write locked. So care
 * must be taken not to sleep within here.
 */

k_mem_grow(register vm_map_entry_t ep,
	vm_prot_t prot,
	register vm_size_t size,
	as_grow_t direction)
{
	register vm_size_t oldsize, psize;
	register vm_offset_t base;
	register struct vm_anon ***appp, **app; 

	if (ep->vme_kwire || ep->vme_protection != prot ||
		(direction == AS_GROWDOWN && ep->vme_object != pkernel_object)) 
		return KERN_NO_SPACE;

	psize = atop(size);
	oldsize = atop(ep->vme_end - ep->vme_start);
	
	base = (vm_offset_t) 
		h_kmem_zalloc_memory((oldsize + (psize << 1)) * sizeof app,
			FALSE);
	if (base == (vm_offset_t) 0) return KERN_NO_SPACE;
	else if (direction == AS_GROWUP) {
		bcopy(ep->vme_private, base, oldsize * sizeof appp);
		ep->vme_end += size;
		appp = (struct vm_anon ***) (base + oldsize * sizeof appp);
	}
	else {
		bcopy(ep->vme_private, base + psize * sizeof appp,
			oldsize * sizeof appp);
		ep->vme_start -= size;
		ep->vme_offset -= size;
		appp = (struct vm_anon ***) base;
	}
	h_kmem_free_memory(ep->vme_private, oldsize * sizeof appp, FALSE);
	ep->vme_private = base;
	app = (struct vm_anon **) (base + (psize + oldsize) * sizeof appp);
	
	while (psize--) *appp++ = app++;
	return KERN_SUCCESS;
}

/*
 * Adjust map entry and account for vme_private area.
 * Other changes will be made by clip start and
 * clip end operations. Also clear the fault lock in
 * all new entries.
 */

k_mem_entry_adjust(register vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t len,
	boolean_t free)
{
	register vm_offset_t base, start, end;

	start = ep->vme_start;
	end = ep->vme_end;
	if (ep->vme_object->ob_type == OT_PKERNEL) {
		register struct vm_anon ***appp;

		appp = (struct vm_anon ***) (ep->vme_private);
		vm_map_clip_start(ep->vme_map, ep, addr);
		vm_map_clip_end(ep->vme_map, ep, addr + len);
		if ((addr + len) < end) 
			ep->vme_next->vme_private = (vm_offset_t) (appp + 
				atop((addr + len) - start));
		if (addr > start) 
			ep->vme_private = (vm_offset_t) (appp +
				atop(addr - start));
		if (free == TRUE) k_mem_free_anon(ep);
	}
	else {
		vm_map_clip_start(ep->vme_map, ep, addr);
		vm_map_clip_end(ep->vme_map, ep, addr + len);
	}

	if ((addr + len) < end) {
		ep->vme_next->vme_faults = 0;
		ep->vme_next->vme_want = 0;
	}
	if (addr > start) {
		ep->vme_prev->vme_faults = 0;
		ep->vme_prev->vme_want = 0;
	}
}

k_mem_free_anon(register vm_map_entry_t ep)
{
	register vm_offset_t base;
	struct vm_anon ***appp, **app;
	register struct vm_anon *ap;
	register vm_size_t count, size;
	register vm_offset_t offset, addr;
	register vm_object_t object;
	vm_page_t pp;
	static char p_remove_failure[] = "k_mem_free_anon: free inconsistency";

	appp = (struct vm_anon ***) (ep->vme_private);
	if (appp) app = *appp;
	else app = (struct vm_anon **) 0;
	offset = ep->vme_offset;
	object = ep->vme_object;
	size = ep->vme_end - ep->vme_start;

	for (addr = ep->vme_start, count = 0; size; 
		size -= PAGE_SIZE, offset += PAGE_SIZE, addr += PAGE_SIZE) {
		
		/*
		 * Check to see if an anon element was allocated
		 */

		vm_object_lock(object);
		if (app && (ap = *app)) {
			vm_object_unlock(object);
			a_lock(ap);
			if (pp = ap->an_page) {
				if (!ep->vme_kwire) {
					if (vm_page_hold(pp) == TRUE) {
						a_unlock(ap);
						vm_page_wait(pp);	
						a_lock(ap);
					}
					vm_page_release(pp);

					if (vm_pageout_remove(pp, TRUE, TRUE) 
						== FALSE)
						panic(p_remove_failure);
					a_unlock(ap);
				}
				else {
					pmap_change_wiring(
						vm_map_pmap(ep->vme_map), 
						addr, FALSE);	
					pmap_page_protect(page_to_phys(pp), 
						VM_PROT_NONE);
					vm_page_unwire(pp, FALSE);
					a_unlock(ap);
				}


				vm_anon_page_free((struct vm_anon_object *) 0,
					(vm_offset_t) 0, ap);
			} 
			else a_unlock(ap);
			a_swap_free(ap, TRUE);
		}
		else {
			pp = vm_page_lookup(object, offset);
			if (pp != VM_PAGE_NULL) {
				if (!ep->vme_kwire) {
					if (vm_pageout_remove(pp, TRUE, TRUE) 
						== FALSE)
						panic(p_remove_failure);
					vm_page_remove(pp);
					vm_pg_free(pp);
				}
				else {
					pmap_change_wiring(
						vm_map_pmap(ep->vme_map), 
						addr, FALSE);	
					pmap_page_protect(page_to_phys(pp), 
						VM_PROT_NONE);
					vm_page_wire_free(pp);
				}
			}
			else if (ep->vme_kwire) 
				panic("k_mem_anon_free: page not found");
			vm_object_unlock(object);
		}

		if (!appp) continue;

		if (!count) {
			base = (vm_offset_t) app;
			count = 1;
		}
		else if ((base + (count * sizeof app)) == (vm_offset_t) app) 
			count++;
		else {
			h_kmem_free_memory(base, count * sizeof app, FALSE);
			count = 1;
			base = (vm_offset_t) app;
		}
		appp++; 
		app = *appp; 
	}

	if (count) h_kmem_free_memory(base, count * sizeof app, FALSE);

	/*
	 * The app and ap have been freed now free the appp array.
	 */

	if (appp) h_kmem_free_memory(ep->vme_private, 
		atop(ep->vme_end - ep->vme_start) * sizeof appp, FALSE);

	if (ep->vme_kwire) {
		assert(ep->vme_kwire == 1);
		pmap_pageable(vm_map_pmap(ep->vme_map), ep->vme_start,
			ep->vme_end, TRUE);
		ep->vme_kwire = 0;
	}
}

/*
 * A new map entry is allocated.
 * The pageable specific operations to the allocation
 * are completed here.  The entry is protected from
 * any activity.
 */

k_mem_grow_pageable(register vm_map_entry_t ep)
{
	register int i;
	register struct vm_anon ***appp, **app;
	vm_offset_t base;

	i = atop(ep->vme_end - ep->vme_start);
	base = (vm_offset_t) h_kmem_zalloc((i << 1) * sizeof app);
	appp = (struct vm_anon ***) base;
	app = (struct vm_anon **) (base + i * sizeof app);	
	ep->vme_private = base;
	while (i--) *appp++ = app++;
}

k_mem_badop()
{
	panic("k_mem_badop: operation not supported");
}

kern_return_t
kernel_object_pagesteal()
{
	return KERN_FAILURE;
}

kernel_object_terminate(vm_object_t obj)
{
	panic("kernel_object_terminate: kernel object reference went to zero");
}

kernel_object_bad() 
{
	(void) panic("kernel_object_bad: bad operation");
}


pkernel_oops_klock_try(vm_object_t obj, 
	vm_offset_t offset)
{
	return vm_object_lock_try(obj);
}

/*
 * This code attempts to allocate the anon pointer for the page.
 */

pkernel_oops_pageout(register vm_object_t obj,
	vm_offset_t offset,
	vm_size_t size)
{
	register vm_page_t pp;
	register struct vm_anon *ap;
	register struct vm_swap *sp;

	pp = vm_page_lookup(obj, offset);
	if (vm_pageout_remove(pp, TRUE, TRUE) == FALSE) {
		vm_pageout_abort(pp);
		vm_object_unlock(obj);
		return 0;
	}
	else if (!pp->pg_dirty) {
		vm_page_remove(pp);
		vm_object_unlock(obj);
		vm_pg_free(pp);
		return 0;
	}
	else if ((ap = a_swap_alloc(FALSE, FALSE, ANT_SWAP, 
			(struct vm_swap *) 0)) == (struct vm_anon *) 0) {
		vm_pageout_abort(pp);
		vm_object_unlock(obj);
		return 0;
	}
	else {
		register vm_object_t sop;
		vm_offset_t soffset;
		vm_page_t pagelist[1];
		

		sp = a_aptosp(ap, &soffset);
		vm_page_remove(pp);
		sop = (vm_object_t) (sp->vs_object);
		pp->pg_object = sop;
		pp->pg_offset = soffset;
		vm_object_lock(sop);
		vm_page_insert_bucket(pp, sop, soffset);
		vm_object_unlock(sop);
		*(pp->pg_app) = ap;
		pp->pg_app = (struct vm_anon **) 0;
		ap->an_page = pp;
		AN_TRACK_WRITE(ap);

		vm_object_unlock(obj);

		pagelist[0] = pp;
		pp->pg_pnext = VM_PAGE_NULL;
		vm_swap_io(pagelist, B_WRITE);
		return 1;
	}
}

 
struct vm_object_ops kernel_object_oops = {
	OOP_KLOCK_TRY_K 
	&kernel_object_bad,		/* klock try */
	OOP_UNKLOCK_K
	&kernel_object_bad,		/* unlock */
	&vm_ops_reference_def,		/* reference */
	&vm_ops_deallocate_def,		/* deallocate */
	OOP_PAGEIN_K
	&kernel_object_bad,		/* pagein */
	OOP_PAGEOUT_K
	&kernel_object_bad,		/* pageout */
	OOP_SWAP_K
	&kernel_object_bad,		/* swap */
	OOP_CONTROL_K
	&kernel_object_bad,		/* control */
	OOP_PAGECTL_K
	&kernel_object_bad,		/* pagectl */
	OOP_PAGESTEAL_K
	&kernel_object_pagesteal,	/* pagesteal */
};

struct vm_object_config kernel_object_conf = {
	(int (*)()) 0,
	&kernel_object_terminate,
	sizeof (struct vm_object),
	&kernel_object_oops,
	&k_mape_ops_mem,
};


struct vm_object_ops pkernel_object_oops = {
	OOP_KLOCK_TRY_K
	&pkernel_oops_klock_try,	/* klock try */
	OOP_UNKLOCK_K
	&kernel_object_bad,		/* unlock */
	&vm_ops_reference_def,		/* reference */
	&vm_ops_deallocate_def,		/* deallocate */
	OOP_PAGEIN_K
	&kernel_object_bad,		/* pagein */
	&pkernel_oops_pageout,		/* pageout */
	OOP_SWAP_K
	&kernel_object_bad,		/* swap */
	OOP_CONTROL_K
	&kernel_object_bad,		/* control */
	OOP_PAGECTL_K
	&kernel_object_bad,		/* pagectl */
	OOP_PAGESTEAL_K
	&kernel_object_pagesteal,	/* pagesteal */
};

struct vm_object_config pkernel_object_conf = {
	(int (*)()) 0,
	&kernel_object_terminate,
	sizeof (struct vm_object),
	&pkernel_object_oops,
	&k_mape_ops_mem,
};
