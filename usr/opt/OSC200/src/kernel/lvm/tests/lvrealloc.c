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
static char	*sccsid = "@(#)$RCSfile: lvrealloc.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:22:41 $";
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

int size = 15;
extern char *progname;
extern int errno;

int
main(argc, argv)
int argc;
char **argv;
{
int c, errflag = 0;

/* Establish defaults */
char *vgname = NULL;
char *sourcelv = NULL;
char *destlv = NULL;
int perm = 0777;
struct stat statbuf;
int sminor, dminor;
int vgfd, sfd, dfd;
int optind = 1;


	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	if (argc < 3) {
		fprintf(stderr, "usage: %s: vgname sourcelv destlv", progname);
		exit(1);
	}

	/* Do the volume group thing */
	vgname = argv[optind];
	if ((vgfd = open(vgname, O_RDWR)) < 0) {
		fprintf(stderr, "%s: open of %s failed.  Errno = %d\n",
			progname, vgname, errno);
		exit(1);
	}
	/* Discover the major/minor numbers for the
	   volume group device */
	if (fstat(vgfd, &statbuf) < 0) {
		return(report_error(stderr, "fstat"));
	}
	if (minor(statbuf.st_rdev) != 0) {
		fprintf(stderr, "%s: specified device (%s)", progname, vgname);
		fprintf(stderr, " is not minor device #0\n");
		exit(1);
	}
	optind++;

	/* Get the source logical volume. */
	sourcelv = argv[optind];
	if ((sfd = open(sourcelv, O_RDWR)) < 0) {
		fprintf(stderr, "%s: open of %s failed.  Errno = %d\n",
			progname, sourcelv, errno);
		exit(1);
	}
	/* Discover the major/minor numbers for the source */
	if (fstat(sfd, &statbuf) < 0) {
		return(report_error(stderr, "fstat"));
	}
	if ((sminor = minor(statbuf.st_rdev)) == 0) {
		fprintf(stderr, "%s: can not realloc from lvol 0\n", progname);
		exit(1);
	}
	optind++;

	/* Discover the major/minor numbers for the destination */
	dminor = 2;
	if (dminor == sminor) {
		fprintf(stderr, "%s: can not realloc to itself\n", progname);
		exit(1);
	}

	printf("creating lv %d\n", dminor);
	create_lv(vgfd, dminor);
	printf("reallocating from lv %d to lv %d\n", sminor, dminor);
	realloc_lv(vgfd, sminor, dminor);

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
		slv.maxlxs = 15;
		slv.lv_flags = 0;
		slv.sched_strat = LVM_PARALLEL;
		slv.maxmirrors = 1;
		if (ioctl(vgfd, LVM_CREATELV, &slv) < 0) {
			report_error(stderr, "LVM_CREATELV");
			return (-1);	
		} else
			printf("Created logical volume %d.\n", minor_num);
		slv.lv_flags |= LVM_LVDEFINED;
 		if (check_lv(vgfd, &slv))
			return(-1);
	}
	return(0);
}

realloc_lv(vgfd, sminor, dminor)
int vgfd, sminor, dminor;
{
struct lv_realloclv rlv;
int extents, lx;
int i, m;

	rlv.sourcelv = sminor;
	rlv.destlv = dminor;
	extents = 15;
	rlv.size = extents;
	rlv.extents = (lxmap_t *)malloc(sizeof(lxmap_t)*extents);

	for (i = 0; i < extents; i++) {
		rlv.extents[i].lx_num = i;
		rlv.extents[i].pv_key = 1;
		rlv.extents[i].px_num = i;
		rlv.extents[i].status = 0;
	}
	if (ioctl(vgfd, LVM_REALLOCLV, &rlv) < 0) {
		report_error(stderr, "LVM_REALLOCLV");
		return (-1);	
	} else
		printf("Reallocated %d to %d.\n", sminor, dminor);
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
