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
static char	*sccsid = "@(#)$RCSfile: msck.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:22:56 $";
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

int verbose, debug;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "debug", 1, &debug, NULL, ARG_BOOL, NULL },
	{ NULL, 0, NULL, 0, NULL }
};
#define BUFFERSIZE 8192

/* Mirror Synchronization ChecK program  (msck - "Em-suck") */

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
    struct lv_queryvg qvg;
    struct lv_querylv qlv;
    struct lv_statuslv slv;
    struct lv_lvsize lvsize;
    char mess[1024];

    /* usage: msck [options] vgname lvnum */
    /*        msck [options] lvname */
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

	parse_opts(argc, argv);

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
	if (ioctl(fd, LVM_QUERYVG, &qvg) < 0) {
		return(report_error(stderr,"LVM_QUERYVG"));
	}

	qlv.minor_num = minor_num;
	if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
		return(report_error(stderr,"LVM_QUERYLV"));
	}

	if (debug) {
		printf("Queried %s (LV%d):\n", fname, qlv.minor_num);
		print_qlv(&qlv);
	}
	lvsize.minor_num = minor_num;
	lvsize.size = qlv.maxlxs * (qlv.maxmirrors+1);
	lvsize.extents = (lxmap_t *)malloc(sizeof(lxmap_t)*lvsize.size);
	if (ioctl(fd, LVM_QUERYLVMAP, &lvsize) < 0) {
		return(report_error(stderr,"LVM_QUERYLVMAP"));
	}
	if (debug) {
		printf("\nLV Map:\n");
		print_qlvm(&lvsize);
	}
	/*
	 * Here we do the work: for every physical extent found
	 * in STALE state, set the avoid mask to various values and
	 * verify that we cannot read the STALE data, and that
	 * we can always read good data.
	 */
	if (check_mirrors(fd, qvg.pxsize, &lvsize)) {
		return(200);
	}

	/*
	 * Close LVOL 0 if everything worked...
	 */
	if (close(fd) < 0) {
		return(report_error(stderr,"close"));
	}

	return(0);
}

int
check_mirrors(fd, extsize, lvsize)
int fd;
int extsize;
struct lv_lvsize *lvsize;
{
lxmap_t *map;
int lxnum;
int x, m, e;
int bad, exists;

	lxnum = -1;
	for (map = lvsize->extents, x = 0; x < lvsize->size; map++, x++) {
		if (map->lx_num != lxnum) {
			if (lxnum != -1) {
				e = do_mirrors(fd, extsize, lxnum, exists, bad);
				if (e) 
					return(e);
			}
			m = 0;
			exists = 0;
			bad = 0;
			lxnum = map->lx_num;
		}
		if (map->status & LVM_PXSTALE) {
			bad |= (1<<m);
		}
		exists |= (1<<m);
		m++;
	}
	if (lxnum != -1) {
		e = do_mirrors(fd, extsize, lxnum, exists, bad);
	}
	return(e);
}

int
do_mirrors(fd, extsize, lxnum, exists, bad)
int fd, extsize, lxnum, exists, bad;
{
int good, reference, mirror, mask;
off_t offset, end;
char goodbuf[BUFFERSIZE];
char ckbuf[BUFFERSIZE];

	if (verbose)
		printf("Checking logical extent 0x%x (E: 0x%x, B: 0x%x)\n",
			lxnum, exists, bad);

	/* First determine if a "good" copy exists. */
	if ((good = (exists & ~bad)) == 0) {
		fprintf(stderr, "No good copy for logical extent %d\n",
				lxnum);
		return(1);
	}
	/* Pick a single copy to use as the reference. */
	if (good & 1) reference = 1;
	else if (good & 2) reference = 2;
	else if (good & 4) reference = 4;
	
	if ((reference|bad) == exists) {
		/* only one good copy exists - it can't be out of sync */
		return(0);
	}

	offset = lxnum * extsize;
	end = offset + extsize;

	for (; offset < end; offset += BUFFERSIZE) {
		/* first read "good" copy" */
		if (avoid(fd, (~reference)&exists) > 0) {
			fprintf(stderr,"avoid failed.\n");
			return(1);
		}
		if (lseek(fd, offset, 0) < 0) {
			return(report_error(stderr,"lseek"));
		}
		bzero(goodbuf, BUFFERSIZE);
		if (read(fd, goodbuf, BUFFERSIZE) < 0) {
			fprintf(stderr,"Read of reference failed: extent %d\n",
				lxnum);
			return(1);
		}
		for (mirror = 0; mirror < 3; mirror++) {
			mask = 1 << mirror;
			/* can't check non-existent mirror */
			if ((mask & exists) == 0) continue;

			/* don't check bad mirrors, or compare against self */
			if (mask & (reference|bad)) continue;

			/* Avoid all mirrors except this one */
			if (avoid(fd, (~mask)&exists) > 0) {
				fprintf(stderr,"avoid failed.\n");
				return(1);
			}
			if (lseek(fd, offset, 0) < 0) {
				return(report_error(stderr,"lseek"));
			}
			bzero(ckbuf, BUFFERSIZE);
			if (read(fd, ckbuf, BUFFERSIZE) < 0) {
				fprintf(stderr,"Read failed: extent %d\n",
					lxnum);
				return(1);
			}
			if (bcmp(goodbuf, ckbuf, BUFFERSIZE)) {
				fprintf(stderr, "compare failed: ");
				fprintf(stderr, "extent %d, ", lxnum);
				fprintf(stderr, "reference 0x%X, ", reference);
				fprintf(stderr, "check 0x%X\n", mask);
				if (debug) {
					int xx;
					for (xx = 0; xx < BUFFERSIZE; xx++) {
					    if (goodbuf[xx] == ckbuf[xx])
						continue;
					    fprintf(stderr, "0x%x: %d %d\n",
						xx, goodbuf[xx], ckbuf[xx]);
					}
				}
				return(1);
			}
		}
	}
	return(0);
}

int
avoid(fd, mask)
int fd, mask;
{
struct lv_option lv_option;

	/* Check that avoiding all existing mirrors fails on read */
	lv_option.opt_avoid = mask;
	lv_option.opt_options = 0;

	if (ioctl(fd, LVM_OPTIONSET, &lv_option) < 0) {
		return(report_error(stderr, "LVM_OPTIONSET"));
	}
	if (ioctl(fd, LVM_OPTIONGET, &lv_option) < 0) {
		return(report_error(stderr, "LVM_OPTIONGET"));
	}
	if (lv_option.opt_avoid != mask) {
		fprintf(stderr, "Avoid mask did not set: Ex: x%x Rcd: x%x\n",
			mask, lv_option.opt_avoid);
		return(1);
	}
	return(0);
}

#if __STDC__
#define print_member(S,M) printf("%15s: %8d (0x%08x)", #M, S->M, S->M)
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
    int first = 1;

    for (m = p->extents; m < &p->extents[p->size]; m++) {
	if (m->status) {
		if (first) {
			first = 0;
			printf("lx_num  pv_key  px_num  status\n");
		}
		printf("%6d  %6d  %6d  0x%04x ", m->lx_num, m->pv_key,
			m->px_num, m->status);
		print_bits(m->status, status_values); printf("\n");
	}
    }
    return;
}
