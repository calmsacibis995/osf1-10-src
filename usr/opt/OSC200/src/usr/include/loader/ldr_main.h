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
 *	@(#)$RCSfile: ldr_main.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:09:05 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_main.h
 * Main internal interfaces for format-independent loader routines
 *
 * This file contains all the declarations for the interfaces to the
 * format-independent loader used by the loader system calls.  The
 * internal loader data structures are not visible to the loader
 * system call layer.
 * This file depends on: loader.h ldr_types.h standards.h
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_MAIN
#define _H_LDR_MAIN

#include <lib_lock.h>

/* Exported externals */

extern	ldr_context_t	ldr_process_context;
extern const char	*ldr_global_data_file;
extern const char	*ldr_dyn_database;


/* Default "expected" number of modules in a loader process context.  This
 * is a hint to the context code; it is not a hard upper limit.
 */

#define	LDR_NMODULES		16

/* Standard global file location */

#define	LDR_GLOBAL_DATA_FILE	"/var/adm/loader/ldr_global.dat"

/* Standard dynamic manager database file location */

#define LDR_DYN_DATABASE 	"/etc/ldr_dyn_mgr.conf"


/* Entry points to format-independent loader */

/* Load the specified module and all its required dependencies (both
 * static dependencies and dependencies to resolve import symbols)
 * into the specified loader context.  This includes calling the
 * module initialization routines for all modules, unless the load_flags
 * specified no initialization (eg. for kernel modules).  Return the
 * module ID of the loaded module in *mod_id.
 * Returns LDR_SUCCESS on success or a negative error status on error.
 */

extern int
ldr_context_load __((ldr_context_t ctxt, const char *module_name,
		     ldr_load_flags_t load_flags, ldr_module_t *mod_id));

/* Return the entry point for the module named by the specified module ID
 * in the specified loader context.  Returns LDR_SUCCESS on success or
 * a negative error status on error.
 */

extern int
ldr_context_get_entry_pt __((ldr_context_t ctxt, ldr_module_t mod_id,
			     ldr_entry_pt_t *entry));

/* Unload the specified module.  Don't do anything to any of this module's
 * dependencies.
 * Return LDR_SUCCESS on success or a negative error status on error (EINVAL
 * for invalid module module ID).
 */

extern int
ldr_context_unload __((ldr_context_t ctxt, ldr_module_t mod_id));

/* Look up the specified symbol in the symbols exported by the specified
 * module ID, and return the absolute value of the symbol in *value.
 * Return LDR_SUCCESS on success or negative error status on error (ERANGE
 * if symbol value cannot be represented as a void *).
 */

extern int
ldr_context_lookup __((ldr_context_t ctxt, ldr_module_t mod_id, char *symbol_name,
		       void **value));

/* Look up the specified symbol in the symbols exported from the specified
 * package, and return the absolute value of the symbol in *value.
 * Return LDR_SUCCESS on success or negative error status on error (ERANGE
 * if symbol value cannot be represented as a void *).
 */

extern int
ldr_context_lookup_package __((ldr_context_t ctxt, char *package,
			       char *symbol_name, void **value));

/* Iterator through the module IDs for all modules currently loaded
 * in the specified context.  To initialize the iterator, set
 * *mod_id_ptr to LDR_MODULE_NULL.  The next call to this routine
 * should be made using the handle returned from this call and so on.
 * After the last module, the handle returned is the LDR_NULL_MODULE.
 * On success the call returns a zero, on failure it returns a
 * negative error number.
 */

extern int ldr_context_next_module __((ldr_context_t ctxt, ldr_module_t
				       *mod_id_ptr));


/* Return module information about the module with the specified ID
 * in the specified context, into the info buffer supplied by the
 * caller.  info_size is the size of the buffer provided.  Returns the
 * actual size of the returned structure on success, or a negative
 * error status on error.
 */

extern int ldr_context_inq_module __((ldr_context_t ctxt, ldr_module_t mod_id,
				      ldr_module_info_t *info, size_t info_size,
				      size_t *ret_size));


/* Return module information about the specified region of the
 * module with the specified ID in the specified context, into the
 * info buffer supplied by the caller.  info_size is the size of the
 * buffer provided.  Returns the actual size of the returned structure in
 * *ret_size.  Return 0 on success, or a negative error status on error.
 */

extern int ldr_context_inq_region __((ldr_context_t ctxt, ldr_module_t mod_id,
				      ldr_region_t region, ldr_region_info_t *info,
				      size_t info_size, size_t *ret_size));

/* Install the specified module in the private known package table
 * of the specified context.  If the private known package table does
 * not yet exist, create it.  The module must not duplicate any currently-
 * installed package names in the private known package table.  Returns
 * LDR_SUCCESS on success or negative error status on error.
 */

extern int
ldr_context_install __((ldr_context_t ctxt, const char *module_name));

/* Remove the specified module from the private known package table
 * of the specified context.  Returns LDR_SUCCESS on success or
 *  negative error status on error.
 */

extern int
ldr_context_remove __((ldr_context_t ctxt, const char *module_name));

/* Allocate a loader context and initialize it.  Arguments are the expected
 * number of modules that will be loaded into the context.  The context
 * is initialized to use the default loader switch.  Returns LDR_SUCCESS
 * on success or a negative loader status code on error.
 */

extern int
ldr_context_create  __((int nmodules, alloc_abs_region_p absp, alloc_rel_region_p
			relp, dealloc_region_p deallocp, ldr_context_t *ctxt));

/* Check to see whether a loader private data file has been inherited from
 * our parent process, and if so, inherit it and get from it the private 
 * KPT of the specified loader context.  Then, try to open the loader
 * global data file and inherit it, setting up the global KPT
 * of the specified context.  Called only during loader
 * bootstrapping.  Returns LDR_SUCCESS on success, negative error
 * status on error.
 */

extern int
ldr_context_inherit __((ldr_context_t ctxt));

/* The procedure is intended to be called only from the global library
 * installation program.  It initializes the loader global data file,
 * which contains the global known package table and all
 * pre-loaded libraries (not yet supported).  The global known package
 * table is initialized to a copy of the private installed package
 * table from the specified loader context.  When pre-loading is
 * supported, the pre-loaded libraries will be initialized from the
 * known module list of the specified context as well.
 *
 * This routine constructs the global data file header, initializes
 * the heap in the global data file, and copies the private KPT
 * from the context into the global data file's heap.
 *
 * Note that for this routine to be successful, the calling process
 * must not currently be using the global data file.  A caller
 * should call ldr_context_remove_global_file() before calling
 * this routine.
 *
 * Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_context_global_file_init __((ldr_context_t ctxt, ldr_file_t fd));

/* The procedure is intended to be called only from the global library
 * installation program.  It removes all dependencies on the loader global
 * data file from the specified loader context, and unmaps the
 * loader global data file from the address space.  It is intended
 * to be used in preparation for a call to ldr_context_global_file_init,
 * which must map the new global file into the same region of the
 * address space that the current global file is using.
 * Note that the calling program must not be using any pre-loaded libraries.
 * Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_context_global_file_remove __((ldr_context_t ctxt));

/* Bootstrap the loader system.  Includes building the loader process
 * context, containing the module and region records and exported symbol
 * list for the loader itself.  The loader_name argument is the name of
 * the loader's object module, for building the module record.  Returns
 * the address of the newly-created context in *ctxt.
 */

extern int ldr_bootstrap __((const char *loader_name, ldr_context_t *ctxt));

/* Bootstrap the specified loader context, by constructing a module
 * record for the loader itself and filling it in with the loader's
 * exports.  We bootstrap the context by doing a "fake load" on the
 * specified loader file, using a private loader switch as a temporary
 * switch so the recognizer will not try to open the file.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_context_bootstrap __((ldr_context_t ctxt, const char *loader_name));

/* Absolute region allocator for regions to be loaded into a process.  This is
 * the "default" absolute region allocator used in initializing the process
 * loader context.
 */

/* Translate a loader error status to a system errno.  If loader error status
 * is non-negative, simply returns status; if negative and magnitude is less
 * than SYSTEM_ERRNO_MAX, returns negative of loader error status; otherwise
 * uses table lookup to translate.
 */

extern int
ldr_status_to_errno __((int rc));

extern int alloc_abs_process_region __((univ_t vaddr, size_t size,
					ldr_prot_t prot, univ_t *baseaddr));

/* Relocatable region allocator for regions to be loaded into a process.  This is
 * the "default" relocatable region allocator used in initializing the process
 * loader context.
 */

extern int alloc_rel_process_region __((size_t size, ldr_prot_t prot,
					univ_t *vaddr, univ_t *baseaddr));

/* Region deallocator for regions loaded into a process.  This is
 * the "default" region deallocator used in initializing the process loader
 * context.
 */

extern int dealloc_process_region __((univ_t vaddr, univ_t mapaddr, size_t size));

/* Relocatable region allocator for regions of a main routine being
 * loaded into a process.  This is used only when loading relocatable
 * regions of the main program.
 */

extern int alloc_rel_main_region __((size_t size, ldr_prot_t prot,
				     univ_t *vaddr, univ_t *baseaddr));

/* Run any per-module termination routines for modules loaded into the
 * specified context.  This is intended to be called from the exit()
 * procedure during normal process exit.  Returns LDR_SUCCESS on success,
 * or negative error status on error (but tries to run all termination
 * routines even if one or more return errors).
 */

extern int
ldr_context_atexit __((ldr_context_t ctxt));

/* Install the locking functions to be used by the loader.  This is
 * intended to be run from pthreads_init() or other thread package
 * initialization routines.
 */

extern void
ldr_declare_lock_functions __((lib_lock_functions_t *funcs));

/* Set the loader's idea of the current break to the specified address,
 * If we don't yet have any idea of the current break, this routine sets
 * the minimum and current break addresses to the specified value, and does
 * not allocate any memory.  Otherwise, it grows or shrinks the break area
 * (by using mmap() and munmap()) so that the break area ends at the
 * specified address.
 */

extern int
ldr_brk __((char *addr));

/* Add the specified (signed) quantity to the loader's idea of the current
 * break.  If shrinking, unmap the space being removed.  If growing,
 * map more space.  Returns the old break.
 */

extern int
ldr_sbrk __((int incr, char **pobrk));

#endif	/* _H_LDR_MAIN */
