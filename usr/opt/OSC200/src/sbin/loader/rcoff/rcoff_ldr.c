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
static char	*sccsid = "@(#)$RCSfile: rcoff_ldr.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/07 16:18:58 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * rcoff_ldr.c
 * relocatable COFF loader routines
 *
 * Loader switch routines called by the object file
 * format independent loader when accessing relocatable COFF files.
 *
 * OSF/1 Release 1.0
 */

/* #define	DEBUG */

#include <sys/types.h>
#include <strings.h>

#include "rcoff_machdep.h"

#include "loader.h"
#include "ldr_types.h"
#include "ldr_sys_int.h"
#include "ldr_windows.h"
#include "ldr_malloc.h"
#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_switch.h"
#include "ldr_hash.h"

#include "chain_hash.h"
#include "ldr_known_pkg.h"
#include "ldr_module.h"

#include "open_hash.h"
#include "rcoff_ldr.h"
#include "ldr_errno.h"

#ifndef TRUE
#define TRUE	1
#define	FALSE	0
#endif

/*
 * Currently set up so debugging only works with
 * kernel load server.  Must be compiled -DDEBUG
 * and use '-d 5' (or greater) command line option.
 */
int kls_debug_level;

#ifdef	DEBUG
#define	dprintf(x)	if (kls_debug_level >= 5) ldr_msg x
#else
#define	dprintf(x)
#endif /* DEBUG */

#define	round(x, s)	(((unsigned)(x) + (unsigned)(s) - 1) & ~((unsigned)(s) - 1))
#define	trunc(x, s)	(((unsigned)(x)) & ~((unsigned)(s) - 1))
#define aligned(x, s)	(((unsigned)(x) & ((unsigned)(s) - 1)) == 0)

/* Names of the sections to be loaded  */

#define	TEXT_REGION		".text"
#define DATA_REGION		".data"
#define BSS_REGION		".bss"

/* Forward references */

static int internal_map_region(rcoff_module_handle_t,
			       ldr_region_allocs *allocps,
			       int regno, ldr_region_rec *region,
			       int relocatable);
static int internal_free_regions(int nregions, ldr_region_rec *regions,
				 ldr_region_allocs *allocps);
static int internal_unload_region(int regno, ldr_region_rec *region,
				  ldr_region_allocs *allocps);
static int internal_free_pkg_and_syms(int ipkg_count, ldr_package_rec *import_pkgs,
			   int import_count, ldr_symbol_rec *imports,
			   int epkg_count, ldr_package_rec *export_pkgs);
static int rcoff_recognizer(const char *filename, ldr_file_t fd,
			       ldr_module_handle *mod);
static int rcoff_get_static_dep(ldr_module_handle handle, int depno,
				   char **dep);
static int rcoff_get_imports(ldr_module_handle handle, int *pkg_count,
				ldr_package_rec **pkgs, int *sym_count,
				ldr_symbol_rec **imports);
static int rcoff_map_regions(ldr_module_handle handle, ldr_region_allocs
				    *allocps, int *reg_count,
				    ldr_region_rec **regions);
static int rcoff_get_export_pkgs(ldr_module_handle handle, int *count,
				    ldr_package_rec **packages);
static int rcoff_get_exports(ldr_module_handle handle, int *sym_count,
				ldr_symbol_rec **exports);
static int rcoff_lookup_export(ldr_module_handle handle,
				  ldr_package_rec *package,
				  ldr_symbol_rec *symbol);
static int rcoff_relocate(ldr_module_handle handle, int nregions,
			     ldr_region_rec regions[], int npackages,
			     ldr_package_rec import_pkgs[], int nimports,
			     ldr_symbol_rec imports[]);
static int rcoff_get_entry_pt(ldr_module_handle mod, ldr_entry_pt_t *entry_pt);
static int rcoff_run_inits(ldr_module_handle handle, entry_pt_kind kind);
static int rcoff_cleanup(ldr_module_handle mod);
static int rcoff_unload(ldr_module_handle handle, ldr_region_allocs *allocps,
			   int reg_count, ldr_region_rec *regions,
			   int ipkg_count, ldr_package_rec *import_pkgs,
			   int import_count, ldr_symbol_rec *imports,
			   int epkg_count, ldr_package_rec *export_pkgs);
static int rcoff_unres(ldr_context *context, ldr_package_rec *pkg,
		       ldr_symbol_rec *sym);

/* If we're linked with the kernel server, pickup the default package name */

char *kls_default_package_name;

#define PACKAGE_NAME \
	(kls_default_package_name ? kls_default_package_name : "kernel")

/* The loader switch entry for the rcoff manager */

const struct loader_switch_entry rcoff_switch_entry = {
	LSW_VERSION,
	LSF_MUSTOPEN,
	rcoff_recognizer,
	rcoff_get_static_dep,
	rcoff_get_imports,
	rcoff_map_regions,
	rcoff_get_export_pkgs,
	rcoff_get_exports,
	rcoff_lookup_export,
	rcoff_relocate,
	rcoff_get_entry_pt,
	rcoff_run_inits,
	rcoff_cleanup,
	rcoff_unload,
	rcoff_unres,
};

static rcoff_module_handle_t first_module, last_module;

/*
 * The manager entry point is called with a pointer to a loader context.
 * It is responsible for linking its switch entry into the context's
 * switch (by calling ldr_switch_ins_tail()).  This procedure
 * allows dynamically-loaded auxiliary managers to be initialized in
 * the same way as statically-linked managers.
 */

int
ldr_rcoff_entry(ldr_context_t ctxt)
{
	return(ldr_switch_ins_tail(ctxt, (ldr_switch_t)&rcoff_switch_entry));
}


/*
 * This routine reads in and scans the symbol table to determine the
 * number of symbols which are imported and exported.
 */

