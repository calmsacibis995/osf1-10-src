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
static char	*sccsid = "@(#)$RCSfile: hd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:23 $";
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
 *
 *  Copyright 1988, 1989 by Intel Corporation
 *
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 *    Copyright 1988  Intel Corporation.
 */

/*
 *  AT Hard Disk Driver
 *  Copyright Ing. C. Olivetti & S.p.A. 1989
 *  All rights reserved.
 *
 */
/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/


#include <hd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/mode.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <i386/ipl.h>
#include <i386/AT386/atbus.h>
#include <i386/AT386/hdreg.h>
#include <i386/AT386/disk.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>
#include <sys/vmparam.h>
#include <sys/uio.h>
#include <vm/vm_kern.h>

#define B_MD1		0x10000000
#define	B_FORMAT	0x20000000
#define WHOLE_DISK(unit) ((unit << 4) + PART_DISK)

#define NOP_DELAY {asm("nop");}

#define PRIBIO	20
#define PAGESIZ 	4096

#define CMOS_ADDR	0x70	/* I/O port address for CMOS ram addr */
#define CMOS_DATA	0x71	/* I/O port address for CMOS ram data */
#define HDTBL		0x12	/* byte offset of the disk type in CMOS ram */

#ifndef	NULL
#define NULL	0
#endif

#define ONE_MEGABYTE	0x00100000	/* Used in dump printf */
int hd_dump_size = 8192;		/* Number of bytes to dump at once */

/* From sys/systm.h */
struct buf *geteblk();
extern vm_offset_t map_pva_kva();
extern vm_offset_t alloc_kva();

extern u_int minphys();

#define b_cylin b_resid

struct hh 				hh;
struct isa_dev *hdinfo[NHD];
struct alt_info 			alt_info[NDRIVES];
struct buf hdbuf[NDRIVES]; 	/* buffer for raw io */
struct buf hdunit[NDRIVES];

partition_t	partition_struct[NDRIVES][V_NUMPAR];
int	hd_blkopen[NDRIVES];
int	hd_chropen[NDRIVES];

typedef struct {
	unsigned short	ncylinders;
	unsigned short	nheads;
	unsigned short	precomp;
	unsigned short	landzone;
	unsigned short	nsecpertrack;
		} hdisk_t;
hdisk_t	hdparams[NDRIVES];	
hdisk_t rlparams[NDRIVES];

int ndrives = 1;

#define HD_DEBUG
#ifdef HD_DEBUG
#include <sys/syslog.h>
unsigned hddebug = 0;
#define TRACE(F,X)	if (hddebug & (F)) log X
#else
#define TRACE(F,X)
#endif

int hdstrategy();
int hdprobe(), hdslave(), hdattach();
int hdintr();

int (*hdintrs[])() = {hdintr, 0};

struct  isa_driver      hddriver = {
        hdprobe, hdslave, hdattach, "hd", 0, 0, 0};

static ihandler_t hd_handler;
static ihandler_id_t *hd_handler_id;

hdprobe(ctlr)
struct isa_ctlr *ctlr;
{
        int port_status = (int)ctlr->ctlr_addr + (PORT_STATUS-PORT_DATA);
        u_char stat = inb(port_status);

        if ((stat & STAT_READY) == STAT_READY) {
		hd_handler.ih_level = ctlr->ctlr_pic;
		hd_handler.ih_handler = ctlr->ctlr_intr[0];
		hd_handler.ih_resolver = i386_resolver;
		hd_handler.ih_rctlr = ctlr;
		hd_handler.ih_hparam[0].intparam = ctlr->ctlr_ctlr;
		if ((hd_handler_id = handler_add(&hd_handler)) != NULL)
			handler_enable(hd_handler_id);
		else
			panic("Unable to add hd interrupt handler");
		hh.k_window = (caddr_t)alloc_kva(SECSIZE*256+PAGESIZ);
                return 1;
        }
	return 0;
}

/*
 * hdslave:
 *
 *      Actually should be thought of as a slave probe.  Since we are
 *      a driver for an AT, and only one ESDI controller lives in the
 *      normal AT, we assume the controller and check for one or two
 *      hard disks that may be attached.
 *
 */

hdslave(dev)
struct isa_dev          *dev;
{
        int slave = dev->dev_slave;
        uchar   hdtype;

        outb(CMOS_ADDR, HDTBL);
        NOP_DELAY
        hdtype = inb(CMOS_DATA);
        dev->dev_type = hdtype;

        if ((slave == 0) && (hdtype & 0xf0))
                return(1);
        if ((slave == 1) && (hdtype & 0x0f))
                return(1);
        return(0);
}

/*
 * hdattach:
 *
 *      Attach the drive unit that has been successfully probed.  For the
 *      AT ESDI drives we will initialize all driver specific structures
 *      and complete the controller attach of the drive.
 *
 */

hdattach(dev)
struct  isa_dev *dev;
{
        int     unit = dev->dev_unit;
        unsigned long n;
        unsigned char *tbl;

        hdinfo[unit] = dev;

        ndrives = max(ndrives, unit+1);

        n = *(unsigned long *)phystokv(dev->dev_addr);
        tbl = (unsigned char *)(phystokv((n&0xffff) + ((n >> 12)&0xffff0) ));

