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
 *	@(#)$RCSfile: rec_mutex.h,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/06/11 02:27:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * rec_mutex.h
 *
 * Mutex locks which allow recursive calls by the same thread.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _REC_MUTEX_H
#define _REC_MUTEX_H 1

/*
 * Recursive mutex definition.
 */

#include "lib_lock.h"

struct rec_mutex {
	lib_threadid_t	thread_id;	/* id of thread holding the lock */
	int		count;		/* Number of outstanding locks */
	lib_mutex_t	mutex;		/* The mutex itself */
};

typedef  struct rec_mutex	*rec_mutex_t;

/*
 * Recursive mutex operations.
 */

extern int		rec_mutex_alloc(rec_mutex_t *);
extern int		rec_mutex_init(rec_mutex_t);
extern void		rec_mutex_free(rec_mutex_t);
extern void		rec_mutex_lock(rec_mutex_t);
extern void		rec_mutex_unlock(rec_mutex_t);
extern int		rec_mutex_trylock(rec_mutex_t);

#define	_rec_mutex_lock		rec_mutex_lock
#define	_rec_mutex_unlock	rec_mutex_unlock
#define	_rec_mutex_alloc	rec_mutex_alloc
#define	_rec_mutex_init		rec_mutex_init
#define	_rec_mutex_free		rec_mutex_free
#define	_rec_mutex_trylock	rec_mutex_trylock

#endif /* _REC_MUTEX_H */
