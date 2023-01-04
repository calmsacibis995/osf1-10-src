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
static char	*sccsid = "@(#)$RCSfile: standard.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:41:38 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * standard.c
 *
 * Machine dependent portion of the Mach-O format
 * dependent manager for the PMAX.  This should
 * probably work unchanged for most architectures.
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
#include <strings.h>

#include "ldr_types.h"
#include "ldr_sys_int.h"
#include "ldr_windows.h"
#include "ldr_malloc.h"
#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_errno.h"

#include <mach_o_header.h>
#include <mach_o_format.h>

#include "ldr_hash.h"
#include "open_hash.h"
#include "mo_ldr.h"
#include "relocate.h"

/* declarations for local routines */

static int 
internal_get_reloc_value(macho_module_handle_ptr_t handle, reloc_info_t *reloc_p,
			 reloc_command_t *reloc_lc, ldr_symbol_rec *imports, 
			 ldr_region_rec *regions, int nregions,
			 mo_long_t *relocation, int nimports);

/*
 *	relocate all symbols for a particular region that need to be
 *	relocated.
 */
/*
 *	NOTE: fix the way a region's protection is changed to make it
 *	writable for relocation purposes.  Currently such regions 
 *	come back as rx. This is not right !
 */
int
mach_dep_relocate_region(macho_module_handle_ptr_t handle, 
			 ldr_region_rec *regions, int nregions,
			 int npackages, ldr_package_rec *import_pkgs, 
			 int nimports, ldr_symbol_rec *imports,
			 int reg_no)
{
	region_command_t *reg_lc;		/* region load command (LC) */
	mo_lc_entry_t *region_entry;		/* region load command table entry */
	reloc_command_t *reloc_lc;		/* relocation LC */
	mo_lc_entry_t *reloc_entry;		/* relocation LC table entry */
	reloc_info_t *reloc_p;			/* per symbol relocation info */
	ldr_prot_t old_prot;			/* original protection for region */
	int reloc_count;			/* number of relocations */
	int write_prot_flag;			/* TRUE if region was originally writable */
	univ_t target;				/* memory location to patch with relocation */
	univ_t vaddr;				/* it's corresponding virtual address */
	mo_long_t relocation;			/* relocation value to be poked at target */
	ldr_region_rec *reg;			/* region information for region being relocated */
	int i;					/* loop variable */
	int rc;					/* return code/status */
	
	/* region record for the region being relocated */
	CHECK_REG_NO(reg_no, nregions);

	reg = &regions[reg_no];
	/* region load command */
	region_entry = (mo_lc_entry_t *)
		REGION_LCTABLE_ENTRY(handle->region_table, reg_no);
	reg_lc = (region_command_t *) region_entry->LC;
	reloc_entry = handle->load_cmd_table + 	reg_lc->regc_reloc_addr;
	reloc_lc = (reloc_command_t *) reloc_entry->LC;

	/* 
	 * if region is write protected, change its protection
	 * temporarily to permit relocation.
	 */
	write_prot_flag = (reg->lr_prot & LDR_W) ? TRUE : FALSE;
	if (write_prot_flag == FALSE) {
		rc = ldr_mprotect(reg->lr_mapaddr, reg->lr_size, 
			 LDR_PROT_READ | LDR_PROT_WRITE | LDR_PROT_EXEC);
		if (rc != LDR_SUCCESS) {
			dprintf(("mach_dep_relocate_region : failure to make region writeable\n"));
			return rc;
		}
		dprintf(("mach_dep_relocate_region: region made writable for reloc\n"));
	}

	/* for each relocation record, perform relocation */
	reloc_count = reloc_lc->relc_nentries;
	for (i = 0; i < reloc_count; i++) {
		reloc_p = ((reloc_info_t *)(reloc_entry->section_data)
			   + i);

		/* compute address to relocate (called target and vaddr) */
		dprintf(("mach_dep_relocate_region: regno : %d lr_mapaddr : 0x%x ri_offset 0x%x\n",
			 reg_no, regions[reg_no].lr_mapaddr, reloc_p->ri_roffset));
		target = (univ_t)((char*) regions[reg_no].lr_mapaddr + 
				  reloc_p->ri_roffset);
		dprintf(("mach_dep_relocate_region: target : 0x%x\n",target));
		vaddr = (univ_t)((char*) regions[reg_no].lr_vaddr + 
				  reloc_p->ri_roffset);
		dprintf(("mach_dep_relocate_region: vaddr : 0x%x\n",vaddr));

		/* get relocation value */
		rc = internal_get_reloc_value(handle, reloc_p, reloc_lc, imports, 
				    regions, nregions, &relocation, nimports);
		if (rc != LDR_SUCCESS) {
			dprintf(("mach_dep_relocate_region: error getting relocation value\n"));
			return rc;
		}
		dprintf(("mach_dep_relocate_region: relocation : 0x%x\n",relocation));

		/* fill in relocation value at target address */
		rc = patch_reloc_addr(regions, reg_no, reloc_p,
				      target, vaddr, relocation);
		if ((rc != LDR_SUCCESS) && (rc != LDR_RELOC_HI_LO)) {
			dprintf(("mach_dep_relocate_region: failure to reloc (reloc rec no %d)\n",
				 i));
			return rc;
		}

		/* this special kind of PMAX relocation looks at two relocation recs */
		if (rc == LDR_RELOC_HI_LO) {
			i++;
		}

		dprintf(("mach_dep_relocate_region: relocation of symbol done\n"));

	}

	/* if region protection was changed, make region non-writable again */
	if (write_prot_flag == FALSE) {
		rc = ldr_mprotect(reg->lr_mapaddr, reg->lr_size, 
				  LDR_PROT_READ | LDR_PROT_EXEC);
		if (rc != LDR_SUCCESS) {
			dprintf(("internal_relocate : failure to reset region protection\n"));
			return rc;
		}
	}

	return LDR_SUCCESS;

}

