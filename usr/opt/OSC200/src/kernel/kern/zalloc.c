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
static char	*sccsid = "@(#)$RCSfile: zalloc.c,v $ $Revision: 4.2.22.4 $ (DEC) $Date: 1993/11/02 21:01:12 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	kern/zalloc.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Zone-based memory allocator.  A zone is a collection of fixed size
 *	data blocks for which quick allocation/deallocation is possible.
 */

#include <mach_debug.h>

#include <mach/vm_param.h>
#include <kern/zalloc.h>
#include <kern/macro_help.h>
#include <vm/vm_kern.h>
#include <vm/vm_tune.h>
#include <sys/syslog.h>
#ifdef MEMLOG
#include <sys/memlog.h>
#endif

zone_t		zone_zone;	/* this is the zone containing other zones */

boolean_t	zone_ignore_overflow = FALSE;

extern vm_offset_t	zdata;
extern vm_size_t	zdata_size;

#define	ZONE_DEBUG 0

#if	ZONE_DEBUG
#define	ZD_TRACE_LIMIT	200
struct zd_trace {
	vm_offset_t zd_va;
	vm_size_t zd_sz;
	vm_offset_t zd_pc;
};

long zd_alloc_count;
struct zd_trace zd_alloc[ZD_TRACE_LIMIT];
long zd_free_count;
struct zd_trace zd_free[ZD_TRACE_LIMIT];
long zd_gc_count;
struct zd_trace zd_gc[ZD_TRACE_LIMIT];

#define	zd_trace_load(TYPE,VA,SZ,PC) do {				\
	register struct zd_trace *ZDP = 				\
		&zd_/**/TYPE/**/[zd_/**/TYPE/**/_count];		\
		ZDP->zd_va = (VA);					\
		ZDP->zd_sz = (SZ);					\
		ZDP->zd_pc = (vm_offset_t) (PC);			\
		if (++zd_/**/TYPE/**/_count >= ZD_TRACE_LIMIT)		\
			zd_/**/TYPE/**/_count = 0;			\
} while (1 == 2)

#else

#define	zd_trace_load(TYPE,VA,SZ,PC)

#endif	/* ZONE_DEBUG */


vm_map_t	zone_map = VM_MAP_NULL;
vm_map_t      kentry_map = VM_MAP_NULL;

#if	MACH_LDEBUG || MACH_LTRACKS
#include <kern/thread.h>
/*
 * Some of this is useless given that we call the
 *  macro from within the functions below and we
 *  don't know who called the function, but the
 *  thread info should help that out.
 */
#define lock_zone(zone)					\
MACRO_BEGIN						\
	if (zone->pageable) { 				\
		lock_write(&zone->complex_lock);	\
	} else {					\
		simple_lock(&zone->lock);		\
		zone->complex_lock.lck_addr = getpc();	\
		zone->complex_lock.thread = (char *)current_thread();	\
	}						\
MACRO_END

#define unlock_zone(zone)				\
MACRO_BEGIN						\
	if (zone->pageable) { 				\
		lock_done(&zone->complex_lock);		\
	} else {					\
		zone->complex_lock.lck_addr |= 0x80000000;	\
		zone->complex_lock.unlck_addr = getpc();	\
		zone->complex_lock.thread = (char *)current_thread();	\
		simple_unlock(&zone->lock);		\
	}						\
MACRO_END

#else	/* MACH_LDEBUG || MACH_LTRACKS */
#define lock_zone(zone)					\
MACRO_BEGIN						\
	if (zone->pageable) { 				\
		lock_write(&zone->complex_lock);	\
	} else {					\
		simple_lock(&zone->lock);		\
	}						\
MACRO_END

#define unlock_zone(zone)				\
MACRO_BEGIN						\
	if (zone->pageable) { 				\
		lock_done(&zone->complex_lock);		\
	} else {					\
		simple_unlock(&zone->lock);		\
	}						\
MACRO_END

#endif	/* MACH_LDEBUG || MACH_LTRACKS */

#define lock_zone_init(zone)				\
MACRO_BEGIN						\
	if (zone->pageable) { 				\
		lock_init(&zone->complex_lock, TRUE);	\
	} else {					\
		simple_lock_init(&zone->lock);		\
	}						\