        hdparams[unit].ncylinders = *tbl++;
        hdparams[unit].ncylinders |= *tbl++ << 8;
        hdparams[unit].nheads = *tbl++ & 0x00ff;
        tbl +=2;
        hdparams[unit].precomp = *tbl++;
        hdparams[unit].precomp |= *tbl++ << 8;
        tbl +=5;
        hdparams[unit].landzone = *tbl++;
        hdparams[unit].landzone |= *tbl++ << 8;
        hdparams[unit].nsecpertrack = *tbl;
        rlparams[unit] = hdparams[unit];
        printf("hd%d: (hard disk type TBD [0x%x]) irq = %d\n",
                unit, dev->dev_type, dev->dev_mi->ctlr_pic);
        printf("\t%dMB, cyls %d, heads %d, secs %d, precomp %d\n",
                (hdparams[unit].ncylinders * hdparams[unit].nheads
				* hdparams[unit].nsecpertrack * 512) / 1000000,
                hdparams[unit].ncylinders, hdparams[unit].nheads,
		hdparams[unit].nsecpertrack, hdparams[unit].precomp);
	BUF_LOCKINIT(&hdbuf[unit]);
        hdunit[unit].b_active = 0;
        hdunit[unit].b_actf = hdunit[unit].b_actl = 0;
        hh.reset_request = 1;
        return;

}

hdopen(dev, flags, fmt)
dev_t dev;
int flags, fmt;
{
	unsigned char unit, part, n;

	unit = UNIT(dev);
	part = PARTITION(dev);

        if (unit >= ndrives || part >= V_NUMPAR
		|| !hdinfo[unit] || !hdinfo[unit]->dev_alive) {
                return(ENXIO);
	}

        if (partition_struct[unit][PART_DISK].p_size == 0) {
                getvtoc(unit);
        }

	if (!(partition_struct[unit][part].p_flag & V_VALID)
		&& (suser(u.u_cred, &u.u_acflag) != 0)) {
		return(ENXIO);
	}
	partition_struct[unit][part].p_flag |= V_OPEN;
	switch (fmt) {
		case S_IFCHR:	hd_chropen[unit] |= (1<<part); break;
		case S_IFBLK:	hd_blkopen[unit] |= (1<<part); break;
		default:	panic("hdopen fmt");
	}
	return(0);
}

hdclose(dev, flag, fmt)
dev_t dev;
int flag, fmt;
{
	unsigned char unit, part;

	unit = UNIT(dev);
	part = PARTITION(dev);

	switch (fmt) {
		case S_IFCHR:	hd_chropen[unit] &= ~(1<<part); break;
		case S_IFBLK:	hd_blkopen[unit] &= ~(1<<part); break;
		default:	panic("hdopen fmt");
	}
	if (((hd_chropen[unit]|hd_blkopen[unit]) & (1<<part)) == 0) 
		partition_struct[unit][part].p_flag &= ~V_OPEN;

	return(0);
}

hdread(dev,uio)
register short  dev;
struct uio 	*uio;
{
	struct buf *bp;
	int error;

	bp = &hdbuf[UNIT(dev)];
	BUF_LOCK(bp);

	error = physio(hdstrategy, bp, dev, B_READ, minphys, uio);

	BUF_UNLOCK(bp);
	return(error);
}

hdwrite(dev,uio)
dev_t	 	dev;
struct uio	*uio;
{
	struct buf *bp;
	int error;

	bp = &hdbuf[UNIT(dev)];
	BUF_LOCK(bp);

	error = physio(hdstrategy, bp, dev, B_WRITE, minphys, uio);

	BUF_UNLOCK(bp);
	return(error);
}

