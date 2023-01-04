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
static char	*sccsid = "@(#)$RCSfile: rec_mutex.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/07/07 20:49:52 $";
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

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak libc_declare_lock_functions = __libc_declare_lock_functions
#pragma weak rec_mutex_alloc = __rec_mutex_alloc
#pragma weak rec_mutex_free = __rec_mutex_free
#pragma weak rec_mutex_init = __rec_mutex_init
#pragma weak rec_mutex_lock = __rec_mutex_lock
#pragma weak rec_mutex_readlock = __rec_mutex_readlock
#pragma weak rec_mutex_reinit = __rec_mutex_reinit
#pragma weak rec_mutex_trylock = __rec_mutex_trylock
#pragma weak rec_mutex_unlock = __rec_mutex_unlock
#pragma weak rec_mutex_readunlock = __rec_mutex_readunlock
#endif
#endif
#include "lib_lock.h"
#include "rec_mutex.h"

#define	NULL	0

lib_lock_functions_t	libc_lock_funcs;

void
libc_declare_lock_functions(lib_lock_functions_t *f)
{
	libc_lock_funcs = *f;
	libc_locks_init();
}


int
rec_mutex_alloc(rec_mutex_t *m)
{
	extern char *malloc();

	*m = (rec_mutex_t) malloc(sizeof(struct rec_mutex));
	if (*m == NULL)
		return(-1);
	if (rec_mutex_init(*m)) {
		free((char *)*m);
		return(-1);
	}
	return(0);
}


int
rec_mutex_init(rec_mutex_t m)
{
	m->thread_id = NO_THREAD;
	m->count = 0;
	return (lib_mutex_create(libc_lock_funcs, &m->mutex));
}


void
rec_mutex_free(rec_mutex_t m)
{
	lib_mutex_delete(libc_lock_funcs, &m->mutex);
	free((char *) m);
}


int
rec_mutex_trylock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(libc_lock_funcs);

	if (m->thread_id == self) {	/* If already holding lock */
		m->count += 1;
		return(1);
	}
	if (lib_mutex_trylock(libc_lock_funcs, &m->mutex)) {
		/* We got the lock */
		m->count = 1;
		m->thread_id = self;
		return(1);
	}
	return(0);
}


void
rec_mutex_lock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(libc_lock_funcs);

	if (m->thread_id == self) {	/* If already holding lock */
		m->count += 1;
	} else {
		lib_mutex_lock(libc_lock_funcs, &m->mutex);
		m->count = 1;
		m->thread_id = self;
	}
}


void
rec_mutex_unlock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(libc_lock_funcs);

	/* Must be holding lock to unlock! */
	if (m->thread_id == self) {
		if (--(m->count) == 0) {
			m->thread_id = NO_THREAD;
			lib_mutex_unlock(libc_lock_funcs, &m->mutex);
		}
	}
}
void
rec_mutex_reinit(register rec_mutex_t m)
{
	register lib_threadid_t self;

	m->thread_id = NO_THREAD;
	m->count = 0;
        lib_mutex_unlock(libc_lock_funcs, &m->mutex);
}

void
rec_mutex_readlock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(libc_lock_funcs);

	if (lib_mutex_trylock(libc_lock_funcs, &m->mutex)) {
		/* first one in so we lock it */
		m->count = 1;
		m->thread_id = NO_THREAD;
	} else {
		/* allow multiple readers */
		m->count += 1;
	}
}


long
rec_mutex_readunlock(register rec_mutex_t m)
{
	register lib_threadid_t self;

	self = lib_thread_id(libc_lock_funcs);

	/* Up to readers to be correct */
	if (m->count == 0) {
	    return 0;;
	} else if (--(m->count) == 0) {
		m->thread_id = NO_THREAD;
		lib_mutex_unlock(libc_lock_funcs, &m->mutex);
	}
	return 1;
}
