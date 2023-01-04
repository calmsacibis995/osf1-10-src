
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
static char	*sccsid = "@(#)$RCSfile: lv_lvsubr.c,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/11/06 14:02:59 $";
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
 *	lv_lvsubr.c - logical volume (LV) subroutines.
 *
 * This software is derived from sources containing the following copyright:
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
 *  Modification History:  lv_lvsubr.c
 *
 *  15-MAY-91     Terry Carruthers
 *      ULTRIX/OSF revision 3.2
 *	Added logic to disable and enable Prestoserve accelerated
 *      devices when extents are added or deleted from a logical
 *      volume in the lv_extendlv and lv_reducelv functions.
 *
 *      Added the function lv_deviocget function for Prestoserve
 *      to determine if any of the physical devices supporting
 *      a logical volume are on the QBUS.
 *
 */


#include <sys/types.h>
#include <sys/errno.h>

#include <lvm/lvmd.h>

/* ULTRIX/OSF revision 3.2 */
/*
 * New include files for Prestoserve changes.
 */
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
#include <sys/prestoioctl.h>
#include <sys/presto.h>
#include <io/common/devio.h>

/* Forward declarations of the scheduling switches. */
extern  struct lv_sched lv_reserved_ops;
extern  struct lv_sched lv_sequential_ops;
extern  struct lv_sched lv_parallel_ops;

/* ULTRIX/OSF revision 3.2 */
int lv_prestodisable();
void lv_prestoenable();
extern int prattached;
extern int pr_maj_devnum;


#define LVM_LVFLAGS (LVM_LVDEFINED|LVM_DISABLED|LVM_RDONLY|LVM_NORELOC|\
			LVM_VERIFY|LVM_STRICT|LVM_NOMWC)

int
lv_changelv(dev, vg, clv)
dev_t dev;
struct volgrp *vg;
struct lv_statuslv *clv;
{
int minor_num;
int error = ESUCCESS;

	if (minor(dev) != 0)
		clv->minor_num = minor(dev);

	if (clv->minor_num != minor(clv->minor_num))
		return(EINVAL); /* not a valid minor device number */

	if ((minor_num = clv->minor_num) == 0)
		return(EINVAL); /* Cannot changelv the control device */
	
	if ((clv->maxmirrors < 0) || (clv->maxmirrors > (LVM_MAXCOPIES-1)))
		return(EINVAL);

	if (clv->lv_flags & ~(LVM_LVFLAGS))
		return(EINVAL); /* No 'undefined' bits may be set */

	lock_write(&(vg->vg_lock));

	error = lv_changelv_common(minor_num, vg, clv);

	lock_done(&(vg->vg_lock));

	return(error);
}

int
lv_changelv_common(minor_num, vg, clv)
int minor_num;
struct volgrp *vg;
struct lv_statuslv *clv;
{
struct lvol *lv;
struct LV_entry *LV;
struct extent *exts[LVM_MAXCOPIES], *e;
struct lextent *lext, *le;
int i, j, mapsize, omapsize, lmapsize, olmapsize, copysize, lcopysize;
int error = ESUCCESS;

	if (!(vg->vg_flags & VG_ACTIVATED))
		return(EROFS);		/* cannot 'write' unless activated */

	if ((lv = VG_DEV2LV(vg, minor_num)) == NULL)
		return(ENODEV);		/* Cannot change non-existing lvol */

	if ((clv->sched_strat != LVM_RESERVED)
		&& (clv->sched_strat != LVM_SEQUENTIAL)
		&& (clv->sched_strat != LVM_PARALLEL))
			return(EINVAL);

	lock_write(&(lv->lv_lock));

	/*
	 * Verify that you're not shrinking the size smaller than is used in
	 * any mirror.  Checking mirror 0 is sufficient as mirrors are
	 * compressed down. 
	 */
	if (clv->maxlxs < lv->lv_maxlxs) {
		for (i = clv->maxlxs; i < lv->lv_maxlxs; i++) {
			if (EXTENT(lv, i, 0)->e_pvnum != PX_NOPV) {
				lock_done(&(lv->lv_lock));
				return(EBUSY);
			}
		}
	}

	/*
	 * Verify that any mirrors being deleted are not in use.  We only need
	 * to check the lowest mirror number being removed and mirrors are
	 * compressed
	 */
	if (clv->maxmirrors < lv->lv_maxmirrors) {
		j = clv->maxmirrors + 1;
		for (i = 0; i < lv->lv_maxlxs; i++) {
			if (EXTENT(lv, i, j)->e_pvnum != PX_NOPV) {
				lock_done(&(lv->lv_lock));
				return(EBUSY);
			}
		}
	}

	mapsize = clv->maxlxs * sizeof(extent_t);
	omapsize = lv->lv_maxlxs * sizeof(extent_t);
	if (mapsize > omapsize)
		copysize = omapsize;
	else
		copysize = mapsize;
	lmapsize = clv->maxlxs *sizeof(lextent_t);
	olmapsize = lv->lv_maxlxs * sizeof(lextent_t);
	if (lmapsize > olmapsize)
		lcopysize = olmapsize;
	else
		lcopysize = lmapsize;

	for (i = 0; i < LVM_MAXCOPIES; i++) exts[i] = NULL;
	lext = NULL;

	if (clv->maxlxs != lv->lv_maxlxs) {
		/* If the size changes, reallocate the mirror maps. */
		for (i = 0; i < (clv->maxmirrors + 1); i++) {
			if ((exts[i] = (extent_t *) kalloc(mapsize)) == NULL) {
				/* cleanup on error */
				for (j = 0; j < i; j++)
					KFREE(exts[j], mapsize);
				lock_done(&(lv->lv_lock));
				return(ENOMEM);
			}
			if (lv->lv_exts[i]) {
				e = exts[i];
				bcopy(lv->lv_exts[i], e, copysize);
				e += lv->lv_maxlxs;
				for (j = lv->lv_maxlxs; j < clv->maxlxs; j++) {
					e->e_state = PX_STALE;
					e->e_pvnum = PX_NOPV;
					e++;
				}
			} else {
				e = exts[i];
				for (j = 0; j < clv->maxlxs; j++) {
					e->e_state = PX_STALE;
					e->e_pvnum = PX_NOPV;
					e++;
				}
			}
		}
		/* Reallocate and copy the logical extent map. */
		if ((lext = (lextent_t *) kalloc(lmapsize)) == NULL) {
			for (j = 0; j < (clv->maxmirrors + 1); j++)
				KFREE(exts[j], mapsize);
			lock_done(&(lv->lv_lock));
			return(ENOMEM);
		}
		bcopy(lv->lv_lext, lext, lcopysize);

		/* Zero the remainder of the new area. */
		le = lext + lv->lv_maxlxs;
		for (i = lv->lv_maxlxs; i < clv->maxlxs; i++) {
			le->lx_synctrk = -1;
			le->lx_syncmsk = 0;
			le->lx_syncerr = 0;
			le++;
		}
			
	} else {
		/* If the max mirror count changes, allocate new mirror maps */
		for (i = (lv->lv_maxmirrors+1); i < (clv->maxmirrors+1); i++) {
			if ((exts[i] = (extent_t *)kalloc(mapsize)) == NULL) {
				/* cleanup on error */
				for (j = (lv->lv_maxmirrors+1); j < i; j++)
					KFREE(exts[j], mapsize);
				lock_done(&(lv->lv_lock));
				return(ENOMEM);
			}
			e = exts[i];
			for (j = 0; j < clv->maxlxs; j++) {
				e->e_state = PX_STALE;
				e->e_pvnum = PX_NOPV;
				e++;
			}
		}
	}

	if (vg->vg_vgda == NULL) {
		panic("no vgda!\n");
	}

	/* fetch LV_entry pointer */
	LV = LV_ent(vg, minor_num);
	
	/* Update LV_entry with data from clv */
	LV->maxlxs = clv->maxlxs;
	LV->lv_flags = LVM_LVDEFINED|clv->lv_flags;
	LV->sched_strat = clv->sched_strat;
	LV->maxmirrors = clv->maxmirrors;

