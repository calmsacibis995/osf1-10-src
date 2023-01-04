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
static char	sccsid[] = "@(#)$RCSfile: coff_ldr.c,v $ $Revision: 4.2.5.6 $ (DEC) $Date: 1992/12/07 16:18:35 $";
#endif 
/*
 *
 * 01-Mar-1990, Ken Lesniak
 *	Major overhaul to add COFF shared library support, and ELF
 *	executable and shared library support.
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * coff_ldr.c
 * COFF loader routines
 *
 * Loader switch routines called by the object file
 * format independent loader when accessing COFF files.
 *
 * OSF/1 Release 1.0
 */

/* #define	DEBUG */
#define PARANOID

#include <sys/types.h>
#include <strings.h>

#include "coff_machdep.h"

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

#include "open_hash.h"
#include "coff_ldr.h"

#include "ldr_errno.h"

#ifndef TRUE
#define TRUE	1
#define	FALSE	0
#endif

#ifdef	DEBUG
#define	dprintf(x)	ldr_msg x
#else
#define	dprintf(x)
#endif

#define	round(x, s)	(((unsigned)(x) + (unsigned)(s) - 1) & ~((unsigned)(s) - 1))
#define	trunc(x, s)	(((unsigned)(x)) & ~((unsigned)(s) - 1))
#define aligned(x, s)	(((unsigned)(x) & ((unsigned)(s) - 1)) == 0)

/* Names of the sections to be loaded  */

#define	TEXT_REGION		".text"
#define DATA_REGION		".data"
#define BSS_REGION		".bss"

/* Macros to locate various COFF headers */

#define COFF_FILEHDR(p) ((struct filehdr *)(p))
#define COFF_AOUTHDR(p) ((struct aouthdr *)((char *)(p) + sizeof(struct filehdr)))
#define COFF_SCNHDR(p) ((struct scnhdr *)((char *)(p) + \
	sizeof(struct filehdr) + COFF_FILEHDR(p)->f_opthdr))

/* Macros to locate various ELF headers */

#define ELF_HEADER(p) ((Elf32_Ehdr *)(p))
#define ELF_PGMHDR(p) ((Elf32_Phdr *)((char *)(p) + ELF_HEADER(p)->e_phoff))
#define ELF_SCNHDR(p) ((Elf32_Shdr *)((char *)(p) + ELF_HEADER(p)->e_shoff))

/* The following mask identifies all the COFF sections that need to be
 * mapped in order to load a COFF shared object
 */

#define MAPPED_COFF_SECTIONS (STYP_DYNAMIC | STYP_PACKAGE | STYP_PACKSYM \
	| STYP_DYNSTR | STYP_DYNSYM)

/* Define a type to describe the infomation needed to map a region */

typedef struct {
	char		*rname;		/* region name */
	long		rstart;		/* starting address of region */
	size_t		rsize;		/* size of region */
	off_t		off;		/* offset into file to be mapped */
	int		prot;		/* mmap protection of region */
	int		mapflags;	/* mmap flags */
	ldr_prot_t	rprot;		/* region protection */
	int		fd;		/* file descriptor */
	univ_t		baseaddr;	/* base address for mapping */
	univ_t		mapaddr;	/* address it ended up mapped at */
} map_info_t;

/* Forward references */

static int map_regions(map_info_t *mi, int nregions,
	ldr_region_allocs *allocps, int map_PIC, int is_dynamic);

static int free_export_list(coff_module_handle_t module);

static int build_export_list(coff_module_handle_t module);

static int get_objfmtdep_info(coff_module_handle_t module, int is_ELF);

static int internal_free_regions(int nregions, ldr_region_rec *regions,
				ldr_region_allocs *allocps);

static int internal_unload_region(int regno, ldr_region_rec *region,
				  ldr_region_allocs *allocps);

static int internal_free_pkg_and_syms(int ipkg_count, ldr_package_rec *import_pkgs,
			   int import_count, ldr_symbol_rec *imports,
			   int epkg_count, ldr_package_rec *export_pkgs);

static int coff_recognizer(const char *filename, ldr_file_t fd,
			       ldr_module_handle *mod);

static int coff_get_static_dep(ldr_module_handle handle, int depno,
				   char **dep);

static int coff_get_imports(ldr_module_handle handle, int *pkg_count,
				ldr_package_rec **pkgs, int *sym_count,
				ldr_symbol_rec **imports);

static int coff_map_regions(ldr_module_handle handle, ldr_region_allocs
				    *allocps, int *reg_count,
				    ldr_region_rec **regions);

static int coff_get_export_pkgs(ldr_module_handle handle, int *count,
				    ldr_package_rec **packages);

static int coff_get_exports(ldr_module_handle handle, int *sym_count,
				ldr_symbol_rec **exports);

static int coff_lookup_export(ldr_module_handle handle,
				  ldr_package_rec *package,
				  ldr_symbol_rec *symbol);

static int coff_relocate(ldr_module_handle handle, int nregions,
			     ldr_region_rec regions[], int npackages,
			     ldr_package_rec import_pkgs[], int nimports,
			     ldr_symbol_rec imports[]);

static int coff_get_entry_pt(ldr_module_handle mod, ldr_entry_pt_t *entry_pt);

static int coff_run_inits(ldr_module_handle handle, entry_pt_kind kind);

static int coff_cleanup(ldr_module_handle mod);

static int coff_unload(ldr_module_handle handle, ldr_region_allocs *allocps,
			   int reg_count, ldr_region_rec *regions,
			   int ipkg_count, ldr_package_rec *import_pkgs,
			   int import_count, ldr_symbol_rec *imports,
			   int epkg_count, ldr_package_rec *export_pkgs);


/* The loader switch entry for the coff manager */

const struct loader_switch_entry coff_switch_entry = {
	LSW_VERSION,
	LSF_MUSTOPEN,
	coff_recognizer,
	coff_get_static_dep,
	coff_get_imports,
	coff_map_regions,
	coff_get_export_pkgs,
	coff_get_exports,
	coff_lookup_export,
	coff_relocate,
	coff_get_entry_pt,
	coff_run_inits,
	coff_cleanup,
	coff_unload,
};


