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
static char	*sccsid = "@(#)$RCSfile: lv_syscalls.c,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/11/06 14:03:48 $";
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
 *	lv_syscalls.c - Logical volume manager system calls.
 *
 * FUNCTIONS:
 *	lv_open  - open a logical volume.
 *	lv_close - close a logical volume.
 *	lv_read  - read from a logical volume.
 *	lv_write - write to a logical volume.
 *	lv_ioctl - perform an IOCTL on a logical volume.
 *	lv_minphys
 *      Added for ULTRIX/OSF revision 3.2 
 *      lv_ready - check if logical volume ready for Prestoserve use.
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when combined with the
 * aggregated modules for this product) 
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  lv_syscalls.c -- System call interface to the logical volume manager.
 *
 *	This module contains the system call interfaces for the logical volume
 *	manager.  This component provides character and block entry points
 *	just like a real disk driver, with compatible arguments.
 */

/*
 *  Modification History:  lv_syscalls.c
 *
 *  15-MAY-91     Terry Carruthers
 *	ULTRIX/OSF revision 3.2
 *	Added the function lv_ready for Prestoserve to use as
 *      a check to see if a logical device was ready to be used.
 *
 *      Added the ioctl entry point DEVIOCGET which Prestoserve
 *      uses to determine if logical volume has physical volume
 *      members which are on the QBUS.
 *
 */


/*
 * Include files.
 */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/mode.h>

#include <lvm/lvmd.h>

/* Added for ULTRIX/OSF revision 3.2 */
#include <sys/ioctl.h>
#include <io/common/devio.h>


/* Internal Function Declarations */
extern int lv_minphys();

int lv_trace = 0;


/*
 * NAME:	lv_ready
 *
 * FUNCTION:
 *
 *      Added for ULTRIX/OSF revision 3.2 
 *
 * This entry point in the LVM driver should only be used by the
 * Prestoserve driver to check if a logical volume device is 
 * ready to be opened.
 *
 * This function checks:
 *      - the volume group exists in the system
 *      - the volume group is configured 
 *	- the volume group is activated
 *	- the volume group is not blocking logical volume opens
 *	- the logical volume is available
 *
 * If all the above is true, the the device is "ready".
 *
 * PARAMETERS:
 *	dev - The logical volume (block?) major+minor device number.
 *
 * NOTES:
 *
 * RETURN:
 *
 *	TRUE - device is ready for Prestoserve's use
 *	FALSE - device is NOT ready for Prestoserve's use
 */
int
lv_ready(dev)
dev_t dev;	/* device number (major,minor) of LV to be checked */
{
	struct lvol *lv = NULL;		/* pointer to lvol struct	*/
	register struct volgrp *vg;	/* pointer to volgrp struct	*/


	/* Special case minor number 0 which is not a true LV */
	if (minor(dev) == 0) {
	        return(FALSE);
        }

	/* Look up the VG pointer */
	if (!(vg = DEV2VG(dev)) ) { 
		/* device does not exist */
		return(FALSE);
	}

	/*
         * Verify that the VG is appropriately configured,
         * activated and allows LV opens.
	 */
	if ((vg->vg_flags & VG_NOTCONFIGURED) ||
	    !(vg->vg_flags & VG_ACTIVATED)    ||
	    (vg->vg_flags & VG_NOLVOPENS)) {
		return(FALSE);
	}	

	/*
	 * Look up the lvol given the device number.
	 */
	if ((lv = VG_DEV2LV(vg, dev)) == 0) {
		return(FALSE);
	} 

	/*
	 * Verify that the logical volume is available
	 */
	if (lv->lv_flags & LVM_DISABLED) {
		return(FALSE);  
	}	

	/* All checks passed, so the device is "ready" */
	return(TRUE);
}


/*
 * NAME:	lv_open 
 *
 * FUNCTION:	open logical volume
 *
 * PARAMETERS:	dev - dev_t (major,minor) of logical volume to be opened
 *		flags - specifies if device being opened for read or write
 *
 * NOTES:	If the dev_t passed in points to a valid logical volume,
 *		allocate the work_Q & extent structures, alloc
 *		physical buf pool for bottom half.  If 1st open for
 *		the VG, alloc the volgrp structure.
 *
 * RETURN VALUE: errno returned in return code
 *
 */

