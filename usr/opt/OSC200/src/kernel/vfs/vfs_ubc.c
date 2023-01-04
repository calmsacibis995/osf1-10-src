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
static char *rcsid = "@(#)$RCSfile: vfs_ubc.c,v $ $Revision: 1.1.27.11 $ (DEC) $Date: 1994/01/14 18:53:28 $";
#endif

#include <kern/assert.h>
#include <mach/vm_param.h>
#include <mach/mach_types.h>
#include <vm/vm_vp.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <vm/pmap.h>
#include <sys/vnode.h>
#include <sys/buf.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <kern/sched_prim.h>
#include <vm/vm_tune.h>
#include <sys/vfs_ubc.h>
#include <vm/heap_kmem.h>
#include <sys/kernel.h>
#include <vm/vm_perf.h>
#include <vm/vm_debug.h>
#include <sys/lwc.h>
#include <kern/processor.h>

#include <dcedfs.h>

/*
 * Exported by vm_pagelru.c
 */

udecl_simple_lock_data(extern, vm_page_wire_count_lock);

/*
 * Head of lru list.
 */

vm_page_t ubc_lru;				/* Head of lru idle list */
udecl_simple_lock_data(, ubc_memory_lock);
int ubc_lru_page_count = 0;			/* number of pages on the lru queue */
vm_page_t ubc_pfree;				/* Page free list if used */
long ubc_pages;					/* Total physical pages */
udecl_simple_lock_data(, ubc_lru_lock);
udecl_simple_lock_data(, ubc_pfree_lock);
extern vm_page_t ubc_alloc(struct vm_vp_object *vop, vm_offset_t offset);

/*
 * For now use the VM subsystem hashing.
 * In the future there be an opportunity to instrument
 * and possibly change to a different hash scheme.
 */

#define	vop_page_hash(PP, OBJ, OFFSET)					\
	vm_page_insert_bucket((PP), (OBJ), (OFFSET))

#define vop_page_unhash(PP) 						\
	vm_page_remove_bucket((PP))

#define	vop_page_lookup(VOP, OFFSET)					\
	vm_page_lookup((VOP), (OFFSET))

extern int ubc_dirty_thread_loop(); 
extern void ubc_async_iodone_lwc();
lwc_id_t ubc_lwc_id;
thread_t ubc_dirty_thread;
static boolean_t ubc_active;			

long ubc_minpages;
extern int ubc_minpercent;
long ubc_maxpages;
extern int ubc_maxpercent;

extern struct buf *swdone;

udecl_simple_lock_data(, ubc_iodone_lock);
struct buf *ubc_iodone_buf;

extern struct buf *ubc_bufget();
extern int ubc_buffree(register struct buf *bp);
int ubc_maxbuffers = 0; 
int ubc_buf_allocated;
int ubc_iocount;
#define	UBC_BUF_GROW		0x20
#define	UBC_BUF_RESERVED 	0x8		/* reserved for vm priv  */
struct buf *ubc_free_buffers;
struct buf *ubc_reserved_buffers, *ubc_first_reserved, *ubc_last_reserved;
int ubc_buf_wanted;				/* bufs wanted */
int ubc_buf_wanted_priv;			/* bufs wanted by priv */
udecl_simple_lock_data(, ubc_buffree_lock);

extern int vm_managed_pages;
long ubc_seqdrain;

long ubc_dirtypages;				/* total dirty pages in ubc */
long ubc_dirty_limit;
#define	UBC_MAX_DIRTY_WRA	20		/* max write aheads */
int ubc_dirty_wra = 0;
#define	UBC
#define	UBC_MAX_DIRTY_PERCENT	5
int ubc_dirty_percent = 0;			/* max pages to examine */
udecl_simple_lock_data(, ubc_dirty_lock);

#define ubc_dirty_limit_compute						\
		ubc_dirty_limit = 					\
		MAX(							\
		((vm_tune_value(ubcdirtypercent) * ubc_minpages)/100),	\
		((vm_tune_value(ubcdirtypercent) * ubc_pages)/100))

#define	ubc_dirty_check (ubc_dirtypages > ubc_dirty_limit)

#define	ubc_dirty_subtract() 						\
	usimple_lock(&ubc_dirty_lock);					\
	ubc_dirtypages--;						\
	usimple_unlock(&ubc_dirty_lock)

#define	ubc_dirty_add() 						\
	usimple_lock(&ubc_dirty_lock);					\
	ubc_dirtypages++;						\
	usimple_unlock(&ubc_dirty_lock)

#define	UBC_DIRTY_SECONDS	5			/* 1 seconds */
long ubc_dirty_seconds = 0;
long ubc_dirty_time;


#define	UBC_EVENTS_ON 1

#if	UBC_EVENTS_ON

#define UBC_EVENTS  14
int ubc_number_events = UBC_EVENTS;
struct ubc_events {
	long ue_count;
	char ue_name[30];
} ubc_events[UBC_EVENTS] = {
{0, "mem pages scanned"},
{0, "mem ref"},
{0, "mem freed"},
{0, "total freed"},
{0, "mem eol"},
{0, "dirty calls"},
{0, "dirty scans"},
{0, "dirty eol"},
{0, "push B_AGE page alloc"},
{0, "stole pg in page alloc"},
{0, "seq drain attempts"},
{0, "I/O target met"},
{0, "MEM target met"},
{0, "ucb_mem_purges"},
};

#define event_mscans		ubc_events[0].ue_count++
#define event_memref		ubc_events[1].ue_count++
#define event_memfree		ubc_events[2].ue_count++
#define event_totalfree		ubc_events[3].ue_count++
#define event_meol		ubc_events[4].ue_count++
#define event_dirty		ubc_events[5].ue_count++
#define event_dirtyscan		ubc_events[6].ue_count++
#define event_dirtyeol		ubc_events[7].ue_count++
#define	event_pushpagealloc	ubc_events[8].ue_count++
#define event_stolepagealloc	ubc_events[9].ue_count++
#define	event_seqdrain		ubc_events[10].ue_count++
#define	event_scan_io		ubc_events[11].ue_count++
#define	event_scan_mem		ubc_events[12].ue_count++
#define	event_purges		ubc_events[13].ue_count++

#else

#define event_mscans
#define event_memref
#define event_memfree
#define event_totalfree
#define event_meol
#define event_dirty
#define event_dirtyscan
#define event_dirtyeol
#define	event_pushpagealloc
#define event_stolepagealloc
#define	event_seqdrain
#define	event_scan_io
#define	event_scan_mem
#define	event_purges

#endif	/* UBC_EVENTS_ON */

ubc_init()
{
	register int i;
	extern task_t first_task;
	thread_t ubc_thread;
	struct buf *bp;

	usimple_lock_init(&ubc_lru_lock);
	usimple_lock_init(&ubc_pfree_lock);
	usimple_lock_init(&ubc_memory_lock);
	usimple_lock_init(&ubc_iodone_lock);
	usimple_lock_init(&ubc_buffree_lock);
	usimple_lock_init(&ubc_dirty_lock);

	if (!ubc_maxbuffers) {
		ubc_maxbuffers = vm_tune_value(ubcbuffers);
		if (ubc_maxbuffers <= 0) 
			ubc_maxbuffers = UBC_BUF_GROW * 4;
	}
	ubc_maxbuffers = (ubc_maxbuffers + UBC_BUF_GROW - 1) & 
		~(UBC_BUF_GROW - 1);

	if (!ubc_seqdrain) {
		ubc_seqdrain = (vm_managed_pages 
			* vm_tune_value(ubcseqstartpercent)) / 100;
		if (ubc_seqdrain == 0) ubc_seqdrain = vm_managed_pages / 2;
	}

	if (ubc_minpercent > ubc_maxpercent) ubc_minpercent = ubc_maxpercent;
	ubc_minpages = (vm_managed_pages * ubc_minpercent ) / 100;
	ubc_maxpages = (vm_managed_pages * ubc_maxpercent ) / 100;

	ubc_dirty_limit_compute;
	if (!ubc_dirty_seconds) ubc_dirty_seconds = UBC_DIRTY_SECONDS;
	if (!ubc_dirty_wra) ubc_dirty_wra = UBC_MAX_DIRTY_WRA;
	if (!ubc_dirty_percent) 
		ubc_dirty_percent = UBC_MAX_DIRTY_PERCENT;
	ubc_dirty_time = hz*ubc_dirty_seconds;	

	/*
	 * Prime the general buffer list.
	 */

	ubc_buffree(ubc_bufget());

	/*
	 * Allocate the reserved buffers.
	 */

	i = UBC_BUF_RESERVED;
	bp = (struct buf *) h_kmem_zalloc(sizeof (struct buf) * i);	
	ubc_first_reserved = bp;
	ubc_last_reserved = bp + i - 1;
	ubc_reserved_buffers = bp;
	while (--i) {
		* (caddr_t *) bp = (caddr_t) (bp + 1);
		bp++;
	}
	* (caddr_t) bp = NULL;

	if (thread_create(first_task, &ubc_thread) != KERN_SUCCESS)
		panic("ubc_init: thread creation failed");
	ubc_dirty_thread = ubc_thread;
	thread_start(ubc_thread, ubc_dirty_thread_loop, THREAD_SYSTEMMODE);
	ubc_thread->priority = BASEPRI_USER;
	ubc_thread->sched_pri = BASEPRI_USER;
	ubc_thread->vm_privilege = TRUE;
	ubc_thread->ipc_kernel = TRUE;
	thread_swappable(ubc_thread, FALSE);
	if (thread_resume(ubc_thread) != KERN_SUCCESS)
		panic("ubc_init: resume of thread failed");

	ubc_lwc_id = lwc_create(1, &ubc_async_iodone_lwc);
	if (ubc_lwc_id == LWC_ID_NULL) panic("ubc_init: lwc_id null");
}

#if	UBC_ASYNC_TRACE
#define	UBC_ASYNC_TRACE_SIZE 200

long	ubc_atcount;

struct ubc_async_trace {
	struct buf	*at_bp;
	int		at_flags;
	struct vm_page	*at_pp;
} ubc_async_trace[UBC_ASYNC_TRACE_SIZE];

#define	ubc_atrace(BP,PP) {					\
	ubc_async_trace[ubc_atcount].at_bp = (BP);		\
	ubc_async_trace[ubc_atcount].at_flags = (BP)->b_flags;	\
	ubc_async_trace[ubc_atcount].at_pp = (PP);		\
	if (++ubc_atcount >= UBC_ASYNC_TRACE_SIZE) 		\
		ubc_atcount = 0;				\
}

#else

#define	ubc_atrace(BP,PP)

#endif	/* UBC_ASYNC_TRACE */

/*
 * Async I/O done LWC
 */

void
ubc_async_iodone_lwc()
{
	register struct buf *bp;
	register vm_page_t pp, pn;
	int error, flags;
	register int s;
	register struct vnode *vp;
	int wakeup;

	while (1) {
		s = splbio();	
		usimple_lock(&ubc_iodone_lock);
		if ((bp = ubc_iodone_buf) == BUF_NULL) {
			lwc_rfc(ubc_lwc_id);
			usimple_unlock(&ubc_iodone_lock); 
			(void) splx(s);
			return;	
		}
		if (bp->av_forw == bp) ubc_iodone_buf = BUF_NULL;
		else {
			ubc_iodone_buf = bp->av_forw;
			bremfree(bp);
		}
		usimple_unlock(&ubc_iodone_lock);
		(void) splx(s);

		pp = bp->b_pagelist;
		ubc_atrace(bp, pp);
		error = (bp->b_flags & B_ERROR) != 0;
		flags = bp->b_flags & ~B_BUSY;
		vp = bp->b_vp;
		ubc_buffree(bp);
		do {
			vm_page_lock(pp);
			pp->pg_iocnt--;
			pp->pg_error |= error;
			vm_page_unlock(pp);
			if (pp->pg_iocnt) {
				if (pp->pg_pnext) 
			panic("ubc_async_iodone_lwc: page list not empty");
				else break;
			}
			else pn = pp->pg_pnext;
			ubc_page_release(pp, 
				(flags | B_DONE |
				((pp->pg_error) ? B_ERROR : 0)));
			pp = pn;
		} while (pp);

		/*
		 * Wakeup potential v_vumoutput waiters only after
		 * all pages have been released from the I/O operation.
		 */

		if (((flags & B_READ) == 0) && (vp != NULLVP)) {
			wakeup = 0;
			s = splbio();
			VN_OUTPUT_LOCK(vp);
			vp->v_numoutput--;
			if ((vp->v_outflag & VOUTWAIT) &&
			    vp->v_numoutput <= 0) {
				vp->v_outflag &= ~VOUTWAIT;
				wakeup++;
			}
			VN_OUTPUT_UNLOCK(vp);
			splx(s);
			if (wakeup)
				thread_wakeup((vm_offset_t)&vp->v_numoutput);
		}
	}
}

