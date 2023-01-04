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
static char	*sccsid = "@(#)$RCSfile: mo_ldr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:41:54 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * mo_ldr.c
 *
 * Macho format dependent (FD) loader.
 *
 * OSF/1 Release 1.0
 */

/* #define DEBUG */

#ifdef DEBUG
#define	dprintf(x)	ldr_msg x
#else
#define	dprintf(x)
#endif /* DEBUG */

#include <sys/types.h>
#include <sys/param.h>
#include <loader.h>
#include <math.h>
#include <strings.h>

#include "ldr_types.h"
#include "ldr_sys_int.h"
#include "ldr_windows.h"
#include "ldr_malloc.h"
#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_switch.h"
#include "ldr_errno.h"
#include "ldr_hash.h"

#include <mach_o_header.h>
#include <mach_o_format.h>

#include "open_hash.h"
#include "mo_ldr.h"

#define	round(x, s)	(((unsigned)(x) + (unsigned)(s) - 1) & ~((unsigned)(s) - 1))
#define	trunc(x, s)	(((unsigned)(x)) & ~((unsigned)(s) - 1))
#define aligned(x, s)	(((unsigned)(x) & ((unsigned)(s) - 1)) == 0)

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#define	MO_LDR_FIRST_REGION_ID			0
#define MO_LDR_INVALID_REGION_ID		-1

#define MO_LDR_INVALID_LC_ENTRY			NULL

#define MO_NO_EXPORTS				-1 /* module has no exports */


/* declarations for local routines */

static int macho_recognizer(const char *filename, ldr_file_t fd,
			    ldr_module_handle *handle_fi);

static int macho_get_static_dep(ldr_module_handle handle, int depno,
				char **dep);

static int macho_get_imports(ldr_module_handle handle, int *pkg_count,
		     ldr_package_rec **pkgs, int *sym_count,
		     ldr_symbol_rec **imports);

static int macho_map_regions(ldr_module_handle handle_fi,
			     ldr_region_allocs *allocps, int *reg_count, 
			     ldr_region_rec **regions);

static int macho_get_export_pkgs(ldr_module_handle handle, int *count,
				 ldr_package_rec **packages);

static int macho_get_exports(ldr_module_handle handle,int *sym_count,
			     ldr_symbol_rec **exports);

static int macho_lookup_export(ldr_module_handle handle_fi,
			       ldr_package_rec *package,
			       ldr_symbol_rec *symbol);

static int macho_relocate(ldr_module_handle handle, int nregions,
			  ldr_region_rec *regions, int npackages,
			  ldr_package_rec *import_pkgs, int nimports,
			  ldr_symbol_rec *imports);

static int macho_get_entry_pt(ldr_module_handle handle_fi,
			      ldr_entry_pt_t *entry_pt);

static int macho_run_inits(ldr_module_handle handle, entry_pt_kind kind);

static int macho_cleanup(ldr_module_handle handle_fi);

static int macho_unload(ldr_module_handle handle_fi, 
			ldr_region_allocs *allocsp,
			int reg_count, ldr_region_rec *regions,
			int ipkg_count, ldr_package_rec *import_pkgs,
			int import_count, ldr_symbol_rec *imports,
			int epkg_count, ldr_package_rec *export_pkgs);

static int 
window(ldr_window_t **wp, ldr_file_t fd, unsigned long start, 
       size_t size, univ_t *windowed_data);

static int
build_lc_table(macho_module_handle_ptr_t handle, load_cmd_map_command_t *lc_map,
	       univ_t all_lc_p);

static int
cache_lc_table_entry(macho_module_handle_ptr_t handle, ldc_header_t *lc_hd_p,
		       mo_lc_entry_t *cur_lc_entry);
static int
build_region_table(macho_module_handle_ptr_t handle);

static int
run_funcs(mo_addr_t *init_entry_loc, mo_short_t nentries, 
	  macho_module_handle_ptr_t handle);

static int
internal_map_region(macho_module_handle_ptr_t handle,
		    int regno, ldr_region_allocs *allocps,
		    ldr_region_rec *region_list, size_t pagesize);

static int
internal_free_regions(macho_module_handle_ptr_t handle, int nregions, 
		      ldr_region_rec *regions, ldr_region_allocs *allocps);

static int
internal_unload_region(macho_module_handle_ptr_t handle, int regno,
		       ldr_region_rec *region, dealloc_region_p deallocp);

static int
internal_free_pkg_import_recs(macho_module_handle_ptr_t handle,
			      int ipkg_count, ldr_package_rec *import_pkgs,
			      int import_count, ldr_symbol_rec *imports,
			      int epkg_count, ldr_package_rec *export_pkgs);

static int 
internal_finish_unload(macho_module_handle_ptr_t handle);

static int
free_export_list(macho_module_handle_ptr_t handle);

static int
build_export_list(macho_module_handle_ptr_t handle);

static int
get_sym_pkg_info(macho_module_handle_ptr_t handle, int index, 
		 char **pkgname, char **symname, ldr_symval *symvalue,
		 int *pkgno);

static int
internal_relocate_region(macho_module_handle_ptr_t handle, 
			 ldr_region_rec *regions, ldr_region_rec *reg,
			 int npackages, ldr_package_rec *import_pkgs, 
			 int nimports, ldr_symbol_rec *imports,
			 mo_lc_entry_t *region_entry, 
			 mo_lc_entry_t *reloc_entry);
static int
reloc_info(macho_module_handle_ptr_t handle, reloc_info_t *reloc_p,
	   int *reloc_type, int *lc_id, int *offset_index, int *reg_id,
	   univ_t *regvaddr);

static int
get_relocation(macho_module_handle_ptr_t handle, reloc_info_t *reloc_p, 
	       reloc_command_t *reloc_lc, ldr_symbol_rec *imports, 
	       ldr_region_rec *reg, mo_long_t *relocation);

static int
internal_cleanup(macho_module_handle_ptr_t handle);

static int
build_internal_term_funcs(macho_module_handle_ptr_t handle, ldc_header_t *lc_hd_p);

static int
run_term_funcs(macho_module_handle_ptr_t handle);

static int 
run_init_funcs(macho_module_handle_ptr_t handle);


#ifdef DEBUG

void 
print_lc_map(load_cmd_map_command_t *p);

void 
print_hdr(mo_header_t *p);

void 
print_lc_table(mo_lc_entry_t *lc_table, int size);

void 
print_region_table(mo_region_entry_t *region_table, int size);

void 
print_handle(macho_module_handle_ptr_t p);

void 
print_handle_and_state(void *handle_fi);

#endif


/* The macho loader switch entry */

const struct loader_switch_entry macho_switch_entry = {
	LSW_VERSION,
	LSF_MUSTOPEN,
	macho_recognizer,
	macho_get_static_dep,
	macho_get_imports,
	macho_map_regions,
	macho_get_export_pkgs,
	macho_get_exports,
	macho_lookup_export,
	macho_relocate,
	macho_get_entry_pt,
	macho_run_inits,
	macho_cleanup,
	macho_unload,
};


int
ldr_macho_entry(ldr_context_t ctxt)

/* The manager entry point is called with a pointer to a loader context.
 * It is responsible for linking its switch entry into the context's
 * switch (by calling ldr_switch_ins_tail()).  This procedure
 * allows dynamically-loaded auxiliary managers to be initialized in
 * the same way as statically-linked managers.
 */
{
	return(ldr_switch_ins_tail(ctxt, (ldr_switch_t)&macho_switch_entry));
}


/*	
 *	The recognizer routine checks to see whether the specified file
 *	(opened for at least read access on file descriptor fd)
 *	is of an object file format supported by this format-dependent
 *	manager.  It returns LDR_SUCCESS on success or a negative loader error
 *	status on failure.  On success, the format-dependent manager's handle
 *	is left in the handle variable.  Also, after a successful recognition,
 *	the open file descriptor is the resposibility of the format-dependent
 *	manager; it is never used again by the format-independent manager.
 */

static int
macho_recognizer(const char *filename, ldr_file_t fd,
		 ldr_module_handle *handle_fi)
{
	ldr_window_t *hd_w;			/* header window */
	ldr_window_t *lc_w;			/* load cmds window */
	macho_module_handle_ptr_t handle;	/* FD manager's handle */
	univ_t raw_hd_p;			/* ptr to raw macho header */
	mo_header_t *hd_p;			/* ptr to macho header */
	univ_t lc_p;				/* ptr to load cmds */
	univ_t adjusted_lc_p;			/* lc_p adjusted for start of mapping */
	load_cmd_map_command_t *lc_map_p;	/* ptr to load cmd map */
	int rc;					/* return code/status */


	/*
	 * decode Mach-O header
	 */

	/* window raw (unconverted byte order) file header */
	if (rc = window(&hd_w, fd, 0, MO_SIZEOF_RAW_HDR, (univ_t *)&raw_hd_p) 
	    != LDR_SUCCESS) return rc;

	/* allocate buffer for converted macho header (correct byte order) */
	rc = ldr_malloc(sizeof(mo_header_t), LDR_MACHO_T, (univ_t*) &hd_p);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "allocation of macho header failed\n", rc);

	rc = decode_mach_o_hdr(raw_hd_p, MO_SIZEOF_RAW_HDR, 
			       MOH_HEADER_VERSION, hd_p);
	if (rc == MO_HDR_CONV_SUCCESS) {

		dprintf(("macho_recognizer : macho file = TRUE\n"));

		/* should never need the raw header again */ 
		ldr_unwindow(hd_w);

		/* we need a ldr_calloc() here ? */
		rc = ldr_malloc(sizeof(struct macho_module_handle_t), LDR_MACHO_T,
				    (univ_t*)&handle);
		if (rc != LDR_SUCCESS) {
			dprintf(("macho_recognizer: failure to allocate handle\n"));
			ldr_free(hd_p);
			return rc;
		}
		bzero((char*) handle, sizeof(struct macho_module_handle_t));
		
		/* window *all* LCs in a single window */
		if (rc = window(&lc_w, fd, hd_p->moh_first_cmd_off,
				hd_p->moh_sizeofcmds, (univ_t *)&lc_p)
		    != LDR_SUCCESS) {
			ldr_free(hd_p);
			return rc;
		}
		handle->load_cmd_w = lc_w;	/* save ptr to window */


		/* locate load cmd map and build load command table */
		lc_map_p = (load_cmd_map_command_t *)
			((char*)lc_p + (hd_p->moh_load_map_cmd_off - 
					hd_p->moh_first_cmd_off));

		/* hack alert ! */
		/* adjust lc_p as we mapped file starting at first LC offset */
		adjusted_lc_p = ((char *)lc_p - hd_p->moh_first_cmd_off);

		/* build_lc_table() also caches ptrs to LC table entries in handle */
		rc = build_lc_table(handle, lc_map_p, adjusted_lc_p);

		if (rc !=  LDR_SUCCESS) {
			dprintf(("macho_recognizer: failure to build load cmd table\n"));
			ldr_free(hd_p);
			return rc;
		}

		/* 
		 * note: demand driven - we don't build FD region table until 
		 * get_region_count().
		 */

		/* never need the macho header */
		ldr_free(hd_p);

		/* module successfully recognized, fill in fields of handle */
		handle->fd = fd;

		/* fill in Format independent handle */
		(*handle_fi) = (ldr_module_handle) handle;

		dprintf(("macho_recognizer: macho file successfully recognized\n"));

		return LDR_SUCCESS;
	}

	dprintf(("macho_recognizer: not a macho file rc = %d\n", rc));

	/* 
	 * It is a not a mach-O file so cleanup the raw mach-O header and
	 * memory allocated for a converted Mach-O header.
	 */
	ldr_unwindow(hd_w);
	ldr_free(hd_p);
	return LDR_ENOEXEC;
}