MACRO_END

/*
 *	Protects the static variables inside zget_space.
 */
decl_simple_lock_data(,	zget_space_lock)

/*
 *	Garbage collection map information
 */
decl_simple_lock_data(, zone_page_table_lock)
union zone_page_table_entry *	zone_page_table;
vm_offset_t			zone_map_min_address;
vm_offset_t			zone_map_max_address;
int				zone_pages;

extern void zone_page_init();

#define	ZONE_PAGE_USED  0
#define ZONE_PAGE_UNUSED -1


/*
 *	Protects first_zone, last_zone, num_zones,
 *	and the next_zone field of zones.
 */
decl_simple_lock_data(,	all_zones_lock)
zone_t			first_zone;
zone_t			*last_zone;
int			num_zones;

/*
 *	zinit initializes a new zone.  The zone data structures themselves
 *	are stored in a zone, which is initially a static structure that
 *	is initialized by zone_init.
 */
zone_t zinit(size, max, alloc, name)
	vm_size_t	size;		/* the size of an element */
	vm_size_t	max;		/* maximum memory to use */
	vm_size_t	alloc;		/* allocation size */
	char		*name;		/* a name for the zone */
{
	register zone_t		z;

	if (zone_zone == ZONE_NULL)
		z = (zone_t) zdata;
	else if ((z = (zone_t) zalloc(zone_zone)) == ZONE_NULL)
		return(ZONE_NULL);

 	if (alloc == 0)
		alloc = PAGE_SIZE;
	if (size == 0)
		size = sizeof(z->free_elements);

	/*
	 *	Round off all the parameters appropriately.
	 */

	if ((max = round_page(max)) < (alloc = round_page(alloc)))
		max = alloc;

	z->free_elements = 0;
	z->cur_size = 0;
	z->max_size = max;
	z->elem_size = ((size-1) + sizeof(z->free_elements)) -
			((size-1) % sizeof(z->free_elements));
	z->alloc_size = alloc;
	z->zone_name = name;
	z->count = 0;
	z->doing_alloc = FALSE;
	z->pageable = FALSE;
	z->exhaustible = FALSE;
	z->sleepable = FALSE;
	z->collectable = TRUE;
	z->expandable = TRUE;
	lock_zone_init(z);

	/*
	 *	Add the zone to the all-zones list.
	 */

	z->next_zone = ZONE_NULL;
	simple_lock(&all_zones_lock);
	*last_zone = z;
	last_zone = &z->next_zone;
	num_zones++;
	simple_unlock(&all_zones_lock);

	return(z);
}

extern zone_t vm_map_kentry_zone;

/*
 *	Cram the given memory into the specified zone.
 */
void zcram(zone, newmem, size)
	register zone_t		zone;
	vm_offset_t		newmem;
	vm_size_t		size;
{
	register vm_size_t	elem_size;

	if (newmem == (vm_offset_t) 0) {
		printf("zcram: config parameter zone_size is set too low to support the present system workload. \n");
		printf("zcram: increase zone_size and reconfigure the kernel. \n");  
		panic("zcram: system cannot continue, zone subsystem out of kernel virtual address space. \n");
	}
	elem_size = zone->elem_size;

	lock_zone(zone);
	while (size >= elem_size) {
		ADD_TO_ZONE(zone, newmem);
                if ( zone != vm_map_kentry_zone )
                        zone_page_alloc(newmem, elem_size);
		zone->count++;	/* compensate for ADD_TO_ZONE */
		size -= elem_size;
		newmem += elem_size;
		zone->cur_size += elem_size;
	}
	unlock_zone(zone);
}

/*
 * Contiguous space allocator for non-paged zones. Allocates "size" amount
 * of memory from zone_map.
 */
vm_offset_t zalloc_next_space = 0;
vm_offset_t zalloc_end_of_space = 0;


