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
static char	*sccsid = "@(#)$RCSfile: vm_map.c,v $ $Revision: 4.4.20.6 $ (DEC) $Date: 1993/11/23 16:32:55 $";
#endif 
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
 *	File:	vm/vm_map.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory mapping module.
 */

#include <mach_ldebug.h>
#include <kern/assert.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <kern/zalloc.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <mach/vm_prot.h>
#include <vm/u_mape_seg.h>
#include <mach/port.h>
#include <mach/vm_attributes.h>
#include <vm/vm_fault.h>
#include <vm/vm_tune.h>
#include <sys/table.h>

/*
 *	Virtual memory maps provide for the mapping, protection,
 *	and sharing of virtual memory objects.  In addition,
 *	this module provides for an efficient virtual copy of
 *	memory from one map to another.
 *
 *	Synchronization is required prior to most operations.
 *
 *	Maps consist of an ordered doubly-linked list of simple
 *	entries; a single hint is used to speed up lookups.
 *
 *
 *
 */


zone_t		vm_map_zone;		/* zone for vm_map structures */
zone_t		vm_map_entry_zone;	/* zone for vm_map_entry structures */
zone_t		vm_map_kentry_zone;	/* zone for kernel entry structures */
zone_t		vm_map_copy_zone;	/* zone for vm_map_copy structures */

boolean_t	vm_map_lookup_entry();	/* forward declaration */

/*
 *	Placeholder object for submap operations.
 */

vm_object_t	vm_submap_object;

/*
 *	vm_map_init:
 *
 *	Initialize the vm_map module.  Must be called before
 *	any other vm_map routines.
 *
 *	Map and entry structures are allocated from zones -- we must
 *	initialize those zones.
 *
 *	There are three zones of interest:
 *
 *	vm_map_zone:		used to allocate maps.
 *	vm_map_entry_zone:	used to allocate map entries.
 *	vm_map_kentry_zone:	used to allocate map entries for the kernel.
 *
 *	The kernel allocates map entries from a special zone that is initially
 *	"crammed" with memory.  It would be difficult (perhaps impossible) for
 *	the kernel to allocate more memory to a entry zone when it became
 *	empty since the very act of allocating memory implies the creatio
 *	of a new entry.  Further, since the kernel map is created from the
 *	map zone, the map zone is initially "crammed" with enough memory
 *	to fullfill that need.
 */

void
vm_map_init()
{
	extern vm_offset_t map_data, kentry_data;
	extern vm_size_t map_data_size, kentry_data_size;
	extern int nproc;

	vm_map_zone = zinit((vm_size_t) sizeof(struct vm_map),
				sizeof(struct vm_map) * nproc * 4,
				PAGE_SIZE, "maps");
	vm_map_entry_zone = zinit((vm_size_t) sizeof(struct vm_map_entry),
				1024*1024, PAGE_SIZE*5,
				"non-kernel map entries");
	vm_map_kentry_zone = zinit((vm_size_t) sizeof(struct vm_map_entry),
				kentry_data_size, kentry_data_size,
				"kernel map entries");
	vm_map_copy_zone = zinit((vm_size_t) sizeof(struct vm_map_copy),
				16*1024, PAGE_SIZE,
				"map copies");

	u_map_init();
	vl_init();

	/*
	 * Cram the map and kentry zones with initial data.
	 * Mark the kentry zone exhaustible and not to be garbage collected.
	 */

	zchange(vm_map_kentry_zone, FALSE, FALSE, TRUE, FALSE);
	zcram(vm_map_zone, map_data, map_data_size);
	zcram(vm_map_kentry_zone, kentry_data, kentry_data_size);
}

/*
 *	vm_map_create:
 *
 *	Creates and returns a new empty VM map with
 *	the given physical map structure, and having
 *	the given lower and upper address bounds.
 */