hdioctl(dev, cmd, arg, mode)
dev_t dev;
int cmd;
caddr_t arg;
int mode;
{
	unsigned char unit, part;
	union io_arg  *arg_kernel; 
	unsigned int i, snum;
	struct absio *absio_kernel;
	union vfy_io *vfy_io_kernel;
	struct disk_parms *disk_parms;
	struct buf *bp;
	int xcount, errcode = 0;
	int intlv, track;
	int s;

			 
	unit = UNIT(dev);
	part = PARTITION(dev);

	switch (cmd) {
	case V_CONFIG:
		arg_kernel = (union io_arg *)arg;
		if (arg_kernel->ia_cd.secsiz != SECSIZE) {
			/* changing sector size NOT allowed */
		  	errcode = EINVAL;
			break;
		}
		hdparams[unit].ncylinders = arg_kernel->ia_cd.ncyl;
		hdparams[unit].nheads = arg_kernel->ia_cd.nhead;
		hdparams[unit].nsecpertrack = arg_kernel->ia_cd.nsec;
TRACE(2,(LOG_DEBUG, "hdioctl: V_CONFIG (%s) ncyl %d nheads %d sec/trk %d\n",
			u.u_comm, hdparams[unit].ncylinders,
			hdparams[unit].nheads, hdparams[unit].nsecpertrack));
		setcontroller(unit);
		break;

	case V_REMOUNT:
TRACE(2,(LOG_DEBUG, "hdioctl: V_REMOUNT (%s) unit %d\n", u.u_comm, unit));
		getvtoc(unit);	
		break;

	case V_ADDBAD:
		/* this adds a bad block to IN CORE alts table ONLY */
		arg_kernel = (union io_arg *)arg;
		alt_info[unit].alt_sec.alt_used++;
		alt_info[unit].alt_sec.alt_bad[
		alt_info[unit].alt_sec.alt_used]=  arg_kernel->ia_abs.bad_sector;
TRACE(2,(LOG_DEBUG, "hdioctl: V_ADDBAD (%s) blkno %d\n", u.u_comm, arg_kernel->ia_abs.bad_sector));
		break;

	case V_GETPARMS:
TRACE(2,(LOG_DEBUG, "hdioctl: V_GETPARMS (%s)\n", u.u_comm));
		disk_parms = (struct disk_parms *)arg;
		
		disk_parms->dp_type = DPT_WINI;
		disk_parms->dp_heads = hdparams[unit].nheads;
		disk_parms->dp_cyls = hdparams[unit].ncylinders;
		disk_parms->dp_sectors  = hdparams[unit].nsecpertrack;
  		disk_parms->dp_dosheads = rlparams[unit].nheads;
		disk_parms->dp_doscyls = rlparams[unit].ncylinders;
		disk_parms->dp_dossectors  = rlparams[unit].nsecpertrack;
		disk_parms->dp_secsiz = SECSIZE;
		disk_parms->dp_ptag = partition_struct[unit][part].p_tag;
		disk_parms->dp_pflag = partition_struct[unit][part].p_flag;
		disk_parms->dp_pstartsec = partition_struct[unit][part].p_start;
		disk_parms->dp_pnumsec = partition_struct[unit][part].p_size;
		break;

	case V_FORMAT:
TRACE(2,(LOG_DEBUG, "hdioctl: V_FORMAT (%s)\n", u.u_comm));
		if (suser(u.u_cred, &u.u_acflag))
			return(EACCES);

		if (dev != WHOLE_DISK(unit)) {
			return(EINVAL);
		}
		if ((hd_blkopen[unit]|hd_chropen[unit]) != (1 << PART_DISK)) {
			/* Don't allow format if ANY partition is open except
			 * the raw partition */
			return(EBUSY);
		}
		return(EOPNOTSUPP);

		arg_kernel = (union io_arg *)arg;

		bp = &hdbuf[unit];
		BUF_LOCK(bp);

		for (i = 0; i < SECSIZE; i++) 
			hh.interleave_tab[i] = 0; /* 0 means not taken */
	/*
	 * THIS INTERLEAVE TABLE IS ABSOLUTELY WRONG! There should
	 * be '00' bytes between each sector number in the list.
	 */

		/* start formatting at sector one, not sector zero */
		intlv = arg_kernel->ia_fmt.intlv;
		i = 0;
		snum = 1;
		hh.interleave_tab[i] = snum++;	
		do {
			i = (i + intlv) % hdparams[unit].nsecpertrack;
			while (hh.interleave_tab[i] != 0) { /* taken */
				i = (i + 1) % 
					(hdparams[unit].nsecpertrack );	
				if (snum > hdparams[unit].nsecpertrack)
					goto fmt1;
			}
			hh.interleave_tab[i] = snum++;	
		
		} while (snum <= hdparams[unit].nsecpertrack);
fmt1:
		/* format all tracks in request */

		track = arg_kernel->ia_fmt.start_trk;
		bp->b_dev = dev;
		for (i=0; i <arg_kernel->ia_fmt.num_trks; i++) {
			/* Each format request formats 1 entire track */
			event_clear(&bp->b_iocomplete);
			bp->b_flags = B_BUSY|B_WRITE|B_MD1|B_FORMAT;
			bp->b_blkno = track * hdparams[unit].nsecpertrack;
			bp->b_bcount = hdparams[unit].nsecpertrack * SECSIZE;
			bp->b_un.b_addr = (char *)hh.interleave_tab;
			hdstrategy(bp);
			if (errcode = biowait(bp)) {
				break;
			}
			track++;
		}
		BUF_UNLOCK(bp);
		break;

	case V_PDLOC:
TRACE(2,(LOG_DEBUG, "hdioctl: V_PDLOC (%s)\n", u.u_comm));
		{
		unsigned int *pd_loc;
		
		pd_loc = (unsigned int *)arg;
		*pd_loc = (unsigned int)(hh.start_of_unix[unit]) + PDLOCATION; 
		break;
		}

	case V_RDABS:
		/* V_RDABS is relative to head 0, sector 0, cylinder 0 */
		if (suser(u.u_cred, &u.u_acflag))
			return(EACCES);

		bp = geteblk(SECSIZE);
		absio_kernel = (struct absio *)arg;
		bp->b_flags = B_BUSY|B_READ|B_MD1;	/* MD1 is be absolute */
		bp->b_blkno = absio_kernel->abs_sec;
TRACE(2,(LOG_DEBUG, "hdioctl: V_RDABS (%s) blkno %d\n", u.u_comm, bp->b_blkno));
		bp->b_dev = WHOLE_DISK(unit);
		bp->b_bcount = SECSIZE;
		hdstrategy(bp);
		errcode = biowait(bp);
		bp->b_flags &= ~B_MD1;
		if (!errcode) {
			errcode = copyout(bp->b_un.b_addr,
				absio_kernel->abs_buf, SECSIZE);
#ifdef HD_DEBUG
		} else {
			printf("hd: read failure on V_RDABS ioctl\n");
#endif
		}
		brelse(bp);
		break;

	case V_WRABS:
		/* V_WRABS is relative to head 0, sector 0, cylinder 0 */
		if (suser(u.u_cred, &u.u_acflag))
			return(EACCES);

		bp = geteblk(SECSIZE);
		absio_kernel = (struct absio *)arg;
		if (copyin(absio_kernel->abs_buf, bp->b_un.b_addr, 
				SECSIZE) !=0 ) {
			errcode = ENXIO;
			brelse(bp);
			break;
		}
		bp->b_flags = B_BUSY|B_WRITE|B_MD1;	/* MD1 is be absolute */
		bp->b_blkno = absio_kernel->abs_sec;
TRACE(2,(LOG_DEBUG, "hdioctl: V_WRABS (%s) blkno %d\n", u.u_comm, bp->b_blkno));
		bp->b_dev = WHOLE_DISK(unit);
		bp->b_bcount = SECSIZE;
		hdstrategy(bp);
		errcode = biowait(bp);
		bp->b_flags &= ~B_MD1;
#ifdef HD_DEBUG
		if (errcode) {
			printf("hd: write failure on V_WRABS ioctl\n");
		}
#endif
		brelse(bp);
		break;

	case V_VERIFY:
		if (suser(u.u_cred, &u.u_acflag))
			return(EACCES);

		bp = geteblk(PAGESIZ);
		vfy_io_kernel = (union vfy_io *)arg;
		bp->b_flags = B_BUSY|B_READ;
		bp->b_blkno = vfy_io_kernel->vfy_in.abs_sec;
TRACE(2,(LOG_DEBUG, "hdioctl: V_VERIFY (%s) blkno %d\n",
					u.u_comm, bp->b_blkno));
		bp->b_dev = WHOLE_DISK(unit);
		xcount = vfy_io_kernel->vfy_in.num_sec;
		vfy_io_kernel->vfy_out.err_code = 0;
		snum = PAGESIZ >> 9;
		bp->b_flags |= B_MD1;
		while (xcount > 0) {
			i = (xcount > snum) ? snum : xcount;
			bp->b_bcount = i << 9;
			hdstrategy(bp);
			if (biowait(bp)) {
				vfy_io_kernel->vfy_out.err_code = BAD_BLK;
				break;
			}
			xcount -= i;
			bp->b_blkno += i;
			event_clear(&bp->b_iocomplete);
		}
		bp->b_flags &= ~B_MD1;
		brelse(bp);
		break;

	default:
		errcode = EINVAL;
	}
	return(errcode);
}

