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
static char *rcsid = "@(#)$RCSfile: rpc.sprayd.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/26 17:49:42 $";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <rpcsvc/spray.h>

#define CLOSEDOWN 120		/* how many secs to wait before exiting */

int dirty;
unsigned cnt;
struct timeval tv;
struct spraycumul cumul;
int spray(), closedown();

main(argc, argv)
	char **argv;
{
	SVCXPRT *transp;
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

#define USE_INET
	
#ifdef USE_INET

	if (getsockname(0, &addr, &len) != 0) {
		perror("rstat: getsockname");
		exit(1); 
	}

	transp = svcudp_create(0);
	if (transp == NULL) {
		fprintf(stderr, "%s: couldn't create RPC server\n", argv[0]);
		exit(1);
	}
	if (!svc_register(transp, SPRAYPROG, SPRAYVERS, spray, 0)) {
		fprintf(stderr, "%s: couldn't register SPRAYPROG\n", argv[0]);
		exit(1);
	}

#else  /* USE_INET */

        (void) pmap_unset(SPRAYPROG, SPRAYVERS);
	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf(stderr, "%s: couldn't create RPC server\n", argv[0]);
		exit(1);
	}
	if (!svc_register(transp, SPRAYPROG, SPRAYVERS, spray, IPPROTO_UDP)) {
		fprintf(stderr, "%s: couldn't register SPRAYPROG\n", argv[0]);
		exit(1);
	}
#endif /* USE_INET */
	signal(SIGALRM, closedown);
	alarm(CLOSEDOWN);
	svc_run();
	fprintf(stderr, "%s shouldn't reach this point\n", argv[0]);
	exit(1);
	/* NOTREACHED */
}

spray(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	dirty = 1;
	switch (rqstp->rq_proc) {
		case NULLPROC:
			if (!svc_sendreply(transp, xdr_void, 0)) {
				fprintf(stderr,"couldn't reply to RPC call\n");
				exit(1);
			}
			return;
		case SPRAYPROC_SPRAY:
			cumul.counter++;
			break;
		case SPRAYPROC_GET:
			gettimeofday(&cumul.clock, 0);
			if (cumul.clock.tv_usec < tv.tv_usec) {
				cumul.clock.tv_usec += 1000000;
				cumul.clock.tv_sec -= 1;
			}
			cumul.clock.tv_sec -= tv.tv_sec;
			cumul.clock.tv_usec -= tv.tv_usec;
			if (!svc_sendreply(transp, xdr_spraycumul, &cumul)) { 

				fprintf(stderr,"couldn't reply to RPC call\n");
				exit(1);
			} 
			return;

		case SPRAYPROC_CLEAR:
			cumul.counter = 0;
			gettimeofday(&tv, 0);
			if (!svc_sendreply(transp, xdr_void, 0)) {
				fprintf(stderr,"couldn't reply to RPC call\n");
				exit(1);
			}
			return;
		default:
			svcerr_noproc(transp);
			return;
	}
}

closedown()
{
	if (dirty) {
		dirty = 0;
	}
	else {
		exit(0);
	}
	alarm(CLOSEDOWN);
}
