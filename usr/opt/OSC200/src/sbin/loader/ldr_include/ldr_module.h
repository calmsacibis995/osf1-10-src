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
 *	@(#)$RCSfile: ldr_module.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:16:57 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_module.h
 * Definitions for loader module records
 *
 * Loader module records hold the information about each loaded module
 * in the process (or other loader context).  Module records are accessed
 * either by hashing the module name into the module name hash table
 * for the context, or by looking up the external name of
 * the module in the list of all loaded modules.
 *
 * This file depends on: loader.h ldr_types.h chain_hash.h ldr_region.h
 * 			 ldr_import.h ldr_package.h ldr_known_pkg.h
 *			 ldr_lock.h standards.h dqueue.h
 *
 * OSF/1 Release 1.0
 */


#ifndef _H_LDR_MODULE
#define _H_LDR_MODULE

#include "ldr_macro_help.h"

/* Flags for module record */

typedef	unsigned	ldr_module_flags_t;

#define	LMF_NONE		0
#define	LMF_UNRESOLVED		0x1	/* module still has unresolved symbols */
#define	LMF_LOADED		0x2	/* module has been fully loaded */
#define	LMF_ONLIST		0x4	/* module rec on known module list */
#define LMF_LOADING		0x8	/* module rec being loaded now */
#define LMF_STATIC_DEP_DONE	0x10	/* all static dependencies of module 
					 * are on known module list
					 */
#define LMF_INSTALLED		0x20	/* module installed, not loaded */


/* Entry for a module in the module name hash chains.
 * This structure corresponds to the chain_hash_entry structure in
 * chain_hash.h
 */

struct	lm_hash_entry {
	struct	lm_hash_entry	*lh_next; /* forward link */
	char			*lh_name; /* key */
	struct	ldr_module_rec	*lh_rec; /* slight kludge -- ptr to this record */
};

/* Entry for a module in the known module list; also used for export-only
 * list links (a module can't be on both lists at the same time).
 * Corresponds to the dqueue structure in dqueue.h
 */

struct lm_module_entry {
	struct	ldr_module_rec	*le_forw;
	struct	ldr_module_rec	*le_back;
};


/* The loader module record itself */

typedef struct ldr_module_rec {

	/* NOTE: next field must first in the structure so 
	 * that the dqueue routines can be used.
	 */

	struct lm_module_entry	lm_list; /* known/exportonly module list chains */
	struct lm_hash_entry	lm_hash; /* forward link, key for hash chain */
	ldr_module_t		lm_module; /* module ID for this module */
	struct loader_switch_entry *lm_switch; /* format-dependent routines */
	ldr_module_handle	lm_handle; /* handle for format-dep routines */
	int			lm_region_count; /* number of regions */
	ldr_region_rec		*lm_regions; /* regions of this module */
	int			lm_import_pkg_count; /* number of import pkgs */
	ldr_package_rec		*lm_import_pkgs; /* import package list */
	int			lm_import_count; /* number of imports */
	ldr_symbol_rec		*lm_imports; /* import list */
	int			lm_export_pkg_count; /* number of export pkgs */
	ldr_package_rec		*lm_export_pkgs; /* export package list */
	ldr_kpt_rec		*lm_kpt_list; /* module's loaded package list */
	ldr_load_flags_t	lm_load_flags; /* load flags for module */
	ldr_module_flags_t	lm_flags; /* flags */
} ldr_module_rec;

#define	lm_forw	lm_list.le_forw
#define	lm_back	lm_list.le_back
#define	lm_next	lm_hash.lh_next
#define	lm_name	lm_hash.lh_name
#define	lm_rec	lm_hash.lh_rec


/* The module name hash table type */

typedef	chain_hashtab_t	ldr_module_hashtab;


/* Module list header; corresponds to struct dqueue in dqueue.h */

struct lm_module_list {
	struct ldr_module_rec	*ll_forw;
	struct ldr_module_rec	*ll_back;
};


/* The loader switch is a doubly-linked list of loader switch links.  Each
 * link points to the loader switch entry as supplied by the manager.
 * The loader context contains the list header for the switch.
 */

struct ldr_switch_links {		/* must match struct dqueue_elem */
	struct ldr_switch_links *lsl_forw;
	struct ldr_switch_links *lsl_back;
	struct loader_switch_entry *lsl_entry;
};

/* The list header for the loader switch */

typedef struct ldr_switch_t {		/* must match struct dqueue_elem */
	struct ldr_switch_links *lss_forw;
	struct ldr_switch_links *lss_back;
} ldr_switch;


/* The loader context is the structure that contains all the loader state
 * for a given process.  A given process may maintain more than one
 * loader context; for example, the kernel loader process maintains both
 * its own loader context and also the kernel's loader context.
 */

typedef struct ldr_context {

	ldr_switch		lc_switch; /* this context's loader switch */
	struct lm_module_list	lc_known_modules; /* known module table */
	struct lm_module_list	lc_exportonly_modules; /* exportonly module table */
	ldr_module_hashtab	lc_module_hash; /* module name hash table */
	ldr_kpt			lc_lpt;	/* loaded package table */
	ldr_kpt_header		*lc_global_kpt; /* global known pkg table header */
	ldr_kpt_header		*lc_private_kpt; /* global known pkg table header */
	ldr_region_allocs	lc_allocsp; /* region alloc/dealloc procs */
	ldr_module_t		lc_next_module_id; /* next module ID to assign */
	ldr_module_t		lc_dynmgr; /* mod ID of highest known dyn mgr */
} ldr_context;


/* Locking macros for context */

#define	ldr_lock_context(ctx)	ldr_lock(&ldr_global_lock)

#define	ldr_unlock_context(ctx)	ldr_unlock(&ldr_global_lock)


/* The following macros manipulate the module flags in the module record
 * that describe the current state of the module.
 */

/* Mark a module as being loaded.  Should only be applied to a newly-allocated
 * module or an export-only module.
 */

#define lm_flag_loading(mod)	((mod)->lm_flags = \
	(((mod)->lm_flags & ~LMF_LOADED) | LMF_LOADING) )

