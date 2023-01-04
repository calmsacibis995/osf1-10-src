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
static char	*sccsid = "@(#)$RCSfile: relocate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:41:33 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * relocate.c
 *
 * machine dependent relocation for a Mach-O file
 * on the PMAX.
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
#include "ldr_errno.h"
#include "ldr_symval_std.h"
#include "ldr_hash.h"

#include <mach_o_header.h>
#include <mach_o_format.h>

#include "open_hash.h"
#include "mo_ldr.h"
#include "relocate.h"


#ifdef DEBUG

/* debugging procedures */

extern void 
print_symbol_rec(symbol_info_t *p);

void 
print_relocation_rec(reloc_info_t *p);

void 
print_region_recs(ldr_region_rec *p, int no_recs);

#endif /* DEBUG */

/* macros and data structures needed for PMAX relocation */

#define RELOC_VALUE_RIGHTSHIFT(r) (reloc_target_rightshift[(r)->ri_size_type])
#define RELOC_TARGET_BITSIZE(r) (reloc_target_bitsize[(r)->ri_size_type])

#define	JMPADDR_SEGMENT_MASK	0xf0000000

/* 
 * NOTE : these are very dependent on the order of the relocation types
 * in mach_o_types.h.
 */

/*	ABS	RHALF	RWOR	JMPAD	REFHI	REFLO	GPREL	LIT */
static int reloc_target_rightshift[] = {
	0,	0,	0,	2,	16,	0,	0,	0
};

static int reloc_target_bitsize[] = {
	32,	16,	32,	26,	16,	16,	0,	0
};

#define REFLO_SIGN_BIT	0x00008000

/* 
 * MACHINE DEPENDENT ROUTINE :
 *
 * process a single relocation for a region.
 *
 */
/* note : loader could see two types of relocation. Imports (symbol relocation)
 * and region relative relocation (regions in this module). Oh ! and until
 * the linker is fixed - also see symbol relative relocation for exports
 * and local defined symbols.
 */
