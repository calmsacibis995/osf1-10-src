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
static char *rcsid = "@(#)$RCSfile: vm_kmap.c,v $ $Revision: 1.1.17.4 $ (DEC) $Date: 1993/11/23 16:32:53 $";
#endif
#include <mach_ldebug.h>
#include <kern/assert.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <vm/vm_fault.h>
#include <vm/vm_control.h>
#include <vm/vm_lock.h>
#include <vm/vm_kmap.h>
#include <vm/vm_perf.h>

extern struct vm_map_entry_ops k_mape_ops_invalid;
extern struct vm_map_entry_ops k_mape_ops_io;
extern struct vm_map_entry_ops k_mape_ops_mem;
extern kern_return_t k_map_bad();


extern vm_map_entry_t k_map_insert(vm_map_t map, vm_map_entry_ops_t op,
		vm_object_t object, vm_offset_t offset,	vm_offset_t start, 
		vm_offset_t end, vm_prot_t prot,
		register vm_map_entry_t prev_entry);

extern void k_map_simplify(vm_map_t map, vm_offset_t start);

/*
 * Kernel map operations.
 */

extern int
	k_map_deallocate(),
	k_map_fault(),
	k_map_wire(),
	k_map_allocate(),
	k_map_protect(),
	k_map_delete(),
	k_map_check_protection(),
	k_map_fork();

struct vm_map_ops k_map_ops = {
	&k_map_deallocate,
	&k_map_fault,
	&k_map_wire,
	&k_map_allocate,
	&k_map_bad,
	&k_map_protect,
	&k_map_bad,
	&k_map_bad,
	&k_map_bad,
	&k_map_delete,
	&k_map_check_protection,
	&k_map_bad,
	&k_map_bad,
	&k_map_bad,
	&k_map_fork,
};


kern_return_t
k_map_bad()
{
	panic("k_map_bad: kernel map operation not supported");
}

k_map_create(register vm_map_t map)
{
	map->vm_ops = &k_map_ops;
	return KERN_SUCCESS;
}

kern_return_t
k_map_fault(vm_map_t map,
	vm_offset_t addr,
	vm_prot_t prot,
	vm_fault_t wire)
{
	vm_map_entry_t entry;
	kern_return_t ret;

	addr = trunc_page(addr);
retry:
	vm_map_lock_read(map);

	simple_lock(&map->vm_hint_lock);
	entry = map->vm_hint;
	simple_unlock(&map->vm_hint_lock);

	if (entry == vm_map_to_entry(map) ||
	    addr < entry->vme_start || addr >= entry->vme_end) {
		vm_map_entry_t	tmp_entry;

		if (!vm_map_lookup_entry(map, addr, &tmp_entry)) {
			vm_map_unlock_read(map);
			return KERN_INVALID_ADDRESS;
		}
		entry = tmp_entry;
	}

	if (entry->vme_is_submap) {
		vm_map_t oldmap = map;

		map = entry->vme_submap;
		vm_map_unlock_read(oldmap);
		if (entry->vme_copymap) 
			return (*map->vm_fault_map)(map, addr, prot, wire);
		else goto retry;
	}

	vm_mape_fault(entry);
	vm_map_unlock_read(map);
	vpf_ladd(kpagefaults,1);
	ret = (*entry->vme_fault)(entry, addr, PAGE_SIZE, prot, wire,
			(vm_page_t *) 0);
	vm_mape_faultdone(entry);
	return ret;
}

/*
 * Free a kernel submap
 */

kern_return_t
k_map_deallocate(vm_map_t map)
{
	k_map_delete(map, vm_map_min(map), vm_map_max(map), FALSE);
}

/*
 * General kernel allocator for 
 * obtaining kernel virtual space.
 */

