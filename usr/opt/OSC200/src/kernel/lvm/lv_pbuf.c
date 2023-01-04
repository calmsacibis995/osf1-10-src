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
static char	*sccsid = "@(#)$RCSfile: lv_pbuf.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:19:58 $";
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
 * This file is derived from code that carries the following copyright:
 * 
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver
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
 *  lv_pbuf.c -- LVM device driver free pbuf pool manager
 *
 */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>

#include <lvm/lvmd.h>		/* LVM device driver internal definitions */

/*
 * The lv_pbuf_lock is used to protect all of the data structures associated
 * with pbuf allocation and deallocation.  This includes the counts, the pbuf
 * free list and the pbuf pending list.
 */
struct lv_crit lv_pbuf_lock;

/*
 * The following are used to control the allocation/deallocation of pbuf at VG
 * and LV open and close time.
 *
 *    lv_pbuf_grab - Number of pbuf to allocate for each open LV.
 *    lv_pbuf_min -  Minimum number of pbuf to allocate per VG.
 *    lv_pbuf_max -  Maximum number of pbuf to allocate per VG.
 *    lv_pbuf_sys -  Maximum number of pbuf to allocate in the system.
 */

int lv_pbuf_grab = PBSUBPOOLSIZE;
int lv_pbuf_min = PBSUBPOOLSIZE;
int lv_pbuf_max = PBSUBPOOLSIZE * MAXGRABLV;
int lv_pbuf_sys = PBSUBPOOLSIZE * MAXGRABLV * MAXSYSVG;

int lv_vgs_opn = 0;		/* Number of VGs opened			*/
int lv_lvs_opn = 0;		/* Number of LVs opened			*/
int lv_pbuf_cnt = 0;		/* Number of pbuf allocated to all VGs	*/
int lv_pbuf_inuse = 0;		/* Number of pbufs currently in use	*/
int lv_pbuf_maxuse = 0;		/* Maximum number of pbufs ever used	*/

/*
 * Anchor for free pbuf pool. This is locked with the lv_pbuf_lock.
 */
struct pbuf *lv_freebuf = NULL;

/*
 *  NAME:	lv_adjpool
 *
 *  FUNCTION:	Adjust pbuf subpool based on changing number of open devices.
 *
 *  PARAMETERS:	deltavg, deltalv: change in open volgrp and lvol counts.
 *
 *  RETURN:	0 - Success. The appropriate number of pbufs are now in
 *		    the pool.
 *		ENOMEM - Failure. Insufficient memory was available to add
 *		    new pbufs to the pool. Do not proceed with opening
 *		    the device.
 *
 */
int
lv_adjpool(deltavg, deltalv)
int	deltavg;
int	deltalv;
{
	register struct pbuf *pb;	/* ptr to pbuf structure */
	register struct pbuf *subpool;	/* ptr to pool of pbufs to release */
	register struct pbuf *subend;	/* ptr to last pbuf in pool */
	register int deltapool;		/* count of pbufs being added */
	register int i;

	LOCK_INTERRUPT(&lv_pbuf_lock);
	lv_vgs_opn += deltavg;
	lv_lvs_opn += deltalv;
	deltapool = lv_numpbufs(lv_vgs_opn, lv_lvs_opn) - lv_pbuf_cnt;
	UNLOCK_INTERRUPT(&lv_pbuf_lock);
	/* lv_pbuf_cnt is now the ``desired'' pool size */

	if (deltapool > 0) {
		/*
		 * Allocate as many pbufs as I can and put them on the free
		 * pbuf list.
		 */
		for (i = 0; i < deltapool; i++) {
			if ((pb = NEW(struct pbuf)) == NULL) {
				/*
				 * allocation failed, use what we got...
				 */
				break;
			}
			bzero(pb, sizeof(struct pbuf));
#if MACH_LDEBUG
			/*
			 * This trash is to keep overly-assertive
			 * disk drivers happy. The struct buf contained
			 * in the struct pbuf never has it's lock
			 * manipulated, but it must always appear to 
			 * be locked to keep some (notably the Multimax)
			 * disk drivers happy.
			 */
			BUF_LOCKINIT(&(pb->pb));
			BUF_LOCK(&(pb->pb));
			BUF_GIVE_AWAY(&(pb->pb));
#endif
			LOCK_INTERRUPT(&lv_pbuf_lock);
			pb->pb.av_forw = (struct buf *)lv_freebuf;
			lv_freebuf = pb;
			lv_pbuf_cnt++;
			UNLOCK_INTERRUPT(&lv_pbuf_lock);
		}
		thread_call_alloc(&lv_threadq, i);

		/*
		 * If I didn't manage to allocate any pbufs, then return an
		 * error. 
		 */
		if (lv_pbuf_cnt == 0) {
			LOCK_INTERRUPT(&lv_pbuf_lock);
			lv_vgs_opn -= deltavg;
			lv_lvs_opn -= deltalv;
			UNLOCK_INTERRUPT(&lv_pbuf_lock);
			return(ENOMEM);
		}
		return(ESUCCESS);
	}

	/*
	 * There are only closes in progress, or the pool should be reduced.
	 * The locking is structured so that the pbuf_lock does not need to be
	 * held over the call to kfree.
	 */
	else {
		deltapool = -deltapool;

		for (i = 0; i < deltapool; i++) {
			/*
			 * Grab a free buffer to discard. Note: ALLOC_PBUF
			 * macro not used here in order to bypass statistics.
			 * This will require locking in the future.
			 */
			LOCK_INTERRUPT(&lv_pbuf_lock);
			if ((pb = lv_freebuf) == NULL) {
				/*
				 * adjust count for number of buffers not freed
				 */
				UNLOCK_INTERRUPT(&lv_pbuf_lock);
				break;
			}
			lv_freebuf = (struct pbuf *)pb->pb.av_forw;
			pb->pb.av_forw = NULL;
			lv_pbuf_cnt--;
			UNLOCK_INTERRUPT(&lv_pbuf_lock);
			KFREE(pb, sizeof(struct pbuf));
		}
		thread_call_alloc(&lv_threadq, -i);
		return(ESUCCESS);
	}
}

