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
static char	*sccsid = "@(#)$RCSfile: lv_schedule.c,v $ $Revision: 4.2.3.6 $ (DEC) $Date: 1993/01/22 15:50:15 $";
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
 * This file is derived from:

 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_sched.c
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NOTES:
 *    * The selection of the best physical volume to perform I/O to is rather
 *	simplistic.  It relies only on the number of I/Os outstanding to a
 *	physical volume.  At some time it would probably be good to take the
 *	current arm position of the physical volumes into account.  This is
 *	not currently done because the arm position is basically indeterminate
 *	at this level.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <lvm/lvmd.h>


extern void	lv_reschedule();
extern void	lv_reserved();
extern void	lv_finished();
extern void	lv_xlate();

extern void	lv_parallel();
extern void	lv_parread();
extern void	lv_parread_done();
extern void	lv_parwrite_done();
extern void	lv_append();

extern void	lv_sequential();
extern void	lv_seqwrite();
extern void	lv_seqread();
extern void	lv_seqread_done();

extern void	lv_stalepp();
extern void	lv_stalepp_done();

extern void	lv_fixup();

extern struct	pbuf	*lv_allocmulti();	/* Allocate multiple pbufs. */
extern struct	pbuf	*lv_alloc_pbuf();

/*
 * Scheduling switches.
 */
	/* LVM_RESERVED */
struct	lv_sched lv_reserved_ops = {
	lv_reserved,			/* schedule */
	lv_alloc_pbuf,			/* allocate buffer */
};

	/* LVM_SEQUENTIAL */
struct	lv_sched lv_sequential_ops = {	
	lv_sequential,			/* schedule */
	lv_alloc_pbuf,			/* allocate buffer */
};

	/* LVM_PARALLEL */
struct	lv_sched lv_parallel_ops = {
	lv_parallel,			/* schedule */
	lv_allocmulti,			/* allocate buffer */
};

/*
 * Queue of logical requests that are waiting for available physical buffers.
 */
struct	lv_queue lv_pbuf_pending_Q;


/*
 * NAME:         lv_schedule
 *
 * FUNCTION:
 *	Interface to the scheduling layer.  This function will store all
 * logical requests on the pbuf pending queue, and then call lv_reschedule to
 * actually perform the scheduling.  This code is structured this way as there
 * is only one list of available physical buffers.  All requests have to be
 * serialized through the buffer allocation for fairness.
 *
 * RETURN VALUE:
 *	None.
 *
 */
void
lv_schedule(lb, flag)
struct	buf	*lb;			/* Logical buffer list. */
boolean_t flag;				/* thread context/interrupt context */
{
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*next;			/* Next logical buffer in list.	    */
struct	pbuf	*pb;			/* Physical buffer allocated.	    */

	/* Remove and process each logical buffer on the lbuf list. */
	LOCK_INTERRUPT(&(lv_pbuf_lock));
	for (; lb != NULL; lb = next) {
		next = lb->av_forw;	/* unlink this request from list */
		lb->av_forw = NULL;
		
		/* Place the logical request on the pending queue. */
		LV_QUEUE_APPEND(&lv_pbuf_pending_Q, lb);
	}
	UNLOCK_INTERRUPT(&(lv_pbuf_lock));

	/* Start the requests. */
	if (flag == TRUE) {
		/* Called from interrupt context */
		if (thread_call_one(&lv_threadq, lv_reschedule,
				(void *)0) == FALSE) {
			panic("lv_schedule thread_call");
		}
	} else {
		lv_reschedule();
	}
	return;
}

/*
 * NAME:         lv_reschedule
 *
 * FUNCTION:
 *	Schedule pended logical operations.  This function is called when ever
 * physical buffers have been released and there are logical buffers waiting
 * for physical buffers.  This will schedule as many pended logical buffers as
 * possible given the number of available physical buffers.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_reschedule()
{
struct	buf	*lb;			/* Logical buffer to schedule.	    */
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* LV of the I/O.		    */
struct	pbuf	*pb;			/* Physical buffer allocated.	    */

	/*
	 * While there are logical requests on the pending queue, attempt to
	 * schedule them.  This loop is broken out of when it is no longer
	 * possible to allocate physical buffers.
	 */
	LOCK_INTERRUPT(&(lv_pbuf_lock));
	while (lv_pbuf_pending_Q.lv_head != NULL) {
		LV_QUEUE_FETCH(&lv_pbuf_pending_Q, lb);

		/* Get the volume group and the logical volume pointers. */
		vg = DEV2VG(lb->b_dev);
		lv = VG_DEV2LV(vg, lb->b_dev);

		/*
		 * Attempt to allocate pbufs.  If there are not enough, then
		 * place the I/O back on the FRONT of the pending queue.  This
		 * ensures that no I/O is starved.
		 */
		if ((pb = (*lv->lv_schedule->lv_allocbuf)(lv, lb)) == NULL) {
			LV_QUEUE_PREPEND(&lv_pbuf_pending_Q, lb);
			break;
		}

		/* If the pbufs were allocated, schedule the I/O. */
		else {
			UNLOCK_INTERRUPT(&(lv_pbuf_lock));
			(*lv->lv_schedule->lv_schedule)(vg, lv, lb, pb);
			LOCK_INTERRUPT(&(lv_pbuf_lock));
		}
	}
	UNLOCK_INTERRUPT(&(lv_pbuf_lock));

	return;
}

