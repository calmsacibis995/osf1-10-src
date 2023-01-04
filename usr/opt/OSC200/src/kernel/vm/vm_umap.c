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
static char *rcsid = "@(#)$RCSfile: vm_umap.c,v $ $Revision: 1.1.25.2 $ (DEC) $Date: 1994/03/02 00:15:42 $";
#endif
#include <mach_ldebug.h>
#include <kern/assert.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <vm/vm_umap.h>
#include <vm/vm_fault.h>
#include <vm/vm_control.h>
#include <vm/vm_lock.h>
#include <vm/vm_tune.h>
#include <vm/heap_kmem.h>

/*
 * User map operations.
 */

extern int
	u_map_deallocate(),
	u_map_fault(),
	u_map_wire(),
	u_map_allocate(),
	u_map_enter(),
	u_map_protect(),
	u_map_inherit(),
	u_map_keep_on_exec(),
	u_map_exec(),
	u_map_delete(),
	u_map_check_protection(),
	u_map_copy_overwrite(),
	u_map_copyout(),
	u_map_copyin(),
	u_map_fork();

struct vm_map_ops u_map_ops = {
	&u_map_deallocate,
	&u_map_fault,
	&u_map_wire,
	&u_map_allocate,
	&u_map_enter,
	&u_map_protect,
	&u_map_inherit,
	&u_map_keep_on_exec,
	&u_map_exec,
	&u_map_delete,
	&u_map_check_protection,
	&u_map_copy_overwrite,
	&u_map_copyout,
	&u_map_copyin,
	&u_map_fork,
};

extern void u_map_free_vas(vm_map_t map, vm_size_t length);

/*
 * map initialization for user mode maps.
 */

void
u_map_init()
{
}

kern_return_t
u_map_fault(vm_map_t map,
	vm_offset_t addr,
	vm_prot_t prot,
	vm_fault_t wire)
{
	vm_map_entry_t entry;
	kern_return_t ret;
	vm_size_t size;

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

	vm_mape_fault(entry);
	vm_map_unlock_read(map);

	size = addr - (addr & ~page_mask) + 1;
	addr = trunc_page(addr);
	ret = (*entry->vme_fault)(entry, addr, size, prot,
				wire, (vm_page_t *) 0);
	map->vm_pagefaults++;
	vm_mape_faultdone(entry);
	return ret;
}

kern_return_t
u_map_deallocate(vm_map_t map)
{
	((struct u_map_private *) (map->vm_private))->um_unload_all = 1;
	u_map_delete(map, vm_map_min(map), vm_map_max(map), FALSE);
	vl_remove(map);
	lw_remove(map);
	h_kmem_free((caddr_t) (map->vm_private), 
		sizeof (struct u_map_private));
	return KERN_SUCCESS;
}

kern_return_t
u_map_create(register vm_map_t map)
{
	struct u_map_private *up;
	static char *message = "u_map_create: depletion reboot";

	map->vm_umap = 1;
	up = (struct u_map_private *)
		h_kmem_zalloc(sizeof (struct u_map_private));
	if (up == NULL) {
		uprintf(message); printf(message);
		return KERN_RESOURCE_SHORTAGE;
	}
	up->um_maxvas = vm_tune_value(maxvas);
	up->um_maxwired = vm_tune_value(maxwire);
	usimple_lock_init(&up->um_resource);
	(struct u_map_private *)(map->vm_private) = up;
	map->vm_ops = &u_map_ops;
	return KERN_SUCCESS;
}

/*
 * Default anonymous memory allocator for umaps
 */

kern_return_t
u_map_allocate(
	vm_map_t map,
	vm_object_t object,
	vm_offset_t offset,
	vm_offset_t *addr, 			/* IN/OUT */
	vm_size_t length,
	boolean_t find_space)
{
	return u_map_grow(map, object, offset, addr,
			length, find_space, AS_GROWANY);
}

kern_return_t
u_map_grow(
	vm_map_t map,
	vm_object_t object,
	vm_offset_t offset,
	vm_offset_t *addr, 			/* IN/OUT */
	vm_size_t size,
	boolean_t find_space,
	as_grow_t grow)
{
	vm_map_entry_t	new_entry;
	vm_map_entry_t prev_entry;
	vm_offset_t start, end;
	kern_return_t ret;
	register struct u_map_private *up;
	boolean_t lock_future;

	if ((ret = u_map_grow_vas(map, size)) != KERN_SUCCESS)
		goto nolockfail;

	if (find_space && *addr > map->vm_max_offset) start = vm_map_min(map);
	else start = *addr;

	ret = vm_map_space(map, &start, size, page_mask,
				find_space, &prev_entry);

	if (ret != KERN_SUCCESS) {
		u_map_free_vas(map, size);
		goto nolockfail;
	}

	up = (struct u_map_private *) (map->vm_private);
	lock_future = up->um_lock_future;
	end = start + size;


	/*
	 * Now the map is write locked and any failures
	 * in u_map_expand result in the object's deallocation
	 */

	*addr = start;

	/*
	 * First see if the predecessor can be expanded
	 * because of anonymous memory expansion.
	 */

	if (object == VM_OBJECT_NULL &&
		(ret = u_map_entry_grow(map, &prev_entry, VM_OBJECT_NULL,
			(vm_offset_t)0, start, end, VM_INHERIT_DEFAULT,
			VM_PROT_ALL, VM_PROT_READ|VM_PROT_WRITE, grow))
				== KERN_SUCCESS) {

		/*
		 * Don't deallocate the object because the
		 * object was expanded and the reference count
		 * didn't increase.
		 */

		if (lock_future) {
			ret = (*prev_entry->vme_lockop)
				(prev_entry, start, end - start, VM_WIRE);
			if (ret != KERN_SUCCESS)
				u_map_delete(map, start, end, TRUE);
		}
		vm_map_unlock(map);
		return ret;
	}

	/*
	 * Allocate an anonymous object if required
	 */

	if (object == VM_OBJECT_NULL) {
		ret = a_object_allocate(map, &object, size);
		if (ret != KERN_SUCCESS) goto failed;
	}


	/*
	 *	Create a new entry
	 */

	if (ret = vm_map_entry_create(map, &new_entry)) {
		goto failed;
	}

	new_entry->vme_map = map;
	new_entry->vme_start = start;
	new_entry->vme_end = end;

	new_entry->vme_keep_on_exec = FALSE;
	new_entry->vme_inheritance = VM_INHERIT_COPY;
	new_entry->vme_object = object;
	new_entry->vme_offset = offset;
	new_entry->vme_ops = vm_object_to_vmeops(object);
	new_entry->vme_protection = VM_PROT_READ|VM_PROT_WRITE;
	new_entry->vme_maxprot = VM_PROT_ALL;


	/*
	 *	Insert the new entry into the list
	 */

	vm_map_entry_link(map, prev_entry, new_entry);

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


	if (lock_future) ret = u_map_lock_vas_expand(map, new_entry);
	else ret = KERN_SUCCESS;

	vm_map_unlock(map);
	return ret;

failed:
	u_map_free_vas(map, size);
	if (object != VM_OBJECT_NULL) OOP_DEALLOCATE(object);
	vm_map_unlock(map);
	return ret;
nolockfail:
	if (object != VM_OBJECT_NULL) OOP_DEALLOCATE(object);
	return ret;
}

