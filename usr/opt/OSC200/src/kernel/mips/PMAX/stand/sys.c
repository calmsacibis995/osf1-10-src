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
 * sys.c
 */

#if !defined(lint) && defined(SECONDARY)
static char	*sccsid = "@(#)$RCSfile: sys.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:54 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#include "../../../sys/param.h"
#include <sys/secdefines.h>
#if SEC_FSCHANGE
#include <sys/security.h>
#endif
#include "gnode_common.h"
#include "ufs_inode.h"
#include "gnode.h"

#include "dir.h"
#include <a.out.h>
#include "saio.h"

#ifdef vax
#include "vax/vmb.h"
#include "../vax/rpb.h"
#endif vax

#ifdef mips
#ifdef LABELS
#include <sys/disklabel.h>
#endif
#define printf _prom_printf
#define strlen prom_strlen
#define strcmp prom_strcmp
#endif mips
extern	int devpart;
#define RPAREN ')'
#define LPAREN '('

ino_t	dlook();

struct dirstuff {
	int loc;
	struct iob *io;
};

static
openi(n, io)
	register struct iob *io;
{
	register struct dinode *dp;
	int cc;

#ifdef DEBUG
printf("openi:\n");
#endif DEBUG
	io->i_offset = 0;
	io->i_bn = fsbtodb(&io->i_fs, itod(&io->i_fs, n)) + io->i_boff;
	io->i_cc = io->i_fs.fs_bsize;
	io->i_ma = io->i_buf;
	cc = devread(io);
	dp = (struct dinode *)io->i_buf;
#if SEC_FSCHANGE
	if (FsSEC(&io->i_fs)) {
		struct sec_dinode *sdp;
		sdp = ((struct sec_dinode *) io->i_buf) + itoo(&io->i_fs,n);
		G_TO_I(&io->i_ino)->di_ic = sdp->di_node.di_ic;
	} else
#endif
	G_TO_I(&io->i_ino)->di_ic = dp[itoo(&io->i_fs, n)].di_ic;
	return (cc);
}

static
find(path, file)
	register char *path;
	struct iob *file;
{
	register char *q;
	char c;
	int n;

#ifdef DEBUG
printf("find:\n");
#endif DEBUG
	if (path==NULL || *path=='\0') {
#ifdef DEBUG
		printf("null path\n");
#endif DEBUG
		return (0);
	}
	if (openi((gno_t) ROOTINO, file) < 0) {
#ifdef DEBUG
		printf("can't read root inode\n");
#endif DEBUG
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

		if ((n = dlook(path, file)) != 0) {
			if (c == '\0')
				break;
			if (openi(n, file) < 0)
				return (0);
			*q = c;
			path = q;
			continue;
		} else {
#ifdef DEBUG
			printf("%s not found\n", path);
#endif DEBUG
			return (0);
		}
	}
	return (n);
}