static int scan_symtab(rcoff_module_handle_t module)
{
	struct filehdr	*filehdr;
	pEXTR		extr;
	HDRR		hdrr;
	size_t		size;
	int		rc, i;


	/* Read symbolic header */

	filehdr = (struct filehdr *)module->fhd;
	if (!filehdr->f_symptr) {
		return(LDR_SUCCESS);
	}
	if ((rc = ldr_lseek(module->fd, filehdr->f_symptr, LDR_L_SET)) < 0)
		return(rc);
	if ((rc = ldr_read(module->fd, (char *)&hdrr, sizeof(hdrr))) < 0)
		return(rc);

	/* Read external symbols */

	module->nsym = hdrr.iextMax;
	if (module->nsym <= 0) {
		return(LDR_SUCCESS);
	}
	if ((rc = ldr_lseek(module->fd, hdrr.cbExtOffset, LDR_L_SET)) < 0)
		return(rc);
	size = module->nsym * sizeof(*extr);
	if ((rc = ldr_malloc(size, LDR_RCOFF_T,
	    (univ_t *)&extr)) != LDR_SUCCESS)
		return(rc);
	module->symtab = (univ_t)extr;
	if ((rc = ldr_read(module->fd, (char *)extr, size)) < 0)
		goto scan_error;

	/* Read strings */

	if ((rc = ldr_lseek(module->fd, hdrr.cbSsExtOffset, LDR_L_SET)) < 0)
		goto scan_error;
	size = hdrr.issExtMax;
	if ((rc = ldr_malloc(size, LDR_RCOFF_T,
	    (univ_t *)&module->strtab)) != LDR_SUCCESS)
		goto scan_error;
	if ((rc = ldr_read(module->fd, module->strtab, size)) < 0)
		goto scan_error;

	/* Count up the imports and exports */

	for (i = 0; i < module->nsym; i++) {
		switch (extr[i].asym.sc) {
		case scUndefined:
		case scSUndefined:
			module->nimport++;
			break;

		default:
			module->nexport++;
			break;
		}
	}

	return LDR_SUCCESS;

	/* Free any allocated memory and return an error code */

scan_error:
	if (module->strtab != NULL)
		(void)ldr_free(module->strtab);
	if (module->symtab != NULL)
		(void)ldr_free(module->symtab);
	return rc;
}


/*
 * Look up a symbol that has been reported unresolved by
 * the general symbol resolution policy.  In kernel loading,
 * we pretend that all symbols resolve against "kernel" which
 * contains only the symbols in /vmunix.  Any drivers loaded
 * since then also export symbols from their own packages.
 * Drivers loaded later can find symbols loaded in previous
 * drivers packages.
 */
static int rcoff_unres(ldr_context *context,
		       ldr_package_rec *pkg,
		       ldr_symbol_rec *sym)
{
	int			rc;
	rcoff_module_handle_t	module;
	ldr_kpt_rec		*kpte;

	/* Try looking in all loaded modules */

	for (module = first_module; module; module = module->next) {
		pkg = module->package;
		if (rcoff_lookup_export(module, pkg, sym) == LDR_SUCCESS) {

			/* Found it; fill in exporting package and return */

			rc = ldr_kpt_lookup(context->lc_lpt, pkg, &kpte);
			if (rc != LDR_SUCCESS) {
				dprintf(("rcoff_unres: FAILED (%d)\n",
					rc));
				return(rc);
			}

			/* Found package in loaded package table */

			sym->ls_module = kpte->lkp_module;
			dprintf(("rcoff_unres: SUCCESS\n"));
			return(LDR_SUCCESS);
		}
	}
	dprintf(("rcoff_unres: ENOSYM\n"));
	return(LDR_ENOSYM);
}

/*
 * The recognizer routine checks to see whether the specified file
 * (opened for atleast O_READ|O_EXEC access mode on the file descriptor fd)
 * is of an object file format supported by this format-dependent 
 * manager.  It returns LDR_SUCCESS on success or a negative loader error
 * status on failure.  On success, the format-dependent manager's handle
 * is left in the handle variable.  Also, after a sucessful recognition,
 * the open file descriptor is the responsiblity of the format-dependent
 * manager; it is never used again by the format-independent manager.
 */