kern_return_t
u_map_entry_grow(
	vm_map_t map,
	vm_map_entry_t *prev_entry,
	vm_object_t object,
	vm_offset_t offset,
	vm_offset_t start,
	vm_offset_t end,
	vm_inherit_t inherit,
	vm_prot_t max_prot,
	vm_prot_t cur_prot,
	as_grow_t grow)
{
	kern_return_t ret;
	register vm_map_entry_t entry;


	entry = *prev_entry;
	if (grow == AS_GROWDOWN) entry = entry->vme_next;
	else if (grow == AS_GROWANY) grow = AS_GROWUP;

	/* Fail if the growth requested doesn't match the existing entry. */
	if ((entry == vm_map_to_entry(map))
	    || ((grow == AS_GROWUP)
		? (entry->vme_end != start)
		: (entry->vme_start != end))
	    || entry->vme_keep_on_exec
	    || (entry->vme_maxprot != max_prot)
	    || (entry->vme_inheritance != inherit))
		return KERN_FAILURE;

	/* If there is an object, verify object-offset match. */
	if (object && ((entry->vme_object != object)
		       || ((grow == AS_GROWUP)
			   ? (entry->vme_end - entry->vme_start
			      + entry->vme_offset != offset)
			   : (end - start + offset != entry->vme_offset))))
		return KERN_FAILURE;

	vm_mape_faultlock(entry);
	if ((ret = (*entry->vme_grow)(entry, cur_prot,
				      end - start, grow)) == KERN_SUCCESS) {
		if (grow == AS_GROWDOWN) *prev_entry = entry;
	}
	return ret;
}


/*
 * Administer growth and freeing of virtual address space resource
 */

void
u_map_free_vas(register vm_map_t map, vm_size_t length)
{
	register struct u_map_private *up;

	up = (struct u_map_private *) (map->vm_private);
	usimple_lock(&up->um_resource);
	map->vm_size -= length;
	usimple_unlock(&up->um_resource);
}

kern_return_t
u_map_grow_vas(register vm_map_t map, vm_size_t length)
{
	int noexpand;
	register struct u_map_private *up;

	up = (struct u_map_private *) (map->vm_private);

	/*
	 * Check whether map can grow in size
	 */


	usimple_lock(&up->um_resource);
	if ((map->vm_size + length) > up->um_maxvas)
		noexpand = TRUE;
	else {
		noexpand = FALSE;
		map->vm_size += length;
	}
	usimple_unlock(&up->um_resource);
	if (noexpand) return KERN_RESOURCE_SHORTAGE;
	else return KERN_SUCCESS;
}


/*
 *	Routine:	u_vm_map_enter
 *
 *	Description:
 *		Allocate a range in the specified virtual address map.
 *		The resulting range will refer to memory defined by
 *		the given memory object and offset into that object.
 *
 *		Arguments are as defined in the vm_map call.
 *
 */

