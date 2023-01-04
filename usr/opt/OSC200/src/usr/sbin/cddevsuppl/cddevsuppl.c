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
static char rcsid[] = "@(#)$RCSfile: cddevsuppl.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/08/02 13:35:05 $";
#endif

/*
 * cddevsuppl: Map device nodes on a RRIP-format ISO-9660 CD-ROM.
 */

#include <sys/cdrom.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <cdfs/cdfsmount.h>

extern int errno;

void
usage()
{
    fprintf(stderr,
	    "cddevsuppl: usage: cddevsuppl [ -m mapfile | -u unmapfile ] [-c]\n");
    exit(4);
}

void printmappings(void);
void mapfile(char *, int);
void unmapfile(char *, int);

void
main(int argc, char *argv[])
{
    int opt;
    extern char *optarg;
    char *mfile = 0, *ufile = 0;
    extern int optind;
    extern int opterr;
    int contflag = 0;

    while ((opt = getopt(argc, argv, "m:u:c")) != EOF) {
	switch (opt) {
	case 'm':
	    if (ufile || mfile)
		usage();
	    mfile = optarg;
	    break;
	case 'u':
	    if (ufile || mfile)
		usage();
	    ufile = optarg;
	    break;
	case 'c':
	    contflag++;
	    break;
	default:
	    usage();
	}
    }
    if (optind != argc)
	usage();
    if (mfile)
	mapfile(mfile, contflag);
    else if (ufile)
	unmapfile(ufile, contflag);
    else
	printmappings();
    exit(0);
}

void
printmappings(void)
{
    /* we have no file with which to start looking, so we
       walk the mount table looking for M_CDFS file systems with RRIP set
       in the mount flags.  For each one, get its mappings */

    struct statfs *mntbufp;
    int count;

    count = getmntinfo(&mntbufp, MNT_NOWAIT);
    while (count--) {
	if (mntbufp[count].f_type == MOUNT_CDFS &&
	    mntbufp[count].mount_info.cdfs_args.flags & M_RRIP) {
	    /* found one */
	    char path[MAXPATHLEN];
	    int devindex;
	    int new_major, new_minor;
	    int rval;

	    for (devindex = 1; ; devindex++) {
		strcpy(path, mntbufp[count].f_mntonname);
		rval = cd_getdevmap(path, sizeof(path), devindex,
				    &new_major, &new_minor);
		if (rval == 0)		/* no more mappings */
		    break;
		if (rval == -1) {
		    if (errno != EINVAL)
			/* EINVAL probably means devindex out of bounds */
			perror("cddevsuppl: cd_getdevmap");
		    break;
		}
		printf("%s: (%d,%d)\n",
		       path, new_major, new_minor);
	    }
	}
    }
    /* we're just exiting, so don't bother freeing mntbufp */
}

/* ANSI C magic to turn x into "x" */

#define quote(x) _quote(x)
#define _quote(x) # x

/*
 * take a file, and use it to generate mappings.
 */

void
mapfile(char *fname, int cont)
{
    FILE *fp;
    int new_major, new_minor;
    char path[MAXPATHLEN];
    int count;
    int retval;
    int ch;
    int exitstat = 0;

    fp = fopen(fname, "r");
    if (!fp) {
	fprintf(stderr, "cddevsuppl: cannot open file ");
	perror(fname);
	exit(1);
    }

    while ((count = fscanf(fp, "%" quote(MAXPATHLEN) "s%*[ \t]%d%*[ \t]%d",
			   path, &new_major, &new_minor)) == 3) {
	retval = cd_setdevmap(path, CD_SETDMAP, &new_major, &new_minor);
	if (retval == 0) {
	    /* no more mappings allowed */
	    fprintf(stderr, "cddevsuppl: %s: no more device mapping space\n",
		    path);
	    exit(3);
	}
	if (retval == -1) {
	    fprintf(stderr, "cddevsuppl: ");
	    perror(path);
	    if (!cont)
		exit(1);
	    else switch(errno) {
	    case EPERM:
		exitstat = 2;
	    default:
		exitstat = 1;
	    }
	}
	printf("%s: (%d,%d)\n", path, new_major, new_minor);
	/* gobble any comments */
	for (ch = getc(fp); ch != '\n' && ch != EOF; ch = getc(fp));
    }
    fclose(fp);
    if (count == EOF)
	exit(exitstat);
    fprintf(stderr, "cddevsuppl: format error in file %s\n", fname);
    exit(4);
}

void
unmapfile(char *fname, int cont)
{
    FILE *fp;
    int new_major, new_minor;
    char path[MAXPATHLEN];
    int count;
    int retval;
    int ch;
    int exitstat = 0;

    fp = fopen(fname, "r");
    if (!fp) {
	fprintf(stderr, "cddevsuppl: cannot open file ");
	perror(fname);
	exit(1);
    }

    while ((count = fscanf(fp, "%" quote(MAXPATHLEN) "s", path)) == 1) {
	retval = cd_setdevmap(path, CD_UNSETDMAP, &new_major, &new_minor);
	if (retval == 0) {
	    /* mapping not found */
	    fprintf(stderr, "cddevsuppl: %s: not mapped\n",
		    path);
	    if (!cont)
		exit(1);
	    else
		exitstat = 6;
	}
	if (retval == -1) {
	    fprintf(stderr, "cddevsuppl: ");
	    perror(path);
	    if (!cont)
		exit(1);
	    else switch(errno) {
	    case EPERM:
		exitstat = 2;
	    default:
		exitstat = 1;
	    }
	}
	printf("%s: (%d,%d)\n", path, new_major, new_minor);
	/* gobble any comments */
	for (ch = getc(fp); ch != '\n' && ch != EOF; ch = getc(fp));
    }
    fclose(fp);
    if (count == EOF)
	exit(exitstat);
    fprintf(stderr, "cddevsuppl: format error in file %s\n", fname);
    exit(4);
}
