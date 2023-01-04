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
static char	*sccsid = "@(#)$RCSfile: preen.c,v $ $Revision: 4.3.13.3 $ (DEC) $Date: 1994/01/10 18:37:20 $";
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

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <fstab.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>

char	*rawname(), *unrawname(), *blockcheck();
void	addpart();

struct part {
	struct	part *next;		/* forward link of partitions on disk */
	char	*name;			/* device name */
	char	*fsname;		/* mounted filesystem name */
	caddr_t	auxdata;		/* auxillary data for application */
} *badlist, **badnext = &badlist;

struct disk {
	char	*name;			/* disk base name */
	struct	disk *next;		/* forward link for list of disks */
	struct	part *part;		/* head of list of partitions on disk */
	int	pid;			/* If != 0, pid of proc working on */
} *disks;

int	nrun, ndisks;
char	hotroot;

checkfstab(preen, maxrun, docheck, chkit)
	int preen, maxrun;
	caddr_t (*docheck)();
	int (*chkit)();
{
	register struct fstab *fsp;
	register struct disk *dk, *nextdisk;
	register struct part *pt;
	int ret, pid, retcode, passno, sumstatus, status;
	caddr_t auxdata;
	char *name;

#ifdef DEBUG
	printf ("Entering checkfstab\n");
#endif
	sumstatus = 0;
	for (passno = 1; passno <= 2; passno++) {
		if (setfsent() == 0) {
			fprintf(stderr, "Can't open checklist file: %s\n",
			    _PATH_FSTAB);
			return (8);
		}
		while ((fsp = getfsent()) != 0) {
			if ((auxdata = (*docheck)(fsp)) == 0)
				continue;
			if (preen == 0 || passno == 1 && fsp->fs_passno == 1) {
				name = blockcheck(fsp->fs_spec, &ret);
				if (name) {
					if (preen &&
					    strcmp(fsp->fs_type, FSTAB_RO) == 0
					    && check_readonly(name)) {
						continue;	/* skip cdrom */
					}
					if (sumstatus = (*chkit)(name, 
						fsp->fs_file, auxdata, 0))
						return (sumstatus);
				} else if (preen) {
					if (ret == ENODEV)
						continue;
					return (8);
				}
			} else if (passno == 2 && fsp->fs_passno > 1) {
				if ((name = blockcheck(fsp->fs_spec, &ret)) == NULL) {
					fprintf(stderr, "BAD DISK NAME %s\n",
						fsp->fs_spec);
					if (ret == ENODEV)
						continue;
					sumstatus |= 8;
					continue;
				}
				if (preen &&
				    strcmp(fsp->fs_type, FSTAB_RO) == 0 &&
				    check_readonly(name)) {
					continue;	/* skip cdrom */
				}
				addpart(name, fsp->fs_file, auxdata);
			}
		}
		if (preen == 0)
			return (0);
	}
	if (preen) {
		if (maxrun == 0)
			maxrun = ndisks;
		if (maxrun > ndisks)
			maxrun = ndisks;
		nextdisk = disks;
		for (passno = 0; passno < maxrun; ++passno) {
			while (ret = startdisk(nextdisk, chkit) && nrun > 0)
				sleep(10);
			if (ret)
				return (ret);
			nextdisk = nextdisk->next;
		}
		while ((pid = wait(&status)) != -1) {
			for (dk = disks; dk; dk = dk->next)
				if (dk->pid == pid)
					break;
			if (dk == 0) {
				printf("Unknown pid %d\n", pid);
				continue;
			}
			if (WIFEXITED(status))
				retcode = WEXITSTATUS(status);
			else
				retcode = 0;
			if (WIFSIGNALED(status)) {
				printf("%s (%s): EXITED WITH SIGNAL %d\n",
					dk->part->name, dk->part->fsname,
					WTERMSIG(status));
				retcode = 8;
			}
			if (retcode != 0) {
				sumstatus |= retcode;
				*badnext = dk->part;
				badnext = &dk->part->next;
				dk->part = dk->part->next;
				*badnext = NULL;
			} else
				dk->part = dk->part->next;
			dk->pid = 0;
			nrun--;
			if (dk->part == NULL)
				ndisks--;

			if (nextdisk == NULL) {
				if (dk->part) {
					while (ret = startdisk(dk, chkit) &&
					    nrun > 0)
						sleep(10);
					if (ret)
						return (ret);
				}
			} else if (nrun < maxrun && nrun < ndisks) {
				for ( ;; ) {
					if ((nextdisk = nextdisk->next) == NULL)
						nextdisk = disks;
					if (nextdisk->part != NULL &&
					    nextdisk->pid == 0)
						break;
				}
				while (ret = startdisk(nextdisk, chkit) &&
				    nrun > 0)
					sleep(10);
				if (ret)
					return (ret);
			}
		}
	}
	if (sumstatus) {
		if (badlist == 0)
			return (sumstatus);
		fprintf(stderr, "THE FOLLOWING FILE SYSTEM%s HAD AN %s\n\t",
			badlist->next ? "S" : "", "UNEXPECTED INCONSISTENCY:");
		for (pt = badlist; pt; pt = pt->next)
			fprintf(stderr, "%s (%s)%s", pt->name, pt->fsname,
			    pt->next ? ", " : "\n");
		return (sumstatus);
	}
	(void)endfsent();
	return (0);
}

