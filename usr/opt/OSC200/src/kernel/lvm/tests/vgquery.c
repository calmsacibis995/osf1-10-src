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
static char	*sccsid = "@(#)$RCSfile: vgquery.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:30:35 $";
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
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>

#include <lvm/lvm.h>

extern char *progname;

int
main(argc, argv)
int  argc;
char **argv;
{
    struct stat statbuf;
    extern int optind;
    int fd;
    int type;
    char *path;
    struct lv_queryvg qvg;

    /*
     * Get the program name.
     */
    if ((progname = rindex(argv[0], '/')) != NULL) {
	*progname++ = '\0';
    } else {
	progname = argv[0];
    }
    optind = 1;
    if (optind >= argc) {
	fprintf(stderr, "%s: must specify device name.\n", progname);
	usage(progname);
	return(-1);
    }

    path = argv[optind++];

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
    } else {
	printf("Device %s does not exist.\n", path);
	exit(ENOENT);
    }

    /*
     * Open LVOL 0, see if this works...
     */
    if ((fd = open(path, O_RDONLY, 0)) < 0) {
	return(report_error(stderr,"open"));
    }

    /*
     * Attempt the query ioctl.
     */
    if (ioctl(fd, LVM_QUERYVG, &qvg)) {
	return(report_error(stderr,"queryvg"));
    }
    print_qvg(&qvg);

    /*
     * Close LVOL 0 if everything worked...
     */
    if (close(fd) < 0) {
	return(report_error(stderr,"close"));
    }
    return(0);
}

#if __STDC__
#define print_member(S,M) printf("\t" #M ": %8d (0x%08x)\n", S->M, S->M)
#else
#define print_member(S,M) printf("%15s: %8d (0x%08x)\n", "M", S->M, S->M)
#endif

print_qvg(qvg)
struct lv_queryvg *qvg;
{
    printf("Volume Group Unique Id:    0x%08x%08x\n",
	   qvg->vg_id.id1, qvg->vg_id.id2);
    print_member(qvg, maxlvs);
    print_member(qvg, maxpvs);
    print_member(qvg, pxsize);
    print_member(qvg, freepxs);
    print_member(qvg, cur_lvs);
    print_member(qvg, cur_pvs);
    print_member(qvg, status);

    return;
}

usage(s)
char *s;
{
    fprintf(stderr, "usage: %s {device name}\n", s);

    return;
}
