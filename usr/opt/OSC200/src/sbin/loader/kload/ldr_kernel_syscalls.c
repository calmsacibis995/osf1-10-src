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
static char	*sccsid = "@(#)$RCSfile: ldr_kernel_syscalls.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:16:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * ldr_kernel_syscalls.c
 *
 * OSF/1 Release 1.0
 */

/*
 * ldr_kernel_syscalls.c
 *
 * This file implements the kernel versions of all of the loader
 * system calls.  In other words, the functions implemented below, are
 * the ones that make the ldr_context*() calls, specifying
 * ldr_kernel_context.  The implementation of these functions is
 * fairly straightforward and the code below should provide an
 * adequate description. 
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <loader.h>
#include <mach.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>
#include <loader/kloadsrv.h>

#include "ldr_types.h"
#include "ldr_lock.h"
#include "ldr_errno.h"

#include "kernel_loader.h"
#include "ldr_kernel_main.h"
#include "ldr_macro_help.h"

/* #define	TEST	1 */

#define	dprintf(x) \
	MACRO_BEGIN \
		if (kls_debug_level) \
			(void) printf x ; \
	MACRO_END

#ifdef	TEST
int next_mod_id;
int next_entry_pt;
int next_symbol_addr;
#endif

int kls_debug_level;

static int write_regions(ldr_module_t module);
static int pagesize;

/* Return a pointer to any retained error messages */
static char *
errmsgs()
{
	static char	buffer[1024];

	ldr_sprintf(buffer, sizeof(buffer), "%B");
	return(buffer);
}