/*
 * NAME:         lv_allocmulti
 *
 * FUNCTION:
 *	Allocate the required number of physical buffers for the logical I/O.
 * This is used by the parallel scheduling policy to allocate physical buffers.
 *
 * LOCKING:
 *	The lv_pbuf_lock must be held.
 *
 * RETURN VALUE:
 *	pbuf - A pointer to the allocate pbuf (or pbuf list) is returned.  If
 * there are not enough pbufs to satisfy the I/O, then NULL is returned.
 */

struct pbuf *
lv_allocmulti(lv, lb)
struct	lvol	*lv;			/* LV of the I/O.		*/
struct	buf	*lb;			/* The logical buffer.		*/
{
struct	pbuf	*pb;			/* The pbuf allocated.		*/
struct	pbuf	*q = NULL;		/* The queue of pbufs allocated.*/
	int	mirror;			/* The mirror being processed.	*/

	/* If this is a parallel read then we only need one buffer. */
	if (lb->b_flags & B_READ)
		return (lv_alloc_pbuf());

	/* Otherwise, allocate and chain together the required buffers. */
	for (mirror = 0; mirror < (lv->lv_maxmirrors+1); mirror++) {
		if ((pb = lv_alloc_pbuf()) == NULL) {
			while (q != NULL) {
				pb = q;
				q = (struct pbuf *)q->pb.b_forw;
				lv_free_pbuf(pb);
			}
			return(NULL);
		}
		pb->pb.b_forw = (struct buf *)q;
		q = pb;
	}
	return(pb);
}

/*
 * NAME:         lv_finished
 *
 * FUNCTION:
 *	End of last physical operation for this logical request.  Free the
 * associated pbuf and terminate the logical request.  If there are logical
 * buffers waiting for available physical buffers, then attempt to reschedule
 * them.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_finished(pb)
struct	pbuf	*pb;
{
struct	buf	*lb;

	lb = pb->pb_lbuf;

	/*
	 * If there was an error from the physical device, then back off the
	 * residual count in the logical buffer to indicate that the operation
	 * do not complete.
	 */
	if (pb->pb.b_flags & B_ERROR)  {
		lb->b_flags |= B_ERROR;
		lb->b_error = pb->pb.b_error;
		lb->b_resid = lb->b_bcount - (pb->pb_addr - lb->b_un.b_addr);
	}
	/*
	 * If an update of the status area failed, mark the logical buffer.
	 * This will cause the mirror write cache to maintain the record for
	 * this I/O.
	 */
	else if (pb->pb_vgsa_failed) {
		lb->b_options |= LVM_VGSA_FAILED;
	}

	/* reinit resid since used when figuring pbuf before PVDD called. */
	else
		lb->b_resid=0;

	/*
	 * Lock the pbuf queue and free the pbuf.  If there are any pending
	 * lbufs waiting for pbufs, then attempt to reschedule them.  This
	 * must be done before lv_mwc_done() as lv_mwc_done() may attempt to
	 * start more operations.
	 */
	LOCK_INTERRUPT(&(lv_pbuf_lock));
	lv_free_pbuf(pb);
	if (lv_pbuf_pending_Q.lv_head != NULL) {
		UNLOCK_INTERRUPT(&(lv_pbuf_lock));
		if (thread_call_one(&lv_threadq, lv_reschedule,
				(void *)0) == FALSE) {
			panic("lv_finished thread_call");
		}
	}
	else {
		UNLOCK_INTERRUPT(&(lv_pbuf_lock));
	}
	lv_mwc_done(lb);
	return;
}

/*
 * NAME:         lv_xlate
 *
 * FUNCTION:
 *	Translate logical to physical address.
 *
 * NOTES:
 *	INPUT:	pb -> pbuf to translate.
 *		mirror = mirror number of extent to use (0, 1 or 2).
 *	OUTPUT:	pb->pb_mirror = mirror number selected.
 *		pb->pb.b_dev = physical device containing this mirror.
 *		pb->pb_start = starting physical block number within b_dev.
 *		pb->pb_pvol = pointer to pvol structure for physical device.
 *
 * RETURN VALUE:
 *	 None.
 */