int
patch_reloc_addr(ldr_region_rec *regions, int reg_no,
		 reloc_info_t *reloc_p,  univ_t target, univ_t vaddr,
		 mo_long_t relocation)
{
	mo_long_t relocation_saved;	/* place to save relocation value */
	unsigned int mask;		/* relocation mask */
	int ref_hilo_flag = FALSE;	/* TRUE if a REF_HI followed by REF_LO is seen */
	int rc;				/* return code/status */


	relocation_saved = relocation;
	relocation >>= RELOC_VALUE_RIGHTSHIFT(reloc_p);

	dprintf(("patch_reloc_addr: reloc_p->ri_size_type : 0x%x relocation : 0x%x\n",
		 reloc_p->ri_size_type, relocation));

	/* unshifted mask for relocation */
	mask = 1 << RELOC_TARGET_BITSIZE(reloc_p) - 1;

	dprintf(("patch_reloc_addr: mask : 0x%x \n"));
	mask |= mask - 1;
	dprintf(("patch_reloc_addr: mask = 0x%x\n", mask));
	relocation &= mask;
	dprintf(("relocation_symbol : relocation after masking : 0x%x\n",
		 relocation));

	dprintf(("patch_reloc_addr: fixup target based on relocation\n"));
	/* fixup target with relocation based on relocation type */

	switch(reloc_p->ri_size_type) {
	case R_ABS_T :
		dprintf(("patch_reloc_addr (pmax) : encountered R_ABS_T\n"));
		break;
	case R_REFHALF_T :
		dprintf(("patch_reloc_addr: R_REFHALF_T\n"));

		relocation += mask & *(short *) target;
		*(short *) target &= ~mask;
		*(short *) target |= relocation;
		break;

	case R_REFWOR_T :
		dprintf(("patch_reloc_addr: R_REFWOR_T\n"));

		relocation += mask & *(long *) target;
		*(long *) target &= ~mask;
		*(long *) target |= relocation;
		break;

	case R_JMPADDR_T :
		dprintf(("patch_reloc_addr: R_JMPADDR_T\n"));

		if (((((int)vaddr) + 4) & JMPADDR_SEGMENT_MASK)
		    != (((int)relocation_saved) & JMPADDR_SEGMENT_MASK)) {
			ldr_log("loader patch_reloc_addr: R_JMPADDR_T crosses segment boundary: source=0x%x, target=0x%x\n",
				vaddr, relocation_saved);
			return LDR_ENOEXEC;
		}

		dprintf(("relocation: 0x%x *target : 0x%x\n", relocation, 
			 *(long *)target));
		relocation += mask & *(long *) target;
		dprintf(("relocation: after += mask : 0x%x\n",relocation));
		*(long *) target &= ~mask;
		*(long *) target |= relocation;
		dprintf(("R_JMPADDR_T: relocation poked at target : 0x%x\n",
			 *(long *)target));
		break;

	case R_REFHI_T :
		dprintf(("patch_reloc_addr: R_REFHI_T\n"));
		{
		char *reflo_target;
		mo_long_t old_value;
		mo_long_t new_value;
		mo_long_t refhi_relocation;
		mo_long_t reflo_relocation;

		/* the REFHI/REFLO relocation types are processed together. */
		/* next entry must be a REFLO */
		reloc_p++;
		if (reloc_p->ri_size_type != R_REFLO_T) {
			ldr_msg(("patch_reloc_addr: REFHI not followed by REFLO\n"));
			return -1;
		}
		ref_hilo_flag = TRUE;
		dprintf(("patch_reloc_addr: processing REFLO following R_REFHI_T\n"));

		/* get new target for reflo */
		/* note: next reloc's region must be the same for a REFLO reloc */
		reflo_target = (univ_t)((char*) regions[reg_no].lr_mapaddr + 
				  reloc_p->ri_roffset);
		dprintf(("patch_reloc_addr: reflo target : 0x%x\n",target));

		/* let us first get the old value that is in the targets */
		old_value = (mo_long_t) (((*(long *)target & mask) << 16)
					 + (signed int) 
					 (*(long *) reflo_target & mask));

		/* 
		 * if the old_value is negative, subtract 1 from the refhi 
		 * portion... it was incremented to compensate for sign
		 * extension by the assembler.  What do we want gas to do ?
		 */


		if (REFLO_SIGN_BIT & old_value) old_value -= 0x00010000;

		new_value = old_value + relocation_saved;

		/* now lets take this new value and place it in the instructions */
		refhi_relocation = new_value >> 16;
		reflo_relocation = new_value & mask;
		
		/* before we relocate the HI target, see if we need to add 1 to
		   the high 16 bits to account for sign extension of the low part. */
		if (reflo_relocation & 0x00008000) refhi_relocation += 1;

		/* relocate the REFLO part */
		*(short *) reflo_target &= ~mask;
		*(short *) reflo_target |= reflo_relocation;

		/* relocate the REFHI part */
		*(short *) target &= ~mask;
		*(short *) target |= refhi_relocation;

		break;
	}

	case R_REFLO_T :
		dprintf(("patch_reloc_addr: R_REFLO_T\n"));

		relocation += mask & *(short *) target;
		*(short *) target &= ~mask;
		*(short *) target |= relocation;
		break;

	case R_GPREL_T :
		ldr_log("patch_reloc_addr: R_GPREL_T - unsupported reloc type\n");
		return LDR_ENOEXEC;
		break;

	case R_LITERAL_T :
		ldr_log("patch_reloc_addr: R_LITERAL_F - unsupported reloc type\n");
		return LDR_ENOEXEC;
		break;

	default:
		ldr_log("patch_reloc_addr: default : unsupported relocation type\n");
		return LDR_ENOEXEC;
		break;
	}

	dprintf(("patch_reloc_addr: target addr fixed - relocation done\n"));

	/* LDR_RELOC_HI_LO tells the caller that we looked at 2 reloc records */

	return (ref_hilo_flag == TRUE) ? LDR_RELOC_HI_LO : LDR_SUCCESS;
}