struct pbuf *
lv_alloc_pbuf()
{
register struct pbuf	*pb;		/* physical request buf		*/

	if ((pb = lv_freebuf) == NULL)
		return(NULL);

	lv_freebuf = (struct pbuf *)pb->pb.av_forw;
	pb->pb.av_forw = NULL;
	pb->pb_vgsa_failed = 0;
	lv_pbuf_inuse++;
	if (lv_pbuf_inuse > lv_pbuf_maxuse)
		lv_pbuf_maxuse = lv_pbuf_inuse;
	return(pb);
}

void
lv_free_pbuf(pb)
register struct pbuf	*pb;		/* physical request buf		*/
{
	pb->pb.av_forw = (struct buf *)lv_freebuf;
	lv_freebuf = pb;
	lv_pbuf_inuse--;

	return;
}

/*
 * NAME:	lv_numpbufs 
 * 
 * FUNCTION:	Calculate the number of pbufs that should be currently 
 *		allocated given the number of open VGs and LVs in the
 *		system.  This is a crude method but it does leave some
 * 		room for tuning and prevents LVM from grabbing large
 *		quantities of memory that it may not use.
 *  
 *		This algorithm should include some idea of the mirror
 *		policy.  But currently does not.
 *
 * EXTERN REFERENCES:	lv_pbuf_grab: attempted pbufs/lvol
 *			lv_pbuf_min: minimum pbufs/volgrp
 *			lv_pbuf_max: maximum pbufs/volgrp
 *			lv_pbuf_sys: maximum pbufs/system
 *			lv_pbuf_cnt: current pbufs allocated
 *
 * RETURN VALUE: total number of pbuf structs that should be allocated
 *
 */
int
lv_numpbufs(lvs_open,vgs_open)
int lvs_open;
int vgs_open;
{
	register int	calmax;	/* Calculate maximum	*/
	register int	calmin;	/* Calculate minimum	*/

	/* If no open VGs and LVs then we should not have any pbufs	*/
	if ((lvs_open == 0) && (vgs_open == 0)) {
	    return( 0 );
	}

	/* Calculate the maximum and minimum number of pbufs for system */
	calmax = lvs_open * lv_pbuf_grab; /* Num of open LVs * num to grab */

	/* If calmax exceeds maximum allowed for open VGs adjust it	*/
	if (calmax > (vgs_open * lv_pbuf_max)) {
	    calmax = vgs_open * lv_pbuf_max;
	}

	/* If calmax exceeds maximum pbufs in the system adjust it	*/
	if (calmax > lv_pbuf_sys) {
	    calmax = lv_pbuf_sys;
	}

	calmin = vgs_open * lv_pbuf_min;

	/*
	 * If the calculated minimum is greater than the number already
	 * allocated return the minimum else return the calculate max.
	 */
	if (calmin > lv_pbuf_cnt) {
		return(calmin);
	} else {
		return(calmax);
	}
}