void
lv_xlate(vg, pb, mirror)
struct	volgrp	*vg;			/* VG volgrp ptr		    */
struct	pbuf	*pb;			/* physical request buf		    */
	int	mirror;			/* mirror number		    */
{
struct	buf	*lb;			/* logical request buf		    */
struct	lvol	*lv;			/* logical volume stucture	    */
struct	pvol	*pv;			/* The physical volume		    */
struct	extent	*ext;			/* physical extent structure	    */
	int	e_no;			/* logical extent number	    */

	/* Access logical requests parameters. */
	lb = pb->pb_lbuf;
	lv = pb->pb_lv;

	/* Compute the extent number and find its structure address. */
	e_no = BLK2EXT(vg, lb->b_blkno);
	ext = EXTENT(lv, e_no, mirror);

	/* Fill in physical operation parameters. */
	pb->pb_addr	 = lb->b_un.b_addr;
	pb->pb.b_bcount	 = lb->b_bcount;
	pb->pb.b_proc	 = lb->b_proc;
	pb->pb.b_flags	 = lb->b_flags;
	(u_int)pb->pb.b_flags &= ~(B_UBC | B_SWAP); /* zap B_UBC and B_SWAP */
	pb->pb_mirror	 = mirror;
	pb->pb_pvol	 = vg->pvols[ext->e_pvnum];
	pb->pb_startaddr = pb->pb_addr;
	pb->pb_endaddr   = pb->pb_startaddr + pb->pb.b_bcount;
	pb->pb_options	 = lb->b_options;

	pv = pb->pb_pvol;

	/* Have to special case lvol 0. - Mapping function is different */
	if (lv == vg->lvols[0])
		pb->pb_start = pv->pv_vgra_psn +
			       (lb->b_blkno - EXT2BLK(vg, e_no));
	else
		pb->pb_start = (ext->e_pxnum * pv->pv_pxspace) +
			       pv->pv_data_psn + 
			       (lb->b_blkno - EXT2BLK(vg, e_no));
}

/*=========================================================================*
 *									   *
 *									   *
 * 		Reserved Area Scheduling Policy				   *
 *									   *
 *									   *
 *=========================================================================*/
/*
 * NAME:         lv_reserved
 *
 * FUNCTION:
 *	Initiate I/O to the physical volume reserved area.  This is a
 * non-mirrored area.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_reserved(vg, lv, lb, pb)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pbuf	*pb;			/* Physical buffer allocated.	    */
{
	struct extent *ext;
	int e_no, pvnum;
	struct pvol *pv;

	/* Initialize the physical buffer. */
	pb->pb_lv = lv;
	pb->pb_lbuf = lb;
	pb->pb_sched = lv_finished; /* regular I/O done handler*/

	e_no = BLK2EXT(vg, lb->b_blkno);
	ext = EXTENT(lv, e_no, PRIMMIRROR);
	pvnum = ext->e_pvnum;

	if ((pvnum != PX_NOPV)
		&& ((pv = vg->pvols[pvnum]) != NULL)
		&& ((pv->pv_flags & LVM_NOTATTACHED) == 0)) {
		lv_xlate(vg, pb, PRIMMIRROR);
		lv_begin(pb);				/* Start the I/O. */
		return;
	}
	pb->pb.b_flags |= B_ERROR;
	pb->pb.b_error = EIO;

	/* Dummy the address up so b_resid will be correct */
	pb->pb_addr = lb->b_un.b_addr;
	lv_finished(pb);

	return;
}

/*=========================================================================*
 *									   *
 *									   *
 * 		Parallel Scheduling Policy				   *
 *									   *
 *									   *
 *=========================================================================*/
/*
 * NAME:         lv_parallel
 *
 * FUNCTION:
 *	Initiate parallel mirrored physical operations.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_parallel(vg, lv, lb, free_q)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pbuf	*free_q;		/* Single linked pbuf free list.    */
{
struct	pbuf	*io_q = NULL;		/* Double linked pbuf I/O list.	    */
struct	pbuf	*pb, *next;		/* Physical buffer being used.	    */
struct	extent	*ext;			/* Generic extent pointer.	    */
struct	lextent	*lext;
struct	pvol	*pv;			/* Generic pvol pointer.	    */
	int	e_no;			/* Logical extent number.	    */
	int	mirror, mirror_mask;	/* Mirror number.		    */
	int	avoidmask = 0;		/* Mask of done mirrors.	    */
	int	donemask = 0;		/* Mask of done mirrors.	    */
	int	actmask = 0;		/* Mask of active mirrors.	    */

	/* Reads are done the same for parallel and sequential schedules */
	if (lb->b_flags & B_READ) {
		lv_parread(vg, lv, lb, free_q);
		return;
	}

	/* Compute the extent number of the operation. */
	e_no = BLK2EXT(vg, lb->b_blkno);

	/* For each mirror, setup the physical I/O buffer. */
	for (mirror = 0; mirror < (lv->lv_maxmirrors+1); mirror++) {

		mirror_mask = MIRROR_MASK(mirror); 
		/* Mark this mirror as having been processed. */
		donemask |= mirror_mask;

		/* If there is no mirror here, then avoid this mirror. */
		ext = EXTENT(lv, e_no, mirror);
		if (ext->e_pvnum == PX_NOPV) {
			continue;
		}

		pv = vg->pvols[ext->e_pvnum];
		if (pv->pv_flags & LVM_PVMISSING) {
			avoidmask |= mirror_mask;
			continue;
		}
		if (lb->b_options & LVM_RESYNC_OP) {
			lext = LEXTENT(lv,e_no);
			if ((lext->lx_syncmsk & mirror_mask) == 0) {
				continue;
			}
		}
		/* On writes, we write to stale extents, even though
		 * the cycles are probably wasted. Otherwise, we have
		 * a nasty race against resync in progress. */

		/* Mark this mirror as active. */
		actmask |= mirror_mask;

		/* Use the next buffer on the list,
		 * Make sure that enough pbufs have been allocated.
		 */
		if ((pb = free_q) == NULL)
			panic("lv_parallel: pbuf list exhausted.\n");

		if (free_q != NULL)
			free_q = (struct pbuf *)free_q->pb.b_forw;

		/* Set up physical request parameters */
		pb->pb_lbuf = lb;
		pb->pb_lv = lv;
		pb->pb_sched = lv_parwrite_done;
		lv_xlate(vg, pb, mirror);
		lv_append(pb, &io_q);
	};

	/* If there are any free physical buffers left over, release them. */
	while (free_q != NULL) {
		pb = free_q;
		free_q = (struct pbuf *)free_q->pb.b_forw;
		LOCK_INTERRUPT(&(lv_pbuf_lock));
		lv_free_pbuf(pb);
		UNLOCK_INTERRUPT(&(lv_pbuf_lock));
	}

	/* If none of the mirrors were good, then return an error in lb. */
	if (io_q == NULL) {
		lb->b_flags |= B_ERROR;
		lb->b_error = EIO;
		lv_mwc_done(lb);	/* notify requestor */
		return;
	}

	/* Initiate the write for each pbuf in the work queue. */
	next = io_q;
	do {
		pb = next;
		next = (struct pbuf *) pb->pb.b_forw;

		/* Set the masks in all of the physical buffers. */
		pb->pb_miravoid = avoidmask;
		pb->pb_mirdone = donemask;
		pb->pb_miract = actmask;
		pb->pb_mirbad = 0;

		lv_begin(pb);
	} while (pb != (struct pbuf *)io_q->pb.b_back);

	return;
}

