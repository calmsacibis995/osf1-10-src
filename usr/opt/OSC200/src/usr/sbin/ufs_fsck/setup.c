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
static char	*sccsid = "@(#)$RCSfile: setup.c,v $ $Revision: 4.3.11.3 $ (DEC) $Date: 1993/11/23 22:43:52 $";
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
 * Copyright (c) 1980, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

#define DKTYPENAMES
#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/file.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <machine/endian.h>
#include <ctype.h>
#include "fsck.h"
#if SEC_BASE
#include <sys/security.h>
#if SEC_ARCH
#include <sys/secpolicy.h>
#endif
#endif

struct bufarea asblk;
#define altsblock (*asblk.b_un.b_fs)
#define POWEROF2(num)	(((num) & ((num) - 1)) == 0)

/*
 * The size of a cylinder group is calculated by CGSIZE. The maximum size
 * is limited by the fact that cylinder groups are at most one block.
 * Its size is derived from the size of the maps maintained in the 
 * cylinder group and the (struct cg) size.
 */
#define CGSIZE(fs) \
    /* base cg */	(sizeof(struct cg) + \
    /* blktot size */	(fs)->fs_cpg * sizeof(int) + \
    /* blks size */	(fs)->fs_cpg * (fs)->fs_nrpos * sizeof(short) + \
    /* inode map */	howmany((fs)->fs_ipg, NBBY) + \
    /* block map */	howmany((fs)->fs_cpg * (fs)->fs_spc / NSPF(fs), NBBY))

struct	disklabel *getdisklabel();

