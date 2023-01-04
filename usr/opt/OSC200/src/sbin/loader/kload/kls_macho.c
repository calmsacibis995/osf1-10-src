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
static char	*sccsid = "@(#)$RCSfile: kls_macho.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <string.h>
#include <loader.h>
#include <unistd.h>
#include <mach_o_header.h>
#include <mach_o_format.h>

#include "ldr_types.h"
#include "ldr_lock.h"
#include "ldr_hash.h"
#include "chain_hash.h"
#include "open_hash.h"
#include "dqueue.h"
#include "ldr_errno.h"
#include "ldr_malloc.h"

#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_known_pkg.h"
#include "ldr_module.h"
#include "ldr_switch.h"
#include "ldr_sys_int.h"

#define	malloc(nbytes, ptr) \
	ldr_malloc(((size_t)(nbytes)), LDR_MACHO_T, ((univ_t)(ptr)))

#define	free(ptr) \
	ldr_free(((univ_t)(ptr)))

#define	read(fhandle, buf, nbytes) \
	ldr_read((fhandle), ((char *)(buf)), ((unsigned)(nbytes)))

#define	lseek(fhandle, offset, whence) \
	ldr_lseek((fhandle), ((off_t)(offset)), (whence))

			   
extern  char *kls_default_package_name;
typedef	void *object_t;

			    
/************************* List Abstraction *************************/
typedef	object_t list_t;
typedef	void* list_iterator_id_t;
#define	LIST_ITERATOR_MARK	((list_iterator_id_t)0)
static int                list_create(list_t *list_ptr);
static int                list_append(list_t *list, void *data);
static list_iterator_id_t list_next(list_t list, list_iterator_id_t id,
	void **datap);
static int                list_destroy(list_t list);

/****************** OSF/Mach-O Header Abstraction ******************/
typedef object_t header_t;
static int header_create(ldr_file_t fd, header_t *header_ptr);
static int header_get_header(header_t header, mo_header_t **mohp);
static int header_destroy(header_t header);

/********************* Load Commands Abstraction *********************/
typedef object_t commands_t;
#define	COMMANDS_ITERATOR_MARK	((mo_lcid_t)MO_INVALID_LCID)
static int       commands_create(ldr_file_t fd, header_t header,
				 commands_t *commands_ptr);
static mo_lcid_t commands_next(commands_t commands, mo_lcid_t lcid);
static int       commands_get(commands_t commands, mo_lcid_t lcid,
			      void **cmdp);
static int       commands_destroy(commands_t commands);


/******************* String Sections Abstraction *******************/
typedef object_t strings_t;
static int strings_create(ldr_file_t fd, commands_t commands,
			  strings_t *strings_ptr);
static int strings_get(strings_t strings, mo_lcid_t lcid,
		       mo_offset_t sctoff, char **cpp);
static int strings_destroy(strings_t strings);


/*********************** Regions Abstraction ***********************/
typedef object_t regions_t;
static int regions_create(commands_t commands, strings_t strings,
			  regions_t *regions_ptr);
static int regions_get(regions_t regions, int *countp,
		       ldr_region_rec **regionsp);
static int regions_number(regions_t regions, mo_lcid_t lcid,
			  int *number_pointer);
static int regions_destroy(regions_t regions);

		      
/*********************** Packages Abstraction ***********************/
typedef object_t packages_t;
static int packages_create(ldr_file_t fd, commands_t commands,
			   strings_t strings, packages_t *packages_ptr);
static int packages_get(packages_t packages, mo_lcid_t lcid,
			mo_short_t idx, char **namep);
static int packages_records(packages_t packages, int *countp,
			    ldr_package_rec **packagesp);
static int packages_destroy(packages_t packages);


/*********************** Exports Abstraction ***********************/
typedef object_t exports_t;
static int exports_create(ldr_file_t fd, commands_t commands,
			  strings_t strings, regions_t regions,
			  packages_t packages, exports_t *exports_ptr);
static int exports_lookup(exports_t exports, char *package_name,
			  char *symbol_name, ldr_symval **sym);
static int exports_destroy(exports_t exports);


/********************* Entry Point Abstraction *********************/
typedef object_t entry_t;
static int entry_create(commands_t commands, entry_t *entry_ptr);
static int entry_get(entry_t entry, ldr_entry_pt_t *entryp);
static int entry_destroy(entry_t entry);

			 
/************************ Handle Abstraction ************************/
typedef struct handle_object {
	header_t   ho_header;
	commands_t ho_commands;
	strings_t  ho_strings;
	packages_t ho_packages;
	exports_t  ho_exports;
	regions_t  ho_regions;
	entry_t    ho_entry;
} handle_object_t;

