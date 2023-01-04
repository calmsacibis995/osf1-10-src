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
static char	*sccsid = "@(#)$RCSfile: lv_strategy.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/03 15:19:32 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_strat.c
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *
 * lv_strategy.c -- Logical Volume Manager device driver strategy routines.
 *
 *	These functions process logical block requests for the Logical Volume
 *	Manager pseudo-device driver.  They belong to the bottom half
 *	of this device driver, whose structure is described below.
 *
 *	Each volume group has a separate device switch entry; its logical
 *	volumes are distinguished by their minor numbers.
 *
 * Function:
 *
 *	The routines in this source file deal only with logical requests.
 *	Translation of these requests to physical addresses, scheduling
 *	of mirrored operations, and bad block relocation are all handled
 *	at a lower level in the driver.
 *
 *	Logical operations to overlapping page ranges must complete in
 *	FIFO order.  These routines keep track of all outstanding requests
 *	using the work_Q for this logical volume.  Whenever a new request
 *	arrives, it blocks until all earlier conflicting requests
 *	have completed.  When each request finishes, those that blocked
 *	waiting for it will be scheduled.  Serializing requests at the
 *	logical layer makes things much simpler for the lower-level
 *	physical operations.  Bad block relocation and mirror retries
 *	can be scheduled without fear that an overlapping request has
 *	slipped into the queue out of order and changed the data.
 *
 *	This "exclusive lock" on the block range of each request is
 *	sufficient to ensure FIFO operation, but does not provide
 *	maximum overlap for concurrent reads.  This is not a problem,
 *	because most users of this service (file system, virtual memory
 *	manager, etc.) are combining concurrent reads of the same block
 *	at a higher level.  In practice, the opportunity to schedule
 *	overlapping reads concurrently will be rare, so the simplicity
 *	of exclusive locks for both reads and writes is attractive.
 *
 * Execution environment:
 *
 *	All these routines run on interrupt levels, so they are not
 *	permitted to block.
 *
 * 	NOTE:  lv_strategy can NOT be called at a higher interrupt
 *	 	priority level than splbio().
 */

/*
 *  Modification History:  lv_strategy.c
 *
 *  04-Nov-91     Tom Tierney
 *	Modified LVM strategy return type to be int like all others (one
 *	day all driver entrypoints that return no value will be "void").
 * 
 *  18-Sep-91     Terry Carruthers
 *	Modified lv_terminate to recognize the completion of
 *      a synchronization write during LVM_RECOVER operations.
 *
 *  06-Jun-91     Terry Carruthers
 *	Added code which causes new I/O to be blocked
 *      on the lv specific work queue, if the lv has
 *      a status of "paused".  Previously, the I/O
 *      was block only if the status was "pausing".
 *
 */


/*
 *			S T R U C T U R E   O V E R V I E W
 *
 *
 *	The bottom half of this device driver is structured in three
 *	"layers", corresponding to the following source files:
 *
 *		lv_strategy.c -- logical request validation, initiation
 *				and termination.  Serializes logical
 *				requests when their block ranges overlap.
 *
 *		lv_schedule.c -- scheduling physical requests for logical
 *				operations.  
 *
 *		lv_mircons.c --	Handles mirroring and the
 *				mirror write consistency cache.
 *
 *		lv_phys.c   --	physical request startup and termination.
 *				Handles bad block relocation.
 *
 *	Here is how a simple request flows through these layers:
 *
 *
 *   original	|	    -- LVM device driver --		|    device
 *   requestor	| lv_strategy.c	    lv_schedule.c lv_phys.c	|    driver
 *   -----------+-----------------------------------------------+-----------
 *		|						|
 *	    ------->						|
 *		|  lv_strategy					|
 *		|	    ------->				|
 *		|		    lv_schedule			|
 *		|			    ------->		|
 *		|				    lv_begin	|
 *		|					    ------->
 *		|						|   device
 *		|						|   strategy
 *		|						|   routine
 *		|						|
 *		|						|	*
 *		|						|	*
 *		|						|	*
 *		|						|
 *		|						|	<-----
 *		|						|   device
 *		|						|   interrupt
 *		|						|   handler
 *		|						|
 *		|					    <-------
 *		|				    lv_end	|
 *		|			    <-------		|
 *		|		    lv_finished			|
 *		|	    <-------				|
 *		|   lv_terminate				|
 *	    <-------						|
 *    biodone	|						|
 *		|						|
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/conf.h>

#include <lvm/lvmd.h>		/* LVM device driver internal declares */

