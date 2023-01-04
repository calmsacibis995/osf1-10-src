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
 * @(#)$RCSfile: vm_swap.h,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/08/20 19:26:24 $
 */
#ifndef	__VM_SWAP__
#define __VM_SWAP__
#include <sys/vp_swap.h>
#include <sys/unix_defs.h>
#include <kern/lock.h>

/*
 * swap arguments for swapon
 */

#define	MS_PREFER	0x0001

struct vm_swap {
	struct vm_swap *vs_fl, *vs_bl;		/* FL & BL swap structures */
	unsigned char vs_flags;			/* Flags */
	decl_simple_lock_data(,	vs_lock)	/* Lookup and allocation lock */
	struct h_kmem_info *vs_hinfo;		/* Kernel heap */
	int vs_oshift;				/* Object shift */
	int vs_swapsize;			/* Space size */
	int vs_freespace;			/* Free space */
	struct vm_anon *vs_anfree;		/* anon free list */
	struct vm_anon *vs_anbase;		/* base of anon array */
	struct vps_info vs_vinfo;		/* from VOP_SWAP open */
	struct vm_swap_object *vs_object;	/* swap object */
	char *vs_path;				/* pathname of swap file */
};

#define	vs_vp		vs_vinfo.vps_vp
#define	vs_rvp		vs_vinfo.vps_rvp
#define	vs_dev		vs_vinfo.vps_dev
#define	vs_shift	vs_vinfo.vps_shift

#define	VS_SWAP_GROW	0x20			

struct vm_swap_object {
	struct vm_object sw_object;		/* Object common part */
	struct vm_swap 	*sw_sp;			/* Pointer to vm_swap */
};

/*
 * Swap flags
 */

#define	VS_ENABLED	0x01			/* Swapping enabled */


#ifdef	KERNEL

extern int vm_swap_eager;			/* Eager swap allocation */
extern int vm_swap_written;			/* Total pages out */
extern struct vm_swap *vm_swap_head;		/* Head of swap containers */
extern struct vm_swap *vm_swap_lazy;		/* Lazy swap structure */
extern struct vm_swap_object 
		*vm_swap_lazy_object;		/* Lazy swap object */
extern vm_size_t vm_swap_space;			/* Space available in */
extern vm_size_t vm_total_swap_space;
extern int vm_swap_lock_wait_time;

extern struct vm_swap *vm_swap_circular;	/* Next swap device to use */

#if	UNIX_LOCKS && (MACH_SLOCKS || RT_PREEMPT)
extern simple_lock_data_t vm_swap_space_lock;
extern simple_lock_data_t vm_swap_lock;

extern simple_lock_data_t vm_swap_circular_lock;/* Mutex for vm_swap_circular */

#define	swap_space_lock()	usimple_lock(&vm_swap_space_lock)
#define	swap_space_unlock()	usimple_unlock(&vm_swap_space_lock)
#else
#define	swap_space_lock()	1
#define	swap_space_unlock()	1
#endif	/* UNIX_LOCKS */

#define	swap_space_free(SZ) {					\
	usimple_lock(&vm_swap_space_lock);			\
	vm_swap_space += (SZ);					\
	usimple_unlock(&vm_swap_space_lock);			\
}

#define	swap_space_alloc(SZ)					\
	(swap_space_lock(), 					\
		(((SZ) > vm_swap_space) ?			\
		(swap_space_unlock(), FALSE) :			\
		(vm_swap_space -= (SZ), 			\
		swap_space_unlock(), TRUE)))



struct vm_swap_lock_list {
	unsigned int
		vsl_read 	: 24, 		/* Readers */
		vsl_write 	: 1,		/* Write locked */
		vsl_wantwrite 	: 1,		/* Want write */
				: 6;
};

extern struct vm_swap_lock_list vm_swap_lock_list;


#define	swap_read_lock() {					\
	usimple_lock(&vm_swap_lock);				\
	while (vm_swap_lock_list.vsl_write) {			\
		register int VMSLW;				\
		usimple_unlock(&vm_swap_lock);			\
		for (VMSLW = vm_swap_lock_wait_time; 		\
			VMSLW && vm_swap_lock_list.vsl_write;	\
			VMSLW--);				\
		usimple_lock(&vm_swap_lock);			\
	}							\
	vm_swap_lock_list.vsl_read++;				\
	usimple_unlock(&vm_swap_lock);				\
}

#define	swap_write_lock() {					\
	usimple_lock(&vm_swap_lock);				\
	while (vm_swap_lock_list.vsl_read) {			\
		vm_swap_lock_list.vsl_wantwrite = 1;		\
		assert_wait((vm_offset_t)&vm_swap_lock_list, FALSE);	\
		usimple_unlock(&vm_swap_lock);			\
		thread_block();					\
		usimple_lock(&vm_swap_lock);			\
	}							\
	vm_swap_lock_list.vsl_write = 1;			\
	usimple_unlock(&vm_swap_lock);				\
}

#define	swap_write_unlock() {					\
	usimple_lock(&vm_swap_lock);				\
	vm_swap_lock_list.vsl_write = 0;			\
	usimple_unlock(&vm_swap_lock);				\
}

#define	swap_read_unlock() {					\
	usimple_lock(&vm_swap_lock);				\
	vm_swap_lock_list.vsl_read--;				\
	if (!vm_swap_lock_list.vsl_read && 			\
		vm_swap_lock_list.vsl_wantwrite) {		\
		thread_wakeup((vm_offset_t) &vm_swap_lock_list);\
		vm_swap_lock_list.vsl_wantwrite = 0;		\
	}							\
	usimple_unlock(&vm_swap_lock);				\
}

#define swap_vslock(VS)		usimple_lock(&((VS)->vs_lock))
#define	swap_vsunlock(VS) 	usimple_unlock(&((VS)->vs_lock))


/*
 * Minimum swdone threads
 */

#define	MIN_SWDONE_THREADS	1

/*
 * Number of buffers to grow by
 */

#define	VM_SWAP_BGROW		1

#endif	/* KERNEL */

#endif /* !__VM_SWAP__ */