/*
 * NAME:         lv_parread
 *
 * FUNCTION:
 *	Initiate a read from a possibly mirrored logical volume.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_parread(vg, lv, lb, pb)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pbuf	*pb;			/* Physical buffer allocated.	    */
{

	/* Initialize the physical buffer. */
	pb->pb_lbuf = lb;
	pb->pb_lv = lv;
	pb->pb_sched = lv_parread_done;
	pb->pb_mirdone = lb->b_options & LVM_MIRAVOID;
	pb->pb_mirbad = 0;
	pb->pb_miract = 0;

	/* Select the best mirror, if there are no available mirrors, return */
	if (lv_bestmir(vg, pb) != -1) {
		/* Start the I/O */
		pb->pb.b_flags &= ~B_ERROR;
		lv_begin(pb);
		return;
	}

	pb->pb.b_flags |= B_ERROR;
	pb->pb.b_error = EIO;
	lv_finished(pb);
	return;
}

/*
 * NAME:         lv_parwrite_done
 *
 * FUNCTION:
 *	Parallel mirrored write completion routine.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_parwrite_done(pb)
struct	pbuf	*pb;			/* Physical request buffer.	*/
{
struct	pbuf	*q;			/* Physical request queue.	*/
struct	volgrp	*vg;

	vg = pb->pb_pvol->pv_vg;

	/*
	 * We're touching the contents of 2 pbufs simultaneously. Use
	 * the global pbuf lock to serialize access.
	 */

	LOCK_INTERRUPT(&(lv_pbuf_lock));
	/* if physical operation failed, mark this mirror as broken */
	if (pb->pb.b_flags & B_ERROR)  {
		pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);
	}

	/*
	 * If there are some request still outstanding, save the mirbad
	 * information, dequeue the buffer and release it.
	 */
	q = (struct pbuf *) pb->pb.b_forw;
	if (pb != q)  {

		/* Propogate the bad mask. */
		q->pb_mirbad |= pb->pb_mirbad;

		/* Unlink the physical buffer. */
		q->pb.b_back = pb->pb.b_back;
		pb->pb.b_back->b_forw = pb->pb.b_forw;

		/* Release the physical buffer. */
		lv_free_pbuf(pb);
		if (lv_pbuf_pending_Q.lv_head != NULL) {
			UNLOCK_INTERRUPT(&(lv_pbuf_lock));
			if (thread_call_one(&lv_threadq, lv_reschedule,
					(void *)0) == FALSE) {
				panic("lv_parwrite_done thread_call");
			}
		} else {
			UNLOCK_INTERRUPT(&(lv_pbuf_lock));
		}
	} else {
		UNLOCK_INTERRUPT(&(lv_pbuf_lock));
		/*
		 * This is the last request, mark the I/O as complete.
		 * If any of the mirrors were bad, do stale PP handling.
		 */
		pb->pb_mirbad |= pb->pb_miravoid;
		if (pb->pb_mirbad || (pb->pb_lbuf->b_options & LVM_RESYNC_OP))
			lv_stalepp(vg, pb);
		else
			lv_finished(pb);
	}
}

