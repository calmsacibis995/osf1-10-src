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
static char *rcsid = "@(#)$RCSfile: pmap_kgetport.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/03/30 23:11:53 $";
#endif
#ifdef KERNEL
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)pmap_kgetport.c	1.6 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.10 88/02/08 
 */


/*
 * pmap_kgetport.c
 * Kernel interface to pmap rpc service.
 */
#include <rpc/rpc.h>
#include <sys/time.h>
#include <rpc/pmap_prot.h>

static struct ucred cred;
#define retries 4
static struct timeval tottimeout = { 1, 0 };


/*
 * Find the mapped port for program, version.
 * Calls the pmap service remotely to do the lookup.
 *
 * The 'address' argument is used to locate the portmapper, then
 * modified to contain the port number, if one was found.  If no
 * port number was found, 'address'->sin_port returns unchanged.
 *
 * Returns:	 0  if port number successfully found for 'program'
 *		-1  (<0) if 'program' was not registered
 *		 1  (>0) if there was an error contacting the portmapper
 */
int
pmap_kgetport(address, program, version, protocol)
	struct sockaddr_in *address;
	u_int program;
	u_int version;
	u_int protocol;
{
	u_short port = 0;
	register CLIENT *client;
	struct rpcpmap parms;
	int error = 0;
	struct sockaddr_in tmpaddr;
	 
	if (cred.cr_ref == 0) {
		/*
		 * Reduce the number of groups in the cred from NGROUPS to 0.
		 */
		cred.cr_ngroups = 0;
		cred.cr_ref++;
	}

	/* copy 'address' so that it doesn't get trashed */
	tmpaddr = *address;

	tmpaddr.sin_port = htons(PMAPPORT);
	client = clntkudp_create(&tmpaddr, PMAPPROG, PMAPVERS, retries, &cred);

	if (client != (CLIENT *)NULL) {
		parms.pm_prog = program;
		parms.pm_vers = version;
		parms.pm_prot = protocol;
		parms.pm_port = 0;  /* not needed or used */
		if (CLNT_CALL(client, PMAPPROC_GETPORT, xdr_pmap, &parms,
		    xdr_u_short, &port, tottimeout) != RPC_SUCCESS){
			error = 1;	/* error contacting portmapper */
		} else if (port == 0) {
			error = -1;	/* program not registered */
		} else {
			address->sin_port = htons(port);	/* save the port # */
		}
		AUTH_DESTROY(client->cl_auth);
		CLNT_DESTROY(client);
	}

	return (error);
}

/*
 * getport_loop -- kernel interface to pmap_kgetport()
 *
 * Talks to the portmapper using the sockaddr_in supplied by 'address',
 * to lookup the specified 'program'.
 *
 * Modifies 'address'->sin_port by rewriting the port number, if one
 * was found.  If a port number was not found (ie, return value != 0),
 * then 'address'->sin_port is left unchanged.
 *
 * If the portmapper does not respond, prints console message (once).
 * Retries forever, unless a signal is received.
 *
 * Returns:	 0  the port number was successfully put into 'address'
 *		-1  (<0) the requested process is not registered.
 *		 1  (>0) the portmapper did not respond and a signal occurred.
 */
getport_loop(address, program, version, protocol)
	struct sockaddr_in *address;
	u_int program;
	u_int version;
	u_int protocol;
{
	register int pe = 0;
	register int i = 0;

	/* sit in a tight loop until the portmapper responds */
	while ((i = pmap_kgetport(address, program, version, protocol)) > 0) {

		/* test to see if a signal has come in */
		if (ISSIG(u.u_procp)) {
			printf("Portmapper not responding; giving up\n");
			goto out;		/* got a signal */
		}
		/* print this message only once */
		if (pe++ == 0) {
			printf("Portmapper not responding; still trying\n");
		}
	}				/* go try the portmapper again */

	/* got a response...print message if there was a delay */
	if (pe != 0) {
		printf("Portmapper ok\n");
	}
out:
	return(i);	/* may return <0 if program not registered */
}
#endif	/* KERNEL */
