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
static char	sccsid[] = "@(#)$RCSfile: kls_coff_machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:46 $";
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

#define	STYP_MASK	0x1f

extern char *kls_coff_get_region_name(char *);
extern int kls_strnlen(char *, int);
extern void kls_strncpy(char *, char *, int);

int
kls_coff_machdep_recog(filehdr, aouthdr)
	struct filehdr *filehdr;
	struct aouthdr *aouthdr;
{
	/* check magic numbers and flags */
	if ((filehdr->f_magic != MIPSELMAGIC)
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
	pEXTR          extr, ex;
	char          *strings, *name;
	pHDRR          hdrr;
	size_t         size;
	int            export_count, rc, i;
	ldr_symval    *export_list, *sv;

	/* read symbolic information */
	if (!filehdr->f_symptr) {
		*tablep = (open_hashtab_t *)0;
		return(LDR_SUCCESS);
	}

	/* read symbolic header */
	if ((rc = ldr_lseek(fd, filehdr->f_symptr, SEEK_SET)) < 0)
		return(rc);
	if ((rc = ldr_malloc(sizeof(*hdrr), LDR_COFF_T,
	    (univ_t *)&hdrr)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)hdrr, sizeof(*hdrr))) < 0)
		return(rc);

	/* read external symbols */
	export_count = hdrr->iextMax;
	if (export_count <= 0) {
		*tablep = (open_hashtab_t *)0;
		return(LDR_SUCCESS);
	}
	if ((rc = ldr_lseek(fd, hdrr->cbExtOffset, SEEK_SET)) < 0)
		return(rc);
	size = export_count * sizeof(*extr);
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&extr)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)extr, size)) < 0)
		return(rc);

	/* read strings */
	if ((rc = ldr_lseek(fd, hdrr->cbSsExtOffset, SEEK_SET)) < 0)
		return(rc);
	size = hdrr->issExtMax;
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&strings)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)strings, size)) < 0)
		return(rc);

	/* build export list */
	size = export_count * sizeof(*export_list);
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&export_list)) != LDR_SUCCESS)
		return(rc);

	if ((rc = open_hash_create(export_count, (ldr_hash_p)hash_string,
	    (ldr_hash_compare_p)strcmp, (open_hash_flags_t)0, &table)) != LDR_SUCCESS)
		return(rc);

	for (i = 0; i < export_count; i++) {
		ex = &extr[i];
		sv = &export_list[i];

		sv->ls_tag = ldr_sym_abs;
		switch (ex->asym.sc) {

		    default:
			sv->ls_kind = ldr_sym_unknown;
			break;

		    case scText:
			sv->ls_kind = ldr_sym_function;
			break;

		    case scData:
		    case scBss:
		    case scSData:
		    case scSBss:
		    case scRData:
		    case scCommon:
			sv->ls_kind = ldr_sym_data;
			break;
		}
		sv->ls_abs = (univ_t)ex->asym.value;

		name = strings + ex->asym.iss;

		if ((rc = open_hash_insert(table, (const univ_t)name,
		    (univ_t *)&sv)) != LDR_SUCCESS)
			return(rc);
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
	if ((s_flags & STYP_TEXT) || (s_flags & STYP_RDATA))
		return(LDR_R|LDR_X);
	if ((s_flags & STYP_DATA) || (s_flags & STYP_SDATA)
	    || (s_flags & STYP_BSS) || (s_flags & STYP_SBSS))
		return(LDR_R|LDR_W|LDR_X);
	return(LDR_R|LDR_W|LDR_X);
}