/*
 * NAME:	lv_parread_done
 *
 * FUNCTION:	Mirrored read completion function.
 *
 * RETURN VALUE: None
 */
void
lv_parread_done(pb)
struct	pbuf	*pb;			/* Physical buffer written.	    */
{
struct volgrp 	*vg;

	/* On a read failure, attempt to read the next mirror. */
	if (pb->pb.b_flags & B_ERROR) {

		/* If media error, mark this mirror for future fixup */
		if (pb->pb.b_error == EMEDIA)
			pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);

		/* If there is another mirror, retry the read. */
		vg = pb->pb_pvol->pv_vg;
		if (lv_bestmir(vg, pb) != -1)  {
			pb->pb.b_flags &= ~B_ERROR;
			lv_begin(pb);
			return;
		}
		lv_finished(pb);	/* all possible mirrors bad */
		return;
	}

	/* The read was successful, fixup any previous errors. */
	if (pb->pb_mirbad && !(pb->pb_lbuf->b_options & LVM_OPT_NORELOC)) {
		lv_fixup(pb);
		return;
	}

	lv_finished(pb);
	return;
}

/*
 * NAME:	lv_append
 *
 * FUNCTION:
 *	Append the specified pb to a doubly-linked circular queue containing
 * all the physical requests for this operation.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_append(pb, qq)
struct	pbuf	*pb;			/* Physical buffer to queue.	*/
struct	pbuf	**qq;			/* Physical buffer queue.	*/
{
struct	pbuf	*q;

	q = *qq;
	if (q == NULL)  {				/* First request. */
		pb->pb.b_forw = &pb->pb;
		pb->pb.b_back = &pb->pb;
		*qq = pb;
	}
	else  {
		pb->pb.b_forw = q->pb.b_back->b_forw;
		q->pb.b_back->b_forw = &pb->pb;
		pb->pb.b_back = q->pb.b_back;
		q->pb.b_back = &pb->pb;
	}
}

/*
 * NAME:	lv_bestmir
 *
 * FUNCTION:
 *	Find the best mirror to read.  The best mirror is selected
 * from those mirrors that are not already marked done.  The first factor that
 * is used to find the best mirror is the number of existing I/O operations to
 * the physical volume.  The PV with the fewest number of I/Os is chosen.
 *
 * RETURN VALUE:
 *	The best mirror to use.  If -1 is returned, then there is no active
 * mirror to be read.
 */
int
lv_bestmir(vg, pb)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	pbuf	*pb;			/* Physical request buffer.	    */
{
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pvol	*pv;			/* Generic physical volume pointer. */
struct	extent	*ext;			/* physical extent structure	    */
struct 	lextent	*lext;			/* logical extent structure	    */
	int	minxfs = -1;		/* The minimum number of xfrs found */
	int	e_no;			/* Logical extent number.	    */
	int	mirror, mirror_mask;	/* The current mirror being checked */
	int	bestmir = -1;		/* The best mirror.		    */

	/* Initialize the logical pointers. */
	pb->pb_pvol = NULL;
	lv = pb->pb_lv;
	lb = pb->pb_lbuf;

	/* Get the extent number and block offset. */
	e_no = BLK2EXT(vg, lb->b_blkno);

	/* For each copy. */
	for (mirror = 0; mirror < (lv->lv_maxmirrors + 1); mirror++) {

		mirror_mask = MIRROR_MASK(mirror);
		/* If the mirror is already done, skip it. */
		if (pb->pb_mirdone & mirror_mask)
			continue;

		/* If there is no mirror here, skip it. */
		ext = EXTENT(lv, e_no, mirror);
		if (ext->e_pvnum == PX_NOPV) {
			continue;
		}

		/*
		 * If the extent is missing, avoid it
		 * and mark it done.
		 */
		pv = vg->pvols[ext->e_pvnum];
		if (pv->pv_flags & LVM_PVMISSING) {
			pb->pb_mirdone |= mirror_mask;
			if ((lb->b_flags & B_READ) == 0) {
				/*
				 * This is a write, remember that we skipped
				 * writing this mirror.
				 */
				pb->pb_miravoid |= mirror_mask;
			}
			continue;
		}
		/* We must not access a stale mirror on reads */
		if ((lb->b_flags & B_READ) && (ext->e_state & PX_STALE)) {
			pb->pb_mirdone |= mirror_mask;
			continue;
		}

		/* Fewest number of outstanding I/Os is first determinant. */
		if ((minxfs == -1) || (pv->pv_curxfs < minxfs)) {
			bestmir = mirror;
			minxfs = pv->pv_curxfs;
		}
	}

	/* If a best mirror was selected, then mark this I/O as active. */
	if (bestmir != -1) {
		pb->pb_mirdone |= MIRROR_MASK(bestmir);
		pb->pb_miract |= MIRROR_MASK(bestmir);
		lv_xlate(vg, pb, bestmir);
	}
	return(bestmir);
}

/*=========================================================================*
 *									   *
 *									   *
 * 		Sequential Scheduling Policy				   *
 *									   *
 *									   *
 *=========================================================================*/
