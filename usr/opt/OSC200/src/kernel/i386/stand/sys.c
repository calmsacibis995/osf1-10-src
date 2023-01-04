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
static char	*sccsid = "@(#)$RCSfile: sys.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:15:34 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include "small.h"

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/fs.h>
#include <sys/reboot.h>
#if	CMU && OLDFS
#include <sys/filsys.h>
#endif	CMU
#include <sys/dir.h>
#include "saio.h"

static ino_t	dlook();
char *calloc();
char *xx();

/* Types in r10 specifying major device */
char	devname[][2] = {
	'h','d',	/* 0 = hd */
	'f','p',	/* 1 = fp */
	'q','t',
};

struct dirstuff {
	int loc;
	struct iob *io;
};
static
openi(n, io)
	register struct iob *io;
{
	register struct dinode *dp;
	register int cc;

	io->i_offset = 0;
	io->i_bn = fsbtodb(io->i_fs, itod(io->i_fs, n)) + io->i_boff;
	io->i_cc = io->i_fs->fs_bsize;
	io->i_ma = io->i_buf;
	cc = devread(io);
	dp = (struct dinode *)io->i_buf;
#if	CMU && OLDFS
	if (isoldfs(io->i_fs))
	{
#define op	((struct oinode *)dp)
		op = &op[itooo(io->i_fs, n)];
		io->i_ino.i_number = n;
		io->i_ino.i_mode = op->oi_mode;
		io->i_ino.i_size = op->oi_size;
		l3tol((char *)io->i_ino.i_db, (char *)op->oi_addr, NOADDR-NIADDR);
		l3tol((char *)io->i_ino.i_ib, ((char *)op->oi_addr)+(3*(NOADDR-NIADDR)), NIADDR);
#undef	op
	}
	else
#endif	CMU
	io->i_ino.i_ic = dp[itoo(io->i_fs, n)].di_ic;
	return (cc);
}

static
find(path, file)
	register char *path;
	struct iob *file;
{
	register char *q;
	register char c;
	register int n = 0;

#ifndef	SMALL
	if (path==NULL || *path=='\0') {
		printf("null path\n");
		return (0);
	}
#endif	SMALL
	if (openi((ino_t) ROOTINO, file) < 0) {
		printf("bad root inode\n");
		return (0);
	}
	while (*path) {
		while (*path == '/')
			path++;
		q = path;
		while(*q != '/' && *q != '\0')
			q++;
		c = *q;
		*q = '\0';
		if (q == path) path = "." ;	/* "/" means "/." */

		if ((n = dlook(path, file)) != 0) {
			if (c == '\0')
				break;
			if (openi(n, file) < 0)
				return (0);
			*q = c;
			path = q;
			continue;
		} else {
			printf("%s: not found\n", path);
			return (0);
		}
	}
	return (n);
}