vm_offset_t zget_space(size)
	vm_offset_t size;
{
	vm_offset_t	new_space = 0;
	vm_offset_t	result;
	vm_size_t	space_to_add;

	simple_lock(&zget_space_lock);
	while ((zalloc_next_space + size) > zalloc_end_of_space) {
		/*
		 *	Add at least one page to allocation area.
		 */

		space_to_add = (PAGE_SIZE < size) ? 
						round_page(size) : PAGE_SIZE;

		if (new_space == 0) {
			/*
			 *	Memory cannot be wired down while holding
			 *	any locks that the pageout daemon might
			 *	need to free up pages.  [Making the zget_space
			 *	lock a complex lock does not help in this
			 *	regard.]
			 *
			 *	Unlock and allocate memory.  Because several
			 *	threads might try to do this at once, don't
			 *	use the memory before checking for available
			 *	space again.
			 */

			simple_unlock(&zget_space_lock);

			new_space = kmem_alloc(zone_map, space_to_add);
			zone_page_init(new_space, space_to_add,
							ZONE_PAGE_USED);
			if (new_space == 0)
				return(0);
			zd_trace_load(alloc,new_space, space_to_add,
				get_caller_ra());
			simple_lock(&zget_space_lock);
			continue;
		}

		
		/*
	  	 *	Memory was allocated in a previous iteration.
		 *
		 *	Check whether the new region is contiguous
		 *	with the old one.
		 */

		if (new_space != zalloc_end_of_space) {
			/*
			 *	Throw away the remainder of the
			 *	old space, and start a new one.
			 */
			zalloc_next_space = new_space;
		}

		zalloc_end_of_space = new_space + space_to_add;

		new_space = 0;
	}
	result = zalloc_next_space;
	zalloc_next_space += size;		
	simple_unlock(&zget_space_lock);

	if (new_space != 0) {
		kmem_free(zone_map, new_space, space_to_add);
		zd_trace_load(free,new_space, space_to_add, -1);
	}

	return(result);
}


/*
 *	Initialize the "zone of zones" which uses fixed memory allocated
 *	earlier in memory initialization.  zone_bootstrap is called
 *	before zone_init.
 */
void zone_bootstrap()
{
	simple_lock_init(&zget_space_lock);

	simple_lock_init(&all_zones_lock);
	first_zone = ZONE_NULL;
	last_zone = &first_zone;
	num_zones = 0;

	zone_zone = ZONE_NULL;
	zone_zone = zinit(sizeof(struct zone), sizeof(struct zone), 0, "zones");
	zchange(zone_zone, FALSE, FALSE, FALSE, FALSE);

	zcram(zone_zone, (vm_offset_t)(zone_zone + 1),
	      zdata_size - sizeof *zone_zone);
}

void zone_init()
{
	vm_offset_t	zone_min;
	vm_offset_t	zone_max;
	vm_size_t	zone_size;
	vm_size_t	zone_table_size;
        vm_offset_t     kentry_zone_min;
        vm_offset_t     kentry_zone_max;
        vm_size_t       kentry_zone_size;


        zone_size = vm_tune_value(zone_size);
        kentry_zone_size = vm_tune_value(kentry_zone_size);

	zone_map = kmem_suballoc(kernel_map, &zone_min, &zone_max,
				 zone_size, FALSE);

	kentry_map = kmem_suballoc(kernel_map, &kentry_zone_min,
                        &kentry_zone_max, kentry_zone_size, FALSE);

	/*
	 * Setup garbage collection information:
	 */
  	zone_table_size = atop(zone_max - zone_min) * 
				sizeof(union zone_page_table_entry);
	zone_page_table = (union zone_page_table_entry *)
				kmem_alloc(zone_map, zone_table_size);
	zone_min = (vm_offset_t)zone_page_table + round_page(zone_table_size);
	zone_pages = atop(zone_max - zone_min);
	zone_map_min_address = zone_min;
	zone_map_max_address = zone_max;
	simple_lock_init(&zone_page_table_lock);
	zone_page_init(zone_min, zone_max - zone_min, ZONE_PAGE_UNUSED);
}
	

/*
 *	zalloc returns an element from the specified zone.
 */
vm_offset_t zalloc(zone)
	register zone_t	zone;
{
	register vm_offset_t	addr;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(ZALLOC_LOG, caller, zone->elem_size);
	}