vm_map_t
vm_map_create(pmap_t pmap, vm_offset_t min, vm_offset_t max, boolean_t pageable)
{
	register vm_map_t result;

	ZALLOC(vm_map_zone, result, vm_map_t);
	if (result == VM_MAP_NULL)
		panic("vm_map_create: out of maps");

	result->vm_next = result->vm_prev = vm_map_to_entry(result);
	result->vm_nentries = 0;
	result->vm_size = 0;
	result->vm_ref_count = 1;
	result->vm_res_count = 1;
	result->vm_pmap = pmap;
	result->vm_min_offset = min;
	result->vm_max_offset = max;
	result->vm_entries_pageable = pageable;
	result->vm_is_mainmap = 1;
	result->vm_wait_for_space = 0;
	result->vm_first_free = vm_map_to_entry(result);
	result->vm_hint = vm_map_to_entry(result);
	result->vm_fault_rate = 0;
	result->vm_pagefaults = 0;
	result->vm_faultrate_time = sched_tick;
	result->vm_color_bucket = vm_initial_color_bucket(result);
	vm_map_lock_init(result);
	simple_lock_init(&result->vm_ref_lock);
	simple_lock_init(&result->vm_hint_lock);
	if (min >= VM_MIN_KERNEL_ADDRESS && max <= VM_MAX_KERNEL_ADDRESS) {
		k_map_create(result);
		return result;
	}
	else if (max <= VM_MAX_ADDRESS) {
		if (u_map_create(result) != KERN_SUCCESS) {
			ZFREE(vm_map_zone, result);
			return (vm_map_t) 0;
		}
		else return result;
	}
	else panic("vm_map_create: illegal address range");
}

/*
 *	vm_map_entry_create
 */

extern vm_map_t	kentry_map;
extern int	kentry_count;	/* initial data size for vm_map_kentry_zone */

kern_return_t
vm_map_entry_create(register vm_map_t map, vm_map_entry_t *entryp)
{
	vm_map_entry_t entry;
	register zone_t	zone;
        register vm_offset_t    addr;
        int  mem_left, save_priv;

	if (!map->vm_entries_pageable) zone = vm_map_kentry_zone;
	else if (map->vm_umap &&
		vm_tune_compare(map->vm_nentries, mapentries, ==)) {
		*entryp = 0;
		return(KERN_MAPENTRIES_LIMIT); 
	}
	else zone = vm_map_entry_zone;

        if ((zone == vm_map_kentry_zone) && (current_thread() != 0)) {
	    mem_left = zone->cur_size - zone->count * zone->elem_size;
	    if (mem_left < (kentry_count/2) * zone->elem_size ) {

                if (lock_try_write(&kentry_map->vm_lock)) {
                    lock_set_recursive(&kentry_map->vm_lock);
		}
		else {
		    if((current_thread()->vm_privilege)) {
			    goto use_reserve;
	 	    }
		    else {
                	lock_write(&kentry_map->vm_lock);
	    		mem_left = zone->cur_size - zone->count * zone->elem_size;
	    		if (mem_left < (kentry_count/2) * zone->elem_size )
                    	    lock_set_recursive(&kentry_map->vm_lock);
			else {
                	    lock_done(&kentry_map->vm_lock);
			    goto use_reserve;
			}
		    }
		}
                if (!zone->doing_alloc) {
                    zone->doing_alloc = 1;
		    save_priv = current_thread()->vm_privilege;
		    current_thread()->vm_privilege = TRUE;
		    addr = kmem_alloc(kentry_map, zone->alloc_size);
		    if (addr == NULL) {
			panic("vm_map_entry_create:can't expand kentry_zone");
		    }
	            zcram(zone, addr, zone->alloc_size);
		    if(current_thread()->vm_privilege != save_priv)
		        current_thread()->vm_privilege = save_priv;
		    zone->doing_alloc = 0;
		}
                lock_clear_recursive(&kentry_map->vm_lock);
                lock_done(&kentry_map->vm_lock);
	    }
	}

use_reserve:

	ZALLOC(zone, entry, vm_map_entry_t);
	if (entry == VM_MAP_ENTRY_NULL)
		panic("vm_map_entry_create: out of map entries");
	else {
		bzero(entry, sizeof (*entry));
		simple_lock_init(&entry->vme_faultlock);
	}
	*entryp = entry;
	return(KERN_SUCCESS);
}

/*
 * Splits a map entry into two or three pieces.
 * No quota checking of entries has been done.
 * It is assumed to have been done previously.
 */

vm_map_entry_split(vm_map_t map,
	register vm_map_entry_t entry,
	register vm_offset_t start,
	vm_offset_t end)
{
	register vm_map_entry_t	new_entry;

	if (start > entry->vme_start) {
		ZALLOC(vm_map_entry_zone, new_entry, vm_map_entry_t);
		*new_entry = *entry;
		new_entry->vme_end = start;
		entry->vme_offset += (start - entry->vme_start);
		entry->vme_start = start;
		vm_object_reference(new_entry->vme_object);
		vm_map_entry_link(map, entry->vme_prev, new_entry);
	}

	if (end < entry->vme_end) {
		ZALLOC(vm_map_entry_zone, new_entry, vm_map_entry_t);
		*new_entry = *entry;
		new_entry->vme_start = entry->vme_end = end;
		new_entry->vme_offset += (end - entry->vme_start);
		vm_object_reference(new_entry->vme_object);
		vm_map_entry_link(map, entry, new_entry);
	}
}

