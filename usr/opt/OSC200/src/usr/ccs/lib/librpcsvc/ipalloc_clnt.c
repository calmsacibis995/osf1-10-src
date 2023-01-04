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
static char *sccsid = "@(#)$RCSfile: ipalloc_clnt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:00:04 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Portions of this software have been licensed to
 * Digital Equipment Company, Maynard, MA.
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/ipalloc.h>

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

ip_alloc_res *
ip_alloc_2(argp, clnt)
	ip_alloc_arg *argp;
	CLIENT *clnt;
{
	static ip_alloc_res res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, IP_ALLOC, xdr_ip_alloc_arg, argp, xdr_ip_alloc_res, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

ip_toname_res *
ip_toname_2(argp, clnt)
	ip_addr_arg *argp;
	CLIENT *clnt;
{
	static ip_toname_res res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, IP_TONAME, xdr_ip_addr_arg, argp, xdr_ip_toname_res, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

ip_status *
ip_free_2(argp, clnt)
	ip_addr_arg *argp;
	CLIENT *clnt;
{
	static ip_status res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, IP_FREE, xdr_ip_addr_arg, argp, xdr_ip_status, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}
