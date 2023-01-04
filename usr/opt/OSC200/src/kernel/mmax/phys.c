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
static char	*sccsid = "@(#)$RCSfile: phys.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:26 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

/*
 *	File:	mmax/phys.c
 *	Original Author:	Jim Van Sciver [Encore]
 *	Modifications:		David L. Black [CMU]
 *
 *	Copyright (C) 1986, Encore Computer, Carnegie-Mellon University.
 *
 *	Physical memory routines.
 *
 */

#include <mach/boolean.h>
#include <sys/errno.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <sys/types.h>
#include <vm/vm_fault.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>

#include <mmax/cpu.h>
#include <mmax/pte.h>
#include <mmax/pmap.h>

#include <mmax/mtpr.h>

#define NS_PAGES  PAGE_SIZE/NS32K_PGBYTES


/*
 *	Minimum size of copyin or copyout requests to go through
 *	copy_{to,from}_phys mechanism.
 */
#define	MIN_MAP_COPYIN_SIZE	1500
#define	MIN_MAP_COPYOUT_SIZE	1750
#define MIN_PHYS_COPYOUT_SIZE	400

/*
 *	pmap_zero_page zeros the specified (machine independent) page.
 *	It maps the given physical page to a pre-allocated kernel virtual
 *	address and clears the page using the virtual address.
 */
pmap_zero_page(phys)
    vm_offset_t	phys;
{
    int		s;
    register int proc_num;
    register pt_entry_t	*pte, *end_pte;
    register template_t	pte_template;
    register vm_offset_t va;

    s = splvm();

    proc_num = cpu_number();
    pte = phys_map_pte1   + proc_num*NS_PAGES;
    va  = phys_map_vaddr1 + proc_num*PAGE_SIZE;

    end_pte = pte + NS_PAGES;
    pte_template.bits = PHYSTOPTE(phys)|(NS32K_KW << PG_PROTOFF)| PG_R |
	PG_M | PG_V ;
    TBIA_K;

    for (; pte < end_pte ; pte++, pte_template.bits += NS32K_PGBYTES) {
	*pte = pte_template.pte;
    }

    blkclr(va, PAGE_SIZE);

    splx(s);
}


/*
 *	pmap_copy_page copies the specified (machine independent) pages.
 *	It maps the given physical pages to pre-allocated kernel virtual
 *	addresses and copies the pages using these virtual addresses.
 */
pmap_copy_page(src, dst)
    vm_offset_t	src, dst;
{
    int		s;
    register int proc_num;
    register pt_entry_t	*pte1, *pte2, *pte2_end;
    template_t  pte1_template, pte2_template;
    vm_offset_t	va1, va2;
    pt_entry_t	*old_start_pte2, old_pte2;

    s = splvm();

    proc_num = cpu_number();

    pte1 = phys_map_pte1   + proc_num*NS_PAGES;
    va1  = phys_map_vaddr1 + proc_num*PAGE_SIZE;
    pte2 = phys_map_pte2   + proc_num*NS_PAGES;
    va2  = phys_map_vaddr2 + proc_num*PAGE_SIZE;

    pte2_end = pte2 + NS_PAGES;
    pte1_template.bits = PHYSTOPTE(src)|(NS32K_KR << PG_PROTOFF) | PG_R |
	PG_V ;
    pte2_template.bits = PHYSTOPTE(dst)|(NS32K_KW << PG_PROTOFF) | PG_R |
	PG_M | PG_V ;
    TBIA_K;

    /*
     *	If pte2 ptes are in use, save the first one (from which the others
     *	can be regenerated.  This routine can't fault, but we'll mark them
     *	in use anyway for consistency.
     */
    if (phys_pte2_in_use[proc_num]) {
	old_start_pte2 = pte2;
	old_pte2 = *pte2;
    }
    else {
	old_start_pte2 = PT_ENTRY_NULL;
	phys_pte2_in_use[proc_num] = TRUE;
    }

    for (; pte2 < pte2_end; pte1++, pte2++,
	pte1_template.bits += NS32K_PGBYTES,
	pte2_template.bits += NS32K_PGBYTES) {

	*pte1 = pte1_template.pte;
	*pte2 = pte2_template.pte;
    }

    blkcpy(va1, va2, PAGE_SIZE);

    /*
     *	Now put back the saved pte2 ptes, or mark them not in use.
     */
    if (old_start_pte2 != PT_ENTRY_NULL) {
	pte2_template.pte = old_pte2;
	for (pte2 = old_start_pte2; pte2 < pte2_end; pte2++,
	     pte2_template.bits += NS32K_PGBYTES) {
		 *pte2 = pte2_template.pte;
	}
    }
    else {
	phys_pte2_in_use[proc_num] = FALSE;
    }

    splx(s);
}

