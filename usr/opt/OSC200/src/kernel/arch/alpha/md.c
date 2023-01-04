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
static char	*sccsid = "@(#)md.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: machine/alpha/md.c
 *
 *  30-Jan-92 -- mjr
 *	Fixed initialization syntax for memdev.
 *
 *  20-Jun-91 -- afd
 *	Ported to OSF (largely based on mips version).
 *
 *  3-Oct-90 -- jmartin
 *	Add some #ifdef __alpha code.
 *
 * 11-Sep-90 -- afd
 *	Created this file for Alpha SAS memory driver.
 */

#include <memd.h>
#if NMEMD > 0

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/vm.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>

int mdsize=NMEMD;		/* number of 512 byte fs blocks */

long memdev[NMEMD*64]={1};	/* alloc in .data space, 512bytes long aligned */

/* from h/param.h */
#ifndef INTRLVE
/* macros replacing interleaving functions */
#define	dkblock(bp)	((bp)->b_blkno)
#endif

vm_offset_t MD_mach_buf;	 /* Allocated in sys_space */

/*
 * NMEMD is #-of-disk-blocks, * 512 gives size in bytes
 */
mdopen(dev, flag)
	dev_t	dev;
	long	flag;
{
	return(0);
}

mdstrategy(bp)
	register struct buf *bp;
{
	int sz, bn;
	register char *mdv;
	long *bufp;
	unsigned long v;
	struct proc *rp;
	int md_unit;
	pmap_t	pmap;

	md_unit = GETUNIT(bp->b_dev);
	sz = (bp->b_bcount + 511) >> 9;		/* fs rounding */
	if (md_unit != 0 || bp->b_blkno < 0 ||
	    (minor(bp->b_dev) != 0) ||
	    (bn = dkblock(bp)) + sz > mdsize) {
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
		}
	mdv = (char *) memdev + (bn*512);	/* file system offset */
	if ((bp->b_flags & B_PHYS) == 0) {
		bufp = (long *)bp->b_un.b_addr;	/* not phys: buffer cache */
	} else {
		int           cnt;
		kern_return_t ret;

		/*
		 * map to user space
		 */
		v = (unsigned long)bp->b_un.b_addr;
		pmap = bp->b_proc->task->map->vm_pmap;
		cnt = bp->b_bcount;
		bufp = (long *)MD_mach_buf;
		ret = pmap_dup(pmap, v, cnt, bufp, VM_PROT_WRITE, TB_SYNC_ALL);
		if(ret != KERN_SUCCESS)
			panic("mdstrategy: pmap_dup");
	}
	if (bp->b_flags & B_READ) {
		bcopy (mdv, bufp, bp->b_bcount);
	} else {
		bcopy (bufp, mdv, bp->b_bcount);
	}
	iodone (bp);
}


md_size(dev)
	dev_t dev;
{
	/*
	 * Lie a little
	 */
	return (33440);
}

mdioctl(dev, cmd, data, flag)
	register dev_t dev;
	register unsigned int cmd;
	register caddr_t data;
	register int flag;
{
	register struct devget *devget;

	switch (cmd) {

#if 0
	case DEVIOCGET: 			/* device status */
		devget = (struct devget *)data;
		bzero(devget,sizeof(struct devget));
		devget->category = DEV_SPECIAL; 	/* special disk */
		devget->bus = DEV_NB;			/* no bus con.	*/
		bcopy(DEV_UNKNOWN,devget->interface,
		      strlen(DEV_UNKNOWN));		/* n/a		*/
		bcopy(DEV_RAMDISK,devget->device,
		      strlen(DEV_RAMDISK));		/* memory disk	*/
		devget->adpt_num = -1;			/* n/a		*/
		devget->nexus_num = -1; 		/* n/a		*/
		devget->bus_num = -1;			/* n/a		*/
		devget->ctlr_num = -1;			/* n/a		*/
		devget->slave_num = -1; 		/* n/a		*/
		bcopy("md",devget->dev_name,3); 	/* Ultrix "md"	*/
		devget->unit_num = 0;			/* md0		*/
		devget->soft_count = 0; 		/* always 0	*/
		devget->hard_count = 0; 		/* always 0	*/
		devget->stat = 0;			/* always 0	*/
		devget->category_stat = GETDEVS(dev);	/* parition ?	*/
		break;
#endif

	default:
		return (ENXIO);
	}
	return (0);
}
#endif NMEMD > 0
