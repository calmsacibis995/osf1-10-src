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
static char	*sccsid = "@(#)$RCSfile: lv_ioctls.c,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1993/01/05 16:18:42 $";
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
 *	lv_ioctls.c - Logical volume manager IOCTL functions.
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
 *  lv_ioctls.c -- I/O control functions for the logical volume manager.
 *
 *	This module contains the I/O control functions for the logical volume
 *	manager.
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
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/user.h>

#define	PV_FLAGS	(LVM_PVNOALLOC|LVM_NOVGDA|LVM_PVRORELOC|LVM_PVDEFINED)
#define VG_FLAGS	(LVM_ACTIVATE_LVS|\
				LVM_ALL_PVS_REQUIRED|\
				LVM_NONMISSING_PVS_REQUIRED)

/* Forward declarations of the scheduling switches. */
extern	struct lv_sched	lv_reserved_ops;
extern	struct lv_sched	lv_sequential_ops;
extern	struct lv_sched	lv_parallel_ops;

/*
 * NAME:	lv_attachpv
 *
 * FUNCTION:
 *	Attach the specified physical volume to the current volume group.  The
 * physcial volume is opened and its LVM record is read.  If the LVM record
 * indicates that the physical volume is a member of the volume group, then
 * the attach will be performed.
 *
 * PARAMETERS:
 *	vg  - The volume group to attach the PV to.
 *	path - pointer to the pathname of the physical volume.
 *
 * RETURN:
 *	EINVAL - Invalid ioctl argument structure.
 *
 */
int
lv_attachpv(vg, path)
struct volgrp *vg;		/* The volume group be ioctled.	*/
char *path;			/* Path of PV to attach.	*/
{
    int error, i;
    struct pvol *pv, *pv1;
    int pvnum;
    lv_lvmrec_t *lvmrec;

    /* Allocate a pvol and clear it */
    if ((pv = NEW(struct pvol)) == NULL)
	return(ENOMEM);

    bzero((caddr_t)pv, sizeof(struct pvol));

    pv->pv_vg = vg;
    /* Open the passed volume and read in the LVM record */
    if ((error = lv_openpv(path, pv)) != ESUCCESS) {
	KFREE(pv, sizeof(struct pvol));
	return(error);
    }

    lock_write(&(vg->vg_lock));

    /* Check to make sure that the volume is a member of this volume group. */
    lvmrec = pv->pv_lvmrec;
    if (zeroID(&vg->vg_id)) {
	/*
	 * This volume group doesn't know it's own name yet.
	 */
	if (zeroID(&lvmrec->vg_id)) {
		/* both vg_id's are null, return ENODEV */
		error = ENODEV;
	} else {
		/* Set volume group's vg_id to physical volume's vg_id; */
		vg->vg_id = lvmrec->vg_id;
	}
    } else {
	if (!equalID(&vg->vg_id, &lvmrec->vg_id)) {
		/* both vg_id's are different, return EXDEV */
		error = EXDEV;
	}
    }
    if (error) {
	lock_done(&(vg->vg_lock));
	lv_closepv(pv);
	KFREE(pv, sizeof(struct pvol));
	return(error);
    }
    pvnum = lvmrec->pv_num;
    /*
     * See if a physical volume structure is defined for this PV.
     * This will occur if the volume group is activated with some
     * PVs missing, and they are later brought on-line.
     */
    if ((pvnum < vg->size_pvols) && ((pv1 = vg->pvols[pvnum]) != NULL)) {
	if (pv1->pv_flags & LVM_NOTATTACHED) {
		/*
		 * This pvol was not previously attached, substitute
		 * the real decription, and free the extra data structures.
		 */
		pv1->pv_lvmrec = pv->pv_lvmrec; pv->pv_lvmrec = NULL;
		pv1->pv_vp = pv->pv_vp; pv->pv_vp = NULL;
		/*
		 * Pretend this pvol is missing so that I/O
		 * won't get queued to it until the SA update
		 * completes.
		 */
		pv1->pv_flags |= LVM_PVMISSING;
		pv1->pv_flags &= ~LVM_NOTATTACHED;

		lv_pvol_linit(pv1);

		if ((error = lv_initdefects(pv1, 0)) == ESUCCESS) {

			lv_readvgdats(vg, pv1);

			lv_sa_config(vg, CNFG_INSTALLPV, pv1);
			pv1->pv_flags &= ~LVM_PVMISSING;

			lock_done(&(vg->vg_lock));
		} else {
			pv1->pv_flags |= LVM_NOTATTACHED;
			lock_done(&(vg->vg_lock));
			lv_closepv(pv1);
		}
		KFREE(pv, sizeof(struct pvol));
		return(error);
	} else {
		lock_done(&(vg->vg_lock));
		lv_closepv(pv);
		KFREE(pv, sizeof(struct pvol));
		return(EEXIST);
	}
    }

    if (vg->num_pvols == 0) {
	/*
	 * This PV becomes the "official" definition of what the volume group
	 * looks like.  All other PVs that are added to this volume group must
	 * conform to this.
	 */
        vg->vgda_len = lvmrec->vgda_len;
        vg->vgsa_len = lvmrec->vgsa_len;
        vg->mcr_len = lvmrec->mcr_len;
        vg->vg_pxsize = lvmrec->pxsize;
	vg->vg_extshift = (vg->vg_pxsize - DEV_BSHIFT);
    }

    /* 
     * Initialize the pvol defect table, updating the appropriate defect pool
     * fields.  If there where no errors, then we are done and the PV is added
     * to the VG.  Read in the VGDA timestamps in anticipation of activation.
     */
    if (((error = lv_initdefects(pv, 0))     != ESUCCESS) ||
	((error = lv_vgaddpv(vg, pv, pvnum)) != ESUCCESS)) {
	lock_done(&(vg->vg_lock));
	lv_closepv(pv);
	KFREE(pv, sizeof(struct pvol));
	return(error);
    }
    lv_readvgdats(vg, pv);

    /* Return success */
    lock_done(&(vg->vg_lock));
    return(error);
}

/*
 * NAME:	lv_installpv
 *
 * FUNCTION:
 *	Stub for install physical volume ioctl.
 *
 * PARAMETERS:
 *	vg  - The volume group to attach the PV to.
 *	ipv - The install PV ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *	EINVAL - Invalid ioctl argument structure.
 *
 */
int
lv_installpv(vg, ipv)
struct	volgrp	*vg;		/* The volume group be ioctled.	*/
struct	lv_installpv *ipv;	/* Description of PV to install.*/
{
	int error;

