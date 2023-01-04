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
static char	*sccsid = "@(#)$RCSfile: lvdelete.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:47 $";
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
#include <signal.h>
#include <setjmp.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <lvm/lvm.h>

/* utility program to invoke the delete lv function */
extern char *progname;

int
main(argc, argv)
int argc;
char **argv;
{
struct lv_querylv qlv;
struct lv_statuslv slv;
struct stat statbuf;
int fd;
int minor_num;

	/* usage: lvdelete vgname lvname */
	/* Assumes that volume group is already activated */

	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}
	if (argc < 3) {
		usage(progname);
		return(-1);
	}

	if ((fd = open(argv[1],O_RDWR)) < 0) {
		return(report_error(stderr,"open"));
	}
	if (isdigit(*argv[2])) {
		minor_num = atoi(argv[2]);
	} else if (*argv[2] == '/') {
		if (stat(argv[2], &statbuf) < 0) {
			return(report_error("stat lv"));
		}
		minor_num = minor(statbuf.st_rdev);
	} else {
		fprintf(stderr,
	"%s: must supply path name or minor number of logical volume.\n",
			argv[0]);
		return(-1);
	}

	if (ioctl(fd, LVM_DELETELV, &minor_num) < 0) {
		return(report_error(stderr,"LVM_DELETELV"));
	}
	printf("Deleted Logical Volume %s (LV%d).\n", argv[2], minor_num);
	slv.minor_num = minor_num;
	slv.maxlxs = 0;
	slv.lv_flags = 0;
	slv.sched_strat = 0;
	slv.maxmirrors = 0;

	check_lv(fd, &slv);

	return (0);
}

check_lv(fd, slv)
int fd;
struct lv_statuslv *slv;
{
struct lv_querylv qlv;

	qlv.minor_num = slv->minor_num;
	if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
		perror("LVM_QUERYLV");
		return(-1);
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

usage(s)
char *s;
{
    fprintf(stderr,
	"Usage: %s [options] {volgroup device} {lvol no}\n", s);
    fprintf(stderr,
	"       %s [options] {volgrp device} {lvol device}\n", s);
    return;
}