	lock_write_to_read(&(vg->vg_lock));

	/* Write VGDA to all attached PV's */
	error = lv_updatevgda(vg);

	if (lv->lv_status & LV_OPEN)
		lv_pause(lv);	/* Wait for all I/O to complete to this lvol */

	lv->lv_flags = (LV->lv_flags & LVM_LVFLAGS)
				| (lv->lv_flags & ~LVM_LVFLAGS);
	lv->lv_sched_strat = clv->sched_strat;

	switch (lv->lv_sched_strat) {
	case LVM_RESERVED:    lv->lv_schedule = &lv_reserved_ops; break;
	case LVM_SEQUENTIAL:  lv->lv_schedule = &lv_sequential_ops; break;
	case LVM_PARALLEL:    lv->lv_schedule = &lv_parallel_ops; break;
	default:
		panic("lv_changelv_common: illegal scheduling strategy\n"); break;
	}

	lv->lv_maxmirrors = clv->maxmirrors;
	lv->lv_maxlxs = clv->maxlxs;

	/* substitute new mirror maps */
	for (i = 0; i < (lv->lv_maxmirrors+1); i++) {
		if (e = exts[i]) {
			exts[i] = lv->lv_exts[i];
			lv->lv_exts[i] = e;
		}
	}
	if (le = lext) {
		lext = lv->lv_lext;
		lv->lv_lext = le;
	}

	if (lv->lv_status & LV_OPEN)
		lv_continue(lv);	/* Restart the logical volume */

	lock_done(&(lv->lv_lock));

	/* free old extent maps */
	for (i = 0; i < LVM_MAXCOPIES; i++) {
		if (exts[i] != NULL)
			KFREE(exts[i], omapsize);
	}

	if (lext)
		KFREE(lext, olmapsize);

	return(error);
}

int
lv_createlv(vg, qlv)
struct volgrp *vg;
struct lv_statuslv *qlv;
{
struct lvol *lv;
struct lvol *lv_alloclv();
int error;
int minor_num;

	minor_num = minor(qlv->minor_num);
	if ((minor_num == 0) || (qlv->minor_num != minor_num))
		return(EINVAL); /* not a valid minor device number */

	lock_write(&(vg->vg_lock));
	
	if (minor_num > vg->vg_maxlvs) {
		lock_done(&(vg->vg_lock));
		return(EDOM);
	}

	if ((lv = VG_DEV2LV(vg, minor_num)) != NULL) {
		lock_done(&(vg->vg_lock));
		return(EEXIST); /* Cannot create already-existing lvol */
	}

	if ((lv = lv_alloclv(vg, minor_num)) == NULL) {
		lock_done(&(vg->vg_lock));
		return(ENOMEM);
	}

	if ((error = lv_changelv_common(minor_num, vg, qlv)) != ESUCCESS) {
		lv_freelv(vg, minor_num);
	}
	lock_done(&(vg->vg_lock));
	return(error);
}

int
lv_deletelv(vg, minor_num)
struct volgrp *vg;
int minor_num;
{
struct lvol *lv;
struct LV_entry *LV;
int m;
int error;

	if (minor_num != minor(minor_num))
		return(EINVAL);	/* not a valid minor device number */

	if (minor_num == 0)
		return(EINVAL);	/* Cannot deletelv the control device */

	lock_write(&(vg->vg_lock));

	if (!(vg->vg_flags & VG_ACTIVATED))
		error = EROFS;		/* cannot 'write' unless activated */
	else if ((lv = VG_DEV2LV(vg, minor_num)) == NULL)
		error = ENODEV;		/* Cannot delete non-existing lvol */
	else {
		lock_write(&(lv->lv_lock));
		if ((lv->lv_status & LV_OPEN) != 0){
			error = EBUSY;	/* Cannot delete open lvol */
			lock_done(&(lv->lv_lock));
		} else {
			if (vg->vg_vgda == NULL) {
				panic("no vgda!\n");
			}
			for (m = 0; m < (lv->lv_maxmirrors+1); m++) {
				lv_dealloc_mirror(vg, lv, m);
			}

			/* fetch LV_entry pointer */
			LV = LV_ent(vg, minor_num);
			
			/* Overwrite LV_entry */
			LV->maxlxs = 0;
			LV->lv_flags = 0;
			LV->sched_strat = 0;
			LV->maxmirrors = 0;

			lock_write_to_read(&(vg->vg_lock));

			/* Write VGDA to all attached PV's */
			error = lv_updatevgda(vg);

			lv->lv_flags = 0;
			lv->lv_sched_strat = 0;
			lv->lv_schedule = NULL;
			lv->lv_maxmirrors = 0;
			lock_done(&(lv->lv_lock));

			lv_freelv(vg, minor_num);

		}
	}
	lock_done(&(vg->vg_lock));

	return (error);
}

int
lv_dealloc_mirror(vg, lv, m)
struct volgrp *vg;
struct lvol *lv;
int m;
{
struct extent *e;
struct PX_entry *PX;
int pvnum, lx;

	for (e = lv->lv_exts[m], lx = 0; lx < lv->lv_maxlxs; lx++, e++) {
		if ((pvnum = e->e_pvnum) != PX_NOPV) {
			vg->pvols[pvnum]->pv_freepxs++;
			PX = PX_ent(vg,pvnum,e->e_pxnum);
			if ((VG_DEV2LV(vg, PX->lv_index) == lv) 
					&& (PX->lx_num == lx)) {
				PX->lv_index = 0;
				PX->lx_num = 0;
			} else {
				panic("lv_dealloc_mirror");
			}
		}
	}
	return;
}

int
lv_extendlv(dev, vg, arg)
dev_t dev;
struct volgrp *vg;
struct lv_lvsize *arg;
{
struct	lvol	*lv;
struct	lxmap	*lxmap;
struct	lxmap	*lx;
struct	extent	*extents;
struct	extent	*ext;
struct	extent	*t_ext;
struct	sa_px_info *px_info;
struct	sa_px_info *pxi;
struct	pvol *pv;
	int	lxnum, pvnum, pxnum;
	int	i, j;
	int	error = ESUCCESS;

/* ULTRIX/OSF revision 3.2 */
int prestodisabled = FALSE;

	if (minor(dev) != 0) {
		arg->minor_num = minor(dev);
	}

	/* Check the argument array. */
	if ((arg->minor_num == 0) || (arg->minor_num != minor(arg->minor_num)))
		return(EINVAL); /* Improper minor device number */
	if (arg->size > LVM_MAXLXS)
		return(E2BIG);
	
	lock_write(&(vg->vg_lock));

	/* Cannot extend unless the volume group is activated. */
	if ((vg->vg_flags & VG_ACTIVATED) == 0) {
		lock_done(&(vg->vg_lock));
		return(EROFS);
	}
	
	/* Check the logical volume. */
	if ((lv = VG_DEV2LV(vg, arg->minor_num)) == NULL) {
		lock_done(&(vg->vg_lock));
		return(ENODEV);
	}

	/* Allocate memory to copy in the logical extent map. */
	if ((lxmap = (lxmap_t *)kalloc(arg->size * sizeof(lxmap_t))) == NULL) {
		lock_done(&(vg->vg_lock));
		return(ENOMEM);
	}

	/* Allocate the physical extent stale configuration structure. */
	px_info = (sa_px_info_t *)kalloc((arg->size+1)* sizeof(sa_px_info_t));
	if (px_info == NULL) {
		lock_done(&(vg->vg_lock));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		return(ENOMEM);
	}

	/* Allocate space for the actual extent structures. */
	extents = (extent_t *)kalloc(arg->size * sizeof(extent_t));
	if (extents == NULL) {
		lock_done(&(vg->vg_lock));
		KFREE(px_info, (arg->size+1) * sizeof(sa_px_info_t));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		return (error);
	}

