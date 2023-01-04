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
static char	*sccsid = "@(#)$RCSfile: dir.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/10/07 17:27:07 $";
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
#if SEC_ARCH
#include <sys/security.h>
#endif
#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#define _KERNEL
#include <ufs/dir.h>
#undef _KERNEL
#include <stdlib.h>
#include <string.h>
#include "fsck.h"

char	*lfname = "lost+found";
int	lfmode = 01777;
struct	dirtemplate emptydir = { 0, DIRBLKSIZ };
struct	dirtemplate dirhead = { 0, 12, 1, ".", 0, DIRBLKSIZ - 12, 2, ".." };

struct dirent	*fsck_readdir();
struct bufarea	*getdirblk();

/*
 * Propagate connected state through the tree.
 */
propagate()
{
	register struct inoinfo **inpp, *inp;
	struct inoinfo **inpend;
	int change;

	inpend = &inpsort[inplast];
	do {
		change = 0;
		for (inpp = inpsort; inpp < inpend; inpp++) {
			inp = *inpp;
			if (inp->i_parent == 0)
				continue;
			if (statemap[inp->i_parent] == DFOUND &&
			    statemap[inp->i_number] == DSTATE) {
				statemap[inp->i_number] = DFOUND;
				change++;
			}
		}
	} while (change > 0);
}

/*
 * Scan each entry in a directory block.
 */
dirscan(idesc)
	register struct inodesc *idesc;
{
	register struct dirent *dp;
	register struct bufarea *bp;
	int dsize, n;
	int blksiz;
	char dbuf[DIRBLKSIZ];

	if (idesc->id_type != DATA)
		errexit("wrong type to dirscan %d\n", idesc->id_type);
	if (idesc->id_entryno == 0 &&
	    (idesc->id_filesize & (DIRBLKSIZ - 1)) != 0)
		idesc->id_filesize = roundup(idesc->id_filesize, DIRBLKSIZ);
	blksiz = idesc->id_numfrags * sblock.fs_fsize;
	if (chkrange(idesc->id_blkno, idesc->id_numfrags)) {
		idesc->id_filesize -= (off_t)blksiz;
		return (SKIP);
	}
	idesc->id_loc = 0;
	for (dp = fsck_readdir(idesc); dp != NULL; dp = fsck_readdir(idesc)) {
		dsize = dp->d_reclen;
		bcopy((char *)dp, dbuf, (size_t)dsize);
		idesc->id_dirp = (struct dirent *)dbuf;
		if ((n = (*idesc->id_func)(idesc)) & ALTERED) {
			bp = getdirblk(idesc->id_blkno, blksiz);
			bcopy(dbuf, bp->b_un.b_buf + idesc->id_loc - dsize,
			    (size_t)dsize);
			dirty(bp);
			sbdirty();
		}
		if (n & STOP) 
			return (n);
	}
	return (idesc->id_filesize > 0 ? KEEPON : STOP);
}

/*
 * get next entry in a directory (i.e. at position idesc->id_loc).
 *
 *	The ufs design requires that if a dir entry is bad, it needs
 *	to be coalesced into the previous entry (unless it is the
 *	first entry in a DIRBLK).  So, what we will do is check the
 *	next entry to be returned, and also check the entry following
 *	it (if it is contained within a DIRBLK):
 *
 *	1. if on a DIRBLK boundary:
 *		a) if dircheck fails, make the entire entry null
 *		   for the DIRBLK;  return this entry.
 *		b) if dircheck is ok (i.e. dpok), check the following
 *		   entry (i.e. the 2nd entry in this DIRBLK);  since
 *		   if the following entry is bad it will need to be coalesced
 *		   into the 1st entry in this DIRBLK.
 *	2. if not on a DIRBLK boundary (i.e. dpok):
 *		a) if this is _not_ the last entry in this DIRBLK
 *		   (i.e. idesc->id_loc % DIRBLKSIZ), check the following entry
 *		   since if the following entry is bad it will need to be
 *		   coalesced into this current entry in this DIRBLK.
 *		b) if this entry is the last entry in this DIRBLK
 *		   it must be okay, since we've been checking the following
 *		   entry as we go (i.e. step#1b and step#2a above).
 */