/* Internal function declarations */
extern void lv_initiate();
extern void lv_reject();
extern void lv_complete();
extern void lv_collect();

/*
 * ------------------------------------------------------------------------
 *
 *		L O G I C A L   R E Q U E S T   P R O C E S S I N G
 *
 * ------------------------------------------------------------------------
 */

/*
 * NAME:	lv_strategy
 *
 * FUNCTION:	Validate and initiate logical volume read & write requests
 *
 * PARAMETERS:	lb - pointer to a buf struct
 *
 * INPUTS:	The following fields in lb are expected to be initialized:
 *		lb->b_dev: logical volume device number
 *		lb->b_flags: request flags
 *		lb->b_blkno: block number with logical volume
 *		lb->b_bcount: byte count of transfer request
 *		lb->b_options: options passed through from raw I/O.
 *
 * OUTPUTS:	If successful, lb is linked into the per-lvol work_Q.
 *		Once a logical request has been initiated, it will remain
 *		in the the work_Q for its logical volume until removed
 *		by lv_terminate when the entire request is done.
 *
 *		If there are no conflicts with outstanding requests,
 *		it is also placed onto the pending_Q to pass to the
 *		scheduler.
 *
 *		The following fields in lb are updated:
 *		lb->b_work: mask of pages affected by transfer
 *		lb->b_error: set to ELBBLOCKED if conflicts, 0 if none.
 *
 *		If an error is detected, the following fields are
 *		initialized and biodone() is invoked:
 *		lb->b_error: set to indicate the type of error
 *		lb->b_resid: set to number of bytes _not_ transferred
 *		lb->b_flags: B_ERROR set.
 *
 * NOTES:	Invalid requests (error, EOF, etc.), are immediately kicked
 *		back to the requestor (via biodone).
 *
 * RETURN VALUE: none
 *
 */