static int 
rcoff_recognizer(const char *filename, ldr_file_t fd, ldr_module_handle *mod)
{
	rcoff_module_handle_t module; 
	int rc;			/* return code */
	ldr_window_t *wp;	/* window pointer for file header */
	ldr_window_t *owp;	/* window pointer for optional header */
	ldr_window_t *swp;	/* window pointer for section headers */
	struct filehdr *hdr;	/* COFF file header */
	struct aouthdr *ohdr;	/* COFF optional header */
	struct scnhdr *shdr;	/* COFF section header */

	/* Window the header of file */

	wp = ldr_init_window(fd);
	hdr = (struct filehdr *)
		ldr_file_window(0, sizeof(struct filehdr), wp);
	if (hdr == NULL) {
		dprintf(("rcoff_recognizer: failure to window file header\n"));
		return LDR_ENOEXEC;
	}

	/* Check its magic number */

	if (!RCOFF_MAGICOK(hdr)) {
		ldr_unwindow(wp);
		return LDR_ENOEXEC;
	}

	rc = ldr_malloc(sizeof(struct rcoff_module_handle), LDR_RCOFF_T,
		       (univ_t *)&module);
	if (rc != 0) {
		dprintf(("rcoff_recognizer: alloc module handle failed\n"));
		ldr_unwindow(wp);
		return rc;
	}
	bzero((char *)module, sizeof(*module));
	dprintf(("rcoff_recognizer: handle=%x\n", module));

	/* Window the optional header */

	owp = ldr_init_window(fd);
	ohdr = (struct aouthdr *)ldr_file_window(
		sizeof(struct filehdr), hdr->f_opthdr, owp);
	if (ohdr == NULL) {
		dprintf(("rcoff_recognizer: failure to window optional header\n"));

		/* cleanup */
		ldr_unwindow(wp);
		rc = ldr_free(module);	/* should check rc */
		return LDR_ENOEXEC;
	}

	/* Window the section headers */

	swp = ldr_init_window(fd);
	shdr = (struct scnhdr *)ldr_file_window(
		sizeof(struct filehdr) + sizeof(struct aouthdr),
		sizeof(struct scnhdr) * hdr->f_nscns, swp);
	if (shdr == NULL) {
		dprintf(("rcoff_recognizer: failure to window section headers\n"));

		/* cleanup */
		ldr_unwindow(wp);
		ldr_unwindow(owp);
		rc = ldr_free(module);	/* should check rc */
		return LDR_ENOEXEC;
	}

	module->wpo = owp;
	module->ohd = (univ_t) ohdr;
	module->wps = swp;
	module->shd = (univ_t) shdr;
	module->fd = fd;	
	module->wp = wp;
	module->fhd = (univ_t) hdr;
	module->filename = ldr_strdup(filename);

	/* Scan the symbol table for imports/exports */

	rc = scan_symtab(module);
	if (rc != LDR_SUCCESS) {
		ldr_unwindow(wp);
		ldr_unwindow(owp);
		ldr_unwindow(swp);
		ldr_free(module);
		return rc;
	}

	/* Link the module into the list */

	if (!first_module) {
		first_module = module;
		last_module = module;
	}
	else {
		last_module->next = module;
		module->prev = last_module;
		last_module = module;
	}

	*mod = (univ_t)module;

	dprintf(("rcoff_recognizer: headers windowed: fh 0x%x fhs %d oh 0x%x ohs %d sh 0x%x shs %d\n",
		 hdr, sizeof(struct filehdr),
		 ohdr, sizeof(struct aouthdr),
		 shdr, sizeof(struct scnhdr)));

	return LDR_SUCCESS;
}


/*
 * Map the regions of the object file into the process' address space.
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
 * allocsp is a pointer to a structure holding addresses of allocation
 * and deallocation procedures to use; see ldr_types.h for description.
 * Returns the number of regions in the region list in *reg_count.
 * Return LDR_SUCCESS on success or negative error status on error.
 */

static int 
rcoff_map_regions(ldr_module_handle handle, ldr_region_allocs *allocps,
		     int *reg_count, ldr_region_rec **regions)
{
	int rc;				/* return code */
	struct filehdr *fhdr;		/* file header */
	struct aouthdr *ohdr;		/* optional header */
	ldr_region_rec *reg_list;	/* regions list */
	int relocatable;		/* true if image is relocatable */
	int nregions;			/* temp for region count */
	int i;
	long bss_start, bss_end;	/* start and end of bss */
	rcoff_module_handle_t module 	/* switching types across interface */
		= (rcoff_module_handle_t)handle;


	dprintf(("rcoff_map_regions: entered; handle=%x\n", module));

	CHECK_RCOFF_HANDLE(module);

	fhdr = (struct filehdr *) module->fhd;
	ohdr = (struct aouthdr *) module->ohd;

	relocatable = FALSE;
	
	/* Do any special processing based on magic number */

	switch (ohdr->magic) {
	default:
		return(LDR_ENOEXEC);

	case OMAGIC:
		if ((fhdr->f_flags & F_RELFLG) == 0)
			relocatable = TRUE;
		break;

	case ZMAGIC:
	case NMAGIC:
		break;
	}

	/* Get the region count; it will either be 2 or 3 depending on whether
	 * there is a BSS region */

	bss_start = RCOFF_BSSSTART(fhdr, ohdr);
	bss_end = RCOFF_BSSEND(fhdr, ohdr);

	/* If no bss, no of regions is one less than usual */

	nregions = (bss_end <= bss_start) ? MAX_RCOFF_REGIONS - 1 :
	    MAX_RCOFF_REGIONS;

	dprintf(("rcoff_map_regions: no of regions = %d\n", nregions));

	/* Allocate the region list */

	if ((rc = ldr_regions_create(nregions, LDR_REGION_VERSION, &reg_list)) != LDR_SUCCESS) {

		dprintf(("rcoff_map_regions: region create error %d\n", rc));
		return(rc);
	}

	/* Now map the regions */

	for (i = 0; i < nregions; i++) {
		if ((rc = internal_map_region(module, allocps,
		    i, &reg_list[i], relocatable)) != LDR_SUCCESS) {
			(void)internal_free_regions(i - 1, reg_list, allocps);
			return(rc);
		}
	}

	*reg_count = nregions;
	*regions = reg_list;

	/* Map the entry point (this a nop if the image is absolute) */

	module->entry_pt = (ldr_entry_pt_t)((char *)RCOFF_ENTRYPT(fhdr, ohdr) +
	    ((char *)reg_list[TEXT_REGNO].lr_vaddr - (char *)RCOFF_TXTSTART(fhdr, ohdr)));

	return(LDR_SUCCESS);
}


/*
 * Map the specified region of the object module with the specified headers,
 * using the specified allocation procedures.  Return LDR_SUCCESS on success
 * or negative error status on error.
 */

