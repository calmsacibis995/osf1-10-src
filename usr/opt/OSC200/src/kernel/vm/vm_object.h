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
 *	@(#)$RCSfile: vm_object.h,v $ $Revision: 4.2.17.3 $ (DEC) $Date: 1993/09/22 14:16:31 $
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
 *	File:	vm_object.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory object module definitions.
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */
#ifndef	__VM_OBJECT__
#define	__VM_OBJECT__ 1
#include <sys/param.h>
#include <sys/types.h>
#include <sys/unix_defs.h>
#include <vm/pmap.h>


#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/memory_object.h>
#include <mach/port.h>
#include <mach/machine/vm_types.h>
#include <kern/queue.h>
#include <kern/lock.h>
#include <kern/assert.h>
#include <kern/macro_help.h>



/*
 *	The common part of all VM objects
 *
 */

struct vm_object {
	struct vm_page  *ob_memq;	/* Resident memory */
	int 	ob_resident_pages;	/* Page in ob_memq list */
	udecl_simple_lock_data (,ob_lock)/* Simple lock to protect common */
	struct vm_object_ops *ob_ops;	/* Object Operations */
	int	ob_ref_count;		/* Number of references */
	int 	ob_res_count;		/* Object resident count */
	vm_size_t ob_size;		/* Size of object */
	unsigned short ob_flags;	/* Flags */
	unsigned char ob_type;		/* Object type */
};

typedef struct vm_object * vm_object_t;
#define VM_OBJECT_NULL		((vm_object_t) 0)


/*
 * Flags field of object
 */

#define	OB_SWAPON	0x01		/* Object swapping enabled */
#define	OB_SWAP		0x02		/* Object is being swapped */
#define	OB_SWAPWAIT	0x04		/* Waiting for swap to complete */
#define	OB_CHANGE	0x08		/* Object change taking place */
#define	OB_CHANGEWAIT	0x10		/* Waiting for change to complete */
#define	OB_SEMWAIT	0x20		/* Semaphore wait */


/*
 * Object address plus this offset for wakeup address
 */

#define	OB_WSWAP	0x0
#define	OB_WCHANGE	0x1
#define	OB_WSEM		0x2

#define	vm_object_wait(OBJ,WHY) {					\
	while (((vm_object_t)(OBJ))->ob_flags & OB_/**/WHY) {		\
		assert_wait(vm_object_sleep_addr(OBJ,WHY), FALSE);	\
		((vm_object_t)(OBJ))->ob_flags |= OB_/**/WHY/**/WAIT;	\
		vm_object_unlock((vm_object_t)(OBJ));			\
		thread_block();						\
		vm_object_lock((vm_object_t)(OBJ));			\
	}								\
}

#define	vm_object_assert_wait(OBJ,WHY,INTERRUPT) {			\
	assert_wait(vm_object_sleep_addr(OBJ,WHY), INTERRUPT);		\
	((vm_object_t)(OBJ))->ob_flags |= OB_/**/WHY/**/WAIT;		\
}

#define	vm_object_sleep_addr(OBJ,WHY)					\
	((vm_offset_t)(OBJ) + OB_W/**/WHY)

#define	vm_object_wakeup(OBJ,WHY)					\
	if (((vm_object_t)(OBJ))->ob_flags & OB_/**/WHY/**/WAIT) {	\
		thread_wakeup(vm_object_sleep_addr(OBJ, WHY));		\
		((vm_object_t) (OBJ))->ob_flags ^= OB_/**/WHY/**/WAIT;	\
	}

/*
 * Object control operations used for control functions.
 */

typedef enum {
	VMOC_NOOP, 			/* No operation */
	VMOC_MSYNC,			/* Sync memory cache with backstore */
	VMOC_PAGEUNLOCK,		/* Unlock the page */
	VMOC_FREE			/* Free the pages */
	} vm_ocontrol_t;

/*
 * Object page control operations
 */

#define	VMOP_RELEASE	0x01		/* Release a hold on the page */
#define	VMOP_LOCK	0x02		/* Lock the page */

/*
 * Object operations called by map map entry fault handles when there
 * is a backing object.  Without a backing object, the handle should 
 * know how to call the call the object management functions explictly.
 * To avoid polluting the kernel with numerous common operations handles
 * when object(s) don't require a handle or any special action, the vm_object.c
 * module exports routines with the naming convention:
 *
 *	vm_ops_##name##def
 */