int
lv_open(dev, flags, fmt)
dev_t dev;	/* device number (major,minor) of LV to be opened */
int flags;	/* read/write flag	*/
int fmt;
{
	struct lvol *lv = NULL;		/* pointer to lvol struct	*/
	register struct volgrp *vg;	/* pointer to volgrp struct	*/
	register int error = ESUCCESS;
	extern struct lvol *lv_alloclv();

	if (!(vg = DEV2VG(dev)) ) { /* get volgrp pointer */
		/* device does not exist - is being configured */
		return(ENODEV);
	}
	/* We may modify the volgrp data structures if this is the
	 * first open of lvol[0]
	 */
	lock_write(&(vg->vg_lock));		/* lock volume group */

	vg->major_num = major(dev);

	/*
         * verify that the VG is appropriately configured.
	 */
	if ((vg->vg_flags & VG_NOTCONFIGURED)
		|| (minor(dev) && (vg->vg_flags & VG_NOLVOPENS))) {
		/* NOTE: if VG_CLOSING persists after a system call,
		   we risk loosing the VG entirely here. */
		lock_done(&(vg->vg_lock));	/* Release volgrp */
		return(ENXIO);
	}	

	/*
	 * Look up the lvol given the device number.  If the lookup fails,
	 * then attempt to expand the LVOL structures.
	 */
	if ((lv = VG_DEV2LV(vg, dev)) == 0) {
		if (minor(dev) == 0) {
			if ((lv = lv_alloclv(vg, dev)) == NULL) {
				lock_done(&(vg->vg_lock));
				return(ENOMEM);
			}
		} else {
			lock_done(&(vg->vg_lock));	/* Release volgrp */
			return(ENXIO);
		}
	} 
	/* No longer need to modify the volgrp data structures. */
	lock_write_to_read(&(vg->vg_lock));

	/*
	 * If logical volume is read only and the flags indicate the open
	 * is for appending or writing then declare an error.
	 */
	if ((flags&(FWRITE|FAPPEND)) && (lv->lv_flags&LVM_RDONLY)) {
		lock_done(&(vg->vg_lock));	/* Release volgrp */
		return(EROFS);   /* Read only device */
	}	

	if (minor(dev) && (lv->lv_flags&LVM_DISABLED)) {
		lock_done(&(vg->vg_lock));	/* Release volgrp */
		return(ENXIO);
	}

	lock_write(&(lv->lv_lock));
	if ((lv->lv_status & LV_OPEN) == 0) {
		/*
		 * if LV is not already open:
		 *	allocate work_Q
		 * 	allocate pbuf subpool (add to central free list)
		 *	set lvol open flag
		 */
		error = lv_openlv(vg, lv);
	}
	if (error == ESUCCESS) {
		switch(fmt) {
			case S_IFCHR:	
				if ((lv->lv_status & LV_RAWOPEN) == 0) {
					/* first open of raw device */
					lv->lv_rawavoid = 0;
					lv->lv_rawoptions = 0;
					lv->lv_status |= LV_RAWOPEN;
				}
				break;
			case S_IFBLK:
				lv->lv_status |= LV_BLOCKOPEN; break;
			default:
				panic("lv_open fmt"); break;
		}
	}

	lock_done(&(lv->lv_lock));
	lock_done(&(vg->vg_lock));	/* Release volgrp */
	return(error);
}

int
lv_openlv(vg, lv)
register struct volgrp *vg;
register struct lvol *lv;
{
int error;
	/* first open for this LV */
	/* allocate space */
	if (lv->work_Q != NULL) {
		printf("lv_openlv: LEAK: info not NULL on open.\n");
	}
	if (lv->work_Q == NULL) {
		lv->work_Q = (struct h_anchor *)
			kalloc(sizeof(struct h_anchor)*WORKQ_SIZE);
	}

	/*
	 * We have to do this here so we can calculate the correct
	 * number of pbufs to allocate.  If any failures happen
	 * after this will have to be adjusted then.
	 */

	if ((lv->work_Q == NULL) || (lv_adjpool((vg->vg_opencount==0), 1))) {
		/* Allocation failed. Need to back out allocations */
		KFREE(lv->work_Q, sizeof(struct h_anchor)*WORKQ_SIZE);
		error = ENOMEM;
	} else {
		bzero(lv->work_Q, sizeof(struct h_anchor)*WORKQ_SIZE);

		/*
		 * set LV status to open, increment the open count
		 * in the VG.
		 */
		lv->lv_status |= LV_OPEN;
		vg->vg_opencount++;
		error = ESUCCESS;
	}
	return(error);
}

