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
static char	*sccsid = "@(#)$RCSfile: vm_kern.c,v $ $Revision: 4.2.18.7 $ (DEC) $Date: 1993/12/02 22:04:56 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from vm_kern.c	2.1	(ULTRIX/OSF)	12/3/90";
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
 *	File:	vm/vm_kern.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Kernel memory management.
 */

#include <mach_xp_fpd.h>

#include <kern/assert.h>
#include <mach/kern_return.h>
#include <sys/types.h>

#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <mach/vm_param.h>
#include <sys/table.h>	/* for SAR counters */
#include <sys/dk.h>
#ifdef MEMLOG
#include <sys/memlog.h>
#endif

/*
 *	Variables exported by this module.
 */

vm_map_t	kernel_map;
vm_map_t	kernel_pageable_map;
vm_map_t	kernel_copy_map;
vm_map_t	user_pt_map;

vm_offset_t	vm_kern_zero_page;

/*
 *	kmem_alloc_pageable:
 *
 *	Allocate pageable memory.
 *	This is pageable memory and not to be overloaded with the
 *	misguided practices of the past.
 */

vm_offset_t kmem_alloc_pageable(vm_map_t map, vm_size_t size)
{
	vm_offset_t addr;

	if (kernel_memory_allocate_paged(map, &addr, size, TRUE) == 
		KERN_SUCCESS) return addr;
	else return (vm_offset_t)0;
}

kern_return_t 
kernel_memory_allocate_paged(
	vm_map_t map, 
	vm_offset_t *addrp, 
	vm_size_t size, 
	boolean_t anywhere)
{
	register kern_return_t result;
	vm_offset_t addr;


	if (map->vm_copy_map) 
		result = vm_allocate(map, addrp, size, anywhere);
        else {
		size = round_page(size);
		if (anywhere) addr = vm_map_min(map);
		else addr = trunc_page(*addrp);
		result = k_map_allocate_paged(map, pkernel_object, 
			&addr, size, VM_PROT_READ|VM_PROT_WRITE, anywhere);
		*addrp = addr;
	}
	if (result != KERN_SUCCESS) {
        	/* global table() system call counter (see table.h) */
		sar_kmem_fail++;
	}
	return result;
}

/*
 *	Allocate wired-down memory anywhere in the kernel's address map
 *	or a submap.  The memory is wired such that read and write,
 *	but not execute, accesses will not cause page faults.
 */

vm_offset_t 
kmem_alloc(vm_map_t map, vm_size_t size)
{
	vm_offset_t addr;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(KMEM_ALLOC_LOG, caller, size);
	}
#endif

	if (kernel_memory_allocate_wired(map, &addr, size,
			(VM_PROT_READ|VM_PROT_WRITE), TRUE) == KERN_SUCCESS)
		return(addr);
	else
		return((vm_offset_t)0);
}

/*
 *	Allocate wired-down memory in the kernel's address map
 *	or a submap.
 */

kern_return_t 
kernel_memory_allocate_wired(
	register vm_map_t map, 
	register vm_offset_t *addrp, 
	register vm_size_t size, 
	register vm_prot_t wire_prot, 
	boolean_t anywhere)
{
	vm_offset_t addr;
	register kern_return_t result;

	size = round_page(size);
	if (anywhere) addr = vm_map_min(map);
	else addr = trunc_page(*addrp);

	if (map->vm_copy_map) 
		result = c_map_allocate_nonpaged(map, &addr, size,
					wire_prot, anywhere);
	else result = k_map_allocate_nonpaged(map, kernel_object, &addr, size, 
			wire_prot, anywhere);

	if (result != KERN_SUCCESS) {
		*addrp = (vm_offset_t) 0;
        	/* global table() system call counter (see table.h) */
		sar_kmem_fail++;
		return result;	
	}
	else {
		*addrp = addr;
		return KERN_SUCCESS;
	}
}

/*
 *	kmem_free:
 *
 *	Release a region of kernel virtual memory allocated
 *	with kmem_alloc, and return the physical pages
 *	associated with that region.
 */

void 
kmem_free(vm_map_t map, 
	register vm_offset_t addr, 
	vm_size_t size)
{
	kern_return_t ret;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(KMEM_FREE_LOG, caller, size);
	}
#endif
 
	ret = vm_map_remove(map, trunc_page(addr), round_page(addr + size));
	assert(ret == KERN_SUCCESS);
}

/*
 *	kmem_suballoc:
 *
 *	Allocates a map to manage a subrange
 *	of the kernel virtual address space.
 *
 *	Arguments are as follows:
 *
 *	parent		Map to take range from
 *	size		Size of range to find
 *	min, max	Returned endpoints of map
 *	pageable	Can the region be paged
 */