/* Mark a module as being loaded.  Should only be applied to a module currently
 * marked as loading.
 */

#define lm_flag_loaded(mod)	((mod)->lm_flags = \
	(((mod)->lm_flags & ~LMF_LOADING) | LMF_LOADED) )

/* Mark a module as being on the known module list. */

#define lm_flag_onlist(mod)	((mod)->lm_flags |= LMF_ONLIST)

/* Mark a module as having unresolved globals. */

#define lm_flag_unresolved(mod)	((mod)->lm_flags |= LMF_UNRESOLVED)

/* Mark a module as having all its static dependencies on the known module list */

#define lm_flag_static_dep_done(mod) ((mod)->lm_flags |= LMF_STATIC_DEP_DONE)

/* Mark a module record as being installed, rather than loaded.  This should
 * only be done to a module record copy, as made by ldr_module_copy().
 */

#define lm_flag_installed(mod)	((mod)->lm_flags = \
	(((mod)->lm_flags & ~(LMF_LOADING|LMF_LOADED)) | \
	  LMF_INSTALLED) )

/* The following macros manipulate the load flags in the module record */

/* Save the load flags in the module record */

#define	lm_set_load_flags(mod, flags)	((mod)->lm_load_flags = (flags))


/* Other macros */

/* Upgrade the specified module from export-only loading to full loading.
 * This involves removing it from the export-only list if it's on it,
 * clearing the export-only flag, and flagging the module as "loading".
 */

#define lm_upgrade_exportonly(mod) MACRO_BEGIN \
	if ((mod)->lm_flags & LMF_ONLIST) { \
		dq_rem_elem(&(mod)->lm_list); \
		(mod)->lm_flags &= ~LMF_ONLIST; \
	} \
	(mod)->lm_load_flags &= ~LDR_EXPORTONLY; \
	(mod)->lm_flags |= LMF_LOADING; \
	MACRO_END

