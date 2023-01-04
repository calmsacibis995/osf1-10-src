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
static char     *sccsid = "@(#)$RCSfile: svc_simple.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:11:08 $";
#endif
/*
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak registerrpc = __registerrpc
#endif
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.19 88/02/08
 */


/* 
 * svc_simple.c
 * Simplified front end to rpc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/syslog.h>

static struct proglst {
	char *(*p_progname)();
	int  p_prognum;
	int  p_procnum;
	xdrproc_t p_inproc, p_outproc;
	struct proglst *p_nxt;
} *proglst;
static void universal();
static SVCXPRT *transp;
struct proglst *pl;

registerrpc(prognum, versnum, procnum, progname, inproc, outproc)
	char *(*progname)();
	xdrproc_t inproc, outproc;
{
	
	if (procnum == NULLPROC) {
		(void) syslog(LOG_ERR,
		    "can't reassign procedure number %d", NULLPROC);
		return (-1);
	}
	if (transp == 0) {
		transp = svcudp_create(RPC_ANYSOCK);
		if (transp == NULL) {
			(void) syslog(LOG_ERR, "couldn't create an rpc server");
			return (-1);
		}
	}
	(void) pmap_unset((u_int)prognum, (u_int)versnum);
	if (!svc_register(transp, (u_int)prognum, (u_int)versnum, 
	    universal, IPPROTO_UDP)) {
		(void) syslog(LOG_ERR, "couldn't register prog %d vers %d",
		    prognum, versnum);
		return (-1);
	}
	pl = (struct proglst *)malloc(sizeof(struct proglst));
	if (pl == NULL) {
		(void) syslog(LOG_ERR, "registerrpc: out of memory");
		return (-1);
	}
	pl->p_progname = progname;
	pl->p_prognum = prognum;
	pl->p_procnum = procnum;
	pl->p_inproc = inproc;
	pl->p_outproc = outproc;
	pl->p_nxt = proglst;
	proglst = pl;
	return (0);
}

#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef pl
#undef pl
#endif
#endif

static void
universal(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	int prog, proc;
	char *outdata;
	char xdrbuf[UDPMSGSIZE];
	struct proglst *pl;

	/* 
	 * enforce "procnum 0 is echo" convention
	 */
	if (rqstp->rq_proc == NULLPROC) {
		if (svc_sendreply(transp, xdr_void, (char *)NULL) == FALSE) {
			(void) syslog(LOG_ERR, "svc_sendreply failed");
		}
		return;
	}
	prog = rqstp->rq_prog;
	proc = rqstp->rq_proc;
	for (pl = proglst; pl != NULL; pl = pl->p_nxt)
		if (pl->p_prognum == prog && pl->p_procnum == proc) {
			/* decode arguments into a CLEAN buffer */
			bzero(xdrbuf, sizeof(xdrbuf)); /* required ! */
			if (!svc_getargs(transp, pl->p_inproc, xdrbuf)) {
				svcerr_decode(transp);
				return;
			}
			outdata = (*(pl->p_progname))(xdrbuf);
			if (outdata == NULL && pl->p_outproc != xdr_void)
				/* there was an error */
				return;
			if (!svc_sendreply(transp, pl->p_outproc, outdata)) {
				(void) syslog(LOG_ERR,
				    "trouble replying to prog %d",
				    pl->p_prognum);
				return;
			}
			/* free the decoded arguments */
			(void)svc_freeargs(transp, pl->p_inproc, xdrbuf);
			return;
		}
	(void) syslog(LOG_ERR, "never registered prog %d", prog);
	exit(1);
}
