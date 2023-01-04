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
static char	*sccsid = "@(#)$RCSfile: dummy.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:36:16 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* dummy.c
 * Dummy format-dependent manager for testing
 *
 * This is a dummy format-dependent manager for testing the
 * format-independent parts of the loader.  It reads an ASCII
 * text file containing "load commands", and logs the format-
 * dependent operations it is performing to the tty.
 *
 * The format of the "object file" it reads is:
 * #! dummy
 * # Comments
 * region <name> <vaddr> <size> [text | data | bss]
 * picregion <name> <size> [text | data | bss]
 * dependency <name>
 * entry <vaddr>
 * import <package name> <symbol name> [text | data]
 * export_abs <package name> <symbol name> <value> [text | data]
 * export_rel <package name> <symbol name> <region no> <offset> [text | data]
 * reloc_region <region no> <offset> <from region> <from offset>
 * reloc_import <region no> <offset> <import no> <offset>
 *
 * Eventually we'll add other "load commands" for testing other
 * portions of the format-independent loader.
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <stdio.h>
#include <sys/errno.h>
#include <loader.h>

#include "ldr_types.h"
#include "ldr_errno.h"
#include "ldr_malloc.h"
#include "ldr_sys_int.h"
#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_switch.h"


#define	NREGS		16		/* maximum allowable # of regions */
#define	NDEPS		16		/* maximum allowable static dependencies */
#define NIMP		16		/* maximum allowable # of imports */
#define NEXP		16		/* maximum allowable # of exports */
#define NRELOC		16		/* maximum allowable # of relocations */

typedef struct dummy_region {
	char		*name;		/* region name */
	univ_t		vaddr;		/* virtual address */
	univ_t		mapaddr;	/* map address */
	size_t		size;		/* size */
	ldr_prot_t	prot;		/* protection */
	int		pic;		/* true iff a pic region */
} dummy_region;

typedef struct dummy_import {
	char		*name;		/* symbol name */
	int		pkgno;		/* package */
	ldr_symbol_kind_t kind;		/* text or data */
} dummy_import;

typedef struct dummy_export {
	char		*name;		/* symbol name */
	int		pkgno;		/* package */
	ldr_symval	value;		/* value */
} dummy_export;

enum reloc_kind { import, region };

typedef struct dummy_reloc {
	enum	reloc_kind kind;	/* relocation kind (region or import) */
	int		regno;		/* region of loc. to relocate */
	int		offset;		/* offset to relocate */
	int		from;		/* import or region no. of source */
	int		from_offset;	/* offset from source base */
} dummy_reloc;

typedef struct dummy {			/* format-dependent data */
	ldr_entry_pt_t	entry;		/* the entry point */
	int		nreg;		/* number of regions */
	dummy_region	regions[NREGS];	/* the regions */
	int		ndeps;		/* number of dependencies */
	char		*deps[NDEPS];	/* static dependencies */
	int		nipkg;		/* number of import pkgs */
	char		*ipkg[NIMP];	/* import packages */
	int		nimps;		/* number of import symbols */
	dummy_import	imports[NIMP];	/* import symbols */
	int		nepkg;		/* number of export pkgs */
	char		*epkg[NEXP];	/* export packages */
	int		nexps;		/* number of export symbols */
	dummy_export	exports[NEXP];	/* export symbols */
	int		nreloc;		/* number of relocations */
	dummy_reloc	relocs[NRELOC];	/* relocations */
} dummy;

/* Forward references */

static int parse_dummy_file(FILE *fp, dummy **dump);
static int import_pkgno(dummy *dum, char *name);
static int export_pkgno(dummy *dum, char *name);
static char *prots[] = { "", "x", "w", "wx", "r", "rx", "rw", "rwx" };

static int int_map_region(dummy *dum, ldr_region_allocs *allocsp, int regno, 
			  ldr_region_rec *region);
static int int_unload_region(dummy *dum, int regno, ldr_region_rec *region,
			     ldr_region_allocs *allocsp);
static int int_free_regions(dummy *dum, int nregions, ldr_region_rec *regions,
			    ldr_region_allocs *allocsp);

