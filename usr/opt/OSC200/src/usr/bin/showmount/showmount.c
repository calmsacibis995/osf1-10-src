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
static char	*sccsid = "@(#)$RCSfile: showmount.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/27 11:02:58 $";
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
/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint


#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <nfs/rpcv2.h>
#include <sys/time.h>

/* Constant defs */
#define	ALL	1
#define	DIRS	2

#define	DODUMP		0x1
#define	DOEXPORTS	0x2

struct timeval TIMEOUT = { 25, 0 };

struct mountlist {
	struct mountlist *ml_left;
	struct mountlist *ml_right;
	char	ml_host[RPCMNT_NAMELEN+1];
	char	ml_dirp[RPCMNT_PATHLEN+1];
};

struct grouplist {
	struct grouplist *gr_next;
	char	gr_name[RPCMNT_NAMELEN+1];
};

struct exportslist {
	struct exportslist *ex_next;
	struct grouplist *ex_groups;
	char	ex_dirp[RPCMNT_PATHLEN+1];
};

static struct mountlist *mntdump;
static struct exportslist *exports;
static int type = 0;
int xdr_mntdump(), xdr_exports();

/*
 * This command queries the NFS mount daemon for it's mount list and/or
 * it's exports list and prints them out.
 * See "NFS: Network File System Protocol Specification, RFC1094, Appendix A"
 * for detailed information on the protocol.
 */
