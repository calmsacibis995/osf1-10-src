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
static char	*sccsid = "@(#)$RCSfile: inode.c,v $ $Revision: 4.3.4.5 $ (DEC) $Date: 1992/08/07 15:06:58 $";
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

#include <sys/secdefines.h>
#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <ufs/dir.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include "fsck.h"
#if SEC_BASE
#include <sys/security.h>
#if SEC_ARCH
#include <sys/secpolicy.h>
#endif
#endif

static ino_t startinum;

ckinode(dp, idesc)
	struct dinode *dp;
	register struct inodesc *idesc;
{
	register daddr_t *ap;
	int ret, n, ndb, offset;
	struct dinode dino;

	if (idesc->id_fix != IGNORE)
		idesc->id_fix = DONTKNOW;
	idesc->id_entryno = 0;
	idesc->id_filesize = dp->di_size;
	if ((dp->di_mode & IFMT) == IFBLK || (dp->di_mode & IFMT) == IFCHR ||
		(dp->di_flags & IC_FASTLINK) != 0)
		return (KEEPON);
	dino = *dp;
	ndb = howmany(dino.di_size, sblock.fs_bsize);
	for (ap = &dino.di_db[0]; ap < &dino.di_db[NDADDR]; ap++) {
		if (--ndb == 0 && (offset = blkoff(&sblock, dino.di_size)) != 0)
			idesc->id_numfrags =
				numfrags(&sblock, fragroundup(&sblock, offset));
		else
			idesc->id_numfrags = sblock.fs_frag;
		if (*ap == 0)
			continue;
		idesc->id_blkno = *ap;
		if (idesc->id_type == ADDR)
			ret = (*idesc->id_func)(idesc);
		else
			ret = dirscan(idesc);
		if (ret & STOP)
			return (ret);
	}
	idesc->id_numfrags = sblock.fs_frag;
	for (ap = &dino.di_ib[0], n = 1; n <= NIADDR; ap++, n++) {
		if (*ap) {
			idesc->id_blkno = *ap;
			ret = iblock(idesc, n,
				     (off_t)(dino.di_size - 
					     sblock.fs_bsize * NDADDR));
			if (ret & STOP)
				return (ret);
		}
	}
	return (KEEPON);
}

iblock(idesc, ilevel, isize)
	struct inodesc *idesc;
	register int ilevel;
	off_t isize;
{
	register daddr_t *ap;
	register daddr_t *aplim;
	int i, n, (*func)(), nif;
	long sizepb;
	register struct bufarea *bp;
	char buf[BUFSIZ];
	extern int dirscan(), pass1check();

	if (idesc->id_type == ADDR) {
		func = idesc->id_func;
		if (((n = (*func)(idesc)) & KEEPON) == 0)
			return (n);
	} else
		func = dirscan;
	if (chkrange(idesc->id_blkno, idesc->id_numfrags))
		return (SKIP);
	bp = getdatablk(idesc->id_blkno, sblock.fs_bsize);
	ilevel--;
	for (sizepb = sblock.fs_bsize, i = 0; i < ilevel; i++)
		sizepb *= NINDIR(&sblock);
	if (debug)
		printf("iblock: inode %d isize %ld, sizepb %ld ilevel %d\n",
		       idesc->id_number, isize, sizepb, ilevel);
	nif = isize / sizepb + 1L;
	if (nif > NINDIR(&sblock))
		nif = NINDIR(&sblock);
	if (debug)
		printf("iblock: nif %d\n", nif);
	if (idesc->id_func == pass1check && nif < NINDIR(&sblock)) {
		aplim = &bp->b_un.b_indir[NINDIR(&sblock)];
		for (ap = &bp->b_un.b_indir[nif]; ap < aplim; ap++) {
			if (*ap == 0)
				continue;
			(void)sprintf(buf, "PARTIALLY TRUNCATED INODE I=%lu",
				idesc->id_number);
			if (dofix(idesc, buf)) {
				*ap = 0;
				dirty(bp);
			}
		}
		flush(fswritefd, bp);
	}
	aplim = &bp->b_un.b_indir[nif];
	for (ap = bp->b_un.b_indir, i = 1; ap < aplim; ap++, i++) {
		if (*ap) {
			idesc->id_blkno = *ap;
			if (ilevel > 0)
				n = iblock(idesc, ilevel, 
					   (off_t)(isize - i * sizepb));
			else
				n = (*func)(idesc);
			if (n & STOP) {
				bp->b_flags &= ~B_INUSE;
				return (n);
			}
		}
	}
	bp->b_flags &= ~B_INUSE;
	return (KEEPON);
}