int
ubc_sync_iodone(register struct buf *bp)
{
	register int error, flags;
	register vm_page_t pp, pn;

	biowait(bp);
	ubc_atrace(bp, bp->b_pagelist);

	flags = bp->b_flags & ~(B_READ|B_BUSY);
	if (bp->b_flags & B_ERROR) {
		error = bp->b_error;
		flags |= B_ERROR | B_DONE;
	}
	else {
		error = 0;
		flags |= B_DONE;
	}

	pp = bp->b_pagelist;

#if	MACH_LDEBUG
	lock_done(&bp->b_lock);
#endif	/* MACH_LDEBUG */

	ubc_buffree(bp);
	do {
		pn = pp->pg_pnext;
		vm_page_lock(pp);
		pp->pg_iocnt--;
		pp->pg_error |= (error != 0);
		vm_page_unlock(pp);
		if (pp->pg_iocnt) break;
		else ubc_page_release(pp, flags);
	} while (pp = pn);

	return error;
}

struct buf *
ubc_bufget()
{
	register struct buf *bp;
	register int incr, vm_priv;
	vm_offset_t sleepaddr;
	register thread_t thread;
		
	thread = current_thread();
	vm_priv = thread->vm_privilege; 
	bp = BUF_NULL;
	usimple_lock(&ubc_buffree_lock);
	do {
		if (ubc_free_buffers == BUF_NULL) {
			if (ubc_buf_allocated < ubc_maxbuffers) 
				incr = UBC_BUF_GROW;
			else incr = 1;
			bp = (struct buf *) h_kmem_fast_zalloc_memory(
				(caddr_t) &ubc_free_buffers,
				sizeof (struct buf), incr, FALSE);	
		}
		else {
			incr = 0;
			bp = (struct buf *) 
				h_kmem_fast_zalloc_memory(
					(caddr_t) &ubc_free_buffers,
					sizeof (struct buf), 
					UBC_BUF_GROW, FALSE);	
		}

		if (bp == NULL) {
			if (vm_priv && ubc_reserved_buffers) {
				bp = ubc_reserved_buffers;
				ubc_reserved_buffers = (struct buf *)
					(* (caddr_t *) bp);
				incr = 0;
				(void) bzero((caddr_t) bp, sizeof (struct buf));
				break;
			}

			ubc_buf_wanted++;
			if (vm_priv) {
				ubc_buf_wanted_priv++;
				sleepaddr = 
					((vm_offset_t) &ubc_buf_wanted) + 1;
			}
			else sleepaddr = (vm_offset_t) &ubc_buf_wanted;
			thread->sched_pri = BASEPRI_SYSTEM;
			mpsleep(sleepaddr, PZERO, "UBCBUFWAIT", FALSE,
				simple_lock_addr(ubc_buffree_lock), 
				MS_LOCK_ON_ERROR|MS_LOCK_SIMPLE);
		}
	} while (bp == NULL);
	ubc_buf_allocated += incr;
	ubc_iocount++;
	usimple_unlock(&ubc_buffree_lock);
	event_init(&bp->b_iocomplete);
	return bp;
}

int
ubc_buffree(register struct buf *bp)
{
	register boolean_t reserved;

	bp_mapout(bp);
	usimple_lock(&ubc_buffree_lock);

	/*
	 * Decide whether bp came from reserved pool when allocation
	 * wasn't possible for a vm_privileged thread.
	 */

	if (bp >= ubc_first_reserved && bp <= ubc_last_reserved) {
		* (caddr_t *) bp = (caddr_t) ubc_reserved_buffers;
		ubc_reserved_buffers = bp;
		reserved = TRUE;
	}

	/*
	 * Trim BPs allocated back to maximum when there aren't any wanted
	 * and/or its not a vm_priv thread.  The first condition's last
	 * term avoids allocator allocation of a free header when vm_priv
	 * has the bp which will also avoid deadlock.
	 */

	else {
		reserved = FALSE;
		if (ubc_buf_allocated <= ubc_maxbuffers || ubc_buf_wanted ||
			current_thread()->vm_privilege) 
			h_kmem_fast_free(&ubc_free_buffers, bp);
		else {
			ubc_buf_allocated--;
			h_kmem_free_memory((caddr_t) bp, 
				sizeof (struct buf), FALSE);
		}
	}
	ubc_iocount--;
	if (ubc_buf_wanted) {
		if (ubc_buf_wanted_priv) {
			ubc_buf_wanted--;
			ubc_buf_wanted_priv--;
			thread_wakeup_one(((vm_offset_t)&ubc_buf_wanted) + 1);
		}
		else if (reserved == FALSE) {
			thread_wakeup_one((vm_offset_t)&ubc_buf_wanted);
			ubc_buf_wanted--;
		}
	}
	usimple_unlock(&ubc_buffree_lock);
}

ubc_bufalloc(register vm_page_t pp, 
	register int pcnt,
	struct buf **rbp,
	vm_size_t iosize,
	int transfers,
	int flags)
{
	register struct buf *bp;
	static char page_not_busy [] = "ubc_bufalloc: page not busy";

	if (iosize == 0 || transfers <= 0) 
		panic("ubc_bufalloc: bad iosize or transfer count");

	if ((flags & B_READ) == 0) {
		register int s;
		register struct vnode *vp = 
		((struct vm_vp_object *) (pp->pg_object))->vo_vp;

		if (flags & B_UBC) {
			vpf_ladd(ubcpushes, transfers);
			vpf_ladd(ubcpagepushes, 
				round_page(iosize * transfers) / PAGE_SIZE);
		}

		s = splbio();
		VN_OUTPUT_LOCK(vp);
		vp->v_numoutput += transfers;
		VN_OUTPUT_UNLOCK(vp);
		(void) splx(s);
	}
	if (flags & B_ASYNC) flags |= B_UBC;
	
	if (transfers > 1) {
		register struct buf *fbp;
		register caddr_t baddr;

		if (pp->pg_busy ^ 1) panic(page_not_busy);
		baddr = (caddr_t) (page_to_phys(pp));
		pp->pg_iocnt = transfers;
		fbp = BUF_NULL;
		while (transfers--) {
			bp = ubc_bufget();
			bp->b_flags = flags;

#if	MACH_LDEBUG
			if ((flags & B_ASYNC) == 0) {
				lock_init(&bp->b_lock, TRUE);
				lock_write(&bp->b_lock);
			}
#endif	/* MACH_LDEBUG */

			bp->b_bcount = iosize;
			bp->b_resid = 0;
			bp->b_pagelist = pp;
			bp->b_un.b_addr = baddr;
			/* no need to check return since a single page
			 * should get mapped via KSEG0
			 */
			(void)bp_mapin(bp);
			ubc_atrace(bp, pp);
			baddr += iosize;
			if (fbp == BUF_NULL) {
				*rbp = bp;
				fbp = bp;
			}
			else {
				fbp->b_forw = bp;
				fbp = bp;
			}
		}
		bp->b_forw = BUF_NULL;
		pp->pg_pnext = VM_PAGE_NULL;
	}
	else {
		bp = ubc_bufget();
		bp->b_flags = flags;

#if	MACH_LDEBUG
		if ((flags & B_ASYNC) == 0) {
			lock_init(&bp->b_lock, TRUE);
			lock_write(&bp->b_lock);
		}
#endif	/* MACH_LDEBUG */

		bp->b_bcount = iosize;
		bp->b_resid = 0;
		bp->b_pagelist = pp;
		ubc_atrace(bp, pp);
		if (iosize > PAGE_SIZE) {
			bp->b_un.b_addr = (caddr_t) (page_to_phys(pp));
			do {
				if (pp->pg_busy ^ 1) panic(page_not_busy);
				pp->pg_iocnt = 1;
			} while (pp = pp->pg_pnext);
			if(bp_mapin(bp) != 0)
				panic("ubc_bufalloc: can't map pages");
		}
		else {
			if (pp->pg_busy ^ 1) panic(page_not_busy);
			pp->pg_iocnt = 1;
			pp->pg_pnext = VM_PAGE_NULL;
			bp->b_un.b_addr = (caddr_t) (page_to_phys(pp));
			/* no need to check return since a single page
			 * should get mapped via KSEG0
			 */
			(void)bp_mapin(bp);
		}
		*rbp = bp;
	}
	return;
}
	
/*
 * Detect excessive dirty pages in UBC
 */

ubc_dirty_memory()
{

	ubc_dirty_limit_compute;
	if (ubc_dirty_check) {
		if (!ubc_active) {
			thread_wakeup((vm_offset_t)&ubc_active);
			return;
		}
	}
	timeout(ubc_dirty_memory, (caddr_t) 0, ubc_dirty_time);
}

/*
 * The equivalent of pageout 
 * for File system pages.
 */

ubc_dirty_thread_loop()
{
	while (1) {
		assert_wait((vm_offset_t)&ubc_active, FALSE);
		timeout(ubc_dirty_memory, (caddr_t) 0, ubc_dirty_time);
		thread_block();
		ubc_memory_flushdirty();
	}
}



ubc_memory_flushdirty()
{
	register long dirtyflush;
	register struct vm_vp_object *vop;
	vm_page_t plist[VP_PAGELIST];
	register vm_page_t pp, pn;
	int status;
	register int pushes, pagestoscan;

	event_dirty;
	ubc_dirty_limit_compute;
	usimple_lock(&ubc_dirty_lock);
	if (ubc_dirty_check) {
		dirtyflush = ubc_dirtypages - ubc_dirty_limit;
		usimple_unlock(&ubc_dirty_lock);
		pushes = MIN(dirtyflush, ubc_dirty_wra);
	}
	else {
		usimple_unlock(&ubc_dirty_lock);
		return 0;
	}

	pagestoscan = (ubc_pages * ubc_dirty_percent) / 100;

top:
	usimple_lock(&ubc_lru_lock);
	for (pn = VM_PAGE_NULL, pp = ubc_lru; pushes && pagestoscan; ) {
		pagestoscan--;
		if (ubc_lru == VM_PAGE_NULL || pn == ubc_lru) {
			event_dirtyeol;
			break;
		}
		else {
			event_dirtyscan;
			if (pn) pp = pn;
			pn = pp->pg_pnext;
		}
		if (vm_object_lock_try(pp->pg_object) == FALSE) continue;

		vop = (struct vm_vp_object *) (pp->pg_object);
		if (!pp->pg_dirty) {
			vm_object_unlock(vop);
			continue;
		}

		pgl_remove(ubc_lru,pp,p);
		ubc_lru_page_count--;
		usimple_unlock(&ubc_lru_lock);

		pgl_remove(vop->vo_dirtypl,pp,o);
		pp->pg_dirty = 0;
		ubc_dirty_subtract();
		pgl_insert(vop->vo_cleanpl,pp,o);
		pmap_page_protect(page_to_phys(pp), VM_PROT_READ);    /* ok */
		pmap_clear_modify(page_to_phys(pp));
		pp->pg_busy = 1;
		pp->pg_reserved |= VPP_UBCIO;
		vm_object_unlock(vop);

		plist[0] = pp;
		pushes--;
		VOP_PUTPAGE(vop->vo_vp, plist, 1, 
			(B_UBC|B_ASYNC|B_WRITE), u.u_cred, status);
		
		vpf_ladd(ubcdirtywra,1);

		if (ubc_active) return;
		ubc_dirty_limit_compute;
		usimple_lock(&ubc_dirty_lock);
		if (ubc_dirty_check) {
			dirtyflush = ubc_dirtypages - ubc_dirty_limit;
			usimple_unlock(&ubc_dirty_lock);
			if (pushes > dirtyflush) pushes = dirtyflush;
			goto top;
		}
		else {
			usimple_unlock(&ubc_dirty_lock);
			return;
		}
	}
	usimple_unlock(&ubc_lru_lock);
}