main(argc, argv)
	int argc;
	char **argv;
{
	register struct mountlist *mntp;
	register struct exportslist *exp;
	register struct grouplist *grp;
	extern char *optarg;
	extern int optind;
	register int rpcs = 0;
	char ch;
	char *host;
	int estat;
	CLIENT *cl;

	while ((ch = getopt(argc, argv, "ade")) != EOF)
		switch((char)ch) {
		case 'a':
			if (type == 0) {
				type = ALL;
				rpcs |= DODUMP;
			} else
				usage();
			break;
		case 'd':
			if (type == 0) {
				type = DIRS;
				rpcs |= DODUMP;
			} else
				usage();
			break;
		case 'e':
			rpcs |= DOEXPORTS;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		host = *argv;
	else
		host = "localhost";
	if (rpcs == 0)
		rpcs = DODUMP;

	/*
	 * First try tcp, then drop back to udp if tcp is unavailable 
	 * (an old version of mountd perhaps).  Using tcp is preferred
	 * because it can handle arbitrarily long export lists.
	 */
	cl = clnt_create(host, RPCPROG_MNT, RPCMNT_VER1, "tcp");
	if (cl == NULL) {
		cl = clnt_create(host, RPCPROG_MNT, RPCMNT_VER1, "udp");
		if (cl == NULL) {
			clnt_pcreateerror(host);
			exit(1);
		}
	}

	if (rpcs & DODUMP)
		if (estat = clnt_call(cl, RPCMNT_DUMP,
		    xdr_void, 0, xdr_mntdump, (char *)&mntdump, TIMEOUT)) {
			fprintf(stderr, "Can't do Mountdump rpc: %s\n",
				clnt_sperrno(estat));
			exit(1);
		}
	if (rpcs & DOEXPORTS)
		if (estat = clnt_call(cl, RPCMNT_EXPORT,
		    xdr_void, 0, xdr_exports, (char *)&exports, TIMEOUT)) {
			fprintf(stderr, "Can't do Exports rpc: %s\n", 
				clnt_sperrno(estat));
			exit(1);
		}

	/* Now just print out the results */
	if (rpcs & DODUMP) {
		switch (type) {
		case ALL:
			printf("All mount points on %s:\n", host);
			break;
		case DIRS:
			printf("Directories on %s:\n", host);
			break;
		default:
			printf("Hosts on %s:\n", host);
			break;
		};
		print_dump(mntdump);
	}
	if (rpcs & DOEXPORTS) {
		printf("Exports list on %s:\n", host);
		exp = exports;
		while (exp) {
			printf("%-35s", exp->ex_dirp);
			grp = exp->ex_groups;
			if (grp == NULL) {
				printf("Everyone\n");
			} else {
				while (grp) {
					printf("%s ", grp->gr_name);
					grp = grp->gr_next;
				}
				printf("\n");
			}
			exp = exp->ex_next;
		}
	}
}

/*
 * Xdr routine for retrieving the mount dump list
 */
xdr_mntdump(xdrsp, mlp)
	XDR *xdrsp;
	struct mountlist **mlp;
{
	register struct mountlist *mp;
	register struct mountlist *tp;
	register struct mountlist **otp;
	int val, val2;
	int bool;
	char *strp;

	*mlp = (struct mountlist *)0;
	if (!xdr_bool(xdrsp, &bool))
		return (0);
	while (bool) {
		mp = (struct mountlist *)malloc(sizeof(struct mountlist));
		if (mp == NULL)
			return (0);
		mp->ml_left = mp->ml_right = (struct mountlist *)0;
		strp = mp->ml_host;
		if (!xdr_string(xdrsp, &strp, RPCMNT_NAMELEN))
			return (0);
		strp = mp->ml_dirp;
		if (!xdr_string(xdrsp, &strp, RPCMNT_PATHLEN))
			return (0);

		/*
		 * Build a binary tree on sorted order of either host or dirp.
		 * Drop any duplications.
		 */
		if (*mlp == NULL) {
			*mlp = mp;
		} else {
			tp = *mlp;
			while (tp) {
				val = strcmp(mp->ml_host, tp->ml_host);
				val2 = strcmp(mp->ml_dirp, tp->ml_dirp);
				switch (type) {
				case ALL:
					if (val == 0) {
						if (val2 == 0) {
							free((caddr_t)mp);
							goto next;
						}
						val = val2;
					}
					break;
				case DIRS:
					if (val2 == 0) {
						free((caddr_t)mp);
						goto next;
					}
					val = val2;
					break;
				default:
					if (val == 0) {
						free((caddr_t)mp);
						goto next;
					}
					break;
				};
				if (val < 0) {
					otp = &tp->ml_left;
					tp = tp->ml_left;
				} else {
					otp = &tp->ml_right;
					tp = tp->ml_right;
				}
			}
			*otp = mp;
		}
next:
		if (!xdr_bool(xdrsp, &bool))
			return (0);
	}
	return (1);
}

/*
 * Xdr routine to retrieve exports list
 */
xdr_exports(xdrsp, exp)
	XDR *xdrsp;
	struct exportslist **exp;
{
	register struct exportslist *ep;
	register struct grouplist *gp;
	int bool, grpbool;
	char *strp;

	*exp = (struct exportslist *)0;
	if (!xdr_bool(xdrsp, &bool))
		return (0);
	while (bool) {
		ep = (struct exportslist *)malloc(sizeof(struct exportslist));
		if (ep == NULL)
			return (0);
		ep->ex_groups = (struct grouplist *)0;
		strp = ep->ex_dirp;
		if (!xdr_string(xdrsp, &strp, RPCMNT_PATHLEN))
			return (0);
		if (!xdr_bool(xdrsp, &grpbool))
			return (0);
		while (grpbool) {
			gp = (struct grouplist *)malloc(sizeof(struct grouplist));
			if (gp == NULL)
				return (0);
			strp = gp->gr_name;
			if (!xdr_string(xdrsp, &strp, RPCMNT_NAMELEN))
				return (0);
			gp->gr_next = ep->ex_groups;
			ep->ex_groups = gp;
			if (!xdr_bool(xdrsp, &grpbool))
				return (0);
		}
		ep->ex_next = *exp;
		*exp = ep;
		if (!xdr_bool(xdrsp, &bool))
			return (0);
	}
	return (1);
}

static
usage()
{
	fprintf(stderr, "Usage: showmount [-ade] host\n");
	exit(1);
}

/*
 * Print the binary tree in inorder so that output is sorted.
 */
print_dump(mp)
	struct mountlist *mp;
{

	if (mp == NULL)
		return;
	if (mp->ml_left)
		print_dump(mp->ml_left);
	switch (type) {
	case ALL:
		printf("%s:%s\n", mp->ml_host, mp->ml_dirp);
		break;
	case DIRS:
		printf("%s\n", mp->ml_dirp);
		break;
	default:
		printf("%s\n", mp->ml_host);
		break;
	};
	if (mp->ml_right)
		print_dump(mp->ml_right);
}