/*
 *	window : window in data from a file given a ptr to a window,
 *	file handle, start position in file and size of window in
 *	bytes.  The output argument window_data must be allocated
 *	by caller and will contain on success a pointer to the
 *	windowed data. Return code is LDR_SUCCESS on success.
 */
static int
window(ldr_window_t **wp, ldr_file_t fd, unsigned long start, 
       size_t size, univ_t *windowed_data)
{
	*wp = ldr_init_window(fd);
	MO_ASSERT((int)*wp, (int)NULL, "failure to initialize window", LDR_ENOMEM);
	(*windowed_data) = ldr_file_window(start, size, *wp);
#ifdef undef
	dprintf(("window: *windowed_data : 0x%x start 0x%x size 0x%x\n", 
		 *windowed_data, start, size));
#endif
	MO_ASSERT((int)(*windowed_data), (int)NULL, "failure to window", LDR_ENOMEM);
	return LDR_SUCCESS;
}

/*
 *	build_lc_table : builds the load command table for a macho
 *	loader module handle given a pointer to the load command
 *	map.  As a side effect, the no of load commands entered in
 *	the handle (field no_load_cmds).
 *	Another, side effect is that all the mo_lc_entry_t *'s in the handle
 *	that need to be cached are filled in. (These are ptrs to LC table
 *	entries such as the LC entry for the "string_cmd", "import_cmd"
 *	etc. ) See cache_lc_table_entry for details.
 */
static int
build_lc_table(macho_module_handle_ptr_t handle, load_cmd_map_command_t *lc_map,
	       univ_t all_lc_p)
{
	mo_lc_entry_t	*p;		/* ptr to load command table entries */
	ldc_header_t	*lc_hd_p;	/* ptr to LC header */
	int		i;		/* loop variable */
	int		rc;		/* return code/status */
	int		regno = MO_LDR_FIRST_REGION_ID;


	rc = ldr_malloc((lc_map->lcm_nentries * sizeof(mo_lc_entry_t)), 
			LDR_MACHO_T, (univ_t*) &(handle->load_cmd_table));
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "build_lc_table: failure to allocate LC table", rc);


	p = handle->load_cmd_table;

	/* fill in load command table entries from data in load cmd map */

	for (i = 0; i < lc_map->lcm_nentries; i++, p++) {

		lc_hd_p = (ldc_header_t *) 
			((char*)all_lc_p + lc_map->lcm_map[i]); 
		p->LC = (void *) lc_hd_p;

		p->section_data = NULL;	/* retrieved on demand */
		p->section_window = NULL;

		if (lc_hd_p->ldci_cmd_type == LDC_REGION) {
			p->region_id = regno++;
		}

		/* cache in handle, ptrs to certain LC table entries */
		rc = cache_lc_table_entry(handle, lc_hd_p, p);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, "build_lc_table: error in cache_lc_table_entry", rc);
	}


	/* In the LC table we have built, the LC map is never used
	 * but is left as a place holder. Hence, no_load_cmds in LC table
	 * equal those in the LC map.
	 */
	handle->no_load_cmds = lc_map->lcm_nentries; 

	return LDR_SUCCESS;
}

/*
 *	cache_lc_table_entry : this routine caches the LC table entry 
 *	in the macho handle if it is one of the special LC types (see
 *	code and macho_module_handle_t defn for such LC's).
 */
static int
cache_lc_table_entry(macho_module_handle_ptr_t handle, ldc_header_t *lc_hd_p,
		       mo_lc_entry_t *cur_lc_entry)
{
	switch (lc_hd_p->ldci_cmd_type) {
	            case LDC_UNDEFINED : /* throw away undefined load cmds */
			    dprintf(("cache_lc_table_entry: type LDC_UNDEFINED %d\n",
				    LDC_UNDEFINED));
			    break;
		    case LDC_STRINGS : /* NOTE: add meta string */
			    handle->string_cmd = cur_lc_entry;
			    cur_lc_entry->type = MO_STRINGS;
			    break;
		    case LDC_REGION : 	
			    cur_lc_entry->type = MO_REGION;
			    break;
		    case LDC_RELOC:
			    cur_lc_entry->type = MO_RELOC;
			    break;
		    case LDC_PACKAGE :
			    if (((package_command_t *)lc_hd_p)->pkgc_flags & 
				PKG_EXPORT_F) {
				    handle->export_package_cmd = cur_lc_entry;
				    cur_lc_entry->type = MO_PACKAGE;
			    }
			    else if (((package_command_t *)lc_hd_p)->pkgc_flags & 
				     PKG_IMPORT_F) {
				    handle->import_package_cmd = cur_lc_entry;
				    cur_lc_entry->type = MO_PACKAGE;
			    }
			    break;
		    case LDC_SYMBOLS :
			    if (((symbols_command_t *)lc_hd_p)->symc_kind 
				== SYMC_IMPORTS) {
				    handle->import_cmd = cur_lc_entry;
				    cur_lc_entry->type = MO_SYMBOLS;
			    }
			    else if (((symbols_command_t *)lc_hd_p)->symc_kind 
				     == SYMC_DEFINED_SYMBOLS) {
				    handle->def_symb_cmd = cur_lc_entry;
				    cur_lc_entry->type = MO_SYMBOLS;
			    }
			    break;
		    case LDC_CMD_MAP :
			    cur_lc_entry->type = MO_CMD_MAP;
			    cur_lc_entry->LC = NULL; /* don't use the LC map */
			    break;

		    case LDC_ENTRY :
			    handle->entry_pt_cmd = cur_lc_entry;
			    cur_lc_entry->type = MO_ENTRY;
			    break;

		    case MO_FUNC_TABLE :
			    cur_lc_entry->type = MO_FUNC_TABLE;
			    build_internal_term_funcs(handle, lc_hd_p);
			    break;

		    case MO_GEN_INFO :
			    cur_lc_entry->type = MO_GEN_INFO;
			    break;

		    default :
			    dprintf(("cache_lc_table_entry: default case : LC type : %d\n",
				    lc_hd_p->ldci_cmd_type));
			    break;
		    }


	return LDR_SUCCESS;
}

/*
 *	build_term_funcs_table: build a table of termination
 *	functions. The reason we do this is that
 *	termination routines need to be run after macho_cleanup
 *	at which point we no longer have the load commands mapped.
 */
static int
build_internal_term_funcs(macho_module_handle_ptr_t handle, ldc_header_t *lc_hd_p)
{
	func_table_command_t *ft_lc = (func_table_command_t *)  lc_hd_p;
	mo_addr_t *to;
	mo_addr_t *from;
	int rc;
	int i;

	/* only save termination routine entry points */
	if (ft_lc->fntc_type != FNTC_TERMINATION) return LDR_SUCCESS;

	if (handle->term_funcs == NULL) {
		rc = ldr_malloc((ft_lc->fntc_nentries * sizeof(mo_addr_t)), LDR_MACHO_T,
				(univ_t *) &handle->term_funcs);
		if (rc != LDR_SUCCESS) return rc;
		/* a bcopy would be more efficient ! */
		for (i = 0, to = handle->term_funcs, 
		     from = &ft_lc->fntc_entry_loc[0]; i < ft_lc->fntc_nentries; i++) 
			*to++ = *from++;
		handle->term_funcs_nentries = ft_lc->fntc_nentries;
	}
	else {
		ldr_msg("multiple termination load cmds not supported, ignoring all but first\n");
	}

	return LDR_SUCCESS;
}

/*
 *	build_region_table : build the region table for a loader module
 *	handle provided the load command table for that handle exists.
 *
 *	side effects :
 *		handle->no_regions 
 *		handle->region_table
 */
static int
build_region_table(macho_module_handle_ptr_t handle)
{
	mo_region_entry_t *rt_p;	/* ptr to region table entries */
	mo_lc_entry_t	*lc_p;		/* ptr to load cmd table entries */
	int		rcount;		/* count of the no. of regions */
	int		rc;		/* return code/status */
	int		i;		/* loop variable */

	MO_ASSERT((int)handle->load_cmd_table, (int)NULL, 
		      "build_region_table: no load cmd table",LDR_ENOEXEC);

	/* 
	 * safe upper bound on no of region LC's is total no of LC's 
	 * excluding the LC map. This is the max no of entries in region
	 * table.
	 */
	rc = ldr_malloc(((handle->no_load_cmds - 1) * sizeof(mo_region_entry_t)), 
			LDR_MACHO_T, (univ_t *) &rt_p);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, 
		      "build_region_table: alloc region table failed\n", rc);

	/* save ptr to region table in handle before rt_p is modified */
	handle->region_table = rt_p;

	lc_p = handle->load_cmd_table;
	rcount = 0;

	for (i = 0; i < handle->no_load_cmds; i++) {
		if (lc_p->type == MO_REGION) {
			rt_p->LC_entry = lc_p;
			lc_p->region_id = rcount;  /* fixup load_cmd_table */
			rt_p++; rcount++;
		}
		lc_p++;
	}

	/* region table successfully built, return values in handle */
	/* region_table stored in handle previously as rt_p is a temp */
	handle->no_regions = rcount;

	return LDR_SUCCESS;
}


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

