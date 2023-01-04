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
 *	@(#)$RCSfile: ldr_switch.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:15:25 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_switch.h
 * Definitions for loader switch
 *
 * The loader switch is the primary procedural interface between the
 * format-independent and format-dependent portions of the loader.
 * There is one entry in the switch per format-dependent manager.
 * This file depends on: ldr_types.h ldr_import.h ldr_region.h ldr_package.h
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_SWITCH
#define _H_LDR_SWITCH


/* The manager entry point is called with a pointer to a loader context.
 * It is responsible for pushing its switch entry onto the context's
 * switch (by calling ldr_context_push_switch_ent()).  This procedure
 * allows dynamically-loaded auxiliary managers to be initialized in
 * the same way as statically-linked managers.
 */

typedef int (*ldr_mgr_entry_p) __((ldr_context_t ctxt));


/* Loader switch procedure types */

/* The recognizer routine checks to see whether the specified file
 * (opened for at least O_READ access mode on file descriptor fd)
 * is of an object file format supported by this format-dependent
 * manager.  It returns LDR_SUCCESS on success or a negative loader error
 * status on failure.  On success, the format-dependent manager's handle
 * is left in the handle variable.  Also, after a successful recognition,
 * the open file descriptor is the resposibility of the format-dependent
 * manager; it is never used again by the format-independent manager.
 */

typedef	int (*lsw_recog_p) __((const char *filename, ldr_file_t fd,
			       ldr_module_handle *handle));

/* Iterator returing the pathnames of the static dependencies of the object
 * module with the specified format-dependent handle.  depno is the index of
 * the dependency to be found, starting at zero.  Return pointer to pathname
 * of static dependency (as a ldr_strdup'ed string; caller will ldr_free it)
 * in *dep.  Returns LDR_SUCCESS on success, a negative loader error
 * status on error (including LDR_EAGAIN to indicate the end of the
 * dependencies). 
 */

typedef int (*lsw_get_static_dep_p) __((ldr_module_handle handle, int depno,
					char **dep));

/* Return the lists of import packages and import symbols for the
 * specified object module.  The callee allocates the lists and their
 * contents, and will be responsible for freeing them.  The callee must
 * fill in the following fields of each package record:
 *  - structure version number (compatibility check)
 *  - import package name
 *  - import package kind
 * and the following fields of each symbol record:
 *  - structure version number (compatibility check)
 *  - symbol name
 *  - import package number
 * Returns the number of packages in the package list in *pkg_count, and
 * the number of symbols in the import list in *sym_count.
 * Return LDR_SUCCESS on success or negative error status on error.
 */

typedef int (*lsw_get_imports_p) __((ldr_module_handle handle, int *pkg_count,
				     ldr_package_rec **pkgs, int *sym_count,
				     ldr_symbol_rec **imports));

/* Map the regions of the object file into the process' address space.
 * The callee allocates the list of mapped regions and its contents,
 * and will be responsible for freeing it.  The callee must fill in
 * these fields of the region descriptor record for each region mapped:
 *   - structure version number
 *   - region name (may be NULL if no region name is known)
 *   - region kind
 *   - region protection
 *   - the address it is to ultimately live at in the destination process'
 *     address space (vaddr)
 *   - the address it is currently mapped at in this process (mapaddr)
 *   - region size
 * 
 * allocsp is pointer to structure holding address allocation and deallocation
 * procedures to use; see ldr_types.h for description.
 * Returns the number of regions in the region list in *reg_count.
 * Return LDR_SUCCESS on success or negative error status on error.

 */

typedef int (*lsw_map_regions_p) __((ldr_module_handle handle,
				     ldr_region_allocs *allocsp,
				     int *reg_count, ldr_region_rec **regions));

/* Return the list of packages exported by this object module.  The
 * callee allocates the list and its contents, and will be responsible
 * for freeing it. The calle must fill in the following fields of each
 * package record:
 *  - structure version number
 *  - export package name
 *  - export package kind
 * Returns the number of exported packages in *count.
 * Return LDR_SUCCESS on success or negative error status on error.
 */

typedef int (*lsw_get_export_pkgs_p) __((ldr_module_handle handle, int *count,
					 ldr_package_rec **packages));

/* Return the list of exported symbols for the
 * specified object module.  The callee allocates the list and its
 * contents (the list MUST be allocated by calling ldr_symbols_create()),
 * but the CALLER is responsible for freeing the list.  The caller must
 * have previously called the get_export_pkgs_p call to get the list of
 * packages exported by this module.  The callee must
 * fill in the following fields of each symbol record:
 *  - structure version number (compatibility check)
 *  - symbol name
 *  - export package number (in previously-obtained export pkg list)
 *  - symbol value
 * Returns the number of symbols in the export list in *sym_count.
 * Return LDR_SUCCESS on success or negative error status on error.
 *
 * This routine is not called by the format-independent manager in normal
 * module loading.  It is intended for use only when pre-loading modules,
 * and possibly to allow format-dependent managers such as ELF to implement
 * their own symbol resolution algorithms.
 */

typedef int (*lsw_get_exports_p) __((ldr_module_handle handle, int *sym_count,
				     ldr_symbol_rec **exports));

/* Look up the specified import symbol from the specified packge in
 * the specified object module, and fill in its value in the import
 * symbol record.  Can use the following fields in the import record:
 *  - symbol name
 * Must fill in the following fields in the import symbol record:
 *  - symbol value
 * Return LDR_SUCCESS on success or negative error status on error.
 */

typedef int (*lsw_lookup_export_p) __((ldr_module_handle handle,
				       ldr_package_rec *package,
				       ldr_symbol_rec *symbol));

/* Relocate all the relocatable addresses everywhere in the specified
 * object module.  regions is the array of nregions region description
 * records describing the regions mapped from this object module, as
 * returned from the lsw_map_regions call.  import_pkgs and imports
 * are arrays on npackages package records and nimports import records
 * (respectively) describing the packages and symbols imported by this
 * object module, as returned by the lsw_get_imports call.  All
 * symbols have been resolved to a symbol value.  Return LDR_SUCCESS
 * on success or negative error status on error.
 */

typedef int (*lsw_relocate_p) __((ldr_module_handle handle, int nregions,
				  ldr_region_rec *regions, int npackages,
				  ldr_package_rec *import_pkgs, int nimports,
				  ldr_symbol_rec *imports));

/* Return the address of the entry point of the specified module, if
 * any, in *entry_pt.  Return LDR_SUCCESS on success or negative
 * error status on error.
 */

typedef int (*lsw_get_entry_pt_p) __((ldr_module_handle handle,
				      ldr_entry_pt_t *entry_pt));

/* Run the specified module's initialization or termination entry points,
 * as specified by the kind flag, if any.  Return LDR_SUCCESS on success
 * or negative error status on error.
 */

typedef int (*lsw_run_inits_p) __((ldr_module_handle handle, entry_pt_kind kind));

/* Complete the loading of the specified module, clean up open files,
 * temporary data structures, etc.  Return LDR_SUCCESS on success or
 * negative error status on error.
 */

typedef int (*lsw_cleanup_p) __((ldr_module_handle handle));

/* Unload the specified object module.  allocsp is pointer to
 * structure holding address allocation and deallocation procedures to
 * use; see ldr_types.h for description.  The region list describes
 * the address and size of each mapped region; the callee is
 * responsible for freeing this list.  The imports, import_pkgs, and
 * export_pkgs lists are also passed in to this procedure in case they
 * are needed during unloading; the callee is also responsible for
 * freeing them.  On return, the module handle, and the region,
 * import, import package, and export package lists are dead and
 * cannot be used further by the caller.
 */

typedef int (*lsw_unload_p) __((ldr_module_handle handle, ldr_region_allocs *allocsp,
				int reg_count, ldr_region_rec *regions,
				int ipkg_count, ldr_package_rec *import_pkgs,
				int import_count, ldr_symbol_rec *imports,
				int epkg_count, ldr_package_rec *export_pkgs));

#ifdef __osf__
/* Resolve an unresolved symbol imported by an object module.
 * On success, fill in the sym->ls_module with the module defining
 * the symbol and return LDR_SUCCESS.  Return negative error status
 * on error.  Used to support kernel loading and other symbol resolution
 * policies that are not strictly package-based.  This handler can
 * be left as NULL, which indicates that there is no special handler
 * for unresolved symbols and a resolution error should result.
 */

typedef int (*lsw_unres_p) __((ldr_context_t context,
			       ldr_package_rec *pkg,
			       ldr_symbol_rec *sym));

#endif /* __osf__ */


#define	LSW_VERSION	1		/* current structure version */

/* Loader switch flags */

typedef	int	loader_switch_flags_t;

#define LSF_NONE	(loader_switch_flags_t)(0) /* no flags */
#define	LSF_MUSTOPEN	(loader_switch_flags_t)(0x1) /* must open file for recog */

/* Each manager supplies a loader_switch_entry structure.  The manager
 * pushes this structure onto the loader context's loader switch by calling
 * ldr_switch_ins_head(), during execution of the manager's
 * entry point.
 */

struct loader_switch_entry {
	int			lsw_version; /* structure version number */
	loader_switch_flags_t	lsw_flags;
	lsw_recog_p		lsw_recog;
	lsw_get_static_dep_p	lsw_get_static_dep;
	lsw_get_imports_p	lsw_get_imports;
	lsw_map_regions_p	lsw_map_regions;
	lsw_get_export_pkgs_p	lsw_get_export_pkgs;
	lsw_get_exports_p	lsw_get_exports;
	lsw_lookup_export_p	lsw_lookup_export;
	lsw_relocate_p		lsw_relocate;
	lsw_get_entry_pt_p	lsw_get_entry_pt;
	lsw_run_inits_p		lsw_run_inits;
	lsw_cleanup_p		lsw_cleanup;
	lsw_unload_p		lsw_unload;
	lsw_unres_p		lsw_unres;
};

/* Insert the specifed loader switch entry at the head of the loader
 * switch in the specified context.  Only a loader context created from
 * the default heap should ever have loader switch entries, as the
 * loader switch links are always allocated from the default heap.
 * This routine is mostly useful for special tasks such as
 * loader bootstrapping.  Returns LDR_SUCCESS on success or negative
 * error status on error.
 */

extern int
ldr_switch_ins_head __((ldr_context_t ctxt, ldr_switch_t swtch));

/* Remove the first loader switch entry from the head of the loader
 * switch in the specified context.
 * Used to undo the effects of a ldr_switch_ins_head.  Can't fail.
 */

extern void
ldr_switch_rem_head __((ldr_context_t ctxt));

/* Insert the specifed loader switch entry at the tail of the loader
 * switch in the specified context.  Only a loader context created from
 * the default heap should ever have loader switch entries, as the
 * loader switch links are always allocated from the default heap.
 * This routine is called by each format-dependent manager during
 * manager initialization, to declare the manager's switch entries
 * to the format-independent manager.  Returns LDR_SUCCESS on success
 *  or negative error status on error.
 */

extern int
ldr_switch_ins_tail __((ldr_context_t ctxt, ldr_switch_t swtch));

/* Remove the last loader switch entry from the tail of the loader
 * switch in the specified context.
 * Used to undo the effects of a ldr_switch_ins_tail.  Can't fail.
 */

extern void
ldr_switch_rem_tail __((ldr_context_t ctxt));

/* The table of builtin manager entry points */

extern ldr_entry_pt_t	ldr_manager_entries[];
extern int n_ldr_mgr_entries;

#endif /* _H_LDR_SWITCH */