	if (ipv->pv_flags & ~PV_FLAGS) {
		return(EINVAL);
	}
	if (ipv->pv_flags & LVM_NOVGDA) {
		/* Temporary change until this function works */
		return(EOPNOTSUPP);
	}

	lock_write(&(vg->vg_lock));

	if (ipv->pxspace == 0) {
		ipv->pxspace = (1<<vg->vg_pxsize);
	} else if (ipv->pxspace < (1 << vg->vg_pxsize)) {
		lock_done(&(vg->vg_lock));
		return(EINVAL);
	}

	/* Call the common install pv function. */
	error = lv_installpv_common(vg, ipv->path, ipv->pxspace,
			ipv->pv_flags, ipv->maxdefects);

	lock_done(&(vg->vg_lock));
	return(error);
}

/*
 * NAME:	lv_installpv_common
 *
 * FUNCTION:
 *	Common physical volume install function.  This function will read the
 * LVM record from the physical volume, perform some error checking and if
 * everything is kosher, install the physical volume in the specified volume
 * group.
 *
 * PARAMETERS:
 *	vg  - The volume group structure to install the pv into.
 *	path - The path to the physical volume device.
 *	flags - The installation flags.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_installpv_common(vg, path, pxspace, pv_flags, maxdefects)
struct	volgrp	*vg;		/* The volume group be ioctled.	     */
char		*path;		/* The physical volume pathname.     */
uint_t		pxspace;	/* space to allocate for each extent */
ushort_t	pv_flags;	/* Same as lv_statuspv.pv_flags.     */
ushort_t	maxdefects;	/* Maximum defects in defect dir     */
{
struct	pvol		*pv;
	lv_lvmrec_t	*lvmrec;
	int		error;
	uint_t		pxcount;
	struct PV_header *PV;

	/*
	 * Make sure that the volume group is activated.
	 */
	if ((vg->vg_flags & VG_ACTIVATED) == 0)
		return(EROFS);	/* Can't modify volume group */

	if (vg->num_pvols >= vg->vg_maxpvs)
		return(EMLINK);	/* Too many physical volumes */

	/* Allocate a pvol and clear it */
	if ((pv = NEW(struct pvol)) == NULL)
		return(ENOMEM);

	bzero((caddr_t)pv, sizeof(struct pvol));

	pv->pv_vg = vg;
	/* Open the passed volume and read in the LVM record */
	if ((error = lv_openpv(path, pv)) != ESUCCESS) {
		KFREE(pv, sizeof(struct pvol));
		return(error);
	}
	/*
	 * Error check the LVM record.
	 */
	lvmrec = pv->pv_lvmrec;

	if (!zeroID(&lvmrec->vg_id)) {
		error = EBUSY;
	} else if ((bcmp(lvmrec->lvm_id, "LVMREC01", 8) != 0) ||
	    (lvmrec->last_psn == 0) ||
	    (!zeroID(&lvmrec->vg_id)) ||
	    (zeroID(&lvmrec->pv_id))) {
		error = ENODEV;
	} else {
		/*
		 * Update the LVM record based on the volume group data.
		 */
		lvmrec->vg_id    = vg->vg_id;
		lvmrec->vgda_len = vg->vgda_len;
		lvmrec->vgsa_len = vg->vgsa_len;
		lvmrec->mcr_len  = vg->mcr_len;
		lvmrec->vgra_psn = PVRA_SIZE;
		if ((pv_flags & LVM_NOVGDA) == 0) {
			lvmrec->vgra_len = 2 *
					(vg->vgda_len+vg->vgsa_len+vg->mcr_len);
			lvmrec->vgda_psn1 = PVRA_SIZE;
			lvmrec->mcr_psn1 = lvmrec->vgda_psn1 + vg->vgda_len
					+ vg->vgsa_len;
			lvmrec->vgda_psn2 = lvmrec->mcr_psn1 + lvmrec->mcr_len;
			lvmrec->mcr_psn2 = lvmrec->vgda_psn2 + vg->vgda_len
					+ vg->vgsa_len;
		} else {
			lvmrec->vgra_len = 2 * vg->mcr_len;
			lvmrec->vgda_psn1 = 0;
			lvmrec->mcr_psn1 = PVRA_SIZE;
			lvmrec->vgda_psn2 = 0;
			lvmrec->mcr_psn2 = lvmrec->mcr_psn1 + lvmrec->mcr_len;
		}		
		/*
		 * Make sure that the VGRA fits on the disk.
		 */
		if (lvmrec->vgra_len > lvmrec->last_psn) {
			error = ENOSPC;
		}
	}
	if (error) {
		lv_closepv(pv);
		KFREE(pv, sizeof(struct pvol));
		return(error);
	}
	lvmrec->data_psn = lvmrec->mcr_psn2 + lvmrec->mcr_len;
	lvmrec->pxsize = vg->vg_pxsize;
	pxspace = pxspace >> DEV_BSHIFT;
	lvmrec->pxspace = pxspace;
	pxcount = (lvmrec->last_psn - lvmrec->data_psn) / pxspace;
	lvmrec->altpool_psn = lvmrec->data_psn + (pxcount * pxspace);
	lvmrec->altpool_len = lvmrec->last_psn - lvmrec->altpool_psn;

	if (lvmrec->altpool_len < BYTE2BLK(maxdefects)+1) {
		pxcount--;
		lvmrec->altpool_psn -= pxspace;
		lvmrec->altpool_len += pxspace;
	}
	if (pxcount > vg->vg_maxpxs) pxcount = vg->vg_maxpxs;
	lvmrec->data_len = lvmrec->altpool_psn - lvmrec->data_psn;

	if ((error = lv_vgaddpv(vg, pv, -1)) != ESUCCESS) {
		lv_closepv(pv);
		KFREE(pv, sizeof(struct pvol));
		return(error);
	}
	lvmrec->pv_num = pv->pv_num;

	pv->pv_pxcount = pxcount;
	pv->pv_freepxs = pxcount;
	pv->pv_vgra_psn = lvmrec->vgra_psn;
	pv->pv_data_psn = lvmrec->data_psn;
	pv->pv_flags = pv_flags|LVM_PVDEFINED;

	/*
	 * Initialize the pvol defect table, updating the appropriate defect
	 * pool fields.  If there where no errors, then we are done and the PV
	 * is added to the VG.  Write the LVM record back to the disk using
	 * the block device. 
	 */
	if (((error = lv_initdefects(pv, maxdefects)) != ESUCCESS) ||
	    ((error = lv_writelvmrec(pv))	      != ESUCCESS) ||
	    ((error = lv_writemwcrecs(vg, pv))	      != ESUCCESS)) {
		if ((lvmrec = pv->pv_lvmrec) != NULL) {
			clearID(&lvmrec->vg_id);
			lvmrec->pv_num = 0;
			lv_writelvmrec(pv);
		}
		lv_closepv(pv);
		lv_vgsubpv(vg, pv);
		KFREE(pv, sizeof(struct pvol));
		return(error);
	}

	/* Add the physical volume to the VGDA PV list. */
	PV = PV_head(vg, pv->pv_num);
	PV->pv_id = lvmrec->pv_id;
	PV->px_count = pv->pv_pxcount;
	PV->pv_flags = pv_flags;
	PV->pv_msize = vg->vg_maxpxs;
	VG_head(vg)->numpvs++;
	if (vg->num_pvols != VG_head(vg)->numpvs) {
		panic("lv_installpv_common: num_pvols");
	}

	/* Write the VGDA to all physical volumes. */
	if ((error = lv_updatevgda(vg)) != ESUCCESS) {
		if ((lvmrec = pv->pv_lvmrec) != NULL) {
			clearID(&lvmrec->vg_id);
			lvmrec->pv_num = 0;
			lv_writelvmrec(pv);
		}
		lv_closepv(pv);
		lv_vgsubpv(vg, pv);
		KFREE(pv, sizeof(struct pvol));
		return(error);
	}

	/* Tell the status area manager that a new physical volume exists. */
	if (lv_sa_config(vg, CNFG_INSTALLPV, pv) != ESUCCESS) {
	    return(ENODEV);
	}
	
	/* KFREE(pv->pv_lvmrec); eventually */
	/*
	 * If there where no errors, then we are done and the PV is added to
	 * the VG. 
	 */
	return(error);
}

