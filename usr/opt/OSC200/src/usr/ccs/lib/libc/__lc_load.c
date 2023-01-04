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
static char *rcsid = "@(#)$RCSfile: __lc_load.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:21:15 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __lc_load
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * com/lib/c/loc/__lc_load.c, bos320 2/28/91 08:21:59
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <sys/types.h>
#include <dlfcn.h>

/*
*  FUNCTION: __lc_load
*
*  DESCIPTION:
*  The function loads an object file from 'path'.  If the object file is
*  successfully loaded, the routine invokes the return address.  The
*  pointer returned is assumed to be an LC_OBJECT.  If the routine
*  cannot load 'path', the function invokes the 'instantiate' method if
*  this method is not null.
*/
void * __lc_load(const char *path, void *(*instantiate)())
{
    /* void * (*p)();  */
    _LC_object_t * (*p)();
    void *handle;
    _LC_object_t * q;

    /* load specified object */
    /* p = load(path, 0, ""); */
    handle = dlopen((char *)path, RTLD_LAZY);
    if (handle == NULL)
        return NULL;
    p = (_LC_object_t * (*)()) dlsym(handle, "instantiate");

    /*
      If load() succeeded,
         execute the method pointer which was returned.
         return the return value of the method.
      else if an instantiation method was provided,
         execute the instantiation method, and
         return the return value of the instantiation method.
      else
         return 0.
    */
    if (p != NULL) {

        /* invoke object instantiation method */
        q = (_LC_object_t *)(*p)(path);

        /* verify that what was returned was actually an object */
        if (q->magic == _LC_MAGIC)
            return q;

    }

    /*
     * At this point, either the object couldn't be loaded, or the
     * instantiation method did not return a valid object.
     */

    /* invoke the instantiate method if it is not null */
    if (instantiate != NULL) {

        /* invoke the caller supplied instantiate method */
        q = (*instantiate)(path);

        /* verify that it returned a valid object */
        if (q->magic == _LC_MAGIC)
            return q;
    }

    /* could not load specified object */
    return NULL;
}
