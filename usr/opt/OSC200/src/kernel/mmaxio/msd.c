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
static char	*sccsid = "@(#)$RCSfile: msd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:25 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989,1990 by Encore Computer Corporation.
 * All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include "mmax_debug.h"

/*

 *
 */
#ifdef	SCCS

#endif

/*1
 *  DISK driver for MULTIMAX/EMC.
 *
 *  This code comprises the top level generic disk driver.
 *  It communicates via 'crq' mechanisms to an EMC board on a multimax.
 *
 *  This file was created from ms.c and ms_subr.c in splitting the
 *  disk and tape drivers as part of the OSF loadable device driver
 *  project.
 */

#include <sys/param.h>
#include <sys/user.h>

#include <sys/proc.h>
#include <mach/boolean.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/dk.h>
#include <kern/queue.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/syslog.h>
#include <kern/sched_prim.h>
#include <mmaxio/io.h>
#include <mmaxio/crqdefs.h>
#include <mmaxio/emcdefs.h>
#include <mmaxio/msdefs.h>
#include <mmaxio/msioctl.h>
#include <mmaxio/elog.h>
#include <mmaxio/ms_dev.h>
#include <mmax/cpu.h>
#include <sys/lock_types.h>

#include <emc.h>
#include <msd.h>
#include <mst.h>

#include <kern/assert.h>
#include <kern/thread.h>

extern struct devaddr emc_devaddr[];
extern struct subdevaddr msd_subdevaddr[];

ms_struct_t	msd_struct[NMSD];
ms_layout_t	msd_layout[NMSD];

int		ms_physio();

crq_ms_xfer_msg_t *ms_getcmd();

/* ERROR MESSAGES for logging */

static char checkhdr_err[] =
    "ms_init_disk: layout hdr format err: dev 0x%x, sts = %x\n";

static char cksum_err[] =
    "ms_init_disk: layout hdr cksum err: dev 0x%x, stored %x, calc %x\n";

static char hdr_read_err[] = 
	"ms_init_disk: layout hdr read err: dev 0x%x,\n\t... sts 0x%x, blknum %x\n";

static char fakehdr_err[] =
	"ms_init_disk: using fake header: dev 0x%x\n";

static char badblock_err[] =
	"msdstrategy: bad block number 0x%x, dev 0x%x\n";

static char wrtlay_cksum_err[] =
	"write_layout: cksum err: dev 0x%x, stored %x, calc %x\n";

/* FORWARD FUNCTION REFERENCE */

struct	buf	*msdstrategy();

msd_config (log_emc)
	int	log_emc;
{
	int 	ldev;
	if (emc_devaddr[log_emc].v_valid != DEV_INITIALIZED) return;
	for (ldev=0; ldev < NMSD; ldev++) {
		if (msd_subdevaddr[ldev].s_slotunit == log_emc) {
			msd_subdevaddr[ldev].s_valid = DEV_VALID;
			ms_init_slot(log_emc, ldev, CLASS_DISK);
		} /* else device still DEV_INVALID */
	}
	return;
}

msdblkopen(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	return(ms_open (dev, mode, CLASS_DISK, BLOCK_DEV));
	}

msdopen(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	return(ms_open (dev, mode, CLASS_DISK, CHAR_DEV));
	}

msdblkclose(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	return(msd_close (dev, mode, BLOCK_DEV));
	}

msdclose(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	return(msd_close (dev, mode, CHAR_DEV));
	}

ms_init_disk(ms,mode,dev,form)
ms_struct_t *ms;
int mode;
dev_t dev;
int form;	/* BLOCK_DEV or CHAR_DEV */
{
bp_env_t bp_env;
struct	buf    *bp;
int	 sts;
unsigned char *bptr;
unsigned long cksum, blknum, numreads, laysize;

    ms->ms_state = MST_QACTIVE;
    msd_null_layout(ms->ms_layout);
    bp = ms_getblk (&bp_env, dev);

    laysize = sizeof(layout_t) - 1;
    bptr   = (unsigned char *) bp->b_un.b_addr;

    /* Read in all but last block of layout, accumulate cksum */

    numreads = laysize >> DEV_BSHIFT;
    cksum  = 0;
    blknum = 0;
    while (numreads-- > 0) {
	    sts =ms_rw (ms, bp, blknum, DEV_BSIZE, CRQOP_MS_READ);
	    if (sts != ESUCCESS) goto readerror;
	    blknum += 1;
	    ms_acc_cksum(bptr, DEV_BSIZE, &cksum);
    }

    /* Read in LAST BLOCK of layout, accumulate cksum (except cksum) */

    sts =ms_rw (ms, bp, blknum, DEV_BSIZE, CRQOP_MS_READ);
    if (sts != ESUCCESS) goto readerror;
    numreads = (laysize & (DEV_BSIZE-1)) - sizeof(long) + 1;
    ms_acc_cksum(bptr, numreads, &cksum);

    /* CHECKSUM test */

    if (cksum != *((unsigned long *) (bptr + numreads))) {
	ms_deverr(cksum_err, dev, ms->ms_multidev, cksum,
			*((unsigned long *) (bptr+numreads)));
	 goto check_hdr_part;	
    }

    /* REREAD FIRST BLOCK with partition INFO */

	sts =ms_rw (ms, bp, blknum=0 , DEV_BSIZE, CRQOP_MS_READ);
	if (sts != ESUCCESS) goto readerror;

    /* check first block for consistency */

	sts = msd_check_layout(bptr);
	if (sts != ESUCCESS) {
		ms_deverr (checkhdr_err, dev, ms->ms_multidev, sts, 0);
check_hdr_part:
		/* best multimax can do is partial (questionable) open */
		if (MS_PART(dev) != HEADER_PART)
			goto anyerror;	/* sts from msd_check_layout */
		/* ... may require crq traffic */
		if (ms_fix_layout (ms, bp) != ESUCCESS)
			goto anyerror;	/* sts from msd_check_layout */
		ms_deverr (fakehdr_err, dev, ms->ms_multidev, sts, 0);
	} else {
		/* ... The header seemed cool, copy it */
		ms_laycopy (bptr, ms->ms_layout);
		ms->ms_state = MST_ACTIVE;
	}

	/* ...setup for sadp utility */
	ms->ms_total_blocks = ms->ms_layout->lay_geom.total_avail_blocks;

	ms_putblk (bp, &bp_env);
	return(ESUCCESS);

readerror:	/* couldn't read first block, bail out */
		ms_deverr (hdr_read_err, dev, ms->ms_multidev, sts, blknum);

anyerror:	/* error clean up */
		ms_putblk (bp, &bp_env);
		ms->ms_state = MST_CLOSED;
		return(ENXIO);
}

