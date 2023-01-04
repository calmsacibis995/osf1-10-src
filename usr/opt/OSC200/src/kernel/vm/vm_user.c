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
static char	*sccsid = "@(#)$RCSfile: vm_user.c,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/10/19 19:45:57 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	vm/vm_user.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory functions exported to users.
 *	[See "mach/mach.defs" for full interface definition.]
 */

#include <sys/types.h>

#include <mach/vm_param.h>
#include <mach/std_types.h>	/* to get pointer_t */
#include <mach/mach_types.h>	/* to get vm_address_t */
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <vm/vm_object.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_pagelru.h>
#include <mach/vm_statistics.h>
#include <mach/vm_attributes.h>

#include <kern/task.h>

#include <kern/ipc_globals.h>

#include <mach/memory_object.h>

#include <vm/vm_perf.h>

vm_statistics_data_t	vm_stat;

/*
 *	vm_allocate allocates "zero fill" memory in the specfied
 *	map.
 */
kern_return_t vm_allocate(map, addr, size, anywhere)
	register vm_map_t	map;
	register vm_offset_t	*addr;
	register vm_size_t	size;
	boolean_t		anywhere;
{
	kern_return_t	result;

	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);
	if (size == 0) {
		*addr = 0;
		return(KERN_SUCCESS);
	}

	if (anywhere)
		if ((*addr < vm_map_min(map)) || (*addr > vm_map_max(map)))
			*addr = vm_map_min(map);
	else
		*addr = trunc_page(*addr);
	size = round_page(size);

	result = vm_map_find(map, VM_OBJECT_NULL, (vm_offset_t) 0, addr,
			size, anywhere);

	return(result);
}

/*
 *	vm_deallocate deallocates the specified range of addresses in the
 *	specified address map.
 */
kern_return_t vm_deallocate(map, start, size)
	register vm_map_t	map;
	vm_offset_t		start;
	vm_size_t		size;
{
	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);

	if (size == (vm_offset_t) 0)
		return(KERN_SUCCESS);

	return(vm_map_remove(map, trunc_page(start), round_page(start+size)));
}

/*
 *	vm_inherit sets the inheritence of the specified range in the
 *	specified map.
 */
kern_return_t vm_inherit(map, start, size, new_inheritance)
	register vm_map_t	map;
	vm_offset_t		start;
	vm_size_t		size;
	vm_inherit_t		new_inheritance;
{
	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);

	return(vm_map_inherit(map, trunc_page(start), round_page(start+size), new_inheritance));
}

/*
 *	vm_protect sets the protection of the specified range in the
 *	specified map.
 */

kern_return_t vm_protect(map, start, size, set_maximum, new_protection)
	register vm_map_t	map;
	vm_offset_t		start;
	vm_size_t		size;
	boolean_t		set_maximum;
	vm_prot_t		new_protection;
{
	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);

	return(vm_map_protect(map, trunc_page(start), round_page(start+size), new_protection, set_maximum));
}

kern_return_t vm_statistics(map, stat)
	vm_map_t	map;
	vm_statistics_data_t	*stat;
{

extern int ubc_pages;

	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);
	
	*stat = vm_stat;

	stat->pagesize = PAGE_SIZE;
	stat->free_count = vm_page_free_count;
	stat->active_count = vm_page_active_count + ubc_pages;
	stat->inactive_count = vm_page_inactive_count;
	stat->wire_count = vm_page_wire_count;
	stat->reactivations = vpf_sload(reactivate);
	stat->zero_fill_count = vpf_sload(zfod) + vpf_sload(kzfod);
	stat->cow_faults = vpf_sload(cowfaults);
	stat->pageins = vpf_sload(pgioreads);
	stat->pageouts = vpf_sload(pgiowrites);
	stat->faults = vpf_sload(pagefaults);

	return KERN_SUCCESS;
}

kern_return_t vm_read(map, address, size, data, data_size)
	vm_map_t	map;
	vm_address_t	address;
	vm_size_t	size;
	pointer_t	*data;
	unsigned int	*data_size; /* type must match mig caller _Xvm_read */
{
	kern_return_t	error;
	vm_map_copy_t	ipc_address;

	if ((trunc_page(address) != address) || (trunc_page(size) != size))
		return(KERN_INVALID_ARGUMENT);

	if ((error = vm_map_copyin(map, address, size, FALSE, &ipc_address))
			== KERN_SUCCESS) {
		*data = (pointer_t) ipc_address;
		*data_size = size;
	}
	return(error);
}

kern_return_t vm_write(map, address, data, size)
	vm_map_t	map;
	vm_address_t	address;
	pointer_t	data;
	unsigned int	size; /* type must match mig caller _Xvm_write */
{
	if ((trunc_page(address) != address) || (trunc_page(size) != size))
		return(KERN_INVALID_ARGUMENT);

	return(vm_map_copy_overwrite(
			map,
			address,
			(vm_map_copy_t) data,
			FALSE /* interruptible XXX */)
			);
}

