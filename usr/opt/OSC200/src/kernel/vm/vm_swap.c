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
static char *rcsid = "@(#)$RCSfile: vm_swap.c,v $ $Revision: 1.1.19.12 $ (DEC) $Date: 1994/01/06 16:19:20 $";
#endif
#include <sys/unix_defs.h>
#include <sys/buf.h>
#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <vm/vm_object.h>
#include <vm/vm_swap.h>
#include <vm/vm_anon.h>
#include <vm/vm_page.h>
#include <vm/vm_pagelru.h>
#include <vm/vm_anonpage.h>
#include <sys/vp_swap.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <vm/vm_tune.h>
#include <vm/heap_kmem.h>
#include <vm/vm_debug.h>
#include <vm/vm_perf.h>
#include <sys/lwc.h>
#if SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif
#include <mach/machine/vm_param.h>

#define SVR4_SYSCALLS


struct vm_swap *vm_swap_head;
/*
 * Swap i/o load balancing.  The vm_swap_circular_lock is used as a mutex
 * for reading and writing to vm_swap_circular.  If/when device deallocation
 * (ie. "swapoff") becomes supported, verify if vm_swap_circular is set to
 * the device to be deallocated and set it to next one in the list if it is.
 */
struct vm_swap *vm_swap_circular;		/* Next swap device to use */
decl_simple_lock_data(, vm_swap_circular_lock);	/* Mutex for vm_swap_circular */

/*
 * The vm_swap_lock is used to synchronize adding and deleting
 * swap devices and/or files.  A read lock must be acquired
 * for normal allocation from any swap device in the list.
 * A write lock must be acquired for modifications to the swap list.
 */

lock_data_t vm_swap_modify_lock;
udecl_simple_lock_data(, vm_swap_space_lock)
udecl_simple_lock_data(, vm_swap_lock);
struct vm_swap_lock_list vm_swap_lock_list;
#if	UNIX_LOCKS
int vm_swap_lock_wait_time = 200;
#else
int vm_swap_lock_wait_time = 0;
#endif	/* UNIX_LOCKS */
vm_size_t vm_total_swap_space;			/* Total space */
vm_size_t vm_swap_space;			/* Free space */

static char swapdefault[] = "/sbin/swapdefault";

struct vm_swap *vm_swap_lazy;
struct vm_swap_object *vm_swap_lazy_object;
extern vm_map_t zone_map;
extern vm_size_t vm_max_wrpgio_kluster;
extern vm_size_t vm_max_rdpgio_kluster;

extern dev_t dumpdev;
int vm_swap_eager;

#if defined(SVR4_SYSCALLS)
#ifdef MACH
#define PATH_ALLOCATE(path)     ZALLOC(pathname_zone, (path), char *)
#define PATH_DEALLOCATE(path)   ZFREE(pathname_zone, (path))
#else
#define PATH_ALLOCATE(path)     MALLOC((path), char *, MAXPATHLEN,      \
                                       M_NAMEI,M_WAITOK)
#define PATH_DEALLOCATE(path)   FREE((path), M_NAMEI)
#endif /* MACH */
#endif /* SVR4_SYSCALLS */

void
vm_swap_init()
{

	lock_init(&vm_swap_modify_lock);
	usimple_lock_init(&vm_swap_space_lock);
	usimple_lock_init(&vm_swap_lock);
	vm_swap_io_init();
	if (vm_swapon(swapdefault, 0, 0, 0, UIO_SYSSPACE)) {
		vm_swap_lazy = (struct vm_swap *) 
			h_kmem_zalloc_memory(sizeof (struct vm_swap), FALSE);

		if (!vm_swap_lazy || (vm_object_allocate(OT_SWAP, 1, 
			(caddr_t) 0, (vm_object_t *) 
			&vm_swap_lazy_object) != KERN_SUCCESS)) 
			panic("vm_swap_init: failed to alloc swap init data");

		vm_swap_lazy->vs_anbase = (struct vm_anon *) 
			zone_map->vm_min_offset;
		vm_swap_lazy->vs_oshift = 0;
		vm_swap_lazy->vs_object = vm_swap_lazy_object;
		vm_swap_lazy_object->sw_sp = vm_swap_lazy;

		printf("vm_swap_init: warning %s swap device not found\n",
				swapdefault);
		printf("vm_swap_init: in swap over commitment mode\n");
	}

	/*
	 * No dump dev and the swapdefault device was seen.
	 * Set dump dev to the default swap device.
	 */ 

	else {
		vm_swap_eager = 1;
		if (dumpdev == NODEV) dumpdev = vm_swap_head->vs_dev;
	}

	a_init();
}