k_map_allocate(vm_map_t map,
	vm_object_t object,
	vm_offset_t offset,
	vm_offset_t *addr,
	vm_size_t size,
	boolean_t find_space)
{
	if (find_space && 
		(*addr < vm_map_min(map) || *addr >= vm_map_max(map))) 
		*addr = vm_map_min(map);
	if (object->ob_type == OT_KERNEL) 
		return k_map_allocate_nonpaged(map, object, addr, size, 
			VM_PROT_READ|VM_PROT_WRITE, find_space);
	if (object == VM_OBJECT_NULL) object = pkernel_object;
	return k_map_allocate_paged(map, object, addr, size, 
			VM_PROT_READ|VM_PROT_WRITE, find_space);
}

/*
 * Allocate virtual memory for a map that
 * already has its virtual space allocated.
 * The addr can be found in a map entry.
 */

k_mem_allocate(vm_map_t submap,
	register vm_offset_t addr,
	vm_size_t size)
{
	register vm_offset_t offset, end;
	register vm_page_t pp;
	vm_map_entry_t tmp;
	register vm_map_entry_t entry;
	register vm_object_t object;

	end = addr + size;
	vm_map_lock(submap);
	while (addr < end) {
		if (!vm_map_lookup_entry(submap, addr, &tmp))
			(void) panic("k_mem_allocate: entry not found");
		entry = tmp;
		pmap_pageable(vm_map_pmap(submap), addr, 
			MIN(end, entry->vme_end), FALSE);
		assert((entry->vme_object != VM_OBJECT_NULL) && 
			(entry->vme_object.ob_type == OT_NULL));
		object = entry->vme_object;
		offset = entry->vme_offset + (addr - entry->vme_start);
		do {
			vm_object_lock(object);
			while ((pp = vm_zeroed_page_alloc(object, offset)) ==
				VM_PAGE_NULL) {
				vm_object_unlock(object);
				vm_wait();
				vm_object_lock(object);
			}
			vm_object_unlock(object);
			vm_page_wire(pp, FALSE);
			pmap_enter(vm_map_pmap(submap), addr, page_to_phys(pp),
				VM_PROT_READ|VM_PROT_WRITE, TRUE,
				VM_PROT_READ|VM_PROT_WRITE);
			addr += PAGE_SIZE;
			offset += PAGE_SIZE;
		} while (addr < end && addr < entry->vme_end);
	}
	vm_map_unlock(submap);
}

k_mem_free(vm_map_t submap,
	register vm_offset_t addr,
	vm_size_t size)
{
	register vm_offset_t offset, end;
	vm_offset_t start;
	register vm_page_t pp;
	vm_map_entry_t tmp;
	register vm_map_entry_t entry;
	register vm_object_t object;

	end = addr + size;
	vm_map_lock(submap);
	while (addr < end) {
		if (!vm_map_lookup_entry(submap, addr, &tmp))
			(void) panic("k_mem_free: entry not found");
		entry = tmp;
		assert((entry->vme_object != VM_OBJECT_NULL) && 
			(entry->vme_object.ob_type == OT_NULL));
		object = entry->vme_object;
		offset = entry->vme_offset + (addr - entry->vme_start);
		start = addr;
		do {
			vm_object_lock(object);
			pp = vm_page_lookup(object, offset);
			if (pp == VM_PAGE_NULL) 
				panic("k_mem_free: page not found");
			pmap_change_wiring(vm_map_pmap(submap), addr, FALSE);
			pmap_remove(vm_map_pmap(submap), addr, 
				addr + PAGE_SIZE);
			vm_page_free(pp);
			vm_object_unlock(object);
			addr += PAGE_SIZE;
			offset += PAGE_SIZE;
		} while (addr < end && addr < entry->vme_end);
		vm_object_unlock(object);
		pmap_pageable(vm_map_pmap(submap), start, 
			MIN(end, entry->vme_end), TRUE);
	}
	vm_map_unlock(submap);
}

/*
 * These two routines are used to support kernel allocators
 * which always wire memory down and never fault on it.
 * 
 * Doing any map entry operations on this map will result in a panic.
 *
 */