struct dirent *
fsck_readdir(idesc)
	register struct inodesc *idesc;
{
	register struct dirent	*dp,	/* current entry */
				*ndp;	/* next entry    */
	register struct bufarea *bp;
	int size, blksiz, fix;

	blksiz = idesc->id_numfrags * sblock.fs_fsize;
	bp = getdirblk(idesc->id_blkno, blksiz);
	if (idesc->id_loc % DIRBLKSIZ == 0 && idesc->id_filesize > 0 &&
	    idesc->id_loc < blksiz) {
		dp = (struct dirent *)(bp->b_un.b_buf + idesc->id_loc);
		if (dircheck(idesc, dp))
			goto dpok;
		if (debug)
			printf("readdir.dp: bp=%#lx b_bno %d id_loc=%#lx id_blkno=%#lx\n",
				bp, bp->b_bno, idesc->id_loc, idesc->id_blkno);
		/* null out this entire DIRBLK entry */
		fix = dofix(idesc, "DIRECTORY CORRUPTED");
		/*
		 * ``dofix()'' eventually calls getpathname() which could
		 * re-use bp for another block. Read it in again...
		 */
		bp = getdirblk(idesc->id_blkno, blksiz);
		dp = (struct dirent *)(bp->b_un.b_buf + idesc->id_loc);
		dp->d_reclen = DIRBLKSIZ;
		dp->d_fileno = 0;
		dp->d_namlen = 0;
		dp->d_name[0] = '\0';
		if (fix)
			dirty(bp);
		idesc->id_loc += DIRBLKSIZ;
		idesc->id_filesize -= (off_t)DIRBLKSIZ;
		return (dp);
	}
dpok:
	if (idesc->id_filesize <= 0 || idesc->id_loc >= blksiz)
		return NULL;
	dp = (struct dirent *)(bp->b_un.b_buf + idesc->id_loc);
	idesc->id_loc += dp->d_reclen;		/* incr to next entry */
	idesc->id_filesize -= (off_t)dp->d_reclen;
	if ((idesc->id_loc % DIRBLKSIZ) == 0)
		return (dp);	/* this is last entry for this DIRBLK */
	ndp = (struct dirent *)(bp->b_un.b_buf + idesc->id_loc);
	if (idesc->id_loc < blksiz && idesc->id_filesize > 0 &&
	    dircheck(idesc, ndp) == 0) {
		if (debug)
			printf("readdir.ndp: bp=%#lx b_bno %d id_loc=%#lx id_blkno=%#lx\n",
				bp, bp->b_bno, idesc->id_loc, idesc->id_blkno);
		/* coalesce next entry into current entry */
		size = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
		idesc->id_loc -= dp->d_reclen;		/* reset to dp offset */
		fix = dofix(idesc, "DIRECTORY CORRUPTED");
		/*
		 * ``dofix()'' eventually calls getpathname() which could
		 * re-use bp for another block. Read it in again...
		 */
		bp = getdirblk(idesc->id_blkno, blksiz);
		dp = (struct dirent *)(bp->b_un.b_buf + idesc->id_loc);
		dp->d_reclen += size;
		if (fix)
			dirty(bp);
		idesc->id_loc += size;
		idesc->id_filesize -= (off_t)size;
	}
	return (dp);
}

/*
 * Verify that a directory entry is valid.
 * This is a superset of the checks made in the kernel.
 */
