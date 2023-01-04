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
static char	*sccsid = "@(#)$RCSfile: mkproto.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1994/01/10 18:09:41 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mkproto.c,v $ $Revision: 4.2.5.4 $ (OSF) $Date: 1994/01/10 18:09:41 $";
#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/***
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
***/

/*** "mkproto.c	5.6 (Berkeley) 11/1/89"; ***/

/*
 * Make a file system prototype.
 * usage: mkproto filsys proto
 */
#include <sys/param.h>
#include <sys/dir.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <stdio.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "mkproto_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MKPROTO,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

union {
	struct	fs fs;
	char	fsx[SBSIZE];
} ufs;
#define sblock	ufs.fs
union {
	struct	cg cg;
	char	cgx[MAXBSIZE];
} ucg;
#define	acg	ucg.cg
struct	fs *fs;
struct	csum *fscs;
int	fso, fsi;
FILE	*proto;
char	token[BUFSIZ];
int	errs;
long	dev_bsize = 1;
int	ino = 10;
long	getnum();
char	*strcpy();
ino_t	ialloc();

main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	char *calloc();

#ifdef NLS
	setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_MKPROTO,NL_CAT_LOCALE);
#endif
	if (argc != 3) {
		fprintf(stderr, MSGSTR(USAGE, "usage: mkproto filsys proto\n"));
		exit(1);
	}
	fso = open(argv[1], 1);
	fsi = open(argv[1], 0);
	if (fso < 0 || fsi < 0) {
		perror(argv[1]);
		exit(1);
	}
	fs = &sblock;
	rdfs(SBOFF, SBSIZE, (char *)fs);
	dev_bsize = fs->fs_fsize / fsbtodb(fs, 1);
	fscs = (struct csum *)calloc(1, (size_t)fs->fs_cssize);
	for (i = 0; i < fs->fs_cssize; i += fs->fs_bsize)
		rdfs(fsbtodb(fs, fs->fs_csaddr + numfrags(fs, i)),
			(int)(fs->fs_cssize - i < fs->fs_bsize ?
			    fs->fs_cssize - i : fs->fs_bsize),
			((char *)fscs) + i);
	proto = fopen(argv[2], "r");
	descend((struct dinode *)0, ROOTINO);
	wtfs(SBOFF / dev_bsize, SBSIZE, (char *)fs);
	for (i = 0; i < fs->fs_cssize; i += fs->fs_bsize)
		wtfs(fsbtodb(&sblock, fs->fs_csaddr + numfrags(&sblock, i)),
			(int)(fs->fs_cssize - i < fs->fs_bsize ?
			    fs->fs_cssize - i : fs->fs_bsize),
			((char *)fscs) + i);
	exit(errs);
}

