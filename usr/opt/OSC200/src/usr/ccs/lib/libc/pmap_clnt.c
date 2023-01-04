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
static char     *sccsid = "@(#)$RCSfile: pmap_clnt.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:33:32 $";
#endif
/*
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak pmap_set = __pmap_set
#pragma weak pmap_unset = __pmap_unset
#endif
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef socket
#undef socket
#endif
#endif


/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.38 88/02/08 
 */


/*
 * pmap_clnt.c
 * Client interface to pmap rpc service.
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/syslog.h>

static struct timeval timeout = { 5, 0 };
static struct timeval tottimeout = { 60, 0 };

/*
 * Set a mapping between program,version and port.
 * Calls the pmap service remotely to do the mapping.
 */
bool_t
pmap_set(program, version, protocol, port)
	u_int program;
	u_int version;
	int protocol;
	u_short port;
{
	struct sockaddr_in myaddress;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;
	bool_t rslt;

	get_myaddress(&myaddress);
	client = clntudp_bufcreate(&myaddress, PMAPPROG, PMAPVERS,
	    timeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client == (CLIENT *)NULL)
		return (FALSE);
	parms.pm_prog = program;
	parms.pm_vers = version;
	parms.pm_prot = protocol;
	parms.pm_port = port;
	if (CLNT_CALL(client, PMAPPROC_SET, xdr_pmap, &parms, xdr_bool, &rslt,
	    tottimeout) != RPC_SUCCESS) {
		(void) syslog(LOG_ERR, clnt_sperror(client, "Cannot register service"));
		return (FALSE);
	}
	CLNT_DESTROY(client);
	(void)close(socket);
	return (rslt);
}

/*
 * Remove the mapping between program,version and port.
 * Calls the pmap service remotely to do the un-mapping.
 */
bool_t
pmap_unset(program, version)
	u_int program;
	u_int version;
{
	struct sockaddr_in myaddress;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;
	bool_t rslt;

	get_myaddress(&myaddress);
	client = clntudp_bufcreate(&myaddress, PMAPPROG, PMAPVERS,
	    timeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client == (CLIENT *)NULL)
		return (FALSE);
	parms.pm_prog = program;
	parms.pm_vers = version;
	parms.pm_port = parms.pm_prot = 0;
	CLNT_CALL(client, PMAPPROC_UNSET, xdr_pmap, &parms, xdr_bool, &rslt,
	    tottimeout);
	CLNT_DESTROY(client);
	(void)close(socket);
	return (rslt);
}