/*
 * Check that a block in a legal block number.
 * Return 0 if in range, 1 if out of range.
 */
chkrange(blk, cnt)
	daddr_t blk;
	int cnt;
{
	register int c;

	/*
	 * Extra paranoia check for very large or negative disk addresses
	 */
	if (blk < 0 || cnt < 0)
		return(1);
	if ((unsigned)(blk + cnt) > maxfsblock)
		return (1);
	c = dtog(&sblock, blk);
	if (blk < cgdmin(&sblock, c)) {
		if ((blk + cnt) > cgsblock(&sblock, c)) {
			if (debug) {
				printf("blk %ld < cgdmin %ld;",
				    blk, cgdmin(&sblock, c));
				printf(" blk + cnt %ld > cgsbase %ld\n",
				    blk + cnt, cgsblock(&sblock, c));
			}
			return (1);
		}
	} else {
		if ((blk + cnt) > cgbase(&sblock, c+1)) {
			if (debug)  {
				printf("blk %ld >= cgdmin %ld;",
				    blk, cgdmin(&sblock, c));
				printf(" blk + cnt %ld > sblock.fs_fpg %ld\n",
				    blk+cnt, sblock.fs_fpg);
			}
			return (1);
		}
	}
	return (0);
}

/*
 * General purpose interface for reading inodes.
 */
struct dinode *
ginode(inumber)
	ino_t inumber;
{
	daddr_t iblk;
#if SEC_FSCHANGE
        struct dinode *dp;
#endif

	if (inumber < ROOTINO || inumber > maxino)
		errexit("bad inode number %d to ginode\n", inumber);
	if (startinum == 0 ||
	    inumber < startinum || inumber >= startinum + INOPB(&sblock)) {
		iblk = itod(&sblock, inumber);
		if (pbp != 0)
			pbp->b_flags &= ~B_INUSE;
		pbp = getdatablk(iblk, sblock.fs_bsize);
		startinum = (inumber / INOPB(&sblock)) * INOPB(&sblock);
	}
#if SEC_FSCHANGE
        disk_inode_in_block(&sblock, pbp->b_un.b_buf, &dp, inumber);
        return(dp);
#else
        return (&pbp->b_un.b_dinode[inumber % INOPB(&sblock)]);
#endif
}

/*
 * Special purpose version of ginode used to optimize first pass
 * over all the inodes in numerical order.
 */
ino_t nextino, lastinum;
int readcnt, readpercg, fullcnt, inobufsize, partialcnt, partialsize;
#if SEC_FSCHANGE
char *inodebuf;
#else
struct dinode *inodebuf;
#endif

struct dinode *
getnextinode(inumber)
	ino_t inumber;
{
	int size;
	daddr_t dblk;
	static struct dinode *dp;

	if (inumber != nextino++ || inumber > maxino)
		errexit("bad inode number %d to nextinode\n", inumber);
	if (inumber >= lastinum) {
		readcnt++;
		dblk = fsbtodb(&sblock, itod(&sblock, lastinum));
		if (readcnt % readpercg == 0) {
			size = partialsize;
			lastinum += partialcnt;
		} else {
			size = inobufsize;
			lastinum += fullcnt;
		}
		(void)bread(fsreadfd, (char *)inodebuf, dblk, size); /* ??? */
#if SEC_FSCHANGE
		dp = (struct dinode *) inodebuf;
		return dp;
#else
		dp = inodebuf;
#endif
	}
#if SEC_FSCHANGE
	else {
		/*
		 * if last inode in block, increment to next block
		 */

		if ((inumber % INOPB(&sblock)) == 0) {
			dp = (struct dinode *) (((int) dp) -
				((INOPB(&sblock) - 1) * disk_dinode_size()) +
				sblock.fs_bsize);
		}
		else
			disk_inode_incr(&dp, 1);
		return dp;
	}
#else
	return (dp++);
#endif
}