/*
 * NAME:	lv_setvgid
 *
 * FUNCTION:
 *	Set the volume group ID for subsequent lv_attachpv calls.  The
 * volume group ID will be used to validate the attached physical volumes,
 * making they are members of the volume group.
 *
 * PARAMETERS:
 *	vg  - The volume group structure to hold the ID.
 *	apv - The lv_setvgid ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_setvgid(vg, vg_id)
struct volgrp *vg;	/* Volume group to be ID'd */
lv_uniqueID_t *vg_id;	/* Volume group unique ID   */
{
	int error;

	lock_write(&(vg->vg_lock));
   
	if (!equalID(&vg->vg_id, vg_id) && (vg->num_pvols != 0)) {
		error = EEXIST;
	} else {
		/*
		 * Set the volume group ID in the volgrp structure
		 * and return success
		 */
		vg->vg_id = *vg_id;
		error = ESUCCESS;
	}

	lock_done(&(vg->vg_lock));
	return(error);
}

/*
 * NAME:	lv_createvg
 *
 * FUNCTION:
 *      If no unique ID exists for this volume group, and generate one.
 * After verifying that the volume group has not already been created, mark
 * the volume group as active, and install the first physical volume.
 *
 * PARAMETERS:
 *	vg  - The volume group structure.
 *	apv - The lv_createvg ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_createvg(vg, cvg)
struct volgrp      *vg;		/* Volume group being created     */
struct lv_createvg *cvg;	/* Volume group parameters to set */
{
	int	error;
	int	i;

	/* Verify that there are no physical volumes in this volume group. */
	if (vg->num_pvols != 0)
		return(EEXIST);

	/*
 	 * Check all of the arguments before setting anything in the volume
	 * group header. Any constraint violation provokes EINVAL.
	 */
	if (zeroID(&cvg->vg_id))	/* Must specify vgid */
    		return(EINVAL);
	if (cvg->maxlvs > LVM_MAXLVS)
		return(EINVAL);
	if (cvg->maxpvs > LVM_MAXPVS)
		return(EINVAL);
	if (cvg->maxpxs > LVM_MAXPXS)
		return(EINVAL);
	for (i = LVM_MINPXSIZE; i <= LVM_MAXPXSIZE; i *= 2)
		if (cvg->pxsize == i)
			break;
	if (i > LVM_MAXPXSIZE)
		return(EINVAL);
	if ((cvg->pxspace < cvg->pxsize) || (cvg->pxspace & (DEV_BSIZE-1)))
		return(EINVAL);
	if (cvg->pv_flags & LVM_NOVGDA)	/* logically inconsistent */
		return(EINVAL);
	if (cvg->pv_flags & ~PV_FLAGS)
		return(EINVAL);

	lock_write(&(vg->vg_lock));

	/*
	 * All checks were passed, set up the volume group structure.
	 */
	vg->vg_id = cvg->vg_id;
	vg->vg_maxlvs = cvg->maxlvs;
	vg->vg_maxpvs = cvg->maxpvs;
	vg->vg_maxpxs = cvg->maxpxs;
	for (i = 0; i < (NBBY*NBPW); i++)
		if (cvg->pxsize == (1 << i))
			break;
	vg->vg_pxsize = i;
	vg->vg_extshift = (vg->vg_pxsize - DEV_BSHIFT);

	vg->vgsa_len = ((20 +
	    (((vg->vg_maxpvs+((NBBY*NBPW)-1))/(NBBY*NBPW))*NBPW) +
	    ((((vg->vg_maxpxs*vg->vg_maxpvs)+((NBBY*NBPW)-1))/(NBBY*NBPW))*NBPW)) +
			(DEV_BSIZE-1))/DEV_BSIZE;

	vg->mcr_len = 1;

	if ((error = lv_cache_activate(vg, 0)) != ESUCCESS)
		goto out;

	if ((error = lv_sa_createvgsa(vg)) != ESUCCESS)
		goto out;

	/* Activate the volume group */
	vg->vg_flags |= VG_ACTIVATED;

	/*
	 * Create the vgda and fill it in with the current known values.
	 *
	 * Attempt to install the first volume, if this fails, then we have to
	 * back out the previous operations.
	 */
	if (((error = lv_createvgda(vg)) != ESUCCESS) ||
	    ((error = lv_installpv_common(vg, cvg->path, cvg->pxspace,
			cvg->pv_flags, cvg->maxdefects)) != ESUCCESS)) {
out:
		clearID(&vg->vg_id);
		vg->vg_maxlvs = 0;
		vg->vg_maxpvs = 0;
		vg->vg_maxpxs = 0;
		vg->vg_pxsize = 0;
		vg->mcr_len = 0;
		vg->vg_flags &= ~VG_ACTIVATED;

		/* Deallocate the VGDA if it was allocated. */
		KFREE(vg->vg_vgda, vg->vgda_len*DEV_BSIZE);
	}
	lock_done(&(vg->vg_lock));
	return(error);
}