static int dummy_recog(const char *filename, ldr_file_t fd,
			   ldr_module_handle *handle);
static int dummy_get_static_dep(ldr_module_handle handle, int depno,
				    char **dep);
static int dummy_get_imports(ldr_module_handle handle, int *pkg_count,
				 ldr_package_rec **pkgs, int *sym_count,
				 ldr_symbol_rec **imports);
static int dummy_get_region_count(ldr_module_handle handle, int *count);
static int dummy_map_regions(ldr_module_handle handle,
				 ldr_region_allocs *allocsp, int *reg_count,
				 ldr_region_rec **regions);
static int dummy_get_export_pkgs(ldr_module_handle handle, int *count,
				     ldr_package_rec **packages);
static int dummy_get_exports(ldr_module_handle handle,int *sym_count,
				 ldr_symbol_rec **exports);
static int dummy_lookup_export(ldr_module_handle handle,
				   ldr_package_rec *package,
				   ldr_symbol_rec *symbol);
static int dummy_relocate(ldr_module_handle handle, int nregions,
			      ldr_region_rec regions[], int npackages,
			      ldr_package_rec import_pkgs[], int nimports,
			      ldr_symbol_rec imports[]);
static int dummy_get_entry_pt(ldr_module_handle handle,
				  ldr_entry_pt_t *entry_pt);
static int dummy_run_inits(ldr_module_handle handle, entry_pt_kind kind);
static int dummy_cleanup(ldr_module_handle handle);
static int dummy_unload(ldr_module_handle handle, ldr_region_allocs *allocsp,
			    int reg_count, ldr_region_rec *regions,
			    int ipkg_count, ldr_package_rec *import_pkgs,
			    int import_count, ldr_symbol_rec *imports,
			    int epkg_count, ldr_package_rec *export_pkgs);

/* The dummy loader switch entry */

const struct loader_switch_entry dummy_switch_entry = {
	LSW_VERSION,
	LSF_MUSTOPEN,
	dummy_recog,
	dummy_get_static_dep,
	dummy_get_imports,
	dummy_map_regions,
	dummy_get_export_pkgs,
	dummy_get_exports,
	dummy_lookup_export,
	dummy_relocate,
	dummy_get_entry_pt,
	dummy_run_inits,
	dummy_cleanup,
	dummy_unload,
};


int
ldr_dummy_entry(ldr_context_t ctxt)

/* The manager entry point is called with a pointer to a loader context.
 * It is responsible for linking its switch entry into the context's
 * switch (by calling ldr_switch_ins_tail()).  This procedure
 * allows dynamically-loaded auxiliary managers to be initialized in
 * the same way as statically-linked managers.
 */
{
	return(ldr_switch_ins_tail(ctxt, (ldr_switch_t)&dummy_switch_entry));
}


static int
dummy_recog(const char *filename, ldr_file_t fd, ldr_module_handle *handle)

/* Recognizer.  Compare the first line of the file to "#! dummy".
 * If it matches, allocate the format-dependent descriptor, fill it in,
 * and return it in *handle.  Return LDR_SUCCESS on success, or negative
 * error status on error (LDR_ENOEXEC if we don't recognize the format).
 */
{
	FILE		*fp;
	int		fd2;
	char		buf[BUFSIZ];
	dummy		*dum;
	int		rc;

	if ((fd2 = dup(fd)) < 0)
		return(ldr_errno_to_status(errno));

	if ((fp = fdopen(fd2, "r")) == NULL) {
		rc = ldr_errno_to_status(errno);
		goto cleanup;
	}

	if (fgets(buf, BUFSIZ, fp) == NULL) {
		rc = LDR_ENOEXEC;
		goto cleanup;
	}

	if (strcmp(buf, "#! dummy\n") != 0) {
/*		fprintf(stderr, "dummy: %s not recognized\n", buf); */
		rc = LDR_ENOEXEC;
		goto cleanup;
	}

	/* It's ours; parse the file */

	if ((rc = parse_dummy_file(fp, &dum)) != LDR_SUCCESS)
		goto cleanup;

	(void)ldr_close(fd);
	fd = LDR_FILE_NONE;
	fprintf(stderr, "dummy: recognized %s\n", filename);
	*handle = (univ_t)dum;
	rc = LDR_SUCCESS;

cleanup:
	fclose(fp);
	if (fd != LDR_FILE_NONE)
		(void)ldr_lseek(fd, 0, LDR_L_SET);
	return(rc);
}