setup(dev)
	char *dev;
{
	int cg, size, asked, i, j;
	int bmapsize;
	struct disklabel *lp;
	struct stat statb;
	struct fs proto;

#ifdef DEBUG
	printf ("Entering Setup\n");
#endif
	havesb = 0;
	fswritefd = -1;
	if (stat(dev, &statb) < 0) {
		register int err = errno;
		printf("Can't stat %s: %s\n", dev, strerror(err));
		if (preen) {
			/*
			 * if stat(2) fails for any reason, assume
			 * device is not there;  we'll return ENODEV
			 * as a signal to skip this entry and keep
			 * going.
			 */
			return (ENODEV);
		}
		return (0);
	}
	if ((statb.st_mode & S_IFMT) != S_IFCHR) {
		pfatal("%s is not a character device", dev);
		if (reply("CONTINUE") == 0)
			return (0);
	}
	if ((fsreadfd = open(dev, O_RDONLY)) < 0) {
		register int err = errno;
		printf("Can't open %s: %s\n", dev, strerror(err));
		if (preen) {
			/*
			 * if open(2) fails for any reason, assume
			 * device is not there;  we'll return ENODEV
			 * as a signal to skip this entry and keep
			 * going.
			 */
			return (ENODEV);
		}
		return (0);
	}
	if (preen == 0)
		printf("** %s", dev);
	else {
		if (check_readonly(dev)) {
			/* we have readonly device, we'll return
			 * EROFS as a signal to keep going.	
			 */
			close(fsreadfd);
			return(EROFS);
		}
	}
	if (nflag || (fswritefd = open(dev, O_WRONLY)) < 0) {
		fswritefd = -1;
		if (preen)
			pfatal("NO WRITE ACCESS");
		printf(" (NO WRITE)");
	}
	if (preen == 0)
		printf("\n");
	fsmodified = 0;
	lfdir = 0;
	initbarea(&sblk);
	initbarea(&asblk);
#ifdef __alpha
	sblk.b_un.b_buf = malloc(SBSIZE+ALPHA_EXT);
	asblk.b_un.b_buf = malloc(SBSIZE+ALPHA_EXT);
#else
	sblk.b_un.b_buf = malloc(SBSIZE);
	asblk.b_un.b_buf = malloc(SBSIZE);
#endif
	if (sblk.b_un.b_buf == NULL || asblk.b_un.b_buf == NULL)
		errexit("cannot allocate space for superblock\n");
	if (lp = getdisklabel((char *)NULL, fsreadfd))
		dev_bsize = secsize = lp->d_secsize;
	else
		dev_bsize = secsize = DEV_BSIZE;
	/*
	 * Read in the superblock, looking for alternates if necessary
	 */
	if (readsb(1) == 0) {
		if (bflag || preen || calcsb(dev, fsreadfd, &proto) == 0)
			return(0);
		if (reply("LOOK FOR ALTERNATE SUPERBLOCKS") == 0)
			return (0);
		for (cg = 0; cg < proto.fs_ncg; cg++) {
			bflag = fsbtodb(&proto, cgsblock(&proto, cg));
			if (readsb(0) != 0)
				break;
		}
		if (cg >= proto.fs_ncg) {
			printf("%s %s\n%s %s\n%s %s\n",
				"SEARCH FOR ALTERNATE SUPER-BLOCK",
				"FAILED. YOU MUST USE THE",
				"-b OPTION TO FSCK TO SPECIFY THE",
				"LOCATION OF AN ALTERNATE",
				"SUPER-BLOCK TO SUPPLY NEEDED",
				"INFORMATION; SEE fsck(8).");
			return(0);
		}
		pwarn("USING ALTERNATE SUPERBLOCK AT %d\n", bflag);
	}
	maxfsblock = sblock.fs_size;
	maxino = sblock.fs_ncg * sblock.fs_ipg;
	/*
	 * Check and potentially fix certain fields in the super block.
	 */
	if (sblock.fs_optim != FS_OPTTIME && sblock.fs_optim != FS_OPTSPACE) {
		pfatal("UNDEFINED OPTIMIZATION IN SUPERBLOCK");
		if (reply("SET TO DEFAULT") == 1) {
			sblock.fs_optim = FS_OPTTIME;
			sbdirty();
		}
	}
	if ((sblock.fs_minfree < 0 || sblock.fs_minfree > 99)) {
		pfatal("IMPOSSIBLE MINFREE=%d IN SUPERBLOCK",
			sblock.fs_minfree);
		if (reply("SET TO DEFAULT") == 1) {
			sblock.fs_minfree = 10;
			sbdirty();
		}
	}
	if (sblock.fs_interleave < 1 ||
	    sblock.fs_interleave > sblock.fs_nsect) {
		pwarn("IMPOSSIBLE INTERLEAVE=%d IN SUPERBLOCK",
			sblock.fs_interleave);
		sblock.fs_interleave = 1;
		if (preen)
			printf(" (FIXED)\n");
		if (preen || reply("SET TO DEFAULT") == 1) {
			sbdirty();
			dirty(&asblk);
		}
	}
	if (sblock.fs_npsect < sblock.fs_nsect ||
	    sblock.fs_npsect > sblock.fs_nsect*2) {
		pwarn("IMPOSSIBLE NPSECT=%d IN SUPERBLOCK",
			sblock.fs_npsect);
		sblock.fs_npsect = sblock.fs_nsect;
		if (preen)
			printf(" (FIXED)\n");
		if (preen || reply("SET TO DEFAULT") == 1) {
			sbdirty();
			dirty(&asblk);
		}
	}
	if (cvtflag) {
		if (sblock.fs_postblformat == FS_42POSTBLFMT) {
			/*
			 * Requested to convert from old format to new format
			 */
			if (preen)
				pwarn("CONVERTING TO NEW FILE SYSTEM FORMAT\n");
			else if (!reply("CONVERT TO NEW FILE SYSTEM FORMAT"))
				return(0);
			sblock.fs_postblformat = FS_DYNAMICPOSTBLFMT;
			sblock.fs_nrpos = 8;
			sblock.fs_postbloff =
			    (char *)(&sblock.fs_opostbl[0][0]) -
			    (char *)(&sblock.fs_link);
			sblock.fs_rotbloff = &sblock.fs_space[0] -
			    (u_char *)(&sblock.fs_link);
			sblock.fs_cgsize =
				fragroundup(&sblock, CGSIZE(&sblock));
			/*
			 * Planning now for future expansion.
			 */
#			if (BYTE_ORDER == BIG_ENDIAN)
				sblock.fs_qbmask.val[0] = 0;
				sblock.fs_qbmask.val[1] = ~sblock.fs_bmask;
				sblock.fs_qfmask.val[0] = 0;
				sblock.fs_qfmask.val[1] = ~sblock.fs_fmask;
#			endif /* BIG_ENDIAN */
#			if (BYTE_ORDER == LITTLE_ENDIAN)
				sblock.fs_qbmask.val[0] = ~sblock.fs_bmask;
				sblock.fs_qbmask.val[1] = 0;
				sblock.fs_qfmask.val[0] = ~sblock.fs_fmask;
				sblock.fs_qfmask.val[1] = 0;
#			endif /* LITTLE_ENDIAN */
			sbdirty();
			dirty(&asblk);
		} else if (sblock.fs_postblformat == FS_DYNAMICPOSTBLFMT) {
			/*
			 * Requested to convert from new format to old format
			 */
			if (sblock.fs_nrpos != 8 || sblock.fs_ipg > 2048 ||
			    sblock.fs_cpg > 32 || sblock.fs_cpc > 16) {
				printf(
				"PARAMETERS OF CURRENT FILE SYSTEM DO NOT\n\t");
				errexit(
				"ALLOW CONVERSION TO OLD FILE SYSTEM FORMAT\n");
			}
			if (preen)
				pwarn("CONVERTING TO OLD FILE SYSTEM FORMAT\n");
			else if (!reply("CONVERT TO OLD FILE SYSTEM FORMAT"))
				return(0);
			sblock.fs_postblformat = FS_42POSTBLFMT;
			sblock.fs_cgsize = fragroundup(&sblock,
			    sizeof(struct ocg) + howmany(sblock.fs_fpg, NBBY));
			sbdirty();
			dirty(&asblk);
		} else {
			errexit("UNKNOWN FILE SYSTEM FORMAT\n");
		}
	}
	if (asblk.b_dirty) {
		bcopy((char *)&sblock, (char *)&altsblock,
			(size_t)sblock.fs_sbsize);
		flush(fswritefd, &asblk);
	}
	/*
	 * read in the summary info.
	 */
	asked = 0;
	for (i = 0, j = 0; i < sblock.fs_cssize; i += sblock.fs_bsize, j++) {
		size = sblock.fs_cssize - i < sblock.fs_bsize ?
		    sblock.fs_cssize - i : sblock.fs_bsize;
		sblock.fs_csp[j] = (struct csum *)calloc(1, (unsigned)size);
		if (bread(fsreadfd, (char *)sblock.fs_csp[j],
		    fsbtodb(&sblock, sblock.fs_csaddr + j * sblock.fs_frag),
		    size) != 0 && !asked) {
			pfatal("BAD SUMMARY INFORMATION");
			if (reply("CONTINUE") == 0)
				errexit("");
			asked++;
		}
	}
	/* see if we can skip this fsck run. conditions are:
	 *      - not processing root file system
	 *      - super block has not been modified
	 *      - not using alternative superblock
	 */
	if (!hotroot && sblock.fs_clean == FS_CLEAN &&
	    sblk.b_dirty == 0 && !bflag && !nflag && !clnoverride) {
		pwarn("File system unmounted cleanly - no fsck needed\n");
		/* release the space we allocated */
		free(sblk.b_un.b_buf);
		return (FS_CLEAN);
	}
	/* running fsck on an active/mounted filesystem introduces
	 * a race-condition on accessing the physcial filesystem,
	 * i.e. it can panic the system.  we want to check for
	 * the following conditions:
	 *	a) fsck is not in "NO WRITE" mode.
	 *	b) "fsck -n" is not specified, i.e. no change mode.
	 */
	if (!nflag && (fswritefd >= 0) && check_for_mounted(dev, &statb)) {
		if (hotroot)
			return (1);		/* policy: always fsck root */
		if (preen)
			return (FS_CLEAN);	/* skip active fs in preen */
		pfatal("FSCK CANNOT BE RUN ON AN ACTIVE FILESYSTEM.\n");
		return (0);
	}
	/*
	 * allocate and initialize the necessary maps
	 */
	bmapsize = roundup(howmany(maxfsblock, NBBY), sizeof(short));
	blockmap = calloc((unsigned)bmapsize, sizeof (char));
	if (blockmap == NULL) {
		printf("cannot alloc %u bytes for blockmap\n", 
			(unsigned)bmapsize);
		goto badsb;
	}
	statemap = calloc((unsigned)(maxino + 1), sizeof(char));
	if (statemap == NULL) {
		printf("cannot alloc %u bytes for statemap\n", 
			(unsigned)(maxino + 1));
		goto badsb;
	}
	lncntp = (short *)calloc((unsigned)(maxino + 1), sizeof(short));
	if (lncntp == NULL) {
		printf("cannot alloc %u bytes for lncntp\n", 
		    (unsigned)(maxino + 1) * sizeof(short));
		goto badsb;
	}
	numdirs = sblock.fs_cstotal.cs_ndir;
	inplast = 0;
	listmax = numdirs + 10;
	inpsort = (struct inoinfo **)calloc((unsigned)listmax,
	    sizeof(struct inoinfo *));
	inphead = (struct inoinfo **)calloc((unsigned)numdirs,
	    sizeof(struct inoinfo *));
	if (inpsort == NULL || inphead == NULL) {
		printf("cannot alloc %u bytes for inphead\n", 
		    (unsigned)numdirs * sizeof(struct inoinfo *));
		goto badsb;
	}
	bufinit();
	return (1);

badsb:
	ckfini();
	return (0);
}

