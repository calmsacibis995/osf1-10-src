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
static char	*sccsid = "@(#)$RCSfile: lvreduce.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:58 $";
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
#include <fcntl.h>

#include <lvm/lvm.h>

#include "parse_opts.h"

extern char *progname;

int verbose;
int mirror = -1, mir_mask;
struct option opts[] = {
	{ "mirror", 1, &mirror, &mir_mask, ARG_INT, NULL },
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
	{ NULL, 0, NULL, 0, NULL }
};

extern char *progname;

int
main(argc, argv)
	int	argc;
	char	**argv;
{
extern int optind;
int c, errflag = 0;
struct stat statbuf;
	int		fd;
	int		type;
	char		*path;
	ushort_t	minor_num;
	int		i;
struct	lv_lvsize	lvs;
struct	lv_querylv	qlv;

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
		return(-1);
	}

	if (optind >= argc) {
		fprintf(stderr,
			"Volume Group or Logical Volume name is required.\n");
		usage(progname);
		return(-1);
	}
	path = argv[optind++];
	if ((fd = open(path, O_RDWR)) < 0) {
		return(report_error(stderr,"open"));
	}

	if (optind >= argc) {
		if (fstat(fd, &statbuf) < 0) {
			return(report_error(stderr,"fstat lv"));
		}
		if (minor(statbuf.st_rdev) == 0) {
		    fprintf(stderr,
			"%s: Logical Volume name or number is required.\n",
				progname);
		    return(-1);
		}
	} else {
		minor_num = atoi(argv[optind++]);
	}

	if (optind != argc) {
		fprintf(stderr, "%s: extra arguments ignored.\n", progname);
	}
	qlv.minor_num = minor_num;

	if (ioctl(fd, LVM_QUERYLV, &qlv) < 0) {
		return(report_error(stderr, "LVM_QUERYLV"));
	}
	lvs.minor_num = minor_num;
	lvs.size = qlv.numpxs;
	lvs.extents = (lxmap_t *)malloc(sizeof(lxmap_t)*lvs.size);

	if (ioctl(fd, LVM_QUERYLVMAP, &lvs) < 0) {
		return(report_error(stderr, "LVM_QUERYLVMAP"));
	}
	if (mirror != -1) {
	}

	if (ioctl(fd, LVM_REDUCELV, &lvs) < 0) {
		return(report_error(stderr,"LVM_REDUCELV"));
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
		"Usage: %s {volgroup device} {logical volume no}\n", s);
	fprintf(stderr,
		"       %s {logical volume device}\n", s);
	return;
}