/*
 * get the relocation value that needs to be patched at
 * the target address.
 */
static int 
internal_get_reloc_value(macho_module_handle_ptr_t handle, reloc_info_t *reloc_p,
			 reloc_command_t *reloc_lc, ldr_symbol_rec *imports, 
			 ldr_region_rec *regions, int nregions, 
			 mo_long_t *relocation, int nimports)
{
	symbol_info_t *sym_p;		/* symbol information record for symbol rel reloc */
	symbols_command_t *sym_lc;	/* symbol load cmd */
	mo_lc_entry_t *reg_entry;	/* region load cmd table entry */
	mo_lc_entry_t *sym_lc_entry;	/* symbol load cmd table entry */
	ldr_symbol_rec *import_p;	/* import containing relocation value */
	region_command_t *reg_lc;	/* region load cmd */
	univ_t regvaddr;		/* linker's idea of region VA */
	univ_t reg_vm_start;		/* used for region relocation */
	char *reg_addr;			/* address where region is mapped */
	int sym_lc_id;			/* symbol load command id */
	int sym_id;			/* index into table of symbol_info_t's/ imports */
	int reg_lc_id;			/* region load command id */
	int reg_id;			/* region number */
	int reg_off;			/* offset into region cmd section */
	int rc;				/* return code/status */


	if (reloc_p->ri_flags & RI_SYMBOL_F) {	/* symbol relative imp/exp relocation */
		/* check whether import or export symbol rel relocation */
		sym_lc_id = reloc_p->ri_symbol_index.adx_lcid;
		CHECK_LC_ID(handle, sym_lc_id);
		sym_lc_entry = handle->load_cmd_table + sym_lc_id;
		sym_lc = sym_lc_entry->LC;
		sym_id = reloc_p->ri_symbol_index.adx_index;
		if (sym_lc->ldc_cmd_type == LDC_SYMBOLS) {
			if (sym_lc->symc_kind ==  SYMC_IMPORTS) {	/* import relocation */
				if (sym_id >= nimports) {
					dprintf(("internal_get_reloc_value: import symbol id %d out of range\n",
						 sym_id));
					return LDR_ENOEXEC;
				}
				import_p = imports + sym_id;
				if (ldr_symval_is_abs(&(import_p->ls_value))) {
					dprintf(("internal_get_reloc_value: *relocation from abs value\n"));
					*relocation = (mo_long_t) 
						ldr_symval_abs(&(import_p->ls_value));
				}
				else {
					dprintf(("internal_get_reloc_value: the loader cannot handle non abs import symvals\n"));
					return LDR_ERANGE;
				}
			}						/* else SYMC_IMPORTS */
			else if (sym_lc->symc_kind == SYMC_DEFINED_SYMBOLS) { /* export relocation */
				rc = window_in_section(handle, sym_lc_entry);
				if (rc != LDR_SUCCESS) {
					dprintf(("internal_get_reloc_value: failure to window defined symbols\n"));
					return rc;
				}

				if (sym_id >= sym_lc->symc_nentries) {
					dprintf(("internal_get_reloc_value: invalid index %d into symbol section\n",
						 sym_id));
					return LDR_ENOEXEC;
				}
				sym_p = (symbol_info_t *)sym_lc_entry->section_data + sym_id;

				if (sym_p->si_flags & SI_COMMON_F) {
					ldr_msg("internal_get_reloc_value: error loader cannot handle SI_COMMON_F\n");
					return LDR_ENOEXEC;
				}

				if (sym_p->si_flags & SI_ABSOLUTE_VALUE_F) {
					dprintf(("internal_get_reloc_value: SI_ABSOLUTE_VALUE_F\n"));
					*relocation = (mo_long_t) sym_p->si_abs_val;
				}
				else if (sym_p->si_flags & SI_IMPORT_F) {
					dprintf(("internal_get_reloc_value: error expected local/export symbol not import\n"));
					return LDR_ENOEXEC;
				}
				else if (sym_p->si_flags & SI_LITERAL_F) {
					dprintf(("internal_get_reloc_value: error cannot handle SI_LITERAL_F\n"));
					return LDR_ENOEXEC;
				}
				else {
					reg_lc_id = sym_p->si_def_val.adr_lcid;
					CHECK_LC_ID(handle, reg_lc_id);
					reg_id = LC_TO_REGNO(((mo_lc_entry_t *)handle->load_cmd_table + reg_lc_id));
					dprintf(("internal_get_reloc_value: def_val - reg_id : %d\n", reg_id));
					CHECK_REG_NO(reg_id, nregions);
					reg_addr = (char*) regions[reg_id].lr_vaddr;
					*relocation = (mo_long_t) ((char *)reg_addr + 
								   (int)sym_p->si_def_val.adr_sctoff);
				}
			}
			else {
				dprintf(("internal_get_reloc_value: symbol rel reloc not for export or import\n"));
				return LDR_ENOEXEC;
			}
		}
		else {
			dprintf(("internal_get_reloc_value: symbol rel reloc does not point at LDC_SYMBOLS LC\n"));
			return LDR_ENOEXEC;
		}
	}
	else if (reloc_p->ri_flags & RI_LOC_F) { /* region relative relocation */
		dprintf(("internal_get_reloc_value: region realtive relocation\n"));
		CHECK_LC_ID(handle, (int)(reloc_p->ri_loc_addr.adr_lcid));
		reg_entry = handle->load_cmd_table + reloc_p->ri_loc_addr.adr_lcid;
		reg_id = LC_TO_REGNO(reg_entry); /* region number */
		CHECK_REG_NO(reg_id, nregions);
		reg_lc = reg_entry->LC;
		reg_off = reloc_p->ri_loc_addr.adr_sctoff; /* offset into region */
		reg_vm_start = (regions + reg_id)->lr_vaddr; 
		*relocation = (mo_long_t) ((char *)reg_vm_start + reg_off);

		/* If the value at the relocation address contains an actual
		 * virtual address instead of an offset into the section (as
		 * is the case with the certain generated coff files, then subtract
		 * the input virtual address of the region being relocated. 
		 */
		if ((reloc_p->ri_flags & RI_RELOC_VADDR_F) || 
		    (!(reloc_p->ri_flags & RI_RELOC_OFFSET_F))) {
			dprintf(("internal_get_reloc_value: subtract region virtual addr from relocation\n"));
			(*relocation) -= (mo_long_t) regvaddr;
		}

	}
	else {
		dprintf(("internal_get_reloc_value: neither RI_SYMBOL_F nor RI_LOC_F flag set for reloc record\n"));
		return LDR_ENOEXEC;
	}

	return LDR_SUCCESS;
}


/* convert the si_value stored in the macho symbol_info_t
 * into a loader ldr_symval_t.
 */
int sivalue_to_ldrsymval(macho_module_handle_ptr_t handle, symbol_info_t *sym_p, 
			ldr_symval *sym_value)
{
	mo_lc_entry_t *reg_entry;

	if (sym_p->si_flags & SI_COMMON_F)
		return(LDR_ERANGE);
	else if ((sym_p->si_flags & SI_LITERAL_F) || 
	    (sym_p->si_flags & SI_ABSOLUTE_VALUE_F)) {

			ldr_symval_make_abs(sym_value, 
					    (univ_t)sym_p->si_lit_val);
	}
	else {
		reg_entry = handle->load_cmd_table + 
			sym_p->si_def_val.adr_lcid;
		ldr_symval_make_regrel(sym_value,
				       reg_entry->region_id,
				       sym_p->si_def_val.adr_sctoff);
	}

	if (sym_p->si_flags & SI_DATA_F)
		ldr_symval_make_data(sym_value);
	else if (sym_p->si_flags & SI_CODE_F)
		ldr_symval_make_function(sym_value);
	/* else it's undefined */

	return LDR_SUCCESS;
}