hdstrategy(bp)
struct	buf	*bp;
{
	struct	buf	*dp;
	partition_t	*pp;
	unsigned char unit, partition;
	unsigned int sz;
	unsigned int s;

	if (bp->b_bcount == 0) {
		goto done;
	}

	unit = UNIT((bp->b_dev));
	partition = PARTITION((bp->b_dev));

	pp = &(partition_struct[unit][partition]);

	if ( !(bp->b_flags & B_READ) && (pp->p_flag & V_RONLY)) {
		bp->b_error = EROFS;
		goto bad;
	}
	if (bp->b_bcount & (DEV_BSIZE-1)) {
		/*
		 * Requests must be in whole sectors (for now).
		 */
		bp->b_error = EINVAL;
		goto bad;
	}

	/* if request is off the end or trying to write last block on out */
	sz = (bp->b_bcount + DEV_BSIZE-1) >> DEV_BSHIFT;
	if (bp->b_flags & B_MD1) {
		if (bp->b_blkno > partition_struct[unit][PART_DISK].p_start +
			  partition_struct[unit][PART_DISK].p_size - 1) {
			bp->b_error = ENXIO;
			goto bad;
		}
	} else {
		if ( ((pp->p_flag & V_VALID) == 0)
			|| (bp->b_blkno < 0) || (bp->b_blkno >  pp->p_size)) {
			/*
			 * Request is outside the partition.
			 */
			bp->b_error = ENXIO;
			goto bad;
		}
		if (bp->b_blkno+sz > pp->p_size) {
			/*
			 * Truncate requests that are too long.
			 */
			bp->b_bcount = (pp->p_size - bp->b_blkno)<<DEV_BSHIFT;

		} else if (bp->b_blkno == pp->p_size) {
			/*
			 * indicate EOF by setting b_resid to b_bcount on
			 * the last block.
			 */
			bp->b_resid = bp->b_bcount;
			goto done;
		}
	}

	bp->b_cylin = (((bp->b_flags & B_MD1) ? 0 : pp->p_start) + bp->b_blkno)
		/ (hdparams[unit].nsecpertrack * hdparams[unit].nheads);

	s = spl5();
	dp = &hdunit[unit];

	disksort(dp, bp);

	if (!hh.controller_busy)
		hdstart();

	splx(s);
	return;
bad:
	bp->b_flags |= B_ERROR;
done:
	biodone(bp);
	
	return;
}

/* hdstart() is called at spl5 */
hdstart()
{
	partition_t	*pp;
	int drivecount;
	int unit;
	register struct buf *bp, *dp;

	ASSERT(!hh.controller_busy);

	if (hh.reset_request) {
		hh.reset_request = 0;
		hh.controller_busy = 1;
		reset_controller();
		return;
	}
	
	for (drivecount = 0; drivecount < NDRIVES; drivecount++) {
		if ( hh.curdrive < (NDRIVES-1) )
			hh.curdrive++;
		else
			hh.curdrive = 0;

		dp = &hdunit[hh.curdrive];
		if ((bp = dp->b_actf) != NULL)
			break;
	}
	if (drivecount == NDRIVES) {
		return;
	}
	unit = hh.curdrive;

	hh.controller_busy = 1;
	hh.blocktotal = (bp->b_bcount + DEV_BSIZE-1) >> DEV_BSHIFT;

	/* see V_RDABS and V_WRABS in hdioctl() */
	if (bp->b_flags & B_MD1) {
		hh.physblock = bp->b_blkno;
	} else {
 		hh.physblock = bp->b_blkno +
			partition_struct[unit][PARTITION(bp->b_dev)].p_start;
	}

	hh.blockcount = 0;
	if (bp->b_flags & B_PHYS) {
		hh.rw_addr = (caddr_t)map_pva_kva(bp->b_proc, bp->b_un.b_addr,
			bp->b_bcount, hh.k_window);
	} else {
		hh.rw_addr = bp->b_un.b_addr;
	}
	hh.retry_count = 0;

TRACE(1,(LOG_DEBUG, "["));

	start_rw(unit); 
}

hd_dump_registers()
{
	printf("Controller registers:\n");
	printf("Status Register: 0x%x\n", inb(PORT_STATUS));
	waitcontroller();
	printf("Error Register: 0x%x\n", inb(PORT_ERROR));
	printf("Sector Count: 0x%x\n", inb(PORT_NSECTOR));
	printf("Sector Number: 0x%x\n", inb(PORT_SECTOR));
	printf("Cylinder High: 0x%x\n", inb(PORT_CYLINDERHIBYTE));
	printf("Cylinder Low: 0x%x\n", inb(PORT_CYLINDERLOWBYTE));
	printf("Drive/Head Register: 0x%x\n", inb(PORT_DRIVE_HEADREGISTER));
}

int hd_print_error = 1;

