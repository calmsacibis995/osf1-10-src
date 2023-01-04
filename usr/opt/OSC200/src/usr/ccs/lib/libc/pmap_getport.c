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
static char     *sccsid = "@(#)$RCSfile: pmap_getport.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:33:59 $";
#endif
/*
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak pmap_getport = __pmap_getport
#endif
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef socket
#undef socket
#endif
#endif


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.10 88/02/08 
 */


/*
 * pmap_getport.c
 * Client interface to pmap rpc service.
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>
#include <net/if.h>

static struct timeval timeout = { 5, 0 };
static struct timeval tottimeout = { 60, 0 };

/*
 * Find the mapped port for program, version.
 * Calls the pmap service remotely to do the lookup.
 * Returns 0 if no map exists.
 */
u_short
pmap_getport(address, program, version, protocol)
	struct sockaddr_in *address;
	u_int program;
	u_int version;
	u_int protocol;
{
	u_short port = 0;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;

	address->sin_port = htons(PMAPPORT);
	client = clntudp_bufcreate(address, PMAPPROG,
	    PMAPVERS, timeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client != (CLIENT *)NULL) {
		parms.pm_prog = program;
		parms.pm_vers = version;
		parms.pm_prot = protocol;
		parms.pm_port = 0;  /* not needed or used */
		if (CLNT_CALL(client, PMAPPROC_GETPORT, xdr_pmap, &parms,
		    xdr_u_short, &port, tottimeout) != RPC_SUCCESS){
			rpc_createerr.cf_stat = RPC_PMAPFAILURE;
			clnt_geterr(client, &rpc_createerr.cf_error);
		} else if (port == 0) {
			rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
		}
		CLNT_DESTROY(client);
	}
	(void)close(socket);
	address->sin_port = 0;
	return (port);
}
