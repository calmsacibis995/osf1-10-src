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
static char	*sccsid = "@(#)$RCSfile: setstale.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:30:02 $";
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

/* utility program to invoke the stalepx function */

#ifndef LVM_PXSTALE
#define LVM_PXSTALE 0x1
#endif
struct values status_values[] = {
	bitname_value(LVM_PXSTALE),
	{ NULL, 0, 0}
};

int verbose;
int lxnum = -1, lxn_mask;
int mirno = -1, mirn_mask;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "lxnum", 2, &lxnum, &lxn_mask, ARG_INT, NULL },
	{ "mirrornum", 1, &mirno, &mirn_mask, ARG_INT, NULL },
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
    struct lv_lvsize lvsize;
    char mess[1024];
    lxmap_t *lxmap, *lxnew;
    int i, sizenew;

    /* usage: setstale [options] vgname lvname */
    /* Assumes that volume group is already activated */

    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }

    if (parse_opts(argc, argv)) {
	usage(progname);
	return(-1);
    }

    if (optind >= argc) {
	fprintf(stderr, "Volume Group or Logical Volume name is required.\n");
	usage(progname);
	return(-1);
    }

    fname = argv[optind++];
    if ((fd = open(fname, O_RDWR)) < 0) {
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
    if (mirno == -1) {
	fprintf(stderr,"%s: Must specify mirror number.\n");
	return(-1);
    }
 
    qlv.minor_num = minor_num;
    if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
	return(report_error(stderr,"LVM_QUERYLV"));
    }

    lvsize.minor_num = minor_num;
    lvsize.size = qlv.maxlxs * (qlv.maxmirrors+1);
    lvsize.extents = (lxmap_t *)calloc(lvsize.size, sizeof(lxmap_t));
    if (ioctl(fd, LVM_QUERYLVMAP, &lvsize) < 0) {
	return(report_error(stderr,"LVM_QUERYLVMAP"));
    }

    lxmap = lvsize.extents;
    if (lxnum != -1) {
	for (i = 0; i < lvsize.size; lxmap++, i++) {
		if (lxmap->lx_num != lxnum) continue;
		if ((lxmap+mirno)->lx_num != lxnum) {
			fprintf(stderr,
			"Mirror %d of logical extent %d not found.\n",
				mirno, lxnum);
			return(-1);
		}
		lvsize.extents = lxmap+mirno;
		break;
	}
	if (i == lvsize.size) {
		fprintf(stderr,"Logical extent %d not found.\n", lxnum);
		return(-1);
	}
	lvsize.size = 1;
    } else {
	lxnew = lxmap;
	sizenew = 0;
	for (i = 0; i < lvsize.size; lxmap++, i++) {
		if (lxmap->lx_num == lxnum) continue;
		lxnum = lxmap->lx_num;
		if ((lxmap+mirno)->lx_num == lxnum) {
			*lxnew = *(lxmap+mirno);
			lxnew++;
			sizenew++;
		}
	}
	if (sizenew == 0) {
		fprintf(stderr,"No physical extents found for mirror %d.\n",
			mirno);
		return(-1);
	}
	lvsize.size = sizenew;
    }

    if (verbose && (lxnum != -1)) {
	printf("Setting logical extent %d, mirror %d to stale.\n",
		lxnum, mirno);
    } else if (verbose) {
	printf("Setting %d physical extents to stale.\n", lvsize.size);
    }

    if (ioctl(fd, LVM_DEBUG_STALEPX, &lvsize) < 0) {
	return(report_error(stderr,"LVM_DEBUG_STALEPX"));
    }
    printf("Marked %d extents as stale.\n", lvsize.size);
    /*
     * Close LVOL 0 if everything worked...
     */
    if (close(fd) < 0) {
	return(report_error(stderr,"close"));
    }

    return(0);
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