kern_return_t
k_map_allocate_fast(vm_map_t submap,
	vm_object_t object,
	vm_offset_t *addr,
	vm_size_t size,
	boolean_t canwait)
{
	register vm_offset_t start;
	kern_return_t ret;
	vm_offset_t offset;
	register vm_page_t pglp;
	register vm_map_entry_t entry;
	vm_map_entry_t tmp_entry;
	boolean_t wait_for_space;

	*addr = vm_map_min(submap);
	if (canwait == FALSE) {
		if (lock_try_write(&submap->vm_lock) == FALSE) 
			return KERN_FAILURE;
		else lock_set_recursive(&submap->vm_lock);
		if (wait_for_space = submap->vm_wait_for_space) 
			submap->vm_wait_for_space = 0;
	}

	ret = vm_map_space(submap, addr, size, page_mask, TRUE, &tmp_entry);

	if (canwait == FALSE) {
		submap->vm_wait_for_space = wait_for_space;
		if (ret != KERN_SUCCESS) {
			lock_clear_recursive(&submap->vm_lock);	
			vm_map_unlock(submap);
		}
		else {
			vm_map_unlock(submap);
			lock_clear_recursive(&submap->vm_lock);	
		}
	}

	if (ret != KERN_SUCCESS) return ret;
	
	entry = tmp_entry;	
	start = *addr;
	offset = start - vm_map_min(submap);

	while ((pglp = (vm_page_t)
		vm_pages_alloc_private(object, offset, size)) == VM_PAGE_NULL 
			&& canwait == TRUE) vm_wait();
		

	if (pglp != VM_PAGE_NULL) {

		if (entry->vme_end == start) {
			entry->vme_end += size;
			submap->vm_size += size;
		}
		else if (entry->vme_next != vm_map_to_entry(submap) &&
			entry->vme_next->vme_start == (start + size)) {
			entry->vme_next->vme_start = start;
			submap->vm_size += size;
		}
		else k_map_insert(submap, &k_mape_ops_invalid,
			object, offset, start, start + size, 
			VM_PROT_READ|VM_PROT_WRITE, entry);

		do {
			pmap_enter(vm_map_pmap(submap), start, 
				page_to_phys(pglp),
				VM_PROT_WRITE|VM_PROT_READ, TRUE,
				VM_PROT_WRITE|VM_PROT_READ);
			start += PAGE_SIZE;
			pglp = pglp->pg_pnext;
		} while (pglp);

		vm_map_unlock(submap);
		return KERN_SUCCESS;
	}
	else {
		vm_map_unlock(submap);
		return KERN_RESOURCE_SHORTAGE;
	}
}

/*
 * Allocate an object with which we never plan
 * to require any object specific operations.
 */

kern_return_t
k_map_object_fast(vm_object_t *object, vm_size_t size)
{

	if (vm_object_allocate(OT_NULL, size, (caddr_t) 0, object) 
		!= KERN_SUCCESS) panic("k_map_wobject_allocate: no object");
	return KERN_SUCCESS;
}

/*
 *	vm_map_submap:		
 *
 *	Mark the given range as handled by a subordinate map.
 *
 *	The region of memory is carved from a kernel map.
 *
 *	This region of kernel virtual space can't be operated
 *	on by specifying the kernel_map.
 */

kern_return_t 
k_map_submap(register vm_map_t map, 
	vm_offset_t *start, 
	vm_offset_t *end, 
	vm_size_t size,
	register vm_map_t submap)
{
	kern_return_t ret;
	vm_map_entry_t entry;
	


	/*
	 * A submap's main map entry for it can't be operated on.
	 * A submap implies that the subsystem knows its
	 * a special map.  If it doesn't, then it never should have
	 * allocated the submap in the first place.
	 */

	ret = k_map_allocate_space(map, &k_mape_ops_invalid,
		vm_submap_object, start, size, TRUE);

	if (ret == KERN_SUCCESS) {
		
		vm_map_lock(map);
		if (!vm_map_lookup_entry(map, *start, &entry))
			(void) panic("k_map_submap: entry not found");
		*end = *start + size;	
		submap->vm_min_offset = *start;
		submap->vm_max_offset = *end;
		submap->vm_is_mainmap = 0;
		vm_map_reference(entry->vme_submap = submap);
		vm_map_unlock(map);
	}

	return ret;
}

