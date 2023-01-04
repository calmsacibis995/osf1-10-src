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
static char	sccsid[] = "@(#)$RCSfile: kernel_region_alloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:20 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * This file contains the routines for allocating and deallocating
 * virtual address space for regions loaded into the kernel.  
 * These routines are potentially machine-dependent, so they  are in a
 * separate file to allow them to be overridden by a machine-dependent
 * version.
 *
 * The key concept to understand about kernel loading is that when
 * regions are to be loaded into the kernel, the loader calls the
 * routines contained herein to allocate space in the kernel and the
 * loader then maps the regions into the current process's address
 * space, yet relocates them with respect to where they will live in
 * the kernel's address space.  Eventually, the entity that called the
 * loader is expected to copy the region, now loaded and relocated in
 * the current process's address space, into the kernel.
 */

#include <sys/types.h>
#include <loader.h>
#include <mach.h>

#include "ldr_types.h"
#include "ldr_errno.h"
#include "ldr_sys_int.h"

#include "kls_vm.h"
#include "kls_subr.h"

static int get_local_mapaddr(univ_t vaddr, univ_t *baseaddrp);

/*
 * alloc_abs_kernel_region() - region allocator for absolute regions
 *                             to be loaded into the kernel
 *
 * This is the absolute region allocator used in initializing the
 * kernel loader context.  The absolute region allocator procedure is
 * called by the format-dependent map_region routine to determine the
 * base address at which the region is to be mapped.  Arguments are
 * the virtual address at which the region must run, region size and
 * protection.  On return, baseaddr is set to the best-guess starting
 * address at which the region is to be mapped.  Returns LDR_SUCCESS
 * on success, negative error status on error. 
 *
 * For loading an absolute region into the kernel, vaddr and size
 * specify the area within the kernel's address space, at which we
 * must allocate space for the region to live.  If that area is
 * unavailable, we must return a failure.  Note that in the call to
 * allocate memory in the kernel's address space, the anywhere
 * parameter is FALSE.  Lastly, we must find some area within the
 * current process's address space to locally map the region.  See
 * get_local_mapaddr() below for details.
 *
 * In the current implementation we always allocate wired memory in
 * the kernel's addres space.
 */
int
alloc_abs_kernel_region(univ_t vaddr, size_t size, ldr_prot_t ldr_prot,
			univ_t *baseaddrp)
{
	vm_address_t	vm_address;
	vm_size_t	vm_size;
	vm_prot_t	vm_prot;
	int		rc;

	vm_address = (vm_address_t)vaddr;
	vm_size = (vm_size_t)size;
	vm_prot = convert_ldr_prot_to_vm_prot(ldr_prot);

	if (rc = kls_vm_allocate_wired(&vm_address, vm_size, vm_prot, FALSE))
		return(convert_kls_vm_status_to_ldr_status(rc));

	if ((rc = get_local_mapaddr(vaddr, baseaddrp)) != LDR_SUCCESS) {
		(void)kls_vm_deallocate(vm_address, vm_size);
		return(rc);
	}
	return(LDR_SUCCESS);
}


/*
 * alloc_rel_kernel_region() - region allocator for relocatable
 *                             regions to be loaded into the kernel's
 *                             address space
 *
 * This is the relocatable region allocator used in initializing the
 * kernel loader context.  The relocatable region allocator procedure
 * is called by the format-dependent map_region routine to determine
 * the virtual address at which the region is to be run and the base
 * address at which the region is to be mapped.  Arguments are the
 * region size and  protection.  On return, *vaddrp is set to the
 * address at which the code is to be relocated to run, and *baseaddrp
 * is set to the  best-guess starting address at which the region is
 * to be mapped.  Returns LDR_SUCCESS on success, negative error status
 * on error. 
 * 
 * For loading a relocatable region into the kernel, any area of size
 * size in the kernel's address space is acceptable.  Note that in the
 * call to allocate memory in the kernel's address space, the anywhere
 * parameter is TRUE.  Lastly, we must find some area within the
 * current process's address space to locally map the region.  See
 * get_local_mapaddr() below for details.
 *
 * In the current implementation we always allocate wired memory in
 * the kernel's addres space.
 */
