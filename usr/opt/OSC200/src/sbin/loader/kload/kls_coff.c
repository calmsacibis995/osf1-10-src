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
static char	*sccsid = "@(#)$RCSfile: kls_coff.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:29 $";
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
 * Implementation Notes
 *
 * The big differences between fully linked executables and
 * relocatables (.o files) seems to be F_RELFLG in filehdr.f_flags.
 * It would be nice to check F_EXEC, but ld -x -r on .o files seems to
 * set F_EXEC.  This is probably a bug.
 *
 * We could get the region by looking at the a.out header or by looking
 * at the section headers.  The below is configurable.  We chose to
 * use the section headers because it is fancier.
 *
 * We don't worry about freeing space before returning errors because we
 * don't think the various object file data structure waste too much
 * space.
 */

#include <sys/types.h>
#include <string.h>
#include <loader.h>
#include <unistd.h>

#include "coff_machdep.h"

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

typedef struct handle_data {
	open_hashtab_t  export_list;
	int             region_count;
	ldr_region_rec *region_list;
	ldr_entry_pt_t  entry_pt;
} handle_data_t;

ldr_package_rec kernel_package = {
	LDR_PACKAGE_VERSION,
	ldr_package
};
extern char *kls_default_package_name;

extern char *kls_coff_get_region_name(char *);
extern int kls_strnlen(char *, int);
extern void kls_strncpy(char *, char *, int);

static int get_exports(ldr_file_t, struct filehdr *, open_hashtab_t *);
static int get_regions(ldr_file_t, struct filehdr *, struct aouthdr *,
		       int *, ldr_region_rec **);
static ldr_prot_t get_region_prot(long);


int
kls_coff_recog(filename, fd, handle)
	const char        *filename;
	ldr_file_t         fd;
	ldr_module_handle *handle;
{
	ldr_region_rec *list;
	struct filehdr *filehdr;
	struct aouthdr *aouthdr;
	open_hashtab_t table;
	handle_data_t  *hdp;
	int            count, rc;

	/* read the filehdr */
	if ((rc = ldr_lseek(fd, 0, SEEK_SET)) < 0)
		return(LDR_ENOEXEC);
	if ((rc = ldr_malloc(sizeof(*filehdr), LDR_COFF_T,
	    (univ_t *)&filehdr)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)filehdr, sizeof(*filehdr))) < 0)
		return(LDR_ENOEXEC);

	/* read the aouthdr */
	if ((rc = ldr_malloc(sizeof(*aouthdr), LDR_COFF_T,
	    (univ_t *)&aouthdr)) != LDR_SUCCESS)
		return(LDR_ENOEXEC);
	if ((rc = ldr_read(fd, (char *)aouthdr, sizeof(*aouthdr))) < 0)
		return(LDR_ENOEXEC);

	/* check magic numbers and flags */
	if ((rc = kls_coff_machdep_recog(filehdr, aouthdr)) != LDR_SUCCESS)
		return(rc);

	/* get/build export list */
	if ((rc = kls_coff_machdep_get_exports(fd, filehdr, &table)) != LDR_SUCCESS)
		return(LDR_ENOEXEC);

	/* get/build region list */
	if ((rc = get_regions(fd, filehdr, aouthdr, &count, &list)) != LDR_SUCCESS)
		return(LDR_ENOEXEC);

	if ((rc = ldr_malloc(sizeof(*hdp), LDR_COFF_T,
	    (univ_t *)&hdp)) != LDR_SUCCESS)
		return(rc);

	hdp->export_list = table;
	hdp->region_count = count;
	hdp->region_list = list;
	hdp->entry_pt = (ldr_entry_pt_t)aouthdr->entry;

	(void)ldr_close(fd);

	*handle = (ldr_module_handle)hdp;
	return(LDR_SUCCESS);
}

int
kls_coff_map_regions(handle, allocsp, reg_count, regions)
	ldr_module_handle handle;
	ldr_region_allocs *allocsp;
	int *reg_count;
	ldr_region_rec **regions;
{
	handle_data_t		*hdp;

	hdp = (handle_data_t *)handle;
	*reg_count = hdp->region_count;
	*regions = hdp->region_list;
	return(LDR_SUCCESS);
}

int
kls_coff_lookup_export(handle, package, symbol)
	ldr_module_handle handle;
	ldr_package_rec  *package;
	ldr_symbol_rec   *symbol;
{
	handle_data_t		*hdp;
	open_hashtab_t		table;
	ldr_symval		*sym;
	int			rc;

	if (strcmp(package->lp_name, kernel_package.lp_name))
		return(LDR_ENOPKG);

	hdp = (handle_data_t *)handle;
	table = hdp->export_list;

	if ((rc = open_hash_lookup(table, (const univ_t)symbol->ls_name,
	    (univ_t *)&sym)) != LDR_SUCCESS)
		return(rc);

	symbol->ls_value = *sym;
	return(LDR_SUCCESS);
}

