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
static char	*sccsid = "@(#)$RCSfile: lv_mircons.c,v $ $Revision: 4.3.3.5 $ (DEC) $Date: 1993/01/08 18:00:50 $";
#endif 
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
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_mircach.c
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  lv_mircons.c -- LVM device driver write consistency cache routines
 *
 *	These routines handle the mirror write consistency cache used
 *	to maintain a record of possible mirrors that may be inconsistent
 *	if the system crashes.  They are part of the bottom
 *	half of the device driver.
 *
 *  Function:
 *
 *  Execution environment:
 *
 *	All these routines run on interrupt levels, so they are not
 *	permitted to block or access the user process.  They run within
 * 	critical sections that are serialized with block I/O offlevel
 * 	biodone() processing.
 */

/*
 *  Modification History:  lv_mircons.c
 *
 *  18-Sep-91     Terry Carruthers
 *	Modified lv_cache_recover to correctly implement a call to 
 *      the lv_recover_ltg routine and to guarantee that open logical
 *      volumes are used in the mirror consistency record recovery
 *      operations.
 *
 *      Modified lv_recover_ltg to correctly calculate block number of 
 *      logical track group for mirror consistency record.
 *
 */

#include <lvm/lvmd.h>		/* LVM device driver internal declares */

#include <sys/errno.h>
#include <sys/conf.h>

#define MWC_DEBUG 0

/*
 *  NAME:	lv_mwcm
 *
 *  FUNCTION:	Manage the mirror write consistency cache.
 *
 *  NOTES:
 *	input:	List of logical buf structs to check for mirror consistency.
 *
 *	output:	All requests that were on the queue are moved to 1 of
 *		3 queues as follows:
 *
 *		sched_Q - scheduling queue - this queue is immediately
 *			passed to the scheduler layer.
 *
 *		    requests ready for logical to physical translation
 *		    and scheduling to disk drivers.
 *
 *		    * Reads from LVs
 *			ALL
 *
 *		    * Writes to LVs
 *			with only 1 active copy
 *			OR that don't want write consistency
 *			OR that are in the cache (dirty)
 *
 *		vg_cache_wait - cache wait queue - 1 per VG
 *
 *		    requests that can not be allowed to proceed further
 *		    because
 *
 *		    * The cache is in flight and
 *			it is not in the cache
 *
 *		    When the cache becomes available these requests will
 *		    move to a pv_cache_wait, or rejected.
 *
 *		pv_cache_wait  - PV cache write wait queue - 1 per PV
 *
 *		    requests that are suspended waiting on the cache
 *		    write to complete for the PV.
 *
 *		    When the write is successfully completed these requests
 *		    will be moved to sched_Q.
 *
 *  PARAMETERS:   none
 *
 *  RETURN VALUE: none
 *
 */
lv_mwcm(lb)
register struct buf *lb;
{
	register struct buf	*next;
	register struct lvol	*lv;	/* logical volume structure	   */
	register int mircnt;		/* number of active mirrors	   */
	struct lv_queue sched_Q;

	struct volgrp	*vg;		/* VG volgrp ptr from devsw table  */

	LV_QUEUE_INIT(&sched_Q);
	/*
	 *  Remove and process the next logical buf on the input list
	 */
	for (; lb != NULL; lb = next) {
		next = lb->av_forw;
		lb->av_forw = NULL;
		/*
		 * Get the volgrp ptr from device switch table and the 
		 * lvol ptr from the volgrp structure
		 */
		vg = DEV2VG(lb->b_dev);
		lv = VG_DEV2LV(vg, lb->b_dev);

		/*
		 * If the request is a read, or there is only 1 copy, or
		 * the LV/request does not want mirror consistency then put it
		 * on the sched_Q.
		 */

		if ((lb->b_flags & B_READ)
			|| (lv->lv_maxmirrors == 0)
			|| (lb->b_options & (LVM_OPT_NOMWC|LVM_RESYNC_OP))) {
				/* flag this buf for mwc_done() */
				lb->b_options |= LVM_OPT_NOMWC;

				LV_QUEUE_APPEND(&sched_Q, lb);
		} else {
			/* <<<BEGIN MWC CRITICAL SECTION>>> */
			LOCK_INTERRUPT(&(vg->vg_ca_intlock));

			if (lv_checkcache(vg, lb, &sched_Q)) {
				/* <<<END MWC CRITICAL SECTION>>> */
				UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));
				/*
				 * This logical track group is not allocated,
				 * so the only thing we can do is fail the
				 * request.
				 */
				lb->b_flags |= B_ERROR;
				lb->b_resid = lb->b_bcount;
				lb->b_error = EIO;
				lv_terminate(lb);
				continue;
			}
			/* <<<END MWC CRITICAL SECTION>>> */
			UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));
		}
	}
	
	/*
	 * If any cache writes need to be started go start them.
	 */
	LOCK_INTERRUPT(&(vg->vg_ca_intlock));

	lv_cache_start(vg, &sched_Q);

	UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

	/*
	 * Any requests that were not blocked at this level are
	 * allowed to proceed downward.
	 */
	if (sched_Q.lv_head)
		lv_schedule(sched_Q.lv_head, FALSE);

	return;
}

/*
 *  NAME:	lv_mwc_done
 *		
 *  FUNCTION:	perform completion processing for mirror consistency
 *		manager layer.  The scheduler layer calls this routine
 *		when it is done processing a logical request.
 */