/*
 *	vm_map_entry_dispose
 */


void
vm_map_entry_dispose(register vm_map_t map, vm_map_entry_t entry)
{

	ZFREE((map->vm_entries_pageable ?
		vm_map_entry_zone : vm_map_kentry_zone), entry);
}


/*
 *	vm_map_reference:
 *
 *	Creates another valid reference to the given map.
 *
 */

void
vm_map_reference(register vm_map_t map)
{
	if (map == VM_MAP_NULL)
		return;

	simple_lock(&map->vm_ref_lock);
	map->vm_ref_count++;
	map->vm_res_count++;
	simple_unlock(&map->vm_ref_lock);
}

/*
 *	vm_map_deallocate:
 *
 *	Removes a reference from the specified map,
 *	destroying it if no references remain.
 *	The map should not be locked.
 */

void
vm_map_deallocate(register vm_map_t map)
{
	register int c;
	register vm_map_entry_t entry, next;

	if (map == VM_MAP_NULL)
		return;

	simple_lock(&map->vm_ref_lock);
	c = --map->vm_ref_count;
	map->vm_res_count--;
	simple_unlock(&map->vm_ref_lock);

	if (c > 0) return;

	/*
	 *	Lock the map, to wait out all other references
	 *	to it.
	 */

	vm_map_lock(map);

	(*map->vm_deallocate_map)(map);

	pmap_destroy(map->vm_pmap);
#if	MACH_LDEBUG
	vm_map_unlock(map);
#endif

	ZFREE(vm_map_zone, (vm_offset_t) map);
}

/*
 *	vm_map_swapout:
 *	decrement the vm_map->res_count and if it goes to zero
 *	call vm_object_swapout for each object pointed to by each
 *	vm_map_entry.
 */

kern_return_t
vm_map_swapout(vm_map_t map,
	int *pagecount)
{
	vm_map_entry_t	entry;
	vm_object_t	object;
	kern_return_t	error;
	int		pages;
	register	pagetotal;

	pagetotal = 0;
	error = KERN_SUCCESS;
	if (--map->vm_res_count <= 0) {
		entry = vm_map_first_entry(map);
		while (entry != vm_map_to_entry(map)) {
			if (entry->vme_object) {
				pages = 0;
				error = vm_object_swapout(entry->vme_object,
					&pages);
				pagetotal += pages;
				if (error != KERN_SUCCESS) break;
			}
			entry = entry->vme_next;
		}
		if (error != KERN_SUCCESS) {
			register vm_map_entry_t last;

			for (last = entry, entry = vm_map_first_entry(map);
				entry != last; entry = entry->vme_next)
				if (entry->vme_object)
					vm_object_swapin(entry->vme_object);
			map->vm_res_count++;
			pagetotal = 0;
		}
	}
	*pagecount = pagetotal;
	return error;
}

/*
 *	vm_map_swapin:
 *
 *	Increment the vm_map->vm_res_count and call vm_object_swapin for
 *	each vm_object pointed to by each vm_map_entry.
 *
 */

vm_map_swapin(vm_map_t map)
{
	vm_map_entry_t	entry;
	vm_object_t	object;

	map->vm_res_count++;
	entry = vm_map_first_entry(map);
	while (entry != vm_map_to_entry(map)) {
		vm_object_swapin(entry->vme_object);
		entry = entry->vme_next;
	}
	
}

/*
 *	vm_map_lookup_entry:	[ internal use only ]
 *
 *	Finds the map entry containing (or
 *	immediately preceding) the specified address
 *	in the given map; the entry is returned
 *	in the "entry" parameter.  The boolean
 *	result indicates whether the address is
 *	actually contained in the map.
 *
 */