static int
macho_map_regions(ldr_module_handle handle_fi,
		 ldr_region_allocs *allocps, int *reg_count, 
		 ldr_region_rec **regions)
{
	size_t pagesize;		/* VM page size */
	ldr_region_rec *reg_list;	/* regions list */
	int regcount;			/* no of regions */
	int regno;			/* region number */
	int rc;				/* return code */
	mo_lc_entry_t *entry_p;		/* load command table entry ptr */
	univ_t reg_vaddr;		/* region virtual address for entry pt */
	entry_command_t *entry_lc;	/* load command ptr */
	int abs_flag;			/* TRUE if entry point is absolute, else FALSE */
	int entry_reg_id;		/* region number of region containing entry pt. */
	macho_module_handle_ptr_t handle/* switching types across interface */
		= (macho_module_handle_ptr_t) handle_fi;


	dprintf(("macho_map_regions : entered\n"));

	CHECK_MACHO_HANDLE(handle);

	/*
	 * We build the FD region table here when the region
	 * count is needed.  The region table is needed when
	 * mapping regions in internal_macho_map_region().
	 */

	rc = build_region_table(handle);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_get_region_count: building region table failed", rc);

	dprintf(("macho_map_regions : region table built\n"));

	pagesize = ldr_getpagesize();	/* virtual memory page size */

	regcount = handle->no_regions;

	/* Allocate the region list */

	if ((rc = ldr_regions_create(handle->no_regions, LDR_REGION_VERSION,
				     &reg_list)) != LDR_SUCCESS) {

		dprintf(("macho_map_regions: region create error %d\n", rc));
		return rc;
	}

	dprintf(("macho_map_regions: regions created\n"));

	/* Now map the regions */

	for (regno = 0; regno < handle->no_regions; regno++) {
		
		if ((rc = internal_map_region(handle, regno, allocps,
					      reg_list, pagesize)) != LDR_SUCCESS) {

			dprintf(("macho_map_regions: failure to map region %d\n",
				 regno));

			/* number of regions loaded is 1 greater than regno */
			(void)internal_free_regions(handle, regno, 
						    reg_list, allocps);
			return rc;
		}
	}

	/* all regions mapped successfully, fill in return results */

	*reg_count = regcount;
	*regions = reg_list;

	dprintf(("macho_map_regions : mapped all %d regions\n",*reg_count));

	/* Fix ! find the entry point at this time and cache in handle 
	 * this is necessary as the interface to macho_get_entry_pt is
	 * broken. It needs to pass in a linked list of ldr_region_rec's.
	 */
	entry_p = handle->entry_pt_cmd;
	if (entry_p != NULL) {
		entry_lc = (entry_command_t *) entry_p->LC;
		abs_flag = (entry_lc->entc_flags & ENT_VALID_ABSADDR_F)
			? TRUE : FALSE;
		if (abs_flag == TRUE) {
			dprintf(("macho_map_regions: absolute entry pt\n"));
			handle->entry_pt = (ldr_entry_pt_t) entry_lc->entc_absaddr;
		}
		else {
			CHECK_LC_ID(handle, (entry_lc->entc_entry_pt.adr_lcid));
			entry_reg_id = LC_TO_REGNO((handle->load_cmd_table + 
						   (entry_lc->entc_entry_pt.adr_lcid)));
			dprintf(("macho_map_regions: non abs entry pt, reg_id : %d\n",
				 entry_reg_id));
			reg_vaddr = reg_list[entry_reg_id].lr_vaddr;
			handle->entry_pt = (ldr_entry_pt_t) 
				((char *)reg_vaddr + (entry_lc->entc_entry_pt.adr_sctoff));
		}
		dprintf(("macho_map_regions : entry_pt : 0x%x\n", handle->entry_pt));
	}
	else {
		dprintf(("macho_map_regions : module has no main_entry LC \n"));
	}

	return LDR_SUCCESS;
}
/*
 * internal_map_region : map a region into the process's address space.
 * NOTE 1 : we currently cannot load regions which have non zero section
 * lengths in the file and the vmsize is not equal to the section length.
 * This case, implies that the region needs to be mapped partly from the 
 * file and partly from anonymous memory.  If this is needed, we can map
 * the pages from the file and map anon, fixed the additinal pages
 * so that although we use two calls to mmap the region is contiguous
 * in the process's address space.  This is important as the region 
 * could have relocation associated with it.
 * NOTE 2 : for PC relative code, the loader loads regions that are
 * marked REG_REL_ADDR_F as absolute regions at abs addresses equal
 * to the offset from the previously loaded region it is relative to.
 * We use a very simple algorithm so it is likely to fail when attempting
 * to load many such regions.  We will then need to do something appropriate.
 */

static int
internal_map_region(macho_module_handle_ptr_t handle,
		    int regno, ldr_region_allocs *allocps,
		    ldr_region_rec *region_list, size_t pagesize)
{
	mo_lc_entry_t *reg_entry;	/* region LC table entry */
	region_command_t *reg_lc;	/* ptr to region load cmd */
	univ_t vaddr;			/* virtual addr at which reg will live */
	ldr_prot_t protection;		/* region protection */
	int region_prot;		/* protection used for addr map allocator */
	size_t size;			/* region size */
	univ_t baseaddr;		/* base address for mapping */
	univ_t mapaddr;			/* address region got mapped at */
	int off;			/* offset within file - for mmap */
	int mapflags;			/* flags for ldr_mmap */
	ldr_file_t fd;			/* file descriptor for mmap */
	int string_id;			/* string load cmd index */
	mo_lc_entry_t *string_entry;	/* string LC table entry */
	ldr_region_rec *region;		/* rec for region being loaded */
	ldr_region_rec *rel_reg_rec;	/* region record for previously loaded region */
	mo_lc_entry_t *rel_reg_entry;	/* LC table entry for region relative to which we are loading */
	int rel_reg_lc_id;		/* load cmd id of previously loaded region */
	int rel_reg_id;			/* region id of prev loaded relative region */
	int abs_flag;			/* TRUE - absoulte reg; FALSE - reloc reg */
	int anon_flag;			/* TRUE - map region anon, FALSE - map from file */
	int rc;				/* return code */


	region = region_list + regno;

	/* find region to be loaded in region table and the region load cmd */
	reg_entry = REGION_LCTABLE_ENTRY(handle->region_table, regno);
	reg_lc = (region_command_t *) LOAD_CMD(reg_entry);

	/* 
	 * does not handle the case when we have to map a region
	 * partly from the file and zero fill the rest i.e map
	 * anon. See comment above as to how we might support this
	 * if we need it.
	 */

	if ((reg_lc->regc_vm_size != reg_lc->ldc_section_len) &&
	    (reg_lc->ldc_section_len != 0)) {
		ldr_msg("loader: error cannot map a region partly from file and zero fill rest\n");
		return LDR_ENOEXEC;
	}


	/* should we map this region from anonymous memory ? */
	if (reg_lc->regc_vm_size != reg_lc->ldc_section_len) {
		anon_flag = TRUE;
	}

	if (!aligned(reg_lc->ldc_section_off, pagesize)) {
		dprintf(("internal_map_region: section off 0x%x not page aligned\n",
			 reg_lc->ldc_section_off));
		return LDR_ENOEXEC;
	}

	size =  round(reg_lc->regc_vm_size, ldr_getpagesize());
	
	protection = LDR_PROT_NONE;
	if (reg_lc->regc_initprot & MO_PROT_READ) {
		protection |= LDR_PROT_READ;
		region_prot |= LDR_R;
	}
	if (reg_lc->regc_initprot & MO_PROT_WRITE) {
		protection |= LDR_PROT_WRITE;
		region_prot |= LDR_W;
	}
	if (reg_lc->regc_initprot & MO_PROT_EXECUTE) {
		protection |= LDR_PROT_EXEC;
		region_prot |= LDR_X;
	}

	if (anon_flag == TRUE) {
		off = 0;
		mapflags = LDR_MAP_ANON;
		fd = LDR_FILE_NONE;
	}
	else {
		off = reg_lc->ldc_section_off;
		mapflags = LDR_MAP_FILE;
		fd = handle->fd;
	}

	/* save region size in FD region table - get rid off later ??? */
	((mo_region_entry_t*)
	 (handle->region_table + regno))->region_size = size;
	
	/* 
	 * regions that need to be mapped at fixed offset's from other
	 * regions are treated as absolute regions which are to be
	 * mapped at the virtual address (vaddr of previously mapped region
	 * + offset for region being mapped).
	 */
	/* region absolute or relocatable ? Relative regions are always abs */
	abs_flag = ((reg_lc->regc_flags & REG_ABS_ADDR_F) || 
		    (reg_lc->regc_flags & REG_REL_ADDR_F)) ? TRUE : FALSE;

	/* 
	 * compute addresses at which region should most likely be mapped
	 * and at which it will finally live.
	 */
	if (abs_flag == TRUE) {		/* absolute region */
		dprintf(("internal_map_region: absolute region\n"));
		if (reg_lc->regc_flags & REG_REL_ADDR_F) {
			rel_reg_lc_id = reg_lc->regc_rel_addr.adrl_lcid;
			rel_reg_entry = handle->load_cmd_table + rel_reg_lc_id;
			rel_reg_id = rel_reg_entry->region_id;
			rel_reg_rec = region_list + rel_reg_id;
			dprintf(("internal_map_region: rel_reg_rec->lr_vaddr : 0x%x\n",
				 rel_reg_rec->lr_vaddr));
			vaddr = (univ_t)((char *)rel_reg_rec->lr_vaddr + 
				(int)(reg_lc->regc_rel_addr.adrl_reloff));
			dprintf(("internal_map_region: rel region id %d reloff 0x%x vaddr 0x%x\n",
				 rel_reg_id, reg_lc->regc_rel_addr.adrl_reloff, vaddr));
		}
		else {
			vaddr = reg_lc->regc_vm_addr;
		}
		if (!aligned(vaddr, pagesize)) {
			dprintf(("internal_map_region: vaddr : 0x%x not page aligned for abs region\n",
				 vaddr));
			return LDR_ENOEXEC;
		}

		rc = (*(allocps->lra_abs_alloc)) 
			((univ_t) vaddr, (size_t) size, protection,
			 &baseaddr);
		if (rc != LDR_SUCCESS) {
			dprintf(("internal_map_region : failure to get baseaddr from abs allocator\n"));
			return rc;
		}
		
		mapflags |= (baseaddr == vaddr) ? 
			(LDR_MAP_FIXED | LDR_MAP_PRIVATE) : (LDR_MAP_PRIVATE);
	}
	else {				/* relocatabe region */
		dprintf(("internal_map_region: relocatable region\n"));

		rc = (*(allocps->lra_rel_alloc)) (size, protection, 
						  &vaddr, &baseaddr);
		if (rc != LDR_SUCCESS) {
			dprintf(("internal_map_region : failure to get baseaddr for rel region\n"));
			return rc;
		}
		mapflags |= LDR_MAP_PRIVATE;
	}

	/* 
	 * mmap region if it is not a zero length region.  It is too late
	 * to do anything else at this time but, we can't actually map it.
	 */
	if (size > 0) {
		dprintf(("internal_map_region: baseaddr : 0x%x size : 0x%x \n",
			 baseaddr, size));
		dprintf(("protection : 0x%x mapflags : 0x%x fd : 0x%x off : 0x%x\n",
			 protection,  mapflags, fd, off));

		rc = ldr_mmap(baseaddr, size, protection, mapflags, fd,
			      off, &mapaddr);
		if (rc != LDR_SUCCESS) {
			dprintf(("internal_map_region: failure to map region rc : %d\n", rc));
			return rc;
		}

		reg_entry->section_data = mapaddr;

		if (vaddr == NULL) {	/* only happens for relocatable regs */
			vaddr = mapaddr;
		}
	}

	/* 
	 * Region mapped succesfully; return info in the region record.
	 */

	string_id = reg_lc->regc_region_name.adr_lcid;
	string_entry = handle->load_cmd_table + string_id;
	rc = window_in_section(handle, string_entry);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_map_region: failure to window string sect\n"));
		return rc;
	}
	region->lr_name = 
		ldr_strdup(get_mo_ldr_addr(&(reg_lc->regc_region_name),handle));
	region->lr_vaddr = vaddr;
	region->lr_mapaddr = mapaddr;
	region->lr_size = size;
	region->lr_prot = protection;

	dprintf(("internal_map_region : region %d mapped, mapaddr : 0x%x vaddr : 0x%x\n",
		 regno, region->lr_mapaddr, region->lr_vaddr));

	return LDR_SUCCESS;
}



