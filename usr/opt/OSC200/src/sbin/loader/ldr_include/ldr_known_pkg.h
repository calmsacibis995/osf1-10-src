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
 *	@(#)$RCSfile: ldr_known_pkg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:51 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_known_pkg.h
 * Definitions for known package tables
 *
 * The known package tables maintain the mappings from package name
 * to module name or loaded module record.  They are used by the
 * symbol resolution algorithm when resolving an unresolved symbol,
 * to map from package name to a module to be loaded.  All known package
 * tables have a common format; they consist of an array of known package
 * records, pointed to by the module record.  Each known package record
 * points back to the module.  The known package records are hashed by
 * package name.
 *    
 * There are three known package tables:
 * - The global known package table is maintained in a mapped file in
 *   a standard location in the file system.  It is read-only to all
 *   processes, once created.  The same file contains the pre-loaded
 *   libraries.
 * - The private known package table is maintained in a keep-on-exec
 *   mapped region of address space.  This region is mapped private (copy-
 *   on-write).
 * - The loaded package table is pointed to by the context.
 *   It is modified by loading or unloading modules.
 *
 * This file depends on: loader.h ldr_types.h 
 *			 ldr_hash.h chain_hash.h open_hash.h
 * 			 ldr_package.h standards.h
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_KNOWN_PKG
#define _H_LDR_KNOWN_PKG


/* The known package table record contains a pointer to the package
 * record for a known package, and a pointer to module record for the
 * module exporting the package.  The known package table itself is a
 * chained hash table of loaded package table records, hashed by package name.
 *
 * All the kpt records for a given module are kept in an array pointed
 * to by the module record, so that they can easily be found when the
 * module is removed.
 * 
 * Note that this structure corresponds to a chain_hash_entry structure
 * in chain_hash.h.
 */

typedef struct ldr_kpt_rec {

/* Note: next two fields must be first in structure so chain_hash routines work */

	struct	ldr_kpt_rec	*lkp_next; /* hash chain link */
	char			*lkp_name; /* package name */

	struct	ldr_module_rec	*lkp_module; /* module -- note circular reference */
	ldr_package_rec		*lkp_package; /* package record for this pkg */
} ldr_kpt_rec;


/* The known package table itself */

typedef chain_hashtab_t	ldr_kpt;


/* The global and private known package tables are accessed through mapped
 * regions of memory -- the global table is mapped from a file, the private
 * table is anonymous memory mapped keep-on-exec.  The mapped regions begin
 * with a standard header that contains:
 *  - The heap pointer (for future allocations within the region)
 *  - The head of the list of known module records in the region.  Note
 *    that the module records are not complete; the only valid fields are
 *    the module name, list pointers, export package table, and known
 *    package table pointers.
 *  - The pointer to the known package table in the region.
 * It also contains a magic number, for use in verifying that the
 * mapped region contains valid data.
 */

typedef struct ldr_kpt_header {

	int			lkh_magic; /* magic number */
	ldr_heap_t		lkh_heap; /* heap for allocations */
	struct {			/* struct dqueue */
		struct ldr_module_rec *lkh_modforw;
		struct ldr_module_rec *lkh_modback;
	}			lkh_known_modules; /* known module list */
	ldr_kpt			lkh_kpt; /* known package table */
} ldr_kpt_header;

#define lkh_forw	lkh_known_modules.lkh_modforw
#define lkh_back	lkh_known_modules.lkh_modback

#define	LKH_MAGIC	0xdb6db6db

#define	LDR_N_INSTALLS	32		/* initial size of install table */


/* Initialize the specified KPT header, using storage from the
 * specified heap.  Returns LDR_SUCCESS on success, negative
 * error status on error.
 */

extern int
ldr_kpt_hdr_init __((ldr_kpt_header *kpt_hdr, ldr_heap_t heap));

/* Try to inherit the specified KPT header.  Basically just involves
 * error checking.  Returns LDR_SUCCESS on success, negative error
 * status on error.
 */

extern int
ldr_kpt_hdr_inherit __((ldr_kpt_header *kpt_hdr));

/* Copy all the installed modules from the source KPT header to
 * the destination KPT header.  Used in constructing the global
 * KPT from a private KPT.  The module records and KPT list are
 * copied, using the destination KPT's heap for allocation.
 * Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_kpt_copy __((ldr_kpt_header *from_hdr, ldr_kpt_header *to_hdr));

/* Create a known package table list large enough to hold count records,
 * initialize it, and return in in *kpts.  Allocate space for the
 * kpt list from the specified heap.  Returns LDR_SUCCESS on success, or
 * negative error status on error.
 */

extern int ldr_kpt_list_create __((ldr_heap_t heap, int count,
				   ldr_kpt_rec **kpts));

/* Look up the specified package by name in the specified known package
 * table, and return the kpt record corresponding to it in
 * *kpt_rec.  Returns LDR_SUCCESS on success, or negative error status on
 * failure (including LDR_ENOPKG if the lookup fails).
 */

extern int ldr_kpt_lookup __((ldr_kpt kpt, ldr_package_rec *pkg,
			      ldr_kpt_rec **kpte));

/* Free a list of kpt records containing count records.  Free the
 * storage into the specified heap.  Returns LDR_SUCCESS on success,
 * negative error status on error.
 */

extern int ldr_kpt_list_free __((ldr_heap_t heap, int count, ldr_kpt_rec *kpts));

#endif /* _H_LDR_KNOWN_PKG */
