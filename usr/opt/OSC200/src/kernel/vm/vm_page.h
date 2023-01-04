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
 *	@(#)$RCSfile: vm_page.h,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1992/12/11 15:39:21 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	vm/vm_page.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Resident memory system definitions.
 */

#ifndef	__VM_PAGE__
#define __VM_PAGE__ 1

#include <mach/boolean.h>
#include <mach/vm_prot.h>
#include <mach/vm_param.h>
#include <vm/vm_object.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/zalloc.h>

#include <kern/macro_help.h>
#include <kern/sched_prim.h>	/* definitions of wait/wakeup */
#include <vm/vm_anonpage.h>
#include <vm/vm_vppage.h>
#include <vm/vm_debug.h>


#define	VM_MAXPIO	5
#define	VM_PAGE_PRIVATE	2		/* size of private info */

struct vm_page {
	struct vm_page	*pg_pnext,	/* resident page management */
			*pg_pprev;
	struct vm_page  *pg_onext,	/* all pages in same object */
			*pg_oprev;	
	struct	vm_page	*pg_hnext,	/* Hash byte object offset */
			*pg_hprev;
	vm_object_t	pg_object;	/* which object am I in */
	vm_offset_t	pg_offset;	/* offset in object */
				
	unsigned int		
					/* page wire count */
			pg_wire_count:16,
					/* I/O transfer against page */
			pg_iocnt:VM_MAXPIO,
			pg_free:1,	/* page is on free list  */
			pg_busy:1,	/* page is in busy  */
			pg_wait:1,	/* someone is waiting for page */
			pg_error:1,	/* an error was detected */
			pg_dirty:1,	/* Page must be cleaned */
			pg_zeroed:1,	/* Page has been zeroed */
			pg_reserved:5;	/* Reserved for paging subsystem */
	unsigned short  pg_hold:8;	/* 
					 * Reason page can't be touched 
					 * manipulated by paging subsystem.
					 */
	vm_offset_t	pg_phys_addr;	/* 
					 * Physical address of page, passed
					 *  to pmap_enter (read-only) 
					 */
	union {				/* Object pager specific info */
		struct vm_anonpage _apg;/* Anonymous memory pages */
		struct vm_vppage _vppg; /* vnode memory pages */	
		struct vm_anon **_kpapp;/* pageable kernel memory */
		vm_offset_t 		/* must be equal to max size of above */
			_pg_private[VM_PAGE_PRIVATE];
	} _upg;
};

#define pg_private 		_upg._pg_private
#define	pg_app			_upg._kpapp

typedef struct vm_page	*vm_page_t;

#define VM_PAGE_NULL		((vm_page_t) 0)
#define VM_PAGE_EMPTY		((vm_page_t) 1)


/*
 *	Each pageable resident page falls into one of three lists:
 *
 *	free	
 *		Available for allocation now.
 *	inactive
 *		Not referenced in any map, but still has an
 *		object/offset-page mapping, and may be dirty.
 *		This is the list of pages that should be
 *		paged out next.
 *	active
 *		A list of pages which have been placed in
 *		at least one physical map.  This list is
 *		ordered, in LRU-like fashion.
 */

extern
vm_page_t vm_page_queue_free;	/* memory free queue */

#ifdef	VM_PAGE_DEBUG
extern
vm_page_t	vm_page_array;		/* First resident page in table */
extern
long		first_page;		/* first physical page number */
					/* ... represented in vm_page_array */
extern
long		last_page;		/* last physical page number */
					/* ... represented in vm_page_array */
					/* [INCLUSIVE] */
#endif	/* VM_PAGE_DEBUG */

#ifdef	KERNEL

extern
vm_offset_t	first_phys_addr;	/* physical address for first_page */
extern
vm_offset_t	last_phys_addr;		/* physical address for last_page */

extern
int	vm_page_free_count;		/* How many pages are free? */
extern
int	vm_page_zeroed_count;		/* How many zeroed pages are there? */
extern
int	vm_page_wire_count;		/* How many pages are wired? */
extern
int	vm_page_kluster;		/* Free page kluster enabling */

/*
 * Page struct to physical page.  Would like the
 * inverse operation but must wait for vm_resident.c changes.
 */

#define	page_to_phys(PP)	(PP)->pg_phys_addr
#define vm_page_to_phys(PAGE)	page_to_phys(PAGE)

udecl_simple_lock_data(extern,vm_page_free_lock)
						/* lock on free page queue */

extern
boolean_t		vm_page_free_wanted;	/* a thread is waiting for memory */


extern void		vm_page_module_init();
extern vm_offset_t	vm_page_startup();
extern void		vm_page_create();
extern vm_page_t	vm_page_lookup();
extern vm_page_t	vm_page_alloc();
extern vm_page_t	vm_zeroed_page_alloc();
extern vm_page_t	vm_page_clean();
extern void		vm_page_clean_done();
extern void		vm_page_init();
extern void		vm_page_free();
extern void		vm_page_free_synchronized();
extern void		vm_page_activate();
extern void		vm_page_deactivate();
extern void		vm_page_rename();
extern void		vm_page_insert();
extern void		vm_page_remove();

extern void		vm_page_zero_fill();
extern void		vm_page_copy();

extern void		vm_page_wire();
extern void		vm_page_unwire();

extern void		vm_set_page_size();


#if	!UNIX_LOCKS || (!MACH_SLOCKS && !RT_PREEMPT)
#define	vm_page_lock(PP)	1
#define vm_page_unlock(PP)	1
#else
udecl_simple_lock_data(extern,vm_page_lock_data)
#define	vm_page_lock(PP) 	usimple_lock(&vm_page_lock_data)
#define	vm_page_unlock(PP) 	usimple_unlock(&vm_page_lock_data)
#endif	/* !UNIX_LOCKS */

 
#define	vm_page_hold(PP)						\
	(vm_page_lock((PP)), 						\
	(PP)->pg_hold++,						\
	((PP)->pg_busy ? 						\
			(vm_page_unlock((PP)), TRUE) :			\
			(vm_page_unlock((PP)), FALSE)))

#define	vm_page_release(PP)						\
	(vm_page_lock((PP)),						\
	(PP)->pg_hold--,						\
	vm_page_unlock((PP)))

#if	VM_FREE_CHECK

#define	vm_free_check_pp(PP)  						\
	if ((PP)->pg_free) {						\
		panic("vm_free_check: ", (PP));				\
		(PP)->pg_free = 1;					\
	}

#define	vm_free_check_po(PG,FLD) 					\
		if (&((struct vm_page *) 0)->pg_pnext == 		\
			&((struct vm_page *) 0)->pg_/**/FLD/**/next) {	\
			vm_free_check_pp((PG)->pg_pnext);		\
			vm_free_check_pp((PG)->pg_pprev);		\
		}

#else
#define	vm_free_check_pp(PP)
#define	vm_free_check_po(PG,FLD)
#endif	/* VM_FREE_CHECK */

/*
 * It assumed the caller of this has
 * asserted a hold against the page.
 * Otherwise the page could be gone again by
 * the time the thread is scheduled to run.
 */

#define	vm_page_wait(PP) {						\
	vm_page_lock((PP));						\
	while ((PP)->pg_busy) {						\
		(PP)->pg_wait = 1;					\
		assert_wait((vm_offset_t) (PP), FALSE);			\
		vm_page_unlock((PP));					\
		thread_block();						\
		vm_page_lock((PP));					\
	}								\
	vm_page_unlock((PP));						\
}
	