static int
internal_free_regions(macho_module_handle_ptr_t handle, int nregions,
		      ldr_region_rec *regions, ldr_region_allocs *allocps)

/* Unmap the first nregions regions described by the specified list of
 * region records.  Don't give up if one unmap fails, but continue on
 * and unmap as many of the regions as possible.  Then free the
 * list of region records.  Note that nregions may be less than the
 * size of the region list, if we're being called as a result of a
 * map_regions error.
 */
{
	int		regno;
	int		rc, rrc;

	rc = LDR_SUCCESS;

	for (regno = 0; regno < nregions; regno++) {
		rrc = internal_unload_region(handle, regno, 
					     &regions[regno], 
					     allocps->lra_dealloc);
		if (rc == LDR_SUCCESS) rc = rrc;
		if (regions[regno].lr_name != NULL)
			(void)ldr_free(regions[regno].lr_name);
	}

	rrc = ldr_regions_free(nregions, regions);
	if (rc == LDR_SUCCESS) rc = rrc;
	return(rc);
}


/* Iterator returing the pathnames of the static dependencies of the object
 * module with the specified format-dependent handle.  depno is the index of
 * the dependency to be found, starting at zero.  Return pointer to pathname
 * of static dependency (as a ldr_strdup'ed string; caller will ldr_free it)
 * in *dep.  Returns LDR_SUCCESS on success, a negative loader error
 * status on error (including LDR_EAGAIN to indicate the end of the
 * dependencies). 
 */
/*
 * NOTE: not meaningful for Mach-O
 */
static int macho_get_static_dep(ldr_module_handle handle, int depno,
			char **dep)
{
	return LDR_EAGAIN;
}

/*
 * Return the address of the entry point for the specified module, if
 * any, in *entry_pt.  Return LDR_SUCCESS on success or negative error
 * status on error.
 */
/*
 * Hacked ! Because the loader switch interface does not pass in
 * the linked list of region records, macho_get_entry_pt uses 
 * a precomputed entry point which is stored in handle by 
 * macho_map_regions.
 */
static int 
macho_get_entry_pt(ldr_module_handle handle_fi, ldr_entry_pt_t *entry_pt)
{
	mo_lc_entry_t *entry_p;			/* load command table entry ptr */
	macho_module_handle_ptr_t handle =	/* switching types across interface */
		(macho_module_handle_ptr_t) handle_fi;


	CHECK_MACHO_HANDLE(handle);

	entry_p = handle->entry_pt_cmd;

	if (entry_p != NULL) {
		*entry_pt = handle->entry_pt;
		return ((*entry_pt == NULL) ? LDR_ENOMAIN : LDR_SUCCESS);
	}
	else {
		dprintf(("macho_get_entry_pt : module has no main_entry LC \n"));
		return LDR_ENOMAIN;	/* no main_entry pt LC found */
	}
}

/*
 * get_mo_ldr_addr : converts a macho mo_addr_t (which is an object
 * file address) to a pointer to the appropriate address. 
 * This call assumes, that section data for the LC has been previously
 * fetched from file and a ptr to it exists in LC table.  Also, this
 * only works for LC's that have associated sections.
 */
/*
 * NOTE : this should be made a macro for efficiency.
 * NOTE : we could use the flag field in mo_lc_entry to mark that
 * a region is mapped or not (for later).
 */
void *
get_mo_ldr_addr(mo_addr_t *fileaddr, macho_module_handle_ptr_t handle)
{
	mo_lcid_t index;		/* LC index */
	mo_offset_t offset;		/* offset from section start */
	mo_lc_entry_t *p;		/* ptr to LC table entry */

	index = fileaddr->adr_lcid;
	offset = fileaddr->adr_sctoff;
#ifdef undef
	dprintf(("get_mo_ldr_addr: offset : 0x%x index : 0x%x\n",
		 index, offset));
#endif

	p = handle->load_cmd_table + index;

	/* check to see if this type of LC table entry has sections */
	if ((p->type != MO_REGION) && (p->type != MO_STRINGS)) {
		ldr_msg("get_mo_ldr_addr: such LC donot have sections\n");
		return NULL;
	}
	/* section data for the LC should have been pre-fetched or error */
	if ((p->section_data != NULL) && (index >= 0) && 
	    (index < handle->no_load_cmds)) {
		return (void *)((char*)(p->section_data) + offset);
	}
	else {
		ldr_msg("get_mo_ldr_addr: error converting file addr\n");
		return NULL;
	}
}

/*
 * Run the specified module's initialization or termination entry points,
 * as specified by the kind flag, if any.  Return LDR_SUCCESS on success
 * or negative error status on error.
 */
static int 
macho_run_inits(ldr_module_handle handle_fi, entry_pt_kind kind)
{
	func_table_command_t *lc_p;	/* load command ptr */
	int rc;				/* return code/status */
	macho_module_handle_ptr_t	/* switching types across interface */
		handle = (macho_module_handle_ptr_t) handle_fi;

	CHECK_MACHO_HANDLE(handle);

	if (kind == init_routines) {
		dprintf(("macho_run_inits: run initialization routines\n"));
		if (rc = run_init_funcs(handle) != LDR_SUCCESS) {
			return rc;
		}
	}
	else if (kind == term_routines) {
		dprintf(("macho_run_inits: run termination routines\n"));
		if (rc = run_term_funcs(handle) != LDR_SUCCESS) {
			return rc;
		}
	}
	else {
		ldr_msg("invalid routine type, loader can execute only initialization or termination functions\n");
		return LDR_EINVAL;
	}

	return LDR_SUCCESS;
}

/*
 *	run_term_funcs: run the termination functions for this module.
 *	The termination functions are stored in the handle on recognization
 *	of object file format.  This means that termination functions
 *	can be called even after macho_cleanup which unmaps the load
 *	commands.
 */
static int
run_term_funcs(macho_module_handle_ptr_t handle)
{
	int rc;			/* return code/status */

	if (handle->term_funcs != NULL) {
		rc = run_funcs(handle->term_funcs, 
			       handle->term_funcs_nentries, handle);
		if (rc != LDR_SUCCESS) {
			dprintf(("macho_run_inits: error executing termination funcs\n"));
			return rc;
		}
	}

	return LDR_SUCCESS;
}

/*
 *	run_init_funcs: run the initialization functions for this module.
 *	These are fetched by looking through the load commands unlike
 *	the termination functions which are pre-fetched.
 */
static int 
run_init_funcs(macho_module_handle_ptr_t handle)
{
	func_table_command_t *lc_p;	/* load command ptr */
	int rc;				/* return code/status */
	int i;				/* generic loop variable */


	/* 
	 * run thru load cmds calling initialization functions.
	 */

	for (i = 0; i < handle->no_load_cmds; i++) {
		lc_p = (func_table_command_t *) LOAD_CMD_N(i, handle);

		/* load cmd map has a null LC entry */
		if (lc_p == NULL) continue;

		if ((lc_p->ldc_cmd_type == LDC_FUNC_TABLE) && (lc_p->fntc_type == FNTC_INITIALIZATION)) {

			if (rc = run_funcs(&(lc_p->fntc_entry_loc[0]), 
					   lc_p->fntc_nentries, handle) != LDR_SUCCESS) {
				dprintf(("macho_run_inits: error executing initialization funcs\n"));
				return rc;
			}
		}
	}

	return LDR_SUCCESS;
}

/*
 * run_funcs : causes all the functions pointed at by an array of
 * mo_addr_t's to be executed.  A pointer to the first mo_addr_t 
 * the total number of mo_addr_t's and a macho manager handle
 * are provided as input arguments.
 */