/*
 * Note a scan_depth of zero is ubc_pages worth,
 * ie., infinity from our perspective.
 */

ubc_memory_purge(int burst,
	register int fpages,
	register int scan_depth)
{
	struct vm_vp_object *vop;
	vm_page_t plist[VP_PAGELIST];
	register vm_page_t pp, pn, pl;
	register int scans, freed = 0;
	int status;

	event_purges;
	ubc_active = TRUE;
	scans = 0;
top:

	if (ubc_pfree) {
		usimple_lock(&ubc_pfree_lock);
		while ((fpages != freed) && (pp = ubc_pfree)) {
			ubc_pfree = pp->pg_pnext;
			vm_pg_free(pp);
			freed++;
		}
		usimple_unlock(&ubc_pfree_lock);
		if (fpages == freed) {
			ubc_active = FALSE;
			return freed;
		}
	}

next:
	usimple_lock(&ubc_lru_lock);
	pn = ubc_lru;
	if (pn) pl = pn->pg_pprev;
	for (pp = VM_PAGE_NULL; fpages != freed; ) {
		if (ubc_lru == VM_PAGE_NULL || pp == pl) {
			event_meol;
			break;
		}
		else {
			event_mscans;
			if (scan_depth && (scans++ >= scan_depth)) {
				event_scan_mem;
				break;
			}
			pp = pn;
			pn = pp->pg_pnext;
		}

		vop = (struct vm_vp_object *) (pp->pg_object);

		if (vm_object_lock_try(vop) == FALSE) 
			continue;

		pgl_remove(ubc_lru,pp,p);
		ubc_lru_page_count--;

		if (pmap_verify_free(page_to_phys(pp)) == FALSE) {
			if (pp->pg_reserved & VPP_REFBIT) {
				pp->pg_reserved ^= VPP_REFBIT;
				pgl_insert_tail(ubc_lru,pp,p);
				ubc_lru_page_count++;
				ubc_page_clear_reference(pp);
				vm_object_unlock(vop);
				continue;
			}
			else if (ubc_page_referenced(pp)) {
				vpf_add(ubchit,1);
				event_memref;
				pgl_insert_tail(ubc_lru,pp,p);
				ubc_lru_page_count++;
				ubc_page_clear_reference(pp);
				vm_object_unlock(vop);
				continue;
			}
			else event_memfree;
		}

		if (pmap_page_protect(page_to_phys(pp), VM_PROT_NONE|VM_PROT_TRY) ==
		    KERN_FAILURE) {
		        pgl_insert_tail(ubc_lru,pp,p);
			ubc_lru_page_count++;
			ubc_page_clear_reference(pp);
			vm_object_unlock(vop);
			continue;
		}

		if (!pp->pg_dirty) {
			if (!pmap_is_modified(page_to_phys(pp))) {
				ubc_free_page(vop, pp, 0);
				vm_object_unlock(vop);
				freed++;
				event_totalfree;
				continue;
			}
			else if (!burst || (ubc_iocount == ubc_maxbuffers)) {
				pgl_insert(ubc_lru,pp,p);
				ubc_lru_page_count++;
				vm_object_unlock(vop);
				if (ubc_iocount == ubc_maxbuffers)
					break;
				else
					continue;
			}
			else usimple_unlock(&ubc_lru_lock);
		}
		else if (!burst || (ubc_iocount == ubc_maxbuffers)) {
			pgl_insert(ubc_lru,pp,p);
			ubc_lru_page_count++;
			vm_object_unlock(vop);
			if (ubc_iocount == ubc_maxbuffers)
				break;
			else
				continue;
		}
		else {
			usimple_unlock(&ubc_lru_lock);
			pgl_remove(vop->vo_dirtypl,pp,o);
			pp->pg_dirty = 0;
			pgl_insert(vop->vo_cleanpl,pp,o);
			ubc_dirty_subtract();
		}

		freed++;
		event_totalfree;
		pp->pg_busy = 1;
		pmap_clear_modify(page_to_phys(pp));
		pp->pg_reserved |= VPP_UBCIO;
		vm_object_unlock(vop);
		plist[0] = pp;
		burst--;
		if (!burst) event_scan_io;
		VOP_PUTPAGE(vop->vo_vp, plist, 1, 
			(B_UBC|B_FREE|B_ASYNC|B_WRITE), u.u_cred, status);

		if (ubc_pfree) goto top;
		else if (fpages != freed) goto next;
		else {
			ubc_active = FALSE;
			return freed;
		}
	}
	usimple_unlock(&ubc_lru_lock);
	ubc_active = FALSE;
	return freed;
}


/* ubc_page_stealer():
 *
 * 	Called from vm_pg_alloc() if the free list is less that vm_page_free_reserved
 *	so the calling thread was going to sleep for more pages and the ubc has the
 *	majority of the available pages.
 */

boolean_t
ubc_page_stealer()
{

	extern int vm_page_free_count;

	vm_page_t pp;
	struct vm_vp_object *vop;

	/* dont let the ubc shrink down below ubc_minpages */

	if (ubc_pages <= ubc_minpages)
		return FALSE;

	/* first try to get a page off the ubc's free list */

        if (pp = ubc_pfree) {
                usimple_lock(&ubc_pfree_lock);
                ubc_pfree = pp->pg_pnext;
                vm_pg_free(pp);
		usimple_unlock(&ubc_pfree_lock);
			/* tell caller that a page was freed */
		return TRUE;
        } else {

	/* if no free list pages try to get the oldest lru page */

		usimple_lock(&ubc_lru_lock);
		if (pp = ubc_lru) {
			do {

                                if ((cpu_to_processor(cpu_number())->runq.count > 0) ||
                                    (cpu_to_processor(cpu_number())->processor_set->runq.count > 0) ||
                                    (ubc_iodone_buf) || (swdone)) {
                                        usimple_unlock(&ubc_lru_lock);
                                        return FALSE;
                                }

				vop = (struct vm_vp_object *) (pp->pg_object);

				if (vm_object_lock_try(vop)) {
			            	if ((!pp->pg_busy) &&
			            	    (!pp->pg_hold) &&
			            	    (!pp->pg_dirty) &&
				    	    (!ubc_page_referenced(pp)) &&
			            	    (!pmap_is_modified(page_to_phys(pp))) &&
					    (pmap_page_protect(page_to_phys(pp), VM_PROT_NONE|VM_PROT_TRY) != KERN_FAILURE)) {

				     		/* take this page if it is not being used */

						pgl_remove(ubc_lru,pp,p);
						ubc_lru_page_count--;
						usimple_unlock(&ubc_lru_lock);
						ubc_free_page(vop, pp, 0);
						vm_object_unlock(vop);
							/* tell caller that a page was freed */
						return TRUE;

					} else 
						vm_object_unlock(vop);
				}
		
			} while ((pp = pp->pg_pnext) != ubc_lru); 

		}

		/* tell the caller that there were no ubc pages to give up */
		usimple_unlock(&ubc_lru_lock);	
		return FALSE;

	}

}
			
   
/*
 * Called from vn_open with vnode locked
 * and vnode type checked for VREG.
 */

ubc_object_allocate(register struct vnode *vp)
{
	struct vm_vp_object *vop;

	VN_LOCK(vp);
	if (vp->v_type != VREG) goto done;
	while (vp->v_flag & VCOBJECT) {
		vp->v_flag |= VCWAIT;
		assert_wait((vm_offset_t)&vp->v_object, FALSE);
		VN_UNLOCK(vp);
		thread_block();
		VN_LOCK(vp);
	}
	if (vp->v_object == (vm_object_t) 0) {
		vp->v_flag |= VCOBJECT;
		VN_UNLOCK(vp);
		vm_object_allocate(OT_VP, (vm_size_t) 0,
			(caddr_t) 0,  (vm_object_t *) &vop);
		VN_LOCK(vp);
		usimple_lock_init(&vop->vo_seglock);
		vp->v_object = (vm_object_t) vop;
		vop->vo_vp = vp;
		if (vp->v_flag & VCWAIT) {
			thread_wakeup((vm_offset_t)&vp->v_object);
			vp->v_flag ^= (VCOBJECT|VCWAIT);
		}
		else vp->v_flag ^= VCOBJECT;
	}
	else
		printf( "Warning: vm_obj_alloc: vp->v_object not NULL:%lx\n", vp->v_object);
done:
	VN_UNLOCK(vp);
	return;
}

/*
 * We enter with the vnode locked.
 * XXX A big assumption and currently correct.
 * This routine is called once from vclean when
 * the vnode is about to be reused.  If and when
 * forced unmounting of file systems is implemented,
 * then this routine will have to be totally rewritten.
 */

ubc_object_free(register struct vnode *vp,
	struct vnodeops *newops)
{
	register struct vm_vp_object *vop;

	vop = (struct vm_vp_object *) (vp->v_object); 
	if (vop) VN_UNLOCK(vp);
	else {
#if defined(DCEDFS) && DCEDFS
	if (!(vp->v_flag & V_CONVERTED))
		vp->v_op = newops;
#else
		vp->v_op = newops;
#endif  /* DCEDFS */
		return;
	}
	if (vop->vo_wirecnt) panic("ubc_object_free: wired page");
	if (vop->vo_npages) ubc_invalidate(vp, 0, 0, B_DONE);
	if (vop->vo_npages) panic("ubc_object_free: page(s) still resident");

	vm_object_free((vm_object_t) vop);
	VN_LOCK(vp);
	vp->v_object = VM_OBJECT_NULL;
#if defined(DCEDFS) && DCEDFS
	if (!(vp->v_flag & V_CONVERTED))
	vp->v_op = newops;
#else
	vp->v_op = newops;
#endif  /* DCEDFS */
}

ubc_page_lookup(struct vm_vp_object *vop,
	vm_offset_t offset,
	vm_page_t *pp)
{
	vm_object_lock(vop);
	*pp = vop_page_lookup(vop, offset);
	vm_object_unlock(vop);
	return;
}

/*
 * See if any page from trunc_page(start) up to 
 * round_page(start + size) are in memory.
 */

ubc_incore(struct vnode *vp,
	vm_offset_t start,
	vm_size_t size)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp;
	register vm_offset_t offset, end;

	offset = trunc_page(start);
	end = round_page(start + size);

	vop = (struct vm_vp_object *) vp->v_object;
	vm_object_lock(vop);
	for (; offset < end; offset += PAGE_SIZE) {
		pp = vop_page_lookup(vop, offset);
		if (pp != VM_PAGE_NULL) {
			vm_object_unlock(vop);
			return 1;
		}
	}
	vm_object_unlock(vop);
	return 0;
}

/*
 * Find vnode page at offset.
 * If flags is B_CACHE then only attempt to find resident page.
 * Return B_NOCACHE when not found.
 * A found page will also indicate whether its dirty
 * by set B_DIRTY in flags.  
 */

