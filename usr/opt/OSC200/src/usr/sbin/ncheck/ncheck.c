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
static char	*sccsid = "@(#)$RCSfile: ncheck.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/08/30 21:25:32 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint


/*
 * ncheck -- obtain file names from reading filesystem
 */

#define	NB		500
#define	MAXNINDIR	(MAXBSIZE / sizeof (daddr_t))

#include <sys/secdefines.h>

#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <sys/dir.h>
#include <stdio.h>

union {
	struct	fs	u_sblock;
	char	u_sbsize_buf[SBSIZE];
} sbsize_sblock;
#define sblock	sbsize_sblock.u_sblock

struct	dinode	itab[MAXBSIZE/sizeof(struct dinode)];
struct 	dinode	*gip;
struct ilist {
	ino_t	ino;
	mode_t	mode;
	uid_t	uid;
	gid_t	gid;
} ilist[NB];
struct	htab
{
	ino_t	h_ino;
	ino_t	h_pino;
	char	*h_name;
} *htab;
char *strngtab;
int hsize;
int strngloc;

struct dirstuff {
	int loc;
	struct dinode *ip;
	char dbuf[MAXBSIZE];
};

int	aflg;
int	sflg;
int	iflg; /* number of inodes being searched for */
int	mflg;
int	fi;
ino_t	ino;
int	nhent;
int	nxfile;
int	dev_bsize = 1;

int	nerror;
daddr_t	bmap();
int	atoi();
off_t	lseek();
char	*malloc(), *strcpy();
struct htab *lookup();
struct dirent *nreaddir();

main(argc, argv)
	int argc;
	char *argv[];
{
	int n;

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();

        if (!authorized_user("sysadmin")) {
                fprintf(stderr, "ncheck: need sysadmin authorization\n");
                exit(1);
        }
#endif

	for (--argc, ++argv; argc && **argv == '-'; --argc, ++argv) {
		switch ((*argv)[1]) {

		case 'a':
			aflg++;
			break;

		case 'i':
			/* check for list of numbers */
			if (argc == 1 || !isdigit(argv[1][0])) {
				(void) fprintf(stderr,
					"ncheck: -i requires a list ");
				(void) fprintf(stderr, "of i-numbers\n");
				exit(1);
			}
			for(iflg=0; iflg<NB; iflg++) {
				if (!isdigit(argv[1][0]))
					break;
				n = atoi(argv[1]);
				if(n == 0) {
					argc--;
					argv++;
					break;
				}
				ilist[iflg].ino = n;
				nxfile = iflg;
				argv++;
				argc--;
				/* Make sure the list continues */
				if (argc == 1)
					break;
			}
			break;

		case 'm':
			mflg++;
			break;

		case 's':
			sflg++;
			break;

		default:
			(void) fprintf(stderr, "ncheck: bad flag %c\n",
			    (*argv)[1]);
			exit(1);
		}
	}
	if (argc == 0) {
		(void) fprintf(stderr, "Usage: ncheck [-i numbers] [-a] [-s] ");
		(void) fprintf(stderr, "[-m] file-systems...\n");
		exit(1);
	}
	while (argc--)
		check(*argv++);
	return(nerror);
}

