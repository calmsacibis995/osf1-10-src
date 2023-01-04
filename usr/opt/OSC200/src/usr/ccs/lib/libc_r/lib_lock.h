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
 *	@(#)$RCSfile: lib_lock.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:53:26 $
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
/*
 * Library locking functions provided by a threads package usable by a library
 */

#ifndef _LIB_LOCK_H_
#define _LIB_LOCK_H_

typedef	void	*lib_mutex_t;
typedef	void	*lib_spinlock_t;
typedef void	*lib_threadid_t;

typedef	int		(*lib_mutex_func_t)(lib_mutex_t *);
typedef	int		(*lib_spinlock_func_t)(lib_spinlock_t *);
typedef	lib_threadid_t	(*lib_threadid_func_t)(void);

typedef struct lib_lock_functions {
	lib_mutex_func_t	mutex_create;
	lib_mutex_func_t	mutex_delete;
	lib_mutex_func_t	mutex_lock;
	lib_mutex_func_t	mutex_unlock;
	lib_mutex_func_t	mutex_trylock;
	lib_spinlock_func_t	spinlock_create;
	lib_spinlock_func_t	spinlock_delete;
	lib_spinlock_func_t	spinlock_lock;
	lib_spinlock_func_t	spinlock_unlock;
	lib_spinlock_func_t	spinlock_trylock;
	lib_threadid_func_t	thread_id;
} lib_lock_functions_t;

#ifndef ESUCCESS
#define ESUCCESS	0
#endif

#ifndef NO_THREAD
#define NO_THREAD	(lib_threadid_t)0
#endif

#define	LIB_LOCK_FUNCTION(lockstruct, operation, arg) \
	((lockstruct).operation ? (*(lockstruct).operation)(arg) : ESUCCESS)

#define lib_mutex_create(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_create, lock)

#define lib_mutex_delete(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_delete, lock)

#define lib_mutex_lock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_lock, lock)

#define lib_mutex_unlock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_unlock, lock)

#define lib_mutex_trylock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, mutex_trylock, lock)

#define lib_spinlock_create(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_create, lock)

#define lib_spinlock_delete(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_delete, lock)

#define lib_spinlock_lock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_lock, lock)

#define lib_spinlock_unlock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_unlock, lock)

#define lib_spinlock_trylock(lockstruct, lock) \
		LIB_LOCK_FUNCTION(lockstruct, spinlock_trylock, lock)

#define lib_thread_id(lockstruct) \
	((lockstruct).thread_id ? (*(lockstruct).thread_id)() : 0)

#endif /* _LIB_LOCK_H_ */