msd_close (dev, fp_flag, form)
	dev_t dev;
	int fp_flag;
	int form;		/* BLOCK_DEV or CHAR_DEV */
	{
	int sts;
	int subunit;
	ms_struct_t *ms;

	subunit = MS_SUBUNIT(dev);
	if (subunit > NMSD) return (ENODEV);
	ms = &(msd_struct[subunit]);

	/* in case device somehow isn't configured, stop now */
	if (ms->ms_state == MST_INVALID)
		return (ENODEV);

	lock_write(&ms->ms_openlock);
	switch (ms->ms_state) {
	    case MST_ACTIVE:    sts = ESUCCESS;	break;
	    case MST_QACTIVE:   sts = ESUCCESS;	break;
	    case MST_CLOSED:	sts = ENXIO;	break;
	    case MST_VALID:	sts = ENXIO;	break;
	    default:		sts = ENODEV;	break;
	}
	if (sts != ESUCCESS) {
		lock_write_done(&ms->ms_openlock);
		return (sts);
	}
	
	/*
	 * Since called on final close, mark partition as closed.  If
	 * all partitions on both block and char device are closed, the
	 * entire device is closed.
	 */
	if (form == BLOCK_DEV) {
		ms->ms_block_parts &= ~(1 << MS_PART(dev));
	} else { /* CHAR_DEV */
		ms->ms_char_parts  &= ~(1 << MS_PART(dev));
	}
	if (0 == (ms->ms_block_parts | ms->ms_char_parts)) {
		ms->ms_state = MST_CLOSED;
		/* could delete channel and change open logic */
	}
	lock_write_done(&ms->ms_openlock);
	return(ESUCCESS);
	}


msdread(dev, uio)
dev_t dev;
{
	dev_t	subdev = MS_SUBUNIT(dev);
	int error;

	if (error = ms_valid(dev,CLASS_DISK,NULL))
		return(error);

	return ms_physio(msdstrategy, dev, B_READ, minphys, uio);
}

msdwrite(dev, uio)
dev_t dev;
{
	dev_t	subdev = MS_SUBUNIT(dev);
	int error;

	if (error = ms_valid(dev,CLASS_DISK,NULL))
		return(error);

	return ms_physio(msdstrategy, dev, B_WRITE, minphys, uio);
}

