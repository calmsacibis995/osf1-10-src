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
static char     *sccsid = "@(#)$RCSfile: vm_object.c,v $ $Revision: 4.2.13.2 $ (DEC) $Date: 1993/09/22 14:20:06 $";
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	vm/vm_object.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory object module.
 */

/*
 * vm_object.c
 *
 *      Modification History:
 *
 *07-Nov-91	Larry Woodman
 *		Added object resident count to handle swapping out objects.
 *
 */
#include <sys/unix_defs.h>
#include <mach/kern_return.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <mach/vm_param.h>
#include <vm/vm_object.h>
#include <kern/assert.h>
#include <kern/zalloc.h>



/*
 *	All wired-down kernel memory belongs to a single virtual
 *	memory object (kernel_object) to avoid wasting data structures.
 */

struct vm_object kernel_object_store;
vm_object_t kernel_object = &kernel_object_store;

/*
 *	All pageable kernel memory that transition from
 *	a non-pageable object to a pageable one because
 *	of a unwiring belongs to a single object ("pkernel_object").
 */

struct vm_object pkernel_object_store;
vm_object_t pkernel_object = &pkernel_object_store;

/*
 *	A kernel object for funny mappings like kernel memory
 *	allocators and the traditional buffer cache.
 */

struct vm_object nkernel_object_store;
vm_object_t nkernel_object = &nkernel_object_store;

/*
 *	The submap object is used as a placeholder for vm_map_submap
 *	operations.  The object is declared in vm_map.c because it
 *	is exported by the vm_map module.  The storage is declared
 *	here because it must be initialized here.
 */

struct vm_object vm_submap_object_store;
vm_object_t vm_submap_object = &vm_submap_object_store;


/*
 *	Virtual memory objects are initialized from
 *	a template (see vm_object_allocate).
 *
 *	When adding a new field to the virtual memory
 *	object structure, be sure to add initialization
 *	(see vm_object_init).
 */

struct vm_object	vm_object_template;

extern struct vm_object_config null_object_conf;
extern struct vm_object_config vp_object_conf;
extern struct vm_object_config vpmap_object_conf;
extern struct vm_object_config anon_object_conf;
extern struct vm_object_config swap_object_conf;
extern struct vm_object_config dev_object_conf;
extern struct vm_object_config kernel_object_conf;
extern struct vm_object_config pkernel_object_conf;
extern struct vm_object_config shm_object_conf;
extern struct vm_object_config seg_object_conf;

struct vm_object_config *vm_object_config[] = {
	&null_object_conf,
	&vp_object_conf,
	&vpmap_object_conf,
	&anon_object_conf,
	&swap_object_conf,
	&dev_object_conf,
	&kernel_object_conf,
	&pkernel_object_conf,
	&shm_object_conf,
	&seg_object_conf,
};

zone_t vm_object_zone;


/*
 *	vm_object_init:
 *
 *	Initialize the VM objects module.
 */

void 
vm_object_init()
{
	register int i, last;
	register vm_size_t maxsize;
	register struct vm_object_config *cp;

	for(maxsize = 0, last = OT_LAST, i = 0, cp = vm_object_config[0]; 
		i < last; i++, cp = vm_object_config[i]) 
		if (cp->oc_osize > maxsize) maxsize = cp->oc_osize;
		else continue;

	vm_object_zone = zinit((vm_size_t) maxsize,
				round_page(512*1024),
				round_page(12*1024),
				"objects");

	/*
	 *	Fill in a template object, for quick initialization
	 */

	vm_object_template.ob_ref_count = 1;
	vm_object_template.ob_res_count = 1;
	vm_object_template.ob_size = 0;
	vm_object_template.ob_resident_pages = 0;
	vm_object_template.ob_memq = (vm_page_t) 0;

	/*
	 *	Initialize the kernel object for non-paged memory
	 */

	_vm_object_allocate(VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS,
		kernel_object, OT_KERNEL);

	_vm_object_allocate(VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS,
		pkernel_object, OT_PKERNEL);

	_vm_object_allocate(VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS,
		nkernel_object, OT_NULL);

	/*
	 *	Initialize the "submap object".  Make it as large as the
	 *	kernel object so that no limit is imposed on submap sizes.
	 */

	_vm_object_allocate(VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS,
			vm_submap_object, OT_KERNEL);
}