int
kls_macho_recog(const char *filename, ldr_file_t fd,
	ldr_module_handle *handle_ptr)
{
	handle_object_t  *object;
	int                rc;

	/* allocate a handle object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);

	/* bootstrap all abstractions */
	if ((rc = header_create(fd, &object->ho_header)) != LDR_SUCCESS)
		return(rc);
	if ((rc = commands_create(fd, object->ho_header,
	    &object->ho_commands)) != LDR_SUCCESS)
		goto error_cleanup_header;
	if ((rc = strings_create(fd, object->ho_commands,
	    &object->ho_strings)) != LDR_SUCCESS)
		goto error_cleanup_commands;
	if ((rc = regions_create(object->ho_commands, object->ho_strings,
	    &object->ho_regions)) != LDR_SUCCESS)
		goto error_cleanup_strings;
	if ((rc = packages_create(fd, object->ho_commands,
	    object->ho_strings, &object->ho_packages)) != LDR_SUCCESS)
		goto error_cleanup_regions;
	if ((rc = exports_create(fd, object->ho_commands, object->ho_strings,
	    object->ho_regions, object->ho_packages, &object->ho_exports)) != LDR_SUCCESS)
		goto error_cleanup_packages;
	if ((rc = entry_create(object->ho_commands, &object->ho_entry)) != LDR_SUCCESS)
		goto error_cleanup_exports;

	(void)ldr_close(fd);
	*handle_ptr = (ldr_module_handle)object;
	return(LDR_SUCCESS);


error_cleanup_exports:
	(void)exports_destroy(object->ho_exports);
error_cleanup_packages:
	(void)packages_destroy(object->ho_packages);
error_cleanup_regions:
	(void)regions_destroy(object->ho_regions);
error_cleanup_strings:
	(void)strings_destroy(object->ho_strings);
error_cleanup_commands:
	(void)commands_destroy(object->ho_commands);
error_cleanup_header:
	(void)header_destroy(object->ho_header);
	return(rc);
}

int
kls_macho_map_regions(ldr_module_handle handle, ldr_region_allocs *allocsp,
	int *reg_count, ldr_region_rec **regions)
{
	handle_object_t *object = (handle_object_t *)handle;

	return(regions_get(object->ho_regions, reg_count, regions));
}

int
kls_macho_lookup_export(ldr_module_handle handle, ldr_package_rec *package,
	ldr_symbol_rec *symbol)
{
	handle_object_t *object = (handle_object_t *)handle;
	ldr_symval      *symp;
	int              rc;

	if ((rc = exports_lookup(object->ho_exports, package->lp_name,
	    symbol->ls_name, &symp)) != LDR_SUCCESS)
		return(rc);

	symbol->ls_value = *symp;
	return(LDR_SUCCESS);
}

int
kls_macho_get_entry_pt(ldr_module_handle handle, ldr_entry_pt_t *entry_pt)
{
	handle_object_t *object = (handle_object_t *)handle;

	return(entry_get(object->ho_entry, entry_pt));
}

int
kls_macho_get_export_pkgs(ldr_module_handle handle, int *countp,
	ldr_package_rec **packagesp)
{
	handle_object_t *object = (handle_object_t *)handle;

	return(packages_records(object->ho_packages, countp, packagesp));
}

int
kls_macho_get_exports(ldr_module_handle handle, int *countp,
	ldr_symbol_rec **symbolsp)
{
	/* I haven't a clue how this thing works, so I'm not
	 * going to implement it.  It should never be used anyway.
	 */

	return(LDR_EINVAL);
}


/********************* Entry Point Abstraction *********************/
typedef struct entry_object {
	ldr_entry_pt_t eo_entry;
} entry_object_t;