lv_mwc_done(lb)
register struct buf *lb;
{
	struct buf *next;
	struct volgrp *vg;
	struct lvol *lv;
	struct lv_queue done_Q, sched_Q;
	struct ca_mwc_mp *h, *ha;
	struct ca_mwc_mp *lv_search_cache();
	int rminor, rltg;
	register int mwc_flag = 0;

	LV_QUEUE_INIT(&done_Q);
	LV_QUEUE_INIT(&sched_Q);

	for( ; lb != NULL; lb = next) {
		next = lb->av_forw;
		lb->av_forw = NULL;
		/*
		 * Get the volgrp ptr from device switch table and the 
		 * lvol ptr from the volgrp structure
		 */
		vg = DEV2VG(lb->b_dev);

		if (lb == &(vg->vg_mwc_lbuf)) {
			/*
			 * This is a mirror cache write I/O completion.
			 * Process the cache wait queues.
			 */
			lv_mwc_cache_done(vg, lb, &done_Q, &sched_Q);
			mwc_flag++;
			continue;
		}
		if ((lb->b_options & LVM_OPT_NOMWC) == 0) {
			/*
			 * Mirror consistency used for this request.
			 * Locate cache entry, decrement outstanding I/O count.
			 * This request is now complete.
			 * At this point, we do not care about errors in
			 * the logical request, they will be handled at
			 * a higher layer.
			 */
			/*
			 * Find cache index, get pointer to hash queue
			 * save request minor number, request LTG
			 */
			rltg = BLK2TRK(lb->b_blkno);
			rminor = minor(lb->b_dev);
			ha = vg->ca_hash[MWC_THASH(rltg)];

			/* <<<< BEGIN MWC CRITICAL SECTION >>>> */
			LOCK_INTERRUPT(&(vg->vg_ca_intlock));

			if ((h = lv_search_cache(ha, rminor, rltg)) == NULL)
				panic("lv_mwc_done");

			if (--h->ca_iocnt < 0)
				panic("dup lv_mwc_done");

                       if (lb->b_options & LVM_VGSA_FAILED) {
                                h->ca_state |= CACHE_ENTRY_FROZEN;
                        }

                        if ((h->ca_iocnt == 0)
                                && ((h->ca_state & CACHE_ENTRY_FROZEN) == 0)) {
                                vg->ca_free++;
                                mwc_flag++;
                        }

			/* <<<< END MWC CRITICAL SECTION >>>> */
			UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));
		}
		/* Completion of a logical I/O request. */
		LV_QUEUE_APPEND(&done_Q, lb);
	}
	/*
	 * If there is any chance of doing anything, process the
	 * cache wait queue.
	 */
	if (mwc_flag) {
		/* <<<< BEGIN MWC CRITICAL SECTION >>>> */
		LOCK_INTERRUPT(&(vg->vg_ca_intlock));

		if ((vg->ca_free != 0)
			&& ((vg->ca_flags & CACHE_INFLIGHT) == 0)
			&& ((lb = vg->vg_cache_wait.lv_head) != NULL)) {

			LV_QUEUE_INIT(&vg->vg_cache_wait);

			for (; lb != NULL; lb = next) {
				next = lb->av_forw;
				lb->av_forw = NULL;

				if (vg != DEV2VG(lb->b_dev)) {
					panic("vg->vg_cache_wait");
				}

				if (lv_checkcache(vg, lb, &sched_Q)) {
					/*
					 * This logical track group is not
					 * allocated, so the only thing we
					 * can do is fail the request.
					 */
					lb->b_flags |= B_ERROR;
					lb->b_resid = lb->b_bcount;
					lb->b_error = EIO;
					LV_QUEUE_APPEND(&done_Q, lb);
				}
			}
		}
		/* <<<< END MWC CRITICAL SECTION >>>> */
		UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));
	}

	/*
	 * Pass any completed logical requests back up to the 
	 * logical (strategy) layer.
	 */
	if (done_Q.lv_head)
		lv_terminate(done_Q.lv_head);

	/*
	 * If any cache writes need to be started go start them.
	 */
	LOCK_INTERRUPT(&(vg->vg_ca_intlock));

	lv_cache_start(vg, &sched_Q);

	UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

	/*
	 * This completion may allow other requests to proceed down, if
	 * they were waiting for a cache write completion.
	 */
	if (sched_Q.lv_head) {
		lv_schedule(sched_Q.lv_head, TRUE);
	}
	return;
}

/*
 *  NAME:	lv_mwc_cache_done
 *
 *  FUNCTION:	Performs the necessary processing caused by the completion
 *		of a MWC record write. The cache is no longer INFLIGHT.
 *		If the write was successful,
 *		   All requests that were waiting for this pvol write to
 *		   complete are passed to the scheduler layer.
 *		Otherwise,
 *		   These requests are scheduled to wait for a write to
 *		   another pvol, and placed on the appropriate pvol
 *		   cache wait queue. If there is nowhere for the cache
 *		   to be written, then the request fails without performing
 *		   any I/O, and is placed on the done_Q.
 *		Since the cache is no longer INFLIGHT, the cache_wait 
 *		queue can be processed.
 */
int
lv_mwc_cache_done(vg, bp, done_Q, sched_Q)
register struct volgrp *vg;
register struct buf *bp;
register struct lv_queue *done_Q, *sched_Q;
{
	register struct buf *lb, *next;
	struct ca_mwc_mp *h, **hap;
	struct pvol *pv;
	int errorflag;
	int rminor, rltg;

	errorflag = bp->b_flags & B_ERROR;

	/* <<<< BEGIN MWC CRITICAL SECTION >>>> */
	LOCK_INTERRUPT(&(vg->vg_ca_intlock));

	vg->ca_flags &= ~CACHE_INFLIGHT;
	if (vg->ca_flags & CACHE_CHANGED) {
		panic("lv_mwc_cache_done changed");
	}
	if (vg->ca_flags & CACHE_CLEAN) {
		lv_cache_scrub(vg, vg->ca_clean_minor);
		vg->ca_flags &= ~CACHE_CLEAN;
		thread_wakeup((vm_offset_t)&vg->ca_clean_minor);
	}
	pv = (struct pvol *)bp->b_driver_un_2.pointvalue;
	if (errorflag) {
		pv->pv_mwc_flags ^= PV_CACHE_TOGGLE;
	} else {
		pv->pv_mwc_latest = vg->vg_mwc_rec->b_tmstamp;
	}

	lb = pv->pv_cache_wait.lv_head;
	/* reinitialize the pvol cache wait queue */
	LV_QUEUE_INIT(&pv->pv_cache_wait);

	/* <<<< END MWC CRITICAL SECTION >>>> */
	UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

	for (; lb != NULL; lb = next) {
		next = lb->av_forw;
		lb->av_forw = NULL;

		rltg = BLK2TRK(lb->b_blkno);
		rminor = minor(lb->b_dev);
		hap = &(vg->ca_hash[MWC_THASH(rltg)]);

		/* <<<< BEGIN MWC CRITICAL SECTION >>>> */
		LOCK_INTERRUPT(&(vg->vg_ca_intlock));

		if ((h = lv_search_cache(*hap, rminor, rltg)) == NULL)
			panic("lv_mwc_cache_done");

		if (!errorflag) {
			/*
			 * Successfule cache write causes this I/O to
			 * be scheduled.
			 */
			h->ca_state &= ~CACHE_ENTRY_CHANGING;

			/* <<<< END MWC CRITICAL SECTION >>>> */
			UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

			LV_QUEUE_APPEND(sched_Q, lb);
			continue;
		}
		if (lv_cache_queue(vg, h, lb)) {
			/* This request loses, as there is nowhere to
			 * write it's cache entry. This can only happen
			 * if all extent are on pvols that have gone
			 * off-line, or we have completely deallocated
			 * the extent.
			 */
			if (--h->ca_iocnt < 0) {
				panic("dup lv_mwc_cache_done");
			}
			if (h->ca_iocnt == 0) {
				vg->ca_free++;
				/*
				 * This cache entry is not valid, so we
				 * need to remove it from the hash chain,
				 * and clean it out.
				 */
				lv_cache_remove(hap, h);
				lv_cache_use(vg, h, !CACHE_MRU);
			}

			/* <<<< END MWC CRITICAL SECTION >>>> */
			UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

			lb->b_flags |= B_ERROR;
			lb->b_resid = lb->b_bcount;
			lb->b_error = EIO;
			LV_QUEUE_APPEND(done_Q, lb);

			continue;
		}
		/* <<<< END MWC CRITICAL SECTION >>>> */
		UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

		/* successful lv_cache_queue places the request on
		 * a pvol cache wait queue */
	}
	return;
}

