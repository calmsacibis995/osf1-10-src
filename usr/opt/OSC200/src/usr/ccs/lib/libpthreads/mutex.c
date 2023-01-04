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
static char	*sccsid = "@(#)$RCSfile: mutex.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:48 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	lint
#endif	not lint

/*
 * File: mutex.c
 *
 * Support for mutexes and their attributes. There are no attributes defined
 * for mutexes currently. Mutexes are implemented using spin locks. The lock
 * is spun on for max_spin_limit attempts and then the thread yields between
 * each subsequent attempt at trying to get the lock.
 */

#include <pthread.h>
#include "internal.h"
#include <errno.h>

/*
 * Local Definitions
 */
#define	MUTEX_SPIN_MAX	20

/*
 * Local Variables
 */
private int	mutex_spin_limit;

/*
 * Global Variables
 */
pthread_mutexattr_t	pthread_mutexattr_default;

/*
 * Function:
 *	pthread_mutexattr_startup
 *
 * Description:
 *	Initialize mutex attributes. This function creates the default
 *	attribute structure.
 */
private void
pthread_mutexattr_startup()
{
	pthread_mutexattr_create(&pthread_mutexattr_default);
	/* No attributes to initialize */
}

/*
 * Function:
 *	pthread_mutexattr_create
 *
 * Parameters:
 *	attr - pointer to the newly created attribute structure
 *
 * Return value:
 *	0	Success
 *	-1	if the pointer passed is and invalid pointer (EINVAL)
 *
 * Description:
 *	The pthread_mutexattr_t is the attribute structure and this function
 *	marks it as being initialized. There is no real initialization to be
 *	done as there are no attributes.
 */
int
pthread_mutexattr_create(pthread_mutexattr_t *attr)
{
	if (attr == NULL) {
		set_errno(EINVAL);
		return(-1);
	}
	*attr = (pthread_mutexattr_t)malloc(sizeof(struct pthread_mutexattr));
	if (*attr == NO_MUTEX_ATTRIBUTE) {
		set_errno(ENOMEM);
		return(-1);
	}
	(*attr)->flags = MUTEXATTR_VALID;
	return(0);
}

/*
 * Function:
 *	pthread_mutexattr_delete
 *
 * Parameters:
 *	attr - pointer to the attribute structure to be deleted
 *
 * Return value:
 *	0	Success
 *	-1	if the pointer passed is and invalid pointer (EINVAL)
 *		if the structure had not be previously initialized (EINVAL)
 *
 * Description:
 *	The attribute structure is simply marked as being no longer
 *	valid after the appropriate checks have been made.
 */
int
pthread_mutexattr_delete(pthread_mutexattr_t *attr)
{
	if ((attr == NULL) || (*attr == NO_MUTEX_ATTRIBUTE) ||
	    !((*attr)->flags&MUTEXATTR_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}
	(*attr)->flags &= ~MUTEXATTR_VALID;
	free(*attr);
	*attr = NO_MUTEX_ATTRIBUTE;
	return(0);
}

/*
 * Function:
 *	pthread_mutex_startup
 *
 * Description:
 *	Initialize everything for the mutex functions. This involves
 *	initializing mutex attributes and the limit on how many times
 *	to spin before yielding between attempts to get the mutex.
 */
void
pthread_mutex_startup()
{
	pthread_mutexattr_startup();
	mutex_spin_limit = MUTEX_SPIN_MAX;
}

/*
 * Function:
 *	initialize_mutex
 *
 * Description:
 *	Initialize a mutex structure. The mutex will be valid, unlocked and
 *	therefore unowned and un-named.
 */
void
initialize_mutex(pthread_mutex_t *mutex, pthread_mutexattr_t attr)
{
#ifdef MUTEX_OWNER
	mutex->owner = NO_PTHREAD;
#endif
	mutex->lock = SPIN_LOCK_UNLOCKED;
	mutex->name = NULL;
	mutex->flags = MUTEX_VALID;
}

/*
 * Function:
 *	pthread_mutex_init
 *
 * Parameters:
 *	mutex - the mutex to be created
 *	attr - attributes to indicate how it should be created
 *
 * Return value:
 *	0       Success
 *	-1      The pointer to the condition variable was invalid (EINVAL)
 *		The condition attribute was invalid (EINVAL)
 *
 * Description:
 *	The initialize_mutex function is used to do all the real
 *	work of creation once the parameters have been checked.
 */
int
pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t attr)
{
	if ((mutex == NO_MUTEX) || (attr == NO_MUTEX_ATTRIBUTE) ||
	    !(attr->flags&MUTEXATTR_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	initialize_mutex(mutex, attr);

	return(0);
}

/*
 * Function:
 *	pthread_mutex_destroy
 *
 * Parameters:
 *	mutex - the mutex to be deleted
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the mutex was invalid (EIVAL)
 *		The mutex was invalid (EINVAL)
 *		The mutex was locked (EBUSY)
 *
 * Description:
 *	After doing some validation that this is a real mutex and ready
 *	to be freed, the flag is set that the mutex is invalid. The
 *	mutex will be locked by the caller before it is made invalid.
 */
int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * If we can't lock the mutex then someone is using it, so we fail
	 */
	if (!lock_try_set(&mutex->lock)) {
		set_errno(EBUSY);
		return(-1);
	}

	mutex->flags &= ~MUTEX_VALID;
	return(0);
}

/*
 * Function:
 *	pthread_mutex_trylock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be locked
 *
 * Return value:
 *	1	the lock was successful
 *	0	the mutex was already locked by another thread
 *	-1	The pointer to the mutex was invalid (EIVAL)
 *		The mutex was invalid (EINVAL)
 *
 * Description:
 *	Try to lock the mutex after being sure that we have been passed
 *	a real mutex.
 */
int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

	if (!lock_try_set(&mutex->lock))
		return(0);

#ifdef MUTEX_OWNER
	mutex->owner = pthread_self();
#endif
	return(1);
}

