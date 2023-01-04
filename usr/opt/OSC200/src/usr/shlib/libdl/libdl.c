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
static char *rcsid = "@(#)$RCSfile: libdl.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/06/08 01:15:50 $";
#endif
/*
 * libdl.c
 * Emulations of libdl calls.  These call entry points in
 * rld.  In addition to dlopen(), etc.  we also implement
 * calls to implement all operation codes of _rld_new_interface().
 * The user is encouraged to use these instead of calling
 * _rld_new_interface() so that there will be no undefs in
 * the user program.
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE)
#pragma weak dlclose = __dlclose
#pragma weak dlerror = __dlerror
#pragma weak dlopen = __dlopen
#pragma weak dlsym = __dlsym
#endif
#endif
#include <dlfcn.h>
#include <rld_interface.h>

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ldr_rmutex;
extern int __multithreaded;
#define TS_LOCK(lock)           rec_mutex_lock(lock)
#define TS_UNLOCK(lock)         rec_mutex_unlock(lock)

#else
#define TS_LOCK(lock)           
#define TS_UNLOCK(lock)       
#endif

void *
dlopen(char *libname, int mode)
{
    void *rld_status;
    TS_LOCK(&_ldr_rmutex);
#ifdef _THREAD_SAFE
    if (__multithreaded) 
       mode = RTLD_NOW;           /* over-ride user specified mode    */
                                  /* force all symbols to be resolved */
                                  /* now. */
#endif
    rld_status = _rld_new_interface(_RLD_LDR_DLOPEN, libname, mode);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

void *
dlsym(void *handle, char *symname)
{
    void *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_LDR_DLSYM, handle, symname);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

char *
dlerror(void)
{
    char *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_LDR_DLERROR);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

int
dlclose(void *handle)
{
    int rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = (int) _rld_new_interface(_RLD_LDR_DLCLOSE, handle);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

void *
_rld_dlopen(char *libname, int mode)
{
    void *rld_status;
    TS_LOCK(&_ldr_rmutex);
#ifdef _THREAD_SAFE
    if (__multithreaded) 
       mode = RTLD_NOW;           /* over-ride user specified mode    */
                                  /* force all symbols to be resolved */
#endif
    rld_status = _rld_new_interface(_RLD_LDR_DLOPEN, libname, mode);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

void *
_rld_dlsym(void *handle, char *symname)
{
    void *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_LDR_DLSYM, handle, symname);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

char *
_rld_dlerror(void)
{
    char *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_LDR_DLERROR);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

int
_rld_dlclose(void *handle)
{
    int rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = (int) _rld_new_interface(_RLD_LDR_DLCLOSE, handle);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

Elf32_Addr
_rld_name_to_address(char *name)
{
    Elf32_Addr rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = (Elf32_Addr)_rld_new_interface(_RLD_NAME_TO_ADDR, name);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

char *
_rld_address_to_name(Elf32_Addr addr)
{
    char *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_ADDR_TO_NAME, addr);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

char *
_rld_first_pathname(void)
{
    char *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_FIRST_PATHNAME);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

char *
_rld_next_pathname(void)
{
    char *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_NEXT_PATHNAME);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}

char *
_rld_modify_list(Elf32_Word operation,
		 char *orig_pathname,
		 char *name)
{
    char *rld_status;
    TS_LOCK(&_ldr_rmutex);
    rld_status = _rld_new_interface(_RLD_MODIFY_LIST,
					  operation, orig_pathname, name);
    TS_UNLOCK(&_ldr_rmutex);
    return(rld_status);
}
