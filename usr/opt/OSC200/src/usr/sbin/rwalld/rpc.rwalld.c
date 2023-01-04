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
static char *rcsid = "@(#)$RCSfile: rpc.rwalld.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/26 17:48:51 $";
#endif


/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */



#include <rpcsvc/rwall.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

int splat();

main()
{
	register SVCXPRT *transp;
	struct sockaddr_in addr;
	int len = sizeof(addr);
	int sock;

#define USE_INETD

#ifdef USE_INETD    

	if (getsockname(0, (struct sockaddr *)&addr, &len) == -1) {
	  perror("rstat: getsockname");
	  exit(1);
	}

	/*
	 * creating a UDP/IP based transport service.
	 */
	if ((transp = svcudp_create(0)) == NULL) {
		fprintf(stderr, "svc_rpc_udp_create: error\n");
		exit(1);
	}

	/*
         *
	 */
	if (!svc_register(transp, WALLPROG, WALLVERS, splat, 0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}

#else   /*  if not defined USE_INETD */

        /* 
	 * The non inetd version of the above 
	 */
	sock = RPC_ANYSOCK;
	(void) pmap_unset(WALLPROG, WALLVERS);

	if ((transp = svcudp_create(sock)) == NULL) {
		fprintf(stderr, "svc_rpc_udp_create: error\n");
		exit(1);
	}

	if (!svc_register(transp, WALLPROG, WALLVERS, splat, IPPROTO_UDP)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);

	/*
	 * added the following lines to unset any previous regs
         */

#endif /* USE_INETD */	

	/*
	 * Loop on select() till a request comes.
	 */
	svc_run(); 

	fprintf(stderr, "Error: svc_run shouldn't have returned\n");
	exit(1);
	/* NOTREACHED */
}

/*
 * splat(): The function that actually invokes the local wall to
 *          print the requested message.
 */

/* 
 * Interesting quarks:
 *   Will not print the same message twice, by remembering the old message 
 *   Will kill itself in 60 seconds after the first message is printed. 
 *        Looks like a safety mechanism put in by Sun, to prevent havoc
 *        to users on the system, with tons of messages
 */ 
char *oldmsg;

splat(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	FILE *fp, *popen();
	char *msg = NULL;

	switch (rqstp->rq_proc) {
		case 0:
			if (svc_sendreply(transp, xdr_void, 0)  == FALSE) {
				fprintf(stderr, "err: rusersd");
				exit(1);
			    }
			exit(0);
		case WALLPROC_WALL:
			if (!svc_getargs(transp, xdr_wrapstring, &msg)) {
			    	svcerr_decode(transp);
				exit(1);
			}
			if (svc_sendreply(transp, xdr_void, 0)  == FALSE) {
				fprintf(stderr, "err: rusersd");
				exit(1);
			}

			/* primitive duplicate filtering */
			if ((oldmsg == (char *) 0) || (strcmp (msg, oldmsg))) {
				fp = popen("/usr/sbin/wall", "w");
				fprintf(fp, "%s", msg);
				(void) pclose(fp);
				if (oldmsg != (char *) 0)
					(void) svc_freeargs (transp,
							     xdr_wrapstring,
							     &oldmsg);
				oldmsg = msg;
			}
			alarm (60);  /* self destruct in 60 secinds */
			return;
		default: 
			svcerr_noproc(transp);
			exit(0);
	}
}