static int
run_funcs(mo_addr_t *init_entry_loc, mo_short_t nentries, 
	  macho_module_handle_ptr_t handle)
{
	typedef int (*entry_pt_t)();
	int (*func)();
	mo_addr_t *p;
	int i;

	p = init_entry_loc;
	for (i = 0; i < nentries; i++) {
		func = (entry_pt_t) get_mo_ldr_addr(p, handle);
		(*func)();	/* call the init/term function */
		p++;
	}

	return LDR_SUCCESS;
}


/*
 * Complete the loading of the specified module, clean up open files,
 * temporary data structures, etc.  Return LDR_SUCCESS on success or
 * negative error status on error.
 */
static int
macho_cleanup(ldr_module_handle handle_fi)
{
	int rc;					/* return code/status */
	macho_module_handle_ptr_t handle = 	/* switching types across interface */
		(macho_module_handle_ptr_t) handle_fi;

	/* check for a valid module handle */
	CHECK_MACHO_HANDLE(handle);
	
	/* Note: throw away any temporary data structures here ! */
	if ((rc = internal_cleanup(handle)) != LDR_SUCCESS) {
		return rc;
	}

	dprintf(("macho_cleanup : done\n"));

	return LDR_SUCCESS;
}

/*
 * Unload the specified region of the specified object module.  deallocp is
 * pointer to procedure for deallocating space, if required (inverse of
 * allocp for map_region); see the note on address space management for a
 * description of this procedure.  Return LDR_SUCCESS on success or negative
 * error status on error.
 * [ NOTE: will be changed to take pointer to region description
 * structure instead of current region info. ]
 */

static int
internal_unload_region(macho_module_handle_ptr_t handle, int regno,
		       ldr_region_rec *region, dealloc_region_p deallocp)
{
	mo_lc_entry_t *region_lc_entry_p;	/* LC table entry for a region LC */
	int rc;					/* return code/status */


	CHECK_MACHO_HANDLE(handle);

	/* check for valid region number */
	if ((regno < 0) || (regno >= handle->no_regions)) {
		return LDR_EINVAL;
	}

	region_lc_entry_p = REGION_LCTABLE_ENTRY(handle->region_table, regno);

	if (region_lc_entry_p->type != MO_REGION) {
		return LDR_EINVAL;
	}
	
	/* region was never mapped in - do nothing more */
	if (region_lc_entry_p->section_data == NULL) {
		return LDR_SUCCESS;
	}

	/* unmap the region */
	rc = ldr_munmap(region_lc_entry_p->section_data, region->lr_size);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_unload_region: failure to ldr_munmap region\n", rc);

	region_lc_entry_p->section_data = NULL;
	region_lc_entry_p->section_window = NULL;
	region_lc_entry_p->type = MO_UNDEFINED;
	
	dprintf(("internal_unload_region: region no %d unloaded\n", regno));

	return ((*deallocp) (region->lr_vaddr, region->lr_mapaddr, 
			     region->lr_size));
}

/*
 * Finish the unloading of the specified module, clean up open files,
 * temporary data structures, etc.  This should include deallocating
 * the module's export list.  Once this routine is complete, the
 * ldr_module_handle is no longer useable.  Return LDR_SUCCESS on
 * success or negative error status on error.  NOTE: this routine is
 * also used to abort the loading of a module when a load error has
 * occurred.
 */
/*
 * NOTE: need to add checks for rc values.
 */
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
static int
macho_unload(ldr_module_handle handle_fi, ldr_region_allocs *allocsp,
	     int reg_count, ldr_region_rec *regions,
	     int ipkg_count, ldr_package_rec *import_pkgs,
	     int import_count, ldr_symbol_rec *imports,
	     int epkg_count, ldr_package_rec *export_pkgs)
{
	int rc;
	macho_module_handle_ptr_t handle = /* switching types across interface */
		(macho_module_handle_ptr_t) handle_fi;

	CHECK_MACHO_HANDLE(handle);

	rc = internal_free_regions(handle, reg_count, regions, allocsp);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_unload: error freeing regions", rc);

	rc = internal_free_pkg_import_recs(handle, ipkg_count, import_pkgs,
					   import_count, imports,
					   epkg_count, export_pkgs);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_unload: error freeing pkgs/import recs", rc);

	/*
	 * cleanup up by first going thru the LC table and freeing/
	 * unwindowing/unmmapping portions of the object file. 
	 * Once the LC table has been disposed with, the region table
	 * is next, followed by the windowed load commands in the 
	 * handle.
	 */
	rc = internal_cleanup(handle);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_unload: internal error while finishing cleanup", rc);

	rc = internal_finish_unload(handle);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_unload: internal error while finishing unloading", rc);

	return LDR_SUCCESS;
}

/* 
 * free the ldr_package_recs and ldr_symbol_recs.
 */
static int
internal_free_pkg_import_recs(macho_module_handle_ptr_t handle,
			      int ipkg_count, ldr_package_rec *import_pkgs,
			      int import_count, ldr_symbol_rec *imports,
			      int epkg_count, ldr_package_rec *export_pkgs)
{
	int rc;				/* return code/status */
	int src = 0;			/* saved return code/status */

	rc = ldr_packages_free(ipkg_count, import_pkgs);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_free_pkg_import_recs: error freeing import package records\n"));
		src = rc;
	}

	rc = ldr_packages_free(epkg_count, export_pkgs);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_free_pkg_import_recs: error freeing export package records\n"));
		src = rc;
	}

	rc = ldr_symbols_free(import_count, imports);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_free_pkg_import_recs: error freeing ldr_symbol_rec records\n"));
		src = rc;
	}
	
	return ((src != LDR_SUCCESS) ? src : LDR_SUCCESS);
}

static int
internal_cleanup(macho_module_handle_ptr_t handle)
{
	mo_lc_entry_t *lc_entry_p;		/* ptr to LC table entry */
	int rc;					/* return code/status */
	int i;					/* loop variable */

	CHECK_MACHO_HANDLE(handle);

	lc_entry_p = handle->load_cmd_table;
	if (lc_entry_p != NULL) {
		for (i = 0; i < handle->no_load_cmds; i++) {

			if (lc_entry_p == NULL) break;

			/*
			 * check type of LC table entry, and take cleanup action
			 */

			switch (lc_entry_p->type) {
			      case MO_CMD_MAP:	case MO_INTERPRETER: case MO_PACKAGE:
			      case MO_ENTRY: case MO_FUNC_TABLE: case MO_GEN_INFO: 
			      case MO_REGION:
				lc_entry_p->LC = NULL;	/* LC's are about to be unwindowed */
				break;

			      case MO_STRINGS:		/* unwindow the associated load cmd section */
			      case MO_RELOC: case MO_SYMBOLS:
				UNWINDOW_SECTION(lc_entry_p);
				lc_entry_p->LC = NULL;	/* LC's are about to be unwindowed */
				break;

			      case MO_UNDEFINED:
				dprintf(("macho_cleanup: warning: LDC_UNDEFINED load cmd - no cleanup action\n"));
				lc_entry_p->LC = NULL;	/* LC's are about to be unwindowed */
				break;

			      default:
				ldr_msg("macho_cleanup: warning: unsupported load cmd - no cleanup action\n");
				break;
			}

			lc_entry_p++;
		}
	}

	if (handle->load_cmd_w != NULL) { 	/* unwindow the LC's */
		ldr_unwindow(handle->load_cmd_w);
		handle->load_cmd_w = NULL;
	}

	/* close file if not previously closed */
	if (handle->fd != LDR_FILE_NONE){
		rc = ldr_close(handle->fd);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, 
			      "macho_finish_unload: error closing file", rc);
		handle->fd = LDR_FILE_NONE;	/* mark file closed */
	}

	return LDR_SUCCESS;
}

static int 
internal_finish_unload(macho_module_handle_ptr_t handle)
{
	mo_lc_entry_t *lc_entry_p;		/* ptr to LC table entry */
	mo_region_entry_t *reg_p;		/* ptr to region table entry */
	int rc;					/* return code/status */
	int i;					/* loop variable */


	CHECK_MACHO_HANDLE(handle);


	if (handle->region_table != NULL) {	/* free the FD region table */
		ldr_free(handle->region_table);	
	}

	/* NOTE: free the export list here i.e. handle->export_list */
	if (free_export_list(handle) != LDR_SUCCESS) {
		ldr_msg("macho_finish_unload: macho_ldr - error freeing export list\n");
		ldr_msg("macho_ldr - continuing unloading\n");
	}

	/* close file if not previously closed */
	if (handle->fd != LDR_FILE_NONE){
		rc = ldr_close(handle->fd);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, 
			      "macho_finish_unload: error closing file", rc);
		handle->fd = LDR_FILE_NONE;	/* mark file closed */
	}

	if (handle->load_cmd_table != NULL) {
		ldr_free((univ_t) handle->load_cmd_table);
	}

	/* invalid to use handle after this */
	ldr_free((univ_t)handle);	

	dprintf(("internal_finish_unload: unloading complete\n"));

	return LDR_SUCCESS;
}

/* export symbols will be maintained as a linked list
 * in an open hash table.
 */
typedef struct symbol_val {
	struct symbol_val *sv_next;
	ldr_symval         sv_ldr_symval;
	char              *sv_pkg_name;
	int		   sv_pkg_no;
} symbol_val_t;

/*
 * free the export list and its key value pairs.
 * NOTE : this assumes export_list is an open hash table.
 */
