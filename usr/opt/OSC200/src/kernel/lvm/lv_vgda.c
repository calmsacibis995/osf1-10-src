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
static char	*sccsid = "@(#)$RCSfile: lv_vgda.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:21:21 $";
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
 *	lv_vgda.c - Functions to manipulate the volume group descriptor area.
 *
 * FUNCTIONS:
 */

/*
 * Include files.
 */

#include <lvm/lvmd.h>

#include <sys/errno.h>


/*
 * NAME:	lv_createvgda
 *
 * FUNCTION:
 *	Create a VGDA.  This is only called at create VG.  This function
 * will allocate memory for the VGDA and initialize it.  The initialization is
 * based on the values in the supplied volgrp structure.
 *
 * PARAMETERS:
 *	vg  - The volume group structure.
 *
 * RETURN:
 *	ENOMEM - Could not allocate sufficient memory.
 */

lv_createvgda(vg)
struct	volgrp	*vg;		/* Volume group being created.     */
{
	/* We can not already have one... */
	if (vg->vg_vgda != NULL)
		panic("lv_createvgda: VGDA already exists!\n");

	/*
	 * Set up the pointers into the VGDA structure, and
	 * calculate the size.
	 */
	if ((vg->vg_maxlvs == 0)
		|| (vg->vg_maxpxs == 0)
		|| (vg->vg_maxpvs == 0))
		panic("lv_createvgda insufficient info");

	vg->vg_LVentry_off = ROUNDUP(sizeof(struct VG_header), DEV_BSIZE);
	vg->vg_PVentry_off = vg->vg_LVentry_off
		 + ROUNDUP(sizeof(struct LV_entry) * vg->vg_maxlvs, DEV_BSIZE);

	vg->vg_PVentry_len = ROUNDUP(sizeof(struct PV_header) +
		       (sizeof(struct PX_entry) * vg->vg_maxpxs), DEV_BSIZE);

	vg->vg_VGtrail_off = vg->vg_PVentry_off +
				(vg->vg_maxpvs*vg->vg_PVentry_len);

	vg->vgda_len = (vg->vg_VGtrail_off / DEV_BSIZE) +
		((sizeof(struct VG_trailer) + (DEV_BSIZE-1))/DEV_BSIZE);

	/* Allocate space for the VGDA. */
	if ((vg->vg_vgda = (char *)kalloc(vg->vgda_len*DEV_BSIZE)) == NULL)
		return(ENOMEM);

	bzero(vg->vg_vgda, vg->vgda_len*DEV_BSIZE);

	/*
	 * Initialize the VGDA header.  The timestamp is only set when the
	 * write is actually done.
         */
	VG_head(vg)->vg_id = vg->vg_id;
	VG_head(vg)->maxlvs = vg->vg_maxlvs;
	VG_head(vg)->maxpxs = vg->vg_maxpxs;
	VG_head(vg)->numpvs = 0;		/* No PVs installed yet. */

	/*
	 * All of the physical volumes are marked as "unused".  At some later
	 * time in this createvg function, the first PV will be installed.
	 * This will update the PV Header for that physical volume as active.
	 */
	return(ESUCCESS);
}

/*
 * NAME:	lv_readvgdats
 *
 * FUNCTION:
 *	Read the VGDA timestamps from the specified physical volume.  Each
 * timestamp is saved in the pvol structure.  If the head and trailer time
 * stamps do not match, then the VGDA is considered invalid and a 0 timestamp
 * is saved for that VGDA in the pvol.
 *
 * PARAMETERS:
 *	pv  - The physical volume to read the timestamps from.
 *
 * RETURN:
 */

lv_readvgdats(vg, pv)
struct	volgrp	*vg;		/* The volume group it belongs to   */
struct	pvol	*pv;		/* Physical volume to read.	    */
{
struct	timeval	header;		/* VGDA header timestamp.	    */
struct	timeval	trailer;	/* VGDA trailer timestamp.	    */
struct	buf	*lb;		/* Logical buffer.		    */
struct	lv_lvmrec *lvmrec;	/* Pointer to the pvol's lvm record */
	char	*addr;		/* Temporary buffer.		    */
	int	error;		/* Error indication.		    */