/*
 * Read in the super block and its summary info.
 */
readsb(listerr)
	int listerr;
{
	daddr_t super = bflag ? bflag : SBOFF / dev_bsize;

#ifdef DEBUG
	printf ("Reading superblock...\n");
#endif
	if (bread(fsreadfd, (char *)&sblock, super, (int)SBSIZE) != 0)
		return (0);
	sblk.b_bno = super;
	sblk.b_size = SBSIZE;
	/*
	 * run a few consistency checks of the super block
	 */
#if SEC_FSCHANGE
        if ((sblock.fs_magic != FS_MAGIC) && (sblock.fs_magic != FS_SEC_MAGIC))
#else
        if (sblock.fs_magic != FS_MAGIC)
#endif
		{ badsb(listerr, "MAGIC NUMBER WRONG"); return (0); }
	if (sblock.fs_ncg < 1)
		{ badsb(listerr, "NCG OUT OF RANGE"); return (0); }
	if (sblock.fs_cpg < 1)
		{ badsb(listerr, "CPG OUT OF RANGE"); return (0); }
	if (sblock.fs_ncg * sblock.fs_cpg < sblock.fs_ncyl ||
	    (sblock.fs_ncg - 1) * sblock.fs_cpg >= sblock.fs_ncyl)
		{ badsb(listerr, "NCYL LESS THAN NCG*CPG"); return (0); }
	if (sblock.fs_sbsize > SBSIZE)
		{ badsb(listerr, "SIZE PREPOSTEROUSLY LARGE"); return (0); }
	/*
	 * Compute block size that the filesystem is based on,
	 * according to fsbtodb, and adjust superblock block number
	 * so we can tell if this is an alternate later.
	 */
#if SEC_FSCHANGE
        if (preen == 0)
                if (sblock.fs_magic == FS_SEC_MAGIC)
                        printf("** Extended Format Filesystem\n");
                else
                        printf("** Non-extended Format Filesystem\n");

        disk_set_file_system(&sblock, sblock.fs_bsize);
#endif
	super *= dev_bsize;
	dev_bsize = sblock.fs_fsize / fsbtodb(&sblock, 1);
	sblk.b_bno = super / dev_bsize;
	/*
	 * Set all possible fields that could differ, then do check
	 * of whole super block against an alternate super block.
	 * When an alternate super-block is specified this check is skipped.
	 */
	getblk(&asblk, cgsblock(&sblock, sblock.fs_ncg - 1), sblock.fs_sbsize);
	if (asblk.b_errs)
		return (0);
	if (bflag) {
		havesb = 1;
		return (1);
	}
	altsblock.fs_link = sblock.fs_link;
	altsblock.fs_rlink = sblock.fs_rlink;
	altsblock.fs_time = sblock.fs_time;
	altsblock.fs_cstotal = sblock.fs_cstotal;
	altsblock.fs_cgrotor = sblock.fs_cgrotor;
	altsblock.fs_fmod = sblock.fs_fmod;
	altsblock.fs_clean = sblock.fs_clean;
	altsblock.fs_ronly = sblock.fs_ronly;
	altsblock.fs_flags = sblock.fs_flags;
	altsblock.fs_maxcontig = sblock.fs_maxcontig;
	altsblock.fs_minfree = sblock.fs_minfree;
	altsblock.fs_optim = sblock.fs_optim;
	altsblock.fs_rotdelay = sblock.fs_rotdelay;
	altsblock.fs_maxbpg = sblock.fs_maxbpg;
#ifndef __alpha
	/* In alpha, the csp's are in memory only */
	bcopy((char *)sblock.fs_csp, (char *)altsblock.fs_csp,
		sizeof sblock.fs_csp);
#endif
	bcopy((char *)sblock.fs_fsmnt, (char *)altsblock.fs_fsmnt,
		sizeof sblock.fs_fsmnt);
	bcopy((char *)sblock.fs_sparecon, (char *)altsblock.fs_sparecon,
		sizeof sblock.fs_sparecon);
	/*
	 * The following should not have to be copied.
	 */
	altsblock.fs_fsbtodb = sblock.fs_fsbtodb;
	altsblock.fs_interleave = sblock.fs_interleave;
	altsblock.fs_npsect = sblock.fs_npsect;
	altsblock.fs_nrpos = sblock.fs_nrpos;
	if (bcmp((char *)&sblock, (char *)&altsblock, (int)sblock.fs_sbsize)) {
#ifdef __alpha
#ifdef DEBUG
		printf("New SB = %d bytes\n", sizeof(struct fs));
		printf("csp 1 = %X%X  csp 2 = %X%X\n", sblock.fs_blank[0],
			sblock.fs_blank[1], altsblock.fs_blank[0],
			altsblock.fs_blank[1]);
		printf("Size fs_blank = %d\n", sizeof(sblock.fs_blank));
#endif
		if (bcmp((char *)&sblock.fs_blank[0], 
		    (char *)&altsblock.fs_blank[0], MAXCSBUFS*sizeof(int))) {
			bzero((char *)sblock.fs_blank,
					sizeof(sblock.fs_blank));
		}
		else {
#endif
			badsb(listerr,
		"VALUES IN SUPER BLOCK DISAGREE WITH THOSE IN FIRST ALTERNATE");
			return (0);
#ifdef __alpha
		}
#endif
	}
	havesb = 1;
	return (1);
}

