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
static char	*sccsid = "@(#)$RCSfile: lv_subr.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:21:00 $";
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
 * MODULE_NAME:
 *	lv_subr.c - Random logical volume subroutines.
 *
 * FUNCTIONS:
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
 * Include files.
 */
#include <lvm/lvmd.h>

#include <sys/errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/namei.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/ucred.h>
#include <sys/user.h>


/*
 * NAME:	lv_openpv
 *
 * FUNCTION:
 *	Open a physical volume.  The block device containing the physical
 * volume is opened and the LVM record is read.
 *
 * PARAMETERS:
 *	path - The pathname of the physical volume.
 *	lvmrec - Pass by reference pointer to the LVM record.
 *
 * NOTES:
 *
 * RETURN:
 *	EINVAL - Invalid ioctl argument structure.
 *
 */
extern int
lv_openpv(path, pv)
char *path;		/* The physical volume pathname. */
struct pvol *pv;
{
    struct nameidata	*ndp = &u.u_nd;	/* Nameidata for path lookup.	 */
    struct vnode	*vp;		/* Vnode of the physical volume. */
    int error = ESUCCESS;		/* The running error.		 */
    int status;
					
    /*
     * Get a vnode for the physical volume.
     */
    if ((error = getmdev(&vp, path, ndp)) != 0)
	return(error);

    /*
     * Open the vnode and read the logical volume record.
     */
    VOP_OPEN(&vp, FREAD|FWRITE, NOCRED, error);
    if (error != ESUCCESS) {
	vrele(vp);
	return(error);
    }
    pv->pv_vp = vp;
    error = lv_readlvmrec(pv);

    if (error != ESUCCESS) {
	VOP_CLOSE(vp, FREAD|FWRITE, NOCRED, status);
	KFREE(pv->pv_lvmrec, DEV_BSIZE);
	vrele(vp);
	pv->pv_vp = NULL;
    }
    return(error);
}

int
lv_readlvmrec(pv)
struct pvol *pv;
{
	register int error;
	lv_lvmrec_t	*lvmrec = NULL;	/* The LVM record from the PV.	    */
	register struct buf *bp;
	register struct vnode *vp;

	/*
	 * Allocate an LVM record.
	 */
	if ((lvmrec = (lv_lvmrec_t *)kalloc(DEV_BSIZE)) == NULL) {
		error = ENOMEM;
		return (error);
	}
	bp = &(VG_LVOL0(pv->pv_vg)->lv_rawbuf);
	BUF_LOCK(bp);
	vp = pv->pv_vp;

	bp->b_flags = B_BUSY|B_READ;
	bp->b_blkno = PVRA_LVM_REC_SN1;
	bp->b_bcount = DEV_BSIZE; /* Should check the label if it exists... */
	bp->b_un.b_addr = (caddr_t)lvmrec;
	bp->b_vp = NULL;
	bp->b_dev = vp->v_rdev;

	if (error = lv_bufio(bp)) {
		/* Log this problem...... */

		bp->b_flags = B_READ;
		bp->b_blkno = PVRA_LVM_REC_SN2;
		if (error = lv_bufio(bp)) {
			/* Log this problem...... */
			/* Maybe even set error number */
			goto out;
		}
	}
out:
	if (error) {
		KFREE(lvmrec, DEV_BSIZE);
	} else {
		pv->pv_lvmrec = lvmrec;
		pv->pv_vgra_psn = lvmrec->vgra_psn;
		pv->pv_data_psn = lvmrec->data_psn;
	}
	BUF_UNLOCK(bp);
	return(error);
}

int
lv_bufio(bp)
struct buf *bp;
{
extern int lv_minphys();
int error = 0, done, tdone;
int dev, count, icount;
int (*strat)();
int (*mincnt)();
caddr_t addr;
daddr_t blkno;

	icount = count = bp->b_bcount;
	addr = bp->b_un.b_addr;
	blkno = bp->b_blkno;
	tdone = 0;
	dev = bp->b_dev;

	strat = bdevsw[major(dev)].d_strategy;
	mincnt = lv_minphys;

