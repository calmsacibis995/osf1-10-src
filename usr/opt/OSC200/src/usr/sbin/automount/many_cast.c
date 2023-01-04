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
#ifdef ultrix
static char     *sccsid = "%W%  ULTRIX  %G%";
#else /* ultrix */
static char     *sccsid = "@(#)$RCSfile: many_cast.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/11 17:32:25 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

/*
 * many_cast: like broadcast, but to a specific group of hosts
 *
 *		**** This module is not used ****
 * A similar routine, nfs_cast(), is used.  The main difference is that this
 * calls NFS through portmap's PMAPPROC_CALLIT, nfs_cast goes to NFS directly
 * via NFS_PORT.
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

extern int errno;

/*
 * Structures and XDR routines for parameters to and replys from
 * the pmapper remote-call-service.
 */

struct rmtcallargs {
	u_int prog, vers, proc, arglen;
	caddr_t args_ptr;
	xdrproc_t xdr_args;
};
static bool_t xdr_rmtcall_args();

struct rmtcallres {
	u_long *port_ptr;
	u_int resultslen;
	caddr_t results_ptr;
	xdrproc_t xdr_results;
};
static bool_t xdr_rmtcallres();

/*
 * XDR remote call arguments
 * written for XDR_ENCODE direction only
 */
