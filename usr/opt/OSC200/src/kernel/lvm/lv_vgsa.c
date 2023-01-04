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
static char	*sccsid = "@(#)$RCSfile: lv_vgsa.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/01/08 18:01:21 $";
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
 * This file is derived from the file hd_vgsa.c
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lvm/lvmd.h>

#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/vnode.h>
#include <sys/specdev.h>

/* Forward declarations for internal functions. */
extern void lv_sa_iodone();
extern void lv_sa_continue();
extern void lv_sa_write();
extern void lv_sa_hback();
extern void lv_sa_done();
extern void lv_initvgsaptr();
extern void lv_sa_update();
extern void lv_sa_qrmcheck();

/*
 * NAME:         lv_sa_createvgsa
 *
 * FUNCTION:
 *	Create a VGSA.  This function is only called during a LVM_CREATEVG
 * operation.
 *
 * RETURN VALUE:
 *		ESUCCESS - Everything succeeded.
 *		ENOMEM   - Could not allocate enough memory.
 */
int
lv_sa_createvgsa(vg)
struct	volgrp	*vg;			/* The VG to create a VGSA for.     */
{
struct	vgsa	*sap;
	int	maxpvs;
	int	error;
	int	i;

	if ((error = lv_sa_initvgsa(vg)) != ESUCCESS)
		return(error);

	/* Initialize the VGSA header. */
	SA_MAXPVS(vg) = vg->vg_maxpvs;
	SA_MAXPXS(vg) = vg->vg_maxpxs;

	/* Initialize the pointers into the VGSA */
	sap = &(vg->vg_sa_ptr);
	lv_initvgsaptr(sap, vg->vgsa_len);

	/* Initialize the VGSA missing PV array. */
	maxpvs = (vg->vg_maxpvs + (sizeof(int)*NBBY) - 1)
				& ~((sizeof(int)*NBBY) - 1);
	for (i = 0; i < (maxpvs/(sizeof(int)*NBBY)); i++)
		sap->sa_pvmiss[i] = ~0;

	return(ESUCCESS);
}

/*
 * NAME:	lv_sa_initvgsa
 *
 * FUNCTION:
 *	Allocate and initialize memory resources for the volume group status
 * area.  This function is used by both lv_sa_createvgsa and lv_sa_readvgsa.
 *
 * RETURN VALUE:
 *		ESUCCESS - Operation succeeded.
 *		ENOMEM   - Could not allocate enough memory.
 */
int
lv_sa_initvgsa(vg)
struct	volgrp	*vg;
{
struct	vgsa	*sap;

	sap = &(vg->vg_sa_ptr);

	/* Allocate space for the VGSA. */
	sap->sa_header = (struct vgsaheader *)kalloc(vg->vgsa_len * DEV_BSIZE);
	if (sap->sa_header == NULL) {
		return(ENOMEM);
	}
	bzero(sap->sa_header, vg->vgsa_len * DEV_BSIZE);

	/* Initialize the VGSA reserved physical buffer. */
	bzero(&(vg->vg_sa_pbuf), sizeof(struct pbuf));
#if MACH_LDEBUG
	/*
	 * This buf will never be locked or unlocked, but some
	 * overly-assertive disk drivers (like the multimax)
	 * want to make claims about the state of buffers
	 * that they are operating on.
	 */
	BUF_LOCKINIT(&(vg->vg_sa_pbuf.pb));
	BUF_LOCK(&(vg->vg_sa_pbuf.pb));
	BUF_GIVE_AWAY(&(vg->vg_sa_pbuf.pb));
#endif
	vg->vg_sa_pbuf.pb_lbuf	= (struct buf *)NULL;

	/* Initialize the rest of the status area data. */
	LOCK_INTERRUPT_INIT(&(vg->vg_sa_intlock));
	vg->vg_sa_state = 0;
	LV_QUEUE_INIT(&vg->vg_sa_hold);
	LV_QUEUE_INIT(&vg->vg_sa_active);
	vg->vg_sa_wheel = 0;
	vg->vg_sa_seqnum = 0;

