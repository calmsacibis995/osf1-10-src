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
static char	*sccsid = "@(#)$RCSfile: resync.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:23:12 $";
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

extern char *malloc();

#include "parse_opts.h"

struct values flag_values[] = {
	bitname_value(LVM_LVDEFINED),
	bitname_value(LVM_DISABLED),
	bitname_value(LVM_RDONLY),
	bitname_value(LVM_NORELOC),
	bitname_value(LVM_VERIFY),
	bitname_value(LVM_STRICT),
	bitname_value(LVM_NOMWC),
	{ NULL, 0, 0 }
};
struct values sched_values[] = {
	name_value(LVM_PARALLEL),
	name_value(LVM_SEQUENTIAL),
	{ NULL, 0, 0 },
};
#define NON_MIRRORED 0
#define SINGLY_MIRRORED 1
#define DOUBLY_MIRRORED 2
struct values mirror_values[] = {
	name_value(NON_MIRRORED),
	name_value(SINGLY_MIRRORED),
	name_value(DOUBLY_MIRRORED),
	{ NULL, 0, 0 },
};
#ifndef LVM_PXSTALE
#define LVM_PXSTALE 0x1
#endif
struct values status_values[] = {
	bitname_value(LVM_PXSTALE),
	{ NULL, 0, 0}
};

int verbose;
int lxnum = -1, lxn_mask;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "lxnum", 2, &lxnum, &lxn_mask, ARG_INT, NULL },
	{ NULL, 0, NULL, 0, NULL }
};

/* utility verify the mirror avoid function */

extern char *progname;

int
main(argc, argv)
int argc;
char **argv;
{
    int	fd;
    struct stat statbuf;
    char *fname;
    ushort_t minor_num;
    struct lv_querylv qlv;
    struct lv_statuslv slv;
    struct lv_lvsize lvsize;
    struct lv_resynclx rlx;
    char mess[1024];

    /* usage: resync [options] vgname lvnum */
    /*        resync [options] lvname */
    /* Assumes that volume group is already activated */

	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	if (argc < 2) {
		usage(progname);
		return(-1);
	}

	if (parse_opts(argc, argv)) {
		usage(progname);
		return(-1);
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: Logical Volume name required.\n",
			progname);
		usage(progname);
		return(1);
	}

	fname = argv[optind++];
	if ((fd = open(fname, O_RDWR)) < 0) {
		sprintf(mess, "open %s", fname);
		return(report_error(stderr, mess));
	}
	if (fstat(fd, &statbuf) < 0) {
		return(report_error(stderr,"fstat lv"));
	}
	if ((minor_num = minor(statbuf.st_rdev)) == 0) {
		fprintf(stderr, "%s: Logical Volume name required.\n",
			progname);
		usage(progname);
		return(-1);
	}
	if ((statbuf.st_mode&S_IFMT) != S_IFCHR) {
		fprintf(stderr, "%s: Must specify character device.\n",
			progname);
		usage(progname);
		return(-1);
	}

	qlv.minor_num = minor_num;
	if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
		return(report_error(stderr,"LVM_QUERYLV"));
	}

	lvsize.minor_num = minor_num;
	lvsize.size = qlv.maxlxs * (qlv.maxmirrors+1);
	lvsize.extents = (lxmap_t *)malloc(sizeof(lxmap_t)*lvsize.size);
	if (ioctl(fd, LVM_QUERYLVMAP, &lvsize) < 0) {
		return(report_error(stderr,"LVM_QUERYLVMAP"));
	}
	if (lxnum == -1) {
		if (ioctl(fd, LVM_RESYNCLV, &minor_num) < 0) {
			return(report_error(stderr,"LVM_RESYNCLV"));
		}
	} else {
		rlx.minor_num = minor_num;
		rlx.lx_num = lxnum;
		if (ioctl(fd, LVM_RESYNCLX, &rlx) < 0) {
			return(report_error(stderr,"LVM_RESYNCLX"));
		}
		check_extent(fd, qlv.maxmirrors, lxnum);
	}

	/*
	 * Close LVOL 0 if everything worked...
	 */
	if (close(fd) < 0) {
		return(report_error(stderr,"close"));
	}

	return(0);
}