/*
 * NAME:	lv_activatevg
 *
 * FUNCTION:
 *      Reconcile the on-disk volume group descriptor areas (VGDA's) of all
 * attached volumes.  This involves establishing the volume group state by
 * locating the most recent VGDA on all attached physical volumes.  Quorum
 * is determined by ensuring that more than half of the physical volumes in
 * the volume group are present.  If discrepancies exist among the various
 * VGDA's, the newest is written to all members of the set.
 *
 * PARAMETERS:
 *	vg  - The volume group structure to hold the ID.
 *	avg - The lv_activatevg ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_activatevg(vg, flags)
struct volgrp *vg;		/* Volume group being activated */
uint_t      flags;		/* Flags controlling activation */
{
    int		     pvnum, pxnum,
		     lvnum, lxnum,
		     mirror, m,
		     need_update;
    struct PV_header *PV;	/* Pointer to physical volumes in the VGDA   */
    struct extent    *ext;
    struct lextent   *le;
    struct pvol	     *pv;	/* Pointer to current physical volume	     */
    int		     error;

    /* Reject if there are any flag bits specified that we don't understand */
    if ((flags & ~(VG_FLAGS)) != 0) {
	return(EINVAL);
    }

    if (vg->vg_flags & VG_ACTIVATED) {
	if (flags & LVM_ACTIVATE_LVS)
		vg->vg_flags &= ~VG_NOLVOPENS;
	else
		vg->vg_flags |= VG_NOLVOPENS;
	return(ESUCCESS);
    }

    if ((error = lv_getvgda(vg, &need_update)) != ESUCCESS) {
	return(error);
    }

    if ((error = lv_init_pvols(vg, flags)) != ESUCCESS) {
	return(error);
    }

    /*
     * On attach, the defect directory on each physical volume was initialized
     * according to heuristics, rather than the system administrator's wishes.
     * This was because, at attach time, the SA's wishes weren't known.  Now
     * they are, so reinitialize the defect directories on all attached
     * physical volumes.  If pv_defectlim is zero, nothing can be gained from
     * reinitializing.
     */
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
        if (vg->pvols[pvnum] == NULL) continue;
	if (PV_head(vg, pvnum)->pv_defectlim != 0)
	    lv_initdefects(vg->pvols[pvnum], PV_head(vg, pvnum)->pv_defectlim);
    }

    /* Read the VGSA, if there is an error - return it. */
    if ((error = lv_sa_readvgsa(vg)) != ESUCCESS) {
    	return(error);
    }

    if ((error = lv_init_lvols(vg)) != ESUCCESS) {
    	return(error);
    }
    /*
     * Now that we have a valid VGDA, and VGSA in memory, it is time to
     * start fixing things up:
     * We must not have issued any writes to the volume group up until
     * this point (we should probably put a flag in the struct pvol and
     * assert this)
     * First, we activate the mirror write cache, this will cause writes
     * to occur to the volume group, updating the VGSA as a side effect.
     */
    if ((error = lv_cache_activate(vg, (flags & LVM_NONMISSING_PVS_REQUIRED)))
		!= ESUCCESS)
	return(error);

    /* If there are any physical volumes missing, mark them such. */
    if (lv_sa_missing(vg) != ESUCCESS)
	    return(ENOENT);

    /*
     * Unless all volumes contain identical VGDA's, update the VGDA's on ALL
     * volumes in the volume group.  Set the timestamp on the VGDA to the
     * current time.
     */
    if (need_update) {
	lv_updatevgda(vg);
    }

    /* If requested, activate all the logical volumes as well. */
    if (flags & LVM_ACTIVATE_LVS)
	vg->vg_flags &= ~VG_NOLVOPENS;
    else
	vg->vg_flags |= VG_NOLVOPENS;

    /* Mark the volume group as active and return success */
    vg->vg_flags |= VG_ACTIVATED;
    return(ESUCCESS);
}

int
lv_getvgda(vg, need_update)
struct volgrp *vg;
int *need_update;
{
    int		     masterpv,	/* physical volume containing master VGDA    */
		     updates,
		     error,
		     pvnum;
    struct timeval   pvstamp,	/* Max timestamp for this physical volume    */
		     timestamp;	/* Most recent timestamp so far		     */
    struct pvol      *pv;

    /*
     * Determine the Volume Group state - find the newest VGDA.  This is done
     * by comparing the timestamps from both copies of each VGDA on all
     * attached physical volumes.  The VGDA with the most recent timestamp is
     * used as the master.
     */
    *need_update = 0;
    masterpv = -1;
    timerclear(&timestamp);
    timerclear(&pvstamp);
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
	if ((pv = vg->pvols[pvnum]) == NULL) continue;

	/* Set pvstamp to the maximum of the two timestamps */
	if (timercmp(&pv->pv_vgdats[0], &pv->pv_vgdats[1], >))
	    pvstamp = pv->pv_vgdats[0];
	else
	    pvstamp = pv->pv_vgdats[1];

	/*
	 * If pvstamp is greater than the previous timestamp, make it the
	 * new timestamp, and remember which physical volume it came from.
	 */
	if (timercmp(&pvstamp, &timestamp, >)) {
	    timestamp = pvstamp;
	    masterpv = pvnum;
	    *need_update++;
	}
    }
    /*
     * Either lv_readvgdats failed each time through, or no valid timestamps
     * were found  - either way, activation would be difficult.
     */
    if (masterpv == -1)
	return(ENODEV);

    /*
     * masterpv is physical volume containing the most recent valid VGDA in the
     * group.  Read in that vgda and initialize.
     */
    error = lv_readvgda(vg, vg->pvols[masterpv], 0);

    return(error);
}