	timerclear(&pv->pv_vgdats[0]);
	timerclear(&pv->pv_vgdats[1]);

	timerclear(&pv->pv_vgsats[0]);
	timerclear(&pv->pv_vgsats[1]);

	/* Does this physical volume have a VGDA? */
	if (pv->pv_flags & LVM_NOVGDA)
		return;

	/*
	 * By using a buf from the buffer cache here we reduce the
	 * failure modes of this routine, since we don't have to
	 * allocate memory.
	 */
	lb = geteblk(DEV_BSIZE);

	lvmrec = pv->pv_lvmrec;

	lb->b_dev = makedev(vg->major_num, 0);
	addr = lb->b_un.b_addr;
	lb->b_bcount = DEV_BSIZE;
	lb->b_resid = 0;
	lb->b_vp = NULL;

	/* Read the first VGDA's timestamps. */
	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno = EXT2BLK(vg, pv->pv_num);
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGDA 1 header\n");
		timerclear(&header);
	} else {
		header = ((struct VG_header *)addr)->vg_timestamp;
	}

	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno += (lvmrec->vgda_len - 1);
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGDA 1 trailer\n");
		timerclear(&trailer);
	} else {
		trailer = ((struct VG_trailer *)addr)->vg_timestamp;
	}

	if (!(timercmp(&header, &trailer, !=)))		/* If equal. */
		pv->pv_vgdats[0] = header;

	/* Read the second VGDA's timestamps. */
	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno = EXT2BLK(vg, pv->pv_num) + lvmrec->vgda_len +
						lvmrec->vgsa_len +
	    					lvmrec->mcr_len;
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGDA 2 header\n");
		timerclear(&header);
	} else {
		header = ((struct VG_header *)addr)->vg_timestamp;
	}

	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno += (lvmrec->vgda_len - 1);
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGDA 2 trailer\n");
		timerclear(&trailer);
	} else {
		trailer = ((struct VG_trailer *)addr)->vg_timestamp;
	}
	if (!(timercmp(&header, &trailer, !=)))
		pv->pv_vgdats[1] = header;

	/* Read the first VGSA's timestamps. */
	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno = EXT2BLK(vg, pv->pv_num) + lvmrec->vgda_len;
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGSA 1 header\n");
		timerclear(&header);
	} else {
		header = ((struct vgsaheader *)addr)->sa_h_timestamp;
	}

	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno += (lvmrec->vgsa_len - 1);
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGSA 1 trailer\n");
		timerclear(&trailer);
	} else {
		trailer = ((struct vgsatrailer *)
		   (addr + DEV_BSIZE - sizeof(struct timeval)))->sa_t_timestamp;
	}

	if (!(timercmp(&header, &trailer, !=)))		/* If equal. */
		pv->pv_vgsats[0] = header;

	/* Read the second VGSA's timestamps. */
	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno = EXT2BLK(vg, pv->pv_num) + (2 * lvmrec->vgda_len) +
						lvmrec->vgsa_len +
	    					lvmrec->mcr_len;
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGSA 2 header\n");
		timerclear(&header);
	} else {
		header = ((struct vgsaheader *)addr)->sa_h_timestamp;
	}

	lb->b_flags = B_BUSY|B_READ;
	lb->b_blkno += (lvmrec->vgsa_len - 1);
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
		printf("lv_readvgdats: Could not read VGSA 2 trailer\n");
		timerclear(&trailer);
	} else {
		trailer = ((struct vgsatrailer *)
		   (addr + DEV_BSIZE - sizeof(struct timeval)))->sa_t_timestamp;
	}

	if (!(timercmp(&header, &trailer, !=)))
		pv->pv_vgsats[1] = header;

	brelse(lb);

	return;
}

