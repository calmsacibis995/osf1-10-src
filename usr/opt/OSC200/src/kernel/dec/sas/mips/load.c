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
static char	*sccsid = "@(#)$RCSfile: load.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:38:09 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from load.c	3.1	(ULTRIX/OSF)	2/28/91";
 */

/*
 * This program strips the headers from a executable object,
 * producing the appropriate memory image to be executed. The
 * loaded object is written to standard output, and diagnostics
 * and informative messages are written to standard error. The
 * program returns 0 for success, -1 for failure.
 * Note that .bss is _not_ initialized to zero by this program.
 */
#include <stdio.h>
#include <sys/file.h>
#include <strings.h>
#include <a.out.h>

/*
 * format of a.out file headers
 */
struct execinfo {
	struct filehdr fh;
	AOUTHDR ah;
};

main(argc, argv)
int argc;
char **argv;
{
int fd;
char *progname;
char *filename;
int c;
int errorflag = 0;
extern int optind;
extern char *optarg;
int imagesize;
int blocksize = 0, maxblocks = 0, maxsize = 0;

	while ((c = getopt(argc,argv,"b:N:s:")) != EOF) {
		switch (c) {
		case 'b': blocksize = atoi(optarg); break;
		case 'N': maxblocks = atoi(optarg); break;
		case 's': maxsize = atoi(optarg); break;
		default:
			errorflag++;
		}
	}
	if ((progname = rindex(argv[0],'/')) == NULL) {
		progname = argv[0];
	} else {
		progname++;
	}
	if (blocksize || maxblocks) {
		if ((maxsize != 0) || ((maxsize = blocksize * maxblocks) == 0))
			errorflag++;
	}
	if (errorflag) {
		fprintf(stderr,
	"Usage: %s [-b blocksize -N maxblocks | -s maxbytes] filename\n",
			progname);
		return(-1);
	}
	filename = argv[optind];
	if ((fd = open(filename, O_RDONLY)) < 0) {
		perror(filename);
		return(-1);
	}
	imagesize = load(fd);
	if (maxsize && (imagesize > maxsize)) {
		fprintf(stderr, "%s: load image too large:", progname);
		fprintf(stderr, " %d bytes, maximum is %d bytes.\n",
imagesize, maxsize);
		return(-1);
	} else {
		return(imagesize<0);
	}
}

load(fd)
int fd;
{
struct execinfo  ei;
HDRR		*si;
char *buffer;
char *malloc();
int imagesize;

	if (read(fd, &ei, sizeof(ei)) != sizeof(ei)) {
		fprintf(stderr, "bad a.out format\n");
		return(-1);
	}
	if (N_BADMAG(ei.ah)) {
		fprintf(stderr, "bad magic number\n");
		return(-1);
	}
	lseek(fd, N_TXTOFF(ei.fh, ei.ah), 0);

	fprintf(stderr, "\nSizes:\ntext start 0x%08x length %d\n",
		ei.ah.text_start, ei.ah.tsize);
	imagesize = ei.ah.tsize + 
		(ei.ah.data_start - (ei.ah.text_start+ei.ah.tsize))
		+ ei.ah.dsize;

	buffer = malloc(imagesize);
	bzero(buffer, imagesize);

	if (read(fd, buffer, ei.ah.tsize) != ei.ah.tsize) {
		fprintf(stderr, "short read\n");
		return(-1);
	}

	fprintf(stderr, "data start 0x%08x length %d\n", ei.ah.data_start,
			ei.ah.dsize);
	if (read(fd, buffer + (ei.ah.data_start-ei.ah.text_start), ei.ah.dsize)
	    != ei.ah.dsize) {
		fprintf(stderr, "short read\n");
		return(-1);
	}

	fprintf(stderr, "bss start  0x%08x length %d\n", ei.ah.bss_start,
		ei.ah.bsize);

	fprintf(stderr, "Entry point at 0x%x\n\n", ei.ah.entry);

	fwrite(buffer, 1, imagesize, stdout);

	return(imagesize);
}