/*
 * NAME:	lv_close 
 *
 * FUNCTION:	close logical volume
 * 
 * PARAMETERS:	dev - dev_t (major,minor) of logical volume to be closed
 *
 * RETURN VALUE: ERRNO returned in return code
 *
 */
/* ARGSUSED */
int
lv_close(dev, flags, fmt)
dev_t	dev;	/* device number (major,minor) of LV to be closed */
int flags;
int fmt;
{
	register struct lvol *lv;	/* lvol pointer		  */
	register int s;			/* saved interrupt priority*/
	register int i;			/* counter		  */
	register int cnt;		/* number of pbuf to free */
	register struct pbuf *bp;	/* pointer to pbuf struct */
  	struct volgrp   *vg;		/* volgrp pointer	  */

	vg = DEV2VG(dev);			/* get volgrp pointer */
	lv = VG_DEV2LV(vg,dev);			/* get lvol pointer */

	lock_read(&(vg->vg_lock));		/* lock volume group */

	lock_write(&(lv->lv_lock));
	if ((minor(dev) == 0)
			&& ((vg->vg_flags & VG_ACTIVATED) == 0)
			&& (vg->vg_opencount == 1)) {
		/* lv->lv_status &= ~LV_FAKEOPEN; */
		/* Work_Q and rawbuf for lvol[0] only get freed if this
		 * is the last close of an inactive volume group */
	}
	switch(fmt) {
		case S_IFCHR:	lv->lv_status &= ~LV_RAWOPEN; break;
		case S_IFBLK:	lv->lv_status &= ~LV_BLOCKOPEN; break;
		default:	panic("lv_close fmt"); break;
	}
	lv_closelv(dev, vg, lv);

	lock_done(&(lv->lv_lock));
	lock_done(&(vg->vg_lock));

	/* all done, unlock volume group structures */
	return(0);
}

lv_closelv(dev, vg, lv)
dev_t dev;
register struct volgrp *vg;
register struct lvol *lv;
{
	if ((lv->lv_status & LV_OPEN) == 0) {
		/* 
		 * This is a logical error, closing a closed file.
		 */
		panic("lv_close: multiple close");
	}
	if (lv->lv_status & (LV_FAKEOPEN|LV_BLOCKOPEN|LV_RAWOPEN))
		return;

	lv->lv_status &= ~LV_OPEN;
	if (lv->lv_status != 0) {
		panic("lv_close status");
	}
	if (minor(dev))
		lv_cache_clean(vg, minor(dev));

	/*
	 * Release resources.  Free pbuf's
	 * here to avoid possible collisions here.
	 */
	
	vg->vg_opencount--;	/* Decrement open LV count in VG */
	lv_adjpool(((vg->vg_opencount==0)?-1:0),-1);

	KFREE(lv->work_Q, sizeof(struct h_anchor)*WORKQ_SIZE);

	if (vg->vg_opencount == 0) {
		/* This must be the final close of a deactivated
		 * volume group. Free the final struct lvol. */
		if (minor(dev)) panic("lv_close");
		lv_freelv(vg, 0);
	}
	return;
}

int
lv_lvhold(vg, dev, minor_num, lvp)
struct volgrp *vg;
dev_t dev;
int minor_num;
struct lvol **lvp;
{
struct lvol *lv;
int error;

	lock_read(&(vg->vg_lock));

	if ((lv = VG_DEV2LV(vg, minor_num)) == NULL) {
		lock_done(&(vg->vg_lock));
		return(ENODEV);
	}
	if (minor(dev) == 0) {
		/*
		 * I need to fake an open here so that all of the right 
		 * data structures are initialized to perform the I/O. This
		 * references the appropriate data structures so they can't
		 * go away.
		 */
		lock_write(&(lv->lv_lock));
		if (lv->lv_ref++ == 0)  {
			lv->lv_status |= LV_FAKEOPEN;
		}
		if ((lv->lv_status & LV_OPEN) == 0) {
			if ((error = lv_openlv(vg, lv)) != ESUCCESS) {
				lock_done(&(lv->lv_lock));
				lock_done(&(vg->vg_lock));
				return(error);
			}
		}
		lock_done(&(lv->lv_lock));
	}
	lock_done(&(vg->vg_lock));