int
kernel_load(file_pathname, load_flags, module)
	char            *file_pathname;
	ldr_load_flags_t load_flags;
	ldr_module_t     *module;
{
	ldr_module_t     my_module;
	ldr_load_flags_t my_flags;
	int              rc;

#ifdef	TEST
	*module = (ldr_module_t)++next_mod_id;
	rc = LDR_SUCCESS;
#else
	my_flags = load_flags | LDR_NOINIT | LDR_NOPREXIST;
	rc = ldr_context_load(ldr_kernel_context, file_pathname,
	    my_flags, &my_module);
	if (rc == LDR_SUCCESS) {
		if ((rc = write_regions(my_module)) == LDR_SUCCESS)
			*module = my_module;
		else
			(void)kernel_unload(my_module);
	} else if (rc == LDR_EEXIST) {
			*module = my_module;
			if (!(load_flags & LDR_NOPREXIST))
				rc = LDR_SUCCESS;
	}
#endif
	dprintf(("%s: kernel_load(\"%s\", %#x, module) = %d", KLS_SERVER_NAME,
		file_pathname, load_flags, rc));
	if (rc == LDR_SUCCESS)
		dprintf(("; *module = %d\n", (int)*module));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

int
kernel_entry(module, entry)
	ldr_module_t    module;
	ldr_entry_pt_t *entry;
{
	int rc;

#ifdef	TEST
	next_entry_pt += 0x1000;
	*entry = (ldr_entry_pt_t)next_entry_pt;
	rc = LDR_SUCCESS;
#else
	rc = ldr_context_get_entry_pt(ldr_kernel_context, module, entry);
#endif
	dprintf(("%s: kernel_entry(%d, entry) = %d", KLS_SERVER_NAME,
		module, rc));
	if (rc == LDR_SUCCESS)
		dprintf(("; *entry = %#x\n", (int)*entry));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

int
kernel_unload(module)
	ldr_module_t module;
{
	int rc;

#ifdef	TEST
	rc = LDR_SUCCESS;
#else
	rc = ldr_context_unload(ldr_kernel_context, module);
#endif
	dprintf(("%s: kernel_unload(%d) = %d\n", KLS_SERVER_NAME, module, rc));
	return(rc);
}

int
kernel_lookup(module, symbol_name, symbol_addr)
	ldr_module_t module;
	char *symbol_name;
	void **symbol_addr;
{
	int rc;

#ifdef	TEST
	next_symbol_addr += 0x1000;
	*((int *)symbol_addr) = next_symbol_addr;
	rc = LDR_SUCCESS;
#else
	rc = ldr_context_lookup(ldr_kernel_context, module, symbol_name,
		symbol_addr);
#endif
	dprintf(("%s: kernel_lookup(%d, \"%s\", symbol_addr) = %d",
		KLS_SERVER_NAME, module, symbol_name, rc)); 
	if (rc == LDR_SUCCESS)
		dprintf(("; *symbol_addr = %#x\n", (int)*symbol_addr));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

int
kernel_lookup_package(package_name, symbol_name, symbol_addr)
	char *package_name;
	char *symbol_name;
	void **symbol_addr;
{
	int rc;

#ifdef	TEST
	next_symbol_addr += 0x1000;
	*((int *)symbol_addr) = next_symbol_addr;
	rc = LDR_SUCCESS;
#else
	rc = ldr_context_lookup_package(ldr_kernel_context,
		package_name, symbol_name, symbol_addr);
#endif
	dprintf(("%s: kernel_lookup_package(\"%s\", \"%s\", symbol_addr) = %d",
		KLS_SERVER_NAME, package_name, symbol_name, rc)); 
	if (rc == LDR_SUCCESS)
		dprintf(("; *symbol_addr = %#x\n", (int)*symbol_addr));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

int
kernel_next_module(module)
	ldr_module_t *module;
{
	int rc;

	dprintf(("%s: kernel_next_module(*module == %d)", KLS_SERVER_NAME,
		(int)*module));
#ifdef	TEST
	*module = (ldr_module_t)++next_mod_id;
	rc = LDR_SUCCESS;
#else
	rc = ldr_context_next_module(ldr_kernel_context, module);
#endif
	dprintf((" = %d", rc));
	if (rc == LDR_SUCCESS)
		dprintf(("; *module = %d\n", (int)*module));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

int
kernel_inq_module(module, info, info_size, ret_size)
	ldr_module_t module;
	ldr_module_info_t *info;
	size_t info_size;
	size_t *ret_size;
{
	int rc;

#ifdef	TEST
	rc = LDR_EINVAL;
#else
	rc = ldr_context_inq_module(ldr_kernel_context, module,
		info, info_size, ret_size);
#endif
	dprintf(("%s: kernel_inq_module(%d, info, %d, ret_size) = %d",
		KLS_SERVER_NAME, module, info_size, rc)); 
	if (rc == LDR_SUCCESS)
		dprintf(("; *ret_size = %d\n", (int)*ret_size));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

int
kernel_inq_region(module, region, info, info_size, ret_size)
	ldr_module_t module;
	ldr_region_t region;
	ldr_region_info_t *info;
	size_t info_size;
	size_t *ret_size;
{
	int rc;

#ifdef	TEST
	rc = LDR_EINVAL;
#else
	rc = ldr_context_inq_region(ldr_kernel_context, module,
		region, info, info_size, ret_size);
#endif
	dprintf(("%s: kernel_inq_region(%d, %d, info, %d, ret_size) = %d",
		KLS_SERVER_NAME, module, region, info_size, rc)); 
	if (rc == LDR_SUCCESS)
		dprintf(("; *ret_size = %d\n", (int)*ret_size));
	else
		dprintf(("\n%s", errmsgs()));
	return(rc);
}

static int
write_regions(module)
	ldr_module_t		module;
{
	ldr_module_info_t	ldr_module_info, *lmip;
	ldr_region_info_t	ldr_region_info, *lrip;
	ldr_region_t		region;
	size_t			ret_size;
	vm_address_t		vm_address;
	pointer_t		data;
	int			data_count;
	vm_prot_t		vm_prot;
	vm_size_t		vm_size;
	int			rc;

	lmip = &ldr_module_info;
	rc = kernel_inq_module(module, lmip, sizeof(*lmip), &ret_size);
	if (rc != LDR_SUCCESS)
		return(rc);

	lrip = &ldr_region_info;
	for (region = 0; region < ldr_module_info.lmi_nregion; region++) {

		rc = kernel_inq_region(module, region, lrip,
			sizeof(*lrip), &ret_size);
		if (rc != LDR_SUCCESS)
			return(rc);

		vm_address = (vm_address_t)lrip->lri_vaddr;
		data = (pointer_t)lrip->lri_mapaddr;
		if (!pagesize)
			pagesize = getpagesize();
		data_count = (int)roundup((int)lrip->lri_size, pagesize);
		rc = kls_vm_write(vm_address, data, data_count);
		if (rc)
			return(convert_kls_vm_status_to_ldr_status(rc));

		vm_size = (vm_size_t)data_count;
		vm_prot = convert_ldr_prot_to_vm_prot(lrip->lri_prot);
		rc = kls_vm_protect(vm_address, vm_size, FALSE, vm_prot);
		if (rc)
			return(convert_kls_vm_status_to_ldr_status(rc));
	}

	return(LDR_SUCCESS);
}