static int
entry_create(commands_t commands, entry_t *entry_ptr)
{
	entry_object_t  *object;
	entry_command_t *ecp;
	mo_lcid_t        lcid;
	int              rc;

	/* allocate an entry object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);

	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid))
	    != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&ecp)) != LDR_SUCCESS) {
			(void)free(object);
			return(rc);
		}
		if ((ecp->ldc_cmd_type != LDC_ENTRY) ||
		    (!(ecp->entc_flags & ENT_VALID_ABSADDR_F)))
			continue;

		object->eo_entry = (ldr_entry_pt_t)ecp->entc_absaddr;
		return(LDR_SUCCESS);
	}
	(void)free(object);
	return(LDR_ENOMAIN);
}


static int
entry_get(entry_t entry, ldr_entry_pt_t *entryp)
{
	entry_object_t *object = (entry_object_t *)entry;

	*entryp = object->eo_entry;
	return(LDR_SUCCESS);
}

static int
entry_destroy(entry_t entry)
{
	entry_object_t *object = (entry_object_t *)entry;

	(void)free(object);
	return(LDR_SUCCESS);
}
			
			
/*********************** Regions Abstraction ***********************/
typedef struct regions_object {
	int             ro_count;
	ldr_region_rec *ro_regions;
	mo_lcid_t      *ro_lcids;
} regions_object_t;

static int
regions_create(commands_t commands, strings_t strings, regions_t *regions_ptr)
{
	regions_object_t *object;
	region_command_t *rcp;
	ldr_region_rec   *lrp;
	mo_lcid_t        *rlp;
	mo_lcid_t         lcid;
	int               count, size, rc;

	/* allocate a regions object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);
	object->ro_count = 0;

	/* count the regions */
	count = 0;
	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid))
	    != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&rcp)) != LDR_SUCCESS)
			goto error_cleanup_object;
		if (rcp->ldc_cmd_type == LDC_REGION)
			count++;
	}

	if (!count) {
		*regions_ptr = (regions_t)object;
		return(LDR_SUCCESS);
	}

	/* allocate region records */
	if ((rc = ldr_regions_create(count, LDR_REGION_VERSION,
				     &object->ro_regions)) != LDR_SUCCESS)
		goto error_cleanup_object;
	size = sizeof(*rlp) * count;
	if ((rc = malloc(size, &object->ro_lcids)) < 0)
		goto error_cleanup_regions;

	object->ro_count = count;

	lrp = object->ro_regions;
	rlp = object->ro_lcids;

	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid))
	    != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&rcp)) != LDR_SUCCESS)
			goto error_cleanup_regions;
		if (rcp->ldc_cmd_type != LDC_REGION)
			continue;

		lrp->lr_version = LDR_REGION_VERSION;
		if ((rc = strings_get(strings, rcp->regc_region_name.adr_lcid,
		    rcp->regc_region_name.adr_sctoff, &lrp->lr_name)) != LDR_SUCCESS)
			return(rc);

		lrp->lr_prot = (ldr_prot_t)0;
		if (rcp->regc_initprot & MO_PROT_READ)
			lrp->lr_prot |= LDR_R;
		if (rcp->regc_initprot & MO_PROT_WRITE)
			lrp->lr_prot |= LDR_W;
		if (rcp->regc_initprot & MO_PROT_EXECUTE)
			lrp->lr_prot |= LDR_X;

		lrp->lr_vaddr = (univ_t)rcp->regc_vm_addr;
		lrp->lr_mapaddr = (univ_t)-1;
		lrp->lr_size = (size_t)rcp->regc_vm_size;
		lrp->lr_flags = LRF_LOADED;

		*rlp++ = lcid;

		lrp++;
	}

	*regions_ptr = (regions_t)object;
	return(LDR_SUCCESS);


error_cleanup_regions:
	(void)ldr_regions_free(object->ro_count, object->ro_regions);

error_cleanup_object:
	(void)free(object);
	return(rc);
}

static int
regions_get(regions_t regions, int *countp, ldr_region_rec **regionsp)
{
	regions_object_t *object = (regions_object_t *)regions;

	if (*countp = object->ro_count)
		*regionsp = object->ro_regions;
	return(LDR_SUCCESS);
}

static int
regions_number(regions_t regions, mo_lcid_t lcid, int *number_pointer)
{
	regions_object_t *object = (regions_object_t *)regions;
	int               number;

	for (number = 0; number < object->ro_count; number++) {
		if (object->ro_lcids[number] == lcid) {
			*number_pointer = number;
			return(LDR_SUCCESS);
		}
	}
	return(LDR_ENOEXEC);
}

static int
regions_destroy(regions_t regions)
{
	regions_object_t *object = (regions_object_t *)regions;

	if (object->ro_count) {
		(void)ldr_regions_free(object->ro_count, object->ro_regions);
		(void)free(object->ro_lcids);
	}
	(void)free(object);
	return(LDR_SUCCESS);
}



/*********************** Exports Abstraction ***********************/
typedef struct symbol_val {
	struct symbol_val *sv_next;
	ldr_symval         sv_ldr_symval;
	char              *sv_pkg_name;
} symbol_val_t;

typedef struct exports_object {
	int            eo_count;
	symbol_info_t *eo_sym_infos;
	symbol_val_t  *eo_sym_vals;
	open_hashtab_t eo_table;
} exports_object_t;

static int
exports_create(ldr_file_t fd, commands_t commands, strings_t strings,
	regions_t regions, packages_t packages, exports_t *exports_ptr)
{
	exports_object_t  *object;
	mo_lcid_t          lcid;
	symbols_command_t *scp;
	symbol_info_t     *symbol_info, *sip;
	symbol_val_t      *svp, *prev, *p;
	int                number;
	int                i, rc, count, size;
	char              *symbol_name;

	/* allocate an exports object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);

	object->eo_sym_infos = (symbol_info_t *)0;
	object->eo_sym_vals = (symbol_val_t *)0;
	object->eo_table = (open_hashtab_t)0;

	/* count exported symbols */
	count = 0;
	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid))
	    != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&scp)) != LDR_SUCCESS)
			goto error;
		if ((scp->ldc_cmd_type != LDC_SYMBOLS)
		    || (scp->symc_kind != SYMC_DEFINED_SYMBOLS))
			continue;