/*
 * Free the object.  This eliminates exporting
 * the object allocation mechanism which is
 * currently very simple.  The object is assumed
 * lock and free of any other associated information.
 */

void
vm_object_free(vm_object_t obj)
{
	if (obj->ob_ref_count > 1) panic("vm_object_free: ref count > 1");
	else if(obj->ob_res_count > 1) panic("vm_object_free: res count > 1");
	else ZFREE(vm_object_zone, obj);
}


/*
 * Null object operation handles.  The action
 * performed assumes failures for some handles.
 *
 *	vm_ops_##name##def
 */

int
vm_ops_reference_def(register vm_object_t obj)
{
	vm_object_lock(obj);
	obj->ob_ref_count++;
	obj->ob_res_count++;
	vm_object_unlock(obj);

}

/*
 * A deallocate and reference count of zero assumes
 * that the object doesn't require any special action
 * other than freeing the object structure.
 */

int
vm_ops_deallocate_def(register vm_object_t obj)
{
	vm_object_lock(obj);
	if (obj->ob_ref_count == 1) {
		if (vm_object_config[obj->ob_type]->oc_terminate) 
			(*vm_object_config[obj->ob_type]->oc_terminate)(obj); 
		else {
			vm_object_unlock(obj);
			vm_object_free(obj);
		}
	}
	else {
		obj->ob_ref_count--;	
		obj->ob_res_count--;
		vm_object_unlock(obj);
	}
	return;
}

int 
vm_ops_pagein_def(vm_object_t obj, 
	vm_offset_t offset, 
	vm_size_t size,
	int *protobug, 
	vm_offset_t addr, 
	vm_size_t psize, 
	struct vm_page **page,
	vm_prot_t *fault_type) 
{
	struct vm_map_entry *ep = (struct vm_map_entry *) protobug;

	return KERN_FAILURE;
}

/*
 * Scan down pages and unload.
 * It is assumed the object is locked from
 * [offset, offset + size )
 * pageout could grab a page we're looking at and
 * delete it.  Hence we can have cluster locks but also
 * require a lock to protect the object page list.
 * They can't be the same or we deadlock.  Also the cluster
 * lock must always be taken before the general object lock.
 */

int
vm_ops_pmap_protect_def(vm_object_t obj, 
	register vm_offset_t offset, 
	register vm_size_t size, 
	pmap_t pmap, 
	vm_offset_t vaddr,
	vm_prot_t prot)
{
	register vm_page_t pp;
	register vm_offset_t voffset;

	 pp = obj->ob_memq;
	if (pp) do {
		if (pp->pg_offset >= offset && pp->pg_offset < (offset + size)){
			if (pmap) {
				voffset = (pp->pg_offset - offset);
				pmap_protect(pmap, 
					vaddr + voffset,
					vaddr + voffset + PAGE_SIZE,
					prot);
			}
			else pmap_page_protect(page_to_phys(pp), prot);
		}
		pp = pp->pg_onext;
	} while (pp != obj->ob_memq);
}

int
vm_ops_swap_def(vm_object_t obj,
	int *pages)
{
	return KERN_FAILURE;
}

int
vm_ops_pageout_def(vm_object_t object, 
	vm_offset_t offset,
	vm_size_t size)
{
	return KERN_FAILURE;
}

/*
 *	vm_object_allocate:
 *
 *	Returns a new object with the given size.
 */