boolean_t
vm_map_lookup_entry(
	register vm_map_t map,
	register vm_offset_t address,
	vm_map_entry_t *entry				/* OUT */
	)
{
	register vm_map_entry_t		cur;
	register vm_map_entry_t		last;

	/*
	 *	Start looking either from the head of the
	 *	list, or from the hint.
	 */

	simple_lock(&map->vm_hint_lock);
	cur = map->vm_hint;
	simple_unlock(&map->vm_hint_lock);

	if (cur == vm_map_to_entry(map))
		cur = cur->vme_next;

	if (address >= cur->vme_start) {
	    	/*
		 *	Go from hint to end of list.
		 *
		 *	But first, make a quick check to see if
		 *	we are already looking at the entry we
		 *	want (which is usually the case).
		 *	Note also that we don't need to save the hint
		 *	here... it is the same hint (unless we are
		 *	at the header, in which case the hint didn't
		 *	buy us anything anyway).
		 */
		last = vm_map_to_entry(map);
		if ((cur != last) && (cur->vme_end > address)) {
			*entry = cur;
			return TRUE;
		}
	}
	else {
	    	/*
		 *	Go from start to hint, *inclusively*
		 */
		last = cur->vme_next;
		cur = vm_map_first_entry(map);
	}

	/*
	 *	Search linearly
	 */

	while (cur != last) {
		if (cur->vme_end > address) {
			if (address >= cur->vme_start) {
			    	/*
				 *	Save this lookup for future
				 *	hints, and return
				 */

				*entry = cur;
				SAVE_HINT(map, cur);
				return TRUE;
			}
			break;
		}
		cur = cur->vme_next;
	}
	*entry = cur->vme_prev;
	SAVE_HINT(map, *entry);
	return FALSE;
}

kern_return_t
vm_map_space(
	register vm_map_t map,
	vm_offset_t *address,
	vm_size_t size,
	vm_offset_t mask,
	boolean_t anywhere,
	vm_map_entry_t *prev_entry)
{
	register vm_map_entry_t	entry;
	register vm_offset_t start;
	register vm_offset_t end;
	kern_return_t result = KERN_SUCCESS;

#define RETURN(value)	{ result = value; goto failed; }

again: ;

	start = *address;

	if (anywhere) {
		vm_offset_t	hint = 0;
		boolean_t	first_time;

		vm_map_lock(map);

		/*
		 *	Calculate the first possible address.
		 */

		if (start < map->vm_min_offset)
			start = map->vm_min_offset;
		if (start > map->vm_max_offset)
			RETURN(KERN_NO_SPACE);

		/*
		 *	Look for the first possible address;
		 *	if there's already something at this
		 *	address, we have to start after it.
		 */

		if (start == map->vm_min_offset) {
			if ((entry = map->vm_first_free) !=
					vm_map_to_entry(map))
				start = entry->vme_end;
		} else {
			vm_map_entry_t	tmp_entry;

			hint = start;
			first_time = TRUE;
			if (vm_map_lookup_entry(map, start, &tmp_entry))
				start = tmp_entry->vme_end;
			entry = tmp_entry;
		}

		/*
		 *	In any case, the "entry" always precedes
		 *	the proposed new region throughout the
		 *	loop:
		 */
		while (TRUE) {
			register vm_map_entry_t	next;

		    	/*
			 *	Find the end of the proposed new region.
			 *	Be sure we didn't go beyond the end, or
			 *	wrap around the address.
			 */
			start = ((start + mask) & ~mask);
			end = start + size;

			if ((end > map->vm_max_offset) || (end < start) ||
			    (hint && !first_time && start >= hint)) {
				if (hint && first_time) {
					start = map->vm_min_offset;
					if ((entry = map->vm_first_free) !=
							vm_map_to_entry(map))
						start = entry->vme_end;
					first_time = FALSE;
					continue;
				}
				if (map->vm_wait_for_space &&
				    (size <= (map->vm_max_offset -
						     map->vm_min_offset))) {
					assert_wait((vm_offset_t)map, TRUE);
					vm_map_unlock(map);
					thread_block();
					goto again;
				}
				RETURN(KERN_NO_SPACE);
			}

			/*
			 *	If there are no more entries, we must win.
			 */
			next = entry->vme_next;
			if (next == vm_map_to_entry(map))
				break;

			/*
			 *	If there is another entry, it must be
			 *	after the end of the potential new region.
			 */

			if (next->vme_start >= end)
				break;

			/*
			 *	Didn't fit -- move to the next entry.
			 */

			entry = next;
			start = entry->vme_end;
		}
		*address = start;
	} else {
		vm_map_entry_t		temp_entry;

		/*
		 *	Verify that:
		 *		the address doesn't itself violate
		 *		the mask requirement.
		 */

		if ((start & mask) != 0)
			return KERN_NO_SPACE;

		vm_map_lock(map);

		/*
		 *	...	the address is within bounds
		 */

		end = start + size;

		if ((start < map->vm_min_offset) ||
		    (end > map->vm_max_offset) ||
		    (start >= end)) {
			RETURN(KERN_INVALID_ADDRESS);
		}

		/*
		 *	...	the starting address isn't allocated
		 */

		if (vm_map_lookup_entry(map, start, &temp_entry))
			RETURN(KERN_NO_SPACE);

		entry = temp_entry;

		/*
		 *	...	the next region doesn't overlap the
		 *		end point.
		 */

		if ((entry->vme_next != vm_map_to_entry(map)) &&
		    (entry->vme_next->vme_start < end))
			RETURN(KERN_NO_SPACE);
	}

	*prev_entry = entry;
	return KERN_SUCCESS;

failed:
	vm_map_unlock(map);
	return result;
#undef	RETURN
}