static int 
internal_map_region(rcoff_module_handle_t module,
		    ldr_region_allocs *allocps, int regno,
		    ldr_region_rec *region, int relocatable)
{
	struct filehdr *fhdr;
	struct aouthdr *ohdr;
	int rc;				/* return code */
	long region_start;		/* map region to this address */
	long region_end;		/* end address of this region */
	long region_start_pad;
	long region_file_size;
	size_t rsize;			/* size of this region */
	long data_start;		/* for special case checks */
	long data_end;			/* for special case checks */
	int prot;			/* protection attributes for region */
	char rname[8];			/* region name */
	int found;			/* true/false flag */
	int i;				/* loop variable */
	size_t pagesize;		/* VM page size */
	off_t off, noff;		/* offset into file to be mapped, new offset  */
	ldr_prot_t rprot;		/* region protection */
	univ_t baseaddr;		/* base address for mapping */
	univ_t mapaddr;			/* address it ended up mapped at */
	int mapflags;			/* flags for ldr_mmap */
	int mapfd;
	off_t mapoff;

	fhdr = (struct filehdr *)module->fhd;
	ohdr = (struct aouthdr *)module->ohd;
	mapfd = module->fd;
	pagesize = ldr_getpagesize();

	switch(regno) {
	case TEXT_REGNO: 
		region_start = RCOFF_TXTSTART(fhdr, ohdr);
		region_end = RCOFF_TXTEND(fhdr, ohdr);
		region_start_pad = ohdr->text_start - region_start;
		region_file_size = ohdr->tsize;
		off = RCOFF_TXTOFF(fhdr, ohdr);
		rprot = LDR_R | LDR_X;
		if (ohdr->magic == ZMAGIC) {

			/* Special case: if text overlaps part of data, map
			 * that piece of the text region as part of the data
			 * region instead.  Sigh...
			 */

			data_start = RCOFF_DATASTART(fhdr, ohdr);
			if (region_end > data_start && region_start < data_start)
				region_end = data_start;

			mapflags = LDR_MAP_FILE;
			mapoff = off;
			prot = LDR_PROT_READ | LDR_PROT_EXEC;
		} else {
			mapflags = LDR_MAP_ANON;
			mapfd = LDR_FILE_NONE;
			mapoff = 0;
			prot = LDR_PROT_READ | LDR_PROT_EXEC | LDR_PROT_WRITE;
		}
		strcpy(rname, TEXT_REGION);

		/* If this object has imports then allocate extra space
		 * in the text segment for transfer vectors
		 */

		if (module->nimport) {
			region_end = round(region_end +
			    module->nimport * sizeof(vector_t), pagesize);
		}
		break;

	case DATA_REGNO:
		region_start = RCOFF_DATASTART(fhdr, ohdr);
		region_end = RCOFF_DATAEND(fhdr, ohdr);
		region_start_pad = ohdr->data_start - region_start;
		region_file_size = ohdr->dsize;
		strcpy(rname, DATA_REGION);
		prot = LDR_PROT_READ | LDR_PROT_WRITE;
		off = RCOFF_DATAOFF(fhdr, ohdr);
		rprot = LDR_R | LDR_W;
		if (ohdr->magic == ZMAGIC) {
			mapflags = LDR_MAP_FILE;
			mapoff = off;
		} else {
			mapflags = LDR_MAP_ANON;
			mapfd = LDR_FILE_NONE;
			mapoff = 0;
		}
		break;

	case BSS_REGNO:
		region_start = RCOFF_BSSSTART(fhdr, ohdr);
		region_end = RCOFF_BSSEND(fhdr, ohdr);
		region_file_size = 0;
		if (ohdr->magic == ZMAGIC) {

			/* Special case: if BSS overlaps part of data, map
			 * that piece of BSS as part of the data region
			 * instead. Sigh...
			 */

			data_end = RCOFF_DATAEND(fhdr, ohdr);
			if (data_end > region_start && data_end < region_end)
				region_start = data_end;
		}

		strcpy(rname, BSS_REGION);   /* section named for consistency */
		prot = LDR_PROT_READ | LDR_PROT_WRITE;
		mapoff = 0;
		rprot = LDR_R | LDR_W;
		mapflags = LDR_MAP_ANON;
		mapfd = LDR_FILE_NONE;
		break;

	default:
		dprintf(("rcoff_map_regions: invalid region no %d in switch\n",regno));
		return LDR_EINVAL;
	}

	rsize = region_end - region_start;

	/* Special-case for old binaries where file offset is not page-aligned; tell
	 * mmap to bypass alignment restrictions.
	 */

	if (ohdr->magic == ZMAGIC && !aligned(mapoff, pagesize))
		mapflags |= LDR_MAP_UNALIGNED;

	/* Determine address at which region should be mapped */

	if (!relocatable) {
		dprintf(("rcoff_map_regions: absolute region (%d)\n", regno));

		if ((rc = (*(allocps->lra_abs_alloc))((univ_t)region_start,
		    rsize, rprot, &baseaddr)) != LDR_SUCCESS)
			return(rc);

		mapflags |= (baseaddr == (univ_t)region_start) ?
		    (LDR_MAP_FIXED | LDR_MAP_PRIVATE) : LDR_MAP_PRIVATE;
	} else {
		dprintf(("rcoff_map_regions: relocatable region (%d)\n", regno));

		if ((rc = (*(allocps->lra_rel_alloc))(rsize, rprot,
		    (univ_t *)&region_start, &baseaddr)) != LDR_SUCCESS)
			return(rc);

		mapflags |= LDR_MAP_PRIVATE;
	}

	/* Don't even try to map a zero-length region.  We'll still list it in
	 * the region list (it's too late to do anything else at this point),
	 * but we can't actually map it.
	 */

	if (rsize != 0) {
		rc = ldr_mmap(baseaddr, rsize, prot, mapflags, mapfd, mapoff, &mapaddr);
		if (rc != 0) {
			dprintf(("rcoff_map_regions: failure to map %s segment rc = %d\n", rname, rc));
			return rc;
		}

		if (ohdr->magic != ZMAGIC && region_file_size != 0) {
			rc = ldr_lseek(module->fd, off, LDR_L_SET);
			if (rc != off) {
				dprintf(("rcoff_map_regions: failure to seek to region\n"));
				return rc;
			}
			rc = ldr_read(module->fd, 
			    (char *)mapaddr + region_start_pad,
			    region_file_size);
			if (rc != region_file_size) {
				dprintf(("rcoff_map_regions: failure to read region\n"));
				return rc;
			}
		}

		if (region_start == NULL) /* Only happens for relocatable regions */
			region_start = (long)mapaddr;

		dprintf(("rcoff_map_regions: mapped %s at 0x%x vaddr=0x%x size=%d prot=0x%x fd=%d off=0x%x\n",
		    rname, mapaddr, region_start, rsize, prot, mapfd, mapoff));
	}
		
	/* Region mapped successfully; return all the information in the
	 * region record.
	 */

	region->lr_name = ldr_strdup(rname);
	region->lr_vaddr = (univ_t) region_start;
	region->lr_mapaddr = mapaddr;
	region->lr_size = rsize;
	region->lr_prot = rprot;

	return LDR_SUCCESS;
}