/*
 *	copy_to_phys(src_addr_v, dst_addr_p, count)
 *
 *	Copy virtual memory to physical memory.  
 */

copy_to_phys(src_addr_v, dst_addr_p, count)
    vm_offset_t	src_addr_v;
    vm_offset_t	dst_addr_p;
    register int count;
{
    int		s;
    int		ns_pages, proc_num;
    register pt_entry_t	*pte, *pte_end;
    pt_entry_t	*pte_save;
    template_t	pte_template;
    register vm_offset_t src, dst;
    vm_offset_t	va, end, dst_offset;
    pt_entry_t	old_pte2, *old_start_pte2;

    /*
     *	Find this processor's reserved ptes and corresponding virtual address.
     */
    proc_num = cpu_number();
    pte_save = phys_map_pte2 + proc_num*NS_PAGES;
    va  = phys_map_vaddr2 + proc_num*PAGE_SIZE;

    /*
     *		Make working copies of src and dst addresses.  Calculate
     *		virtual copy end and physical page offset of copy start.
     */
    src = src_addr_v;
    end = src_addr_v + count;
    /*
     *		On multimax round to a physical page because it's faster
     *		that way, and 16Meg boundary doesn't exist.
     */
    dst = ns32k_trunc_page(dst_addr_p);
    dst_offset = dst_addr_p - dst;

    /*
     *		Save old ptes if needed.
     */
    if (phys_pte2_in_use[proc_num]) {
	old_start_pte2 = pte_save;
	old_pte2 = *old_start_pte2;
    }
    else {
	old_start_pte2 = PT_ENTRY_NULL;
	phys_pte2_in_use[proc_num] = TRUE;
    }

    while (src < end) {

	/*
	 *	Try to copy it all in one chunk.  Failing that, do
	 *	as much as we can map with one vm page.  dst_offset is
	 *	the page alignment offset of the destination.  (i.e. if
	 *	dst is page-aligned, dst_offset is 0).
	 */
	count = end - src;
	ns_pages = ns32k_btop(ns32k_round_page(count+dst_offset));
	if (ns_pages > NS_PAGES) {
	    count = PAGE_SIZE - dst_offset;
	    ns_pages = NS_PAGES;
	}

	/*
	 *	Set up the ptes to map dst.
	 */
	pte_end = pte_save + ns_pages;
	pte_template.bits = PHYSTOPTE(dst)|(NS32K_KW << PG_PROTOFF) | PG_R |
		PG_M | PG_V;
	TBIA_K;
	s = splvm();

	for (pte = pte_save; pte < pte_end; pte++,
	    pte_template.bits += NS32K_PGBYTES) {

	    *pte = pte_template.pte;
	}

	/*
	 *	Copy the data, then set up for the next time around.
	 */
	blkcpy(src, va + dst_offset, count);
	splx(s);

	src += count;
	dst += count + dst_offset;
	dst_offset = 0;
    }

    /*
     *	Restore the ptes if needed.
     */

    if (old_start_pte2 != PT_ENTRY_NULL) {
	pte_end = pte_save + NS_PAGES;
	pte_template.pte = old_pte2;
	for (pte = pte_save; pte < pte_end; pte++,
	     pte_template.bits += NS32K_PGBYTES) {
		 *pte = pte_template.pte;
	}
    }
    else {
	phys_pte2_in_use[proc_num] = FALSE;
    }
}