	/* Copy in the logical extent map from user space. */
	error = copyin(arg->extents, lxmap, arg->size * sizeof(lxmap_t));
	if (error != ESUCCESS) {
		lock_done(&(vg->vg_lock));
		KFREE(px_info, (arg->size+1) * sizeof(sa_px_info_t));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		KFREE(extents, arg->size * sizeof(extent_t));
		return (error);
	}

	lock_write(&(lv->lv_lock));
	/*
	 * Check the entire lx map.  If there are any bad entries, then the
	 * whole thing is bad.
	 */
	lx = &lxmap[0];
	pxi = &px_info[0];
	ext = &extents[0];
	for (i = 0; (i < arg->size) && (error == ESUCCESS); i++, lx++) {
		pvnum = lx->pv_key;
		pxnum = lx->px_num;
		lxnum = lx->lx_num;
		if (lxnum > lv->lv_maxlxs)
			error = ENOSPC;
		else if ((pvnum > vg->size_pvols)
			|| ((pv = vg->pvols[pvnum]) == NULL))
			error = ENODEV;
		else if (pxnum > pv->pv_pxcount)
			error = EXDEV;
		else if (pv->pv_flags & LVM_PVNOALLOC)
			error = EADDRNOTAVAIL;
		else if (PX_ent(vg, pvnum, pxnum)->lv_index != 0)
			error = EBUSY;
		else {
			for (j = 0; j < lv->lv_maxmirrors+1; j++) {
				t_ext = EXTENT(lv, lxnum, j);
				if (t_ext->e_pvnum == PX_NOPV)
					break;
			}
			if (j == lv->lv_maxmirrors+1) {/* Room not found. */
				error = ENOSPC;
				break;
			}
		}
		if (error == ESUCCESS) {
			ext->e_pvnum = pvnum;
			ext->e_pxnum = pxnum;
			ext->e_state = 0;
			if (EXTENT(lv, lxnum, 0)->e_pvnum != PX_NOPV) {
				pxi->pxi_pvnum = pvnum;
				pxi->pxi_pxnum = pxnum;
				ext->e_state |= PX_STALE;
				pxi++;
			}
			ext++;
		}
	}
	if (error != ESUCCESS) {
		lock_done(&(lv->lv_lock));
		lock_done(&(vg->vg_lock));
		KFREE(px_info, (arg->size+1) * sizeof(sa_px_info_t));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		KFREE(extents, arg->size * sizeof(extent_t));
		return(error);
	}

	/* Terminate the PXI array. */
	pxi->pxi_pvnum = PX_NOPV;
	pxi->pxi_pxnum = 0;

        /* ULTRIX/OSF revision 3.2 */
        /* Check if we need to disable Prestoserve caching
         * for this logical volume device
         */

        if (lv->lv_status & LV_OPEN) {
	    prestodisabled = lv_prestodisable(dev);
        }

	/* Pause the logical volume. */
	if (lv->lv_status & LV_OPEN)
		lv_pause(lv);

	/*
	 * For each logical extent, convert it into an internal extent, add
	 * the internal extent to the logical volume and mark the physical
	 * extent as in use.
	 */

	/* Add all of the extents. */
	lx = &lxmap[0];
	ext = &extents[0];
	for (i = 0; i < arg->size; i++, lx++, ext++) {
		uchar_t  pvnum = lx->pv_key;
		ushort_t pxnum = lx->px_num;
		ushort_t lxnum = lx->lx_num;

		lv_addextent(lv, ext, lxnum, ANYMIRROR);
		vg->pvols[pvnum]->pv_freepxs--;
		PX_ent(vg, pvnum, pxnum)->lv_index = arg->minor_num;
		PX_ent(vg, pvnum, pxnum)->lx_num = lxnum;
	}

        /* ULTRIX/OSF revision 3.2 */
        /* Now check if we need to re-enable Prestoserve caching
         * for this logical volume device
         */
        if (prestodisabled)
	    lv_prestoenable(dev);

	/* Free the copy of the extent map and the extent array. */
	KFREE(lxmap, arg->size * sizeof(lxmap_t));
	KFREE(extents, arg->size * sizeof(extent_t));

	/* Update the status area manager - this can not fail. */
	lv_sa_config(vg, CNFG_STALEPX, px_info);
	KFREE(px_info, (arg->size+1) * sizeof(sa_px_info_t));

	if (lv->lv_status & LV_OPEN)
		lv_continue(lv);

	lock_done(&(lv->lv_lock));
	lock_write_to_read(&(vg->vg_lock));

	/* Write the new VGDA (which was produced above) to the pvols. */
	error = lv_updatevgda(vg);

	lock_done(&(vg->vg_lock));

	return(error);
}

int
lv_lvoption(dev, vg, cmd, arg)
dev_t dev;
struct volgrp *vg;
int cmd;
struct lv_option *arg;
{
ushort_t minor_num;
int error = ESUCCESS;
struct lvol *lv;

	minor_num = minor(dev);

	lock_read(&(vg->vg_lock));

	if ((lv = VG_DEV2LV(vg, minor_num)) == NULL)
		panic("lv_lvoption");

	lock_write(&(lv->lv_lock));

	switch (cmd) {
	case LVM_OPTIONGET:
		arg->opt_avoid = lv->lv_rawavoid;
		arg->opt_options = lv->lv_rawoptions;
		break;
	case LVM_OPTIONSET:
		if ((arg->opt_avoid & ~LVM_MIRAVOID)
			|| (arg->opt_options & ~(LVM_VERIFY|LVM_NORELOC))) {
			error = EINVAL;
		} else {
			lv->lv_rawavoid = arg->opt_avoid;
			lv->lv_rawoptions = arg->opt_options;
		}
		break;
	}

	lock_done(&(lv->lv_lock));
	lock_done(&(vg->vg_lock));

	return(error);
}

int
lv_querylv(dev, vg, qlv)
dev_t dev;
struct volgrp *vg;
struct lv_querylv *qlv;
{
struct lvol *lv;
struct extent *ext;
int error = ESUCCESS;

	if (minor(dev) != 0)
		qlv->minor_num = minor(dev);

	lock_read(&(vg->vg_lock));

	if ((qlv->minor_num == 0) || (qlv->minor_num != minor(qlv->minor_num)))
		error = EINVAL;		/* Cannot querylv the control device */
	else if (!(vg->vg_flags & VG_ACTIVATED))
		error = ENXIO;		/* Cannot query unless VG activated */
	else if ((lv = VG_DEV2LV(vg, qlv->minor_num)) != NULL) {
		/* copy information from the struct lvol */
		lock_read(&(lv->lv_lock));
		qlv->lv_flags = lv->lv_flags;
		qlv->sched_strat = lv->lv_sched_strat;
		qlv->maxmirrors = lv->lv_maxmirrors;
		qlv->maxlxs = lv->lv_maxlxs;
		qlv->numpxs = lv->lv_curpxs;
		qlv->numlxs = lv->lv_curlxs;
		lock_done(&(lv->lv_lock));
	} else {
		qlv->lv_flags = 0;	/* !LVM_LVDEFINED */
		qlv->sched_strat = 0;
		qlv->maxmirrors = 0;
		qlv->maxlxs = 0;
		qlv->numpxs = 0;
		qlv->numlxs = 0;
	}
	lock_done(&(vg->vg_lock));
	return(error);
}

int
lv_querylvmap(dev, vg, qlv)
dev_t dev;
struct volgrp *vg;
struct lv_lvsize *qlv;
{
struct lvol *lv;
struct lxmap xmap;
int error = 0;
int i, lxnum, maxlxs, numexts;
struct extent *exts, *ext;
int remaining;
struct lxmap *uext;
daddr_t ustart;

	if (minor(dev) != 0)
		qlv->minor_num = minor(dev);

	if ((qlv->minor_num == 0) || (qlv->minor_num != minor(qlv->minor_num)))
		return(EINVAL);	/* Improper minor device number */

	lock_read(&(vg->vg_lock));
	
