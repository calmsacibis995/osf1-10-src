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
 *	@(#)$RCSfile: ldr_kernel_machdep.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:58 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/* Note that this include file allocates static storage, and hence must
 * only be included by ldr_kernel_bootstrap.c
 */

#ifdef KLS_MACHO_MGR
/* OSF/Mach-O */
int kls_macho_recog(const char *filename, ldr_file_t fd,
		   ldr_module_handle *handle);
int kls_macho_map_regions(ldr_module_handle handle, ldr_region_allocs *allocsp,
			 int *reg_count, ldr_region_rec **regions);
int kls_macho_get_export_pkgs(ldr_module_handle handle, int *count,
			     ldr_package_rec **packages);
int kls_macho_get_exports(ldr_module_handle handle, int *countp,
			  ldr_symbol_rec **exports);
int kls_macho_lookup_export(ldr_module_handle handle,
			    ldr_package_rec *package, ldr_symbol_rec *symbol);
int kls_macho_get_entry_pt(ldr_module_handle handle, ldr_entry_pt_t *entry_pt);
#endif

#ifdef KLS_COFF_MGR
/* COFF */
int kls_coff_recog(const char *filename, ldr_file_t fd,
		   ldr_module_handle *handle);
int kls_coff_map_regions(ldr_module_handle handle, ldr_region_allocs *allocsp,
			 int *reg_count, ldr_region_rec **regions);
int kls_coff_get_export_pkgs(ldr_module_handle handle, int *count,
			     ldr_package_rec **packages);
int kls_coff_get_exports(ldr_module_handle handle, int *countp,
			 ldr_symbol_rec **exports);
int kls_coff_lookup_export(ldr_module_handle handle,
			   ldr_package_rec *package, ldr_symbol_rec *symbol);
int kls_coff_get_entry_pt(ldr_module_handle handle, ldr_entry_pt_t *entry_pt);
#endif

/* This is the fake kernel loader switch entry used for the loader's module
 * record.  Only the minimal set of required operations are supplied.
 */ 

static struct loader_switch_entry kernel_loader_switch[] = {
#ifdef KLS_COFF_MGR
    {
	LSW_VERSION,			/* structure version */
	LSF_MUSTOPEN,			/* open required */
	kls_coff_recog,			/* recognizer */
	get_static_dep,			/* get_static_dep */
	get_imports,			/* get_imports */
	kls_coff_map_regions,		/* map_regions */
	kls_coff_get_export_pkgs,	/* get export packages */
	kls_coff_get_exports,		/* get exports */
	kls_coff_lookup_export,		/* lookup export */
	relocate,			/* relocate */
	kls_coff_get_entry_pt,		/* get entry */
	NULL,				/* no inits */
	cleanup,			/* cleanup */
	NULL				/* unload; can't be called */
    },
#endif
#ifdef KLS_MACHO_MGR
    {
	LSW_VERSION,			/* structure version */
	LSF_MUSTOPEN,			/* open required */
	kls_macho_recog,		/* recognizer */
	get_static_dep,			/* get_static_dep */
	get_imports,			/* get_imports */
	kls_macho_map_regions,		/* map_regions */
	kls_macho_get_export_pkgs,	/* get export packages */
	kls_macho_get_exports,		/* get exports */
	kls_macho_lookup_export,	/* lookup export */
	relocate,			/* relocate */
	kls_macho_get_entry_pt,		/* get entry */
	NULL,				/* no inits */
	cleanup,			/* cleanup */
	NULL				/* unload; can't be called */
    }
#endif
};

int n_kernel_lsw_entries = 
	sizeof(kernel_loader_switch)/sizeof(kernel_loader_switch[0]);
