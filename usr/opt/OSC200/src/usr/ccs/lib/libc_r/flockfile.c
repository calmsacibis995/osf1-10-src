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
static char	*sccsid = "@(#)$RCSfile: flockfile.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/08 00:30:21 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * This file contains the POSIX (P1003.4a) functions for locking and unlocking
 * of stdio file descriptors.
 *
 * OSF/1 Release 1.0
 */

#ifndef lint
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak flockfile = __flockfile
#pragma weak funlockfile = __funlockfile
#endif
#endif
#include <stdio.h>
#include "stdio_lock.h"

#undef flockfile
#undef funlockfile
#ifdef _NAME_SPACE_WEAK_STRONG
#define flockfile __flockfile
#define funlockfile __funlockfile
#endif

void
flockfile(FILE *iop)
{
	if (iop->_lock != (void *) NULL) {
		rec_mutex_lock(iop->_lock);
	}
}

void
funlockfile(FILE *iop)
{
	if (iop->_lock != (void *) NULL) {
		rec_mutex_unlock(iop->_lock);
	}
}
