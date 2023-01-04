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
static char	*sccsid = "@(#)$RCSfile: vgcreate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:30:18 $";
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

int maxpvs = 32, maxpxs = 1016, maxlvs = 255, extentsize = 1024*1024;
int maxp_mask, maxx_mask, maxl_mask, ex_mask;
int verbose;
struct option opts[] = {
	{ "maxpvs", 4, &maxpvs, &maxp_mask, ARG_INT, NULL },
	{ "maxpxs", 5, &maxpxs, &maxx_mask, ARG_INT, NULL },
	{ "maxlvs", 5, &maxlvs, &maxl_mask, ARG_INT, NULL },
	{ "extentsize", 2, &extentsize, &ex_mask, ARG_INT, NULL },
	{ "verbose", 1, &verbose, NULL, ARG_BOOL, NULL },
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
	int		fd;
	int		type;
	char		*path;
	char		*pvol;
struct	lv_createvg	cvg;
struct	lv_installpv	ipv;
struct	lv_uniqueID	vg_id;

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

	if (optind > argc) {
		fprintf(stderr,"%s: Volume Group device name required.\n",
			progname);
		usage(progname);
		return(-1);
	}
	path = argv[optind++];
	if (optind > argc) {
		fprintf(stderr,"%s: Physical Volume device name required.\n",
			progname);
		usage(progname);
		return(-1);
	}
	pvol = argv[optind++];

	printf("%s: Creating volume group %s on physical volume %s\n",
	       progname, path, pvol);

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
	vg_id.id1 = 0;
	vg_id.id2 = 0;
	if (ioctl(fd, LVM_SETVGID, &vg_id) < 0) {
		return(report_error(stderr, "LVM_SETVGID"));
	}

	/*
	 * Stuff the createvg argument array.
	 */
	cvg.path = pvol;
	cvg.vg_id.id1 = gethostid();
	cvg.vg_id.id2 = time(NULL);
	cvg.pv_flags = 0;	/* !LVM_NOVGDA */
	cvg.maxlvs = LVM_MAXLVS;
	cvg.maxpvs = maxpvs;
	cvg.maxpxs = maxpxs;
	cvg.pxsize = extentsize;
	cvg.pxspace = extentsize;

	if (verbose) {
		printf("Creating Volume Group.\n");
		print_cvg(&cvg);
	}
	/*
	 * Attempt the create ioctl.
	 */
	if (ioctl(fd, LVM_CREATEVG, &cvg)) {
		return(report_error(stderr,"createvg"));
	}
	printf("Created Volume Group %s\n", path);

	while (pvol = argv[optind++]) {
		ipv.path = pvol;
		ipv.pxspace = 0;
		if (ioctl(fd, LVM_INSTALLPV, &ipv) < 0) {
			return(report_error(stderr, "installpv"));
		} else {
			printf("Installed physical volume %s\n", pvol);
		}
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
	"Usage: %s [options] {volgroup device} {physical volume device}\n", s);
    return;
}

#if __STDC__
#define print_member(S,M)	printf("\t" #M ": %8d (0x%08x)", S->M, S->M)
#define print_member_str(S,M)	printf("\t" #M ": %s", S->M)
#else
#define print_member(S,M)	printf("%15s: %8d (0x%08x)", "M", S->M, S->M)
#define print_member_str(S,M)	printf("%15s: %s", "M", S->M)
#endif

print_cvg(cvg)
struct lv_createvg *cvg;
{
	print_member_str(cvg, path); printf("\n");
	printf("Volume Group Unique Id:    0x%08x%08x\n",
		cvg->vg_id.id1, cvg->vg_id.id2);
	print_member(cvg, pv_flags); printf("\n");
	print_member(cvg, maxlvs); printf("\n");
	print_member(cvg, maxpvs); printf("\n");
	print_member(cvg, maxpxs); printf("\n");
	print_member(cvg, pxsize); printf("\n");
	print_member(cvg, pxspace); printf("\n");

	return;
}