msdioctl(dev, cmd, arg)
	dev_t dev;
	int cmd, arg;
	{
	int 	 	sts;
	struct	buf		*bp;
	bp_env_t	bp_env;
	ms_struct_t	*ms;

	if (sts = ms_valid(dev,CLASS_DISK,&ms)) return(sts);
	sts = 0;
	bp = ms_getblk(&bp_env, dev);
	switch(cmd) {
	    
	    case MSIOCCHKTRK:	sts = ms_check_track (ms, bp, (long)(*(int *)arg));
				break;

	    case MSIOCRECAL:	sts = ms_calibrate (ms, bp);
				break;

	    case MSIOCFMT:  {
		fmt_blk_t fmt;
		int cnt, size;

#if     SEC_BASE
		if (!privileged(SEC_FILESYS, 0))
#else
		if (suser(u.u_cred, &u.u_acflag))
#endif
			break;
		copyin ((char *) (*(struct fmt_blk_t **)arg), &fmt, sizeof(fmt_blk_t));
		cnt = fmt.fmt_defect_cnt;
		size = cnt * sizeof(md_defect_t);
		if ((cnt > 0) && (cnt <= MAX_DEFECTS)) {
			copyin (fmt.fmt_bad_blk, 
				bp->b_un.b_addr,
				cnt*sizeof(md_defect_t));
			/* BD_ALIGN ??; */
			/* BD_TAIL  ??; */
		} else if (cnt == 0) {
			/* make up null bdl ?? */
		} else {
			sts = EIO;
			break;
		}
		/* Only allow format if disk is already blown away (QACTIVE)
		 * OR if the only open partitions are the HDR_PART.  In any
		 * event, mark the disk as closed so any other i/o to the 
		 * disk (from someone who already has it open will fail).
		 * A better solution would have a new state (MST_FORMATTING)
		 * so lock can be unlocked.  Once the format operation is
		 * started, assume the disk is trashed, and set the open
		 * state to QACTIVE on exit
		 */
		lock_write(&ms->ms_openlock);
		if ((ms->ms_state != MST_QACTIVE) && 
		  ((ms->ms_block_parts|ms->ms_char_parts)!=(1<<HEADER_PART))) {
			lock_write_done(&ms->ms_openlock);
			sts = EACCES;
			break;
		}
		ms->ms_state = MST_CLOSED;
		sts = ms_format(ms, bp, fmt.fmt_disk_type, cnt);
		ms->ms_state = MST_QACTIVE;
		lock_write_done(&ms->ms_openlock);
		if (sts != ESUCCESS)
			goto ioerror;
		break;
		}

	    case MSIOCSECID:
		{
		sect_id_t sec;

		copyin((char *) (*(struct sect_id_t **)arg), &sec, sizeof(sec));
		sts = ms_read_sector_id(ms, bp, sec.sect_bcnt, sec.sect_pbn);
		if (sts != ESUCCESS)
			goto ioerror;
		}
		break;

	    case MSIOCRDLAY:   /* Return On Disk Layout Structure To User */
		{
		unsigned char *uaddr, *bptr;
		unsigned long laysize, numreads, blknum;

		/* ... Read Blocks from Disk, Copy to User Space ... */
		laysize = sizeof(layout_t) - 1;
		numreads = laysize >> DEV_BSHIFT;
		uaddr  = (unsigned char *) (*(struct layout **)arg);
		bptr   = (unsigned char *) bp->b_un.b_addr;
		blknum = 0;
		while (numreads-- > 0) {
			sts = ms_rw(ms, bp, blknum, DEV_BSIZE, CRQOP_MS_READ);
			if (sts != ESUCCESS) goto ioerror;
			sts = copyout (bptr, uaddr, DEV_BSIZE);
			if (sts != 0) goto ufaulterror;
			uaddr += DEV_BSIZE;
			blknum++;
		}

		/*  ... Last Block might be smaller ... */
		sts = ms_rw (ms, bp, blknum, DEV_BSIZE, CRQOP_MS_READ);
		if (sts != ESUCCESS) goto ioerror;
		sts = copyout (bptr, uaddr, (laysize&(DEV_BSIZE-1)) + 1);
		if (sts != 0) goto ufaulterror;
		
		break;
		}
				
	    case MSIOCWRLAY:  
		{
		unsigned char *uaddr, *bptr;
		unsigned long laysize, numreads, blknum, cksum;

		laysize = sizeof(layout_t) - 1;
		bptr   = (unsigned char *) bp->b_un.b_addr;

		/* ... Copy from user space to accumulate Checksum ...*/
		cksum  = 0;
		uaddr  = (unsigned char *) (*(struct layout **)arg);
		numreads = laysize >> DEV_BSHIFT;
		while (numreads-- > 0) {
			sts = copyin (uaddr, bptr, DEV_BSIZE);
			if (sts != 0) 
				goto ufaulterror;
			ms_acc_cksum(bptr, DEV_BSIZE, &cksum);
			uaddr += DEV_BSIZE;
		}

		/* ... Copy last short block in to accumulate Checksum ... */
		sts = copyin(uaddr, bptr, (laysize&(DEV_BSIZE-1)) + 1);
		if (sts != 0) 
			goto ufaulterror;
		numreads = (laysize & (DEV_BSIZE-1)) - sizeof(long) + 1;
		ms_acc_cksum(bptr, numreads, &cksum);

		/* ... Verify Checksum ... */
		if (cksum != *((unsigned long *) (bptr + numreads))) {
#if	MMAX_DEBUG
			printf(wrtlay_cksum_err, dev, 
				cksum, *((unsigned long *) (bptr+numreads)));
#endif	MMAX_DEBUG
			sts = EINVAL;
			break;
		}

		/* ... check first block for consistency ... */
		uaddr = (unsigned char *) (*(struct layout **)arg);
		sts = copyin(uaddr, bptr, DEV_BSIZE);
		if (sts != 0) 
			goto ufaulterror;
		sts = msd_check_layout(bptr);
		if (sts != ESUCCESS) { 
			sts = EINVAL;
			break;
		}

		/* ... Start over, this time copying blocks to disk ... */
		uaddr  = (unsigned char *) (*(struct layout **)arg);
		numreads = laysize >> DEV_BSHIFT;
		blknum = 0;
		while (numreads-- > 0) {
			sts = copyin (uaddr, bptr, DEV_BSIZE);
			if (sts != 0) 
				goto ufaulterror;
			sts = ms_rw(ms, bp, blknum, DEV_BSIZE,CRQOP_MS_WRITE);
			if (sts != ESUCCESS) 
				goto ioerror; 
			uaddr += DEV_BSIZE;
			blknum++;
		}

		/* ... Copy Last short block from user space to disk ... */
		sts = copyin(uaddr, bptr, (laysize&(DEV_BSIZE-1))+1);
		if (sts != 0) 
			goto ufaulterror;
		sts = ms_rw(ms, bp, blknum, DEV_BSIZE, CRQOP_MS_WRITE);
		if (sts != ESUCCESS) 
			goto ioerror; 

		/* ... Recalculate in core copy of partition table ... */
		uaddr = (unsigned char *) (*(struct layout **)arg);
		sts = copyin(uaddr, bptr, DEV_BSIZE);
		if (sts != 0) {
			sts = ms_rw (ms, bp, 0, DEV_BSIZE, CRQOP_MS_READ);
			if (sts != ESUCCESS) 
				goto ioerror;
		}
		ms_laycopy ( bptr, ms->ms_layout);
		ms->ms_state=MST_ACTIVE;
		break;
		}

ufaulterror:	sts = EFAULT;
ioerror:	break;

	    case MSIOCGEOM: 
		{
		unsigned char *uaddr, *bptr;
		int size = sizeof(struct dk_geom);

		uaddr  = (unsigned char *) (*(struct dk_geom **)arg);
		LASSERT(BUF_LOCK_HOLDER(bp));
		bp->b_bcount = size;
		sts = ms_geom(ms, bp);
		if (sts != ESUCCESS)
			goto ioerror;
		sts = copyout (bp->b_un.b_addr, uaddr, size);
		if (sts != 0)
			goto ufaulterror;
		break;
		}

	    case MSIOCBADTRACK:
		sts = ms_replace_bad_tracks(ms, bp, arg);
		break;

	    default:		sts = EINVAL;
				break;
	}
	ms_putblk(bp,&bp_env);
	return(sts);
	}