ubc_lookup(register struct vnode *vp,
	register vm_offset_t offset,
	vm_size_t blocksize,
	vm_size_t len,
	vm_page_t *ppp,
	int *flags)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp;
	boolean_t invalid;

	vop = (struct vm_vp_object *) vp->v_object;

	vm_object_lock(vop);
lookup:	
	pp = vop_page_lookup(vop, offset);
	if (pp == VM_PAGE_NULL)	{
		if (*flags & B_CACHE) {
			*flags = B_NOCACHE;
			vm_object_unlock(vop);
			*ppp = VM_PAGE_NULL;
			return 0;
		}
		else {
			*flags |= (B_LOCKED | B_BUSY);
			return ubc_page_alloc(vp, offset, blocksize, 
					len, ppp,  flags);
		}
	}
	else if (pp->pg_busy) {

		if (pp->pg_reserved & VPP_INVALID) invalid = TRUE;
		else {

			/*
			 * The page hold is released when
			 * an I/O error occurs.
			 */

			pp->pg_hold++;
			invalid = FALSE;
		}
		vm_page_wait(pp);
		vm_object_lock(vop);
		if (invalid) goto lookup;
		else if (pp->pg_error) {
			vm_object_unlock(vop);
			ubc_page_release(pp, 0);
			*ppp = VM_PAGE_NULL;
			return EIO;
		}
	}
	else {
		pp->pg_hold++;
		if ((pp->pg_hold == 1) && !pp->pg_wire_count) {
			usimple_lock(&ubc_lru_lock);
			pgl_remove(ubc_lru,pp,p);
			ubc_lru_page_count--;
			usimple_unlock(&ubc_lru_lock);
		}
	}

	*ppp = pp;
	if (pp->pg_dirty) *flags = B_DIRTY;
	else *flags = 0;
	vm_object_unlock(vop);
	return 0;
}

/*
 * Transition page from clean to dirty list
 * if its not already there.
 */

ubc_page_dirty(register vm_page_t pp)
{
	register struct vm_vp_object *vop;

	vop = (struct vm_vp_object *) (pp->pg_object);
	vm_object_lock(vop);
	if (!pp->pg_dirty) {
		if (pp->pg_wire_count) {
			pgl_remove(vop->vo_cleanwpl,pp,o);
			pgl_insert(vop->vo_dirtywpl,pp,o);
		}
		else {
			pgl_remove(vop->vo_cleanpl,pp,o);
			pgl_insert(vop->vo_dirtypl,pp,o);
			ubc_dirty_add();
		}
		pp->pg_dirty = 1;
	}
	vm_object_unlock(vop);
}

ubc_sequpdate(register struct vm_vp_object *vop,
	register vm_page_t pp)
{
#define	ubc_sequpdate(VOP,PP) 						\
	if ((VOP)->vo_nsequential) { 					\
		if ((PP)->pg_offset == 					\
			((VOP)->vo_lastpage - 				\
			ptoa((VOP)->vo_nsequential))) {			\
			(VOP)->vo_nsequential--;			\
		}							\
		else (VOP)->vo_nsequential = 0;				\
	}								\
	ubc_sequpdate(vop,pp);
}

/*
 * Allocate a page for a vnode
 * Return it held and busy if its allocated
 * otherwise its returned held.
 */

ubc_page_alloc(register struct vnode *vp,
	register vm_offset_t offset,
	register vm_size_t blocksize,
	vm_size_t len,
	vm_page_t *ppp,
	int *flags)
{
	register vm_offset_t roffset;
	register struct vm_vp_object *vop, *avop;
	register vm_page_t pp, pn, pl;
	vm_page_t page;
	boolean_t stole, hardsteal, invalid;
	register int excess;
	vm_page_t plist[1];
	int status;
	u_long stamp;
	extern int vm_page_free_count, vm_page_free_min; 

	vop = (struct vm_vp_object *) vp->v_object;
	stole = FALSE;

	if(!(*flags & B_LOCKED)) {
		vm_object_lock(vop);
		pp = vop_page_lookup(vop, offset);
		if (pp != VM_PAGE_NULL) goto pagefound;
	}

	if (((vpf_cload(pgwrites) + vpf_cload(ubcpagepushes)) > 0) &&
		ubc_pages >= ubc_seqdrain &&
		vop->vo_nsequential > 
		(excess = (ubc_pages * vm_tune_value(ubcseqpercent))/ 100)) {
		vm_offset_t loffset, lastpage;
		
		event_seqdrain;
		stamp = vop->vo_stamp;
		lastpage = vop->vo_lastpage;
		roffset = offset & ~(blocksize - 1);
		excess = vop->vo_nsequential-excess;
		excess = MIN(
			(vpf_cload(pgwrites) + vpf_cload(ubcpagepushes)),
			excess);
			
		for (loffset = vop->vo_lastpage - ptoa(vop->vo_nsequential);
			excess; loffset += PAGE_SIZE, excess--) {
			pp = vop_page_lookup(vop, loffset);
			if (pp == VM_PAGE_NULL) {
				vop->vo_nsequential--;
				continue;
			}
			else if ((pp->pg_offset >= roffset) && 
				(pp->pg_offset < (offset + len))) break;
			else if (pp->pg_busy || pp->pg_hold || 
				pp->pg_wire_count) break;

			vpf_ladd(vpseqdrain,1);
			usimple_lock(&ubc_lru_lock);
			pgl_remove(ubc_lru,pp,p);
			ubc_lru_page_count--;
			usimple_unlock(&ubc_lru_lock);

			if (pmap_page_protect(page_to_phys(pp), VM_PROT_NONE|VM_PROT_TRY)
			    == KERN_FAILURE) {
			        usimple_lock(&ubc_lru_lock);
				pgl_insert_tail(ubc_lru,pp,p);
				ubc_lru_page_count++;
				usimple_unlock(&ubc_lru_lock);
				break;
			}

			vop->vo_nsequential--;
			if (!pp->pg_dirty) {
				if (pmap_is_modified(page_to_phys(pp)) == 
					FALSE) {
					ubc_free_page(vop, pp, 0);
					continue;
				}
			}
			else {
				pgl_remove(vop->vo_dirtypl,pp,o);
				pgl_insert(vop->vo_cleanpl,pp,o);
				pp->pg_dirty = 0;
				ubc_dirty_subtract();
			}
			pp->pg_busy = 1;
			pp->pg_reserved |= VPP_UBCIO;
			vm_object_unlock(vop);
			plist[0] = pp;
			VOP_PUTPAGE(vop->vo_vp, plist, 1, 
				(B_UBC|B_ASYNC|B_WRITE|B_FREE), 
				u.u_cred, status);
			vm_object_lock(vop);
			if (lastpage != vop->vo_lastpage) break;
		}
		if (stamp != vop->vo_stamp) goto lookup;
	}

	/*
	 * Try to acquire page from lru list first
	 * when the ubc is causing excessive memory consumption.
	 */

	stamp = vop->vo_stamp;
top:
	hardsteal = FALSE;
	if (ubc_pages >= ubc_maxpages) {
		usimple_lock(&ubc_lru_lock);
hardway:
		pn = ubc_lru;
		if (pn) pl = pn->pg_pprev;
		for (pp = VM_PAGE_NULL;; ) {
			if (ubc_lru == VM_PAGE_NULL || pp == pl) {
				event_meol;
				break;
			}
			else {
				event_mscans;
				pp = pn;
				pn = pp->pg_pnext;
			}

			if (pp->pg_object != (vm_object_t) vop &&
				!vm_object_lock_try(pp->pg_object)) 
				continue;

			avop = (struct vm_vp_object *) (pp->pg_object);

			pgl_remove(ubc_lru,pp,p);
			ubc_lru_page_count--;

			if ((hardsteal == FALSE) && 
				(pmap_verify_free(page_to_phys(pp)) == FALSE)) {
				if (pp->pg_reserved & VPP_REFBIT) {
					pp->pg_reserved ^= VPP_REFBIT;
					pgl_insert_tail(ubc_lru,pp,p);
					ubc_lru_page_count++;
					ubc_page_clear_reference(pp);
					if (avop != vop) vm_object_unlock(avop);
					continue;
				}
				else if (ubc_page_referenced(pp)) {
					vpf_add(ubchit,1);
					event_memref;
					pgl_insert_tail(ubc_lru,pp,p);
					ubc_lru_page_count++;
					ubc_page_clear_reference(pp);
					if (avop != vop) vm_object_unlock(avop);
					continue;
				}
				else event_memfree;
			}

			if (pmap_page_protect(page_to_phys(pp), VM_PROT_NONE|VM_PROT_TRY)
			    == KERN_FAILURE) {
				pgl_insert_tail(ubc_lru,pp,p);
				ubc_lru_page_count++;
				ubc_page_clear_reference(pp);
				if (avop != vop) vm_object_unlock(avop);
				continue;
			}

			if (pp->pg_dirty ||
				pmap_is_modified(page_to_phys(pp))) {
				pmap_clear_modify(page_to_phys(pp));
				pp->pg_busy = 1;
				usimple_unlock(&ubc_lru_lock);

					
				if (pp->pg_dirty) {
					ubc_dirty_subtract();
					pp->pg_dirty = 0;
					pgl_remove(avop->vo_dirtypl,pp,o);
					pgl_insert(avop->vo_cleanpl,pp,o);
				}

				if (hardsteal == TRUE) {
					pp->pg_wait = 1;
					pp->pg_hold = 1;
				}

				pp->pg_reserved |= VPP_UBCIO;
				vm_object_unlock(avop);
				if (avop != vop) vm_object_unlock(vop);

				event_pushpagealloc;

				plist[0] = pp;
				VOP_PUTPAGE(avop->vo_vp, plist, 1, 
					B_AGE|B_UBC|B_WRITE|B_ASYNC, 
					u.u_cred, status);

				if (hardsteal == TRUE) vm_page_wait(pp);

				vm_object_lock(vop);
				if (stamp != vop->vo_stamp) {
					if (hardsteal == TRUE) {
						vm_object_unlock(vop);
						ubc_page_release(pp, 0);
						vm_object_lock(vop);
					}
					goto lookup;
				}
				else if (hardsteal == TRUE) {
					if (avop != vop)
						vm_object_lock(avop);
					if ((pp->pg_hold != 1) ||
						pp->pg_busy ||
						pp->pg_wire_count) {
						if (avop == vop)
							pp->pg_hold--;
						else {
							vm_object_unlock(avop);
							ubc_page_release(pp, 0);
						}
						goto top;
					}
				}
				else goto top;
			}
			else usimple_unlock(&ubc_lru_lock);


			/*
			 * Clean page and easily taken.
			 */

			event_stolepagealloc;
			vop_page_unhash(pp);
			pgl_remove(avop->vo_cleanpl,pp,o);
			ubc_sequpdate(avop,pp);
			if (avop != vop) {
				avop->vo_npages--;
				vm_object_unlock(avop);
				vop->vo_npages++;
			}
			pp->pg_busy = 1;
			pp->pg_object = (vm_object_t) vop;
			pp->pg_offset = offset;
			vop_page_hash(pp, vop, offset);
			pp->pg_pfs = 0;
			goto stolepage;
		}
		if (ubc_pages >= ubc_maxpages && hardsteal == FALSE) {
			hardsteal = TRUE;
			goto hardway;
		}
		else usimple_unlock(&ubc_lru);
	}

	if ((pp = ubc_alloc(vop, offset)) == VM_PAGE_NULL) {

		roffset = offset & ~(blocksize - 1);

		/*
		 * Before waiting for free memory,
		 * consider stealing from our own clean list.
		 * Note the private file system word in the page structure
		 * is kernel assume destructable by the UBC code.
		 */

		if (vop->vo_npages > vm_tune_value(ubcpagesteal)) {
			pp = vop->vo_cleanpl;
			vpf_ladd(vplmsteal,1);
			if (pp) do { 
				if (!pp->pg_busy && !pp->pg_hold && 
					((pp->pg_offset < roffset) || 
					(pp->pg_offset > (offset + len)))) {
					if(pmap_page_protect(page_to_phys(pp),
							VM_PROT_NONE|VM_PROT_TRY) ==
					   KERN_FAILURE) {
						pn = pp->pg_onext;
						if (pp == pn) break;
						else {
							pp = pn;
							continue;
						}
					}
					if (pmap_is_modified(page_to_phys(pp))){
						pn = pp->pg_onext;
						pgl_remove(vop->vo_cleanpl,
							pp,o);
						pp->pg_dirty = 1;
						pgl_insert(vop->vo_dirtypl,
							pp,o);
						ubc_dirty_add();
						if (pp == pn) break;
						else {
							pp = pn;
							continue;
						}
					}

					vpf_ladd(vplmstealwins,1);
					usimple_lock(&ubc_lru_lock);
					pgl_remove(ubc_lru,pp,p);
					ubc_lru_page_count--;
					usimple_unlock(&ubc_lru_lock);

					vop_page_unhash(pp);
					pp->pg_offset = offset;
					pp->pg_pfs = 0;
					vop_page_hash(pp, vop, offset);
					pgl_remove(vop->vo_cleanpl,pp,o);
					pp->pg_busy = 1;
					stole = TRUE;
					ubc_sequpdate(vop, pp);
					goto stolepage;
				}
				pp = pp->pg_onext;
				if (vop->vo_cleanpl == pp) break;
			} while (1);
		}

		vm_object_unlock(vop);
		vm_wait();

		/*
		 * See if the page was allocated while this
		 * thread was waiting for memory.
		 */

		vm_object_lock(vop);
		if (stamp == vop->vo_stamp) 
			goto top;
lookup:
		pp = vop_page_lookup(vop, offset);
		if (pp == VM_PAGE_NULL) {
			stamp = vop->vo_stamp;
			goto top;
		}
pagefound:
		if (pp->pg_busy) {
			if (pp->pg_reserved & VPP_INVALID) 
				invalid = TRUE;
			else {
				pp->pg_hold++;
				invalid = FALSE;
			}
			vm_page_wait(pp);
			vm_object_lock(vop);
			if (invalid) 
				goto lookup;
			else if (pp->pg_error) {
				vm_object_unlock(vop);
				ubc_page_release(pp, 0);
				*ppp = VM_PAGE_NULL;
				return EIO;
			}
		}
		else {
			pp->pg_hold++;
			if ((pp->pg_hold == 1) && !pp->pg_wire_count) {
				usimple_lock(&ubc_lru_lock);
				pgl_remove(ubc_lru,pp,p);
				ubc_lru_page_count--;
				usimple_unlock(&ubc_lru_lock);
			}
		}
		*ppp = pp;
		if (pp->pg_dirty) *flags = B_DIRTY;
		else *flags = 0;
		vm_object_unlock(vop);
		return 0;
	}

	vop->vo_npages++;
	pp->pg_dirty = 0;
stolepage:
	if ((vop->vo_lastpage + PAGE_SIZE) == offset) 
		vop->vo_nsequential++;
	else vop->vo_nsequential = 0;
	vop->vo_lastpage = offset;
	pp->pg_reserved = VPP_REFBIT; /* VPP_INVALID|VPP_UBCIO not set */
	pgl_insert_tail(vop->vo_cleanpl,pp,o);
	pp->pg_hold = 1;
	if ((*flags & B_BUSY) == 0) pp->pg_busy = 0;
	vop->vo_stamp++;
	vm_object_unlock(vop);
	*flags = B_NOCACHE;
	if (stole == FALSE) ubc_kmem_alloc(pp);
	*ppp = pp;
	vpf_add(ubcalloc,1); 
	return 0;
}