int
lv_strategy(lb)
register struct buf *lb;		/* input list of logical buf's  */
{
     	register struct buf *next;	/* next buf			*/
	struct buf *buf1, *buf2;
	struct volgrp	*vg;		/* vol group struct		*/
	register struct lvol *lv;	/* logical volume structure	*/

	register int hash;		/* hash class of this request	   */
	register int x_no;		/* logical extent number	   */
	register int nblocks;		/* number of blocks in logical vol */
	register struct extent *ext;	/* physical extent structure	   */
	u_int bct;
	u_int bwp;
	u_int pwt;
	u_int pct;
	dev_t dev;
	int error;
	int i;

#ifndef BUF_CHAINS
	lb->av_forw = NULL;		/* guarantee that pointer is NULL */
	/*
	 * If buffer chains are allowed, the following restrictions
	 * apply:
	 * - all device major numbers [major(lb->b_dev)] must be
	 *   the same for all bufs on a given chain. This is a consequence
	 *   of the devsw/major number definition.
	 * - all device minor numbers [minor(lb->b_dev)] must be
	 *   the same for all bufs on a given chain. This simplifies 
	 *   this code greatly.
	 */
#endif

	dev = lb->b_dev;
	vg = DEV2VG(dev);		/* get volgrp pointer */
	lv = VG_DEV2LV(vg, dev); /* locate the logical volume structure */

	/* should not be here if lvol is closed */
	if (minor(dev) && ((lv->lv_status & LV_OPEN) == 0)) {
		panic("lv_strategy");
		/* NOTREACHED */
	}

	/*
	 *  compute mask of pages affected by this operation:
	 *	bwp:	offset of 1st block within its page
	 *	bct:	count of blocks transferred
	 *	pct:	count of pages affected
	 *	pwt:	offset of 1st page within logical track group
	 */

	bwp = lb->b_blkno & (BPPG-1);
	bct = BYTE2BLK(lb->b_bcount);

	pwt = BLK2PG(lb->b_blkno) & (PGPTRK-1);

        /* 
         *   If the request size is less than 512 bytes (1 disk block) 
         *   and we are reading at offset=0, the argument to BLK2PG will 
         *   be -1. We hence check if (bct != 0). 
         *   Also this function does not accept request sizes which are 
         *   not multiples of 512 bytes. Such requests are rejected below.
         *   This check is also done in lv_minphys() were nothing is done 
         *   about it. 
         */

        if(bct)
		pct = BLK2PG(bwp + bct - 1) + 1;
        else
		pct = BLK2PG(bwp);

	/* check for track group misalignment */
	if (pwt+pct > PGPTRK) {
		/* Allocate 2 new buf's */
		buf1 = NEW(struct buf);
		bzero((caddr_t)buf1, sizeof(struct buf));
		buf2 = NEW(struct buf);
		bzero((caddr_t)buf2, sizeof(struct buf));

		/* Link them together using b_forw */
		buf1->b_forw = buf2;
		buf2->b_forw = buf1;

		/* Link them to the original buffer using b_back */
		buf1->b_back = lb;
		buf2->b_back = lb;

		/* Set b_bcount, b_blkno, b_addr and the r/w flag */
		/* b_dev, and b_iodone to lv_collect              */
		buf1->b_bcount = BLK2BYTE(((PGPTRK - pwt) * BPPG) + bwp);
		buf1->b_blkno = lb->b_blkno;
		buf1->b_un.b_addr = lb->b_un.b_addr;
		buf1->b_flags = (lb->b_flags & B_READ);
		buf1->b_dev = lb->b_dev;
		buf1->b_iodone = lv_collect;

		buf2->b_bcount = lb->b_bcount - buf1->b_bcount;
		buf2->b_blkno = lb->b_blkno + BYTE2BLK(buf1->b_bcount);
		buf2->b_un.b_addr = lb->b_un.b_addr + buf1->b_bcount;
		buf2->b_flags = (lb->b_flags & B_READ);
		buf2->b_dev = lb->b_dev;
		buf2->b_iodone = lv_collect;

		/* Link the bufs, give them to strategy */
		buf1->av_forw = buf2;
		lb = buf1;
	}

	for (; lb != NULL; lb = next) {
		next = lb->av_forw;	/* unlink this request from list */
		lb->av_forw = NULL;

		if ((lb->b_flags & B_PRIVATE) == 0) {
			/* options only initialized when raw interface used */
			lb->b_options = 0;
		}

		/* Request must be in whole disk blocks only */
		if (lb->b_bcount & (DEV_BSIZE-1)) {
			lv_reject(lb, EINVAL);
			continue;
		}
		if (lb->b_options & LVM_RESYNC_OP) {
			if ((lb->b_blkno&(BLKPTRK-1))
				|| ((lb->b_flags & B_READ) == 0)
				|| (lb->b_bcount != BYTEPTRK)) {
				lv_reject(lb, EINVAL);
				continue;
			}
		}

		/* This is inside the loop to fail all requests */
		if (lv == NULL)  {
			lv_reject(lb, ENXIO);
			continue;	/* no such LV */
		}
		if (lb->b_dev != dev)
			panic("lv_strategy devno");

		/*
		 * Search hash class for conflicting requests.
		 * this will fail only if the request crosses
		 * logical track group boundaries.
		 */
		hash = LV_HASH(lb);	/* work in progress hash class */

		LOCK_INTERRUPT(&(lv->lv_intlock));

		/* ULTRIX/OSF:  Need to block the I/O if the
		 * logical volume has a status of "pausing" or
		 * "paused"!
		 */
		error = lv_block(&(lv->work_Q[hash]), &lv->lv_ready_Q, lb,
				(lv->lv_status & (LV_PAUSING|LV_PAUSED)));
		
		if (error) {
			UNLOCK_INTERRUPT(&(lv->lv_intlock));
			lv_reject(lb, error);
			continue;
		}
		lv->lv_requestcount++;
		lv->lv_totalcount++;
		UNLOCK_INTERRUPT(&(lv->lv_intlock));

		LOCK_INTERRUPT(&(vg->vg_intlock));
		vg->vg_requestcount++;
		vg->vg_totalcount++;
		UNLOCK_INTERRUPT(&(vg->vg_intlock));
	}
	if (lv->lv_ready_Q.lv_head)
		lv_initiate(lv);
	return;
}

