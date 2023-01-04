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
static char	*sccsid = "@(#)$RCSfile: vgactivate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:30:11 $";
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
#include <fcntl.h>

#include <lvm/lvm.h>

#include "parse_opts.h"

extern char *progname;
char string[1024];

struct values flag_values[] = {
	bitname_value(LVM_ACTIVATE_LVS),
	bitname_value(LVM_ALL_PVS_REQUIRED),
	bitname_value(LVM_NONMISSING_PVS_REQUIRED),
	{ NULL, 0, 0}
};

int verbose, nonmissing, all;
int flags = LVM_ACTIVATE_LVS, flag_mask;
struct option opts[] = {
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ "nonmissing", 1, &nonmissing, NULL, ARG_BOOL, NULL },
	{ "all", 1, &all, NULL, ARG_BOOL, NULL },
 	{ "flags", 1, &flags, &flag_mask, ARG_INT, flag_values },
	{ NULL, 0, NULL, 0, NULL }
};

int
main(argc, argv)
	int	argc;
	char	**argv;
{
extern int optind;
extern char *optarg;
int c, errflag = 0;
struct stat statbuf;
	int	fd;
	int	type;
	char	*path;
	char	*pvol;

	/*
	 * Get the program name.
	 */
	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	if (parse_opts(argc, argv)) {
		usage(progname);
		exit(-1);
	}
	if (optind >= argc) {
		fprintf(stderr,"%s: Volume Group pathname required.\n",
			progname);
		usage(progname);
		return(-1);
	}
	path = argv[optind++];

	printf("%s: Activating volume group %s\n", progname, path);

	/*
	 * Make sure that the volume group is real...
	 */
	if (stat(path, &statbuf) == 0) {
		/* something exists with this name already */
		type = statbuf.st_mode&S_IFMT;
		if ((type != S_IFCHR) && (type != S_IFBLK)) {
			printf("%s is not a device.\n", path);
			exit(ENODEV);
		}
	}
	else {
		printf("Must the create volume group device node.\n");
		exit(ENOENT);
	}

	/*
	 * Open LVOL 0, see if this works...
	 */
	if ((fd = open(path, O_RDWR, 0)) < 0) {
		return(report_error(stderr,"open"));
	}

	for (;optind < argc; optind++) {
		if (ioctl(fd, LVM_ATTACHPV, argv[optind]) > 0) {
			sprintf(string, "attach pv %s", argv[optind]);
			return(report_error(stderr, string));
		}
		if (verbose) {
			printf("Attached physical volume %s.\n", argv[optind]);
		}
	}

	/*
	 * Stuff the createvg argument array.
	 */
	if (nonmissing) flags |= LVM_NONMISSING_PVS_REQUIRED;
	if (all) flags |= LVM_ALL_PVS_REQUIRED;

	/*
	 * Attempt the activate ioctl.
	 */
	if (ioctl(fd, LVM_ACTIVATEVG, &flags)) {
		return(report_error(stderr,"activatevg"));
	}

	/*
	 * Close LVOL 0 if everything worked...
	 */
	if (close(fd) < 0) {
		return(report_error(stderr,"close"));
	}
}

usage(s)
char *s;
{
	fprintf(stderr,"Usage: %s volgrp-name [pvol-name ...]\n", s);
}