#endif

	if (zone == ZONE_NULL)
		panic ("zalloc: null zone");

	lock_zone(zone);
	REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
	while (addr == 0) {
                if ( zone == vm_map_kentry_zone )
                        panic("zalloc: no more kernel map entries\n");

		/*
 		 *	If nothing was there, try to get more
		 */
		if (zone->doing_alloc) {
			/*
			 *	Someone is allocating memory for this zone.
			 *	Wait for it to show up, then try again.
			 */
			assert_wait((vm_offset_t)&zone->doing_alloc, TRUE);
			/* XXX say wakeup needed */
			unlock_zone(zone);
			thread_block();
			lock_zone(zone);
		}
		else {
			if ((zone->cur_size + (zone->pageable ?
				zone->alloc_size : zone->elem_size)) >
			    zone->max_size) {
				if (zone->exhaustible)
					break;
				/*
				 * Printf calls logwakeup, which calls
				 * select_wakeup which will do a zfree
				 * (which tries to take the select_zone
				 * lock... Hang.  Release the lock now
				 * so it can be taken again later.
				 * NOTE: this used to be specific to
				 * the select_zone, but for
				 * cleanliness, we just unlock all
				 * zones before this.
				 */
				if (zone->expandable) {
					/*
					 * This is best used in conjunction
					 * with the collecatable flag. What we
					 * want is an assurance we can get the
					 * memory back, assuming there's no
					 * leak. 
					 */
					zone->max_size += (zone->max_size >> 1);
				} else if (!zone_ignore_overflow) {
					unlock_zone(zone);
					printf("zone \"%s\" empty.\n",
						zone->zone_name);
					panic("zalloc");
				}
			}

			if (zone->pageable)
				zone->doing_alloc = TRUE;
			unlock_zone(zone);

			if (zone->pageable) {
				zcram(zone, kmem_alloc_pageable(zone_map, 
 						        zone->alloc_size), 
				      zone->alloc_size);
				zd_trace_load(alloc,0,zone->alloc_size,1);
				lock_zone(zone);
				zone->doing_alloc = FALSE; 
				/* XXX check before doing this */
				thread_wakeup((vm_offset_t)&zone->doing_alloc);

				REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
			} else  if (zone->collectable) {
				addr = kmem_alloc(zone_map, zone->alloc_size);
				zone_page_init(addr,
					 zone->alloc_size, ZONE_PAGE_USED);
				zd_trace_load(alloc,addr,zone->alloc_size,2);
				zcram(zone, addr, zone->alloc_size);
				lock_zone(zone);
				REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
			} else {
				addr = zget_space(zone->elem_size);
				if (addr == 0)
					panic("zalloc");

				lock_zone(zone);
				zone->count++;
				zone->cur_size += zone->elem_size;
				unlock_zone(zone);
				zone_page_alloc(addr, zone->elem_size);
				return(addr);
			}
		}
	}

	unlock_zone(zone);
	return(addr);
}

/*
 *	zget returns an element from the specified zone
 *	and immediately returns nothing if there is nothing there.
 *
 *	This form should be used when you can not block (like when
 *	processing an interrupt).
 */
vm_offset_t zget(zone)
	register zone_t	zone;
{
	register vm_offset_t	addr;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(ZGET_LOG, caller, zone->elem_size);
	}
#endif
	if (zone == ZONE_NULL)
		panic ("zalloc: null zone");

	lock_zone(zone);
	REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
	unlock_zone(zone);

	return(addr);
}

/*	Free an element back to a zone.
 */
void zfree(zone, elem)
	register zone_t	zone;
	vm_offset_t	elem;
{

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(ZFREE_LOG, caller, zone->elem_size);
	}
#endif
	lock_zone(zone);
	ADD_TO_ZONE(zone, elem);
	unlock_zone(zone);
}


/*	Change a zone's flags.
 *	This routine must be called immediately after zinit.
 */
void zchange(zone, pageable, sleepable, exhaustible, collectable)
	zone_t		zone;
	boolean_t	pageable;
	boolean_t	sleepable;
	boolean_t	exhaustible;
	boolean_t	collectable;
{
	zone->pageable = pageable;
	zone->sleepable = sleepable;
	zone->exhaustible = exhaustible;
	zone->collectable = collectable;
	lock_zone_init(zone);
}