#ifdef	EXPORTS_ONLY
		/* only look at symbols marked as exported */
		count += scp->symc_n_exported_symb;
#else
		/* look at all symbols */
		count += scp->symc_nentries;
#endif
	}

	object->eo_count = count;
	if (!object->eo_count) {
		*exports_ptr = (exports_t)object;
		return(LDR_SUCCESS);
	}

	/* allocate space for everything */
	size = object->eo_count * sizeof(*object->eo_sym_infos);
	if ((rc = malloc(size, &object->eo_sym_infos)) < 0)
		goto error;
	size = object->eo_count * sizeof(*object->eo_sym_vals);
	if ((rc = malloc(size, &object->eo_sym_vals)) < 0)
		goto error;
	if ((rc = open_hash_create(object->eo_count, (ldr_hash_p)hash_string,
	    (ldr_hash_compare_p)strcmp, (open_hash_flags_t)0,
	    &object->eo_table)) != LDR_SUCCESS)
		goto error;

	/* build export list */
	symbol_info = object->eo_sym_infos;
	svp = object->eo_sym_vals;
	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid)) != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&scp)) != LDR_SUCCESS)
			goto error;
		if ((scp->ldc_cmd_type != LDC_SYMBOLS)
		    || (scp->symc_kind != SYMC_DEFINED_SYMBOLS))
			continue;

		/* read all symbol_info_t structures */
		if ((rc = lseek(fd, scp->ldc_section_off, SEEK_SET)) < 0)
			goto error;
		if ((rc = read(fd, symbol_info, scp->ldc_section_len)) < 0)
			goto error;

		/* pass over all symbol_info_t structures */
		for (i = 0; i < scp->symc_nentries; i++) {
			sip = &symbol_info[i];

#ifdef	EXPORTS_ONLY
			/* only look at symbols marked as exported */
			if (!(sip->si_flags & SI_EXPORT_F))
				continue;
#endif

			/* get symbol name */
			if ((rc = strings_get(strings, scp->symc_strings_section,
			    sip->si_symbol_name, &symbol_name)) != LDR_SUCCESS)
				goto error;

			/* fill in symbol_val structure */
			svp->sv_next = (symbol_val_t *)0;
			if (sip->si_flags & SI_CODE_F)
				svp->sv_ldr_symval.ls_kind = ldr_sym_function;
			else if (sip->si_flags & SI_DATA_F)
				svp->sv_ldr_symval.ls_kind = ldr_sym_data;
			else
				svp->sv_ldr_symval.ls_kind = ldr_sym_unknown;
			if (sip->si_flags & SI_ABSOLUTE_VALUE_F) {
				svp->sv_ldr_symval.ls_tag = ldr_sym_abs;
				svp->sv_ldr_symval.ls_abs = (univ_t)sip->si_abs_val;
			} else if (sip->si_flags & SI_LITERAL_F) {
				svp->sv_ldr_symval.ls_tag = ldr_sym_abs;
				svp->sv_ldr_symval.ls_abs = (univ_t)sip->si_lit_val;
			} else {
				/* must be a def_val */
				if ((rc = regions_number(regions,
				    sip->si_def_val.adr_lcid, &number)) < 0) {
					/* couldn't find it */
					svp->sv_ldr_symval.ls_tag = ldr_sym_unres;
				} else {
					svp->sv_ldr_symval.ls_tag = ldr_sym_regrel;
					svp->sv_ldr_symval.ls_regno = number;
					svp->sv_ldr_symval.ls_offset = (long)sip->si_def_val.adr_sctoff;
				}
			}

			/* use default package name when index invalid */
			if (sip->si_package_index == MO_INVALID_PKG_INDEX)
				svp->sv_pkg_name = kls_default_package_name;
			else if ((rc = packages_get(packages,
				    scp->symc_pkg_list, sip->si_package_index,
				    &svp->sv_pkg_name)) != LDR_SUCCESS)
					goto error;

			/* insert into hash table */
			prev = svp;
			if ((rc = open_hash_search(object->eo_table,
			    (const univ_t)symbol_name, (univ_t *)&prev,
			    (LDR_HASH_INSERT | LDR_HASH_LOOKUP))) != LDR_SUCCESS)
				goto error;

			/* symbol name already in table, chain to end of list */
			if (prev != svp) {
				for (p = prev; p->sv_next; p = p->sv_next)
					;
				p->sv_next = svp;
			}

			svp++;
		}

		symbol_info += count;
	}

	*exports_ptr = (exports_t)object;
	return(LDR_SUCCESS);