	numexts = 0;
	if ((lv = VG_DEV2LV(vg, qlv->minor_num)) == NULL) {
		qlv->size = numexts;
		lock_done(&(vg->vg_lock));
		return(ESUCCESS);
	}
	lock_read(&(lv->lv_lock));
	maxlxs = lv->lv_maxlxs;
	remaining = qlv->size;
	uext = qlv->extents;
	for (lxnum = 0; !error && remaining && (lxnum < maxlxs); lxnum++) {
		xmap.lx_num = lxnum;
		for (i = 0; i < LVM_MAXCOPIES; i++) {
			if (!(exts = lv->lv_exts[i])) break;

			ext = exts+lxnum;
			if (ext->e_pvnum != PX_NOPV) {
				xmap.pv_key = ext->e_pvnum;
				xmap.px_num = ext->e_pxnum;
				xmap.status = ext->e_state & LVM_PXSTALE;
				if ((vg->pvols[ext->e_pvnum])->pv_flags
							& LVM_PVMISSING)
					xmap.status |= LVM_PXMISSING;

				if (error = copyout(&xmap, uext, sizeof(xmap)))
					break;
				uext++;
				numexts++;
				remaining--;
			}
		}
	}
	qlv->size = numexts;

	lock_done(&(lv->lv_lock));
	lock_done(&(vg->vg_lock));

	return(error);	/* ESUCCESS, EFAULT */
}

int
lv_realloclv(vg, arg)
struct volgrp *vg;
struct lv_realloclv *arg;
{
struct	lvol		*slv;
struct	lvol		*dlv;
struct	lxmap		*lxmap;
struct	lxmap		*lx;
struct	extent		*extents;
struct	extent		*ext;
struct	extent		t_ext;
struct	extent		*s_ext;
struct	extent		*d_ext;
struct	sa_px_info	*px_info;
struct	sa_px_info	*pxi;
struct 	PX_entry	*PX_e;
	uchar_t		pvnum;
	ushort_t	pxnum;
	ushort_t	lxnum;
	int		i, j;
	int		error = ESUCCESS;

	/* Check the argument array. */
	if ((arg->sourcelv == 0) || (arg->sourcelv != minor(arg->sourcelv))
		|| (arg->destlv == 0) || (arg->destlv != minor(arg->destlv))
		|| (arg->sourcelv == arg->destlv))
		return(EINVAL);

	if (arg->size > LVM_MAXLXS)
		return(E2BIG);

	lock_write(&(vg->vg_lock));

	/* Cannot extend unless the volume group is activated. */
	if ((vg->vg_flags & VG_ACTIVATED) == 0) {
		lock_done(&(vg->vg_lock));
		return(EROFS);
	}

	/* Check the logical volumes. */
	if (((slv = VG_DEV2LV(vg, arg->sourcelv)) == NULL) ||
	    ((dlv = VG_DEV2LV(vg, arg->destlv)) == NULL)) {
		lock_done(&(vg->vg_lock));
		return(ENODEV);
	}

	/* Allocate memory to copy in the logical extent map. */
	if ((lxmap = (lxmap_t *)kalloc(arg->size * sizeof(lxmap_t))) == NULL) {
		lock_done(&(vg->vg_lock));
		return(ENOMEM);
	}

	/* Allocate the physical extent stale configuration structure. */
	px_info = (sa_px_info_t *)kalloc(arg->size * sizeof(sa_px_info_t));
	if (px_info == NULL) {
		lock_done(&(vg->vg_lock));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		return(ENOMEM);
	}

	/* Allocate space for the actual extent structures. */
	extents = (extent_t *)kalloc(arg->size * sizeof(extent_t));
	if (extents == NULL) {
		lock_done(&(vg->vg_lock));
		KFREE(px_info, arg->size * sizeof(sa_px_info_t));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		return (error);
	}

	/* Copy in the logical extent map from user space. */
	error = copyin(arg->extents, lxmap, arg->size * sizeof(lxmap_t));
	if (error != ESUCCESS) {
		lock_done(&(vg->vg_lock));
		KFREE(px_info, arg->size * sizeof(sa_px_info_t));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		KFREE(extents, arg->size * sizeof(extent_t));
		return (error);
	}

	/*
	 * Lock the source and the destination extent logical volumes.  There
	 * is no deadlock problem as this is the only place where two logical
	 * volumes are locked at the same time, and the VG lock must be taken
	 * to get here.
	 */
	lock_write(&(slv->lv_lock));
	lock_write(&(dlv->lv_lock));

	/*
	 * Check the entire lx map.  If there are any bad entries, then the
	 * whole thing is bad.
	 */
	lx = &lxmap[0];
	pxi = &px_info[0];
	ext = &extents[0];
	for (i = 0; (i < arg->size) && (error == ESUCCESS); i++, lx++) {

		/* Check the extent against the volume group. */
		if (lx->pv_key > vg->size_pvols)
			error = ENODEV;
		else if (lx->px_num > vg->pvols[lx->pv_key]->pv_pxcount)
			error = EXDEV;

		/* Check the source, to make sure the extent can be removed */
		else if (lx->lx_num > slv->lv_maxlxs)
			error = ENOSPC;
		else {
			for (j = 0; j < slv->lv_maxmirrors+1; j++) {
				s_ext = EXTENT(slv, lx->lx_num, j);
				if (s_ext->e_pvnum == lx->pv_key &&
				    s_ext->e_pxnum == lx->px_num)
					break;
			}
			if (j == slv->lv_maxmirrors+1) {/* Extent not found. */
				error = ENOENT;
				break;
			}
		}

		/* Check the destination, make sure the extent can be added */
		if (lx->lx_num >= dlv->lv_maxlxs)
			error = ENOSPC;
		else {
			for (j = 0; j < dlv->lv_maxmirrors+1; j++) {
				s_ext = EXTENT(dlv, lx->lx_num, j);
				if (s_ext->e_pvnum == PX_NOPV)
					break;
			}
			if (j == dlv->lv_maxmirrors+1) {/* Room not found. */
				error = ENOSPC;
				break;
			}
		}
		if (error == ESUCCESS) {
			ext->e_pvnum = lx->pv_key;
			ext->e_pxnum = lx->px_num;
			ext->e_state = 0;
			if (EXTENT(dlv, lx->lx_num, 0)->e_pvnum != PX_NOPV) {
				pxi->pxi_pvnum = lx->pv_key;
				pxi->pxi_pxnum = lx->px_num;
				ext->e_state |= PX_STALE;
				pxi++;
			}
			ext++;
		}
	}
	if (error != ESUCCESS) {
		lock_done(&(dlv->lv_lock));
		lock_done(&(slv->lv_lock));
		lock_done(&(vg->vg_lock));
		KFREE(px_info, arg->size * sizeof(sa_px_info_t));
		KFREE(lxmap, arg->size * sizeof(lxmap_t));
		KFREE(extents, arg->size * sizeof(extent_t));
		return(error);
	}

	/* Terminate the PXI array. */
	pxi->pxi_pvnum = PX_NOPV;
	pxi->pxi_pxnum = 0;

	/* Pause the logical volumes. */
	if (slv->lv_status & LV_OPEN)
		lv_pause(slv);
	if (dlv->lv_status & LV_OPEN)
		lv_pause(dlv);

	/*
	 * Mark each logical extent that is being deallocated so 
	 * that new I/O will avoid putting a mirror consistency cache
	 * on this pvol.
	 */
	lx = &lxmap[0];
	for (i = 0; i < arg->size; i++, lx++) {

		pvnum = lx->pv_key;
		pxnum = lx->px_num;
 		lxnum = lx->lx_num;
		for (j = 0; j < (slv->lv_maxmirrors + 1); j++) {
			s_ext = EXTENT(slv, lxnum, j);
			if ((s_ext->e_pvnum == pvnum) &&
			    (s_ext->e_pxnum == pxnum)) {
				/* Mark it as going away.
				 * We compact the list here by shuffling the
				 * extent toward the end.
				 */
				s_ext->e_state |= PX_NOMWC;
				if (j < slv->lv_maxmirrors) {
					d_ext = EXTENT(slv,lxnum,j+1);
					if (d_ext->e_pvnum != PX_NOPV) {
						t_ext = *s_ext;
						*s_ext = *d_ext;
						*d_ext = t_ext;
					}
				}
				if (j == slv->lv_maxmirrors) break;
			}
		}
		if (j == slv->lv_maxmirrors+1)
			panic("lv_realloclv extmap");
	}