int
swapon(void *p, void *args, int *retval)
{
	struct	args {
		char 	*filename;
		int	flags;
		long	lowat;
		long	hiwat;
	} *uap = (struct args *) args;
	int error;

	/*
	 * Only the super-user can turn on a swapping file.
	 */

#if     SEC_BASE
	if (!privileged(SEC_MOUNT, EPERM)) return EPERM;
#else
	if (error = suser(u.u_cred, &u.u_acflag)) return error;
#endif

	return vm_swapon(uap->filename, uap->flags,
		uap->lowat, uap->hiwat, UIO_USERSPACE);
}

vm_swapon(caddr_t filename,
	int flags,
	long lowat,
	long hiwat,
	enum uio_seg seg)
{
	struct nameidata *ndp = &u.u_nd;
	struct vnode *vp;
	int error;
#if defined(SVR4_SYSCALLS)
	int len;
	char *path;
#endif

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = seg;
	ndp->ni_dirp = filename;

	if (error = namei(ndp))
		return (error);

	vp = ndp->ni_vp;
	if (vp->v_type != VBLK) {
		error = EINVAL;
		goto rel_out;
	}

#if defined(SVR4_SYSCALLS)
	/*
	 * Get the file name to system space.
	 * SVR4 swapctl() needs to copy swap file path to user
	 */
	if (seg==UIO_USERSPACE || seg==UIO_USERISPACE) {
		PATH_ALLOCATE(path);
		if (path == (char *)0) {
			error = ENOMEM;
			goto rel_out;
		}
		if (error=copyinstr(filename, path, MAXPATHLEN, &len)){
			PATH_DEALLOCATE(path);
			goto rel_out;
		}
		filename = path;
	}

	if (error = vm_swap_add(vp, flags&MS_PREFER, lowat, hiwat,filename)) {
		if (seg==UIO_USERSPACE || seg==UIO_USERISPACE) 
			PATH_DEALLOCATE(path); 
	}
#else 

	error = vm_swap_add(vp, flags&MS_PREFER, lowat, hiwat);

#endif /* defined(SVR4_SYSCALLS) */

rel_out:
	vrele(vp);
	return error;
}

/*
 * Add this vp to be used for swap
 */

vm_swap_add(struct vnode *vp, 
	int prefer, 
	int lowat, 
#if defined(SVR4_SYSCALLS)
	int hiwat,
	char *filename)
#else
	int hiwat)