/* Zone garbage collection subroutines
 *
 *  These routines have in common the modification of entries in the
 *  zone_page_table.  The latter contains one entry for every page
 *  in the zone_map.  
 *
 *  For each page table entry in the given range:
 *
 *	zone_page_in_use        - decrements in_free_list
 *	zone_page_free          - increments in_free_list
 *	zone_page_init          - initializes in_free_list and alloc_count
 *	zone_page_alloc         - increments alloc_count
 *	zone_page_dealloc       - decrements alloc_count
 *	zone_add_free_page_list - adds the page to the free list
 *   
 *  Two counts are maintained for each page, the in_free_list count and
 *  alloc_count.  The alloc_count is how many zone elements have been
 *  allocated from a page.  (Note that the page could contain elements
 *  that span page boundaries.  The count includes these elements so
 *  one element may be counted in two pages.) In_free_list is a count
 *  of how many zone elements are currently free.  If in_free_list is
 *  equal to z.alloc_count then the page is eligible for garbage
 *  collection.
 *
 *  Alloc_count and in_free_list are initialized to the correct values
 *  for a particular zone when a page is zcram'ed into a zone.  Subsequent
 *  gets and frees of zone elements will call zone_page_in_use and 
 *  zone_page_free which modify the in_free_list count.  When the zones
 *  garbage collector runs it will walk through a zones free element list,
 *  remove the elements that reside on collectable pages, and use 
 *  zone_add_free_page_list to create a list of pages to be collected.
 */

void zone_page_in_use(addr, size)
vm_offset_t	addr;
vm_size_t	size;
{
	int i, j;
	if ((addr < zone_map_min_address) ||
	    (addr+size > zone_map_max_address)) return;
	i = atop(addr-zone_map_min_address);
	j = atop((addr+size-1) - zone_map_min_address);
	lock_zone_page_table();
	for (; i <= j; i++) {
		zone_page_table[i].z.in_free_list--;
	}
	unlock_zone_page_table();
}

void zone_page_free(addr, size)
vm_offset_t	addr;
vm_size_t	size;
{
	int i, j;
	if ((addr < zone_map_min_address) ||
	    (addr+size > zone_map_max_address)) return;
	i = atop(addr-zone_map_min_address);
	j = atop((addr+size-1) - zone_map_min_address);
	lock_zone_page_table();
	for (; i <= j; i++) {
		/* Set in_free_list to (ZONE_PAGE_USED + 1) if
		 * it was previously set to ZONE_PAGE_UNUSED.
		 */
		if (zone_page_table[i].z.in_free_list == ZONE_PAGE_UNUSED) {
			zone_page_table[i].z.in_free_list = 1;
		} else {
			zone_page_table[i].z.in_free_list++;
		}
	}
	unlock_zone_page_table();
}

void zone_page_init(addr, size, value)
vm_offset_t	addr;
vm_size_t	size;
int		value;
{
	int i, j;
	if ((addr < zone_map_min_address) ||
	    (addr+size > zone_map_max_address)) return;
	i = atop(addr-zone_map_min_address);
	j = atop((addr+size-1) - zone_map_min_address);
	lock_zone_page_table();
	for (; i <= j; i++) {
		zone_page_table[i].z.alloc_count = value;
		zone_page_table[i].z.in_free_list = 0;
	}
	unlock_zone_page_table();
}

void zone_page_alloc(addr, size)
vm_offset_t	addr;
vm_size_t	size;
{
	int i, j;
	if ((addr < zone_map_min_address) ||
	    (addr+size > zone_map_max_address)) return;
	i = atop(addr-zone_map_min_address);
	j = atop((addr+size-1) - zone_map_min_address);
	lock_zone_page_table();
	for (; i <= j; i++) {
		/* Set z.alloc_count to (ZONE_PAGE_USED + 1) if
		 * it was previously set to ZONE_PAGE_UNUSED.
		 */
		if (zone_page_table[i].z.alloc_count == ZONE_PAGE_UNUSED) {
			zone_page_table[i].z.alloc_count = 1;
		} else {
			zone_page_table[i].z.alloc_count++;
		}
	}
	unlock_zone_page_table();
}

