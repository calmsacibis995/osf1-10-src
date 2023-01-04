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
static char	*sccsid = "@(#)$RCSfile: lv_block.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:18:43 $";
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

#include <sys/errno.h>
#include <sys/buf.h>
#include <lvm/lv_q.h>
#include <lvm/ltg.h>

/*
 * The functions in this file must only be called from within the
 * associated logical volume critical section.
 */

/*
 * NAME:	lv_block
 *
 * FUNCTION:	Place a request onto a hash chain, possibly blocking
 *		it due to overlapping requests.
 *
 * OUTPUT:	logical request added to queue, if it is not blocked.
 *
 * RETURN VALUE: 0 for success, errno from lv_workmask (EINVAL)
 *		if transfer is not wholly contained within 1 logical
 *		track group.
 *
 */
int
lv_block(hashpp, q, lb, flag)
register struct h_anchor *hashpp;	/* pointer to start of hash chain */
register struct lv_queue *q;	/* queue to place non-blocked requests on */
register struct buf *lb;	/* the request */
int flag;			/* LV pause flag */
{
register struct buf **p;
register int conflicts;		/* Mask of conflicting (overlapping) pages */
register int ltgno;		/* logical track group number */
register int error;

	if (hashpp == NULL)
		panic("lv_block hash chain pointer");

	if (q == NULL)
		panic("lv_block queue");

	if (lb == NULL)
		panic("lv_block request");


	if (error = lv_workmask(lb)) {
		return(error);
	}

	conflicts = 0;
	ltgno = BLK2TRK(lb->b_blkno);

	for (p = &(hashpp->bp); *p != NULL; p = &((*p)->av_back)) {
		/*
		 * If the new request is in the same LTG as this entry
		 * in the hash queue, record which blocks in the LTG
		 * it is accessing.
		 */
		if (BLK2TRK((*p)->b_blkno) == ltgno) {
			conflicts |= (*p)->b_work;
		}
	}

	/* append buf to end of hash chain */
	*p = lb;
	lb->av_back = NULL;

	/* is there a conflict? */
	if (!flag && !(conflicts & lb->b_work)) {
		lb->b_error = 0;	/* request is not blocked */
			/* add valid, unblocked request to queue */
		LV_QUEUE_APPEND(q,lb);
	} else {
		lb->b_error = ELBBLOCKED;	/* request is blocked */
	}
	return (0);
}

/*
 * NAME:	lv_unblock
 *
 * FUNCTION:	Try to unblock a logical request.
 *
 * OUTPUT:	logical request added to queue, if no longer blocked.
 *
 * RETURN VALUE: none
 *
 */
int
lv_unblock(hashpp, q, lb, flag)
register struct h_anchor *hashpp;	/* pointer to the hash chain */
register struct lv_queue *q;	/* queue to put unblocked requests on */
register struct buf *lb;	/* request that is to be extracted. */
int flag;			/* LV pause flag */
{
register struct buf **p;
register int conflicts;		/* Mask of conflicting (overlapping) pages */
register int ltgno;		/* logical track group number */
register struct buf *next;
register struct buf *chain;

	if (hashpp == NULL)
		panic("lv_unblock hash chain pointer");

	if (lb == NULL)
		panic("lv_unblock request");


	/* Search this hash chain for this entry. */
        for (p = &(hashpp->bp); *p != lb; p = &((*p)->av_back)) {
		if (*p == NULL) {
			panic("lvm: terminated request not on work_Q");
		}
	}

	/* pull this request out of the hash chain.
	 * p points to a thing that points to the current request. By
	 * by overwriting *p with the link, we remove our request 
	 * from the list
	 */
        *p = lb->av_back;

	if (flag)
		return;
		
	if (q == NULL)
		panic("lv_unblock queue");

	/*
	 *  Unblock any logical requests waiting on the one that finished.
	 *  Scan newer blocked requests on the same hash chain to see if they
	 *  can be unblocked.  Don't look at older requests, they cannot
	 *  have been waiting on this event.
	 */
	for (next = lb->av_back; next != NULL; next = next->av_back)  {
		if (next->b_error != ELBBLOCKED) continue;

		conflicts = 0;
		ltgno = BLK2TRK(next->b_blkno);

		/*
		 * Walk from begining of hash chain, building a busy mask.
		 * for this logical track group.
		 */
		for (chain=hashpp->bp; chain != next; chain=chain->av_back) {
			if (BLK2TRK(chain->b_blkno) == ltgno)
				conflicts |= chain->b_work;
		}

		/* if no longer any conflicts, put on queue */
		if (!(conflicts & next->b_work))  {
			/* no more conflicts */
			next->b_error = 0; /* no longer blocked. */
			LV_QUEUE_APPEND(q,next);
		}
	}
	return;
}

/*
 * lv_workmask: initializes the b_work field of a logical struct buf
 * to contain a mask of bits corresponding to which pages within the
 * logical track group are affected by this I/O. This routine will
 * fail if the I/O request is not wholly contained in 1 logical track
 * group.
 */
int
lv_workmask(lb)
register struct buf *lb;
{
register u_int bct;
register u_int bwp;
register u_int pwt;
register u_int pct;
int error = 0;

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
	pct = BLK2PG(bwp + bct - 1) + 1;

#if NOTDEF
	printf("blockno:           %d\n", lb->b_blkno);
	printf("block within page: %d\n", bwp);
	printf("block count:       %d\n", bct);
	printf("page count:        %d\n", pct);
	printf("page within ltg:   %d\n", pwt);
#endif

	lb->b_work = (((unsigned)(~0))>>(PGPTRK-pct)) << pwt;

	/* check for invalid length or track group misalignment */
	if (pwt+pct > PGPTRK) {
		error = EINVAL;
	}
	return(error);
}

/*
 *  NAME:	lv_workqcount
 *
 *  FUNCTION:	counts all in-progress requests on a work_Q hash
 *		chain so that completion of these requests can
 *		trigger lv_pause to continue.
 */
int
lv_workqcount(q)
register struct h_anchor *q;
{
	register struct buf *lb;
	register int count = 0;

	for (lb = q->bp; lb != NULL; lb = lb->av_back)  {
		if (lb->b_error != ELBBLOCKED) {
			count++;
		}
	}
	return(count);
}