	*lvp = lv;
	return(ESUCCESS);
}

lv_lvrelease(vg, dev, lv)
struct volgrp *vg;
dev_t dev;
struct lvol *lv;
{
	if (minor(dev) == 0) {
		lock_write(&(vg->vg_lock));
		lock_write(&(lv->lv_lock));

		if (--lv->lv_ref == 0) {
			lv->lv_status &= ~LV_FAKEOPEN;
			lv_closelv(dev, vg, lv);
		}
		lock_done(&(lv->lv_lock));
		lock_done(&(vg->vg_lock));
	}
	return;
}

/*
 *  NAME:         lv_read 
 * 
 *  FUNCTION:     raw device read entry point
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume requested.       
 *		  uiop -pointer to uio structure that specifies location &
 *			length of caller's data buffer.
 *
 *  NOTE: 	  calls physio for queuing request to lv_strategy
 *			until all of request is completed.
 *			
 *  RETURN VALUE: If error, return ERRNO value in return code 
 *
 */
int
lv_read(dev, uiop)
dev_t	   dev;		/* device number (major,minor) of LV to be read	*/
struct uio *uiop;	/* pointer to uio structure that specifies	*/
			/* location & length of caller's data buffer.	*/
{
	register struct lvol *lv;	/* lvol pointer		  */
  	struct volgrp   *vg;		/* volgrp pointer	  */
	struct buf *lb;
	int error;

	vg = DEV2VG(dev);			/* get volgrp pointer */
	lv = VG_DEV2LV(vg,dev);			/* get lvol pointer */

	/*
	 * call physio send request to lv_strategy.  	
	 */
	lb = &(lv->lv_rawbuf);
	BUF_LOCK(lb);

	lb->b_options = lv->lv_rawavoid;
	if (lv->lv_rawoptions & LVM_NORELOC) lb->b_options |= LVM_OPT_NORELOC;
	error = physio(lv_strategy, lb, dev, B_PRIVATE|B_READ,
				lv_minphys, uiop);

	BUF_UNLOCK(lb);
	return(error);
}
	
/*
 *  NAME:         lv_write 
 * 
 *  FUNCTION:     raw device write entry point
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume to be written
 *		  uiop -pointer to uio structure that specifies location &
 *			length of caller's data buffer.
 *
 *  NOTE: 	  calls physio for queuing request to lv_strategy
 *			until all of request is completed.
 *			
 *  RETURN VALUE: If error, return ERRNO value in return code 
 *
 */
int
lv_write(dev, uiop)
dev_t	   dev;		/* device number (major,minor) of LV to be written*/
struct uio *uiop;	/* pointer to uio structure that specifies	*/
			/* location & length of caller's data buffer.	*/
{
	register struct lvol *lv;	/* lvol pointer		  */
  	struct volgrp   *vg;		/* volgrp pointer	  */
	struct buf *lb;
	int error, flags = B_PRIVATE;

	vg = DEV2VG(dev);			/* get volgrp pointer */
	lv = VG_DEV2LV(vg,dev);			/* get lvol pointer */

	if (minor(dev) == 0) {
		/* No writes allowed to Control Device */
		return(EROFS);
	}
	/*
	 * call physio to send request to lv_strategy.  	
	 */
	lb = &(lv->lv_rawbuf);
	BUF_LOCK(lb);

	lb->b_options = 0;
	if (lv->lv_rawoptions & LVM_VERIFY) flags |= B_WRITEV;
	if (lv->lv_rawoptions & LVM_NORELOC) lb->b_options |= LVM_OPT_NORELOC;
	error = physio(lv_strategy, lb, dev, B_WRITE|flags, lv_minphys, uiop);

	BUF_UNLOCK(lb);
	return(error);
}

/*
 *  NAME:         lv_minphys
 * 
 *  FUNCTION:     called by physio to split up request to logical track
 *			group (128K) size, if needed.  
 *  
 *  PARAMETERS:   bp - pointer to buf struct to be checked            
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: EINVAL - request is not a multiple of blocks
 *
 */
lv_minphys(bp)
struct buf	*bp;		/* ptr to buf struct to be checked	*/
{
	register int blkcnt;		/* block count */
	register int blkwt;		/* block within LTG */