#ifdef DEBUG


void print_relocation_rec(reloc_info_t *p)
{
	ldr_msg("print_relocation_rec : at 0x%x\n",p);
	ldr_msg("reloc.ri_roffset : 0x%x reloc.ri_flags : 0x%x reloc.ri_size_type : 0x%x\n",
	       p->ri_roffset, p->ri_flags, p->ri_size_type);

	if (p->ri_flags & RI_SYMBOL_F) {
		ldr_msg("reloc.ri_symbol_index.adx_lcid : 0x%x\n",
		       p->ri_symbol_index.adx_lcid);
		ldr_msg("reloc.ri_symbol_index.adx_index : 0x%x\n",
		       p->ri_symbol_index.adx_index);
	}
	else if (p->ri_flags & RI_LOC_F) {
		ldr_msg("reloc.ri_loc_addr.adr_lcid : 0x%x\n",
		       p->ri_loc_addr.adr_lcid);
		ldr_msg("reloc.ri_loc_addr.adr_sctoff : 0x%x\n",
		       p->ri_loc_addr.adr_sctoff);
		       
	}
	else {
		ldr_msg("print_relocation_rec : error neither SYMBOL nor LOC\n");
		ldr_msg("print_relocation_rec : default - as mo_addr_t\n");
		ldr_msg("reloc.ri_loc_addr.adr_lcid : 0x%x\n",
		       p->ri_loc_addr.adr_lcid);
		ldr_msg("reloc.ri_loc_addr.adr_sctoff : 0x%x\n",
		       p->ri_loc_addr.adr_sctoff);
	}
}

void print_symbol_rec(symbol_info_t *p)
{
	ldr_msg("print_symbol_rec : at 0x%x\n", p);
	ldr_msg("symbol.symbol_name : 0x%x symbol.package_index : 0x%x symbol.type : 0x%x\n",
	       p->si_symbol_name, p->si_package_index, p->si_type);
	ldr_msg("symbol.flags : 0x%x symbol.reserved_byte : 0x%x symbol.sc_type : 0x%x\n",
	       p->si_flags, p->si_reserved_byte, p->si_sc_type);
	

	if (p->si_flags & SI_EXPORT_F) {
		ldr_msg("symbol is an EXPORT\n");
	}
	if (p->si_flags & SI_ABSOLUTE_VALUE_F) {
		ldr_msg("symbol.abs_val : 0x%x\n", p->si_abs_val);
	}
	else if (p->si_flags & SI_LITERAL_F) {
		ldr_msg("symbol.lit_val : 0x%x\n", p->si_lit_val);
	}
	else if (p->si_flags & SI_IMPORT_F) {
		ldr_msg("symbol.imp_val : 0x%x\n", p->si_imp_val);
	}
	else {
		ldr_msg("default symbol.def_val.adr_lcid : 0x%x symbol.def_val.adr_sctoff : 0x%x\n",
		       p->si_def_val.adr_lcid, p->si_def_val.adr_sctoff);
	}
	
}

void print_region_recs(ldr_region_rec *p, int no_recs) 
{
	int i;

	ldr_msg("print_region_recs : no of region recs : %d\n",no_recs);

	for (i = 0; i < no_recs; i++, p++) {
		ldr_msg("%d : region.version: %d region.name: %s region.prot : 0x%x\n",
		       i, p->lr_version, p->lr_name, p->lr_prot);
		ldr_msg("     region.vaddr : 0x%x region.mapaddr : 0x%x region.size : 0x%x region.flags : 0x%x\n",
		       p->lr_vaddr, p->lr_mapaddr, p->lr_size, p->lr_flags);

	}
}


#endif /* DEBUG */