/*
 *  NAME:       lv_checkcache
 *
 *  FUNCTION:   Scan write consistency cache for request
 *
 *  NOTES:	The following actions are taken under the described 
 *		conditions:
 *
 *		1. LV/LTG pair is in cache and marked on the disk,
 *		   (not CACHE_ENTRY_CHANGING)
 *
 *		   Increment IO count, move to head of hash queue,
 *		   Put request onto schedule queue.
 *
 *		2. LV/LTG pair is in cache but it is NOT marked. (cache
 *		   write is still in progress: CACHE_ENTRY_CHANGING).
 *
 *			   Increment IO count, move to head of hash queue,
 *			   put request pointer on PV cache wait queue,
 *
 *		3. LV/LTG pair is not in cache.
 *
 *			if the cache is not in flight, and space
 *				is available,
 *			   find an available cache entry, fill it with request
 *			   information, move to head of hash queue,
 *			   put request pointer on PV cache wait queue,
 *			   set PV cache write flag
 *			otherwise
 *		   	   put request on volume group cache wait queue
 */
int
lv_checkcache(vg, lb, sched_Q)
register struct volgrp	*vg;		/* ptr to volgrp structure	*/
register struct buf	*lb;		/* current logical buf struct	*/
register struct lv_queue *sched_Q;
{
	struct ca_mwc_mp *h, **hap;
	struct ca_mwc_mp *lv_cache_new(), *lv_search_cache();
	ushort_t rminor;
	register int rltg;

	/*
	 * Find cache index, get pointer to hash queue
	 * save request minor number, request LTG
	 */
	rltg = BLK2TRK(lb->b_blkno);
	rminor = minor(lb->b_dev);
	hap = &(vg->ca_hash[MWC_THASH(rltg)]);

        if ((h = lv_search_cache(*hap, rminor, rltg))
                && (h->ca_iocnt || (h->ca_state & CACHE_ENTRY_FROZEN) == 0)) {
		/*
		 * We have a cache hit.  Bump iocnt,
		 * make entry most recently used.
		 */
		lv_cache_use(vg, h, CACHE_MRU);
		if (h->ca_iocnt++ == 0) {
			vg->ca_free--;
		}
		if (h->ca_state == CACHE_ENTRY_CHANGING) {
			/*
			 * This cache entry is in the process of
			 * being written to disk. We have to wait for
			 * the approriate disk write to complete.
			 */
			LV_QUEUE_APPEND(&h->ca_pvol->pv_cache_wait, lb);
		} else {
			/*
			 * This cache entry is already marked as 
			 * dirty on the disk, so we can proceed.
			 */
			LV_QUEUE_APPEND(sched_Q, lb);
		}
	} else {
		/*
		 * Made it all the way through our hash class without a hit.
		 * Now we have to acquire a cache entry and write it to disk.
		 */
		if (((vg->ca_flags & CACHE_INFLIGHT) == 0)
			&& (h = lv_cache_new(vg))) {
			/*
			 * found a free cache entry, find a place to
			 * write it, and hang it off the right queues.
			 */
			if (lv_cache_queue(vg, h, lb)) {
				/*
				 * Nowhere appropriate to write this mirror
				 * cache entry. This can only happen when a
				 * logical extent has no physical extents.
				 */
				return(1);
			} else {
				vg->ca_free--;
				vg->ca_flags |= CACHE_CHANGED;
				vg->ca_chgcount++;

#if MWC_DEBUG
				/* Diddle the head tstamp so if somebody
				 * reads this it'll show as garbage. */
				lv_microtime(&(vg->vg_mwc_rec->b_tmstamp));
#endif
				/*
				 * Fill it in, insert it in the front 
				 * of the appropriate hash chain, make it
				 * most recently used.
				 */
				h->ca_iocnt = 1;
				h->ca_state = CACHE_ENTRY_CHANGING;
				h->ca_mwc_ent->lv_ltg = rltg;
				h->ca_mwc_ent->lv_minor = rminor;

				h->ca_hq_next = *hap;
				*hap = h;

				lv_cache_use(vg, h, CACHE_MRU);
			}
		} else {
			/*
			 * Cache is INFLIGHT or full, this request must wait
			 * for something to happen.
			 */
			LV_QUEUE_APPEND(&vg->vg_cache_wait,lb);
		}
	}
	return(0);
}

/*
 *  NAME:       lv_search_cache
 *
 *  FUNCTION:   Search the mirror consistency cache for an entry
 *
 *  PARAMETERS: vg - volume group that this request belongs to
 *		lb - the request to search for.
 *
 *  RETURN VALUE:  pointer to a (struct ca_mwc_mp) found in the mirror
 *		consistency cache.  NULL if entry not found.
 *
 *  NOTES: 	This routine must be called within the MWC critical
 *		section, since it searches the hash chains, and does
 *		not perform any locking or reference counts itself.
 *
 */
struct ca_mwc_mp *
lv_search_cache(h, rminor, rltg)
register struct ca_mwc_mp *h;	/* hash anchor for appropriate chain */
register ushort_t rminor;	/* request minor number		*/
register uint_t rltg;		/* request logical track group	*/
{
	int length = 0;
	/*
	 * Scan list for a match.
	 */
	while (h) {
		if (++length > LVM_NUMMWC) {
			panic("lv_search_cache loop");
		}
		if ((h->ca_mwc_ent->lv_ltg == rltg)
			&& (h->ca_mwc_ent->lv_minor == rminor))
				break;
		h = h->ca_hq_next;
	}
#if MWC_DEBUG
	if (h) {
		printf("lv_search_cache: found entry (%d, %d), 0x%x\n",
			rminor, rltg, h);
	} else {
		printf("lv_seach_cache: cache miss (%d, %d)\n", rminor, rltg);
	}
#endif
	return(h);
}

/*
 *  NAME:       lv_cache_queue
 *
 *  FUNCTION:   Do all the things necessary to occupy a cache entry,
 *		including scribbling into it, and hanging the request
 *		onto the appropriate pvol wait queue. Must be called
 *		within the MWC critical section. The cache must not
 *		already be INFLIGHT, and the designated entry marked
 *		as CHANGING. On completion, the request is either queued
 *		on a pvol wait queue (SUCCESS), or should be discarded
 *		(FAILURE).
 */