static daddr_t
sbmap(io, bn)
	register struct iob *io;
	daddr_t bn;
{
	register struct gnode *ip;
	int i, j, sh;
	daddr_t nb, *bap;

#ifdef DEBUG
printf("sbmap:\n");
#endif DEBUG
	ip = &io->i_ino;
	if (bn < 0) {
#ifdef DEBUG
		printf("bn negative\n");
#endif DEBUG
		return ((daddr_t)0);
	}

	/*
	 * blocks 0..NDADDR are direct blocks
	 */
	if(bn < NDADDR) {
		nb = G_TO_I(ip)->di_db[bn];
		return (nb);
	}

	/*
	 * addresses NIADDR have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	sh = 1;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(&io->i_fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0) {
#ifdef DEBUG
		printf("bn ovf %D\n", bn);
#endif DEBUG
		return ((daddr_t)0);
	}

	/*
	 * fetch the first indirect block address from the inode
	 */
	nb = G_TO_I(ip)->di_ib[NIADDR - j];
	if (nb == 0) {
#ifdef DEBUG
		printf("bn void %D\n",bn);
#endif DEBUG
		return ((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for (; j <= NIADDR; j++) {
		if (blknos[j] != nb) {
			io->i_bn = fsbtodb(&io->i_fs, nb) + io->i_boff;
			io->i_ma = b[j];
			io->i_cc = io->i_fs.fs_bsize;
			if (devread(io) != io->i_fs.fs_bsize) {
				if (io->i_error)
					errno = io->i_error;
#ifdef DEBUG
				printf("bn %D: read error\n", io->i_bn);
#endif DEBUG
				return ((daddr_t)0);
			}
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh /= NINDIR(&io->i_fs);
		i = (bn / sh) % NINDIR(&io->i_fs);
		nb = bap[i];
		if(nb == 0) {
#ifdef DEBUG
			printf("bn void %D\n",bn);
#endif DEBUG
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
	register struct gen_dir *dp;
	register struct gnode *ip;
	struct dirstuff dirp;
	int len;
	struct gen_dir *readdir();

#ifdef DEBUG
printf("dlook:\n");
#endif DEBUG
	if (s == NULL || *s == '\0')
		return (0);
	ip = &io->i_ino;
	if ((ip->g_mode&GFMT) != GFDIR) {
#ifdef DEBUG
		printf("not a directory\n");
#endif DEBUG
		return (0);
	}
	if (ip->g_size == 0) {
#ifdef DEBUG
		printf("zero length directory\n");
#endif DEBUG
		return (0);
	}
	len = strlen(s);
	dirp.loc = 0;
	dirp.io = io;
	for (dp = readdir(&dirp); dp != NULL; dp = readdir(&dirp)) {
		if(dp->d_ino == 0)
			continue;
#ifdef DEBUG
		printf("%s\n", dp->d_name);
#endif DEBUG
		if (dp->d_namlen == len && !strcmp(s, dp->d_name))
			return (dp->d_ino);
	}
	return (0);
}

/*
 * get next entry in a directory.
 */
struct gen_dir *readdir(dirp)
	register struct dirstuff *dirp;
{
	register struct gen_dir *dp;
	register struct iob *io;
	daddr_t lbn, d;
	int off;

#ifdef DEBUG
printf("readdir:\n");
#endif DEBUG
	io = dirp->io;
	for(;;) {
		if (dirp->loc >= io->i_ino.g_size)
			return (NULL);
		off = blkoff(&io->i_fs, dirp->loc);
		if (off == 0) {
			lbn = lblkno(&io->i_fs, dirp->loc);
			d = sbmap(io, lbn);
			if(d == 0)
				return NULL;
			io->i_bn = fsbtodb(&io->i_fs, d) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = blksize(&io->i_fs, &io->i_ino, lbn);
			if (devread(io) < 0) {
				errno = io->i_error;
#ifdef DEBUG
				printf("bn %D: read error\n", io->i_bn);
#endif DEBUG
				return (NULL);
			}
		}
		dp = (struct gen_dir *)(io->i_buf + off);
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

#ifdef DEBUG
printf("lseek:\n");
#endif DEBUG
	if (ptr != 0) {
#ifdef DEBUG
		printf("Seek not from beginning of file\n");
#endif DEBUG
		errno = EOFFSET;
		return (-1);
	}
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

getc(fdesc)
	int fdesc;
{
	register struct iob *io;
	register struct fs *fs;
	register char *p;
	int c, lbn, off, size, diff;

#ifdef DEBUG
printf("getc:\n");
#endif DEBUG
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	p = io->i_ma;
	if (io->i_cc <= 0) {
		if ((io->i_flgs & F_FILE) != 0) {
			diff = io->i_ino.g_size - io->i_offset;
			if (diff <= 0)
				return (-1);
			fs = &io->i_fs;
			lbn = lblkno(fs, io->i_offset);
			io->i_bn = fsbtodb(fs, sbmap(io, lbn)) + io->i_boff;
			off = blkoff(fs, io->i_offset);
			size = blksize(fs, &io->i_ino, lbn);
		} else {
			io->i_bn = io->i_offset / DEV_BSIZE;
			off = 0;
			size = DEV_BSIZE;
		}
		io->i_ma = io->i_buf;
		io->i_cc = size;
		if (devread(io) < 0) {
			errno = io->i_error;
			return (-1);
		}
		if ((io->i_flgs & F_FILE) != 0) {
			if (io->i_offset - off + size >= io->i_ino.g_size)
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
	register i;
	register struct iob *file;

#ifdef DEBUG
printf("read:\n");
#endif DEBUG
	errno = 0;
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
	if ((file->i_flgs & F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		file->i_bn = file->i_boff + (file->i_offset / DEV_BSIZE);
		i = devread(file);
		file->i_offset += count;
		if (i < 0)
			errno = file->i_error;
		return (i);
	} else {
		if (file->i_offset+count > file->i_ino.g_size)
			count = file->i_ino.g_size - file->i_offset;
		if ((i = count) <= 0)
			return (0);

		do {
			register cnt;

			cnt = (file->i_cc < i) ? file->i_cc : i;
			if (cnt > 0) {
				bcopy(file->i_ma, buf, cnt);
				file->i_cc -= cnt;
				file->i_offset += cnt;
				file->i_ma += cnt;
				i -= cnt;
				buf += cnt;
			} else {
				*buf++ = getc(fdesc+3);
				i--;
			}
		} while (i > 0);
		/*
		do {
			*buf++ = getc(fdesc+3);
		} while (--i);
		*/
		return (count);
	}
}

extern int oldunit;
int openfirst = 1;
int syntax_done = 0;

open(str, how)
	char *str;
	int how;
{
	register char *cp;
	int i;
	register struct iob *file;
	int tmp_boff, status;
	int fdesc, oldunit, newunit;
	long atol();
	extern	struct	vmb_info *vmbinfo;

#ifdef DEBUG
printf("open:\n");
#endif DEBUG
	if (openfirst) {
		for (i = 0; i < NFILES; i++)
			iob[i].i_flgs = 0;
		openfirst = 0;
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (iob[fdesc].i_flgs == 0)
			goto gotfile;
#ifdef DEBUG
#define _stop _prom_restart
	_stop("No more file slots");
#endif DEBUG
	return (-1);
gotfile:
	/*
	(file = &iob[fdesc])->i_flgs |= F_ALLOC;
	*/
	file = &iob[fdesc];

	file->i_boff = devpart;
#ifdef SECONDARY
	/*
	 * Valid input syntax:
	 *
	 *	vmunix		/ boot device, partition a
	 *	(g)vmunix	/ boot device, partition g
	 *	(3)vmunix	/ drive 3 (SAME CTLR) partition a
	 *	(9,g)vmunix	/ drive 9 (SAME CTLR), partition g
	 *
	 *	The old way, too.
	 *	(4,6)vmunix	/ drive 4 (SAME CTLR), partition g
	 */
#ifdef LABELS
	/* Actually, above comments are wrong for mips. */
	/* But with labels, they're almost right. The pathname is of
	 * the form:
	 * 	devtype(ctrlr,unit)(partition)file
	 * (previously, it was:
	 *	devtype(ctrlr,unit,partition)file )
	 * splitting the partition out prevents the ROMs from trying
	 * to interpret it as an Ultrix partition.
	 * We only interpret the bytes from the first closing
	 * parentheses to the end of string: i.e.
	 * (partition)file
	 * therefore, we can only read a file from the same device
	 * (controller & unit number) as we were booted from.
	 */
#endif /* LABELS */
#ifdef mips
	cp = str;
	if (*cp != LPAREN) {
		/* if we start with not open-paren, */
		while (*cp && (*cp++ != RPAREN));
		/* skip through to first close paren (if any) */
		/* if none, reset to original */
		if (!*cp) {
			cp = str;
		} else {
			/* we have been given a device identifier */
			if (devopen(str, 0) < 0) {
				printf("Cannot open channel to device %s\n",
					str);
				return -1;
			}
		}
		/* cp now points to either:
		 *    1 - the begining of the string
		 *    2 - the first character following the device specifier.
		 *	(i.e., the filename)
		 */
	}
	str = cp;
#ifdef LABELS
	/* Check for partition designator. */
	if (*cp == LPAREN) {
		cp++;
		if ((*cp >= '0') && (*cp <= '7')) {
			file->i_boff = *cp - '0';
		} else if ((*cp >= 'a') && (*cp <= 'h')) {
			file->i_boff = *cp - 'a';
		} else {
			file->i_boff = -1;
		}
		while (*cp && (*cp++ != RPAREN ));
		/* cp now points to the filename part (or EOS) */
		str = cp;
	}
	if (file->i_boff < 0) {
		printf("bad partition\n");
		return -1;
	}
#endif /* LABELS */
#endif /* mips */
#ifdef vax
	while ((*str == ' ') || (*str == '\t'))
		str++;
	cp = str;
	newunit = -1;
	if (*cp++ == '(') {
		if ((*cp >= '0') && (*cp <= '9')) {
			/*
			 * get the unit
			 */
			newunit = atoi(cp);
			while ((*cp >= '0') && (*cp <= '9'))
				cp++;
			if ((*cp != ',') && (*cp != ')'))
				goto syntax;
		}
		if (*cp != ')') {
			/*
	  	 	 * get the partition
	 	 	 */
			if (*cp == ',')
				cp++;
			if (*cp < 'a')
				file->i_boff = (*cp - '0');	/* old */
			else
				file->i_boff = (*cp - 'a');	/* new */
			if ((file->i_boff < 0) || (file->i_boff > 7)) {
				printf("bad partition specified\n");
				return (-1);
			}
			if (*++cp != ')')
				goto syntax;
		}
		cp++;
	} else {
		while (*cp) {
			if (*cp == '(')
				goto syntax;
			cp++;
		}
		cp = str;
	}
#endif /* vax */
	if (*cp == '\0') {
		printf("missing file name\n");
		return -1;
	}
#ifdef vax
	if (newunit != -1) {
		oldunit = ((struct rpb *)(vmbinfo->rpbbas))->unit;
		if (oldunit != newunit) {
			status = drvinit(newunit);
			if ((status & 1) == 0) {
				printf("Init error: %s\n", geterr(status));
				drvinit(oldunit);	/* switch back */
				return (-1);
			}
		}
	}
#endif vax
#ifndef LABELS
	tmp_boff = file->i_boff;
	file->i_boff = 0;
	file->i_ma = (char *)(&file->i_fs);
	file->i_cc = SBSIZE;
	file->i_bn = SBLOCK + file->i_boff;
	file->i_offset = 0;
	if (devread(file) >= 0) {
		if (ptinfo(file,tmp_boff) < 0) {
			printf ("bad partition table\n");
			if ((file->i_boff = tmp_boff) != 0)
				return -1;
		}
	} else {
		errno = file->i_error;
		return -1;		/* read of SBLOCK failed */
	}
#endif
	str = cp;
#endif /* SECONDARY */
#ifdef LABELS
	tmp_boff = file->i_boff;
	file->i_boff = 0;
	file->i_ma = file->i_buf;
	file->i_cc = DEV_BSIZE;
	file->i_bn = LABELSECTOR;
	file->i_offset = 0;
	if (devread(file) >= 0) {
		if (labelinfo(file,tmp_boff) < 0) {
			printf("No disk label\n");
			if ((file->i_boff = tmp_boff) != 0) {
				return (-1);
			}
		}
	} else {
		errno = file->i_error;
		return (-1);		/* read of LABELSECTOR failed */
	}
#endif
	file->i_ma = (char *)(&file->i_fs);
	file->i_cc = SBSIZE;
	file->i_bn = SBLOCK + file->i_boff;
	file->i_offset = 0;
	if (devread(file) < 0) {
		errno = file->i_error;
#ifdef DEBUG
		printf("super block read error\n");
#endif DEBUG
		return (-1);
	}
	if ((i = find(str, file)) == 0) {
		errno = ESRCH;
		return (-1);
	}
	if (openi(i, file) < 0) {
		errno = file->i_error;
		return (-1);
	}
 	if (how != 0) {
#ifdef DEBUG
		printf("Can't write files yet.. Sorry\n");
#endif DEBUG
 		errno = EIO;
 		return (-1);
 	}
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | F_ALLOC | (how+1);
	return (fdesc+3);
#ifdef SECONDARY
syntax:
	printf("Syntax error\n");
	if (!syntax_done) {
		printf("\
\n    Examples of valid input syntax are:\n\
\n\
    newvmunix     - load newvmunix from the booted device, partition a\n\
    (g)vmunix     - load vmunix from the booted device, partition g\n\
    (3)vmunix.old - load vmunix.old from device unit 3, partition a\n\
    (9,g)vmunix   - load vmunix from device unit 9, partition g\n\
    (4,7)vmunix   - load vmunix from device unit 4, partition h\n\
\n\
    NOTE: If specified, the device unit number must be the PHYSICAL\n\
    unit number of a device connected to the SAME CONTROLLER as the\n\
    booted device.\n\n");
		syntax_done=1;
	}
	return(-1);
#endif SECONDARY
}

#ifdef SECONDARY
close(fdesc)
	int fdesc;
{
	struct iob *file;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	file->i_flgs = 0;
	return (0);
}

#ifndef LABELS
ptinfo(file, boff)
	register struct iob *file;
	int boff;
{
	struct pt *pt;

	/*
	 *	Check to see if the block that was read in is a superblock
	 */
#if SEC_FSCHANGE
	if ( file->i_fs.fs_magic == FS_MAGIC || FsSEC(&file->i_fs))
#else
	if ( file->i_fs.fs_magic == FS_MAGIC )
#endif
	{
		/*
		 *	Get the possible partition table
		 */
		pt= (struct pt *)&file->i_un.dummy[SBSIZE - sizeof(struct pt)];
		/*
		 *	Id the a real partition table
		 */
		if ( pt->pt_magic == PT_MAGIC ) {

			/*
			 *	We have a real partition table so now
			 *	set the driver's part tbl
			 */
			file->i_boff = pt->pt_part[boff].pi_blkoff;
			return(0);
		}
		else
			return(-1);
	}
	else
		return(-1);
}
#endif /* LABELS */
#endif /* SECONDARY */

#ifdef LABELS
labelinfo(file,partno)
	register struct iob *file;
	register int partno;
{
	register struct disklabel *dlp, *lp;

	lp = (struct disklabel *)file->i_buf;
	for (dlp = lp; dlp <= (lp + DEV_BSIZE - sizeof(*dlp));
		dlp = (struct disklabel *)((char *)dlp + sizeof(long))) {
		if ((dlp->d_magic == DISKMAGIC)
			&& (dlp->d_magic2 == DISKMAGIC)
			&& (dkcksum(dlp) == 0)) {
			if ((dlp->d_npartitions > partno)
				&& (dlp->d_partitions[partno].p_size != 0)) {
				file->i_boff =
					dlp->d_partitions[partno].p_offset;
				return(0);
			} else {
				printf("partition '%c' not found: using 'a'\n",
					partno + 'a');
				file->i_boff = 0;
				return(0);
			}
		}
	}
	return (-1);
}

/*
 * Compute checksum for disk label.
 */
dkcksum(lp)
	register struct disklabel *lp;
{
	register u_short *start, *end;
	register u_short sum = 0;

	start = (u_short *)lp;
	end = (u_short *)&lp->d_partitions[lp->d_npartitions];
	while (start < end)
		sum ^= *start++;
	return (sum);
}
#endif
