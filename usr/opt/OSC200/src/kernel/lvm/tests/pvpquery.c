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
static char	*sccsid = "@(#)$RCSfile: pvpquery.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:34 $";
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
    struct lv_querypvpath qpvp;
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
	fprintf(stderr, "Usage: %s <volume group> <physical volume>\n",
		progname);
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
    if ((fd = open(path, O_RDWR, 0)) < 0) {
	return(report_error("open"));
    }

    /* Attempt the query ioctl. */
    qpvp.path = argv[2];
    if (ioctl(fd, LVM_QUERYPVPATH, &qpvp)) {
	return(report_error("querypv"));
    }

    printf("Queried PV %d:\n", qpvp.pv_key);
    print_qpvp(&qpvp);

    qpvm.pv_key = qpvp.pv_key;
    qpvm.numpxs = qpvp.px_count;
    qpvm.map    = (pxmap_t *)calloc(sizeof(pxmap_t), qpvp.px_count);
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

print_qpvp(qpvp)
struct lv_querypvpath *qpvp;
{
    print_member(qpvp,pv_flags);
    print_member(qpvp,px_count);
    print_member(qpvp,px_free);
    print_member(qpvp,px_space);
    print_member(qpvp,pv_rdev);
    print_member(qpvp,maxdefects);
    print_member(qpvp,bbpool_len);
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