int
lv_init_pvols(vg, flags)
struct volgrp *vg;
int flags;
{
    int missing, pvnum, error;
    struct pvol *pv;
    struct PV_header *PV;

    missing = 0;
    for (pvnum = 0;
		(PV = PV_head(vg,pvnum)) < (struct PV_header *)VG_trail(vg);
							pvnum++) {
	/*
	 * Skip non-existent physical volumes, possibly disposing of
	 * charlatans. This check is separate from the equalID check
	 * only to prevent allocating a max-size (pvol *) array every
	 * time we activate the volume group.
	 */
	if (zeroID(&PV->pv_id)) {
		if ((pvnum < vg->size_pvols)
			&& (pv = vg->pvols[pvnum]) != NULL) {
			lv_vgsubpv(vg, pv);
			lv_closepv(pv);
			KFREE(pv, sizeof(struct pvol));
		}
		continue;
	}

	/*
	 * There should be a physical volume at this location. Make
	 * sure the correct struct pvol is there.
	 */
	if ((pvnum >= vg->size_pvols)
		|| ((pv = vg->pvols[pvnum]) == NULL)) {
		/*
		 * This pvol was not attached
		 */
		if (flags & LVM_ALL_PVS_REQUIRED)
			return(EEXIST);

		if ((pv = NEW(struct pvol)) == NULL) {
			return(ENOMEM);
		}
		bzero(pv, sizeof(struct pvol));
		pv->pv_vg = vg;
		missing++;
		if (error = lv_vgaddpv(vg, pv, pvnum)) {
			KFREE(pv, sizeof(struct pvol));
			return(error);
		}
		pv->pv_flags = LVM_NOTATTACHED;
	} else {
		/*
		 * There is a pvol attached at this location,
		 * make sure it is the right one.
		 */
		if (!equalID(&PV->pv_id, &pv->pv_lvmrec->pv_id)) {
			/*
			 * This pvol was lying. Discard it. What we
			 * really have here is a non-attached physical
			 * volume.
			 */
			lv_closepv(pv);
			lv_vgsubpv(vg, pv);
			if (flags & LVM_ALL_PVS_REQUIRED) {
				KFREE(pv, sizeof(struct pvol));
				return(EEXIST);
			}
			bzero(pv, sizeof(struct pvol));
			missing++;
			lv_vgaddpv(vg, pv, pvnum);
			pv->pv_flags = LVM_NOTATTACHED;
		}
	}
	pv->pv_flags |= PV->pv_flags;
	pv->pv_pxcount = PV->px_count;
	if (pv->pv_flags & LVM_NOVGDA) {
		struct extent *ext;
		/*
		 * This pvol does not (or shouldn't) contain a VGDA/VGSA.
		 * Remove it from the lvol[0] extent map. XXX not supported.
		 */
		ext = EXTENT(VG_LVOL0(vg), pvnum, PRIMMIRROR);
		ext->e_pvnum = PX_NOPV;
	}
    }
    if (vg->num_pvols != VG_head(vg)->numpvs) {
		panic("lv_activatevg: num_pvols");
    }

    /*
     * Check to see if there is quorum - this is defined as having more than
     * half of the volume group's volumes currently attached.  The number used
     * is obtained from the above VGDA.
     */
    if ((vg->num_pvols - missing) <= (vg->num_pvols / 2))
    	return(ENXIO);

    return(ESUCCESS);
}

int
lv_init_lvols(vg)
struct volgrp *vg;
{
    struct lvol *lv;
    struct lvol *lv_alloclv();
    struct pvol *pv;
    struct extent *ext;
    int lvnum, lxnum, mirror, m, pvnum;
    int error;

    /* 
     * Now fill out the extents for any logical volumes in the volume group.
     * First create lvol structures for each logical volume, as well as
     * extent arrays.
     */
    for (lvnum = 1; lvnum < vg->vg_maxlvs; lvnum++) {
	struct LV_entry *LV = LV_ent(vg, lvnum);
	struct lextent *le;

	if (LV->lv_flags & LVM_LVDEFINED) {
	    if ((lv = lv_alloclv(vg, makedev(vg->major_num,lvnum))) == NULL)
		return(ENOMEM);
	    lv->lv_maxlxs      = LV->maxlxs;
	    lv->lv_flags       = LV->lv_flags;
	    lv->lv_sched_strat = LV->sched_strat;

	    switch (lv->lv_sched_strat) {
	    case LVM_RESERVED:	  lv->lv_schedule = &lv_reserved_ops; break;
	    case LVM_SEQUENTIAL:  lv->lv_schedule = &lv_sequential_ops; break;
	    case LVM_PARALLEL:    lv->lv_schedule = &lv_parallel_ops; break;
	    default:
		    panic("lv_init_lvols: illegal scheduling strategy\n");
		    break;
	    }

	    lv->lv_maxmirrors  = LV->maxmirrors;

	    for (mirror = 0; mirror <= lv->lv_maxmirrors; mirror++) {
		lv->lv_exts[mirror] = (extent_t *)kalloc(sizeof(extent_t) * 
						      lv->lv_maxlxs);
		if (lv->lv_exts[mirror] == NULL) {
		    for (m = 0; m < mirror; m++)
		    	KFREE(lv->lv_exts[m],sizeof(extent_t) * lv->lv_maxlxs);
		    return(ENOMEM);
		}
		ext = lv->lv_exts[mirror];
		bzero(ext, sizeof(extent_t) * lv->lv_maxlxs);
		for (lxnum = 0; lxnum < lv->lv_maxlxs; lxnum++) {
		    ext->e_pvnum = PX_NOPV;
		    ext++;
		}
	    }
	    le = (lextent_t *)kalloc(sizeof(lextent_t) * lv->lv_maxlxs);
	    if (le == NULL) {
		for (mirror = 0; mirror <= lv->lv_maxmirrors; mirror++)
		    KFREE(lv->lv_exts[mirror],
					sizeof(extent_t) * lv->lv_maxlxs);
		return(ENOMEM);
	    }
	    lv->lv_lext = le;

	    /* Initialize the logical extent structure */
	    for (lxnum = 0; lxnum < lv->lv_maxlxs; lxnum++) {
		le->lx_synctrk = -1;
		le->lx_syncmsk = 0;
		le->lx_syncerr = 0;
		le++;
	    }
	}
    }

    /*
     * Then cycle through the available physical volumes, adding logical
     * extents to correspond to the physical extents encountered, and
     * initializing the physical state.
     */
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
	int pxnum;
	struct extent t_ext;
	if (((pv = vg->pvols[pvnum])) == NULL) continue;

	pv->pv_freepxs = 0;
	t_ext.e_pvnum = pvnum;

	if (SA_TST_PVMISSING(vg, pvnum)) {
		pv->pv_flags |= LVM_PVMISSING;
	}