kern_return_t 
vm_object_allocate(type, size, private, robj)
	char type; 
	vm_size_t size; 
	caddr_t private;
	vm_object_t *robj;
{
	register vm_object_t obj;

	 ZALLOC(vm_object_zone, obj, vm_object_t);
	_vm_object_allocate(size, obj, type);
	if (vm_object_config[type]->oc_allocate) {
		kern_return_t ret;

		ret = (*vm_object_config[type]->oc_allocate)(obj, private);
		if (ret != KERN_SUCCESS) {
			vm_object_free(obj);
			return ret;
		}
	}
	*robj  = obj;
	return KERN_SUCCESS;
}

_vm_object_allocate(size, object, type)
	vm_size_t size;
	vm_object_t object;
	char type;
{
	*object = vm_object_template;
	vm_object_lock_init(object);
	object->ob_size = size;
	object->ob_type = type;
	object->ob_ops = vm_object_config[type]->oc_ops;
	if (vm_object_config[type]->oc_osize > sizeof (struct vm_object)) 
		bzero(((vm_offset_t) object) + sizeof (struct vm_object),
			vm_object_config[type]->oc_osize - 
			sizeof (struct vm_object));
}

void
vm_object_swapoff(register vm_object_t obj)
{
	while (obj->ob_flags & OB_SWAP) 
#if	UNIX_LOCKS
		mpsleep(vm_object_sleep_addr(obj,SWAP),
			PRIBIO, "SWAPIO", 0, simple_lock_addr(obj->ob_lock), 
			MS_LOCK_SIMPLE|MS_LOCK_ON_ERROR);
#else
		mpsleep(vm_object_sleep_addr(obj,SWAP),
			PRIBIO, "SWAPIO", 0, (simple_lock_t) 0, 0);
#endif	/* UNIX_LOCKS */
	obj->ob_flags ^= OB_SWAPON;
}

/*
 * vm_object_swapout decrements the ob_res_count and if it goes
 * to zero indicating that there are no more reasons not to swap
 * the object out, calls the object swapout operation handler.
 */

kern_return_t
vm_object_swapout(vm_object_t obj,
	int *pagecount)
{
	kern_return_t error;

	error = KERN_SUCCESS;
        vm_object_lock(obj);
	if (obj->ob_flags & OB_SWAPON) {
        	if (--obj->ob_res_count <= 0) {
                	obj->ob_flags |= OB_SWAP;
                	vm_object_unlock(obj);
                	error = OOP_SWAP(obj, pagecount);
                	vm_object_lock(obj);
                	obj->ob_flags ^= OB_SWAP;
                	vm_object_wakeup(obj,SWAP);
			if (error != KERN_SUCCESS) obj->ob_res_count++;
        	}
	}
	vm_object_unlock(obj);
        return error;
}

/*
 * vm_object_swapin increnents the ob_res_count indicating
 * that there is another reason not to swap out the object.
 */

void
vm_object_swapin(vm_object_t obj)
{
        vm_object_lock(obj);
	if (obj->ob_flags & OB_SWAPON)
        	obj->ob_res_count++;
        vm_object_unlock(obj);
}


/*
 * Null object configuration.
 */

null_object_bad()
{
	panic("null_object_bad: bad operation for object");
}

/*
 * Null object no-operation.
 */

null_object_pagesteal()
{
	return KERN_FAILURE;
}

struct vm_object_ops null_object_oops = {
	&null_object_bad,		/* klock try */
	&null_object_bad,		/* unlock */
	&vm_ops_reference_def,		/* reference */
	&vm_ops_deallocate_def,		/* deallocate */
	&null_object_bad,		/* pagein */
	&null_object_bad,		/* pageout */
	&null_object_bad,		/* swap */
	&null_object_bad,		/* control */
	&null_object_bad,		/* pagectl */
	&null_object_pagesteal,		/* pagesteal */
};

extern struct vm_map_entry_ops k_mape_ops_invalid;

struct vm_object_config null_object_conf = {
	(int (*)()) 0,
	(int (*)()) 0,
	sizeof (struct vm_object),
	&null_object_oops,
	&k_mape_ops_invalid,
};