/*
 * NAME:	lv_readvgda
 *
 * FUNCTION:
 *	Read the VGDA from the specified physical volume.
 *
 * PARAMETERS:
 *	vg  - The volume group structure.
 *	pv  - The physical volume to read the VGDA from.
 *	which - which VGDA to read: 0 - latest, 1 - first, 2 - second.
 *
 * RETURN:
 */

lv_readvgda(vg, pv, which)
struct	volgrp	*vg;		/* Volume group being created.     */
struct	pvol	*pv;		/* Physical volume to read the VGDA from. */
	int	which;		/* Which VGDA to read.		*/
{
struct	buf	*lb;		/* Logical buffer.		   */
	int	error;		/* Error indication.		    */

	/* Does this physical volume have a VGDA? */
	if (pv->pv_flags & LVM_NOVGDA) {
		return(ENOENT);
	}

	/* If the VG does not already have a VGDA, allocate space for one */
	if (vg->vg_vgda == NULL)
		if ((vg->vg_vgda = (char *)kalloc(vg->vgda_len*DEV_BSIZE)) == NULL)
			return(ENOMEM);

	/* Allocate and initialize lvol 0's raw I/O buffer. */
	lb = &(VG_LVOL0(vg)->lv_rawbuf);
	BUF_LOCK(lb);

	lb->b_flags = B_BUSY|B_READ;
	lb->b_dev = makedev(vg->major_num, 0);
	lb->b_un.b_addr = vg->vg_vgda;
	lb->b_bcount = vg->vgda_len*DEV_BSIZE;
	lb->b_resid = 0;
	lb->b_vp = NULL;

	/* Determine which block to read based on the which argument. */
	switch (which) {
	case 0:
		if (timercmp(&pv->pv_vgdats[0], &pv->pv_vgdats[1], >))
			lb->b_blkno =  EXT2BLK(vg, pv->pv_num);
		else
			lb->b_blkno =  EXT2BLK(vg, pv->pv_num) + vg->vgda_len +
					vg->vgsa_len + vg->mcr_len;
		break;
	case 1:
		lb->b_blkno =  EXT2BLK(vg, pv->pv_num);
		break;
	case 2:
		lb->b_blkno =  EXT2BLK(vg, pv->pv_num) + vg->vgda_len +
				vg->vgsa_len + vg->mcr_len;
		break;
	}

	/* Perform the read of the VGDA. */
	if (error = lv_bufio(lb)) {
		/* Log this problem..... */
	}
	lb->b_un.b_addr = NULL;
	/* Free Lvol 0's logical buffer. */
	BUF_UNLOCK(lb);

	vg->vg_maxlvs = VG_head(vg)->maxlvs;
	vg->vg_maxpxs = VG_head(vg)->maxpxs;

	vg->vg_LVentry_off = ROUNDUP(sizeof(struct VG_header), DEV_BSIZE);
	vg->vg_PVentry_off = vg->vg_LVentry_off
		+ ROUNDUP(sizeof(struct LV_entry) * vg->vg_maxlvs, DEV_BSIZE);
	vg->vg_PVentry_len = ROUNDUP(sizeof(struct PV_header) +
			(sizeof(struct PX_entry) * vg->vg_maxpxs), DEV_BSIZE);
	vg->vg_VGtrail_off = (vg->vgda_len - 1) * DEV_BSIZE;

	return(error);
}


/*
 * NAME:	lv_updatevgda
 *
 * FUNCTION:
 *	Update the VGDA on all of the physical volumes in the volume group.
 *
 * PARAMETERS:
 *	vg  - The volume group structure.
 *
 * RETURN:
 */

lv_updatevgda(vg)
struct	volgrp	*vg;			/* Volume group being created.     */
{
int	pvnum;
struct pvol *pv;

	/* Update the timestamps on the VGDA. */
	lv_microtime(&(VG_head(vg)->vg_timestamp));
	VG_trail(vg)->vg_timestamp = VG_head(vg)->vg_timestamp;