int
hdintr(vec,locore)
int vec;
int *locore;
{
	register struct buf *bp, *dp;
	int unit = hh.curdrive;

	if (!hh.controller_busy) {
		printf("hd: false interrupt\n");
		hd_dump_registers();
		return(0);
	}
	if (inb(PORT_STATUS) & STAT_BUSY) {
		printf("hdintr: interrupt w/controller not done.\n");
	}
	if (hh.setparam_request) {
		/* We requested a SETPARAM, and the controller responded */
		hh.setparam_request = 0;
		hh.controller_busy = 0;
		hdstart();
		return(1);
	}
		
	waitcontroller();
	hh.status = inb(PORT_STATUS);	

	dp = &hdunit[unit];
	bp = dp->b_actf;

	if (bp == NULL ) {
		/* there should be a read/write buffer queued at this point */
		printf("hdintr: no bp buffer to read or write\n");
		return(1);
	}

	if (hh.restore_request == 1) { /* Restore command has completed */
		hh.restore_request = 0;
		if (hh.status & STAT_ERROR) {
			hderror(bp);
		} else {
			start_rw(unit);
		}
		return(1);
	}

	if (hh.status & STAT_WRITEFAULT) {
		panic("hd: write fault\n");
	}
#if 0
	if (hh.format_request) {
		printf("hdintr: format request\n");
		wakeup(&hh.interleave_tab[0]);
		return(1);
	}
#endif
	if (hh.status & STAT_ECC) {
		printf("hd%d: ECC soft error fixed, partition %d, physical block %d \n",
			unit, PARTITION(bp->b_dev), hh.physblock);
	}

	if (bp->b_flags & B_READ) {
TRACE(4,(LOG_DEBUG, "hdintr: reading a sector into 0x%x\n", hh.rw_addr));
		while ((inb(PORT_STATUS) & STAT_DATAREQUEST) == 0) {
				NOP_DELAY
		}
		linw(PORT_DATA, hh.rw_addr, SECSIZE/2); 
	}

	if (hh.status & STAT_ERROR) {
		if ((hd_print_error > 1) ||
		    (!(bp->b_flags & B_MD1)) && hd_print_error) {
		    printf("hd%d: state error %x, error = %x\n",
			 unit, hh.status, inb(PORT_ERROR));
		    printf("hd%d: state error. block %d, count %d, total %d\n",
			 unit, hh.physblock, hh.blockcount, hh.blocktotal);
		    printf("hd%d: state error. cyl %d, head %d, sector %d\n",
			 unit, hh.cylinder, hh.head, hh.sector);

		}
		hderror(bp);
		return(1);
	}

	if ( ++hh.blockcount == hh.blocktotal ) {
		dp->b_actf = bp->av_forw;
		bp->b_resid = 0;
		hh.controller_busy = 0;

		biodone(bp);

TRACE(1,(LOG_DEBUG, "]"));

		hdstart();
	} else {
		hh.rw_addr += SECSIZE;
		hh.physblock++;
		if (hh.single_mode) {
			start_rw(unit);
		} else if (!(bp->b_flags & B_READ)) {
			/* Load sector into controller for next write */
			waitcontroller();
			while ((inb(PORT_STATUS) & STAT_DATAREQUEST) == 0 ) {
				NOP_DELAY
			}
			loutw(PORT_DATA, hh.rw_addr, SECSIZE/2);
		}
	}
	return(1);
}

hderror(bp)
struct buf *bp;
{

	if(++hh.retry_count > 3) {
		if(bp) {
			/************************************************
			* We have a problem with this block, set the	*
			* error flag, terminate the operation and move	*
			* on to the next request.			*
			* With every hard disk transaction error we set	*
			* the reset requested flag so that the contrlr	*
			* is reset before next operation is started.	*
			* A reset is a relatively long operation, the	*
			* upper level routines are better qualified for	*
			* such an operation than the interrupt service	*
			* routines.					*
			************************************************/

			hdunit[hh.curdrive].b_actf = bp->av_forw;
			bp->b_flags |= B_ERROR;
			bp->b_resid = 0;
			hh.controller_busy = 0;

			biodone(bp);

			hh.reset_request = 1;
			hdstart();
		} else {
			/* give up, its way beyond hope */
#if 0
			if (hh.format_request) {
				printf("hd: can't format track number %d\n", hh.format_track);
				wakeup (&hh.interleave_tab[0]);
			}
#endif
		}
	} else {
		/* lets do a recalibration */
		waitcontroller();
		hh.restore_request = 1;
		outb(PORT_PRECOMP, hdparams[hh.curdrive].precomp >>2);
		outb(PORT_NSECTOR, hdparams[hh.curdrive].nsecpertrack);
		outb(PORT_SECTOR, hh.sector);
		outb(PORT_CYLINDERLOWBYTE, hh.cylinder & 0xff );
		outb(PORT_CYLINDERHIBYTE,  (hh.cylinder >> 8) & 0xff );
		outb(PORT_DRIVE_HEADREGISTER, 0);
		outb(PORT_COMMAND, CMD_RESTORE);
	}
}

