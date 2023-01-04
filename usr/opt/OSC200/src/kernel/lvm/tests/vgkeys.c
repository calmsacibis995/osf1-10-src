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
static char	*sccsid = "@(#)$RCSfile: vgkeys.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:30:23 $";
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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <lvm/lvm.h>

char *progname;

main(argc,argv)
int argc;
char **argv;
{
    char *path;
    int	fd, type;
    struct stat statbuf;
    struct lv_queryvg qvg;
    struct lv_querypvs qpvs;

    /*
     * Get the program name.
     */
    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }

    if (argc < 2) {
	fprintf(stderr, "Usage: %s <volume group>\n", progname);
	exit(1);
    }

    path = argv[1];

    /* Make sure that the volume group is real. */
    if (stat(path, &statbuf) == 0) {
	/* something exists with this name already */
	type = statbuf.st_mode&S_IFMT;
	if ((type != S_IFCHR) && (type != S_IFBLK)) {
	    printf("%s is not a device.\n", path);
	    exit(ENODEV);
	}
    }
    else {
	printf("No such volume group device.\n");
	exit(ENOENT);
    }

    /* Open LVOL 0, see if this works. */
    if ((fd = open(path, O_RDWR, 0)) < 0)
	return(report_error("open"));

    /* Attempt the query ioctl. */
    if (ioctl(fd, LVM_QUERYVG, &qvg))
	return(report_error("queryvg"));

    /* Attempt the query ioctl. */
    qpvs.numpvs  = qvg.cur_pvs;
    qpvs.pv_keys = (ushort_t *)calloc(sizeof(ushort_t), qvg.cur_pvs);
    if (qpvs.pv_keys == NULL) {
	fprintf(stderr, "calloc failed\n");
	exit(-1);
    }
	
    if (ioctl(fd, LVM_QUERYPVS, &qpvs))
	return(report_error("queryvg"));

    printf("VG keylist:\n");
    print_qpvs(&qpvs);

    /* Close LVOL 0 if everything worked. */
    if (close(fd) < 0) {
	return(report_error(stderr,"close", errno));
    }
}

#if __STDC__
#define print_member(S,M) printf("\t" #M ": %8d (0x%08x)\n", S->M, S->M)
#else
#define print_member(S,M) printf("%10s: %8d (0x%08x)\n", "M", S->M, S->M)
#endif

print_qpvs(qpvs)
struct lv_querypvs *qpvs;
{
    int i;

    printf("Volume   pv_key\n");
    for (i = 0; i < qpvs->numpvs; i++)
	if (qpvs->pv_keys[i] != -1)
	    printf("%6d   %6d\n", i, qpvs->pv_keys[i]);
	else
	    printf("%6d   Invalid\n", i);
}

int
report_error(operation)
char *operation;
{
    fprintf(stderr,"%s: %s failed: %s (errno %d).\n", progname, operation,
	    (errno<sys_nerr)?sys_errlist[errno]:"Unknown error", errno);
    return(errno);
}