	/* Write the VGDA to all of the physical volumes. */
	for (pvnum = 0; pvnum < vg->size_pvols; pvnum++) {
		if ((pv = vg->pvols[pvnum]) == NULL) continue;
		lv_writevgda(vg, pv);
	}
	return(ESUCCESS);
}

/*
 * NAME:	lv_writevgda
 *
 * FUNCTION:
 *	Write the volume group descriptor to a specified physical volume.
 *
 * PARAMETERS:
 *	vg  - The volume group structure.
 *	pv  - The physical volume to write.
 *
 * RETURN:
 */

lv_writevgda(vg, pv)
struct	volgrp	*vg;			/* Volume group being created.     */
struct	pvol	*pv;			/* The physical volume to write.   */
{
struct	buf	*lb;			/* Logical buffer.		   */
	int	error;			/* Error indication.		    */
	int	i;

	/* Does this physical volume have a VGDA? */
	if (pv->pv_flags & (LVM_NOVGDA|LVM_NOTATTACHED))
		return(ESUCCESS);

	/* Allocate and initialize lvol 0's raw I/O buffer. */
	lb = &(VG_LVOL0(vg)->lv_rawbuf);
	BUF_LOCK(lb);

	lb->b_flags = B_BUSY|B_WRITE;
	lb->b_dev = makedev(vg->major_num, 0);
	lb->b_un.b_addr = vg->vg_vgda;
	lb->b_bcount = vg->vgda_len * DEV_BSIZE;
	lb->b_resid = 0;
	lb->b_vp = NULL;

	/*
	 * If the is the only physical volume in the volume group, then write
	 * both copies of the VGDA.
	 */
	if (VG_head(vg)->numpvs == 1) {
		lb->b_blkno = EXT2BLK(vg, pv->pv_num);
		if (error = lv_bufio(lb)) {
			/* Log this problem..... */
			printf("Error writing VGDA [1] (errno = %d)\n", error);
			goto out;
		}
		else {
			pv->pv_vgdats[0] = VG_head(vg)->vg_timestamp;
		}

		lb->b_flags = B_BUSY|B_WRITE;
		lb->b_blkno =  EXT2BLK(vg, pv->pv_num) + vg->vgda_len +
				vg->vgsa_len + vg->mcr_len;
		if (error = lv_bufio(lb)) {
			/* Log this problem..... */
			printf("Error writing VGDA [2] (errno = %d)\n", error);
			goto out;
		}
		else {
			pv->pv_vgdats[1] = VG_head(vg)->vg_timestamp;
		}
	}

	/*
	 * If this is not the only PV, then write the VGDA to the copy with
	 * the earliest timestamp.  If the timestamps are equal, then the
	 * first copy of the VGDA is written.
	 */
	else if (timercmp(&pv->pv_vgdats[0], &pv->pv_vgdats[1], >)) {
		/* Write the second copy of the VGDA. */
		lb->b_blkno =  EXT2BLK(vg, pv->pv_num) + vg->vgda_len +
				vg->vgsa_len + vg->mcr_len;
		if (error = lv_bufio(lb)) {
			/* Log this problem...... */
			printf("Error writing VGDA [3] (errno = %d)\n", error);
		}
		else {
			pv->pv_vgdats[1] = VG_head(vg)->vg_timestamp;
		}
	}
	else {
		/* Write the first copy of the VGDA. */
		lb->b_blkno =  EXT2BLK(vg, pv->pv_num);
		if (error = lv_bufio(lb)) {
			/* Log this problem...... */
			printf("Error writing VGDA [4] (errno = %d)\n", error);
		}
		else {
			pv->pv_vgdats[0] = VG_head(vg)->vg_timestamp;
		}
	}

out:
	lb->b_un.b_addr = NULL;
	/* Free Lvol 0's logical buffer. */
	BUF_UNLOCK(lb);

	return(error);
}