static daddr_t
sbmap(io, bn)
	register struct iob *io;
	register daddr_t bn;
{
	register struct inode *ip;
	int i, j, sh;
	register daddr_t nb, *bap;

	ip = &io->i_ino;
	if (bn < 0) {
		printf("bn negative\n");
		return ((daddr_t)0);
	}

	/*
	 * blocks 0..NDADDR are direct blocks
	 */
#if	CMU && OLDFS
	i = isoldfs(io->i_fs)?NOADDR-NIADDR:NDADDR;
	if (bn < i)
#else	CMU
	if(bn < NDADDR)
#endif	CMU
	{
		nb = ip->i_db[bn];
		return (nb);
	}

	/*
	 * addresses NIADDR have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	sh = 1;
#if	CMU && OLDFS
	bn -= isoldfs(io->i_fs)?NOADDR-NIADDR:NDADDR;
#else	CMU
	bn -= NDADDR;
#endif	CMU
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(io->i_fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0) {
		printf("bn ovf %D\n", bn);
		return ((daddr_t)0);
	}

	/*
	 * fetch the first indirect block address from the inode
	 */
	nb = ip->i_ib[NIADDR - j];
	if (nb == 0) {
		printf("bn void %D\n",bn);
		return ((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for (; j <= NIADDR; j++) {
		if (blknos[j] != nb) {
			io->i_bn = fsbtodb(io->i_fs, nb) + io->i_boff;
			if (b[j] == (char *)0)
				b[j] = calloc(MAXBSIZE);
			io->i_ma = b[j];
			io->i_cc = io->i_fs->fs_bsize;
			if (devread(io) != io->i_fs->fs_bsize) {
				if (io->i_error)
					errno = io->i_error;
				printf("bn %D: read error\n", io->i_bn);
				return ((daddr_t)0);
			}
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh /= NINDIR(io->i_fs);
		i = (bn / sh) % NINDIR(io->i_fs);
		nb = bap[i];
		if(nb == 0) {
			printf("bn void %D\n",bn);
			return ((daddr_t)0);
		}
	}

	return (nb);
}

static ino_t
dlook(s, io)
	char *s;
	register struct iob *io;
{
	register struct direct *dp;
	register struct inode *ip;
	struct dirstuff dirp;
	register int len;

	if (s == NULL || *s == '\0')
		return (0);
	ip = &io->i_ino;
	if ((ip->i_mode&IFMT) != IFDIR) {
		printf(". before %s not a dirn", s);
		return (0);
	}
	if (ip->i_size == 0) {
		printf("%s: 0 length dir\n", s);
		return (0);
	}
	len = strlen(s);
	dirp.loc = 0;
	dirp.io = io;
	for (dp = readdir(&dirp); dp != NULL; dp = readdir(&dirp)) {
		if(dp->d_ino == 0)
			continue;
		if (dp->d_namlen == len && !strcmp(s, dp->d_name))
			return (dp->d_ino);
	}
	return (0);
}

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register struct dirstuff *dirp;
{
	register struct direct *dp;
	register struct iob *io;
	register daddr_t lbn, d;
	register int off;

	io = dirp->io;
	for(;;) {
		if (dirp->loc >= io->i_ino.i_size)
			return (NULL);
		off = blkoff(io->i_fs, dirp->loc);
		if (off == 0) {
			lbn = lblkno(io->i_fs, dirp->loc);
			d = sbmap(io, lbn);
			if(d == 0)
				return NULL;
			io->i_bn = fsbtodb(io->i_fs, d) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = blksize(io->i_fs, &io->i_ino, lbn);
			if (devread(io) < 0) {
				errno = io->i_error;
				printf("bn %D: directory read error\n",
					io->i_bn);
				return (NULL);
			}
		}
		dp = (struct direct *)(io->i_buf + off);
		dirp->loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}

lseek(fdesc, addr, ptr)
	int fdesc, ptr;
	off_t addr;
{
	register struct iob *io;

#ifndef	SMALL
	if (ptr != 0) {
		printf("Seek not from beginning of file\n");
		errno = EOFFSET;
		return (-1);
	}
#endif	SMALL
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	io->i_offset = addr;
	io->i_bn = addr / DEV_BSIZE;
	io->i_cc = 0;
	return (0);
}

tell (fdesc)
	int fdesc;
{
	/* assume we know what we're doing... */
	if (fdesc >= 3)
		return iob[fdesc-3].i_offset;
	else
		return 0;
}

static getc(fdesc)
	int fdesc;
{
	register struct iob *io;
	register struct fs *fs;
	register char *p;
	int c, lbn, off, size, diff;

	if (fdesc >= 0 && fdesc <= 2)
		return (getchar());
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	p = io->i_ma;
	if (io->i_cc <= 0) {
		if ((io->i_flgs & F_FILE) != 0) {
			diff = io->i_ino.i_size - io->i_offset;
			if (diff <= 0)
				return (-1);
			fs = io->i_fs;
			lbn = lblkno(fs, io->i_offset);
			io->i_bn = fsbtodb(fs, sbmap(io, lbn)) + io->i_boff;
			off = blkoff(fs, io->i_offset);
			size = blksize(fs, &io->i_ino, lbn);
		} else {
#ifndef	SMALL
			io->i_bn = io->i_offset / DEV_BSIZE;
			off = 0;
			size = DEV_BSIZE;
#endif	SMALL
		}
		io->i_ma = io->i_buf;
		io->i_cc = size;
		if (devread(io) < 0) {
			errno = io->i_error;
			return (-1);
		}
		if ((io->i_flgs & F_FILE) != 0) {
			if (io->i_offset - off + size >= io->i_ino.i_size)
				io->i_cc = diff + off;
			io->i_cc -= off;
		}
		p = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = (unsigned)*p++;
	io->i_ma = p;
	return (c);
}

int	errno;

read(fdesc, buf, count)
	int fdesc, count;
	char *buf;
{
	register i, size;
	register struct iob *file;
	register struct fs *fs;
	register int lbn, off;

	errno = 0;
	if (fdesc >= 0 & fdesc <= 2) {
		i = count;
		do {
			*buf = getchar();
		} while (--i && *buf++ != '\n');
		return (count - i);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs&F_READ) == 0) {
		errno = EBADF;
		return (-1);
	}
#ifndef	SMALL
	if ((file->i_flgs & F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		file->i_bn = file->i_boff + (file->i_offset / DEV_BSIZE);
		i = devread(file);
		file->i_offset += count;
		if (i < 0)
			errno = file->i_error;
		return (i);
	}
#endif	SMALL
	if (file->i_offset+count > file->i_ino.i_size)
		count = file->i_ino.i_size - file->i_offset;
	if ((i = count) <= 0)
		return (0);
	/*
	 * While reading full blocks, do I/O into user buffer.
	 * Anything else uses getc().
	 */
	fs = file->i_fs;
	while (i) {
		off = blkoff(fs, file->i_offset);
		lbn = lblkno(fs, file->i_offset);
		size = blksize(fs, &file->i_ino, lbn);
		if (off == 0 && size <= i) {
			file->i_bn = fsbtodb(fs, sbmap(file, lbn)) +
			    file->i_boff;
			file->i_cc = size;
			file->i_ma = buf;
			if (devread(file) < 0) {
				errno = file->i_error;
				return (-1);
			}
			file->i_offset += size;
			file->i_cc = 0;
			buf += size;
			i -= size;
		} else {
			size -= off;
			if (size > i)
				size = i;
			i -= size;
			do {
				*buf++ = getc(fdesc+3);
			} while (--size);
		}
	}
	return (count);
}

#ifndef	SMALL
write(fdesc, buf, count)
	int fdesc, count;
	char *buf;
{
	register i;
	register struct iob *file;

	errno = 0;
	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		while (i--)
			putchar(*buf++);
		return (count);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs&F_WRITE) == 0) {
		errno = EBADF;
		return (-1);
	}
	file->i_cc = count;
	file->i_ma = buf;
	file->i_bn = file->i_boff + (file->i_offset / DEV_BSIZE);
	i = devwrite(file);
	file->i_offset += count;
	if (i < 0)
		errno = file->i_error;
	return (i);
}
#endif	SMALL

int	openfirst = 1;

open(str, how)
	char *str;
	int how;
{
	register char *cp;
	register int i;
	register struct iob *file;
	int fdesc;

	if (openfirst) {
		for (i = 0; i < NFILES; i++)
			iob[i].i_flgs = 0;
		openfirst = 0;
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (iob[fdesc].i_flgs == 0)
			goto gotfile;
	_stop("Out of slots");

gotfile:
	(file = &iob[fdesc])->i_flgs |= F_ALLOC;

	if ((cp = xx(str, file)) == (char *) -1)
		return -1;

	if (*cp == '\0') {
		file->i_flgs |= how+1;
		file->i_cc = 0;
		file->i_offset = 0;
		return (fdesc+3);
	}
	if (! (int) file->i_fs) {
		file->i_fs  = (struct fs *)calloc(SBSIZE);
		file->i_buf = calloc(MAXBSIZE);
	}
	file->i_ma = (char *)(file->i_fs);
	file->i_cc = SBSIZE;
	file->i_bn = SBLOCK + file->i_boff;
	file->i_offset = 0;
	if (devread(file) < 0) {
		errno = file->i_error;
		printf("bad super block\n");
		return (-1);
	}
#if	CMU && OLDFS
	file->i_fs->fs_oldfs = 0;
	if (file->i_fs->fs_magic != FS_MAGIC
	    ||
	    file->i_fs->fs_bsize > MAXBSIZE
	    ||
	    file->i_fs->fs_sbsize < SBSIZE)
	{
		file->i_fs->fs_bsize = BSIZE;
		file->i_fs->fs_bmask = ~(BSIZE-1);
		file->i_fs->fs_bshift = 10;	/* LOG2(BSIZE) */
		file->i_fs->fs_fsbtodb = 1;	/* LOG2(BSIZE/DEV_BSIZE) */
		file->i_fs->fs_nindir = BSIZE/sizeof(daddr_t);
		file->i_fs->fs_inopb = BSIZE/sizeof(struct oinode);
		file->i_fs->fs_oldfs++;
	}
#endif	CMU
	if ((i = find(cp, file)) == 0) {
		file->i_flgs = 0;
		errno = ESRCH;
		return (-1);
	}
#ifndef	SMALL
	if (how != 0) {
		printf("Can't write files yet.. Sorry\n");
		file->i_flgs = 0;
		errno = EIO;
		return (-1);
	}
#endif	SMALL
	if (openi(i, file) < 0) {
		errno = file->i_error;
		return (-1);
	}
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | (how+1);
	return (fdesc+3);
}

#define LP '('
#define RP ')'

char *openstr = (char *)0;
int unit_part_dev;

char *
xx(str, file)
char *str;
struct iob *file;
{
	register char *cp = str, *xp;
	register struct devsw *dp;
	register int dev = (unit_part_dev >> B_TYPESHIFT) & B_TYPEMASK;
	register int unit = (unit_part_dev >> B_UNITSHIFT) & B_UNITMASK;
	register int part = (unit_part_dev >> B_PARTITIONSHIFT) & B_PARTITIONMASK;
	int i;
	int no_dev;

	unit += 8 * ((unit_part_dev >> B_ADAPTORSHIFT) & B_ADAPTORMASK);

	for (; *cp && *cp != LP; cp++) ;
	if (no_dev = !*cp) {
		cp = str;
		xp = devname[dev];
	} else if (cp == str) {	/* no_dev.  use dev */
		cp++;
		xp = devname[dev];
	} else {
		xp = str;
		cp++;
	}
	for (dp = devsw; dp->dv_name; dp++) {
		if (xp[0] == dp->dv_name[0] && xp[1] == dp->dv_name[1]) {
			goto gotdev;
		}
	}
notdev:
	printf("Unknown device\n");
	file->i_flgs = 0;
	errno = ENXIO;
	return ((char *)-1);
gotdev:
	if (no_dev)
		goto none;
#ifndef	SMALL
	i = 0;
	while (*cp >= '0' && *cp <= '9') {
		unit = 0;
		i = i * 10 + *cp++ - '0';
	}
	unit = i | unit;	/* one or the other is zero */
	if (unit < 0 || unit > 63) {
		printf("Bad unit specifier\n");
		file->i_flgs = 0;
		errno = EUNIT;
		return ((char *)-1);
	}
	if (*cp == RP || no_dev)
		/* do nothing since ptol(")") returns 0 */ ;
	else if (*cp == ',' )
		part = ptol(++cp);
	else if (cp[-1] == LP) 
		part = ptol(cp);
	else {
badoff:
		printf("Missing offset specification\n");
		file->i_flgs = 0;
		errno = EOFFSET;
		return ((char *)-1);
	}

	for ( ;!no_dev ;) {
		if (*cp == RP)
			break;
		if (*cp++)
			continue;
		goto badoff;
	}
#endif	SMALL
none:
	if (openstr) {
		*openstr++ = xp[0];
		*openstr++ = xp[1];
		*openstr++ = LP;
		if (unit >= 10)
			*openstr++ = unit / 10 + '0';
		*openstr++ = unit % 10 + '0';
		*openstr++ = ',';
		*openstr++ = part % 10 + 'a';
		*openstr++ = RP;
	}
	file->i_ino.i_dev = dev = dp-devsw;
	file->i_unit = unit;
	file->i_boff = part;	/* mas grody */
	/*
	 * reset unit_part_dev
	 */
	unit_part_dev = 
		(dev << B_TYPESHIFT) |
		(unit << B_UNITSHIFT) |
		(part << B_PARTITIONSHIFT);

	devopen(file);
	if (file->i_error) 
		return (char *)-1;
	if (!no_dev && *cp) cp++;
	if (openstr)
		bcopy((caddr_t) cp, openstr, strlen(cp)+1);
	return cp;
}

close(fdesc)
	register int fdesc;
{
#ifndef	SMALL
	register struct iob *file;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs&F_FILE) == 0)
		devclose(file);
	file->i_flgs = 0;
	return (0);
#endif	SMALL

}

#ifndef	SMALL
ioctl(fdesc, cmd, arg)
	register int fdesc, cmd;
	char *arg;
{
	register struct iob *file;
	int error = 0;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	switch (cmd) {

	case SAIOHDR:
		file->i_flgs |= F_HDR;
		break;

	case SAIOCHECK:
		file->i_flgs |= F_CHECK;
		break;

	case SAIOHCHECK:
		file->i_flgs |= F_HCHECK;
		break;

	case SAIONOBAD:
		file->i_flgs |= F_NBSF;
		break;

	case SAIODOBAD:
		file->i_flgs &= ~F_NBSF;
		break;

	default:
		error = devioctl(file, cmd, arg);
		break;
	}
	if (error < 0)
		errno = file->i_error;
	return (error);
}
#endif	SMALL


/* Quick and dirty memory allocator.  Notice that memory is never freed. */

static char *calloc_base = (char *)0;

char *
calloc(size)
	register int size;
{
	register char *ret;
	extern unsigned int end;

	if (!calloc_base)
		calloc_base = (char *) (( ((int) &end) + NBPG-1 ) & ~(NBPG-1));

	ret = calloc_base;
#if	DEBUG
	if (debug) {
		printf("calloc: ret 0x%x\n", ret);
	}
#endif
	size =  (( size + NBPG-1 ) & ~(NBPG-1));

#ifndef	SMALL
	if (calloc_base + size >= (char*)_sp())
		_stop("calloc: stack clobbered");
	if (calloc_base + size + NBPG > (char *)_sp())
		printf("calloc: may clobber stack\n");
#endif	SMALL

	calloc_base += size;
	bzero(ret, size);
	return ret;
}

exit()
{
	_stop("Exit\n");
}

_stop(s)
	char *s;
{
	register int i;

#ifndef	SMALL
	for (i = 0; i < NFILES; i++)
		if (iob[i].i_flgs != 0)
			close(i);
#endif	SMALL
	printf("%s\n", s);
	sleep(4);
	halt();
}

#if	CMU
 
strlen(s)
	register char	*s;
{
	register n;

	n = 0;
	while (*s++)
		n++;
	return(n);
}

strcmp(s1, s2)
	register char	*s1, *s2;
{
	while (*s1 == *s2) {
		if (*s1++ == '\0')
			return(0);
		s2++;
	}
	return(*s1 - *s2);
}

strcpy(s1, s2)
	register char	*s1, *s2;
{
	while (*s1++ = *s2++)
		/* zip */;
}

ptol(str)
register char *str;
{
register int c;
	c = *str;
	if (c <= '7' && c >= '0')
		c -= '0';
	else if (c <= 'h' && c >= 'a')
		c -= 'a';
	return c;
}
#endif	CMU
