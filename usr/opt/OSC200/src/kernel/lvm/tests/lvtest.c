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
static char	*sccsid = "@(#)$RCSfile: lvtest.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:00 $";
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

#include <lvm/lvm.h>

/* test program to test the creation/change/delete/query lv functions */

int
main(argc, argv)
int argc;
char **argv;
{
struct lv_querylv qlv;
struct lv_statuslv slv;
int fd;
ushort_t minor_num;

	/* usage: lvtest vgname lvname */
	/* Assumes that volume group is already activated */

	if ((fd = open(argv[1],O_RDWR)) < 0) {
		perror("open");
		return(-1);
	}
	minor_num = 1;

	qlv.minor_num = minor_num;
	if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
		perror("LVM_QUERYLV");
		return (-1);
	}

	printf("Queried LV%d:\n", qlv.minor_num);
	print_qlv(&qlv);

	slv.minor_num = minor_num;
	if (!(qlv.lv_flags & LVM_LVDEFINED)) {
		slv.maxlxs = 0;
		slv.lv_flags = LVM_DISABLED;
		slv.sched_strat = LVM_SEQUENTIAL;
		slv.maxmirrors = 0;
		if (ioctl(fd, LVM_CREATELV, &slv) < 0) {
			perror("LVM_CREATELV");
			return (-1);	
		} else
			printf("Created logical volume %d.\n", minor_num);
		slv.lv_flags |= LVM_LVDEFINED;
		check_lv(fd, &slv);
	} else {
		slv.maxlxs = qlv.maxlxs;
		slv.lv_flags = qlv.lv_flags;
		slv.sched_strat = qlv.sched_strat;
		slv.maxmirrors = qlv.maxmirrors;
	}

	slv.maxlxs = 1000;
	slv.lv_flags = slv.lv_flags|LVM_RDONLY;
	if (ioctl(fd, LVM_CHANGELV, &slv) < 0) {
		perror("LVM_CHANGELV");
		return(-1);
	}
	printf("Changed LV data.\n");
	check_lv(fd, &slv);

	if (ioctl(fd, LVM_DELETELV, minor_num) < 0) {
		perror("LVM_DELETELV");
		return (-1);
	}
	printf("Deleted Logical Volume.\n");
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

	qlv.minor_num = 1;
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
	print_member(qlv,numpxs);
	print_member(qlv,numlxs);
	print_member(qlv,maxlxs);
	print_member(qlv,lv_flags);
	print_member(qlv,sched_strat);
	print_member(qlv,maxmirrors);
}