/*
 *	copy_from_phys(src_addr_p, dst_addr_v, count)
 *
 *	Copy physical memory to virtual memory.  The virtual memory
 *	must be resident (e.g. the buffer pool).
 *
 */

copy_from_phys(src_addr_p, dst_addr_v, count)
    vm_offset_t	src_addr_p;
    vm_offset_t	dst_addr_v;
    register int count;
{
    int		s;
    int		ns_pages, proc_num;
    register pt_entry_t	*pte, *pte_end;
    pt_entry_t	*pte_save;
    template_t	pte_template;
    register vm_offset_t src, dst;
    vm_offset_t	va, end, src_offset;
    pt_entry_t	old_pte2, *old_start_pte2;

    /*
     *	Find this processor's reserved ptes and corresponding virtual address.
     */
    proc_num = cpu_number();
    pte_save = phys_map_pte2 + proc_num*NS_PAGES;
    va  = phys_map_vaddr2 + proc_num*PAGE_SIZE;

    /*
     *		Make working copies of src and dst addresses.  Calculate
     *		virtual copy end and physical page offset of copy start.
     */
    src = ns32k_trunc_page(src_addr_p);
    src_offset = src_addr_p - src;
    dst = dst_addr_v;
    end = dst_addr_v + count;

    /*
     *		Save old ptes if needed.
     */
    if (phys_pte2_in_use[proc_num]) {
	old_start_pte2 = pte_save;
	old_pte2 = *old_start_pte2;
    }
    else {
	old_start_pte2 = PT_ENTRY_NULL;
	phys_pte2_in_use[proc_num] = TRUE;
    }

    while (dst < end) {

	/*
	 *	Try to copy it all in one chunk.  Failing that, do
	 *	as much as we can map with one vm page.  src_offset is
	 *	the page alignment offset of the source.  (i.e. if
	 *	src is page-aligned, src_offset is 0).
	 */
	count = end - dst;
	ns_pages = ns32k_btop(ns32k_round_page(count + src_offset));
	if (ns_pages > NS_PAGES) {
	    count = PAGE_SIZE - src_offset;
	    ns_pages = NS_PAGES;
	}

	/*
	 *	Set up the ptes to map dst.
	 */
	pte_end = pte_save + ns_pages;
	pte_template.bits = PHYSTOPTE(src)|(NS32K_KR << PG_PROTOFF) | PG_R |
		PG_V;
	TBIA_K;
	s = splvm();

	for (pte = pte_save; pte < pte_end; pte++,
	    pte_template.bits += NS32K_PGBYTES) {

	    *pte = pte_template.pte;
	}

	/*
	 *	Copy the data, then set up for the next time around.
	 */
	blkcpy(va + src_offset, dst, count);
	splx(s);

	dst += count;
	src += count + src_offset;
	src_offset = 0;
    }

    /*
     *	Restore the ptes if needed.
     */

    if (old_start_pte2 != PT_ENTRY_NULL) {
	pte_end = pte_save + NS_PAGES;
	pte_template.pte = old_pte2;
	for (pte = pte_save; pte < pte_end; pte++,
	     pte_template.bits += NS32K_PGBYTES) {
		 *pte = pte_template.pte;
	}
    }
    else {
	phys_pte2_in_use[proc_num] = FALSE;
    }
}

/*
 *	grab_phys_page: locate and wire down physical page for copyin.
 *
 *	When this returns successfully, the following are true:
 *
 *	1.  The page in result page is wired down.
 *	2.  That page's object has an extra reference with paging in
 *		progress (prevents vm_object_collapse from getting in
 *		and confusing things).
 *
 *	The caller must call release_phys_page to clean this up.
 */
