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
 *	@(#)$RCSfile: ldr_preload_mgr.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:59 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_preload_mgr.h
 * Decls for preload format-dependent manager
 *
 * This file contains the procedure declarations for the
 * preload format-dependent manager.
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_LDR_PRELOAD_MGR
#define	_H_LDR_PRELOAD_MGR

extern int preload_recog __((const char *filename, ldr_file_t fd,
			     ldr_module_handle *handle));
extern int preload_get_static_dep __((ldr_module_handle handle, int depno,
				      char **dep));
extern int preload_get_imports __((ldr_module_handle handle, int *pkg_count,
				   ldr_package_rec **pkgs, int *sym_count,
				   ldr_symbol_rec **imports));
extern int preload_map_regions __((ldr_module_handle handle, ldr_region_allocs *allocsp,
				   int *reg_count, ldr_region_rec **regions));
extern int preload_get_export_pkgs __((ldr_module_handle handle, int *count,
				       ldr_package_rec **packages));
extern int preload_get_exports __((ldr_module_handle handle, int *sym_count,
				   ldr_symbol_rec **exports));
extern int preload_lookup_export __((ldr_module_handle handle, ldr_package_rec *package,
				     ldr_symbol_rec *symbol));
extern int preload_relocate __((ldr_module_handle handle, int nregions, ldr_region_rec *regions,
				int npackages, ldr_package_rec *import_pkgs, int nimports,
				ldr_symbol_rec *imports));
extern int preload_get_entry_pt __((ldr_module_handle handle, ldr_entry_pt_t *entry_pt));
extern int preload_run_inits __((ldr_module_handle handle, entry_pt_kind kind));
extern int preload_cleanup __((ldr_module_handle handle));
extern int preload_unload __((ldr_module_handle handle, ldr_region_allocs *allocsp,
			      int reg_count, ldr_region_rec *regions,
			      int ipkg_count, ldr_package_rec *import_pkgs,
			      int import_count, ldr_symbol_rec *imports,
			      int epkg_count, ldr_package_rec *export_pkgs));


#endif	/* _H_LDR_PRELOAD_MGR */
