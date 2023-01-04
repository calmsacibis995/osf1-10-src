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
static char	*sccsid = "@(#)$RCSfile: msem_remove.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:03:56 $";
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
#include <sys/mman.h>
#include <errno.h>

int msem_remove(msemaphore *sem)
{
	int	wanted;

	if (sem->msem_wanted == -1) {
		errno = EINVAL;
		return(-1);
	}
	/*
	 * The wanted count is also used to show if the semaphore is removed.
	 * Save away whether there are waiters. We must mark the semaphore
	 * as removed before we wake up the waiters in case their priority is
	 * higher than ours
	 */
	wanted = sem->msem_wanted;
	sem->msem_wanted = -1;	
	if (wanted > 0)
		mwakeup(sem);

	sem->msem_state = -1;
	return(0);
}
