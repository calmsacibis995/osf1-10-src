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
 *	@(#)$RCSfile: ldr_main_types.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:09:08 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	ldr_main_types.h
 *	loader defined types visible to loader clients
 *	NOTE: other include files needed in order to use this one :
 *		<sys/types.h> <loader.h>
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_MAIN_TYPES
#define _H_LDR_MAIN_TYPES

#ifndef __
#ifdef _NO_PROTO
#define __(args)	()
#else /* _NO_PROTO */
#define __(args)	args
#endif /* _NO_PROTO */
#endif /* __ */

#ifdef __STDC__
typedef	void	*univ_t;
#else /* __STDC__ */
typedef	char	*univ_t;
#define	const
#endif /* __STDC__ */

typedef int	ldr_file_t;		/* loader file handle */

#define	LDR_FILE_NONE	((ldr_file_t)(-1))

typedef	univ_t	ldr_module_handle;	/* opaque type for fmt-dep handle */

/* The loader context is the anchor for all the loaded modules in a
 * process or other related group.  Each process has at least one
 * static loader context describing all the modules loaded into that
 * process.  A process may choose to keep other independnent contexts,
 * for example to manage the loading of modules into the kernel.
 */

typedef univ_t		ldr_context_t;

/* Procedure type declarations */

/* The alloc_abs_region_p procedure is called by the format-dependent
 * map_region routine to decide what base address to use in mapping an
 * absolute region.  Arguments are the virtual address at which the
 * region is relocated to run, the region size, and the protection for
 * the region.  On return, baseaddr is set to the best-guess starting
 * address at which the region is to be mapped; if baseaddr == vaddr
 * on return, the region is to be mapped using the LDR_MAP_FIXED flag,
 * otherwise, the baseaddr is just a hint to ldr_mmap.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

typedef int (*alloc_abs_region_p) __((univ_t vaddr, size_t size,
				      ldr_prot_t prot, univ_t *baseaddr));

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
 */

typedef int (*alloc_rel_region_p) __((size_t size, ldr_prot_t prot,
				      univ_t *vaddr, univ_t *baseaddr));

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
 */

typedef int (*dealloc_region_p) __((univ_t vaddr, univ_t mapaddr, size_t size));

/* Structure holding the allocation and deallocation procedures */

typedef struct ldr_region_allocs {
	alloc_abs_region_p	lra_abs_alloc;	/* absolute allocator */
	alloc_rel_region_p	lra_rel_alloc;	/* relocatable allocator */
	dealloc_region_p	lra_dealloc; /* deallocator */
} ldr_region_allocs;

#endif /* _H_LDR_MAIN_TYPES */
