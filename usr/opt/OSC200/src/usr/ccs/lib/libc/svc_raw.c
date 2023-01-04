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
static char     *sccsid = "@(#)$RCSfile: svc_raw.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/21 21:31:14 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.16 88/02/08 
 */


/*
 * svc_raw.c,   This a toy for simple testing and timing.
 * Interface to create an rpc client and server in the same UNIX process.
 * This lets us similate rpc and get rpc (round trip) overhead, without
 * any interference from the kernal.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak svcraw_create = __svcraw_create
#endif
#endif
#include <rpc/rpc.h>
#include <rpc/raw.h>


/*
 * This is the "network" that we will be moving data over
 */
static struct svcraw_private {
	char	*raw_buf;
	SVCXPRT	server;
	XDR	xdr_stream;
	char	verf_body[MAX_AUTH_BYTES];
} *svcraw_private;

extern char	*_rpcrawcombuf;
static bool_t		svcraw_recv();
static enum xprt_stat 	svcraw_stat();
static bool_t		svcraw_getargs();
static bool_t		svcraw_reply();
static bool_t		svcraw_freeargs();
static void		svcraw_destroy();

static struct xp_ops server_ops = {
	svcraw_recv,
	svcraw_stat,
	svcraw_getargs,
	svcraw_reply,
	svcraw_freeargs,
	svcraw_destroy
};

SVCXPRT *
svcraw_create()
{
	register struct svcraw_private *srp = svcraw_private;

	if (srp == NULL) {
		srp = (struct svcraw_private *)calloc(1, sizeof (*srp));
		if (srp == NULL)
			return (NULL);
		if (_rpcrawcombuf == NULL) {
			_rpcrawcombuf = (char *)calloc(UDPMSGSIZE,
							sizeof (char));
			if (_rpcrawcombuf == NULL)
				return (NULL);
		}
		srp->raw_buf = _rpcrawcombuf; /* Share it with the client */
		svcraw_private = srp;
	}
	srp->server.xp_sock = 0;
	srp->server.xp_port = 0;
	srp->server.xp_p3 = NULL;
	srp->server.xp_ops = &server_ops;
	srp->server.xp_verf.oa_base = srp->verf_body;
	xdrmem_create(&srp->xdr_stream, srp->raw_buf, UDPMSGSIZE, XDR_FREE);
	/*
	 * Normally the transport layer (ie tcp,udp) would register the
	 * transport.  However since no protocol is used for the raw case
	 * it is necessary to manually resister here.  (Otherwise a seg
	 * fault will occur when the "xports" array is accessed in svc.c
	 * because it hasn't been initialized from the xprt_register call.
	 */
	xprt_register(&srp->server);	/* tim added */
	return (&srp->server);
}

static enum xprt_stat
svcraw_stat()
{

	return (XPRT_IDLE);
}

static bool_t
svcraw_recv(xprt, msg)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register struct svcraw_private *srp = svcraw_private;
	register XDR *xdrs;

	if (srp == 0)
		return (0);
	xdrs = &srp->xdr_stream;
	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	if (! xdr_callmsg(xdrs, msg))
	       return (FALSE);
	return (TRUE);
}

static bool_t
svcraw_reply(xprt, msg)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register struct svcraw_private *srp = svcraw_private;
	register XDR *xdrs;

	if (srp == 0)
		return (FALSE);
	xdrs = &srp->xdr_stream;
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	if (! xdr_replymsg(xdrs, msg))
	       return (FALSE);
	(void)XDR_GETPOS(xdrs);  /* called just for overhead */
	return (TRUE);
}

static bool_t
svcraw_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{
	register struct svcraw_private *srp = svcraw_private;

	if (srp == 0)
		return (FALSE);
	return ((*xdr_args)(&srp->xdr_stream, args_ptr));
}

static bool_t
svcraw_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{ 
	register struct svcraw_private *srp = svcraw_private;
	register XDR *xdrs;

	if (srp == 0)
		return (FALSE);
	xdrs = &srp->xdr_stream;
	xdrs->x_op = XDR_FREE;
	return ((*xdr_args)(xdrs, args_ptr));
} 

static void
svcraw_destroy()
{
}
