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
static char	*sccsid = "@(#)$RCSfile: pvdelete.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:23 $";
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

/* utility program to invoke the deletepv function */

int verbose, removeflag;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "remove", 1, &removeflag, NULL, ARG_BOOL, NULL },
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
    struct lv_querypvpath qpvp;
    char *fname;
    int pv_key;
    char mess[1024];

    /* usage: pvremove [options] vgname pvname */
    /*        pvremove [options] vgname pv_key */
    /* Assumes that volume group is already activated */

    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }
    if (strcmp(progname, "pvremove") == 0) {
	removeflag = 1;
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
    if ((fd = open(fname, O_RDWR)) < 0) {
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
	pv_key = atoi(argv[optind++]);
    } else {
	qpvp.path = argv[optind++];
	if (ioctl(fd, LVM_QUERYPVPATH, &qpvp) < 0) {
		return(report_error(stderr,"LVM_QUERYPVPATH"));
	}
	pv_key = qpvp.pv_key;
    }

    if (removeflag) {
	if (ioctl(fd, LVM_REMOVEPV, &pv_key) < 0) {
		return(report_error(stderr,"LVM_REMOVEPV"));
	}
    } else {
	if (ioctl(fd, LVM_DELETEPV, &pv_key) < 0) {
		return(report_error(stderr,"LVM_DELETEPV"));
	}
    }

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
	"Usage: %s [options] {volgroup device} {physical volume key}\n", s);
    fprintf(stderr,
	"       %s [options] {volgroup device} {physical volume device}\n", s);
    return;
}