static int
parse_dummy_file(FILE *fp, dummy **dump)

/* Allocate a descriptor, parse the file, and fill in the descriptor to
 * return.
 */
{
	dummy		*dum;
	char		buf[BUFSIZ];
	char		buf2[BUFSIZ];
	char		buf3[BUFSIZ];
	char		buf4[BUFSIZ];
	univ_t		vaddr;
	size_t		size;
	unsigned long	val;
	int		expreg;
	int		rel_regno;
	int		offset;
	int		from_regno;
	int		from_impno;
	int		from_offset;
	int		regno;
	int		depno;
	int		impno;
	int		expno;
	int		relocno;
	int		rc;
	extern char	*ldr_strdup(const char *str);

	if ((rc = ldr_malloc(sizeof(*dum), 0, (univ_t *)&dum)) != LDR_SUCCESS)
		return(rc);

	dum->entry = NULL;
	dum->nreg = regno = 0;
	dum->ndeps = depno = 0;
	dum->nipkg = 0;
	dum->nimps = impno = 0;
	dum->nepkg = 0;
	dum->nexps = expno = 0;
	dum->nreloc = relocno = 0;
	while (fgets(buf, BUFSIZ, fp) != NULL) {

		if (buf[0] == '#')	/* comment */
			continue;

		if (sscanf(buf, "region %s %x %d %s\n", buf3, &vaddr, &size,
			   buf2) == 4) {
			if (regno >= NREGS) {
				fprintf(stderr, "dummy: too many regions\n");
				goto cleanup;
			}
			dum->regions[regno].name = ldr_strdup(buf3);
			dum->regions[regno].vaddr = vaddr;
			dum->regions[regno].size = size;
			if (strcmp(buf2, "text") == 0) {
				dum->regions[regno].prot = LDR_R|LDR_X;
			} else if (strcmp(buf2, "data") == 0) {
				dum->regions[regno].prot = LDR_R|LDR_W|LDR_X;
			} else if (strcmp(buf2, "bss") == 0) {
				dum->regions[regno].prot = LDR_R|LDR_W|LDR_X;
			} else {
				fprintf(stderr, "dummy: unknown type %s\n", buf2);
				goto cleanup;
			}
			dum->regions[regno].pic = 0;
			dum->nreg = ++regno;
		} else if (sscanf(buf, "picregion %s %d %s\n", buf3, &size,
				  buf2) == 3) {
			if (regno >= NREGS) {
				fprintf(stderr, "dummy: too many regions\n");
				goto cleanup;
			}
			dum->regions[regno].name = ldr_strdup(buf3);
			dum->regions[regno].vaddr = 0;
			dum->regions[regno].size = size;
			if (strcmp(buf2, "text") == 0) {
				dum->regions[regno].prot = LDR_R|LDR_X;
			} else if (strcmp(buf2, "data") == 0) {
				dum->regions[regno].prot = LDR_R|LDR_W|LDR_X;
			} else if (strcmp(buf2, "bss") == 0) {
				dum->regions[regno].prot = LDR_R|LDR_W|LDR_X;
			} else {
				fprintf(stderr, "dummy: unknown type %s\n", buf2);
				goto cleanup;
			}
			dum->regions[regno].pic= 1;
			dum->nreg = ++regno;
		} else if (sscanf(buf, "dependency %s\n", buf2) == 1) {
			if (depno >= NDEPS) {
				fprintf(stderr, "dummy: too many dependencies\n"); 
				goto cleanup;
			}
			dum->deps[depno] = ldr_strdup(buf2);
			dum->ndeps = ++depno;
		} else if (sscanf(buf, "entry %x\n", &vaddr) == 1) {
			dum->entry = (ldr_entry_pt_t)vaddr;
		} else if (sscanf(buf, "import %s %s %s\n", buf4, buf2, buf3) == 3) {
			if (impno >= NIMP) {
				fprintf(stderr, "dummy: too many imports\n");
				goto cleanup;
			}
			if (strcmp(buf3, "text") == 0)
				dum->imports[impno].kind = ldr_sym_function;
			else if (strcmp(buf3, "data") == 0)
				dum->imports[impno].kind = ldr_sym_data;
			else {
				fprintf(stderr, "dummy: unknown import symbol kind\n");
				goto cleanup;
			}
			dum->imports[impno].name = ldr_strdup(buf2);
			dum->imports[impno].pkgno = import_pkgno(dum, buf4);
			dum->nimps = ++impno;
		} else if (sscanf(buf, "export_abs %s %s %x %s\n", buf4, buf2,
				  &val, buf3) == 4) {
			if (expno >= NEXP) {
				fprintf(stderr, "dummy: too many exports\n");
				goto cleanup;
			}

			ldr_symval_make_abs(&dum->exports[expno].value,
					    (univ_t)val);
			if (strcmp(buf3, "text") == 0)
				ldr_symval_make_function(&dum->exports[expno].value);
			else if (strcmp(buf3, "data") == 0)
				ldr_symval_make_data(&dum->exports[expno].value);
			else {
				fprintf(stderr, "dummy: unknown export symbol kind\n");
				goto cleanup;
			}
			dum->exports[expno].name = ldr_strdup(buf2);
			dum->exports[expno].pkgno = export_pkgno(dum, buf4);
			dum->nexps = ++expno;
		} else if (sscanf(buf, "export_rel %s %s %d %x %s\n", buf4, buf2,
				  &expreg, &val, buf3) == 5) {
			if (expno >= NEXP) {
				fprintf(stderr, "dummy: too many exports\n");
				goto cleanup;
			}
			if (expreg >= dum->nreg) {
				fprintf(stderr, "dummy: bad export region ref\n");
				goto cleanup;
			}

			ldr_symval_make_regrel(&dum->exports[expno].value, expreg,
					       val);
			if (strcmp(buf3, "text") == 0)
				ldr_symval_make_function(&dum->exports[expno].value);
			else if (strcmp(buf3, "data") == 0)
				ldr_symval_make_data(&dum->exports[expno].value);
			else {
				fprintf(stderr, "dummy: unknown export symbol kind\n");
				goto cleanup;
			}
			dum->exports[expno].name = ldr_strdup(buf2);
			dum->exports[expno].pkgno = export_pkgno(dum, buf4);
			dum->nexps = ++expno;
		} else if (sscanf(buf, "reloc_region %d %d %d %d\n", &rel_regno, &offset,
				  &from_regno, &from_offset) == 4) {
			if (relocno >= NRELOC) {
				fprintf(stderr, "dummy: too many relocations\n");
				goto cleanup;
			}
			dum->relocs[relocno].kind = region;
			dum->relocs[relocno].regno = rel_regno;
			dum->relocs[relocno].offset = offset;
			dum->relocs[relocno].from = from_regno;
			dum->relocs[relocno].from_offset = from_offset;
			dum->nreloc = ++relocno;
		} else if (sscanf(buf, "reloc_import %d %d %d %d\n", &rel_regno, &offset,
				  &from_impno, &from_offset) == 4) {
			if (relocno >= NRELOC) {
				fprintf(stderr, "dummy: too many relocations\n");
				goto cleanup;
			}
			dum->relocs[relocno].kind = import;
			dum->relocs[relocno].regno = rel_regno;
			dum->relocs[relocno].offset = offset;
			dum->relocs[relocno].from = from_impno;
			dum->relocs[relocno].from_offset = from_offset;
			dum->nreloc = ++relocno;
		} else {
			fprintf(stderr, "dummy: unrecognized line %s\n", buf);
			goto cleanup;
		}

	}
	*dump = dum;
	return(LDR_SUCCESS);

cleanup:
	ldr_free(dum);
	return(LDR_ENOEXEC);
}