/*
 * Create a new region of virtual space in a map.
 * The map is locked and the new spaces previous entry is prev_entry.
 * The object is assumed to be null.
 */

vm_map_entry_t 
k_map_insert(vm_map_t map, 
	vm_map_entry_ops_t op,
	vm_object_t object,
	vm_offset_t offset,	
	vm_offset_t start, 
	vm_offset_t end,
	vm_prot_t prot,
	register vm_map_entry_t prev_entry)
{
	vm_map_entry_t new_entry;


	/*
	 *	Create a new entry for the kernel
	 */

	vm_map_entry_create(map, &new_entry);
	new_entry->vme_map = map;
	new_entry->vme_start = start;
	new_entry->vme_end = end;

	if (object == vm_submap_object) {
		new_entry->vme_is_submap = TRUE;
		new_entry->vme_object = VM_OBJECT_NULL;
		new_entry->vme_offset = (vm_offset_t) 0;
	}
	else {
		new_entry->vme_is_submap = FALSE;
		new_entry->vme_object = object;
		new_entry->vme_offset = offset;
	}
	new_entry->vme_ops = op;


	new_entry->vme_protection = prot;
	new_entry->vme_kwire = 0;

	/*
	 *	Insert the new entry into the list
	 */

	vm_map_entry_link(map, prev_entry, new_entry);
	map->vm_size += new_entry->vme_end - new_entry->vme_start;

	/*
	 *	Update the free space hint
	 */

        if ((map->vm_first_free == prev_entry) &&
                (((prev_entry == vm_map_to_entry(map)) &&
                  (prev_entry->vme_start == new_entry->vme_start))
                ||
                 ((prev_entry != vm_map_to_entry(map)) &&
                  (prev_entry->vme_end >= new_entry->vme_start))))
                map->vm_first_free = new_entry;

	return new_entry;
}

/*
 * Allocate space in a map without any object backing it.
 * No map simplification are performed.  This is intended for the
 * kernel which might not have objects backing it.  For example I/O
 * space and double mapping stuff.
 */

kern_return_t
k_map_allocate_va(
	vm_map_t map,
	register vm_object_t object,
	vm_offset_t *addr,
	vm_size_t length,
	boolean_t find_space)
{
	register struct vm_map_entry_ops *mop;

	if (object == VM_OBJECT_NULL) {
		assert(map == kernel_map);
		object = nkernel_object;
		mop = &k_mape_ops_io;
	}
	else mop = vm_object_to_vmeops(object);

	return k_map_allocate_space(map, mop, object, addr, length, find_space);
}

/*
 * Allocate kernel space with object possibly backing it.
 */

kern_return_t
k_map_allocate_space(
	vm_map_t map,
	vm_map_entry_ops_t ops,
	vm_object_t object,
	vm_offset_t *addr,
	vm_size_t length,
	boolean_t find_space) 
{
	vm_map_entry_t entry;
	vm_offset_t start;
	kern_return_t result;
	register vm_offset_t offset;
	

	assert(object != VM_OBJECT_NULL);
	start = *addr;
	result = vm_map_space(map, &start, length, page_mask, 
				find_space, &entry);
	if (result != KERN_SUCCESS) return result;
	*addr = start;
	offset = start - vm_map_min(map);
	vm_object_reference(object);
	k_map_insert(map, ops, object, offset, start, start + length, 
			VM_PROT_READ|VM_PROT_WRITE, entry);
	vm_map_unlock(map);
	return KERN_SUCCESS;
}

/*
 * Wire down kernel memory.  
 * The algorithm is to operate on one map entry at a time.
 */

