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
static char	*sccsid = "@(#)$RCSfile: pvquery.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:37 $";
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
    struct lv_querypv qpv;
    struct lv_querypvmap qpvm;

    /*
     * Get the program name.
     */
    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }

    if (argc < 3) {
	fprintf(stderr, "Usage: %s <volume group> <pv_key>\n", progname);
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
    qpv.pv_key = atoi(argv[2]);
    if (ioctl(fd, LVM_QUERYPV, &qpv))
	return(report_error("querypv"));

    printf("Queried PV %d:\n", qpv.pv_key);
    print_qpv(&qpv);

    qpvm.pv_key = qpv.pv_key;
    qpvm.numpxs = qpv.px_count;
    qpvm.map    = (pxmap_t *)calloc(sizeof(pxmap_t), qpv.px_count);
    if (qpvm.map == NULL) {
	fprintf(stderr, "calloc failed\n");
	exit(-1);
    }
    if (ioctl(fd, LVM_QUERYPVMAP, &qpvm))
	return(report_error("querypvmap"));
    printf("\nPV Map:\n");
    print_qpvm(&qpvm);

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

print_qpv(qpv)
struct lv_querypv *qpv;
{
    print_member(qpv,pv_flags);
    print_member(qpv,px_count);
    print_member(qpv,px_free);
    print_member(qpv,px_space);
    print_member(qpv,pv_rdev);
    print_member(qpv,maxdefects);
    print_member(qpv,bbpool_len);
}

print_qpvm(p)
struct lv_querypvmap *p;
{
    pxmap_t *m;

    printf("lv_minor lv_extent    status\n");
    for (m = p->map; m < &p->map[p->numpxs]; m++)
	printf("%8d  %8d  0x%08x\n", m->lv_minor, m->lv_extent, m->status);
}

int
report_error(operation)
char *operation;
{
    fprintf(stderr,"%s: %s failed: %s (errno %d).\n", progname, operation,
	    (errno<sys_nerr)?sys_errlist[errno]:"Unknown error", errno);
    return(errno);
}