/*
 * NAME:	lv_initiate
 *
 * FUNCTION:	Initiate all ready logical requests for a logical volume.
 */
void
lv_initiate(lv)
register struct lvol *lv;
{
	register struct buf *lb;
	register struct buf **prev;	/* Previous buf */
	register struct volgrp *vg;
	struct lv_queue pending_Q;
	int lvsize;
	int i;

	/*
	 * The following checks are here rather than
	 * in lv_strategy in order to synchronize with pause/continue:
	 * here, we are guaranteed that the contents of the
	 * struct lvol are stable. In particular, the extent arrays are
	 * guaranteed not to change. Above, the requests
	 * have not yet been synchronized with changes to the
	 * struct lvol.
	 */
	LV_QUEUE_INIT(&pending_Q);

	LOCK_INTERRUPT(&(lv->lv_intlock));

	while ((LV_QUEUE_FETCH(&lv->lv_ready_Q,lb), lb) != NULL) {

		/* It would be good to be able to do this outside the loop */
		vg = DEV2VG(lb->b_dev);
		lvsize = EXT2BLK(vg, lv->lv_maxlxs);

		if (lv->lv_flags & LVM_VERIFY)
			lb->b_flags |= B_WRITEV;

		if (lv->lv_flags & LVM_NORELOC)
			lb->b_options |= LVM_OPT_NORELOC;

		if (lv->lv_flags & LVM_NOMWC)
			lb->b_options |= LVM_OPT_NOMWC;
		
		if ((lv->lv_flags & LVM_RDONLY) && !(lb->b_flags & B_READ))  {
			/* write of R/O LV */
			UNLOCK_INTERRUPT(&(lv->lv_intlock));

			lb->b_flags |= B_ERROR;
			lb->b_resid = lb->b_bcount;
			lb->b_error = EROFS;
			lv_complete(lv, lb);

			LOCK_INTERRUPT(&(lv->lv_intlock));
			continue;
		}

		/*
		 * Since an lvol is a whole number of LTGs (and
		 * extents) and lv_mincnt prevents requests from crossing
		 * LTG boundaries, we only have to check the starting
		 * block number to check for end of media.
		 */
		if ((daddr_t)lb->b_blkno >= lvsize)  {
			/*
			 * If at End of Media(EOM) return no error if read 
			 * return error and ENXIO for writes
			 */
			UNLOCK_INTERRUPT(&(lv->lv_intlock));

			LOCK_INTERRUPT(&(vg->vg_intlock));
			vg->vg_requestcount--;
			UNLOCK_INTERRUPT(&(vg->vg_intlock));

			lb->b_resid = lb->b_bcount;
			if (lb->b_flags & B_READ) {	/* Read past EOM */
				lb->b_error = 0;
			} else {			/* Write past EOM */
				lb->b_flags |= B_ERROR;
				lb->b_error = ENXIO;
			}
			lv_complete(lv, lb);

			LOCK_INTERRUPT(&(lv->lv_intlock));
			continue;
		}

		if (lb->b_options & LVM_RESYNC_OP) {
			int lxno, trkinext;
			struct lextent *lext;
			struct extent *ext;
			struct pvol *pv;

			lxno = BLK2EXT(vg,lb->b_blkno);
			trkinext = TRKINEXT(vg,lb->b_blkno);
			lext = LEXTENT(lv,lxno);
			if (lb->b_flags & B_READ) {
				if (lext->lx_synctrk == -1) {
				    /* No sync in progress, initialize masks */
				    int m, maxm, syncmask = 0;
				    maxm = lv->lv_maxmirrors + 1;
				    for (m = 0; m < maxm; m++) {
					ext = EXTENT(lv,lxno,m);
					if (ext->e_pvnum == PX_NOPV)
						continue;
					pv = vg->pvols[ext->e_pvnum];
					if (pv->pv_flags & LVM_PVMISSING)
						continue;
					if (lb->b_options & LVM_RECOVERY) {
						if (!(ext->e_state & PX_STALE))
							syncmask |= (1<<m);
					} else {
						if (ext->e_state & PX_STALE)
							syncmask |= (1<<m);
					}
				    }
				    lext->lx_syncmsk = syncmask;
				    lext->lx_syncerr = 0;
				} else if (lext->lx_synctrk != (trkinext - 1)) {
					/* Bad (non-sequential) resync. */
					panic("lv_initiate resync read");
				}
				lext->lx_synctrk = trkinext;
				if (lext->lx_syncmsk == 0) {
					if (trkinext == (TRKPEREXT(vg) - 1))
						lext->lx_synctrk = -1;
					/* quick and easy */
					UNLOCK_INTERRUPT(&(lv->lv_intlock));

					LOCK_INTERRUPT(&(vg->vg_intlock));
					vg->vg_requestcount--;
					UNLOCK_INTERRUPT(&(vg->vg_intlock));

					lb->b_resid = 0;
					lv_complete(lv, lb);

					LOCK_INTERRUPT(&(lv->lv_intlock));
					continue;
				}
			} else { 	/* !B_READ */
				if (lext->lx_synctrk != trkinext) {
					/* Bad (non-sequential) resync. */
					panic("lv_initiate resync write");
				}
			}
		}
		LV_QUEUE_APPEND(&pending_Q, lb);
	} /* End While */

	UNLOCK_INTERRUPT(&(lv->lv_intlock));

	if (pending_Q.lv_head)
		lv_mwcm(pending_Q.lv_head);
	return;
}