void zone_page_dealloc(addr, size)
vm_offset_t	addr;
vm_size_t	size;
{
	int i, j;
	if ((addr < zone_map_min_address) ||
	    (addr+size > zone_map_max_address)) return;
	i = atop(addr-zone_map_min_address);
	j = atop((addr+size-1) - zone_map_min_address);
	lock_zone_page_table();
	for (; i <= j; i++) {
		zone_page_table[i].z.alloc_count--;
	}
	unlock_zone_page_table();
}

void
zone_add_free_page_list(free_list, addr, size)
	union zone_page_table_entry	**free_list;
	vm_offset_t	addr;
	vm_size_t	size;
{
	int i, j;
	if ((addr < zone_map_min_address) ||
	    (addr+size > zone_map_max_address)) return;

	i = atop(addr-zone_map_min_address);
	j = atop((addr+size-1) - zone_map_min_address);

	lock_zone_page_table();

	for (; i <= j; i++) {
		if (zone_page_table[i].z.alloc_count == 0) {
			zone_page_table[i].next = *free_list;
			*free_list = &zone_page_table[i];
		}
	}
	unlock_zone_page_table();
}


/* This is used for walking through a zone's free element list.
 */
struct zone_free_entry {
	struct zone_free_entry * next;
};

/*	Zone garbage collection
 *
 *	zone_gc will walk through all the free elements in all the
 *	zones that are marked collectable looking for reclaimable
 *	pages.  Zone_gc is called by swapout_scan when the system
 *	begins to run out of memory.
 */
void
zone_gc() 
{
	int		max_zones;
	zone_t		z;
	int		i;
	union zone_page_table_entry	*freep, *nextfreep;
	union zone_page_table_entry	*zone_free_page_list;

	simple_lock(&all_zones_lock);
	max_zones = num_zones;
	z = first_zone;
	simple_unlock(&all_zones_lock);

	zone_free_page_list = (union zone_page_table_entry *) 0;

	for (i = 0; i < max_zones; i++) {
		struct zone_free_entry * last;
		struct zone_free_entry * elt;
		assert(z != ZONE_NULL);
		lock_zone(z);

		if (!z->pageable && z->collectable) {

		    /* Count the free elements in each page.  This loop
		     * requires that all in_free_list entries are zero.
		     */
		    elt = (struct zone_free_entry *)(z->free_elements);
		    while ((elt != (struct zone_free_entry *)0)) {
			   zone_page_free(elt, z->elem_size);
			   elt = elt->next;
		    }

		    /* Now determine which elements should be removed
		     * from the free list and, after all the elements
		     * on a page have been removed, add the element's
		     * page to a list of pages to be freed.
		     */
		    elt = (struct zone_free_entry *)(z->free_elements);
		    last = elt;
		    while ((elt != (struct zone_free_entry *)0)) {
			if (((vm_offset_t)elt>=zone_map_min_address)&&
			    ((vm_offset_t)elt<=zone_map_max_address)&&
			    (zone_page(elt)->z.in_free_list ==
			     zone_page(elt)->z.alloc_count)) {

			    z->cur_size -= z->elem_size;
			    zone_page_in_use(elt, z->elem_size);
			    zone_page_dealloc(elt, z->elem_size);
			    if (zone_page(elt)->z.alloc_count == 0 ||
			      zone_page(elt+(z->elem_size-1))->z.alloc_count==0) {
				    zone_add_free_page_list(
						    &zone_free_page_list, 
						    elt, z->elem_size);
			    }


			    if (elt == last) {
				elt = elt->next;
				z->free_elements =(vm_offset_t)elt;
				last = elt;
			    } else {
				last->next = elt->next;
				elt = elt->next;
			    }
			} else {
			    /* This element is not eligible for collection
			     * so clear in_free_list in preparation for a
			     * subsequent garbage collection pass.
			     */
			    if (((vm_offset_t)elt>=zone_map_min_address)&&
				((vm_offset_t)elt<=zone_map_max_address)) {
				zone_page(elt)->z.in_free_list = 0;
			    }
			    last = elt;
			    elt = elt->next;
			}
		    }
		}
		unlock_zone(z);		
		simple_lock(&all_zones_lock);
		z = z->next_zone;
		simple_unlock(&all_zones_lock);
	}



	for (freep = zone_free_page_list; freep != 0; freep = nextfreep) {
		vm_offset_t	free_addr;
		
		nextfreep = freep->next;   /* need to save BEFORE freeing */
		free_addr = zone_map_min_address + 
		PAGE_SIZE * (freep - zone_page_table);
		kmem_free(zone_map, free_addr, PAGE_SIZE);
		zd_trace_load(gc,free_addr,PAGE_SIZE,-1);

	}

}