	return(ESUCCESS);
}

/*
 * NAME:         lv_sa_readvgsa
 *
 * FUNCTION:
 *	Read the volume group status area from a valid physical volume.
 *
 * RETURN VALUE:
 *		ESUCCESS - Operation succeeded.
 *		ENODEV   - The volume group is closing due to lack of quorum.
 *		ENOMEM   - Could not allocate enough memory.
 */
int
lv_sa_readvgsa(vg)
struct	volgrp	*vg;			/* The volume group with missing PVs */
{
struct	pvol	*pv;
struct	vgsa	*sap;
struct	buf	*lb;
	int	wheel;
	int	error;

	/* Allocate space for the VGSA, and initialize various data. */
	if ((error = lv_sa_initvgsa(vg)) != ESUCCESS)
		return(error);

	lb = &(VG_LVOL0(vg)->lv_rawbuf);
	BUF_LOCK(lb);

	lb->b_proc	= NULL;
	lb->b_vp	= NULL;
	lb->b_dev	= makedev(vg->major_num,0);
	lb->av_forw	= NULL;

	/* Initialize the buf and read the VGSA. */
	error = ENODEV;
	while (error && ((wheel = lv_sa_getnewest(vg)) != -1)) {

		pv = vg->pvols[wheel >> 1];

		lb->b_flags	= B_READ;
		lb->b_bcount	= vg->vgsa_len << DEV_BSHIFT;
		lb->b_un.b_addr	= (char *)vg->vg_sa_ptr.sa_header;
		lb->b_blkno	= SA_LSN(vg, wheel);

		/* Start the I/O and synchronize with its completion. */
		if ((error = lv_bufio(lb)) != ESUCCESS) {
			timerclear(&pv->pv_vgsats[wheel&1]);
		}
	}

	BUF_UNLOCK(lb);

	if (error)
		return(error);

	vg->vg_sa_wheel = wheel;

ASSERT(!timercmp(&pv->pv_vgsats[wheel&1],&SA_H_TIMESTAMP(vg),!=));

	/* Set the maxpvs and maxpxs in the VG from the status area. */
	vg->vg_maxpvs = SA_MAXPVS(vg);
	vg->vg_maxpxs = SA_MAXPXS(vg);

	/* Initialize the pointers into the VGSA */
	sap = &(vg->vg_sa_ptr);
	lv_initvgsaptr(sap, vg->vgsa_len);

ASSERT(!timercmp(&pv->pv_vgsats[wheel&1],&SA_T_TIMESTAMP(vg),!=));

	return(ESUCCESS);
}

/*
 * NAME:	lv_initvgsaptr
 *
 * FUNCTION:	Initialize the struct vgsa in the volgrp.
 *
 */
void
lv_initvgsaptr(sap, vgsalen)
struct	vgsa	*sap;
uint_t vgsalen;
{
struct vgsaheader *vgsahdr;
int	sizepvmiss;

	vgsahdr = sap->sa_header;

	sap->sa_pvmiss = (uint_t *)(vgsahdr + 1);	/* Structure math... */

	/* Round up maxpvs to whole number or words for PV missing array */
	sizepvmiss = ((vgsahdr->sa_maxpvs + (NBPW*NBBY) - 1) 
				& ~((NBPW*NBBY)-1))/(NBPW*NBBY);
	sap->sa_pxstale = (uchar_t *)&sap->sa_pvmiss[sizepvmiss];
	sap->sa_trailer = (struct vgsatrailer *)
		((char *)vgsahdr + (vgsalen * DEV_BSIZE) -
		 sizeof(struct vgsatrailer));

	return;
}

/*
 * NAME:	lv_sa_getnewest
 *
 * FUNCTION:	Determine the newest VGSA in the volume group.
 *
 * RETURNS:	Wheel index of newest VGSA found in volume group, or -1 for
 *		no valid VGSA in group.
 */