struct disk *
finddisk(name)
	char *name;
{
	register struct disk *dk, **dkp;
	register char *p;
	size_t len;

	for (p = name + strlen(name) - 1; p >= name; --p)
		if (isdigit(*p)) {
			len = p - name + 1;
			break;
		}
	if (p < name)
		len = strlen(name);

	for (dk = disks, dkp = &disks; dk; dkp = &dk->next, dk = dk->next) {
		if (strncmp(dk->name, name, len) == 0 &&
		    dk->name[len] == 0)
			return (dk);
	}
	if ((*dkp = (struct disk *)malloc(sizeof(struct disk))) == NULL) {
		fprintf(stderr, "out of memory");
		exit (8);
	}
	dk = *dkp;
	if ((dk->name = malloc(len + 1)) == NULL) {
		fprintf(stderr, "out of memory");
		exit (8);
	}
	(void)strncpy(dk->name, name, len);
	dk->name[len] = '\0';
	dk->part = NULL;
	dk->next = NULL;
	dk->pid = 0;
	ndisks++;
	return (dk);
}

void
addpart(name, fsname, auxdata)
	char *name, *fsname;
	caddr_t auxdata;
{
	struct disk *dk = finddisk(name);
	register struct part *pt, **ppt = &dk->part;

	for (pt = dk->part; pt; ppt = &pt->next, pt = pt->next)
		if (strcmp(pt->name, name) == 0) {
			printf("%s in fstab more than once!\n", name);
			return;
		}
	if ((*ppt = (struct part *)malloc(sizeof(struct part))) == NULL) {
		fprintf(stderr, "out of memory");
		exit (8);
	}
	pt = *ppt;
	if ((pt->name = malloc(strlen(name) + 1)) == NULL) {
		fprintf(stderr, "out of memory");
		exit (8);
	}
	(void)strcpy(pt->name, name);
	if ((pt->fsname = malloc(strlen(fsname) + 1)) == NULL) {
		fprintf(stderr, "out of memory");
		exit (8);
	}
	(void)strcpy(pt->fsname, fsname);
	pt->next = NULL;
	pt->auxdata = auxdata;
}

startdisk(dk, checkit)
	register struct disk *dk;
	int (*checkit)();
{
	register struct part *pt = dk->part;

	dk->pid = fork();
	if (dk->pid < 0) {
		perror("fork");
		return (8);
	}
	if (dk->pid == 0)
		exit((*checkit)(pt->name, pt->fsname, pt->auxdata, 1));
	nrun++;
	return (0);
}

char *
blockcheck(name, ret)
	char *name;
	int  *ret;
{
	struct stat stslash, stblock, stchar;
	char *raw;
	int retried = 0;

	*ret = hotroot = 0;
	if (stat("/", &stslash) < 0) {
		perror("/");
		printf("Can't stat root\n");
		return (0);
	}
retry:
	if (stat(name, &stblock) < 0) {
		*ret = errno;
		perror(name);
		printf("Can't stat %s\n", name);
		/*
		 * return ENODEV to note that we couldn't
		 * stat our device;  in preen mode, we (now)
		 * ignore failed devices so that we don't fail
		 * bootup.
		 */
		*ret = ENODEV;
		return (0);
	}
	if ((stblock.st_mode & S_IFMT) == S_IFBLK) {
		if (stslash.st_dev == stblock.st_rdev)
			hotroot++;
		raw = rawname(name);
		if (stat(raw, &stchar) < 0) {
			*ret = errno;
			perror(raw);
			printf("Can't stat %s\n", raw);
			/*
			 * see ENODEV comment above.
			 */
			*ret = ENODEV;
			return (name);
		}
		if ((stchar.st_mode & S_IFMT) == S_IFCHR) {
			return (raw);
		} else {
			printf("%s is not a character device\n", raw);
			return (name);
		}
	} else if ((stblock.st_mode & S_IFMT) == S_IFCHR && !retried) {
		name = unrawname(name);
		retried++;
		goto retry;
	}
	printf("Can't make sense out of name %s\n", name);
	return (0);
}

/* check_readonly()
 * 	check to see if device is readonly (i.e. cdrom),
 *	in which case we return TRUE.
 */
int
check_readonly(char *name)
{
	int fd;
	struct devget dg;

	if ((fd = open(name, O_RDONLY, (0))) < 0)
		return(0);
	if (ioctl(fd, DEVIOCGET, &dg) < 0) {
		close(fd);
		return(0);
	}
	close(fd);
	if (dg.stat & DEV_RDONLY)
		return(1);
	return(0);
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