static int
import_pkgno(dummy *dum, char *name)

/* Lookup package name in dummy import package table; if found return pkgno.  Else
 * add it and return new pkg no.
 */
{
	int	pkgno;

	for (pkgno = 0; pkgno < dum->nipkg; pkgno++) {
		if (strcmp(dum->ipkg[pkgno], name) == 0)
			return(pkgno);
	}

	dum->ipkg[pkgno] = ldr_strdup(name);
	dum->nipkg = pkgno + 1;
	return(pkgno);
}


static int
export_pkgno(dummy *dum, char *name)

/* Lookup package name in dummy export package table; if found return pkgno.  Else
 * add it and return new pkg no.
 */
{
	int	pkgno;

	for (pkgno = 0; pkgno < dum->nepkg; pkgno++) {
		if (strcmp(dum->epkg[pkgno], name) == 0)
			return(pkgno);
	}

	dum->epkg[pkgno] = ldr_strdup(name);
	dum->nepkg = pkgno + 1;
	return(pkgno);
}


static int
dummy_get_static_dep(ldr_module_handle handle, int depno, char **dep)
{
	dummy		*dum = (dummy *)handle;

	if (depno >= dum->ndeps) {
		fprintf(stderr, "dummy: no more dependencies\n");
		return(LDR_EAGAIN);
	}

	fprintf(stderr, "dummy: returning dependency %s\n", dum->deps[depno]);
	*dep = ldr_strdup(dum->deps[depno]);
	if (*dep != NULL)
		return(LDR_SUCCESS);
	else {
		fprintf(stderr, "dummy: ldr_strdup error in get_static_dep\n");
		return(LDR_EALLOC);
	}
}