check_extent(fd, maxmirrors, lxnum)
int fd, maxmirrors, lxnum;
{
struct lv_option opt;
int mirror;
extern int errno;
int extsize = 1024*1024;
int offset;
char *master, *copy;

	if (verbose)
		printf("Checking data.\n");

	if (ioctl(fd, LVM_OPTIONGET, &opt) < 0)
		return(report_error(stderr,"LVM_OPTIONGET"));

	if ((master = malloc(extsize)) == NULL) {
		return(report_error(stderr,"malloc master"));
	}
	if ((copy = malloc(extsize)) == NULL) {
		return(report_error(stderr,"malloc copy"));
	}

	mirror = 0;
	opt.opt_avoid = ~(1 << mirror) & ((1 << LVM_MAXCOPIES)-1);
	if (ioctl(fd, LVM_OPTIONSET, &opt) < 0)
		return(report_error(stderr,"LVM_OPTIONSET"));
	if (lseek(fd, (off_t)(lxnum*extsize), L_SET) < 0)
		return(report_error(stderr,"lseek"));
	if (read(fd, master, extsize) < 0) 
		return(report_error(stderr,"read"));
	mirror++;

	for (; mirror < (maxmirrors+1); mirror++) {
		opt.opt_avoid = ~(1 << mirror) & ((1 << LVM_MAXCOPIES)-1);
		if (ioctl(fd, LVM_OPTIONSET, &opt) < 0)
			return(report_error(stderr,"LVM_OPTIONSET"));

		if (lseek(fd, (off_t)(lxnum*extsize), L_SET) < 0)
			return(report_error(stderr,"lseek"));
		if (read(fd, copy, extsize) < 0) {
			return(report_error(stderr,"read"));
		}
		for (offset = 0; offset < extsize; offset++) {
			if (*(master+offset) == *(copy+offset)) continue;
			printf("Mirrors %d and %d differ at byte 0x%x\n",
				0, mirror, offset);
			printf("Data 0: 0x%x 0x%x 0x%x 0x%x\n",
				*(master+offset),
				*(master+offset+1),
				*(master+offset+2),
				*(master+offset+3));
			printf("Data %d: 0x%x 0x%x 0x%x 0x%x\n",
				mirror,
				*(copy+offset),
				*(copy+offset+1),
				*(copy+offset+2),
				*(copy+offset+3));
			break;
		}
	}
	return;
}

#if __STDC__
#define print_member(S,M) printf("\t" #M ": %8d (0x%08x)", S->M, S->M)
#else
#define print_member(S,M) printf("%15s: %8d (0x%08x)", "M", S->M, S->M)
#endif

print_qlv(qlv)
struct lv_querylv *qlv;
{
	print_member(qlv,numpxs); printf("\n");
	print_member(qlv,numlxs); printf("\n");
	print_member(qlv,maxlxs); printf("\n");
	print_member(qlv,lv_flags);
	print_bits(qlv->lv_flags, flag_values); printf("\n");
	print_member(qlv,sched_strat);
	print_bits(qlv->sched_strat, sched_values); printf("\n");
	print_member(qlv,maxmirrors);
		print_bits(qlv->maxmirrors, mirror_values); printf("\n");
}

print_slv(slv)
struct lv_statuslv *slv;
{
	print_member(slv,maxlxs); printf("\n");
	print_member(slv,lv_flags);
	print_bits(slv->lv_flags, flag_values); printf("\n");
	print_member(slv,sched_strat);
	print_bits(slv->sched_strat, sched_values); printf("\n");
	print_member(slv,maxmirrors);
		print_bits(slv->maxmirrors, mirror_values); printf("\n");
}

print_bits(arg, val)
int arg;
struct values *val;
{
int sep;
	sep = '\t';
	for (; val->keyword; val++) {
		if ((arg & val->mask) == val->value) {
			printf("%c%s", sep, val->keyword);
			sep = '|';
		}
	}
}

usage(s)
char *s;
{
	fprintf(stderr,
		"Usage: %s [options] {logical volume device}\n", s);
	return;
}

print_qlvm(p)
struct lv_lvsize *p;
{
lxmap_t *m;

    printf("lx_num  pv_key  px_num  status\n");
    for (m = p->extents; m < &p->extents[p->size]; m++) {
	printf("%6d  %6d  %6d  0x%04x", m->lx_num, m->pv_key, m->px_num, 
	       m->status);
	print_bits(m->status, status_values); printf("\n");
    }
}