resetinodebuf()
{

	startinum = 0;
	nextino = 0;
	lastinum = 0;
	readcnt = 0;
	inobufsize = blkroundup(&sblock, INOBUFSIZE);
#if SEC_FSCHANGE
	fullcnt = howmany(inobufsize, sblock.fs_bsize) * INOPB(&sblock);
#else
	fullcnt = inobufsize / sizeof(struct dinode);
#endif
	readpercg = sblock.fs_ipg / fullcnt;
	partialcnt = sblock.fs_ipg % fullcnt;
#if SEC_FSCHANGE
	partialsize = howmany(partialcnt, INOPB(&sblock)) * sblock.fs_bsize;
#else
	partialsize = partialcnt * sizeof(struct dinode);
#endif
	if (partialcnt != 0) {
		readpercg++;
	} else {
		partialcnt = fullcnt;
		partialsize = inobufsize;
	}
#if SEC_FSCHANGE
	if (inodebuf == NULL &&
	    (inodebuf = (char *) malloc((unsigned)inobufsize)) == NULL)
		errexit("Cannot allocate space for inode buffer\n");
#else
	if (inodebuf == NULL &&
	    (inodebuf = (struct dinode *)malloc((unsigned)inobufsize)) == NULL)
		errexit("Cannot allocate space for inode buffer\n");
#endif
	while (nextino < ROOTINO)
		(void)getnextinode(nextino);
}

freeinodebuf()
{

	if (inodebuf != NULL)
		free((char *)inodebuf);
	inodebuf = NULL;
}

/*
 * Routines to maintain information about directory inodes.
 * This is built during the first pass and used during the
 * second and third passes.
 *
 * Enter inodes into the cache.
 */
cacheino(dp, inumber)
	register struct dinode *dp;
	ino_t inumber;
{
	register struct inoinfo *inp;
	struct inoinfo **inpp;
	unsigned int blks;

	blks = howmany(dp->di_size, sblock.fs_bsize);
	if (blks > NDADDR)
		blks = NDADDR + NIADDR;
	inp = (struct inoinfo *)
		malloc(sizeof(*inp) + (blks - 1) * sizeof(daddr_t));
	if (inp == NULL)
		return;
	inpp = &inphead[inumber % numdirs];
	inp->i_nexthash = *inpp;
	*inpp = inp;
	inp->i_parent = (ino_t)0;
	inp->i_dotdot = (ino_t)0;
	inp->i_number = inumber;
	inp->i_isize = dp->di_size;
	inp->i_flags = dp->di_flags;
	inp->i_numblks = blks * sizeof(daddr_t);
	bcopy((char *)&dp->di_db[0], (char *)&inp->i_blks[0],
	    (size_t)inp->i_numblks);
	if (inplast == listmax) {
		listmax += 100;
		inpsort = (struct inoinfo **)realloc((char *)inpsort,
		    (unsigned)listmax * sizeof(struct inoinfo *));
		if (inpsort == NULL)
			errexit("cannot increase directory list");
	}
	inpsort[inplast++] = inp;
}

/*
 * Look up an inode cache structure.
 */
struct inoinfo *
getinoinfo(inumber)
	ino_t inumber;
{
	register struct inoinfo *inp;

	for (inp = inphead[inumber % numdirs]; inp; inp = inp->i_nexthash) {
		if (inp->i_number != inumber)
			continue;
		return (inp);
	}
	errexit("cannot find inode %d\n", inumber);
	return ((struct inoinfo *)0);
}

/*
 * Clean up all the inode cache structure.
 */
inocleanup()
{
	register struct inoinfo **inpp;

	if (inphead == NULL)
		return;
	for (inpp = &inpsort[inplast - 1]; inpp >= inpsort; inpp--)
		free((char *)(*inpp));
	free((char *)inphead);
	free((char *)inpsort);
	inphead = inpsort = NULL;
}
	
inodirty()
{
	
	dirty(pbp);
}

clri(idesc, type, flag)
	register struct inodesc *idesc;
	char *type;
	int flag;
{
	register struct dinode *dp;

	dp = ginode(idesc->id_number);
	if (flag == 1) {
		pwarn("%s %s", type,
		    (dp->di_mode & IFMT) == IFDIR ? "DIR" : "FILE");
		pinode(idesc->id_number);
	}
	if (preen || reply("CLEAR") == 1) {
		if (preen)
			printf(" (CLEARED)\n");
		n_files--;
		(void)ckinode(dp, idesc);
		clearinode(dp);
		statemap[idesc->id_number] = USTATE;
		inodirty();
	}
}

