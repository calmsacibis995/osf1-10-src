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
static char	*sccsid = "@(#)$RCSfile: rcoff_relocate.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:18:17 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * rcoff_relocate.c
 *
 * Machine dependent relocation for a relocatable COFF file on the PMAX.
 *
 * OSF/1 Release 1.0
 */

/* #define DEBUG */

/*
 * Currently set up so debugging only works with
 * kernel load server.  Must be compiled -DDEBUG
 * and use '-d 7' (or greater) command line option.
 */
int kls_debug_level;

#ifdef	DEBUG
#define	dprintf(x)	if (kls_debug_level >= 7) ldr_msg x
#else
#define	dprintf(x)
#endif /* DEBUG */

#include <sys/types.h>
#include <sys/param.h>
#include <loader.h>

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

#include "open_hash.h"
#include "rcoff_machdep.h"
#include "rcoff_ldr.h"
#include "rcoff_relocate.h"

/*
 * Various definitions needed for PMAX relocation
 */

typedef int word_t;
typedef short half_t;

#define HALF_MAX 0x7fff
#define HALF_MIN -0x8000

#define JMPADDR_MASK 0x03ffffff
#define JMPADDR_SIGN_BIT 0x02000000

#define BITSIZEOF(t) (sizeof(t) * 8)

/* For RCOFF_*START macros */

#define round(x, s)     (((unsigned)(x) + (unsigned)(s) - 1) & ~((unsigned)(s) `
#define trunc(x, s)     (((unsigned)(x)) & ~((unsigned)(s) - 1))
#define aligned(x, s)   (((unsigned)(x) & ((unsigned)(s) - 1)) == 0)

/* 
 * MACHINE DEPENDENT ROUTINE:
 *
 * Process a single relocation for a section.
 *
 * NOTE: The loader could see several types of relocation. Imports
 * (symbol relocation), region relative relocation (regions in this module),
 * symbol relative relocation for exports, and local defined symbols.
 */

int
rcoff_relocate(rcoff_module_handle_t module,
	ldr_symbol_rec *imports, int nimports,
	ldr_region_rec *regions, int nregions,
	struct reloc *reloc, univ_t target, univ_t vaddr)
{
	struct filehdr *fhdr;
	struct aouthdr *ohdr;
	pEXTR esym;
	word_t value;
	word_t word;
	word_t immed;
	word_t vec_vaddr;

	esym = (pEXTR)module->symtab;
	fhdr = (struct filehdr *)module->fhd;
	ohdr = (struct aouthdr *)module->ohd;

	/* Determine the value for the relocation */

	vec_vaddr = 0;
	if (reloc->r_extern) {
		value = esym[reloc->r_symndx].asym.value;

		switch (esym[reloc->r_symndx].asym.sc) {
		case scText:
			value += (word_t)regions[TEXT_REGNO].lr_vaddr -
			    RCOFF_TXTSTART(fhdr, ohdr);
			break;

		case scBss:
		case scSBss:
			value += (word_t)regions[BSS_REGNO].lr_vaddr -
			    RCOFF_BSSSTART(fhdr, ohdr);
			break;

		case scData:
		case scSData:
			value += (word_t)regions[DATA_REGNO].lr_vaddr -
			    RCOFF_DATASTART(fhdr, ohdr);
			break;

		case scUndefined:
		case scSUndefined:
			imports += module->import_map[reloc->r_symndx];
			if (!ldr_symval_is_abs(&imports->ls_value)) {
				ldr_log("loader rcoff_relocate: can't relocate non-absolute symbols\n");
				return LDR_ERANGE;
			}
			value = (word_t)ldr_symval_abs(&imports->ls_value);
			vec_vaddr = (word_t)regions[TEXT_REGNO].lr_vaddr +
			    (word_t)regions[TEXT_REGNO].lr_size -
				module->nimport * sizeof(vector_t) +
			    module->import_map[reloc->r_symndx] * sizeof(vector_t);
			break;

		case scCommon:
		case scSCommon:
			ldr_log("loader rcoff_relocate: common relocation\n");
			return LDR_ENOEXEC;

		default:
			ldr_log("loader rcoff_relocate: unexpected symbol class (%d)\n",
			    esym[reloc->r_symndx].asym.sc);
			return LDR_ENOEXEC;
		}
	} else {
		switch (reloc->r_symndx) {
		case R_SN_TEXT:
		case R_SN_RDATA:
			value = (word_t)regions[TEXT_REGNO].lr_vaddr -
			    RCOFF_TXTSTART(fhdr, ohdr);
			break;

		case R_SN_DATA:
		case R_SN_SDATA:
			value = (word_t)regions[DATA_REGNO].lr_vaddr -
			    RCOFF_DATASTART(fhdr, ohdr);
			break;

		case R_SN_BSS:
		case R_SN_SBSS:
			value = (word_t)regions[BSS_REGNO].lr_vaddr -
			    RCOFF_BSSSTART(fhdr, ohdr);
			break;

		default:
			ldr_log("loader rcoff_relocate: bad section index (%d)\n",
			    reloc->r_symndx);
			return LDR_ENOEXEC;
		}
	}

	/* Do the relocation */