kern_return_t
u_map_enter(
	register vm_map_t map,
	vm_offset_t *address, 			/* IN/OUT */
	vm_size_t size,
	vm_offset_t mask,
	boolean_t anywhere,
	vm_object_t object,
	vm_offset_t offset,
	boolean_t copy,
	vm_prot_t cur_protection,
	vm_prot_t max_protection,
	vm_inherit_t inheritance)
{
	vm_map_entry_t prev_entry;
	vm_map_entry_t	new_entry;
	kern_return_t ret;
	boolean_t lock_future;
	register struct u_map_private *up;
	vm_offset_t start, end;

	if ((ret = u_map_grow_vas(map, size)) != KERN_SUCCESS)
		goto nolockfail;

	if ((ret = vm_map_space(map, address, size, mask,
		anywhere, &prev_entry)) != KERN_SUCCESS) {
		u_map_free_vas(map, size);
		goto nolockfail;
	}

	up = (struct u_map_private *) (map->vm_private);
	lock_future = up->um_lock_future;

	start = *address;
	end = start + size;

	if ((ret = u_map_entry_grow(map, &prev_entry, object, offset, start,
				end, VM_INHERIT_DEFAULT, max_protection,
				cur_protection, AS_GROWANY)) == KERN_SUCCESS) {
		if (lock_future) {
			ret = (*prev_entry->vme_lockop)
				(prev_entry, start, end - start, VM_WIRE);
			if (ret != KERN_SUCCESS)
				u_map_delete(map, start, end, TRUE);
		}
		vm_map_unlock(map);
		return ret;
	}

	if (object == VM_OBJECT_NULL) {
		ret = a_object_allocate(map, &object, size);
		if (ret != KERN_SUCCESS) goto failed;
	}


	/*
	 *	Create a new entry
	 */


	if (ret = vm_map_entry_create(map, &new_entry)) {
		goto failed;
	}

	new_entry->vme_start = start;
	new_entry->vme_end = end;

	new_entry->vme_keep_on_exec = FALSE;
	new_entry->vme_object = object;
	new_entry->vme_offset = offset;


	new_entry->vme_inheritance = inheritance;
	new_entry->vme_protection = cur_protection;
	new_entry->vme_maxprot = max_protection;
	new_entry->vme_map = map;
	new_entry->vme_ops = vm_object_to_vmeops(object);

	/*
	 *	Insert the new entry into the list
	 */

	vm_map_entry_link(map, prev_entry, new_entry);

	/*
	 *	Update the free space hint and the lookup hint
	 */

        if ((map->vm_first_free == prev_entry) &&
                (((prev_entry == vm_map_to_entry(map)) &&
                  (prev_entry->vme_start == new_entry->vme_start))
                ||
                 ((prev_entry != vm_map_to_entry(map)) &&
                  (prev_entry->vme_end >= new_entry->vme_start))))
                map->vm_first_free = new_entry;

	SAVE_HINT(map, new_entry);

	if (lock_future) ret = u_map_lock_vas_expand(map, new_entry);
	else ret = KERN_SUCCESS;


	vm_map_unlock(map);
	return ret;

failed:
	u_map_free_vas(map, size);
	if (object != VM_OBJECT_NULL) OOP_DEALLOCATE(object);
	vm_map_unlock(map);
	return ret;

nolockfail:
	if (object != VM_OBJECT_NULL) OOP_DEALLOCATE(object);
	return ret;
}

/*
 * Handle user space automatic wiring of VAS increase.
 */

kern_return_t
u_map_lock_vas_expand(register vm_map_t map,
	register vm_map_entry_t entry)
{
	register vm_size_t length;
	kern_return_t ret;

	length = entry->vme_end - entry->vme_start;
	ret = (*entry->vme_lockop)(entry, entry->vme_start, length, VM_WIRE);
	if (ret != KERN_SUCCESS)
		if(u_map_delete(map, entry->vme_start, entry->vme_end, TRUE)
		!= KERN_SUCCESS) panic("u_map_lock_vas_expand: delete");
	return ret;
}

/*
 * The kernel use's u_map_wire to wire down user memory.
 * This is considered a brief operation.  No other map operations
 * are allowed during this time.   It is assumed that any faulters
 * will release a map entry so that the memory can be wired down
 * by the kernel activity that requires it.
 */

kern_return_t
u_map_wire(
	vm_map_t map,
	register vm_offset_t start,
	register vm_offset_t end,
	vm_prot_t access_type)
{
	register vm_map_entry_t	entry, first_entry;
	vm_map_entry_t	start_entry;
	register vm_size_t size;
	enum vm_fault_t lockop;
	vm_offset_t first_addr;
	kern_return_t ret;


	vm_map_lock(map);

	if (!vm_map_lookup_entry(map, start, &start_entry)) {
		vm_map_unlock(map);
		return KERN_INVALID_ADDRESS;
	}

	first_entry = entry = start_entry;
	first_addr = start = trunc_page(start);
	end = round_page(end);

	while (start < end) {
		if (vm_map_to_entry(map) == entry || start < entry->vme_start) {
			ret = KERN_INVALID_ADDRESS;
			goto bad;
		}

		/* Dont take the fault lock for unwires.  If this is an isr thread the fault	*/
		/* completion may be comming up later and a block here will deadlock the thread	*/

		if (access_type != VM_PROT_NONE)
			vm_mape_faultlock(entry);

		size = MIN(end, entry->vme_end) - start;
		if (access_type == VM_PROT_NONE)
			ret = vl_unwire(entry, start, start + size);
		else if ((ret =
			vl_wire(entry, start, start + size, access_type)) !=
				KERN_SUCCESS) goto bad;
		start += size;
		entry = entry->vme_next;
	}

	vm_map_unlock(map);
	return KERN_SUCCESS;

	/*
	 * We failed some part of a wiring operation.
	 */

bad:
	switch (access_type) {
		case VM_PROT_READ:
		case VM_PROT_READ|VM_PROT_WRITE:
			access_type = VM_PROT_NONE;
			break;
		case VM_PROT_NONE:
		panic("u_map_wire: failed to unwire");
	}

	end = start;
	start = first_addr;
	entry = first_entry;

	while (start < end ) {
		size = MIN(end, entry->vme_end) - start;
		vl_unwire(entry, start, start + size);
		start += size;
		entry = entry->vme_next;
	}

	vm_map_unlock(map);
	return ret;
}

/*
 *	vm_map_lockvas:
 *
 *	Lock and Unlock user mappings.
 *	Not to be called by the kernel.
 */

