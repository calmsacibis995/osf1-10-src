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
static char	*sccsid = "@(#)$RCSfile: md.c,v $ $Revision: 1.2.3.5 $ (DEC) $Date: 1992/06/24 15:54:51 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from md.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 * md.c
 *
 * Modification history
 *
 * SAS memory device driver
 *
 * 26-Aug-86 - rsp (Ricky Palmer)
 *
 *	Cleaned up devioctl code to (1) zero out devget structure
 *	upon entry and (2) use strlen instead of fixed storage
 *	for bcopy's.
 *
 *  5-Aug-86 - fred (Fred Canter)
 *
 *	Changed DEV_NOB to DEV_NB.
 *
 * 12-Mar-86 - tresvik
 *
 *	This  driver  is  intended to be used in the Ultrix standalone
 *	environment where a device named md0a will be the root device.
 *	md0b  will  be made the swap device, but the kernel must never
 *	try  to page.  A monstrous lie is told when the drive is asked
 *	about  the  size  of  the swap partition.  This environment is
 *	intended  primarily to support an installation which will have
 *	limited commands and functions available.  It runs single user
 *	only,  with  minimal  kernel function built in.  This is in an
 *	effort	to  minimize  it's  size as the associated memory file
 *	system occupies a fair amount of memory too.  This driver will
 *	not  be  supported  in	any  other running environment in it's
 *	current state.	- tresvik
 *
 * 23-Apr-86 - Ricky Palmer
 *
 *	Added new DEVIOCGET ioctl request code.  V2.0
 *
 * 18-Jun-86 - Tresvik
 *
 *	Prevent reads and writes on partitions other than 0.
 *
 * 11-Jul-86 - ricky palmer
 *
 *	Added adpt and nexus fields to DEVIOCGET code.
 *
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
#ifdef	MACH
#include <vm/vm_kern.h>
#include <vm/pmap.h>
#else	MACH
#include <io/common/devio.h>
#endif	MACH
#ifdef vax
#include <vax/mtpr.h>
#endif vax

int mdsize=NMEMD;			/* number of 512 byte fs blocks */
char memdev[NMEMD*512]={1};		/* allocate in data space */

#ifdef	MACH
/* from h/param.h */
#ifndef INTRLVE
/* macros replacing interleaving functions */
#define	dkblock(bp)	((bp)->b_blkno)
#endif

static vm_offset_t MD_mach_buf;

int md_first_open = 1;

mdopen(dev, flag)
	dev_t	dev;
	int	flag;
{
     if (md_first_open) {
       MD_mach_buf = kmem_alloc_pageable(kernel_map, NMEMD*512);
       md_first_open = 0;
     }
     return(0);
}

#else	MACH
struct	mdspace {
	char md_pad[NBPG];		/* memory pages */
};
extern struct	mdspace MD_bufmap[];	/* pte's point here */
#endif	MACH

mdstrategy(bp)
register struct buf *bp;
{
	long sz, bn;
	register char *mdv;
#ifndef	MACH
	register struct pte *pte, *mpte;
#endif	MACH
	int *bufp;
	unsigned v;
	int i, o, npf;
	struct proc *rp;
	int md_unit;
#ifdef	MACH
	pmap_t	pmap;
#endif	MACH

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
		bufp = (int *)bp->b_un.b_addr;
	} else {
		int           cnt;
		kern_return_t ret;

		/*
		 * map to user space
		 */
#ifdef	MACH
		v = (unsigned)bp->b_un.b_addr;

		pmap = bp->b_proc->task->map->vm_pmap;
		cnt = bp->b_bcount;
		bufp = (int *)MD_mach_buf;
		ret = pmap_dup(pmap, v, cnt, bufp, VM_PROT_WRITE, TB_SYNC_ALL);
		if(ret != KERN_SUCCESS)
			panic("mdstrategy: pmap_dup");
#else	MACH
		o = (int)bp->b_un.b_addr & PGOFSET;
		npf = btoc(bp->b_bcount + o);
		v = btop(bp->b_un.b_addr);

		rp = bp->b_flags&B_DIRTY ? &proc[2] : bp->b_proc;
		pte = vtopte(rp, v);

		bufp = (int *)MD_bufmap;
		mpte = (struct pte *)mdbufmap;
		for (i=0;i<npf;i++) {
			if (pte->pg_pfnum == 0)
				panic("mda0: zero pfn in pte");
			mpte->pg_pfnum = pte->pg_pfnum;
			*(int *)mpte |= PG_V | PG_KW | PG_M;
			tlbdropin(rp->p_tlbpid,
				(char *)MD_bufmap+(i*NBPG),*(int *)mpte);
			mpte++; pte++;
		}
#endif	MACH
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
	register int cmd;
	register caddr_t data;
	register int flag;
{
	register struct devget *devget;

	switch (cmd) {

#ifdef	notdef
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