int
lv_sa_getnewest(vg)
struct	volgrp	*vg;			/* The volume group with missing PVs */
{
struct	timeval	latest;
	int	wheel;
	int	pvnum;
struct	pvol	*pv;

	wheel = -1;
	timerclear(&latest);

	/* Determine the wheel index with the latest VGSA. */
	for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
		if ((pv = vg->pvols[pvnum]) == NULL) continue;

		if (timercmp(&pv->pv_vgsats[0], &pv->pv_vgsats[1], >)) {
			if (timercmp(&pv->pv_vgsats[0], &latest, >)) {
				latest = pv->pv_vgsats[0];
				wheel = pvnum << 1;
			}
		} else {
			if (timercmp(&pv->pv_vgsats[1], &latest, >)) {
				latest = pv->pv_vgsats[1];
				wheel = (pvnum << 1) | 1;
			}
		}
	}
	return(wheel);
}

/*
 * NAME:	lv_sa_setstale
 *
 * FUNCTION:
 *	Set the specified physical extent as stale in the in-memory
 *	copy of the VGSA.
 *
 * NOTE:
 *	This function can only be called during mirror recovery operations.
 *	This is because we must be assured that the wheel is not running.
 */

lv_sa_setstale(vg, pvnum, pxnum)
struct	volgrp	*vg;			/* The volume group with missing PVs */
	int	pvnum;
	int	pxnum;
{
	ASSERT((vg->vg_sa_state & SA_ACTIVE) == 0);
	SA_SET_PXSTALE(vg, pvnum, pxnum);
}

/*
 * NAME:         lv_sa_missing
 *
 * FUNCTION:
 *	Mark the physical volumes as either present or missing.  This does not
 * take into regard the past state of the physical volumes as the whole VGSA
 * will be overwritten anyway.
 *
 * NOTE:
 *	This code is not re-entrant.  It is assumed that all calls to this
 * function are from the top-half of the driver, and that the volume group
 * lock must have been taken to get here.
 *
 * RETURN VALUE:
 *		ESUCCESS - Operation succeeded.
 *		ENODEV   - The volume group is closing due to lack of quorum.
 */

lv_sa_missing(vg)
struct	volgrp	*vg;			/* The volume group with missing PVs */
{
	struct pvol *pv;
	int	     pvnum;

	ASSERT((vg->vg_sa_state & SA_ACTIVE) == 0);

	for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
		if ((pv = vg->pvols[pvnum]) == NULL) continue;

		if (pv->pv_flags & LVM_NOTATTACHED) {
			pv->pv_flags |= LVM_PVMISSING;
			SA_SET_PVMISSING(vg, pvnum);
		} else {
			pv->pv_flags &= ~LVM_PVMISSING;
			SA_CLR_PVMISSING(vg, pvnum);
		}
	}

	/* Set the stale bits in the VGSA and check quorum. */
	return(lv_sa_config(vg, CNFG_SYNCWRITE, NULL));
}

/*
 * NAME:         lv_sa_config
 *
 * FUNCTION:
 *	Process a new Status Area configuration request.  This is similar to
 * lv_sa_start except that it is possible to have multiple changes to the
 * status area, and there is no physical buffer supplied.
 *
 *	This function is synchronous.  This function will not return until
 * the newly modified status area has been written to all of the physical
 * volumes.
 *
 * NOTE:
 *	This code is not re-entrant.  It is assumed that all calls to this
 * function are from the top-half of the driver, and that the volume group
 * lock must have been taken to get here.
 *
 * RETURN VALUE:
 *		ESUCCESS - Operation succeeded.
 *		ENODEV   - The volume group is closing due to lack of quorum.
 */