dircheck(idesc, dp)
	struct inodesc *idesc;
	register struct dirent *dp;
{
	register int size;
	register char *cp;
	int spaceleft;

	size = DIRSIZ(dp);
	spaceleft = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
	if (dp->d_fileno < maxino &&
	    dp->d_reclen != 0 &&
	    dp->d_reclen <= spaceleft &&
	    (dp->d_reclen & 0x3) == 0 &&
	    dp->d_reclen >= size &&
	    idesc->id_filesize >= (off_t)size &&
	    dp->d_namlen <= NAME_MAX) {
		if (dp->d_fileno == 0)
			return (1);
		for (cp = dp->d_name, size = 0; size < dp->d_namlen; size++)
			if (*cp == 0 || (*cp++ == '/'))
				goto baddir;
		if (*cp == 0)
			return (1);
	}
	/*
	 * we know directory entry is bad.
	 * (print out inconsistency if debugging)
	 */
baddir:
	if (debug) {
		size = DIRSIZ(dp);
		if (dp->d_fileno >= maxino)
			printf("dircheck: d_fileno=%ld >= maxino=%ld\n",
				(long)dp->d_fileno, (long)maxino);
		if (dp->d_reclen == 0)
			printf("dircheck: d_reclen zero\n");
		if (dp->d_reclen > spaceleft)
			printf("dircheck: d_reclen=%ld > spaceleft=%ld\n",
				(long)dp->d_reclen, (long)spaceleft);
		if (dp->d_reclen & 0x3)
			printf("dircheck: d_reclen=%ld & 0x3\n",
				(long)dp->d_reclen);
		if (dp->d_reclen < size)
			printf("dircheck: d_reclen=%ld < size=%ld\n",
				(long)dp->d_reclen, (long)size);
		if (idesc->id_filesize < (off_t)size)
			printf("dircheck: id_filesize=%ld < size=%ld\n",
				(long)idesc->id_filesize, (long)size);
		if (dp->d_namlen <= NAME_MAX)
			printf("dircheck: d_namlen=%ld > NAME_MAX=%ld\n",
				(long)dp->d_namlen, (long)NAME_MAX);
		for (cp = dp->d_name, size = 0; size < dp->d_namlen; size++)
			if (*cp == 0 || (*cp++ == '/'))
				break;
		if (size >= dp->d_namlen) {
			printf("dircheck: d_name=\"");
			for (cp=dp->d_name,size=0; size<dp->d_namlen; size++)
				printf("%c", cp);
			printf("\"\n");
		}
		if (dp->d_name[dp->d_namlen])
			printf("dircheck: d_name[d_namlen]=%c non-null\n",
				dp->d_name[dp->d_namlen]);
	}
	return (0);
}

direrror(ino, errmesg)
	ino_t ino;
	char *errmesg;
{

	fileerror(ino, ino, errmesg);
}

fileerror(cwd, ino, errmesg)
	ino_t cwd, ino;
	char *errmesg;
{
	register struct dinode *dp;
	char pathbuf[MAXPATHLEN + 1];

	pwarn("%s ", errmesg);
	pinode(ino);
	printf("\n");
	getpathname(pathbuf, cwd, ino);
	if (ino < ROOTINO || ino > maxino) {
		pfatal("NAME=%s\n", pathbuf);
		return;
	}
	dp = ginode(ino);
	if (ftypeok(dp))
		pfatal("%s=%s\n",
		    (dp->di_mode & IFMT) == IFDIR ? "DIR" : "FILE", pathbuf);
	else
		pfatal("NAME=%s\n", pathbuf);
}

adjust(idesc, lcnt)
	register struct inodesc *idesc;
	short lcnt;
{
	register struct dinode *dp;

	dp = ginode(idesc->id_number);
	if (dp->di_nlink == lcnt) {
		if (linkup(idesc->id_number, (ino_t)0) == 0)
			clri(idesc, "UNREF", 0);
	} else {
		pwarn("LINK COUNT %s", (lfdir == idesc->id_number) ? lfname :
			((dp->di_mode & IFMT) == IFDIR ? "DIR" : "FILE"));
		pinode(idesc->id_number);
		printf(" COUNT %d SHOULD BE %d",
			dp->di_nlink, dp->di_nlink - lcnt);
		if (preen) {
			if (lcnt < 0) {
				printf("\n");
				pfatal("LINK COUNT INCREASING");
			}
			printf(" (ADJUSTED)\n");
		}
		if (preen || reply("ADJUST") == 1) {
			dp->di_nlink -= lcnt;
			inodirty();
		}
	}
}

mkentry(idesc)
	struct inodesc *idesc;
{
	register struct dirent *dirp = idesc->id_dirp;
	struct dirent newent;
	int newlen, oldlen;

	newent.d_namlen = strlen(idesc->id_name);
	newlen = DIRSIZ(&newent);
	if (dirp->d_fileno != 0)
		oldlen = DIRSIZ(dirp);
	else
		oldlen = 0;
	if (dirp->d_reclen - oldlen < newlen)
		return (KEEPON);
	newent.d_reclen = dirp->d_reclen - oldlen;
	dirp->d_reclen = oldlen;
	dirp = (struct dirent *)(((char *)dirp) + oldlen);
	dirp->d_fileno = idesc->id_parent;	/* ino to be entered is in id_parent */
	dirp->d_reclen = newent.d_reclen;
	dirp->d_namlen = newent.d_namlen;
	bcopy(idesc->id_name, dirp->d_name, (size_t)dirp->d_namlen + 1);
	return (ALTERED|STOP);
}

