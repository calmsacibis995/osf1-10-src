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
static char *rcsid = "@(#)$RCSfile: tcp.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 12:12:54 $";
#endif
#ifndef lint
static char sccsid[] = "@(#)tcp.c	1.2 90/07/23 NFSSRC4.1 Copyr 1990 Sun Micro";
#endif

	/*
	 * Copyright (c) 1988 by Sun Microsystems, Inc.
	 */

	/*
	 * make tcp calls
	 */
#include <stdio.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/time.h>

extern int debug;
/*
 *  routine taken from new_calltcp.c;
 *  no caching is done!
 *  continueously calling if timeout;
 *  in case of error, print put error msg; this msg usually is to be
 *  thrown away
 */
int
call_tcp(host, prognum, versnum, procnum, inproc, in, outproc, out, tot )
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
	int tot;
{
	struct sockaddr_in server_addr;
	struct in_addr *get_addr_cache();
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval  tottimeout;
	register CLIENT *client;
	int socket = RPC_ANYSOCK;

	if ((hp = gethostbyname(host)) == NULL) {
		if (debug)
			printf( "RPC_UNKNOWNHOST\n");
		return ((int) RPC_UNKNOWNHOST);
	}
	bcopy(hp->h_addr, (caddr_t)&server_addr.sin_addr, hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port =  0;

	tottimeout.tv_usec = 0;
	tottimeout.tv_sec = tot;
	if ((client = clnttcp_create(&server_addr, prognum, versnum, &socket,
		0, 0)) == NULL) {
		clnt_pcreateerror("clnttcp_create");   /* RPC_PMAPFAILURE or RPC_SYSTEMERROR */
		return ((int) rpc_createerr.cf_stat);  /* return (svr_not_avail); */
	}
again:
	clnt_stat = clnt_call(client, procnum, inproc, in, outproc, out,
			tottimeout);
	if (clnt_stat != RPC_SUCCESS)  {
		if (clnt_stat == RPC_TIMEDOUT) {
			if (tot != 0) {
				if (debug)
					printf("call_tcp timeout, retry\n");
				goto again;
			}
			/* if tot == 0, no reply is expected */
		}
		else {
			if (debug) {
				clnt_perrno(clnt_stat);
				fprintf(stderr, "\n");
			}
		}
	}
	/* should do cacheing, rather than always destroy */
	(void) close(socket);
	clnt_destroy(client);
	return (int) clnt_stat;
}