kern_return_t 
k_map_wire(
	vm_map_t map, 
	register vm_offset_t start, 
	register vm_offset_t end, 
	vm_prot_t access_type)
{
	vm_map_entry_t tmp_entry;
	register vm_map_entry_t entry;
	register vm_size_t size;
	vm_fault_t wire;



	assert((start < end));
	wire = (access_type != VM_PROT_NONE) ? VM_WIRE: VM_UNWIRE;

	while (start < end) {
		vm_map_lock(map);
		if (!vm_map_lookup_entry(map, start, &tmp_entry)) 
			panic("k_map_wire: entry not found");
		entry = tmp_entry;
		if (start < entry->vme_start || entry == vm_map_to_entry(map)) 
				panic("k_map_wire: a hole");
		else if (entry->vme_is_submap) panic("k_map_wire: submap");

		size = MIN(entry->vme_end, end) - start;
		if (wire == VM_WIRE && 
			(*entry->vme_check_protect)(entry, start, 
				size, access_type) == FALSE) 
				panic("k_map_wire: protection");
		vm_mape_faultlock(entry, continue);
		if ((*entry->vme_lockop)(entry, start, size, wire) 
				!= KERN_SUCCESS)
			(wire == VM_WIRE) ? 
				panic("k_map_wire: failed to wire")
				:
				panic("k_map_wire: failed to unwire");
		entry = entry->vme_next;
		start += size;
	}

	return KERN_SUCCESS;
	
}


/*
 * Allocate non-pageable kernel memory.
 */

kern_return_t
k_map_allocate_nonpaged(vm_map_t map, 
	register vm_object_t obj,
	vm_offset_t *addr, 
	vm_size_t size, 
	vm_prot_t prot,
	boolean_t anywhere)
{
	kern_return_t result;
	vm_map_entry_t entry;
	register vm_map_entry_t new_entry;
	vm_offset_t start;
	register vm_offset_t offset, i;
	register vm_page_t pp;


	start = *addr;
	result = vm_map_space(map, &start, size, page_mask, 
				anywhere, &entry);
	if (result != KERN_SUCCESS) return result;

	*addr = start;
	vm_object_reference(obj);
	if (obj != kernel_object) offset = start - vm_map_min(map);
	else offset = start - vm_map_min(kernel_map);

	/*
	 * All access to this (these) page (pages) is prevented
	 * by inserting invalid operation handles.
	 */

	new_entry = k_map_insert(map, &k_mape_ops_invalid, obj,
			offset, start, start + size, prot, entry);

	vm_map_unlock(map);

	pmap_pageable(vm_map_pmap(map), start, start + size, FALSE);

	for (i = 0 ; i < size; i+= PAGE_SIZE, start += PAGE_SIZE) {

		vm_object_lock(obj);
		while ((pp = vm_zeroed_page_alloc(obj, offset+i))
			    == VM_PAGE_NULL) {
			vm_object_unlock(obj);
			vm_wait();
			vm_object_lock(obj);
		}
		vm_object_unlock(obj);
		vm_page_wire(pp, FALSE);
		pmap_enter(vm_map_pmap(map), start, page_to_phys(pp),
			VM_PROT_READ|VM_PROT_WRITE, TRUE,
			VM_PROT_READ|VM_PROT_WRITE);
	}

	/*
	 * Update the new entries wire count
	 * change the operation handles to normal ones.
	 */

	vm_map_lock(map);
	new_entry->vme_kwire = 1;
	new_entry->vme_ops = &k_mape_ops_mem;
	vm_map_unlock(map);

	k_map_simplify(map, *addr);
	return KERN_SUCCESS;
}

/*
 * Allocate pageable kernel memory
 */