int
lv_sa_config(vg, type, arg)
struct	volgrp	*vg;			/* The volume group being updated.  */
int	type;				/* The type of the request.	    */
void	*arg;				/* Argument array.		    */
{
	/* If we have lost quorum, simply return an error. */
	if (vg->vg_flags & VG_LOST_QUORUM)
		return(ENODEV);

	LOCK_INTERRUPT(&(vg->vg_sa_intlock));

	ASSERT(vg->vg_sa_conf_op == CNFG_NOP);

	/* Mark the VGSA as indicated. */
	switch (type) {
	case CNFG_SYNCWRITE:
		ASSERT((vg->vg_sa_state & SA_ACTIVE) == 0);
		/* FALLTHROUGH */
	case CNFG_PVMISSING:
	case CNFG_INSTALLPV:
	case CNFG_FRESHPX:
	case CNFG_STALEPX:
		vg->vg_sa_conf_op = type;
		vg->vg_sa_conf_arg = arg;
		break;
	default:
		panic("lv_sa_config: unknown configuration operation\n");
	}
	if ((vg->vg_sa_state & SA_ACTIVE) == 0) {
		vg->vg_sa_state |= SA_ACTIVE;
		UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));

		lv_sa_continue(vg);

		LOCK_INTERRUPT(&(vg->vg_sa_intlock));
	}
	/* Wait for the wheel completion. */

	while (vg->vg_sa_conf_op != CNFG_NOP) {

		assert_wait((vm_offset_t)&vg->vg_sa_conf_op, FALSE);
		UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));

		thread_block();
	
		LOCK_INTERRUPT(&(vg->vg_sa_intlock));
	}
	UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));

	return(ESUCCESS);
}

/*
 * NAME:         lv_sa_start
 *
 * FUNCTION:
 *	Process a new Status Area request.  Put the request on the hold list
 * (sa_hld_lst).  Start the update wheel if it is not already rolling.  The
 * specified physical request is suspended until the status area is updated.
 *
 * RETURN VALUE:
 * 	None. This routine indicates success or failure through the
 * pbuf pb_vgsa_failed flag when the reqest is released to the
 * (*pb->pb_sched_done)() function.
 */
void
lv_sa_start(vg, pb, type)
struct	volgrp	*vg;			/* The VG being updated.	*/
struct	pbuf	*pb;			/* Physical buffer to suspend.	*/
	int	type;			/* Type of the SA request.	*/
{
	int	s;

	/* Place the request on the end of the hold list. */
	pb->pb_type = type;
	LOCK_INTERRUPT(&(vg->vg_sa_intlock));
	LV_QUEUE_APPEND(&vg->vg_sa_hold, &(pb->pb));

	/* If the wheel is not rolling, start it. */
	if ((vg->vg_sa_state & SA_ACTIVE) == 0) {
		vg->vg_sa_state |= SA_ACTIVE;
		UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));
		lv_sa_continue(vg);
	} else {
		UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));
	}
	return;
}

/*
 * NAME:         lv_sa_continue
 *
 * FUNCTION:
 *	Start the status area wheel.  The only thing that stops the wheel once
 * it is rolling is the sa_seqnum variables.  When the last write sa_seqnum
 * matches the next one, we are complete.
 *
 *	If the volume group is colsing due to a loss of quorum then all active
 * requests are returned with errors.  This will result in an error being
 * retunred with the original request.  Because of the loss of quorum we can
 * not guarantee the VGSA was updated with the correct information.  Any user
 * data will be recovered by the MWC cache.
 *
 * NOTES:
 *	This function is not reentrant.  It is assumed that only one thread
 * can be running in this function at a time.  This is ensured by the
 * SA_ACTIVE flag in the sa_flags field of the sa structure.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_continue(vg)
struct	volgrp	*vg;			/* The volume group pointer.	    */
{
struct	lvol	*lv;			/* The logical volume of the request*/
struct	pbuf	*pb;			/* The pbuf being processed.	    */
struct	pbuf	*active;		/* A pbuf on the active list.	    */
struct	pbuf	*new_req = NULL;	/* First new pbuf in active list.   */
	uint_t	mirmask;		/* Mirror mask.			    */
	ushort_t lxnum;			/* Logical extent number.	    */
	ushort_t pxnum;			/* Physical extent number.	    */
	ushort_t pvnum;			/* Physical volume number.	    */
	int	mir;			/* Mirror number.		    */
	uint_t	c_whl_indx;		/* The current wheel index.	    */
	uint_t	n_whl_indx;		/* The new wheel index.		    */
struct	extent	*ext;
struct	lv_queue done_req;