error:
	(void)exports_destroy((exports_t)object);
	return(rc);
}
			
static int
exports_lookup(exports_t exports, char *package_name, char *symbol_name,
	ldr_symval **sym)
{
	exports_object_t *object = (exports_object_t *)exports;
	symbol_val_t   *svp, *p;
	int             rc;

	if (!object->eo_count)
		return(LDR_ENOSYM);

	if ((rc = open_hash_lookup(object->eo_table,
	    (const univ_t)symbol_name, (univ_t *)&svp)) != LDR_SUCCESS)
		return(rc);
	for (p = svp; p; p = p->sv_next)
		if (!strcmp(package_name, p->sv_pkg_name)) {
			*sym = &svp->sv_ldr_symval;
			return(LDR_SUCCESS);
		}
	return(LDR_ENOSYM);
}

static int
exports_destroy(exports_t exports)
{
	exports_object_t *object = (exports_object_t *)exports;

	if (object->eo_sym_infos)
		(void)free(object->eo_sym_infos);
	if (object->eo_sym_vals)
		(void)free(object->eo_sym_vals);
	if (object->eo_table)
		(void)open_hash_destroy(object->eo_table);
	(void)free(object);
	return(LDR_SUCCESS);
}

			
/*********************** Packages Abstraction ***********************/
typedef struct package_entry {
	char *pe_name;
#ifdef	VESION
	mo_addr_t pe_version;
#endif
} package_entry_t;

typedef struct package_array {
	mo_lcid_t        pa_lcid;
	int              pa_nentries;
	package_entry_t *pa_entries;
} package_array_t;

typedef struct packages_object {
	int              po_package_record_count;
	ldr_package_rec *po_package_records;
	list_t           po_package_arrays;
} packages_object_t;


static int
packages_create(ldr_file_t fd, commands_t commands, strings_t strings, packages_t *packages_ptr)
{
	packages_object_t  *object;
	package_array_t    *pap;
	package_command_t *pcp;
	ldr_package_rec   *lpr;
	mo_lcid_t          lcid;
	list_iterator_id_t id;
	char              *name;
	int                nentries, size;
	int		   count;
	int                default_found = 0;
	int                i, rc;

	/* allocate a packages object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);
	if ((rc = list_create(&object->po_package_arrays)) != LDR_SUCCESS) {
		(void)free(object);
		return(rc);
	}
	object->po_package_record_count = 0;

	count = 0;
	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid)) != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&pcp)) != LDR_SUCCESS)
			goto error;
		if ((pcp->ldc_cmd_type != LDC_PACKAGE)
		    || (!(pcp->pkgc_flags & PKG_EXPORT_F)))
			continue;

		/* allocate package array and space for entries */
		if ((rc = malloc(sizeof(*pap), &pap)) < 0)
			goto error;
		nentries = pcp->pkgc_nentries;
		size = nentries * sizeof(*pap->pa_entries);
		if ((rc = malloc(size, &pap->pa_entries)) < 0) {
			(void)free(pap);
			goto error;
		}

		/* append to list */
		if ((rc = list_append(object->po_package_arrays, (void *)pap)) != LDR_SUCCESS) {
			(void)free(pap->pa_entries);
			(void)free(pap);
			goto error;
		}

		/* fill in data */
		pap->pa_lcid = lcid;
		pap->pa_nentries = nentries;

		for (i = 0; i < pcp->pkgc_nentries; i++) {
			/* get package name */
			if ((rc = strings_get(strings, pcp->pkgc_strings_id,
			    pcp->pkgc_pkg_list[i].pe_pkg_name, &name)) != LDR_SUCCESS)
				goto error;

			if (!strcmp(name, kls_default_package_name))
				default_found = 1;

			pap->pa_entries[i].pe_name = name;
#ifdef	VERSION
			pap->pa_entries[i].pe_version = pcp->pkgc_pkg_list[i].pe_version_addr;
#endif	
		}

		/* take care of loader package recs */
		count += nentries;
	}

	/* build loader package records */
	if (!default_found)
		count++;

	if ((rc = ldr_packages_create(count, LDR_PACKAGE_VERSION,
				      &object->po_package_records)) != LDR_SUCCESS)
		goto error;
	object->po_package_record_count = count;

	lpr = object->po_package_records;
	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->po_package_arrays, id, (void **)&pap))
	    != LIST_ITERATOR_MARK) {
		for (i = 0; i < pap->pa_nentries; i++) {
			lpr->lp_version = LDR_PACKAGE_VERSION;
			lpr->lp_kind = ldr_package;
			lpr->lp_name = pap->pa_entries[i].pe_name;
			lpr++;
		}
	}
	
	if (!default_found) {
		lpr->lp_version = LDR_PACKAGE_VERSION;
		lpr->lp_kind = ldr_package;
		lpr->lp_name = kls_default_package_name;
	}

	*packages_ptr = (packages_t)object;
	return(LDR_SUCCESS);