int
lv_cache_queue(vg, h, lb)
register struct volgrp *vg;
register struct ca_mwc_mp *h;
register struct buf *lb;
{
	register int lxno;
	register struct lvol *lv;
	register struct pvol *pv;
	register struct extent *ext;
	int i, maxmirs;

	/*
	 * Need to locate an appropriate place to write the cache.
	 * This I/O request (lb), and any future I/O requests for this
	 * Logical Track Group, will wait for this write request to
	 * complete prior to falling through to the scheduler layer.
	 */
	if ((lv = VG_DEV2LV(vg, lb->b_dev)) == NULL) {
		panic("lv_cache_queue: NULL lvol");
	}
	lxno = BLK2EXT(vg, lb->b_blkno);
	if ((maxmirs = lv->lv_maxmirrors) == 0) {
		panic("lv_cache_queue: maxmirrors");
	}
	/*
	 * Try to piggy-back it off of a write that's already scheduled
	 * to go. This way, we don't initiate multiple cache writes when
	 * they can all go to the same pvol. Note that the algorithm is
	 * far from perfect: if a later request can't be written to a
	 * pvol that already has I/O queued, but an earlier request could
	 * be moved to collapse multiples into a single write, we won't
	 * do it, it will still be done as multiple writes.
	 */
	pv = NULL;
	for (i = maxmirs; i >= 0; i--) {
		ext = EXTENT(lv,lxno,i);

		if (ext->e_pvnum == PX_NOPV)
			continue;
		pv = vg->pvols[ext->e_pvnum];
		if (pv->pv_flags & LVM_PVMISSING)
			continue;
		if (ext->e_state & PX_NOMWC)
			continue;
		
		if (pv->pv_cache_wait.lv_head)
			/* we can piggy-back off of this one */
			break;
	}

	if (pv && ((pv->pv_flags & LVM_PVMISSING) == 0)) {
		/*
		 * Found a place to put it - either the lowest-numbered
		 * mirror, or one that already has I/O queued. If the
		 * lowest is being deallocated, (PX_NOMWC) we'll
		 * write to it anyway, but that's the breaks.
		 */
		h->ca_pvol = pv;
		if ((pv->pv_mwc_flags & PV_CACHE_QUEUED) == 0) {
			/* First MWC I/O queued to this pvol */
			LV_PVQUEUE_APPEND(&vg->vg_cache_write, pv);
			pv->pv_mwc_flags |= PV_CACHE_QUEUED;
		}
		LV_QUEUE_APPEND(&pv->pv_cache_wait, lb);
		return(0);
	} else {
		return(1);
	}
}

/*
 *  NAME:       lv_cache_use
 *
 *  FUNCTION:   Put cache entry at head or tail of in-use list
 *
 *  NOTES:	This is a simple most recently used(MRU) and least
 *		recently used(LRU) mechanism.
 *
 *  PARAMETERS:
 *
 *  RETURN VALUE: none
 *
 */
int
lv_cache_use(vg, ca_ent, flag)
register struct volgrp	  *vg;		/* ptr to volgrp structure	*/
register struct ca_mwc_mp *ca_ent;	/* cache entry pointer		*/
register int	flag;			/* head/tail flag		*/
{
	register struct ca_mwc_mp *ca_anchor;

	ca_anchor = vg->ca_lst;

	if (flag == CACHE_MRU) {
		if (ca_ent == ca_anchor) {
			/*
			 * If entry is to be moved to the head and it
			 * is already at head just return
			 */
			return;
		} else if (ca_ent == ca_anchor->ca_prev) {
			/*
			 * If entry is to be moved to the head and it
			 * is currently at the tail then just move
			 * the anchor to point to the previous entry.
			 */
			vg->ca_lst = ca_anchor->ca_prev;
			return;
		}
	} else { /* flag != CACHE_MRU */
		if (ca_ent == ca_anchor->ca_prev) {
			/*
			 * If entry is to be moved to the tail and it
			 * is already at the tail just return
			 */
			return;
		} else if (ca_ent == ca_anchor) {
			/*
			 * If entry is to be moved to the tail and it
			 * is currently at the head then just move the
			 * anchor to point to the next entry.
			 */
			vg->ca_lst = ca_anchor->ca_next;
			return;
		}
	}

	/*
	 * Otherwise, we must extract from current position, and move
	 * to the desired position.
	 *
	 * Remove entry from current position
	 */
	ca_ent->ca_prev->ca_next = ca_ent->ca_next;
	ca_ent->ca_next->ca_prev = ca_ent->ca_prev;

	/*
	 * Add it back to the list
	 */
	ca_ent->ca_next = ca_anchor;
	ca_ent->ca_prev = ca_anchor->ca_prev;
	ca_anchor->ca_prev->ca_next = ca_ent;
	ca_anchor->ca_prev = ca_ent;

	/*
	 * If moving to head change anchor to point to it
	 */
	if (flag == CACHE_MRU)
		vg->ca_lst = ca_ent;
	return;
}

/*
 *  NAME:       lv_cache_new
 *
 *  FUNCTION:   Find an available entry in cache for a new request
 *
 *  NOTES:	Follow the LRU chain back looking for an entry that
 *		is available to use.  Available is defined as:
 *
 *		1. Entry has a zero iocnt
 *
 *		If the cache is full, no available entries, return NULL.
 *
 *		This function may only be called with the MWC critical section,
 *		and when the cache is not INFLIGHT.
 *
 *  RETURN VALUE: Address of entry if found or NULL if cache full
 *
 */
struct ca_mwc_mp *
lv_cache_new(vg)
register struct volgrp	*vg;		/* ptr to volgrp structure	*/
{
	register struct ca_mwc_mp *ca_ent;
	register struct ca_mwc_mp *lststrt;
	register struct ca_mwc_mp **hp, **hap;
	int rltg;
#if MWC_DEBUG
	register int size = 0;
#endif

	/*
	 * Set ca_ent to the LRU end of the MRU/LRU chain.
	 */
	ca_ent = vg->ca_lst->ca_prev;
	lststrt = ca_ent;

	if (vg->ca_free == 0) return (NULL);

	do {
#if MWC_DEBUG
		size++;
#endif
		/*
		 * If entry has no IO outstanding then use it.
		 */
		if (ca_ent->ca_iocnt == 0) {
			if (ca_ent->ca_mwc_ent->lv_minor != 0) {
				rltg = ca_ent->ca_mwc_ent->lv_ltg;
				hap = &(vg->ca_hash[MWC_THASH(rltg)]);

				/* Remove this entry from the hash chain */
				for (hp = hap; *hp != ca_ent;
					hp = &((*hp)->ca_hq_next)) {
					if (*hp == NULL)
					    panic("lv_cache_new not hashed");
				}
				*hp = ca_ent->ca_hq_next;
			}
			return(ca_ent);
		}
		ca_ent = ca_ent->ca_prev;
		/*
		 * If the current entry is the one we started with we
		 * have gone through the entire list.
		 */
	} while (ca_ent != lststrt);
#if MWC_DEBUG
	if (size != vg->ca_size) {
		panic("lv_cache_new cache changed size");
	}
#endif
	panic("lv_cache_new not free");
}

/*
 *  NAME:       lv_cache_init
 *
 *  FUNCTION:   Initialize the data structures associated with
 *		mirror consistency manager. Must be called prior
 *		to performing logical I/O that requires mirror
 *		consistency.
 *
 *  RETURNS:	0 if successful, error number on failure.
 *		ENOMEM: could not allocate memory.
 */