	ASSERT(vg->vg_sa_state & SA_ACTIVE);

	/* Process the hold list moving requests to the active list. */
	LOCK_INTERRUPT(&(vg->vg_sa_intlock));
	LV_QUEUE_INIT(&done_req);

	lv_sa_qrmcheck(vg);

	while (vg->vg_sa_hold.lv_head != NULL) {

		LV_QUEUE_FETCH(&vg->vg_sa_hold, (struct buf *)pb);

		/*
		 * Scan the active list for any request that is doing the same
		 * operation.  If one is found, then queue this request after
		 * the existing one and mark it to complete at the same time.
		 */
		active = (struct pbuf *)vg->vg_sa_active.lv_head;
		while (active && pb) {

			/* If not the same type, then continue. */
			if (active->pb_type != pb->pb_type) {
				active = (struct pbuf *)active->pb.av_forw;
				continue;
			}

			/* If it is the same type, check the description. */
			switch (active->pb_type) {
			case SA_STALEPX:
				lv = active->pb_lv;
				if ((lv != pb->pb_lv) ||
				    (BLK2EXT(vg, active->pb.b_blkno) !=
				     BLK2EXT(vg, pb->pb.b_blkno)))
					break;

				mirmask = (~active->pb_mirbad & pb->pb_mirbad);
				if (mirmask == 0) {
					lv_sa_hback(active, pb);
					pb = NULL;
				}
				break;

			case SA_FRESHPX:
				break;
			default:
				panic("lv_sa_continue operation type");
				break;
			}

			/* If the buffer still around? */
			if (pb)
				active = (struct pbuf *)active->pb.av_forw;
		}

		/* If active is NULL, then the pbuf goes on the end. */
		if (active == NULL) {
			LV_QUEUE_APPEND(&vg->vg_sa_active, &(pb->pb));

			/* Save the start of the new requests. */
			if (new_req == NULL)
				new_req = pb;

			/* Update the status area to reflect the request. */
			switch (pb->pb_type) {
			case SA_STALEPX:
				mirmask = pb->pb_mirbad;
				lv = pb->pb_lv;
				lxnum = BLK2EXT(vg, pb->pb_lbuf->b_blkno);
				while (mirmask) {
					mir = FIRST_MASK(mirmask);
					mirmask &= ~(MIRROR_MASK(mir));

					ext = EXTENT(lv, lxnum, mir);
					pxnum = ext->e_pxnum;
					pvnum = ext->e_pvnum;
					SA_SET_PXSTALE(vg, pvnum, pxnum);
				}
				break;

			case SA_FRESHPX:
				mirmask = pb->pb_mirdone;
				lv = pb->pb_lv;
				lxnum = BLK2EXT(vg, pb->pb_lbuf->b_blkno);
				while (mirmask) {
					mir = FIRST_MASK(mirmask);
					mirmask &= ~(MIRROR_MASK(mir));

					ext = EXTENT(lv, lxnum, mir);
					pxnum = ext->e_pxnum;
					pvnum = ext->e_pvnum;
					SA_CLR_PXSTALE(vg, pvnum, pxnum);
				}
				break;

			default:
				panic("lv_sa_continue: unknown type");
			}
		}
	}
	if (vg->vg_sa_conf_op > CNFG_NOP) {
		struct pvol *pv;
		struct sa_px_info *px_info;
		switch (vg->vg_sa_conf_op) {
		case CNFG_PVMISSING:
			pv = (struct pvol *)vg->vg_sa_conf_arg;
			pv->pv_flags |= LVM_PVMISSING;
			SA_SET_PVMISSING(vg, pv->pv_num);
			break;

		case CNFG_INSTALLPV:
			pv = (struct pvol *)vg->vg_sa_conf_arg;
			SA_CLR_PVMISSING(vg, pv->pv_num);
			break;

		case CNFG_FRESHPX:
			px_info = (struct sa_px_info *)vg->vg_sa_conf_arg;
			while (px_info->pxi_pvnum != PX_NOPV) {
				SA_CLR_PXSTALE(vg, px_info->pxi_pvnum,
					       px_info->pxi_pxnum);
				px_info++;
			}
			break;

		case CNFG_STALEPX:
			px_info = (struct sa_px_info *)vg->vg_sa_conf_arg;
			while (px_info->pxi_pvnum != PX_NOPV) {
				SA_SET_PXSTALE(vg, px_info->pxi_pvnum,
					       px_info->pxi_pxnum);
				px_info++;
			}
			break;

		case CNFG_SYNCWRITE:
			break;

		default:
			panic("lv_sa_continue: unknown config op");
		}
	}

