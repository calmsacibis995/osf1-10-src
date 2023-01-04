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
static char *rcsid = "@(#)$RCSfile: init_shared_libraries.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/11/03 18:27:47 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef _SHARED_LIBRARIES 
/*
 * The following routine should be called by any thread
 * library which is built shareable. The loader is currently
 * not re-entrant so all calls to the loader must be done serially
 * The _ldr_rmutex is used to ensure that all calls to the loader api are
 * serial.  
 *
 * The intent of this routine is to force all symbols external to an application
 * to be statically resolved.  It also forces any symbols which
 * aren't currently resolved to get resolved. This is done so that
 * loader will not attempt resolve symbols in an application asynchronous
 * to the application.  This routine should be called prior to any threads
 * being created.  
 *
 */

#include <stdio.h>
#include <dlfcn.h>

int __multithreaded = 0;
__init_shared_libs_for_threads()
{
    void *handle;

    /* Force immediate symbol resolution
     * for shlibs already loaded
     */
	if(!__multithreaded) {
    handle = dlopen(NULL, RTLD_NOW);
    if (handle == NULL) {
        /* Possible unresolved 
         * symbols in linked libraries 
         * Since the loader is not re-entrant print an error
         * exit.
         */
        fprintf(stderr,"thread_init: loader error: %s\n", dlerror());
        exit(1);
    }
    dlclose(handle);
    __multithreaded = 1;      /* let dlopen know we are multi threaded */
	}
}
#endif