/*
 *	vm_map_clip_start
 */

kern_return_t
vm_map_clip_start(vm_map_t map,
	register vm_map_entry_t entry,
	register vm_offset_t start)
{
	vm_map_entry_t	new_entry;
	kern_return_t ret;

	if (start <= entry->vme_start) return KERN_SUCCESS;
	else if (ret = vm_map_entry_create(map, &new_entry))
		 	return(ret);

	/*
	 *	Split off the front portion --
	 *	note that we must insert the new
	 *	entry BEFORE this one, so that
	 *	this entry has the specified starting
	 *	address.
	 */

	*new_entry = *entry;

	new_entry->vme_end = start;
	entry->vme_offset += (start - entry->vme_start);
	entry->vme_start = start;

	vm_object_reference(new_entry->vme_object);

	vm_map_entry_link(map, entry->vme_prev, new_entry);
	return KERN_SUCCESS;
}


/*
 *	vm_map_clip_end
 */

kern_return_t
vm_map_clip_end(vm_map_t map,
	register vm_map_entry_t	entry,
	register vm_offset_t end)
{
	vm_map_entry_t	new_entry;
	kern_return_t ret;

	if (end >= entry->vme_end) return KERN_SUCCESS;
	else if (ret = vm_map_entry_create(map, &new_entry))
		return(ret);

	/*
	 *	Fill in an entry to be placed
	 *	AFTER the specified entry
	 */

	*new_entry = *entry;

	new_entry->vme_start = entry->vme_end = end;
	new_entry->vme_offset += (end - entry->vme_start);

	vm_object_reference(new_entry->vme_object);
	vm_map_entry_link(map, entry, new_entry);

	return KERN_SUCCESS;
}

	
/*
 *	VM_MAP_RANGE_CHECK:	[ internal use only ]
 *
 *	Asserts that the starting and ending region
 *	addresses fall within the valid range of the map.
 *	Returns error for selected routines.
 */
#define VM_MAP_RANGE_CHECK(map, start, end, error)	\
		{					\
		error = 0;				\
		if (start < vm_map_min(map)){		\
			start = vm_map_min(map);	\
			error = KERN_INVALID_ADDRESS;  	\
		}					\
		if (end > vm_map_max(map)){		\
			end = vm_map_max(map);		\
			error = KERN_INVALID_ADDRESS;  	\
		}					\
		if (start > end){			\
			start = end;			\
			error = KERN_INVALID_ADDRESS;  	\
		}					\
		}

kern_return_t
vm_map_pageable(vm_map_t map,
		vm_offset_t start,
		vm_offset_t end,
		vm_prot_t access_type)
{
	int error;

	VM_MAP_RANGE_CHECK(map, start, end, error);

	switch (access_type) {
		case VM_PROT_READ:
		case VM_PROT_READ|VM_PROT_WRITE:
		case VM_PROT_WRITE:
		case VM_PROT_NONE:
			break;
		default:
			return KERN_INVALID_ARGUMENT;
	}

	start = trunc_page(start);
	end = round_page(end);

	return (*map->vm_wire_map)(map, start, end, access_type);
}

kern_return_t
vm_map_delete(vm_map_t map,
		vm_offset_t start,
		vm_offset_t end,
		boolean_t contain)
{
	kern_return_t ret;
	int error;

	VM_MAP_RANGE_CHECK(map, start, end, error);
	if (error)
		return (error);
	start = trunc_page(start);
	end = round_page(end);
	vm_map_lock(map);
	ret = (*map->vm_delete_map)(map, start, end, contain);
	vm_map_unlock(map);
	return ret;
}

kern_return_t
vm_map_protect(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	vm_prot_t new_prot,
	boolean_t set_max)
{
	int error;

	VM_MAP_RANGE_CHECK(map, start, end, error);

	return (*map->vm_protect_map)(map, start, end, new_prot, set_max);
}


/*
 *	vm_map_inherit:
 *
 *	Sets the inheritance of the specified address
 *	range in the target map.  Inheritance
 *	affects how the map will be shared with
 *	child maps at the time of vm_map_fork.
 */

kern_return_t
vm_map_inherit(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	vm_inherit_t new_inheritance)
{
	switch (new_inheritance) {
	case VM_INHERIT_NONE:
	case VM_INHERIT_COPY:
	case VM_INHERIT_SHARE:
		break;
	default:
		return KERN_INVALID_ARGUMENT;
	}


	return (*map->vm_inherit_map)(map, start, end, new_inheritance);
}