	/* Update the timestamps and sequence number in the VGSA if needed. */
	if (new_req || (vg->vg_sa_conf_op > CNFG_NOP)) {
		lv_sa_update(vg);
	}
	/*
	 * If initiating a configuration operation, set the completion
	 * sequence number.
	 */
	if (vg->vg_sa_conf_op > CNFG_NOP) {
		vg->vg_sa_conf_seq = vg->vg_sa_seqnum;
		vg->vg_sa_conf_op = CNFG_INPROGRESS;
	}
	/*
	 * If there are any new requests on the active list, set their
	 * sequence number to the current sequence number.
	 */
	while (new_req) {
		new_req->pb_seqnum = vg->vg_sa_seqnum;
		new_req = (struct pbuf *)new_req->pb.av_forw;
	}

	ASSERT(vg->vg_sa_hold.lv_head == NULL);
	/*
	 * The hold list has been processed.  At this point, we have to
	 * recheck quorum as we may have lost it above.  If we did lose
	 * quorum, then we have to continue all of the requests on the active
	 * list with an error.
	 */
	if (vg->vg_flags & VG_LOST_QUORUM) {
		while (vg->vg_sa_active.lv_head) {
			LV_QUEUE_FETCH(&vg->vg_sa_active, (struct buf *)active);
			LV_QUEUE_APPEND(&done_req, (struct buf *)active);
			lv_sa_done(active, ENXIO);
		}
		vg->vg_sa_state &= ~SA_ACTIVE;
		UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));
		while (LV_QUEUE_FETCH(&done_req, (struct buf *)active),
				active) {
			lv_sa_done(active, ENXIO);
		}
		return;
	}

	/*
	 * New see if any request can be removed from the active list.  This
	 * is possible when the NEXT VGSA location has a sequence number
	 * higher than the request's sequence number.
	 */
	c_whl_indx = vg->vg_sa_wheel;
	n_whl_indx = lv_sa_whladv(vg, c_whl_indx);
	while (active = (struct pbuf *)vg->vg_sa_active.lv_head) {

		/* Check if the first pbuf on the list is done. */
		if (active->pb_seqnum <= SA_SEQNUM(vg, n_whl_indx)) {
			LV_QUEUE_FETCH(&vg->vg_sa_active, (struct buf *)active);
			LV_QUEUE_APPEND(&done_req, (struct buf *)active);
		}

		/* The list is sorted.  If the first is not done, none are. */
		else
			break;
	}
	/*
	 * Check for completion of configuration wheel turn.
	 */
	if (vg->vg_sa_conf_op == CNFG_INPROGRESS) {
		if (vg->vg_sa_conf_seq <= SA_SEQNUM(vg, n_whl_indx)) {
			vg->vg_sa_conf_op = CNFG_NOP;
			thread_wakeup((vm_offset_t)&(vg->vg_sa_conf_op));
		}
	} else {
		ASSERT(vg->vg_sa_conf_op == CNFG_NOP);
	}

	/* Save the next wheel index. */
	vg->vg_sa_wheel = n_whl_indx;

	/*
	 * If the current sequence number has not made it all the way around,
	 * then keep on writting.
	 */
	if (vg->vg_sa_seqnum > SA_SEQNUM(vg, n_whl_indx)) {
		if (thread_call(&lv_threadq,lv_sa_write,vg) == FALSE) {
			panic("lv_sa_continue thread_call");
		}
	} else {
		/* Wheel if finished. Mark it stopped */
		ASSERT(vg->vg_sa_active.lv_head == NULL);
		vg->vg_sa_state &= ~SA_ACTIVE;
	}
	UNLOCK_INTERRUPT(&(vg->vg_sa_intlock));
	while (LV_QUEUE_FETCH(&done_req, (struct buf *)active), active) {
		lv_sa_done(active, ESUCCESS);
	}
	return;
}

