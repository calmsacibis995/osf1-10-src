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
 *	@(#)$RCSfile: ldr_lock.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:43 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_lock.h
 * Lock management routines for loader
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_LDR_LOCK
#define	_H_LDR_LOCK

#include "lib_lock.h"	/* ugly - as this lives in /usr/ccs/lib/libc_r */

/* locking functions for loader's use */
extern lib_lock_functions_t	ldr_lock_funcs;
extern int disable_all_signals __((void));
extern int enable_all_signals __((void));

typedef	univ_t	ldr_lock_t;		/* a lock */

/* the loader uses a single global lock */
extern ldr_lock_t ldr_global_lock;

#define LDR_LOCK_FUNCTION(operation, argument) \
	(ldr_lock_funcs.operation ? \
	 (*ldr_lock_funcs.operation)((lib_mutex_t *)argument) : LDR_SUCCESS)

/* Initialize a lock.  Must be called before the lock is used.
 *	extern void ldr_lock_init(ldr_lock_t *lck);
 */

#define ldr_lock_init(lck) LDR_LOCK_FUNCTION(mutex_create, (lck))


/* Acquire a lock, sleeping until the lock is available.
 *	extern void ldr_lock(ldr_lock_t *lck);
 */

#define ldr_lock(lck) \
	(ldr_lock_funcs.mutex_lock ? \
	 (*ldr_lock_funcs.mutex_lock)((lib_mutex_t *)lck) : disable_all_signals())

/* Release a lock, awakening any thread waiting for the lock.
 *	extern void ldr_unlock(ldr_lock_t *lck);
 */
#define ldr_unlock(lck) \
	(ldr_lock_funcs.mutex_unlock ? \
	 (*ldr_lock_funcs.mutex_unlock)((lib_mutex_t *)lck) : enable_all_signals())

/* Return 0 if lock is currently held, <> 0 otherwise.
 *	extern void ldr_lock_test(ldr_lock_t *lck);
 */

#define	ldr_lock_test(lck) LDR_LOCK_FUNCTION(mutex_trylock, (lck))


/* Terminate use of a lock.  Should be called before any data structure
 * containing the lock is freed.
 *	extern void ldr_lock_term(ldr_lock_t *lck);
 */

#define	ldr_lock_term(lck) LDR_LOCK_FUNCTION(mutex_delete, (lck))

#endif	/* _H_LDR_LOCK */