/* The following macro can be used in a for loop to iterate through all
 * the modules on the known module record list of a specified context.
 * The list should not be changed during the iteration.
 */

#define	for_all_modules(context, mod) \
	for ((mod) = (context)->lc_known_modules.ll_forw; \
	     (mod) != (ldr_module_rec *)&((context)->lc_known_modules); \
	     (mod) = (mod)->lm_forw)

/* The following macro can be used as a for loop to iterate through all
 * the regions in the region list of the specified module.
 * id gives the region number, reg a pointer to the region record.
 */

#define for_all_regions(mod, id, reg) \
	for ((id) = 0, (reg) = &(mod)->lm_regions[0]; \
	     (id) < (mod)->lm_region_count; \
	     (id)++, (reg)++)

/* Set the region list of a module record */

#define	lm_set_region_list(mod, count, regions)	MACRO_BEGIN \
	(mod)->lm_region_count = (count); \
	(mod)->lm_regions = (regions); \
	MACRO_END

/* The following macro can be used as a for loop to iterate through all
 * the packages in the export package list of the specified module.
 * i gives the package number, pkg a pointer to the package record.
 */

#define for_all_export_pkgs(mod, i, pkg) \
	for ((i) = 0, (pkg) = &(mod)->lm_export_pkgs[(i)]; \
	     (i) < (mod)->lm_export_pkg_count; \
	     (i)++, (pkg)++)

/* The following macro can be used as a for loop to iterate through all
 * the packages in the loaded package table list of the specified module.
 * i gives the package number, kpte a pointer to the kpt list record.
 */

#define for_all_kptes(mod, i, kpte) \
	for ((i) = 0, (kpte) = &(mod)->lm_kpt_list[(i)]; \
	     (i) < (mod)->lm_export_pkg_count; \
	     (i)++, (kpte)++)

/* Set the export package list and kpt list of a module record */

#define	lm_set_kpt_list(mod, count, epkgs, kpts) MACRO_BEGIN \
	(mod)->lm_export_pkg_count = (count); \
	(mod)->lm_export_pkgs = (epkgs); \
	(mod)->lm_kpt_list = (kpts); \
	MACRO_END

/* The following macro can be used as a for loop to iterate through all
 * the import symbols in the import symbol table of the specified module.
 * i gives the package number, sym a pointer to the import symbol record,
 * pkg a pointer to the symbol's package record.
 */

#define for_all_imports(mod, i, pkg, sym) \
	for ((i) = 0, (sym) = (mod)->lm_imports, \
	     pkg = ((sym) == NULL ? NULL : &((mod)->lm_import_pkgs[(sym)->ls_packageno])); \
	     (i) < (mod)->lm_import_count; \
	     (i)++, (sym)++, pkg = &((mod)->lm_import_pkgs[(sym)->ls_packageno]) )


/* Set the import package list and import symbol table of a module record */

#define	lm_set_imports(mod, pkg_count, pkgs, sym_count, syms) MACRO_BEGIN \
	(mod)->lm_import_pkg_count = (pkg_count); \
	(mod)->lm_import_pkgs = (pkgs); \
	(mod)->lm_import_count = (sym_count); \
	(mod)->lm_imports = (syms); \
	MACRO_END


/* Macro to iterate through all the links in a loader switch */

#define for_all_switch_entries(lss, lswl) \
	for ((lswl) = (lss)->lss_forw; \
	     (lswl) != (struct ldr_switch_links *)(lss); \
	     (lswl) = (lswl)->lsl_forw)

/* Macro to init a loader switch */

#define lsw_init(lsw)	dq_init(lsw)

/* Macro to add loader switch links to head of specified switch */

#define	lsw_ins_head(lsw, lswl)	dq_ins_head(lsw, lswl)

