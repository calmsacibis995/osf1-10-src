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
static char *sccsid = "@(#)$RCSfile: pnprpc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:00:01 $";
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

bool_t
xdr_pnp_errcode(xdrs, objp)
	XDR *xdrs;
	pnp_errcode *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_net_type(xdrs, objp)
	XDR *xdrs;
	net_type *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_hw_addr(xdrs, objp)
	XDR *xdrs;
	hw_addr *objp;
{
	if (!xdr_net_type(xdrs, &objp->hw)) {
		return (FALSE);
	}
	switch (objp->hw) {
	case ethernet:
		if (!xdr_opaque(xdrs, objp->hw_addr_u.enetaddr, 6)) {
			return (FALSE);
		}
		break;
	case ieee802_16:
		if (!xdr_opaque(xdrs, objp->hw_addr_u.lanaddr, 2)) {
			return (FALSE);
		}
		break;
	case ptp:
		break;
	case atalk:
		if (!xdr_string(xdrs, &objp->hw_addr_u.nbpid, 99)) {
			return (FALSE);
		}
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pnp_whoami_arg(xdrs, objp)
	XDR *xdrs;
	pnp_whoami_arg *objp;
{
	if (!xdr_hw_addr(xdrs, &objp->linkaddr)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->inetaddr)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_env_string(xdrs, objp)
	XDR *xdrs;
	env_string *objp;
{
	if (!xdr_string(xdrs, objp, MAX_ENVSTRING)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pnp_whoami_ret(xdrs, objp)
	XDR *xdrs;
	pnp_whoami_ret *objp;
{
	if (!xdr_pnp_errcode(xdrs, &objp->status)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->name, MAX_MACHINELEN)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->domain, MAX_DOMAINLEN)) {
		return (FALSE);
	}
	if (!xdr_array(xdrs, (char **)&objp->extension.extension_val, (u_int *)&objp->extension.extension_len, ~0, sizeof(env_string), xdr_env_string)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_how_to_boot(xdrs, objp)
	XDR *xdrs;
	how_to_boot *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pnp_acquire_arg(xdrs, objp)
	XDR *xdrs;
	pnp_acquire_arg *objp;
{
	if (!xdr_hw_addr(xdrs, &objp->linkaddr)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->inetaddr)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->arch, MAX_ARCHLEN)) {
		return (FALSE);
	}
	if (!xdr_how_to_boot(xdrs, &objp->how)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->memsize)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->disksize)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->hostid, ~0)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_pnp_setup_arg(xdrs, objp)
	XDR *xdrs;
	pnp_setup_arg *objp;
{
	if (!xdr_pnp_acquire_arg(xdrs, &objp->pa)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->name, MAX_MACHINELEN)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->keydata, ~0)) {
		return (FALSE);
	}
	return (TRUE);
}