/*
 * NAME:	lv_sa_write
 *
 * FUNCTION:
 *	Write the status area.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_write(vg)
struct	volgrp	*vg;			/* Volume group to write to.	*/
{
struct	pbuf	*pb;			/* Buffer used to do the write. */

	/* Initialize the status area physical buffer. */
	pb = &(vg->vg_sa_pbuf);

	pb->pb_pvol	 = vg->pvols[vg->vg_sa_wheel >> 1];
	pb->pb.b_flags	 = B_BUSY|B_WRITE;
	pb->pb_start	 = SA_PSN(vg, vg->vg_sa_wheel);
	pb->pb_addr	 = (char *)vg->vg_sa_ptr.sa_header;
	pb->pb.b_bcount  = vg->vgsa_len << DEV_BSHIFT;
	pb->pb_sched	 = lv_sa_iodone;
	pb->pb_startaddr = pb->pb_addr;
	pb->pb_endaddr	 = pb->pb_startaddr + pb->pb.b_bcount;
	pb->pb_options	 = 0;

	lv_begin(pb);
}

/*
 * NAME:	lv_sa_iodone
 *
 * FUNCTION:
 *	Return point for VGSA write operation.  Process any error on the write
 * and call lv_sa_continue() to start the next write.
 *
 *	If the write failed, then the physical volume is marked as missing.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_iodone(bp)
struct buf *bp;			/* The physical buffer. */
{
struct pvol *pv;
struct volgrp *vg;

	/* If there was an error, mark the PV missing and continue the write */
	pv = ((struct pbuf *)bp)->pb_pvol;
	vg = pv->pv_vg;

	ASSERT(vg->vg_sa_state & SA_ACTIVE);

	if (bp->b_flags & B_ERROR) {
		pv->pv_flags |= LVM_PVMISSING;
		SA_SET_PVMISSING(vg, pv->pv_num);
		lv_sa_update(vg);
	} else {
		SA_SEQNUM(vg, vg->vg_sa_wheel) = vg->vg_sa_seqnum;
		SA_PV_TIMESTAMP(vg, vg->vg_sa_wheel) = SA_H_TIMESTAMP(vg);
	}

	lv_sa_continue(vg);
}

/*
 * NAME:	lv_sa_hback
 *
 * FUNCTION:
 *	This function is used to link physical buffers with identical status
 * area update requests together.  This allows subsequent identical requests
 * to be continued at the same time.  The linking is done using the av_back
 * pointers.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_hback(head, new)
struct	pbuf	*head;			/* The head of the list.	    */
struct	pbuf	*new;			/* The new pbuf to add to the list. */
{
	/* Put the new one on the head of the back list. */
	new->pb.av_back = head->pb.av_back;
	head->pb.av_back = &new->pb;
}