/*
 * Called for mmap file msync
 * Possible input flags are B_FREE and B_ASYNC
 * where B_FREE is an invalidation attempt.  
 */

extern long lw_waiters;

ubc_msync(register struct vnode *vp,
	register vm_offset_t offset,
	register vm_size_t len,
	register int flags)
{
	register struct vm_vp_object *vop;
	vm_page_t plist[1];
	register vm_page_t pp, pn, pl, *plc, *pld;
	register int s;
	int mflags, pushes, status;

	pushes = 0;
	vop = (struct vm_vp_object *) (vp->v_object);
loop:
	vm_object_lock(vop);
	if (flags & B_FREE && vop->vo_cleanpl) {
		pn = vop->vo_cleanpl;
		pl = pn->pg_oprev;
		do {
			pp = pn;
			pn = pp->pg_onext;

			if (len && !(pp->pg_offset >= offset && 
				pp->pg_offset < (offset + len))) continue;

			if (pp->pg_hold || pp->pg_busy) {
				assert_wait((vm_offset_t)pp, FALSE);
				pp->pg_wait = 1;
				vm_object_unlock(vop);
				thread_block();
				goto loop;
			}
			if (pmap_is_modified(page_to_phys(pp))) {
				pp->pg_dirty = 1;
				pgl_remove(vop->vo_cleanpl,pp,o);
				pgl_insert(vop->vo_dirtypl,pp,o);
				ubc_dirty_add();
			}
			else {
				usimple_lock(&ubc_lru_lock);
				pgl_remove(ubc_lru,pp,p);
				ubc_lru_page_count--;
				usimple_unlock(&ubc_lru_lock);
				if (pmap_page_protect(page_to_phys(pp),
				   VM_PROT_NONE|VM_PROT_TRY) == KERN_FAILURE) {
				     usimple_lock(&ubc_lru_lock);
				     pgl_insert_tail(ubc_lru,pp,p);
				     ubc_lru_page_count++;
				     usimple_unlock(&ubc_lru_lock);
				     assert_wait((vm_offset_t)&lw_waiters, FALSE);
				     lw_waiters = 1;
				     vm_object_unlock(vop);
				     thread_block();
				     if (current_thread()->wait_result
					 == THREAD_SHOULD_TERMINATE)
					   return KERN_FAILURE;
				     else
				           goto loop;
				}
				ubc_free_page(vop, pp, 0);
			}
		} while (vop->vo_cleanpl && pl != pp);
	}
	vm_object_unlock(vop);

	/*
	 * Any pages that transition from dirty to 
	 * clean or wired to unwired dirty after the dirty
	 * list is scanned will not be caught.  
	 */

loop2:
	vm_object_lock(vop);
	pld = &vop->vo_dirtypl; 
	plc = &vop->vo_cleanpl; 
	pn = *pld;
	if (pn) pl = pn->pg_oprev;
	pp = VM_PAGE_NULL;

	while (1) {
		if (*pld == VM_PAGE_NULL || pp == pl) {
			if (pld != &vop->vo_dirtywpl) {
				pld = &vop->vo_dirtywpl;
				pn = *pld;
				if (pn) {
					plc = &vop->vo_cleanwpl;
					pl = pn->pg_oprev;
					pp = VM_PAGE_NULL;
					continue;
				}
			}
			break;
		}
		else {
			pp = pn;
			pn = pp->pg_onext;
		}

		if (len && !(pp->pg_offset >= offset && 
				pp->pg_offset < (offset + len)))  continue;

		if (pp->pg_hold) {
			assert_wait((vm_offset_t)pp, FALSE);
			pp->pg_wait = 1;
			vm_object_unlock(vop);
			thread_block();
			goto loop2;
		}


		if (pld != &vop->vo_dirtywpl) {
			usimple_lock(&ubc_lru_lock);
			pgl_remove(ubc_lru,pp,p);
			ubc_lru_page_count--;
			usimple_unlock(&ubc_lru_lock);
		}

		pp->pg_busy = 1;
		pgl_remove(*pld,pp,o);
		if (pld != &vop->vo_dirtywpl) {
			ubc_dirty_subtract();
			pmap_page_protect(page_to_phys(pp), VM_PROT_READ);   /* ok */
			pmap_clear_modify(page_to_phys(pp));
		}
		pgl_insert(*plc,pp,o);
		pp->pg_dirty = 0;
		pp->pg_reserved |= VPP_UBCIO;
		vm_object_unlock(vop);


		plist[0] = pp;
		if (pp->pg_wire_count) 
			mflags = (flags & ~B_FREE) | B_MSYNC;
		else mflags = flags |  B_MSYNC;
		pushes++;
		VOP_PUTPAGE(vp, plist, 1, mflags, u.u_cred, status);
		goto loop2;	
	}
	vm_object_unlock(vop);

	if (pushes && !(flags & B_ASYNC)) {
		s = splbio();
		VN_OUTPUT_LOCK(vp);
		while (vp->v_numoutput) {
			vp->v_outflag |= VOUTWAIT;
			assert_wait((vm_offset_t)&vp->v_numoutput, FALSE);
			VN_OUTPUT_UNLOCK(vp);
			thread_block();
			VN_OUTPUT_LOCK(vp);
		}
		VN_OUTPUT_UNLOCK(vp);
		(void) splx(s);
	}

	if (pushes) 
		VOP_PUTPAGE(vp, (vm_page_t *) 0, 0, B_MSYNC, u.u_cred, status);
	return 0;
}

/*
 * Called when vnode is being reclaimed (len = 0)
 * or part of the vnode's node has been removed
 * in which case B_INVAL should be in the flags.
 * A B_DONE bit in the flags always stalls us until
 * v_numoutput transitions to zero.
 * All pages within [offset..offset+len) or 
 * [min-offset...max-offset) are recovered.
 */