	while ((count > 0) && !error) {
		bp->b_flags &= ~B_ERROR;
		event_clear(&bp->b_iocomplete);
		bp->b_bcount = count;

		(*mincnt)(bp);
		(*strat)(bp);

		error = biowait(bp);

		done = bp->b_bcount - bp->b_resid;
		count -= done;
		tdone += done;
		bp->b_un.b_addr += done;
		bp->b_blkno += btodb(done);
	}
	bp->b_bcount = icount;
	bp->b_resid = icount - tdone;
	bp->b_un.b_addr = addr;
	bp->b_blkno = blkno;
	return(error);
}

/*
 * NAME:	lv_writelvmrec
 *
 * FUNCTION:
 *	Write an LVM Record..  The block device containing the physical
 * volume is written.
 *
 * PARAMETERS:
 *	pvol - The physical volume to write the LVM Record to.
 *
 * NOTES:
 *
 * RETURN:
 *	
 *
 */
int
lv_writelvmrec(pv)
struct pvol *pv;
{
	register int error;
	lv_lvmrec_t	*lvmrec = NULL;	/* The LVM record from the PV.	    */
	register struct buf *bp;
	register struct vnode *vp;

	if ((lvmrec = pv->pv_lvmrec) == NULL) {
		error = ENOMEM;
		return (error);
	}

	bp = &(VG_LVOL0(pv->pv_vg)->lv_rawbuf);
	BUF_LOCK(bp);
	vp = pv->pv_vp;

	bp->b_flags = B_BUSY|B_WRITE;
	bp->b_blkno = PVRA_LVM_REC_SN1;
	bp->b_bcount = DEV_BSIZE; /* Should check the label if it exists... */
	bp->b_un.b_addr = (caddr_t)lvmrec;
	bp->b_vp = NULL;
	bp->b_dev = vp->v_rdev;

	if (error = lv_bufio(bp)) {
		/* Log this problem...... */
	}

	bp->b_flags = B_WRITE;
	bp->b_blkno = PVRA_LVM_REC_SN2;
	if (error = lv_bufio(bp)) {
		/* Log this problem...... */
	}
out:
	BUF_UNLOCK(bp);
	return(ESUCCESS);
}

/*
 * NAME:	lv_closepv
 *
 * FUNCTION:
 *	Close a physical volume.  The block device containing the physical
 * volume is closed and the data structures are freed.
 *
 * PARAMETERS:
 *	pvol - The physical volume to close.
 *
 * NOTES:
 *
 * RETURN:
 *	An errno.
 *
 */
int
lv_closepv(pv)
struct pvol *pv;
{
	int error;

	KFREE(pv->pv_lvmrec, DEV_BSIZE);
	KFREE(pv->pv_bbdir, pv->bbdirsize); pv->bbdirsize = 0;
	KFREE(pv->freelist, pv->freelistsize); pv->freelistsize = 0;
	if (pv->pv_vp) {
		VOP_CLOSE(pv->pv_vp, FREAD|FWRITE, NOCRED, error);
		vrele(pv->pv_vp);
		pv->pv_vp = NULL;
	}
	return(ESUCCESS);
}


/*
 * NAME:	lv_vgaddpv
 *
 * FUNCTION:
 *	Add a pvol to a volume group pvol array.
 */
int
lv_vgaddpv(vg, pv, pvnum)
struct volgrp *vg;		/* The volume group to install into.	*/
struct pvol *pv;		/* The physical volume to install.	*/
int pvnum;			/* Where to install it			*/
{
    int size_pvols;
    struct pvol **pvols = NULL, **pvp = NULL;
    struct extent extent;
    struct lvol *lv;

    if (pvnum < 0) {
	/* Look for an empty pvol entry in the volume group's array */
	for (pvnum = 0, pvp = vg->pvols;
			pvnum < vg->size_pvols;
			pvnum++, pvp++)
		if (*pvp == NULL) break;
    } else {
	if (pvnum < vg->size_pvols) {
		pvp = &(vg->pvols[pvnum]);
		if (*pvp != NULL) return(EBUSY);
	}
    }
    if (pvnum >= vg->size_pvols) {
	/* If free slot was not found, we need a bigger array */

	size_pvols = ROUNDUP(pvnum+1,PVOLSALLOC);
	pvols = (struct pvol **)kalloc(sizeof(struct pvol *) * size_pvols);
 	if (pvols == NULL) {
	    return(ENOMEM);
	}
	/* copy the old array to the new one */
	bcopy((caddr_t)vg->pvols, (caddr_t)pvols,
		sizeof(struct pvol *) * vg->size_pvols);
	bzero((caddr_t)(pvols + vg->size_pvols),
		(size_pvols - vg->size_pvols) * sizeof(struct pvol *));

	/* Free the old array */
	KFREE(vg->pvols, sizeof(struct pvol *) * vg->size_pvols);

	vg->size_pvols = size_pvols;
	vg->pvols = pvols;
	pvp = &vg->pvols[pvnum];
    }
    *pvp = pv;
    vg->num_pvols++;

    lv_pvol_init(pv, pvnum);

    /*
     * Add the PV's VGRA to lvol[0]'s extent map.
     */
    extent.e_pvnum = pvnum;
    extent.e_pxnum = 0;
    extent.e_state = 0;
    lv = VG_LVOL0(vg);
    return(lv_addextent(lv, &extent, pvnum, PRIMMIRROR));

    return (ESUCCESS);
}

