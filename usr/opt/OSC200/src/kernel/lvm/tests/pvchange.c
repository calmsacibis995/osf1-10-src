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
static char	*sccsid = "@(#)$RCSfile: pvchange.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:20 $";
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

#include "parse_opts.h"

/* utility program to invoke the changelv function */

struct values flag_values[] = {
	bitname_value(LVM_PVNOALLOC),
	bitname_value(LVM_PVDEFINED),
	{ NULL, 0, 0 }
};

struct values status_values[] = {
	bitname_value(LVM_PXSTALE),
	{ NULL, 0, 0 }
};

int verbose, map, query;
int pv_flags, maxdefects;
int pv_mask, maxd_mask;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "query", 1, &query, NULL, ARG_BOOL, NULL },
 	{ "flags", 1, &pv_flags, &pv_mask, ARG_INT, flag_values },
	{ "map", 1, &map, NULL, ARG_BOOL, NULL },
	{ "maxdefects", 4, &maxdefects, &maxd_mask, ARG_INT, NULL },
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
    struct lv_querypv qpv;
    struct lv_querypvpath qpvp;
    struct lv_querypvmap pvmap;
    struct lv_changepv cpv;
    char mess[1024];

    /* usage: [pvchange|pvquery|pvquerymap] [options] vgname pvid */
    /* Assumes that volume group is already activated */

    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }

    if (strcmp(progname, "pvquery") == 0) {
	query = 1;
    } else if (strcmp(progname, "pvquerymap") == 0) {
	query = 1;
	map = 1;
    }
    if (parse_opts(argc, argv)) {
	usage(progname);
	return(-1);
    }

    if (optind >= argc) {
	fprintf(stderr, "Volume Group name is required.\n");
	usage(progname);
	return(1);
    }

    fname = argv[optind++];
    if ((fd = open(fname, query?O_RDONLY:O_RDWR)) < 0) {
	sprintf(mess, "open %s", fname);
	return(report_error(stderr, mess));
    }
    if (fstat(fd, &statbuf) < 0) {
	return(report_error(stderr,"fstat vg"));
    }
    if (minor(statbuf.st_rdev) != 0) {
	fprintf(stderr, "%s: Volume Group name is required.\n", progname);
	usage(progname);
	return(-1);
    }
    if (optind >= argc) {
	fprintf(stderr, "%s: Physical Volume name or key is required.\n",
		progname);
	usage(progname);
	return(-1);
    }
    if (isdigit(*argv[optind])) {
	qpv.pv_key = atoi(argv[optind++]);
	if (ioctl(fd, LVM_QUERYPV, &qpv) < 0) {
		return(report_error(stderr,"LVM_QUERYPV"));
	}
    } else {
	qpvp.path = argv[optind++];
	if (ioctl(fd, LVM_QUERYPVPATH, &qpvp) < 0) {
		return(report_error(stderr,"LVM_QUERYPVPATH"));
	}
	qpv.pv_key = qpvp.pv_key;
	qpv.pv_flags = qpvp.pv_flags;
	qpv.px_count = qpvp.px_count;
	qpv.px_free = qpvp.px_free;
	qpv.px_space = qpvp.px_space;
	qpv.pv_rdev = qpvp.pv_rdev;
	qpv.maxdefects = qpvp.maxdefects;
	qpv.bbpool_len = qpvp.bbpool_len;
    }

    if (verbose || query) {
	    printf("Queried %s (PV%d):\n", fname, qpv.pv_key);
	    print_qpv(&qpv);
    }
    if (map) {
	    pvmap.pv_key = qpv.pv_key;
	    pvmap.numpxs = qpv.px_count;
	    pvmap.map = (pxmap_t *)calloc(pvmap.numpxs, sizeof(pxmap_t));
	    if (ioctl(fd, LVM_QUERYPVMAP, &pvmap) < 0) {
		return(report_error(stderr,"LVM_QUERYPVMAP"));
	    }
	    printf("\nPV Map:\n");
	    print_qpvm(&pvmap);
    }
    if (query) return(0);

    cpv.pv_key = qpv.pv_key;
    cpv.pv_flags = ((qpv.pv_flags & (~pv_mask)) | pv_flags);
    cpv.maxdefects = maxdefects;

    if (verbose) {
	printf("Changing %s (PV%d) to:\n", fname, qpv.pv_key);
	print_cpv(&cpv);
    }

    if (ioctl(fd, LVM_CHANGEPV, &cpv) < 0) {
	return(report_error(stderr,"LVM_CHANGEPV"));
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

print_qpv(qpv)
struct lv_querypv *qpv;
{
	print_member(qpv,pv_key); printf("\n");
	print_member(qpv,pv_flags);
		print_bits(qpv->pv_flags, flag_values); printf("\n");
	print_member(qpv,px_count); printf("\n");
	print_member(qpv,px_free); printf("\n");
	print_member(qpv,px_space); printf("\n");
	print_member(qpv,pv_rdev); printf("\n");
	print_member(qpv,maxdefects); printf("\n");
	print_member(qpv,bbpool_len); printf("\n");
	return;
}

print_cpv(cpv)
struct lv_changepv *cpv;
{
	print_member(cpv,pv_flags);
		print_bits(cpv->pv_flags, flag_values); printf("\n");
	print_member(cpv,maxdefects); printf("\n");
	return;
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
	    "Usage: %s [options] {volgroup device} {physical volume no}\n",
		s);
	fprintf(stderr,
	    "       %s [options] {volgroup device} {physical volume device}\n",
		s);
	return;
}

print_qpvm(p)
struct lv_querypvmap *p;
{
    pxmap_t *m;

    printf("px_num lv_minor lx_num status\n");
    for (m = p->map; m < &p->map[p->numpxs]; m++) {
	printf("%6d  %6d  %6d  0x%04x ", (m - p->map)/sizeof(pxmap_t),
		m->lv_minor, m->lv_extent, m->status);
	print_bits(m->status, status_values); printf("\n");
    }
	
}