badsb(listerr, s)
	int listerr;
	char *s;
{

	if (!listerr)
		return;
	if (preen)
		printf("%s: ", devname);
	pfatal("BAD SUPER BLOCK: %s\n", s);
}

/*
 * Calculate a prototype superblock based on information in the disk label.
 * When done the cgsblock macro can be calculated and the fs_ncg field
 * can be used. Do NOT attempt to use other macros without verifying that
 * their needed information is available!
 */
calcsb(dev, devfd, fs)
	char *dev;
	int devfd;
	register struct fs *fs;
{
	register struct disklabel *lp;
	register struct partition *pp;
	register char *cp;
	int i;

	cp = index(dev, '\0') - 1;
	if (cp == (char *)-1 || (*cp < 'a' || *cp > 'h') && !isdigit(*cp)) {
		pfatal("%s: CANNOT FIGURE OUT FILE SYSTEM PARTITION\n", dev);
		return (0);
	}
	lp = getdisklabel(dev, devfd);
	if (isdigit(*cp))
		pp = &lp->d_partitions[0];
	else
		pp = &lp->d_partitions[*cp - 'a'];
	if (pp->p_fstype != FS_BSDFFS) {
		pfatal("%s: NOT LABELED AS A BSD FILE SYSTEM (%s)\n",
			dev, pp->p_fstype < FSMAXTYPES ?
			fstypenames[pp->p_fstype] : "unknown");
		return (0);
	}
	bzero((char *)fs, sizeof(struct fs));
	fs->fs_fsize = pp->p_fsize;
	fs->fs_frag = pp->p_frag;
	fs->fs_cpg = pp->p_cpg;
	fs->fs_size = pp->p_size;
	fs->fs_ntrak = lp->d_ntracks;
	fs->fs_nsect = lp->d_nsectors;
	fs->fs_spc = lp->d_secpercyl;
	fs->fs_nspf = fs->fs_fsize / lp->d_secsize;
	fs->fs_sblkno = roundup(
		howmany(lp->d_bbsize + lp->d_sbsize, fs->fs_fsize),
		fs->fs_frag);
	fs->fs_cgmask = 0xffffffff;
	for (i = fs->fs_ntrak; i > 1; i >>= 1)
		fs->fs_cgmask <<= 1;
	if (!POWEROF2(fs->fs_ntrak))
		fs->fs_cgmask <<= 1;
	fs->fs_cgoffset = roundup(
		howmany(fs->fs_nsect, NSPF(fs)), fs->fs_frag);
	fs->fs_fpg = (fs->fs_cpg * fs->fs_spc) / NSPF(fs);
	fs->fs_ncg = howmany(fs->fs_size / fs->fs_spc, fs->fs_cpg);
	for (fs->fs_fsbtodb = 0, i = NSPF(fs); i > 1; i >>= 1)
		fs->fs_fsbtodb++;
	dev_bsize = lp->d_secsize;
	return (1);
}