#define pgl_insert_tail(Q,PP,FLD) {					\
	vm_free_check_pp(PP);						\
	if ((Q) == VM_PAGE_NULL) {					\
		(Q) = (PP);						\
		(PP)->pg_/**/FLD/**/next = 				\
			(PP)->pg_/**/FLD/**/prev = (PP);		\
	}								\
	else {								\
		register vm_page_t PG;					\
		PG = (Q);						\
		vm_free_check_pp(PG);					\
		vm_free_check_po(PG,FLD);				\
		PG->pg_/**/FLD/**/prev->pg_/**/FLD/**/next = (PP);	\
		(PP)->pg_/**/FLD/**/prev = PG->pg_/**/FLD/**/prev;	\
		PG->pg_/**/FLD/**/prev = (PP);				\
		(PP)->pg_/**/FLD/**/next = PG;				\
	}								\
}

#define pgl_insert(Q,PP,FLD) {						\
	vm_free_check_pp(PP);						\
	if ((Q) == VM_PAGE_NULL) {					\
		(Q) = (PP);						\
		(PP)->pg_/**/FLD/**/next = 				\
			(PP)->pg_/**/FLD/**/prev = (PP);		\
	}								\
	else {								\
		register vm_page_t PG;					\
		PG = (Q);						\
		vm_free_check_pp(PG);					\
		vm_free_check_po(PG,FLD);				\
		PG->pg_/**/FLD/**/prev->pg_/**/FLD/**/next = (PP);	\
		(PP)->pg_/**/FLD/**/prev = PG->pg_/**/FLD/**/prev;	\
		PG->pg_/**/FLD/**/prev = (PP);				\
		(PP)->pg_/**/FLD/**/next = PG;				\
		(Q) = (PP);						\
	}								\
}

#define pgl_remove(Q,PP,FLD) {						\
	vm_free_check_pp(PP);						\
	if ((PP)->pg_/**/FLD/**/next == (PP))  (Q) = VM_PAGE_NULL;	\
	else {								\
		if ((Q) == (PP)) (Q) = (PP)->pg_/**/FLD/**/next;	\
		vm_free_check_po(PP,FLD);				\
		(PP)->pg_/**/FLD/**/next->pg_/**/FLD/**/prev =		\
			(PP)->pg_/**/FLD/**/prev;			\
		(PP)->pg_/**/FLD/**/prev->pg_/**/FLD/**/next =		\
			(PP)->pg_/**/FLD/**/next;			\
	}								\
}

#define	VM_PAGE_CHECK(PP)

#endif	/* KERNEL */
#endif	/* !__VM_PAGE__ */