kern_return_t
k_map_allocate_paged(vm_map_t map, 
	register vm_object_t obj,
	vm_offset_t *addr, 
	vm_size_t size, 
	vm_prot_t prot,
	boolean_t anywhere)
{
	kern_return_t result;
	vm_map_entry_t prev_entry;
	register vm_map_entry_t new_entry, next;
	vm_offset_t offset, start;

retry:
	start = *addr;
	result = vm_map_space(map, &start, size, page_mask, 
				anywhere, &prev_entry);
	if (result != KERN_SUCCESS) return result;


	/*
	 * The map is write locked
	 */

	if (prev_entry->vme_end == start && prev_entry->vme_object == obj) {
		vm_mape_faultlock(prev_entry, goto retry);
		vm_mape_fault(prev_entry);
		if ((*prev_entry->vme_grow)(prev_entry, prot, size, AS_GROWUP) 
				== KERN_SUCCESS) {
			map->vm_size += size;
			vm_map_unlock(map);
			vm_mape_faultdone(prev_entry);
			*addr = start;
			return KERN_SUCCESS;
		}
		else vm_mape_faultdone(prev_entry);
	}
	else if ((next = prev_entry->vme_next) != vm_map_to_entry(map) &&
		next->vme_start == (start + size) && next->vme_object == obj) {
		vm_mape_faultlock(next, goto retry);
		vm_mape_fault(next);
		if ((*next->vme_grow)(next, prot, size, AS_GROWDOWN)
				== KERN_SUCCESS) {
			map->vm_size += size;
			vm_map_unlock(map);
			vm_mape_faultdone(next);
			*addr = start;
			return KERN_SUCCESS;
		}
		else vm_mape_faultdone(next);
	}

	*addr = start;
	vm_object_reference(obj);
	if ((vm_map_max(map) - vm_map_min(map)) == obj->ob_size) 
		offset = start - vm_map_min(map);
	else {
		assert((obj == pkernel_object));
		offset = start - VM_MIN_KERNEL_ADDRESS;
	}

	new_entry = k_map_insert(map, &k_mape_ops_invalid, obj,
			offset, start, start + size, prot, prev_entry);
	vm_map_unlock(map);

	k_mem_grow_pageable(new_entry);

	vm_map_lock(map);
	new_entry->vme_ops = &k_mape_ops_mem;
	vm_map_unlock(map);
	return KERN_SUCCESS;
}

/*
 * We enter with the map locked.
 */

kern_return_t 
k_map_delete(register vm_map_t map, 
	register vm_offset_t start, 
	register vm_offset_t end,
	boolean_t contain)
{
	register vm_map_entry_t entry;
	vm_map_entry_t tmp_entry;
	register vm_size_t size;
	register boolean_t first;

	first = TRUE;

	while (start < end) {
		if (!vm_map_lookup_entry(map, start, &tmp_entry)) goto fail;
		else entry = tmp_entry;
		if (entry->vme_is_submap) goto fail;
		size = MIN(entry->vme_end, end) - start;
		vm_mape_faultlock(entry, 
			{
				vm_map_lock(map);
				continue;
			}
		);
		vm_mape_fault(entry);
		if ((*entry->vme_unmap)(entry, start, size) != KERN_SUCCESS)
			goto fail;

		/*
		 * Acquire the map write lock 
		 */

		vm_map_lock(map);

		if (first) {
			first = FALSE;
			SAVE_HINT(map, entry->vme_prev);
			if (map->vm_first_free->vme_start >= start) 
				map->vm_first_free = entry->vme_prev;
		}

		/*
		 * Clear the fault hold condition
		 */

		vm_mape_faultdone(entry);
		vm_map_entry_delete(map, entry);
		map->vm_size -= size;
		start += size;
	}
	
	if (map->vm_wait_for_space)
		thread_wakeup((vm_offset_t) map);
	return KERN_SUCCESS;
fail:
	/*
	 * Normally its the caller's responsibility
	 * to release the map lock.  Since were going
	 * down we'll release it here to avoid hangs
	 * during the panic.
	 */

	vm_map_unlock(map);
	panic("k_map_delete: unable to delete kernel space");
}

/*
 * Allow protection check to span a submap. 
 */