struct disklabel *
getdisklabel(s, fd)
	char *s;
	int	fd;
{
	static struct disklabel lab;

	if (ioctl(fd, DIOCGDINFO, (char *)&lab) < 0) {
		if (s == NULL)
			return ((struct disklabel *)NULL);
		pwarn("ioctl (GCINFO): %s\n", strerror(errno));
		errexit("%s: can't read disk label\n", s);
	}
	return (&lab);
}

#include <sys/mount.h>
#include <sys/table.h>
#include <fcntl.h>
#include <dirent.h>

/*
 * routines used to see if partitions overlap, i.e. we don't want to
 * simultaneously mount overlapping partitions.  this code is 'borrowed'
 * from newfs/newfs.c:
 *	ultrix_style()
 *	init_partid()
 *	check_for_overlap()
 *	verify_ok()
 * Detect overlapping mounted partitions.
 *
 * This code (from verify_ok() on down) is substantially similar to
 * that of usr/sbin/newfs/newfs.c, usr/sbin/mount/mount.c,
 * and usr/sbin/swapon/swapon.c
 * Change all places!
 * (XXX maybe we should pull this out into a shared source file?)
 *
 *
 * XXX - Only 8 partitions are used in overlap detection for ultrix style,
 *       otherwise MAXPARTITIONS are used.
 */

struct ufs_partid {
	dev_t lvm_or_lsm_dev;			/* lvm device		*/
	short bus;			/* Bus				*/
	short adpt_num;			/* Adapter number		*/
	short nexus_num;		/* Nexus or node on adapter no.	*/
	short bus_num;			/* Bus number			*/
	short ctlr_num;			/* Controller number		*/
	short rctlr_num;		/* Remote controller number	*/
	short slave_num;		/* Plug or line number		*/
	short unit_num;			/* Ultrix device unit number	*/
	long  category_stat;		/* Category specific mask	*/
	struct pt {
		long part_blkoff;	/* Beginning partition sector	*/
		long part_nblocks;	/* Number of sectors in partition */
	} pt_part[MAXPARTITIONS];
} ufs_partid;