getvtoc(unit)
unsigned char	unit;
{
	struct buf *bp;
	unsigned char *c_p;
	unsigned int n, m;
	char *pt1;
	struct boot_record *boot_record_p;
	struct evtoc *evp;
	partition_t *pp;
	int error;

TRACE(4,(LOG_DEBUG, "getvtoc: entering\n"));

	/*
	 * make PART_DISK partition the whole disk in case of failure
  	 * then get pdinfo 
	 */
	pp = &partition_struct[unit][PART_DISK];
	pp->p_tag   = V_BACKUP;
	pp->p_flag  = V_OPEN|V_UNMNT|V_VALID;
	pp->p_start = 0; 
	pp->p_size  = hdparams[unit].ncylinders
		   * hdparams[unit].nheads * hdparams[unit].nsecpertrack;

	/* get active partition */
	bp = geteblk(SECSIZE);
	bp->b_flags = B_BUSY|B_READ|B_MD1;
	bp->b_blkno = 0;
	bp->b_dev = WHOLE_DISK(unit);		/* partition is bottom 4 bits */
	bp->b_bcount = SECSIZE;
	hdstrategy(bp);
	error = biowait(bp);
	bp->b_flags &= ~B_MD1;
	if (error) {
		printf("hd: can not read sector 0 on drive %d, the boot record partition (see AT Tech. Ref.)\n", unit);
		brelse(bp);
		return;
	}
	c_p = (unsigned char *)bp->b_un.b_addr + 511;
	if (*c_p != BOOTRECORDSIGNATURE) {
		printf("hd: sector 0 signature bad, can not read active partition on drive %d\n", unit);
		brelse(bp);
		return;
	}
	c_p = (unsigned char *)bp->b_un.b_addr + 446;
	boot_record_p = (struct boot_record *)(c_p);
	for (n=0; n<4; n++, boot_record_p++)
		if (boot_record_p->boot_ind == 0x80) break;

	if (boot_record_p->boot_ind != 0x80) {
		printf("hd: no active partition on drive %d\n", unit);
		brelse(bp);
		return;
	}
	hh.start_of_unix[unit] = boot_record_p->rel_sect;	
	
	/* set correct partition information */

	pp->p_start = boot_record_p->rel_sect; 
	pp->p_size  = hdparams[unit].ncylinders
			* hdparams[unit].nheads * hdparams[unit].nsecpertrack
			- pp->p_start;

	/* get evtoc out of active unix partition */
	event_clear(&bp->b_iocomplete);
	bp->b_flags = B_BUSY|B_READ;
	bp->b_blkno = PDLOCATION;
	bp->b_dev = WHOLE_DISK(unit);		/* partition is bottom 4 bits */
	bp->b_bcount = SECSIZE;
	hdstrategy(bp);
	error = biowait(bp);
	if (error) {
		printf("hd: can not read evtoc on drive %d\n", unit);
		brelse(bp);
		return;
	}
	evp = (struct evtoc *)bp->b_un.b_addr;
	if (evp->sanity != VTOC_SANE) {
		printf("hd: evtoc invalid on drive %d\n",unit);
		brelse(bp);
		return;
	}

	/* pd info from disk must be more accurate than that in cmos
	 * thus override hdparams and re- setcontroller()
	 */			
	hdparams[unit].ncylinders = evp->cyls;
	hdparams[unit].nheads = evp->tracks;
	hdparams[unit].nsecpertrack = evp->sectors;
#ifdef	OOPS
	printf("cyl = %d, heads = %d, sectors = %d\n",
		evp->cyls, evp->tracks, evp->sectors);
#else	OOPS
	pp->p_size = hdparams[unit].ncylinders
			* hdparams[unit].nheads * hdparams[unit].nsecpertrack
			- pp->p_start;
	hh.reset_request = 1;
#endif	OOPS

	/* copy info on all valid partition, zero the others */
	for (n = 0; n < evp->nparts; n++) {
		if (n == PART_DISK)
			continue;
		/* this is a STRUCTURE copy */
		partition_struct[unit][n] = evp->part[n];
	}
	for ( ; n < V_NUMPAR; n++) {
		if (n == PART_DISK)
			continue;
		partition_struct[unit][n].p_flag = 0;
		partition_struct[unit][n].p_size = 0;
	}
	brelse(bp);

	/* get alternate sectors out of active unix partition */
	bp = &hdbuf[unit];
	BUF_LOCK(bp);

	event_clear(&bp->b_iocomplete);
	bp->b_flags = B_BUSY|B_READ;
	bp->b_blkno = evp->alt_ptr/SECSIZE;
	bp->b_dev = WHOLE_DISK(unit);	/* partition is bottom 4 bits */
	bp->b_bcount = 4 * SECSIZE;
	bp->b_un.b_addr = (char *)&alt_info[unit];
	hdstrategy(bp);
	error =	biowait(bp);
	if (error) {
		printf("hd: can not read alternate sectors on drive %d\n",
				unit);
	} else if (alt_info[unit].alt_sanity != ALT_SANITY) {
		printf("hd: alternate sectors corrupted on drive %d\n", unit);
	}
	BUF_UNLOCK(bp);
}

#if 0
format_command()
{
	unsigned int track;
	int unit = hh.curdrive;
		
	if ( hdparams[unit].nheads > 8)
		outb(FIXED_DISK_REG, MORETHAN8HEADS);
	else
		outb(FIXED_DISK_REG, EIGHTHEADSORLESS);
	
	if (hh.block_is_bad)
		track = hh.substitutetrack;
	else
		track = hh.format_track;
	hh.head = track % hdparams[unit].nheads; 
	hh.head = hh.head | (unit << 4) | FIXEDBITS;
	hh.cylinder = track / hdparams[unit].nheads;

	waitcontroller();
	outb(PORT_PRECOMP, hdparams[unit].precomp >>2);
	outb(PORT_NSECTOR, hdparams[unit].nsecpertrack);
	/* Western Digital 1010 and 1005 want the following line */
	outb(PORT_SECTOR, 36);
	outb(PORT_CYLINDERLOWBYTE, hh.cylinder & 0xff );
	outb(PORT_CYLINDERHIBYTE,  (hh.cylinder >> 8) & 0xff );
	outb(PORT_DRIVE_HEADREGISTER, hh.head);
	outb(PORT_COMMAND, CMD_FORMAT);
	waitcontroller();
	loutw(PORT_DATA, hh.interleave_tab, SECSIZE/2);
}
#endif

reset_controller()
{
	int	i;

	outb(0x3F6, 4);			/* Start controller reset */
	for(i = 0; i < 10000; i++) NOP_DELAY;
	outb(0x3F6, 0);			/* Exit controller reset mode */
	waitcontroller();		/* Wait for controller not busy */
	if(1 != (i = inb(PORT_ERROR)))	/* Verify that controller not failed */
		printf("reset_controller(): error code %d\n", i);
	setcontroller(0);		/* Set parameters for drive 0 */
	if(ndrives > 1)
		setcontroller(1);	/* Set parameters for drive 1 */
}