int
alloc_rel_kernel_region(size_t size, ldr_prot_t ldr_prot, univ_t *vaddrp,
			univ_t *baseaddrp)
{
	vm_address_t	vm_address;
	vm_size_t	vm_size;
	vm_prot_t	vm_prot;
	univ_t		vaddr;
	int		rc;

	vm_size = (vm_size_t)size;
	vm_prot = convert_ldr_prot_to_vm_prot(ldr_prot);

	if (rc = kls_vm_allocate_wired(&vm_address, vm_size, vm_prot, TRUE))
		return(convert_kls_vm_status_to_ldr_status(rc));
	vaddr = (univ_t)vm_address;

	if ((rc = get_local_mapaddr(vaddr, baseaddrp)) != LDR_SUCCESS) {
		(void)kls_vm_deallocate(vm_address, vm_size);
		return(rc);
	}

	*vaddrp = vaddr;
	return(LDR_SUCCESS);
}


/*
 * dealloc_kernel_region() - region deallocator for regions loaded
 *                           into the kernel's address space
 *
 * This is the deallocator used in initializing the kernel loader
 * context.  The region deallocator procedure is the inverse to the
 * region allocator procedure; it is called by the format-dependent
 * unmap_region routine to deallocate any storage allocated by the
 * either alloc_abs_kernel_region() or alloc_rel_kernel_region().
 * mapaddr is the actual address at which the region was mapped into
 * the current process's address space.   vaddr is the address at
 * which the region was loaded into the kernel's address space.  It is
 * assumed that the format dependent unmap_region routine has handled
 * or will handle the unmapping of the region (i.e. at mapaddr) mapped
 * into the current process's address space.  Therefore all we need to
 * do is unmap the region in the kernel's adress space.  Returns
 * LDR_SUCCESS on success or negative error status on error. 
 */
int
dealloc_kernel_region(univ_t vaddr, univ_t mapaddr, size_t size)
{
	vm_address_t	vm_address;
	vm_size_t	vm_size;
	int		rc;

	vm_address = (vm_address_t)vaddr;
	vm_size = (vm_size_t)size;

	if (rc = kls_vm_deallocate(vm_address, vm_size))
		return(convert_kls_vm_status_to_ldr_status(rc));
	return(LDR_SUCCESS);
}

/*
 * get_local_mapaddr() - get an address within this process (i.e. the
 *                       kernel load server) at which to map this region
 *
 * From a kernel loading perspective, it really doesn't matter where
 * we map the region.  After all, the region isn't going to finally
 * live in this process (i.e. the kernel load server).  The region
 * will finally live in the kernel's address space.  However, it may
 * make a difference to this process, from the perspective of how the
 * application (i.e. the kernel load server) want's to use its address
 * space.  Therefore we simply say that this region should be mapped
 * somewhere in the area reserved for mmap'ed file data (i.e.
 * AC_MMAP_DATA).  It should only be necessary to call
 * getaddressconf(2) (i.e. ldr_getaddressconf() once, to determine the
 * location of that area.
 *
 * There is one potential gotcha to worry about.  Under some user
 * process and kernel address space layouts, it will be possible for
 * vaddr to equal baseaddr.  If vaddr is equal to baseaddr, then the
 * format dependent map_region routine will call ldr_mmap() with
 * LDR_MAP_FIXED.  That would be an incorrect action to take for
 * several reasons.
 *
 *      o the call to ldr_mmap() would fail if something is already
 *        mapped at baseaddr
 *      o for kernel loading, the baseaddr selected really doesn't
 *        matter, subject to the area constraints mentioned in the
 *        previous paragraph
 *
 * The intent with our selection of baseaddr is to specify a location,
 * close (not exactly) to which the region should be mapped into this
 * process.  Therefore if the selected baseaddr happens to coincide
 * with vaddr, a very, very unlikely event, we simply bump baseaddr to
 * the next page.
 */
static univ_t local_mapaddr;
static int have_local_mapaddr = FALSE;

static int
get_local_mapaddr(univ_t vaddr, univ_t *baseaddrp)
{
	struct addressconf *addr_conf;
	univ_t              baseaddr;
	int                 rc;

	if (have_local_mapaddr == FALSE) {
		if ((rc = ldr_getaddressconf(&addr_conf)) != LDR_SUCCESS)
			return(rc);
		local_mapaddr = (univ_t)addr_conf[AC_MMAP_DATA].ac_base; 
		have_local_mapaddr = TRUE;
	}

	baseaddr = local_mapaddr;

	if (vaddr == baseaddr)
		baseaddr = (univ_t)((char *)baseaddr + ldr_getpagesize());
	
	*baseaddrp = baseaddr;
	return(LDR_SUCCESS);
}