static int
dummy_get_imports(ldr_module_handle handle, int *pkg_count,
		  ldr_package_rec **pkgs, int *sym_count,
		  ldr_symbol_rec **imports)
{
	dummy		*dum = (dummy *)handle;
	ldr_package_rec	*pkg_list;
	ldr_symbol_rec	*imp_list;
	int		pkgno;
	int		symno;
	int 		rc;

	if ((rc = ldr_packages_create(dum->nipkg, LDR_PACKAGE_VERSION,
				      &pkg_list)) != LDR_SUCCESS) {
		fprintf(stderr, "dummy: error creating import pkg list %d\n", rc);
		return(rc);
	}
	if ((rc = ldr_symbols_create(dum->nimps, LDR_SYMBOL_VERSION,
				     &imp_list)) != LDR_SUCCESS) {
		fprintf(stderr, "dummy: error creating import list %d\n", rc);
		(void)ldr_packages_free(dum->nipkg, pkg_list);
		return(rc);
	}

	for (pkgno = 0; pkgno < dum->nipkg; pkgno++) {

		/* Special case here: if "package name" is really a pathname (ie
		 * begins with '/'), treat it as a "module-style" package, not
		 * a "package-style" package.  This simulates behavior of
		 * managers like ELF, that want to control the exact module
		 * to which each symbol is resolved; it bypasses the entire
		 * package table lookup scheme.
		 */

		if (*(dum->ipkg[pkgno]) == '/')
			pkg_list[pkgno].lp_kind = ldr_package_module;
		else
			pkg_list[pkgno].lp_kind = ldr_package;
		pkg_list[pkgno].lp_name = ldr_strdup(dum->ipkg[pkgno]);
	}

	for (symno = 0; symno < dum->nimps; symno++) {
		imp_list[symno].ls_name = ldr_strdup(dum->imports[symno].name);
		imp_list[symno].ls_packageno = dum->imports[symno].pkgno;
		/* change to use mach-dep symval contructor */
		imp_list[symno].ls_value.ls_tag = ldr_sym_unres;
	}

	fprintf(stderr, "dummy: returning %d import pkgs, %d import syms\n",
		dum->nipkg, dum->nimps);

	*pkg_count = dum->nipkg;
	*pkgs = pkg_list;
	*sym_count = dum->nimps;
	*imports = imp_list;
	return(LDR_SUCCESS);
}