kern_return_t
u_map_lockvas(vm_map_t map,
	register vm_offset_t start,
	register vm_offset_t end,
	int locktype)
{
	register vm_size_t size;
	int future;
	vm_offset_t first_addr;
	vm_map_entry_t first_entry, last_entry;
	register vm_map_entry_t entry;
	register struct u_map_private *up;
	boolean_t lockop, allspace;
	kern_return_t ret;


	up = (struct u_map_private *) (map->vm_private);

	switch (locktype) {
	case VML_FUTURE:
	case VML_NOFUTURE:
		vm_map_lock(map);
		if (locktype == VML_FUTURE) up->um_lock_future = 1;
		else up->um_lock_future = 0;
		vm_map_unlock(map);
		return KERN_SUCCESS;
	case VML_LOCK_ALL:
	case VML_LOCK_ALL|VML_FUTURE:
	case VML_LOCK_RANGE:
	case VML_LOCK_RANGE|VML_FUTURE:
	case VML_UNLOCK_ALL:
	case VML_UNLOCK_ALL|VML_NOFUTURE:
	case VML_UNLOCK_RANGE:
	case VML_UNLOCK_RANGE|VML_NOFUTURE:
		break;
	default:
		return KERN_INVALID_ARGUMENT;
	}


	/*
	 * Gain the write lock and keep it until we're finished.
	 */

	vm_map_lock(map);

	future = locktype & (VML_FUTURE|VML_NOFUTURE);
	locktype ^=  future;


	lockop = ((locktype & (VML_LOCK_ALL|VML_LOCK_RANGE)) != 0);
	allspace = ((locktype & (VML_LOCK_ALL|VML_UNLOCK_ALL)) != 0);

	/*
	 * Find the first entry.
	 */

	if (allspace) {
		entry = first_entry = vm_map_first_entry(map);
		if (entry == vm_map_to_entry(map)) {
			vm_map_unlock(map);
			return KERN_INVALID_ADDRESS;
		}
		first_addr = start = entry->vme_start;
		end = vm_map_last_entry(map)->vme_end;
	}

	else {
		first_addr = start = trunc_page(start);
		end = round_page(end);

		/*
		 * We're doing some range of virtual address space.
		 */

		if (!vm_map_lookup_entry(map, start, &first_entry)) {
			vm_map_unlock(map);
			return KERN_INVALID_ADDRESS;
		}
		else entry = first_entry;
	}



	while (start < end) {
		if (start < entry->vme_start) {
			if (!allspace) {
				ret = KERN_INVALID_ADDRESS;
				goto failed;
			}
		}
		else if (vm_map_to_entry(map) == entry) {
			if (!allspace) {
				ret = KERN_INVALID_ADDRESS;
				goto failed;
			}
			else break;
		}

		size = MIN(end, entry->vme_end) - start;

		/*
		 * Wait for faults to complete
		 */

		vm_mape_faultlock(entry);

		if (lockop) {
			if ((ret =
			(*entry->vme_lockop)(entry, start, size, VM_WIRE)) !=
				KERN_SUCCESS) goto failed;
		}
		else {
			if ((ret =
			(*entry->vme_lockop)(entry, start, size, VM_UNWIRE)) !=
				KERN_SUCCESS) goto failed;
		}

		entry = entry->vme_next;
		if (allspace) start = entry->vme_start;
		else start += size;
	}

	if (future) {
		if (future & VML_FUTURE) up->um_lock_future = 1;
		else up->um_lock_future = 0;
	}
	vm_map_unlock(map);
	return KERN_SUCCESS;

failed:
	/*
	 * If we were locking undo all the lock taken
	 */

	if (lockop && (start != first_addr)) {
		end = start;
		entry = first_entry;
		start = first_addr;
		while (start < end) {
			if (entry == vm_map_to_entry(map)) break;
			size = MIN(end, entry->vme_end) - start;
			if ((*entry->vme_lockop)(entry, start, size, VM_UNWIRE)
				!= KERN_SUCCESS)
				panic("u_map_lockvas: lockvas undo failed");
			entry = entry->vme_next;
			if (allspace) start = entry->vme_start;
			else start += size;
		}
	}

	vm_map_unlock(map);
	return ret;
}

/*
 *	u_map_delete
 *
 *	On entry and exit the map is write locked.
 *
 *
 */

kern_return_t
u_map_delete(register vm_map_t map,
	register vm_offset_t start,
	register vm_offset_t end,
	boolean_t contain)
{
	register vm_size_t size;
	register vm_map_entry_t	entry, next;
	vm_map_entry_t	first_entry;
	kern_return_t ret;

	assert(start < end);



	/*
	 *	Find the start of the region.
	 */

	if (!vm_map_lookup_entry(map, start, &first_entry)) {
		if (contain == FALSE) {
			entry = vm_map_first_entry(map);
			while (entry != vm_map_to_entry(map)) {
				if (start >= entry->vme_end)
					entry = entry->vme_next;
				else if (end <= entry->vme_start)
					return KERN_SUCCESS;
				else {
					start = entry->vme_start;
					break;
				}
			}
			if (start < end && entry != vm_map_to_entry(map)) {
				first_entry = entry;
				ret = KERN_SUCCESS;
			}
			else return KERN_SUCCESS;
		}
		else return KERN_INVALID_ADDRESS;
	}
	else entry = first_entry;

	/*
	 * Check to see if any of this virtual
	 * space is wired by the kernel.
	 */

	if (vl_kwire(entry, start, end)
	    || lw_is_wired(entry, start, end)
	    || pmap_coproc_page_is_busy(map->vm_pmap, start, end))
	          return KERN_PAGELOCKED;


	/*
	 *	Step through all entries in this region
	 */


	while (start < end) {
		if (entry == vm_map_to_entry(map)) {
			if (contain == TRUE) ret = KERN_INVALID_ADDRESS;
			break;
		}
		else if (start < entry->vme_start) {
			if (contain == TRUE) {
				ret = KERN_INVALID_ADDRESS;
				break;
			}
			else {
				start = entry->vme_start;
				continue;
			}
		}
		else if (entry == first_entry) {
			vm_mape_faultlock(entry);
			size = MIN(end, entry->vme_end) - start;
			if ((ret = (*entry->vme_unmap)(entry, start, size)) !=
				KERN_SUCCESS && ret != KERN_SEGHOLE) break;
			SAVE_HINT(map, entry->vme_prev);
			if (map->vm_first_free->vme_start >= start)
				map->vm_first_free = entry->vme_prev;
		}
		else {
			vm_mape_faultlock(entry);
			size = MIN(end, entry->vme_end) - start;
			if ((ret = (*entry->vme_unmap)(entry, start, size)) !=
				KERN_SUCCESS && ret != KERN_SEGHOLE) break;
		}
		next = entry->vme_next;
		if (ret != KERN_SEGHOLE) {
			u_map_free_vas(map, size);
			vm_map_entry_delete(map, entry);
		}
		else ret = KERN_SUCCESS;
		entry = next;
		if (contain == FALSE) start = entry->vme_start;
		else start += size;
	}

	if (map->vm_wait_for_space)
		thread_wakeup((vm_offset_t)map);

	return ret;
}

