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
static char	*sccsid = "@(#)$RCSfile: ldr_atexit.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:23:32 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_atexit.c
 * Implementations of the ldr_atexit() routine
 *
 * ldr_atexit() is called from exit() to perform loader cleanup.
 * Specifically, it calls all the termination functions for each
 * loader object module.
 *
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ldr_atexit = __ldr_atexit
#endif
#include <sys/types.h>
#include <sys/errno.h>
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ldr_rmutex;
#endif

int
ldr_atexit(void)

/* Run any per-module termination routines for this process' loader
 * context.  Intended to be called only from the exit() procedure
 * during normal process exit.  Returns 0 on success,
 * or a negative error status on error.
 */
{
	int			rc;
        
        TS_LOCK(&_ldr_rmutex); 
	rc = ldr_context_atexit(ldr_process_context);
	if (rc != 0)
		TS_SETERR(ldr_status_to_errno(rc));
        TS_UNLOCK(&_ldr_rmutex);
	return(rc);
}
