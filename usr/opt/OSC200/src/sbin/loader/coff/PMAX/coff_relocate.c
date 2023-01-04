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
static char	sccsid[] = "@(#)$RCSfile: coff_relocate.c,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/03/19 09:14:31 $";
#endif 
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
 *
 * 01-Mar-1991, Ken Lesniak
 *	Add support for COFF/ELF shared library relocations
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * coff_relocate.c
 *
 * Machine dependent portion of the COFF format
 * dependent manager for the PMAX.  This should
 * probably work unchanged for most architectures.
 *
 * OSF/1 Release 1.0
 */


/* #define DEBUG 1  */
#define PARANOID 1

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

#include "ldr_hash.h"
#include "open_hash.h"
#include "coff_machdep.h"
#include "coff_ldr.h"

/* Perform all dynamic relocations for an object. The following diagrams
 * show some of the important relationships between the dynamic sections:
 *
 *	   GOT				   .dynsym
 *
 *					------------
 *					| reserved |
 *					------------
 *	------------			| locals   |
 *	|  binder  |			|          |
 *	------------			------------
 *	|          |			| unref'd  |<- first one pointed to
 *	|  locals  |			| globals  |   by DT_MIPS_UNREFEXTNO
 *	|          |			|          |
 *	------------			------------
 *	|          | <-- one to one -->	|  ref'd   |<- first one pointed to
 *	| globals  | <-- one to one -->	| globals  |   by DT_GOTSYM
 *	|          | <-- one to one -->	|          |
 *	------------			------------
 *
 *
 *	  .dynsym			  .packsym		  .package
 *
 *	------------
 *	| reserved |
 *	------------						------------
 *	| locals   |						| reserved |
 *	|          |						------------
 *	------------			------------		| imports  |
 *	| unref'd  | <-- one to one -->	|          |		|          |
 *	| globals  | <-- one to one -->	|  index   |		------------
 *	|          | <-- one to one -->	|    of    |		| import & |
 *	------------			|  entry   | =========>	| exports  |
 *	|  ref'd   | <-- one to one -->	|    in    |		------------
 *	| globals  | <-- one to one -->	| .package |		| exports  |
 *	|          | <-- one to one -->	|          |		|          |
 *	------------			------------		------------
 */