vm_map_t 
kmem_suballoc(register vm_map_t parent, 
	vm_offset_t *min, 
	vm_offset_t *max, 
	vm_size_t size, 
	boolean_t pageable)
{
	register kern_return_t	ret;
	vm_map_t result;

	size = round_page(size);

	*min = vm_map_min(parent);
	*max = *min + size;
	pmap_reference(vm_map_pmap(parent));
	result = vm_map_create(vm_map_pmap(parent), *min, *max, pageable);
	if (result == VM_MAP_NULL)
		panic("kmem_suballoc: cannot create submap");
	if ((ret = k_map_submap(parent, min, max, size, result)) != 
			KERN_SUCCESS)
		panic("kmem_suballoc: unable to change range to submap");
	return result;
}

#if	MACH_XP_FPD 

#ifndef	map_physical_page

#include <kern/lock.h>
#include <kern/zalloc.h>

lock_data_t	v_to_p_lock_data;
lock_t		v_to_p_lock = &v_to_p_lock_data;
zone_t		v_to_p_zone;
boolean_t	v_to_p_initialized = FALSE;
int 		n_v_to_p;
int		v_to_p_hiwater;

struct 		v_to_p_entry {
		struct v_to_p_entry * 	next;
		vm_offset_t		vaddr;
} *free_v_to_p_list, *allocated_v_to_p_list;

#define V_TO_P_NULL	((struct v_to_p_entry *)0)

/*
 *	Routine:	map_physical_page
 *
 * 	Purpose:
 *		Establish a temporary mapping to a physical page,
 *		in order to copy data into it.
 *	
 *	Algorithm:
 *		Allocate a kernel virtual address using kmem_alloc_pageable
 *		and pmap_enter and remove the target page m to that address.
 *		For speed, previous kmem_alloc_pageable results are kept
 *		around in a linked list to allow quick reuse.  
 */

vm_offset_t
map_physical_page(vm_offset_t physaddr)
{
	struct v_to_p_entry *entry;

	/*
	 * Lock access to virtual address list.
	 */
	lock_write(v_to_p_lock);
	if (!v_to_p_initialized) {
		/*
		 * Create a zone for v_to_p_entries.  Setup list.
		 */		
		v_to_p_zone = zinit(sizeof(struct v_to_p_entry),
				sizeof(struct v_to_p_entry), 
				sizeof(struct v_to_p_entry)*1024,
				"v_to_p_zone");
		v_to_p_initialized = TRUE;
		free_v_to_p_list = V_TO_P_NULL;
		allocated_v_to_p_list = V_TO_P_NULL;
	}

	while (free_v_to_p_list == V_TO_P_NULL) {
		vm_offset_ addr;

		/*
		 * Allocate a new entry from zone.
		 */
		free_v_to_p_list = (struct v_to_p_entry *)zalloc(v_to_p_zone);
		if (free_v_to_p_list == V_TO_P_NULL) {
			/*
			 * Give someone a chance to return entry to pool.
			 */
			lock_write_done(v_to_p_lock);
			thread_block();
			lock_write(v_to_p_lock);
			continue;
		}
		free_v_to_p_list->next = V_TO_P_NULL;
		free_v_to_p_list->vaddr = vm_alloc_kva(PAGE_SIZE);
		if (free_v_to_p_list->vaddr == (vm_offset_t) 0)
			panic("map_physical_page: No kernel memory");
	}

	entry = free_v_to_p_list;
	free_v_to_p_list = free_v_to_p_list->next;
	entry->next = allocated_v_to_p_list;
	allocated_v_to_p_list = entry;
	if (++n_v_to_p > v_to_p_hiwater)
		v_to_p_hiwater = n_v_to_p;
	lock_write_done(v_to_p_lock);
	ret = pmap_load(entry->vaddr, physaddr, PAGE_SIZE, 
		(VM_PROT_READ|VM_PROT_WRITE), TB_SYNC_ALL);
	return entry->vaddr;
}

/*
 *	Routine:	unmap_physical_page
 *
 * 	Purpose:
 *		Remove a previously established temporary mapping
 *		to a physical page.
 *	
 *	Algorithm:
 *		Use pmap_remove to undo the mapping, take the entry off
 *		the allocated list, and insert it onto the free list
 *		(of available virtual addresses).
 */