struct	buf *
msdstrategy(bp)
	struct	buf *bp;
	{
	int sts;
	int part;
	long	sizepart, leftover;
	ms_struct_t *ms;
	crq_ms_xfer_msg_t *mscmd;

	LASSERT(BUF_IS_LOCKED(bp));
	ASSERT(bp->b_bcount > 0);
	if (sts = ms_valid(bp->b_dev,CLASS_DISK,&ms)) {
		goto badstuff;
	}
	part = MS_PART(bp->b_dev);

	/* Trap bad block numbers */
	sizepart = ms->ms_layout->partitions[part].part_size;
	if ( (bp->b_blkno < 0) || (bp->b_blkno > (sizepart))) {
		if( (bp->b_blkno == (sizepart+1)) && (bp->b_flags & B_READ)) {
			/* Return 'EOF' by returning resid=count */
			goto kindofbad;
		} else {
			/* Return an Error */
			goto badblock;
		}
	}

	
	ASSERT(bp->b_bcount > 0);
	mscmd = ms_getcmd(bp,&sts,TRUE);
	if (sts != ESUCCESS) goto badstuff;
#if	MMAX_DEBUG
	ASSERT(chk_bdl(mscmd, 9));
#endif	MMAX_DEBUG
	mscmd->ms_xfer_hdr.em_msg_hdr.crq_msg_code = 
		(bp->b_flags & B_READ) ? CRQOP_MS_READ : CRQOP_MS_WRITE;
	mscmd->ms_xfer_lbn = bp->b_blkno +
				ms->ms_layout->partitions[part].part_off;

#if	MMAX_DEBUG
	ASSERT(chk_bdl(mscmd, 10));
#endif	MMAX_DEBUG
	leftover = (bp->b_blkno + (bp->b_bcount>>DEV_BSHIFT)) - sizepart;
	if (leftover > 0) {
		/* This i/o overlaps end of partition */
		/* Need to amend bdl_size (easy), and then put it back
		 * at end of IO (hard) for correct operation on multimax
		 */
		bp->b_resid = bp->b_bcount; 
		mscmd->ms_xfer_bcnt = bp->b_bcount - (leftover * DEV_BSIZE); 
	} else {
		mscmd->ms_xfer_bcnt = bp->b_resid = bp->b_bcount; 
	}

#if	UNIX_LOCKS && never
	ASSERT( bp->b_bdl_p->bun.bm.bd_valid);
	ASSERT( !(bp->b_bdl_p->bun.bm.bd_chain));
#endif	UNIX_LOCKS && never
	ASSERT( (bp->b_flags & B_PHYS) || ((bp->b_bcount & (DEV_BSIZE-1)) == 0));
	
#if	MMAX_DEBUG
	ASSERT(chk_bdl(mscmd, 11));
#endif	MMAX_DEBUG
	((crq_msg_t *) mscmd)->crq_msg_unitid = ms->ms_crq.crq_unitid;
	((crq_msg_t *) mscmd)->crq_msg_refnum = (long) bp;
	mscmd->ms_xfer_hdr.em_status_code = 0; /* EMC bug */
	ms_start (ms, bp, mscmd);

	return(bp);


badblock:
	sts = EIO;
#if	MMAX_DEBUG
	printf (badblock_err, bp->b_blkno, bp->b_dev);
#endif	MMAX_DEBUG

badstuff:
	bp->b_flags |= B_ERROR;
	bp->b_error  = sts;

kindofbad:
	bp->b_resid  = bp->b_bcount;
	iodone(bp);
	return(bp);
	}

/*
 * partition info.
 */
msdpsize(dev)
dev_t dev;
	{
	ms_struct_t *ms;
	int subunit, part, size;

	if ((subunit = MS_SUBUNIT(dev)) > NMSD)
		return(-1);

	ms = &msd_struct[subunit];

	lock_write(&ms->ms_openlock);
	if ((ms->ms_state != MST_ACTIVE) ||
	    ((part = MS_PART(dev)) >= MAX_V_PARTITIONS))
		size = -1;
	else
		size = ms->ms_layout->partitions[part].part_size;
	lock_done(&ms->ms_openlock);

	return(size);
}
	

/*
 * Support routines
 */

/*
 * Check On Disk Layout structure for validity:
 *	Assumes only first 1k is present.
 *	Should add cksum check and full 16k in future.
 */
ms_acc_cksum ( bptr, nbytes, cksum)
	unsigned char	*bptr;
	unsigned int	nbytes;
	unsigned long	*cksum;
{
	unsigned char	*bend;
	for (bend = bptr + nbytes; bptr < bend; bptr++) {
		*cksum = *cksum + *bptr;
	}
}