	/* error if request not a multiple of blocks */          
	if (bp->b_bcount & (DEV_BSIZE-1)) {
		/* strategy will reject this request */
	} else {
		/*
		 * request CANNOT cross Logical Track Group boundaries 
		 * - break up request if it does.
		 */

		/* count of blocks transferred */
		blkcnt = BYTE2BLK(bp->b_bcount);

		/* offset of 1st blk within LTG */
		blkwt = (bp->b_blkno & BLKPTRK-1);

		/*
		 * if request crosses LTG, change bcount to have only
		 * the amount in the first LTG affected by this request 
		 */
		if ((blkwt + (blkcnt-1)) >= BLKPTRK)  {
			bp->b_bcount = BLK2BYTE(BLKPTRK-blkwt);	
		}
	}
	return;
}

/*
 *  NAME:         lv_ioctl
 * 
 *  FUNCTION:     ioctl entry point                    
 *			IOCINFO:  return size of logical volume:
 *					CYL=extent
 *					TRK=logical track group
 *					SECT=block
 * 			XLATE:    translate logical address to physical address
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume to be used
 * 		  cmd - specific ioctl command to be performed
 * 	 	  arg - address of parameter block for the specific ioctl cmd
 * 		  flag- file flag
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: ERRNO returned in return code
 *
 */
int
lv_ioctl(dev, cmd, arg, flag)
	dev_t	dev;			/* Device number VG/LV.		*/
register unsigned int	cmd;		/* Specific ioctl command.	*/
	char	*arg; 	 		/* Address of parameter block.	*/
	int	flag;			/* File flag.			*/
{
	register struct volgrp *vg;	/* ptr from device switch table	*/
	register int	rc = 0;		/* return value			*/

	/* Most ioctls must be performed on logical volume 0, check this. */
	switch (cmd) {
		/*
		 * This class of commands may be performed on
		 * any logical volume, and do not require write access.
		 */
	        /* Added DEVIOCGET for ULTRIX/OSF revision 3.2 */
	case DEVIOCGET:
	case LVM_OPTIONSET:
	case LVM_OPTIONGET:
	case LVM_QUERYLV:
	case LVM_QUERYLVMAP:
	case LVM_QUERYVG:
#ifdef LVM_OQUERYVG
	case LVM_OQUERYVG:
#endif
		break;

		/*
		 * This class of commands may be performed on
		 * any logical volume, but require write access.
		 */
	case LVM_CHANGELV:
#ifdef LVM_DEBUG_STALEPX
	case LVM_DEBUG_STALEPX:
#endif
	case LVM_EXTENDLV:
	case LVM_REDUCELV:
	case LVM_RESYNCLV:
	case LVM_RESYNCLX:
	case LVM_REALLOCLV:
		if ( (flag&(FWRITE|FAPPEND)) == 0) {
			/* Must have write access to do these things */
			return(EBADF);
		}
		break;

		/*
		 * This class of commands may only be performed on
		 * the control device, but do not require write access.
		 */
	case LVM_QUERYPV:
	case LVM_QUERYPVMAP:
	case LVM_QUERYPVPATH:
	case LVM_QUERYPVS:
		if (minor(dev) != 0) {
			/* "Inappropriate ioctl for device" */
			return(ENOTTY);
		}
		break;

		/*
		 * This class of commands may only be performed on
		 * the control device, and require write access.
		 */
	default:
		if (minor(dev) != 0) {
			/* "Inappropriate ioctl for device" */
			return(ENOTTY);
		}
		if ( (flag&(FWRITE|FAPPEND)) == 0) {
			/* Must have write access to do these things */
			return(EBADF);
		}
		break;
	}

	/* Convert the device major number into the vg pointer. */
	vg = DEV2VG(dev);

	/*
	 * Perform the I/O controls.
	 */
	switch(cmd) {
	case LVM_ACTIVATEVG:	rc = lv_activatevg(vg, *(int *)arg); break;
	case LVM_ATTACHPV:	rc = lv_attachpv(vg, *(char **)arg); break;
	case LVM_CHANGELV:	rc = lv_changelv(dev, vg, arg); break;
	case LVM_CHANGEPV:	rc = lv_changepv(vg, arg); break;
	case LVM_CREATELV:	rc = lv_createlv(vg, arg); break;
	case LVM_CREATEVG:	rc = lv_createvg(vg, arg); break;
	case LVM_DEACTIVATEVG:  rc = lv_deactivatevg(vg); break;
#ifdef LVM_DEBUG_STALEPX
	case LVM_DEBUG_STALEPX: rc = lv_debug_stalepx(dev, vg, arg); break;
#endif
	case LVM_DELETELV:	rc = lv_deletelv(vg, *(int *)arg); break;
	case LVM_DELETEPV:	rc = lv_deletepv(vg, *(int *)arg); break;
	case LVM_EXTENDLV:	rc = lv_extendlv(dev, vg, arg); break;
	case LVM_INSTALLPV:	rc = lv_installpv(vg, arg); break;
	/* Added DEVIOCGET for ULTRIX/OSF revision 3.2 */
	case DEVIOCGET:		rc = lv_deviocget(dev, vg, arg); break;
	case LVM_OPTIONGET:	/* FALLTHROUGH */
	case LVM_OPTIONSET:	rc = lv_lvoption(dev, vg, cmd, arg); break;
	case LVM_QUERYLV:	rc = lv_querylv(dev, vg, arg); break;
	case LVM_QUERYLVMAP:	rc = lv_querylvmap(dev, vg, arg); break;
	case LVM_QUERYPV:	rc = lv_querypv(vg, arg); break;
	case LVM_QUERYPVMAP:	rc = lv_querypvmap(vg, arg); break;
	case LVM_QUERYPVPATH:	rc = lv_querypvpath(vg, arg); break;
	case LVM_QUERYPVS:	rc = lv_querypvs(vg, arg); break;
	case LVM_QUERYVG:	rc = lv_queryvg(vg, arg); break;
#ifdef LVM_OQUERYVG
	case LVM_OQUERYVG:	rc = lv_oqueryvg(vg, arg); break;
#endif
	case LVM_REALLOCLV:	rc = lv_realloclv(vg, arg); break;
	case LVM_REDUCELV:	rc = lv_reducelv(dev, vg, arg); break;
	case LVM_REMOVEPV:	rc = lv_removepv(vg, *(int *)arg); break;
	case LVM_RESYNCLV:	rc = lv_resynclv(dev, vg, *(int *)arg); break;
	case LVM_RESYNCLX:	rc = lv_resynclx(dev, vg, arg); break;
	case LVM_RESYNCPV:	rc = lv_resyncpv(dev, vg, *(int *)arg); break;
	case LVM_SETVGID:	rc = lv_setvgid(vg, arg); break;

	default:
		rc = EINVAL;
		break;

	}
	return(rc);
}