#endif
{
	int error, dummy;
	vm_size_t amemsize;
	register long size, pages;
	struct ucred *ucred;
	register struct vm_swap *newsp;
	struct vm_swap *sp;
	struct h_kmem_info *hkmp;
	struct vm_swap_object *sop;
	struct vps_info vps;
	caddr_t	temp;

	/*
	 * Assure that only one thread is modify the swap list
	 * during any period of time.
	 */

	lock_write(&vm_swap_modify_lock);

	VOP_SWAP(vp, VPS_OPEN, &vps, u.u_cred, error);

	if (error) {
		lock_done(&vm_swap_modify_lock);
		return error;
	}
	sop = (struct vm_swap_object *) 0;
	newsp = (struct vm_swap *) 0;

	size = vps.vps_size;
	if (size == 0) {
		error = ENODEV;
		goto bad;
	}

	newsp = (struct vm_swap *) 
		h_kmem_zalloc_memory(sizeof (struct vm_swap), TRUE);	
	if (newsp == NULL) {
		error = ENOMEM;
		goto bad;
	}
	newsp->vs_vinfo = vps;

	pages = atop(size);


	/*
	 * Allocate the swap object which is used
	 * only for pageout activities.
	 */

	if (vm_object_allocate(OT_SWAP, size, 
		(caddr_t) 0, (vm_object_t *) &sop) != KERN_SUCCESS) {
		error = ENOMEM;
		goto bad;
	}

	/*
	 * Size and allocate the anon array
	 */

#if	VM_ANON_TRACK
	amemsize = pages * (sizeof (struct vm_anon) + sizeof (struct an_track));
#else
	amemsize = pages * sizeof (struct vm_anon);
#endif	/* VM_ANON_TRACK */

	hkmp = h_kmem_alloc_init(amemsize, TRUE);

	if (hkmp == NULL) {
		error = ENOMEM;
		goto bad;
	}

	sop->sw_sp = newsp;
	newsp->vs_object = sop;
	usimple_lock_init(&newsp->vs_lock);

	if (newsp->vs_vinfo.vps_flags & VPS_NOMAP) {
		if (newsp->vs_shift < page_shift) 
			newsp->vs_shift = page_shift - newsp->vs_shift;
		else newsp->vs_shift = 0;
	}

	newsp->vs_swapsize = newsp->vs_freespace = pages;
	newsp->vs_oshift = page_shift;
	newsp->vs_hinfo = hkmp;
	h_get_kmem_info(hkmp, &newsp->vs_anbase, &amemsize);
	newsp->vs_anfree = (struct vm_anon *) 0;

	/* 
	 * HACK ALERT !  This code makes the first page of the newly
	 * created swap space unavailable by stealing its anon element.
	 * This hack prevents the system from clobbering the disk label
	 * if a user does a swapon to a disk partition that contains
	 * block 0 of the physical disk.  This hack is patterned after
	 * the code contained in a_swap_alloc() in vm_anon.c
         */

	temp = (caddr_t)h_kmem_fast_alloc_memory_(hkmp,
					&newsp->vs_anfree,
					sizeof (struct vm_anon),
					1,
					FALSE);  /* Don't wait */

	if (temp == NULL) {
		error = ENOMEM;
		goto bad;
	}

	newsp->vs_freespace--;		/* Account for stolen page */
	pages--;

#if defined(SVR4_SYSCALLS)
	/*
	 * We are adding filename only if everything went ok.
	 */
	newsp->vs_path = filename;
#endif

	/*
	 * Add the swap to the system
	 */

	swap_write_lock();
	if (vm_swap_head) {
		sp = vm_swap_head;	
		newsp->vs_bl = sp->vs_bl;
		newsp->vs_fl = sp;
		sp->vs_bl->vs_fl = newsp;
		sp->vs_bl = newsp;
	}
	else {
		vm_swap_head = newsp;
		newsp->vs_fl = newsp->vs_bl = newsp;

		/*
		 * Initialize swap i/o load balancing
		 */
		vm_swap_circular = vm_swap_head;
		simple_lock_init(&vm_swap_circular_lock);
	}
	vm_total_swap_space += (pages+1);
	swap_write_unlock();

	lock_done(&vm_swap_modify_lock);

	/*
	 * Increase the swap free space
	 */

	swap_space_free(pages);	

	return 0;
bad:
	if (newsp) h_kmem_free_memory((caddr_t) newsp, 
			sizeof (struct vm_swap), FALSE);
	if (sop) vm_object_deallocate(sop);
	VOP_SWAP(vp, VPS_CLOSE, &vps, u.u_cred, dummy);
	lock_done(&vm_swap_modify_lock);
	return error;
}

boolean_t
vm_swap_lock_try_anon(struct vm_swap_object *sop,
	register vm_offset_t offset)
{
	register struct vm_swap *sp;
	register struct vm_anon *ap;

	sp = sop->sw_sp;
	ap = sp->vs_anbase + (offset >> sp->vs_oshift);
	ap = a_saptoap(ap);
	return a_lock_try(ap);
}

vm_swap_unlock_anon(struct vm_swap_object *sop,
	register vm_offset_t offset)
{
	register struct vm_swap *sp;
	register struct vm_anon *ap;

	sp = sop->sw_sp;
	ap = sp->vs_anbase + (offset >> sp->vs_oshift);
	ap = a_saptoap(ap);
	a_unlock(ap);
}

/*
 * This release code is required for copyoverwrite in the u_map code.
 */