ubc_invalidate(register struct vnode *vp,
	register vm_offset_t offset,
	register vm_size_t len,
	register int flags)
{
	register struct vm_vp_object *vop;
	vm_page_t plist[1];
	register vm_page_t pp, pn, pe, *pl;
	register int s;
	int dirty = 0, done, ignorebusy, status;

	ignorebusy = flags & B_BUSY;
	done = flags & B_DONE;
	flags &= ~(B_BUSY|B_DONE);
	

	VN_LOCK(vp);
	vop = (struct vm_vp_object *) (vp->v_object);
	if (vop == (struct vm_vp_object *) 0) {
		VN_UNLOCK(vp);
		return 0;		
	}
	else VN_UNLOCK(vp);

	if (len == 0 || (flags & B_INVAL))
		u_seg_uncache_vnode(vp);

	pl = &vop->vo_cleanpl;
loop:
	vm_object_lock(vop);
	pn = *pl;
	if (pn) pe = pn->pg_oprev;
	for(pp = VM_PAGE_NULL;;) {
		if (*pl == VM_PAGE_NULL || pp == pe) {
			if (pl != &vop->vo_dirtypl) {
				pl = &vop->vo_dirtypl;
				pn = *pl;
				if (pn) {
					pe = pn->pg_oprev;
					pp = VM_PAGE_NULL;
					continue;
				}
				else break;
			}
			else break;
		}
		else {
			pp = pn;
			pn = pp->pg_onext;
		}
	

		if (len && !(pp->pg_offset >= offset && 
				pp->pg_offset  < (offset + len))) continue;


		if (pp->pg_busy || pp->pg_hold) {
			if (ignorebusy) 
				continue;

			/*
			 * Not held, busy, no one waiting and
			 * data is no longer of value.
			 */

			if (!pp->pg_hold && !pp->pg_wait && (flags & B_INVAL)) {
				if (pp->pg_reserved & VPP_INVALID)
					pp->pg_reserved ^= VPP_INVALID;
				else {
					pgl_remove(vop->vo_cleanpl,pp,o);
				}
				ubc_sequpdate(vop,pp);
				vop_page_unhash(pp);
				pp->pg_reserved |= VPP_DEAD;
				vop->vo_npages--;
				continue;
			}
			assert_wait((vm_offset_t)pp, FALSE);
			pp->pg_wait = 1;
			vm_object_unlock(vop);
			thread_block();
			goto loop;
		}



		usimple_lock(&ubc_lru_lock);
		pgl_remove(ubc_lru,pp,p);
		ubc_lru_page_count--;
		usimple_unlock(&ubc_lru_lock);

		if (pmap_page_protect(page_to_phys(pp),
		      VM_PROT_NONE|VM_PROT_TRY) == KERN_FAILURE) {
		       usimple_lock(&ubc_lru_lock);
		       pgl_insert_tail(ubc_lru,pp,p);
	               ubc_lru_page_count++;
		       usimple_unlock(&ubc_lru_lock);
		       assert_wait((vm_offset_t)&lw_waiters, FALSE);
		       lw_waiters = 1;
		       vm_object_unlock(vop);
		       thread_block();
		       if (current_thread()->wait_result
			   == THREAD_SHOULD_TERMINATE)
			 return KERN_FAILURE;
		       else
			 goto loop;
		}

		if (pl == &vop->vo_cleanpl && (flags & B_INVAL) == 0) {
			if (pmap_is_modified(page_to_phys(pp)) == FALSE) {
				ubc_free_page(vop, pp, 0);
				continue;
			}
		}

		if (flags & B_INVAL) {
			ubc_free_page(vop, pp, 0);
			continue;
		}

		if (pl == &vop->vo_dirtypl) {
			pgl_remove(vop->vo_dirtypl,pp,o);
			pp->pg_dirty = 0;
			ubc_dirty_subtract();
		}
		else {
			pgl_remove(vop->vo_cleanpl,pp,o);
		}

		pp->pg_reserved |= (VPP_INVALID|VPP_UBCIO);
		pp->pg_busy = 1;
		dirty++;
		vm_object_unlock(vop);


		plist[0] = pp;
		VOP_PUTPAGE(vp, plist, 1, flags|B_INVAL|B_ASYNC, 
			u.u_cred, status);
		goto loop;	
	}

	pl = &vop->vo_cleanwpl;
	pn = *pl;
	if (pn) pe = pn->pg_oprev;
	for (pp = VM_PAGE_NULL;;) {
		if (*pl == VM_PAGE_NULL || pe == pp) {
			if (pl != &vop->vo_dirtywpl) {
				pl = &vop->vo_dirtywpl;
				pn = *pl;
				if (pn) {
					pe = pn->pg_oprev;
					pp = VM_PAGE_NULL;
					continue;
				}
				else break;
			}
			else break;
		}
		else {
			pp = pn;
			pn = pp->pg_onext;
		}

		if (len && !(pp->pg_offset >= offset && 
				pp->pg_offset  < (offset + len))) continue;


		assert_wait((vm_offset_t)pp, FALSE);
		pp->pg_wait = 1;
		vm_object_unlock(vop);
		thread_block();
		goto loop;
	}

	vm_object_unlock(vop);

	if (dirty || done) {
		s = splbio();
		VN_OUTPUT_LOCK(vp);
		while (vp->v_numoutput) {
			vp->v_outflag |= VOUTWAIT;
			assert_wait((vm_offset_t)&vp->v_numoutput, FALSE);
			VN_OUTPUT_UNLOCK(vp);
			thread_block();
			VN_OUTPUT_LOCK(vp);
		}
		VN_OUTPUT_UNLOCK(vp);
		(void) splx(s);
	}
	return dirty;
}

/*
 * Called by file system to flush dirty pages and
 * then wait for all I/O in progress to complete
 * for all pages that are marked busy.
 */

ubc_flush_sync(register struct vnode *vp)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp, *pl, pe, pn;

	ubc_flush_dirty(vp, 0);
	vop = (struct vm_vp_object *) (vp->v_object);
	pl = &vop->vo_cleanpl;
loop:
	vm_object_lock(vop);
	pn = *pl;
	if (pn) pe = pn->pg_oprev;
	for(pp = VM_PAGE_NULL;;) {
		if (*pl == VM_PAGE_NULL || pp == pe) {
			if (pl != &vop->vo_cleanwpl) {
				pl = &vop->vo_cleanwpl;
				pn = *pl;
				if (pn) {
					pe = pn->pg_oprev;
					pp = VM_PAGE_NULL;
					continue;
				}
				else break;
			}
			else break;
		}
		else {
			pp = pn;
			pn = pp->pg_onext;
		}

		/*
		 * Wait for any busy pages.
		 */

		if (pp->pg_busy) {
			pp->pg_hold++;
			pp->pg_wait = 1;
			assert_wait((vm_offset_t)pp, FALSE);
			vm_object_unlock(vop);
			thread_block();
			ubc_page_release(pp, 0);
			goto loop;
		}
	}
	vm_object_unlock(vop);
}

/*
 * Called by file system to flush dirty pages.
 */

int
ubc_flush_dirty(register struct vnode *vp,
	register int flags)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp;
	register int s;
	vm_page_t plist[1];
	int dirty = 0, status;

	VN_LOCK(vp);
	vop = (struct vm_vp_object *) (vp->v_object);
	if (vop == (struct vm_vp_object *) 0) {
		VN_UNLOCK(vp);
		return 0;		
	}
	else VN_UNLOCK(vp);
loop:
	vm_object_lock(vop);
	for (pp = vop->vo_dirtypl; pp; ) {
		if (pp->pg_busy) 
			panic("ubc_flush_dirty: dirty page is busy");
		if (pp->pg_hold) {
			assert_wait((vm_offset_t)pp, FALSE);
			pp->pg_wait = 1;
			vm_object_unlock(vop);
			thread_block((vm_offset_t) pp);
			goto loop;
		}
		usimple_lock(&ubc_lru_lock);
		pgl_remove(ubc_lru,pp,p);
		ubc_lru_page_count--;
		usimple_unlock(&ubc_lru_lock);
		pp->pg_busy = 1;
		pgl_remove(vop->vo_dirtypl,pp,o);
		ubc_dirty_subtract();
		pmap_page_protect(page_to_phys(pp), VM_PROT_READ);  /* ok */
		pmap_clear_modify(page_to_phys(pp));
		pgl_insert(vop->vo_cleanpl,pp,o);
		pp->pg_dirty = 0;
		pp->pg_reserved |= VPP_UBCIO;
		vm_object_unlock(vop);
		dirty++;

		plist[0] = pp;
		VOP_PUTPAGE(vp, plist, 1, flags|B_ASYNC, u.u_cred, status);
		goto loop;	
	}

	vm_object_unlock(vop);

	if (((flags & B_ASYNC) == 0) && dirty) {
		s = splbio();
		VN_OUTPUT_LOCK(vp);
		while (vp->v_numoutput) {
			vp->v_outflag |= VOUTWAIT;
			assert_wait((vm_offset_t)&vp->v_numoutput, FALSE);
			VN_OUTPUT_UNLOCK(vp);
			thread_block();
			VN_OUTPUT_LOCK(vp);
		}
		VN_OUTPUT_UNLOCK(vp);
		(void) splx(s);
	}
	return dirty;
}

/*
 * Called by file system to determine the start and length of
 * dirty pages for a vnode.
 */

ubc_count_dirty(register struct vnode *vp,
		vm_offset_t *start,
		vm_size_t *length)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp;
	register vm_offset_t min, max;

	min = max = *start = *length = 0;
	VN_LOCK(vp);
	vop = (struct vm_vp_object *) (vp->v_object);
	if (vop == (struct vm_vp_object *) 0) {
		VN_UNLOCK(vp);
		return 0;		
	} else
		VN_UNLOCK(vp);

	vm_object_lock(vop);
	pp = vop->vo_dirtypl;
	if (pp != VM_PAGE_NULL) {
		min = max = pp->pg_offset;
		*length = PAGE_SIZE;
		for (pp = pp->pg_onext; pp != vop->vo_dirtypl;
		     pp = pp->pg_onext) {
			if (pp->pg_offset < min)
				min = pp->pg_offset;
			else if (pp->pg_offset > max)
				max = pp->pg_offset;
		}
	}
	vm_object_unlock(vop);

	if (min || max) {
		*start = min;
		*length = (max - min) + PAGE_SIZE;
	}
}
	
ubc_page_release(register vm_page_t pp,
	register int flags)
{
	register struct vm_vp_object *vop;
	static char illegal_wait[] = "ubc_page_release: illegal wait condition";

#define	ubc_release_wakeup() 						\
	if (pp->pg_wait) {						\
		pp->pg_wait = 0;					\
		thread_wakeup((vm_offset_t)pp);				\
	}

	vop = (struct vm_vp_object *) (pp->pg_object);
	vm_object_lock(vop);
	vm_page_lock(pp);


	/*
	 * Releasing a page that was previously busy.
	 */

	if (flags & B_DONE) {

		pp->pg_busy = 0;
		pp->pg_reserved &= ~VPP_UBCIO;

		if (flags & B_BUSY) pp->pg_hold--;

		/*
		 * Page in dead state.
		 */

		if (pp->pg_reserved & VPP_DEAD) {
			vm_page_unlock(pp);
			vm_object_unlock(vop);
			ubc_free_memory(pp);
			return;
		}

		/*
		 * If there was an I/O error and there isn't any
		 * wirings then disassociate the object with the page.
		 * All waiters (hold > 0) are expected to release the page
		 * for eventual deletion.
		 * If its wired memory then it had to be a write.
		 * In this case leave the memory resident and
		 * transition the page back to the dirty list. 
		 */

		else if ((flags & (B_ERROR|B_INVAL)) == B_ERROR) {
			if (pp->pg_hold && !pp->pg_wire_count) {
				vop_page_unhash(pp);
				pp->pg_error = 1;
				if (!pp->pg_dirty) {
					pgl_remove(vop->vo_cleanpl,pp,o);
				}
				else {
					pgl_remove(vop->vo_dirtypl,pp,o);
					ubc_dirty_subtract();
				}
				ubc_release_wakeup();
				vm_page_unlock(pp);
			}
			else if (pp->pg_wire_count) {
				if (!pp->pg_dirty) {
					pgl_remove(vop->vo_cleanwpl,pp,o);
					pgl_insert(vop->vo_dirtywpl,pp,o);
					pp->pg_dirty = 1;
				}
				ubc_release_wakeup();
				vm_page_unlock(pp);
			}
			else {
				ubc_release_wakeup();
				vm_page_unlock(pp);
				ubc_free_page(vop, pp, 0);
			}
			vm_object_unlock(vop);
			return;
		}
		else if (flags & B_FREE) {
			ubc_release_wakeup();
			vm_page_unlock(pp);
			if (!pp->pg_hold) {
				ubc_free_page(vop, pp, 0);
				vm_object_unlock(vop);
			}
			else {
				vm_object_unlock(vop);
				vpf_ladd(ubcreclaim,1);
			}
			return;
		}
		else if (flags & B_INVAL) {
			assert(pp->pg_hold == 0);
			ubc_release_wakeup();
			vm_page_unlock(pp);
			ubc_free_page(vop, pp, B_INVAL);
			vm_object_unlock(vop);
			return;
		}
		else ubc_release_wakeup();
	} 
	else {
		pp->pg_hold--;
		if (flags & B_FREE) {
			if (pp->pg_busy || pp->pg_hold || 
				pp->pg_wire_count || pp->pg_wait)
					panic("ubc_page_release: B_FREE bad");
			vm_page_unlock(pp);
			ubc_free_page(vop, pp, 0);
			vm_object_unlock(vop);
			return;
		}
		else if (!pp->pg_busy) ubc_release_wakeup();
	}

	if (!pp->pg_hold && !pp->pg_busy) {
		if (pp->pg_error && !pp->pg_wire_count) {
			vm_page_unlock(pp);
			ubc_free_page(vop, pp, B_ERROR);
			vm_object_unlock(vop);
			return;
		}
		if (!pp->pg_wire_count) {
			usimple_lock(&ubc_lru_lock);
			if (flags & B_AGE) {
				pgl_insert(ubc_lru,pp,p);
			}
			else {
				pgl_insert_tail(ubc_lru,pp,p);
			}
			ubc_lru_page_count++;
			usimple_unlock(&ubc_lru_lock);
		}
			
	}

	if ((flags & B_DIRTY) && !pp->pg_dirty) {
		if (pp->pg_busy) 
			panic("ubc_page_release: illegal page transition");
		if (!pp->pg_wire_count) {
			pgl_remove(vop->vo_cleanpl,pp,o);
			pgl_insert_tail(vop->vo_dirtypl,pp,o);
			ubc_dirty_add();
		}
		else {
			pgl_remove(vop->vo_cleanwpl,pp,o);
			pgl_insert(vop->vo_dirtywpl,pp,o);
		}
		pp->pg_dirty = 1;
	}

	vm_page_unlock(pp);
	vm_object_unlock(vop);

#undef	ubc_release_wakeup
}