/*
 * Return the address of the entry point of the specified module, if
 * any, in *entry_pt.  Return LDR_SUCCESS on success or negative
 * error status on error.
 */

static int 
rcoff_get_entry_pt(ldr_module_handle mod, ldr_entry_pt_t *entry_pt)
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)mod;


	CHECK_RCOFF_HANDLE(module);
	
	if (module->entry_pt == NULL)
		return LDR_ENOMAIN;

	*entry_pt = module->entry_pt;

	dprintf(("rcoff_get_entry_pt: ok entry pt = 0x%x\n", *entry_pt));

	return LDR_SUCCESS;
}


/*
 * Complete the loading of the specified module, clean up open files,
 * temporary data structures, etc.  Return LDR_SUCCESS on success or
 * negative error status on error.
 */

static int 
rcoff_cleanup(ldr_module_handle mod)
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)mod;
	int rc;

	CHECK_RCOFF_HANDLE(module);

	/* Unwindow file header */

	if (module->fhd != NULL) {
		ldr_unwindow(module->wp);
		module->fhd = NULL;
	}

	/* If optional aout header exists, unwindow it */

	if (module->ohd != NULL) {
		ldr_unwindow(module->wpo);
		module->ohd = NULL;
	}

	/* If section headers exist, unwindow them */

	if (module->shd != NULL) {
		ldr_unwindow(module->wps);
		module->shd = NULL;
	}

	if (module->symtab != NULL) {
		ldr_free(module->symtab);
		module->symtab = NULL;
	}

	if (module->strtab != NULL) {
		ldr_free(module->strtab);
		module->strtab = NULL;
	}

	if (module->import_map != NULL) {
		ldr_free(module->import_map);
		module->import_map = NULL;
	}

	/* Don't free module->export_list here, it may be needed later */

	if ((rc = ldr_close(module->fd)) != 0) {
		return rc;
	}

	module->fd = LDR_FILE_NONE;	/* mark module handle file closed */

	/* Throw away any other temporary data structures */

	dprintf(("rcoff_cleanup: cleanup done\n"));

	return LDR_SUCCESS;
}


/*
 * Unmap the first nregions regions described by the specified list of
 * region records.  Don't give up if one unmap fails, but continue on
 * and unmap as many of the regions as possible.  Then free the
 * list of region records.  Note that nregions may be less than the
 * size of the region list, if we're being called as a result of a
 * map_regions error.
 */

static int
internal_free_regions(int nregions, ldr_region_rec *regions,
		      ldr_region_allocs *allocps)
{
	int		regno;
	int		rc, rrc;

	rc = LDR_SUCCESS;

	for (regno = 0; regno < nregions; regno++) {
		rrc = internal_unload_region(regno, &regions[regno], allocps);
		if (rc == LDR_SUCCESS) rc = rrc;
		if (regions[regno].lr_name != NULL)
			(void)ldr_free(regions[regno].lr_name);
	}

	rrc = ldr_regions_free(nregions, regions);
	if (rc == LDR_SUCCESS) rc = rrc;
	return(rc);
}


/*
 * Unload the specified region of the specified object module. Return 
 * LDR_SUCCESS on success or negative error status on error.
 */

static int 
internal_unload_region(int regno, ldr_region_rec *region, ldr_region_allocs
		       *allocps)
{
	int rc;

	/* Unmap region (but not if it's zero-length) */

	if (region->lr_size != 0) {
		if ((rc = ldr_munmap(region->lr_mapaddr, region->lr_size)) != 0) {
			dprintf(("rcoff_unload_region: failure to ldr_munmap region\n"));
			return rc;
		}
	}

	dprintf(("rcoff_unload_region: ok ldr_munmap done\n"));

	/* Deallocate space on return */

	return((*(allocps->lra_dealloc))(region->lr_vaddr, region->lr_mapaddr,
					 region->lr_size));
}


/* 
 * Free the package and symbols lists.
 */

static int
internal_free_pkg_and_syms(int ipkg_count, ldr_package_rec *import_pkgs,
			   int import_count, ldr_symbol_rec *imports,
			   int epkg_count, ldr_package_rec *export_pkgs)
{
	int rc;				/* return code/status */
	int src = 0;			/* saved return code/status */

	rc = ldr_packages_free(ipkg_count, import_pkgs);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_free_pkg_and_syms: error freeing import package records\n"));
		src = rc;
	}

	rc = ldr_packages_free(epkg_count, export_pkgs);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_free_pkg_and_syms: error freeing export package records\n"));
		src = rc;
	}

	rc = ldr_symbols_free(import_count, imports);
	if (rc != LDR_SUCCESS) {
		dprintf(("internal_free_pkg_and_syms: error freeing ldr_symbol_rec records\n"));
		src = rc;
	}

	return src;
}


/*
 * Unload the specified module.  Unmap all regions, clean up open files,
 * temporary data structures, etc.  This should include deallocating 
 * the module's export list.  Once this routine is complete, the 
 * ldr_module_handle is no longer usable.  Return LDR_SUCCESS on success
 * or negative error status on error. NOTE: this routine is also used
 * to abort the loading of a module when a load error has occurred.
 */