/*
 *
 *	Sets the protection of the specified address
 *	region in the target map.  If "set_max" is
 *	specified, the maximum protection is to be set;
 *	otherwise, only the current protection is affected.
 */

kern_return_t
u_map_protect(register vm_map_t map,
	register vm_offset_t start,
	register vm_offset_t end,
	register vm_prot_t new_prot,
	register boolean_t set_max)
{
	register vm_map_entry_t	current;
	register vm_size_t size;
	vm_map_entry_t entry;
	kern_return_t ret;

	vm_map_lock(map);

	if (!vm_map_lookup_entry(map, start, &entry)) {
		ret = KERN_INVALID_ADDRESS;
		goto bad;
	}
	else current = entry;

	if (set_max == TRUE) {
		vm_map_unlock(map);
		return KERN_PROTECTION_FAILURE;
	}

	/*
	 *	Go back and fix up protections.
	 */


	while (start < end) {
		if (current == vm_map_to_entry(map) ||
			start < current->vme_start) {
			ret = KERN_INVALID_ADDRESS;
			goto bad;
		}
		if ((current->vme_maxprot & new_prot) != new_prot) {
			ret = KERN_PROTECTION_FAILURE;
			goto bad;
		}
		vm_mape_faultlock(current);
		size = MIN(current->vme_end, end) - start;
		if ((ret = (*current->vme_protect)(current, start, size,
			 new_prot)) != KERN_SUCCESS)
				goto bad;
		current = current->vme_next;
		start += size;
	}

	vm_map_unlock(map);
	return KERN_SUCCESS;
bad:
	vm_map_unlock(map);
	return ret;
}

kern_return_t
u_map_inherit(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	vm_inherit_t new_inheritance)
{
	return u_map_control(map, start, end,
		VMC_INHERITANCE, new_inheritance);
}

kern_return_t
u_map_keep_on_exec(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	boolean_t new_koe)
{
	return u_map_control(map, start, end,
		VMC_KEEP_ON_EXEC, new_koe);
}

static kern_return_t
u_map_msmem(vm_map_t map,
	register vm_offset_t start,
	vm_control_t control)
{
	vm_map_entry_t tmp_entry;
	register vm_map_entry_t entry;
	kern_return_t ret;

	vm_map_lock_read(map);
	if (!vm_map_lookup_entry(map, trunc_page(start), &tmp_entry)) {
		vm_map_unlock_read(map);
		return KERN_INVALID_ADDRESS;
	}
	else entry = tmp_entry;

	vm_mape_fault(entry);
	vm_map_unlock_read(map);

	/*
	 * The fault done is made the the vme_control operation.
	 */

	ret = (*entry->vme_control)(entry, start,
		(vm_size_t) 0, control, (int) 0);
	return ret;
}

/*
 * Perform simple map entry control operations
 */

kern_return_t
u_map_control(vm_map_t map,
	register vm_offset_t start,
	register vm_offset_t end,
	vm_control_t control,
	int data)
{
	kern_return_t ret;
	register vm_size_t size;
	vm_map_entry_t entry;
	register vm_map_entry_t current;

	switch (control) {
	case VMC_SEM_SLEEP:
	case VMC_SEM_WAKEUP:
		return u_map_msmem(map, start, control);
	default:
		break;
	}

	start = trunc_page(start);
	end = round_page(end);
	size = end - start;

	assert((start < end));

	vm_map_lock(map);


	if (!vm_map_lookup_entry(map, start, &entry)) {
		ret = KERN_INVALID_ADDRESS;
		goto bad;
	}

	current = entry;
	while (start < end) {
		if (current == vm_map_to_entry(map) ||
				start < current->vme_start) {
			ret = KERN_INVALID_ADDRESS;
			goto bad;
		}
		vm_mape_faultlock(current);
		size = MIN(end, current->vme_end) - start;
		if (control == VMC_MSYNC) {
			if ((ret =
			(*current->vme_msync)(current, start, size, data))
			!= KERN_SUCCESS) goto bad;
		}
		else if ((ret = (*current->vme_control)(current, start, size,
			control, data)) != KERN_SUCCESS) goto bad;
		start += size;
		current = current->vme_next;
	}

	vm_map_unlock(map);
	return KERN_SUCCESS;
bad:
	vm_map_unlock(map);
	return ret;
}

/*
 *	vm_map_exec:
 *
 * 	This is only used by the UNIX exec code.  Why is it a general
 *	map operations exported to the world ?  Or is it that anything
 *	defined in vm_unix.c isn't exported to the world ?
 *	Deallocates all currently-mapped portions of the given
 *	address range that are not marked keep-on-exec from the
 *	target map.
 *
 */

kern_return_t
u_map_exec(map, start, end)
	register vm_map_t	map;
	vm_offset_t		start;
	register vm_offset_t	end;
{
	register vm_map_entry_t	entry, next;

	vm_map_lock(map);

	if (start != vm_map_min(map) || end != vm_map_max(map))
		panic("u_map_exec: address range is incorrect");



	SAVE_HINT(map, vm_map_to_entry(map));
	map->vm_first_free = vm_map_to_entry(map);

	/*
	 *	Step through all entries in this region
	 */

	entry = vm_map_first_entry(map);

	while (entry != vm_map_to_entry(map)){

		next = entry->vme_next;
		if (entry->vme_keep_on_exec) {
			entry = next;
			continue;
		}

		if (u_map_delete(map, entry->vme_start, entry->vme_end, TRUE) !=
			KERN_SUCCESS) panic("u_map_exec: deletion failed");

		entry = next;
	}


	vm_map_unlock(map);
	return KERN_SUCCESS;
}

