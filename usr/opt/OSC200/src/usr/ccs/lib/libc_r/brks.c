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
static char *rcsid = "@(#)$RCSfile: brks.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/06/08 02:08:15 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_SHARED_LIBRARIES) && defined(_THREAD_SAFE)
#pragma weak brk = __brk
#pragma weak sbrk = __sbrk
#endif
#endif
#ifndef _SHARED_LIBRARIES
/*
 * Thread safe versions of brk() and sbrk() for the static library
 */

#include <sys/types.h>
#include "rec_mutex.h"

extern struct rec_mutex _brk_rmutex;
#ifdef brk
#undef brk
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define brk __brk
#endif

extern char *_unlocked_sbrk();

char * 
brk(void *addr )
{
       int rc;

       rec_mutex_lock(&_brk_rmutex);
       rc = _unlocked_brk(addr);
       rec_mutex_unlock(&_brk_rmutex);
       return((char *)rc);
}

#ifdef sbrk
#undef sbrk
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define sbrk __sbrk
#endif

char *
sbrk(ssize_t incr)
{ 
       char *obrk;

       rec_mutex_lock(&_brk_rmutex);
       obrk = _unlocked_sbrk(incr);	
       rec_mutex_unlock(&_brk_rmutex);
       return(obrk);
}
#endif /* !SHARED_LIBRARIES */