ms_laycopy ( disklay, corelay)
	layout_t	*disklay;
	ms_layout_t	*corelay;
{
	int	numparts;

	*((dk_geom_t *) &corelay->lay_geom) =
			*((dk_geom_t *) &disklay->lay_geom);

	bcopy(&(disklay->vol_label[0]),&(corelay->vol_label[0]),VOL_LABEL_SIZE);
	corelay->current_part = disklay->current_part;
	corelay->maximum_part = disklay->maximum_part;
	if (corelay->current_part > MAX_V_PARTITIONS)
		corelay->current_part = MAX_V_PARTITIONS;

	for (numparts = 0; numparts < MAX_V_PARTITIONS; numparts++) {
		corelay->partitions[numparts].part_off  =
			disklay->partitions[numparts].part_off;
		corelay->partitions[numparts].part_size =
			disklay->partitions[numparts].part_size;
		corelay->partitions[numparts].part_type =
			disklay->partitions[numparts].part_type;
	}
	return;
}

msd_check_layout (layout)
	layout_t	*layout;
{
	if ( (layout->lay_geom.byte_sector != NBYTESPERSECTOR) ||
	     (layout->current_part <= 0) ||
	     (layout->current_part >= 64)) {
		return(ENODEV);
	}
	/* ???? check revnums or magic nums ???? */
	return(ESUCCESS);
}

msd_null_layout (mslayout)
	ms_layout_t	*mslayout;
{
	int part;
	mslayout->lay_geom.byte_sector = NBYTESPERSECTOR;	/* 512 always */
	mslayout->current_part = mslayout->maximum_part = MAX_V_PARTITIONS;
	bcopy("NULL_LAYOUT!",&(mslayout->vol_label[0]),VOL_LABEL_SIZE);
	for (part=0; part < MAX_V_PARTITIONS; part++)  {
		mslayout->partitions[part].part_off  = 0;
		mslayout->partitions[part].part_size = MAX_PARTSIZE;
	}
	return(ESUCCESS);
}


ms_fix_layout (ms, bp)
ms_struct_t	*ms;
struct	buf	*bp;
{

	if (ms_geom(ms,bp) != ESUCCESS) {
		printf("ms_fix_layout: unable to get disk_geometry \n");
		return(EIO);
	}

/***
 ************************************************************************
 ***		*               *				*********
 ***		*               *	part5			*********
 ***	     	*               *				*********
 ***	part0	*     part1     *****************************************
 ***        	*               *****************			*
 ***		*               *****************   part6	 	*
 ***	     	*               *****************			*
 ***		*               *****************************************
 ***		*		*		*		*	*
 ***		*          	*    part2	*    part3	* part4 *
 ************************************************************************
 ******************************** 	 part8				*
 *******************************************************************************
 ***									| B B **
 ***			part 7   (entire disk)				| A L **
 ***									| D K **
 *******************************************************************************
 ***/

    {
    long offbk[MAX_V_PARTITIONS];	/* Offset table for partition config */
    long header_size, sects, blocks, part;
    ms_layout_t	*mslayout = ms->ms_layout;
    dk_geom_t	*dkgp = ((dk_geom_t *) (bp->b_un.b_addr));

    header_size = 0;
    /* mslayout->format_rev 	 = FORMAT_VERSION; */
    mslayout->current_part = MAX_V_PARTITIONS;
    mslayout->maximum_part = MAX_V_PARTITIONS;

    mslayout->lay_geom = *dkgp;

    sects = (dkgp->track_cylinder * dkgp->sector_track); /* sector per cyl */
    blocks = dkgp->total_avail_blocks;

    for (part=0; part < MAX_V_PARTITIONS; part++) {
	    mslayout->partitions[part].part_off  = 0;
	    mslayout->partitions[part].part_size = 0;
    }
    }
	return(ESUCCESS);
}

ms_rw (ms, bp, lbn, count, opcode)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		lbn, count;
	int		opcode;
	{
	bd_t *bdp;
	char *addr;
	crq_ms_xfer_msg_t	*mscmd;
	int sts;

	mscmd = ms_getcmd(bp,&sts,TRUE);
	if ( sts != ESUCCESS ) return(sts);
	mscmd->ms_xfer_lbn  = lbn;
	mscmd->ms_xfer_bcnt = count;
	bp->b_resid = count;
	return (ms_syncio (ms, mscmd, bp, opcode) );
	}