/*
 * NAME:         lv_sequential
 *
 * FUNCTION:
 *	Initiate sequential mirrored physical operation.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sequential(vg, lv, lb, pb)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pbuf	*pb;			/* Physical buffer allocated.	    */
{

	/* Initialize the physical buffer. */
	pb->pb_lbuf = lb;
	pb->pb_lv = lv;

	if (lb->b_flags & B_READ)
		pb->pb_sched = lv_seqread_done;
	else
		pb->pb_sched = lv_seqwrite;
	pb->pb_mirdone = 0;
	pb->pb_mirbad = 0;
	pb->pb_miract = 0;

	/* Select the next mirror, if there are no available mirrors, return */
	if (lv_nextmir(vg, pb) != -1) {
		/* Start the I/O */
		pb->pb.b_flags &= ~B_ERROR;
		lv_begin(pb);
		return;
	}
	pb->pb.b_flags |= B_ERROR;
	pb->pb.b_error = EIO;
	/* Dummy the address up so b_resid will be correct */
	pb->pb_addr = lb->b_un.b_addr;
	lv_finished(pb);

	return;
}

/*
 * NAME:         lv_seqwrite
 *
 * FUNCTION:
 *	Mirrored sequential write physical operation end routine.
 *
 * IBM NOTE:
 *	  Writes to all mirrors are attempted before reporting any stale
 * mirrors to the daemon.  The assumption is that if all mirrors fail on
 * writes then they are not out of sync with each other.  Of course this
 * leaves a small window if the mirrors all fail but fail at different disk
 * blocks in the request.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_seqwrite(pb)
struct	pbuf	*pb;			/* Physical buffer written.	    */
{
struct volgrp *vg;

	vg = pb->pb_pvol->pv_vg;

	/* If physical operation failed mark this mirror as broken. */
	if (pb->pb.b_flags & B_ERROR) {
		pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);
	}

	/* Select next mirror and begin another physical write. */
	if (lv_nextmir(vg, pb) != -1) {
		pb->pb.b_flags &= ~B_ERROR;
		lv_begin(pb);
		return;
	}

	/* this was the final request, mark the I/O as complete. */
	/* If any of the mirrors were bad, do stale PP handling first. */

	pb->pb_mirbad |= pb->pb_miravoid;
	if (pb->pb_mirbad || (pb->pb_lbuf->b_options & LVM_RESYNC_OP))
		lv_stalepp(vg, pb);
	else
		lv_finished(pb);
	return;
}

/*
 * NAME:         lv_seqread_done
 *
 * FUNCTION:
 *	Sequential mirrored read completion function.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_seqread_done(pb)
struct	pbuf	*pb;			/* Physical buffer written.	    */
{

	/* On a read failure, attempt to read the next mirror. */
	if (pb->pb.b_flags & B_ERROR) {

		/* If media error, mark this mirror for future fixup */
		if (pb->pb.b_error == EMEDIA)
			pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);

		/* If there is another mirror, retry the read. */
		if (lv_nextmir(pb) != -1)  {
			pb->pb.b_flags &= ~B_ERROR;
			lv_begin(pb);
			return;
		}
		lv_finished(pb);	/* all possible mirrors bad */
		return;
	}

	/* The read was successful, fixup any previous errors. */
	if (pb->pb_mirbad && !(pb->pb_lbuf->b_options & LVM_OPT_NORELOC)) {
		lv_fixup(pb);
		return;
	}

	lv_finished(pb);
	return;
}

/*
 * NAME:	lv_nextmir
 *
 * FUNCTION:	Choose the next mirror to be processed, in sequential order.
 *		Used by sequential read and sequential write.
 *
 * RETURN VALUE:
 *		The next mirror to use.  If -1 is returned, then there are
 *		no remaining active mirrors to be accessed.
 */
int
lv_nextmir(vg, pb)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	pbuf	*pb;			/* Physical request buffer.	    */
{
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pvol	*pv;			/* Generic physical volume pointer. */
struct	extent	*ext;			/* physical extent structure	    */
struct 	lextent	*lext;
	int	e_no;			/* Logical extent number.	    */
	int	mirror, mirror_mask;	/* The current mirror being checked */

	/* Initialize the logical pointers. */
	pb->pb_pvol = NULL;
	lv = pb->pb_lv;
	lb = pb->pb_lbuf;

	/* Get the extent number and block offset. */
	e_no = BLK2EXT(vg, lb->b_blkno);

	/* For each copy. */
	for (mirror = 0; mirror < (lv->lv_maxmirrors + 1); mirror++) {

		mirror_mask = MIRROR_MASK(mirror);
		/* If the mirror is already done, skip it. */
		if (pb->pb_mirdone & mirror_mask)
			continue;

		/* Mark this mirror as having already been processed */
		pb->pb_mirdone |= mirror_mask;

		/* If there is no mirror here, skip it. */
		ext = EXTENT(lv, e_no, mirror);
		if (ext->e_pvnum == PX_NOPV) {
			break;
		}

		pv = vg->pvols[ext->e_pvnum];
		/*
		 * Don't access missing physical volumes, but
		 * remember that we avoided writing to them.
		 */
		if (pv->pv_flags & LVM_PVMISSING) {
			if ((lb->b_flags & B_READ) == 0) {
				/*
				 * This is a write, remember that we skipped
				 * writing this mirror.
				 */
				pb->pb_miravoid |= mirror_mask;
			}
			continue;
		}
		if (lb->b_options & LVM_RESYNC_OP) {
			lext = LEXTENT(lv,e_no);
			if ((lext->lx_syncmsk & mirror_mask) == 0) {
				continue;
			}
		} else {
			/* Must never read a stale mirror */
			if ((lb->b_flags & B_READ)
					&& (ext->e_state & PX_STALE)) {
				continue;
			}
		}
		pb->pb_miract |= mirror_mask;
		lv_xlate(vg, pb, mirror);
		return(mirror);
	}
	return(-1);
}