/*
 * NAME:	lv_reject
 *
 * FUNCTION:	Reject invalid logical requests.
 *
 * INPUT:	lb - pointer to a logical buf struct request
 *		errno - ERRNO value to return to caller
 *
 * OUTPUT:	buf struct returned to original requestor with status set.
 *
 * RETURN VALUE: none
 *
 */
void
lv_reject(lb, error)
struct buf	*lb;		/* offending buf structure	*/
int		error;		/* error number			*/
{
	if (error) lb->b_flags |= B_ERROR;

	lb->b_error = error;
	lb->b_resid = lb->b_bcount;	/* set residual count */

	/* if an iodone routine is given, go to buffer collector */
	/* for spanned logical requests.  This will call biodone */
	/* when all span buffers are accounted for.              */
	if (lb->b_iodone) {
		(*lb->b_iodone)(lb);
		return;
	}

	biodone(lb);			/* notify the perpetrator */

	return;
}

/*
 * NAME:	lv_terminate
 *
 * FUNCTION:	Process completed logical operation
 *
 * NOTE:	The logical request is removed from the work_Q, its requestor
 *		is notified, and any subsequent logical requests that were
 *		blocked waiting on this one are added to the lv_ready_Q.
 *
 *		Someone must run lv_schedule() to redrive the lv_ready_Q.
 *		If this routine was called from within the main lv_schedule()
 *		loop, all required cleanup will be done automatically, after
 *		the return.
 *
 * PARAMETERS:	lb - pointer to a logical buf request
 *
 * DATA STRUCTS:
 *
 * RETURN VALUE: none
 *
 */
