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
static char	*sccsid = "@(#)$RCSfile: vgdeactivate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:30:29 $";
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

extern char *sys_errlist[];
extern int sys_nerr;
extern int errno;

char *progname;

int
main(argc, argv)
	int	argc;
	char	**argv;
{
extern int optind;
extern char *optarg;
int c, errflag = 0;
struct stat statbuf;
	int		fd;
	int		type;
	char		*path;
	char		*pvol;

	/*
	 * Get the program name.
	 */
	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	path = argv[1];

	printf("%s: Deactivating volume group %s\n", progname, path);

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
		return(report_error(stderr,"open", errno));
	}

	/*
	 * Attempt the create ioctl.
	 */
	if (ioctl(fd, LVM_DEACTIVATEVG)) {
		return(report_error(stderr,"deactivatevg", errno));
	}

	/*
	 * Close LVOL 0 if everything worked...
	 */
	if (close(fd) < 0) {
		return(report_error(stderr,"close", errno));
	}
}

int
report_error(file, operation, error)
	FILE	*file;
	char	*operation;
	int	error;
{
	fprintf(stderr,"%s: %s failed: %s (errno %d).\n",
		progname, operation,
		(error<sys_nerr)?sys_errlist[error]:"Unknown error", error);
	return(error);
}