boolean_t
u_map_check_protection(
	vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	vm_prot_t protection)
{
	register vm_size_t size;
	register vm_map_entry_t	entry;
	vm_map_entry_t tmp_entry;

	vm_map_lock(map);

	if (!vm_map_lookup_entry(map, start, &tmp_entry)) goto bad;

	entry = tmp_entry;

	while (start < end) {
		if (entry == vm_map_to_entry(map) || start < entry->vme_start)
			goto bad;
		vm_mape_faultlock(entry);
		size = MIN(entry->vme_end, end) - start;
		if ((*entry->vme_check_protect)(entry, start, size, protection)
			!= TRUE) goto bad;
		start += size;
		entry = entry->vme_next;
	}
	vm_map_unlock(map);
	return TRUE;
bad:
	vm_map_unlock(map);
	return FALSE;
}

/*
 * These routines support the kernel C submap
 *  and the U maps.
 */

u_map_copy_overwrite(vm_map_t dst_map,
	vm_offset_t dst_addr,
	vm_map_copy_t copy,
	boolean_t interruptible,
	register vm_size_t size)
{
	register vm_offset_t start, end, cstart;
	register vm_map_entry_t ep, cp;
	register vm_size_t length;
	vm_map_entry_t tmp_entry;
	vm_page_t spp, dpp;
	kern_return_t ret;

	/*
	 * Lock the destination map and verify that the entire
	 * space is writable and without holes.
	 */

	vm_map_lock(dst_map);

	if (!vm_map_lookup_entry(dst_map, dst_addr, &tmp_entry)) {
		vm_map_unlock(dst_map);
		return KERN_INVALID_ADDRESS;
	}

	ep = tmp_entry;
	start = dst_addr;
	end = start + size;
	start = dst_addr;
	while (start < end) {
		length = MIN(ep->vme_end, end) - start;
		if (ep == vm_map_to_entry(dst_map) ||
			start < ep->vme_start ||
			(ret = (*ep->vme_check_protect)(ep, start,
				length, VM_PROT_WRITE)) != TRUE) {

			vm_map_unlock(dst_map);
			return KERN_INVALID_ADDRESS;
		}
		ep = ep->vme_next;
		start += length;
	}

	/*
	 * Copy each page from the source to the destination.
	 * Do a write faullt on the destination to verify we can really write.
	 * Do a read fault on the source copy entry.
	 */

	ep = tmp_entry;
	vm_mape_faultlock(ep);
	cp = vm_map_copy_first_entry(copy);
	start = dst_addr;
	cstart = cp->vme_start;

	while (1) {
		if ((ret = (*ep->vme_fault)(ep, start, PAGE_SIZE,
			VM_PROT_WRITE, VM_PAGEGET, &dpp)) != KERN_SUCCESS)
				goto failed;
		else if ((ret = (*cp->vme_fault)(cp, cstart, PAGE_SIZE,
			VM_PROT_READ, VM_PAGEGET, &spp)) != KERN_SUCCESS) {
			OOP_PAGECTL(dpp->pg_object, &dpp, 1, VMOP_RELEASE);
			goto failed;
		}
		pmap_protect(dst_map->vm_pmap, start,
			start + PAGE_SIZE, VM_PROT_NONE);
		vm_page_copy(spp, dpp);
		OOP_PAGECTL(dpp->pg_object, &dpp, 1, VMOP_RELEASE);
		OOP_PAGECTL(spp->pg_object, &spp, 1, VMOP_RELEASE);

		start += PAGE_SIZE;
		if (start < end) {
			cstart += PAGE_SIZE;
			if (cstart >= cp->vme_end) cp = cp->vme_next;

			if (start >= ep->vme_end) {
				ep = ep->vme_next;
				vm_mape_faultlock(ep);
			}
		}
		else break;
	}

	vm_map_unlock(dst_map);
	return KERN_SUCCESS;

failed:
	vm_map_unlock(dst_map);
	return ret;
}

