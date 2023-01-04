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
static char *sccsid = "@(#)$RCSfile: pnprpc_clnt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:00:16 $";
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
#include <rpcsvc/pnprpc.h>

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

pnp_whoami_ret *
pnp_whoami_2(argp, clnt)
	pnp_whoami_arg *argp;
	CLIENT *clnt;
{
	static pnp_whoami_ret res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, PNP_WHOAMI, xdr_pnp_whoami_arg, argp, xdr_pnp_whoami_ret, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

pnp_errcode *
pnp_acquire_2(argp, clnt)
	pnp_acquire_arg *argp;
	CLIENT *clnt;
{
	static pnp_errcode res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, PNP_ACQUIRE, xdr_pnp_acquire_arg, argp, xdr_pnp_errcode, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

pnp_errcode *
pnp_setup_2(argp, clnt)
	pnp_setup_arg *argp;
	CLIENT *clnt;
{
	static pnp_errcode res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, PNP_SETUP, xdr_pnp_setup_arg, argp, xdr_pnp_errcode, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

pnp_errcode *
pnp_poll_2(argp, clnt)
	pnp_setup_arg *argp;
	CLIENT *clnt;
{
	static pnp_errcode res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, PNP_POLL, xdr_pnp_setup_arg, argp, xdr_pnp_errcode, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}