/* Macro to add loader switch links to tail of specified switch */

#define	lsw_ins_tail(lsw, lswl)	dq_ins_tail(lsw, lswl)

/* Macro to remove loader switch links from head of specified switch */

#define	lsw_rem_head(lsw)	dq_rem_head(lsw, struct ldr_switch_links *)

/* Macro to remove loader switch links from tail of specified switch */

#define	lsw_rem_tail(lsw)	dq_rem_tail(lsw, struct ldr_switch_links *)


/* Loader switch operations called through a module record */

#define	LSW_GET_STATIC_DEP(m, dp, d) \
	(*((m)->lm_switch->lsw_get_static_dep))((m)->lm_handle, (dp), (d))
#define	LSW_GET_IMPORTS(m, pc, p, ic, i) \
	(*((m)->lm_switch->lsw_get_imports))((m)->lm_handle, (pc), (p), (ic), (i))
#define	LSW_MAP_REGIONS(m, asp, rc, rr) \
	(*((m)->lm_switch->lsw_map_regions))((m)->lm_handle, (asp), (rc), (rr))
#define	LSW_GET_EXPORT_PKGS(m, pc, ep) \
	(*((m)->lm_switch->lsw_get_export_pkgs))((m)->lm_handle, (pc), (ep))
#define	LSW_GET_EXPORTS(m, ec, e) \
	(*((m)->lm_switch->lsw_get_exports))((m)->lm_handle, (ec), (e))
#define	LSW_LOOKUP_EXPORT(m, p, s) \
	(*((m)->lm_switch->lsw_lookup_export))((m)->lm_handle, (p), (s))
#define	LSW_RELOCATE(m, nr, rl, np, ip, ni, il) \
	(*((m)->lm_switch->lsw_relocate))((m)->lm_handle, (nr), (rl), (np), (ip),\
					  (ni), (il))
#define	LSW_GET_ENTRY_PT(m, e) \
	(*((m)->lm_switch->lsw_get_entry_pt))((m)->lm_handle, (e))
#define	LSW_RUN_INITS(m, k) \
	(*((m)->lm_switch->lsw_run_inits))((m)->lm_handle, (k))
#define	LSW_CLEANUP(m) \
	(*((m)->lm_switch->lsw_cleanup))((m)->lm_handle)
#define LSW_UNLOAD(m, asp, rc, r, ipc, ip, ic, i, epc, ep) \
	(*((m)->lm_switch->lsw_unload))((m)->lm_handle, (asp), (rc), (r), \
					(ipc), (ip), (ic), (i), (epc), (ep))
#define	LSW_UNRES(m, c, p, s) \
	(*((m)->lm_switch->lsw_unres))(c, p, s)
#define	LSW_HAS_UNRES(m) \
	((m)->lm_switch->lsw_unres)


/* Routines for creating and freeing module records and components */

/* Attempt to inherit the specified loader context, presumably from a
 * keep-on-exec region or mapped file.  Currently only does error
 * checking on the inherited components.
 */

extern int
ldr_context_inherit_ctxt __((ldr_context *context));

/* Create a copy of the specified loader context from the specified heap.
 * The new loader context is allocated from the heap and initialized to a copy
 * of selected contents of the original.  Since this routine is intended to
 * be used only during pre-loading, only those fields important for preloading
 * are copied.  These are:
 *  the known module list
 *  the module name hash table
 * Note in particular that the copy will have no loaded or known
 * package tables, and no loader switch.
 * Returns LDR_SUCCESS on success or negative status on error.
 */

extern int
ldr_context_copy __((ldr_heap_t heap, ldr_context *ctxt, ldr_context **new_ctxt));

/* Create a module record for the specified module name, and return it.
 * The module record is initialized and installed on the context's hash
 * chains, but is NOT yet linked into the known module list.
 * Returns LDR_SUCCESS on success or negative status on error.
 */

extern int
ldr_module_create __((ldr_context * context, const char *name,
		      ldr_module_rec **mod));

