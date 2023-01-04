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
static char	*sccsid = "@(#)$RCSfile: openclose.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:09 $";
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
#include <sys/param.h>
#include <fcntl.h>

extern char *sys_errlist[];
extern int sys_nerr;
extern int errno;
extern char *malloc();

char *progname;

int
main(argc, argv)
int argc;
char **argv;
{
extern int optind;
extern char *optarg;
int c, errflag = 0;

/* Establish defaults */
char *devname = "/dev/lv0";
dev_t devno = makedev(16, 0);
int major_no, minor_no;
int mode = 020000;	/* char special */
int perm = 0777;
int readflag = 0;
int writeflag = 0;
char *comma;
int fd;

	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	while ((c = getopt(argc, argv, "b:c:p:rw")) != EOF) {
		switch (c) {
			case 'b': /* block special */
				mode = 010000;
					/* FALLTHROUGH */
			case 'c': /* character special: depends on init value */
				if ((comma = rindex(optarg,',')) != NULL) {
					minor_no = atoi(comma+1);
					*comma = '\0';
				}
				major_no = atoi(optarg);
				devno = makedev(major_no, minor_no);
				break;
			case 'p': /* permissions */
				perm = atoi(optarg);
				break;
			case 'r':
				readflag++;
				break;
			case 'w':
				writeflag++;
				break;
			default: errflag++;
				break;
		}
	}
	mode |= (perm&0777);
	umask(0);

	if (optind < argc) {
		for (; optind < argc; optind++) {
			devname = argv[optind];
			if (do_open(devname, devno, mode, &fd)==0) {
				if (readflag)do_read(fd);
				do_close(fd);
			}
			devno++;
		}
	} else {
		if (do_open(devname, devno, mode, &fd)==0) {
			if (readflag)do_read(fd);
			do_close(fd);
		}
	}
	return(0);
}


int
do_open(path, devno, mode, fdp)
char *path;
dev_t devno;
int mode;
int *fdp;
{
struct stat statbuf;
int type;

	if (stat(path, &statbuf) == 0) {
		/* something exists with this name already */
		type = statbuf.st_mode&S_IFMT;
		if ((type == S_IFCHR) || (type == S_IFBLK)) {
			printf("%s exists: %s special, major %d, minor %d.\n",
				path, type==S_IFCHR?"character":"block",
				major(statbuf.st_rdev),
				minor(statbuf.st_rdev));
			if (statbuf.st_rdev != devno) {
				printf("%s is not correct device.\n", path);
				return(EEXIST);
			}
		} else {
			printf("%s is not a device.\n", path);
			return(ENODEV);
		}
	} else {
		if (geteuid()) {
			printf("Must be superuser to create device node.\n");
			return(EPERM);
		}
		if (mknod(path, mode, devno) < 0) {
			return(report_error(stderr,"mknod", errno));
		}
	}

	if ((*fdp = open(path, O_RDWR, 0)) < 0) {
		return(report_error(stderr,"open", errno));
	} else {
		printf("open succeeded: file descriptor is %d\n", *fdp);
	}
	return(0);
}

int
do_read(fd)
int fd;
{
char *buffer;
int error = 0;
int iocnt;

	buffer = malloc(DEV_BSIZE);
	if ((iocnt = read(fd, buffer, DEV_BSIZE)) < 0) {
		error = report_error(stderr,"read", errno);
	} else {
		printf("Read %d bytes from device.\n", iocnt);
	}

	free(buffer);

	return(error);
}


int
do_close(fd)
int fd;
{
	if (close(fd) < 0) {
		return(report_error(stderr,"close", errno));
	} else {
		printf("close succeeded.\n");
	}
	return;
}

int
report_error(file, operation, error)
FILE *file;
char *operation;
int error;
{
	fprintf(stderr,"%s: %s failed: %s (errno %d).\n",
		progname, operation,
		(error<sys_nerr)?sys_errlist[error]:"Unknown error", error);
	return(error);
}
