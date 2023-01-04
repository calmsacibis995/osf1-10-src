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
 * @(#)$RCSfile: vm_umap.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/07/08 16:39:59 $
 */
#ifndef	__VM_UMAP__
#define	__VM_UMAP__ 1
#include <kern/lock.h>
#include <sys/unix_defs.h>
#include <vm/vm_vlock.h>
#include <sys/param.h>

/*
 * user mode private map structure
 */

struct u_map_private {

	vm_size_t		um_maxvas;	/* Maximum VAS allowed */
	udecl_simple_lock_data	(,um_resource)	/* Protect resource info */
	vm_size_t		um_maxwired;	/* Max wired space allowed */
	vm_size_t		um_wired;	/* Pages wired */
						/* 
						 * For a working set model.
						 * Any other policy leads
						 * to complication when
						 * maintaining these fields.
						 */
	int			um_vpage;	/* vpages allocated */
	vm_size_t		um_maxrss;	/* Maximum rss allowed */
	vm_size_t		um_rss;		/* Resident pages in map */
	unsigned int		
						/* lock all future pages */
				um_lock_future:1,
						/* unloading all address sp */
				um_unload_all:1,
				:30;
	struct vm_vlock		*um_vlock;	/* Virtual space lock by K */
};

/*
 * The vm_mape_faultlock is called by a thread that has
 * the address map write locked.  An anchor means that this
 * entry can't be mutated.  The thread goes to sleep on the 
 * fault lock until the anchor count goes to zero.  
 */

#define	vm_mape_lockaddr(VME) simple_lock_addr((VME)->vme_faultlock)
		
#define vm_mape_faultlock(VME) {					\
	if ((VME)->vme_anchor) {					\
		usimple_lock(&(VME)->vme_faultlock);			\
		if ((VME)->vme_anchor) {				\
			(VME)->vme_mutate = 1;				\
			do {						\
				(void) mpsleep(				\
				(vm_offset_t) (VME),			\
				PZERO, "FAULT", FALSE,			\
				vm_mape_lockaddr(VME),			\
				MS_LOCK_ON_ERROR|MS_LOCK_SIMPLE);	\
			} while ((VME)->vme_anchor);			\
		}							\
		usimple_unlock(&(VME)->vme_faultlock);			\
	}								\
}

#define vm_mape_fault(VME) {						\
	usimple_lock(&(VME)->vme_faultlock);		 		\
	(VME)->vme_anchor++;						\
	usimple_unlock(&(VME)->vme_faultlock);				\
}

#define vm_mape_faultdone(VME) {					\
	usimple_lock(&(VME)->vme_faultlock);				\
	(VME)->vme_anchor--;						\
	if ((VME)->vme_anchor == 0 && (VME)->vme_mutate) {		\
		thread_wakeup((vm_offset_t) (VME));			\
		(VME)->vme_mutate = 0;					\
	}								\
	usimple_unlock(&(VME)->vme_faultlock);				\
}

#ifdef	KERNEL
extern boolean_t u_map_entry_delete_check(vm_map_t map, vm_map_entry_t ep, 
			vm_offset_t start, vm_offset_t end);
extern boolean_t u_map_entry_clip_check(vm_map_t map, vm_map_entry_t ep,
			vm_offset_t start, vm_offset_t end);
#endif	/* KERNEL */

#endif /* !__VM_UMAP__ */
