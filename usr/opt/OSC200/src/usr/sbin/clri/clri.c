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
static char	*sccsid = "@(#)$RCSfile: clri.c,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1992/12/18 16:34:53 $";
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
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rich $alz of BBN Inc.
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
char copyright[] =
"@(#) Copyright (c) 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * clri(8)
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>

extern priv_t *privvec();
#endif

#include <sys/param.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <ufs/quota.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

char *fs;
/* NOTE- add ALPHA_EXT for superblock extension */
char sblock[SBSIZE];
int wroteone;

main(argc, argv)
	int argc;
	char **argv;
{
	register struct fs *sbp;
	register struct dinode *ip;
	register int fd;
#if SEC_FSCHANGE
	struct dinode *dip;
#endif
	struct dinode ibuf[MAXBSIZE / sizeof (struct dinode)];
	int generation, bsize;
	off_t offset;
	int inonum;

	if (argc < 3) {
		(void)fprintf(stderr, "usage: clri filesystem inode ...\n");
		exit(1);
	}
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, "clri: need sysadmin authorization\n");
		exit(1);
	}
	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, "clri: insufficient privileges\n");
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
#endif /* SEC_BASE */

	fs = *++argv;

	/* get the superblock. */
	if ((fd = open(fs, O_RDWR, 0)) < 0)
		error(0);
	if (lseek(fd, (off_t)(SBLOCK * DEV_BSIZE), SEEK_SET) < 0)
		error(fd);
	if (read(fd, sblock, sizeof(sblock)) != sizeof(sblock)) {
		(void)fprintf(stderr,
		    "clri: %s: can't read the superblock.\n", fs);
		exit(1);
	}

	sbp = (struct fs *)sblock;
#if SEC_FSCHANGE
	if (sbp->fs_magic != FS_MAGIC && sbp->fs_magic != FS_SEC_MAGIC)
#else
	if (sbp->fs_magic != FS_MAGIC)
#endif
	{
		(void)fprintf(stderr,
		    "clri: %s: superblock magic number 0x%x, not 0x%x.\n",
		    fs, sbp->fs_magic, FS_MAGIC);
		exit(1);
	}
	bsize = sbp->fs_bsize;
#if SEC_FSCHANGE
	disk_set_file_system(sbp, bsize);
#endif

	/* remaining arguments are inode numbers. */
	while (*++argv) {
		/* get the inode number. */
		if ((inonum = atoi(*argv)) <= 0) {
			(void)fprintf(stderr,
			    "clri: %s is not a valid inode number.\n", *argv);
			sbsync(fd);
			exit(1);
		}
		(void)printf("clearing %d\n", inonum);

		/* read in the appropriate block. */
		offset = itod(sbp, inonum);	/* inode to fs block */
		offset = fsbtodb(sbp, offset);	/* fs block to disk block */
		offset *= (off_t)DEV_BSIZE;	/* disk block to disk bytes */

		/* seek and read the block */
		if (lseek(fd, offset, SEEK_SET) < 0)
			error(fd);
		if (read(fd, (char *)ibuf, bsize) != bsize)
			error(fd);

		/* get the inode within the block. */
#if SEC_FSCHANGE
		disk_inode_in_block(sbp, (char *) ibuf, &dip, inonum);
		ip = dip;
#else
		ip = &ibuf[itoo(sbp, inonum)];
#endif

		/* clear the inode, and bump the generation count. */
		generation = ip->di_gen + 1;
#if SEC_FSCHANGE
		bzero((char *)ip, disk_dinode_size());
#else
		bzero((char *)ip, sizeof *ip);
#endif
		ip->di_gen = generation;

		/* backup and write the block */
		if (lseek(fd, (off_t)(-bsize), SEEK_CUR) < 0)
			error(fd);
		if (write(fd, (char *)ibuf, bsize) != bsize)
			error(fd);
		wroteone++;
		(void)fsync(fd);
	}
	sbsync(fd);
	(void)close(fd);
	exit(0);
}

error(fd)
int	fd;
{
	(void)fprintf(stderr, "clri: %s: %s\n", fs, strerror(errno));
	if (fd)
		sbsync(fd);
	exit(1);
}

sbsync(fd)
int     fd;
{
	struct fs *sbp = (struct fs *)sblock;

	if (wroteone) {
		if (lseek(fd, SBLOCK * DEV_BSIZE, SEEK_SET) < 0)
		   (void)fprintf(stderr, "clri: %s: %s\n", fs, strerror(errno));
		sbp->fs_clean = 0;
		(void)write(fd, (char *)sbp, sizeof(sblock));
	}
}