error:
	(void)packages_destroy((packages_t)object);
	return(rc);
}

#ifdef	VERSION
static int
packages_get(packages_t packages, mo_lcid_t lcid, mo_short_t idx, char **namep, mo_addr_t **versionp)
#else
static int
packages_get(packages_t packages, mo_lcid_t lcid, mo_short_t idx, char **namep)
#endif
{
	packages_object_t *object = (packages_object_t *)packages;
	package_array_t   *pap;
	list_iterator_id_t id;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->po_package_arrays, id, (void **)&pap))
	    != LIST_ITERATOR_MARK) {
		if (lcid == pap->pa_lcid) {
			if (idx >= pap->pa_nentries)
				return(LDR_ENOEXEC);
			*namep =  pap->pa_entries[idx].pe_name;
			return(LDR_SUCCESS);
		}
	}
	return(LDR_ENOPKG);
}

static int
packages_records(packages_t packages, int *countp, ldr_package_rec **packagesp)
{
	packages_object_t *object = (packages_object_t *)packages;

	if (*countp = object->po_package_record_count)
		*packagesp = object->po_package_records;

	return(LDR_SUCCESS);
}

static int
packages_destroy(packages_t packages)
{
	packages_object_t *object = (packages_object_t *)packages;
	package_array_t   *pap;
	list_iterator_id_t id;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->po_package_arrays, id, (void **)&pap))
	    != LIST_ITERATOR_MARK) {
		(void)free(pap->pa_nentries);
		(void)free(pap);
	}
	(void)list_destroy(object->po_package_arrays);
	if (object->po_package_record_count)
		(void)ldr_packages_free(object->po_package_record_count,
			object->po_package_records);
	(void)free(object);
	return(LDR_SUCCESS);
}


/******************* String Sections Abstraction *******************/
typedef struct string_section {
	mo_lcid_t              ss_lcid;
	int                    ss_size;
	char                  *ss_strings;
} string_section_t;

typedef struct strings_object {
	list_t so_sections;
} strings_object_t;

static int
strings_create(ldr_file_t fd, commands_t commands, strings_t *strings_ptr)
{
	strings_object_t  *object;
	string_section_t  *ssp;
	strings_command_t *scp;
	mo_lcid_t          lcid;
	int                rc;

	/* allocate a strings object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);
	if ((rc = list_create(&object->so_sections)) != LDR_SUCCESS) {
		(void)free(object);
		return(rc);
	}

	lcid = COMMANDS_ITERATOR_MARK;
	while ((lcid = commands_next(commands, lcid)) != COMMANDS_ITERATOR_MARK) {
		if ((rc = commands_get(commands, lcid, (void **)&scp)) != LDR_SUCCESS)
			goto error;
		if (scp->ldc_cmd_type != LDC_STRINGS)
			continue;

		/* allocate strings section and space for strings */
		if ((rc = malloc(sizeof(*ssp), &ssp)) < 0)
			goto error;
		ssp->ss_size = scp->ldc_section_len;
		if ((rc = malloc(ssp->ss_size, &ssp->ss_strings)) < 0) {
			(void)free(ssp);
			goto error;
		}

		/* append to list */
		if ((rc = list_append(object->so_sections, (void *)ssp)) != LDR_SUCCESS) {
			(void)free(ssp->ss_strings);
			(void)free(ssp);
			goto error;
		}

		/* get the data */
		ssp->ss_lcid = lcid;
		if ((rc = lseek(fd, scp->ldc_section_off, SEEK_SET)) < 0)
			goto error;
		if ((rc = read(fd, ssp->ss_strings, ssp->ss_size)) < 0)
			goto error;
	}

	*strings_ptr = (strings_t)object;
	return(LDR_SUCCESS);

error:
	(void)strings_destroy((strings_t)object);
	return(rc);
}

static int
strings_get(strings_t strings, mo_lcid_t lcid, mo_offset_t sctoff, char **cpp)
{
	strings_object_t  *object = (strings_object_t *)strings;
	string_section_t  *ssp;
	list_iterator_id_t id;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->so_sections, id, (void **)&ssp))
	    != LIST_ITERATOR_MARK) {
		if (lcid == ssp->ss_lcid) {
			if (sctoff >= ssp->ss_size)
				return(LDR_ENOEXEC);
			*cpp =  ssp->ss_strings + sctoff;
			return(LDR_SUCCESS);
		}
	}
	return(LDR_ENOSYM);
}