#ifdef LVM_DEBUG_STALEPX
int
lv_debug_stalepx(dev, vg, arg)
dev_t dev;
struct volgrp *vg;
struct lv_lvsize *arg;
{
struct lvol *lv;
lxmap_t *lxmap;
struct lextent *lext;
struct extent *ext;
int i, m, size, lxno;
int error, stale;

	if (minor(dev) != 0)
		arg->minor_num = minor(dev);

	if ((lv = VG_DEV2LV(vg, arg->minor_num)) == NULL) {
		return(ENODEV);
	}

	stale = 0;
	size = sizeof(lxmap_t)*arg->size;
	if ((lxmap = (lxmap_t *)kalloc(size)) == NULL)
		return(ENOMEM);

	/* Copy in the logical extent map from user space. */
	if ((error = copyin(arg->extents, lxmap, size)) != ESUCCESS) {
		KFREE(lxmap, size);
		return(error);
	}
		
	for (i = 0; i < arg->size; i++) {
		if ((lxno = lxmap[i].lx_num) > lv->lv_maxlxs) continue;
		lext = LEXTENT(lv, lxno);
		for (m = 0; m < (lv->lv_maxmirrors+1); m++) {
			ext = EXTENT(lv,lxno,m);
			if ((ext->e_pvnum == lxmap[i].pv_key)
				&& (ext->e_pxnum == lxmap[i].px_num)
				&& ((ext->e_state & LVM_PXSTALE) == 0)) {
				ext->e_state |= LVM_PXSTALE;
				lv_sa_setstale(vg, ext->e_pvnum, ext->e_pxnum);
				stale++;
				break;
			}
		}
	}
	if (stale)
		lv_sa_config(vg, CNFG_SYNCWRITE, NULL);

	arg->size = stale;
	KFREE(lxmap, size);
	return (ESUCCESS);
}
#endif