check(file)
	char *file;
{
	register int i, j, c;

	fi = open(file, 0);
	if(fi < 0) {
		(void) fprintf(stderr, "ncheck: cannot open ");
		(void) perror(file);
		nerror++;
		return;
	}
	nhent = 0;
	(void) printf("%s:\n", file);
	sync();
	dev_bsize = 1;
	bread(SBOFF, (char *)&sblock, SBSIZE);
#if SEC_FSCHANGE
        if (sblock.fs_magic != FS_MAGIC && sblock.fs_magic != FS_SEC_MAGIC)
#else
	if (sblock.fs_magic != FS_MAGIC) 
#endif
	{
		(void) printf("%s: not a file system\n", file);
		nerror++;
		return;
	}
#if SEC_FSCHANGE
        disk_set_file_system(&sblock, sblock.fs_bsize);
#endif
	dev_bsize = sblock.fs_fsize / fsbtodb(&sblock, 1);
	hsize = sblock.fs_ipg * sblock.fs_ncg - sblock.fs_cstotal.cs_nifree + 1;
	htab = (struct htab *)malloc((unsigned)hsize * sizeof(struct htab));
	strngtab = malloc((unsigned)(30 * hsize));
	if (htab == 0 || strngtab == 0) {
		(void) printf("not enough memory to allocate tables\n");
		nerror++;
		return;
	}
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		for (i = 0;
#if SEC_FSCHANGE
                     i / sblock.fs_frag * INOPB(&sblock) < sblock.fs_ipg;
#else
		     i < sblock.fs_ipg / INOPF(&sblock);
#endif
		     i += sblock.fs_frag) {
			bread(fsbtodb(&sblock, cgimin(&sblock, c) + i),
			    (char *)itab, sblock.fs_bsize);
			for (j = 0; j < INOPB(&sblock); j++) {
#if SEC_FSCHANGE
                                struct dinode *dip;

                                disk_inode_in_block(&sblock, (char *) itab,
                                                        &dip, ino);
                                if (dip->di_mode != 0)
                                        pass1(dip);
#else

				if (itab[j].di_mode != 0)
					pass1(&itab[j]);
#endif
				ino++;
			}
		}
	}
	ilist[nxfile+1].ino = 0;
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		for (i = 0;
#if SEC_FSCHANGE
                     i / sblock.fs_frag * INOPB(&sblock) < sblock.fs_ipg;
#else
		     i < sblock.fs_ipg / INOPF(&sblock);
#endif
		     i += sblock.fs_frag) {
			bread(fsbtodb(&sblock, cgimin(&sblock, c) + i),
			    (char *)itab, sblock.fs_bsize);
			for (j = 0; j < INOPB(&sblock); j++) {
#if SEC_FSCHANGE
                                struct dinode *dip;

                                disk_inode_in_block(&sblock, (char *) itab,
                                                        &dip, ino);
                                if (dip->di_mode != 0)
                                        pass2(dip);
#else

				if (itab[j].di_mode != 0)
					pass2(&itab[j]);
#endif
				ino++;
			}
		}
	}
	ino = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		for (i = 0;
#if SEC_FSCHANGE
                     i / sblock.fs_frag * INOPB(&sblock) < sblock.fs_ipg;
#else
		     i < sblock.fs_ipg / INOPF(&sblock);
#endif
		     i += sblock.fs_frag) {
			bread(fsbtodb(&sblock, cgimin(&sblock, c) + i),
			    (char *)itab, sblock.fs_bsize);
			for (j = 0; j < INOPB(&sblock); j++) {
#if SEC_FSCHANGE
                                struct dinode *dip;

                                disk_inode_in_block(&sblock, (char *) itab,
                                                        &dip, ino);
                                if (dip->di_mode != 0)
                                        pass3(dip);
#else

				if (itab[j].di_mode != 0)
					pass3(&itab[j]);
#endif
				ino++;
			}
		}
	}
	(void) close(fi);
	for (i = 0; i < hsize; i++)
		htab[i].h_ino = 0;
	for (i = iflg; i < NB; i++)
		ilist[i].ino = 0;
	nxfile = iflg;
}

pass1(ip)
	register struct dinode *ip;
{
	int i;

	if (mflg)
		for (i = 0; i < iflg; i++)
			if (ino == ilist[i].ino) {
				ilist[i].mode = ip->di_mode;
				ilist[i].uid = ip->di_uid;
				ilist[i].gid = ip->di_gid;
			}
	if ((ip->di_mode & IFMT) != IFDIR) {
		if (sflg==0 || nxfile>=NB)
			return;
		if ((ip->di_mode&IFMT)==IFBLK || (ip->di_mode&IFMT)==IFCHR
		  || ip->di_mode&(ISUID|ISGID)) {
			ilist[nxfile].ino = ino;
			ilist[nxfile].mode = ip->di_mode;
			ilist[nxfile].uid = ip->di_uid;
			ilist[nxfile++].gid = ip->di_gid;
			return;
		}
	}
	(void) lookup(ino, 1);
}

pass2(ip)
	register struct dinode *ip;
{
	register struct dirent *dp;
	struct dirstuff dirp;
	struct htab *hp;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	dirp.loc = 0;
	dirp.ip = ip;
	gip = ip;
	for (dp = nreaddir(&dirp); dp != NULL; dp = nreaddir(&dirp)) {
		if(dp->d_fileno == 0)
			continue;
		hp = lookup(dp->d_fileno, 0);
		if(hp == 0)
			continue;
		if(dotname(dp))
			continue;
		hp->h_pino = ino;
		hp->h_name = &strngtab[strngloc];
		strngloc += strlen(dp->d_name) + 1;
		(void) strcpy(hp->h_name, dp->d_name);
	}
}

pass3(ip)
	register struct dinode *ip;
{
	register struct dirent *dp;
	struct dirstuff dirp;
	int k;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	dirp.loc = 0;
	dirp.ip = ip;
	gip = ip;
	for(dp = nreaddir(&dirp); dp != NULL; dp = nreaddir(&dirp)) {
		if(aflg==0 && dotname(dp))
			continue;
		if(sflg == 0 && iflg == 0)
			goto pr;
		for(k = 0; ilist[k].ino != 0; k++)
			if(ilist[k].ino == dp->d_fileno)
				break;
		if (ilist[k].ino == 0)
			continue;
		if (mflg)
			(void) printf("mode %-6o uid %d gid %d ino ",
			    ilist[k].mode, ilist[k].uid, ilist[k].gid);
	pr:
		(void) printf("%-5lu\t", dp->d_fileno);
		pname(ino, 0);
		(void) printf("/%s", dp->d_name);
		if (lookup(dp->d_fileno, 0))
			(void) printf("/.");
		(void) printf("\n");
	}
}

