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
static char *rcsid = "@(#)$RCSfile: libdl_dummy.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/07 23:25:13 $";
#endif
/*
 * libdl_dummy.c
 * This source file provides dummy versions of the shared-library loader's API.
 * These dummy routines are built into the non-shared libc.a, so that 
 * which are built both shared non-shared do not require conditional 
 * compilation for the loader calls.
 *
 * These routines will always return a failure status of NULL,
 * except for dlerror which will return the error string indicating that
 * the shared-library loader interface is not supported for non-shared
 * executables.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE)
#pragma weak dlclose = __dlclose
#pragma weak dlerror = __dlerror
#pragma weak dlopen = __dlopen
#pragma weak dlsym = __dlsym
#endif
#endif
#include <dlfcn.h>
#include <rld_interface.h>

/* These weak pragmas will provide protection for call_shared programs.
 * If libc.a is incorporated into a shared-library which uses the
 * loader API, these dummy routines could preempt their real counterparts.
 * Defining the symbols as WEAK will allow the STRONG counterparts to
 * preempt them.
 */

#ifndef _NAME_SPACE_WEAK_STRONG
#pragma weak dlopen
#pragma weak dlsym
#pragma weak dlerror
#pragma weak dlclose
#else
#pragma weak __dlopen
#pragma weak __dlsym
#pragma weak __dlerror
#pragma weak __dlclose
#endif
#pragma weak _rld_dlopen
#pragma weak _rld_dlsym
#pragma weak _rld_dlerror
#pragma weak _rld_dlclose
#pragma weak _rld_name_to_address
#pragma weak _rld_address_to_name
#pragma weak _rld_first_pathname
#pragma weak _rld_next_pathname
#pragma weak _rld_modify_list

static char *nonshared_errmsg = "The shared-library loader API is not supported for non_shared executables.";

void *
dlopen(char *libname, int mode)
{
    return(0L);
}

void *
dlsym(void *handle, char *symname)
{
    return(0L);
}

char *
dlerror(void)
{
    return(nonshared_errmsg);
}

int
dlclose(void *handle)
{
    return(0L);
}

void *
_rld_dlopen(char *libname, int mode)
{
    return(0L);
}

void *
_rld_dlsym(void *handle, char *symname)
{
    return(0L);
}

char *
_rld_dlerror(void)
{
    return(nonshared_errmsg);
}

int
_rld_dlclose(void *handle)
{
    return(0L);
}

Elf32_Addr
_rld_name_to_address(char *name)
{
    return(0L);
}

char *
_rld_address_to_name(Elf32_Addr addr)
{
    return(0L);
}

char *
_rld_first_pathname(void)
{
    return(0L);
}

char *
_rld_next_pathname(void)
{
    return(0L);
}

char *
_rld_modify_list(Elf32_Word operation,
		 char *orig_pathname,
		 char *name)
{
    return(0L);
}