/* Free the specified module record.  Dequeue it from the known module
 * list it's on (if any), delete it from the hash chains, and free it.
 */

extern void
ldr_module_destroy __((ldr_context *context, ldr_module_rec *mod));

/* Create a copy of the specified module record from the specified heap.
 * The new module record is allocated from the heap and initialized to a copy
 * of the original (including copies of the original's package and region
 * tables, also allocated from the heap).
 * The copy is NOT installed on any hash chains or known module lists.
 * Returns LDR_SUCCESS on success or negative status on error.
 */

extern int
ldr_module_copy __((ldr_heap_t heap, ldr_module_rec *orig, ldr_module_rec **mod));

/* Free the specified module record copy into the specified heap.
 * This includes freeings its region, import, and package table
 * copies as well.  Module must have been removed from the known
 * package table already.  Dequeue it from the module list it's on
 * (if any), free its contents, and free it.
 */

extern void
ldr_module_free_copy __((ldr_heap_t heap, ldr_module_rec *mod));

/* Translate the specified module ID into a pointer to the module
 * record for the module.  Return LDR_SUCCESS on success, EINVAL if the
 * module record is not found.
 */

extern int
translate_module_name __((ldr_context *context, ldr_module_t mod_id,
			    ldr_module_rec **module));

/* Create a copy of the specified module record, and install the copy
 * in the specified known package table.  First, scan the list to make
 * sure it's not already there.  Then copy the module record (copying
 * only the relevant fields) into space allocated from the heap in the
 * kpt header.  Link the copied module record onto the installed module list
 * in the kpt header. Returns LDR_SUCCESS on success, negative error
 * status on error.
 */

extern int ldr_install_module __((ldr_kpt_header *kpt_hdr, ldr_module_rec *mod));

/* Remove the specified module record from the known module list of
 * the specified KPT, and free it into the KPT's heap.  Module is
 * assumed to be linked to the KPT's known module list.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

extern int ldr_remove_module __((ldr_kpt_header *kpt_hdr, ldr_module_rec *mod));

/* Look up the specified module by name in the list of modules installed
 * in the specified known package table.  If found, return the module
 * record copy describing the module.  Returns LDR_SUCCESS on success,
 * negative error status on failure.
 */

extern int ldr_lookup_installed_module __((ldr_kpt_header *kpt_hdr,
					   const char *name, ldr_module_rec **mod));

/* Insert all the packages in the specified module's export package
 * table list into the specified known package table.
 * Note that package names must be unique.  Return LDR_SUCCESS on
 * success or negative error status on error, including LDR_EEXIST
 * on duplicate package name.
 */

extern int ldr_kpt_insert __((ldr_kpt kpt, ldr_module_rec *mod));

/* Remove the specified module's kpt list from the specified known
 * package table.  Walk the kpt list, unhashing each record
 * from the kpt.  Returns LDR_SUCCESS on success, or negative error status
 * on error.
 */

extern int ldr_kpt_remove __((ldr_kpt kpt, ldr_module_rec *mod));

/* Routines to copy standard data structures to storage allocated from a
 * specified heap, and to free the copies.
 */

extern int ldr_packages_copy __((ldr_heap_t heap, int count, ldr_package_rec *orig,
				 ldr_package_rec **retval));
extern int ldr_packages_free_copy __((ldr_heap_t heap, int count, ldr_package_rec *val));
extern int ldr_symbols_copy __((ldr_heap_t heap, int count, ldr_symbol_rec *orig,
				ldr_symbol_rec **retval));
extern int ldr_symbols_free_copy __((ldr_heap_t heap, int count, ldr_symbol_rec *val));
extern int ldr_regions_copy __((ldr_heap_t heap, int count, ldr_region_rec *orig,
				ldr_region_rec **retval));
extern int ldr_regions_free_copy __((ldr_heap_t heap, int count, ldr_region_rec *val));

#endif /* _H_LDR_MODULE */
