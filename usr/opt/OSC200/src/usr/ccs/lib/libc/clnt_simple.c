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
static char     *sccsid = "@(#)$RCSfile: clnt_simple.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:40:47 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.36 88/02/08 
 */


/* 
 * clnt_simple.c
 * Simplified front end to rpc.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak callrpc = __callrpc
#endif
#endif
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>

static struct callrpc_private {
	CLIENT	*client;
	int	socket;
	int	oldprognum, oldversnum, valid;
	char	*oldhost;
} *callrpc_private;

callrpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	register struct callrpc_private *crp = callrpc_private;
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;

	if (crp == 0) {
		crp = (struct callrpc_private *)calloc(1, sizeof (*crp));
		if (crp == 0)
			return (0);
		callrpc_private = crp;
	}
	if (crp->oldhost == NULL) {
		crp->oldhost = malloc(256);
		crp->oldhost[0] = 0;
		crp->socket = RPC_ANYSOCK;
	}
	if (crp->valid && crp->oldprognum == prognum &&
		crp->oldversnum == versnum &&
		strcmp(crp->oldhost, host) == 0) {
		/* reuse old client */
	} else {
		crp->valid = 0;
		(void)close(crp->socket);
		crp->socket = RPC_ANYSOCK;
		if (crp->client) {
			clnt_destroy(crp->client);
			crp->client = NULL;
		}
		if ((hp = gethostbyname(host)) == NULL)
			return ((int) RPC_UNKNOWNHOST);
		timeout.tv_usec = 0;
		timeout.tv_sec = 5;
		bcopy(hp->h_addr, (char *)&server_addr.sin_addr, hp->h_length);
		server_addr.sin_family = AF_INET;
		server_addr.sin_port =  0;
		if ((crp->client = clntudp_create(&server_addr, (u_int)prognum,
		    (u_int)versnum, timeout, &crp->socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		crp->valid = 1;
		crp->oldprognum = prognum;
		crp->oldversnum = versnum;
		(void) strcpy(crp->oldhost, host);
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(crp->client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		crp->valid = 0;
	return ((int) clnt_stat);
}
