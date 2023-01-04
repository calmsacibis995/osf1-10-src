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
static char     *sccsid = "@(#)$RCSfile: auth_none.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/06/08 01:26:12 $";
#endif
/*
 */


/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 * 1.20 88/02/08
 */


/*
 * auth_none.c
 * Creates a client authentication handle for passing "null" 
 * credentials and verifiers to remote systems. 
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak authnone_create = __authnone_create
#endif
#endif
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#define MAX_MARSHEL_SIZE 20

/*
 * Authenticator operations routines
 */
static void	authnone_verf();
static void	authnone_destroy();
static bool_t	authnone_marshal();
static bool_t	authnone_validate();
static bool_t	authnone_refresh();

static struct auth_ops ops = {
	authnone_verf,
	authnone_marshal,
	authnone_validate,
	authnone_refresh,
	authnone_destroy
};

static struct authnone_private {
	AUTH	no_client;
	char	marshalled_client[MAX_MARSHEL_SIZE];
	u_int	mcnt;
} *authnone_private;

AUTH *
authnone_create()
{
	register struct authnone_private *ap = authnone_private;
	XDR xdr_stream;
	register XDR *xdrs;

	if (ap == 0) {
		ap = (struct authnone_private *)calloc(1, sizeof (*ap));
		if (ap == 0)
			return (0);
		authnone_private = ap;
	}
	if (!ap->mcnt) {
		/* ap->no_client.ah_cred = ap->no_client.ah_verf = _null_auth; */
		memcpy (&ap->no_client.ah_cred, &_null_auth, sizeof(_null_auth));
		memcpy (&ap->no_client.ah_verf, &_null_auth, sizeof(_null_auth));
		ap->no_client.ah_ops = &ops;
		xdrs = &xdr_stream;
		xdrmem_create(xdrs, ap->marshalled_client, (u_int)MAX_MARSHEL_SIZE,
		    XDR_ENCODE);
		(void)xdr_opaque_auth(xdrs, &ap->no_client.ah_cred);
		(void)xdr_opaque_auth(xdrs, &ap->no_client.ah_verf);
		ap->mcnt = XDR_GETPOS(xdrs);
		XDR_DESTROY(xdrs);
	}
	return (&ap->no_client);
}

/*ARGSUSED*/
static bool_t
authnone_marshal(client, xdrs)
	AUTH *client;
	XDR *xdrs;
{
	register struct authnone_private *ap = authnone_private;

	if (ap == 0)
		return (0);
	return ((*xdrs->x_ops->x_putbytes)(xdrs,
	    ap->marshalled_client, ap->mcnt));
}

static void 
authnone_verf()
{
}

static bool_t
authnone_validate()
{

	return (TRUE);
}

static bool_t
authnone_refresh()
{

	return (FALSE);
}

static void
authnone_destroy()
{
}