chgino(idesc)
	struct inodesc *idesc;
{
	register struct dirent *dirp = idesc->id_dirp;

	if (bcmp(dirp->d_name, idesc->id_name, (int)dirp->d_namlen + 1))
		return (KEEPON);
	dirp->d_fileno = idesc->id_parent;
	return (ALTERED|STOP);
}

linkup(orphan, parentdir)
	ino_t orphan;
	ino_t parentdir;
{
	register struct dinode *dp;
	int lostdir;
	ino_t oldlfdir;
	struct inodesc idesc;
	char tempname[BUFSIZ];
	extern int pass4check();

	bzero((char *)&idesc, sizeof(struct inodesc));
	dp = ginode(orphan);
	lostdir = (dp->di_mode & IFMT) == IFDIR;
	pwarn("UNREF %s ", lostdir ? "DIR" : "FILE");
	pinode(orphan);
	if (preen && dp->di_size == 0)
		return (0);
	if (preen)
		printf(" (RECONNECTED)\n");
	else
		if (reply("RECONNECT") == 0)
			return (0);
	if (lfdir == 0) {
		dp = ginode(ROOTINO);
#if SEC_MAC
		/*
		 * Can't make lost+found directory in multilevel dir
		 */
		if (((struct sec_dinode *) dp)->di_sec.di_type_flags &
						SEC_I_MLD) {
			pfatal(
"SORRY. CANNOT CREATE lost+found DIRECTORY IN A MULTILEVEL DIRECTORY");
			printf("\n\n");
			return 0;
		}
#endif
		idesc.id_name = lfname;
		idesc.id_type = DATA;
		idesc.id_func = findino;
		idesc.id_number = ROOTINO;
		if ((ckinode(dp, &idesc) & FOUND) != 0) {
			lfdir = idesc.id_parent;
		} else {
			pwarn("NO lost+found DIRECTORY");
			if (preen || reply("CREATE")) {
				lfdir = allocdir(ROOTINO, (ino_t)0, lfmode);
				if (lfdir != 0) {
					if (makeentry(ROOTINO, lfdir, lfname) != 0) {
						if (preen)
							printf(" (CREATED)\n");
					} else {
						freedir(lfdir, ROOTINO);
						lfdir = 0;
						if (preen)
							printf("\n");
					}
				}
			}
		}
		if (lfdir == 0) {
			printf("\n");
			if (preen)
				pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY");
			else {
				pwarn("SORRY. CANNOT CREATE lost+found DIRECTORY");
				if (reply("USE ROOT INSTEAD of lost+found") == 1)
					lfdir = ROOTINO;
			}
			if (lfdir == 0) {
				pfatal("UNREF %s ", lostdir ? "DIR" : "FILE");
				printf("(I=%d) left orphaned.\n\n",
					orphan); 
				return (0);
			}
		}
	}
	dp = ginode(lfdir);
	if ((dp->di_mode & IFMT) != IFDIR) {
		pfatal("lost+found IS NOT A DIRECTORY");
		if (reply("REALLOCATE") == 0)
			return (0);
		oldlfdir = lfdir;
		if ((lfdir = allocdir(ROOTINO, (ino_t)0, lfmode)) == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
			return (0);
		}
		if ((changeino(ROOTINO, lfname, lfdir) & ALTERED) == 0) {
			pfatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
			return (0);
		}
		inodirty();
		idesc.id_type = ADDR;
		idesc.id_func = pass4check;
		idesc.id_number = oldlfdir;
		adjust(&idesc, lncntp[oldlfdir] + 1);
		lncntp[oldlfdir] = 0;
		dp = ginode(lfdir);
	}
	if (statemap[lfdir] != DFOUND) {
		pfatal("SORRY. NO lost+found DIRECTORY\n\n");
		return (0);
	}
	(void)lftempname(tempname, orphan);
	if (makeentry(lfdir, orphan, tempname) == 0) {
		pfatal("SORRY. NO SPACE IN lost+found DIRECTORY");
		printf("\n\n");
		return (0);
	}
	lncntp[orphan]--;
	if (lostdir) {
		if ((changeino(orphan, "..", lfdir) & ALTERED) == 0 &&
		    parentdir >= ROOTINO)
			(void)makeentry(orphan, lfdir, "..");
		/* we added orphan to lfdir, so we need to adjust
		 * the link-counts for lfdir to reflect the new entry.
		 */
		dp = ginode(lfdir);
		dp->di_nlink++;
		inodirty();
		lncntp[lfdir]++;
		pwarn("DIR I=%lu CONNECTED. ", orphan);
		if (parentdir >= ROOTINO) {	/* valid parentdir? */
			printf("PARENT WAS I=%lu\n", parentdir);
			/* in effect, we're removing the orphan from
			 * it's old parent, so fix those link-counts.
			 */
			dp = ginode(parentdir);
			dp->di_nlink--;
			inodirty();
			/* pass4 will fix the lncntp[parentdir] */
		}
		if (preen == 0)
			printf("\n");
	} else {
		/*
		 * Non directories connected to lost+found should
		 * have a link count of 1
		 */
		dp = ginode(orphan);
		dp->di_nlink = 1;
		inodirty();
		lncntp[orphan] = 1;
	}
	return (1);
}