static int
free_export_list(macho_module_handle_ptr_t handle)
{
	univ_t key;		/* pointer to key for hash table element */
	symbol_val_t *p, *next; /* ptrs to walk chained list of values */
	int found;		/* flag true/false */
	int rc;			/* return code/status */
	int src = 0;		/* saved return code/status */
	int index = 0;		/* index for element in hash table */


	if (handle->export_list == NULL) return LDR_SUCCESS;

	do {
		rc = open_hash_elements(handle->export_list, &index, 
					&key, (univ_t *)&p);
		if (rc == 0) {
			found = TRUE;
			ldr_free(key);
			while (p) {
				next = p->sv_next;
				ldr_free(p);
				p = next;
			}
		}
		else {
			found = FALSE;
			if (rc != LDR_EAGAIN) {
				ldr_msg("macho_ldr - error fetching element from hash table\n");
				src = rc;
			}
		}

	} while (found == TRUE);

	rc = open_hash_destroy(handle->export_list);
	if (rc !=  0) {
		ldr_msg("macho_ldr - error destroying open hash table\n");
		src = rc;
	}

	if (src == 0) {
		handle->export_list = NULL;
		handle->no_exports = 0;

		dprintf(("free_export_list: export list deallocated\n"));

		return LDR_SUCCESS;
	}
	else {
		return src;
	}
}

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
static int 
macho_get_imports(ldr_module_handle handle_fi, int *pkg_count,
		     ldr_package_rec **pkgs, int *sym_count,
		     ldr_symbol_rec **imports)
{
	mo_lc_entry_t *pkg_entry;	/* package LC table entry */
	mo_lc_entry_t *imp_entry;	/* symbols LC table entry */
	mo_lc_entry_t *string_entry;	/* strings LC table entry for symbols */
	mo_lc_entry_t *pkg_string_entry;/* strings for package LC */
	package_command_t *pkg_lc;	/* package load cmd */
	pkg_entry_t *pkg_p;		/* package information ptr */
	symbols_command_t *imp_lc;	/* symbols load cmd */
	symbol_info_t *sym_p;		/* symbol information */
	ldr_package_rec *pkg_list;	/* linked list of pkg records */
	ldr_symbol_rec *imp_list;	/* linked list of import records */
	mo_addr_t fileaddr;		/* temp to convert macho address */
	char *str_p;			/* temp for a C string */
	char *name_p;			/* temp for a C string */
	int no_pkgs;			/* number of packages */
	int no_imps;			/* number of import symbols */
	int pkg_string_index;		/* index of string load cmd for pkgs */
	int imp_string_index;		/* index of string load cmd for imports */
	int rc;				/* return code/status */
	int i;				/* loop variable */
	macho_module_handle_ptr_t handle /* switching types across interface */
		= (macho_module_handle_ptr_t) handle_fi;


	CHECK_MACHO_HANDLE(handle);

	/* 
	 * presumption: if there are no imported packages, there are no
	 * imported symbols
	 */
	if (handle->import_package_cmd != NULL) {
		pkg_entry = handle->import_package_cmd;
		pkg_lc = pkg_entry->LC;
		no_pkgs = pkg_lc->pkgc_nentries;
		dprintf(("macho_get_imports: no of pkgs = %d\n",no_pkgs));
		
		if ((rc = ldr_packages_create(no_pkgs, LDR_PACKAGE_VERSION,
					      &pkg_list)) != LDR_SUCCESS) {
			dprintf(("macho_get_imports: error creating import pkg list\n"));
			return rc;
		}

		imp_entry = handle->import_cmd;
		if (imp_entry == NULL) {
			dprintf(("macho_get_imports: error no import LC\n"));
			return LDR_ENOEXEC;
		}
		imp_lc = imp_entry->LC;
		no_imps = imp_lc->symc_nentries;

		if ((rc = ldr_symbols_create(no_imps, LDR_SYMBOL_VERSION,
					     &imp_list)) != LDR_SUCCESS) {
			dprintf(("macho_get_imports: error creating import list\n"));
			(void)ldr_packages_free(no_pkgs, pkg_list);
			return rc;
		}

		pkg_string_index = pkg_lc->pkgc_strings_id;
		pkg_string_entry = handle->load_cmd_table + pkg_string_index;
		rc = window_in_section(handle, pkg_string_entry);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, 
			      "macho_get_imports: failure to window pkg strings", rc);

		fileaddr.adr_lcid = pkg_string_index;
		pkg_p = &(pkg_lc->pkgc_pkg_list[0]);

		dprintf(("macho_get_imports: fill in pkg list\n"));
		/* run thru package info filling in package list */
		for (i = 0; i < no_pkgs; i++) {
			pkg_list[i].lp_kind = ldr_package;
			fileaddr.adr_sctoff = pkg_p->pe_pkg_name;
#ifdef undef
			dprintf(("macho_get_imports: fileaddr adr_lcid : 0x%x adr_sctoff : 0x%x\n",
				 fileaddr.adr_lcid, fileaddr.adr_sctoff));
#endif
			str_p = get_mo_ldr_addr(&fileaddr, handle);
			pkg_list[i].lp_name = ldr_strdup(str_p);
			dprintf(("macho_get_imports: %d : pkg-name : %s\n",
				 i, pkg_list[i].lp_name));

			pkg_p++;
		}

		/* fetch the section associated with the symbols load cmd */
		rc = window_in_section(handle, imp_entry);
		if (rc != LDR_SUCCESS) {
			dprintf(("macho_get_imports: failure to window symbols for LC\n"));
			return rc;
		}

		/* 
		 * fetch the strings section assoc with symbol_info_t's in 
		 * the symbol_info_t LC.
		 */
		sym_p = imp_entry->section_data;
		imp_string_index = imp_lc->symc_strings_section;
		string_entry = handle->load_cmd_table + imp_string_index;
		rc = window_in_section(handle, string_entry);
		if (rc != LDR_SUCCESS) {
			dprintf(("macho_get_imports: failure to window strings section\n"));
			return rc;
		}		

		dprintf(("macho_get_imports: fill in import recs\n"));

		fileaddr.adr_lcid = imp_string_index;
		for (i = 0; i < no_imps; i++) {
			fileaddr.adr_sctoff = sym_p->si_symbol_name;
			name_p = get_mo_ldr_addr(&fileaddr, handle);
			imp_list[i].ls_name = ldr_strdup(name_p);
			imp_list[i].ls_packageno = sym_p->si_package_index;
			ldr_symval_make_unres(&imp_list[i].ls_value);
			if (sym_p->si_flags & SI_DATA_F)
				ldr_symval_make_data(&imp_list[i].ls_value);
			else if (sym_p->si_flags & SI_CODE_F)
				ldr_symval_make_function(&imp_list[i].ls_value);
			/* else it's unknown */

			dprintf(("macho_get_imports: %d symbol name: %s pkgno : %d\n",
				 i, imp_list[i].ls_name, imp_list[i].ls_packageno));
			sym_p++;
		}

		*pkg_count = no_pkgs;
		*sym_count = no_imps;
		*pkgs = pkg_list;
		*imports = imp_list;

		dprintf(("macho_get_imports: pkg_count : %d sym_count : %d\n",
			 *pkg_count, *sym_count));
		return LDR_SUCCESS;
	}
	else {
		dprintf(("macho_get_imports: returning 0\n"));
		*pkg_count = *sym_count = 0;
		*pkgs = NULL;
		*imports = NULL;
		return LDR_SUCCESS;
	}
}

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
/*
 * NOTE: as a side effect this routine build the export list.
 */

static int 
macho_get_export_pkgs(ldr_module_handle handle_fi, int *count,
		     ldr_package_rec **packages)
{
	package_command_t *pkg_lc;		/* package load cmd */
	pkg_entry_t *pkg_entry_p;		/* package list entry */
	ldr_package_rec *package;		/* package info to be filled */
	mo_lc_entry_t *string_entry;		/* string LC table entry */
	mo_addr_t fileaddr;			/* used in translating macho address */	
	int pkg_count;				/* no. of packages exported */
	int string_index;			/* assoc. string section id */
	int rc;					/* return code/status */
	int i;					/* loop variable */
	macho_module_handle_ptr_t handle = 	/* switch types across interface */
		(macho_module_handle_ptr_t) handle_fi;
	void *temp;


	CHECK_MACHO_HANDLE(handle);

	if (handle->export_package_cmd != NULL) {
		pkg_lc = (package_command_t *) 
			(handle->export_package_cmd)->LC;
		pkg_count = pkg_lc->pkgc_nentries;

		rc = ldr_packages_create(pkg_count, LDR_PACKAGE_VERSION,
					 packages);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, 
			      "macho_get_export_pkgs: failure alloc pkgs\n", rc);

		/* map in assoc strings section, if not done previously */
		string_index = pkg_lc->pkgc_strings_id;

		dprintf(("macho_get_export_pkgs: window_in_section %d string sect\n",
			 string_index));

		string_entry = handle->load_cmd_table + string_index;
		if (string_entry->section_data == NULL) {
			rc = window_in_section(handle, string_entry);
			if (rc != LDR_SUCCESS) {
				dprintf(("macho_get_export_pkgs: window_in_section rc = %d\n",
					 rc));
				return rc;
			}
		}

		pkg_entry_p = &(pkg_lc->pkgc_pkg_list[0]);
		package = *packages;

		for (i = 0; i < pkg_count; i++) {

			package->lp_kind = ldr_package;

			/* setup to call get_mo_ldr_addr() */
			fileaddr.adr_lcid = (mo_long_t) string_index;
			fileaddr.adr_sctoff = pkg_entry_p->pe_pkg_name;

			dprintf(("macho_get_export_pkgs: call get_mo_ldr_addr\n"));

			temp = get_mo_ldr_addr(&fileaddr, handle);
			if (temp == NULL) {
				dprintf(("macho_get_export_pkgs: failure converting addr\n"));
				return LDR_ENOEXEC;
			}

			package->lp_name = ldr_strdup((char *) temp);

			dprintf(("macho_get_export_pkgs: pkg name : %s\n",
				 package->lp_name));

			package++;
			pkg_entry_p++;
		}

		*count = pkg_count;

		dprintf(("macho_get_export_pkgs: no of exported pkgs : %d\n",
			 *count));

		/* 
		 * we build the export list at this time. That way,
		 * we avoid having to keep the file open for mapping
		 * longer.
		 */
		if (handle->export_list == NULL) {
			dprintf(("macho_get_export_pkgs: building export table\n"));
			rc = build_export_list(handle);
			if (rc != LDR_SUCCESS) {
				dprintf(("macho_get_export_pkgs: failure building export list\n"));
				return rc;
			}

			dprintf(("macho_get_export_pkgs: export table built\n"));
		}

		return LDR_SUCCESS;
	}
	else {
		dprintf(("macho_get_export_pkgs no export pkg load cmd\n"));
		*count = 0;
		*packages = NULL;
		return LDR_SUCCESS;
	}

}

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
 * their own symbol resolution algorithms.  It works like the build_export_list
 * routine below, but simply builds the linear list rather than a
 * hash table.
 *
 *	macho_get_exports: builds its list of exports from the export_list
 *	hash table stored in the handle.  In this way, this routine can
 *	be called long after macho_cleanup has closed the file and got
 *	rid of windows to the load commands and various sections of the
 *	macho file.
 */
