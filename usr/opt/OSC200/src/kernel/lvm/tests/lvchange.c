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
static char	*sccsid = "@(#)$RCSfile: lvchange.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:44 $";
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
#include <signal.h>
#include <setjmp.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <lvm/lvm.h>

#include "parse_opts.h"

/* utility program to invoke the changelv function */

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
struct values status_values[] = {
	bitname_value(LVM_PXSTALE),
	bitname_value(LVM_PXMISSING),
	{ NULL, 0, 0}
};

int verbose, map, query;
int maxmirrors, maxextents, lv_flags, sched_strat;
int maxm_mask, maxe_mask, lv_mask, sched_mask;
struct option opts[] = {
	{ "maxmirrors", 4, &maxmirrors, &maxm_mask, ARG_INT, mirror_values },
	{ "maxextents", 4, &maxextents, &maxe_mask, ARG_INT, NULL },
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "map", 3, &map, NULL, ARG_BOOL, NULL },
	{ "query", 1, &query, NULL, ARG_BOOL, NULL },
 	{ "lv_flags", 1, &lv_flags, &lv_mask, ARG_INT, flag_values },
	{ "scheduler", 5, &sched_strat, &sched_mask, ARG_INT, sched_values },
	{ NULL, 0, NULL, 0, NULL }
};

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
    char mess[1024];

    /* usage: lvchange [options] vgname lvname */
    /* Assumes that volume group is already activated */

    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }

    if (strcmp(progname, "lvquery") == 0) {
	query = 1;
    } else if (strcmp(progname, "lvquerymap") == 0) {
	query = 1;
	map = 1;
    }
    if (parse_opts(argc, argv)) {
	usage(progname);
	return(-1);
    }

    if (optind >= argc) {
	fprintf(stderr, "Volume Group or Logical Volume name is required.\n");
	usage(progname);
	return(1);
    }

    fname = argv[optind++];
    if ((fd = open(fname, query?O_RDONLY:O_RDWR)) < 0) {
	sprintf(mess, "open %s", fname);
	return(report_error(stderr, mess));
    }
    if (optind >= argc) {
	if (fstat(fd, &statbuf) < 0) {
		return(report_error(stderr,"fstat lv"));
	}
	if (minor(statbuf.st_rdev) == 0) {
	    fprintf(stderr, "%s: Logical Volume name or number is required.\n",
		progname);
	    usage(progname);
	    return(-1);
	}
    } else {
	minor_num = atoi(argv[optind++]);
    }

    qlv.minor_num = minor_num;
    if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
	return(report_error(stderr,"LVM_QUERYLV"));
    }

    if (verbose || query) {
	    printf("Queried %s (LV%d):\n", fname, qlv.minor_num);
	    print_qlv(&qlv);
    }
    if (map) {
	    lvsize.minor_num = minor_num;
	    lvsize.size = qlv.maxlxs * (qlv.maxmirrors+1);
	    lvsize.extents = (lxmap_t *)calloc(lvsize.size, sizeof(lxmap_t));
	    if (ioctl(fd, LVM_QUERYLVMAP, &lvsize) < 0) {
		return(report_error(stderr,"LVM_QUERYLVMAP"));
	    }

	    printf("\nLV Map:\n");
	    print_qlvm(&lvsize);
    }
    if (query) return(0);

    slv.minor_num = minor_num;
    slv.lv_flags = ((qlv.lv_flags & (~lv_mask)) | lv_flags);
    slv.sched_strat = ((qlv.sched_strat & (~sched_mask)) | sched_strat);
    slv.maxlxs = ((qlv.maxlxs & (~maxe_mask)) | maxextents);
    slv.maxmirrors = ((qlv.maxmirrors & (~maxm_mask)) | maxmirrors);

    if (verbose) {
	printf("Changing %s (LV%d) to:\n", fname, qlv.minor_num);
	print_slv(&slv);
    }

    if (ioctl(fd, LVM_CHANGELV, &slv) < 0) {
	return(report_error(stderr,"LVM_CHANGELV"));
    }
    /*
     * Close LVOL 0 if everything worked...
     */
    if (close(fd) < 0) {
	return(report_error(stderr,"close"));
    }

    return(0);
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
		"Usage: %s [options] {volgroup device} {logical volume no}\n",
		s);
	fprintf(stderr,
		"       %s [options] {logical volume device}\n", s);
	return;
}

print_qlvm(p)
struct lv_lvsize *p;
{
    lxmap_t *m;

    printf("lx_num  pv_key  px_num  status\n");
    for (m = p->extents; m < &p->extents[p->size]; m++) {
	printf("%6d  %6d  %6d  0x%04x ", m->lx_num, m->pv_key, m->px_num, 
	       m->status);
	print_bits(m->status, status_values); printf("\n");
    }
	
}