static	int    verify_ok(struct ufs_partid *,
			 const char *,
			 const char *,
			 const char *,
			 const char *);

#define DEV_NAME_LSM	"LSM"
#define DEV_NAME_LVM	"LVM"

devget_to_partid(fd, devget, partid)
	int fd;
	struct devget *devget;
	struct ufs_partid *partid;
{
#ifdef DEBUG
    printf("devget_to_partid\n");
#endif
    bzero(partid, sizeof(*partid));
    if (strncmp(devget->dev_name, DEV_NAME_LVM, strlen(DEV_NAME_LVM)) == 0 ||
	strncmp(devget->dev_name, DEV_NAME_LSM, strlen(DEV_NAME_LSM)) == 0)
    {
	struct stat info;

	if (fstat(fd, &info) < 0)
		return(-1);
	partid->lvm_or_lsm_dev = info.st_rdev;
#ifdef DEBUG
	printf("lvm/lsm partition %d, %d\n",
		major(partid->lvm_or_lsm_dev),
		minor(partid->lvm_or_lsm_dev));
#endif
    } else
    {
        partid->bus = devget->bus;
        partid->adpt_num = devget->adpt_num;
        partid->nexus_num = devget->nexus_num;
        partid->bus_num = devget->bus_num;
        partid->ctlr_num = devget->ctlr_num;
        partid->rctlr_num = devget->rctlr_num;
        partid->slave_num = devget->slave_num;
        partid->unit_num = devget->unit_num;
        partid->category_stat = devget->category_stat & (MAXPARTITIONS - 1);
    }
    return (0);
}

print_partid(device, partid)
	char *device;
	struct ufs_partid *partid;
{
	int ind;

	printf("device %s\n", device);
	if (partid->lvm_or_lsm_dev)
		printf("\tLVM/LSM %d, %d\n",
			major(partid->lvm_or_lsm_dev),
			minor(partid->lvm_or_lsm_dev));
	else
	{
		printf("\tbus %d\n", partid->bus);
		printf("\tadpt_num %d\n", partid->adpt_num);
		printf("\tnexus_num %d\n", partid->nexus_num);
		printf("\tbus_num %d\n", partid->bus_num);
		printf("\tctlr_num %d\n", partid->ctlr_num);
		printf("\trctlr_num %d\n", partid->rctlr_num);
		printf("\tslave_num %d\n", partid->slave_num);
		printf("\tunit_num %d\n", partid->unit_num);
		printf("\tcategory_stat %d\n", partid->category_stat);
		for (ind = 0; ind < MAXPARTITIONS; ind++)
			printf("\tpt_offset %d length %d\n",
		       		partid->pt_part[ind].part_blkoff,
		       		partid->pt_part[ind].part_nblocks);
	}
	printf("\n");
}

#define PT_MAGIC        0x032957        /* Partition magic number */
#define PT_VALID        1               /* Indicates if struct is valid */

/*
 * Structure that is used to determine the partitioning of the disk.
 * It's location is at the end of the superblock area.
 * The reason for both the cylinder offset and block offset
 * is that some of the disk drivers (most notably the uda
 * driver) require the block offset rather than the cyl.
 * offset.
 */
struct ult_pt {
	int	pt_magic;       /* magic no. indicating part. info exits */
	int     pt_valid;       /* set by driver if pt is current */
	struct  pt_info {
		int     pi_nblocks;     /* no. of sectors for the partition */
		daddr_t pi_blkoff;      /* block offset for start of part. */
	} pt_part[8];
};

ultrix_style(device, partid, dl)
	char *device;
	struct ufs_partid *partid;
	struct disklabel *dl;
{
	struct fs *fs;
	struct ult_pt *pt;
	char buf[32];
	char sb[SBSIZE];
	int len, ind, fd;

	len = strlen(device);
	strcpy(buf, device);
	buf[len - 1] = 'c';

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return(-1);
	lseek(fd, SBOFF, 0);
	if (read(fd, sb, SBSIZE) != SBSIZE) {	
		close(fd);
		return(-1);
	}
	close(fd);

	fs = (struct fs *)sb;
	if (fs->fs_magic != FS_MAGIC)
		return(-1);

	pt = (struct ult_pt *)&sb[8192 - sizeof(struct ult_pt)];
	if (pt->pt_magic != PT_MAGIC || pt->pt_valid != PT_VALID)
		return(-1);

	/*
	 * Valid ULTRIX Super block and partition table
	 */
	for (ind = 0; ind < 8; ind++) {
		dl->d_partitions[ind].p_offset = pt->pt_part[ind].pi_blkoff;
		dl->d_partitions[ind].p_size = pt->pt_part[ind].pi_nblocks;
	}
	return(0);
}