descend(par, parinum)
	struct dinode *par;
	ino_t parinum;
{
	struct dinode in;
	ino_t inum;
	int ibc = 0;
	int i, f, c;
	struct dinode *dip, inos[MAXBSIZE / sizeof (struct dinode)];
	daddr_t ib[MAXBSIZE / sizeof (daddr_t)];
	char buf[MAXBSIZE];

	getstr();
	in.di_mode = gmode(token[0], "-bcd", IFREG, IFBLK, IFCHR, IFDIR);
	in.di_mode |= gmode(token[1], "-u", 0, ISUID, 0, 0);
	in.di_mode |= gmode(token[2], "-g", 0, ISGID, 0, 0);
	for (i = 3; i < 6; i++) {
		c = token[i];
		if (c < '0' || c > '7') {
			fprintf(stderr,MSGSTR(BADOCT, "%c/%s: bad octal mode digit\n"), c, token);
			errs++;
			c = 0;
		}
		in.di_mode |= (c-'0')<<(15-3*i);
	}
	in.di_uid = getnum(); in.di_gid = getnum();
	for (i = 0; i < fs->fs_bsize; i++)
		buf[i] = 0;
	for (i = 0; i < NINDIR(fs); i++)
		ib[i] = (daddr_t)0;
	in.di_nlink = 1;
	in.di_size = 0;
	in.di_blocks = 0;
	in.di_flags = 0;
	for (i = 0; i < NDADDR; i++)
		in.di_db[i] = (daddr_t)0;
	for (i = 0; i < NIADDR; i++)
		in.di_ib[i] = (daddr_t)0;
	if (par != (struct dinode *)0) {
		inum = ialloc(&in);
	} else {
		par = &in;
		i = itod(fs, ROOTINO);
		rdfs(fsbtodb(fs, i), fs->fs_bsize, (char *)inos);
		dip = &inos[ROOTINO % INOPB(fs)];
		inum = ROOTINO;
		in.di_nlink = dip->di_nlink;
		in.di_size = dip->di_size;
		in.di_blocks = dip->di_blocks;
		in.di_db[0] = dip->di_db[0];
		rdfs(fsbtodb(fs, in.di_db[0]), fs->fs_bsize, buf);
	}

	switch (in.di_mode&IFMT) {

	case IFREG:
		getstr();
		f = open(token, 0);
		if (f < 0) {
			fprintf(stderr,MSGSTR(CANNOTOP, "%s: cannot open\n"), token);
			errs++;
			break;
		}
		while ((i = read(f, buf, (size_t)fs->fs_bsize)) > 0) {
		        int size;
			in.di_size += i;
			size = (int)dblksize(fs, &in, ibc);
			in.di_blocks += btodb(size);
			newblk(buf, &ibc, ib, size);
		}
		close(f);
		break;

	case IFBLK:
	case IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		in.di_rdev = makedev(i, f);
		break;

	case IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		if (inum != ROOTINO) {
			par->di_nlink++;
			in.di_nlink++;
			entry(&in, inum, ".", buf);
			entry(&in, parinum, "..", buf);
		}
		for (;;) {
			getstr();
			if (token[0]=='$' && token[1]=='\0')
				break;
			entry(&in, (ino_t)(ino+1), token, buf);
			descend(&in, inum);
		}
		in.di_size = (in.di_size + DIRBLKSIZ - 1) & (-DIRBLKSIZ); /* round to block size */
		in.di_blocks = btodb(fragroundup(fs, in.di_size));
		if (inum != ROOTINO)
			newblk(buf, &ibc, ib, (int)dblksize(fs, &in, 0));
		else
			wtfs(fsbtodb(fs, in.di_db[0]), (int)fs->fs_bsize, buf);
		break;
	}
	iput(&in, &ibc, ib, inum);
}

/*ARGSUSED*/
gmode(c, s, m0, m1, m2, m3)
	char c, *s;
{
	int i;

  	for (i = 0; s[i]; i++)
  		if (c == s[i])
			switch(i) {
			
				case 0: return(m0);

				case 1: return(m1);

				case 2: return(m2);

				case 3: return(m3);

				default: fprintf(stderr,MSGSTR(BADMODE, "%c/%s: bad mode\n"), c, token);
  				         errs++;
  			                 return(0);
				}
   
}

long
getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	i = 0;
	for (i = 0; c=token[i]; i++) {
		if (c<'0' || c>'9') {
			fprintf(stderr, MSGSTR(BADNUM, "%s: bad number\n"), token);
			errs++;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch (c = getc(proto)) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case EOF:
		fprintf(stderr, MSGSTR(UNXEOF, "Unexpected EOF\n"));
		exit(1);

	case ':':
		while (getc(proto) != '\n')
			;
		goto loop;

	}
	i = 0;
	do {
		token[i++] = c;
		c = getc(proto);
	} while (c != ' ' && c != '\t' && c != '\n' && c != '\0');
	token[i] = 0;
}

entry(ip, inum, str, buf)
	struct dinode *ip;
	ino_t inum;
	char *str;
	char *buf;
{
	register struct dirent *dp, *odp;
	int oldsize, newsize, spacefree;

	odp = dp = (struct dirent *)buf;
	while ((char *)dp - (char *)buf < ip->di_size) {
		odp = dp;
		dp = (struct dirent *)((char *)dp + dp->d_reclen);
	}
	if (odp != dp)
		oldsize = DIRSIZ(odp);
	else
		oldsize = 0;
	spacefree = odp->d_reclen - oldsize;
	dp = (struct dirent *)((char *)odp + oldsize);
	dp->d_ino = inum;
	dp->d_namlen = strlen(str);
	newsize = DIRSIZ(dp);
	if (spacefree >= newsize) {
		odp->d_reclen = oldsize;
		dp->d_reclen = spacefree;
	} else {
		dp = (struct dirent *)((char *)odp + odp->d_reclen);
		if ((char *)dp - (char *)buf >= fs->fs_bsize) {
			fprintf(stderr, MSGSTR(DIRLARG, "directory too large\n"));
			exit(1);
		}
		dp->d_ino = inum;
		dp->d_namlen = strlen(str);
		dp->d_reclen = DIRBLKSIZ;
	}
	strcpy(dp->d_name, str);
	ip->di_size = ((char *)dp - (char *)buf) + newsize;
}

