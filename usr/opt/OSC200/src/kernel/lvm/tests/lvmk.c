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
static char	*sccsid = "@(#)$RCSfile: lvmk.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:22:34 $";
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <fcntl.h>

#include <lvm/lvm.h>

extern char *malloc();
int otoi();

int mirror = 0;		/* Allocate a mirror?  Default = NO */
int mircons = 1;	/* Enable mirror consistency? Default = YES */
int size = 15;
extern char *progname;
int offset = 0;

int
main(argc, argv)
int argc;
char **argv;
{
extern int optind;
extern char *optarg;
int c, errflag = 0;

/* Establish defaults */
char *vgname = NULL;
int perm = 0777;
struct stat statbuf;
int minor_num;
int fd;

	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	while ((c = getopt(argc, argv, "mnp:s:o:")) != EOF) {
		switch (c) {
			case 'p': perm = otoi(optarg); break;
			case 'm': mirror++; break;
			case 'n': mircons = 0; break;
			case 's': size = atoi(optarg); break;
			case 'o': offset = atoi(optarg); break;
			default: errflag++;
				break;
		}
	}
	umask(0);

	if (optind < argc) {
		/* Do the volume group thing */
		vgname = argv[optind];
		if ((fd = open(vgname, O_RDWR)) < 0) {
			return(report_error(stderr, "open"));
		}
		/* Discover the major/minor numbers for the
		   volume group device */
		if (fstat(fd, &statbuf) < 0) {
			return(report_error(stderr, "fstat"));
		}
		if (minor(statbuf.st_rdev) != 0) {
			fprintf(stderr, "%s: specified device (%s)",
				progname, vgname);
			fprintf(stderr, " is not minor device #0\n");
		}
		optind++;
		for (; optind < argc; optind++) {
			if ((minor_num = create_lv_nodes(argv[optind],
				major(statbuf.st_rdev), perm)) < 0) {
				fprintf(stderr,"create_lv_nodes failed.\n");
				return(-minor_num);
			}
			create_lv(fd, minor_num);
		}
	}
	return(0);
}

int
otoi(s)
char *s;
{
int result;
int c, digit;

        result = 0;
        while (c = *s++) {
                digit = c - '0';
                result = result * 010 + digit;
        }
        return (result);
}

int
create_lv_nodes(path, maj_no, perm)
char *path;
int maj_no;
int perm;
{
struct stat statbuf;
char tmppath[1024];
char *dpath;
char *simplename;
dev_t devno;
int type;

	if (simplename = rindex(path, '/')) simplename++;
	else simplename = path;

	/* Deduce the minor number from the pathname. */
	dpath = simplename + strlen(simplename) - 1;
	for (; dpath >= simplename; dpath--) {
		if (!isdigit(*dpath)) break;
	}
	dpath++;
	if (*dpath != NULL) {
		devno = makedev(maj_no, atoi(dpath));
	} else {
		fprintf(stderr, "logical volume pathname must have trailing digits: %s\n", path);
		return (EINVAL);
	}

	/* First check for character device. */
	if (simplename[0] != 'r') {
		/* we've been given the block device name, squeeze	
		 * in an 'r' to make it the raw device name. */
		strcpy(tmppath, path);
		*(tmppath+(simplename-path)) = 'r';
		strcpy(tmppath+(simplename-path)+1, simplename);
		dpath = tmppath;
	} else {
		dpath = path;
	}

	if (stat(dpath, &statbuf) == 0) {
		/* something exists with this name already */
		type = statbuf.st_mode&S_IFMT;
		if (type == S_IFCHR) {
			printf("%s exists: %s special, major %d, minor %d.\n",
				dpath, type==S_IFCHR?"character":"block",
				major(statbuf.st_rdev),
				minor(statbuf.st_rdev));
			if (major(statbuf.st_rdev) != maj_no) {
				printf("%s is not correct device.\n", dpath);
				return(EEXIST);
			}
		} else {
			printf("%s is not a character special file.\n", dpath);
			return(ENODEV);
		}
	} else {
		if (geteuid()) {
			printf("Must be superuser to create device node.\n");
			return(EPERM);
		}
		if (mknod(dpath, 020000|perm, devno) < 0) {
			return(report_error(stderr,"mknod"));
		}
	}

	/* Next check on the block special file */
	if (simplename[0] != 'r') {
		dpath = path;
	} else {
		/* we've been given the character device name, squeeze	
		 * out the 'r' to make it the block device name. */
		strcpy(tmppath, path);
		strcpy(tmppath+(simplename-path)-1, simplename);
		dpath = tmppath;
	}
	
	if (stat(dpath, &statbuf) == 0) {
		/* something exists with this name already */
		type = statbuf.st_mode&S_IFMT;
		if (type == S_IFBLK) {
			printf("%s exists: %s special, major %d, minor %d.\n",
				dpath, type==S_IFCHR?"character":"block",
				major(statbuf.st_rdev),
				minor(statbuf.st_rdev));
			if (major(statbuf.st_rdev) != maj_no) {
				printf("%s is not correct device.\n", dpath);
				return(EEXIST);
			}
		} else {
			printf("%s is not a block special file.\n", dpath);
			return(ENODEV);
		}
	} else {
		if (geteuid()) {
			printf("Must be superuser to create device node.\n");
			return(EPERM);
		}
		if (mknod(dpath, 060000|perm, devno) < 0) {
			return(report_error(stderr,"mknod"));
		}
	}
	return(minor(devno));
}