/*
 * Return -1 if partid could not be initialized.
 * Rerurn 0 if partid was initialized.
 */
init_partid(file, partid)
	char *file;
	struct ufs_partid *partid;
{
	int fd, ind;
	struct devget devget;
	struct disklabel disklabel, *dl;
	

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return(-1);
	/*
	 * Call drive to fill in devget struct
	 */
	if (ioctl(fd, DEVIOCGET, (char *)&devget) < 0) {
		close(fd);
		return(-1);
	}
	if (devget_to_partid(fd, &devget, partid) < 0)
	{
		close(fd);
		return(-1);
	}

	/*
	 * If logical volume then we are finished.
	 */
	if (partid->lvm_or_lsm_dev)
	{
		close(fd);
		return(0);
	}	

	/*
	 * Get partition table info for drive:
	 *
	 *	Check for a disklabel
	 * 	Check for ULTRIX style partition table
	 *	Use disktab
	 */
	dl = &disklabel;
	if (ioctl(fd, DIOCGDINFO, dl) < 0 && ultrix_style(file, partid, dl) < 0)
		dl = (struct disklabel *)getdiskbyname(devget.device);
	close(fd);

	/*
	 * If no partition table found, no testing possible.
	 */
	if (!dl)
		return(-1);

	for (ind = 0; ind < MAXPARTITIONS; ind++) {
		partid->pt_part[ind].part_blkoff = dl->d_partitions[ind].p_offset;
		partid->pt_part[ind].part_nblocks = dl->d_partitions[ind].p_size;
	}
	return(0);
}

extern char *rawname();

/*
 * Return 0 if no overlap or if we could not tell
 *	if overlapped or not.
 * Return 1 if overlap
 * Return -1 if overlap with exact match (ie same partition).
 *
 * WARNING:
 *	THIS CODE DOES NOT DETECT AN ATTEMPT TO USE
 *	A PARTITION WHICH IS USE BY THE LVM.
 *
 *	THIS CODE DOES NOT DETECT AN ATTEMPT TO USE
 *	A PARTITION WHICH WOULD OVERLAP A PARTITION
 *	WHICH IS IN USE BY THE LVM.
 *
 *	AT PRESENT NO EASY WAY EXISTS TO PERFORM
 *	THESE TESTS
 */  
static int
verify_ok(struct ufs_partid *partid,
	  const char *device,
	  const char *blockdev,
	  const char *usename,
	  const char *msg)
{
    struct devget mnt_devget;
    struct ufs_partid mnt_partid;
    int fd, ret = 0;
    long bot, top, mbot, mtop;
    char *raw = rawname(blockdev);

    fd = open(raw, O_RDONLY);
    if (fd < 0)
	return (0);
    /*
     * Call drive to fill in devget struct
     */
    if (ioctl(fd, DEVIOCGET, (char *)&mnt_devget) < 0) {
	close(fd);
	return (0);
    }

    /*
     * Extract what we need
     */
    if (devget_to_partid(fd, &mnt_devget, &mnt_partid) < 0)
    {
	close(fd);
	return(0);
    }
    close(fd);

    /*
     * Either one a logical volume ?
     */
    if (mnt_partid.lvm_or_lsm_dev || partid->lvm_or_lsm_dev)
    {
	/*
	 * Same logical volume ?
	 */
    	if (mnt_partid.lvm_or_lsm_dev == partid->lvm_or_lsm_dev)
	{
	    printf("New filesystem would overlap mounted filesystem(s) or active swap area\n");
	    printf("Unmount required before creating %s\n", device);
		return (-1);
	}
	/*
	 * Assume no overlap if different logical volume or
	 * only one logical volume.
	 */
	return(0);	
    }
    
    /*
     * Check for drive match
     */
    if (mnt_partid.bus != partid->bus ||
	mnt_partid.adpt_num != partid->adpt_num ||
	mnt_partid.nexus_num != partid->nexus_num ||
	mnt_partid.bus_num != partid->bus_num ||
	mnt_partid.ctlr_num != partid->ctlr_num ||
	mnt_partid.rctlr_num != partid->rctlr_num ||
	mnt_partid.slave_num != partid->slave_num ||
	mnt_partid.unit_num != partid->unit_num)
	return (0);

    bot = partid->pt_part[partid->category_stat].part_blkoff;
    top = bot + partid->pt_part[partid->category_stat].part_nblocks - 1;

    mbot = partid->pt_part[mnt_partid.category_stat].part_blkoff;
    mtop = mbot + partid->pt_part[mnt_partid.category_stat].part_nblocks - 1;
    /*
     * if the exact same partition,
     * report specially; callee will report it more concisely.
     */
    if (bot == mbot && top == mtop)
	return (-1);
    /*
     * If same partition or partitions overlap,
     * return true
     */
    if (top > 0 && mtop > 0 &&
	(mnt_partid.category_stat == partid->category_stat ||
	 bot >= mbot && bot <= mtop ||
	 top >= mbot && top <= mtop ||
	 mbot >= bot && mbot <= top)) {
	if (ret++ == 0) {
	    printf("%s overlaps mounted filesystem(s) or an active swap area\n", device);
	    printf("Unmount or reboot required before fsck on %s (start %d end %d)\n", device, bot, top);
	}
	printf("\t%s %s %s (start %d end %d)\n",
	       blockdev, msg, usename, mbot, mtop);
    }
    return (ret);
}