newblk(buf, aibc, ib, size)
	int *aibc;
	char *buf;
	daddr_t *ib;
	int size;
{
	int i;
	daddr_t bno, alloc();

	bno = alloc(size);
	wtfs(fsbtodb(fs, bno), (int)fs->fs_bsize, buf);
	for (i = 0; i < fs->fs_bsize; i++)
		buf[i] = 0;
	ib[(*aibc)++] = bno;
	if (*aibc >= NINDIR(fs)) {
		fprintf(stderr,MSGSTR(INDBLKFULL, "indirect block full\n"));
		errs++;
		*aibc = 0;
	}
}

iput(ip, aibc, ib, inum)
	struct dinode *ip;
	int *aibc;
	daddr_t *ib;
	ino_t inum;
{
	daddr_t d, alloc();
	int i;
	struct dinode buf[MAXBSIZE / sizeof (struct dinode)];
	time_t time();

	ip->di_atime = ip->di_mtime = ip->di_ctime = time((time_t *)NULL);
	switch (ip->di_mode&IFMT) {

	case IFDIR:
	case IFREG:
		for (i = 0; i < *aibc; i++) {
			if (i >= NDADDR)
				break;
			ip->di_db[i] = ib[i];
		}
		if (*aibc > NDADDR) {
			ip->di_ib[0] = alloc((int)fs->fs_bsize);
 			ip->di_blocks += btodb((int)fs->fs_bsize);
			for (i = 0; i < NINDIR(fs) - NDADDR; i++) {
				ib[i] = ib[i+NDADDR];
				ib[i+NDADDR] = (daddr_t)0;
			}
			wtfs(fsbtodb(fs, ip->di_ib[0]),
			    (int)fs->fs_bsize, (char *)ib);
		}
		break;

	case IFBLK:
	case IFCHR:
		break;

	default:
		fprintf(stderr,MSGSTR(BADMOD2, "bad mode %o\n"), ip->di_mode);
		exit(1);
	}
	d = fsbtodb(fs, itod(fs, inum));
	rdfs(d, (int)fs->fs_bsize, (char *)buf);
	buf[itoo(fs, inum)] = *ip;
	wtfs(d, (int)fs->fs_bsize, (char *)buf);
}

daddr_t
alloc(size)
	int size;
{
	int i, frag;
	daddr_t d;
	static int cg = 0;

again:
	rdfs(fsbtodb(&sblock, cgtod(&sblock, cg)), (int)sblock.fs_cgsize,
	    (char *)&acg);
	if (!cg_chkmagic(&acg)) {
		fprintf(stderr, MSGSTR(MAGICNUM, "cg %d: bad magic number\n"), cg);
		return (0);
	}
	if (acg.cg_cs.cs_nbfree == 0) {
		cg++;
		if (cg >= fs->fs_ncg) {
			fprintf(stderr,MSGSTR(NOSPACE, "ran out of space\n"));
			return (0);
		}
		goto again;
	}
	for (d = 0; d < acg.cg_ndblk; d += sblock.fs_frag)
		if (isblock(&sblock, (u_char *)cg_blksfree(&acg),
		    d / sblock.fs_frag))
			goto goth;
	fprintf(stderr,MSGSTR(CYL, "internal error: can't find block in cyl %d\n"), cg);
	return (0);
goth:
	clrblock(&sblock, (u_char *)cg_blksfree(&acg), d / sblock.fs_frag);
	acg.cg_cs.cs_nbfree--;
	sblock.fs_cstotal.cs_nbfree--;
	fscs[cg].cs_nbfree--;
	cg_blktot(&acg)[cbtocylno(&sblock, d)]--;
	cg_blks(&sblock, &acg, cbtocylno(&sblock, d))[cbtorpos(&sblock, d)]--;
	if (size != sblock.fs_bsize) {
		frag = howmany(size, sblock.fs_fsize);
		fscs[cg].cs_nffree += sblock.fs_frag - frag;
		sblock.fs_cstotal.cs_nffree += sblock.fs_frag - frag;
		acg.cg_cs.cs_nffree += sblock.fs_frag - frag;
		acg.cg_frsum[sblock.fs_frag - frag]++;
		for (i = frag; i < sblock.fs_frag; i++)
			setbit(cg_blksfree(&acg), d + i);
	}
	wtfs(fsbtodb(&sblock, cgtod(&sblock, cg)), (int)sblock.fs_cgsize,
	    (char *)&acg);
	return (acg.cg_cgx * fs->fs_fpg + d);
}