/*
 * NAME:	lv_sa_done
 *
 * FUNCTION:
 *	Continue the specified pbuf and any pbufs hanging off of its av_back
 * list.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_done(head, errno)
struct	pbuf	*head;			/* The head of the list to continue. */
	int	errno;			/* The error number. 		     */
{
struct	pbuf	*next;			/* The next buffer on the list.	     */

	/* Walk the list, continuing all of the buffers. */
	for (; head != NULL; head = next) {

		/* If there is an error, turn it on in the buffer. */
		if (errno != ESUCCESS)
			head->pb_vgsa_failed = 1;

		/* Remove the buffer from the list and continue it. */
		next = (struct pbuf *)head->pb.av_back;
		head->pb.av_back = NULL;
		head->pb.av_forw = NULL;
		LV_SCHED_DONE(head);
	}
}

/*
 * NAME:	lv_sa_whladv
 *
 * FUNCTION:
 *	Advance the VGSA wheel to the next valid location.  The wheel has two
 * components, the primary/secondary bit and the physical volume index.
 * Together these two components describe which VGSA location is being
 * written.  The wheel will not be advanced to an inactive or missing physical
 * volume.
 *
 * RETURN VALUE:
 *	The next wheel index.
 */
int
lv_sa_whladv(vg, c_whl_indx)
struct volgrp *vg;			/* Status are data.		*/
int c_whl_indx;				/* The current wheel index.	*/
{
struct pvol *pv;

	/* Bump the wheel index. */
	c_whl_indx += 1;

	/* Keep looking until we get to a good physical volume. */
	for (;;) {
		/* If we have gone past the size of the PVOL structure, wrap */
		if ((c_whl_indx >> 1) >= vg->size_pvols)
			c_whl_indx = 0;

		/* If the physical volume is missing, skip to the next one. */
		if (((pv = vg->pvols[c_whl_indx >> 1]) == NULL) ||
		    (pv->pv_flags & LVM_PVMISSING)) {
			if (c_whl_indx & 1)
				c_whl_indx += 1;
			else
				c_whl_indx += 2;
		} else if (SA_PSN(vg, c_whl_indx)) {
			/* The physical volume has a VGSA here, then use it. */
			break;
		} else {
			/* One to the next index. */
			c_whl_indx += 1;
		}
	}

	/* Return the new wheel index. */
	return(c_whl_indx);
}

/*
 * NAME:	lv_sa_update
 *
 * FUNCTION:
 *	Update the in-memory versions of the VGSA timestamps and the sequence
 * number.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_update(vg)
struct volgrp *vg;			/* The status area to update. */
{
	/* Get the current time into the status area header and trailer. */
	lv_microtime(&SA_H_TIMESTAMP(vg));
	SA_T_TIMESTAMP(vg) = SA_H_TIMESTAMP(vg);

	/* Bump the wheel sequence */
	vg->vg_sa_seqnum++;

	return;
}

/*
 * NAME:	lv_sa_qrmcheck
 *
 * FUNCTION:
 *	Check that quorum still exists.  If quorum has been lost, set the
 * closing bit in the volume group.
 *
 * RETURN VALUE:
 *	None.
 */
void
lv_sa_qrmcheck(vg)
struct volgrp *vg;			/* VG being checked for quorum.	    */
{
	struct pvol *pv;
	int active = 0, eligible = 0;
	int pvnum;

	/* Count the number of active pvols in the volume group. */
	for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
		if ((pv = vg->pvols[pvnum]) == NULL) continue;
		if ((pv->pv_flags & LVM_NOVGDA) == 0) {
			eligible++;
			if ((pv->pv_flags & LVM_PVMISSING) == 0) {
				active++;
			}
		}
	}

	/*
	 * More than half the number of eligible pvols must be
	 * accessible for quorum.
	 */
	if (active < ((eligible + 1) >> 1)) {
		vg->vg_flags |= VG_LOST_QUORUM;
	}

	return;
}