	/* Let the Mirror Consistency Manager clean out this lvol */
	lv_cache_clean(vg, arg->sourcelv);

	/*
	 * Remove the deallocated extents from the in-memory extent
	 * maps. All of them must have previously been marked PX_NOMWC
	 * or we've blown something here.
	 */
	lx = &lxmap[0];
	for (i = 0; i < arg->size; i++, lx++) {

		pvnum = lx->pv_key;
		pxnum = lx->px_num;
		lxnum = lx->lx_num;

		PX_e = PX_ent(vg, pvnum, pxnum);
		/* Remove the extent from the source logical volume. */
		for (j = 0; j < (slv->lv_maxmirrors + 1); j++) {
			s_ext = EXTENT(slv, lxnum, j);
			if ((s_ext->e_pvnum == pvnum) &&
			    (s_ext->e_pxnum == pxnum)) {
				s_ext->e_pvnum = PX_NOPV;
				slv->lv_curpxs--;
				if (j == 0) slv->lv_curlxs--;
				break;
			}
		}

		/* Add the extent to the destination logical volume. */
		lv_addextent(dlv, ext, lxnum, ANYMIRROR);
		PX_e->lv_index = arg->destlv;
		PX_e->lx_num = lxnum;
	}

	/* Free the copy of the extent map and the extent array. */
	KFREE(lxmap, arg->size * sizeof(lxmap_t));
	KFREE(extents, arg->size * sizeof(extent_t));

	/* Update the status area manager - this can not fail. */
	lv_sa_config(vg, CNFG_STALEPX, px_info);
	KFREE(px_info, arg->size * sizeof(sa_px_info_t));

	/* Reset I/O on the source and destination logical volumes. */
	if (slv->lv_status & LV_OPEN)
		lv_continue(slv);
	if (dlv->lv_status & LV_OPEN)
		lv_continue(dlv);

	lock_done(&(slv->lv_lock));
	lock_done(&(dlv->lv_lock));
	lock_write_to_read(&(vg->vg_lock));

	/* Write the new VGDA (which was produced above) to the pvols. */
	error = lv_updatevgda(vg);

	lock_done(&(vg->vg_lock));

	return(error);
}

int
lv_reducelv(dev, vg, arg)
dev_t dev;
struct volgrp *vg;
struct lv_lvsize *arg;
{
struct	lvol		*lv;
struct	lxmap		*lxmap;
struct	lxmap		*lx;
struct	extent		*s_ext;
struct	extent		*d_ext;
struct	extent		t_ext;
struct 	PX_entry	*PX;
	uchar_t		pvnum;
	ushort_t	pxnum;
	ushort_t	lxnum;
	int		mirror;
	int		i;
	int		error = ESUCCESS;
	int		mapsize;
	int		minor_num;

/* ULTRIX/OSF revision 3.2 */
int prestodisabled = FALSE;

	if (minor(dev) != 0) {
		arg->minor_num = minor(dev);
	}

	/* Check the logical volume. */
	if (((minor_num = arg->minor_num) == 0)
			|| (minor_num != minor(minor_num)))
		return(EINVAL);		/* Cannot reduce the control device */
	
	if(arg->size > LVM_MAXLXS)
		return(E2BIG);

	lock_write(&(vg->vg_lock));

	/* Cannot extend unless the volume group is activated. */
	if ((vg->vg_flags & VG_ACTIVATED) == 0) {
		lock_done(&(vg->vg_lock));
		return(EROFS);
	}
	
	if ((lv = VG_DEV2LV(vg, minor_num)) == NULL) {
		lock_done(&(vg->vg_lock));
		return(ENODEV);
	}
	lock_write(&(lv->lv_lock));

	mapsize = arg->size * sizeof(lxmap_t);
	/* Allocate memory to copy in the logical extent map. */
	if ((lxmap = (lxmap_t *)kalloc(mapsize)) == NULL) {
		lock_done(&(lv->lv_lock));
		lock_done(&(vg->vg_lock));
		return(ENOMEM);
	}

	/* Copy in the logical extent map from user space. */
	if ((error = copyin(arg->extents, lxmap, mapsize)) != ESUCCESS) {
		lock_done(&(lv->lv_lock));
		lock_done(&(vg->vg_lock));
		return (error);
	}

	/*
	 * Check the entire lx map.  If there are any bad entries, then the
	 * whole thing is bad.
	 */
	lx = &lxmap[0];
	for (i = 0; (i < arg->size) && (error == ESUCCESS); i++, lx++) {
		pvnum = lx->pv_key;
		pxnum = lx->px_num;
		lxnum = lx->lx_num;
		if (lxnum > lv->lv_maxlxs)
			error = ENOENT;
		else {
			for (mirror = 0; mirror <= lv->lv_maxmirrors;
								mirror++) {
				s_ext = EXTENT(lv, lxnum, mirror);
				if ((s_ext->e_pvnum == pvnum) &&
				    (s_ext->e_pxnum == pxnum))
					break;
			}
			if (mirror > lv->lv_maxmirrors) {
				/* Extent not found. */
				error = ENOENT;
			}
		}
	}
	if (error != ESUCCESS) {
		lock_done(&(lv->lv_lock));
		lock_done(&(vg->vg_lock));
		KFREE(lxmap, mapsize);
		return(error);
	}
	/*
	 * At this point we are committed.
	 */

        /* ULTRIX/OSF revision 3.2 */
        /* Check if we need to disable Prestoserve caching
         * for this logical volume device
         */
        if (lv->lv_status & LV_OPEN) {
	    prestodisabled = lv_prestodisable(dev);
        }

	/* 
	 * Wait for all I/O to complete
	 */
	if (lv->lv_status & LV_OPEN)
		lv_pause(lv);

	/*
	 * Mark each logical extent that is being deallocated so 
	 * that new I/O will avoid putting a mirror consistency cache
	 * on this pvol. Also change the in-memory VGDA to reflect
	 * the new allocations.
	 */
	lx = &lxmap[0];
	for (i = 0; i < arg->size; i++, lx++) {

		pvnum = lx->pv_key;
		pxnum = lx->px_num;
 		lxnum = lx->lx_num;

		/* If it's not allocated to me, something is seriously wrong */
		PX = PX_ent(vg, pvnum, pxnum);
		if (PX->lv_index != minor_num)
			panic("lv_reducelv");

		PX->lv_index = 0;
		PX->lx_num = 0;
		vg->pvols[pvnum]->pv_freepxs++;

		for (mirror = 0; mirror <= lv->lv_maxmirrors; mirror++) {
			s_ext = EXTENT(lv, lxnum, mirror);
			if ((s_ext->e_pvnum == pvnum) &&
			    (s_ext->e_pxnum == pxnum)) {
				/* Mark it as going away.
				 * We compact the list here by shuffling the
				 * extent toward the end.
				 */
				s_ext->e_state |= PX_NOMWC;
				if (mirror < lv->lv_maxmirrors) {
					d_ext = EXTENT(lv,lxnum,mirror+1);
					if (d_ext->e_pvnum != PX_NOPV) {
						t_ext = *s_ext;
						*s_ext = *d_ext;
						*d_ext = t_ext;
					}
				}
				if (mirror == lv->lv_maxmirrors)
					break;
			}
		}
		if (mirror > lv->lv_maxmirrors)
			panic("lv_reducelv extmap");
	}

	/* Let the Mirror Consistency Manager clean out this lvol */
	lv_cache_clean(vg, minor_num);

        /* ULTRIX/OSF revision 3.2 */
        /* Now check if we need to re-enable Prestoserve caching
         * for this logical volume device
         */
        if (prestodisabled)
	    lv_prestoenable(dev);

	/* Restart I/O to this lvol */
	if (lv->lv_status & LV_OPEN)
		lv_continue(lv);