static char *
swap_partition_name(dev_t dev)
{
  static char device_name[256+4+1];     /* `/dev/file-name-up-to-255-characters'<NUL> */
  DIR *dir;
  struct dirent *dirent;

  /*
  ** Find the device in the /tmp directory.
  */
  if (!(dir = opendir("/dev")))
    return (NULL);

  /*
  ** Directory is accessed, read through each name in `/dev', and
  ** stat the file to obtain the (possible) major/minor numbers.
  ** When we have a block device which matches, return its name.
  */
  while (dirent = readdir(dir)) {
    /*
    ** Got some directory information.  Check for a block special
    ** with matching major and minor numbers.  If (when) found,
    ** return a pointer to the generated name.
    */
    struct stat local_stat;

    sprintf(device_name, "/dev/%s", dirent->d_name);
    if (!stat(device_name, &local_stat)) {
      if (S_ISBLK(local_stat.st_mode) &&
          major(local_stat.st_rdev) == major(dev) &&
          minor(local_stat.st_rdev) == minor(dev)) {
        closedir(dir);
        return (device_name);
        }
      }
    }
  closedir(dir);
  return (NULL);
  }

/*
 * check_for_mounted:
 *	returns TRUE if device is already/actively mounted.
 *	note: will skip checking device(s) under following conditions:
 *	      a) is not ufs.
 *	      b) fails stat(2).
 *	      c) if preen, skip over active fs.
 *	      d) if getmntinfo(3) fails.
 */
int
check_for_mounted(device, statbp)
	char *device;
	struct stat *statbp;	/* from stat(device, statbp) */
{
	struct statfs *mntbuf;
	int i, mntsize, ret;
	struct stat target_stat;
	struct ufs_partid *partid = &ufs_partid;
	int verify = 1;
	if (hotroot) {
		/* if this is the rootdev, it is always mounted. */
		return (0);
	}
	ret = 0;

	if (init_partid(device, partid) == 0) {

	    /* Get swap info & verify non-overlapping */
	    for (i = 0;; i++) {
		struct tbl_swapinfo swapinfo;
		char *name;

		/*
		 ** Attempt to read the swap information.
		 */
		if (table(TBL_SWAPINFO, i, &swapinfo, 1, sizeof(swapinfo)) < 0)
		    break;

		/*
		 ** check this swap partition.
		 */
	    
		if ((name = swap_partition_name(swapinfo.dev)) == NULL)
			continue;
	
		ret += verify_ok(partid, device, name, "swap", "in use as");
	    }
	} else
	    verify = 0;
	if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0)
		return(ret);

	for (i = 0; i < mntsize; i++) {
	        int exact;
		if (mntbuf[i].f_type != MOUNT_UFS)
			continue;
		if (verify) {
		    exact = verify_ok(partid, device, mntbuf[i].f_mntfromname,
				      mntbuf[i].f_mntonname, "mounted on");
		    if (exact == -1) {
			/* exact overlap...spit out more concise message. */

			if (mntbuf[i].f_flags & M_RDONLY) {
				/* if preen, we'll assume this is the startup
				 * case (for the rootfs), and the 'right' thing
				 * of a mount-update is going to be done w/o
				 * having to print warnings.
				 */
				if (!preen) {
					printf("WARNING: %s is already mounted read-only:\n",
						device);
					printf("    'mount -ur %s' should be done after fsck.\n",
						mntbuf[i].f_mntonname);
				}
				continue;
			}
			if (preen) {
				/* ignore active/mounted filesystems,
				 * (since this causes bcheckrc to fail, which
				 *  causes the init-3 transition to fail.
				 */
				printf("%s: skipping filesystem already ",
					devname);
				printf("mounted (read-write) on '%s'.\n",
					mntbuf[i].f_mntonname);
				return(1);
			}
			if (ret++ == 0)
				pwarn("This is already mounted as '%s on %s'.\n",
					mntbuf[i].f_mntfromname,
					mntbuf[i].f_mntonname);
		    } else
			ret += exact;
		}
	    }
	return(ret);		
}