setcontroller(unit)
{
	unsigned char nheads = hdparams[unit].nheads;

	waitcontroller();		/* Wait for controller not busy */
	if (nheads > 16) {
		/* This must be temporary, so fake it. */
		nheads = 16;
	}
        nheads = FIXEDBITS | (unit << 4) | (nheads - 1);
	outb(PORT_DRIVE_HEADREGISTER, nheads); 
	outb(PORT_NSECTOR, hdparams[unit].nsecpertrack);
	outb(PORT_COMMAND, CMD_SETPARAMETERS);
	hh.setparam_request = 1;
}

waitcontroller()
{
	unsigned int n;

	for (n = 0; n < PATIENCE; n++) {
		if ((inb(PORT_STATUS) & STAT_BUSY) == 0)
			return;
		NOP_DELAY
	}
	panic("hard disk drive: waitcontroller() times out");
}

start_rw(unit)
int unit;
{
	unsigned int track, disk_block, xblk;
	struct buf *bp = hdunit[unit].b_actf;

	if (hdparams[unit].nheads > 8)
		outb(FIXED_DISK_REG, MORETHAN8HEADS);
	else
		outb(FIXED_DISK_REG, EIGHTHEADSORLESS);

	disk_block = hh.physblock;

	xblk = hh.blocktotal - hh.blockcount;	/* # of blks to transfer */

	if ((bp->b_flags & B_MD1) == 0) {
		xfermode();	/* determine the transfer mode */
		if(hh.single_mode) {
			xblk = 1;
			badblock_mapping();
			if(hh.block_is_bad)
				disk_block = hh.substituteblock;
		}
	}

	/* disk is formatted starting sector 1, not sector 0 */
	hh.sector = (disk_block % hdparams[unit].nsecpertrack) + 1;

	track = disk_block / hdparams[unit].nsecpertrack;

	hh.head = track % hdparams[unit].nheads; 
	hh.head = hh.head | (unit << 4) | FIXEDBITS;
	hh.cylinder = track / hdparams[unit].nheads;

	waitcontroller();
	outb(PORT_PRECOMP, hdparams[unit].precomp >>2);
	outb(PORT_NSECTOR, xblk);
	outb(PORT_SECTOR, hh.sector);
	outb(PORT_CYLINDERLOWBYTE, hh.cylinder & 0xff );
	outb(PORT_CYLINDERHIBYTE,  (hh.cylinder >> 8) & 0xff );
	outb(PORT_DRIVE_HEADREGISTER, hh.head );
	if (bp->b_flags & B_READ) {
		outb(PORT_COMMAND, CMD_READ);
 	} else {
		if (bp->b_flags & B_FORMAT) {
			Debugger("format command detected");
			outb(PORT_COMMAND, CMD_FORMAT);
		} else {
			outb(PORT_COMMAND, CMD_WRITE); 
		}
		waitcontroller();
		while ((inb(PORT_STATUS) & STAT_DATAREQUEST) == 0) {
			NOP_DELAY
		}
		loutw(PORT_DATA, hh.rw_addr, SECSIZE/2);
	}

}

badblock_mapping()
{
	unsigned short n;
	unsigned int track;
	int unit = hh.curdrive;
	struct buf *bp = hdunit[unit].b_actf;

	hh.block_is_bad = 0;

	/* PART_DISK partition is whole disk, bad blocks and all */
	if (PARTITION(bp->b_dev) == PART_DISK)
		return;

	/* to support V_RDABS and V_WRABS in hdioctl() */
	if (bp->b_flags & B_MD1)
		return;

	/* BAD TRACK MAPPING */
	track = hh.physblock / hdparams[unit].nsecpertrack;	
	
	for (n = 0; n < alt_info[unit].alt_trk.alt_used; n++) {
		if (track == alt_info[unit].alt_trk.alt_bad[n]) {
			hh.block_is_bad = 1;
			hh.substituteblock =  
				alt_info[unit].alt_trk.alt_base + 
				hdparams[unit].nsecpertrack * n +
				(hh.physblock % hdparams[unit].nsecpertrack);	
			hh.substitutetrack = track;
			return;
		}	
	} 


	/* BAD BLOCK MAPPING */
	/* add do while if substituteblock is bad !!!!!!!!!!  */

	for (n = 0; n < alt_info[unit].alt_sec.alt_used; n++) {
		if (hh.physblock == alt_info[unit].alt_sec.alt_bad[n]) {
			hh.block_is_bad = 1;
			hh.substituteblock = 
				alt_info[unit].alt_sec.alt_base + n;
			break;
		}	
	} 
}

dynamic_badblock()
{
	printf("dynamic_badblock()	--not implemented yet\n");
}

hdsize()
{
	printf("hdsize()	-- not implemented\n");
}


/* Used with the dump routine, hd_check_status returns 0 if
 * no error, EIO otherwise.  It returns disk controller status
 * in the status variable.
 */
static int
hd_check_status(status)
	unsigned char	*status;
{
	waitcontroller();

	*status = inb(PORT_STATUS);	

	if (*status & STAT_WRITEFAULT) {
	    printf("hddump: state error %x, error = %x\n",
		    *status, inb(PORT_ERROR));
	    printf("hddump: aborting dump\n");
	    return EIO;
	}

	if (*status & STAT_ERROR) {
	    printf("hddump: state error %x, error = %x\n",
		    *status, inb(PORT_ERROR));
	    printf("hddump: aborting dump\n");
	    return EIO;
	}

	/* Return success.  */
	return 0;
}


/* This is the disk driver's dump routine.  This routine is called when
 * the system panics.  It writes the given address range in kernel memory
 * out to the dump device at the offset dumplo.  It must return zero if
 * successful or the appropriate error number otherwise.  This routine is
 * called at high ipl which block disk interrupts so it does polling writes.
 */
