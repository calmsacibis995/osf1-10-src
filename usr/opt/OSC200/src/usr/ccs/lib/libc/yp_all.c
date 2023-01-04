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
static char     *sccsid = "@(#)$RCSfile: yp_all.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/07/29 22:06:51 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak yp_all = __yp_all
#endif
#endif
#define NULL 0L
#include <sys/time.h>
#include <rpc/rpc.h>
#include <sys/syslog.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypv1_prot.h>
#include <rpcsvc/ypclnt.h>


static struct timeval tcp_timout = {
	120,				/* 120 seconds */
	0
	};
extern int _yp_dobind();
extern unsigned int _ypsleeptime;


/*
 * This does the "glommed enumeration" stuff.  callback->foreach is the name
 * of a function which gets called per decoded key-value pair:
 * 
 * (*callback->foreach)(status, key, keylen, val, vallen, callback->data);
 *
 * If the server we get back from _yp_dobind speaks the old protocol, this
 * returns YPERR_VERS, and does not attempt to emulate the new functionality
 * by using the old protocol.
 */
int
yp_all (domain, map, callback)
	char *domain;
	char *map;
	struct ypall_callback *callback;
{
	int domlen;
	int maplen;
	struct ypreq_nokey req;
	int reason;
	struct dom_binding *pdomb;
	enum clnt_stat s;

	if ( (map == NULL) || (domain == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ||
	    (callback == (struct ypall_callback *) NULL) ) {
		return(YPERR_BADARGS);
	}

	if (reason = _yp_dobind(domain, &pdomb) ) {
		return(reason);
	}

	if (pdomb->dom_vers == YPOLDVERS) {
		return (YPERR_VERS);
	}
		
	clnt_destroy(pdomb->dom_client);
	(void) close(pdomb->dom_socket);
	pdomb->dom_socket = RPC_ANYSOCK;
	pdomb->dom_server_port = pdomb->dom_server_addr.sin_port = 0;
	
	if ((pdomb->dom_client = clnttcp_create(&(pdomb->dom_server_addr),
	    YPPROG, YPVERS, &(pdomb->dom_socket), 0, 0)) ==
	    (CLIENT *) NULL) {
		    (void) syslog(LOG_ERR,
		    clnt_spcreateerror("yp_all - TCP channel create failure"));
		    return YPERR_RPC;
	}

	req.domain = domain;
	req.map = map;
	
	s = clnt_call(pdomb->dom_client, YPPROC_ALL, xdr_ypreq_nokey, &req,
	    xdr_ypall, callback, tcp_timout);

	if (s != RPC_SUCCESS) {
		(void) syslog(LOG_ERR, clnt_sperror(pdomb->dom_client,
		    "yp_all - RPC clnt_call (TCP) failure"));
	}

	(void) close(pdomb->dom_socket);
	yp_unbind(domain);
	
	if (s == RPC_SUCCESS) {
		return(0);
	} else {
		return(YPERR_RPC);
	}
}