static int 
rcoff_unload(ldr_module_handle handle, ldr_region_allocs *allocps,
	    int reg_count, ldr_region_rec *regions,
	    int ipkg_count, ldr_package_rec *import_pkgs,
	    int import_count, ldr_symbol_rec *imports,
	    int epkg_count, ldr_package_rec *export_pkgs)
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)handle;
	rcoff_module_handle_t m;
	int rc, rrc;
	int idx;
	char *name;
	univ_t ignore;

	CHECK_RCOFF_HANDLE(module);

	rc = LDR_SUCCESS;

	/* Unmap all regions */

	rrc = internal_free_regions(reg_count, regions, allocps);
	if (rc == LDR_SUCCESS) rc = rrc;

	/* Free import/export symbol and package lists */

	rrc = internal_free_pkg_and_syms(ipkg_count, import_pkgs,
	    import_count, imports, epkg_count, export_pkgs);
	if (rc == LDR_SUCCESS) rc = rrc;

	/* Remove from doubly linked list */
	if (module->prev)
	    module->prev->next = module->next;
	else
	    first_module = module->next;

	if (module->next)
	    module->next->prev = module->prev;
	else
	    last_module = module->prev;

	/* Deallocate module's import and export packages and lists */
	
	/* Unwindow file header */

	if (module->fhd != NULL)
		ldr_unwindow(module->wp);

	/* If optional aout header exists, unwindow it */

	if (module->ohd != NULL) 
		ldr_unwindow(module->wpo);

	/* If section headers exist, unwindow them */

	if (module->shd != NULL)
		ldr_unwindow(module->wps);

	if (module->symtab != NULL)
		ldr_free(module->symtab);

	if (module->strtab != NULL)
		ldr_free(module->strtab);

	if (module->import_map != NULL)
		ldr_free(module->import_map);

	if (module->filename != NULL)
		ldr_free(module->filename);

	/* Free module->export_list and strings that are keys to that table */

	if (module->export_list != NULL) {
		idx = 0;
		while ((rrc = open_hash_elements(module->export_list, &idx,
				  (univ_t)&name, &ignore) == LDR_SUCCESS)) {
			if (name)
				ldr_free(name);
		}
		open_hash_destroy(module->export_list);
		if (rc == LDR_SUCCESS && rrc != LDR_EAGAIN) rc = rrc;
	}

	/* Close file if not previously closed */

	if (module->fd != LDR_FILE_NONE) {
		if ((rrc = ldr_close(module->fd)) != 0) {
			dprintf(("rcoff_finish_unload: error closing file\n"));
		}
		if (rc == LDR_SUCCESS) rc = rrc;
		module->fd = LDR_FILE_NONE;	/* mark file closed */
	}

	(void)ldr_free(module);	/* invalid to use module handle after this */

	dprintf(("rcoff_finish_unload: ok handle disposed\n"));

	return rc;
}


/*
 * Relocate all the relocatable addresses everywhere in the specified
 * object module. `regions' is the array of `nregions' region description
 * records describing the regions mapped from this object module, as
 * returned from the lsw_map_regions call. `packages' and `imports'
 * are arrays of `npackages' package records and `nimports' import records
 * (respectively) describing the packages and symbols imported by this
 * object module, as returned by the lsw_get_imports call.
 * All symbols have been resolved to a symbol value.
 * Return LDR_SUCCESS on success or negative error status on error.
 *
 * NOTE: relocation is a machine dependent operation and hence the
 * routine rcoff_dep_relocation_section() is machine dependent.
 */

static int 
rcoff_relocate(ldr_module_handle handle, int nregions, ldr_region_rec regions[],
	      int npackages, ldr_package_rec packages[],
	      int nimports, ldr_symbol_rec imports[])
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)handle;
	int i;
	int rc;
	struct scnhdr *shdr;
	struct filehdr *fhdr;


	dprintf(("rcoff_relocate: entered; handle=%x\n", module));

	CHECK_RCOFF_HANDLE(module);

	shdr = (struct scnhdr *)module->shd;
	fhdr = (struct filehdr *)module->fhd;

	/* Do relocation by coff sections */

	for (i = 0; i < fhdr->f_nscns; i++) {

		/* Check if section needs relocation */

		if (shdr[i].s_nreloc != 0) {
			dprintf(("rcoff_relocate: region %d (%s) needs relocation\n", i, shdr[i].s_name));

			if ((rc = rcoff_relocate_section(module, nregions,
			    regions, nimports, imports, i)) != LDR_SUCCESS) {
				dprintf(("rcoff_relocate: failure to relocate section\n"));
				return rc;
			}
		} else {
			dprintf(("rcoff_relocate: section %d (%s) needs no relocation\n", i, shdr[i].s_name));
		}
	}

	/* If there are imports then initialize the transfer vectors */

	if (module->nimport) {
		vector_t *vec;

		vec = (vector_t *)((char *)regions[TEXT_REGNO].lr_mapaddr +
		    regions[TEXT_REGNO].lr_size -
		    module->nimport * sizeof(vector_t));
		rc = rcoff_init_vectors(vec, nimports, imports);
		if (rc != LDR_SUCCESS) {
			dprintf(("rcoff_relocate: failure to relocate transfer vectors\n"));
			return rc;
		}
	}

	/* Finally reset protections on any regions that had write enabled
	 * for relocations, etc.
	 */

	for (i = 0; i < nregions; i++) {
		int prot;

		if ((regions->lr_prot & LDR_W) == 0) {
			prot = LDR_PROT_NONE;;
			if (regions->lr_prot & LDR_R)
				prot |= LDR_PROT_READ;
			if (regions->lr_prot & LDR_X)
				prot |= LDR_PROT_EXEC;

			/* Use kloadcall() to reprotect segment, rather than
			 * mprotect().  This assumes the rcoff calls are used
			 * only by kloadsrv.  Qar 2939: lowell@krisis:
			 * 3/17/92 
			 */
			prot = convert_ldr_prot_to_vm_prot(prot);
			rc = kls_vm_protect(regions->lr_mapaddr, 
					    regions->lr_size,
					    FALSE,
					    prot);
			if (rc != LDR_SUCCESS) {
				dprintf(("rcoff_relocate: failure to reset write protection on region %d\n",
				    i));
				return rc;
			}
		}
		regions++;
	}

	return(LDR_SUCCESS);
}


