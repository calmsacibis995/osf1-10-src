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
static char	*sccsid = "@(#)$RCSfile: nfsd.c,v $ $Revision: 4.2.2.5 $ (DEC) $Date: 1992/11/25 15:46:24 $";
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
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <nfs/nfs.h>
#include <nfs/rpcv2.h>
#include <nfs/nfsv2.h>
#if	UNIX_DOMAIN
#include <sys/un.h>
#define NFSD_PATH_NAME "/tmp/.nfsd"
#endif	/* UNIX_DOMAIN */

/* Global defs */
#ifdef DEBUG
#define	syslog(e, s)	fprintf(stderr,(s))
int debug = 1;
#else
int debug = 0;
#endif

/*
 * Nfs server daemon mostly just a user context for nfssvc()
 * 1 - do file descriptor and signal cleanup
 * 2 - create server socket
 * 3 - register socket with portmap
 * 4 - nfssvc(sock, msk, mtch)
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	register int i;
	int cnt, sock;
#if	UNIX_DOMAIN
	struct sockaddr_un saddr_un;
#else
	struct sockaddr_in saddr;
#endif	/* UNIX_DOMAIN */
	u_int msk, mtch;

#if	!UNIX_DOMAIN
	if (debug == 0) {
		daemon(0, 0);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
	}
#endif	/* UNIX_DOMAIN */
	openlog("nfsd:", LOG_PID, LOG_DAEMON);
#if	UNIX_DOMAIN
	/*
	 * we need to change this so it's more flexible and can accept
	 * other domains as well.  At least make it so that one nfsd 
	 * call can listen on both INET and UNIX domains.
	 */
	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "Can't create socket");
		exit(1);
	}
	saddr_un.sun_family = AF_UNIX;
	strcpy(saddr_un.sun_path, NFSD_PATH_NAME);
	saddr_un.sun_len = 0;
	sizeof(u_char) + 1;
	if (bind(sock, &saddr_un, strlen(saddr_un.sun_path) +
		 sizeof(u_char) + sizeof(u_char) + 1) < 0) {
		syslog(LOG_ERR, "Can't bind addr");
		exit(1);
	}
#else
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "Can't create socket");
		exit(1);
	}
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(NFS_PORT);
	if (bind(sock, &saddr, sizeof(saddr)) < 0) {
		syslog(LOG_ERR, "Can't bind addr");
		exit(1);
	}
	pmap_unset(RPCPROG_NFS, NFS_VER2);
	if (!pmap_set(RPCPROG_NFS, NFS_VER2, IPPROTO_UDP, NFS_PORT)) {
		syslog(LOG_ERR, "Can't register with portmap");
		exit(1);
	}
#endif	/* UNIX_DOMAIN */
	if (argc == 2) {
		if ((cnt = atoi(argv[1])) <= 0)
		    cnt = 1;
		if (cnt > MAXNFSDS)
		    cnt = MAXNFSDS;
		msk = 0;
		mtch = 0;
	} else if (argc == 4) {
		if ((cnt = atoi(argv[1])) <= 0)
		    cnt = 1;
		if (cnt > MAXNFSDS)
		    cnt = MAXNFSDS;
		msk = inet_addr(argv[2]);
		mtch = inet_addr(argv[3]);
	} else {
		cnt = 1;
		msk = 0;
		mtch = 0;
	}
	for (i = 1; i < cnt; i++)
		if (fork() == 0)
			break;
	if (nfssvc(sock, msk, mtch) < 0)	/* Only returns on error */
		syslog(LOG_ERR, "nfssvc() failed %m");
	exit();
}