/*=========================================================================*
 *									   *
 *									   *
 * 		Stale Extent Processing					   *
 *									   *
 *									   *
 *=========================================================================*/
/*
 * NAME:	lv_stalepp
 *
 * FUNCTION:
 *	Change physical extent states from active to stale.
 *
 *	If all of the active mirrors failed, one mirror will be selected to
 * remain active while the others are marked stale.  The last error will be
 * returned to the originator.
 *
 *	If a loss of quorum happens while this operation is in progress (i.e.
 * between here and lv_stalepp_done()) then the request will be returned with
 * an error.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_stalepp(vg, pb)
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	pbuf	*pb;			/* Physical request buffer.	    */
{
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	extent	*ext;			/* Generic extent pointer.	    */
struct	lextent	*lext;
	int	e_no;			/* Logical extent number.	    */
	int	badmask;		/* Mask of the bad disks.	    */
	int	mirror;			/* The current mirror be examined.  */
	int	fresh;

	/* Access the logical request structures. */
	lv = pb->pb_lv;
	lb = pb->pb_lbuf;

	/* Compute the extent number of the operation. */
	e_no = BLK2EXT(vg, lb->b_blkno);

	/*
	 * If no I/O succeeded, then we have to have to chose one of active
	 * extents to NOT mark stale.  This is because we should never mark
	 * all of the extents as stale.  The other failing extents will be
	 * marked stale.
	 */
	if (pb->pb_mirdone == pb->pb_mirbad) {
		pb->pb_mirbad &= ~(MIRROR_MASK(FIRST_MASK(pb->pb_miract)));
		if ((pb->pb.b_flags & B_ERROR) == 0) {
			pb->pb.b_flags |= B_ERROR;
			pb->pb.b_error = EIO;
		}
	}
	else
		pb->pb.b_flags &= ~B_ERROR;

	badmask = pb->pb_mirbad;
	if (lb->b_options & LVM_RESYNC_OP) {
		lext = LEXTENT(lv, e_no);
		LOCK_INTERRUPT(&(lv->lv_intlock));
		lext->lx_syncerr |= pb->pb_mirbad;
		lext->lx_syncerr |= pb->pb_miravoid;
		lext->lx_syncmsk &= ~lext->lx_syncerr;
		if (lext->lx_syncmsk == 0) {
			pb->pb.b_flags |= B_ERROR;
		} else if ((lb->b_options & LVM_RECOVERY) == 0) {
			if (lext->lx_synctrk == (TRKPEREXT(vg)-1)) {
				fresh = 0;
				while ((mirror = FIRST_MASK(lext->lx_syncmsk))
						!= LVM_MAXCOPIES) {
					lext->lx_syncmsk &=
							~(MIRROR_MASK(mirror));
					ext = EXTENT(lv, e_no, mirror);
					if (ext->e_pvnum == PX_NOPV)
						panic("lvresync changed");
					if (ext->e_state & PX_STALE) {
						ext->e_state &= ~PX_STALE;
						fresh |= MIRROR_MASK(mirror);
					} else {
						panic("sync not stale");
					}
				}
				UNLOCK_INTERRUPT(&(lv->lv_intlock));
				if (fresh) {
					pb->pb_pvol = (struct pvol *)vg;
					pb->pb_sched = lv_stalepp_done;
					pb->pb_mirdone = fresh;
					lv_sa_start(vg, pb, SA_FRESHPX);
				} else  {
					lv_finished(pb);
				}
				return;
			}
		}
		UNLOCK_INTERRUPT(&(lv->lv_intlock));
	} else if (badmask != 0)  {
		lext = LEXTENT(lv, e_no);
		LOCK_INTERRUPT(&(lv->lv_intlock));
		lext->lx_syncerr |= pb->pb_mirbad;
		UNLOCK_INTERRUPT(&(lv->lv_intlock));
	}

	/*
	 * For all of the bad mirrors, check to see if it is currently marked
	 * stale on disk.  If it is, then we don't have to mark it.  If it is
	 * not, then we either have to mark it or wait for it to be marked.
	 */
	while ((mirror = FIRST_MASK(badmask)) != LVM_MAXCOPIES) {
		badmask &= ~(MIRROR_MASK(mirror));
		ext = EXTENT(lv, e_no, mirror);
		if (((ext->e_state & (PX_STALE|PX_CHANGING)) == PX_STALE)) {
			pb->pb_mirbad &= ~(MIRROR_MASK(mirror));
		} else if ((ext->e_state & PX_STALE) == 0) {
			ext->e_state |= (PX_STALE | PX_CHANGING);
		}
	}

	/* If there are any bad mirrors left, then write the VGSA. */
	if (pb->pb_mirbad) {
		pb->pb_pvol = (struct pvol *)vg;
		pb->pb_sched = lv_stalepp_done;
		lv_sa_start(vg, pb, SA_STALEPX);
	} else {
		lv_finished(pb);
	}
}