static int
dummy_map_regions(ldr_module_handle handle, ldr_region_allocs *allocsp,
		  int *reg_count, ldr_region_rec **regions)
{
	dummy		*dum = (dummy *)handle;
	ldr_region_rec	*reg_list;
	int		regno;
	int		rc;
	
	/* Allocate the region list */

	if ((rc = ldr_regions_create(dum->nreg, LDR_REGION_VERSION, &reg_list)) != LDR_SUCCESS) {

		fprintf(stderr, "dummy: region create error %d\n", rc);
		return(rc);
	}

	/* Now map the regions */

	for (regno = 0; regno < dum->nreg; regno++) {

		if ((rc = int_map_region(dum, allocsp, regno, &reg_list[regno])) != LDR_SUCCESS) {

			(void)int_free_regions(dum, regno - 1, reg_list, allocsp);
			return(rc);
		}
	}

	*reg_count = dum->nreg;
	*regions = reg_list;

	return(LDR_SUCCESS);
}


static int
int_map_region(dummy *dum, ldr_region_allocs *allocsp, int regno, 
	       ldr_region_rec *region)
{
	univ_t		baseaddr;
	univ_t		realaddr;
	univ_t		virtaddr;
	int		mapflags;
	int		rc;

	mapflags = LDR_MAP_ANON|LDR_MAP_PRIVATE;

	if (!(dum->regions[regno].pic)) {

		/* absolute region, call abs region allocator */

		virtaddr = dum->regions[regno].vaddr;
		rc = (*(allocsp->lra_abs_alloc))(virtaddr,
						 dum->regions[regno].size,
						 dum->regions[regno].prot,
						 &baseaddr);

		if (virtaddr == baseaddr)
			mapflags |= LDR_MAP_FIXED;

	} else {

		/* relocatable region, call rel region allocator */

		rc = (*(allocsp->lra_rel_alloc))(dum->regions[regno].size,
						 dum->regions[regno].prot,
						 &virtaddr, &baseaddr);
	}
	if (rc != LDR_SUCCESS) {
		fprintf(stderr, "dummy: error %d from region allocator, region %d\n",
			rc, regno);
		return(rc);
	}

	if ((rc = ldr_mmap(baseaddr, dum->regions[regno].size,
			   dum->regions[regno].prot, mapflags, -1, 0,
			   &realaddr)) != LDR_SUCCESS) {
		fprintf(stderr, "dummy: mmap error %d on region %d\n", rc, regno);
		return(rc);
	}

	/* Fill in region record fields for return */

	region->lr_name = ldr_strdup(dum->regions[regno].name);
	region->lr_vaddr = (virtaddr != NULL ? virtaddr : realaddr);
	region->lr_mapaddr = realaddr;
	region->lr_size = dum->regions[regno].size;
	region->lr_prot = dum->regions[regno].prot;

	fprintf(stderr, "dummy: map %s region %s vaddr %x mapaddr %x size %d prot %s\n",
		(mapflags & LDR_MAP_FIXED ? "fixed" : "pic"), region->lr_name,
		region->lr_vaddr, region->lr_mapaddr, region->lr_size,
		prots[region->lr_prot]);

	return(LDR_SUCCESS);
}


static int
dummy_get_export_pkgs(ldr_module_handle handle, int *count,
		      ldr_package_rec **packages)
{
	dummy		*dum = (dummy *)handle;
	ldr_package_rec	*pkg_list;
	int		pkgno;
	int		rc;

	if ((rc = ldr_packages_create(dum->nepkg, LDR_PACKAGE_VERSION,
				      &pkg_list)) != LDR_SUCCESS) {
		fprintf(stderr, "dummy: error creating export pkg list %d\n", rc);
		return(rc);
	}

	for (pkgno = 0; pkgno < dum->nepkg; pkgno++) {
		pkg_list[pkgno].lp_kind = ldr_package;
		pkg_list[pkgno].lp_name = ldr_strdup(dum->epkg[pkgno]);
	}

	fprintf(stderr, "dummy: returning %d export pkgs\n", dum->nepkg);

	*count = dum->nepkg;
	*packages = pkg_list;
	return(LDR_SUCCESS);
}