/*
 * NAME:	lv_vgsubpv
 *
 * FUNCTION:	Subtract a pvol from the volume group data structures.
 *		(Inverse off lv_vgaddpv)
 */
lv_vgsubpv(vg, pv)
struct volgrp *vg;
struct pvol *pv;
{
	struct extent *ext;
	struct lvol *lv;
	int pvnum;

	pvnum = pv->pv_num;
	lv = VG_LVOL0(vg);

	/* Remove it from lvol[0] */
	ext = EXTENT(lv,pvnum,PRIMMIRROR);
	ext->e_pvnum = PX_NOPV;
	ext->e_pxnum = 0;
	ext->e_state = 0;

	vg->pvols[pvnum] = NULL;

	vg->num_pvols--;

	return;
}

lv_pvol_init(pv, pvnum)
struct pvol *pv;
int pvnum;
{

    pv->pv_num = pvnum;
    LV_QUEUE_INIT(&(pv->pv_cache_wait));
    LV_QUEUE_INIT(&(pv->pv_ready_Q));
    LOCK_INTERRUPT_INIT(&(pv->pv_intlock));
    BUF_LOCKINIT(&(pv->pv_buf));

    lv_pvol_linit(pv);

    return;
}

lv_pvol_linit(pv)
struct pvol *pv;
{
    lv_lvmrec_t *lvmrec;

    if (lvmrec = pv->pv_lvmrec) {

	/* Fill in the appropriate parts of the pvol structure */
	pv->pv_datapsn = lvmrec->data_psn;
	pv->pv_pxspace = lvmrec->pxspace;

	pv->pv_mwc_loc[0] = lvmrec->mcr_psn1 - lvmrec->vgra_psn;
	pv->pv_mwc_loc[1] = lvmrec->mcr_psn2 - lvmrec->vgra_psn;
	pv->pv_mwc_flags &= ~(PV_CACHE_QUEUED|PV_CACHE_TOGGLE);

	/* These do not go through Lvol 0 so the PSN's are straight forward. */
	pv->pv_sa_psn[0] = lvmrec->vgda_psn1 + lvmrec->vgda_len;
	pv->pv_sa_psn[1] = lvmrec->vgda_psn2 + lvmrec->vgda_len;
	pv->pv_sa_seqnum[0] = -1;
	pv->pv_sa_seqnum[1] = -1;
    }
    return;
}

static struct timeval lv_lasttime;
struct lv_crit lv_lasttime_intlock;

/*
 * lv_microtime: provide a _strictly_ monotonic increasing timestamp.
 * The main reason this routine exists is the wide number of
 * incorrect versions of microtime(). We also need to be able to 
 * fake out the "most recent" time. (not yet implemented.)
 */
extern void
lv_microtime(tvp)
struct timeval *tvp;
{
	/*
	 * Fetch the allegedly unique, increasing time from 
	 * the system.
	 */
	microtime(tvp);

	LOCK_INTERRUPT(&lv_lasttime_intlock);

	if (!timercmp(tvp,&lv_lasttime,>)) {
		/*
		 * Time is not strictly greater than the last time
		 * returned. Fudge It.
		 */
		tvp->tv_sec = lv_lasttime.tv_sec;
		if ((tvp->tv_usec = lv_lasttime.tv_usec + 1) > 1000000) {
			tvp->tv_sec++;
			tvp->tv_usec -= 1000000;
		}
	}
	ASSERT(timercmp(tvp,&lv_lasttime,>));
	lv_lasttime = *tvp;

	UNLOCK_INTERRUPT(&lv_lasttime_intlock);

	return;
}