create_lv(vgfd, minor_num)
int vgfd, minor_num;
{
struct lv_querylv qlv;
struct lv_statuslv slv;
struct lv_lvsize lvs;
int extents, lx;
int i, m;

	qlv.minor_num = minor_num;
	if (ioctl(vgfd, LVM_QUERYLV, &qlv) < 0) {
		return(report_error(stderr, "LVM_QUERYLV"));
	}

	slv.minor_num = minor_num;
	if (!(qlv.lv_flags & LVM_LVDEFINED)) {
		slv.maxlxs = size;
		slv.lv_flags = 0;
		if (!mircons)
			slv.lv_flags |= LVM_NOMWC;
		slv.sched_strat = LVM_PARALLEL;
		slv.maxmirrors = mirror;
		if (ioctl(vgfd, LVM_CREATELV, &slv) < 0) {
			report_error(stderr, "LVM_CREATELV");
			return (-1);	
		} else
			printf("Created logical volume %d.\n", minor_num);
		slv.lv_flags |= LVM_LVDEFINED;
 		if (check_lv(vgfd, &slv))
			return(-1);
		lvs.minor_num = minor_num;
		extents = size * (mirror + 1);

		lvs.size = extents;
		lvs.extents = (lxmap_t *)malloc(sizeof(lxmap_t)*extents);

		i = 0;
		for (lx = 0; lx < size; lx++) {
			for (m = 0; m < (mirror+1); m++) {
				lvs.extents[i].lx_num = lx;
				lvs.extents[i].pv_key = m;
				lvs.extents[i].px_num = lx + offset;
				lvs.extents[i].status = 0;
				i++;
			}
		}

		if (ioctl(vgfd, LVM_EXTENDLV, &lvs) < 0) {
			return(report_error(stderr,"extendlv"));
		}
	}
	return(0);
}

check_lv(fd, slv)
int fd;
struct lv_statuslv *slv;
{
struct lv_querylv qlv;

	qlv.minor_num = slv->minor_num;
	if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
		return(report_error(stderr, "LVM_QUERYLV"));
	}
	if ((qlv.minor_num != slv->minor_num)
		|| (qlv.maxlxs != slv->maxlxs)
		|| (qlv.lv_flags != slv->lv_flags)
		|| (qlv.sched_strat != slv->sched_strat)
		|| (qlv.maxmirrors != slv->maxmirrors)) {
		printf("Query results do not match expected:\n");
		print_qlv(&qlv);
		return (-1);
	}
	return (0);
}

#if __STDC__
#define print_member(S,M) printf("\t" #M ": 0x%08x\n", S->M)
#else
#define print_member(S,M) printf("\t%s: 0x%08x\n", "M", S->M)
#endif

print_qlv(qlv)
struct lv_querylv *qlv;
{
	print_member(qlv,minor_num);
	print_member(qlv,numpxs);
	print_member(qlv,numlxs);
	print_member(qlv,maxlxs);
	print_member(qlv,lv_flags);
	print_member(qlv,sched_strat);
	print_member(qlv,maxmirrors);
}
