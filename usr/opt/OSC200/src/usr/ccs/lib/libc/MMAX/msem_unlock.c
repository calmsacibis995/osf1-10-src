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
static char	*sccsid = "@(#)$RCSfile: msem_unlock.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:04:02 $";
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

#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>

int
msem_unlock(msemaphore *sem, int condition)
{
	
	if ((sem->msem_wanted == -1) ||
	    (!((condition == MSEM_IF_WAITERS) || (condition ==0)))) {
		errno = EINVAL;
		return(-1);
	}

	if ((condition == MSEM_IF_WAITERS) && (sem->msem_wanted == 0)) {
		errno = EAGAIN;
		return(-1);
	}
	sem->msem_state = 0;		/* Clear the lock. */
	
	if (sem->msem_wanted != 0) {	/* See if anyone is waiting. */
		return(mwakeup(sem));
	}
		
	return(0);
}