#if	UBC_FREE_TRACE
#define	UBC_FREE_TRACE_SIZE 200

long ubc_free_trace_index = 0;
struct ubc_free_trace_data {
	long uft_pc;
	struct vm_vp_object *uft_vop;
	vm_page_t uft_pp;
	int uft_flags;
} ubc_free_trace_data[UBC_FREE_TRACE_SIZE];

#define	ubc_free_trace(VOP,PP,FLAGS) {					\
	ubc_free_trace_data[ubc_free_trace_index].uft_pc =		\
		get_caller_ra();					\
	ubc_free_trace_data[ubc_free_trace_index].uft_vop = (VOP);	\
	ubc_free_trace_data[ubc_free_trace_index].uft_pp = (PP);	\
	ubc_free_trace_data[ubc_free_trace_index].uft_flags = (FLAGS);	\
	if (++ubc_free_trace_index >= UBC_FREE_TRACE_SIZE)		\
		ubc_free_trace_index =  0;				\
}

#else

#define	ubc_free_trace(VOP,PP,FLAGS)

#endif	/* UBC_FREE_TRACE */

/*
 * Enter with vop lock and leave with it unlocked.
 */

ubc_free_page(register struct vm_vp_object *vop,
	register vm_page_t pp,
	int flags)
{
	ubc_free_trace(vop,pp,flags);

	ubc_sequpdate(vop,pp);

	if ((flags & B_ERROR) == 0) 
		vop_page_unhash(pp);

	if ((flags & (B_INVAL|B_ERROR)) == 0) {
		if (!pp->pg_dirty) {
			pgl_remove(vop->vo_cleanpl,pp,o);
		}
		else {
			pgl_remove(vop->vo_dirtypl,pp,o);
			ubc_dirty_subtract();
		}
	}
	vop->vo_npages--;
	ubc_free_memory(pp);
}

ubc_free_memory(register vm_page_t pp)
{
	if (ubc_kmem_cache(pp)) {
		usimple_lock(&ubc_pfree_lock);
		if (ubc_pfree) pp->pg_pnext = ubc_pfree;
		else pp->pg_pnext = VM_PAGE_NULL;
		ubc_pfree = pp;	
		usimple_unlock(&ubc_pfree_lock);
	}
	else {
		ubc_kmem_free(pp);
		usimple_lock(&ubc_memory_lock);
		ubc_pages--;
		vpf_store(ubcpages,ubc_pages);
		usimple_unlock(&ubc_memory_lock);
		vm_pg_free(pp);
	}
}

vm_page_t
ubc_alloc(struct vm_vp_object *vop,
	vm_offset_t offset)
{
	register vm_page_t pp;
	vm_page_t page;
	register boolean_t doinit;

	pp = VM_PAGE_NULL;
	doinit = FALSE;
	if (ubc_pfree) {
		usimple_lock(&ubc_pfree_lock);
		if ((pp = ubc_pfree) != VM_PAGE_NULL) {
			ubc_pfree = pp->pg_pnext;
			doinit = TRUE;
		}
		usimple_unlock(&ubc_pfree_lock);
	}

	if ((pp != VM_PAGE_NULL) || 
		(vm_pg_alloc(&page), ((pp = page) != VM_PAGE_NULL))) {
		if (doinit == TRUE) 
			vm_page_init(pp, VM_OBJECT_NULL, (vm_offset_t) 0);
		else {
			usimple_lock(&ubc_memory_lock);
			ubc_pages++;
			vpf_store(ubcpages,ubc_pages);
			usimple_unlock(&ubc_memory_lock);
		}
		pp->pg_offset = offset;
		pp->pg_object = (vm_object_t) vop;
		vop_page_hash(pp, vop, offset);
		pmap_clear_modify(page_to_phys(pp));
	}
	return pp;
}

/*
 * The page is assumed held and it isn't on the lru list
 * of active ubc pages.  Wired pages require the wiree
 * to do the actual msynch to force the dirty pages out.
 */

ubc_wire(register vm_page_t *pl,
	register int pcnt,
	boolean_t release)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp;

	vop = (struct vm_vp_object *) ((*pl)->pg_object);
	vm_object_lock(vop);
	for (pp = *pl; pcnt--; pl++, pp = *pl) {
		if (!pp->pg_wire_count) {
			if (!pp->pg_dirty) {
				pgl_remove(vop->vo_cleanpl,pp,o);
				pgl_insert(vop->vo_cleanwpl,pp,o);
			}
			else {
				ubc_dirty_subtract();
				pgl_remove(vop->vo_dirtypl,pp,o);
				pgl_insert(vop->vo_dirtywpl,pp,o);
			}
			usimple_lock(&ubc_memory_lock);
			ubc_pages--;
			vpf_store(ubcpages,ubc_pages);
			usimple_unlock(&ubc_memory_lock);
			simple_lock(&vm_page_wire_count_lock);
			vm_page_wire_count++;
			vpf_store(wiredpages,vm_page_wire_count);
			simple_unlock(&vm_page_wire_count_lock);
			vop->vo_wirecnt++;
		}
		pp->pg_wire_count++;
		if (release == TRUE) pp->pg_hold--;
	}
	vm_object_unlock(vop);
}

ubc_unwire(register vm_page_t *pl,
	register int pcnt)
{
	register struct vm_vp_object *vop;
	register vm_page_t pp;

	vop = (struct vm_vp_object *) ((*pl)->pg_object);
	vm_object_lock(vop);
	for (pp = *pl;pcnt--; pl++, pp = *pl) {
		vm_page_lock(pp);
		if (pp->pg_wire_count <= 0) panic("ubc_unwire: not wired");
loop:
		if (pp->pg_wire_count == 1) {

			/*
			 * The only busy condition should be an msync
			 * of a dirty page.
			 */

			if (pp->pg_busy || pp->pg_hold) {
				assert_wait((vm_offset_t)pp, FALSE);
				vm_page_unlock(pp);
				vm_object_unlock(vop);
				thread_block();
				vm_object_lock(vop);
				vm_page_lock(pp);
				goto loop;
			}
			pp->pg_wire_count = 0;

			if (!pp->pg_dirty) {
				pgl_remove(vop->vo_cleanwpl,pp,o);
				if (pmap_is_modified(page_to_phys(pp))) {
					pmap_clear_modify(page_to_phys(pp));
					pp->pg_dirty = 1;
					pgl_insert_tail(vop->vo_dirtypl,pp,o);
					ubc_dirty_add();
				}
				else {
					pmap_page_protect(page_to_phys(pp),  /* ok */
						VM_PROT_READ);
					pgl_insert(vop->vo_cleanpl,pp,o);
				}
			}
			else {
				pgl_remove(vop->vo_dirtywpl,pp,o);
				pgl_insert_tail(vop->vo_dirtypl,pp,o);
				ubc_dirty_add();
			}

			vop->vo_wirecnt--;
			usimple_lock(&ubc_memory_lock);
			ubc_pages++;
			vpf_store(ubcpages,ubc_pages);
			usimple_unlock(&ubc_memory_lock);
			simple_lock(&vm_page_wire_count_lock);
			vm_page_wire_count--;
			vpf_store(wiredpages,vm_page_wire_count);
			simple_unlock(&vm_page_wire_count_lock);

			/*
			 * An invalidate will wait for the page to
			 * transition to not wired.
			 */

			if (pp->pg_wait) {
				pp->pg_wait = 0;
				thread_wakeup((vm_offset_t)pp);	
			}

			if  (pp->pg_hold == 0) {
				usimple_lock(&ubc_lru_lock);
				pgl_insert_tail(ubc_lru,pp,p);
				ubc_lru_page_count++;
				usimple_unlock(&ubc_lru_lock);
			}
		}
		else pp->pg_wire_count--;
		vm_page_unlock(pp);
	}
	vm_object_unlock(vop);
}

/*
 * Use to stage I/O.  Note the page transitions
 * from the dirty list to the clean list if dirty
 * is true.  We don't allow busy pages on the dirty list.
 * This assumes that the caller is about to do a write into
 * the page's file system.
 */

ubc_page_busy(register vm_page_t pp,
	register int pcnt)
{
	register int i;
	register struct vm_vp_object *vop;

	vop = (struct vm_vp_object *) (pp->pg_object);

	vm_object_lock(vop);
	for (i = 0; i < pcnt; pp = pp->pg_pnext, i++) {
		vm_page_lock(pp);
		if (!pp->pg_hold || pp->pg_busy) 
			panic("ubc_page_busy: hold == 0 or page busy");
		while (pp->pg_hold > 1) {
			pp->pg_wait = 1;
			assert_wait((vm_offset_t)pp, FALSE);
			vm_page_unlock(pp);
			vm_object_unlock(vop);
			thread_block();
			vm_object_lock(vop);
			vm_page_lock(pp);	
		}
		assert(pp->pg_hold == 1);
		pp->pg_busy = 1;
		if (pp->pg_dirty) {
			pp->pg_dirty = 0;
			if (pp->pg_wire_count) {
				pgl_remove(vop->vo_dirtywpl,pp,o);
				pgl_insert(vop->vo_cleanwpl,pp,o);
			}
			else {
				ubc_dirty_subtract();
				pgl_remove(vop->vo_dirtypl,pp,o);
				pgl_insert(vop->vo_cleanpl,pp,o);
			}
		}
		vm_page_unlock(pp);
	}
	vm_object_unlock(vop);
}

static vm_page_t
ubc_dirty_page(register struct vm_vp_object *vop,
		vm_offset_t offset,
		int flags,
		boolean_t hold)
{
	register vm_page_t pp;


	pp = vop_page_lookup(vop, offset);
	if (pp == VM_PAGE_NULL) return pp;

	/*
	 * Page is on the clean list.
	 */

	if (!pp->pg_dirty) return (vm_page_t) 0;

	/*
	 * If B_WANTED and page is not available then wait for it.
	 */

	if (pp->pg_busy || pp->pg_hold) {
		if ((flags & B_WANTED) == 0) return (vm_page_t) 0;
		else do {
			pp->pg_wait = 1;
			assert_wait((vm_offset_t) pp, FALSE);
			vm_object_unlock(vop);
			thread_block();
			vm_object_lock(vop);

			pp = vop_page_lookup(vop, offset);
			if (pp == VM_PAGE_NULL) return pp;

			/*
			 * Page transitioned to clean so no longer candidate
			 */

			if (!pp->pg_dirty) return VM_PAGE_NULL;
		} while (pp->pg_busy || pp->pg_hold);

		}

	if (pp->pg_wire_count) {
		if (flags & (B_FREE|B_INVAL)) return (vm_page_t) 0;
		pgl_remove(vop->vo_dirtywpl,pp,o);
		pgl_insert(vop->vo_cleanwpl,pp,o);
	}
	else {
loop:
		usimple_lock(&ubc_lru_lock);
		pgl_remove(ubc_lru,pp,p);
		ubc_lru_page_count--;
		usimple_unlock(&ubc_lru_lock);
	        if(flags & (B_FREE|B_INVAL))
		     if(pmap_page_protect(page_to_phys(pp),
			  VM_PROT_NONE|VM_PROT_TRY) == KERN_FAILURE) {
		          if ((flags & B_WANTED) == 0) {
			       usimple_lock(&ubc_lru_lock);
			       pgl_insert_tail(ubc_lru,pp,p);
	                       ubc_lru_page_count++;
			       usimple_unlock(&ubc_lru_lock);
			       return (vm_page_t)0;
			     } 
			  else {
			       usimple_lock(&ubc_lru_lock);
			       pgl_insert_tail(ubc_lru,pp,p);
	                       ubc_lru_page_count++;
			       usimple_unlock(&ubc_lru_lock);
			       assert_wait((vm_offset_t)&lw_waiters, FALSE);
			       lw_waiters = 1;
			       vm_object_unlock(vop);
			       thread_block();
			       if (current_thread()->wait_result
				   == THREAD_SHOULD_TERMINATE)
				    return VM_PAGE_NULL;
			       else {
				    vm_object_lock(vop);
				    goto loop;
			       }
			  }
		     }
		else
		     pmap_page_protect(page_to_phys(pp), VM_PROT_READ);
		pgl_remove(vop->vo_dirtypl,pp,o);
		ubc_dirty_subtract();
		if ((flags & B_INVAL) == 0) pgl_insert(vop->vo_cleanpl,pp,o);
	}
	pmap_clear_modify(page_to_phys(pp));
	pp->pg_dirty = 0;
	pp->pg_busy = 1;
	if (hold == TRUE) pp->pg_hold = 1;
	return pp;
}