static int
strings_destroy(strings_t strings)
{
	strings_object_t  *object = (strings_object_t *)strings;
	string_section_t  *ssp;
	list_iterator_id_t id;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->so_sections, id, (void **)&ssp))
	    != LIST_ITERATOR_MARK) {
		(void)free(ssp->ss_strings);
		(void)free(ssp);
	}
	(void)list_destroy(object->so_sections);
	(void)free(object);
	return(LDR_SUCCESS);
}

		      
/********************* Load Commands Abstraction *********************/
typedef struct commands_object {
	char        *co_memory;		/* memory that holds all load	*/
					/*   commands			*/
	int          co_nentries;
	mo_offset_t *co_offset;		/* array of load command file	*/
					/*   offsets that is lcm_map	*/
					/*   entries			*/
	void       **co_command;	/* array of pointers to		*/
					/*   individual load commands	*/
} commands_object_t;

static int offset_to_pointer(mo_header_t *, char *, mo_offset_t, void **);

static int
commands_create(ldr_file_t fd, header_t header, commands_t *commands_ptr)
{
	commands_object_t       *object;
	mo_header_t             *moh;
	load_cmd_map_command_t *lp;	/* memory pointer	*/
	load_cmd_map_command_t *fp;	/* file pointer		*/
	int                     i, rc, size;

	/* allocate a commands object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);

	/* get header */
	if ((rc = header_get_header(header, &moh)) != LDR_SUCCESS)
		goto error_cleanup_object;

	/* read load commands */
	if ((rc = lseek(fd, moh->moh_first_cmd_off, SEEK_SET)) < 0)
		goto error_cleanup_object;
	if ((rc = malloc(moh->moh_sizeofcmds, &object->co_memory)) < 0)
		goto error_cleanup_object;
	if ((rc = ldr_read(fd, object->co_memory, moh->moh_sizeofcmds)) < 0)
		goto error_cleanup_memory;

	/* find load command map load command */
	if ((rc = offset_to_pointer(moh, object->co_memory,
	    moh->moh_load_map_cmd_off, (void **)&lp)) < 0)
		goto error_cleanup_memory;
	if (lp->lcm_nentries <= 0) {
		rc = LDR_ENOEXEC;
		goto error_cleanup_memory;
	}

	/* fp contains offset into file, check if nentries in out of bounds */
	fp = (load_cmd_map_command_t *)moh->moh_first_cmd_off;
	if ((int)&fp->lcm_map[lp->lcm_nentries]
	    > (moh->moh_first_cmd_off + moh->moh_sizeofcmds)) {
		rc = LDR_ENOEXEC;
		goto error_cleanup_memory;
	}

	object->co_offset = lp->lcm_map;
	object->co_nentries = lp->lcm_nentries;

	/* get pointers to load commands */
	size = object->co_nentries * sizeof(*object->co_command);
	if ((rc = malloc(size, &object->co_command)) < 0)
		goto error_cleanup_memory;
	for (i = 0; i < object->co_nentries; i++) {
		if (object->co_offset[i] == 0)
			continue;
		if ((rc = offset_to_pointer(moh, object->co_memory,
		    object->co_offset[i], &object->co_command[i])) < 0)
			goto error_cleanup_pointers;
	}

	*commands_ptr = (commands_t)object;
	return(LDR_SUCCESS);

error_cleanup_pointers:
	(void)free(object->co_command);

error_cleanup_memory:
	(void)free(object->co_memory);

error_cleanup_object:
	(void)free(object);
	return(rc);
}

static mo_lcid_t
commands_next(commands_t commands, mo_lcid_t lcid)
{
	commands_object_t *object = (commands_object_t *)commands;

	if (lcid == MO_INVALID_LCID)
		lcid = 0;
	else if ((lcid < 0) || (lcid >= (object->co_nentries - 1)))
		lcid = MO_INVALID_LCID;
	else
		lcid++;
	return(lcid);
}

static int
commands_get(commands_t commands, mo_lcid_t lcid, void **cmdp)
{
	commands_object_t *object = (commands_object_t *)commands;

	if ((lcid < 0) || (lcid >= (object->co_nentries)))
		return(LDR_ENOEXEC);
	*cmdp = object->co_command[lcid];
	return(LDR_SUCCESS);
}

static int
commands_destroy(commands_t commands)
{
	commands_object_t *object = (commands_object_t *)commands;

	(void)free(object->co_command);
	(void)free(object->co_memory);
	(void)free(object);
	return(LDR_SUCCESS);
}