int
lv_cache_init(vg)
register struct volgrp *vg;
{
	register struct ca_mwc_mp *ca_ent;
	register struct ca_mwc_dp *ca_dp;
	register int i, size;

	/* Allocate the struct buf, and the data area for the
	 * on-disk mirror consistency record. At some point,
	 * we must read in and initialize the in-memory MWC, etc.
	 */
	if ((vg->vg_mwc_rec = NEW(struct mwc_rec)) == NULL) {
		return(ENOMEM);
	}
	bzero(vg->vg_mwc_rec, sizeof(struct mwc_rec));
	ca_dp = &(vg->vg_mwc_rec->ca_p1[0]);
	if (vg->ca_size <= 0) {
		vg->ca_size = LVM_NUMMWC;
	}
	if (vg->ca_size > MWC_SIZE) {
		vg->ca_size = MWC_SIZE;
	}
	size = vg->ca_size;
	if ((ca_ent = (struct ca_mwc_mp *)
		kalloc(sizeof(struct ca_mwc_mp)*size)) == NULL) {
		KFREE(vg->vg_mwc_rec, sizeof(struct mwc_rec));
		return(ENOMEM);
	}
	vg->ca_part2 = ca_ent;
	/*
	 * Carve up the MWC memory-part data into individual pieces,
	 * and hang off the MRU list, all entries marked as free.
	 */
	vg->ca_lst = ca_ent;

	/* Initialize first entry on the list */
	ca_ent->ca_next = ca_ent+1;
	ca_ent->ca_prev = NULL;
	ca_ent->ca_iocnt = 0;
	ca_ent->ca_hq_next = NULL;
	ca_ent->ca_state = 0;
	ca_ent->ca_mwc_ent = ca_dp++;

	for (i = 1; i < size; i++) {
		ca_ent++;
		ca_ent->ca_next = ca_ent+1;
		ca_ent->ca_prev = ca_ent-1;
		ca_ent->ca_iocnt = 0;
		ca_ent->ca_hq_next = NULL;
		ca_ent->ca_state = 0;
		ca_ent->ca_mwc_ent = ca_dp++;
	}
	/* Fix 'next' pointer in last entry */
	ca_ent->ca_next = vg->ca_lst;

	/* Fix 'prev' pointer in first entry */
	vg->ca_lst->ca_prev = ca_ent;

	vg->ca_flags = 0;
	vg->ca_free = vg->ca_size;
	vg->ca_chgcount = 0;
#if MWC_DEBUG
	/*
	 * Here we assert some things about the contents of
	 * the MWC... same number of entries on both the forward and
	 * reverse links, dp pointer always points within dp memory
	 * image, iocnt's all zero, etc.
	 */
	size = 0;
	for (ca_ent = vg->ca_lst;
		ca_ent->ca_next != vg->ca_lst;
		ca_ent = ca_ent->ca_next) {
		size++;
		if (size > vg->ca_size) {
			dprintf("lv_cache_init: forward count %d.\n", size);
			panic("lv_cache_init");
			break;
		}
		if ((ca_ent->ca_prev == NULL)
			|| (ca_ent->ca_next == NULL)
			|| (ca_ent->ca_mwc_ent == NULL)
			|| (ca_ent->ca_iocnt != 0)
			|| (ca_ent->ca_state != 0)
			|| (ca_ent->ca_mwc_ent->lv_minor != 0)
			|| (ca_ent->ca_mwc_ent->lv_ltg != 0)) {
			dprintf("lv_cache_init: bad data 0x%x\n", ca_ent);
			panic("lv_cache_init");
		}
		
	}
	if (ca_ent->ca_next != vg->ca_lst) {
		dprintf("lv_cache_init: forward size %d.\n", size);
		panic("lv_cache_init");
	}
	size = 0;
	for (ca_ent = vg->ca_lst;
		ca_ent->ca_prev != vg->ca_lst;
		ca_ent = ca_ent->ca_prev) {
		size++;
		if (size > vg->ca_size) {
			dprintf("lv_cache_init: reverse count %d.\n", size);
			panic("lv_cache_init");
			break;
		}
	}
	if (ca_ent->ca_prev != vg->ca_lst) {
		dprintf("lv_cache_init: reverse size %d.\n", size);
		panic("lv_cache_init");
	}
#endif
	LOCK_INTERRUPT_INIT(&vg->vg_ca_intlock);
	return(ESUCCESS);
}

/*
 *  NAME:	lv_cache_activate
 *
 *  FUNCTION:	Activate the mirror consistency cache, recovering
 *		all logical track groups that are marked as in transition.
 *
 *  RETURN VALUE: 0 for success, non-zero error code on failure.
 */
int
lv_cache_activate(vg, flag)
register struct volgrp *vg;
int flag;
{
	register struct pvol *pv;
	register int pv_num;
	int error;
	struct timeval latest;
	int latest_pvnum;
	int missing = 0;

	if (error = lv_cache_init(vg))
		return(error);

	/*
	 * Read all MWC's from all pvol's that are not either
	 * marked as missing in the VGSA or missing by virtue
	 * of not being attached. Find the newest, call it real,
	 * with an I/O count of 1 for each entry. If we encounter
	 * a read error in a MWC, we have to treat that pvol
	 * as we would a "newly missing" pvol, and pretend that it
	 * was, in fact, the newest MWC.
	 */
	timerclear(&latest);
	latest_pvnum = vg->size_pvols;
	for (pv_num = 0; pv_num < vg->size_pvols; pv_num++) {
		if (((pv = vg->pvols[pv_num]) != NULL)
			&& ((pv->pv_flags & LVM_PVMISSING) == 0)) {
			if ((pv->pv_mwc_rec = NEW(struct mwc_rec)) == NULL) {
				error = ENOMEM;
				break;
			}
			if ((pv->pv_flags & LVM_NOTATTACHED) && flag) {
				error = ENOTDIR;
				break;
			}
			if (lv_readmwcrecs(vg, pv) != ESUCCESS) {
				pv->pv_flags |= LVM_MWCMISSING;
				missing++;
			} else {
				if (timercmp(&pv->pv_mwc_latest,&latest,>)) {
					latest = pv->pv_mwc_latest;
					latest_pvnum = pv_num;
				}
			}
		}
	}
	if (error) {
		for (pv_num = 0; pv_num < vg->size_pvols; pv_num++) {
			if ((pv = vg->pvols[pv_num]) != NULL)
				KFREE(pv->pv_mwc_rec, sizeof(struct mwc_rec));
		}
		return(error);
	}
#if MWC_DEBUG
	if (missing) {
		printf("lv_cache_activate: ");
		printf("%d missing mirror consistency records.\n", missing);
	}
#endif
	/* The end result of the above loops is:
	 * - pv_mwc_flags is set for all non-missing pvols
	 * - pv_mwc_latest is set to the newest found timestamp 
	 *   on each pvol (0 for both MWC's invalid)
	 * - pv_flags(LVM_MWCMISSING) is set for all MWC that
	 *   had read errors or were on pvols LVM_NOTATTACHED & not
	 *   LVM_PVMISSING.
	 */
	for (pv_num = 0; pv_num < vg->size_pvols; pv_num++) {
		if ((pv = vg->pvols[pv_num]) != NULL) {
			if (pv_num == latest_pvnum) {
				bcopy(pv->pv_mwc_rec,vg->vg_mwc_rec,
					sizeof(struct mwc_rec));
			}
			KFREE(pv->pv_mwc_rec, sizeof(struct mwc_rec));
		}
	}
	if (latest_pvnum == vg->size_pvols) {
		/* We found no MWC records on any pvols */
		bzero(vg->vg_mwc_rec, sizeof(struct mwc_rec));
	}
	/*
	 * For all logical partitions that have physical copies on the
	 * physical volumes that have LVM_MWCMISSING state, set the STALE
	 * bit in the VGSA. Simply scan through and pick a partition to
	 * call good, set the other (one or two) as STALE, and write the
	 * VGSA. Show a preference (but not overriding) for the good
	 * copy to be accessible. The write of this VGSA will trigger
	 * marking some (if not all) if the LVM_PVMISSING bits.
	 */
	missing = 0;
	for (pv_num = 0; pv_num < vg->size_pvols; pv_num++) {
		if ((pv = vg->pvols[pv_num]) != NULL) {
			if ((pv->pv_flags & LVM_MWCMISSING) != 0) {
				pv->pv_flags &= ~LVM_MWCMISSING;
				lv_cache_missing(vg, pv);
				missing++;
			}
		}
	}
	if (missing)
		lv_sa_config(vg, CNFG_SYNCWRITE, NULL);

	/*
	 * To recover the entries in the MWC record, we must read
	 * one of the copies of the logical track group, then write
	 * the others. On completion of the write, either the track
	 * group will be in sync, or the extent will be marked as
	 * STALE. We can then clean this entry out of the MWC. The
	 * MWC will get written to some pvol(s) after (all|each)
	 * of the recover ops is complete. In order to perform the
	 * recovery op, we need a piece of memory one LTG in size
	 * (default 128K).
	 */
	if ((error = lv_cache_recover(vg)) < 0) {
		struct lv_queue sched_Q;

		LV_QUEUE_INIT(&sched_Q);
		lv_cache_start(vg, &sched_Q);
		if(sched_Q.lv_head)
			lv_schedule(sched_Q.lv_head, FALSE);
		error = ESUCCESS;
	}
	return (error);
}

