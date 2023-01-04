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
/*	
 *	@(#)$RCSfile: ldr_preload.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:53 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_preload.h
 * Routines for preloading libraries
 *
 * This file contains the visible data structure and routine
 * declarations for pre-loading libraries.  The procedure
 * declarations for the format-dependent preload manager are
 * not in this file -- they are in preload_mgr.h .
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_LDR_PRELOAD
#define	_H_LDR_PRELOAD

/* The preload descriptor is located in the loader global data file.
 * It points to the remainder of the preload information.
 */

typedef struct ldr_preload_desc {
	int		pd_magic;	/* magic number */
	ldr_context	*pd_context;	/* context holding preloaded libs */
	void		*pd_baseaddr;	/* base virt. address of preload data */
	char		*pd_fname;	/* preload data file name */
	size_t		pd_size;	/* size of preload data in file */
	ldr_heap_t	pd_heap;	/* heap for allocations */
} ldr_preload_desc;

/* Entry points for preload file creation and management */

/* Initialize the specified preload descriptor.  Initially there is
 * no preload context, and so no base address or data file.
 * This routine is only to be called from ldr_global_file_init
 * during global data file creation.
 */

extern int
ldr_preload_init __((ldr_preload_desc *pr_ptr, ldr_heap_t heap));

/* Copy out the specified loader context to the preload file,
 * using the preload descriptor initialized by
 * the previous call to preload_init.  First, get a unique preload data
 * file name and create the preload data file.  Next, copy out the
 * context and its consitutent module records (and their contents)
 * to the heap in the global file.  Then walk the copied context
 * and modules, copying the data from its current location (mapped
 * into the process' VA space) into the preload data file, and fixing
 * up the virtual and mapped addresses in the region records.
 * Finally, close the preload data file.
 * Returns LDR_SUCCESS on success, negative error status on error.
 * If we fail, we leave the preload data file in an indeterminate state.
 * This routine is only to be called from ldr_global_file_init
 * during global data file creation.
 */

extern int
ldr_preload_copyout __((ldr_preload_desc *pr_ptr, ldr_context *context));

/* Remove the current preload descriptor, so that future installs or
 * loads will not use the preload cache.  This is intended to be used
 * only during removal of the loader global file, as part of a new
 * global library install.
 */

extern int
ldr_preload_remove __((ldr_preload_desc *desc));

/* Inherit the specified preload descriptor.  Mostly just
 * includes error checking; also sets up the static data
 * used by the format-dependent manager.
 */

extern int
ldr_preload_inherit __((ldr_preload_desc *desc));

/* Region allocation routines for preload context */

/* The alloc_abs_region_p procedure is called by the format-dependent
 * map_region routine to decide what base address to use in mapping an
 * absolute region.  Arguments are the virtual address at which the
 * region is relocated to run, the region size, and the protection for
 * the region.  On return, baseaddr is set to the best-guess starting
 * address at which the region is to be mapped; if baseaddr == vaddr
 * on return, the region is to be mapped using the LDR_MAP_FIXED flag,
 * otherwise, the baseaddr is just a hint to ldr_mmap.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 *
 * The absolute allocator just returns an error for pre-loading -- we
 * can't preload a non-relocatable module.
 */

extern int
preload_alloc_abs __((univ_t vaddr, size_t size, ldr_prot_t prot, univ_t *baseaddr));

/* The alloc_rel_region_p procedure is called by the format-dependent
 * map_region routine to decide what virtual address a relocatable
 * region is to be relocated to run at, and what base address to use
 * in mapping the region.  Arguments are the region size and the
 * protection for the region.  On return, vaddr is set to the address
 * to which the region is to be relocated, and baseaddr is set to the
 * best-guess starting address at which the region is to be mapped.
 * If vaddr == NULL on return, the region is to be relocated to run
 * at whatever address it ends up being mapped at.  In either case,
 * baseaddr is to be used as a hint to ldr_mmap.  Returns LDR_SUCCESS
 * on success, negative error status on error.
 *
 * The preload relative region allocator just allocates the next chunk
 * of space from the space reserved for the preload file and returns it
 * as the virtual address.  It returns the base of the standard mmap
 * region as the base address at which the region is to be mapped.
 * Note that it must be possible to call this procedure before
 * the loader global file is created.
 */

extern int
preload_alloc_rel __((size_t size, ldr_prot_t prot, univ_t *vaddr, univ_t *baseaddr));

/* The dealloc_region_p procedure is the inverse to the
 * alloc_xxx_region_p procedures; it is called by the format-dependent
 * unmap_region routine to deallocate any storage allocated by either
 * of the alloc_xxx_region_p procedures.  The mapaddr argument is the
 * actual address to which the region was mapped.  The vaddr is the
 * virtual address to which the region was relocated to run (equal to
 * the vaddr passed in to alloc_abs_region_p, or the vaddr returned
 * from alloc_rel_region_p if non-NULL, or equal to the mapaddr if
 * alloc_rel_region_p returned a NULL vaddr).  The size is the
 * argument passed to alloc_region_p.  Returns LDR_SUCCESS on success
 * or negative error status on error.
 *
 * It doesn't have to do anything for preloaded libraries.
 */

extern int
preload_dealloc __((univ_t vaddr, univ_t mapaddr, size_t size));

#endif	/* _H_LDR_PRELOAD */