vm_swap_pagectl(struct vm_swap_object *sop,
	register vm_page_t *pp,
	register int pcnt,
	int flags)
{
	if (flags != VMOP_RELEASE) panic("vm_swap_pagectl: bad op");
	while (pcnt--) vm_page_release(*pp++);
}

/*
 * Swap object pageout code.
 * Will call anon's object owner pageout if there is one.
 * We enter with the anon object lock held on the region in question.
 */

int
vm_swap_pageout(struct vm_swap_object *sop,
	register vm_offset_t offset,
	vm_size_t size)
{
	register struct vm_swap *sp;
	register struct vm_anon *ap, *sap;
	register vm_page_t pp;

	sp = sop->sw_sp;
	ap = sp->vs_anbase + (offset >> sp->vs_oshift);
	ap = a_saptoap(ap);
	pp = ap->an_page;


	/*
	 * If the page has an owner then we attempt to acquire
	 * the kluster lock of the owner.
	 */

	if (pp->pg_owner) {

		/*
		 * Failed to acquire the lock
		 */

		if (!OOP_KLOCK_TRY((vm_object_t) (pp->pg_owner), 
				pp->pg_roffset)) {
			vm_pageout_abort(pp);
			a_unlock(ap);	
			return 0;
		}
		else {
			a_unlock(ap);
			return OOP_PAGEOUT((vm_object_t) (pp->pg_owner), 
					pp->pg_roffset, size);
		}
	}
	else {
		register vm_page_t pagelist[1];

		if (vm_pageout_remove(pp, TRUE, TRUE) == FALSE) {
			vm_pageout_abort(pp);	
			a_unlock(ap);
			return 0;
		}
		else if (!pp->pg_dirty) {
			AN_TRACK_WRITTEN(ap);
			vm_anon_page_free((struct vm_anon_object *) 0,
				(vm_offset_t) 0, ap);
			a_unlock(ap);
			return 0;
		}
		else if (!a_swap_lazy_alloc(ap, pp, FALSE,(struct vm_swap *)0)){
			a_unlock(ap);
			vm_pageout_abort(pp);
			return 0;
		} 
		else a_unlock(ap);
		
		AN_TRACK_WRITE(ap);
		pagelist[0] = pp;
		pp->pg_pnext = VM_PAGE_NULL;
		vm_swap_io(pagelist, B_WRITE);
		return 1;
	}
}

kern_return_t
vm_swap_pagesteal(struct vm_swap_object *sop, struct vm_page *pp)
{

	/* if this page is owned by an anon object call the anon page stealer */

	if (pp->pg_owner)

		return OOP_PAGESTEAL((vm_object_t)pp->pg_owner, pp);

	else {

		/* see it this page is a clean page belonging to a swap object */

		vm_offset_t offset;
		register struct vm_anon *ap;
		struct vm_swap *sp;

                if (vm_object_lock_try(sop)) {
				
			/* take this page if it is not being used */

                        offset = (vm_offset_t) pp->pg_offset;
                       	sp = sop->sw_sp;
                        ap = a_saptoap(sp->vs_anbase + (offset >> sp->vs_oshift));
                        VM_PAGE_QUEUES_REMOVE(pp);
                        vm_anon_page_free((struct vm_anon_object *) 0, (vm_offset_t) 0 , ap);
                       	vm_page_unlock_queues();
                        vm_object_unlock(sop);
                        return KERN_SUCCESS;
			
		} else {

			/* dont take the page if we couldnt grab the lock */

			return KERN_FAILURE;

		}

	}
     

}

vm_swap_oop_bad()
{
	panic("vm_swap_oop_bad: invalid operation");
}

struct vm_object_ops vm_swap_object_ops = {
	OOP_KLOCK_TRY_K 
	&vm_swap_lock_try_anon,		/* klock_try */
	OOP_UNKLOCK_K 
	&vm_swap_unlock_anon,		/* kunlock */
	&vm_ops_reference_def,		/* reference */
	&vm_ops_deallocate_def,		/* deallocate */
	OOP_PAGEIN_K &vm_swap_oop_bad,	/* pagein */
	OOP_PAGEOUT_K &vm_swap_pageout,	/* pageout */
	OOP_SWAP_K &vm_swap_oop_bad,	/* swap */
	OOP_CONTROL_K &vm_swap_oop_bad,	/* control */
	OOP_PAGECTL_K &vm_swap_pagectl,	/* page ctl */
	OOP_PAGESTEAL_K &vm_swap_pagesteal /* page steal */
};