int
lv_cache_recover(vg)
struct volgrp *vg;
{
	struct ca_mwc_dp *ca_dp;
	struct lvol *lv;
	struct lvol *held_lv = NULL;	/* logical volume held open */
	struct buf *lb = NULL;
	int lv_minor, i;
	caddr_t bufaddr = NULL;
	int ltgshift, chgcount = 0;
	dev_t dev = makedev(vg->major_num, 0);
	int error;

	ca_dp = &(vg->vg_mwc_rec->ca_p1[0]);
	ltgshift = ca_dp->ltgshift;
	vg->vg_mwc_rec->ca_p1[0].ltgshift = LTGSHIFT;
	for (i = 0; i < LVM_NUMMWC; i++) {
		if ((lv_minor = ca_dp->lv_minor) != 0) {
			if ((lv = VG_DEV2LV(vg,lv_minor)) != NULL) {
				if (bufaddr == NULL) {
					if ((bufaddr=(caddr_t)kalloc(BYTEPTRK))
						== NULL) {
						return(ENOMEM);
					}
				}
				if (lv != held_lv) {
					if (held_lv != NULL) {
						ASSERT(lb);
						LASSERT(BUF_LOCK_HOLDER(lb));
						lb->b_un.b_addr = NULL;
						BUF_UNLOCK(lb);
						lb = NULL;
						lv_lvrelease(vg, dev, held_lv);
						held_lv = NULL;
					}
					error = lv_lvhold(vg, dev, lv_minor,
						&held_lv);
					if (error != ESUCCESS) {
						KFREE(bufaddr, BYTEPTRK);
						return(error);
					}
					lb = &lv->lv_rawbuf;
					BUF_LOCK(lb);
					lb->b_un.b_addr = bufaddr;
				}
				lv_recover_ltg(vg, lb, ca_dp, ltgshift);
			}
			chgcount++;
			ca_dp->lv_minor = 0;
			ca_dp->lv_ltg = 0;
		}
		ca_dp++;
	}
	KFREE(bufaddr, BYTEPTRK);
	if (held_lv != NULL) {
		lb->b_un.b_addr = NULL;
		BUF_UNLOCK(lb);
		lb = NULL;
		lv_lvrelease(vg, dev, held_lv);
		held_lv = NULL;
	}
	if (chgcount != 0) {
		lv_cache_update(vg);
		return(-1);
	}
	return(0);
}

lv_recover_ltg(vg, lb, ca_dp, ltgshift)
struct volgrp *vg;
struct buf *lb;
struct ca_mwc_dp *ca_dp;
int ltgshift;
{
	int blkno, lastblk;
	int requested, done, error;

	blkno = (ca_dp->lv_ltg << ltgshift);
	lastblk = (ca_dp->lv_ltg+1) << ltgshift;

	lb->b_proc = NULL;
	lb->b_vp = NULL;
	lb->b_dev = makedev(vg->major_num, ca_dp->lv_minor);
	lb->av_forw = NULL;

	for ( ; blkno < lastblk; blkno += BYTEPTRK) {
		event_clear(&lb->b_iocomplete);
		lb->b_flags = B_PRIVATE|B_READ;
		lb->b_blkno = blkno;
		lb->b_bcount = BYTEPTRK;
		lb->b_options = LVM_RESYNC_OP|LVM_RECOVERY;
		requested = lb->b_bcount;

		/* Perform the recovery operation */
		lv_strategy(lb);

		error = biowait(lb);
		done = lb->b_bcount - lb->b_resid;
		if (!error && (requested != done))
			error = EIO;

	}
	return;
}

lv_cache_missing(vg, pv)
register struct volgrp *vg;
register struct pvol *pv;
{
	struct lvol *lv;
	struct PV_header *PV;
	struct PX_entry *PX;
	int lxnum, pvnum, pxnum, pxcount;

	pvnum = pv->pv_num;
	PV = PV_head(vg, pvnum);
	pxcount = PV->px_count;
	/*  This loop could conceivably take a long time */
	for (pxnum = 0; pxnum < pxcount; pxnum++) {
		PX = PX_ent(vg,pvnum,pxnum);
		if (PX->lv_index != 0) {
			lxnum = PX->lx_num;
			lv = VG_DEV2LV(vg,PX->lv_index);
			lv_cache_stale(vg, lv, lxnum, pxnum, pvnum);
		}
	}
	return;
}