kern_return_t
grab_phys_page(map, vaddr, fault_type, result_page)
vm_map_t	map;
vm_offset_t	vaddr;
vm_prot_t	fault_type;
vm_page_t	*result_page;
{
	kern_return_t		result;
	vm_map_version_t	version;	/* not needed here */
	vm_object_t		object;		/* top level object */
	vm_offset_t		offset;		/* offset in that object */
	vm_prot_t		prot;		/* for wired check */
	boolean_t		wired;		/* ditto */
	boolean_t		su;		/* not used here */
	vm_page_t		top_page;	/* keep vm_fault_page happy */
	register vm_page_t	m;		/* optimization */

        /*
         *      Find the backing store object and offset into it.
         */
RetryFault: ;

        if ((result = vm_map_lookup(&map, vaddr, fault_type, &version,
                        &object, &offset,
                        &prot, &wired, &su)) != KERN_SUCCESS) {
                return(result);
	}

	/*
	 *	If page is wired, upgrade access to current protection.
	 */
	if (wired)
		fault_type = prot;

	/*
	 *	Grab an object reference and indicate that we're doing
	 *	paging on the object.
	 */
        object->ref_count++;
        vm_object_paging_begin(object);

	/*
	 *	Call vm_fault_page to find the physical page
	 */
	result = vm_fault_page(object, offset, fault_type,
		FALSE, TRUE, &prot, result_page, &top_page);

	/*
	 *	Drop object reference if this failed.
	 */
	if (result != VM_FAULT_SUCCESS)
		vm_object_deallocate(object);

	/*
	 *	Now decipher what happened.
	 */
        switch (result) {
                case VM_FAULT_SUCCESS:
                        break;
                case VM_FAULT_RETRY:
                        goto RetryFault;
                case VM_FAULT_INTERRUPTED:
                        return(KERN_FAILURE);
                case VM_FAULT_MEMORY_SHORTAGE:
                        VM_WAIT;
                        goto RetryFault;
                case VM_FAULT_MEMORY_ERROR:
                        return(KERN_MEMORY_ERROR);
	}

	/*
	 *	Wire down the page.
	 */
	m = *result_page;
	vm_page_lock_queues();
	vm_page_wire(m);
	vm_page_unlock_queues();

	/*
	 *	Now tidy up the fault handler state.  On exit, retain
	 *	a reference to the object and a paging_in_progress count.
	 */
#ifdef notdef
	vm_map_verify_done(map, &version);
#endif
	PAGE_WAKEUP_DONE(m);
        if (top_page != VM_PAGE_NULL) {
		/*
		 *	result_page is not in top object.  Need to grab
		 *	reference on correct object, and clean up top object.
		 */
		m->object->ref_count++;
	        vm_object_unlock(m->object);
                vm_object_lock(object);
                VM_PAGE_FREE(top_page);
                vm_object_paging_end(object);
                vm_object_unlock(object);
		vm_object_deallocate(object);
        }
	else {
		/*
		 *	result_page is in top object, how convenient!
		 */
		vm_object_unlock(m->object);
	}

	return(KERN_SUCCESS);
}


/*
 *	release_phys_page: release physical page grabbed above.
 */
release_phys_page(m)
vm_page_t	m;
{
	register vm_object_t	object;
	/*
	 *	Lock the object while cleaning up the mess.
	 */
	object = m->object;
	vm_object_lock(object);

	/*
	 *	Must wait for page to be not busy; parallel fault on
	 *	on this page could be mucking with it.  The paging
	 *	in progress reference makes sure that m doesn't change
	 *	objects while we're not looking.
	 */
	while (m->busy) {
	    PAGE_ASSERT_WAIT(m, FALSE);
	    vm_object_unlock(object);
	    thread_block();
	    vm_object_lock(object);
	}

	/*
	 *	Unwire the page - should still be in same object because
	 *	paging in progress was set by grab_phys_page.
	 */
	vm_page_lock_queues();
	vm_page_unwire(m);
	vm_page_unlock_queues();
	PAGE_WAKEUP(m);

	/*
	 *	Drop the paging reference, the lock, and the object reference.
	 */
	vm_object_paging_end(object);
	vm_object_unlock(object);
	vm_object_deallocate(object);
}

/*
 *	copyin - copy a certain number of bytes from user space to kernel.
 */