static int macho_get_exports(ldr_module_handle handle_fi, int *sym_count,
		      ldr_symbol_rec **exports)
{
	ldr_symbol_rec *exp_list;
	char *key;				/* symbol name will be returned here */
	symbol_val_t *svp, *p;			/* export hash table elements */
	int export_count;			/* total number of exports for module */
	int rc;					/* return code/status */
	int found;				/* no of exports found in handle's export list */
	int pkg_id;
	open_hash_element_index index = 0; 	/* used by open_hash_elements to search hash table */
	macho_module_handle_ptr_t handle = 	/* switch types across interface */
		(macho_module_handle_ptr_t) handle_fi;


	if (handle->no_exports == 0) {
		dprintf(("macho_get_exports: building export table\n"));
		rc = build_export_list(handle);
		if (rc != LDR_SUCCESS) {
			dprintf(("macho_get_exports: failure building export list\n"));
			return rc;
		}
		dprintf(("macho_get_exports: export table built\n"));
	}

	if (handle->no_exports == MO_NO_EXPORTS) {
		dprintf(("get_export_list: MO_NO_EXPORTS symbols\n"));
		*sym_count = 0;
		*exports = NULL;
		return LDR_SUCCESS;
	}

	export_count = handle->no_exports;
	
	if ((rc = ldr_symbols_create(export_count, LDR_SYMBOL_VERSION,
				     &exp_list)) != LDR_SUCCESS) {

		dprintf(("get_exports: failure creating export list\n"));
		return rc;
	}


	found = 0;
	while (found < export_count) {

		/* get next bucket from hash table */
		rc = open_hash_elements(handle->export_list, &index, (univ_t *)&key, (univ_t *)&svp);
		if (rc != LDR_SUCCESS) {
			if (rc == LDR_EAGAIN) /* no more elements */
				break;
			else  {
				ldr_msg("macho_get_exports: open_hash_elements error rc = %d\n",rc);
				return rc;
			}
		}

		/* walk the chained hash table linked list ... */
		p = svp;
		while (p != NULL) {

			/* Get symbol's string name */
			exp_list[found].ls_name = ldr_strdup((char *) key);

			/* And package number.. */
			exp_list[found].ls_packageno = p->sv_pkg_no;

			/* And, finally, value */
			exp_list[found].ls_value = p->sv_ldr_symval;

			p = p->sv_next;
			found++;

			if (found > export_count) {
				ldr_msg("macho_get_exports: internal error found = %d no of exports = %d\n",
					found, export_count);
				return(LDR_ENOEXEC);
			}
		}


	}

	dprintf(("get_exports : found = %d total no exports = %d\n",found, export_count));

	*sym_count = found;
	*exports = exp_list;

	return LDR_SUCCESS;
}

/* 
 * window in data associated with a load command and save its window
 * pointer and data pointer in mo_lc_entry_t.
 */
int
window_in_section(macho_module_handle_ptr_t handle, mo_lc_entry_t *p)
{
	ldr_window_t *w_p;
	ldc_header_t *lc_hdr;
	univ_t data;
	int rc;


	lc_hdr = (ldc_header_t *) p->LC;
	if (lc_hdr == NULL) {
		dprintf(("window_in_section: lc entry points to NULL load cmd\n"));
		return LDR_ENOEXEC;
	}

	/* Do nothing if this load cmd has no section in the file */
	if ((lc_hdr->ldci_section_off == 0) && 
	    (lc_hdr->ldci_section_len == 0)) {
		dprintf(("window_in_section: load cmd has no section in file\n"));
		return LDR_SUCCESS;
	}

	/* map in file section if not previously mapped */
	if (p->section_data == NULL) {

#ifdef undef
		ldr_msg("window_in_section : off : 0x%x len : 0x%x\n",
			 (unsigned long)lc_hdr->ldci_section_off, 
			 (size_t)lc_hdr->ldci_section_len);
#endif

		rc = window(&w_p, handle->fd, 
			    (unsigned long) lc_hdr->ldci_section_off, 
			    (size_t) lc_hdr->ldci_section_len, (univ_t*)&data);
		if (rc != LDR_SUCCESS) return rc;

		p->section_data = data;
		p->section_window = w_p;

#ifdef undef
		ldr_msg("window_in_section: section mapped in data at 0x%x\n",
			 data);

		ldr_msg("window_in_section: first word of seg data : 0x%x\n",
			 *((char*)data));
#endif

	}
#ifdef undef
	else {
		dprintf(("window_in_section: section was previously mapped\n"));
	}
#endif

	return LDR_SUCCESS;
}

/* Look up the specified import symbol from the specified packge in
 * the specified object module, and fill in its value in the import
 * symbol record.  Can use the following fields in the import record:
 *  - symbol name
 * Must fill in the following fields in the import symbol record:
 *  - symbol value
 * Return LDR_SUCCESS on success or negative error status on error.
 */

static int
macho_lookup_export(ldr_module_handle handle_fi,
		    ldr_package_rec *package,
		    ldr_symbol_rec *symbol)
{
	char *pkgname;				/* package name */
	char *symname;				/* symbol name */
	symbol_val_t *svp, *p;			/* used for hash table lookup */
	int rc;					/* return status/code */
	int i;					/* loop variable */
	macho_module_handle_ptr_t handle = 	/* switching types across interface */
		(macho_module_handle_ptr_t) handle_fi;


	CHECK_MACHO_HANDLE(handle);

	/* Just in-case macho_lookup_export is called before 
	 * macho_get_export_pkgs, we can build the export_list here.
	 * However, this may not always work as the file may have
	 * been closed and it may not be possible to map the file
	 * so late in the game.
	 */
	if (handle->export_list == NULL) {
		dprintf(("macho_lookup_export: building export list\n"));
		rc = build_export_list(handle);
		if (rc != LDR_SUCCESS) {
			dprintf(("macho_lookup_export: failure to build export list\n"));
			return rc;
		}
		dprintf(("macho_lookup_export: export list built\n"));
	}

	/* key is <symbol-name> */
	pkgname = package->lp_name;
	symname = symbol->ls_name;
#ifdef undef
	dprintf(("macho_lookup_export: lookup symbol in export list\n"));
#endif

	/* lookup the symbol in the export list */
	if ((rc = open_hash_lookup(handle->export_list, (const univ_t) symname,
				   (univ_t *) &svp)) != LDR_SUCCESS) {
		dprintf(("macho_lookup_export: hash lookup on key failed\n"));
		return rc;
	}

	for(p = svp; p; p = p->sv_next) {
		if (!strcmp(pkgname, p->sv_pkg_name)) {

			/* Check for type matches.  Type mismatch
			 * is an error.
			 */

			if (!ldr_symval_type_check(&symbol->ls_value,
						   &svp->sv_ldr_symval))
				return(LDR_EVERSION);

			symbol->ls_value = svp->sv_ldr_symval;
			return LDR_SUCCESS;
		}
	}

	return LDR_ENOSYM;

}


/* 
 * build_export_list : builds an open hash table for the symbols
 * exported by this module.  Symbols with the same name result
 * to same hash-table bucket which are then chained. The package
 * name is stored in the symbol_val_t.
 */
static int
build_export_list(macho_module_handle_ptr_t handle)
{
	char *symname;			/* symbol name */
	mo_lc_entry_t *sym_entry;	/* symbol load cmd table entry */
	symbol_info_t *sym_p;		/* symbol information */
	symbol_val_t *svp, *prev, *p;	/* hash table elements */
	int export_count;		/* no of symbols exported by module */
	int rc;				/* return code/status */
	int i;				/* loop variable */
	int found;			/* flag */


	if (handle->no_exports == MO_NO_EXPORTS) {
		dprintf(("build_export_list: MO_NO_EXPORTS\n"));
		return LDR_ENOSYM;
	}

	if (handle->no_exports == 0) {
		/* macho_cleanup has been done - too late to build export list */
		if ((handle->def_symb_cmd == NULL) || 
		    ((handle->def_symb_cmd)->LC == NULL)) {
			dprintf(("build_export_list: no defined symbol load cmd\n"));
			return LDR_ENOEXEC;
		}
			
		export_count = ((symbols_command_t *)((handle->def_symb_cmd)->LC))->symc_n_exported_symb;
		if (export_count == 0) {

			dprintf(("build_export_list : 0 exports\n"));

			handle->no_exports = MO_NO_EXPORTS;
			handle->export_list = NULL;
			return LDR_SUCCESS;
		}
		else {
			handle->no_exports = export_count;
		}
	}
	else {
		export_count = handle->no_exports;
	}
	
	if ((rc = open_hash_create(export_count, (ldr_hash_p)hash_string,
				   (ldr_hash_compare_p)strcmp, 
				   (open_hash_flags_t)0, 
				   &(handle->export_list))) != LDR_SUCCESS) {
		dprintf(("build_export_list: failure creating export list\n"));
		return rc;
	}

	dprintf(("build_hash_table: created hash table. size = %d\n", 
		 export_count));

	/* window in section for defined symbols */
	sym_entry = handle->def_symb_cmd;
	if (rc = window_in_section(handle, sym_entry) != LDR_SUCCESS) {
		return rc;
	}

	i = 0; found = 0;
	while (found < export_count) {
		
		/* not all defined symbol are exports - skip locals */
		sym_p = ((symbol_info_t *)sym_entry->section_data) + i;

		if (!(sym_p->si_flags & SI_EXPORT_F)) continue;
		
		rc = ldr_malloc(sizeof(symbol_val_t), LDR_MACHO_T, 
				(univ_t*) &svp);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, 
			      "build_export_list: allocation of ldr_symval failed", rc);
		svp->sv_next = NULL;

		/* key = <symbol-name> */
		rc = get_sym_pkg_info(handle, i, &(svp->sv_pkg_name), 
				      &symname, &(svp->sv_ldr_symval), &(svp->sv_pkg_no));
		if (rc != LDR_SUCCESS) {
			dprintf(("build_export_list: failed getting sym/pkg info\n"));
			return rc;
		}
		dprintf(("build_export_list: inserted key : %s in hash table\n",
			symname));

		prev = svp;
		rc = open_hash_search(handle->export_list, (const univ_t)symname,
				      (univ_t *) &prev, 
				      (LDR_HASH_INSERT | LDR_HASH_LOOKUP));
		if (rc != LDR_SUCCESS) {
			free_export_list(handle);
			dprintf(("build_export_list: open_hash_search failed rc %d\n",
				 rc));
			return rc;
		}

		/* symbol name already in table, chain to end of list */
		if (prev != svp) {
			for (p=prev; p->sv_next; p = p->sv_next)
				;
			p->sv_next = svp;
			svp->sv_next = NULL;
		}

		i++; found++; svp = NULL;
#ifdef undef
		dprintf(("build_export_list: inserted key in hash table\n"));
#endif
	}

	dprintf(("build_export_list : found = %d\n",found));

	return LDR_SUCCESS;
}