/* Build an open hash table for the symbols exported by this module. */

static int build_export_list(rcoff_module_handle_t module)
{
	open_hashtab_t	table;
	pEXTR		extr, ex;
	char		*name;
	size_t		size;
	int		rc, i;
	ldr_symval	*export_list, *sv;
	long		text_base;
	long		data_base;


	dprintf(("build_export_list: entered; handle=%x\n", module));

	/* Build export list */

	size = module->nexport * sizeof(*export_list);
	if ((rc = ldr_malloc(size, LDR_RCOFF_T,
	    (univ_t *)&export_list)) != LDR_SUCCESS)
		return(rc);

	if ((rc = open_hash_create(module->nexport, (ldr_hash_p)hash_string,
	    (ldr_hash_compare_p)strcmp, (open_hash_flags_t)0, &table)) != LDR_SUCCESS)
		return(rc);

	extr = (pEXTR)module->symtab;

	text_base = ((struct aouthdr *)module->ohd)->text_start;
	data_base = ((struct aouthdr *)module->ohd)->data_start;

	sv = export_list;
	for (i = 0; i < module->nsym; i++) {
		ex = &extr[i];

		switch (ex->asym.sc) {

		default:
			ldr_symval_make_abs(sv, (univ_t)ex->asym.value);
			break;

		case scText:
			ldr_symval_make_regrel(sv, TEXT_REGNO,
			    ex->asym.value - text_base);
			ldr_symval_make_function(sv);
			break;

		case scData:
		case scBss:
		case scSData:
		case scSBss:
		case scRData:
		case scCommon:
			ldr_symval_make_regrel(sv, DATA_REGNO,
			    ex->asym.value - data_base);
			ldr_symval_make_data(sv);
			break;

		case scUndefined:
		case scSUndefined:
			continue;
		}

		name = ldr_strdup(module->strtab + ex->asym.iss);

		dprintf(("build_export_list: %s:%s regno=%d offset=0x%x\n",
		    module->filename, name, sv->ls_regno, sv->ls_offset));

		if ((rc = open_hash_insert(table, (const univ_t)name,
		    (univ_t *)&sv)) != LDR_SUCCESS)
			return(rc);

		sv++;
	}

	module->export_list = table;

	return(LDR_SUCCESS);
}


/*
 * Return the list of packages exported by this object module.
 * The calle must fill in the following fields of each package record:
 *	- structure version number
 *	- export package name
 *	- export package kind
 * Return the number of exported packages in *count.
 * Return LDR_SUCCESS on success or negative error status on error.
 */

static int 
rcoff_get_export_pkgs(ldr_module_handle handle, int *count,
		     ldr_package_rec **packages)
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)handle;
	int rc;


	dprintf(("rcoff_get_export_pkgs: entered; handle=%x\n", module));

	CHECK_RCOFF_HANDLE(module);

	/* If no symbols exported, then there are no packages */

	if (module->nexport == 0) {
		*count = 0;
		*packages = NULL;
		return(LDR_SUCCESS);
	}

	/* Build the export list for use by rcoff_lookup_export() */

	if (module->export_list == NULL) {
		rc = build_export_list(module);
		if (rc != LDR_SUCCESS) {
			dprintf(("rcoff_get_export_pkgs: failure to build export list\n"));
			return rc;
		}
	}

	/* Allocate the one and only package */

	rc = ldr_packages_create(1, LDR_PACKAGE_VERSION, packages);
	if (rc != LDR_SUCCESS) {
		dprintf(("rcoff_get_export_pkgs: failure creating packages\n"));
		return rc;
	}

	module->package = *packages;
	(*packages)->lp_name = ldr_strdup(module->filename);

	*count = 1;

	return LDR_SUCCESS;
}


/*
 * Lookup the specified import symbol from the specified package in
 * the specified object module, and fill in its value in the import
 * symbol record. Can use the following fields in the import record:
 *	- symbol name
 * Must fill in the following fields in the import symbol record:
 *	- symbol value
 * Return LDR_SUCCESS on success or negative error status on error.
 */

static int 
rcoff_lookup_export(ldr_module_handle handle, ldr_package_rec *package,
		   ldr_symbol_rec *symbol)
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)handle;
	int rc;
	ldr_symval *sym;

	dprintf(("rcoff_lookup_export: %s:%s\n",
		 package->lp_name, symbol->ls_name));
	CHECK_RCOFF_HANDLE(module);

	/* Make sure there are export symbols and the package name is correct */
	dprintf(("rcoff_lookup_export: nexports=%d, filename=%s\n",
		 module->nexport, module->filename));

	/* For now (in kloadsrv), package name and filename
	 * may not be the same - this is okay, as we allow
	 * .o's to be loaded that depend on other loaded .o's.
	 * Neither .o has knowledge of the other, so we are
	 * actually forgoing the package mechanism.
	 */
	if (module->nexport == 0 || strcmp(package->lp_name, module->filename))
		return(LDR_ENOPKG);