#if	MACH_DEBUG
#include <mach/kern_return.h>
#include <mach/machine/vm_types.h>
#include <mach_debug/zone_info.h>
#include <kern/ipc_globals.h>
#include <vm/vm_user.h>
#include <vm/vm_map.h>

kern_return_t host_zone_info(task, names, namesCnt, info, infoCnt)
	task_t		task;
	zone_name_array_t *names;
	unsigned int	*namesCnt;
	zone_info_array_t *info;
	unsigned int	*infoCnt;
{
	int		max_zones;
	vm_offset_t	addr1, addr2;
	vm_size_t	size1, size2;
	zone_t		z;
	zone_name_t	*zn;
	zone_info_t	*zi;
	int		i;
	kern_return_t	kr;

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	We assume that zones aren't freed once allocated.
	 *	We won't pick up any zones that are allocated later.
	 */

	simple_lock(&all_zones_lock);
	max_zones = num_zones;
	z = first_zone;
	simple_unlock(&all_zones_lock);

	assert(max_zones >= 0);
	*namesCnt = max_zones;
	*infoCnt = max_zones;

	if (max_zones == 0) {
		*names = 0;
		*info = 0;
		return KERN_SUCCESS;
	}

	size1 = round_page(max_zones * sizeof(zone_name_t));
	size2 = round_page(max_zones * sizeof(zone_info_t));

	/*
	 *	Allocate memory in the ipc_kernel_map, because
	 *	we need to touch it, and then move it to the ipc_soft_map
	 *	(where the IPC code expects to find it) when we're done.
	 *
	 *	Because we don't touch the memory with any locks held,
	 *	it can be left pageable.
	 */

	kr = vm_allocate(ipc_kernel_map, &addr1, size1, TRUE);
	if (kr != KERN_SUCCESS)
		panic("host_zone_info: vm_allocate");

	kr = vm_allocate(ipc_kernel_map, &addr2, size2, TRUE);
	if (kr != KERN_SUCCESS)
		panic("host_zone_info: vm_allocate");

	zn = (zone_name_t *) addr1;
	zi = (zone_info_t *) addr2;

	for (i = 0; i < max_zones; i++, zn++, zi++) {
		struct zone zcopy;

		assert(z != ZONE_NULL);

		lock_zone(z);
		zcopy = *z;
		unlock_zone(z);

		simple_lock(&all_zones_lock);
		z = z->next_zone;
		simple_unlock(&all_zones_lock);

		/* assuming here the name data is static */
		(void) strncpy(zn->name, zcopy.zone_name, sizeof zn->name);

		zi->count = zcopy.count;
		zi->cur_size = zcopy.cur_size;
		zi->max_size = zcopy.max_size;
		zi->elem_size = zcopy.elem_size;
		zi->alloc_size = zcopy.alloc_size;
		zi->pageable = zcopy.pageable;
		zi->sleepable = zcopy.sleepable;
		zi->exhaustible = zcopy.exhaustible;
	}

	/*
	 *	Move memory to ipc_soft_map, and free unused memory.
	 */

	kr = vm_map_copyin(ipc_kernel_map, addr1,
		     size1, TRUE,
		     (vm_map_copy_t *) names);
	if (kr != KERN_SUCCESS)
		panic("host_zone_info: vm_map_copyin");

	kr = vm_map_copyin(ipc_kernel_map, addr2,
		     size2, TRUE,
		     (vm_map_copy_t *) info);
	if (kr != KERN_SUCCESS)
		panic("host_zone_info: vm_map_copyin");

	return KERN_SUCCESS;
}
#endif	/* MACH_DEBUG */