int
coff_dynamic_relocate(coff_module_handle_t module,
			int nimports, ldr_symbol_rec *imports,
			int nregions, ldr_region_rec *regions)
{
	Elf32_Sym *dynsym;		/* Dynamic symbol table */
	Elf32_Rel *rel_dyn;		/* Dynamic relocation data */
	Elf32_Got *got;			/* Global offset table */
	Elf32_Package *package;		/* Package table */
	Elf32_Word *packsym;		/* Package/symbol map */
	long vdiff, pdiff, diff;
	int i;
	int ri;				/* Relocation (.rel_dyn) index */
	int si;				/* Symbol (.dynsym) index */
	int ii;				/* Import symbol index */
	int relno;			/* Number of relocations */
	Elf32_Addr paddr;
	Elf32_Word word;
	int ilow, ihigh;		/* For binary searching */
	int rc;


	dprintf(("coff_dynamic_relocate: entered\n"));

	/* Get the difference between the linked base address of the module
	 * and the address where it has been mapped (physical and virtual)
	 * These are the same except for kernel loading.
	 */

	pdiff = (long)module->base_paddress - module->d_base_address;
	vdiff = (long)module->base_vaddress - module->d_base_address;
	if (module->rdata_start) {
	    module->rdata_start += pdiff;
	    if ((rc = ldr_mprotect((univ_t) module->rdata_start,
			   module->rdata_size,
			   LDR_PROT_WRITE|LDR_PROT_READ|LDR_PROT_EXEC)) !=
		LDR_SUCCESS) return rc;
	}

	/* Get pointers to where the various data structures have been mapped */

	rel_dyn = (Elf32_Rel *)((char *)module->d_rel + pdiff);
	dynsym = (Elf32_Sym *)((char *)module->d_symtab + pdiff);
	got = (Elf32_Got *)((char *)module->d_got + pdiff);
	package = (Elf32_Package *)((char *)module->d_package + pdiff);
	packsym = (Elf32_Word *)((char *)module->d_packsym + pdiff);

	relno = module->d_relsz / sizeof(Elf32_Rel);
	si = module->d_gotsym;

	/* Handle the local GOT entries and relocations */

	if (vdiff != 0) {

		/* If the object has moved from its linked base address, then
		 * we have to additional adjustments to make. First adjust
		 * all the local GOT entries. The first entry is used for
		 * the auto-loader routine, so we can skip over it
		 */

		got++;
		for (i = 1; i < module->d_locgotno; i++) {
			dprintf(("coff_dynamic_relocate: local GOT reloc: old=0x%x; new=0x%x\n",
			    got->g_index, got->g_index + vdiff));

#if PARANOID
			if (got->g_index < module->d_base_address) {
				ldr_msg("coff_dymamic_relocate: **** bad GOT entry (%d) for %s\n",
				    i, module->filename);
			}
#endif
			got->g_index += vdiff;
			got++;
		}

		/* Perform all local (non-symbolic) relocations, skipping the
		 * first entry, which is reserved.
		 */

		for (ri = 1; ri < relno && ELF32_R_SYM(rel_dyn[ri].r_info) < si; ri++) {
			paddr = rel_dyn[ri].r_offset + pdiff;

			dprintf(("coff_dynamic_relocate: local reloc at 0x%x; old=0x%x; new=0x%x\n",
			    paddr, *(long *)paddr, *(long *)paddr + vdiff));

#if PARANOID
			if (*(long *)paddr < module->d_base_address) {
				ldr_msg("coff_dynamic_relocate: **** bad relocatation at 0x%x for %s\n",
				    paddr, module->filename);
			}
#endif
			*(long *)paddr += vdiff;
		}
	} else {

		/* The object hasn't moved, so the local GOT entries are
		 * already correct. All we have to do is find the first
		 * relocation which corresponds to a global symbol.
		 * We find the entry using a binary search since the
		 * relocations are ordered by symbol index.
		 */

		ilow = 1;
		ihigh = relno - 1;

		while (ilow <= ihigh) {
		    i = (ilow + ihigh) / 2;

		    if (si < ELF32_R_SYM(rel_dyn[i].r_info)) {
			ihigh = i - 1;
		    } else if (si > ELF32_R_SYM(rel_dyn[i].r_info)) {
			ilow = i + 1;
		    } else { /* si == ELF32_R_SYM(rel_dyn[i].r_info) */

			/* Since there may be multiple relocations for the same
			 * symbol, we need to scan back to find the first
			 * relocation for a previous symbol.
			 */

			while (i-- && ELF32_R_SYM(rel_dyn[i].r_info) >= si)
			    ;

			ilow = i + 1;
			break;
		    }
		}
		ri = ilow;

#if PARANOID
		if ((relno <= 1 && ri != 1) || (relno > 1 && ri < relno)) {
			if (si > ELF32_R_SYM(rel_dyn[ri].r_info)) {
				ldr_msg("coff_dynamic_relocate: **** binary search botch (1) for %s\n",
				    module->filename);
			}
		}
		if (ri > 0 && relno > 1) {
			if (si <= ELF32_R_SYM(rel_dyn[ri - 1].r_info)) {
				ldr_msg("coff_dynamic_relocate: **** binary search botch (2) for %s\n",
				    module->filename);
			}
		}
#endif

		/* Point at the first non-local GOT entry */

		got += module->d_locgotno;
	}

	/* Handle all global symbols by updating their GOT entries and
	 * performing any relocations which reference the symbol.
	 * We perform these two tasks at the same time so that we can
	 * map .dynsym entries to their corresponding import symbol array
	 * elements. The nth entry in the import symbol array corresponds
	 * to the nth imported symbol in .dynsym. Unfortunately, the
	 * mapping is not a direct mapping since .dynsym may have local
	 * symbols interspersed with import symbols.
	 */

	ii = 0;
#if PACKSYM_FIXED
	packsym += si - module->d_unrefextno;
#else
	packsym += si;
#endif
	for (; si < module->d_symtabno; si++) {

		/* Update GOT entry according to symbol type */

		if (package[*packsym++].pkg_flags & PKGF_IMPORT) {

			/* If the symbol is imported then we must use
			 * the address as calculated by the format
			 * independent loader.
			 */

			if (ii >= nimports) {
				dprintf(("coff_dymamic_relocate: invalid import index\n"));
				return LDR_ERANGE;
			}
			if (!ldr_symval_is_abs(&imports[ii].ls_value)) {
				dprintf(("coff_dynamic_relocate: can't handle non-absolute symbols\n"));
				return LDR_ERANGE;
			}
			word = (Elf32_Word)ldr_symval_abs(&imports[ii].ls_value);

			dprintf(("coff_dynamic_relocate: import GOT reloc (%s): old=0x%x; new=0x%x\n",
			    imports[ii].ls_name, got->g_index, word));

			diff = word - got->g_index;

			got->g_index = word;
			got++;

			ii++;
		} else {

			/* If the symbol is local to the module then
			 * we only need adjust its entry by the amount
			 * the base address has moved.
			 */

			if (vdiff != 0) {
				dprintf(("coff_dynamic_relocate: global GOT reloc: old=0x%x; new=0x%x\n",
				    got->g_index, got->g_index + vdiff));

#if PARANOID
				if (got->g_index < module->d_base_address) {
					ldr_msg("coff_dymamic_relocate: **** bad GOT entry at 0x%x for %s\n",
					    got, module->filename);
				}
#endif
				got->g_index += vdiff;
			}
			got++;

			diff = vdiff;
		}

		/* Relocate all references to the current symbol */

		for (; ri < relno && ELF32_R_SYM(rel_dyn[ri].r_info) == si; ri++) {
			if (diff != 0) {
				paddr = rel_dyn[ri].r_offset + pdiff;

				dprintf(("coff_dynamic_relocate: global reloc at 0x%x; old=0x%x; new=0x%x\n",
				    paddr, *(long *)paddr, *(long *)paddr + diff));

#if PARANOID
#if 0
				/* Message removed because *paddr can be
				 * out of bounds of THIS module if this
				 * is an external reference to ANOTHER
				 * module.   -dda 10/30/91
				 */
				if (*(long *)paddr < module->d_base_address) {
					ldr_msg("coff_dynamic_relocate: **** bad relocatation at 0x%x for %s\n",
					    paddr, module->filename);
				}
#endif
#endif
				*(long *)paddr += diff;
			}
		}
	}

#if PARANOID

	/* Make sure we processed all the relocation entries */

	if ((relno <= 1 && ri != 1) || (relno > 1 && ri != relno)) {
		ldr_msg("loader: **** wrong number of relocs handled for %s\n",
		    module->filename);
		return LDR_ENOEXEC;
	}
#endif

	/* Correct any regions we made writable to allow relocations. */

	if (module->rdata_start) {
	    rc = ldr_mprotect((univ_t) module->rdata_start,
			      module->rdata_size,
			      LDR_PROT_READ|LDR_PROT_EXEC);
	    return(rc);
	}

	return LDR_SUCCESS;
}
