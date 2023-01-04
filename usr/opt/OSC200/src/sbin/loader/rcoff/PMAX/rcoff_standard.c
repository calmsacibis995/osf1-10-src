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
static char	*sccsid = "@(#)$RCSfile: rcoff_standard.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:18:34 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * rcoff_standard.c
 *
 * Machine dependent portion of the relocatable COFF format
 * dependent manager for the PMAX.  This should
 * probably work unchanged for most architectures.
 *
 * OSF/1 Release 1.0
 */


/* #define DEBUG */

/*
 * Currently set up so debugging only works with
 * kernel load server.  Must be compiled -DDEBUG
 * and use '-d 3' (or greater) command line option.
 */
int kls_debug_level;

#ifdef	DEBUG
#define	dprintf(x)	if (kls_debug_level >= 3) ldr_msg x
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

#include "ldr_hash.h"
#include "open_hash.h"
#include "rcoff_machdep.h"
#include "rcoff_ldr.h"
#include "rcoff_relocate.h"

/* For RCOFF_*START macros */

#define round(x, s)     (((unsigned)(x) + (unsigned)(s) - 1) & ~((unsigned)(s) `
#define trunc(x, s)     (((unsigned)(x)) & ~((unsigned)(s) - 1))
#define aligned(x, s)   (((unsigned)(x) & ((unsigned)(s) - 1)) == 0)

/* Relocate all symbols for a particular section that need to be relocated. */

int
rcoff_relocate_section(rcoff_module_handle_t module,
			int nregions, ldr_region_rec *regions,
			int nimports, ldr_symbol_rec *imports,
			int sectno)
{
	ldr_window_t *rwp;		/* Relocations window pointer */
	struct reloc *relocs;		/* The section's relocations */
	struct scnhdr *shdr;		/* The section's header */
	struct filehdr *fhdr;		/* The file's header */
	struct aouthdr *ohdr;		/* The optional header */
	int regno;			/* Region # containing the section */
	int rc;				/* Return code */
	int i;
	long orig_vaddr;		/* vaddr of section as linked */

	fhdr = (struct filehdr *)module->fhd;
	ohdr = (struct aouthdr *)module->ohd;
	shdr = ((struct scnhdr *)module->shd) + sectno;
	dprintf(("rcoff_relocate_section: relocating %s\n", shdr->s_name));

	/* Locate the region containing the section */

	if (shdr->s_flags == STYP_TEXT) {
		regno = TEXT_REGNO;
		orig_vaddr = RCOFF_TXTSTART(fhdr, ohdr);
	} else if (shdr->s_flags == STYP_DATA || shdr->s_flags == STYP_SDATA ||
		   shdr->s_flags == STYP_RDATA) {
		regno = DATA_REGNO;
		orig_vaddr = RCOFF_DATASTART(fhdr, ohdr);
	} else {
		dprintf(("rcoff_relocate_section: unable to locate region\n"));
		return LDR_ENOEXEC;
	}
	if (regno >= nregions) {
		dprintf(("rcoff_relocate_section: region screw up\n"));
		return LDR_ENOEXEC;
	}

	/* Window in the relocations */

	rwp = ldr_init_window(module->fd);
	relocs = (struct reloc *)ldr_file_window(shdr->s_relptr,
	    shdr->s_nreloc * sizeof(struct reloc), rwp);
	if (relocs == NULL) {
		dprintf(("rcoff_relocate_section: failure to window in relocs\n"));
		return LDR_ENOEXEC;
	}

	/* Handle each of the relocations */

	for (i = 0; i < shdr->s_nreloc; i++) {
		univ_t target;		/* location to patch */
		univ_t vaddr;		/* its corresponding virtual addr */

		/* Compute the address to relocate.
		 * These may be different when kernel loading.
		 */

		target = (univ_t)((char *)regions[regno].lr_mapaddr +
		    relocs[i].r_vaddr - orig_vaddr);
		vaddr = (univ_t)((char *)regions[regno].lr_vaddr +
		    relocs[i].r_vaddr - orig_vaddr);

		/* Do the relocation */

		rc = rcoff_relocate(module, imports, nimports,
		    regions, nregions, relocs + i, target, vaddr);
		if (rc == LDR_RELOC_HI_LO) {
			i++;
		} else if (rc != LDR_SUCCESS) {
			goto exit_relocate;
		};
	}
	rc = LDR_SUCCESS;

	/* Cleanup */

    exit_relocate:
	ldr_unwindow(rwp);

	return rc;
}


int rcoff_init_vectors(vector_t *vec, int nimports, ldr_symbol_rec *imports)
{
	unsigned long value;
	int i;


	dprintf(("rcoff_init_vectors: entered\n"));

	for (i = 0; i < nimports; i++) {
		value = (unsigned long)ldr_symval_abs(&imports[i].ls_value);

		vec[i][0].word = LUI(AT_REG, (value >> 16));
		vec[i][1].word = ORI(AT_REG, AT_REG, value & 0xffff);
		vec[i][2].word = JR(AT_REG);
		vec[i][3].word = NOP;
	}

	return LDR_SUCCESS;
}
