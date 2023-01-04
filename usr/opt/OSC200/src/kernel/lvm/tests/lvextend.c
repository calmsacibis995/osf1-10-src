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
static char	*sccsid = "@(#)$RCSfile: lvextend.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:49 $";
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

extern char *progname;

char mess[1024];

int verbose;
int size = -1, mirrors = 0, pvnum = 2;
int size_mask, mir_mask, pv_mask;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "size", 1, &size, &size_mask, ARG_INT, NULL },
	{ "mirrors", 1, &mirrors, &mir_mask, ARG_INT, NULL },
	{ "pvnum", 1, &pvnum, &pv_mask, ARG_INT, NULL },
	{ NULL, 0, NULL, 0, NULL }
};

int
main(argc, argv)
int argc;
char **argv;
{
	int	fd;
struct	stat statbuf;
extern	int optind;
	char *fname;
	ushort_t minor_num;
struct	lv_lvsize lvs;
	int lxnum;
	int j;

	/* Store the program name. */
	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	/* Check the command line. */
	if (argc < 2) {
		usage(progname);
		return(-1);
	}

	/* Open the device. */
    	optind = 1;
	fname = argv[optind++];
	if ((fd = open(fname, O_RDWR)) < 0) {
		sprintf(mess, "open %s", fname);
		return(report_error(stderr, mess));
	}
	minor_num = -1;
	if (optind >= argc) {
		if (fstat(fd, &statbuf) < 0) {
			sprintf(mess, "fstat lv %s", fname);
			return(report_error(stderr, mess));
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
    
	lvs.minor_num = minor_num;
	lvs.size = size;
	lvs.extents = (lxmap_t *)malloc(sizeof(lxmap_t) * size);
	j = 0;
	for (lxnum = 0; lxnum < size; lxnum++) {
		lvs.extents[j].lx_num = lxnum;
		lvs.extents[j].pv_key = pvnum;
		lvs.extents[j].px_num = lxnum;
		lvs.extents[j].status = 0;
		j++;
	}

	if (ioctl(fd, LVM_EXTENDLV, &lvs) < 0) {
		return(report_error(stderr, "LVM_EXTENDLV"));
	}

	return(0);
}

usage(s)
char *s;
{
    fprintf(stderr, "Usage: %s {volgroup device} {logical volume no}\n", s);
    fprintf(stderr, "       %s {logical volume device}\n", s);
    return;
}