void
unmap_physical_page(vm_offset_t kvaddr)
{
	struct v_to_p_entry *entry, *prev_entry;

	pmap_unload(kvaddr, PAGE_SIZE, TB_SYNC_ALL);

	prev_entry = V_TO_P_NULL;
	lock_write(v_to_p_lock);
	for (entry = allocated_v_to_p_list; entry != V_TO_P_NULL; entry = entry->next) {
		if (entry->vaddr == kvaddr) {
			if (prev_entry != V_TO_P_NULL)
				prev_entry->next = entry->next;
			else
				allocated_v_to_p_list = entry->next;
			entry->next = free_v_to_p_list;
			free_v_to_p_list = entry;
			--n_v_to_p;
			lock_done(v_to_p_lock);
			return;
		}
		prev_entry = entry;
	}
	panic("unmap_phys: no v_to_p");
}
#endif	/* map_physical_page */

#if	MACH_XP_FPD
/*
 *	Routine:	copy_user_to_physical_page
 *
 * 	Purpose:	
 *		Copy a virtual page which may be either kernel or
 *		user and may be currently paged out, to a physical
 *		page. 
 *	
 *	Algorithm:
 *		Establish a temporary mapping to the physical page,
 *		copy in the data, and remove the temporary mapping.
 */

copy_user_to_physical_page(vm_offset_t v, 
	vm_page_t m, 
	vm_size_t data_cnt) 
{
	vm_offset_t kva;

	kva = map_physical_page(m->phys_addr);
	copyin((caddr_t) v, (caddr_t) kva, (unsigned int) data_cnt);
	unmap_physical_page(kva);
}
#endif	/* MACH_XP_FPD */
#endif	/* MACH_XP_FPD */

#include <sys/param.h>

/*
 *	kmem_init:
 *
 *	Initialize the kernel's virtual memory map, taking
 *	into account all memory allocated up to this time.
 */

void 
kmem_init(vm_offset_t start, vm_offset_t end)
{
	vm_offset_t addr;
	extern vm_map_t	kernel_map;

	kernel_map = vm_map_create(pmap_kernel(), 
		VM_MIN_KERNEL_ADDRESS, end, FALSE);
	addr = VM_MIN_KERNEL_ADDRESS;
	k_map_allocate_va(kernel_map, nkernel_object, &addr, 
			start - VM_MIN_KERNEL_ADDRESS, FALSE);

	vm_kern_zero_page = kmem_alloc(kernel_map, PAGE_SIZE);

#if	MACH_XP_FPD &&	!defined(map_physical_page)
	v_to_p_initialized = FALSE;
	lock_init(v_to_p_lock, TRUE);
#endif
}

kern_return_t
vm_map_io(phys, siz, kva)
	vm_offset_t phys;
	vm_size_t   siz;
	vm_offset_t *kva;
{
	kern_return_t ret;

	if(siz % PAGE_SIZE)
		return KERN_FAILURE;
	*kva = vm_alloc_kva(siz);
	if(*kva == NULL) 
		return KERN_NO_SPACE;
	ret = pmap_map_io(phys, siz, *kva, (VM_PROT_READ|VM_PROT_WRITE), TB_SYNC_ALL);
	if(ret != KERN_SUCCESS)
		kmem_free(kernel_map, *kva, siz);
	return ret;
}

kern_return_t 
vm_dup_va(umap, uva, siz, kva, kprot)
	vm_map_t    umap;
	vm_offset_t uva;
	vm_size_t   siz;
	vm_offset_t *kva;
	vm_prot_t   kprot;
{
	kern_return_t ret;
	vm_offset_t	uva_offset;

	uva_offset = (uva & (PAGE_SIZE-1));

	if ((uva_offset+siz) > PAGE_SIZE)
                siz+=PAGE_SIZE;

	*kva = vm_alloc_kva(siz);
	if(*kva == NULL)
		return KERN_NO_SPACE;
	ret = pmap_dup(umap->vm_pmap, uva, siz, *kva, kprot, TB_SYNC_NONE);
	if(ret != KERN_SUCCESS)
		kmem_free(kernel_map, *kva, siz);
	*kva += uva_offset;
	return ret;
}

kern_return_t
vm_free_dup_va(kva, siz)
	vm_offset_t kva; 
	vm_size_t   siz;
{
        if ((kva - trunc_page(kva) + siz) > PAGE_SIZE)
                siz+=PAGE_SIZE;
	kva = trunc_page(kva);
	kmem_free(kernel_map, kva, siz);
	pmap_mmu_unload(kva,round_page(siz),TB_SYNC_LOCAL);
	tbsync(pmap_kernel(), kva, siz);
	return KERN_SUCCESS;
}
