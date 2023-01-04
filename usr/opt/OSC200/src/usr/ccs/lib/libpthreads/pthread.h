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
 *	@(#)$RCSfile: pthread.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/21 11:00:19 $
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
 * Definitions for the Pthreads package. This package redefines many of the
 * standards calls so that mutex, condition, and all attribute structures
 * do not have to be allocated at run time (eg malloc'd). The consequence of
 * this is that the opaque types must be passed by reference all the time,\
 * hence the redefinitions.
 */

#ifndef	_PTHREAD_H_
#define	_PTHREAD_H_

#define	_REENTRANT	1

#include <sys/timers.h>

typedef	void			*any_t;

#ifdef _NO_PROTO
#define C_PROTOTYPE(args)	()
#else
#define C_PROTOTYPE(args)	args
#endif

/*
 * pthread queue structures
 */

struct p_queue {
	struct p_queue	*next;
	struct p_queue	*prev;
};

#define PTHREAD_QUEUE_INVALID	{ NULL, NULL }

typedef	struct p_queue	pthread_queue;

/*
 * Mutex Attributes
 */

struct pthread_mutexattr {
	long	flags;
};

#define MUTEXATTR_VALID	0x1

typedef	struct pthread_mutexattr	*pthread_mutexattr_t;

#define NO_MUTEX_ATTRIBUTE	((pthread_mutexattr_t)0)

extern	pthread_mutexattr_t	pthread_mutexattr_default;

extern int
pthread_mutexattr_create C_PROTOTYPE((pthread_mutexattr_t *attr));

extern int
pthread_mutexattr_delete C_PROTOTYPE((pthread_mutexattr_t *attr));

/*
 * Mutexes
 */

#define MUTEX_OWNER	1

struct pthread_mutex {
	volatile int	lock;
	long		flags;
	char		*name;
#ifdef MUTEX_OWNER
	struct pthread	*owner;
#endif
};

#define	MUTEX_VALID	0x1

#ifdef MUTEX_OWNER
#define PTHREAD_MUTEX_INVALID { 0, 0L, NULL, NULL }
#else
#define PTHREAD_MUTEX_INVALID { 0, 0L, NULL }
#endif

typedef struct pthread_mutex	pthread_mutex_t;

#define NO_MUTEX	((pthread_mutex_t *)0)

extern int
pthread_mutex_init C_PROTOTYPE((pthread_mutex_t *mutex, pthread_mutexattr_t attr));

extern int
pthread_mutex_destroy C_PROTOTYPE((pthread_mutex_t *mutex));

extern int
pthread_mutex_lock C_PROTOTYPE((pthread_mutex_t *mutex));

extern int
pthread_mutex_trylock C_PROTOTYPE((pthread_mutex_t *mutex));

extern int
pthread_mutex_unlock C_PROTOTYPE((pthread_mutex_t *mutex));

#define	pthread_mutex_setname_np(mutex, string)	((mutex)->name = (string))
#define	pthread_mutex_getname_np(mutex)	((mutex)->name != 0 ? (mutex)->name : "?")

#ifdef MUTEX_OWNER
#define	pthread_mutex_getowner_np(mutex)	((mutex)->owner)
#endif

/*
 * Condition variable attributes
 */

struct pthread_condattr {
	long	flags;
};

#define CONDATTR_VALID	0x1

typedef	struct pthread_condattr	*pthread_condattr_t;

#define NO_COND_ATTRIBUTE	((pthread_condattr_t)0)

extern	pthread_condattr_t	pthread_condattr_default;

extern int
pthread_condattr_create C_PROTOTYPE((pthread_condattr_t *attr));

extern int
pthread_condattr_delete C_PROTOTYPE((pthread_condattr_t *attr));

/*
 * Condition Variables
 */

struct pthread_condition {
	pthread_queue	waiters;
	long		flags;
	int		lock;
	char		*name;
};

#define COND_VALID	0x1

#define PTHREAD_COND_INVALID	{ PTHREAD_QUEUE_INVALID, 0, 0, NULL }

typedef struct pthread_condition	pthread_cond_t;

#define NO_COND	((pthread_cond_t *)0)

extern int
pthread_cond_init C_PROTOTYPE((pthread_cond_t *cond, pthread_condattr_t attr));

extern int
pthread_cond_destroy C_PROTOTYPE((pthread_cond_t *cond));

extern int
pthread_cond_wait C_PROTOTYPE((pthread_cond_t *cond, pthread_mutex_t *mutex));

extern int
pthread_cond_timedwait C_PROTOTYPE((pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *timeout));

extern int
pthread_cond_signal C_PROTOTYPE((pthread_cond_t *cond));

extern int
pthread_cond_broadcast C_PROTOTYPE((pthread_cond_t *cond));

#define	pthread_cond_setname_np(cond, string)	((cond)->name = (string))
#define	pthread_cond_getname_np(cond)	((cond)->name != 0 ? (cond)->name : "?")

/*
 * Pthread Attributes
 */

struct pthread_attr {
	long		flags;
	vm_size_t	stacksize;
};

#define	ATTRIBUTE_VALID	0x1

typedef	struct pthread_attr	*pthread_attr_t;

#define NO_ATTRIBUTE	((pthread_attr_t)0)

extern	pthread_attr_t	pthread_attr_default;

extern int
pthread_attr_create C_PROTOTYPE((pthread_attr_t *attr));

extern int
pthread_attr_delete C_PROTOTYPE((pthread_attr_t *attr));

extern int
pthread_attr_setstacksize C_PROTOTYPE((pthread_attr_t *attr, long stacksize));

extern long
pthread_attr_getstacksize C_PROTOTYPE((pthread_attr_t attr));

extern int
pthread_attr_setprio C_PROTOTYPE((pthread_attr_t *attr, int priority));

extern int
pthread_attr_getprio C_PROTOTYPE((pthread_attr_t attr));

extern int
pthread_attr_setsched C_PROTOTYPE((pthread_attr_t *attr, int scheduler));

extern int
pthread_attr_getsched C_PROTOTYPE((pthread_attr_t attr));

extern int
pthread_attr_setinheritsched C_PROTOTYPE((pthread_attr_t *attr, int inherit));

extern int
pthread_attr_getinheritsched C_PROTOTYPE((pthread_attr_t attr));

/*
 * Thread specific data
 */

typedef any_t (*pthread_func_t) C_PROTOTYPE((any_t arg));

typedef unsigned	pthread_key_t;

extern int
pthread_keycreate C_PROTOTYPE((pthread_key_t *key_ptr, void (*destructor)(any_t)));

extern int
pthread_getspecific C_PROTOTYPE((pthread_key_t key, any_t *value));

extern int
pthread_setspecific C_PROTOTYPE((pthread_key_t key, any_t value));

/*
 * Cancellation
 */

struct pthread_cancel_state {
	unsigned int	async:1,
			sync:1,
			pending:1;
};

typedef struct pthread_cancel_state	pthread_cancel_t;

#define	CANCEL_OFF	0
#define	CANCEL_ON	1

struct	pthread_cleanup_handler {
	struct pthread_cleanup_handler	*next_handler;
	void				(*handler_function)();
	any_t				handler_arg;
};

typedef struct pthread_cleanup_handler	pthread_cleanup_handler_t;

extern void
pthread_testcancel C_PROTOTYPE((void));

extern int
pthread_setcancel C_PROTOTYPE((int state));

extern int
pthread_setasynccancel C_PROTOTYPE((int state));

#define pthread_cleanup_push(func, arg) { \
	pthread_cleanup_handler_t	__handler, **__handler_queue; \
	__handler.handler_function = func; \
	__handler.handler_arg = arg; \
	(void)pthread_getspecific(_pthread_cleanup_handlerqueue, (void *)&__handler_queue); \
	__handler.next_handler = *__handler_queue; \
	*__handler_queue = &__handler;

#define	pthread_cleanup_pop(execute) \
	*__handler_queue = __handler.next_handler; \
	if (execute) \
		(*__handler.handler_function)(__handler.handler_arg); \
}

extern pthread_key_t	_pthread_cleanup_handlerqueue;

/*
 * Pthreads
 */

#include <setjmp.h>

struct pthread {
	pthread_queue		link;
	struct pthread		*all_thread_link;
	unsigned		flags;
	unsigned		state;
	struct vp		*vp;
	char			*name;
	pthread_mutex_t		lock;
	unsigned		join_count;
	pthread_cond_t		done;
	pthread_func_t		func;
	any_t			arg;
	any_t			returned;
	int			thread_errno;
	struct pthread_attr	attr;
	struct specific_data	*specific_data;
	pthread_cancel_t	cancel_state;
	pthread_cleanup_handler_t *cleanup_queue;
	jmp_buf			exit_jmp;
};

#define async_cancel	cancel_state.async
#define sync_cancel	cancel_state.sync
#define pending_cancel	cancel_state.pending

typedef struct pthread	*pthread_t;

#define	NO_PTHREAD	((pthread_t)0)

extern pthread_t
pthread_self C_PROTOTYPE((void));

extern int
pthread_create C_PROTOTYPE((pthread_t *thread, pthread_attr_t attr, pthread_func_t start_routine, any_t arg));

extern int
pthread_detach C_PROTOTYPE((pthread_t *thread));

extern int
pthread_join C_PROTOTYPE((pthread_t thread, any_t *status));

extern void
pthread_exit C_PROTOTYPE((void *));

extern void
pthread_yield C_PROTOTYPE((void));

extern int
pthread_cancel C_PROTOTYPE((pthread_t thread));

extern int
pthread_setprio C_PROTOTYPE((pthread_t thread, int prio));

extern int
pthread_getprio C_PROTOTYPE((pthread_t thread));

extern int
pthread_setscheduler C_PROTOTYPE((pthread_t thread, int alg, int prio));

extern int
pthread_getscheduler C_PROTOTYPE((pthread_t thread));

extern int
pthread_equal C_PROTOTYPE((pthread_t t1, pthread_t t2));

#define	pthread_equal(t1, t2)	(t1 == t2)

#define pthread_setname_np(thread, string)	(thread->name = string)
#define	pthread_getname_np(thread)	((thread)->name != 0 ? (thread)->name : "?")

/*
 * pthread_once
 */

struct pthread_once {
	int		lock;
	int		flag;
	pthread_mutex_t	mutex;
	pthread_cond_t	executed;
};

typedef struct pthread_once	pthread_once_t;

#define	ONCE_UNINITIALIZED	0

#define	pthread_once_init	{ 0, ONCE_UNINITIALIZED, PTHREAD_MUTEX_INVALID, PTHREAD_COND_INVALID }

extern int
pthread_once C_PROTOTYPE((pthread_once_t *once_block, void (*init_routine)()));

#endif	/* _PTHREAD_H_ */
