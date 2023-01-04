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
static char	*sccsid = "@(#)$RCSfile: tunefs.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1994/01/12 00:06:07 $";
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
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * tunefs: change layout parameters to an existing file system.
 */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif
#include <sys/param.h>
#include <sys/stat.h>
#include <ufs/fs.h>
#include <fstab.h>
#include <stdio.h>
#include <paths.h>


union {
	struct	fs sb;
	char pad[MAXBSIZE];
} sbun;
#define	sblock sbun.sb

int fi;
int dev_bsize = 1;
char *rindex(), *rawname();

main(argc, argv)
	int argc;
	char *argv[];
{
	char *cp, *special, *name;
	struct stat st;
	int i;
	int Aflag = 0;
	struct fstab *fs;
	char *chg[2], device[MAXPATHLEN];
	char rawdevice[MAXPATHLEN];

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, "%s: need sysadmin authorization\n",
			command_name);
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
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
#endif
	argc--, argv++; 
	if (argc < 2)
		goto usage;
	special = argv[argc - 1];
	fs = getfsfile(special);
	if (fs)
		special = rawname(fs->fs_spec);
again:
	if (stat(special, &st) < 0) {
		if (*special != '/') {
			if (*special == 'r')
				special++;
			(void)sprintf(device, "%s/%s", _PATH_DEV, special);
			special = device;
			goto again;
		}
		fprintf(stderr, "tunefs: "); perror(special);
		exit(1);
	}
	if ((st.st_mode & S_IFMT) != S_IFBLK &&
	    (st.st_mode & S_IFMT) != S_IFCHR)
		fatal("%s: not a block or character device", special);
	getsb(&sblock, special);
	for (; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
		for (cp = &argv[0][1]; *cp; cp++)
			switch (*cp) {

			case 'A':
				Aflag++;
				continue;

			case 'a':
				name = "maximum contiguous block count";
				if (argc < 1)
					fatal("-a: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 1)
					fatal("%s: %s must be >= 1",
						*argv, name);
				fprintf(stdout, "%s changes from %d to %d\n",
					name, sblock.fs_maxcontig, i);
				sblock.fs_maxcontig = i;
				continue;

			case 'd':
				name =
				   "rotational delay between contiguous blocks";
				if (argc < 1)
					fatal("-d: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				fprintf(stdout,
					"%s changes from %dms to %dms\n",
					name, sblock.fs_rotdelay, i);
				sblock.fs_rotdelay = i;
				continue;

			case 'e':
				name =
				  "maximum blocks per file in a cylinder group";
				if (argc < 1)
					fatal("-e: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 1)
					fatal("%s: %s must be >= 1",
						*argv, name);
				fprintf(stdout, "%s changes from %d to %d\n",
					name, sblock.fs_maxbpg, i);
				sblock.fs_maxbpg = i;
				continue;

			case 'm':
				name = "minimum percentage of free space";
				if (argc < 1)
					fatal("-m: missing %s", name);
				argc--, argv++;
				i = atoi(*argv);
				if (i < 0 || i > 99)
					fatal("%s: bad %s", *argv, name);
				fprintf(stdout,
					"%s changes from %d%% to %d%%\n",
					name, sblock.fs_minfree, i);
				sblock.fs_minfree = i;
				if (i >= 10 && sblock.fs_optim == FS_OPTSPACE)
					fprintf(stdout, "should optimize %s",
					    "for time with minfree >= 10%\n");
				if (i < 10 && sblock.fs_optim == FS_OPTTIME)
					fprintf(stdout, "should optimize %s",
					    "for space with minfree < 10%\n");
				continue;

			case 'o':
				name = "optimization preference";
				if (argc < 1)
					fatal("-o: missing %s", name);
				argc--, argv++;
				chg[FS_OPTSPACE] = "space";
				chg[FS_OPTTIME] = "time";
				if (strcmp(*argv, chg[FS_OPTSPACE]) == 0)
					i = FS_OPTSPACE;
				else if (strcmp(*argv, chg[FS_OPTTIME]) == 0)
					i = FS_OPTTIME;
				else
					fatal("%s: bad %s (options are `space' or `time')",
						*argv, name);
				if (sblock.fs_optim == i) {
					fprintf(stdout,
						"%s remains unchanged as %s\n",
						name, chg[i]);
					continue;
				}
				fprintf(stdout,
					"%s changes from %s to %s\n",
					name, chg[sblock.fs_optim], chg[i]);
				sblock.fs_optim = i;
				if (sblock.fs_minfree >= 10 && i == FS_OPTSPACE)
					fprintf(stdout, "should optimize %s",
					    "for time with minfree >= 10%\n");
				if (sblock.fs_minfree < 10 && i == FS_OPTTIME)
					fprintf(stdout, "should optimize %s",
					    "for space with minfree < 10%\n");
				continue;

			default:
				fatal("-%c: unknown flag", *cp);
			}
	}
	if (argc != 1)
		goto usage;
	bwrite(SBOFF / dev_bsize, (char *)&sblock, SBSIZE);
	if (Aflag)
		for (i = 0; i < sblock.fs_ncg; i++)
			bwrite(fsbtodb(&sblock, cgsblock(&sblock, i)),
			    (char *)&sblock, SBSIZE);
	close(fi);
	exit(0);
usage:
	fprintf(stderr, "Usage: tunefs tuneup-options special-device\n");
	fprintf(stderr, "where tuneup-options are:\n");
	fprintf(stderr, "\t-a maximum contiguous blocks\n");
	fprintf(stderr, "\t-d rotational delay between contiguous blocks\n");
	fprintf(stderr, "\t-e maximum blocks per file in a cylinder group\n");
	fprintf(stderr, "\t-m minimum percentage of free space\n");
	fprintf(stderr, "\t-o optimization preference (`space' or `time')\n");
	exit(2);
}

getsb(fs, file)
	register struct fs *fs;
	char *file;
{

	fi = open(file, 2);
	if (fi < 0) {
		fprintf(stderr, "cannot open");
		perror(file);
		exit(3);
	}
	if (bread(SBOFF, (char *)fs, SBSIZE)) {
		fprintf(stderr, "bad super block");
		perror(file);
		exit(4);
	}
#if SEC_FSCHANGE
	if (fs->fs_magic != FS_MAGIC && fs->fs_magic != FS_SEC_MAGIC)
#else
	if (fs->fs_magic != FS_MAGIC)
#endif
	{
		fprintf(stderr, "%s: bad magic number\n", file);
		exit(5);
	}
	dev_bsize = fs->fs_fsize / fsbtodb(fs, 1);
}

bwrite(blk, buf, size)
	char *buf;
	daddr_t blk;
	register size;
{
	if (lseek(fi, ((off_t)blk * dev_bsize), 0) < 0) {
		perror("FS SEEK");
		exit(6);
	}
	if (write(fi, buf, size) != size) {
		perror("FS WRITE");
		exit(7);
	}
}

bread(bno, buf, cnt)
	daddr_t bno;
	char *buf;
{
	register i;

	if (lseek(fi, ((off_t)bno * dev_bsize), 0) < 0)
		return(1);
	if ((i = read(fi, buf, cnt)) != cnt) {
		for(i=0; i<sblock.fs_bsize; i++)
			buf[i] = 0;
		return (1);
	}
	return (0);
}

/* VARARGS1 */
fatal(fmt, arg1, arg2)
	char *fmt, *arg1, *arg2;
{

	fprintf(stderr, "tunefs: ");
	fprintf(stderr, fmt, arg1, arg2);
	putc('\n', stderr);
	exit(10);
}

#define	LSM_CDEV	"rvol"
#define	LSM_CDEV_LEN	4
#define	LSM_BDEV	"vol"
#define	LSM_BDEV_LEN	3

char *
unrawname(name)
	char *name;
{
	char *dp;
	char *ddp;
	char *p;
	struct stat stb;

	if (stat(name, &stb) < 0)
		return (name);
	if ((stb.st_mode & S_IFMT) != S_IFCHR)
		return (name);

	for (;;)	/* just so we can use break */
	{
		/* see if any '/' */
		if ((dp = rindex(name, '/')) == 0)
		{
			dp = name-1;	/* simple name */
			break;
		}

		/* look for a second slash */
		p = dp-1;
		ddp = 0;
		while (p >= name)
		{
			if (*p == '/')
			{
				ddp = p;
				break;
			}
			p--;
		}
		if (ddp)
		{
			/* look for "rvol" */
			p = ddp - LSM_CDEV_LEN;
			/* is place we are looking at valid ? */
			if (p == name || (p > name && p[-1] == '/'))
			{
				/* actually look for string */
				if (strncmp(p, LSM_CDEV, LSM_CDEV_LEN) == 0)
				{
					dp = p-1; /* name preceeded by rvol/xxxx/ */
					break;
				}
			}
		}

		/* look for "rvol" */
		p = dp - LSM_CDEV_LEN;
		/* is place we are looking at valid ? */
		if (p == name || (p > name && p[-1] == '/'))
		{
			/* actually look for string */
			if (strncmp(p, LSM_CDEV, LSM_CDEV_LEN) == 0)
			{

				dp = p-1; /* name preceeded by rvol/ */
				break;
			}
		}

		break;
	}
	
	if (*(dp + 1) != 'r')
		return (name);
	(void)strcpy(dp + 1, dp + 2);
	return (name);
}

char *
rawname(name)
	char *name;
{
	static char rawbuf[MAXPATHLEN];
	char *dp;
	char *ddp;
	char *p;

	for (;;)	/* just so we can use break */
	{
		/* see if any '/' */
		if ((dp = (char *)rindex(name, '/')) == 0)
		{
			dp = name-1;	/* a simple name */
			break;
		}

		/* look for a second '/' */
		p = dp-1;
		ddp = 0;
		while (p >= name)
		{
			if (*p == '/')
			{
				ddp = p;
				break;
			}
			p--;
		}
		if (ddp)
		{
			/* look for "vol" */
			p = ddp - LSM_BDEV_LEN;
			/* is place we are looking at valid ? */
			if (p == name || (p > name && p[-1] == '/'))
			{
				/* actually look for string */
				if (strncmp(p, LSM_BDEV, LSM_BDEV_LEN) == 0)
				{
					dp = p-1; /* name preceeded by vol/xxxx/ */
					break;
				}
			}
		}

		/* look for "vol" */
		p = dp - LSM_BDEV_LEN;
		/* is place we are looking at valid ? */
		if (p == name || (p > name && p[-1] == '/'))
		{
			/* actually look for string */
			if (strncmp(p, LSM_BDEV, LSM_BDEV_LEN) == 0)
			{
				dp = p-1; /* a name preceeded by vol/ */
				break;
			}
		}

		break;
	}
	
	dp++;
	memcpy(rawbuf, name, dp - name);
	strcpy(rawbuf + (dp-name), "r");
	strcat(rawbuf, dp);

	return (rawbuf);
}

