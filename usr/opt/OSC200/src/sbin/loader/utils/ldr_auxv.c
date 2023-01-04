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
static char	sccsid[] = "@(#)$RCSfile: ldr_auxv.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/12/07 16:20:16 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_auxv.c
 * loader auxiliary vector abstraction
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <sys/auxv.h>
#include <loader.h>
#include <loader/ldr_main_types.h>
#include "ldr_errno.h"
#include "ldr_auxv.h"

extern auxv_t *_auxv;
int _auxv_size;

static int exec_filename_found;
static int exec_loader_filename_found;
static int exec_loader_flags_found;
static int pagesz_found;
static int execfd_found;

static char *exec_filename;
static char *exec_loader_filename;
static int exec_loader_flags;
static int pagesz;
static int execfd;

int
ldr_auxv_init(void)
{
	auxv_t *avp;

	/* 
	 * Pass over auxv.  Cache useful entries.  Set those entries
	 * that shouldn't be passed to program to AT_IGNORE.  Compute
	 * the size of auxv, excluding the terminating AT_NULL entry.
	 * The size may eventually be of some use.
	 */
	for (avp = _auxv; avp->a_type != AT_NULL; avp++) {
		switch (avp->a_type) {

		case AT_EXEC_FILENAME:
			exec_filename_found = 1;
			exec_filename = (char *)avp->a_un.a_ptr;
			break;

		case AT_EXEC_LOADER_FILENAME:
			exec_loader_filename_found = 1;
			exec_loader_filename = (char *)avp->a_un.a_ptr;
#if 0
			avp->a_type = AT_IGNORE;
			avp->a_un.a_val = 0;
#endif
			break;

		case AT_EXEC_LOADER_FLAGS:
			exec_loader_flags_found = 1;
			exec_loader_flags = avp->a_un.a_val;
#if 0
			avp->a_type = AT_IGNORE;
			avp->a_un.a_val = 0;
#endif
			break;

		case AT_PAGESZ:
			pagesz_found = 1;
			pagesz = avp->a_un.a_val;
			break;

		case AT_EXECFD:
			execfd_found = 1;
			execfd = avp->a_un.a_val;
			break;
		}			
		_auxv_size++;
	}
}

int
ldr_auxv_get_exec_filename(filename)
	char **filename;
{
	if (exec_filename_found) {
		*filename = exec_filename;
		return(LDR_SUCCESS);
	}
	return(LDR_ESRCH);
}

int
ldr_auxv_get_exec_loader_filename(filename)
	char **filename;
{
	if (exec_loader_filename_found) {
		*filename = exec_loader_filename;
		return(LDR_SUCCESS);
	}
	return(LDR_ESRCH);
}

int
ldr_auxv_get_exec_loader_flags(flags)
	int *flags;
{
	if (exec_loader_flags_found) {
		*flags = exec_loader_flags;
		return(LDR_SUCCESS);
	}
	return(LDR_ESRCH);
}

int
ldr_auxv_get_pagesz(size)
	int *size;
{
	if (pagesz_found) {
		*size = pagesz;
		return(LDR_SUCCESS);
	}
	return(LDR_ESRCH);
}

int
ldr_auxv_get_execfd (int *fd)
{
	if (execfd_found)
		*fd = execfd;
	else
		*fd = LDR_FILE_NONE;
	return (LDR_SUCCESS);
}