/*
	if (module->nexport == 0)
		return(LDR_ENOPKG);
*/
	/* Make sure the export list is built */

	if (module->export_list == NULL) {
		dprintf(("rcoff_lookup_export: building export list\n"));
		rc = build_export_list(module);
		if (rc != LDR_SUCCESS) {
			dprintf(("rcoff_lookup_export: cannot build list\n"));
			return rc;
		}
	}

	/* Lookup the symbol */

	if ((rc = open_hash_lookup(module->export_list,
	    (const univ_t)symbol->ls_name, (univ_t *)&sym)) == LDR_SUCCESS) {
		symbol->ls_value = *sym;
	}
	dprintf(("rcoff_lookup_export: returns %d\n", rc));
	return(rc);
}


/*
 * Return the list of import packages and import symbols for the
 * specified module. The callee allocates the lists and their contents,
 * and will be responsible for freeing them. The callee must fill in
 * the following fields of each package record:
 *	- structure version number (compatibility check)
 *	- import package name
 *	- import package kind
 * and the following fields of each symbol record:
 *	- structure version number (compatibility check)
 *	- symbol name
 *	- import package number
 * Returns the number of packages in the package list in *pkg_count and
 * the number opf symbols in the import list in *sym_count.
 */

static int
rcoff_get_imports(ldr_module_handle handle, int *pkg_count,
		     ldr_package_rec **pkgs, int *sym_count,
		     ldr_symbol_rec **imports)
{
	rcoff_module_handle_t module = (rcoff_module_handle_t)handle;
	pEXTR		extr, ex;
	size_t		size;
	int		rc, i;
	ldr_symbol_rec	*import;
	int		*import_map;
	int		import_ndx;


	dprintf(("rcoff_get_imports: entered; handle=%x\n", module));

	/* This is real easy if there are no imports! */

	if ((*sym_count = module->nimport) == 0) {
		*pkg_count = 0;
		*pkgs = NULL;
		*imports = NULL;
		return(LDR_SUCCESS);
	}

	/* Allocate the import map */

	size = module->nsym * sizeof(int);
	if ((rc = ldr_malloc(size, LDR_RCOFF_T,
	    (univ_t *)&import_map)) != LDR_SUCCESS)
		return rc;
	module->import_map = import_map;

	/* Create the one fixed package */

	rc = ldr_packages_create(1, LDR_PACKAGE_VERSION, pkgs);
	if (rc != LDR_SUCCESS) {
		dprintf(("rcoff_get_imports: error creating import package list\n"));
		return rc;
	}
	*pkg_count = 1;

	(*pkgs)->lp_name = ldr_strdup(PACKAGE_NAME);

	/* Allocate buffer space for the symbols records */

	rc = ldr_symbols_create(module->nimport, LDR_SYMBOL_VERSION, imports);
	if (rc != LDR_SUCCESS) {
		dprintf(("rcoff_get_imports: error creating import symbols list\n"));
		(void)ldr_packages_free(1, *pkgs);
		return rc;
	}

	/* Fill in the symbol records */

	import = *imports;
	import_ndx = 0;
	extr = (pEXTR)module->symtab;
	for (i = 0; i < module->nsym; i++) {
		ex = &extr[i];

		if (ex->asym.sc == scUndefined | ex->asym.sc == scSUndefined) {
			import_map[i] = import_ndx++;

			import->ls_name = ldr_strdup(module->strtab + ex->asym.iss);
			import->ls_packageno = 0;
			ldr_symval_make_unres(&import->ls_value);

			dprintf(("rcoff_get_imports: %s:%s\n",
			    (*pkgs)->lp_name, import->ls_name));

			import++;
		}
	}

	return(LDR_SUCCESS);
}


/*
 * Iterator returning the path names of the static dependencies of the
 * object module with the specified format-dependent handle.
 * `depno' is the index of the dependency to be found, starting at zero.
 * Return pointer to path name of static dependency (as a ldr_strdup-ed
 * string; caller will ldr_free it) in *dep.
 * Returns LDR_SUCCESS on success, a negative loader error status
 * on error (including LDR_EAGAIN to indicate the end of the dependencies).
 */

static int 
rcoff_get_static_dep(ldr_module_handle handle, int depno, char **dep)
{

	/* Never any static dependencies for COFF */

	return(LDR_EAGAIN);
}


/*
 * Return the list of exported symbols for the specified object module.
 * The callee allocates the list and its contents (the list MUST be
 * allocated by calling ldr_symbols_create()), but the CALLER is responsible
 * for freeing the list.  The caller must have previously called the
 * coff_get_export_pkgs call to get the list of packages exported by
 * this module.  The callee must fill in the following fields of each
 * symbol record:
 *	- structure version number (compatibility check)
 *	- symbol name
 *	- export package number (in previously-obtained export pkg list)
 *	- symbol value
 * Returns the number of symbols in the export list in *sym_count.
 * Return LDR_SUCCESS on success or negative error status on error.
 *
 * This routine is not called by the format-independent manager in normal
 * module loading.  It is intended for use only when pre-loading modules,
 * and possibly to allow format-dependent managers such as ELF to implement
 * their own symbol resolution algorithms.  It builds its list of exports
 * from the export_list hash table stored in the handle.  In this way, this
 * routine can be called long after coff_cleanup has closed the file and got
 * rid of windows to the various sections of the coff file.
 */

static int
rcoff_get_exports(ldr_module_handle handle,int *sym_count,
		     ldr_symbol_rec **exports)
{
	dprintf(("rcoff_get_exports: returning 0\n"));
	*sym_count = 0;
	*exports = NULL;
	return(LDR_SUCCESS);
}


/*
 * Run the specified module's initialization or termination routines as
 * specified by the kind flag.
 * Return success or negative error status on error.
 */

static int 
rcoff_run_inits(ldr_module_handle handle, entry_pt_kind kind)
{

	/* Never any init/term routines in a COFF module */

	return(LDR_SUCCESS);
}