int copyin(user_addr, kernel_addr, count)
caddr_t  kernel_addr;
register caddr_t user_addr;
register int count;
{
    register vm_map_t		mymap;
    vm_page_t			m;
    register vm_offset_t	base, offset;
    register int		cur_count;

    /*
     *		If this is a small operation, do it directly.
     */
    if (count < MIN_MAP_COPYIN_SIZE)
	return(simple_copyin(user_addr, kernel_addr, count));

    /*
     *	Loop over user pages because virtual->physical mapping is
     *	discontiguous at page boundaries.
     */
    mymap = current_thread()->task->map;
    base = (vm_offset_t)trunc_page(user_addr);
    offset = (vm_offset_t)user_addr - base;
    while (count > 0) {
	/*
	 *	Figure out how much we can do this time.
	 */
	if(count+offset > PAGE_SIZE)
	    cur_count = PAGE_SIZE - offset;
	else
	    cur_count = count;

	/*
	 *	If this chunk is below the cutoff, just call simple_copyin.
	 *	Else use copy_from_phys.
	 */
	if (cur_count < MIN_MAP_COPYIN_SIZE) {
	    if (simple_copyin(user_addr, kernel_addr, cur_count) != 0) {
		return(EFAULT);
	    }
	}
	else {
	    if (grab_phys_page(mymap, base, VM_PROT_READ, &m)
		!= KERN_SUCCESS) {
		    return(EFAULT);
	    }

	    copy_from_phys(m->phys_addr + offset, kernel_addr, cur_count);
	    release_phys_page(m);
	}

	/*
	 *	Set up for next time around.
	 */
	user_addr += cur_count;
	kernel_addr += cur_count;
	count -= cur_count;
	base += PAGE_SIZE;
	offset = 0;
    }

    return(0);
}

/*
 *	copyout - copy a certain number of bytes to user space from kernel.
 */

int copyout(kernel_addr, user_addr, count)
caddr_t  kernel_addr;
register caddr_t user_addr;
register int count;
{
    register vm_map_t	mymap;
    vm_offset_t	start_addr, end_addr;
    vm_offset_t	paddr, offset;
    register int	cur_count;

    /*
     *		If this is a small operation, don't bother with the
     *		map hairiness.
     */
    if (count < MIN_MAP_COPYOUT_SIZE)
	return(simple_copyout(kernel_addr, user_addr, count));

    /*
     *	Wire down target.  Checks for holes and access violations.
     */
    mymap = current_thread()->task->map;
    start_addr = trunc_page(user_addr);
    end_addr = round_page(user_addr + count);

    if (vm_map_pageable(mymap, start_addr, end_addr, VM_PROT_WRITE)
	!= KERN_SUCCESS)
		return (EFAULT);

    /*
     *	Loop over user pages because virtual->physical mapping is
     *	discontiguous at page boundaries.
     */
    while (count > 0) {
	/*
	 *	Figure out how much we can do this time.
	 */
	offset = (vm_offset_t)user_addr - (vm_offset_t)trunc_page(user_addr);
	if(count+offset > PAGE_SIZE)
	    cur_count = PAGE_SIZE - offset;
	else
	    cur_count = count;

	/*
	 *	If this chunk is below the cutoff, just call simple_copyout.
	 *	Else use copy_to_phys.
	 */
	if (cur_count < MIN_PHYS_COPYOUT_SIZE) {
	    if (simple_copyout(kernel_addr, user_addr, cur_count) != 0) {
		goto Error;
	    }
	}
	else {
	    paddr = pmap_resident_extract(mymap->pmap, user_addr);
	    if (paddr == 0) {
		goto Error;
	    }
	    copy_to_phys(kernel_addr, paddr, cur_count);
	    pmap_set_modify(paddr);
	}

	/*
	 *	Set up for next time around.
	 */
	user_addr += cur_count;
	kernel_addr += cur_count;
	count -= cur_count;
    }

    /*
     *	Make sure to unwire all the wired user pages before returning.
     */
    (void) vm_map_pageable(mymap, start_addr, end_addr, VM_PROT_NONE);
    return(0);

Error:
    (void) vm_map_pageable(mymap, start_addr, end_addr, VM_PROT_NONE);
    return(EFAULT);
}
