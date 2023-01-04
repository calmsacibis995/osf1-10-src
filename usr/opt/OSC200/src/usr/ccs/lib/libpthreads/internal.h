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
 *	@(#)$RCSfile: internal.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/21 11:00:11 $
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
 * Internal definitions for the pthreads package.
 */

#include <mach.h>
#include <stdlib.h>

#define MACRO_BEGIN	do {
#define	MACRO_END	} while (0);

#ifndef NULL
#define	NULL		0
#endif

/*
 * General Macros
 */
#define	min(a,b)	((a) < (b) ? (a) : (b))

/*
 * Pthread Queue Macros
 */
#define pthread_queue_init(q)	((q)->next = (q)->prev = (q))
#define pthread_queue_head(q)	((q)->next)
#define pthread_queue_end(q)	(q)
#define pthread_queue_next(q)	((q)->next)
#define pthread_queue_empty(q)	((q)->next == (q) ? TRUE : FALSE)
#define	pthread_queue_deq(q)	pthread_queue_remove(NULL, q)

#define pthread_queue_enq(q, e) \
	MACRO_BEGIN \
		(e)->next = (q); \
		(e)->prev = (q)->prev; \
		(q)->prev->next = (e); \
		(q)->prev = (e); \
	MACRO_END

#define	pthread_queue_remove(q, e) \
	MACRO_BEGIN \
		(e)->prev->next = (e)->next; \
		(e)->next->prev = (e)->prev; \
	MACRO_END

#define pthread_queue_move(to, from) \
	MACRO_BEGIN \
 		pthread_queue_enq(from, to); \
 		pthread_queue_remove(to, from); \
 		pthread_queue_init(from); \
	MACRO_END

/*
 * Error macros
 */

#define set_errno(e)	(pthread_self()->thread_errno = e)
#define get_errno()	(pthread_self()->thread_errno)

/*
 * Pthread internal definitions
 */

/* pthread flags */
#define	PTHREAD_INITIAL_THREAD	0x01

/* pthread state */
#define	PTHREAD_DETACHED	0x01
#define	PTHREAD_RETURNED	0x02
#define	PTHREAD_CONDWAIT	0x04

void
pthread_deactivate C_PROTOTYPE((pthread_t thread));

void
pthread_activate C_PROTOTYPE((pthread_t thread));

void
pthread_internal_error C_PROTOTYPE((char *error));

/*
 * Definitions for the Virtual Processor (vp) layer
 */

struct	vp {
	pthread_queue	link;
	unsigned int	flags;
	pthread_t	pthread;
	vm_size_t	stacksize;
	vm_offset_t	stackbase;
	thread_t	id;
	port_t		event_port;
	pthread_func_t	async_func;
	any_t		async_arg;
};

typedef struct vp	*vp_t;

#define	NO_VP	((vp_t)0)

/* vp flags */
#define	VP_INITIAL_STACK	0x01
#define VP_STARTED		0x02

extern	vp_t
vp_startup C_PROTOTYPE((void));

extern	vp_t
vp_self C_PROTOTYPE((void));

extern	vp_t
vp_create C_PROTOTYPE((pthread_attr_t attr));

extern void
vp_dealloc C_PROTOTYPE((vp_t vp));

extern void
vp_bind C_PROTOTYPE((vp_t vp, pthread_t thread));

extern void
vp_suspend C_PROTOTYPE((vp_t vp));

extern void
vp_resume C_PROTOTYPE((vp_t vp));

extern void
vp_yield C_PROTOTYPE((void));

extern void
vp_setup C_PROTOTYPE((vp_t vp));

/*
 * vp events
 */
extern void
vp_event_wait C_PROTOTYPE((vp_t vp, int *event, struct timespec *timeout));

extern void
vp_event_notify C_PROTOTYPE((vp_t vp, int event));

extern void
vp_event_flush C_PROTOTYPE((vp_t vp));

#define	NO_TIMEOUT	((struct timespec *)0)
#define	EVT_NONE	0
#define	EVT_RESUME	1
#define	EVT_TIMEOUT	2
#define	EVT_SIGNAL	3
#define	EVT_CANCEL	4
#define EVT_WAITER	5
#define EVT_SIGWAIT	0x80000000

extern int
allocate_event_port C_PROTOTYPE((port_t *port));

extern void
deallocate_event_port C_PROTOTYPE((port_t port));

/*
 * Stack information
 */

#define	RED_ZONE_SIZE	(2 * vm_page_size)

extern int
alloc_stack C_PROTOTYPE((vp_t vp, vm_size_t size));

extern void
dealloc_stack C_PROTOTYPE((vp_t vp));

extern int
realloc_stack C_PROTOTYPE((vp_t vp, vm_size_t newsize));

extern int
stack_self C_PROTOTYPE((vp_t vp));

/*
 * Spin locks
 */

extern void
spin_lock C_PROTOTYPE((volatile int *lock));

extern int
spin_trylock C_PROTOTYPE((volatile int *lock));

extern void
spin_unlock C_PROTOTYPE((volatile int *lock));

extern void
spinlock_create C_PROTOTYPE((volatile int *lock));

extern void
spinlock_delete C_PROTOTYPE((volatile int *lock));

extern int
lock_try_set C_PROTOTYPE((volatile int *lock));

extern void
lock_unset C_PROTOTYPE((volatile int *lock));

#define spin_unlock(lock)	lock_unset(lock)
#define spin_trylock(lock)	lock_try_set(lock)

#define SPIN_LOCK_LOCKED	1
#define SPIN_LOCK_UNLOCKED	0

/*
 * Initialization Functions
 */

extern void
pthread_mutex_startup C_PROTOTYPE((void));

extern void
initialize_mutex C_PROTOTYPE((pthread_mutex_t *mutex, pthread_mutexattr_t attr));

extern void
pthread_cond_startup C_PROTOTYPE((void));

extern void
initialize_condition C_PROTOTYPE((pthread_cond_t *cond, pthread_condattr_t attr));

extern void
pthread_attr_startup C_PROTOTYPE((void));

extern void
stack_startup C_PROTOTYPE((vp_t vp));

extern void
specific_data_startup C_PROTOTYPE((void));

/*
 * Pthread_once definitions
 */

#define	ONCE_INITIALIZED	1
#define	ONCE_EXECUTING		2
#define	ONCE_EXECUTED		3

/*
 * Thread specific data
 */

struct specific_key {
	long	flags;
	void	(*destructor)(void *);
};

typedef	struct specific_key	specific_key_t;

#define	KEY_FREE	0x00
#define	KEY_ALLOCATED	0x01

struct specific_data {
	long	flags;
	any_t	value;
};

typedef	struct specific_data	specific_data_t;

#define	SPECIFIC_DATA_SET	0x01

extern void
specific_data_setup C_PROTOTYPE((pthread_t thread));

extern void
specific_data_cleanup C_PROTOTYPE((pthread_t thread));

/*
 * Functions for fork()
 */
extern void
pthread_fork_prepare C_PROTOTYPE((void));

extern void
pthread_fork_parent C_PROTOTYPE((void));

extern void
pthread_fork_child C_PROTOTYPE((void));

extern void
vp_fork_prepare C_PROTOTYPE((void));

extern void
vp_fork_parent C_PROTOTYPE((void));

extern void
vp_fork_child C_PROTOTYPE((void));

extern void
stack_fork_prepare C_PROTOTYPE((void));

extern void
stack_fork_parent C_PROTOTYPE((void));

extern void
stack_fork_child C_PROTOTYPE((void));

extern void
malloc_fork_prepare C_PROTOTYPE((void));

extern void
malloc_fork_parent C_PROTOTYPE((void));

extern void
malloc_fork_child C_PROTOTYPE((void));

extern void
specific_fork_prepare C_PROTOTYPE((void));

extern void
specific_fork_parent C_PROTOTYPE((void));

extern void
specific_fork_child C_PROTOTYPE((void));

/*
 * Debugging information
 */

#ifdef	DEBUG

#define	private

extern	int	pthread_trace;

extern	void
pthread_log C_PROTOTYPE((char *string, any_t value));

extern void
pthread_dump C_PROTOTYPE((void));

extern void
pthread_vp_dump C_PROTOTYPE((void));

extern void
pthread_mutex_dump C_PROTOTYPE((pthread_mutex_t *mutex));

extern void
pthread_cond_dump C_PROTOTYPE((pthread_cond_t *cond));

#define PTHREAD_LOG(a, b)	if (pthread_trace) pthread_log(a, b)

#else	/* DEBUG */

#define	private	static
#define PTHREAD_LOG(a, b)

#endif	/* DEBUG */