kern_return_t vm_copy(map, source_address, size, dest_address)
	vm_map_t	map;
	vm_address_t	source_address;
	vm_size_t	size;
	vm_address_t	dest_address;
{
	vm_map_copy_t	copy;
	kern_return_t	error;

	if ( (trunc_page(source_address) != source_address) || (trunc_page(dest_address) != dest_address)
	     || (trunc_page(size) != size) )
		return(KERN_INVALID_ARGUMENT);

	if ((error = vm_map_copyin(map, source_address, size, FALSE, &copy))
			!= KERN_SUCCESS)
		return(error);

	error = vm_map_copy_overwrite(map, dest_address, copy, FALSE /* interruptible XXX */ );
	vm_map_copy_discard(copy);

	return(error);
}


/*
 *	Routine:	vm_map
 */
kern_return_t vm_map(
		target_map,
		address, size, mask, anywhere,
		memory_object, offset,
		copy,
		cur_protection, max_protection,	inheritance)
	vm_map_t	target_map;
	vm_offset_t	*address;
	vm_size_t	size;
	vm_offset_t	mask;
	boolean_t	anywhere;
	memory_object_t	memory_object;
	vm_offset_t	offset;
	boolean_t	copy;
	vm_prot_t	cur_protection;
	vm_prot_t	max_protection;
	vm_inherit_t	inheritance;
{
	register
	vm_object_t	object;
	register
	kern_return_t	result;

	if (target_map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);

	/*
	 * Currently no external pager support or
	 * Mach COPY technology support.
	 */

	if (memory_object != MEMORY_OBJECT_NULL || copy == TRUE) 
		return KERN_INVALID_ARGUMENT;

	*address = trunc_page(*address);
	size = round_page(size);

	if (memory_object == MEMORY_OBJECT_NULL) {
		object = VM_OBJECT_NULL;
		offset =0;
		copy = FALSE;
	}
#ifdef	notyet

	if (memory_object == MEMORY_OBJECT_NULL) {
		object = VM_OBJECT_NULL;
		offset = 0;
		copy = FALSE;
	} else if ((object = vm_object_enter(memory_object, size, FALSE))
			== VM_OBJECT_NULL)
		return(KERN_INVALID_ARGUMENT);

	/*
	 *	Perform the copy if requested
	 */

	if (copy) {
		vm_object_t	new_object;
		vm_offset_t	new_offset;

		result = vm_object_copy_strategically(object, offset, size,
				&new_object, &new_offset,
				&copy);

		/*
		 *	Throw away the reference to the
		 *	original object, as it won't be mapped.
		 */

		vm_object_deallocate(object);

		if (result != KERN_SUCCESS)
			return(result);

		object = new_object;
		offset = new_offset;
	}

#endif	/* notyet */

	if ((result = vm_map_enter(target_map,
				address, size, mask, anywhere,
				object, offset,
				copy,
				cur_protection, max_protection, inheritance
				)) != KERN_SUCCESS)
		if (object != VM_OBJECT_NULL) vm_object_deallocate(object);

	return(result);
}

kern_return_t vm_allocate_with_pager(map, addr, size, find_space, pager,
		pager_offset)
	register vm_map_t	map;
	register vm_offset_t	*addr;
	register vm_size_t	size;
	boolean_t		find_space;
	memory_object_t		pager;
	vm_offset_t		pager_offset;
{
	uprintf("vm_allocate_with_pager: use vm_map_instead!\n");

	return KERN_INVALID_ARGUMENT;

#ifdef	notyet

	return ((pager == MEMORY_OBJECT_NULL) ?
		KERN_INVALID_ARGUMENT :
		vm_map(map, addr, size, 0, find_space,
		pager, pager_offset, FALSE,
		VM_PROT_ALL, VM_PROT_ALL, VM_INHERIT_COPY)
		);
#endif	/* notyet */
}

/*
 * Handle machine-specific attributes for a mapping, such
 * as cachability, migrability, etc.
 */
kern_return_t vm_machine_attribute(map, address, size, attribute, value)
	vm_map_t	map;
	vm_address_t	address;
	vm_size_t	size;
	vm_machine_attribute_t	attribute;
	vm_machine_attribute_val_t* value;		/* IN/OUT */
{
	extern kern_return_t	vm_map_machine_attribute();

	if (map == VM_MAP_NULL)
		return(KERN_INVALID_ARGUMENT);

	return vm_map_machine_attribute(map, address, size, attribute, value);
}

void vm_delete_wirings(map)
        vm_map_t        map;
{

	pmap_coproc_exit_notify(map->vm_pmap);
        lw_delete_wirings(map);
}