hddump(dumpdev, dumplo, dump_addr, numblks, flag)
	dev_t		dumpdev;		/* disk partition to dump to */
	long		dumplo;			/* disk offset to dump to */
	int		dump_addr;		/* first dump address */
	long		numblks;		/* number of blocks to dump */
	int		flag;			/* unused (set to write) */
{
	unsigned char	unit;			/* dumpdev unit number */
	unsigned char	part;			/* dumpdev partition number */
	partition_t	*partition_p;		/* ptr to dumpdev partition */
	unsigned int	disk_block;		/* current block number */
	int		xfr_size;		/* number of bytes per write */
	int		end_addr;		/* last dump address */
	int		error;			/* io error? */


	/* This routine is called at high ipl which blocks disk interrupts
	 * so reset the controller to start from a clean slate.
	 */
	reset_controller();


	/* Make sure dumpdev is real and has a partition_struct.
	 */
	unit = UNIT(dumpdev);
	part = PARTITION(dumpdev);
	partition_p = &partition_struct[unit][part];

        if (unit >= ndrives || part >= V_NUMPAR || 
	    !hdinfo[unit] || !hdinfo[unit]->dev_alive) {
		printf("hddump: invalid dumpdev = 0x%x\n", dumpdev);
		printf("hddump: not performing dump\n");
                return ENXIO;
	}
        if (partition_struct[unit][PART_DISK].p_size == 0) {
                return ENXIO;
        }


	/* Insure that memory will fit into the dump parition.
	 */
	if ((dumplo + numblks) > partition_p->p_size) {
		printf("hddump: dump size exceeds partition size\n");
		printf("        dumplo = %d, numblks = %d, p_size = %d\n", 
				dumplo, numblks, partition_p->p_size);
		printf("hddump: not performing dump\n");
                return EINVAL;
	}
	disk_block = partition_p->p_start + dumplo;


	/* Start the dump, this code is derived from start_rw and hdintr.
	 * It writes one block at a time to the dump partition until
	 * all of memory has been written.  
	 *
	 * NOTE THAT THIS CODE CURRENTLY DOES NOT DO BAD BLOCK MAPPING.
	 */
	if ( hdparams[unit].nheads > 8) {
		outb(FIXED_DISK_REG, MORETHAN8HEADS);
	} else {
		outb(FIXED_DISK_REG, EIGHTHEADSORLESS);
	}

	xfr_size = hd_dump_size;
	end_addr = dump_addr + numblks*SECSIZE;

	while ( numblks > 0 ) {
		unsigned int	sector;
		unsigned int	nsectors;
		unsigned int	track;
		unsigned int	head;
		unsigned int	cylinder;
		unsigned char	status;

		/* disk is formatted starting sector 1, not sector 0 */
		sector = (disk_block % hdparams[unit].nsecpertrack) + 1;

		track = disk_block / hdparams[unit].nsecpertrack;

		head = track % hdparams[unit].nheads; 
		head = head | (unit << 4) | FIXEDBITS;
		cylinder = track / hdparams[unit].nheads;

		waitcontroller();
		nsectors = xfr_size/SECSIZE;
		outb(PORT_PRECOMP, hdparams[unit].precomp >>2);
		outb(PORT_NSECTOR, nsectors);
		outb(PORT_SECTOR, sector);
		outb(PORT_CYLINDERLOWBYTE, cylinder & 0xff );
		outb(PORT_CYLINDERHIBYTE,  (cylinder >> 8) & 0xff );
		outb(PORT_DRIVE_HEADREGISTER, head );
		outb(PORT_COMMAND, CMD_WRITE); 

		while (nsectors--) {

			/* Give watcher a warm feeling.  */
			if (dump_addr % ONE_MEGABYTE == 0) {
				printf(".");
			}

			/* wait for controller and check for problems */
			error = hd_check_status(&status);
			if (error) {
				return error;
			}

			while ((status & STAT_DATAREQUEST) == 0) {
				status = inb(PORT_STATUS);	
			}
			loutw(PORT_DATA, dump_addr, SECSIZE/2);

			/* Increment per loop variables */
			dump_addr += SECSIZE;
			disk_block++;
			numblks--;
		}

		/* last write is finished,
		 * wait for controller and check for problems
		 */
		error = hd_check_status(&status);
		if (error) {
			return error;
		}

		/* Set the xfr_size for the next write.
		 */
		if (dump_addr + xfr_size > end_addr) {
			xfr_size = end_addr - dump_addr;
		}
	}

	/* Return success.  */
	return 0;
}


/*
 * get the disk type from CMOS ram
 */
unsigned char
disktype()
{
	outb(CMOS_ADDR, HDTBL);
	return inb(CMOS_DATA);
}

/*
 *  determine single block or multiple blocks transfer mode
 */
xfermode()
{
	int n, bblk, eblk, btrk, etrk;
	struct buf *bp = hdunit[hh.curdrive].b_actf;

	hh.single_mode = 0;	/* default is multiple mode */

	bblk = hh.physblock;
	eblk = bblk + hh.blocktotal - 1;

	btrk = bblk / hdparams[hh.curdrive].nsecpertrack;	
	etrk = eblk / hdparams[hh.curdrive].nsecpertrack;	
	
	for (n = 0; n < alt_info[hh.curdrive].alt_trk.alt_used; n++) {
		if ((btrk <= alt_info[hh.curdrive].alt_trk.alt_bad[n]) &&
		     (etrk >= alt_info[hh.curdrive].alt_trk.alt_bad[n])) {
			hh.single_mode = 1;
			return;
		}	
	} 


	for (n = 0; n < alt_info[hh.curdrive].alt_sec.alt_used; n++) {
		if ((bblk <= alt_info[hh.curdrive].alt_sec.alt_bad[n]) &&
		    (eblk >= alt_info[hh.curdrive].alt_sec.alt_bad[n])) {
			hh.single_mode = 1;
			break;
		}	
	} 
}