static int
dummy_get_exports(ldr_module_handle handle, int *sym_count,
		  ldr_symbol_rec **exports)
{
	dummy		*dum = (dummy *)handle;
	ldr_symbol_rec	*exp_list;
	int		symno;
	int 		rc;

	if ((rc = ldr_symbols_create(dum->nexps, LDR_SYMBOL_VERSION,
				     &exp_list)) != LDR_SUCCESS) {
		fprintf(stderr, "dummy: error creating export list %d\n", rc);
		return(rc);
	}

	for (symno = 0; symno < dum->nexps; symno++) {
		exp_list[symno].ls_name = ldr_strdup(dum->exports[symno].name);
		exp_list[symno].ls_packageno = dum->exports[symno].pkgno;
		exp_list[symno].ls_value = dum->exports[symno].value;
	}

	fprintf(stderr, "dummy: returning %d export syms\n",
		dum->nexps);

	*sym_count = dum->nexps;
	*exports = exp_list;
	return(LDR_SUCCESS);
}


static int
dummy_lookup_export(ldr_module_handle handle, ldr_package_rec *package,
		    ldr_symbol_rec *symbol)
{
	dummy		*dum = (dummy *)handle;
	int		i;

	for (i = 0; i < dum->nexps; i++) {
		if ((strcmp(dum->exports[i].name, symbol->ls_name) == 0) &&
		    (strcmp(dum->epkg[dum->exports[i].pkgno], package->lp_name) == 0)) {
			symbol->ls_value = dum->exports[i].value;

			fprintf(stderr, "dummy: lookup of pkg %s sym %s succeeded\n",
				package->lp_name, symbol->ls_name);
			return(LDR_SUCCESS);
		}
	}
	fprintf(stderr, "dummy: lookup of pkg %s sym %s failed\n",
		package->lp_name, symbol->ls_name);
	return(LDR_ENOSYM);
}


int dummy_relocate(ldr_module_handle handle, int nregions,
		   ldr_region_rec regions[], int npackages,
		   ldr_package_rec import_pkgs[], int nimports,
		   ldr_symbol_rec imports[])
{
	dummy		*dum = (dummy *)handle;
	int	i;
	int	regno, offset;
	int	from_regno, from_impno, from_offset;
	unsigned long *ptr;
	unsigned long val;

	fprintf(stderr, "dummy: relocate called\n");
	for (i = 0; i < nimports; i++) {

		if (!ldr_symval_is_abs(&imports[i].ls_value)) {
			fprintf(stderr, "dummy: import %s not abs!\n",
				imports[i].ls_name);
			return(LDR_ERANGE);
		}
		printf("import %s$%s: %x\n", import_pkgs[imports[i].ls_packageno].lp_name,
		       imports[i].ls_name, ldr_symval_abs(&imports[i].ls_value));
	}


	/* Do the relocations */

	for (i = 0; i < dum->nreloc; i++) {

		regno = dum->relocs[i].regno;
		offset = dum->relocs[i].offset;
		if (regno > nregions || offset >= regions[regno].lr_size) {
			fprintf(stderr, "dummy: bad reloc region %d off %x\n",
				regno, offset);
			return(LDR_ENOEXEC);
		}
		ptr = (unsigned long *)((char *)(regions[regno].lr_mapaddr) +
					offset);

		switch (dum->relocs[i].kind) {

		      case region:

			from_regno = dum->relocs[i].from;
			from_offset = dum->relocs[i].from_offset;
			if (from_regno >= nregions) {
				fprintf(stderr, "dummy: bad from reloc region %d\n",
					from_regno);
				return(LDR_ENOEXEC);
			}
			val = (unsigned long)((char *)(regions[from_regno].lr_vaddr) +
				      from_offset);
			break;

		      case import:
			from_impno = dum->relocs[i].from;
			from_offset = dum->relocs[i].from_offset;
			if (from_impno >= nimports) {
				fprintf(stderr, "dummy: bad from reloc impno %d\n",
					from_impno);
				return(LDR_ENOEXEC);
			}
			val = (unsigned long)((char *)(ldr_symval_abs(&imports[from_impno].ls_value)) +
				      from_offset);
			break;
		}

		fprintf(stderr, "dummy: reloc addr %x val %x\n", ptr, val);

		*ptr = val;
	}

	return(LDR_SUCCESS);
}