	for (pxnum = 0; pxnum < pv->pv_pxcount; pxnum++) {

	    lvnum = PX_ent(vg, pvnum, pxnum)->lv_index;
	    lxnum = PX_ent(vg, pvnum, pxnum)->lx_num;

	    if (lvnum != 0) {
		t_ext.e_pxnum = pxnum;
		lv = VG_DEV2LV(vg, makedev(vg->major_num, lvnum));
		if (SA_TST_PXSTALE(vg, pvnum, pxnum))
		    t_ext.e_state = PX_STALE;
		else
		    t_ext.e_state = 0;
		if ((error = lv_addextent(lv, &t_ext, lxnum, ANYMIRROR)) !=
			    ESUCCESS)
		    return(error);
	    } else {
		pv->pv_freepxs++;
	    }
	}
    }
    return(ESUCCESS);
}

/*
 * NAME:	lv_deactivatevg
 *
 * FUNCTION:
 *      Verify that all logical volumes in the volume group are closed.
 * Flush the mirror write cache to all physical volumes of the group.  Mark
 * the volume group as inactive.
 *
 * PARAMETERS:
 *	vg  - The volume group structure to hold the ID.
 *	apv - The lv_deactivatevg ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_deactivatevg(vg)
struct volgrp *vg;	/* Volume group being deactivated */
{
    int lvnum, pvnum;
    struct pvol *pv;

    /*
     * Verify that all logical volumes in the volume group are closed.  Note
     * that logical volume 0 must be open to get here, so the number of
     * logical volumes must always be 1 or more.
     */
    lock_write(&(vg->vg_lock));

    if (vg->vg_opencount > 1) {
        lock_done(&(vg->vg_lock));
	return(EBUSY);
    }

    /* Flush the mirror write consistency cache. */
    lv_cache_deactivate(vg);

    /* Deallocate all logical volumes but lvol 0 */
    for (lvnum = 1; lvnum < vg->num_lvols; lvnum++) {
	struct lvol *lv = vg->lvols[lvnum];

	if (lv != NULL) {
	    lv_freelv(vg, lvnum);
	}
    }
    /* For all physical volumes that aren't attached, free up the
     * struct pvols. */
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
	if (((pv = vg->pvols[pvnum]) == NULL)
		|| ((pv->pv_flags & LVM_NOTATTACHED) == 0)) continue;
	lv_vgsubpv(vg, pv);
	KFREE(pv, sizeof(struct pvol));
    }

    /* Deallocate the VGDA currently associated with the volume group. */
    KFREE(vg->vg_vgda, vg->vgda_len*DEV_BSIZE);

    /* Mark the volume group as inactive. */
    vg->vg_flags &= ~VG_ACTIVATED;

    /* Return success */
    lock_done(&(vg->vg_lock));
    return(ESUCCESS);
}

/*
 * NAME:	lv_queryvg
 *
 * FUNCTION:	Return information about the current state of the volume group.
 *
 * PARAMETERS:
 *	vg  - The volume group structure to hold the ID.
 *	avg - The lv_queryvg ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_queryvg(vg, avg)
struct volgrp *vg;	/* Volume group being queried */
struct lv_queryvg *avg;	/* Information returned */
{
    int lvnum, pvnum, pxnum;
    struct pvol *pv;

    lock_read(&(vg->vg_lock));
    /*
     * The volume group must be activated to return any interesting
     * information.  If it is not, set the status field to say so.
     */
    bzero(avg, sizeof(struct lv_queryvg));
    if (vg->vg_flags & VG_ACTIVATED) {
	avg->status = LVM_VGACTIVATED;
	if ((vg->vg_flags & VG_NOLVOPENS) == 0) {
		avg->status |= LVM_LVSACTIVATED;
	}
    }

    /* 
     * Fill in those fields in the lv_queryvg structure available from the
     * volgrp structure.
     */
    avg->vg_id  = vg->vg_id;
    avg->maxlvs = vg->vg_maxlvs;
    avg->maxpvs = vg->vg_maxpvs;
    avg->maxpxs = vg->vg_maxpxs;
    avg->pxsize = vg->vg_pxsize?(1 << vg->vg_pxsize):0;
    avg->cur_pvs = vg->num_pvols;

    /*
     * The remaining fields are derived.  First count the number of logical
     * volumes currently in the volume group.
     */
    for (lvnum = 1; lvnum < vg->num_lvols; lvnum++)
	if (vg->lvols[lvnum] != NULL)
	    avg->cur_lvs++;

    /*
     * Next determine the total number of free physical extents
     * in the volume group.
     */
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
	if ((pv = vg->pvols[pvnum]) != NULL) {
		avg->freepxs += pv->pv_freepxs;
        }
    }

    lock_done(&(vg->vg_lock));
    /* Return success */
    return(ESUCCESS);
}

#ifdef LVM_OQUERYVG
/*
 * NAME:	lv_oqueryvg
 *
 * FUNCTION:	Return information about the current state of the volume group.
 *		(TEMPORARY COMPATIBILITY FUNCTION)
 *
 * PARAMETERS:
 *	vg  - The volume group structure to hold the ID.
 *	avg - The lv_oqueryvg ioctl argument structure.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_oqueryvg(vg, avg)
struct volgrp *vg;	/* Volume group being queried */
struct lv_oqueryvg *avg;	/* Information returned */
{
    int lvnum, pvnum, pxnum;
    struct pvol *pv;

    lock_read(&(vg->vg_lock));
    /*
     * The volume group must be activated to return any interesting
     * information.  If it is not, set the status field to say so.
     */
    bzero(avg, sizeof(struct lv_queryvg));
    if (vg->vg_flags & VG_ACTIVATED) {
	avg->status = LVM_VGACTIVATED;
	if ((vg->vg_flags & VG_NOLVOPENS) == 0) {
		avg->status |= LVM_LVSACTIVATED;
	}
    }

    /* 
     * Fill in those fields in the lv_queryvg structure available from the
     * volgrp structure.
     */
    avg->vg_id  = vg->vg_id;
    avg->maxlvs = vg->vg_maxlvs;
    avg->maxpvs = vg->vg_maxpvs;
    avg->pxsize = vg->vg_pxsize?(1 << vg->vg_pxsize):0;
    avg->cur_pvs = vg->num_pvols;

    /*
     * The remaining fields are derived.  First count the number of logical
     * volumes currently in the volume group.
     */
    for (lvnum = 1; lvnum < vg->num_lvols; lvnum++)
	if (vg->lvols[lvnum] != NULL)
	    avg->cur_lvs++;

    /*
     * Next determine the total number of free physical extents
     * in the volume group.
     */
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
	if ((pv = vg->pvols[pvnum]) != NULL) {
		avg->freepxs += pv->pv_freepxs;
        }
    }

    lock_done(&(vg->vg_lock));
    /* Return success */
    return(ESUCCESS);
}
#endif /* LVM_OQUERYVG */