/*
 *	vm_map_keep_on_exec:
 *
 *	Sets the keep-on-exec state of the specified address
 *	range in the target map.  A keep-on-exec region is
 *	preserved across vm_map_exec operations, while all
 *	other regions are deleted.  This implements behavior
 *	of the UNIX mmap(MAP_INHERIT) function.
 */

kern_return_t
vm_map_keep_on_exec(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	boolean_t new_koe)
{
	return (*map->vm_keep_on_exec_map)(map, start, end, new_koe);
}


kern_return_t
vm_map_exec(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end)
{
	return (*map->vm_exec_map)(map,start, end);
}

/*
 *	vm_map_entry_delete:	[ internal use only ]
 *
 *	Deallocate the given entry from the target map.
 */	

void
vm_map_entry_delete(register vm_map_t map,
	register vm_map_entry_t entry)
{
	
	vm_map_entry_unlink(map, entry);

	assert((entry->vme_is_sub_map == 0));
 	vm_object_deallocate(entry->vme_object);
	vm_map_entry_dispose(map, entry);
}


/*
 *	vm_map_remove:
 *
 *	Remove the given address range from the target map.
 *	This is the exported form of vm_map_delete.
 */

kern_return_t
vm_map_remove(vm_map_t map,
	vm_offset_t start,
	vm_offset_t end)
{
	register kern_return_t result;
	int error;

	vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end, error);
	result = (*map->vm_delete_map)(map, start, end, TRUE);
	vm_map_unlock(map);

	return(result);
}


/*
 *	vm_map_check_protection:
 *
 *	Assert that the target map allows the specified
 *	privilege on the entire address region given.
 *	The entire region must be allocated.
 */

boolean_t
vm_map_check_protection(
	vm_map_t map,
	vm_offset_t start,
	vm_offset_t end,
	vm_prot_t protection)
{

	return (*map->vm_check_protection_map)(map, start, end, protection);
}


/*
 *	vm_map_find
 */

kern_return_t
vm_map_find(vm_map_t map,
	vm_object_t object,
	vm_offset_t offset,
	vm_offset_t *addr,
	vm_size_t length,
	boolean_t find_space)
{
	return (*map->vm_allocate_map)(map, object, offset, addr,
			length, find_space);
}

/*
 *	vm_map_copy_discard
 */

void	
vm_map_copy_discard(register vm_map_copy_t copy)
{
	register vm_map_entry_t entry;

	if (copy == VM_MAP_COPY_NULL)
		return;

	while (vm_map_copy_first_entry(copy) != vm_map_copy_to_entry(copy)) {
		entry = vm_map_copy_first_entry(copy);
		if ((*entry->vme_copy)(entry, VME_COPYFREE) != KERN_SUCCESS)
			panic("vm_map_copy_discard: entry unmap failed");
		vm_map_entry_unlink(copy, entry);
		vm_object_deallocate(entry->vme_object);
		vm_map_entry_dispose((vm_map_t) copy, entry);
	}
	zfree(vm_map_copy_zone, (vm_offset_t) copy);
}

/*
 *	Routine:	vm_map_copy_overwrite
 *
 *	Description:
 *		Copy the memory described by the map copy
 *		object (copy; returned by vm_map_copyin) onto
 *		the specified destination region (dst_map, dst_addr).
 *		The destination must be writeable.
 *
 *		Unlike vm_map_copyout, this routine actually
 *		writes over previously-mapped memory.  If the
 *		previous mapping was to a permanent (user-supplied)
 *		memory object, it is preserved.
 *
 *		The attributes (protection, inheritance, keep_on_exec)
 *		of the destination region are preserved.
 *
 *		The map copy object is not destroyed, but it may
 *		no longer contain any valid data.  The caller is
 *		responsible for calling vm_map_copy_discard.
 *
 *	Implementation notes:
 *		To overwrite temporary virtual memory, it is
 *		sufficient to remove the previous mapping and insert
 *		the new copy.  This replacement is done either on
 *		the whole region (if no permanent virtual memory
 *		objects are embedded in the destination region) or
 *		in individual map entries.
 *
 *		To overwrite permanent virtual memory, it is
 *		necessary to copy each page, as the external
 *		memory management interface currently does not
 *		provide any optimizations.
 *
 *		Once a page of permanent memory has been overwritten,
 *		it is impossible to interrupt this function; otherwise,
 *		the call would be neither atomic nor location-independent.
 *		The kernel-state portion of a user thread must be
 *		interruptible.
 *
 *		It may be expensive to forward all requests that might
 *		overwrite permanent memory (vm_write, vm_copy) to
 *		uninterruptible kernel threads.  This routine may be
 *		called by interruptible threads; however, success is
 *		not guaranteed -- if the request cannot be performed
 *		atomically and interruptibly, an error indication is
 *		returned.
 */