struct vm_object_config swap_object_conf = {
	(int (*)()) 0,
	(int (*)()) 0,
	sizeof (struct vm_swap_object),
	&vm_swap_object_ops,
	(struct vm_map_entry_ops *) 0,
};

udecl_simple_lock_data(, vm_swap_swdone_lock);
udecl_simple_lock_data(, vm_swap_buffree_lock);
udecl_simple_lock_data(, vm_swap_page_lock);

/* synchronous swap buf counters */
int vm_sync_swap_buf_want;
int vm_sync_swap_iocount;
int vm_sync_swap_iomax;

/* ssynchronous swap buf counters */
int vm_async_swap_buf_want;
int vm_async_swap_iocount;
int vm_async_swap_iomax;

int vm_swap_write_done;
struct buf *swdone;
struct buf *vm_swap_free_buf;
extern struct buf *vm_swap_bp_alloc();
extern void vm_swap_swdone_lwc();
lwc_id_t vm_lwc_id;
 
/*
 * Initialization
 */

vm_swap_io_init()
{
	register int i;

	usimple_lock_init(&vm_swap_swdone_lock);
	usimple_lock_init(&vm_swap_buffree_lock);
	usimple_lock_init(&vm_swap_page_lock);

	/* initialize the synchronous buffer count */
	vm_sync_swap_iomax = vm_tune_value(syncswapbuffers);
	vm_sync_swap_iomax = ((vm_sync_swap_iomax + VM_SWAP_BGROW - 1 ) 
		/ VM_SWAP_BGROW) * VM_SWAP_BGROW;
	if (!vm_sync_swap_iomax) vm_sync_swap_iomax = VM_SWAP_BGROW * 4;

	/* initialize the synchronous buffer count */
        vm_async_swap_iomax = vm_tune_value(asyncswapbuffers);
        vm_async_swap_iomax = ((vm_async_swap_iomax + VM_SWAP_BGROW - 1 )
                / VM_SWAP_BGROW) * VM_SWAP_BGROW;
        if (!vm_async_swap_iomax) vm_async_swap_iomax = VM_SWAP_BGROW * 4;

	vm_lwc_id = lwc_create(0, &vm_swap_swdone_lwc);
	if (vm_lwc_id == LWC_ID_NULL) 
		panic("vm_swapio_init: lwc id null");
	vm_swap_bp_free(vm_swap_bp_alloc(FALSE),FALSE);
}

#if	VM_SWAP_BUF_TRACE
#define	VM_STSIZE	1024
long	vm_stcount;	

struct vm_swap_buf_trace {
	struct buf 	*st_bp;
	vm_page_t 	st_page;
	struct buf	st_cbp;
	struct vm_page	st_cpage;
} vm_swap_buf_trace[VM_STSIZE];
	
#define	vm_swap_trace_next					\
	if (++vm_stcount >= VM_STSIZE) vm_stcount = 0

#define	vm_swap_trace(PP,BP) {						\
	vm_swap_buf_trace[vm_stcount].st_bp = (BP);			\
	vm_swap_buf_trace[vm_stcount].st_page = (PP);			\
	vm_swap_buf_trace[vm_stcount].st_cbp = *(BP);			\
	vm_swap_buf_trace[vm_stcount].st_cpage = *(PP);			\
	vm_swap_trace_next;						\
}

#else
#define	vm_swap_trace(PP,BP)
#endif	/* VM_SWAP_BUF_TRACE */


/*
 * LWC main execution loop.
 */