/*
 * Collect into a sorted list
 * all pages possible between trunc_page(start) (inclusive)
 * and round_page(start + size) (exclusive).  
 * Each page found is marked busy
 * and transitioned to the clean list.   
 * Optionally the page may also be held.
 * Specify B_WANTED with the flags argument causes the caller to
 * wait for B_BUSY and/or held pages.  
 * B_FREE|B_INVAL isn't allowed on wired pages.
 * It is assumed these pages are being scheduled for a write
 * and all references in the pmap subsystem are updated
 * according to the flags passed.
 * An optional center page can be passed which is already in
 * a busy state.  Note the hold count on the
 * center page must be one if hold is true.  
 * If the center page passed isn't on
 * the clean list, then the pmap is updated and the page is
 * place on the clean list.
 * Also hold and B_BREE|B_INVAL are mutually exclusive operations.
 *
 */

vm_page_t
ubc_dirty_kluster(struct vnode *vp,
		register vm_page_t center,
		register vm_offset_t start,
		vm_size_t size,
		int flags,
		boolean_t hold,
		int *pcnt)
{
	register struct vm_vp_object *vop;
	register vm_page_t fp, pp, lp, pn;
	register vm_offset_t offset, end;
	register int pages;

	if (hold == TRUE && flags & (B_FREE|B_INVAL)) 
		panic("ubc_dirty_kluster: hold and B_BREE mutually exclusive");

	vop = (struct vm_vp_object *) vp->v_object;
	end = round_page(start + size);
	start = trunc_page(start);

	
	vm_object_lock(vop);

	if (center != VM_PAGE_NULL) { 

		/*
		 * If this center page didn't originate from the UBC,
		 * then we must be certain that the hold count is
		 * one if the page isn't busy.  Note UBC doesn't do
		 * VOP_PUTPAGE operations until the hold count goes to
		 * zero.
		 */

		if (!(center->pg_reserved & VPP_UBCIO) && 
			!center->pg_busy && (center->pg_hold > 1))
			do {
				assert_wait((vm_offset_t) center, FALSE);
				center->pg_wait = 1;
				vm_object_unlock(vop);
				thread_block();
				vm_object_lock(vop);
			} while (center->pg_hold > 1);

		/*
		 * A non UBC center page target may be on the dirty list.
		 * The correct dirty list management must be processed first.
		 */

		if (center->pg_dirty) {

		       if(flags & (B_FREE|B_INVAL)) {
			      if(pmap_page_protect(page_to_phys(center),
				   VM_PROT_NONE|VM_PROT_TRY) == KERN_FAILURE)
				     do {
				            assert_wait((vm_offset_t)&lw_waiters,
						FALSE);
					    lw_waiters = 1;
					    vm_object_unlock(vop);
					    thread_block();
					    if (current_thread()->wait_result
						== THREAD_SHOULD_TERMINATE)
					         return VM_PAGE_NULL;
					    else 
					         vm_object_lock(vop);
				     } while (pmap_page_protect(
					page_to_phys(center),
					VM_PROT_NONE|VM_PROT_TRY) == 
					      KERN_FAILURE);
		       }	
		       else
			      pmap_page_protect(page_to_phys(center),
				VM_PROT_READ);

		       pgl_remove(vop->vo_dirtypl,center,o);
		       pgl_insert(vop->vo_cleanpl,center,o);
		       ubc_dirty_subtract();
		       center->pg_dirty = 0;
		       pmap_clear_modify(page_to_phys(center));
		}


		/*
		 * If this center page didn't originate from UBC and the
		 * caller doesn't want to hold the pages after I/O, then
		 * release the center page hold value.
		 */

		if (!(center->pg_reserved & VPP_UBCIO) && (hold == FALSE))
			center->pg_hold--;

		center->pg_busy = 1;
		offset = center->pg_offset + PAGE_SIZE;
		fp = center;
		pages = 1;
	}
	else {
		offset = start;
		fp = VM_PAGE_NULL;
		pages = 0;
	}

	if (start == end) {
		vm_object_unlock(vop);
		if (center) {
			if (center->pg_offset != start)
				panic("ubc_dirty_kluster: invalid request");
			*pcnt = 1;
			center->pg_pnext = VM_PAGE_NULL;
			return center;	
		}
		else {
			*pcnt = 0;
			return (vm_page_t) 0;
		}
	}

	for (lp = VM_PAGE_NULL; offset < end; offset += PAGE_SIZE) {
		if ((pp = ubc_dirty_page(vop, offset, flags, hold)) ==
			VM_PAGE_NULL) {
			if (fp != VM_PAGE_NULL) break;
			else continue;
		}
		pages++;
		if (fp == VM_PAGE_NULL) fp = pp;
		else {
			if (lp != VM_PAGE_NULL) lp->pg_pnext = pp;
			else fp->pg_pnext = pp;
		}
		lp = pp;
	}

	if (center && center->pg_offset) for(pn = VM_PAGE_NULL,
		offset = center->pg_offset - PAGE_SIZE; 
		offset >= start;
		offset -= PAGE_SIZE) {
		if ((pp = ubc_dirty_page(vop, offset, flags, hold)) ==
			VM_PAGE_NULL) break;
		fp = pp;
		if (pn != VM_PAGE_NULL) fp->pg_pnext = pn;
		else {
			fp->pg_pnext = center;
			if (lp == VM_PAGE_NULL) lp = center;
		}
		pages++;
		pn = pp;
		if (offset == (vm_offset_t) 0) break;
	}

	vm_object_unlock(vop);

	*pcnt = pages;
	if (fp) {
		if (lp && (fp != lp)) {
			fp->pg_pprev = lp;
			lp->pg_pnext =  VM_PAGE_NULL;
		}
		else fp->pg_pprev = fp->pg_pnext = VM_PAGE_NULL;
	}
	return fp;
}

static vm_page_t
ubc_clean_page(register struct vm_vp_object *vop,
		vm_offset_t offset,
		vm_offset_t bsize,
		vm_size_t len,
		int flags,
		int hold)
{
	int aflags, error;
	vm_page_t app;
	register vm_page_t pp;

	vm_object_lock(vop);
	pp = vop_page_lookup(vop, offset);
	if (pp == VM_PAGE_NULL) {
		if ((flags & B_WANTFREE) == 0) {
			vm_object_unlock(vop);
			return pp;
		}
		aflags = B_LOCKED | (flags & B_BUSY); 	
		error = ubc_page_alloc(vop->vo_vp, offset, bsize, len, 
				&app, &aflags);
		if (error) panic("ubc_clean_page: ubc_page_alloc");
		pp = app;
		vm_object_lock(vop);
		if (((aflags & B_NOCACHE) == 0) && 
			((pp->pg_hold != 1) || pp->pg_wire_count)) {
			vm_object_unlock(vop);
			ubc_page_release(pp, 0);
			return (vm_page_t) 0;
		}
	}
	else {
		if (pp->pg_busy || pp->pg_hold || pp->pg_dirty ||
			pp->pg_wire_count ||
			(flags & B_CACHE)) {
			vm_object_unlock(vop);
			return (vm_page_t) 0;
		}
		else {
			usimple_lock(&ubc_lru_lock);
			pgl_remove(ubc_lru,pp,p);
			ubc_lru_page_count--;
			usimple_unlock(&ubc_lru_lock);
		}
	}

	pp->pg_busy = ((flags & B_BUSY) != 0);
	pp->pg_hold = (hold != UBC_HNONE);
	vm_object_unlock(vop);
	return pp;
}

/*
 * Creating a kluster list of pages.
 * Any page marked busy and/or having a hold count is ignored.
 * Also pages on the vop's dirty list are excluded.
 *
 * The flags use is a follows:
 *	B_CACHE		-	stop search if page is already cached
 *	B_WANTFREE	- 	allocate a page if the page isn't cached
 *	B_BUSY		-	mark pages busy for I/O
 *			
 */

vm_page_t
ubc_kluster(struct vnode *vp,
	vm_page_t center,
	vm_offset_t start,
	vm_size_t  size,
	int flags,
	int hold,
	int *pcnt)
{
	register struct vm_vp_object *vop;
	register int pages;
	register vm_offset_t end, offset;
	register vm_page_t fp, lp, pp, pn;
	
	if ((flags & B_BUSY) && center && (center->pg_busy ^ 1))
		panic("ubc_kluster: center page not busy");

	end = round_page(start + size);
	start = trunc_page(start);
	size = end - start;

	if (start == end) {
		if (center) {
			if (center->pg_offset != start)
				panic("ubc_kluster: invalid request");
			*pcnt = 1;
			center->pg_pnext = VM_PAGE_NULL;
			return center;
		}
		else {
			*pcnt = 0;
			return (vm_page_t) 0;
		}
	}

	vop = (struct vm_vp_object *) vp->v_object;

	if (center) {
		offset = center->pg_offset + PAGE_SIZE;
		fp = center;
		pages = 1;
			
	}
	else {
		offset = start;
		fp = VM_PAGE_NULL;
		pages = 0;
	}
	
	for (lp = VM_PAGE_NULL; offset < end; offset += PAGE_SIZE) {
		if ((pp = ubc_clean_page(vop, offset, 
			size, end - offset, flags, 
			(center ? (hold & UBC_HACP) : hold))) 
			== VM_PAGE_NULL) break;
		pages++;
		if (fp == VM_PAGE_NULL) fp = pp;
		else {
			if (lp != VM_PAGE_NULL) lp->pg_pnext = pp;
			else fp->pg_pnext = pp;
		}
		lp = pp;
	}

	if (center && center->pg_offset) for(pn = VM_PAGE_NULL,
		offset = center->pg_offset - PAGE_SIZE; 
		offset >= start;
		offset -= PAGE_SIZE) {
		if ((pp = ubc_clean_page(vop, offset, 
			size, end - offset, flags, 
			(center ? (hold & UBC_HBCP) : hold))) 
			== VM_PAGE_NULL) break;
		pages++;
		fp = pp;
		if (pn != VM_PAGE_NULL) fp->pg_pnext = pn;
		else {
			fp->pg_pnext = center;
			if (lp == VM_PAGE_NULL) lp = center;
		}
		pn = pp;
		if (offset == (vm_offset_t) 0) break;
	}

	*pcnt = pages;
	if (fp) {
		if (lp && (fp != lp)) {
			fp->pg_pprev = lp;
			lp->pg_pnext =  VM_PAGE_NULL;
		}
		else fp->pg_pprev = fp->pg_pnext = VM_PAGE_NULL;
	}
	return fp;
}