static int
offset_to_pointer(moh, load_commands, offset, lcmdp)
	mo_header_t *moh;
	char        *load_commands;
	mo_offset_t  offset;
	void       **lcmdp;
{
	char *cp;

	if ((offset < moh->moh_first_cmd_off)
	    || (offset >= (moh->moh_first_cmd_off + moh->moh_sizeofcmds)))
		return(LDR_ENOEXEC);
	cp = load_commands + (offset - moh->moh_first_cmd_off);
	if (((unsigned int)cp) & 3)
		return(LDR_ENOEXEC);
	*lcmdp = (void *)cp;
	return(LDR_SUCCESS);
}


/****************** OSF/Mach-O Header Abstraction ******************/
typedef struct header_object {
	mo_header_t ho_mo_header;
} header_object_t;

static int
header_create(ldr_file_t fd, header_t *header_ptr)
{
	header_object_t *object;
	char             buffer[MO_SIZEOF_RAW_HDR];
	mo_header_t     *moh;
	int              rc;

	/* allocate a header object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);
	
	/* read the OSF/Mach-O header */
	if ((rc = lseek(fd, 0, SEEK_SET)) < 0)
		goto error;
	if ((rc = read(fd, buffer, MO_SIZEOF_RAW_HDR)) < 0)
		goto error;
	if (rc = decode_mach_o_hdr((void *)buffer, MO_SIZEOF_RAW_HDR,
	    MOH_HEADER_VERSION, &object->ho_mo_header)) {
		if (rc > 0)
			rc = LDR_ENOEXEC;
		goto error;
	}
	moh = &object->ho_mo_header;

	/* check magic number */
	if (moh->moh_magic != MOH_MAGIC) {
		rc = LDR_ENOEXEC;
		goto error;
	}

	if ((moh->moh_byte_order != OUR_BYTE_ORDER)
	  || (moh->moh_data_rep_id != OUR_DATA_REP_ID)
	  || (moh->moh_cpu_type != OUR_CPU_TYPE)
/*	  || (moh->moh_max_page_size != PAGE_SIZE)	*/
	  || (moh->moh_sizeofcmds <= 0)
	  || (moh->moh_first_cmd_off <= 0)
	  || (moh->moh_load_map_cmd_off < moh->moh_first_cmd_off)
	  || (moh->moh_load_map_cmd_off
	     >= (moh->moh_first_cmd_off + moh->moh_sizeofcmds))) {
		rc = LDR_ENOEXEC;
		goto error;
	}

	*header_ptr = (header_t)object;
	return(LDR_SUCCESS);

error:
	(void)free(object);
	return(rc);	
}

static int
header_get_header(header_t header, mo_header_t **mohp)
{
	header_object_t *object = (header_object_t *)header;

	*mohp = &object->ho_mo_header;
	return(LDR_SUCCESS);
}

static int
header_destroy(header_t header)
{
	header_object_t *object = (header_object_t *)header;
	int              rc;

	rc = free(object);
	return(rc);
}


/************************* List Abstraction *************************/
typedef struct element {
	struct element *e_next;
	void           *e_data;
} element_t;

typedef struct list_object {
	element_t *lo_head;
	element_t *lo_tail;
} list_object_t;

static int
list_create(list_t *list_ptr)
{
	list_object_t *object;
	int            rc;

	/* allocate a list object */
	if ((rc = malloc(sizeof(*object), &object)) < 0)
		return(rc);

	object->lo_head = (element_t *)0;
	object->lo_tail = (element_t *)0;

	*list_ptr = (list_t)object;
	return(LDR_SUCCESS);
}

static int
list_append(list_t *list, void *data)
{
	list_object_t  *object = (list_object_t *)list;
	element_t      *e;
	int             rc;

	/* allocate a list element */
	if ((rc = malloc(sizeof(*e), &e)) < 0)
		return(rc);

	e->e_next = (element_t *)0;
	e->e_data = data;

	if (object->lo_head) {
		object->lo_tail->e_next = e;
		object->lo_tail = e;
	} else {
		object->lo_head = e;
		object->lo_tail = e;
	}

	return(LDR_SUCCESS);
}

static list_iterator_id_t
list_next(list_t list, list_iterator_id_t id, void **datap)
{
	list_object_t *object = (list_object_t *)list;
	element_t     *e;

	if (id == LIST_ITERATOR_MARK) {
		if (e = object->lo_head) {
			*datap = e->e_data;
			return((list_iterator_id_t)e);
		} else
			return(LIST_ITERATOR_MARK);
	}
	e = (element_t *)id;
	if (e = e->e_next) {
		*datap = e->e_data;
		return((list_iterator_id_t)e);
	}
	return(LIST_ITERATOR_MARK);
}

static int
list_destroy(list_t list)
{
	list_object_t  *object = (list_object_t *)list;
	element_t      *e, *next;

	e = object->lo_head;
	while (e) {
		next = e->e_next;
		(void)free(e);
		e = next;
	}
	(void)free(object);
	return(LDR_SUCCESS);
}