/*
 * NAME:	lv_changepv
 *
 * FUNCTION:	Change the characteristics of a physical volume.
 */
int
lv_changepv(vg, arg)
struct volgrp *vg;
struct lv_changepv *arg;
{
	struct pvol *pv;
	struct PV_header *PV;
	int pvnum = arg->pv_key;

	if (arg->pv_flags & ~PV_FLAGS) {
		return(EINVAL);
	}
	if (arg->pv_flags & LVM_NOVGDA) {
		/* Temporary change until this function works */
		return(EOPNOTSUPP);
	}

	lock_write(&(vg->vg_lock));

	if ((pvnum >= vg->size_pvols)
		|| ((pv = vg->pvols[pvnum]) == NULL)) {
		lock_done(&(vg->vg_lock));
		return(ENXIO);
	}
	if ((int)arg->maxdefects < (int)(pv->pv_maxdefects - pv->freelistsize)) {
		lock_done(&(vg->vg_lock));
		return(EBUSY);
	}
	PV = PV_head(vg,pvnum);
	PV->pv_defectlim = arg->maxdefects;
	PV->pv_flags = arg->pv_flags|LVM_PVDEFINED;

	pv->pv_flags = (PV->pv_flags & PV_FLAGS) | (pv->pv_flags & ~PV_FLAGS);
	/* NOTE: we do not change pv->pv_maxdefects, it will not
	 * become effective until the next activate */

	lock_write_to_read(&(vg->vg_lock));

	lv_updatevgda(vg);

	lock_done(&(vg->vg_lock));
	return(ESUCCESS);
}

/*
 * NAME:	lv_deletepv
 *
 * FUNCTION:	Delete (permanently remove) a physical volume from
 *		a volume group. The VGDA on all other physical volumes
 *		in the volume group is updated, and this physical volume's
 *		LVM record is updated to show that it no longer a member.
 *		The device is closed.
 */
int
lv_deletepv(vg, pvnum)
struct volgrp *vg;
int pvnum;
{
	struct pvol *pv;
	struct PV_header *PV;
	struct PX_entry *PX;
	lv_lvmrec_t *lvmrec;
	int pxnum;

	lock_write(&(vg->vg_lock));

	if ((vg->vg_flags & VG_ACTIVATED) == 0){
		lock_done(&(vg->vg_lock));
		return(EROFS);
	}

	if ((pvnum >= vg->size_pvols) || ((pv = vg->pvols[pvnum]) == NULL)) {
		lock_done(&(vg->vg_lock));
		return(ENXIO);
	}

	/* If deleting last pvol, make sure no logical volumes are open. */
	if (vg->num_pvols == 1 && vg->vg_opencount > 1) {
		lock_done(&(vg->vg_lock));
		return(EBUSY);
	}

	if (pv->pv_freepxs != pv->pv_pxcount) {
		lock_done(&(vg->vg_lock));
		return(EBUSY);
	}

	PV = PV_head(vg,pvnum);
	clearID(&PV->pv_id);
	PV->px_count = 0;	/* do NOT change pv_msize! */
	PV->pv_flags = 0;
	VG_head(vg)->numpvs--;
	
	lock_write_to_read(&(vg->vg_lock));

	lv_updatevgda(vg);

	/*
	 * Now delete the LVM Record. The PV can't be reattached
	 * once this occurs. This cannot be done if the PVOL is
	 * not currently attached.
	 */
	if ((lvmrec = pv->pv_lvmrec) != NULL) {
		clearID(&lvmrec->vg_id);
		lvmrec->pv_num = 0;
		lv_writelvmrec(pv);
	}

	/*
	 * If there are other physical volumes in the volume group,
	 * mark this PV as non-existent in their versions of the VGSA.
	 */
	if (vg->num_pvols != 1) {
		lv_sa_config(vg, CNFG_PVMISSING, pv);
	}

	if ((pv->pv_flags & LVM_NOTATTACHED) == 0) {
		lv_closepv(pv);
	}

	lv_vgsubpv(vg, pv);

	if (vg->num_pvols != VG_head(vg)->numpvs) {
		panic("lv_deletepv: num_pvols");
	}

	if (vg->num_pvols == 0) {
		/* Flush the mirror write consistency cache. */
		lv_cache_deactivate(vg);

		/*
		 * Deallocate the VGDA currently associated with the volume
		 * group.
		 */
		KFREE(vg->vg_vgda, vg->vgda_len * DEV_BSIZE);

		/* Mark the volume group as inactive and non-existent */
		vg->vg_flags &= ~VG_ACTIVATED;
		clearID(&vg->vg_id);
		vg->vg_maxlvs = 0;
		vg->vg_maxpvs = 0;
		vg->vg_maxpxs = 0;
		vg->vg_pxsize = 0;
	}

	lock_done(&(vg->vg_lock));

	KFREE(pv, sizeof(struct pvol));

	return(ESUCCESS);
}

/*
 * NAME:	lv_removepv
 * 
 * FUNCTION:	Temporarily remove a physical volume from a volume
 *		group. This function is the inverse of 'lv_attachpv',
 *		and results in the device being closed.
 */
int
lv_removepv(vg, pvnum)
struct volgrp *vg;
int pvnum;
{
	struct pvol *pv;
	struct lvol *lv;
	struct extent *ext;

	lock_write(&(vg->vg_lock));

	if (vg->vg_flags & VG_ACTIVATED) {
		/* Eventually we should be able to do this */
		lock_done(&(vg->vg_lock));
		return(EBUSY);
	}
	if ((pvnum >= vg->size_pvols) || (pvnum < 0)
		|| ((pv = vg->pvols[pvnum]) == NULL)) {
		lock_done(&(vg->vg_lock));
		return(ENXIO);
	}
	if ((pv->pv_flags & LVM_NOTATTACHED) == 0) {
		lv_closepv(pv);
	}

	lv_vgsubpv(vg, pv);

	lock_done(&(vg->vg_lock));

	KFREE(pv, sizeof(struct pvol));

	return(ESUCCESS);
}

/*
 * NAME:	lv_querypv
 *
 * FUNCTION:
 *	Return information about a given physical volume in the buffer
 *	provided.
 *
 * PARAMETERS:
 *	qpv - the lv_querypv structure to be filled in.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_querypv(vg, qpv)
struct volgrp *vg;		/* The volume group be ioctled.	*/
struct lv_querypv *qpv;
{
    int i;
    struct pvol *pv;		/* Physical volume structure */

    lock_read(&(vg->vg_lock));