/*
 * The manager entry point is called with a pointer to a loader context.
 * It is responsible for linking its switch entry into the context's
 * switch (by calling ldr_switch_ins_tail()).  This procedure
 * allows dynamically-loaded auxiliary managers to be initialized in
 * the same way as statically-linked managers.
 */

int
ldr_coff_entry(ldr_context_t ctxt)
{
	return(ldr_switch_ins_tail(ctxt, (ldr_switch_t)&coff_switch_entry));
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
coff_recognizer(const char *filename, ldr_file_t fd, ldr_module_handle *mod)
{
	coff_module_handle_t module; 
	int rc;			/* return code */
	ldr_window_t *wp;	/* window pointer for file header */
	univ_t p_raw;		/* pointer to raw object file data */
	int is_ELF;		/* True if ELF object */


	dprintf(("coff_recognizer: %s\n", filename));

	/* Map the first page of the object. The intent is to map all of
	 * the headers and dynamic information. However until we can read
	 * some of the headers, we don't know how much to map. We can
	 * however guarantee that all of the headers will be contained
	 * within the first page. Later, if we need to, we can adjust
	 * the mapping of some of the dynamic information was not contained
	 * within the orignal mapping.
	 */

	wp = ldr_init_window(fd);
	p_raw = ldr_file_window(0, ldr_getpagesize(), wp);
	if (p_raw == NULL) {
		dprintf(("coff_recognizer: failure to map first page of object\n"));
		return LDR_ENOEXEC;
	}

	/* Make sure we have a COFF or ELF file */

	if (COFF_MAGICOK(COFF_FILEHDR(p_raw))) {
		is_ELF = FALSE;
	} else if (IS_ELF((*ELF_HEADER(p_raw)))) {
		is_ELF = TRUE;
	} else {
		ldr_unwindow(wp);
		return LDR_ENOEXEC;
	}

	rc = ldr_malloc(sizeof(struct coff_module_handle), LDR_COFF_T,
		       (univ_t *)&module);
	if (rc != LDR_SUCCESS) {
		dprintf(("coff_recognizer: alloc module handle failed\n"));
		ldr_unwindow(wp);
		return rc;
	}
	bzero((char *)module, sizeof(struct coff_module_handle));
	dprintf(("coff_recognizer: handle=%x\n", module));

	module->fd = fd;	
	module->wp = wp;
	module->p_raw = p_raw;
#ifdef	PARANOID
	module->filename = ldr_strdup(filename);
#endif

	/* Get the object format dependent information */

	rc = get_objfmtdep_info(module, is_ELF);
	if (rc != LDR_SUCCESS) {
		dprintf(("coff_recognizer: failure getting object format dependent info\n"));

		/* cleanup */
		if (module->p_raw != NULL)
			ldr_unwindow(wp);
		ldr_free(module);
		return rc;
	}

	*mod = (univ_t)module;

	dprintf(("coff_recognizer: object file windowed at 0x%x\n", module->p_raw));

	return LDR_SUCCESS;
}


/*
 * This routine gets all object format dependent information from the
 * object file and stores it in the module handle
 */

static int
get_objfmtdep_info(coff_module_handle_t module, int is_ELF)
{
	unsigned long mapsize = 0;
	unsigned long dyn_offset = 0;
	Elf32_Dyn *dy;


	if (is_ELF) {
		Elf32_Ehdr *ehdr = ELF_HEADER(module->p_raw);
		Elf32_Phdr *phdr = ELF_PGMHDR(module->p_raw);
		int i;

		/* Make sure this is an acceptable object:
		 *	little endian header
		 *	machine type is MIPS
		 *	program headers are present
		 *	object is executable or dynamic object
		 */

		if ((ehdr->e_ident[EI_DATA] != ELFDATA2LSB) ||
		    (ehdr->e_machine != EM_MIPS) ||
		    (ehdr->e_phoff == 0) ||
		    (ehdr->e_phentsize == 0) ||
		    (ehdr->e_phnum <= 0) ||
		    (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN)) {
			return LDR_ENOEXEC;
		}

		/* Get the entry point */

		module->entry_pt = (ldr_entry_pt_t)ehdr->e_entry;

		/* Get the start address and size of the segments and
		 * determine how much of the object needs to be mapped
		 * to access all the dynamic information. Also, determine
		 * the offset to the .dynamic section.
		 */

		i = ehdr->e_phnum;
		do {
			switch (phdr->p_type) {
			case PT_LOAD:
				switch (phdr->p_flags & (PF_R + PF_W)) {
				case PF_R:		/* text */
					module->text_start = phdr->p_vaddr;
					module->text_size = phdr->p_filesz;
					break;

				case PF_R + PF_W:	/* data */
					module->data_start = phdr->p_vaddr;
					module->data_size = phdr->p_filesz;
					module->bss_start = phdr->p_vaddr +
					    phdr->p_filesz;
					module->bss_size = phdr->p_memsz -
					    phdr->p_filesz;
					break;
				}
				break;

			case PT_DYNAMIC:
				dyn_offset = phdr->p_offset;
				mapsize = phdr->p_offset + phdr->p_filesz;
				break;
			}

			phdr = (Elf32_Phdr *)((char *)phdr + ehdr->e_phentsize);
		} while (--i);

		/* Is this PIC code? */

		module->is_PIC = (ehdr->e_flags & EF_MIPS_PIC) != 0;
	} else {
		struct aouthdr *ohdr = COFF_AOUTHDR(module->p_raw);
		struct filehdr *fhdr = COFF_FILEHDR(module->p_raw);
		struct scnhdr *shdr = COFF_SCNHDR(module->p_raw);
		int i;

		/* We only handle demand-paged executables */

		if (ohdr->magic != ZMAGIC)
			return(LDR_ENOEXEC);

		/* Get the entry point */

		module->entry_pt = (ldr_entry_pt_t)COFF_ENTRYPT(fhdr, ohdr);

		/* Get the start address and size of the segments */

		module->text_start = ohdr->text_start;
		module->text_size = ohdr->tsize;
		module->data_start = ohdr->data_start;
		module->data_size = ohdr->dsize;
		module->bss_start = ohdr->bss_start;
		module->bss_size = ohdr->bsize;

		/* Is this PIC code? */

		module->is_PIC =
		    (fhdr->f_flags & F_MIPS_CALL_SHARED) == F_MIPS_SHARABLE;

		/* If this object is or uses shared libraries, determine
		 * how much of it needs to be mapped to access the
		 * dynamic information. Also, determine the offset to
		 * the .dynamic section data
		 */

		module->rdata_start = 0;

		for (i = fhdr->f_nscns; i > 0; i--) {
		    if (shdr->s_flags & STYP_RDATA) {
			module->rdata_start = shdr->s_vaddr &
			  ~(ldr_getpagesize() - 1);

			/* Size is calculated by subtracting the start of
			 * the rdata section rounded down to a page boundary
			 * from the end rounded up.
			 */

			module->rdata_size = ((shdr->s_vaddr + 
					       shdr->s_size + 
					       ldr_getpagesize() - 1) & 
					      ~(ldr_getpagesize() - 1)) -
						module->rdata_start;

			/* Fix for qar 3839:  If the rdata region has been
			 * placed in the data segment it does not require
			 * special rdata handling.  6/2/92 lowell@krisis.
			 */

			if (module->rdata_start >= module->data_start &&
			    module->rdata_start < (module->data_start +
						   module->data_size)) {
			    module->rdata_start = 0;
			    module->rdata_size = 0;
			}

		    }
		    if (fhdr->f_flags & F_MIPS_SHARABLE) {
			if (shdr->s_flags & STYP_DYNAMIC) {
			    dyn_offset = shdr->s_scnptr;
			}
			if (shdr->s_flags & MAPPED_COFF_SECTIONS) {
			    if (mapsize < shdr->s_scnptr + shdr->s_size)
			        mapsize = shdr->s_scnptr + shdr->s_size;
			}
		    }
		    shdr++;
		}
	}

	/* If no dynamic information then we're done */

	if (dyn_offset == 0) {
		return LDR_SUCCESS;
	}
	module->is_dynamic = TRUE;

	/* Make sure we have enough of the object file mapped */

	if (mapsize > ldr_getpagesize()) {
		univ_t p_raw;
		ldr_window_t *wp;

		dprintf(("get_objfmtdep: need %d bytes raw bytes mapped\n", mapsize));

		/* Undo the current mapping */

		ldr_unwindow(module->wp);
		module->p_raw = NULL;

		/* Map the file using the new size */

		wp = ldr_init_window(module->fd);
		p_raw = ldr_file_window(0, mapsize, wp);
		if (p_raw == NULL) {
			dprintf(("get_objfmtdep: unable to re-map object file\n"));
			return LDR_ENOEXEC;
		}
		module->wp = wp;
		module->p_raw = p_raw;
	}

	/* Get address of dynamic section */

	dy = (Elf32_Dyn *)((char *)module->p_raw + dyn_offset);

	/* Scan the dynamic section for interesting info */

	for (;;) {
		switch (dy->d_tag) {
		case DT_NULL:
			return LDR_SUCCESS;

		case DT_REL:
			module->d_rel = (Elf32_Rel *)dy->d_un.d_ptr;
			break;

		case DT_SYMTAB:
			module->d_symtab = (Elf32_Sym *)dy->d_un.d_ptr;
			break;

		case DT_RELSZ:
			module->d_relsz = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_INIT:
			module->d_init = (unsigned long)dy->d_un.d_ptr;
			break;

		case DT_FINI:
			module->d_fini = (unsigned long)dy->d_un.d_ptr;
			break;

		case DT_PLTGOT:
			module->d_got = (Elf32_Got *)dy->d_un.d_ptr;
			break;

		case DT_STRTAB:
			module->d_strtab = (char *)dy->d_un.d_ptr;
			break;

		case DT_STRSZ:
			module->d_strsz = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_LOCAL_GOTNO:
			module->d_locgotno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_UNREFEXTNO:
			module->d_unrefextno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_GOTSYM:
			module->d_gotsym = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_SYMTABNO:
			module->d_symtabno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_BASE_ADDRESS:
			module->d_base_address = (unsigned long)dy->d_un.d_ptr;
			break;

		case DT_MIPS_PACKAGE:
			module->d_package = (Elf32_Package *)dy->d_un.d_ptr;
			break;

		case DT_MIPS_PACKSYM:
			module->d_packsym = (Elf32_Word *)dy->d_un.d_ptr;
			break;

		case DT_MIPS_PACKAGENO:
			module->d_packageno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_IMPACKNO:
			module->d_impackno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_EXPACKNO:
			module->d_expackno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_IMPSYMNO:
			module->d_impsymno = (Elf32_Word)dy->d_un.d_val;
			break;

		case DT_MIPS_EXPSYMNO:
			module->d_expsymno = (Elf32_Word)dy->d_un.d_val;
			break;
		}

		++dy;
	}
	/*NOTREACHED*/
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
coff_map_regions(ldr_module_handle handle, ldr_region_allocs *allocps,
		     int *reg_count, ldr_region_rec **regions)
{
	int rc;				/* return code */
	ldr_region_rec *reg_list;	/* regions list */
	int nregions;			/* temp for region count */
	int i;
	coff_module_handle_t module 	/* switching types across interface */
		= (coff_module_handle_t)handle;
	map_info_t mapinfo[MAX_COFF_REGIONS]; /* mapping info */


	dprintf(("coff_map_regions: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* If no bss, no of regions is one less than usual */

	nregions = (module->bss_size == 0 ? (MAX_COFF_REGIONS -1) :
	    MAX_COFF_REGIONS);

	dprintf(("coff_map_regions: no of regions = %d\n", nregions));

	/* Allocate the region list */

	if ((rc = ldr_regions_create(nregions, LDR_REGION_VERSION, &reg_list)) != LDR_SUCCESS) {

		dprintf(("coff_map_regions: region create error %d\n", rc));
		return(rc);
	}

	/* Initialize mapping info for all the segments, even if they're
	 * not present in the object (it won't hurt)
	 */

	mapinfo[TEXT_REGNO].rname = TEXT_REGION;
	mapinfo[TEXT_REGNO].rstart = module->text_start;
	mapinfo[TEXT_REGNO].rsize = module->text_size;
	mapinfo[TEXT_REGNO].off = 0;
	mapinfo[TEXT_REGNO].prot = LDR_PROT_READ | LDR_PROT_EXEC;
	mapinfo[TEXT_REGNO].mapflags = LDR_MAP_FILE;
	mapinfo[TEXT_REGNO].rprot = LDR_R | LDR_X;
	mapinfo[TEXT_REGNO].fd = module->fd;

	mapinfo[DATA_REGNO].rname = DATA_REGION;
	mapinfo[DATA_REGNO].rstart = module->data_start;
	mapinfo[DATA_REGNO].rsize = module->data_size;
	mapinfo[DATA_REGNO].off = module->text_size;
	mapinfo[DATA_REGNO].prot = LDR_PROT_READ | LDR_PROT_WRITE | LDR_PROT_EXEC;
	mapinfo[DATA_REGNO].mapflags = LDR_MAP_FILE;
	mapinfo[DATA_REGNO].rprot = LDR_R | LDR_W|LDR_X;
	mapinfo[DATA_REGNO].fd = module->fd;

	mapinfo[BSS_REGNO].rname = BSS_REGION;   /* section named for consistency */
	mapinfo[BSS_REGNO].rstart = module->bss_start;
	mapinfo[BSS_REGNO].rsize = module->bss_size;
	mapinfo[BSS_REGNO].off = 0;
	mapinfo[BSS_REGNO].prot = LDR_PROT_READ | LDR_PROT_WRITE | LDR_PROT_EXEC;
	mapinfo[BSS_REGNO].mapflags = LDR_MAP_ANON;
	mapinfo[BSS_REGNO].rprot = LDR_R | LDR_W | LDR_X;
	mapinfo[BSS_REGNO].fd = LDR_FILE_NONE;

	/* Try loading the object at its linked address.
	 * If loading at the linked address fails and we're loading a shared
	 * object, load it wherever it will fit.
	 */

	rc = map_regions(mapinfo, nregions, allocps, module->is_PIC, module->is_dynamic);
	if (rc != LDR_SUCCESS) {
		dprintf(("coff_map_regions: failure to map regions\n"));
		return rc;
	}

	/* Regions mapped successfully; return all the information in the
	 * region record.
	 */

	for (i = 0; i < nregions; i++) {
		reg_list[i].lr_name = ldr_strdup(mapinfo[i].rname);
		reg_list[i].lr_vaddr = (univ_t)mapinfo[i].rstart;
		reg_list[i].lr_mapaddr = mapinfo[i].mapaddr;
		reg_list[i].lr_size = mapinfo[i].rsize;
		reg_list[i].lr_prot = mapinfo[i].rprot;
	}
	*reg_count = nregions;
	*regions = reg_list;

	/* Remember the actual loaded base address of the module */

	module->base_vaddress = reg_list[TEXT_REGNO].lr_vaddr;
	module->base_paddress = reg_list[TEXT_REGNO].lr_mapaddr;

	/* Map the entry point for a PIC object */

	if (module->is_PIC) {
		module->entry_pt = (ldr_entry_pt_t)((char *)module->entry_pt +
		    ((unsigned long)module->base_vaddress - module->d_base_address));
	}

	return(LDR_SUCCESS);
}


/*
 * Map the regions in the object into memory.
 * First we attempt to load the object at the base address determined at link 
 * time.  If this fails and "map_PIC" is TRUE, the text segment is loaded at 
 * an aribtrary address, and the remaining segments are loaded at absolute 
 * addresses, which we calculate, so that the relative positioning of the 
 * segments does not change.
 */

static int 
map_regions(map_info_t *mi_array, int nregions, ldr_region_allocs *allocps,
	    int map_PIC, int is_dynamic)
{
    int i;
    int rc;
    int mapflags;
    long diff;		/* diff between segs linked and mapped addr */
    int map_attempt;
    map_info_t *mi;
    long lowmem = 0;
    long highmem = 0;
    long availmem;

    /* Try to map the regions to their preloaded locations.  ldr_mmap() must
     * be called with the LDR_MAP_VARIABLE flag so that the mapping will be
     * guaranteed to fail if something is already mapped at the address
     * requested.
     */

    diff = 0;
    for (map_attempt = 0; map_attempt < 2; map_attempt++) {

	for (i = 0, mi = mi_array; i < nregions; i++, mi++) {

	    mi->mapaddr = 0;   

	    /* The first mapping attempt will attempt to use the preloaded
	     * addresses.  For shared-libraries the second mapping attempt
	     * will determine an address space at which the regions are guaranteed
	     * to be mappable.  The "diff" variable will hold the bias needed
	     * to convert from the preloaded to the new addresses.
	     */

	    mi->rstart += diff;

	    /* Reserve the address space */
	    rc = (*(allocps->lra_abs_alloc))((univ_t)mi->rstart,
					     mi->rsize, 
					     mi->rprot, 
					     &mi->baseaddr);
	    if (rc != LDR_SUCCESS) {
		mi->baseaddr = 0;
		dprintf(("map_regions: failure allocating memory for region\n"));
		break;
	    }

	    /* Map the region */

	    if (mi->rsize != 0) {

		mapflags = mi->mapflags | LDR_MAP_VARIABLE | LDR_MAP_PRIVATE;

		rc = ldr_mmap(mi->baseaddr, mi->rsize, mi->prot,
			      mapflags, mi->fd, mi->off, &mi->mapaddr);

		if (rc != LDR_SUCCESS) {
		    dprintf(("map_regions: failure mapping memory for region\n"));
		    break;
		}

		else if (mi->mapaddr != mi->baseaddr) {
		    dprintf(("baseaddr = %x rstart = %x mapaddr = %x\n",
			     mi->baseaddr, mi->rstart, mi->mapaddr));
		    break;
		}

		dprintf(("map_regions: mapped %s at 0x%x vaddr=0x%x size=%d prot=0x%x fd=%d off=%d\n",
			 mi->rname, mi->mapaddr, mi->rstart,
			 mi->rsize, mi->prot, mi->fd, mi->off));
	    }
	}

	/* If every region was mapped correctly return success. */

	if (i == nregions) {
	    return LDR_SUCCESS;
	}

	/* Unmap and deallocate any regions which were mapped successfully 
	 * starting with the region that caused the failure.
	 */

	for (; i >= 0; i--, mi--) {

	    /* Unmap the region */
	    if (mi->mapaddr) {
		ldr_munmap(mi->mapaddr, mi->rsize);
	    }

	    /* Un-reserve the address space */

	    if (mi->baseaddr) {
		(*(allocps->lra_dealloc))((univ_t)mi->rstart,
					  mi->mapaddr, mi->rsize);
	    }
		
	    /* Remove the address bias for PIC mappings */
	    if (map_attempt == 1) { 
		mi->rstart -= diff;
	    }
	}

	/* If we're mapping a shared library we can determine a different 
	 * address which the regions will be loadable.  If not, we must 
	 * exit here.
	 */

	if (!map_PIC)
	  break;

	/* Assuming that the regions failed to map due to an address
	 * collision, we must find an address space at which the library
	 * regions will be guaranteed to map without conflict.
	 *
	 * First calculate the range of addresses needed.
	 */

	for (i = 0, mi = mi_array; i < nregions; i++, mi++) {
	    if (!lowmem || mi->rstart < lowmem) {
		lowmem = mi->rstart;
	    }
	    if ((mi->rstart + mi->rsize) > highmem) {
		highmem = mi->rstart + mi->rsize;
	    }
	}

	/* Use the address range calculated to find an available chunk
	 * of space.
	 */

	rc = ldr_mmap(0, highmem - lowmem, PROT_NONE, 
		      MAP_ANONYMOUS | MAP_VARIABLE, -1, 0, &availmem);
	if (rc != LDR_SUCCESS)
	    break;
	if (ldr_munmap(availmem, highmem - lowmem) != LDR_SUCCESS)
	    break;
	    
	diff = availmem - lowmem;

    }

    /* Mapping failed.  Return error status. */

    return(rc);
}


/*
 * Return the address of the entry point of the specified module, if
 * any, in *entry_pt.  Return LDR_SUCCESS on success or negative
 * error status on error.
 */

int 
coff_get_entry_pt(ldr_module_handle mod, ldr_entry_pt_t *entry_pt)
{
	coff_module_handle_t module = (coff_module_handle_t)mod;

	dprintf(("coff_get_entry_pt: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);
	
	if (module->entry_pt == NULL)
		return LDR_ENOMAIN;

	*entry_pt = module->entry_pt;

	dprintf(("coff_get_entry_pt: ok entry pt = 0x%x\n", *entry_pt));

	return LDR_SUCCESS;
}


/*
 * Complete the loading of the specified module, clean up open files,
 * temporary data structures, etc.  Return LDR_SUCCESS on success or
 * negative error status on error.
 */

static int 
coff_cleanup(ldr_module_handle mod)
{
	coff_module_handle_t module = (coff_module_handle_t)mod;
	int rc;


	dprintf(("coff_cleanup: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* Unwindow headers */

	if (module->p_raw != NULL) {
		ldr_unwindow(module->wp);
		module->p_raw = NULL;
	}

	if ((rc = ldr_close(module->fd)) != 0) {
		return rc;
	}

	module->fd = LDR_FILE_NONE;	/* mark module handle file closed */

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
internal_unload_region(int regno, ldr_region_rec *region,
		       ldr_region_allocs *allocps)
{
	int rc;


	/* unmap region (but not if it's zero-length) */
	if (region->lr_size != 0) {
		if ((rc = ldr_munmap(region->lr_mapaddr, region->lr_size)) != 0) {
			dprintf(("coff_unload_region: failure to ldr_munmap region\n"));
			return rc;
		}
	}

	dprintf(("coff_unload_region: ok ldr_munmap done\n"));

	/* deallocate space on return */

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
coff_unload(ldr_module_handle handle, ldr_region_allocs *allocps,
	    int reg_count, ldr_region_rec *regions,
	    int ipkg_count, ldr_package_rec *import_pkgs,
	    int import_count, ldr_symbol_rec *imports,
	    int epkg_count, ldr_package_rec *export_pkgs)
{
	coff_module_handle_t module = (coff_module_handle_t)handle;
	int rc, rrc;


	CHECK_COFF_HANDLE(module);

	rc = LDR_SUCCESS;

	/* Unmap all regions */

	rrc = internal_free_regions(reg_count, regions, allocps);
	if (rc == LDR_SUCCESS) rc = rrc;

	/* Free import/export symbol and package lists */

	rrc = internal_free_pkg_and_syms(ipkg_count, import_pkgs,
	    import_count, imports, epkg_count, export_pkgs);
	if (rc == LDR_SUCCESS) rc = rrc;

	/* Unwindow headers */

	if (module->p_raw != NULL)
		ldr_unwindow(module->wp);

	/* Close file if not previously closed */

	if (module->fd != LDR_FILE_NONE) {
		if ((rrc = ldr_close(module->fd)) != 0) {
			dprintf(("coff_unload: error closing file\n"));
		}
		if (rc == LDR_SUCCESS) rc = rrc;
		module->fd = LDR_FILE_NONE;	/* mark file closed */
	}

	/* Free the export list here i.e. handle->export_list */

	if (free_export_list(handle) != LDR_SUCCESS) {
		ldr_msg("coff_unload: coff_ldr - error freeing export list\n");
		ldr_msg("coff_ldr - continuing unloading\n");
	}

#ifdef	PARANOID
	if (module->filename)
		(void)ldr_free(module->filename);
#endif
	(void)ldr_free(module);	/* invalid to use module handle after this */

	dprintf(("coff_unload: ok handle disposed\n"));

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
 * routine coff_dynamic_relocate() is machine dependent.
 */

static int 
coff_relocate(ldr_module_handle handle, int nregions, ldr_region_rec regions[],
	      int npackages, ldr_package_rec packages[],
	      int nimports, ldr_symbol_rec imports[])
{
	coff_module_handle_t module = (coff_module_handle_t)handle;
	int rc;


	dprintf(("coff_relocate: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* Check to see if relocations are needed */

	if (module->is_dynamic) {
		if ((rc = coff_dynamic_relocate(module, nimports, imports,
		    nregions, regions)) != LDR_SUCCESS) {
			dprintf(("coff_relocate: failure to relocate shared object\n"));
			return rc;
		}
	}

	return(LDR_SUCCESS);
}


/*
 * Run the specified module's initialization or termination routines as
 * specified by the kind flag.
 * Return success or negative error status on error.
 */

static int 
coff_run_inits(ldr_module_handle handle, entry_pt_kind kind)
{
        typedef void (*entry_pt_t)();
        entry_pt_t pfunc;
	coff_module_handle_t module = (coff_module_handle_t)handle;


	dprintf(("coff_run_inits: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* Get the address of the init or term function */

	switch (kind) {
	case init_routines:
		pfunc = (entry_pt_t)module->d_init;
		break;

	case term_routines:
		pfunc = (entry_pt_t)module->d_fini;
		break;

	default:
		ldr_msg("invalid routine type, loader can execute only initialization or termination functions\n");
		return LDR_EINVAL;
	}

	/* If there is a function, relocate its address and call it */

	if (pfunc) {
		pfunc = (entry_pt_t)((char *)pfunc +
		    ((unsigned long)module->base_paddress - module->d_base_address));
		dprintf(("coff_run_inits: calling function at 0x%x\n", pfunc));
#if 0
/* this is only need if the loader is not built as a shared library */
		coff_PIC_bridge(pfunc);
#else
		(*pfunc)();
#endif
	}

	return LDR_SUCCESS;
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
coff_get_static_dep(ldr_module_handle handle, int depno, char **dep)
{

	/* Never any static dependencies for COFF */

	return(LDR_EAGAIN);
}


/*
 * Return the list of import packages and import symbols for the
 * specified module. The callee allocates the lists and their contents,
 * and will be responsible for freeing them. The callee must fill in the
 * following fields of each package record:
 *	- structure version number (compatibility check)
 *	- import package name
 *	- import package kind
 * and the following fields of each symbol record:
 *	- structure version number (compatibility check)
 *	- symbol name
 *	- import package number
 * Returns the number of packages in the package list in *pkg_count and
 * the number of symbols in the import list in *sym_count.
 * Return LDR_SUCCESS on success or negative error status on error.
 */

int coff_get_imports(ldr_module_handle handle, int *pkg_count,
		     ldr_package_rec **pkgs, int *sym_count,
		     ldr_symbol_rec **imports)
{
	coff_module_handle_t module = (coff_module_handle_t)handle;
	int i;
	long diff;
	Elf32_Sym *dynsym;
	char *dynstr;
	Elf32_Package *package;
	Elf32_Word *packsym;
	int rc;
	ldr_symbol_rec *import;
#ifdef PARANOID
	int nsym = 0;
#endif


	dprintf(("coff_get_imports: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* If there are no imports then take the easy way out */

	if ((*pkg_count = module->d_impackno) == 0) {
		*pkgs = NULL;
		*sym_count = 0;
		*imports = NULL;
		return LDR_SUCCESS;
	}

	/* Get pointers to where the various dynamic data sections have
	 * have been mapped
	 */

	diff = (long)module->p_raw - module->d_base_address;

	dynsym = (Elf32_Sym *)((char *)module->d_symtab + diff);
	dynstr = (char *)((char *)module->d_strtab + diff);
	package = (Elf32_Package *)((char *)module->d_package + diff);
	packsym = (Elf32_Word *)((char *)module->d_packsym + diff);

	/* Allocate buffer space for the package records */

	rc = ldr_packages_create(module->d_impackno, LDR_PACKAGE_VERSION, pkgs);
	if (rc != LDR_SUCCESS) {
		dprintf(("coff_get_imports: error creating import pkg list\n"));
		(void)ldr_packages_free(module->d_impackno, *pkgs);
		return rc;
	}

	/* Fill in the package records for import packages.
	 * The import packages are the first entries in the package table.
	 */

	for (i = 0; i < module->d_impackno; i++) {
		(*pkgs)[i].lp_name = ldr_strdup(dynstr + package[i + 1].pkg_name);
	}

	/* Allocate buffer space for the symbol records */

	rc = ldr_symbols_create(module->d_impsymno, LDR_SYMBOL_VERSION, imports);
	if (rc != LDR_SUCCESS) {
		dprintf(("coff_get_imports: error creating import symbol list\n"));
		return rc;
	}
	import = *imports;
	*sym_count = module->d_impsymno;

	/* Fill in the symbol records */

#if PACKSYM_FIXED
	packsym += module->d_gotsym - module->d_unrefextno;
#else
	packsym += module->d_gotsym;
#endif
	for (i = module->d_gotsym; i < module->d_symtabno; i++) {
		int pkg_idx;

		pkg_idx = *packsym++;
		if (!(package[pkg_idx].pkg_flags & PKGF_IMPORT)) {
			continue;
		}

#ifdef PARANOID
		if (++nsym > module->d_impsymno) {
			ldr_msg("loader: *** too many imports -- ignoring rest\n");
			break;
		}
#endif

		import->ls_name = ldr_strdup(dynstr + dynsym[i].st_name);
		import->ls_packageno = pkg_idx - 1;
		if (package[pkg_idx].pkg_flags & PKGF_EXPORT) {
			if (dynsym[i].st_shndx == SHN_MIPS_TEXT) {
				ldr_symval_make_function(&import->ls_value);
			} else {
				ldr_symval_make_data(&import->ls_value);
			}
		} else {
			ldr_symval_make_unres(&import->ls_value);
		}

		dprintf(("coff_get_imports: %s:%s\n",
		    (*pkgs)[import->ls_packageno].lp_name,
		    import->ls_name));

		import++;
	}

#ifdef PARANOID
	if (nsym != module->d_impsymno) {
		ldr_msg("loader: *** botch build of import symbol list for %s\n",
		    module->filename);
	}
#endif

	return LDR_SUCCESS;
}


/*
 * Return the list of packages exported by this object module.
 * The callee allocates the list and its contents and will be responsible
 * for freeing it. The callee must fill in the following fields of
 * each package record:
 *	- structure version number
 *	- export package name
 *	- export package kind
 * Return the number of exported packages in *count.
 * Return LDR_SUCCESS on success or negative error status on error.
 */

static int 
coff_get_export_pkgs(ldr_module_handle handle, int *count,
		     ldr_package_rec **packages)
{
	coff_module_handle_t module = (coff_module_handle_t)handle;
	int rc;
	int i;


	dprintf(("coff_get_export_pkgs: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* If there are no export packages then save us some work */

	if ((*count = module->d_expackno) == 0) {
		*packages = NULL;
		return LDR_SUCCESS;
	}

	/* Build the export list for use by coff_lookup_export() */

	if (module->export_list == NULL) {
		rc = build_export_list(module);
		if (rc != LDR_SUCCESS) {
			dprintf(("coff_get_export_package: failure to build export list\n"));
			return rc;
		}
	}

	/* Allocate space for the export package records */

	rc = ldr_packages_create(*count, LDR_PACKAGE_VERSION, packages);
	if (rc != LDR_SUCCESS) {
		dprintf(("coff_get_export_pkgs: failure creating packages\n"));
		return rc;
	}

	/* Fill in the export package records */

	for (i = 0; i < *count; i++) {
		(*packages)[i].lp_name = ldr_strdup(module->expkgnam[i]);
	}

	return LDR_SUCCESS;
}


/*
 * Export symbols will be maintained as a linked list
 * in an open hash table.
 */

typedef struct symbol_val {
	struct symbol_val *sv_next;
	ldr_symval         sv_ldr_symval;
	int		   sv_pkg_no;
} symbol_val_t;

/*
 * Free the export list and its key value pairs.
 * NOTE : this assumes export_list is an open hash table.
 */

static int
free_export_list(coff_module_handle_t module)
{
	univ_t key;		/* pointer to key for hash table element */
	symbol_val_t *p;	/* ptr to returned value */
	int rc;			/* return code/status */
	int src = 0;		/* saved return code/status */
	int index = 0;		/* index for element in hash table */
	int i;


	/* Free the exported package name list */

	if (module->expkgnam != NULL) {
		for (i = 0; i < module->d_expackno; i++) {
			ldr_free(module->expkgnam[i]);
		}

		ldr_free(module->expkgnam);
		module->expkgnam = NULL;
	}

	/* If no export symbol table then done */

	if (module->export_list == NULL)
		return LDR_SUCCESS;

	while ((rc = open_hash_elements(module->export_list, &index, 
	    &key, (univ_t *)&p)) == 0) {
		ldr_free(key);
	}
	if (rc != LDR_EAGAIN) {
		ldr_msg("coff_ldr - error fetching element from hash table\n");
		src = rc;
	}

	rc = open_hash_destroy(module->export_list);
	if (rc !=  0) {
		ldr_msg("coff_ldr - error destroying open hash table\n");
		src = rc;
	}

	if (src == 0) {
		module->export_list = NULL;
		ldr_free(module->export_svp);
		module->export_svp = NULL;

		dprintf(("free_export_list: export list deallocated\n"));

		return LDR_SUCCESS;
	}
	else {
		return src;
	}
}


/*
 * Look up the specified import symbol from the specified packge in
 * the specified object module, and fill in its value in the import
 * symbol record.  Can use the following fields in the import record:
 *	- symbol name
 * Must fill in the following fields in the import symbol record:
 *	- symbol value
 * Return LDR_SUCCESS on success or negative error status on error.
 */

static int
coff_lookup_export(ldr_module_handle handle,
		   ldr_package_rec *package,
		   ldr_symbol_rec *symbol)
{
	char *name;				/* package/symbol name */
	symbol_val_t *svp;			/* used for hash table lookup */
	int rc;					/* return status/code */
	coff_module_handle_t module = 		/* switching types across interface */
		(coff_module_handle_t)handle;


	CHECK_COFF_HANDLE(module);

	/* If nothing is exported, take the easy way out */

	if (module->d_expsymno == 0) {
		return LDR_ENOSYM;
	}

	/* Just in-case coff_lookup_export is called before 
	 * coff_get_export_pkgs, we can build the export_list here.
	 * However, this may not always work as the file may have
	 * been closed and it may not be possible to map the file
	 * so late in the game.
	 */

	if (module->export_list == NULL) {
		rc = build_export_list(module);
		if (rc != LDR_SUCCESS) {
			dprintf(("coff_lookup_export: failure to build export list\n"));
			return rc;
		}
	}

	/* Lookup the symbol in the export list */

	name = symbol->ls_name;
	if ((rc = open_hash_lookup(module->export_list,
	    (const univ_t)name, (univ_t *)&svp)) != LDR_SUCCESS) {
		dprintf(("coff_lookup_export: hash lookup on key failed\n"));
		return rc;
	}

	/* Find the symbol with the correct package */

	name = package->lp_name;
	do {
		if (!strcmp(name, module->expkgnam[svp->sv_pkg_no])) {

			/* Check for type matches -- type mismatch is an error */

			if (!ldr_symval_type_check(&symbol->ls_value,
						   &svp->sv_ldr_symval))
				return(LDR_EVERSION);

			symbol->ls_value = svp->sv_ldr_symval;
			return LDR_SUCCESS;
		}
	} while (svp = svp->sv_next);

	return LDR_ENOSYM;
}


/* 
 * build_export_list builds an open hash table for the symbols
 * exported by this module.  Symbols with the same name result
 * to same hash-table bucket which are then chained. The package
 * name is stored in the symbol_val_t.
 */

static int
build_export_list(coff_module_handle_t module)
{
	char *symname;			/* symbol name */
	symbol_val_t *svp, *prev;	/* hash table elements */
	int rc;				/* return code/status */
	int i;				/* loop variable */
	long diff;
	Elf32_Sym *dynsym;
	char *dynstr;
	Elf32_Package *package;
	Elf32_Word *packsym;
	unsigned long expkg_idx;
#ifdef PARANOID
	int nsym = 0;
#endif


	dprintf(("build_export_list: entered; handle=%x\n", module));

	if (module->fd == LDR_FILE_NONE) {

		/* coff_cleanup has been done - too late to build export list.
		 * Ken doesn't know why this would happen, but mo_ldr checks
		 * for this condition
		 */

		return LDR_ENOEXEC;
	}

	if ((rc = open_hash_create(module->d_expsymno,
				   (ldr_hash_p)hash_string,
				   (ldr_hash_compare_p)strcmp, 
				   (open_hash_flags_t)0, 
				   &(module->export_list))) != LDR_SUCCESS) {
		dprintf(("build_export_list: failure creating export list\n"));
		return rc;
	}

	dprintf(("build_export_list: created hash table\n"));


	/* Get pointers to the various tables we need */

	diff = (long)module->p_raw - module->d_base_address;

	dynsym = (Elf32_Sym *)((char *)module->d_symtab + diff);
	dynstr = (char *)((char *)module->d_strtab + diff);
	package = (Elf32_Package *)((char *)module->d_package + diff);
	packsym = (Elf32_Word *)((char *)module->d_packsym + diff);

	/* Allocate buffer space for the package name list */

	rc = ldr_malloc(module->d_expackno * sizeof(char *), LDR_COFF_T,
	    (univ_t *)&module->expkgnam);
	if (rc != LDR_SUCCESS) {
		dprintf(("build_export_list: error creating export pkg name list\n"));
		free_export_list(module);
		return rc;
	}

	/* Fill in the export package name list */

	expkg_idx = module->d_packageno - module->d_expackno;
	for (i = 0; i < module->d_expackno; i++) {
		module->expkgnam[i] =
		    ldr_strdup(dynstr + package[i + expkg_idx].pkg_name);
	}

	/* Allocate symbol value records for all the exports */

	rc = ldr_malloc(module->d_expsymno * sizeof(symbol_val_t), LDR_COFF_T,
		(univ_t *)&svp);
	if (rc != LDR_SUCCESS) {
		dprintf(("build_export_list: allocate of symbol_val_t's failed\n"));
		free_export_list(module);
		return rc;
	}
	module->export_svp = (univ_t)svp;

	/* Enter the exported symbols into the open hash table */

#if !PACKSYM_FIXED
	packsym += module->d_unrefextno;
#endif
	for (i = module->d_unrefextno; i < module->d_symtabno; i++) {
		int pkg_idx;

		/* Get the index into the package table for this symbol */

		pkg_idx = *packsym++;

		/* Skip any symbol which is not exported */

		if (!(package[pkg_idx].pkg_flags & PKGF_EXPORT)) {
			continue;
		}

#ifdef PARANOID
		if (++nsym > module->d_expsymno) {
			ldr_msg("loader: *** too many exports -- ignoring rest\n");
			break;
		}
#endif

		/* `dup' the symbol name */

		symname = ldr_strdup(dynstr + dynsym[i].st_name);

		/* Initialize the symbol value record */

		svp->sv_next = NULL;
		svp->sv_pkg_no = pkg_idx - expkg_idx;
		if (dynsym[i].st_shndx == SHN_MIPS_TEXT) {
			ldr_symval_make_regrel(&svp->sv_ldr_symval,
			    TEXT_REGNO,
			    dynsym[i].st_value - module->d_base_address);
			ldr_symval_make_function(&svp->sv_ldr_symval);
		} else {
			ldr_symval_make_regrel(&svp->sv_ldr_symval,
			    DATA_REGNO,
			    dynsym[i].st_value - module->data_start);
			ldr_symval_make_data(&svp->sv_ldr_symval);
		}

		dprintf(("build_export_list: %s:%s regno=%d offset=0x%x\n",
		    module->expkgnam[svp->sv_pkg_no],
		    symname,
		    svp->sv_ldr_symval.ls_regno,
		    svp->sv_ldr_symval.ls_offset));

		/* Attempt to insert the new record into the hash table
		 * using the symbol name as the key.
		 */

		prev = svp;
		rc = open_hash_search(module->export_list, (const univ_t)symname,
				      (univ_t *) &prev, 
				      (LDR_HASH_INSERT | LDR_HASH_LOOKUP));
		if (rc != LDR_SUCCESS) {
			dprintf(("build_export_list: open_hash_search failed rc %d\n",
				 rc));
			free_export_list(module);
			return rc;
		}

		/* Symbol name already in table, chain to end of list */

		if (prev != svp) {
			while (prev->sv_next)
				prev = prev->sv_next;
			prev->sv_next = svp;

			/* Since open_hash_search didn't insert the record,
			 * the `dup'ed symbol name isn't needed
			 */

			ldr_free(symname);
		}

		svp++;
	}

#ifdef PARANOID
	if (nsym != module->d_expsymno) {
		ldr_msg("loader: *** botched build of export symbol list for %s\n",
		    module->filename);
	}
#endif
	
	dprintf(("build_export_list: export list built\n"));

	return LDR_SUCCESS;
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

static int coff_get_exports(ldr_module_handle handle, int *sym_count,
		      ldr_symbol_rec **exports)
{
	char *key;				/* symbol name will be returned here */
	symbol_val_t *svp;			/* export hash table elements */
	int rc;					/* return code/status */
	int i;
	open_hash_element_index index = 0; 	/* used by open_hash_elements to search hash table */
	coff_module_handle_t module = 		/* switch types across interface */
		(coff_module_handle_t)handle;


	dprintf(("coff_get_exports: entered; handle=%x\n", module));

	CHECK_COFF_HANDLE(module);

	/* If nothing is exported, take the easy eay out */

	if ((*sym_count = module->d_expsymno) == 0) {
		dprintf(("coff_get_exports: 0 symbols\n"));
		*exports = NULL;
		return LDR_SUCCESS;
	}

	/* Build the export list if it hasn't already been built */

	if (module->export_list == NULL) {
		rc = build_export_list(module);
		if (rc != LDR_SUCCESS) {
			dprintf(("coff_get_exports: failure building export list\n"));
			return rc;
		}
	}

	/* Allocate buffer space for symbol records */

	if ((rc = ldr_symbols_create(*sym_count, LDR_SYMBOL_VERSION,
	    exports)) != LDR_SUCCESS) {

		dprintf(("get_exports: failure creating export list\n"));
		return rc;
	}

	/* Sequentially step through the open hash table */

	for (i = 0;;) {

		/* Get next bucket from hash table */

		rc = open_hash_elements(module->export_list, &index,
		    (univ_t *)&key, (univ_t *)&svp);
		if (rc != LDR_SUCCESS) {
			ldr_msg("coff_get_exports: open_hash_elements error rc = %d\n",rc);
			return rc;
		}

		/* Walk the chained hash table linked list ... */

		do {

			/* Fill in the symbol record */

			(*exports)[i].ls_name = ldr_strdup((char *)key);
			(*exports)[i].ls_packageno = svp->sv_pkg_no;
			(*exports)[i].ls_value = svp->sv_ldr_symval;

			if (++i == *sym_count) {
				goto got_exports;
			}
		} while (svp = svp->sv_next);
	}

    got_exports:
	return LDR_SUCCESS;
}