	lock_write_to_read(&(vg->vg_lock));
	/*
	 * Write the new VGDA (which was produced above) to the pvols
	 */
	error = lv_updatevgda(vg);

	/*
	 * Pause the lvol once again.
	 */
	if (lv->lv_status & LV_OPEN)
		lv_pause(lv);

	/*
	 * Remove the deallocated extents from the in-memory extent
	 * maps. All of them must have previously been marked PX_NOMWC
	 * or we've blown something here.
	 */
	lx = &lxmap[0];
	for (i = 0; i < arg->size; i++, lx++) {

		pvnum = lx->pv_key;
		pxnum = lx->px_num;
		lxnum = lx->lx_num;
		for (mirror = 0; mirror <= lv->lv_maxmirrors; mirror++) {
			s_ext = EXTENT(lv, lxnum, mirror);
			if ((s_ext->e_pvnum == pvnum) &&
			    (s_ext->e_pxnum == pxnum)) {
				ASSERT(s_ext->e_state & PX_NOMWC);
				s_ext->e_pvnum = PX_NOPV;
				lv->lv_curpxs--;
				if (mirror == 0) lv->lv_curlxs--;
				break;
			}
		}
	}

	/* Restart I/O to this lvol */
	if (lv->lv_status & LV_OPEN)
		lv_continue(lv);

	lock_done(&(lv->lv_lock));
	lock_done(&(vg->vg_lock));

	/* Free the copy of the extent array. */
	KFREE(lxmap, mapsize);

	/* The error return is from update_vgda */
	return(error);
}

int
lv_resynclx(dev, vg, arg)
dev_t dev;
register struct volgrp *vg;
register struct lv_resynclx *arg;
{
	int minor_num;
	int lxnum;
	int error;
	struct lvol *lv;
	register struct buf *lb;
	int s;

	if (minor(dev) != 0)
		arg->minor_num = minor(dev);

	if (((minor_num = arg->minor_num) == 0)
			|| (minor_num != minor(minor_num)))
		return(EINVAL);	/* Improper minor number */

	/*
	 * We can't check arg->lx_num without holding a lock on
	 * the appropriate lvol. Rather than do that, we initiate
	 * the I/O, and let the normal I/O error checking 
	 * either succeed or fail when the time comes.
	 */
	lxnum = arg->lx_num;

	if ((error = lv_lvhold(vg, dev, minor_num, &lv)) != ESUCCESS) {
		return(error);
	}

	/* Acquire all the necessary resources */
	lb = &lv->lv_rawbuf;
	s = splbio(); BUF_LOCK(lb); splx(s);

	if ((lb->b_un.b_addr = (caddr_t)kalloc(BYTEPTRK)) != NULL) {
		/*
		 * Do resync operation.
		 */
		error = lv_syncx(vg, minor_num, lb, lxnum);

		KFREE(lb->b_un.b_addr, BYTEPTRK);
	} else {
		error = ENOMEM;
	}
	s = splbio(); BUF_UNLOCK(lb); splx(s);
	lv_lvrelease(vg, dev, lv);

	return(error);
}

int
lv_resynclv(dev, vg, minor_num)
dev_t dev;
register struct volgrp *vg;
register int minor_num;
{
	int lxnum;
	int error;
	struct lvol *lv;
	register struct buf *lb;
	int s;

	if (minor(dev) != 0)
		minor_num = minor(dev);

	if ((minor_num == 0) || (minor_num != minor(minor_num)))
		return(EINVAL);	/* Cannot resync LVOL 0 */

	if ((error = lv_lvhold(vg, dev, minor_num, &lv)) != ESUCCESS) {
		return(error);
	}

	/* Acquire all the necessary resources */
	lb = &lv->lv_rawbuf;
	s = splbio(); BUF_LOCK(lb); splx(s);

	if ((lb->b_un.b_addr = (caddr_t)kalloc(BYTEPTRK)) != NULL) {
		/*
		 * Do resync operations.
		 */
		for (lxnum = 0; lxnum < lv->lv_maxlxs; lxnum++) {
			if ((error = lv_syncx(vg, minor_num, lb, lxnum))
				!= ESUCCESS) break;
		}

		KFREE(lb->b_un.b_addr, BYTEPTRK);
	} else {
		error = ENOMEM;
	}
	s = splbio(); BUF_UNLOCK(lb); splx(s);
	lv_lvrelease(vg, dev, lv);

	return(error);
}

int
lv_resyncpv(vg, pvnum)
register struct volgrp *vg;
register int pvnum;
{
	int minor_num = 0;
	int pxnum, lxnum;
	int error;
	struct lvol *lv;
	struct pvol *pv;
	struct PX_entry *PX;
	register struct buf *lb;
	int s;

	lock_read(&(vg->vg_lock));

	if ((pvnum > vg->size_pvols) || ((pv = vg->pvols[pvnum]) == NULL)) {
		lock_done(&(vg->vg_lock));
		return(EINVAL);
	}
	if (pv->pv_flags & (LVM_NOTATTACHED|LVM_PVMISSING)) {
		lock_done(&(vg->vg_lock));
		return(ENXIO);
	}

	if ((lb->b_un.b_addr = (caddr_t)kalloc(BYTEPTRK)) != NULL) {
		lock_done(&(vg->vg_lock));
		return(ENOMEM);
	}

	for (pxnum = 0, PX = PX_ent(vg,pvnum,0);
				 pxnum < pv->pv_pxcount; pxnum++, PX++) {
		if (PX->lv_index == 0) continue;
		if (PX->lv_index != minor_num) {
			if (minor_num) {
				lv_lvrelease(vg,makedev(vg->major_num,0), lv);
			}
			minor_num = PX->lv_index;
			if ((error = lv_lvhold(vg, makedev(vg->major_num,0),
					minor_num, &lv))
				!= ESUCCESS) {
				break;
			}
		}
		lb = &lv->lv_rawbuf;
		s = splbio(); BUF_LOCK(lb); splx(s);
		lxnum = PX->lx_num;
		/*
		 * Do resync operation.
		 */
		if ((error = lv_syncx(vg, minor_num, lb, lxnum)) != ESUCCESS)
			break;

		s = splbio(); BUF_UNLOCK(lb); splx(s);
	}
	if (minor_num) {
		lv_lvrelease(vg,makedev(vg->major_num,0), lv);
	}

	KFREE(lb->b_un.b_addr, BYTEPTRK);
	lock_done(&(vg->vg_lock));
	return(error);
}

int
lv_syncx(vg, minor_num, lb, lxno)
register struct volgrp *vg;
ushort_t minor_num;
register struct buf *lb;
int lxno;
{
	int blkno, lastblk;
	int requested, done, error;

	lb->b_proc = NULL;
	lb->b_vp = NULL;
	lb->b_dev = makedev(vg->major_num, minor_num);
	lb->av_forw = NULL;

	lastblk = EXT2BLK(vg,lxno+1);
	for (blkno = EXT2BLK(vg,lxno); blkno < lastblk; blkno += TRK2BLK(1)) {
		event_clear(&lb->b_iocomplete);
		lb->b_flags = B_PRIVATE|B_READ;
		lb->b_blkno = blkno;
		lb->b_bcount = BYTEPTRK;
		lb->b_options = LVM_RESYNC_OP;
		requested = lb->b_bcount;

		/* Perform the read portion of the sync */
		lv_strategy(lb);

		error = biowait(lb);
		done = lb->b_bcount - lb->b_resid;
		if (!error && (requested != done))
			error = EIO;
		if (error) break;
	}
	return (error);
}

/*
 * NAME:	lv_alloclv
 *
 * FUNCTION:
 *	Allocate an lvol structure for the logical device specified in the
 * minor number of the device specified.  If there is no lvol table, this will
 * allocate one.
 *
 * PARAMETERS:
 *	vg - The volume group.
 *	dev - dev_t (major,minor) of logical volume to be allocated
 *
 * RETURN VALUE:
 *	NULL if this routine is unable to allocate memory.
 */

thread_callq_t lv_threadq;
int lv_thread_initialized = 0;

