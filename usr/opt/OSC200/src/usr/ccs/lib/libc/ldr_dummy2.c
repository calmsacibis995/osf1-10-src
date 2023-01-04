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
static char	*sccsid = "@(#)$RCSfile: ldr_dummy2.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/06/23 21:23:17 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* ldr_dummy2.c
 * Dummies of the exported loader system call ldr_atexit(), for static libc
 *
 * This file contains dummy versions of the exported loader system
 * calls.  It is intended to be included in a statically-linked
 * version of the C library, from which the loader functions are
 * not available.
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_SHARED_LIBRARIES)
#pragma weak ldr_context_atexit = __ldr_context_atexit
#endif
#endif
#include <sys/types.h>
#include <errno.h>
#include "ts_supp.h"
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>


#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ldr_rmutex;
#endif

ldr_context_t	ldr_process_context = NULL;

int
ldr_context_atexit(ldr_context_t ctxt)

/* Run any per-module termination routines for this process' loader
 * context.  Intended to be called only from the exit() procedure
 * during normal process exit.  Returns LDR_SUCCESS on success,
 * or a negative error status on error.
 */
{
#if defined(__mips__) || defined (__alpha)
	/*
	 * This code calls termination routines that were established in a 
	 * statically linked program.  This code depends upon MIPS defined
	 * conventions for the location of termination routines.
	 * __fstart() is a linker defined symbol that is zero if no termination
	 * routines, and is the address of a routine that calls all the termination
	 * routines if there are termination routines.
	 */
        extern int __fstart();
	int (*fini)();

        TS_LOCK(&_ldr_rmutex);
	fini = __fstart;
	if (fini)
	    (* fini)();
#endif /* __mips__ || __alpha */

        TS_UNLOCK(&_ldr_rmutex);
	return(0);
}