/*
 * fix an entry in a directory.
 */
changeino(dir, name, newnum)
	ino_t dir;
	char *name;
	ino_t newnum;
{
	struct inodesc idesc;

	bzero((char *)&idesc, sizeof(struct inodesc));
	idesc.id_type = DATA;
	idesc.id_func = chgino;
	idesc.id_number = dir;
	idesc.id_fix = DONTKNOW;
	idesc.id_name = name;
	idesc.id_parent = newnum;	/* new value for name */
#if SEC_MAC
	/*
	 * Clear the multilevel child bit of the directory when moving it
	 */

	if (FsSEC(&sblock))
	{
		struct sec_dinode *secdchild;

		secdchild = (struct sec_dinode *) ginode(dir);
                if (secdchild->di_sec.di_type_flags & SEC_I_MLDCHILD) {
                        secdchild->di_sec.di_type_flags &= ~SEC_I_MLDCHILD;
                        inodirty();
                }
		return (ckinode((struct dinode *) secdchild, &idesc));
	}
#endif
	return (ckinode(ginode(dir), &idesc));
}

/*
 * make an entry in a directory
 */
makeentry(parent, ino, name)
	ino_t parent, ino;
	char *name;
{
	struct dinode *dp;
	struct inodesc idesc;
	char pathbuf[MAXPATHLEN + 1];
	
	if (parent < ROOTINO || parent >= maxino ||
	    ino < ROOTINO || ino >= maxino)
		return (0);
	bzero((char *)&idesc, sizeof(struct inodesc));
	idesc.id_type = DATA;
	idesc.id_func = mkentry;
	idesc.id_number = parent;
	idesc.id_parent = ino;	/* this is the inode to enter */
	idesc.id_fix = DONTKNOW;
	idesc.id_name = name;
	dp = ginode(parent);
	if (dp->di_size % (off_t)DIRBLKSIZ) {
		dp->di_size = (off_t)roundup(dp->di_size, DIRBLKSIZ);
		inodirty();
	}
	if ((ckinode(dp, &idesc) & ALTERED) != 0)
		return (1);
	getpathname(pathbuf, parent, parent);
	dp = ginode(parent);
	if (expanddir(dp, pathbuf) == 0)
		return (0);
	return (ckinode(dp, &idesc) & ALTERED);
}

/*
 * Attempt to expand the size of a directory
 */
expanddir(dp, name)
	register struct dinode *dp;
	char *name;
{
	daddr_t lastbn, newblk;
	register struct bufarea *bp;
	char *cp, firstblk[DIRBLKSIZ];

	lastbn = lblkno(&sblock, dp->di_size);
	if (lastbn >= NDADDR - 1 || dp->di_db[lastbn] == 0 || dp->di_size == 0)
		return (0);
	if ((newblk = allocblk(sblock.fs_frag)) == 0)
		return (0);
	dp->di_db[lastbn + 1] = dp->di_db[lastbn];
	dp->di_db[lastbn] = newblk;
	dp->di_size += (off_t)sblock.fs_bsize;
	dp->di_blocks += btodb(sblock.fs_bsize);
	bp = getdirblk(dp->di_db[lastbn + 1],
		(int)dblksize(&sblock, dp, lastbn + 1));
	if (bp->b_errs)
		goto bad;
	bcopy(bp->b_un.b_buf, firstblk, DIRBLKSIZ);
	bp = getdirblk(newblk, sblock.fs_bsize);
	if (bp->b_errs)
		goto bad;
	bcopy(firstblk, bp->b_un.b_buf, DIRBLKSIZ);
	for (cp = &bp->b_un.b_buf[DIRBLKSIZ];
	     cp < &bp->b_un.b_buf[sblock.fs_bsize];
	     cp += DIRBLKSIZ)
		bcopy((char *)&emptydir, cp, sizeof emptydir);
	dirty(bp);
	bp = getdirblk(dp->di_db[lastbn + 1],
		(int)dblksize(&sblock, dp, lastbn + 1));
	if (bp->b_errs)
		goto bad;
	bcopy((char *)&emptydir, bp->b_un.b_buf, sizeof emptydir);
	pwarn("NO SPACE LEFT IN %s", name);
	if (preen)
		printf(" (EXPANDED)\n");
	else if (reply("EXPAND") == 0)
		goto bad;
	dirty(bp);
	inodirty();
	return (1);
bad:
	dp->di_db[lastbn] = dp->di_db[lastbn + 1];
	dp->di_db[lastbn + 1] = 0;
	dp->di_size -= (off_t)sblock.fs_bsize;
	dp->di_blocks -= btodb(sblock.fs_bsize);
	freeblk(newblk, sblock.fs_frag);
	return (0);
}

