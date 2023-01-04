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
 *	@(#)$RCSfile: zalloc.h,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/11/01 19:17:34 $
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	zalloc.h
 *	Author:	Avadis Tevanian, Jr.
 *	Copyright (C) 1985, Avadis Tevanian, Jr.
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_ZALLOC_H_
#define _KERN_ZALLOC_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_ZALLOC_H_PREEMPT_
#endif
#endif

#include <mach/machine/vm_types.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/macro_help.h>

/*
 *	A zone is a collection of fixed size blocks for which there
 *	is fast allocation/deallocation access.  Kernel routines can
 *	use zones to manage data structures dynamically, creating a zone
 *	for each type of data structure to be managed.
 *
 */

typedef struct zone {
	decl_simple_lock_data(,lock)	/* generic lock */
	int		count;		/* Number of elements used now */
	vm_offset_t	free_elements;
	vm_size_t	cur_size;	/* current memory utilization */
	vm_size_t	max_size;	/* how large can this zone grow */
	vm_size_t	elem_size;	/* size of an element */
	vm_size_t	alloc_size;	/* size used for more memory */
	boolean_t	doing_alloc;	/* is zone expanding now? */
	char		*zone_name;	/* a name for the zone */
	unsigned int
	/* boolean_t */	pageable :1,	/* zone pageable? */
	/* boolean_t */	sleepable :1,	/* sleep if empty? */
	/* boolean_t */ exhaustible :1,	/* merely return if empty? */
	/* boolean_t */	collectable :1,	/* garbage collect empty pages */
	/* boolean_t */	expandable :1;	/* expand zone (with message)? */
	lock_data_t	complex_lock;	/* Lock for pageable zones */
	struct zone *	next_zone;	/* Link for all-zones list */
} *zone_t;

#define		ZONE_NULL	((zone_t) 0)


union zone_page_table_entry {
	union	zone_page_table_entry	*next;
	struct {
		int	in_free_list;
		int	alloc_count;
	} z;
};


/*
 * Support for garbage collection of unused zone pages:
 */
extern union zone_page_table_entry * zone_page_table;
extern vm_offset_t zone_map_min_address;

#define lock_zone_page_table() simple_lock(&zone_page_table_lock)
#define unlock_zone_page_table() simple_unlock(&zone_page_table_lock)

#define	zone_page(addr) \
    (&(zone_page_table[(atop(((vm_offset_t)addr) - zone_map_min_address))]))

extern void		zone_page_alloc();
extern void		zone_page_dealloc();
extern void		zone_page_in_use();
extern void		zone_page_free();
extern void		zone_gc();

extern vm_offset_t	zalloc();
extern vm_offset_t	zget();
extern zone_t		zinit();
extern void		zfree();
extern void		zchange();

#define ADD_TO_ZONE(zone, element)					\
MACRO_BEGIN								\
		*((vm_offset_t *)(element)) = (zone)->free_elements;	\
		(zone)->free_elements = (vm_offset_t) (element);	\
		(zone)->count--;					\
MACRO_END

#define REMOVE_FROM_ZONE(zone, ret, type)				\
MACRO_BEGIN								\
	(ret) = (type) (zone)->free_elements;				\
	if ((ret) != (type) 0) {					\
		(zone)->count++;					\
		(zone)->free_elements = *((vm_offset_t *)(ret));	\
	}								\
MACRO_END

#ifdef MEMLOG
#define ZFREE(zone, element)		\
MACRO_BEGIN				\
        zfree(zone, element);		\
MACRO_END

#define ZALLOC(zone, ret, type)			\
MACRO_BEGIN					\
                (ret) = (type)zalloc(zone);	\
MACRO_END

#define ZGET(zone, ret, type)			\
MACRO_BEGIN					\
                (ret) = (type)zget(zone);	\
MACRO_END

#else

#define ZFREE(zone, element)		\
MACRO_BEGIN				\
	simple_lock(&(zone)->lock);	\
	ADD_TO_ZONE(zone, element);	\
	simple_unlock(&(zone)->lock);	\
MACRO_END

#define ZALLOC(zone, ret, type)			\
MACRO_BEGIN					\
	register zone_t	z = (zone);		\
						\
	simple_lock(&z->lock);			\
	REMOVE_FROM_ZONE(zone, ret, type);	\
	simple_unlock(&z->lock);		\
	if ((ret) == (type)0)			\
		(ret) = (type)zalloc(z);	\
MACRO_END

#define ZGET(zone, ret, type)			\
MACRO_BEGIN					\
	register zone_t	z = (zone);		\
						\
	simple_lock(&z->lock);			\
	REMOVE_FROM_ZONE(zone, ret, type);	\
	simple_unlock(&z->lock);		\
MACRO_END
#endif /*MEMLOG*/

extern void		zcram();
extern void		zone_bootstrap();
extern void		zone_init();

#if	RT_PREEMPT
#ifdef	_KERN_ZALLOC_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_ZALLOC_H_ */