/*
 * Allocate an inode on the disk
 */
ino_t
ialloc(ip)
	register struct dinode *ip;
{
	ino_t inum;
	int c;

	inum = ++ino;
	c = itog(&sblock, inum);
	rdfs(fsbtodb(&sblock, cgtod(&sblock, c)), (int)sblock.fs_cgsize,
	    (char *)&acg);
	if (!cg_chkmagic(&acg)) {
		fprintf(stderr,MSGSTR(MAGICNUM, "cg %d: bad magic number\n"), c);
		exit(1);
	}
	if ((ip->di_mode & IFMT) == IFDIR) {
		acg.cg_cs.cs_ndir++;
		sblock.fs_cstotal.cs_ndir++;
		fscs[c].cs_ndir++;
	}
	acg.cg_cs.cs_nifree--;
	setbit(cg_inosused(&acg), inum);
	wtfs(fsbtodb(&sblock, cgtod(&sblock, c)), (int)sblock.fs_cgsize,
	    (char *)&acg);
	sblock.fs_cstotal.cs_nifree--;
	fscs[c].cs_nifree--;
	if(inum >= sblock.fs_ipg * sblock.fs_ncg) {
		fprintf(stderr,MSGSTR(FSINIT, "fsinit: inode value out of range (%lu).\n"),
		    inum);
		exit(1);
	}
	return (inum);
}

/*
 * read a block from the file system
 */
rdfs(bno, size, bf)
	int bno, size;
	char *bf;
{
	int n;
	off_t lseek();

	if (lseek(fsi, ((off_t)bno * dev_bsize), 0) < 0) {
		fprintf(stderr,MSGSTR(SEEK, "seek error: %d\n"), bno);
		perror(MSGSTR(RDFS, "rdfs"));
		exit(1);
	}
	n = read(fsi, bf, size);
	if(n != size) {
		fprintf(stderr,MSGSTR(READ, "read error: %d\n"), bno);
		perror(MSGSTR(RDFS, "rdfs"));
		exit(1);
	}
}

/*
 * write a block to the file system
 */
wtfs(bno, size, bf)
	int bno, size;
	char *bf;
{
	int n;
	off_t lseek();

	if (lseek(fso, ((off_t)bno * dev_bsize), 0) < 0) {
		fprintf(stderr,MSGSTR(SEEK, "seek error: %d\n"), bno);
		perror(MSGSTR(WTFS, "wtfs"));
		exit(1);
	}
	n = write(fso, bf, size);
	if(n != size) {
		fprintf(stderr,MSGSTR(WRITE, "write error: %d\n"), bno);
		perror(MSGSTR(WTFS, "wtfs"));
		exit(1);
	}
}
/*
 * check if a block is available
 */
isblock(fs, cp, h)
	struct fs *fs;
	unsigned char *cp;
	int h;
{
	unsigned char mask;

	switch (fs->fs_frag) {
	case 8:
		return (cp[h] == 0xff);
	case 4:
		mask = 0x0f << ((h & 0x1) << 2);
		return ((cp[h >> 1] & mask) == mask);
	case 2:
		mask = 0x03 << ((h & 0x3) << 1);
		return ((cp[h >> 2] & mask) == mask);
	case 1:
		mask = 0x01 << (h & 0x7);
		return ((cp[h >> 3] & mask) == mask);
	default:
		fprintf(stderr, MSGSTR(ISBLOCK, "isblock bad fs_frag %ld\n"), fs->fs_frag);
		return (0);
	}
	/*NOTREACHED*/
}

/*
 * take a block out of the map
 */
clrblock(fs, cp, h)
	struct fs *fs;
	unsigned char *cp;
	int h;
{
	switch ((fs)->fs_frag) {
	case 8:
		cp[h] = 0;
		return;
	case 4:
		cp[h >> 1] &= ~(0x0f << ((h & 0x1) << 2));
		return;
	case 2:
		cp[h >> 2] &= ~(0x03 << ((h & 0x3) << 1));
		return;
	case 1:
		cp[h >> 3] &= ~(0x01 << (h & 0x7));
		return;
	default:
		fprintf(stderr, MSGSTR(CLRBLOCK, "clrblock bad fs_frag %ld\n"), fs->fs_frag);
		return;
	}
}