static bool_t
xdr_rmtcall_args(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcallargs *cap;
{
	u_int lenposition, argposition, position;

	if (xdr_u_int(xdrs, &(cap->prog)) &&
	    xdr_u_int(xdrs, &(cap->vers)) &&
	    xdr_u_int(xdrs, &(cap->proc))) {
		lenposition = XDR_GETPOS(xdrs);
		if (! xdr_u_int(xdrs, &(cap->arglen)))
		    return (FALSE);
		argposition = XDR_GETPOS(xdrs);
		if (! (*(cap->xdr_args))(xdrs, cap->args_ptr))
		    return (FALSE);
		position = XDR_GETPOS(xdrs);
		cap->arglen = position - argposition;
		XDR_SETPOS(xdrs, lenposition);
		if (! xdr_u_int(xdrs, &(cap->arglen)))
		    return (FALSE);
		XDR_SETPOS(xdrs, position);
		return (TRUE);
	}
	return (FALSE);
}

/*
 * XDR remote call results
 * written for XDR_DECODE direction only
 */
static bool_t
xdr_rmtcallres(xdrs, crp)
	register XDR *xdrs;
	register struct rmtcallres *crp;
{

	if (xdr_reference(xdrs, &crp->port_ptr, sizeof (u_int), xdr_u_int) &&
		xdr_u_int(xdrs, &crp->resultslen))
		return ((*(crp->xdr_results))(xdrs, crp->results_ptr));
	return (FALSE);
}

typedef bool_t (*resultproc_t)();

#define	MAXHOSTS	20

enum clnt_stat 
many_cast(addrs, prog, vers, proc, xargs, argsp,
				xresults, resultsp, eachresult, timeout)
	struct in_addr *addrs;		/* list of host addresses */
	u_int		prog;		/* program number */
	u_int		vers;		/* version number */
	u_int		proc;		/* procedure number */
	xdrproc_t	xargs;		/* xdr routine for args */
	caddr_t		argsp;		/* pointer to args */
	xdrproc_t	xresults;	/* xdr routine for results */
	caddr_t		resultsp;	/* pointer to results */
	resultproc_t	eachresult;	/* call with each result obtained */
	int		timeout;	/* timeout (sec) */
{
	enum clnt_stat stat;
	AUTH *unix_auth = authunix_create_default();
	XDR xdr_stream;
	register XDR *xdrs = &xdr_stream;
	int outlen, inlen, fromlen, readfds;
	register int sock, mask, i;
	bool_t done = FALSE;
	register u_int xid;
	u_int port;
	struct sockaddr_in baddr, raddr; /* broadcast and response addresses */
	struct rmtcallargs a;
	struct rmtcallres r;
	struct rpc_msg msg;
	struct timeval t; 
	char outbuf[UDPMSGSIZE], inbuf[UDPMSGSIZE];

	/*
	 * initialization: create a socket, a broadcast address, and
	 * preserialize the arguments into a send buffer.
	 */
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		syslog(LOG_ERR, "Cannot create socket for broadcast rpc: %m");
		stat = RPC_CANTSEND;
		goto done_broad;
	}
	mask = (1 << sock);
	bzero((char *)&baddr, sizeof (baddr));
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(PMAPPORT);
	(void)gettimeofday(&t, (struct timezone *)0);
	xid = (getpid() ^ t.tv_sec ^ t.tv_usec) & ~0xFF;
	t.tv_usec = 0;
	msg.rm_direction = CALL;
	msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	msg.rm_call.cb_prog = PMAPPROG;
	msg.rm_call.cb_vers = PMAPVERS;
	msg.rm_call.cb_proc = PMAPPROC_CALLIT;
	msg.rm_call.cb_cred = unix_auth->ah_cred;
	msg.rm_call.cb_verf = unix_auth->ah_verf;
	a.prog = prog;
	a.vers = vers;
	a.proc = proc;
	a.xdr_args = xargs;
	a.args_ptr = argsp;
	r.port_ptr = (u_long *)&port;
	r.xdr_results = xresults;
	r.results_ptr = resultsp;
	xdrmem_create(xdrs, outbuf, sizeof outbuf, XDR_ENCODE);
	if ((! xdr_callmsg(xdrs, &msg)) || (! xdr_rmtcall_args(xdrs, &a))) {
		stat = RPC_CANTENCODEARGS;
		goto done_broad;
	}
	outlen = (int)xdr_getpos(xdrs);
	xdr_destroy(xdrs);
	/*
	 * Basic loop: send packet to all hosts and wait for response(s).
	 * The response timeout grows larger per iteration.
	 * A unique xid is assigned to each request in order to
	 * correctly match the replies.
	 */
	for (t.tv_sec = 4; t.tv_sec <= 14; t.tv_sec += 2) {
		if (timeout - t.tv_sec <= 0) {
			stat = RPC_TIMEDOUT;
			goto done_broad;
		}
		timeout -= t.tv_sec;
		for (i = 0; addrs[i].s_addr != 0; i++) {
			baddr.sin_addr = addrs[i];
			/* xid is first thing in preserialized buffer */
			*((u_int *)outbuf) = htonl(xid + i);
			if (sendto(sock, outbuf, outlen, 0,
			    (struct sockaddr *)&baddr,
			    sizeof (struct sockaddr)) != outlen) {
				syslog(LOG_ERR,
				    "Cannot send broadcast packet: %m");
				stat = RPC_CANTSEND;
				goto done_broad;
			}
		}
	recv_again:
		msg.acpted_rply.ar_verf = _null_auth;
		msg.acpted_rply.ar_results.where = (caddr_t)&r;
		msg.acpted_rply.ar_results.proc = xdr_rmtcallres;
		readfds = mask;
		switch (select(32, &readfds, (int *)NULL, (int *)NULL, &t)) {

		case 0:  /* timed out */
			stat = RPC_TIMEDOUT;
			continue;

		case -1:  /* some kind of error */
			if (errno == EINTR)
				goto recv_again;
			syslog(LOG_ERR, "Many_cast select problem: %m");
			stat = RPC_CANTRECV;
			goto done_broad;

		}  /* end of select results switch */
		if ((readfds & mask) == 0)
			goto recv_again;
	try_again:
		fromlen = sizeof(struct sockaddr);
		inlen = recvfrom(sock, inbuf, sizeof inbuf, 0,
			(struct sockaddr *)&raddr, &fromlen);
		if (inlen < 0) {
			if (errno == EINTR)
				goto try_again;
			syslog(LOG_ERR,
			    "Cannot receive reply to many_cast: %m");
			stat = RPC_CANTRECV;
			goto done_broad;
		}
		if (inlen < sizeof(u_int))
			goto recv_again;
		/*
		 * see if reply transaction id matches sent id.
		 * If so, decode the results.
		 */
		xdrmem_create(xdrs, inbuf, inlen, XDR_DECODE);
		if (xdr_replymsg(xdrs, &msg)) {
			if (((msg.rm_xid & ~0xFF) == xid) &&
				(msg.rm_reply.rp_stat == MSG_ACCEPTED) &&
				(msg.acpted_rply.ar_stat == SUCCESS)) {
				raddr.sin_port = htons((u_short)port);
				raddr.sin_addr = addrs[msg.rm_xid - xid];
				done = (*eachresult)(resultsp, &raddr);
			}
			/* otherwise, we just ignore the errors ... */
		}
		xdrs->x_op = XDR_FREE;
		msg.acpted_rply.ar_results.proc = xdr_void;
		(void)xdr_replymsg(xdrs, &msg);
		(void)(*xresults)(xdrs, resultsp);
		xdr_destroy(xdrs);
		if (done) {
			stat = RPC_SUCCESS;
			goto done_broad;
		} else {
			goto recv_again;
		}
	}
done_broad:
	(void)close(sock);
	AUTH_DESTROY(unix_auth);
	return (stat);
}