/*
 * get next entry in a directory.
 */
struct dirent *
nreaddir(dirp)
	register struct dirstuff *dirp;
{
	register struct dirent *dp;
	daddr_t lbn, d;

	for(;;) {
		if (dirp->loc >= dirp->ip->di_size)
			return NULL;
		if (blkoff(&sblock, dirp->loc) == 0) {
			lbn = lblkno(&sblock, dirp->loc);
			d = bmap(lbn);
			if(d == 0)
				return NULL;
			bread(fsbtodb(&sblock, d), dirp->dbuf,
			    dblksize(&sblock, dirp->ip, lbn));
		}
		dp = (struct dirent *)
		    (dirp->dbuf + blkoff(&sblock, dirp->loc));
		dirp->loc += dp->d_reclen;
		if (dp->d_fileno == 0)
			continue;
		return (dp);
	}
}

dotname(dp)
	register struct dirent *dp;
{

	if (dp->d_name[0]=='.')
		if (dp->d_name[1]==0 ||
		   (dp->d_name[1]=='.' && dp->d_name[2]==0))
			return(1);
	return(0);
}

pname(i, lev)
	ino_t i;
	int lev;
{
	register struct htab *hp;

	if (i==ROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		(void) printf("???");
		return;
	}
	if (lev > 10) {
		(void) printf("...");
		return;
	}
	pname(hp->h_pino, ++lev);
	(void) printf("/%s", hp->h_name);
}

struct htab *
lookup(i, ef)
	ino_t i;
	int ef;
{
	register struct htab *hp;

	for (hp = &htab[i%hsize]; hp->h_ino;) {
		if (hp->h_ino==i)
			return(hp);
		if (++hp >= &htab[hsize])
			hp = htab;
	}
	if (ef==0)
		return(0);
	if (++nhent >= hsize) {
		(void) fprintf(stderr, "ncheck: hsize of %ld is too small\n",
		    hsize);
		exit(1);
	}
	hp->h_ino = i;
	return(hp);
}

bread(bno, buf, lcount)
	daddr_t bno;
	register char *buf;
	int lcount;
{
	register int i, cnt = lcount;
	register off_t off;

	off = (off_t)bno * dev_bsize;
	(void) lseek(fi, off, 0);
	if (read(fi, buf, cnt) != cnt) {
		(void) fprintf(stderr, "ncheck: read error %ld\n", bno);
		if (cnt % dev_bsize) {
			/* THIS INDICATES A SERIOUS BUG */
			/* bzero is probably not correct, but will do */
			(void) fprintf(stderr,
			    "ncheck: bread: cnt %d not multiple of %d\n",
			    cnt, dev_bsize);
			bzero(buf, cnt);
			return;
		}
		for (i = 0; i < cnt; i += dev_bsize) {
			(void) lseek(fi, off, 0);
			if (read(fi, buf, dev_bsize) != dev_bsize) {
				(void) fprintf(stderr,
				    "ncheck: re-read error %ld\n", bno);
				bzero(buf, dev_bsize);
			}
			off += (off_t)dev_bsize;
			buf += dev_bsize;
			bno++;
		}
	}
}

/*
 * Swiped from standalone sys.c.
 */
#define	NBUFS	4
char	b[NBUFS][MAXBSIZE];
daddr_t	blknos[NBUFS];

daddr_t
bmap(bn)
	register daddr_t bn;
{
	register int j;
	int i, sh;
	daddr_t nb, *bap;

	if (bn < 0) {
		(void) fprintf(stderr, "ncheck: bn %ld negative\n", bn);
		return ((daddr_t)0);
	}

	/*
	 * blocks 0..NDADDR are direct blocks
	 */
	if(bn < NDADDR)
		return(gip->di_db[bn]);

	/*
	 * addresses NIADDR have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	sh = 1;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(&sblock);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0) {
		(void) printf("ncheck: bn %ld ovf, ino %lu\n", bn, ino);
		return ((daddr_t)0);
	}

	/*
	 * fetch the first indirect block address from the inode
	 */
	nb = gip->di_ib[NIADDR - j];
	if (nb == 0) {
		(void) printf("ncheck: bn %ld void1, ino %lu\n", bn, ino);
		return ((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for (; j <= NIADDR; j++) {
		if (blknos[j] != nb) {
			bread(fsbtodb(&sblock, nb), b[j], sblock.fs_bsize);
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh /= NINDIR(&sblock);
		i = (bn / sh) % NINDIR(&sblock);
		nb = bap[i];
		if(nb == 0) {
			(void) printf("ncheck: bn %ld void2, ino %lu\n", bn,
			    ino);
			return ((daddr_t)0);
		}
	}
	return (nb);
}