static int
dummy_get_entry_pt(ldr_module_handle handle, ldr_entry_pt_t *entry_pt)
{
	dummy		*dum = (dummy *)handle;

	fprintf(stderr, "dummy: returning entry pt %x\n", dum->entry);
	*entry_pt = dum->entry;
	return(LDR_SUCCESS);
}


static int
dummy_run_inits(ldr_module_handle handle, entry_pt_kind kind)
{
	fprintf(stderr, "dummy: running inits kind %d\n", kind);
	return(LDR_SUCCESS);
}


static int
dummy_cleanup(ldr_module_handle handle)
{
	dummy		*dum = (dummy *)handle;

	fprintf(stderr, "dummy: cleaning up from load\n");
	return(LDR_SUCCESS);
}


static int
int_unload_region(dummy *dum, int regno, ldr_region_rec *region,
		  ldr_region_allocs *allocsp)
{
	int		rc;

	if (regno >= dum->nreg) {
		fprintf(stderr, "dummy: bad region number in unload%d\n", regno);
		return(LDR_EINVAL);
	}

	if ((rc = ldr_munmap(region->lr_mapaddr, region->lr_size)) !=
	    LDR_SUCCESS) {
		fprintf(stderr, "dummy: unmap error %d region %d\n", rc, regno);
		return(rc);
	}

	fprintf(stderr, "dummy: unmapping region %s at %x size %d\n",
		region->lr_name, region->lr_mapaddr, region->lr_size);

	if ((rc = (*(allocsp->lra_dealloc))(region->lr_vaddr, region->lr_mapaddr,
					    region->lr_size)) != LDR_SUCCESS) {
		fprintf(stderr, "dummy: dealloc proc failed %d\n", rc);
		return(rc);
	}

	/* Free per-region info in dummy handle */

	if (dum->regions[regno].name != NULL) {
		ldr_free(dum->regions[regno].name);
		dum->regions[regno].name = NULL;
	}
	dum->regions[regno].vaddr = 0;
	dum->regions[regno].mapaddr = 0;
	dum->regions[regno].size = 0;
	return(LDR_SUCCESS);
}


static int
int_free_regions(dummy *dum, int nregions, ldr_region_rec *regions,
		 ldr_region_allocs *allocsp)

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
		rrc = int_unload_region(dum, regno, &regions[regno], allocsp);
		if (rc == LDR_SUCCESS) rc = rrc;
		if (regions[regno].lr_name != NULL)
			(void)ldr_free(regions[regno].lr_name);
	}

	rrc = ldr_regions_free(nregions, regions);
	if (rc == LDR_SUCCESS) rc = rrc;
	return(rc);
}


static int
dummy_unload(ldr_module_handle handle, ldr_region_allocs *allocsp,
	     int reg_count, ldr_region_rec *regions,
	     int ipkg_count, ldr_package_rec *import_pkgs,
	     int import_count, ldr_symbol_rec *imports,
	     int epkg_count, ldr_package_rec *export_pkgs)
{
	dummy		*dum = (dummy *)handle;
	int		rc, rrc;
	int		i;

	fprintf(stderr, "dummy: unloading\n");

	/* Unmap all regions */

	rrc = int_free_regions(dum, reg_count, regions, allocsp);
	if (rc == LDR_SUCCESS) rc = rrc;

	/* deallocate module's import and export packages and lists */
	
	rrc = ldr_packages_free(ipkg_count, import_pkgs);
	if (rc == LDR_SUCCESS) rc = rrc;
	rrc = ldr_symbols_free(import_count, imports);
	if (rc == LDR_SUCCESS) rc = rrc;
	rrc = ldr_packages_free(epkg_count, export_pkgs);
	if (rc == LDR_SUCCESS) rc = rrc;

	for (i = 0; i < dum->nipkg; i++)
		ldr_free(dum->ipkg[i]);
	for (i = 0; i < dum->nimps; i++)
		ldr_free(dum->imports[i].name);
	for (i = 0; i < dum->nepkg; i++)
		ldr_free(dum->epkg[i]);
	for (i = 0; i < dum->nexps; i++)
		ldr_free(dum->exports[i].name);
	ldr_free(dum);
	return(LDR_SUCCESS);
}