lv_cache_stale(vg, lv, lxnum, pxnum, pvnum)
struct	volgrp *vg;
register struct lvol *lv;
int lxnum, pxnum, pvnum;
{
	struct extent *ext;
	int pxcount, m;
	int ngood;

	ngood = 0;

	/* Set the specified mirror copy as stale. */
	for (m = lv->lv_maxmirrors; m >= 0; m--) {
		ext = EXTENT(lv,lxnum,m);
		if ((ext->e_state & PX_STALE) == 0) {
			if ((ext->e_pxnum == pxnum)
				&& (ext->e_pvnum == pvnum)) {
				ext->e_state |= PX_STALE;
				lv_sa_setstale(vg, pvnum, pxnum);
			} else {
				ngood++;
			}
		}
	}

	/* Set all other mirrors, except one, as stale. */
	for (m = lv->lv_maxmirrors; m >= 0; m--) {
		ext = EXTENT(lv,lxnum,m);
		if ((ngood > 1)
			&& (ext->e_pvnum != PX_NOPV)
			&& ((ext->e_state & PX_STALE) == 0)) {
			ngood--;
			ext->e_state |= PX_STALE;
			lv_sa_setstale(vg, ext->e_pvnum, ext->e_pxnum);
		}
	}
	if (ngood > 1)
		panic("lv_cache_stale: too good.");

	return;
}

/* 
 *  NAME:	lv_writemwcrecs
 *
 *  FUNCTION:	Perform MWC cache initialization functions necessary
 *		for Physical Volume (PV) installation.
 */
int
lv_writemwcrecs(vg, pv)
register struct volgrp *vg;
register struct pvol *pv;
{
	register int error;
	register struct mwc_rec *mwc_rec;

	mwc_rec = vg->vg_mwc_rec;
	timerclear(&(pv->pv_mwc_latest));
	if ((error = lv_writemwcrec(vg, pv, mwc_rec)) != ESUCCESS) {
		return (error);
	}
	pv->pv_mwc_flags ^= PV_CACHE_TOGGLE;
	if ((error = lv_writemwcrec(vg, pv, mwc_rec)) != ESUCCESS) {
		return (error);
	}
	return (error);
}

/*
 *  NAME:	lv_readmwcrecs
 *
 *  FUNCTION:	Read the Mirror Write Consistency records from
 *		the indicated pvol, and set the following state:
 *		- pv_mwc_flags is set for all non-missing pvols
 *		- pv_mwc_latest is set to the newest found timestamp 
 *		  on each pvol (0 for both MWC's invalid)
 *		- pv_flags(LVM_MWCMISSING) is set for all MWC that
 *		  had read errors or were on pvols not LVM_NOTATTACHED & not
 *		  LVM_PVMISSING.
 */
int
lv_readmwcrecs(vg, pv)
register struct volgrp *vg;
register struct pvol *pv;
{
	int error;
	pv->pv_mwc_flags &= ~(PV_CACHE_QUEUED|PV_CACHE_TOGGLE);

	/* Read the primary MWC record into the pvol MWC record */
	if ((error = lv_readmwcrec(vg, pv, pv->pv_mwc_rec)) != ESUCCESS) {
		/* No need to read the other one */
		timerclear(&pv->pv_mwc_latest);
		return(error);
	}
	if (timercmp(&pv->pv_mwc_rec->b_tmstamp,&pv->pv_mwc_rec->e_tmstamp,!=))
		timerclear(&pv->pv_mwc_latest);
	else
		pv->pv_mwc_latest = pv->pv_mwc_rec->b_tmstamp;

	pv->pv_mwc_flags ^= PV_CACHE_TOGGLE;

	/* Read the secondary MWC record into the volgrp MWC record */
	if ((error = lv_readmwcrec(vg, pv, vg->vg_mwc_rec)) != ESUCCESS) {
		timerclear(&pv->pv_mwc_latest);
		return(error);
	}
	if (!timercmp(&vg->vg_mwc_rec->b_tmstamp,&vg->vg_mwc_rec->e_tmstamp,!=))
		if (timercmp(&vg->vg_mwc_rec->b_tmstamp,&pv->pv_mwc_latest,>)) {
			pv->pv_mwc_latest = vg->vg_mwc_rec->b_tmstamp;
			bcopy(vg->vg_mwc_rec, pv->pv_mwc_rec,
				sizeof(struct mwc_rec));
		}

	return(error);
}

/*
 * lv_readmwcrec()/lv_writemwcrec(): Functions to read and write
 * MWC records during initialization phases.
 */
int
lv_readmwcrec(vg, pv, where)
register struct volgrp *vg;
register struct pvol *pv;
register struct mwc_rec *where;
{
	register int error;
	register struct buf *lb;

	if (pv->pv_flags & LVM_NOTATTACHED) return(EIO);

	lb = &(VG_LVOL0(vg)->lv_rawbuf);
	BUF_LOCK(lb);
	event_clear(&lb->b_iocomplete);
	lb->b_flags = B_READ;
	lb->b_blkno = pv->pv_mwc_loc[(pv->pv_mwc_flags&PV_CACHE_TOGGLE)!=0]
		    + EXT2BLK(vg, pv->pv_num);
	lb->b_bcount = sizeof(struct mwc_rec);
	lb->b_un.b_addr = (caddr_t)where;
	lb->b_vp = NULL;
	lb->b_dev = makedev(vg->major_num,0);
	lb->av_forw = NULL;
	lv_strategy(lb);
	error = biowait(lb);

	BUF_UNLOCK(lb);
	return(error);
}

int
lv_writemwcrec(vg, pv, where)
register struct volgrp *vg;
register struct pvol *pv;
register struct mwc_rec *where;
{
	register int error;
	register struct buf *lb;

	if (pv->pv_flags & LVM_NOTATTACHED) return(EIO);

	lb = &(VG_LVOL0(vg)->lv_rawbuf);
	BUF_LOCK(lb);
	event_clear(&lb->b_iocomplete);
	lb->b_flags = B_WRITE;
	lb->b_blkno = pv->pv_mwc_loc[(pv->pv_mwc_flags&PV_CACHE_TOGGLE)!=0]
		    + EXT2BLK(vg, pv->pv_num);

	if (lb->b_blkno == 0)
		panic("lv_writemwc_rec");

	lb->b_bcount = sizeof(struct mwc_rec);
	lb->b_un.b_addr = (caddr_t)where;
	lb->b_vp = NULL;
	lb->b_dev = makedev(vg->major_num,0);
	lb->av_forw = NULL;
	lv_strategy(lb);
	error = biowait(lb);
	BUF_UNLOCK(lb);
	return(error);
}

int
lv_cache_deactivate(vg)
register struct volgrp *vg;
{
	if (vg->ca_flags & CACHE_INFLIGHT)
		panic("lv_cache_deactivate inflight");

	KFREE(vg->vg_mwc_rec, sizeof(struct mwc_rec));
	KFREE(vg->ca_part2, sizeof(struct ca_mwc_mp)*vg->ca_size);
	vg->ca_lst = NULL;
}

/*
 * NAME:	lv_cache_start
 *
 * FUNCTION:	Initiate I/O on the mirror consistency cache
 *		if the cache is not currently INFLIGHT, and
 *		I/O is waiting for the cache.
 */
