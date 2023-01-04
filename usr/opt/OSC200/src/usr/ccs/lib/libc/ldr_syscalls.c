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
static char	*sccsid = "@(#)$RCSfile: ldr_syscalls.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:24:17 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_syscalls.c
 * Implementations of the exported loader system calls
 *
 * This file contains the implementations of the loader system calls
 * exported to clients of the loader.  In general these routines do
 * not know the internal representations of loader data structures.
 * This file implements the routines which implicitly operate on the
 * current process; ldr_xcalls.c implements the cross-process routines.
 *
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ldr_entry = __ldr_entry
#pragma weak ldr_install = __ldr_install
#pragma weak ldr_lookup = __ldr_lookup
#pragma weak ldr_lookup_package = __ldr_lookup_package
#pragma weak ldr_remove = __ldr_remove
#pragma weak load = __load
#pragma weak unload = __unload
#endif
#include <sys/types.h>
#include <sys/errno.h>
#include "ts_supp.h"
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ldr_rmutex;
#endif

ldr_module_t
load(char *file_pathname, ldr_load_flags_t load_flags)

/* Load the specified module in the current process loader context,
 * and return its module ID.  Returns module ID on success, or 0
 * on failure with error number in errno.
 */
{
	ldr_module_t		mod_id;	/* module ID */
	int			rc;

        TS_LOCK(&_ldr_rmutex);
	rc = ldr_context_load(ldr_process_context, file_pathname, load_flags,
			      &mod_id);
	if (rc != 0) {
		TS_SETERR(ldr_status_to_errno(rc));
                TS_UNLOCK(&_ldr_rmutex);
		return(0);
	}
        TS_UNLOCK(&_ldr_rmutex);
	return(mod_id);
}


ldr_entry_pt_t
ldr_entry(ldr_module_t mod_id)

/* Look up and return the entry point of the loaded module named by the
 * specified module ID.  Returns the entry point on success, or NULL on error.
 */
{
	int			rc;
	ldr_entry_pt_t		entry;

        TS_LOCK(&_ldr_rmutex);
 
	rc = ldr_context_get_entry_pt(ldr_process_context, mod_id, &entry);
	if (rc != 0) {
		TS_SETERR(ldr_status_to_errno(rc));
                TS_UNLOCK(&_ldr_rmutex);
		return(NULL);
	}
        TS_UNLOCK(&_ldr_rmutex);
	return(entry);
}


int
unload(ldr_module_t mod_id)

/* Unload the loaded module named by the specified module ID.  Returns zero on
 * success, -1 on error.  NOTE: does no checking for possible references to
 * the module being unloaded.
 */
{
	int			rc;
        
        TS_LOCK(&_ldr_rmutex);
	rc = ldr_context_unload(ldr_process_context, mod_id);
	if (rc != 0) {
		TS_SETERR(ldr_status_to_errno(rc));
                TS_UNLOCK(&_ldr_rmutex);
		return(-1);
	}
        TS_UNLOCK(&_ldr_rmutex);
	return(0);
}


void *
ldr_lookup(ldr_module_t mod_id, char *symbol_name)

/* Look up the specified symbol in the symbols exported by the specified
 * module ID, and return the absolute value of the symbol.  On error,
 * return NULL with error status in errno.
 */
{
	int			rc;
	void			*value;

        TS_LOCK(&_ldr_rmutex);
	rc = ldr_context_lookup(ldr_process_context, mod_id, symbol_name,
				&value);
	if (rc != 0) {
		TS_SETERR(ldr_status_to_errno(rc));
                TS_UNLOCK(&_ldr_rmutex);
		return(NULL);
	}
        TS_UNLOCK(&_ldr_rmutex);
	return(value);
}


void *
ldr_lookup_package(char *package, char *symbol_name)

/* Look up the specified symbol in the symbols exported from the specified
 * package, and return the absolute value of the symbol.  On error,
 * return NULL with error status in errno.
 */
{
	int			rc;
	void			*value;

        TS_LOCK(&_ldr_rmutex);
	rc = ldr_context_lookup_package(ldr_process_context, package,
					symbol_name, &value);
	if (rc != 0) {
		TS_SETERR(ldr_status_to_errno(rc));
                TS_UNLOCK(&_ldr_rmutex);
		return(NULL);
	}
        TS_UNLOCK(&_ldr_rmutex);
	return(value);
}


int
ldr_install(const char *module_name)

/* Install the specified module in the current process' private known
 * package table.  The private KPT is inherited copy-on-write by
 * this process' children.  Returns 0 on success, or negative
 * error status on error.
 */
{
	int			rc;
       
        TS_LOCK(&_ldr_rmutex);
	rc = ldr_context_install(ldr_process_context, module_name);
	if (rc != 0)
		TS_SETERR(ldr_status_to_errno(rc));
        TS_UNLOCK(&_ldr_rmutex);
	return(rc);
}


int
ldr_remove(const char *module_name)

/* Remove the specified module from the current process' private known
 * package table.  Returns 0 on success, or negative
 * error status on error.
 */
{
	int			rc;

        TS_LOCK(&_ldr_rmutex);
	rc = ldr_context_remove(ldr_process_context, module_name);
	if (rc != 0)
	      TS_SETERR(ldr_status_to_errno(rc));
        TS_UNLOCK(&_ldr_rmutex);
	return(rc);
}