kern_return_t
vm_map_copy_overwrite(
	vm_map_t dst_map,
	vm_offset_t dst_addr,
	vm_map_copy_t copy,
	boolean_t interruptible)
{
	vm_size_t	size;


	interruptible = FALSE;	/* XXX */

	/*
	 *	Check for null copy object.
	 */

	if (copy == VM_MAP_COPY_NULL) return KERN_SUCCESS;

	/*
	 *	Check for special copy object, created
	 *	by vm_map_copyin_object.
	 */

	assert(copy->vm_entries_pageable);

	/*
	 *	Currently this routine only handles page-aligned
	 *	regions.  Eventually, it should handle misalignments
	 *	by actually copying pages.
	 */

	assert(page_aligned(copy->vm_min_offset));
	assert(page_aligned(copy->vm_max_offset));
	assert(page_aligned(dst_addr));

	size = copy->vm_max_offset - copy->vm_min_offset;

	if (size == 0) return KERN_SUCCESS;

	return (*dst_map->vm_copy_overwrite_map)(dst_map, dst_addr, copy,
			interruptible, size);
}


/*
 *	Routine:	vm_map_copyout
 *
 *	Description:
 *		Copy out a copy chain ("copy") into newly-allocated
 *		space in the destination map.
 */

kern_return_t
vm_map_copyout(vm_map_t dst_map,
	vm_offset_t *dst_addr,
	vm_map_copy_t copy)
{
	vm_size_t	size;

	/*
	 *	Check for null copy object.
	 */

	if (copy == VM_MAP_COPY_NULL) {
		*dst_addr = 0;
		return KERN_SUCCESS;
	}

	return (*dst_map->vm_copyout_map)(dst_map, dst_addr, copy);
}

/*
 *	Routine:	vm_map_copyin
 *
 *	Description:
 *		Copy the specified region (src_addr, len) from the
 *		source address space (src_map), possibly removing
 *		the region from the source address space (src_destroy).
 *
 *	Returns:
 *		A vm_map_copy_t object (copy_result), suitable for
 *		insertion into another address space (using vm_map_copyout),
 *		copying over another address space region (using
 *		vm_map_copy_overwrite).  If the copy is unused, it
 *		should be destroyed (using vm_map_copy_discard).
 *
 *	In/out conditions:
 *		The source map should not be locked on entry.
 */

kern_return_t vm_map_copyin(vm_map_t src_map,
	register vm_offset_t src_addr,
	vm_size_t len,
	boolean_t src_destroy,
	vm_map_copy_t *copy_result)
{
	register vm_offset_t src_start, src_end;

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

	src_start = trunc_page(src_addr);
	src_end = round_page(src_addr + len);

	/*
	 *	Check that the end address doesn't overflow
	 */

	if (src_end <= src_start)
		if ((src_end < src_start) || (src_start != 0))
			return KERN_INVALID_ADDRESS;


	return (*src_map->vm_copyin_map)(src_map, src_addr, len,
			src_destroy, copy_result);
}

kern_return_t
vm_map_enter( vm_map_t	map,
	vm_offset_t *address,	/* IN/OUT */
	vm_size_t size,
	vm_offset_t mask,
	boolean_t anywhere,
	vm_object_t object,
	vm_offset_t offset,
	boolean_t needs_copy,
	vm_prot_t cur_protection,
	vm_prot_t max_protection,
	vm_inherit_t inheritance)
{
	return (*map->vm_enter_map)(map, address, size, mask,
		anywhere, object, offset, needs_copy, cur_protection,
		max_protection, inheritance);
}

/*
 *
 */

vm_map_t
vm_map_fork(vm_map_t old_map)
{
	vm_map_t new_map;
	pmap_t new_pmap;
	kern_return_t ret;


	new_pmap = pmap_create((vm_size_t) 0);
	vm_map_lock(old_map);

	new_map = vm_map_create(new_pmap,
			old_map->vm_min_offset,
			old_map->vm_max_offset,
			old_map->vm_entries_pageable);

	ret = (*old_map->vm_fork_map)(old_map, new_map);
	vm_map_unlock(old_map);

	if (ret != KERN_SUCCESS) {
		vm_map_deallocate(new_map);
		return (vm_map_t) 0;
	}
	else return new_map;
}