int
lv_cache_start(vg, sched_Q)
register struct volgrp *vg;
register struct lv_queue *sched_Q;
{
	register struct pvol *pv;
	register struct buf *lb;

	if (((vg->ca_flags & CACHE_INFLIGHT) == 0)
		&& (LV_PVQUEUE_FETCH(&vg->vg_cache_write, pv), pv)) {
		/* cache is !INFLIGHT and an I/O request is waiting */

		/* Mark the cache as INFLIGHT */
		vg->ca_flags |= CACHE_INFLIGHT;
		if (vg->ca_flags & CACHE_CHANGED) {
			vg->ca_flags &= ~CACHE_CHANGED;
			lv_microtime(&(vg->vg_mwc_rec->b_tmstamp));
			vg->vg_mwc_rec->e_tmstamp = vg->vg_mwc_rec->b_tmstamp;
			/* Compute some statistics, etc. */
			vg->ca_chgcount = 0;
		}
		pv->pv_mwc_flags &= ~PV_CACHE_QUEUED;
		pv->pv_mwc_flags ^= PV_CACHE_TOGGLE;

		lb = &(vg->vg_mwc_lbuf);
		lb->b_flags = B_WRITE;
		lb->b_blkno =
			pv->pv_mwc_loc[(pv->pv_mwc_flags&PV_CACHE_TOGGLE)!=0]
			+ EXT2BLK(vg, pv->pv_num);

		if (lb->b_blkno == 0)
			panic("lv_cache_start");

		lb->b_bcount = sizeof(struct mwc_rec);
		lb->b_un.b_addr = (caddr_t)vg->vg_mwc_rec;
		lb->b_vp = NULL;
		lb->b_dev = makedev(vg->major_num,0);
		lb->b_driver_un_2.pointvalue = (void *)pv;

		LV_QUEUE_APPEND(sched_Q, lb);
	}
	return;
}

/*
 *  NAME:	lv_cache_clean
 *
 *  FUNCTION:	Removes all cache entries for a given logical volume.
 *		This function may only be invoked for a PAUSED logical
 *		volume, since it is incorrect to remove an active entry.
 *		This function's primary purpose is to prevent false cache
 *		hits for extents that have been deallocated. It may also
 * 		be useful during logical volume close.
 */
lv_cache_clean(vg, minor_num)
register struct volgrp *vg;
ushort_t minor_num;
{
	struct lv_queue sched_Q;
#if MWC_DEBUG
	register struct lvol *lv;
	
	if ((lv = VG_DEV2LV(vg, minor_num)) == NULL) 
		panic("lv_cache_clean NULL lv");

	if ((lv->lv_status & (LV_OPEN|LV_PAUSED)) == LV_OPEN) 
		panic("lv_cache_clean lv not paused");
#endif
	LV_QUEUE_INIT(&sched_Q);

	/* <<<< BEGIN MWC CRITICAL SECTION >>>> */
	LOCK_INTERRUPT(&(vg->vg_ca_intlock));

	if (vg->ca_flags & CACHE_INFLIGHT) {
		vg->ca_flags |= CACHE_CLEAN;
		vg->ca_clean_minor = minor_num;
		while (vg->ca_flags & CACHE_CLEAN) {
			assert_wait((vm_offset_t)&vg->ca_clean_minor, FALSE);
			UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

			thread_block();

			LOCK_INTERRUPT(&(vg->vg_ca_intlock));
		}
	} else {
		if (lv_cache_scrub(vg, minor_num))
			lv_cache_start(vg, &sched_Q);
	}
	/* <<<< END MWC CRITICAL SECTION >>>> */
	UNLOCK_INTERRUPT(&(vg->vg_ca_intlock));

	/*
	 * Any requests that were generated at this level are
	 * allowed to proceed downward.
	 */
	if (sched_Q.lv_head)
		lv_schedule(sched_Q.lv_head, FALSE);
	return;
}

int
lv_cache_scrub(vg, minor_num)
register struct volgrp *vg;
ushort_t minor_num;
{
        register struct ca_mwc_mp *ca_ent;
        register struct ca_mwc_mp *next_ent;
        register struct ca_mwc_mp *lststart;
        register struct ca_mwc_mp **hap;
        int rltg;
        int chgcount = 0, size = 0;
        /*
         * Walk through the entire cache, and if the cache entry
         * minor number is the one being cleaned, zero out the entry
         * and make it least recently used.
         */
        lststart = vg->ca_lst;          /* MRU entry */
        next_ent = lststart->ca_prev;   /* LRU entry */

        do {
                size++;
                ca_ent = next_ent;
                next_ent = ca_ent->ca_prev;

                if ((ca_ent->ca_mwc_ent->lv_minor == minor_num)
                        && ((ca_ent->ca_state & CACHE_ENTRY_FROZEN) == 0)) {
                        if (ca_ent->ca_iocnt != 0) {
                                panic("lv_cache_scrub busy");
                        }
                        chgcount++;
                        rltg = ca_ent->ca_mwc_ent->lv_ltg;
                        hap = &(vg->ca_hash[MWC_THASH(rltg)]);

                        lv_cache_remove(hap, ca_ent);
                        lv_cache_use(vg, ca_ent, !CACHE_MRU);
                }
        } while (ca_ent != lststart);

        ASSERT(size == vg->ca_size);

        if (chgcount != 0)
                lv_cache_update(vg);

        return(chgcount);
}

lv_cache_update(vg)
struct volgrp *vg;
{
	struct pvol *pv;
	int pvnum;
	/*
	 * Need to queue this newly-modified cache to be written to
	 * one or more new places.
	 */
	for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
		if (((pv = vg->pvols[pvnum]) == NULL) 
			|| (pv->pv_flags & LVM_PVMISSING)) continue;

		if (timerisset(&pv->pv_mwc_latest)) {
			if ((pv->pv_mwc_flags & PV_CACHE_QUEUED) == 0) {
				/* This pvol has dirty caches on it, but
				 * currently does not have any cache I/O
				 * queued. */
				LV_PVQUEUE_APPEND(&vg->vg_cache_write, pv);
				pv->pv_mwc_flags |= PV_CACHE_QUEUED;
			}
		}
	}
	/*
	 * Diddle the head timestamp so if somebody reads this it'll
	 * show as garbage.
	 */
	lv_microtime(&(vg->vg_mwc_rec->b_tmstamp));
	vg->ca_flags |= CACHE_CHANGED;

	return;
}

lv_cache_remove(hap, h)
register struct ca_mwc_mp **hap, *h;
{
register struct ca_mwc_mp **hp;

	/*
	 * Remove this entry from its hash chain.
	 */
	for (hp = hap; *hp != h;
		hp = &((*hp)->ca_hq_next)) {
		if (*hp == NULL)
			panic("lv_cache_remove not hashed");
	}
	*hp = h->ca_hq_next;

	h->ca_hq_next = NULL;
	h->ca_pvol = NULL;
	h->ca_mwc_ent->lv_minor = 0;
	h->ca_mwc_ent->lv_ltg = 0;
	h->ca_state = 0;

	return;
}
