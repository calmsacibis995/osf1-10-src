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
static char	*sccsid = "@(#)$RCSfile: disk.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:15:04 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * 			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license  agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988 Intel Corporation
 * Copyright 1988, 1989 by Intel Corporation
 */
#include "small.h"

#include "i386at/disk.h"
#include "saio.h"

#define	BLKDEBUG 0

#define	BIOSDEV(dev)	((dev) == 0 ? BIOS_DEV_WIN : BIOS_DEV_FLOPPY)

/* diskinfo unpacking */
#define	SPT(di)		((di)&0xff)
#define	HEADS(di)	((((di)>>8)&0xff)+1)
#define	SPC(di)		(SPT(di)*HEADS(di))
#define BPS		512	/* sector size of the device */

#ifdef	SMALL
#define	badsect(dev,secno)	(secno)
#else
extern long badsect();
#endif	SMALL

struct devsw devsw[] = {
	{ "hd", },
	{ "fp", },
};

/* XXX only set up to handle two devices... */
struct diskinfo {
	struct alt_info * alt_info;	/* bad track/sector tables */
	int	spt;			/* sectors per track */
	int	spc;			/* sectors per cylinder */
	int	bps;			/* bytes per sector */
} diskinfo[] = {
	{ /* hd (dev 0) */},
	{ /* floppy (dev 1) */},
};

char * intbuf = 0;

int debug = 0;

devopen(io)
	struct iob	* io;
{
	register int	dev = io->i_ino.i_dev;
	register long	di;
	int		i;

	di = get_diskinfo(BIOSDEV(dev));

	/* initialize disk parameters -- spt and spc */
	diskinfo[dev].bps = BPS;

	io->i_error = 0;

	switch (dev) {
#ifndef	SMALL
	    case 0:	/* winchester */
		diskinfo[dev].spt = SPT(di);
		diskinfo[dev].spc = diskinfo[dev].spt * HEADS(di);
		if (init_win(dev, io->i_boff, &io->i_boff) < 0)
			io->i_error = EIO;
		break;
#endif	SMALL
	    case 1:	/* floppy */
	    case 2:
		io->i_boff = 0;
		for(i=SPT(di); i; i--) {
			if( biosread(BIOSDEV(dev),0,0,i-1) == 0 ) break;
		}
		diskinfo[dev].spt = i;
		diskinfo[dev].spc = diskinfo[dev].spt * HEADS(di);
		/* nothing to do... */
		break;
	}

#ifndef	SMALL
	printf(">> %d spt %d spc.\n", 
		diskinfo[dev].spt, diskinfo[dev].spc);
#endif	SMALL

#if !defined(SMALL) || BLKDEBUG
	if (debug)
		dumpit(dev);
#endif	BLKDEBUG
}


#if !defined(SMALL) || BLKDEBUG

atol(str)
	char * str;
{
	int	l=0;
	while(*str && (*str != '\n'))
		l = (l * 10) + (*str++ - '0');

	return l;
}

dumpsec(buf)
int	* buf;
{
	int i, j;

	for(i=0; i<16; i++) {
		for (j=0; j<8; j++)
			printf("%x ", *buf++);
		printf("\n");
	}
}

dumpit(dev)
{
	char b[20];
	int	i, trkno, cyl, head, sec;
	int	bn;
	int	rc;

	while (1) {
		printf("bn? ");
		gets(b);
		if (b[0] == '\0') break;

		bn = atol(b);

		rc = Biosread(dev, bn);
		if (!rc)
			dumpsec(intbuf);
	}
}
#endif	BLKDEBUG


int devread(io)
	register struct iob *io;
{
	int cc;
	long secno, sector;
	register int offset;
	register int bps;
	int i,j;
	int dev;

	io->i_flgs |= F_RDDATA;

	/* assume the best */
	io->i_error = 0;

	dev = io->i_ino.i_dev;

	bps = diskinfo[dev].bps;
	sector = io->i_bn;

	for (offset = 0; offset < io->i_cc; offset += bps) {

		secno = badsect(dev, sector);

		io->i_error = Biosread(dev, secno);
		if (io->i_error) {
			return(-1);
		}

		/* copy one sector from the internal buffer "intbuf" into buf */
		bcopy(intbuf, &io->i_ma[offset], bps);

		sector++;
	}
	io->i_flgs &= ~F_TYPEMASK;
	return (io->i_cc);
}