/*
 * Function:
 *	pthread_mutex_wait_lock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be locked
 *
 * Description:
 *	This does the blocking wait for mutex locking. Spin for a while
 *	and then if we don't get the lock we lock more sociably by yielding
 *	in the loop. This implementation assumes that there is little to no
 *	contention and that mutexes will only be held for short periods.
 */
private void
pthread_mutex_wait_lock(pthread_mutex_t *mutex)
{
	int	i;

	/*
	 * fast spin
	 */
	for (i = 0; i < mutex_spin_limit; i++)
		if (lock_try_set(&mutex->lock))
			return;

	/*
	 * sociable spin
	 */
	while(!lock_try_set(&mutex->lock))
		pthread_yield();
}

/*
 * Function:
 *	pthread_mutex_lock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be locked
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the mutex was invalid (EIVAL)
 *		The mutex was invalid (EINVAL)
 *		The mutex is already owned by the caller (EDEADLK)
 *
 * Description:
 *	Try once to get the lock, if unsuccessful the blocking lock call
 *	is made. When this returns we have the lock so make the caller the
 *	owner.
 */
int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

#ifdef MUTEX_OWNER
	if (mutex->owner == pthread_self()) {
		set_errno(EDEADLK);
		return(-1);
	}
#endif

	if (!lock_try_set(&mutex->lock))
		pthread_mutex_wait_lock(mutex);

#ifdef MUTEX_OWNER
	mutex->owner = pthread_self();
#endif
	return(0);
}

/*
 * Function:
 *	pthread_mutex_unlock
 *
 * Parameters:
 *	mutex - a pointer to the mutex to be unlocked
 *
 * Return value:
 *	0	Success
 *	-1	The pointer to the mutex was invalid (EIVAL)
 *		The mutex was invalid (EINVAL)
 *		The mutex is not owned by the caller (EDEADLK)
 *
 * Description:
 *	Once the mutex is verified as real and locked by the caller
 *	it is unlocked.
 */
int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID)) {
		set_errno(EINVAL);
		return(-1);
	}

#ifdef MUTEX_OWNER
	if (mutex->owner != pthread_self()) {
		set_errno(EPERM);
		return(-1);
	}

	mutex->owner = NO_PTHREAD;
#endif
	lock_unset(&mutex->lock);
	return(0);
}

#ifdef DEBUG
/*
 * Function:
 *	pthread_mutex_dump
 *
 * Parameters:
 *	mutex - the mutex to be dumped
 *
 * Description:
 *	This is a debugging function which prints out the state of a mutex and
 *	its owner.
 */
void
pthread_mutex_dump(pthread_mutex_t *mutex)
{
	if ((mutex == NO_MUTEX) || !(mutex->flags&MUTEX_VALID)) {
		printf("NO MUTEX\n");
		return;
	}
	printf("MTX: %s %x %s Own: ", pthread_mutex_getname_np(mutex),
		mutex, mutex->lock == SPIN_LOCK_LOCKED ? "Lck" : "Unlck");
	if (mutex->owner == NO_PTHREAD)
		printf("NONE ");
	else
		printf("%x ", mutex->owner);
}
#endif