void
lv_terminate(lb)
register struct buf *lb;		/* logical buf struct */
{
	register struct lvol *lv;
	register struct buf *next;
	dev_t dev;

	struct volgrp	*vg;		/* VG volgrp pointer		*/

	dev = lb->b_dev;

	/* get VG volgrp structure ptr	*/
	/*
	 * All requests passed to lv_terminate as a linked list
	 * must belong to the same logical volume. In reality, 
	 * the mirror consistency layer never does this.
	 */
	vg = DEV2VG(dev);
	lv = VG_DEV2LV(vg, dev);

	for (; lb != NULL; lb = next) {
		next = lb->av_forw;
		lb->av_forw = NULL;

		if (lb->b_dev != dev)
			panic("lv_terminate devno");

		if (lb->b_options & LVM_RESYNC_OP) {
			int lxno;
			struct lextent *lext;
			struct extent *ext;
			lxno = BLK2EXT(vg,lb->b_blkno);
			lext = LEXTENT(lv,lxno);
			LOCK_INTERRUPT(&(lv->lv_intlock));
			if ((((lext->lx_synctrk == ((TRKPEREXT(vg) - 1)))
			                 || (lb->b_options & LVM_RECOVERY))
			                 && ((lb->b_flags & B_READ) == 0))
				|| (lb->b_flags & B_ERROR)) {
			        /*
				 * completion of final write of a RESYNC,
				 * or final (only) write of a RECOVERY,
				 * or any failure
				 */
				lext->lx_synctrk = -1;
				lb->b_flags |= B_READ;
			} else if (lb->b_flags & B_READ) {
				/* completion of sync op read: turn
				 * it into a write.
				 */
				lb->b_flags &= ~B_READ;
				LV_QUEUE_APPEND(&lv->lv_ready_Q, lb);
				UNLOCK_INTERRUPT(&(lv->lv_intlock));
				continue;
			}
			UNLOCK_INTERRUPT(&(lv->lv_intlock));
		}

		LOCK_INTERRUPT(&(vg->vg_intlock));
		vg->vg_requestcount--;
		UNLOCK_INTERRUPT(&(vg->vg_intlock));

		lv_complete(lv, lb);
	}
	/*
	 * If any requests were unblocked due to this completion, hand
	 * them down to the next level. This must be handed off to a thread.
	 */
	if (lv->lv_ready_Q.lv_head) {
		if (thread_call_one(&lv_threadq, lv_initiate, lv)
				== FALSE) panic("lv_terminate thread_call");
	}

	return;
}

void
lv_complete(lv, lb)
struct lvol *lv;
struct buf *lb;
{
int hash;
	/*
	 *  This logical request has been completed, remove it from the
	 *  the work_Q.
	 */
	hash = LV_HASH(lb);

	LOCK_INTERRUPT(&(lv->lv_intlock));

	lv_unblock(&(lv->work_Q[hash]), &lv->lv_ready_Q, lb,
		lv->lv_status&LV_PAUSING);

	if (lv->lv_status & LV_PAUSED) {
		panic("lv_complete");
	}
	if (lv->lv_status & LV_PAUSING) {
		if (--lv->lv_complcnt == 0) {
			thread_wakeup((vm_offset_t)&(lv->lv_complcnt));
		}
	}
	lv->lv_requestcount--;

	UNLOCK_INTERRUPT(&(lv->lv_intlock));
	
	/* if an iodone routine is given, go to buffer collector */
	/* for spanned logical requests.  This will call biodone */
	/* when all span buffers are accounted for.              */
	if (lb->b_iodone) {
		(*lb->b_iodone)(lb);
		return;
	}

	/* notify caller (most likely buffer cache) of completion */
	biodone(lb);

	return;
}

