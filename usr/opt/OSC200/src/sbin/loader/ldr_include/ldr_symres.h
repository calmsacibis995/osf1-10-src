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
 *	@(#)$RCSfile: ldr_symres.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/04/23 14:45:42 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_symres.h
 * External declarations for loader symbol resolution routines
 * This file depends on: <loader.h> ldr_types.h ldr_region.h ldr_package.h 
 *			 ldr_import.h ldr_known_pkg.h ldr_module.h
 *
 *
 * OSF/1 Release 1.0
 */
/*
 * ldr_symres.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

#ifndef	_H_LDR_SYMRES
#define	_H_LDR_SYMRES

/* Return a module record for the specified module.  First look up the
 * library name in the context's hash table; if found, return its module
 * record.  If not found, allocate a module record.  Call the recognizer
 * functions to determine the object file format of the module and to
 * get the format-dependent handle back.  Then call the format-dependent
 * manager to get the export list for the module.  Add the module
 * to the context's hash table (but NOT to its known module list, since
 * we don't know where in the list to link the module).  The flags
 * parameter can indicate that it is an error if the module already exists
 * (LDR_NOPREXIST), or that the module is simply being scanned for exported
 * symbols during symbol resolution instead of being loade (LDR_EXPORTONLY).
 * Return LDR_SUCCESS on success or negative error status on error.
 */

extern int ldr_get_module_record __((ldr_context *context, const char *module_name,
				     ldr_load_flags_t flags, ldr_module_rec **mod,
				     ldr_file_t fd));

/* Run the recognizers of the managers in the loader switch for the
 * specified loader context.  Run them in turn until one recognizes
 * the module, or until all have returned failure.  On success, return
 * the manager's handle in *handle, and the loader switch entry in
 * *lsw.  The fd argument is an in/out argument containing an open
 * file descriptor for the module; if it is not equal LDR_FILE_NONE
 * and the manager demands that the file be opened, we will open
 * it here.  Return LDR_SUCCESS on success or negative error status
 * on error.
 */

extern int
ldr_recognize __((ldr_context *context, const char *module_name, ldr_file_t *fd,
		  ldr_module_handle *handle, struct loader_switch_entry **lswp));

/* Build module records for all the static dependencies (and,
 * recursively, all their static dependencies, etc.) of modules being
 * loaded in the specified context.  Then resolve all unresolved
 * import symbols in the list of modules to be loaded, by querying the
 * format-dependent manager to get the information on the unresolved
 * symbol (including symbol name and package), and then looking up the
 * package and symbol in the installed library table.  The module
 * records are enqueued on the (global) module list.
 *
 * Note that this loop is very careful to keep the static dependencies
 * in their "natural" load order (pre-order depth-first tree walk).
 * This is necessary so that the ELF format-dependent manager can
 * simulate the behavior of the standard System V Release 4 symbol
 * resolution policy, which depends on load order (sigh).
 */

extern int ldr_get_module_dependencies __((ldr_context *context));

/* Get the list of all the packages exported by the specified module
 * and add them to the loaded package table for this context.  Once in
 * the loaded package table, they are available for resolution of
 * unresolved import symbols.
 *
 * Returns LDR_SUCCESS on success and negative error status on error.
 */

extern int ldr_install_lpt __((ldr_context *context, ldr_module_rec *mod));

/* Remove the specified module's loaded packages from the loaded package
 * table for this context, as part of unloading the module.  First, unhash
 * the module's lpt records from the context's loaded package table; then
 * free the lpt list.  Returns LDR_SUCCESS on success or negative error
 * status on error.
 */

extern int ldr_remove_lpt __((ldr_context *context, ldr_module_rec *mod));

/* Try to resolve the specified package/symbol pair in the specified
 * loader context.  If successful, fill in the symbol value structure
 * (including the identity of the exporting module) with the best
 * currently known representation of the symbol value.
 *
 * The "package" may be a real package name, in which case it is looked up
 * in the various known package tables to determine the module to be loaded.
 * Alternatively, it may be a full module name, in which case the symbol
 * must be exported by the specified module; this is the hook which allows
 * format-dependent managers (such as ELF) to control their own symbol
 * resolution policies.
 *
 * The module flags may indicate that no unresolved symbols are allowed; if so
 * and it can't be resolved, don't even try to do any machine-dependent handing
 * for unresolved symbols.
 */

extern int
ldr_resolve_symbol __((ldr_context *context, ldr_package_rec *pkg,
		       ldr_symbol_rec *sym, ldr_load_flags_t flags));

/* Precompute the absolute values of the symbols in the specified module's
 * import symbol table.  This routine deals with converting region-relative
 * symbol values to absolute symbol values, or (theoretically) with other
 * machine-dependent computations.  It is not an error for this routine to
 * do nothing; any symbol that can't be precomputed is assumed to be
 * handled by the format-dependent manager during relocation, possibly
 * in a machine-dependent way.
 */

extern void ldr_precompute_imports __((ldr_module_rec *mod));

/* Load the next dynamic manager, if any, and call its entry point
 * to get it to insert itself onto the specified context's loader
 * switch.
 */

extern int
ldr_load_dyn_mgr __((ldr_context *context));

/* Internal unload procedure for unloading a partially-loaded module */

extern int 
ldr_internal_module_unload __((ldr_context *context, ldr_module_rec *mod));

/* Internal procedure to get import list for a module record */

extern int
ldr_get_import_list __((ldr_module_rec *mod));

/* Internal procedure to resolve imports for a module record */

extern int
ldr_resolve_imports __((ldr_context *context, ldr_module_rec *mod));

#endif	/* _H_LDR_SYMRES */