/*
 * NAME:	lv_stalepp_done
 *
 * FUNCTION:
 *	Completion function for stale physical extent processing.
 *
 *	This function is called after all of the VGSAs have been updated.
 * This will clear all of the changing bits on the extents marked stale on the
 * disk by this operation.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_stalepp_done(pb)
struct	pbuf	*pb;			/* Physical request buffer.	*/
{
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	extent	*ext;			/* Generic extent pointer.	    */
	int	mirmask;		/* The mask of mirrors marked stale */
	int	mirror;			/* The current mirror number.	    */
	int	e_no;			/* Logical extent number.	    */

	/* Access the logical request structures. */
	lb = pb->pb_lbuf;
	lv = pb->pb_lv;
	(struct pvol *)vg = pb->pb_pvol;
/*******vg = pb->pb_pvol->pv_vg; *******/

	/* If the update of the VGSA failed, reflect the error back up. */
	if (pb->pb_vgsa_failed) {
		lv_finished(pb);
		return;
	}

	/* Compute the extent number of the operation. */
	e_no = BLK2EXT(vg, lb->b_blkno);

	/* Reset the changing flag for each extent marked stale. */
	mirmask = pb->pb_mirbad;
	while ((mirror = FIRST_MASK(mirmask)) != LVM_MAXCOPIES) {
		mirmask &= ~(MIRROR_MASK(mirror));
		ext = EXTENT(lv, e_no, mirror);
		ext->e_state &= ~PX_CHANGING;
	}

	/* Mark the operation as finished. */
	lv_finished(pb);
}

/*=========================================================================*
 *									   *
 *									   *
 * 		Mirrored Defect Recovery				   *
 *									   *
 *									   *
 *=========================================================================*/
/*
 * NAME:         lv_fixup
 *
 * FUNCTION:
 *	Rewrite broken mirrors to attempt bad block relocation.  Most media
 * errors are discovered on reads, but bad block relocation only works during
 * writes.  Mirroring provides an opportunity to repair broken blocks.  When a
 * media error was encountered one or more of the mirrors during a read, and
 * one of the mirrors successfully completed the read, this routine is called
 * to initiate writes of the good data to the bad mirror(s).  This routine
 * sets itself up as a new scheduling policy, which runs until all repairs
 * have been attempted.  I/O errors encountered during repair operations are
 * ignored.  The original logical operation has still succeeded.
 *
 * RETURN VALUE:
 *	None
 */
void
lv_fixup(pb)
struct	pbuf	*pb;			/* Physical request buffer.	*/
{
struct	volgrp	*vg;			/* VG of the I/O.		    */
struct	lvol	*lv;			/* The I/O's logical volume.	    */
struct	buf	*lb;			/* Logical buffer list.		    */
struct	pvol	*pv;			/* Generic physical volume pointer. */
struct	extent	*ext;			/* physical extent structure	    */
	int	e_no;			/* The extent number.		    */
	int	mirror;			/* Mirror being fixed.		*/

	/* Initialize the logical pointers. */
	vg = pb->pb_pvol->pv_vg;
	lv = pb->pb_lv;
	lb = pb->pb_lbuf;

	/* Get the extent number and block offset. */
	e_no = BLK2EXT(vg, lb->b_blkno);

	/* Turn this request into a write, and ignore any rewrite errors. */
	pb->pb.b_flags &= ~(B_ERROR|B_READ);
	pb->pb_sched = lv_fixup;	/* new scheduling policy */

	/* Look for the first non-stale and non-missing extent. */
	while ((mirror = FIRST_MASK(pb->pb_mirbad)) != LVM_MAXCOPIES) {
		pb->pb_mirbad &= ~MIRROR_MASK(mirror);

		ext = EXTENT(pb->pb_lv, e_no, mirror);
		pv = vg->pvols[ext->e_pvnum];
		if (!(ext->e_state & PX_STALE)
		    	&& !(pv->pv_flags & LVM_PVMISSING))
			break;
	}
		
	/* Select the next broken mirror.  If all were fixed, we are done. */
	if (mirror == LVM_MAXCOPIES) {
		lv_finished(pb);
		return;
	}

	/* Translate the operation and restart the I/O. */
	lv_xlate(vg, pb, mirror);
	lv_begin(pb);
}