void
vm_swap_swdone_lwc()
{
	register int s, flags;
	register struct buf *bp;
	register vm_page_t pp, pn;
	int error, pageio;
	register struct vm_anon *ap;
	register struct vm_swap *sp;

	while (1) {
		s = splbio();
		usimple_lock(&vm_swap_swdone_lock);
		if ((bp = swdone) == NULL) {
			lwc_rfc(vm_lwc_id);
			usimple_unlock(&vm_swap_swdone_lock);
			(void) splx(s);
			return;
		}

		if (bp->av_forw == bp) swdone = BUF_NULL;
		else {
			swdone = bp->av_forw;
			bremfree(bp);
		}
		usimple_unlock(&vm_swap_swdone_lock);
		(void) splx(s);
	
		pp = bp->b_pagelist;
		vm_swap_trace(pp, bp);
		error = ((bp->b_flags & B_ERROR) != 0);
		flags = bp->b_flags;
		bp_mapout(bp);
		if ((bp->b_flags & B_ASYNC) == 0) {
			vm_swap_write_done = TRUE;
			thread_wakeup((vm_offset_t)&vm_swap_write_done);
		}
			/* B_ASYNC on and B_READ off(B_WRITE) means vm_pageout demon completion */

		vm_swap_bp_free(bp, (((bp->b_flags & (B_ASYNC|B_READ)) == B_ASYNC) ? TRUE : FALSE));

		if (error) {
			if (flags & B_READ) 
				printf("vm_swap I/O error durring pagein\n");
			else
				printf("vm_swap I/O error durring pageout\n");
			}	
		usimple_lock(&vm_swap_page_lock);
		pageio = --pp->pg_iocnt;
		usimple_unlock(&vm_swap_page_lock);

		if (pageio == 0) do {
			pn = pp->pg_pnext; 

			pp->pg_error |= error;

			/* if an IO error occurred during the pagein, process will be signaled */

			if (flags & B_READ) {
				vm_pageout_activate(pp, FALSE);
				continue;
			}

			/* if an IO error occurred during the pageout, abort the operation */

			if (pp->pg_error) {
				pp->pg_error = 0;
				vm_pageout_abort(pp);
				continue;
			}

			pmap_clear_modify(page_to_phys(pp));
			pp->pg_dirty = 0;
			pp->pg_zeroed = 0;
	
			if (pp->pg_reserved & PG_PREWRITE) {
				vm_page_clean_done(pp);
				continue;
			}

			sp = ((struct vm_swap_object *)pp->pg_object)->sw_sp;
			ap = sp->vs_anbase + atop(pp->pg_offset);
			ap = a_saptoap(ap);
			a_lock(ap);
			vm_page_lock(pp);

			if (pp->pg_hold == 0) {
				ap->an_page = VM_PAGE_NULL;
				a_unlock(ap);
				vm_page_unlock(pp);
				vm_object_lock(sp->vs_object);
				vm_page_remove_bucket(pp);
				vm_object_unlock(sp->vs_object);
				vm_pg_free(pp);
			}

			/*
			 * Some one got to us while we were paging out.
			 */

			else {
				vm_page_unlock(pp);
				a_unlock(ap);
				vm_pageout_abort(pp);
				vpf_ladd(swapreclaims,1);
			}
		} while ((pp = pn) != VM_PAGE_NULL);
	}
}


struct buf *
vm_swap_bp_alloc(boolean_t async)
{
	register struct buf *bp;
	
	bp = BUF_NULL;
	usimple_lock(&vm_swap_buffree_lock);
		/* async bufs are used by the vm_pageout routine when it steels pages from anonymous objects */
	if (async) {
		do {
				/* stall vm_pageout if it has vm_async_swap_iomax I/Os outstanding */
			if (vm_async_swap_iocount < vm_async_swap_iomax) bp = (struct buf *) 
				h_kmem_fast_zalloc_memory((caddr_t) &vm_swap_free_buf,
					sizeof (struct buf), VM_SWAP_BGROW, FALSE);	
			if (bp == NULL) {
				vm_async_swap_buf_want++;
				mpsleep((vm_offset_t) &vm_async_swap_buf_want,
					PZERO, "SWAPBWAIT", FALSE,
					simple_lock_addr(vm_swap_buffree_lock), 
					MS_LOCK_ON_ERROR|MS_LOCK_SIMPLE);
			}
		} while (bp == NULL);
		vm_async_swap_iocount++;
	} else {
		/* sync bufs are used by anonymous pagein faults and by the task swapper pushing out anonymous objects */
                do {
				/* stall anon pagins and swapper if there are vm_sync_swap_iomax I/Os outstanding */
                        if (vm_sync_swap_iocount < vm_sync_swap_iomax) bp = (struct buf *)
                                h_kmem_fast_zalloc_memory((caddr_t) &vm_swap_free_buf,
                                        sizeof (struct buf), VM_SWAP_BGROW, FALSE);
                        if (bp == NULL) {
                                vm_sync_swap_buf_want++;
                                mpsleep((vm_offset_t) &vm_sync_swap_buf_want,
                                        PZERO, "SWAPBWAIT", FALSE,
                                        simple_lock_addr(vm_swap_buffree_lock),
                                        MS_LOCK_ON_ERROR|MS_LOCK_SIMPLE);
                        }
                } while (bp == NULL);
                vm_sync_swap_iocount++;
	}
	usimple_unlock(&vm_swap_buffree_lock);
	event_init(&bp->b_iocomplete);
	return bp;
}