int
kls_coff_get_entry_pt(handle, entry_pt)
	ldr_module_handle handle;
	ldr_entry_pt_t *entry_pt;
{
	handle_data_t		*hdp;

	hdp = (handle_data_t *)handle;
	*entry_pt = hdp->entry_pt;
	return(LDR_SUCCESS);
}

int
kls_coff_get_export_pkgs(handle, count, packages)
	ldr_module_handle handle;
	int *count;
	ldr_package_rec **packages;
{
	kernel_package.lp_name = kls_default_package_name;

	*count = 1;
	*packages = &kernel_package;

	return(LDR_SUCCESS);
}

int
kls_coff_get_exports(handle, count, exports)
	ldr_module_handle handle;
	int *count;
	ldr_symbol_rec **exports;
{
	/* This should never be needed, so don't implement it */

	return(LDR_EINVAL);
}

static int
get_regions(fd, filehdr, aouthdr, countp, listp)
	ldr_file_t       fd;
	struct filehdr  *filehdr;
	struct aouthdr  *aouthdr;
	int             *countp;
	ldr_region_rec **listp;
{
	ldr_region_rec *region_list, *r;
	struct scnhdr  *scnhdr, *s;
	off_t           offset;
	size_t          size;
	int             i, rc, region_count;

#ifdef	USE_AOUTHDR
	region_count = aouthdr->bsize ? 3 : 2;
	if ((rc = ldr_regions_create(region_count, LDR_REGION_VERSION,
				     &region_list)) != LDR_SUCCESS)
		return(rc);

	r = region_list;

	r->lr_version = LDR_REGION_VERSION;
	r->lr_name = ".text";
	r->lr_prot = LDR_R|LDR_X;
	r->lr_vaddr = (univ_t)aouthdr->text_start;
	r->lr_mapaddr = (univ_t)-1;
	r->lr_size = (size_t)aouthdr->tsize;
	r->lr_flags = LRF_LOADED;

	r++;

	r->lr_version = LDR_REGION_VERSION;
	r->lr_name = ".data";
	r->lr_prot = LDR_R|LDR_W|LDR_X;
	r->lr_vaddr = (univ_t)aouthdr->data_start;
	r->lr_mapaddr = (univ_t)-1;
	r->lr_size = (size_t)aouthdr->dsize;
	r->lr_flags = LRF_LOADED;

	if (aouthdr->bsize) {
		r++;

		r->lr_version = LDR_REGION_VERSION;
		r->lr_name = ".bss";
		r->lr_prot = LDR_R|LDR_W|LDR_X;
		r->lr_vaddr = (univ_t)aouthdr->bss_start;
		r->lr_mapaddr = (univ_t)-1;
		r->lr_size = (size_t)aouthdr->bsize;
		r->lr_flags = LRF_LOADED;
	}


#else	/* USE_AOUTHDR */

	if (filehdr->f_nscns <= 0) {
		*countp = 0;
		*listp = (ldr_region_rec *)0;
		return(LDR_SUCCESS);
	}

	offset = sizeof(struct filehdr) + filehdr->f_opthdr;
	if ((rc = ldr_lseek(fd, offset, SEEK_SET)) < 0)
		return(rc);
	size = filehdr->f_nscns * sizeof(*scnhdr);
	if ((rc = ldr_malloc(size, LDR_COFF_T,
	    (univ_t *)&scnhdr)) != LDR_SUCCESS)
		return(rc);
	if ((rc = ldr_read(fd, (char *)scnhdr, size)) < 0)
		return(rc);

	region_count = filehdr->f_nscns;
	if ((rc = ldr_regions_create(region_count, LDR_REGION_VERSION,
				     &region_list)) != LDR_SUCCESS)
		return(rc);

	r = region_list;
	s = scnhdr;
	for (i = 0; i < filehdr->f_nscns; i++) {

		r->lr_version = LDR_REGION_VERSION;
		r->lr_name = kls_coff_get_region_name(s->s_name);
		r->lr_prot = kls_coff_machdep_get_region_prot(s->s_flags);
		r->lr_vaddr = (univ_t)s->s_vaddr;
		r->lr_mapaddr = (univ_t)-1;
		r->lr_size = (size_t)s->s_size;
		r->lr_flags = LRF_LOADED;

		r++;
		s++;
	}

#endif	/* USE_AOUTHDR */

	*countp = region_count;
	*listp = region_list;
	return(LDR_SUCCESS);
}

/*
 * Convert a Section Name to a Region Name
 */
char *
kls_coff_get_region_name(s_name)
	char *s_name;
{
	char *name;

	if (kls_strnlen(s_name, 8) == 8) {
		if (ldr_malloc(9, LDR_COFF_T, (univ_t *)&name) != LDR_SUCCESS)
			return((char *)0);
		kls_strncpy(name, s_name, 8);
		name[8] = '\0';
	} else
		name = s_name;
	return(name);
}

int
kls_strnlen(s, n)
	char *s;
{
	int i;

	for (i = 0; i < n; i++)
		if (!*s++)
			return(i);
	return(n);	
}

void
kls_strncpy(s1, s2, n)
	char *s1, *s2;
{
	int i;

	for (i = 0; i < n; i++)
		*s1++ = *s2++;
}