	switch (reloc->r_type) {
	case R_REFHALF:
		word = *(half_t *)target + value;

		/* The value, after relocation, for a REFHALF must fit
		 * into a half-word; if not it is an error
		 */

		if (word > HALF_MAX || word < HALF_MIN) {
			ldr_log("loader rcoff_relocate: half relocation out of range\n");
			return LDR_ENOEXEC;
		}
		*(half_t *)target = word;
		dprintf(("rcoff_relocate: R_REFHALF at 0x%08x (0x%08x): 0x%08x => 0x%08x\n",
		    vaddr, target, *(half_t *)target, (half_t)word));
		break;

	case R_REFWORD:
		dprintf(("rcoff_relocate: R_REFWORD at 0x%08x (0x%08x): 0x%08x => 0x%08x\n",
		    vaddr, target, *(word_t *)target, *(word_t *)target + value));

		*(word_t *)target += value;
		break;

	case R_REFHI: {
		struct reloc *reloc_lo;
		univ_t target_lo;
		word_t word_lo;

		/* An R_REFHI relocation entry MUST be have a corresponding
		 * R_REFLO entry directly following it to allow reconstruction
		 * of the constant to be added to the value for relocation.
		 */

		/* Get and verify the next relocation is a REFLO */

		reloc_lo = reloc + 1;
		if (reloc_lo->r_type != R_REFLO) {
			ldr_log("loader rcoff_relocate: REFHI not followed by REFLO\n");
			return LDR_ENOEXEC;
		}

		/* Make sure the REFLO refers to the same address */

		if (reloc_lo->r_extern != reloc->r_extern ||
		    reloc_lo->r_symndx != reloc->r_symndx) {
			ldr_log("loader rcoff_relocate: bad REFLO entry for REFHI\n");
			return LDR_ENOEXEC;
		}

		/* Get the address being relocated by the REFLO */

		target_lo = ((char *)target - reloc->r_vaddr) + reloc_lo->r_vaddr;

		/* Get the data being relocated */

		word_lo = *(half_t *)target_lo;
		word = *(half_t *)target;
		immed = (word << BITSIZEOF(half_t)) + word_lo;

		/* Perform the relocation */

		dprintf(("rcoff_relocate: R_REFHI   at 0x%08x (0x%08x): 0x%08x => 0x%08x\n",
		    vaddr, target, immed, immed + value));
		immed += value;

		/* Store the relocated values. Note that the immediate
		 * value for REFLO gets sign-extended, so when it is
		 * negative the value for REFHI must be compensated
		 * (the + 0x8000)
		 */

		*(half_t *)target_lo = immed;
		*(half_t *)target = (immed + 0x8000) >> BITSIZEOF(half_t);

		return LDR_RELOC_HI_LO;
	}

	case R_REFLO:
		dprintf(("rcoff_relocate: R_REFLO   at 0x%08x (0x%08x): 0x%08x => 0x%08x\n",
		    vaddr, target, *(half_t *)target, (half_t)(*(half_t *)target + value)));

		/* A REFLO can be seen without a corresponding REFHI.
		 * In this case just the low 16 bits of the value, after
		 * relocation, is used
		 */

		*(half_t *)target += value;
		break;

	case R_JMPADDR:
		word = *(word_t *)target;
		immed = (word & JMPADDR_MASK) << 2;
		if (reloc->r_extern) {

			/* For external references, the constant at the
			 * entry is an offset from the symbol and needs
			 * to be sign extended
			 */

			if (immed & (JMPADDR_SIGN_BIT << 2))
				immed |= ~(JMPADDR_SIGN_BIT << 2);

			/* If the reference is to an import, make sure
			 * the the target is in the same segment.
			 * Otherwise point us to the vector for the import.
			 */

			if (immed == 0 && vec_vaddr != 0 &&
			    (value & ~(JMPADDR_MASK << 2)) !=
			    ((word_t)vaddr & ~(JMPADDR_MASK << 2))) {
				value = vec_vaddr;
			}
		} else {

			/* For local references, the constant at the entry
			 * is the target of the jump. The high 4 bits will
			 * come from the address of the jump + 4
			 */

			immed |= (reloc->r_vaddr + sizeof(word_t)) &
			    ~(JMPADDR_MASK << 2);
		}


		/* Do the relocation and check to see if the target of the
		 * branch can be reached from the location of the branch.
		 * The high 4 bits of both must be the same because of the
		 * way the jump instruction works.
		 */

		dprintf(("rcoff_relocate: R_JMPADDR at 0x%08x (0x%08x): 0x%08x => 0x%08x\n",
		    vaddr, target, immed, immed + value));

		immed += value;
		if ((immed & ~(JMPADDR_MASK << 2)) !=
		    ((word_t)vaddr & ~(JMPADDR_MASK << 2))) {
			ldr_log("loader rcoff_relocate: jump relocation out of range; can't jump from 0x%08x to 0x%08x\n",
			    (word_t)vaddr, immed);
			return LDR_ENOEXEC;
		}

		*(word_t *)target = (word & ~(JMPADDR_MASK)) |
		    (JMPADDR_MASK & (immed >> 2));
		break;

	case R_GPREL:
		ldr_log("loader rcoff_relocate: unsupported relocation type (R_GPREL)\n");
		return LDR_ENOEXEC;

	case R_LITERAL:
		ldr_log("loader rcoff_relocate: unsupported relocation type (R_LITERAL)\n");
		return LDR_ENOEXEC;

	case R_ABS:
		ldr_log("loader rcoff_relocate: unsupported relocation type (R_ABS)\n");
		return LDR_ENOEXEC;

	default:
		ldr_log("loader rcoff_relocate: unsupported relocation type (%d)\n",
		    reloc->r_type);
		return LDR_ENOEXEC;
	}

	return LDR_SUCCESS;
}