struct vm_object_ops {
	boolean_t (*ops_klock_try)(/* vm_object_t obj, vm_offset_t offset */);
	int (*ops_unklock)(/* vm_object_t obj, vm_offset_t offset */);
	int (*ops_reference)(/* vm_object_t obj */);
	int (*ops_deallocate)(/* vm_object_t obj */);
	int (*ops_pagein)(/* vm_object_t obj, vm_offset_t offset, 
				vm_size_t size,
				struct vm_map_entry *ep, vm_offset_t addr, 
				vm_size_t psize, 
				struct vm_page *page, vm_prot_t *fault_type */);
	int (*ops_pageout)(/* vm_object_t object, vm_offset_t offset,
				vm_size_t size */);
	int (*ops_swap)(/* vm_object_t obj, int *pages */);
	int (*ops_control)(/* vm_object_t object, vm_offset_t offset,
				vm_size_t size, 
				vm_ocontrol_t control, int flag */);
	int (*ops_pagectl)(/* vm_object_t object, struct vm_page *pp, 
				int pcnt, int flags */);
	int (*ops_pagesteal)(/* struct vm_page *pp */);
};
				
#define	OOP_KLOCK_TRY_K 						\
	(boolean_t (*)(/* vm_object_t obj, vm_offset_t offset */))

#define	OOP_UNKLOCK_K							\
	(int (*)(/* vm_object_t obj, vm_offset_t offset */))

#define	OOP_REFERENCE_K							\
	(int (*)(/* vm_object_t obj */))

#define	OOP_DEALLOCATE_K						\
	(int (*)(/* vm_object_t obj */))

#define OOP_PAGEIN_K							\
	(int (*)(/* vm_object_t obj, vm_offset_t offset, vm_size_t size,\
		struct vm_map_entry *ep, vm_offset_t addr, 		\
		vm_size_t psize, struct vm_page *page, 			\
		vm_prot_t *fault_type */))

#define	OOP_PAGEOUT_K							\
	(int (*)(/* vm_object_t object, vm_offset_t offset, vm_size_t size */))

#define	OOP_SWAP_K							\
	(int (*)(/* vm_object_t obj, int *pages */))

#define	OOP_CONTROL_K							\
	(int (*)(/* vm_object_t object, vm_offset_t offset,		\
		vm_size_t size, vm_ocontrol_t control, int flag */))

#define	OOP_PAGECTL_K							\
	(int (*)(/* vm_object_t object, struct vm_page *pp, 		\
		int pcnt, int flags */))

#define OOP_PAGESTEAL_K							\
	(int (*)(/* struct vm_page *pp */))


#define	OOP_KLOCK_TRY(OBJ, OFFSET)					\
	(*((vm_object_t)(OBJ))->ob_ops->ops_klock_try)((OBJ), (OFFSET))

#define OOP_UNKLOCK(OBJ, OFFSET)					\
	(*((vm_object_t)(OBJ))->ob_ops->ops_unklock)((OBJ), (OFFSET))

#define	OOP_REFERENCE(OBJ)						\
	(*((vm_object_t)(OBJ))->ob_ops->ops_reference)((OBJ))

#define OOP_DEALLOCATE(OBJ)						\
	(*((vm_object_t)(OBJ))->ob_ops->ops_deallocate)((OBJ))

#define	OOP_PAGEIN(OBJ, OFFSET, SIZE, VME, ADDR, PSIZE, PGLIST, PROT)	\
	(*((vm_object_t)(OBJ))->ob_ops->ops_pagein)			\
	((OBJ), (OFFSET), (SIZE), (VME), (ADDR), (PSIZE), (PGLIST), (PROT))

#define OOP_PAGEOUT(OBJ, OFFSET, SIZE)					\
	(*((vm_object_t)(OBJ))->ob_ops->ops_pageout)			\
	((OBJ), (OFFSET), (SIZE))

#define	OOP_SWAP(OBJ,PAGES)						\
	(*((vm_object_t)(OBJ))->ob_ops->ops_swap)((OBJ), (PAGES))

#define	OOP_CONTROL(OBJ, OFFSET, SIZE, CONTROL, FLAG)			\
	(*((vm_object_t)(OBJ))->ob_ops->ops_control)			\
		((OBJ), (OFFSET), (SIZE), (CONTROL), (FLAG))
	
#define OOP_PAGECTL(OBJ, PLIST, PLS, FLAGS)				\
	(*((vm_object_t)(OBJ))->ob_ops->ops_pagectl)			\
		((OBJ), (PLIST), (PLS), (FLAGS))

#define OOP_PAGESTEAL(OBJ, PP)						\
	(*((vm_object_t)(OBJ))->ob_ops->ops_pagesteal)((OBJ), (PP))	


/*
 * A small amount of informationn is exported to the module
 * vm_object.c by each object type.  The is done thru an
 * object configuration structure.
 */

struct vm_object_config {
						/* 
						 * object private allocation 
						 * this is optional
						 */
	int 	(*oc_allocate)(/* vm_object_t obj, vm_size_t size */);
						/* object termination 
						 * this is optional
						 * ob_ref_count == 1
						 * the termination handle
						 * does the actual free of
						 * the object structure.
						 */
	int	(*oc_terminate)(/* vm_object_t obj */);
	vm_size_t oc_osize;			/* Size of object struct */
	struct vm_object_ops *oc_ops;		/* pointer to op handles */
	struct vm_map_entry_ops *oc_mape;	/* map entry operations */
};