/*
 * Not supported too well.
 * The is_shared returned value could be wrong.
 */

kern_return_t
vm_region(vm_map_t map,
	vm_offset_t *address,
	vm_size_t *size,
	vm_prot_t *protection,
	vm_prot_t *max_protection,
	vm_inherit_t *inheritance,
	boolean_t *is_shared,
	port_t *object_name,
	vm_offset_t *offset_in_object)
{
	vm_map_entry_t tmp_entry;
	register vm_map_entry_t	entry;
	vm_offset_t start;

	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);

	start = *address;

	vm_map_lock_read(map);
	if (!vm_map_lookup_entry(map, start, &tmp_entry)) {
		if ((entry = tmp_entry->vme_next) == vm_map_to_entry(map)) {
			vm_map_unlock_read(map);
		   	return KERN_NO_SPACE;
		}
	} else {
		entry = tmp_entry;
	}

	start = entry->vme_start;
	*address = start;
	*size = (entry->vme_end - start);
	*protection = entry->vme_protection;
	*max_protection = entry->vme_maxprot;
	*inheritance = entry->vme_inheritance;
	*is_shared = FALSE;
	*object_name = PORT_NULL;
	*offset_in_object = entry->vme_offset;

	vm_map_unlock_read(map);

	return KERN_SUCCESS;
}


/*
 *	Routine:	vm_map_machine_attribute
 *	Purpose:
 *		Provide machine-specific attributes to mappings,
 *		such as cachability etc. for machines that provide
 *		them.  NUMA architectures and machines with big/strange
 *		caches will use this.
 *	Note:
 *		Responsibilities for locking and checking are handled here,
 *		everything else in the pmap module. If any non-volatile
 *		information must be kept, the pmap module should handle
 *		it itself. [This assumes that attributes do not
 *		need to be inherited, which seems ok to me]
 */

kern_return_t
vm_map_machine_attribute(vm_map_t map,
	vm_offset_t address,
	vm_size_t size,
	vm_machine_attribute_t attribute,
	vm_machine_attribute_t *value)
{
	kern_return_t	ret;

	if (address < vm_map_min(map) ||
	    (address + size) > vm_map_max(map))
		return KERN_INVALID_ARGUMENT;

	vm_map_lock(map);

	ret = pmap_attribute(map->vm_pmap, address, size, attribute, value);

	vm_map_unlock(map);

	return ret;
}


/***
 ***	This function calculates the virtual space actually used by a map.
 ***	This differs from field "vm_size" since the latter includes the
 ***	entire size of a segment rather than the size of the subsegment a
 ***	map entry is attached to.
 ***/
vm_size_t
vm_map_actual_size(vm_map_t map, int kernel_map)
{
	vm_map_entry_t	ep;
	vm_size_t	result;

	if (kernel_map)
		return (map->vm_size);

	result = 0;
	vm_map_lock_read(map);
	for (ep = vm_map_first_entry(map); ep != vm_map_to_entry(map);
		ep = ep->vme_next) {
		if (vm_object_type(ep->vme_object) == OT_SEG)
			result += round_page(ep->vme_seg->seg_size);
		else
			result += ep->vme_end - ep->vme_start;
	}
	vm_map_unlock_read(map);

	return (result);
}

vm_offset_t
vm_mapinfo(map, va, table_entry)
	vm_map_t		map;
	register vm_offset_t	va;
	struct tbl_mapinfo *	table_entry;
{
	register vm_map_entry_t	cur;
	register vm_map_entry_t	last;

	vm_map_lock_read(map);
	if (vm_map_first_entry(map) == vm_map_to_entry(map)) {
		/* no map entries */
		vm_map_unlock_read(map);
		return (vm_offset_t)0;
	}

	simple_lock(&map->vm_hint_lock);
	cur = map->vm_hint;
	simple_unlock(&map->vm_hint_lock);
	if (cur == vm_map_to_entry(map))
		cur = vm_map_first_entry(map);

	if (va < cur->vme_start) {
		last = cur->vme_next;
		cur = vm_map_first_entry(map);
	} else
		last = vm_map_to_entry(map);

	while (cur != last && va >= cur->vme_end)
		cur = cur->vme_next;
	SAVE_HINT(map, cur);
	if (cur == last) {
		vm_map_unlock_read(map);
		return (vm_offset_t)0;
	}
	if (va < cur->vme_start)
		va = cur->vme_start;

	table_entry->access = cur->vme_protection;
	va = pmap_pt_info(vm_map_pmap(map), va, cur, table_entry);
	vm_map_unlock_read(map);
	return va;
}
