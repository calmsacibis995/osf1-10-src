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
static char	*sccsid = "@(#)$RCSfile: kls_coff_machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:39 $";
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

/*
 * MMAX/kls_coff_machdep.c
 *
 * This is the format dependent manager for Multimax COFF, used by the
 * Kernel Load Server (KLS), to get the initial export list of the
 * kernel.  The following functions are exported by this file.
 *
 *  kls_coff_machdep_recog()
 *	This function merely checks the file header and/or a.out
 *	header magic numbers and checks if the file is a Multimax COFF
 *	file.
 *
 *  kls_coff_machdep_get_exports()
 *	This function builds the export list for the kernel.  It
 *	exports all global symbols found in the Multimax COFF object
 *	file.  We exclude all symbols of class C_FILE, all symbols
 *	whose names are ".text", ".data", or ".bss" and all auxiliary
 *	entries.
 *
 *  kls_coff_machdep_get_region_prot()
 *	This function, when given the flags from a Multimax COFF
 *	section header, returns the appropriate corresponding loader
 *	protection.
 *
 * When any failure occurs, this manager does report it, but it does
 * not free any space that it is allocated.  Since this manager is only
 * used to get the initial kernel export information, this behavior is
 * acceptable.
 */

#include <sys/types.h>
#include <string.h>
#include <loader.h>
#include <unistd.h>
#include <a.out.h>

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

#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0410		/* read-only text */
#define	ZMAGIC	0413		/* demand load format */

#define	STYP_MASK	0x1f

extern char *kls_coff_get_region_name(char *);
extern int kls_strnlen(char *, int);
extern void kls_strncpy(char *, char *, int);

static char *get_symbol_name(SYMENT *, char *);

int
kls_coff_machdep_recog(filehdr, aouthdr)
	struct filehdr *filehdr;
	struct aouthdr *aouthdr;
{
	/* check magic numbers and flags */
	if ((filehdr->f_magic != NS32GMAGIC)
	    || (!(filehdr->f_flags & F_EXEC))
	    || (!(filehdr->f_flags & F_RELFLG)))
		return(LDR_ENOEXEC);
	if (!((aouthdr->magic == OMAGIC) || (aouthdr->magic == NMAGIC)
	    || (aouthdr->magic == ZMAGIC)))
		return(LDR_ENOEXEC);
	return(LDR_SUCCESS);
}

int
kls_coff_machdep_get_exports(fd, filehdr, tablep)
	ldr_file_t      fd;
	struct filehdr *filehdr;
	open_hashtab_t *tablep;
{
	open_hashtab_t table;
	SYMENT        *syment, *syp;
	char          *strings, *name;
	size_t         size;
	int            export_count, offset, rc, i;
	ldr_symval    *export_list, *svp;

	/* read symbolic information */
	if (!filehdr->f_nsyms) {
		*tablep = (open_hashtab_t *)0;
		return(LDR_SUCCESS);
	}

	/* read strings */
	offset = filehdr->f_symptr + (filehdr->f_nsyms * sizeof(SYMENT));
	if ((rc = ldr_lseek(fd, offset, SEEK_SET)) < 0)
		return(rc);
	if ((rc = ldr_read(fd, (char *)&size, sizeof(size))) < 0)
		return(rc);
	if (!size) {
		*tablep = (open_hashtab_t *)0;
		return(LDR_SUCCESS);
	}
	if ((rc = ldr_lseek(fd, offset, SEEK_SET)) < 0)
		return(rc);
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&strings)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)strings, size)) < 0)
		return(rc);

	/* read symbols */
	offset = filehdr->f_symptr;
	if ((rc = ldr_lseek(fd, offset, SEEK_SET)) < 0)
		return(rc);
	size = filehdr->f_nsyms * sizeof(SYMENT);
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&syment)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)syment, size)) < 0)
		return(rc);

	/* count up exports */
	export_count = 0;
	for (i = 0, syp = syment; i < filehdr->f_nsyms; i++, syp++) {
		if ((syp->n_sclass != C_FILE)
		    && strcmp(".text", syp->n_name)
		    && strcmp(".data", syp->n_name)
		    && strcmp(".bss",  syp->n_name))
			export_count++;
		if (syp->n_numaux) {
			i += syp->n_numaux;
			syp += syp->n_numaux;
		}
	}

	/* build export list */
	export_count = filehdr->f_nsyms;
	size = export_count * sizeof(*export_list);
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&export_list)) != LDR_SUCCESS)
		return(rc);

	if ((rc = open_hash_create(export_count, (ldr_hash_p)hash_string,
	    (ldr_hash_compare_p)strcmp, (open_hash_flags_t)0, &table)) != LDR_SUCCESS)
		return(rc);

	svp = export_list;
	for (i = 0, syp = syment; i < filehdr->f_nsyms; i++, syp++) {
		if ((syp->n_sclass != C_FILE)
		    && strcmp(".text", syp->n_name)
		    && strcmp(".data", syp->n_name)
		    && strcmp(".bss",  syp->n_name)) {

			svp->ls_tag = ldr_sym_abs;
			switch (syp->n_scnum) {

			    default:
				svp->ls_kind = ldr_sym_unknown;
				break;

			    case 1: /* .text */
				svp->ls_kind = ldr_sym_function;
				break;

			    case 2: /* .data */
			    case 3: /* .bss */
				svp->ls_kind = ldr_sym_data;
				break;
			}
			svp->ls_abs = (univ_t)syp->n_value;

			name = get_symbol_name(syp, strings);

			if ((rc = open_hash_insert(table, (const univ_t)name,
			    (univ_t)svp)) != LDR_SUCCESS)
				return(rc);

			svp++;
		}
		if (syp->n_numaux) {
			i += syp->n_numaux;
			syp += syp->n_numaux;
		}
	}

	*tablep = table;
	return(LDR_SUCCESS);
}

ldr_prot_t
kls_coff_machdep_get_region_prot(s_flags)
	long s_flags;
{
	if ((s_flags & STYP_MASK) != STYP_REG)
		return(LDR_R|LDR_W|LDR_X);
	if (s_flags & STYP_TEXT)
		return(LDR_R|LDR_X);
	if ((s_flags & STYP_DATA) || (s_flags & STYP_BSS))
		return(LDR_R|LDR_W|LDR_X);
	return(LDR_R|LDR_W|LDR_X);
}

/*
 * get_symbol_name() should be machine independent but MIPS COFF
 * doesn't define SYMENT.
 */
static char *
get_symbol_name(sp, strings)
	SYMENT *sp;
	char   *strings;
{
	char *name;

	if (sp->n_zeroes) {
		if (kls_strnlen(sp->n_name, 8) == 8) {
			if (ldr_malloc(9, LDR_COFF_T, (univ_t *)&name) != LDR_SUCCESS)
				return((char *)0);
			kls_strncpy(name, sp->n_name, 8);
			name[8] = '\0';
		} else
			name = sp->n_name;
	} else
		name = strings + sp->n_offset;
	return(name);
}