/*
 *	get symbol name, package name and symbol value for the "index-th"
 *	symbol exported by this module.
 */

static int
get_sym_pkg_info(macho_module_handle_ptr_t handle, int index, 
		 char **pkgname, char **symname, ldr_symval *symvalue, int *pkgno)
{
	mo_lc_entry_t *sym_entry;		/* symbols LC table entry */
	mo_lc_entry_t *string_entry;		/* strings LC entry for symbols */
	mo_lc_entry_t *pkg_entry;		/* package LC table entry */
	mo_lc_entry_t *pkg_string_entry;	/* strings LC entry for packages */
	mo_lc_entry_t *reg_entry;		/* region load cmd table entry */
	symbols_command_t *sym_lc;		/* symbols load cmd */
	package_command_t *pkg_lc;		/* package load cmd */
	symbol_info_t *sym_p;			/* accessing symbol information */
	pkg_entry_t *pkg_p;			/* package information */
	mo_addr_t fileaddr;			/* temp, for passing args to get_mo_lldr_addr() */
	int pkg_id;
	int rc;					/* return code/status */
	int i;					/* loop variable */
	univ_t temp;


	sym_entry = handle->def_symb_cmd;
	sym_lc = sym_entry->LC;
	rc = window_in_section(handle, sym_entry);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, 
		      "get_sym_pkg_info: window symbol sect data failed", rc);

	sym_p = ((symbol_info_t *)sym_entry->section_data) + index;
	
	/* get symbol name */
	string_entry = handle->load_cmd_table + 
		sym_lc->symc_strings_section;
	rc = window_in_section(handle, string_entry);
	MO_NEG_ASSERT(rc, LDR_SUCCESS, 
		      "get_sym_pkg_info: window symols string section failed", rc);

	fileaddr.adr_lcid = sym_lc->symc_strings_section;
	fileaddr.adr_sctoff = sym_p->si_symbol_name;

	temp = get_mo_ldr_addr(&fileaddr, handle);
	*symname = ldr_strdup((char*)temp);

	/* get the package name and number */
	pkg_id = sym_p->si_package_index;
	*pkgno = pkg_id;
	/* NOTE: might need to check for a valid package-id ! */
	pkg_entry = handle->export_package_cmd;
	pkg_lc = pkg_entry->LC;
	pkg_string_entry = handle->load_cmd_table + 
		pkg_lc->pkgc_strings_id;
	rc = window_in_section(handle, pkg_string_entry); /* assoc strings */
	MO_NEG_ASSERT(rc, LDR_SUCCESS, 
		      "get_sym_pkg_info: window package string section failed", rc);

	fileaddr.adr_lcid = pkg_lc->pkgc_strings_id;
	pkg_p = ((&pkg_lc->pkgc_pkg_list[0]) + pkg_id);
	fileaddr.adr_sctoff = pkg_p->pe_pkg_name;
	*pkgname = ldr_strdup(get_mo_ldr_addr(&fileaddr, handle));

	/* NOTE : I think getting the symval from file needs other falgs ?? */
	/* get symbol value */
	if (sym_p->si_flags & SI_EXPORT_F) {
		rc = sivalue_to_ldrsymval(handle, sym_p, symvalue);
		MO_NEG_ASSERT(rc, LDR_SUCCESS, 
			      "get_sym_pkg_info: failure to make ldr_symval from macho file", rc);
	}
	else {
		ldr_msg("get_sym_pkg_info: error - not an export symbol\n");
		return LDR_ENOEXEC;
	}
#ifdef undef
	dprintf(("get_sym_pkg_info: index : %d pkg-name : %s sym-name : %s\n",
		 index, *pkgname, *symname));
#endif

	return LDR_SUCCESS;
}


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
/*
 * NOTE: relocation is a machine dependent operation and hence
 * the routine mach_dep_relocate_region() is machine dependent.
 */

static int 
macho_relocate(ldr_module_handle handle_fi, int nregions,
	       ldr_region_rec *regions, int npackages,
	       ldr_package_rec *import_pkgs, int nimports,
	       ldr_symbol_rec *imports)
{
	region_command_t *reg_lc;		/* region load cmd */
	mo_lc_entry_t *region_entry;		/* region LC table entry */
	reloc_command_t *reloc_lc;		/* relocation LC */
	mo_lc_entry_t *reloc_entry;		/* region relocation LC table entry */
	int no_regions;				/* region count */
	int i;					/* loop variable */
	int rc;					/* return code/status */
	macho_module_handle_ptr_t handle 	/* switching types across interface */
		= (macho_module_handle_ptr_t) handle_fi;

	CHECK_MACHO_HANDLE(handle);

	no_regions = handle->no_regions;


	for (i = 0; i < no_regions; i++) {

		/* lookup region table for load cmd table entry */

		region_entry = (mo_lc_entry_t *)
			REGION_LCTABLE_ENTRY(handle->region_table, i);
		reg_lc = (region_command_t *) region_entry->LC;

		/* check if region needs relocation */
		if (reg_lc->regc_reloc_addr != MO_INVALID_LCID) {
			dprintf(("macho_relocate: region no %d needs to be relocated\n",i));

			CHECK_LC_ID(handle, reg_lc->regc_reloc_addr);

			/* window in the associated relocation records */
			reloc_entry = handle->load_cmd_table + 
				reg_lc->regc_reloc_addr;
			reloc_lc = (reloc_command_t *) reloc_entry->LC;
			if (reloc_lc->ldc_cmd_type != LDC_RELOC)
				return(LDR_ENOEXEC);
			rc = window_in_section(handle, reloc_entry);
			MO_NEG_ASSERT(rc, LDR_SUCCESS, 
				      "macho_relocate: failure to window relocation section", rc);

			rc = mach_dep_relocate_region(handle, regions, nregions,
						      npackages, import_pkgs,
						      nimports, imports, 
						      i); 

			MO_NEG_ASSERT(rc, LDR_SUCCESS, "macho_relocate: failure to relocate region", rc);

			dprintf(("macho_relocate: region %d successfully relocated\n",i));
		}
		else {
			dprintf(("macho_relocate: region no %d needs no relocation\n",i));
			continue;
		}
	}

	return LDR_SUCCESS;
}


#ifdef DEBUG

void print_hdr(mo_header_t *p)
{
	ldr_msg("printing macho header\n");
	
	ldr_msg("header.moh_magic : 0x%x\n", p->moh_magic);
	/* ldr_msg("header.moh_file_type : 0x%x\n", p->moh_file_type); */
	ldr_msg("header.moh_load_map_cmd_off : 0x%x\n", p->moh_load_map_cmd_off);
	ldr_msg("header.moh_first_cmd_off : 0x%x\n", p->moh_first_cmd_off);
	ldr_msg("header.moh_sizeofcmds : %d\n", p->moh_sizeofcmds);
	ldr_msg("header.moh_n_load_cmds : %d\n",p->moh_n_load_cmds);
}

void print_lc_map(load_cmd_map_command_t *p)
{
	int i;

	ldr_msg("printing LC map from file\n");
	ldr_msg("lc_map.ldc_cmd_type : %d\n",p->ldc_cmd_type);
	ldr_msg("lc_map.ldc_cmd_size : %d\n",p->ldc_cmd_size);
	ldr_msg("lc_map.ldc_section_off : %d\n",p->ldc_section_off);
	ldr_msg("lc_map.ldc_section_len : %d\n",p->ldc_section_len);

	ldr_msg("lc_map.lcm_nentries : %d\n",p->lcm_nentries);

	for (i = 0; i < p->lcm_nentries; i++) {
		ldr_msg("mo_offset[%d] : 0x%x\n", i, p->lcm_map[i]);
	}
}

void print_lc_table(mo_lc_entry_t *lc_table, int size)
{
	int i;
	mo_lc_entry_t *p = lc_table;

	ldr_msg("LC table size : %d\n",size);

	for (i = 0; i < size; i++, p++) {
		ldr_msg("[%d] LC : 0x%x type : 0x%x flags : 0x%x section_data : 0x%x section_window : 0x%x\n",
		       i, p->LC, p->type, p->flags, p->section_data, p->section_window);
	}
}

void print_region_table(mo_region_entry_t *region_table, int size)
{
	int i;
	mo_region_entry_t *p = region_table;

	ldr_msg("region_table : size : %d\n",size);
	for (i = 0; i < size; i++, p++) {
		ldr_msg("[%d] LC_entry : 0x%x region_size : %d\n",
		       i, p->LC_entry, p->region_size);
	}
}

void print_handle(macho_module_handle_ptr_t p)
{
	ldr_msg("Macho module handle\n");
	ldr_msg("handle : no_load_cmds : %d load_cmd_table : 0x%x no_regions : %d\n",
	       p->no_load_cmds, p->load_cmd_table, p->no_regions);
	ldr_msg("handle : region_table : 0x%x def_symb_cmd : 0x%x import_cmd : 0x%x import_package_cmd : 0x%x\n",
	       p->region_table, p->def_symb_cmd, p->import_cmd, p->import_package_cmd);
	ldr_msg("handle : string_cmd : 0x%x fd %d export_list 0x%x\n\n",
	       p->string_cmd, p->fd, p->export_list);
}

void 
print_handle_and_state(void *handle_fi)
{
	macho_module_handle_ptr_t handle = handle_fi;

	ldr_msg(" ----- Macho Mngr state for handle -----\n");
	print_handle(handle);
	print_lc_table(handle->load_cmd_table, handle->no_load_cmds);
	print_region_table(handle->region_table, handle->no_regions);
}

#endif /* DEBUG */