    /* Find the physical volume indicated.  If none exists, return an error. */
    if ((qpv->pv_key >= vg->size_pvols)
	|| ((pv = vg->pvols[qpv->pv_key]) == NULL)) {
	lock_done(&(vg->vg_lock));
	return(ENODEV);
    }

    /* Fill in the structure from the pvol. */
    qpv->pv_flags   = pv->pv_flags;
    qpv->px_count   = pv->pv_pxcount;
    if (pv->pv_flags & LVM_NOTATTACHED) {
	qpv->pv_rdev = NODEV;
	qpv->px_space = 0;
    } else {
	qpv->pv_rdev = pv->pv_vp->v_rdev;
	qpv->px_space = (pv->pv_pxspace << DEV_BSHIFT);
    }
    qpv->bbpool_len = pv->altpool_end - pv->altpool_psn + 1;
    if (vg->vg_vgda != NULL) {
	qpv->maxdefects = PV_head(vg, qpv->pv_key)->pv_defectlim;
    } else {
	qpv->maxdefects = qpv->bbpool_len;
    }
    qpv->px_free    = pv->pv_freepxs;

    /* Return success. */
    lock_done(&(vg->vg_lock));
    return(ESUCCESS);
}

/*
 * NAME:	lv_querypvmap
 *
 * FUNCTION:
 *	Return information about a given physical volume in the buffer
 *	provided.
 *
 * PARAMETERS:
 *	query - the lv_querypv structure to be filled in.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_querypvmap(vg, qpvm)
struct volgrp *vg;		/* The volume group being queried */
struct lv_querypvmap *qpvm;
{
    int		error = ESUCCESS,
		pxnum, lxnum;
    struct pvol *pv;		/* Physical volume structure */
    struct lvol *lv;		/* Logical Volume structure */
    struct PX_entry *pxep;	/* Physical extent descriptor pointer */
    struct extent *ext;		/* logical => physical extent mapping */
    int mirror;			/* mirror number */
    int pvnum;			/* physical volume number */
    pxmap_t	pxmap,		/* Physical extent */
		*pmap;		/* Pointer to user's pxmap buffer */

    lock_read(&(vg->vg_lock));

    /* Find the physical volume indicated.  If none exists, return an error. */
    if (((pvnum = qpvm->pv_key) >= vg->size_pvols)
	|| ((pv = vg->pvols[pvnum]) == NULL)) {
	lock_done(&(vg->vg_lock));
	return(ENODEV);
    }

    /* This ioctl is only meaningful on activated volume groups. */
    if (!(vg->vg_flags & VG_ACTIVATED)) {
	lock_done(&(vg->vg_lock));
	return(ENXIO);
    }

    /* Can only return as much information as we have. */
    if (qpvm->numpxs > pv->pv_pxcount) {
	qpvm->numpxs = pv->pv_pxcount;
    }

    pmap = qpvm->map;
    for (pxnum = 0; pxnum < qpvm->numpxs; pxnum++) {
	pxep = PX_ent(vg, pvnum, pxnum);
	pxmap.lv_minor	= pxep->lv_index;
	if (pxep->lv_index == 0) {
		pxmap.lv_extent = 0;
		pxmap.status = 0;
	} else {
		lxnum = pxep->lx_num;
		pxmap.lv_extent = lxnum;
		lv = VG_DEV2LV(vg, pxep->lv_index);
		for (mirror = 0; mirror < (lv->lv_maxmirrors+1); mirror++) {
			ext = EXTENT(lv, lxnum, mirror);
			if ((ext->e_pvnum == pvnum)
					&& (ext->e_pxnum == pxnum)) {
				pxmap.status = (ext->e_state & PX_STALE)
					? LVM_PXSTALE : 0;
				break;
			}
		}
		if (mirror == (lv->lv_maxmirrors+1)) {
			panic("lv_querypvmap");
		}
	}
	if (error = copyout(&pxmap, pmap, sizeof(pxmap_t)))
	    break;
	pmap++;
    }

    /* Return status. */
    lock_done(&(vg->vg_lock));
    return(error);
}

/*
 * NAME:	lv_querypvpath
 *
 * FUNCTION:
 *	Return information about a given physical volume in the buffer
 *	provided.
 *
 * PARAMETERS:
 *	qpvp - the lv_querypvpath structure to be filled in.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_querypvpath(vg, qpvp)
struct volgrp *vg;			/* The volume group be ioctled.	*/
struct lv_querypvpath *qpvp;
{
    int			error,		/* Return status */
			pvnum;		/* Loop index */
    struct nameidata	*ndp = &u.u_nd;	/* Nameidata for path lookup */
    struct vnode	*vp;		/* Vnode of the physical volume */

    /* Find the physical volume indicated.  If none exists, return an error. */
    if ((error = getmdev(&vp, qpvp->path, ndp)) != 0)
	return (error);

    lock_read(&(vg->vg_lock));

    error = ENODEV;

    /* Search the pvol list for the device. */
    for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
	if (vg->pvols[pvnum] && vg->pvols[pvnum]->pv_vp &&
	    (vg->pvols[pvnum]->pv_vp->v_rdev == vp->v_rdev)) {
	    /* Found it - set the pv_key and call lv_querypv */
	    qpvp->pv_key = pvnum;
	    error = lv_querypv(vg, &qpvp->pv_key);
	    break;
	}
    }

    lock_done(&(vg->vg_lock));
    vrele(vp);
    return(error);
}

/*
 * NAME:	lv_querypvs
 *
 * FUNCTION:
 *	Return information about a given physical volume in the buffer
 *	provided.
 *
 * PARAMETERS:
 *	vg - the volume group containing the physical volume
 *	qpvs - the lv_querypvs structure to be filled in.
 *
 * NOTES:
 *
 * RETURN:
 *
 */
int
lv_querypvs(vg, qpvs)
struct volgrp *vg;		/* The volume group be ioctled.	*/
struct lv_querypvs *qpvs;
{
    int	error, pvnum, j = 0;
    caddr_t keyptr;

    keyptr = (caddr_t)qpvs->pv_keys;

    lock_read(&(vg->vg_lock));

    /*
     * The pv_key is the same as the index into the pvol list.
     */
    for (pvnum = 0; (pvnum < vg->size_pvols) && (j < qpvs->numpvs); pvnum++) {
	if (vg->pvols[pvnum]) {
		if (error = copyout((caddr_t)&pvnum, keyptr, sizeof(ushort_t)))
			break;
		j++;
		keyptr += sizeof(ushort_t);
        }
    }
    qpvs->numpvs = j;

    lock_done(&(vg->vg_lock));
    return(error);
}