findname(idesc)
	struct inodesc *idesc;
{
	register struct dirent *dirp = idesc->id_dirp;

	if (dirp->d_fileno != idesc->id_parent)
		return (KEEPON);
	bcopy(dirp->d_name, idesc->id_name, (size_t)dirp->d_namlen + 1);
	return (STOP|FOUND);
}

findino(idesc)
	struct inodesc *idesc;
{
	register struct dirent *dirp = idesc->id_dirp;

	if (dirp->d_fileno == 0)
		return (KEEPON);
	if (strcmp(dirp->d_name, idesc->id_name) == 0 &&
	    dirp->d_fileno >= ROOTINO && dirp->d_fileno <= maxino) {
		idesc->id_parent = dirp->d_fileno;
		return (STOP|FOUND);
	}
	return (KEEPON);
}

pinode(ino)
	ino_t ino;
{
	register struct dinode *dp;
	register char *p;
	struct passwd *pw;
	char *ctime();

	printf(" I=%lu ", ino);
	if (ino < ROOTINO || ino > maxino)
		return;
	dp = ginode(ino);
	printf(" OWNER=");
#ifndef SAS
	if ((pw = getpwuid((int)dp->di_uid)) != 0)
		printf("%s ", pw->pw_name);
	else
#endif SAS
		printf("%u ", (unsigned)dp->di_uid);
	printf("MODE=%o\n", dp->di_mode);
	if (preen)
		printf("%s: ", devname);
	printf("SIZE=%lu ", dp->di_size);
	p = ctime(&dp->di_mtime);
	printf("MTIME=%12.12s %4.4s ", p + 4, p + 20);
}

blkerror(ino, type, blk)
	ino_t ino;
	char *type;
	daddr_t blk;
{

	pfatal("%ld %s I=%lu", blk, type, ino);
	printf("\n");
	switch (statemap[ino]) {

	case FSTATE:
		statemap[ino] = FCLEAR;
		return;

	case DSTATE:
		statemap[ino] = DCLEAR;
		return;

	case FCLEAR:
	case DCLEAR:
		return;

	default:
		errexit("BAD STATE %d TO BLKERR", statemap[ino]);
		/* NOTREACHED */
	}
}

/*
 * allocate an unused inode
 */
ino_t
allocino(request, type)
	ino_t request;
	int type;
{
	register ino_t ino;
	register struct dinode *dp;

	if (request == 0)
		request = ROOTINO;
	else if (statemap[request] != USTATE)
		return (0);
	for (ino = request; ino < maxino; ino++)
		if (statemap[ino] == USTATE)
			break;
	if (ino == maxino)
		return (0);
	switch (type & IFMT) {
	case IFDIR:
		statemap[ino] = DSTATE;
		break;
	case IFREG:
	case IFLNK:
		statemap[ino] = FSTATE;
		break;
	default:
		return (0);
	}
	dp = ginode(ino);
	dp->di_db[0] = allocblk((int)1);
	if (dp->di_db[0] == 0) {
		statemap[ino] = USTATE;
		return (0);
	}
	dp->di_mode = type;
	(void)time(&dp->di_atime);
	dp->di_mtime = dp->di_ctime = dp->di_atime;
	dp->di_size = (off_t)sblock.fs_fsize;
	dp->di_blocks = btodb(sblock.fs_fsize);
#if SEC_FSCHANGE
	/*
	 * Zero extra fields in the security part of the inode
	 */
	bzero(&(((struct sec_dinode *) dp)->di_sec), sizeof(struct dinode_sec));
#endif
	n_files++;
	inodirty();
	return (ino);
}

/*
 * deallocate an inode
 */
freeino(ino)
	ino_t ino;
{
	struct inodesc idesc;
	extern int pass4check();
	struct dinode *dp;

	bzero((char *)&idesc, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass4check;
	idesc.id_number = ino;
	dp = ginode(ino);
	(void)ckinode(dp, &idesc);
	clearinode(dp);
	inodirty();
	statemap[ino] = USTATE;
	n_files--;
}
