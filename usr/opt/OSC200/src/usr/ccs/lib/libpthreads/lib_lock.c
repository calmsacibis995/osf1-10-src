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
static char	*sccsid = "@(#)$RCSfile: lib_lock.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:45 $";
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
 * File: lib_lock.c
 *
 * This file contains the locking functions and calls for the inter-library
 * locking scheme for libc_r and the loader.
 */

#include <pthread.h>
#include <errno.h>
#include "internal.h"
#include "lib_lock.h"

private lib_lock_functions_t	pthread_funcs;

/*
 * Function:
 *	pthread_libmutex_create
 *
 * Parameters:
 *	mutex - a pointer to a pthread_mutex_t *
 *
 * Return value:
 *	0	success, the mutex id is stored in *mutex
 *	-1	otherwise. Errno is set as in pthread_mutex_init
 *
 * Description:
 *	Pthread mutexes are structures and not malloc'd. library mutexes
 *	are pointers as they have no idea of the size and cannot find out
 *	in a threads provider independent way. So we have to malloc the
 *	mutex and initialize it.
 */
private int
pthread_libmutex_create(pthread_mutex_t **mutex)
{
	pthread_mutex_t	*m;
	int		ret;

	/*
	 * Allocate space for the structure
	 */
	m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (m == NULL) {
		set_errno(ENOMEM);
		return(-1);
	}
	/*
	 * Initialize the mutex
	 */
	if ((ret = pthread_mutex_init(m, pthread_mutexattr_default)) < 0)
		free(m);
	else
		*mutex = m;
	return(ret);
}

/*
 * Function:
 *	pthread_libmutex_delete
 *
 * Parameters:
 *	mutex - a pointer to the mutex to delete
 *
 * Return value:
 *	0	success, and the mutex id is null'd
 *	-1	The pointer is NULL or the mutex is NULL (EINVAL)
 *		whatever pthread_mutex_destroy returns
 *
 * Description:
 *
 */
private int
pthread_libmutex_delete(pthread_mutex_t **mutex)
{
	pthread_mutex_t	*m;
	int		ret;

	/*
	 * Check the pointer is ok to dereference
	 */
	if (mutex == NULL) {
		set_errno(EINVAL);
		return(-1);
	}

	/*
	 * Find the pthread mutex and check that it is OK
	 */
	m = *mutex;
	if (m == NO_MUTEX) {
		set_errno(EINVAL);
		return(-1);
	}
	/*
	 * destroy the pthread mutex and free its memory
	 */
	if ((ret = pthread_mutex_destroy(m)) == 0) {
		*mutex = NO_MUTEX;
		free(m);
	}
	return(ret);
}

/*
 * Function:
 *	pthread_libmutex_lock
 *
 * Parameters:
 *	mutex - pointer to the mutex to be locked
 *
 * Return value:
 *	Same as pthread_mutex_lock
 *
 * Description:
 *	This is needed to dereference the mutex passed for the pthread
 *	interface.
 */
private int
pthread_libmutex_lock(pthread_mutex_t **mutex)
{
	return(pthread_mutex_lock(*mutex));
}

/*
 * Function:
 *	pthread_libmutex_unlock
 *
 * Parameters:
 *	mutex - pointer to the mutex to be unlocked
 *
 * Return value:
 *	Same as pthread_mutex_unlock
 *
 * Description:
 *	This is needed to dereference the mutex passed for the pthread
 *	interface.
 */
private int
pthread_libmutex_unlock(pthread_mutex_t **mutex)
{
	return(pthread_mutex_unlock(*mutex));
}

/*
 * Function:
 *	pthread_libmutex_trylock
 *
 * Parameters:
 *	mutex - pointer to the mutex to be tested
 *
 * Return value:
 *	Same as pthread_mutex_trylock
 *
 * Description:
 *	This is needed to dereference the mutex passed for the pthread
 *	interface.
 */
private int
pthread_libmutex_trylock(pthread_mutex_t **mutex)
{
	return(pthread_mutex_trylock(*mutex));
}

/*
 * Function:
 *	pthread_libs_init
 *
 * Description:
 *	Called by pthread_init, initialize the library locking interfaces.
 *	the structure is constructed with pointers to all the functions
 *	and the two functions (to the reentrant C library and the loader)
 *	are called to initialize them with the locking entry points.
 */
void
pthread_libs_init()
{
	/*
	 * Initialize the mutex functions
	 */
	pthread_funcs.mutex_create = (lib_mutex_func_t)pthread_libmutex_create;
	pthread_funcs.mutex_delete = (lib_mutex_func_t)pthread_libmutex_delete;
	pthread_funcs.mutex_lock = (lib_mutex_func_t)pthread_libmutex_lock;
	pthread_funcs.mutex_unlock = (lib_mutex_func_t)pthread_libmutex_unlock;
	pthread_funcs.mutex_trylock = (lib_mutex_func_t)pthread_libmutex_trylock;

	/*
	 * Initialize the spinlock functions
	 */
	pthread_funcs.spinlock_create = (lib_spinlock_func_t)spinlock_create;
	pthread_funcs.spinlock_delete = (lib_spinlock_func_t)spinlock_delete;
	pthread_funcs.spinlock_lock = (lib_spinlock_func_t)spin_lock;
#undef spin_unlock
	pthread_funcs.spinlock_unlock = (lib_spinlock_func_t)spin_unlock;
#undef spin_trylock
	pthread_funcs.spinlock_trylock = (lib_spinlock_func_t)spin_trylock;

	/*
	 * Initialize the thread id function
	 */
	pthread_funcs.thread_id = (lib_threadid_func_t)pthread_self;

	/*
	 * set up the locks for libc_r
	 */
	libc_declare_lock_functions(&pthread_funcs);

	/*
	 * set up the locks for the loader
	 */
	ldr_declare_lock_functions(&pthread_funcs);
}

/*
 * Function:
 *	seterrno
 *
 * Parameters:
 *	error - the error number
 *
 * Description:
 *	Set the error number in both the per thread error number and the old
 *	global int. This is used by the reentrant C library to set errno.
 */
#undef errno
void
seterrno(int error)
{
	extern int	errno;

	errno = error;
	set_errno(error);
}

/*
 * Function:
 *	geterrno
 *
 * Return value:
 *	The per thread error number
 *
 * Description:
 *	Return the per thread error number not the global integer. This
 *	is used by the reentrant C library to find the contents of errno.
 */
int
geterrno()
{
	return(get_errno());
}

/*
 * Function:
 *	_errno
 *
 * Return value:
 *	The address of where the per thread error number is stored.
 *
 * Description:
 *	This returns the address of the per thread error number. This
 *	allows us to define errno to be
 *	#define errno *_errno()
 *	for the multithreaded programs using errno.
 */
int *
_errno()
{
	return(&(pthread_self()->thread_errno));
}