u_map_copyout(register vm_map_t dst_map,
	register vm_offset_t *dst_addr,
	vm_map_copy_t copy)
{
	kern_return_t ret;
	register vm_offset_t start;
	register vm_size_t size, esize;
	register struct u_map_private *up;
	register vm_map_entry_t ep, nep;
	vm_map_entry_t last;
	register vm_offset_t addr;
	vm_offset_t first;
	vm_fault_t wire;
	vm_size_t adjustment;

	up = (struct u_map_private *) (dst_map->vm_private);
	size =	round_page(copy->vm_max_offset) -
		trunc_page(copy->vm_min_offset);

again:

	/*
	 * Just looking for new contigous address space in
	 * a map doesn't require taking the fault lock.
	 */

	vm_map_lock(dst_map);

	/*
	 * We have to check three things each pass.
	 * Has the target map exceeded its VAS limit ?
	 * Does the target map automagically have its space wired ?
	 * Can the map be extended by the number of entries in the copy list ?
	 */


	wire = up->um_lock_future ? VM_WIRE : VM_NOWIRE;

	if ((size + dst_map->vm_size) > up->um_maxvas) {
		vm_map_unlock(dst_map);
		return KERN_NO_SPACE;
	}
	if (vm_tune_compare((copy->vm_nentries + dst_map->vm_nentries),
							mapentries, >)) {
		vm_map_unlock(dst_map);
		return KERN_MAPENTRIES_LIMIT;
	}

	start = ((last = dst_map->vm_first_free) == vm_map_to_entry(dst_map)) ?
		vm_map_min(dst_map) : last->vme_end;

	while (1) {
		register vm_map_entry_t next = last->vme_next;
		register vm_offset_t end = start + size;

		if ((end > dst_map->vm_max_offset) || (end < start)) {
			if (dst_map->vm_wait_for_space) {
				if (size <= (dst_map->vm_max_offset -
						dst_map->vm_min_offset)) {
					assert_wait((vm_offset_t)dst_map, TRUE);
					vm_map_unlock(dst_map);
					thread_block();
					goto again;
				}
			}
			vm_map_unlock(dst_map);
			return KERN_NO_SPACE;
		}

		if ((next == vm_map_to_entry(dst_map)) ||
			(end < next->vme_start)) break;

		last = next;
		start = last->vme_end;
	}

	/*
	 * We have found some space.
	 */

	adjustment = start - trunc_page(copy->vm_min_offset);
	for (ep = vm_map_copy_first_entry(copy);
	     ep != vm_map_copy_to_entry(copy);
	     ep = ep->vme_next) {

		ep->vme_map = dst_map;
		ep->vme_start += adjustment;
		ep->vme_end += adjustment;
		ep->vme_keep_on_exec = FALSE;
		ep->vme_inheritance = VM_INHERIT_COPY;
	}

	/*
	 *	Correct the page alignment for the result
	 */

	*dst_addr = start +
		(copy->vm_min_offset - trunc_page(copy->vm_min_offset));

	/*
	 *	Update the hints
	 */

	if (dst_map->vm_first_free == last)
		dst_map->vm_first_free = vm_map_copy_last_entry(copy);
	SAVE_HINT(dst_map, vm_map_copy_last_entry(copy));


	/*
	 * Call the map entry copyout handle.
	 * Assuming it succeeds then account for map resources
	 * and remove the entry from the copy list and insert
	 * it into the map.
	 */


	for (ep = vm_map_copy_first_entry(copy), first = addr = ep->vme_start;
		ep != vm_map_copy_to_entry(copy); addr += esize, ep = nep) {

		nep = ep->vme_next;
		esize  = ep->vme_end - ep->vme_start;
		if ((ret = (*ep->vme_copy)(ep, VME_COPYLOAD)) != KERN_SUCCESS)
			goto failed;
		else {
			dst_map->vm_size += esize;
			vm_map_entry_unlink((vm_map_t) copy, ep);
			vm_map_entry_link(dst_map, last, ep);

			/*
			 * The entry is now in the map.  If were wiring
			 * attempt to wire down the memory.
			 */

			if (wire == VM_WIRE) {
				if ((ret = (*ep->vme_lockop)(ep, addr,
					esize, VM_WIRE)) != KERN_SUCCESS) {
					addr += esize;
					goto failed;
				}
			}
			last = ep;
		}
	}

	vm_map_copy_discard(copy);
	vm_map_unlock(dst_map);
	return KERN_SUCCESS;

failed:

	/*
	 *  The hint is invalid
	 */

	SAVE_HINT(dst_map, vm_map_to_entry(dst_map));

	/*
	 * Nothing was inserted
	 */

	if (first == addr) dst_map->vm_first_free = last;

	/*
	 * Use the normal deletion code.
	 */

	else u_map_delete(dst_map, first, addr, TRUE);
	vm_map_unlock(dst_map);
	return ret;
}

extern zone_t vm_map_copy_zone;

/*
 * Copyin part of a map range
 */

u_map_copyin(vm_map_t map,
	vm_offset_t src_addr,
	vm_size_t len,
	boolean_t destroy,
	vm_map_copy_t *copy_result)
{
	register vm_offset_t start, end, first;
	kern_return_t ret;
	vm_map_entry_t tmp_entry;
	vm_map_entry_t entry, copy_entry, next_entry;
	register vm_size_t size;
	register vm_map_copy_t copy;

	/*
	 *	Check for copies of zero bytes.
	 */

	if (len == 0) {
		if (src_addr != 0)
			return KERN_INVALID_ADDRESS;

		*copy_result = VM_MAP_COPY_NULL;
		return KERN_SUCCESS;
	}

	/*
	 *	Compute start and end of region
	 */

	start = trunc_page(src_addr);
	end = round_page(src_addr + len);

	if (end <= start)
		if ((end < start) || (start != 0))
			return KERN_INVALID_ADDRESS;

	copy = (vm_map_copy_t) zalloc(vm_map_copy_zone);
	bzero(copy, sizeof *copy);

	vm_map_copy_first_entry(copy) =
		vm_map_copy_last_entry(copy) = vm_map_copy_to_entry(copy);

	copy->vm_nentries = 0;
	copy->vm_entries_pageable = TRUE;

	copy->vm_min_offset = src_addr;
	copy->vm_max_offset = src_addr + len;


	/*
	 *	Find the beginning of the region.
	 */

 	vm_map_lock(map);

	if (!vm_map_lookup_entry(map, start, &tmp_entry)) {
		ret = KERN_INVALID_ADDRESS;
		goto failed;
	}

	entry = tmp_entry;
	first = start;

	while (start < end) {
		if (start < entry->vme_start ||
			entry == vm_map_to_entry(map) ) goto failed;
		vm_mape_faultlock(entry);
		size = MIN(entry->vme_end, end) - start;
		vm_map_entry_create(copy, &copy_entry);
		next_entry = entry->vme_next;
		copy_entry->vme_start = start;
		copy_entry->vme_end = start + size;
		copy_entry->vme_maxprot = copy_entry->vme_protection =
			VM_PROT_READ|VM_PROT_WRITE;
		if ((ret = (*entry->vme_dup)(entry, start, size,
				copy_entry, VM_COPYMCOPY))
				!= KERN_SUCCESS) {
			vm_map_entry_dispose((vm_map_t) copy, copy_entry);
			goto failed;
		}
		vm_map_entry_link(copy, vm_map_copy_last_entry(copy),
				copy_entry);
		copy_entry->vme_maxprot = copy_entry->vme_protection =
			entry->vme_maxprot;
		start += size;
		entry = next_entry;
	}
	*copy_result = copy;
	if (destroy) (*map->vm_delete_map)(map, first, end, TRUE);
	vm_map_unlock(map);
	return KERN_SUCCESS;

failed:
	vm_map_unlock(map);
	vm_map_copy_discard(copy);
	*copy_result = VM_MAP_COPY_NULL;
	return ret;
}