vm_swap_bp_free(register struct buf *bp, boolean_t async)
{
	usimple_lock(&vm_swap_buffree_lock);
	h_kmem_fast_free(&vm_swap_free_buf, bp);
	if (async) {
			/* page steeler IO completion, wake vm_pageout up if it was stalled  allocating a buf */
		vm_async_swap_iocount--;
		if (vm_async_swap_buf_want) {
			vm_async_swap_buf_want--;
			thread_wakeup_one((vm_offset_t)&vm_async_swap_buf_want);
		}
	} else {
			/* asynch pagein or task_swapper IO completion, wake anyone up wating for a buf */
	        vm_sync_swap_iocount--;
                if (vm_sync_swap_buf_want) {
                        vm_sync_swap_buf_want--;
                        thread_wakeup_one((vm_offset_t)&vm_sync_swap_buf_want);
                }
	}
	usimple_unlock(&vm_swap_buffree_lock);
}

vm_swap_sort_add(vm_page_t *pl, 
		register vm_page_t pp)
{
	register vm_page_t pe, pel;
	register vm_object_t sop;

	if (*pl == VM_PAGE_NULL) {
		*pl = pp;
		pp->pg_pnext = VM_PAGE_NULL;
		return;
	}

	sop = pp->pg_object;
	for (pel = VM_PAGE_NULL, pe = *pl; pe != VM_PAGE_NULL;  
			pel = pe, pe = pe->pg_pnext) 
		if (pe->pg_object == sop) {
			for(;pe != VM_PAGE_NULL && pe->pg_object == sop; 
				pel = pe, pe = pe->pg_pnext)
				if (pp->pg_offset < pe->pg_offset) {
					pp->pg_pnext = pe;
					if (pel != VM_PAGE_NULL)
						pel->pg_pnext = pp;
					else if (*pl == pe) *pl = pp;
					return;
				}
			pel->pg_pnext = pp;
			pp->pg_pnext = pe;
			return;
		}
	pel->pg_pnext = pp;
	pp->pg_pnext = VM_PAGE_NULL;
	return;
}

/*
 * Currently only supports VPS_NOMAP devices.
 */