struct lvol *
lv_alloclv(vg, dev)
	dev_t	dev;		/* Device number (major,minor) of LV.	*/
struct	volgrp	*vg;		/* Volume group to look up LV in.	*/
{
	int	mindev;		/* The minor device number.		*/
struct	lvol	**lvols;	/* Temporary lvols array.		*/
struct	lvol	*lvtmp;		/* Pointer to lvol struct.		*/
struct	extent	*ext;
struct	lextent *le;
	int	i;

	/*
	 * First check to see if the array is large enough, if it isn't then
	 * allocate more space.
	 */
	mindev = minor(dev);
	if (mindev >= vg->num_lvols) { 
		lvols = (struct lvol **)kalloc(sizeof(struct lvol *) *
					       (mindev + 1));
		if (lvols == NULL)
			return(NULL);
		/* NOTE: depends on all-bits-zero NULL pointer */
		bzero(lvols, sizeof(struct lvol *) * (mindev + 1));
		for (i = 0; i < vg->num_lvols; i++)
			lvols[i] = vg->lvols[i];
		KFREE(vg->lvols, sizeof(struct lvol *) * vg->num_lvols);
		vg->lvols = lvols;
		vg->num_lvols = mindev + 1;
	}

	/*
	 * Allocate the logical volume structure
	 */
	if (vg->lvols[mindev] == NULL) {
		if ((lvtmp = NEW(struct lvol)) == NULL) {
			return(NULL);
		}
		/* NOTE: depends on all-bits-zero NULL pointer */
		bzero(lvtmp, sizeof(struct lvol));
		lock_init(&(lvtmp->lv_lock), TRUE);
		LOCK_INTERRUPT_INIT(&(lvtmp->lv_intlock));
		BUF_LOCKINIT(&(lvtmp->lv_rawbuf));
		LV_QUEUE_INIT(&(lvtmp->lv_ready_Q));

		/*
		 * If this is an open of lvol 0 then set the maximums and the
		 * scheduling strategy.
		 */
		if (minor(dev) == 0) {
			if (!lv_thread_initialized) {
				lv_thread_initialized++;
				thread_call_create(&lv_threadq, 1, FALSE);
				thread_call_alloc(&lv_threadq, nbuf+1);
			}
			lvtmp->lv_maxlxs = MAXPVS;
			lvtmp->lv_maxmirrors = NOMIRROR;
			lvtmp->lv_sched_strat = LVM_RESERVED;
			lvtmp->lv_schedule = &lv_reserved_ops;
			lvtmp->lv_exts[PRIMMIRROR] = (extent_t *)
			       kalloc(sizeof(extent_t) * lvtmp->lv_maxlxs);
			if (lvtmp->lv_exts[PRIMMIRROR] == NULL)
				return(NULL);
			ext = lvtmp->lv_exts[PRIMMIRROR];
			for (i = 0; i < lvtmp->lv_maxlxs; i++) {
				ext->e_pvnum = PX_NOPV;
				ext->e_pxnum = 0;
				ext->e_state = 0;
				ext++;
			}
			lvtmp->lv_lext = (lextent_t *)
			       kalloc(sizeof(lextent_t) * lvtmp->lv_maxlxs);
			if (lvtmp->lv_lext == NULL) {
				KFREE(lvtmp->lv_exts[PRIMMIRROR],
				      sizeof(extent_t) * lvtmp->lv_maxlxs);
				return(NULL);
			}
			le = lvtmp->lv_lext;
			for (i = 0; i < lvtmp->lv_maxlxs; i++) {
				le->lx_synctrk = -1;
				le->lx_syncmsk = 0;
				le->lx_syncerr = 0;
				le++;
			}
			lvtmp->lv_status = LV_FAKEOPEN;
		}
		vg->lvols[mindev] = lvtmp;
	} else {
		panic("lv_alloclv: allocating already-allocated lvol!\n");
	}

	return(lvtmp);
}

lv_freelv(vg, minor_num)
register struct volgrp *vg;
ushort_t minor_num;
{
register struct lvol *lv;
int i;

	if ((lv = VG_DEV2LV(vg, minor_num)) == NULL) {
		panic("freeing unallocated lvol");
	}
	if (lv->work_Q) {
		panic("Leak! work_Q not NULL in freelv\n");
	}
	if (lv->lv_maxlxs != 0) {
		for (i = 0; i < LVM_MAXCOPIES; i++) {
			KFREE(lv->lv_exts[i], lv->lv_maxlxs*sizeof(extent_t));
		}
		KFREE(lv->lv_lext, lv->lv_maxlxs * sizeof(lextent_t));
		lv->lv_maxlxs = 0;
	}
	KFREE(lv, sizeof(struct lvol));
	vg->lvols[minor_num] = NULL;

	return;
}

/*
 * NAME:	lv_addextent
 *
 * FUNCTION:
 *	Add an extent to a logical volume.
 *
 * PARAMETERS:
 *	lv - The logical volume to extend.
 *	src_ext - Pointer to the extent to add.  NOTE: This must be copied.
 *	extno - The logical extent number indicating where to add the physical
 *		extent.
 *	mirno - The mirror number to add the physical extent to.
 *
 * RETURN:
 *	ESUCCESS - Successful.
 *	EINVAL - The extent is already in use.
 *	ENOMEM - Could not allocate memorty to perform the extend.
 */

lv_addextent(lv, src_ext, extno, mirno)
struct	lvol	*lv;			/* The logical volume to extent.    */
struct	extent	*src_ext;		/* The extent to add.		    */
	short	extno;			/* The extent number of add.	    */
	short	mirno;			/* The mirror number to add.	    */
{
struct	extent	*dst_ext;
	int	active = 0;		/* Number of active mirrors.	    */

	/* This is no longer supported. */
	if (extno == -1)
		panic("lv_addextent: a -1 extno is no longer supported.\n");

	/*
	 * If the mirror number is ANYMIRROR, then look for the first unused
	 * mirror for the specified logical extent.
	 */
	if (mirno == ANYMIRROR) {
		for (mirno = 0; mirno < (lv->lv_maxmirrors + 1); mirno++) {
			dst_ext = EXTENT(lv, extno, mirno);
			if (dst_ext->e_pvnum == PX_NOPV)
				break;
		}
		if (mirno > lv->lv_maxmirrors)
			return(EINVAL);
	}

	/*
	 * Make sure that there is not already an extent at the specified
	 * location.
	 */
	dst_ext = EXTENT(lv, extno, mirno);
	if (dst_ext->e_pvnum != PX_NOPV)
		return(EINVAL);

	*dst_ext = *src_ext; 	/* Structure copy... */

	lv->lv_curpxs++;
	if (mirno == 0) lv->lv_curlxs++;
	return(ESUCCESS);
}

/*
 * NAME:	lv_prestodisable
 *
 * FUNCTION:
 *
 *      Added for ULTRIX/OSF revision 3.2 
 *
 *	Disable Prestoserve for a logical volume device, if needed.
 *
 *	This function is called during extent additions or deletions to
 * an opened logical volume.  It should be called just before the logical
 * volume's IO is made quiescent and the actual add/delete activity is
 * accomplished.
 *
 *    	The routine checks if Prestoserve is in the system.  Then
 * it opens the Prestoserve device and checks if the logical
 * volume is accelerated by Prestoserve and if Prestoserve caching for the
 * device is enabled.  If everything
 * is true, then it disables Prestoserve on this device and returns.
 *
 * PARAMETERS:
 *	lv_dev - The logical volume (block?) major+minor device number.
 *
 * NOTES:
 *
 * RETURN:
 *
 *	TRUE -   Successfully disabled Prestoserve for 
 *               this device.
 *
 *	FALSE -  Did not disable Prestoserve for this device.
 *               This return value does not signal an error.  Prestoserve
 *               may not be in the system or Prestoserve acceleration on
 *               this device may already be disabled.
 *
 */