ms_format (ms, bp, disktype, badblk_cnt)
	struct	buf	*bp;		/* assumes bp and bdl preset */
	int	disktype;	/* FSD_CDC9715 or SMD_CDC9766 or etc */
	int	badblk_cnt;
	ms_struct_t *ms;
{
	crq_md_fmt_msg_t	*mscmd;
	int sts;
	bd_t			*md_bdl;

	mscmd  = (crq_md_fmt_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	/* ???  setup bdl to correct size ?? */
	mscmd->md_fmt_disk_type  = disktype;
	mscmd->md_fmt_badblk_cnt = badblk_cnt;
	sts = create_bdl(bp, &md_bdl);
	if (sts != ESUCCESS ) {
		ms_freecmd((crq_ms_xfer_msg_t *)mscmd);
		return(sts);
	}
	mscmd->md_fmt_desc = md_bdl;
	sts = ms_syncio(ms, mscmd, bp, CRQOP_MD_FORMAT_DRIVE);
	dealloc_bdl(md_bdl);
	return (sts);
}

ms_calibrate (ms, bp)
	ms_struct_t	*ms;
	struct	buf	*bp;
	{
	crq_ms_xfer_msg_t	*mscmd;
	int sts;

	mscmd  = ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	return (ms_syncio (ms, mscmd, bp, CRQOP_MD_RECALIBRATE) );
	}

ms_check_track (ms, bp, track)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		track;
	{
	crq_md_chktrk_msg_t	*mscmd;
	int sts;

	mscmd  = (crq_md_chktrk_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	mscmd->md_chktrk_pbn = track;
	return (ms_syncio (ms, mscmd, bp, CRQOP_MD_CHECK_TRACK) );
	}

ms_seek_track (ms, bp, track)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		track;
	{
	crq_md_seek_msg_t	*mscmd;
	int sts;

	mscmd = (crq_md_seek_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	mscmd->md_seek_pbn = track;
	return (ms_syncio (ms, mscmd, bp, CRQOP_MD_SEEK) );
	}

ms_replace_bad_tracks (ms, bp, lbn)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		lbn;		/* assumes bdl set up somehow ?? */
{
	crq_md_rpltrk_msg_t	*mscmd;
	int sts;
	bd_t			*ms_bdl;

	mscmd = (crq_md_rpltrk_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	mscmd->md_rpltrk_lbn = lbn;
	sts = create_bdl (bp, &ms_bdl);
	if (sts != ESUCCESS ) {
		ms_freecmd ((crq_ms_xfer_msg_t *) mscmd);
		return(sts);
	}
	mscmd->md_rpltrk_desc = ms_bdl;
	sts = ms_syncio (ms, mscmd, bp, CRQOP_MD_REPLACE_TRACK);
	dealloc_bdl (ms_bdl);
	return (sts);
}

ms_read_bad_blocks (ms, bp, bcount)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		bcount;		/* assumes bdl set up somehow ?? */
{
	crq_md_rdbad_msg_t	*mscmd;
	int sts;
	bd_t			*ms_bdl;

	mscmd = (crq_md_rdbad_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	sts = create_bdl (bp, &ms_bdl);
	if (sts != ESUCCESS ) {
		ms_freecmd((crq_ms_xfer_msg_t *) mscmd);
		return(sts);
	}
	mscmd->md_rdbad_desc = ms_bdl;
	mscmd->md_rdbad_bcnt = bcount;
	sts = ms_syncio (ms, mscmd, bp, CRQOP_MD_READ_BAD_BLOCK);
	dealloc_bdl(ms_bdl);
	return (sts);
}

ms_read_drive_prom (ms, bp, bcount)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		bcount;		/* assumes bdl set up somehow ?? */
{
	crq_md_rdtyp_msg_t	*mscmd;
	int sts;
	bd_t			*ms_bdl;

	mscmd = (crq_md_rdtyp_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	sts = create_bdl (bp, &ms_bdl);
	if (sts != ESUCCESS ) {
		ms_freecmd((crq_ms_xfer_msg_t *) mscmd);
		return(sts);
	}
	mscmd->md_rdtyp_desc = ms_bdl;
	mscmd->md_rdtyp_bcnt = bcount;
	sts = ms_syncio (ms, mscmd, bp, CRQOP_MD_READ_DT_PROM);
	dealloc_bdl(ms_bdl);
	return (sts);
}

ms_read_sector_id (ms, bp, bcount, pbn)
	ms_struct_t	*ms;
	struct	buf	*bp;
	long		bcount;		/* assumes bdl set up somehow ?? */
	long		pbn;
{
	crq_md_rdsid_msg_t	*mscmd;
	int sts;
	bd_t			*ms_bdl;

	mscmd = (crq_md_rdsid_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	sts = create_bdl (bp, &ms_bdl);
	if (sts != ESUCCESS ) {
		ms_freecmd((crq_ms_xfer_msg_t *) mscmd);
		return(sts);
	}
	mscmd->md_rdsid_desc = ms_bdl;
	mscmd->md_rdsid_bcnt = bcount;
	mscmd->md_rdsid_pbn  = pbn;
	sts = ms_syncio (ms, mscmd, bp, CRQOP_MD_READ_SECT_ID);
	dealloc_bdl (ms_bdl);
	return (sts);
}

ms_geom (ms, bp)
	ms_struct_t	*ms;
	struct	buf	*bp;
	{
	crq_md_rdgeom_msg_t	*mscmd;
	int sts;

	mscmd = (crq_md_rdgeom_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS )
		return(sts);
	mscmd->md_rdgeom_bcnt = bp->b_bcount;
	mscmd->md_rdgeom_dkgm = (struct dk_geom *) bp->b_un.b_addr;
	return( ms_syncio (ms, mscmd, bp, CRQOP_MD_READ_GEOMETRY));
	}

/*****************************************************************************
 *
 * NAME:
 *	ms_disksort
 *
 * DESCRIPTION:
 *	This version of ms_disksort uses a single request queue, maintained as
 *	the crq command queue.  To avoid excessive time where the device is
 *	locked out, the sort routine unhooks pending requests from the queue
 *	beginning with the second entry and releases the device crq lock.
 *	This sub-queue is then sorted and hooked back up to the main queue.
 *
 * ARGUMENTS:
 *	mcp_p		- masstore command packet for the current request
 *	crq_p		- pointer to the unit crq
 *	s		- previous spl level.
 *
 * RETURN VALUE:
 *	none
 *
 * SIDE EFFECTS:
 *	none
 *
 * EXCEPTIONS:
 *	none
 *
 * ASSUMPTIONS:
 *	This algorithm does not consider where the disk heads end up after the
 *	current request is finished, only where they are when the request
 *	starts.  The assumptions are that 1) the "cylinder groups" of the file
 *	system preclude any single request's access spanning cylinders and 2)
 *	if such an access were made and were followed by a request to access
 *	the first cylinder again, that the ensuing non-optimal behavior is a
 *	minor concern relative to the complexity that would be introduced into
 *	this algorithm by attempting to account for that case.
 */

#define ms_xfer_flink	ms_xfer_hdr.em_msg_hdr.crq_msg_links.dbl_fwd
#define ms_xfer_blink	ms_xfer_hdr.em_msg_hdr.crq_msg_links.dbl_bwd

ms_disksort(mcp_p, crq_p, s)
	crq_ms_xfer_msg_t	*mcp_p;
	crq_t			*crq_p;
	int			s;
{
	register crq_ms_xfer_msg_t *firstmcp;	/* First in sorted chain  */
	register crq_ms_xfer_msg_t *lastmcp;	/* Last in sorted chain	*/
	register crq_ms_xfer_msg_t *themcp;	/* Walking pointer */
	register int		curr_lbn;	/* lbn of PENDING request */
	register int		new_lbn;	/* lbn of request to add */

/*
 *	If the queue contains 0 or 1 elements, insert the request at the tail
 *	and be done.  Note that if the forward and backward links of the
 *	command queue contain the same address, there is either 0 or 1 element
 *	present.  While there is the possibility that the first element is not
 *	yet "in progress" and could, therefore, be included in the queue sort,
 *	this is not done because 1) if the disk is that lightly loaded, who
 *	cares if seek ordering is perfect? and 2) the algorithm here would be
 *	complicated by then having to worry about whether the first element
 *	were already being timed and should not, therefore, have another
 *	element inserted in front of it.  The assumption is basically that any
 *	first element is already "in progress" with respect to the EMC and
 *	being timed with respect to the host.  This keeps things simple and
 *	has no dire consequences if the assumption is wrong.
 *
 *	In the case of multiple elements, unhook the chain of requests past
 *	the first masstore command packet and release the lock on the crq.
 */


	if(crq_p->crq_cmd.dbl_fwd == crq_p->crq_cmd.dbl_bwd) {/* 0 or 1 cmd */
	    insque(mcp_p, crq_p->crq_cmd.dbl_bwd);
	    return (crq_p->crq_cmd.dbl_fwd == (dbl_link_t *)mcp_p);
	}

	firstmcp = (crq_ms_xfer_msg_t *)crq_p->crq_cmd.dbl_fwd;
	lastmcp = (crq_ms_xfer_msg_t *)crq_p->crq_cmd.dbl_bwd;
	curr_lbn = firstmcp->ms_xfer_lbn;
	firstmcp = (crq_ms_xfer_msg_t *)firstmcp->ms_xfer_flink;
	((crq_ms_xfer_msg_t *)(firstmcp->ms_xfer_blink))->ms_xfer_flink=
		(dbl_link_t *)&crq_p->crq_cmd.dbl_fwd;
	crq_p->crq_cmd.dbl_bwd = firstmcp->ms_xfer_blink;
	crq_unlock(&crq_p->crq_slock);
	splx(s);

/*
 *	Now that the crq has been released for access by the device, if
 *	necessary, we can put the current request in sorted order.  This gets
 *	complicated due to the special cases possible.
 *
 *	The queue can be in one of three states once the cases of 0 or 1
 *	entries are eliminated.  The remaining cases are:
 *
 *	1) the queue is sorted in ascending order and all entries are for
 *	   logical blocks beyond the current one.
 *	2) same as 1, but all entries are for blocks previous to the
 *	   current one.
 *	3) the queue consists of two sub-queues: a 1 and a 2 (in that order).
 *	   The dividing line between the two sections is at some arbitrary
 *	   intermediate entry.
 *
 *	For each of the three possible queue states, the new entry may, of
 *	course, belong at the beginning, middle, or end.  In the third case,
 *	there are really two beginnings and ends.
 *
 *	We proceed by hooking the new request at the head of the queue so
 *	that all links point to something valid within the piece of the
 *	request queue we cut off.  We then search forward or backward from
 *	there based on whether the new request is for a block closer to the
 *	inside or outside track from the current request.  In either case,
 *	before walking the queue, there are special cases to deal with.
 *
 *	If the logical block for the starting entry (head or tail, depending
 *	on direction of search)	compares to the new logical block in the same
 *	sense as it compares to the current block (where the disk head is now)
 *	the new entry belongs right where we have hooked it up and we need
 *	only re-establish the proper links to the complete queue to finish.
 *
 *	Otherwise, we begin walking the queue in forward or reverse order
 *	until we find the spot where the new request fits in.  If we walk
 *	the entire queue, we have, again, already put things in sorted
 *	order, but this time the entry belongs at the "other end" from where
 *	we began looking.
 */

	/* Hook up the new request so it is at the beginning of the unlinked
	 * request chain.
	 */
	mcp_p->ms_xfer_flink	= (dbl_link_t *)firstmcp;
	mcp_p->ms_xfer_blink	= (dbl_link_t *)lastmcp;
	firstmcp->ms_xfer_blink = (dbl_link_t *)mcp_p;
	lastmcp->ms_xfer_flink	= (dbl_link_t *)mcp_p;
	new_lbn = mcp_p->ms_xfer_lbn;
	themcp = mcp_p;

	/* Determine the direction of the scan based on the relative logical
	 * block number between the current active request and the new one.
	 * For the first set of conditionals, the new request is for a logical
	 * block closer to the spindle that the current request (ie - it
	 * should be serviced on this scan).
	 */
	if(new_lbn >= curr_lbn) {

	/* If the new request belongs before its successor on the chain and
	 * both refer to blocks in this scan...
	 */
		if( ((firstmcp->ms_xfer_lbn >= new_lbn) &&
		     (firstmcp->ms_xfer_lbn >= curr_lbn)) ||

	/* or the new request belongs after its successor and the successor
	 * belongs on the next scan of the arm (remember, the queue is already
	 * known to be sorted before this request came along)...
	 */
		     ((firstmcp->ms_xfer_lbn <= new_lbn) &&
		      (firstmcp->ms_xfer_lbn <= curr_lbn))	) {

	/* then things are sorted as they stand and the new request should
	 * be serviced next.
	 */
				firstmcp = themcp;
				goto sorted;
	    	}

	/* Otherwise, scan forward through the part of the list representing
	 * the current scan to position the new request at the proper place.
	 */
	    while( ( ((crq_ms_xfer_msg_t *)(themcp->ms_xfer_flink))->
			ms_xfer_lbn < new_lbn ) &&
		   ( ((crq_ms_xfer_msg_t *)(themcp->ms_xfer_flink))->
			ms_xfer_lbn > curr_lbn ) ) {
		        themcp = (crq_ms_xfer_msg_t *)themcp->ms_xfer_flink;

	/* If we get to the end of the list of requests, the new request is
	 * already in proper sorted order and is the last of the outstanding
	 * requests to process.  That is, all outstanding requests are for
	 * this scan and the new one is at the end.
	 */
			if(themcp->ms_xfer_flink == (dbl_link_t *)mcp_p) {
		    		lastmcp = mcp_p;
		    		goto sorted;
			}
	    }

	/* The second set of conditionals obtains when the new request belongs
	 * in the next scan of the disk arm.
	 */
	} else {

	/* If the new request belongs after its predecessor on the chain and
	 * both refer to blocks in the next scan...
	 */
	    if( ((lastmcp->ms_xfer_lbn <= new_lbn) &&
		 (lastmcp->ms_xfer_lbn <= curr_lbn)) ||

	/* or the new request belongs before its predecessor and the
	 * predecessor belongs on the current scan of the arm...
	 */
		((lastmcp->ms_xfer_lbn >= new_lbn) &&
		 (lastmcp->ms_xfer_lbn >= curr_lbn))	) {

	/* then things are sorted as they stand and the new request should
	 * be serviced last on the next scan.
	 */
		lastmcp = themcp;
		goto sorted;
	     }

	/* Otherwise, scan backward through the part of the list representing
	 * the next scan to position the new request at the proper place.
	 */
	    while( ( ((crq_ms_xfer_msg_t *)(themcp->ms_xfer_blink))->
		   ms_xfer_lbn > new_lbn ) &&
		   ( ((crq_ms_xfer_msg_t *)(themcp->ms_xfer_blink))->
		   ms_xfer_lbn < curr_lbn ) ) {
		themcp = (crq_ms_xfer_msg_t *)themcp->ms_xfer_blink;

	/* If we get to the end of the list of requests, the new request is
	 * already in proper sorted order and should be the next of the
	 * outstanding requests to process.  That is, all outstanding requests
	 * are for the next scan and the new request should be first among
	 * them.
	 */
		if(themcp->ms_xfer_blink == (dbl_link_t *)mcp_p) {
		    firstmcp = mcp_p;
		    goto sorted;
		}
	    }

	/* Insque needs the predecessor to link the current request into the
	 * chain, so in the case of the backward search, back up to the
	 * predecessor of the node identified above as themcp.
	 */
	    themcp = (crq_ms_xfer_msg_t *)themcp->ms_xfer_blink;
	}
	remque(mcp_p);
	insque(mcp_p, themcp);

/*
 *	The queue has been sorted and the first and last elements have been
 *	determined by the time we get here.  Now re-obtain the device lock.
 *	Note that the item we left at the head of the queue originally may
 *	have been de-queued by the device by now.  If so, and if we are
 *	collecting statistics, keep count of it.  Re-hookup the newly sorted
 *	queue to the tail of the list of requests.
 */

sorted:
	spl7();
	crq_lock(&crq_p->crq_slock);
	themcp = (crq_ms_xfer_msg_t *)crq_p->crq_cmd.dbl_bwd;
	firstmcp->ms_xfer_blink = (dbl_link_t *)themcp;
	lastmcp->ms_xfer_flink = (dbl_link_t *)&crq_p->crq_cmd.dbl_fwd;
	themcp->ms_xfer_flink = (dbl_link_t *)firstmcp;
	crq_p->crq_cmd.dbl_bwd = (dbl_link_t *)lastmcp;

	/* Last I/O may have been processed, check again */
	return  (firstmcp == (crq_ms_xfer_msg_t *) crq_p->crq_cmd.dbl_fwd);
}

/*
 * Return the disk unit number and name for table TBL_DKINFO
 */
char *
msdinfo(dkn, unit)
        int dkn;
        int *unit; /* RETURN */
{
        int i;
	ms_struct_t *ms = &(msd_struct[0]);

        for (i = 0; i < NMSD; i++, ms++)
                if (ms->ms_class == CLASS_DISK && ms->ms_dkn == dkn) {
                        *unit = MS_SUBUNIT(ms->ms_dev);
                        return ("md");
                }
        
        /* dkn not found */
        return (NULL);
}
        
/*	---------------  end of msd.c	---------------	*/
