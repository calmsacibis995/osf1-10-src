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
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak msem_lock = __msem_lock
#endif
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include "ts_supp.h"


int
msem_lock(msemaphore *sem, int condition)
{
	/*
	 * Check to see if the semaphore has been removed or if the
	 * condition parameter is bad.
	 */
	if ((sem->msem_wanted == -1) ||
	    (!((condition == MSEM_IF_NOWAIT) || (condition == 0)))) {
                TS_SETERR(EINVAL);
		return(-1);
	}

	while (_msem_tas(&sem->msem_state) != 0) {

		if (condition == MSEM_IF_NOWAIT) {
			TS_SETERR(EAGAIN);
			return(-1);
		}

		if (msleep(sem) != 0) {
			return(-1);
		}

		/*
		 * we have to check to see if the semaphore is still valid
		 * as someone may have removed it while we were asleep
		 */
		if (sem->msem_wanted == -1) {
			TS_SETERR(EINVAL);
			return(-1);
		}
		
	}
	
	return(0);
}