/*
 * Currently we ignore the Mach inheritance model.
 */

kern_return_t
u_map_fork(register vm_map_t oldmap, register vm_map_t newmap)
{
	vm_map_entry_t oep, nep, ep;
	kern_return_t ret;


	for (nep = vm_map_to_entry(newmap), oep = vm_map_first_entry(oldmap);
		oep != vm_map_to_entry(oldmap); oep = oep->vme_next) {

		switch (oep->vme_inheritance) {
		case VM_INHERIT_NONE: break;
		case VM_INHERIT_COPY:
		case VM_INHERIT_SHARE:
			vm_mape_faultlock(oep);
			vm_map_entry_create(newmap, &ep);
			*ep = *oep;
			ep->vme_map = newmap;
			ret = u_map_vpage_dup(oep, ep);
			if (ret != KERN_SUCCESS) {
				vm_map_entry_dispose(newmap, ep);
				return ret;
			}
			vm_map_entry_link(newmap, nep, ep);
			if ((ret = (*oep->vme_dup)(oep, oep->vme_start,
				oep->vme_end - oep->vme_start, ep,
				VM_COPYU)) != KERN_SUCCESS) return ret;
			newmap->vm_size += (ep->vme_end - ep->vme_start);
			nep = ep;
		}
	}
	return KERN_SUCCESS;
}

u_map_entry_split(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t len)
{
	register vm_offset_t start, end, offset;
	register struct vpage *vp;


	if (ep->vme_private) {
		start = ep->vme_start;
		end = ep->vme_end;
		offset = atop(addr - start);
		vp = (struct vpage *) (ep->vme_private);
	}

	/*
	 * Clip the start and end
	 */

	vm_map_entry_split(ep->vme_map, ep, addr, addr + len);

	if (ep->vme_private) {
		if (start == addr) {
			if (end != (addr + len))
				ep->vme_next->vme_private =
					(vm_offset_t) (vp + atop(len));
		}
		else if (end == (addr + len))
			ep->vme_private = (vm_offset_t) (vp + offset);
		else {
			ep->vme_next->vme_private = (vm_offset_t)
					(vp + offset + atop(len));
			ep->vme_private = (vm_offset_t) (vp + offset);
		}
	}
	return;
}

u_map_entry_unmap(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register vm_size_t len)
{
	if (ep->vme_private) {
		register struct vpage *vp;
		register vm_offset_t start, end, offset;

		start = ep->vme_start;
		end = ep->vme_end;
		offset = atop(addr - start);
		vp = (struct vpage *) (ep->vme_private);
		vm_map_entry_split(ep->vme_map, ep, addr, addr + len);

		u_vpage_free(ep->vme_map, vp + offset, atop(len), TRUE);
		if (start == addr) {
			if (end != (addr + len))
				ep->vme_next->vme_private =
					(vm_offset_t) (vp + atop(len));
		}
		else if (end != (addr + len)) ep->vme_next->vme_private =
			(vm_offset_t) (vp + offset + atop(len));
	}
	else vm_map_entry_split(ep->vme_map, ep, addr, addr + len);
}

u_map_vpage_dup(register vm_map_entry_t ep,
	register vm_map_entry_t newep)
{
	register int npages;
	register struct vpage *vp, *vp2;
	kern_return_t ret;

	/*
	 * See if there is a vpage array
	 */


	if (ep->vme_private) {
		npages = atop(ep->vme_end - ep->vme_start);

		if ((ret = u_vpage_alloc(newep->vme_map, npages,
				&newep->vme_private, TRUE)) != KERN_SUCCESS)
			return ret;
		vp2 = (struct vpage *) (newep->vme_private);
		vp = (struct vpage *) (ep->vme_private);
	}
	else {
		npages = 1;
		vp2 = &newep->vme_vpage;
		vp = &ep->vme_vpage;
	}

	while (npages--) {
		*vp2 = *vp++;
		vp2->vp_plock = 0;
		vp2++;
	}

	return KERN_SUCCESS;
}

kern_return_t
u_vpage_init(register vm_map_entry_t ep,
	boolean_t wait)
{
	register int npages;
	register struct vpage *vp;
	kern_return_t ret;

	if (ep->vme_private) return KERN_SUCCESS;
	npages = atop(ep->vme_end - ep->vme_start);
	if ((ret = u_vpage_alloc(ep->vme_map, npages, &ep->vme_private,
		wait)) != KERN_SUCCESS) return ret;
	else vp = (struct vpage *) (ep->vme_private);

	while (npages--) {
		vp->vp_plock = ep->vme_plock;
		(vp)++->vp_prot = ep->vme_protection;

	}
	return KERN_SUCCESS;
}

/*
 * Check whether a unmap can execute
 * the delete and split operation.
 */

boolean_t
u_map_entry_delete_check(vm_map_t map,
	vm_map_entry_t ep,
	vm_offset_t start,
	vm_offset_t end)
{
	/*
	 * We'll need at least one for two created
	 * and one deleted or none for one created
	 * and one deleted.
	 */

	if ((start != ep->vme_start || end != ep->vme_end) &&
		vm_tune_compare((map->vm_nentries), mapentries, ==))
			return FALSE;
	else return TRUE;

}

/*
 * u_map_entry_clip_check
 */

boolean_t
u_map_entry_clip_check(register vm_map_t map,
	register vm_map_entry_t ep,
	register vm_offset_t start,
	register vm_offset_t end)
{
	register int need;

	need = 0;

	if (start != ep->vme_start) need++;
	if (end != ep->vme_end) need++;
	return (vm_tune_compare((map->vm_nentries + need), mapentries, >) ?
			FALSE : TRUE);
}