Biosread(dev,secno)
	int	dev;
	int	secno;
{
	int	rc;
	int	cyl, head, sec;
	register int	spt, spc;

	spt = diskinfo[dev].spt;
	spc = diskinfo[dev].spc;

	cyl = secno / spc;
	head = (secno % spc) / spt;
	sec = secno % spt;

#if	DEBUG
	if (debug)
		printf("biosread: (%x,%x) -> (%x,%x,%x)\n",
			   dev, secno, cyl, head, sec);
#endif	DEBUG
	rc = biosread(BIOSDEV(dev),cyl,head,sec);
	if (rc) {
		printf("biosread: 0x%x\n", rc); sleep(1);
	}
	return rc;
}


#ifndef	SMALL
int devwrite(io)
	struct iob	* io;
{
	return 0;
}

int devioctl(io)
	struct iob	* io;
{
	return 0;
}

int devclose(io)
	struct iob	* io;
{
	return 0;
}
#endif	SMALL


#ifndef	SMALL
long badsect(dev, secno)
int	dev;
long	secno;
{
	register int i;
	int spt;
	int trkno;
	struct alt_info * alt_info;

	alt_info = diskinfo[dev].alt_info;
	if (alt_info == (struct alt_info *)0)
		return secno;

	spt = diskinfo[dev].spt;

	/* bad track mapping */
	trkno = secno / spt;
	for (i = 0; i < alt_info->alt_trk.alt_used; i++) {
		if (trkno == alt_info->alt_trk.alt_bad[i])
			return alt_info->alt_trk.alt_base +
				i * spt + secno % spt;
	}	

	/* bad block mapping */	
	for (i = 0; i < alt_info->alt_sec.alt_used; i++) {
		if (secno == alt_info->alt_sec.alt_bad[i])
			return alt_info->alt_sec.alt_base + i;
	}

	return secno;
}

init_win(dev, part, rel_off)
	int		dev;		/* device */
	int		part;		/* partition */
	int		* rel_off;
{
	struct mboot	*mp;
	struct ipart	*ip;
	struct evtoc	*vp;
	long		secno;
	long		offset;
	int		i, cyl, head, sec;
	register char	*pt;
	int		bps;
	int		rc;

	bps = diskinfo[dev].bps;

	/* read sector 0 into the internal buffer "intbuf" */
	if ( rc = Biosread(dev, 0) ) {
		return -1;
	}

	/* find the active partition */
	mp = (struct mboot *)intbuf;

	ip = (struct ipart *)mp->parts;
	for (i = 0; i < FD_NUMPART; i++, ip++)
		if (ip->bootid == ACTIVE)
			break;

	offset = ip->relsect;

	/* read pdinfo/vtoc */
	secno = offset + HDPDLOC;
	if ( rc = Biosread(dev, secno) ) {
		return -1;
	}

	/* find the offset of the root partition */
	vp = (struct evtoc *)intbuf;
	if (vp->sanity != VTOC_SANE) {
		printf("vtoc insane");
		return -1;
	}

#ifndef	FIND_PART
	*rel_off = vp->part[part].p_start;
	if (vp->part[part].p_tag != V_ROOT)
		printf("warning... partition %d not root\n", part);
#else	
	for (i = 0; i < vp->nparts; i++)
		if (vp->part[i].p_tag == V_ROOT)
			break;

	if (i == vp->nparts) {
		printf("boot: No root file system\n");
		return -1;
	}
	*rel_off = vp->part[i].p_start;
#endif	USE_PART

	/*
	 * read in alternate sector table
	 */
	if (!diskinfo[dev].alt_info) {
		diskinfo[dev].alt_info = 
			(struct alt_info *)calloc(sizeof(struct alt_info));
	}

	secno = offset + HDPDLOC + 1;	/* starting sec of bad block map */
	pt = (char *)diskinfo[dev].alt_info;
	for (i = 0; i++ < 4; pt += bps, secno++) {
		Biosread(dev, secno);
		bcopy(intbuf, pt, bps);
	}

	if (diskinfo[dev].alt_info->alt_sanity != ALT_SANITY) {
		printf("boot: Bad alt_sanity\n");
		return -1;
	}

	return 0;
}
#endif	SMALL