/*
 *  NAME:	lv_pause
 *
 *  FUNCTION:	Pause the logical volume by preventing requests from proceeding
 *		past the strategy layer. Waits for all currently outstanding
 *		I/O to the logical volume to complete.
 */
void
lv_pause(lv)
struct lvol *lv;
{
	register struct h_anchor *q, *qlimit;
	register int count = 0;
	int s;

	if (lv->lv_status & (LV_PAUSED|LV_PAUSING)) {
		panic("lv_pause");
	}
	LOCK_INTERRUPT(&(lv->lv_intlock));
	lv->lv_status |= LV_PAUSING;

	/*
	 *  Go thru lv workQ and for each request in progress,
	 *  set the LVM_REQ_WANTED bit in the buf struct & increment
	 *  the completion counter.
	 */
	qlimit = lv->work_Q + WORKQ_SIZE; /* get max size of work_Q */

      	for (q = lv->work_Q; q < qlimit; q++)  {
		count += lv_workqcount(q);
	}
	lv->lv_complcnt += count;
#ifdef LV_PAUSE_DEBUG
	if (count)
		printf("lv_pause: counted %d requests.\n", count);
#endif
	/*  Wait for pending requests on this LV to complete. */
	while (lv->lv_complcnt) {
		assert_wait((vm_offset_t)&(lv->lv_complcnt), FALSE);
		UNLOCK_INTERRUPT(&(lv->lv_intlock));

		thread_block();

		LOCK_INTERRUPT(&(lv->lv_intlock));
	}
	lv->lv_status &= ~LV_PAUSING;
	lv->lv_status |= LV_PAUSED;
	UNLOCK_INTERRUPT(&(lv->lv_intlock));

	return;
}

/*
 *  NAME:	lv_continue
 *
 *  FUNCTION:	Release the pause function on a logical volume.
 *		Must search through the work_Q and mark lots of
 *		allegedly blocked things as unblocked.
 */
void
lv_continue(lv)
register struct lvol *lv;
{
	register struct h_anchor *q, *qlimit;
	register struct buf *lb, *next;

	LOCK_INTERRUPT(&(lv->lv_intlock));

	if ((lv->lv_status & (LV_PAUSED|LV_PAUSING)) != LV_PAUSED) {
		panic("lv_continue");
	}
	lv->lv_status &= ~LV_PAUSED;

	qlimit = lv->work_Q + WORKQ_SIZE; /* get max size of work_Q */

      	for (q = lv->work_Q; q < qlimit; q++)  {
		if ((lb = q->bp) != NULL) {
			q->bp = NULL;
			while (lb != NULL) {
				next = lb->av_back;
				if (lv_block(q, &lv->lv_ready_Q, lb, 0)) {
					panic("lv_reblock");
				}
				lb = next;
			}
		}
	}
	UNLOCK_INTERRUPT(&(lv->lv_intlock));

	if (lv->lv_ready_Q.lv_head)
		lv_initiate(lv);

	return;
}

/*
 *  NAME:	lv_collect
 *
 *  FUNCTION:	Reassembles spanned logical requests.  Copies
 *		error info into original buf (if any) then
 *		destroys itself.  If it's the last, it biodones
 *		the original buffer.
 */
void
lv_collect(lb)
struct buf *lb;
{
	int done = 0;
	struct buf *orig, *other;

	other = lb->b_forw;
	orig = lb->b_back;

	/* Transfer errors if any */
	if (lb->b_flags & B_ERROR) {
		orig->b_flags |= B_ERROR;
		orig->b_error = lb->b_error;
		orig->b_resid += lb->b_resid;
	}

	/* If linked to other span buffer, unlink */
	/* Otherwise, all requests are accounted for... */
	if (other) 
		other->b_forw = NULL;
	else 
		done = 1;
	
	/* Free the span buffer */
	KFREE (lb, sizeof(struct buf));

	/* Biodone the original buffer */
	if (done)
		biodone (orig);

	return;
}