int
lv_prestodisable(lv_dev)
dev_t lv_dev;
{
    int result = FALSE;			/* The running error status.
					 * We start out assuming that no
					 * disabling of Prestoserve on this
					 * device will occur.
					 */
    struct uprtab lv_uprtab;		/* Presto-ized device data struct*/
    

    /* First determine if Prestoserve is even in the system */

    if (prattached) {

        lv_uprtab.upt_bmajordev = major(lv_dev);
	(*cdevsw[pr_maj_devnum].d_ioctl) (PRGETUPRTAB, &lv_uprtab);

        /* 
	 * Determine if the requested device is "accelerated" by 
	 * Prestoserve and if Prestoserve caching enabled for
	 * this device.
	 */

        if (lv_uprtab.upt_bmajordev != NODEV &&
	    lv_uprtab.upt_enabled.bits[minor(lv_dev)]) {

	/* 
	 * We now know the log vol device is "accelerated"
	 * and enabled, so disable it
	 */

	    (*cdevsw[pr_maj_devnum].d_ioctl) (PRDISABLE, lv_dev);
	    result = TRUE;

	}
    }
    return(result);
}



/*
 * NAME:	lv_prestoenable
 *
 * FUNCTION:
 *
 *      Added for ULTRIX/OSF revision 3.2 
 *
 *	Enable Prestoserve for a logical volume device.
 *
 *	This function is called during extent additions or deletions to
 * an opened logical volume.  It should be called just after
 * the logical volume's actual add/delete activity is
 * accomplished but before the I/O activity is restarted.
 *
 *    	The routine assumes that lv_prestodisable was used to 
 * disable the logical volume device.  Therefore it just re-enables 
 * the caching on this device and returns.
 *
 * PARAMETERS:
 *
 *	lv_dev - The logical volume (block?) major+minor device number.
 *
 * NOTES:
 *
 * RETURN:  NONE
 *
 */

void
lv_prestoenable(lv_dev)
dev_t lv_dev;
{

    /* Enable the accelerated logical volume device */

    (*cdevsw[pr_maj_devnum].d_ioctl) (PRENABLE, lv_dev);

    return;
}


/*
 * NAME:	lv_deviocget
 *
 * FUNCTION:
 *
 *      Added for ULTRIX/OSF revision 3.2 
 *
 *  This function is used by Prestoserve to determine if a logical
 *  volume device is backed by any physical devices which
 *  are on the QBUS.
 *
 *  No other function should really use this entry point, because
 *  only the bus data in the devget structure is GUARANTEED to be
 *  complete or accurate.
 *
 *  Because this code is
 *	1. not called often
 *      2. not called at critical stages of operations
 *  and, there is a desire to not increase LVM maintenance headaches
 *  by adding additions to data structures, the decision was made
 *  to not keep static information about LVs on QBUS but to acquire
 *  the information dynamically each time it is needed.
 *
 *  A Logical volume's extents
 *  may be backed by many physical volumes and there is no
 *  guarantee of a particular mapping.  So we are forced to
 *  go through quite an algorithm in an effort to call a PV
 *  only once for its bus data.
 *  
 * PARAMETERS:
 *	dev -- the logical volume major+minor device number
 *	vg -- the volume group the logical volume belongs to
 *	arg -- a pointer to the return data structure which
 *             this command is expected to complete
 *
 * NOTES:
 *
 * RETURN:
 *	0 -- successful return
 *	otherwise -- error return (data in devget structure may not be
 *                   considered complete or accurate)
 *
 */

int
lv_deviocget(dev, vg, arg)
dev_t dev;
struct volgrp *vg;
struct devget *arg;
{
    struct devget dg;		/* used to get the phy device info */
    struct lvol *lv;		
    struct pvol *pv;
    struct extent *ext;		/* pointer to a physical extent struct */
    uchar_t *checked_pv_ids;	/* pointer to an array of phy vols we
				 * have already checked for the phy 
				 * device info 
				 */
    int lxno;			/* logical extent number */
    int mirror;			/* mirror copy of the logical extent */
    int result_status = 0;	/* final status returned to the caller */
    int qbus_found = FALSE;	/* A PV on a QBUS is found */
    int pv_already_checked;	/* loop termination variable which indicates
				 * if a PV for this LV was already queried
				 * for device information
				 */
    int error;			/* status returned from phy device driver */
    int checked_pv_index = 0;	/* index of next unused spot in the 
				 * checked_pvs_ids
				 */
    int i;


    /* Logic Flow
     * ----------
     *
     * Initialize result status to successful
     * Initialize the devget return data structure
     * For each physical volume which is part of the logical device
     *    Call the physical driver DEVIOCGET entry point to get its device data
     *    If no ioctl error
     *	  Then
     *        If this device is on the QBUS
     *        Then
     *            Stuff the QBUS identifier into the logical volume devget
     *              return structure
     *        Endif
     *    Else
     *        Set an error return status
     *    Endif
     * Endfor
     * Return the result status [and the devget data structure]
     */

    /* Allocate an array to hold the PV IDs that have 
     * already been checked.  The size of this array
     * is equal to the number of PVs in the VG.  The LV
     * can never use PVs not in the VG.
     */
    checked_pv_ids = (uchar_t *) kalloc(vg->num_pvols * sizeof(uchar_t));

    /* Initialize the checked_pv_ids array */
    for (i=0; i<vg->num_pvols; i++) {
        checked_pv_ids[i] = -1;
    }

    /* Initialize the returned devget structure */
    bzero(arg, sizeof(struct devget));
    bcopy(DEV_UNKNOWN, arg->interface, strlen(DEV_UNKNOWN));
    bcopy(DEV_UNKNOWN, arg->device, strlen(DEV_UNKNOWN));
    bcopy("LVM", arg->dev_name, strlen("LVM"));
    arg->category = DEV_SPECIAL;
    arg->bus = DEV_UNKBUS;
    arg->category_stat = GETDEVS(dev);
    arg->unit_num = minor(dev);
    arg->slave_num = minor(dev);

    /* Calculate a pointer to the logical volume structure
     * for this device
     */
    lv = VG_DEV2LV(vg, dev);

    /* For each populated logical extent in the logical volume
     * determine the PV which backs it.
     *
     * We stop this search once we find a PV on the QBUS
     * or if we run into an error
     */
    for (lxno=0; 
	 !qbus_found &&  result_status == 0 && lxno < lv->lv_curlxs; 
	 lxno++) {
	
        /* Need to check each physical mirror copy of the logical extent 
	 */	
        for(mirror=0; 
	    !qbus_found && result_status == 0 && mirror < lv->lv_maxmirrors; 
	    mirror++) {
	
	    /* Get the pointer to the structure which describes the 
	     * physical extent supporting this mirrored copy of the
	     * logical extent
	     */
	    ext = EXTENT(lv, lxno, mirror);

	    /* e_pvnum is a PV ID.  Check that it is valid. */
	    if (ext->e_pvnum == PX_NOPV)
	        continue;

	    /* Has this physical volume already been checked? */
	    pv_already_checked = FALSE;
	    for (i=0; !pv_already_checked && i < vg->num_pvols; i++) {
	        if (ext->e_pvnum == checked_pv_ids[i]) 
		    pv_already_checked = TRUE;
	    }

	    if ( !pv_already_checked ) {
	        /* save this pv_id in our "checked" list */
	        checked_pv_ids[checked_pv_index] = ext->e_pvnum;
		checked_pv_index++;

		/* Ok, we finally got a physical volume that
		 * 1. is part of this logical volume
		 * 2. needs to be checked for qbus
		 */

		pv = vg->pvols[ext->e_pvnum];
		error = bdevsw[major(pv->pv_vp->v_rdev)].d_ioctl 
	                  (pv->pv_vp->v_rdev, DEVIOCGET, &dg, 0);
		if (error == 0) {
	            if (dg.bus == DEV_QB) {
	                arg->bus = DEV_QB;
		        qbus_found = TRUE;
		    }
		}
		else {
	            result_status = error;
		}

	    }
	
	}
    }
    KFREE (checked_pv_ids, vg->num_pvols * sizeof(uchar_t));
    return(result_status);
}