boolean_t
k_map_check_protection(
	vm_map_t map, 
	vm_offset_t start, 
	vm_offset_t end, 
	vm_prot_t protection)
{
	register vm_map_entry_t	entry;
	vm_map_entry_t tmp_entry;
	register vm_size_t size;

retry:
	vm_map_lock_read(map);

	if (!vm_map_lookup_entry(map, start, &tmp_entry)) goto bad;
	entry = tmp_entry;

	while (start < end) {
		if (entry == vm_map_to_entry(map) || 
			start < entry->vme_start) goto bad;
		else if (entry->vme_is_submap) {
			vm_map_t oldmap;
			oldmap = map;
			map = entry->vme_submap;
			vm_map_unlock_read(oldmap);
			goto retry;
		}
		size = MIN(entry->vme_end, end) - start;
		if ((*entry->vme_check_protect)(entry, start,
			size, protection) != TRUE) goto bad;
		start = entry->vme_end;
		entry = entry->vme_next;
	}
	vm_map_unlock_read(map);
	return TRUE;
bad:
	vm_map_unlock_read(map);
	return FALSE;
}

kern_return_t 
k_map_protect(register vm_map_t map, 
	register vm_offset_t start, 
	register vm_offset_t end, 
	register vm_prot_t new_prot, 
	register boolean_t set_max)
{
	register vm_map_entry_t	current;
	register vm_size_t size;
	vm_map_entry_t tmp_entry;
	kern_return_t ret;

	/*
	 * Currently don't support set_max.
	 * If and when set_max is implemented then, vme_prot must change
	 * or instead of overloading vme_prot have a new handle
	 * vme_setmaxprot.
	 */

	if (set_max == TRUE) return KERN_SUCCESS;

	while (start < end) {
		vm_map_lock(map);
		if (!vm_map_lookup_entry(map, start, &tmp_entry)) goto bad2;
		current = tmp_entry;
		if (current->vme_is_submap)  goto bad2;

		vm_mape_faultlock(current, continue);
		vm_mape_fault(current);
		vm_map_unlock(map);

		size = MIN(end, current->vme_end) - start;

		/*
		 * Change the protection
		 */

		if ((ret = (*current->vme_protect)(current, start, size,
			 new_prot)) != KERN_SUCCESS)
			goto bad2;

		vm_mape_faultdone(current);
		start += size;
	}

	vm_map_unlock(map);
	return KERN_SUCCESS;	
bad:
	vm_mape_faultdone(current);
	vm_map_unlock(map);
	return ret;
bad2:
	panic("k_map_protect: kernel virtual space void detected");
	
}

/*
 * Attempt to simplify the kernel map.
 */

void 
k_map_simplify(vm_map_t map, vm_offset_t start)
{
	vm_map_entry_t	this_entry;
	vm_map_entry_t	prev_entry;

	vm_map_lock(map);
	if (
		(vm_map_lookup_entry(map, start, &this_entry)) &&
		((prev_entry = this_entry->vme_prev) != vm_map_to_entry(map)) &&
		(prev_entry->vme_end == start) &&
		(prev_entry->vme_is_submap == FALSE) &&
		(this_entry->vme_is_submap == FALSE) &&
		(prev_entry->vme_protection == this_entry->vme_protection) &&
		(prev_entry->vme_kwire == this_entry->vme_kwire) &&
		(prev_entry->vme_object == this_entry->vme_object) &&
		((prev_entry->vme_offset + 
			(prev_entry->vme_end - prev_entry->vme_start))
		     == this_entry->vme_offset)
	) {
		if (map->vm_first_free == this_entry)
			map->vm_first_free = prev_entry;

		SAVE_HINT(map, prev_entry);
		vm_map_entry_unlink(map, this_entry);
		prev_entry->vme_end = this_entry->vme_end;
	 	vm_object_deallocate(this_entry->vme_object);
		vm_map_entry_dispose(map, this_entry);
	}
	vm_map_unlock(map);
}

k_map_fork(vm_map_t oldmap, vm_map_t newmap)
{
	return KERN_SUCCESS;
}