vm_swap_io(register vm_page_t *pl, int flags)
{
	register vm_page_t pp, pnext, pprev;
	register struct vm_swap *sp;
	struct vm_swap_object *sop;
	register struct vm_anon *ap;
	register vm_offset_t offset, send, inc;
	register vm_size_t size, kluster;
	int error, rw;
	register struct buf *bp;

	rw = flags & ~B_SWAP;
	kluster = (rw) ? vm_max_rdpgio_kluster : vm_max_wrpgio_kluster;
	for (pp = *pl; pp != VM_PAGE_NULL; pp = *pl) {
		size = PAGE_SIZE;
		sop = (struct vm_swap_object *) (pp->pg_object);
		sp = sop->sw_sp;
		for (pnext = pp->pg_pnext, pprev = pp; 
			pnext != VM_PAGE_NULL && size < kluster;
			pprev = pnext, pnext = pnext->pg_pnext) 
			if ((pnext->pg_object != (vm_object_t) sop) ||
				((pprev->pg_offset + PAGE_SIZE) !=
				pnext->pg_offset)) break;
			else size += PAGE_SIZE;

		*pl = pnext;
		pprev->pg_pnext = VM_PAGE_NULL;

		if (((flags & (B_SWAP|B_READ|B_WRITE)) == B_WRITE) && 
			(size < vm_max_wrpgio_kluster)) for (
			send = ptoa(sp->vs_swapsize), 
			(pp->pg_offset) ? 
			(inc = -PAGE_SIZE, 
			offset = pp->pg_offset + inc) :
			(inc = PAGE_SIZE,
			offset = pprev->pg_offset + inc);
			size < vm_max_wrpgio_kluster;
			offset += inc) {


			vm_object_lock(sop);
			pnext = vm_page_lookup(sop, offset);
			if (pnext != VM_PAGE_NULL) {
				ap = sp->vs_anbase + atop(offset);
				ap = a_saptoap(ap);
				if (a_lock_try(ap)) {
					vm_object_unlock(sop);
					if (pnext == ap->an_page) 
						pnext = vm_page_clean(pnext);
					else pnext = VM_PAGE_NULL;
					a_unlock(ap);
				}
				else vm_object_unlock(sop);
			}
			else vm_object_unlock(sop);
			if (pnext != VM_PAGE_NULL) {
				size += PAGE_SIZE;
				if (inc == -PAGE_SIZE) {
					pnext->pg_pnext = pp;
					pp = pnext;
				}
				else {
					pprev->pg_pnext = pnext;
					pnext->pg_pnext = VM_PAGE_NULL;
					pprev = pnext;
				}
			}
			if (inc == -PAGE_SIZE) {
				if (pnext == VM_PAGE_NULL ||
					offset == (vm_offset_t) 0) {
					inc = PAGE_SIZE;
					offset = pprev->pg_offset; 
				}
			}
			else if (pnext == VM_PAGE_NULL) break;
			else if ((offset + PAGE_SIZE) >= send)
				break;
		}

		/*
		 * pp contains a linked list of contiguous swap space.
		 * size is the computed I/O transfer size.
		 */

		pp->pg_iocnt = 1;
			/* both B_SWAP and B_READ zero mean request is from vm_pageout */
		bp = vm_swap_bp_alloc(((flags & (B_SWAP|B_READ)) == 0) ? TRUE : FALSE);
		bp->b_pagelist = pp;
		if ((flags & B_SWAP) == 0) {
			if (rw) {
				vpf_ladd(pgioreads,1);
				vpf_ladd(pgreads, size / PAGE_SIZE);
			}
			else {
				vpf_ladd(pgiowrites,1);
				vpf_ladd(pgwrites, size / PAGE_SIZE);
			}
		}
		bp->b_un.b_addr = (caddr_t) (page_to_phys(pp));
		bp->b_blkno = atop(pp->pg_offset) << sp->vs_shift;
		bp->b_flags = ((flags & B_SWAP) ? 0 : B_ASYNC) | B_SWAP | rw;
		bp->b_bcount = size;
		bp->b_vp = sp->vs_vp;
		bp->b_rvp = sp->vs_rvp;
		bp->b_dev = sp->vs_dev;
		bp_mapin(bp);
		vm_swap_trace(pp, bp);

		if (flags & B_SWAP) 
			vm_swap_write_done = FALSE;
		VOP_STRATEGY(bp, error);
		if (error) goto swapfail;
		else if (flags & B_SWAP) {
 			assert_wait((vm_offset_t)&vm_swap_write_done, FALSE);
			if (vm_swap_write_done == FALSE) 
				thread_block();
			else
                                clear_wait(current_thread(), THREAD_AWAKENED,
                                        FALSE);
		}
	}
	return;
swapfail:
	panic("vm_swap_io: swap I/O failure");
}

/* 
 * SAR: calculate the number of 512-byte disk blocks available for page
 * swapping.
 */
long 
svr4_freeswap_space()
{
	extern lock_data_t vm_swap_modify_lock; 
	struct vm_swap     *sp = vm_swap_head;
	long               total_freeswap = 0;

	/* Get the total number of freeswap pages */
	lock_read(&vm_swap_modify_lock);
	do {
		if (sp)
			total_freeswap += sp->vs_freespace;
	} while ( sp && (sp=sp->vs_fl) != vm_swap_head);
	lock_done(&vm_swap_modify_lock);

	/* Return number of 512-byte disk blocks available */
	return((alpha_ptob(total_freeswap))/512);
}
