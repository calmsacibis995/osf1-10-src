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
static char	*sccsid = "@(#)$RCSfile: spin.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:13:06 $";
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

#ifndef	lint
#endif	not lint

/*
 * File: spin.c
 *
 * implmentation of spin locks. We rely on two machine dependent functions,
 * lock_try_set and lock_unset. spin_trylock() and spin_unlock are #defined
 * to be these functions but we implement spin_lock here. The aim is to be a 
 * little sociable here. We fast spin for a while (spin_limit times) and
 * then yield between each test of the lock after that to try and make sure
 * we don't hog the cpu completely.
 */

#include <pthread.h>
#include "internal.h"

/*
 * Local Definitions
 */
#define	SPIN_MAX	20

/*
 * Local Variables
 */
private	int	spin_limit = SPIN_MAX;

/*
 * Function:
 *	spin_lock
 *
 * Parameters:
 *	lock - a pointer to the lock word
 *
 * Description:
 *	Fast spin for a while and then yield spin if the lock isn't set
 *	quickly. You should not hold these locks for long as contention
 *	is not good.
 */
void
spin_lock(volatile int *lock)
{
	register int	i;

	if (lock_try_set(lock))
		return;

	for (i = 0; i < spin_limit; i++)
		if (lock_try_set(lock))
			return;

	while (!lock_try_set(lock))
		vp_yield();
}

/*
 * Function:
 *	spinlock_create
 *
 * Parameters:
 *	lock - pointer to the new lock
 *
 * Description:
 *	This function is needed by lib_lock.c to pass to other libraries
 *	using threads spin locking.
 */
void
spinlock_create(volatile int *lock)
{
	*lock = SPIN_LOCK_UNLOCKED;
}

/*
 * Function:
 *	spinlock_delete
 *
 * Parameters:
 *	lock - pointer to the lock to be deleted
 *
 * Description:
 *	This function is needed by lib_lock.c to pass to other libraries
 *	using threads spin locking.
 */
void
spinlock_delete(volatile int *lock)
{
	*lock = SPIN_LOCK_UNLOCKED;
}

/*
 * Function:
 *	spin_unlock
 *
 * Parameters:
 *	lock - a pointer to the lock word
 *
 * Description:
 *	This function is normally a macro to reduce one extra function call
 *	overhead. It is needed as a function for lib_lock as the address has
 *	to be taken to put in the table of locking functions.
 */
#undef spin_unlock
void
spin_unlock(volatile int *lock)
{
	lock_unset(lock);
}

/*
 * Function:
 *	spin_trylock
 *
 * Parameters:
 *	lock - a pointer to the lock word
 *
 * Return value:
 *	1	The lock was taken
 *	0	The lock was already locked
 *
 * Description:
 *	This function is normally a macro to reduce one extra function call
 *	overhead. It is needed as a function for lib_lock as the address has
 *	to be taken to put in the table of locking functions.
 */
#undef spin_trylock
int
spin_trylock(volatile int *lock)
{
	lock_try_set(lock);
}