/*
 * allocate a new directory
 */
allocdir(parent, request, mode)
	ino_t parent, request;
	int mode;
{
	ino_t ino;
	char *cp;
	struct dinode *dp;
	register struct bufarea *bp;

#if SEC_ARCH
	tag_t tags[SEC_TAG_COUNT];

	/*
	 * Copy the tags from the parent directory to
	 * the newly allocated inode
	 */

	if (FsSEC(&sblock)) {

		dp = ginode(parent);
		bcopy(((struct sec_dinode *)dp)->di_sec.di_tag,
				tags, sizeof(tags));
	}
#endif
	ino = allocino(request, IFDIR|mode);
	if (ino == 0) {
		return (0);
	}
	dirhead.dot_ino = ino;
	dirhead.dotdot_ino = parent;
	dp = ginode(ino);
#if SEC_ARCH
	if (FsSEC(&sblock))
		bcopy(tags, ((struct sec_dinode *)dp)->di_sec.di_tag,
			sizeof(tags));
#endif
	bp = getdirblk(dp->di_db[0], sblock.fs_fsize);
	if (bp->b_errs) {
		freeino(ino);
		return (0);
	}
	bcopy((char *)&dirhead, bp->b_un.b_buf, sizeof dirhead);
	for (cp = &bp->b_un.b_buf[DIRBLKSIZ];
	     cp < &bp->b_un.b_buf[sblock.fs_fsize];
	     cp += DIRBLKSIZ)
		bcopy((char *)&emptydir, cp, sizeof emptydir);
	dirty(bp);
	dp->di_nlink = 2;
	inodirty();
	if (ino == ROOTINO) {
		lncntp[ino] = dp->di_nlink;
		return(ino);
	}
	if (statemap[parent] != DSTATE && statemap[parent] != DFOUND) {
		freeino(ino);
		return (0);
	}
	statemap[ino] = statemap[parent];
	if (statemap[ino] == DSTATE) {
		lncntp[ino] = dp->di_nlink;
		lncntp[parent]++;
	}
	dp = ginode(parent);
	dp->di_nlink++;
	inodirty();
	return (ino);
}

/*
 * free a directory inode
 */
freedir(ino, parent)
	ino_t ino, parent;
{
	struct dinode *dp;

	if (ino != parent) {
		dp = ginode(parent);
		dp->di_nlink--;
		inodirty();
	}
	freeino(ino);
}

/*
 * generate a temporary name for the lost+found directory.
 */
lftempname(bufp, ino)
	char *bufp;
	ino_t ino;
{
	register ino_t in;
	register char *cp;
	int namlen;

	cp = bufp + 2;
	for (in = maxino; in > 0; in /= 10)
		cp++;
	*--cp = 0;
	namlen = cp - bufp;
	in = ino;
	while (cp > bufp) {
		*--cp = (in % 10) + '0';
		in /= 10;
	}
	*cp = '#';
	return (namlen);
}

/*
 * Get a directory block.
 * Insure that it is held until another is requested.
 */
struct bufarea *
getdirblk(blkno, size)
	daddr_t blkno;
	int size;
{

	if (pdirbp != 0)
		pdirbp->b_flags &= ~B_INUSE;
	pdirbp = getdatablk(blkno, size);
	return (pdirbp);
}