/*
 * Object types
 */

#define	OT_NULL		0x0			/* No actions supported */
#define	OT_VP		0x1			/* File sytem shared vnode */
#define	OT_VPMAP	0x2			/* mmaper of vnode */
#define	OT_ANON		0x3			/* Anonymous memory */
#define	OT_SWAP		0x4			/* Swap object */
#define	OT_DEVMAP	0x5			/* Memory mapped device */
#define	OT_KERNEL	0x6			/* Non-paged kernel */
#define	OT_PKERNEL	0x7			/* Pageable kernel */
#define OT_SHM          0x8                     /* System V shared memory */
#define	OT_SEG		0x9			/* Segment address space */
#define	OT_LAST		0xa			/* Maximum */

#define	vm_object_type(OBJ)	(OBJ)->ob_type
#define ismmaper(entry)							\
	(vm_object_type(entry->vme_object) == OT_VP)                 

#ifdef	KERNEL

/*
 *	Object locking macros for object
 *	common part (with and without debugging).
 */

#if	VM_OBJECT_DEBUG

#define vm_object_lock_init(OBJECT) {					\
		simple_lock_init(&((vm_object_t)(OBJECT))->ob_lock); 	\
		((vm_object_t)(OBJECT))->ob_lockholder = 		\
					(vm_offset_t) 0; 		\
}

#define vm_object_lock(OBJECT) {					\
		simple_lock(&((vm_object_t)(OBJECT))->ob_lock); 	\
		((vm_object_t)(OBJECT))->ob_lockholder = 		\
				(vm_offset_t) current_thread(); 	\
}

#define vm_object_unlock(OBJECT) {					\
		if (((vm_object_t)(OBJECT))->ob_lockholder != 		\
			(vm_offset_t) current_thread()) 		\
		vm_object_disaster((OBJECT));				\
		((vm_object_t)(OBJECT))->ob_lockholder 			\
					= (vm_offset_t) 0;		\
		simple_unlock(&((vm_object_t)(OBJECT))->ob_lock);	\
}

#define vm_object_lock_try(OBJECT)					\
		(simple_lock_try(&((vm_object_t)(OBJECT))->ob_lock) ? 	\
		( (((vm_object_t)(OBJECT))->ob_lockholder = 		\
		(vm_offset_t) current_thread()) , TRUE) : FALSE)

#else	/*VM_OBJECT_DEBUG*/

#define vm_object_lock_init(OBJECT)					\
	simple_lock_init(&((vm_object_t)(OBJECT))->ob_lock)
#define vm_object_lock(OBJECT)						\
	simple_lock(&((vm_object_t)(OBJECT))->ob_lock)
#define vm_object_unlock(OBJECT)					\
	simple_unlock(&((vm_object_t)(OBJECT))->ob_lock)
#define vm_object_lock_try(OBJECT)					\
	simple_lock_try(&((vm_object_t)(OBJECT))->ob_lock)

#endif	/* VM_OBJECT_DEBUG */


/*
 * Objects registered for swap out
 */

extern vm_object_t vm_object_swap;


extern kern_return_t vm_object_allocate(/* char type, vm_size_t size, 
	caddr_t private, vm_object_t *robj */);
extern int vm_ops_reference_def(/* vm_object_t obj */);
extern int vm_ops_deallocate_def(/* vm_object_t obj */);
extern int vm_ops_pagein_def(/* vm_object_t obj, 
	vm_offset_t offset, vm_size_t size, int *protobug, 
	vm_offset_t addr, vm_size_t psize, 
	struct vm_page **page, vm_prot_t *fault_type */);
extern int vm_ops_pmap_protect_def(/* vm_object_t obj, 
	vm_offset_t offset, vm_size_t size, pmap_t pmap, 
	vm_offset_t vaddr, vm_prot_t prot */);
extern int vm_ops_swap_def(/* vm_object_t obj, int *pages */);
extern int vm_ops_pageout_def(/* vm_object_t object, 
	vm_offset_t offset, vm_size_t size */);
extern void vm_object_free(/* vm_object_t obj */);

/*
 * Old functions now implemented as macros
 */

#define vm_object_reference(OBJ) 	OOP_REFERENCE(OBJ)
#define vm_object_deallocate(OBJ)	OOP_DEALLOCATE(OBJ)
#define vm_object_swapon(OBJ) 					\
	((vm_object_t)(OBJ))->ob_flags |= OB_SWAPON

extern struct vm_object_config *vm_object_config[];
#define vm_object_to_vmeops(OBJ)				\
	vm_object_config[((vm_object_t)(OBJ))->ob_type]->oc_mape
	
extern vm_object_t nkernel_object, pkernel_object, kernel_object;

#endif	/* KERNEL */

#endif /* !__VM_OBJECT__ */







