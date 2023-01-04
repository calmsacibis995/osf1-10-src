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
 *	@(#)$RCSfile: vm_map.h,v $ $Revision: 4.2.17.5 $ (DEC) $Date: 1993/11/23 16:33:00 $
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
 *	File:	vm/vm_map.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory map module definitions.
 *
 * Contributors:
 *	avie, dlb, mwyoung
 */

#ifndef	_VM_VM_MAP_H_
#define _VM_VM_MAP_H_

#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <vm/pmap.h>
#include <vm/vm_object.h>
#include <vm/vm_fault.h>
#include <vm/vm_control.h>
#include <vm/vpage.h>
#include <kern/lock.h>
#include <kern/macro_help.h>

/*
 *	Types defined:
 *
 *	vm_map_t		the high-level address map data structure.
 *	vm_map_entry_t		an entry in an address map.
 *	vm_map_copy_t		represents memory copied from an address map,
 *				 used for inter-map copy operations
 */

typedef union vm_map_object {
	struct vm_object	*vm_object;	/* object object */
	struct vm_map		*sub_map;	/* belongs to another map */
} vm_map_object_t;

struct vm_map_links {
	struct vm_map_entry	*prev;		/* previous entry */
	struct vm_map_entry	*next;		/* next entry */
	vm_offset_t		start;		/* start address */
	vm_offset_t		end;		/* end address */
};

struct vm_map_entry {
	struct vm_map_links	vme_links;	/* links to other entries */
	struct vm_map		*vme_map;	/* map which owns us */
	union vm_map_object	vme_uobject;	/* object I point to */
	union {
		vm_offset_t	tvme_offset;	/* offset into object */
		struct vm_seg	*tvme_seg;	/* seg mape points to */
	} vmet;
	struct vm_map_entry_ops *vme_ops;	/* Map entry operations */
	struct	vpage		vme_vpage;	/* Virtual page information */
						/* Fault locking */
	udecl_simple_lock_data	(, vme_faultlock)
	union {
		struct {			/* User map entry info */
			
						/* Immutable reasons */
			unsigned int
				uvme_anchor : 16,
				uvme_mutate : 1,
				uvme_keep_on_exec : 1,
				uvme_inheritance : 1,
				uvme_maxprot : 3,
					: 10;
		} uvme;
		struct {
			unsigned int
				kvme_faults : 8,
				kvme_want : 8,
				kvme_is_sub_map : 1,
				kvme_copymap : 1,
					: 14;
		} kvme;
	} vmeu;
	vm_offset_t		vme_private;	/* Private data for map */
};

typedef struct vm_map_entry	*vm_map_entry_t;

#define	vme_offset		vmet.tvme_offset
#define	vme_seg			vmet.tvme_seg
#define	vme_anchor		vmeu.uvme.uvme_anchor
#define	vme_mutate		vmeu.uvme.uvme_mutate
#define	vme_keep_on_exec	vmeu.uvme.uvme_keep_on_exec
#define	vme_inheritance		vmeu.uvme.uvme_inheritance
#define vme_maxprot		vmeu.uvme.uvme_maxprot
#define vme_faults		vmeu.kvme.kvme_faults
#define	vme_want		vmeu.kvme.kvme_want
#define	vme_is_sub_map		vmeu.kvme.kvme_is_sub_map
#define vme_copymap		vmeu.kvme.kvme_copymap
#define vme_is_submap		vmeu.kvme.kvme_is_sub_map
#define vme_protection		vme_vpage.vp_prot
#define	vme_kwire		vme_vpage.vp_kwire
#define	vme_plock		vme_vpage.vp_plock
#define vme_prev		vme_links.prev
#define vme_next		vme_links.next
#define vme_start		vme_links.start
#define vme_end			vme_links.end
#define	vme_object		vme_uobject.vm_object
#define	vme_submap		vme_uobject.sub_map

#define VM_MAP_ENTRY_NULL	((vm_map_entry_t) 0)

struct vm_map {
	struct vm_map_links	vm_links;	/* links to the entries */
	int			vm_nentries;	/* Number of vme entries */
	unsigned int 				/* Main or submap */
				vm_is_mainmap :1,
				vm_copy_map:1,	/* kernel copy submap */
						/* kernel map entries */
				vm_entries_pageable:1, 
						/* wait for space */
				vm_wait_for_space:1,
				vm_umap:1,	/* user space map */
				:27;
	struct vm_map_ops	*vm_ops;	/* Operations on addr space */
	lock_data_t		vm_lock;	/* Lock for map data */
	vm_size_t		vm_size;	/* virtual size */
	pmap_t			vm_pmap;	/* Physical map */
	int			vm_ref_count;	/* Reference count */
	udecl_simple_lock_data(,vm_ref_lock)	/* Lock for vm_ref_count */
	vm_map_entry_t		vm_hint;	/* hint for quick lookups */
	udecl_simple_lock_data(,vm_hint_lock)	/* lock for vm_hint storage */
	vm_map_entry_t		vm_first_free;	/* First free space hint */
	vm_offset_t		vm_private;	/* Map private information */
        int                     vm_res_count;   /* Map resident count */
        unsigned                vm_fault_rate;  /* Pagefaults over time */
        int                     vm_pagefaults;  /* Accumulated pagefault */
        unsigned                vm_faultrate_time; /* Time of last vm_fault_rate update */
	int			vm_color_bucket; /* base color bucker for address space */
};

typedef	struct vm_map * vm_map_t;

#define		VM_MAP_NULL	((vm_map_t) 0)

#define vm_min_offset		vm_links.start	/* start of range */
#define vm_max_offset		vm_links.end	/* end of range */
#define	vm_next			vm_links.next
#define	vm_prev			vm_links.prev	

#define vm_map_to_entry(map)	((struct vm_map_entry *) &(map)->vm_links)
#define vm_map_first_entry(map)	((map)->vm_links.next)
#define vm_map_last_entry(map)	((map)->vm_links.prev)


typedef struct vm_map_copy {
	struct vm_map_links	vm_links;
	unsigned int		vm_nentries;
	unsigned int 				/* Main or submap */
				vm_is_mainmap :1,
				vm_copy_map:1,	/* kernel copy submap */
						/* kernel map entries */
				vm_entries_pageable:1, 
						/* wait for space */
				vm_wait_for_space:1,
				vm_umap:1,	/* user space map */
				:27;
} *vm_map_copy_t;

#define	VM_MAP_COPY_NULL	((vm_map_copy_t) 0)

#define vm_map_copy_to_entry(map)		\
		((struct vm_map_entry *) &(map)->vm_links)
#define vm_map_copy_first_entry(map)		\
		((map)->vm_links.next)
#define vm_map_copy_last_entry(map)		\
		((map)->vm_links.prev)

/*
 * Map entry space grow
 */

typedef enum {AS_GROWANY, AS_GROWUP, AS_GROWDOWN}	as_grow_t;

/*
 * Dup has to be told the type of copy operation
 * in order to support the Mach like inheritance model.
 */

typedef enum {
	VM_COPYU,				/* UNIX copy technology */
	VM_COPYMSHARE,				/* Mach share */
	VM_COPYMCOPY,				/* Mach copy */
	VM_COPYMNONE,				/* Mach none */
	VM_COPYMDONATE				/* Mach donate */
	} vm_copy_t;

/*
 * Map entry copy operations
 */

typedef	enum {
	VME_COPYLOAD,				/* Copy entry was loaded */
	VME_COPYFREE				/* Free a copy map entry */
	} vm_copy_op_t;

struct vm_map_entry_ops {
	int (*me_fault)(/* vm_map_entry_t vme, vm_offset_t vaddr, 
				vm_size_t size, vm_prot_t fault_type, 
				vm_fault_t wire, struct vm_page **pp */);
	int (*me_dup)(/* vm_map_entry_t vme, vm_offset_t start, vm_size_t size,
				vm_map_entry_t newentry, vm_copy_t copy */);
	int (*me_unmap)(/* vm_map_entry_t vme, vm_offset_t vaddr, 
				vm_size_t size */);
	int (*me_msync)(/* vm_map_entry_t vme, vm_offset_t vaddr, 
				vm_size_t size, int flags */);
	int (*me_lockop)(/* vm_map_entry_t vme, vm_offset_t vaddr, 
				vm_size_t size, vm_fault_t wire */);
	int (*me_swap)(/* vm_map_entry_t vme, int rw */);
	int (*me_core)(/* vm_map_t vme, unsigned int pg, char *vec, int *sz */);
	int (*me_control)(/* vm_map_t vme, vm_offset_t vaddr, vm_size_t size, 
				vm_control_t control, int arg */);
	int (*me_protect)(/* vm_map_entry_t vme, vm_offset_t vaddr, 
				vm_size_t size, vm_prot_t prot */);
	int (*me_check_protect)(/* vm_map_entry_t vme, vm_offset_t vaddr,
				vm_size_t size, vm_prot_t prot */);
	int (*me_kluster)(/* vm_map_entry_t vme, vm_offset_t addr, int pcnt,
			vm_offset_t *back, vm_offset_t *forward */);
	int (*me_copy)(/* vm_map_entry_t vme, vm_copy_op_t op */);
	int (*me_grow)(/* vm_map_entry_t vme, vm_prot_t prot,
				vm_size_t increase, as_grow_t direction */);
};

typedef struct vm_map_entry_ops * vm_map_entry_ops_t;

#define vme_fault		vme_ops->me_fault
#define vme_dup			vme_ops->me_dup
#define	vme_unmap		vme_ops->me_unmap
#define vme_msync		vme_ops->me_msync
#define vme_lockop		vme_ops->me_lockop
#define vme_swap		vme_ops->me_swap
#define vme_core		vme_ops->me_core
#define vme_control		vme_ops->me_control
#define vme_protect		vme_ops->me_protect
#define vme_check_protect	vme_ops->me_check_protect
#define vme_kluster		vme_ops->me_kluster
#define	vme_copy		vme_ops->me_copy
#define	vme_grow		vme_ops->me_grow



/*
 * Map specific operations
 */

struct vm_map_ops {
	int (*mo_deallocate)(/* vm_map_t map */);
	int (*mo_fault)(/* vm_map_t map, vm_offset_t addr, 
				vm_prot_t fault_type, vm_fault_t wire */);
	int (*mo_wire)(/* vm_map_t map, vm_offset_t start, 
				vm_offset_t end, vm_prot_t prot */);
	int (*mo_allocate)(/* vm_map_t map, vm_object_t object, 
				vm_offset_t offset, vm_offset_t *addr,
				vm_size_t length, boolean_t find_space */);
	int (*mo_map_enter)(/* vm_map_t map, vm_offset_t *address, 
			vm_size_t size, vm_offset_t mask, boolean_t anywhere,
			vm_object_t object, vm_offset_t offset,
			boolean_t needs_copy, vm_prot_t cur_protection,
			vm_prot_t max_protection, vm_inherit_t inheritance */);
	int (*mo_protect)(/* vm_map_t map, vm_offset_t start, vm_offset_t end,
				vm_prot_t prot, boolean_t set_max */);
	int (*mo_inherit)(/* vm_map_t map, vm_offset_t start, vm_offset_t end,
				vm_inherit_t new_inheritance */);
	int (*mo_keep_on_exec)(/* vm_map_t map, vm_offset_t start, 
				vm_offset_t end, boolean_t new_koe */);
	int (*mo_exec)(/* vm_map_t map, vm_offset_t start, vm_offset_t end */);
	int (*mo_delete)(/* vm_map_t map, vm_offset_t start, 
				vm_offset_t end, boolean_t contain */);
	boolean_t (*mo_check_protection)(/* vm_map_t map, vm_offset_t start,
				vm_offset_t end, vm_prot_t prot */);
	int (*mo_copy_overwrite)(/* vm_map_t map, vm_offset_t dst_addr,
				vm_map_copy_t copy, 
				boolean_t interruptible, vm_size_t size */);
	int (*mo_copyout)(/* vm_map_t map, vm_offset_t dst_addr, 
				vm_map_copy_t copy */);
	int (*mo_copyin)(/* vm_map_t map, vm_offset_t start, vm_size_t len,
				boolean_t src_destroy, 
				vm_map_copy_t *copy_result */);
	int (*mo_fork)(/* vm_map_t oldmap, vm_map_t newmap */);

};

typedef struct vm_map_ops * vm_map_ops_t;

#define	vm_deallocate_map		vm_ops->mo_deallocate
#define vm_fault_map			vm_ops->mo_fault
#define vm_wire_map			vm_ops->mo_wire
#define vm_allocate_map			vm_ops->mo_allocate
#define	vm_enter_map			vm_ops->mo_map_enter
#define vm_protect_map			vm_ops->mo_protect
#define vm_inherit_map			vm_ops->mo_inherit
#define vm_keep_on_exec_map		vm_ops->mo_keep_on_exec
#define	vm_exec_map			vm_ops->mo_exec
#define vm_delete_map			vm_ops->mo_delete
#define vm_check_protection_map		vm_ops->mo_check_protection
#define vm_copy_overwrite_map		vm_ops->mo_copy_overwrite
#define vm_copyout_map			vm_ops->mo_copyout
#define vm_copyin_map			vm_ops->mo_copyin
#define	vm_fork_map			vm_ops->mo_fork


/*
 *	Macros:		vm_map_lock, etc. [internal use only]
 *	Description:
 *		Perform locking on the data portion of a map.
 */

#define vm_map_lock_init(map)			\
MACRO_BEGIN					\
	lock_init(&(map)->vm_lock, TRUE);	\
MACRO_END

#define vm_map_lock(map)			\
MACRO_BEGIN					\
	lock_write(&(map)->vm_lock);		\
MACRO_END

#define vm_map_unlock(map)	lock_write_done(&(map)->vm_lock)
#define vm_map_lock_read(map)	lock_read(&(map)->vm_lock)
#define vm_map_unlock_read(map)	lock_read_done(&(map)->vm_lock)
#define vm_map_lock_write_to_read(map) \
		lock_write_to_read(&(map)->vm_lock)
#define vm_map_lock_read_to_write(map) \
		(lock_read_to_write(&(map)->vm_lock))
#define vm_map_lock_set_recursive(map) \
		lock_set_recursive(&(map)->vm_lock)
#define vm_map_lock_clear_recursive(map) \
		lock_clear_recursive(&(map)->vm_lock)

/*
 *	vm_map_entry_{un,}link:
 *
 *	Insert/remove entries from maps (or map copies).
 */

#define vm_map_entry_link(map, after_where, entry)		\
		MACRO_BEGIN					\
		(map)->vm_nentries++;				\
		(entry)->vme_prev = (after_where);		\
		(entry)->vme_next = (after_where)->vme_next;	\
		(entry)->vme_prev->vme_next =			\
		 (entry)->vme_next->vme_prev = (entry);		\
		MACRO_END

#define vm_map_entry_unlink(map, entry) 			\
		MACRO_BEGIN 					\
		(map)->vm_nentries--; 				\
		(entry)->vme_next->vme_prev = (entry)->vme_prev;\
		(entry)->vme_prev->vme_next = (entry)->vme_next;\
		MACRO_END

/*
 *	SAVE_HINT:
 *
 *	Saves the specified entry as the hint for
 *	future lookups.  Performs necessary interlocks.
 */

#define SAVE_HINT(MAP,VALUE) \
		simple_lock(&(MAP)->vm_hint_lock); \
		(MAP)->vm_hint = (VALUE); \
		simple_unlock(&(MAP)->vm_hint_lock);


extern kern_return_t vm_map_entry_create();

/*
 *	Exported procedures that operate on vm_map_t.
 */

extern void		vm_map_init();		/* Initialize the module */

extern vm_map_t		vm_map_create();	/* Create an empty map */
extern vm_map_t		vm_map_fork();		/* Create a map in the image
						 * of an existing map */

extern void		vm_map_reference();	/* Gain a reference to
						 * an existing map */
extern void		vm_map_deallocate();	/* Lose a reference */

extern kern_return_t	vm_map_enter();		/* Enter a mapping */
extern kern_return_t	vm_map_remove();	/* Deallocate a region */
extern kern_return_t	vm_map_exec();		/* remove all but keep-on-exec
						 * mappings */
extern kern_return_t	vm_map_protect();	/* Change protection */
extern kern_return_t	vm_map_inherit();	/* Change inheritance */
extern kern_return_t	vm_map_keep_on_exec();  /* Change keep-on-exec state */

extern kern_return_t	vm_map_find();		/* Old allocation primitive */
extern void		vm_map_print();		/* Debugging: print a map */

extern vm_size_t	vm_map_actual_size();	/* Actual size of map */

extern void		vm_map_copy_discard();	/* Discard a copy without
						 * using it */
/* Functions for msemaphore  */
extern kern_return_t	vm_msleep(); /* Wait for a semphore to be freed */
extern kern_return_t	vm_mwakeup(); /* Wakeup sleepers on semphore */
extern void		vm_msem_init();	/* Initialization for msem code */


/*
 *	Functions implemented as macros
 */
#define		vm_map_min(map)		((map)->vm_min_offset)
						/* Lowest valid address in
						 * a map */

#define		vm_map_max(map)		((map)->vm_max_offset)
						/* Highest valid address */

#define		vm_map_pmap(map)	((map)->vm_pmap)
						/* Physical map associated
						 * with this address map */
#define		vm_map_vsize(map)	((map)->vm_size)
						/* size of the virtual
						 * address space */
/*
 *	Submap object.  Must be used to create memory to be put
 *	in a submap by vm_map_submap.
 */
extern vm_object_t	vm_submap_object;

#endif	/* !_VM_VM_MAP_H_ */
